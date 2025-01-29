using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace PC_Application
{
    public partial class Charts : Form
    {
        private MainWindow mainWindow;
        public Charts(MainWindow mainWindow)
        {
            InitializeComponent();
            this.mainWindow = mainWindow;
            this.Owner = this.mainWindow;
            this.Init();
        }

        private void Init()
        {
            this.LocationChanged += Charts_LocationChanged;
            this.Location = new Point(this.mainWindow.steeringWindow.Width + 2, this.mainWindow.Height - 4);
            this.InitCharts();
        }

        private void Charts_LocationChanged(object sender, EventArgs e)
        {
            if (!this.Focused)
                return;

            this.mainWindow.Left = this.mainWindow.steeringWindow.Left + this.mainWindow.steeringWindow.Width - this.mainWindow.Width;
            this.mainWindow.Top = this.mainWindow.steeringWindow.Top + this.mainWindow.steeringWindow.Height - 4;

            this.mainWindow.steeringWindow.Left = this.Left + this.Width - 12;
            this.mainWindow.steeringWindow.Top = this.mainWindow.steeringWindow.Top;
        }

        private void InitCharts()
        {
            this.ChartA.Series.RemoveAt(0);
            this.ChartA.Series.Add("Prędkość mierzona");
            this.ChartA.Series["Prędkość mierzona"].ChartType = System.Windows.Forms.DataVisualization.Charting.SeriesChartType.Line;
            this.ChartA.Series["Prędkość mierzona"].Points.Clear();
            this.ChartA.Series.Add("Prędkość zadana");
            this.ChartA.Series["Prędkość zadana"].ChartType = System.Windows.Forms.DataVisualization.Charting.SeriesChartType.Line;
            this.ChartA.Series["Prędkość zadana"].Points.Clear();
            this.ChartA.Titles.Add("Prawy silnik");



            this.ChartB.Series.RemoveAt(0);
            this.ChartB.Series.Add("Prędkość mierzona");
            this.ChartB.Series["Prędkość mierzona"].ChartType = System.Windows.Forms.DataVisualization.Charting.SeriesChartType.Line;
            this.ChartB.Series["Prędkość mierzona"].Points.Clear();
            this.ChartB.Series.Add("Prędkość zadana");
            this.ChartB.Series["Prędkość zadana"].ChartType = System.Windows.Forms.DataVisualization.Charting.SeriesChartType.Line;
            this.ChartB.Series["Prędkość zadana"].Points.Clear();
            this.ChartB.Titles.Add("Lewy silnik");


            this.ChartC.Series.RemoveAt(0);
            this.ChartC.Series.Add("Prędkość mierzona");
            this.ChartC.Series["Prędkość mierzona"].ChartType = System.Windows.Forms.DataVisualization.Charting.SeriesChartType.Line;
            this.ChartC.Series["Prędkość mierzona"].Points.Clear();
            this.ChartC.Series.Add("Prędkość zadana");
            this.ChartC.Series["Prędkość zadana"].ChartType = System.Windows.Forms.DataVisualization.Charting.SeriesChartType.Line;
            this.ChartC.Series["Prędkość zadana"].Points.Clear();
            this.ChartC.Titles.Add("Wartość średnia");
        }

        public void UpdateCharts(List<int> motorASpeedsMeasured, List<int> motorBSpeedsMeasured,
                                 List<int> motorASpeedsSet, List<int> motorBSpeedsSet)
        {
            this.ChartA.Series["Prędkość mierzona"].Points.Clear();
            this.ChartA.Series["Prędkość zadana"].Points.Clear();
            this.ChartB.Series["Prędkość mierzona"].Points.Clear();
            this.ChartB.Series["Prędkość zadana"].Points.Clear();
            this.ChartC.Series["Prędkość mierzona"].Points.Clear();
            this.ChartC.Series["Prędkość zadana"].Points.Clear();

            this.Label_ChartA_M.Text = "Pomiar: " + motorASpeedsMeasured[motorASpeedsMeasured.Count - 1];
            this.Label_ChartA_S.Text = "Zadano: " + motorASpeedsMeasured[motorASpeedsSet.Count - 1];

            this.Label_ChartB_M.Text = "Pomiar: " + motorBSpeedsMeasured[motorBSpeedsMeasured.Count - 1];
            this.Label_ChartB_S.Text = "Zadano: " + motorBSpeedsMeasured[motorBSpeedsSet.Count - 1];

            int avgMeasured = (motorASpeedsMeasured[motorBSpeedsMeasured.Count - 1] + motorBSpeedsMeasured[motorBSpeedsMeasured.Count - 1]) / 2;
            int avgSet = (motorASpeedsSet[motorBSpeedsMeasured.Count - 1] + motorBSpeedsSet[motorBSpeedsMeasured.Count - 1]) / 2;
            this.Label_ChartC_M.Text = "Pomiar: " + avgMeasured;
            this.Label_ChartC_S.Text = "Zadano: " + avgSet;

            for (int i = 0; i < motorASpeedsMeasured.Count; i++)
            {
                this.ChartA.Series["Prędkość mierzona"].Points.AddXY(i, motorASpeedsMeasured[i]);
                this.ChartA.Series["Prędkość zadana"].Points.AddXY(i, motorASpeedsSet[i]);
            }

            for (int i = 0; i < motorBSpeedsMeasured.Count; i++)
            {
                this.ChartB.Series["Prędkość mierzona"].Points.AddXY(i, motorBSpeedsMeasured[i]);
                this.ChartB.Series["Prędkość zadana"].Points.AddXY(i, motorBSpeedsSet[i]);
            }

            for (int i = 0; i < motorASpeedsMeasured.Count; i++)
            {
                avgMeasured = (motorASpeedsMeasured[i] + motorBSpeedsMeasured[i]) / 2;
                avgSet = (motorASpeedsSet[i] + motorBSpeedsSet[i]) / 2;
                this.ChartC.Series["Prędkość mierzona"].Points.AddXY(i, avgMeasured);
                this.ChartC.Series["Prędkość zadana"].Points.AddXY(i, avgSet);
            }
        }
    }
}
