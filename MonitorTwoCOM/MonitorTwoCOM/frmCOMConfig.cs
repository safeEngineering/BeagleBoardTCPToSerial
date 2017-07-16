using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO.Ports;

namespace MonitorTwoCOM
{
    public partial class frmCOMConfig : Form
    {
        #region Fields

        // setting for first COM
        public COMSetting COM1Setting;
        // setting for second COM
        public COMSetting COM2Setting;

        #endregion

        #region Form handlers

        public frmCOMConfig()
        {
            InitializeComponent();
        }

        private void frmConfig_Load(object sender, EventArgs e)
        {
            try
            {
                string[] portNames = SerialPort.GetPortNames();

                if(COM1Setting != null) // set initial settings for first COM
                {
                    foreach(string port in portNames)
                    {
                        PortName1.Items.Add(port);
                        if (port == COM1Setting.PortName)                        
                            PortName1.SelectedItem = port;                        
                    }
                    Baud1.Text = COM1Setting.Baud.ToString();
                    DataBits1.Text = COM1Setting.DataBits.ToString();
                    StopBits1.Text = ((int)COM1Setting.StopBits).ToString();
                    Parity1.SelectedIndex = (int)COM1Setting.Parity;
                    FlowCtrl1.SelectedIndex = (int)COM1Setting.Handshake;
                    chkUseCOM1.Checked = COM1Setting.Use;
                }

                if (COM2Setting != null)    // set initial settings for second COM
                {
                    foreach (string port in portNames)
                    {
                        PortName2.Items.Add(port);
                        if (port == COM2Setting.PortName)
                            PortName2.SelectedItem = port;
                    }
                    Baud2.Text = COM2Setting.Baud.ToString();
                    DataBits2.Text = COM2Setting.DataBits.ToString();
                    StopBits2.Text = ((int)COM2Setting.StopBits).ToString();
                    Parity2.SelectedIndex = (int)COM2Setting.Parity;
                    FlowCtrl2.SelectedIndex = (int)COM2Setting.Handshake;
                    chkUseCOM2.Checked = COM2Setting.Use;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "COM Setting");
            }
        }

        private void btnRefresh_Click(object sender, EventArgs e)
        {
            try
            {
                // get list that holds all COM ports
                string[] portNames = SerialPort.GetPortNames();
                // clear current content of first COM
                PortName1.Items.Clear();
                // populate with new content
                foreach (string port in portNames)
                {
                    PortName1.Items.Add(port);
                    if (port == COM1Setting.PortName)
                        PortName1.SelectedItem = port;
                }

                // clear current content of second COM
                PortName2.Items.Clear();
                // populate with new content
                foreach (string port in portNames)
                {
                    PortName2.Items.Add(port);
                    if (port == COM2Setting.PortName)
                        PortName2.SelectedItem = port;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "COM Setting");
            }
        }

        private void btnOK_Click(object sender, EventArgs e)
        {
            try
            {
                // save first COM settings
                COM1Setting.PortName = (string)PortName1.SelectedItem;
                COM1Setting.Baud = int.Parse(Baud1.Text);
                COM1Setting.DataBits = int.Parse(DataBits1.Text);
                COM1Setting.StopBits = (StopBits)int.Parse(StopBits1.Text);
                COM1Setting.Parity = (Parity)Parity1.SelectedIndex;
                COM1Setting.Handshake = (Handshake)FlowCtrl1.SelectedIndex;
                COM1Setting.Use = chkUseCOM1.Checked;

                // save second COM settings
                COM2Setting.PortName = (string)PortName2.SelectedItem;
                COM2Setting.Baud = int.Parse(Baud2.Text);
                COM2Setting.DataBits = int.Parse(DataBits2.Text);
                COM2Setting.StopBits = (StopBits)int.Parse(StopBits2.Text);
                COM2Setting.Parity = (Parity)Parity2.SelectedIndex;
                COM2Setting.Handshake = (Handshake)FlowCtrl2.SelectedIndex;
                COM2Setting.Use = chkUseCOM2.Checked;

                DialogResult = DialogResult.OK;
                this.Close();
            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.Message, "COM Setting");
            }
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            try
            {
                DialogResult = DialogResult.Cancel;
                this.Close();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "COM Setting");
            }
        }

        #endregion
    }

    public class COMSetting
    {
        public string PortName;
        public int Baud;
        public int DataBits;
        public StopBits StopBits;
        public Parity Parity;
        public Handshake Handshake;
        public bool Use;
    }
}
