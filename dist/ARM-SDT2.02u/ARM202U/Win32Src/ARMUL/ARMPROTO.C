/***************************************************************************\
* armproto.c                                                                *
* Version 1.20                                                              *
* ARMulator II Fast Prototype Memory Interface.                             *
* Copyright (C) 1991 Advanced RISC Machines Limited. All rights reserved.   *
* Written by Dave Jaggar.                                                   *
* Project started on 1st July 1991.                                         *
\***************************************************************************/

/*
 * RCS $Revision: 1.12 $
 * Checkin $Date: 1995/01/18 12:27:02 $
 * Revising $Author: hmeekings $
 */

/* This file contains the ARMulator interface to the memory model
designed for fast prototyping. This model is also the closest to the
real ARM memory interface. Two routines must be written to build a
memory interface, ARMul_MemoryInit and ARMul_MemoryAccess. An example
memory interface is implemented, with 64K pages, allocated on demand
from a 64K entry page table. The routines PutWord and GetWord implement
this, lifted from armvirt.c, the virtual memory model. Pages are never
freed as they might be needed again. A single area of memory may be
defined to generate aborts. */

#include "armdefs.h"

#ifdef VALIDATE /* for running the validate suite */
#define TUBE 48 * 1024 * 1024 /* write a char on the screen */
#define ABORTS
#endif

#ifdef ABORTS /* the memory system will abort */
/* For the old test suite Abort between 32 Kbytes and 32 Mbytes
   For the new test suite Abort between 8 Mbytes and 26 Mbytes */
/* #define LOWABORT 32 * 1024
   #define HIGHABORT 32 * 1024 * 1024 */
#define LOWABORT 8 * 1024 * 1024
#define HIGHABORT 26 * 1024 * 1024
#endif

/* #define TRACEBUS */

#define NUMPAGES 64 * 1024
#define PAGESIZE 64 * 1024
#define PAGEBITS 16
#define OFFSETBITS 0xffff

static ARMword GetWord(ARMul_State *state,ARMword address) ;
static void PutWord(ARMul_State *state,ARMword address, ARMword word) ;

/***************************************************************************\
*                      Initialise the memory interface                      *
\***************************************************************************/

unsigned ARMul_MemoryInit(ARMul_State *state, unsigned long initmemsize)
{ARMword **pagetable ;
 unsigned page ;

 state->MemSize = initmemsize ;
 pagetable = (ARMword **)malloc(sizeof(ARMword)*NUMPAGES) ;
 if (pagetable == NULL)
    return(FALSE) ;
 for (page = 0 ; page < NUMPAGES ; page++)
    *(pagetable + page) = NULL ;
 state->MemDataPtr = (unsigned char *)pagetable ;
#ifdef BIGEND
 state->bigendSig = HIGH ;
#endif
#ifdef LITTLEEND
 state->bigendSig = LOW ;
#endif
 ARMul_ConsolePrint(state, ", 4 Gb memory") ;
 return(TRUE) ;
}

/***************************************************************************\
*                         Remove the memory interface                       *
\***************************************************************************/

void ARMul_MemoryExit(ARMul_State *state)
{ARMword page ;
 ARMword **pagetable ;
 ARMword *pageptr ;

 pagetable = (ARMword **)state->MemDataPtr ;
 for (page = 0 ; page < NUMPAGES ; page++) {
    pageptr = *(pagetable + page) ;
    if (pageptr != NULL)
       free((char *)pageptr) ;
    }
 free((char *)pagetable) ;
 return ;
 }

/***************************************************************************\
* Generic memory interface. Just alter this for a prototype memory system.  *
\***************************************************************************/

