using PC_Application.Properties;
using System;
using System.Collections.Generic;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
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
        private NetworkStream dataStream;

        // Diagnostyka podczas zestawiania sesji. Nie uruchamia napędu.
        private sealed class HandshakeProbe
        {
            public int BytesReceived;
            public readonly List<byte> FirstBytes = new List<byte>();
        }

        private static string HandshakeSample(HandshakeProbe probe)
        {
            lock (probe.FirstBytes)
                return probe.FirstBytes.Count == 0 ? "brak" : BitConverter.ToString(probe.FirstBytes.ToArray());
        }

        // Ramka 10-bajtowa, wysyłana przez STM32 po INIT oraz PING.
        private static readonly byte[] ReadyFrame = Encoding.ASCII.GetBytes("RCREADY01!");
        private static readonly byte[] InitCommand = Encoding.ASCII.GetBytes("INIT");
        private static readonly byte[] PingCommand = Encoding.ASCII.GetBytes("PING");
        private readonly object writeLock = new object();
        private System.Threading.Timer pingTimer;
        private long lastReplyTicks;
        private volatile int session = 0;
        private int disconnectScheduled = 0;
        private bool connecting = false;
        private Task receiveTask;

        public static volatile Boolean connectionStatus;
        public SteeringWindow steeringWindow;
        public Charts chartsWindow;

        public MainWindow()
        {
            InitializeComponent();
            this.Init();
            this.FormClosing += (sender, e) => this.DisconnectInternal();
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
            if (connecting || connectionStatus) return;
            try
            {
                this.CB_Bluetooth.Items.Clear();
                Array.Clear(this.btDeviceInfos, 0, this.btDeviceInfos.Length);
                if (this.btClient == null)
                    this.btClient = new BluetoothClient();
                ushort counter = 0;
                foreach (BluetoothDeviceInfo device in this.btClient.DiscoverDevices())
                {
                    if (counter >= this.btDeviceInfos.Length) break;
                    this.CB_Bluetooth.Items.Add(device.DeviceName);
                    this.btDeviceInfos[counter++] = device;
                }
                this.CB_Bluetooth.SelectedIndex = counter > 0 ? counter - 1 : -1;
                MessageBox.Show(counter + " bluetooth devices was found");
            }
            catch (Exception ex)
            {
                MessageBox.Show("Wyszukiwanie Bluetooth nie powiodło się: " + ex.Message);
            }
        }

        private async Task ConnectAsync()
        {
            if (connecting || connectionStatus || this.CB_Bluetooth.SelectedIndex < 0) return;
            BluetoothDeviceInfo selectedDevice = this.btDeviceInfos[this.CB_Bluetooth.SelectedIndex];
            if (selectedDevice == null) return;

            connecting = true;
            this.Button_Connect.Enabled = false;
            this.Button_Connect.Text = "Łączenie...";
            this.steeringWindow.ResetConnectionState();
            int currentSession = ++session;
            Interlocked.Exchange(ref disconnectScheduled, 0);
            BluetoothClient candidate = null;

            try
            {
                // Nie używamy klienta z DiscoverDevices ani starego, zamkniętego połączenia.
                if (this.btClient != null)
                {
                    this.btClient.Close();
                    this.btClient = null;
                }

                candidate = new BluetoothClient();
                this.btEndPoint = new BluetoothEndPoint(selectedDevice.DeviceAddress, BluetoothService.SerialPort);
                Task connectTask = Task.Run(() => candidate.Connect(btEndPoint));
                if (await Task.WhenAny(connectTask, Task.Delay(7000)) != connectTask)
                    throw new TimeoutException("Nie udało się zestawić RFCOMM w ciągu 7 s.");
                await connectTask;
                if (!candidate.Connected)
                    throw new IOException("RFCOMM nie zostało połączone.");

                NetworkStream stream = candidate.GetStream();
                if (stream == null || !stream.CanRead || !stream.CanWrite)
                    throw new IOException("Strumień Bluetooth jest niedostępny.");
                try { stream.WriteTimeout = 1000; } catch (Exception) { /* Opcjonalne ustawienie. */ }

                // Sam Connect() oznacza tylko zestawienie RFCOMM. HC-05 może
                // potrzebować chwili, nim zacznie przekazywać pierwsze bajty UART.
                // Rozpoczynamy odczyt PRZED wysłaniem INIT, aby nie zgubić ACK.
                var probe = new HandshakeProbe();
                Task readyTask = WaitForReadyAsync(stream, probe);
                await Task.Delay(350);

                int initAttempts = 0;
                // Ograniczona liczba ponowień; nie włączamy silników.
                // Jeśli poprzedni INIT zniknął przy starcie SPP, następny
                // zostanie przetworzony bez restartowania aplikacji i HC-05.
                while (!readyTask.IsCompleted && initAttempts < 10)
                {
                    initAttempts++;
                    Console.WriteLine("[BT] INIT próba " + initAttempts + "/10");
                    await stream.WriteAsync(InitCommand, 0, InitCommand.Length);
                    await Task.WhenAny(readyTask, Task.Delay(450));
                }

                if (!readyTask.IsCompleted)
                {
                    int count = Interlocked.CompareExchange(ref probe.BytesReceived, 0, 0);
                    string sample = HandshakeSample(probe);
                    Console.WriteLine("[BT] Handshake timeout; wysłane INIT=" + initAttempts
                                      + ", RX bajty=" + count + ", pierwsze bajty(hex)=" + sample);
                    throw new TimeoutException("STM32 nie potwierdził INIT po " + initAttempts
                        + " próbach (~5 s).\n"
                        + "Odebrano " + count + " bajtów od HC-05.\n"
                        + "Pierwsze bajty (HEX): " + sample + "\n"
                        + (count == 0
                          ? "Brak odpowiedzi: sprawdź czy NOWY firmware jest wgrany (nie tylko zbudowany), piny USART2 i zasilanie HC-05."
                          : "Przychodzą bajty, ale bez poprawnego RCREADY01!: sprawdź wersję firmware i konfigurację UART."));
                }
                await readyTask;
                Console.WriteLine("[BT] Handshake: INIT prób=" + initAttempts
                                  + ", RX bajtów=" + Interlocked.CompareExchange(ref probe.BytesReceived, 0, 0));

                if (currentSession != session) return;
                this.btClient = candidate;
                this.dataStream = stream;
                candidate = null;
                MainWindow.connectionStatus = true;
                Interlocked.Exchange(ref lastReplyTicks, DateTime.UtcNow.Ticks);

                this.Button_Connect.Text = "Disconnect";
                this.CB_Bluetooth.Enabled = false;
                this.Button_Search.Enabled = false;
                this.PB_Status.BackgroundImage = global::PC_Application.Properties.Resources.StartControl;
                this.steeringWindow.ResetConnectionState();
                this.steeringWindow.EnableControls(true);
                Console.WriteLine("[BT] STM32 potwierdził INIT – połączenie gotowe.");

                // Osobny timer - również gdy UI spowalniają wykresy/animacje.
                this.pingTimer = new System.Threading.Timer(PingTick, currentSession, 250, 250);
                this.receiveTask = ReceiveDataAsync(currentSession, stream);
            }
            catch (Exception ex)
            {
                Console.WriteLine("[BT] Connect failed: " + ex);
                this.DisconnectInternal();
                if (!this.IsDisposed)
                    MessageBox.Show(this, "Nie można potwierdzić komunikacji ze STM32:\n" + ex.Message,
                        "Bluetooth: brak odpowiedzi", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
            finally
            {
                if (candidate != null) candidate.Close();
                connecting = false;
                if (!this.IsDisposed)
                {
                    this.Button_Connect.Enabled = true;
                    if (!connectionStatus) this.Button_Connect.Text = "Connect";
                }
            }
        }

        // Podczas handshake ignorujemy ewentualne stare bajty telemetrii.
        // Szukamy dokładnie podpisu READY, nie zakładamy granic ramek RFCOMM.
        private static async Task WaitForReadyAsync(NetworkStream stream, HandshakeProbe probe)
        {
            byte[] one = new byte[1];
            int matched = 0;
            while (matched < ReadyFrame.Length)
            {
                int read = await stream.ReadAsync(one, 0, 1);
                if (read == 0) throw new IOException("Bluetooth rozłączony przed READY.");
                Interlocked.Increment(ref probe.BytesReceived);
                lock (probe.FirstBytes)
                {
                    if (probe.FirstBytes.Count < 20)
                        probe.FirstBytes.Add(one[0]);
                }
                if (one[0] == ReadyFrame[matched]) matched++;
                else matched = one[0] == ReadyFrame[0] ? 1 : 0;
            }
        }

        private void PingTick(object state)
        {
            int currentSession = (int)state;
            if (!connectionStatus || currentSession != session) return;

            // Gdy wątek WinForms/odczyt odpowiedzi się zawiesi, przestajemy
            // nadawać PING. STM32 sam zatrzyma PWM przez niezależny timeout.
            if (DateTime.UtcNow.Ticks - Interlocked.Read(ref lastReplyTicks) > TimeSpan.FromMilliseconds(3000).Ticks)
            {
                var timer = this.pingTimer;
                if (timer != null) timer.Change(Timeout.Infinite, Timeout.Infinite);
                ScheduleDisconnect(currentSession, "STM32 przestał odpowiadać na PING przez 3 s.");
                return;
            }
            try
            {
                lock (writeLock)
                {
                    if (!connectionStatus || currentSession != session || this.dataStream == null) return;
                    this.dataStream.Write(PingCommand, 0, PingCommand.Length);
                }
            }
            catch (Exception ex)
            {
                ScheduleDisconnect(currentSession, "Błąd wysyłania PING: " + ex.Message);
            }
        }

        private void ScheduleDisconnect(int currentSession, string reason)
        {
            if (currentSession != session) return;
            if (Interlocked.Exchange(ref disconnectScheduled, 1) != 0) return;
            // Nie podtrzymuj ruchu PING-ami, kiedy chcemy zakończyć sesję.
            var timer = this.pingTimer;
            if (timer != null) timer.Change(Timeout.Infinite, Timeout.Infinite);
            try
            {
                if (this.IsHandleCreated && !this.IsDisposed)
                {
                    this.BeginInvoke(new Action(() =>
                    {
                        if (currentSession != session || !connectionStatus) return;
                        Console.WriteLine("[BT] Utrata połączenia: " + reason);
                        DisconnectInternal();
                        MessageBox.Show(this, reason, "Utracono komunikację STM32",
                            MessageBoxButtons.OK, MessageBoxIcon.Warning);
                    }));
                }
            }
            catch (InvalidOperationException) { }
        }

        private void DisconnectInternal()
        {
            // Bezwarunkowo odcinamy UI od starego strumienia i timerów.
            ++session;
            MainWindow.connectionStatus = false;
            var oldTimer = pingTimer;
            pingTimer = null;
            if (oldTimer != null) oldTimer.Dispose();

            // Żądanie zatrzymania sesji; gdy go nie da się nadać, reaguje
            // firmware (1200 ms od ostatniej poprawnej komendy/PING).
            try
            {
                lock (writeLock)
                {
                    if (dataStream != null && dataStream.CanWrite)
                        dataStream.Write(InitCommand, 0, InitCommand.Length);
                }
            }
            catch (Exception) { }
            try { if (dataStream != null) dataStream.Close(); } catch (Exception) { }
            dataStream = null;
            try { if (btClient != null) btClient.Close(); } catch (Exception) { }
            btClient = null;

            this.Button_Connect.Text = "Connect";
            this.CB_Bluetooth.Enabled = true;
            this.Button_Search.Enabled = true;
            this.PB_Status.BackgroundImage = global::PC_Application.Properties.Resources.StopControl;
            this.steeringWindow.ResetConnectionState();
        }

        private void Button_Connect_Click(object sender, EventArgs e)
        {
            if (connecting) return;
            if (connectionStatus) this.DisconnectInternal();
            else if (this.CB_Bluetooth.SelectedIndex >= 0) { var task = this.ConnectAsync(); }
        }

        public void SendCommand(String command)
        {
            if (!MainWindow.connectionStatus || command == null || command.Length != 4) return;
            try
            {
                byte[] bytes = Encoding.ASCII.GetBytes(command);
                lock (writeLock)
                {
                    if (!MainWindow.connectionStatus || dataStream == null || !dataStream.CanWrite) return;
                    dataStream.Write(bytes, 0, bytes.Length);
                }
            }
            catch (Exception ex)
            {
                ScheduleDisconnect(session, "Błąd wysyłania komend: " + ex.Message);
            }
        }

        private List<int> motorASpeedsMeasured = new List<int>();
        private List<int> motorBSpeedsMeasured = new List<int>();
        private List<int> motorASpeedsSet = new List<int>();
        private List<int> motorBSpeedsSet = new List<int>();
        private const int maxHistorySize = 100;
        public int DistanceToObject = 0;

        private static async Task ReadExactlyAsync(NetworkStream stream, byte[] buffer)
        {
            int offset = 0;
            while (offset < buffer.Length)
            {
                int count = await stream.ReadAsync(buffer, offset, buffer.Length - offset);
                if (count == 0) throw new IOException("STM32 zamknął strumień Bluetooth.");
                offset += count;
            }
        }

        private async Task ReceiveDataAsync(int currentSession, NetworkStream stream)
        {
            try
            {
                while (MainWindow.connectionStatus && currentSession == session)
                {
                    byte[] frame = new byte[10];
                    await ReadExactlyAsync(stream, frame);
                    if (currentSession != session || !connectionStatus) break;
                    if (frame.SequenceEqual(ReadyFrame))
                    {
                        Interlocked.Exchange(ref lastReplyTicks, DateTime.UtcNow.Ticks);
                        continue;
                    }

                    short motorASpeedMeasured = BitConverter.ToInt16(frame, 0);
                    short motorBSpeedMeasured = BitConverter.ToInt16(frame, 2);
                    short motorASpeedSet = BitConverter.ToInt16(frame, 4);
                    short motorBSpeedSet = BitConverter.ToInt16(frame, 6);
                    this.DistanceToObject = BitConverter.ToInt16(frame, 8);

                    motorASpeedsMeasured.Add(motorASpeedMeasured);
                    motorBSpeedsMeasured.Add(motorBSpeedMeasured);
                    motorASpeedsSet.Add(motorASpeedSet);
                    motorBSpeedsSet.Add(motorBSpeedSet);
                    if (motorASpeedsMeasured.Count > maxHistorySize) motorASpeedsMeasured.RemoveAt(0);
                    if (motorBSpeedsMeasured.Count > maxHistorySize) motorBSpeedsMeasured.RemoveAt(0);
                    if (motorASpeedsSet.Count > maxHistorySize) motorASpeedsSet.RemoveAt(0);
                    if (motorBSpeedsSet.Count > maxHistorySize) motorBSpeedsSet.RemoveAt(0);

                    double avgMeasuredSpeed = (motorASpeedMeasured + motorBSpeedMeasured) / 2.0;
                    double avgSetSpeed = (motorASpeedSet + motorBSpeedSet) / 2.0;
                    //this.steeringWindow.SetSpeedometer((float)(avgMeasuredSpeed / 125.0 * 270.0));
                    this.steeringWindow.SetSpeedometer((float)(avgSetSpeed / 125.0 * 270.0));
                    Console.WriteLine($"HB:{motorASpeedMeasured};{motorBSpeedMeasured} -> Last Measured: {avgMeasuredSpeed}");
                    Console.WriteLine($"Set Speeds: {motorASpeedSet}; {motorBSpeedSet}");
                    Console.WriteLine($"Distance: {this.DistanceToObject}");
                    this.chartsWindow.UpdateCharts(this.motorASpeedsMeasured, this.motorBSpeedsMeasured,
                        this.motorASpeedsSet, this.motorBSpeedsSet);
                    this.steeringWindow.CheckDistanceCollision();
                }
            }
            catch (Exception ex)
            {
                ScheduleDisconnect(currentSession, "Błąd odbioru telemetrii: " + ex.Message);
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