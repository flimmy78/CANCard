using System;
using System.Collections.Generic;
using System.Text;
using Jungo.wdapi_dotnet;
using wdc_err = Jungo.wdapi_dotnet.WD_ERROR_CODES;
using DWORD = System.UInt32;
using BOOL = System.Boolean;
using WDC_DRV_OPEN_OPTIONS = System.UInt32;
using System.Runtime.InteropServices;
using System.Collections;
using System.Threading;

namespace CANCard
{
    class CardTest
    {

        enum CardState
        {
            Idle, //停止读写
            Ready,//准备读写
            Reading, //开始读
            Writing //开始写
        }

        private readonly static string ISA_DEFAULT_LICENSE_STRING = "6C3CC2CFE89E7AD0424A070D434A6F6DC49537A5.zhong";
        // TODO: If you have renamed the WinDriver kernel module (windrvr6.sys),
        //  change the driver name below accordingly
        private readonly static string ISA_DEFAULT_DRIVER_NAME = "windrvr6";

        private static bool initialized = false;
        IntPtr ptrCard = IntPtr.Zero;
        ISA_CARD card = new ISA_CARD();
        WDC_DEVICE dev = new WDC_DEVICE();
        INT_HANDLER intHandler;

        byte[] frameBuffer = new byte[12];

        uint ioStart = 0x270; //IO 起始地址
        uint ioRange = 0x10; //IO 范围
        uint intNum = 0x0a; //中断向量号

        CardState workState = CardState.Idle;
        ManualResetEvent eventFrameRcv = new ManualResetEvent(false);
        Queue<byte[]> rcvQueue = new Queue<byte[]>(); //接收的队列
        Queue<byte[]> sendQueue = new Queue<byte[]>(); //发送数据帧的队列

        System.Timers.Timer timerSend = new System.Timers.Timer();

        public event EventHandler FrameReceived = null;

        int bufferFrameCnt = 1024; //缓存数据帧的个数



        public static DWORD LibInit()
        {
            if (initialized)
            {
                return (DWORD)wdc_err.WD_STATUS_SUCCESS;
            }
            if (windrvr_decl.WD_DriverName(ISA_DEFAULT_DRIVER_NAME) == null)
            {
                return (DWORD)wdc_err.WD_SYSTEM_INTERNAL_ERROR;
            }
            DWORD dwStatus = wdc_lib_decl.WDC_SetDebugOptions(wdc_lib_consts.WDC_DBG_DEFAULT, null);
            if (dwStatus != (DWORD)wdc_err.WD_STATUS_SUCCESS)
            {
                return dwStatus;
            }
            dwStatus = wdc_lib_decl.WDC_DriverOpen((WDC_DRV_OPEN_OPTIONS)wdc_lib_consts.WDC_DRV_OPEN_DEFAULT, ISA_DEFAULT_LICENSE_STRING);
            if (dwStatus == (DWORD)wdc_err.WD_STATUS_SUCCESS)
            {
                initialized = true;
            }
            return dwStatus;
        }

        public CardTest()
        {
            if (initialized == false)
            {
                LibInit();
            }
      

        }

    

        public uint IOStart
        {
            get { return ioStart; }
            set
            {
                if (this.IsOpen == false)
                { ioStart = value; }
            }
        }

        public uint IORage
        {
            get { return ioRange; }
            set
            {
                if (this.IsOpen == false)
                { ioRange = value; }
            }
        }

        public uint IntNum
        {
            get { return intNum; }
            set
            {
                if (this.IsOpen == false)
                { intNum = value; }
            }
        }

        public int FrameNum
        {
            get
            {
                return rcvQueue.Count;
            }
        }

        public int BufferFrameCount
        {
            get
            {
                return bufferFrameCnt;
            }
            set
            {
                if (bufferFrameCnt != value)
                {
                    bufferFrameCnt = value;
                    lock (((ICollection)rcvQueue).SyncRoot)
                    {
                        rcvQueue.Clear();
                    }
                }
            }
        }
        /// <summary>
        /// 打开设备
        /// </summary>
        /// <returns></returns>
        public bool Open()
        {
            if (dev.hDev == IntPtr.Zero)
            {

                card.dwItems = 2;

                // BAR0 
                card.Item0.item = (uint)ITEM_TYPE.ITEM_IO;
                card.Item0.fNotSharable = 0;
                card.Item0.I.IO.dwAddr = ioStart;
                card.Item0.I.IO.dwBytes = ioRange;
                card.Item0.I.IO.dwBar = 0;

                // Int
                card.Item1.item = (uint)ITEM_TYPE.ITEM_INTERRUPT;
                card.Item1.fNotSharable = 0;
                card.Item1.I.Int.dwInterrupt = intNum;
                card.Item1.I.Int.dwOptions = 0;

                ptrCard = Marshal.AllocHGlobal(Marshal.SizeOf(card));
                Marshal.StructureToPtr(card, ptrCard, false);


                uint status = wdc_lib_decl.WDC_IsaDeviceOpen(ref dev.hDev, ptrCard, IntPtr.Zero, IntPtr.Zero, null, IntPtr.Zero);


                if (status == (uint)WD_ERROR_CODES.WD_STATUS_SUCCESS)
                {

                    intHandler = new INT_HANDLER(ProcessInterrupt);

                    wdc_lib_decl.WDC_IntEnable((WDC_DEVICE)(dev), null, 0, 0, intHandler, IntPtr.Zero, false);
                    wdc_lib_decl.WDC_IntIsEnabled(dev.hDev);
                    //停止读写
                    return true;
                }
                else
                {
                    dev.hDev = IntPtr.Zero;
                    return false;
                }

            }
            return true;
        }


