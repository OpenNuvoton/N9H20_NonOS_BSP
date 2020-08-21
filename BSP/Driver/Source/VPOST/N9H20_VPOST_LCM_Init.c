/**************************************************************************//**
 * @file     N9H20_VPOST_LCM_Init.c
 * @brief    Panel driver file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#include <string.h>
#include "wblib.h"
#include "N9H20_reg.h"
#include "N9H20_VPOST.h"

volatile S_DRVVPOST_LCM_WINDOW *psLcmWindow;
volatile S_DRVVPOST_SYNCLCM_INIT sLcmInit, *psLcmInit;
volatile S_DRVVPOST_SYNCLCM_HTIMING sHTiming, *psHTiming;
volatile S_DRVVPOST_LCM_REG sLcmReg[OPT_MAXIMUM_LCM_REG], *psLcmReg;

volatile S_DRVVPOST_MPULCM_INIT sMpuLcmInit, *psMpuLcmInit;
volatile E_DRVVPOST_MPULCM_DATABUS eMpuBusType;

extern VOID vpostVAStartTrigger(void);
extern VOID vpostVAStopTrigger(void);
extern VOID vpostVAStopTriggerMPU(void);

typedef void (*LCMDEBUG)(void *);

static void debug_LcmType(void *pLcmType) {

	E_DRVVPOST_LCM_TYPE eLcmType;
	
	eLcmType = *(E_DRVVPOST_LCM_TYPE *)pLcmType;
	if (eLcmType == eDRVVPOST_SYNC)
		PRINTF("Sync type LCM is selected ! \n");
	else if (eLcmType == eDRVVPOST_MPU)
		PRINTF("MPY type LCM is selected ! \n");
	else
		PRINTF("wrong LCM is selected ! \n");
	PRINTF("\n");
}	

static void debug_LcmFrameDataType(void *pLcmFrameDataType) {

	E_DRVVPOST_FRAME_DATA_TYPE eLcmFrameDataType;
	
	eLcmFrameDataType = *(E_DRVVPOST_FRAME_DATA_TYPE *)pLcmFrameDataType;
	PRINTF("LCM frame data type is ");
	switch (eLcmFrameDataType) {
		case eDRVVPOST_FRAME_RGB555:
			PRINTF("RGB555. \n");
			break;
		case eDRVVPOST_FRAME_RGB565:
			PRINTF("RGB565. \n");
			break;
		case eDRVVPOST_FRAME_RGBx888:
			PRINTF("RGBx888. \n");
			break;
		case eDRVVPOST_FRAME_RGB888x:
			PRINTF("RGB888x. \n");
			break;
		case eDRVVPOST_FRAME_CBYCRY:
			PRINTF("CBYCRY. \n");
			break;
		case eDRVVPOST_FRAME_YCBYCR:
			PRINTF("YCBYCR. \n");
			break;
		case eDRVVPOST_FRAME_CRYCBY:
			PRINTF("CRYCBY. \n");
			break;
		case eDRVVPOST_FRAME_YCRYCB:
			PRINTF("YCRYCB. \n");
			break;
		default: 
			PRINTF("UNKNOWN !!! \n");
			break;
	}
	PRINTF("\n");	
}

static void debug_LcmResolution(void *pLcmWindow) {

	S_DRVVPOST_LCM_WINDOW *psLcmWindow;
	
	psLcmWindow = (S_DRVVPOST_LCM_WINDOW *)pLcmWindow;
	PRINTF("LCM pixel per line is %d ! \n", psLcmWindow->u32PixelPerLine);
	PRINTF("LCM line per panel is %d ! \n", psLcmWindow->u32LinePerPanel);
	PRINTF("\n");
}

static void debug_LcmPixelClock(void *pLcmPixelClock) {

	UINT32 u32LcmPixelClock;
	
	u32LcmPixelClock = *(UINT32 *)pLcmPixelClock;
	PRINTF("LCM pixel clock is %d KHz! \n\n", u32LcmPixelClock);
}

static void debug_MpuLcmInterface(void *pMpuLcmInterface) {

	UINT32 u32MpuLcmInterface;
	
	u32MpuLcmInterface = *(UINT32 *)pMpuLcmInterface;
	PRINTF("MPU LCM data bus interface is %d ! \n\n", u32MpuLcmInterface&0xFF);
}

static void debug_LcmHTiming(void *pLcmHTiming) {

	S_DRVVPOST_SYNCLCM_HTIMING *psLcmHTiming;
	
	psLcmHTiming = (S_DRVVPOST_SYNCLCM_HTIMING *)pLcmHTiming;
	/* In N32905 and N32926, the data width of BackPorch and FrontPorch
`	   is different. So, PulseWidth/BackPorch/FrontPorch are unified to
	   32-bit width in this tool. */	   
	PRINTF("LCM HTiming: sync pulse = 0x%x \n", psLcmHTiming->u8PulseWidth);
	PRINTF("LCM HTiming: back porch = 0x%x \n", psLcmHTiming->u8BackPorch);
	PRINTF("LCM HTiming: front porch = 0x%x \n", psLcmHTiming->u8FrontPorch);
	PRINTF("\n");
}

