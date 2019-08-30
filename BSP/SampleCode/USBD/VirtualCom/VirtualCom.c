/****************************************************************************
 * @file     VirtualCom.c
 * @version  V1.00
 * $Revision: 4 $
 * $Date: 18/04/25 11:43a $
 * @brief    Virtual COM sample file
 *
 * @note
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "N9H20.h"

/* length of descriptors */
#define DEVICE_DSCPT_LEN       0x12
#define CONFIG_DSCPT_LEN       0x43
#define STR0_DSCPT_LEN         0x04
#define STR1_DSCPT_LEN         0x10
#define STR2_DSCPT_LEN         0x20
#define STR3_DSCPT_LEN         0x1a
#define QUALIFIER_DSCPT_LEN    0x0a
#define OSCONFIG_DSCPT_LEN     0x43

/* USB Device Property */
extern USB_CMD_T  _usb_cmd_pkt;
extern volatile USBD_INFO_T usbdInfo;

UINT8 _DeviceDescriptor[] __attribute__((aligned(4))) =
{
    0x12,          /* bLength              */
    0x01,          /* bDescriptorType      */
    0x00, 0x02,    /* bcdUSB               */
    0x02,          /* bDeviceClass         */
    0x00,          /* bDeviceSubClass      */
    0x00,          /* bDeviceProtocol      */
    0x40,          /* bMaxPacketSize0      */
    0x16,          /* idVendor             */
    0x04,
    0x11,          /* idProduct            */
    0x50,
    0x00, 0x03,    /* bcdDevice            */
    0x01,          /* iManufacture         */
    0x02,          /* iProduct             */
    0x00,          /* iSerialNumber        */
    0x01           /* bNumConfigurations   */
};


/*!<USB Quflifier Descriptor */
UINT8 _QualifierDescriptor[] __attribute__((aligned(4))) =
{
    0x0a,        /* bLength            */
    0x06,        /* bDescriptorType    */
    0x00, 0x02,  /* bcdUSB             */
    0x00,        /* bDeviceClass       */
    0x00,        /* bDeviceSubClass    */
    0x00,        /* bDeviceProtocol    */
    0x40,        /* bMaxPacketSize0    */
    0x01,        /* bNumConfigurations */
    0x00
};

/*!<USB Configure Descriptor */
UINT8 _ConfigurationBlock[] __attribute__((aligned(4))) =
{
    0x09,        /* bLength              */
    0x02,        /* bDescriptorType      */
    0x43, 0x00,  /* wTotalLength         */
    0x02,        /* bNumInterfaces       */
    0x01,        /* bConfigurationValue  */
    0x00,        /* iConfiguration       */
    0xC0,        /* bmAttributes         */
    0x32,        /* MaxPower             */
/* INTERFACE descriptor */
    0x09,        /* bLength              */
    0x04,        /* bDescriptorType      */
    0x00,        /* bInterfaceNumber     */
    0x00,        /* bAlternateSetting    */
    0x01,        /* bNumEndpoints        */
    0x02,        /* bInterfaceClass      */
    0x02,        /* bInterfaceSubClass   */
    0x01,        /* bInterfaceProtocol   */
    0x00,        /* iInterface           */

/* Communication Class Specified INTERFACE descriptor */
    0x05,        /* Size of the descriptor, in bytes */
    0x24,        /* CS_INTERFACE descriptor type */
    0x00,        /* Header functional descriptor subtype */
    0x10, 0x01,  /* Communication device compliant to the communication spec. ver. 1.10 */

/* Communication Class Specified INTERFACE descriptor */
    0x05,        /* Size of the descriptor, in bytes */
    0x24,        /* CS_INTERFACE descriptor type */
    0x01,        /* Call management functional descriptor */
    0x00,        /* BIT0: Whether device handle call management itself. */
                 /* BIT1: Whether device can send/receive call management information over a Data Class Interface 0 */
    0x01,        /* Interface number of data class interface optionally used for call management */

/* Communication Class Specified INTERFACE descriptor */
    0x04,        /* Size of the descriptor, in bytes */
    0x24,        /* CS_INTERFACE descriptor type */
    0x02,        /* Abstract control management funcational descriptor subtype */
    0x00,        /* bmCapabilities       */
    
/* Communication Class Specified INTERFACE descriptor */
    0x05,        /* bLength              */
    0x24,        /* bDescriptorType: CS_INTERFACE descriptor type */
    0x06,        /* bDescriptorSubType   */
    0x00,        /* bMasterInterface     */
    0x01,        /* bSlaveInterface0     */
    
/* ENDPOINT descriptor */
    0x07,        /* bLength              */
    0x05,        /* bDescriptorType      */
    0x83,        /* bEndpointAddress     */
    0x03,        /* bmAttributes         */
    0x40, 0x00,  /* wMaxPacketSize       */
    0x01,        /* bInterval            */

/* INTERFACE descriptor */
    0x09,        /* bLength              */
    0x04,        /* bDescriptorType      */
    0x01,        /* bInterfaceNumber     */
    0x00,        /* bAlternateSetting    */
    0x02,        /* bNumEndpoints        */
    0x0A,        /* bInterfaceClass      */
    0x00,        /* bInterfaceSubClass   */
    0x00,        /* bInterfaceProtocol   */
    0x00,        /* iInterface           */

/* ENDPOINT descriptor */
    0x07,        /* bLength           */
    0x05,        /* bDescriptorType   */
    0x81,        /* bEndpointAddress  */
    0x02,        /* bmAttributes      */
    0x00, 0x02,  /* wMaxPacketSize    */
    0x00,        /* bInterval         */

/* ENDPOINT descriptor */
    0x07,        /* bLength           */
    0x05,        /* bDescriptorType   */
    0x02,        /* bEndpointAddress  */
    0x02,        /* bmAttributes      */
    0x00, 0x02,  /* wMaxPacketSize    */
    0x00,        /* bInterval         */
};


