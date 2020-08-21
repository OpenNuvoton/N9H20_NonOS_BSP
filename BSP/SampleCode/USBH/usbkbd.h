/**************************************************************************//**
 * @file     usbkbd.h
 * @brief    USB Host keyboard driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#ifndef _USBKBD_H_
#define _USBKBD_H_

#include "N9H20.h"

typedef struct usb_kbd 
{
    struct usb_device  *usbdev;
#ifdef ETST_ALIGNMENT_INT
    UINT8           *newData;
#else    
    UINT8           newData[8];
#endif    
    UINT8           oldData[8];
    URB_T           urbIrq, urbLed;
    DEV_REQ_T       dr;
    UINT8           leds, newleds;
    CHAR            name[128];
    INT             open;
} USB_KBD_T;

extern USB_KBD_T  *_my_usbkbd;

extern INT  USBKeyboardInit(VOID);
extern INT  USBKeyboardOpen(USB_KBD_T *);
extern VOID USBKeyboardClose(USB_KBD_T *);
extern VOID USBKeyboardLED(URB_T *);

#endif
