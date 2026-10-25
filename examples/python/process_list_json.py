import ctypes
import json
import sys
from pathlib import Path


def find_dll() -> Path:
    root = Path(__file__).resolve().parents[2]

    candidates = [
        root / "build" / "nmake-x64" / "bin" / "Win32ProcessHelper.dll",
        Path.cwd() / "Win32ProcessHelper.dll",
    ]

    for candidate in candidates:
        if candidate.exists():
            return candidate

    raise FileNotFoundError("Win32ProcessHelper.dll not found")


def main() -> int:
    try:
        dll_path = find_dll()
        dll = ctypes.CDLL(str(dll_path))
    except Exception as ex:
        print(f"Failed to load Win32ProcessHelper.dll: {ex}", file=sys.stderr)
        return 1

    dll.W32PH_GetProcessListJson.restype = ctypes.c_void_p
    dll.W32PH_GetProcessListJson.argtypes = []

    dll.W32PH_FreeString.restype = None
    dll.W32PH_FreeString.argtypes = [ctypes.c_void_p]

    ptr = dll.W32PH_GetProcessListJson()
    if not ptr:
        print("Failed to get process list JSON.", file=sys.stderr)
        return 1

    try:
        json_text = ctypes.cast(ptr, ctypes.c_char_p).value.decode("utf-8")
    finally:
        dll.W32PH_FreeString(ptr)

    try:
        processes = json.loads(json_text)
    except json.JSONDecodeError as ex:
        print(f"Invalid JSON returned: {ex}", file=sys.stderr)
        return 1

    print(f"Process count: {len(processes)}")
    print("PID      PPID     Name")

    for process in processes:
        pid = process.get("processId", 0)
        ppid = process.get("parentProcessId", 0)
        name = process.get("imageName", "")
        print(f"{pid:<8} {ppid:<8} {name}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())