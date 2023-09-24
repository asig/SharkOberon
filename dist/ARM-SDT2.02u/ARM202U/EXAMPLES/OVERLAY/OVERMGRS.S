; AUTHORS: L.Smith, J. Sutton
;
; DATE:    Last edit 16-Mar-90 (ECN)
;
; VERSION: 0.07
;
; DESCRIPTION: Overlay Manager
;
; CHANGES: 28-Feb-90  LDS
;	   Fixed a bug in check_for_invalidated_returns whereby a return
;	   handler was allocated on return rather than just on call.
;
;	   16-Mar-90 ECN
;	   Use OS_GetEnv to read the overlay directory instead of Obey$Dir
;	   which doesn't work if the image is not executed from an obey file.
;
;	   Tidied up the behaviour of "Disk not present errors". The previous
;	   version just splatted a message all over the desktop. The new
;	   version only prompt for disk insertion if executing outside the
;	   desktop (inside the desktop the wimp will prompt the user).
;
;	   Tidied up the generation of errors and exit from application.
;	   Previously either gave a trap or stiffed the machine.
;
;;; Copyright (C) Advanced RISC Machines Ltd., 1991

        EXPORT  |Image$$overlay_init| 
        EXPORT  |Image$$load_seg| 

; pointers to start and end of workspace area supplied by the linker
        IMPORT  |Overlay$$Data$$Base| 
        IMPORT  |Overlay$$Data$$Limit| 
        IMPORT  |Image$$RO$$Limit|
        IMPORT  |Image$$RW$$Base|
        IMPORT  |Image$$ZI$$Base|
	IMPORT  MemCopy
	IMPORT  LoadOverlaySegment
        IMPORT  SevereErrorHandler, WEAK 

ZeroInitCodeOffset   EQU     64 

; Why does the linker need to generate this zero init area, why can't the
; Overlay Manager define it itself??? ECN.
;
; Layout of workspace allocated by the linker pointed to by Overlay$$Data
; This area is automatically zero-initialised AFTER overlay_init is called
; offsets are prefixed with Work_
        ^       0 
Work_HStack   #       4 ; top of stack of allocated handlers 
Work_HFree    #       4 ; head of free-list 
Work_RSave    #       9*4 ; for R0-R8 
Work_LRSave   #       4 ; saved lr 
Work_PCSave   #       4 ; saved PC 
Work_PSRSave  #       4 ; saved CPSR
Work_ReturnHandlersArea   EQU     @ ; rest of this memory is treated as heap
                        ; space for the return handlers 
Work_MinSize   EQU     @ + 32 * RHandl_Size 

; Return handler. 1 is allocated per inter-segment procedure call
; allocated and free lists of handlers are pointed to from HStack and HFree
; offsets are prefixed with RHandl_
        ^       0 
RHandl_Branch   #       4 ; BL load_seg_and_ret 
RHandl_RealLR   #       4 ; space for the real return address 
RHandl_Segment  #       4 ; -> PCIT section of segment to load 
RHandl_Link     #       4 ; -> next in stack order 
RHandl_Size   EQU     @ 

; set up by check_for_invalidated_returns.

; PCITSection. 1 per segment stored in root segment, allocated by linker
; offsets are prefixed with PCITSect_
        ^       0 
PCITSect_Vecsize   #       4 ; .-4-EntryV ; size of entry vector 
PCITSect_Base   #       4 ; used by load_segment; not initialised 
PCITSect_Limit  #       4 ; used by load_segment; not initialised 
PCITSect_Name   #       11 ; <11 bytes> ; 10-char segment name + NUL in 11 bytes 
PCITSect_Flags   #       1 ; ...and a flag byte 
PCITSect_ClashSz #       4 ; PCITEnd-.-4 ; size of table following 
PCITSect_Clashes #       4 ; >table of pointers to clashing segments 

; Stack structure  (all offsets are negative)
; defined in procedure call standard
; offsets are prefixed with Stack_
        ^       0 
Stack_SaveMask  #       -4 
Stack_LRReturn  #       -4 
Stack_SPReturn  #       -4 
Stack_FPReturn  #       -4 

; the code and private workspace area
        AREA    OverLayMgrArea, PIC, CODE , READONLY

