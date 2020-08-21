/**************************************************************************//**
 * @file     N9H20_edma.c
 * @version  V3.00
 * @brief    N9H20 series EDMA driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "wblib.h"
#include "N9H20_EDMA.h"

#define EDMA_USE_IRQ

//#define DBG_PRINTF		sysprintf
#define DBG_PRINTF(...)

struct EDMA_CHANNEL_INFO{
	volatile int in_request;
	S_DRVEDMA_DESCRIPT_FORMAT *sg;
	unsigned int resbytes;
	void *data;
	int dma_num;
	volatile int in_use;
};

static struct EDMA_CHANNEL_INFO edma_channels_info[MAX_CHANNEL_NUM+1];

struct EDMA_DIRECTION{
	int src_dir;
	int dest_dir;
};

static struct EDMA_DIRECTION edma_set_dir[MAX_CHANNEL_NUM+1];

static BOOL bIsEDMAInit = FALSE;

/**
 * EDMA_SetupCST - setup EDMA channel for color space transform
 *
 * @eSrcFormat: the source color format
 * @eDestFormat: the destination color format
 *
 * Return value: if incorrect parameters are provided EDMA_ERR_BUSY.
 *		Zero indicates success.
 */
int
EDMA_SetupCST(int channel, E_DRVEDMA_COLOR_FORMAT eSrcFormat, E_DRVEDMA_COLOR_FORMAT eDestFormat)
{
	struct EDMA_CHANNEL_INFO *edma_info = &edma_channels_info[channel];

	if (edma_info->in_use)
		return EDMA_ERR_BUSY;

	DrvEDMA_SetColorTransformFormat((E_DRVEDMA_CHANNEL_INDEX)channel, eSrcFormat, eDestFormat);
	DrvEDMA_SetColorTransformOperation((E_DRVEDMA_CHANNEL_INDEX)channel, eDRVEDMA_ENABLE, eDRVEDMA_DISABLE);

	return 0;
}

/**
 * EDMA_ClearCST - clear EDMA channel for color space transform
 *
 * Return value: if incorrect parameters are provided EDMA_ERR_BUSY.
 *		Zero indicates success.
 */
int
EDMA_ClearCST(int channel)
{
	//struct EDMA_CHANNEL_INFO *edma_info = &edma_channels_info[channel];

	//if (edma_info->in_use)
		//return EDMA_ERR_BUSY;

	DrvEDMA_SetColorTransformOperation((E_DRVEDMA_CHANNEL_INDEX)channel, eDRVEDMA_DISABLE, eDRVEDMA_DISABLE);

	return 0;
}

/**
 * EDMA_SetupSingle - setup EDMA channel for linear memory to/from
 * device transfer
 *
 * @channel: EDMA channel number
 * @src_addr: the source EDMA memory address of the linear data block
 * @dest_addr: the destination EDMA memory address of the linear data block
 * @dma_length: length of the data block in bytes
 *
 * Return value: if incorrect parameters are provided EDMA_ERR_INVAL.
 *		Zero indicates success.
 */
int
EDMA_SetupSingle(int channel, unsigned int src_addr, unsigned int dest_addr,
			  unsigned int dma_length)
{
	struct EDMA_CHANNEL_INFO *edma_info = &edma_channels_info[channel];
	S_DRVEDMA_CH_ADDR_SETTING sSrcAddr, sDestAddr;

	edma_info->sg = NULL;

	if (edma_info->in_use)
		return EDMA_ERR_BUSY;

	if ((!src_addr)||(!dest_addr)) {
		DBG_PRINTF("edma%d: EDMA_SetupSingle null address\n", 
			channel);
		return EDMA_ERR_INVAL;
	}

	if (!dma_length) {
		DBG_PRINTF("edma%d: EDMA_SetupSingle zero length\n",
		       channel);
		return EDMA_ERR_INVAL;
	}

	sSrcAddr.u32Addr = src_addr;
	sSrcAddr.eAddrDirection = eDRVEDMA_DIRECTION_INCREMENTED;
	sDestAddr.u32Addr = dest_addr;
	sDestAddr.eAddrDirection = eDRVEDMA_DIRECTION_INCREMENTED;
	
	if (channel != 0)
	{
		if (edma_set_dir[channel].src_dir != -1)
			sSrcAddr.eAddrDirection = (E_DRVEDMA_DIRECTION_SELECT)(edma_set_dir[channel].src_dir);		
		if (edma_set_dir[channel].dest_dir != -1)			
			sDestAddr.eAddrDirection = (E_DRVEDMA_DIRECTION_SELECT)(edma_set_dir[channel].dest_dir);		
	}
	

	DrvEDMA_SetTransferSetting((E_DRVEDMA_CHANNEL_INDEX)channel, &sSrcAddr, &sDestAddr, dma_length);

	return 0;
}

