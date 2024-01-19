/**************************************************************************//**
 * @file     PTR.c
 * @brief    PTR Class Device sample source file
 *           - Device Descriptor
 *           - USB Device Callback functions
 *           - PTR Print Message function
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wblib.h"
#include "N9H20.h"
#include "PTR.h"

/* Mass_Storage command base address */
extern volatile USBD_INFO_T usbdInfo;

/* USB Device Property */
extern USB_CMD_T	_usb_cmd_pkt;

UINT32 volatile u32Ready = 0;

#define BUFFER_SIZE	0x80000

typedef struct {
    unsigned Reserved0:1 ;
    unsigned Reserved1:1 ;
    unsigned Reserved2:1 ;
    unsigned NotError:1 ;
    unsigned Select:1 ;
    unsigned PaperEmpty:1 ;
    unsigned Reserved6:1 ;
    unsigned Reserved7:1 ;
}PTR_STATUS;

PTR_STATUS ptr_status;

UINT8 *pu8ptr_status;

UINT32 g_u32RxPoint = 0, g_u32TxPoint = 0;
UINT32 g_u32DataCount = 0;

#if defined (__GNUC__)
unsigned char buffer[BUFFER_SIZE + 512] __attribute__((aligned(64)));
unsigned char buffer_tmp[512] __attribute__((aligned(64)));
#else
__align(64) unsigned char buffer[BUFFER_SIZE + 512];
__align(64) unsigned char buffer_tmp[512];
#endif

/* PTR Descriptor */
#if defined (__GNUC__)
UINT8 PTR_DeviceDescriptor[] __attribute__((aligned(4))) =
#else
__align(4) UINT8 PTR_DeviceDescriptor[] =
#endif
{
    0x12,                /* bLength */
    DESC_DEVICE,         /* bdescriptorType */
    0x00,                /* bcdUSB version */
    0x02,                /* bcdUSB version */
    0x00,                /* bDeviceClass - Specified by interface */
    0x00,                /* bDeviceSubClass - Specified by interface */
    0x00,                /* bDeviceProtocol - Specified by interface */
    0x40,                /* bMaxPacketSize for EP0 - max = 64*/
    (VENDOR_ID & 0xFF),  /* idVendor */
    (VENDOR_ID >> 8),    /* idVendor */
    (PRODUCT_ID & 0xFF), /* idProduct */
    (PRODUCT_ID >> 8),   /* idProduct */
    (BCD_DEVICE & 0xFF), /* bcdDevice */
    (BCD_DEVICE >> 8),   /* bcdDevice */
    0x01,                /* iManufacturer - index of string*/
    0x02,                /* iProduct - index of string*/
    0x00,                /* iSerialNumber - index of string*/
    0x01                 /* bNumConfigurations */
};

#if defined (__GNUC__)
static UINT8 PTR_ConfigurationBlock[] __attribute__((aligned(4))) =
#else
__align(4) static UINT8 PTR_ConfigurationBlock[] =
#endif
{
    0x09,                /* bLength */
    DESC_CONFIG,         /* bDescriptortype = configuration*/
    0x19, 0x00,          /* wTotalLength of all descriptors */
    0x01,                /* bNumInterfaces */
    0x01,                /* bConfigurationValue */
    0x03,                /* iConfiguration - index of string*/
    0x40,                /* bmAttributes - Self powered*/
    0xFA,                /* bMaxPower - 500mA */
	
    0x09,                /* bLength */
    DESC_INTERFACE,      /* bDescriptorType */
    0x00,                /* bInterfacecNumber */
    0x00,                /* bAlternateSetting */
    0x01,                /* bNumEndpoints */
    PRINTER_CLASS,       /* bInterfaceClass */
    PRINTER_SUBCLASS,    /* bInterfaceSubClass */
    PRINTER_PROTOCOL,    /* bInterfaceProtocol*/
    0x00,                /* iInterface */
	
    0x07,                /* bLength */
    DESC_ENDPOINT,       /* bDescriptorType */
    0x01,                /* bEndpointAddress - EP1, OUT*/
    EP_BULK,             /* bmAttributes */
    0x00,                /* wMaxPacketSize - Low */
    0x02,                /* wMaxPacketSize - High */
    0x00,                /* bInterval */
};


