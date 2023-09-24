    AREA Value, CODE, READONLY	; name this block of code
    ENTRY                    	; mark first instruction
                            	; to execute
start                        
    MOV     r0, #0x1            ; = 1
    MOV     r1, #0xFFFFFFFF     ; = -1 (signed)
    MOV     r2, #0xFF        	; = 255
    MOV     r3, #0x101          ; = 257
    MOV     r4, #0x400          ; = 1024

stop    
    SWI     0x11           		; Terminate
    END                        	; Mark end of file

