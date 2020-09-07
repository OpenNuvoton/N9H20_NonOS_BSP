/**************************************************************************//**
 * @file     Font_demo.h
 * @version  V3.00
 * @brief    N9H20 series Font sample header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#define MAJOR_VERSION_NUM   1
#define MINOR_VERSION_NUM   3

extern UINT g_Font_Height, g_Font_Width, g_Font_Step;

#define COLOR_RGB16_RED     0xF800
#define COLOR_RGB16_WHITE   0xFFFF
#define COLOR_RGB16_GREEN   0x07E0
#define	COLOR_RGB16_BLUE    0x001F
#define COLOR_RGB16_BLACK   0x0000


void Draw_Font(UINT16 u16RGB,S_DEMO_FONT* ptFont,UINT32 u32x,UINT32 u32y,PCSTR  pszString);
void Draw_Status(UINT32 u32x,UINT32 u32y,int Status);
void Draw_Clear(int xStart, int yStart, int xEnd, int yEnd, UINT16 color);
void Draw_CurrentOperation(PCSTR pszString, int Retcode);
void Draw_FinalStatus(int bIsAbort);
void Draw_Init(void);
void Draw_Wait_Status(UINT32 u32x, UINT32 u32y);
void Draw_Clear_Wait_Status(UINT32 u32x, UINT32 u32y);

//int ProcessINI(char *fileName);
