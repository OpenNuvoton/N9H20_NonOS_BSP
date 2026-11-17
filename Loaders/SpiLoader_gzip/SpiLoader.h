/**************************************************************************//**
 * @file     spiloader.h
 * @brief    Spiloader header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
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

#if defined(_N9H20K6_)
#define FB_ADDR		0xC00000
#endif
#if defined(_N9H20K5_) || defined(_N9H20K3_)
#define FB_ADDR		0x500000
#endif
#if defined(_N9H20K1_) 
#define FB_ADDR		0x190000	/* N9H20K1 don't support logo due to conflict with loader's heap area. But still define it*/
#endif
