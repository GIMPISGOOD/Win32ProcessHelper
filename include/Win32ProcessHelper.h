#pragma once

#ifdef W32PH_EXPORTS
#define W32PH_API __declspec(dllexport)
#else
#define W32PH_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 获取 DLL 版本号
 * @return 返回 UTF-8 编码的版本字符串。调用方必须使用 W32PH_FreeString 释放。
 */
W32PH_API const char* W32PH_GetVersion(void);

/**
 * @brief 释放由 DLL 分配的字符串内存
 * @param str 需要释放的字符串指针
 */
W32PH_API void W32PH_FreeString(const char* str);

#ifdef __cplusplus
}
#endif