#include "wph/wph.h"
#include <windows.h>

// DLL 入口点
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            // DLL 被加载到进程时执行（尽量避免在此处做耗时操作）
            DisableThreadLibraryCalls(hModule);
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

extern "C" {
    WPH_API const char* wph_get_version() {
        return "1.0.0-alpha";
    }

    WPH_API int wph_initialize() {
        // 预留：后续在这里初始化核心模块、日志系统等
        return 0; // 0 表示成功
    }

    WPH_API void wph_shutdown() {
        // 预留：后续在这里释放资源、关闭句柄等
    }
}