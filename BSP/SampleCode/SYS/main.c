/***************************************************************************
 *                                                                         									     *
 * Copyright (c) 2008 Nuvoton Technolog. All rights reserved.              					     *
 *                                                                         									     *
 ***************************************************************************/

#include <stdio.h>
#include "N9H20.h"
#include "demo.h"
//#pragma import (__use_no_semihosting_swi)

extern int DemoAPI_AIC(void);
extern void DemoAPI_UART(void);
extern void DemoAPI_Timer0(void);
extern void DemoAPI_Timer1(void);
extern void DemoAPI_WDT(void);
extern void DemoAPI_Cache(BOOL bIsCacheOn);
extern void DemoAPI_CLK(void);
extern void DemoAPI_CLKRandom(void);
extern void DemoAPI_CLKRandom(void);
extern void Demo_PowerDownWakeUp(void);
extern int DemoAPI_HUART(void);
extern void Demo_PowerDownWakeUp(void);
extern void DemoAPI_SetSystemDivider(void);

void DemoAPI_UPLL_APLL(void)
{
	outp32(0xb0000220, 0x1000CE6E);
	outp32(0xb0000220, 0x1008CE6E);
	outp32(0xb0000084, 0x3);	
	outp32(0xb0000230, 0x81);	
}

BOOL bIsFirstApll=TRUE; 
UINT32 u32SysDiv0, u32SysDiv1;
void DemoAPI_AdjustApllDivider(void)
{
	UINT32  i,j; 
	if(bIsFirstApll==TRUE)
	{
		UINT32 u32Reg;
		u32Reg = inp32(REG_CLKDIV0) & 0xF07;		
		u32SysDiv0 = u32Reg & 0x7;	 
		u32SysDiv1 = u32Reg >>8;
		bIsFirstApll = FALSE;
	}
	for(i=u32SysDiv1; i<16;i=i+1)
	{
		for(j=u32SysDiv0; j<8;j=j+1)
		{
			outp32(REG_CLKDIV0, (inp32(REG_CLKDIV0) & ~0xF07) | 
								((i<<8) | j));
			DBG_PRINTF("SYS divider1 %d,  divider0 %d\n", i, j);												
		}
	}	
}

