import ctypes
import os
import sys

def main():
    # 动态计算 DLL 路径 (假设从 examples/python 运行)
    script_dir = os.path.dirname(os.path.abspath(__file__))
    dll_path = os.path.normpath(os.path.join(script_dir, "../../build/nmake-x64/bin/Win32ProcessHelper.dll"))
    
    if not os.path.exists(dll_path):
        print(f"Error: DLL not found at {dll_path}")
        sys.exit(1)

    try:
        dll = ctypes.CDLL(dll_path)
    except OSError as e:
        print(f"Failed to load DLL: {e}")
        sys.exit(1)

    # 设置函数签名
    # 必须使用 c_void_p 接收指针，避免 ctypes 自动转换导致无法获取原始地址进行释放
    dll.W32PH_GetVersion.restype = ctypes.c_void_p
    dll.W32PH_GetVersion.argtypes = []

    dll.W32PH_FreeString.restype = None
    dll.W32PH_FreeString.argtypes = [ctypes.c_char_p]

    # 调用获取版本
    ptr = dll.W32PH_GetVersion()
    if ptr:
        # 将 void* 转换为 c_char_p 以读取字符串
        version_str = ctypes.cast(ptr, ctypes.c_char_p).value.decode('utf-8')
        print(f"Python: DLL Version is {version_str}")
        
        # 释放内存
        dll.W32PH_FreeString(ctypes.cast(ptr, ctypes.c_char_p))
    else:
        print("Python: Failed to get version.")

if __name__ == "__main__":
    main()