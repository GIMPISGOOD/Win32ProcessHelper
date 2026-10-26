import sys

from w32ph_test_common import run_exe, parse_json, check, finish


def main():
    failures = []

    # --------------------------------------------------
    # 1. 测试普通文本输出
    # --------------------------------------------------
    code, out, err = run_exe(["--DetailedProcessEnumeration"])

    check(
        code == 0,
        "--DetailedProcessEnumeration exit code is 0",
        failures,
    )

    if code != 0:
        print(err)

    lines = out.strip().splitlines()

    check(
        len(lines) > 0,
        "--DetailedProcessEnumeration output is not empty",
        failures,
    )

    if lines:
        header = lines[0]

        check(
            header.startswith("PID"),
            "detailed text header starts with PID",
            failures,
        )

        check(
            "Threads" in header and "Priority" in header,
            "detailed text header contains Threads and Priority",
            failures,
        )

    # --------------------------------------------------
    # 2. 测试 JSON 输出
    # --------------------------------------------------
    code, out, err = run_exe(["--DetailedProcessEnumeration", "--Json"])

    check(
        code == 0,
        "--DetailedProcessEnumeration --Json exit code is 0",
        failures,
    )

    if code != 0:
        print(err)

    data = None

    try:
        data = parse_json(out)
        check(
            True,
            "detailed JSON output can be parsed",
            failures,
        )
    except Exception as ex:
        check(
            False,
            f"detailed JSON output can be parsed: {ex}",
            failures,
        )

    if isinstance(data, list):
        check(
            len(data) > 0,
            "detailed process list is not empty",
            failures,
        )

        sample = data[:20]

        for index, process in enumerate(sample):
            ok = (
                isinstance(process.get("processId"), int)
                and isinstance(process.get("parentProcessId"), int)
                and isinstance(process.get("threadCount"), int)
                and isinstance(process.get("basePriority"), int)
                and isinstance(process.get("imageName"), str)
            )

            check(
                ok,
                f"process[{index}] has valid detailed fields",
                failures,
            )
    else:
        check(
            False,
            "detailed JSON root is an array",
            failures,
        )

    return finish("test_detailed_process_enumeration", failures)


if __name__ == "__main__":
    raise SystemExit(main())