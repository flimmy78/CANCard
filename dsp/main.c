#include "CanCardcfg.h"
#include <bios.h>
#include <gio.h>
#include <string.h>

#define DSP28_DATA_TYPES

#include "DSP281x_Device.h"



void InitScia();
void DisableDog();

void InitPeripheralClocks();


extern unsigned int RamfuncsLoadStart; 
extern unsigned int RamfuncsLoadEnd; 
extern unsigned int RamfuncsRunStart; 

GIO_Handle gioHandle;
GIO_Handle gioOut=NULL;

GIO_Handle canOutHandle=NULL;
GIO_Handle canInHandle=NULL;


GIO_Handle isaOutHandle=NULL;
GIO_Handle isaInHandle=NULL;
void userInit()
{
	DisableDog();
	InitPeripheralClocks();
}


#pragma CODE_SECTION(InitFlash, "ramfuncs")
void InitFlash(void)
{
	asm(" EALLOW");									// Enable EALLOW protected register access
	FlashRegs.FPWR.bit.PWR = 3;						// Pump and bank set to active mode
	FlashRegs.FSTATUS.bit.V3STAT = 1;				// Clear the 3VSTAT bit
	FlashRegs.FSTDBYWAIT.bit.STDBYWAIT = 0x01FF;	// Sleep to standby transition cycles
	FlashRegs.FACTIVEWAIT.bit.ACTIVEWAIT = 0x01FF;	// Standby to active transition cycles
	FlashRegs.FBANKWAIT.bit.RANDWAIT = 5;			// Random access waitstates
	FlashRegs.FBANKWAIT.bit.PAGEWAIT = 5;			// Paged access waitstates
	FlashRegs.FOTPWAIT.bit.OTPWAIT = 8;				// OTP waitstates
	FlashRegs.FOPT.bit.ENPIPE = 1;					// Enable the flash pipeline
	asm(" EDIS");									// Disable EALLOW protected register access

/*** Force a complete pipeline flush to ensure that the write to the last register
     configured occurs before returning.  Safest thing is to wait 8 full cycles. ***/

    asm(" RPT #6 || NOP");

} //end of InitFlash()

/*
 * main.c
 */
void main(void) {
	Int status;
	
	memcpy(&RamfuncsRunStart, 
       &RamfuncsLoadStart, 
       &RamfuncsLoadEnd - &RamfuncsLoadStart);
      
    InitFlash();   
	
	gioHandle=GIO_create("/uart0",IOM_INPUT,&status,NULL,NULL);
	gioOut=GIO_create("/uart0",IOM_OUTPUT,&status,NULL,NULL);

	canOutHandle=GIO_create("/CAN",IOM_OUTPUT,&status,NULL,NULL);
	canInHandle=GIO_create("/CAN",IOM_INPUT,&status,NULL,NULL);

	isaOutHandle=GIO_create("/ISA",IOM_OUTPUT,&status,NULL,NULL);
	isaInHandle=GIO_create("/ISA",IOM_INPUT,&status,NULL,NULL);
	if(isaOutHandle==NULL)
	{

	}
}



//---------------------------------------------------------------------------
// Example: DisableDog:
//---------------------------------------------------------------------------
// This function disables the watchdog timer.

void DisableDog(void)
{
    EALLOW;
    SysCtrlRegs.WDCR= 0x0068;
    EDIS;
}

void InitPeripheralClocks(void)
{
   EALLOW;
// HISPCP/LOSPCP prescale register settings, normally it will be set to default values
   SysCtrlRegs.HISPCP.all = 0x0001;
   SysCtrlRegs.LOSPCP.all = 0x0002;

// Peripheral clock enables set for the selected peripherals.
   SysCtrlRegs.PCLKCR.bit.EVAENCLK=1;
   SysCtrlRegs.PCLKCR.bit.EVBENCLK=1;
   SysCtrlRegs.PCLKCR.bit.SCIAENCLK=1;
   SysCtrlRegs.PCLKCR.bit.SCIBENCLK=1;
   SysCtrlRegs.PCLKCR.bit.MCBSPENCLK=1;
   SysCtrlRegs.PCLKCR.bit.SPIENCLK=1;
   SysCtrlRegs.PCLKCR.bit.ECANENCLK=1;
   SysCtrlRegs.PCLKCR.bit.ADCENCLK=1;
   EDIS;
}


char uartFrame[64];

