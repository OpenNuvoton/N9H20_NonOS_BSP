/**************************************************************************//**
 * @file     wb_power.c
 * @version  V3.00
 * @brief    N9H20 series SYS driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

/****************************************************************************
* FILENAME
*   wb_power.c
*
* VERSION
*   1.0
*
* DESCRIPTION
*   The power managemnet related function of Nuvoton ARM9 MCU
*
* DATA STRUCTURES
*   None
*
* FUNCTIONS
*   sysDisableAllPM_IRQ
*   sysEnablePM_IRQ
*   sysPMStart
*
* HISTORY
*   2008-07-24  Ver 1.0 Modified by Min-Nan Cheng
*
* REMARK
*   When enter PD or MIDLE mode, the sysPMStart function will disable cache
*   (in order to access SRAM) and then recovery it after wake up.
****************************************************************************/
#include <string.h>
#include "wblib.h"


//extern BOOL		sysGetCacheState(VOID);
//extern INT32 	sysGetCacheMode(VOID);

#if defined(__GNUC__)
UINT8  _tmp_buf[PD_RAM_SIZE] __attribute__ ((aligned (32)));
#else
__align(32) UINT8 _tmp_buf[PD_RAM_SIZE];
#endif

#define SRAM_VAR_ADDR  0xFF001FF0
 /**************************************************************************
 *	The function is used to power down PLL or not if system power down
 *	If PLL Power down as system power down, the wake up time may take 25ms ~ 75ms
 *	If PLL is not powered down as system power down, the wake up time may take 3ms
 *
 *	Note: Remember that  function Sample_PowerDown() is running on SRAM. 
 *		 Because program code can't access variable on SDRAM after SDRAM entry self-refresh.
 *  	         REG_GPAFUN[31:24] is unused. 
 * 		 REG_GPAFUN[26] is used to record the paramter for power down or not power down PLL.
 *		 REG_GPAFUN[26] = 1. Power down PLL
 *		 REG_GPAFUN[26] = 0. Keep PLL running		
 ***************************************************************************/
