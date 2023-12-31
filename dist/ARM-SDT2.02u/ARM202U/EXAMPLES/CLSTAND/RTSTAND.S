; Issue: 0.03/23-Feb-93
;
; Purpose: Minimal, standalone, C-library kernel.
;
; Copyright (C) 1993 Advanced RISC Machines Limited. All rights reserved.
;
; Advanced RISC Machines Limited does not assume any liability arising out
; of this program or use thereof neither does it convey any licence under
; its intellectual property rights.
;
; Conditions of use:
;
; The terms and conditions under which this software is supplied to you and
; under which you may use it are described in your licence agreement with
; your supplier.
;
;----------------------------------------------------------------------------;
; ABOUT THIS CODE                                                            ;
;                                                                            ;
; This code shows you how to write your own minimal, standalone, run-time    ;
; support system for code compiled by Advanced RISC Machines's C Compiler.   ;
; It can be assembled using Advanced RISC Machines's ARM assembler (armasm)  ;
; or any assembler comaptible with it.                                       ;
;                                                                            ;
; This example code has been written to run under Advanced RISC Machines's   ;
; ARM emulation system (ARMulator). It can also run without modification     ;
; under Acorm Computer's "RISC OS" operating system for its ARM-based        ;
; personal workstations.                                                     ;
;                                                                            ;
; In fact, this code depends hardly at all on its target environment and is  ;
; designed to be very easy to adapt to your particular ARM-based system.     ;
; You can expect it to take about a couple of hours to re-target.            ;
;                                                                            ;
; Much of the code below is generic to the ARM processor and is completely   ;
; independent of your ARM-based hardware or any operating system kernel that ;
; may run on it. To get going, you need write only 4 simple fns.             ;
;                                                                            ;
; WHAT THIS CODE PROVIDES:                                                   ;
;                                                                            ;
;  - Example, executable implementations (for the ARMulator) of the few      ;
;    simple functions you need to implement to customise this code to your   ;
;    environment. These include:                                             ;
;      - setting up the initial stack and heap and calling main (__main)     ;
;      - program termination (__rt_exit)                                     ;
;      - determining FP instruction-set availability (__rt_fpavailable)      ;
;                                                                            ;
;  - Functions to help with heap allocation, stack-limit checking, setjmp    ;
;    and longjmp. These may need to be customised for your environment,      ;
;    but can almost certainly be used as-is in a first re-targetting.        ;
;                                                                            ;
;  - Fully 'rolled' divide (and remainder) functions.                        ;
;                                                                            ;
; WHAT THIS CODE DOES NOT PROVIDE                                            ;
;                                                                            ;
;  - Support for handling traps, faults, escapes, exceptions or interrupts.  ;
;                                                                            ;
;  - A way to print to the debugging channel (use in line SWIs)              ;
;                                                                            ;
;----------------------------------------------------------------------------;

;----------------------------------------------------------------------------;
; The following constant is the Top Of Memory - Adjust this for your system  ;
;----------------------------------------------------------------------------;

TopOfMemory           EQU  0x80000    ; 512Kb

;----------------------------------------------------------------------------;
; Things you may wish to tune, but which you don't need to alter, follow.    ;
;----------------------------------------------------------------------------;

DefaultStackSize      EQU  4*1024     ; The stack starts of this big unless
                                      ; over-ridden by __root_stack_size.

DefaultStackIncrement EQU  1*1024     ; At each overflow it grows by at
                                      ; at least this many bytes.

StackSlop             EQU  512        ; sl is kept this far above the real
                                      ; stack low-water mark. NOTE: MUST be
                                      ; >= 256 or the compiled limit checks
                                      ; will be invalidated.

MinHeapIncrement      EQU  256        ; Min number of WORDS to extend the
                                      ; heap by on calling __rt_alloc.

  GBLL EnsureNoFPSupport
EnsureNoFPSupport     SETL {FALSE}    ; If {TRUE} then the availability of
                                      ; Floating Point Support is ignored.
                                      ; If {FALSE} then FP availability is
                                      ; checked for.
                                      ; Setting to {TRUE} saves a little
                                      ; space.

