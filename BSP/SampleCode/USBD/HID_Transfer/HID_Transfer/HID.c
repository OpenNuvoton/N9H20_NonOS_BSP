#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wblib.h"
#include "N9H20_reg.h"
#include "N9H20_USBD.h"
#include "HID.h"
#include "Common.h"   

//#define __FORCE_FULLSPEED__
  

/* Mass_Storage command base address */
extern volatile USBD_INFO_T usbdInfo;

/* USB Device Property */
extern USB_CMD_T  _usb_cmd_pkt;

UINT32 g_u32EPA_MXP, g_u32EPB_MXP;
UINT32 volatile u32Ready = 0;
UINT32 volatile g_u32ReadWriteSize;
unsigned int g_Version = 0x20230112;
unsigned int g_Status = 0xA5;
unsigned int g_Parameter[50][50] = {0};

#if defined (__GNUC__)
UINT8  image_buffer[512] __attribute__((aligned(32)));
#else
UINT8 __align(32) image_buffer[512];
#endif

unsigned char *imagebuf;

/***************************************************************/
#define HID_CMD_SIGNATURE   0x43444948

/* HID Transfer Commands */
#define HID_CMD_NONE            0x00
#define HID_CMD_GET_VER         0xD3
#define HID_CMD_GET_STS         0xD4
#define HID_CMD_IMAGE_WRITE     0xC4
#define HID_CMD_SET_PARAM       0xC5
#define HID_CMD_GET_PARAM       0xD6
#define HID_CMD_IMAGE_READ      0xD7
#define HID_CMD_EXIT            0xB1
#define HID_CMD_TEST            0xB4

#ifdef __GNUC__

typedef struct __attribute__((__packed__))
{
    UINT8  u8Cmd;
    UINT8  u8Size;
    UINT32 u32Arg1;
    UINT32 u32Arg2;
    UINT32 u32Signature;
    UINT32 u32Checksum;
}
CMD_T;

CMD_T gCmd __attribute__((aligned(4)));
static UINT8  gu8PageBuff[EPA_MAX_PKT_SIZE]  __attribute__((aligned(4)))  = {0};    /* Page buffer to upload/download through HID report */
static UINT8  gu8JPEGBuff[0x100000]  __attribute__((aligned(4))) = {0};    /* JPEG buffer to download through HID report */
static UINT8  gTemp[EPA_MAX_PKT_SIZE] __attribute__((aligned(4)));    /* Page buffer to upload/download through HID report */
#else
typedef __packed struct
{
    UINT8 u8Cmd;
    UINT8 u8Size;
    UINT32 u32Arg1;
    UINT32 u32Arg2;
    UINT32 u32Signature;
    UINT32 u32Checksum;
} CMD_T;

__align(4) CMD_T gCmd;
__align(4) static UINT8  gu8PageBuff[EPA_MAX_PKT_SIZE]  = {0};    /* Page buffer to upload/download through HID report */
__align(4) static UINT8  gu8JPEGBuff[0x100000]  = {0};    /* JPEG buffer to download through HID report */
__align(4) static UINT8  gTemp[EPA_MAX_PKT_SIZE];    /* Page buffer to upload/download through HID report */
#endif

static UINT32 g_u32BytesInJPEGBuf = 0;          /* The bytes of data in g_u8JPEGBuff */

CMD_T *pCmd;
UINT8  *g_u8PageBuff, *g_u8TestPages,*g_Temp, *g_u8JPEGBuff;
INT32 gi32CmdTestCnt = 0;

#if defined (__GNUC__)
UINT8  HID_DeviceReportDescriptor[] __attribute__((aligned(4))) =
#else
__align(4) UINT8  HID_DeviceReportDescriptor[] =
#endif
{
    0x06, 0x06, 0xFF,       /* USAGE_PAGE (Vendor Defined)*/
    0x09, 0x01,             /* USAGE (0x01)*/
    0xA1, 0x01,             /* COLLECTION (Application)*/
    0x15, 0x00,             /* LOGICAL_MINIMUM (0)*/
    0x26, 0xFF, 0x00,       /* LOGICAL_MAXIMUM (255)*/
    0x75, 0x08,             /* REPORT_SIZE (8)*/
    0x96, 0x00, 0x02,       /* REPORT_COUNT*/
    0x09, 0x01,
    0x81, 0x02,             /* INPUT (Data,Var,Abs)*/
    0x96, 0x00, 0x02,       /* REPORT_COUNT*/
    0x09, 0x01,
    0x91, 0x02,             /* OUTPUT (Data,Var,Abs)*/
    0x95, 0x08,             /* REPORT_COUNT (8) */
    0x09, 0x01,
    0xB1, 0x02,             /* FEATURE */
    0xC0,                   /* END_COLLECTION*/
};


