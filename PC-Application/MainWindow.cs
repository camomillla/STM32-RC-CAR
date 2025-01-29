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
        private BluetoothClient btClient;
        private BluetoothEndPoint btEndPoint;
        private BluetoothDeviceInfo[] btDeviceInfos;

        public static Boolean connectionStatus;
        public SteeringWindow steeringWindow;
        public Charts chartsWindow;

        public MainWindow()
        {
            InitializeComponent();
            this.Init();
        }

        public void Init()
        {
            this.steeringWindow = new SteeringWindow(this);
            this.chartsWindow = new Charts(this);
            this.steeringWindow.Show();
            this.chartsWindow.Show();

            this.LocationChanged += MainWindow_LocationChanged;

            this.btClient = new BluetoothClient();
            this.btDeviceInfos = new BluetoothDeviceInfo[32];

            MainWindow.connectionStatus = false;
            this.Location = new Point(this.steeringWindow.Width - this.Width, this.steeringWindow.Height);
        }

        private void MainWindow_LocationChanged(object sender, EventArgs e)
        {
            if (this.steeringWindow.Focused || this.chartsWindow.Focused)
                return;

            this.steeringWindow.Left = this.Left - this.steeringWindow.Width + this.Width;
            this.steeringWindow.Top = this.Top - this.steeringWindow.Height + 4;

            this.chartsWindow.Left = this.steeringWindow.Left + this.steeringWindow.Width - 12;
            this.chartsWindow.Top = this.steeringWindow.Top;
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

            this.SendCommand("INIT");
            await ReceiveDataAsync();
        }

        private void Disconnect()
        {
            if (this.btClient == null)
                throw new Exception("Could not disconnect the device!");

            this.SendCommand("INIT");
            MainWindow.connectionStatus = false;

            if (this.btClient.Connected)
                this.btClient.GetStream()?.Close();
            this.btClient.Close();
            this.btClient = null;

            this.Button_Connect.Text = "Connect";
            this.CB_Bluetooth.Enabled = true;
            this.Button_Search.Enabled = true;
            this.PB_Status.BackgroundImage = global::PC_Application.Properties.Resources.StopControl;
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
                MessageBox.Show($"Wystąpił błąd komunikacji Bluetooth:\n{ex.Message}", "Błąd komunikacji Bluetooth", MessageBoxButtons.OK, MessageBoxIcon.Error);
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

        private List<int> motorASpeedsMeasured = new List<int>();
        private List<int> motorBSpeedsMeasured = new List<int>();
        private List<int> motorASpeedsSet = new List<int>();
        private List<int> motorBSpeedsSet = new List<int>();
        private const int maxHistorySize = 100;

        private async Task ReceiveDataAsync()
        {
            try
            {
                while (MainWindow.connectionStatus && this.btClient != null && this.btClient.Connected)
                {
                    if (this.btClient != null && this.btClient.Connected)
                    {
                        NetworkStream stream = this.btClient.GetStream();
                        if (stream == null)
                            return;

                        byte[] buffer = new byte[8]; // 8 bajtów: 2 dla silnika A zmierzona prędkość, 2 dla silnika B zmierzona, 2 dla silnika A zadana prędkość, 2 dla silnika B zadana
                        int bytesRead;

                        bytesRead = await stream.ReadAsync(buffer, 0, buffer.Length);

                        if (bytesRead == 8) // Oczekujemy 8 bajtów
                        {
                            // Odczytujemy prędkości z otrzymanych bajtów (2 bajty dla każdego silnika)
                            short motorASpeedMeasured = BitConverter.ToInt16(buffer, 0);
                            short motorBSpeedMeasured = BitConverter.ToInt16(buffer, 2);
                            short motorASpeedSet = BitConverter.ToInt16(buffer, 4);
                            short motorBSpeedSet = BitConverter.ToInt16(buffer, 6);

                            motorASpeedsMeasured.Add(motorASpeedMeasured);
                            motorBSpeedsMeasured.Add(motorBSpeedMeasured);
                            motorASpeedsSet.Add(motorASpeedSet);
                            motorBSpeedsSet.Add(motorBSpeedSet);

                            if (motorASpeedsMeasured.Count > maxHistorySize)
                                motorASpeedsMeasured.RemoveAt(0);
                            if (motorBSpeedsMeasured.Count > maxHistorySize)
                                motorBSpeedsMeasured.RemoveAt(0);
                            if (motorASpeedsSet.Count > maxHistorySize)
                                motorASpeedsSet.RemoveAt(0);
                            if (motorBSpeedsSet.Count > maxHistorySize)
                                motorBSpeedsSet.RemoveAt(0);

                            double lastMotorASpeedMeasured = motorASpeedsMeasured.Last();
                            double lastMotorBSpeedMeasured = motorBSpeedsMeasured.Last();
                            double lastMotorASpeedSet = motorASpeedsSet.Last();
                            double lastMotorBSpeedSet = motorBSpeedsSet.Last();

                            this.Invoke(new Action(() =>
                            {
                                double avgMeasuredSpeed = (lastMotorASpeedMeasured + lastMotorBSpeedMeasured) / 2.0F;
                                this.steeringWindow.SetSpeedometer(((int)avgMeasuredSpeed) / 125.0F * 270.0F);

                                Console.WriteLine($"HB:{motorASpeedMeasured};{motorBSpeedMeasured} -> Last Measured: {avgMeasuredSpeed}");
                                Console.WriteLine($"Set Speeds: {lastMotorASpeedSet}; {lastMotorBSpeedSet}");
                                this.chartsWindow.UpdateCharts(this.motorASpeedsMeasured, this.motorBSpeedsMeasured, this.motorASpeedsSet, this.motorBSpeedsSet);
                            }));
                        }
                    }

                    await Task.Delay(50);
                }
            }
            catch (Exception ex)
            {
                //MessageBox.Show($"Error receiving data: {ex.Message}"); <- not important
            }
        }

        private void Button_Export_Click(object sender, EventArgs e)
        {
            try
            {
                using (var writer = new StreamWriter("Plots/MotorA.csv"))
                {
                    writer.WriteLine("Index,MotorA Measured,MotorA Set");
                    for (int i = 0; i < motorASpeedsMeasured.Count && i < maxHistorySize; i++)
                        writer.WriteLine($"{i + 1},{motorASpeedsMeasured[i]},{motorASpeedsSet[i]}");
                }

                using (var writer = new StreamWriter("Plots/MotorB.csv"))
                {
                    writer.WriteLine("Index,MotorB Measured,MotorB Set");
                    for (int i = 0; i < motorBSpeedsMeasured.Count && i < maxHistorySize; i++)
                        writer.WriteLine($"{i + 1},{motorBSpeedsMeasured[i]},{motorBSpeedsSet[i]}");
                }

                using (var writer = new StreamWriter("Plots/MotorAvg.csv"))
                {
                    writer.WriteLine("Index,Avg Measured,Avg Set");
                    for (int i = 0; i < motorASpeedsMeasured.Count && i < maxHistorySize; i++)
                    {
                        int avgMeasured = (motorASpeedsMeasured[i] + motorBSpeedsMeasured[i]) / 2;
                        int avgSet = (motorASpeedsSet[i] + motorBSpeedsSet[i]) / 2;
                        writer.WriteLine($"{i + 1},{avgMeasured},{avgSet}");
                    }
                }

                MessageBox.Show("Zapis do pliku .CSV udany", "Zapis wykresów", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Wystąpił błąd podczas zapisywania plików CSV:\n{ex.Message}", "Zapis wykresów", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }
    }
}