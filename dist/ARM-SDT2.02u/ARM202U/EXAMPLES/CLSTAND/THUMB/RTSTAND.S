; Issue: 1.00/19-Jan-95
;
; Purpose: Minimal, standalone, C-library kernel for Thumb
;
; Copyright (C) 1995 Advanced RISC Machines Limited. All rights reserved.
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
; This code shows you how to write your own minimal, standalone, Thumb       ;
; run-time support system for code compiled by Advanced RISC Machines's      ;
; Thumb C Compiler. It can be assembled using Advanced RISC Machines's Thumb ;
; assembler.                                                         ;
;                                                                            ;
; This code may be used to build a ROM image. It may also be run under       ;
; Advanced RISC Machines's ARM emulation  system (ARMulator).                ;
;                                                                            ;
; In fact, this code depends hardly at all on its target environment and is  ;
; designed to be very easy to adapt to your particular ARM-based system.     ;
;                                                                            ;
; Much of the code below is generic to the ARM processor and is completely   ;
; independent of your ARM-based hardware or any operating system kernel that ;
; may run on it. To get going, you need write only 4 simple fns.             ;
;                                                                            ;
; WHAT THIS CODE PROVIDES:                                                   ;
;                                                                            ;
;  - Example, executable implementations of the few                          ;
;    simple functions you need to implement to customise this code to your   ;
;    environment. These include:                                             ;
;      - setting up the initial stack and heap and calling main (__main)     ;
;      - program termination (__rt_exit)                                     ;
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
; The following constant is the Top Of RAM - Adjust this for your system     ;
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

;----------------------------------------------------------------------------;
; Symbols defined in other, separately-assembled modules, must be IMPORTed.  ;
; We import them WEAKly so that they need not be defined.                    ;
;----------------------------------------------------------------------------;

        IMPORT  __root_stack_size, WEAK

;----------------------------------------------------------------------------;
; If __root_stack_size, also imported WEAKly, exists, the value it addresses ;
; is used as the initial size of the stack. It can be defined in your C      ;
; program as, e.g. int __root_stack_size = 10000; /* 10KB initial stack */   ;
;----------------------------------------------------------------------------;

        IMPORT  __err_handler, WEAK

;----------------------------------------------------------------------------;
; If __err_handler exists, errors are passed to it; otherwise, we print a    ;
; simple diagnostic message and exit.                                        ;
;----------------------------------------------------------------------------;

        IMPORT  |Image$$RO$$Base|
        IMPORT  |Image$$RO$$Limit|
        IMPORT  |Image$$RW$$Base|
        IMPORT  |Image$$RW$$Limit|
        IMPORT  |Image$$ZI$$Base|
        IMPORT  |Image$$ZI$$Limit|

;----------------------------------------------------------------------------;
; The above symbols are created by the linker to define various sections in  ;
; the ROM/RAM image.                                                         ;
;                                                                            ;
; Image$$RO$$Base   defines the code (ROM) base address                      ;
; Image$$RO$$Limit  defines the code limit and the start of a section of     ;
;                   data initialisation values which are copied to RAM       ;
;                   in __main below before main is called.                   ;
; Image$$RW$$Base   defines the data (RAM) base address                      ;
; Image$$RW$$Limit  defines the data end address                             ;
; Image$$ZI$$Base   defines the base of a section to be initialised with 0s  ;
; Image$$ZI$$Limit  defines the end of the region to be initialised with 0s  ;
;                   (must be the same as Image$$RW$$Limit in this model)     ;
;----------------------------------------------------------------------------;

        IMPORT  main

;----------------------------------------------------------------------------;
; The symbol main identifies the C function entered from this code.          ;
;----------------------------------------------------------------------------;

;----------------------------------------------------------------------------;
; THE MEMORY MODEL ASSUMED BY THIS IMPLEMENTATION                            ;
;                                                                            ;
;  RAM                                                                       ;
;                                                                            ;
;       +------------------+    <--- top of memory (high address)            ;
;       | Stack space      |                                                 ;
;       |..................|    <--- stack pointer (sp)                      ;
;       | Free stack       |                                                 ;
;       |..................|    <--- stack limit pointer (sl)                ;
;       +------------------+    <--- stack low-water mark (sl - StackSlop)   ;
;       |                  |                                                 ;
;       | Unused memory    |                                                 ;  
;       |                  |                                                 ;
;       +------------------+    <--- top of heap (HeapLimit)                 ;
;       |                  |                                                 ;
;       | Heap space       |                                                 ;
;       |                  |                                                 ;
;       +------------------+    <--- top of fixed data  (Image$$RW$$Limit)   ;
;       | Zero init data   |                            (=Image$$ZI$$Limit)  ;
;       +------------------+    <--- top of initialised (Image$$ZI$$Base)    ;
;       | Initialised data |                       data                      ;
;       +------------------+    <--- Data base address (Image$$RW$$Base)     ;
;                                                                            ;
; ROM                                                                        ;
;                                                                            ;
;       +------------------+    <--- Top of ROM image                        ;
;       | Initial values   | } Copied to "Initialised data" section in RAM   ;
;       | for Init data    | } on statup in __main below                     ;
;       +------------------+    <--- End of code (Image$$RO$$Limit)          ;
;       |  Code            |                                                 ;
;       +------------------+    <--- Code base address (Image$$RO$$Base)     ;
;----------------------------------------------------------------------------;

