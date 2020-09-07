/**************************************************************************//**
 * @file     Font_demo.c
 * @brief    Font sample application using Font library
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "N9H20.h"
#include "Font_demo.h"


extern S_DEMO_FONT s_sDemo_Font;

int font_x=0, font_y=16;
UINT32 u32SkipX;


#if 1
#define dbgprintf sysprintf
#else
#define dbgprintf(...)
#endif



#if defined (__GNUC__)
char pstrDisp[26][32] __attribute__((aligned(32))) = {
#else
__align(32) char pstrDisp[26][32] = {
#endif
 {"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"},
 {"BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"},
 {"CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"},
 {"DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"},
 {"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"},
 {"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"},
 {"GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG"},
 {"HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH"},
 {"IIIIIIIIIIIIIIIIIIIIIIIIIIIIIII"},
 {"JJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJ"},
 {"KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK"},
 {"LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL"},
 {"MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM"},
 {"NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN"},
 {"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO"},
 {"PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP"},
 {"QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ"},
 {"RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR"},
 {"SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS"},
 {"TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT"},
 {"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU"},
 {"VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV"},
 {"WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"},
 {"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"},
 {"YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY"},
 {"ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ"}
 };





/**********************************/
int main()
{
    DateTime_T ltime;
    WB_UART_T uart;

    int j, i;
    UINT32 wait_ticks, no, dispno, dispcolor, u32ExtFreq, u32PllOutKHz;

    //--- Reset SIC engine to make sure it under normal status.
    outp32(REG_AHBCLK, inp32(REG_AHBCLK) | (SIC_CKE | NAND_CKE | SD_CKE));
    outp32(REG_AHBIPRST, inp32(REG_AHBIPRST) | SICRST);     // SIC engine reset is avtive
    outp32(REG_AHBIPRST, inp32(REG_AHBIPRST) & ~SICRST);    // SIC engine reset is no active. Reset completed.

	u32ExtFreq = sysGetExternalClock();
	u32PllOutKHz = sysGetPLLOutputKhz(eSYS_UPLL, u32ExtFreq);
	sysUartPort(1);
	uart.uiFreq = u32ExtFreq*1000;
	uart.uiBaudrate = 115200;
	uart.uiDataBits = WB_DATA_BITS_8;
	uart.uiStopBits = WB_STOP_BITS_1;
	uart.uiParity = WB_PARITY_NONE;
	uart.uiRxTriggerLevel = LEVEL_1_BYTE;
	sysInitializeUART(&uart);

	sysprintf("PLL out frequency %d Khz\n", u32PllOutKHz);
	sysprintf("Switch clock end\n");


    sysSetSystemClock(eSYS_UPLL, 	//E_SYS_SRC_CLK eSrcClk,
    						192000,		  //UINT32 u32PllKHz,
    						192000,		  //UINT32 u32SysKHz,
    						192000,		  //UINT32 u32CpuKHz,
    						192000/2,	  //UINT32 u32HclkKHz,
    						192000/4);    //UINT32 u32ApbKHz

    /* enable cache */
    sysDisableCache();
    sysInvalidCache();
    sysEnableCache(CACHE_WRITE_BACK);

    /* configure Timer0 for FMI library */
    sysSetTimerReferenceClock(TIMER0, 12000000);
    sysStartTimer(TIMER0, 100, PERIODIC_MODE);

    ltime.year = 2011;
    ltime.mon  = 10;
    ltime.day  = 31;
    ltime.hour = 8;
    ltime.min  = 40;
    ltime.sec  = 0;
    sysSetLocalTime(ltime);

    Draw_Init();
    font_y = g_Font_Height;
    
    i= 0;
    while (1)
    {
        no=i%26;
        dispno = i % 3;
        if (dispno == 0 )
           dispcolor = COLOR_RGB16_RED;
        else if ( dispno == 1 )
           dispcolor = COLOR_RGB16_GREEN;
        else
           dispcolor = COLOR_RGB16_BLUE;      
		DemoFont_ChangeFontColor(&s_sDemo_Font, dispcolor);
		for (j= 0;j<8; j++)
		{
			DemoFont_PaintA(&s_sDemo_Font,				
				font_x,								
				font_y+ j*g_Font_Height,						
	    		pstrDisp[no]);	
	    }	    	
		wait_ticks = sysGetTicks(TIMER0);
		while( 1 )
		{
			if((sysGetTicks(TIMER0) - wait_ticks) > 200)
				break;    	
	    }
        DemoFont_ChangeFontColor(&s_sDemo_Font, COLOR_RGB16_BLACK);
		for (j= 0;j<8; j++)
		{
			DemoFont_PaintA(&s_sDemo_Font,				
				font_x,								
				font_y+ j*g_Font_Height,						
	    		pstrDisp[no]);	
	    }	    		    
	    i++;
    }	    
}
