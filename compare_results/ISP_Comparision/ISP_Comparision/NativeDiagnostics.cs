using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace ISP_Comparision
{
    public static class NativeDiagnostics
    {
        [DllImport("kernel32", SetLastError = true, CharSet = CharSet.Unicode)]
        private static extern IntPtr LoadLibrary(string lpFileName);

        [DllImport("kernel32", SetLastError = true)]
        private static extern bool FreeLibrary(IntPtr hModule);

        public static void DiagnoseIspDll(string dllPath)
        {
            Debug.WriteLine($"Process Is64BitProcess: {Environment.Is64BitProcess}");
            Debug.WriteLine($"OS Is64BitOperatingSystem: {Environment.Is64BitOperatingSystem}");
            Debug.WriteLine($"DLL path: {dllPath}");
            Debug.WriteLine($"DLL exists: {File.Exists(dllPath)}");
            if (!File.Exists(dllPath)) return;

            try
            {
                using (var fs = new FileStream(dllPath, FileMode.Open, FileAccess.Read, FileShare.Read))
                using (var br = new BinaryReader(fs))
                {
                    fs.Seek(0x3C, SeekOrigin.Begin);
                    int e_lfanew = br.ReadInt32();
                    fs.Seek(e_lfanew + 4, SeekOrigin.Begin); // skip "PE\0\0"
                    ushort machine = br.ReadUInt16();
                    string arch;
                    switch (machine)
                    {
                        case 0x014c:
                            arch = "x86 (IMAGE_FILE_MACHINE_I386)";
                            break;
                        case 0x8664:
                            arch = "x64 (IMAGE_FILE_MACHINE_AMD64)";
                            break;
                        case 0x0200:
                            arch = "IA64 (IMAGE_FILE_MACHINE_IA64)";
                            break;
                        default:
                            arch = string.Format("Unknown (0x{0:X4})", machine);
                            break;
                    }
                    Debug.WriteLine($"PE Machine: {arch}");
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine($"Failed to read PE header: {ex.Message}");
            }

            IntPtr h = LoadLibrary(dllPath);
            if (h == IntPtr.Zero)
            {
                int err = Marshal.GetLastWin32Error();
                Debug.WriteLine($"LoadLibrary failed. GetLastError() = {err}");
            }
            else
            {
                Debug.WriteLine("LoadLibrary succeeded (DLL loadable for current bitness).");
                FreeLibrary(h);
            }
        }
    }
}