/***************************************************************************\
* armdefs.h                                                                 *
* ARMulator II environment definitions.                                     *
* Copyright (C) Advanced RISC Machines Limited. All rights reserved.        *
* Written by Dave Jaggar.                                                   *
\***************************************************************************/

/* RCS $Revision: 1.30.2.1 $
 * Checkin $Date: 1995/05/14 13:00:08 $
 * Revising $Author: plg $
 */

#include <stdio.h>
#include <stdlib.h>

#define LOW 0
#define HIGH 1
#define LOWHIGH 1
#define HIGHLOW 2

#include "host.h"

typedef unsigned32 ARMword; /* must be 32 bits wide */

typedef struct ARMul_State ARMul_State ;
typedef struct MemDescr MemDescr;

typedef struct EventNode EventNode;

typedef unsigned ARMul_CPInits(ARMul_State *state) ;
typedef unsigned ARMul_CPExits(ARMul_State *state) ;
typedef unsigned ARMul_LDCs(ARMul_State *state,unsigned type,ARMword instr,ARMword value) ;
typedef unsigned ARMul_STCs(ARMul_State *state,unsigned type,ARMword instr,ARMword *value) ;
typedef unsigned ARMul_MRCs(ARMul_State *state,unsigned type,ARMword instr,ARMword *value) ;
typedef unsigned ARMul_MCRs(ARMul_State *state,unsigned type,ARMword instr,ARMword value) ;
typedef unsigned ARMul_CDPs(ARMul_State *state,unsigned type,ARMword instr) ;
typedef unsigned ARMul_CPReads(ARMul_State *state,unsigned reg,ARMword *value) ;
typedef unsigned ARMul_CPWrites(ARMul_State *state,unsigned reg,ARMword const *value) ;
typedef void ARMul_ModeChangeUpcall(ARMul_State *state, ARMword oldmode, ARMword newmode);

#include "dbg_stat.h"

struct MemDescr {
    MemDescr *next;
    RDI_MemDescr md;
    RDI_MemAccessStats a;
    int ns_LoadInstrS;
    int ns_LoadInstrN;
    int ns_LoadInstr16S;
    int ns_LoadInstr16N;
    int ns_LoadWordS;
    int ns_LoadWordN;
    int ns_LoadHalfWord;
    int ns_LoadByte;
    int ns_StoreWordS;
    int ns_StoreWordN;
    int ns_StoreHalfWord;
    int ns_StoreByte;
    int ns_SwapWord;
    int ns_SwapByte;
};

struct ARMul_State {
   unsigned Processor ; /* type of ARM we are emulating */
   unsigned Instruction_set ;      /* 1=16 bits or 0=32, added for thumb */
   ARMword Emulate ; /* to start and stop emulation */
   unsigned EndCondition ; /* reason for stopping */
   unsigned ErrorCode ; /* type of illegal instruction */
   ARMword Reg[16] ; /* the current register file */
   ARMword RegBank[7][16] ; /* all the registers */
   ARMword Cpsr ; /* the current psr */
   ARMword Spsr[7] ; /* the exception psr's */
   ARMword NFlag, ZFlag, CFlag, VFlag, IFFlags ; /* dummy flags for speed */
   ARMword Bank ; /* the current register bank */
   ARMword Mode ; /* the current mode */
   ARMword instr, pc, temp ; /* saved register state */
   ARMword loaded, decoded ; /* saved pipeline state */
   unsigned long NumScycles,
                 NumNcycles,
                 NumIcycles,
                 NumCcycles,
                 NumFcycles ; /* emulated cycles used */
   unsigned long NumInstrs ; /* the number of instructions executed */
   unsigned long cpu_ns;
   unsigned long ns, s;
   unsigned NextInstr ;
   unsigned VectorCatch ; /* caught exception mask */
   unsigned CallDebug ; /* set to call the debugger */
   unsigned CanWatch ;  /* set by memory interface if its willing to suffer the
                           overhead of checking for watchpoints on each memory
                           access */
   unsigned MemReadDebug, MemWriteDebug ;
   unsigned long StopHandle ;

   unsigned char *MemDataPtr ; /* admin data */
   MemDescr *MemInfoPtr ;
   unsigned char *MemInPtr ; /* the Data In bus */
   unsigned char *MemOutPtr ; /* the Data Out bus (which you may not need */
   unsigned char *MemSparePtr ; /* extra space */
   ARMword MemSize ;

   unsigned char *OSptr ; /* OS Handle */
   char *CommandLine ; /* Command Line from ARMsd */

