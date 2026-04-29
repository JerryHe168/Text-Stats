#include "text_stats.h"
#include "encoding_utils.h"
#include <fstream>
#include <sstream>
#include <cctype>
#include <cstring>
#include <algorithm>
#include <limits>
#include <stdexcept>

#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
#endif

using namespace TextStatsConstants;

namespace {
    uint64_t calculateTotalWords(const std::unordered_map<std::string, uint64_t>& wordCount) {
        uint64_t total = 0;
        for (const auto& pair : wordCount) {
            total += pair.second;
        }
        return total;
    }

    bool isPunctuationCodepoint(uint32_t codepoint) {
        return (codepoint >= 0x21 && codepoint <= 0x2F) ||
               (codepoint >= 0x3A && codepoint <= 0x40) ||
               (codepoint >= 0x5B && codepoint <= 0x60) ||
               (codepoint >= 0x7B && codepoint <= 0x7E);
    }

    bool isWhitespaceCodepoint(uint32_t codepoint) {
        return codepoint == ' ' || codepoint == '\t' || codepoint == '\n' ||
               codepoint == '\r' || codepoint == '\f' || codepoint == '\v';
    }
}

TextStats::TextStats() 
    : max_file_size_(DEFAULT_MAX_FILE_SIZE) {
    reset();
}

uint64_t TextStats::getMaxFileSize() const {
    return max_file_size_;
}

void TextStats::setMaxFileSize(uint64_t maxSize) {
    max_file_size_ = maxSize;
}

void TextStats::reset() {
    std::memset(&stats_, 0, sizeof(stats_));
    stats_.file_info.encoding = Encoding::UNKNOWN;
    stats_.line_stats.shortest_line_length = std::numeric_limits<uint64_t>::max();
    stats_.word_stats.top_words.clear();
}

bool TextStats::analyzeContent(const std::string& filepath, const std::string& content, uint64_t fileSize) {
    stats_.file_info.file_path = filepath;
    stats_.file_info.file_size = fileSize;
    stats_.file_info.encoding = detectEncoding(content);

    countAllStats(content);

    return true;
}

#ifdef _WIN32
bool TextStats::analyzeFile(const std::wstring& filepath) {
    reset();

    FILE* file = _wfopen(filepath.c_str(), L"rb");
    if (!file) {
        return false;
    }

    fseek(file, 0, SEEK_END);
    long fileSizeLong = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (fileSizeLong < 0) {
        fclose(file);
        return false;
    }

    uint64_t fileSize = static_cast<uint64_t>(fileSizeLong);

    if (fileSize > max_file_size_) {
        fclose(file);
        return false;
    }

    std::string content;
    if (fileSize > 0) {
        content.resize(static_cast<size_t>(fileSize), '\0');
        if (fread(&content[0], 1, static_cast<size_t>(fileSize), file) != static_cast<size_t>(fileSize)) {
            fclose(file);
            return false;
        }
    }

    fclose(file);

    return analyzeContent(encoding_utils::wideToUtf8(filepath), content, fileSize);
}
#endif

bool TextStats::analyzeFile(const std::string& filepath) {
    reset();

    std::string content;
    uint64_t fileSize = 0;

#ifdef _WIN32
    std::wstring widePath = encoding_utils::utf8ToWide(filepath);
    FILE* file = _wfopen(widePath.c_str(), L"rb");
    if (!file) {
        return false;
    }

    fseek(file, 0, SEEK_END);
    long fileSizeLong = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (fileSizeLong < 0) {
        fclose(file);
        return false;
    }

    fileSize = static_cast<uint64_t>(fileSizeLong);

    if (fileSize > max_file_size_) {
        fclose(file);
        return false;
    }

    if (fileSize > 0) {
        content.resize(static_cast<size_t>(fileSize), '\0');
        if (fread(&content[0], 1, static_cast<size_t>(fileSize), file) != static_cast<size_t>(fileSize)) {
            fclose(file);
            return false;
        }
    }

    fclose(file);
#else
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file) {
        return false;
    }

    std::streampos fileSizePos = file.tellg();
    if (fileSizePos < 0) {
        return false;
    }

    fileSize = static_cast<uint64_t>(fileSizePos);

    if (fileSize > max_file_size_) {
        return false;
    }

    file.seekg(0, std::ios::beg);

    if (fileSize > 0) {
        content.resize(static_cast<size_t>(fileSize), '\0');
        if (!file.read(&content[0], fileSizePos)) {
            return false;
        }
    }
