/**************************************************************************//**
 * @file     spi.c
 * @version  V3.00
 * @brief    N9H20 series SPI driver source file
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

#ifdef __FreeRTOS__
/* Scheduler include files. */
#include "FreeRTOS.h"
#include "semphr.h"

// the semaphore used to wake the PDMA task when data transfer is finish
INT Spi0CsInit, Spi1CsInit;
xSemaphoreHandle xSpi0CsSemaphore, xSpi1CsSemaphore;

#ifdef __RAK439__
extern void FLASH_LOCK();
extern void FLASH_UNLOCK();
#endif
#endif

static PFN_DRVSPI_CALLBACK g_pfnSPI0callback = NULL;
static PFN_DRVSPI_CALLBACK g_pfnSPI1callback = NULL;

/*-----------------------------------------------------------------------------------*/
int spiActive(int port)
{
	if (port == 0)
	{
		outpw(REG_SPI0_CNTRL, inpw(REG_SPI0_CNTRL)|0x01);
		while(inpw(REG_SPI0_CNTRL) & 0x01);
	}
	else
	{
		outpw(REG_SPI1_CNTRL, inpw(REG_SPI1_CNTRL)|0x01);
		while(inpw(REG_SPI1_CNTRL) & 0x01);
	}
	return 0;
}

/*-----------------------------------------------------------------------------------*/
VOID spiSetGo(UINT8 u8Port)
{
	UINT32 u32Reg;

	if (u8Port == 0)
	{
		u32Reg = inpw(REG_SPI0_CNTRL);
		outpw(REG_SPI0_CNTRL, u32Reg | GO_BUSY);
	}
	else
	{
		u32Reg = inpw(REG_SPI1_CNTRL);
		outpw(REG_SPI1_CNTRL, u32Reg | GO_BUSY);
	}	
}

/*-----------------------------------------------------------------------------------*/
BOOL spiIsBusy(UINT8 u8Port)
{
	UINT32 u32reg;

	if(u8Port == 0)
		u32reg = inpw(REG_SPI0_CNTRL);
	else
		u32reg = inpw(REG_SPI1_CNTRL);	

	return ((u32reg & GO_BUSY)?TRUE:FALSE);
}

/*-----------------------------------------------------------------------------------*/
int spiTxLen(int port, int count, int bitLen)
{
	unsigned int reg;

	if (port == 0)
		reg = inpw(REG_SPI0_CNTRL);
	else
		reg = inpw(REG_SPI1_CNTRL);

	if ((count < 0) || (count > 3))
		return -1;

	if ((bitLen <= 0) || (bitLen > 32))
		return -1;

	if (bitLen == 32)
		reg = reg & 0xffffff07;
	else
		reg = (reg & 0xffffff07) | (bitLen << 3);
	reg = (reg & 0xfffffcff) | (count << 8);

	if (port == 0)
		outpw(REG_SPI0_CNTRL, reg);
	else
		outpw(REG_SPI1_CNTRL, reg);

	return 0;
}

/*-----------------------------------------------------------------------------------*/
void spiSetClock(int port, int clock_by_MHz, int output_by_kHz)
{
	int volatile divider;

	divider = (clock_by_MHz * 1000) / (2 * output_by_kHz) - 1;
	if (port == 0)
		outpw(REG_SPI0_DIVIDER, divider);
	else
		outpw(REG_SPI1_DIVIDER, divider);
}

/*-----------------------------------------------------------------------------------*/
VOID spiSetByteEndin(UINT8 u8Port, E_DRVSPI_OPERATION eOP)
{
	if (u8Port == 0)
	{
		if(eOP == eDRVSPI_ENABLE)
			outpw(REG_SPI0_CNTRL, inpw(REG_SPI0_CNTRL) |BYTE_ENDIN);
		else
			outpw(REG_SPI0_CNTRL, inpw(REG_SPI0_CNTRL) & ~BYTE_ENDIN);
	}
	else
	{
		if(eOP == eDRVSPI_ENABLE)
			outpw(REG_SPI1_CNTRL, inpw(REG_SPI1_CNTRL) |BYTE_ENDIN);
		else
			outpw(REG_SPI1_CNTRL, inpw(REG_SPI1_CNTRL) & ~BYTE_ENDIN);
	}
}