ARMword ARMul_MemAccess(ARMul_State *state,
                        ARMword address,
                        ARMword dataOut,
                        ARMword mas1,
                        ARMword mas0,
                        ARMword rw,
                        ARMword seq,
                        ARMword mreq,
                        ARMword opc,
                        ARMword lock,
                        ARMword trans,
                        ARMword account)
{ARMword dataIn = 0 ;

#ifdef TRACEBUS
       ARMul_DebugPrint(state, "A=%08x, M=%01x, S=%01x, O=%01x\n",
                        address,mreq,seq,opc) ;
#endif

 ARMul_CLEARABORT ;
 if (mreq == LOW) { /* memory request */
    if (account == TRUE) { /* a "real" memory access */
       if (seq == LOW) /* non-sequential (N) cycle */
          state->NumNcycles++ ;
       else
          state->NumScycles++ ;
       }
#ifdef ABORTS
    if (address >= LOWABORT && address < HIGHABORT) {
       if (opc == LOW) { /* opcode fetch */
          ARMul_PREFETCHABORT(address) ;
          }
       else { /* data access */
          ARMul_DATAABORT(address) ;
          }
#ifdef TRACEBUS
       ARMul_DebugPrint(state, "ABORT\n") ;
#endif
       return(ARMul_ABORTWORD) ;
       } /* abort */
#endif
    if (rw == LOW) { /* read */
       /* it's always a word, bytes and halfwords are extracted by ARM */
       dataIn = GetWord(state,address) ;
       } /* read */
    else { /* write */
      if (mas0 == LOW) {   /* byte or word */
        if (mas1 == LOW) { /* byte */
          ARMword word, offset ;

#ifdef VALIDATE
          if (address == TUBE) {
            if (dataOut == 4)
              state->Emulate = FALSE ;
            else
              (void)putc((char)dataOut,stderr) ; /* Write Char */
            return(0) ;
          }
#endif
          word = GetWord(state,address) ;
          offset = (((ARMword)state->bigendSig * 3) ^ (address & 3)) << 3 ; /* bit offset into the word */
          PutWord(state,address,(word & ~(0xffL << offset)) | ((dataOut & 0xffL) << offset)) ;
        }
        else {  /* write word */
          PutWord(state,address,dataOut) ;
        }
      } /* byte or word */
      else {               /* halfword or reserved */
        if (mas1 == LOW) { /* halfword */
          ARMword word, offset ;
          word = GetWord(state,address) ;
          offset = (((ARMword)state->bigendSig * 2) ^ (address & 2)) << 3 ; /* bit offset into the word */
          PutWord(state,address,(word & ~(0xffffL << offset)) | ((dataOut & 0xffffL) << offset)) ;
        } else {           /* reserved */
          ARMul_ConsolePrint(state, "Reserved memory access\n") ;
        }
      } /* halfword or reserved */
    } /* write */
  } /* memory request */
 else { /* not a memory request */
    if (seq == LOW) /* internal (I) cycle */
       state->NumIcycles++ ;
    else /* co-processor (C) cycle */
       state->NumCcycles++ ;
    }
 return(dataIn) ;
 }

/***************************************************************************\
*        Get a Word from Virtual Memory, maybe allocating the page          *
\***************************************************************************/

static ARMword GetWord(ARMul_State *state, ARMword address)
{ARMword page, offset ;
 ARMword **pagetable ;
 ARMword *pageptr ;

 page = address >> PAGEBITS ;
 offset = (address & OFFSETBITS) >> 2 ;
 pagetable = (ARMword **)state->MemDataPtr ;
 pageptr = *(pagetable + page) ;

 if (pageptr == NULL) {
    pageptr = (ARMword *)malloc(PAGESIZE) ;
    if (pageptr == NULL) {
       perror("ARMulator can't allocate VM page") ;
       exit(12) ;
       }
    *(pagetable + page) = pageptr ;
    }

 return(*(pageptr + offset)) ;
 }

/***************************************************************************\
*        Put a Word into Virtual Memory, maybe allocating the page          *
\***************************************************************************/

static void PutWord(ARMul_State *state, ARMword address, ARMword data)
{ARMword page, offset ;
 ARMword **pagetable ;
 ARMword *pageptr ;

 page = address >> PAGEBITS ;
 offset = (address & OFFSETBITS) >> 2 ;
 pagetable = (ARMword **)state->MemDataPtr ;
 pageptr = *(pagetable + page) ;

 if (pageptr == NULL) {
    pageptr = (ARMword *)malloc(PAGESIZE) ;
    if (pageptr == NULL) {
       perror("ARMulator can't allocate VM page") ;
       exit(13) ;
       }
    *(pagetable + page) = pageptr ;
    }

 *(pageptr + offset) = data ;
 }

