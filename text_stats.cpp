#include "text_stats.h"
#include "encoding_utils.h"
#include <fstream>
#include <sstream>
#include <cctype>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
#endif

TextStats::TextStats() {
    reset();
}

void TextStats::reset() {
    std::memset(&stats_, 0, sizeof(stats_));
    stats_.file_info.encoding = Encoding::UNKNOWN;
}

#ifdef _WIN32
bool TextStats::analyzeFile(const std::wstring& filepath) {
    reset();
    stats_.file_info.file_path = encoding_utils::wideToUtf8(filepath);

    FILE* file = _wfopen(filepath.c_str(), L"rb");
    if (!file) {
        return false;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (fileSize < 0) {
        fclose(file);
        return false;
    }

    stats_.file_info.file_size = static_cast<uint64_t>(fileSize);

    std::string content(static_cast<size_t>(fileSize), '\0');
    if (fileSize > 0 && fread(&content[0], 1, static_cast<size_t>(fileSize), file) != static_cast<size_t>(fileSize)) {
        fclose(file);
        return false;
    }

    fclose(file);

    stats_.file_info.encoding = detectEncoding(content);

    if (stats_.file_info.encoding == Encoding::UTF8) {
        countUTF8Stats(content);
    } else if (stats_.file_info.encoding == Encoding::ASCII ||
               stats_.file_info.encoding == Encoding::UNKNOWN) {
        countBasicStats(content);
        countCategoryStats(content);
    }

    return true;
}
#endif

bool TextStats::analyzeFile(const std::string& filepath) {
    reset();
    stats_.file_info.file_path = filepath;

#ifdef _WIN32
    std::wstring widePath = encoding_utils::utf8ToWide(filepath);
    FILE* file = _wfopen(widePath.c_str(), L"rb");
    if (!file) {
        return false;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (fileSize < 0) {
        fclose(file);
        return false;
    }

    stats_.file_info.file_size = static_cast<uint64_t>(fileSize);

    std::string content(static_cast<size_t>(fileSize), '\0');
    if (fileSize > 0 && fread(&content[0], 1, static_cast<size_t>(fileSize), file) != static_cast<size_t>(fileSize)) {
        fclose(file);
        return false;
    }

    fclose(file);
#else
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file) {
        return false;
    }

    std::streampos fileSize = file.tellg();
    stats_.file_info.file_size = static_cast<uint64_t>(fileSize);

    file.seekg(0, std::ios::beg);

    std::string content(static_cast<size_t>(fileSize), '\0');
    if (!file.read(&content[0], fileSize)) {
        return false;
    }
#endif

    stats_.file_info.encoding = detectEncoding(content);

    if (stats_.file_info.encoding == Encoding::UTF8) {
        countUTF8Stats(content);
    } else if (stats_.file_info.encoding == Encoding::ASCII ||
               stats_.file_info.encoding == Encoding::UNKNOWN) {
        countBasicStats(content);
        countCategoryStats(content);
    }

    return true;
}

Encoding TextStats::detectEncoding(const std::string& content) {
    if (content.size() < 2) {
        return Encoding::ASCII;
    }

    if (content.size() >= 3 &&
        static_cast<unsigned char>(content[0]) == 0xEF &&
        static_cast<unsigned char>(content[1]) == 0xBB &&
        static_cast<unsigned char>(content[2]) == 0xBF) {
        return Encoding::UTF8;
    }

    if (content.size() >= 2) {
        if (static_cast<unsigned char>(content[0]) == 0xFF &&
            static_cast<unsigned char>(content[1]) == 0xFE) {
            if (content.size() >= 4 && content[2] == 0x00 && content[3] == 0x00) {
                return Encoding::UTF32_LE;
            }
            return Encoding::UTF16_LE;
        }
        if (static_cast<unsigned char>(content[0]) == 0xFE &&
            static_cast<unsigned char>(content[1]) == 0xFF) {
            return Encoding::UTF16_BE;
        }
    }

    if (content.size() >= 4 &&
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

bool TextStats::isUTF8Valid(const std::string& content) {
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

bool TextStats::isChineseChar(uint32_t codepoint) {
    return (codepoint >= 0x4E00 && codepoint <= 0x9FFF) ||
           (codepoint >= 0x3400 && codepoint <= 0x4DBF) ||
           (codepoint >= 0x20000 && codepoint <= 0x2A6DF) ||
           (codepoint >= 0x2A700 && codepoint <= 0x2B73F) ||
           (codepoint >= 0x2B740 && codepoint <= 0x2B81F) ||
           (codepoint >= 0x2B820 && codepoint <= 0x2CEAF);
}

void TextStats::countBasicStats(const std::string& content) {
    stats_.basic_stats.total_chars = content.size();

    for (char c : content) {
        if (!std::isspace(static_cast<unsigned char>(c))) {
            stats_.basic_stats.chars_without_whitespace++;
        }
        if (c != ' ') {
            stats_.basic_stats.chars_without_spaces++;
        }
    }

    bool inLine = false;
    for (char c : content) {
        if (c == '\n') {
            stats_.basic_stats.total_lines++;
            if (inLine) {
                stats_.basic_stats.non_empty_lines++;
                inLine = false;
            }
        } else if (!inLine && !std::isspace(static_cast<unsigned char>(c))) {
            inLine = true;
        }
    }

    if (!content.empty() && content.back() != '\n') {
        stats_.basic_stats.total_lines++;
        if (inLine) {
            stats_.basic_stats.non_empty_lines++;
        }
    }

    if (content.empty()) {
        stats_.basic_stats.total_lines = 0;
        stats_.basic_stats.non_empty_lines = 0;
    }
}

void TextStats::countCategoryStats(const std::string& content) {
    for (char ch : content) {
        unsigned char c = static_cast<unsigned char>(ch);
        if (c == ' ') {
            stats_.category_stats.spaces++;
        } else if (c == '\t') {
            stats_.category_stats.tabs++;
        } else if (c == '\n') {
            stats_.category_stats.newlines++;
        } else if (std::isalpha(c)) {
            stats_.category_stats.letters++;
        } else if (std::isdigit(c)) {
            stats_.category_stats.digits++;
        } else if (std::ispunct(c)) {
            stats_.category_stats.punctuations++;
        }
    }
}

void TextStats::countUTF8Stats(const std::string& content) {
    size_t charCount = 0;
    size_t charCountWithoutSpaces = 0;
    size_t charCountWithoutWhitespace = 0;
    bool inLine = false;

    size_t i = 0;
    while (i < content.size()) {
        unsigned char c = static_cast<unsigned char>(content[i]);
        size_t charLen = 1;
        uint32_t codepoint = 0;

        if (c <= 0x7F) {
            codepoint = c;
            charLen = 1;
        } else if ((c & 0xE0) == 0xC0) {
            if (i + 1 >= content.size()) break;
            codepoint = ((c & 0x1F) << 6) | (static_cast<unsigned char>(content[i+1]) & 0x3F);
            charLen = 2;
        } else if ((c & 0xF0) == 0xE0) {
            if (i + 2 >= content.size()) break;
            codepoint = ((c & 0x0F) << 12) |
                        ((static_cast<unsigned char>(content[i+1]) & 0x3F) << 6) |
                        (static_cast<unsigned char>(content[i+2]) & 0x3F);
            charLen = 3;
        } else if ((c & 0xF8) == 0xF0) {
            if (i + 3 >= content.size()) break;
            codepoint = ((c & 0x07) << 18) |
                        ((static_cast<unsigned char>(content[i+1]) & 0x3F) << 12) |
                        ((static_cast<unsigned char>(content[i+2]) & 0x3F) << 6) |
                        (static_cast<unsigned char>(content[i+3]) & 0x3F);
            charLen = 4;
        } else {
            codepoint = c;
            charLen = 1;
        }

        charCount++;

        bool isWhitespace = (codepoint == ' ' || codepoint == '\t' || codepoint == '\n' ||
                             codepoint == '\r' || codepoint == '\f' || codepoint == '\v');
        bool isSpace = (codepoint == ' ');

        if (!isWhitespace) {
            charCountWithoutWhitespace++;
        }
        if (!isSpace) {
            charCountWithoutSpaces++;
        }

        if (codepoint == ' ') {
            stats_.category_stats.spaces++;
        } else if (codepoint == '\t') {
            stats_.category_stats.tabs++;
        } else if (codepoint == '\n') {
            stats_.category_stats.newlines++;
            stats_.basic_stats.total_lines++;
            if (inLine) {
                stats_.basic_stats.non_empty_lines++;
                inLine = false;
            }
        } else if ((codepoint >= 'A' && codepoint <= 'Z') ||
                   (codepoint >= 'a' && codepoint <= 'z')) {
            stats_.category_stats.letters++;
            if (!inLine) inLine = true;
        } else if (codepoint >= '0' && codepoint <= '9') {
            stats_.category_stats.digits++;
            if (!inLine) inLine = true;
        } else if (isChineseChar(codepoint)) {
            stats_.category_stats.chinese++;
            if (!inLine) inLine = true;
        } else {
            if ((codepoint >= 0x21 && codepoint <= 0x2F) ||
                (codepoint >= 0x3A && codepoint <= 0x40) ||
                (codepoint >= 0x5B && codepoint <= 0x60) ||
                (codepoint >= 0x7B && codepoint <= 0x7E)) {
                stats_.category_stats.punctuations++;
            }
            if (!inLine && !isWhitespace) inLine = true;
        }

        if (codepoint == '\n') {
            stats_.basic_stats.total_lines++;
            if (inLine) {
                stats_.basic_stats.non_empty_lines++;
                inLine = false;
            }
        }

        i += charLen;
    }

    stats_.basic_stats.total_chars = charCount;
    stats_.basic_stats.chars_without_spaces = charCountWithoutSpaces;
    stats_.basic_stats.chars_without_whitespace = charCountWithoutWhitespace;

    if (!content.empty()) {
        if (i > 0) {
            size_t lastPos = i - 1;
            while (lastPos > 0 && (static_cast<unsigned char>(content[lastPos]) & 0xC0) == 0x80) {
                lastPos--;
            }
            unsigned char lastChar = static_cast<unsigned char>(content[lastPos]);
            uint32_t lastCodepoint = lastChar;
            if (lastPos + 1 < content.size() && (lastChar & 0xE0) == 0xC0) {
                lastCodepoint = ((lastChar & 0x1F) << 6) | (static_cast<unsigned char>(content[lastPos+1]) & 0x3F);
            }
            if (lastCodepoint != '\n') {
                stats_.basic_stats.total_lines++;
                if (inLine) {
                    stats_.basic_stats.non_empty_lines++;
                }
            }
        }
    }

    if (content.empty()) {
        stats_.basic_stats.total_lines = 0;
        stats_.basic_stats.non_empty_lines = 0;
    }
}

const TextStatistics& TextStats::getStatistics() const {
    return stats_;
}
