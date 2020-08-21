/**************************************************************************//**
 * @file     N9H20_VPOST_Mpu_LCM.c
 * @brief    Panel driver file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#include "stdio.h"
#include "stdlib.h"
#include "N9H20_VPOST.h"

typedef enum {
	eEXT 	= 0,
	eX32K 	= 1,
	eAPLL  	= 2,
	eUPLL  	= 3
} E_CLK;

static UINT32 GetPLLOutputKhz(E_CLK eSysPll, UINT32 u32FinKHz )
{
	
	UINT32 u32Freq, u32PllCntlReg;
	UINT32 NF, NR, NO;
	
	UINT8 au8Map[4] = {1, 2, 2, 4};
	if(eSysPll==eSYS_APLL)
		u32PllCntlReg = inp32(REG_APLLCON);
	else if(eSysPll==eSYS_UPLL)	
		u32PllCntlReg = inp32(REG_UPLLCON);		
	
	NF = (u32PllCntlReg&FB_DV)+2;
	NR = ((u32PllCntlReg & IN_DV)>>9)+2;
	NO = au8Map[((u32PllCntlReg&OUT_DV)>>14)];
	u32Freq = u32FinKHz*NF/NR/NO;
	return u32Freq;
}

static void MpuWriteRegIndex(UINT32 u32RegIndex)
{
	INT32 nFlag;

	outpw(REG_LCM_MPUCMD, inpw(REG_LCM_MPUCMD) & ~(MPUCMD_MPU_ON|MPUCMD_MPU_CS|MPUCMD_WR_RS|MPUCMD_MPU_RWn) );	// CS=0, RS=0	
	outpw(REG_LCM_MPUCMD, inpw(REG_LCM_MPUCMD) | (DRVVPOST_MPU_CMD_MODE << 29) );	// R/W command/paramter mode
	outpw(REG_LCM_MPUCMD, inpw(REG_LCM_MPUCMD) & ~MPUCMD_MPU_RWn );					// Write Command/Data Selection			
	outpw(REG_LCM_MPUCMD, (inpw(REG_LCM_MPUCMD) & ~MPUCMD_MPU_CMD) | u32RegIndex );		// WRITE register address
	outpw(REG_LCM_MPUCMD, inpw(REG_LCM_MPUCMD) | MPUCMD_MPU_ON);					// trigger command output
	while(inpw(REG_LCM_MPUCMD) & MPUCMD_BUSY);									// wait command to be sent
	outpw(REG_LCM_MPUCMD, inpw(REG_LCM_MPUCMD) & (~MPUCMD_MPU_ON) );				// reset command ON flag
	outpw(REG_LCM_MPUCMD, inpw(REG_LCM_MPUCMD) | MPUCMD_MPU_CS |MPUCMD_WR_RS);		// CS=1, RS=1	
	
	nFlag = 1000;
	while( nFlag--);	//delay for a while on purpose.
}

//static void MpuWriteRegIndex(UINT16 u16RegIndex)
//{
//	vpostMPULCDWriteAddr16Bit(u16RegIndex);
//}

static void MpuWriteRegData(UINT16 u32RegData)
{
	outpw(REG_LCM_MPUCMD, inpw(REG_LCM_MPUCMD) & ~(MPUCMD_MPU_ON|MPUCMD_MPU_CS|MPUCMD_MPU_RWn) );			// CS=0 	
	outpw(REG_LCM_MPUCMD, inpw(REG_LCM_MPUCMD) | MPUCMD_WR_RS );							// RS=1	
	outpw(REG_LCM_MPUCMD, inpw(REG_LCM_MPUCMD) | (DRVVPOST_MPU_CMD_MODE << 29) );	// R/W command/paramter mode
	outpw(REG_LCM_MPUCMD, inpw(REG_LCM_MPUCMD) & ~MPUCMD_MPU_RWn );					// Write Command/Data Selection			
	outpw(REG_LCM_MPUCMD, (inpw(REG_LCM_MPUCMD) & ~MPUCMD_MPU_CMD) | u32RegData);	// WRITE register data
	outpw(REG_LCM_MPUCMD, inpw(REG_LCM_MPUCMD) | MPUCMD_MPU_ON);					// trigger command output
	
	while(inpw(REG_LCM_MPUCMD) & MPUCMD_BUSY);									// wait command to be sent
	outpw(REG_LCM_MPUCMD, inpw(REG_LCM_MPUCMD) & (~MPUCMD_MPU_ON) );				// reset command ON flag
	outpw(REG_LCM_MPUCMD, inpw(REG_LCM_MPUCMD) | MPUCMD_MPU_CS);					// CS=1
}

//static void MpuWriteRegData(UINT16 u16rRegData)
//{
//	vpostMPULCDWriteData16Bit(u16rRegData);
//}

static void LcmDelay(UINT32 u32TimeCnt)
{
	UINT32 ii, jj;
	for (ii=0; ii<u32TimeCnt; ii++)
		for (jj=0; jj<100; jj++)
			jj++;
}

static VOID vpostLcmBacklight(S_DRVVPOST_BACKLIGHT_CTRL* psBacklight)
{
	UINT32 u32offset, u32pin;
	
	u32offset = 4*psBacklight->eEnaPort;
	u32pin = psBacklight->eEnaPin;
	if (psBacklight->bDoNeedEnaPort == TRUE) {
		/* set BL_EN */
		outpw(REG_GPAFUN+u32offset, inpw(REG_GPAFUN+u32offset) & ~(MF_GPA0 << (u32pin*2)));
		u32offset = 0x10*psBacklight->eEnaPort;	
		outpw(REG_GPIOA_OMD+u32offset, inpw(REG_GPIOA_OMD+u32offset) | 0x00000001<<u32pin);
		if (psBacklight->eEnaStae == eDRVVPOST_PIN_HI)
			outpw(REG_GPIOA_DOUT+u32offset, inpw(REG_GPIOA_DOUT+u32offset) | 0x00000001<<u32pin);
		else
			outpw(REG_GPIOA_DOUT+u32offset, inpw(REG_GPIOA_DOUT+u32offset) & ~(0x00000001<<u32pin));
	}
	
	u32offset = 4*psBacklight->ePWMPort;
	u32pin = psBacklight->ePWMPin;
	if (psBacklight->bDoNeedPWMPort == TRUE) {
		/* set BL_PWM */
		outpw(REG_GPAFUN+u32offset, inpw(REG_GPAFUN+u32offset) & ~(MF_GPA0 << (u32pin*2)));
		u32offset = 0x10*psBacklight->ePWMPort;
		outpw(REG_GPIOA_OMD+u32offset, inpw(REG_GPIOA_OMD+u32offset) | 0x00000001<<u32pin);
		if (psBacklight->ePWMState == eDRVVPOST_PIN_HI)
			outpw(REG_GPIOA_DOUT+u32offset, inpw(REG_GPIOA_DOUT+u32offset) | 0x00000001<<u32pin);
		else
			outpw(REG_GPIOA_DOUT+u32offset, inpw(REG_GPIOA_DOUT+u32offset) & ~(0x00000001<<u32pin));
	}
}

