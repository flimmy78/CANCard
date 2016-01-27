
/*
 *  Copyright 2003 by Texas Instruments Incorporated.
 *  All rights reserved. Property of Texas Instruments Incorporated.
 *  Restricted rights to use, duplicate or disclose this code are
 *  granted through contract.
 *
 */
/* "@(#) DDK 1.10.00.23 07-02-03 (ddk-b12)" */
/*
 *  ======== uartmd.c ========
 */

#include <std.h>
#include <stdlib.h>
#include <string.h>

#include <atm.h>
#include <hwi.h>
#include <iom.h>
#include <que.h>
#include <prd.h>

#include "circ.h"
#include "DSP281x_Device.h"
#include "CanCardcfg.h"

#define INPUT           0
#define OUTPUT          1
#define NUMCHANS        2


#define XINT1CR (*((volatile int*)0x7070))
#define XINT2CR (*((volatile int*)0x7071))

/*
 * There is one ISAChanObj per direction.  This mini-driver must be
 * opened for input and output separately (does not support IOM_INOUT).
 */
typedef struct ISAChanObj {
    Uns                 inUse;          /* TRUE if channel is in use */
    Int                 mode;           /* INPUT or OUTPUT */

    struct ISAPortObj  *port;          /* to support multiple UART ports */

    IOM_Packet          *dataPacket;    /* current active I/O packet */
    Char                *bufptr;        /* pointer within current buf */
    Uns                 bufcnt;         /* size of remaining I/O job */

    QUE_Obj             pendList;       /* IOM_Packets pending I/O go here */

    CIRC_Obj            circ;           /* circular buffer */

    IOM_TiomCallback    cbFxn;          /* to notify client when I/O complete */
    Ptr                 cbArg;          /* argument for cbFxn() */



} ISAChanObj, *ISAChanHandle;

/*
 * There is one ISAPortObj per UART.
 * This mini-driver supports 'NUMPORTS' UART.
 */
typedef struct ISAPortObj {
    //UARTHW_Handle               hUart;
    //UARTMD_TnotifyHandler       notifyFunc;
    Uns                         evtMask;
    ISAChanObj                 chans[NUMCHANS];
} ISAPortObj, *ISAPortHandle;



/*
 * Forward declaration of IOM mini driver interface functions.  These
 * are only exposed via the IOM function table to avoid namespace pollution.
 */
static Int mdBindDev(Ptr *devp, Int devid, Ptr bindParams);
static Int mdControlChan(Ptr chanp, Uns cmd, Ptr arg);
static Int mdCreateChan(Ptr *mdChan, Ptr drvhandle, String name, Int mode, \
        Ptr optArgs, IOM_TiomCallback cbFxn, Ptr cbArg);
static Int mdDeleteChan(Ptr chanp);
static Int mdSubmitChan(Ptr chanp, IOM_Packet *packet);


/*
 * Control functions.  These functions are called by mdControl() for
 * assorted control commmands.
 */
//static Int  controlNotify(ISAChanHandle chan, UARTMD_NotifyStruct* notify);


/*
 * Submit functions.  These functions are called by mdSubmitChan() for
 * assorted submit commands.
 */
static Int  submitAbort(ISAChanHandle chan, IOM_Packet *packet);
static Int  submitFlush(ISAChanHandle chan, IOM_Packet *packet);
static Int  submitRead(ISAChanHandle chan, IOM_Packet *packet);
static Int  submitWrite(ISAChanHandle chan, IOM_Packet *packet);



/* This mini-driver supports 'NUMPORTS' UARTs. */

#define NUMPORTS        1       /* 2 ports for dual UART on EVMDM642 board */

#define ISABUFFERSIZE (16*15)

static ISAPortObj  ports[NUMPORTS];

static  Char ISABuffer[2][ISABUFFERSIZE];

//#pragma CODE_SECTION(XINT1_ISR,"ramfuncs")
static void XINT1_ISR();

static void XINT2_ISR();

void InitISA();



/*
 * Support functions.
 */
static Void getNextPacket(ISAChanHandle chan);
/*
 * Public Mini Driver interface table.
 */
IOM_Fxns ISAMD_FXNS =
{
    &mdBindDev,
    IOM_UNBINDDEVNOTIMPL,
    &mdControlChan,
    &mdCreateChan,
    &mdDeleteChan,
    &mdSubmitChan,
};

static int FrameIndex=0;

/*
 *  ======== UARTMD_init ========
 *  UARTMD_init() initializes the data structures used by this mini-driver.
 */
