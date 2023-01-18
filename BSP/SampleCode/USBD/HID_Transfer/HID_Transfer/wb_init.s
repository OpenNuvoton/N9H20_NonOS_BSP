;/**************************************************************************//**
; * @file     wb_init.s
; * @brief    N9H20 series startup code
; *
; * SPDX-License-Identifier: Apache-2.0
; * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
; *****************************************************************************/

    AREA WB_INIT, CODE, READONLY

;--------------------------------------------
; Mode bits and interrupt flag (I&F) defines
;--------------------------------------------
USR_MODE    EQU    0x10
FIQ_MODE    EQU    0x11
IRQ_MODE    EQU    0x12
SVC_MODE    EQU    0x13
ABT_MODE    EQU    0x17
UDF_MODE    EQU    0x1B
SYS_MODE    EQU    0x1F

I_BIT       EQU    0x80
F_BIT       EQU    0x40

;----------------------------
; System / User Stack Memory
;----------------------------
 IF :DEF:N9H20K1
RAM_Limit       EQU     0x200000            ; For unexpanded hardware board
 ENDIF

 IF :DEF:N9H20K3
RAM_Limit       EQU     0x800000            ; For unexpanded hardware board
 ENDIF

 IF :DEF:N9H20K5
RAM_Limit       EQU     0x2000000           ; For unexpanded hardware board
 ENDIF


UND_Stack       EQU     RAM_Limit
Abort_Stack     EQU     RAM_Limit-256
IRQ_Stack       EQU     RAM_Limit-512       ; followed by IRQ stack
FIQ_Stack       EQU     RAM_Limit-768       ; followed by IRQ stack
SVC_Stack       EQU     RAM_Limit-1024      ; SVC stack at top of memory
USR_Stack       EQU     RAM_Limit


    ENTRY
    EXPORT  Reset_Go


        EXPORT  Vector_Table
Vector_Table
        B       Reset_Go    ; Modified to be relative jumb for external boot
        LDR     PC, Undefined_Addr
        LDR     PC, SWI_Addr
        LDR     PC, Prefetch_Addr
        LDR     PC, Abort_Addr
        DCD     0x0
        LDR     PC, IRQ_Addr
        LDR     PC, FIQ_Addr

        
Reset_Addr      DCD     Reset_Go
Undefined_Addr  DCD     Undefined_Handler
SWI_Addr        DCD     SWI_Handler1
Prefetch_Addr   DCD     Prefetch_Handler
Abort_Addr      DCD     Abort_Handler
                DCD     0
IRQ_Addr        DCD     IRQ_Handler
FIQ_Addr        DCD     FIQ_Handler


; ************************
; Exception Handlers
; ************************

; The following dummy handlers do not do anything useful in this example.
; They are set up here for completeness.

Undefined_Handler
        B       Undefined_Handler
SWI_Handler1
        B       SWI_Handler1     
Prefetch_Handler
        B       Prefetch_Handler
Abort_Handler
        B       Abort_Handler
IRQ_Handler
        B       IRQ_Handler
FIQ_Handler
        B       FIQ_Handler


Reset_Go

;--------------------------------
; Initial Stack Pointer register
;--------------------------------
;INIT_STACK 
 MSR    CPSR_c, #UDF_MODE :OR: I_BIT :OR: F_BIT
 LDR     SP, =UND_Stack

 MSR    CPSR_c, #ABT_MODE :OR: I_BIT :OR: F_BIT
 LDR     SP, =Abort_Stack

 MSR    CPSR_c, #IRQ_MODE :OR: I_BIT :OR: F_BIT
 LDR     SP, =IRQ_Stack

 MSR    CPSR_c, #FIQ_MODE :OR: I_BIT :OR: F_BIT
 LDR     SP, =FIQ_Stack

 MSR    CPSR_c, #SYS_MODE :OR: I_BIT :OR: F_BIT
 LDR     SP, =USR_Stack

 MSR    CPSR_c, #SVC_MODE :OR: I_BIT :OR: F_BIT
 LDR     SP, =SVC_Stack

;------------------------------------------------------
; Set the normal exception vector of CP15 control bit    
;------------------------------------------------------    
    MRC p15, 0, r0 , c1, c0      ; r0 := cp15 register 1
    BIC r0, r0, #0x2000          ; Clear bit13 in r1
    MCR p15, 0, r0 , c1, c0      ; cp15 register 1 := r0

 IF :DEF:SYS_INIT
;-----------------------------
; system initialization 
;------------------------------
;LDR    r0, =0xFFF00004


 ldr    r0, =0xFFF00004
 ldr    r1, =0x00000540
 str    r1, [r0] 

 ldr    r0, =0xFFF0000C
 ldr    r1, =0x00004F47 ; 240MHz: 4F47, 192MHz: 3F47
 str    r1, [r0] 

 ldr    r0, =0XFFF00010
 ldr    r1, =0x001FFFFF
 str    r1, [r0] 

 ldr    r0, =0xFFF00014 
 ldr    r1, =0x104514BB
 str    r1, [r0] 

 ldr    r0, =0xFFF00020
 ldr    r1, =0x00000009
 str    r1, [r0] 

 ldr    r0, =0xFFF00030
 ldr    r1, =0x00000000
 str    r1, [r0] 

 ldr    r0, =0XFFF00034
 ldr    r1, =0x00000000
 str    r1, [r0] 

 ldr    r0, =0xFFF00038
 ldr    r1, =0x000001AB
 str    r1, [r0] 

 ldr    r0, =0xFFF0003C
 ldr    r1, =0x00000050  ;CPU:AHP:APB 1:2:4 0x50, CPU:AHB:APB 1:1:2 0x41
 str    r1, [r0] 

 ldr    r0, =0xFFF00040
 ldr    r1, =0x60000000
 str    r1, [r0] 

 ldr    r0, =0xFFF01000
 ldr    r1, =0x00154201
 str    r1, [r0] 

 ldr    r0, =0xFFF01004
 ldr    r1, =0x4006FFF6
 str    r1, [r0] 

 ldr    r0, =0xFFF01010
 ldr    r1, =0x000090CD  ;16M: 90CC, 32M: 90CD
 str    r1, [r0] 

 ldr    r0, =0xFFF01018 
 ldr    r1, =0x0000015B
 str    r1, [r0] 

 ldr    r0, =0xFFF01020 
 ldr    r1, =0x80007FFD
 str    r1, [r0] 

 ldr    r0, =0xFFF01028
 ldr    r1, =0x00000027
 str    r1, [r0] 


 ldr    r0, =0xFFF0001C
 ldr    r1, =0x00000080
 str    r1, [r0] 

 ENDIF


    IMPORT  __main
;-----------------------------
;    enter the C code
;-----------------------------
    B   __main

    END




