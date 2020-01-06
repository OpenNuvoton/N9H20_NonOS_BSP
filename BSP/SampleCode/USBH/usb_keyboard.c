/****************************************************************************
 * @file     usb_keyboard.c
 * @version  V1.00
 * $Revision: 4 $
 * $Date: 18/04/25 11:43a $
 * @brief    USBH sample source file
 *
 * @note
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

/*
 * $Id: usbkbd.c,v 1.16 2000/08/14 21:05:26 vojtech Exp $
 *
 *  Copyright (c) 1999-2000 Vojtech Pavlik
 *
 *  USB HIDBP Keyboard support
 *
 *  Sponsored by SuSE
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * 
 * Should you need to contact me, the author, you can do so either by
 * e-mail - mail your message to <vojtech@suse.cz>, or by paper mail:
 * Vojtech Pavlik, Ucitelska 1576, Prague 8, 182 00 Czech Republic
 */

#ifdef ECOS
#include "stdio.h"
#include "string.h"
#include "drv_api.h"
#include "diag.h"
#include "wbtypes.h"
#include "wbio.h"
#else
#include <stdio.h>
#include <string.h>
#include "wbtypes.h"
#include "wblib.h"
#endif
#include "usbkbd.h"

USB_KBD_T  *_my_usbkbd = NULL;

#if 0
/* See the HID Usage Tables Document, this is the AT-101 key position */
static UINT8 usb_kbd_keycode[256] = 
{
      0,  0,  0,  0, 30, 48, 46, 32, 18, 33, 34, 35, 23, 36, 37, 38,
     50, 49, 24, 25, 16, 19, 31, 20, 22, 47, 17, 45, 21, 44,  2,  3,
      4,  5,  6,  7,  8,  9, 10, 11, 28,  1, 14, 15, 57, 12, 13, 26,
     27, 43, 84, 39, 40, 41, 51, 52, 53, 58, 59, 60, 61, 62, 63, 64,
     65, 66, 67, 68, 87, 88, 99, 70,119,110,102,104,111,107,109,106,
    105,108,103, 69, 98, 55, 74, 78, 96, 79, 80, 81, 75, 76, 77, 71,
     72, 73, 82, 83, 86,127,116,117, 85, 89, 90, 91, 92, 93, 94, 95,
    120,121,122,123,134,138,130,132,128,129,131,137,133,135,136,113,
    115,114,  0,  0,  0,124,  0,181,182,183,184,185,186,187,188,189,
    190,191,192,193,194,195,196,197,198,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     29, 42, 56,125, 97, 54,100,126,164,166,165,163,161,115,114,113,
    150,158,159,128,136,177,178,176,142,152,173,140
};
#endif

/* YCHuang: I translate the key position into key code. */
#if defined(__GNUC__)
static UINT8 _UsbKeyCodeMap[256][20] __attribute__((aligned (32))) = 
#else
static __align(32) UINT8 _UsbKeyCodeMap[256][20] = 
#endif	
{
    "N/A",
    "N/A",
    "N/A",
    "N/A",
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "0",
    "[Enter]",
    "[Escape]",
    "[BackSpc]",
    "[Tab]",
    " ",
    "-",
    "=",
    "[",
    "]",
    "\\",
    "`",
    ";",
    "'",
    "[Grave]",
    ",",
    ".",
    "/",
    "[Caps Lock]",
    "[F1]",
    "[F2]",
    "[F3]",
    "[F4]",
    "[F5]",
    "[F6]",
    "[F7]",
    "[F8]",
    "[F9]",
    "[F10]",
    "[F11]",
    "[F12]",
    "[PrtScr]",
    "[ScrLock]",
    "[Pause]",
    "[Insert]",
    "[Home]",
    "[PageUp]",
    "[Delete]",
    "[End]",
    "[PageDown]",
    "[->]",
    "[<-]",
    "[DownArrow]",
    "[UpArrow]",
    "[NumLock]",
    "/",
    "*",
    "-",
    "+",
    "[Enter]",
    "1",
    "2",
    "3",
    "4", 
    "5",
    "6",
    "7",
    "8",
    "9",
    "0",
    "[Del]",
    "[?]",
    "[Application]",
    "[Power]",
    "=",
    "F13",
    "F14",
    "F15",
    "F16",
    "F17",
    "F18",
    "F19",
    "F20",
    "F21",
    "F22",
    "F23",
    "F24",
    "[Execute]",
    "[Help]",
    "[Menu]",
    "[Select]",
    "[Stop]",
    "[Again]",
    "[Undo]",
    "[Cut]",
    "[Copy]",
    "[Paste]",
    "[Find]",
    "[Mute]",
    "[VolumeUp]",
    "[VolumeDown]",
    "[Lock NumLock]",
    "[Lock ScrLock]",
    "[Comma]",
    "[Equal Sign]",
    "[International 1]",
    "[International 2]",
    "[International 3]",
    "[International 4]",
    "[International 5]",
    "[International 6]",
    "[International 7]",
    "[International 8]",
    "[International 9]",
    "[LANG 1]",
    "[LANG 2]",
    "[LANG 3]",
    "[LANG 4]",
    "[LANG 5]",
    "[LANG 6]",
    "[LANG 7]",
    "[LANG 8]",
    "[LANG 9]",
    "[Alt Erase]",
    "[SysReq]",
    "[Cancel]",
    "[Clear]",
    "[Prior]",
    "[Return]",
    "[Seperate]",
    "[Out]",
    "[Oper]",
    "[Clear/Again]",
    "[CrSel]",
    "[ExSel]",
};