/*-----------------------------------------------------------------------------------*/
VOID spiEnableInt(UINT8 u8Port)
{
	if(u8Port == 0)
		outpw(REG_SPI0_CNTRL, inpw(REG_SPI0_CNTRL) |IE);
	else
		outpw(REG_SPI1_CNTRL, inpw(REG_SPI1_CNTRL) |IE);	
}	

/*-----------------------------------------------------------------------------------*/
VOID spiDisableInt(UINT8 u8Port)
{
	if(u8Port == 0)
		outpw(REG_SPI0_CNTRL, inpw(REG_SPI0_CNTRL) & ~IE);
	else
		outpw(REG_SPI1_CNTRL, inpw(REG_SPI1_CNTRL) & ~IE);	
}	

/*-----------------------------------------------------------------------------------*/
VOID spi0IRQHandler(void)
{
    outpw(REG_SPI0_CNTRL, inpw(REG_SPI0_CNTRL) |IFG);
	
    if(g_pfnSPI0callback != NULL)
    {
        g_pfnSPI0callback();
    }
}

/*-----------------------------------------------------------------------------------*/
VOID spi1IRQHandler(void)
{
    outpw(REG_SPI1_CNTRL, inpw(REG_SPI1_CNTRL) |IFG);
	
    if(g_pfnSPI1callback != NULL)
    {
        g_pfnSPI1callback();
    }
}

/*-----------------------------------------------------------------------------------*/
INT32
spiInstallCallBack(
	UINT8 u8Port,
	PFN_DRVSPI_CALLBACK pfncallback,
	PFN_DRVSPI_CALLBACK *pfnOldcallback
)
{
	if (u8Port == 0)
	{
		*pfnOldcallback = g_pfnSPI0callback;
		g_pfnSPI0callback = pfncallback;        
		sysInstallISR(IRQ_LEVEL_7, IRQ_SPIMS0, (PVOID)spi0IRQHandler);	
		sysSetLocalInterrupt(ENABLE_IRQ);
		sysEnableInterrupt(IRQ_SPIMS0);
	}
	else
	{
		*pfnOldcallback = g_pfnSPI1callback;
		g_pfnSPI1callback = pfncallback;        
		sysInstallISR(IRQ_LEVEL_7, IRQ_SPIMS1, (PVOID)spi1IRQHandler);	
		sysSetLocalInterrupt(ENABLE_IRQ);
		sysEnableInterrupt(IRQ_SPIMS1);
	}

	return Successful;
}

