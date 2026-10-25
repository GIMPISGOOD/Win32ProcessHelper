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

} // namespace w32ph::process