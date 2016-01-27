using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace CANCard
{
    public partial class Form2 : Form
    {

        const int ISA_START = 0x200;
        const int ISA_RANGE = 0x80;
        CardTest testCard = new CardTest();

        UInt16 [] testData=new ushort[ISA_RANGE];
        public Form2()
        {
            InitializeComponent();
        }

        private void Form2_Load(object sender, EventArgs e)
        {
            testCard.IOStart = ISA_START;
            testCard.IORage = ISA_RANGE;

            if (testCard.Open()==false)
            {
                MessageBox.Show("失败");
            }

            
        }

        const int RAM_SIZE = 0x3ff;
        UInt16[] RAM_Mem=new UInt16[RAM_SIZE];
        byte[] buffer = new byte[RAM_SIZE * 2];
        Random rnd = new Random();

        private void button1_Click(object sender, EventArgs e)
        {


            rnd.NextBytes(buffer);
            
            for (int i = 0; i < RAM_SIZE; i++)
            {
                testCard.WriteUInt16(0, (UInt16)i);
                RAM_Mem[i] = BitConverter.ToUInt16(buffer, i * 2);
                testCard.WriteUInt16(2, (ushort)RAM_Mem[i]);
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            for (int i = 0; i < RAM_SIZE; i ++)
            {
                testCard.WriteUInt16(0, (UInt16)i);
               ushort val= testCard.ReadUInt16(2);
               if (val != RAM_Mem[i])
               {
                   Console.WriteLine(val.ToString("X4"));
               }
            }
        }

        private void button3_Click(object sender, EventArgs e)
        {
            int i = 0;
            while (true)
            {
                button1_Click(null, null);
                button2_Click(null, null);
                Console.WriteLine(i++);
                Application.DoEvents();
            }
        }

       


    }
}