void task0()
{
	char data[32];
	size_t size=32;
	int status;
	int frameIndex=0;
	char metHeader=0;
	int i,j;
	while(TRUE)
	{
		status=GIO_read(gioHandle,(Ptr)data,&size);

		if(status==IOM_COMPLETED)
		{
			for(i=0;i<size;i++)
			{
				if(data[i]==0x10)
				{
					if(metHeader)
					{
						metHeader=0;
						continue;
					}
					metHeader=1;
					uartFrame[frameIndex]=0x10;
				}
				else if(data[i]==0x02)
				{
					if(metHeader)
					{
						metHeader=0;
						uartFrame[1]=0x02;
						frameIndex=2;
						continue;
					}
					else
					{
						uartFrame[frameIndex]=data[i];
					}
				}
				else if(data[i]==0x03)
				{
					if(metHeader)
					{

						if(frameIndex-3<=32)
						{
							char sum=0;
							for(j=0;j<frameIndex-4;j++)
							{
								sum+=uartFrame[2+j];
							}
							sum=sum&0xff;
							if(sum==uartFrame[frameIndex-2])
							{
								MBX_post(&MBX0,uartFrame+2,SYS_FOREVER);
							}
						}
						//uartFrame[frameIndex]=0x03;
						//catch a frame
						metHeader=0;
						frameIndex=0;
						continue;
					}
					else
					{
						uartFrame[frameIndex]=0x03;
					}
				}
				else
				{
					uartFrame[frameIndex]=data[i];
				}
				if(frameIndex>=2)
				{
					frameIndex++;
				}
				if(frameIndex>=sizeof(uartFrame))
				{
					frameIndex=0;
					metHeader=0;
				}
			}
		}
	}
}

	Char CANRxData[16*10];

void task1()
{


	size_t size=16*10;
    int i;
	int cnt;
	while(TRUE)
	{
		//TSK_sleep(10);
		size=16*10;
		GIO_read(canInHandle,CANRxData,&size);
		cnt=size/16;
		if(cnt>0)
		{
			for(i=0;i<cnt;i++)
			{
					MBX_post(&MBX2,CANRxData+16*i,SYS_FOREVER);
			}
		}
	
	}
}

void task2()
{
	Uint16 packet[16];
	Uint32 canData[4];
	size_t size=32;
	int  i;
	
	while(TRUE)
	{
//		canData[0]=0x80001234;
//		canData[1]=0x8;
//		canData[2]=0x11111111;
//		canData[3]=0x22222222;
//		size=32;
//		GIO_write(canOutHandle,canData,&size);
//		continue;
		MBX_pend(&MBX0,packet,SYS_FOREVER);
		if((packet[0]&0x80)==0x80)
		{
			switch(packet[0]&0x07)
			{
				case 1:
					
					for(i=0;i<4;i++)
					{
						canData[i]=(Uint32)packet[i*2+1]+((Uint32)packet[i*2+2]<<16);
					}
//					canData[0]=0x80001234;
//					canData[1]=0x8;
//					canData[2]=0x11111111;
//					canData[3]=0x22222222;
					size=32;
					GIO_write(canOutHandle,canData,&size);
					break;
				case 2:break;
			}
		}
		

	}
}



void task3()
{
	Uint32 p[16];
	char tmp[64];
	char sum,val;
	int i,j,index;
	size_t size;
	while(TRUE)
	{
		MBX_pend(&MBX1,p,SYS_FOREVER);

		sum=17+1; //default length=17Bytes
		for(i=0;i<4;i++)
		{
			for(j=0;j<4;j++)
			{
				sum+=((p[i]>>(j*8))&0xff);
			}
		}
		sum&=0xff;

		tmp[0]=0x10;
		tmp[1]=0x02;
		tmp[2]=17;
		tmp[3]=1;
		index=4;
		for(i=0;i<4;i++)
		{
			for(j=0;j<4;j++)
			{
				val=((p[i]>>(j*8))&0xff);
				tmp[index++]=val;
				if(val==0x10)
				{
					tmp[index++]=0x10;
				}
			}
		}
		//checksum
		tmp[index++]=sum;
		tmp[index++]=0x10;
		tmp[index++]=0x03;
		//size=index;
		size=4;
		GIO_write(gioOut,tmp+4,&size);
	}
}

void task4()
{
	Uint32 data[8];
	size_t size=16;
	Uint16 isaData[16];
	int i;
	while(TRUE)
	{
		MBX_pend(&MBX2,data,SYS_FOREVER);
		isaData[0]=(1<<8)|16;
		for(i=0;i<4;i++)
		{
			isaData[i*2+1]=(Uint16)(data[i]&0xffff);
			isaData[i*2+2]=(Uint16)((data[i]>>16)&0xffff);
		}
		GIO_write(isaOutHandle,isaData,&size);
	}
}

Uint16 packet[16];

void task5()
{

	size_t size;
	while(TRUE)
	{	
		size=16;
		GIO_read(isaInHandle,packet,&size);
		if(size>0)
		{
			MBX_post(&MBX0,packet,SYS_FOREVER);
		}
	}
}








