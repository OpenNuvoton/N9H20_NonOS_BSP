/**************************************************************************//**
 * @file     spitouart.h
 * @version  V3.00
 * @brief    N9H20 series SPI to UART driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#ifndef __SPI0TUART_H__
#define __SPI0TUART_H__

#define LEAD_CODE 0
#define CHEACKSUM 1
#define LENGTH 2
#define COMMAND 3
#define PARAMETER 4
	
#define CMD_ResetUARTPort 0x0
#define CMD_ConfigureUARTPort 0x1
#define CMD_UARTHardwareFlowControl 0x2
#define CMD_UARTNotification 0x3
#define CMD_QueryUARTTXbuffer  0x4
#define CMD_SendDatatoUARTTXBuffer 0x5
#define	CMD_QueryUARTRXbuffer 0x6
#define CMD_ReceiveDatafromUARTRXBuffer 0x7
#define CMD_ReceiveDatafromUARTRXBuffer_Max 0x7 
#define CMD_GetNotification 0x8
#define CMD_CloseUARTPort 0x9
#define CMD_OpenUARTPort 0xA
#define CMD_ClearBuf 0xB

#define LENGTH_ResetUARTPort 0x2
#define LENGTH_UARTNotification 0x2
#define LENGTH_QueryUARTRXbuffer 0x2
#define LENGTH_ReceiveDatafromUARTRXBuffer_Max 0x4
#define LENGTH_ConfigureUARTPort 0x9
#define LENGTH_QueryUARTTXbuffer 0x2
#define LENGTH_SendDatatoUARTTXBuffer 0x2
#define LENGTH_CloseUARTPort 0x2
#define LENGTH_OpenUARTPort 0x2
#define LENGTH_ClearBuf 0x2


#define UART_PORT0 0
#define UART_PORT1 1
#define UART_ALL 0xff
#define UART_ALL_PORT0 2 
#define UART_ALL_PORT1 3

typedef struct VIRTUAL_UART_STRUCT
{
	INT32 count;
	UINT32 status;
}VU_CMD_STATUS;

void UARTNotification(char flag,VU_CMD_STATUS *vu);
void ResetUARTPort(UINT8 UART_port,VU_CMD_STATUS *vu);
void QueryUARTRXBuffer(UINT8 UART_port);
void ReceiveDatafromUARTRXBuffer_Max(UINT8 UART_port,UINT8 UART0_Max,UINT8 UART1_Max);
void ConfigureUARTPort(UINT8 UART_port,UINT32 baudrate,UINT32 datawidth,UINT8 parity,UINT8 stop,VU_CMD_STATUS *vu);
void QueryUARTTXBuffer(UINT8 UART_port,VU_CMD_STATUS *vu);
void SendDatatoUARTTXBuffer(UINT8 UART_port,UINT UART0_len,UINT UART1_len,VU_CMD_STATUS *vu);
void CloseUARTPort(UINT8 UART_port);

void OpenUARTPort(UINT8 UART_port,VU_CMD_STATUS *vu);
void ClearBuf(UINT8 UART_port);


UINT8 deleteq_ReadBuffer(int port);
int IsEmpty_ReadBuffer(int port);
int addq_WriteBuffer(int port,UINT8 data);
UINT8 deleteq_WriteBuffer(int port);
void clr_ReadBuffer(int port);
void clr_WriteBuffer(int port);


char GetNotification(void);
int ByteOfDataRxbuf(UINT8 port);
int AvailableByteTxbuf(UINT8 port);



void _mainbsy(int bsy);

int _FLAG_RESPONSE_PACKAGE(void);
int _sumofDataInReadBuffer0(int port);
int _UART_ALL_WRITE_BUFFER_NUM(void);
void _RSTCMD(void);


#endif // __SPIT0UART_H__
