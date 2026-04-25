#include "output_formatter.h"
#include <iomanip>
#include <sstream>
#include <algorithm>

void OutputFormatter::setFormat(OutputFormat format) {
    format_ = format;
}

OutputFormat OutputFormatter::getFormat() const {
    return format_;
}

void OutputFormatter::output(const TextStatistics& stats, std::ostream& os) {
    switch (format_) {
        case OutputFormat::TEXT:
            outputText(stats, os);
            break;
        case OutputFormat::CSV:
            outputCSV(stats, os);
            break;
        case OutputFormat::JSON:
            outputJSON(stats, os);
            break;
    }
}

std::string OutputFormatter::encodingToString(Encoding encoding) const {
    switch (encoding) {
        case Encoding::ASCII: return "ASCII";
        case Encoding::UTF8: return "UTF-8";
        case Encoding::UTF16_LE: return "UTF-16 LE";
        case Encoding::UTF16_BE: return "UTF-16 BE";
        case Encoding::UTF32_LE: return "UTF-32 LE";
        case Encoding::UTF32_BE: return "UTF-32 BE";
        default: return "Unknown";
    }
}

std::string OutputFormatter::jsonEscape(const std::string& str) const {
    std::ostringstream oss;
    for (char c : str) {
        switch (c) {
            case '"': oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\b': oss << "\\b"; break;
            case '\f': oss << "\\f"; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    oss << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                        << static_cast<int>(static_cast<unsigned char>(c));
                } else {
                    oss << c;
                }
        }
    }
    return oss.str();
}

void OutputFormatter::outputText(const TextStatistics& stats, std::ostream& os) {
    const int labelWidth = 30;
    const int valueWidth = 15;

    os << std::string(50, '=') << std::endl;
    os << "           文本文件统计报告" << std::endl;
    os << std::string(50, '=') << std::endl << std::endl;

    os << "【文件信息】" << std::endl;
    os << std::string(50, '-') << std::endl;
    os << std::left << std::setw(labelWidth) << "文件路径:"
       << stats.file_info.file_path << std::endl;
    os << std::left << std::setw(labelWidth) << "文件大小:"
       << stats.file_info.file_size << " 字节" << std::endl;
    os << std::left << std::setw(labelWidth) << "文件编码:"
       << encodingToString(stats.file_info.encoding) << std::endl;
    os << std::endl;

    os << "【基本统计】" << std::endl;
    os << std::string(50, '-') << std::endl;
    os << std::left << std::setw(labelWidth) << "字符总数:"
       << std::right << std::setw(valueWidth) << stats.basic_stats.total_chars << std::endl;
    os << std::left << std::setw(labelWidth) << "字符数(不含空格):"
       << std::right << std::setw(valueWidth) << stats.basic_stats.chars_without_spaces << std::endl;
    os << std::left << std::setw(labelWidth) << "字符数(不含空白):"
       << std::right << std::setw(valueWidth) << stats.basic_stats.chars_without_whitespace << std::endl;
    os << std::left << std::setw(labelWidth) << "行数:"
       << std::right << std::setw(valueWidth) << stats.basic_stats.total_lines << std::endl;
    os << std::left << std::setw(labelWidth) << "非空行数:"
       << std::right << std::setw(valueWidth) << stats.basic_stats.non_empty_lines << std::endl;
    os << std::endl;

    os << "【分类统计】" << std::endl;
    os << std::string(50, '-') << std::endl;
    os << std::left << std::setw(labelWidth) << "空格:"
       << std::right << std::setw(valueWidth) << stats.category_stats.spaces << std::endl;
    os << std::left << std::setw(labelWidth) << "制表符:"
       << std::right << std::setw(valueWidth) << stats.category_stats.tabs << std::endl;
    os << std::left << std::setw(labelWidth) << "换行符:"
       << std::right << std::setw(valueWidth) << stats.category_stats.newlines << std::endl;
    os << std::left << std::setw(labelWidth) << "字母:"
       << std::right << std::setw(valueWidth) << stats.category_stats.letters << std::endl;
    os << std::left << std::setw(labelWidth) << "数字:"
       << std::right << std::setw(valueWidth) << stats.category_stats.digits << std::endl;
    os << std::left << std::setw(labelWidth) << "标点符号:"
       << std::right << std::setw(valueWidth) << stats.category_stats.punctuations << std::endl;
    os << std::left << std::setw(labelWidth) << "中文字符:"
       << std::right << std::setw(valueWidth) << stats.category_stats.chinese << std::endl;
    os << std::endl;

    os << std::string(50, '=') << std::endl;
}

