/****************************************************************************
 * @file     main.c
 * @version  V1.00
 * $Revision: 4 $
 * $Date: 18/04/25 11:43a $
 * @brief    Virtual COM sample file
 *
 * @note
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "N9H20.h"

//#define SUSPEND_POWERDOWN
void Demo_PowerDownWakeUp(void);
void vcomInit(void);
void VCOM_MainProcess(void);

int main(void)
{
    WB_UART_T uart;
    UINT32 u32ExtFreq;
    sysUartPort(1);
    u32ExtFreq = sysGetExternalClock();    /* Hz unit */
    uart.uiFreq = u32ExtFreq * 1000;
    uart.uiBaudrate = 115200;
    uart.uiDataBits = WB_DATA_BITS_8;
    uart.uiStopBits = WB_STOP_BITS_1;
    uart.uiParity = WB_PARITY_NONE;
    uart.uiRxTriggerLevel = LEVEL_1_BYTE;
    sysInitializeUART(&uart);

    sysprintf("\nVirtual COM Demo\n");
    /* Enable USB */
    udcOpen(); 
    vcomInit();
#ifdef SUSPEND_POWERDOWN
    udcSetSupendCallBack(Demo_PowerDownWakeUp);
#endif
    udcInit();
    VCOM_MainProcess();
}