static void debug_LcmVTiming(void *pLcmVTiming) {

	S_DRVVPOST_SYNCLCM_VTIMING *psLcmVTiming;
	
	psLcmVTiming = (S_DRVVPOST_SYNCLCM_VTIMING *)pLcmVTiming;
	PRINTF("LCM VTiming: sync pulse = 0x%x \n", psLcmVTiming->u8PulseWidth);
	PRINTF("LCM VTiming: back porch = 0x%x \n", psLcmVTiming->u8BackPorch);
	PRINTF("LCM VTiming: front porch = 0x%x \n", psLcmVTiming->u8FrontPorch);
	PRINTF("\n");
}

static void debug_LcmSyncInterface(void *pSyncInterface) {

	E_DRVVPOST_SYNCLCM_INTERFACE eSyncInterface;
	
	eSyncInterface = *(E_DRVVPOST_SYNCLCM_INTERFACE *)pSyncInterface;
	PRINTF("sync LCM interface is ");
	switch (eSyncInterface) {
		case eDRVVPOST_RGB565:
			PRINTF("RGB565. \n");
			break;
		case eDRVVPOST_RGB666:
			PRINTF("RGB666. \n");
			break;
		case eDRVVPOST_RGB888:
			PRINTF("RGB888. \n");
			break;
		case eDRVVPOST_CCIR601:
			PRINTF("CCIR601. \n");
			break;
		case eDRVVPOST_RGBDummy:
			PRINTF("RGBDummy. \n");
			break;
		case eDRVVPOST_CCIR656:
			PRINTF("CCIR656. \n");
			break;
		case eDRVVPOST_RGBThrough:
			PRINTF("RGBThrough. \n");
			break;
		default: 
			PRINTF("UNKNOWN !!! \n");
			break;
	}
	PRINTF("\n");	
}

static void debug_LcmClockPolarity(void *pLcmClockPolarity) {

	S_DRVVPOST_CLK_POLARITY *psLcmClockPolarity;
	
	psLcmClockPolarity = (S_DRVVPOST_CLK_POLARITY *)pLcmClockPolarity;
	PRINTF("HClock polarity is %d (0: Low, 1: High) \n", psLcmClockPolarity->bHClock);
	PRINTF("VClock polarity is %d (0: Low, 1: High) \n", psLcmClockPolarity->bVClock);
	PRINTF("VDenClock polarity is %d (0: Low, 1: High) \n", psLcmClockPolarity->bVDenClock);
	PRINTF("PixelClock polarity is %d (0: Low, 1: High) \n", psLcmClockPolarity->bPixelClock);
	PRINTF("\n");
}

