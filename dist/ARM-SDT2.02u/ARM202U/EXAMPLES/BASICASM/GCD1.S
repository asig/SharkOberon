	AREA 	gcd1, CODE, READONLY	; name this block of code
	ENTRY							; mark first instruction
									; to execute
start
	MOV		r0, #1					; Set up parameters
	MOV		r1, #2
	BL		gcd						; Call subroutine
	SWI		0x11					; terminate

gcd	
	CMP 	r0, r1
	BEQ 	end
	BLT 	less
	SUB 	r0, r0, r1
	BAL 	gcd
less
	SUB 	r1, r1, r0
	BAL 	gcd
end	MOV		pc,lr

	END								; mark end of file