/**
 * EDMA_SetupSG - setup EDMA channel SG list for source address
 * @length: total length of the transfer request in bytes
 * @src_addr: source address
 * @dest_addr: destination address
 *
 * The function sets up EDMA channel state and registers to be ready for
 * transfer specified by provided parameters. The scatter-gather emulation
 * is set up according to the parameters.
 *
 * The full preparation of the transfer requires setup of more register
 * by the caller before EDMA_Enable() can be called.
 *
 * Return value: if incorrect parameters are provided EDMA_ERR_INVAL.
 * Zero indicates success.
 */
int
EDMA_SetupSG(int channel, unsigned int src_addr, unsigned int dest_addr,
				unsigned int dma_length)
{
	struct EDMA_CHANNEL_INFO *edma_info = &edma_channels_info[channel];
	unsigned int sgcount;
	S_DRVEDMA_DESCRIPT_FORMAT *psSGFmt;
	unsigned int u32Value, u32SFR, u32TranferByte;	 
	unsigned int u32SrcAddr, u32DestAddr;

	if (edma_info->in_use)
		return EDMA_ERR_BUSY;

	if (dma_length <= 0) {
		DBG_PRINTF("EDMA_SetupSG zero length\n");
		return EDMA_ERR_INVAL;
	}

	if ((src_addr & 0x0FFF) || (src_addr & 0x0FFF)) {
		DBG_PRINTF("EDMA_SetupSG address is not PAGE_SIZE alignment\n");
		return EDMA_ERR_INVAL;
	}

	sgcount = (dma_length + PAGE_SIZE - 1) / PAGE_SIZE;
	edma_info->sg = (VOID *)((UINT32)malloc(sgcount * sizeof(S_DRVEDMA_DESCRIPT_FORMAT)) | NON_CACHE_BIT);
	edma_info->resbytes = dma_length;

	// Set channel 0 transfer address and Scatter-Gather
	u32SFR = REG_VDMA_CSR + channel * 0x100;
	u32Value = inp32(u32SFR); 
	u32Value = (u32Value & ~SAD_SEL) | (eDRVEDMA_DIRECTION_INCREMENTED << SOURCE_DIRECTION_BIT);
	u32Value = (u32Value & ~DAD_SEL) | (eDRVEDMA_DIRECTION_INCREMENTED << DESTINATION_DIRECTION_BIT);
	outp32(u32SFR, u32Value); 
	DrvEDMA_EnableScatterGather((E_DRVEDMA_CHANNEL_INDEX)channel);
	DrvEDMA_SetScatterGatherTblStartAddr((E_DRVEDMA_CHANNEL_INDEX)channel, (UINT32)edma_info->sg);

	psSGFmt = edma_info->sg;
	u32SrcAddr = src_addr;
	u32DestAddr = dest_addr;
	u32TranferByte = 0;
	
	do {
		u32TranferByte = (edma_info->resbytes >= PAGE_SIZE) ? PAGE_SIZE : edma_info->resbytes;
		edma_info->resbytes -= u32TranferByte;

		// set source and destination address
		psSGFmt->u32SourceAddr = u32SrcAddr;
		psSGFmt->u32DestAddr = u32DestAddr;
		// set stride transfer byte count & byte count
		psSGFmt->u32StrideAndByteCount = u32TranferByte;
		// set source offset byte length and destination offset byte length
		psSGFmt->u32Offset = 0;
		// set EOT for last descript format  	   						  
		if (edma_info->resbytes == 0)
			psSGFmt->u32Offset |= 0x80000000;
		// set next Scatter-Gather table address
		//psSGFmt->u32NextSGTblAddr = (unsigned int)(psSGFmt+1); 
		psSGFmt++;
		
		u32SrcAddr += u32TranferByte;
		u32DestAddr += u32TranferByte;
	} while (edma_info->resbytes > 0);
	
	return 0;
}

