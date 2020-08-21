/**************************************************************************//**
 * @file     HID.c
 * @brief    HID Class Device sample source file
 *           - Device Descriptor
 *           - USB Device Callback functions
 *           - HID Data Update function
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "N9H20.h"
#include "HID.h"

#define LEN_CONFIG_AND_SUBORDINATE      (LEN_CONFIG+LEN_INTERFACE+LEN_HID+LEN_ENDPOINT)

/* Define the interrupt In EP number */
#define INT_IN_EP_NUM  0x01

/* Mass_Storage command base address */
extern volatile USBD_INFO_T usbdInfo;

/* USB Device Property */
extern USB_CMD_T  _usb_cmd_pkt;

UINT32 volatile u32Ready = 0;
#ifdef HID_MOUSE
UINT8  g_HID_au8MouseReportDescriptor[] __attribute__((aligned(4))) =
{
    0x05, 0x01,         /* Usage Page(Generic Desktop Controls) */
    0x09, 0x02,         /* Usage(Mouse) */
    0xA1, 0x01,         /* Collection(Application) */
    0x09, 0x01,         /* Usage(Pointer) */
    0xA1, 0x00,         /* Collection(Physical) */
    0x05, 0x09,         /* Usage Page(Button) */
    0x19, 0x01,         /* Usage Minimum(0x1) */
    0x29, 0x03,         /* Usage Maximum(0x3) */
    0x15, 0x00,         /* Logical Minimum(0x0) */
    0x25, 0x01,         /* Logical Maximum(0x1) */
    0x75, 0x01,         /* Report Size(0x1) */
    0x95, 0x03,         /* Report Count(0x3) */
    0x81, 0x02,         /* Input(3 button bit) */
    0x75, 0x05,         /* Report Size(0x5) */
    0x95, 0x01,         /* Report Count(0x1) */
    0x81, 0x01,         /* Input(5 bit padding) */
    0x05, 0x01,         /* Usage Page(Generic Desktop Controls) */
    0x09, 0x30,         /* Usage(X) */
    0x09, 0x31,         /* Usage(Y) */
    0x09, 0x38,         /* Usage(Wheel) */
    0x15, 0x81,         /* Logical Minimum(0x81)(-127) */
    0x25, 0x7F,         /* Logical Maximum(0x7F)(127) */
    0x75, 0x08,         /* Report Size(0x8) */
    0x95, 0x03,         /* Report Count(0x3) */
    0x81, 0x06,         /* Input(1 byte wheel) */
    0xC0,               /* End Collection */
    0xC0,               /* End Collection */
  
};


#define HID_MOUSE_REPORT_DESCRIPTOR_SIZE \
  sizeof (g_HID_au8MouseReportDescriptor) / sizeof(g_HID_au8MouseReportDescriptor[0])
UINT32 g_HID_u32MouseReportDescriptorSize = HID_MOUSE_REPORT_DESCRIPTOR_SIZE;

#define HID_REPORT_DESCRIPTOR_SIZE HID_MOUSE_REPORT_DESCRIPTOR_SIZE

UINT8 g_au8MouseReport[4] __attribute__((aligned(4)));
UINT32 g_u32MouseReportSize = sizeof(g_au8MouseReport) / sizeof(g_au8MouseReport[0]);

#endif


#ifdef HID_KEYBOARD

//keyboard 101keys
UINT8 g_HID_au8KeyboardReportDescriptor[] __attribute__((aligned(4))) =
{
      0x05, 0x01,
      0x09, 0x06,
      0xA1, 0x01,
      0x05, 0x07,
      0x19, 0xE0,
      0x29, 0xE7,
      0x15, 0x00,
      0x25, 0x01,
      0x75, 0x01,
      0x95, 0x08,
      0x81, 0x02,
      0x95, 0x01,
      0x75, 0x08,
      0x81, 0x01,
      0x95, 0x05,
      0x75, 0x01,
      0x05, 0x08,
      0x19, 0x01,
      0x29, 0x05,
      0x91, 0x02,
      0x95, 0x01,
      0x75, 0x03,
      0x91, 0x01,
      0x95, 0x06,
      0x75, 0x08,
      0x15, 0x00,
      0x25, 0x65,
      0x05, 0x07,
      0x19, 0x00,
      0x29, 0x65,
      0x81, 0x00,
      0xC0  
};

