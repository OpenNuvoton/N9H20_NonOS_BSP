/**************************************************************************//**
 * @file     wberrcode.h
 * @version  V3.00
 * @brief    N9H20 series SYS driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

/****************************************************************************
 *
 * FILENAME
 *     wberrcode.h
 *
 * VERSION
 *     3.0
 *
 * DESCRIPTION
 *	   The Error Codes returned by Nuvoton S/W library are defined as the following
 *     format:
 *              0xFFFFXX## (XX : Module ID, ##:Error Number)
 *
 *     The Module IDs for each S/W library are defined in this file. The error
 *     numbers are defined by S/W library according to its requirement.
 *
 *
 * DATA STRUCTURES
 *     None
 *
 * FUNCTIONS
 *     None
 *
 * HISTORY
 *     2009-02-26  Ver 1.0 draft by Min-Nan Cheng
 * REMARK
 *     None
 **************************************************************************/
#ifndef _WBERRCODE_H_
#define _WBERRCODE_H_

/* Error Code's Module ID */
#define FMI_ERR_ID			0xFFFF0100	/* FMI library ID */
#define APU_ERR_ID			0xFFFF0200	/* Audio Processing library ID */
#define USB_ERR_ID			0xFFFF0300	/* USB Device library ID */
#define GDMA_ERR_ID			0xFFFF0400	/* GDMA library ID */
#define JPG_ERR_ID			0xFFFF0500	/* ATA library ID */
#define DMAC_ERR_ID			0xFFFF0600	/* DMA library ID */
#define TMR_ERR_ID			0xFFFF0700	/* Timer library ID */
#define GE_ERR_ID			0xFFFF0800	/* 2D graphics library ID */
#define AIC_ERR_ID			0xFFFF0900	/* AIC library ID */
#define SYSLIB_ERR_ID			0xFFFF0A00	/* System library ID */
#define USBO_ERR_ID			0xFFFF0C00	/* OHCI library ID */
#define USBH_ERR_ID			0xFFFF0D00	/* USB Host library ID */
#define RTC_ERR_ID			0xFFFF0E00	/* RTC library ID */
#define GPIO_ERR_ID			0xFFFF0F00	/* GPIO library ID */
#define VIN_ERR_ID			0xFFFF1000	/* Video-In library ID */
#define I2C_ERR_ID			0xFFFF1100	/* I2C library ID */
#define SPI_ERR_ID			0xFFFF1200	/* SPI library ID */
#define PWM_ERR_ID			0xFFFF1300	/* PWM library ID */
#define	BLT_ERR_ID			0xFFFF1500	/* BLT library ID */
#define UART_ERR_ID			0xFFFF1700	/* UART library ID */
#define LCD_ERR_ID			0xFFFF1800	/* LCD library ID */
#define ADC_ERR_ID			0xFFFF1A00	/* ADC library ID */

#define FAT_ERR_ID			0xFFFF8200	/* FAT file system library ID */


/* Macros for handing error code */
#define NVTAPI_RESULT_IS_ERROR(value)	((value) < 0) ? 1 : 0		/* 1:error, 0:non-err */
#define NVTAPI_GET_ERROR_ID(value)	((value) & 0x0000FF00) >> 8     /* get Module ID of error code */
#define NVTAPI_GET_ERROR_NUMBER(value)	((value) & 0x000000FF) 		/* get Error Number of error code */

/* Error Number defined by XXX library */

#endif /* _WBERRCODE_H */