/*!<USB Configure Descriptor */
UINT8 _ConfigurationBlockFull[] __attribute__((aligned(4))) =
{
    0x09,        /* bLength              */
    0x02,        /* bDescriptorType      */
    0x43, 0x00,  /* wTotalLength         */
    0x02,        /* bNumInterfaces       */
    0x01,        /* bConfigurationValue  */
    0x00,        /* iConfiguration       */
    0xC0,        /* bmAttributes         */
    0x32,        /* MaxPower             */

/* INTERFACE descriptor */
    0x09,        /* bLength              */
    0x04,        /* bDescriptorType      */
    0x00,        /* bInterfaceNumber     */
    0x00,        /* bAlternateSetting    */
    0x01,        /* bNumEndpoints        */
    0x02,        /* bInterfaceClass      */
    0x02,        /* bInterfaceSubClass   */
    0x01,        /* bInterfaceProtocol   */
    0x00,        /* iInterface           */

/* Communication Class Specified INTERFACE descriptor */
    0x05,           /* Size of the descriptor, in bytes */
    0x24,           /* CS_INTERFACE descriptor type */
    0x00,           /* Header functional descriptor subtype */
    0x10, 0x01,     /* Communication device compliant to the communication spec. ver. 1.10 */

/* Communication Class Specified INTERFACE descriptor */
    0x05,           /* Size of the descriptor, in bytes */
    0x24,           /* CS_INTERFACE descriptor type */
    0x01,           /* Call management functional descriptor */
    0x00,           /* BIT0: Whether device handle call management itself. */
                    /* BIT1: Whether device can send/receive call management information over a Data Class Interface 0 */
    0x01,           /* Interface number of data class interface optionally used for call management */

/* Communication Class Specified INTERFACE descriptor */
    0x04,           /* Size of the descriptor, in bytes */
    0x24,           /* CS_INTERFACE descriptor type */
    0x02,           /* Abstract control management funcational descriptor subtype */
    0x00,           /* bmCapabilities       */
    
/* Communication Class Specified INTERFACE descriptor */
    0x05,           /* bLength              */
    0x24,           /* bDescriptorType: CS_INTERFACE descriptor type */
    0x06,           /* bDescriptorSubType   */
    0x00,           /* bMasterInterface     */
    0x01,           /* bSlaveInterface0     */
    
/* ENDPOINT descriptor */
    0x07,           /* bLength           */
    0x05,           /* bDescriptorType   */
    0x83,           /* bEndpointAddress  */
    0x03,           /* bmAttributes      */
    0x40, 0x00,     /* wMaxPacketSize    */
    0x01,           /* bInterval         */

/* INTERFACE descriptor */
    0x09,           /* bLength              */
    0x04,           /* bDescriptorType      */
    0x01,           /* bInterfaceNumber     */
    0x00,           /* bAlternateSetting    */
    0x02,           /* bNumEndpoints        */
    0x0A,           /* bInterfaceClass      */
    0x00,           /* bInterfaceSubClass   */
    0x00,           /* bInterfaceProtocol   */
    0x00,           /* iInterface           */

/* ENDPOINT descriptor */
    0x07,           /* bLength              */
    0x05,           /* bDescriptorType      */
    0x81,           /* bEndpointAddress     */
    0x02,           /* bmAttributes         */
    0x40, 0x00,     /* wMaxPacketSize       */
    0x00,           /* bInterval            */

/* ENDPOINT descriptor */
    0x07,           /* bLength              */
    0x05,           /* bDescriptorType      */
    0x02,           /* bEndpointAddress     */
    0x02,           /* bmAttributes         */
    0x40, 0x00,     /* wMaxPacketSize       */
    0x00,           /* bInterval            */
};


