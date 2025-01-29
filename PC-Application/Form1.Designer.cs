namespace PC_Application
{
    partial class MainWindow
    {
        /// <summary>
        /// Wymagana zmienna projektanta.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Wyczyść wszystkie używane zasoby.
        /// </summary>
        /// <param name="disposing">prawda, jeżeli zarządzane zasoby powinny zostać zlikwidowane; Fałsz w przeciwnym wypadku.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Kod generowany przez Projektanta formularzy systemu Windows

        /// <summary>
        /// Metoda wymagana do obsługi projektanta — nie należy modyfikować
        /// jej zawartości w edytorze kodu.
        /// </summary>
        private void InitializeComponent()
        {
            this.Label_Devices = new System.Windows.Forms.Label();
            this.PB_Status = new System.Windows.Forms.PictureBox();
            this.Button_Connect = new System.Windows.Forms.Button();
            this.CB_Bluetooth = new System.Windows.Forms.ComboBox();
            this.Button_Search = new System.Windows.Forms.Button();
            this.Button_Export = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.PB_Status)).BeginInit();
            this.SuspendLayout();
            // 
            // Label_Devices
            // 
            this.Label_Devices.AutoSize = true;
            this.Label_Devices.Font = new System.Drawing.Font("CMU Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.Label_Devices.Location = new System.Drawing.Point(55, 36);
            this.Label_Devices.Name = "Label_Devices";
            this.Label_Devices.Size = new System.Drawing.Size(183, 18);
            this.Label_Devices.TabIndex = 4;
            this.Label_Devices.Text = "Search for BlueTooth Device";
            // 
            // PB_Status
            // 
            this.PB_Status.BackgroundImage = global::PC_Application.Properties.Resources.StopControl;
            this.PB_Status.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.PB_Status.Location = new System.Drawing.Point(272, 12);
            this.PB_Status.Name = "PB_Status";
            this.PB_Status.Size = new System.Drawing.Size(100, 100);
            this.PB_Status.TabIndex = 5;
            this.PB_Status.TabStop = false;
            // 
            // Button_Connect
            // 
            this.Button_Connect.Location = new System.Drawing.Point(12, 57);
            this.Button_Connect.Name = "Button_Connect";
            this.Button_Connect.Size = new System.Drawing.Size(254, 23);
            this.Button_Connect.TabIndex = 6;
            this.Button_Connect.Text = "Connect";
            this.Button_Connect.UseVisualStyleBackColor = true;
            this.Button_Connect.Click += new System.EventHandler(this.Button_Connect_Click);
            // 
            // CB_Bluetooth
            // 
            this.CB_Bluetooth.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.CB_Bluetooth.FormattingEnabled = true;
            this.CB_Bluetooth.Location = new System.Drawing.Point(12, 12);
            this.CB_Bluetooth.Name = "CB_Bluetooth";
            this.CB_Bluetooth.Size = new System.Drawing.Size(150, 21);
            this.CB_Bluetooth.TabIndex = 7;
            // 
            // Button_Search
            // 
            this.Button_Search.Location = new System.Drawing.Point(168, 12);
            this.Button_Search.Name = "Button_Search";
            this.Button_Search.Size = new System.Drawing.Size(98, 23);
            this.Button_Search.TabIndex = 8;
            this.Button_Search.Text = "Search";
            this.Button_Search.UseVisualStyleBackColor = true;
            this.Button_Search.Click += new System.EventHandler(this.Button_Search_Click);
            // 
            // Button_Export
            // 
            this.Button_Export.Location = new System.Drawing.Point(272, 128);
            this.Button_Export.Name = "Button_Export";
            this.Button_Export.Size = new System.Drawing.Size(100, 27);
            this.Button_Export.TabIndex = 9;
            this.Button_Export.Text = "Export CSV";
            this.Button_Export.UseVisualStyleBackColor = true;
            this.Button_Export.Click += new System.EventHandler(this.Button_Export_Click);
            // 
            // MainWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.BlanchedAlmond;
            this.ClientSize = new System.Drawing.Size(384, 167);
            this.Controls.Add(this.Button_Export);
            this.Controls.Add(this.Button_Search);
            this.Controls.Add(this.CB_Bluetooth);
            this.Controls.Add(this.Button_Connect);
            this.Controls.Add(this.PB_Status);
            this.Controls.Add(this.Label_Devices);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.MaximizeBox = false;
            this.Name = "MainWindow";
            this.StartPosition = System.Windows.Forms.FormStartPosition.Manual;
            this.Text = "PC-Application";
            ((System.ComponentModel.ISupportInitialize)(this.PB_Status)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.Label Label_Devices;
        private System.Windows.Forms.PictureBox PB_Status;
        private System.Windows.Forms.Button Button_Connect;
        private System.Windows.Forms.ComboBox CB_Bluetooth;
        private System.Windows.Forms.Button Button_Search;
        private System.Windows.Forms.Button Button_Export;
    }
}

