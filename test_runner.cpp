#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

#include "text_stats.h"
#include "output_formatter.h"

#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR "../tests"
#endif

#define TEST_FILE(name) (std::string(TEST_DATA_DIR) + "/" + name)

class TestRunner {
public:
    TestRunner() : passed(0), failed(0) {}

    void runTest(const std::string& testName, bool (*testFunc)()) {
        std::cout << "[" << (testIndex + 1) << "] Running: " << testName << "... ";
        testIndex++;

        try {
            bool result = testFunc();
            if (result) {
                std::cout << "PASSED" << std::endl;
                passed++;
            } else {
                std::cout << "FAILED" << std::endl;
                failed++;
            }
        } catch (...) {
            std::cout << "EXCEPTION" << std::endl;
            failed++;
        }
    }

    void printSummary() {
        std::cout << "\n========== Test Summary ==========" << std::endl;
        std::cout << "Total:  " << (passed + failed) << std::endl;
        std::cout << "Passed: " << passed << std::endl;
        std::cout << "Failed: " << failed << std::endl;
        std::cout << "==================================" << std::endl;
    }

    int getFailedCount() const { return failed; }

private:
    int passed;
    int failed;
    int testIndex = 0;
};

static bool testEmptyFile() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("empty.txt"));
    if (!result) return false;

    const TextStatistics& s = stats.getStatistics();
    if (s.file_info.file_size != 0) return false;
    if (s.basic_stats.total_chars != 0) return false;
    if (s.basic_stats.chars_without_spaces != 0) return false;
    if (s.basic_stats.chars_without_whitespace != 0) return false;
    if (s.basic_stats.total_lines != 0) return false;
    if (s.basic_stats.non_empty_lines != 0) return false;

    if (s.category_stats.spaces != 0) return false;
    if (s.category_stats.tabs != 0) return false;
    if (s.category_stats.newlines != 0) return false;
    if (s.category_stats.letters != 0) return false;
    if (s.category_stats.digits != 0) return false;
    if (s.category_stats.punctuations != 0) return false;
    if (s.category_stats.chinese != 0) return false;

    return true;
}

static bool testSingleLineNoNewline() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("single_line.txt"));
    if (!result) return false;

    const TextStatistics& s = stats.getStatistics();
    if (s.basic_stats.total_chars != 10) return false;
    if (s.basic_stats.total_lines != 1) return false;
    if (s.basic_stats.non_empty_lines != 1) return false;
    if (s.category_stats.letters != 10) return false;

    return true;
}

static bool testWithSpaces() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("with_spaces.txt"));
    if (!result) return false;

    const TextStatistics& s = stats.getStatistics();
    if (s.category_stats.spaces != 2) return false;
    if (s.basic_stats.chars_without_spaces != 14) return false;
    if (s.basic_stats.chars_without_whitespace != 14) return false;
    if (s.basic_stats.total_chars != 16) return false;

    return true;
}

static bool testTabsOnly() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("tabs_only.txt"));
    if (!result) return false;

    const TextStatistics& s = stats.getStatistics();
    if (s.category_stats.tabs != 2) return false;
    if (s.basic_stats.chars_without_whitespace != 0) return false;

    return true;
}

static bool testDigitsOnly() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("digits_only.txt"));
    if (!result) return false;

    const TextStatistics& s = stats.getStatistics();
    if (s.category_stats.digits != 10) return false;
    if (s.basic_stats.total_chars != 10) return false;

    return true;
}

static bool testPunctuations() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("punctuations.txt"));
    if (!result) return false;

    const TextStatistics& s = stats.getStatistics();
    if (s.category_stats.punctuations < 1) return false;

    return true;
}

static bool testChineseFile() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("chinese.txt"));
    if (!result) return false;

    const TextStatistics& s = stats.getStatistics();
    if (s.category_stats.chinese < 1) return false;

    return true;
}

