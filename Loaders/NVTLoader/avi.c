/**************************************************************************//**
 * @file     avi.c
 * @brief    APIs support to play AVI file 
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
/****************************************************************************
*                                                                           *
* Copyright (c) 2009 Nuvoton Tech. Corp. All rights reserved.               *
*                                                                           *
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "N9H20.h"
#include "nvtloader.h"

#if defined(N9H20K3) || defined(N9H20K5)
static int _complete = 0;
static int _offset = 0;
static int _fd;
#endif

extern unsigned char kbuf[CP_SIZE];
extern BOOL bIsIceMode;
void initVPost(unsigned char*);


LCDFORMATEX lcdInfo;

static volatile BOOL bEscapeKeyPress=FALSE;
static volatile BOOL bSetVolume=FALSE;
UINT32 u16Volume = 0x1F;	// 60*63/100=; Linux driver default = 60 
// use callback function of AVI player to load kernel
#if defined(N9H20K3) || defined(N9H20K5)
void loadKernel(AVI_INFO_T *aviInfo)
{
	int bytes, result;

	if(bSetVolume==FALSE)
	{
		aviSetPlayVolume(u16Volume);
		bSetVolume = TRUE;
	}
	
	
	if(bEscapeKeyPress==TRUE)	
		return;
	//sysprintf("\n");
	if(!_complete) {
		if(_offset < CP_SIZE)
		{//1th, keep the original vector table in  SDRAM. 			
			result = fsReadFile(_fd, (kbuf + _offset), (CP_SIZE - _offset), &bytes);
		}	
		else
		{//2nd, 3rd, .... Copy the kernel content to address 16K, 32K, 	
			result = fsReadFile(_fd, (UINT8 *)_offset, CP_SIZE, &bytes);
		}	
		if(result == FS_OK)
			_offset += bytes;
		else
			_complete = 1;
	}
	if(bEscapeKeyPress==FALSE)
	{
#ifdef __CC__	
		result = 0;
#else
		result = kpi_read(KPI_NONBLOCK);	
#endif		
#if defined(__DEMO_MC__) || defined(__WIFI__)
		if(result == 8)	//Home key  
#elif defined(__SD2_DEMO__)
		if(result == 4)
#else
		if(result == 16)	//Home key
#endif		
		{//Stop AVI playback
			bEscapeKeyPress = TRUE;
			sysprintf("Key pressed %d\n", result);
			
			aviStopPlayFile();	
			
		}		
	}	
	return;
}

#endif

void loadKernelCont(int fd, int offset)
{	
	int bytes, result;
	
	while(1) {
		if(offset < CP_SIZE)		
			result = fsReadFile(fd, (kbuf + offset), (CP_SIZE - offset), &bytes);
		else
			result = fsReadFile(fd, (UINT8 *)offset, CP_SIZE, &bytes);	
		if(result == FS_OK)
			offset += bytes;
		else
			return;	
	}
}

void lcmFill2Dark(unsigned char* fb)
{
	lcdInfo.ucVASrcFormat = DRVVPOST_FRAME_YCBYCR;
	if(lcdInfo.ucVASrcFormat == DRVVPOST_FRAME_YCBYCR)
	{	
		UINT32 i;
		UINT32* ptBufAddr=(UINT32*)((UINT32)fb | 0x80000000);
		for(i=0;i<(PANEL_WIDTH * PANEL_HEIGHT * PANEL_BPP);i=i+4)
		{
			outpw(ptBufAddr, 0x80108010);
			ptBufAddr = ptBufAddr+1;		//2 pixels 
		}
	}
	else if(lcdInfo.ucVASrcFormat == DRVVPOST_FRAME_RGB565)
	{
		memset((char*)fb, 0, PANEL_WIDTH * PANEL_HEIGHT * PANEL_BPP);
	} 
}
/*=====================================================
	New Demo board supports amplifier control. 
##demo board	
	HP_DET: GPB2
			1: Earphone  detection
			0: No erephone detection
	SPK_EN: GPB3:
			1: Enable amplifier for speaker
			0: Disable amplifier 
##WINTEK LCM
	HP_DET: GPB2
			1: Earphone  detection
			0: No erephone detection
	SPK_EN: GPB3:
			1: Enable amplifier for speaker
			0: Disable amplifier 	
##128 package for VideoIN+8bit LCM
	HP_DET: GPE0
			1: Earphone  detection
			0: No erephone detection
	SPK_EN: GPE1:
			1: Enable amplifier for speaker
			0: Disable amplifier 			

##INCHu' demo board (SD2)
	HP_DET: GPD3
			1: No Earphone detection
			0: Earphone  detection 
	
	SPK_EN:	GPD4			
			1: Enable amplifier for speaker
			0: Disable amplifier 
										
=====================================================*/
void ampEnable(void)
{
	UINT16 u16HearPhone;		

	
#if defined(__IXL_WINTEK__)	
	gpio_setportdir(GPIO_PORTB, 0x04, 0x00);			//GPIOB2 input mode 
	gpio_readport(GPIO_PORTB, &u16HearPhone);
	if( (u16HearPhone & 0x04) ==0x00 )					
#elif ((defined(__GWMT9360A__) || defined(__GWMT9615A__)) && !defined(__SD2_DEMO__))  
	outp32(REG_GPEFUN, inp32(REG_GPEFUN) & (~(MF_GPE0 | MF_GPE1)));
	gpio_setportdir(GPIO_PORTE, 0x01, 0x00);			//GPIOE0 input mode 
	gpio_readport(GPIO_PORTE, &u16HearPhone);			//GPIOE0 for detect
	if( (u16HearPhone & 0x01) == 0x00)					//128 package for VideoIn and 8 bit LCM	
#elif defined(__SD2_DEMO__)
	if(bIsIceMode==TRUE)
		return; 	
	outp32(REG_GPDFUN, inp32(REG_GPDFUN) & (~(MF_GPD3 | MF_GPD4)));
	gpio_setportdir(GPIO_PORTD, 0x08, 0x00);			//GPIOD3 input mode 
	gpio_readport(GPIO_PORTD, &u16HearPhone);			//GPIOE0 for detect
	if( (u16HearPhone & 0x08) == 0x08 )					//DEMO board
#else
	gpio_setportdir(GPIO_PORTB, 0x04, 0x00);			//GPIOB2 input mode 
	gpio_readport(GPIO_PORTB, &u16HearPhone);
	if( (u16HearPhone & 0x04) == 0x00 )					//DEMO board 
#endif	
	{//Earphone Plug out
		//sysprintf("Earphone out\n");
	#if defined(__SD2_DEMO__)		
		gpio_setportval(GPIO_PORTD, 0x10, 0x10);        //GPIOD4 high to enable Amplifier 
		gpio_setportpull(GPIO_PORTD, 0x10, 0x10);		//GPIOD4 pull high
		gpio_setportdir(GPIO_PORTD, 0x010, 0x10);		//GPIOD4 output mode	
  	#elif ((defined(__GWMT9360A__) || defined(__GWMT9615A__)) && !defined(__SD2_DEMO__))
  		gpio_setportval(GPIO_PORTE, 0x02, 0x02);        	//GPIOE1 high to enable Amplifier 
		gpio_setportpull(GPIO_PORTE, 0x02, 0x02);		//GPIOE1 pull high
		gpio_setportdir(GPIO_PORTE, 0x02, 0x02);		//GPIOE1 output mode	
  	#else
		gpio_setportval(GPIO_PORTB, 0x08, 0x08);        	//GPIOB3 high to enable Amplifier 
		gpio_setportpull(GPIO_PORTB, 0x08, 0x08);		//GPIOB3 pull high
		gpio_setportdir(GPIO_PORTB, 0x08, 0x08);		//GPIOB3 output mode
	#endif	
	}
	else
	{
		//sysprintf("Earphone in\n");
	#if defined(__SD2_DEMO__)		
  		gpio_setportval(GPIO_PORTD, 0x10, 0x00);        //GPIOD4 low to disable Amplifier 
		gpio_setportpull(GPIO_PORTD, 0x10, 0x00);		//GPIOD4 don't pull high
		gpio_setportdir(GPIO_PORTD, 0x10, 0x10);		//GPIOD4 output mode 	
	#elif ((defined(__GWMT9360A__) || defined(__GWMT9615A__)) && !defined(__SD2_DEMO__)) 
  		gpio_setportval(GPIO_PORTE, 0x02, 0x00);        	//GPIOE1 low to disable Amplifier 
		gpio_setportpull(GPIO_PORTE, 0x02, 0x00);		//GPIOE1 don't pull high
		gpio_setportdir(GPIO_PORTE, 0x02, 0x02);		//GPIOE1 output mode
  	#else
		gpio_setportval(GPIO_PORTB, 0x08, 0x00);        	//GPIOB3 low to disable Amplifier 
		gpio_setportpull(GPIO_PORTB, 0x08, 0x00);		//GPIOB3 don't pull high
		gpio_setportdir(GPIO_PORTB, 0x08, 0x08);		//GPIOB3 output mode
	#endif	
	}
}

