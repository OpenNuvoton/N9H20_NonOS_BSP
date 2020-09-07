/**************************************************************************//**
 * @file     Font.c
 * @version  V3.00
 * @brief    N9H20 series Font driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "N9H20_Font.h"
#include "N9H20_NVTFAT.h"
#if defined (__GNUC__)
UINT8 u8Font[] __attribute__((aligned (32))) =
{
    #include "Font.dat"
};
#else
__align(32) UINT8 u8Font[] =
{
	#include "Font.dat"
};
#endif
#define FONT_FILE_PATH	"X:\\Font.bin"
		

static UINT32 g_u32FrameBufAddr;

INT8 	path[128];
void
InitFont(
	S_DEMO_FONT* 	ptFont,
	UINT32 			u32FrameBufAddr	
)
{
	
	
	
	g_u32FrameBufAddr = u32FrameBufAddr;
			
	if ( ptFont->pu32FontFileTmp == NULL )
	{
#if 0	//Load from SD card¡@
		char szPath[128];
		char suPath[256];
		INT  nByte;
		sprintf(szPath, "X:\\Font.bin");	
		
		fsAsciiToUnicode(szPath, suPath, 1);

		g_i32FileHandle1 = fsOpenFile(suPath, 			//CHAR *suFileName,
								0, 		//CHAR *szAsciiName, 	
								O_RDONLY);	//UINT32 uFlag	
		
		g_i32FileSize = fsGetFileSize(g_i32FileHandle1);

		ptFont->pu32FontFileTmp = (PUINT32)malloc( g_i32FileSize + 32 );	/* Allocate buffer for font file */
		ptFont->pu32FontFile =
			(PUINT32)((UINT32)(((UINT32)ptFont->pu32FontFileTmp + 32) & ~32)/* | BIT31*/); /* Alignment to 32 */ 

		fsReadFile(g_i32FileHandle1,
							(PUINT8)ptFont->pu32FontFile,
							g_i32FileSize,
							&nByte);

		fsCloseFile( g_i32FileHandle1 );
#else	//Embedded in code 
		ptFont->pu32FontFile = (PUINT32)u8Font;
#endif		
	}

	ptFont->u32FontRectWidth = _FONT_RECT_WIDTH_;
	ptFont->u32FontRectHeight = _FONT_RECT_HEIGHT_;
	ptFont->u32FontOffset =ptFont->u32FontRectWidth * ptFont->u32FontRectHeight / 32;
	ptFont->u32FontStep = _FONT_OFFSET_;

	ptFont->u32FontOutputStride = _FONT_STRIDE_;


	ptFont->au16FontColor[0] = _FONT_COLOR_BG_;			//font background color in RGB555
	ptFont->au16FontColor[1] = _FONT_COLOR_;			//font color in RGB555
	ptFont->au16FontColor[2] = _FONT_COLOR_BORDER_;		//font border color in RGB555

	ptFont->u32FontInitDone = 1;
}

void
UnInitFont(S_DEMO_FONT* ptFont)
{
	ptFont->u32FontInitDone = 0;
#if 0//Load from SD card¡@
	if ( g_i32FileHandle1 != NULL )
	{
		free(ptFont->pu32FontFileTmp);
			ptFont->pu32FontFileTmp = NULL;
	}
#else//Embedded in code
	
#endif
}

#define DBG_PRINTF(...)	//printf
void Font_ClrFrameBuffer(UINT32 u32FrameBufAddr)
{
	//memset((UINT8*)((UINT32)u32FrameBufAddr), _FONT_COLOR_BG_, _LCM_WIDTH_*_LCM_HEIGHT_*2);
	UINT32 u32X, u32Y;
	UINT16* pu16FB = (PUINT16)u32FrameBufAddr;
	
	for(u32Y=0; u32Y < _LCM_HEIGHT_; u32Y=u32Y+1)
	{		
		for(u32X=0; u32X < _LCM_WIDTH_; u32X=u32X+1)
		{
			*pu16FB++ = _FONT_COLOR_BG_;			
		}
	}
}

