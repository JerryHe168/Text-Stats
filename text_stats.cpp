#include "text_stats.h"
#include "encoding_utils.h"
#include <fstream>
#include <sstream>
#include <cctype>
#include <cstring>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
#endif

using namespace TextStatsConstants;

namespace {
    struct UnicodeRange {
        uint32_t start;
        uint32_t end;
    };

    const UnicodeRange UNICODE_LETTER_RANGES[] = {
        {0x0041, 0x005A},
        {0x0061, 0x007A},
        {0x00C0, 0x00D6},
        {0x00D8, 0x00F6},
        {0x00F8, 0x02B8},
        {0x0370, 0x03FF},
        {0x0400, 0x04FF},
        {0x0500, 0x052F},
        {0x0530, 0x058F},
        {0x0590, 0x05FF},
        {0x0600, 0x06FF},
        {0x0700, 0x074F},
        {0x0750, 0x077F},
        {0x0780, 0x07BF},
        {0x07C0, 0x07FF},
        {0x0900, 0x097F},
        {0x0980, 0x09FF},
        {0x0A00, 0x0A7F},
        {0x0A80, 0x0AFF},
        {0x0B00, 0x0B7F},
        {0x0B80, 0x0BFF},
        {0x0C00, 0x0C7F},
        {0x0C80, 0x0CFF},
        {0x0D00, 0x0D7F},
        {0x0D80, 0x0DFF},
        {0x0E00, 0x0E7F},
        {0x0E80, 0x0EFF},
        {0x0F00, 0x0FFF},
        {0x1040, 0x1049},
        {0x1090, 0x1099},
        {0x1100, 0x11FF},
        {0x17E0, 0x17E9},
        {0x1810, 0x1819},
        {0x1946, 0x194F},
        {0x19D0, 0x19D9},
        {0x1A80, 0x1A89},
        {0x1A90, 0x1A99},
        {0x1B50, 0x1B59},
        {0x1BB0, 0x1BB9},
        {0x1C40, 0x1C49},
        {0x1C50, 0x1C59},
        {0x1E00, 0x1EFF},
        {0x2F800, 0x2FA1F},
        {0x3041, 0x309F},
        {0x30A1, 0x30FF},
        {0x3130, 0x318F},
        {0x31F0, 0x31FF},
        {0x3400, 0x4DBF},
        {0x4E00, 0x9FFF},
        {0xA620, 0xA629},
        {0xA8D0, 0xA8D9},
        {0xA900, 0xA909},
        {0xA960, 0xA97F},
        {0xA9D0, 0xA9D9},
        {0xAA50, 0xAA59},
        {0xABF0, 0xABF9},
        {0xAC00, 0xD7A3},
        {0xD7B0, 0xD7FF},
        {0xF900, 0xFAFF},
        {0x10000, 0x1FFFD},
        {0x20000, 0x2FFFD},
        {0x30000, 0x3FFFD},
    };
    constexpr size_t UNICODE_LETTER_RANGE_COUNT = sizeof(UNICODE_LETTER_RANGES) / sizeof(UNICODE_LETTER_RANGES[0]);

    const UnicodeRange UNICODE_DIGIT_RANGES[] = {
        {0x0030, 0x0039},
        {0x0660, 0x0669},
        {0x06F0, 0x06F9},
        {0x07C0, 0x07C9},
        {0x0966, 0x096F},
        {0x09E6, 0x09EF},
        {0x0A66, 0x0A6F},
        {0x0AE6, 0x0AEF},
        {0x0B66, 0x0B6F},
        {0x0BE6, 0x0BEF},
        {0x0C66, 0x0C6F},
        {0x0CE6, 0x0CEF},
        {0x0D66, 0x0D6F},
        {0x0E50, 0x0E59},
        {0x0ED0, 0x0ED9},
        {0x0F20, 0x0F29},
        {0x1040, 0x1049},
        {0x1090, 0x1099},
        {0x17E0, 0x17E9},
        {0x1810, 0x1819},
        {0x1946, 0x194F},
        {0x19D0, 0x19D9},
        {0x1A80, 0x1A89},
        {0x1A90, 0x1A99},
        {0x1B50, 0x1B59},
        {0x1BB0, 0x1BB9},
        {0x1C40, 0x1C49},
        {0x1C50, 0x1C59},
        {0xA620, 0xA629},
        {0xA8D0, 0xA8D9},
        {0xA900, 0xA909},
        {0xA9D0, 0xA9D9},
        {0xAA50, 0xAA59},
        {0xABF0, 0xABF9},
        {0xFF10, 0xFF19},
    };
    constexpr size_t UNICODE_DIGIT_RANGE_COUNT = sizeof(UNICODE_DIGIT_RANGES) / sizeof(UNICODE_DIGIT_RANGES[0]);

