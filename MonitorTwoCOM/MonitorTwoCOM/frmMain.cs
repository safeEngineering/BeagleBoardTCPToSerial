using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO.Ports;
using System.Xml;
using System.IO;

using TracerX;
using MonitorTwoCOM.Comminication;

namespace MonitorTwoCOM
{
    public partial class frmMain : Form
    {
        #region Fields

        // Logging object
        private static readonly Logger log = Logger.GetLogger(typeof(frmMain));

        // first COM
        private SerialPort port1 = null;
        private COMSetting com1Setting = null;

        // second COM
        private SerialPort port2 = null;
        private COMSetting com2Setting = null;

        // the flag specifes whether sending data to GUI or not
        private bool sendDataToGUI = true;
        // the flag specifes whether tagging time on data from COM port or not
        private bool tagTime = true;

        // configuration data of board
        private BoardConfig boardConfig;
        // current configuration on connected board
        private BoardConfig connectedBoardConfig;

        // the event signals when receiving response from board
        private ManualResetEvent okResponseEvent = new ManualResetEvent(false);

        #endregion

        #region Form handlers

        public frmMain()
        {
            InitializeComponent();
        }

        private void frmMain_Load(object sender, EventArgs e)
        {
            try
            {
                // Assign the name of main thread
                Thread.CurrentThread.Name = "MainThread";

                // Disable standard data in text file
                Logger.StandardData.TextFileTraceLevel = TraceLevel.Off;
                // Text log file settings
                Logger.DefaultTextFile.Directory = @"%EXEDIR%\Logs";
                Logger.DefaultTextFile.AppendIfSmallerThanMb = 1;
                Logger.DefaultTextFile.MaxSizeMb = 100;
                Logger.DefaultTextFile.Archives = 99;
                Logger.DefaultTextFile.CircularStartSizeKb = 0;
                Logger.DefaultTextFile.CircularStartDelaySeconds = 0;
                Logger.DefaultTextFile.FullFilePolicy = FullFilePolicy.Roll;
                Logger.Root.TextFileTraceLevel = TraceLevel.Info;
                Logger.DefaultTextFile.FormatString = "{time:MM/dd/yyyy HH:mm:ss.fff} {level} {thname} {logger}+{method} {ind}{msg}";
                //Logger.DefaultTextFile.FormatString = "{level} {thname} {logger}+{method} {ind}{msg}";
                Logger.DefaultTextFile.Open();

                // Disable standard data in binary file
                Logger.StandardData.BinaryFileTraceLevel = TraceLevel.Off;
                // Binary log file settings
                Logger.DefaultBinaryFile.Directory = @"%EXEDIR%\Logs";
                Logger.DefaultBinaryFile.AppendIfSmallerThanMb = 1;
                Logger.DefaultBinaryFile.MaxSizeMb = 100;
                Logger.DefaultBinaryFile.Archives = 99;
                Logger.DefaultBinaryFile.CircularStartSizeKb = 0;
                Logger.DefaultBinaryFile.CircularStartDelaySeconds = 0;
                Logger.DefaultBinaryFile.FullFilePolicy = FullFilePolicy.Roll;
                Logger.Root.BinaryFileTraceLevel = TraceLevel.Info;
                Logger.DefaultBinaryFile.Open();

                // load parameters for boad from file
                if (LoadSettings() == false)
                {
                    // default settings for first COM
                    com1Setting = new COMSetting();
                    com1Setting.PortName = "COM6";
                    com1Setting.Baud = 9600;
                    com1Setting.DataBits = 8;
                    com1Setting.StopBits = StopBits.One;
                    com1Setting.Parity = Parity.None;
                    com1Setting.Handshake = Handshake.None;
                    com1Setting.Use = true;

                    // default settings for second COM
                    com2Setting = new COMSetting();
                    com2Setting.PortName = "COM5";
                    com2Setting.Baud = 9600;
                    com2Setting.DataBits = 8;
                    com2Setting.StopBits = StopBits.One;
                    com2Setting.Parity = Parity.None;
                    com2Setting.Handshake = Handshake.None;
                    com2Setting.Use = false;

                    // default parameters for board
                    boardConfig = new BoardConfig();
                    boardConfig.MeasurementInterval = 60;
                    boardConfig.TransmittingInterval = 300;
                    boardConfig.DeviceAddress = "008C3552";
                    boardConfig.DeviceEUI = "DEADBEEF05042016";
                    boardConfig.NetworkKey = "40907FF216E94920324E593FBA60DE4F";
                    boardConfig.ApplicationKey = "C7DCB7DF8EFD6DC69B6ABD47ABE75943";
                    boardConfig.SensorCommandList = new List<SensorCommand>();
                    SensorCommand sensorCmd = new SensorCommand();
                    sensorCmd.MeasurementCommand = "0M!";
                    sensorCmd.DataCommand = "0D0!";
                    boardConfig.SensorCommandList.Add(sensorCmd);
                }

                // listView have only one column
                listView1.Columns.Add("Data log", 800);

                // default, we will send to sensors
                radioSensor.Checked = true;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "MonitorCOM");
            }
        }

        private void frmMain_FormClosing(object sender, FormClosingEventArgs e)
        {
            CloseCOMPort(ref port1);
            CloseCOMPort(ref port2);
        }

        private void btnOpen_Click(object sender, EventArgs e)
        {
            bool result;

            try
            {
                if (btnOpen.Text == "Open COM")
                {
                    if (com1Setting.Use)
                    {
                        // initialize first port                
                        result = InitCOMPort(ref port1, com1Setting.PortName, com1Setting.Baud, com1Setting.DataBits,
                            com1Setting.StopBits, com1Setting.Parity, com1Setting.Handshake, true);
                        if (result == false)
                            return;

                        Task comPortPollingTask = new Task(PollCOMPort, null);
                        comPortPollingTask.Start();
                    }

                    if (com2Setting.Use)
                    {
                        // initialize second port
                        result = InitCOMPort(ref port2, com2Setting.PortName, com2Setting.Baud, com2Setting.DataBits,
                        com2Setting.StopBits, com2Setting.Parity, com2Setting.Handshake, true);
                        if (result == false)
                        {
                            CloseCOMPort(ref port1);
                            return;
                        }
                    }

                    // change text of button
                    btnOpen.Text = "Close COM";
                }
                else
                {
                    if (CloseCOMPort(ref port1) && CloseCOMPort(ref port2))
                    {
                        // change text of button
                        btnOpen.Text = "Open COM";
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "OpenCOM");
            }
        }

        private void chkShowData_CheckedChanged(object sender, EventArgs e)
        {
            sendDataToGUI = chkShowData.Checked;
        }

        private void btnSendCmd_Click(object sender, EventArgs e)
        {
            try
            {
                // process command string from GUI
                //ProcessCommand(ref port1, cmbCommand.Text);
                //ProcessModbusMessage(ref port1, cmbCommand.Text);
                ProcessSerialMessage(ref port1, cmbCommand.Text);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "SendCmd");
            }
        }

        private void cmbCommand_KeyPress(object sender, KeyPressEventArgs e)
        {
            try
            {
                if (e.KeyChar == (char)Keys.Enter)
                {
                    // process command string from GUI
                    ProcessCommand(ref port1, cmbCommand.Text);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "ProcessKeyPress");
            }
        }

