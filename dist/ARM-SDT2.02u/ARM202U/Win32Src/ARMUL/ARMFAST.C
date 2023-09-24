/***************************************************************************\
* armfast.c                                                                 *
* Version 1.20                                                              *
* ARMulator II Simple Memory Interface.                                     *
* Copyright (C) 1991 Advanced RISC Machines Limited. All rights reserved.   *
* Written by Dave Jaggar.                                                   *
* Project started on 1st July 1991.                                         *
\***************************************************************************/

/*
 * RCS $Revision: 1.14 $
 * Checkin $Date: 1995/01/18 12:26:52 $
 * Revising $Author: hmeekings $
 */

/* This file contains the fastest of example ARMulator memory systems.
It implements the ARM's memory as a fixed size block of host memory, and
does endian switching for byte accesses only. Memory accesses outside
the memory block are truncated to the size of the memory block. Each
routine is almost a cut-and-paste of some other, it's all quite simple */
/* Halfwords added 940909 by Andy Chapman */

#include "armdefs.h"

#define DEFMEMSIZE (512 * 1024) /* default memory size, must be a power of 2 */
#define WORDWRAP (DEFMEMSIZE - 4)
#define HWORDWRAP (DEFMEMSIZE - 2)
#define BYTEWRAP (DEFMEMSIZE - 1)

#define WORDMASK(addr) (addr & WORDWRAP)
#define HWORDMASK(addr) (addr & HWORDWRAP)
#define BYTEMASK(addr) (addr & BYTEWRAP)
#define ENDSWAP(addr) (addr ^ 3)
#define ENDSWAPH(addr) (addr ^ 2)
#define ENDSWAPB(addr) (addr ^ 1)
static unsigned HostEndian ;

#ifdef DOWATCH
#include "dbg_rdi.h"
#endif

/***************************************************************************\
*                      Initialise the memory interface                      *
\***************************************************************************/

unsigned ARMul_MemoryInit(ARMul_State *state, unsigned long initmemsize)
{unsigned char *Memory ;

 if (initmemsize == 0 || initmemsize > DEFMEMSIZE)
    initmemsize = DEFMEMSIZE ;
 state->MemSize = initmemsize ;
 Memory = (unsigned char *)malloc(DEFMEMSIZE) ;
 if (Memory == NULL)
    return(FALSE) ;
 state->MemInPtr = Memory ;
 *(ARMword *)Memory = 1 ;
 HostEndian = (*Memory != 1) ; /* 1 for big endian, 0 for little */
#ifdef BIGEND
 state->bigendSig = HIGH ;
#endif
#ifdef LITTLEEND
 state->bigendSig = LOW ;
#endif
 ARMul_ConsolePrint(state, ", %d Kb RAM",state->MemSize/1024) ;
#ifdef DOWATCH
 state->CanWatch = TRUE ;
#endif
 return(TRUE) ;
}

/***************************************************************************\
*                         Remove the memory interface                       *
\***************************************************************************/

void ARMul_MemoryExit(ARMul_State *state)
{free((char *)state->MemInPtr) ;
 return ;
 }

/***************************************************************************\
*                   Load Instruction, Sequential Cycle                      *
\***************************************************************************/

ARMword ARMul_LoadInstrS(ARMul_State *state,ARMword address)
{state->NumScycles++ ;

#ifdef DOWATCH
 if (state->MemReadDebug) ARMul_CheckWatch(state, address, RDIWatch_WordRead);
#endif

#ifdef HOURGLASS_RATE
 if( (state->NumScycles & HOURGLASS_RATE) == 0 ) {
    armsd_hourglass();
    }
#endif

 return( *(ARMword *)(state->MemInPtr + WORDMASK(address)) ) ;
}

/***************************************************************************\
*                 Load Instruction, Non Sequential Cycle                    *
\***************************************************************************/

ARMword ARMul_LoadInstrN(ARMul_State *state,ARMword address)
{state->NumNcycles++ ;
#ifdef DOWATCH
 if (state->MemReadDebug) ARMul_CheckWatch(state, address, RDIWatch_WordRead);
#endif
 return( *(ARMword *)(state->MemInPtr + WORDMASK(address)) ) ;
}

/***************************************************************************\
*                   Load 16 Bit Instruction, Sequential Cycle               *
\***************************************************************************/

ARMword ARMul_LoadInstr16S(ARMul_State *state,ARMword address)
{state->NumScycles++ ;

#ifdef HOURGLASS_RATE
 if( (state->NumScycles & HOURGLASS_RATE) == 0 ) {
    armsd_hourglass();
    }
#endif

 return( (*(ARMword *)(state->MemInPtr + WORDMASK(address))) >>
        ((((ARMword)state->bigendSig * 2) ^ (address & 2)) << 3) &
        0xffffL ) ;
}