STRLR   STR     lr, [pc, #-8] ; a word that is to be matched in PCITs 

; Store 2 words which are the addresses of the start and end of the workspace
WorkSpace   DCD     |Overlay$$Data$$Base| 
WorkSpaceEnd   DCD     |Overlay$$Data$$Limit| 
InitFlag    DCD		InitDoneFlag

|Image$$overlay_init|   ROUT    
; Initialise overlay manager. 
; In the AIF format this is is called from offset 8 in header. This routine has
; to pass control to the zero initialisation code.
; In Non AIF formats this routine has to be called explicitly e.g. from
; a customised rtstand.s .
;
; In the AIF example the entire root segment is copied to the load address.
;
; In the non AIF example only the RW part of the root is copied.
; 
	LDR	ip,WorkSpace
        IF :DEF:AIF_EXAMPLE
		STR	lr,[ip,#Work_LRSave]

		ADD	ip,ip,#Work_RSave
		STMIA	ip,{r0-r3}

;
; Look at the AIF header and add compute the total size of the AIF header
;
		SUB     r1,lr,#0xC

		LDR	r0,[r1,#0x28]
	
        	LDR	r2,[r1,#0x14]
		LDR	r3,[r1,#0x18]
		ADD	r2,r2,r3
		LDR	r3,[r1,#0x1C]
		ADD	r2,r2,r3

;
; A system specific Memory Copy call. r0 is the destination area.
; r1 is the source area.
; r2 is the block size in bytes.
;
		BL	MemCopy
		LDR	ip,WorkSpace
		LDR	lr,[ip,#Work_LRSave]
		ADD	ip,ip,#Work_RSave
        	SUB	lr,lr,#12
		LDR	lr,[lr,#0x28]
		ADD	lr,lr,#12
		LDMIA	ip,{r0-r3}
;
;  Return to the Zero Initialisation Code in the AIF header.
;	
        	ADD	pc,lr,#ZeroInitCodeOffset-12
        ELSE
		LDR	ip,WorkSpace
		ADD	ip,ip,#Work_RSave
		STMIA	ip,{r0-r2,lr}

        	LDR	r0,=|Image$$RW$$Base|
        	LDR	r1,=|Image$$RO$$Limit|
        	LDR	r2,=|Image$$ZI$$Base|
		SUB	r2,r2,r0
        	BL 	MemCopy

		LDR	ip,WorkSpace
		ADD	ip,ip,#Work_RSave
		LDMIA	ip,{r0-r2,PC}
        ENDIF

        DCD     0   ; Not needed for a real system but needed when using ARMSD
		    ; is used to simulate a ROMN based system.
; entry point
|Image$$load_seg|   ROUT    
; called when segment has been called but is not loaded
; presume ip is corruptible by this
        LDR     ip, WorkSpace 
        ADD     ip, ip, #Work_RSave 
        STMIA   ip, {r0-r8} ; save working registers 
        MRS     r4, CPSR    ; Save status register -it'll get stored in the
                            ; workspace later.
; (save in my workspace because stack is untouchable during procedure call)
	LDR	r0, InitFlag
        LDRB    r1, [r0]
        CMP     r1, #0 
        BNE     InitDone 

;Initialise Return Handlers on first call to this routine
        MOV     r1, #1 
        STRB    r1, [r0] ; set InitDone flag 
        LDR     r0, WorkSpace 
; r0 points to workspace
; corrupts r0-r3,lr
; create and initialise return handler linked list
        MOV     r2, #0 
        STR     r2, [r0, #Work_HStack] ; initialise start of handler list with NULL 
        ADD     r1, r0, #Work_ReturnHandlersArea ; Start of heap space 
        STR     r1, [r0, #Work_HFree] ; Start of list of free handlers point to heap space 
        LDR     r0, WorkSpaceEnd ; for test in loop to make sure.. 
        SUBS    r0, r0, #RHandl_Size ; ..I dont overrun in init 
01      ADD     r3, r1, #RHandl_Size ; next handler 
; set up link to point to next handler (in fact consecutive locations)
        STR     r3, [r1, #RHandl_Link] 
        MOV     r1, r3 ; next handler 
        CMP     r1, r0 ; test for end of workspace 
        BLT     %BT01 
        SUB     r1, r1, #RHandl_Size ; previous handler 
        STR     r2, [r1, #RHandl_Link] ; NULL-terminate list 

InitDone
        LDR     r3, WorkSpace 
        STR     r4, [r3, #Work_PSRSave] ; CPSR read into R4 before the InitDone
					; test.
        MOV     r8,lr                   ;
        LDR     r0, [r8, #-8] ; saved r14... (is end of PCIT) 
        STR     r0, [r3, #Work_LRSave] ; ...save it here ready for retry 
        LDR     r0, STRLR ; look for this... 
        SUB     r1, r8, #8 ; ... starting at last overwrite 
01      LDR     r2, [r1, #-4]! 
        CMP     r2, r0 ; must stop on guard word... 
        BNE     %B01 
        ADD     r1, r1, #4 ; gone one too far... 
        STR     r1, [r3, #Work_PCSave] ; where to resume at 

load_segment

; ip -> the register save area; r8 -> the PCIT section of the segment to load.
; First re-initialise the PCIT section (if any) which clashes with this one...

        ADD     r1, r8, #PCITSect_Clashes 
        LDR     r0, [r8, #PCITSect_ClashSz] 
01      SUBS    r0, r0, #4 
        BLT     Done_Reinit ; nothing left to do 
        LDR     r7, [r1], #4 ; a clashing segment... 
        LDRB    r2, [r7, #PCITSect_Flags] ; its flags (0 if unloaded) 
        CMPS    r2, #0 ; is it loaded? 
        BEQ     %B01 ; no, so look again 

; clashing segment is loaded (clearly, there can only be 1 such segment)
; mark it as unloaded and reinitialise its PCIT
; r7 -> PCITSection of clashing loaded segment

        MOV     r0, #0 
        STRB    r0, [r7, #PCITSect_Flags] ; mark as unloaded 
        LDR     r0, [r7, #PCITSect_Vecsize] 
        SUB     r1, r7, #4 ; end of vector 
        LDR     r2, STRLR ; init value to store in the vector... 
02      STR     r2, [r1, #-4]! ;> 
        SUBS    r0, r0, #4 ;> loop to initialise the PCIT segment 
        BGT     %B02 ;> 
; Now we check the chain of call frames on the stack for return addresses

; which have been invalidated by loading this segment and install handlers
; for each invalidated return.
; Note: r8 identifies the segment being loaded; r7 the segment being unloaded.

        BL      check_for_invalidated_returns 
Done_Reinit

; All segment clashes have now been dealt with, as have the re-setting
; of the segment-loaded flags and the intercepting of invalidated returns.
; So, now load the required segment.

Retry
;
;       Call a routine to load the overlay segment.
;	First parameter is the length of the segment name.
;	The second parameter is the address of the segment name
;	The third parameter is the base address of the segement.
;	The routine returns the segment length in r0.
;
        MOV     r0,#12
        ADD     r1, r8, #PCITSect_Name
        LDR	r2, [ r8, #PCITSect_Base]
        BL      LoadOverlaySegment
	
	TEQ	r0,#0
	MOVEQ   r0,#2
        BEQ	SevereErrorHandler

	LDR	ip,WorkSpace
	ADD	ip,ip,#Work_RSave
;
;       Mark the segment as loaded.
;
        MOV     r1,#1
        STRB    r1, [r8, #PCITSect_Flags]

        LDR     r2, [r8, #PCITSect_Base]
        ADD     r0, r2, r0 ; start + length = end of file 

; The segment's entry vector is at the end of the segment...
; ...copy it to the PCIT section identified by r8.

        LDR     r1, [r8, #PCITSect_Vecsize] 
        SUB     r3, r8, #8 ; end of entry vector... 
        MOV     r4, #0 ; for data initialisation 
01      LDR     r2, [r0, #-4]! ;>loop to copy 
        STR     r4, [r0] ; (zero-init possible data section) 
        STR     r2, [r3], #-4 ;>the segment's PCIT 
        SUBS    r1, r1, #4 ;>section into the 
        BGT     %B01 ;>global PCIT 

; Finally, continue, unabashed...

        LDR	r3, WorkSpace
        LDR	r3, [r3,#Work_PSRSave]
        MSR	CPSR,r3
        
        LDMIA   ip, {r0-r8, lr, pc} 

load_seg_and_ret
; presume ip is corruptible by this
        LDR     ip, WorkSpace 
        ADD     ip, ip, #Work_RSave 
        STMIA   ip, {r0-r8} ; save working registers 
; (save in my workspace because stack is untouchable during procedure call)
        LDR     r3, WorkSpace 
	MRS	r8, CPSR
        STR     r8, [r3, #Work_PSRSave]

; lr points to the return handler
        MOV     r8, lr
        ;       load return handler fields RealLR, Segment, Link 
        LDMIA   r8, {r0, r1, r2} 
        SUB     r8, r8, #4 ; point to true start of return handler before BL 
        STR     r0, [r3, #Work_LRSave] 
        STR     r0, [r3, #Work_PCSave] 
; Now unchain the handler and return it to the free pool
; HStack points to this handler
        LDR     r0, [r3, #Work_HStack] 
        CMPS    r0, r8 
        MOVNE   r0, #1
        BNE     SevereErrorHandler
        STR     r2, [r3, #Work_HStack] ; new top of handler stack 
        LDR     r2, [r3, #Work_HFree] 
        STR     r2, [r8, #RHandl_Link] ; Link -> old HFree 
        STR     r8, [r3, #Work_HFree] ; new free list 
        MOV     r8, r1 ; segment to load 
        B       load_segment 

check_for_invalidated_returns
; Note: r8 identifies the segment being loaded; r7 the segment being unloaded.
; Note: check for returns invalidated by a call NOT for returns invalidated by
;	a return! In the 2nd case, the saved LR and saved PC are identical.
        LDR     r5, WorkSpace 
        ADD     r6, r5, #Work_LRSave ; 1st location to check 
        LDMIA   r6, {r0, r1} ; saved LR & PC 
        CMPS    r0, r1 
        MOVEQ   pc, lr ; identical => returning... 
        MOV     r0, fp ; temporary FP... 
01      LDR     r1, [r6] ; the saved return address... 
        LDR     r2, [r5, #Work_HStack] ; top of handler stack 
        CMPS    r1, r2 ; found the most recent handler, so 
        MOVEQ   pc, lr ; abort the search 
        LDR     r2, [r7, #PCITSect_Base] 
        CMPS    r1, r2 ; see if >= base... 
        BLT     %F02 
        LDR     r2, [r7, #PCITSect_Limit] 
        CMPS    r1, r2 ; ...and < limit ? 
        BLT     FoundClash 
02      CMPS    r0, #0 ; bottom of stack? 
        MOVEQ   pc, lr ; yes => return 
        ADD     r6, r0, #Stack_LRReturn 
        LDR     r0, [r0, #Stack_FPReturn] ; previous FP 
        B       %B01 
FoundClash
        LDR     r0, [r5, #Work_HFree] ; head of chain of free handlers 
        CMPS    r0, #0 
        MOVEQ   r0, #2
        BEQ     SevereErrorHandler
; Transfer the next free handler to head of the handler stack.
        LDR     r1, [r0, #RHandl_Link] ; next free handler 
        STR     r1, [r5, #Work_HFree] 
        LDR     r1, [r5, #Work_HStack] ; the active handler stack 
        STR     r1, [r0, #RHandl_Link] 
        STR     r0, [r5, #Work_HStack] ; now with the latest handler linked in 
; Initialise the handler with a BL load_seg_and_ret, RealLR and Segment.
        ADR     r1, load_seg_and_ret 
        SUB     r1, r1, r0 ; byte offset for BL in handler 
        SUB     r1, r1, #8 ; correct for PC off by 8 
        MOV     r1, r1, ASR #2 ; word offset 
        BIC     r1, r1, #&FF000000 
        ORR     r1, r1, #&EB000000 ; code for BL 
        STR     r1, [r0, #RHandl_Branch] 

        LDR     r1, [r6] ; LRReturn on stack 
        STR     r1, [r0, #RHandl_RealLR] ; RealLR 
        STR     r0, [r6] ; patch stack to return to handler 

        STR     r7, [r0, #RHandl_Segment] ; segment to re-load on return 
        MOV     pc, lr ; and return 

	AREA	OverlayInit, DATA
InitDoneFlag	DCD 0

        END     
