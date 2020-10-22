/**************************************************************************//**
 * @file     demo_DualUart.c
 * @brief    Sample code to demostrate both UART ports working together
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 * brief: Launch two hyper-terminal application on PC side to handle 
 *        the message from UART 0 and UART 1.
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include "N9H20.h"


UARTDEV_T DUART0;   /*High speed */
UARTDEV_T* pDUART0;

UARTDEV_T DUART1;   /*Normal speed */
UARTDEV_T* pDUART1;

#if defined(__GNUC__)
char pi8DUartBuf0[256] __attribute__((aligned (32)));
char pi8DUartBuf1[256] __attribute__((aligned (32)));

char pi8Uart0RxBuf[256] __attribute__((aligned (32)));
char pi8Uart1RxBuf[256] __attribute__((aligned (32)));
#else
__align(32) char pi8DUartBuf0[256];
__align(32) char pi8DUartBuf1[256];

__align(32) char pi8Uart0RxBuf[256];
__align(32) char pi8Uart1RxBuf[256];
#endif


volatile UINT32 g_u32Idx0=0;
volatile UINT32 g_u32Valid0 = 0;
volatile UINT32 g_u32Timeout0 = 0;

volatile UINT32 g_u32Idx1=0;
volatile UINT32 g_u32Valid1 = 0;
volatile UINT32 g_u32Timeout1 = 0;


volatile BOOL bIsUART0BufEmpty = 0;
void Uart0TranEmpty_Handler(UINT8* buf, UINT32 u32Len)  /* The parameters of Callback function are useless */
{
    bIsUART0BufEmpty = 1;
}

volatile BOOL bIsUART1BufEmpty = 0;
void Uart1TranEmpty_Handler(UINT8* buf, UINT32 u32Len)  /* The parameters of Callback function are useless */
{
    bIsUART1BufEmpty = 1;
}

void Uart0Read_Handler(UINT8* buf, UINT32 u32Len)
{
    g_u32Valid0 = g_u32Valid0+1;

    memcpy((void*)&(pi8Uart0RxBuf[g_u32Idx0]), (void*)buf, u32Len);
    g_u32Idx0 = g_u32Idx0+u32Len;
    pi8Uart0RxBuf[ g_u32Idx0 ] = 0;
}

void Uart1Read_Handler(UINT8* buf, UINT32 u32Len)
{
    g_u32Valid1 = g_u32Valid1+1;

    memcpy((void*)&(pi8Uart1RxBuf[g_u32Idx1]), (void*)buf, u32Len);
    g_u32Idx1 = g_u32Idx1+u32Len;
    pi8Uart1RxBuf[ g_u32Idx1 ] = 0;
}

/* The function is not be demo on the example code */
void Uart0TimeOut_Handler(UINT8* buf, UINT32 u32Len)
{

    g_u32Timeout0 = g_u32Timeout0+1;

    memcpy(&(pi8Uart0RxBuf[g_u32Idx0]), buf, u32Len);
    g_u32Idx0 = g_u32Idx0+u32Len;
    pi8Uart0RxBuf[ g_u32Idx0 ] = 0;
}

/* The function is not be demo on the example code */
void Uart1TimeOut_Handler(UINT8* buf, UINT32 u32Len)
{

    g_u32Timeout1 = g_u32Timeout1+1;

    memcpy(&(pi8Uart1RxBuf[g_u32Idx1]), buf, u32Len);
    g_u32Idx1 = g_u32Idx1+u32Len;
    pi8Uart1RxBuf[ g_u32Idx1 ] = 0;
}


void DemoAPI_DualUART(void)
{
    WB_UART_T uart;
    UINT32 u32ExtFreq;
    UINT32 i,j;


    u32ExtFreq = sysGetExternalClock();
    sysSetLocalInterrupt(ENABLE_IRQ);

    register_uart_device(0, &DUART0);
    pDUART0 = &DUART0;
    pDUART0->UartPort(0);
    uart.uiFreq = u32ExtFreq*1000;
    uart.uiBaudrate = 115200;
    uart.uiDataBits = WB_DATA_BITS_8;
    uart.uiStopBits = WB_STOP_BITS_1;
    uart.uiParity = WB_PARITY_NONE;
    uart.uiRxTriggerLevel = LEVEL_14_BYTES;
    pDUART0->UartInitialize(&uart);
    //pDUART0->UartEnableInt(UART_INT_RDA);
    //pDUART0->UartEnableInt(UART_INT_RDTO);
    pDUART0->UartEnableInt(UART_INT_NONE);
    /* Transfer empty int will be enable/disable automatically */

    pDUART0->UartInstallcallback(2, Uart0TranEmpty_Handler);
    pDUART0->UartInstallcallback(0, Uart0Read_Handler);
    pDUART0->UartInstallcallback(1, Uart0TimeOut_Handler);


    register_uart_device(1, &DUART1);
    pDUART1 = &DUART1;
    pDUART1->UartPort(1);
    uart.uiFreq = u32ExtFreq*1000;
    uart.uiBaudrate = 115200;
    uart.uiDataBits = WB_DATA_BITS_8;
    uart.uiStopBits = WB_STOP_BITS_1;
    uart.uiParity = WB_PARITY_NONE;
    uart.uiRxTriggerLevel = LEVEL_4_BYTES;
    pDUART1->UartInitialize(&uart);
    //pDUART1->UartEnableInt(UART_INT_RDA);
    //pDUART1->UartEnableInt(UART_INT_RDTO);
    pDUART1->UartEnableInt(UART_INT_NONE);
    /* Transfer empty int will be enable/disable automatically */

    pDUART1->UartInstallcallback(2, Uart1TranEmpty_Handler);
    pDUART1->UartInstallcallback(0, Uart1Read_Handler);
    pDUART1->UartInstallcallback(1, Uart1TimeOut_Handler);

    /* Prepare buffer content to send */
    for(i=0; i<24; i=i+1)
    {
        pi8DUartBuf0[i] = 'a'+i;
        pi8DUartBuf1[i] = 'A'+i;
    }
    pi8DUartBuf0[24] = 0x0A;
    pi8DUartBuf0[25] = 0x0D;

    pi8DUartBuf1[24] = 0x0A;
    pi8DUartBuf1[25] = 0x0D;

    for(i=0; i<3; i=i+1)
    {
        for(j=0; j<10; j=j+1)
        {

            bIsUART0BufEmpty = 0;
            bIsUART1BufEmpty = 0;
            pDUART0->UartTransferInt(pi8DUartBuf0,  26);
            pDUART1->UartTransferInt((char*)(pi8DUartBuf1+1),  25);

            while(bIsUART0BufEmpty == 0);
            while(bIsUART1BufEmpty == 0);

        }
        sysprintf0("\n");
        sysprintf("\n");
    }
}
