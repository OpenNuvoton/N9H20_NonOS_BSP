/****************************************************************************
 * @file     HID.h
 * @version  V1.00
 * $Revision: 4 $
 * $Date: 18/04/25 11:43a $
 * @brief    HID Class Device sample header file
 *
 * @note
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/
 
#ifndef __HIDSYS_H__
#define __HIDSYS_H__

#ifdef  __cplusplus
extern "C"
{
#endif
  
#define FUN_MOUSE       2
#define FUN_KEYBOARD    1 

#ifdef HID_MOUSE  
	#define HID_FUNCTION    FUN_MOUSE
#endif
#ifdef HID_KEYBOARD
	#define HID_FUNCTION    FUN_KEYBOARD
#endif

/* Define the vendor id and product id */
#define USB_VID			0x0416
#define USB_PID			(0x9300 + HID_FUNCTION)

/*!<USB Descriptor Type */
#define DESC_DEVICE         0x01
#define DESC_CONFIG         0x02
#define DESC_STRING         0x03
#define DESC_INTERFACE      0x04
#define DESC_ENDPOINT       0x05
#define DESC_QUALIFIER      0x06
#define DESC_OTHERSPEED     0x07
#define DESC_IFPOWER        0x08
#define DESC_OTG            0x09
#define HID_STR0_DSCPT_LEN	0x04	

/*!<USB Descriptor Length */
#define LEN_DEVICE          18
#define LEN_QUALIFIER       10
#define LEN_CONFIG          9
#define LEN_INTERFACE       9
#define LEN_ENDPOINT        7
#define LEN_OTG             5
#define LEN_HID             9

/* Define Descriptor information */
#define HID_DEFAULT_INT_IN_INTERVAL     20
#define HID_IS_SELF_POWERED              0
#define HID_IS_REMOTE_WAKEUP             0
#define HID_MAX_POWER                  	50  /* The unit is in 2mA. ex: 50 * 2mA = 100mA */


//***************************************************
// 		HID Class REQUEST
//***************************************************
#define HID_GET_REPORT          0x01
#define HID_GET_IDLE            0x02
#define HID_GET_PROTOCOL        0x03
#define HID_SET_REPORT          0x09
#define HID_SET_IDLE            0x0A
#define HID_SET_PROTOCOL        0x0B
#define HID_RPT_TYPE_INPUT		0x01
#define HID_RPT_TYPE_OUTPUT		0x02
#define HID_RPT_TYPE_FEATURE	0x03

/* Define EP maximum packet size */
#define CEP_MAX_PKT_SIZE        64
#define CEP_OTHER_MAX_PKT_SIZE  64
#define EPA_MAX_PKT_SIZE        8
#define EPA_OTHER_MAX_PKT_SIZE  64

/*!<USB HID Descriptor Type */
#define DESC_HID            0x21
#define DESC_HID_RPT        0x22

typedef struct
{
	UINT16 u16VendorId;
	UINT16 u16ProductId;
	UINT8 *psVendorStringDesc;
	UINT8 *psProductStringDesc;

} HID_VENDOR_INFO;

void hidInit(void);
void HID_UpdateMouseData(void);
void HID_SetInReport(void);

#define ENTER_KEY			(0x4)
#define HOME_KEY			(0x20)
#define LEFT_KEY			(0x2)
#define RIGHT_KEY			(0x10)
#define UP_KEY				(0x1)
#define DOWN_KEY			(0x8)
#define MASK_KEY			(LEFT_KEY | RIGHT_KEY | UP_KEY | DOWN_KEY)

#define KEY_ADC_CHANNEL	2

#ifdef  __cplusplus
}
#endif

#endif // #ifndef __HIDSYS_H__


