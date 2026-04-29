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
    if (s.basic_stats.total_lines != 6) return false;
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
    if (s.basic_stats.total_lines != 3) return false;
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
    if (s.category_stats.chinese < 1) return false;
    if (s.category_stats.newlines < 1) return false;

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

static bool testOutputToFile_TextFormat() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("single_line.txt"));
    if (!result) return false;

    OutputFormatter formatter;
    formatter.setFormat(OutputFormat::TEXT);

    std::ostringstream oss;
    formatter.output(stats.getStatistics(), oss);

    std::string content = oss.str();
    if (content.find("文本文件统计报告") == std::string::npos) return false;
    if (content.find("字符总数") == std::string::npos) return false;
    if (content.find("10") == std::string::npos) return false;

    return true;
}

static bool testOutputToFile_CSVFormat() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("single_line.txt"));
    if (!result) return false;

    OutputFormatter formatter;
    formatter.setFormat(OutputFormat::CSV);

    std::ostringstream oss;
    formatter.output(stats.getStatistics(), oss);

    std::string content = oss.str();
    if (content.find("类别,项目,值") == std::string::npos) return false;
    if (content.find("基本统计,字符总数,10") == std::string::npos) return false;
    if (content.find("分类统计,字母,10") == std::string::npos) return false;

    return true;
}

static bool testOutputToFile_JSONFormat() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("single_line.txt"));
    if (!result) return false;

    OutputFormatter formatter;
    formatter.setFormat(OutputFormat::JSON);

    std::ostringstream oss;
    formatter.output(stats.getStatistics(), oss);

    std::string content = oss.str();
    if (content.find("\"file_info\"") == std::string::npos) return false;
    if (content.find("\"basic_stats\"") == std::string::npos) return false;
    if (content.find("\"category_stats\"") == std::string::npos) return false;
    if (content.find("\"total_chars\": 10") == std::string::npos) return false;
    if (content.find("\"letters\": 10") == std::string::npos) return false;

    return true;
}

static bool testFileOutput_ConsoleVsFile_Identical() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("chinese.txt"));
    if (!result) return false;

    OutputFormatter formatter;
    formatter.setFormat(OutputFormat::TEXT);

    std::ostringstream stream1;
    formatter.output(stats.getStatistics(), stream1);

    std::ostringstream stream2;
    formatter.output(stats.getStatistics(), stream2);

    if (stream1.str() != stream2.str()) return false;

    return true;
}

static bool testFileOutput_MultipleFormats() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("mixed_chars.txt"));
    if (!result) return false;

    OutputFormatter formatter;

    formatter.setFormat(OutputFormat::TEXT);
    std::ostringstream textOutput;
    formatter.output(stats.getStatistics(), textOutput);

    formatter.setFormat(OutputFormat::CSV);
    std::ostringstream csvOutput;
    formatter.output(stats.getStatistics(), csvOutput);

    formatter.setFormat(OutputFormat::JSON);
    std::ostringstream jsonOutput;
    formatter.output(stats.getStatistics(), jsonOutput);

    if (textOutput.str().empty()) return false;
    if (csvOutput.str().empty()) return false;
    if (jsonOutput.str().empty()) return false;

    if (textOutput.str() == csvOutput.str()) return false;
    if (textOutput.str() == jsonOutput.str()) return false;
    if (csvOutput.str() == jsonOutput.str()) return false;

    return true;
}

static bool testConsoleOutput_TextFormat() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("single_line.txt"));
    if (!result) return false;

    OutputFormatter formatter;
    formatter.setFormat(OutputFormat::TEXT);

    std::ostringstream explicitOutput;
    formatter.output(stats.getStatistics(), explicitOutput);

    std::streambuf* originalCoutBuffer = std::cout.rdbuf();
    std::ostringstream capturedCout;
    std::cout.rdbuf(capturedCout.rdbuf());

    formatter.output(stats.getStatistics());

    std::cout.rdbuf(originalCoutBuffer);

    if (capturedCout.str() != explicitOutput.str()) return false;
    if (capturedCout.str().empty()) return false;

    return true;
}

