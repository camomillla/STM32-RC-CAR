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
            this.PB_Pointer = new System.Windows.Forms.PictureBox();
            ((System.ComponentModel.ISupportInitialize)(this.PB_Pointer)).BeginInit();
            this.SuspendLayout();
            // 
            // PB_Pointer
            // 
            this.PB_Pointer.BackgroundImage = global::PC_Application.Properties.Resources.Pointer;
            this.PB_Pointer.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.PB_Pointer.Location = new System.Drawing.Point(272, 293);
            this.PB_Pointer.Name = "PB_Pointer";
            this.PB_Pointer.Size = new System.Drawing.Size(256, 256);
            this.PB_Pointer.TabIndex = 0;
            this.PB_Pointer.TabStop = false;
            // 
            // MainWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(784, 561);
            this.Controls.Add(this.PB_Pointer);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.MaximizeBox = false;
            this.Name = "MainWindow";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "PC-Application";
            ((System.ComponentModel.ISupportInitialize)(this.PB_Pointer)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.PictureBox PB_Pointer;
    }
}