/***************************************************************************\
*                 Load 16 Bit Instruction, Non Sequential Cycle             *
\***************************************************************************/

ARMword ARMul_LoadInstr16N(ARMul_State *state,ARMword address)
{state->NumNcycles++ ;
 return( (*(ARMword *)(state->MemInPtr + WORDMASK(address))) >>
        ((((ARMword)state->bigendSig * 2) ^ (address & 2)) << 3) &
        0xffffL ) ;
}

/***************************************************************************\
*                        Load Word, Sequential Cycle                        *
\***************************************************************************/

ARMword ARMul_LoadWordS(ARMul_State *state,ARMword address)
{state->NumScycles++ ;
#ifdef DOWATCH
 if (state->MemReadDebug) ARMul_CheckWatch(state, address, RDIWatch_WordRead);
#endif
 return( *(ARMword *)(state->MemInPtr + WORDMASK(address)) ) ;
}

/***************************************************************************\
*                      Load Word, Non Sequential Cycle                      *
\***************************************************************************/

ARMword ARMul_LoadWordN(ARMul_State *state,ARMword address)
{state->NumNcycles++ ;
#ifdef DOWATCH
 if (state->MemReadDebug) ARMul_CheckWatch(state, address, RDIWatch_WordRead);
#endif
 return( *(ARMword *)(state->MemInPtr + WORDMASK(address)) ) ;
}

/***************************************************************************\
*                     Load Halfword, (Non Sequential Cycle)                *
\***************************************************************************/

ARMword ARMul_LoadHalfWord(ARMul_State *state,ARMword address)
{state->NumNcycles++ ;
 address = BYTEMASK(address);
 if (HostEndian == state->bigendSig) {
   return( (ARMword)((*(state->MemInPtr + address) << 8) +
                     *(state->MemInPtr + ENDSWAPB(address)))) ;
 } else {
   return( (ARMword)((*(state->MemInPtr + ENDSWAPH(address)) << 8)+
                     *(state->MemInPtr + ENDSWAPB(ENDSWAPH(address))))) ;
 }
}

/***************************************************************************\
*                     Load Byte, (Non Sequential Cycle)                     *
\***************************************************************************/

ARMword ARMul_LoadByte(ARMul_State *state,ARMword address)
{state->NumNcycles++ ;
#ifdef DOWATCH
 if (state->MemReadDebug) ARMul_CheckWatch(state, address, RDIWatch_ByteRead);
#endif
 if (HostEndian == state->bigendSig)
    return( (ARMword)*(state->MemInPtr + BYTEMASK(address)) ) ;
 else
    return( (ARMword)*(state->MemInPtr + ENDSWAP(BYTEMASK(address))) ) ;
}

/***************************************************************************\
*                       Store Word, Sequential Cycle                        *
\***************************************************************************/

void ARMul_StoreWordS(ARMul_State *state,ARMword address, ARMword data)
{state->NumScycles++ ;
#ifdef DOWATCH
 if (state->MemWriteDebug) ARMul_CheckWatch(state, address, RDIWatch_WordWrite);
#endif
 *(ARMword *)(state->MemInPtr + WORDMASK(address)) = data ;
}
/***************************************************************************\
*                       Store Word, Sequential Cycle                        *
\***************************************************************************/

void ARMul_StoreWordN(ARMul_State *state,ARMword address, ARMword data)
{state->NumNcycles++ ;
#ifdef DOWATCH
 if (state->MemWriteDebug) ARMul_CheckWatch(state, address, RDIWatch_WordWrite);
#endif
 *(ARMword *)(state->MemInPtr + WORDMASK(address) ) = data ;
}

/***************************************************************************\
*                    Store HalfWord, (Non Sequential Cycle)                *
\***************************************************************************/

void ARMul_StoreHalfWord(ARMul_State *state,ARMword address, ARMword data)
{state->NumNcycles++ ;

 address = HWORDMASK(address);
 if (HostEndian == state->bigendSig) {
   *(state->MemInPtr + address) = (unsigned char)((data & 0xff00) >> 8);
   *(state->MemInPtr + address + 1) = (unsigned char)data ;
 } else {
   *(state->MemInPtr + ENDSWAPH(address)) = (unsigned char)((data & 0xff00) >> 8);
   *(state->MemInPtr + ENDSWAPH(address) + 1) = (unsigned char)data ;
 }
}

