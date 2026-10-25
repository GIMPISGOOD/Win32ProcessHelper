#include "Win32ProcessHelper.h"

#include "common/Json.h"
#include "common/Memory.h"
#include "process/ProcessSnapshot.h"

#include <windows.h>

#include <string>
#include <vector>

using w32ph::common::DuplicateStringToHeap;
using w32ph::common::EscapeJson;
using w32ph::process::ProcessEntry;
using w32ph::process::SnapshotAllProcesses;

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

} // namespace

extern "C" {

W32PH_API const char* W32PH_GetVersion(void) {
    try {
        return DuplicateStringToHeap("1.1.0");
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

W32PH_API void W32PH_FreeString(const char* str) {
    if (str != nullptr) {
        HeapFree(GetProcessHeap(), 0, const_cast<char*>(str));
    }
}

} // extern "C"