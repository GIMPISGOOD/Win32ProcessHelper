#pragma once
#include <windows.h>

#ifdef WPH_EXPORTS
#define WPH_API __declspec(dllexport)
#else
#define WPH_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

WPH_API const char* wph_get_version();
WPH_API int wph_initialize();
WPH_API void wph_shutdown();

// 核心功能：获取所有进程的详细信息，返回 UTF-8 编码的 JSON 字符串
// 调用者必须使用 wph_free_string 释放返回的内存
WPH_API char* wph_get_processes_json();

// 释放由 DLL 分配的字符串内存 (跨模块安全)
WPH_API void wph_free_string(char* str);

#ifdef __cplusplus
}
#endif