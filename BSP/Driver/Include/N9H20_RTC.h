/**************************************************************************//**
 * @file     N9H20_RTC.h
 * @version  V3.00
 * @brief    N9H20 series RTC driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#ifndef __DRVRTC_H__
#define __DRVRTC_H__

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
/*  Define Version number                                                                                  */
/*---------------------------------------------------------------------------------------------------------*/
#define RTC_MAJOR_NUM 1
#define RTC_MINOR_NUM 0
#define RTC_BUILD_NUM 1

/*---------------------------------------------------------------------------------------------------------*/
/* Define Error Code                                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
// E_DRVRTC_ERR_CALENDAR_VALUE    Wrong Calendar Value
// E_DRVRTC_ERR_TIMESACLE_VALUE   Wrong Time Scale Value
// E_DRVRTC_ERR_TIME_VALUE        Wrong Time Value
// E_DRVRTC_ERR_DWR_VALUE         Wrong Day Value
// E_DRVRTC_ERR_FCR_VALUE         Wrong Compenation value
// E_DRVRTC_ERR_EIO               Initial RTC Failed.
// E_DRVRTC_ERR_ENOTTY            Command not support, or parameter incorrect.
// E_DRVRTC_ERR_ENODEV            Interface number incorrect.
// E_DRVRTC_ERR_FAILED            Failed.
#define E_RTC_SUCCESS               0
#define E_RTC_ERR_CALENDAR_VALUE    1
#define E_RTC_ERR_TIMESACLE_VALUE   2
#define E_RTC_ERR_TIME_VALUE        3
#define E_RTC_ERR_DWR_VALUE         4
#define E_RTC_ERR_FCR_VALUE         5
#define E_RTC_ERR_EIO               6
#define E_RTC_ERR_ENOTTY            7
#define E_RTC_ERR_ENODEV            8

/*---------------------------------------------------------------------------------------------------------*/
/*  RTC Access Key                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#define RTC_INIT_KEY    0xa5eb1357
#define RTC_WRITE_KEY   0xa965

/*---------------------------------------------------------------------------------------------------------*/
/*  RTC Initial Time Out Value                                                                             */
/*---------------------------------------------------------------------------------------------------------*/
#define RTC_WAIT_COUNT  10000

/*---------------------------------------------------------------------------------------------------------*/
/*  RTC Reference                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
#define RTC_YEAR2000        2000
#define RTC_FCR_REFERENCE   32761

/*---------------------------------------------------------------------------------------------------------*/
/*  Leap Year                                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
#define RTC_LEAP_YEAR   1

/*---------------------------------------------------------------------------------------------------------*/
/*  12-Hour / 24-Hour                                                                                      */
/*---------------------------------------------------------------------------------------------------------*/
#define RTC_CLOCK_12    0
#define RTC_CLOCK_24    1

/*---------------------------------------------------------------------------------------------------------*/
/*  AM / PM                                                                                                */
/*---------------------------------------------------------------------------------------------------------*/
#define RTC_AM    1
#define RTC_PM    2

/*---------------------------------------------------------------------------------------------------------*/
/* INTERRUPT SOURCE                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
typedef enum
{
    RTC_ALARM_INT    =0x01,
    RTC_TICK_INT     =0x02,
    RTC_PSWI_INT     =0x04,
    RTC_ALL_INT      =0x07
}RTC_INT_SOURCE;

/*---------------------------------------------------------------------------------------------------------*/
/* Define Ioctl commands                                                                                   */
/*---------------------------------------------------------------------------------------------------------*/
typedef enum
{
    RTC_IOC_IDENTIFY_LEAP_YEAR    =  0,
    RTC_IOC_SET_TICK_MODE         =  1,
    RTC_IOC_GET_TICK              =  2,
    RTC_IOC_RESTORE_TICK          =  3,
    RTC_IOC_ENABLE_INT            =  4,
    RTC_IOC_DISABLE_INT           =  5,
    RTC_IOC_SET_CURRENT_TIME      =  6,
    RTC_IOC_SET_ALAMRM_TIME       =  7,
    RTC_IOC_SET_FREQUENCY         =  8,
    RTC_IOC_SET_POWER_ON          =  9,
    RTC_IOC_SET_POWER_OFF         =  10,
    RTC_IOC_SET_POWER_OFF_PERIOD  =  11,
    RTC_IOC_ENABLE_HW_POWEROFF    =  12,
    RTC_IOC_DISABLE_HW_POWEROFF   =  13,
    RTC_IOC_GET_POWERKEY_STATUS   =  14,
    RTC_IOC_SET_PSWI_CALLBACK     =  15
}E_RTC_CMD;

