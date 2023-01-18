/**************************************************************************//**
 * @file     N9H20_USBD.h
 * @version  V3.00
 * @brief    N9H20 series USB Device driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#ifndef _N9H20_USBD_H_
#define _N9H20_USBD_H_

/*
 * Standard requests
 */
#define USBR_GET_STATUS        0x00
#define USBR_CLEAR_FEATURE     0x01
#define USBR_SET_FEATURE       0x03
#define USBR_SET_ADDRESS       0x05
#define USBR_GET_DESCRIPTOR    0x06
#define USBR_SET_DESCRIPTOR    0x07
#define USBR_GET_CONFIGURATION 0x08
#define USBR_SET_CONFIGURATION 0x09
#define USBR_GET_INTERFACE     0x0A
#define USBR_SET_INTERFACE     0x0B
#define USBR_SYNCH_FRAME       0x0C

/*
 * Descriptor types
 */
#define USB_DT_DEVICE          0x01
#define USB_DT_CONFIG          0x02
#define USB_DT_STRING          0x03
#define USB_DT_INTERFACE       0x04
#define USB_DT_ENDPOINT        0x05
#define USB_DT_QUALIFIER       0x06
#define USB_DT_OSCONFIG        0x07
#define USB_DT_IFPOWER         0x08

#define USBD_DT_HID            0x21
#define USBD_DT_HID_RPT        0x22

#define EP_INPUT               0x80
#define EP_OUTPUT              0x00

/* USB Endpoint Type */
#define EP_ISO                 0x01
#define EP_BULK                0x02
#define EP_INT                 0x03
#define DEVICE_REMOTE_WAKEUP      1
#define ENDPOINT_HALT             0
#define TEST_MODE                 2

/* USB TEST MODES */
#define TEST_J                 0x01
#define TEST_K                 0x02
#define TEST_SE0_NAK           0x03
#define TEST_PACKET            0x04
#define TEST_FORCE_ENABLE      0x05

/* Driver definition of tokens */
#define OUT_TOK                0x00
#define IN_TOK                 0x01
#define SUP_TOK                0x02
#define PING_TOK               0x03
#define NO_TOK                 0x04

//Bit Definitions of IRQ_ENB/STAT register
#define IRQ_USB_STAT           0x01
#define IRQ_CEP                0x02
#define IRQ_NCEP               0xfc

/* Definition of Bits in USB_IRQ_STS register */
#define USB_SOF                0x01
#define USB_RST_STS            0x02
#define USB_RESUME             0x04
#define USB_SUS_REQ            0x08
#define USB_HS_SETTLE          0x10
#define USB_DMA_REQ            0x20
#define USABLE_CLK             0x40
#define USB_VBUS              0x100

/* Definition of Bits in USB_OPER register */
#define USB_GEN_RES             0x1
#define USB_HS                  0x2
#define USB_CUR_SPD_HS          0x4

/* Definition of Bits in CEP_IRQ_STS register */
#define CEP_SUPTOK           0x0001
#define CEP_SUPPKT           0x0002
#define CEP_OUT_TOK          0x0004
#define CEP_IN_TOK           0x0008
#define CEP_PING_TOK         0x0010
#define CEP_DATA_TXD         0x0020
#define CEP_DATA_RXD         0x0040
#define CEP_NAK_SENT         0x0080
#define CEP_STALL_SENT       0x0100
#define CEP_USB_ERR          0x0200
#define CEP_STS_END          0x0400
#define CEP_BUFF_FULL        0x0800
#define CEP_BUFF_EMPTY       0x1000

/* Definition of Bits in CEP_CTRL_STS register */
#define CEP_NAK_CLEAR          0x00
#define CEP_SEND_STALL         0x02

/* Definition of Bits in EP_IRQ_STS register */
#define EP_BUFF_FULL          0x001
#define EP_BUFF_EMPTY         0x002
#define EP_SHORT_PKT          0x004
#define EP_DATA_TXD           0x008
#define EP_DATA_RXD           0x010
#define EP_OUT_TOK            0x020
#define EP_IN_TOK             0x040
#define EP_PING_TOK           0x080
#define EP_NAK_SENT           0x100
#define EP_STALL_SENT         0x200
#define EP_USB_ERR            0x800

/* Bit Definitons of EP_RSP_SC Register */
#define EP_BUFF_FLUSH          0x01
#define EP_MODE                0x06
#define EP_MODE_AUTO           0x01
#define EP_MODE_MAN            0x02
#define EP_MODE_FLY            0x03
#define EP_TOGGLE              0x08
#define EP_HALT                0x10
#define EP_ZERO_IN             0x20
#define EP_PKT_END             0x40

/* Define Endpoint feature */
#define Ep_In        0x01
#define Ep_Out       0x00
#define Ep_Bulk      0x01
#define Ep_Int       0x02
#define Ep_Iso       0x03
#define EP_A         0x00
#define EP_B         0x01
#define EP_C         0x02
#define EP_D         0x03

typedef struct usb_cmd
{
  UINT8  bmRequestType;
  UINT8  bRequest;
  UINT16  wValue;
  UINT16  wIndex;
  UINT16  wLength;
} USB_CMD_T;