/*-------------------------------------------------------------------------*\
*                                                                           *
|                 NOTHING BELOW HERE SHOULD NEED CHANGING                   |
*                                                                           *
\*-------------------------------------------------------------------------*/


/***************************************************************************\
*                   Load Instruction, Sequential Cycle                      *
\***************************************************************************/

ARMword ARMul_LoadInstrS(ARMul_State *state, ARMword address)
{ARMword temp ;

#ifdef HOURGLASS_RATE
 if( ( state->NumScycles & HOURGLASS_RATE ) == 0 ) {
    armsd_hourglass();
    }
#endif

 /* address, data, mas1, mas0, rw, seq, mreq, opc, lock, trans, account */
 temp = ARMul_MemAccess(state,address,0L, 1L,0L,0L, 1L,0L, 0L, 0L,1L, 1L) ;
 return(temp) ;
}

/***************************************************************************\
*                 Load Instruction, Non Sequential Cycle                    *
\***************************************************************************/

ARMword ARMul_LoadInstrN(ARMul_State *state, ARMword address)
{ARMword temp ;

 /* address, data, mas1, mas0, rw, seq, mreq, opc, lock, trans, account */
 temp = ARMul_MemAccess(state,address,0L, 1L,0L,0L, 0L,0L, 0L, 0L,1L, 1L) ;
 return(temp) ;
 }

/***************************************************************************\
*                   Load 16 bit Instruction, Sequential Cycle               *
\***************************************************************************/

ARMword ARMul_LoadInstr16S(ARMul_State *state, ARMword address)
{ARMword temp, offset ;

#ifdef HOURGLASS_RATE
 if( ( state->NumScycles & HOURGLASS_RATE ) == 0 ) {
    armsd_hourglass();
    }
#endif

 /* address, data, mas1, mas0, rw, seq, mreq, opc, lock, trans, account */
 temp = ARMul_MemAccess(state,address,0L, 0L,1L,0L, 1L,0L, 0L, 0L,1L, 1L) ;
 offset = (((ARMword)state->bigendSig * 2) ^ (address & 2)) << 3 ; /* bit offset into the word */
 return(temp >> offset & 0xffffL) ;
}

/***************************************************************************\
*                 Load 16 bit Instruction, Non Sequential Cycle            *
\***************************************************************************/

ARMword ARMul_LoadInstr16N(ARMul_State *state, ARMword address)
{ARMword temp, offset ;

 /* address, data, mas1, mas0, rw, seq, mreq, opc, lock, trans, account */
 temp = ARMul_MemAccess(state,address,0L, 0L,1L,0L, 0L,0L, 0L, 0L,1L, 1L) ;
 offset = (((ARMword)state->bigendSig * 2) ^ (address & 2)) << 3 ; /* bit offset into the word */
 return(temp >> offset & 0xffffL) ;
 }

/***************************************************************************\
*                        Load Word, Sequential Cycle                        *
\***************************************************************************/

ARMword ARMul_LoadWordS(ARMul_State *state, ARMword address)
{ARMword temp ;

 /* address, data, mas1, mas0, rw, seq, mreq, opc, lock, trans, account */
 temp = ARMul_MemAccess(state,address,0L, 1L,0L,0L, 1L,0L, 1L, 0L,1L, 1L) ;
 return(temp) ;
 }

/***************************************************************************\
*                      Load Word, Non Sequential Cycle                      *
\***************************************************************************/

ARMword ARMul_LoadWordN(ARMul_State *state, ARMword address)
{ARMword temp ;

 /* address, data, mas1, mas0, rw, seq, mreq, opc, lock, trans, account */
 temp = ARMul_MemAccess(state,address,0L, 1L,0L,0L, 0L,0L, 1L, 0L,1L, 1L) ;
 return(temp) ;
 }

/***************************************************************************\
*                     Load Halfword, (Non Sequential Cycle)                *
\***************************************************************************/