#if defined (__GNUC__)
UINT8  HID_DeviceReportDescriptor_FS[] __attribute__((aligned(4))) =
#else
__align(4) UINT8  HID_DeviceReportDescriptor_FS[] =
#endif
{
    0x06, 0x00, 0xFF,   /* Usage Page = 0xFF00 (Vendor Defined Page 1) */
    0x09, 0x01,         /* Usage (Vendor Usage 1) */
    0xA1, 0x01,         /* Collection (Application) */
    0x19, 0x01,         /* Usage Minimum */
    0x29, 0x40,         /* 64 input usages total (0x01 to 0x40) */
    0x15, 0x00,         /* Logical Minimum (data bytes in the report may have minimum value = 0x00) */
    0x26, 0xFF, 0x00,   /* Logical Maximum (data bytes in the report may have maximum value = 0x00FF = unsigned 255) */
    0x75, 0x08,         /* Report Size: 8-bit field size */
    0x95, 0x40,         /* Report Count: Make sixty-four 8-bit fields (the next time the parser hits 
                           an "Input", "Output", or "Feature" item) */
    0x81, 0x00,         /* Input (Data, Array, Abs): Instantiates input packet fields based on the
                           above report size, count, logical min/max, and usage. */
    0x19, 0x01,         /* Usage Minimum */
    0x29, 0x40,         /* 64 output usages total (0x01 to 0x40)*/
    0x91, 0x00,         /* Output (Data, Array, Abs): Instantiates output packet fields. Uses same
                           report size and count as "Input" fields, since nothing new/different was
                           specified to the parser since the "Input" item. */
    0xC0                /* End Collection  */
};

/* HID Descriptor */
#if defined (__GNUC__)
UINT8 gu8DeviceDescriptor[] __attribute__((aligned(4))) =
#else
__align(4) UINT8 gu8DeviceDescriptor[] =
#endif
{
    LEN_DEVICE,     /* bLength */
    DESC_DEVICE,    /* bDescriptorType */
    0x10, 0x01,     /* bcdUSB */
    0x00,           /* bDeviceClass */
    0x00,           /* bDeviceSubClass */
    0x00,           /* bDeviceProtocol */
    CEP_MAX_PKT_SIZE,   /* bMaxPacketSize0 */
    /* idVendor */
    USB_VID & 0x00FF,
    ((USB_VID & 0xFF00) >> 8),
    /* idProduct */
    USB_PID & 0x00FF,
    ((USB_PID & 0xFF00) >> 8),
    0x00, 0x00,     /* bcdDevice */
    0x01,           /* iManufacture */
    0x02,           /* iProduct */
    0x00,           /* iSerialNumber - no serial */
    0x01            /* bNumConfigurations */
};

#if defined (__GNUC__)
static UINT8 HID_ConfigurationBlock[] __attribute__((aligned(4))) =
#else
__align(4) static UINT8 HID_ConfigurationBlock[] =
#endif
{
    LEN_CONFIG,     /* bLength */
    DESC_CONFIG,    /* bDescriptorType */
    /* wTotalLength */
    (LEN_CONFIG + LEN_INTERFACE + LEN_HID + LEN_ENDPOINT * 2) & 0x00FF,
    (((LEN_CONFIG + LEN_INTERFACE + LEN_HID + LEN_ENDPOINT * 2) & 0xFF00) >> 8),
    0x01,           /* bNumInterfaces */
    0x01,           /* bConfigurationValue */
    0x00,           /* iConfiguration */
    0x80 | (USBD_SELF_POWERED << 6) | (USBD_REMOTE_WAKEUP << 5),/* bmAttributes */
    USBD_MAX_POWER,         /* MaxPower */

    /* I/F descr: HID */
    LEN_INTERFACE,  /* bLength */
    DESC_INTERFACE, /* bDescriptorType */
    0x00,           /* bInterfaceNumber */
    0x00,           /* bAlternateSetting */
    0x02,           /* bNumEndpoints */
    0x03,           /* bInterfaceClass */
    0x00,           /* bInterfaceSubClass */
    0x00,           /* bInterfaceProtocol */
    0x00,           /* iInterface */

    /* HID Descriptor */
    LEN_HID,        /* Size of this descriptor in UINT8s. */
    DESC_HID,       /* HID descriptor type. */
    0x10, 0x01,     /* HID Class Spec. release number. */
    0x00,           /* H/W target country. */
    0x01,           /* Number of HID class descriptors to follow. */
    DESC_HID_RPT,   /* Descriptor type. */
    /* Total length of report descriptor. */
    sizeof(HID_DeviceReportDescriptor) & 0x00FF,
    ((sizeof(HID_DeviceReportDescriptor) & 0xFF00) >> 8),

    /* EP Descriptor: interrupt in. */
    LEN_ENDPOINT,                       /* bLength */
    DESC_ENDPOINT,                      /* bDescriptorType */
    (INT_IN_EP_NUM | EP_INPUT),         /* bEndpointAddress */
    EP_INT,                             /* bmAttributes */
    /* wMaxPacketSize */
    EPA_MAX_PKT_SIZE & 0x00FF,
    ((EPA_MAX_PKT_SIZE & 0xFF00) >> 8),
    HID_DEFAULT_INT_IN_INTERVAL,        /* bInterval */

    /* EP Descriptor: interrupt out. */
    LEN_ENDPOINT,                       /* bLength */
    DESC_ENDPOINT,                      /* bDescriptorType */
    (INT_OUT_EP_NUM | EP_OUTPUT),   /* bEndpointAddress */
    EP_INT,                             /* bmAttributes */
    /* wMaxPacketSize */
    EPB_MAX_PKT_SIZE & 0x00FF,
    ((EPB_MAX_PKT_SIZE & 0xFF00) >> 8),
    HID_DEFAULT_INT_IN_INTERVAL         /* bInterval */
};


