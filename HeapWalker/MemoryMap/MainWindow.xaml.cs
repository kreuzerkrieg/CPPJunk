using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Runtime.InteropServices;
using System.Xml;
using System.Diagnostics;

namespace MemoryMap
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            {
                FillProcessList();
                if (ReadMemInfo("4616"))
                    LoadData();
            }
        }

        private bool ReadMemInfo(String ProcessId)
        {
            String FileName = System.IO.Path.GetTempFileName();
            Process HeapWalker = null;
            try
            {
                File.WriteAllBytes(FileName, MemoryMap.Properties.Executables.HeapWalker_x86);
                HeapWalker = new Process();
                HeapWalker.StartInfo.CreateNoWindow = true;
                HeapWalker.StartInfo.WorkingDirectory = System.IO.Path.GetDirectoryName(FileName);
                HeapWalker.StartInfo.Arguments = ProcessId;
                HeapWalker.StartInfo.FileName = FileName;

                // Set UseShellExecute to false for redirection.
                HeapWalker.StartInfo.UseShellExecute = false;

                // Redirect the standard output of the sort command.  
                // This stream is read asynchronously using an event handler.
                HeapWalker.StartInfo.RedirectStandardOutput = true;
                Win32AppOutput = new StringBuilder("");

                // Set our event handler to asynchronously read the sort output.
                HeapWalker.OutputDataReceived += new DataReceivedEventHandler(SortOutputHandler);

                // Redirect standard input as well.  This stream
                // is used synchronously.
                HeapWalker.StartInfo.RedirectStandardInput = true;

                // Start the process.
                HeapWalker.Start();
                HeapWalker.BeginOutputReadLine();

                // Wait for the sort process to write the sorted text lines.
                HeapWalker.WaitForExit();
                File.Delete(FileName);
                return ValidateHeapWalkerOutput();
            }
            catch (Exception ex)
            {
                if (HeapWalker != null)
                {
                    HeapWalker.Close();
                }
                File.Delete(FileName);
                return false;
            }
        }

        private Boolean ValidateHeapWalkerOutput()
        {
            return !Win32AppOutput.ToString(0, 100).StartsWith("Error:");
        }

        private static void SortOutputHandler(object sendingProcess,
            DataReceivedEventArgs outLine)
        {
            if (!String.IsNullOrEmpty(outLine.Data))
            {
                Win32AppOutput.Append(outLine.Data);
            }
        }

        private void LoadData()
        {
            try
            {
                mem_data = new List<MemoryRegion>();
                LoadXml();
                UInt64 max_region_size = GetMaxSize(ref mem_data);
                foreach (MemoryRegion data in mem_data)
                {
                    if (data.NumberOfBlocks == 0)
                    {
                        DrawMemoryRectangle(max_region_size, data.RegionSize, data.RegionStorageType, data.RegionProtection, false, "");
                    }
                    else
                    {
                        foreach (MemoryBlock block in data.MemoryBlocks)
                        {
                            DrawMemoryRectangle(max_region_size, block.BlockSize, block.BlockStorageType, block.BlockProtection, block.IsShared, block.MappedFileName);
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                String m = ex.Message;
            }
        }

        private void DrawBlockRectangle(ulong max_region_size, MemoryBlock block)
        {
            throw new NotImplementedException();
        }

        private Rectangle DrawMemoryRectangle(UInt64 max_region_size,
            ulong Size,
            uint StorageType,
            uint Protection,
            bool IsShared,
            string MappedFileName)
        {
            Rectangle RetVal = new Rectangle();
            SolidColorBrush ColorBrush = new SolidColorBrush();

            RetVal.StrokeThickness = 1;
            RetVal.Stroke = Brushes.Black;

            RetVal.Width = 8 + Math.Min(350, (Size * (350 / (Double)max_region_size)));
            RetVal.Height = 8;
            RetVal.HorizontalAlignment = System.Windows.HorizontalAlignment.Left;
            RetVal.VerticalAlignment = System.Windows.VerticalAlignment.Top;
            RetVal.RadiusX = 1;
            RetVal.RadiusY = 1;
            Thickness margin = new Thickness(1, 0, 0, 1);
            RetVal.Margin = margin;
            RetVal.ToolTip = "Size: " + ((Size / 1024).ToString()) + "Kb\nStorage: ";
            switch (StorageType)
            {
                case (uint)MemoryStorageType.MEM_FREE:
                    {
                        RetVal.ToolTip += "Free";
                        ColorBrush.Color = Color.FromArgb(255, 255, 255, 255);
                        break;
                    }
                case (uint)MemoryStorageType.MEM_RESERVE:
                    {
                        RetVal.ToolTip += "Reserve";
                        ColorBrush.Color = Color.FromArgb(255, 0, 255, 0);
                        break;
                    }
                case (uint)MemoryStorageType.MEM_IMAGE:
                    {
                        RetVal.ToolTip += "Image";
                        ColorBrush.Color = Color.FromArgb(255, 0, 0, 255);
                        break;
                    }
                case (uint)MemoryStorageType.MEM_MAPPED:
                    {
                        RetVal.ToolTip += "Mapped";
                        ColorBrush.Color = Color.FromArgb(255, 0xEB, 0xEB, 0x00);
                        break;
                    }
                case (uint)MemoryStorageType.MEM_PRIVATE:
                    {
                        RetVal.ToolTip += "Private";
                        ColorBrush.Color = Color.FromArgb(255, 255, 0, 0);
                        break;
                    }
            }
            RetVal.Fill = ColorBrush;

            RetVal.ToolTip += "\nProtection: ";
            uint ProtectionFlag = Protection & ~((uint)PageProtection.PAGE_GUARD | (uint)PageProtection.PAGE_NOCACHE | (uint)PageProtection.PAGE_WRITECOMBINE);
            switch (ProtectionFlag)
            {
                case (uint)PageProtection.PAGE_READONLY: RetVal.ToolTip += "-R--"; break;
                case (uint)PageProtection.PAGE_READWRITE: RetVal.ToolTip += "-RW-"; break;
                case (uint)PageProtection.PAGE_WRITECOPY: RetVal.ToolTip += "-RWC"; break;
                case (uint)PageProtection.PAGE_EXECUTE: RetVal.ToolTip += "E---"; break;
                case (uint)PageProtection.PAGE_EXECUTE_READ: RetVal.ToolTip += "ER--"; break;
                case (uint)PageProtection.PAGE_EXECUTE_READWRITE: RetVal.ToolTip += "ERW-"; break;
                case (uint)PageProtection.PAGE_EXECUTE_WRITECOPY: RetVal.ToolTip += "ERWC"; break;
                case (uint)PageProtection.PAGE_NOACCESS: RetVal.ToolTip += "----"; break;
            }
            RetVal.ToolTip += ((Protection & (uint)PageProtection.PAGE_GUARD) == (uint)PageProtection.PAGE_GUARD) ? "G" : "-";
            RetVal.ToolTip += ((Protection & (uint)PageProtection.PAGE_NOCACHE) == (uint)PageProtection.PAGE_NOCACHE) ? "N" : "-";
            RetVal.ToolTip += ((Protection & (uint)PageProtection.PAGE_WRITECOMBINE) == (uint)PageProtection.PAGE_WRITECOMBINE) ? "W" : "-";
            RetVal.ToolTip += (IsShared)?"\nShared":"";
            RetVal.ToolTip += (String.IsNullOrEmpty(MappedFileName)) ? "" : "\n" + MappedFileName;

            MemMapPanel.Children.Add(RetVal);
            return RetVal;
        }

        private ulong GetMaxSize(ref List<MemoryRegion> mem_data)
        {
            ulong RetVal = 0;
            foreach (MemoryRegion data in mem_data)
            {
                if (data.RegionStorageType != (uint)MemoryStorageType.MEM_FREE)
                {
                    RetVal = Math.Max(RetVal, data.RegionSize);
                }
            }
            return RetVal;
        }
        private void LoadXml()
        {
            using (StringReader stream = new StringReader(Win32AppOutput.ToString()))
            {
                XmlReader reader = new XmlTextReader(stream);

                try
                {
                    reader.MoveToContent();
                    while (reader.ReadState == ReadState.Interactive && reader.ReadToFollowing("item"))
                    {
                        MemoryRegion region = new MemoryRegion();
                        XmlReader SubReader = reader.ReadSubtree();
                        ParseRegion(ref region, ref SubReader);
                        mem_data.Add(region);
                    }
                }
                catch (Exception e)
                {
                    String a = e.Message;
                    a += "a";
                }
                
                reader.Close();
                stream.Close();

            }
        }

        private void ParseRegion(ref MemoryRegion region, ref XmlReader reader)
        {
            reader.ReadToDescendant("m_region_base_addr");
            region.RegionAddress = (UInt64)reader.ReadElementContentAs(typeof(UInt64), null);
            reader.ReadToNextSibling("m_region_protection");
            region.RegionProtection = (UInt32)reader.ReadElementContentAs(typeof(UInt32), null);
            reader.ReadToNextSibling("region_size");
            region.RegionSize = (UInt64)reader.ReadElementContentAs(typeof(UInt64), null);
            reader.ReadToNextSibling("m_storage_type");
            region.RegionStorageType = (UInt32)reader.ReadElementContentAs(typeof(UInt32), null);
            reader.ReadToNextSibling("m_region_blocks");
            region.NumberOfBlocks = (UInt32)reader.ReadElementContentAs(typeof(UInt32), null);
            reader.ReadToNextSibling("m_guard_blocks");
            region.NumberOfGuardBlocks = (UInt32)reader.ReadElementContentAs(typeof(UInt32), null);
            reader.ReadToNextSibling("m_is_stack");
            region.IsAStack = (Boolean)reader.ReadElementContentAs(typeof(Boolean), null);
            reader.ReadToFollowing("m_blocks");
            XmlReader SubReader = reader.ReadSubtree();
            ParseBlocks(ref region.MemoryBlocks, ref SubReader);
        }

        private void ParseBlocks(ref List<MemoryBlock> list, ref XmlReader reader)
        {
            reader.ReadToFollowing("count");
            UInt32 ListLength = (UInt32)reader.ReadElementContentAs(typeof(UInt32), null);
            if (ListLength == 0)
                return;
            list = new List<MemoryBlock>();
            while (reader.ReadToFollowing("item"))
            {
                XmlReader SubReader = reader.ReadSubtree();
                ParseBlock(ref list, ref SubReader);
            }
        }

        private void ParseBlock(ref List<MemoryBlock> list, ref XmlReader reader)
        {
            MemoryBlock MemBlock = new MemoryBlock();
            reader.ReadToFollowing("m_block_base_addr");
            MemBlock.BlockAddress = (UInt64)reader.ReadElementContentAs(typeof(UInt64), null);
            reader.ReadToNextSibling("protection");
            MemBlock.BlockProtection = (UInt32)reader.ReadElementContentAs(typeof(UInt32), null);
            reader.ReadToNextSibling("m_size");
            MemBlock.BlockSize = (UInt64)reader.ReadElementContentAs(typeof(UInt64), null);
            reader.ReadToNextSibling("m_storage_type");
            MemBlock.BlockStorageType = (UInt32)reader.ReadElementContentAs(typeof(UInt32), null);
            reader.ReadToNextSibling("m_is_shared");
            MemBlock.IsShared = (Boolean)reader.ReadElementContentAs(typeof(Boolean), null);
            reader.ReadToNextSibling("m_map_file_name");
            MemBlock.MappedFileName = (String)reader.ReadElementContentAs(typeof(String), null);
            list.Add(MemBlock);
        }

        private void FillProcessList()
        {
            Process[] Processes = Process.GetProcesses();
            int i = 0;
            foreach (Process Proc in Processes)
            {
                RowDefinition NewRow = new RowDefinition();
                ProcessesGrid.RowDefinitions.Add(NewRow);
                Label ProcessName = new Label();
                Label ProcessId = new Label();
                ProcessName.Content = Proc.ProcessName;
                ProcessName.HorizontalAlignment= System.Windows.HorizontalAlignment.Left;
                ProcessName.VerticalAlignment = System.Windows.VerticalAlignment.Top;
                Grid.SetColumn(ProcessName, 0);
                Grid.SetRow(ProcessName, i);

                ProcessId.Content = Proc.Id;
                ProcessId.HorizontalAlignment= System.Windows.HorizontalAlignment.Right;
                ProcessId.VerticalAlignment = System.Windows.VerticalAlignment.Top;
                Grid.SetColumn(ProcessId, 2);
                Grid.SetRow(ProcessId, i);
                ProcessesGrid.Children.Add(ProcessName);
                ProcessesGrid.Children.Add(ProcessId);
                i++;
            }
            MMWindow.Show();
        }
        private List<MemoryRegion> mem_data;
        private static StringBuilder Win32AppOutput = null;
    }
}
