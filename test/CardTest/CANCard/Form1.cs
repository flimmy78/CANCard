using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Threading;
using System.Diagnostics;

namespace CANCard
{
    public partial class Form1 : Form
    {
        CANCard card = new CANCard();
        Thread threadRead;
        public Form1()
        {
            InitializeComponent();

            //card.FrameReceived += new EventHandler(card_FrameReceived);
        }

        void card_FrameReceived(object sender, EventArgs e)
        {
           byte[] frame= card.GetFrame(0);
           if (frame != null)
           {
               StringBuilder sb = new StringBuilder();
               for (int i = 0; i < frame.Length; i++)
               {
                   sb.Append(frame[i].ToString("X2")+" ");
               }
               sb.AppendLine();
               
           }
              
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (threadRead == null || threadRead.IsAlive == false)
            {
                threadRead = new Thread(new ThreadStart(ProcRead));
                threadRead.IsBackground = true;
                threadRead.Start();
            }
           


        }

        private void ProcRead()
        {
            while (true)
            {
              byte[] frame=  card.GetFrame(-1);
              if (frame != null)
              {
                  StringBuilder sb = new StringBuilder();
                  for (int i = 0; i < frame.Length; i++)
                  {
                      sb.Append(frame[i].ToString("X2") + " ");
                  }
                  sb.AppendLine();
                  this.Invoke(new MethodInvoker(delegate
                  {
                      textBox1.AppendText(sb.ToString());
                  }));
              }

            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            if (card.IsOpen)
            {
                card.Close();
                button2.Text = "打开";
            }
            else
            {
                //card.IOStart = 0x200;
                
                if(card.Open())
                button2.Text = "关闭";
            }
            
        }

        private void textBox1_DoubleClick(object sender, EventArgs e)
        {
            this.textBox1.Clear();
        }

        private void button3_Click(object sender, EventArgs e)
        {
            
            for (int i = 0; i < 100; i++)
            {
                byte[] frame = new byte[] { 2, 0x23, 0x01, 0x02, 0x03, 0, 0, 0, 0, 0, };
                frame[4] = (byte)i;
                card.PutFrame(frame);
            }
        }

        private void button4_Click(object sender, EventArgs e)
        {
            byte[] frame = new byte[] { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, };
            card.PutFrame(frame);
        }

        private void button5_Click(object sender, EventArgs e)
        {
            for (int i = 0; i < 10000; i++)
            {
                button2_Click(null, null);
                Application.DoEvents();
            }
        }

        private void button6_Click(object sender, EventArgs e)
        {
            UInt16 addr = UInt16.Parse(textBox2.Text, System.Globalization.NumberStyles.AllowHexSpecifier);
            UInt16 data = UInt16.Parse(textBox3.Text, System.Globalization.NumberStyles.AllowHexSpecifier);
            card.WriteUInt16(addr, data);
        }

        private void button7_Click(object sender, EventArgs e)
        {
            card.WriteUInt16(0, 0xffff);
            card.WriteUInt16(0, 0);
        }




        private void button8_Click(object sender, EventArgs e)
        {
            Thread thread = new Thread(new ThreadStart(testProc));
            thread.IsBackground = true;
            thread.Start();
        }



        int count = 0;
        private void testProc()
        {
            while (true)
            {
                Thread.Sleep(10);

                card.WriteUInt16(0, 0);

                Console.WriteLine("T"+count);
                count++;
            }
        }



        /// <summary>
        /// 测试ISA读取速度
        /// </summary>
        private void testProc1()
        {

            Stopwatch sw = new Stopwatch();

            sw.Start();
            long num = 0;
            while (true)
            {
                while (true)
                {
                   UInt16 data= card.ReadUInt16(0);
                   num += 2;
                    long tm=sw.ElapsedMilliseconds;
                    if (tm > 5000)
                    {
                        float rate = num * 1000f / tm/1024;
                        Console.WriteLine("读:"+rate+"KB");
                        num = 0;
                        sw.Reset();
                        sw.Start();
                        break;

                    }
                    if ((num % 2048) == 0)
                    {
                        Thread.Sleep(1);
                    }

                }
             

                while (true)
                {
                    card.WriteUInt16(0,0xff10);
                    num += 2;
                    long tm = sw.ElapsedMilliseconds;
                    if (tm > 5000)
                    {
                        float rate = num * 1000f / tm / 1024;
                        Console.WriteLine("写:"+rate + "KB");
                        num = 0;
                        sw.Reset();
                        sw.Start();
                        break;

                    }
                    if ((num % 2048) == 0)
                    {
                        Thread.Sleep(1);
                    }

                }



            }


 
        }

        private void button7_Click_1(object sender, EventArgs e)
        {
            Thread thread = new Thread(new ThreadStart(testProc1));
            thread.IsBackground = true;
            thread.Start();
        }

        private void button9_Click(object sender, EventArgs e)
        {
            Thread thread = new Thread(new ThreadStart(RAMTest));
            thread.IsBackground = true;
            thread.Start();
        }

        private void RAMTest()
        {
            Stopwatch sw = new Stopwatch();
            sw.Start();
            long sum = 0;


            byte[] data=new byte[2048];

            Random rdn = new Random();

            while (true)
            {

                rdn.NextBytes(data);
                card.ReadUInt16(6); //复位指针
                for (int i = 0; i < 1024; i++)
                {
                    //write ram address
                    UInt16 tmp=(ushort)(data[i*2]+(data[i*2+1]<<8));
                    card.WriteUInt16((UInt16)4, (ushort)tmp);
                    card.ReadUInt16(4);
                }
                sum += 2048; //2K Bytes
                long tm = sw.ElapsedMilliseconds;
                if (tm > 5000)
                {
                    float rate = sum * 1000f / tm;
                    Console.WriteLine(rate);
                    sum = 0;
                    sw.Reset();
                    sw.Start();
                }

                card.ReadUInt16(6); //复位指针
                for (int i = 0; i < 1024; i++)
                {
                    //card.WriteUInt16((UInt16)2, (ushort)i); //write ram address
                    UInt16 ramData = card.ReadUInt16(4); //read ram data
                    if (ramData != (UInt16)(data[i * 2] + (data[i * 2 + 1] << 8)))
                    {
                        Console.WriteLine("Err");
                        Console.Beep();
                    }
                }

            }
        }

        private void button10_Click(object sender, EventArgs e)
        {
            UInt16 addr = UInt16.Parse(textBox2.Text, System.Globalization.NumberStyles.AllowHexSpecifier);
           UInt16 data= card.ReadUInt16(addr);
            textBox3.Text = data.ToString("X2");
        }

        private void button11_Click(object sender, EventArgs e)
        {
            for (int i = 0; i < 1024; i++)
            {
                card.WriteUInt16((UInt16)2, (ushort)i); //write ram address
                UInt16 tmp = (UInt16)i;
                card.WriteUInt16((UInt16)4, tmp); //write ram data
            }

            MessageBox.Show("OK");
        }

        private void button12_Click(object sender, EventArgs e)
        {
            //通知CPU释放总线
            card.WriteUInt16(0, 2);
            card.WriteUInt16(0, 0);

            
        }

        private void button13_Click(object sender, EventArgs e)
        {
            card.Clear();
        }

        private void button14_Click(object sender, EventArgs e)
        {
            for (int i = 0; i < 1024; i++)
            {
                card.WriteUInt16((UInt16)2, (ushort)i); //write ram address
                
                card.WriteUInt16((UInt16)4, (UInt16)i); //write ram data
            }

            MessageBox.Show("OK");
        }

        private void button15_Click(object sender, EventArgs e)
        {

            for(int i=0;i<1024;i++)
            {
               int t= card.ReadUInt16((UInt16)4);
               if (t != 0)
               {
                   Console.Beep();
               }
            }
            
        }

        private void button16_Click(object sender, EventArgs e)
        {
            int cnt = int.Parse(textBox4.Text);

            for (int i = 0; i < cnt; i++)
            {
                card.SendPakcet((UInt32)i);
            }


            card.WriteUInt16(0, 4);
            card.WriteUInt16(0, 0);
        }





    }
}