#if defined (__GNUC__)
static UINT8 HID_ConfigurationBlock_FS[] __attribute__((aligned(4))) =
#else
__align(4) static UINT8 HID_ConfigurationBlock_FS[] =
#endif
{
    LEN_CONFIG,     /* bLength */
    DESC_CONFIG,    /* bDescriptorType */
    /* wTotalLength */
    (LEN_CONFIG + LEN_INTERFACE + LEN_HID + LEN_ENDPOINT * 2) & 0x00FF,
    (((LEN_CONFIG + LEN_INTERFACE + LEN_HID + LEN_ENDPOINT * 2) & 0xFF00) >> 8),
    0x01,           /* bNumInterfaces */
    0x01,           /* bConfigurationValue */
    0x00,           /* iConfiguration */
    0x80 | (USBD_SELF_POWERED << 6) | (USBD_REMOTE_WAKEUP << 5),/* bmAttributes */
    USBD_MAX_POWER,         /* MaxPower */

    /* I/F descr: HID */
    LEN_INTERFACE,  /* bLength */
    DESC_INTERFACE, /* bDescriptorType */
    0x00,           /* bInterfaceNumber */
    0x00,           /* bAlternateSetting */
    0x02,           /* bNumEndpoints */
    0x03,           /* bInterfaceClass */
    0x00,           /* bInterfaceSubClass */
    0x00,           /* bInterfaceProtocol */
    0x00,           /* iInterface */

    /* HID Descriptor */
    LEN_HID,        /* Size of this descriptor in UINT8s. */
    DESC_HID,       /* HID descriptor type. */
    0x10, 0x01,     /* HID Class Spec. release number. */
    0x00,           /* H/W target country. */
    0x01,           /* Number of HID class descriptors to follow. */
    DESC_HID_RPT,   /* Descriptor type. */
    /* Total length of report descriptor. */
    sizeof(HID_DeviceReportDescriptor_FS) & 0x00FF,
    ((sizeof(HID_DeviceReportDescriptor_FS) & 0xFF00) >> 8),

    /* EP Descriptor: interrupt in. */
    LEN_ENDPOINT,                       /* bLength */
    DESC_ENDPOINT,                      /* bDescriptorType */
    (INT_IN_EP_NUM | EP_INPUT),         /* bEndpointAddress */
    EP_INT,                             /* bmAttributes */
    /* wMaxPacketSize */
    EPA_OTHER_MAX_PKT_SIZE & 0x00FF,
    ((EPA_OTHER_MAX_PKT_SIZE & 0xFF00) >> 8),
    HID_DEFAULT_INT_IN_INTERVAL,        /* bInterval */

    /* EP Descriptor: interrupt out. */
    LEN_ENDPOINT,                       /* bLength */
    DESC_ENDPOINT,                      /* bDescriptorType */
    (INT_OUT_EP_NUM | EP_OUTPUT),   /* bEndpointAddress */
    EP_INT,                             /* bmAttributes */
    /* wMaxPacketSize */
    EPB_OTHER_MAX_PKT_SIZE & 0x00FF,
    ((EPB_OTHER_MAX_PKT_SIZE & 0xFF00) >> 8),
    HID_DEFAULT_INT_IN_INTERVAL         /* bInterval */
};

