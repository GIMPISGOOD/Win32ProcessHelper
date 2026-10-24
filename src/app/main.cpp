#include "wph/wph.h"
#include <iostream>
#include <windows.h>

// 设置控制台输出为 UTF-8，防止中文乱码
void SetupConsole() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
}

int main(int argc, char* argv[]) {
    SetupConsole();
    
    std::cout << "=====================================" << std::endl;
    std::cout << " Win32ProcessHelper CLI Tool" << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cout << "DLL Version: " << wph_get_version() << std::endl;

    int init_result = wph_initialize();
    if (init_result == 0) {
        std::cout << "[+] 初始化成功。" << std::endl;
        
        // TODO: 在这里解析命令行参数并调用业务逻辑
        
    } else {
        std::cerr << "[-] 初始化失败，错误码: " << init_result << std::endl;
        return 1;
    }

    // std::cout << "按回车键退出..." << std::endl;
    // std::cin.get();

    wph_shutdown();
    return 0;
}