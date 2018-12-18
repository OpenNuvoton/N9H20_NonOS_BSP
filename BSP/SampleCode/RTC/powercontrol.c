/****************************************************************************
 * @file     powercontrol.c
 * @version  V1.00
 * $Revision: 4 $
 * $Date: 18/04/25 11:43a $
 * @brief    RTC driver sample file
 *
 * @note
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include "wblib.h"
#include "N9H20.h"

char u8Tmp[0x1000];
extern void Sample_PowerDown(void);
extern VOID RTC_AlarmISR(VOID);
extern BOOL volatile g_bAlarm;
BOOL volatile g_bPowerKeyPress = FALSE;

#define HW_POWER_OFF	0
#define SW_POWER_OFF	1
#define SYSTEM_CLOCK	12000000

__align(32) UINT8 u32Array[1024*1024];
/*--------------------------------------------------------------------------------------------------------*
 *                                                                                                        *
 *  Wake up source                                                                                        *
 *  KPI_WE, ADC_WE, UHC_WE, UDC_WE, UART_WE, SDH_WE, RTC_WE, GPIO_WE                                      *
 *  2. Default priority                                                                                   *
 *--------------------------------------------------------------------------------------------------------*/

void Smpl_RTC_Powerdown_Wakeup(void)
{
    RTC_TIME_DATA_T sCurTime;
    PUINT8 pu8Buf, pu8Tmp;
    UINT32 u32Idx, reg_AHBCLK;
    pu8Buf = u32Array;

    sysprintf("\n2. RTC Powerdown Wakeup Test (Wakeup after 10 seconds)\n");
    sysprintf("Allocate memory address =0x%x\n", pu8Buf);
    pu8Tmp = pu8Buf;
    for(u32Idx=0; u32Idx<(1024*1024);u32Idx=u32Idx+1)
        *pu8Tmp++= (UINT8)((u32Idx>>8) + u32Idx);

    outp32(REG_AHBCLK, inp32(REG_AHBCLK) | USBH_CKE);      /* USB Host disable */
    outp32(0xb1009200, 0x08000000);
    sysprintf("Disable USB Transceiver\n");

    outp32(REG_APBCLK, inp32(REG_APBCLK) | ADC_CKE);       /* ADC disable */
    outp32 (REG_ADC_CON, inp32(REG_ADC_CON) & ~ADC_CON_ADC_EN);
    outp32(REG_AUDIO_CON, 0x0);
    outp32(REG_ADC_CON, 0x0);
    outp32(REG_MISCR, inp32(REG_MISCR) & ~LVR_EN);
    sysprintf("Disable ADC and LVR\n");
    outp32(REG_APBCLK, inp32(REG_APBCLK) & ~ADC_CKE);
    outp32(REG_AHBCLK, inp32(REG_AHBCLK) | (SPU_CKE | ADO_CKE));
    outp32(REG_SPU_DAC_VOL, inp32(REG_SPU_DAC_VOL) | ANA_PD);
    sysprintf("Disable SPU and ADO\n");
    outp32(REG_AHBCLK, inp32(REG_AHBCLK) & ~(SPU_CKE | ADO_CKE));

    outp32(REG_AHBCLK, inp32(REG_AHBCLK) | USBD_CKE);    /* USB phy disable */
    outp32(PHY_CTL, 0x0);
    sysprintf("Disable USB phy\n");
    outp32(REG_AHBCLK, inp32(REG_AHBCLK) & ~USBD_CKE);

    outp32(REG_AHBCLK, inp32(REG_AHBCLK) | VPOST_CKE);    /* TV DAC */
    outp32(REG_LCM_TVCtl, inp32(REG_LCM_TVCtl) | TVCtl_Tvdac);
    sysprintf("Disable TV DAC \n");
    outp32(REG_AHBCLK, inp32(REG_AHBCLK) & ~VPOST_CKE);

    /*Save AHB clock */
    reg_AHBCLK = inp32(REG_AHBCLK);
    outp32(REG_AHBCLK, 0x011F);	

    /* change  SD card pin function */
    outpw(REG_GPAFUN, inpw(REG_GPAFUN) & ~MF_GPA1);       /* GPA1 for SD-0 card detection */
    outpw(REG_GPEFUN, inpw(REG_GPEFUN)& ~0x0000FFF0 );    /* SD0_CLK/CMD/DAT0_3 pins selected */
    outpw(REG_GPAFUN, inpw(REG_GPAFUN) & ~MF_GPA0);	

    outp32(REG_GPAFUN, (MF_GPA11 | MF_GPA10));
    outp32(REG_GPBFUN, 0x0);
    outp32(REG_GPCFUN, 0x0);
    outp32(REG_GPDFUN, (MF_GPD4 | MF_GPD3| MF_GPD2 | MF_GPD1 | MF_GPD0));
    outp32(REG_GPEFUN, 0x0);

    g_bAlarm = FALSE;	

    /* Get the currnet time */
    RTC_Read(RTC_CURRENT_TIME, &sCurTime);

    /* Set Alarm call back function */
    sCurTime.pfnAlarmCallBack = RTC_AlarmISR;

    sysprintf("   Current Time:%d/%02d/%02d %02d:%02d:%02d\n",sCurTime.u32Year,sCurTime.u32cMonth,sCurTime.u32cDay,sCurTime.u32cHour,sCurTime.u32cMinute,sCurTime.u32cSecond);

    /* The alarm time setting */
    sCurTime.u32cSecond = sCurTime.u32cSecond + 10;

    if(sCurTime.u32cSecond >= 60)
    {
        sCurTime.u32cSecond -= 60;
        sCurTime.u32cMinute++;

        if(sCurTime.u32cMinute >= 60)
        {
            sCurTime.u32cMinute -= 60;
            sCurTime.u32cHour++;
        }
    }

    /* Set the alarm time (Install the call back function and enable the alarm interrupt)*/
    RTC_Write(RTC_ALARM_TIME,&sCurTime);

    RTC_Read(RTC_ALARM_TIME,&sCurTime);
    sysprintf("   Alarm Time:%d/%02d/%02d %02d:%02d:%02d\n",sCurTime.u32Year,sCurTime.u32cMonth,sCurTime.u32cDay,sCurTime.u32cHour,sCurTime.u32cMinute,sCurTime.u32cSecond);
    sysPowerDown(WE_RTC);

    outp32(REG_GPIOA_OMD, inp32(REG_GPIOA_OMD) | 0x02);     /* GPIOA-1 output.*/
    outp32(REG_GPIOA_DOUT, inp32(REG_GPIOA_DOUT) & ~0x02);  /* GPIOA-1 output LOW. */
    /* Restore AHB clock */
    outp32(REG_AHBCLK, reg_AHBCLK);

    sysprintf("Exit power down\n");
    pu8Tmp = pu8Buf;
    for(u32Idx=0; u32Idx<(1024*1024);u32Idx=u32Idx+1)
    {
        if( *pu8Tmp !=  (UINT8)((u32Idx>>8) + u32Idx))
        {
            sysprintf("!!!!!!!!!!!!!!!Data is non-consistent after power down\n");
            sysprintf("0x%x, 0x%x, 0x%x)\n",u32Idx, *pu8Tmp, (UINT8)((u32Idx>>8) + u32Idx) );
            return;
        }
        pu8Tmp++;
    }
    sysprintf("Data is consisient\n");

    sysprintf("   Wake up!!!\n");	

    while(!g_bAlarm);

    RTC_Read(RTC_CURRENT_TIME,&sCurTime);
    sysprintf("   Current Time:%d/%02d/%02d %02d:%02d:%02d\n",sCurTime.u32Year,sCurTime.u32cMonth,sCurTime.u32cDay,sCurTime.u32cHour,sCurTime.u32cMinute,sCurTime.u32cSecond);
}

