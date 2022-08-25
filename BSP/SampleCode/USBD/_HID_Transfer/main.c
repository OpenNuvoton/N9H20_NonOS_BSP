#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wblib.h"
#include "N9H20.h"
#include "N9H20_USBD.h"
#include "HID.h"

int main(void)
{
    WB_UART_T uart;
    UINT32 u32ExtFreq;
    sysUartPort(1);
    u32ExtFreq = sysGetExternalClock();    /* Hz unit */
    uart.uiFreq = u32ExtFreq*1000;
    uart.uiBaudrate = 115200;
    uart.uiDataBits = WB_DATA_BITS_8;    
    uart.uiStopBits = WB_STOP_BITS_1;
    uart.uiParity = WB_PARITY_NONE;
    uart.uiRxTriggerLevel = LEVEL_1_BYTE;
    sysInitializeUART(&uart);	
    sysEnableCache(CACHE_WRITE_BACK);

    sysprintf("Sample code Start\n");	

    /* Enable USB */
    udcOpen();  

    hidInit();
    udcInit();

    while(1);
}

