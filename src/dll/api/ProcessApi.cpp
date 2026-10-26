#include "Win32ProcessHelper.h"
#include "common/Json.h"
#include "common/Memory.h"
#include "common/Utf8.h"
#include "process/ProcessSnapshot.h"

#include <windows.h>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

using w32ph::common::DuplicateStringToHeap;
using w32ph::common::EscapeJson;
using w32ph::common::WideToUtf8;
using w32ph::process::ProcessEntry;
using w32ph::process::ProcessEntryEx;
using w32ph::process::SnapshotAllProcesses;
using w32ph::process::SnapshotAllProcessesDetailed;
using w32ph::process::FindProcessSnapshotById;

namespace {

void FreeProcessArrayInternal(
    W32PH_PROCESS_INFO* processes,
    uint32_t count
) {
    if (processes == nullptr) {
        return;
    }

    for (uint32_t i = 0; i < count; ++i) {
        if (processes[i].imageName != nullptr) {
            HeapFree(GetProcessHeap(), 0, processes[i].imageName);
            processes[i].imageName = nullptr;
        }
    }

    HeapFree(GetProcessHeap(), 0, processes);
}

void FreeProcessArrayDetailedInternal(
    W32PH_PROCESS_INFO_EX* processes,
    uint32_t count
) {
    if (processes == nullptr) {
        return;
    }

    for (uint32_t i = 0; i < count; ++i) {
        if (processes[i].imageName != nullptr) {
            HeapFree(GetProcessHeap(), 0, processes[i].imageName);
            processes[i].imageName = nullptr;
        }
    }

    HeapFree(GetProcessHeap(), 0, processes);
}

void AppendComma(std::string& json, bool& first) {
    if (!first) {
        json.push_back(',');
    }
    first = false;
}

void AppendJsonBool(
    std::string& json,
    bool& first,
    const char* key,
    bool value
) {
    AppendComma(json, first);
    json.push_back('"');
    json.append(key);
    json.append("\":");
    json.append(value ? "true" : "false");
}

void AppendJsonUInt(
    std::string& json,
    bool& first,
    const char* key,
    uint64_t value
) {
    AppendComma(json, first);
    json.push_back('"');
    json.append(key);
    json.append("\":");
    json.append(std::to_string(value));
}

void AppendJsonInt(
    std::string& json,
    bool& first,
    const char* key,
    int64_t value
) {
    AppendComma(json, first);
    json.push_back('"');
    json.append(key);
    json.append("\":");
    json.append(std::to_string(value));
}

void AppendJsonString(
    std::string& json,
    bool& first,
    const char* key,
    const std::string& value
) {
    AppendComma(json, first);
    json.push_back('"');
    json.append(key);
    json.append("\":\"");
    json.append(EscapeJson(value));
    json.push_back('"');
}

void AppendJsonStringOrNull(
    std::string& json,
    bool& first,
    const char* key,
    const std::string* value
) {
    AppendComma(json, first);
    json.push_back('"');
    json.append(key);
    json.append("\":");

    if (value == nullptr) {
        json.append("null");
        return;
    }

    json.push_back('"');
    json.append(EscapeJson(*value));
    json.push_back('"');
}

std::string FileTimeToIso8601Utc(const FILETIME& fileTime) {
    if (fileTime.dwLowDateTime == 0 && fileTime.dwHighDateTime == 0) {
        return std::string();
    }

    SYSTEMTIME systemTime{};
    if (FileTimeToSystemTime(&fileTime, &systemTime) == FALSE) {
        return std::string();
    }

    char buffer[32];
    const int written = std::snprintf(
        buffer,
        sizeof(buffer),
        "%04u-%02u-%02uT%02u:%02u:%02uZ",
        static_cast<unsigned>(systemTime.wYear),
        static_cast<unsigned>(systemTime.wMonth),
        static_cast<unsigned>(systemTime.wDay),
        static_cast<unsigned>(systemTime.wHour),
        static_cast<unsigned>(systemTime.wMinute),
        static_cast<unsigned>(systemTime.wSecond)
    );

    if (written <= 0) {
        return std::string();
    }

    return std::string(buffer, static_cast<size_t>(written));
}

std::string BuildProcessListJson(const std::vector<ProcessEntry>& processes) {
    std::string json;
    json.reserve(processes.size() * 96 + 2);
    json.push_back('[');

    bool first = true;

    for (const auto& process : processes) {
        if (!first) {
            json.push_back(',');
        }
        first = false;

        json.append("{\"processId\":");
        json.append(std::to_string(process.processId));
        json.append(",\"parentProcessId\":");
        json.append(std::to_string(process.parentProcessId));
        json.append(",\"imageName\":\"");
        json.append(EscapeJson(process.imageName));
        json.push_back('"');
        json.push_back('}');
    }

    json.push_back(']');
    return json;
}

std::string BuildProcessListDetailedJson(const std::vector<ProcessEntryEx>& processes) {
    std::string json;
    json.reserve(processes.size() * 128 + 2);
    json.push_back('[');

    bool firstItem = true;

    for (const auto& process : processes) {
        if (!firstItem) {
            json.push_back(',');
        }
        firstItem = false;

        json.push_back('{');

        bool firstField = true;

        AppendJsonUInt(json, firstField, "processId", process.processId);
        AppendJsonUInt(json, firstField, "parentProcessId", process.parentProcessId);
        AppendJsonUInt(json, firstField, "threadCount", process.threadCount);
        AppendJsonInt(json, firstField, "basePriority", process.basePriority);
        AppendJsonString(json, firstField, "imageName", process.imageName);

        json.push_back('}');
    }

    json.push_back(']');
    return json;
}

std::string BuildProcessDetailJson(uint32_t processId) {
    ProcessEntryEx snapshot;
    DWORD snapshotError = 0;

    const bool found = FindProcessSnapshotById(
        static_cast<DWORD>(processId),
        snapshot,
        snapshotError
    );

    std::string json;
    json.reserve(1024);
    json.push_back('{');

    bool first = true;

    if (!found) {
        AppendJsonBool(json, first, "found", false);
        AppendJsonUInt(json, first, "processId", processId);
        AppendJsonUInt(json, first, "parentProcessId", 0);
        AppendJsonStringOrNull(json, first, "imageName", nullptr);
        AppendJsonUInt(json, first, "threadCount", 0);
        AppendJsonInt(json, first, "basePriority", 0);
        AppendJsonBool(json, first, "openSucceeded", false);
        AppendJsonUInt(json, first, "openErrorCode", 0);
        AppendJsonBool(json, first, "executablePathAvailable", false);
        AppendJsonStringOrNull(json, first, "executablePath", nullptr);
        AppendJsonBool(json, first, "creationTimeAvailable", false);
        AppendJsonStringOrNull(json, first, "creationTimeUtc", nullptr);
        AppendJsonUInt(json, first, "kernelTimeTicks", 0);
        AppendJsonUInt(json, first, "userTimeTicks", 0);
        AppendJsonUInt(json, first, "snapshotErrorCode", snapshotError);
        AppendJsonString(json, first, "errorMessage", "Process not found.");

        json.push_back('}');
        return json;
    }

    HANDLE processHandle = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_QUERY_LIMITED_INFORMATION,
        FALSE,
        static_cast<DWORD>(processId)
    );

