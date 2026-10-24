#pragma once

// 定义导出/导入宏
#ifdef WPH_EXPORTS
#define WPH_API __declspec(dllexport)
#else
#define WPH_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

// 获取 DLL 版本
WPH_API const char* wph_get_version();

// 初始化工具
WPH_API int wph_initialize();

// 清理与关闭
WPH_API void wph_shutdown();

#ifdef __cplusplus
}
#endif