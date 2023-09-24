;
; The AREA must have the attribute READONLY, otherwise the linker will not
; place it in ROM.
;
; The AREA must have the attribute CODE, otherwise the assembler will not
; let us put any code in this AREA
;
; Note the '|' character is used to surround any symbols which contain
; non standard characters like '!'.

		AREA	Init, CODE, READONLY

; Now some standard definitions...

Mode_IRQ	EQU	0x12
Mode_SVC	EQU	0x13

I_Bit		EQU	0x80
F_Bit		EQU	0x40

SWI_Exit	EQU	0x11

; Locations of various things in our memory system

RAM_Base	EQU	0x10000000	; 64k RAM at this base
RAM_Limit	EQU	0x10010000

IRQ_Stack	EQU	RAM_Limit	; 1K IRQ stack at top of memory
SVC_Stack	EQU	RAM_Limit-1024	; followed by SVC stack

; --- Define __main & set entry point
	ENTRY

; --- Setup interrupt / exception vectors
    IF :DEF: ROM_AT_ADDRESS_ZERO
; If the ROM is at address 0 this is just a sequence of branches
	B	Reset_Handler
	B	Undefined_Handler
	B	SWI_Handler
	B	Prefetch_Handler
	B	Abort_Handler
	NOP		; Reserved vector
	B	IRQ_Handler
	B	FIQ_Handler
    ELSE
; Otherwise we copy a sequence of LDR PC instructions over the vectors
; (Note: We copy LDR PC instructions because branch instructions
; could not simply be copied, the offset in the branch instruction
; would have to be modified so that it branched into ROM. Also, a
; branch instructions might not reach if the ROM is at an address
; > 32M).
	MOV	R8, #0
	ADR	R9, Vector_Init_Block
	LDMIA	R9!, {R0-R7}
	STMIA	R8!, {R0-R7}
	LDMIA	R9!, {R0-R7}
	STMIA	R8!, {R0-R7}

; Now fall into the LDR PC, Reset_Addr instruction which will continue
; execution at 'Reset_Handler'

Vector_Init_Block
	LDR	PC, Reset_Addr
	LDR	PC, Undefined_Addr
	LDR	PC, SWI_Addr
	LDR	PC, Prefetch_Addr
	LDR	PC, Abort_Addr
	NOP
	LDR	PC, IRQ_Addr
	LDR	PC, FIQ_Addr

Reset_Addr	DCD	Reset_Handler
Undefined_Addr	DCD	Undefined_Handler
SWI_Addr	DCD	SWI_Handler
Prefetch_Addr	DCD	Prefetch_Handler
Abort_Addr	DCD	Abort_Handler
		DCD	0	; Reserved vector
IRQ_Addr	DCD	IRQ_Handler
FIQ_Addr	DCD	FIQ_Handler
    ENDIF

; The following handlers do not do anything useful in this example.
;
Undefined_Handler
	B	Undefined_Handler
SWI_Handler
	B	SWI_Handler
Prefetch_Handler
	B	Prefetch_Handler
Abort_Handler
	B	Abort_Handler
IRQ_Handler
	B	IRQ_Handler
FIQ_Handler
	B	FIQ_Handler

; The RESET entry point
Reset_Handler

; --- Initialise stack pointer registers
; Enter IRQ mode and set up the IRQ stack pointer
	MOV	R0, #Mode_IRQ:OR:I_Bit:OR:F_Bit ; No interrupts
	MSR	CPSR, R0
	LDR	R13, =IRQ_Stack

; Set up other stack pointers if necessary
	; ...

; Set up the SVC stack pointer last and return to SVC mode
	MOV	R0, #Mode_SVC:OR:I_Bit:OR:F_Bit ; No interrupts
	MSR	CPSR, R0
	LDR	R13, =SVC_Stack

; --- Initialise memory system
	; ...

; --- Initialise critical IO devices
	; ...

; --- Initialise interrupt system variables here
	; ...

; --- Enable interrupts
; Now safe to enable interrupts, so do this and remain in SVC mode
	MOV	R0, #Mode_SVC:OR:F_Bit	; Only IRQ enabled
	MSR	CPSR, R0

; --- Initialise memory required by C code

	IMPORT	|Image$$RO$$Limit|	; End of ROM code (=start of ROM data)
	IMPORT	|Image$$RW$$Base|	; Base of RAM to initialise
	IMPORT	|Image$$ZI$$Base|	; Base and limit of area
	IMPORT	|Image$$ZI$$Limit|	; to zero initialise

	LDR     r0, =|Image$$RO$$Limit| ; Get pointer to ROM data
	LDR     r1, =|Image$$RW$$Base|  ; and RAM copy
	LDR     r3, =|Image$$ZI$$Base|  ; Zero init base => top of initialised data
	CMP     r0, r1                  ; Check that they are different
	BEQ     %1
0	CMP     r1, r3                  ; Copy init data
	LDRCC   r2, [r0], #4
	STRCC   r2, [r1], #4
	BCC     %0
1	LDR     r1, =|Image$$ZI$$Limit| ; Top of zero init segment
	MOV     r2, #0
2	CMP     r3, r1                  ; Zero init
	STRCC   r2, [r3], #4
	BCC     %2

; --- Now we enter the C code

	IMPORT	C_Entry
    [ :DEF:THUMB
	ORR	lr, pc, #1
	BX	lr
	CODE16				; Next instruction will be Thumb
    ]
	BL	C_Entry
; In a real application we wouldn't normally expect to return, however
; this example does so the debug monitor swi SWI_Exit is used to halt the
; application.
	SWI	SWI_Exit

	END
