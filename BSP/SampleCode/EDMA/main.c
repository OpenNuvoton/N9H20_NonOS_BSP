/**************************************************************************//**
 * @file     main.c
 * @brief    N9H20 series EDMA demo code
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "N9H20.h"

#if defined(__GNUC__)
__attribute__((aligned(32))) UINT8 LoadAddr[]=
{
	#include "../../VPOST/ASIC/river_480x272_RGB565.dat"
};
#else
__align(32) UINT8 LoadAddr[]=
{
	#include "../../VPOST/ASIC/river_480x272_RGB565.dat"
};
#endif

LCDFORMATEX lcdFormat;
extern void TransferLengthTest(void);
extern void ColorSpaceTransformTest(void);
extern void SPIFlashTest(void);
extern void UARTTest(void);

int main()
{
	WB_UART_T uart;
	UINT32 u32ExtFreq, u32Item;

	sysSetSystemClock(eSYS_UPLL, 	//E_SYS_SRC_CLK eSrcClk,	
						192000,		//UINT32 u32PllKHz, 	
						192000,		//UINT32 u32SysKHz,
						192000,		//UINT32 u32CpuKHz,
						  96000,		//UINT32 u32HclkKHz,
						  48000);		//UINT32 u32ApbKHz
						  
	u32ExtFreq = sysGetExternalClock();
	sysUartPort(1);
	uart.uiFreq = u32ExtFreq * 1000;
	uart.uiBaudrate = 115200;
	uart.uiDataBits = WB_DATA_BITS_8;
	uart.uiStopBits = WB_STOP_BITS_1;
	uart.uiParity = WB_PARITY_NONE;
	uart.uiRxTriggerLevel = LEVEL_1_BYTE;
	sysInitializeUART(&uart);

	sysDisableCache(); 	
	sysFlushCache(I_D_CACHE);	
	sysEnableCache(CACHE_WRITE_BACK);

	sysSetLocalInterrupt(ENABLE_IRQ);

	lcdFormat.ucVASrcFormat = DRVVPOST_FRAME_RGB565;
	vpostLCMInit(&lcdFormat, (UINT32*)LoadAddr);

	/* If backlight control signal is different from nuvoton's demo board,
	   please don't call this function and must implement another similar one to enable LCD backlight. */	
	vpostEnaBacklight();

	EDMA_Init();

	do
	{
		sysprintf("==================================================================\n");
		sysprintf("[1] Transfer Length and Direction Test \n");
		sysprintf("[2] Color Space Transform Test \n");			
		sysprintf("[3] PDMA+SPIFlash Test \n");	
		sysprintf("[4] PDMA+UART Test \n");	
		sysprintf("[5] PDMA+ADC Test \n");	
		sysprintf("==================================================================\n");

		u32Item = sysGetChar();
		
		switch(u32Item) 
		{
			case '1': 
				TransferLengthTest();			
				break;
					
			case '2':
				ColorSpaceTransformTest();					
				break;		
				
			case '3':
				SPIFlashTest();					
				break;

			case '4':
				UARTTest();					
				break;
				
			case '5':
				sysprintf("Please refer to adc sample code \n");											
				break;
				
			case 'Q':
			case 'q': 
				u32Item = 'Q';
				sysprintf("quit edma test...\n");				
				break;	
				
			}
		
	}while(u32Item!='Q');
	
}








