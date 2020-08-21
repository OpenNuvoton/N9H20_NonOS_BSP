/**************************************************************************//**
 * @file     N9H20_VPOST_Sync_LCM.c
 * @brief    Panel driver file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#include "stdio.h"
#include "stdlib.h"
#include "N9H20_VPOST.h"

volatile UINT32 u32SpiCs_offset, u32SpiClk_offset, u32SpiData_offset;
volatile UINT32 u32SpiCs_port, u32SpiClk_port, u32SpiData_port;
volatile UINT32 u32SpiCs_pin, u32SpiClk_pin, u32SpiData_pin;
volatile UINT32 u32SpiRegIndexWidth, u32SpiRegDataWidth;
volatile UINT8 u8SpiType;

extern VOID vpostSetDataBusPin(E_DRVVPOST_DATABUS eDataBus);
extern VOID vpostSetDataBusPin_noDE(E_DRVVPOST_DATABUS eDataBus);
extern VOID vpostSetDataBusPin_onlyDE(E_DRVVPOST_DATABUS eDataBus);

typedef void (*LCMBUSTYPE)(E_DRVVPOST_DATABUS);

void SetSyncLcmDataBus(E_DRVVPOST_DATABUS eBus, LCMBUSTYPE funptr) {

	funptr(eBus);
}

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

#define SpiDisable()	outpw(REG_GPIOA_DOUT+u32SpiCs_port, inpw(REG_GPIOA_DOUT+u32SpiCs_port) | 0x00000001<<u32SpiCs_pin)
#define SpiEnable()		outpw(REG_GPIOA_DOUT+u32SpiCs_port, inpw(REG_GPIOA_DOUT+u32SpiCs_port) & ~(0x00000001<<u32SpiCs_pin));
#define SpiHighSCK()	outpw(REG_GPIOA_DOUT+u32SpiClk_port, inpw(REG_GPIOA_DOUT+u32SpiClk_port) | 0x00000001<<u32SpiClk_pin)
#define SpiLowSCK()		outpw(REG_GPIOA_DOUT+u32SpiClk_port, inpw(REG_GPIOA_DOUT+u32SpiClk_port) & ~(0x00000001<<u32SpiClk_pin));
#define SpiHighSDA()	outpw(REG_GPIOA_DOUT+u32SpiData_port, inpw(REG_GPIOA_DOUT+u32SpiData_port) | 0x00000001<<u32SpiData_pin)
#define SpiLowSDA()		outpw(REG_GPIOA_DOUT+u32SpiData_port, inpw(REG_GPIOA_DOUT+u32SpiData_port) & ~(0x00000001<<u32SpiData_pin));
#define SpiInSDA()		inpw(REG_GPIOA_PIN+u32SpiData_port)&(0x00000001<<u32SpiData_pin)

static void SpiDelay(UINT32 u32TimeCnt)
{
	UINT32 ii, jj;
	for (ii=0; ii<u32TimeCnt; ii++)
		for (jj=0; jj<100; jj++)
			jj++;
}

static void SpiWrRegInit(void)
{
	/* set SPI_CS to output mode and enable internal pull-up resistor */
	outpw(REG_GPAFUN+u32SpiCs_offset, inpw(REG_GPAFUN+u32SpiCs_offset) & ~(MF_GPA0 << (u32SpiCs_pin*2)));
	outpw(REG_GPIOA_OMD+u32SpiCs_port, inpw(REG_GPIOA_OMD+u32SpiCs_port) | 0x00000001<<u32SpiCs_pin);
	outpw(REG_GPIOA_PUEN+u32SpiCs_port, inpw(REG_GPIOA_PUEN+u32SpiCs_port) | 0x00000001<<u32SpiCs_pin);	

	/* set SPI_CLK to output mode and enable internal pull-up resistor */
	outpw(REG_GPAFUN+u32SpiClk_offset, inpw(REG_GPAFUN+u32SpiClk_offset) & ~(MF_GPA0 << (u32SpiClk_pin*2)));
	outpw(REG_GPIOA_OMD+u32SpiClk_port, inpw(REG_GPIOA_OMD+u32SpiClk_port) | 0x00000001<<u32SpiClk_pin);
	outpw(REG_GPIOA_PUEN+u32SpiClk_port, inpw(REG_GPIOA_PUEN+u32SpiClk_port) | 0x00000001<<u32SpiClk_pin);	

	/* set SPI_DATA to output mode and enable internal pull-up resistor */
	outpw(REG_GPAFUN+u32SpiData_offset, inpw(REG_GPAFUN+u32SpiData_offset) & ~(MF_GPA0 << (u32SpiData_pin*2)));
	outpw(REG_GPIOA_OMD+u32SpiData_port, inpw(REG_GPIOA_OMD+u32SpiData_port) | 0x00000001<<u32SpiData_pin);
	outpw(REG_GPIOA_PUEN+u32SpiData_port, inpw(REG_GPIOA_PUEN+u32SpiData_port) | 0x00000001<<u32SpiData_pin);	

	SpiLowSDA();		// serial data pin low
	SpiHighSCK();		// serial clock pin low
	SpiDisable();
	SpiDelay(5);
}

