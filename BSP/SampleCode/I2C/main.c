/*-----------------------------------------------------------------------------------*/
/* Nuvoton Technology Corporation confidential                                      */
/*                                                                                   */
/* Copyright (c) 2008 by Nuvoton Technology Corporation                             */
/* All rights reserved                                                               */
/*                                                                                   */
/*-----------------------------------------------------------------------------------*/
 
#include <stdio.h>
#include <string.h>
#include "N9H20.h"

#define RETRY		1000  /* Programming cycle may be in progress. Please refer to 24LC64 datasheet */

#define TXSIZE		512
#define ADDROFFSET  1024


//------------------------- Program -------------------------//
void i2cExample (void)
{
	unsigned char data[TXSIZE], value[TXSIZE];
	int i, j, err, cnt;
	INT32 rtval;
	
	/* initialize test data */
	for(i = 0 ; i < TXSIZE ; i++)
		data[i] = i + 1;
	i2cInit();	
	
	/* Byte Write/Random Read */
	sysprintf("\nEEPROM 24LC64 Byte Write/Random Read ...\n");
	
	rtval = i2cOpen();
	if(rtval < 0)
	{
		sysprintf("Open I2C error!\n");
		goto exit_test;
	}	
	i2cIoctl(I2C_IOC_SET_DEV_ADDRESS, 0x50, 0);  
	i2cIoctl(I2C_IOC_SET_SPEED, 100, 0);
	
	/* Tx porcess */
	sysprintf("Start Tx\n");
	for(i = 0 ; i < TXSIZE ; i++)
	{
		i2cIoctl(I2C_IOC_SET_SUB_ADDRESS, i, 2);	
		j = RETRY;	
		while(j-- > 0) 
		{
			if(i2cWrite(&data[i], 1) == 1)
				break;
		}						
		if(j <= 0)
			sysprintf("WRITE ERROR [%d]!\n", i);

		sysDelay(1);
	}
	
	/* Rx porcess */	
	sysprintf("Start Rx\n");
	memset(value, 0 , TXSIZE);
	for(i = 0 ; i < TXSIZE ; i++)
	{
		i2cIoctl(I2C_IOC_SET_SUB_ADDRESS, i, 2);	
		j = RETRY;
		while(j-- > 0) 
		{
			if(i2cRead(&value[i], 1) == 1)
				break;
		}
		if(j <= 0)
			sysprintf("Read ERROR [%d]!\n", i);
	}
	
	/* Compare process */
	sysprintf("Compare data\n");
	err = 0;
	cnt = 0;
	for(i = 0 ; i < TXSIZE ; i++)
	{
		if(value[i] != data[i])	
		{
			sysprintf("[%d] addr = 0x%08x, val = 0x%02x (should be 0x%02x)\n", i, i + ADDROFFSET, value[i], data[i]);
			err = 1;
			cnt++;			
		}
	}					
	if(err)
		sysprintf("Test failed [%d]!\n\n", cnt);	
	else
		sysprintf("Test success\n\n");	

	
	i2cClose();		
	/* Test End */
	
exit_test:
	
	sysprintf("\nTest finish ...\n");	
	i2cExit();
	
	return;
}	

int main (void)
{
	WB_UART_T uart;
	UINT32 u32ExtFreq;

	sysSetSystemClock(eSYS_UPLL, 	//E_SYS_SRC_CLK eSrcClk,	
						192000,		//UINT32 u32PllKHz, 	
						192000,		//UINT32 u32SysKHz,
						192000,		//UINT32 u32CpuKHz,
						  96000,		//UINT32 u32HclkKHz,
						  48000);		//UINT32 u32ApbKHz		

	u32ExtFreq = sysGetExternalClock();    	/* Hz unit */	
	sysUartPort(1); 
	uart.uiFreq = u32ExtFreq * 1000;
	uart.uiBaudrate = 115200;
	uart.uiDataBits = WB_DATA_BITS_8;
	uart.uiStopBits = WB_STOP_BITS_1;
	uart.uiParity = WB_PARITY_NONE;
	uart.uiRxTriggerLevel = LEVEL_1_BYTE;
	sysInitializeUART(&uart);	

	i2cExample();
	
	return 0;
}




