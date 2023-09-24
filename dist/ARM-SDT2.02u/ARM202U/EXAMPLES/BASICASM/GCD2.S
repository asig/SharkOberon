	AREA 	gcd2, CODE, READONLY	; name this block of code
	ENTRY							; mark first instruction
									; to execute
start
	MOV		r0, #1					; Set up parameters
	MOV		r1, #2
	BL		gcd						; Call subroutine
	SWI		0x11					; terminate

gcd	
	CMP 	r0, r1
	SUBGT 	r0, r0, r1
	SUBLT 	r1, r1, r0
	BNE 	gcd
	MOV		pc,lr

	END								; mark end of file