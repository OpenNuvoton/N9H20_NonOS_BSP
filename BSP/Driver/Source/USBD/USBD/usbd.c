/**************************************************************************//**
 * @file     usbd.c
 * @version  V3.00
 * @brief    N9H20 series USB Device driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wblib.h"
#include "N9H20_USBD.h"

#define DATA_CODE  "20230508"

#if defined (__GNUC__)
volatile USBD_INFO_T usbdInfo  __attribute__((aligned(4))) = {0};
volatile USBD_STATUS_T usbdStatus  __attribute__((aligned(4))) = {0};
#else
__align(4) volatile USBD_INFO_T usbdInfo = {0};
__align(4) volatile USBD_STATUS_T usbdStatus = {0};
#endif

UINT32 g_u32Suspend_Flag = 0;
PFN_USBD_CALLBACK pfnSuspend = NULL, pfnResume = NULL;

USB_CMD_T _usb_cmd_pkt;
INT32 volatile usb_halt_ep;
BOOL volatile g_bHostAttached = FALSE;
VOID usbd_isr(void);
UINT32 volatile class_out_len = 0, zeropacket_flag = 0;
VOID usbdClearAllFlags(void)
{
    usbdInfo.GET_DEV_Flag=0;
    usbdInfo.GET_CFG_Flag=0;
    usbdInfo.GET_QUL_Flag=0;
    usbdInfo.GET_OSCFG_Flag=0;
    usbdInfo.GET_STR_Flag=0;
    usbdInfo.GET_HID_Flag = 0;
    usbdInfo.GET_HID_RPT_Flag = 0;
    usbdInfo.usbdGetConfig=0;
    usbdInfo.usbdGetInterface=0;
    usbdInfo.usbdGetStatus=0;
    usbdInfo.enabletestmode = 0;
}

VOID udcOpen(void)
{
    UINT32 u32PllOutKHz,u32ExtFreq;
    int i;

    sysprintf("N9H20 UDC Library (%s)\n",DATA_CODE);

    u32ExtFreq = sysGetExternalClock();

    outp32(REG_CLKDIV2, inp32(REG_CLKDIV2) & ~0x0060F0E0);
    
    if(u32ExtFreq == 27000)
    {
        u32PllOutKHz = sysGetPLLOutputKhz(eSYS_UPLL, u32ExtFreq);
        
        i = (u32PllOutKHz / 2) / 12000 - 1;
        
        /* USB Clock source = UPLL / 2 */
        /* USB Clock = USB Clock source / (i+1) */
        outp32(REG_CLKDIV2, inp32(REG_CLKDIV2) | 0x00600020 | (i << 12));
        
    }
    
#ifndef __USBD_FULL_SPEED_MODE__
    if(inp32(PHY_CTL) & Vbus_status)
    {
        usbdInfo.u32VbusStatus = 1;
        usbdStatus.usbConnected = 1;
    }
    else
    {
        usbdInfo.u32VbusStatus = 0;
        usbdStatus.usbConnected = 0;
    }

#endif

#ifdef __USBD_FULL_SPEED_MODE__
    outp32(REG_AHBCLK, inp32(REG_AHBCLK) | USBH_CKE);
    outp32(REG_HC_RH_OP_MODE, inp32(REG_HC_RH_OP_MODE) | (BIT16|BIT17) );
#endif
    outp32(REG_AHBCLK, inp32(REG_AHBCLK) | USBD_CKE | HCLK3_CKE);
    outpw(REG_AHBIPRST, inpw(REG_AHBIPRST) | UDCRST);
    outpw(REG_AHBIPRST, inpw(REG_AHBIPRST) & ~UDCRST);

    outp32(PHY_CTL, (0x20 | Phy_suspend));

    outp32(CEP_END_ADDR, 0x7F);

    while(inp32(CEP_END_ADDR) != 0x7F);

    outp32(OPER, 0x0);
    while(inp32(OPER) != 0x0);

    usbdInfo.u32UVC = 0;
#ifdef __USBD_FULL_SPEED_MODE__

    for(i=0;i<0x30000;i++)
    {
        if(inp32(PHY_CTL) & Vbus_status)
        {
            usbdInfo.u32VbusStatus = 1;
            usbdStatus.usbConnected = 1;
            break;
        }
        else
        {
            usbdInfo.u32VbusStatus = 0;
            usbdStatus.usbConnected = 0;
        }
    }
#endif
}


VOID udcInit(void)
{  
    /* initial state */
    usbdInfo._usbd_devstate = 0;    /* initial state */
    usbdInfo._usbd_address  = 0;    /* not set */
    usbdInfo._usbd_confsel  = 0;    /* not selected */
    usbdInfo._usbd_intfsel  = 0;    /* not selected */

#ifdef __USBD_FULL_SPEED_MODE__
    usbdInfo._usbd_speedset = 1;    /* default at high speed mode */
#else
    usbdInfo._usbd_speedset = 2;    /* default at high speed mode */
#endif

    usbdInfo.AlternateFlag = 0;
    usbdInfo.AlternateFlag_Audio = 0;         /* For UAC audio */
    usbdStatus.appConnected = 0;
    usbdStatus.appConnected_Audio = 0;        /* For UAC audio */
    usbdInfo._usbd_haltep = -1;

    sysInstallISR(IRQ_LEVEL_1, IRQ_UDC, (PVOID)usbd_isr);
    sysEnableInterrupt(IRQ_UDC);
    sysSetLocalInterrupt(ENABLE_IRQ);
    
    /* 
     * configure mass storage interface endpoint 
     */
    /* Device is in High Speed */
    if (usbdInfo._usbd_speedset == 2)
    {
        usbdInfo.pfnHighSpeedInit();
    }
    /* Device is in Full Speed */
    else if (usbdInfo._usbd_speedset == 1)
    {
        usbdInfo.pfnFullSpeedInit();
    }

    /*
     * configure USB controller 
     */
    outp32(IRQ_ENB_L, (USB_IE|CEP_IE|EPA_IE|EPB_IE|EPC_IE|EPD_IE));  /* enable usb, cep, epa, epb, epc interrupt */
    outp32(USB_IRQ_ENB, (DMACOM_IE | RUM_IE | RST_IE|VBUS_IE));
    outp32(ADDR, 0);
#ifndef __USBD_FULL_SPEED_MODE__
    outp32(OPER, SET_HISPD);
#endif
    /* allocate 0xff bytes mem for cep */
    outp32(CEP_START_ADDR, 0);
    outp32(CEP_END_ADDR, 0x7F);
    outp32(CEP_IRQ_ENB, (CEP_SUPPKT | CEP_STS_END));
    if(inp32(PHY_CTL) & Vbus_status)
        outp32(PHY_CTL, (0x20 | Phy_suspend | vbus_detect));
}

VOID udcDeinit(void)
{
    outp32(PHY_CTL, (inp32(PHY_CTL) & ~vbus_detect));
    usbdInfo.USBModeFlag =0;
}

VOID udcClose(void)
{
    outp32(REG_AHBCLK, inp32(REG_AHBCLK) & ~USBD_CKE);
}

BOOL udcIsAttached(void)
{
    return((inp32(PHY_CTL) & Vbus_status) ? TRUE : FALSE);
}

BOOL udcIsAttachedToHost(void)
{
    return g_bHostAttached;
}