   ARMul_CPInits *CPInit[16] ; /* coprocessor initialisers */
   ARMul_CPExits *CPExit[16] ; /* coprocessor finalisers */
   ARMul_LDCs *LDC[16] ; /* LDC instruction */
   ARMul_STCs *STC[16] ; /* STC instruction */
   ARMul_MRCs *MRC[16] ; /* MRC instruction */
   ARMul_MCRs *MCR[16] ; /* MCR instruction */
   ARMul_CDPs *CDP[16] ; /* CDP instruction */
   ARMul_CPReads *CPRead[16] ; /* Read CP register */
   ARMul_CPWrites *CPWrite[16] ; /* Write CP register */
   unsigned char *CPData[16] ; /* Coprocessor data */
   unsigned char const *CPRegWords[16] ; /* map of coprocessor register sizes */

   unsigned EventSet ; /* the number of events in the queue */
   unsigned long Now ; /* time to the nearest cycle */
   EventNode **EventPtr ; /* the event list */

   unsigned Exception ; /* enable the next four values */
   unsigned Debug ; /* show instructions as they are executed */
   unsigned NresetSig ; /* reset the processor */
   unsigned NfiqSig ;
   unsigned NirqSig ;

   unsigned abortSig ;
   unsigned NtransSig ;
   unsigned bigendSig ;
   unsigned prog32Sig ;
   unsigned data32Sig ;
   unsigned lateabtSig ;
   ARMword Vector ; /* synthesize aborts in cycle modes */
   ARMword Aborted ; /* sticky flag for aborts */
   ARMword Reseted ; /* sticky flag for Reset */
   ARMword Inted, LastInted ; /* sticky flags for interrupts */
   ARMword Base ; /* extra hand for base writeback */
   ARMword AbortAddr ; /* to keep track of Prefetch aborts */

#ifdef CYCLEBASED
   unsigned NextCycle ; /* the position in the state machine */
   ARMword TempReg[16] ; /* undo aborts with these */
   unsigned NumCycles ; /* counter for state iterations */
   ARMword Temp ; /* sometimes you just need an extra hand */

   ARMword addrBus ;
   ARMword datainBus ;
   ARMword dataoutBus ;
   unsigned mas0Sig;     /* Thumb memory access size mas[0] */
   unsigned NbwSig ;     /* = mas[1] */
   unsigned NrwSig ;
   unsigned seqSig ;
   unsigned NmreqSig ;
   unsigned NopcSig ;
   unsigned NexecSig ;
   unsigned lockSig ;
   unsigned NmBus ;
   unsigned NcpiSig ;
   unsigned cpaSig ;
   unsigned cpbSig ;
   unsigned doutenSig ; /* synthesize NENOUT with this plus DBE */

   unsigned LSCActive ; /* LDC or STC is happening */
   ARMword addr01Bus ; /* used to fake the bottom two bits of address */
   unsigned LastCycle ; /* so you can know what you did last */
   ARMword NextaddrBus ; /* extra hands for the coprocessor fix up routine */
   unsigned NextNrwSig ;
   unsigned NextseqSig ;
   unsigned NextNmreqSig ;
   unsigned NextNopcSig ;
   ARMword PrevaddrBus ;
   unsigned PrevNrwSig ;
   ARMword OldBase ; /* undo aborts with this */

   unsigned mclkSig ;
   unsigned NwaitSig ;
   unsigned tdiSig, tdoSig, tmsSig, NtrstSig ; /* Boundary scan interface */
   char *TAPDataPtr ;
   struct ARMul_State *Core ; /* used when in BS test mode */
   ARMword BSID ; /* the boundary scan identification number */
#endif

   const struct Dbg_HostosInterface *hostif;

   ARMul_ModeChangeUpcall *ModeChange;
   unsigned clearAborts; /* Set this <>0 to have aborts cleared for you */

   unsigned fpe;                /* 0 if fpe shouldn't be/isn't loaded */
 } ;

#define ResetPin NresetSig
#define FIQPin NfiqSig
#define IRQPin NirqSig
#define AbortPin abortSig
#define TransPin NtransSig
#define BigEndPin bigendSig
#define Prog32Pin prog32Sig
#define Data32Pin data32Sig
#define LateAbortPin lateabtSig
#ifdef CYCLEBASED
#define AddrBus addrBus
#define DataInBus datainBus
#define DataOutBus dataoutBus
#define BWPin NbwSig
#define RWPin NrwSig
#define SeqPin seqSig
#define MreqPin NmreqSig
#define OpcPin NopcSig
#define ExecPin NexecSig
#define LockPin lockSig
#define ModePins NmBus
#define CPIPin NcpiSig
#define CPAPin cpaSig
#define CPBPin cpbSig
#define TDIPin tdiSig
#define TDOPin tdoSig
#define TMSPin tmsSig
#define NTRSTPin NtrstSig
#endif

