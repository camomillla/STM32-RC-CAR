using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace PC_Application
{
    public partial class Charts : Form
    {
        private MainWindow mainWindow;
        public Charts(MainWindow mainWindow)
        {
            InitializeComponent();
            this.mainWindow = mainWindow;
            this.Owner = this.mainWindow;
            this.Init();
        }

        private void Init()
        {
            this.LocationChanged += Charts_LocationChanged;
            this.Location = new Point(this.mainWindow.steeringWindow.Width + 2, this.mainWindow.Height - 4);
        }

        private void Charts_LocationChanged(object sender, EventArgs e)
        {
            if (!this.Focused)
                return;

            this.mainWindow.steeringWindow.Left = this.mainWindow.Left - this.mainWindow.steeringWindow.Width + 12;
            this.mainWindow.steeringWindow.Top = this.mainWindow.Top;

            this.mainWindow.Left = this.Left;
            this.mainWindow.Top = this.Top - this.mainWindow.Height + 4;
        }
    }
}