    if (processHandle == nullptr) {
        processHandle = OpenProcess(
            PROCESS_QUERY_LIMITED_INFORMATION,
            FALSE,
            static_cast<DWORD>(processId)
        );
    }

    const bool openSucceeded = (processHandle != nullptr);
    const DWORD openErrorCode = openSucceeded ? 0 : GetLastError();

    std::string executablePath;
    bool executablePathAvailable = false;

    std::string creationTimeUtc;
    bool creationTimeAvailable = false;

    uint64_t kernelTimeTicks = 0;
    uint64_t userTimeTicks = 0;

    if (openSucceeded) {
        std::vector<wchar_t> pathBuffer(32768);
        DWORD pathLength = static_cast<DWORD>(pathBuffer.size());

        if (QueryFullProcessImageNameW(
                processHandle,
                0,
                pathBuffer.data(),
                &pathLength
            ) == TRUE) {
            executablePath = WideToUtf8(
                pathBuffer.data(),
                static_cast<size_t>(pathLength)
            );
            executablePathAvailable = true;
        }

        FILETIME creationTime{};
        FILETIME exitTime{};
        FILETIME kernelTime{};
        FILETIME userTime{};

        if (GetProcessTimes(
                processHandle,
                &creationTime,
                &exitTime,
                &kernelTime,
                &userTime
            ) == TRUE) {
            creationTimeUtc = FileTimeToIso8601Utc(creationTime);
            creationTimeAvailable = !creationTimeUtc.empty();

            ULARGE_INTEGER kernel;
            kernel.LowPart = kernelTime.dwLowDateTime;
            kernel.HighPart = kernelTime.dwHighDateTime;
            kernelTimeTicks = kernel.QuadPart;

            ULARGE_INTEGER user;
            user.LowPart = userTime.dwLowDateTime;
            user.HighPart = userTime.dwHighDateTime;
            userTimeTicks = user.QuadPart;
        }

        CloseHandle(processHandle);
    }

