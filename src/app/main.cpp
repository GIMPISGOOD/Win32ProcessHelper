#include "wph/wph.h"
#include <iostream>
#include <windows.h>
#include <vector>
#include <iomanip>

void SetupConsole() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
}

int main() {
    SetupConsole();
    
    std::cout << "=====================================" << std::endl;
    std::cout << " Win32ProcessHelper v" << wph_get_version() << std::endl;
    std::cout << "=====================================" << std::endl;

    if (wph_initialize() != 0) {
        std::cerr << "[-] 初始化失败。" << std::endl;
        return 1;
    }

    std::cout << "[*] 正在枚举进程并采集详细信息..." << std::endl;
    char* json_str = wph_get_processes_json();
    
    if (json_str) {
        // 为了演示，这里直接输出部分 JSON，实际产品中 EXE 可以解析 JSON 并打印漂亮的表格
        // 或者直接将 JSON 导出给 Python/C# 脚本使用
        std::string json(json_str);
        
        std::cout << "\n[+] 采集完成！返回的 JSON 数据长度: " << json.length() << " bytes" << std::endl;
        std::cout << "-------------------------------------" << std::endl;
        std::cout << "JSON Preview (First 500 chars):" << std::endl;
        std::cout << json.substr(0, 500) << "..." << std::endl;
        std::cout << "-------------------------------------" << std::endl;
        
        wph_free_string(json_str);
    } else {
        std::cerr << "[-] 获取进程列表失败。" << std::endl;
    }

    wph_shutdown();
    return 0;
}