/*-----------------------------------------------------------------------------------*/
/* Nuvoton Electronics Corporation confidential                                      */
/*                                                                                   */
/* Copyright (c) 2009 by Nuvoton Electronics Corporation                             */
/* All rights reserved                                                               */
/*                                                                                   */
/*-----------------------------------------------------------------------------------*/
/* File Contents:                                                                    */
/*   SpiToUart.c                 		                                             */
/*                                                                                   */
/* This file contains:                                                               */
/*                                                                                   */
/* Project:                                                                          */
/*                                                                                   */
/* Remark:                                                                           */
/*                                                                                   */
/*-----------------------------------------------------------------------------------*/

#include <stdio.h>
#include "N9H20.h"
#include "N9H20_SPI.h"
#include "N9H20_SPI_SPIToUART.h"


unsigned  char *pSrc,*pDst,*pDst2;
#define TEST_SIZE	512 * 2 * 64
#if defined(__GNUC__)
__attribute__((aligned(4096))) UINT8 WriteBuffer[TEST_SIZE];
__attribute__((aligned(4096))) UINT8 ReadBuffer[TEST_SIZE];
#else
__align(4096) UINT8 WriteBuffer[TEST_SIZE];
__align(4096) UINT8 ReadBuffer[TEST_SIZE];
#endif

void UARTTestItem()
{
	sysprintf("\n\n");
	sysprintf("+--------------------------------------------------+\n");
  sysprintf("| N9H20-MINI58  SPI-UART Sample Code              |\n");
  sysprintf("+--------------------------------------------------+\n");
	sysprintf("| [1] UART0_LOOPBACK                               |\n");
	sysprintf("| [2] UART1_LOOPBACK                               |\n");
	sysprintf("| [3] UART1&UART0_LOOPBACK                         |\n");
	sysprintf("| [4] Close all UART                               |\n");
	sysprintf("| [5] Reset all UART                               |\n");
	sysprintf("| [6] Clear TXRX Buffer                            |\n");
	sysprintf("+--------------------------------------------------+\n");
}


void UART_LOOPBACK(int SPI_UART)
{

	int tmp2,tmp3,tmp4;

	if(SPI_UART!=UART_ALL)
	{
		while(1)
		{
			while(vu_GetStatus()==1);
			while(VUART_RX_INT(SPI_UART))
			{
				tmp4=vu_GetRXAvailLen(SPI_UART);
				tmp2=vu_UARTRead(SPI_UART, tmp4,pDst);
				vu_UARTWrite(SPI_UART,pDst,tmp2);
			}
		}
	}
	else
	{
		while(1)
		{
			while(vu_GetStatus()==1); 							//wait mini58 response interrupt
			while(VUART_RX_INT(SPI_UART))
			{ 					//wait rx read interrupt
				tmp4=vu_GetRXAvailLen(SPI_UART);  		//get count of UART0 & UART1 rx data
				if((tmp4&0xf)>0)
				{
						tmp2=vu_UARTRead(0,tmp4&0xf,pDst);	//read uart0 rx and get count of UART0 rx data from uart0 rx buffer 
						vu_UARTWrite(0,pDst,tmp2);
				}
				
				if(((tmp4&0xf0)>>4)>0)
				{
					tmp3=vu_UARTRead(1,((tmp4&0xf0)>>4),pDst2); 	//read uart1 rx and get count of UART1 rx data from uart1 rx buffer 
					vu_UARTWrite(1,pDst2,tmp3);						//write # bytes to uart1 rx
				}
			}
		}
	}
}
int main()
{
	WB_UART_T uart;
	UINT32 u32ExtFreq;
	unsigned char  *pDst;
	int volatile i;
	char item;
	int  SPI_UART=1;
	sysSetSystemClock(eSYS_UPLL, 	//E_SYS_SRC_CLK eSrcClk,	
						192000,		//UINT32 u32PllKHz, 	
						192000,		//UINT32 u32SysKHz,
						192000,		//UINT32 u32CpuKHz,
						  96000,		//UINT32 u32HclkKHz,
						  48000);		//UINT32 u32ApbKHz	

	u32ExtFreq = sysGetExternalClock();
	sysUartPort(1);
	uart.uiFreq = u32ExtFreq*1000;
	uart.uart_no = 1;
  uart.uiBaudrate = 115200;
  uart.uiDataBits = WB_DATA_BITS_8;
  uart.uiStopBits = WB_STOP_BITS_1;
  uart.uiParity = WB_PARITY_NONE;
  uart.uiRxTriggerLevel = LEVEL_1_BYTE;
  sysInitializeUART(&uart);

	pSrc = (UINT8 *)((UINT32)WriteBuffer | 0x80000000);
	pDst = (UINT8 *)((UINT32)ReadBuffer | 0x80000000);


	sysEnableCache(CACHE_WRITE_BACK);
	
	sysprintf("\nSpi to UART Test...\n");

	UARTTestItem();
	
	do
	{
		item = sysGetChar();
		sysprintf("%c\n",item);   
		switch(item)
		{
			case '1':
				vu_OpenUART(UART_PORT0);
				vu_ResetUART(UART_PORT0);
				vu_SetBaudRate(0,115200);
				UART_LOOPBACK(UART_PORT0); break;		
			case '2':
				vu_OpenUART(UART_PORT1);
				vu_ResetUART(UART_PORT1);
				vu_SetBaudRate(1,115200);
				UART_LOOPBACK(UART_PORT1); break;			
			case '3':
				vu_OpenUART(UART_ALL);
				vu_ResetUART(UART_ALL);
				vu_SetBaudRate(0,115200);
				vu_SetBaudRate(1,57600);
				UART_LOOPBACK(UART_ALL); break;
			case '4':
				vu_OpenUART(UART_ALL);
				vu_CloseUART(UART_ALL); break;
			case '5':
				vu_OpenUART(UART_ALL);
				vu_SetBaudRate(0,57600);
				vu_SetBaudRate(1,57600);
				sysprintf("Press any key to UART0 or UART1\n");
				while(VUART_RX_INT(UART_ALL)!=1);
				sysprintf("Berfor:(UART0,UART1)= (%d,%d) bytes data in RX buffer \n",(vu_GetRXAvailLen(UART_ALL)&0XF),(vu_GetRXAvailLen(UART_ALL)&0XF0)>>4);
				vu_ResetUART(UART_ALL); 
				sysprintf("After: (UART0,UART1)= (%d,%d) bytes data in RX buffer \n",(vu_GetRXAvailLen(UART_ALL)&0XF),(vu_GetRXAvailLen(UART_ALL)&0XF0)>>4);	
				vu_CloseUART(UART_ALL); break;
			case '6':
				vu_OpenUART(UART_PORT0);
				vu_SetBaudRate(0,115200);
				sysprintf("Press any key to UART0 \n");
				while(VUART_RX_INT(UART_PORT0)!=1);
				sysprintf("Berfor:UART0= %d bytes data in RX buffer \n",(vu_GetRXAvailLen(UART_ALL)&0XF));
				vu_ClearBuf(UART_PORT0);
				sysprintf("After:UART0= %d bytes data in RX buffer \n",(vu_GetRXAvailLen(UART_ALL)&0XF));	
				sysprintf("Press any key to UART0. \n");		
				while(VUART_RX_INT(UART_PORT0)!=1);		
				vu_UARTRead(SPI_UART, 1,pDst);
				sysprintf("READ(UART%d)=%c\n",UART_PORT0,*pDst); break;

			default:
				break;
				
		}
	}while(item!=0x1B);


	while(1);


	return 0;
}