#if defined (__GNUC__) && !defined (__CC_ARM)
#pragma GCC push_options
#pragma GCC optimize ("O0")
#endif
void sysPowerDownPLLDuringSysPowerDown(BOOL bIsPowerDownPLL)
{//GPAFUN[31:26] is unused. Use Bit 26 to judge power down PLL or not	
	if(bIsPowerDownPLL==1)
		outp32(REG_GPAFUN, inp32(REG_GPAFUN) | BIT26);
	else
		outp32(REG_GPAFUN, inp32(REG_GPAFUN) & ~BIT26);	
}	
#if defined(__CC_ARM)
#pragma diag_suppress 1287
#endif
static void Sample_PowerDown(void)
{
    register int reg3, reg2, reg1, reg0;
	/* Enter self refresh */
    //outp32(0xFF001FF0, 0xB0000200);
    reg3 = 0xB0000200;
#if defined (__GNUC__)
	asm volatile
	(
		"  mov 	%0, #100       \n"
		"  mov  %1, #0         \n"
		"  mov  %2, #1         \n"
		" loopa:	           \n"
		"  add  %1, %1, %2     \n"
		"  cmp 	%1, %0         \n"
		"  bne  loopa          \n"
		: : "r"(reg2), "r"(reg1), "r"(reg0) :"memory"
	);
#else
	__asm
	{/* Dealy */
		mov 	reg2, #100
		mov		reg1, #0
		mov		reg0, #1
	loopa:		add reg1, reg1, reg0
		cmp 	reg1, reg2
		bne		loopa
	}
#endif

	outp32(REG_SDOPM, inp32(REG_SDOPM) & ~OPMODE);
	outp32(REG_SDCMD, (inp32(REG_SDCMD) & ~(AUTOEXSELFREF | CKE_H)) | SELF_REF);	
			
	//outp32(REG_GPIOD_DOUT, inp32(REG_GPIOD_DOUT) & ~0x3);  
	/* Change the system clock souce to 12M crystal*/
	outp32(REG_CLKDIV0, (inp32(REG_CLKDIV0) & (~0x18)) );
#if defined (__GNUC__)
	asm volatile
	(
		"  mov 	%0, #100       \n"
		"  mov  %1, #0         \n"
		"  mov  %2, #1         \n"
		" loop0:	           \n"
		"  add  %1, %1, %2     \n"
		"  cmp 	%1, %0         \n"
		"  bne  loop0          \n"
		: : "r"(reg2), "r"(reg1), "r"(reg0) :"memory"
	);
#else
	__asm
	{/* Delay */
		mov 	reg2, #100
		mov		reg1, #0
		mov		reg0, #1
	loop0:	add 		reg1, reg1, reg0
		cmp 		reg1, reg2
		bne		loop0
	}
#endif
	//outp32(REG_GPIOD_DOUT, inp32(REG_GPIOD_DOUT) | 0x3);  
	if( (inp32(REG_GPAFUN)&BIT26) == BIT26){
		outp32(REG_UPLLCON, inp32(REG_UPLLCON) | PD);		/* Power down UPLL and APLL */
		outp32(REG_APLLCON, inp32(REG_APLLCON) | PD);	
	}else{
		outp32(REG_UPLLCON, inp32(REG_UPLLCON) & ~PD);		
		outp32(REG_APLLCON, inp32(REG_APLLCON) & ~PD);	
	}
#if defined (__GNUC__)
	asm volatile
	(
		"  mov 	%0, #300       \n"
		"  mov  %1, #0         \n"
		"  mov  %2, #1         \n"
		" loop1:	           \n"
		"  add  %1, %1, %2     \n"
		"  cmp 	%1, %0         \n"
		"  bne  loop1          \n"
		: : "r"(reg2), "r"(reg1), "r"(reg0) :"memory"
	);
#else
	__asm
	{
			mov 	reg2, #300
			mov		reg1, #0
			mov		reg0, #1
	loop1:	add 	reg1, reg1, reg0
			cmp 	reg1, reg2
			bne		loop1
	}
#endif
	/* Fill and enable the pre-scale for power down wake up */
	//outp32(REG_PWRCON, (inp32(REG_PWRCON)  & ~0xFFFF00) | 0xFFFF02);	//1.2 s
	if( (inp32(REG_GPAFUN)&BIT26) == BIT26)
		outp32(REG_PWRCON, (inp32(REG_PWRCON)  & ~0xFFFF00) | 0xFF02);     // 25ms ~ 75ms depends on system power and PLL character	
	else	
		outp32(REG_PWRCON, (inp32(REG_PWRCON)  & ~0xFFFF00) | 0x0002);     // 3ms wake up
#if defined (__GNUC__)

	asm volatile
	(
		"  mov 	%0, #100       \n"
		"  mov  %1, #0         \n"
		"  mov  %2, #1         \n"
		" loop2:	           \n"
		"  add  %1, %1, %2     \n"
		"  cmp 	%1, %0         \n"
		"  bne  loop2          \n"
		: : "r"(reg2), "r"(reg1), "r"(reg0) :"memory"
	);
#else
	__asm
	{
		mov 	reg2, #10
		mov		reg1, #0
		mov		reg0, #1
	loop2:	add reg1, reg1, reg0
		cmp 	reg1, reg2
		bne		loop2
	}
#endif
	//outp32(REG_GPIOD_DOUT, (inp32(REG_GPIOD_DOUT) & ~0x03) | 0x1);
	/*  Enter power down. (Stop the external clock */
#if defined (__GNUC__)
	asm volatile
	(
		"MOV     %0,#0xb0000000 \n"
        "LDR     %0,[%0,#0x200] \n"
		"BIC     %0,%0,#0x1     \n"
		"MOV     %1,#0xb0000000 \n"
		"STR     %0,[%1,#0x200] \n"
		: : "r"(reg3), "r"(reg1), "r"(reg0):
    );
#else
	__asm 
	{/* Power down */
		MOV     reg3,#0xb0000000 
        LDR     reg3,[reg3,#0x200] 
		BIC     reg3,reg3,#0x1     
		MOV     reg2,#0xb0000000 
		STR     reg3,[reg2,#0x200] 
	}   
#endif

#if defined (__GNUC__)
	asm volatile
	(
		"  mov 	%0, #300       \n"
		"  mov  %1, #0         \n"
		"  mov  %2, #1         \n"
		" loop3:	           \n"
		"  add  %1, %1, %2     \n"
		"  cmp 	%1, %0         \n"
		"  bne  loop3          \n"
		: : "r"(reg2), "r"(reg1), "r"(reg0) :"memory"
	);
#else
	__asm
	{/* Wake up*/ 
			mov 	reg2, #300
			mov		reg1, #0
			mov		reg0, #1
	loop3:	add 	reg1, reg1, reg0
			cmp 	reg1, reg2
			bne		loop3
	}
#endif
	//outp32(REG_GPIOD_DOUT, (inp32(REG_GPIOD_DOUT) & ~0x03) | 0x2);
	

 		
	 /* Force UPLL and APLL in normal mode */ 	 
	outp32(REG_UPLLCON, inp32(REG_UPLLCON) & (~PD));		
	outp32(REG_APLLCON, inp32(REG_APLLCON) & (~PD));	
	if( (inp32(REG_GPAFUN)&BIT26) == BIT26)
	{//Waitting for PLL stable if enable PLL again
#if defined (__GNUC__)
	asm volatile
	(
		"  mov 	%0, #500       \n"
		"  mov  %1, #0         \n"
		"  mov  %2, #1         \n"
		" loop4:	           \n"
		"  add  %1, %1, %2     \n"
		"  cmp 	%1, %0         \n"
		"  bne  loop4          \n"
		: : "r"(reg2), "r"(reg1), "r"(reg0) :"memory"
	 );
#else
		__asm
		{/* Delay a moment for PLL stable */
				mov 	reg2, #500
				mov		reg1, #0
				mov		reg0, #1
		loop4:	add 	reg1, reg1, reg0
				cmp 	reg1, reg2
				bne		loop4
		}
#endif
	}	
	/* Change system clock to PLL and delay a moment.  Let DDR/SDRAM lock the frequency */
	outp32(REG_CLKDIV0, inp32(REG_CLKDIV0) | 0x18);	

#if defined (__GNUC__)
	asm volatile
	(
		"  mov 	%0, #500       \n"
		"  mov  %1, #0         \n"
		"  mov  %2, #1         \n"
		" loop5:	           \n"
		"  add  %1, %1, %2     \n"
		"  cmp 	%1, %0         \n"
		"  bne  loop5          \n"
		: : "r"(reg2), "r"(reg1), "r"(reg0) :"memory"
	);
#else
	__asm
	{
			mov 	reg2, #500
			mov		reg1, #0
			mov		reg0, #1
	loop5:	add 	reg1, reg1, reg0
			cmp 	reg1, reg2
			bne		loop5
	}
#endif

	
	/* Set CKE to Low and escape self-refresh mode, Force DDR/SDRAM escape self-refresh */
	outp32(0xB0003004,  0x20);
#if defined (__GNUC__)
	asm volatile
	(
		"  mov 	%0, #100       \n"
		"  mov  %1, #0         \n"
		"  mov  %2, #1         \n"
		" loop6:	           \n"
		"  add  %1, %1, %2     \n"
		"  cmp 	%1, %0         \n"
		"  bne  loop6          \n"
		: : "r"(reg2), "r"(reg1), "r"(reg0) :"memory"
	);
#else
	__asm
	{/*  Delay a moment until the escape self-refresh command reached to DDR/SDRAM */
			mov 	reg2, #100
			mov		reg1, #0
			mov		reg0, #1
	loop6:	add 	reg1, reg1, reg0
			cmp 	reg1, reg2
			bne		loop6
	}			
#endif
}
#if defined (__GNUC__) && !defined (__CC_ARM)
#pragma GCC pop_options
#endif

static void Entry_PowerDown(UINT32 u32WakeUpSrc)
{
	UINT32 j;
	UINT32 u32IntEnable;
	void (*wb_fun)(void);
	
	UINT32 u32RamBase = PD_RAM_BASE;
	UINT32 u32RamSize = PD_RAM_SIZE;	
	BOOL bIsEnableIRQ = FALSE;
	
	if( sysGetIBitState()==TRUE )
	{
		bIsEnableIRQ = TRUE;
		sysSetLocalInterrupt(DISABLE_IRQ);	
	}		
	memcpy((char*)((UINT32)_tmp_buf| 0x80000000), (char*)(u32RamBase | 0x80000000), u32RamSize);
	memcpy((VOID *)((UINT32)u32RamBase | 0x80000000),
			(VOID *)( ((UINT32)Sample_PowerDown -(PD_RAM_START-PD_RAM_BASE)) | 0x80000000), 
			PD_RAM_SIZE);	
	if(memcmp((char*)(u32RamBase | 0x80000000), (char*)((UINT32)((UINT32)Sample_PowerDown -(PD_RAM_START-PD_RAM_BASE)) | 0x80000000), u32RamSize)!=0)
	{
		sysprintf("Memcpy copy wrong\n");
	}
	
	
	sysFlushCache(I_CACHE);		
	//wb_fun = (void(*)(void)) u32RamBase;
	wb_fun = (void(*)(void))PD_RAM_START;
	sysprintf("Jump to SRAM (Suspend)\n");
	

	u32IntEnable = inp32(REG_AIC_IMR);	
	outp32(REG_AIC_MDCR, 0xFFFFFFFE);
	outp32(REG_AIC_MECR, 0x00000000);	
	j = 0x800;
	while(j--);
	outp32(0xb0000030, ((u32WakeUpSrc<<24)|(u32WakeUpSrc<<16)));	//Enable and Clear interrupt		
	
	wb_fun();
	
	outp32(0xb0000030, ((u32WakeUpSrc<<24)|(u32WakeUpSrc<<16)));	//Enable and Clear interrupt
	memcpy((VOID *)u32RamBase, (VOID *)_tmp_buf, PD_RAM_SIZE);
	
	
	sysprintf("Exit to SRAM (Suspend)\n");
	outp32(REG_AIC_MECR, u32IntEnable);								/*  Restore the interrupt channels */		
	if(bIsEnableIRQ==TRUE)
		sysSetLocalInterrupt(ENABLE_IRQ);	
}
INT32 sysPowerDown(WAKEUP_SOURCE_E eWakeUpSrc)
{
	Entry_PowerDown(eWakeUpSrc);
	return Successful;
}