        private void btnEditLoRaBoard_Click(object sender, EventArgs e)
        {
            try
            {
                frmBoardConfig dlg = new frmBoardConfig();
                dlg.BoardConfig = boardConfig;
                dlg.FrmMain = this;
                if (dlg.ShowDialog() == DialogResult.OK)
                    SaveSettings();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "btnEditLoRaBoard_Click");
            }
        }

        private void btnSendAll_Click(object sender, EventArgs e)
        {
            try
            {
                // create task to send new all parameters to board
                Task wsTxTask = new Task(SendParametersToBoard, boardConfig);
                wsTxTask.Start();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "SendAll");
            }
        }

        private void btnSendTime_Click(object sender, EventArgs e)
        {
            try
            {
                // create task to send new all parameters to board
                boardConfig.SendSaveCmd = true;
                Task wsTxTask = new Task(SendTimeToBoard, boardConfig);
                wsTxTask.Start();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "SendTime");
            }
        }

        private void btnSendLoRa_Click(object sender, EventArgs e)
        {
            try
            {
                // create task to send new LoRa parameters to board
                boardConfig.SendSaveCmd = true;
                Task wsTxTask = new Task(SendLoRaParamsToBoard, boardConfig);
                wsTxTask.Start();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "SendLoRa");
            }
        }

        private void btnSendSDI12Command_Click(object sender, EventArgs e)
        {
            try
            {
                // create task to send new SDI12 commands to board
                boardConfig.SendSaveCmd = true;
                Task wsTxTask = new Task(SendSDI12CommandsToBoard, boardConfig);
                wsTxTask.Start();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "btnSendSDI12Command_Click");
            }
        }

        private void btnGetTime_Click(object sender, EventArgs e)
        {
            try
            {
                // send 'get time' command
                GetTime(ref port1);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "btnGetTime_Click");
            }
        }

        private void listView1_KeyPress(object sender, KeyPressEventArgs e)
        {
            ListViewItem item;

            try
            {
                if (e.KeyChar == (char)Keys.Enter)
                {
                    // add empty item to tail
                    item = new ListViewItem("<<");
                    listView1.Items.Add(item);
                    listView1.EnsureVisible(listView1.Items.Count - 1);
                }                
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "ProcessKeyPress");
            }
        }

        private void radioLoggingMode_Click(object sender, EventArgs e)
        {
            Packet packet;

            try
            {
                // create new packet for soft reset command
                packet = new Packet("CMD", "SOFT_REST");
                // send command string to COM
                SendCmdToCOM(ref port1, packet.BuildCommandString());
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "ResetBridge");
            }
        }

        #endregion

        #region Menu handlers

        private void menuTBS12_DropDownOpening(object sender, EventArgs e)
        {
            itemTagTime.Checked = tagTime;
        }

        private void itemGetTime_Click(object sender, EventArgs e)
        {
            try
            {
                // send 'get time' command
                GetTime(ref port1);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "itemGetTime_Click");
            }
        }

        private void itemGetBattery_Click(object sender, EventArgs e)
        {
            try
            {
                // send 'get battery' command
                GetBattery(ref port1);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "itemGetBattery_Click");
            }
        }

        private void itemGetFWVersion_Click(object sender, EventArgs e)
        {
            try
            {
                // send 'get firmware version' command
                GetFirmwareVersion(ref port1);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "itemGetFWVersion_Click");
            }
        }

        private void itemGetTimeSettings_Click(object sender, EventArgs e)
        {
            try
            {
                // send 'get time parameters' command
                GetTimeParameters(ref port1);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "itemGetTimeSettings_Click");
            }
        }

        private void itemGetLoRaSettings_Click(object sender, EventArgs e)
        {
            try
            {
                // send 'get LoRa parameters' command
                GetLoRaParameters(ref port1);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "itemGetLoRaSettings_Click");
            }
        }

        private void itemGetSDI12Commands_Click(object sender, EventArgs e)
        {
            try
            {
                // send 'get LoRa parameters' command
                GetSDI12Parameters(ref port1);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "itemGetSDI12Commands_Click");
            }
        }

        private void itemClearLog_Click(object sender, EventArgs e)
        {
            try
            {
                listView1.Items.Clear();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "itemClearLog_Click");
            }
        }

        private void itemTagTime_Click(object sender, EventArgs e)
        {
            try
            {
                tagTime = !tagTime;
                itemTagTime.Checked = tagTime;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "itemTagTime_Click");
            }
        }

        private void itemOpen_Click(object sender, EventArgs e)
        {
            try
            {
                OpenFileDialog openFileDialog = new OpenFileDialog();
                openFileDialog.InitialDirectory = Application.StartupPath;
                openFileDialog.DefaultExt = ".xml";
                openFileDialog.Filter = "XML files (*.xml)|*.xml|All files (*.*)|*.*";
                openFileDialog.FilterIndex = 1;
                if (openFileDialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
                {
                    BoardConfig savedCfg = LoadBoardSettings(openFileDialog.FileName);
                    if (savedCfg != null)
                    {
                        // Copy settings from file to current settings
                        boardConfig.MeasurementInterval = savedCfg.MeasurementInterval;
                        boardConfig.TransmittingInterval = savedCfg.TransmittingInterval;
                        boardConfig.BatteryCycle = savedCfg.BatteryCycle;

                        boardConfig.DeviceAddress = savedCfg.DeviceAddress;
                        boardConfig.DeviceEUI = savedCfg.DeviceEUI;
                        boardConfig.NetworkKey = savedCfg.NetworkKey;
                        boardConfig.ApplicationKey = savedCfg.ApplicationKey;

                        boardConfig.SensorCommandList.Clear();
                        foreach (SensorCommand sCmd in savedCfg.SensorCommandList)
                        {
                            boardConfig.SensorCommandList.Add(sCmd);
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "itemOpen_Click");
            }
        }

        private void itemSave_Click(object sender, EventArgs e)
        {
            try
            {
                SaveSettings();
                MessageBox.Show("Settings have saved successfully", "SDI-12 LoRa Bridge");
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "itemSave_Click");
            }
        }

        private void itemSaveAs_Click(object sender, EventArgs e)
        {
            try
            {
                SaveFileDialog saveFileDlg = new SaveFileDialog();
                saveFileDlg.InitialDirectory = Application.StartupPath;
                saveFileDlg.DefaultExt = ".xml";
                saveFileDlg.AddExtension = true;
                saveFileDlg.Filter = "XML files (*.xml)|*.xml|All files (*.*)|*.*";
                saveFileDlg.FilterIndex = 1;

                if (saveFileDlg.ShowDialog() == System.Windows.Forms.DialogResult.OK)
                {
                    string fileName = saveFileDlg.FileName;
                    if (SaveBoardSettings(boardConfig, fileName) == true)
                    {
                        MessageBox.Show("Settings have saved successfully", "SDI-12 LoRa Bridge");
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "itemSaveAs_Click");
            }
        }

        private void itemExit_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void itemCOMPort_Click(object sender, EventArgs e)
        {
            try
            {
                frmCOMConfig dlg = new frmCOMConfig();
                dlg.COM1Setting = com1Setting;
                dlg.COM2Setting = com2Setting;
                if (dlg.ShowDialog() == DialogResult.OK)
                    SaveSettings();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "itemCOMPort_Click");
            }
        }

        private void itemBoardConfig_Click(object sender, EventArgs e)
        {
            try
            {
                frmBoardConfig dlg = new frmBoardConfig();
                dlg.BoardConfig = boardConfig;
                dlg.FrmMain = this;
                if (dlg.ShowDialog() == DialogResult.OK)
                    SaveSettings();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "itemBoardConfig_Click");
            }
        }

        #endregion

        #region COM Ports

        private bool InitCOMPort(ref SerialPort port, string portName, int baud, int dataBits, 
            StopBits stopBits, Parity parity, Handshake flowCtrl, bool displayError)
        {
            bool result = false;

            try
            {
                // write to log file
                log.InfoFormat("Open {0}", portName);

                // create new serial port object
                port = new SerialPort(portName);

                // setup callback handlers
                port.DataReceived += new System.IO.Ports.SerialDataReceivedEventHandler(DataReceived);
                port.ErrorReceived += new System.IO.Ports.SerialErrorReceivedEventHandler(ErrorReceived);

                // set up settings
                port.BaudRate = baud;
                port.Parity = parity;
                port.DataBits = dataBits;
                port.StopBits = stopBits;
                port.DiscardNull = false;
                port.Handshake = flowCtrl;
                if (port.Handshake != Handshake.RequestToSendXOnXOff)
                {
                    port.DtrEnable = false;
                    port.RtsEnable = false;
                }
                else
                {
                    port.DtrEnable = true;
                    port.RtsEnable = true;
                }
                port.NewLine = Environment.NewLine;
                port.ReadBufferSize = 4096;
                port.ReadTimeout = -1;
                port.WriteBufferSize = 2048;
                port.WriteTimeout = -1;

                // open COM port
                port.Open();

                // clear garbage data
                port.DiscardInBuffer();
                port.DiscardOutBuffer();

                // write to log file
                log.InfoFormat("{0} have open successfully", portName);

                result = true;
            }
            catch (Exception ex)
            {
                if (displayError == true)
                    MessageBox.Show(ex.Message, "InitCOMPort");
                // set port object to NULL when we get any error
                port = null;
            }
            return result;
        }

        private bool CloseCOMPort(ref SerialPort port, bool displayError = true)
        {
            bool result = false;

            try
            {
                if (port != null && port.IsOpen)
                {
                    port.DiscardInBuffer();
                    port.DiscardOutBuffer();
                    port.Close();
                    port.DataReceived -= DataReceived;
                    port.ErrorReceived -= ErrorReceived;
                    port = null;
                }
                result = true;
            }
            catch (Exception ex)
            {
                if(displayError == true)
                    MessageBox.Show(ex.Message, "CloseCOMPort");
            }
            return result;
        }

        private void DataReceived(object sender, System.IO.Ports.SerialDataReceivedEventArgs e)
        {
            SerialPort port = (SerialPort)sender;
            string data = ""; //, trimedData;

            try
            {
                if (port.BytesToRead > 0)
                {
                    byte[] bytes = new byte[port.BytesToRead];
                    port.Read(bytes, 0, port.BytesToRead);
                    data = BitConverter.ToString(bytes);

                    /*
                    // read to end of line                    
                    data = port.ReadLine();

                    if (connectedBoardConfig != null)
                    {
                        if (data.Contains("RES,GET_MINT,"))
                        {
                            connectedBoardConfig.MeasurementInterval = int.Parse(data.Substring(13));
                        }
                        if (data.Contains("RES,GET_TINT,"))
                        {
                            connectedBoardConfig.TransmittingInterval = int.Parse(data.Substring(13));
                        }
                        if (data.Contains("RES,GET_B_CYC,"))
                        {
                            connectedBoardConfig.BatteryCycle = int.Parse(data.Substring(14));
                            // Signal GetSettingsOnBoard function
                            okResponseEvent.Set();
                        }

                        if (data.Contains("RES,GET_DEVADD,"))
                        {
                            connectedBoardConfig.DeviceAddress = data.Substring(15);
                        }
                        if (data.Contains("RES,GET_DEVEUI,"))
                        {
                            connectedBoardConfig.DeviceEUI = data.Substring(15);
                        }
                        if (data.Contains("RES,GET_NKEY,"))
                        {
                            connectedBoardConfig.NetworkKey = data.Substring(13);
                        }
                        if (data.Contains("RES,GET_AKEY,"))
                        {
                            connectedBoardConfig.ApplicationKey = data.Substring(13);
                        }
                        if (data.Contains("RES,GET_ACK,"))
                        {
                            connectedBoardConfig.WaitACK = data.Substring(12) == "1" ? true : false;
                        }
                        if (data.Contains("RES,GET_REPEAT,"))
                        {
                            connectedBoardConfig.RepeatTimes = int.Parse(data.Substring(15));
                            // Signal GetSettingsOnBoard function
                            okResponseEvent.Set();
                        }

                        if (data.Contains("RES,GET_SDI12,"))
                        {
                            string sdi12Commands = data.Substring(14).Trim();
                            string[] pairs = sdi12Commands.Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);
                            if (pairs != null && pairs.Length == 2)
                            {
                                SensorCommand sensorCmd = new SensorCommand();
                                sensorCmd.MeasurementCommand = pairs[0];
                                sensorCmd.DataCommand = pairs[1];
                                connectedBoardConfig.SensorCommandList.Add(sensorCmd);
                            }
                        }
                    }
                    */

                    // send to data GUI
                    if (sendDataToGUI == true)
                    {
                        PostToListView(port, data);
                    }

                    // sometime, we received '?' character at the beginning, so remove it if it is there
                    //trimedData = data.Trim(new char[] { '?' });
                    // analyse data from COM
                    //ParsePacket(port, trimedData);
                }
            }
            catch(Exception ex)
            {
                if (!ex.Message.StartsWith("The I/O Operation"))
                    MessageBox.Show(ex.Message, "DataReceived");
            }
        }

        private void ErrorReceived(object sender, System.IO.Ports.SerialErrorReceivedEventArgs e)
        {
            SerialPort port = (SerialPort)sender;

            try
            {
                // display on GUI
                PostToListView(port, e.ToString());
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "ErrorReceived");
            }
        }

        #endregion

        #region Task functions

        /// <summary>
        /// Send all parameters (Interval, LoRa, SDI12 commands) to board
        /// </summary>
        private void SendParametersToBoard(object state)
        {
            bool waitResult;
            BoardConfig boardCfg = (BoardConfig)state;

            try
            {
                if (port1 == null || port1.IsOpen == false)
                {
                    MessageBox.Show("The port have not open yet!", "SendParametersToBoard", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }

                boardCfg.SendSaveCmd = false;
                // send time settings
                SendTimeToBoard(boardCfg);
                // send LoRa settings
                SendLoRaParamsToBoard(boardCfg);
                // send SDI12 settings
                SendSDI12CommandsToBoard(boardCfg);

                // store parameters to EEPROM
                SendDataSaving(ref port1);
                okResponseEvent.Reset();
                waitResult = okResponseEvent.WaitOne(5000);
                if (waitResult == false)
                    MessageBox.Show("Don't receive response from the board", "SendParametersToBoard", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "SendParametersToBoard");
            }
        }

        /// <summary>
        /// Send new current time, interval parameters to board
        /// </summary>
        private void SendTimeToBoard(object state)
        {
            bool waitResult;
            BoardConfig boardCfg = (BoardConfig)state;

            try
            {
                if(port1 == null || port1.IsOpen == false)
                {
                    MessageBox.Show("The port have not open yet!", "SendTimeToBoard", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }

                // set new measurement interval
                SendMeasureInterval(ref port1, boardCfg.MeasurementInterval);
                okResponseEvent.Reset();
                waitResult = okResponseEvent.WaitOne(2000);
                if (waitResult == false)
                    goto Error;

                // set new transmitting interval
                SendTransmittingInterval(ref port1, boardCfg.TransmittingInterval);
                okResponseEvent.Reset();
                waitResult = okResponseEvent.WaitOne(2000);
                if (waitResult == false)
                    goto Error;

                // set battery cycle
                SendBatteryCycle(ref port1, boardCfg.BatteryCycle);
                okResponseEvent.Reset();
                waitResult = okResponseEvent.WaitOne(2000);
                if (waitResult == false)
                    goto Error;

                // set current time
                SendTime(ref port1);
                okResponseEvent.Reset();
                waitResult = okResponseEvent.WaitOne(2000);
                if (waitResult == false)
                    goto Error;

                if (boardConfig.SendSaveCmd == true)
                {
                    // store parameters to EEPROM
                    SendDataSaving(ref port1);
                    okResponseEvent.Reset();
                    waitResult = okResponseEvent.WaitOne(5000);
                    if (waitResult == false)
                        goto Error;
                }

                return;

                Error:
                MessageBox.Show("Don't receive response from the board", "SendTimeToBoard", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "SendTimeToBoard");
            }
        }

        /// <summary>
        /// Send new LoRa parameters to board
        /// </summary>
        private void SendLoRaParamsToBoard(object state)
        {
            bool waitResult;
            BoardConfig boardCfg = (BoardConfig)state;

            try
            {
                if (port1 == null || port1.IsOpen == false)
                {
                    MessageBox.Show("The port have not open yet!", "SendLoRaParamsToBoard", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }

                // set new device address
                SendDeviceAddress(ref port1, boardCfg.DeviceAddress);
                okResponseEvent.Reset();
                waitResult = okResponseEvent.WaitOne(2000);
                if (waitResult == false)
                    goto Error;

                // set new device EUI
                SendDeviceEUI(ref port1, boardCfg.DeviceEUI);
                okResponseEvent.Reset();
                waitResult = okResponseEvent.WaitOne(2000);
                if (waitResult == false)
                    goto Error;

                // set new network key
                SendNetworkKey(ref port1, boardConfig.NetworkKey);
                okResponseEvent.Reset();
                waitResult = okResponseEvent.WaitOne(2000);
                if (waitResult == false)
                    goto Error;

                // set new application key
                SendApplicationKey(ref port1, boardConfig.ApplicationKey);
                okResponseEvent.Reset();
                waitResult = okResponseEvent.WaitOne(2000);
                if (waitResult == false)
                    goto Error;

                // set new WaitACK flag
                SendWaitACK(ref port1, boardCfg.WaitACK);
                okResponseEvent.Reset();
                waitResult = okResponseEvent.WaitOne(2000);
                if (waitResult == false)
                    goto Error;

                // set new repeat times
                SendRepeatTimes(ref port1, boardCfg.RepeatTimes);
                okResponseEvent.Reset();
                waitResult = okResponseEvent.WaitOne(2000);
                if (waitResult == false)
                    goto Error;

                if (boardConfig.SendSaveCmd == true)
                {
                    // store parameters to EEPROM
                    SendDataSaving(ref port1);
                    okResponseEvent.Reset();
                    waitResult = okResponseEvent.WaitOne(5000);
                    if (waitResult == false)
                        goto Error;
                }

                return;
                Error:
                MessageBox.Show("Don't receive response from the board", "SendLoRaParamsToBoard", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "SendLoRaParamsToBoard");
            }
        }

        /// <summary>
        /// Send new SDI12 commands to board
        /// </summary>
        private void SendSDI12CommandsToBoard(object state)
        {
            bool waitResult;
            BoardConfig boardCfg = (BoardConfig)state;
            int index = 0;

            try
            {
                if (port1 == null || port1.IsOpen == false)
                {
                    MessageBox.Show("The port have not open yet!", "SendSDI12CommandsToBoard", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }

                foreach (SensorCommand sensorCmd in boardCfg.SensorCommandList)
                {
                    SendSDI12Command(ref port1, index++, sensorCmd.MeasurementCommand + " " + sensorCmd.DataCommand);
                    okResponseEvent.Reset();
                    waitResult = okResponseEvent.WaitOne(2000);
                    if (waitResult == false)
                        goto Error;
                }

                if (boardConfig.SendSaveCmd == true)
                {
                    // store parameters to EEPROM
                    SendDataSaving(ref port1);
                    okResponseEvent.Reset();
                    waitResult = okResponseEvent.WaitOne(5000);
                    if (waitResult == false)
                        goto Error;
                }

                return;
                Error:
                MessageBox.Show("Don't receive response from the board", "SendSDI12CommandsToBoard", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "SendSDI12CommandsToBoard");
            }
        }

        /// <summary>
        /// Poll the COM port and re-open when it is plugged again
        /// </summary>
        private void PollCOMPort(object state)
        {
            string[] portNames;
            string polledPortName;
            bool reOpen;

            try
            {
                polledPortName = port1.PortName;
                if (port1.IsOpen == true && SerialPort.GetPortNames().Contains(polledPortName))
                    reOpen = false;
                else
                    reOpen = false;

                // write to log file
                log.InfoFormat("Begin to monitor state of {0}", polledPortName);

                while (true)
                {
                    // get list that holds all COM ports
                    portNames = SerialPort.GetPortNames();
                    if (portNames.Contains(polledPortName))
                    {
                        if (reOpen == true)
                        {
                            // write to log file
                            log.WarnFormat("{0} have plugged again", polledPortName);
                            // Chờ cổng COM ổn định
                            Thread.Sleep(100);
                            // Reset flag
                            reOpen = false;
                            // Post request to main thread to re-open COM port
                            this.BeginInvoke(new Action(() =>
                            {
                                // initialize first port                
                                InitCOMPort(ref port1, com1Setting.PortName, com1Setting.Baud, com1Setting.DataBits,
                                    com1Setting.StopBits, com1Setting.Parity, com1Setting.Handshake, false);
                            }));
                            // write to log file
                            log.Info("Post request to main thread to re-open COM port");
                        }
                    }
                    else
                    {
                        if (reOpen == false)
                        {
                            reOpen = true;
                            // write to log file
                            log.WarnFormat("{0} have unplugged", polledPortName);
                            // Close the COM port
                            CloseCOMPort(ref port1, false);
                        }
                        reOpen = true;
                    }

                    Thread.Sleep(100);
                }
            }
            catch(Exception ex)
            {
                // write to log file
                log.ErrorFormat("PollCOMPort COM port thrown exception: {0}", ex.Message);
            }
        }

        #endregion

        #region Helpers

        /// <summary>
        /// Add new text to List view
        /// </summary>
        private void PostToListView(SerialPort port, string comData, bool fromCOM = true)
        {
            ListViewItem item;
            string[] arr = new string[1];
            string cursor;

            try
            {
                lock (this)
                {
                    this.listView1.BeginInvoke(new Action(() =>
                    {
                        if (fromCOM)
                            cursor = "<<";
                        else
                            cursor = ">>";

                        if (tagTime == true)
                            arr[0] = cursor + DateTime.Now.ToString("HH:mm:ss.fff ") + comData;
                        else
                            arr[0] = cursor + comData;

                        // write to log file
                        log.Info(arr[0]);

                        // Trim response command string and replace by another string for easily to view
                        arr[0] = arr[0].Replace("RES,GET_TIME,", "TIME: ");
                        arr[0] = arr[0].Replace("CMD,SET_TIME,", "Set current time: ");

                        arr[0] = arr[0].Replace("RES,GET_BATT,", "Battery Voltage: ");
                        arr[0] = arr[0].Replace("RES,GET_FW_VER,", "Firmware version: ");

                        arr[0] = arr[0].Replace("RES,GET_MINT,", "Measurement interval (m): ");
                        arr[0] = arr[0].Replace("CMD,SET_MINT,", "Set measurement interval (m): ");

                        arr[0] = arr[0].Replace("RES,GET_TINT,", "Transmission interval (m): ");
                        arr[0] = arr[0].Replace("CMD,SET_TINT,", "Set transmission interval (m): ");

                        arr[0] = arr[0].Replace("RES,GET_B_CYC,", "Battery cycles: ");
                        arr[0] = arr[0].Replace("CMD,SET_B_CYC,", "Set battery cycles: ");

                        arr[0] = arr[0].Replace("RES,GET_DEVADD,", "Device address: ");
                        arr[0] = arr[0].Replace("CMD,SET_DEVADD,", "Set device address: ");

                        arr[0] = arr[0].Replace("RES,GET_DEVEUI,", "Device EUI: ");
                        arr[0] = arr[0].Replace("CMD,SET_DEVEUI,", "Set device EUI: ");

                        arr[0] = arr[0].Replace("RES,GET_NKEY,", "Network session key: ");
                        arr[0] = arr[0].Replace("CMD,SET_NKEY,", "Set network session key: ");

                        arr[0] = arr[0].Replace("RES,GET_AKEY,", "Application session key: ");
                        arr[0] = arr[0].Replace("CMD,SET_AKEY,", "Set application session key: ");

                        if (arr[0].Contains("RES,GET_ACK,"))
                        {
                            arr[0] = arr[0].Replace("RES,GET_ACK,", "Wait ACK from Server: ");
                            arr[0] = arr[0].Replace("Server: 1", "Server: true");
                            arr[0] = arr[0].Replace("Server: 0", "Server: false");
                        }
                        if (arr[0].Contains("CMD,SET_ACK,"))
                        {
                            arr[0] = arr[0].Replace("CMD,SET_ACK,", "Set 'Wait ACK from Server': ");
                            arr[0] = arr[0].Replace("Server': 1", "Server': true");
                            arr[0] = arr[0].Replace("Server': 0", "Server': false");
                        }

                        arr[0] = arr[0].Replace("RES,GET_REPEAT,", "Repeat times: ");
                        arr[0] = arr[0].Replace("CMD,SET_REPEAT,", "Set repeat times: ");

                        arr[0] = arr[0].Replace("RES,GET_SDI12,", "SDI-12 commands: ");
                        arr[0] = arr[0].Replace("CMD,SET_SDI12,", "Set SDI-12 commands: ");

                        arr[0] = arr[0].Replace("CMD,SAVE_PARAM", "Save parameters to Flash");

                        item = new ListViewItem(arr);
                        if (port == port1)
                        {
                            if (fromCOM)
                                item.ForeColor = Color.Tomato;
                            else
                                item.ForeColor = Color.LimeGreen;
                        }
                        else
                        {
                            item.ForeColor = Color.Blue;
                        }
                        // add item to tail
                        listView1.Items.Add(item);
                        listView1.EnsureVisible(listView1.Items.Count - 1);
                    }));
                }
            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.Message, "PostToListView");
            }
        }

        /// <summary>
        /// Process command string from user
        /// </summary>
        private void ProcessCommand(ref SerialPort port, string command)
        {
            try
            {
                if (string.IsNullOrEmpty(command))
                    return;

                if (port1 == null || port1.IsOpen == false)
                {
                    MessageBox.Show("The port have not open yet!", "ProcessCommand", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }


                if (!cmbCommand.Items.Contains(command))
                {
                    // save new item to history
                    cmbCommand.Items.Add(command);
                }

                if (radioSensor.Checked)
                {
                    // send command to COM port
                    SendSDI12Commnad(ref port, command);
                }
                if (radioLoRa.Checked)
                {
                    // send command to COM port
                    SendLoRaCommnad(ref port, command);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "ProcessCommand");
            }
        }

        // Process HEX command string and send to RS-485 bus
        private void ProcessModbusMessage(ref SerialPort port, string command)
        {
            try
            {
                command = command.Replace(" ", "");
                if((command.Length % 2) != 0)
                {
                    MessageBox.Show("The command string was invalid");
                    return;
                }

                int pos = 0;
                byte[] modbusCmd = new byte[command.Length / 2];
                for(int i = 0; i < command.Length / 2; i++)
                {
                    modbusCmd[i] = Convert.ToByte(command.Substring(pos, 2), 16);
                    pos += 2;
                }
                SendBytesToCOM(ref port1, modbusCmd, modbusCmd.Length);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "ProcessModbusMessage");
            }
        }

        /// <summary>
        /// Send command string to COM port
        /// </summary>
        private bool SendCmdToCOM(ref SerialPort port, string command)
        {
            bool result = false;

            try
            {
                if (port == null || port.IsOpen == false)
                {
                    MessageBox.Show("Can not send data to COM port", "SDI12-LoRa Bridge", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                    return result;  // COM port was invalid
                }

                if (string.IsNullOrEmpty(command))
                    return result;  // command string was invalid

                // write to selected COM port
                port.WriteLine(command);

                // show sent command on GUI
                PostToListView(port, command, false);
                
                result = true;
            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.Message, "SendCmdToCOM");
            }
            return result;
        }

        private bool SendBytesToCOM(ref SerialPort port, byte[] bytes, int len)
        {
            bool result = false;

            try
            {
                if (port == null || port.IsOpen == false)
                {
                    MessageBox.Show("Can not send data to COM port", "SDI12-LoRa Bridge", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                    return result;  // COM port was invalid
                }

                if (bytes == null || len == 0)
                    return result;  // command byes was invalid

                // write to selected COM port
                port.Write(bytes, 0, len);

                // show sent command on GUI
                PostToListView(port, BitConverter.ToString(bytes, 0, len), false);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "SendBytesToCOM");
            }
            return result;
        }

        /// <summary>
        /// Parse data from COM port to consider whether receiving command from board or not. In case of valid packet
        /// the function will return string that should be displayed on GUI
        /// </summary>
        private void ParsePacket(SerialPort port, string comData)
        {
            Packet packet = null;
            Packet response;

            try
            {
                packet = Packet.Parse(comData);
                if(packet != null)
                {
                    if (packet.PacketType == "CMD" && (packet.PacketID == "PING" || (radioConsoleMode.Checked && packet.PacketID == "CONSOLE")))
                    {
                        // board queries PC by sending "PING" or "CONSOLE" command
                        response = new Packet("RES", "OK");
                        // send response to board
                        SendCmdToCOM(ref port, response.BuildCommandString());
                    }

                    if(packet.PacketType == "RES" && packet.PacketID == "OK")
                    {
                        okResponseEvent.Set();
                    }
                }
            }
            catch(Exception ex)
            {
                // write to log
                log.Error("Function BuildCommandString have thrown exception: ", ex.Message);
            }
        }

        /// <summary>
        /// Create SDI12 command string and send to COM port
        /// </summary>
        private bool SendSDI12Commnad(ref SerialPort port, string command)
        {
            bool result = false;
            Packet packet;

            try
            {
                // create new packet for required command
                packet = new Packet("CMD", "SDI12", Encoding.ASCII.GetBytes(command), (byte)command.Length);
                // send command string to COM
                return SendCmdToCOM(ref port, packet.BuildCommandString());
            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.Message, "SendSDI12Commnad");
            }
            return result;
        }

        /// <summary>
        /// Create LoRa command string and send to COM port
        /// </summary>
        private bool SendLoRaCommnad(ref SerialPort port, string command)
        {
            bool result = false;
            Packet packet;

            try
            {
                // create new packet for required command
                packet = new Packet("CMD", "LORA", Encoding.ASCII.GetBytes(command), (byte)command.Length);
                // send command string to COM
                return SendCmdToCOM(ref port, packet.BuildCommandString());
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "SendLoRaCommnad");
            }
            return result;
        }

        /// <summary>
        /// Send raw data to COM port
        /// </summary>
        private bool SendRawCommand(ref SerialPort port, string command)
        {
            bool result = false;

            try
            {
                // send command string to COM
                return SendCmdToCOM(ref port, command);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "SendRawCommand");
            }
            return result;
        }

        /// <summary>
        /// Create 'measure interval' command string and send to COM port
        /// </summary>
        private bool SendMeasureInterval(ref SerialPort port, int interval)
        {
            bool result = false;
            Packet packet;
            string data;

            try
            {
                data = interval.ToString();
                // create new packet for required command
                packet = new Packet("CMD", "SET_MINT", Encoding.ASCII.GetBytes(data), (byte)data.Length);
                // send command string to COM
                return SendCmdToCOM(ref port, packet.BuildCommandString());
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "SendMeasureInterval");
            }
            return result;
        }

        /// <summary>
        /// Create 'transmitting interval' command string and send to COM port
        /// </summary>
        private bool SendTransmittingInterval(ref SerialPort port, int interval)
        {
            bool result = false;
            Packet packet;
            string data;

            try
            {
                data = interval.ToString();
                // create new packet for required command
                packet = new Packet("CMD", "SET_TINT", Encoding.ASCII.GetBytes(data), (byte)data.Length);
                // send command string to COM
                return SendCmdToCOM(ref port, packet.BuildCommandString());
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "SendTransmittingInterval");
            }
            return result;
        }

        /// <summary>
        /// Create 'battery cycle' command string and send to COM port
        /// </summary>
        private bool SendBatteryCycle(ref SerialPort port, int cycle)
        {
            bool result = false;
            Packet packet;
            string data;

            try
            {
                data = cycle.ToString();
                // create new packet for required command
                packet = new Packet("CMD", "SET_B_CYC", Encoding.ASCII.GetBytes(data), (byte)data.Length);
                // send command string to COM
                return SendCmdToCOM(ref port, packet.BuildCommandString());
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "SendBatteryCycle");
            }
            return result;
        }

        /// <summary>
        /// Create 'current date' string and send to COM port
        /// </summary>
        private bool SendTime(ref SerialPort port)
        {
            bool result = false;
            Packet packet;
            string data;

            try
            {
                data = DateTime.Now.ToString("yyyy MM dd HH mm ss");
                // create new packet for required command
                packet = new Packet("CMD", "SET_TIME", Encoding.ASCII.GetBytes(data), (byte)data.Length);
                // send command string to COM
                return SendCmdToCOM(ref port, packet.BuildCommandString());
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "SendTime");
            }
            return result;
        }

        /// <summary>
        /// Create 'get current date' string and send to COM port
        /// </summary>
        private bool GetTime(ref SerialPort port)
        {
            bool result = false;
            Packet packet;

            try
            {
                // create new packet for required command
                packet = new Packet("CMD", "GET_TIME");
                // send command string to COM
                return SendCmdToCOM(ref port, packet.BuildCommandString());
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "GetTime");
            }
            return result;
        }

        /// <summary>
        /// Create 'get current battery' string and send to COM port
        /// </summary>
        private bool GetBattery(ref SerialPort port)
        {
            bool result = false;
            Packet packet;

            try
            {
                // create new packet for required command
                packet = new Packet("CMD", "GET_BATT");
                // send command string to COM
                return SendCmdToCOM(ref port, packet.BuildCommandString());
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "GetBattery");
            }
            return result;
        }

        /// <summary>
        /// Create 'get firmware version' string and send to COM port
        /// </summary>
        private bool GetFirmwareVersion(ref SerialPort port)
        {
            bool result = false;
            Packet packet;

            try
            {
                // create new packet for required command
                packet = new Packet("CMD", "GET_FW_VER");
                // send command string to COM
                return SendCmdToCOM(ref port, packet.BuildCommandString());
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "GetFirmwareVersion");
            }
            return result;
        }

        /// <summary>
        /// Create 'get all time parameters' string and send to COM port
        /// </summary>
        private bool GetTimeParameters(ref SerialPort port)
        {
            bool result = false;
            Packet packet;

            try
            {
                // create new packet for required command
                packet = new Packet("CMD", "GET_TIME_PARAMS");
                // send command string to COM
                return SendCmdToCOM(ref port, packet.BuildCommandString());
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "GetTimeParameters");
            }
            return result;
        }

        /// <summary>
        /// Create 'get all LoRa parameters' string and send to COM port
        /// </summary>
        private bool GetLoRaParameters(ref SerialPort port)
        {
            bool result = false;
            Packet packet;

            try
            {
                // create new packet for required command
                packet = new Packet("CMD", "GET_LORA_PARAMS");
                // send command string to COM
                return SendCmdToCOM(ref port, packet.BuildCommandString());
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "GetLoRaParameters");
            }
            return result;
        }

        /// <summary>
        /// Create 'get all SDI-12 parameters' string and send to COM port
        /// </summary>
        private bool GetSDI12Parameters(ref SerialPort port)
        {
            bool result = false;
            Packet packet;

            try
            {
                // create new packet for required command
                packet = new Packet("CMD", "GET_SDI12");
                // send command string to COM
                return SendCmdToCOM(ref port, packet.BuildCommandString());
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "GetSDI12Parameters");
            }
            return result;
        }

        /// <summary>
        /// Create 'device address' command string and send to COM port
        /// </summary>
        private bool SendDeviceAddress(ref SerialPort port, string deviceAddress)
        {
            bool result = false;
            Packet packet;

            try
            {
                if (string.IsNullOrEmpty(deviceAddress))
                    deviceAddress = "";

                // create new packet for required command
                packet = new Packet("CMD", "SET_DEVADD", Encoding.ASCII.GetBytes(deviceAddress), (byte)deviceAddress.Length);
                // send command string to COM
                return SendCmdToCOM(ref port, packet.BuildCommandString());
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "SendDeviceAddress");
            }
            return result;
        }

        /// <summary>
        /// Create 'device EUI' command string and send to COM port
        /// </summary>
        private bool SendDeviceEUI(ref SerialPort port, string deviceEUI)
        {
            bool result = false;
            Packet packet;

            try
            {
                if (string.IsNullOrEmpty(deviceEUI))
                    deviceEUI = "";

                // create new packet for required command
                packet = new Packet("CMD", "SET_DEVEUI", Encoding.ASCII.GetBytes(deviceEUI), (byte)deviceEUI.Length);
                // send command string to COM
                return SendCmdToCOM(ref port, packet.BuildCommandString());
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "SendDeviceEUI");
            }
            return result;
        }

        /// <summary>
        /// Create 'network key' command string and send to COM port
        /// </summary>
        private bool SendNetworkKey(ref SerialPort port, string networkKey)
        {
            bool result = false;
            Packet packet;

            try
            {
                if (string.IsNullOrEmpty(networkKey))
                    networkKey = "";

                // create new packet for required command
                packet = new Packet("CMD", "SET_NKEY", Encoding.ASCII.GetBytes(networkKey), (byte)networkKey.Length);
                // send command string to COM
                return SendCmdToCOM(ref port, packet.BuildCommandString());
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "SendNetworkKey");
            }
            return result;
        }

        /// <summary>
        /// Create 'application key' command string and send to COM port
        /// </summary>
        private bool SendApplicationKey(ref SerialPort port, string applicationKey)
        {
            bool result = false;
            Packet packet;

            try
            {
                if (string.IsNullOrEmpty(applicationKey))
                    applicationKey = "";

                // create new packet for required command
                packet = new Packet("CMD", "SET_AKEY", Encoding.ASCII.GetBytes(applicationKey), (byte)applicationKey.Length);
                // send command string to COM
                return SendCmdToCOM(ref port, packet.BuildCommandString());
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "SendApplicationKey");
            }
            return result;
        }

        /// <summary>
        /// Create 'wait ACK' command string and send to COM port
        /// </summary>
        private bool SendWaitACK(ref SerialPort port, bool waitACK)
        {
            bool result = false;
            Packet packet;
            string data;

            try
            {
                if (waitACK)
                    data = "1";
                else
                    data = "0";

                // create new packet for required command
                packet = new Packet("CMD", "SET_ACK", Encoding.ASCII.GetBytes(data), (byte)data.Length);
                // send command string to COM
                return SendCmdToCOM(ref port, packet.BuildCommandString());
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "SendWaitACK");
            }
            return result;
        }

        /// <summary>
        /// Create 'repeat times' command string and send to COM port
        /// </summary>
        private bool SendRepeatTimes(ref SerialPort port, int repeatTimes)
        {
            bool result = false;
            Packet packet;
            string data;

            try
            {
                data = repeatTimes.ToString();
                // create new packet for required command
                packet = new Packet("CMD", "SET_REPEAT", Encoding.ASCII.GetBytes(data), (byte)data.Length);
                // send command string to COM
                return SendCmdToCOM(ref port, packet.BuildCommandString());
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "SendRepeatTimes");
            }
            return result;

        }

        /// <summary>
        /// Create 'application key' command string and send to COM port
        /// </summary>
        private bool SendSDI12Command(ref SerialPort port, int index, string sdi12Command)
        {
            bool result = false;
            Packet packet;
            string data;

            try
            {
                if (string.IsNullOrEmpty(sdi12Command))
                    sdi12Command = "";

                data = index.ToString() + " " + sdi12Command;
                // create new packet for required command
                packet = new Packet("CMD", "SET_SDI12", Encoding.ASCII.GetBytes(data), (byte)data.Length);
                // send command string to COM
                return SendCmdToCOM(ref port, packet.BuildCommandString());
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "SendSDI12Command");
            }
            return result;
        }

        /// <summary>
        /// Create 'save data' command string and send to COM port
        /// </summary>
        private bool SendDataSaving(ref SerialPort port)
        {
            bool result = false;
            Packet packet;

            try
            {
                // create new packet for required command
                packet = new Packet("CMD", "SAVE_PARAM");
                // send command string to COM
                return SendCmdToCOM(ref port, packet.BuildCommandString());
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "SendDataSaving");
            }
            return result;
        }

        /// <summary>
        /// Load all settings of board from file
        /// </summary>
        private bool LoadSettings()
        {
            bool result = false;
            int index;
            COMSetting comObj;
            string value, cmd1, cmd2;
            SensorCommand sensorCmd;

            try
            {
                XmlDocument xmlDoc = new XmlDocument();
                //xmlDoc.Load(@"Settings.xml");   // Application.StartupPath + "\\Settings.xml";
                xmlDoc.Load(Application.StartupPath + "\\Settings.xml");

                // create empty COM1 settings
                com1Setting = new COMSetting();
                // create empty COM2 settings
                com2Setting = new COMSetting();

                // get COM settings
                XmlNodeList comNode = xmlDoc.SelectNodes("LoRaBridge/COMPorts/COMPort");
                index = 0;
                foreach (XmlNode node in comNode)
                {
                    if (index == 0)
                        comObj = com1Setting;
                    else
                        comObj = com2Setting;

                    comObj.PortName = node.SelectSingleNode("PortName").InnerText;
                    comObj.Baud = int.Parse(node.SelectSingleNode("Baud").InnerText);
                    comObj.DataBits = int.Parse(node.SelectSingleNode("DataBits").InnerText);
                    comObj.StopBits = (StopBits)Enum.Parse(typeof(StopBits), node.SelectSingleNode("StopBits").InnerText);
                    comObj.Parity = (Parity)Enum.Parse(typeof(Parity), node.SelectSingleNode("Parity").InnerText);
                    comObj.Handshake = (Handshake)Enum.Parse(typeof(Handshake), node.SelectSingleNode("Handshake").InnerText);
                    comObj.Use = bool.Parse(node.SelectSingleNode("Use").InnerText);

                    index++;
                }

                // create new empty structure
                boardConfig = new BoardConfig();

                // get time settings
                XmlNodeList timeNode = xmlDoc.SelectNodes("LoRaBridge/Time");
                foreach (XmlNode node in timeNode)
                {
                    value = node.SelectSingleNode("MeasurementInterval").InnerText;
                    if (string.IsNullOrEmpty(value))
                        value = "60";
                    boardConfig.MeasurementInterval = int.Parse(value);

                    value = node.SelectSingleNode("TransmittingInterval").InnerText;
                    if (string.IsNullOrEmpty(value))
                        value = "300";
                    boardConfig.TransmittingInterval = int.Parse(value);

                    value = node.SelectSingleNode("BatteryCycle").InnerText;
                    if (string.IsNullOrEmpty(value))
                        value = "1";
                    boardConfig.BatteryCycle = int.Parse(value);
                }

                // get LoRa settings
                XmlNodeList loRaNode = xmlDoc.SelectNodes("LoRaBridge/LoRa");
                foreach (XmlNode node in loRaNode)
                {
                    value = node.SelectSingleNode("DeviceAddress").InnerText;
                    if (string.IsNullOrEmpty(value))
                        value = "";
                    boardConfig.DeviceAddress = value;

                    value = node.SelectSingleNode("DeviceEUI").InnerText;
                    if (string.IsNullOrEmpty(value))
                        value = "";
                    boardConfig.DeviceEUI = value;

                    value = node.SelectSingleNode("NetworkKey").InnerText;
                    if (string.IsNullOrEmpty(value))
                        value = "";
                    boardConfig.NetworkKey = value;

                    value = node.SelectSingleNode("ApplicationKey").InnerText;
                    if (string.IsNullOrEmpty(value))
                        value = "";
                    boardConfig.ApplicationKey = value;

                    boardConfig.WaitACK = bool.Parse(node.SelectSingleNode("WaitACK").InnerText);
                    boardConfig.RepeatTimes = int.Parse(node.SelectSingleNode("RepeatTimes").InnerText);
                }

                // get SDI12 commands
                boardConfig.SensorCommandList = new List<SensorCommand>();
                XmlNodeList sdi12Node = xmlDoc.SelectNodes("LoRaBridge/SDI12Commands");
                foreach (XmlNode node in sdi12Node)
                {
                    foreach(XmlNode cNode in node.ChildNodes)
                    {
                        cmd1 = cNode.SelectSingleNode("MeasurementCommand").InnerText;
                        if (string.IsNullOrEmpty(cmd1))
                            continue;
                        cmd2 = cNode.SelectSingleNode("DataCommand").InnerText;
                        if (string.IsNullOrEmpty(cmd2))
                            continue;
                        sensorCmd = new SensorCommand();
                        sensorCmd.MeasurementCommand = cmd1;
                        sensorCmd.DataCommand = cmd2;
                        boardConfig.SensorCommandList.Add(sensorCmd);
                    }
                }
                
                result = true;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "LoadSettings");
            }
            return result;
        }

        /// <summary>
        /// Save all settings of board to file
        /// </summary>
        private void SaveSettings()
        {
            XmlElement element, measureElement, dataElement, subElement;
            COMSetting comSetting;
            XmlText xmlText;

            try
            {
                XmlDocument xmlDoc = new XmlDocument();
                //xmlDoc.Load(@"Settings.xml");   // Application.StartupPath + "\\Settings.xml";
                xmlDoc.Load(Application.StartupPath + "\\Settings.xml");

                // remove all COM settings
                XmlNode comNode = xmlDoc.SelectSingleNode("LoRaBridge/COMPorts");
                comNode.RemoveAll();

                for (int i = 0; i < 2; i++)
                {
                    if (i == 0)
                        comSetting = com1Setting;
                    else
                        comSetting = com2Setting;

                    // add settings of COM1
                    element = xmlDoc.CreateElement("COMPort");

                    subElement = xmlDoc.CreateElement("PortName");
                    xmlText = xmlDoc.CreateTextNode(comSetting.PortName);
                    subElement.AppendChild(xmlText);
                    element.AppendChild(subElement);

                    subElement = xmlDoc.CreateElement("Baud");
                    xmlText = xmlDoc.CreateTextNode(comSetting.Baud.ToString());
                    subElement.AppendChild(xmlText);
                    element.AppendChild(subElement);

                    subElement = xmlDoc.CreateElement("DataBits");
                    xmlText = xmlDoc.CreateTextNode(comSetting.DataBits.ToString());
                    subElement.AppendChild(xmlText);
                    element.AppendChild(subElement);

                    subElement = xmlDoc.CreateElement("StopBits");
                    xmlText = xmlDoc.CreateTextNode(comSetting.StopBits.ToString());
                    subElement.AppendChild(xmlText);
                    element.AppendChild(subElement);

                    subElement = xmlDoc.CreateElement("Parity");
                    xmlText = xmlDoc.CreateTextNode(comSetting.Parity.ToString());
                    subElement.AppendChild(xmlText);
                    element.AppendChild(subElement);

                    subElement = xmlDoc.CreateElement("Handshake");
                    xmlText = xmlDoc.CreateTextNode(comSetting.Handshake.ToString());
                    subElement.AppendChild(xmlText);
                    element.AppendChild(subElement);

                    subElement = xmlDoc.CreateElement("Use");
                    xmlText = xmlDoc.CreateTextNode(comSetting.Use.ToString());
                    subElement.AppendChild(xmlText);
                    element.AppendChild(subElement);

                    comNode.AppendChild(element);
                }

                // update time settings
                XmlNodeList timeNode = xmlDoc.SelectNodes("LoRaBridge/Time");
                foreach (XmlNode node in timeNode)
                {
                    node.SelectSingleNode("MeasurementInterval").InnerText = boardConfig.MeasurementInterval.ToString();
                    node.SelectSingleNode("TransmittingInterval").InnerText = boardConfig.TransmittingInterval.ToString();
                    node.SelectSingleNode("BatteryCycle").InnerText = boardConfig.BatteryCycle.ToString();
                }

                // update LoRa settings
                XmlNodeList loRaNode = xmlDoc.SelectNodes("LoRaBridge/LoRa");
                foreach (XmlNode node in loRaNode)
                {
                    node.SelectSingleNode("DeviceAddress").InnerText = boardConfig.DeviceAddress;
                    node.SelectSingleNode("DeviceEUI").InnerText = boardConfig.DeviceEUI;                   
                    node.SelectSingleNode("NetworkKey").InnerText = boardConfig.NetworkKey;
                    node.SelectSingleNode("ApplicationKey").InnerText = boardConfig.ApplicationKey;
                    node.SelectSingleNode("WaitACK").InnerText = boardConfig.WaitACK.ToString();
                    node.SelectSingleNode("RepeatTimes").InnerText = boardConfig.RepeatTimes.ToString();
                }

                // remove all old SDI12 commands
                XmlNode sdi12Node = xmlDoc.SelectSingleNode("LoRaBridge/SDI12Commands");
                sdi12Node.RemoveAll();
                // add new commands
                foreach(SensorCommand cmd in boardConfig.SensorCommandList)
                {
                    // create new command element
                    element = xmlDoc.CreateElement("Command");

                    // measure command
                    measureElement = xmlDoc.CreateElement("MeasurementCommand");
                    xmlText = xmlDoc.CreateTextNode(cmd.MeasurementCommand);
                    measureElement.AppendChild(xmlText);
                    // data command
                    dataElement = xmlDoc.CreateElement("DataCommand");
                    xmlText = xmlDoc.CreateTextNode(cmd.DataCommand);
                    dataElement.AppendChild(xmlText);

                    // add measure command
                    element.AppendChild(measureElement);
                    // add data command
                    element.AppendChild(dataElement);
                    sdi12Node.AppendChild(element);
                }

                //xmlDoc.Save(@"Settings.xml");
                xmlDoc.Save(Application.StartupPath + "\\Settings.xml");
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "SaveSettings");
            }
        }

        /// <summary>
        /// Load settings of board from specified location
        /// </summary>
        public BoardConfig LoadBoardSettings(string boardPath)
        {
            BoardConfig boardConfig = null;
            string value, cmd1, cmd2;
            SensorCommand sensorCmd;

            try
            {
                if (File.Exists(boardPath) == false)
                {
                    MessageBox.Show("File doesn't exist");
                    return null;
                }

                XmlDocument xmlDoc = new XmlDocument();
                xmlDoc.Load(boardPath);

                // create new empty structure
                boardConfig = new BoardConfig();

                // get time settings
                XmlNodeList timeNode = xmlDoc.SelectNodes("LoRaBridge/Time");
                foreach (XmlNode node in timeNode)
                {
                    value = node.SelectSingleNode("MeasurementInterval").InnerText;
                    if (string.IsNullOrEmpty(value))
                        return null;
                    boardConfig.MeasurementInterval = int.Parse(value);

                    value = node.SelectSingleNode("TransmittingInterval").InnerText;
                    if (string.IsNullOrEmpty(value))
                        return null;
                    boardConfig.TransmittingInterval = int.Parse(value);

                    value = node.SelectSingleNode("BatteryCycle").InnerText;
                    if (string.IsNullOrEmpty(value))
                        return null;
                    boardConfig.BatteryCycle = int.Parse(value);
                }

                // get LoRa settings
                XmlNodeList loRaNode = xmlDoc.SelectNodes("LoRaBridge/LoRa");
                foreach (XmlNode node in loRaNode)
                {
                    value = node.SelectSingleNode("DeviceAddress").InnerText;
                    if (string.IsNullOrEmpty(value))
                        value = "";
                    boardConfig.DeviceAddress = value;

                    value = node.SelectSingleNode("DeviceEUI").InnerText;
                    if (string.IsNullOrEmpty(value))
                        value = "";
                    boardConfig.DeviceEUI = value;

                    value = node.SelectSingleNode("NetworkKey").InnerText;
                    if (string.IsNullOrEmpty(value))
                        value = "";
                    boardConfig.NetworkKey = value;

                    value = node.SelectSingleNode("ApplicationKey").InnerText;
                    if (string.IsNullOrEmpty(value))
                        value = "";
                    boardConfig.ApplicationKey = value;

                    boardConfig.WaitACK = bool.Parse(node.SelectSingleNode("WaitACK").InnerText);
                    boardConfig.RepeatTimes = int.Parse(node.SelectSingleNode("RepeatTimes").InnerText);
                }

                // get SDI12 commands
                boardConfig.SensorCommandList = new List<SensorCommand>();
                XmlNodeList sdi12Node = xmlDoc.SelectNodes("LoRaBridge/SDI12Commands");
                foreach (XmlNode node in sdi12Node)
                {
                    foreach (XmlNode cNode in node.ChildNodes)
                    {
                        cmd1 = cNode.SelectSingleNode("MeasurementCommand").InnerText;
                        if (string.IsNullOrEmpty(cmd1))
                            continue;
                        cmd2 = cNode.SelectSingleNode("DataCommand").InnerText;
                        if (string.IsNullOrEmpty(cmd2))
                            continue;
                        sensorCmd = new SensorCommand();
                        sensorCmd.MeasurementCommand = cmd1;
                        sensorCmd.DataCommand = cmd2;
                        boardConfig.SensorCommandList.Add(sensorCmd);
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "LoadBoardSettings");
            }

            return boardConfig;
        }

        /// <summary>
        /// Save settings of board to specified location
        /// </summary>
        private bool SaveBoardSettings(BoardConfig savedConfig, string boardPath)
        {
            bool result = false;

            try
            {
                if (savedConfig == null || string.IsNullOrEmpty(boardPath))
                    return result;

                XmlWriterSettings writerSettings = new XmlWriterSettings();
                writerSettings.Indent = true;
                using (XmlWriter writer = XmlWriter.Create(boardPath, writerSettings))
                {
                    writer.WriteStartDocument();

                    // Start LoRaBridge element
                    writer.WriteStartElement("LoRaBridge");

                    // Start Time element
                    writer.WriteStartElement("Time");
                    writer.WriteElementString("MeasurementInterval", savedConfig.MeasurementInterval.ToString());
                    writer.WriteElementString("TransmittingInterval", savedConfig.TransmittingInterval.ToString());
                    writer.WriteElementString("BatteryCycle", savedConfig.BatteryCycle.ToString());
                    // Stop Time element
                    writer.WriteEndElement();

                    // Start LoRa element
                    writer.WriteStartElement("LoRa");
                    writer.WriteElementString("DeviceAddress", savedConfig.DeviceAddress);
                    writer.WriteElementString("DeviceEUI", savedConfig.DeviceEUI);
                    writer.WriteElementString("NetworkKey", savedConfig.NetworkKey);
                    writer.WriteElementString("ApplicationKey", savedConfig.ApplicationKey);
                    writer.WriteElementString("WaitACK", savedConfig.WaitACK.ToString());
                    writer.WriteElementString("RepeatTimes", savedConfig.RepeatTimes.ToString());
                    // Stop LoRa element
                    writer.WriteEndElement();

                    // Start SDI12Commands element
                    writer.WriteStartElement("SDI12Commands");
                    foreach (SensorCommand sCmd in savedConfig.SensorCommandList)
                    {
                        // Start Command element
                        writer.WriteStartElement("Command");
                        writer.WriteElementString("MeasurementCommand", sCmd.MeasurementCommand);
                        writer.WriteElementString("DataCommand", sCmd.DataCommand);
                        // Stop Command element
                        writer.WriteEndElement();
                    }
                    // Stop SDI12Commands element
                    writer.WriteEndElement();

                    // Stop LoRaBridge element
                    writer.WriteEndElement();

                    writer.WriteEndDocument();

                    result = true;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "SaveBoardSettings");
            }
            return result;
        }

        /// <summary>
        /// Get current settings on connected board
        /// </summary>
        public BoardConfig GetSettingsOnBoard()
        {
            bool waitResult;

            try
            {
                connectedBoardConfig = new BoardConfig();
                connectedBoardConfig.SensorCommandList = new List<SensorCommand>();

                // Reset event
                okResponseEvent.Reset();
                // send 'get time parameters' command
                GetTimeParameters(ref port1);
                waitResult = okResponseEvent.WaitOne(3000);
                if (waitResult == false)
                    return null;

                // Reset event
                okResponseEvent.Reset();
                // send 'get LoRa parameters' command
                GetLoRaParameters(ref port1);
                waitResult = okResponseEvent.WaitOne(3000);
                if (waitResult == false)
                    return null;

                // send 'get LoRa parameters' command
                GetSDI12Parameters(ref port1);
                // Wait for SDI-12 commands
                Thread.Sleep(2000);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "GetSettingsOnBoard");
            }

            return connectedBoardConfig;
        }

        #endregion

        #region PLC

        private void itemCmdOut1_On_Click(object sender, EventArgs e)
        {
            byte[] data = new byte[8];

            data[0] = 0x01;
            data[1] = 0x06;
            data[2] = 0x63;
            data[3] = 0x00;
            data[4] = 0x00;
            data[5] = 0x01;

            data[6] = 0x56;
            data[7] = 0x4E;
            SendBytesToCOM(ref port1, data, data.Length);
        }

        private void itemCmdOut1_Off_Click(object sender, EventArgs e)
        {
            byte[] data = new byte[8];

            data[0] = 0x01;
            data[1] = 0x06;
            data[2] = 0x63;
            data[3] = 0x00;
            data[4] = 0x00;
            data[5] = 0x00;

            data[6] = 0x97;
            data[7] = 0x8E;
            SendBytesToCOM(ref port1, data, data.Length);
        }

        private void itemCmdGetOut1_Click(object sender, EventArgs e)
        {
            byte[] data = new byte[8];

            data[0] = 0x01;
            data[1] = 0x03;
            data[2] = 0x63;
            data[3] = 0x00;
            data[4] = 0x00;
            data[5] = 0x01;

            data[6] = 0x9A;
            data[7] = 0x4E;
            SendBytesToCOM(ref port1, data, data.Length);
        }

        private void itemCmdGetPort1Cfg_Click(object sender, EventArgs e)
        {
            byte[] data = new byte[8];

            data[0] = 0x01;
            data[1] = 0x03;
            data[2] = 0x8C;
            data[3] = 0x0C;
            data[4] = 0x00;
            data[5] = 0x01;

            data[6] = 0x6E;
            data[7] = 0x99;
            SendBytesToCOM(ref port1, data, data.Length);
        }

        private void itemCmdGetFWVersion_Click(object sender, EventArgs e)
        {
            byte[] data = new byte[8];

            data[0] = 0x01;
            data[1] = 0x03;
            data[2] = 0x89;
            data[3] = 0x08;
            data[4] = 0x00;
            data[5] = 0x08;

            data[6] = 0xEF;
            data[7] = 0x92;
            SendBytesToCOM(ref port1, data, data.Length);
        }

        #endregion

        #region SafeEngineering

        private void ProcessSerialMessage(ref SerialPort port, string command)
        {
            try
            {
                command = command.Replace(" ", "");
                
                int pos = 0;
                byte[] externalCmd = new byte[command.Length / 2];
                for (int i = 0; i < command.Length / 2; i++)
                {
                    externalCmd[i] = Convert.ToByte(command.Substring(pos, 2), 16);
                    pos += 2;
                }
                SendBytesToCOM(ref port1, externalCmd, externalCmd.Length);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "ProcessSerialMessage");
            }
        }

        private void itemCmd1_Click(object sender, EventArgs e)
        {
            byte[] data = new byte[8];

            data[0] = 0x02; // STX
            data[1] = 0x40; // Command
            data[2] = 0x30;
            data[3] = 0x31;
            data[4] = 0x32;
            data[5] = 0x33;
            data[6] = 0x34;
            data[7] = 0x03; // ETX
            SendBytesToCOM(ref port1, data, data.Length);
        }

        private void itemCmd2_Click(object sender, EventArgs e)
        {
            byte[] data = new byte[8];

            data[0] = 0x02; // STX
            data[1] = 0x41; // Command
            data[2] = 0x35;
            data[3] = 0x36;
            data[4] = 0x37;
            data[5] = 0x38;
            data[6] = 0x39;
            data[7] = 0x03; // ETX
            SendBytesToCOM(ref port1, data, data.Length);
        }

        private void itemCmd3_Click(object sender, EventArgs e)
        {
            byte[] data = new byte[8];

            data[0] = 0x02; // STX
            data[1] = 0x42; // Command
            data[2] = 0x3A;
            data[3] = 0x3B;
            data[4] = 0x3C;
            data[5] = 0x3D;
            data[6] = 0x3E;
            data[7] = 0x03; // ETX
            SendBytesToCOM(ref port1, data, data.Length);
        }

        private void itemCmd4_Click(object sender, EventArgs e)
        {
            byte[] data = new byte[8];

            data[0] = 0x02; // STX
            data[1] = 0x43; // Command
            data[2] = 0x3F;
            data[3] = 0x30;
            data[4] = 0x31;
            data[5] = 0x32;
            data[6] = 0x33;
            data[7] = 0x03; // ETX
            SendBytesToCOM(ref port1, data, data.Length);
        }

        #endregion
    }
}