void startAmpEnableCheck(void)
{
	UINT32 u32ExtFreq;
	
	ampEnable();	
	
	u32ExtFreq = sysGetExternalClock()*1000;
	DBG_PRINTF("Timer 0 Test...\n");	
	sysSetTimerReferenceClock(TIMER0, u32ExtFreq); 			//External Crystal	
	sysStartTimer(TIMER0, 100, PERIODIC_MODE);				/* 100 ticks/per sec ==> 1tick/10ms */	
	sysSetTimerEvent(TIMER0, 10, (PVOID)ampEnable);	/* 10 ticks = 100ms call back */
}	
void ampDisable(void)
{
#if defined(__SD2_DEMO__)
	if(bIsIceMode==TRUE)
		gpio_setportval(GPIO_PORTD, 0x10, 0x00);        	//GPIOD4 low to disable Amplifier
#elif ((defined(__GWMT9360A__) || defined(__GWMT9615A__)) && !defined(__SD2_DEMO__)) 
	gpio_setportval(GPIO_PORTE, 0x02, 0x00);        	//GPIOE1 high to disable Amplifier 
#else
	gpio_setportval(GPIO_PORTB, 0x08, 0x00);        	//GPIOB2 low to disable Amplifier 
#endif	
}
/*
	In Demo board, change GPIOD1 from pull high 10K to pull low 10K.
	Defaule GPIOD1 is low (Turn off back light).
	After initialize VPOST and delay a period for panel show first frame ready.   	
	Turn on back light. (GPIOD1=1)
*/
void backLightEnable(void)
{
	/* 1. If backlight control signal is different from nuvoton¡¦s demo board,
              please don't call this function and must implement another similar one to enable LCD backlight. */
	vpostEnaBacklight();
}

