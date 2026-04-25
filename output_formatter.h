#ifndef OUTPUT_FORMATTER_H
#define OUTPUT_FORMATTER_H

#include "text_stats.h"
#include <iostream>

enum class OutputFormat {
    TEXT,
    CSV,
    JSON
};

class OutputFormatter {
public:
    OutputFormatter() = default;
    ~OutputFormatter() = default;

    void setFormat(OutputFormat format);
    OutputFormat getFormat() const;

    void output(const TextStatistics& stats, std::ostream& os = std::cout);

    void outputText(const TextStatistics& stats, std::ostream& os);
    void outputCSV(const TextStatistics& stats, std::ostream& os);
    void outputJSON(const TextStatistics& stats, std::ostream& os);

private:
    OutputFormat format_ = OutputFormat::TEXT;

    std::string encodingToString(Encoding encoding) const;
    std::string jsonEscape(const std::string& str) const;
};

#endif // OUTPUT_FORMATTER_H
