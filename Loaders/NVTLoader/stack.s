;/**************************************************************************//**
; * @file     stack.s
; * @brief    N9H20 series stack code
; *
; * SPDX-License-Identifier: Apache-2.0
; * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
; *****************************************************************************/

        AREA    Stacks, DATA, NOINIT

        EXPORT top_of_stacks

; Create dummy variable used to locate stacks in memory

top_of_stacks    SPACE   1

        END
