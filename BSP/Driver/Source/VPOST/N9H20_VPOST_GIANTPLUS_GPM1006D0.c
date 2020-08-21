/**************************************************************************//**
 * @file     N9H20_VPOST_GIANTPLUS_GPM1006D0.c
 * @brief    Panel driver file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#include "stdio.h"
#include "stdlib.h"
#include "N9H20_VPOST.h"

extern void LCDDelay(unsigned int nCount);

#if defined(__HAVE_GIANTPLUS_GPM1006D0__)

//#define OPT_SPI_CS_BY_GPD2
static UINT32 g_nScreenWidth;
static UINT32 g_nScreenHeight;

typedef enum 
{
	eEXT 	= 0,
	eX32K 	= 1,
	eAPLL  	= 2,
	eUPLL  	= 3
}E_CLK;

static void delay_loop(UINT8 u8delay)
{
	volatile UINT8 ii, jj;
	for (jj=0; jj<u8delay; jj++)
		for (ii=0; ii<200; ii++);
}

//=====================================================================
//	for panel init
//=====================================================================
//	#define SpiEnable()		outp32(REG_GPIOD_DOUT, inp32(REG_GPIOD_DOUT) & ~0x2000)
//	#define SpiDisable()	outp32(REG_GPIOD_DOUT, inp32(REG_GPIOD_DOUT) |  0x2000)	
//	#define SpiHighSCK()	outp32(REG_GPIOD_DOUT, inp32(REG_GPIOD_DOUT) |  0x1000)	
//	#define SpiLowSCK()		outp32(REG_GPIOD_DOUT, inp32(REG_GPIOD_DOUT) & ~0x1000)
//	#define SpiHighSDA()	outp32(REG_GPIOD_DOUT, inp32(REG_GPIOD_DOUT) |  0x8000)	
//	#define SpiLowSDA()		outp32(REG_GPIOD_DOUT, inp32(REG_GPIOD_DOUT) & ~0x8000)

	#define SpiEnable()		outp32(REG_GPIOD_DOUT, inp32(REG_GPIOD_DOUT) & ~0x4)
	#define SpiDisable()	outp32(REG_GPIOD_DOUT, inp32(REG_GPIOD_DOUT) |  0x4)	

	#define SpiHighSCK()	outp32(REG_GPIOB_DOUT, inp32(REG_GPIOB_DOUT) |  0x2000)	
	#define SpiLowSCK()		outp32(REG_GPIOB_DOUT, inp32(REG_GPIOB_DOUT) & ~0x2000)
	#define SpiHighSDA()	outp32(REG_GPIOB_DOUT, inp32(REG_GPIOB_DOUT) |  0x4000)	
	#define SpiLowSDA()		outp32(REG_GPIOB_DOUT, inp32(REG_GPIOB_DOUT) & ~0x4000)

	void SpiDelay(UINT32 u32TimeCnt)
	{
		UINT32 ii, jj;
		
		for (ii=0; ii<u32TimeCnt; ii++)
			for (jj=0; jj<100; jj++)
				jj++;
	}

	void SpiInit()
	{	
//		outp32(REG_GPIOD_OMD, inp32(REG_GPIOD_OMD) | 0xB000);
//		outp32(REG_GPIOD_PUEN, inp32(REG_GPIOD_PUEN) | 0x8000);		
		outp32(REG_GPIOB_OMD, inp32(REG_GPIOB_OMD) | 0x6000);
		outp32(REG_GPIOB_PUEN, inp32(REG_GPIOB_PUEN) | 0x6000);		
		outp32(REG_GPIOD_OMD, inp32(REG_GPIOD_OMD) | 0x04);
		outp32(REG_GPIOD_PUEN, inp32(REG_GPIOD_PUEN) | 0x04);		
		
	}

	void ILI_RdWtRegInit()
	{
//		outp32(REG_GPDFUN, inp32(REG_GPDFUN) & ~(MF_GPD15+MF_GPD13+MF_GPD12));	//GPD_12/13/15 to GPIO mode
//		outp32(REG_GPBFUN, inp32(REG_GPBFUN) & ~(MF_GPB13+MF_GPB14));	//GPB13/14 
		outp32(REG_GPBFUN, inp32(REG_GPBFUN) & ~(MF_GPB13+MF_GPB14));	//GPB13/14 
		outp32(REG_GPDFUN, inp32(REG_GPDFUN) & ~(MF_GPD2));				//GPD2 
			
		SpiInit();
		SpiLowSDA();		// serial data pin low
		SpiHighSCK();		// serial clock pin low
		SpiDisable();
		SpiDelay(5);				
	}
	
	void ILITek_WtReg(UINT8 nRegAddr, UINT8 nData)
	{
		UINT32 i;
		
		nRegAddr <<= 1;
		SpiEnable();
		SpiHighSCK();		
		
		// send WR bit		
		SpiLowSCK();
		SpiDelay(2);		
		SpiLowSDA();		
		SpiDelay(2);
		SpiHighSCK();		
		SpiDelay(2);		
		
		// Send register address, MSB first
		for( i = 0; i < 7; i ++ )
		{
			SpiLowSCK();				
			if ( nRegAddr&0x80 )
				SpiHighSDA();
			else
				SpiLowSDA();
			
			SpiDelay(3);
			SpiHighSCK();
			nRegAddr<<=1;
			SpiDelay(2);
		}
		// Send register data LSB first
		for( i = 0; i < 8; i ++ )
		{
			SpiLowSCK();						
			if ( nData&0x80 )
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

void ILI_I9322_RGBDummy_NTSC_INIT(void)
{
	ILI_RdWtRegInit();

#if 1
	ILITek_WtReg(0x07,0xEE);		// turn off charge-pump
	delay_loop(200);
	ILITek_WtReg(0x07,0xEF);		// turn on charge-pump
	delay_loop(200);
	
	ILITek_WtReg(0x04,0x00);		// reset all registers to the default values
	delay_loop(200);
	ILITek_WtReg(0x04,0x01);		// normal operation
	
	ILITek_WtReg(0x03,0x09);		// Vreg1 out = 4.5V
	ILITek_WtReg(0x02,0x32);		// Vcom high voltage
	ILITek_WtReg(0x01,0x12);		// Vcom AC voltage	
	ILITek_WtReg(0x06,0x23);		// RGB Dummy mode
	ILITek_WtReg(0x0A,0x78);		// signal polarity
	ILITek_WtReg(0x10,0xA7);		// Gamma correction settings
	ILITek_WtReg(0x11,0x57);		// Gamma correction settings
	ILITek_WtReg(0x12,0x73);		// Gamma correction settings
	ILITek_WtReg(0x13,0x72);		// Gamma correction settings
	ILITek_WtReg(0x14,0x73);		// Gamma correction settings
	ILITek_WtReg(0x15,0x55);		// Gamma correction settings
	ILITek_WtReg(0x16,0x17);		// Gamma correction settings
	ILITek_WtReg(0x17,0x62);		// Gamma correction settings							
	
#else	
	
	ILITek_WtReg(0x04,0x00);		// Reset to all registers to default values
	ILITek_WtReg(0x04,0x01);		// Normal operation
	
	ILITek_WtReg(0x06,0x2C);		// Bit[7:4] = 0x2 --> RGB Dummy 320x240
									// Bit[3:2] = 0x3 --> auto detection mode for NTSC/PAL
									// Bit[1:1] = 0x1 --> up-to-down scan
									// Bit[0:0] = 0x1 --> left-to-right scan										
									
	delay_loop(2000);												
	ILITek_WtReg(0x07,0xEE);		// turn off charge-pump
	delay_loop(4000);												
	ILITek_WtReg(0x07,0xEF);		// turn off charge-pump		
	delay_loop(40000);												
#endif	
}			

void ILI_I9322_RGBThrough_NTSC_INIT(void)
{
	ILI_RdWtRegInit();
	
#if 1
//	ILITek_WtReg(0x07,0xEE);		// turn off charge-pump
	ILITek_WtReg(0x07,0xEF);		// turn on charge-pump
	ILITek_WtReg(0x03,0x09);		// Vreg1 out = 4.5V
	ILITek_WtReg(0x02,0x32);		// Vcom high voltage
	ILITek_WtReg(0x01,0x12);		// Vcom AC voltage	
//	ILITek_WtReg(0x06,0x23);		// RGB Dummy mode
	ILITek_WtReg(0x06,0x03);		// RGB Dummy mode
	ILITek_WtReg(0x0A,0x78);		// signal polarity
	ILITek_WtReg(0x10,0xA7);		// Gamma correction settings
	ILITek_WtReg(0x11,0x57);		// Gamma correction settings
	ILITek_WtReg(0x12,0x73);		// Gamma correction settings
	ILITek_WtReg(0x13,0x72);		// Gamma correction settings
	ILITek_WtReg(0x14,0x73);		// Gamma correction settings
	ILITek_WtReg(0x15,0x55);		// Gamma correction settings
	ILITek_WtReg(0x16,0x17);		// Gamma correction settings
	ILITek_WtReg(0x17,0x62);		// Gamma correction settings							
	
#else	
//	ILITek_WtReg(0x04,0x00);		// Reset to all registers to default values
//	ILITek_WtReg(0x04,0x01);		// Normal operation
	
	ILITek_WtReg(0x06,0x0c);		// Bit[7:4] = 0x0 --> RGB Through 320x240
									// Bit[3:2] = 0x3 --> auto detection mode for NTSC/PAL
									// Bit[1:1] = 0x1 --> up-to-down scan
									// Bit[0:0] = 0x1 --> left-to-right scan										
									
	delay_loop(2000);												
	ILITek_WtReg(0x07,0xEE);		// turn off charge-pump
	delay_loop(4000);												
	ILITek_WtReg(0x07,0xEF);		// turn off charge-pump		
	delay_loop(40000);												
#endif	
}			
//=====================================================================
//=====================================================================
static UINT32 GetPLLOutputKhz(
	E_CLK eSysPll,
	UINT32 u32FinKHz
	)
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
//	sysprintf("PLL regster = 0x%x\n", u32PllCntlReg);	
//	sysprintf("NF = %d\n", NF);
//	sysprintf("NR = %d\n", NR);
//	sysprintf("NOr = %d\n", NO);
		
	u32Freq = u32FinKHz*NF/NR/NO;
//	sysprintf("PLL Freq = %d\n", u32Freq);
	return u32Freq;
}

static void BacklightControl(int OnOff)
{	
	// set GPD_0 to GPIO mode
	outpw(REG_GPDFUN, inpw(REG_GPDFUN) & ~MF_GPD0);
	
	// GPD[0] set OUTPUT mode  => control the backlight
	outpw(REG_GPIOD_OMD, inpw(REG_GPIOD_OMD) | 0x00000001);
	if(OnOff==TRUE) {
		// GPD[0] turn on the backlight
		outpw(REG_GPIOD_DOUT, inpw(REG_GPIOD_DOUT) | 0x00000001);
	} else {
		// GPD[0] diable backlight
		outpw(REG_GPIOD_DOUT, inpw(REG_GPIOD_DOUT) & ~0x00000001);
	}
}

static INT Clock_Control(void)
{
}

INT vpostLCMInit_GIANTPLUS_GPM1006D0(PLCDFORMATEX plcdformatex, UINT32 *pFramebuf)
{
	volatile S_DRVVPOST_SYNCLCM_WINDOW sWindow = {960,240,320};	
	volatile S_DRVVPOST_SYNCLCM_HTIMING sHTiming = {2,0xCA,(UINT8)0x3C};
//	volatile S_DRVVPOST_SYNCLCM_VTIMING sVTiming = {0x12,0x13,3};
	volatile S_DRVVPOST_SYNCLCM_VTIMING sVTiming = {0x11,0x12,2};
	volatile S_DRVVPOST_SYNCLCM_POLARITY sPolarity = {FALSE,FALSE,FALSE,TRUE};

	UINT32 nBytesPixel, u32PLLclk, u32ClockDivider;

	// VPOST clock control
	outpw(REG_AHBCLK, inpw(REG_AHBCLK) | VPOST_CKE | HCLK4_CKE);
	outpw(REG_AHBIPRST, inpw(REG_AHBIPRST) | VPOSTRST);
	delay_loop(2);			
	outpw(REG_AHBIPRST, inpw(REG_AHBIPRST) & ~VPOSTRST);	
	delay_loop(2);				
	
//	u32PLLclk = sysGetPLLOutputKhz(eSYS_UPLL, 27000);	// CLK_IN = 27 MHz
	u32PLLclk = GetPLLOutputKhz(eUPLL, 12000);			// CLK_IN = 12 MHz
	u32ClockDivider = u32PLLclk / 24000;
	
	u32ClockDivider--;
	outpw(REG_CLKDIV1, (inpw(REG_CLKDIV1) & ~VPOST_N0));							
	outpw(REG_CLKDIV1, (inpw(REG_CLKDIV1) & ~VPOST_N1) | ((u32ClockDivider & 0xFF) << 8));						
	outpw(REG_CLKDIV1, inpw(REG_CLKDIV1) & ~VPOST_S);
	outpw(REG_CLKDIV1, inpw(REG_CLKDIV1) | (3<<3));		// VPOST clock from UPLL		

	vpostVAStopTrigger();	

	// Enable VPOST function pins
	vpostSetDataBusPin(eDRVVPOST_DATA_8BITS);	
		  
	// LCD image source select
	vpostSetLCM_ImageSource(eDRVVPOST_FRAME_BUFFER);
	
	// configure LCD interface
	vpostSetLCM_TypeSelect(eDRVVPOST_SYNC);

	// configure LCD timing sync or async with TV timing	
	vpostsetLCM_TimingType(eDRVVPOST_ASYNC_TV);
	
    // Configure Serial LCD interface (8-bit data bus)
    vpostSetSerialSyncLCM_Interface(eDRVVPOST_SRGB_RGBTHROUGH);    
    
//	ILI_I9322_RGBThrough_NTSC_INIT();

    // set Horizontal scanning line timing for Syn type LCD 
    vpostSetSyncLCM_HTiming((S_DRVVPOST_SYNCLCM_HTIMING *)&sHTiming);

	// set Vertical scanning line timing for Syn type LCD   
    vpostSetSyncLCM_VTiming((S_DRVVPOST_SYNCLCM_VTIMING *)&sVTiming);
	
	// set both "active pixel per line" and "active lines per screen" for Syn type LCD   
	vpostSetSyncLCM_ImageWindow((S_DRVVPOST_SYNCLCM_WINDOW *)&sWindow);

  	// set Hsync/Vsync/Vden/Pclk poalrity
	vpostSetSyncLCM_SignalPolarity((S_DRVVPOST_SYNCLCM_POLARITY *)&sPolarity);
    
    // set frambuffer base address
    if(pFramebuf != NULL) {
		vpostAllocVABufferFromAP(pFramebuf);
	} else {
    	if( vpostAllocVABuffer(plcdformatex, nBytesPixel)==FALSE)
    		return ERR_NULL_BUF;
    }
	
	// set frame buffer data format
	vpostSetFrameBuffer_DataType(plcdformatex->ucVASrcFormat);
	
	vpostSetYUVEndianSelect(eDRVVPOST_YUV_LITTLE_ENDIAN);
	
	// enable LCD controller
	vpostVAStartTrigger();
	
//	BacklightControl(TRUE);			
	return 0;
}

INT32 vpostLCMDeinit_GIANTPLUS_GPM1006D0(VOID)
{
	vpostVAStopTrigger();
	vpostFreeVABuffer();
	outpw(REG_AHBCLK, inpw(REG_AHBCLK) & ~VPOST_CKE);	
	return 0;
}

VOID vpostEnaBacklight(void)
{
#define VPOST_ICE_DEBUG	
#ifndef VPOST_ICE_DEBUG	
	/* set BL_EN (GPE1) to High */
	outpw(REG_GPEFUN, inpw(REG_GPEFUN) & ~MF_GPE1);
	outpw(REG_GPIOE_OMD, inpw(REG_GPIOE_OMD) | 0x00000002);
	outpw(REG_GPIOE_DOUT, inpw(REG_GPIOE_DOUT) | 0x00000002);

	/* set BL_CTL (GPD0) to Low */
	outpw(REG_GPDFUN, inpw(REG_GPDFUN) & ~MF_GPD0);
	outpw(REG_GPIOD_OMD, inpw(REG_GPIOD_OMD) | 0x00000001);
	outpw(REG_GPIOD_DOUT, inpw(REG_GPIOD_DOUT) & ~0x00000001);
#endif
}

#endif    //__HAVE_GIANTPLUS_GPM1006D0__