Void ISAMD_init(Void)
{
    Int i, j;

    /* initialize all ISAPortObj fields to '0' */
    memset(ports, 0, sizeof(ports));

    for (i=0; i < NUMPORTS; i++) {
        for (j=0; j < NUMCHANS; j++) {
            /* initialize port->chans */
            QUE_new(&ports[i].chans[j].pendList);
            CIRC_new(&ports[i].chans[j].circ,ISABuffer[j],ISABUFFERSIZE);
            ports[i].chans[j].port = &ports[i];
        }
    }
}




/*
 *  ======== mdBindDev ========
 *  mdBindDev() is called by DEV_init() to bind and initialize the hardware.
 */
static Int mdBindDev(Ptr *devp, Int devid, Ptr devParams)
{


    if ((Uns)devid > NUMPORTS-1) {
        return (IOM_EBADARGS);
    }
    //Initialize CAN
    InitISA();
    //InitISA1();
    *devp = &ports[devid];
    return (IOM_COMPLETED);
}

/*
 *  ======== mdControlChan ========
 *  The Mini driver ctrl function. Catch all for adding device or vendor
 *  specific functionality to a mini driver.
 */
static Int mdControlChan(Ptr chanp, Uns cmd, Ptr arg)
{
    ISAChanHandle chan = (ISAChanHandle)chanp;
    //UARTHW_Handle hUart = chan->port->hUart;
    Int status;

//    if (cmd == UARTMD_REGISTER_NOTIFY) {
//        //status = controlNotify(chan, arg);
//    }
//    else if (cmd == UARTMD_SETBREAK) {
//        //status = UARTHW_setBreak(hUart, ArgToInt(arg));
//    }
//    else if (cmd == UARTMD_GETMODEMSTATUS) {
//        //status = UARTHW_getModemStatus(hUart, (char*)arg);
//    }
//    else if (cmd == UARTMD_SETRTS) {
//        //status = UARTHW_setRTS(hUart, ArgToInt(arg));
//    }
//    else if (cmd == UARTMD_SETDTR) {
//        //status = UARTHW_setDTR(hUart, ArgToInt(arg));
//    }
//    else {
//        status = IOM_ENOTIMPL;
//    }

    return (status);
}


/*
 *  ======== mdCreateChan ========
 */
static Int mdCreateChan(Ptr *chanp, Ptr devp, String name, Int mode, \
                         Ptr chanParams, IOM_TiomCallback cbFxn, Ptr cbArg)
{
    ISAChanHandle      chan;
    ISAPortHandle      port = (ISAPortHandle)devp;

    if (mode == IOM_INPUT) {
        chan = &port->chans[INPUT];
        chan->mode = INPUT;
    }
    else if (mode == IOM_OUTPUT) {
        chan = &port->chans[OUTPUT];
        chan->mode = OUTPUT;
    }
    else {
        return (IOM_EBADMODE);
    }

    if (ATM_setu(&chan->inUse, TRUE)) {
        return (IOM_EINUSE);
    }

    /*
     * Save the callback function and argument.  cbFxn() is called every
     * time an I/O job completes.
     */
    chan->cbFxn = cbFxn;
    chan->cbArg = cbArg;

    /* chanp will be passed to subsequent mini-driver calls */
    *chanp = chan;

    return (IOM_COMPLETED);
}

/*
 *  ======== mdDeleteChan ========
 *  Deletes an instance of the UART channel.
 *  All I/O jobs must be completed prior to calling mdDelete().
 */
static Int mdDeleteChan(Ptr chanp)
{
    ISAChanHandle chan = (ISAChanHandle)chanp;

    chan->inUse = FALSE;

    return (IOM_COMPLETED);
}


/*
 *  ======== mdSubmitChan ========
 *  The main entry point to the mini driver for read / write operations.
 *  mdSubmitChan() also handles the flush and abort operations.
 */
static Int mdSubmitChan(Ptr chanp, IOM_Packet *packet)
{
    Uns cmd = (packet->cmd);
    Int status;

    if (cmd == IOM_READ){
        status = submitRead(chanp, packet);
    }
    else if (cmd == IOM_WRITE){
        status = submitWrite(chanp, packet);
    }
    else if (cmd == IOM_ABORT){
        status = submitAbort(chanp, packet);
    }
    else if (cmd == IOM_FLUSH){
        status = submitFlush(chanp, packet);
    }
    else {
        status = IOM_ENOTIMPL;
    }

    return (status);
}



/*
 *  -------- submit functions --------
 */

/*
 *  ======== submitAbort ========
 *  The local routine to handle an IOM_ABORT command.
 */
