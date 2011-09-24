using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows;
using System.Globalization;
using System.Windows.Shapes;

namespace MemoryMap
{
    class MemoryBlocksVisualizer
    {
        public void VisualizeMemoryData(Panel MemMapPanel, int ProcessId)
        {
            try
            {
                ProcessOutputRedirection POR = new ProcessOutputRedirection(ProcessId);
                StringBuilder ExecutableOutput = POR.ReadExecutableOutput();
                MemoryDataXmlParser MDXParser = new MemoryDataXmlParser();
                MDXParser.ParseXml(ExecutableOutput);
                UInt64 max_region_size = GetMaxSize(MDXParser.MemoryData);
                foreach (MemoryRegion data in MDXParser.MemoryData)
                {
                    if (data.NumberOfBlocks == 0)
                    {
                        MemMapPanel.Children.Add(
                            DrawMemoryRectangle(max_region_size, data.RegionSize, data.RegionStorageType, data.RegionProtection, false, "")
                            );
                    }
                    else
                    {
                        foreach (MemoryBlock block in data.MemoryBlocks)
                        {
                            MemMapPanel.Children.Add(
                                DrawMemoryRectangle(max_region_size, block.BlockSize, block.BlockStorageType, block.BlockProtection, block.IsShared, block.MappedFileName)
                                );
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
            NumberFormatInfo nfi = new CultureInfo("en-US", false).NumberFormat;
            nfi.NumberGroupSeparator = ",";
            nfi.NumberDecimalDigits = 0;
            //nfi.NumberGroupSizes = 3;
            RetVal.ToolTip = "Size: " + ((Size / (Double)1024).ToString("N", nfi)) + " Kb\nStorage: ";
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
            RetVal.ToolTip += (IsShared) ? "\nShared" : "";
            RetVal.ToolTip += (String.IsNullOrEmpty(MappedFileName)) ? "" : "\n" + MappedFileName;

            return RetVal;
        }

        private ulong GetMaxSize(List<MemoryRegion> mem_data)
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
    }
}
