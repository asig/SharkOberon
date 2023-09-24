/***************************************************************************
 * armvirt.c
 * ARMulator II virtual memory interface.
 * Copyright (C) 1991 Advanced RISC Machines Limited.  All rights reserved.
 ***************************************************************************/

/*
 * RCS $Revision: 1.19.2.4 $
 * Checkin $Date: 1995/08/31 09:36:54 $
 * Revising $Author: plg $
 */

/* This file contains a complete ARMulator memory model, modelling a
"virtual memory" system.  A much simpler model can be found in
armfast.c, and that model goes faster too, but has a fixed amount of
memory.  This model's memory has 64K pages, allocated on demand from a
64K entry page table, when the page is first written.  Reads to an
unwritten (un-allocated) page are serviced by an access to a dummy page.
Pages are never freed as they might be needed again.  A single area of
memory can be defined to generate aborts.  */

#include "armdefs.h"

/* The following hack, if defined, uses the F counter to count 32 bit
   word accesses. This is useful when pretending to have 16 bit wide memory.
 */
/* #define RUN_IN_16_BIT_MEMORY_HACK */


#ifdef VALIDATE /* for running the validate suite */
#define TUBE 48 * 1024 * 1024 /* write a char on the screen */
#define ABORTS 1
#endif

#ifdef ABORTS /* the memory system will abort */
/* For the old test suite Abort between 32 Kbytes and 32 Mbytes
   For the new test suite Abort between 8 Mbytes and 26 Mbytes */
#define LOWABORT 32 * 1024
#define HIGHABORT 32 * 1024 * 1024
/* #define LOWABORT 8 * 1024 * 1024
#define HIGHABORT 26 * 1024 * 1024 */
#endif

#define TOP_OF_MEM 0x80000000   /* 2Gb to avoid stack-checking probs */
#define NUMPAGES 64 * 1024
#define PAGESIZE 64 * 1024
#define PAGEBITS 16
#define WORDOFFSETBITS 0xfffc
#define HWRDOFFSETBITS 0xfffe
#define BYTEOFFSETBITS 0xffff
#define ENDSWAP(addr) (addr ^ 3)
#define ENDSWAPH(addr) (addr ^ 2)
static unsigned HostEndian ;

#define PAGETABLE ((unsigned char **)state->MemDataPtr)
#define DUMMYPAGE ((unsigned char *)state->MemSparePtr)

/***************************************************************************\
*                   Get a Word/Byte from Virtual Memory                     *
\***************************************************************************/

#define GetWord(state, address)                                         \
   *( (ARMword *)                                                       \
      ( *(PAGETABLE + (address >> PAGEBITS)) + (address & WORDOFFSETBITS) ) \
      )

#define GetHalfWord(state, address)                                                  \
  (HostEndian ?                                                                      \
   (                                                                                 \
    (*( *(PAGETABLE + (address >> PAGEBITS)) + (address & HWRDOFFSETBITS)) << 8) +   \
     *( *(PAGETABLE + (address >> PAGEBITS)) + (address & HWRDOFFSETBITS) + 1)       \
   ) :                                                                               \
   (                                                                                 \
     *( *(PAGETABLE + (address >> PAGEBITS)) + (address & HWRDOFFSETBITS)) +         \
    (*( *(PAGETABLE + (address >> PAGEBITS)) + (address & HWRDOFFSETBITS) + 1) << 8) \
   ))

#define GetByte(state, address)                                         \
   *(                                                                   \
      ( *(PAGETABLE + (address >> PAGEBITS)) + (address & BYTEOFFSETBITS) ) \
      )

/***************************************************************************\
*      Put a Word/Byte into Virtual Memory, maybe allocating the page       *
\***************************************************************************/

#define PutWord(state, address, data)                                   \
  {unsigned char *xxpageptr;                                            \
   if ((xxpageptr = PAGETABLE[address >> PAGEBITS]) == DUMMYPAGE)       \
      xxpageptr = AllocatePage(state,address);                          \
   *(ARMword *)(xxpageptr + (address & WORDOFFSETBITS)) = data;         \
   }

