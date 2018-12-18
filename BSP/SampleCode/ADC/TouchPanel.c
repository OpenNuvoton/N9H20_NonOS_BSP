#include <stdio.h>
#include "N9H20.h"
char u8Tmp[0x1000];
extern void Sample_PowerDown(void);
//extern void pfnRecordCallback(void);

#define POWER_DOWN
//static volatile BOOL g_i8PcmReady = FALSE;
static void pfnWaitForTriggerCallback(void)
{
	//g_i8PcmReady = 1;
	//TBD...
}
static void pfnRecordCallback(void)
{
	//g_i8PcmReady = 1;
	//TBD...
}
#pragma diag_suppress 1287
void Sample_PowerDown(void)
{	
	register int reg2, reg1, reg0;
	/* Enter self refresh */	
	outp32(REG_SDCMD, inp32(REG_SDCMD) | 0x10);				
	__asm
	{/* Dealy */
		mov 	reg2, #100
		mov		reg1, #0
		mov		reg0, #1
loopa:	add 	reg1, reg1, reg0
		cmp 	reg1, reg2
		bne		loopa
	}	
	/* Change the system clock souce to 12M crystal*/
	outp32(REG_CLKDIV0, (inp32(REG_CLKDIV0) & (~0x18)) );			
	__asm
	{/* Dealy */
		mov 	reg2, #100
		mov		reg1, #0
		mov		reg0, #1
loop0:	add 	reg1, reg1, reg0
		cmp 	reg1, reg2
		bne		loop0
	}		
	outp32(REG_UPLLCON, inp32(REG_UPLLCON) | 0x10000);		/* Power down UPLL and APLL */
	outp32(REG_APLLCON, inp32(REG_APLLCON) | 0x10000);		
	__asm
	{
		mov 	reg2, #300
		mov		reg1, #0
		mov		reg0, #1
loop1:	add 	reg1, reg1, reg0
		cmp 	reg1, reg2
		bne		loop1
	}	
	/* Fill and enable the pre-scale for power down wake up */
	outp32(REG_PWRCON, (inp32(REG_PWRCON)  & ~0xFFFF00) | 0xFFFF02);	
	
	__asm
	{
		mov 	reg2, #10
		mov		reg1, #0
		mov		reg0, #1
loop2:	add 	reg1, reg1, reg0
		cmp 	reg1, reg2
		bne		loop2
	}
	/*  Enter power down. (Stop the external clock */
	__asm 
    	{/* Power down */
    		mov 	reg2, 0xb0000200
    		ldmia 	reg2, {reg0}
    		bic		reg0, reg0, #0x01
    		stmia 	reg2, {reg0}
    	}
	__asm
	{/* Wake up*/ 
		mov 	reg2, #5000
		mov		reg1, #0
		mov		reg0, #1
loop3:	add 	reg1, reg1, reg0
		cmp 	reg1, reg2
		bne		loop3
	}		
	 /* Force UPLL and APLL in normal mode */ 
	outp32(REG_UPLLCON, inp32(REG_UPLLCON) & (~0x10000));		
	outp32(REG_APLLCON, inp32(REG_APLLCON) & (~0x10000));	

	__asm
	{/* Delay a moment for PLL stable */
			mov 	reg2, #3000
			mov		reg1, #0
			mov		reg0, #1
loop4:	    add 	reg1, reg1, reg0
			cmp 	reg1, reg2
			bne		loop4
	}		
	/* Change system clock to PLL and delay a moment.  Let DDR/SDRAM lock the frequency */
	outp32(REG_CLKDIV0, inp32(REG_CLKDIV0) | 0x18);	
	__asm
	{
			mov 	reg2, #500
			mov		reg1, #0
			mov		reg0, #1
loop5:		add 	reg1, reg1, reg0
			cmp 	reg1, reg2
			bne		loop5
	}
	/*Force DDR/SDRAM escape self-refresh */
	outp32(0xB0003004,  0x20);				
	__asm
	{/*  Delay a moment until the escape self-refresh command reached to DDR/SDRAM */
			mov 	reg2, #500
			mov		reg1, #0
			mov		reg0, #1
	loop6:	add 	reg1, reg1, reg0
			cmp 	reg1, reg2
			bne		loop6
	}				
}
INT32 Entry_PowerDown(void)
{
	UINT32 u32IntEnable;
	void (*wb_fun)(void);
	UINT32 u32RamBase= 0xFF000000;
	UINT32 u32RamSize= 0x1000;		
	memcpy((char*)(u32RamBase | 0x80000000), (char*)((UINT32)Sample_PowerDown | 0x80000000), u32RamSize);
	if(memcmp((char*)(u32RamBase | 0x80000000), (char*)((UINT32)Sample_PowerDown | 0x80000000), u32RamSize)!=0)
	{
		sysprintf("Memcpy copy wrong\n");
		return -1;
	}	
	wb_fun = (void(*)(void)) u32RamBase;
	sysprintf("Jump to SRAM (Power down)\n");
	u32IntEnable = inp32(REG_AIC_IMR);	
	outp32(REG_AIC_MDCR, 0xFFFFFFFF);
	outp32(0xb0000030, 0x40400000);			/*  Enable the wake up interrupt channel. */
	wb_fun();
	outp32(0xb0000030, 0x40400000);			/*  Clear the wake up interrupt status */
	outp32(REG_AIC_MECR, u32IntEnable);	/*  Restore the interrupt channels */
	sysprintf("Exit to SRAM (Wake up)\n");
	return Successful;
}


void Smpl_TscPanel_WT_Powerdown_Wakeup(void)
{
	PFN_ADC_CALLBACK pfnOldCallback;
	UINT32 u32Count=1;
		
	adc_enableInt(eADC_WT_INT);
	  
	adc_installCallback(eADC_WT_INT,
							pfnWaitForTriggerCallback,
							&pfnOldCallback);
	adc_enableInt(eADC_ADC_INT);							
	adc_installCallback(eADC_ADC_INT,
							pfnRecordCallback,
							&pfnOldCallback);							
	outp32(REG_GPIOA_OMD, inp32(REG_GPIOA_OMD) | 0x01);	//GPIO output mode
	outp32(REG_GPIOA_PUEN, inp32(REG_GPIOA_PUEN) | 0x01);	//GPIO to high	
	outp32(REG_GPIOA_DOUT, inp32(REG_GPIOA_DOUT) | 0x01);	//GPIO to high	

	outp32(REG_GPBFUN, inp32(REG_GPBFUN) & ~3);
	outp32(REG_GPIOB_OMD, inp32(REG_GPIOB_OMD) | 0x01);	//GPIOB0 output mode
	outp32(REG_GPIOB_PUEN, inp32(REG_GPIOB_PUEN) | 0x01);	//GPIOB0 to high	
	outp32(REG_GPIOB_DOUT, inp32(REG_GPIOB_DOUT) | 0x01);	//GPIOB0 to high	

	while(1)
	{/* Power down wake up for test 30 times */										
		adc_setTouchScreen(eADC_TSCREEN_TRIG,	//E_DRVADC_TSC_MODE eTscMode,
							180,						//Delay cycle
							TRUE,					//BOOL bIsPullup,
							FALSE);					//BOOL bMAVFilter
		sysDelay(50);	
		sysprintf("\n");
		if(Entry_PowerDown()!=Successful)
		{
			sysprintf("Memory copy error \n");
			break;	
		}	
		outp32(0xb0000030, inp32(0xb0000030)| 0x40000000);	//Clear ADC wakeup status										
		sysprintf("Wake up!!! %d \n", u32Count);									
		u32Count = u32Count +1;
	}		
}	
