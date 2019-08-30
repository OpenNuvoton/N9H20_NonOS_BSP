/****************************************************************************
 *
 **************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wbio.h"
#include "wbtypes.h"
#include "N9H20.h"


UINT32 u32SecCnt;
UINT32 u32backup[10];

#if defined (__GNUC__) && !(__CC_ARM)
__attribute__ ((aligned (32))) UINT8 Vpost_Frame[]=
#else
__align(32) UINT8 Vpost_Frame[]=
#endif
{
	
#ifdef __LCD_800x600__
	#include "roof_800x600_RGB565.dat"		// for SVGA size test
#endif

#ifdef __LCD_800x480__
	#include "sea_800x480_RGB565.dat"		
//	#include "roof_800x480_RGB565.dat"				
#endif

#ifdef __LCD_720x480__
	#include "lake_720x480_RGB565.dat"		// for D1 size test
//	#include "roof_720x480_RGB565.dat"		// for D1 size test
#endif

#ifdef __LCD_640x480__
    #include "mountain_640x480_RGB565.dat"	// for VGA size test	
#endif

#ifdef __LCD_480x272__
	#include "river_480x272_RGB565.dat"
#endif

#ifdef __LCD_320x240__	
	#include "roof_320x240_RGB565.dat"
//	#include "roof_320x240_YUV422.dat"	
//	#include "roof_320x240_RGB888x.dat"		
#endif
};

#ifdef __LCD_320x240__
#define OSD_HSIZE       320
#define OSD_VSIZE       240
#endif

#ifdef __LCD_480x272__
#define OSD_HSIZE       480
#define OSD_VSIZE       272
#endif

#ifdef __LCD_800x480__
#define OSD_HSIZE       800
#define OSD_VSIZE       480
#endif

#if defined(__LCD_320x240__)||defined(__LCD_480x272__)
#define 	OPT_OSD_DEMO
#endif 

#ifdef OPT_OSD_DEMO
#if defined (__GNUC__) && !(__CC_ARM)
__attribute__ ((aligned (32))) UINT8 OSD_Frame_Buffer[]=
#else
__align (32) UINT8 OSD_Frame_Buffer[]=
#endif
{
#ifdef __LCD_320x240__
	#include "osd_320x240_RGB888x.dat"
#endif
#ifdef __LCD_480x272__
	#include "osd_480x272_RGB888x.dat"
#endif
};
#endif

LCDFORMATEX lcdFormat;
int volatile gTotalSectors_SD = 0, gTotalSectors_SM = 0;
void SmplVpost_OSD_Display(void);
//extern void vpostEnaBacklight(void);

#ifdef OPT_LCMTESTTOOL
extern int main_forLcmTestTool(void);
#endif


void initUART(void)
{
	WB_UART_T uart;
	UINT32 u32ExtFreq;
	
	u32ExtFreq = sysGetExternalClock();
	
	/* Please run the path if you want to use normal speed UART */
	sysUartPort(1);
	uart.uiFreq = u32ExtFreq*1000;
	uart.uiBaudrate = 115200;
	uart.uiDataBits = WB_DATA_BITS_8;
	uart.uiStopBits = WB_STOP_BITS_1;
	uart.uiParity = WB_PARITY_NONE;
	uart.uiRxTriggerLevel = LEVEL_1_BYTE;
	sysInitializeUART(&uart);
}

void initTimer(void)
{
	UINT32	u32ExtFreq;	    	    	
	UINT32 	u32Cke = inp32(REG_AHBCLK);

	/* init timer */	
	u32ExtFreq = sysGetExternalClock()*1000;    	/* Hz unit */	
	sysSetTimerReferenceClock (TIMER0, u32ExtFreq);	
	sysStartTimer(TIMER0, 100, PERIODIC_MODE);
	sysSetLocalInterrupt(ENABLE_IRQ);
}

int main(void)
{
	int tick;
	int osd_color_key, osd_pattern;
	
    initUART();
	initTimer();

#ifdef OPT_LCMTESTTOOL
	main_forLcmTestTool();
#else
	lcdFormat.ucVASrcFormat = DRVVPOST_FRAME_RGB565;
//	lcdFormat.ucVASrcFormat = DRVVPOST_FRAME_RGB888x;
	vpostLCMInit(&lcdFormat, (UINT32*)Vpost_Frame);
	
	/* If backlight control signal is different from nuvoton's demo board,
	   please don't call this function and must implement another similar one to enable LCD backlight. */
	vpostEnaBacklight();
#endif

    sysprintf("LCD initialized successfully.\n");	

#ifndef OPT_OSD_DEMO
	/* Since the limitation of bandwidth, N9H20 does not provide OSD overlay feature. */
	while(1);
#endif
	
    /* OSD display */
    sysprintf("Begin OSD position control !!!\n");	
    SmplVpost_OSD_Display();
    
	/* OSD enable/disable */
    while(0)
    {
  		tick = sysGetTicks(TIMER0);
		while((sysGetTicks(TIMER0)-tick) < 100);
        vpostSetOSD_Disable();

  		tick = sysGetTicks(TIMER0);
        while((sysGetTicks(TIMER0)-tick) < 100);
        vpostSetOSD_Enable();
    }
	
	/* OSD color key setting */
	vpostSetOSD_Transparent_Enable();
	osd_pattern = 0;
    while(1)
    {
  		tick = sysGetTicks(TIMER0);
		while((sysGetTicks(TIMER0)-tick) < 100);
		switch (osd_pattern)
		{
			case 0:
				osd_color_key = 0x00FFFFFF;		// white
				break;
			case 1:				
				osd_color_key = 0x00FF0000;		// red
				break;
			case 2:
				osd_color_key = 0x0000FF00;		// green
				break;
			case 3:
				osd_color_key = 0x000000FF;		// blue
				break;
			case 4:
				osd_color_key = 0x00FFFF00;		// yellow
				break;
			case 5:
			default:
				osd_color_key = 0x00FF00FF;		// pink
				break;
		}
		osd_pattern++;
		osd_pattern %= 6;
        vpostSetOSD_Transparent(eDRVVPOST_OSD_TRANSPARENT_RGB888, osd_color_key);
    }
}

#ifdef OPT_OSD_DEMO
void SmplVpost_OSD_Display(void)
{
	S_DRVVPOST_OSD_SIZE sSize;
	S_DRVVPOST_OSD_POS sPos;
	S_DRVVPOST_OSD sOSD, *psOSD;

	psOSD = &sOSD;
	
	/* set OSD position */
	sPos.u16VStart_1st = 0; 						
	sPos.u16VEnd_1st = OSD_VSIZE-1; 							
	sPos.u16VOffset_2nd = 0; 						
	sPos.u16HStart_1st = 0; 						
	sPos.u16HEnd_1st = OSD_HSIZE-1; 							
	sPos.u16HOffset_2nd = 0;
	psOSD->psPos = &sPos;

	/* set OSD size */
	sSize.u16HSize = OSD_HSIZE; 						
	sSize.u16VSize = OSD_VSIZE; 
	psOSD->psSize = &sSize;	

	/* set OSD tyoe */
//	psOSD->eType = eDRVVPOST_OSD_RGB565;
	psOSD->eType = eDRVVPOST_OSD_RGB888x;
	
	/* configure OSD */
	vpostSetOSD_DataType(psOSD->eType);
	vpostSetOSD_BaseAddress((UINT32)(UINT16*) OSD_Frame_Buffer);
	vpostSetOSD_Size(psOSD->psSize);
	vpostSetOSD_Pos(psOSD->psPos);	
	vpostSetOSD_Enable();	    	
}
#endif