#endif

    return analyzeContent(filepath, content, fileSize);
}

Encoding TextStats::detectEncoding(const std::string& content) const {
    if (content.size() < BOM_UTF16_LENGTH) {
        return Encoding::ASCII;
    }

    if (content.size() >= BOM_UTF8_LENGTH &&
        static_cast<unsigned char>(content[0]) == 0xEF &&
        static_cast<unsigned char>(content[1]) == 0xBB &&
        static_cast<unsigned char>(content[2]) == 0xBF) {
        return Encoding::UTF8;
    }

    if (content.size() >= BOM_UTF16_LENGTH) {
        if (static_cast<unsigned char>(content[0]) == 0xFF &&
            static_cast<unsigned char>(content[1]) == 0xFE) {
            if (content.size() >= BOM_UTF32_LENGTH && content[2] == 0x00 && content[3] == 0x00) {
                return Encoding::UTF32_LE;
            }
            return Encoding::UTF16_LE;
        }
        if (static_cast<unsigned char>(content[0]) == 0xFE &&
            static_cast<unsigned char>(content[1]) == 0xFF) {
            return Encoding::UTF16_BE;
        }
    }

    if (content.size() >= BOM_UTF32_LENGTH &&
        static_cast<unsigned char>(content[0]) == 0x00 &&
        static_cast<unsigned char>(content[1]) == 0x00 &&
        static_cast<unsigned char>(content[2]) == 0xFE &&
        static_cast<unsigned char>(content[3]) == 0xFF) {
        return Encoding::UTF32_BE;
    }

    if (isUTF8Valid(content)) {
        return Encoding::UTF8;
    }

    bool isAscii = true;
    for (unsigned char c : content) {
        if (c > 127) {
            isAscii = false;
            break;
        }
    }
    if (isAscii) {
        return Encoding::ASCII;
    }

    return Encoding::UNKNOWN;
}

bool TextStats::isUTF8Valid(const std::string& content) const {
    size_t i = 0;
    while (i < content.size()) {
        unsigned char c = static_cast<unsigned char>(content[i]);

        if (c <= 0x7F) {
            i++;
        } else if ((c & 0xE0) == 0xC0) {
            if (i + 1 >= content.size()) return false;
            if ((static_cast<unsigned char>(content[i+1]) & 0xC0) != 0x80) return false;
            i += 2;
        } else if ((c & 0xF0) == 0xE0) {
            if (i + 2 >= content.size()) return false;
            if ((static_cast<unsigned char>(content[i+1]) & 0xC0) != 0x80) return false;
            if ((static_cast<unsigned char>(content[i+2]) & 0xC0) != 0x80) return false;
            i += 3;
        } else if ((c & 0xF8) == 0xF0) {
            if (i + 3 >= content.size()) return false;
            if ((static_cast<unsigned char>(content[i+1]) & 0xC0) != 0x80) return false;
            if ((static_cast<unsigned char>(content[i+2]) & 0xC0) != 0x80) return false;
            if ((static_cast<unsigned char>(content[i+3]) & 0xC0) != 0x80) return false;
            i += 4;
        } else {
            return false;
        }
    }
    return true;
}

bool TextStats::isChineseChar(uint32_t codepoint) const {
    return (codepoint >= 0x4E00 && codepoint <= 0x9FFF) ||
           (codepoint >= 0x3400 && codepoint <= 0x4DBF) ||
           (codepoint >= 0x20000 && codepoint <= 0x2A6DF) ||
           (codepoint >= 0x2A700 && codepoint <= 0x2B73F) ||
           (codepoint >= 0x2B740 && codepoint <= 0x2B81F) ||
           (codepoint >= 0x2B820 && codepoint <= 0x2CEAF);
}

bool TextStats::isWordChar(uint32_t codepoint) const {
    return (codepoint >= 'A' && codepoint <= 'Z') ||
           (codepoint >= 'a' && codepoint <= 'z') ||
           (codepoint >= '0' && codepoint <= '9') ||
           codepoint == '_' ||
           codepoint == '\'';
}

char TextStats::toLowerChar(uint32_t codepoint) const {
    if (codepoint >= 'A' && codepoint <= 'Z') {
        return static_cast<char>(codepoint - 'A' + 'a');
    }
    return static_cast<char>(codepoint);
}

