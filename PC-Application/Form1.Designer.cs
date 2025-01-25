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
            this.TB_IPAddress = new System.Windows.Forms.TextBox();
            this.TB_Port = new System.Windows.Forms.TextBox();
            this.Label_IPAddress = new System.Windows.Forms.Label();
            this.Label_Port = new System.Windows.Forms.Label();
            this.PB_Status = new System.Windows.Forms.PictureBox();
            this.Button_Connect = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.PB_Status)).BeginInit();
            this.SuspendLayout();
            // 
            // TB_IPAddress
            // 
            this.TB_IPAddress.Location = new System.Drawing.Point(12, 12);
            this.TB_IPAddress.Name = "TB_IPAddress";
            this.TB_IPAddress.Size = new System.Drawing.Size(160, 20);
            this.TB_IPAddress.TabIndex = 1;
            // 
            // TB_Port
            // 
            this.TB_Port.Location = new System.Drawing.Point(178, 12);
            this.TB_Port.Name = "TB_Port";
            this.TB_Port.Size = new System.Drawing.Size(48, 20);
            this.TB_Port.TabIndex = 2;
            // 
            // Label_IPAddress
            // 
            this.Label_IPAddress.AutoSize = true;
            this.Label_IPAddress.Font = new System.Drawing.Font("CMU Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.Label_IPAddress.Location = new System.Drawing.Point(95, 35);
            this.Label_IPAddress.Name = "Label_IPAddress";
            this.Label_IPAddress.Size = new System.Drawing.Size(77, 18);
            this.Label_IPAddress.TabIndex = 3;
            this.Label_IPAddress.Text = "IP Address";
            // 
            // Label_Port
            // 
            this.Label_Port.AutoSize = true;
            this.Label_Port.Font = new System.Drawing.Font("CMU Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.Label_Port.Location = new System.Drawing.Point(191, 35);
            this.Label_Port.Name = "Label_Port";
            this.Label_Port.Size = new System.Drawing.Size(35, 18);
            this.Label_Port.TabIndex = 4;
            this.Label_Port.Text = "Port";
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
            this.Button_Connect.Location = new System.Drawing.Point(12, 56);
            this.Button_Connect.Name = "Button_Connect";
            this.Button_Connect.Size = new System.Drawing.Size(214, 23);
            this.Button_Connect.TabIndex = 6;
            this.Button_Connect.Text = "Connect";
            this.Button_Connect.UseVisualStyleBackColor = true;
            // 
            // MainWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(384, 217);
            this.Controls.Add(this.Button_Connect);
            this.Controls.Add(this.PB_Status);
            this.Controls.Add(this.Label_Port);
            this.Controls.Add(this.Label_IPAddress);
            this.Controls.Add(this.TB_Port);
            this.Controls.Add(this.TB_IPAddress);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.MaximizeBox = false;
            this.Name = "MainWindow";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "PC-Application";
            ((System.ComponentModel.ISupportInitialize)(this.PB_Status)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.TextBox TB_IPAddress;
        private System.Windows.Forms.TextBox TB_Port;
        private System.Windows.Forms.Label Label_IPAddress;
        private System.Windows.Forms.Label Label_Port;
        private System.Windows.Forms.PictureBox PB_Status;
        private System.Windows.Forms.Button Button_Connect;
    }
}