#if defined (__GNUC__)
static UINT8 PTR_FullConfigurationBlock[] __attribute__((aligned(4))) =
#else
__align(4) static UINT8 PTR_FullConfigurationBlock[] =
#endif
{
    0x09,                /* bLength */
    DESC_CONFIG,         /* bDescriptortype = configuration*/
    0x19, 0x00,          /* wTotalLength of all descriptors */
    0x01,                /* bNumInterfaces */
    0x01,                /* bConfigurationValue */
    0x03,                /* iConfiguration - index of string*/
    0x40,                /* bmAttributes - Self powered*/
    0xFA,                /* bMaxPower - 500mA */
	
    0x09,                /* bLength */
    DESC_INTERFACE,      /* bDescriptorType */
    0x00,                /* bInterfacecNumber */
    0x00,                /* bAlternateSetting */
    0x01,                /* bNumEndpoints */
    PRINTER_CLASS,       /* bInterfaceClass */
    PRINTER_SUBCLASS,    /* bInterfaceSubClass */
    PRINTER_PROTOCOL,    /* bInterfaceProtocol*/
    0x00,                /* iInterface */
	
    0x07,                /* bLength */
    DESC_ENDPOINT,       /* bDescriptorType */
    0x01,                /* bEndpointAddress - EP1, OUT*/
    EP_BULK,             /* bmAttributes */
    0x40,                /* wMaxPacketSize - Low */
    0x00,                /* wMaxPacketSize - High */
    0x00,                /* bInterval */
};


/* Identifier Language */
#if defined (__GNUC__)
static UINT8 PTR_StringDescriptor0[4] __attribute__((aligned(4))) =
#else
__align(4) static UINT8 PTR_StringDescriptor0[4] = 
#endif
{
    4,				      // bLength
    USB_DT_STRING,	// bDescriptorType
    0x09, 0x04	
};

/* iManufacturer */
#if defined (__GNUC__)
UINT8 PTR_StringDescriptor1[] __attribute__((aligned(4))) =
#else
__align(4) UINT8 PTR_StringDescriptor1[] = 
#endif
{
    0x10,				 	/* bLength (Dafault Value is 0x10, the value will be set to actual value according to the Descriptor size wehn calling ptrInit) */
    0x03,					/* bDescriptorType */
    'n', 0, 'u', 0,	'v', 0, 'o', 0,	'T', 0, 'o', 0,	'n', 0
};

/* iProduct */
#if defined (__GNUC__)
UINT8 PTR_StringDescriptor2[] __attribute__((aligned(4))) =
#else
__align(4) UINT8 PTR_StringDescriptor2[] = 
#endif
{
    0x10,				 	/* bLength (Dafault Value is 0x10, the value will be set to actual value according to the Descriptor size wehn calling ptrInit) */
    0x03,					/* bDescriptorType */
    'U', 0, 'S', 0,	'B', 0, ' ', 0,	'P', 0, 'T', 0,	'R', 0
};

/* iSerialNumber */
#if defined (__GNUC__)
UINT8 PTR_StringDescriptor3[] __attribute__((aligned(4))) =
#else
__align(4) UINT8 PTR_StringDescriptor3[] = 
#endif
{
    0x1A,				 	/* bLength (Dafault Value is 0x1A, the value will be set to actual value according to the Descriptor size wehn calling ptrInit) */
    0x03,					/* bDescriptorType */
    '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, 'P', 0, 'T', 0, 'R', 0, '0', 0, '0', 0, '1', 0	
};

char pnp_string [] = "XXMFG:Nuvoton;MDL:PTR-2024;CLS:PRINTER";

/* PTR High Speed Init */
void ptrHighSpeedInit(void)
{
    usbdInfo.usbdMaxPacketSize = 0x200;
    /* bulk in */
    outp32(EPA_RSP_SC, 0x00000000);		// auto validation
    outp32(EPA_MPS, EPA_MAX_PKT_SIZE);		// mps 8
    outp32(EPA_CFG, 0x00000013);		// bulk out ep no 1
    outp32(EPA_START_ADDR, CEP_MAX_PKT_SIZE);
    outp32(EPA_END_ADDR, CEP_MAX_PKT_SIZE + EPA_MAX_PKT_SIZE - 1);
    outp32(EPA_IRQ_ENB, 0x00000010);    /* data pkt received */
}