#define PutHalfWord(state, address, data)                               \
  {unsigned char *xxpageptr;                                            \
   ARMword xxaddr;                                                      \
   if ((xxpageptr = PAGETABLE[(address) >> PAGEBITS]) == DUMMYPAGE)     \
      xxpageptr = AllocatePage(state, address);                         \
   xxaddr = (address) & HWRDOFFSETBITS;                                 \
   if (HostEndian) {                                                    \
      xxpageptr[xxaddr] = (unsigned char)(((data) & 0xff00) >> 8);      \
      xxpageptr[xxaddr + 1] = (unsigned char)((data) & 0xff);           \
   } else {                                                             \
      xxpageptr[xxaddr] = (unsigned char)((data) & 0xff);               \
      xxpageptr[xxaddr + 1] = (unsigned char)(((data) & 0xff00) >> 8);  \
   }                                                                    \
   }

#define PutByte(state, address, data)                                   \
  {unsigned char *xxpageptr ;                                           \
   if ((xxpageptr = PAGETABLE[address >> PAGEBITS]) == DUMMYPAGE)       \
      xxpageptr = AllocatePage(state,address) ;                         \
   xxpageptr[address & BYTEOFFSETBITS] = (unsigned char)data ;          \
   }

/***************************************************************************\
*                    Allocate and return a memory page                      *
\***************************************************************************/

static unsigned char * AllocatePage(ARMul_State *state, ARMword address)
{unsigned char *pageptr ;

 pageptr = (unsigned char *)malloc(PAGESIZE) ;
 if (pageptr == NULL) {
    perror("ARMulator can't allocate VM page") ;
    exit(13) ;
    }
 *(PAGETABLE + (address >> PAGEBITS)) = pageptr ;
 return(pageptr) ;
 }

/***************************************************************************\
*                      Initialise the memory interface                      *
\***************************************************************************/

unsigned ARMul_MemoryInit(ARMul_State *state, unsigned long initmemsize)
{unsigned char **pagetable ;
 unsigned page ;
 unsigned char *dummypage ;

 if (initmemsize == 0)
   state->MemSize = TOP_OF_MEM; /* initialise to 4Gb if no size specified */
 else
   state->MemSize = initmemsize;
 if ((pagetable = (unsigned char **)malloc(sizeof(ARMword *)*NUMPAGES))==NULL)
    return(FALSE) ;
 if ((dummypage = (unsigned char *)malloc(PAGESIZE))==NULL)
    return(FALSE) ;
 for (page = 0 ; page < NUMPAGES ; page++)
    *(pagetable + page) = dummypage ;
 state->MemDataPtr = (unsigned char *)pagetable ;
 state->MemSparePtr = (unsigned char *)dummypage ;
 *(ARMword *)dummypage = 1 ;
 HostEndian = (*dummypage != 1) ; /* 1 for big endian, 0 for little */
  *(ARMword *)dummypage = 0 ;
#ifdef BIGEND
 state->bigendSig = HIGH ;
#endif
#ifdef LITTLEEND
 state->bigendSig = LOW ;
#endif
 if (state->MemSize >= 10 * 1024 * 1024)
   ARMul_ConsolePrint(state, ", %dMbyte",state->MemSize/1024/1024);
 else
   ARMul_ConsolePrint(state, ", %dKbyte",state->MemSize/1024);
 return(TRUE) ;
}

/***************************************************************************\
*                         Remove the memory interface                       *
\***************************************************************************/

void ARMul_MemoryExit(ARMul_State *state)
{ARMword page ;
 unsigned char *pageptr ;

 for (page = 0 ; page < NUMPAGES ; page++) {
    pageptr = *(PAGETABLE + page) ;
    if (pageptr != DUMMYPAGE)
       free((unsigned char *)pageptr) ;
    }
 free((unsigned char *)DUMMYPAGE) ;
 free((unsigned char *)PAGETABLE) ;
 return ;
 }

/***************************************************************************\
*                   Load Instruction, Sequential Cycle                      *
\***************************************************************************/

