/**************************************************************************//**
 * @file     drawFont.c
 * @brief    N9H20 series Font driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "N9H20.h"
#include "writer.h"

#define	 LAST_LINE	11

UINT	g_Font_Height, g_Font_Width, g_Font_Step;

#if defined (__GNUC__)
    S_DEMO_FONT s_sDemo_Font __attribute__((aligned (32)));
    UINT16 FrameBuffer[_LCM_WIDTH_*_LCM_HEIGHT_] __attribute__((aligned (32)));
#else
    __align(32) S_DEMO_FONT s_sDemo_Font;
    __align(32) UINT16 FrameBuffer[_LCM_WIDTH_*_LCM_HEIGHT_];
#endif

#if 0
#define dbgprintf sysprintf
#else
#define dbgprintf(...)
#endif


//***********************************************************************************
// initVPost :
//			Call VPOST to initialize panel
//***********************************************************************************
void initVPost(PUINT16 fb)
{
	LCDFORMATEX lcdInfo;
	static BOOL bIsInitVpost=FALSE;
	if(bIsInitVpost==FALSE)
	{
		bIsInitVpost = TRUE;
		lcdInfo.ucVASrcFormat = DRVVPOST_FRAME_RGB565;
		lcdInfo.nScreenWidth = _LCM_WIDTH_;
		lcdInfo.nScreenHeight = _LCM_HEIGHT_;
		vpostLCMInit(&lcdInfo, (UINT32*)fb);

        /* If backlight control signal is different from nuvoton's demo board,
           please don't call this function and must implement another similar one to enable LCD backlight. */
        vpostEnaBacklight();
	}
}

//***********************************************************************************
// Draw_InitialBorder :
//			Draw the outside border
//***********************************************************************************
void Draw_InitialBorder(S_DEMO_FONT* ptFont)
{
	S_DEMO_RECT s_sDemo_Rect;
	s_sDemo_Rect.u32StartX =0;
	s_sDemo_Rect.u32StartY = 0;
	s_sDemo_Rect.u32EndX = _LCM_WIDTH_-1,
	s_sDemo_Rect.u32EndY = _LCM_HEIGHT_-1;
	DemoFont_Border(ptFont,
					&s_sDemo_Rect,
					2);
}

//***********************************************************************************
// Draw_Font :
//			Draw the test font items on panel
//			For example, below "Mount SD Card" string is draw by this function
//						Mount SD card :PASS
//***********************************************************************************
void Draw_Font(
	UINT16	u16RGB,
	S_DEMO_FONT* ptFont,
	UINT32	u32x,
	UINT32	u32y,
	PCSTR	pszString
)
{
	DemoFont_ChangeFontColor(&s_sDemo_Font, u16RGB);

	DemoFont_PaintA(&s_sDemo_Font,
			u32x,
			u32y,
			pszString);
}

//***********************************************************************************
// Draw_Status :
//			Draw the test font items status on panel
//			For example, below "PASS" string is draw by this function
//						Mount SD card :PASS
//***********************************************************************************
void Draw_Status(UINT32	u32x,UINT32	u32y,int Status)
{
	if (Status)
	{
		DemoFont_ChangeFontColor(&s_sDemo_Font, COLOR_RGB16_RED);
		DemoFont_PaintA(&s_sDemo_Font,
				u32x,
				u32y,
				"FAIL");
	}
	else
	{
		DemoFont_ChangeFontColor(&s_sDemo_Font, COLOR_RGB16_GREEN);
		DemoFont_PaintA(&s_sDemo_Font,
				u32x,
				u32y,
				"PASS");
	}
}


//***********************************************************************************
// Draw_Clear_Wait_Status :
//			It is used to clear waiting status.
//***********************************************************************************
void Draw_Clear_Wait_Status(UINT32 u32x, UINT32 u32y)
{
	Draw_Clear(u32x+2, u32y+1, u32x+g_Font_Step*3+2, u32y+g_Font_Height, COLOR_RGB16_BLACK);
}


