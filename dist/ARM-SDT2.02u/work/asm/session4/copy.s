    AREA    Copy, CODE, READONLY

    ENTRY                  ; mark the first instruction to call
start
    LDR     r1, =srcstr    ; pointer to first string
    LDR     r0, =dststr    ; pointer to second string
strcopy                    ; copy first string over second
    LDRB    r2, [r1],#1    ; load byte and update address
    STRB    r2, [r0],#1    ; store byte and update address;
    CMP     r2, #0         ; check for zero terminator
    BNE     strcopy        ; keep going if not
stop
    SWI     0x11           ; terminate

    AREA    Strings, DATA, READWRITE

srcstr      DCB "First string - source",0
dststr      DCB "Second string - destination",0

    END
