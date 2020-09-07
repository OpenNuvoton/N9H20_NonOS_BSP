/**************************************************************************//**
 * @file     N9H20_ADC.h
 * @version  V3.00
 * @brief    N9H20 series ADC driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

/****************************************************************************
* FILENAME
*   N9H20_adc.h
*
* VERSION
*   1.0
*
* DESCRIPTION
*   ADC library header file
*
* DATA STRUCTURES
*   None
*
* FUNCTIONS
*
* HISTORY
*
* REMARK
*   None
****************************************************************************/
#ifndef __N9H20_ADC__
#define __N9H20_ADC__
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wblib.h"
typedef void (*PFN_ADC_CALLBACK)(VOID);	

#define ADC_TS_4WIRE	0

#define ADC_NONBLOCK	0
#define ADC_BLOCK	1

#define ERR_ADC				(0xFFFF0000 | ((ADC_BA>>16) & 0xFF00) |((ADC_BA>>8) & 0xFF))

typedef enum{ 
	eADC_ADC_INT = 0,                                           
   	eADC_AUD_INT,
	eADC_LVD_INT,
	eADC_WT_INT   	                                                                                  
}E_ADC_INT;

typedef enum{                            
	eADC_TSCREEN_NORMAL = 0,         	  
	eADC_TSCREEN_SEMI,       
	eADC_TSCREEN_AUTO,
	eADC_TSCREEN_TRIG                                                           
}E_ADC_TSC_MODE;  

typedef enum{ 
	eADC_BAND_P0P5 = 0,                                             
	eADC_BAND_P0P75
}E_ADC_UPBAND;
    
typedef enum{ 
	eADC_BAND_N0P5 = 0,                                             
	eADC_BAND_N0P75
}E_ADC_DOWNBAND;   

#define E_DRVADC_INVALID_INT		(ERR_ADC | 00)	
#define E_DRVADC_INVALID_CHANNEL	(ERR_ADC | 01)	
#define E_DRVADC_INVALID_TIMING		(ERR_ADC | 02)	
#define E_ADC_INVALID_INT			E_DRVADC_INVALID_INT
#define E_ADC_INVALID_CHANNEL		E_DRVADC_INVALID_CHANNEL
#define E_ADC_INVALID_TIMING		E_DRVADC_INVALID_TIMING	


extern void adc_init(void);
extern int adc_open(unsigned char type, unsigned short hr, unsigned short vr);
extern int adc_read(unsigned char mode, unsigned short *x, unsigned short *y);	/* Read Touch panel */
extern UINT32 adc_normalread(UINT32 u32Channel, PUINT16 pu16Data);		/* Low battery */
extern void adc_close(void);
extern UINT32 adc_enableInt(E_ADC_INT eIntType);
extern UINT32 adc_disableInt(E_ADC_INT eIntType);
extern UINT32 adc_installCallback(E_ADC_INT eIntType,PFN_ADC_CALLBACK pfnCallback,PFN_ADC_CALLBACK* pfnOldCallback);
extern void adc_setTouchScreen(E_ADC_TSC_MODE eTscMode,UINT32 u32DleayCycle,BOOL bIsPullup,BOOL bMAVFilter);

/***********************************************************************************************/
INT32 audio_Open(E_SYS_SRC_CLK eSrcClock, UINT32 u32ConvClock);
void audio_StartRecord(void);
void audio_StopRecord(void);
PINT16 audio_GetRecordData(void);
void audio_SetAutoGainTiming(UINT32 u32Period, UINT32 u32Attack, UINT32 u32Recovery, UINT32 u32Hold);
void audio_GetAutoGainTiming(PUINT32 pu32Period,PUINT32 pu32Attack,PUINT32 pu32Recovery,PUINT32 pu32Hold);
void audio_SetAutoGainControl(BOOL bIsEnable, UINT32 u32OutputLevel, E_ADC_UPBAND eAdcUpBand,E_ADC_DOWNBAND eAdcDownBand);
#endif  /* __N9H20_ADC__ */


