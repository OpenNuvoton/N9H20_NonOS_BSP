/*-----------------------------------------------------------------------------------*/
/* Nuvoton Electronics Corporation confidential                                      */
/*                                                                                   */
/* Copyright (c) 2009 by Nuvoton Electronics Corporation                             */
/* All rights reserved                                                               */
/*                                                                                   */
/*-----------------------------------------------------------------------------------*/
/* File Contents:                                                                    */
/*   spiflash.c                 		                                             */
/*                                                                                   */
/* This file contains:                                                               */
/*                                                                                   */
/* Project:                                                                          */
/*                                                                                   */
/* Remark:                                                                           */
/*                                                                                   */
/*-----------------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "N9H20.h"

#define TEST_SIZE	512 * 2 * 64
#if defined(__GNUC__)
__attribute__((aligned(4096))) UINT8 WriteBuffer[TEST_SIZE];
__attribute__((aligned(4096))) UINT8 ReadBuffer[TEST_SIZE];
#else
__align(4096) UINT8 WriteBuffer[TEST_SIZE];
__align(4096) UINT8 ReadBuffer[TEST_SIZE];
#endif

int main()
{
	WB_UART_T uart;
	UINT32 u32ExtFreq;
	unsigned char *pSrc, *pDst;
	int volatile i;

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

	sysprintf("SpiFlash Test...\n");

	pSrc = (UINT8 *)((UINT32)WriteBuffer | 0x80000000);
	pDst = (UINT8 *)((UINT32)ReadBuffer | 0x80000000);

	for (i=0; i<TEST_SIZE; i++)
		*(pSrc+i) = i & 0xff;

	spiFlashInit();

	sysprintf("Erase all SpiFlash\n");
	spiFlashEraseAll();

	sysprintf("\tWrite SpiFlash\n");
	spiFlashWrite(0, TEST_SIZE, (UINT32 *)pSrc);

	sysprintf("\tRead and Compare SpiFlash\n");
	spiFlashRead(0, TEST_SIZE, (UINT32 *)pDst);

	for (i=0; i<TEST_SIZE; i++)
	{
		if (*(pSrc+i) != *(pDst+i))
		{
			sysprintf("error!! Src[%d] = 0x%X, Dst[%d] = 0x%X\n", i, *(pSrc+i), i, *(pDst+i));
			break;
		}
	}

	sysprintf("finish SPI test\n");
	return 0;
}

