
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


#define CANRXSIZE  	(16*32)
/*
 * There is one CanChanObj per direction.  This mini-driver must be
 * opened for input and output separately (does not support IOM_INOUT).
 */
typedef struct CanChanObj {
    Uns                 inUse;          /* TRUE if channel is in use */
    Int                 mode;           /* INPUT or OUTPUT */

    struct CanPortObj  *port;          /* to support multiple UART ports */

    IOM_Packet          *dataPacket;    /* current active I/O packet */
    Char                *bufptr;        /* pointer within current buf */
    Uns                 bufcnt;         /* size of remaining I/O job */

    QUE_Obj             pendList;       /* IOM_Packets pending I/O go here */

    CIRC_Obj            circ;           /* circular buffer */

    IOM_TiomCallback    cbFxn;          /* to notify client when I/O complete */
    Ptr                 cbArg;          /* argument for cbFxn() */



} CanChanObj, *CanChanHandle;

/*
 * There is one CanPortObj per UART.
 * This mini-driver supports 'NUMPORTS' UART.
 */
typedef struct CanPortObj {
    //UARTHW_Handle               hUart;
    //UARTMD_TnotifyHandler       notifyFunc;
    Uns                         evtMask;
    CanChanObj                 chans[NUMCHANS];
} CanPortObj, *CanPortHandle;



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
//static Int  controlNotify(CanChanHandle chan, UARTMD_NotifyStruct* notify);


/*
 * Submit functions.  These functions are called by mdSubmitChan() for
 * assorted submit commands.
 */
static Int  submitAbort(CanChanHandle chan, IOM_Packet *packet);
static Int  submitFlush(CanChanHandle chan, IOM_Packet *packet);
static Int  submitRead(CanChanHandle chan, IOM_Packet *packet);
static Int  submitWrite(CanChanHandle chan, IOM_Packet *packet);



/* This mini-driver supports 'NUMPORTS' UARTs. */

#define NUMPORTS        1       /* 2 ports for dual UART on EVMDM642 board */

static CanPortObj  ports[NUMPORTS];

static Char CANBufferTX[32];
static Char CANBufferRX[CANRXSIZE];

//#pragma CODE_SECTION(can0Isr,"ramfuncs")
 void can0Isr();

static void can1Isr();

void InitECan();
void InitECan1();

static Void cbRxHandler(CanPortHandle port, Char c);
static Void cbTxHandler(CanPortHandle port,Uint16 len);
/*
 * Support functions.
 */
static Void getNextPacket(CanChanHandle chan);
/*
 * Public Mini Driver interface table.
 */
