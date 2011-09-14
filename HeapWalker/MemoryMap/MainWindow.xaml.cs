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

namespace MemoryMap
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            mem_data = new List<VMQUERY>();
            InitializeComponent();
            LoadData(@"f:\dump.mem");
            UInt64 max_region_size = mem_data.Max(data => data.RgnSize);
            foreach (VMQUERY data in mem_data)
            {
                Rectangle rect = new Rectangle();
                SolidColorBrush ColorBrush = new SolidColorBrush();

                rect.StrokeThickness = 1;
                rect.Stroke = Brushes.Black;

                rect.Width = 10 + data.BlkSize * (40 / max_region_size);
                rect.Height = 10;
                rect.HorizontalAlignment = System.Windows.HorizontalAlignment.Left;
                rect.VerticalAlignment = System.Windows.VerticalAlignment.Top;
                rect.RadiusX = 1;
                rect.RadiusY = 1;
                Thickness margin = new Thickness(1);
                rect.Margin = margin;
                rect.ToolTip = "Size: " + data.BlkSize.ToString() + "\nStorage: ";
                switch (data.dwRgnStorage)
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
                uint ProtectionFlag = data.dwBlkProtection & ~((uint)PageProtection.PAGE_GUARD | (uint)PageProtection.PAGE_NOCACHE | (uint)PageProtection.PAGE_WRITECOMBINE);
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
                rect.ToolTip += ((data.dwBlkProtection & (uint)PageProtection.PAGE_GUARD) == (uint)PageProtection.PAGE_GUARD) ? "G" : "-";
                rect.ToolTip += ((data.dwBlkProtection & (uint)PageProtection.PAGE_NOCACHE) == (uint)PageProtection.PAGE_NOCACHE) ? "N" : "-";
                rect.ToolTip += ((data.dwBlkProtection & (uint)PageProtection.PAGE_WRITECOMBINE) == (uint)PageProtection.PAGE_WRITECOMBINE) ? "W" : "-";

                MemMapPanel.Children.Add(rect);
            }
        }
        private void LoadData(String FileName)
        {
            using (FileStream stream = new FileStream(FileName, FileMode.Open, FileAccess.Read))
            {
                using (XmlReader reader = new XmlTextReader(stream))
                {
                    try
                    {
                        while (true)
                        {
                           
                            tmp.pvRgnBaseAddress = reader.ReadUInt64();

                            tmp.dwRgnProtection = reader.ReadUInt32();
                            tmp.RgnSize = reader.ReadUInt64();
                            tmp.dwRgnStorage = reader.ReadUInt32();
                            tmp.dwRgnBlocks = reader.ReadUInt32();
                            tmp.dwRgnGuardBlks = reader.ReadUInt32();
                            tmp.bRgnIsAStack = reader.ReadInt32() != 0;


                            tmp.pvBlkBaseAddress = reader.ReadUInt64();
                            tmp.dwBlkProtection = reader.ReadUInt32();
                            tmp.BlkSize = reader.ReadUInt64();
                            tmp.dwBlkStorage = reader.ReadUInt32();
                            mem_data.Add(tmp);
                        }
                    }
                    catch (EndOfStreamException e)
                    {
                    }
                    finally
                    {
                        reader.Close();
                    }

                }
            }
        }
        private List<MemoryRegion> mem_data;
    }
}
