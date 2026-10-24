cmake_minimum_required(VERSION 3.20)
project(Win32ProcessHelper VERSION 1.0.0 LANGUAGES CXX)

# 设置 C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 解决 MSVC 编译中文乱码问题，强制使用 UTF-8
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# 设置静态链接 CRT (Release 使用 /MT, Debug 使用 /MTd)
# 这样生成的 exe 和 dll 就不依赖目标机器上的 vcruntime140.dll
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# 统一输出目录，确保 exe 和 dll 在同一个文件夹，避免运行时找不到 dll
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 包含公共头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# ==========================================
# 1. 构建 DLL (Win32ProcessHelper.dll)
# ==========================================
add_library(Win32ProcessHelper_dll SHARED
    src/dll/dllmain.cpp
    # 后续在这里添加 src/core, src/detect 等模块的 .cpp 文件
)
set_target_properties(Win32ProcessHelper_dll PROPERTIES OUTPUT_NAME "Win32ProcessHelper")
target_compile_definitions(Win32ProcessHelper_dll PRIVATE WPH_EXPORTS)

# ==========================================
# 2. 构建 EXE (Win32ProcessHelper.exe)
# ==========================================
add_executable(Win32ProcessHelper_exe
    src/app/main.cpp
)
set_target_properties(Win32ProcessHelper_exe PROPERTIES OUTPUT_NAME "Win32ProcessHelper")
target_link_libraries(Win32ProcessHelper_exe PRIVATE Win32ProcessHelper_dll)