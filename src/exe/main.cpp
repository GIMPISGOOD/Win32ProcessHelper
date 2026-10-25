#include <windows.h>
#include <iostream>
#include <string>
#include "Win32ProcessHelper.h"

void SetupConsoleUTF8() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
}

int main(int argc, char* argv[]) {
    SetupConsoleUTF8();

    if (argc < 2) {
        std::cout << "Usage: Win32ProcessHelper.exe <command> [args...]\n";
        std::cout << "Commands:\n";
        std::cout << "  version    Show DLL version\n";
        return 1;
    }

    std::string cmd = argv[1];

    if (cmd == "version") {
        const char* ver = W32PH_GetVersion();
        if (ver) {
            std::cout << "Win32ProcessHelper DLL Version: " << ver << "\n";
            W32PH_FreeString(ver);
        } else {
            std::cerr << "Error: Failed to get version from DLL.\n";
            return 1;
        }
    } else {
        std::cerr << "Unknown command: " << cmd << "\n";
        return 1;
    }

    return 0;
}