#define HID_KEYBOARD_REPORT_DESCRIPTOR_SIZE \
    sizeof(g_HID_au8KeyboardReportDescriptor) / sizeof(g_HID_au8KeyboardReportDescriptor[0])
UINT32 g_HID_u32KeyboardReportDescriptorSize = HID_KEYBOARD_REPORT_DESCRIPTOR_SIZE;

# define HID_REPORT_DESCRIPTOR_SIZE HID_KEYBOARD_REPORT_DESCRIPTOR_SIZE

UINT8 g_au8KeyboardReport[8] __attribute__((aligned(4)));
UINT32 g_u32KeyboardReportSize = sizeof(g_au8KeyboardReport) / sizeof(g_au8KeyboardReport[0]);

#endif


/* MSC Descriptor */
UINT8 HID_DeviceDescriptor[] __attribute__((aligned(4))) =
{
    LEN_DEVICE,     /* bLength */
    DESC_DEVICE,    /* bDescriptorType */
    0x10, 0x01,     /* bcdUSB */
    0x00,           /* bDeviceClass */
    0x00,           /* bDeviceSubClass */
    0x00,           /* bDeviceProtocol */
    CEP_MAX_PKT_SIZE,/* bMaxPacketSize0 */
    /* idVendor */
    USB_VID & 0x00FF,
    (USB_VID & 0xFF00) >> 8,
    /* idProduct */
    USB_PID & 0x00FF,
    (USB_PID & 0xFF00) >> 8,
    0x00, 0x00,     /* bcdDevice */
    0x01,           /* iManufacture */
    0x02,           /* iProduct */
    0x00,           /* iSerialNumber - no serial */
    0x01            /* bNumConfigurations */
};

static UINT8 HID_ConfigurationBlock[] __attribute__((aligned(4))) =
{

    LEN_CONFIG,     /* bLength */
    DESC_CONFIG,    /* bDescriptorType */
    /* wTotalLength */
    LEN_CONFIG_AND_SUBORDINATE & 0x00FF,
    (LEN_CONFIG_AND_SUBORDINATE & 0xFF00) >> 8,
    0x01,           /* bNumInterfaces */
    0x01,           /* bConfigurationValue */
    0x00,           /* iConfiguration */
    0x80 | (HID_IS_SELF_POWERED << 6) | (HID_IS_REMOTE_WAKEUP << 5),/* bmAttributes */
    HID_MAX_POWER,         /* MaxPower */

    /* I/F descr: HID */
    LEN_INTERFACE,  /* bLength */
    DESC_INTERFACE, /* bDescriptorType */
    0x00,           /* bInterfaceNumber */
    0x00,           /* bAlternateSetting */
    0x01,           /* bNumEndpoints */
    0x03,           /* bInterfaceClass */
    0x01,           /* bInterfaceSubClass */
    0x01,           /* bInterfaceProtocol */
    0x00,           /* iInterface */

    /* HID Descriptor */
    LEN_HID,        /* Size of this descriptor in UINT8s. */
    DESC_HID,       /* HID descriptor type. */
    0x10, 0x01,     /* HID Class Spec. release number. */
    0x00,           /* H/W target country. */
    0x01,           /* Number of HID class descriptors to follow. */
    DESC_HID_RPT,   /* Dscriptor type. */
    /* Total length of report descriptor. */
    HID_REPORT_DESCRIPTOR_SIZE & 0x00FF,
    (HID_REPORT_DESCRIPTOR_SIZE & 0xFF00) >> 8,

    /* EP Descriptor: interrupt in. */
    LEN_ENDPOINT,   /* bLength */
    DESC_ENDPOINT,  /* bDescriptorType */
    (INT_IN_EP_NUM | EP_INPUT), /* bEndpointAddress */
    EP_INT,         /* bmAttributes */
    /* wMaxPacketSize */
    EPA_MAX_PKT_SIZE & 0x00FF,
    (EPA_MAX_PKT_SIZE & 0xFF00) >> 8,
    HID_DEFAULT_INT_IN_INTERVAL     /* bInterval */
};

