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
using System.Text.RegularExpressions;
using System.Net.Sockets;
using System.Runtime.CompilerServices;

namespace PC_Application
{
    public partial class MainWindow : Form
    {

        private String ipAddress;
        private UInt16 port;
        private static TcpClient tcpClient;
        private static NetworkStream networkStream;
        public static Boolean connectionStatus;
        private SteeringWindow steeringWindow;

        public MainWindow()
        {
            InitializeComponent();
            this.Init();
        }

        public void Init()
        {
            this.steeringWindow = new SteeringWindow();
            this.steeringWindow.Hide();

            MainWindow.connectionStatus = false;
        }

        private Boolean CheckIPAddress()
        {
            String pattern = @"^(25[0-5]|2[0-4][0-9]|[0-1]?[0-9]?[0-9])\.(25[0-5]|2[0-4][0-9]|[0-1]?[0-9]?[0-9])\.(25[0-5]|2[0-4][0-9]|[0-1]?[0-9]?[0-9])\.(25[0-5]|2[0-4][0-9]|[0-1]?[0-9]?[0-9])$";
            return Regex.IsMatch(this.TB_IPAddress.Text, pattern);
        }

        private Boolean CheckPort()
        {
            return Convert.ToUInt16(this.TB_Port.Text) < 65535;
        }

        private void Connect()
        {
            if (!CheckIPAddress())
            {
                MessageBox.Show("Invalid IP Address");
                return;
            }

            if (!CheckPort())
            {
                MessageBox.Show("Invalid Port");
                return;
            }

            this.ipAddress = this.TB_IPAddress.Text;
            this.port = Convert.ToUInt16(this.TB_Port.Text);

            try
            {
                tcpClient = new TcpClient();
                tcpClient.Connect(this.ipAddress, this.port);

                networkStream = tcpClient.GetStream();
                byte[] data = Encoding.UTF8.GetBytes("LIGHTS");
                networkStream.Write(data, 0, data.Length);

                MainWindow.connectionStatus = true;
                this.Button_Connect.Text = "Disconnect";
                this.TB_IPAddress.Enabled = false;
                this.TB_Port.Enabled = false;
                this.PB_Status.BackgroundImage = global::PC_Application.Properties.Resources.StartControl;
                this.steeringWindow.Show();
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Błąd: {ex.Message}");
            }
        }

        private void Disconnect()
        {
            networkStream?.Close();
            tcpClient?.Close();

            MainWindow.connectionStatus = false;
            this.Button_Connect.Text = "Connect";
            this.TB_IPAddress.Enabled = true;
            this.TB_Port.Enabled = true;
            this.PB_Status.BackgroundImage = global::PC_Application.Properties.Resources.StopControl;
            this.steeringWindow.Hide();
        }

        private void Button_Connect_Click(object sender, EventArgs e)
        {
            if (!MainWindow.connectionStatus)
                this.Connect();

            else this.Disconnect();
            
        }

        public static void SendCommand(String command)
        {
            if (!MainWindow.connectionStatus)
                return;

            try
            {
                if (networkStream.CanWrite)
                {
                    byte[] data = Encoding.UTF8.GetBytes(command);
                    networkStream.Write(data, 0, data.Length);
                }
            }
            catch (IOException ex)
            {
                MessageBox.Show("Błąd zapisu do strumienia: " + ex.Message);
            }
            catch (ObjectDisposedException ex)
            {
                MessageBox.Show("Strumień został zamknięty: " + ex.Message);
            }
        }
    }
}