        public bool IsOpen
        {
            get
            {
                if (dev.hDev == IntPtr.Zero)
                {
                    return false;
                }
                return true;
            }
        }

        public void Close()
        {
            wdc_lib_decl.WDC_IntDisable((WDC_DEVICE)(dev));
            wdc_lib_decl.WDC_IsaDeviceClose(dev.hDev);
            dev.hDev = IntPtr.Zero;
            if (ptrCard != IntPtr.Zero)
            {
                Marshal.FreeHGlobal(ptrCard);
            }
        }


        public UInt16 ReadUInt16(uint offset)
        {
            ushort data = 0;
            wdc_lib_decl.WDC_ReadAddr16(dev.hDev, 0, offset, ref data);
            return data;
        }

        public void WriteUInt16(uint offset, UInt16 data)
        {
            wdc_lib_decl.WDC_WriteAddr16(dev.hDev, 0, offset, data);
        }

        private void WriteData(UInt16 data)
        {
            WriteUInt16(0x08, data);
        }
        private UInt16 ReadData()
        {
            return ReadUInt16(0x04);
        }


        public byte[] GetFrame(int timeout)
        {
            byte[] frame = null;
            lock (((ICollection)rcvQueue).SyncRoot)
            {
                int cnt = rcvQueue.Count;
                if (cnt > 0)
                {
                    frame = rcvQueue.Dequeue();
                }
                if (cnt <= 1)
                {
                    eventFrameRcv.Reset();
                }
            }
            if (frame != null || timeout == 0) return frame;
            if (timeout < 0)
            {
                eventFrameRcv.WaitOne();
            }
            else
            {
                eventFrameRcv.WaitOne(timeout, false);
            }
            lock (((ICollection)rcvQueue).SyncRoot)
            {
                int cnt = rcvQueue.Count;
                if (cnt > 0)
                {
                    frame = rcvQueue.Dequeue();
                }
                if (cnt <= 1)
                {
                    eventFrameRcv.Reset();
                }
            }
            return frame;
        }

        public void PutFrame(byte[] frame)
        {
            lock (((ICollection)sendQueue).SyncRoot)
            {
                sendQueue.Enqueue(frame);
                if (timerSend.Enabled == false)
                {
                    timerSend.Start();
                }
            }
        }



        private void ProcessInterrupt(IntPtr pData)
        {
            if (workState == CardState.Idle) //开始读操作
            {
                ReadUInt16(6);  //开始读
                workState = CardState.Reading;
                int index = 0;
                int invalidCnt = 0;
                while (true)
                {
                    UInt16 data = ReadData();
                    if (data == 0xffff) { invalidCnt++; }
                    frameBuffer[index * 2] = (byte)((data >> 8) & 0xff);
                    frameBuffer[index * 2 + 1] = (byte)(data & 0xff);
                    index++;
                    if (index == 6)
                    {
                        if (invalidCnt == 6)
                        {
                            WriteUInt16(2, 0); //停止读写
                            workState = CardState.Idle;
                            //看看发送缓存区是否有数据,有数据就启动发送
                            lock (((ICollection)sendQueue).SyncRoot)
                            {
                                if (sendQueue.Count > 0)
                                {
                                    if (timerSend.Enabled == false)
                                    {
                                        timerSend.Start();
                                    }
                                }
                            }
                            break;
                        }
                        else
                        {
                            byte[] frame = new byte[10];
                            Array.Copy(frameBuffer, 2, frame, 0, 10);
                            lock (((ICollection)rcvQueue).SyncRoot)
                            {
                                if (rcvQueue.Count > bufferFrameCnt)
                                {
                                    rcvQueue.Dequeue();
                                }
                                rcvQueue.Enqueue(frame);
                                eventFrameRcv.Set();
                            }

                            if (FrameReceived != null)
                            {
                                FrameReceived(this, null);
                            }
                        }
                        index = 0;
                        invalidCnt = 0;
                    }
                }

            }
            else if (workState == CardState.Ready)
            {
                byte[] frame = null;
                lock (((ICollection)sendQueue).SyncRoot)
                {
                    frame = sendQueue.Dequeue();
                }
                if (frame != null)
                {
                    WriteUInt16(0x06, 0);//开始写操作()

                    WriteData(0xffff);
                    for (int i = 0; i < 5; i++)
                    {
                        UInt16 data = 0;
                        if ((2 * i + 1) < frame.Length)
                        {
                            data = (UInt16)(frame[i * 2 + 1] + (frame[i * 2] << 8));
                        }
                        WriteData(data);
                    }
                    WriteUInt16(2, 0); //停止读写
                    workState = CardState.Idle;

                }
                else
                {
                    workState = CardState.Idle;
                    WriteUInt16(2, 0); //停止读写
                }

            }
        }


    }



}

