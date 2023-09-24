        AREA    |!!!|, CODE, READONLY

        IMPORT  __entry                 ; The C library entry point

        EXPORT  __main

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

; Code is enter in ARM mode
    [ {CONFIG} = 16
        CODE32
    ]

__main
        LDR     r0, =|Image$$RO$$Limit| ; Get pointer to ROM data
        LDR     r1, =|Image$$RW$$Base|  ; and RAM copy
        LDR     r3, =|Image$$ZI$$Base|  ; Zero init base => top of initialised data
        CMP     r0, r1                  ; Check that they are different
        BEQ     %FT1
0       CMP     r1, r3                  ; Copy init data
        LDRCC   r2, [r0], #4
        STRCC   r2, [r1], #4
        BCC     %BT0
1       LDR     r1, =|Image$$ZI$$Limit| ; Top of zero init segment
        MOV     r2, #0
2       CMP     r3, r1                  ; Zero init
        STRCC   r2, [r3], #4
        BCC     %BT2

        B       __entry

        END