/*!<USB Configure Descriptor */
UINT8 _OSConfigurationBlock[] __attribute__((aligned(4))) =
{
    0x09,           /* bLength              */
    0x07,           /* bDescriptorType      */
    0x43, 0x00,     /* wTotalLength         */
    0x02,           /* bNumInterfaces       */
    0x01,           /* bConfigurationValue  */
    0x00,           /* iConfiguration       */
    0xC0,           /* bmAttributes         */
    0x32,           /* MaxPower             */

/* INTERFACE descriptor */
    0x09,           /* bLength              */
    0x04,           /* bDescriptorType      */
    0x00,           /* bInterfaceNumber     */
    0x00,           /* bAlternateSetting    */
    0x01,           /* bNumEndpoints        */
    0x02,           /* bInterfaceClass      */
    0x02,           /* bInterfaceSubClass   */
    0x01,           /* bInterfaceProtocol   */
    0x00,           /* iInterface           */

/* Communication Class Specified INTERFACE descriptor */
    0x05,           /* Size of the descriptor, in bytes */
    0x24,           /* CS_INTERFACE descriptor type */
    0x00,           /* Header functional descriptor subtype */
    0x10, 0x01,     /* Communication device compliant to the communication spec. ver. 1.10 */

/* Communication Class Specified INTERFACE descriptor */
    0x05,           /* Size of the descriptor, in bytes */
    0x24,           /* CS_INTERFACE descriptor type */
    0x01,           /* Call management functional descriptor */
    0x00,           /* BIT0: Whether device handle call management itself. */
                    /* BIT1: Whether device can send/receive call management information over a Data Class Interface 0 */
    0x01,           /* Interface number of data class interface optionally used for call management */

/* Communication Class Specified INTERFACE descriptor */
    0x04,           /* Size of the descriptor, in bytes */
    0x24,           /* CS_INTERFACE descriptor type */
    0x02,           /* Abstract control management funcational descriptor subtype */
    0x00,           /* bmCapabilities       */
    
/* Communication Class Specified INTERFACE descriptor */
    0x05,           /* bLength              */
    0x24,           /* bDescriptorType: CS_INTERFACE descriptor type */
    0x06,           /* bDescriptorSubType   */
    0x00,           /* bMasterInterface     */
    0x01,           /* bSlaveInterface0     */
    
/* ENDPOINT descriptor */
    0x07,           /* bLength              */
    0x05,           /* bDescriptorType      */
    0x83,           /* bEndpointAddress     */
    0x03,           /* bmAttributes         */
    0x40, 0x00,     /* wMaxPacketSize       */
    0x01,           /* bInterval            */

/* INTERFACE descriptor */
    0x09,           /* bLength              */
    0x04,           /* bDescriptorType      */
    0x01,           /* bInterfaceNumber     */
    0x00,           /* bAlternateSetting    */
    0x02,           /* bNumEndpoints        */
    0x0A,           /* bInterfaceClass      */
    0x00,           /* bInterfaceSubClass   */
    0x00,           /* bInterfaceProtocol   */
    0x00,           /* iInterface           */

/* ENDPOINT descriptor */
    0x07,           /* bLength              */
    0x05,           /* bDescriptorType      */
    0x81,           /* bEndpointAddress     */
    0x02,           /* bmAttributes         */
    0x40, 0x00,     /* wMaxPacketSize       */
    0x00,           /* bInterval            */

/* ENDPOINT descriptor */
    0x07,           /* bLength              */
    0x05,           /* bDescriptorType      */
    0x02,           /* bEndpointAddress     */
    0x02,           /* bmAttributes         */
    0x40, 0x00,     /* wMaxPacketSize       */
    0x00,           /* bInterval            */
};