;----------------------------------------------------------------------------;
; Symbols defined in other, separately-assembled modules, must be IMPORTed.  ;
; We import them WEAKly so that they need not be defined.                    ;
;----------------------------------------------------------------------------;

  IF EnsureNoFPSupport = {FALSE}
        IMPORT  |__fp_initialise|, WEAK
        IMPORT  |__fp_finalise|, WEAK
  ENDIF

;----------------------------------------------------------------------------;
; The existence of __fp_initialise (imported WEAKly) indicates that floating ;
; point support code (or the access stub thereof) has been linked with this  ;
; application. If you wish to load the FP support code separately, you may   ;
; want to define some other mechanism for detecting the presence/absence of  ;
; floating point support. Note that setjmp and longjmp must know whether the ;
; floating-point instruction set is supported.                               ;
; __fp_initialise is called by __main and __fp_finalise is called by _exit.  ;
;----------------------------------------------------------------------------;

        IMPORT  |__root_stack_size|, WEAK

;----------------------------------------------------------------------------;
; If __root_stack_size, also imported WEAKly, exists, the value it addresses ;
; is used as the initial size of the stack. It can be defined in your C      ;
; program as, e.g. int __root_stack_size = 10000; /* 10KB initial stack */   ;
;----------------------------------------------------------------------------;

        IMPORT  |__err_handler|, WEAK

;----------------------------------------------------------------------------;
; If __err_handler exists, errors are passed to it; otherwise, we print a    ;
; simple diagnostic message and exit.                                        ;
;----------------------------------------------------------------------------;

        IMPORT  |Image$$RW$$Limit|

;----------------------------------------------------------------------------;
; Image$$RW$$Limit is a linker-created symbol marking the end of the image.  ;
; Its value is used as the heap base.                                        ;
;----------------------------------------------------------------------------;

        IMPORT  main

;----------------------------------------------------------------------------;
; The symbol main identifies the C function entered from this code.          ;
;----------------------------------------------------------------------------;

;----------------------------------------------------------------------------;
; THE MEMORY MODEL ASSUMED BY THIS IMPLEMENTATION                            ;
;                                                                            ;
;       +----------------+    <--- top of memory (high address)              ;
;       | Stack space    |                                                   ;
;       |................|    <--- stack pointer (sp)                        ;
;       | Free stack     |                                                   ;
;       |................|    <--- stack limit pointer (sl)                  ;
;       +----------------+    <--- stack low-water mark (sl - StackSlop)     ;
;       |                |                                                   ;
;       | Unused memory  |                                                   ;  
;       |                |                                                   ;
;       +----------------+    <--- top of heap (HeapLimit)                   ;
;       |                |                                                   ;
;       | Heap space     |                                                   ;
;       |                |                                                   ;
;       +----------------+    <--- top of application (Image$$RW$$Limit)     ;
;       | Static data    |  }                                                ;
;       |................|  } the application's memory image                 ;
;       | Code           |  }                                                ;
;       +----------------+    <--- application load address                  ;
;----------------------------------------------------------------------------;

;----------------------------------------------------------------------------;
; Now the symbols we define and EXPORT from this this module.                ;
;----------------------------------------------------------------------------;
; First, symbols identifying the four functions you have to implement to     ;
; make this run-time kernel work on your hardware.                           ;
;----------------------------------------------------------------------------;

        EXPORT  |__main|
        EXPORT  |__rt_exit|
        EXPORT  |__rt_fpavailable|
        EXPORT  |__rt_trap|

;----------------------------------------------------------------------------;
; Then some simple support for C heap management. It interacts with stack-   ;
; limit checking but should require no attention in a first re-targetting.   ;
;----------------------------------------------------------------------------;

        EXPORT  |__rt_alloc|

;----------------------------------------------------------------------------;
; Next, optional support for C stack-limit checking. This code should need   ;
; no attention in a first re-targetting.                                     ;
;----------------------------------------------------------------------------;

        EXPORT  |__rt_stkovf_split_small| ; veneer
        EXPORT  |__rt_stkovf_split_big|