    AppendJsonBool(json, first, "found", true);
    AppendJsonUInt(json, first, "processId", snapshot.processId);
    AppendJsonUInt(json, first, "parentProcessId", snapshot.parentProcessId);
    AppendJsonString(json, first, "imageName", snapshot.imageName);
    AppendJsonUInt(json, first, "threadCount", snapshot.threadCount);
    AppendJsonInt(json, first, "basePriority", snapshot.basePriority);
    AppendJsonBool(json, first, "openSucceeded", openSucceeded);
    AppendJsonUInt(json, first, "openErrorCode", openErrorCode);
    AppendJsonBool(json, first, "executablePathAvailable", executablePathAvailable);
    AppendJsonStringOrNull(json, first, "executablePath",
        executablePathAvailable ? &executablePath : nullptr);
    AppendJsonBool(json, first, "creationTimeAvailable", creationTimeAvailable);
    AppendJsonStringOrNull(json, first, "creationTimeUtc",
        creationTimeAvailable ? &creationTimeUtc : nullptr);
    AppendJsonUInt(json, first, "kernelTimeTicks", kernelTimeTicks);
    AppendJsonUInt(json, first, "userTimeTicks", userTimeTicks);
    AppendJsonUInt(json, first, "snapshotErrorCode", 0);
    AppendJsonStringOrNull(json, first, "errorMessage", nullptr);

    json.push_back('}');
    return json;
}

} // namespace

extern "C" {

W32PH_API const char* W32PH_GetVersion(void) {
    try {
        return DuplicateStringToHeap("1.2.0");
    } catch (...) {
        return nullptr;
    }
}

W32PH_API int W32PH_EnumerateProcesses(
    W32PH_PROCESS_INFO** outProcesses,
    uint32_t* outCount
) {
    if (outProcesses == nullptr || outCount == nullptr) {
        return W32PH_E_INVALID_ARGUMENT;
    }

    *outProcesses = nullptr;
    *outCount = 0;

    try {
        std::vector<ProcessEntry> processes;
        DWORD lastError = 0;

        if (!SnapshotAllProcesses(processes, lastError)) {
            return W32PH_E_SNAPSHOT_FAILED;
        }

        if (processes.empty()) {
            return W32PH_OK;
        }

        const uint32_t count = static_cast<uint32_t>(processes.size());
        const size_t bytes = sizeof(W32PH_PROCESS_INFO) * static_cast<size_t>(count);

        W32PH_PROCESS_INFO* array = static_cast<W32PH_PROCESS_INFO*>(
            HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bytes)
        );

        if (array == nullptr) {
            return W32PH_E_OUT_OF_MEMORY;
        }

        uint32_t initialized = 0;

        for (; initialized < count; ++initialized) {
            array[initialized].processId =
                static_cast<uint32_t>(processes[initialized].processId);

            array[initialized].parentProcessId =
                static_cast<uint32_t>(processes[initialized].parentProcessId);

            array[initialized].imageName =
                DuplicateStringToHeap(processes[initialized].imageName.c_str());

            if (array[initialized].imageName == nullptr) {
                FreeProcessArrayInternal(array, initialized);
                return W32PH_E_OUT_OF_MEMORY;
            }
        }

        *outProcesses = array;
        *outCount = count;
        return W32PH_OK;
    } catch (...) {
        return W32PH_E_INTERNAL;
    }
}

