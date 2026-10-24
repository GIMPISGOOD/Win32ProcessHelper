#include "wph/wph.h"
#include "../util/privilege.h"
#include "../core/process_enumerator.h"
#include <objbase.h> // For CoTaskMemAlloc
#include <string>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) DisableThreadLibraryCalls(hModule);
    return TRUE;
}

extern "C" {
    WPH_API const char* wph_get_version() { return "1.0.1-alpha"; }

    WPH_API int wph_initialize() {
        // 尝试提升权限，以便读取更多受保护的进程信息
        wph::util::EnableDebugPrivilege();
        return 0; 
    }

    WPH_API void wph_shutdown() {}

    WPH_API char* wph_get_processes_json() {
        auto processes = wph::core::EnumerateProcesses();
        std::string json = wph::core::GenerateProcessesJson(processes);
        
        // 使用 COM 内存分配器，确保跨 DLL/EXE 边界释放安全
        size_t len = json.length() + 1;
        char* buffer = (char*)CoTaskMemAlloc(len);
        if (buffer) {
            memcpy(buffer, json.c_str(), len);
        }
        return buffer;
    }

    WPH_API void wph_free_string(char* str) {
        if (str) CoTaskMemFree(str);
    }
}