static void debug_LcmInternalRegister(void *pConfig) {

	S_DRVVPOST_CONFIGURE *psConfig;
	S_DRVVPOST_LCM_REG *psReg;
	UINT32 ii, u32RegNum;;

	ii = 0;
	psConfig = (S_DRVVPOST_CONFIGURE *)pConfig;
	psReg = psConfig->psLcmReg;
	u32RegNum = psConfig->u32RegNum;
	PRINTF("total 0x%x LCM internal registers must be set ! \n", u32RegNum);
	while(ii < u32RegNum) {
		PRINTF("reg_index = 0x%x, reg_data = 0x%x \n", psReg->u32RegIndex, psReg->u32RegData);
		psReg++;
		ii++;
	}
	PRINTF("\n");
}

static void debug_MpuLcmInternalRegister(void *pConfig) {

	S_DRVVPOST_MPULCM_INIT *psMpuLcm;
	S_DRVVPOST_LCM_REG *psReg;
	UINT32 ii, u32RegNum;

	ii = 0;
	psMpuLcm = (S_DRVVPOST_MPULCM_INIT *)pConfig;
	psReg = (S_DRVVPOST_LCM_REG*)psMpuLcm->psLcmReg;
	u32RegNum = (UINT32)psMpuLcm->u32RegNum;
	PRINTF("total 0x%x LCM internal registers must be set ! \n", u32RegNum);
	while(ii < u32RegNum) {
		PRINTF("reg_index = 0x%x, reg_data = 0x%x \n", psReg->u32RegIndex, psReg->u32RegData);
		psReg++;
		ii++;
	}
	PRINTF("\n");
}

static void debug_LcmConfigure(void *pLcmConfig) {

	S_DRVVPOST_CONFIGURE *psLcmConfig;
	S_DRVVPOST_SPI_CONFIGURE *pslcmSpiConfig;
	S_DRVVPOST_I2C_CONFIGURE *pslcmI2cConfig;

	psLcmConfig = (S_DRVVPOST_CONFIGURE *)pLcmConfig;
	pslcmSpiConfig = psLcmConfig->psSpiConfig;
	pslcmI2cConfig = psLcmConfig->psI2cConfig;	

	if (psLcmConfig->eInterface == eDRVVPOST_SPI) {
		PRINTF("Config interface is SPI ! \n");	
		/* SPI port setting */
		PRINTF("SPI CS port is GP_%d, pin_%d \n", pslcmSpiConfig->eCSPort, pslcmSpiConfig->eCSPin);
		PRINTF("SPI Clk port is GP_%d, pin_%d \n", pslcmSpiConfig->eClkPort, pslcmSpiConfig->eClkPin);
		PRINTF("SPI Data port is GP_%d, pin_%d \n", pslcmSpiConfig->eDataPort, pslcmSpiConfig->eDataPin);
		/* SPI type */
		PRINTF("SPI tpye: RW_ADDR_DATA (reserved) ! \n");						
		/* SPI internal register information */
		PRINTF("SPI register index width is %d bits \n", pslcmSpiConfig->u8RegIndexWidth);
		PRINTF("SPI register data width is %d bits \n", pslcmSpiConfig->u8RegDataWidth);
	} else if (psLcmConfig->eInterface == eDRVVPOST_I2C) {
		PRINTF("Config interface is I2C ! \n");	
		/* I2C ID address */
		PRINTF("I2C ID address is %x ! \n", pslcmI2cConfig->u8I2C_ID);
		/* I2C port setting */
		PRINTF("I2C Clk port is GP_%d, pin_%d \n", pslcmI2cConfig->eClkPort, pslcmI2cConfig->eClkPin);
		PRINTF("I2C Data port is GP_%d, pin_%d \n", pslcmI2cConfig->eDataPort, pslcmI2cConfig->eDataPin);
		/* get I2C internal register information */
		PRINTF("I2C register index width is %d bits \n", pslcmI2cConfig->u8RegIndexWidth);
		PRINTF("I2C register index width is %d bits \n", pslcmI2cConfig->u8RegDataWidth);
	} else
		PRINTF("Need not config LCM internal registers ! \n");
	PRINTF("\n");
}