/* Identifier Language */
#if defined (__GNUC__)
static UINT8 HID_StringDescriptor0[4] __attribute__((aligned(4))) =
#else
__align(4) static UINT8 HID_StringDescriptor0[4] = 
#endif
{
    4,              /* bLength */
    USB_DT_STRING,  /* bDescriptorType */
    0x09, 0x04
};

/* iManufacturer */
#if defined (__GNUC__)
UINT8 HID_StringDescriptor1[] __attribute__((aligned(4))) =
#else
__align(4) UINT8 HID_StringDescriptor1[] = 
#endif
{
    0x10,    /* bLength */
    0x03,    /* bDescriptorType */
    'n', 0, 'u', 0, 'v', 0, 'o', 0, 'T', 0, 'o', 0, 'n', 0
};

/* iProduct */
#if defined (__GNUC__)
UINT8 HID_StringDescriptor2[] __attribute__((aligned(4))) =
#else
__align(4) UINT8 HID_StringDescriptor2[] = 
#endif
{
    0x10,          /* bLength */
    0x03,          /* bDescriptorType */
    'U', 0, 'S', 0, 'B', 0, ' ', 0, 'H', 0, 'I', 0, 'D', 0
};

/* iSerialNumber */
#if defined (__GNUC__)
UINT8 HID_StringDescriptor3[] __attribute__((aligned(4))) =
#else
__align(4) UINT8 HID_StringDescriptor3[] = 
#endif
{
    0x1A,         /* bLength */
    0x03,         /* bDescriptorType */
    '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '5', 0, '5', 0, 'F', 0, 'A', 0, '9', 0, '2', 0
};


/* HID High Speed Init */
void hidHighSpeedInit(void)
{
    usbdInfo.usbdMaxPacketSize = 0x40;
    outp32(EPA_MPS, 0x40);                    /* mps */
    while(inp32(EPA_MPS) != 0x40);            /* mps */

    /* Interrupt in */
    outp32(EPA_IRQ_ENB, 0x00000008);          /* tx transmitted */
    outp32(EPA_RSP_SC, 0x000000000);          /* auto validation */
    outp32(EPA_MPS, EPA_MAX_PKT_SIZE);        /* mps 512 */
    outp32(EPA_CFG, 0x0000001D);              /* Interrupt in ep no 1 */
    outp32(EPA_START_ADDR, EPA_BASE);
    outp32(EPA_END_ADDR, EPA_BASE + EPA_MAX_PKT_SIZE - 1);

    /* Interrupt out */
    outp32(EPB_IRQ_ENB, 0x00000010);          /* data pkt received */
    outp32(EPB_RSP_SC, 0x00000000);           /* auto validation */
    outp32(EPB_MPS, EPB_MAX_PKT_SIZE);        /* mps 512 */
    outp32(EPB_CFG, 0x00000025);              /* Interrupt out ep no 2 */
    outp32(EPB_START_ADDR, EPB_BASE);
    outp32(EPB_END_ADDR, EPB_BASE + EPB_MAX_PKT_SIZE - 1);

    g_u32EPA_MXP = EPA_MAX_PKT_SIZE;
    g_u32EPB_MXP = EPB_MAX_PKT_SIZE;

    g_u32ReadWriteSize = EPA_MAX_PKT_SIZE;

    usbdInfo.pu32HIDRPTDescriptor[0] =  (PUINT32) &HID_DeviceReportDescriptor;
    usbdInfo.u32HIDRPTDescriptorLen[0] = sizeof(HID_DeviceReportDescriptor);
}

/* HID Full Speed Init */
void hidFullSpeedInit(void)
{       
    usbdInfo.usbdMaxPacketSize = 0x40;
    outp32(EPA_MPS, 0x40);                    /* mps */
    while(inp32(EPA_MPS) != 0x40);            /* mps */

    /* Interrupt in */
    outp32(EPA_IRQ_ENB, DATA_TxED_IE);        /* tx transmitted */
    outp32(EPA_RSP_SC, 0x000000000);          /* auto validation */
    outp32(EPA_MPS, EPA_OTHER_MAX_PKT_SIZE);  /* mps 64 */
    outp32(EPA_CFG, 0x0000001D);              /* Interrupt in ep no 1 */
    outp32(EPA_START_ADDR, EPA_OTHER_BASE);
    outp32(EPA_END_ADDR, EPA_OTHER_BASE + EPA_OTHER_MAX_PKT_SIZE - 1);

    /* Interrupt out */
    outp32(EPB_IRQ_ENB, 0x00000010);          /* data pkt received */
    outp32(EPB_RSP_SC, 0x00000000);           /* auto validation */
    outp32(EPB_MPS, EPB_OTHER_MAX_PKT_SIZE);  /* mps 64 */
    outp32(EPB_CFG, 0x00000025);              /* Interrupt out ep no 2 */
    outp32(EPB_START_ADDR, EPB_OTHER_BASE);
    outp32(EPB_END_ADDR, EPB_OTHER_BASE + EPB_OTHER_MAX_PKT_SIZE - 1);

    g_u32EPA_MXP = EPA_OTHER_MAX_PKT_SIZE;
    g_u32EPB_MXP = EPB_OTHER_MAX_PKT_SIZE;

#ifdef __FORCE_FULLSPEED__
    outp32(OPER, 0);
#endif
    usbdInfo.pu32HIDRPTDescriptor[0] =  (PUINT32) &HID_DeviceReportDescriptor_FS;
    usbdInfo.u32HIDRPTDescriptorLen[0] = sizeof(HID_DeviceReportDescriptor_FS);
}