/*!<USB Language String Descriptor */
UINT8 _StringDescriptor0[4] __attribute__((aligned(4))) =
{
    4,            /* bLength           */
    0x03,        /* bDescriptorType    */
    0x09, 0x04
};

UINT8 _StringDescriptor1[] __attribute__((aligned(4))) =
{
    16,
    0x03,
    'N', 0, 'u', 0, 'v', 0, 'o', 0, 't', 0, 'o', 0, 'n', 0
};

UINT8 _StringDescriptor2[] __attribute__((aligned(4))) =
{
    32,             /* bLength            */
    0x03,           /* bDescriptorType    */
    'U', 0, 'S', 0, 'B', 0, ' ', 0, 'V', 0, 'i', 0, 'r', 0, 't', 0, 'u', 0, 'a', 0, 'l', 0, 
    ' ', 0, 'C', 0, 'O', 0, 'M', 0
};


/************************************************/
/* for CDC class */
typedef struct { 
    UINT32  u32DTERate;     /* Baud rate    */
    UINT8   u8CharFormat;   /* stop bit     */
    UINT8   u8ParityType;   /* parity       */
    UINT8   u8DataBits;     /* data bits    */
} S_VCOM_LINE_CODING;


S_VCOM_LINE_CODING gLineCoding = {115200, 0, 0, 8};    /* Baud rate : 115200    */
                                                       /* Stop bit     */
                                                       /* parity       */
                                                       /* data bits    */

#define BUFFER_SIZE  0x100000

UINT32 volatile  u32RxPoint =0, u32TxPoint = 0, u32DataCount =0;

unsigned char rec[BUFFER_SIZE + 512] __attribute__((aligned(64)));

unsigned char rec_tmp[512] __attribute__((aligned(64)));

void vcomHighSpeedInit()
{
    usbdInfo.usbdMaxPacketSize = 0x200;
    /* bulk in */
    outp32(EPA_RSP_SC, 0x00000000);     /* auto validation */
    outp32(EPA_MPS, 0x00000200);        /* mps 512 */
    outp32(EPA_CFG, 0x0000001b);        /* Bulk in, ep no 1 */
    outp32(EPA_START_ADDR, 0x00000100);
    outp32(EPA_END_ADDR, 0x000002ff);

    /* bulk out */
    outp32(EPB_IRQ_ENB, 0x00000010);    /* data pkt received */
    outp32(EPB_RSP_SC, 0x00000000);     /* auto validation */
    outp32(EPB_MPS, 0x00000200);        /* mps 512 */
    outp32(EPB_CFG, 0x00000023);        /* Bulk out ep no 2 */
    outp32(EPB_START_ADDR, 0x00000300);
    outp32(EPB_END_ADDR, 0x000004ff);

    /* interrupt in */
    outp32(EPC_RSP_SC, 0x00000000);     /* auto validation */
    outp32(EPC_MPS, 0x00000200);        /* mps 512 */
    outp32(EPC_CFG, 0x0000003d);        /* bulk in ep no 3 */
    outp32(EPC_START_ADDR, 0x00000500);
    outp32(EPC_END_ADDR, 0x000006ff);
}

void vcomFullSpeedInit()
{
    usbdInfo.usbdMaxPacketSize = 0x40;
    /* bulk in */
    outp32(EPA_RSP_SC, 0x00000000);     /* auto validation */
    outp32(EPA_MPS, 0x00000040);        /* mps 64 */
    outp32(EPA_CFG, 0x0000001b);        /* bulk in ep no 1 */
    outp32(EPA_START_ADDR, 0x00000100);
    outp32(EPA_END_ADDR, 0x0000017f);

    /* bulk out */
    outp32(EPB_IRQ_ENB, 0x00000010);    /* data pkt received */
    outp32(EPB_RSP_SC, 0x00000000);     /* auto validation */
    outp32(EPB_MPS, 0x00000040);        /* mps 64 */
    outp32(EPB_CFG, 0x00000023);        /* bulk out ep no 2 */
    outp32(EPB_START_ADDR, 0x00000200);
    outp32(EPB_END_ADDR, 0x0000027f);

    /* interrupt in */
    outp32(EPC_RSP_SC, 0x00000000);     /* auto validation */
    outp32(EPC_MPS, 0x00000040);        /* mps 64 */
    outp32(EPC_CFG, 0x0000003d);        /* bulk in ep no 3 */
    outp32(EPC_START_ADDR, 0x00000300);
    outp32(EPC_END_ADDR, 0x0000037f);
}