#define NS_PER_S 1000000000
#define ADDNS(p,t) \
  do { \
    (p)->a.ns += (p)->t; \
    if ((p)->a.ns >= NS_PER_S) (p)->a.ns -= NS_PER_S, (p)->a.s++; } \
  while (0)

ARMword ARMul_LoadInstrS(ARMul_State *state,ARMword address)
{
    MemDescr *m;

    state->NumScycles++ ;
    for (m = state->MemInfoPtr; m != NULL; m = m->next)
        if (address >= m->md.start && address < m->md.limit) {
            m->a.Sreads +=  4L >> m->md.width;
            ADDNS(m, ns_LoadInstrS);
            break;
        }
    if (!m) {
         state->ns += state->cpu_ns;
         if (state->ns >= NS_PER_S) state->ns -= NS_PER_S, state->s++;
    }

#ifdef HOURGLASS_RATE
 if( ( state->NumScycles & HOURGLASS_RATE ) == 0 ) {
    armsd_hourglass();
    }
#endif

#ifdef ABORTS
 if (address >= LOWABORT && address < HIGHABORT) {
    ARMul_PREFETCHABORT(address) ;
    return(ARMul_ABORTWORD) ;
    }
 else {
    ARMul_CLEARABORT ;
    }
#endif

 return(GetWord(state,address)) ;
}

/***************************************************************************\
*                 Load Instruction, Non Sequential Cycle                    *
\***************************************************************************/

ARMword ARMul_LoadInstrN(ARMul_State *state,ARMword address)
{
    MemDescr *m;

    state->NumNcycles++ ;
#ifdef RUN_IN_16_BIT_MEMORY_HACK
state->NumFcycles++ ;
#endif
    for (m = state->MemInfoPtr; m != NULL; m = m->next)
        if (address >= m->md.start && address < m->md.limit) {
            m->a.Nreads++;
            m->a.Sreads += (4L >> m->md.width) - 1;
            ADDNS(m, ns_LoadInstrN);
            break;
        }
    if (!m) {
         state->ns += state->cpu_ns;
         if (state->ns >= NS_PER_S) state->ns -= NS_PER_S, state->s++;
    }

#ifdef ABORTS
 if (address >= LOWABORT && address < HIGHABORT) {
    ARMul_PREFETCHABORT(address) ;
    return(ARMul_ABORTWORD) ;
    }
 else {
    ARMul_CLEARABORT ;
    }
#endif

 return(GetWord(state,address)) ;
}

/***************************************************************************\
*                   Load 16 bit Instruction, Sequential Cycle               *
\***************************************************************************/

ARMword ARMul_LoadInstr16S(ARMul_State *state,ARMword address)
{
    MemDescr *m;
    ARMword temp ;

    state->NumScycles++ ;
    for (m = state->MemInfoPtr; m != NULL; m = m->next)
        if (address >= m->md.start && address < m->md.limit) {
            if ((m->md.access & 4) && (address & 2) && m->md.width == 2) {
                m->a.ns += state->cpu_ns;
            } else {
                m->a.Sreads++;
                if (m->md.width == 0) m->a.Sreads++;
                m->a.ns += m->ns_LoadInstr16S;
            }
            if (m->a.ns >= NS_PER_S) m->a.ns -= NS_PER_S, m->a.s++;
            break;
        }
    if (!m) {
         state->ns += state->cpu_ns;
         if (state->ns >= NS_PER_S) state->ns -= NS_PER_S, state->s++;
    }

#ifdef HOURGLASS_RATE
 if( ( state->NumScycles & HOURGLASS_RATE ) == 0 ) {
    armsd_hourglass();
    }
#endif

#ifdef ABORTS
 if (address >= LOWABORT && address < HIGHABORT) {
    ARMul_PREFETCHABORT(address) ;
    return(ARMul_ABORTWORD) ;
    }
 else {
    ARMul_CLEARABORT ;
    }
#endif

 temp = (HostEndian == state->bigendSig)?address:ENDSWAPH(address) ;
 return((ARMword)GetHalfWord(state,temp)) ;
}

/***************************************************************************\
*                 Load 16 bit Instruction, Non Sequential Cycle             *
\***************************************************************************/

