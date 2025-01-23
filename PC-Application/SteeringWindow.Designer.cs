namespace PC_Application
{
    partial class SteeringWindow
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
            this.PB_Pointer = new System.Windows.Forms.PictureBox();
            ((System.ComponentModel.ISupportInitialize)(this.PB_Pointer)).BeginInit();
            this.SuspendLayout();
            // 
            // PB_Pointer
            // 
            this.PB_Pointer.BackgroundImage = global::PC_Application.Properties.Resources.Pointer;
            this.PB_Pointer.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.PB_Pointer.Location = new System.Drawing.Point(254, 146);
            this.PB_Pointer.Name = "PB_Pointer";
            this.PB_Pointer.Size = new System.Drawing.Size(256, 256);
            this.PB_Pointer.TabIndex = 1;
            this.PB_Pointer.TabStop = false;
            // 
            // SteeringWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(784, 561);
            this.ControlBox = false;
            this.Controls.Add(this.PB_Pointer);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "SteeringWindow";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "SteeringWindow";
            ((System.ComponentModel.ISupportInitialize)(this.PB_Pointer)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.PictureBox PB_Pointer;
    }
}