/* USB Endpoint B Interrupt Callback function */
VOID VCOM_EPB_CallBack(UINT32 u32IntEn,UINT32 u32IntStatus)
{
    /* Receive data from HOST */
    if(u32IntStatus & (DATA_RxED_IS | SHORT_PKT_IS))
    {
        UINT32 len;

        len = inp32(EPB_DATA_CNT) & 0xffff;                  /* Get data from Endpoint FIFO */   

        outp32(DMA_CTRL_STS, 0x02);                          /* Config DMA - Bulk OUT, Write data from Endpoint Buufer to DRAM, EP2 */

        if((UINT32)&rec[u32RxPoint] & 0x3)                   /* Address for DMA must be word-aligned */
        {
            outp32(AHB_DMA_ADDR, (UINT32)&rec_tmp[0]);       /* Using word-aligned for DMA */
        }
        else
            outp32(AHB_DMA_ADDR, (UINT32)&rec[u32RxPoint]);

        outp32(DMA_CNT, len);                                /* DMA length */

        outp32(DMA_CTRL_STS, inp32(DMA_CTRL_STS)|DMA_EN);    /* Trigger DMA */

        while(inp32(DMA_CTRL_STS) & DMA_EN);                 /* Wait DMA Ready */

        if((UINT32)&rec[u32RxPoint] & 0x3)
        {
            memcpy(&rec[u32RxPoint], rec_tmp, len);
        }

        u32RxPoint += len;

        u32DataCount += len;

        if(u32RxPoint >= BUFFER_SIZE)                        /* Check DRAM Buffer - Buffer size is BUFFER_SIZE + EPB Maximum Packet Size(512) */
            u32RxPoint = 0;

        sysprintf("Rx %3d - RxPoint %8d TxPoint %8d Data %8d\n",len, u32RxPoint, u32TxPoint,u32DataCount);

    }
}

/* USB Endpoint A Interrupt Callback function */
VOID VCOM_EPA_CallBack(UINT32 u32IntEn,UINT32 u32IntStatus)
{
    /* Send data to HOST */
    if(u32IntStatus & IN_TK_IS)                              /* IN Token Interrupt - Host wants to get data from Device */
    {
        UINT32 volatile len = inp32(EPA_MPS);                /* Get EPA Maximum Packet */
        if(u32DataCount == 0)         
            return;
      
        if(u32DataCount < len)                               /* Data is less than Maximum Packet */
            len = u32DataCount;

        if((len+inp32(EPA_DATA_CNT)) > inp32(EPA_MPS))       /* Endpopint Buffer is not enough - Must wait Host read data to release more space */
        {
            sysprintf("IN Endpoint Buffer is Full!!!\n");
            return;
        }

        while(inp32(DMA_CTRL_STS) & DMA_EN);                 /* Wait DMA Ready */

        outp32(DMA_CTRL_STS, 0x11);                          /* Config DMA - Bulk IN, Read data from DRAM to Endpoint Buufer, EP1 */

        if((UINT32)&rec[u32TxPoint] & 0x3)                   /* Address for DMA must be word-aligned */
        {
            memcpy(rec_tmp, &rec[u32TxPoint], len);          /* Using word-aligned for DMA */ 
            outp32(AHB_DMA_ADDR, (UINT32)&rec_tmp[0]);
        }
        else
            outp32(AHB_DMA_ADDR, (UINT32)&rec[u32TxPoint]);

        outp32(DMA_CNT, len);                                /* DMA length */

        outp32(DMA_CTRL_STS, inp32(DMA_CTRL_STS)|DMA_EN);    /* Trigger DMA */

        while(inp32(DMA_CTRL_STS) & DMA_EN);                 /* Wait DMA Complete */

        if(inp32(EPA_DATA_CNT) < inp32(EPA_MPS))             /* If data is less than Maximum packet size */
            outp32(EPA_RSP_SC,  PK_END);                     /* Trigger Short Packet */

        u32TxPoint += len;
        u32DataCount -= len;

        if(u32TxPoint >= BUFFER_SIZE)                        /* Check DRAM Buffer - Buffer size is BUFFER_SIZE + EPB Maximum Packet Size(512) */
            u32TxPoint = 0;                                  /* Reset Tx Pointer */

        sysprintf("Tx %3d - RxPoint %8d TxPoint %8d Data %8d\n",len, u32RxPoint, u32TxPoint,u32DataCount);
    }
}

