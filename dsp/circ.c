/*
 *  Copyright 2003 by Texas Instruments Incorporated.
 *  All rights reserved. Property of Texas Instruments Incorporated.
 *  Restricted rights to use, duplicate or disclose this code are
 *  granted through contract.
 *  
 */
/* "@(#) DDK 1.10.00.23 07-02-03 (ddk-b12)" */
/*
 *  ======== circ.c ========
 */

#include <std.h>

#include <atm.h>
#include "circ.h"

/*
 *  ======== CIRC_new ========
 *
 *  Initializes the circular buffer structure
 */
Void CIRC_new(CIRC_Handle circ,char *buf,int size)
{
    circ->writeIndex = 0;
    circ->readIndex = 0;
    circ->charCount = 0;
    circ->size = size;
    circ->buf=buf;
}

/*
 *  ======== CIRC_readChar ========
 *
 *  Reads a character from the circular buffer.
 */
Char CIRC_readChar(CIRC_Handle circ)
{
    Char c;

    /* read character and increment the character count */
    c = circ->buf[circ->readIndex];
    circ->readIndex = CIRC_nextIndex(circ,circ->readIndex);
    ATM_decu(&circ->charCount);
	//circ->charCount--;
    return (c);
}

/*
 *  ======== CIRC_writeChar ========
 *
 *  Writes a character into the circular buffer 
 */
Void CIRC_writeChar(CIRC_Handle circ, Char c)
{
    /* write character and decrement the character count */
    circ->buf[circ->writeIndex] = c;
    circ->writeIndex = CIRC_nextIndex(circ,circ->writeIndex);
    //circ->charCount++;
    ATM_incu(&circ->charCount);
}