VOID usbd_update_device(void)
{
    /* update this device for set requests */
    switch(_usb_cmd_pkt.bRequest)
    {
        case USBR_SET_ADDRESS:
            outp32(ADDR, usbdInfo._usbd_address);
            break;

        case USBR_SET_CONFIGURATION:
            if(usbdInfo.i32EPA_Num != -1)
            {
                outp32(EPA_RSP_SC, EP_TOGGLE);
            }
            if(usbdInfo.i32EPB_Num != -1)
            {
                outp32(EPB_RSP_SC, EP_TOGGLE);
            }
            if(usbdInfo.i32EPC_Num != -1)
            {
                outp32(EPC_RSP_SC, EP_TOGGLE);
            }
            if(usbdInfo.i32EPD_Num != -1)
            {
                outp32(EPD_RSP_SC, EP_TOGGLE);
            }
            break;

        case USBR_SET_INTERFACE:
            break;

        case USBR_SET_FEATURE:
            if(usbdInfo._usbd_haltep == 0)
                outp32(CEP_CTRL_STAT, CEP_SEND_STALL);
            else if(usbdInfo.i32EPA_Num != -1 && usbdInfo._usbd_haltep == usbdInfo.i32EPA_Num )
            {
                outp32(EPA_RSP_SC, EP_HALT);
            }
            else if(usbdInfo.i32EPB_Num != -1 && usbdInfo._usbd_haltep == usbdInfo.i32EPB_Num )
            {
                outp32(EPB_RSP_SC, EP_HALT);
            }
            else if(usbdInfo.i32EPC_Num != -1 && usbdInfo._usbd_haltep == usbdInfo.i32EPC_Num )
            {
                outp32(EPC_RSP_SC,EP_HALT);
            }
            else if(usbdInfo.i32EPD_Num != -1 && usbdInfo._usbd_haltep == usbdInfo.i32EPD_Num )
            {
                outp32(EPD_RSP_SC, EP_HALT);
            }
            else if(usbdInfo.enableremotewakeup == 1)
            {
                usbdInfo.enableremotewakeup = 0;
                usbdInfo.remotewakeup = 1;
            }
            if(usbdInfo.enabletestmode == 1)
            {
                usbdInfo.enabletestmode = 0;
                usbdInfo.testmode = 1;
                if(usbdInfo.testselector == TEST_J)
                    outp32(USB_TEST, TEST_J);
                else if(usbdInfo.testselector==TEST_K)
                    outp32(USB_TEST, TEST_K);
                else if(usbdInfo.testselector==TEST_SE0_NAK)
                    outp32(USB_TEST, TEST_SE0_NAK);
                else if(usbdInfo.testselector==TEST_PACKET)
                    outp32(USB_TEST, TEST_PACKET);
                else if(usbdInfo.testselector==TEST_FORCE_ENABLE)
                    outp32(USB_TEST, TEST_FORCE_ENABLE);
            }
            break;

        case USBR_CLEAR_FEATURE:
            if(usbdInfo.i32EPA_Num != -1 && usbdInfo._usbd_unhaltep == usbdInfo.i32EPA_Num && usbdInfo._usbd_haltep == usbdInfo.i32EPA_Num)
            {
                outp32(EPA_RSP_SC, 0x0);
                usb_halt_ep = usb_halt_ep & ~0x01;
                outp32(EPA_RSP_SC, EP_TOGGLE);
                usbdInfo._usbd_haltep = -1; /* just for changing the haltep value */
            }
            if(usbdInfo.i32EPB_Num != -1 && usbdInfo._usbd_unhaltep == usbdInfo.i32EPB_Num && usbdInfo._usbd_haltep == usbdInfo.i32EPB_Num)
            {
                outp32(EPB_RSP_SC, 0x0);
                usb_halt_ep = usb_halt_ep & ~0x02;
                outp32(EPB_RSP_SC, EP_TOGGLE);
                usbdInfo._usbd_haltep = -1; /* just for changing the haltep value */
            }
            if(usbdInfo.i32EPC_Num != -1 && usbdInfo._usbd_unhaltep == usbdInfo.i32EPC_Num && usbdInfo._usbd_haltep == usbdInfo.i32EPC_Num)
            {
                outp32(EPC_RSP_SC, 0x0);
                usb_halt_ep = usb_halt_ep & ~0x04;
                outp32(EPC_RSP_SC, EP_TOGGLE);
                usbdInfo._usbd_haltep = -1; /* just for changing the haltep value */
            }
            if(usbdInfo.i32EPD_Num != -1 && usbdInfo._usbd_unhaltep == usbdInfo.i32EPD_Num && usbdInfo._usbd_haltep == usbdInfo.i32EPD_Num)
            {
                outp32(EPD_RSP_SC, 0x0);
                usb_halt_ep = usb_halt_ep & ~0x08;
                outp32(EPD_RSP_SC, EP_TOGGLE);
                usbdInfo._usbd_haltep = -1; /* just for changing the haltep value */
			}			
            else if(usbdInfo.disableremotewakeup == 1)
            {
                usbdInfo.disableremotewakeup=0;
                usbdInfo.remotewakeup=0;
            }
            break;
        default:
            break;
    }
}

VOID usbd_send_descriptor(void)
{
    UINT32 volatile temp_cnt;
    int volatile i;
    UINT32 volatile *ptr;
    UINT8 volatile *pRemainder;

    if ((usbdInfo.CLASS_CMD_Iflag==1) || (usbdInfo.GET_DEV_Flag == 1) || (usbdInfo.GET_QUL_Flag == 1) || 
        (usbdInfo.GET_CFG_Flag == 1) || (usbdInfo.GET_OSCFG_Flag == 1) || (usbdInfo.GET_STR_Flag == 1) ||
        (usbdInfo.usbdGetConfig == 1) || (usbdInfo.usbdGetInterface == 1) ||(usbdInfo.usbdGetStatus == 1)
        || (usbdInfo.GET_HID_Flag == 1) || (usbdInfo.GET_HID_RPT_Flag == 1)) 
    {
        if (usbdInfo._usbd_remlen_flag == 0)
        {
            if (usbdInfo.GET_DEV_Flag == 1)
                ptr = (UINT32 *)usbdInfo.pu32DevDescriptor;
            else if (usbdInfo.GET_QUL_Flag == 1)
                ptr =(UINT32 *)usbdInfo.pu32QulDescriptor;
            else if (usbdInfo.GET_CFG_Flag == 1)
            {
                if (usbdInfo._usbd_speedset == 2)
                    ptr = (UINT32 *)usbdInfo.pu32HSConfDescriptor;
                else if (usbdInfo._usbd_speedset == 1)
                    ptr = (UINT32 *)usbdInfo.pu32FSConfDescriptor;
            }
            else if (usbdInfo.GET_OSCFG_Flag == 1)
            {
                if (usbdInfo._usbd_speedset == 2)
                    ptr = (UINT32 *)usbdInfo.pu32HOSConfDescriptor;
                else if (usbdInfo._usbd_speedset == 1)
                    ptr = (UINT32 *)usbdInfo.pu32FOSConfDescriptor;
            }
            else if (usbdInfo.GET_STR_Flag == 1)
                ptr = (UINT32 *)usbdInfo.pu32StringDescriptor[_usb_cmd_pkt.wValue & 0x7];
            else if (usbdInfo.GET_HID_Flag == 1)
                ptr = (UINT32 *)usbdInfo.pu32HIDDescriptor;
            else if (usbdInfo.GET_HID_RPT_Flag == 1)
                ptr = (UINT32 *)usbdInfo.pu32HIDRPTDescriptor[_usb_cmd_pkt.wIndex & 0x7];								
            else if (usbdInfo.usbdGetConfig == 1)
            {
                /* Send Configuration Selector Data (1 bytes) to Host */
                outp8(CEP_DATA_BUF, usbdInfo._usbd_confsel);
                outp32(IN_TRNSFR_CNT, 1);
                return;
            }
            else if (usbdInfo.usbdGetInterface == 1)
            {
                /* Send Alternate Selector Data (1 bytes) to Host */
                outp8(CEP_DATA_BUF, usbdInfo._usbd_altsel);
                outp32(IN_TRNSFR_CNT, 1);
                return;
            }
            else if (usbdInfo.usbdGetStatus == 1)
            {
                /* Send Status (2 bytes) to Host */
                outp8(CEP_DATA_BUF, (UINT8)usbdInfo.usbdGetStatusData);
                outp8(CEP_DATA_BUF, 0);
                outp32(IN_TRNSFR_CNT, 2);
                return;
            }
            else if (usbdInfo.CLASS_CMD_Iflag == 1)
            {
                if(usbdInfo.u32UVC == 1 && usbdStatus.appConnected == 1)
                {
                    int volatile flag = 0;
                    int volatile i = 0;
                    int volatile count0 = 0, count1 = 0;
                    while(1)
                    {
                        count0 = inp32(EPA_DATA_CNT) & 0xFFFF;

                        for(i = 0; i < 5; i++)
                            ;
                        count1 = inp32(EPA_DATA_CNT) & 0xFFFF;

                        if(count0 == count1)
                            flag++;
                        else
                            flag = 0;

                        if(flag > 5)
                            break;
                    }
                }

                outp32(CEP_END_ADDR, 0x7F);
                /* Class Data In */
                if(usbdInfo.pfnClassDataINCallBack != NULL)
                    usbdInfo.pfnClassDataINCallBack();

                return;
            }
        }
        else
            ptr = usbdInfo._usbd_ptr;

        if (_usb_cmd_pkt.wLength > 0x40)
        {
            usbdInfo._usbd_remlen_flag = 1;
            usbdInfo._usbd_remlen = _usb_cmd_pkt.wLength - 0x40;
            _usb_cmd_pkt.wLength = 0x40;
        }
        else if (usbdInfo._usbd_remlen != 0)
        {
            if (usbdInfo._usbd_remlen > 0x40)
            {
                usbdInfo._usbd_remlen_flag = 1;
                usbdInfo._usbd_remlen = usbdInfo._usbd_remlen -0x40;
                _usb_cmd_pkt.wLength = 0x40;
            }
            else
            {
                usbdInfo._usbd_remlen_flag = 0;
                _usb_cmd_pkt.wLength = usbdInfo._usbd_remlen;
                usbdInfo._usbd_remlen = 0;
            }
        }
        else
        {
            usbdInfo._usbd_remlen_flag = 0;
            usbdInfo._usbd_remlen = 0;
        }

        temp_cnt = _usb_cmd_pkt.wLength / 4;

        for (i=0; i<temp_cnt; i++)
            outp32(CEP_DATA_BUF, *ptr++);

        temp_cnt = _usb_cmd_pkt.wLength % 4;

        if (temp_cnt)
        {
            pRemainder = (UINT8 *)ptr;
            for (i=0; i<temp_cnt; i++)
            {
                outp8(CEP_DATA_BUF, *pRemainder);
                pRemainder++;
            }
        }

        if (usbdInfo._usbd_remlen_flag)
            usbdInfo._usbd_ptr = (UINT32  *)ptr;

        outp32(IN_TRNSFR_CNT, _usb_cmd_pkt.wLength);
    }
}

