;/******************************************************************************
; *
; * Copyright (c) 2008 Nuvoton Tech. Corp.
; * All rights reserved.
; *
; * $Workfile: vectors.s $
; *
; * 
; ******************************************************************************/


	AREA Vect, CODE, READONLY


; *****************
; Exception Vectors
; *****************

; Note: LDR PC instructions are used here, though branch (B) instructions
; could also be used, unless the ROM is at an address >32MB.

;        ENTRY
        EXPORT	Vector_Table
Vector_Table		
        LDR     PC, Reset_Addr
        LDR     PC, Undefined_Addr
        LDR     PC, SWI_Addr
        LDR     PC, Prefetch_Addr
        LDR     PC, Abort_Addr
        NOP                             ; Reserved vector
        LDR     PC, IRQ_Addr
        LDR     PC, FIQ_Addr
        
        IMPORT  Reset_Go           ; In wb_init.s
        
Reset_Addr      DCD     Reset_Go
Undefined_Addr  DCD     Undefined_Handler
SWI_Addr        DCD     SWI_Handler
Prefetch_Addr   DCD     Prefetch_Handler
Abort_Addr      DCD     Abort_Handler
				DCD		0
IRQ_Addr        DCD     IRQ_Handler
FIQ_Addr        DCD     FIQ_Handler


; ************************
; Exception Handlers
; ************************

; The following dummy handlers do not do anything useful in this example.
; They are set up here for completeness.

Undefined_Handler
        B       Undefined_Handler
SWI_Handler
		MOV		r0, #0		; Pretend it was all okay
		MOVS	pc, lr				; just ignore SWI  
Prefetch_Handler
        B       Prefetch_Handler
Abort_Handler
        B       Abort_Handler
		NOP
IRQ_Handler
        B       IRQ_Handler
FIQ_Handler
        B       FIQ_Handler
        
        END