static void debug_LcmBacklightCtrl(void *pLcmBacklight) {

	S_DRVVPOST_BACKLIGHT_CTRL *psLcmBacklight;
	
	psLcmBacklight = (S_DRVVPOST_BACKLIGHT_CTRL *)pLcmBacklight;
	/* ENA port setting for backlight control */
	if (psLcmBacklight->bDoNeedEnaPort) {
		PRINTF("Backlight control needs ENA port. \n");	
		PRINTF("ENA port is GP_%d, pin_%d. \n", psLcmBacklight->eEnaPort, psLcmBacklight->eEnaPin);
		PRINTF("ENA port state is ");
		if (psLcmBacklight->eEnaStae)
			PRINTF("High ! \n");
		else
			PRINTF("Low ! \n");
	}
	else
		PRINTF("Backlight control need not ENA port. \n");	

	/* PWM port setting for backlight control */
	if (psLcmBacklight->bDoNeedPWMPort) {
		PRINTF("Backlight control needs PWM port. \n");	
		PRINTF("PWM port is GP_%d, pin_%d. \n", psLcmBacklight->ePWMPort, psLcmBacklight->ePWMPin);
		PRINTF("PWM port state is ");
		if (psLcmBacklight->ePWMState)
			PRINTF("High ! \n");
		else
			PRINTF("Low ! \n");
	}
	else
		PRINTF("Backlight control need not PWM port. \n");	
		
	PRINTF("\n");
}

static void debug_LcmResetCtrl(void *pLcmReset) {

	S_DRVVPOST_RESET_CTRL *psLcmReset;

	psLcmReset = (S_DRVVPOST_RESET_CTRL *)pLcmReset;
	/* get setting for Reset control */
	if (psLcmReset->bDoNeedReset) {
		PRINTF("Need reset LCM ! \n");	
		PRINTF("Reset port is GP_%d, pin_%d. \n", psLcmReset->eResetPort, psLcmReset->eResetPin);
		PRINTF("Reset port state is  \n");
		if (psLcmReset->eResetStae)
			PRINTF("High ! \n");
		else
			PRINTF("Low ! \n");
	}
	else
		PRINTF("Need not reset LCM ! \n");	
		
	PRINTF("\n");
}

static void debug_LcmHVDeMode(void *pMode) {

	E_DRVVPOST_HSYNC_VSYNC_DE_MODE eHVDeMode;
	
	eHVDeMode = *(E_DRVVPOST_HSYNC_VSYNC_DE_MODE *)pMode;
	switch (eHVDeMode) {
		case eDRVVPOST_HSYNC_VSYNC_DE:
			PRINTF("Need Hsync+Vsync+DE sync signals !!\n");
			break;
		case eDRVVPOST_HSYNC_VSYNC_ONLY:
			PRINTF("Only need Hsync+Vsync sync signals !!\n");
			break;
		case eDRVVPOST_DE_ONLY:
			PRINTF("Only need DE sync signal !!\n");
			break;
		default: 
			PRINTF("Unknown sync signal !!\n");
			break;
	}
}

void lcm_debug(void *ptr, LCMDEBUG funptr) {

	funptr((void *)ptr);
}

