using System;
using System.Runtime.InteropServices;

namespace Win32ProcessHelperExample
{
    class Program
    {
        [DllImport("Win32ProcessHelper.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr W32PH_GetVersion();

        [DllImport("Win32ProcessHelper.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void W32PH_FreeString(IntPtr str);

        static void Main(string[] args)
        {
            try
            {
                IntPtr ptr = W32PH_GetVersion();
                if (ptr != IntPtr.Zero)
                {
                    // 注意：Marshal.PtrToStringUTF8 需要 .NET Core 2.1+ 或 .NET 5+
                    // 如果是 .NET Framework，请使用 Encoding.UTF8.GetString(ptr, length)
                    string version = Marshal.PtrToStringUTF8(ptr);
                    Console.WriteLine($"C#: DLL Version is {version}");
                    
                    W32PH_FreeString(ptr);
                }
                else
                {
                    Console.WriteLine("C#: Failed to get version.");
                }
            }
            catch (DllNotFoundException ex)
            {
                Console.WriteLine($"Error: Cannot find Win32ProcessHelper.dll. Ensure it is in the same directory as the EXE.\n{ex.Message}");
            }
        }
    }
}