ARMword ARMul_LoadInstr16N(ARMul_State *state,ARMword address)
{
    MemDescr *m;
    ARMword temp ;

    state->NumNcycles++ ;
    for (m = state->MemInfoPtr; m != NULL; m = m->next)
        if (address >= m->md.start && address < m->md.limit) {
            m->a.Nreads++;
            if (m->md.width == 0) m->a.Sreads++;
            ADDNS(m, ns_LoadInstr16N);
            break;
        }
    if (!m) {
         state->ns += state->cpu_ns;
         if (state->ns >= NS_PER_S) state->ns -= NS_PER_S, state->s++;
    }

#ifdef ABORTS
 if (address >= LOWABORT && address < HIGHABORT) {
    ARMul_PREFETCHABORT(address) ;
    return(ARMul_ABORTWORD) ;
    }
 else {
    ARMul_CLEARABORT ;
    }
#endif

 temp = (HostEndian == state->bigendSig)?address:ENDSWAPH(address) ;
 return((ARMword)GetHalfWord(state,temp)) ;
 }

/***************************************************************************\
*                        Load Word, Sequential Cycle                        *
\***************************************************************************/

ARMword ARMul_LoadWordS(ARMul_State *state,ARMword address)
{
    MemDescr *m;

    state->NumScycles++ ;
#ifdef RUN_IN_16_BIT_MEMORY_HACK
state->NumFcycles++ ;
#endif
    for (m = state->MemInfoPtr; m != NULL; m = m->next)
        if (address >= m->md.start && address < m->md.limit) {
            m->a.Sreads += 4L >> m->md.width;
            ADDNS(m, ns_LoadWordS);
            break;
        }
    if (!m) {
         state->ns += state->cpu_ns;
         if (state->ns >= NS_PER_S) state->ns -= NS_PER_S, state->s++;
    }

#ifdef ABORTS
 if (address >= LOWABORT && address < HIGHABORT) {
    ARMul_DATAABORT(address) ;
    return(0) ;
    }
 else {
    ARMul_CLEARABORT ;
    }
#endif

 return(GetWord(state,address)) ;
 }

/***************************************************************************\
*                      Load Word, Non Sequential Cycle                      *
\***************************************************************************/

ARMword ARMul_LoadWordN(ARMul_State *state,ARMword address)
{
    MemDescr *m;

    state->NumNcycles++ ;
#ifdef RUN_IN_16_BIT_MEMORY_HACK
state->NumFcycles++ ;
#endif
    for (m = state->MemInfoPtr; m != NULL; m = m->next)
        if (address >= m->md.start && address < m->md.limit) {
            m->a.Nreads++;
            m->a.Sreads += (4L >> m->md.width) - 1;
            ADDNS(m, ns_LoadWordN);
            break;
        }
    if (!m) {
         state->ns += state->cpu_ns;
         if (state->ns >= NS_PER_S) state->ns -= NS_PER_S, state->s++;
    }

#ifdef ABORTS
 if (address >= LOWABORT && address < HIGHABORT) {
    ARMul_DATAABORT(address) ;
    return(0) ;
    }
 else {
    ARMul_CLEARABORT ;
    }
#endif

 return(GetWord(state,address)) ;
 }

/***************************************************************************\
*                     Load Halfword, (Non Sequential Cycle)                *
\***************************************************************************/

ARMword ARMul_LoadHalfWord(ARMul_State *state,ARMword address)
{
    MemDescr *m;
    ARMword temp ;

 state->NumNcycles++ ;
    for (m = state->MemInfoPtr; m != NULL; m = m->next)
        if (address >= m->md.start && address < m->md.limit) {
            m->a.Nreads++;
            if (m->md.width == 0) m->a.Sreads++;
            ADDNS(m, ns_LoadHalfWord);
            break;
        }
    if (!m) {
         state->ns += state->cpu_ns;
         if (state->ns >= NS_PER_S) state->ns -= NS_PER_S, state->s++;
    }

#ifdef ABORTS
 if (address >= LOWABORT && address < HIGHABORT) {
    ARMul_DATAABORT(address) ;
    return(0) ;
    }
 else {
    ARMul_CLEARABORT ;
    }
#endif

 temp = (HostEndian == state->bigendSig)?address:ENDSWAPH(address) ;
 return((ARMword)GetHalfWord(state,temp)) ;
 }