/*-----------------------------------------------------------------------------------*/
static int _spi_init_flag0 = FALSE, _spi_init_flag1 = FALSE;
int spiOpen(SPI_INFO_T *pInfo)
{
	int volatile i;

	if (pInfo->nPort == 0)
	{
		if(_spi_init_flag0)
		{
			sysprintf("SPI0 pin is unavailable!!\n");
			return -1;
		}

		if (!_spi_init_flag0)
		{
			outpw(REG_APBCLK, inpw(REG_APBCLK) | SPIMS0_CKE);   // enable the clocks of SPI
			outpw(REG_APBIPRST, inpw(REG_APBIPRST) | SPI0RST);
			outpw(REG_APBIPRST, inpw(REG_APBIPRST) & ~SPI0RST);
			for (i=0; i<200; i++);

			outpw(REG_GPDFUN, inpw(REG_GPDFUN) | 0xFF000000);	// enable spi0 pin function

			if (pInfo->bIsSlaveMode)
				outpw(REG_SPI0_CNTRL, inpw(REG_SPI0_CNTRL) | 0x40000);
			else	// master mode
				outpw(REG_SPI0_CNTRL, inpw(REG_SPI0_CNTRL) & ~0x40000);

			if (pInfo->bIsClockIdleHigh)
				outpw(REG_SPI0_CNTRL, inpw(REG_SPI0_CNTRL) | 0x800);
			else	// clock idle low
				outpw(REG_SPI0_CNTRL, inpw(REG_SPI0_CNTRL) & ~0x800);

			if (pInfo->bIsLSBFirst)
				outpw(REG_SPI0_CNTRL, inpw(REG_SPI0_CNTRL) | 0x400);
			else	// MSB first
				outpw(REG_SPI0_CNTRL, inpw(REG_SPI0_CNTRL) & ~0x400);

			if (pInfo->bIsAutoSelect)
				outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) | 0x8);
			else
				outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & ~0x8);

			if (pInfo->bIsActiveLow)
				outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & ~0x4);
			else	// active high
				outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) | 0x4);

			if (pInfo->bIsTxNegative)
				outpw(REG_SPI0_CNTRL, (inpw(REG_SPI0_CNTRL) & ~0x6)| 0x4); // Tx falling; Rx rising
			else
				outpw(REG_SPI0_CNTRL, (inpw(REG_SPI0_CNTRL) & ~0x6)| 0x2); // Tx rising; Rx falling

			if (pInfo->bIsLevelTrigger)
				outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) |SS_LTRIG);
			else	// edge trigger
				outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & ~SS_LTRIG);				

			_spi_init_flag0 = TRUE;
		}
	}
	else
	{
		if(_spi_init_flag1)
		{
			sysprintf("SPI1 pin is unavailable!!\n");
			return -1;
		}

		if (!_spi_init_flag1)
		{
			outpw(REG_APBCLK, inpw(REG_APBCLK) | SPIMS1_CKE);   // enable the clocks of SPI
			outpw(REG_APBIPRST, inpw(REG_APBIPRST) | SPI1RST);
			outpw(REG_APBIPRST, inpw(REG_APBIPRST) & ~SPI1RST);
			for (i=0; i<200; i++);

			outpw(REG_GPBFUN, (inpw(REG_GPBFUN) & ~0x03FC0000) | 0x01540000);	// enable spi1 pin function

			if (pInfo->bIsSlaveMode)
				outpw(REG_SPI1_CNTRL, inpw(REG_SPI1_CNTRL) | 0x40000);		
			else	// master mode
				outpw(REG_SPI1_CNTRL, inpw(REG_SPI1_CNTRL) & ~0x40000);

			if (pInfo->bIsClockIdleHigh)
				outpw(REG_SPI1_CNTRL, inpw(REG_SPI1_CNTRL) | 0x800);
			else	// clock idle low
				outpw(REG_SPI1_CNTRL, inpw(REG_SPI1_CNTRL) & ~0x800);

			if (pInfo->bIsLSBFirst)
				outpw(REG_SPI1_CNTRL, inpw(REG_SPI1_CNTRL) | 0x400);
			else	// MSB first
				outpw(REG_SPI1_CNTRL, inpw(REG_SPI1_CNTRL) & ~0x400);

			if (pInfo->bIsAutoSelect)
				outpw(REG_SPI1_SSR, inpw(REG_SPI1_SSR) | 0x8);
			else
				outpw(REG_SPI1_SSR, inpw(REG_SPI1_SSR) & ~0x8);

			if (pInfo->bIsActiveLow)
				outpw(REG_SPI1_SSR, inpw(REG_SPI1_SSR) & ~0x4);
			else	// active high
				outpw(REG_SPI1_SSR, inpw(REG_SPI1_SSR) | 0x4);

			if (pInfo->bIsTxNegative)
				outpw(REG_SPI1_CNTRL, (inpw(REG_SPI1_CNTRL) & ~0x6)| 0x4); // Tx falling; Rx rising
			else
				outpw(REG_SPI1_CNTRL, (inpw(REG_SPI1_CNTRL) & ~0x6)| 0x2); // Tx rising; Rx falling

			if (pInfo->bIsLevelTrigger)
				outpw(REG_SPI1_SSR, inpw(REG_SPI1_SSR) |SS_LTRIG);
			else	// edge trigger
				outpw(REG_SPI1_SSR, inpw(REG_SPI1_SSR) & ~SS_LTRIG);				

			_spi_init_flag1 = TRUE;
		}
	}
	return 0;
}

