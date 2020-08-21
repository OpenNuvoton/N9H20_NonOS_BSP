/**************************************************************************//**
 * @file     Smpl_Pdma_SPI.c
 * @brief    Demonstrate SPI data transfer with PDMA.
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "N9H20.h"

//#define TEST_SIZE	512 * 2 * 1024
#define TEST_SIZE	100 * 1024
#if defined(__GNUC__)
__attribute__((aligned(4096))) UINT8 WriteBuffer[TEST_SIZE];
__attribute__((aligned(4096))) UINT8 ReadBuffer[TEST_SIZE];
#else
__align(4096) UINT8 WriteBuffer[TEST_SIZE];
__align(4096) UINT8 ReadBuffer[TEST_SIZE];
#endif

static INT32 g_PdmaCh = 0;
volatile static BOOL g_bPdmaInt = FALSE;

extern int usiWriteEnable(void);
extern int usiCheckBusy(void);
extern int usiStatusRead(UINT8 cmd, PUINT8 data);

void PdmaCallback_SPI(unsigned int arg)
{ 	
	EDMA_Free(g_PdmaCh);  	
	g_bPdmaInt = TRUE;
}

int initSPIPDMA_Write(UINT32 src_addr, UINT32 dma_length)
{
	
	g_PdmaCh = PDMA_FindandRequest();
	EDMA_SetAPB(g_PdmaCh,			//int channel, 
						eDRVEDMA_SPIMS0,			//E_DRVEDMA_APB_DEVICE eDevice, 
						eDRVEDMA_WRITE_APB,		//E_DRVEDMA_APB_RW eRWAPB, 
						eDRVEDMA_WIDTH_32BITS);	//E_DRVEDMA_TRANSFER_WIDTH eTransferWidth	

	EDMA_SetupHandlers(g_PdmaCh, 		//int channel
						eDRVEDMA_BLKD, 			//int interrupt,	
						PdmaCallback_SPI, 				//void (*irq_handler) (void *),
						NULL);					//void *data

	EDMA_SetWrapINTType(g_PdmaCh , 0);								

	EDMA_SetDirection(g_PdmaCh , eDRVEDMA_DIRECTION_INCREMENTED, eDRVEDMA_DIRECTION_FIXED);


	EDMA_SetupSingle(g_PdmaCh,		// int channel, 
								src_addr,		// unsigned int src_addr,  (ADC data port physical address) 
								REG_SPI0_TX0, //phaddrrecord,		// unsigned int dest_addr,
								dma_length);	// unsigned int dma_length /* Lenth equal 2 half buffer */

	return Successful;
}

int initSPIPDMA_Read(UINT32 dest_addr, UINT32 dma_length)
{
	
	g_PdmaCh = PDMA_FindandRequest();
	EDMA_SetAPB(g_PdmaCh,			//int channel, 
						eDRVEDMA_SPIMS0,			//E_DRVEDMA_APB_DEVICE eDevice, 
						eDRVEDMA_READ_APB,		//E_DRVEDMA_APB_RW eRWAPB, 
						eDRVEDMA_WIDTH_32BITS);	//E_DRVEDMA_TRANSFER_WIDTH eTransferWidth	

	EDMA_SetupHandlers(g_PdmaCh, 		//int channel
						eDRVEDMA_BLKD, 			//int interrupt,	
						PdmaCallback_SPI, 				//void (*irq_handler) (void *),
						NULL);					//void *data

	EDMA_SetWrapINTType(g_PdmaCh , 0);								

	EDMA_SetDirection(g_PdmaCh , eDRVEDMA_DIRECTION_FIXED, eDRVEDMA_DIRECTION_INCREMENTED);


	EDMA_SetupSingle(g_PdmaCh,		// int channel, 
								REG_SPI0_RX0,		// unsigned int src_addr,  (ADC data port physical address) 
								dest_addr, //phaddrrecord,		// unsigned int dest_addr,
								dma_length);	// unsigned int dma_length /* Lenth equal 2 half buffer */

	return Successful;
}


