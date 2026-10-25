
# Win32ProcessHelper

Windows 用户态进程管理工具。

## 项目结构

- `include/`: DLL 导出头文件
- `src/dll/`: DLL 核心实现
- `src/exe/`: CLI 薄壳程序
- `examples/`: C# 和 Python 调用示例
- `build/`: 构建输出目录

## 构建方法

需要安装 Visual Studio 2022 并包含 C++ 桌面开发组件，以及 CMake (3.15+)。

1. 打开 "x64 Native Tools Command Prompt for VS 2022"
2. 执行以下命令：

```cmd
cd C:\Users\60401\CPlusProjects\Win32ProcessHelper
cmake -S . -B build\nmake-x64 -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build\nmake-x64
```
