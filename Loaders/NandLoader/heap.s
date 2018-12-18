;/******************************************************************************
; *
; * Copyright (c) 2008 Nuvoton Tech. Corp.
; * All rights reserved.
; *
; * $Workfile: heap.s $
; *
; * 
; ******************************************************************************/


        AREA    Heap, DATA, NOINIT

        EXPORT bottom_of_heap

; Create dummy variable used to locate bottom of heap

bottom_of_heap    SPACE   1

        END