static Int submitAbort(ISAChanHandle chan, IOM_Packet *packet)
{
    IOM_Packet  *dataPacket;
    Uns         imask;
    CIRC_Handle circ = &chan->circ;

    /*
     * Atomically save dataPacket and set chan->dataPacket to NULL.
     * 'chan->dataPacket' is used to synchronize with the ISR.  If the
     * ISR sees chan->dataPacket == NULL, it will not attempt to
     * reload any packets from the pendList.
     */
    imask = HWI_disable();
    CIRC_reset(circ);
    dataPacket = chan->dataPacket;
    chan->dataPacket = NULL;            /* stop all active I/O */


    HWI_restore(imask);

    /*
     * Return all packets in order with their status tagged as aborted.
     * Since chan->dataPacket was set to NULL above, we don't need to
     * worry about synchronizing with the ISR.
     */

    /*
     * Return active dataPacket first.
     */
    if (dataPacket != NULL) {
        dataPacket->status = IOM_ABORTED;
        chan->cbFxn(chan->cbArg, dataPacket);
    }

    /*
     * Now remove remaining packets from the pending list and call
     * the callback one at a time.  We use QUE_get() here for code size
     * savings, but we could use QUE_dequeue() since ISR will not
     * reference this queue.
     */
    dataPacket = QUE_get(&chan->pendList);
    while (dataPacket != (IOM_Packet *)&chan->pendList) {
        dataPacket->status = IOM_ABORTED;
        chan->cbFxn(chan->cbArg, dataPacket);

        dataPacket = QUE_get(&chan->pendList);
    }

    packet->status = IOM_COMPLETED;
    return (IOM_COMPLETED);
}

/*
 *  ======== submitFlush ========
 *  The local routine to handle an IOM_FLUSH command.
 */
static Int submitFlush(ISAChanHandle chan, IOM_Packet *packet)
{
    Uns         imask;
    Int         status;

    /*
     * Abort the current read operation.
     * Wait for all output operations to complete in order.
     */

    /*
     * Abort the current read operations
     */
    if (chan->mode == IOM_INPUT) {
        /* abort and flush are equivalent for input channels */
        return (submitAbort(chan, packet));
    }
    else {
        imask = HWI_disable();

        /*
         * If output is in process, add 'flush' packet to pendList.
         * txIsr will handle flush packet when all output is complete.
         */
        if (chan->dataPacket) {
            packet->status = IOM_PENDING;
            QUE_enqueue(&chan->pendList, packet);
            status = IOM_PENDING;
        }
        else {
            status = IOM_COMPLETED;
        }

        HWI_restore(imask);
    }

    return (status);
}


/*
 *  ======== submitRead ========
 *  The local routine to handle application reads.
 */
static Int submitRead(ISAChanHandle chan, IOM_Packet *packet)
{
    CIRC_Handle circ = &chan->circ;
    //UARTHW_Handle       hUart = chan->port->hUart;
    Uns         imask;
    int i;
    /*
     * Disable interrupts since several of these variables are
     * shared with the ISR.
     */
    imask = HWI_disable();

    /*
     * If chan->dataPacket is non-NULL, then there is already a packet
     * in process.  Simply enqueue the new packet and return IOM_PENDING.
     */
    if (chan->dataPacket) {
        QUE_enqueue(&chan->pendList, packet);

        HWI_restore(imask);
        return (IOM_PENDING);
    }

    /*
     * Update local bufptr and bufcnt variables from new packet.  We
     * don't set 'chan->dataPacket' until we are sure that no other
     * characters exist in the circular buffer.  The ISR will then
     * put characters directly in 'chan->dataPacket'.
     */
    chan->bufptr = packet->addr;
    chan->bufcnt = packet->size;

    /*
     * This loop will copy characters from circular buffer one at a
     * time until the read is satisfied or there are no more characters
     * in the circular buffer.   Interrupts are disabled while this
     * loop executes, but window is opened at bottom of loop to minimize
     * interrupt latency.
     */
    for (;;) {
        /*
         * If enough characters were available in the circular buffer
         * to satisfy read then return IOM_COMPLETED.
         */
        if (chan->bufcnt == 0) {
            HWI_restore(imask);
            return (IOM_COMPLETED);
        }

        /*
         * If no more characters exist in the CIRC, set chan->dataPacket
         * and return IOM_PENDING.  Remember that chan->dataPacket is
         * used to synchronize with the ISR.
         */
        if (CIRC_fullCount(circ) == 0) {
            chan->dataPacket = packet;
            HWI_restore(imask);
            return (IOM_PENDING);
        }

        /*
         * Read a character from the circular buffer and decrement the count.
         */
        for(i=0;i<16;i++)
        {
        	CIRC_readChar(circ);
			*chan->bufptr++ = CIRC_readChar(circ);
			chan->bufcnt--;
        }
        //UARTHW_enableRx(hUart);

        /* open window for interrupt(s) */
        HWI_restore(imask);
        imask = HWI_disable();
    }
}



