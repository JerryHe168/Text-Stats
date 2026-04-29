#ifndef TEXT_STATS_H
#define TEXT_STATS_H

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstddef>
#include <functional>

namespace TextStatsConstants {
    constexpr size_t BOM_UTF8_LENGTH = 3;
    constexpr size_t BOM_UTF16_LENGTH = 2;
    constexpr size_t BOM_UTF32_LENGTH = 4;
    constexpr size_t TOP_WORDS_COUNT = 10;
    constexpr uint64_t DEFAULT_MAX_FILE_SIZE = 100ULL * 1024ULL * 1024ULL;
}

enum class Encoding {
    UNKNOWN,
    ASCII,
    UTF8,
    UTF16_LE,
    UTF16_BE,
    UTF32_LE,
    UTF32_BE
};

struct BasicStats {
    uint64_t total_chars;
    uint64_t chars_without_spaces;
    uint64_t chars_without_whitespace;
    uint64_t total_lines;
    uint64_t non_empty_lines;
};

struct LineStats {
    uint64_t total_lines;
    uint64_t non_empty_lines;
    uint64_t longest_line_length;
    uint64_t shortest_line_length;
    double average_line_length;
};

struct WordFrequency {
    std::string word;
    uint64_t count;
};

struct WordStats {
    uint64_t total_words;
    uint64_t unique_words;
    std::vector<WordFrequency> top_words;
};

struct CategoryStats {
    uint64_t spaces;
    uint64_t punctuations;
    uint64_t letters;
    uint64_t digits;
    uint64_t tabs;
    uint64_t newlines;
    uint64_t chinese;
};

struct FileInfo {
    uint64_t file_size;
    Encoding encoding;
    std::string file_path;
};

struct TextStatistics {
    FileInfo file_info;
    BasicStats basic_stats;
    CategoryStats category_stats;
    LineStats line_stats;
    WordStats word_stats;
};

struct UnicodeChar {
    uint32_t codepoint;
    size_t byte_length;
    bool valid;
};

class TextStats {
public:
    TextStats();
    ~TextStats() = default;

    bool analyzeFile(const std::string& filepath);
#ifdef _WIN32
    bool analyzeFile(const std::wstring& filepath);
#endif

    const TextStatistics& getStatistics() const;
    uint64_t getMaxFileSize() const;
    void setMaxFileSize(uint64_t maxSize);

    void reset();

    static bool isUnicodeLetter(uint32_t codepoint);
    static bool isUnicodeDigit(uint32_t codepoint);
    static bool isUnicodeWhitespace(uint32_t codepoint);
    static bool isUnicodePunctuation(uint32_t codepoint);
    static uint32_t unicodeToLower(uint32_t codepoint);
    static std::string codepointToUTF8(uint32_t codepoint);

    static bool isChineseCharOptimized(uint32_t codepoint);
    static bool isHiraganaOptimized(uint32_t codepoint);
    static bool isKatakanaOptimized(uint32_t codepoint);
    static bool isHangulOptimized(uint32_t codepoint);

private:
    TextStatistics stats_;
    uint64_t max_file_size_;

    Encoding detectEncoding(const std::string& content) const;
    bool isUTF8Valid(const std::string& content) const;

    UnicodeChar decodeUTF8(const std::string& content, size_t offset) const;
    UnicodeChar decodeUTF16LE(const std::string& content, size_t offset) const;
    UnicodeChar decodeUTF16BE(const std::string& content, size_t offset) const;
    UnicodeChar decodeUTF32LE(const std::string& content, size_t offset) const;
    UnicodeChar decodeUTF32BE(const std::string& content, size_t offset) const;
    UnicodeChar decodeChar(const std::string& content, size_t offset, Encoding encoding) const;

    void processAllEncodings(const std::string& content);
    void countStatsFromCodepoint(uint32_t codepoint, 
                                   bool& inLine,
                                   uint64_t& currentLineLength,
                                   uint64_t& totalLineLength,
                                   uint64_t& lineCount,
                                   std::string& currentWord,
                                   std::unordered_map<std::string, uint64_t>& wordCount);

    void calculateTopWords(const std::unordered_map<std::string, uint64_t>& wordCount);
    void finalizeLineStats(uint32_t lastCodepoint,
                           bool inLine,
                           uint64_t currentLineLength,
                           uint64_t totalLineLength,
                           uint64_t lineCount);

    bool analyzeContent(const std::string& filepath, const std::string& content, uint64_t fileSize);
};

#endif
