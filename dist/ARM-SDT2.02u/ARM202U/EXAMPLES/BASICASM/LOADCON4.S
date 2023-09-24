 	AREA Loadcon4, CODE
	ENTRY					; Mark first instruction
Start	
	BL	func1				; Branch to first subroutine
	BL	func2				; Branch to second subroutine
	SWI	0x11				; Terminate

func1
	LDR 	r0, =Start		; => LDR R0,[PC, #offset to 
							;					Litpool 1]
	LDR 	r1, =Darea +12	; => LDR R1,[PC, #offset to
							;					Litpool 1]
	LDR 	r2, =Darea + 6000 ; => LDR R2, [PC, #offset to
							;					Litpool 1]
	MOV	pc,lr				; Return

	LTORG					; Literal Pool 1 contains 3 literals

func2
	LDR	r3, =Darea +6000  	; => LDR r3, [PC, #offset to
							;					Litpool 1]
							; (sharing with previous literal)
 	; LDR	r4, =Darea +6004 ; If uncommented will produce an
							; error as Litpool 2 is out of range
	MOV		pc, lr			; Return

Darea		% 8000
      END					; Literal Pool 2 is out of range of
							; the LDR instructions above
