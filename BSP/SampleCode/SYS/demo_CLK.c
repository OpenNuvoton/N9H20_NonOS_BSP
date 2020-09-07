/**************************************************************************//**
 * @file     demo_CLK.c
 * @brief    Sample code to demostrate the APIs related to clock
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "N9H20.h"
#include "demo.h"

/* =============================================================
	This enumation is only for clock test mode. 
	Specified clock will be outputed through pin SEN_CLK 
	The register is located CLK_BA[0x30]
==============================================================*/


void DemoAPI_CLK(void)
{
	UINT32 u32Item; 
	UINT32 u32PllOutKHz;	
	UINT32 u32SysSrc;	
	DBG_PRINTF("Please Choise combination clock  ...\n");
	DBG_PRINTF("[1] UPLL=192MHz. SYS=192MHz. CPU=192MHz. HCLK=96MHz. APB=48MHz ...\n");	
	DBG_PRINTF("[2] UPLL=192MHz. SYS=192MHz. CPU=96MHz. HCLK=96MHz. APB=48MHz ...\n");	
	DBG_PRINTF("[3] UPLL=192MHz. SYS=96MHz. CPU=96MHz. HCLK=48MHz. APB=24MHz ...\n");
	DBG_PRINTF("[4] APLL=192MHz. SYS=192MHz. CPU=192MHz. HCLK=96MHz. APB=48MHz ...\n");	
	DBG_PRINTF("[5] APLL=192MHz. SYS=192MHz. CPU=96MHz. HCLK=96MHz. APB=48MHz ...\n");	
	DBG_PRINTF("[6] APLL=192MHz. SYS=96MHz. CPU=96MHz. HCLK=48MHz. APB=24MHz ...\n");
	DBG_PRINTF("[7] EXT= 12MHz. SYS=12MHz. CPU=12MHz. HCLK=6MHz. APB=3MHz ...\n");
	DBG_PRINTF("****Note: Test the function by SPI, SD, NAND or USB booting****\n");
	DBG_PRINTF("****System with 12M external clock may crash if run it by ICE****\n");
	u32Item = sysGetChar();
	DBG_PRINTF("You choice item %s..\n", u32Item);
	switch(u32Item)
	{	
		case '1':	
				sysSetSystemClock(eSYS_UPLL, 	//E_SYS_SRC_CLK eSrcClk,	
								192000,		//UINT32 u32PllKHz, 	
								192000,		//UINT32 u32SysKHz,
								192000,		//UINT32 u32CpuKHz,
								  96000,		//UINT32 u32HclkKHz,
								  48000);		//UINT32 u32ApbKHz	
				u32SysSrc = eSYS_UPLL;														
				break;						
		case '2':	
				sysSetSystemClock(eSYS_UPLL, 	//E_SYS_SRC_CLK eSrcClk,	
								192000,		//UINT32 u32PllKHz, 	
								192000,		//UINT32 u32SysKHz,
								  96000,		//UINT32 u32CpuKHz,
								  96000,		//UINT32 u32HclkKHz,
								  48000);		//UINT32 u32ApbKHz	
				u32SysSrc = eSYS_UPLL;													
				break;	
		case '3':	
				sysSetSystemClock(eSYS_UPLL, 	//E_SYS_SRC_CLK eSrcClk,	
								192000,		//UINT32 u32PllKHz, 	
								  96000,		//UINT32 u32SysKHz,
								  96000,		//UINT32 u32CpuKHz,
								  48000,		//UINT32 u32HclkKHz,
								  24000);		//UINT32 u32ApbKHz	
				u32SysSrc = eSYS_UPLL;													
				break;			
		case '4':	
				sysSetSystemClock(eSYS_APLL, 	//E_SYS_SRC_CLK eSrcClk,	
								192000,		//UINT32 u32PllKHz, 	
								192000,		//UINT32 u32SysKHz,
								192000,		//UINT32 u32CpuKHz,
								  96000,		//UINT32 u32HclkKHz,
								  48000);		//UINT32 u32ApbKHz	
				u32SysSrc = eSYS_APLL;														
				break;						
		case '5':	
				sysSetSystemClock(eSYS_APLL, 	//E_SYS_SRC_CLK eSrcClk,	
								192000,		//UINT32 u32PllKHz, 	
								192000,		//UINT32 u32SysKHz,
								  96000,		//UINT32 u32CpuKHz,
								  96000,		//UINT32 u32HclkKHz,
								  48000);		//UINT32 u32ApbKHz	
				u32SysSrc = eSYS_APLL;													
				break;	
		case '6':	
				sysSetSystemClock(eSYS_APLL, 	//E_SYS_SRC_CLK eSrcClk,	
								192000,		//UINT32 u32PllKHz, 	
								  96000,		//UINT32 u32SysKHz,
								  96000,		//UINT32 u32CpuKHz,
								  48000,		//UINT32 u32HclkKHz,
								  24000);		//UINT32 u32ApbKHz	
				u32SysSrc = eSYS_APLL;													
				break;	
		case '7':	
				sysSetSystemClock(eSYS_EXT, 	//E_SYS_SRC_CLK eSrcClk,	
								12000,		//UINT32 u32PllKHz, 	
								  12000,		//UINT32 u32SysKHz,
								  12000,		//UINT32 u32CpuKHz,
								  6000,		//UINT32 u32HclkKHz,
								  3000);		//UINT32 u32ApbKHz	
				u32SysSrc = eSYS_EXT;													
				break;															
						
	}		
	u32PllOutKHz = sysGetPLLOutputKhz((E_SYS_SRC_CLK)u32SysSrc, 12000ul);
	sysprintf("PLL out frequency %d Khz\n", u32PllOutKHz);			
	DBG_PRINTF("Clock switch successful ...\n");
}