VOID usbd_control_packet(void)
{
    UINT32  volatile temp;
    BOOL  volatile ReqErr = 0;

    /* Get Command */
    temp = inp32(SETUP1_0);
    _usb_cmd_pkt.bmRequestType = (UINT8)temp & 0xff;
    _usb_cmd_pkt.bRequest = (UINT8)(temp >> 8) & 0xff;
    _usb_cmd_pkt.wValue = (UINT16)inp32(SETUP3_2);
    _usb_cmd_pkt.wIndex = (UINT16)inp32(SETUP5_4);
    _usb_cmd_pkt.wLength = (UINT16)inp32(SETUP7_6);

    usbdClearAllFlags();

    usbdInfo.CLASS_CMD_Iflag = 0;
    usbdInfo.CLASS_CMD_Oflag = 0;

    if ((_usb_cmd_pkt.bmRequestType &0xE0) == 0xa0)     /* 0xA1 or 0xA2 is Class Get Request */
    {
        if (_usb_cmd_pkt.wLength == 0)
        {
            /* Class Data Out without Data */
            if(usbdInfo.pfnClassDataINCallBack != NULL)
            usbdInfo.pfnClassDataINCallBack();
            return;
        } 
        usbdInfo.CLASS_CMD_Iflag = 1;
        usbdInfo._usbd_DMA_In =1; 
        outp32(CEP_IRQ_STAT, CEP_IN_TK_IS);       /* BA+0x34 , clear status, In talken */
        outp32(CEP_IRQ_ENB, CEP_IN_TK_IE);        /* suppkt int ,status and in token */
        return;         
    }         
    else if ((_usb_cmd_pkt.bmRequestType &0xE0) == 0x20)     /* 0x21 or 0x22 is Class Set Request */
    {
        if(usbdInfo.u32UVC == 1 && usbdStatus.appConnected == 1)
        {
            int volatile flag = 0;
            int volatile i = 0;
            int volatile count0 = 0, count1 = 0;
            outp32(CEP_END_ADDR, 0x7F);
            while(1)
            {
                count0 = inp32(EPA_DATA_CNT) & 0xFFFF;

                for(i = 0; i < 5; i++)
                    ;
                count1 = inp32(EPA_DATA_CNT) & 0xFFFF;

                if(count0 == count1)
                    flag++;
                else
                    flag = 0;

                if(flag > 5)
                    break;
            }
        }
        usbdInfo.CLASS_CMD_Oflag = 1; 

        if (_usb_cmd_pkt.wLength == 0)
        {
            /* Class Data Out without Data */
            if(usbdInfo.pfnClassDataOUTCallBack != NULL)
                usbdInfo.pfnClassDataOUTCallBack();
            outp32(CEP_IRQ_STAT, CEP_STACOM_IS);
            outp32(CEP_CTRL_STAT, CEP_NAK_CLEAR);    /* clear nak so that sts stage is complete */
            outp32(CEP_IRQ_ENB,  (CEP_STACOM_IE|CEP_PING_IE|CEP_SETUP_PK_IE));    /* suppkt int enb/sts completion int */
        }
        else
            outp32(CEP_IRQ_ENB,  CEP_NAK_IE | CEP_DATA_RxED_IE); /* OUT_TK_IE */

        return;
    }
    switch (_usb_cmd_pkt.bRequest)
    {
        case USBR_GET_DESCRIPTOR:
            switch ((_usb_cmd_pkt.wValue & 0xff00) >> 8)
            {
                case USB_DT_DEVICE:
                    /* clear flags */
                    usbdClearAllFlags();
                    usbdInfo.GET_DEV_Flag = 1;
                    usbdInfo.USBModeFlag=1;  /* acpi debug */
                    if (_usb_cmd_pkt.wLength > usbdInfo.u32DevDescriptorLen)
                        _usb_cmd_pkt.wLength = usbdInfo.u32DevDescriptorLen;
                    break;

                case USB_DT_CONFIG:
                    usbdClearAllFlags();
                    usbdInfo.GET_CFG_Flag = 1;
                    {
                        if (usbdInfo._usbd_speedset == 2)
                        {
                            if (_usb_cmd_pkt.wLength > usbdInfo.u32HSConfDescriptorLen)
                                _usb_cmd_pkt.wLength = usbdInfo.u32HSConfDescriptorLen;
                        }
                        else
                        {
                            if (_usb_cmd_pkt.wLength > usbdInfo.u32FSConfDescriptorLen)
                                _usb_cmd_pkt.wLength = usbdInfo.u32FSConfDescriptorLen;
                        }
                    }
                    break;
                case USB_DT_QUALIFIER:    /* high-speed operation */
                    usbdClearAllFlags();
                    usbdInfo.GET_QUL_Flag = 1;
                    if (_usb_cmd_pkt.wLength > usbdInfo.u32QulDescriptorLen)
                        _usb_cmd_pkt.wLength = usbdInfo.u32QulDescriptorLen;
                    break;
                case USB_DT_OSCONFIG:  /* other speed configuration */
                    usbdClearAllFlags();
                    usbdInfo.GET_OSCFG_Flag = 1;

                    if (usbdInfo._usbd_speedset == 2)
                    {
                        if (_usb_cmd_pkt.wLength > usbdInfo.u32HOSConfDescriptorLen)
                            _usb_cmd_pkt.wLength = usbdInfo.u32HOSConfDescriptorLen;
                    }
                    else if (usbdInfo._usbd_speedset == 1)
                    {
                        if (_usb_cmd_pkt.wLength > usbdInfo.u32FOSConfDescriptorLen)
                            _usb_cmd_pkt.wLength = usbdInfo.u32FOSConfDescriptorLen;
                    }
                    break;

                case USB_DT_STRING:
                    usbdClearAllFlags();
                    usbdInfo.GET_STR_Flag = 1;
                    if (_usb_cmd_pkt.wLength > usbdInfo.u32StringDescriptorLen[_usb_cmd_pkt.wValue & 0x7])
                        _usb_cmd_pkt.wLength = usbdInfo.u32StringDescriptorLen[_usb_cmd_pkt.wValue & 0x7];
                    break;
                case USBD_DT_HID:
                    usbdClearAllFlags();
                    usbdInfo.GET_HID_Flag = 1;
                    if (_usb_cmd_pkt.wLength > usbdInfo.u32HIDDescriptorLen)
                        _usb_cmd_pkt.wLength = usbdInfo.u32HIDDescriptorLen;
                    break;
                case USBD_DT_HID_RPT:
                    usbdClearAllFlags();
					//sysprintf("USB_DT_HID_RPT\n");
                    usbdInfo.GET_HID_RPT_Flag = 1;
                    usbdStatus.appConnected = 1;
                    if (_usb_cmd_pkt.wLength > usbdInfo.u32HIDRPTDescriptorLen[_usb_cmd_pkt.wIndex & 0x7])
                        _usb_cmd_pkt.wLength = usbdInfo.u32HIDRPTDescriptorLen[_usb_cmd_pkt.wIndex & 0x7];
                    break;
                default:
                    ReqErr=1;
                    break;
            }  /* end of switch */
            if(_usb_cmd_pkt.wLength % 64 == 0)
                zeropacket_flag = 1;
            else
                zeropacket_flag = 0;
            if (ReqErr == 0)
            {
                outp32(CEP_IRQ_STAT, CEP_IN_TK_IS);
                outp32(CEP_IRQ_ENB, CEP_IN_TK_IE);    /* suppkt int ,status and in token */
            }
            break;

        case USBR_SET_ADDRESS:
            ReqErr = ((_usb_cmd_pkt.bmRequestType == 0) && ((_usb_cmd_pkt.wValue & 0xff00) == 0)
                        && (_usb_cmd_pkt.wIndex == 0) && (_usb_cmd_pkt.wLength == 0)) ? 0 : 1;

            if ((_usb_cmd_pkt.wValue & 0xffff) > 0x7f)
                ReqErr=1;  /* Devaddr > 127 */

            if (usbdInfo._usbd_devstate == 3)
                ReqErr=1;  /* Dev is configured */

            if (ReqErr==1) 
                break;
            if(usbdInfo._usbd_devstate == 2)
            {
                if(_usb_cmd_pkt.wValue == 0)
                    usbdInfo._usbd_devstate = 1;    /* enter default state */
                usbdInfo._usbd_address = _usb_cmd_pkt.wValue;  /* if wval !=0,use new address */
            }

            if(usbdInfo._usbd_devstate == 1)
            {
                if(_usb_cmd_pkt.wValue != 0)
                {
                    usbdInfo._usbd_address = _usb_cmd_pkt.wValue;
                    usbdInfo._usbd_devstate = 2;
                }
            }
            outp32(CEP_IRQ_STAT, CEP_STACOM_IS);
            outp32(CEP_CTRL_STAT, CEP_NAK_CLEAR);    /* clear nak so that sts stage is complete */
            outp32(CEP_IRQ_ENB, CEP_STACOM_IE);      /* enable status complete interrupt */
            break;

        case USBR_GET_CONFIGURATION:
            ReqErr = ((_usb_cmd_pkt.bmRequestType == 0x80) && (_usb_cmd_pkt.wValue == 0) &&
                        (_usb_cmd_pkt.wIndex == 0) && (_usb_cmd_pkt.wLength == 0x1) ) ? 0 : 1;

            if (ReqErr==1)
                break;

            usbdClearAllFlags();
            usbdInfo.usbdGetConfig=1;
            outp32(CEP_IRQ_STAT, CEP_IN_TK_IS);
            outp32(CEP_IRQ_ENB, CEP_IN_TK_IE);    /* status and in token */
            break;

        case USBR_SET_CONFIGURATION:
            g_u32Suspend_Flag = 1;
            ReqErr = ((_usb_cmd_pkt.bmRequestType == 0) && ((_usb_cmd_pkt.wValue & 0xff00) == 0) &&
                ((_usb_cmd_pkt.wValue & 0x80) == 0) && (_usb_cmd_pkt.wIndex == 0) && 
                (_usb_cmd_pkt.wLength == 0)) ? 0 : 1;

            if (usbdInfo._usbd_devstate == 1)
                ReqErr=1; /* Device is in Default state */

            if ((_usb_cmd_pkt.wValue != 1) && (_usb_cmd_pkt.wValue != 0) )  /* Only configuration one is supported */
                ReqErr=1;  /* Configuration choosed is invalid */

            if(ReqErr==1)
                break;

            if (_usb_cmd_pkt.wValue == 0)
            {
                usbdInfo._usbd_confsel = 0;
                usbdInfo._usbd_devstate = 2;
            }
            else
            {
                usbdInfo._usbd_confsel = _usb_cmd_pkt.wValue;
                usbdInfo._usbd_devstate = 3;
            }
            usbdInfo.USBStartFlag = 1;
            outp32(CEP_IRQ_STAT, CEP_STACOM_IS);
            outp32(CEP_CTRL_STAT, CEP_NAK_CLEAR);     /* clear nak so that sts stage is complete */
            outp32(CEP_IRQ_ENB, CEP_STACOM_IE);       /* suppkt int ,status and in token */
            break;

        case USBR_GET_INTERFACE:
            ReqErr = ((_usb_cmd_pkt.bmRequestType == 0x81) && (_usb_cmd_pkt.wValue == 0)
                    && (_usb_cmd_pkt.wLength == 0x1)) ? 0 : 1;

            if ((usbdInfo._usbd_devstate == 1) || (usbdInfo._usbd_devstate == 2))
                ReqErr=1; /* Device state is not valid */

            if(ReqErr == 1) 
                break;    /*  Request Error */

            usbdClearAllFlags();
            usbdInfo.usbdGetInterface = 1;
            outp32(CEP_IRQ_STAT, CEP_IN_TK_IS);
            outp32(CEP_IRQ_ENB, CEP_IN_TK_IE);    /* status and in token */
            break;

        case USBR_SET_INTERFACE:     
            usbdInfo._usbd_altsel = _usb_cmd_pkt.wValue;
            usbdInfo._usbd_intfsel = _usb_cmd_pkt.wIndex;
            if(usbdInfo.u32UVC)
            {
                if ( usbdInfo._usbd_intfsel == 1 )  /* for video interface */
                {
                    if (_usb_cmd_pkt.wValue != 0)   /* Set alternative interface Video */
                    {
                        usbdInfo.AlternateFlag=1;
                        usbdStatus.appConnected = 1;
                        outp32(CEP_END_ADDR, 0x0);
                        //sysprintf("Interface 1, alter 1\n");
                    }
                    else
                    {
                        outp32(DMA_CTRL_STS, inp32(DMA_CTRL_STS)|0x00000080);  /* Reset DMA */
                        outp32(DMA_CTRL_STS, inp32(DMA_CTRL_STS)&0x0000007F);
                        outp32(EPA_RSP_SC, BUF_FLUSH);  /* flush fifo */
                        usbdInfo._usbd_DMA_In =1;                
                        usbdInfo.AlternateFlag=0;
                        usbdStatus.appConnected = 0;
                        outp32(CEP_END_ADDR, 0x7F);     
                        //sysprintf("Interface 1, alter 0\n");
                    }
                }
                else if ( usbdInfo._usbd_intfsel == 3 )  /* for audio microphone interface */
                {
                    if (_usb_cmd_pkt.wValue == 1)        /* for audio settng of Isochronous In */
                    {
                        usbdInfo.AlternateFlag_Audio=1;
                        usbdStatus.appConnected_Audio = 1;
                        //sysprintf("Setup Interface 3, alter 1\n");
                    }
                    else
                    {
                        usbdInfo.AlternateFlag_Audio=0;
                        usbdStatus.appConnected_Audio = 0;
                        //sysprintf("Setup Interface 3, alter 0\n");
                    }
                }
                else
                {
                    //outp32(DMA_CTRL_STS, inp32(DMA_CTRL_STS)|0x00000080);  /* Reset DMA */
                    //outp32(DMA_CTRL_STS, inp32(DMA_CTRL_STS)&0x0000007F);    
                    //usbdInfo._usbd_DMA_In =1;    
                    //sysprintf("Interface no, alter no\n");
                }  
            }
            else
            {
                if (_usb_cmd_pkt.wValue !=0)
                {
                    usbdInfo.AlternateFlag=1;
                    usbdStatus.appConnected = 1;
                    outp32(CEP_START_ADDR, 0);
                    outp32(CEP_END_ADDR, 0x0);
                }
                else
                {
                    outp32(DMA_CTRL_STS, inp32(DMA_CTRL_STS)|0x00000080);    /* Reset DMA */
                    outp32(DMA_CTRL_STS, inp32(DMA_CTRL_STS)&0x0000007F);    
                    usbdInfo._usbd_DMA_In =1;
                    usbdInfo.AlternateFlag=0;
                    usbdStatus.appConnected = 0;
                }
            }
            outp32(CEP_IRQ_STAT, CEP_STACOM_IS);
            outp32(CEP_CTRL_STAT, CEP_NAK_CLEAR);  /* clear nak so that sts stage is complete */
            outp32(CEP_IRQ_ENB, CEP_STACOM_IE);    /* suppkt int ,status and in token */
            break;

        case USBR_SET_FEATURE:
            if (usbdInfo._usbd_devstate == 1)
            {
                if((_usb_cmd_pkt.bmRequestType & 0x3) == 0x0)  /* Receipent is Device */
                {
                    if((_usb_cmd_pkt.wValue & 0x3) == TEST_MODE)
                    {
                        usbdInfo.enabletestmode = 1;
                        usbdInfo.testselector = (_usb_cmd_pkt.wIndex >> 8);
                        outp32(CEP_IRQ_STAT, CEP_STACOM_IS);
                        outp32(CEP_IRQ_ENB, CEP_STACOM_IE);    /* suppkt int ,status and in token */
                    }
                }
                else
                    ReqErr=1; /* Device is in Default State */
            }

            if (usbdInfo._usbd_devstate == 2)
            {
                if ((_usb_cmd_pkt.bmRequestType & 0x3 == 2) && ((_usb_cmd_pkt.wIndex & 0xff) != 0))  /* ep but not cep */
                    ReqErr =1; /* Device is in Addressed State, but for noncep */
                else if ((_usb_cmd_pkt.bmRequestType & 0x3) == 0x1)
                    ReqErr=1; /* Device is in Addressed State, but for interfac */
            }

            if (ReqErr == 1) 
                break;  /* Request Error */

            /* check if recipient and wvalue are appropriate */
            switch(_usb_cmd_pkt.bmRequestType & 0x3)
            {
                case 0:  /* device */
                    if ((_usb_cmd_pkt.wValue & 0x3) == DEVICE_REMOTE_WAKEUP)
                        usbdInfo.enableremotewakeup = 1;
                    else if((_usb_cmd_pkt.wValue & 0x3) == TEST_MODE)
                    {
                        usbdInfo.enabletestmode = 1;
                        usbdInfo.testselector = (_usb_cmd_pkt.wIndex >> 8);
                    }
                    else
                        ReqErr=1; /* No such feature for device */
                    break;
                case 1:  /* interface */
                    break;
                case 2:  /* endpoint */
                    if((_usb_cmd_pkt.wValue & 0x3) == ENDPOINT_HALT)
                    {
                        if((_usb_cmd_pkt.wIndex & 0xF) == 0)   /* endPoint zero */
                            usbdInfo._usbd_haltep = 0;
                        else if((_usb_cmd_pkt.wIndex & 0xF) == usbdInfo.i32EPA_Num)  /* endPoint A */
                            usbdInfo._usbd_haltep = usbdInfo.i32EPA_Num;
                        else if((_usb_cmd_pkt.wIndex & 0xF) == usbdInfo.i32EPB_Num)  /* endPoint B */
                            usbdInfo._usbd_haltep = usbdInfo.i32EPB_Num;
                        else if((_usb_cmd_pkt.wIndex & 0xF) == usbdInfo.i32EPC_Num)  /* endPoint C */
                            usbdInfo._usbd_haltep = usbdInfo.i32EPC_Num;
                        else if((_usb_cmd_pkt.wIndex & 0xF) == usbdInfo.i32EPD_Num)  /* endPoint D */
                            usbdInfo._usbd_haltep = usbdInfo.i32EPD_Num;
                        else
                            ReqErr=1; /* Selected endpoint was not present */
                    }
                    else
                        ReqErr=1; /* Neither device,endpoint nor interface was choosen */

                    break;

                default:
                    break;
            } /* device */

            outp32(CEP_IRQ_STAT, CEP_STACOM_IS);
            outp32(CEP_CTRL_STAT, CEP_NAK_CLEAR);  /* clear nak so that sts stage is complete */
            outp32(CEP_IRQ_ENB, CEP_STACOM_IE);    /* suppkt int ,status and in token */
            break;

        case USBR_CLEAR_FEATURE:
            ReqErr = (((_usb_cmd_pkt.bmRequestType & 0xfc) == 0) && ((_usb_cmd_pkt.wValue & 0xfffc) == 0) 
                    && ((_usb_cmd_pkt.wIndex & 0xff00) == 0) && (_usb_cmd_pkt.wLength == 0)) ? 0 : 1;

            if (usbdInfo._usbd_devstate == 1) 
                ReqErr =1; /* Device is in default state */

            if (usbdInfo._usbd_devstate == 2)
            {
                if((_usb_cmd_pkt.bmRequestType == 2) && (_usb_cmd_pkt.wIndex != 0)) /* ep but not cep */
                    ReqErr =1; /* Device is in Addressed State, but for noncep */
                else if(_usb_cmd_pkt.bmRequestType == 0x1)  /*recip is interface */
                    ReqErr=1;  /* Device is in Addressed State, but for interface */
            }
            if(ReqErr == 1) 
                break;  /* Request Error */

            switch((_usb_cmd_pkt.bmRequestType & 0x3))
            {
                case 0:  /* device */ 
                    if((_usb_cmd_pkt.wValue & 0x3) == DEVICE_REMOTE_WAKEUP)
                        usbdInfo.disableremotewakeup = 1;
                    else
                        ReqErr=1;  /* No such feature for device */
                    break;
                case 1:  /* interface */
                    break;
                case 2:  /* endpoint */
                    if((_usb_cmd_pkt.wValue & 0x3) == ENDPOINT_HALT)
                    {
                        if((_usb_cmd_pkt.wIndex & 0xF) == 0)   /* endPoint zero */
                            usbdInfo._usbd_unhaltep = 0;
                        else if((_usb_cmd_pkt.wIndex & 0xF) == usbdInfo.i32EPA_Num)  /* endPoint A */
                            usbdInfo._usbd_unhaltep = usbdInfo.i32EPA_Num;
                        else if((_usb_cmd_pkt.wIndex & 0xF) == usbdInfo.i32EPB_Num)  /* endPoint B */
                            usbdInfo._usbd_unhaltep = usbdInfo.i32EPB_Num;
                        else if((_usb_cmd_pkt.wIndex & 0xF) == usbdInfo.i32EPC_Num)  /* endPoint C */
                            usbdInfo._usbd_unhaltep = usbdInfo.i32EPC_Num;
                        else if((_usb_cmd_pkt.wIndex & 0xF) == usbdInfo.i32EPD_Num)  /* endPoint D */
                            usbdInfo._usbd_unhaltep = usbdInfo.i32EPD_Num;
                        else
                            ReqErr=1;  /* endpoint choosen was not supported */
                    }
                    else
                        ReqErr=1;  /* Neither device,interface nor endpoint was choosen */
                    break;

                default:
                    break;
            }  /* device */

            outp32(CEP_IRQ_STAT, CEP_STACOM_IS);
            outp32(CEP_CTRL_STAT, CEP_NAK_CLEAR);  /* clear nak so that sts stage is complete */
            outp32(CEP_IRQ_ENB, CEP_STACOM_IE);
            break;

        case USBR_GET_STATUS:
            /* check if this is valid */
            ReqErr = (((_usb_cmd_pkt.bmRequestType & 0xfc) == 0x80) && (_usb_cmd_pkt.wValue == 0) 
                    && ((_usb_cmd_pkt.wIndex & 0xff00) == 0) && (_usb_cmd_pkt.wLength == 0x2)) ? 0 : 1;

            if (usbdInfo._usbd_devstate == 1)
                ReqErr =1;  /* Device is in default State */

            if (usbdInfo._usbd_devstate == 2)
            {
                if ((_usb_cmd_pkt.bmRequestType & 0x3 == 0x2) && (_usb_cmd_pkt.wIndex != 0))
                    ReqErr =1;  /* Device is in Addressed State, but for noncep */
                else if (_usb_cmd_pkt.bmRequestType & 0x3 == 0x1)
                    ReqErr =1;  /* Device is in Addressed State, but for interface */
            }

            if (ReqErr == 1)
                break;  /* Request Error */

            usbdClearAllFlags();
            usbdInfo.usbdGetStatus=1;

            switch (_usb_cmd_pkt.bmRequestType & 0x3)
            {
                case 0:
                    if (usbdInfo.remotewakeup == 1)
                        usbdInfo.usbdGetStatusData = 0x3;
                    else 
                        usbdInfo.usbdGetStatusData = 0x1;
                    break;
                case 1:   /* interface */
                    if (_usb_cmd_pkt.wIndex == 0)
                        usbdInfo.usbdGetStatusData = 0;  /* Status of interface zero */
                    else
                        ReqErr=1;  /* Status of interface non zero */  
                    break;
                case 2:  /* endpoint */
                    if (_usb_cmd_pkt.wIndex == 0x0)
                        usbdInfo.usbdGetStatusData = 0;  /* Status of Endpoint zero */
                    else if (((_usb_cmd_pkt.wIndex & 0xF) == usbdInfo.i32EPA_Num )||
                            ((_usb_cmd_pkt.wIndex & 0xF) == usbdInfo.i32EPB_Num )||
                            ((_usb_cmd_pkt.wIndex & 0xF) == usbdInfo.i32EPC_Num )||
                            ((_usb_cmd_pkt.wIndex & 0xF) == usbdInfo.i32EPD_Num )
                            )
                    {
                        /* Status of Endpoint one */
                        if (usbdInfo._usbd_haltep == (_usb_cmd_pkt.wIndex & 0xF))
                            usbdInfo.usbdGetStatusData = 0x1;
                        else
                            usbdInfo.usbdGetStatusData = 0;
                    }
                    else
                        ReqErr=1;  /* Status of non-existing Endpoint */
                    break;

                default:
                    break;
            }
            outp32(CEP_IRQ_STAT, CEP_IN_TK_IS);
            outp32(CEP_IRQ_ENB, CEP_IN_TK_IE);    /* suppkt int ,status and in token */
            break;

        default:
            break;
    }
    if (ReqErr == 1)
    {
        outp32(CEP_IRQ_ENB, (inp32(CEP_IRQ_ENB) | (CEP_SETUP_TK_IE |CEP_SETUP_PK_IE)));
        outp32(CEP_CTRL_STAT, CEP_SETUP_PK_IS);

    }
}        