/**
 * EDMA_FreeSG - release previously acquired channel
 * @channel: EDMA channel number
 */
void EDMA_FreeSG(int channel)
{
	struct EDMA_CHANNEL_INFO *edma_info = &edma_channels_info[channel];
	S_DRVEDMA_DESCRIPT_FORMAT *psSGFmt;
	S_DRVEDMA_DESCRIPT_FORMAT *psSGFree;

	if (!edma_info->in_request) {
		DBG_PRINTF("trying to free free channel %d\n",
		       channel);
		return;
	}

	/* Free Scatter-Gather table */
	psSGFmt = edma_info->sg;
	while (psSGFmt) {
		psSGFree = psSGFmt;
		if (psSGFmt->u32Offset & (1<<15))
			psSGFmt = (S_DRVEDMA_DESCRIPT_FORMAT *)psSGFmt->u32NextSGTblAddr;
		else
			psSGFmt = NULL;
		free(psSGFree);
	}
}

/**
 * EDMA_SetupHandlers - setup EDMA channel notification handlers
 * @channel: EDMA channel number
 * @interrupt: EDMA interrupt enable
 * @irq_handler: the pointer to the function called if the transfer
 *		ends successfully
 * @data: user specified value to be passed to the handlers
 */
int
EDMA_SetupHandlers(int channel, int interrupt,		       
		       PFN_DRVEDMA_CALLBACK irq_handler,
		       void *data)
{
	struct EDMA_CHANNEL_INFO *edma_info = &edma_channels_info[channel];	
	int ret;

	if (!edma_info->in_request) {
		DBG_PRINTF("called for not allocated channel %d\n",
		       channel);
		return EDMA_ERR_NODEV;
	}	

//	__raw_writel(1 << channel, EDMA_DISR);
	ret = DrvEDMA_InstallCallBack((E_DRVEDMA_CHANNEL_INDEX)channel, (E_DRVEDMA_INT_ENABLE)interrupt, (PFN_DRVEDMA_CALLBACK)irq_handler, (PFN_DRVEDMA_CALLBACK *)data);	

	return ret;
}

/**
 * EDMA_Enable - function to start EDMA channel operation
 * @channel: EDMA channel number
 *
 * The channel has to be allocated by driver through EDMA_Request()
 * or PDMA_FindandRequest() function.
 * The transfer parameters has to be set to the channel registers through
 * call of the EDMA_SetupSingle() or EDMA_SetupSG() function
 */
void EDMA_Enable(int channel)
{
	struct EDMA_CHANNEL_INFO *edma_info = &edma_channels_info[channel];	

	if (!edma_info->in_request) {
		DBG_PRINTF("called for not allocated channel %d\n",
		       channel);
		return;
	}	

	DrvEDMA_EnableCH((E_DRVEDMA_CHANNEL_INDEX)channel, eDRVEDMA_ENABLE);
#ifdef EDMA_USE_IRQ
	DrvEDMA_EnableInt((E_DRVEDMA_CHANNEL_INDEX)channel, (E_DRVEDMA_INT_ENABLE)(eDRVEDMA_SG | eDRVEDMA_BLKD | eDRVEDMA_TABORT));
#endif
	
}

/**
 * EDMA_Disable - stop, finish EDMA channel operatin
 * @channel: EDMA channel number
 */
void EDMA_Disable(int channel)
{	
	DBG_PRINTF("edma%d: EDMA_Disable\n", channel);
	
#ifdef EDMA_USE_IRQ
	DrvEDMA_DisableInt((E_DRVEDMA_CHANNEL_INDEX)channel, (E_DRVEDMA_INT_ENABLE)(eDRVEDMA_SG | eDRVEDMA_BLKD | eDRVEDMA_TABORT));
#endif
	DrvEDMA_EnableCH((E_DRVEDMA_CHANNEL_INDEX)channel, eDRVEDMA_DISABLE);
	
}

/**
 * EDMA_Request - request/allocate specified channel number
 * @channel: EDMA channel number
  */
