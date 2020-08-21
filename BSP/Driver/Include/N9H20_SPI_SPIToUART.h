/**************************************************************************//**
 * @file     N9H20_SPI_SPIToUART.h
 * @version  V3.00
 * @brief    N9H20 series SPI to UART driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#ifndef __SPI_SPITOUART_H__
#define __SPI_SPITOUART_H__

#define UART_PORT0 0
#define UART_PORT1 1
#define UART_ALL 0xff
#define RST_SPI 0xfe
#define CLOSE_SPI 0xfe



#define VUART_TX_EMPTY(port) (port==0?(vu_GetNotification()&0x2?1:0): port==1? (vu_GetNotification()&0x8?1:0): ((vu_GetNotification()&0x8|vu_GetNotification()&0x2)?1:0))
#define VUART_RX_INT(port)   (port==0?(vu_GetNotification()&0x1?1:0): port==1? (vu_GetNotification()&0x4?1:0): ((vu_GetNotification()&0x1|vu_GetNotification()&0x4)?1:0))




void vu_OpenUART(UINT8 UART_port);
void vu_ResetUART(UINT8 UART_port);
void vu_SetBaudRate(UINT8 UART_port, UINT32 baudrate);

int vu_UARTWrite(UINT8 UART_port, unsigned char *pSrc, INT32 len);
int vu_UARTRead (UINT8 UART_port,INT32 Max,unsigned char *pDst);

int vu_GetRXAvailLen(UINT8 UART_port);
int vu_GetTXFreeLen(UINT8 UART_port);


char vu_GetNotification(void);
void vu_CloseUART(UINT8 UART_port);


void vu_ClearBuf(UINT8 UART_port);
int vu_GetStatus(void);

void _vu_UARTReadMax(UINT8 UART_port,INT32 UART0_Max,INT32 UART1_Max,UINT8 *pDst,UINT8 *pDst2);
int _vu_UARTWrite(UINT8 UART_port,unsigned char *pSrc,  unsigned char *pSrc2 ,INT32 UART0_len, INT32 UART1_len);


#endif //__SPI_SPITOUART_H__
