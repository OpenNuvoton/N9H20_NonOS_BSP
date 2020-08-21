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

int main(void)
{
	WB_UART_T uart;
	UINT32 u32ExtFreq;	
	int i, cnt=0;

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
	
	gpio_setportval(GPIO_PORTB, 0xf, 0);
	gpio_setportpull(GPIO_PORTB, 0xf, 0);
	gpio_setportdir(GPIO_PORTB, 0xf, 0xf);
	
	sysprintf("start gpio test... \n");
	while(cnt++<1000000) {
		for(i = 0; i < 0xf; i++) {
			gpio_setportval(GPIO_PORTB, 0xf, i);
			sysDelay(100);	
		}	
	}
	
	sysprintf("quit gpio test\n");
	return(0);
}