/***************************************************************************\
*                     Load Byte, (Non Sequential Cycle)                     *
\***************************************************************************/

ARMword ARMul_LoadByte(ARMul_State *state,ARMword address)
{
    MemDescr *m;
    ARMword temp ;

 state->NumNcycles++ ;
    for (m = state->MemInfoPtr; m != NULL; m = m->next)
        if (address >= m->md.start && address < m->md.limit) {
            m->a.Nreads++;
            ADDNS(m, ns_LoadByte);
            break;
        }
    if (!m) {
         state->ns += state->cpu_ns;
         if (state->ns >= NS_PER_S) state->ns -= NS_PER_S, state->s++;
    }

#ifdef ABORTS
 if (address >= LOWABORT && address < HIGHABORT) {
    ARMul_DATAABORT(address) ;
    return(0) ;
    }
 else {
    ARMul_CLEARABORT ;
    }
#endif

 temp = (HostEndian == state->bigendSig)?address:ENDSWAP(address) ;
 return((ARMword)GetByte(state,temp)) ;
 }

/***************************************************************************\
*                       Store Word, Sequential Cycle                        *
\***************************************************************************/

void ARMul_StoreWordS(ARMul_State *state,ARMword address, ARMword data)
{
    MemDescr *m;

    state->NumScycles++ ;
#ifdef RUN_IN_16_BIT_MEMORY_HACK
state->NumFcycles++ ;
#endif
    for (m = state->MemInfoPtr; m != NULL; m = m->next)
        if (address >= m->md.start && address < m->md.limit) {
            m->a.Swrites += 4L >> m->md.width;
            ADDNS(m, ns_StoreWordS);
            break;
        }
    if (!m) {
         state->ns += state->cpu_ns;
         if (state->ns >= NS_PER_S) state->ns -= NS_PER_S, state->s++;
    }

#ifdef ABORTS
 if (address >= LOWABORT && address < HIGHABORT) {
    ARMul_DATAABORT(address) ;
    return ;
    }
 else {
    ARMul_CLEARABORT ;
    }
#endif

 PutWord(state,address,data) ;
 }

/***************************************************************************\
*                       Store Word, Non Sequential Cycle                        *
\***************************************************************************/

void ARMul_StoreWordN(ARMul_State *state, ARMword address, ARMword data)
{
    MemDescr *m;

    state->NumNcycles++ ;
#ifdef RUN_IN_16_BIT_MEMORY_HACK
state->NumFcycles++ ;
#endif
    for (m = state->MemInfoPtr; m != NULL; m = m->next)
        if (address >= m->md.start && address < m->md.limit) {
            m->a.Nwrites++;
            m->a.Swrites += (4L >> m->md.width) - 1;
            ADDNS(m, ns_StoreWordN);
            break;
        }
    if (!m) {
         state->ns += state->cpu_ns;
         if (state->ns >= NS_PER_S) state->ns -= NS_PER_S, state->s++;
    }

#ifdef ABORTS
 if (address >= LOWABORT && address < HIGHABORT) {
    ARMul_DATAABORT(address) ;
    return ;
    }
 else {
    ARMul_CLEARABORT ;
    }
#endif

 PutWord(state,address,data) ;
 }

/***************************************************************************\
*                       Store HalfWord, Non Sequential Cycle                *
\***************************************************************************/

void ARMul_StoreHalfWord(ARMul_State *state, ARMword address, ARMword data)
{
    MemDescr *m;
    ARMword temp ;

    state->NumNcycles++ ;
    for (m = state->MemInfoPtr; m != NULL; m = m->next)
        if (address >= m->md.start && address < m->md.limit) {
            m->a.Nwrites++;
            if (m->md.width == 0) m->a.Swrites++;
            ADDNS(m, ns_StoreHalfWord);
            break;
        }
    if (!m) {
         state->ns += state->cpu_ns;
         if (state->ns >= NS_PER_S) state->ns -= NS_PER_S, state->s++;
    }

#ifdef ABORTS
 if (address >= LOWABORT && address < HIGHABORT) {
    ARMul_DATAABORT(address) ;
    return ;
    }
 else {
    ARMul_CLEARABORT ;
    }
#endif

 temp = (HostEndian == state->bigendSig)?address:ENDSWAPH(address) ;
 PutHalfWord(state,temp,data) ;
}