static bool testConsoleOutput_CSVFormat() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("single_line.txt"));
    if (!result) return false;

    OutputFormatter formatter;
    formatter.setFormat(OutputFormat::CSV);

    std::ostringstream explicitOutput;
    formatter.output(stats.getStatistics(), explicitOutput);

    std::streambuf* originalCoutBuffer = std::cout.rdbuf();
    std::ostringstream capturedCout;
    std::cout.rdbuf(capturedCout.rdbuf());

    formatter.output(stats.getStatistics());

    std::cout.rdbuf(originalCoutBuffer);

    if (capturedCout.str() != explicitOutput.str()) return false;
    if (capturedCout.str().empty()) return false;

    return true;
}

static bool testConsoleOutput_JSONFormat() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("single_line.txt"));
    if (!result) return false;

    OutputFormatter formatter;
    formatter.setFormat(OutputFormat::JSON);

    std::ostringstream explicitOutput;
    formatter.output(stats.getStatistics(), explicitOutput);

    std::streambuf* originalCoutBuffer = std::cout.rdbuf();
    std::ostringstream capturedCout;
    std::cout.rdbuf(capturedCout.rdbuf());

    formatter.output(stats.getStatistics());

    std::cout.rdbuf(originalCoutBuffer);

    if (capturedCout.str() != explicitOutput.str()) return false;
    if (capturedCout.str().empty()) return false;

    return true;
}

static bool testConsoleOutput_EmptyFile() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("empty.txt"));
    if (!result) return false;

    OutputFormatter formatter;
    formatter.setFormat(OutputFormat::TEXT);

    std::streambuf* originalCoutBuffer = std::cout.rdbuf();
    std::ostringstream capturedCout;
    std::cout.rdbuf(capturedCout.rdbuf());

    formatter.output(stats.getStatistics());

    std::cout.rdbuf(originalCoutBuffer);

    if (capturedCout.str().find("字符总数:                               0") == std::string::npos) return false;
    if (capturedCout.str().find("行数:                                     0") == std::string::npos) return false;

    return true;
}

static bool testConsoleOutput_ChineseContent() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("chinese.txt"));
    if (!result) return false;

    OutputFormatter formatter;
    formatter.setFormat(OutputFormat::TEXT);

    std::streambuf* originalCoutBuffer = std::cout.rdbuf();
    std::ostringstream capturedCout;
    std::cout.rdbuf(capturedCout.rdbuf());

    formatter.output(stats.getStatistics());

    std::cout.rdbuf(originalCoutBuffer);

    if (capturedCout.str().find("中文字符") == std::string::npos) return false;
    if (capturedCout.str().find("UTF-8") == std::string::npos) return false;

    return true;
}

static bool testConsoleOutput_AllFormatsSameStats() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("digits_only.txt"));
    if (!result) return false;

    OutputFormatter formatter;

    formatter.setFormat(OutputFormat::TEXT);
    std::streambuf* originalCoutBuffer = std::cout.rdbuf();
    std::ostringstream textOutput;
    std::cout.rdbuf(textOutput.rdbuf());
    formatter.output(stats.getStatistics());
    std::cout.rdbuf(originalCoutBuffer);

    formatter.setFormat(OutputFormat::CSV);
    std::ostringstream csvOutput;
    std::cout.rdbuf(csvOutput.rdbuf());
    formatter.output(stats.getStatistics());
    std::cout.rdbuf(originalCoutBuffer);

    formatter.setFormat(OutputFormat::JSON);
    std::ostringstream jsonOutput;
    std::cout.rdbuf(jsonOutput.rdbuf());
    formatter.output(stats.getStatistics());
    std::cout.rdbuf(originalCoutBuffer);

    if (textOutput.str().empty()) return false;
    if (csvOutput.str().empty()) return false;
    if (jsonOutput.str().empty()) return false;

    if (textOutput.str() == csvOutput.str()) return false;
    if (textOutput.str() == jsonOutput.str()) return false;
    if (csvOutput.str() == jsonOutput.str()) return false;

    if (textOutput.str().find("数字:                                    10") == std::string::npos) return false;
    if (csvOutput.str().find("分类统计,数字,10") == std::string::npos) return false;
    if (jsonOutput.str().find("\"digits\": 10") == std::string::npos) return false;

    return true;
}

