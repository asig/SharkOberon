        EXPORT  __main
        IMPORT  main
        IMPORT  |sb$$interLUdata$$Base|

SWI_GetEnv  *       &10
SWI_Exit    *       &11

        AREA |StartupCode|, CODE, READONLY, REENTRANT

;
; This is the initial entry point to the image.
; (The compiler ensures it is linked in to the image by generating a reference
;  to __main from the object module generated from compiling a file containing
;  an  extern main()).
        ENTRY

; We have to establish a stack for C, initialise the library, then call _main
; (which will set up the arguments for main, then call it).
|__main|
        LDR     r9, =|sb$$interLUdata$$Base|
        SWI     SWI_GetEnv              ; to decide heap limit
        MOV     r13, r1                 ; top of program workspace
        BL      |main|
        SWI     SWI_Exit

        END
