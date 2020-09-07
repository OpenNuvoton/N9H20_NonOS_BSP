/**************************************************************************//**
 * @file     Demo_AVI.c
 * @brief    Demo how to play AVI file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "N9H20.h"


#if (N9H20K1)
#define VPOST_FRAME_BUFSZ		(320*240*2)
#else
#define VPOST_FRAME_BUFSZ		(800*480*2)
#endif

#if defined (__GNUC__)
UINT8  _VpostFrameBufferPool[VPOST_FRAME_BUFSZ] __attribute__((aligned (256)));
#else
static __align(256) UINT8  _VpostFrameBufferPool[VPOST_FRAME_BUFSZ];
#endif
static UINT8   *_VpostFrameBuffer;


void  avi_play_control(AVI_INFO_T *aviInfo)
{
	static INT	last_time;
	int    frame_rate;
	
	if (aviInfo->uPlayCurTimePos != 0)
		frame_rate = ((aviInfo->uVidFramesPlayed - aviInfo->uVidFramesSkipped) * 100) / aviInfo->uPlayCurTimePos;
	
	if (aviInfo->uPlayCurTimePos - last_time > 100)
	{
		sysprintf("%02d:%02d / %02d:%02d  Vid fps: %d / %d\n", 
			aviInfo->uPlayCurTimePos / 6000, (aviInfo->uPlayCurTimePos / 100) % 60,
			aviInfo->uMovieLength / 6000, (aviInfo->uMovieLength / 100) % 60,
			frame_rate, aviInfo->uVideoFrameRate);
		last_time = aviInfo->uPlayCurTimePos;
	}
}


int main()
{
  WB_UART_T 	uart;
	LCDFORMATEX lcdformatex;
	CHAR		suFileName[128];
	INT			nStatus;

	
	/* CACHE_ON	*/
	sysEnableCache(CACHE_WRITE_BACK);

	/*-----------------------------------------------------------------------*/
	/*  CPU/HCLK/APB:  192/96/48                                             */
	/*-----------------------------------------------------------------------*/
	sysSetSystemClock(eSYS_UPLL, 	//E_SYS_SRC_CLK eSrcClk,	
									192000,		//UINT32 u32PllKHz, 	
									192000,		//UINT32 u32SysKHz,
									192000,		//UINT32 u32CpuKHz,
									192000/2,		//UINT32 u32HclkKHz,
									  192000/4);		//UINT32 u32ApbKHz

	/*-----------------------------------------------------------------------*/
	/*  Init UART, N,8,1, 115200                                             */
	/*-----------------------------------------------------------------------*/
	sysUartPort(1);
	uart.uiFreq = 12000000;					//use XIN clock
  uart.uiBaudrate = 115200;
  uart.uiDataBits = WB_DATA_BITS_8;
  uart.uiStopBits = WB_STOP_BITS_1;
  uart.uiParity = WB_PARITY_NONE;
	uart.uiRxTriggerLevel = LEVEL_1_BYTE;
  sysInitializeUART(&uart);
	sysprintf("UART initialized.\n");

	_VpostFrameBuffer = (UINT8 *)((UINT32)_VpostFrameBufferPool | 0x80000000);

	/*-----------------------------------------------------------------------*/
	/*  Init timer                                                           */
	/*-----------------------------------------------------------------------*/
	sysSetTimerReferenceClock (TIMER0, 12000000);
	sysStartTimer(TIMER0, 100, PERIODIC_MODE);

	/*-----------------------------------------------------------------------*/
	/*  Init FAT file system                                                 */
	/*-----------------------------------------------------------------------*/
	sysprintf("fsInitFileSystem.\n");
	fsInitFileSystem();

	/*-----------------------------------------------------------------------*/
	/*  Init SD card                                                         */
	/*-----------------------------------------------------------------------*/
	sicIoctl(SIC_SET_CLOCK, 200000, 0, 0);
	sicOpen();
	sysprintf("total sectors (%x)\n", sicSdOpen0());
	
	spuOpen(eDRVSPU_FREQ_8000);
	spuDacOn(1);

#if 0
	/*-----------------------------------------------------------------------*/
	/*                                                                       */
	/*  Direct RGB555 AVI playback 	               							 */
	/*                                                                       */
	/*-----------------------------------------------------------------------*/
	lcdformatex.ucVASrcFormat = DRVVPOST_FRAME_RGB555;
  vpostLCMInit(&lcdformatex, (UINT32 *)_VpostFrameBuffer);

	/* If backlight control signal is different from nuvoton's demo board,
	   please don't call this function and must implement another similar one to enable LCD backlight. */
	vpostEnaBacklight();

	fsAsciiToUnicode("c:\\movie.avi", suFileName, TRUE);	

  if (aviPlayFile(suFileName, 0, 0, DIRECT_RGB555, avi_play_control) < 0)
		sysprintf("Playback failed, code = %x\n", nStatus);
	else
		sysprintf("Playback done.\n");

#endif
#if 1
	/*-----------------------------------------------------------------------*/
	/*                                                                       */
	/*  Direct RGB565 AVI playback 	                                         */
	/*                                                                       */
	/*-----------------------------------------------------------------------*/
	lcdformatex.ucVASrcFormat = DRVVPOST_FRAME_RGB565;
  vpostLCMInit(&lcdformatex, (UINT32 *)_VpostFrameBuffer);
	
	/* If backlight control signal is different from nuvoton's demo board,
	   please don't call this function and must implement another similar one to enable LCD backlight. */
	vpostEnaBacklight();	

	fsAsciiToUnicode("c:\\movie.avi", suFileName, TRUE);	

  if (aviPlayFile(suFileName, 0, 0, DIRECT_RGB565, avi_play_control) < 0)
		sysprintf("Playback failed, code = %x\n", nStatus);
	else
		sysprintf("Playback done.\n");
#endif
#if 0
	/*-----------------------------------------------------------------------*/
	/*                                                                       */
	/*  Direct YUV422 AVI playback 	                                         */
	/*                                                                       */
	/*-----------------------------------------------------------------------*/
	lcdformatex.ucVASrcFormat = DRVVPOST_FRAME_YCBYCR;
    vpostLCMInit(&lcdformatex, (UINT32 *)_VpostFrameBuffer);

	/* If backlight control signal is different from nuvoton's demo board,
	   please don't call this function and must implement another similar one to enable LCD backlight. */
	vpostEnaBacklight();	

	fsAsciiToUnicode("c:\\movie.avi", suFileName, TRUE);	

  if (aviPlayFile(suFileName, 0, 0, DIRECT_YUV422, avi_play_control) < 0)
		sysprintf("Playback failed, code = %x\n", nStatus);
	else
		sysprintf("Playback done.\n");
#endif	

	while(1);		
}