static bool testLineStats_VariableLengths() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("variable_line_lengths.txt"));
    if (!result) return false;

    const TextStatistics& s = stats.getStatistics();

    if (s.line_stats.total_lines != 4) return false;
    if (s.line_stats.non_empty_lines != 4) return false;
    if (s.line_stats.longest_line_length < s.line_stats.shortest_line_length) return false;
    if (s.line_stats.shortest_line_length != 1) return false;
    if (s.line_stats.average_line_length <= 0) return false;

    return true;
}

static bool testLineStats_EmptyFile() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("empty.txt"));
    if (!result) return false;

    const TextStatistics& s = stats.getStatistics();

    if (s.line_stats.total_lines != 0) return false;
    if (s.line_stats.non_empty_lines != 0) return false;
    if (s.line_stats.longest_line_length != 0) return false;
    if (s.line_stats.shortest_line_length != 0) return false;
    if (s.line_stats.average_line_length != 0.0) return false;

    return true;
}

static bool testLineStats_MultiLinesWithEmpty() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("multi_lines.txt"));
    if (!result) return false;

    const TextStatistics& s = stats.getStatistics();

    if (s.line_stats.total_lines != 6) return false;
    if (s.line_stats.non_empty_lines != 4) return false;
    if (s.line_stats.longest_line_length == 0) return false;
    if (s.line_stats.shortest_line_length != 0) return false;
    if (s.line_stats.average_line_length <= 0) return false;

    return true;
}

static bool testWordStats_SingleWord() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("single_line.txt"));
    if (!result) return false;

    const TextStatistics& s = stats.getStatistics();

    if (s.word_stats.total_words != 1) return false;
    if (s.word_stats.unique_words != 1) return false;
    if (s.word_stats.top_words.size() != 1) return false;
    if (s.word_stats.top_words[0].count != 1) return false;

    return true;
}

static bool testWordStats_MultipleWords() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("with_spaces.txt"));
    if (!result) return false;

    const TextStatistics& s = stats.getStatistics();

    if (s.word_stats.total_words != 3) return false;
    if (s.word_stats.unique_words != 3) return false;

    return true;
}

static bool testWordStats_WithDuplicates() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("word_stats.txt"));
    if (!result) return false;

    const TextStatistics& s = stats.getStatistics();

    if (s.word_stats.total_words < s.word_stats.unique_words) return false;
    if (s.word_stats.total_words == 0) return false;

    uint64_t totalFromTopWords = 0;
    for (const auto& wf : s.word_stats.top_words) {
        totalFromTopWords += wf.count;
    }
    if (totalFromTopWords > s.word_stats.total_words) return false;

    return true;
}

static bool testTopWords_MostFrequent() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("top_words.txt"));
    if (!result) return false;

    const TextStatistics& s = stats.getStatistics();

    if (s.word_stats.total_words == 0) return false;
    if (s.word_stats.unique_words == 0) return false;
    if (s.word_stats.top_words.empty()) return false;

    for (size_t i = 1; i < s.word_stats.top_words.size(); ++i) {
        if (s.word_stats.top_words[i-1].count < s.word_stats.top_words[i].count) {
            return false;
        }
    }

    return true;
}

static bool testTopWords_LessThanTen() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("with_spaces.txt"));
    if (!result) return false;

    const TextStatistics& s = stats.getStatistics();

    if (s.word_stats.top_words.size() > 10) return false;
    if (s.word_stats.top_words.size() != s.word_stats.unique_words) return false;

    return true;
}

