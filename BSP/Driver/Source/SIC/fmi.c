/**************************************************************************//**
 * @file     fmi.c
 * @version  V3.00
 * @brief    N9H20 series SIC driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#ifdef ECOS
	#include "drv_api.h"
	#include "diag.h"
	#include "wbtypes.h"
	#include "wbio.h"
	#define IRQ_SD   16
#else
    #define IRQ_SD  IRQ_SIC
	#include "wblib.h"
#endif

#include "N9H20_SIC.h"
#include "fmi.h"
#include "N9H20_NVTFAT.h"

#ifdef ECOS
cyg_interrupt  /* dmac_interrupt, */ fmi_interrupt;
cyg_handle_t   /* dmac_interrupt_handle, */ fmi_interrupt_handle;
#endif

#if defined (__GNUC__)
    #ifdef NULL
        #undef NULL
    #endif
    #define NULL ((void *)0)
#endif

// global variable
UINT32 _fmi_uFMIReferenceClock;
BOOL volatile _fmi_bIsSDDataReady=FALSE, _fmi_bIsSMDataReady=FALSE;

typedef void (*fmi_pvFunPtr)();   /* function pointer */
void (*fmiSD0RemoveFun)() = NULL;
void (*fmiSD0InsertFun)() = NULL;

#ifdef _SIC_USE_INT_

extern PDISK_T *pDisk_SD0;
//extern PDISK_T *pDisk_SD1;
//extern PDISK_T *pDisk_SD2;

#ifdef ECOS
static cyg_uint32 fmiIntHandler(cyg_vector_t vector, cyg_addrword_t data)
#else
VOID fmiIntHandler()
#endif
{
	unsigned int volatile isr;

	// FMI data abort interrupt
	if (inpw(REG_FMIISR) & FMI_DAT_IF)
	{
		/* fmiResetAllEngine() */
		outpw(REG_FMICR, inpw(REG_FMICR) | FMI_SWRST);
		outpw(REG_FMIISR, FMI_DAT_IF);
	}

	// SD interrupt status
	isr = inpw(REG_SDISR);
	if (isr & SDISR_BLKD_IF)		// block down
	{
		_fmi_bIsSDDataReady = TRUE;
		outpw(REG_SDISR, SDISR_BLKD_IF);
	}

	if (isr & SDISR_CD_IF)	// port 0 card detect
	{
		if (inpw(REG_SDISR) & SDISR_CD_Card)
		{
			pSD0->bIsCardInsert = FALSE;
			if (fmiSD0RemoveFun != NULL)
				(*fmiSD0RemoveFun)(pDisk_SD0);
		}
		else
		{
			pSD0->bIsCardInsert = TRUE;
			if (fmiSD0InsertFun != NULL)
				(*fmiSD0InsertFun)();
		}
		outpw(REG_SDISR, SDISR_CD_IF);
	}

	// SM interrupt status
	isr = inpw(REG_SMISR);
	if (isr & SMISR_DMA_IF)
	{
		_fmi_bIsSMDataReady = TRUE;
		outpw(REG_SMISR, SMISR_DMA_IF);
	}

#ifdef ECOS
	cyg_drv_interrupt_acknowledge(IRQ_SD);
	return CYG_ISR_HANDLED;
#endif
}
#endif	//_SIC_USE_INT_

VOID fmiSetCallBack(UINT32 uCard, PVOID pvRemove, PVOID pvInsert)
{
	switch (uCard)
	{
		case FMI_SD_CARD:
			fmiSD0RemoveFun = (fmi_pvFunPtr)pvRemove;
			fmiSD0InsertFun = (fmi_pvFunPtr)pvInsert;
			break;
	}
}


VOID fmiInitDevice()
{
	// Enable SD Card Host Controller operation and driving clock.
	outpw(REG_AHBCLK, inpw(REG_AHBCLK) | HCLK3_CKE | SIC_CKE | SD_CKE | NAND_CKE);

#ifdef _SIC_USE_INT_
	/* Install ISR */
#ifdef ECOS
	cyg_drv_interrupt_create(IRQ_SD, 10, 0, fmiIntHandler, NULL,
								&fmi_interrupt_handle, &fmi_interrupt);
    cyg_drv_interrupt_attach(fmi_interrupt_handle);
#else
    sysInstallISR(IRQ_LEVEL_1, IRQ_SD, (PVOID)fmiIntHandler);
#endif

#ifndef ECOS
    /* enable CPSR I bit */
    sysSetLocalInterrupt(ENABLE_IRQ);
#endif
#endif	//_SIC_USE_INT_

	// DMAC Initial
	outpw(REG_DMACCSR, DMAC_SWRST | DMAC_EN);
	outpw(REG_DMACCSR, DMAC_EN);

#ifdef _SIC_USE_INT_
#ifdef ECOS
    cyg_drv_interrupt_unmask(IRQ_SD);
#else
	sysEnableInterrupt(IRQ_SD);
#endif
#endif	//_SIC_USE_INT_

	outpw(REG_FMICR, FMI_SWRST);		// reset FMI engine
	while(inpw(REG_FMICR)&FMI_SWRST);
}

VOID fmiSetFMIReferenceClock(UINT32 uClock)
{
	_fmi_uFMIReferenceClock = uClock;	// kHz
}
