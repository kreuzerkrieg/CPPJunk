using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;
using System.IO;
using System.ComponentModel;

namespace MemoryMap
{
    class ResourceExecutableLauncher
    {
        public ResourceExecutableLauncher()
        {
            Init(Process.GetCurrentProcess().Id);
        }

        public ResourceExecutableLauncher(int ProcId)
        {
            Init(ProcId);
        }

        ~ResourceExecutableLauncher()
        {
            try
            {
                if (!String.IsNullOrEmpty(FileName))
                    File.Delete(FileName);
            }
            catch (Exception)
            {
            }
        }

        private void Init(int ProcId)
        {
            FileName = System.IO.Path.GetTempFileName();
            try
            {
                if (Is64BitProcess(ProcId))
                {
                    File.WriteAllBytes(FileName, MemoryMap.Properties.Executables.HeapWalker_amd64);
                }
                else
                {
                    File.WriteAllBytes(FileName, MemoryMap.Properties.Executables.HeapWalker_x86);
                }
            }
            catch (Exception ex)
            {
                if (!String.IsNullOrEmpty(FileName))
                {
                    File.Delete(FileName);
                    FileName = "";
                }
                throw new Exception("Failed to launch resource executable", ex);
            }
        }

        private Boolean Is64BitProcess(int ProcessId)
        {
            try
            {
                Process Proc = Process.GetProcessById(ProcessId);
                Int64 BaseAddress;
                try
                {
                    BaseAddress = (Int64)Proc.MainModule.BaseAddress;
                }
                catch (Win32Exception ee)
                {
                    return ee.ErrorCode == -2147467259;
                }
                return BaseAddress > Int32.MaxValue;
            }
            catch (Exception)
            {
                return true;
            }
        }

        public String FileName;
    }
}
