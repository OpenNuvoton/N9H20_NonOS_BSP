;/**************************************************************************//**
; * @file     wb_dcache.s
; * @brief    N9H20 series dcache code
; *
; * SPDX-License-Identifier: Apache-2.0
; * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
; *****************************************************************************/
	
	AREA MEM_INIT, CODE, READONLY
	
	EXPORT	sys_flush_and_clean_dcache

sys_flush_and_clean_dcache
		
tci_loop
	MRC p15, 0, r15, c7, c14, 3 ; test clean and invalidate
	BNE tci_loop		
 	BX  r14
		
	END
	