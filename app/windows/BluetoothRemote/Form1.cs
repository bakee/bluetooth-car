using System;
using System.IO.Ports;
using System.Windows.Forms;

namespace BluetoothRemote
{
    public partial class Form1 : Form
    {
        int _currentSpeed = 100;
        private SerialPort _serialPort;

        public Form1()
        {
            InitializeComponent();
        }

        private void trackBarSpeed_Scroll(object sender, EventArgs e)
        {
        }

        private void ReloadPorts()
        {
            comboBoxPorts.Items.Clear();
            var availablePorts = SerialPort.GetPortNames();

            comboBoxPorts.Items.AddRange(availablePorts);
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            ReloadPorts();
            EnableControls(false);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if(button1.Text == "Connect")
            {
                handleConnect();
            }
            else
            {
                handleDisconnect();
            }
        }

        private void EnableControls(bool enable)
        {
            buttonForward.Enabled = enable;
            buttonBackward.Enabled = enable;
            buttonLeft.Enabled = enable;
            buttonRight.Enabled = enable;
            buttonStop.Enabled = enable;
            trackBarSpeed.Enabled = enable;

            comboBoxPorts.Enabled = !enable;
            buttonRefresh.Enabled = !enable;
        }

        private void handleConnect()
        {
            if (comboBoxPorts.SelectedItem == null || String.IsNullOrEmpty(comboBoxPorts.SelectedItem.ToString()))
            {
                MessageBox.Show("Please select a port first", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            var portName = comboBoxPorts.SelectedItem.ToString();
            _serialPort = new SerialPort(portName, 9600, Parity.None, 8, StopBits.One);
            _serialPort.Open();
            button1.Text = "DisConnect";
            EnableControls(true);
        }

        private void handleDisconnect()
        {
            _serialPort.Close();
            button1.Text = "Connect";
            EnableControls(false);
        }

        private void buttonRefresh_Click(object sender, EventArgs e)
        {
            ReloadPorts();
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            Stop();
            if (_serialPort != null && _serialPort.IsOpen)
            {
                _serialPort.Close();
            }
        }

        private void Stop()
        {
            if (_serialPort == null || !_serialPort.IsOpen) return;
            _serialPort.Write("d0\n");
        }

        private void Forward()
        {
            if (_serialPort == null || !_serialPort.IsOpen) return;
            _serialPort.Write("d1\n");
        }

        private void Backward()
        {
            if (_serialPort == null || !_serialPort.IsOpen) return;
            _serialPort.Write("d2\n");
        }

        private void Left()
        {
            if (_serialPort == null || !_serialPort.IsOpen) return;
            _serialPort.Write("d3\n");
        }

        private void Right()
        {
            if (_serialPort == null || !_serialPort.IsOpen) return;
            _serialPort.Write("d4\n");
        }

        private void SetSpeed(int speed)
        {
            if(speed > 100)
            {
                speed = 100;
            }

            if(speed < 0)
            {
                speed = 0;
            }

            _currentSpeed = speed;

            if(trackBarSpeed.Value * 10 != _currentSpeed)
            {
                trackBarSpeed.Value = _currentSpeed / 10;
            }

            if (_serialPort == null || !_serialPort.IsOpen) return;
            var speedString = String.Format("s{0}\n", speed);
            _serialPort.Write(speedString);
        }

        private void buttonForward_Click(object sender, EventArgs e)
        {
            Forward();
        }

        private void buttonBackward_Click(object sender, EventArgs e)
        {
            Backward();
        }

        private void buttonLeft_Click(object sender, EventArgs e)
        {
            Left();
        }

        private void buttonRight_Click(object sender, EventArgs e)
        {
            Right();
        }

        private void buttonStop_Click(object sender, EventArgs e)
        {
            Stop();
        }

        private void trackBarSpeed_ValueChanged(object sender, EventArgs e)
        {
            var newValue = trackBarSpeed.Value;
            newValue *= 10;
            SetSpeed(newValue);
        }

        private void Form1_KeyPress(object sender, KeyPressEventArgs e)
        {
            if(e.KeyChar == 32)
            {
                Stop();
            }
            this.Focus();
            e.Handled = true;
        }

        protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
        {
            //capture up arrow key
            if (keyData == Keys.Up)
            {
                Forward();
                return true;
            }
            //capture down arrow key
            if (keyData == Keys.Down)
            {
                Backward();
                return true;
            }
            //capture left arrow key
            if (keyData == Keys.Left)
            {
                Left();
                return true;
            }
            //capture right arrow key
            if (keyData == Keys.Right)
            {
                Right();
                return true;
            }
            //capture right arrow key
            if (keyData == Keys.Add)
            {
                if(_currentSpeed < 100)
                {
                    _currentSpeed += 10;
                }
                SetSpeed(_currentSpeed);
                return true;
            }
            //capture right arrow key
            if (keyData == Keys.Subtract)
            {
                if (_currentSpeed > 0)
                {
                    _currentSpeed -= 10;
                }
                SetSpeed(_currentSpeed);
                return true;
            }            

            return base.ProcessCmdKey(ref msg, keyData);
        }
    }
}
