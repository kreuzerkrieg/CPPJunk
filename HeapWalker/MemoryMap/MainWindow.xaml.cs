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

namespace MemoryMap
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private enum PageProtection : uint
        {
            PAGE_NOACCESS = 0x01,
            PAGE_READONLY = 0x02,
            PAGE_READWRITE = 0x04,
            PAGE_WRITECOPY = 0x08,
            PAGE_EXECUTE = 0x10,
            PAGE_EXECUTE_READ = 0x20,
            PAGE_EXECUTE_READWRITE = 0x40,
            PAGE_EXECUTE_WRITECOPY = 0x80,
            PAGE_GUARD = 0x100,
            PAGE_NOCACHE = 0x200,
            PAGE_WRITECOMBINE = 0x400
        };

        private enum MemoryStorageType : uint
        {
            MEM_COMMIT = 0x1000,
            MEM_RESERVE = 0x2000,
            MEM_DECOMMIT = 0x4000,
            MEM_RELEASE = 0x8000,
            MEM_FREE = 0x10000,
            MEM_PRIVATE = 0x20000,
            MEM_MAPPED = 0x40000,
            MEM_RESET = 0x80000,
            MEM_TOP_DOWN = 0x100000,
            MEM_WRITE_WATCH = 0x200000,
            MEM_PHYSICAL = 0x400000,
            MEM_ROTATE = 0x800000,
            SEC_IMAGE = 0x1000000,
            MEM_IMAGE = SEC_IMAGE,
            MEM_LARGE_PAGES = 0x20000000,
            MEM_4MB_PAGES = 0x80000000
        };
        private struct VMQUERY
        {
            // Region information
            public UInt64 pvRgnBaseAddress;
            public UInt32 dwRgnProtection; // PAGE_*
            public UInt64 RgnSize;
            public UInt32 dwRgnStorage; // MEM_*: Free, Image, Mapped, Private
            public UInt32 dwRgnBlocks;
            public UInt32 dwRgnGuardBlks; // If > 0, region contains thread stack
            public Boolean bRgnIsAStack; // TRUE if region contains thread stack

            // Block information
            public UInt64 pvBlkBaseAddress;
            public UInt32 dwBlkProtection; // PAGE_*
            public UInt64 BlkSize;
            public UInt32 dwBlkStorage; // MEM_*: Free, Reserve, Image, Mapped, Private
        };

        public MainWindow()
        {
            mem_data = new List<VMQUERY>();
            InitializeComponent();
            LoadData(@"f:\dump.mem");
            foreach (VMQUERY data in mem_data)
            {
                Rectangle rect = new Rectangle();
                SolidColorBrush ColorBrush = new SolidColorBrush();

                rect.StrokeThickness = 1;
                rect.Stroke = Brushes.Black;

                rect.Width = Math.Max(10, data.BlkSize % 50);
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
                using (BinaryReader reader = new BinaryReader(stream))
                {
                    try
                    {
                        while (true)
                        {
                            VMQUERY tmp;
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
        private List<VMQUERY> mem_data;
    }
}
