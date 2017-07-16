using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace MonitorTwoCOM
{
    public partial class frmBoardConfig : Form
    {
        #region Fields

        public frmMain FrmMain = null;
        public BoardConfig BoardConfig = null;

        #endregion

        #region Form methods

        public frmBoardConfig()
        {
            InitializeComponent();
        }

        private void frmBoardConfig_Load(object sender, EventArgs e)
        {
            try
            {
                UpdateGUI(BoardConfig);
            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.Message, "BoardConfig");
            }
        }

        private void radio_NoWaitACK_CheckedChanged(object sender, EventArgs e)
        {
            if (radio_NoWaitACK.Checked)
                txtRepeatTimes.Enabled = true;
            else
                txtRepeatTimes.Enabled = false;
        }

        private void btnOK_Click(object sender, EventArgs e)
        {
            SensorCommand sensorCmd;
            string warningMsg;
            int repeat, measurement;

            try
            {
                if (BoardConfig == null)
                {
                    BoardConfig = new BoardConfig();
                    BoardConfig.SensorCommandList = new List<SensorCommand>();
                }

                measurement = int.Parse(txtMInterval.Text);
                if ((1440 % measurement) != 0)
                {
                    warningMsg = "Measurement interval was invalid for time synchronization. If you want to synchronize time";
                    warningMsg += "stamps on all boards the measurement interval must be submultiple of 1440 (day's minutes).";
                    warningMsg += " Do you want to continue?";
                    if (MessageBox.Show(warningMsg, "BoardConfig", MessageBoxButtons.YesNo, MessageBoxIcon.Warning) == DialogResult.No)
                        return;
                }

                repeat = int.Parse(txtRepeatTimes.Text);
                if (repeat < 1 || repeat > 15)
                {
                    MessageBox.Show("Repeat times was invalid!", "BoardConfig", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                    return;
                }

                BoardConfig.MeasurementInterval = int.Parse(txtMInterval.Text);
                BoardConfig.TransmittingInterval = int.Parse(txtTInterval.Text);
                BoardConfig.BatteryCycle = int.Parse(txtTBattery.Text);

                BoardConfig.DeviceAddress = txtDevAdd.Text.Trim();
                BoardConfig.DeviceEUI = txtDevEUI.Text.Trim();
                BoardConfig.NetworkKey = txtNetSKey.Text.Trim();
                BoardConfig.ApplicationKey = txtAppSKey.Text.Trim();
                BoardConfig.WaitACK = radio_WaitACK.Checked;
                BoardConfig.RepeatTimes = int.Parse(txtRepeatTimes.Text);

                // remove all current commands
                BoardConfig.SensorCommandList.Clear();
                foreach(DataGridViewRow row in gridSDICommand.Rows)
                {
                    if(row.Cells[1].Value != null && !string.IsNullOrEmpty(row.Cells[1].Value.ToString()) &
                       row.Cells[2].Value != null && !string.IsNullOrEmpty(row.Cells[2].Value.ToString()))
                    {
                        sensorCmd = new SensorCommand();
                        sensorCmd.MeasurementCommand = row.Cells[1].Value.ToString().Trim();
                        sensorCmd.DataCommand = row.Cells[2].Value.ToString().Trim();
                        BoardConfig.SensorCommandList.Add(sensorCmd);
                    }
                }

                this.DialogResult = DialogResult.OK;
                this.Close();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "BoardConfig");
            }
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            try
            {
                this.DialogResult = DialogResult.Cancel;
                this.Close();
            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.Message, "BoardConfig");
            }
        }

        private void gridSDICommand_RowsRemoved(object sender, DataGridViewRowsRemovedEventArgs e)
        {
            try
            {
                // add new row at the tail
                int index = gridSDICommand.Rows.Add();
                gridSDICommand.Rows[index].Cells[1].Value = "";
                gridSDICommand.Rows[index].Cells[2].Value = "";
                // update index column value
                for (int i = e.RowIndex; i < 40; i++)
                {
                    gridSDICommand.Rows[i].Cells[0].Value = (i + 1).ToString();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "RemoveSDICommand");
            }
        }

        private void btnOpen_Click(object sender, EventArgs e)
        {
            try
            {
                if (FrmMain == null)
                    return;

                OpenFileDialog openFileDialog = new OpenFileDialog();
                openFileDialog.InitialDirectory = Application.StartupPath;
                openFileDialog.DefaultExt = ".xml";
                openFileDialog.Filter = "XML files (*.xml)|*.xml|All files (*.*)|*.*";
                openFileDialog.FilterIndex = 1;
                if (openFileDialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
                {
                    BoardConfig savedCfg = FrmMain.LoadBoardSettings(openFileDialog.FileName);
                    if (savedCfg != null)
                    {
                        // Disable temporary RowRemoved event from Grid
                        gridSDICommand.RowsRemoved -= gridSDICommand_RowsRemoved;
                        // Clear current sensor comamnds
                        gridSDICommand.Rows.Clear();

                        // Update GUI
                        UpdateGUI(savedCfg);

                        // Enable RowRemoved event again
                        gridSDICommand.RowsRemoved += new System.Windows.Forms.DataGridViewRowsRemovedEventHandler(gridSDICommand_RowsRemoved);
                    }
                }
            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.Message, "btnOpen_Click");
            }
        }

        private void btnGetFromBoard_Click(object sender, EventArgs e)
        {
            try
            {
                BoardConfig deviceSettings = FrmMain.GetSettingsOnBoard();
                if (deviceSettings != null)
                {
                    // Disable temporary RowRemoved event from Grid
                    gridSDICommand.RowsRemoved -= gridSDICommand_RowsRemoved;
                    // Clear current sensor comamnds
                    gridSDICommand.Rows.Clear();

                    // Update GUI
                    UpdateGUI(deviceSettings);

                    // Enable RowRemoved event again
                    gridSDICommand.RowsRemoved += new System.Windows.Forms.DataGridViewRowsRemovedEventHandler(gridSDICommand_RowsRemoved);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "btnGetFromBoard_Click");
            }
        }

        #endregion

        #region Helpers

        private void UpdateGUI(BoardConfig newSettings)
        {
            int index;
            SensorCommand sensorCmd;

            try
            {
                if (newSettings == null)
                    return;

                // add 40 empty rows to grid
                for (int i = 0; i < 40; i++)
                {
                    index = gridSDICommand.Rows.Add();

                    gridSDICommand.Rows[index].Cells[0].Value = (i + 1).ToString();
                    if (i < newSettings.SensorCommandList.Count)
                    {
                        sensorCmd = newSettings.SensorCommandList[i];
                        gridSDICommand.Rows[index].Cells[1].Value = sensorCmd.MeasurementCommand;
                        gridSDICommand.Rows[index].Cells[2].Value = sensorCmd.DataCommand;
                    }
                    else
                    {
                        gridSDICommand.Rows[index].Cells[1].Value = "";
                        gridSDICommand.Rows[index].Cells[2].Value = "";
                    }
                }

                txtMInterval.Text = newSettings.MeasurementInterval.ToString();
                txtTInterval.Text = newSettings.TransmittingInterval.ToString();
                txtTBattery.Text = newSettings.BatteryCycle.ToString();

                txtDevAdd.Text = newSettings.DeviceAddress;
                txtDevEUI.Text = newSettings.DeviceEUI;
                txtNetSKey.Text = newSettings.NetworkKey;
                txtAppSKey.Text = newSettings.ApplicationKey;

                if (newSettings.WaitACK)
                {
                    radio_WaitACK.Checked = true;
                    txtRepeatTimes.Enabled = false;
                }
                else
                {
                    radio_NoWaitACK.Checked = true;
                    txtRepeatTimes.Enabled = true;
                }

                txtRepeatTimes.Text = newSettings.RepeatTimes.ToString();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "UpdateGUI");
            }
        }

        #endregion
    }

    public class SensorCommand
    {
        public string MeasurementCommand { get; set; }      // command requests sensor making measurement operation
        public string DataCommand { get; set; }             // command requests sensor getting data from sensor
    }

    public class BoardConfig
    {
        public bool SendSaveCmd { get; set; }               // specify whether sending "Save EEPROM" command or not

        public int MeasurementInterval { get; set; }        // measurement interval (s)
        public int TransmittingInterval { get; set; }       // transmitting interval (s)
        public int BatteryCycle { get; set; }               // the transmitting intervals number for sending battery. Zero if we don't want to send

        public string DeviceAddress { get; set; }           // LoRa device address
        public string DeviceEUI { get; set; }               // LoRa EUI
        public string NetworkKey { get; set; }              // network security key in ABP mode
        public string ApplicationKey { get; set; }          // application security key in ABP mode
        public bool  WaitACK { get; set; }                  // specify whether waiting ACK from server or not
        public int RepeatTimes { get; set; }                // the times re-send a packet

        public List<SensorCommand> SensorCommandList { get; set; }  // SDI12 command collection
    }
}
