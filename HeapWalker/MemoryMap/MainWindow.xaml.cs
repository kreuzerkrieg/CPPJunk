using System.Collections.Generic;
using System.Diagnostics;
using System.Windows;
using System.Windows.Controls;

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
                MemoryBlocksVisualizer MBViz = new MemoryBlocksVisualizer();
                MBViz.VisualizeMemoryData(MemMapPanel,2704);
            }
        }

        private static int CompareProcesses(Process lhs, Process rhs)
        {
            return lhs.ProcessName.CompareTo(rhs.ProcessName);
        }

        private void FillProcessList()
        {
            Process[] Processes = Process.GetProcesses();
            List<Process> ProcList = new List<Process>(Processes);
            ProcList.Sort(CompareProcesses);

            int i = 0;
            foreach (Process Proc in ProcList)
            {
                RowDefinition NewRow = new RowDefinition();
                NewRow.Height = new GridLength(0, GridUnitType.Auto);
                ProcessesGrid.RowDefinitions.Add(NewRow);
                Label ProcessName = new Label();
                Label ProcessId = new Label();
                ProcessName.Content = Proc.ProcessName;
                ProcessName.HorizontalAlignment = System.Windows.HorizontalAlignment.Left;
                ProcessName.VerticalAlignment = System.Windows.VerticalAlignment.Top;
                Grid.SetRow(ProcessName, i);
                Grid.SetColumn(ProcessName, 0);

                ProcessId.Content = Proc.Id;
                ProcessId.HorizontalAlignment = System.Windows.HorizontalAlignment.Right;
                ProcessId.VerticalAlignment = System.Windows.VerticalAlignment.Top;
                Grid.SetRow(ProcessId, i);
                Grid.SetColumn(ProcessId, 1);
                ProcessesGrid.Children.Add(ProcessName);
                ProcessesGrid.Children.Add(ProcessId);
                i++;
            }
        }
    }
}