static bool testMultiLinesWithEmpty() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("multi_lines.txt"));
    if (!result) return false;

    const TextStatistics& s = stats.getStatistics();
    if (s.basic_stats.total_lines != 7) return false;
    if (s.basic_stats.non_empty_lines != 4) return false;
    if (s.category_stats.newlines != 6) return false;

    return true;
}

static bool testOnlyNewlines() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("only_newlines.txt"));
    if (!result) return false;

    const TextStatistics& s = stats.getStatistics();
    if (s.category_stats.newlines != 3) return false;
    if (s.basic_stats.total_lines != 4) return false;
    if (s.basic_stats.non_empty_lines != 0) return false;

    return true;
}

static bool testSingleChar() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("single_char.txt"));
    if (!result) return false;

    const TextStatistics& s = stats.getStatistics();
    if (s.basic_stats.total_chars != 1) return false;
    if (s.basic_stats.total_lines != 1) return false;
    if (s.category_stats.letters != 1) return false;

    return true;
}

static bool testMixedChars() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("mixed_chars.txt"));
    if (!result) return false;

    const TextStatistics& s = stats.getStatistics();
    if (s.category_stats.letters < 1) return false;
    if (s.category_stats.digits < 1) return false;
    if (s.category_stats.spaces < 1) return false;
    if (s.category_stats.tabs < 1) return false;
    if (s.category_stats.punctuations < 1) return false;
    if (s.category_stats.newlines >= 1) return false;

    return true;
}

static bool testWhitespaceOnly() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("whitespace_only.txt"));
    if (!result) return false;

    const TextStatistics& s = stats.getStatistics();
    if (s.basic_stats.chars_without_whitespace != 0) return false;
    if (s.basic_stats.chars_without_spaces == s.basic_stats.total_chars) return false;

    return true;
}

static bool testLinesWithoutTrailingNewline() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("lines_ending.txt"));
    if (!result) return false;

    const TextStatistics& s = stats.getStatistics();
    if (s.basic_stats.total_lines != 3) return false;
    if (s.category_stats.newlines != 2) return false;

    return true;
}

static bool testInvalidFile() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("nonexistent_file_12345.txt"));
    if (result) return false;

    return true;
}

static bool testOutputTextFormat() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("single_line.txt"));
    if (!result) return false;

    OutputFormatter formatter;
    formatter.setFormat(OutputFormat::TEXT);

    std::ostringstream oss;
    formatter.output(stats.getStatistics(), oss);

    std::string output = oss.str();
    if (output.find("文本文件统计报告") == std::string::npos) return false;
    if (output.find("字符总数") == std::string::npos) return false;
    if (output.find("文件信息") == std::string::npos) return false;
    if (output.find("基本统计") == std::string::npos) return false;
    if (output.find("分类统计") == std::string::npos) return false;

    return true;
}

static bool testOutputCSVFormat() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("single_line.txt"));
    if (!result) return false;

    OutputFormatter formatter;
    formatter.setFormat(OutputFormat::CSV);

    std::ostringstream oss;
    formatter.output(stats.getStatistics(), oss);

    std::string output = oss.str();
    if (output.find("类别,项目,值") == std::string::npos) return false;
    if (output.find("文件信息") == std::string::npos) return false;
    if (output.find("基本统计") == std::string::npos) return false;
    if (output.find("分类统计") == std::string::npos) return false;

    return true;
}

static bool testOutputJSONFormat() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("single_line.txt"));
    if (!result) return false;

    OutputFormatter formatter;
    formatter.setFormat(OutputFormat::JSON);

    std::ostringstream oss;
    formatter.output(stats.getStatistics(), oss);

    std::string output = oss.str();
    if (output.find("\"file_info\"") == std::string::npos) return false;
    if (output.find("\"basic_stats\"") == std::string::npos) return false;
    if (output.find("\"category_stats\"") == std::string::npos) return false;
    if (output.find("\"total_chars\"") == std::string::npos) return false;
    if (output.find("\"letters\"") == std::string::npos) return false;

    return true;
}