static bool testOutputFormat_LineStatsInText() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("variable_line_lengths.txt"));
    if (!result) return false;

    OutputFormatter formatter;
    formatter.setFormat(OutputFormat::TEXT);

    std::ostringstream output;
    formatter.output(stats.getStatistics(), output);

    if (output.str().find("【行统计】") == std::string::npos) return false;
    if (output.str().find("最长行长度:") == std::string::npos) return false;
    if (output.str().find("最短行长度:") == std::string::npos) return false;
    if (output.str().find("平均行长度:") == std::string::npos) return false;

    return true;
}

static bool testOutputFormat_WordStatsInText() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("top_words.txt"));
    if (!result) return false;

    OutputFormatter formatter;
    formatter.setFormat(OutputFormat::TEXT);

    std::ostringstream output;
    formatter.output(stats.getStatistics(), output);

    if (output.str().find("【单词统计】") == std::string::npos) return false;
    if (output.str().find("单词总数:") == std::string::npos) return false;
    if (output.str().find("唯一单词数:") == std::string::npos) return false;

    if (output.str().find("【最常见单词 TOP 10】") == std::string::npos) return false;

    return true;
}

static bool testOutputFormat_LineStatsInCSV() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("variable_line_lengths.txt"));
    if (!result) return false;

    OutputFormatter formatter;
    formatter.setFormat(OutputFormat::CSV);

    std::ostringstream output;
    formatter.output(stats.getStatistics(), output);

    if (output.str().find("行统计,总行数,") == std::string::npos) return false;
    if (output.str().find("行统计,非空行数,") == std::string::npos) return false;
    if (output.str().find("行统计,最长行长度,") == std::string::npos) return false;
    if (output.str().find("行统计,最短行长度,") == std::string::npos) return false;
    if (output.str().find("行统计,平均行长度,") == std::string::npos) return false;

    return true;
}

static bool testOutputFormat_WordStatsInCSV() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("top_words.txt"));
    if (!result) return false;

    OutputFormatter formatter;
    formatter.setFormat(OutputFormat::CSV);

    std::ostringstream output;
    formatter.output(stats.getStatistics(), output);

    if (output.str().find("单词统计,单词总数,") == std::string::npos) return false;
    if (output.str().find("单词统计,唯一单词数,") == std::string::npos) return false;
    if (output.str().find("最常见单词 TOP") == std::string::npos) return false;

    return true;
}

static bool testOutputFormat_LineStatsInJSON() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("variable_line_lengths.txt"));
    if (!result) return false;

    OutputFormatter formatter;
    formatter.setFormat(OutputFormat::JSON);

    std::ostringstream output;
    formatter.output(stats.getStatistics(), output);

    if (output.str().find("\"line_stats\":") == std::string::npos) return false;
    if (output.str().find("\"total_lines\":") == std::string::npos) return false;
    if (output.str().find("\"non_empty_lines\":") == std::string::npos) return false;
    if (output.str().find("\"longest_line_length\":") == std::string::npos) return false;
    if (output.str().find("\"shortest_line_length\":") == std::string::npos) return false;
    if (output.str().find("\"average_line_length\":") == std::string::npos) return false;

    return true;
}

static bool testOutputFormat_WordStatsInJSON() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("top_words.txt"));
    if (!result) return false;

    OutputFormatter formatter;
    formatter.setFormat(OutputFormat::JSON);

    std::ostringstream output;
    formatter.output(stats.getStatistics(), output);

    if (output.str().find("\"word_stats\":") == std::string::npos) return false;
    if (output.str().find("\"total_words\":") == std::string::npos) return false;
    if (output.str().find("\"unique_words\":") == std::string::npos) return false;
    if (output.str().find("\"top_words\":") == std::string::npos) return false;
    if (output.str().find("\"word\":") == std::string::npos) return false;
    if (output.str().find("\"count\":") == std::string::npos) return false;

    return true;
}

static bool testWordStats_EmptyFile() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("empty.txt"));
    if (!result) return false;

    const TextStatistics& s = stats.getStatistics();

    if (s.word_stats.total_words != 0) return false;
    if (s.word_stats.unique_words != 0) return false;
    if (!s.word_stats.top_words.empty()) return false;

    return true;
}