static VOID vpostLcmReset(S_DRVVPOST_RESET_CTRL* psReset)
{
	UINT32 u32offset, u32pin, ii;
	
	u32offset = 4*psReset->eResetPort;
	u32pin = psReset->eResetPin;
	
	/* do LCM Reset */
	outpw(REG_GPAFUN+u32offset, inpw(REG_GPAFUN+u32offset) & ~(MF_GPA0 << (u32pin*2)));
	u32offset = 0x10*psReset->eResetPort;	
	outpw(REG_GPIOA_OMD+u32offset, inpw(REG_GPIOA_OMD+u32offset) | 0x00000001<<u32pin);
	if (psReset->eResetStae == eDRVVPOST_PIN_HI) {
		outpw(REG_GPIOA_DOUT+u32offset, inpw(REG_GPIOA_DOUT+u32offset) & ~(0x00000001<<u32pin));
		for (ii=0; ii<500; ii++);
		outpw(REG_GPIOA_DOUT+u32offset, inpw(REG_GPIOA_DOUT+u32offset) | 0x00000001<<u32pin);
		for (ii=0; ii<1000; ii++);
		outpw(REG_GPIOA_DOUT+u32offset, inpw(REG_GPIOA_DOUT+u32offset) & ~(0x00000001<<u32pin));
		for (ii=0; ii<500; ii++);		
	} else {
		outpw(REG_GPIOA_DOUT+u32offset, inpw(REG_GPIOA_DOUT+u32offset) | 0x00000001<<u32pin);
		for (ii=0; ii<500; ii++);
		outpw(REG_GPIOA_DOUT+u32offset, inpw(REG_GPIOA_DOUT+u32offset) & ~(0x00000001<<u32pin));		
		for (ii=0; ii<1000; ii++);
		outpw(REG_GPIOA_DOUT+u32offset, inpw(REG_GPIOA_DOUT+u32offset) | 0x00000001<<u32pin);
		for (ii=0; ii<500; ii++);		
	}
}

