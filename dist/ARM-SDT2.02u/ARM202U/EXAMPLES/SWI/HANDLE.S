	AREA 	TopSwiHandler, CODE 	; name this block of code

	EXPORT	SWIHandler
	EXPORT  MakeChain
	IMPORT  C_SWI_Handler	
	IMPORT 	Dswivec
SWIHandler
	SUB r13, r13, #4		; leave space to store spsr
	STMFD r13!, {r0-r12,r14}	; store registers
	MOV r1, r13			; second parameter to C routine
					; is register values.
	LDR r0,[r14,#-4]		; Calculate address of SWI instruction
					; and load it into r0
	BIC r0,r0,#0xff000000		; mask off top 8 bits of instruction
	MRS r2, spsr
	STR r2,[r13,#14*4]		; store spsr on stack at original r13
	BL  C_SWI_Handler		; Call C routine to handle SWI
	CMP r0, #0			; Has C routine handled SWI ?
					;  0 = no, 1 = yes
	LDR r2, [r13,#14*4]		; extract spsr from stack
	MSR spsr,r2			; and restore it
	LDMFD r13!, {r0-r12,lr}		; Restore original registers
	ADD r13,r13,#4
	; Now need to decide whether to return from handler or to call
	; the next handler in the chain (the debugger's).
	MOVNES pc,lr			; return from handler if SWI handled
	LDR pc, swichain		; else jump to address containing
					; instruction to branch to address of
					; debugger's SWI handler.

swichain
	DCD 0

MakeChain
	LDR r0, =swichain
	LDR r1, =Dswivec
	LDR r2, [r1]
	STR r2, [r0]
	MOV pc,lr

	END			; mark end of this file