typedef void (*PFN_USBD_CALLBACK)(void);
typedef BOOL (PFN_USBD_EXIT_CALLBACK)(void);
typedef void (*PFN_USBD_EP_CALLBACK)(UINT32 u32IntEn,UINT32 u32IntStatus);

typedef struct  __attribute__((__packed__)) {
/* Descriptor pointer */
    PUINT32 pu32DevDescriptor;
    PUINT32 pu32QulDescriptor;
    PUINT32 pu32HSConfDescriptor;
    PUINT32 pu32FSConfDescriptor;
    PUINT32 pu32HOSConfDescriptor;
    PUINT32 pu32FOSConfDescriptor;
    PUINT32 pu32HIDDescriptor;
	PUINT32 pu32HIDRPTDescriptor[8];		
	PUINT32 pu32StringDescriptor[8];

/* Descriptor length */
    UINT32  u32DevDescriptorLen;
    UINT32  u32QulDescriptorLen;
    UINT32  u32HSConfDescriptorLen;
    UINT32  u32FSConfDescriptorLen;
    UINT32  u32HOSConfDescriptorLen;
    UINT32  u32FOSConfDescriptorLen;
    UINT32  u32HIDDescriptorLen;
	UINT32	u32HIDRPTDescriptorLen[8];		
	UINT32 	u32StringDescriptorLen[8];

/* USBD Init */
    PFN_USBD_CALLBACK pfnHighSpeedInit;
    PFN_USBD_CALLBACK pfnFullSpeedInit;

/* Endpoint Number */
    INT32  i32EPA_Num;
    INT32  i32EPB_Num;
    INT32  i32EPC_Num;
    INT32  i32EPD_Num;

/* Endpoint Call Back */
    PFN_USBD_EP_CALLBACK pfnEPACallBack;
    PFN_USBD_EP_CALLBACK pfnEPBCallBack;
    PFN_USBD_EP_CALLBACK pfnEPCCallBack;
    PFN_USBD_EP_CALLBACK pfnEPDCallBack;

/* Class Call Back */
    PFN_USBD_CALLBACK pfnClassDataINCallBack;
    PFN_USBD_CALLBACK pfnClassDataOUTCallBack;
    PFN_USBD_CALLBACK pfnDMACompletion;
    PFN_USBD_CALLBACK pfnReset;
    PFN_USBD_CALLBACK pfnSOF;
    PFN_USBD_CALLBACK pfnPlug;
    PFN_USBD_CALLBACK pfnUnplug;

/* Transfer Control */
    UINT32  _usbd_remlen;
    BOOL  _usbd_remlen_flag;
    UINT32  *_usbd_ptr;

/* Command Status Control */
    UINT8  GET_DEV_Flag;
    UINT8  GET_CFG_Flag;
    UINT8  GET_QUL_Flag;
    UINT8  GET_OSCFG_Flag;
    UINT8  GET_STR_Flag;
    UINT8  GET_VEN_Flag;
    UINT8  GET_VENO_Flag;
    UINT8  GET_HID_Flag;
    UINT8  GET_HID_RPT_Flag;
    UINT8  CLASS_CMD_Flag;
    UINT8  CLASS_CMD_Iflag;
    UINT8  CLASS_CMD_Oflag;

    UINT32  usbdMaxPacketSize;

/* DMA Status flag */
    UINT8  _usbd_DMA_Flag;
    UINT8  _usbd_DMA_In;

/* Status flag */	
    UINT8  _usbd_resume;
    UINT8  USBModeFlag;   
    UINT32  _usbd_devstate;
    UINT32  _usbd_address;
    UINT32  _usbd_speedset;
    UINT16  _usbd_confsel;
    UINT16  _usbd_intfsel;
    UINT16  _usbd_altsel;
    UINT32  _usbd_feature;
    INT32  _usbd_haltep;
    INT32  _usbd_unhaltep;
    INT32  remotewakeup;
    INT32  testmode;
    INT32  enableremotewakeup;
    INT32  enabletestmode;
    INT32  disableremotewakeup;
    INT32  disabletestmode;
    INT32  testselector;
    UINT32  usbdGetStatusData;
    UINT8  usbdGetConfig;
    UINT8  usbdGetInterface;
    UINT8  usbdGetStatus;

/* for isochronous */
    UINT32  AlternateFlag;
    UINT32  u32VbusStatus;
/* for Isochronous */
    UINT32  AlternateFlag_Audio;
    UINT32  u32UVC;
    UINT32  USBStartFlag;

}USBD_INFO_T;

typedef struct __attribute__((__packed__)) {
    UINT32  appConnected;
    UINT32  usbConnected;
    UINT32  appConnected_Audio;
}USBD_STATUS_T;


typedef struct
{
    UINT8 EP_Num;
    UINT8 EP_Dir;
    UINT8 EP_Type;
} USB_EP_Inf_T;


VOID udcOpen(void);
VOID udcClose(void);
VOID udcInit(void);
VOID udcDeinit(void);
BOOL udcIsAttached(void);
BOOL udcIsAttachedToHost(void);
VOID udcSetSupendCallBack(PFN_USBD_CALLBACK pfun);

#endif

