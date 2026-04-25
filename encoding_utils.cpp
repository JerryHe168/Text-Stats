#include "encoding_utils.h"

#ifdef _WIN32
namespace encoding_utils {

std::wstring utf8ToWide(const std::string& utf8Str) {
    if (utf8Str.empty()) {
        return std::wstring();
    }

    int wideLen = MultiByteToWideChar(
        CP_UTF8,
        0,
        utf8Str.c_str(),
        -1,
        nullptr,
        0
    );

    if (wideLen <= 0) {
        return std::wstring();
    }

    std::wstring wideStr(wideLen - 1, L'\0');
    MultiByteToWideChar(
        CP_UTF8,
        0,
        utf8Str.c_str(),
        -1,
        &wideStr[0],
        wideLen
    );

    return wideStr;
}

std::string wideToUtf8(const std::wstring& wideStr) {
    if (wideStr.empty()) {
        return std::string();
    }

    int utf8Len = WideCharToMultiByte(
        CP_UTF8,
        0,
        wideStr.c_str(),
        -1,
        nullptr,
        0,
        nullptr,
        nullptr
    );

    if (utf8Len <= 0) {
        return std::string();
    }

    std::string utf8Str(utf8Len - 1, '\0');
    WideCharToMultiByte(
        CP_UTF8,
        0,
        wideStr.c_str(),
        -1,
        &utf8Str[0],
        utf8Len,
        nullptr,
        nullptr
    );

    return utf8Str;
}

}
#endif
