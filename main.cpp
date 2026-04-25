#include <iostream>
#include <string>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <shellapi.h>
#endif

#include "text_stats.h"
#include "output_formatter.h"
#include "encoding_utils.h"

void printUsage(const char* programName) {
    std::cerr << "用法: " << programName << " [选项] <文件路径>" << std::endl;
    std::cerr << std::endl;
    std::cerr << "选项:" << std::endl;
    std::cerr << "  -f, --format <格式>  指定输出格式" << std::endl;
    std::cerr << "                          可用格式: text (默认), csv, json" << std::endl;
    std::cerr << "  -h, --help            显示此帮助信息" << std::endl;
    std::cerr << std::endl;
    std::cerr << "输出格式说明:" << std::endl;
    std::cerr << "  text  - 对齐的表格形式输出（默认）" << std::endl;
    std::cerr << "  csv   - 逗号分隔的键值对格式" << std::endl;
    std::cerr << "  json  - 结构化的JSON对象格式" << std::endl;
    std::cerr << std::endl;
    std::cerr << "示例:" << std::endl;
    std::cerr << "  " << programName << " example.txt" << std::endl;
    std::cerr << "  " << programName << " -f csv example.txt" << std::endl;
    std::cerr << "  " << programName << " --format json /path/to/file.txt" << std::endl;
}

#ifdef _WIN32
int wmain(int argc, wchar_t* argv[]) {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    _setmode(_fileno(stdout), _O_BINARY);
    _setmode(_fileno(stderr), _O_BINARY);

    OutputFormat outputFormat = OutputFormat::TEXT;
    std::wstring filepathW;
    std::string programNameUtf8 = encoding_utils::wideToUtf8(argv[0]);

    if (argc < 2) {
        printUsage(programNameUtf8.c_str());
        return 1;
    }

    int i = 1;
    while (i < argc) {
        std::string argUtf8 = encoding_utils::wideToUtf8(argv[i]);

        if (argUtf8 == "-h" || argUtf8 == "--help") {
            printUsage(programNameUtf8.c_str());
            return 0;
        } else if (argUtf8 == "-f" || argUtf8 == "--format") {
            if (i + 1 >= argc) {
                std::cerr << "错误: 格式选项缺少参数" << std::endl;
                printUsage(programNameUtf8.c_str());
                return 1;
            }
            std::string formatStr = encoding_utils::wideToUtf8(argv[i + 1]);
            if (formatStr == "text") {
                outputFormat = OutputFormat::TEXT;
            } else if (formatStr == "csv") {
                outputFormat = OutputFormat::CSV;
            } else if (formatStr == "json") {
                outputFormat = OutputFormat::JSON;
            } else {
                std::cerr << "错误: 未知的输出格式 '" << formatStr << "'" << std::endl;
                std::cerr << "可用格式: text, csv, json" << std::endl;
                return 1;
            }
            i += 2;
        } else if (argUtf8.length() > 0 && argUtf8[0] == '-') {
            std::cerr << "错误: 未知选项 '" << argUtf8 << "'" << std::endl;
            printUsage(programNameUtf8.c_str());
            return 1;
        } else {
            filepathW = argv[i];
            i++;
        }
    }

    if (filepathW.empty()) {
        std::cerr << "错误: 未指定文件路径" << std::endl;
        printUsage(programNameUtf8.c_str());
        return 1;
    }

    TextStats stats;
    if (!stats.analyzeFile(filepathW)) {
        std::string filepathUtf8 = encoding_utils::wideToUtf8(filepathW);
        std::cerr << "错误: 无法打开或读取文件 '" << filepathUtf8 << "'" << std::endl;
        return 1;
    }

    OutputFormatter formatter;
    formatter.setFormat(outputFormat);
    formatter.output(stats.getStatistics());

    return 0;
}
#else
int main(int argc, char* argv[]) {
    OutputFormat outputFormat = OutputFormat::TEXT;
    std::string filepath;

    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    int i = 1;
    while (i < argc) {
        if (std::strcmp(argv[i], "-h") == 0 || std::strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            return 0;
        } else if (std::strcmp(argv[i], "-f") == 0 || std::strcmp(argv[i], "--format") == 0) {
            if (i + 1 >= argc) {
                std::cerr << "错误: 格式选项缺少参数" << std::endl;
                printUsage(argv[0]);
                return 1;
            }
            std::string formatStr = argv[i + 1];
            if (formatStr == "text") {
                outputFormat = OutputFormat::TEXT;
            } else if (formatStr == "csv") {
                outputFormat = OutputFormat::CSV;
            } else if (formatStr == "json") {
                outputFormat = OutputFormat::JSON;
            } else {
                std::cerr << "错误: 未知的输出格式 '" << formatStr << "'" << std::endl;
                std::cerr << "可用格式: text, csv, json" << std::endl;
                return 1;
            }
            i += 2;
        } else if (argv[i][0] == '-') {
            std::cerr << "错误: 未知选项 '" << argv[i] << "'" << std::endl;
            printUsage(argv[0]);
            return 1;
        } else {
            filepath = argv[i];
            i++;
        }
    }

    if (filepath.empty()) {
        std::cerr << "错误: 未指定文件路径" << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    TextStats stats;
    if (!stats.analyzeFile(filepath)) {
        std::cerr << "错误: 无法打开或读取文件 '" << filepath << "'" << std::endl;
        return 1;
    }

    OutputFormatter formatter;
    formatter.setFormat(outputFormat);
    formatter.output(stats.getStatistics());

    return 0;
}
#endif