VOID usbd_isr(void)
{
    UINT32 volatile IrqStL, IrqEnL, IrqSt, IrqEn;

    IrqStL = inp32(IRQ_STAT_L);    /* Wnat kind od EP's interrupt */
    IrqEnL = inp32(IRQ_ENB_L);    /* Which EP'S interrupt is enable */

    if (!(IrqStL & IrqEnL))
        return;

    /* USB interrupt */
    if (IrqStL & IRQ_USB_STAT)
    {
        IrqSt = inp32(USB_IRQ_STAT);
        IrqEn = inp32(USB_IRQ_ENB);

        if(IrqSt & USB_VBUS & IrqEn)
        {
            if(inp32(PHY_CTL) & Vbus_status)
            {
                usbdInfo._usbd_devstate = 1;    /* default state */
                usbdInfo._usbd_address = 0;    /* zero */
                if(usbdInfo.pfnPlug!=NULL)
                    usbdInfo.pfnPlug();
                usbdInfo.u32VbusStatus = 1;
                usbdStatus.usbConnected = 1;
                outp32(PHY_CTL, (0x20 | Phy_suspend | vbus_detect));
            }
            else
            {
                if(usbdInfo.pfnUnplug!=NULL)
                    usbdInfo.pfnUnplug();
                usbdInfo.u32VbusStatus = 0;
                usbdStatus.usbConnected = 0;
                usbdInfo.USBModeFlag =0;
                usbdInfo.AlternateFlag = 0;
                usbdStatus.appConnected = 0;
                usbdInfo.AlternateFlag_Audio = 0;
                usbdStatus.appConnected_Audio = 0;
                usbdClearAllFlags();
                g_bHostAttached = FALSE;
                g_u32Suspend_Flag = 0;
                outp32(PHY_CTL, (0x20 | Phy_suspend));
            }
            outp32(USB_IRQ_STAT, VBUS_IS);
        }
        if (IrqSt & SOF_IS & IrqEn)
        {
            if(usbdInfo.pfnSOF!=NULL)
                usbdInfo.pfnSOF();
            g_bHostAttached = TRUE;
            outp32(USB_IRQ_STAT, SOF_IS);
        }

        if (IrqSt & RST_IS & IrqEn)
        {
            usbdInfo._usbd_devstate = 0;
            usbdInfo._usbd_speedset = 0;
            usbdInfo._usbd_address = 0;

            /* clear flags */
            usbdClearAllFlags();
            usbdInfo.USBModeFlag=0;
            usbdInfo._usbd_DMA_Flag=0;
            usbdInfo._usbd_DMA_In=1;
            g_bHostAttached = TRUE;
            /* reset dma */
            if(usbdInfo.pfnReset!=NULL)
                usbdInfo.pfnReset();

            outp32(DMA_CTRL_STS, RST_DMA);
            {
                UINT32 iii;
                for (iii=0; iii<0x100; iii++);
            }
            outp32(DMA_CTRL_STS, 0);

            usbdInfo._usbd_devstate = 1;    /* default state */
            usbdInfo._usbd_address = 0;     /* zero */
#ifdef __USBD_STANDARD_MODE__
            if (inp32(OPER) & CUR_SPD)      /* 1:High Speed ; 0:Full speed */
            {
                usbdInfo._usbd_speedset = 2;
                usbdInfo.pfnHighSpeedInit();
            }
            else
            {
                usbdInfo._usbd_speedset = 1;
                usbdInfo.pfnFullSpeedInit();
            }
#else

    #ifdef __USBD_HIGH_SPEED_MODE__
            {
                usbdInfo._usbd_speedset = 2;    /* for high speed */
                usbdInfo.pfnHighSpeedInit();
            }
    #endif

    #ifdef __USBD_FULL_SPEED_MODE__
            {
                usbdInfo._usbd_speedset = 1;    /* or full speed */
                usbdInfo.pfnFullSpeedInit();
            }
    #endif
#endif

            outp32(CEP_IRQ_ENB, CEP_SETUP_PK_IE);    /* enable stetup interrupt */

            if(usbdInfo.i32EPA_Num != -1)
            {
                outp32(EPA_RSP_SC, 0);
                outp32(EPA_RSP_SC, BUF_FLUSH);    /* flush fifo */
                outp32(EPA_RSP_SC, TOGGLE);
            }
            if(usbdInfo.i32EPB_Num != -1)
            {
                outp32(EPB_RSP_SC, 0);
                outp32(EPB_RSP_SC, BUF_FLUSH);    /* flush fifo */
                outp32(EPB_RSP_SC, TOGGLE);
            }
            if(usbdInfo.i32EPC_Num != -1)
            {
                outp32(EPC_RSP_SC, 0);
                outp32(EPC_RSP_SC, BUF_FLUSH);    /* flush fifo */
                outp32(EPC_RSP_SC, TOGGLE);
            }
            if(usbdInfo.i32EPD_Num != -1)
            {
                outp32(EPD_RSP_SC, 0);
                outp32(EPD_RSP_SC, BUF_FLUSH);    /* flush fifo */
                outp32(EPD_RSP_SC, TOGGLE);
            }

            outp32(ADDR, 0);
            outp32(USB_IRQ_STAT, RST_IS);
            outp32(USB_IRQ_ENB, (RST_IE|SUS_IE|RUM_IE|VBUS_IE));
            outp32(CEP_IRQ_STAT, ~(CEP_SETUP_TK_IS | CEP_SETUP_PK_IS));
        }

        if (IrqSt & RUM_IS & IrqEn)
        {                     
            usbdInfo._usbd_resume = 1;
            outp32(USB_IRQ_STAT, RUM_IS);    /* Resume */
            g_bHostAttached = TRUE;
            if(pfnResume != NULL)
                pfnResume();          
            g_u32Suspend_Flag = 1;
            outp32(USB_IRQ_ENB, (USB_RST_STS|USB_SUS_REQ|VBUS_IE));
        }

        if (IrqSt & SUS_IS & IrqEn)
        {
            int volatile i;
            int volatile test;      
            usbdInfo._usbd_resume = 0;
            g_bHostAttached = TRUE;
            outp32(USB_IRQ_ENB, (USB_RST_STS|USB_RESUME|VBUS_IE));

            test = inp32(PHY_CTL) & Vbus_status;
            for(i=0;i<0x40000;i++)
            {
                outp32(USB_IRQ_STAT, SOF_IS);
                outp32(CEP_IRQ_STAT, CEP_NAK_IS);
                if(test != (inp32(PHY_CTL) & Vbus_status))
                {
                    if(inp32(PHY_CTL) & Vbus_status)
                    {
                        usbdInfo.u32VbusStatus = 1;
                        usbdStatus.usbConnected = 1;
                        outp32(PHY_CTL, (0x20 | Phy_suspend | vbus_detect));
                    }
                    else
                    {
                        outp32(USB_IRQ_ENB, (USB_RST_STS|USB_RESUME|VBUS_IE));
                        usbdInfo.u32VbusStatus = 0;
                        usbdStatus.usbConnected = 0;
                        usbdInfo.USBModeFlag =0;
                        usbdInfo.AlternateFlag = 0;
                        usbdStatus.appConnected = 0;
                        usbdInfo.AlternateFlag_Audio = 0;
                        usbdStatus.appConnected_Audio = 0;
                        outp32(PHY_CTL, (0x20 | Phy_suspend));
                        g_u32Suspend_Flag = 0;
                       // sysprintf("Unplug(S)!!\n");
                        outp32(USB_IRQ_ENB, (USB_RST_STS|USB_RESUME|VBUS_IE|USB_SUS_REQ));
                        outp32(USB_IRQ_STAT, SUS_IS);    /* Suspend */
                        outp32(USB_IRQ_ENB, (USB_RST_STS|USB_RESUME|VBUS_IE));
                    }
                    outp32(USB_IRQ_STAT, VBUS_IS);
                    return;
                }
                if((inp32(CEP_IRQ_STAT) & CEP_NAK_IS) ||(inp32(USB_IRQ_STAT) & SOF_IS))
                {
                    outp32(USB_IRQ_ENB, (USB_RST_STS|USB_RESUME|VBUS_IE));
                    outp32(USB_IRQ_STAT, SUS_IS);    /* Suspend */
                    return;
                }
            }
            outp32(USB_IRQ_ENB, (USB_RST_STS|USB_RESUME|VBUS_IE|USB_SUS_REQ));
            outp32(USB_IRQ_STAT, SUS_IS);    /* Suspend */

            if(usbdInfo.u32UVC)
            {
                if(pfnSuspend !=NULL)
                {
                    pfnSuspend();
                    g_u32Suspend_Flag = 0;
                }
            }
            else
            {
                if(g_u32Suspend_Flag == 1 && pfnSuspend !=NULL)
                {
                    pfnSuspend();
                    g_u32Suspend_Flag = 0;
                }
            }
            outp32(USB_IRQ_ENB, (USB_RST_STS|USB_RESUME|VBUS_IE));
        }

        if (IrqSt & HISPD_IS & IrqEn)
        {
            usbdInfo._usbd_devstate = 1;            /* default state */
            usbdInfo._usbd_speedset = 2;            /* for high speed */
            usbdInfo._usbd_address = 0;             /* zero */
            g_bHostAttached = TRUE;
            outp32(CEP_IRQ_ENB, CEP_SETUP_PK_IE);   /* enable stetup interrupt */
            outp32(USB_IRQ_STAT, HISPD_IS);
        }

        if (IrqSt & DMACOM_IS & IrqEn)    /* DMA Completion */
        {
            usbdInfo._usbd_DMA_Flag = 1;
            g_bHostAttached = TRUE;
            outp32(USB_IRQ_STAT, DMACOM_IS);
            if(usbdInfo.pfnDMACompletion!=NULL)
                usbdInfo.pfnDMACompletion();
        }

        if (IrqSt & TCLKOK_IS & IrqEn)
            outp32(USB_IRQ_STAT, TCLKOK_IS);

        outp32(IRQ_STAT_L, IRQ_USB_STAT);
    }
    /* Control Endpoint Interrupt */
    if (IrqStL & IRQ_CEP) 
    {
        IrqSt = inp32(CEP_IRQ_STAT);
        IrqEn = inp32(CEP_IRQ_ENB);
        g_bHostAttached = TRUE;
        //sysprintf("Control IRQ status = %x, ControlIRQ Enable=%x\n", IrqSt, IrqEn);
        if (IrqSt & CEP_SUPTOK & IrqEn)    /* SETUP TOKEN */
        {
            outp32(CEP_IRQ_STAT, CEP_SETUP_TK_IS);
            return;
        }

        if (IrqSt & CEP_SUPPKT & IrqEn)    /* SETUP PACKET */
        {
            outp32(CEP_IRQ_STAT, CEP_SETUP_PK_IS);
            usbd_control_packet();    /* Setup Packet */
            return;
        }

        if (IrqSt & CEP_OUT_TOK & IrqEn)    /* OUT TOKEN */
        {
            outp32(CEP_IRQ_STAT, CEP_OUT_TK_IS);
            return;
        }

        if (IrqSt & CEP_IN_TOK & IrqEn)
        {
            /* IN TOKEN */
            outp32(CEP_IRQ_STAT, CEP_IN_TK_IS);
            if (!(IrqSt & CEP_STACOM_IS))
            {
                outp32(CEP_IRQ_STAT, CEP_DATA_TxED_IS);
                outp32(CEP_IRQ_ENB, CEP_DATA_TxED_IE);
                usbd_send_descriptor();    /* Send DAESCRIPTOR */
            }
            else
            {
                outp32(CEP_IRQ_STAT, CEP_DATA_TxED_IS);
                outp32(CEP_IRQ_ENB, (CEP_STACOM_IE | CEP_DATA_TxED_IE));
            }
            return;
        }

        if (IrqSt & CEP_PING_TOK & IrqEn)
        {
            outp32(CEP_IRQ_STAT, CEP_PING_IS);
            return;
        }

        if (IrqSt & CEP_DATA_TXD & IrqEn)
        {
            outp32(CEP_IRQ_STAT, CEP_DATA_TxED_IS); 

            if (usbdInfo._usbd_remlen_flag)
            {
                outp32(CEP_IRQ_STAT, CEP_IN_TK_IS);
                outp32(CEP_IRQ_ENB, CEP_IN_TK_IE);    /* suppkt int ,status and in token */
            }
            else
            {
                if(zeropacket_flag)
                {
                    zeropacket_flag = 0;
                    outp32(CEP_CTRL_STAT, CEP_ZEROLEN);
                    outp32(CEP_IRQ_STAT, CEP_IN_TK_IS);
                    outp32(CEP_IRQ_ENB, CEP_IN_TK_IE);    /* suppkt int ,status and in token */
                    return;
                }
                outp32(CEP_IRQ_STAT, CEP_STACOM_IS);
                outp32(CEP_CTRL_STAT, CEP_NAK_CLEAR);    /* clear nak so that sts stage is complete */
                outp32(CEP_IRQ_ENB, CEP_SETUP_PK_IE|CEP_STACOM_IE);     /* suppkt int ,status and in token */
            }         
            if(usbdStatus.appConnected == 1)
            {
                if(usbdInfo.u32UVC)
                outp32(CEP_END_ADDR, 0x0);
            }
            return;
        }

        if (IrqSt & CEP_DATA_RXD & IrqEn)
        {
            outp32(CEP_IRQ_STAT, (CEP_DATA_RxED_IS | CEP_NAK_IS));
            /* Data Packet receive(OUT) */
            if (usbdInfo.CLASS_CMD_Oflag && usbdInfo.pfnClassDataOUTCallBack != NULL)
            {
                class_out_len = _usb_cmd_pkt.wLength;
                class_out_len -= inp32(OUT_TRNSFR_CNT);
                usbdInfo.pfnClassDataOUTCallBack();
                usbdInfo.CLASS_CMD_Oflag = 0;
            }

            if(class_out_len > 0)
            {
                outp32(CEP_IRQ_ENB,  CEP_DATA_RxED_IE);    /* suppkt int/enb sts completion int */
            }
            else
            {             
                outp32(CEP_IRQ_STAT, CEP_STACOM_IS);
                outp32(CEP_CTRL_STAT, CEP_NAK_CLEAR);    /* clear nak so that sts stage is complete */
                outp32(CEP_IRQ_ENB,  (CEP_STACOM_IE|CEP_PING_IE|CEP_SETUP_PK_IE));    /* suppkt int enb/sts completion int */
            }
            if(usbdInfo.u32UVC == 1 && usbdStatus.appConnected == 1)
            {
                outp32(CEP_START_ADDR, 0);
                outp32(CEP_END_ADDR, 0x0);
            }
            return;
        }

        if (IrqSt & CEP_NAK_IS & IrqEn)
        {
            outp32(CEP_IRQ_STAT, CEP_NAK_IS);
            
            if(inp32(OUT_TRNSFR_CNT) && usbdInfo.CLASS_CMD_Oflag && ((inp32(CEP_IRQ_STAT) & CEP_DATA_RxED_IS) == 0))
            {
                /* Data Packet receive(OUT) */
                if (usbdInfo.pfnClassDataOUTCallBack != NULL)
                {
                    class_out_len = _usb_cmd_pkt.wLength;
                    class_out_len -= inp32(OUT_TRNSFR_CNT);
                    usbdInfo.pfnClassDataOUTCallBack();
                    usbdInfo.CLASS_CMD_Oflag = 0;
                }

                if(class_out_len > 0)
                {
                    outp32(CEP_IRQ_ENB,  CEP_DATA_RxED_IE);    /* suppkt int/enb sts completion int */
                }
                else
                {
                    outp32(CEP_IRQ_STAT, CEP_STACOM_IS);
                    outp32(CEP_CTRL_STAT, CEP_NAK_CLEAR);    /* clear nak so that sts stage is complete */
                    outp32(CEP_IRQ_ENB,  (CEP_STACOM_IE|CEP_PING_IE|CEP_SETUP_PK_IE));    /* suppkt int enb/sts completion int */
                }
                return;
            }
        }

        if (IrqSt & CEP_STALL_IS & IrqEn)
        {
            outp32(CEP_IRQ_STAT, CEP_STALL_IS);
            return;
        }

        if (IrqSt & CEP_ERR_IS & IrqEn)
        {
            outp32(CEP_IRQ_STAT, CEP_ERR_IS);
            return;
        }

        if (IrqSt & CEP_STACOM_IS & IrqEn)
        {
            outp32(CEP_IRQ_STAT, CEP_STACOM_IS);
            /* Update Device */
            if (_usb_cmd_pkt.bmRequestType == 0)
                usbd_update_device();

            if (usbdInfo.CLASS_CMD_Iflag || usbdInfo._usbd_resume || usbdInfo.GET_DEV_Flag)
                usbdInfo.USBModeFlag = 1;

            outp32(CEP_IRQ_ENB, CEP_SETUP_PK_IE);
            return;
        }

        if (IrqSt & CEP_FULL_IS & IrqEn)
        {
            outp32(CEP_IRQ_STAT, CEP_FULL_IS);
            return;
        }

        if (IrqSt & CEP_EMPTY_IS & IrqEn)
        {
            outp32(CEP_IRQ_STAT, CEP_EMPTY_IS);
            return;
        }
        outp32(IRQ_STAT_L, IRQ_CEP);
    }
    /* Endpoint Interrupt */
    if (IrqStL & IRQ_NCEP) 
    {
        g_bHostAttached = TRUE;
        if (IrqStL & EPA_INT)  /* Endpoint A */
        {
            IrqSt = inp32(EPA_IRQ_STAT);
            IrqEn = inp32(EPA_IRQ_ENB);

            outp32(EPA_IRQ_STAT, IrqSt);

            if (usbdInfo.pfnEPACallBack != NULL)
                usbdInfo.pfnEPACallBack(IrqEn, IrqSt);

        }
        if (IrqStL & EPB_INT)  /* Endpoint B */
        {
            IrqSt = inp32(EPB_IRQ_STAT);
            IrqEn = inp32(EPB_IRQ_ENB);

            outp32(EPB_IRQ_STAT, IrqSt);

            if (usbdInfo.pfnEPBCallBack != NULL)
                usbdInfo.pfnEPBCallBack(IrqEn, IrqSt);
        }

        if (IrqStL & EPC_INT)  /* Endpoint C */
        {
            IrqSt = inp32(EPC_IRQ_STAT);
            IrqEn = inp32(EPC_IRQ_ENB);
            outp32(EPC_IRQ_STAT, IrqSt);
            if (usbdInfo.pfnEPCCallBack != NULL)
                usbdInfo.pfnEPCCallBack(IrqEn, IrqSt);
        }
        if (IrqStL & EPD_INT)  /* Endpoint D */
        {
            IrqSt = inp32(EPD_IRQ_STAT);
            IrqEn = inp32(EPD_IRQ_ENB);
            outp32(EPD_IRQ_STAT, IrqSt);

            if (usbdInfo.pfnEPDCallBack != NULL)
                usbdInfo.pfnEPDCallBack(IrqEn, IrqSt);
        }
        outp32(IRQ_STAT_L, IRQ_NCEP);
    }
}

VOID udcSetSupendCallBack(PFN_USBD_CALLBACK pfun)
{
    pfnSuspend = pfun;
}

VOID udcSetResumeCallBack(PFN_USBD_CALLBACK pfun)
{
    pfnResume = pfun;
}