VOID PowerKeyPress(VOID)
{
    sysprintf("\nPower Key Press!!\n");
    g_bPowerKeyPress = TRUE;
    return;
}


VOID Smpl_RTC_PowerOff_Control(UINT32 u32Mode)
{
    UINT32 u32PowerKeyStatus;
    INT32 volatile i;

    sysSetTimerReferenceClock (TIMER0, SYSTEM_CLOCK);
    sysStartTimer(TIMER0, 100, PERIODIC_MODE);

    g_bPowerKeyPress = FALSE;

    sysprintf("Turn on H/W power off function\n");

    /* Press Power Key during 6 sec to Power off */
    RTC_Ioctl(0, RTC_IOC_SET_POWER_OFF_PERIOD, 6, 0);

    /* Install the callback function for Power Key Press */
    RTC_Ioctl(0, RTC_IOC_SET_PSWI_CALLBACK, (UINT32)PowerKeyPress, 0);

    /* Enable Hardware Power off */
    RTC_Ioctl(0, RTC_IOC_ENABLE_HW_POWEROFF, 0, 0);

    /* Wait Key Press */
    sysprintf("Wait for Key Press\n");

    while(!g_bPowerKeyPress);

    /* Query Power Key 3 SEC (query a time per 1 sec)*/
    sysprintf("Press Key 3 seconds (Power off procedure starts after 3 seconds)\n");
    for(i = 0 ; i< 3;i++)
    {
        /* Delay 1 second */
        sysDelay(100);

        /* Query Power Key Status */
        RTC_Ioctl(0, RTC_IOC_GET_POWERKEY_STATUS, (UINT32)&u32PowerKeyStatus, 0);

        if(u32PowerKeyStatus)
        {
            sysprintf("	Power Key Release\n");
            sysprintf("	Power Off Flow Stop\n");
            return;
        }
        else
            sysprintf("	Power Key Press\n");
    }

    /* S/W Power off sequence */
    sysprintf("S/W Power off sequence start (1 second)..\n");

    /* Use time to simulate the S/W Power off sequence (1 sec) */
    sysDelay(100);

    if(u32Mode == SW_POWER_OFF) 
    {
        sysprintf("Power off sequence Complete -> Power Off ");

        /* Power Off - S/W can call the API to power off any time he wnats */
        RTC_Ioctl(0, RTC_IOC_SET_POWER_OFF, 0, 0);
    }
    else
    {
        sysprintf("S/W Crash!!\n");
        sysprintf("Wait for HW Power off");
    }

    i = 0;
    while(1)
    {
        if((i%50000) == 0)
            sysprintf(".");
        i++;
    }
}

