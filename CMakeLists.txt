cmake_minimum_required(VERSION 3.15)
project(Win32ProcessHelper LANGUAGES CXX)

# 设置 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 强制使用静态 CRT (/MT)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# 编译选项：/W4 和 /permissive-
add_compile_options(/W4 /permissive-)

# 统一输出目录到 bin/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 1. 核心 DLL
add_library(Win32ProcessHelper SHARED
    src/dll/dllmain.cpp
    src/dll/exports.cpp
)
target_include_directories(Win32ProcessHelper PUBLIC include)
target_compile_definitions(Win32ProcessHelper PRIVATE W32PH_EXPORTS)

# 2. CLI EXE (薄壳)
add_executable(Win32ProcessHelperCLI
    src/exe/main.cpp
)
target_link_libraries(Win32ProcessHelperCLI PRIVATE Win32ProcessHelper)
# 将 EXE 输出名称设为 Win32ProcessHelper.exe
set_target_properties(Win32ProcessHelperCLI PROPERTIES OUTPUT_NAME Win32ProcessHelper)