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
            this.Label_ChartA_M = new System.Windows.Forms.Label();
            this.Label_ChartA_S = new System.Windows.Forms.Label();
            this.Label_ChartB_S = new System.Windows.Forms.Label();
            this.Label_ChartB_M = new System.Windows.Forms.Label();
            this.Label_ChartC_S = new System.Windows.Forms.Label();
            this.Label_ChartC_M = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.ChartA)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.ChartB)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.ChartC)).BeginInit();
            this.SuspendLayout();
            // 
            // ChartA
            // 
            this.ChartA.BackColor = System.Drawing.Color.Transparent;
            this.ChartA.BackHatchStyle = System.Windows.Forms.DataVisualization.Charting.ChartHatchStyle.Cross;
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
            this.ChartA.Size = new System.Drawing.Size(600, 200);
            this.ChartA.TabIndex = 0;
            this.ChartA.Text = "Lewy silnik";
            // 
            // ChartB
            // 
            this.ChartB.BackColor = System.Drawing.Color.Transparent;
            this.ChartB.BackHatchStyle = System.Windows.Forms.DataVisualization.Charting.ChartHatchStyle.Cross;
            chartArea2.Name = "ChartArea1";
            this.ChartB.ChartAreas.Add(chartArea2);
            legend2.Name = "Legend1";
            this.ChartB.Legends.Add(legend2);
            this.ChartB.Location = new System.Drawing.Point(12, 213);
            this.ChartB.Name = "ChartB";
            this.ChartB.Palette = System.Windows.Forms.DataVisualization.Charting.ChartColorPalette.Bright;
            series2.ChartArea = "ChartArea1";
            series2.Legend = "Legend1";
            series2.Name = "Series1";
            this.ChartB.Series.Add(series2);
            this.ChartB.Size = new System.Drawing.Size(600, 200);
            this.ChartB.TabIndex = 1;
            this.ChartB.Text = "Prawy silnik";
            // 
            // ChartC
            // 
            this.ChartC.BackColor = System.Drawing.Color.Transparent;
            this.ChartC.BackHatchStyle = System.Windows.Forms.DataVisualization.Charting.ChartHatchStyle.Cross;
            chartArea3.Name = "ChartArea1";
            this.ChartC.ChartAreas.Add(chartArea3);
            legend3.Name = "Legend1";
            this.ChartC.Legends.Add(legend3);
            this.ChartC.Location = new System.Drawing.Point(12, 414);
            this.ChartC.Name = "ChartC";
            this.ChartC.Palette = System.Windows.Forms.DataVisualization.Charting.ChartColorPalette.Fire;
            series3.ChartArea = "ChartArea1";
            series3.Legend = "Legend1";
            series3.Name = "Series1";
            this.ChartC.Series.Add(series3);
            this.ChartC.Size = new System.Drawing.Size(600, 200);
            this.ChartC.TabIndex = 2;
            this.ChartC.Text = "Średnia";
            // 
            // Label_ChartA_M
            // 
            this.Label_ChartA_M.AutoSize = true;
            this.Label_ChartA_M.Font = new System.Drawing.Font("CMU Serif", 10F);
            this.Label_ChartA_M.Location = new System.Drawing.Point(483, 83);
            this.Label_ChartA_M.Name = "Label_ChartA_M";
            this.Label_ChartA_M.Size = new System.Drawing.Size(69, 18);
            this.Label_ChartA_M.TabIndex = 3;
            this.Label_ChartA_M.Text = "Pomiar: 0";
            // 
            // Label_ChartA_S
            // 
            this.Label_ChartA_S.AutoSize = true;
            this.Label_ChartA_S.Font = new System.Drawing.Font("CMU Serif", 10F);
            this.Label_ChartA_S.Location = new System.Drawing.Point(483, 116);
            this.Label_ChartA_S.Name = "Label_ChartA_S";
            this.Label_ChartA_S.Size = new System.Drawing.Size(70, 18);
            this.Label_ChartA_S.TabIndex = 4;
            this.Label_ChartA_S.Text = "Zadano: 0";
            // 
            // Label_ChartB_S
            // 
            this.Label_ChartB_S.AutoSize = true;
            this.Label_ChartB_S.Font = new System.Drawing.Font("CMU Serif", 10F);
            this.Label_ChartB_S.Location = new System.Drawing.Point(483, 323);
            this.Label_ChartB_S.Name = "Label_ChartB_S";
            this.Label_ChartB_S.Size = new System.Drawing.Size(70, 18);
            this.Label_ChartB_S.TabIndex = 6;
            this.Label_ChartB_S.Text = "Zadano: 0";
            // 
            // Label_ChartB_M
            // 
            this.Label_ChartB_M.AutoSize = true;
            this.Label_ChartB_M.Font = new System.Drawing.Font("CMU Serif", 10F);
            this.Label_ChartB_M.Location = new System.Drawing.Point(483, 290);
            this.Label_ChartB_M.Name = "Label_ChartB_M";
            this.Label_ChartB_M.Size = new System.Drawing.Size(69, 18);
            this.Label_ChartB_M.TabIndex = 5;
            this.Label_ChartB_M.Text = "Pomiar: 0";
            // 
            // Label_ChartC_S
            // 
            this.Label_ChartC_S.AutoSize = true;
            this.Label_ChartC_S.Font = new System.Drawing.Font("CMU Serif", 10F);
            this.Label_ChartC_S.Location = new System.Drawing.Point(483, 526);
            this.Label_ChartC_S.Name = "Label_ChartC_S";
            this.Label_ChartC_S.Size = new System.Drawing.Size(70, 18);
            this.Label_ChartC_S.TabIndex = 8;
            this.Label_ChartC_S.Text = "Zadano: 0";
            // 
            // Label_ChartC_M
            // 
            this.Label_ChartC_M.AutoSize = true;
            this.Label_ChartC_M.Font = new System.Drawing.Font("CMU Serif", 10F);
            this.Label_ChartC_M.Location = new System.Drawing.Point(483, 493);
            this.Label_ChartC_M.Name = "Label_ChartC_M";
            this.Label_ChartC_M.Size = new System.Drawing.Size(69, 18);
            this.Label_ChartC_M.TabIndex = 7;
            this.Label_ChartC_M.Text = "Pomiar: 0";
            // 
            // Charts
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.BlanchedAlmond;
            this.ClientSize = new System.Drawing.Size(614, 640);
            this.ControlBox = false;
            this.Controls.Add(this.Label_ChartC_S);
            this.Controls.Add(this.Label_ChartC_M);
            this.Controls.Add(this.Label_ChartB_S);
            this.Controls.Add(this.Label_ChartB_M);
            this.Controls.Add(this.Label_ChartA_S);
            this.Controls.Add(this.Label_ChartA_M);
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
            this.Text = "Wykresy silników";
            ((System.ComponentModel.ISupportInitialize)(this.ChartA)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.ChartB)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.ChartC)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.DataVisualization.Charting.Chart ChartA;
        private System.Windows.Forms.DataVisualization.Charting.Chart ChartB;
        private System.Windows.Forms.DataVisualization.Charting.Chart ChartC;
        private System.Windows.Forms.Label Label_ChartA_M;
        private System.Windows.Forms.Label Label_ChartA_S;
        private System.Windows.Forms.Label Label_ChartB_S;
        private System.Windows.Forms.Label Label_ChartB_M;
        private System.Windows.Forms.Label Label_ChartC_S;
        private System.Windows.Forms.Label Label_ChartC_M;
    }
}