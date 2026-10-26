#include "process/ProcessSnapshot.h"
#include "common/Utf8.h"

#include <TlHelp32.h>
#include <wchar.h>

namespace w32ph::process {

bool SnapshotAllProcesses(
    std::vector<ProcessEntry>& outProcesses,
    DWORD& lastError
) {
    outProcesses.clear();
    lastError = 0;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        lastError = GetLastError();
        return false;
    }

    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);

    if (Process32FirstW(snapshot, &entry) == FALSE) {
        lastError = GetLastError();
        CloseHandle(snapshot);
        return false;
    }

    do {
        ProcessEntry item;
        item.processId = entry.th32ProcessID;
        item.parentProcessId = entry.th32ParentProcessID;

        const size_t maxChars = sizeof(entry.szExeFile) / sizeof(entry.szExeFile[0]);
        const size_t nameLength = wcsnlen_s(entry.szExeFile, maxChars);
        item.imageName = common::WideToUtf8(entry.szExeFile, nameLength);

        outProcesses.push_back(std::move(item));
    } while (Process32NextW(snapshot, &entry) == TRUE);

    const DWORD lastEnumerationError = GetLastError();
    CloseHandle(snapshot);

    if (lastEnumerationError != ERROR_NO_MORE_FILES) {
        lastError = lastEnumerationError;
        return false;
    }

    return true;
}

bool SnapshotAllProcessesDetailed(
    std::vector<ProcessEntryEx>& outProcesses,
    DWORD& lastError
) {
    outProcesses.clear();
    lastError = 0;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        lastError = GetLastError();
        return false;
    }

    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);

    if (Process32FirstW(snapshot, &entry) == FALSE) {
        lastError = GetLastError();
        CloseHandle(snapshot);
        return false;
    }

    do {
        ProcessEntryEx item;
        item.processId = entry.th32ProcessID;
        item.parentProcessId = entry.th32ParentProcessID;
        item.threadCount = entry.cntThreads;
        item.basePriority = static_cast<int>(entry.pcPriClassBase);

        const size_t maxChars = sizeof(entry.szExeFile) / sizeof(entry.szExeFile[0]);
        const size_t nameLength = wcsnlen_s(entry.szExeFile, maxChars);
        item.imageName = common::WideToUtf8(entry.szExeFile, nameLength);

        outProcesses.push_back(std::move(item));
    } while (Process32NextW(snapshot, &entry) == TRUE);

    const DWORD lastEnumerationError = GetLastError();
    CloseHandle(snapshot);

    if (lastEnumerationError != ERROR_NO_MORE_FILES) {
        lastError = lastEnumerationError;
        return false;
    }

    return true;
}

bool FindProcessSnapshotById(
    DWORD processId,
    ProcessEntryEx& outProcess,
    DWORD& lastError
) {
    std::vector<ProcessEntryEx> processes;

    if (!SnapshotAllProcessesDetailed(processes, lastError)) {
        return false;
    }

    for (const auto& process : processes) {
        if (process.processId == processId) {
            outProcess = process;
            lastError = 0;
            return true;
        }
    }

    lastError = ERROR_PROC_NOT_FOUND;
    return false;
}

} // namespace w32ph::process