VOID vpostMpuLcmConfigure(S_DRVVPOST_LCM_REG* psLcmReg, UINT32	u32RegNum)
{
	UINT32 ii;
	
	/* begin register settings */
	ii = 0;
	PRINTF("total 0x%x LCM internal registers will be set ! \n", u32RegNum);
	while(ii < u32RegNum) {
		PRINTF("reg_index = 0x%x, reg_data = 0x%x \n", psLcmReg->u32RegIndex, psLcmReg->u32RegData);
		if ((psLcmReg->u32RegIndex&0xFFFF) == DRVVPOST_REG_DELAY) {
			LcmDelay(100*psLcmReg->u32RegData);
		} else if ((psLcmReg->u32RegIndex&0xFFFF) == DRVVPOST_REG_NO_INDEX) {
			MpuWriteRegData(psLcmReg->u32RegData);		
	//	} else if ((psLcmReg->u32RegIndex&0xFFFF) == DRVVPOST_REG_END) {		
	//		break;
		} else {
			MpuWriteRegIndex(psLcmReg->u32RegIndex);
			if (psLcmReg->u32RegData != DRVVPOST_REG_NO_DATA)
				MpuWriteRegData(psLcmReg->u32RegData);
		}
		psLcmReg++;
		ii++;
	}
}

