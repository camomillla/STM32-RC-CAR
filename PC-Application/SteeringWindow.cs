using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Windows.Input;

namespace PC_Application
{
    public partial class SteeringWindow : Form
    {
        private float pointerAngle = 0.0F;
        private float targetAngle = 0.0F;
        private Timer updateTimer;
        private Image originalPointerImage;

        private bool isKeyUpPressed = false;
        private bool isKeyDownPressed = false;
        private bool isKeyLeftPressed = false;
        private bool isKeyRightPressed = false;
        public SteeringWindow()
        {
            InitializeComponent();
            this.Init();
        }

        private void Init()
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

        private Boolean CheckKeys(System.Windows.Forms.KeyEventArgs e)
        {
            if (e.KeyCode != Keys.Up &&
                e.KeyCode != Keys.Down &&
                e.KeyCode != Keys.Left &&
                e.KeyCode != Keys.Right
                ) return false;
            return true;
        }

        private void MainWindow_KeyUp(object sender, System.Windows.Forms.KeyEventArgs e)
        {
            if (!CheckKeys(e))
                return;

                this.UpdateTargetAngle();

            if (!Keyboard.IsKeyDown(Key.Up) && Keyboard.IsKeyDown(Key.Left) && isKeyUpPressed && isKeyLeftPressed)
                MainWindow.SendCommand("MOTOR7");

            if (!Keyboard.IsKeyDown(Key.Up) && Keyboard.IsKeyDown(Key.Right) && isKeyUpPressed && isKeyRightPressed)
                MainWindow.SendCommand("MOTOR3");

            if (Keyboard.IsKeyDown(Key.Up) && !Keyboard.IsKeyDown(Key.Left) && isKeyUpPressed && isKeyLeftPressed)
                MainWindow.SendCommand("MOTOR1");

            if (Keyboard.IsKeyDown(Key.Up) && !Keyboard.IsKeyDown(Key.Right) && isKeyUpPressed && isKeyRightPressed)
                MainWindow.SendCommand("MOTOR1");

            if (isKeyUpPressed && !Keyboard.IsKeyDown(Key.Up))
                isKeyUpPressed = false;

            if (!Keyboard.IsKeyDown(Key.Down) && Keyboard.IsKeyDown(Key.Left) && isKeyDownPressed && isKeyLeftPressed)
                MainWindow.SendCommand("MOTOR7");

            if (!Keyboard.IsKeyDown(Key.Down) && Keyboard.IsKeyDown(Key.Right) && isKeyDownPressed && isKeyRightPressed)
                MainWindow.SendCommand("MOTOR3");

            if (Keyboard.IsKeyDown(Key.Down) && !Keyboard.IsKeyDown(Key.Left) && isKeyDownPressed && isKeyLeftPressed)
                MainWindow.SendCommand("MOTOR5");

            if (Keyboard.IsKeyDown(Key.Down) && !Keyboard.IsKeyDown(Key.Right) && isKeyDownPressed && isKeyRightPressed)
                MainWindow.SendCommand("MOTOR5");

            if (isKeyDownPressed && !Keyboard.IsKeyDown(Key.Down))
                isKeyDownPressed = false;

            if (isKeyLeftPressed && !Keyboard.IsKeyDown(Key.Left))
                isKeyLeftPressed = false;

            if (isKeyRightPressed && !Keyboard.IsKeyDown(Key.Right))
                isKeyRightPressed = false;


            if (!isKeyUpPressed &&
                !isKeyDownPressed &&
                !isKeyLeftPressed &&
                !isKeyRightPressed
                ) MainWindow.SendCommand("MOTOR0");
        }

        private void MainWindow_KeyDown(object sender, System.Windows.Forms.KeyEventArgs e)
        {
            if (!CheckKeys(e))
                return;

            this.UpdateTargetAngle();

            if ((!isKeyUpPressed || !isKeyLeftPressed) && Keyboard.IsKeyDown(Key.Up) && Keyboard.IsKeyDown(Key.Left))
            {
                isKeyUpPressed = true;
                isKeyLeftPressed = true;
                MainWindow.SendCommand("MOTOR8");
            }

            if ((!isKeyUpPressed || !isKeyRightPressed) && Keyboard.IsKeyDown(Key.Up) && Keyboard.IsKeyDown(Key.Right))
            {
                isKeyUpPressed = true;
                isKeyRightPressed = true;
                MainWindow.SendCommand("MOTOR2");
            }

            if (!isKeyUpPressed && Keyboard.IsKeyDown(Key.Up))
            {
                isKeyUpPressed = true;
                MainWindow.SendCommand("MOTOR1");
            }

            if ((!isKeyDownPressed || !isKeyLeftPressed) && Keyboard.IsKeyDown(Key.Down) && Keyboard.IsKeyDown(Key.Left))
            {
                isKeyDownPressed = true;
                isKeyLeftPressed = true;
                MainWindow.SendCommand("MOTOR6");
            }

            if ((!isKeyDownPressed || !isKeyRightPressed) && Keyboard.IsKeyDown(Key.Down) && Keyboard.IsKeyDown(Key.Right))
            {
                isKeyDownPressed = true;
                isKeyRightPressed = true;
                MainWindow.SendCommand("MOTOR4");
            }

            if (!isKeyDownPressed && Keyboard.IsKeyDown(Key.Down))
            {
                isKeyDownPressed = true;
                MainWindow.SendCommand("MOTOR5");
            }

            if (!isKeyLeftPressed && Keyboard.IsKeyDown(Key.Left))
            {
                isKeyLeftPressed = true;
                MainWindow.SendCommand("MOTOR7");
            }

            if (!isKeyRightPressed && Keyboard.IsKeyDown(Key.Right))
            {
                isKeyRightPressed = true;
                MainWindow.SendCommand("MOTOR3");
            }
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
    }
}