void hidClassOUT(void)
{
    if(_usb_cmd_pkt.bRequest == HID_SET_IDLE)
    {
//        sysprintf("\rSet IDLE\n");
    }
    else if(_usb_cmd_pkt.bRequest == HID_SET_REPORT)
    {
        u32Ready = 1;
        sysprintf("\rSET_REPORT 0x%X\n",inp8(CEP_DATA_BUF));
    }
}

void GetInfo(UINT32 u32Arg1, UINT32 u32Arg2, UINT32 u32Address)
{
     *((unsigned int *)u32Address) = g_Parameter[u32Arg1][u32Arg2];
     sysprintf("\rGetInfo %d %d = %d\n",u32Arg1, u32Arg2, g_Parameter[u32Arg1][u32Arg2]);
}

void SetInfo(UINT32 u32Arg1, UINT32 u32Arg2, UINT32 u32Address)
{
     g_Parameter[u32Arg1][u32Arg2] = *((unsigned int *)u32Address);
     sysprintf("\rSetInfo %d %d = %d\n",u32Arg1, u32Arg2, g_Parameter[u32Arg1][u32Arg2]);
}

INT32 HID_CmdReadImage(CMD_T *pCmd)
{
    UINT32 u32Address;
    UINT32 u32Size;

    u32Size     = pCmd->u32Arg1;

    sysprintf("\rRead Image command - u32Size: %d\n", u32Size);

    /* The signature is used to data counter */
    pCmd->u32Signature = 0;

    if(u32Size)
    {
        u32Address = (UINT32)g_pu8FrameBuffer | 0x80000000;

        /* Update data to page buffer to upload */
        memcpy((UINT8 *)g_u8PageBuff, (UINT8 *) (u32Address), g_u32EPA_MXP);

        /* The signature word is used as page counter */
        pCmd->u32Signature = g_u32EPA_MXP;

        while(usbdInfo.USBModeFlag)
        {
            if((inp32(DMA_CTRL_STS) & 0x00000020) == 0)
                break;
        }
        /* Trigger HID IN */
        outp32(DMA_CTRL_STS, 0x11);    /* bulk in, dma read, ep1 */
        outp32(AHB_DMA_ADDR, (UINT32)g_u8PageBuff);
        outp32(DMA_CNT, g_u32EPA_MXP);
        outp32(DMA_CTRL_STS, inp32(DMA_CTRL_STS)|0x00000020);
        while(usbdInfo.USBModeFlag)
        {
            if((inp32(DMA_CTRL_STS) & 0x00000020) == 0)
                break;
        }
    }

    return 0;
}


INT32 HID_CmdGetStatus(CMD_T *pCmd)
{
    *((unsigned int *)g_Temp) = g_Status;
    while(usbdInfo.USBModeFlag)
    {
        if((inp32(DMA_CTRL_STS) & 0x00000020) == 0)
            break;
    }
    /* Trigger HID IN */
    outp32(DMA_CTRL_STS, 0x11);    /* bulk in, dma read, ep1 */
    outp32(AHB_DMA_ADDR, (UINT32)g_Temp);
    outp32(DMA_CNT, g_u32EPA_MXP);
    outp32(DMA_CTRL_STS, inp32(DMA_CTRL_STS)|0x00000020);
    while(usbdInfo.USBModeFlag)
    {
        if((inp32(DMA_CTRL_STS) & 0x00000020) == 0)
            break;
    }
    return 0;
}

INT32 HID_CmdGetVersion(CMD_T *pCmd)
{
    *((unsigned int *)g_Temp) = g_Version;
    while(usbdInfo.USBModeFlag)
    {
        if((inp32(DMA_CTRL_STS) & 0x00000020) == 0)
            break;
    }
    /* Trigger HID IN */
    outp32(DMA_CTRL_STS, 0x11);    /* bulk in, dma read, ep1 */
    outp32(AHB_DMA_ADDR, (UINT32)g_Temp);
    outp32(DMA_CNT, g_u32EPA_MXP);
    outp32(DMA_CTRL_STS, inp32(DMA_CTRL_STS)|0x00000020);
    while(usbdInfo.USBModeFlag)
    {
        if((inp32(DMA_CTRL_STS) & 0x00000020) == 0)
            break;
    }
    return 0;
}