/***************************************************************************\
*                        Types of ARM we know about                         *
\***************************************************************************/

/* The bitflags */
#define ARM_Fix26_Prop   0x01
#define ARM_Nexec_Prop   0x02
#define ARM_Abort7_Prop  0x04
#define ARM_Mult64_Prop  0x08
#define ARM_Debug_Prop   0x10
#define ARM_Isync_Prop   ARM_Debug_Prop
#define ARM_Lock_Prop    0x20
#define ARM_Halfword_Prop 0x40
#define ARM_Code16_Prop  0x80
#define ARM_System32_Prop 0x100

/* ARM2 family */
#define ARM2    (ARM_Fix26_Prop)
#define ARM2as  ARM2
#define ARM61   ARM2
#define ARM3    ARM2

/* ARM6 family */
#define ARM6    (ARM_Lock_Prop)
#define ARM60   ARM6
#define ARM600  ARM6
#define ARM610  ARM6
#define ARM620  ARM6

/* ARM7 family */
#define ARM7    (ARM_Nexec_Prop | ARM_Abort7_Prop)
#define ARM70   ARM7
#define ARM700  ARM7
/* ...with debug */
#define ARM7d   (ARM7 | ARM_Debug_Prop)
#define ARM70d  ARM7d
/* ...with extended multiply */
#define ARM7dm  (ARM7d | ARM_Mult64_Prop)
#define ARM70dm ARM7dm
/* ...with halfwords and 16 bit instruction set and system mode */
#define ARM7tdm (ARM7dm | ARM_Halfword_Prop | ARM_Code16_Prop | ARM_System32_Prop)

/***************************************************************************\
*                   Macros to extract instruction fields                    *
\***************************************************************************/

#define BIT(n) ( (ARMword)(instr>>(n))&1)   /* bit n of instruction */
#define BITS(m,n) ( (ARMword)(instr<<(31-(n))) >> ((31-(n))+(m)) ) /* bits m to n of instr */
#define TOPBITS(n) (instr >> (n)) /* bits 31 to n of instr */

/***************************************************************************\
*                      The hardware vector addresses                        *
\***************************************************************************/

#define ARMResetV 0L
#define ARMUndefinedInstrV 4L
#define ARMSWIV 8L
#define ARMPrefetchAbortV 12L
#define ARMDataAbortV 16L
#define ARMAddrExceptnV 20L
#define ARMIRQV 24L
#define ARMFIQV 28L
#define ARMErrorV 32L /* This is an offset, not an address ! */

#define ARMul_ResetV ARMResetV
#define ARMul_UndefinedInstrV ARMUndefinedInstrV
#define ARMul_SWIV ARMSWIV
#define ARMul_PrefetchAbortV ARMPrefetchAbortV
#define ARMul_DataAbortV ARMDataAbortV
#define ARMul_AddrExceptnV ARMAddrExceptnV
#define ARMul_IRQV ARMIRQV
#define ARMul_FIQV ARMFIQV

/***************************************************************************\
*                          Mode and Bank Constants                          *
\***************************************************************************/

#define USER26MODE 0L
#define FIQ26MODE 1L
#define IRQ26MODE 2L
#define SVC26MODE 3L
#define USER32MODE 16L
#define FIQ32MODE 17L
#define IRQ32MODE 18L
#define SVC32MODE 19L
#define ABORT32MODE 23L
#define UNDEF32MODE 27L
#define SYSTEM32MODE 31L

#define ARM32BITMODE (state->Mode > 3)
#define ARM26BITMODE (state->Mode <= 3)
#define ARMMODE (state->Mode)
#define ARMul_MODEBITS 0x1fL
#define ARMul_MODE32BIT ARM32BITMODE
#define ARMul_MODE26BIT ARM26BITMODE

#define USERBANK 0
#define FIQBANK 1
#define IRQBANK 2
#define SVCBANK 3
#define ABORTBANK 4
#define UNDEFBANK 5
#define DUMMYBANK 6

/***************************************************************************\
*                  Definitions of things in the emulator                    *
\***************************************************************************/

extern void ARMul_EmulateInit(void) ;
extern ARMul_State *ARMul_NewState(void) ;
extern void ARMul_Reset(ARMul_State *state) ;
extern void ARMul_SelectProcessor(ARMul_State *state, unsigned processor) ;
#ifdef CYCLEBASED
extern ARMword ARMul_DoCycle(ARMul_State *state) ;
extern unsigned ARMul_DoCoPro(ARMul_State *state) ;
#else
extern ARMword ARMul_DoProg(ARMul_State *state) ;
extern ARMword ARMul_DoInstr(ARMul_State *state) ;
#endif