void N9HxxLCMInit(UINT32* Vpost_Frame, UINT8* Lcm_Data) {

	UINT32 *ptr, ii;
	UINT8 u8LcmType;	

	PRINTF("\n================");
	PRINTF("\n=== LCM Init ===");	
	PRINTF("\n================\n");			
	
	/* get LCM data file */
	ptr = (UINT32* )Lcm_Data;
	ptr += eDRVVPOST_LCM_TYPE_OFFSET;
	u8LcmType = (*ptr) & 0xFF;
	lcm_debug((void *)ptr, debug_LcmType);	
	if (u8LcmType == eDRVVPOST_SYNC){
		/* get pointer */	
		psLcmInit = &sLcmInit;	

		/* get frame buffer data type */
		ptr = (UINT32 *)Lcm_Data;	
		ptr += eDRVVPOST_DATA_TYPE_OFFSET;
		psLcmInit->eDataFormat = (E_DRVVPOST_FRAME_DATA_TYPE)*ptr;
		lcm_debug((void *)&psLcmInit->eDataFormat, debug_LcmFrameDataType);
		
		/* get frame buffer */
		psLcmInit->u32BufAddr = (UINT32)Vpost_Frame;

		/* get LCM resolution */
		ptr = (UINT32 *)Lcm_Data;	
		ptr += eDRVVPOST_RESOLUTION_OFFSET;
		psLcmWindow = (S_DRVVPOST_LCM_WINDOW*)ptr;
		psLcmInit->u16HSize = psLcmWindow->u32PixelPerLine;
		psLcmInit->u16VSize = psLcmWindow->u32LinePerPanel;	
		lcm_debug((void *)psLcmWindow, debug_LcmResolution);		

		/* get LCM pixel clock */
		ptr = (UINT32 *)Lcm_Data;	
		ptr += eDRVVPOST_CLOCK_FREQ_OFFSET;
		psLcmInit->u32PixClock = *ptr;
		lcm_debug((void *)&psLcmInit->u32PixClock, debug_LcmPixelClock);		
		
		/* get LCM horizontal timing */
		ptr = (UINT32 *)Lcm_Data;	
		ptr += eDRVVPOST_HTIMING_OFFSET;
	#if 1
		// N9H20 only uses 8-bit to record horizontal sync pulse, porch width
		psHTiming = &sHTiming;
		psHTiming->u8PulseWidth = (*ptr++)&0xFF;
		psHTiming->u8BackPorch = (*ptr++)&0xFF;
		psHTiming->u8FrontPorch = (*ptr++)&0xFF;
		psLcmInit->psHTiming = (S_DRVVPOST_SYNCLCM_HTIMING*)psHTiming;
	#else
		psLcmInit->psHTiming = (S_DRVVPOST_SYNCLCM_HTIMING*)ptr;
	#endif
		lcm_debug((void *)psLcmInit->psHTiming, debug_LcmHTiming);

		/* get LCM vertical timing */
		ptr = (UINT32 *)Lcm_Data;	
		ptr += eDRVVPOST_VTIMING_OFFSET;
		psLcmInit->psVTiming = (S_DRVVPOST_SYNCLCM_VTIMING*)ptr;
		lcm_debug((void *)psLcmInit->psVTiming, debug_LcmVTiming);

		/* get Sync interface */
		ptr = (UINT32 *)Lcm_Data;	
		ptr += eDRVVPOST_SYNC_INTERFACE_OFFSET;
		psLcmInit->eSyncInterface = (E_DRVVPOST_SYNCLCM_INTERFACE)*ptr;
		lcm_debug((void *)&psLcmInit->eSyncInterface, debug_LcmSyncInterface);		

		/* get LCM clock polarity */
		ptr = (UINT32 *)Lcm_Data;	
		ptr += eDRVVPOST_POLARITY_OFFSET;
		psLcmInit->psClockPolarity = (S_DRVVPOST_CLK_POLARITY*)ptr;
		lcm_debug((void *)psLcmInit->psClockPolarity, debug_LcmClockPolarity);

		/* get Config interface */
		ptr = (UINT32 *)Lcm_Data;	
		ptr += eDRVVPOST_CONFIG_INTERFACE_OFFSET;
		if ((*ptr & 0xFF) == eDRVVPOST_SPI) {
			/* 1. get SPI Config information */
			psLcmInit->bDoNeedConfigLcm = TRUE;
			psLcmInit->psConfig->eInterface = (E_DRVVPOST_CONFIG_INTERFACE)(*ptr & 0xFF);
			ptr = (UINT32 *)Lcm_Data;	
			ptr += eDRVVPOST_SPI_CONFIG_OFFSET;
			psLcmInit->psConfig->psSpiConfig = (S_DRVVPOST_SPI_CONFIGURE*)ptr;
		} else if ((*ptr & 0xFF) == eDRVVPOST_I2C) {
			/* 2. get I2C Config information */
			psLcmInit->bDoNeedConfigLcm = TRUE;
			psLcmInit->psConfig->eInterface = (E_DRVVPOST_CONFIG_INTERFACE)(*ptr & 0xFF);
			ptr = (UINT32 *)Lcm_Data;	
			ptr += eDRVVPOST_I2C_CONFIG_OFFSET;
			psLcmInit->psConfig->psI2cConfig = (S_DRVVPOST_I2C_CONFIGURE*)ptr;
		} else {
			psLcmInit->bDoNeedConfigLcm = FALSE;	// don't need to configure LCM internal registers
		}
		lcm_debug((void *)psLcmInit->psConfig, debug_LcmConfigure);

		/* get LCM internal register settings */
		if (psLcmInit->bDoNeedConfigLcm == TRUE) {
			/* 1. get register number */
			ptr = (UINT32 *)Lcm_Data;	
			ptr += eDRVVPOST_REG_NUM_OFFSET;
			psLcmInit->psConfig->u32RegNum = *ptr;
			
			/* 2. get register pointer */
			ptr = (UINT32 *)Lcm_Data;	
			ptr += eDRVVPOST_REG_OFFSET;
			psLcmInit->psConfig->psLcmReg = (S_DRVVPOST_LCM_REG*)ptr;
			lcm_debug((void *)psLcmInit->psConfig, debug_LcmInternalRegister);
		}
		
		/* get Backlight control */		
		ptr = (UINT32 *)Lcm_Data;
		ptr += eDRVVPOST_BACKLIGHT_CTRL_OFFSET;
		psLcmInit->psBacklight = (S_DRVVPOST_BACKLIGHT_CTRL*)ptr;

		if ((psLcmInit->psBacklight->bDoNeedEnaPort)
		  ||(psLcmInit->psBacklight->bDoNeedPWMPort)) {
			psLcmInit->bDoNeedCtrlBacklight = TRUE;
		} else 
			psLcmInit->bDoNeedCtrlBacklight = FALSE;
		lcm_debug(psLcmInit->psBacklight, debug_LcmBacklightCtrl);		

		/* get Reset control */
		ptr = (UINT32 *)Lcm_Data;	
		ptr += eDRVVPOST_RESET_CTRL_OFFSET;
		psLcmInit->psReset = (S_DRVVPOST_RESET_CTRL*)ptr;
		lcm_debug((void *)psLcmInit->psReset, debug_LcmResetCtrl);		

		/* get Hsync+Vsync+DE mode */
		ptr = (UINT32 *)Lcm_Data;
		ptr += eDRVVPOST_HSYNC_VSYNC_DE_OFFSET;
		psLcmInit->eHVDeMode = (E_DRVVPOST_HSYNC_VSYNC_DE_MODE)*ptr;
		lcm_debug((void *)&psLcmInit->eHVDeMode, debug_LcmHVDeMode);
		
		/* initialize LCM */
		vpostSyncLCMInit((S_DRVVPOST_SYNCLCM_INIT*)psLcmInit);
		
	} else if (*ptr == eDRVVPOST_MPU) {
		/* get pointer */	
		psMpuLcmInit = &sMpuLcmInit;	

		/* get frame buffer data type */
		ptr = (UINT32 *)Lcm_Data;	
		ptr += eDRVVPOST_DATA_TYPE_OFFSET;
		psMpuLcmInit->eDataFormat = (E_DRVVPOST_FRAME_DATA_TYPE)*ptr;
		lcm_debug((void *)&psMpuLcmInit->eDataFormat, debug_LcmFrameDataType);
		
		/* get frame buffer */
		psMpuLcmInit->u32BufAddr = (UINT32)Vpost_Frame;

		/* get LCM resolution */
		ptr = (UINT32 *)Lcm_Data;	
		ptr += eDRVVPOST_RESOLUTION_OFFSET;
		psLcmWindow = (S_DRVVPOST_LCM_WINDOW*)ptr;
		psMpuLcmInit->u16HSize = psLcmWindow->u32PixelPerLine;
		psMpuLcmInit->u16VSize = psLcmWindow->u32LinePerPanel;	
		lcm_debug((void *)psLcmWindow, debug_LcmResolution);		

		/* get LCM pixel clock */
		ptr = (UINT32 *)Lcm_Data;	
		ptr += eDRVVPOST_CLOCK_FREQ_OFFSET;
		psMpuLcmInit->u32PixClock = *ptr;
		lcm_debug((void *)&psMpuLcmInit->u32PixClock, debug_LcmPixelClock);		

		/* get MPU LCM data interface */
		ptr = (UINT32 *)Lcm_Data;	
		ptr += eDRVVPOST_MPU_INTERFACE_OFFSET;
		psMpuLcmInit->eBusType = *ptr;
		lcm_debug((void *)&psMpuLcmInit->eBusType, debug_MpuLcmInterface);		
		
		/* get LCM internal register settings */
		/* 1. get register number */
		ptr = (UINT32 *)Lcm_Data;	
		ptr += eDRVVPOST_REG_NUM_OFFSET;
		psMpuLcmInit->u32RegNum = *ptr;
		
		/* 2. get register pointer */
		ptr = (UINT32 *)Lcm_Data;	
		ptr += eDRVVPOST_REG_OFFSET;
		psMpuLcmInit->psLcmReg = (S_DRVVPOST_LCM_REG*)ptr;
		lcm_debug((void *)psMpuLcmInit, debug_MpuLcmInternalRegister);
		
		/* get Backlight control */		
		ptr = (UINT32 *)Lcm_Data;
		ptr += eDRVVPOST_BACKLIGHT_CTRL_OFFSET;
		psMpuLcmInit->psBacklight = (S_DRVVPOST_BACKLIGHT_CTRL*)ptr;

		if ((psMpuLcmInit->psBacklight->bDoNeedEnaPort)
		  ||(psMpuLcmInit->psBacklight->bDoNeedPWMPort)) {
			psMpuLcmInit->bDoNeedCtrlBacklight = TRUE;
		} else 
			psMpuLcmInit->bDoNeedCtrlBacklight = FALSE;
		lcm_debug(psMpuLcmInit->psBacklight, debug_LcmBacklightCtrl);		

		/* get Reset control */
		ptr = (UINT32 *)Lcm_Data;	
		ptr += eDRVVPOST_RESET_CTRL_OFFSET;
		psMpuLcmInit->psReset = (S_DRVVPOST_RESET_CTRL*)ptr;
		lcm_debug((void *)psMpuLcmInit->psReset, debug_LcmResetCtrl);		
		
		/* initialize LCM */
		vpostMpuLCMInit((S_DRVVPOST_MPULCM_INIT*)psMpuLcmInit);

		PRINTF("MPU LCM run !!! \n");
	} else
		PRINTF("wrong LCM selected !!! \n");
		
	#define OPT_REG_DEBUG
	#ifdef OPT_REG_DEBUG
		for (ii=0; ii<100; ii+=4) {
			PRINTF("VPOST reg_index = 0x%x, reg_data = 0x%x !!!\n", 0xb1002000+ii, inpw(0xb1002000+ii));
		}
	#endif

}

void N9HxxLCMStop(UINT8 *Lcm_Data) {

	UINT32 *ptr;
	
	ptr = (UINT32 *)Lcm_Data;
	ptr += eDRVVPOST_LCM_TYPE_OFFSET;
	
	PRINTF("LCM stop !!! \n");		
	if ((*ptr & 0xFF) == eDRVVPOST_SYNC)
		vpostVAStopTrigger();
	else	// eDRVVPOST_MPU
		vpostVAStopTriggerMPU();
}

