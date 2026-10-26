#include "commands/ProcessCommands.h"
#include "Win32ProcessHelper.h"

#include <iostream>

namespace {

const char* GetErrorMessage(int errorCode) {
    switch (errorCode) {
        case W32PH_E_INVALID_ARGUMENT:
            return "Invalid argument.";
        case W32PH_E_OUT_OF_MEMORY:
            return "Out of memory.";
        case W32PH_E_SNAPSHOT_FAILED:
            return "Failed to create process snapshot.";
        case W32PH_E_INTERNAL:
            return "Internal error.";
        default:
            return "Unknown error.";
    }
}

} // namespace

namespace w32ph::cli::commands {

int RunBasicProcessEnumeration(bool json) {
    if (json) {
        const char* text = W32PH_GetProcessListJson();

        if (text == nullptr) {
            std::cout << "{\"ok\":false,\"errorMessage\":\"Failed to get basic process list JSON.\"}\n";
            return 1;
        }

        std::cout << text << '\n';
        W32PH_FreeString(text);
        return 0;
    }

    W32PH_PROCESS_INFO* processes = nullptr;
    uint32_t count = 0;

    const int result = W32PH_EnumerateProcesses(&processes, &count);

    if (result != W32PH_OK) {
        std::cerr << "Failed to enumerate processes. Error code: "
                  << result
                  << " (" << GetErrorMessage(result) << ")\n";
        return 1;
    }

    std::cout << "PID\tPPID\tImageName\n";

    for (uint32_t i = 0; i < count; ++i) {
        const W32PH_PROCESS_INFO& process = processes[i];

        std::cout << process.processId
                  << '\t' << process.parentProcessId
                  << '\t' << (process.imageName != nullptr ? process.imageName : "")
                  << '\n';
    }

    W32PH_FreeProcessList(processes, count);
    return 0;
}

int RunDetailedProcessEnumeration(bool json) {
    if (json) {
        const char* text = W32PH_GetProcessListDetailedJson();

        if (text == nullptr) {
            std::cout << "{\"ok\":false,\"errorMessage\":\"Failed to get detailed process list JSON.\"}\n";
            return 1;
        }

        std::cout << text << '\n';
        W32PH_FreeString(text);
        return 0;
    }

    W32PH_PROCESS_INFO_EX* processes = nullptr;
    uint32_t count = 0;

    const int result = W32PH_EnumerateProcessesDetailed(&processes, &count);

    if (result != W32PH_OK) {
        std::cerr << "Failed to enumerate detailed processes. Error code: "
                  << result
                  << " (" << GetErrorMessage(result) << ")\n";
        return 1;
    }

    std::cout << "PID\tPPID\tThreads\tPriority\tImageName\n";

    for (uint32_t i = 0; i < count; ++i) {
        const W32PH_PROCESS_INFO_EX& process = processes[i];

        std::cout << process.processId
                  << '\t' << process.parentProcessId
                  << '\t' << process.threadCount
                  << '\t' << process.basePriority
                  << '\t' << (process.imageName != nullptr ? process.imageName : "")
                  << '\n';
    }

    W32PH_FreeProcessListDetailed(processes, count);
    return 0;
}

int RunProcessDetail(uint32_t processId) {
    const char* text = W32PH_GetProcessDetailJson(processId);

    if (text == nullptr) {
        std::cout << "{\"found\":false,\"processId\":"
                  << processId
                  << ",\"errorMessage\":\"Failed to build process detail JSON.\"}\n";
        return 1;
    }

    std::cout << text << '\n';
    W32PH_FreeString(text);
    return 0;
}

} // namespace w32ph::cli::commands