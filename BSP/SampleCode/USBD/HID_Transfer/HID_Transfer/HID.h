/****************************************************************
 *                                                              *
 * Copyright (c) Nuvoton Technology Corp. All rights reserved.  *
 *                                                              *
 ****************************************************************/
#ifndef __HIDSYS_H__
#define __HIDSYS_H__

#ifdef  __cplusplus
extern "C"
{
#endif
  
/* Define the vendor id and product id */
#define USB_VID    0x0416
#define USB_PID    0x5020

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

/*!<USB Descriptor Length */
#define LEN_DEVICE          18
#define LEN_QUALIFIER       10
#define LEN_CONFIG          9
#define LEN_INTERFACE       9
#define LEN_ENDPOINT        7
#define LEN_OTG             5
#define LEN_HID             9

/* Define Descriptor information */
#define HID_DEFAULT_INT_IN_INTERVAL       1
#define USBD_SELF_POWERED                 0
#define USBD_REMOTE_WAKEUP                0
#define USBD_MAX_POWER                   50  /* The unit is in 2mA. ex: 50 * 2mA = 100mA */

#define LEN_CONFIG_AND_SUBORDINATE      (LEN_CONFIG+LEN_INTERFACE+LEN_HID+LEN_ENDPOINT*2)

/* Define the EP number */
#define INT_IN_EP_NUM           0x01
#define INT_OUT_EP_NUM          0x02

//***************************************************
//    HID Class REQUEST
//***************************************************
#define HID_GET_REPORT          0x01
#define HID_GET_IDLE            0x02
#define HID_GET_PROTOCOL        0x03
#define HID_SET_REPORT          0x09
#define HID_SET_IDLE            0x0A
#define HID_SET_PROTOCOL        0x0B
#define HID_RPT_TYPE_INPUT      0x01
#define HID_RPT_TYPE_OUTPUT     0x02
#define HID_RPT_TYPE_FEATURE    0x03

/* Define EP maximum packet size */
#define CEP_MAX_PKT_SIZE        64
#define CEP_OTHER_MAX_PKT_SIZE  64
#define EPA_MAX_PKT_SIZE        512
#define EPB_MAX_PKT_SIZE        512
#define EPA_OTHER_MAX_PKT_SIZE  64
#define EPB_OTHER_MAX_PKT_SIZE  64

#define EPA_BASE                CEP_MAX_PKT_SIZE
#define EPB_BASE                (EPA_BASE + EPA_MAX_PKT_SIZE)


#define EPA_OTHER_BASE          CEP_OTHER_MAX_PKT_SIZE
#define EPB_OTHER_BASE          (EPA_OTHER_BASE + EPA_OTHER_MAX_PKT_SIZE)

/*!<USB HID Descriptor Type */
#define DESC_HID                0x21
#define DESC_HID_RPT            0x22

#define BLOCK_SIZE       (64 * 1024)
void hidInit(void);
void HID_UpdateMouseData(void);
void HID_SetInReport(void);

#ifdef  __cplusplus
}
#endif

#endif // #ifndef __HIDSYS_H__