/***************************************************************************\
*                    Store Byte, (Non Sequential Cycle)                     *
\***************************************************************************/

void ARMul_StoreByte(ARMul_State *state,ARMword address, ARMword data)
{state->NumNcycles++ ;
#ifdef DOWATCH
 if (state->MemWriteDebug) ARMul_CheckWatch(state, address, RDIWatch_ByteWrite);
#endif
 if (HostEndian == state->bigendSig)
    *(state->MemInPtr + BYTEMASK(address)) = (unsigned char)data ;
 else
    *(state->MemInPtr + ENDSWAP(BYTEMASK(address))) = (unsigned char)data ;
}

/***************************************************************************\
*                   Swap Word, (Two Non Sequential Cycles)                  *
\***************************************************************************/

ARMword ARMul_SwapWord(ARMul_State *state,ARMword address, ARMword data)
{ARMword temp ;

 temp = ARMul_LoadWordN(state,address) ;
 ARMul_StoreWordN(state,address,data) ;
 return(temp) ;
}

/***************************************************************************\
*                   Swap Byte, (Two Non Sequential Cycles)                  *
\***************************************************************************/

ARMword ARMul_SwapByte(ARMul_State *state,ARMword address, ARMword data)
{ARMword temp ;

 temp = ARMul_LoadByte(state,address) ;
 ARMul_StoreByte(state,address,data) ;
 return(temp) ;
}

/***************************************************************************\
*                             Count I Cycles                                *
\***************************************************************************/

void ARMul_Icycles(ARMul_State *state,unsigned number, ARMword address)
{state->NumIcycles += number ;
}

/***************************************************************************\
*                             Count C Cycles                                *
\***************************************************************************/

void ARMul_Ccycles(ARMul_State *state,unsigned number, ARMword address)
{state->NumCcycles += number ;
}

/***************************************************************************\
*                      Read Word (but don't tell anyone!)                   *
\***************************************************************************/

ARMword ARMul_ReadWord(ARMul_State *state,ARMword address)
{
 return( *(ARMword *)(state->MemInPtr + WORDMASK(address)) ) ;
}

/***************************************************************************\
*                      Read Halfword (but don't tell anyone!)              *
\***************************************************************************/

ARMword ARMul_ReadHalfWord(ARMul_State *state,ARMword address)
{
 address = BYTEMASK(address);
 if (HostEndian == state->bigendSig) {
   return( (ARMword)((*(state->MemInPtr + address) << 8) +
                     *(state->MemInPtr + ENDSWAPB(address)))) ;
 } else {
   return( (ARMword)((*(state->MemInPtr + ENDSWAPH(address)) << 8)+
                     *(state->MemInPtr + ENDSWAPB(ENDSWAPH(address))))) ;
 }
}

/***************************************************************************\
*                      Read Byte (but don't tell anyone!)                   *
\***************************************************************************/

ARMword ARMul_ReadByte(ARMul_State *state,ARMword address)
{
 if (HostEndian == state->bigendSig)
    return( (ARMword)*(state->MemInPtr + BYTEMASK(address)) ) ;
 else
    return( (ARMword)*(state->MemInPtr + ENDSWAP(BYTEMASK(address))) ) ;
}

/***************************************************************************\
*                     Write Word (but don't tell anyone!)                   *
\***************************************************************************/

void ARMul_WriteWord(ARMul_State *state,ARMword address, ARMword data)
{
 *(ARMword *)(state->MemInPtr + WORDMASK(address)) = data ;
}

/***************************************************************************\
*                     Write Halfword (but don't tell anyone!)              *
\***************************************************************************/

void ARMul_WriteHalfWord(ARMul_State *state,ARMword address, ARMword data)
{
 address = HWORDMASK(address);
 if (HostEndian == state->bigendSig) {
   *(state->MemInPtr + address) = (unsigned char)((data & 0xff00) >> 8);
   *(state->MemInPtr + address + 1) = (unsigned char)data ;
 } else {
   *(state->MemInPtr + ENDSWAPH(address)) = (unsigned char)((data & 0xff00) >> 8);
   *(state->MemInPtr + ENDSWAPH(address) + 1) = (unsigned char)data ;
 }
}

/***************************************************************************\
*                     Write Byte (but don't tell anyone!)                   *
\***************************************************************************/

void ARMul_WriteByte(ARMul_State *state,ARMword address, ARMword data)
{
 if (HostEndian == state->bigendSig)
    *(state->MemInPtr + BYTEMASK(address)) = (unsigned char)data ;
 else
    *(state->MemInPtr + ENDSWAP(BYTEMASK(address))) = (unsigned char)data ;
}

