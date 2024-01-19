/**************************************************************************//**
 * @file     HID.h
 * @brief    HID Class Device sample header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#ifndef __HIDSYS_H__
#define __HIDSYS_H__

#ifdef  __cplusplus
extern "C"
{
#endif
  
/* Define the vendor id and product id */
#define VENDOR_ID    0x0416
#define PRODUCT_ID   0xB001
	
#define BCD_DEVICE   0x0200

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

#define PRINTER_CLASS                  0x07
#define PRINTER_SUBCLASS               0x01
#define PRINTER_PROTOCOL               0x02

//***************************************************
// 		PTR Class REQUEST
//***************************************************
#define PTR_GET_DEVICE_ID              0x0
#define PTR_GET_PORT_STATUS            0x1
#define PTR_SOFT_RESET                 0x2 

/* Define EP maximum packet size */
#define CEP_MAX_PKT_SIZE        64
#define CEP_OTHER_MAX_PKT_SIZE  64
#define EPA_MAX_PKT_SIZE        512
#define EPA_OTHER_MAX_PKT_SIZE  64

#define PRINT_SIZE 512

void ptrInit(void);
void print_message(void);

#ifdef  __cplusplus
}
#endif

#endif // #ifndef __HIDSYS_H__