/*
 *  ======== submitWrite ========
 *  The local routine to handle application writes.
 */
static Int submitWrite(ISAChanHandle chan, IOM_Packet *packet)
{
	Uint16 *val;
    Int                 status;
    Uns                 imask;
    CIRC_Handle         circ = &chan->circ;
    //UARTHW_Handle       hUart = chan->port->hUart;

	
    Uint32 tmp;
    Uint16 * p=(Uint16*)0x9fc00;
    int i;
    
    imask = HWI_disable();


    /*
     * If there's a packet in progress, 'chan->dataPacket' will be non-NULL.
     * Just queue up the new job and return IOM_PENDING.
     */
    if (chan->dataPacket) {
        QUE_enqueue(&chan->pendList, packet);


        HWI_restore(imask);

        return (IOM_PENDING);
    }

    /*
     * Update local bufptr and bufcnt variables from new packet.  We
     * don't set 'chan->dataPacket' until we are sure that there's no
     * more room in the circular buffer.  Unlike the rxIsr, the txIsr
     * always takes characters from the circular buffer.
     */
    chan->bufptr = packet->addr;
    chan->bufcnt = packet->size;

    chan->dataPacket = packet;
    status = IOM_PENDING;
    val=packet->addr;
	
	if((XintfRegs.XINTCNF2.bit.HOLDAS==1)&&(FrameIndex<64))
	{
		for(i=0;i<16;i++)
		{
			p[FrameIndex*16+i]=val[i];
		}
		p[FrameIndex*16+16]=0xffff;
		if(FrameIndex==63)
		{
			GpioDataRegs.GPACLEAR.bit.GPIOA15    =1;
			while(XintfRegs.XINTCNF2.bit.HOLDAS==1);
			
			//FrameIndex=0;
		}
		else
		{
		
			getNextPacket(chan);
			//status = IOM_COMPLETED;
			//chan->cbFxn(chan->cbArg, chan->dataPacket);
			
		
		    PRD_start(&PRD1);
			
		}
		FrameIndex++;
	}
    HWI_restore(imask);

    return (status);
}



void InitISA()
{

	Uint16 * p=(Uint16*)0x9fc00;
    int i;
	EALLOW;
	
	XINT1CR =0x1;/*打开管脚xint1外中断*/
	XINT2CR =0x1;/*打开管脚xint2外中断*/
	
	//config GPIOA.15 to output pin
	GpioMuxRegs.GPAMUX.bit.C3TRIP_GPIOA15=0;
	GpioMuxRegs.GPADIR.bit.GPIOA15=1;
	
	EDIS;

	GpioDataRegs.GPASET.bit.GPIOA15    =1;

    while(XintfRegs.XINTCNF2.bit.HOLDAS==0);
    
	for(i=0;i<16;i++)
	{
		p[i]=0xffff;
	}
	/*Configure Pie Interrupts*/
	HWI_dispatchPlug(35,(Fxn)XINT1_ISR,NULL);
	HWI_dispatchPlug(36,(Fxn)XINT2_ISR,NULL);

	

	/* Configure PIE interrupts */
	
	PieCtrlRegs.PIECRTL.bit.ENPIE = 1;  // Enable vector fetching from PIE block
	PieCtrlRegs.PIEACK.bit.ACK1 = 1;    // Enables PIE to drive a pulse into the CPU
	// The ’Write–denied’ interrupt can be asserted in either of the eCAN interrupt lines
	// Comment out the unwanted line...
	PieCtrlRegs.PIEIER1.bit.INTx4 = 1;  // Enable INTx.5 of INT9 (XINT1INT)
	PieCtrlRegs.PIEIER1.bit.INTx5 = 1;  // Enable INTx.6 of INT9 (XINT2INT)
	/* Configure system interrupts */
	IER |= 0x0001; // Enable INT1 of CPU
	EINT; // Global enable of interrupts
}



/*
 * ISR for Scia Receive Interrupt
 */
