#pragma once
#include <string>
namespace wph { namespace util {
    std::string EscapeJson(const std::wstring& wstr);
    std::string WStringToUtf8(const std::wstring& wstr);
}}