/* PTR Full Speed Init */
void ptrFullSpeedInit(void)
{
    usbdInfo.usbdMaxPacketSize = 0x40;
    /* bulk in */
    outp32(EPA_RSP_SC, 0x00000000);		// auto validation
    outp32(EPA_MPS, EPA_OTHER_MAX_PKT_SIZE);		// mps 8
    outp32(EPA_CFG, 0x00000013);		// bulk out ep no 1
    outp32(EPA_START_ADDR, CEP_MAX_PKT_SIZE);
    outp32(EPA_END_ADDR, CEP_MAX_PKT_SIZE + EPA_OTHER_MAX_PKT_SIZE - 1);
    outp32(EPA_IRQ_ENB, 0x00000010);    /* data pkt received */
}
	
void ptrClassOUT(void)
{	
    if(_usb_cmd_pkt.bRequest == PTR_SOFT_RESET)
    {
        sysprintf(" * PTR SOFT RESET\n");
    }
} 	


void ptrClassIN(void)
{	
	  UINT32 value, i;
    if(_usb_cmd_pkt.bRequest == PTR_GET_DEVICE_ID)
    {
        sysprintf(" * PTR GET DEVICE ID\n");
		  	value = sizeof(pnp_string) - 1;
			  pnp_string[0] = (value >> 8) & 0xFF;
			  pnp_string[1] = value & 0xFF;
			  for(i=0;i<value;i++)
          outp8(CEP_DATA_BUF, pnp_string[i]);
        outp32(IN_TRNSFR_CNT, value);			
    }
    else if(_usb_cmd_pkt.bRequest == PTR_GET_PORT_STATUS)
    {
			  /*
			     7..6 Reserved Reserved for future use; device shall return these bits reset to '0'
              5 Paper Empty 1 = Paper Empty, 0 = Paper Not Empty
              4 Select 1 = Selected, 0 = Not Selected
              3 Not Error 1 = No Error, 0 = Error
           2..0 Reserved Reserved for future use; device shall return these bits reset to '0'
        */
			  ptr_status.NotError = 1;
			  ptr_status.PaperEmpty = 1;
        sysprintf(" * PTR GET PORT STATUS - 0x%X\n",*pu8ptr_status);
        sysprintf("   - Paper Empty = %d (%s)\n", ptr_status.PaperEmpty, ptr_status.PaperEmpty?"Paper Empty":"Paper Not Empty");
        sysprintf("   - Select = %d (%s)\n", ptr_status.Select, ptr_status.Select?"Selected":"Not Selected");
        sysprintf("   - Not Error = %d (%s)\n", ptr_status.NotError, ptr_status.NotError?"No Error":"Error");
        outp8(CEP_DATA_BUF, *pu8ptr_status);
        outp32(IN_TRNSFR_CNT, 1);			
    }
} 	
void EPA_Handler(UINT32 u32IntEn,UINT32 u32IntStatus)  /* Interrupt IN handler */
{
    /* Receive data from HOST */
    if(u32IntStatus & (DATA_RxED_IS | SHORT_PKT_IS))
    {
        UINT32 len;
		
        len = inp32(EPA_DATA_CNT) & 0xffff;                  /* Get data from Endpoint FIFO */   
					
        outp32(DMA_CTRL_STS, 0x01);                          /* Config DMA - Bulk OUT, Write data from Endpoint Buufer to DRAM, EP2 */
	   	
        if((UINT32)&buffer[g_u32RxPoint] & 0x3)                   /* Address for DMA must be word-aligned */
        {
            outp32(AHB_DMA_ADDR, (UINT32)&buffer_tmp[0]);       /* Using word-aligned for DMA */
        }
        else		
            outp32(AHB_DMA_ADDR, (UINT32)&buffer[g_u32RxPoint]);	   	
	   	
        outp32(DMA_CNT, len);                                /* DMA length */
		
        outp32(DMA_CTRL_STS, inp32(DMA_CTRL_STS)|DMA_EN);    /* Trigger DMA */
		
        while(inp32(DMA_CTRL_STS) & DMA_EN);                 /* Wait DMA Ready */

        if((UINT32)&buffer[g_u32RxPoint] & 0x3)
        {
            memcpy(&buffer[g_u32RxPoint], buffer_tmp, len);
        }
			
        g_u32RxPoint += len;
		
        g_u32DataCount += len;
		
        if(g_u32RxPoint >= BUFFER_SIZE)                        /* Check DRAM Buffer - Buffer size is BUFFER_SIZE + EPB Maximum Packet Size(512) */
            g_u32RxPoint = 0;
		
//        sysprintf(" + Rx %3d - RxPoint %8d Data %8d\n",len, g_u32RxPoint, g_u32DataCount);
    }
}

	
/* PTR Init */
void ptrInit(void)
{
    /* Set Endpoint map */
    usbdInfo.i32EPA_Num = 1;	/* Endpoint 1 */
    usbdInfo.i32EPB_Num = -1;	/* Not use */
    usbdInfo.i32EPC_Num = -1;	/* Not use */
    usbdInfo.i32EPD_Num = -1;	/* Not use */
	
    /* Set Callback Function */
    /* Set PTR initialize function */
    usbdInfo.pfnFullSpeedInit = ptrFullSpeedInit;
    usbdInfo.pfnHighSpeedInit = ptrHighSpeedInit;
	
    usbdInfo.pfnEPACallBack = EPA_Handler;
	
    /* Set Descriptor pointer */
    usbdInfo.pu32DevDescriptor = (PUINT32) &PTR_DeviceDescriptor;
    usbdInfo.pu32HSConfDescriptor = (PUINT32) &PTR_ConfigurationBlock;
    usbdInfo.pu32FSConfDescriptor = (PUINT32) &PTR_FullConfigurationBlock;
	
    usbdInfo.pfnClassDataOUTCallBack = ptrClassOUT;	
    usbdInfo.pfnClassDataINCallBack = ptrClassIN;	
	
    usbdInfo.pu32StringDescriptor[0] = (PUINT32) &PTR_StringDescriptor0;
    usbdInfo.pu32StringDescriptor[1] = (PUINT32) &PTR_StringDescriptor1;
    usbdInfo.pu32StringDescriptor[2] = (PUINT32) &PTR_StringDescriptor2;
    usbdInfo.pu32StringDescriptor[3] = (PUINT32) &PTR_StringDescriptor3;
	
    /* Set Descriptor length */	
    usbdInfo.u32DevDescriptorLen =  LEN_DEVICE;
    usbdInfo.u32HSConfDescriptorLen =  sizeof(PTR_ConfigurationBlock);
    usbdInfo.u32FSConfDescriptorLen =  sizeof(PTR_FullConfigurationBlock);	
    usbdInfo.u32StringDescriptorLen[0] =  PTR_StringDescriptor0[0] = sizeof(PTR_StringDescriptor0);
    usbdInfo.u32StringDescriptorLen[1] = PTR_StringDescriptor1[0] = sizeof(PTR_StringDescriptor1);
    usbdInfo.u32StringDescriptorLen[2] = PTR_StringDescriptor2[0] = sizeof(PTR_StringDescriptor2);
    usbdInfo.u32StringDescriptorLen[3] = PTR_StringDescriptor3[0] = sizeof(PTR_StringDescriptor3);		
	
    pu8ptr_status = (UINT8 *)&ptr_status;
}

void print_message(void)
{
    sysDisableInterrupt(IRQ_UDC); 
	  if(g_u32DataCount)
		{
			  UINT32 u32i, len;
			
			  if(g_u32DataCount > PRINT_SIZE)
            len = PRINT_SIZE;
				else
					  len = g_u32DataCount;
				
        sysprintf("\n================= Message Start =================\n");		
				
			  for(u32i=0;u32i < len;u32i++)
            sysprintf("%c", buffer[g_u32TxPoint++]);
				
				g_u32DataCount -= len;
				
        if(g_u32TxPoint >= BUFFER_SIZE)                       
            g_u32TxPoint = 0;  
				
        sysprintf("\n================= Message  End  =================\n");
        sysprintf(" - Print Message - RxPoint %8d TxPoint %8d Data %8d\n", g_u32RxPoint, g_u32TxPoint, g_u32DataCount);
    }
    sysEnableInterrupt(IRQ_UDC);		
}