/* Identifier Language */
static UINT8 HID_StringDescriptor0[4] __attribute__((aligned(4))) =
{
    4,               /* bLength */
    USB_DT_STRING,   /* bDescriptorType */
    0x09, 0x04
};

/* iManufacturer */
UINT8 HID_StringDescriptor1[] __attribute__((aligned(4))) =
{
    0x10,        /* bLength (Dafault Value is 0x10, the value will be set to actual value according to the Descriptor size wehn calling mscdInit) */
    0x03,        /* bDescriptorType */
    'n', 0, 'u', 0, 'v', 0, 'o', 0, 'T', 0, 'o', 0, 'n', 0
};

/* iProduct */
UINT8 HID_StringDescriptor2[] __attribute__((aligned(4))) =
{
    0x10,        /* bLength (Dafault Value is 0x10, the value will be set to actual value according to the Descriptor size wehn calling mscdInit) */
    0x03,        /* bDescriptorType */
    'U', 0, 'S', 0, 'B', 0, ' ', 0, 'H', 0, 'I', 0, 'D', 0
};

/* iSerialNumber */
UINT8 HID_StringDescriptor3[] __attribute__((aligned(4))) =
{
    0x1A,        /* bLength (Dafault Value is 0x1A, the value will be set to actual value according to the Descriptor size wehn calling mscdInit) */
    0x03,        /* bDescriptorType */
    '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, 'N', 0, '9', 0, 'H', 0, '2', 0, '0', 0
};

/* HID Full Speed Init */
void hidFullSpeedInit(void)
{
    usbdInfo.usbdMaxPacketSize = 0x40;
    /* bulk in */
    outp32(EPA_RSP_SC, 0x00000000);         /* auto validation */
    outp32(EPA_MPS, EPA_MAX_PKT_SIZE);      /* mps 8 */
    outp32(EPA_CFG, 0x0000001b);            /* bulk in ep no 1 */
    outp32(EPA_START_ADDR, CEP_MAX_PKT_SIZE);
    outp32(EPA_END_ADDR, CEP_MAX_PKT_SIZE +EPA_MAX_PKT_SIZE -1);
    outp32(OPER, 0);
}

void hidClassOUT(void)
{
    if(_usb_cmd_pkt.bRequest == HID_SET_IDLE)
    {
        outp32(CEP_CTRL_STAT, ZEROLEN);
        sysprintf("Set IDLE\n");
        outp32(CEP_IRQ_STAT, CEP_STACOM_IS);
    }
    else if(_usb_cmd_pkt.bRequest == HID_SET_REPORT)
    {
        u32Ready = 1;
        sysprintf("SET_REPORT 0x%X\n",inp8(CEP_DATA_BUF));
    }
}

#ifdef HID_MOUSE
UINT8 volatile g_u8EPAReady = 0;
signed char mouse_table[] = {-16, -16, -16, 0, 16, 16, 16, 0};
UINT8 mouse_idx = 0;
UINT8 move_len, mouse_mode=1;

