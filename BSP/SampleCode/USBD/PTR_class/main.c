/**************************************************************************//**
 * @file     main.c
 * @brief    Demo how to use USB Device & HID driver to implement HID Device
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wblib.h"
#include "N9H20.h"
#include "PTR.h"

int main(void)
{
    WB_UART_T uart;
    UINT32 u32ExtFreq;
    sysUartPort(1);
    u32ExtFreq = sysGetExternalClock();
    uart.uiFreq = u32ExtFreq*1000;	
    uart.uiBaudrate = 115200;
    uart.uiDataBits = WB_DATA_BITS_8;
    uart.uiStopBits = WB_STOP_BITS_1;
    uart.uiParity = WB_PARITY_NONE;
    uart.uiRxTriggerLevel = LEVEL_1_BYTE;
    sysInitializeUART(&uart);

    sysSetSystemClock(eSYS_UPLL,    //E_SYS_SRC_CLK eSrcClk,
                    192000,         //UINT32 u32PllKHz,
                    192000,         //UINT32 u32SysKHz,
                    192000,         //UINT32 u32CpuKHz,
                    192000/2,       //UINT32 u32HclkKHz,
                    192000/4);      //UINT32 u32ApbKHz	
		
    sysEnableCache(CACHE_WRITE_BACK);
	
    sysprintf("Sample code Start\n");	   

    /* Enable USB */
    udcOpen();  
    
    ptrInit();
		
    udcInit();
    
    while(1)
    {
        print_message();
    };
}

