using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace ClientAppC
{
    public partial class CSApp : Form
    {
        private string _baseUrl;
        private readonly HashSet<Keys> _heldArrowKeys = new HashSet<Keys>();
        private int _lastMotor = -1;

        public CSApp()
        {
            InitializeComponent();

            KeyPreview = true;

            buttonConnect.Click += (_, __) => Connect();
            buttonEngine.Click += (_, __) => SendEndpoint("/engine", showMissingAddressMessage: true);
            buttonHorn.Click += (_, __) => SendEndpoint("/horn", showMissingAddressMessage: true);
            buttonLights.Click += (_, __) => SendEndpoint("/lights", showMissingAddressMessage: true);

            HookMotorButton(buttonT, 2);
            HookMotorButton(buttonTR, 3);
            HookMotorButton(buttonR, 4);
            HookMotorButton(buttonBR, 5);
            HookMotorButton(buttonB, 6);
            HookMotorButton(buttonBL, 7);
            HookMotorButton(buttonL, 8);
            HookMotorButton(buttonTL, 9);
            buttonS.Click += (_, __) => SendMotor(1);

            KeyDown += CSApp_KeyDown;
            KeyUp += CSApp_KeyUp;
            Deactivate += (_, __) => EmergencyStop();
            FormClosing += (_, __) => EmergencyStop();
        }

        private void Connect()
        {
            var normalized = NormalizeBaseUrl(textBox1.Text);
            if (string.IsNullOrWhiteSpace(normalized))
            {
                MessageBox.Show("Wpisz IP/adres w polu tekstowym.", "Brak adresu", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            _baseUrl = normalized;
            SendEndpoint("/init", showMissingAddressMessage: true);
        }

        private void HookMotorButton(Button button, int motorId)
        {
            button.MouseDown += (_, __) => SendMotor(motorId);
            button.MouseUp += (_, __) => SendMotor(1);
            button.MouseLeave += (_, __) =>
            {
                if (Control.MouseButtons.HasFlag(MouseButtons.Left))
                    SendMotor(1);
            };
        }

        private void CSApp_KeyDown(object sender, KeyEventArgs e)
        {
            if (!IsArrowKey(e.KeyCode))
                return;

            e.Handled = true;
            e.SuppressKeyPress = true;

            if (_heldArrowKeys.Add(e.KeyCode))
                SendMotorForHeldArrowKeys();
        }

        private void CSApp_KeyUp(object sender, KeyEventArgs e)
        {
            if (!IsArrowKey(e.KeyCode))
                return;

            e.Handled = true;
            e.SuppressKeyPress = true;

            if (_heldArrowKeys.Remove(e.KeyCode))
                SendMotorForHeldArrowKeys();
        }

        private void SendMotorForHeldArrowKeys()
        {
            var motor = GetMotorFromHeldArrowKeys();
            if (motor == _lastMotor)
                return;

            _lastMotor = motor;
            SendMotor(motor);
        }

        private int GetMotorFromHeldArrowKeys()
        {
            var up = _heldArrowKeys.Contains(Keys.Up);
            var down = _heldArrowKeys.Contains(Keys.Down);
            var left = _heldArrowKeys.Contains(Keys.Left);
            var right = _heldArrowKeys.Contains(Keys.Right);

            if (up && left) return 9;
            if (up && right) return 3;
            if (down && left) return 7;
            if (down && right) return 5;
            if (up) return 2;
            if (down) return 6;
            if (left) return 8;
            if (right) return 4;
            return 1;
        }

        private static bool IsArrowKey(Keys key) =>
            key == Keys.Up || key == Keys.Down || key == Keys.Left || key == Keys.Right;

        private void EmergencyStop()
        {
            _heldArrowKeys.Clear();
            _lastMotor = -1;
            SendMotor(1);
        }

        private void SendMotor(int motorId)
        {
            SendEndpoint($"/motor/{motorId}", showMissingAddressMessage: false);
        }

        private void SendEndpoint(string endpoint, bool showMissingAddressMessage)
        {
            if (!TryGetBaseUrl(out var baseUrl, showMissingAddressMessage))
                return;

            if (string.IsNullOrWhiteSpace(endpoint))
                return;

            if (!endpoint.StartsWith("/"))
                endpoint = "/" + endpoint;

            RunCurlGet(baseUrl + endpoint);
        }

        private bool TryGetBaseUrl(out string baseUrl, bool showMissingAddressMessage)
        {
            if (!string.IsNullOrWhiteSpace(_baseUrl))
            {
                baseUrl = _baseUrl;
                return true;
            }

            var normalized = NormalizeBaseUrl(textBox1.Text);
            if (string.IsNullOrWhiteSpace(normalized))
            {
                if (showMissingAddressMessage)
                    MessageBox.Show("Wpisz IP/adres w polu tekstowym.", "Brak adresu", MessageBoxButtons.OK, MessageBoxIcon.Warning);

                baseUrl = string.Empty;
                return false;
            }

            _baseUrl = normalized;
            baseUrl = normalized;
            return true;
        }

        private static string NormalizeBaseUrl(string input)
        {
            var s = (input ?? string.Empty).Trim();
            if (string.IsNullOrWhiteSpace(s))
                return string.Empty;

            if (!s.Contains("://"))
                s = "http://" + s;

            return s.TrimEnd('/');
        }

        private static void RunCurlGet(string url)
        {
            Task.Run(() =>
            {
                try
                {
                    var psi = new ProcessStartInfo
                    {
                        FileName = "curl",
                        Arguments = $"-s -m 2 -X GET \"{url}\"",
                        UseShellExecute = false,
                        RedirectStandardOutput = true,
                        RedirectStandardError = true,
                        CreateNoWindow = true
                    };

                    var p = Process.Start(psi);
                    if (p == null)
                        return;

                    p.WaitForExit(2500);
                }
                catch
                {
                }
            });
        }
    }
}