static void SpiDataPortInput(void)
{
	/* set SPI_DATA to input mode and disable internal pull-up resistor */
	outpw(REG_GPAFUN+u32SpiData_offset, inpw(REG_GPAFUN+u32SpiData_offset) & ~(MF_GPA0 << (u32SpiData_pin*2)));
	outpw(REG_GPIOA_OMD+u32SpiData_port, inpw(REG_GPIOA_OMD+u32SpiData_port) & ~(0x00000001<<u32SpiData_pin));
	outpw(REG_GPIOA_PUEN+u32SpiData_port, inpw(REG_GPIOA_PUEN+u32SpiData_port) & ~(0x00000001<<u32SpiData_pin));
}

static void SpiDataPortOutput(void)
{
	/* set SPI_DATA to output mode and enable internal pull-up resistor */
	outpw(REG_GPAFUN+u32SpiData_offset, inpw(REG_GPAFUN+u32SpiData_offset) & ~(MF_GPA0 << (u32SpiData_pin*2)));
	outpw(REG_GPIOA_OMD+u32SpiData_port, inpw(REG_GPIOA_OMD+u32SpiData_port) | 0x00000001<<u32SpiData_pin);
	outpw(REG_GPIOA_PUEN+u32SpiData_port, inpw(REG_GPIOA_PUEN+u32SpiData_port) | 0x00000001<<u32SpiData_pin);	
}

static void SpiWrReg(UINT16 nRegAddr, UINT16 nData)
{
	UINT32 i;

	// shift rgsiter index and data
	nRegAddr <<= 16-u32SpiRegIndexWidth;
	nData <<= 16-u32SpiRegDataWidth;
	
	SpiHighSCK();
	SpiDelay(2);
	SpiEnable();
	SpiDelay(2);
	
	// send WR bit	
	SpiLowSCK();
	SpiDelay(2);		
	SpiLowSDA();	// WR operation
	SpiDelay(2);
	SpiHighSCK();		
	SpiDelay(2);
	
	// Send register address, MSB first
	for( i = 0; i < u32SpiRegIndexWidth; i ++ )
	{
		SpiLowSCK();				
		if ( nRegAddr&0x8000 )
			SpiHighSDA();
		else
			SpiLowSDA();
		
		SpiDelay(3);
		SpiHighSCK();
		nRegAddr<<=1;
		SpiDelay(2);
	}
	// Send register data MSB first
	for( i = 0; i < u32SpiRegDataWidth; i ++ )
	{
		SpiLowSCK();						
		if ( nData&0x8000 )
			SpiHighSDA();
		else
			SpiLowSDA();
		
		SpiDelay(3);
		SpiHighSCK();
		nData<<=1;
		SpiDelay(2);
	}
	SpiDisable();
	SpiDelay(5);		
}

static UINT32 SpiRdReg(UINT16 nRegAddr)
{
	UINT32 i;
	UINT16 nData;

	// shift rgsiter index and data
	nRegAddr <<= 16-u32SpiRegIndexWidth;
	SpiHighSCK();
	SpiDelay(2);
	SpiEnable();
	SpiDelay(2);
	
	// send RD bit	
	SpiLowSCK();
	SpiDelay(2);		
	SpiHighSDA();	// RD operation
	SpiDelay(2);
	SpiHighSCK();		
	SpiDelay(2);
	
	// Send register address, MSB first
	for( i = 0; i < u32SpiRegIndexWidth; i ++ )
	{
		SpiLowSCK();
		if ( nRegAddr&0x8000 )
			SpiHighSDA();
		else
			SpiLowSDA();
		
		SpiDelay(3);
		SpiHighSCK();
		nRegAddr<<=1;
		SpiDelay(2);
	}
	// Get register data MSB first
	SpiDataPortInput();
	nData = 0;
	for( i = 0; i < u32SpiRegDataWidth; i ++ )
	{
		nData <<= 1;	
		SpiLowSCK();
		SpiDelay(3);
		SpiHighSCK();
		if (SpiInSDA())
			nData |= 0x01;
		SpiDelay(3);
	}
	SpiDisable();
	SpiDelay(5);
	PRINTF("reg_data = 0x%x !!! \n", nData);			
	return nData;
}