;----------------------------------------------------------------------------;
; Then two C-specific functions which should require no attention in a first ;
; re-targetting. Note that they depend on __rt_fpavailable.                  ;
;----------------------------------------------------------------------------;

        EXPORT  |setjmp|
        EXPORT  |longjmp|

;----------------------------------------------------------------------------;
; And, finally, generic ARM functions, referred to by the C compiler.        ;
; You should not need to alter any of these unless you wish to incorporate   ;
; them in your operating system kernel. See also later comments.             ;
;----------------------------------------------------------------------------;

        EXPORT  |__rt_udiv|
        EXPORT  |__rt_udiv10|
        EXPORT  |__rt_sdiv|
        EXPORT  |__rt_sdiv10|
        EXPORT  |__rt_divtest|

;----------------------------------------------------------------------------;
        AREA    |C$$data|                  ; This module's data area         ;
;----------------------------------------------------------------------------;

HeapLimit
        DCD     |Image$$RW$$Limit|         ; initialised by the linker.

;----------------------------------------------------------------------------;
; This code has to run in but 26-bit ARM modes and 32-bit modes. To allow    ;
; for this, the code is carefully written so that all PSR restoration in     ;
; 26-bit mode is via the following macro.                                    ;
;----------------------------------------------------------------------------;

        MACRO
        RET     $cond
    IF      {CONFIG} = 26
        MOV$cond.S    pc, lr
    ELSE
        MOV$cond      pc, lr
    ENDIF
        MEND

;----------------------------------------------------------------------------;
; The following four SWI definitions are specific to ARMulator/RISC OS.      ;
; However, you will need to replace the whole of this following section...   ;
; and all uses of these SWIs should also be replaced.                        ;
;----------------------------------------------------------------------------;

WriteC  EQU     0                          ; Write r0 to error/debug stream.
Write0  EQU     2                          ; Write 0-terminated string pointed
                                           ; to by r0 to error/debug stream.
Exit    EQU     17                         ; Terminate program execution.

;----------------------------------------------------------------------------;
        AREA    |C$$code$$__main|, CODE, READONLY
; The code area containing __main, __rt_exit                                 ;
;----------------------------------------------------------------------------;

        ENTRY                              ; Define the image entry point.

|__main|
;
; This is the initial entry point to the image.
; Have to establish a stack for C
; No arguments are passed to main from an embedded application,
; so argc and argv are set up to 0

        MOV     sp, #TopOfMemory           ; Initial stack pointer...
        MOV     fp, #0                     ; No previous frame, so fp=0

        LDR     a3, RootStackSize
        CMP     a3, #0                     ; Is RootStackSize defined?
        LDRNE   a3, [a3]                   ; Yes: use value...
        CMPNE   a3, #DefaultStackSize      ; but check caller not being silly.
        MOVLE   a3, #DefaultStackSize      ; No/silly: use default size.

        SUB     sl, sp, a3                 ; stack low-water mark
        ADD     sl, sl, #StackSlop         ; sl = LWM + StackSlop

  IF EnsureNoFPSupport = {FALSE}
        LDR     a1, fp_initialise          ; initialise FP code if present
        CMP     a1, #0
        MOVNE   lr, pc
        MOVNE   pc, a1
  ENDIF

        MOV     a1, #0                     ; set argc to 0
        MOV     a2, #0                     ; and argv to NUL
        BL      main                       ; Call main, falling through to
                                           ; exit on return.

|__rt_exit|                                ; exit
;
; void __rt_exit(int code);
; Terminate execution, optionally setting return code (ignored here).
; MUST NOT RETURN.

  IF EnsureNoFPSupport = {FALSE}
        LDR     a2, fp_finalise            ; finalise FP code if present
        CMP     a2, #0
        MOVNE   lr, pc
        MOVNE   pc, a2
  ENDIF
        SWI     Exit                       ; suicide...

RootStackSize
        DCD     |__root_stack_size|

  IF EnsureNoFPSupport = {FALSE}
fp_initialise
        DCD     |__fp_initialise|
fp_finalise
        DCD     |__fp_finalise|
  ENDIF

