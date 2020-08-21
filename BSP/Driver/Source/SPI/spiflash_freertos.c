/**************************************************************************//**
 * @file     spiflash_freertos.c
 * @version  V3.00
 * @brief    N9H20 series SPI flash FreeRTOS driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
/* Header files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wblib.h"

#include "N9H20_SPI.h"

UINT8 g_u8Is4ByteMode;
//#define SPI_CLOCK_MODE	0
#define SPI_CLOCK_MODE	3

int usiCheckBusy(UINT32 spiPort, UINT32 SSPin)
{
	volatile UINT32 u32status;
	// check status
	spiSSEnable(spiPort, SSPin, SPI_CLOCK_MODE);

	// status command
	outpw(REG_SPI0_TX0 + 0x400 * spiPort, 0x05);
	spiTxLen(spiPort, 0, 8);
	spiActive(spiPort);

	// get status
	while(1)
	{
		outpw(REG_SPI0_TX0 + 0x400 * spiPort, 0xff);
		spiTxLen(spiPort, 0, 8);
		spiActive(spiPort);

		u32status = inpw(REG_SPI0_RX0 + 0x400 * spiPort);		
		if(((u32status & 0xff) & 0x01) != 0x01)
			break;		
	}

	spiSSDisable(spiPort, SSPin);

	return Successful;
}

INT16 usiReadID(UINT32 spiPort, UINT32 SSPin)
{
	UINT16 volatile id;

	spiSSEnable(spiPort, SSPin, SPI_CLOCK_MODE);

	// command 8 bit
	outpw(REG_SPI0_TX0 + 0x400 * spiPort, 0x90);
	spiTxLen(spiPort, 0, 8);
	spiActive(spiPort);

	// address 24 bit
	outpw(REG_SPI0_TX0 + 0x400 * spiPort, 0x000000);
	spiTxLen(spiPort, 0, 24);
	spiActive(spiPort);

	// data 16 bit
	outpw(REG_SPI0_TX0 + 0x400 * spiPort, 0xffff);
	spiTxLen(spiPort, 0, 16);
	spiActive(spiPort);
	id = inpw(REG_SPI0_RX0 + 0x400 * spiPort) & 0xffff;

	spiSSDisable(spiPort, SSPin);

	return id;
}


int usiWriteEnable(UINT32 spiPort, UINT32 SSPin)
{
	spiSSEnable(spiPort, SSPin, SPI_CLOCK_MODE);

	outpw(REG_SPI0_TX0 + 0x400 * spiPort, 0x06);
	spiTxLen(spiPort, 0, 8);
	spiActive(spiPort);

	spiSSDisable(spiPort, SSPin);

	return Successful;
}

int usiWriteDisable(UINT32 spiPort, UINT32 SSPin)
{
	spiSSEnable(spiPort, SSPin, SPI_CLOCK_MODE);

	outpw(REG_SPI0_TX0 + 0x400 * spiPort, 0x04);
	spiTxLen(spiPort, 0, 8);
	spiActive(spiPort);

	spiSSDisable(spiPort, SSPin);

	return Successful;
}

int usiStatusWrite(UINT32 spiPort, UINT32 SSPin, UINT8 data)
{
	usiWriteEnable(spiPort, SSPin);

	spiSSEnable(spiPort, SSPin, SPI_CLOCK_MODE);

	// status command
	outpw(REG_SPI0_TX0 + 0x400 * spiPort, 0x01);
	spiTxLen(spiPort, 0, 8);
	spiActive(spiPort);

	// write status
	outpw(REG_SPI0_TX0 + 0x400 * spiPort, data);
	spiTxLen(spiPort, 0, 8);
	spiActive(spiPort);

	spiSSDisable(spiPort, SSPin);

	return Successful;
}

/**************************************************/
INT spiFlashInit(UINT32 spiPort, UINT32 SSPin)
{
	static BOOL bIsSpiFlashOK = 0;
	int volatile loop;
	UINT32 u32APBClk;	

	if (!bIsSpiFlashOK)
	{
		outpw(REG_APBCLK, inpw(REG_APBCLK) | SPIMS0_CKE);	// turn on SPI clk
		//Reset SPI controller first
//		outpw(REG_APBIPRST, inpw(REG_APBIPRST) | SPI0RST);
//		outpw(REG_APBIPRST, inpw(REG_APBIPRST) & ~SPI0RST);
		// delay for time
		// Delay
	    for (loop=0; loop<500; loop++);  
		//configure SPI0 interface, Base Address 0xB800C000

		/* apb clock is 48MHz, output clock is 10MHz */
		u32APBClk = sysGetAPBClock();		
		spiIoctl(spiPort, SPI_SET_CLOCK, u32APBClk/1000, 10000);

		//Startup SPI0 multi-function features, chip select using SS0
		outpw(REG_GPDFUN, inpw(REG_GPDFUN) | 0xFF000000);

		outpw(REG_SPI0_SSR + 0x400 * spiPort, 0x00);		// CS active low
		outpw(REG_SPI0_CNTRL + 0x400 * spiPort, 0x04);		// Tx: falling edge, Rx: rising edge

		if ((loop=usiReadID(spiPort, SSPin)) == -1)
		{
			sysprintf("read id error !! [0x%x]\n", loop&0xffff);
			return -1;
		}

		sysprintf("SPI flash id [0x%x]\n", loop&0xffff);
		usiStatusWrite(spiPort, SSPin, 0x00);	// clear block protect

		// check status
		usiCheckBusy(spiPort, SSPin);		

		bIsSpiFlashOK = 1;
	}	
	return 0;
 }

