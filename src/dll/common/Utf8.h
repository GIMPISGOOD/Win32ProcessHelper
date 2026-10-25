#pragma once

#include <string>

namespace w32ph::common {

std::string WideToUtf8(const wchar_t* src, size_t length);
std::string WideToUtf8(const std::wstring& src);

} // namespace w32ph::common