;----------------------------------------------------------------------------;
        AREA    |C$$code$$__rt_fpavailable|, CODE, READONLY
; The code area containing __rt_fpavailable                                 ;
;----------------------------------------------------------------------------;

|__rt_fpavailable|
;
; int __rt_fpavailable(); return non-0 if FP support code linked.

  IF EnsureNoFPSupport = {FALSE}
        LDR     a1, fp_initialise
  ELSE
        MOV     a1, #0
  ENDIF
        RET

;----------------------------------------------------------------------------;
        AREA    |C$$code$$__rt_trap|, CODE, READONLY
; The code area containing __rt_trap                                         ;
;----------------------------------------------------------------------------;
; Support for low-level failures - currently stack overflow, divide by 0 and ;
; floating-point exceptions. If there is a higher level handler, call it;    ;
; otherwise, print a message and exit gracefully.                            ;
;                                                                            ;
; NOTES                                                                      ;
;                                                                            ;
; typedef struct { unsigned code; char message[252];} __rt_error;            ;
; typedef struct { unsigned r[16];} __rt_registers;                          ;
;                                                                            ;
;----------------------------------------------------------------------------;

|__rt_trap|
;
; void __rt_trap(__rt_error *e, __rt_registers *r);

        STMFD   sp!, {a1}             ; save e in case handler returns...
        LDR     ip, err_handler
        CMP     ip, #0
        MOVNE   lr, pc
    IF      {CONFIG} = 26
        MOVNES  pc, ip                ; if got a handler, use it and
    ELSE
        MOVNE   pc, ip                ; if got a handler, use it and
    ENDIF
        LDMFD   sp!, {v1}             ; hope not to return...

        ADR     a1, RTErrorHead
        SWI     Write0                ; write preamble...
        ADD     a1, v1, #4
        SWI     Write0                ; write error diagnosis
        ADR     a1, RTErrorTail
        SWI     Write0                ; write postlude
        MOV     a1, #255
        B       |__rt_exit|           ; and terminate with non-zero exit code
err_handler
        DCD     |__err_handler|