void XINT1_ISR()
{

	Uint16 * p=(Uint16*)0x9fc00;
	Uint16* frame;
	ISAChanHandle chan;
	int i,j;
	int tag;
	int sndCnt;
	Char tChar;
	if(XintfRegs.XINTCNF2.bit.HOLDAS==1)  //
	{
		PieCtrlRegs.PIEACK.bit.ACK1=1;
		return;
	}
	//Pull up XHOLD ;
	GpioDataRegs.GPASET.bit.GPIOA15    =1;
	//wait for XHOLDA to 0
	while(XintfRegs.XINTCNF2.bit.HOLDAS==0);
	
	
	
	FrameIndex=0;
	//Receive Data First
	
	chan = &(ports[0].chans[INPUT]);
	
	for(i=0;i<64;i++)
	{
	  	tag=p[i*16];
		if((tag&0x80)!=0x80) break;
		if(((tag>>8)&0xff)>32) break;
		
		if(chan->dataPacket==NULL)
		{
			CIRC_Handle         circ = &chan->circ;
			for(j=0;j<16;j++)
			{
				//fixed 32Bytes Size
				if (CIRC_emptyCount(circ)) 
				{
					CIRC_writeChar(circ, (Char)p[i*16+j]);
				}
			}
		}
		else
		{
			Char* addr=chan->bufptr;
			for(j=0;j<16;j++)
			{
			 //fixed 32Bytes Size
				tChar=(Char)p[i*16+j];
				addr[j]=tChar;
			}
			chan->bufcnt-=16;
			chan->bufptr+=16;


			if ((chan->bufcnt)>>4 == 0)
        	{
            	getNextPacket(chan);
           		
        	}	
		}
		p[i*16]=0xffff; //Data has read	
	}
	
	//Start Transmit Data
	chan = &(ports[0].chans[OUTPUT]);
	sndCnt=0;
	if(chan->dataPacket!=NULL)
	{
		getNextPacket(chan);
	
		for(i=0;i<64;i++)
		{
			if(chan->dataPacket==NULL) break;
			
			frame=chan->dataPacket->addr;
			for(j=0;j<16;j++)
			{
				p[16*i+j]=frame[j];
			}
			sndCnt++;
			
			getNextPacket(chan);
		}
		if(sndCnt>0)
		{
			tag=0;
			GpioDataRegs.GPACLEAR.bit.GPIOA15    =1; //
			while(XintfRegs.XINTCNF2.bit.HOLDAS==1);
		}
		
	}
	
	if((tag==0xff80))
	{
		PRD_start(&PRD1);
	}
	
	
	
	PieCtrlRegs.PIEACK.bit.ACK1=1;


}

/*
 * ISR for Scia Transmit Interrupt
 */
 static void XINT2_ISR()
{
	
	PRD_start(&PRD1);
	PieCtrlRegs.PIEACK.bit.ACK1=1;
}



/*
 *  -------- support functions --------
 */

/*
 *  ======== getNextPacket ========
 */
static Void getNextPacket(ISAChanHandle chan)
{
    IOM_Packet  *donePacket, *nextPacket, *flushPacket;

    /* Intialize flushPacket */
    flushPacket = NULL;

    /* Save complete packet for callback */
    donePacket = chan->dataPacket;

    /* get next data packet */
    nextPacket = QUE_get(&chan->pendList);

    /*
     * If nextPacket points to head of the queue, then the list is
     * empty so there's no pending I/O packets.
     */
    if (nextPacket != (IOM_Packet *)&chan->pendList) {
        /* Check if packet is a FLUSH packet */
        if (nextPacket->cmd != IOM_FLUSH) {
            /* Set address and size of next packet */
            chan->bufptr = nextPacket->addr;
            chan->bufcnt = nextPacket->size;
            chan->dataPacket = nextPacket;
        }
        else {
            /* Set flushPacket and clear dataPacket */
            flushPacket = nextPacket;
            chan->dataPacket = NULL;
        }
    }
    else {
        chan->dataPacket = NULL;
    }

    /* Call the callback for the completed packet */
    donePacket->status = IOM_COMPLETED;
    //donePacket->size-=chan->bufcnt;
    chan->cbFxn(chan->cbArg, donePacket);

    /*
     * Call the callback for flushPacket *after* the callback for
     * the completed packet.
     */
    if (flushPacket != NULL) {
        flushPacket->status = IOM_COMPLETED;
        chan->cbFxn(chan->cbArg, flushPacket);
    }
}


void IsaPrd()
{
	int i=0;
	 //imask = HWI_disable();
	 if(XintfRegs.XINTCNF2.bit.HOLDAS==0)
	 {
	 	GpioDataRegs.GPASET.bit.GPIOA15    =1;
	 	while(XintfRegs.XINTCNF2.bit.HOLDAS==0);
	 }
	 for(i=0;i<100;i++);
	 GpioDataRegs.GPACLEAR.bit.GPIOA15    =1;
	 while(XintfRegs.XINTCNF2.bit.HOLDAS==1);
	    //HWI_restore(imask);
}