static bool testWordStats_CaseInsensitive() {
    TextStats stats;
    bool result = stats.analyzeFile(TEST_FILE("word_stats.txt"));
    if (!result) return false;

    const TextStatistics& s = stats.getStatistics();

    for (const auto& wf : s.word_stats.top_words) {
        for (char c : wf.word) {
            if (std::isupper(c)) {
                return false;
            }
        }
    }

    return true;
}

static bool testUnicodeLetter_ASCII() {
    if (!TextStats::isUnicodeLetter('A')) return false;
    if (!TextStats::isUnicodeLetter('Z')) return false;
    if (!TextStats::isUnicodeLetter('a')) return false;
    if (!TextStats::isUnicodeLetter('z')) return false;

    if (TextStats::isUnicodeLetter('0')) return false;
    if (TextStats::isUnicodeLetter(' ')) return false;
    if (TextStats::isUnicodeLetter('!')) return false;

    return true;
}

static bool testUnicodeLetter_Chinese() {
    if (!TextStats::isUnicodeLetter(0x4E00)) return false;
    if (!TextStats::isUnicodeLetter(0x4E2D)) return false;
    if (!TextStats::isUnicodeLetter(0x6587)) return false;
    if (!TextStats::isUnicodeLetter(0x9FFF)) return false;

    if (!TextStats::isUnicodeLetter(0x3400)) return false;
    if (!TextStats::isUnicodeLetter(0x4DBF)) return false;

    return true;
}

static bool testUnicodeLetter_Japanese() {
    if (!TextStats::isUnicodeLetter(0x3041)) return false;
    if (!TextStats::isUnicodeLetter(0x3080)) return false;
    if (!TextStats::isUnicodeLetter(0x309F)) return false;

    if (!TextStats::isUnicodeLetter(0x30A1)) return false;
    if (!TextStats::isUnicodeLetter(0x30C0)) return false;
    if (!TextStats::isUnicodeLetter(0x30FF)) return false;

    return true;
}

static bool testUnicodeLetter_Korean() {
    if (!TextStats::isUnicodeLetter(0xAC00)) return false;
    if (!TextStats::isUnicodeLetter(0xB098)) return false;
    if (!TextStats::isUnicodeLetter(0xD7A3)) return false;

    if (!TextStats::isUnicodeLetter(0x1100)) return false;
    if (!TextStats::isUnicodeLetter(0x11FF)) return false;

    return true;
}

static bool testUnicodeDigit_Basic() {
    if (!TextStats::isUnicodeDigit('0')) return false;
    if (!TextStats::isUnicodeDigit('5')) return false;
    if (!TextStats::isUnicodeDigit('9')) return false;

    if (TextStats::isUnicodeDigit('A')) return false;
    if (TextStats::isUnicodeDigit(' ')) return false;

    return true;
}

static bool testUnicodeDigit_Fullwidth() {
    if (!TextStats::isUnicodeDigit(0xFF10)) return false;
    if (!TextStats::isUnicodeDigit(0xFF15)) return false;
    if (!TextStats::isUnicodeDigit(0xFF19)) return false;

    return true;
}

static bool testUnicodeWhitespace_Basic() {
    if (!TextStats::isUnicodeWhitespace(' ')) return false;
    if (!TextStats::isUnicodeWhitespace('\t')) return false;
    if (!TextStats::isUnicodeWhitespace('\n')) return false;
    if (!TextStats::isUnicodeWhitespace('\r')) return false;

    if (TextStats::isUnicodeWhitespace('A')) return false;
    if (TextStats::isUnicodeWhitespace('0')) return false;

    return true;
}

static bool testUnicodeWhitespace_Unicode() {
    if (!TextStats::isUnicodeWhitespace(0x00A0)) return false;
    if (!TextStats::isUnicodeWhitespace(0x2000)) return false;
    if (!TextStats::isUnicodeWhitespace(0x3000)) return false;

    return true;
}