/*-----------------------------------------------------------------------------------*/
INT32 spiClose(UINT8 u8Port)
{
	INT32 i32TimeOut;	

	i32TimeOut = 0x10000;
	while(spiIsBusy(u8Port))
	{	
		if(i32TimeOut-- <= 0)
			break;            
	}

	if(i32TimeOut-- <= 0)
		return -1;	

	if (u8Port == 0)
		_spi_init_flag0 = FALSE;
	else
		_spi_init_flag1 = FALSE;

	if (u8Port == 0)
	{
		outpw(REG_APBIPRST, inpw(REG_APBIPRST) | SPI0RST);
		outpw(REG_APBIPRST, inpw(REG_APBIPRST) & ~SPI0RST);			
		sysDisableInterrupt(IRQ_SPIMS0);	
		g_pfnSPI0callback = NULL;
		outpw(REG_APBCLK, inpw(REG_APBCLK) & ~SPIMS0_CKE);   // disable the clocks of SPI
	}
	else
	{		
		outpw(REG_APBIPRST, inpw(REG_APBIPRST) | SPI1RST);
		outpw(REG_APBIPRST, inpw(REG_APBIPRST) & ~SPI1RST);
		sysDisableInterrupt(IRQ_SPIMS1);
		g_pfnSPI1callback = NULL;
		outpw(REG_APBCLK, inpw(REG_APBCLK) & ~SPIMS1_CKE);   // disable the clocks of SPI
	}

	return Successful;		
}

/*-----------------------------------------------------------------------------------*/

VOID spiIoctl(INT32 spiPort, INT32 spiFeature, INT32 spiArg0, INT32 spiArg1)
{
	switch(spiFeature)
	{
		case SPI_SET_CLOCK:
				spiSetClock(spiPort, spiArg0, spiArg1);
			break;
	}
}

/*-----------------------------------------------------------------------------------*/

INT spiEnable(INT32 spiPort)
{
	if (spiPort == 0)
		outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) | 0x01);
	else
		outpw(REG_SPI1_SSR, inpw(REG_SPI1_SSR) | 0x01);
	return 0;
}

/*-----------------------------------------------------------------------------------*/

INT spiDisable(INT32 spiPort)
{
	if (spiPort == 0)
		outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & ~0x01);
	else
		outpw(REG_SPI1_SSR, inpw(REG_SPI1_SSR) & ~0x01);
	return 0;
}

INT spiRead(INT port, INT RxBitLen, INT len, CHAR *pDst)
{
	switch (RxBitLen)
	{
		case SPI_32BIT:
			if (port == 0)
				outpw(REG_SPI0_TX0, 0xffffffff);
			else
				outpw(REG_SPI1_TX0, 0xffffffff);

			while (len-- > 0)
			{
				spiTxLen(port, 0, SPI_32BIT);
				spiActive(port);
				if (port == 0)
					*((INT32 *)pDst) = inpw(REG_SPI0_RX0) & 0xffffffff;
				else
					*((INT32 *)pDst) = inpw(REG_SPI1_RX0) & 0xffffffff;
				pDst += 4;
			}

			break;

		case SPI_16BIT:
			if (port == 0)
				outpw(REG_SPI0_TX0, 0xffff);
			else
				outpw(REG_SPI1_TX0, 0xffff);

			while (len-- > 0)
			{
				spiTxLen(port, 0, SPI_16BIT);
				spiActive(port);
				if (port == 0)
					*((INT16 *)pDst) = inpw(REG_SPI0_RX0) & 0xffff;
				else
					*((INT16 *)pDst) = inpw(REG_SPI1_RX0) & 0xffff;
				pDst += 2;
			}

			break;

		case SPI_8BIT:
			if (port == 0)
				outpw(REG_SPI0_TX0, 0xff);
			else
				outpw(REG_SPI1_TX0, 0xff);

			while (len-- > 0)
			{
				spiTxLen(port, 0, SPI_8BIT);
				spiActive(port);
				if (port == 0)
					*((INT8 *)pDst) = inpw(REG_SPI0_RX0) & 0xff;
				else
					*((INT8 *)pDst) = inpw(REG_SPI1_RX0) & 0xff;
				pDst ++;
			}

			break;
	}
	return 0;
}

