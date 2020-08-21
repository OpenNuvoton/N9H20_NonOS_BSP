/**************************************************************************//**
 * @file     N9H20_I2C.h
 * @version  V3.00
 * @brief    N9H20 series I2C driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
 
#ifndef _N9H20_I2C_H_
#define _N9H20_I2C_H_

#include "wbtypes.h"
#include "N9H20_reg.h"
#include "DrvI2CH.h"

/*-----------------------------------------*/
/* marco, type and constant definitions    */
/*-----------------------------------------*/
#define I2C_NUMBER				2
#define I2C_MAX_BUF_LEN			450

#define I2C_INPUT_CLOCK			33000           /* 33 000 KHz */

/*-----------------------------------------*/
/* global interface variables declarations */
/*-----------------------------------------*/
/* 
	bit map in CMDR 
*/
#define I2C_CMD_START			0x10
#define I2C_CMD_STOP			0x08
#define I2C_CMD_READ			0x04
#define I2C_CMD_WRITE			0x02
#define I2C_CMD_NACK			0x01

/* 
	for transfer use 
*/
#define I2C_WRITE				0x00
#define I2C_READ				0x01

#define I2C_STATE_NOP			0x00
#define I2C_STATE_READ			0x01
#define I2C_STATE_WRITE			0x02
#define I2C_STATE_PROBE			0x03

/*
	ioctl commands 
*/
#define I2C_IOC_SET_DEV_ADDRESS		0
#define I2C_IOC_SET_SUB_ADDRESS		1
#define I2C_IOC_SET_SPEED			2
//#define I2C_IOC_GET_LAST_ERROR		3

/* 
	error code 
*/
#define I2C_ERR_ID					0xFFFF1100	         /* I2C library ID                 */
#define I2C_ERR_NOERROR				(0x00)
#define I2C_ERR_LOSTARBITRATION		(0x01 | I2C_ERR_ID)
#define I2C_ERR_BUSBUSY				(0x02 | I2C_ERR_ID)
#define I2C_ERR_NACK				(0x03 | I2C_ERR_ID)	 /* data transfer error             */
#define I2C_ERR_SLAVENACK			(0x04 | I2C_ERR_ID)	 /* slave not respond after address */
#define I2C_ERR_NODEV				(0x05 | I2C_ERR_ID)
#define I2C_ERR_BUSY				(0x06 | I2C_ERR_ID)
#define I2C_ERR_IO					(0x07 | I2C_ERR_ID)
#define I2C_ERR_NOTTY				(0x08 | I2C_ERR_ID)

/* 
	i2c register offset 
*/
#define     I2C_CSR		(0x00)  /* Control and Status Register */
#define     I2C_DIVIDER	(0x04)  /* Clock Prescale Register */
#define     I2C_CMDR	(0x08)  /* Command Register */
#define     I2C_SWR		(0x0C)  /* Software Mode Control Register */
#define     I2C_RxR		(0x10)  /* Data Receive Register */
#define     I2C_TxR		(0x14)  /* Data Transmit Register */

/*-----------------------------------------*/
/* interface function declarations         */
/*-----------------------------------------*/
extern INT32 i2cInit(VOID);
extern INT32 i2cOpen(VOID);
extern INT32 i2cClose(VOID);
extern INT32 i2cRead(PUINT8 buf, UINT32 len);
extern INT32 i2cRead_OV(PUINT8 buf, UINT32 len);
extern INT32 i2cWrite(PUINT8 buf, UINT32 len);
extern INT32 i2cIoctl(UINT32 cmd, UINT32 arg0, UINT32 arg1);
extern INT32 i2cExit(VOID);



#endif