/* USB Class Data IN Callback function */
VOID VCOM_ClassDataIn(VOID)
{
    int i = 0;
    char buf[64];
    int class_in_len = _usb_cmd_pkt.wLength;
    if(class_in_len)
    {
        memcpy(buf, (UINT8 *)&gLineCoding, class_in_len);

        for (i=0; i<class_in_len; i++)
            outp8(CEP_DATA_BUF, buf[i]);
    }
    outp32(IN_TRNSFR_CNT, class_in_len);
}


/* USB Class Data OUT Callback function */
VOID VCOM_ClassDataOut(VOID)
{
    int i = 0;
    char buf[64];
    int class_out_len = _usb_cmd_pkt.wLength;

    if( _usb_cmd_pkt.wLength != 0)
    {
        if(class_out_len)
        {
            for (i=0; i<class_out_len; i++)
                buf[i] = inp8(CEP_DATA_BUF);

            memcpy((UINT8 *)&gLineCoding, buf, class_out_len);
        }
    }
}

void udcVcomTest(void)
{
    outp32(EPB_IRQ_ENB, inp32(EPB_IRQ_ENB) | O_SHORT_PKT_IE | DATA_RxED_IE);    
    outp32(EPA_IRQ_ENB, IN_TK_IE);
    u32TxPoint = 0;
    u32RxPoint = 0;
    u32DataCount = 0;
    
    while(1);
}

/* VCOM Init */
void vcomInit(void)
{
    /* Set Endpoint map */
    usbdInfo.i32EPA_Num = 1;    /* Endpoint 1 */
    usbdInfo.i32EPB_Num = 2;    /* Endpoint 2 */
    usbdInfo.i32EPC_Num = 3;    /* Endpoint 3 */
    usbdInfo.i32EPD_Num = -1;   /* Not use */

    /* Set Callback Function */
    usbdInfo.pfnClassDataOUTCallBack = VCOM_ClassDataOut;
    usbdInfo.pfnClassDataINCallBack = VCOM_ClassDataIn;
    usbdInfo.pfnEPACallBack = VCOM_EPA_CallBack;
    usbdInfo.pfnEPBCallBack = VCOM_EPB_CallBack;

    /* Set MSC initialize function */
    usbdInfo.pfnFullSpeedInit = vcomFullSpeedInit;
    usbdInfo.pfnHighSpeedInit = vcomHighSpeedInit;

    /* Set Descriptor pointer */
    usbdInfo.pu32DevDescriptor = (PUINT32) &_DeviceDescriptor;
    usbdInfo.pu32QulDescriptor = (PUINT32) &_QualifierDescriptor;
    usbdInfo.pu32HSConfDescriptor = (PUINT32) &_ConfigurationBlock;
    usbdInfo.pu32FSConfDescriptor = (PUINT32) &_ConfigurationBlockFull;
    usbdInfo.pu32HOSConfDescriptor = (PUINT32) &_OSConfigurationBlock;
    usbdInfo.pu32FOSConfDescriptor = (PUINT32) &_OSConfigurationBlock;

    usbdInfo.pu32StringDescriptor[0] = (PUINT32) &_StringDescriptor0;
    usbdInfo.pu32StringDescriptor[1] = (PUINT32) &_StringDescriptor1;
    usbdInfo.pu32StringDescriptor[2] = (PUINT32) &_StringDescriptor2;

    /* Set Descriptor length */
    usbdInfo.u32DevDescriptorLen =  DEVICE_DSCPT_LEN;
    usbdInfo.u32QulDescriptorLen =  QUALIFIER_DSCPT_LEN;
    usbdInfo.u32HSConfDescriptorLen =  CONFIG_DSCPT_LEN;
    usbdInfo.u32FSConfDescriptorLen =  CONFIG_DSCPT_LEN;
    usbdInfo.u32HOSConfDescriptorLen =  OSCONFIG_DSCPT_LEN;
    usbdInfo.u32FOSConfDescriptorLen =  OSCONFIG_DSCPT_LEN;
    usbdInfo.u32StringDescriptorLen[0] = STR0_DSCPT_LEN;
    usbdInfo.u32StringDescriptorLen[1] = STR1_DSCPT_LEN;
    usbdInfo.u32StringDescriptorLen[2] = STR2_DSCPT_LEN;
}

void VCOM_MainProcess(void)
{   
    while (1)
    {
        if (usbdInfo.USBStartFlag)  
        {
            outp32(EPA_RSP_SC, BUF_FLUSH);    /* flush fifo */
            break;
        }
    }
    while (1)
    {
        if (usbdInfo.USBStartFlag)
            udcVcomTest();
    }
}

