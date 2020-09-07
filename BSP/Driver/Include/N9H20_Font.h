/**************************************************************************//**
 * @file     N9H20_Font.h
 * @version  V3.00
 * @brief    N9H20 series Font driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#ifndef	__FONT_H__
#define	__FONT_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include "wblib.h"

typedef	struct
{
	UINT32	volatile	u32FontRectWidth;
	UINT32	volatile	u32FontRectHeight;
	UINT32	volatile	u32FontOffset;
	UINT32	volatile	u32FontStep;
	UINT32	volatile	u32FontOutputStride;
	UINT32	volatile	u32FontInitDone;
	UINT32				u32FontFileSize;
	PUINT32	pu32FontFileTmp;
	PUINT32	pu32FontFile;
	UINT16	au16FontColor[3];
} S_DEMO_FONT;


typedef	struct
{
	UINT32	volatile	u32StartX;
	UINT32	volatile	u32StartY;
	UINT32	volatile	u32EndX;
	UINT32	volatile	u32EndY;
} S_DEMO_RECT;

/* Please choice a option */ 
//#define _DEMO_WQVGA_		/* 480x272 */
//#define _DEMO_QVGA_		/* 320x240 */
//#define _DEMO_VGA_		/* 640x480 */
//#define _DEMO_WVGA_		/* 800x480 */

#define _BPP_				2
#define _BORDER_WIDTH_		2			//2 Pixels		
#ifdef _DEMO_WQVGA_
	#define _FONT_STRIDE_	480
	#define _FONT_LINE_		47			//480/10 = 48,
	#define _LCM_WIDTH_		480
	#define _LCM_HEIGHT_	272
#endif
#ifdef _DEMO_QVGA_
	#define _FONT_STRIDE_	320
	#define _FONT_LINE_		31			//320/10 = 32,
	#define _LCM_WIDTH_		320
	#define _LCM_HEIGHT_	240
#endif
#ifdef _DEMO_VGA_
	#define _FONT_STRIDE_	640
	#define _FONT_LINE_		63			//640/10 = 64,
	#define _LCM_WIDTH_		640
	#define _LCM_HEIGHT_	480
#endif
#ifdef _DEMO_WVGA_
	#define _FONT_STRIDE_	800
	#define _FONT_LINE_		79			//800/10 = 80,
	#define _LCM_WIDTH_		800
	#define _LCM_HEIGHT_	480
#endif
#ifdef _DEMO_SVGA_
	#define _FONT_STRIDE_	800
	#define _FONT_LINE_		79			//800/10 = 80,
	#define _LCM_WIDTH_		800
	#define _LCM_HEIGHT_	600
#endif

#define _FONT_ASCII_START_		32
#define _FONT_ASCII_END_		126
#define _FONT_ASCII_OUTRANGE_	(63 - _FONT_ASCII_START_)
#define _FONT_RECT_WIDTH_		16
#define _FONT_RECT_HEIGHT_		22
#define _FONT_OFFSET_			10



#define _FONT_COLOR_BG_			0x0000
#define _FONT_COLOR_			0x7FFF
#define _FONT_COLOR_BORDER_		0x001F


#ifndef NULL
#define NULL 0
#endif

void InitFont(S_DEMO_FONT* ptFont, UINT32 u32FrameBufAddr);
void UnInitFont(S_DEMO_FONT* ptFont);
void DemoFont_PaintA(S_DEMO_FONT* ptFont, UINT32 u32x, UINT32 u32y, PCSTR pszString);
void DemoFont_Rect(S_DEMO_FONT* ptFont, S_DEMO_RECT* ptRect);
void DemoFont_RectClear(S_DEMO_FONT* ptFont, S_DEMO_RECT* ptRect);
void DemoFont_Border(S_DEMO_FONT* ptFont, S_DEMO_RECT* ptRect, UINT32 u32Width);
void Font_ClrFrameBuffer(UINT32 u32FrameBufAddr);
void DemoFont_ChangeFontColor(S_DEMO_FONT* ptFont, UINT16	u16TRGB565);
UINT16 DemoFont_GetFontColor(S_DEMO_FONT* ptFont);

#ifdef	__cplusplus
}
#endif

#endif	//__FONT_H__
