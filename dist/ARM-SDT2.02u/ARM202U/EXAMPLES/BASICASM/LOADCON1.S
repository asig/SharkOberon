	AREA	loadcon1, CODE
	ENTRY					; mark first instruction

	MOV 	r0, #0			; => MOV r0, #0
	MOV 	r1, #0xFF000000	; => MOV r1, #0xFF, 8 
	MOV 	r2, #0xFFFFFFFF	; => MVN r2, #0
	MVN 	r0, #1			; => MVN r0, #1
	MOV 	r1, #0xFC000003	; => MOV r1, #0xFF, 6
	MOV 	r2, #0x03FFFFFC	; => MVN r2, #0xFF, 6
	;MOV 	r3, #0x55555555	; Reports an error (it cannot
							; be constructed)

	SWI		0x11			; terminate
	END