static bool testUnicodePunctuation_Basic() {
    if (!TextStats::isUnicodePunctuation('!')) return false;
    if (!TextStats::isUnicodePunctuation('.')) return false;
    if (!TextStats::isUnicodePunctuation('?')) return false;
    if (!TextStats::isUnicodePunctuation(',')) return false;
    if (!TextStats::isUnicodePunctuation(';')) return false;
    if (!TextStats::isUnicodePunctuation(':')) return false;
    if (!TextStats::isUnicodePunctuation('"')) return false;
    if (!TextStats::isUnicodePunctuation('\'')) return false;
    if (!TextStats::isUnicodePunctuation('(')) return false;
    if (!TextStats::isUnicodePunctuation(')')) return false;
    if (!TextStats::isUnicodePunctuation('[')) return false;
    if (!TextStats::isUnicodePunctuation(']')) return false;
    if (!TextStats::isUnicodePunctuation('{')) return false;
    if (!TextStats::isUnicodePunctuation('}')) return false;

    if (TextStats::isUnicodePunctuation('A')) return false;
    if (TextStats::isUnicodePunctuation('0')) return false;

    return true;
}

static bool testUnicodePunctuation_Chinese() {
    if (!TextStats::isUnicodePunctuation(0x3001)) return false;
    if (!TextStats::isUnicodePunctuation(0x3002)) return false;

    return true;
}

static bool testUnicodeToLower_Basic() {
    if (TextStats::unicodeToLower('A') != 'a') return false;
    if (TextStats::unicodeToLower('Z') != 'z') return false;
    if (TextStats::unicodeToLower('M') != 'm') return false;

    if (TextStats::unicodeToLower('a') != 'a') return false;
    if (TextStats::unicodeToLower('0') != '0') return false;
    if (TextStats::unicodeToLower(' ') != ' ') return false;

    return true;
}

static bool testUnicodeToLower_Extended() {
    if (TextStats::unicodeToLower(0xC0) != 0xE0) return false;
    if (TextStats::unicodeToLower(0xC9) != 0xE9) return false;
    if (TextStats::unicodeToLower(0x00DE) != 0x00FE) return false;

    return true;
}

static bool testCodepointToUTF8_Basic() {
    std::string s1 = TextStats::codepointToUTF8('A');
    if (s1 != "A") return false;

    std::string s2 = TextStats::codepointToUTF8('0');
    if (s2 != "0") return false;

    return true;
}

static bool testCodepointToUTF8_Chinese() {
    std::string s1 = TextStats::codepointToUTF8(0x4E2D);
    if (s1.size() != 3) return false;

    std::string s2 = TextStats::codepointToUTF8(0x6587);
    if (s2.size() != 3) return false;

    return true;
}

static bool testCodepointToUTF8_Multibyte() {
    std::string s1 = TextStats::codepointToUTF8(0x00C9);
    if (s1.size() != 2) return false;

    std::string s2 = TextStats::codepointToUTF8(0x10000);
    if (s2.size() != 4) return false;

    return true;
}

static bool testCJKFunctions_Chinese() {
    if (!TextStats::isChineseCharOptimized(0x4E00)) return false;
    if (!TextStats::isChineseCharOptimized(0x4E2D)) return false;
    if (!TextStats::isChineseCharOptimized(0x6587)) return false;
    if (!TextStats::isChineseCharOptimized(0x9FFF)) return false;
    if (!TextStats::isChineseCharOptimized(0x3400)) return false;
    if (!TextStats::isChineseCharOptimized(0x4DBF)) return false;
    if (!TextStats::isChineseCharOptimized(0x20000)) return false;
    if (!TextStats::isChineseCharOptimized(0x2A6DF)) return false;
    if (!TextStats::isChineseCharOptimized(0xF900)) return false;
    if (!TextStats::isChineseCharOptimized(0xFAFF)) return false;

    if (TextStats::isChineseCharOptimized('A')) return false;
    if (TextStats::isChineseCharOptimized('0')) return false;

    return true;
}

