#pragma once
#include <string>
#include <vector>

namespace wph { namespace core {

struct ProcessInfo {
    DWORD pid;
    DWORD ppid;
    std::wstring name;
    std::wstring path;
    std::wstring commandLine;
    std::wstring integrityLevel;
};

std::vector<ProcessInfo> EnumerateProcesses();
std::string GenerateProcessesJson(const std::vector<ProcessInfo>& processes);

}}