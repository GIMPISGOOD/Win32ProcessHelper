import os
import sys

from w32ph_test_common import run_exe, parse_json, check, finish


def check_required_fields(data, failures):
    required_fields = [
        "found",
        "processId",
        "parentProcessId",
        "imageName",
        "threadCount",
        "basePriority",
        "openSucceeded",
        "openErrorCode",
        "executablePathAvailable",
        "executablePath",
        "creationTimeAvailable",
        "creationTimeUtc",
        "kernelTimeTicks",
        "userTimeTicks",
        "snapshotErrorCode",
        "errorMessage",
    ]

    for field in required_fields:
        check(
            field in data,
            f"process detail JSON contains field: {field}",
            failures,
        )


def main():
    failures = []

    # 使用当前 Python 进程作为目标进程。
    # 这样可以保证进程一定存在。
    pid = os.getpid()

    # --------------------------------------------------
    # 1. 测试正常进程详情
    # --------------------------------------------------
    code, out, err = run_exe(["--ProcessDetail", "--ProcessId", str(pid)])

    check(
        code == 0,
        "--ProcessDetail --ProcessId <current pid> exit code is 0",
        failures,
    )

    if code != 0:
        print(err)

    data = None

    try:
        data = parse_json(out)
        check(
            True,
            "process detail output can be parsed as JSON",
            failures,
        )
    except Exception as ex:
        check(
            False,
            f"process detail output can be parsed as JSON: {ex}",
            failures,
        )

    if isinstance(data, dict):
        check_required_fields(data, failures)

        check(
            data.get("found") is True,
            "process detail found is true",
            failures,
        )

        check(
            data.get("processId") == pid,
            "process detail processId matches requested pid",
            failures,
        )

        check(
            isinstance(data.get("imageName"), str) and len(data.get("imageName")) > 0,
            "process detail imageName is a non-empty string",
            failures,
        )

        if data.get("executablePathAvailable") is True:
            check(
                isinstance(data.get("executablePath"), str),
                "executablePath is a string when executablePathAvailable is true",
                failures,
            )

        if data.get("creationTimeAvailable") is True:
            check(
                isinstance(data.get("creationTimeUtc"), str),
                "creationTimeUtc is a string when creationTimeAvailable is true",
                failures,
            )
    else:
        check(
            False,
            "process detail JSON root is an object",
            failures,
        )

    # --------------------------------------------------
    # 2. 测试缺少 --ProcessId 时应该失败
    # --------------------------------------------------
    code, out, err = run_exe(["--ProcessDetail"])

    check(
        code != 0,
        "--ProcessDetail without --ProcessId should fail",
        failures,
    )

    return finish("test_process_detail", failures)


if __name__ == "__main__":
    raise SystemExit(main())