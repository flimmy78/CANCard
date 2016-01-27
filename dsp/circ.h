/*
 *  Copyright 2003 by Texas Instruments Incorporated.
 *  All rights reserved. Property of Texas Instruments Incorporated.
 *  Restricted rights to use, duplicate or disclose this code are
 *  granted through contract.
 *  
 */
/* "@(#) DDK 1.10.00.23 07-02-03 (ddk-b12)" */
/*
 *  ======== circ.h ========
 */

#ifndef CIRC_
#define CIRC_

#ifdef __cplusplus
extern "C" {
#endif

#define CIRC_BUFSIZE    (128)      /* must be ^2! */

typedef struct CIRC_Obj {
    Uns         writeIndex;     /* write pointer for the buffer */
    Uns         readIndex;      /* read pointer fro the buffer */
    Uns         charCount;      /* buffer character count */
    Uns         size;
    char*        buf;      /* circular buffer */
} CIRC_Obj, *CIRC_Handle;

extern Void CIRC_new(CIRC_Handle circ,char *buf,int size);
extern Char CIRC_readChar(CIRC_Handle circ);
extern Void CIRC_writeChar(CIRC_Handle circ, Char c);

#define CIRC_fullCount(circ)            ((circ)->charCount)
#define CIRC_emptyCount(circ)           ((circ)->size - (circ)->charCount)
//#define CIRC_nextIndex(circ,index)      (((index) + 1) & ((circ)->size - 1))
#define CIRC_nextIndex(circ,index)      (((index) + 1)>((circ)->size)?0:((index) + 1))
//#define CIRC_prevIndex(circ,index)      (((index) - 1) & ((circ)->size - 1))
#define CIRC_prevIndex(circ,index)      (((index)==0)?((circ)->size):((index) - 1))

#define CIRC_reset(circ)            {(circ)->charCount = 0; (circ)->readIndex = (circ)->writeIndex; }
#define CIRC_isFull(circ)           ((circ)->charCount == (circ)->size)

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif /* CIRC_ */

