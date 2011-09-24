using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;
using System.IO;
using System.ComponentModel;

namespace MemoryMap
{
    class ProcessOutputRedirection
    {
        public ProcessOutputRedirection(int ProcessId)
        {
            HeapWalker = null;
            try
            {
                ResourceExecutableLauncher Launcher = new ResourceExecutableLauncher(ProcessId);
                String FileName = Launcher.FileName;
                HeapWalker = new Process();
                HeapWalker.StartInfo.CreateNoWindow = true;
                HeapWalker.StartInfo.WorkingDirectory = System.IO.Path.GetDirectoryName(FileName);
                HeapWalker.StartInfo.Arguments = ProcessId.ToString();
                HeapWalker.StartInfo.FileName = FileName;

                // Set UseShellExecute to false for redirection.
                HeapWalker.StartInfo.UseShellExecute = false;

                // Redirect the standard output of the sort command.  
                // This stream is read asynchronously using an event handler.
                HeapWalker.StartInfo.RedirectStandardOutput = true;
                Win32AppOutput = new StringBuilder("");

                // Set our event handler to asynchronously read the sort output.
                HeapWalker.OutputDataReceived += new DataReceivedEventHandler(OutputHandler);

                // Redirect standard input as well.  This stream
                // is used synchronously.
                HeapWalker.StartInfo.RedirectStandardInput = true;

            }
            catch (Exception ex)
            {
                if (HeapWalker != null)
                {
                    HeapWalker.Close();
                }
                throw new Exception("Cannot create process.", ex);
            }
        }

        ~ProcessOutputRedirection()
        {
            if (HeapWalker != null)
            {
                HeapWalker.Close();
            }
        }

        public StringBuilder ReadExecutableOutput()
        {
            try
            {
                // Start the process.
                HeapWalker.Start();
                HeapWalker.BeginOutputReadLine();

                // Wait for the sort process to write the sorted text lines.
                HeapWalker.WaitForExit();
                HeapWalker.Close();
                if (ValidateHeapWalkerOutput())
                    return new StringBuilder(Win32AppOutput.ToString());
                else
                {
                    System.Text.RegularExpressions.Regex RegEx = new System.Text.RegularExpressions.Regex(@"Error\: (.+)");
                    String ErrorMessage = "";
                    if (RegEx.IsMatch(Win32AppOutput.ToString()))
                    {
                        ErrorMessage = RegEx.Match(Win32AppOutput.ToString()).Groups[0].Value;
                    }
                    throw new Exception(ErrorMessage);
                }
            }
            catch (Exception ex)
            {
                throw new Exception("Cannot read executable output.", ex);
            }
        }

        private Boolean ValidateHeapWalkerOutput()
        {
            return !Win32AppOutput.ToString(0, Math.Min(256, Win32AppOutput.Length)).StartsWith("Error:");
        }

        private static void OutputHandler(object sendingProcess, DataReceivedEventArgs outLine)
        {
            if (!String.IsNullOrEmpty(outLine.Data))
            {
                Win32AppOutput.Append(outLine.Data);
            }
        }

        private static StringBuilder Win32AppOutput;
        private Process HeapWalker;
    }
}