INT spiWrite(INT port, INT TxBitLen, INT len, CHAR *pSrc)
{
	switch (TxBitLen)
	{
		case SPI_32BIT:
			while (len-- > 0)
			{
				if (port == 0)
					outpw(REG_SPI0_TX0, *((INT32 *)pSrc));
				else
					outpw(REG_SPI1_TX0, *((INT32 *)pSrc));
				spiTxLen(port, 0, SPI_32BIT);
				spiActive(port);
				pSrc += 4;
			}

			break;

		case SPI_16BIT:
			while (len-- > 0)
			{
				if (port == 0)
					outpw(REG_SPI0_TX0, *((INT16 *)pSrc));
				else
					outpw(REG_SPI1_TX0, *((INT16 *)pSrc));
				spiTxLen(port, 0, SPI_16BIT);
				spiActive(port);
				pSrc += 2;
			}

			break;

		case SPI_8BIT:
			while (len-- > 0)
			{
				if (port == 0)
					outpw(REG_SPI0_TX0, *((INT8 *)pSrc));
				else
					outpw(REG_SPI1_TX0, *((INT8 *)pSrc));
				spiTxLen(port, 0, SPI_8BIT);
				spiActive(port);
				pSrc ++;
			}

			break;
	}
	return 0;
}

/*-----------------------------------------------------------------------------------*/
INT spiSSEnable(UINT32 spiPort, UINT32 SSPin, UINT32 ClockMode)
{
	UINT reg;

#ifdef __FreeRTOS__
	if(spiPort == 0)	
#ifdef __RAK439__
		// flash wait for RAK439 idle
		if(SSPin == 0)
			FLASH_LOCK();
#else
		// wait the CS is idle
		while (xSemaphoreTake(xSpi0CsSemaphore, portMAX_DELAY) != pdPASS ) ;	
	else
		while (xSemaphoreTake(xSpi1CsSemaphore, portMAX_DELAY) != pdPASS ) ;
#endif
#endif
	reg = inpw(REG_SPI0_CNTRL+0x400*spiPort) & ~0x806;
	switch (ClockMode) {
	case 0:
		outpw(REG_SPI0_CNTRL+0x400*spiPort, reg | 0x004);
		break;
	case 1:
		outpw(REG_SPI0_CNTRL+0x400*spiPort, reg | 0x002);
		break;
	case 2:
		outpw(REG_SPI0_CNTRL+0x400*spiPort, reg | 0x802);
		break;
	case 3:
		outpw(REG_SPI0_CNTRL+0x400*spiPort, reg | 0x804);
	}
 
	outpw(REG_SPI0_SSR+0x400*spiPort, inpw(REG_SPI0_SSR+0x400*spiPort) | (0x01<<SSPin));	

	return Successful;
}

/*-----------------------------------------------------------------------------------*/
INT spiSSDisable(UINT32 spiPort, UINT32 SSPin)
{
	outpw(REG_SPI0_SSR+0x400*spiPort, inpw(REG_SPI0_SSR+0x400*spiPort) & ~(0x01<<SSPin));

#ifdef __FreeRTOS__	
	if (spiPort == 0)	
#ifdef __RAK439__	
		// free the mutext which is shared with RAK439 lib
		if(SSPin == 0)
			FLASH_UNLOCK();
#else	
		// free the CS
		xSemaphoreGive(xSpi0CsSemaphore);	
	else
		xSemaphoreGive(xSpi1CsSemaphore);
#endif
#endif

	return Successful;
}