/***************************************************************************\
*                Definitions of things for event handling                   *
\***************************************************************************/

extern void ARMul_ScheduleEvent(ARMul_State *state, unsigned long delay, unsigned (*func)() ) ;
extern void ARMul_InvokeEvent(ARMul_State *state) ;
extern unsigned long ARMul_Time(ARMul_State *state) ;

/***************************************************************************\
*                          Useful support routines                          *
\***************************************************************************/

extern ARMword ARMul_GetReg(ARMul_State *state, unsigned mode, unsigned reg) ;
extern void ARMul_SetReg(ARMul_State *state, unsigned mode, unsigned reg, ARMword value) ;
extern ARMword ARMul_GetPC(ARMul_State *state) ;
extern ARMword ARMul_GetNextPC(ARMul_State *state) ;
extern void ARMul_SetPC(ARMul_State *state, ARMword value) ;
extern ARMword ARMul_GetR15(ARMul_State *state) ;
extern void ARMul_SetR15(ARMul_State *state, ARMword value) ;

extern ARMword ARMul_GetCPSR(ARMul_State *state) ;
extern void ARMul_SetCPSR(ARMul_State *state, ARMword value) ;
extern ARMword ARMul_GetSPSR(ARMul_State *state, ARMword mode) ;
extern void ARMul_SetSPSR(ARMul_State *state, ARMword mode, ARMword value) ;

/***************************************************************************\
*                  Definitions of things to handle aborts                   *
\***************************************************************************/

extern void ARMul_Abort(ARMul_State *state, ARMword address) ;
#define ARMul_ABORTWORD 0xefffffff /* SWI -1 */
#ifdef CODE16
#define ARMul_PREFETCHABORT(address) \
     if (state->AbortAddr == 1)      \
       state->AbortAddr = (address & (INSTRUCTION16STATE ? ~1L : ~3L))
#else
#define ARMul_PREFETCHABORT(address) if (state->AbortAddr == 1) \
                                        state->AbortAddr = (address & ~3L)
#endif
#define ARMul_DATAABORT(address) state->abortSig = HIGH ; \
                                 state->Aborted = ARMul_DataAbortV ;
#define ARMul_CLEARABORT state->abortSig = LOW

#ifndef CYCLEBASED

/***************************************************************************\
*              Definitions of things in the memory interface                *
\***************************************************************************/

extern unsigned ARMul_MemoryInit(ARMul_State *state,unsigned long initmemsize) ;
extern void ARMul_MemoryExit(ARMul_State *state) ;

extern ARMword ARMul_LoadInstrS(ARMul_State *state,ARMword address) ;
extern ARMword ARMul_LoadInstrN(ARMul_State *state,ARMword address) ;

extern ARMword ARMul_LoadWordS(ARMul_State *state,ARMword address) ;
extern ARMword ARMul_LoadWordN(ARMul_State *state,ARMword address) ;
extern ARMword ARMul_LoadByte(ARMul_State *state,ARMword address) ;

extern void ARMul_StoreWordS(ARMul_State *state,ARMword address, ARMword data) ;
extern void ARMul_StoreWordN(ARMul_State *state,ARMword address, ARMword data) ;
extern void ARMul_StoreByte(ARMul_State *state,ARMword address, ARMword data) ;

extern ARMword ARMul_LoadInstr16S(ARMul_State *state,ARMword address) ;
extern ARMword ARMul_LoadInstr16N(ARMul_State *state,ARMword address) ;
extern ARMword ARMul_LoadHalfWord(ARMul_State *state,ARMword address) ;
extern void ARMul_StoreHalfWord(ARMul_State *state,ARMword address, ARMword data) ;
extern ARMword ARMul_ReadHalfWord(ARMul_State *state,ARMword address) ;
extern void ARMul_WriteHalfWord(ARMul_State *state,ARMword address, ARMword data) ;

extern ARMword ARMul_SwapWord(ARMul_State *state,ARMword address, ARMword data) ;
extern ARMword ARMul_SwapByte(ARMul_State *state,ARMword address, ARMword data) ;

extern void ARMul_Icycles(ARMul_State *state,unsigned number, ARMword address) ;
extern void ARMul_Ccycles(ARMul_State *state,unsigned number, ARMword address) ;

