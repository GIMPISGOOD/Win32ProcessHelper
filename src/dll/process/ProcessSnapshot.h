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

struct ProcessEntryEx {
    DWORD processId = 0;
    DWORD parentProcessId = 0;
    DWORD threadCount = 0;
    int basePriority = 0;
    std::string imageName;
};

bool SnapshotAllProcesses(
    std::vector<ProcessEntry>& outProcesses,
    DWORD& lastError
);

bool SnapshotAllProcessesDetailed(
    std::vector<ProcessEntryEx>& outProcesses,
    DWORD& lastError
);

bool FindProcessSnapshotById(
    DWORD processId,
    ProcessEntryEx& outProcess,
    DWORD& lastError
);

} // namespace w32ph::process