void TextStats::countAllStats(const std::string& content) {
    if (content.empty()) {
        stats_.line_stats.total_lines = 0;
        stats_.line_stats.non_empty_lines = 0;
        stats_.line_stats.longest_line_length = 0;
        stats_.line_stats.shortest_line_length = 0;
        stats_.line_stats.average_line_length = 0.0;
        stats_.word_stats.total_words = 0;
        stats_.word_stats.unique_words = 0;
        return;
    }

    std::unordered_map<std::string, uint64_t> wordCount;
    std::string currentWord;

    uint64_t charCount = 0;
    uint64_t charCountWithoutSpaces = 0;
    uint64_t charCountWithoutWhitespace = 0;

    bool inLine = false;
    uint64_t currentLineLength = 0;
    uint64_t totalLineLength = 0;
    uint64_t lineCount = 0;

    bool isUTF8 = (stats_.file_info.encoding == Encoding::UTF8);

    size_t i = 0;
    while (i < content.size()) {
        unsigned char c = static_cast<unsigned char>(content[i]);
        size_t charLen = 1;
        uint32_t codepoint = 0;

        if (isUTF8 && c > 0x7F) {
            if ((c & 0xE0) == 0xC0) {
                if (i + 1 >= content.size()) {
                    codepoint = c;
                    charLen = 1;
                } else {
                    codepoint = ((c & 0x1F) << 6) | (static_cast<unsigned char>(content[i+1]) & 0x3F);
                    charLen = 2;
                }
            } else if ((c & 0xF0) == 0xE0) {
                if (i + 2 >= content.size()) {
                    codepoint = c;
                    charLen = 1;
                } else {
                    codepoint = ((c & 0x0F) << 12) |
                                ((static_cast<unsigned char>(content[i+1]) & 0x3F) << 6) |
                                (static_cast<unsigned char>(content[i+2]) & 0x3F);
                    charLen = 3;
                }
            } else if ((c & 0xF8) == 0xF0) {
                if (i + 3 >= content.size()) {
                    codepoint = c;
                    charLen = 1;
                } else {
                    codepoint = ((c & 0x07) << 18) |
                                ((static_cast<unsigned char>(content[i+1]) & 0x3F) << 12) |
                                ((static_cast<unsigned char>(content[i+2]) & 0x3F) << 6) |
                                (static_cast<unsigned char>(content[i+3]) & 0x3F);
                    charLen = 4;
                }
            } else {
                codepoint = c;
                charLen = 1;
            }
        } else {
            codepoint = c;
            charLen = 1;
        }

        charCount++;

        bool isWhitespace = isWhitespaceCodepoint(codepoint);
        bool isSpace = (codepoint == ' ');

        if (!isWhitespace) {
            charCountWithoutWhitespace++;
        }
        if (!isSpace) {
            charCountWithoutSpaces++;
        }

        if (codepoint == ' ') {
            stats_.category_stats.spaces++;
            if (!currentWord.empty()) {
                wordCount[currentWord]++;
                currentWord.clear();
            }
        } else if (codepoint == '\t') {
            stats_.category_stats.tabs++;
            if (!currentWord.empty()) {
                wordCount[currentWord]++;
                currentWord.clear();
            }
        } else if (codepoint == '\n') {
            stats_.category_stats.newlines++;
            stats_.basic_stats.total_lines++;
            if (inLine) {
                stats_.basic_stats.non_empty_lines++;
                inLine = false;
            }

            if (lineCount > 0 || currentLineLength > 0) {
                if (currentLineLength > stats_.line_stats.longest_line_length) {
                    stats_.line_stats.longest_line_length = currentLineLength;
                }
                if (currentLineLength < stats_.line_stats.shortest_line_length) {
                    stats_.line_stats.shortest_line_length = currentLineLength;
                }
                totalLineLength += currentLineLength;
                lineCount++;
            }
            currentLineLength = 0;

            if (!currentWord.empty()) {
                wordCount[currentWord]++;
                currentWord.clear();
            }
        } else if (codepoint == '\r') {
        } else if ((codepoint >= 'A' && codepoint <= 'Z') ||
                   (codepoint >= 'a' && codepoint <= 'z')) {
            stats_.category_stats.letters++;
            if (!inLine) inLine = true;
            currentLineLength++;

            if (isWordChar(codepoint)) {
                currentWord += toLowerChar(codepoint);
            }
        } else if (codepoint >= '0' && codepoint <= '9') {
            stats_.category_stats.digits++;
            if (!inLine) inLine = true;
            currentLineLength++;

            if (isWordChar(codepoint)) {
                currentWord += static_cast<char>(codepoint);
            }
        } else if (isChineseChar(codepoint)) {
            stats_.category_stats.chinese++;
            if (!inLine) inLine = true;
            currentLineLength++;

            if (!currentWord.empty()) {
                wordCount[currentWord]++;
                currentWord.clear();
            }
        } else {
            if (isPunctuationCodepoint(codepoint)) {
                stats_.category_stats.punctuations++;
            }
            if (!inLine && !isWhitespace) inLine = true;
            if (!isWhitespace) {
                currentLineLength++;
            }

            if (isWordChar(codepoint)) {
                currentWord += static_cast<char>(codepoint);
            } else if (!currentWord.empty()) {
                wordCount[currentWord]++;
                currentWord.clear();
            }
        }

        i += charLen;
    }

    if (!currentWord.empty()) {
        wordCount[currentWord]++;
    }

    stats_.basic_stats.total_chars = charCount;
    stats_.basic_stats.chars_without_spaces = charCountWithoutSpaces;
    stats_.basic_stats.chars_without_whitespace = charCountWithoutWhitespace;

    uint32_t lastCodepoint = 0;
    if (!content.empty()) {
        size_t lastPos = content.size() - 1;
        while (lastPos > 0 && isUTF8 && (static_cast<unsigned char>(content[lastPos]) & 0xC0) == 0x80) {
            lastPos--;
        }
        unsigned char lastChar = static_cast<unsigned char>(content[lastPos]);
        lastCodepoint = lastChar;
        if (isUTF8 && lastPos + 1 < content.size() && (lastChar & 0xE0) == 0xC0) {
            lastCodepoint = ((lastChar & 0x1F) << 6) | (static_cast<unsigned char>(content[lastPos+1]) & 0x3F);
        }
    }

    if (!content.empty() && lastCodepoint != '\n') {
        stats_.basic_stats.total_lines++;
        if (inLine) {
            stats_.basic_stats.non_empty_lines++;
        }

        if (currentLineLength > 0 || (lineCount > 0 && lastCodepoint != '\n')) {
            if (currentLineLength > stats_.line_stats.longest_line_length) {
                stats_.line_stats.longest_line_length = currentLineLength;
            }
            if (currentLineLength < stats_.line_stats.shortest_line_length) {
                stats_.line_stats.shortest_line_length = currentLineLength;
            }
            totalLineLength += currentLineLength;
            lineCount++;
        }
    }

    if (lineCount == 0 && !content.empty()) {
        lineCount = 1;
        totalLineLength = charCount;
        stats_.line_stats.longest_line_length = charCount;
        stats_.line_stats.shortest_line_length = charCount;
    }

    if (lineCount > 0) {
        stats_.line_stats.total_lines = stats_.basic_stats.total_lines;
        stats_.line_stats.non_empty_lines = stats_.basic_stats.non_empty_lines;
        stats_.line_stats.average_line_length = static_cast<double>(totalLineLength) / lineCount;
    } else {
        stats_.line_stats.longest_line_length = 0;
        stats_.line_stats.shortest_line_length = 0;
        stats_.line_stats.average_line_length = 0.0;
    }

    calculateTopWords(wordCount);
}

void TextStats::calculateTopWords(const std::unordered_map<std::string, uint64_t>& wordCount) {
    uint64_t totalWords = calculateTotalWords(wordCount);

    stats_.word_stats.total_words = totalWords;
    stats_.word_stats.unique_words = static_cast<uint64_t>(wordCount.size());

    if (wordCount.empty()) {
        return;
    }

    std::vector<std::pair<std::string, uint64_t>> wordsVec(wordCount.begin(), wordCount.end());

    std::sort(wordsVec.begin(), wordsVec.end(),
        [](const std::pair<std::string, uint64_t>& a,
           const std::pair<std::string, uint64_t>& b) {
            if (a.second != b.second) {
                return a.second > b.second;
            }
            return a.first < b.first;
        });

    stats_.word_stats.top_words.clear();
    size_t count = std::min(TOP_WORDS_COUNT, wordsVec.size());
    for (size_t i = 0; i < count; ++i) {
        WordFrequency wf;
        wf.word = wordsVec[i].first;
        wf.count = wordsVec[i].second;
        stats_.word_stats.top_words.push_back(wf);
    }
}

const TextStatistics& TextStats::getStatistics() const {
    return stats_;
}
