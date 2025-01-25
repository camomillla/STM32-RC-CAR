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
            this.steeringWindow = new SteeringWindow();
            this.btClient = new BluetoothClient();

            foreach (var u in this.btClient.DiscoverDevices()) {
                Console.WriteLine(u.DeviceName);
                if (u.DeviceName == "RC_Car") // PIN 0000
                {
                    this.btEndPoint = new BluetoothEndPoint(u.DeviceAddress, BluetoothService.SerialPort);
                    this.btClient.Connect(this.btEndPoint);

                    if (this.btClient.Connected)
                    {
                        Console.WriteLine("Connected!");
                    }
                    else Console.WriteLine("ERROR");
                }
            }


            MainWindow.connectionStatus = false;
        }        
    }
}