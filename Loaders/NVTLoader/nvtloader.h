/**************************************************************************//**
 * @file     nvtloader.h
 * @version  V3.00
 * @brief    N9H20 series NVTLoader header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#define KERNEL_PATH_SD 	"x:\\conprog.bin"
#define MOVIE_PATH_SD 		"x:\\movie.avi"
#define VOLUME_PATH_SD	"x:\\volume.cfg"

#define KERNEL_PATH 		"c:\\conprog.bin"
#define MOVIE_PATH 		"c:\\movie.avi"
#define VOLUME_PATH		"c:\\volume.cfg"

#define CP_SIZE 16 * 1024
//#define __BAT_DET__

/* PLL clock option */
//#define __CLK_CTL__			/* Clock change in NVT-Loader */ 
#ifdef __CLK_CTL__
	/* Clock Skew */ 
	#define E_CLKSKEW	0x00888800
		
	/* DDR timing option */
	#define __DDR_6__
	//#define __DDR_75__
	
	/* PLL setting */
	#define __UPLL_240__
	//#define __UPLL_192__
	//#define __UPLL_288__
#endif

/* Start for option for VPOST frame buffer */
#if defined(__TV__)
	#ifdef __TV_QVGA__ 
	#define PANEL_WIDTH		320
	#define PANEL_HEIGHT		240
	#define LOW_BATTERY_LEVEL	(3.6)
	#else
	#define PANEL_WIDTH		640
	#define PANEL_HEIGHT		480
	#define LOW_BATTERY_LEVEL	(3.6)
	#endif
#elif defined( __LCM_800x600__) 
	#define PANEL_WIDTH		800
	#define PANEL_HEIGHT		600
	#define LOW_BATTERY_LEVEL	(3.6)
#elif defined( __LCM_480x272__)
	#define PANEL_WIDTH		480
	#define PANEL_HEIGHT		272
	#define LOW_BATTERY_LEVEL	(3.5)
#elif defined( __LCM_800x480__)
	#define PANEL_WIDTH		800
	#define PANEL_HEIGHT		480
	#define LOW_BATTERY_LEVEL	(3.6)
#elif defined( __LCM_QVGA__)
	#define PANEL_WIDTH		320
	#define PANEL_HEIGHT		240
	#define LOW_BATTERY_LEVEL	(3.5)
#elif defined( __LCM_128x64__)
	#define PANEL_WIDTH		128
	#define PANEL_HEIGHT		64	
	#define LOW_BATTERY_LEVEL	(3.5)
#else 	
	#define PANEL_WIDTH		480
	#define PANEL_HEIGHT		272
	#define LOW_BATTERY_LEVEL	(3.5)
#endif
/* End for option for VPOST frame buffer */

#define PANEL_BPP		2
#if defined(N9H20K5) || defined(N9H20K3)
#define FB_ADDR		0x500000
#endif
#if defined(N9H20K1) 
#define FB_ADDR		0x190000	/* N9H20K1 don't support logo due to conflict with loader's heap area. But still define it*/
#endif

#ifdef __DEBUG__
#define DBG_PRINTF		sysprintf
#else
#define DBG_PRINTF(...)		
#endif
 
 
 /* Turn on the optional. Back light enable */
 /* Turn off the optional, ICE can connect to */
 /* Default Demo Board  GPD1 keep pull high */
 /* 								*/ 
//#define __BACKLIGHT__
 

/* NAND1-1 Size */
#define NAND1_1_SIZE	 32	/* MB unit */
#define SD1_1_SIZE	 	128 /* MB unit */ 


void BatteryDetection(BOOL bIsExtraPower);