/*-----------------------------------------------------------------------------------*/
INT spiTransfer(UINT32 port, UINT32 TxBitLen, UINT32 len, PUINT8 RxBuf , PUINT8 TxBuf)
{
	UINT i, transfer_len, u32TxNum;
	PUINT pU32Rx, pU32Tx;
	PUINT16 pU16Rx, pU16Tx;
	PUINT8 pU8Rx, pU8Tx;

	//sysprintf("len=%d\n", len);		
	switch (TxBitLen)
	{
		case SPI_32BIT:
			pU32Rx = (PUINT)RxBuf;
			pU32Tx = (PUINT)TxBuf;
			
			transfer_len = len / 4;
			if ( transfer_len )
			{					
				outpw(REG_SPI0_CNTRL+0x400*port, inpw(REG_SPI0_CNTRL+0x400*port) |BYTE_ENDIN);
				
				u32TxNum = transfer_len%4;
				if ( !u32TxNum )
					u32TxNum = 4;

				outpw(REG_SPI0_CNTRL+0x400*port, (inpw(REG_SPI0_CNTRL+0x400*port) & ~(Tx_NUM | Tx_BIT_LEN)) |((u32TxNum-1) << 8));

				for ( i=0; i<u32TxNum  ; i++ )
				{
					if (pU32Tx != NULL)
						outpw((REG_SPI0_TX0+0x400*port) + (i<<2), *pU32Tx++);							
					//else
						//outpw((REG_SPI0_TX0+0x400*port) + (i<<2), 0xffffffff);							
				}			
				spiActive(port);
				
				if (pU32Rx != NULL) 
				for ( i=0; i< u32TxNum ; i++ )
					*pU32Rx++ = inpw((REG_SPI0_RX0+0x400*port) + (i<<2));

				transfer_len -= u32TxNum;

				u32TxNum = 4;
				outpw(REG_SPI0_CNTRL+0x400*port, (inpw(REG_SPI0_CNTRL+0x400*port) & ~(Tx_NUM | Tx_BIT_LEN)) |((u32TxNum-1) << 8));
				
				while ( transfer_len > 0 )
				{														
					for ( i=0; i<u32TxNum  ; i++ )
					{
						if (pU32Tx != NULL)
							outpw((REG_SPI0_TX0+0x400*port) + (i<<2), *pU32Tx++);							
						//else
							//outpw((REG_SPI0_TX0+0x400*port) + (i<<2), 0xffffffff);							
					}				
					spiActive(port);
					
					if (pU32Rx != NULL) 
					for ( i=0; i< u32TxNum ; i++ )
						*pU32Rx++ = inpw((REG_SPI0_RX0+0x400*port) + (i<<2));

					transfer_len -= u32TxNum;											
				}
			
				outpw(REG_SPI0_CNTRL+0x400*port, inpw(REG_SPI0_CNTRL+0x400*port) & ~BYTE_ENDIN);
				
			}

			transfer_len = len % 4;
			if ( transfer_len )
			{
				pU8Rx = (PUINT8)pU32Rx;
				pU8Tx = (PUINT8)pU32Tx;

				outpw(REG_SPI0_CNTRL+0x400*port, (inpw(REG_SPI0_CNTRL+0x400*port) & ~(Tx_NUM | Tx_BIT_LEN)) |(0x08 << 3));

				for ( i=0; i<transfer_len; i++ )
				{
					if (pU8Tx != NULL) 
						outpw(REG_SPI0_TX0+0x400*port, pU8Tx[i]);						
					//else
						//outpw(REG_SPI0_TX0+0x400*port, 0xff);
					
					spiActive(port);
					if (pU8Rx != NULL) 
						pU8Rx[i] = inpw(REG_SPI0_RX0+0x400*port)&0xFF ;
				}
			}
			break;

		case SPI_16BIT:
			pU16Rx = (PUINT16)RxBuf;
			pU16Tx = (PUINT16)TxBuf;
			
			transfer_len = len / 2;
			if ( transfer_len )
			{
				outpw(REG_SPI0_CNTRL+0x400*port, inpw(REG_SPI0_CNTRL+0x400*port) |BYTE_ENDIN);

				u32TxNum = transfer_len%4;
				if ( !u32TxNum )
					u32TxNum = 4;

				outpw(REG_SPI0_CNTRL+0x400*port, (inpw(REG_SPI0_CNTRL+0x400*port) & ~(Tx_NUM | Tx_BIT_LEN)) |(((u32TxNum-1) << 8) |(16<<0x03)));

				for ( i=0; i<u32TxNum  ; i++ )
				{
					if (pU16Tx != NULL)
						outpw((REG_SPI0_TX0+0x400*port) + (i<<2), *pU16Tx++);							
					//else
						//outpw((REG_SPI0_TX0+0x400*port) + (i<<2), 0xffff);							
				}
				spiActive(port);
				
				if (pU16Rx != NULL) 
				for ( i=0; i< u32TxNum ; i++ )
					*pU16Rx++ = inpw((REG_SPI0_RX0+0x400*port) + (i<<2))&0xFFFF;

				transfer_len -= u32TxNum;

				u32TxNum = 4;
				outpw(REG_SPI0_CNTRL+0x400*port, (inpw(REG_SPI0_CNTRL+0x400*port) & ~(Tx_NUM | Tx_BIT_LEN)) |(((u32TxNum-1) << 8) |(16<<0x03)));
				
				while ( transfer_len > 0 )
				{						
					for ( i=0; i<u32TxNum  ; i++ )
					{
						if (pU16Tx != NULL)
							outpw((REG_SPI0_TX0+0x400*port) + (i<<2), *pU16Tx++);							
						//else
							//outpw((REG_SPI0_TX0+0x400*port) + (i<<2), 0xffff);							
					}
					spiActive(port);
					
					if (pU16Rx != NULL) 
					for ( i=0; i< u32TxNum ; i++ )
						*pU16Rx++ = inpw((REG_SPI0_RX0+0x400*port) + (i<<2))&0xFFFF;

					transfer_len -= u32TxNum;
				}

				outpw(REG_SPI0_CNTRL+0x400*port, inpw(REG_SPI0_CNTRL+0x400*port) & ~BYTE_ENDIN);
			}

			transfer_len = len % 2;
			if ( transfer_len )
			{
				pU8Rx = (PUINT8)pU16Rx;
				pU8Tx = (PUINT8)pU16Tx;

				outpw(REG_SPI0_CNTRL+0x400*port, (inpw(REG_SPI0_CNTRL+0x400*port) & ~(Tx_NUM | Tx_BIT_LEN)) |(0x08 << 3));

				for ( i=0; i<transfer_len; i++ )
				{
					if (pU8Tx != NULL) 
						outpw(REG_SPI0_TX0+0x400*port, pU8Tx[i]);						
					//else
						//outpw(REG_SPI0_TX0+0x400*port, 0xff);
					
					spiActive(port);
					if (pU8Rx != NULL) 
						pU8Rx[i] = inpw(REG_SPI0_RX0+0x400*port)&0xFF ;
				}
			}
			break;

		case SPI_8BIT:
			pU8Rx = RxBuf;
			pU8Tx = TxBuf;
			
			transfer_len = len;		

			u32TxNum = transfer_len%4;
			if ( !u32TxNum )
				u32TxNum = 4;

			outpw(REG_SPI0_CNTRL+0x400*port, (inpw(REG_SPI0_CNTRL+0x400*port) & ~(Tx_NUM | Tx_BIT_LEN)) |(((u32TxNum-1) << 8) |(8<<0x03)));

			if (pU8Tx != NULL)
			for ( i=0; i<u32TxNum  ; i++ )
				outpw((REG_SPI0_TX0+0x400*port) + (i<<2), *pU8Tx++);

			spiActive(port);				
			
			if (pU8Rx != NULL) 
			for ( i=0; i< u32TxNum ; i++ )
				*pU8Rx++ = inpw((REG_SPI0_RX0+0x400*port) + (i<<2))&0xFF;

			transfer_len -= u32TxNum;

			u32TxNum = 4;
			outpw(REG_SPI0_CNTRL+0x400*port, (inpw(REG_SPI0_CNTRL+0x400*port) & ~(Tx_NUM)) |((u32TxNum-1) << 8));
										
			while ( transfer_len > 0 )
			{					
				if (pU8Tx != NULL)
				for ( i=0; i<u32TxNum  ; i++ )
					outpw((REG_SPI0_TX0+0x400*port) + (i<<2), *pU8Tx++);
				
				spiActive(port);	
				
				if (pU8Rx != NULL) 
				for ( i=0; i< u32TxNum ; i++ )
					*pU8Rx++ = inpw((REG_SPI0_RX0+0x400*port) + (i<<2))&0xFF;

				transfer_len -= u32TxNum;
			}				
			break;
	}		
		
	return Successful;
}

/*-----------------------------------------------------------------------------------*/
INT spiRtosInit(INT32 spiPort)
{
#ifdef __FreeRTOS__
	// create the semaphore used for SPI two CS
	if (spiPort == 0)
		if (Spi0CsInit == 0) {
			xSpi0CsSemaphore = xSemaphoreCreateBinary();
			xSemaphoreGive(xSpi0CsSemaphore);
			Spi0CsInit = 1;
		}
	else
		if (Spi1CsInit == 0) {
			xSpi1CsSemaphore = xSemaphoreCreateBinary();
			xSemaphoreGive(xSpi1CsSemaphore);
			Spi1CsInit = 1;
		}
#endif
	return Successful;
}