static bool testOutputFormatterSetGet() {
    OutputFormatter formatter;
    if (formatter.getFormat() != OutputFormat::TEXT) return false;

    formatter.setFormat(OutputFormat::CSV);
    if (formatter.getFormat() != OutputFormat::CSV) return false;

    formatter.setFormat(OutputFormat::JSON);
    if (formatter.getFormat() != OutputFormat::JSON) return false;

    formatter.setFormat(OutputFormat::TEXT);
    if (formatter.getFormat() != OutputFormat::TEXT) return false;

    return true;
}

static bool testEncodingDetection() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("single_line.txt"));
    if (!result) return false;

    const TextStatistics& s = stats.getStatistics();
    if (s.file_info.encoding != Encoding::UTF8 && s.file_info.encoding != Encoding::ASCII) {
        return false;
    }

    return true;
}

static bool testFileSize() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("single_line.txt"));
    if (!result) return false;

    const TextStatistics& s = stats.getStatistics();
    if (s.file_info.file_size != 10) return false;

    return true;
}

static bool testFilePathStored() {
    TextStats stats;
    std::string testPath = TEST_FILE("single_line.txt");
    bool result = stats.analyzeFile(testPath);
    if (!result) return false;

    const TextStatistics& s = stats.getStatistics();
    if (s.file_info.file_path != testPath) return false;

    return true;
}

static bool testStatsReset() {
    TextStats stats;
    stats.analyzeFile(TEST_FILE("single_line.txt"));

    const TextStatistics& s1 = stats.getStatistics();
    if (s1.basic_stats.total_chars != 10) return false;

    stats.reset();
    const TextStatistics& s2 = stats.getStatistics();
    if (s2.basic_stats.total_chars != 0) return false;
    if (s2.file_info.file_size != 0) return false;
    if (s2.category_stats.letters != 0) return false;
    if (s2.file_info.encoding != Encoding::UNKNOWN) return false;

    return true;
}

int main() {
    std::cout << "==========================================" << std::endl;
    std::cout << "      Text Stats Test Runner" << std::endl;
    std::cout << "==========================================" << std::endl;
    std::cout << std::endl;

    TestRunner runner;

    std::cout << "--- Basic Stats Tests ---" << std::endl;
    runner.runTest("Empty file (all zeros)", testEmptyFile);
    runner.runTest("Single line no newline", testSingleLineNoNewline);
    runner.runTest("Single character", testSingleChar);
    runner.runTest("File size check", testFileSize);
    runner.runTest("File path stored", testFilePathStored);
    runner.runTest("Stats reset functionality", testStatsReset);

    std::cout << "\n--- Line Count Tests ---" << std::endl;
    runner.runTest("Multi lines with empty lines", testMultiLinesWithEmpty);
    runner.runTest("Only newline characters", testOnlyNewlines);
    runner.runTest("Lines without trailing newline", testLinesWithoutTrailingNewline);

    std::cout << "\n--- Category Stats Tests ---" << std::endl;
    runner.runTest("Spaces count", testWithSpaces);
    runner.runTest("Tabs only", testTabsOnly);
    runner.runTest("Digits only", testDigitsOnly);
    runner.runTest("Punctuations", testPunctuations);
    runner.runTest("Chinese characters", testChineseFile);
    runner.runTest("Mixed characters", testMixedChars);
    runner.runTest("Whitespace only", testWhitespaceOnly);

    std::cout << "\n--- Output Format Tests ---" << std::endl;
    runner.runTest("Text format output", testOutputTextFormat);
    runner.runTest("CSV format output", testOutputCSVFormat);
    runner.runTest("JSON format output", testOutputJSONFormat);
    runner.runTest("Output formatter set/get", testOutputFormatterSetGet);

    std::cout << "\n--- Encoding & Error Tests ---" << std::endl;
    runner.runTest("Encoding detection", testEncodingDetection);
    runner.runTest("Invalid file path", testInvalidFile);

    runner.printSummary();

    return runner.getFailedCount() > 0 ? 1 : 0;
}
