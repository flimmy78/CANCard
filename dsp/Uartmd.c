
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

/*
 * There is one UartChanObj per direction.  This mini-driver must be
 * opened for input and output separately (does not support IOM_INOUT).
 */
typedef struct UartChanObj {
    Uns                 inUse;          /* TRUE if channel is in use */
    Int                 mode;           /* INPUT or OUTPUT */

    struct UartPortObj  *port;          /* to support multiple UART ports */

    IOM_Packet          *dataPacket;    /* current active I/O packet */
    Char                *bufptr;        /* pointer within current buf */
    Uns                 bufcnt;         /* size of remaining I/O job */

    QUE_Obj             pendList;       /* IOM_Packets pending I/O go here */

    CIRC_Obj            circ;           /* circular buffer */

    IOM_TiomCallback    cbFxn;          /* to notify client when I/O complete */
    Ptr                 cbArg;          /* argument for cbFxn() */



} UartChanObj, *UartChanHandle;

/*
 * There is one UartPortObj per UART.
 * This mini-driver supports 'NUMPORTS' UART.
 */
typedef struct UartPortObj {
    //UARTHW_Handle               hUart;
    //UARTMD_TnotifyHandler       notifyFunc;
    Uns                         evtMask;
    UartChanObj                 chans[NUMCHANS];
} UartPortObj, *UartPortHandle;



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
//static Int  controlNotify(UartChanHandle chan, UARTMD_NotifyStruct* notify);


/*
 * Submit functions.  These functions are called by mdSubmitChan() for
 * assorted submit commands.
 */
static Int  submitAbort(UartChanHandle chan, IOM_Packet *packet);
static Int  submitFlush(UartChanHandle chan, IOM_Packet *packet);
static Int  submitRead(UartChanHandle chan, IOM_Packet *packet);
static Int  submitWrite(UartChanHandle chan, IOM_Packet *packet);



/* This mini-driver supports 'NUMPORTS' UARTs. */

#define NUMPORTS        1       /* 2 ports for dual UART on EVMDM642 board */

static UartPortObj  ports[NUMPORTS];

static Char UARTRxBuf[32];
static Char UARTTxBuf[32];

static void SciaTxIsr();

static void SciaRxIsr();

void InitScia();


static Void cbRxHandler(UartPortHandle port, Char c);
static Void cbTxHandler(UartPortHandle port,Uint16 len);
/*
 * Support functions.
 */
static Void getNextPacket(UartChanHandle chan);
/*
 * Public Mini Driver interface table.
 */
IOM_Fxns UARTMD_FXNS =
{
    &mdBindDev,
    IOM_UNBINDDEVNOTIMPL,
    &mdControlChan,
    &mdCreateChan,
    &mdDeleteChan,
    &mdSubmitChan,
};

/*
 *  ======== UARTMD_init ========
 *  UARTMD_init() initializes the data structures used by this mini-driver.
 */
