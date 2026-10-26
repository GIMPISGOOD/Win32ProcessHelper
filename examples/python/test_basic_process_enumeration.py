import sys

from w32ph_test_common import run_exe, parse_json, check, finish


def main():
    failures = []

    # --------------------------------------------------
    # 1. 测试普通文本输出
    # --------------------------------------------------
    code, out, err = run_exe(["--BasicProcessEnumeration"])

    check(
        code == 0,
        "--BasicProcessEnumeration exit code is 0",
        failures,
    )

    if code != 0:
        print(err)

    lines = out.strip().splitlines()

    check(
        len(lines) > 0,
        "--BasicProcessEnumeration output is not empty",
        failures,
    )

    if lines:
        check(
            lines[0].startswith("PID"),
            "text output header starts with PID",
            failures,
        )

    # --------------------------------------------------
    # 2. 测试 JSON 输出
    # --------------------------------------------------
    code, out, err = run_exe(["--BasicProcessEnumeration", "--Json"])

    check(
        code == 0,
        "--BasicProcessEnumeration --Json exit code is 0",
        failures,
    )

    if code != 0:
        print(err)

    data = None

    try:
        data = parse_json(out)
        check(
            True,
            "JSON output can be parsed",
            failures,
        )
    except Exception as ex:
        check(
            False,
            f"JSON output can be parsed: {ex}",
            failures,
        )

    if isinstance(data, list):
        check(
            len(data) > 0,
            "basic process list is not empty",
            failures,
        )

        sample = data[:20]

        for index, process in enumerate(sample):
            ok = (
                isinstance(process.get("processId"), int)
                and isinstance(process.get("parentProcessId"), int)
                and isinstance(process.get("imageName"), str)
            )

            check(
                ok,
                f"process[{index}] has valid basic fields",
                failures,
            )
    else:
        check(
            False,
            "JSON root is an array",
            failures,
        )

    return finish("test_basic_process_enumeration", failures)


if __name__ == "__main__":
    raise SystemExit(main())