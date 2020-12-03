/**************************************************************************//**
 * @file     main.c
 * @brief    Show how to set GPIO pin mode and output control.
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "N9H20.h"

void EXTINT0_IRQHandler(void)
{
	unsigned int read0;
	
	outpw(REG_IRQTGSRC0 , inpw(REG_IRQTGSRC0) & BIT18);	/* Clear GPB2 interrupt flag */
	sysprintf("EXTINT0 INT occurred.\n");

	gpio_readport(GPIO_PORTB, (unsigned short *)&read0);
	sysprintf("GPB2 pin status %d\n", (read0 & BIT2) >> 2);
}

void GPIOOutputTest(void)
{
	int i;
	
	gpio_setportval(GPIO_PORTB, (1 << 0), 0);
	gpio_setportdir(GPIO_PORTB, (1 << 0), (1 << 0));		/* Set GPB0 output mode */
	
	sysprintf("start gpio output test... \n");
	
	for(i = 0; i < 2; i++) {
		gpio_setportval(GPIO_PORTB, (1 << 0), (1 << 0));	/* Set GPB0 output high */
		sysDelay(50);	
		
		gpio_setportval(GPIO_PORTB, (1 << 0), 0);					/* Set GPB0 output low */
		sysDelay(50);		
	}	
	
	sysprintf("exit gpio output test\n");	
}

void GPIOInterruptTest(void)
{
	int i;
	
	sysprintf("Connect GPB2 and GPB0 to test interrupt ......\n");

	gpio_setportval(GPIO_PORTB, BIT0, BIT0);			/* Set GPB0 output high */
	gpio_setportdir(GPIO_PORTB, BIT0, BIT0);			/* Set GPB0 output mode */	
	
	sysInstallISR(IRQ_LEVEL_7, (INT_SOURCE_E)IRQ_EXTINT0, (PVOID)EXTINT0_IRQHandler);
	sysSetInterruptType((INT_SOURCE_E)IRQ_EXTINT0, HIGH_LEVEL_SENSITIVE);
	sysEnableInterrupt((INT_SOURCE_E)IRQ_EXTINT0);

	gpio_setportdir(GPIO_PORTB, BIT2, 0x0);				/* Set GPB2 input mode */
	gpio_setportpull(GPIO_PORTB, BIT2, BIT2);			/* Set GPB2 pull-high */
	gpio_setsrcgrp(GPIO_PORTB, BIT2, 0x0);				/* set to IRQ_EXTINT0 */
	gpio_setintmode(GPIO_PORTB, BIT2, BIT2, 0);		/* Set GPB2 falling edge trigger */

	sysSetLocalInterrupt(ENABLE_IRQ);

	for(i = 0; i < 2; i++) {
		gpio_setportval(GPIO_PORTB, BIT0, BIT0);		/* Set GPB0 output high */
		sysDelay(50);	
		
		gpio_setportval(GPIO_PORTB, BIT0, 0);				/* Set GPB0 output low */
		sysDelay(50);		
	}	

}

int main(void)
{
	WB_UART_T uart;
	UINT32 u32ExtFreq;	
	INT8 i8Item;

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

	do
	{
		sysprintf("\n");	
		sysprintf("==================================================================\n");
		sysprintf("[1] GPIO Output Test \n");
		sysprintf("[2] GPIO Interrupt Test \n");			
		sysprintf("==================================================================\n");

		i8Item = sysGetChar();
		
		switch(i8Item) 
		{
			case '1': 
				GPIOOutputTest();			
				break;
					
			case '2':
				GPIOInterruptTest();					
				break;		
								
			case 'Q':
			case 'q': 
				i8Item = 'Q';
				sysprintf("quit GPIO test...\n");				
				break;	
				
			}
		
	}while(i8Item!='Q');

	return(0);
}
