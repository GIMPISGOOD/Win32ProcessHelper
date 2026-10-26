#include <windows.h>
#include <iostream>
#include <string>
#include <cstdint>

#include "commands/ProcessCommands.h"

namespace {

enum class Command {
    None,
    BasicProcessEnumeration,
    DetailedProcessEnumeration,
    ProcessDetail
};

struct CliOptions {
    Command command = Command::None;
    uint32_t processId = 0;
    bool hasProcessId = false;
    bool json = false;
};

void SetupConsoleUtf8() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
}

void PrintUsage() {
    std::cout <<
        "Win32ProcessHelper.exe <command> [options]\n"
        "\n"
        "Commands:\n"
        "  --BasicProcessEnumeration      Enumerate basic process list\n"
        "  --DetailedProcessEnumeration   Enumerate detailed process list\n"
        "  --ProcessDetail                Show detailed information for one process. Output is JSON.\n"
        "\n"
        "Options:\n"
        "  --ProcessId <pid>              Target process id for --ProcessDetail\n"
        "  --Json                         Output JSON for list commands\n"
        "\n"
        "Examples:\n"
        "  Win32ProcessHelper.exe --BasicProcessEnumeration\n"
        "  Win32ProcessHelper.exe --DetailedProcessEnumeration\n"
        "  Win32ProcessHelper.exe --DetailedProcessEnumeration --Json\n"
        "  Win32ProcessHelper.exe --ProcessDetail --ProcessId 4\n";
}

bool ParseUInt32(const std::string& text, uint32_t& out) {
    if (text.empty()) {
        return false;
    }

    unsigned long long value = 0;

    for (char c : text) {
        if (c < '0' || c > '9') {
            return false;
        }

        const unsigned long long digit = static_cast<unsigned long long>(c - '0');

        if (value > (0xFFFFFFFFull - digit) / 10) {
            return false;
        }

        value = value * 10 + digit;
    }

    out = static_cast<uint32_t>(value);
    return true;
}

bool ParseCommandLine(
    int argc,
    char* argv[],
    CliOptions& options,
    std::string& error
) {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (arg == "--BasicProcessEnumeration") {
            if (options.command != Command::None) {
                error = "Multiple commands are not supported.";
                return false;
            }
            options.command = Command::BasicProcessEnumeration;
        } else if (arg == "--DetailedProcessEnumeration") {
            if (options.command != Command::None) {
                error = "Multiple commands are not supported.";
                return false;
            }
            options.command = Command::DetailedProcessEnumeration;
        } else if (arg == "--ProcessDetail") {
            if (options.command != Command::None) {
                error = "Multiple commands are not supported.";
                return false;
            }
            options.command = Command::ProcessDetail;
        } else if (arg == "--Json") {
            options.json = true;
        } else if (arg == "--ProcessId") {
            if (i + 1 >= argc) {
                error = "Missing value for --ProcessId.";
                return false;
            }

            const std::string value = argv[++i];

            if (!ParseUInt32(value, options.processId)) {
                error = "Invalid value for --ProcessId. Expected a decimal process id.";
                return false;
            }

            options.hasProcessId = true;
        } else {
            error = "Unknown parameter: " + arg;
            return false;
        }
    }

    return true;
}

} // namespace

int main(int argc, char* argv[]) {
    SetupConsoleUtf8();

    CliOptions options;
    std::string error;

    if (!ParseCommandLine(argc, argv, options, error)) {
        std::cerr << error << "\n\n";
        PrintUsage();
        return 1;
    }

    if (options.command == Command::None) {
        PrintUsage();
        return 1;
    }

    if (options.command == Command::ProcessDetail && !options.hasProcessId) {
        std::cerr << "--ProcessDetail requires --ProcessId <pid>.\n\n";
        PrintUsage();
        return 1;
    }

    if (options.command != Command::ProcessDetail && options.hasProcessId) {
        std::cerr << "--ProcessId can only be used with --ProcessDetail.\n\n";
        PrintUsage();
        return 1;
    }

    switch (options.command) {
        case Command::BasicProcessEnumeration:
            return w32ph::cli::commands::RunBasicProcessEnumeration(options.json);

        case Command::DetailedProcessEnumeration:
            return w32ph::cli::commands::RunDetailedProcessEnumeration(options.json);

        case Command::ProcessDetail:
            return w32ph::cli::commands::RunProcessDetail(options.processId);

        default:
            PrintUsage();
            return 1;
    }
}