INT32 HID_CmdGetParameter(CMD_T *pCmd)
{
    GetInfo(pCmd->u32Arg1, pCmd->u32Arg2, (UINT32)g_Temp);

    while(usbdInfo.USBModeFlag)
    {
        if((inp32(DMA_CTRL_STS) & 0x00000020) == 0)
            break;
    }
    /* Trigger HID IN */
    outp32(DMA_CTRL_STS, 0x11);    /* bulk in, dma read, ep1 */
    outp32(AHB_DMA_ADDR, (UINT32)g_Temp);
    outp32(DMA_CNT, g_u32EPA_MXP);
    outp32(DMA_CTRL_STS, inp32(DMA_CTRL_STS)|0x00000020);
    while(usbdInfo.USBModeFlag)
    {
        if((inp32(DMA_CTRL_STS) & 0x00000020) == 0)
            break;
    }
    return 0;
}

INT32 HID_CmdImageWrite(CMD_T *pCmd)
{
    UINT32 u32Size;

    u32Size     = pCmd->u32Arg1;

    sysprintf("\rImage Write command - u32Size: %d\n", u32Size);

    g_u32BytesInJPEGBuf = 0;

    /* The signature is used to data counter */
    pCmd->u32Signature = 0;

    return 0;
}

INT32 HID_CmdTest(CMD_T *pCmd)
{
    INT32 i;
    UINT8 *pu8;

    pu8 = (UINT8 *)pCmd;
    sysprintf("\rGet test command #%d (%d bytes)\n", gi32CmdTestCnt++, pCmd->u8Size);
    for(i=0; i<pCmd->u8Size; i++)
    {
        if((i&0xF) == 0)
        {
            sysprintf("\n");
        }
        sysprintf(" %02x", pu8[i]);
    }

    sysprintf("\n");


    /* To note the command has been done */
    pCmd->u8Cmd = HID_CMD_NONE;

    return 0;
}

UINT32 CalCheckSum(UINT8 *buf, UINT32 size)
{
    UINT32 sum;
    INT32 i;

    i = 0;
    sum = 0;
    while(size--)
    {
        sum+=buf[i++];
    }

    return sum;
}

INT32 ProcessCommand(UINT32 u32BufferLen)
{
    UINT32 u32sum;
    while(usbdInfo.USBModeFlag)
    {
        if((inp32(DMA_CTRL_STS) & 0x00000020) == 0)
            break;
    }
    /* Read CMD for OUT Endpoint */
    outp32(DMA_CTRL_STS, 0x02);    /* bulk out, dma write, ep2 */
    outp32(AHB_DMA_ADDR, (UINT32)pCmd);
    outp32(DMA_CNT, u32BufferLen);
    outp32(DMA_CTRL_STS, inp32(DMA_CTRL_STS)|0x00000020);
    while(usbdInfo.USBModeFlag)
    {
        if((inp32(DMA_CTRL_STS) & 0x00000020) == 0)
            break;
    }

    /* Check size */
    if((pCmd->u8Size > sizeof(gCmd)) || (pCmd->u8Size > u32BufferLen))
        return -1;

    /* Check signature */
    if(pCmd->u32Signature != HID_CMD_SIGNATURE)
        return -1;

    /* Calculate checksum & check it*/
    u32sum = CalCheckSum((UINT8 *)pCmd, pCmd->u8Size);
    if(u32sum != pCmd->u32Checksum)
        return -1;

    switch(pCmd->u8Cmd)
    {
        case HID_CMD_GET_STS:
        {
            HID_CmdGetStatus(pCmd);
            break;
        }

        case HID_CMD_GET_VER:
        {
            HID_CmdGetVersion(pCmd);
            break;
        }

        case HID_CMD_EXIT:
        {
            sysEnableWatchDogTimer();
            sysEnableWatchDogTimerReset();
            while(1);

        }

        case HID_CMD_SET_PARAM:
        {
            break;
        }

        case HID_CMD_GET_PARAM:
        {
            HID_CmdGetParameter(pCmd);
            break;
        }

        case HID_CMD_IMAGE_WRITE:
        {
            HID_CmdImageWrite(pCmd);
            break;
        }

        case HID_CMD_IMAGE_READ:
        {
            HID_CmdReadImage(pCmd);
            break;
        }

        case HID_CMD_TEST:
        {
            HID_CmdTest(pCmd);
            break;
        }
        default:
            return -1;
    }

    return 0;
}

