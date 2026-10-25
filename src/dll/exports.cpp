#define W32PH_EXPORTS
#include "Win32ProcessHelper.h"
#include <windows.h>
#include <string.h>

// 内部辅助函数：使用进程堆分配内存，确保跨模块安全
static char* DuplicateStringToHeap(const char* src) {
    if (!src) return nullptr;
    size_t len = strlen(src) + 1;
    char* dst = (char*)HeapAlloc(GetProcessHeap(), 0, len);
    if (dst) {
        memcpy(dst, src, len);
    }
    return dst;
}

extern "C" {

W32PH_API const char* W32PH_GetVersion(void) {
    try {
        const char* version = "1.0.0";
        return DuplicateStringToHeap(version);
    } catch (...) {
        return nullptr;
    }
}

W32PH_API void W32PH_FreeString(const char* str) {
    if (str) {
        HeapFree(GetProcessHeap(), 0, (LPVOID)str);
    }
}

} // extern "C"