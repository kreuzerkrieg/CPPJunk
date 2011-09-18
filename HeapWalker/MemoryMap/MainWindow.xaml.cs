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
            //Process proc = new Process();
            //proc.BeginOutputReadLine();
            InitializeComponent();
            LoadData();
        }

        private void LoadData()
        {
            try
            {
                mem_data = new List<MemoryRegion>();
                LoadData(@"f:\file.txt");
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

        private void DrawMemoryRectangle(UInt64 max_region_size,
            ulong Size,
            uint StorageType,
            uint Protection,
            bool IsShared,
            string MappedFileName)
        {
            Rectangle rect = new Rectangle();
            SolidColorBrush ColorBrush = new SolidColorBrush();

            rect.StrokeThickness = 1;
            rect.Stroke = Brushes.Black;

            rect.Width = 8 + Math.Min(350, (Size * (350 / (Double)max_region_size)));
            rect.Height = 8;
            rect.HorizontalAlignment = System.Windows.HorizontalAlignment.Left;
            rect.VerticalAlignment = System.Windows.VerticalAlignment.Top;
            rect.RadiusX = 1;
            rect.RadiusY = 1;
            Thickness margin = new Thickness(1, 0, 0, 1);
            rect.Margin = margin;
            rect.ToolTip = "Size: " + ((Size / 1024).ToString()) + "Kb\nStorage: ";
            switch (StorageType)
            {
                case (uint)MemoryStorageType.MEM_FREE:
                    {
                        rect.ToolTip += "Free";
                        ColorBrush.Color = Color.FromArgb(255, 255, 255, 255);
                        break;
                    }
                case (uint)MemoryStorageType.MEM_RESERVE:
                    {
                        rect.ToolTip += "Reserve";
                        ColorBrush.Color = Color.FromArgb(255, 0, 255, 0);
                        break;
                    }
                case (uint)MemoryStorageType.MEM_IMAGE:
                    {
                        rect.ToolTip += "Image";
                        ColorBrush.Color = Color.FromArgb(255, 0, 0, 255);
                        break;
                    }
                case (uint)MemoryStorageType.MEM_MAPPED:
                    {
                        rect.ToolTip += "Mapped";
                        ColorBrush.Color = Color.FromArgb(255, 0xEB, 0xEB, 0x00);
                        break;
                    }
                case (uint)MemoryStorageType.MEM_PRIVATE:
                    {
                        rect.ToolTip += "Private";
                        ColorBrush.Color = Color.FromArgb(255, 255, 0, 0);
                        break;
                    }
            }
            rect.Fill = ColorBrush;

            rect.ToolTip += "\nProtection: ";
            uint ProtectionFlag = Protection & ~((uint)PageProtection.PAGE_GUARD | (uint)PageProtection.PAGE_NOCACHE | (uint)PageProtection.PAGE_WRITECOMBINE);
            switch (ProtectionFlag)
            {
                case (uint)PageProtection.PAGE_READONLY: rect.ToolTip += "-R--"; break;
                case (uint)PageProtection.PAGE_READWRITE: rect.ToolTip += "-RW-"; break;
                case (uint)PageProtection.PAGE_WRITECOPY: rect.ToolTip += "-RWC"; break;
                case (uint)PageProtection.PAGE_EXECUTE: rect.ToolTip += "E---"; break;
                case (uint)PageProtection.PAGE_EXECUTE_READ: rect.ToolTip += "ER--"; break;
                case (uint)PageProtection.PAGE_EXECUTE_READWRITE: rect.ToolTip += "ERW-"; break;
                case (uint)PageProtection.PAGE_EXECUTE_WRITECOPY: rect.ToolTip += "ERWC"; break;
                case (uint)PageProtection.PAGE_NOACCESS: rect.ToolTip += "----"; break;
            }
            rect.ToolTip += ((Protection & (uint)PageProtection.PAGE_GUARD) == (uint)PageProtection.PAGE_GUARD) ? "G" : "-";
            rect.ToolTip += ((Protection & (uint)PageProtection.PAGE_NOCACHE) == (uint)PageProtection.PAGE_NOCACHE) ? "N" : "-";
            rect.ToolTip += ((Protection & (uint)PageProtection.PAGE_WRITECOMBINE) == (uint)PageProtection.PAGE_WRITECOMBINE) ? "W" : "-";
            rect.ToolTip += (IsShared)?"\nShared":"";
            rect.ToolTip += (String.IsNullOrEmpty(MappedFileName)) ? "" : "\n" + MappedFileName;

            MemMapPanel.Children.Add(rect);
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
        private void LoadData(String FileName)
        {
            using (FileStream stream = new FileStream(FileName, FileMode.Open, FileAccess.Read))
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
                //finally
                //{
                //    reader.Close();
                //}


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
        private List<MemoryRegion> mem_data;
    }
}
