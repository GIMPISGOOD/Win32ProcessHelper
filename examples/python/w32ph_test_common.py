import json
import subprocess
from pathlib import Path

EXE_NAME = "Win32ProcessHelper.exe"


def _unique_roots():
    roots = []

    # examples/python/xxx.py 的上级目录：
    # examples/python -> examples -> project root
    try:
        roots.append(Path(__file__).resolve().parents[2])
    except Exception:
        pass

    roots.append(Path.cwd())

    try:
        roots.extend(Path.cwd().parents)
    except Exception:
        pass

    unique = []
    seen = set()

    for root in roots:
        try:
            resolved = root.resolve()
        except Exception:
            continue

        if resolved not in seen:
            seen.add(resolved)
            unique.append(resolved)

    return unique


def find_exe():
    for root in _unique_roots():
        candidates = [
            root / "build" / "nmake-x64" / "bin" / EXE_NAME,
            root / "build" / "bin" / EXE_NAME,
            root / EXE_NAME,
        ]

        for candidate in candidates:
            if candidate.exists():
                return candidate

    raise FileNotFoundError(
        "Win32ProcessHelper.exe not found. "
        "Please build the project first, for example: "
        "cmake --build build\\nmake-x64"
    )


def run_exe(args):
    exe = find_exe()
    command = [str(exe)] + [str(arg) for arg in args]

    completed = subprocess.run(
        command,
        capture_output=True,
        timeout=60,
    )

    stdout = completed.stdout.decode("utf-8", errors="replace")
    stderr = completed.stderr.decode("utf-8", errors="replace")

    return completed.returncode, stdout, stderr


def parse_json(text):
    return json.loads(text)


def check(condition, message, failures):
    if condition:
        print(f"PASS: {message}")
    else:
        print(f"FAIL: {message}")
        failures.append(message)

    return condition


def finish(test_name, failures):
    if failures:
        print(f"{test_name}: FAILED ({len(failures)} problem(s))")
        return 1

    print(f"{test_name}: PASSED")
    return 0