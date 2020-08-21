/**************************************************************************//**
 * @file     virtualuart_api.c
 * @version  V3.00
 * @brief    N9H20 series virtual UART driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
/* Header files */

#include <stdio.h>
#include "wblib.h"
#include "spitouart.h"
#include "N9H20_SPI_SPIToUART.h"
#include "N9H20_SPI.h"


//#define DBG_PRINTF(...)
#define DBG_PRINTF		sysprintf
#define BUF_FULL 0
#define TEST_SIZE	0xa
#define SPI_SELECT 0


#define ERROR "ERROR"
#define PASS "PASS"
int tmp_i=0;
char GPIO_INT=0;
char LAST_CMD=0;

int tmp_stage=0;
volatile int tmp_j=0;
volatile int tmp_uart0_j=0;
volatile int tmp_uart1_j=0;
int FLAG_CLOSE_SPI=1;
volatile VU_CMD_STATUS vu[12];

void vu_OpenUART(UINT8 UART_port)
{
	

	int i=0;
	SPI_INFO_T spi0;
	//apb clock is 48MHz, output clock is 8MHz
	DBG_PRINTF("\nInitialize SPI...");
	if(FLAG_CLOSE_SPI==1)
	{
		spi0.nPort = 0;
		spi0.bIsSlaveMode = FALSE;
		spi0.bIsClockIdleHigh = FALSE;
		spi0.bIsLSBFirst = FALSE;
		spi0.bIsAutoSelect = FALSE;
		spi0.bIsActiveLow = TRUE;
		spi0.bIsTxNegative = FALSE;
		spiOpen(&spi0);
		spiSetClock(0,48,8000);
	}else
	{
		spiEnable(SPI_SELECT);
	}
	
	FLAG_CLOSE_SPI=0;
	for(i=0;i<10;i++)
	{
		vu[i].count=0;
		vu[i].status=0;
	}
	


	outpw(REG_GPDFUN, inpw(REG_GPDFUN) & 0xF3FFFFFF); //disable GPD13_CS0
	outpw(REG_GPAFUN, inpw(REG_GPAFUN) | 0x00000C00); //enable GPA5_SPI0_CS1_

	UARTNotification(1,vu);
	while(vu[CMD_UARTNotification].status==0xff);
	DBG_PRINTF("\nGPIO_INT_EABLE:OK\n");
	clr_ReadBuffer(UART_port);
	clr_WriteBuffer(UART_port);
	OpenUARTPort(UART_port,vu);
	while(vu[CMD_OpenUARTPort].status==0xff);
	DBG_PRINTF("\nOpenUARTPort(%d):OK\n",UART_port);
}


void vu_ResetUART (UINT8 UART_port)
{

	ResetUARTPort(UART_port,vu);
	while(vu[CMD_ResetUARTPort].status==0xff);
	if(UART_port==UART_PORT0 || UART_port==UART_PORT1){
		clr_ReadBuffer(UART_port);
		clr_WriteBuffer(UART_port);
	}else if(UART_port==UART_ALL){
		clr_ReadBuffer(0);
		clr_WriteBuffer(0);
		clr_ReadBuffer(1);
		clr_WriteBuffer(1);
	}else if(UART_port==RST_SPI){
		spiDisable(SPI_SELECT);
		spiEnable(SPI_SELECT);
	}
	DBG_PRINTF("ResetUARTport(%d):OK\n",UART_port);
	

}



int vu_UARTRead(UINT8 UART_port,INT32 Max,UINT8 *pDst)
{

	int i=0,j;
	unsigned char  tmp;
	if(Max<=0 || UART_port==UART_ALL){
		return 0;
	}else{
		//_mainbsy(1);
	}
	while(vu[CMD_ReceiveDatafromUARTRXBuffer_Max].status==0xff);
	for(j=0;j<Max;j++){
	
			tmp=deleteq_ReadBuffer(UART_port);
			//sysprintf("%d=%c\n",tmp_i,tmp);
			*(pDst+i)=tmp;
			i++;
		tmp_i++;
	}
	
	_mainbsy(0);
	return i;
}	
	
void vu_SetBaudRate(UINT8 UART_port, UINT32 baudrate)
{
	// (datawidth,parity,stop) are useless
	UINT8 datawidth,parity,stop; 
	datawidth=0;
	parity=0;
	stop=0;

	ConfigureUARTPort(UART_port,baudrate,datawidth,parity,stop,vu);
	while(vu[CMD_ConfigureUARTPort].status==0xff);
	DBG_PRINTF("SetBaudRate(%d,%d):OK\n",UART_port,baudrate);
}