void
DemoFont_PaintA(
	S_DEMO_FONT* ptFont,
	UINT32	u32x,
	UINT32	u32y,
	PCSTR	pszString
)
{
	UINT32	u32X, u32Y, u32FontIndex, u32Index, u32TempFontIndex, u32Counter, u32TempPixel, u32Index2;
	UINT16* pu16FB;

	u32X = u32x;
	u32Y = u32y;	
	if ( ptFont->u32FontInitDone == 0 )
	{
		return;
	}
	
	pu16FB = (PUINT16)g_u32FrameBufAddr;
	while ( *pszString != 0 )
	{
		u32FontIndex = *pszString++;
		DBG_PRINTF("Character= %c\n",u32FontIndex);
		if ( (u32FontIndex >= _FONT_ASCII_START_) && (u32FontIndex <= _FONT_ASCII_END_) )
		{
			u32FontIndex -= _FONT_ASCII_START_;
		}
		else
		{
			//out of ASCII range, ?=63, 63-32=31
			u32FontIndex = _FONT_ASCII_OUTRANGE_;
		}

		u32Counter = 0;
		u32FontIndex = ptFont->u32FontOffset * u32FontIndex;
		
		for ( u32Index = u32FontIndex; u32Index < (ptFont->u32FontOffset + u32FontIndex); u32Index++ )
		{
			DBG_PRINTF("Index= %d\n",u32Index);
			u32TempFontIndex = ptFont->pu32FontFile[u32Index];
			
			for ( u32Index2 = 0; u32Index2 < 32; u32Index2++ )
			{
				DBG_PRINTF("Index2= %d\n",u32Index2);
				u32TempPixel = (u32TempFontIndex >> u32Index2) & 0x00000001;
				if ( u32TempPixel != 0 )
				{				
					volatile UINT32 u32Addr;					
					u32Addr = u32y * ptFont->u32FontOutputStride + u32x;					
					pu16FB[u32Addr] =
						ptFont->au16FontColor[u32TempPixel];
				}	

				u32x++;
				u32Counter++;
				if ( u32Counter >= ptFont->u32FontRectWidth )
				{
					DBG_PRINTF("Next Line %d\n", u32y);
					u32Counter = 0;
					u32y++;
					u32x = u32X;
				}
			}
		}

		u32X += ptFont->u32FontStep;
		if ( u32X > (ptFont->u32FontStep * _FONT_LINE_) )
		{
			break;
		}
		u32x = u32X;
		u32y = u32Y;
	}
}

void
DemoFont_Rect(
	S_DEMO_FONT* ptFont,
	S_DEMO_RECT* ptRect
)
{
	UINT32 u32X, u32Y;
	UINT16* pu16FB;
	if ( ptFont->u32FontInitDone == 0 )
	{
		return;
	}
	
	for(u32Y=ptRect->u32StartY; u32Y<=ptRect->u32EndY; u32Y=u32Y+1)
	{		
		for(u32X=ptRect->u32StartX; u32X<=ptRect->u32EndX; u32X=u32X+1)
		{
			pu16FB = (PUINT16)(g_u32FrameBufAddr + (u32Y*_FONT_STRIDE_ +u32X)*_BPP_);
			*pu16FB = ptFont->au16FontColor[1];
		}
	}
}

void
DemoFont_RectClear(
	S_DEMO_FONT* ptFont,
	S_DEMO_RECT* ptRect
)
{
	UINT32 u32X, u32Y;
	UINT16* pu16FB;
	if ( ptFont->u32FontInitDone == 0 )
	{
		return;
	}
	
	for(u32Y=ptRect->u32StartY; u32Y<=ptRect->u32EndY; u32Y=u32Y+1)
	{		
		for(u32X=ptRect->u32StartX; u32X<=ptRect->u32EndX; u32X=u32X+1)
		{
			pu16FB = (PUINT16)(g_u32FrameBufAddr + (u32Y*_FONT_STRIDE_ +u32X)*_BPP_);
			*pu16FB = _FONT_COLOR_BG_;
		}
	}
}

void
DemoFont_Border(
	S_DEMO_FONT* ptFont,
	S_DEMO_RECT* ptRect,
	UINT32 		 u32Width	
)
{
	UINT32 u32X, u32Y;
	UINT32 u32Xws, u32Yws, u32Xwe, u32Ywe;
	UINT16* pu16FB;
	
	u32Xws = ptRect->u32StartX+u32Width;
	u32Yws = ptRect->u32StartY+u32Width;
	u32Xwe = ptRect->u32EndX-u32Width;
	u32Ywe = ptRect->u32EndY-u32Width;	
	if ( ptFont->u32FontInitDone == 0 )
	{
		return;
	}
	
	for(u32Y=ptRect->u32StartY; u32Y<=ptRect->u32EndY; u32Y=u32Y+1)
	{		
		for(u32X=ptRect->u32StartX; u32X<=ptRect->u32EndX; u32X=u32X+1)
		{		
			if( ((u32X>=u32Xws) && (u32X<=u32Xwe)) && ((u32Y>=u32Yws) &&(u32Y<=u32Ywe)) )
				continue;		
			pu16FB = (PUINT16)(g_u32FrameBufAddr + (u32Y*_FONT_STRIDE_ +u32X)*_BPP_);
			*pu16FB = ptFont->au16FontColor[1];
		}
	}
}
void
DemoFont_ChangeFontColor(
	S_DEMO_FONT* ptFont,
	UINT16	u16TRGB565
)
{
	ptFont->au16FontColor[1] = u16TRGB565;
}

UINT16
DemoFont_GetFontColor(
	S_DEMO_FONT* ptFont
)
{
	return ptFont->au16FontColor[1];
}