INT spiFlashPDMAWrite(UINT32 addr, UINT32 len, UINT32 *buf)
{
	UINT32 count=0, page, i, tmp;	
	
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

		//sysprintf("len %d page %d\n",len , page);
		usiWriteEnable();

		outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) | 0x01);	// CS0

		// write command
		outpw(REG_SPI0_TX0, 0x02);
		spiTxLen(0, 0, 8);
		spiActive(0);

		// address
		outpw(REG_SPI0_TX0, addr+i*256);
		spiTxLen(0, 0, 24);
		spiActive(0);	

		outpw(REG_SPI0_CNTRL, inpw(REG_SPI0_CNTRL) | BYTE_ENDIN);
		spiTxLen(0, 0, 32);		

		initSPIPDMA_Write((UINT32)buf, page);
		tmp = (UINT32)buf + page;
		buf = (UINT32 *)tmp;		

		EDMA_Trigger(g_PdmaCh);
		outp32(REG_SPI0_EDMA, (inp32(REG_SPI0_EDMA) & ~0x03) | EDMA_GO);							

		while(g_bPdmaInt == FALSE);	
		g_bPdmaInt = FALSE;

		outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & 0xfe);	// CS0

		outpw(REG_SPI0_CNTRL, inpw(REG_SPI0_CNTRL) & ~BYTE_ENDIN);
		spiTxLen(0, 0, 8);
		outp32(REG_SPI0_EDMA, inp32(REG_SPI0_EDMA) & ~EDMA_GO);	

		// check status
		usiCheckBusy();
	}

	return Successful;
}

INT spiFlashPDMARead(UINT32 addr, UINT32 len, UINT32 *buf)
{
	outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) | 0x01);	// CS0

	// read command
	outpw(REG_SPI0_TX0, 0x0b);
	spiTxLen(0, 0, 8);
	spiActive(0);

	// address
	outpw(REG_SPI0_TX0, addr);
	spiTxLen(0, 0, 24);
	spiActive(0);

	// dummy byte
	outp32(REG_SPI0_TX0, 0xff);
	spiTxLen(0, 0, 8);
	spiActive(0);

	outpw(REG_SPI0_CNTRL, inpw(REG_SPI0_CNTRL) | BYTE_ENDIN);
	spiTxLen(0, 0, 32);

	initSPIPDMA_Read((UINT32)buf, len);
	
	EDMA_Trigger(g_PdmaCh);
	outp32(REG_SPI0_EDMA, (inp32(REG_SPI0_EDMA) & ~0x03) | (EDMA_RW | EDMA_GO));	
	outp32(REG_SPI0_CNTRL, inp32(REG_SPI0_CNTRL) |GO_BUSY);	    
	
	while(g_bPdmaInt == FALSE);	
	g_bPdmaInt = FALSE;
	
	outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & 0xfe);	// CS0

	outpw(REG_SPI0_CNTRL, inpw(REG_SPI0_CNTRL) & ~BYTE_ENDIN);
	spiTxLen(0, 0, 8);
	outp32(REG_SPI0_EDMA, inp32(REG_SPI0_EDMA) & ~EDMA_GO);

	return Successful;
}

void SPIFlashTest(void)
{
	unsigned char *pSrc, *pDst;
	int i;	

	pSrc = (UINT8 *)((UINT32)WriteBuffer | NON_CACHE_BIT);
	pDst = (UINT8 *)((UINT32)ReadBuffer | NON_CACHE_BIT);

	for (i=0; i<TEST_SIZE; i++)
		*(pSrc+i) = i & 0xff;

	spiFlashInit();		

	sysprintf("\tErase all SpiFlash\n");
	spiFlashEraseAll();

	sysprintf("\tWrite SpiFlash\n");	
	spiFlashPDMAWrite(0, TEST_SIZE, (UINT32 *)pSrc);	

	sysprintf("\tRead and Compare SpiFlash\n");	
	spiFlashPDMARead(0, TEST_SIZE, (UINT32 *)pDst);	

	for (i=0; i<TEST_SIZE; i++)
	{
		if (*(pSrc+i) != *(pDst+i))
		{
			sysprintf("error!! Src[%d] = 0x%X, Dst[%d] = 0x%X\n", i, *(pSrc+i), i, *(pDst+i));
			while(1);
		}
	}

	sysprintf("finish SPI test\n");
	
}