void HID_UpdateMouseData(void)
{
    int volatile i;
#ifdef HID_KEYPAD    
    signed char buf[4];
    UINT32 u32KpiReport = 0x0;
    
    if (g_u8EPAReady)
    {
        u32KpiReport = kpi_read(KPI_NONBLOCK);
        u32KpiReport &= MASK_KEY;
        if(u32KpiReport != 0)
        {
            switch(u32KpiReport)
            {
                case UP_KEY:
                    buf[0] = 0;
                    buf[1] = 0;
                    buf[2] = -5;
                    break;
                case DOWN_KEY:
                    buf[0] = 0;
                    buf[1] = 0;
                    buf[2] = 5;
                    break;
                case LEFT_KEY:
                    buf[0] = 0;
                    buf[1] = -5;
                    buf[2] = 0;
                    break;
                case RIGHT_KEY:
                    buf[0] = 0;
                    buf[1] = 5;
                    buf[2] = 0;
                    break;
                default:
                    break;
            }
            g_u8EPAReady = 0;

            /* Set transfer length and trigger IN transfer */
            for (i=0; i<4; i++)
                outp8(EPA_DATA_BUF ,buf[i]);
            outp32(EPA_IRQ_ENB, IN_TK_IE);

            outp32(EPA_RSP_SC, PK_END); /* Set Packet End */
        }
    }
#else
    UINT8 buf[4];

    if (g_u8EPAReady) 
    {
        mouse_mode ^= 1;
        if (mouse_mode)
        {
            if (move_len > 14)
            {
                /* Update new report data */
                buf[0] = 0x00;
                buf[1] = mouse_table[mouse_idx & 0x07];
                buf[2] = mouse_table[(mouse_idx + 2) & 0x07];
                buf[3] = 0x00;
                mouse_idx++;
                move_len = 0;
            }
        }
        else 
        {
            buf[0] = buf[1] = buf[2] = buf[3] = 0;
        }
        move_len++;
        g_u8EPAReady = 0;
        /* Set transfer length and trigger IN transfer */
        for (i=0; i<g_u32MouseReportSize; i++)
            outp8(EPA_DATA_BUF ,buf[i]);
        outp32(EPA_IRQ_ENB, IN_TK_IE);

        outp32(EPA_RSP_SC, PK_END); /* Set Packet End */
    }
#endif
}

void EPA_Handler(UINT32 u32IntEn,UINT32 u32IntStatus)  /* Interrupt IN handler */
{
    g_u8EPAReady = 1;
}

#endif


#ifdef HID_KEYBOARD

char output_string[] = "THIS IS HID KEYBOARD DEMO";

UINT32 preKey = 0 ;
UINT32 string_index = 0;
UINT32 delay = 0;
void HID_SetInReport(void)
{
    INT32 i;
#ifdef HID_KEYPAD
    UINT32 u32KpiReport = 0x0;
#endif    
    UINT8 *buf; 

    if (u32Ready) 
    {
        if(!(inp32(EPA_IRQ_STAT) & EMPTY_IS))
            return;
        buf = g_au8KeyboardReport;
#ifdef HID_KEYPAD
        u32KpiReport = kpi_read(KPI_NONBLOCK);

        if(u32KpiReport == 0) 
        {
            /* No any key pressed */
            for (i=0;i<8;i++)
                buf[i] = 0;

            if(u32KpiReport != preKey)
            {
                /* Trigger to note key release */
                for (i=0; i<g_u32KeyboardReportSize; i++)
                    outp8(EPA_DATA_BUF ,buf[i]);
                outp32(EPA_RSP_SC, PK_END); /* Set Packet End */
            }
        }
        else
        {
            switch(u32KpiReport)
            {
                case UP_KEY:
                    buf[2] = 0x04;    /* "A" */
                    break;
                case DOWN_KEY:
                    buf[2] = 0x05;    /* "B" */
                    break;
                case LEFT_KEY:
                    buf[2] = 0x06;    /* "C" */
                    break;
                case RIGHT_KEY:
                    buf[2] = 0x07;    /* "D" */
                    break;
                case ENTER_KEY:
                    buf[2] = 0x08;    /* "E" */
                    break;  
                case HOME_KEY:
                    buf[2] = 0x09;    /* "F" */
                    break;
            }
            preKey = u32KpiReport;

            /* Trigger to note key release */
            for (i=0; i<g_u32KeyboardReportSize; i++)
                outp8(EPA_DATA_BUF ,buf[i]);
            outp32(EPA_RSP_SC, PK_END); /* Set Packet End */
        }  
#else
        if(delay)
        {
            delay--;
            return;
        }
        else if (preKey  != 0) 
        {
            /* No any key pressed */
            for (i=0;i<8;i++)
                buf[i] = 0;
            preKey = 0;
            /* Trigger to note key release */
            for (i=0; i<g_u32KeyboardReportSize; i++)
                outp8(EPA_DATA_BUF ,buf[i]);
            outp32(EPA_RSP_SC, PK_END); /* Set Packet End */        

        }
        else
        {
            if(string_index == sizeof(output_string))
                buf[2] = 0x28;        /* Return */
            else if (output_string[string_index] == ' ')
                buf[2] = 0x2C;        /* Space */
            else
                buf[2] = output_string[string_index] - 'A' + 4;

            preKey = buf[2];
            /* Trigger to note key release */
            for (i=0; i<g_u32KeyboardReportSize; i++)
                outp8(EPA_DATA_BUF ,buf[i]);
            outp32(EPA_RSP_SC, PK_END); /* Set Packet End */

            delay  = 2;
            string_index++;
            if(string_index > sizeof(output_string))
            {
                string_index = 0;
            }
        }
#endif
    }
}

