using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

internal static class Program
{
    [DllImport("Win32ProcessHelper.dll", CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr W32PH_GetProcessListJson();

    [DllImport("Win32ProcessHelper.dll", CallingConvention = CallingConvention.Cdecl)]
    private static extern void W32PH_FreeString(IntPtr str);

    private static string ReadUtf8String(IntPtr ptr)
    {
        if (ptr == IntPtr.Zero)
        {
            return string.Empty;
        }

        var bytes = new List<byte>();
        var offset = 0;

        while (true)
        {
            var b = Marshal.ReadByte(ptr, offset);
            if (b == 0)
            {
                break;
            }

            bytes.Add(b);
            offset++;
        }

        return Encoding.UTF8.GetString(bytes.ToArray());
    }

    private static int Main()
    {
        try
        {
            var jsonPtr = W32PH_GetProcessListJson();
            if (jsonPtr == IntPtr.Zero)
            {
                Console.Error.WriteLine("Failed to get process list JSON.");
                return 1;
            }

            var json = ReadUtf8String(jsonPtr);
            W32PH_FreeString(jsonPtr);

            Console.WriteLine("Process list JSON:");
            Console.WriteLine(json);
            return 0;
        }
        catch (DllNotFoundException ex)
        {
            Console.Error.WriteLine("Cannot find Win32ProcessHelper.dll.");
            Console.Error.WriteLine(ex.Message);
            return 1;
        }
    }
}