namespace MonitorTwoCOM
{
    partial class frmCOMConfig
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
            this.PortName1 = new System.Windows.Forms.ComboBox();
            this.chkUseCOM1 = new System.Windows.Forms.CheckBox();
            this.FlowCtrl1 = new System.Windows.Forms.ComboBox();
            this.Parity1 = new System.Windows.Forms.ComboBox();
            this.Baud1 = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.StopBits1 = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.DataBits1 = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.PortName2 = new System.Windows.Forms.ComboBox();
            this.chkUseCOM2 = new System.Windows.Forms.CheckBox();
            this.FlowCtrl2 = new System.Windows.Forms.ComboBox();
            this.Parity2 = new System.Windows.Forms.ComboBox();
            this.Baud2 = new System.Windows.Forms.TextBox();
            this.label7 = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.StopBits2 = new System.Windows.Forms.TextBox();
            this.label9 = new System.Windows.Forms.Label();
            this.label10 = new System.Windows.Forms.Label();
            this.DataBits2 = new System.Windows.Forms.TextBox();
            this.label11 = new System.Windows.Forms.Label();
            this.label12 = new System.Windows.Forms.Label();
            this.btnOK = new System.Windows.Forms.Button();
            this.btnCancel = new System.Windows.Forms.Button();
            this.btnRefresh = new System.Windows.Forms.Button();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.PortName1);
            this.groupBox1.Controls.Add(this.chkUseCOM1);
            this.groupBox1.Controls.Add(this.FlowCtrl1);
            this.groupBox1.Controls.Add(this.Parity1);
            this.groupBox1.Controls.Add(this.Baud1);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.label6);
            this.groupBox1.Controls.Add(this.StopBits1);
            this.groupBox1.Controls.Add(this.label4);
            this.groupBox1.Controls.Add(this.label5);
            this.groupBox1.Controls.Add(this.DataBits1);
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Location = new System.Drawing.Point(12, 10);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(368, 139);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "First COM";
            // 
            // PortName1
            // 
            this.PortName1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.PortName1.FormattingEnabled = true;
            this.PortName1.Location = new System.Drawing.Point(88, 19);
            this.PortName1.Name = "PortName1";
            this.PortName1.Size = new System.Drawing.Size(87, 21);
            this.PortName1.TabIndex = 0;
            // 
            // chkUseCOM1
            // 
            this.chkUseCOM1.AutoSize = true;
            this.chkUseCOM1.Checked = true;
            this.chkUseCOM1.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chkUseCOM1.Location = new System.Drawing.Point(18, 113);
            this.chkUseCOM1.Name = "chkUseCOM1";
            this.chkUseCOM1.Size = new System.Drawing.Size(48, 17);
            this.chkUseCOM1.TabIndex = 6;
            this.chkUseCOM1.Text = "Use ";
            this.chkUseCOM1.UseVisualStyleBackColor = true;
            // 
            // FlowCtrl1
            // 
            this.FlowCtrl1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.FlowCtrl1.FormattingEnabled = true;
            this.FlowCtrl1.Items.AddRange(new object[] {
            "None",
            "XON/XOFF",
            "RTS/CTS",
            "DSR/DTR"});
            this.FlowCtrl1.Location = new System.Drawing.Point(269, 81);
            this.FlowCtrl1.Name = "FlowCtrl1";
            this.FlowCtrl1.Size = new System.Drawing.Size(87, 21);
            this.FlowCtrl1.TabIndex = 5;
            // 
            // Parity1
            // 
            this.Parity1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.Parity1.FormattingEnabled = true;
            this.Parity1.Items.AddRange(new object[] {
            "None",
            "Odd",
            "Even",
            "Mark",
            "Space"});
            this.Parity1.Location = new System.Drawing.Point(88, 81);
            this.Parity1.Name = "Parity1";
            this.Parity1.Size = new System.Drawing.Size(87, 21);
            this.Parity1.TabIndex = 4;
            // 
            // Baud1
            // 
            this.Baud1.Location = new System.Drawing.Point(269, 19);
            this.Baud1.Name = "Baud1";
            this.Baud1.Size = new System.Drawing.Size(87, 20);
            this.Baud1.TabIndex = 1;
            this.Baud1.Text = "9600";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(190, 22);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(35, 13);
            this.label2.TabIndex = 0;
            this.label2.Text = "Baud:";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(190, 84);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(67, 13);
            this.label6.TabIndex = 0;
            this.label6.Text = "Flow control:";
            // 
            // StopBits1
            // 
            this.StopBits1.Location = new System.Drawing.Point(269, 51);
            this.StopBits1.Name = "StopBits1";
            this.StopBits1.Size = new System.Drawing.Size(87, 20);
            this.StopBits1.TabIndex = 3;
            this.StopBits1.Text = "1";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(190, 54);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(51, 13);
            this.label4.TabIndex = 0;
            this.label4.Text = "Stop bits:";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(15, 84);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(36, 13);
            this.label5.TabIndex = 0;
            this.label5.Text = "Parity:";
            // 
            // DataBits1
            // 
            this.DataBits1.Location = new System.Drawing.Point(88, 50);
            this.DataBits1.Name = "DataBits1";
            this.DataBits1.Size = new System.Drawing.Size(87, 20);
            this.DataBits1.TabIndex = 2;
            this.DataBits1.Text = "8";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(15, 53);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(52, 13);
            this.label3.TabIndex = 0;
            this.label3.Text = "Data bits:";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(15, 22);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(60, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Port Name:";
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.PortName2);
            this.groupBox2.Controls.Add(this.chkUseCOM2);
            this.groupBox2.Controls.Add(this.FlowCtrl2);
            this.groupBox2.Controls.Add(this.Parity2);
            this.groupBox2.Controls.Add(this.Baud2);
            this.groupBox2.Controls.Add(this.label7);
            this.groupBox2.Controls.Add(this.label8);
            this.groupBox2.Controls.Add(this.StopBits2);
            this.groupBox2.Controls.Add(this.label9);
            this.groupBox2.Controls.Add(this.label10);
            this.groupBox2.Controls.Add(this.DataBits2);
            this.groupBox2.Controls.Add(this.label11);
            this.groupBox2.Controls.Add(this.label12);
            this.groupBox2.Location = new System.Drawing.Point(12, 155);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(368, 139);
            this.groupBox2.TabIndex = 1;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Second COM";
            // 
            // PortName2
            // 
            this.PortName2.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.PortName2.FormattingEnabled = true;
            this.PortName2.Location = new System.Drawing.Point(88, 19);
            this.PortName2.Name = "PortName2";
            this.PortName2.Size = new System.Drawing.Size(87, 21);
            this.PortName2.TabIndex = 0;
            // 
            // chkUseCOM2
            // 
            this.chkUseCOM2.AutoSize = true;
            this.chkUseCOM2.Location = new System.Drawing.Point(18, 114);
            this.chkUseCOM2.Name = "chkUseCOM2";
            this.chkUseCOM2.Size = new System.Drawing.Size(48, 17);
            this.chkUseCOM2.TabIndex = 7;
            this.chkUseCOM2.Text = "Use ";
            this.chkUseCOM2.UseVisualStyleBackColor = true;
            // 
            // FlowCtrl2
            // 
            this.FlowCtrl2.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.FlowCtrl2.FormattingEnabled = true;
            this.FlowCtrl2.Items.AddRange(new object[] {
            "None",
            "XON/XOFF",
            "RTS/CTS",
            "DSR/DTR"});
            this.FlowCtrl2.Location = new System.Drawing.Point(269, 81);
            this.FlowCtrl2.Name = "FlowCtrl2";
            this.FlowCtrl2.Size = new System.Drawing.Size(87, 21);
            this.FlowCtrl2.TabIndex = 5;
            // 
            // Parity2
            // 
            this.Parity2.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.Parity2.FormattingEnabled = true;
            this.Parity2.Items.AddRange(new object[] {
            "None",
            "Odd",
            "Even",
            "Mark",
            "Space"});
            this.Parity2.Location = new System.Drawing.Point(88, 81);
            this.Parity2.Name = "Parity2";
            this.Parity2.Size = new System.Drawing.Size(87, 21);
            this.Parity2.TabIndex = 4;
            // 
            // Baud2
            // 
            this.Baud2.Location = new System.Drawing.Point(269, 19);
            this.Baud2.Name = "Baud2";
            this.Baud2.Size = new System.Drawing.Size(87, 20);
            this.Baud2.TabIndex = 1;
            this.Baud2.Text = "9600";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(190, 22);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(35, 13);
            this.label7.TabIndex = 0;
            this.label7.Text = "Baud:";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(190, 84);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(67, 13);
            this.label8.TabIndex = 0;
            this.label8.Text = "Flow control:";
            // 
            // StopBits2
            // 
            this.StopBits2.Location = new System.Drawing.Point(269, 51);
            this.StopBits2.Name = "StopBits2";
            this.StopBits2.Size = new System.Drawing.Size(87, 20);
            this.StopBits2.TabIndex = 3;
            this.StopBits2.Text = "1";
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(190, 54);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(51, 13);
            this.label9.TabIndex = 0;
            this.label9.Text = "Stop bits:";
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(15, 84);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(36, 13);
            this.label10.TabIndex = 0;
            this.label10.Text = "Parity:";
            // 
            // DataBits2
            // 
            this.DataBits2.Location = new System.Drawing.Point(88, 50);
            this.DataBits2.Name = "DataBits2";
            this.DataBits2.Size = new System.Drawing.Size(87, 20);
            this.DataBits2.TabIndex = 2;
            this.DataBits2.Text = "8";
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Location = new System.Drawing.Point(15, 53);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(52, 13);
            this.label11.TabIndex = 0;
            this.label11.Text = "Data bits:";
            // 
            // label12
            // 
            this.label12.AutoSize = true;
            this.label12.Location = new System.Drawing.Point(15, 22);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(60, 13);
            this.label12.TabIndex = 0;
            this.label12.Text = "Port Name:";
            // 
            // btnOK
            // 
            this.btnOK.Location = new System.Drawing.Point(147, 309);
            this.btnOK.Name = "btnOK";
            this.btnOK.Size = new System.Drawing.Size(75, 23);
            this.btnOK.TabIndex = 3;
            this.btnOK.Text = "OK";
            this.btnOK.UseVisualStyleBackColor = true;
            this.btnOK.Click += new System.EventHandler(this.btnOK_Click);
            // 
            // btnCancel
            // 
            this.btnCancel.Location = new System.Drawing.Point(239, 309);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new System.Drawing.Size(75, 23);
            this.btnCancel.TabIndex = 4;
            this.btnCancel.Text = "Cancel";
            this.btnCancel.UseVisualStyleBackColor = true;
            this.btnCancel.Click += new System.EventHandler(this.btnCancel_Click);
            // 
            // btnRefresh
            // 
            this.btnRefresh.Location = new System.Drawing.Point(12, 309);
            this.btnRefresh.Name = "btnRefresh";
            this.btnRefresh.Size = new System.Drawing.Size(102, 23);
            this.btnRefresh.TabIndex = 2;
            this.btnRefresh.Text = "Refresh COMs";
            this.btnRefresh.UseVisualStyleBackColor = true;
            this.btnRefresh.Click += new System.EventHandler(this.btnRefresh_Click);
            // 
            // frmCOMConfig
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(398, 340);
            this.Controls.Add(this.btnRefresh);
            this.Controls.Add(this.btnCancel);
            this.Controls.Add(this.btnOK);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.groupBox2);
            this.Name = "frmCOMConfig";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Config COM";
            this.Load += new System.EventHandler(this.frmConfig_Load);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.ComboBox FlowCtrl1;
        private System.Windows.Forms.ComboBox Parity1;
        private System.Windows.Forms.TextBox Baud1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.TextBox StopBits1;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TextBox DataBits1;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.ComboBox FlowCtrl2;
        private System.Windows.Forms.ComboBox Parity2;
        private System.Windows.Forms.TextBox Baud2;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.TextBox StopBits2;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.TextBox DataBits2;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.Label label12;
        private System.Windows.Forms.Button btnOK;
        private System.Windows.Forms.Button btnCancel;
        private System.Windows.Forms.CheckBox chkUseCOM1;
        private System.Windows.Forms.CheckBox chkUseCOM2;
        private System.Windows.Forms.ComboBox PortName1;
        private System.Windows.Forms.ComboBox PortName2;
        private System.Windows.Forms.Button btnRefresh;
    }
}