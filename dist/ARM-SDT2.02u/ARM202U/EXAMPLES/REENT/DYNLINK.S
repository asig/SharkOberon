        AREA |DynamicLinker|, CODE, READONLY, REENTRANT

        EXPORT  |__rt_dynlink|

|__rt_dynlink|
; Enter from a call through the stub vector with r0-r6, lr saved on sp...
; ip (the 'new sb' value) is the index of the proxy function in the stub
; vector which caused this entry to the dynamic linker and identifies
; which call to resume after linking.
; On entry r0 points to the 4-word dynamic linker entry veneer at the
; end of the stub vector.

        MOV     r6, r0
        LDR     r5, [r6, #-8]           ; # entries - 1
        ADD     r5, r5, #1              ; # entries
        MOV     r4, ip                  ; resume index

; r6+24 points to the EFT parameter block; call the library location function
; to return a pointer to the matching library... Here we assume it's loaded
; at 0x40000...

; In the following line, the Makefile relies on a space after '#' and on the
; contents of the comment. DO NOT CHANGE THESE OR Makefile WILL FAIL.

        MOV     r0, # 0x40000        ; EFT Address (DO NOT ALTER THIS COMMENT)

; r0 now points to the EFT
        LDR     ip, [r0]                ; #entries
        CMPS    ip, r5
        BLT     Botched                 ; not enough entries to init the stub

        LDR     ip, [r6, #16]           ; stub data len 
        BIC     ip, ip, #3              ; word aligned, I insist...
        ADD     r3, r0, #4
        LDR     r3, [r3, r5, LSL #2]    ; library data len...
        CMPS    r3, ip
        BNE     Botched

        LDR     r3, [r6, #20]           ; stub data dest
        SUB     r2, r0, ip              ; library data src - precedes EFT
01      SUBS    ip, ip, #4              ; word by word copy loop
        LDRGE   r1, [r2], #4
        STRGE   r1, [r3], #4
        BGE     %B01

        LDR     ip, [r6, #12]           ; length of the inter-LU data area
        ADD     r3, r6, #24             ; end of the inter-LU data area...
        SUB     r3, r3, ip              ; sb = start of inter-LU data area

        LDR     r2, [r6, #-8]!          ; index of stub entry
00      SUB     ip, r5, #1              ; index of the lib entry
        CMPS    ip, r2                  ; is this lib entry in the stub?
        SUBGT   r5, r5, #1              ; no, skip it
        BGT     %B00
        CMPS    r2, r4                  ; found the retry index?
        MOVEQ   lr, r6                  ; yes: remember it
        LDR     ip, [r0, r5, lsl #2]    ; entry point offset
        ADD     ip, ip, r0              ; entry point address
        STMIA   r6, {r3, ip}            ; save {sb, pc}
        LDR     r2, [r6, #-8]!          ; load index and decrement r6...
        TST     r2, #&ff000000          ; ... or if loaded instruction?
        LDRNE   r2, [r6, #-8]!          ; load index and decrement r6 if instr
        SUBS    r5, r5, #1
        BGT     %B00

        MOV     ip, lr                  ; retry address
        LDMFD   sp!, {r0-r6, lr}        ; restore saved regs
        LDMIA   ip, {ip, pc}            ; and retry the call

Botched B       Botched

        END
