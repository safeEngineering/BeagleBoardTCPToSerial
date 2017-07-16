namespace MonitorTwoCOM
{
    partial class frmBoardConfig
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
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.txtTBattery = new System.Windows.Forms.TextBox();
            this.txtTInterval = new System.Windows.Forms.TextBox();
            this.label9 = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.txtMInterval = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.label7 = new System.Windows.Forms.Label();
            this.txtRepeatTimes = new System.Windows.Forms.TextBox();
            this.radio_NoWaitACK = new System.Windows.Forms.RadioButton();
            this.radio_WaitACK = new System.Windows.Forms.RadioButton();
            this.txtDevEUI = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.txtAppSKey = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.txtNetSKey = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.txtDevAdd = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.gridSDICommand = new System.Windows.Forms.DataGridView();
            this.Ordinal = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.MeasurementCmd = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.DataCmd = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.btnOK = new System.Windows.Forms.Button();
            this.btnCancel = new System.Windows.Forms.Button();
            this.btnOpen = new System.Windows.Forms.Button();
            this.btnGetFromBoard = new System.Windows.Forms.Button();
            this.label10 = new System.Windows.Forms.Label();
            this.cbDatarateScheme = new System.Windows.Forms.ComboBox();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.groupBox4.SuspendLayout();
            this.groupBox3.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.gridSDICommand)).BeginInit();
            this.SuspendLayout();
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.txtTBattery);
            this.groupBox1.Controls.Add(this.txtTInterval);
            this.groupBox1.Controls.Add(this.label9);
            this.groupBox1.Controls.Add(this.label8);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.txtMInterval);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Location = new System.Drawing.Point(12, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(351, 105);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Interval";
            // 
            // txtTBattery
            // 
            this.txtTBattery.Location = new System.Drawing.Point(188, 73);
            this.txtTBattery.Name = "txtTBattery";
            this.txtTBattery.Size = new System.Drawing.Size(33, 22);
            this.txtTBattery.TabIndex = 1;
            this.txtTBattery.Text = "5";
            // 
            // txtTInterval
            // 
            this.txtTInterval.Location = new System.Drawing.Point(204, 44);
            this.txtTInterval.Name = "txtTInterval";
            this.txtTInterval.Size = new System.Drawing.Size(136, 22);
            this.txtTInterval.TabIndex = 1;
            this.txtTInterval.Text = "300";
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(226, 76);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(108, 14);
            this.label9.TabIndex = 0;
            this.label9.Text = "transmitting cycles";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(10, 76);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(174, 14);
            this.label8.TabIndex = 0;
            this.label8.Text = "Send battery information each";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(10, 47);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(165, 14);
            this.label2.TabIndex = 0;
            this.label2.Text = "Transmiting interval (minute)";
            // 
            // txtMInterval
            // 
            this.txtMInterval.Location = new System.Drawing.Point(204, 16);
            this.txtMInterval.Name = "txtMInterval";
            this.txtMInterval.Size = new System.Drawing.Size(136, 22);
            this.txtMInterval.TabIndex = 0;
            this.txtMInterval.Text = "60";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(10, 19);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(176, 14);
            this.label1.TabIndex = 0;
            this.label1.Text = "Measurement interval (minute)";
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.cbDatarateScheme);
            this.groupBox2.Controls.Add(this.groupBox4);
            this.groupBox2.Controls.Add(this.txtDevEUI);
            this.groupBox2.Controls.Add(this.label3);
            this.groupBox2.Controls.Add(this.txtAppSKey);
            this.groupBox2.Controls.Add(this.label10);
            this.groupBox2.Controls.Add(this.label6);
            this.groupBox2.Controls.Add(this.txtNetSKey);
            this.groupBox2.Controls.Add(this.label5);
            this.groupBox2.Controls.Add(this.txtDevAdd);
            this.groupBox2.Controls.Add(this.label4);
            this.groupBox2.Location = new System.Drawing.Point(12, 123);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(351, 284);
            this.groupBox2.TabIndex = 1;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "LoRa Module";
            // 
            // groupBox4
            // 
            this.groupBox4.Controls.Add(this.label7);
            this.groupBox4.Controls.Add(this.txtRepeatTimes);
            this.groupBox4.Controls.Add(this.radio_NoWaitACK);
            this.groupBox4.Controls.Add(this.radio_WaitACK);
            this.groupBox4.Location = new System.Drawing.Point(12, 142);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.Size = new System.Drawing.Size(326, 75);
            this.groupBox4.TabIndex = 4;
            this.groupBox4.TabStop = false;
            this.groupBox4.Text = "ACK Options";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(164, 48);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(115, 14);
            this.label7.TabIndex = 2;
            this.label7.Text = "repeat times (1-15)";
            // 
            // txtRepeatTimes
            // 
            this.txtRepeatTimes.Location = new System.Drawing.Point(283, 45);
            this.txtRepeatTimes.Name = "txtRepeatTimes";
            this.txtRepeatTimes.Size = new System.Drawing.Size(33, 22);
            this.txtRepeatTimes.TabIndex = 1;
            // 
            // radio_NoWaitACK
            // 
            this.radio_NoWaitACK.AutoSize = true;
            this.radio_NoWaitACK.Location = new System.Drawing.Point(24, 46);
            this.radio_NoWaitACK.Name = "radio_NoWaitACK";
            this.radio_NoWaitACK.Size = new System.Drawing.Size(132, 18);
            this.radio_NoWaitACK.TabIndex = 0;
            this.radio_NoWaitACK.TabStop = true;
            this.radio_NoWaitACK.Text = "No ACK from server";
            this.radio_NoWaitACK.UseVisualStyleBackColor = true;
            this.radio_NoWaitACK.CheckedChanged += new System.EventHandler(this.radio_NoWaitACK_CheckedChanged);
            // 
            // radio_WaitACK
            // 
            this.radio_WaitACK.AutoSize = true;
            this.radio_WaitACK.Location = new System.Drawing.Point(24, 22);
            this.radio_WaitACK.Name = "radio_WaitACK";
            this.radio_WaitACK.Size = new System.Drawing.Size(142, 18);
            this.radio_WaitACK.TabIndex = 0;
            this.radio_WaitACK.TabStop = true;
            this.radio_WaitACK.Text = "Wait ACK from server";
            this.radio_WaitACK.UseVisualStyleBackColor = true;
            // 
            // txtDevEUI
            // 
            this.txtDevEUI.Location = new System.Drawing.Point(110, 52);
            this.txtDevEUI.Name = "txtDevEUI";
            this.txtDevEUI.Size = new System.Drawing.Size(224, 22);
            this.txtDevEUI.TabIndex = 1;
            this.txtDevEUI.Text = "DEADBEEF05042016";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(10, 55);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(66, 14);
            this.label3.TabIndex = 0;
            this.label3.Text = "Device EUI";
            // 
            // txtAppSKey
            // 
            this.txtAppSKey.Location = new System.Drawing.Point(71, 112);
            this.txtAppSKey.Name = "txtAppSKey";
            this.txtAppSKey.Size = new System.Drawing.Size(263, 22);
            this.txtAppSKey.TabIndex = 3;
            this.txtAppSKey.Text = "C7DCB7DF8EFD6DC69B6ABD47ABE75943";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(10, 115);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(53, 14);
            this.label6.TabIndex = 0;
            this.label6.Text = "App Key";
            // 
            // txtNetSKey
            // 
            this.txtNetSKey.Location = new System.Drawing.Point(71, 82);
            this.txtNetSKey.Name = "txtNetSKey";
            this.txtNetSKey.Size = new System.Drawing.Size(263, 22);
            this.txtNetSKey.TabIndex = 2;
            this.txtNetSKey.Text = "40907FF216E94920324E593FBA60DE4F";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(10, 85);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(51, 14);
            this.label5.TabIndex = 0;
            this.label5.Text = "Net Key";
            // 
            // txtDevAdd
            // 
            this.txtDevAdd.Location = new System.Drawing.Point(110, 22);
            this.txtDevAdd.Name = "txtDevAdd";
            this.txtDevAdd.Size = new System.Drawing.Size(224, 22);
            this.txtDevAdd.TabIndex = 0;
            this.txtDevAdd.Text = "008C3552";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(10, 25);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(90, 14);
            this.label4.TabIndex = 0;
            this.label4.Text = "Device Address";
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.gridSDICommand);
            this.groupBox3.Location = new System.Drawing.Point(369, 12);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(325, 395);
            this.groupBox3.TabIndex = 2;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "SDI12 Commands";
            // 
            // gridSDICommand
            // 
            this.gridSDICommand.AllowUserToAddRows = false;
            this.gridSDICommand.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.gridSDICommand.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.Ordinal,
            this.MeasurementCmd,
            this.DataCmd});
            this.gridSDICommand.Location = new System.Drawing.Point(6, 21);
            this.gridSDICommand.Name = "gridSDICommand";
            this.gridSDICommand.Size = new System.Drawing.Size(314, 368);
            this.gridSDICommand.TabIndex = 0;
            this.gridSDICommand.RowsRemoved += new System.Windows.Forms.DataGridViewRowsRemovedEventHandler(this.gridSDICommand_RowsRemoved);
            // 
            // Ordinal
            // 
            this.Ordinal.HeaderText = "Ordinal";
            this.Ordinal.Name = "Ordinal";
            this.Ordinal.ReadOnly = true;
            this.Ordinal.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            this.Ordinal.Width = 50;
            // 
            // MeasurementCmd
            // 
            this.MeasurementCmd.HeaderText = "Measurement Command";
            this.MeasurementCmd.Name = "MeasurementCmd";
            this.MeasurementCmd.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // DataCmd
            // 
            this.DataCmd.HeaderText = "Data Command";
            this.DataCmd.Name = "DataCmd";
            this.DataCmd.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            this.DataCmd.Width = 80;
            // 
            // btnOK
            // 
            this.btnOK.Location = new System.Drawing.Point(506, 413);
            this.btnOK.Name = "btnOK";
            this.btnOK.Size = new System.Drawing.Size(75, 23);
            this.btnOK.TabIndex = 5;
            this.btnOK.Text = "Save";
            this.btnOK.UseVisualStyleBackColor = true;
            this.btnOK.Click += new System.EventHandler(this.btnOK_Click);
            // 
            // btnCancel
            // 
            this.btnCancel.Location = new System.Drawing.Point(614, 413);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new System.Drawing.Size(75, 23);
            this.btnCancel.TabIndex = 6;
            this.btnCancel.Text = "Cancel";
            this.btnCancel.UseVisualStyleBackColor = true;
            this.btnCancel.Click += new System.EventHandler(this.btnCancel_Click);
            // 
            // btnOpen
            // 
            this.btnOpen.Location = new System.Drawing.Point(16, 413);
            this.btnOpen.Name = "btnOpen";
            this.btnOpen.Size = new System.Drawing.Size(212, 23);
            this.btnOpen.TabIndex = 3;
            this.btnOpen.Text = "Open settings file";
            this.btnOpen.UseVisualStyleBackColor = true;
            this.btnOpen.Click += new System.EventHandler(this.btnOpen_Click);
            // 
            // btnGetFromBoard
            // 
            this.btnGetFromBoard.Location = new System.Drawing.Point(261, 413);
            this.btnGetFromBoard.Name = "btnGetFromBoard";
            this.btnGetFromBoard.Size = new System.Drawing.Size(212, 23);
            this.btnGetFromBoard.TabIndex = 4;
            this.btnGetFromBoard.Text = "Get settings from LoRa Board";
            this.btnGetFromBoard.UseVisualStyleBackColor = true;
            this.btnGetFromBoard.Click += new System.EventHandler(this.btnGetFromBoard_Click);
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(10, 234);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(102, 14);
            this.label10.TabIndex = 0;
            this.label10.Text = "Datarate Scheme";
            // 
            // cbDatarateScheme
            // 
            this.cbDatarateScheme.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbDatarateScheme.FormattingEnabled = true;
            this.cbDatarateScheme.Items.AddRange(new object[] {
            "EU868",
            "US915",
            "CUSTOM"});
            this.cbDatarateScheme.Location = new System.Drawing.Point(118, 231);
            this.cbDatarateScheme.Name = "cbDatarateScheme";
            this.cbDatarateScheme.Size = new System.Drawing.Size(216, 22);
            this.cbDatarateScheme.TabIndex = 5;
            // 
            // frmBoardConfig
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 14F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(704, 439);
            this.Controls.Add(this.btnCancel);
            this.Controls.Add(this.btnGetFromBoard);
            this.Controls.Add(this.btnOpen);
            this.Controls.Add(this.btnOK);
            this.Controls.Add(this.groupBox3);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Font = new System.Drawing.Font("Tahoma", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Name = "frmBoardConfig";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "TBS12-LoRa Settings";
            this.Load += new System.EventHandler(this.frmBoardConfig_Load);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.groupBox4.ResumeLayout(false);
            this.groupBox4.PerformLayout();
            this.groupBox3.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.gridSDICommand)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.TextBox txtTInterval;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox txtMInterval;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.TextBox txtDevEUI;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox txtDevAdd;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TextBox txtAppSKey;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.TextBox txtNetSKey;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.DataGridView gridSDICommand;
        private System.Windows.Forms.DataGridViewTextBoxColumn Ordinal;
        private System.Windows.Forms.DataGridViewTextBoxColumn MeasurementCmd;
        private System.Windows.Forms.DataGridViewTextBoxColumn DataCmd;
        private System.Windows.Forms.Button btnOK;
        private System.Windows.Forms.Button btnCancel;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.TextBox txtRepeatTimes;
        private System.Windows.Forms.RadioButton radio_NoWaitACK;
        private System.Windows.Forms.RadioButton radio_WaitACK;
        private System.Windows.Forms.TextBox txtTBattery;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.Button btnOpen;
        private System.Windows.Forms.Button btnGetFromBoard;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.ComboBox cbDatarateScheme;
    }
}