/*---------------------------------------------------------------------------------------------------------*/
/* Define for RTC Tick mode                                                                                */
/*---------------------------------------------------------------------------------------------------------*/
typedef enum
{
    RTC_TICK_1_SEC       =         0,                           /* 1     sec                            */
    RTC_TICK_1_2_SEC     =         1,                           /* 1/2   sec                            */
    RTC_TICK_1_4_SEC     =         2,                           /* 1/4   sec                            */ 
    RTC_TICK_1_8_SEC     =         3,                           /* 1/8   sec                            */
    RTC_TICK_1_16_SEC    =         4,                           /* 1/16  sec                            */
    RTC_TICK_1_32_SEC    =         5,                           /* 1/32  sec                            */ 
    RTC_TICK_1_64_SEC    =         6,                           /* 1/64  sec                            */
    RTC_TICK_1_128_SEC   =         7                            /* 1/128 sec                            */
}RTC_TICK;

/*---------------------------------------------------------------------------------------------------------*/
/* Define Time data struct & some parameters                                                               */
/*---------------------------------------------------------------------------------------------------------*/
typedef void (PFN_RTC_CALLBACK)(void);

typedef enum
{
    RTC_CURRENT_TIME    =    0,
    RTC_ALARM_TIME      =    1 
}E_RTC_TIME_SELECT;

/*---------------------------------------------------------------------------------------------------------*/
/* Define Day of week parameter                                                                            */
/*---------------------------------------------------------------------------------------------------------*/
typedef enum
{
    RTC_SUNDAY         =   0,
    RTC_MONDAY         =   1,
    RTC_TUESDAY        =   2,
    RTC_WEDNESDAY      =   3,
    RTC_THURSDAY       =   4,
    RTC_FRIDAY         =   5,
    RTC_SATURDAY       =   6
}E_RTC_DWR_PARAMETER;
/*---------------------------------------------------------------------------------------------------------*/
/* Define Time Data Struct                                                                                 */
/*---------------------------------------------------------------------------------------------------------*/
typedef struct
{
    UINT8 u8cClockDisplay;                                         /* 12-Hour, 24-Hour */
    UINT8 u8cAmPm;                                                 /* Only 12-hr used */
    UINT32 u32cSecond;
    UINT32 u32cMinute;
    UINT32 u32cHour;
    UINT32 u32cDayOfWeek;
    UINT32 u32cDay;
    UINT32 u32cMonth;
    UINT32 u32Year;
    PFN_RTC_CALLBACK *pfnAlarmCallBack;    
}RTC_TIME_DATA_T;

/*---------------------------------------------------------------------------------------------------------*/
/* Define Tick Struct                                                                                      */
/*---------------------------------------------------------------------------------------------------------*/
typedef struct
{
    UINT8 ucMode;
    PFN_RTC_CALLBACK *pfnTickCallBack;
}RTC_TICK_T;

/*---------------------------------------------------------------------------------------------------------*/
/* Define Function Prototype                                                                               */
/*---------------------------------------------------------------------------------------------------------*/
UINT32 RTC_Init(VOID);   
UINT32 RTC_Open(RTC_TIME_DATA_T *sPt);
UINT32 RTC_Ioctl(INT32 i32Num, E_RTC_CMD eCmd, UINT32 u32Arg0, UINT32 u32Arg1);
UINT32 RTC_Read(E_RTC_TIME_SELECT eTime, RTC_TIME_DATA_T *sPt);
UINT32 RTC_Write(E_RTC_TIME_SELECT eTime, RTC_TIME_DATA_T *sPt);
UINT32 RTC_SetFrequencyCompensation(FLOAT fnumber);
UINT32 RTC_WriteEnable (VOID);
UINT32 RTC_Close(VOID);

#ifdef  __cplusplus
}
#endif

#endif /* __DRVRTC_H__ */