INT spiFlashReset(UINT32 spiPort, UINT32 SSPin)
{
	// check status
	usiCheckBusy(spiPort, SSPin);

	spiSSEnable(spiPort, SSPin, SPI_CLOCK_MODE);

	// enable reset command
	outpw(REG_SPI0_TX0 + 0x400 * spiPort, 0x66);
	spiTxLen(spiPort, 0, 8);
	spiActive(spiPort);

	spiSSDisable(spiPort, SSPin);

	spiSSEnable(spiPort, SSPin, SPI_CLOCK_MODE);

	// reset command
	outpw(REG_SPI0_TX0 + 0x400 * spiPort, 0x99);
	spiTxLen(spiPort, 0, 8);
	spiActive(spiPort);

	spiSSDisable(spiPort, SSPin);

	sysDelay(5);

	usiStatusWrite(spiPort, SSPin, 0x00);	// clear block protect

	return Successful;
}


INT spiFlashEraseSector(UINT32 spiPort, UINT32 SSPin, UINT32 addr, UINT32 secCount)
{
	int volatile i;

	if ((addr % (4*1024)) != 0)
		return -1;

	for (i=0; i<secCount; i++)
	{
		usiWriteEnable(spiPort, SSPin);

		spiSSEnable(spiPort, SSPin, SPI_CLOCK_MODE);

		// erase command
		outpw(REG_SPI0_TX0 + 0x400 * spiPort, 0x20);
		spiTxLen(spiPort, 0, 8);
		spiActive(spiPort);

		// address
		if(g_u8Is4ByteMode == TRUE)
		{
			outpw(REG_SPI0_TX0 + 0x400 * spiPort, addr+i*4*1024);
			spiTxLen(spiPort, 0, 32);
			spiActive(spiPort);
		}
		else
		{
			outpw(REG_SPI0_TX0 + 0x400 * spiPort, addr+i*4*1024);
			spiTxLen(spiPort, 0, 24);
			spiActive(spiPort);
		}

		spiSSDisable(spiPort, SSPin);

		// check status
		usiCheckBusy(spiPort, SSPin);
	}

	return Successful;
}


INT spiFlashEraseBlock(UINT32 spiPort, UINT32 SSPin, UINT32 addr, UINT32 blockCount)
{
	int volatile i;

	if ((addr % (64*1024)) != 0)
		return -1;

	for (i=0; i<blockCount; i++)
	{
		usiWriteEnable(spiPort, SSPin);

		spiSSEnable(spiPort, SSPin, SPI_CLOCK_MODE);

		// erase command
		outpw(REG_SPI0_TX0 + 0x400 * spiPort, 0xd8);
		spiTxLen(spiPort, 0, 8);
		spiActive(spiPort);

		// address
		if(g_u8Is4ByteMode == TRUE)
		{
			outpw(REG_SPI0_TX0 + 0x400 * spiPort, addr+i*64*1024);
			spiTxLen(spiPort, 0, 32);
			spiActive(spiPort);
		}
		else
		{
			outpw(REG_SPI0_TX0 + 0x400 * spiPort, addr+i*64*1024);
			spiTxLen(spiPort, 0, 24);
			spiActive(spiPort);
		}

		spiSSDisable(spiPort, SSPin);

		// check status
		usiCheckBusy(spiPort, SSPin);
	}

	return Successful;
}


INT spiFlashEraseAll(UINT32 spiPort, UINT32 SSPin)
{
	usiWriteEnable(spiPort, SSPin);

	spiSSEnable(spiPort, SSPin, SPI_CLOCK_MODE);

	outpw(REG_SPI0_TX0 + 0x400 * spiPort, 0xc7);
	spiTxLen(spiPort, 0, 8);
	spiActive(spiPort);

	spiSSDisable(spiPort, SSPin);

	// check status
	usiCheckBusy(spiPort, SSPin);

	return Successful;
}


