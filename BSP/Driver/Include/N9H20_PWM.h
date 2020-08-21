/**************************************************************************//**
 * @file     N9H20_PWM.h
 * @version  V3.00
 * @brief    N9H20 series PWM driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#ifndef __DRVPWM_H__
#define __DRVPWM_H__


/*---------------------------------------------------------------------------------------------------------*/
/* Includes of system headers                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
#include "wbio.h"
#include "wblib.h"


#ifdef  __cplusplus
extern "C"
{
#endif

/*---------------------------------------------------------------------------------------------------------*/
/*  Define Version number								                                                   */
/*---------------------------------------------------------------------------------------------------------*/
#define	PWM_MAJOR_NUM 1
#define	PWM_MINOR_NUM 00
#define	PWM_BUILD_NUM 1

/*---------------------------------------------------------------------------------------------------------*/
/*  PWM Timer 								                                                               */
/*---------------------------------------------------------------------------------------------------------*/
#define	PWM_TIMER0     0x0
#define	PWM_TIMER1     0x1
#define	PWM_TIMER2     0x2
#define	PWM_TIMER3     0x3

/*---------------------------------------------------------------------------------------------------------*/
/*  PWM Capture								                                                               */
/*---------------------------------------------------------------------------------------------------------*/
#define	PWM_CAP0     0x10
#define	PWM_CAP1	 0x11
#define	PWM_CAP2     0x12
#define	PWM_CAP3     0x13

/*---------------------------------------------------------------------------------------------------------*/
/*  PWM Capture Interrupt Enable/Disable Flag                                                              */
/*---------------------------------------------------------------------------------------------------------*/
#define PWM_CAP_RISING_INT	1
#define PWM_CAP_FALLING_INT	2
#define PWM_CAP_ALL_INT		0

/*---------------------------------------------------------------------------------------------------------*/
/*  PWM Capture Interrupt Flag				                                                               */
/*---------------------------------------------------------------------------------------------------------*/
#define PWM_CAP_RISING_FLAG	6
#define PWM_CAP_FALLING_FLAG	7

/*---------------------------------------------------------------------------------------------------------*/
/*  PWM Clcok Selector						                                                               */
/*---------------------------------------------------------------------------------------------------------*/
#define PWM_CLOCK_DIV_1    1
#define PWM_CLOCK_DIV_2    2
#define PWM_CLOCK_DIV_4    4
#define PWM_CLOCK_DIV_8    8
#define PWM_CLOCK_DIV_16   16

/*---------------------------------------------------------------------------------------------------------*/
/*  PWM Mode Selector						                                                               */
/*---------------------------------------------------------------------------------------------------------*/
#define PWM_TOGGLE_MODE		TRUE
#define PWM_ONE_SHOT_MODE	FALSE

/*---------------------------------------------------------------------------------------------------------*/
/* Define PWM Time Data Struct                                                                             */
/*---------------------------------------------------------------------------------------------------------*/
typedef struct
{
	UINT8	u8Mode;
	FLOAT	fFrequency;
	UINT8	u8HighPulseRatio;
	BOOL	bInverter;
	UINT8	u8ClockSelector;
	UINT8	u8PreScale;
	UINT32	u32Duty;
 
}PWM_TIME_DATA_T;

/*---------------------------------------------------------------------------------------------------------*/
/* Define PWM Call back funvtion Data Struct                                                               */
/*---------------------------------------------------------------------------------------------------------*/
typedef void (*PFN_PWM_CALLBACK)(void);

/*---------------------------------------------------------------------------------------------------------*/
/* Define PWM Call back function Data Struct                                                               */
/*---------------------------------------------------------------------------------------------------------*/
typedef struct
{
    PFN_PWM_CALLBACK pfnPWM0CallBack;    
    PFN_PWM_CALLBACK pfnCAP0CallBack;
   
    PFN_PWM_CALLBACK pfnPWM1CallBack;    
    PFN_PWM_CALLBACK pfnCAP1CallBack;
    
    PFN_PWM_CALLBACK pfnPWM2CallBack;    
    PFN_PWM_CALLBACK pfnCAP2CallBack;
    
    PFN_PWM_CALLBACK pfnPWM3CallBack;    
    PFN_PWM_CALLBACK pfnCAP3CallBack;        
   
}PWM_CALLBACK_T;


/*---------------------------------------------------------------------------------------------------------*/
/* Define PWM functions prototype                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
VOID	PWM_Open(VOID);
VOID	PWM_Close(VOID);
VOID	PWM_SetTimerCounter(UINT8 u8Timer, UINT16 u16Counter);
VOID	PWM_EnableInt(UINT8	u8Timer, UINT8 u8Int);
VOID	PWM_ClearInt(UINT8 u8Timer);
VOID	PWM_DisableInt(UINT8 u8Timer, UINT8 u8Int);
VOID	PWM_ClearCaptureIntStatus(UINT8	u8Capture, UINT8 u8IntType);
VOID	PWM_EnableDeadZone(UINT8 u8Timer, UINT8 u8Length, BOOL bEnableDeadZone);
VOID	PWM_SetTimerIO(UINT8 u8Timer, BOOL bEnable);
VOID	PWM_Enable(UINT8 u8Timer, BOOL bEnable);
BOOL	PWM_IsTimerEnabled(UINT8 u8Timer);
BOOL	PWM_GetCaptureIntStatus(UINT8 u8Capture, UINT8 u8IntType);
BOOL	PWM_GetIntFlag(UINT8 u8Timer);
UINT16	PWM_GetRisingCounter(UINT8 u8Capture);
UINT16	PWM_GetFallingCounter(UINT8 u8Capture);
UINT32	PWM_GetTimerCounter(UINT8 u8Timer);
FLOAT	PWM_SetTimerClk(UINT8 u8Timer, PWM_TIME_DATA_T *sPt);
VOID PWM_InstallCallBack(UINT8 u8Timer,	PFN_PWM_CALLBACK pfncallback,	PFN_PWM_CALLBACK *pfnOldcallback);
#ifdef  __cplusplus
}
#endif

#endif

















