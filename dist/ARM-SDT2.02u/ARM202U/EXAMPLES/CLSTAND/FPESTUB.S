;;; fpestub.s: library stub for fpe400 (emulator for fp instruction set 3)
;;;
;;; Copyright (C) Advanced RISC Machines Ltd., 1993

;;; RCS $Revision: 1.5 $
;;; Checkin $Date: 1994/01/24 14:19:42 $
;;; Revising $Author: irickard $

        AREA    |FP$$code|, CODE

        IMPORT  __rt_trap, WEAK                 ; from run-time kernel...

; change FPE_ to FPS_ to allow FPA support code to be incorporated in library
; (there is currently no veneer support for the combined FPE/FPASC)
        IMPORT  FPE_Install
        IMPORT  FPE_DeInstall
        EXPORT  FPE_GenerateError               ; to FPE

        EXPORT  __fp_initialise                 ; to client
        EXPORT  __fp_finalise                   ; to client
        EXPORT  __fp_address_in_emulator        ; to client

        IMPORT  |FP$$code$$Base|
        IMPORT  |FP$$code$$Limit|


; RISCOS SWI names (for use in very_standalone only).

Write0     * 2 + (1:SHL:17)
NewLine    * 3 + (1:SHL:17)
Exit       * &11

;******************************************************************************
;


__fp_initialise
        B       FPE_Install

__fp_finalise
        B       FPE_DeInstall

;******************************************************************************
;
;       Come here for a floating point exception, such as divide by zero.
;
; r0 = error descriptor
; r1 -> cpu register dump
;

FPE_GenerateError
; still in some non-user mode...
        LDR     r2, =|__rt_trap|
        CMP     r2, #0
        BEQ     very_standalone
        ADD     r3, r1, #r13*4
        LDMIA   r3, {r13}^              ; retrieve user's sp
        NOP
        MOVS    pc, r2                  ; to __rt_ in user mode


very_standalone
 [ {CONFIG} = 26
        TEQP    pc, #0                  ; to user mode
 |
        MSR     CPSR_ctl, #&10          ; to user32
 ]
        ADD     r0, r0, #4              ; ignore the error code
        SWI     Write0                  ; write the message
        SWI     NewLine
        BL      |__fp_finalise|         ; tidy the ill-instr vector
        SWI     Exit                    ; and exit

|__fp_address_in_emulator|
        ; for the benefit of abort handling, determine whether an address
        ; is within the code of the fp emulator.  (Allowing a data abort or
        ; address exception in a floating-point load or store to be reported
        ; as occurring at that instruction, rather than somewhere in the code
        ; of the emulator).
 [ {CONFIG} = 26
        BIC     r0, r0, #&fc000003      ; remove PSR bits in case
 ]
        LDR     r1, =|FP$$code$$Base|
        CMP     r0, r1
        LDRGT   r1, =|FP$$code$$Limit|
        CMPGT   r1, r0
        MOVLE   r0, #0
        MOVGT   r0, #1
 [ {CONFIG} = 26
        MOVS    pc, lr
 |
        MOV     pc, lr
 ]
        LTORG

        END