int vu_UARTWrite(UINT8 UART_port, unsigned char *pSrc, INT32 len)
{
	
	int i=0;
	char tmp;

	tmp_j=0;	
	if(len==0)
	{
		return 0 ; 
	}
	else
	{
		_mainbsy(1);	
		QueryUARTTXBuffer(UART_port,vu);	
	}	
	while(len>0)
	{
		while((vu[CMD_QueryUARTTXbuffer].status==0xff || vu[CMD_SendDatatoUARTTXBuffer].status==0xff));
		if(UART_port!=UART_ALL){
			if(vu[CMD_QueryUARTTXbuffer].status==0x0 && vu[CMD_SendDatatoUARTTXBuffer].status==0x0)
			{
				tmp=(TEST_SIZE-vu[CMD_SendDatatoUARTTXBuffer].count); 
				if(len<=tmp)
				{
					if(vu[CMD_QueryUARTTXbuffer].count>=len)
					{ 
						tmp_stage=1;
						for(i=0;i<len;i++)
						{
							addq_WriteBuffer(UART_port,*(pSrc+tmp_j));
							tmp_j++;
						}
						SendDatatoUARTTXBuffer(UART_port,len,0,vu); 
						len=len-len;
						vu[CMD_QueryUARTTXbuffer].count=vu[CMD_QueryUARTTXbuffer].count-len;
					}
					else
					{ 
						tmp_stage=2;
						for(i=0;i<vu[CMD_QueryUARTTXbuffer].count;i++){
							addq_WriteBuffer(UART_port,*(pSrc+tmp_j));
							tmp_j++;
						}
						SendDatatoUARTTXBuffer(UART_port,vu[CMD_QueryUARTTXbuffer].count,0,vu); 
						len=len-vu[CMD_QueryUARTTXbuffer].count;
						vu[CMD_QueryUARTTXbuffer].count=vu[CMD_QueryUARTTXbuffer].count-vu[CMD_QueryUARTTXbuffer].count;
					}
				}
				else
				{
					if(vu[CMD_QueryUARTTXbuffer].count>tmp)
					{
						tmp_stage=3;
						for(i=0;i<tmp;i++){
							addq_WriteBuffer(UART_port,*(pSrc+tmp_j));
							tmp_j++;
						}
						SendDatatoUARTTXBuffer(UART_port,tmp,0,vu); 
						len=len-tmp;
						vu[CMD_QueryUARTTXbuffer].count=vu[CMD_QueryUARTTXbuffer].count-tmp;
					}
					else
					{
						tmp_stage=4;
						for(i=0;i<vu[CMD_QueryUARTTXbuffer].count;i++)
						{
							addq_WriteBuffer(UART_port,*(pSrc+tmp_j));
							tmp_j++;
						}
						SendDatatoUARTTXBuffer(UART_port,vu[CMD_QueryUARTTXbuffer].count,0,vu); 
						len=len-vu[CMD_QueryUARTTXbuffer].count;
						vu[CMD_QueryUARTTXbuffer].count=vu[CMD_QueryUARTTXbuffer].count-vu[CMD_QueryUARTTXbuffer].count;
					}
				}
				while((vu[CMD_QueryUARTTXbuffer].status==0xff || vu[CMD_SendDatatoUARTTXBuffer].status==0xff));
				if(len>0 && vu[CMD_QueryUARTTXbuffer].count<=0)QueryUARTTXBuffer(UART_port,vu);
			}
		}
	
	}	
	_mainbsy(0);		
	return 0;
}

void vu_ClearBuf(UINT8 UART_port)
{
	
	ClearBuf(UART_port);
	while(vu[CMD_ClearBuf].status==0xff);
	if(UART_port!=UART_ALL)
	{
		clr_ReadBuffer(UART_port);
		clr_WriteBuffer(UART_port);
	}
	else
	{
		clr_ReadBuffer(UART_ALL);
		clr_WriteBuffer(UART_ALL);

	}

	DBG_PRINTF("ClearBuf(%d):OK\n",UART_port);

}



void vu_CloseUART(UINT8 UART_port)
{
	CloseUARTPort(UART_port);
	while(vu[CMD_CloseUARTPort].status==0xff);
	if(UART_port==CLOSE_SPI)
	{
		spiDisable(SPI_SELECT);
		FLAG_CLOSE_SPI=1;
		DBG_PRINTF("SPI(%d) CLOSE:OK\n",SPI_SELECT);
	}
	DBG_PRINTF("CloseUARTPort(%d):OK\n",UART_port);
}

int vu_GetStatus()
{
	return (vu[CMD_SendDatatoUARTTXBuffer].status==0x0 && vu[CMD_ReceiveDatafromUARTRXBuffer_Max].status==0x0)?0:1;
}

char vu_GetNotification()
{
	return GetNotification();
}

int vu_GetRXAvailLen(UINT8 UART_port)
{
	return ByteOfDataRxbuf(UART_port);
}
int vu_GetTXFreeLen(UINT8 UART_port)
{
	return AvailableByteTxbuf(UART_port);
}





 const UINT8 cmdName[9][40] = 
 {
	 "ResetUARTport()","ConfigureUARTport()","UARTHardwareFlowControl()","UARTNotification()",
	 "QueryUARTTXbuffer()","SendDatatoUARTTXBuffer()","QueryUARTRXbuffer()",
	 "ReceiveDatafromUARTRXBuffer()","GetNotification()"
 };
int LASE_CMD=0;
 
 
int VU_FLAG(VU_CMD_STATUS *vu)
{
	int i=0 ;
	for(i=0;i<9;i++){
		if(vu[i].status==1){
			sysprintf("%s:%d\n",*(cmdName+i),vu[i].status);
			return 1;
		}else if(vu[i].status==2){
			sysprintf("%s:OK\n",*(cmdName+i));
			vu[i].status=0;
			return 0;
		}else if(vu[i].status==0xff){
			sysprintf("%s:WAIT\n",*(cmdName+i));
			return 0;
		}
	}
	return 0;
}