;----------------------------------------------------------------------------;
; Now the symbols we define and EXPORT from this module.                     ;
;----------------------------------------------------------------------------;
; First, symbols identifying the four functions you have to implement to     ;
; make this run-time kernel work on your hardware.                           ;
;----------------------------------------------------------------------------;

        EXPORT  __main
        EXPORT  __rt_exit
        EXPORT  __rt_trap

;----------------------------------------------------------------------------;
; Then some simple support for C heap management. It interacts with stack-   ;
; limit checking but should require no attention in a first re-targetting.   ;
;----------------------------------------------------------------------------;

        EXPORT  __rt_alloc

;----------------------------------------------------------------------------;
; Next, optional support for C stack-limit checking. This code should need   ;
; no attention in a first re-targetting.                                     ;
;----------------------------------------------------------------------------;

        EXPORT  __16__rt_stkovf_split_small ; veneer
        EXPORT  __16__rt_stkovf_split_big

;----------------------------------------------------------------------------;
; Then two C-specific functions which should require no attention in a first ;
; re-targetting.                                                             ;
;----------------------------------------------------------------------------;

        EXPORT  setjmp
        EXPORT  longjmp

;----------------------------------------------------------------------------;
; And, finally, generic ARM functions, referred to by the C compiler.        ;
; You should not need to alter any of these unless you wish to incorporate   ;
; them in your operating system kernel. See also later comments.             ;
;----------------------------------------------------------------------------;

        EXPORT  __16__rt_udiv
        EXPORT  __16__rt_udiv10
        EXPORT  __16__rt_sdiv
        EXPORT  __16__rt_sdiv10
        EXPORT  __16__rt_divtest

;----------------------------------------------------------------------------;
        AREA    |C$$data|                  ; This module's data area         ;
;----------------------------------------------------------------------------;

HeapLimit
        DCD     |Image$$RW$$Limit|         ; initialised by the linker.

;----------------------------------------------------------------------------;
; Macro to return from a function                                            ;
; We use the BX instruction below rather than MOV pc, lr so that the return  ;
; will work correctly if we are called from ARM state                        ;
;----------------------------------------------------------------------------;

        MACRO
        RET
        BX      lr
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

        CODE16

TBit    EQU     1                          ; Bit to set in register to enter
                                           ; Thumb state with BX <reg>

;----------------------------------------------------------------------------;
; Use area name "!!!" so this area is placed first as AREAs are sorted by
; area name.
        AREA    |!!!|, CODE, READONLY, INTERWORK
; The code area containing __main, __rt_exit                                 ;
;----------------------------------------------------------------------------;

        ENTRY                              ; Define the image entry point.

__main
        CODE32                             ; Entered in ARM state presumeably
        ADR     lr, __main_16+TBit
        BX      lr
        CODE16
__main_16
;
; This is the initial entry point to the image.
; Have to establish a stack for C
; No arguments are passed to main from an embedded application,
; so argc and argv are set up to 0

        LDR     r0, =TopOfMemory           ; Set up initial stack pointer
        MOV     sp, r0

        MOV     r0, #0
        MOV     fp, r0                     ; No previous frame, so fp=0

        LDR     r0, =DefaultStackSize
        LDR     r1, =__root_stack_size
        CMP     r1, #0                     ; Is RootStackSize defined?
        BEQ     %F0                        ; No => Use default
        LDR     r1, [r1]                   ; Yes => Get value
        CMP     r1, r0                     ; But check value >= DefaultStackSize
        BCC     %F0                        ; if >= use default in r0.
        MOV     r0, r1
0
        MOV     r1, sp
        SUB     r1, r0                     ; stack low limit
        LDR     r0, =StackSlop
        ADD     r1, r0                     ; plus a bit spare
        MOV     sl, r1