void HID_GetOutReport(void)
{
    UINT8  u8Cmd;
    UINT32 u32TotalCnt;
    UINT32 u32DataCnt;
    UINT32 u32Size = inp32(EPB_DATA_CNT) & 0xFFFF;

    /* Get command information */
    u8Cmd        = pCmd->u8Cmd;
    u32TotalCnt  = pCmd->u32Arg1;     /* The word is used to target data count for IMAGE_WRITE CMD */  
    u32DataCnt   = pCmd->u32Signature; /* The signature word is used to count data for IMAGE_WRITE CMD */

    /* Check if it is in the data phase of image write command */
    if((u8Cmd == HID_CMD_IMAGE_WRITE) &&  (u32DataCnt < u32TotalCnt))
    {
        while(usbdInfo.USBModeFlag)
        {
            if((inp32(DMA_CTRL_STS) & 0x00000020) == 0)
                break;
        }
        /* Process the data phase of write command */
        outp32(DMA_CTRL_STS, 0x02);    /* bulk out, dma write, ep2 */
        outp32(AHB_DMA_ADDR, (UINT32)&g_u8JPEGBuff[g_u32BytesInJPEGBuf]);
        outp32(DMA_CNT, u32Size);
        outp32(DMA_CTRL_STS, inp32(DMA_CTRL_STS)|0x00000020);
        while(usbdInfo.USBModeFlag)
        {
            if((inp32(DMA_CTRL_STS) & 0x00000020) == 0)
                break;
        }
        g_u32BytesInJPEGBuf += u32Size;

        /* Write command complete! */
        if(g_u32BytesInJPEGBuf >= u32TotalCnt)
        {
            pCmd->u8Cmd = HID_CMD_NONE;
            JpegDec((UINT32)g_u8JPEGBuff,(UINT32) g_pu8FrameBuffer);
        }
        else
        {   
            /* Update command status */
            pCmd->u8Cmd        = u8Cmd;
            pCmd->u32Signature = u32DataCnt;
        }
    }
    /* Check if it is in the data phase of image set parameter command */
    else if(u8Cmd == HID_CMD_SET_PARAM)
    {
        while(usbdInfo.USBModeFlag)
        {
            if((inp32(DMA_CTRL_STS) & 0x00000020) == 0)
                break;
        }
        /* Trigger HID IN */
        outp32(DMA_CTRL_STS, 0x02);    /* bulk out, dma read, ep2 */
        outp32(AHB_DMA_ADDR, (UINT32)g_Temp);
        outp32(DMA_CNT, g_u32EPA_MXP);
        outp32(DMA_CTRL_STS, inp32(DMA_CTRL_STS)|0x00000020);
        while(usbdInfo.USBModeFlag)
        {
            if((inp32(DMA_CTRL_STS) & 0x00000020) == 0)
            break;
        }
        SetInfo(pCmd->u32Arg1, pCmd->u32Arg2, (UINT32)g_Temp);  
        /* The data transfer is complete. */
        pCmd->u8Cmd = HID_CMD_NONE;
    }
    else
    {
        /* Check and process the command packet */
        if(ProcessCommand(u32Size))
        {
            sysprintf("\rUnknown HID command!\n");
        }
    }
}

void HID_SetInReport(void)
{
    UINT32 u32Address;
    UINT32 u32TotalCnt;
    UINT32 u32DataCnt;
    UINT8  u8Cmd;

  /* Get command information */
    u8Cmd        = pCmd->u8Cmd;
    u32TotalCnt  = pCmd->u32Arg1;     /* The word is used to target data count for IMAGE_READ CMD */  
    u32DataCnt   = pCmd->u32Signature; /* The signature word is used to count data for IMAGE_READ CMD */

    /* Check if it is in data phase of read command */
    if((u8Cmd == HID_CMD_IMAGE_READ) &&  (u32DataCnt < u32TotalCnt))
    {
        /* Process the data phase of read command */
        if(u32DataCnt >= u32TotalCnt)
        {
            /* The data transfer is complete. */
            u8Cmd = HID_CMD_NONE;
        }
        else
        {
            u32Address = (UINT32)g_pu8FrameBuffer | 0x80000000;

            /* Update data to page buffer to upload */
            memcpy((UINT8 *)g_u8PageBuff, (UINT8 *) (u32Address + u32DataCnt), g_u32EPA_MXP);

            u32DataCnt += g_u32EPA_MXP;

            while(usbdInfo.USBModeFlag)
            {
                if((inp32(DMA_CTRL_STS) & 0x00000020) == 0)
                    break;
            }
            /* Prepare the data for next HID IN transfer */
            outp32(DMA_CTRL_STS, 0x11);    /* bulk in, dma read, ep1 */
            outp32(AHB_DMA_ADDR, (UINT32)&g_u8PageBuff[0]);
            outp32(DMA_CNT, g_u32EPA_MXP);
            outp32(DMA_CTRL_STS, inp32(DMA_CTRL_STS)|0x00000020);
            while(usbdInfo.USBModeFlag)
            {
                if((inp32(DMA_CTRL_STS) & 0x00000020) == 0)
                    break;
            }
        }
    }

    pCmd->u8Cmd        = u8Cmd;
    pCmd->u32Signature = u32DataCnt;
}