//***********************************************************************************
// Draw_Wait_Status :
//			It is used to show waiting status.
//			The string of status is also toggled to highlight it.
//***********************************************************************************
void Draw_Wait_Status(UINT32 u32x, UINT32 u32y)
{
    static int Wait_Status = TRUE;
	Draw_Clear_Wait_Status(u32x, u32y);
	DemoFont_ChangeFontColor(&s_sDemo_Font, COLOR_RGB16_GREEN);
	switch (Wait_Status)
	{
	    case 0: DemoFont_PaintA(&s_sDemo_Font, u32x, u32y, " | ");  break;
	    case 1: DemoFont_PaintA(&s_sDemo_Font, u32x, u32y, " / ");  break;
	    case 2: DemoFont_PaintA(&s_sDemo_Font, u32x, u32y, "---");  break;
	    case 3: DemoFont_PaintA(&s_sDemo_Font, u32x, u32y, " \\ ");  break;
	}
	Wait_Status = (Wait_Status + 1) % 4;
}


//***********************************************************************************
// Draw_Clear :
//			Clear the specified Rectangle. It can be used to clear the display buffer
//			For example, it can be used to clear the current operation buffer
//***********************************************************************************
void Draw_Clear(int xStart, int yStart, int xEnd, int yEnd, UINT16 color)
{
	S_DEMO_RECT s_sRect;
	s_sRect.u32StartX = xStart;
	s_sRect.u32StartY = yStart;
	s_sRect.u32EndX = xEnd;
	s_sRect.u32EndY = yEnd;

	DemoFont_ChangeFontColor(&s_sDemo_Font, color);
	DemoFont_Rect(&s_sDemo_Font,&s_sRect);
}

//***********************************************************************************
// Draw_CurrentOperation :
//			It is used to show currnt operation with the return code.
//			Update at the bottom of center retangle
//***********************************************************************************
void Draw_CurrentOperation(PCSTR pszString, int Retcode)
{
	char Array1[64];

	Draw_Clear(2, _LCM_HEIGHT_-1-g_Font_Height*2, _LCM_WIDTH_-2, _LCM_HEIGHT_-5-g_Font_Height*1,  COLOR_RGB16_BLACK);

	DemoFont_ChangeFontColor(&s_sDemo_Font, COLOR_RGB16_WHITE);

	DemoFont_PaintA(&s_sDemo_Font,
					0,
					_LCM_HEIGHT_-1-g_Font_Height*2,
					pszString);
	if (Retcode < 0)
	{
		DemoFont_ChangeFontColor(&s_sDemo_Font, COLOR_RGB16_RED);
		sprintf(Array1, "Code:%x", Retcode);
		DemoFont_PaintA(&s_sDemo_Font,
					_LCM_WIDTH_ - 14 * g_Font_Step,
					_LCM_HEIGHT_-1-g_Font_Height*2,
					Array1);
	}
	else
	{
		DemoFont_ChangeFontColor(&s_sDemo_Font, COLOR_RGB16_GREEN);
		sprintf(Array1, "Code:%x", Retcode);
		DemoFont_PaintA(&s_sDemo_Font,
					_LCM_WIDTH_ - 14 * g_Font_Step,
					_LCM_HEIGHT_-1-g_Font_Height*2,
					Array1);
	}
}

