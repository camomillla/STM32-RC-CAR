using System;
using System.IO;
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
        private Form mainWindow;

        private float pointerAngle = 0.0F;
        private float targetAngle = 0.0F;

        private float speedometerArrowAngle = 0.0F;
        private float speedometerArrowTargetAngle = 0.0F;

        private float carKeyAngle = 0.0F;
        private float carKeyTargetAngle = 0.0F;

        private Timer updateTimer;
        private Image originalPointerImage;
        private Image originalArrowImage;
        private Image originalCarKeyImage;

        private bool isKeyUpPressed = false;
        private bool isKeyDownPressed = false;
        private bool isKeyLeftPressed = false;
        private bool isKeyRightPressed = false;

        private bool carIsRunning = false;

        private System.Media.SoundPlayer soundPlayer;
        public SteeringWindow(Form mainWindow)
        {
            InitializeComponent();
            this.mainWindow = mainWindow;
            this.Owner = this.mainWindow;
            this.Init();
        }

        private void Init()
        {
            this.originalPointerImage = this.PB_Pointer.Image;
            this.originalArrowImage = this.PB_Speedometer_Arrow.Image;
            this.originalCarKeyImage = this.PB_CarKey.Image;

            this.LocationChanged += SteeringWindow_LocationChanged;
            this.KeyDown += MainWindow_KeyDown;
            this.KeyUp += MainWindow_KeyUp;

            this.updateTimer = new Timer
            {
                Interval = 16
            };
            this.updateTimer.Tick += UpdateTimer_Tick;
            this.updateTimer.Start();
            this.DisableControls();

            this.soundPlayer = new System.Media.SoundPlayer();

            this.Location = new Point(0, 0);

            PB_Speedometer_Arrow.Parent = PB_Speedometer_Back;
            PB_Speedometer_Arrow.Location = Point.Empty;

            this.PB_CarKey.Parent = PB_Ignition;
            this.PB_CarKey.Location = Point.Empty;
        }

        public void EnableControls(Boolean onlyStart)
        {
            if (onlyStart)
            {
                this.PB_Ignition.BackColor = Color.Transparent;
                this.PB_Ignition.Enabled = true;
                return;
            }

            foreach (Control control in this.Controls)
            {

                if (control.GetType().Name != typeof(PictureBox).Name)
                {
                    continue;
                }

                control.Enabled = true;
                control.BackColor = Color.Transparent;
            }
        }

        public void DisableControls()
        {
            foreach (Control control in this.Controls)
            {
                if (control.GetType().Name != typeof(PictureBox).Name)
                {
                    continue;
                }

                control.Enabled = false;

                if (control != this.PB_Speedometer_Arrow &&
                    control != this.PB_CarKey)
                    control.BackColor = Color.Gray;
            }
        }

        private void SteeringWindow_LocationChanged(object sender, EventArgs e)
        {
            if (!this.Focused)
                return;

            this.mainWindow.Left = this.Right - 12;
            this.mainWindow.Top = this.Top;
        }

        private Boolean CheckKeys(System.Windows.Forms.KeyEventArgs e)
        {

            if (e.KeyCode == Keys.Enter ||
                e.KeyCode == Keys.H ||
                e.KeyCode == Keys.F ||
                e.KeyCode == Keys.ShiftKey) return true;

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

            if (e.KeyCode == Keys.H)
                this.PB_Horn_MouseUp(null, null);

            if (e.KeyCode == Keys.ShiftKey)
                this.PB_CarKey_MouseUp(null, null);

            if (!Keyboard.IsKeyDown(Key.Up) && Keyboard.IsKeyDown(Key.Left) && isKeyUpPressed && isKeyLeftPressed)
                this.CarCommand_Motor(7);

            if (!Keyboard.IsKeyDown(Key.Up) && Keyboard.IsKeyDown(Key.Right) && isKeyUpPressed && isKeyRightPressed)
                this.CarCommand_Motor(3);

            if (Keyboard.IsKeyDown(Key.Up) && !Keyboard.IsKeyDown(Key.Left) && isKeyUpPressed && isKeyLeftPressed)
                this.CarCommand_Motor(1);

            if (Keyboard.IsKeyDown(Key.Up) && !Keyboard.IsKeyDown(Key.Right) && isKeyUpPressed && isKeyRightPressed)
                this.CarCommand_Motor(1);

            if (isKeyUpPressed && !Keyboard.IsKeyDown(Key.Up))
                isKeyUpPressed = false;

            if (!Keyboard.IsKeyDown(Key.Down) && Keyboard.IsKeyDown(Key.Left) && isKeyDownPressed && isKeyLeftPressed)
                this.CarCommand_Motor(7);

            if (!Keyboard.IsKeyDown(Key.Down) && Keyboard.IsKeyDown(Key.Right) && isKeyDownPressed && isKeyRightPressed)
                this.CarCommand_Motor(3);

            if (Keyboard.IsKeyDown(Key.Down) && !Keyboard.IsKeyDown(Key.Left) && isKeyDownPressed && isKeyLeftPressed)
                this.CarCommand_Motor(5);

            if (Keyboard.IsKeyDown(Key.Down) && !Keyboard.IsKeyDown(Key.Right) && isKeyDownPressed && isKeyRightPressed)
                this.CarCommand_Motor(5);

            if (isKeyDownPressed && !Keyboard.IsKeyDown(Key.Down))
                isKeyDownPressed = false;

            if (isKeyLeftPressed && !Keyboard.IsKeyDown(Key.Left))
                isKeyLeftPressed = false;

            if (isKeyRightPressed && !Keyboard.IsKeyDown(Key.Right))
                isKeyRightPressed = false;


            if (!isKeyUpPressed &&
                !isKeyDownPressed &&
                !isKeyLeftPressed &&
                !isKeyRightPressed &&
                (e.KeyCode == Keys.Up ||
                e.KeyCode == Keys.Down ||
                e.KeyCode == Keys.Right ||
                e.KeyCode == Keys.Left)
                ) this.CarCommand_Motor(0);
        }

        private void MainWindow_KeyDown(object sender, System.Windows.Forms.KeyEventArgs e)
        {
            if (!CheckKeys(e))
                return;

            this.UpdateTargetAngle();

            if (e.KeyCode == Keys.F)
                this.PB_Lights_Click(null, null);

            if (e.KeyCode == Keys.H)
                this.PB_Horn_MouseDown(null, null);

            if (e.KeyCode == Keys.ShiftKey)
                this.PB_CarKey_MouseDown(null, null);

            if ((!isKeyUpPressed || !isKeyLeftPressed) && Keyboard.IsKeyDown(Key.Up) && Keyboard.IsKeyDown(Key.Left))
            {
                isKeyUpPressed = true;
                isKeyLeftPressed = true;
                this.CarCommand_Motor(8);
            }

            if ((!isKeyUpPressed || !isKeyRightPressed) && Keyboard.IsKeyDown(Key.Up) && Keyboard.IsKeyDown(Key.Right))
            {
                isKeyUpPressed = true;
                isKeyRightPressed = true;
                this.CarCommand_Motor(2);
            }

            if (!isKeyUpPressed && Keyboard.IsKeyDown(Key.Up))
            {
                isKeyUpPressed = true;
                this.CarCommand_Motor(1);
            }

            if ((!isKeyDownPressed || !isKeyLeftPressed) && Keyboard.IsKeyDown(Key.Down) && Keyboard.IsKeyDown(Key.Left))
            {
                isKeyDownPressed = true;
                isKeyLeftPressed = true;
                this.CarCommand_Motor(6);
            }

            if ((!isKeyDownPressed || !isKeyRightPressed) && Keyboard.IsKeyDown(Key.Down) && Keyboard.IsKeyDown(Key.Right))
            {
                isKeyDownPressed = true;
                isKeyRightPressed = true;
                this.CarCommand_Motor(4);
            }

            if (!isKeyDownPressed && Keyboard.IsKeyDown(Key.Down))
            {
                isKeyDownPressed = true;
                this.CarCommand_Motor(5);
            }

            if (!isKeyLeftPressed && Keyboard.IsKeyDown(Key.Left))
            {
                isKeyLeftPressed = true;
                this.CarCommand_Motor(7);
            }

            if (!isKeyRightPressed && Keyboard.IsKeyDown(Key.Right))
            {
                isKeyRightPressed = true;
                this.CarCommand_Motor(3);
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
        }

        private void UpdateTimer_Tick(object sender, EventArgs e)
        {
            const float angleSpeed = 5.0F;

            if (this.PB_Pointer.Enabled)
            {
                if (Math.Abs(this.pointerAngle - this.targetAngle) > angleSpeed / 2)
                {
                    if (this.pointerAngle < this.targetAngle)
                        this.pointerAngle += angleSpeed;
                    else this.pointerAngle -= angleSpeed;
                }
                else
                    this.pointerAngle = this.targetAngle;
            }

            if (Math.Abs(this.speedometerArrowAngle - this.speedometerArrowTargetAngle) > angleSpeed)
            {
                if (this.speedometerArrowAngle < this.speedometerArrowTargetAngle)
                    this.speedometerArrowAngle += angleSpeed;
                else this.speedometerArrowAngle -= angleSpeed;
            }
            else
                this.speedometerArrowAngle = this.speedometerArrowTargetAngle;

            //if (this.PB_CarKey.Enabled)
            {
                if (Math.Abs(this.carKeyAngle - this.carKeyTargetAngle) > angleSpeed)
                {
                    if (this.carKeyAngle < this.carKeyTargetAngle)
                        this.carKeyAngle += angleSpeed * 2.5F;
                    else this.carKeyAngle -= angleSpeed * 2.5F;
                }
                else
                    this.carKeyAngle = this.carKeyTargetAngle;
            }


            this.Invalidate();
            this.PB_Pointer.Image = RotateImage(originalPointerImage, this.pointerAngle);
            this.PB_Speedometer_Arrow.Image = RotateImage(originalArrowImage, this.speedometerArrowAngle);
            this.PB_CarKey.Image = RotateImage(originalCarKeyImage, this.carKeyAngle);
        }

        public Image RotateImage(Image original, float angle)
        {
            Bitmap rotatedImg = new Bitmap(original.Width, original.Height);
            rotatedImg.SetResolution(original.HorizontalResolution, original.VerticalResolution);

            Graphics g = Graphics.FromImage(rotatedImg);
            g.TranslateTransform(original.Width / 2.0F, original.Height / 2.0F);
            g.RotateTransform(angle);
            g.TranslateTransform(-original.Width / 2.0F, -original.Height / 2.0F);
            g.DrawImage(original, new PointF(0, 0));

            return rotatedImg;
        }

        private Boolean commandState_Lights = false;
        private Boolean commandState_Horn = false;
        private Boolean commandState_Engine = false;

        private void CarCommand_Lights()
        {
            if (!this.PB_Lights.Enabled)
                return;

            if (commandState_Lights)            
                this.PB_Lights.BackColor = Color.Transparent;
            else this.PB_Lights.BackColor= Color.Lime;

            this.commandState_Lights = !this.commandState_Lights;
            //MainWindow.SendCommand("LIGHTS");
        }

        private void CarCommand_Horn()
        {
            if (!this.PB_Horn.Enabled)
                return;

            if (!commandState_Horn)
                this.PB_Horn.BackColor = Color.Transparent;
            else this.PB_Horn.BackColor = Color.Lime;

            //MainWindow.SendCommand("HORN");
        }

        private void CarCommand_Motor(ushort motorID)
        {
            if (motorID != 0 &&
                !this.PB_Pointer.Enabled)
                return;

            if (motorID == 0 && this.PB_Ignition.Enabled)
            {
                this.PB_Pointer.BackColor = Color.Transparent;
            }
            else if (this.PB_Pointer.Enabled && this.PB_Ignition.Enabled)
            {
                this.PB_Pointer.BackColor = Color.Lime;
            }

            //MainWindow.SendCommand("MOTOR" + motorID);
        }

        private void CarCommand_Engine()
        {
            if (!this.PB_Ignition.Enabled)
                return;

            this.carIsRunning = !this.carIsRunning;

            if (carIsRunning)
            {
                this.carKeyTargetAngle = 135.0F;
                this.EnableControls(false);
                this.soundPlayer.SoundLocation = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "Ignition.wav");
                this.soundPlayer.Play();
            }
            else
            {
                this.carKeyTargetAngle = 0.0F;
                this.DisableControls();
                this.EnableControls(true);
                this.soundPlayer.SoundLocation = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "Ignition_Off.wav");
                this.soundPlayer.Play();
            }
        }

        private void PB_Lights_Click(object sender, EventArgs e)
        {
            this.CarCommand_Lights();
        }

        private void PB_Horn_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            this.commandState_Horn = true;
            this.CarCommand_Horn();
        }

        private void PB_Horn_MouseUp(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            this.commandState_Horn = false;
            this.CarCommand_Horn();
        }

        public void SetSpeedometer(float speed)
        {
            this.speedometerArrowTargetAngle = speed;
        }

        private void PB_CarKey_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (!this.PB_Ignition.Enabled)
                return;

            if (!this.commandState_Engine)
            {
                this.commandState_Engine = true;
                this.CarCommand_Engine();
            }
        }

        private void PB_CarKey_MouseUp(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            this.commandState_Engine = false;
        }
    }
}
