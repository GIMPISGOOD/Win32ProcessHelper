#include <windows.h>

#include <iostream>
#include <string>

#include "commands/ProcessCommands.h"

namespace {

void SetupConsoleUtf8() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
}

void PrintUsage() {
    std::cout <<
        "Win32ProcessHelper.exe <parameter>\n\n"
        "Parameters:\n"
        "  --BasicProcessEnumeration    Enumerate basic process list\n";
}

} // namespace

int main(int argc, char* argv[]) {
    SetupConsoleUtf8();

    if (argc < 2) {
        PrintUsage();
        return 1;
    }

    const std::string parameter = argv[1];

    if (parameter == "--BasicProcessEnumeration") {
        return w32ph::cli::commands::RunBasicProcessEnumeration();
    }

    std::cerr << "Unknown parameter: " << parameter << "\n\n";
    PrintUsage();
    return 1;
}