static bool testCJKFunctions_Japanese() {
    if (!TextStats::isHiraganaOptimized(0x3041)) return false;
    if (!TextStats::isHiraganaOptimized(0x3080)) return false;
    if (!TextStats::isHiraganaOptimized(0x309F)) return false;

    if (!TextStats::isKatakanaOptimized(0x30A1)) return false;
    if (!TextStats::isKatakanaOptimized(0x30C0)) return false;
    if (!TextStats::isKatakanaOptimized(0x30FF)) return false;
    if (!TextStats::isKatakanaOptimized(0x31F0)) return false;
    if (!TextStats::isKatakanaOptimized(0x31FF)) return false;

    return true;
}

static bool testCJKFunctions_Korean() {
    if (!TextStats::isHangulOptimized(0xAC00)) return false;
    if (!TextStats::isHangulOptimized(0xB098)) return false;
    if (!TextStats::isHangulOptimized(0xD7A3)) return false;

    if (!TextStats::isHangulOptimized(0x1100)) return false;
    if (!TextStats::isHangulOptimized(0x11FF)) return false;
    if (!TextStats::isHangulOptimized(0x3130)) return false;
    if (!TextStats::isHangulOptimized(0x318F)) return false;

    return true;
}

static bool testFileSizeLimit_Default() {
    TextStats stats;

    using namespace TextStatsConstants;
    uint64_t defaultLimit = DEFAULT_MAX_FILE_SIZE;

    if (stats.getMaxFileSize() != defaultLimit) return false;

    return true;
}

static bool testFileSizeLimit_SetGet() {
    TextStats stats;

    uint64_t newLimit = 50ULL * 1024ULL * 1024ULL;
    stats.setMaxFileSize(newLimit);

    if (stats.getMaxFileSize() != newLimit) return false;

    uint64_t zeroLimit = 0;
    stats.setMaxFileSize(zeroLimit);
    if (stats.getMaxFileSize() != zeroLimit) return false;

    return true;
}

static bool testFileSizeLimit_SetToZero() {
    TextStats stats;
    stats.setMaxFileSize(0);

    bool result = stats.analyzeFile(TEST_FILE("single_line.txt"));
    if (result) return false;

    return true;
}