/***************************************************************************\
*                    Store Byte, (Non Sequential Cycle)                     *
\***************************************************************************/

void ARMul_StoreByte(ARMul_State *state, ARMword address, ARMword data)
{
    MemDescr *m;

    ARMword temp ;

    state->NumNcycles++ ;
    for (m = state->MemInfoPtr; m != NULL; m = m->next)
        if (address >= m->md.start && address < m->md.limit) {
            m->a.Nwrites++;
            ADDNS(m, ns_StoreByte);
            break;
        }
    if (!m) {
         state->ns += state->cpu_ns;
         if (state->ns >= NS_PER_S) state->ns -= NS_PER_S, state->s++;
    }

#ifdef VALIDATE
 if (address == TUBE) {
    if (data == 4)
       state->Emulate = FALSE ;
    else
       (void)putc((char)data,stderr) ; /* Write Char */
    return ;
    }
#endif

#ifdef ABORTS
 if (address >= LOWABORT && address < HIGHABORT) {
    ARMul_DATAABORT(address) ;
    return ;
    }
 else {
    ARMul_CLEARABORT ;
    }
#endif

 temp = (HostEndian == state->bigendSig)?address:ENDSWAP(address) ;
 PutByte(state,temp,(unsigned char)data) ;
 }

/***************************************************************************\
*                   Swap Word, (Two Non Sequential Cycles)                  *
\***************************************************************************/

ARMword ARMul_SwapWord(ARMul_State *state, ARMword address, ARMword data)
{
    MemDescr *m;
    ARMword temp ;

    state->NumNcycles+=2;
    for (m = state->MemInfoPtr; m != NULL; m = m->next)
        if (address >= m->md.start && address < m->md.limit) {
            m->a.Nreads++;
            m->a.Nwrites++;
            m->a.Sreads += (4L >> m->md.width) - 1;
            m->a.Swrites += (4L >> m->md.width) - 1;
            ADDNS(m, ns_SwapWord);
            break;
        }
    if (!m) {
         state->ns += state->cpu_ns;
         if (state->ns >= NS_PER_S) state->ns -= NS_PER_S, state->s++;
    }

#ifdef ABORTS
 if (address >= LOWABORT && address < HIGHABORT) {
    ARMul_DATAABORT(address) ;
    return(ARMul_ABORTWORD) ;
    }
 else {
    ARMul_CLEARABORT ;
    }
#endif

 temp = GetWord(state,address) ;
 PutWord(state,address,data) ;
 return(temp) ;
}

/***************************************************************************\
*                   Swap Byte, (Two Non Sequential Cycles)                  *
\***************************************************************************/

ARMword ARMul_SwapByte(ARMul_State *state, ARMword address, ARMword data)
{
    MemDescr *m;
    ARMword temp ;

    state->NumNcycles+=2;
    for (m = state->MemInfoPtr; m != NULL; m = m->next)
        if (address >= m->md.start && address < m->md.limit) {
            m->a.Nreads++;
            m->a.Nwrites++;
            ADDNS(m, ns_SwapByte);
            break;
        }
    if (!m) {
         state->ns += state->cpu_ns;
         if (state->ns >= NS_PER_S) state->ns -= NS_PER_S, state->s++;
    }

#ifdef ABORTS
 if (address >= LOWABORT && address < HIGHABORT) {
    ARMul_DATAABORT(address) ;
    return(ARMul_ABORTWORD) ;
    }
 else {
    ARMul_CLEARABORT ;
    }
#endif

 temp = ARMul_LoadByte(state,address) ;
 ARMul_StoreByte(state,address,data) ;
 return(temp) ;
 }

/***************************************************************************\
*                             Count I Cycles                                *
\***************************************************************************/

