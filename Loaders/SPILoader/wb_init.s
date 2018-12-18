

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
 
USR_Stack       EQU     RAM_Limit
SVC_Stack       EQU     (USR_Stack-1024)      ; SVC stack at top of memory
FIQ_Stack       EQU     (SVC_Stack-2*1024)     ; followed by IRQ stack
IRQ_Stack       EQU     (FIQ_Stack-1024)       ; followed by IRQ stack
Abort_Stack     EQU     (IRQ_Stack-1024)
UND_Stack       EQU     (Abort_Stack-1024)




    ENTRY
    EXPORT  Reset_Go

Reset_Go

;--------------------------------
; Initial Stack Pointer register, keep them here to make .axf work if NAND loader not execute first
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
    MRC p15, 0, r0 , c1, c0     ; r0 := cp15 register 1
    BIC r0, r0, #0x2000         ; Clear bit13 in r1
    MCR p15, 0, r0 , c1, c0     ; cp15 register 1 := r0 

    IMPORT  __main
;-----------------------------
;    enter the C code
;-----------------------------
    B   __main

    END
