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

            this.mainWindow.steeringWindow.Left = this.mainWindow.Left - this.mainWindow.steeringWindow.Width + 12;
            this.mainWindow.steeringWindow.Top = this.mainWindow.Top;

            this.mainWindow.Left = this.Left;
            this.mainWindow.Top = this.Top - this.mainWindow.Height + 4;
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

            // Czyszczenie poprzednich danych
            this.ChartA.Series["Prędkość mierzona"].Points.Clear();
            this.ChartA.Series["Prędkość zadana"].Points.Clear();
            this.ChartB.Series["Prędkość mierzona"].Points.Clear();
            this.ChartB.Series["Prędkość zadana"].Points.Clear();
            this.ChartC.Series["Prędkość mierzona"].Points.Clear();
            this.ChartC.Series["Prędkość zadana"].Points.Clear();

            // Dodawanie punktów do wykresu A (lewy silnik)
            for (int i = 0; i < motorASpeedsMeasured.Count; i++)
            {
                this.ChartA.Series["Prędkość mierzona"].Points.AddXY(i, motorASpeedsMeasured[i]); // Pomiar A
                this.ChartA.Series["Prędkość zadana"].Points.AddXY(i, motorASpeedsSet[i]); // Zadana prędkość A
            }

            // Dodawanie punktów do wykresu B (prawy silnik)
            for (int i = 0; i < motorBSpeedsMeasured.Count; i++)
            {
                this.ChartB.Series["Prędkość mierzona"].Points.AddXY(i, motorBSpeedsMeasured[i]); // Pomiar B
                this.ChartB.Series["Prędkość zadana"].Points.AddXY(i, motorBSpeedsSet[i]); // Zadana prędkość B
            }

            // Dodawanie punktów do wykresu C (średnia)
            for (int i = 0; i < motorASpeedsMeasured.Count; i++)
            {
                int avgMeasured = (motorASpeedsMeasured[i] + motorBSpeedsMeasured[i]) / 2;
                int avgSet = (motorASpeedsSet[i] + motorBSpeedsSet[i]) / 2;
                this.ChartC.Series["Prędkość mierzona"].Points.AddXY(i, avgMeasured); // Średnia zmierzona
                this.ChartC.Series["Prędkość zadana"].Points.AddXY(i, avgSet); // Średnia zadana
            }
        }
    }
}
