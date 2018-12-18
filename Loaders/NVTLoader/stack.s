;/******************************************************************************
; *
; * Copyright (c) 2008 Nuvoton Tech. Corp.
; * All rights reserved.
; *
; * $Workfile: stack.s $
; *
; * 
; ******************************************************************************/


        AREA    Stacks, DATA, NOINIT

        EXPORT top_of_stacks

; Create dummy variable used to locate stacks in memory

top_of_stacks    SPACE   1

        END