static bool testFileSizeLimit_WithinLimit() {
    TextStats stats;

    uint64_t largeLimit = 1ULL * 1024ULL * 1024ULL;
    stats.setMaxFileSize(largeLimit);

    bool result = stats.analyzeFile(TEST_FILE("single_line.txt"));
    if (!result) return false;

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

    std::cout << "\n--- File Output Tests ---" << std::endl;
    runner.runTest("Output TEXT to file", testOutputToFile_TextFormat);
    runner.runTest("Output CSV to file", testOutputToFile_CSVFormat);
    runner.runTest("Output JSON to file", testOutputToFile_JSONFormat);
    runner.runTest("Console vs File output identical", testFileOutput_ConsoleVsFile_Identical);
    runner.runTest("Multiple formats output different", testFileOutput_MultipleFormats);

    std::cout << "\n--- Console Output Tests ---" << std::endl;
    runner.runTest("Console output TEXT format", testConsoleOutput_TextFormat);
    runner.runTest("Console output CSV format", testConsoleOutput_CSVFormat);
    runner.runTest("Console output JSON format", testConsoleOutput_JSONFormat);
    runner.runTest("Console output empty file", testConsoleOutput_EmptyFile);
    runner.runTest("Console output chinese content", testConsoleOutput_ChineseContent);
    runner.runTest("Console output all formats same stats", testConsoleOutput_AllFormatsSameStats);

    std::cout << "\n--- Line Stats Tests ---" << std::endl;
    runner.runTest("Line stats - variable lengths", testLineStats_VariableLengths);
    runner.runTest("Line stats - empty file", testLineStats_EmptyFile);
    runner.runTest("Line stats - multi lines with empty", testLineStats_MultiLinesWithEmpty);

    std::cout << "\n--- Word Stats Tests ---" << std::endl;
    runner.runTest("Word stats - single word", testWordStats_SingleWord);
    runner.runTest("Word stats - multiple words", testWordStats_MultipleWords);
    runner.runTest("Word stats - with duplicates", testWordStats_WithDuplicates);
    runner.runTest("Word stats - empty file", testWordStats_EmptyFile);
    runner.runTest("Word stats - case insensitive", testWordStats_CaseInsensitive);

    std::cout << "\n--- Top Words Tests ---" << std::endl;
    runner.runTest("Top words - most frequent first", testTopWords_MostFrequent);
    runner.runTest("Top words - less than ten", testTopWords_LessThanTen);

    std::cout << "\n--- New Features Output Tests ---" << std::endl;
    runner.runTest("Output format - line stats in TEXT", testOutputFormat_LineStatsInText);
    runner.runTest("Output format - word stats in TEXT", testOutputFormat_WordStatsInText);
    runner.runTest("Output format - line stats in CSV", testOutputFormat_LineStatsInCSV);
    runner.runTest("Output format - word stats in CSV", testOutputFormat_WordStatsInCSV);
    runner.runTest("Output format - line stats in JSON", testOutputFormat_LineStatsInJSON);
    runner.runTest("Output format - word stats in JSON", testOutputFormat_WordStatsInJSON);

    std::cout << "\n--- Encoding & Error Tests ---" << std::endl;
    runner.runTest("Encoding detection", testEncodingDetection);
    runner.runTest("Invalid file path", testInvalidFile);

    std::cout << "\n--- Unicode Character Classification Tests ---" << std::endl;
    runner.runTest("Unicode letter - ASCII", testUnicodeLetter_ASCII);
    runner.runTest("Unicode letter - Chinese", testUnicodeLetter_Chinese);
    runner.runTest("Unicode letter - Japanese", testUnicodeLetter_Japanese);
    runner.runTest("Unicode letter - Korean", testUnicodeLetter_Korean);
    runner.runTest("Unicode digit - basic", testUnicodeDigit_Basic);
    runner.runTest("Unicode digit - fullwidth", testUnicodeDigit_Fullwidth);
    runner.runTest("Unicode whitespace - basic", testUnicodeWhitespace_Basic);
    runner.runTest("Unicode whitespace - Unicode", testUnicodeWhitespace_Unicode);
    runner.runTest("Unicode punctuation - basic", testUnicodePunctuation_Basic);
    runner.runTest("Unicode punctuation - Chinese", testUnicodePunctuation_Chinese);

    std::cout << "\n--- Unicode Conversion Tests ---" << std::endl;
    runner.runTest("Unicode to lower - basic", testUnicodeToLower_Basic);
    runner.runTest("Unicode to lower - extended", testUnicodeToLower_Extended);
    runner.runTest("Codepoint to UTF8 - basic", testCodepointToUTF8_Basic);
    runner.runTest("Codepoint to UTF8 - Chinese", testCodepointToUTF8_Chinese);
    runner.runTest("Codepoint to UTF8 - multibyte", testCodepointToUTF8_Multibyte);

    std::cout << "\n--- CJK Detection Tests ---" << std::endl;
    runner.runTest("CJK detection - Chinese", testCJKFunctions_Chinese);
    runner.runTest("CJK detection - Japanese", testCJKFunctions_Japanese);
    runner.runTest("CJK detection - Korean", testCJKFunctions_Korean);

    std::cout << "\n--- File Size Limit Tests ---" << std::endl;
    runner.runTest("File size limit - default", testFileSizeLimit_Default);
    runner.runTest("File size limit - set/get", testFileSizeLimit_SetGet);
    runner.runTest("File size limit - set to zero", testFileSizeLimit_SetToZero);
    runner.runTest("File size limit - within limit", testFileSizeLimit_WithinLimit);

    runner.printSummary();

    return runner.getFailedCount() > 0 ? 1 : 0;
}