#if 0
/* Offset 224 */
static UINT8  _UsbControlKey[8][24] =
{
    "[Left Ctrl]",
    "[Left Shift]",
    "[Left Alt]",
    "[Left GUI]",
    "[Right Ctrl]",
    "[Right Shift]",
    "[Right Alt]",
    "[Right GUI]"
};
#endif


/* 
 *  The interrupt in pipe of usb keyboard returns 8 bytes.
 *  Byte 0: modifier bytes
 *  Byte 1: reserved byte
 *  Byte 2: [0..4] LED report, [5..7] LED report padding
 *  Byte 3~7: Key arrays
 */
static VOID usb_kbd_irq(URB_T *urb)
{
    USB_KBD_T *kbd = urb->context;
    INT  i;

    if (urb->status)
    {
        sysprintf("usb_kbd_irq, urb error:%d\n", urb->status);
        return;
    }
    
    for (i=2; i<8; i++)
        if (kbd->newData[i])
            sysprintf("%s\n", _UsbKeyCodeMap[kbd->newData[i]]);
}

static VOID usb_kbd_led(URB_T *urb)
{
    USB_KBD_T  *kbd = urb->context;
    INT   ret;

    if (urb->status)
        sysprintf("Warning - led urb status %d received\n", urb->status);
        
    if (kbd->leds == kbd->newleds)
        return;

    kbd->leds = kbd->newleds;
    kbd->urbLed.dev = kbd->usbdev;
    ret = USB_SubmitUrb(&kbd->urbLed);
    if (ret)
        sysprintf("Error - USB_SubmitUrb(leds) failed\n");
}


VOID USBKeyboardLED(URB_T *urb)
{
    USB_KBD_T *kbd = urb->context;

    if (urb->status)
        sysprintf("Warning - led urb status %d received\n", urb->status);
        
    if (kbd->leds == kbd->newleds)
        return;

    kbd->leds = kbd->newleds;
    kbd->urbLed.dev = kbd->usbdev;
    if (USB_SubmitUrb(&kbd->urbLed))
        sysprintf("Error - USB_SubmitUrb(leds) failed\n");
}


INT USBKeyboardOpen(USB_KBD_T *kbd)
{
    if (USB_SubmitUrb(&kbd->urbIrq))
        return -1; /* -EIO; */
    return 0;
}

VOID USBKeyboardClose(USB_KBD_T *kbd)
{
    USB_UnlinkUrb(&kbd->urbIrq);
}


