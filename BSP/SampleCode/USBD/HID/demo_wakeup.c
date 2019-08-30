/****************************************************************************
 * @file     demo_wakeup.c
 * @version  V1.00
 * $Revision: 4 $
 * $Date: 18/04/25 11:43a $
 * @brief    HID Class Device sample header file
 *
 * @note
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include "N9H20.h"

UINT8 u32Array[1024*1024] __attribute__((aligned(32)));
/*--------------------------------------------------------------------------------------------------------*
 *                                                                                                        *
 *  Wake up source                                                                                        *
 *  KPI_WE, ADC_WE, UHC_WE, UDC_WE, UART_WE, SDH_WE, RTC_WE, GPIO_WE                                      *
 *  2. Default priority                                                                                   *
 *--------------------------------------------------------------------------------------------------------*/

void Demo_PowerDownWakeUp(void)
{
    PUINT8 pu8Buf, pu8Tmp;
    UINT32 u32Idx;
    pu8Buf = u32Array;

    sysprintf("Allocate memory address =0x%x\n", pu8Buf);
    pu8Tmp = pu8Buf;
    for(u32Idx=0; u32Idx<(1024*1024);u32Idx=u32Idx+1)
        *pu8Tmp++= (UINT8)((u32Idx>>8) + u32Idx);

    /* Set gpio wake up source */
    sysprintf("Enter power down, GPIO Int status 0x%x\n", inp32(REG_IRQTGSRC0));

    outp32(REG_AHBCLK, inp32(REG_AHBCLK) | USBH_CKE);     /* USB Host disable */
    outp32(0xb1009200, 0x08000000);
    sysprintf("Disable USB Transceiver\n");

    outp32(REG_APBCLK, inp32(REG_APBCLK) | ADC_CKE);      /* ADC disable */
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

#if 0
    outp32(REG_AHBCLK, inp32(REG_AHBCLK) | USBD_CKE);    /* USB phy disable */
    outp32(PHY_CTL, 0x0);
    sysprintf("Disable USB phy\n");
    outp32(REG_AHBCLK, inp32(REG_AHBCLK) & ~USBD_CKE);
#endif

    outp32(REG_AHBCLK, inp32(REG_AHBCLK) | VPOST_CKE);    /* TV DAC */
    outp32(REG_LCM_TVCtl, inp32(REG_LCM_TVCtl) | TVCtl_Tvdac);
    sysprintf("Disable TV DAC \n");
    outp32(REG_AHBCLK, inp32(REG_AHBCLK) & ~VPOST_CKE);


    /* change  SD card pin function */
    outpw(REG_GPAFUN, inpw(REG_GPAFUN) & ~MF_GPA1);       /* GPA1 for SD-0 card detection */
    outpw(REG_GPEFUN, inpw(REG_GPEFUN)& ~0x0000FFF0 );    /* SD0_CLK/CMD/DAT0_3 pins selected */
    outpw(REG_GPAFUN, inpw(REG_GPAFUN) & ~MF_GPA0);

    outp32(REG_GPAFUN, (MF_GPA11 | MF_GPA10));
    outp32(REG_GPBFUN, 0x0);
    outp32(REG_GPCFUN, 0x0);
    outp32(REG_GPDFUN, (MF_GPD4 | MF_GPD3| MF_GPD2 | MF_GPD1 | MF_GPD0));
    outp32(REG_GPEFUN, 0x0);

#ifdef POWERDOWN_PLL
    sysPowerDownPLLDuringSysPowerDown(1);    /* Power down PLL as system power down, the wake-up time about 25ms */
#else
    sysPowerDownPLLDuringSysPowerDown(0);    /* Not to Power down PLL as system power down to shrink wake-up time from 25ms to 3ms */
#endif
    sysPowerDown(WE_UDC);
 
    outp32(REG_GPIOA_OMD, inp32(REG_GPIOA_OMD) | 0x02);    /* GPIOA-1 output.*/
    outp32(REG_GPIOA_DOUT, inp32(REG_GPIOA_DOUT) & ~0x02); /* GPIOA-1 output LOW.*/


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
}