    const UnicodeRange UNICODE_PUNCTUATION_RANGES[] = {
        {0x0021, 0x002F},
        {0x003A, 0x0040},
        {0x005B, 0x0060},
        {0x007B, 0x007E},
        {0x00A1, 0x00A1},
        {0x00AB, 0x00BB},
        {0x00BF, 0x00BF},
        {0x2010, 0x2027},
        {0x2018, 0x2019},
        {0x201C, 0x201D},
        {0x2030, 0x204E},
        {0x3001, 0x3003},
        {0x3008, 0x3011},
        {0x3014, 0x301F},
        {0xFF01, 0xFF0F},
        {0xFF1A, 0xFF20},
        {0xFF3B, 0xFF40},
        {0xFF5B, 0xFF65},
    };
    constexpr size_t UNICODE_PUNCTUATION_RANGE_COUNT = sizeof(UNICODE_PUNCTUATION_RANGES) / sizeof(UNICODE_PUNCTUATION_RANGES[0]);

    const UnicodeRange UNICODE_WHITESPACE_RANGES[] = {
        {0x0009, 0x000D},
        {0x0020, 0x0020},
        {0x00A0, 0x00A0},
        {0x1680, 0x1680},
        {0x2000, 0x200A},
        {0x2028, 0x2029},
        {0x202F, 0x202F},
        {0x205F, 0x205F},
        {0x3000, 0x3000},
    };
    constexpr size_t UNICODE_WHITESPACE_RANGE_COUNT = sizeof(UNICODE_WHITESPACE_RANGES) / sizeof(UNICODE_WHITESPACE_RANGES[0]);

    bool binarySearchRange(uint32_t codepoint, const UnicodeRange* ranges, size_t rangeCount) {
        if (rangeCount == 0) {
            return false;
        }

        int low = 0;
        int high = static_cast<int>(rangeCount) - 1;

        while (low <= high) {
            int mid = low + (high - low) / 2;
            const UnicodeRange& range = ranges[mid];

            if (codepoint < range.start) {
                high = mid - 1;
            } else if (codepoint > range.end) {
                low = mid + 1;
            } else {
                return true;
            }
        }

        return false;
    }