INT spiFlashWrite(UINT32 spiPort, UINT32 SSPin, UINT32 addr, UINT32 len, UINT8 *buf)
{
	int volatile count=0, page, i, j;	
	PUINT8 ptr;
	PUINT32 p32tmp;

	p32tmp = (PUINT32)buf;	

	count = len / 256;
	if ((len % 256) != 0)
		count++;

	for (i=0; i<count; i++)
	{
		// check data len
		if (len >= 256)
		{
			page = 256;
			len = len - 256;
		}
		else
			page = len;

		usiWriteEnable(spiPort, SSPin);

		spiSSEnable(spiPort, SSPin, SPI_CLOCK_MODE);

		// write command
		outpw(REG_SPI0_TX0 + 0x400 * spiPort, 0x02);
		spiTxLen(spiPort, 0, 8);
		spiActive(spiPort);

		// address
		if(g_u8Is4ByteMode == TRUE)
		{
			outpw(REG_SPI0_TX0 + 0x400 * spiPort, addr+i*256);
			spiTxLen(spiPort, 0, 32);
			spiActive(spiPort);
		}
		else
		{
			outpw(REG_SPI0_TX0 + 0x400 * spiPort, addr+i*256);
			spiTxLen(spiPort, 0, 24);
			spiActive(spiPort);
		}

		spiSetByteEndin(spiPort, eDRVSPI_ENABLE);

		// write data
		while (page > 0)
		{			
			spiTxLen(spiPort, 0, 32);
			outpw(REG_SPI0_TX0 + 0x400 * spiPort, *p32tmp);			
			spiActive(spiPort);
			p32tmp++;			
			page -=4;

			if(page < 4)
			{
				if(page > 0)
				{
					spiSetByteEndin(spiPort, eDRVSPI_DISABLE);
					ptr = (PUINT8)p32tmp;	
					
					for (j=0; j<(page %4); j++)
					{
						spiTxLen(spiPort, 0, 8);
						outpw(REG_SPI0_TX0 + 0x400 * spiPort, *ptr);						
						spiActive(spiPort);
						ptr++;			
						page -=1;
					}
				}
			}
			
		}

		spiSetByteEndin(spiPort, eDRVSPI_DISABLE);		

		spiSSDisable(spiPort, SSPin);

		// check status
		usiCheckBusy(spiPort, SSPin);
	}

	return Successful;
}


INT spiFlashRead(UINT32 spiPort, UINT32 SSPin, UINT32 addr, UINT32 len, UINT8 *buf)
{
	int volatile i;	
	PUINT8 ptr;
	PUINT32 p32tmp;	

	p32tmp = (PUINT32)buf;	

	spiSSEnable(spiPort, SSPin, SPI_CLOCK_MODE);

	// read command
	outpw(REG_SPI0_TX0 + 0x400 * spiPort, 03);
	spiTxLen(spiPort, 0, 8);
	spiActive(spiPort);

	// address
	if(g_u8Is4ByteMode == TRUE)
	{
		outpw(REG_SPI0_TX0 + 0x400 * spiPort, addr);
		spiTxLen(spiPort, 0, 32);
		spiActive(spiPort);
	}
	else
	{
		outpw(REG_SPI0_TX0 + 0x400 * spiPort, addr);
		spiTxLen(spiPort, 0, 24);
		spiActive(spiPort);
	}

	spiSetByteEndin(spiPort, eDRVSPI_ENABLE);

	// data	 
	for (i=0; i<len/4; i++)
	{
		spiTxLen(spiPort, 0, 32);
		outpw(REG_SPI0_TX0 + 0x400 * spiPort, 0xffffffff);		
		spiActive(spiPort);
		*p32tmp = inp32(REG_SPI0_RX0 + 0x400 * spiPort);
		p32tmp ++;		
	}	

	spiSetByteEndin(spiPort, eDRVSPI_DISABLE);
	
	if(len % 4)
	{
		ptr = (PUINT8)p32tmp;		
		
		for (i=0; i<(len %4); i++)
		{
			spiTxLen(spiPort, 0, 8);
			outpb(REG_SPI0_TX0 + 0x400 * spiPort, 0xff);			
			spiActive(spiPort);
			*ptr++ = inpb(REG_SPI0_RX0 + 0x400 * spiPort);
		}		
	}	

	spiSSDisable(spiPort, SSPin);

	return Successful;
}

INT spiFlashEnter4ByteMode(UINT32 spiPort, UINT32 SSPin)
{
	spiSSEnable(spiPort, SSPin, SPI_CLOCK_MODE);

	// enter 4 byte mode command
	outpw(REG_SPI0_TX0 + 0x400 * spiPort, 0xB7);
	spiTxLen(spiPort, 0, 8);
	spiActive(spiPort);

	spiSSDisable(spiPort, SSPin);

	g_u8Is4ByteMode = TRUE;

	return Successful;
}

INT spiFlashExit4ByteMode(UINT32 spiPort, UINT32 SSPin)
{
	spiSSEnable(spiPort, SSPin, SPI_CLOCK_MODE);

	// exit 4 byte mode command
	outpw(REG_SPI0_TX0 + 0x400 * spiPort, 0xE9);
	spiTxLen(spiPort, 0, 8);
	spiActive(spiPort);

	spiSSDisable(spiPort, SSPin);

	g_u8Is4ByteMode = FALSE;

	return Successful;
}


