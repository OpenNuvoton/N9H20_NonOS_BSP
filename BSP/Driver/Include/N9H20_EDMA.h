/**************************************************************************//**
 * @file     N9H20_EDMA.h
 * @version  V3.00
 * @brief    N9H20 series EDMA driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

 #include "wblib.h"
 #include "wbtypes.h"
 #include "DrvEDMA.h"

#define PAGE_SIZE 0x1000

#define EDMA_ERR_NOERROR			(0x00)
#define EDMA_ERR_NODEV				(0x01 | GDMA_ERR_ID)
#define EDMA_ERR_INVAL				(0x02 | GDMA_ERR_ID)
#define EDMA_ERR_BUSY				(0x03 | GDMA_ERR_ID)


#define NON_CACHE_BIT		0x80000000

int
EDMA_SetupCST(
	int channel, 
	E_DRVEDMA_COLOR_FORMAT eSrcFormat, 
	E_DRVEDMA_COLOR_FORMAT eDestFormat
);

int
EDMA_ClearCST(
	int channel
);

int
EDMA_SetupSingle(
	int channel, 
	unsigned int src_addr, 
	unsigned int dest_addr,
	unsigned int dma_length
);

int
EDMA_SetupSG(
	int channel, 
	unsigned int src_addr, 
	unsigned int dest_addr,
	unsigned int dma_length
);

void 
EDMA_FreeSG(
	int channel
);

int
EDMA_SetupHandlers(
	int channel, 
	int interrupt,	
	PFN_DRVEDMA_CALLBACK irq_handler,
	void *data
);

void 
EDMA_Enable(
	int channel
);

void 
EDMA_Disable(
	int channel
);

int 
EDMA_Request(
	int channel
);

void 
EDMA_Free(
	int channel
);

int 
VDMA_FindandRequest(void);

int 
PDMA_FindandRequest(void);

void 
EDMA_Trigger(
	int channel
);

void 
EDMA_TriggerDone(
	int channel
);

int 
EDMA_IsBusy(
	int channel
);

int 
EDMA_Init(void);

void 
EDMA_Exit(void);

int 
EDMA_SetAPB(
	int channel, 
	E_DRVEDMA_APB_DEVICE eDevice, 
	E_DRVEDMA_APB_RW eRWAPB, 
	E_DRVEDMA_TRANSFER_WIDTH eTransferWidth
);

int 
EDMA_SetWrapINTType(
	int channel, 
	int type
);

int 
EDMA_SetDirection(
	int channel, 
	int src_dir, 
	int dest_dir
);