; Now initialise the data segment by copying the initial values from ROM
; to RAM and by clearing the zero init segment to 0.

        LDR     r0, =|Image$$RO$$Limit|    ; Get pointer to ROM initial values
        LDR     r1, =|Image$$RW$$Base|     ; And RAM data segment
        LDR     r3, =|Image$$ZI$$Base|     ; Zero init base = top of initialised segment
        CMP     r0, r1                     ; Check that they are different, (they may be
        BEQ     %F3                        ; the same if the image is running in RAM)
        B       %F2
1
        LDMIA   r0!, {r2}                  ; Copy the initialising data over
        STMIA   r1!, {r2}
2       CMP     r1, r3
        BCC     %B1
3
        LDR     r1, =|Image$$ZI$$Limit|    ; Top of area to be zero initialised
        MOV     r2, #0
        B       %F5

4       STMIA   r3!, {r2}                  ; Clear out zero init segment
5       CMP     r3, r1
        BCC     %B4

        MOV     r0, #0                     ; set argc to 0
        MOV     r1, #0                     ; and argv to NUL
        BL      main                       ; Call main, falling through to
                                           ; exit on return.

__rt_exit                                  ; exit
;
; void __rt_exit(int code);
; Terminate execution, optionally setting return code (ignored here).
; MUST NOT RETURN.

        SWI     Exit                       ; suicide...

;----------------------------------------------------------------------------;
        AREA    |C$$code$$__rt_trap|, CODE, READONLY
; The code area containing __rt_trap                                         ;
;----------------------------------------------------------------------------;
; Support for low-level failures - currently stack overflow and divide by 0. ;
; If there is a higher level handler, call it otherwise, print a message and ;
; exit gracefully.                                                           ;
;                                                                            ;
; NOTES                                                                      ;
;                                                                            ;
; typedef struct { unsigned code; char message[252];} __rt_error;            ;
; typedef struct { unsigned r[16];} __rt_registers;                          ;
;                                                                            ;
;----------------------------------------------------------------------------;

__rt_trap
;
; void __rt_trap(__rt_error *e, __rt_registers *r);

        PUSH    {r0}                  ; save e in case handler returns...
        LDR     r3, =__err_handler
        CMP     r3, #0
        BEQ     %F0

        BL      call_via_r3           ; Call the routine pointed to by R3
                                      ; Note: BL sets bit 0 of LR so return will
                                      ; be to Thumb state. This is why we use this
                                      ; rather than the sequence
                                      ;         MOV     lr, pc
                                      ;         BX      r3
                                      ; which may return to ARM state!

0
        ADR     r0, RTErrorHead       ; No handler, or handler returned
        SWI     Write0                ; write preamble...
        POP     {r0}
        ADD     r0, #4
        SWI     Write0                ; write error diagnosis
        ADR     r0, RTErrorTail
        SWI     Write0                ; write postlude
        MOV     a1, #255
        BL      __rt_exit             ; and terminate with non-zero exit code

call_via_r3
        BX      r3

