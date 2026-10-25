#pragma once

#include <windows.h>

#include <string>
#include <vector>

namespace w32ph::process {

struct ProcessEntry {
    DWORD processId = 0;
    DWORD parentProcessId = 0;
    std::string imageName;
};

bool SnapshotAllProcesses(
    std::vector<ProcessEntry>& outProcesses,
    DWORD& lastError
);

} // namespace w32ph::process