void AudioChanelControl(void)
{
#if defined(__IXL_WINTEK__)
	outp32(REG_GPAFUN, inp32(REG_GPAFUN) & (~MF_GPA6));		
	gpio_setportval(GPIO_PORTA, 0x40, 0x00);			//GPIOA6 output low 
	gpio_setportpull(GPIO_PORTA, 0x40, 0x00);			//GPIOA6 pull high disable 
	gpio_setportdir(GPIO_PORTA, 0x40, 0x40);			//GPIOA6 output mode
#endif
#if defined(__GWMT9360A__) || defined(__GWMT9615A__)
	
#endif
}
static UINT32 bIsInitVpost=FALSE;
void initVPostShowLogo(void)
{
	if(bIsInitVpost==FALSE)
	{
		bIsInitVpost = TRUE;
		//lcdInfo.ucVASrcFormat = DRVVPOST_FRAME_YCBYCR;	
		lcdInfo.ucVASrcFormat = DRVVPOST_FRAME_RGB565;	
		lcdInfo.nScreenWidth = PANEL_WIDTH;	
		lcdInfo.nScreenHeight = PANEL_HEIGHT;
		vpostLCMInit(&lcdInfo, (UINT32*)FB_ADDR);
		/* 1. If backlight control signal is different from nuvoton¡¦s demo board,
           please don't call this function and must implement another similar one to enable LCD backlight. */
        vpostEnaBacklight();
	}	
}

/*
void moveFrameBufferData(PUINT8 pu8Src, UINT32 u32Len)
{
	UINT32 u32Idx;
	PUINT32 pu32Src, pu32Dst;
	pu32Src = (PUINT32)pu8Src;
	pu32Dst = (PUINT32)0x500000;
	printf("Src addr = 0x%x\n", pu32Src);
	printf("Dst addr = 0x%x\n", pu32Dst);
	printf("Len = 0x%x\n", u32Len);
	for(u32Idx=0; u32Idx<u32Len; u32Idx= u32Idx+2)
	{
		*pu32Dst = *pu32Src;
		pu32Src++;
		pu32Dst++;
	}
}
*/
#if defined(N9H20K3) || defined(N9H20K5)
void playAni(int kfd, char* pcString)
{
	char aniPath[64];
		
	lcdInfo.ucVASrcFormat = DRVVPOST_FRAME_YCBYCR;	
	//cdInfo.ucVASrcFormat = DRVVPOST_FRAME_RGB565;	
	
//	spuOpen(eDRVSPU_FREQ_8000);
//	spuDacOn(1);
//	spuIoctl(SPU_IOCTL_SET_VOLUME, 0x1F, 0x1F);
		
#if defined(__LCM_800x600__) || defined(__LCM_800x480__)
	outp32(REG_GPDFUN, inp32(REG_GPDFUN) & (~MF_GPD0));		//!!!! Remember switch GPIOD 0 to GPIO. It will cause the ICE crash. 
	gpio_setportval(GPIO_PORTD, 0x01, 0x01);			//GPIOD0 output mode 
	gpio_setportpull(GPIO_PORTD, 0x01, 0x01);			//GPIOD0 pull high
	gpio_setportdir(GPIO_PORTD, 0x01, 0x01);			//GPIOD0 output mode
#endif		
	fsAsciiToUnicode(pcString, aniPath, TRUE);
	
	startAmpEnableCheck();	
	_fd = kfd; // let callback function know the file descriptor
	bEscapeKeyPress = FALSE;
   	if (aviPlayFile(aniPath, 0, 0, DIRECT_RGB565, (kfd > 0) ? loadKernel : NULL) < 0)
		DBG_PRINTF("Playback failed\n");
	else
		DBG_PRINTF("Playback done.\n");
	ampDisable();
	
	//moveFrameBufferData(fb, PANEL_WIDTH * PANEL_HEIGHT * PANEL_BPP);
	
	// If movie is too short for callback function to load kernel to SDRAM, keep working...
	if(kfd > 0 && _complete == 0) {
		loadKernelCont(kfd, _offset);
	}
	
	if(kfd > 0) {
		fsCloseFile(_fd);
	}
	
	return;

}
#endif