int main()
{
	//unsigned int volatile i;
	WB_UART_T uart;
	UINT32 u32Item, u32ExtFreq;
	UINT32 u32PllOutKHz;
	u32ExtFreq = sysGetExternalClock();
#if 1				 
	/* Please run the path if you want to use normal speed UART */
	sysUartPort(1);
	uart.uiFreq = u32ExtFreq*1000;
	uart.uiBaudrate = 115200;
#else
	/* Please run the path if you want to use high speed UART */
	/* Remember that high speed UART shared pins with ICE */
	/* You need to burn code to NAND or SD */
	sysUartPort(0);	
	uart.uiFreq = u32ExtFreq*1000;
	uart.uiBaudrate = 921600;
#endif	
	uart.uiDataBits = WB_DATA_BITS_8;
	uart.uiStopBits = WB_STOP_BITS_1;
	uart.uiParity = WB_PARITY_NONE;
	uart.uiRxTriggerLevel = LEVEL_1_BYTE;
	sysInitializeUART(&uart);
	sysSetSystemClock(eSYS_UPLL, 	//E_SYS_SRC_CLK eSrcClk,	
						192000,		  //UINT32 u32PllKHz, 	
						192000,		  //UINT32 u32SysKHz,
						192000,		  //UINT32 u32CpuKHz,
						192000/2,		//UINT32 u32HclkKHz,
						192000/4);  //UINT32 u32ApbKHz
					   	 
	u32ExtFreq = sysGetExternalClock();
	u32PllOutKHz = sysGetPLLOutputKhz(eSYS_UPLL, u32ExtFreq);
	DBG_PRINTF("PLL out frequency %d Khz\n", u32PllOutKHz);	
	
	sysUartEnableDebugMessage(TRUE);
	DBG_PRINTF("Debug message on\n");	
	sysUartEnableDebugMessage(FALSE);
	DBG_PRINTF("Debug message off\n");	
	sysUartEnableDebugMessage(TRUE);
	DBG_PRINTF("Debug message on\n");	
	do
	{    	
		DBG_PRINTF("================================================================\n");
		DBG_PRINTF("                System library demo code                        \n");
		DBG_PRINTF(" [1] UART demo                                                  \n");
		DBG_PRINTF(" [2] Timer0 demo                                                \n");
		DBG_PRINTF(" [3] Timer1 demo                                                \n");
		DBG_PRINTF(" [4] Watch dog                                                  \n");		
		DBG_PRINTF(" [5] Cache demo disable                                         \n");
		DBG_PRINTF(" [6] Cache demo enable                                          \n");
		DBG_PRINTF(" [7] AIC demo                                                   \n");				
		DBG_PRINTF(" [8] Clock switch                                               \n");
		DBG_PRINTF(" [9] Clock switch random                                        \n");
		DBG_PRINTF(" [a] Power down then wake up by GPIOA-0                         \n");
		DBG_PRINTF(" [b] High speed UART demo                                       \n");
		DBG_PRINTF(" [c] Set system divider                                         \n");
		DBG_PRINTF(" [d] Set system clock from external	(CPU/DRAM=0.75MHz/0.75MHz)  \n");
		DBG_PRINTF("================================================================\n");
		DBG_PRINTF("REG_CLKDIV0 = %x\n", inp32(REG_CLKDIV0));
		DBG_PRINTF("REG_APLLCON = %x\n", inp32(REG_APLLCON));
		DBG_PRINTF("REG_UPLLCON = %x\n", inp32(REG_UPLLCON));
		
		u32Item = sysGetChar();
		switch(u32Item)
		{
      case '1': DemoAPI_UART();		break; 	//OK-sysprintf
      case '2': DemoAPI_Timer0();		break;
      case '3': DemoAPI_Timer1();		break;	
      case '4':	DemoAPI_WDT();		break;
      case '5':	DemoAPI_Cache(FALSE);	break;	
      case '6':	DemoAPI_Cache(TRUE);	break;	
      case '7': DemoAPI_AIC(); 		break;
      case '8':	sysPowerDownPLL(eSYS_UPLL, FALSE);
								sysPowerDownPLL(eSYS_APLL, FALSE);
	    					DemoAPI_CLK();		break;	  	
      case '9':	sysPowerDownPLL(eSYS_UPLL, FALSE);
                sysPowerDownPLL(eSYS_APLL, FALSE);
	    					DemoAPI_CLKRandom();	
	    					break;		    		
      case 'a':	Demo_PowerDownWakeUp();	
	    					break;
      case 'b':	DemoAPI_HUART();	
	    					break;
	    		
      case 'c':	DemoAPI_SetSystemDivider();
	    					break;
      case 'd':	sysSetSystemClock(eSYS_EXT, 	     //E_SYS_SRC_CLK eSrcClk,	
                                   u32ExtFreq,	   //UINT32 u32PllKHz/EXT, 	
                                   u32ExtFreq,	   //UINT32 u32SysKHz,
                                   u32ExtFreq/2,   //UINT32 u32CpuKHz,
                                   u32ExtFreq/2,   //UINT32 u32HclkKHz,
                                   u32ExtFreq/4);	 //UINT32 u32ApbKHz			
			
							sysClockDivSwitchStart(7);	  	/* Max system divider */													
							sysPowerDownPLL(eSYS_UPLL, TRUE);
							sysPowerDownPLL(eSYS_APLL, TRUE); 
	    		case 'Q':	break;
	    		case 'q':	break;
		}
	}while((u32Item!= 'q') || (u32Item!= 'Q'));	
    	return 0;
} /* end main */
