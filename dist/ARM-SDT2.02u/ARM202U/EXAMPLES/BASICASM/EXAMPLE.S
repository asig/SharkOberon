	AREA 	Example, CODE, READONLY	; name this block of code
	ENTRY							; mark first instruction
									; to execute
start
	MOV		r0, #15					; Set up parameters
	MOV		r1, #20
	BL		firstfunc				; Call subroutine
	SWI		0x11					; terminate

firstfunc							; Subroutine firstfunc
	ADD		r0, r0, r1				; r0 = r0 + r1
	MOV		pc, lr					; Return from subroutine
									; with result in r0
	END								; mark end of file