; SP has already been decremented by 16 * 4 and R0..R7 saved.
; IP points to the error description
; R7 contains the LR register which has been destroyed as a BL was required
; to get here.
save_regs_and_trap
        MOV     ip, r0
        MOV     r0, r8
        MOV     r1, r9
        MOV     r2, r10
        MOV     r3, r11
        MOV     r4, r12
        ADD     r5, sp, #16*4         ; Take account of previous SP adjustment
        ADD     r6, sp, #8*4          ; Pointer to hi reg save area
        STMIA   r6!, {r0-r5, r7}
        STR     r7, [sp, #15*4]       ; Save my pc as callers lr
        MOV     a2, sp
        MOV     a1, ip
        B       __rt_trap

        ALIGN
RTErrorHead
        DCB     10, 13, "run time error: ", 0

        ALIGN
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


__rt_alloc                               ; alloc
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

        LDR     r2, =MinHeapIncrement
        CMP     r0, r2                  ; round up to at least
        BGE     %F0
        MOV     r0, r2                  ; MinHeapIncrement words...
0
        MOV     ip, sl
        LDR     r2, =-StackSlop
        ADD     ip, r2                  ; current stack low-water mark
        LDR     r2, =HeapLimit
        LDR     r3, [r2]                ; current heap high-water mark
        CMP     r3, ip
        BLE     %F1
        MOV     r3, #0                  ; no space, *block = NULL
        MOV     r0, #0                  ; no space, return 0
1
        STR     r3, [r1]
        LSL     r0, #2                  ; Convert size request to bytes
        ADD     r3, r0                  ; proposed new heap limit
        CMP     r3, ip
        BLE     %F2
        ADD     r0, ip                  ; Reduce size request by amount
        SUB     r0, r3                  ; of overlap
        MOV     r3, ip                  ; new high-water = stack low-water
2
        STR     r3, [r2]  
        LSR     r0, #2                  ; Convert return size to words
        RET     

;----------------------------------------------------------------------------;
        AREA    |C$$code$$__rt_stkovf|, CODE, READONLY
; The code area containing __rt_stkovf_*                                     ;

        [ :LNOT::DEF:STACK_EXTENSION
                GBLL    STACK_EXTENSION
STACK_EXTENSION SETL    {FALSE}
        ]
;----------------------------------------------------------------------------;
; C stack-limit checking support.                                            ;
;                                                                            ;
; NOTES                                                                      ;
;                                                                            ;
; 1/ This code is only called if stack checking is enabled with the          ;
;    -apcs 3/swst option on the Thumb compiler. Typically you will only do   ;
;    this when debugging.                                                    ;
;                                                                            ;
; 2/ The code may be configured to either perform automatic stack extension  ;
;    or to generate an error on stack overflow. By default it is set up to   ;
;    generate an error. To change this to extend the stack uncomment the     ;
;    following line.                                                         ;
;                                                                            ;
;STACK_EXTENSION SETL   {TRUE}                                               ;
;                                                                            ;
; 3/ The stack extension embeds knowledge of the memory layout and interacts ;
;    with the primitive memory management supported by __rt_alloc. Here, we  ;
;    assume a single address space with the stack at the top growing down    ;
;    and the heap below it growing up, with a gap (free memory) in between.  ;
;                                                                            ;
; 4/ Failure of the stack-limit check is fatal. However, failure of the low- ;
;    level heap allocator is passed back to its caller.                      ;
;                                                                            ;
; 5/ This implementation of stack extension never reduces the size of the    ;
;    stack. It simply moves the low-water mark downwards. It is easy to do   ;
;    better, but, of course, it takes more code and is more target-specific. ;
;----------------------------------------------------------------------------;

__16__rt_stkovf_split_small
;
; Enter here when a C function with frame size <= 256 bytes underflows
; the stack low-water mark + StackSlop (sl). The stack space required has
; already been claimed by decrementing sp, so we set the proposed sp (ip)
; to the actual sp and fall into the big-frame case.

    [ STACK_EXTENSION
        MOV     ip, sp       ; fall into big-frame case with size of 0.
    ]

__16__rt_stkovf_split_big
;
; Enter here when a C function with frame size > 256 bytes would underflow
; the stack low-water mark + StackSlop (sl). No stack space has been claimed
; but the proposed new stack pointer is in ip.

    [ STACK_EXTENSION
        PUSH    {r0,r1}
        ADD     r0, sp, #8                      ; get original sp
        MOV     r1, ip
        SUB     r0, r1                          ; frame size required...
        LDR     r1, =DefaultStackIncrement
        CMP     r0, r1                          ; rounded up to at least
        BGE     %F0
        MOV     r0, r1                          ; the default increment
0
        MOV     r1, sl
        SUB     r1, r0
        LDR     r0, =StackSlop
        SUB     r1, r0                          ; new stack low-water mark
        MOV     sl, r1

        LDR     r1, =HeapLimit
        LDR     r1, [r1]                        ; check doesn't collide with
        CMP     r1, sl                          ; the heap.
        ADD     sl, r0                          ; restore safety margin
                                                ; (preserves CC)

        POP     {r0, r1}
        BGT     stackoverflow
        RET                                     ; and return if OK...

    ]
stackoverflow
        SUB     sp, sp, #8*4                    ; Room for R8..R15
        PUSH    {r0-r7}
        MOV     r7, lr
        ADR     r0, StackOverflowError
        BL      save_regs_and_trap

        ALIGN
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


setjmp
;
; int setjmp(jmp_buf env);
; Saves everything that might count as a register variable in 'env'.

        STMIA   a1!, {r4-r7}
        MOV     r0, r8
        MOV     r1, r9
        MOV     r2, sl
        MOV     r3, fp
        STMIA   a1!, {r0-r3}
        MOV     r0, sp
        MOV     r1, lr
        STMIA   a1!, {r0-r1}
        MOV     a1, #0                 ; must return 0 from a direct call
        RET

longjmp
; int longjmp(jmp_buf env, int val);

        ADD     r0, #4*4
        LDMIA   r0!, {r2-r7}           ; Restore r8 .. lr
        MOV     r8, r2
        MOV     r9, r3
        MOV     sl, r4
        MOV     fp, r5
        MOV     sp, r6
        MOV     lr, r7
        SUB     r0, #10*4
        LDMIA   r0!, {r4-r7}           ; Restore v1..v4
        MOV     r0, r1
        BNE     %F0
        MOV     r0, #1                 ; Must not return 0
0
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

; Signed divide of a2 by a1: returns quotient in a1, remainder in a2
; Quotient is truncated (rounded towards zero).
; Sign of remainder = sign of dividend.
; Destroys a3, a4 and ip
; Negates dividend and divisor, then does an unsigned divide; signs
; get sorted out again at the end.

__16__rt_sdiv
        ASR     a4, a2, #31
        EOR     a2, a4
        SUB     a2, a4

        ASR     a3, a1, #31
        EOR     a1, a3
        SUB     a1, a3

        BEQ     dividebyzero

        PUSH    {a3, a4}        ; Save so we can look at signs later on

        LSR     a4, a2, #1
        MOV     a3, a1

s_loop  CMP     a3, a4
        BNLS    %FT0
        LSL     a3, #1
0       BLO     s_loop

        MOV     a4, #0
        B       %FT0
s_loop2 LSR     a3, #1
0       CMP     a2, a3
        ADC     a4, a4
        CMP     a2, a3
        BCC     %FT0
        SUB     a2, a3
0
        CMP     a3, a1
        BNE     s_loop2
        MOV     a1, a4

        POP     {a3, a4}

        EOR     a3, a4
        EOR     a1, a3
        SUB     a1, a3

        EOR     a2, a4
        SUB     a2, a4

        RET

; Unsigned divide of a2 by a1: returns quotient in a1, remainder in a2
; Destroys a4, ip and r5

__16__rt_udiv
        LSR     a4, a2, #1
        MOV     a3, a1
        BEQ     dividebyzero

u_loop  CMP     a3, a4
        BNLS    %FT0
        LSL     a3, #1
0       BLO     u_loop

        MOV     a4, #0
        B       %FT0
u_loop2 LSR     a3, #1
0       CMP     a2, a3
        ADC     a4, a4
        CMP     a2, a3
        BCC     %FT0
        SUB     a2, a3
0
        CMP     a3, a1
        BNE     u_loop2
        MOV     a1, a4

        RET

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

__16__rt_udiv10                                                         ; udiv10 ;
        MOV     a2, a1
        LSR     a1, #1
        LSR     a3, a1, #1
        ADD     a1, a3
        LSR     a3, a1, #4
        ADD     a1, a3
        LSR     a3, a1, #8
        ADD     a1, a3
        LSR     a3, a1, #16
        ADD     a1, a3
        LSR     a1, #3
        ASL     a3, a1, #2
        ADD     a3, a1
        ASL     a3, #1
        SUB     a2, a3
        CMP     a2, #10
        BLT     %FT0
        ADD     a1, #1
        SUB     a2, #10
0
        RET

;
; Fast signed divide by 10: dividend in a1, divisor in a2.
; Returns quotient in a1, remainder in a2.
; Also destroys a3 and a4.
; Quotient is truncated (rounded towards zero).
; Make use of __rt_udiv10

__16__rt_sdiv10
        ASR     a4, a1, #31
        EOR     a1, a4
        SUB     a1, a4

        MOV     a2, a1
        LSR     a1, #1
        LSR     a3, a1, #1
        ADD     a1, a3
        LSR     a3, a1, #4
        ADD     a1, a3
        LSR     a3, a1, #8
        ADD     a1, a3
        LSR     a3, a1, #16
        ADD     a1, a3
        LSR     a1, #3
        ASL     a3, a1, #2
        ADD     a3, a1
        ASL     a3, #1
        SUB     a2, a3
        CMP     a2, #10
        BLT     %FT0
        ADD     a1, #1
        SUB     a2, #10
0
        EOR     a1, a4
        SUB     a1, a4
        EOR     a2, a4
        SUB     a2, a4
        RET

;
; Test for division by zero (used when division is voided).

__16__rt_divtest                                                     ; divtest ;
        CMPS    a1, #0
        BEQ     dividebyzero
        RET

dividebyzero
        SUB     sp, sp, #8*4                    ; Room for R8..R15
        PUSH    {r0-r7}
        MOV     r7, lr
        ADR     r0, DivideByZeroError
        BL      save_regs_and_trap

        ALIGN
DivideByZeroError
        DCD     1
        DCB     "divide by 0", 0
        ALIGN

        END
