cmake_minimum_required(VERSION 3.20)
project(Win32ProcessHelper VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# ==========================================
# 1. 构建 DLL (Win32ProcessHelper.dll)
# ==========================================
add_library(Win32ProcessHelper_dll SHARED
    src/dll/dllmain.cpp
    src/util/privilege.cpp
    src/util/json_util.cpp
    src/core/process_enumerator.cpp
)
set_target_properties(Win32ProcessHelper_dll PROPERTIES OUTPUT_NAME "Win32ProcessHelper")
target_compile_definitions(Win32ProcessHelper_dll PRIVATE WPH_EXPORTS)
# 链接必要的 Windows 库
target_link_libraries(Win32ProcessHelper_dll PRIVATE ntdll) 

# ==========================================
# 2. 构建 EXE (Win32ProcessHelper.exe)
# ==========================================
add_executable(Win32ProcessHelper_exe
    src/app/main.cpp
)
set_target_properties(Win32ProcessHelper_exe PROPERTIES OUTPUT_NAME "Win32ProcessHelper")
target_link_libraries(Win32ProcessHelper_exe PRIVATE Win32ProcessHelper_dll)