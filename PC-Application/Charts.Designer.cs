namespace PC_Application
{
    partial class Charts
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.Windows.Forms.DataVisualization.Charting.ChartArea chartArea1 = new System.Windows.Forms.DataVisualization.Charting.ChartArea();
            System.Windows.Forms.DataVisualization.Charting.Legend legend1 = new System.Windows.Forms.DataVisualization.Charting.Legend();
            System.Windows.Forms.DataVisualization.Charting.Series series1 = new System.Windows.Forms.DataVisualization.Charting.Series();
            System.Windows.Forms.DataVisualization.Charting.ChartArea chartArea2 = new System.Windows.Forms.DataVisualization.Charting.ChartArea();
            System.Windows.Forms.DataVisualization.Charting.Legend legend2 = new System.Windows.Forms.DataVisualization.Charting.Legend();
            System.Windows.Forms.DataVisualization.Charting.Series series2 = new System.Windows.Forms.DataVisualization.Charting.Series();
            System.Windows.Forms.DataVisualization.Charting.ChartArea chartArea3 = new System.Windows.Forms.DataVisualization.Charting.ChartArea();
            System.Windows.Forms.DataVisualization.Charting.Legend legend3 = new System.Windows.Forms.DataVisualization.Charting.Legend();
            System.Windows.Forms.DataVisualization.Charting.Series series3 = new System.Windows.Forms.DataVisualization.Charting.Series();
            this.ChartA = new System.Windows.Forms.DataVisualization.Charting.Chart();
            this.ChartB = new System.Windows.Forms.DataVisualization.Charting.Chart();
            this.ChartC = new System.Windows.Forms.DataVisualization.Charting.Chart();
            ((System.ComponentModel.ISupportInitialize)(this.ChartA)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.ChartB)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.ChartC)).BeginInit();
            this.SuspendLayout();
            // 
            // ChartA
            // 
            chartArea1.Name = "ChartArea1";
            this.ChartA.ChartAreas.Add(chartArea1);
            legend1.Name = "Legend1";
            this.ChartA.Legends.Add(legend1);
            this.ChartA.Location = new System.Drawing.Point(12, 12);
            this.ChartA.Name = "ChartA";
            series1.ChartArea = "ChartArea1";
            series1.Legend = "Legend1";
            series1.Name = "Series1";
            this.ChartA.Series.Add(series1);
            this.ChartA.Size = new System.Drawing.Size(360, 300);
            this.ChartA.TabIndex = 0;
            this.ChartA.Text = "Lewy silnik";
            // 
            // ChartB
            // 
            chartArea2.Name = "ChartArea1";
            this.ChartB.ChartAreas.Add(chartArea2);
            legend2.Name = "Legend1";
            this.ChartB.Legends.Add(legend2);
            this.ChartB.Location = new System.Drawing.Point(380, 12);
            this.ChartB.Name = "ChartB";
            series2.ChartArea = "ChartArea1";
            series2.Legend = "Legend1";
            series2.Name = "Series1";
            this.ChartB.Series.Add(series2);
            this.ChartB.Size = new System.Drawing.Size(360, 300);
            this.ChartB.TabIndex = 1;
            this.ChartB.Text = "Prawy silnik";
            // 
            // ChartC
            // 
            chartArea3.Name = "ChartArea1";
            this.ChartC.ChartAreas.Add(chartArea3);
            legend3.Name = "Legend1";
            this.ChartC.Legends.Add(legend3);
            this.ChartC.Location = new System.Drawing.Point(746, 12);
            this.ChartC.Name = "ChartC";
            series3.ChartArea = "ChartArea1";
            series3.Legend = "Legend1";
            series3.Name = "Series1";
            this.ChartC.Series.Add(series3);
            this.ChartC.Size = new System.Drawing.Size(360, 300);
            this.ChartC.TabIndex = 2;
            this.ChartC.Text = "Średnia";
            // 
            // Charts
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1118, 345);
            this.ControlBox = false;
            this.Controls.Add(this.ChartC);
            this.Controls.Add(this.ChartB);
            this.Controls.Add(this.ChartA);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "Charts";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.Manual;
            this.Text = "Charts";
            ((System.ComponentModel.ISupportInitialize)(this.ChartA)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.ChartB)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.ChartC)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.DataVisualization.Charting.Chart ChartA;
        private System.Windows.Forms.DataVisualization.Charting.Chart ChartB;
        private System.Windows.Forms.DataVisualization.Charting.Chart ChartC;
    }
}