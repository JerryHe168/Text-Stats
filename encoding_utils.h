#ifndef ENCODING_UTILS_H
#define ENCODING_UTILS_H

#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

namespace encoding_utils {

#ifdef _WIN32
std::wstring utf8ToWide(const std::string& utf8Str);
std::string wideToUtf8(const std::wstring& wideStr);
#endif

}

#endif // ENCODING_UTILS_H
