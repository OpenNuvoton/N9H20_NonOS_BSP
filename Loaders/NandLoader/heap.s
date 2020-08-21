;/**************************************************************************//**
; * @file     heap.s
; * @brief    NandLoader source code for heap configuration.
; *
; * SPDX-License-Identifier: Apache-2.0
; * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
; *****************************************************************************/

        AREA    Heap, DATA, NOINIT

        EXPORT bottom_of_heap

; Create dummy variable used to locate bottom of heap

bottom_of_heap    SPACE   1

        END