#endif

/* HID Init */
void hidInit(void)
{
    /* Set Endpoint map */
    usbdInfo.i32EPA_Num = 1;     /* Endpoint 1 */
    usbdInfo.i32EPB_Num = -1;    /* Not use */
    usbdInfo.i32EPC_Num = -1;    /* Not use */
    usbdInfo.i32EPD_Num = -1;    /* Not use */

    /* Set Callback Function */
    /* Set MSC initialize function */
    usbdInfo.pfnFullSpeedInit = hidFullSpeedInit;
    usbdInfo.pfnHighSpeedInit = hidFullSpeedInit;

    /* Set Descriptor pointer */
    usbdInfo.pu32DevDescriptor = (PUINT32) &HID_DeviceDescriptor;
    usbdInfo.pu32HSConfDescriptor = (PUINT32) &HID_ConfigurationBlock;
    usbdInfo.pu32FSConfDescriptor = (PUINT32) &HID_ConfigurationBlock;
    usbdInfo.pu32HIDDescriptor = (PUINT32) ((UINT32)&HID_ConfigurationBlock + LEN_CONFIG + LEN_INTERFACE);

    usbdInfo.pfnClassDataOUTCallBack = hidClassOUT;

#ifdef HID_MOUSE
    /* Set the HID report descriptor */
    usbdInfo.pu32HIDRPTDescriptor =  (PUINT32) &g_HID_au8MouseReportDescriptor;
    usbdInfo.u32HIDRPTDescriptorLen = g_HID_u32MouseReportDescriptorSize;
    usbdInfo.pfnEPACallBack = EPA_Handler;
    g_u8EPAReady = 1;
#endif

#ifdef HID_KEYBOARD
    /* Set the HID report descriptor */
    usbdInfo.pu32HIDRPTDescriptor = (PUINT32) &g_HID_au8KeyboardReportDescriptor;
    usbdInfo.u32HIDRPTDescriptorLen = g_HID_u32KeyboardReportDescriptorSize;
#endif

    usbdInfo.pu32StringDescriptor[0] = (PUINT32) &HID_StringDescriptor0;
    usbdInfo.pu32StringDescriptor[1] = (PUINT32) &HID_StringDescriptor1;
    usbdInfo.pu32StringDescriptor[2] = (PUINT32) &HID_StringDescriptor2;
    usbdInfo.pu32StringDescriptor[3] = (PUINT32) &HID_StringDescriptor3;

    /* Set Descriptor length */
    usbdInfo.u32DevDescriptorLen =  LEN_DEVICE;
    usbdInfo.u32HSConfDescriptorLen =  LEN_CONFIG_AND_SUBORDINATE;
    usbdInfo.u32FSConfDescriptorLen =  LEN_CONFIG_AND_SUBORDINATE;
    usbdInfo.u32StringDescriptorLen[0] =  HID_STR0_DSCPT_LEN;
    usbdInfo.u32StringDescriptorLen[1] = HID_StringDescriptor1[0] = sizeof(HID_StringDescriptor1);
    usbdInfo.u32StringDescriptorLen[2] = HID_StringDescriptor2[0] = sizeof(HID_StringDescriptor2);
    usbdInfo.u32StringDescriptorLen[3] = HID_StringDescriptor3[0] = sizeof(HID_StringDescriptor3);
}

