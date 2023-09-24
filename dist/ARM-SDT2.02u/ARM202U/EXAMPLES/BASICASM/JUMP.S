	AREA 	ArithGate, CODE	; name this block of code
	ENTRY					; mark the first instruction to call
main	
	MOV		r0, #2			; set up three parameters
	MOV		r1, #5
	MOV		r2, #15
	BL		arithfunc		; call the function
	SWI		0x11			; terminate

arithfunc					; label the function
	CMP		r0, #4			; Treat code as unsigned integer
	BHI		ReturnA1		; If code > 4 then return first
							; argument
	ADR		r3, JumpTable	; Load address of the jump table
	LDR		pc,[r3,r0,LSL #2] ; Jump to appropriate routine

JumpTable
	DCD		ReturnA1
	DCD		ReturnA2
	DCD		DoAdd
	DCD		DoSub
	DCD		DoRsb

ReturnA1
	MOV		r0, r1			; Operation 0, >4
	MOV	pc,lr
ReturnA2
	MOV		r0, r2			; Operation 1
	MOV		pc,lr
DoAdd
	ADD		r0, r1, r2		; Operation 2
	MOV		pc,lr
DoSub
	SUB		r0, r1, r2		; Operation 3
	MOV		pc,lr
DoRsb
	RSB		r0, r1, r2		; Operation 4
	MOV		pc,lr

	END						; mark the end of this file
