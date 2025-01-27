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
using DarrenLee.Bluetooth;
using InTheHand.Net.Sockets;
using InTheHand.Net;
using InTheHand.Net.Bluetooth;

namespace PC_Application
{
    public partial class MainWindow : Form
    {
        private Boolean IsActive;

        private BluetoothClient btClient;
        private BluetoothEndPoint btEndPoint;
        private BluetoothDeviceInfo[] btDeviceInfos;

        public static Boolean connectionStatus;
        private SteeringWindow steeringWindow;
        private Guid btGuid;

        public MainWindow()
        {
            InitializeComponent();
            this.Init();
        }

        public void Init()
        {
            this.steeringWindow = new SteeringWindow(this);
            this.steeringWindow.Show();

            this.LocationChanged += MainWindow_LocationChanged;

            this.btClient = new BluetoothClient();
            this.btDeviceInfos = new BluetoothDeviceInfo[32];

            MainWindow.connectionStatus = false;
            this.Location = new Point(this.steeringWindow.Width - 12, 0);
        }

        private void MainWindow_LocationChanged(object sender, EventArgs e)
        {
            if (this.steeringWindow.Focused)
                return;

            this.steeringWindow.Left = this.Left - this.steeringWindow.Width + 12;
            this.steeringWindow.Top = this.Top;
        }

        private void Button_Search_Click(object sender, EventArgs e)
        {
            this.CB_Bluetooth.Items.Clear();
            Array.Clear(this.btDeviceInfos, 0, this.btDeviceInfos.Length - 1);

            if (this.btClient == null)
                this.btClient = new BluetoothClient();

            ushort counter = 0;

            foreach (BluetoothDeviceInfo devInfo in this.btClient.DiscoverDevices())
            {
                this.CB_Bluetooth.Items.Add(devInfo.DeviceName);
                this.btDeviceInfos[counter++] = devInfo;
            }

            this.CB_Bluetooth.SelectedIndex = counter - 1;
            MessageBox.Show(counter + " bluetooth devices was found");
        }

        private async void Connect()
        {
            if (this.btClient == null)
                this.btClient = new BluetoothClient();

            this.btEndPoint = new BluetoothEndPoint(
                this.btDeviceInfos[this.CB_Bluetooth.SelectedIndex].DeviceAddress,
                BluetoothService.SerialPort
            );
            this.btClient.Connect(this.btEndPoint);

            if (!this.btClient.Connected)
                throw new Exception("Could not connect to the device!");

            this.Button_Connect.Text = "Disconnect";
            this.CB_Bluetooth.Enabled = false;
            this.Button_Search.Enabled = false;
            this.PB_Status.BackgroundImage = global::PC_Application.Properties.Resources.StartControl;
            MainWindow.connectionStatus = true;
            this.steeringWindow.EnableControls(true);

            await ReceiveDataAsync();
        }

        private void Disconnect()
        {
            if (this.btClient == null)
                throw new Exception("Could not disconnect the device!");

            if (this.btClient.Connected)
                this.btClient.GetStream()?.Close();
            this.btClient.Close();
            this.btClient = null;

            this.Button_Connect.Text = "Connect";
            this.CB_Bluetooth.Enabled = true;
            this.Button_Search.Enabled = true;
            this.PB_Status.BackgroundImage = global::PC_Application.Properties.Resources.StopControl;
            MainWindow.connectionStatus = false;
            this.steeringWindow.DisableControls();
        }

        private void Button_Connect_Click(object sender, EventArgs e)
        {
            if (this.CB_Bluetooth.SelectedIndex == -1)
                return;

            try
            {
                if (!MainWindow.connectionStatus)
                    this.Connect();
                else this.Disconnect();
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
            }
        }

        public void SendCommand(String command)
        {
            if (!MainWindow.connectionStatus)
                return;

            if (!this.btClient.GetStream().CanWrite ||
                this.btClient == null)
                return;

            this.btClient.GetStream().Write(Encoding.ASCII.GetBytes(command), 0, Encoding.ASCII.GetBytes(command).Length);
            this.btClient.GetStream().Flush();
        }

        private async Task ReceiveDataAsync()
        {
            try
            {
                while (MainWindow.connectionStatus)
                {
                    if (this.btClient != null && this.btClient.Connected)
                    {
                        NetworkStream stream = this.btClient.GetStream();

                        byte[] buffer = new byte[1024];
                        int bytesRead;

                        bytesRead = await stream.ReadAsync(buffer, 0, buffer.Length);

                        if (bytesRead > 0)
                        {
                            string receivedData = Encoding.ASCII.GetString(buffer, 0, bytesRead);

                            if (receivedData.StartsWith("HB:"))
                            {
                                string speedString = receivedData.Substring(3);
                                if (int.TryParse(speedString, out int speed))
                                {
                                    this.Invoke(new Action(() =>
                                    {
                                        this.steeringWindow.SetSpeedometer(speed / 125.0F * 270.0F);
                                        Console.WriteLine($"HB:{speed}");
                                    }));
                                }
                            }
                        }
                    }

                    await Task.Delay(50);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Error receiving data: {ex.Message}");
            }
        }
    }
}