void DemoAPI_SetSystemDivider(void)
{
	UINT32 u32Item;
			
	sysprintf("Input system clock divider from 0~7\n");
	u32Item = sysGetChar();
	if(u32Item<'0')
		return;
	if(u32Item>'7')
		return;	
	u32Item = u32Item-0x30; 	
	sysEnableCache(CACHE_WRITE_BACK);					//Enable cache to reduce the memory power.	    				
	sysClockDivSwitchStart(u32Item);	//system divider = 0==> Divider = (0+1), HCLK=Source clock/(divider+1)/2
}	    				

void Delay(UINT32 u32DelaySec)
{
	volatile unsigned int btime, etime, tmp, tsec;
	btime = sysGetTicks(TIMER0);
	tsec = 0;
	tmp = btime;
	while (1)
	{			
		etime = sysGetTicks(TIMER0);
		if ((etime - btime) >= (100*u32DelaySec))
		{	
			break;		
		}
	}
}
void DemoAPI_CLKRandom(void)
{
	UINT16 u16Item, u16Item1;
	UINT32 u32ExtFreq;
	
	sysSetTimerReferenceClock(TIMER0, 27000000); //External Crystal	
	sysStartTimer(TIMER0, 100, PERIODIC_MODE);			/* 100 ticks/per sec ==> 1tick/10ms */	
	sysSetLocalInterrupt(ENABLE_IRQ);	
	 u32ExtFreq = sysGetExternalClock();
	DBG_PRINTF("Random clock switch test\n");
	DBG_PRINTF("****Note: Test the function by SPI, SD, NAND or USB booting****\n");
	DBG_PRINTF("****System with 12M external clock may crash if run it by ICE****\n");
	//for(i=0;i<300;i=i+1)
#if 1	
	u16Item = 0x3B4D; //rand();
#endif	
	while(1)
	{
#if 1		
		DBG_PRINTF("Pre  item = %d\n", u16Item); 
		u16Item = ((u16Item)*(u16Item+1))>>8;					
		DBG_PRINTF("Post  item = %d\n", u16Item); 	
		u16Item1 = ((u16Item&0x7));
		DBG_PRINTF("Random test item = %d\n", u16Item1);	
		switch(u16Item1)
#else
		u16Item = rand() & 0x7;
		DBG_PRINTF(" item = %d\n", u16Item); 	
		switch(u16Item)
#endif		
		{
			case 0:	
					sysSetSystemClock(eSYS_UPLL, 	//E_SYS_SRC_CLK eSrcClk,	
									72000,		//UINT32 u32PllKHz, 	
									72000,		//UINT32 u32SysKHz,
									72000,		//UINT32 u32CpuKHz,
									  36000,		//UINT32 u32HclkKHz,
									  18000);		//UINT32 u32ApbKHz																			 		
					break;	
			case 1:	
					sysSetSystemClock(eSYS_UPLL, 	//E_SYS_SRC_CLK eSrcClk,	
									144000,		//UINT32 u32PllKHz, 	
									144000,		//UINT32 u32SysKHz,
									144000,		//UINT32 u32CpuKHz,
									  72000,		//UINT32 u32HclkKHz,
									  36000);		//UINT32 u32ApbKHz																			 		
					break;	
			case 2:						
					sysSetSystemClock(eSYS_UPLL, 	//E_SYS_SRC_CLK eSrcClk,	
									144000,		//UINT32 u32PllKHz, 	
									144000,		//UINT32 u32SysKHz,
									  72000,		//UINT32 u32CpuKHz,
									  36000,		//UINT32 u32HclkKHz,
									  18000);		//UINT32 u32ApbKHz										
					break;						
			case 3:	
					sysSetSystemClock(eSYS_UPLL, 	//E_SYS_SRC_CLK eSrcClk,	
									144000,		//UINT32 u32PllKHz, 	
									  72000,		//UINT32 u32SysKHz,
									  72000,		//UINT32 u32CpuKHz,
									  36000,		//UINT32 u32HclkKHz,
									  18000);		//UINT32 u32ApbKHz																													
					break;	
					
			case 4:	
					sysSetSystemClock(eSYS_UPLL, 	//E_SYS_SRC_CLK eSrcClk,	
									192000,		//UINT32 u32PllKHz, 	
									192000,		//UINT32 u32SysKHz,
									192000,		//UINT32 u32CpuKHz,
									  96000,		//UINT32 u32HclkKHz,
									  48000);		//UINT32 u32ApbKHz																		 		
					break;	
			case 5:						
					sysSetSystemClock(eSYS_UPLL, 	//E_SYS_SRC_CLK eSrcClk,	
									192000,		//UINT32 u32PllKHz, 	
									192000,		//UINT32 u32SysKHz,
									  96000,		//UINT32 u32CpuKHz,
									  96000,		//UINT32 u32HclkKHz,
									  48000);		//UINT32 u32ApbKHz										
					break;						
			case 6:	
					sysSetSystemClock(eSYS_UPLL, 	//E_SYS_SRC_CLK eSrcClk,	
									192000,		//UINT32 u32PllKHz, 	
									  96000,		//UINT32 u32SysKHz,
									  96000,		//UINT32 u32CpuKHz,
									  48000,		//UINT32 u32HclkKHz,
									  24000);		//UINT32 u32ApbKHz																													
					break;	
			case 7:		
					sysSetSystemClock(eSYS_EXT, 	//E_SYS_SRC_CLK eSrcClk,	
									u32ExtFreq,	//UINT32 u32PllKHz/EXT, 	
								 	u32ExtFreq,	//UINT32 u32SysKHz,
								  	u32ExtFreq/2,	//UINT32 u32CpuKHz,
								  	u32ExtFreq/2,	//UINT32 u32HclkKHz,
								  	u32ExtFreq/4);	//UINT32 u32ApbKHz				
					break;
			default:
					while(1);
														
		}	
		DBG_PRINTF("\n");
		DBG_PRINTF("\n");
		DBG_PRINTF("\n");
		Delay(1);
	}				
}