    uint64_t calculateTotalWords(const std::unordered_map<std::string, uint64_t>& wordCount) {
        uint64_t total = 0;
        for (const auto& pair : wordCount) {
            total += pair.second;
        }
        return total;
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

#ifdef _WIN32
namespace {
    struct FileReadResult {
        std::string content;
        uint64_t fileSize;
        bool success;
    };

    FileReadResult readFileContent(const std::wstring& filepath, uint64_t maxFileSize) {
        FileReadResult result;
        result.fileSize = 0;
        result.success = false;

        FILE* file = _wfopen(filepath.c_str(), L"rb");
        if (!file) {
            return result;
        }

        fseek(file, 0, SEEK_END);
        long fileSizeLong = ftell(file);
        fseek(file, 0, SEEK_SET);

        if (fileSizeLong < 0) {
            fclose(file);
            return result;
        }

        uint64_t fileSize = static_cast<uint64_t>(fileSizeLong);
        result.fileSize = fileSize;

        if (fileSize > maxFileSize) {
            fclose(file);
            return result;
        }

        if (fileSize > 0) {
            result.content.resize(static_cast<size_t>(fileSize), '\0');
            if (fread(&result.content[0], 1, static_cast<size_t>(fileSize), file) != static_cast<size_t>(fileSize)) {
                fclose(file);
                result.content.clear();
                return result;
            }
        }

        fclose(file);
        result.success = true;
        return result;
    }
}
#endif

bool TextStats::analyzeContent(const std::string& filepath, const std::string& content, uint64_t fileSize) {
    stats_.file_info.file_path = filepath;
    stats_.file_info.file_size = fileSize;
    stats_.file_info.encoding = detectEncoding(content);

    processAllEncodings(content);

    return true;
}

#ifdef _WIN32
bool TextStats::analyzeFile(const std::wstring& filepath) {
    reset();

    FileReadResult result = readFileContent(filepath, max_file_size_);
    if (!result.success) {
        return false;
    }

    return analyzeContent(encoding_utils::wideToUtf8(filepath), result.content, result.fileSize);
}
#endif

bool TextStats::analyzeFile(const std::string& filepath) {
    reset();

    std::string content;
    uint64_t fileSize = 0;

#ifdef _WIN32
    std::wstring widePath = encoding_utils::utf8ToWide(filepath);
    FileReadResult result = readFileContent(widePath, max_file_size_);
    if (!result.success) {
        return false;
    }
    content = result.content;
    fileSize = result.fileSize;
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

namespace {
    bool hasBOM_UTF8(const std::string& content) {
        if (content.size() < BOM_UTF8_LENGTH) {
            return false;
        }
        return static_cast<unsigned char>(content[0]) == 0xEF &&
               static_cast<unsigned char>(content[1]) == 0xBB &&
               static_cast<unsigned char>(content[2]) == 0xBF;
    }

    bool hasBOM_UTF16LE(const std::string& content) {
        if (content.size() < BOM_UTF16_LENGTH) {
            return false;
        }
        if (content.size() >= BOM_UTF32_LENGTH &&
            static_cast<unsigned char>(content[2]) == 0x00 &&
            static_cast<unsigned char>(content[3]) == 0x00) {
            return false;
        }
        return static_cast<unsigned char>(content[0]) == 0xFF &&
               static_cast<unsigned char>(content[1]) == 0xFE;
    }

    bool hasBOM_UTF16BE(const std::string& content) {
        if (content.size() < BOM_UTF16_LENGTH) {
            return false;
        }
        return static_cast<unsigned char>(content[0]) == 0xFE &&
               static_cast<unsigned char>(content[1]) == 0xFF;
    }

    bool hasBOM_UTF32LE(const std::string& content) {
        if (content.size() < BOM_UTF32_LENGTH) {
            return false;
        }
        return static_cast<unsigned char>(content[0]) == 0xFF &&
               static_cast<unsigned char>(content[1]) == 0xFE &&
               static_cast<unsigned char>(content[2]) == 0x00 &&
               static_cast<unsigned char>(content[3]) == 0x00;
    }

    bool hasBOM_UTF32BE(const std::string& content) {
        if (content.size() < BOM_UTF32_LENGTH) {
            return false;
        }
        return static_cast<unsigned char>(content[0]) == 0x00 &&
               static_cast<unsigned char>(content[1]) == 0x00 &&
               static_cast<unsigned char>(content[2]) == 0xFE &&
               static_cast<unsigned char>(content[3]) == 0xFF;
    }

    size_t getBOMLength(Encoding encoding) {
        switch (encoding) {
            case Encoding::UTF8:
                return BOM_UTF8_LENGTH;
            case Encoding::UTF16_LE:
            case Encoding::UTF16_BE:
                return BOM_UTF16_LENGTH;
            case Encoding::UTF32_LE:
            case Encoding::UTF32_BE:
                return BOM_UTF32_LENGTH;
            default:
                return 0;
        }
    }
}

Encoding TextStats::detectEncoding(const std::string& content) const {
    if (content.size() < BOM_UTF16_LENGTH) {
        return Encoding::ASCII;
    }

    if (hasBOM_UTF32LE(content)) {
        return Encoding::UTF32_LE;
    }
    if (hasBOM_UTF32BE(content)) {
        return Encoding::UTF32_BE;
    }
    if (hasBOM_UTF16LE(content)) {
        return Encoding::UTF16_LE;
    }
    if (hasBOM_UTF16BE(content)) {
        return Encoding::UTF16_BE;
    }
    if (hasBOM_UTF8(content)) {
        return Encoding::UTF8;
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

bool TextStats::isChineseCharOptimized(uint32_t codepoint) {
    return (codepoint >= 0x4E00 && codepoint <= 0x9FFF) ||
           (codepoint >= 0x3400 && codepoint <= 0x4DBF) ||
           (codepoint >= 0x20000 && codepoint <= 0x2A6DF) ||
           (codepoint >= 0x2A700 && codepoint <= 0x2B73F) ||
           (codepoint >= 0x2B740 && codepoint <= 0x2B81F) ||
           (codepoint >= 0x2B820 && codepoint <= 0x2CEAF) ||
           (codepoint >= 0xF900 && codepoint <= 0xFAFF) ||
           (codepoint >= 0x2F800 && codepoint <= 0x2FA1F);
}

bool TextStats::isHiraganaOptimized(uint32_t codepoint) {
    return (codepoint >= 0x3041 && codepoint <= 0x309F);
}

bool TextStats::isKatakanaOptimized(uint32_t codepoint) {
    return (codepoint >= 0x30A1 && codepoint <= 0x30FF) ||
           (codepoint >= 0x31F0 && codepoint <= 0x31FF);
}

bool TextStats::isHangulOptimized(uint32_t codepoint) {
    return (codepoint >= 0xAC00 && codepoint <= 0xD7A3) ||
           (codepoint >= 0x1100 && codepoint <= 0x11FF) ||
           (codepoint >= 0x3130 && codepoint <= 0x318F) ||
           (codepoint >= 0xA960 && codepoint <= 0xA97F) ||
           (codepoint >= 0xD7B0 && codepoint <= 0xD7FF);
}

bool TextStats::isUnicodeLetter(uint32_t codepoint) {
    if (codepoint <= 0x007F) {
        return (codepoint >= 'A' && codepoint <= 'Z') ||
               (codepoint >= 'a' && codepoint <= 'z');
    }

    if (codepoint >= 0x4E00 && codepoint <= 0x9FFF) {
        return true;
    }

    if (codepoint >= 0x3040 && codepoint <= 0x30FF) {
        return true;
    }

    if (codepoint >= 0xAC00 && codepoint <= 0xD7A3) {
        return true;
    }

    return binarySearchRange(codepoint, UNICODE_LETTER_RANGES, UNICODE_LETTER_RANGE_COUNT);
}

bool TextStats::isUnicodeDigit(uint32_t codepoint) {
    if (codepoint >= '0' && codepoint <= '9') {
        return true;
    }

    if (codepoint >= 0xFF10 && codepoint <= 0xFF19) {
        return true;
    }

    return binarySearchRange(codepoint, UNICODE_DIGIT_RANGES, UNICODE_DIGIT_RANGE_COUNT);
}

bool TextStats::isUnicodeWhitespace(uint32_t codepoint) {
    if (codepoint <= 0x0020) {
        return codepoint == ' ' ||
               codepoint == '\t' ||
               codepoint == '\n' ||
               codepoint == '\r' ||
               codepoint == '\f' ||
               codepoint == '\v';
    }

    return binarySearchRange(codepoint, UNICODE_WHITESPACE_RANGES, UNICODE_WHITESPACE_RANGE_COUNT);
}

bool TextStats::isUnicodePunctuation(uint32_t codepoint) {
    if (codepoint <= 0x007E) {
        return (codepoint >= 0x21 && codepoint <= 0x2F) ||
               (codepoint >= 0x3A && codepoint <= 0x40) ||
               (codepoint >= 0x5B && codepoint <= 0x60) ||
               (codepoint >= 0x7B && codepoint <= 0x7E);
    }

    return binarySearchRange(codepoint, UNICODE_PUNCTUATION_RANGES, UNICODE_PUNCTUATION_RANGE_COUNT);
}

uint32_t TextStats::unicodeToLower(uint32_t codepoint) {
    if (codepoint >= 'A' && codepoint <= 'Z') {
        return codepoint - 'A' + 'a';
    }

    if (codepoint >= 0xC0 && codepoint <= 0xD6) {
        return codepoint + 0x20;
    }
    if (codepoint >= 0xD8 && codepoint <= 0xDE) {
        return codepoint + 0x20;
    }

    if (codepoint == 0x0178) return 0x00FF;
    if (codepoint == 0x01F8) return 0x01F9;
    if (codepoint == 0x01FA) return 0x01FB;
    if (codepoint == 0x01FC) return 0x01FD;
    if (codepoint == 0x01FE) return 0x01FF;

    return codepoint;
}

std::string TextStats::codepointToUTF8(uint32_t codepoint) {
    std::string result;

    if (codepoint <= 0x7F) {
        result += static_cast<char>(codepoint);
    } else if (codepoint <= 0x7FF) {
        result += static_cast<char>(0xC0 | (codepoint >> 6));
        result += static_cast<char>(0x80 | (codepoint & 0x3F));
    } else if (codepoint <= 0xFFFF) {
        result += static_cast<char>(0xE0 | (codepoint >> 12));
        result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (codepoint & 0x3F));
    } else if (codepoint <= 0x10FFFF) {
        result += static_cast<char>(0xF0 | (codepoint >> 18));
        result += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
        result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (codepoint & 0x3F));
    }

    return result;
}

UnicodeChar TextStats::decodeUTF8(const std::string& content, size_t offset) const {
    UnicodeChar result = {0, 1, false};

    if (offset >= content.size()) {
        return result;
    }

    unsigned char c = static_cast<unsigned char>(content[offset]);

    if (c <= 0x7F) {
        result.codepoint = c;
        result.byte_length = 1;
        result.valid = true;
    } else if ((c & 0xE0) == 0xC0) {
        if (offset + 1 >= content.size()) {
            result.codepoint = c;
            result.byte_length = 1;
            result.valid = false;
            return result;
        }
        unsigned char c2 = static_cast<unsigned char>(content[offset + 1]);
        if ((c2 & 0xC0) != 0x80) {
            result.codepoint = c;
            result.byte_length = 1;
            result.valid = false;
            return result;
        }
        result.codepoint = ((c & 0x1F) << 6) | (c2 & 0x3F);
        result.byte_length = 2;
        result.valid = true;
    } else if ((c & 0xF0) == 0xE0) {
        if (offset + 2 >= content.size()) {
            result.codepoint = c;
            result.byte_length = 1;
            result.valid = false;
            return result;
        }
        unsigned char c2 = static_cast<unsigned char>(content[offset + 1]);
        unsigned char c3 = static_cast<unsigned char>(content[offset + 2]);
        if ((c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80) {
            result.codepoint = c;
            result.byte_length = 1;
            result.valid = false;
            return result;
        }
        result.codepoint = ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
        result.byte_length = 3;
        result.valid = true;
    } else if ((c & 0xF8) == 0xF0) {
        if (offset + 3 >= content.size()) {
            result.codepoint = c;
            result.byte_length = 1;
            result.valid = false;
            return result;
        }
        unsigned char c2 = static_cast<unsigned char>(content[offset + 1]);
        unsigned char c3 = static_cast<unsigned char>(content[offset + 2]);
        unsigned char c4 = static_cast<unsigned char>(content[offset + 3]);
        if ((c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80 || (c4 & 0xC0) != 0x80) {
            result.codepoint = c;
            result.byte_length = 1;
            result.valid = false;
            return result;
        }
        result.codepoint = ((c & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
        result.byte_length = 4;
        result.valid = true;
    } else {
        result.codepoint = c;
        result.byte_length = 1;
        result.valid = false;
    }

    return result;
}

UnicodeChar TextStats::decodeUTF16LE(const std::string& content, size_t offset) const {
    UnicodeChar result = {0, 2, false};

    if (offset + 1 >= content.size()) {
        if (offset < content.size()) {
            result.codepoint = static_cast<unsigned char>(content[offset]);
            result.byte_length = 1;
        }
        return result;
    }

    uint16_t w1 = static_cast<unsigned char>(content[offset]) | 
                  (static_cast<unsigned char>(content[offset + 1]) << 8);

    if (w1 >= 0xD800 && w1 <= 0xDBFF) {
        if (offset + 3 >= content.size()) {
            result.codepoint = w1;
            result.byte_length = 2;
            result.valid = false;
            return result;
        }
        uint16_t w2 = static_cast<unsigned char>(content[offset + 2]) | 
                      (static_cast<unsigned char>(content[offset + 3]) << 8);
        if (w2 >= 0xDC00 && w2 <= 0xDFFF) {
            result.codepoint = 0x10000 + ((w1 - 0xD800) << 10) + (w2 - 0xDC00);
            result.byte_length = 4;
            result.valid = true;
        } else {
            result.codepoint = w1;
            result.byte_length = 2;
            result.valid = false;
        }
    } else if (w1 >= 0xDC00 && w1 <= 0xDFFF) {
        result.codepoint = w1;
        result.byte_length = 2;
        result.valid = false;
    } else {
        result.codepoint = w1;
        result.byte_length = 2;
        result.valid = true;
    }

    return result;
}

UnicodeChar TextStats::decodeUTF16BE(const std::string& content, size_t offset) const {
    UnicodeChar result = {0, 2, false};

    if (offset + 1 >= content.size()) {
        if (offset < content.size()) {
            result.codepoint = static_cast<unsigned char>(content[offset]);
            result.byte_length = 1;
        }
        return result;
    }

    uint16_t w1 = (static_cast<unsigned char>(content[offset]) << 8) | 
                  static_cast<unsigned char>(content[offset + 1]);

    if (w1 >= 0xD800 && w1 <= 0xDBFF) {
        if (offset + 3 >= content.size()) {
            result.codepoint = w1;
            result.byte_length = 2;
            result.valid = false;
            return result;
        }
        uint16_t w2 = (static_cast<unsigned char>(content[offset + 2]) << 8) | 
                      static_cast<unsigned char>(content[offset + 3]);
        if (w2 >= 0xDC00 && w2 <= 0xDFFF) {
            result.codepoint = 0x10000 + ((w1 - 0xD800) << 10) + (w2 - 0xDC00);
            result.byte_length = 4;
            result.valid = true;
        } else {
            result.codepoint = w1;
            result.byte_length = 2;
            result.valid = false;
        }
    } else if (w1 >= 0xDC00 && w1 <= 0xDFFF) {
        result.codepoint = w1;
        result.byte_length = 2;
        result.valid = false;
    } else {
        result.codepoint = w1;
        result.byte_length = 2;
        result.valid = true;
    }

    return result;
}

UnicodeChar TextStats::decodeUTF32LE(const std::string& content, size_t offset) const {
    UnicodeChar result = {0, 4, false};

    if (offset + 3 >= content.size()) {
        if (offset < content.size()) {
            result.codepoint = static_cast<unsigned char>(content[offset]);
            result.byte_length = content.size() - offset;
        }
        return result;
    }

    result.codepoint = static_cast<unsigned char>(content[offset]) |
                       (static_cast<unsigned char>(content[offset + 1]) << 8) |
                       (static_cast<unsigned char>(content[offset + 2]) << 16) |
                       (static_cast<unsigned char>(content[offset + 3]) << 24);

    result.byte_length = 4;
    result.valid = (result.codepoint <= 0x10FFFF);

    return result;
}

UnicodeChar TextStats::decodeUTF32BE(const std::string& content, size_t offset) const {
    UnicodeChar result = {0, 4, false};

    if (offset + 3 >= content.size()) {
        if (offset < content.size()) {
            result.codepoint = static_cast<unsigned char>(content[offset]);
            result.byte_length = content.size() - offset;
        }
        return result;
    }

    result.codepoint = (static_cast<unsigned char>(content[offset]) << 24) |
                       (static_cast<unsigned char>(content[offset + 1]) << 16) |
                       (static_cast<unsigned char>(content[offset + 2]) << 8) |
                       static_cast<unsigned char>(content[offset + 3]);

    result.byte_length = 4;
    result.valid = (result.codepoint <= 0x10FFFF);

    return result;
}

UnicodeChar TextStats::decodeChar(const std::string& content, size_t offset, Encoding encoding) const {
    switch (encoding) {
        case Encoding::UTF8:
            return decodeUTF8(content, offset);
        case Encoding::UTF16_LE:
            return decodeUTF16LE(content, offset);
        case Encoding::UTF16_BE:
            return decodeUTF16BE(content, offset);
        case Encoding::UTF32_LE:
            return decodeUTF32LE(content, offset);
        case Encoding::UTF32_BE:
            return decodeUTF32BE(content, offset);
        case Encoding::ASCII:
        case Encoding::UNKNOWN:
        default:
            UnicodeChar result = {0, 1, true};
            if (offset < content.size()) {
                result.codepoint = static_cast<unsigned char>(content[offset]);
                result.valid = (result.codepoint <= 0x7F);
            }
            return result;
    }
}

void TextStats::countStatsFromCodepoint(uint32_t codepoint,
                                          bool& inLine,
                                          uint64_t& currentLineLength,
                                          uint64_t& totalLineLength,
                                          uint64_t& lineCount,
                                          std::string& currentWord,
                                          std::unordered_map<std::string, uint64_t>& wordCount) {
    stats_.basic_stats.total_chars++;

    bool isWhitespace = isUnicodeWhitespace(codepoint);
    bool isSpace = (codepoint == ' ');

    if (!isWhitespace) {
        stats_.basic_stats.chars_without_whitespace++;
    }
    if (!isSpace) {
        stats_.basic_stats.chars_without_spaces++;
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
    } else if (isUnicodeLetter(codepoint)) {
        stats_.category_stats.letters++;
        if (!inLine) inLine = true;
        currentLineLength++;

        if (isChineseCharOptimized(codepoint)) {
            stats_.category_stats.chinese++;
            if (!currentWord.empty()) {
                wordCount[currentWord]++;
                currentWord.clear();
            }
            std::string charStr = codepointToUTF8(codepoint);
            wordCount[charStr]++;
        } else {
            currentWord += codepointToUTF8(unicodeToLower(codepoint));
        }
    } else if (isUnicodeDigit(codepoint)) {
        stats_.category_stats.digits++;
        if (!inLine) inLine = true;
        currentLineLength++;

        currentWord += codepointToUTF8(codepoint);
    } else {
        if (isUnicodePunctuation(codepoint)) {
            stats_.category_stats.punctuations++;
        }
        if (!inLine && !isWhitespace) inLine = true;
        if (!isWhitespace) {
            currentLineLength++;
        }

        if (codepoint == '_' || codepoint == '\'') {
            currentWord += codepointToUTF8(codepoint);
        } else if (!currentWord.empty()) {
            wordCount[currentWord]++;
            currentWord.clear();
        }
    }
}

void TextStats::finalizeLineStats(uint32_t lastCodepoint,
                                    bool inLine,
                                    uint64_t currentLineLength,
                                    uint64_t totalLineLength,
                                    uint64_t lineCount) {
    if (lastCodepoint != '\n') {
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

    uint64_t totalChars = stats_.basic_stats.total_chars;
    if (lineCount == 0 && totalChars > 0) {
        lineCount = 1;
        totalLineLength = totalChars;
        stats_.line_stats.longest_line_length = totalChars;
        stats_.line_stats.shortest_line_length = totalChars;
    }

    stats_.line_stats.total_lines = stats_.basic_stats.total_lines;
    stats_.line_stats.non_empty_lines = stats_.basic_stats.non_empty_lines;

    if (lineCount > 0) {
        stats_.line_stats.average_line_length = static_cast<double>(totalLineLength) / lineCount;
    } else {
        stats_.line_stats.longest_line_length = 0;
        stats_.line_stats.shortest_line_length = 0;
        stats_.line_stats.average_line_length = 0.0;
    }
}

void TextStats::processAllEncodings(const std::string& content) {
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

    bool inLine = false;
    uint64_t currentLineLength = 0;
    uint64_t totalLineLength = 0;
    uint64_t lineCount = 0;

    size_t offset = 0;
    uint32_t lastCodepoint = 0;
    Encoding encoding = stats_.file_info.encoding;

    if (encoding == Encoding::UTF8 && hasBOM_UTF8(content)) {
        offset = BOM_UTF8_LENGTH;
    } else if ((encoding == Encoding::UTF16_LE && hasBOM_UTF16LE(content)) ||
               (encoding == Encoding::UTF16_BE && hasBOM_UTF16BE(content))) {
        offset = BOM_UTF16_LENGTH;
    } else if ((encoding == Encoding::UTF32_LE && hasBOM_UTF32LE(content)) ||
               (encoding == Encoding::UTF32_BE && hasBOM_UTF32BE(content))) {
        offset = BOM_UTF32_LENGTH;
    }

    while (offset < content.size()) {
        UnicodeChar uc = decodeChar(content, offset, encoding);

        if (!uc.valid && uc.byte_length == 0) {
            break;
        }

        lastCodepoint = uc.codepoint;

        countStatsFromCodepoint(uc.codepoint, inLine, currentLineLength, 
                                 totalLineLength, lineCount, currentWord, wordCount);

        offset += uc.byte_length;
    }

    if (!currentWord.empty()) {
        wordCount[currentWord]++;
    }

    finalizeLineStats(lastCodepoint, inLine, currentLineLength, 
                       totalLineLength, lineCount);

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
