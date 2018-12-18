/***************************************************************************
 *                                                                         									     *
 * Copyright (c) 2008 Nuvoton Technolog. All rights reserved.              					     *
 *                                                                         									     *
 ***************************************************************************/

#include <stdio.h>
#include <string.h>
#include "N9H20.h"

UINT32 g_u32Idx=0;
volatile BOOL bIsTimeOut=0;
char pi8UartBuf[10010];

UINT32 g_u32Len; 
UINT32 g_u32Valid = 0;
UINT32 g_u32Timeout = 0;

UINT32 u32LenR[30]={0};
UINT32 u32LenPtr = 0;
/*
	Receive file with ASCII code from PC. The program excape if receive the 'q' letter
	Remember taht if using high speed UART, it is UART port 0 shared with ICE mode. 
	
*/
void UartDataValid_Handler(UINT8* buf, UINT32 u32Len)
{
	UINT32 u32Idx = 0;
	g_u32Len = u32Len;
	g_u32Valid = g_u32Valid+1;
	
	//u32LenR[u32LenPtr] = u32Len;
	//u32LenPtr = u32LenPtr+1;
	
	memcpy((void*)&(pi8UartBuf[g_u32Idx]), (void*)buf, u32Len);
	g_u32Idx = g_u32Idx+u32Len;	

	
	while(u32Idx++<u32Len)
	{
		if(*buf++ =='q')
		{
			bIsTimeOut = 1;
			//u8RevBuf[(g_u32Idx-1)] = 0;	/* Cover the "quit " pattern */
			break;
		}		
	}		
}
void UartDataTimeOut_Handler(UINT8* buf, UINT32 u32Len)
{
	UINT32 u32Idx = 0;
	g_u32Timeout = g_u32Timeout+1;
	
	//u32LenR[u32LenPtr] = u32Len;
	//u32LenPtr = u32LenPtr+1;
	
	memcpy(&(pi8UartBuf[g_u32Idx]), buf, u32Len);
	g_u32Idx = g_u32Idx+u32Len;	
	
	while(u32Idx++<u32Len)
	{
		if(*buf++ =='q')
		{
			bIsTimeOut = 1;
			//u8RevBuf[(g_u32Idx-1)] = 0;	/* Cover the "quit " pattern */
			break;
		}	
	}		
}
UARTDEV_T UART0;	/*High speed */
UARTDEV_T* pUART0;
int DemoAPI_HUART(void)
{
	WB_UART_T uart;
	char* pi8String;
	UINT8 u8Item;
	UINT32 u32Len;
	UINT32 u32ExtFreq;
	UINT32 u32Count;
	INT8 i8UartData=0; 
	memset(pi8UartBuf, 0, 1024);
	u32ExtFreq = sysGetExternalClock();
#if 0	/* Old style */	
	/* Init UART 0 (High speed) */
	sysUartPort(0);
	//sysUartPort(1);
	
	uart.uiFreq = u32ExtFreq*1000;	
	uart.uiBaudrate =  921600;
	
	uart.uiDataBits = WB_DATA_BITS_8;
	uart.uiStopBits = WB_STOP_BITS_1;
	uart.uiParity = WB_PARITY_NONE;
	uart.uiRxTriggerLevel = LEVEL_1_BYTE;		
	sysInitializeUART(&uart);		
	sysUartEnableInt(UART_INT_NONE);
   	sysUartInstallcallback(0, UartDataValid_Handler);
	sysUartInstallcallback(1, UartDataTimeOut_Handler);
	sysprintf("\n\nDemo Start, please input some characters\n");	
	sysprintf("Receive data one by one by manual input on hyper-terminal until [q] ASCII code received\n");
	do
	{
        	i8UartData = sysGetChar();
        	sysprintf("%c", i8UartData);
        }while (i8UartData!='q');
         sysprintf("\nSingle character input done \n");	
        
        sysprintf("\n\nReceive ASCII file from PC until [q] ASCII code received\n");
        sysprintf("To start up the operation, please choice  'Send ASCII' on hyper-terminal\n");
        uart.uiRxTriggerLevel = LEVEL_1_BYTE;//LEVEL_62_BYTES;		/* high speed max trigger level */
        sysInitializeUART(&uart);	
  
	while(bIsTimeOut==0);
#else /* Both UARTs port works together */		
   	register_uart_device(0, &UART0);
   	pUART0 = &UART0;
   	pUART0->UartPort(0);
	uart.uiFreq = u32ExtFreq*1000;
	uart.uiBaudrate = 115200;
	uart.uiDataBits = WB_DATA_BITS_8;
	uart.uiStopBits = WB_STOP_BITS_1;
	uart.uiParity = WB_PARITY_NONE;
	uart.uiRxTriggerLevel = LEVEL_1_BYTE;
   	pUART0->UartInitialize(&uart);
   	pUART0->UartEnableInt(UART_INT_NONE);		
   	pUART0->UartInstallcallback(0, UartDataValid_Handler);	
   	pUART0->UartInstallcallback(1, UartDataTimeOut_Handler);	
   
   	sysprintf("\nHigh Speed UART port  initialize done, please pressing any key in hyper terminal. It will be stop after pressing 'q'\n");   	
   	do
	{
        i8UartData = pUART0->UartGetChar();
        sysprintf0("%c", i8UartData);
    }while (i8UartData!='q');
#endif 	
	/* Hyper-terminal will converse 0x0D,0x0A to 0x0D only. Following code convese the carriage return code 0x0D to 0x0 */ 
#if 0	
	sysUartEnableInt(UART_INT_RDA);
    sysUartEnableInt(UART_INT_RDTO);        
    sysSetLocalInterrupt(ENABLE_IRQ);          
#else
	pUART0->UartEnableInt(UART_INT_RDA);	
   	pUART0->UartEnableInt(UART_INT_RDTO);	
   	sysSetLocalInterrupt(ENABLE_IRQ); 	
#endif	
	pi8String = pi8UartBuf;
	u32Len = strlen(pi8String);
	while(*pi8String!=0)
	{
		if(*pi8String==0x0d)
			*pi8String = 0;
		pi8String ++;
	}
	pi8String = pi8UartBuf;
	do
	{
		u32Len = strlen(pi8String);
		sysprintf("%s\n", pi8String);
		pi8String = (char*)((UINT32)pi8String+(u32Len+1));
	}while(u32Len!=0);	
	
	/* Transfer data to PC */
	uart.uiDataBits = WB_DATA_BITS_8;
	uart.uiStopBits = WB_STOP_BITS_1;
	uart.uiParity = WB_PARITY_NONE;
	uart.uiRxTriggerLevel = LEVEL_1_BYTE;		
	sysInitializeUART(&uart);	
	sysUartEnableInt(UART_INT_NONE);
#if 0	
	sysprintf("\n\nTranfer data to PC by starting up the 'Receive ASCII' on the hyper-terminal. After this operation complete, pressing any key to start receive a file \n");	
	sysGetChar();
#else
	sysprintf0("\n\nTranfer data to PC by starting up the 'Receive ASCII' on the hyper-terminal. After this operation complete, pressing any key to start receive a file \n");	
	pUART0->UartGetChar();
#endif	
	
	u32Count = 0;
	while(1)
	{
		for(u8Item='0'; u8Item<='Z'; u8Item=u8Item+1)
		{
			pi8UartBuf[u32Count] = u8Item;
			u32Count = u32Count+1;
			if(u32Count>10000)
				break;
		}	
		if(u32Count>10000)
				break;
		for(u8Item='Z'; u8Item>='0'; u8Item=u8Item-1)
		{
			pi8UartBuf[u32Count] = u8Item;
			u32Count = u32Count+1;
			if(u32Count>10000)
				break;
		}	
		if(u32Count>10000)
				break;
	}	
#if 0	
	sysUartTransfer(pi8UartBuf,  u32Count);
#else
	pUART0->UartTransfer(pi8UartBuf,  u32Count);
#endif	
	
    	return 0;
} /* end main */