static VOID *usb_kbd_probe(USB_DEV_T *dev, UINT32 ifnum,
                           const USB_DEV_ID_T *id)
{
    struct usb_interface *iface;
    struct usb_interface_descriptor *interface;
    struct usb_endpoint_descriptor *endpoint;
    USB_KBD_T *kbd;
    INT  pipe, maxp;
    char  buf[64];
    
    /* added by YCHuang, reject interface 1 */
    if (ifnum != 0)
        return NULL;

    iface = &dev->actconfig->interface[ifnum];
    interface = &iface->altsetting[iface->act_altsetting];

    if (interface->bNumEndpoints != 1) 
        return NULL;

    endpoint = interface->endpoint + 0;
    if (!(endpoint->bEndpointAddress & 0x80)) 
        return NULL;
    if ((endpoint->bmAttributes & 3) != 3) 
        return NULL;

    pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress);
    maxp = usb_maxpacket(dev, pipe, usb_pipeout(pipe));

    USB_SetProtocol(dev, interface->bInterfaceNumber, 0);
    USB_SetIdle(dev, interface->bInterfaceNumber, 0, 0);  /* duration:indefinite, for all reports */

    kbd = USB_malloc(sizeof(USB_KBD_T), 4);
    if (!kbd) 
        return NULL;
    memset(kbd, 0, sizeof(USB_KBD_T));

    kbd->usbdev = dev;

    sysprintf("DEVICE SLOW: %d  %d\n", dev->slow, usb_pipeslow(pipe));
    dev->slow = 1;
    
#ifdef ETST_ALIGNMENT_INT
    kbd->newData = (UINT8 *)0xC103FFFF;
    //kbd->newData = (UINT8 *)0xC105FFF1;
    //kbd->newData = (UINT8 *)0xC107FFFD;
    //kbd->newData = (UINT8 *)0xC1090000;
#endif

    FILL_INT_URB(&kbd->urbIrq, dev, pipe, kbd->newData, maxp > 8 ? 8 : maxp,
                usb_kbd_irq, kbd, endpoint->bInterval);

    kbd->dr.requesttype = USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    kbd->dr.request = USB_REQ_SET_REPORT;
    kbd->dr.value = 0x200;
    kbd->dr.index = interface->bInterfaceNumber;
    kbd->dr.length = 1;

    if (dev->descriptor.iManufacturer &&
        (USB_TranslateString(dev, dev->descriptor.iManufacturer, buf, 63) > 0))
        sysprintf(kbd->name, "%s %s", kbd->name, buf);
    if (dev->descriptor.iProduct &&
        (USB_TranslateString(dev, dev->descriptor.iProduct, buf, 63) > 0))
        sysprintf(kbd->name, "%s %s", kbd->name, buf);

    if (!strlen(kbd->name))
        sysprintf(kbd->name, "USB HIDBP Keyboard %04x:%04x",
                          dev->descriptor.idVendor, dev->descriptor.idProduct);

    FILL_CONTROL_URB(&kbd->urbLed, dev, usb_sndctrlpipe(dev, 0),
                    (VOID*) &kbd->dr, &kbd->leds, 1, usb_kbd_led, kbd);

    //input_register_device(&kbd->dev);

    //printk(KERN_INFO "input%d: %s on on usb%d:%d.%d\n",
    //        kbd->dev.number, kbd->name, dev->bus->busnum, dev->devnum, ifnum);
    sysprintf("usb_kbd_probe - %s on usb%d:%d.%d\n", kbd->name, dev->bus->busnum, dev->devnum, ifnum);
    _my_usbkbd = kbd;
    USBKeyboardOpen(_my_usbkbd);
    return kbd;
}


static VOID usb_kbd_disconnect(USB_DEV_T *dev, VOID *ptr)
{
    USB_KBD_T  *kbd = ptr;
    USB_UnlinkUrb(&kbd->urbIrq);
    //input_unregister_device(&kbd->dev);
    USB_free(kbd);
}


static USB_DEV_ID_T usb_kbd_id_table[] = 
{
    USB_DEVICE_ID_MATCH_INT_INFO,      /* match_flags */
    0, 0, 0, 0, 0, 0, 0,
    3,                                 /* bInterfaceClass */
    1,                                 /* bInterfaceSubClass */
    1,                                 /* bInterfaceProtocol */
    0 
};


USB_DRIVER_T  usb_kbd_driver = 
{
    "keyboard",
    usb_kbd_probe,
    usb_kbd_disconnect,
    {NULL,NULL},                       /* driver_list */
 //   {0},                               /* semaphore */
    NULL,                              /* *ioctl */
    usb_kbd_id_table,
    NULL,                              /* suspend */
    NULL                               /* resume */
};


INT  USBKeyboardInit()
{
    USB_RegisterDriver(&usb_kbd_driver);
    return 0;
}

