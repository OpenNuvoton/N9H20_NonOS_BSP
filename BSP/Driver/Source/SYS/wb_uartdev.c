/**************************************************************************//**
 * @file     wb_uartdev.c
 * @version  V3.00
 * @brief    N9H20 series SYS driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include "wblib.h"
extern UARTDEV_T nvt_uart0;
extern UARTDEV_T nvt_uart1;
INT32 register_uart_device(UINT32 u32port, UARTDEV_T* pUartDev)
{
	if(u32port==0)
		*pUartDev = nvt_uart0;
	else if(u32port==1)
		*pUartDev = nvt_uart1;
	else 	
		return -1;
	return Successful;	
}