Void UARTMD_init(Void)
{
    Int i, j;

    /* initialize all uartPortObj fields to '0' */
    memset(ports, 0, sizeof(ports));

    for (i=0; i < NUMPORTS; i++) {
        for (j=0; j < NUMCHANS; j++) {
            /* initialize port->chans */
            QUE_new(&ports[i].chans[j].pendList);
            CIRC_new(&ports[i].chans[j].circ,j==0?UARTRxBuf:UARTTxBuf,32);
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
    //Initialize UART



    //hUart = UARTHW_open(devid, params->uarthwParams, &ports[devid], cbFxns);


    EALLOW;

    SysCtrlRegs.PCLKCR.bit.SCIAENCLK=1;
    SysCtrlRegs.PCLKCR.bit.SCIBENCLK=1;

    EDIS;


    InitScia();

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
    UartChanHandle chan = (UartChanHandle)chanp;
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
    UartChanHandle      chan;
    UartPortHandle      port = (UartPortHandle)devp;

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
    UartChanHandle chan = (UartChanHandle)chanp;

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
static Int submitAbort(UartChanHandle chan, IOM_Packet *packet)
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
static Int submitFlush(UartChanHandle chan, IOM_Packet *packet)
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
static Int submitRead(UartChanHandle chan, IOM_Packet *packet)
{
    CIRC_Handle circ = &chan->circ;
    //UARTHW_Handle       hUart = chan->port->hUart;
    Uns         imask;

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
        *chan->bufptr++ = CIRC_readChar(circ);
        chan->bufcnt--;
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
static Int submitWrite(UartChanHandle chan, IOM_Packet *packet)
{
    Int                 status;
    Uns                 imask;
    CIRC_Handle         circ = &chan->circ;
    //UARTHW_Handle       hUart = chan->port->hUart;

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

    for (;;) {
        /*
         * 'chan->bufcnt' will reach 0 if we were able to copy all characters
         * to the circular buffer.  Return IOM_COMPLETED in this case.
         */
        if (chan->bufcnt == 0) {
            status = IOM_COMPLETED;
            break;
        }

        /*
         * Set 'chan->dataPacket' and return IOM_PENDING if there is
         * no more room for characters in the the circular buffer.  The
         * txIsr will copy the remaining characters to the circular buffer
         * as room becomes available.
         */
        if (CIRC_emptyCount(circ) == 0) {
            chan->dataPacket = packet;
            status = IOM_PENDING;
            break;
        }

        /*
         * Put character into circular buffer and decrement count.
         */
        CIRC_writeChar(circ, *chan->bufptr++);
        chan->bufcnt--;

        /* Open the window for interrupt(s) */

        HWI_restore(imask);
        imask = HWI_disable();
    }

    /*
     * Interrupts must be disabled here since UART may have only
     * one ISR for both rx and tx.  An rx ISR may call cbTxHandler()
     * and cbTxHandler() is not reentrant.
     */
//    if (UARTHW_txEmpty(hUart)) {
//        cbTxHandler(chan->port);
//    }

    SciaRegs.SCIFFTX.bit.TXFFIENA=1;// //enable Tx Interrupt

    HWI_restore(imask);

    return (status);
}



void scia_fifo_init()
{
   SciaRegs.SCICCR.all =0x0007;   // 1 stop bit,  No loopback
                                  // No parity,8 char bits,
                                  // async mode, idle-line protocol
   SciaRegs.SCICTL1.all =0x0003;  // enable TX, RX, internal SCICLK,
                                  // Disable RX ERR, SLEEP, TXWAKE
   SciaRegs.SCICTL2.bit.TXINTENA =1; //
   SciaRegs.SCICTL2.bit.RXBKINTENA =1;
   SciaRegs.SCIHBAUD = 0x0000;
   SciaRegs.SCILBAUD = 40;//115200
   SciaRegs.SCICCR.bit.LOOPBKENA =0; // Disable loop back
   SciaRegs.SCIFFTX.all=0xE048;
   SciaRegs.SCIFFRX.all=0x0028;
   SciaRegs.SCIFFCT.all=0x00;

   SciaRegs.SCICTL1.all =0x0023;     // Relinquish SCI from Reset
   SciaRegs.SCIFFTX.bit.TXFIFOXRESET=1;
   SciaRegs.SCIFFRX.bit.RXFIFORESET=1;


}

void InitScia()
{
	// Step 2. Initalize GPIO:
	// This example function is found in the DSP281x_Gpio.c file and
	// illustrates how to set the GPIO to it's default state.
	// InitGpio();
	// Setup only the GP I/O only for SCI-A and SCI-B functionality
	   EALLOW;
	   GpioMuxRegs.GPFMUX.bit.SCITXDA_GPIOF4 = 1;
	   GpioMuxRegs.GPFMUX.bit.SCIRXDA_GPIOF5 = 1;
	   GpioMuxRegs.GPGMUX.bit.SCITXDB_GPIOG4 = 1;
	   GpioMuxRegs.GPGMUX.bit.SCIRXDB_GPIOG5 = 1;
	   EDIS;


	   HWI_dispatchPlug(96,(Fxn)SciaRxIsr,NULL);
	   HWI_dispatchPlug(97,(Fxn)SciaTxIsr,NULL);

	   scia_fifo_init();

	   // Enable interrupts required for this example
	      PieCtrlRegs.PIECRTL.bit.ENPIE = 1;   // Enable the PIE block
	      PieCtrlRegs.PIEIER9.bit.INTx1=1;     // PIE Group 9, INT1
	      PieCtrlRegs.PIEIER9.bit.INTx2=1;     // PIE Group 9, INT2
	      //PieCtrlRegs.PIEIER9.bit.INTx3=1;     // PIE Group 9, INT3
	      //PieCtrlRegs.PIEIER9.bit.INTx4=1;     // PIE Group 9, INT4
	      IER |= 0x100;	// Enable CPU INT
	      EINT;

	      PRD_start(&PRD0);
}


/*
 * ISR for Scia Receive Interrupt
 */
void SciaRxIsr()
{
	Uint16 data=0;
	int i;
	Uint16 rxBytes=SciaRegs.SCIFFRX.bit.RXFIFST;
	for(i=0;i<rxBytes;i++)
	{
		data=SciaRegs.SCIRXBUF.all;
		cbRxHandler(&(ports[0]),(Char)data);
	}
	SciaRegs.SCIFFRX.bit.RXFFINTCLR=1;
	PieCtrlRegs.PIEACK.bit.ACK9=1;

	PRD_start(&PRD0);

}

/*
 * ISR for Scia Transmit Interrupt
 */
 static void SciaTxIsr()
{
	int i=0;
	Uint16 leftBytes= 16-SciaRegs.SCIFFTX.bit.TXFFST;

	cbTxHandler(&(ports[0]),leftBytes);
	SciaRegs.SCIFFTX.bit.TXINTCLR=1;
	PieCtrlRegs.PIEACK.bit.ACK9=1;
}


 void prd0()
 {

	 UartChanHandle      chan = &(ports[0].chans[INPUT]);
	 Uint16 rxBytes=SciaRegs.SCIFFRX.bit.RXFIFST;
	 Uint16 data;
	 int i;

	 for(i=0;i<rxBytes;i++)
	 {
	 	data=SciaRegs.SCIRXBUF.all;
	 	cbRxHandler(&(ports[0]),(Char)data);
	 }
	 if((chan->dataPacket!=NULL)&&(chan->bufcnt <chan->dataPacket->size))
	 {
		 chan->dataPacket->size-=chan->bufcnt;
		 getNextPacket(chan);
	 }


	 PRD_start(&PRD0);
 }



/*
 *  ======== cbRxHandler ========
 *  The interrupt handler routine for a receive data ready.
 */
static Void cbRxHandler(UartPortHandle port, Char c)
{
    UartChanHandle      chan = &port->chans[INPUT];
    CIRC_Handle         circ = &chan->circ;


    if (chan->dataPacket) {
        *chan->bufptr++ = (Char)c;
        chan->bufcnt--;

        if (chan->bufcnt == 0) {
            getNextPacket(chan);
        }
    }
    else if (CIRC_emptyCount(circ)) {
        CIRC_writeChar(circ, (Char)c);
    }
    else {
        /* SYS_abort("cbRxHandler: Rx overrun"); */
    }
    if (CIRC_isFull(circ)) {
        //UARTHW_disableRx(hUart);
    }
}


/*
 *  ======== cbTxHandler ========
 *  The interrupt handler routine for a transmit buffer empty.
 *  This function is also called from submitWrite() to get the data flowing.
 */
static Void cbTxHandler(UartPortHandle port,Uint16 len)
{
    Char                c;
    UartChanHandle      chan = &port->chans[OUTPUT];
    CIRC_Handle         circ = &chan->circ;


    int i=0;
    for(i=0;i<len;i++)
    {
    	if (CIRC_fullCount(circ) > 0)
    	{
    		c = CIRC_readChar(circ);
    		//UARTHW_writeChar(hUart, c);
    		SciaRegs.SCITXBUF=c;
    	}
    	 if (chan->dataPacket)
    	 {
    		 CIRC_writeChar(circ, *chan->bufptr++);
    		 chan->bufcnt--;
    		 if (chan->bufcnt == 0)
    		 {
    			 getNextPacket(chan);
    		 }
    	 }
    	 else
    	 {

    		 break;
    	 }
    }

    if(CIRC_fullCount(circ)==0)
    {
    	 SciaRegs.SCIFFTX.bit.TXFFIENA=0;//
    }




}


/*
 *  -------- support functions --------
 */

/*
 *  ======== getNextPacket ========
 */
static Void getNextPacket(UartChanHandle chan)
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