void ARMul_Icycles(ARMul_State *state, unsigned number, ARMword address)
{
 state->NumIcycles += number ;
 state->ns += state->cpu_ns * number;
 if (state->ns >= NS_PER_S) state->ns -= NS_PER_S, state->s++;
 ARMul_CLEARABORT ;
 }

/***************************************************************************\
*                             Count C Cycles                                *
\***************************************************************************/

void ARMul_Ccycles(ARMul_State *state, unsigned number, ARMword address)
{
 state->NumCcycles += number ;
 state->ns += state->cpu_ns * number;
 if (state->ns >= NS_PER_S) state->ns -= NS_PER_S, state->s++;
 ARMul_CLEARABORT ;
 }

/***************************************************************************\
*                      Read Word (but don't tell anyone!)                   *
\***************************************************************************/

ARMword ARMul_ReadWord(ARMul_State *state, ARMword address)
{
#ifdef ABORTS
 if (address >= LOWABORT && address < HIGHABORT) {
    ARMul_DATAABORT(address) ;
    return(ARMul_ABORTWORD) ;
    }
 else {
    ARMul_CLEARABORT ;
    }
#endif

 return(GetWord(state,address)) ;
 }

/***************************************************************************\
*                      Read HalfWord (but don't tell anyone!)              *
\***************************************************************************/

ARMword ARMul_ReadHalfWord(ARMul_State *state, ARMword address)
{ARMword temp;
#ifdef ABORTS
 if (address >= LOWABORT && address < HIGHABORT) {
    ARMul_DATAABORT(address) ;
    return(ARMul_ABORTWORD) ;
    }
 else {
    ARMul_CLEARABORT ;
    }
#endif

 temp = (HostEndian == state->bigendSig)?address:ENDSWAPH(address) ;
 return((ARMword)GetHalfWord(state,temp)) ;
 }

/***************************************************************************\
*                      Read Byte (but don't tell anyone!)                   *
\***************************************************************************/

ARMword ARMul_ReadByte(ARMul_State *state, ARMword address)
{ARMword temp ;

#ifdef ABORTS
 if (address >= LOWABORT && address < HIGHABORT) {
    ARMul_DATAABORT(address) ;
    return(ARMul_ABORTWORD) ;
    }
 else {
    ARMul_CLEARABORT ;
    }
#endif

 temp = (HostEndian == state->bigendSig)?address:ENDSWAP(address) ;
 return((ARMword)GetByte(state,temp)) ;
 }

/***************************************************************************\
*                     Write Word (but don't tell anyone!)                   *
\***************************************************************************/

void ARMul_WriteWord(ARMul_State *state, ARMword address, ARMword data)
{
#ifdef ABORTS
 if (address >= LOWABORT && address < HIGHABORT) {
    ARMul_DATAABORT(address) ;
    return ;
    }
 else {
    ARMul_CLEARABORT ;
    }
#endif

 PutWord(state,address,data) ;
 }

/***************************************************************************\
*                     Write HalfWord (but don't tell anyone!)                   *
\***************************************************************************/

void ARMul_WriteHalfWord(ARMul_State *state, ARMword address, ARMword data)
{ARMword temp;
#ifdef ABORTS
 if (address >= LOWABORT && address < HIGHABORT) {
    ARMul_DATAABORT(address) ;
    return ;
    }
 else {
    ARMul_CLEARABORT ;
    }
#endif

 temp = (HostEndian == state->bigendSig)?address:ENDSWAPH(address) ;
 PutHalfWord(state,temp,data) ;
 }

/***************************************************************************\
*                     Write Byte (but don't tell anyone!)                   *
\***************************************************************************/

void ARMul_WriteByte(ARMul_State *state, ARMword address, ARMword data)
{ARMword temp ;

#ifdef ABORTS
 if (address >= LOWABORT && address < HIGHABORT) {
    ARMul_DATAABORT(address) ;
    return ;
    }
 else {
    ARMul_CLEARABORT ;
    }
#endif

 temp = (HostEndian == state->bigendSig)?address:ENDSWAP(address) ;
 PutByte(state,temp,(unsigned char)data) ;
 }