//***********************************************************************************
// Draw_FinalStatus :
//			It is used to show final status of NandWriter.
//			The background of status is also toggled to highlight it.
//***********************************************************************************
void Draw_FinalStatus(int bIsAbort)
{
	char Array1[64];
	S_DEMO_RECT s_sRect;
	UINT16 u16RGB;
	UINT32 u32Tick;

	s_sRect.u32StartX = _LCM_WIDTH_ - 5 * g_Font_Step;
	s_sRect.u32StartY = 2;
	s_sRect.u32EndX = _LCM_WIDTH_-3;
	s_sRect.u32EndY = g_Font_Height-1;

	u16RGB = COLOR_RGB16_BLACK;

	u32Tick = sysGetTicks(0);

	do {
		if ((sysGetTicks(0) - u32Tick ) > 30)
		{
			if (u16RGB == COLOR_RGB16_BLACK)
				u16RGB = COLOR_RGB16_WHITE;
			else
				u16RGB = COLOR_RGB16_BLACK;

			DemoFont_ChangeFontColor(&s_sDemo_Font, u16RGB);
			DemoFont_Rect(&s_sDemo_Font,&s_sRect);

			if (bIsAbort == -1)
			{
				DemoFont_ChangeFontColor(&s_sDemo_Font, COLOR_RGB16_RED);

				sprintf(Array1, "UMRK");

				DemoFont_PaintA(&s_sDemo_Font,
						_LCM_WIDTH_ - 5 * g_Font_Step,
						0,
						Array1);
			}
			else if (bIsAbort)
			{
				DemoFont_ChangeFontColor(&s_sDemo_Font, COLOR_RGB16_RED);

				sprintf(Array1, "FAIL");

				DemoFont_PaintA(&s_sDemo_Font,
						_LCM_WIDTH_ - 5 * g_Font_Step,
						0,
						Array1);
			}
			else
			{
				DemoFont_ChangeFontColor(&s_sDemo_Font, COLOR_RGB16_GREEN);

				sprintf(Array1, "PASS");

				DemoFont_PaintA(&s_sDemo_Font,
						_LCM_WIDTH_ - 5 * g_Font_Step,
						0,
						Array1);
			}
			u32Tick = sysGetTicks(0);
		}
	} while(1);
}


//***********************************************************************************
// Draw_Init :
//			Initalize the VPOST, Font and prepared the related retangle for display
//***********************************************************************************
void Draw_Init(void)
{
	S_DEMO_RECT	s_sRect;
	char Array1[64];

	initVPost(FrameBuffer);
#if 0
	outp32(REG_GPDFUN, inp32(REG_GPDFUN) | 0x3FF);	// For ICE Debug if VPOST cause ICE disconnect
#endif

	InitFont(&s_sDemo_Font,(UINT32)FrameBuffer);

	// Font Width = 16, Height = 22
	g_Font_Width = s_sDemo_Font.u32FontRectWidth;
	g_Font_Height = s_sDemo_Font.u32FontRectHeight;
	g_Font_Step = s_sDemo_Font.u32FontStep;

	// Draw the outside boarder
	DemoFont_ChangeFontColor(&s_sDemo_Font, COLOR_RGB16_WHITE);
	Draw_InitialBorder(&s_sDemo_Font);

	// Draw the Boarder for "N9H20 NandWriter (..)"
	s_sRect.u32StartX =0;
	s_sRect.u32StartY = 0;
	s_sRect.u32EndX = _LCM_WIDTH_,
	s_sRect.u32EndY = g_Font_Height+1;
	DemoFont_Border(&s_sDemo_Font,
					&s_sRect,
                    2);

	sprintf(Array1, "N9H20 NandWriter (v%d.%d)",MAJOR_VERSION_NUM, MINOR_VERSION_NUM);
#if 0
	Draw_Font(COLOR_RGB16_WHITE, &s_sDemo_Font,
						(_LCM_WIDTH_ - 32*g_Font_Step)/2, // 32 Character for Array1
						0,
						Array1);
#else
	Draw_Font(COLOR_RGB16_WHITE, &s_sDemo_Font,
						0, // 32 Character for Array1
						0,
						Array1);
#endif

	// Only draw the Boarder for "Nand: Block * Page * PageSize"
	// But, Nand information is delayed to Nand initialization
	s_sRect.u32StartX =0;
	s_sRect.u32StartY = _LCM_HEIGHT_-1-g_Font_Height;
	s_sRect.u32EndX = _LCM_WIDTH_-1,
	s_sRect.u32EndY =  _LCM_HEIGHT_-1;
	DemoFont_Border(&s_sDemo_Font,
					&s_sRect,
					2);
}
