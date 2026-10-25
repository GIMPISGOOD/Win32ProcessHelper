#pragma once

#include <stdint.h>

#ifdef W32PH_EXPORTS
#define W32PH_API __declspec(dllexport)
#else
#define W32PH_API __declspec(dllimport)
#endif

#define W32PH_OK 0
#define W32PH_E_INVALID_ARGUMENT (-1)
#define W32PH_E_OUT_OF_MEMORY (-2)
#define W32PH_E_SNAPSHOT_FAILED (-3)
#define W32PH_E_INTERNAL (-4)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct W32PH_PROCESS_INFO {
    uint32_t processId;
    uint32_t parentProcessId;
    char* imageName;
} W32PH_PROCESS_INFO;

// 获取 DLL 版本。
// 返回字符串由 DLL 分配，调用方必须调用 W32PH_FreeString 释放。
W32PH_API const char* W32PH_GetVersion(void);

// 枚举当前系统进程列表。
// 成功返回 W32PH_OK。
// outProcesses 由 DLL 分配，调用方必须调用 W32PH_FreeProcessList 释放。
W32PH_API int W32PH_EnumerateProcesses(
    W32PH_PROCESS_INFO** outProcesses,
    uint32_t* outCount
);

// 释放 W32PH_EnumerateProcesses 返回的进程数组。
W32PH_API void W32PH_FreeProcessList(
    W32PH_PROCESS_INFO* processes,
    uint32_t count
);

// 获取进程列表 JSON。
// 返回字符串由 DLL 分配，调用方必须调用 W32PH_FreeString 释放。
// 失败时返回 NULL。
W32PH_API const char* W32PH_GetProcessListJson(void);

// 释放由 DLL 分配的字符串内存。
W32PH_API void W32PH_FreeString(const char* str);

#ifdef __cplusplus
}
#endif