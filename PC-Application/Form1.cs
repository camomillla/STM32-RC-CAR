using PC_Application.Properties;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Windows.Input;

namespace PC_Application
{
    public partial class MainWindow : Form
    {

        private float pointerAngle = 0.0F;
        private float targetAngle = 0.0F;
        private Timer updateTimer;
        private Image originalPointerImage;

        public MainWindow()
        {
            InitializeComponent();
            this.Init();
        }

        public void Init()
        {
            this.originalPointerImage = this.PB_Pointer.BackgroundImage;

            this.KeyDown += MainWindow_KeyDown;
            this.KeyUp += MainWindow_KeyUp;

            this.updateTimer = new Timer
            {
                Interval = 16
            };
            this.updateTimer.Tick += UpdateTimer_Tick;
            this.updateTimer.Start();
        }

        public void RotateImage(float angle)
        {
            Bitmap rotatedImg = new Bitmap(this.originalPointerImage.Width, this.originalPointerImage.Height);
            rotatedImg.SetResolution(this.originalPointerImage.HorizontalResolution, this.originalPointerImage.VerticalResolution);

            Graphics g = Graphics.FromImage(rotatedImg);
            //g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.AntiAlias;
            //g.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.HighQualityBicubic;
            g.TranslateTransform(this.originalPointerImage.Width / 2.0F, this.originalPointerImage.Height / 2.0F);
            g.RotateTransform(angle);
            g.TranslateTransform(-this.originalPointerImage.Width / 2.0F, -this.originalPointerImage.Height / 2.0F);
            g.DrawImage(this.originalPointerImage, new PointF(0, 0));

            this.PB_Pointer.BackgroundImage = rotatedImg;
        }

        private void UpdateTargetAngle()
        {
            bool keyUp = Keyboard.IsKeyDown(Key.Up);
            bool keyDown = Keyboard.IsKeyDown(Key.Down);
            bool keyLeft = Keyboard.IsKeyDown(Key.Left);
            bool keyRight = Keyboard.IsKeyDown(Key.Right);

            if (keyUp && !keyDown && !keyLeft && !keyRight)
                this.targetAngle = 0.0F;
            else if (keyUp && keyLeft)
                this.targetAngle = -45.0F;
            else if (keyUp && keyRight)
                this.targetAngle = 45.0F;
            else if (!keyUp && keyDown && !keyLeft && !keyRight)
                this.targetAngle = 180.0F;
            else if (keyDown && keyLeft)
                this.targetAngle = -135.0F;
            else if (keyDown && keyRight)
                this.targetAngle = 135.0F;
            else if (keyLeft && !keyUp && !keyDown)
                this.targetAngle = -90.0F;
            else if (keyRight && !keyUp && !keyDown)
                this.targetAngle = 90.0F;
            else
                this.targetAngle = 0.0F;

            //PB_Pointer.Image?.Dispose();
            //PB_Pointer.Image = RotateImage(this.originalPointerImage, this.pointerAngle);
        }

        private void MainWindow_KeyUp(object sender, System.Windows.Forms.KeyEventArgs e)
        {
            this.UpdateTargetAngle();
        }

        private void MainWindow_KeyDown(object sender, System.Windows.Forms.KeyEventArgs e)
        {
            this.UpdateTargetAngle();
        }

        private void UpdateTimer_Tick(object sender, EventArgs e)
        {
            const float angleSpeed = 5.0F;
            if (Math.Abs(this.pointerAngle - this.targetAngle) > angleSpeed)
            {
                if (this.pointerAngle < this.targetAngle)
                    this.pointerAngle += angleSpeed;
                else this.pointerAngle -= angleSpeed;
            }
            else
                this.pointerAngle = this.targetAngle;
            this.Invalidate();
            this.RotateImage(this.pointerAngle);
        }
    }
}