void OutputFormatter::outputCSV(const TextStatistics& stats, std::ostream& os) {
    auto quoteString = [](const std::string& s) {
        if (s.find(',') != std::string::npos ||
            s.find('"') != std::string::npos ||
            s.find('\n') != std::string::npos) {
            std::string result;
            result += '"';
            for (char c : s) {
                if (c == '"') result += '"';
                result += c;
            }
            result += '"';
            return result;
        }
        return s;
    };

    os << "类别,项目,值" << std::endl;

    os << "文件信息,文件路径," << quoteString(stats.file_info.file_path) << std::endl;
    os << "文件信息,文件大小," << stats.file_info.file_size << std::endl;
    os << "文件信息,文件编码," << encodingToString(stats.file_info.encoding) << std::endl;

    os << "基本统计,字符总数," << stats.basic_stats.total_chars << std::endl;
    os << "基本统计,字符数(不含空格)," << stats.basic_stats.chars_without_spaces << std::endl;
    os << "基本统计,字符数(不含空白)," << stats.basic_stats.chars_without_whitespace << std::endl;
    os << "基本统计,行数," << stats.basic_stats.total_lines << std::endl;
    os << "基本统计,非空行数," << stats.basic_stats.non_empty_lines << std::endl;

    os << "分类统计,空格," << stats.category_stats.spaces << std::endl;
    os << "分类统计,制表符," << stats.category_stats.tabs << std::endl;
    os << "分类统计,换行符," << stats.category_stats.newlines << std::endl;
    os << "分类统计,字母," << stats.category_stats.letters << std::endl;
    os << "分类统计,数字," << stats.category_stats.digits << std::endl;
    os << "分类统计,标点符号," << stats.category_stats.punctuations << std::endl;
    os << "分类统计,中文字符," << stats.category_stats.chinese << std::endl;
}

void OutputFormatter::outputJSON(const TextStatistics& stats, std::ostream& os) {
    os << "{" << std::endl;
    os << "  \"file_info\": {" << std::endl;
    os << "    \"file_path\": \"" << jsonEscape(stats.file_info.file_path) << "\"," << std::endl;
    os << "    \"file_size\": " << stats.file_info.file_size << "," << std::endl;
    os << "    \"encoding\": \"" << encodingToString(stats.file_info.encoding) << "\"" << std::endl;
    os << "  }," << std::endl;

    os << "  \"basic_stats\": {" << std::endl;
    os << "    \"total_chars\": " << stats.basic_stats.total_chars << "," << std::endl;
    os << "    \"chars_without_spaces\": " << stats.basic_stats.chars_without_spaces << "," << std::endl;
    os << "    \"chars_without_whitespace\": " << stats.basic_stats.chars_without_whitespace << "," << std::endl;
    os << "    \"total_lines\": " << stats.basic_stats.total_lines << "," << std::endl;
    os << "    \"non_empty_lines\": " << stats.basic_stats.non_empty_lines << std::endl;
    os << "  }," << std::endl;

    os << "  \"category_stats\": {" << std::endl;
    os << "    \"spaces\": " << stats.category_stats.spaces << "," << std::endl;
    os << "    \"tabs\": " << stats.category_stats.tabs << "," << std::endl;
    os << "    \"newlines\": " << stats.category_stats.newlines << "," << std::endl;
    os << "    \"letters\": " << stats.category_stats.letters << "," << std::endl;
    os << "    \"digits\": " << stats.category_stats.digits << "," << std::endl;
    os << "    \"punctuations\": " << stats.category_stats.punctuations << "," << std::endl;
    os << "    \"chinese\": " << stats.category_stats.chinese << std::endl;
    os << "  }" << std::endl;
    os << "}" << std::endl;
}
