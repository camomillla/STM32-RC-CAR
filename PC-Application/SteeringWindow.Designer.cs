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
            this.PB_Lights = new System.Windows.Forms.PictureBox();
            this.PB_Horn = new System.Windows.Forms.PictureBox();
            this.PB_Speedometer_Arrow = new System.Windows.Forms.PictureBox();
            this.PB_Speedometer_Back = new System.Windows.Forms.PictureBox();
            this.PB_Pointer = new System.Windows.Forms.PictureBox();
            ((System.ComponentModel.ISupportInitialize)(this.PB_Lights)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.PB_Horn)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.PB_Speedometer_Arrow)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.PB_Speedometer_Back)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.PB_Pointer)).BeginInit();
            this.SuspendLayout();
            // 
            // PB_Lights
            // 
            this.PB_Lights.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.PB_Lights.Image = global::PC_Application.Properties.Resources.Lights;
            this.PB_Lights.Location = new System.Drawing.Point(16, 281);
            this.PB_Lights.Name = "PB_Lights";
            this.PB_Lights.Size = new System.Drawing.Size(128, 128);
            this.PB_Lights.TabIndex = 7;
            this.PB_Lights.TabStop = false;
            // 
            // PB_Horn
            // 
            this.PB_Horn.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.PB_Horn.Image = global::PC_Application.Properties.Resources.Horn;
            this.PB_Horn.Location = new System.Drawing.Point(16, 409);
            this.PB_Horn.Name = "PB_Horn";
            this.PB_Horn.Size = new System.Drawing.Size(128, 128);
            this.PB_Horn.TabIndex = 6;
            this.PB_Horn.TabStop = false;
            // 
            // PB_Speedometer_Arrow
            // 
            this.PB_Speedometer_Arrow.BackColor = System.Drawing.Color.Transparent;
            this.PB_Speedometer_Arrow.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.PB_Speedometer_Arrow.Image = global::PC_Application.Properties.Resources.Speedometer_Arrow;
            this.PB_Speedometer_Arrow.Location = new System.Drawing.Point(150, 12);
            this.PB_Speedometer_Arrow.Name = "PB_Speedometer_Arrow";
            this.PB_Speedometer_Arrow.Size = new System.Drawing.Size(256, 256);
            this.PB_Speedometer_Arrow.TabIndex = 5;
            this.PB_Speedometer_Arrow.TabStop = false;
            // 
            // PB_Speedometer_Back
            // 
            this.PB_Speedometer_Back.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.PB_Speedometer_Back.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.PB_Speedometer_Back.Image = global::PC_Application.Properties.Resources.Speedometer;
            this.PB_Speedometer_Back.Location = new System.Drawing.Point(150, 12);
            this.PB_Speedometer_Back.Name = "PB_Speedometer_Back";
            this.PB_Speedometer_Back.Size = new System.Drawing.Size(256, 256);
            this.PB_Speedometer_Back.TabIndex = 4;
            this.PB_Speedometer_Back.TabStop = false;
            // 
            // PB_Pointer
            // 
            this.PB_Pointer.BackColor = System.Drawing.Color.Transparent;
            this.PB_Pointer.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.PB_Pointer.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.PB_Pointer.Image = global::PC_Application.Properties.Resources.Pointer;
            this.PB_Pointer.Location = new System.Drawing.Point(150, 281);
            this.PB_Pointer.Name = "PB_Pointer";
            this.PB_Pointer.Size = new System.Drawing.Size(256, 256);
            this.PB_Pointer.TabIndex = 1;
            this.PB_Pointer.TabStop = false;
            // 
            // SteeringWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(772, 549);
            this.ControlBox = false;
            this.Controls.Add(this.PB_Lights);
            this.Controls.Add(this.PB_Horn);
            this.Controls.Add(this.PB_Speedometer_Arrow);
            this.Controls.Add(this.PB_Speedometer_Back);
            this.Controls.Add(this.PB_Pointer);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "SteeringWindow";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "SteeringWindow";
            ((System.ComponentModel.ISupportInitialize)(this.PB_Lights)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.PB_Horn)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.PB_Speedometer_Arrow)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.PB_Speedometer_Back)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.PB_Pointer)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.PictureBox PB_Pointer;
        private System.Windows.Forms.PictureBox PB_Speedometer_Back;
        private System.Windows.Forms.PictureBox PB_Speedometer_Arrow;
        private System.Windows.Forms.PictureBox PB_Horn;
        private System.Windows.Forms.PictureBox PB_Lights;
    }
}