//static VOID vpostLcmConfigure(S_DRVVPOST_CONFIGURE* psConfig)
VOID vpostLcmConfigure(S_DRVVPOST_CONFIGURE* psConfig)
{
	S_DRVVPOST_LCM_REG* 	psLcmReg;
	UINT32 ii;
	
	psLcmReg = psConfig->psLcmReg;
	
	if (psConfig->eInterface == eDRVVPOST_SPI) {
		/* get SPI pins and other setting */
#if 0
	PRINTF("SpiCs_port = 0x%x \n", psConfig->psSpiConfig->eCSPort);
	PRINTF("SpiCs_pin = 0x%x \n", psConfig->psSpiConfig->eCSPin);	
	PRINTF("SpiData_port = 0x%x \n", psConfig->psSpiConfig->eDataPort);
	PRINTF("SpiData_pin = 0x%x \n", psConfig->psSpiConfig->eDataPin);	
	PRINTF("SpiClk_port = 0x%x \n", psConfig->psSpiConfig->eClkPort);
	PRINTF("SpiClk_pin = 0x%x \n", psConfig->psSpiConfig->eClkPin);
	PRINTF("Spi type = 0x%x \n", psConfig->psSpiConfig->eSpiType);
#endif
		
		u32SpiCs_offset = 4*psConfig->psSpiConfig->eCSPort;
		u32SpiCs_port = 0x10*psConfig->psSpiConfig->eCSPort;
		u32SpiCs_pin = psConfig->psSpiConfig->eCSPin;
		u32SpiClk_offset = 4*psConfig->psSpiConfig->eClkPort;
		u32SpiClk_port = 0x10*psConfig->psSpiConfig->eClkPort;
		u32SpiClk_pin = psConfig->psSpiConfig->eClkPin;
		u32SpiData_offset = 4*psConfig->psSpiConfig->eDataPort;
		u32SpiData_port = 0x10*psConfig->psSpiConfig->eDataPort;
		u32SpiData_pin = psConfig->psSpiConfig->eDataPin;
		u32SpiRegIndexWidth = psConfig->psSpiConfig->u8RegIndexWidth;
		u32SpiRegDataWidth = psConfig->psSpiConfig->u8RegDataWidth;
		u8SpiType = psConfig->psSpiConfig->eSpiType;

		/* SPI pins setting */
		SpiWrRegInit();
		/* begin register settings */
		ii = 0;
		PRINTF("total 0x%x LCM internal registers will be set ! \n", psConfig->u32RegNum);
		while(ii < psConfig->u32RegNum) {
			PRINTF("reg_index = 0x%x, reg_data = 0x%x \n", psLcmReg->u32RegIndex, psLcmReg->u32RegData);
			if ((psLcmReg->u32RegIndex&0xFFFF) == DRVVPOST_REG_DELAY) {
				SpiDelay(100*psLcmReg->u32RegData);
		//	} else if ((psLcmReg->u32RegIndex&0xFFFF) == DRVVPOST_REG_END) {
		//		break;
			} else {
				SpiWrReg(psLcmReg->u32RegIndex, psLcmReg->u32RegData);
			}
			psLcmReg++;
			ii++;
		}
	} else {
		// for I2C register configuration
	}
}