int EDMA_Request(int channel)
{
	struct EDMA_CHANNEL_INFO *edma_info = &edma_channels_info[channel];	
	int ret = 0;

	sysSetLocalInterrupt(DISABLE_IRQ);
	
	if (edma_info->in_use)
		return EDMA_ERR_BUSY;	

	if (channel > MAX_CHANNEL_NUM) {
		DBG_PRINTF("called for non-existed channel %d\n",
		       channel);
		return EDMA_ERR_INVAL;
	}	

	if (edma_info->in_request) 	    		
		return EDMA_ERR_BUSY;
	
	memset((void*)edma_info, 0, sizeof(edma_info));
	edma_info->in_request = 1;	
	
	DrvEDMA_EnableCH((E_DRVEDMA_CHANNEL_INDEX)channel, eDRVEDMA_ENABLE);
#ifdef EDMA_USE_IRQ
	DrvEDMA_EnableInt((E_DRVEDMA_CHANNEL_INDEX)channel, (E_DRVEDMA_INT_ENABLE)(eDRVEDMA_SG | eDRVEDMA_BLKD | eDRVEDMA_TABORT));
#endif

	sysSetLocalInterrupt(ENABLE_IRQ);

	return ret;
}

/**
 * EDMA_Free - release previously acquired channel
 * @channel: EDMA channel number
 */
void EDMA_Free(int channel)
{	
	struct EDMA_CHANNEL_INFO *edma_info = &edma_channels_info[channel];
	unsigned int regval;

	if (!edma_info->in_request) {
		DBG_PRINTF("trying to free channel %d\n",
		       channel);
		return;
	}
	
	regval = inp32(REG_VDMA_CSR + channel * 0x100);
	// Reset channel if source for destination in wrap-around mode
	if (((regval & 0xC0) == 0xC0) || ((regval & 0x30) == 0x30)) 
	{
		regval = regval & ~0xF0;
		outp32(REG_VDMA_CSR + channel * 0x100, regval | 0x02);
		outp32(REG_VDMA_ISR + channel * 0x100, 0xF00);
	}

#ifdef EDMA_USE_IRQ
	DrvEDMA_DisableInt((E_DRVEDMA_CHANNEL_INDEX)channel, (E_DRVEDMA_INT_ENABLE)(eDRVEDMA_SG | eDRVEDMA_BLKD | eDRVEDMA_TABORT | eDRVEDMA_WAR));
#endif
	DrvEDMA_EnableCH((E_DRVEDMA_CHANNEL_INDEX)channel, eDRVEDMA_DISABLE);

	if (edma_info->in_use)
		edma_info->in_use = 0;
	if (edma_info->sg)
		EDMA_FreeSG(channel);

	edma_info->in_request = 0;
	
}

/**
 * VDMA_FindandRequest - find and request some of free channels
  *
 * This function tries to find a free channel in the specified priority group
 *
 * Return value: If there is no free channel to allocate, EDMA_ERR_NODEV is returned.
 *               On successful allocation channel is returned.
 */
int VDMA_FindandRequest(void)
{
	if (!EDMA_Request(0))
			return 0;		

	DBG_PRINTF("no free VDMA channel found\n");

	return EDMA_ERR_NODEV;
}

/**
 * PDMA_FindandRequest - find and request some of free channels
 *
 * This function tries to find a free channel in the specified priority group
 *
 * Return value: If there is no free channel to allocate, EDMA_ERR_NODEV is returned.
 *               On successful allocation channel is returned.
 */
int PDMA_FindandRequest(void)
{
	int i;

	for (i = 1; i <= MAX_CHANNEL_NUM; i++)
		if (!EDMA_Request(i))
			return i;

	DBG_PRINTF("no free PDMA channel found\n");

	return EDMA_ERR_NODEV;
}

/**
 * EDMA_Trigger - function to start EDMA channel transfer
 * @channel: EDMA channel number
 */
void EDMA_Trigger(int channel)
{	
	struct EDMA_CHANNEL_INFO *edma_info = &edma_channels_info[channel];

	if (edma_info->in_use)
		return;

	edma_info->in_use = 1;
	DrvEDMA_CHEnablelTransfer((E_DRVEDMA_CHANNEL_INDEX)channel);
}

/**
 * EDMA_TriggerDone - function to set EDMA channel transfer done
 * @channel: EDMA channel number
 */
