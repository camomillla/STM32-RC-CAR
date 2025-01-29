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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(SteeringWindow));
            this.PB_ABS = new System.Windows.Forms.PictureBox();
            this.PB_CarKey = new System.Windows.Forms.PictureBox();
            this.PB_Ignition = new System.Windows.Forms.PictureBox();
            this.PB_Lights = new System.Windows.Forms.PictureBox();
            this.PB_Horn = new System.Windows.Forms.PictureBox();
            this.PB_Speedometer_Arrow = new System.Windows.Forms.PictureBox();
            this.PB_Speedometer_Back = new System.Windows.Forms.PictureBox();
            this.PB_Pointer = new System.Windows.Forms.PictureBox();
            ((System.ComponentModel.ISupportInitialize)(this.PB_ABS)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.PB_CarKey)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.PB_Ignition)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.PB_Lights)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.PB_Horn)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.PB_Speedometer_Arrow)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.PB_Speedometer_Back)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.PB_Pointer)).BeginInit();
            this.SuspendLayout();
            // 
            // PB_ABS
            // 
            this.PB_ABS.BackColor = System.Drawing.Color.Transparent;
            this.PB_ABS.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.PB_ABS.Image = global::PC_Application.Properties.Resources.ABS;
            this.PB_ABS.Location = new System.Drawing.Point(402, 12);
            this.PB_ABS.Name = "PB_ABS";
            this.PB_ABS.Size = new System.Drawing.Size(128, 128);
            this.PB_ABS.TabIndex = 10;
            this.PB_ABS.TabStop = false;
            this.PB_ABS.MouseDown += new System.Windows.Forms.MouseEventHandler(this.PB_ABS_MouseDown);
            this.PB_ABS.MouseUp += new System.Windows.Forms.MouseEventHandler(this.PB_ABS_MouseUp);
            // 
            // PB_CarKey
            // 
            this.PB_CarKey.BackColor = System.Drawing.Color.Transparent;
            this.PB_CarKey.Image = global::PC_Application.Properties.Resources.CarKey;
            this.PB_CarKey.Location = new System.Drawing.Point(12, 12);
            this.PB_CarKey.Name = "PB_CarKey";
            this.PB_CarKey.Size = new System.Drawing.Size(128, 128);
            this.PB_CarKey.TabIndex = 9;
            this.PB_CarKey.TabStop = false;
            this.PB_CarKey.MouseDown += new System.Windows.Forms.MouseEventHandler(this.PB_CarKey_MouseDown);
            this.PB_CarKey.MouseUp += new System.Windows.Forms.MouseEventHandler(this.PB_CarKey_MouseUp);
            // 
            // PB_Ignition
            // 
            this.PB_Ignition.BackColor = System.Drawing.Color.Transparent;
            this.PB_Ignition.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.PB_Ignition.Image = ((System.Drawing.Image)(resources.GetObject("PB_Ignition.Image")));
            this.PB_Ignition.Location = new System.Drawing.Point(12, 12);
            this.PB_Ignition.Name = "PB_Ignition";
            this.PB_Ignition.Size = new System.Drawing.Size(128, 128);
            this.PB_Ignition.TabIndex = 8;
            this.PB_Ignition.TabStop = false;
            // 
            // PB_Lights
            // 
            this.PB_Lights.BackColor = System.Drawing.Color.Transparent;
            this.PB_Lights.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.PB_Lights.Image = global::PC_Application.Properties.Resources.Lights;
            this.PB_Lights.Location = new System.Drawing.Point(140, 12);
            this.PB_Lights.Name = "PB_Lights";
            this.PB_Lights.Size = new System.Drawing.Size(128, 128);
            this.PB_Lights.TabIndex = 7;
            this.PB_Lights.TabStop = false;
            this.PB_Lights.Click += new System.EventHandler(this.PB_Lights_Click);
            // 
            // PB_Horn
            // 
            this.PB_Horn.BackColor = System.Drawing.Color.Transparent;
            this.PB_Horn.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.PB_Horn.Image = global::PC_Application.Properties.Resources.Horn;
            this.PB_Horn.Location = new System.Drawing.Point(274, 12);
            this.PB_Horn.Name = "PB_Horn";
            this.PB_Horn.Size = new System.Drawing.Size(128, 128);
            this.PB_Horn.TabIndex = 6;
            this.PB_Horn.TabStop = false;
            this.PB_Horn.MouseDown += new System.Windows.Forms.MouseEventHandler(this.PB_Horn_MouseDown);
            this.PB_Horn.MouseUp += new System.Windows.Forms.MouseEventHandler(this.PB_Horn_MouseUp);
            // 
            // PB_Speedometer_Arrow
            // 
            this.PB_Speedometer_Arrow.BackColor = System.Drawing.Color.Transparent;
            this.PB_Speedometer_Arrow.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.PB_Speedometer_Arrow.Image = global::PC_Application.Properties.Resources.Speedometer_Arrow;
            this.PB_Speedometer_Arrow.Location = new System.Drawing.Point(12, 146);
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
            this.PB_Speedometer_Back.Location = new System.Drawing.Point(12, 146);
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
            this.PB_Pointer.Location = new System.Drawing.Point(274, 146);
            this.PB_Pointer.Name = "PB_Pointer";
            this.PB_Pointer.Size = new System.Drawing.Size(256, 256);
            this.PB_Pointer.TabIndex = 1;
            this.PB_Pointer.TabStop = false;
            // 
            // SteeringWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.BlanchedAlmond;
            this.ClientSize = new System.Drawing.Size(530, 404);
            this.ControlBox = false;
            this.Controls.Add(this.PB_ABS);
            this.Controls.Add(this.PB_CarKey);
            this.Controls.Add(this.PB_Ignition);
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
            this.StartPosition = System.Windows.Forms.FormStartPosition.Manual;
            this.Text = "Panel sterowniczy";
            ((System.ComponentModel.ISupportInitialize)(this.PB_ABS)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.PB_CarKey)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.PB_Ignition)).EndInit();
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
        private System.Windows.Forms.PictureBox PB_Ignition;
        private System.Windows.Forms.PictureBox PB_CarKey;
        private System.Windows.Forms.PictureBox PB_ABS;
    }
}