extern ARMword ARMul_ReadWord(ARMul_State *state,ARMword address) ;
extern ARMword ARMul_ReadByte(ARMul_State *state,ARMword address) ;
extern void ARMul_WriteWord(ARMul_State *state,ARMword address, ARMword data) ;
extern void ARMul_WriteByte(ARMul_State *state,ARMword address, ARMword data) ;

extern ARMword ARMul_MemAccess(ARMul_State *state,ARMword,ARMword,ARMword,
             ARMword,ARMword,ARMword,ARMword,ARMword,ARMword,ARMword,ARMword) ;

/***************************************************************************\
*            Definitions of things in the co-processor interface            *
\***************************************************************************/

#define ARMul_FIRST 0
#define ARMul_TRANSFER 1
#define ARMul_BUSY 2
#define ARMul_DATA 3
#define ARMul_INTERRUPT 4
#define ARMul_DONE 0
#define ARMul_CANT 1
#define ARMul_INC 3

extern unsigned ARMul_CoProInit(ARMul_State *state) ;
extern void ARMul_CoProExit(ARMul_State *state) ;
extern void ARMul_CoProAttach(ARMul_State *state, unsigned number,
                              ARMul_CPInits *init, ARMul_CPExits *exit,
                              ARMul_LDCs *ldc, ARMul_STCs *stc,
                              ARMul_MRCs *mrc, ARMul_MCRs *mcr,
                              ARMul_CDPs *cdp,
                              ARMul_CPReads *read, ARMul_CPWrites *write,
                              unsigned char const *regwords) ;
extern void ARMul_CoProDetach(ARMul_State *state, unsigned number) ;

/***************************************************************************\
*               Definitions of things in the host environment               *
\***************************************************************************/

extern unsigned ARMul_OSInit(ARMul_State *state) ;
extern void ARMul_OSExit(ARMul_State *state) ;
extern unsigned ARMul_OSHandleSWI(ARMul_State *state,ARMword number) ;
extern ARMword ARMul_OSLastErrorP(ARMul_State *state) ;
#else
char *ARMul_Disass(ARMword instr, ARMword address, ARMword cpsr) ;
extern char *ARMul_ErrorMess(ARMul_State *state,unsigned number) ;
#define UNDEF_BadInstruction 132
#define UNDEF_IllInstruction 133
#endif /* CYCLEBASED */

extern ARMword ARMul_Debug(ARMul_State *state, ARMword pc, ARMword instr) ;
extern void ARMul_CheckWatch(ARMul_State *state, ARMword addr, int access) ;

extern unsigned ARMul_OSException(ARMul_State *state, ARMword vector, ARMword pc) ;
extern void ARMul_DebugPrint(ARMul_State *state, const char *format, ...);
extern void ARMul_DebugPause(ARMul_State *state);
extern void ARMul_ConsolePrint(ARMul_State *state, const char *format, ...);

/* IF the macro HOURGLASS_RATE is defined, then you must provide this fn */
#ifdef HOURGLASS_RATE
extern void armsd_hourglass(void);
#endif

/***************************************************************************\
*               ARM / THUMB differences                                    *
\***************************************************************************/

#define INSTRUCTION32SIZE (4)
#define INSTRUCTION32 (0)

#ifdef CODE16
/* state->Instruction_set values */

#define INSTRUCTION16 (1)

#define INSTRUCTION16STATE (state->Instruction_set == INSTRUCTION16)

#define INSTRUCTION16SIZE (2)
#define INSTRUCTIONSIZE (INSTRUCTION16STATE ? INSTRUCTION16SIZE : INSTRUCTION32SIZE)

/* bits in registers */

#define BXINSTRUCTIONSETBITS (0x1)
#define CPSRINSTRUCTIONSETBITPOSN (5)
#define CPSRINSTRUCTIONSETBITS (0x1 << CPSRINSTRUCTIONSETBITPOSN)

#define BXINSTRUCTIONSET(r) ((r & BXINSTRUCTIONSETBITS) ? INSTRUCTION16 : INSTRUCTION32)
#define CPSRINSTRUCTIONSET(r) ((r & CPSRINSTRUCTIONSETBITS) ? INSTRUCTION16 : INSTRUCTION32)
#define CPSRINSTRUCTIONSIZE(r) ((r & CPSRINSTRUCTIONSETBITS) ? INSTRUCTION16SIZE : INSTRUCTION32SIZE)

#else
#define INSTRUCTIONSIZE INSTRUCTION32SIZE
#define CPSRINSTRUCTIONSET(r) (INSTRUCTION32)
#define CPSRINSTRUCTIONSIZE(r) (INSTRUCTION32SIZE)
#endif