void EPA_Handler(UINT32 u32IntEn,UINT32 u32IntStatus)  /* Interrupt IN handler */
{
    HID_SetInReport();
}

void EPB_Handler(UINT32 u32IntEn,UINT32 u32IntStatus)  /* Interrupt OUT handler */
{
    HID_GetOutReport();
}
void Reset_Handler(VOID)  /* Reset handler */
{
    //isConnectToHost = 1;
}

/* HID Init */
void hidInit(void)
{
    /* Set Endpoint map */
    usbdInfo.i32EPA_Num = INT_IN_EP_NUM;    /* Endpoint 1 */
    usbdInfo.i32EPB_Num = INT_OUT_EP_NUM;   /* Endpoint 2 */
    usbdInfo.i32EPC_Num = -1;               /* Not use */
    usbdInfo.i32EPD_Num = -1;               /* Not use */

    /* Set Callback Function */
    /* Set MSC initialize function */
    usbdInfo.pfnFullSpeedInit = hidFullSpeedInit;
    usbdInfo.pfnHighSpeedInit = hidHighSpeedInit;

    /* Set Descriptor pointer */
    usbdInfo.pu32DevDescriptor = (PUINT32) &gu8DeviceDescriptor;
    usbdInfo.pu32HSConfDescriptor = (PUINT32) &HID_ConfigurationBlock;
    usbdInfo.pu32FSConfDescriptor = (PUINT32) &HID_ConfigurationBlock_FS;
    usbdInfo.pu32HIDDescriptor = (PUINT32) ((UINT32)&HID_ConfigurationBlock + LEN_CONFIG + LEN_INTERFACE);

    usbdInfo.pfnClassDataOUTCallBack = hidClassOUT;

    /* Set the HID report descriptor */
    usbdInfo.pu32HIDRPTDescriptor[0] =  (PUINT32) &HID_DeviceReportDescriptor;
    usbdInfo.u32HIDRPTDescriptorLen[0] = sizeof(HID_DeviceReportDescriptor);
    usbdInfo.pfnEPACallBack = EPA_Handler;
    usbdInfo.pfnEPBCallBack = EPB_Handler;
    usbdInfo.pfnReset = Reset_Handler;

    usbdInfo.pu32StringDescriptor[0] = (PUINT32) &HID_StringDescriptor0;
    usbdInfo.pu32StringDescriptor[1] = (PUINT32) &HID_StringDescriptor1;
    usbdInfo.pu32StringDescriptor[2] = (PUINT32) &HID_StringDescriptor2;
    usbdInfo.pu32StringDescriptor[3] = (PUINT32) &HID_StringDescriptor3;

    /* Set Descriptor length */	
    usbdInfo.u32DevDescriptorLen =  LEN_DEVICE;
    usbdInfo.u32HSConfDescriptorLen =  LEN_CONFIG_AND_SUBORDINATE;
    usbdInfo.u32FSConfDescriptorLen =  LEN_CONFIG_AND_SUBORDINATE;
    usbdInfo.u32StringDescriptorLen[0] = HID_StringDescriptor0[0] = sizeof(HID_StringDescriptor0);
    usbdInfo.u32StringDescriptorLen[1] = HID_StringDescriptor1[0] = sizeof(HID_StringDescriptor1);
    usbdInfo.u32StringDescriptorLen[2] = HID_StringDescriptor2[0] = sizeof(HID_StringDescriptor2);
    usbdInfo.u32StringDescriptorLen[3] = HID_StringDescriptor3[0] = sizeof(HID_StringDescriptor3);

    pCmd = (CMD_T *)((UINT32)&gCmd | BIT31);
    g_u8PageBuff = (UINT8 *)((UINT32)gu8PageBuff | BIT31);
    g_u8JPEGBuff = (UINT8 *)((UINT32)gu8JPEGBuff | BIT31);
    g_Temp = (UINT8 *)((UINT32)gTemp | BIT31);
}

void HIDStart(void)
{
    /* Enable USB */
    udcOpen();  
    hidInit();
    udcInit();
}
