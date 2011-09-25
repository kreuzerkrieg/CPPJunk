using System.Collections.Generic;
using System.Diagnostics;
using System.Windows;
using System.Windows.Controls;
using System.Collections.ObjectModel;

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
            }
        }

        private void FillProcessList()
        {
            Process[] Processes = Process.GetProcesses();
            ObservableCollection<Process> SortedProcList = new ObservableCollection<Process>(Processes);
            ProcDataGrid.DataContext = SortedProcList;
        }

        private void ProcDataGrid_MouseDoubleClick(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            DataGrid grid = (DataGrid)sender;
            Process CurrentProc = (Process)grid.CurrentItem;
            MemoryBlocksVisualizer MBViz = new MemoryBlocksVisualizer();
            MBViz.VisualizeMemoryData(MemMapPanel, CurrentProc.Id);
        }
    }
}
