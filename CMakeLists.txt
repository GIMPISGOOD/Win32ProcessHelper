cmake_minimum_required(VERSION 3.15)
project(Win32ProcessHelper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 静态 CRT：/MT
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# 警告与标准符合性
add_compile_options(/W4 /permissive-)
if(MSVC)
    add_compile_options(/utf-8)
endif()

# 通用 Windows 宏
add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN UNICODE _UNICODE)

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 为了方便后续扩展，DLL/EXE 源码按目录自动收集。
# 新增 .cpp 文件后建议重新执行一次 cmake 配置。
file(GLOB_RECURSE W32PH_DLL_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/dll/*.cpp")
file(GLOB_RECURSE W32PH_EXE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/exe/*.cpp")

# 核心 DLL
add_library(Win32ProcessHelper SHARED ${W32PH_DLL_SOURCES})
target_include_directories(Win32ProcessHelper PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(Win32ProcessHelper PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/dll)
target_compile_definitions(Win32ProcessHelper PRIVATE W32PH_EXPORTS)

# CLI EXE 薄壳
add_executable(Win32ProcessHelperCLI ${W32PH_EXE_SOURCES})
target_include_directories(Win32ProcessHelperCLI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Win32ProcessHelperCLI PRIVATE Win32ProcessHelper)
set_target_properties(Win32ProcessHelperCLI PROPERTIES OUTPUT_NAME Win32ProcessHelper)