VOID vpostLcmReadInternalReg(S_DRVVPOST_CONFIGURE* psConfig)
{
	S_DRVVPOST_LCM_REG* 	psLcmReg;
	UINT32 ii;
	
	psLcmReg = psConfig->psLcmReg;
	
	if (psConfig->eInterface == eDRVVPOST_SPI) {
		/* get SPI pins and other setting */
		u32SpiCs_offset = 4*psConfig->psSpiConfig->eCSPort;
		u32SpiCs_port = 0x10*psConfig->psSpiConfig->eCSPort;
		u32SpiCs_pin = psConfig->psSpiConfig->eCSPin;
		u32SpiClk_offset = 4*psConfig->psSpiConfig->eClkPort;
		u32SpiClk_port = 0x10*psConfig->psSpiConfig->eClkPort;
		u32SpiClk_pin = psConfig->psSpiConfig->eClkPin;
		u32SpiData_offset = 4*psConfig->psSpiConfig->eDataPort;
		u32SpiData_port = 0x10*psConfig->psSpiConfig->eDataPort;
		u32SpiData_pin = psConfig->psSpiConfig->eDataPin;
		u32SpiRegIndexWidth = psConfig->psSpiConfig->u8RegIndexWidth;
		u32SpiRegDataWidth = psConfig->psSpiConfig->u8RegDataWidth;
		u8SpiType = psConfig->psSpiConfig->eSpiType;

		/* SPI pins setting */
		SpiWrRegInit();
		/* begin register settings */
		ii = 0;
		PRINTF("total 0x%x LCM internal registers will be got ! \n", psConfig->u32RegNum);
		while(ii < psConfig->u32RegNum) {
			psLcmReg->u32RegData = SpiRdReg(psLcmReg->u32RegIndex);
			PRINTF("reg_index = 0x%x, reg_data = 0x%x \n", psLcmReg->u32RegIndex, psLcmReg->u32RegData);			
			psLcmReg++;
			ii++;
		}
	} else {
		// for I2C register configuration
	}
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

void vpostSyncLCMInit(S_DRVVPOST_SYNCLCM_INIT* pLCM)
{
	volatile S_DRVVPOST_SYNCLCM_WINDOW sWindow;
	UINT32 u32ClockDivider;
	LCMBUSTYPE pBusType;

	// enable VPOST engine clock
	outpw(REG_AHBCLK, inpw(REG_AHBCLK) | VPOST_CKE | HCLK4_CKE);
	
	// reset VPOST engine
	outpw(REG_AHBIPRST, inpw(REG_AHBIPRST) | VPOSTRST);
	outpw(REG_AHBIPRST, inpw(REG_AHBIPRST) & ~VPOSTRST);	

	// set pixel clock
	u32ClockDivider = GetPLLOutputKhz(eUPLL, 12000)/(pLCM->u32PixClock);
	u32ClockDivider--;
	outpw(REG_CLKDIV1, (inpw(REG_CLKDIV1) & ~VPOST_N0));
	outpw(REG_CLKDIV1, (inpw(REG_CLKDIV1) & ~VPOST_N1)|((u32ClockDivider & 0xFF) << 8));
	outpw(REG_CLKDIV1, (inpw(REG_CLKDIV1) & ~VPOST_S)|(3<<3));	// VPOST clock from UPLL

	// stop VPOST
	vpostVAStopTrigger();

	// control Reset pin
	if (pLCM->psReset->bDoNeedReset==TRUE) {
		vpostLcmReset(pLCM->psReset);
	}
	
	// set frame buffer start addr
	outpw(REG_LCM_FSADDR, (UINT32)pLCM->u32BufAddr);
	
	// set frame buffer data format
	vpostSetFrameBuffer_DataType(pLCM->eDataFormat);
	
	// check data bus type
	switch (pLCM->eHVDeMode) {
		case eDRVVPOST_HSYNC_VSYNC_DE:
		default:
			pBusType = vpostSetDataBusPin;
			break;
		
		case eDRVVPOST_HSYNC_VSYNC_ONLY:
			pBusType = vpostSetDataBusPin_noDE;
			break;
		
		case eDRVVPOST_DE_ONLY:
			pBusType = vpostSetDataBusPin_onlyDE;
			break;
	}
	
	// set data bus and interface formats
	switch (pLCM->eSyncInterface) {
		case eDRVVPOST_RGB565:
		default:
			SetSyncLcmDataBus(eDRVVPOST_DATA_16BITS, pBusType);
			vpostSetLCM_TypeSelect(eDRVVPOST_HIGH_RESOLUTINO_SYNC);
			vpostSetParalelSyncLCM_Interface(eDRVVPOST_PRGB_16BITS);
			break;
		case eDRVVPOST_RGB666:
			SetSyncLcmDataBus(eDRVVPOST_DATA_18BITS, pBusType);
			vpostSetLCM_TypeSelect(eDRVVPOST_HIGH_RESOLUTINO_SYNC);
			vpostSetParalelSyncLCM_Interface(eDRVVPOST_PRGB_18BITS);
			break;
		case eDRVVPOST_RGB888:
			SetSyncLcmDataBus(eDRVVPOST_DATA_24BITS, pBusType);
			vpostSetLCM_TypeSelect(eDRVVPOST_HIGH_RESOLUTINO_SYNC);
			vpostSetParalelSyncLCM_Interface(eDRVVPOST_PRGB_24BITS);			
			break;
		case eDRVVPOST_CCIR601:
			SetSyncLcmDataBus(eDRVVPOST_DATA_8BITS, pBusType);
			// set 8-bit serial interface format
			vpostSetLCM_TypeSelect(eDRVVPOST_SYNC);		
			vpostSetSerialSyncLCM_Interface(eDRVVPOST_SRGB_YUV422);    
			break;
		case eDRVVPOST_RGBDummy:
			SetSyncLcmDataBus(eDRVVPOST_DATA_8BITS, pBusType);
			// set 8-bit serial interface format
			vpostSetLCM_TypeSelect(eDRVVPOST_SYNC);		
			vpostSetSerialSyncLCM_Interface(eDRVVPOST_SRGB_RGBDUMMY);    
			break;
		case eDRVVPOST_CCIR656:
			SetSyncLcmDataBus(eDRVVPOST_DATA_8BITS, pBusType);
			// set 8-bit serial interface format
			vpostSetLCM_TypeSelect(eDRVVPOST_SYNC);		
			vpostSetSerialSyncLCM_Interface(eDRVVPOST_SRGB_CCIR656);    
			break;
		case eDRVVPOST_RGBThrough:
			SetSyncLcmDataBus(eDRVVPOST_DATA_8BITS, pBusType);		
			// set 8-bit serial interface format
			vpostSetLCM_TypeSelect(eDRVVPOST_SYNC);		
			vpostSetSerialSyncLCM_Interface(eDRVVPOST_SRGB_RGBTHROUGH);    
			break;
	}

	// configure LCM internal registers if necessary
	if (pLCM->bDoNeedConfigLcm==TRUE) {
	//	PRINTF("configure LCM registers !!! \n");
		vpostLcmConfigure(pLCM->psConfig);
	}
	
	// configure LCD timing sync or async with TV timing	
	switch (pLCM->eSyncInterface) {
		case eDRVVPOST_RGB565:
		case eDRVVPOST_RGB666:
		case eDRVVPOST_RGB888:
		case eDRVVPOST_RGBThrough:
			vpostsetLCM_TimingType(eDRVVPOST_ASYNC_TV);		
		default:
			break;
		case eDRVVPOST_CCIR601:
		case eDRVVPOST_CCIR656:		
		case eDRVVPOST_RGBDummy:
			vpostsetLCM_TimingType(eDRVVPOST_SYNC_TV);
			break;
	}
	
	// set Horizontal scanning line timing for Syn type LCD 
	vpostSetSyncLCM_HTiming(pLCM->psHTiming);

	// set Vertical scanning line timing for Syn type LCD   
	vpostSetSyncLCM_VTiming(pLCM->psVTiming);
	
  	// set Hsync/Vsync/Vden/Pclk poalrity
	outpw(REG_LCM_TCON4, ((inpw(REG_LCM_TCON4) & ~(TCON4_VSP | TCON4_HSP | TCON4_DEP | TCON4_PCLKP))
	    | ((pLCM->psClockPolarity->bVClock & 0x1) << 3)
	    | ((pLCM->psClockPolarity->bHClock & 0x1) << 2)
	    | ((pLCM->psClockPolarity->bVDenClock & 0x1) << 1)
	    | (((~pLCM->psClockPolarity->bPixelClock) & 0x1))));

	// set display window
	switch (pLCM->eSyncInterface) {
		case eDRVVPOST_RGB565:
		case eDRVVPOST_RGB666:
		case eDRVVPOST_RGB888:		
		default:
			sWindow.u16ClockPerLine = pLCM->u16HSize;
			break;
		case eDRVVPOST_CCIR601:
		case eDRVVPOST_CCIR656:		
			sWindow.u16ClockPerLine = 2*pLCM->u16HSize;
			break;
		case eDRVVPOST_RGBDummy:
			sWindow.u16ClockPerLine = 4*pLCM->u16HSize;
			break;
		case eDRVVPOST_RGBThrough:
			sWindow.u16ClockPerLine = 3*pLCM->u16HSize;
			break;
	}
	sWindow.u16PixelPerLine = pLCM->u16HSize;
	sWindow.u16LinePerPanel = pLCM->u16VSize;		
	vpostSetSyncLCM_ImageWindow((S_DRVVPOST_SYNCLCM_WINDOW *)&sWindow);

	// little-endian for YUV format
	vpostSetYUVEndianSelect(eDRVVPOST_YUV_LITTLE_ENDIAN);
	
	// LCD image source select
	vpostSetLCM_ImageSource(eDRVVPOST_FRAME_BUFFER);
	
	// enable VPOST
	vpostVAStartTrigger();

	// control backlight
	if (pLCM->bDoNeedCtrlBacklight==TRUE) {
	//	PRINTF("Do backlight control !!! \n");
		vpostLcmBacklight(pLCM->psBacklight);
	}
}

