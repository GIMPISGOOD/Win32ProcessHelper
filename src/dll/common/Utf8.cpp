#include "common/Utf8.h"

#include <windows.h>
#include <climits>

namespace w32ph::common {

std::string WideToUtf8(const wchar_t* src, size_t length) {
    if (src == nullptr || length == 0) {
        return std::string();
    }

    if (length > static_cast<size_t>(INT_MAX)) {
        return std::string();
    }

    const int srcLen = static_cast<int>(length);

    const int required = WideCharToMultiByte(
        CP_UTF8,
        0,
        src,
        srcLen,
        nullptr,
        0,
        nullptr,
        nullptr
    );

    if (required <= 0) {
        return std::string();
    }

    std::string result(static_cast<size_t>(required), '\0');

    const int converted = WideCharToMultiByte(
        CP_UTF8,
        0,
        src,
        srcLen,
        result.data(),
        required,
        nullptr,
        nullptr
    );

    if (converted <= 0) {
        return std::string();
    }

    result.resize(static_cast<size_t>(converted));
    return result;
}

std::string WideToUtf8(const std::wstring& src) {
    return WideToUtf8(src.c_str(), src.size());
}

} // namespace w32ph::common