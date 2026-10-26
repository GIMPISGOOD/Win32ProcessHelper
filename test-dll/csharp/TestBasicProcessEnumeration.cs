using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Text;

internal static class Program
{
    private static int _failures;

    private static int Main()
    {
        try
        {
            Console.OutputEncoding = Encoding.UTF8;
        }
        catch
        {
            // Ignore console encoding errors.
        }

        try
        {
            string exe = FindExe();
            Console.WriteLine("Using EXE: " + exe);

            int exitCode = RunExe(
                exe,
                "--BasicProcessEnumeration",
                out string stdout,
                out string stderr
            );

            if (exitCode != 0)
            {
                Console.Error.WriteLine(stderr);
            }

            Check(exitCode == 0, "--BasicProcessEnumeration exit code is 0");
            Check(!string.IsNullOrWhiteSpace(stdout), "--BasicProcessEnumeration output is not empty");
            Check(stdout.Contains("PID"), "--BasicProcessEnumeration output contains PID header");

            exitCode = RunExe(
                exe,
                "--BasicProcessEnumeration --Json",
                out stdout,
                out stderr
            );

            if (exitCode != 0)
            {
                Console.Error.WriteLine(stderr);
            }

            Check(exitCode == 0, "--BasicProcessEnumeration --Json exit code is 0");

            string trimmed = stdout.Trim();

            Check(trimmed.StartsWith("["), "JSON output starts with [");
            Check(trimmed.Contains("\"processId\""), "JSON contains processId");
            Check(trimmed.Contains("\"parentProcessId\""), "JSON contains parentProcessId");
            Check(trimmed.Contains("\"imageName\""), "JSON contains imageName");
        }
        catch (Exception ex)
        {
            Check(false, "Exception: " + ex.Message);
        }

        Console.WriteLine(_failures == 0
            ? "TestBasicProcessEnumeration: PASSED"
            : "TestBasicProcessEnumeration: FAILED");

        return _failures == 0 ? 0 : 1;
    }

    private static void Check(bool condition, string message)
    {
        if (condition)
        {
            Console.WriteLine("PASS: " + message);
        }
        else
        {
            Console.WriteLine("FAIL: " + message);
            _failures++;
        }
    }

    private static int RunExe(
        string exe,
        string arguments,
        out string stdout,
        out string stderr
    ) {
        ProcessStartInfo info = new ProcessStartInfo();
        info.FileName = exe;
        info.Arguments = arguments;
        info.UseShellExecute = false;
        info.RedirectStandardOutput = true;
        info.RedirectStandardError = true;
        info.CreateNoWindow = true;
        info.StandardOutputEncoding = Encoding.UTF8;
        info.StandardErrorEncoding = Encoding.UTF8;

        using (Process process = Process.Start(info))
        {
            stdout = process.StandardOutput.ReadToEnd();
            stderr = process.StandardError.ReadToEnd();
            process.WaitForExit();
            return process.ExitCode;
        }
    }

    private static string FindExe()
    {
        List<string> candidates = new List<string>();

        AddRootCandidates(Directory.GetCurrentDirectory(), candidates);
        AddRootCandidates(AppDomain.CurrentDomain.BaseDirectory, candidates);

        AddParentCandidates(Directory.GetCurrentDirectory(), candidates);
        AddParentCandidates(AppDomain.CurrentDomain.BaseDirectory, candidates);

        foreach (string candidate in candidates)
        {
            if (File.Exists(candidate))
            {
                return candidate;
            }
        }

        throw new FileNotFoundException(
            "Win32ProcessHelper.exe not found. Build the project first."
        );
    }

    private static void AddRootCandidates(string root, List<string> candidates)
    {
        if (string.IsNullOrWhiteSpace(root))
        {
            return;
        }

        candidates.Add(Path.Combine(root, "build", "nmake-x64", "bin", "Win32ProcessHelper.exe"));
        candidates.Add(Path.Combine(root, "build", "bin", "Win32ProcessHelper.exe"));
        candidates.Add(Path.Combine(root, "Win32ProcessHelper.exe"));
    }

    private static void AddParentCandidates(string start, List<string> candidates)
    {
        try
        {
            DirectoryInfo dir = new DirectoryInfo(start);

            for (int i = 0; i < 8 && dir != null; i++)
            {
                AddRootCandidates(dir.FullName, candidates);
                dir = dir.Parent;
            }
        }
        catch
        {
            // Ignore invalid path errors.
        }
    }
}