namespace MonitorTwoCOM
{
    partial class frmMain
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.btnOpen = new System.Windows.Forms.Button();
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.listView1 = new System.Windows.Forms.ListView();
            this.panel1 = new System.Windows.Forms.Panel();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.radioLoggingMode = new System.Windows.Forms.RadioButton();
            this.radioConsoleMode = new System.Windows.Forms.RadioButton();
            this.cmbCommand = new System.Windows.Forms.ComboBox();
            this.radioLoRa = new System.Windows.Forms.RadioButton();
            this.radioSensor = new System.Windows.Forms.RadioButton();
            this.btnSendSDI12Command = new System.Windows.Forms.Button();
            this.btnGetTime = new System.Windows.Forms.Button();
            this.btnSendLoRa = new System.Windows.Forms.Button();
            this.btnEditLoRaBoard = new System.Windows.Forms.Button();
            this.btnSendAll = new System.Windows.Forms.Button();
            this.btnSendTime = new System.Windows.Forms.Button();
            this.btnSendCmd = new System.Windows.Forms.Button();
            this.chkShowData = new System.Windows.Forms.CheckBox();
            this.label1 = new System.Windows.Forms.Label();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.menuFile = new System.Windows.Forms.ToolStripMenuItem();
            this.itemOpen = new System.Windows.Forms.ToolStripMenuItem();
            this.itemSave = new System.Windows.Forms.ToolStripMenuItem();
            this.itemSaveAs = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
            this.itemExit = new System.Windows.Forms.ToolStripMenuItem();
            this.menuTBS12 = new System.Windows.Forms.ToolStripMenuItem();
            this.itemGetTime = new System.Windows.Forms.ToolStripMenuItem();
            this.itemGetBattery = new System.Windows.Forms.ToolStripMenuItem();
            this.itemGetFWVersion = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator5 = new System.Windows.Forms.ToolStripSeparator();
            this.itemGetTimeSettings = new System.Windows.Forms.ToolStripMenuItem();
            this.itemGetLoRaSettings = new System.Windows.Forms.ToolStripMenuItem();
            this.itemGetSDI12Commands = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator3 = new System.Windows.Forms.ToolStripSeparator();
            this.itemClearLog = new System.Windows.Forms.ToolStripMenuItem();
            this.itemTagTime = new System.Windows.Forms.ToolStripMenuItem();
            this.menuSettings = new System.Windows.Forms.ToolStripMenuItem();
            this.itemCOMPort = new System.Windows.Forms.ToolStripMenuItem();
            this.itemBoardConfig = new System.Windows.Forms.ToolStripMenuItem();
            this.menuPLC = new System.Windows.Forms.ToolStripMenuItem();
            this.itemCmdOut1_On = new System.Windows.Forms.ToolStripMenuItem();
            this.itemCmdOut1_Off = new System.Windows.Forms.ToolStripMenuItem();
            this.itemCmdGetOut1 = new System.Windows.Forms.ToolStripMenuItem();
            this.itemCmdGetPort1Cfg = new System.Windows.Forms.ToolStripMenuItem();
            this.itemCmdGetFWVersion = new System.Windows.Forms.ToolStripMenuItem();
            this.menuSafeEngineering = new System.Windows.Forms.ToolStripMenuItem();
            this.itemCmd1 = new System.Windows.Forms.ToolStripMenuItem();
            this.itemCmd2 = new System.Windows.Forms.ToolStripMenuItem();
            this.itemCmd3 = new System.Windows.Forms.ToolStripMenuItem();
            this.itemCmd4 = new System.Windows.Forms.ToolStripMenuItem();
            this.menuHelp = new System.Windows.Forms.ToolStripMenuItem();
            this.itemAbout = new System.Windows.Forms.ToolStripMenuItem();
            this.itemManual = new System.Windows.Forms.ToolStripMenuItem();
            this.tableLayoutPanel1.SuspendLayout();
            this.panel1.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.menuStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // btnOpen
            // 
            this.btnOpen.Location = new System.Drawing.Point(496, 3);
            this.btnOpen.Name = "btnOpen";
            this.btnOpen.Size = new System.Drawing.Size(127, 23);
            this.btnOpen.TabIndex = 11;
            this.btnOpen.Text = "Open COM";
            this.btnOpen.UseVisualStyleBackColor = true;
            this.btnOpen.Click += new System.EventHandler(this.btnOpen_Click);
            // 
            // tableLayoutPanel1
            // 
            this.tableLayoutPanel1.ColumnCount = 1;
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel1.Controls.Add(this.listView1, 0, 1);
            this.tableLayoutPanel1.Controls.Add(this.panel1, 0, 0);
            this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 28);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            this.tableLayoutPanel1.RowCount = 2;
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 99F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
            this.tableLayoutPanel1.Size = new System.Drawing.Size(892, 645);
            this.tableLayoutPanel1.TabIndex = 3;
            // 
            // listView1
            // 
            this.listView1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.listView1.Location = new System.Drawing.Point(3, 102);
            this.listView1.Name = "listView1";
            this.listView1.Size = new System.Drawing.Size(886, 540);
            this.listView1.TabIndex = 0;
            this.listView1.UseCompatibleStateImageBehavior = false;
            this.listView1.View = System.Windows.Forms.View.Details;
            this.listView1.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.listView1_KeyPress);
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.groupBox1);
            this.panel1.Controls.Add(this.cmbCommand);
            this.panel1.Controls.Add(this.btnOpen);
            this.panel1.Controls.Add(this.radioLoRa);
            this.panel1.Controls.Add(this.radioSensor);
            this.panel1.Controls.Add(this.btnSendSDI12Command);
            this.panel1.Controls.Add(this.btnGetTime);
            this.panel1.Controls.Add(this.btnSendLoRa);
            this.panel1.Controls.Add(this.btnEditLoRaBoard);
            this.panel1.Controls.Add(this.btnSendAll);
            this.panel1.Controls.Add(this.btnSendTime);
            this.panel1.Controls.Add(this.btnSendCmd);
            this.panel1.Controls.Add(this.chkShowData);
            this.panel1.Controls.Add(this.label1);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panel1.Location = new System.Drawing.Point(3, 3);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(886, 93);
            this.panel1.TabIndex = 2;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.radioLoggingMode);
            this.groupBox1.Controls.Add(this.radioConsoleMode);
            this.groupBox1.Enabled = false;
            this.groupBox1.Location = new System.Drawing.Point(707, 4);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(179, 86);
            this.groupBox1.TabIndex = 15;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Operation Mode";
            // 
            // radioLoggingMode
            // 
            this.radioLoggingMode.AutoSize = true;
            this.radioLoggingMode.Location = new System.Drawing.Point(46, 53);
            this.radioLoggingMode.Name = "radioLoggingMode";
            this.radioLoggingMode.Size = new System.Drawing.Size(120, 22);
            this.radioLoggingMode.TabIndex = 14;
            this.radioLoggingMode.Text = "Logging mode";
            this.radioLoggingMode.UseVisualStyleBackColor = true;
            this.radioLoggingMode.Click += new System.EventHandler(this.radioLoggingMode_Click);
            // 
            // radioConsoleMode
            // 
            this.radioConsoleMode.AutoSize = true;
            this.radioConsoleMode.Checked = true;
            this.radioConsoleMode.Location = new System.Drawing.Point(46, 25);
            this.radioConsoleMode.Name = "radioConsoleMode";
            this.radioConsoleMode.Size = new System.Drawing.Size(121, 22);
            this.radioConsoleMode.TabIndex = 14;
            this.radioConsoleMode.TabStop = true;
            this.radioConsoleMode.Text = "Console mode";
            this.radioConsoleMode.UseVisualStyleBackColor = true;
            // 
            // cmbCommand
            // 
            this.cmbCommand.FormattingEnabled = true;
            this.cmbCommand.Items.AddRange(new object[] {
            "02 40 30 30 30 30 30 03",
            "02 41 31 31 31 31 10 03"});
            this.cmbCommand.Location = new System.Drawing.Point(126, 60);
            this.cmbCommand.Name = "cmbCommand";
            this.cmbCommand.Size = new System.Drawing.Size(361, 26);
            this.cmbCommand.TabIndex = 0;
            this.cmbCommand.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.cmbCommand_KeyPress);
            // 
            // radioLoRa
            // 
            this.radioLoRa.AutoSize = true;
            this.radioLoRa.Enabled = false;
            this.radioLoRa.Location = new System.Drawing.Point(634, 73);
            this.radioLoRa.Name = "radioLoRa";
            this.radioLoRa.Size = new System.Drawing.Size(61, 22);
            this.radioLoRa.TabIndex = 3;
            this.radioLoRa.TabStop = true;
            this.radioLoRa.Text = "LoRa";
            this.radioLoRa.UseVisualStyleBackColor = true;
            // 
            // radioSensor
            // 
            this.radioSensor.AutoSize = true;
            this.radioSensor.Checked = true;
            this.radioSensor.Enabled = false;
            this.radioSensor.Location = new System.Drawing.Point(634, 52);
            this.radioSensor.Name = "radioSensor";
            this.radioSensor.Size = new System.Drawing.Size(80, 22);
            this.radioSensor.TabIndex = 2;
            this.radioSensor.TabStop = true;
            this.radioSensor.Text = "Sensors";
            this.radioSensor.UseVisualStyleBackColor = true;
            // 
            // btnSendSDI12Command
            // 
            this.btnSendSDI12Command.Enabled = false;
            this.btnSendSDI12Command.Location = new System.Drawing.Point(341, 3);
            this.btnSendSDI12Command.Name = "btnSendSDI12Command";
            this.btnSendSDI12Command.Size = new System.Drawing.Size(146, 23);
            this.btnSendSDI12Command.TabIndex = 9;
            this.btnSendSDI12Command.Text = "Send SDI-12 settings";
            this.btnSendSDI12Command.UseVisualStyleBackColor = true;
            this.btnSendSDI12Command.Click += new System.EventHandler(this.btnSendSDI12Command_Click);
            // 
            // btnGetTime
            // 
            this.btnGetTime.Enabled = false;
            this.btnGetTime.ForeColor = System.Drawing.Color.SlateBlue;
            this.btnGetTime.Location = new System.Drawing.Point(341, 31);
            this.btnGetTime.Name = "btnGetTime";
            this.btnGetTime.Size = new System.Drawing.Size(146, 23);
            this.btnGetTime.TabIndex = 10;
            this.btnGetTime.Text = "Get current time";
            this.btnGetTime.UseVisualStyleBackColor = true;
            this.btnGetTime.Click += new System.EventHandler(this.btnGetTime_Click);
            // 
            // btnSendLoRa
            // 
            this.btnSendLoRa.Enabled = false;
            this.btnSendLoRa.Location = new System.Drawing.Point(178, 31);
            this.btnSendLoRa.Name = "btnSendLoRa";
            this.btnSendLoRa.Size = new System.Drawing.Size(146, 23);
            this.btnSendLoRa.TabIndex = 8;
            this.btnSendLoRa.Text = "Send LoRa settings";
            this.btnSendLoRa.UseVisualStyleBackColor = true;
            this.btnSendLoRa.Click += new System.EventHandler(this.btnSendLoRa_Click);
            // 
            // btnEditLoRaBoard
            // 
            this.btnEditLoRaBoard.Enabled = false;
            this.btnEditLoRaBoard.Location = new System.Drawing.Point(3, 3);
            this.btnEditLoRaBoard.Name = "btnEditLoRaBoard";
            this.btnEditLoRaBoard.Size = new System.Drawing.Size(157, 23);
            this.btnEditLoRaBoard.TabIndex = 5;
            this.btnEditLoRaBoard.Text = "Edit LoRa Board settings";
            this.btnEditLoRaBoard.UseVisualStyleBackColor = true;
            this.btnEditLoRaBoard.Click += new System.EventHandler(this.btnEditLoRaBoard_Click);
            // 
            // btnSendAll
            // 
            this.btnSendAll.Enabled = false;
            this.btnSendAll.Location = new System.Drawing.Point(3, 31);
            this.btnSendAll.Name = "btnSendAll";
            this.btnSendAll.Size = new System.Drawing.Size(157, 23);
            this.btnSendAll.TabIndex = 6;
            this.btnSendAll.Text = "Send all settings";
            this.btnSendAll.UseVisualStyleBackColor = true;
            this.btnSendAll.Click += new System.EventHandler(this.btnSendAll_Click);
            // 
            // btnSendTime
            // 
            this.btnSendTime.Enabled = false;
            this.btnSendTime.Location = new System.Drawing.Point(178, 3);
            this.btnSendTime.Name = "btnSendTime";
            this.btnSendTime.Size = new System.Drawing.Size(146, 23);
            this.btnSendTime.TabIndex = 7;
            this.btnSendTime.Text = "Send time settings";
            this.btnSendTime.UseVisualStyleBackColor = true;
            this.btnSendTime.Click += new System.EventHandler(this.btnSendTime_Click);
            // 
            // btnSendCmd
            // 
            this.btnSendCmd.Location = new System.Drawing.Point(496, 59);
            this.btnSendCmd.Name = "btnSendCmd";
            this.btnSendCmd.Size = new System.Drawing.Size(127, 23);
            this.btnSendCmd.TabIndex = 1;
            this.btnSendCmd.Text = "Send command to";
            this.btnSendCmd.UseVisualStyleBackColor = true;
            this.btnSendCmd.Click += new System.EventHandler(this.btnSendCmd_Click);
            // 
            // chkShowData
            // 
            this.chkShowData.AutoSize = true;
            this.chkShowData.Checked = true;
            this.chkShowData.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chkShowData.Location = new System.Drawing.Point(502, 33);
            this.chkShowData.Name = "chkShowData";
            this.chkShowData.Size = new System.Drawing.Size(128, 22);
            this.chkShowData.TabIndex = 9;
            this.chkShowData.Text = "Show Log Data";
            this.chkShowData.UseVisualStyleBackColor = true;
            this.chkShowData.CheckedChanged += new System.EventHandler(this.chkShowData_CheckedChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(3, 63);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(117, 18);
            this.label1.TabIndex = 0;
            this.label1.Text = "Enter command:";
            // 
            // menuStrip1
            // 
            this.menuStrip1.ImageScalingSize = new System.Drawing.Size(20, 20);
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuFile,
            this.menuTBS12,
            this.menuSettings,
            this.menuPLC,
            this.menuSafeEngineering,
            this.menuHelp});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(892, 28);
            this.menuStrip1.TabIndex = 4;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // menuFile
            // 
            this.menuFile.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.itemOpen,
            this.itemSave,
            this.itemSaveAs,
            this.toolStripSeparator2,
            this.itemExit});
            this.menuFile.Name = "menuFile";
            this.menuFile.Size = new System.Drawing.Size(44, 24);
            this.menuFile.Text = "File";
            // 
            // itemOpen
            // 
            this.itemOpen.Name = "itemOpen";
            this.itemOpen.Size = new System.Drawing.Size(148, 26);
            this.itemOpen.Text = "Open";
            this.itemOpen.Click += new System.EventHandler(this.itemOpen_Click);
            // 
            // itemSave
            // 
            this.itemSave.Name = "itemSave";
            this.itemSave.Size = new System.Drawing.Size(148, 26);
            this.itemSave.Text = "Save";
            this.itemSave.Click += new System.EventHandler(this.itemSave_Click);
            // 
            // itemSaveAs
            // 
            this.itemSaveAs.Name = "itemSaveAs";
            this.itemSaveAs.Size = new System.Drawing.Size(148, 26);
            this.itemSaveAs.Text = "Save As ...";
            this.itemSaveAs.Click += new System.EventHandler(this.itemSaveAs_Click);
            // 
            // toolStripSeparator2
            // 
            this.toolStripSeparator2.Name = "toolStripSeparator2";
            this.toolStripSeparator2.Size = new System.Drawing.Size(145, 6);
            // 
            // itemExit
            // 
            this.itemExit.Name = "itemExit";
            this.itemExit.Size = new System.Drawing.Size(148, 26);
            this.itemExit.Text = "Exit";
            this.itemExit.Click += new System.EventHandler(this.itemExit_Click);
            // 
            // menuTBS12
            // 
            this.menuTBS12.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.itemGetTime,
            this.itemGetBattery,
            this.itemGetFWVersion,
            this.toolStripSeparator5,
            this.itemGetTimeSettings,
            this.itemGetLoRaSettings,
            this.itemGetSDI12Commands,
            this.toolStripSeparator3,
            this.itemClearLog,
            this.itemTagTime});
            this.menuTBS12.Name = "menuTBS12";
            this.menuTBS12.Size = new System.Drawing.Size(62, 24);
            this.menuTBS12.Text = "TBS12";
            this.menuTBS12.Visible = false;
            this.menuTBS12.DropDownOpening += new System.EventHandler(this.menuTBS12_DropDownOpening);
            // 
            // itemGetTime
            // 
            this.itemGetTime.Name = "itemGetTime";
            this.itemGetTime.Size = new System.Drawing.Size(233, 26);
            this.itemGetTime.Text = "Get current date";
            this.itemGetTime.Click += new System.EventHandler(this.itemGetTime_Click);
            // 
            // itemGetBattery
            // 
            this.itemGetBattery.Name = "itemGetBattery";
            this.itemGetBattery.Size = new System.Drawing.Size(233, 26);
            this.itemGetBattery.Text = "Get current battery";
            this.itemGetBattery.Click += new System.EventHandler(this.itemGetBattery_Click);
            // 
            // itemGetFWVersion
            // 
            this.itemGetFWVersion.Name = "itemGetFWVersion";
            this.itemGetFWVersion.Size = new System.Drawing.Size(233, 26);
            this.itemGetFWVersion.Text = "Get firmware version";
            this.itemGetFWVersion.Click += new System.EventHandler(this.itemGetFWVersion_Click);
            // 
            // toolStripSeparator5
            // 
            this.toolStripSeparator5.Name = "toolStripSeparator5";
            this.toolStripSeparator5.Size = new System.Drawing.Size(230, 6);
            // 
            // itemGetTimeSettings
            // 
            this.itemGetTimeSettings.Name = "itemGetTimeSettings";
            this.itemGetTimeSettings.Size = new System.Drawing.Size(233, 26);
            this.itemGetTimeSettings.Text = "Get time settings";
            this.itemGetTimeSettings.Click += new System.EventHandler(this.itemGetTimeSettings_Click);
            // 
            // itemGetLoRaSettings
            // 
            this.itemGetLoRaSettings.Name = "itemGetLoRaSettings";
            this.itemGetLoRaSettings.Size = new System.Drawing.Size(233, 26);
            this.itemGetLoRaSettings.Text = "Get LoRa settings";
            this.itemGetLoRaSettings.Click += new System.EventHandler(this.itemGetLoRaSettings_Click);
            // 
            // itemGetSDI12Commands
            // 
            this.itemGetSDI12Commands.Name = "itemGetSDI12Commands";
            this.itemGetSDI12Commands.Size = new System.Drawing.Size(233, 26);
            this.itemGetSDI12Commands.Text = "Get SDI-12 commands";
            this.itemGetSDI12Commands.Click += new System.EventHandler(this.itemGetSDI12Commands_Click);
            // 
            // toolStripSeparator3
            // 
            this.toolStripSeparator3.Name = "toolStripSeparator3";
            this.toolStripSeparator3.Size = new System.Drawing.Size(230, 6);
            // 
            // itemClearLog
            // 
            this.itemClearLog.Name = "itemClearLog";
            this.itemClearLog.Size = new System.Drawing.Size(233, 26);
            this.itemClearLog.Text = "Clear history";
            this.itemClearLog.Click += new System.EventHandler(this.itemClearLog_Click);
            // 
            // itemTagTime
            // 
            this.itemTagTime.Name = "itemTagTime";
            this.itemTagTime.Size = new System.Drawing.Size(233, 26);
            this.itemTagTime.Text = "Tag time";
            this.itemTagTime.Click += new System.EventHandler(this.itemTagTime_Click);
            // 
            // menuSettings
            // 
            this.menuSettings.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.itemCOMPort,
            this.itemBoardConfig});
            this.menuSettings.Name = "menuSettings";
            this.menuSettings.Size = new System.Drawing.Size(74, 24);
            this.menuSettings.Text = "Settings";
            // 
            // itemCOMPort
            // 
            this.itemCOMPort.Name = "itemCOMPort";
            this.itemCOMPort.Size = new System.Drawing.Size(201, 26);
            this.itemCOMPort.Text = "COM port";
            this.itemCOMPort.Click += new System.EventHandler(this.itemCOMPort_Click);
            // 
            // itemBoardConfig
            // 
            this.itemBoardConfig.Name = "itemBoardConfig";
            this.itemBoardConfig.Size = new System.Drawing.Size(201, 26);
            this.itemBoardConfig.Text = "SDI12-LoRa boad";
            this.itemBoardConfig.Visible = false;
            this.itemBoardConfig.Click += new System.EventHandler(this.itemBoardConfig_Click);
            // 
            // menuPLC
            // 
            this.menuPLC.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.itemCmdOut1_On,
            this.itemCmdOut1_Off,
            this.itemCmdGetOut1,
            this.itemCmdGetPort1Cfg,
            this.itemCmdGetFWVersion});
            this.menuPLC.Name = "menuPLC";
            this.menuPLC.Size = new System.Drawing.Size(44, 24);
            this.menuPLC.Text = "PLC";
            this.menuPLC.Visible = false;
            // 
            // itemCmdOut1_On
            // 
            this.itemCmdOut1_On.Name = "itemCmdOut1_On";
            this.itemCmdOut1_On.Size = new System.Drawing.Size(184, 26);
            this.itemCmdOut1_On.Text = "Set Out1 On";
            this.itemCmdOut1_On.Click += new System.EventHandler(this.itemCmdOut1_On_Click);
            // 
            // itemCmdOut1_Off
            // 
            this.itemCmdOut1_Off.Name = "itemCmdOut1_Off";
            this.itemCmdOut1_Off.Size = new System.Drawing.Size(184, 26);
            this.itemCmdOut1_Off.Text = "Set Out1 Off";
            this.itemCmdOut1_Off.Click += new System.EventHandler(this.itemCmdOut1_Off_Click);
            // 
            // itemCmdGetOut1
            // 
            this.itemCmdGetOut1.Name = "itemCmdGetOut1";
            this.itemCmdGetOut1.Size = new System.Drawing.Size(184, 26);
            this.itemCmdGetOut1.Text = "Get Out1";
            this.itemCmdGetOut1.Click += new System.EventHandler(this.itemCmdGetOut1_Click);
            // 
            // itemCmdGetPort1Cfg
            // 
            this.itemCmdGetPort1Cfg.Name = "itemCmdGetPort1Cfg";
            this.itemCmdGetPort1Cfg.Size = new System.Drawing.Size(184, 26);
            this.itemCmdGetPort1Cfg.Text = "Get Port1 Cfg";
            this.itemCmdGetPort1Cfg.Click += new System.EventHandler(this.itemCmdGetPort1Cfg_Click);
            // 
            // itemCmdGetFWVersion
            // 
            this.itemCmdGetFWVersion.Name = "itemCmdGetFWVersion";
            this.itemCmdGetFWVersion.Size = new System.Drawing.Size(184, 26);
            this.itemCmdGetFWVersion.Text = "Get FW Version";
            this.itemCmdGetFWVersion.Click += new System.EventHandler(this.itemCmdGetFWVersion_Click);
            // 
            // menuSafeEngineering
            // 
            this.menuSafeEngineering.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.itemCmd1,
            this.itemCmd2,
            this.itemCmd3,
            this.itemCmd4});
            this.menuSafeEngineering.Name = "menuSafeEngineering";
            this.menuSafeEngineering.Size = new System.Drawing.Size(129, 24);
            this.menuSafeEngineering.Text = "SafeEngineering";
            // 
            // itemCmd1
            // 
            this.itemCmd1.Name = "itemCmd1";
            this.itemCmd1.Size = new System.Drawing.Size(165, 26);
            this.itemCmd1.Text = "Command 1";
            this.itemCmd1.Click += new System.EventHandler(this.itemCmd1_Click);
            // 
            // itemCmd2
            // 
            this.itemCmd2.Name = "itemCmd2";
            this.itemCmd2.Size = new System.Drawing.Size(165, 26);
            this.itemCmd2.Text = "Command 2";
            this.itemCmd2.Click += new System.EventHandler(this.itemCmd2_Click);
            // 
            // itemCmd3
            // 
            this.itemCmd3.Name = "itemCmd3";
            this.itemCmd3.Size = new System.Drawing.Size(165, 26);
            this.itemCmd3.Text = "Command 3";
            this.itemCmd3.Click += new System.EventHandler(this.itemCmd3_Click);
            // 
            // itemCmd4
            // 
            this.itemCmd4.Name = "itemCmd4";
            this.itemCmd4.Size = new System.Drawing.Size(165, 26);
            this.itemCmd4.Text = "Command 4";
            this.itemCmd4.Click += new System.EventHandler(this.itemCmd4_Click);
            // 
            // menuHelp
            // 
            this.menuHelp.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.itemAbout,
            this.itemManual});
            this.menuHelp.Name = "menuHelp";
            this.menuHelp.Size = new System.Drawing.Size(53, 24);
            this.menuHelp.Text = "Help";
            // 
            // itemAbout
            // 
            this.itemAbout.Name = "itemAbout";
            this.itemAbout.Size = new System.Drawing.Size(133, 26);
            this.itemAbout.Text = "About";
            // 
            // itemManual
            // 
            this.itemManual.Name = "itemManual";
            this.itemManual.Size = new System.Drawing.Size(133, 26);
            this.itemManual.Text = "Manual";
            // 
            // frmMain
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 18F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(892, 673);
            this.Controls.Add(this.tableLayoutPanel1);
            this.Controls.Add(this.menuStrip1);
            this.Font = new System.Drawing.Font("Tahoma", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Name = "frmMain";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "SDI12-LoRa Bridge";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.frmMain_FormClosing);
            this.Load += new System.EventHandler(this.frmMain_Load);
            this.tableLayoutPanel1.ResumeLayout(false);
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.Button btnOpen;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
        private System.Windows.Forms.ListView listView1;
        private System.Windows.Forms.CheckBox chkShowData;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button btnSendCmd;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem menuFile;
        private System.Windows.Forms.ToolStripMenuItem menuSettings;
        private System.Windows.Forms.ToolStripMenuItem menuHelp;
        private System.Windows.Forms.ToolStripMenuItem itemExit;
        private System.Windows.Forms.ToolStripMenuItem itemAbout;
        private System.Windows.Forms.ToolStripMenuItem itemManual;
        private System.Windows.Forms.ToolStripMenuItem itemCOMPort;
        private System.Windows.Forms.RadioButton radioLoRa;
        private System.Windows.Forms.RadioButton radioSensor;
        private System.Windows.Forms.ComboBox cmbCommand;
        private System.Windows.Forms.ToolStripMenuItem itemBoardConfig;
        private System.Windows.Forms.Button btnSendSDI12Command;
        private System.Windows.Forms.Button btnSendLoRa;
        private System.Windows.Forms.Button btnSendTime;
        private System.Windows.Forms.Button btnSendAll;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
        private System.Windows.Forms.Button btnGetTime;
        private System.Windows.Forms.ToolStripMenuItem itemOpen;
        private System.Windows.Forms.ToolStripMenuItem itemSave;
        private System.Windows.Forms.ToolStripMenuItem itemSaveAs;
        private System.Windows.Forms.Button btnEditLoRaBoard;
        private System.Windows.Forms.ToolStripMenuItem menuTBS12;
        private System.Windows.Forms.ToolStripMenuItem itemGetTime;
        private System.Windows.Forms.ToolStripMenuItem itemGetBattery;
        private System.Windows.Forms.ToolStripMenuItem itemGetFWVersion;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator5;
        private System.Windows.Forms.ToolStripMenuItem itemGetTimeSettings;
        private System.Windows.Forms.ToolStripMenuItem itemGetLoRaSettings;
        private System.Windows.Forms.ToolStripMenuItem itemGetSDI12Commands;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator3;
        private System.Windows.Forms.ToolStripMenuItem itemClearLog;
        private System.Windows.Forms.ToolStripMenuItem itemTagTime;
        private System.Windows.Forms.RadioButton radioConsoleMode;
        private System.Windows.Forms.RadioButton radioLoggingMode;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.ToolStripMenuItem menuPLC;
        private System.Windows.Forms.ToolStripMenuItem itemCmdOut1_On;
        private System.Windows.Forms.ToolStripMenuItem itemCmdOut1_Off;
        private System.Windows.Forms.ToolStripMenuItem itemCmdGetOut1;
        private System.Windows.Forms.ToolStripMenuItem itemCmdGetPort1Cfg;
        private System.Windows.Forms.ToolStripMenuItem itemCmdGetFWVersion;
        private System.Windows.Forms.ToolStripMenuItem menuSafeEngineering;
        private System.Windows.Forms.ToolStripMenuItem itemCmd1;
        private System.Windows.Forms.ToolStripMenuItem itemCmd2;
        private System.Windows.Forms.ToolStripMenuItem itemCmd3;
        private System.Windows.Forms.ToolStripMenuItem itemCmd4;
    }
}

