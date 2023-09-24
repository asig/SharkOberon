	AREA Loadcon2, CODE
	ENTRY					; Mark first instruction
	BL	func1				; Branch to first subroutine
	BL	func2				; Branch to second subroutine
	SWI	0x11				; Terminate
func1
 	LDR 	r0, =42			; => MOV R0, #42
	LDR 	r1, =0x55555555	; => LDR R1, [PC, #offset to Literal
							;							Pool 1]
	LDR 	r2, =0xFFFFFFFF ; => MVN R2, #0
	MOV	pc, lr

	LTORG					; Literal Pool 1 contains 
							; literal &55555555
func2
    LDR 	r3, =0x55555555 ; => LDR R3, [PC, #offset to Literal
							;							Pool 1]
    ; LDR 	r4, =0x66666666 ; If this is uncommented it will
							; fail, as Literal Pool 2 is not
							; accessible (out of reach)
	MOV	pc, lr

LargeTable	% 4200		; Clears a 4200 byte area of memory,
							; starting at the current location,
							; to zero.
	END						;Literal Pool 2 is empty
