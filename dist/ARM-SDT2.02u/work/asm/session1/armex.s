    AREA ARMex, CODE, READONLY  ; name this block of code
    ENTRY                       ; mark first instruction
                                ; to execute
start
    MOV     r0, #10         ; Set up parameters
    MOV     r1, #3
    ADD     r0, r0, r1      ; r0 = r0 + r1
stop
    SWI     0x11            ; Terminate
    END                     ; Mark end of file