ARMword ARMul_LoadHalfWord(ARMul_State *state, ARMword address)
{ARMword word, offset ;

 /* address, data, mas1, mas0, rw, seq, mreq, opc, lock, trans, account */
 word = ARMul_MemAccess(state,address,0L, 0L,1L,0L, 0L,0L, 1L, 0L,1L, 1L) ;
 offset = (((ARMword)state->bigendSig * 2) ^ (address & 2)) << 3 ; /* bit offset into the word */
 return(word >> offset & 0xffffL) ;
 }

/***************************************************************************\
*                     Load Byte, (Non Sequential Cycle)                     *
\***************************************************************************/

ARMword ARMul_LoadByte(ARMul_State *state, ARMword address)
{ARMword word, offset ;

 /* address, data, mas1, mas0, rw, seq, mreq, opc, lock, trans, account */
 word = ARMul_MemAccess(state,address,0L, 0L,0L,0L, 0L,0L, 1L, 0L,1L, 1L) ;
 offset = (((ARMword)state->bigendSig * 3) ^ (address & 3)) << 3 ; /* bit offset into the word */
 return(word >> offset & 0xffL) ;
 }

/***************************************************************************\
*                       Store Word, Sequential Cycle                        *
\***************************************************************************/

void ARMul_StoreWordS(ARMul_State *state, ARMword address, ARMword data)
{
 /* address, data, mas1, mas0, rw, seq, mreq, opc, lock, trans, account */
 (void)ARMul_MemAccess(state,address,data, 1L,0L,1L, 1L,0L, 1L, 0L,1L, 1L) ;
}
/***************************************************************************\
*                       Store Word, Sequential Cycle                        *
\***************************************************************************/

void ARMul_StoreWordN(ARMul_State *state, ARMword address, ARMword data)
{
 /* address, data, mas1, mas0, rw, seq, mreq, opc, lock, trans, account */
 (void)ARMul_MemAccess(state,address,data, 1L,0L,1L, 0L,0L, 1L, 0L,1L, 1L) ;
}

/***************************************************************************\
*                    Store Halfword, (Non Sequential Cycle)                *
\***************************************************************************/

void ARMul_StoreHalfWord(ARMul_State *state, ARMword address, ARMword data)
{
 /* address, data, mas1, mas0, rw, seq, mreq, opc, lock, trans, account */
 (void)ARMul_MemAccess(state,address,data, 0L,1L,1L, 0L,0L, 1L, 0L,1L, 1L) ;
 }

/***************************************************************************\
*                    Store Byte, (Non Sequential Cycle)                     *
\***************************************************************************/

void ARMul_StoreByte(ARMul_State *state, ARMword address, ARMword data)
{
 /* address, data, mas1, mas0, rw, seq, mreq, opc, lock, trans, account */
 (void)ARMul_MemAccess(state,address,data, 0L,0L,1L, 0L,0L, 1L, 0L,1L, 1L) ;
 }

/***************************************************************************\
*                   Swap Word, (Two Non Sequential Cycles)                  *
\***************************************************************************/

ARMword ARMul_SwapWord(ARMul_State *state, ARMword address, ARMword data)
{ARMword temp ;

 /* address, data, mas1, mas0, rw, seq, mreq, opc, lock, trans, account */
 temp = ARMul_MemAccess(state,address,0L, 1L,0L,0L, 0L,0L, 1L, 1L,1L, 1L) ;
 (void)ARMul_MemAccess(state,address,data, 1L,0L,1L, 0L,0L, 1L, 0L,1L, 1L) ;
 return(temp) ;
 }

/***************************************************************************\
*                   Swap Byte, (Two Non Sequential Cycles)                  *
\***************************************************************************/

ARMword ARMul_SwapByte(ARMul_State *state, ARMword address, ARMword data)
{ARMword word, offset ;

 /* address, data, mas1, mas0, rw, seq, mreq, opc, lock, trans, account */
 word = ARMul_MemAccess(state,address,0L, 0L,0L,0L, 0L,0L, 1L, 1L,1L, 1L) ;
 (void)ARMul_MemAccess(state,address,data, 0L,0L,1L, 0L,0L, 1L, 0L,1L, 1L) ;
 offset = (((ARMword)state->bigendSig * 3) ^ (address & 3)) << 3 ; /* bit offset into the word */
 return(word >> offset & 0xffL) ;
 }