save_regs_and_trap
        STMFD   sp!, {sp, lr, pc}
        STMFD   sp!, {r0-r12}
        STR     lr, [sp, #4*15]       ; caller's pc is my lr
        MOV     a2, sp
        MOV     a1, ip
        B       |__rt_trap|

RTErrorHead
        DCB     10, 13, "run time error: ", 0

RTErrorTail
        DCB     10, 13, "program terminated", 10, 13, 10, 13, 0

        ALIGN

;----------------------------------------------------------------------------;
; YOU SHOULDN'T NEED TO ALTER ANY OF THE FOLLOWING IN A FIRST RETARGETTING.  ;
;----------------------------------------------------------------------------;

;----------------------------------------------------------------------------;
        AREA    |C$$code$$__rt_alloc|, CODE, READONLY
; The code area containing __rt_alloc                                        ;
;----------------------------------------------------------------------------;
; Primitive support for heap memory management.                              ;
;                                                                            ;
; NOTES                                                                      ;
;                                                                            ;
; 1/ The allocator embeds knowledge of the memory layout and interacts with  ;
;    the stack limit checking code. Here we assume a single address space    ;
;    with the stack at the top growing down and the heap below it growing    ;
;    up, with a gap (free memory) in between.                                ;
;                                                                            ;
; 2/ Failure of the stack-limit check is fatal. However, failure of the low- ;
;    level heap allocator is passed back to its caller.                      ;
;----------------------------------------------------------------------------;


|__rt_alloc|                             ; alloc
;
; unsigned __rt_alloc(unsigned minwords, void **block);
;
; This tries to allocate a block of sensible size >= minwords. Failing that,
; it allocates the largest possible block of sensible size. If it can't do
; that, it returns zero. *block is set to point to the start of the allocated
; block (NULL if none has been allocated).
;
; NOTE: works in units of WORDS, NOT bytes.
;
; In this implementation, sl - StackSlop marks the end of allocatable store.

        CMP     a1, #MinHeapIncrement    ; round up to at least
        MOVLT   a1, #MinHeapIncrement    ; MinHeapIncrement words...
        LDR     a3, HeapLimitAdr
        LDR     a4, [a3]                 ; current heap high-water mark
        SUB     ip, sl, #StackSlop       ; current stack low-water mark
        CMP     a4, ip
        MOVGE   a4, #0                   ; no space, *block = NULL
        STR     a4, [a2]
        MOVGE   a1, #0                   ; no space, return 0
        ADD     a4, a4, a1, LSL #2       ; proposed new heap limit
        CMP     a4, ip
        SUBGT   a2, a4, ip               ; byte overlap, >= 0 by earlier code
        SUBGT   a1, a1, a2, LSR #2       ; reduce word request
        MOVGT   a4, ip                   ; new high-water = stack low-water
        STR     a4, [a3]  
        RET     
HeapLimitAdr
        DCD     HeapLimit

;----------------------------------------------------------------------------;
        AREA    |C$$code$$__rt_stkovf|, CODE, READONLY
; The code area containing __rt_stkovf_*                                     ;
;----------------------------------------------------------------------------;
; C stack-limit checking support.                                            ;
;                                                                            ;
; NOTES                                                                      ;
;                                                                            ;
; 1/ Stack-limit-checking is optional - you can compile your C code without  ;
;    stack-limit checks (#pragma nocheck_stack or cc -zps0). However, the    ;
;    cost of the check is (very) small and the value sometimes considerable. ;
;                                                                            ;
; 2/ The limit check embeds knowledge of the memory layout and interacts     ;
;    with the primitive memory management supported by __rt_alloc. Here, we  ;
;    assume a single address space with the stack at the top growing down    ;
;    and the heap below it growing up, with a gap (free memory) in between.  ;
;                                                                            ;
; 3/ Failure of the stack-limit check is fatal. However, failure of the low- ;
;    level heap allocator is passed back to its caller.                      ;
;                                                                            ;
; 4/ This implementation never reduces the size of the stack. It simply      ;
;    moves the low-water mark monatonically downwards. It is easy to do      ;
;    better, but, of course, it takes more code and is more target-specific. ;
;----------------------------------------------------------------------------;

|__rt_stkovf_split_small|                         ; stkovf_split_small_frame ;
;
; Enter here when a C function with frame size <= 256 bytes underflows
; the stack low-water mark + StackSlop (sl). The stack space required has
; already been claimed by decrementing sp, so we set the proposed sp (ip)
; to the actual sp and fall into the big-frame case.

        MOV     ip, sp       ; fall into big-frame case with size of 0.

|__rt_stkovf_split_big|                             ; stkovf_split_big_frame ;
;
; Enter here when a C function with frame size > 256 bytes would underflow
; the stack low-water mark + StackSlop (sl). No stack space has been claimed
; but the proposed new stack pointer is in ip.

        SUB     ip, sp, ip                      ; frame size required...
        CMP     ip, #DefaultStackIncrement      ; rounded up to at least
        MOVLT   ip, #DefaultStackIncrement      ; the default increment

        SUB     sl, sl, ip
        SUB     sl, sl, #StackSlop              ; new stack low-water mark

        LDR     ip, HeapLimitAdr                ; check doesn't collide with
        LDR     ip, [ip]                        ; the heap.
        CMP     ip, sl
        ADD     sl, sl, #StackSlop              ; restore safety margin
        BGT     stackoverflow
        RET                                     ; and return if OK...

stackoverflow
        ADR     ip, StackOverflowError
        B       save_regs_and_trap

StackOverflowError
        DCD     3
        DCB     "stack overflow", 0
        ALIGN


;----------------------------------------------------------------------------;
        AREA    |C$$code$$__jmp|, CODE, READONLY
; The code area containing setjmp, longjmp                                   ;
;----------------------------------------------------------------------------;
; Setjmp and longjmp support.                                                ;
;                                                                            ;
; NOTES                                                                      ;
;                                                                            ;
; 1/ Specific to C and not implementable in C.                               ;
;                                                                            ;
; 2/ Interacts with stack management and possibly with memory management.    ;
;    e.g. on a chunked stack, longjmp must de-allocate jumped-over chunks.   ;
;                                                                            ;
; 3/ Must know whether the floating-point instruction-set is supported!      ;
;    (DEPENDS ON __rt_fpavailable to discover this).                         ;
;                                                                            ;
;----------------------------------------------------------------------------;

        MAP 0                 ; This structure maps the jmp_buf
sj_v1   #       4             ; data type assumed by the C compiler.
sj_v2   #       4             ; First, space to save the v-registers...
sj_v3   #       4
sj_v4   #       4
sj_v5   #       4
sj_v6   #       4
sj_sl   #       4             ; then the frame registers sl, fp, sp (ap),
sj_fp   #       4             ; and pc/lr...
sj_ap   #       4
sj_pc   #       4
sj_f4   #       3*4           ; and finally the floating-point reisters,
sj_f5   #       3*4           ; used only if floating point support is
sj_f6   #       3*4           ; available.
sj_f7   #       3*4


|setjmp|                      ; setjmp
;
; int setjmp(jmp_buf env);
; Saves everything that might count as a register variable in 'env'.

        STMIA   a1!, {v1-v6, sl, fp, sp, lr}
        MOV     v6, a1                 ; v6 safe in env - use to point past
                                       ; saved lr (at 1st FP slot)
        BL      |__rt_fpavailable|
        CMP     a1, #0
        BEQ     setjmp_return          ; no fp
        STFE    f4, [v6, #sj_f4-sj_f4]
        STFE    f5, [v6, #sj_f5-sj_f4]
        STFE    f6, [v6, #sj_f6-sj_f4]
        STFE    f7, [v6, #sj_f7-sj_f4]
        MOV     a1, #0                 ; must return 0 from a direct call
setjmp_return
        LDMDB   v6, {v6, sl, fp, sp, lr}
        RET

|longjmp|                                                          ; longjmp ;
; int longjmp(jmp_buf env, int val);

        MOV     v1, a1                 ; save env ptr over call to fpavailable
        MOVS    v6, a2                 ; ensure non-0 return value...
        MOVEQ   v6, #1                 ; (must NOT return 0 on longjmp(env, 0))
        BL      |__rt_fpavailable|
        CMP     a1, #0
        BEQ     longjmp_return
        LDFE    f7, [v1, #sj_f7]
        LDFE    f6, [v1, #sj_f6]
        LDFE    f5, [v1, #sj_f5]
        LDFE    f4, [v1, #sj_f4]
longjmp_return
        MOV     a1, v6
        LDMIA   v1, {v1-v6, sl, fp, sp, lr}
        RET

;----------------------------------------------------------------------------;
        AREA    |C$$code$$__divide|, CODE, READONLY
; The code area containing __rt_sdiv, __rt_udiv, __rt_sdiv_10, __rt_udiv10   ;
;----------------------------------------------------------------------------;
; GENERIC ARM FUNCTIONS - divide and remainder.                              ;
;                                                                            ;
; NOTES                                                                      ;
;                                                                            ;
; 1/ You may wish to make these functions part of your O/S kernel, replacing ;
;    the implementations here by branches to the relevant entry addresses.   ;
;                                                                            ;
; 2/ Each divide function is a div-rem function, returning the quotient in   ;
;    r0 and the remainder in r1. Thus (r0, r1) -> (r0/r1, r0%r1). This is    ;
;    understood by the C compiler.                                           ;
;                                                                            ;
; 3/ Because of its importance in many applications, divide by 10 is treated ;
;    as a special case. The C compiler recognises divide by 10 and generates ;
;    calls to __rt_{u,s}div10, as appropriate.                               ;
;                                                                            ;
; 4/ Each of the implementations below has been coded with smallness as a    ;
;    higher priority than speed.  Unrolling the loops will allow faster      ;
;    execution, but will produce much larger code.  If the speed of divides  ;
;    is critical then unrolled versions can be extracted from the ARM ANSI C ;
;    Library.                                                                ;
;                                                                            ;
;----------------------------------------------------------------------------;

; div_core is used by __rt_sdiv and __rt_udiv, and corrupts a3, a4 and ip
div_core
        CMP     a3, a4
        MOVHI   a4, a4, ASL #1
        BHI     div_core
div_core2
        CMP     a2, a4
        ADC     ip, ip, ip
        SUBHS   a2, a2, a4
        CMP     a1, a4
        MOVLO   a4, a4, LSR #1
        BLO     div_core2
        MOV     a1, ip
        RET

; Signed divide of a2 by a1: returns quotient in a1, remainder in a2
; Quotient is truncated (rounded towards zero).
; Sign of remainder = sign of dividend.
; Destroys a3, a4 and ip
; Negates dividend and divisor, then does an unsigned divide; signs
; get sorted out again at the end.

|__rt_sdiv|
        MOVS    a3, a1
        BEQ     dividebyzero            ; ip now unwanted

        RSBMI   a1, a1, #0              ; absolute value of divisor
        EOR     a3, a3, a2
        ANDS    ip, a2, #&80000000
        ORR     a3, ip, a3, LSR #1
        STMFD   sp!,{a3,lr}
        ; saved a3:
        ;  bit 31  sign of dividend (= sign of remainder)
        ;  bit 30  sign of dividend EOR sign of divisor (= sign of quotient)
        RSBNE   a2, a2, #0              ; absolute value of dividend

        MOV     a3, a2
        MOV     a4, a1
        MOV     ip, #0
        BL      div_core
        LDMFD   sp!,{a3}
        MOVS    a3, a3, ASL #1
        RSBMI   a1, a1, #0
        RSBCS   a2, a2, #0
    IF {CONFIG} = 26
        LDMFD   sp!,{pc}^
    ELSE
        LDMFD   sp!,{pc}
    ENDIF

; Unsigned divide of a2 by a1: returns quotient in a1, remainder in a2
; Destroys a4, ip and r5

|__rt_udiv|
        MOVS    a4, a1
        BEQ     dividebyzero

        MOV     ip, #0
        MOV     a3, #&80000000
        CMP     a2, a3
        MOVLO   a3, a2
        B       div_core

;
; Fast unsigned divide by 10: dividend in a1, divisor in a2.
; Returns quotient in a1, remainder in a2.
; Also destroys a3.
;
; Calculate x / 10 as (x * 2**32/10) / 2**32.
; That is, we calculate the most significant word of the double-length
; product. In fact, we calculate an approximation which may be 1 off
; because we've ignored a carry from the least significant word we didn't
; calculate. We correct for this by insisting that the remainder < 10
; and by incrementing the quotient if it isn't.

|__rt_udiv10|                                                       ; udiv10 ;
        MOV     a2, a1
        MOV     a1, a1, LSR #1
        ADD     a1, a1, a1, LSR #1
        ADD     a1, a1, a1, LSR #4
        ADD     a1, a1, a1, LSR #8
        ADD     a1, a1, a1, LSR #16
        MOV     a1, a1, LSR #3
        ADD     a3, a1, a1, ASL #2
        SUB     a2, a2, a3, ASL #1
        CMP     a2, #10
        ADDGE   a1, a1, #1
        SUBGE   a2, a2, #10
        RET

;
; Fast signed divide by 10: dividend in a1, divisor in a2.
; Returns quotient in a1, remainder in a2.
; Also destroys a3 and a4.
; Quotient is truncated (rounded towards zero).
; Make use of __rt_udiv10

|__rt_sdiv10|                                                       ; sdiv10 ;
        MOV     ip, lr
        MOVS    a4, a1
        RSBMI   a1, a1, #0
        BL      __rt_udiv10
        CMP     a4, #0
        RSBMI   a1, a1, #0
        RSBMI   a2, a2, #0
    IF {CONFIG} = 26
        MOVS    pc, ip
    ELSE
        MOV     pc, ip
    ENDIF

;
; Test for division by zero (used when division is voided).

|__rt_divtest|                                                     ; divtest ;
        CMPS    a1, #0
        RET     NE
dividebyzero
        ADR     ip, DivideByZeroError
        B       save_regs_and_trap

DivideByZeroError
        DCD     1
        DCB     "divide by 0", 0
        ALIGN

        END