W32PH_API void W32PH_FreeProcessList(
    W32PH_PROCESS_INFO* processes,
    uint32_t count
) {
    FreeProcessArrayInternal(processes, count);
}

W32PH_API int W32PH_EnumerateProcessesDetailed(
    W32PH_PROCESS_INFO_EX** outProcesses,
    uint32_t* outCount
) {
    if (outProcesses == nullptr || outCount == nullptr) {
        return W32PH_E_INVALID_ARGUMENT;
    }

    *outProcesses = nullptr;
    *outCount = 0;

    try {
        std::vector<ProcessEntryEx> processes;
        DWORD lastError = 0;

        if (!SnapshotAllProcessesDetailed(processes, lastError)) {
            return W32PH_E_SNAPSHOT_FAILED;
        }

        if (processes.empty()) {
            return W32PH_OK;
        }

        const uint32_t count = static_cast<uint32_t>(processes.size());
        const size_t bytes = sizeof(W32PH_PROCESS_INFO_EX) * static_cast<size_t>(count);

        W32PH_PROCESS_INFO_EX* array = static_cast<W32PH_PROCESS_INFO_EX*>(
            HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bytes)
        );

        if (array == nullptr) {
            return W32PH_E_OUT_OF_MEMORY;
        }

        uint32_t initialized = 0;

        for (; initialized < count; ++initialized) {
            array[initialized].processId =
                static_cast<uint32_t>(processes[initialized].processId);

            array[initialized].parentProcessId =
                static_cast<uint32_t>(processes[initialized].parentProcessId);

            array[initialized].threadCount =
                static_cast<uint32_t>(processes[initialized].threadCount);

            array[initialized].basePriority =
                static_cast<int32_t>(processes[initialized].basePriority);

            array[initialized].imageName =
                DuplicateStringToHeap(processes[initialized].imageName.c_str());

            if (array[initialized].imageName == nullptr) {
                FreeProcessArrayDetailedInternal(array, initialized);
                return W32PH_E_OUT_OF_MEMORY;
            }
        }

        *outProcesses = array;
        *outCount = count;
        return W32PH_OK;
    } catch (...) {
        return W32PH_E_INTERNAL;
    }
}

W32PH_API void W32PH_FreeProcessListDetailed(
    W32PH_PROCESS_INFO_EX* processes,
    uint32_t count
) {
    FreeProcessArrayDetailedInternal(processes, count);
}

W32PH_API const char* W32PH_GetProcessListJson(void) {
    try {
        std::vector<ProcessEntry> processes;
        DWORD lastError = 0;

        if (!SnapshotAllProcesses(processes, lastError)) {
            return nullptr;
        }

        const std::string json = BuildProcessListJson(processes);
        return DuplicateStringToHeap(json.c_str());
    } catch (...) {
        return nullptr;
    }
}

W32PH_API const char* W32PH_GetProcessListDetailedJson(void) {
    try {
        std::vector<ProcessEntryEx> processes;
        DWORD lastError = 0;

        if (!SnapshotAllProcessesDetailed(processes, lastError)) {
            return nullptr;
        }

        const std::string json = BuildProcessListDetailedJson(processes);
        return DuplicateStringToHeap(json.c_str());
    } catch (...) {
        return nullptr;
    }
}

W32PH_API const char* W32PH_GetProcessDetailJson(uint32_t processId) {
    try {
        const std::string json = BuildProcessDetailJson(processId);
        return DuplicateStringToHeap(json.c_str());
    } catch (...) {
        return nullptr;
    }
}

W32PH_API void W32PH_FreeString(const char* str) {
    if (str != nullptr) {
        HeapFree(GetProcessHeap(), 0, const_cast<char*>(str));
    }
}

} // extern "C"