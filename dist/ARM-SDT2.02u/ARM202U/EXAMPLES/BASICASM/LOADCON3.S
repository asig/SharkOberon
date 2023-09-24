	AREA 	Example, CODE
	ENTRY						; Mark first instruction
Start
	ADR		r0, Start			; => SUB r0, PC, #offset to Start
	ADR		r1, DataArea		; => ADD r1, PC, #offset to DataArea
	; ADR	r2, DataArea+4300	; This would fail as the offset is
								; cannot be expressed by operand2 
								; of an ADD
	ADRL	r3, DataArea+4300	; => ADD r2, PC, #offset1
 								;    ADD r2, r2, #offset2
	SWI		0x11					; Terminate
DataArea	% 8000
    
      END