IOM_Fxns CANMD_FXNS =
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
Void CANMD_init(Void)
{
    Int i, j;

    /* initialize all CanPortObj fields to '0' */
    memset(ports, 0, sizeof(ports));

    for (i=0; i < NUMPORTS; i++) {
        for (j=0; j < NUMCHANS; j++) {
            /* initialize port->chans */
            QUE_new(&ports[i].chans[j].pendList);
            CIRC_new(&ports[i].chans[j].circ,j==0?CANBufferRX:CANBufferTX,j==0?CANRXSIZE:32);
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
    InitECan();
    InitECan1();
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
    CanChanHandle chan = (CanChanHandle)chanp;
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
    CanChanHandle      chan;
    CanPortHandle      port = (CanPortHandle)devp;

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
    CanChanHandle chan = (CanChanHandle)chanp;

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
static Int submitAbort(CanChanHandle chan, IOM_Packet *packet)
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
static Int submitFlush(CanChanHandle chan, IOM_Packet *packet)
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
static Int submitRead(CanChanHandle chan, IOM_Packet *packet)
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
        //if ((packet->size)-(chan->bufcnt) == 16) {
        if (chan->bufcnt == 0) {
        	//packet->size=32;
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
            PRD_start(&PRD2);
            HWI_restore(imask);
            return (IOM_PENDING);
        }

        /*
         * Read a character from the circular buffer and decrement the count.
         */
         for(i=0;i<16;i++)
         {
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
static Int submitWrite(CanChanHandle chan, IOM_Packet *packet)
{
	Uint32 *val;
    Int                 status;
    Uns                 imask;
    CIRC_Handle         circ = &chan->circ;
    //UARTHW_Handle       hUart = chan->port->hUart;

    Uint32 tmp;
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

    tmp=ECanaRegs.CANME.all;
    tmp&=0xfffffffe;

    ECanaRegs.CANME.all=tmp; //disable this mailBox

    ECanaMboxes.MBOX0.MSGID.all=val[0];
    ECanaMboxes.MBOX0.MSGCTRL.all=val[1];
    ECanaMboxes.MBOX0.MDL.all=val[2];
    ECanaMboxes.MBOX0.MDH.all=val[3];

    ECanaRegs.CANME.all=0xffffffff; //enable this mailbox again

    ECanaRegs.CANTRS.bit.TRS0=1;//start transmit

    /*
     * Interrupts must be disabled here since UART may have only
     * one ISR for both rx and tx.  An rx ISR may call cbTxHandler()
     * and cbTxHandler() is not reentrant.
     */
//    if (UARTHW_txEmpty(hUart)) {
//        cbTxHandler(chan->port);
//    }

    //SciaRegs.SCIFFTX.bit.TXFFIENA=1;// //enable Tx Interrupt

    HWI_restore(imask);

    return (status);
}



void InitECan1()
{
	 struct ECAN_REGS ECanaShadow;
	 // Mailboxs can be written to 16-bits or 32-bits at a time
	    // Write to the MSGID field of TRANSMIT mailboxes MBOX0 - 15
	    ECanaMboxes.MBOX0.MSGID.all = 0x9555AAA0;
	    ECanaMboxes.MBOX1.MSGID.all = 0x9555AAA1;
	    ECanaMboxes.MBOX2.MSGID.all = 0x9555AAA2;
	    ECanaMboxes.MBOX3.MSGID.all = 0x9555AAA3;
	    ECanaMboxes.MBOX4.MSGID.all = 0x9555AAA4;
	    ECanaMboxes.MBOX5.MSGID.all = 0x9555AAA5;
	    ECanaMboxes.MBOX6.MSGID.all = 0x9555AAA6;
	    ECanaMboxes.MBOX7.MSGID.all = 0x9555AAA7;
	    ECanaMboxes.MBOX8.MSGID.all = 0x9555AAA8;
	    ECanaMboxes.MBOX9.MSGID.all = 0x9555AAA9;
	    ECanaMboxes.MBOX10.MSGID.all = 0x9555AAAA;
	    ECanaMboxes.MBOX11.MSGID.all = 0x9555AAAB;
	    ECanaMboxes.MBOX12.MSGID.all = 0x9555AAAC;
	    ECanaMboxes.MBOX13.MSGID.all = 0x9555AAAD;
	    ECanaMboxes.MBOX14.MSGID.all = 0x9555AAAE;
	    ECanaMboxes.MBOX15.MSGID.all = 0x9555AAAF;

	    // Write to the MSGID field of RECEIVE mailboxes MBOX16 - 31
	    ECanaMboxes.MBOX16.MSGID.all = 0xc0000000;
	    ECanaMboxes.MBOX17.MSGID.all = 0xc0000000;
	    ECanaMboxes.MBOX18.MSGID.all = 0xc0000000;
	    ECanaMboxes.MBOX19.MSGID.all = 0xc0000000;
	    ECanaMboxes.MBOX20.MSGID.all = 0xc0000000;
	    ECanaMboxes.MBOX21.MSGID.all = 0xc0000000;
	    ECanaMboxes.MBOX22.MSGID.all = 0xc0000000;
	    ECanaMboxes.MBOX23.MSGID.all = 0xc0000000;
	    ECanaMboxes.MBOX24.MSGID.all = 0xc0000000;
	    ECanaMboxes.MBOX25.MSGID.all = 0xc0000000;
	    ECanaMboxes.MBOX26.MSGID.all = 0xc0000000;
	    ECanaMboxes.MBOX27.MSGID.all = 0xc0000000;
	    ECanaMboxes.MBOX28.MSGID.all = 0xc0000000;
	    ECanaMboxes.MBOX29.MSGID.all = 0xc0000000;
	    ECanaMboxes.MBOX30.MSGID.all = 0xc0000000;
	    ECanaMboxes.MBOX31.MSGID.all = 0xc0000000;

	    // Configure Mailboxes 0-15 as Tx, 16-31 as Rx
	    // Since this write is to the entire register (instead of a bit
	    // field) a shadow register is not required.
	    ECanaRegs.CANMD.all = 0xFFFF0000;

	    // Enable all Mailboxes */
	    // Since this write is to the entire register (instead of a bit
	    // field) a shadow register is not required.
	    ECanaRegs.CANME.all = 0xFFFFFFFF;


	    ECanaLAMRegs.LAM16.all=0x9fffffff;
	    ECanaLAMRegs.LAM17.all=0x9fffffff;
	    ECanaLAMRegs.LAM18.all=0x9fffffff;
	    ECanaLAMRegs.LAM19.all=0x9fffffff;
	    ECanaLAMRegs.LAM20.all=0x9fffffff;
	    ECanaLAMRegs.LAM21.all=0x9fffffff;
	    ECanaLAMRegs.LAM22.all=0x9fffffff;
	    ECanaLAMRegs.LAM23.all=0x9fffffff;
	    ECanaLAMRegs.LAM24.all=0x9fffffff;
	    ECanaLAMRegs.LAM25.all=0x9fffffff;
	    ECanaLAMRegs.LAM26.all=0x9fffffff;
	    ECanaLAMRegs.LAM27.all=0x9fffffff;
	    ECanaLAMRegs.LAM28.all=0x9fffffff;
	    ECanaLAMRegs.LAM29.all=0x9fffffff;
	    ECanaLAMRegs.LAM30.all=0x9fffffff;
	    ECanaLAMRegs.LAM31.all=0x9fffffff;


	    // Specify that 8 bits will be sent
	    ECanaMboxes.MBOX0.MSGCTRL.bit.DLC = 8;
	    ECanaMboxes.MBOX1.MSGCTRL.bit.DLC = 8;
	    ECanaMboxes.MBOX2.MSGCTRL.bit.DLC = 8;
	    ECanaMboxes.MBOX3.MSGCTRL.bit.DLC = 8;
	    ECanaMboxes.MBOX4.MSGCTRL.bit.DLC = 8;
	    ECanaMboxes.MBOX5.MSGCTRL.bit.DLC = 8;
	    ECanaMboxes.MBOX6.MSGCTRL.bit.DLC = 8;
	    ECanaMboxes.MBOX7.MSGCTRL.bit.DLC = 8;
	    ECanaMboxes.MBOX8.MSGCTRL.bit.DLC = 8;
	    ECanaMboxes.MBOX9.MSGCTRL.bit.DLC = 8;
	    ECanaMboxes.MBOX10.MSGCTRL.bit.DLC = 8;
	    ECanaMboxes.MBOX11.MSGCTRL.bit.DLC = 8;
	    ECanaMboxes.MBOX12.MSGCTRL.bit.DLC = 8;
	    ECanaMboxes.MBOX13.MSGCTRL.bit.DLC = 8;
	    ECanaMboxes.MBOX14.MSGCTRL.bit.DLC = 8;
	    ECanaMboxes.MBOX15.MSGCTRL.bit.DLC = 8;

	    // Write to the mailbox RAM field of MBOX0 - 15
	    ECanaMboxes.MBOX0.MDL.all = 0x9555AAA0;
	    ECanaMboxes.MBOX0.MDH.all = 0x89ABCDEF;

	    ECanaMboxes.MBOX1.MDL.all = 0x9555AAA1;
	    ECanaMboxes.MBOX1.MDH.all = 0x89ABCDEF;

	    ECanaMboxes.MBOX2.MDL.all = 0x9555AAA2;
	    ECanaMboxes.MBOX2.MDH.all = 0x89ABCDEF;

	    ECanaMboxes.MBOX3.MDL.all = 0x9555AAA3;
	    ECanaMboxes.MBOX3.MDH.all = 0x89ABCDEF;

	    ECanaMboxes.MBOX4.MDL.all = 0x9555AAA4;
	    ECanaMboxes.MBOX4.MDH.all = 0x89ABCDEF;

	    ECanaMboxes.MBOX5.MDL.all = 0x9555AAA5;
	    ECanaMboxes.MBOX5.MDH.all = 0x89ABCDEF;

	    ECanaMboxes.MBOX6.MDL.all = 0x9555AAA6;
	    ECanaMboxes.MBOX6.MDH.all = 0x89ABCDEF;

	    ECanaMboxes.MBOX7.MDL.all = 0x9555AAA7;
	    ECanaMboxes.MBOX7.MDH.all = 0x89ABCDEF;

	    ECanaMboxes.MBOX8.MDL.all = 0x9555AAA8;
	    ECanaMboxes.MBOX8.MDH.all = 0x89ABCDEF;

	    ECanaMboxes.MBOX9.MDL.all = 0x9555AAA9;
	    ECanaMboxes.MBOX9.MDH.all = 0x89ABCDEF;

	    ECanaMboxes.MBOX10.MDL.all = 0x9555AAAA;
	    ECanaMboxes.MBOX10.MDH.all = 0x89ABCDEF;

	    ECanaMboxes.MBOX11.MDL.all = 0x9555AAAB;
	    ECanaMboxes.MBOX11.MDH.all = 0x89ABCDEF;

	    ECanaMboxes.MBOX12.MDL.all = 0x9555AAAC;
	    ECanaMboxes.MBOX12.MDH.all = 0x89ABCDEF;

	    ECanaMboxes.MBOX13.MDL.all = 0x9555AAAD;
	    ECanaMboxes.MBOX13.MDH.all = 0x89ABCDEF;

	    ECanaMboxes.MBOX14.MDL.all = 0x9555AAAE;
	    ECanaMboxes.MBOX14.MDH.all = 0x89ABCDEF;

	    ECanaMboxes.MBOX15.MDL.all = 0x9555AAAF;
	    ECanaMboxes.MBOX15.MDH.all = 0x89ABCDEF;

	    // Configure the eCAN for self test mode
	    // Enable the enhanced features of the eCAN.
	    EALLOW;
	    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
	    ECanaShadow.CANMC.bit.STM = 0;    // Configure CAN for self-test mode
	    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;
	    EDIS;
}

void InitECan()
{
	/* Create a shadow register structure for the CAN control registers. This is
	 needed, since only 32-bit access is allowed to these registers. 16-bit access
	 to these registers could potentially corrupt the register contents or return
	 false data. This is especially true while writing to/reading from a bit
	 (or group of bits) among bits 16 - 31 */

		struct ECAN_REGS ECanaShadow;

		asm("  EALLOW");

	/* Configure eCAN pins for CAN operation using GPIO regs*/

		GpioMuxRegs.GPFMUX.bit.CANTXA_GPIOF6 = 1;
		GpioMuxRegs.GPFMUX.bit.CANRXA_GPIOF7 = 1;

		// eCAN control registers require 32-bit access.
	    // If you want to write to a single bit, the compiler may break this
	    // access into a 16-bit access.  One solution, that is presented here,
	    // is to use a shadow register to force the 32-bit access.

	    // Read the entire register into a shadow register.  This access
	    // will be 32-bits.  Change the desired bit and copy the value back
	    // to the eCAN register with a 32-bit write.

	/* Configure eCAN RX and TX pins for CAN operation using eCAN regs*/

	    ECanaShadow.CANTIOC.all = ECanaRegs.CANTIOC.all;
	    ECanaShadow.CANTIOC.bit.TXFUNC = 1;
	    ECanaRegs.CANTIOC.all = ECanaShadow.CANTIOC.all;

	    ECanaShadow.CANRIOC.all = ECanaRegs.CANRIOC.all;
	    ECanaShadow.CANRIOC.bit.RXFUNC = 1;
	    ECanaRegs.CANRIOC.all = ECanaShadow.CANRIOC.all;

	/* Configure eCAN for HECC mode - (reqd to access mailboxes 16 thru 31) */
	// HECC mode also enables time-stamping feature

		ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
		ECanaShadow.CANMC.bit.SCB = 1;
		ECanaShadow.CANMC.bit.DBO=1;
		ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;

	/* Initialize all bits of 'Master Control Field' to zero */
	// Some bits of MSGCTRL register may come up in an unknown state. For proper operation,
	// all bits (including reserved bits) of MSGCTRL must be initialized to zero

	    ECanaMboxes.MBOX0.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX1.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX2.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX3.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX4.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX5.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX6.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX7.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX8.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX9.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX10.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX11.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX12.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX13.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX14.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX15.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX16.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX17.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX18.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX19.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX20.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX21.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX22.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX23.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX24.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX25.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX26.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX27.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX28.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX29.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX30.MSGCTRL.all = 0x00000000;
	    ECanaMboxes.MBOX31.MSGCTRL.all = 0x00000000;

	// TAn, RMPn, GIFn bits are all zero upon reset and are cleared again
	// as a matter of precaution.

	/* Clear all TAn bits */

		ECanaRegs.CANTA.all	= 0xFFFFFFFF;

	/* Clear all RMPn bits */

		ECanaRegs.CANRMP.all = 0xFFFFFFFF;

	/* Clear all interrupt flag bits */

		ECanaRegs.CANGIF0.all = 0xFFFFFFFF;
		ECanaRegs.CANGIF1.all = 0xFFFFFFFF;

	/* Configure bit timing parameters */

		ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
		ECanaShadow.CANMC.bit.CCR = 1 ;            		// Set CCR = 1
	    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;

	    ECanaShadow.CANES.all = ECanaRegs.CANES.all;

		// Wait until the CPU has been granted permission to change the configuration registers
	    do
	    {
	      ECanaShadow.CANES.all = ECanaRegs.CANES.all;
	    } while(ECanaShadow.CANES.bit.CCE != 1 );  		// Wait for CCE bit to be set..

	    ECanaShadow.CANBTC.all = 0;
	    ECanaShadow.CANBTC.bit.BRPREG = 19;				// 1 Mbps @ 150 MHz SYSCLKOUT
	    ECanaShadow.CANBTC.bit.TSEG2REG = 2;
	    ECanaShadow.CANBTC.bit.TSEG1REG = 7;
	    ECanaShadow.CANBTC.bit.SAM = 1;
	    ECanaRegs.CANBTC.all = ECanaShadow.CANBTC.all;

	    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
		ECanaShadow.CANMC.bit.CCR = 0 ;            		// Set CCR = 0
	    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;

	    ECanaShadow.CANES.all = ECanaRegs.CANES.all;

		// Wait until the CPU no longer has permission to change the configuration registers
	    do
	    {
	      ECanaShadow.CANES.all = ECanaRegs.CANES.all;
	    } while(ECanaShadow.CANES.bit.CCE != 0 ); 		// Wait for CCE bit to be  cleared..

	/* Disable all Mailboxes */
	    // Since this write is to the entire register (instead of a bit
	    // field) a shadow register is not required.

	 	ECanaRegs.CANME.all = 0;		// Required before writing the MSGIDs


	 	/* Configure CAN interrupts */

			ECanaShadow.CANGIM.all = ECanaRegs.CANGIM.all;
			ECanaShadow.CANGIM.bit.WDIM = 1;   // Enable ¡±Write denied¡± int
			ECanaShadow.CANGIM.bit.GIL = 0;    // GIL value determines eCAN(0/1)INT
			// Enable the int line chosen by SIL
			ECanaShadow.CANGIM.bit.I0EN = 1;   // Uncomment this line if GIL = 0
			ECanaShadow.CANGIM.bit.I1EN=1;
			//ECanaShadow.CANGIM.bit.I1EN = 1;   // Uncomment this line if GIL = 1

	 	    ECanaRegs.CANGIM.all = ECanaShadow.CANGIM.all;

	 	 	ECanaRegs.CANMIM.all=0xFFFFFFFF;


	 	/*Configure Pie Interrupts*/
	 	   HWI_dispatchPlug(100,(Fxn)can0Isr,NULL);
	 	   HWI_dispatchPlug(101,(Fxn)can1Isr,NULL);



	 	/* Configure PIE interrupts */

	 	PieCtrlRegs.PIECRTL.bit.ENPIE = 1;  // Enable vector fetching from PIE block
	 	PieCtrlRegs.PIEACK.bit.ACK9 = 1;    // Enables PIE to drive a pulse into the CPU
	 	// The ¡¯Write¨Cdenied¡¯ interrupt can be asserted in either of the eCAN interrupt lines
	 	// Comment out the unwanted line...
	 	PieCtrlRegs.PIEIER9.bit.INTx5 = 1;  // Enable INTx.5 of INT9 (eCAN0INT)
	 	PieCtrlRegs.PIEIER9.bit.INTx6 = 1;  // Enable INTx.6 of INT9 (eCAN1INT)
	 	/* Configure system interrupts */
	 	IER |= 0x0100; // Enable INT9 of CPU
	 	EINT; // Global enable of interrupts
}


/*
 * ISR for Scia Receive Interrupt
 */

 void can0Isr()
{

	Uint32 status=ECanaRegs.CANTA.all;
	CanChanHandle chan;
	Uint32 tmp;
	Uint32 *val;

	int index;
	volatile struct MBOX * mbox;
	int i;
	Uint32 buffer[8];
	
	if(status!=0) //transmit successful
	{
		ECanaRegs.CANTA.all=status;
		chan = &(ports[0].chans[OUTPUT]);
		getNextPacket(chan);
		if(chan->dataPacket!=NULL)
		{
			tmp=ECanaRegs.CANME.all;
			tmp&=0xfffffffe;

			ECanaRegs.CANME.all=tmp; //disable this mailBox
			val=chan->dataPacket->addr;
			ECanaMboxes.MBOX0.MSGID.all=val[0];
			ECanaMboxes.MBOX0.MSGCTRL.all=val[1];
			ECanaMboxes.MBOX0.MDL.all=val[2];
			ECanaMboxes.MBOX0.MDH.all=val[3];

			ECanaRegs.CANME.all=0xffffffff; //enable this mailbox again

			ECanaRegs.CANTRS.bit.TRS0=1;//start transmit
		}
	}
	status=ECanaRegs.CANRMP.all;
	if(status!=0) //Receive successful
	{
		for(index=0;index<32;index++)
		{
			if(status&(1UL<<index))
			{
				mbox=&(ECanaMboxes.MBOX0)+index;
				chan = &(ports[0].chans[INPUT]);
				if(chan->dataPacket==NULL) //write data to circle buffer
				{
					CIRC_Handle         circ = &chan->circ;
					Char * c=(Char*)buffer;
					
					buffer[0]=mbox->MSGID.all;
					buffer[1]=mbox->MSGCTRL.all;
					buffer[2]=mbox->MDL.all;
					buffer[3]=mbox->MDH.all;
					for(i=0;i<16;i++)
					{
						//fixed 32Bytes Size
						if (CIRC_emptyCount(circ)) 
						{
        					CIRC_writeChar(circ, c[i]);
						}
					}
					
					
				}
				else
				{
					
						Uint32* addr=(Uint32*)(chan->bufptr);
						addr[0]=mbox->MSGID.all;
						addr[1]=mbox->MSGCTRL.all;
						addr[2]=mbox->MDL.all;
						addr[3]=mbox->MDH.all;
        				chan->bufcnt-=16;
        				chan->bufptr+=16;
				        if (chan->bufcnt == 0) 
				        {
				            getNextPacket(chan);
				            PRD_start(&PRD2);
				        }
				        
				}
				
				//mbox->MSGID
			}
		}
		ECanaRegs.CANRMP.all=status;
	}


	PieCtrlRegs.PIEACK.bit.ACK9=1;



}

/*
 * ISR for Scia Transmit Interrupt
 */
 static void can1Isr()
{
//	int i=0;
//	Uint16 leftBytes= 16-SciaRegs.SCIFFTX.bit.TXFFST;
//
//	cbTxHandler(&(ports[0]),leftBytes);
//	SciaRegs.SCIFFTX.bit.TXINTCLR=1;
//	PieCtrlRegs.PIEACK.bit.ACK9=1;
}












/*
 *  -------- support functions --------
 */

/*
 *  ======== getNextPacket ========
 */
static Void getNextPacket(CanChanHandle chan)
{
    IOM_Packet  *donePacket, *nextPacket, *flushPacket;

    /* Intialize flushPacket */
    flushPacket = NULL;

    /* Save complete packet for callback */
    donePacket = chan->dataPacket;
    donePacket->size-=chan->bufcnt;

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


void CANPrd()
{
		CanChanHandle chan=&(ports[0].chans[INPUT]);
		if(chan->dataPacket!=NULL)
		{
			getNextPacket(chan);
		}
		
}