void EDMA_TriggerDone(int channel)
{	
	struct EDMA_CHANNEL_INFO *edma_info = &edma_channels_info[channel];

	DBG_PRINTF("edma%d: EDMA_TriggerDone\n", channel);

	edma_info->in_use = 0;
}

/**
 * EDMA_IsBusy - function to query EDMA channel is busy or not
 * @channel: EDMA channel number
 */
int EDMA_IsBusy(int channel)
{
	DBG_PRINTF("edma%d: EDMA_IsBusy\n", channel);

	return DrvEDMA_IsCHBusy((E_DRVEDMA_CHANNEL_INDEX)channel);
}

int EDMA_Init(void)
{	
	int i;

	DBG_PRINTF("EDMA_Init\n");	

	if(bIsEDMAInit == FALSE)
	{
		bIsEDMAInit = TRUE;

		// EDMA open
		DrvEDMA_Open();

		for (i = 0; i <= MAX_CHANNEL_NUM; i++) {
			edma_channels_info[i].in_request = 0;
			edma_channels_info[i].sg = NULL;
			edma_channels_info[i].dma_num = i;
			
			edma_set_dir[i].src_dir = -1;		
			edma_set_dir[i].dest_dir = -1;					
		}
	}

	return 0;
}


void EDMA_Exit(void)
{
	DrvEDMA_Close();
}

/**
 */
int EDMA_SetAPB(int channel, E_DRVEDMA_APB_DEVICE eDevice, E_DRVEDMA_APB_RW eRWAPB, E_DRVEDMA_TRANSFER_WIDTH eTransferWidth)
{
	struct EDMA_CHANNEL_INFO *edma_info = &edma_channels_info[channel];

	if (edma_info->in_use)
		return EDMA_ERR_BUSY;	

	DBG_PRINTF("EDMA_SetAPB ch:%d: device =%d, width:%d: read/write=%d\n", channel,eDevice, eTransferWidth,eRWAPB);
	
	DrvEDMA_SetAPBTransferWidth((E_DRVEDMA_CHANNEL_INDEX)channel, eTransferWidth);
	DrvEDMA_SetCHForAPBDevice((E_DRVEDMA_CHANNEL_INDEX)channel, eDevice, eRWAPB);

	return 0;
}

int EDMA_SetWrapINTType(int channel, int type)
{
	struct EDMA_CHANNEL_INFO *edma_info = &edma_channels_info[channel];

	if (edma_info->in_use)
		return EDMA_ERR_BUSY;	

	DBG_PRINTF("EDMA_SetWrapINTType ch:%d: WrapIntType:%d:\n", channel,type);
	
	DrvEDMA_SetWrapIntType((E_DRVEDMA_CHANNEL_INDEX)channel, (E_DRVEDMA_WRAPAROUND_SELECT)type);
	if (type !=0)
	{
		DrvEDMA_DisableInt((E_DRVEDMA_CHANNEL_INDEX)channel,(E_DRVEDMA_INT_ENABLE)(eDRVEDMA_SG | eDRVEDMA_BLKD | eDRVEDMA_TABORT));
		DrvEDMA_EnableInt((E_DRVEDMA_CHANNEL_INDEX)channel, eDRVEDMA_WAR);
	}
	else
	{
		DrvEDMA_DisableInt((E_DRVEDMA_CHANNEL_INDEX)channel,eDRVEDMA_WAR);
		DrvEDMA_EnableInt((E_DRVEDMA_CHANNEL_INDEX)channel, (E_DRVEDMA_INT_ENABLE)(eDRVEDMA_SG | eDRVEDMA_BLKD | eDRVEDMA_TABORT));		
	}
		
	return 0;
}

int EDMA_SetDirection(int channel, int src_dir, int dest_dir)
{
	struct EDMA_CHANNEL_INFO *edma_info = &edma_channels_info[channel];

	if (edma_info->in_use)
		return EDMA_ERR_BUSY;	

	DBG_PRINTF("EDMA_SetTransferDirection ch:%d: Src Dir:%d, Dest Dir:%d\n", channel,src_dir, dest_dir);
	
	if ((channel > 0) && (channel <=MAX_CHANNEL_NUM))
	{
		edma_set_dir[channel].src_dir = src_dir;
		edma_set_dir[channel].dest_dir = dest_dir;	
	}

	return 0;
}



