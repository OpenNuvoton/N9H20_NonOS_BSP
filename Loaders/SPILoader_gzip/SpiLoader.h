/****************************************************************************
 * @file     SpiLoader.h
 * @version  V1.00
 * $Revision: 4 $
 * $Date: 18/04/25 11:43a $
 * @brief    SpiLoader_gzip header file
 *
 * @note
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#define IMAGE_TYPE_DATA    0
#define IMAGE_TYPE_EXE     1
#define IMAGE_TYPE_ROMFS   2
#define IMAGE_TYPE_SYSTEM  3
#define IMAGE_TYPE_LOGO    4

VOID SPI_OpenSPI(VOID);
VOID SPI_CloseSPI(VOID);
int SPIReadFast(BOOL bEDMAread, UINT32 addr, UINT32 len, UINT32 *buf);

/* Start for option for VPOST frame buffer */
#if defined(__TV__)
    #ifdef __TV_QVGA__ 
        #define PANEL_WIDTH     320
        #define PANEL_HEIGHT    240
    #else
        #define PANEL_WIDTH     640
        #define PANEL_HEIGHT    480
    #endif
#elif defined( __LCM_800x600__) 
    #define PANEL_WIDTH     800
    #define PANEL_HEIGHT    600
#elif defined( __LCM_480x272__)
    #define PANEL_WIDTH     480
    #define PANEL_HEIGHT    272
#elif defined( __LCM_800x480__)
    #define PANEL_WIDTH     800
    #define PANEL_HEIGHT    480
#elif defined( __LCM_QVGA__)
    #define PANEL_WIDTH     320
    #define PANEL_HEIGHT    240
#else 
    #define PANEL_WIDTH     480
    #define PANEL_HEIGHT    272
#endif

#define FB_ADDR  0x500000