void vpostMpuLCMInit(S_DRVVPOST_MPULCM_INIT* pMpuLCM)
{
	volatile S_DRVVPOST_MPULCM_TIMING sTiming = {1,1,2,1};
	volatile S_DRVVPOST_MPULCM_WINDOW sWindow = {240,320};
	UINT32 u32ClockDivider;

	// enable VPOST engine clock
	outpw(REG_AHBCLK, inpw(REG_AHBCLK) | VPOST_CKE | HCLK4_CKE);
	
	// reset VPOST engine
	outpw(REG_AHBIPRST, inpw(REG_AHBIPRST) | VPOSTRST);
	outpw(REG_AHBIPRST, inpw(REG_AHBIPRST) & ~VPOSTRST);	

	// set pixel clock
//	PRINTF("set pixel clock = %d \n", pMpuLCM->u32PixClock);
//	u32ClockDivider = GetPLLOutputKhz(eUPLL, 12000)/(pMpuLCM->u32PixClock);
	PRINTF("set pixel clock = %d \n", 40000);
	u32ClockDivider = GetPLLOutputKhz(eUPLL, 12000)/40000;
	u32ClockDivider--;
	outpw(REG_CLKDIV1, (inpw(REG_CLKDIV1) & ~VPOST_N0));
	outpw(REG_CLKDIV1, (inpw(REG_CLKDIV1) & ~VPOST_N1)|((u32ClockDivider & 0xFF) << 8));
	outpw(REG_CLKDIV1, (inpw(REG_CLKDIV1) & ~VPOST_S)|(3<<3));	// VPOST clock from UPLL
	PRINTF("clock divider is 0x%x \n", u32ClockDivider);	

	// stop VPOST
	PRINTF("stop VPOST !! \n");
	vpostVAStopTriggerMPU();
	vpostVAStopTrigger();

	// control Reset pin
	if (pMpuLCM->psReset->bDoNeedReset==TRUE) {
		PRINTF("reset LCM !! \n");	
		vpostLcmReset(pMpuLCM->psReset);
	}

	// configure LCD interface
	PRINTF("MPU type is selected !! \n");
	vpostSetLCM_TypeSelect(eDRVVPOST_MPU);
	
	// configure LCD timing sync or async with TV timing	
	vpostsetLCM_TimingType(eDRVVPOST_ASYNC_TV);

    // set MPU timing 
	PRINTF("confiure LCM timing !! \n");
    vpostSetMPULCM_TimingConfig((S_DRVVPOST_MPULCM_TIMING *)&sTiming);
	
	// set frame buffer start addr
	outpw(REG_LCM_FSADDR, (UINT32)pMpuLCM->u32BufAddr);
	
	// set frame buffer data format
	vpostSetFrameBuffer_DataType(pMpuLCM->eDataFormat);
	
	// set data bus and interface formats
	PRINTF("MPU data is %d \n", pMpuLCM->eBusType);
	switch (pMpuLCM->eBusType) {
		case eDRVVPOST_MPU_8_8:
		default:
			vpostSetDataBusPin(eDRVVPOST_DATA_8BITS);
			vpostSetMPULCM_BusModeSelect(eDRVVPOST_MPU_8_8);			
			break;
		case eDRVVPOST_MPU_2_8_8:
			vpostSetDataBusPin(eDRVVPOST_DATA_8BITS);
			vpostSetMPULCM_BusModeSelect(eDRVVPOST_MPU_2_8_8);						
			break;
		case eDRVVPOST_MPU_8_8_8:
			vpostSetDataBusPin(eDRVVPOST_DATA_8BITS);
			vpostSetMPULCM_BusModeSelect(eDRVVPOST_MPU_8_8_8);			
			break;
		case eDRVVPOST_MPU_9_9:
			vpostSetDataBusPin(eDRVVPOST_DATA_9BITS);
			vpostSetMPULCM_BusModeSelect(eDRVVPOST_MPU_9_9);			
			break;
		case eDRVVPOST_MPU_16:
			vpostSetDataBusPin(eDRVVPOST_DATA_16BITS);
			vpostSetMPULCM_BusModeSelect(eDRVVPOST_MPU_16);			
			break;
		case eDRVVPOST_MPU_16_2:
			vpostSetDataBusPin(eDRVVPOST_DATA_16BITS);
			vpostSetMPULCM_BusModeSelect(eDRVVPOST_MPU_16_2);			
			break;
		case eDRVVPOST_MPU_2_16:
			vpostSetDataBusPin(eDRVVPOST_DATA_16BITS);
			vpostSetMPULCM_BusModeSelect(eDRVVPOST_MPU_2_16);			
			break;
		case eDRVVPOST_MPU_16_8:
			vpostSetDataBusPin(eDRVVPOST_DATA_16BITS);
			vpostSetMPULCM_BusModeSelect(eDRVVPOST_MPU_16_8);			
			break;
		case eDRVVPOST_MPU_18:
			vpostSetDataBusPin(eDRVVPOST_DATA_18BITS);
			vpostSetMPULCM_BusModeSelect(eDRVVPOST_MPU_18);			
			break;
		case eDRVVPOST_MPU_18_6:
			vpostSetDataBusPin(eDRVVPOST_DATA_18BITS);
			vpostSetMPULCM_BusModeSelect(eDRVVPOST_MPU_18_6);			
			break;
		case eDRVVPOST_MPU_24:
			vpostSetDataBusPin(eDRVVPOST_DATA_24BITS);
			vpostSetMPULCM_BusModeSelect(eDRVVPOST_MPU_24);			
			break;
	}

	// configure MPU LCM internal regiters
	PRINTF("configure LCM internal registers !!\n");
	vpostMpuLcmConfigure(pMpuLCM->psLcmReg, pMpuLCM->u32RegNum);
	
    // set MPU LCM window
	sWindow.u16PixelPerLine = pMpuLCM->u16HSize;
	sWindow.u16LinePerPanel = pMpuLCM->u16VSize;		
	vpostSetMPULCM_ImageWindow((S_DRVVPOST_MPULCM_WINDOW *)&sWindow);

	// little-endian for YUV format
	vpostSetYUVEndianSelect(eDRVVPOST_YUV_LITTLE_ENDIAN);
	
	// LCD image source select
	vpostSetLCM_ImageSource(eDRVVPOST_FRAME_BUFFER);

	// RS pin to High
	outp32(REG_LCM_MPUCMD, inp32(REG_LCM_MPUCMD) | MPUCMD_MPU_CS |MPUCMD_WR_RS);				// CS=1, RS=1		

	// enable MPU LCD controller
	PRINTF("trigger MPU LCM !!\n", pMpuLCM->eBusType);	
	vpostVAStartTrigger_MPUContinue();

	// control backlight
	if (pMpuLCM->bDoNeedCtrlBacklight==TRUE) {
	//	PRINTF("Do backlight control !!! \n");
		vpostLcmBacklight(pMpuLCM->psBacklight);
	}
}
