	AREA	StrCopy2, CODE
	ENTRY				; mark the first instruction
main	
	LDR		r1, =srcstr		; pointer to first string
	LDR		r0, =dststr		; pointer to second string
	BL		strcopy			; copy the first into second
	SWI		0x11			; and exit

srcstr		DCB "This is my first (source) string",0
dststr	DCB "This is my second (destination) string",0

	ALIGN					; realign address to word boundary

strcopy
	LDRB	r2, [r1], #1	; load byte, then update address
	STRB	r2, [r0], #1	; store byte, then update address
	CMP		r2, #0			; check for zero terminator
	BNE		strcopy			; keep going if not
	MOV		pc, lr			; return
	END
