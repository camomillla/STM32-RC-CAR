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

        private static String ipAddress;
        private static UInt16 port;
        private static TcpClient tcpClient;
        private static NetworkStream networkStream;
        public static Boolean connectionStatus;
        public static Int16 heartBeatCheck;
        private SteeringWindow steeringWindow;

        public MainWindow()
        {
            InitializeComponent();
            this.Init();
        }

        public void Init()
        {
            this.steeringWindow = new SteeringWindow();

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

            MainWindow.ipAddress = this.TB_IPAddress.Text;
            MainWindow.port = Convert.ToUInt16(this.TB_Port.Text);

            try
            {
                tcpClient = new TcpClient();
                tcpClient.Connect(MainWindow.ipAddress, MainWindow.port);
                tcpClient.ReceiveTimeout = 5000;
                tcpClient.SendTimeout = 5000;
                networkStream = tcpClient.GetStream();
                Task.Run(() => ReceiveDataAsync());

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
            networkStream = null;
            tcpClient?.Close();

            MainWindow.connectionStatus = false;
            this.Button_Connect.Text = "Connect";
            this.TB_IPAddress.Enabled = true;
            this.TB_Port.Enabled = true;
            this.PB_Status.BackgroundImage = global::PC_Application.Properties.Resources.StopControl;
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
                networkStream = tcpClient.GetStream();
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

        public async Task ReceiveDataAsync()
        {
            try
            {

                if (tcpClient == null || !tcpClient.Connected)
                {
                    Console.WriteLine("Klient nie jest połączony.");
                    return;
                }

                NetworkStream stream = tcpClient.GetStream();

                byte[] buffer = new byte[1024];

                while (tcpClient.Connected)
                {
                    int bytesRead = await stream.ReadAsync(buffer, 0, buffer.Length);
                    if (bytesRead > 0)
                    {
                        string receivedData = Encoding.UTF8.GetString(buffer, 0, bytesRead);

                        if (receivedData.StartsWith("HB:"))
                        {
                            // Wyodrębnij część po prefiksie "HB:"
                            string numericPart = receivedData.Substring(3).Trim();

                            // Próbuj przekonwertować na liczbę i wypisz ją
                            if (int.TryParse(numericPart, out int speedValue))
                            {
                                steeringWindow.SetSpeedometer(speedValue);
                                this.steeringWindow.SetSpeedometer(Convert.ToInt16(speedValue));
                                Console.WriteLine($"Odebrano wartość: {speedValue}");
                            }
                        }
                    }
                    else
                    {
                        Console.WriteLine("Połączenie zamknięte przez klienta.");
                        break;
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Błąd podczas odbierania danych: {ex.Message}");
            }
            finally
            {
                Disconnect();
                Console.WriteLine("Połączenie zamknięte.");
            }
        }
    }
}