/***************************************************************************\
*                             Count I Cycles                                *
\***************************************************************************/

void ARMul_Icycles(ARMul_State *state, unsigned number, ARMword address)
{
 /* address, data, mas1, mas0, rw, seq, mreq, opc, lock, trans, account */
 while (number--)
    (void)ARMul_MemAccess(state,address,0L, 0L,0L,0L, 0L,1L, 1L, 0L,1L, 1L) ;
 }
/***************************************************************************\
*                             Count C Cycles                                *
\***************************************************************************/

void ARMul_Ccycles(ARMul_State *state, unsigned number, ARMword address)
{
 /* address, data, mas1, mas0, rw, seq, mreq, opc, lock, trans, account */
 while (number--)
   (void)ARMul_MemAccess(state,address,0L, 0L,0L,0L, 1L,1L, 1L, 0L,1L, 1L) ;
 }

/***************************************************************************\
*                      Read Word (but don't tell anyone!)                   *
\***************************************************************************/

ARMword ARMul_ReadWord(ARMul_State *state, ARMword address)
{
 /* address, data, mas1, mas0, rw, seq, mreq, opc, lock, trans, account */
 return(ARMul_MemAccess(state,address,0L, 1L,0L,0L, 0L,0L, 1L, 0L,1L, 0L)) ;
 }

/***************************************************************************\
*                      Read Halfword (but don't tell anyone!)              *
\***************************************************************************/

ARMword ARMul_ReadHalfWord(ARMul_State *state, ARMword address)
{ARMword word, offset ;

 /* address, data, mas1, mas0, rw, seq, mreq, opc, lock, trans, account */
 word = ARMul_MemAccess(state,address,0L, 0L,1L,0L, 0L,0L, 1L, 0L,1L, 0L) ;
 offset = (((ARMword)state->bigendSig * 2) ^ (address & 2)) << 3 ; /* bit offset into the word */
 return(word >> offset & 0xffffL) ;
 }

/***************************************************************************\
*                      Read Byte (but don't tell anyone!)                   *
\***************************************************************************/

ARMword ARMul_ReadByte(ARMul_State *state, ARMword address)
{ARMword word, offset ;

 /* address, data, mas1, mas0, rw, seq, mreq, opc, lock, trans, account */
 word = ARMul_MemAccess(state,address,0L, 0L,0L,0L, 0L,0L, 1L, 0L,1L, 0L) ;
 offset = (((ARMword)state->bigendSig * 3) ^ (address & 3)) << 3 ; /* bit offset into the word */
 return(word >> offset & 0xffL) ;
 }

/***************************************************************************\
*                     Write Word (but don't tell anyone!)                   *
\***************************************************************************/

void ARMul_WriteWord(ARMul_State *state, ARMword address, ARMword data)
{
 /* address, data, mas1, mas0, rw, seq, mreq, opc, lock, trans, account */
 (void)ARMul_MemAccess(state,address,data, 1L,0L,1L, 0L,0L, 1L, 0L,1L, 0L) ;
}

/***************************************************************************\
*                     Write HalfWord (but don't tell anyone!)              *
\***************************************************************************/

void ARMul_WriteHalfWord(ARMul_State *state, ARMword address, ARMword data)
{
 /* address, data, mas1, mas0, rw, seq, mreq, opc, lock, trans, account */
 (void)ARMul_MemAccess(state,address,data, 0L,1L,1L, 0L,0L, 1L, 0L,1L, 0L) ;
 }

/***************************************************************************\
*                     Write Byte (but don't tell anyone!)                   *
\***************************************************************************/

void ARMul_WriteByte(ARMul_State *state, ARMword address, ARMword data)
{
 /* address, data, mas1, mas0, rw, seq, mreq, opc, lock, trans, account */
 (void)ARMul_MemAccess(state,address,data, 0L,0L,1L, 0L,0L, 1L, 0L,1L, 0L) ;
 }

