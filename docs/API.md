# Win32ProcessHelper API Reference

本文档描述 `Win32ProcessHelper.dll` 当前对外提供的 C API。

当前版本：`1.1.0`

---

## 目录

- [1. 简介](#1-简介)
- [2. 头文件与集成方式](#2-头文件与集成方式)
- [3. 内存管理规则](#3-内存管理规则)
- [4. 错误码](#4-错误码)
- [5. 数据结构](#5-数据结构)
- [6. 导出函数](#6-导出函数)
  - [W32PH_GetVersion](#w32ph_getversion)
  - [W32PH_EnumerateProcesses](#w32ph_enumerateprocesses)
  - [W32PH_FreeProcessList](#w32ph_freeprocesslist)
  - [W32PH_GetProcessListJson](#w32ph_getprocesslistjson)
  - [W32PH_FreeString](#w32ph_freestring)
- [7. JSON 输出格式](#7-json-输出格式)
- [8. C/C++ 调用示例](#8-cc-调用示例)
- [9. Python 调用示例](#9-python-调用示例)
- [10. C# 调用示例](#10-c-调用示例)
- [11. CLI 使用方式](#11-cli-使用方式)
- [12. 注意事项](#12-注意事项)

---

## 1. 简介

`Win32ProcessHelper` 是一个 Windows 用户态进程管理工具库。

当前版本提供以下能力：

- 枚举当前系统中的进程列表；
- 返回进程 ID、父进程 ID、进程映像名；
- 提供 C 结构体数组接口；
- 提供 JSON 字符串接口；
- 提供配套释放函数；
- 提供简单 CLI 工具。

该 DLL 使用 C 风格 ABI，便于从 C、C++、Python、C# 等语言调用。

---

## 2. 头文件与集成方式

公共头文件为：

```cpp
#include "Win32ProcessHelper.h"
```

链接时需要链接：

```text
Win32ProcessHelper.lib
```

运行时需要在可执行文件搜索路径中找到：

```text
Win32ProcessHelper.dll
```

DLL 编译时使用宏：

```cpp
W32PH_EXPORTS
```

调用方不需要定义该宏。

头文件中导出宏定义如下：

```cpp
#ifdef W32PH_EXPORTS
#define W32PH_API __declspec(dllexport)
#else
#define W32PH_API __declspec(dllimport)
#endif
```

---

## 3. 内存管理规则

`Win32ProcessHelper.dll` 返回的所有动态内存都由 DLL 内部分配。

调用方不能直接使用 `free`、`delete`、`LocalFree`、`CoTaskMemFree` 等函数释放这些内存。

必须使用本库提供的释放函数。

| 函数 | 返回内容 | 释放方式 |
|---|---|
| `W32PH_GetVersion` | `const char*` | `W32PH_FreeString` |
| `W32PH_GetProcessListJson` | `const char*` | `W32PH_FreeString` |
| `W32PH_EnumerateProcesses` | `W32PH_PROCESS_INFO*` | `W32PH_FreeProcessList` |

错误示例：

```cpp
const char* json = W32PH_GetProcessListJson();
free((void*)json); // 错误
```

正确示例：

```cpp
const char* json = W32PH_GetProcessListJson();
if (json != nullptr) {
    // use json
    W32PH_FreeString(json);
}
```

---

## 4. 错误码

以下错误码由 `W32PH_EnumerateProcesses` 返回：

```cpp
#define W32PH_OK 0
#define W32PH_E_INVALID_ARGUMENT (-1)
#define W32PH_E_OUT_OF_MEMORY (-2)
#define W32PH_E_SNAPSHOT_FAILED (-3)
#define W32PH_E_INTERNAL (-4)
```

| 错误码 | 值 | 说明 |
|---|---:|---|
| `W32PH_OK` | `0` | 调用成功 |
| `W32PH_E_INVALID_ARGUMENT` | `-1` | 参数无效，通常是传入了空指针 |
| `W32PH_E_OUT_OF_MEMORY` | `-2` | 内存分配失败 |
| `W32PH_E_SNAPSHOT_FAILED` | `-3` | 创建进程快照或枚举进程失败 |
| `W32PH_E_INTERNAL` | `-4` | 内部错误，通常是未预期的 C++ 异常 |

---

## 5. 数据结构

### `W32PH_PROCESS_INFO`

定义如下：

```cpp
typedef struct W32PH_PROCESS_INFO {
    uint32_t processId;
    uint32_t parentProcessId;
    char* imageName;
} W32PH_PROCESS_INFO;
```

| 字段 | 类型 | 说明 |
|---|---|---|
| `processId` | `uint32_t` | 进程 ID |
| `parentProcessId` | `uint32_t` | 父进程 ID |
| `imageName` | `char*` | 进程映像名，UTF-8 编码字符串 |

说明：

- `imageName` 由 DLL 分配。
- 调用方不应修改该字符串。
- 调用 `W32PH_FreeProcessList` 后，该数组及其中所有 `imageName` 都不应再被访问。

---

## 6. 导出函数

---

### `W32PH_GetVersion`

```cpp
W32PH_API const char* W32PH_GetVersion(void);
```

获取 DLL 版本字符串。

#### 返回值

| 返回值 | 说明 |
|---|---|
| 非 `NULL` | 版本字符串，UTF-8 编码 |
| `NULL` | 获取失败，通常是内存分配失败 |

#### 内存释放

返回值必须使用以下函数释放：

```cpp
W32PH_FreeString
```

#### 示例

```cpp
const char* version = W32PH_GetVersion();

if (version != nullptr) {
    printf("Version: %s\n", version);
    W32PH_FreeString(version);
}
```

---

### `W32PH_EnumerateProcesses`

```cpp
W32PH_API int W32PH_EnumerateProcesses(
    W32PH_PROCESS_INFO** outProcesses,
    uint32_t* outCount
);
```

枚举当前系统中的进程列表。

#### 参数

| 参数 | 类型 | 说明 |
|---|---|---|
| `outProcesses` | `W32PH_PROCESS_INFO**` | 接收进程数组指针 |
| `outCount` | `uint32_t*` | 接收进程数量 |

两个参数都不能为 `NULL`。

#### 返回值

| 返回值 | 说明 |
|---|---|
| `W32PH_OK` | 成功 |
| `W32PH_E_INVALID_ARGUMENT` | 参数为空 |
| `W32PH_E_OUT_OF_MEMORY` | 内存分配失败 |
| `W32PH_E_SNAPSHOT_FAILED` | 进程快照失败 |
| `W32PH_E_INTERNAL` | 内部错误 |

#### 行为说明

调用成功后：

- `*outProcesses` 指向由 DLL 分配的进程数组；
- `*outCount` 表示数组元素数量；
- 如果系统中没有可返回的进程，函数仍可能返回 `W32PH_OK`，此时 `*outProcesses` 为 `NULL`，`*outCount` 为 `0`。

调用失败后：

- `*outProcesses` 会被设置为 `NULL`；
- `*outCount` 会被设置为 `0`。

#### 内存释放

成功返回后，必须调用：

```cpp
W32PH_FreeProcessList(*outProcesses, *outCount);
```

释放内存。

#### 示例

```cpp
W32PH_PROCESS_INFO* processes = nullptr;
uint32_t count = 0;

int result = W32PH_EnumerateProcesses(&processes, &count);

if (result == W32PH_OK) {
    for (uint32_t i = 0; i < count; ++i) {
        printf(
            "%u %u %s\n",
            processes[i].processId,
            processes[i].parentProcessId,
            processes[i].imageName ? processes[i].imageName : ""
        );
    }

    W32PH_FreeProcessList(processes, count);
} else {
    printf("Failed, error code: %d\n", result);
}
```

---

### `W32PH_FreeProcessList`

```cpp
W32PH_API void W32PH_FreeProcessList(
    W32PH_PROCESS_INFO* processes,
    uint32_t count
);
```

释放由 `W32PH_EnumerateProcesses` 返回的进程数组。

#### 参数

| 参数 | 类型 | 说明 |
|---|---|---|
| `processes` | `W32PH_PROCESS_INFO*` | 进程数组指针 |
| `count` | `uint32_t` | 进程数组长度 |

#### 说明

该函数会释放：

- 每个进程项中的 `imageName`；
- 进程数组本身。

如果 `processes` 为 `NULL`，函数直接返回。

调用该函数后，不应继续访问该数组。

#### 示例

```cpp
W32PH_FreeProcessList(processes, count);
processes = nullptr;
count = 0;
```

---

### `W32PH_GetProcessListJson`

```cpp
W32PH_API const char* W32PH_GetProcessListJson(void);
```

获取当前进程列表的 JSON 字符串。

#### 返回值

| 返回值 | 说明 |
|---|---|
| 非 `NULL` | JSON 字符串，UTF-8 编码 |
| `NULL` | 获取失败 |

#### 内存释放

返回值必须使用以下函数释放：

```cpp
W32PH_FreeString
```

#### 示例

```cpp
const char* json = W32PH_GetProcessListJson();

if (json != nullptr) {
    printf("%s\n", json);
    W32PH_FreeString(json);
} else {
    printf("Failed to get process list JSON.\n");
}
```

---

### `W32PH_FreeString`

```cpp
W32PH_API void W32PH_FreeString(const char* str);
```

释放由 DLL 分配的字符串内存。

#### 参数

| 参数 | 类型 | 说明 |
|---|---|---|
| `str` | `const char*` | 由 DLL 返回的字符串指针 |

#### 适用对象

该函数用于释放以下函数返回的字符串：

- `W32PH_GetVersion`
- `W32PH_GetProcessListJson`

#### 说明

如果 `str` 为 `NULL`，函数直接返回。

调用该函数后，不应继续访问该字符串。

#### 示例

```cpp
const char* version = W32PH_GetVersion();

if (version != nullptr) {
    W32PH_FreeString(version);
    version = nullptr;
}
```

---

## 7. JSON 输出格式

`W32PH_GetProcessListJson` 返回一个 JSON 数组。

数组中每个元素表示一个进程。

### 字段说明

| 字段 | 类型 | 说明 |
|---|---|---|
| `processId` | number | 进程 ID |
| `parentProcessId` | number | 父进程 ID |
| `imageName` | string | 进程映像名，UTF-8 编码 |

### 示例输出

```json
[
  {
    "processId": 4,
    "parentProcessId": 0,
    "imageName": "System"
  },
  {
    "processId": 892,
    "parentProcessId": 556,
    "imageName": "svchost.exe"
  },
  {
    "processId": 1234,
    "parentProcessId": 5678,
    "imageName": "example.exe"
  }
]
```

### 转义规则

`imageName` 字段会进行 JSON 转义。

以下字符会被转义：

| 原始字符 | JSON 转义 |
|---|---|
| `"` | `\"` |
| `\` | `\\` |
| 退格 | `\b` |
| 换页 | `\f` |
| 换行 | `\n` |
| 回车 | `\r` |
| 制表符 | `\t` |
| 其他小于 `0x20` 的控制字符 | `\u00XX` |

---

## 8. C/C++ 调用示例

### 结构体接口示例

```cpp
#include <cstdio>
#include "Win32ProcessHelper.h"

int main() {
    W32PH_PROCESS_INFO* processes = nullptr;
    uint32_t count = 0;

    int result = W32PH_EnumerateProcesses(&processes, &count);

    if (result != W32PH_OK) {
        printf("Failed to enumerate processes: %d\n", result);
        return 1;
    }

    printf("PID\tPPID\tImageName\n");

    for (uint32_t i = 0; i < count; ++i) {
        const W32PH_PROCESS_INFO& process = processes[i];

        printf(
            "%u\t%u\t%s\n",
            process.processId,
            process.parentProcessId,
            process.imageName ? process.imageName : ""
        );
    }

    W32PH_FreeProcessList(processes, count);

    return 0;
}
```

### JSON 接口示例

```cpp
#include <cstdio>
#include "Win32ProcessHelper.h"

int main() {
    const char* json = W32PH_GetProcessListJson();

    if (json == nullptr) {
        printf("Failed to get process list JSON.\n");
        return 1;
    }

    printf("%s\n", json);

    W32PH_FreeString(json);

    return 0;
}
```

---

## 9. Python 调用示例

项目中已有示例：

```text
examples/python/process_list_json.py
```

核心调用方式如下：

```python
import ctypes

dll = ctypes.CDLL("Win32ProcessHelper.dll")

dll.W32PH_GetProcessListJson.restype = ctypes.c_void_p
dll.W32PH_GetProcessListJson.argtypes = []

dll.W32PH_FreeString.restype = None
dll.W32PH_FreeString.argtypes = [ctypes.c_void_p]

ptr = dll.W32PH_GetProcessListJson()

if not ptr:
    raise RuntimeError("Failed to get process list JSON.")

try:
    json_text = ctypes.cast(ptr, ctypes.c_char_p).value.decode("utf-8")
finally:
    dll.W32PH_FreeString(ptr)

print(json_text)
```

---

## 10. C# 调用示例

项目中已有示例：

```text
examples/csharp/Program.cs
```

核心调用方式如下：

```csharp
using System;
using System.Runtime.InteropServices;

internal static class Program
{
    [DllImport("Win32ProcessHelper.dll", CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr W32PH_GetProcessListJson();

    [DllImport("Win32ProcessHelper.dll", CallingConvention = CallingConvention.Cdecl)]
    private static extern void W32PH_FreeString(IntPtr str);

    private static int Main()
    {
        IntPtr jsonPtr = W32PH_GetProcessListJson();

        if (jsonPtr == IntPtr.Zero)
        {
            Console.Error.WriteLine("Failed to get process list JSON.");
            return 1;
        }

        // 读取 UTF-8 字符串后释放
        // 实际项目中可参考 examples/csharp/Program.cs
        W32PH_FreeString(jsonPtr);

        return 0;
    }
}
```

---

## 11. CLI 使用方式

构建后会生成：

```text
Win32ProcessHelper.exe
```

当前支持以下命令：

```cmd
Win32ProcessHelper.exe --BasicProcessEnumeration
```

输出示例：

```text
PID     PPID    ImageName
4       0       System
892     556     svchost.exe
1234    5678    example.exe
```

---

## 12. 注意事项

### 12.1 `imageName` 来源

当前实现使用 Windows Toolhelp32 API：

```cpp
CreateToolhelp32Snapshot
Process32FirstW
Process32NextW
```

返回的 `imageName` 来自：

```cpp
PROCESSENTRY32W::szExeFile
```

因此它通常是进程映像文件名，例如：

```text
example.exe
```

不保证是完整路径。

---

### 12.2 父进程 ID 说明

`parentProcessId` 来自：

```cpp
PROCESSENTRY32W::th32ParentProcessID
```

该字段可能具有以下限制：

- 父进程可能已经退出；
- 父进程 ID 可能被系统复用；
- 不应无条件认为该父进程在调用时仍然存在。

---

### 12.3 编码说明

所有返回字符串均为 UTF-8 编码。

调用方在 Windows 控制台、日志系统、JSON 解析器中使用时，应确保使用 UTF-8 解码。

---

### 12.4 线程安全说明

当前 API 没有全局可变状态，主要数据来自调用时创建的进程快照。

但从兼容性角度考虑，建议：

- 不要在多个线程中共享同一个返回数组；
- 每个线程如果需要数据，应各自调用接口并各自释放；
- 不要跨线程传递未复制的返回指针。

---

### 12.5 版本兼容性

当前接口版本为：

```text
1.1.0
```

未来如果新增接口，建议保持已有 C API 的兼容性。

如果修改结构体或函数语义，应升级主版本号或明确标注 Breaking Change。
下面按你的要求处理：

1. **Git 日志只给一条可直接复制的正文**
2. **新增第二个功能：详细进程列表**
3. **新增第三个功能：单进程详细查看**
4. **单进程详细查看默认返回 JSON，方便机器解析**
5. **对中文做 UTF-8 适配**


# 6. `docs/API.md` 需要追加的内容

## 详细进程列表 API

### `W32PH_PROCESS_INFO_EX`

```cpp
typedef struct W32PH_PROCESS_INFO_EX {
    uint32_t processId;
    uint32_t parentProcessId;
    uint32_t threadCount;
    int32_t basePriority;
    char* imageName;
} W32PH_PROCESS_INFO_EX;
```

| 字段 | 类型 | 说明 |
|---|---|---|
| `processId` | `uint32_t` | 进程 ID |
| `parentProcessId` | `uint32_t` | 父进程 ID |
| `threadCount` | `uint32_t` | 线程数 |
| `basePriority` | `int32_t` | 基础优先级 |
| `imageName` | `char*` | 进程映像名，UTF-8 编码 |

---

### `W32PH_EnumerateProcessesDetailed`

```cpp
W32PH_API int W32PH_EnumerateProcessesDetailed(
    W32PH_PROCESS_INFO_EX** outProcesses,
    uint32_t* outCount
);
```

枚举当前系统的详细进程列表。

成功后必须调用：

```cpp
W32PH_FreeProcessListDetailed(*outProcesses, *outCount);
```

---

### `W32PH_FreeProcessListDetailed`

```cpp
W32PH_API void W32PH_FreeProcessListDetailed(
    W32PH_PROCESS_INFO_EX* processes,
    uint32_t count
);
```

释放由 `W32PH_EnumerateProcessesDetailed` 返回的详细进程数组。

---

### `W32PH_GetProcessListDetailedJson`

```cpp
W32PH_API const char* W32PH_GetProcessListDetailedJson(void);
```

返回详细进程列表 JSON。

返回值必须使用：

```cpp
W32PH_FreeString
```

释放。

---

### `W32PH_GetProcessDetailJson`

```cpp
W32PH_API const char* W32PH_GetProcessDetailJson(uint32_t processId);
```

根据进程 ID 查询单个进程的详细信息。

返回值必须使用：

```cpp
W32PH_FreeString
```

释放。

---

## 单进程详细信息 JSON 格式

```json
{
  "found": true,
  "processId": 4,
  "parentProcessId": 0,
  "imageName": "System",
  "threadCount": 0,
  "basePriority": 0,
  "openSucceeded": true,
  "openErrorCode": 0,
  "executablePathAvailable": true,
  "executablePath": "C:\\Windows\\System32\\System",
  "creationTimeAvailable": true,
  "creationTimeUtc": "2026-10-25T08:00:00Z",
  "kernelTimeTicks": 123456,
  "userTimeTicks": 0,
  "snapshotErrorCode": 0,
  "errorMessage": null
}
```

如果进程不存在：

```json
{
  "found": false,
  "processId": 999999,
  "parentProcessId": 0,
  "imageName": null,
  "threadCount": 0,
  "basePriority": 0,
  "openSucceeded": false,
  "openErrorCode": 0,
  "executablePathAvailable": false,
  "executablePath": null,
  "creationTimeAvailable": false,
  "creationTimeUtc": null,
  "kernelTimeTicks": 0,
  "userTimeTicks": 0,
  "snapshotErrorCode": 127,
  "errorMessage": "Process not found."
}
```

---

## CLI 命令

```cmd
Win32ProcessHelper.exe --BasicProcessEnumeration
Win32ProcessHelper.exe --BasicProcessEnumeration --Json
Win32ProcessHelper.exe --DetailedProcessEnumeration
Win32ProcessHelper.exe --DetailedProcessEnumeration --Json
Win32ProcessHelper.exe --ProcessDetail --ProcessId 4
```

---

## Python 示例

```python
import ctypes

dll = ctypes.CDLL("Win32ProcessHelper.dll")

dll.W32PH_GetProcessListDetailedJson.restype = ctypes.c_void_p
dll.W32PH_GetProcessListDetailedJson.argtypes = []

dll.W32PH_GetProcessDetailJson.restype = ctypes.c_void_p
dll.W32PH_GetProcessDetailJson.argtypes = [ctypes.c_uint32]

dll.W32PH_FreeString.restype = None
dll.W32PH_FreeString.argtypes = [ctypes.c_void_p]

ptr = dll.W32PH_GetProcessDetailJson(4)
if ptr:
    text = ctypes.cast(ptr, ctypes.c_char_p).value.decode("utf-8")
    dll.W32PH_FreeString(ptr)
    print(text)
```

---

## C# 示例

```csharp
using System;
using System.Runtime.InteropServices;

internal static class Program
{
    [DllImport("Win32ProcessHelper.dll", CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr W32PH_GetProcessDetailJson(uint processId);

    [DllImport("Win32ProcessHelper.dll", CallingConvention = CallingConvention.Cdecl)]
    private static extern void W32PH_FreeString(IntPtr str);

    private static int Main()
    {
        IntPtr ptr = W32PH_GetProcessDetailJson(4);

        if (ptr == IntPtr.Zero)
        {
            Console.Error.WriteLine("Failed to get process detail.");
            return 1;
        }

        // 实际读取时请使用 UTF-8。
        // 这里仅演示调用关系。
        W32PH_FreeString(ptr);

        return 0;
    }
}
```
```

---

# 7. README 可以顺手更新成这个

```markdown
# Win32ProcessHelper

Windows 用户态进程管理工具。

## 当前功能

- 基础进程列表枚举
- 详细进程列表枚举
- 单进程详细信息查询
- DLL 提供结构体接口与 JSON 接口
- EXE 通过隐式链接调用 DLL

## 命令

```cmd
Win32ProcessHelper.exe --BasicProcessEnumeration
Win32ProcessHelper.exe --BasicProcessEnumeration --Json
Win32ProcessHelper.exe --DetailedProcessEnumeration
Win32ProcessHelper.exe --DetailedProcessEnumeration --Json
Win32ProcessHelper.exe --ProcessDetail --ProcessId 4
```
```

---

# 8. 中文适配说明

这次已经做了这些中文适配：

1. Windows API 使用 `Process32FirstW` / `Process32NextW`；
2. 内部字符串统一转换为 UTF-8；
3. CLI 启动时执行：

```cpp
SetConsoleOutputCP(CP_UTF8);
SetConsoleCP(CP_UTF8);
```

4. CMake 已启用：

```cmake
add_compile_options(/utf-8)
```

5. JSON 输出不会主动转义中文，只转义 JSON 必需字符；
6. Python / C# 示例按 UTF-8 解码。

如果你在 PowerShell 中解析输出，建议也先执行：

```powershell
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
```

例如：

```powershell
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
.\Win32ProcessHelper.exe --ProcessDetail --ProcessId 4 | ConvertFrom-Json
```