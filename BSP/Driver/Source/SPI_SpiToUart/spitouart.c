/**************************************************************************//**
 * @file     spitouart.c
 * @version  V3.00
 * @brief    N9H20 series SPI to UART driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
/* Header files */
#include <stdio.h>
#include "wblib.h"
#include "spitouart.h"
#include "N9H20_SPI.h"
#include "N9H20_GPIO.h"

#define GP_BA 0xB8001000
#define GPIOA_PIN GP_BA+0x0C
#define SPI_port 0
#define TEST_SIZE	0xa
#define RST_SPI 0xFE
#define DBG_PRINTF(...)

#define FULL 1

#define clr_IRQ0_GPA0 outpw(REG_IRQTGSRC0,inpw(REG_IRQTGSRC0) &0x01); //N9H20
#define clr_CMD_NUM_LENGTH CMD_NUM=0;CMD_LENGTH=0;receive_buf_cnt=0;

#if defined(__GNUC__)
__attribute__((aligned(8))) volatile UINT8 WriteBuffer0[TEST_SIZE];
__attribute__((aligned(8))) volatile UINT8 ReadBuffer0[TEST_SIZE];

__attribute__((aligned(8))) volatile UINT8 WriteBuffer1[TEST_SIZE];
__attribute__((aligned(8))) volatile UINT8 ReadBuffer1[TEST_SIZE];
#else	
__align(8) volatile UINT8 WriteBuffer0[TEST_SIZE];
__align(8) volatile UINT8 ReadBuffer0[TEST_SIZE];

__align(8) volatile UINT8 WriteBuffer1[TEST_SIZE];
__align(8) volatile UINT8 ReadBuffer1[TEST_SIZE];
#endif


void vu_SPI_WRITE(int len,char *pSrc);
void vu_SPI_READ(int len,char *pDst);
int recieveStatusPackage(void);


int FLAG_MAIN_BSY=0;
int addq_ReadBuffer(int port,UINT8 data);
UINT8 deleteq_ReadBuffer(int port);

void GPIO_GPAR_ENABLE(void);

int FLAG_RESPONSE_PACKAGE=0;
int CMD_NUM=0;
int CMD_LENGTH=0;
int TMP_BUF[8]={0};
int RECEIVE_BUF[8]={0};
int RECEIVE_BUF_CNT=0;
int FLGA_UARTNotification_ENABLE=0;
char FLAVG_RX_LEVEAL_PRIORITY=0;
int flag_stage=0;
volatile int uartAllWriteCnt=0;
int tmp_src[4];
int tmp_Dst[4];
volatile int numOfDataInWriteBuffer0=0;
volatile int numOfDataInReadBuffer0=0;
volatile int rear_WriteBuffer0=-1;
volatile int front_WriteBuffer0=-1;
volatile int rear_ReadBuffer0=-1;
volatile int front_ReadBuffer0=-1;

volatile int numOfDataInWriteBuffer1=0;
volatile int numOfDataInReadBuffer1=0;
volatile int rear_WriteBuffer1=-1;
volatile int front_WriteBuffer1=-1;
volatile int rear_ReadBuffer1=-1;
volatile int front_ReadBuffer1=-1;

volatile unsigned int tmp_rx_cnt=0;
volatile unsigned char response_cmd=0;
volatile unsigned int tmp_in=0;
volatile unsigned int tmp_uart1_tx=0;
volatile unsigned int UART_PORT_ENABLE=0;
volatile unsigned char TMP_CMD_NUM=0;
int tmp_partial=0;
int tmp_partial_uart0=0;
int tmp_partial_uart1=0;

#if defined(__GNUC__)
__attribute__((aligned(32))) char CMD_LIST[12][16]={  //leading,checksum,length,cmd,parameters
	//ResetUARTport={lead,check,len,cmd,port}
	{'C',0x0,0x2,0x0,0x0,       0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},	
	//ConfigureUARTport={lead,check,len,cmd,port,baudrate[5~8byte],datawidth,parity,stop}
	{'C',0x0,0x9,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0    	 ,0x0,0x0,0x0,0x0},
	//UARTHardwareFlowControl={lead,check,len,cmd,flag}
	{'C',0x0,0x2,0x2,0x0,       0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	//UARTNotification={lead,check,len,cmd,flag}
	{'C',0x0,0x2,0x3,0x0,       0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	//QueryUARTTXbuffer={lead,check,len,cmd,port}
	{'C',0x0,0x2,0x4,0x0,       0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	//SendDatatoUARTTXBuffer={lead,check,len,cmd,port,data...n}
	{'C',0x0,0x2,0x5,0x0,       0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	//QueryUARTRXbuffer={lead,check,len,cmd,port}
	{'C',0x0,0x2,0x6,0x0,       0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	//ReceiveDatafromUARTRXBuffer={lead,check,len,cmd,port}
	//{'C',0x0,0x2,0x7,0x0,       0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	//ReceiveDatafromUARTRXBuffer_MAX={lead,check,len,cmd,port,max}
	{'C',0x0,0x4,0x7,0x0,0x0   		 ,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	//GetNotification={lead,check,len,cmd}
	{'C',0x9,0x2,0x8,           0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	//CloseUartPort
	{'C',0x0,0x2,0x9,           0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	//OpenUARTPort
	{'C',0x0,0x2,0xA,           0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	//ClearBuf
	{'C',0x0,0x2,0xB,           0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
};
#else
__align(32) char CMD_LIST[12][16]={  //leading,checksum,length,cmd,parameters
	//ResetUARTport={lead,check,len,cmd,port}
	{'C',0x0,0x2,0x0,0x0,       0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},	
	//ConfigureUARTport={lead,check,len,cmd,port,baudrate[5~8byte],datawidth,parity,stop}
	{'C',0x0,0x9,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0    	 ,0x0,0x0,0x0,0x0},
	//UARTHardwareFlowControl={lead,check,len,cmd,flag}
	{'C',0x0,0x2,0x2,0x0,       0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	//UARTNotification={lead,check,len,cmd,flag}
	{'C',0x0,0x2,0x3,0x0,       0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	//QueryUARTTXbuffer={lead,check,len,cmd,port}
	{'C',0x0,0x2,0x4,0x0,       0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	//SendDatatoUARTTXBuffer={lead,check,len,cmd,port,data...n}
	{'C',0x0,0x2,0x5,0x0,       0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	//QueryUARTRXbuffer={lead,check,len,cmd,port}
	{'C',0x0,0x2,0x6,0x0,       0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	//ReceiveDatafromUARTRXBuffer={lead,check,len,cmd,port}
	//{'C',0x0,0x2,0x7,0x0,       0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	//ReceiveDatafromUARTRXBuffer_MAX={lead,check,len,cmd,port,max}
	{'C',0x0,0x4,0x7,0x0,0x0   		 ,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	//GetNotification={lead,check,len,cmd}
	{'C',0x9,0x2,0x8,           0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	//CloseUartPort
	{'C',0x0,0x2,0x9,           0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	//OpenUARTPort
	{'C',0x0,0x2,0xA,           0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	//ClearBuf
	{'C',0x0,0x2,0xB,           0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
};
#endif


VU_CMD_STATUS *vu_cmd;

static void gpio_isr(void)
{

	int tmp,tmp_uart0_query,tmp_uart1_query;

	if(FLAG_RESPONSE_PACKAGE==1 && vu_cmd[CMD_NUM].status==0xff)
	{
 		recieveStatusPackage();
		tmp=vu_cmd[CMD_QueryUARTRXbuffer].count;
		if(tmp>0)
		{
			if(UART_PORT_ENABLE==UART_PORT0 || UART_PORT_ENABLE==UART_ALL_PORT0 )
			{
				if(vu_cmd[CMD_NUM].status!=0xff && tmp>0 && (numOfDataInReadBuffer0<TEST_SIZE)&& FLAG_MAIN_BSY!=1 )
				{
					tmp_partial=TEST_SIZE-numOfDataInReadBuffer0;
					if(((tmp&0xf0)>>4)>0)vu_cmd[CMD_QueryUARTRXbuffer].count=vu_cmd[CMD_QueryUARTRXbuffer].count&0xf;
					if(tmp_partial>0)
					{
							if(tmp<=tmp_partial)
							{
								flag_stage=1;
								ReceiveDatafromUARTRXBuffer_Max(0,tmp,0);
							}
							else
							{
									flag_stage=2;
									ReceiveDatafromUARTRXBuffer_Max(0,tmp_partial,0);
							}
					}
					
				}
				else
				{
					flag_stage=44;
					//error
				}
			}
			else if(UART_PORT_ENABLE==UART_PORT1 || UART_PORT_ENABLE==UART_ALL_PORT1)
			{
				if(vu_cmd[CMD_NUM].status!=0xff && tmp>0 && (numOfDataInReadBuffer1<TEST_SIZE)&& FLAG_MAIN_BSY!=1 )
				{
					tmp_partial=TEST_SIZE-numOfDataInReadBuffer1;
					if(((tmp&0xf0)>>4)>0)vu_cmd[CMD_QueryUARTRXbuffer].count=(vu_cmd[CMD_QueryUARTRXbuffer].count&0xf0)>>4;
					if(tmp_partial>0)
					{
						if(tmp<=tmp_partial)
						{
							flag_stage=4;
							ReceiveDatafromUARTRXBuffer_Max(1,tmp,0);
						}
						else
						{
								flag_stage=5;
								ReceiveDatafromUARTRXBuffer_Max(1,tmp_partial,0);
						}
					}
				
				}else{
					flag_stage=45;
					//error
				}
			}
			else if(UART_PORT_ENABLE==UART_ALL)
			{
				flag_stage=20;
				if(vu_cmd[CMD_NUM].status!=0xff && tmp>0 && ((numOfDataInReadBuffer1<TEST_SIZE) || (numOfDataInReadBuffer0<TEST_SIZE))&& FLAG_MAIN_BSY!=1 )
				{
					tmp_partial_uart0=TEST_SIZE-numOfDataInReadBuffer0;
					tmp_partial_uart1=TEST_SIZE-numOfDataInReadBuffer1;
					tmp_uart0_query=tmp&0x0f;
					tmp_uart1_query=(tmp&0xf0)>>4;
					flag_stage=21;
									
					if(tmp_uart0_query>TEST_SIZE/2 && tmp_uart1_query<=TEST_SIZE/2 )
					{
							UART_PORT_ENABLE=UART_ALL_PORT0;
							if(tmp_uart0_query<=tmp_partial_uart0)
							{
								ReceiveDatafromUARTRXBuffer_Max(UART_PORT0,tmp_uart0_query,0); flag_stage=7;
							}
							else
							{
									flag_stage=8;
									ReceiveDatafromUARTRXBuffer_Max(0,tmp_partial_uart0,0);	
							}
					}else if(tmp_uart0_query<=TEST_SIZE/2 && tmp_uart1_query>TEST_SIZE/2)
					{
						UART_PORT_ENABLE=UART_ALL_PORT1;
						if(tmp_uart0_query>0)vu_cmd[CMD_QueryUARTRXbuffer].count=(vu_cmd[CMD_QueryUARTRXbuffer].count&0xf0)>>4;
						if(tmp_uart1_query<=tmp_partial_uart1)
						{
							ReceiveDatafromUARTRXBuffer_Max(UART_PORT1,tmp_uart1_query,0); flag_stage=10;
						}
						else
						{
								flag_stage=11;
								ReceiveDatafromUARTRXBuffer_Max(1,tmp_partial_uart1,0);	
						}	
					}else if(tmp_uart0_query<=TEST_SIZE/2 && tmp_uart1_query<=TEST_SIZE/2 )
					{
						if(tmp_uart0_query<=tmp_partial_uart0 && tmp_uart1_query<=tmp_partial_uart1)
						{
							ReceiveDatafromUARTRXBuffer_Max(UART_ALL,tmp_uart0_query,tmp_uart1_query); flag_stage=13;
						}
						else if(tmp_uart0_query>tmp_partial_uart0 && tmp_uart1_query<=tmp_partial_uart1)
						{
							ReceiveDatafromUARTRXBuffer_Max(UART_ALL,tmp_partial_uart0,tmp_uart1_query); flag_stage=14;
						}
						else if(tmp_uart0_query<=tmp_partial_uart0 && tmp_uart1_query>tmp_partial_uart1)
						{
							ReceiveDatafromUARTRXBuffer_Max(UART_ALL,tmp_uart0_query,tmp_partial_uart1); flag_stage=15;	
						}
						else if(tmp_uart0_query>tmp_partial_uart0 && tmp_uart1_query>tmp_partial_uart1)
						{
							ReceiveDatafromUARTRXBuffer_Max(UART_ALL,tmp_partial_uart0,tmp_partial_uart1); flag_stage=16;
						}
					}else{ //if(tmp_uart0_query>TEST_SIZE/2 && tmp_uart1_query>TEST_SIZE/2){
						if(tmp_uart0_query>TEST_SIZE/2 && tmp_uart1_query>TEST_SIZE/2){
								if(tmp_partial_uart0>TEST_SIZE/2 && tmp_partial_uart1>TEST_SIZE/2)
								{
									ReceiveDatafromUARTRXBuffer_Max(UART_ALL,5,5); flag_stage=18;
								}
								else if(tmp_partial_uart0<=TEST_SIZE/2 && tmp_partial_uart1>TEST_SIZE/2)
								{
									ReceiveDatafromUARTRXBuffer_Max(UART_ALL,tmp_partial_uart0,5); flag_stage=19;
								}
								else if(tmp_partial_uart0>TEST_SIZE/2 && tmp_partial_uart1<=TEST_SIZE/2)
								{
									ReceiveDatafromUARTRXBuffer_Max(UART_ALL,5,tmp_partial_uart1); flag_stage=20;
								}
								else if(tmp_partial_uart0<TEST_SIZE/2 && tmp_partial_uart1<TEST_SIZE/2)
								{
									ReceiveDatafromUARTRXBuffer_Max(UART_ALL,tmp_partial_uart0,tmp_partial_uart0); flag_stage=21;
								}
							}else
							{
							//error
							flag_stage=41;
						}
					}
				
				}else
				{
						//WAIT
						flag_stage=42;
				}
			}//else if(UART_PORT_ENABLE==UART_ALL)
		}//if(tmp>0){
	}else if(FLAG_RESPONSE_PACKAGE==0 && vu_cmd[CMD_NUM].status==0x0 )
	{
		if(FLAVG_RX_LEVEAL_PRIORITY>'R' ){
			if(FLAG_MAIN_BSY!=1)
			{
				if(numOfDataInReadBuffer0>=TEST_SIZE && numOfDataInReadBuffer1>=TEST_SIZE)
				{
					flag_stage=28;
					FLAVG_RX_LEVEAL_PRIORITY='R';
					//sysprintf("ERROR:RX_BUFF_FULL\n");
				}else if(FLAG_MAIN_BSY!=1)
				{
					if(FLAVG_RX_LEVEAL_PRIORITY=='S' && (TEST_SIZE-numOfDataInReadBuffer0)>0)
					{
						if(UART_PORT_ENABLE==UART_ALL || UART_PORT_ENABLE==UART_ALL_PORT1|| UART_PORT_ENABLE==UART_ALL_PORT0)
						{
								UART_PORT_ENABLE=UART_ALL_PORT0;
						}
						else
						{
								UART_PORT_ENABLE=UART_PORT0;
						}
						flag_stage=30;
						QueryUARTRXBuffer(UART_PORT0); 
					}
					else if(FLAVG_RX_LEVEAL_PRIORITY=='T'  && (TEST_SIZE-numOfDataInReadBuffer1)>0)
					{
						if(UART_PORT_ENABLE==UART_ALL || UART_PORT_ENABLE==UART_ALL_PORT1|| UART_PORT_ENABLE==UART_ALL_PORT0)
						{
								UART_PORT_ENABLE=UART_ALL_PORT1;
						}
						else
						{
								UART_PORT_ENABLE=UART_PORT1;
						}
						flag_stage=31;
						QueryUARTRXBuffer(UART_PORT1); 
					}
					else if(FLAVG_RX_LEVEAL_PRIORITY=='U')
					{
						flag_stage=32;
						UART_PORT_ENABLE=UART_ALL;
						QueryUARTRXBuffer(UART_ALL);
					}
					else
					{
						flag_stage=33;
					}
					FLAVG_RX_LEVEAL_PRIORITY='R'; 
				}
			}else
			{
				FLAVG_RX_LEVEAL_PRIORITY='R'; 	flag_stage=37;
			}
		}else{
			if( numOfDataInReadBuffer0>0 || numOfDataInReadBuffer1>0 || numOfDataInWriteBuffer0>0||numOfDataInWriteBuffer1>0)
			{
				flag_stage=25;
			}
			else if(FLAG_MAIN_BSY!=1)
			{
				flag_stage=22;
				QueryUARTRXBuffer(UART_PORT_ENABLE);
			}		
		}
	}
	clr_IRQ0_GPA0
}
		

			

UINT8 checksum(int len,char *ptr)
{
	
	int i;
	UINT8 sum=0;	
	for(i=0;i<=len;i++){
		DBG_PRINTF("ptr[%d]=%x,%c\n",i,*(ptr+i),*(ptr+i));
		sum+=*(ptr+i);
	}
	
	return sum;
}

void CMD_32BITS_DIVIDEINTO_8BITS(int *_buf,int cnt,char *buf_){
	int i=0;
		for(i=0;i<cnt*4;i=i+4)
		{
			buf_[i]	 =(_buf[i/4]    )&0xff;
			buf_[i+1]=(_buf[i/4]>>8 )&0xff;
			buf_[i+2]=(_buf[i/4]>>16)&0xff;
			buf_[i+3]=(_buf[i/4]>>24)&0xff;
		}
}


int recieveStatusPackage(void){
	UINT8 reorder[28]={0};
	int sum,i;
	if(RECEIVE_BUF_CNT<=CMD_LENGTH)
	{
		if((CMD_LENGTH)<=4)
		{
				vu_SPI_READ((CMD_LENGTH),(char *)(RECEIVE_BUF));
				RECEIVE_BUF_CNT=RECEIVE_BUF_CNT+CMD_LENGTH;
		}
		else
		{
				sysprintf("ERROR:ONLY RECEIVE 5 INTEGER=%d,%d\n",RECEIVE_BUF_CNT,CMD_LENGTH);
				FLAG_RESPONSE_PACKAGE=0xff;//error
				vu_cmd[CMD_NUM].status=0x1;
		}		
		if((RECEIVE_BUF[0]&0xf)==0)
		{
				return 0; //THIS GPIO_INT IS FOR RXEINT , IS NOT RESPONSE INT
		}
		if(RECEIVE_BUF_CNT==CMD_LENGTH)
		{
			CMD_32BITS_DIVIDEINTO_8BITS((int *)RECEIVE_BUF,CMD_LENGTH,reorder);
	
			if(UART_PORT_ENABLE==UART_ALL && CMD_NUM==CMD_ReceiveDatafromUARTRXBuffer_Max)
			{
				sum=checksum(2+TEST_SIZE,(reorder+LENGTH));
			}
			else
			{
				sum=checksum(reorder[LENGTH],(reorder+LENGTH));
			}
			
			if(reorder[CHEACKSUM]==sum)
			{
				if(CMD_NUM!=reorder[COMMAND])
				{
					if(reorder[COMMAND]==0)
					{
							return 0;
					}
					else
					{
						sysprintf("not correct response(%d,%d)\n",CMD_NUM,reorder[COMMAND]);
					}
				}
				if(CMD_NUM==CMD_UARTNotification)
				{
					vu_cmd[CMD_UARTNotification].count=FLGA_UARTNotification_ENABLE;
					vu_cmd[CMD_UARTNotification].status=0x0;
				}
				else if(CMD_NUM==CMD_QueryUARTRXbuffer)
				{
					if(UART_PORT_ENABLE==UART_PORT0 || UART_PORT_ENABLE==UART_ALL_PORT0)
					{
							if(reorder[PARAMETER]>TEST_SIZE )
							{
								vu_cmd[CMD_QueryUARTRXbuffer].count=TEST_SIZE;
							}
							else
							{
								vu_cmd[CMD_QueryUARTRXbuffer].count=reorder[PARAMETER];
							}
						
					}
					else if(UART_PORT_ENABLE==UART_PORT1||UART_PORT_ENABLE==UART_ALL_PORT1)
					{
						if(reorder[PARAMETER+1]>TEST_SIZE)
						{
							vu_cmd[CMD_QueryUARTRXbuffer].count=TEST_SIZE;
						}
						else
						{
							vu_cmd[CMD_QueryUARTRXbuffer].count=reorder[PARAMETER+1];
						}
						
					}else if(UART_PORT_ENABLE==UART_ALL)
					{
						if(reorder[PARAMETER]>TEST_SIZE)
						{
								vu_cmd[CMD_QueryUARTRXbuffer].count=TEST_SIZE; //10'b1010_1010 --> uar1=10 byte & uart0=10 byte
						}
						else if(reorder[PARAMETER]<=TEST_SIZE&&reorder[PARAMETER]>0)
						{
								vu_cmd[CMD_QueryUARTRXbuffer].count=reorder[PARAMETER];
						}
						else{ //<0
								UART_PORT_ENABLE=UART_ALL_PORT1;
						}
						
						if(reorder[PARAMETER+1]>TEST_SIZE)
						{
								vu_cmd[CMD_QueryUARTRXbuffer].count=vu_cmd[CMD_QueryUARTRXbuffer].count|0xA0; //10'b1010_1010 --> uar1=10 byte & uart0=10 byte
						}
						else if(reorder[PARAMETER+1]<=TEST_SIZE&&reorder[PARAMETER+1]>0)
						{
								vu_cmd[CMD_QueryUARTRXbuffer].count=(vu_cmd[CMD_QueryUARTRXbuffer].count|((reorder[PARAMETER+1])<<4));
						}
						else
						{ //<0
							UART_PORT_ENABLE= UART_PORT_ENABLE==UART_ALL_PORT1? UART_ALL/*error*/:UART_ALL_PORT0;  //0x99 == error
						}
						
						if(UART_PORT_ENABLE==UART_ALL_PORT1)
						{
								vu_cmd[CMD_QueryUARTRXbuffer].count=reorder[PARAMETER+1];
						}
						else if(UART_PORT_ENABLE==0x99)
						{
								//error
						}
					}				
					
				}
				else if(CMD_NUM==CMD_ReceiveDatafromUARTRXBuffer_Max)
				{
					if(UART_PORT_ENABLE==UART_PORT0 || UART_PORT_ENABLE==UART_ALL_PORT0)
					{
						for(i=0;i<(reorder[LENGTH]-1);i++)
						{
							addq_ReadBuffer(UART_PORT0,reorder[PARAMETER+i]);
						}
						
						if((reorder[LENGTH]-1)==0)
						{
								FLAG_RESPONSE_PACKAGE=0xff;//error
						}
					
						if(UART_PORT_ENABLE==UART_ALL_PORT0)
						{
							UART_PORT_ENABLE=UART_ALL;	
						}
						if(	(vu_cmd[CMD_QueryUARTRXbuffer].count&0x0f)>TEST_SIZE)
						{
								vu_cmd[CMD_ReceiveDatafromUARTRXBuffer_Max].status=0x2;
						}
						vu_cmd[CMD_QueryUARTRXbuffer].count=vu_cmd[CMD_QueryUARTRXbuffer].count-(reorder[LENGTH]-1);
						if(	(vu_cmd[CMD_QueryUARTRXbuffer].count&0x0f)>TEST_SIZE){
								vu_cmd[CMD_ReceiveDatafromUARTRXBuffer_Max].status=0x2;
						}
				  }
				  else if(UART_PORT_ENABLE==UART_PORT1 || UART_PORT_ENABLE==UART_ALL_PORT1)
				  {

					for(i=0;i<(reorder[LENGTH]-1);i++)
					{
						addq_ReadBuffer(UART_PORT1,reorder[PARAMETER+i]);
					}		
					vu_cmd[CMD_QueryUARTRXbuffer].count=(vu_cmd[CMD_QueryUARTRXbuffer].count-(reorder[LENGTH]-1));			
																
					if(UART_PORT_ENABLE==UART_ALL_PORT1)
					{
						UART_PORT_ENABLE=UART_ALL;	
					}
					}
					else if(UART_PORT_ENABLE==UART_ALL)
					{

						for(i=0;i<((reorder[LENGTH]&0x0f));i++)
						{
							addq_ReadBuffer(UART_PORT0,reorder[PARAMETER+i]);
						}

						vu_cmd[CMD_QueryUARTRXbuffer].count=vu_cmd[CMD_QueryUARTRXbuffer].count-((reorder[LENGTH]&0x0f));
		
						for(i=0;i<(((reorder[LENGTH])&0xf0)>>4);i++)
						{
							addq_ReadBuffer(UART_PORT1,reorder[PARAMETER+5+i]);
						}

						vu_cmd[CMD_QueryUARTRXbuffer].count=vu_cmd[CMD_QueryUARTRXbuffer].count-((reorder[LENGTH]&0xf0));
						
					}
				}
				else if(CMD_NUM==CMD_QueryUARTTXbuffer)
				{
					if(UART_PORT_ENABLE==UART_PORT0 || UART_PORT_ENABLE==UART_ALL_PORT0)
					{
						vu_cmd[CMD_QueryUARTTXbuffer].count=  *(reorder+PARAMETER);
					}
					else if(UART_PORT_ENABLE==UART_PORT1|| UART_PORT_ENABLE==UART_ALL_PORT1)
					{
						vu_cmd[CMD_QueryUARTTXbuffer].count=  *(reorder+PARAMETER+1);
					}
					else
					{
						vu_cmd[CMD_QueryUARTTXbuffer].count= ((reorder[PARAMETER])|((reorder[PARAMETER+1])<<8));
					}
					
				}
				else if(CMD_NUM==CMD_SendDatatoUARTTXBuffer)
				{
					if(UART_PORT_ENABLE==UART_ALL_PORT0 || UART_PORT_ENABLE==UART_ALL_PORT1)UART_PORT_ENABLE=UART_ALL;
				}
				
				FLAVG_RX_LEVEAL_PRIORITY=(reorder[0]&0xff);
				RECEIVE_BUF_CNT=0;
				FLAG_RESPONSE_PACKAGE=0;
				vu_cmd[CMD_NUM].status=0x0;//reorder[PARAMETER];
				
			}
			else
			{
				RECEIVE_BUF_CNT=0;
				CMD_LENGTH=0;
				FLAG_RESPONSE_PACKAGE=0x0;//error
				vu_cmd[CMD_NUM].status=0x0;
				DBG_PRINTF("ERROR:CHECKSUM(golden,silver)= %d,%d\n",reorder[CHEACKSUM],sum);
			}
		}
		else
		{
			
			DBG_PRINTF("ERROR:ONLY RECEIVE 5 INTEGER=%d,%d\n",RECEIVE_BUF_CNT,CMD_LENGTH);
		}
	}
	else
	{
		RECEIVE_BUF_CNT=0;
		CMD_LENGTH=0;
		DBG_PRINTF("ERROR:(RECEIVE_BUF_CNT,CMD_LENGTH)=%d,%d\n",RECEIVE_BUF_CNT,CMD_LENGTH);
		FLAG_RESPONSE_PACKAGE=0x0;//error
		vu_cmd[CMD_NUM].status=0x0;
		
	}
	return 0;
}


void OpenUARTPort(UINT8 UART_port,VU_CMD_STATUS *vu)
{
		/*
			offset
					0:'C'		(leadcode:0x43)
					1:			(checksum)
					2:2 		(length)
					3:0x0A	(command)
					4:port	(paramter)
	 */
	
		char *buf;
		while(_FLAG_RESPONSE_PACKAGE());
		vu_cmd=vu;
		UART_PORT_ENABLE=UART_port;
		vu_cmd[CMD_OpenUARTPort].status=0xff; //wait
		buf=CMD_LIST[CMD_OpenUARTPort];

		CMD_NUM=buf[COMMAND];

		*(buf)= 'C';
		*(buf+PARAMETER)=UART_port;
		*(buf+CHEACKSUM)=checksum(CMD_OpenUARTPort,(buf+2));
		
		CMD_LENGTH=2; //response cmd = (lead+checksum+len+cmd)+flag
	
		//sysprintf("0");

		vu_SPI_WRITE(2,buf);
		FLAG_RESPONSE_PACKAGE=1;
}

void ResetUARTPort(UINT8 UART_port,VU_CMD_STATUS *vu)
{
		/*
			offset
					0:'C'		(leadcode:0x43)
					1:			(checksum)
					2:2 		(length)
					3:0x00	(command)
					4:port	(paramter)
	 */

		char *buf;
		if(UART_port==RST_SPI)
		{
				FLAG_RESPONSE_PACKAGE=0;//error
				vu_cmd[CMD_NUM].status=0x0;
		}
		while(_FLAG_RESPONSE_PACKAGE());
	
		vu_cmd=vu;
		vu_cmd[CMD_ResetUARTPort].status=0xff; //wait
		buf=CMD_LIST[CMD_ResetUARTPort];
		CMD_NUM=buf[COMMAND];
		

	
		*(buf)= 'C';
		*(buf+PARAMETER)=UART_port;
		*(buf+CHEACKSUM)=checksum(LENGTH_ResetUARTPort,(buf+2));
		
		CMD_LENGTH=2; //response cmd = (lead+checksum+len+cmd)+flag
	
		//sysprintf("0");

		vu_SPI_WRITE(2,buf);
		FLAG_RESPONSE_PACKAGE=1;
		
}

void ConfigureUARTPort(UINT8 UART_port,UINT32 baudrate,UINT32 datawidth,UINT8 parity,UINT8 stop,VU_CMD_STATUS *vu)
{
		/*
			offset
					0:'C'					(leadcode:0x43)
					1:						(checksum)
					2:9 					(length)
					3:0x01				(command)
					4:port				(paramter+0)
					5~8:Baudrate	(paramter+(1~4))	
					9:Data Width  (paramter+5)
				 10:Parity      (paramter+6)
				 11:Stop        (paramter+7)
	 */
	
	
	char *buf;
	while(_FLAG_RESPONSE_PACKAGE());

	vu_cmd[CMD_ConfigureUARTPort].status=0xff; //wait
	buf=CMD_LIST[CMD_ConfigureUARTPort];
	CMD_NUM=buf[COMMAND];

	*(buf)= 'C';
	*(buf+PARAMETER+0)= UART_port;
	*(buf+PARAMETER+1)= (baudrate&0xff);
	*(buf+PARAMETER+2)= ((baudrate>>8)&0xff);
	*(buf+PARAMETER+3)= ((baudrate>>(8*2))&0xff);
	*(buf+PARAMETER+4)= ((baudrate>>(8*3))&0xff);
	*(buf+PARAMETER+5)= datawidth;
	*(buf+PARAMETER+6)= parity;
	*(buf+PARAMETER+7)= stop;
	*(buf+CHEACKSUM)=checksum(LENGTH_ConfigureUARTPort,(buf+2));


	CMD_LENGTH=2; //response cmd = (lead+checksum+len+cmd)+FLAG


	//sysprintf("1");
	vu_SPI_WRITE(3,buf); // CMD= (lead+checksum+len+cmd)+(Port+buadrate0+buadrate1+buadrate2)+(buadrate3+datawidth+parity+stop)
	FLAG_RESPONSE_PACKAGE=1;
}
int first_gpio_enable=0;
void UARTNotification(char flag,VU_CMD_STATUS *vu)
{
	
		/*
		1.Require a pin as interrupt from the Controller to 
		Host Processor/MCU to notify (finish of TX) or (Data in RX buffer)
		2.Default is disabled

			offset
					0:'C'		(leadcode:0x43)
					1:			(checksum)
					2:2 		(length)
					3:0x03	(command)
					4:flag	(0:disable/1:enable)
	 */
		
		char *buf;
		while(_FLAG_RESPONSE_PACKAGE());
		vu_cmd=vu;
		vu_cmd[CMD_UARTNotification].status=0xff; //wait
		if(first_gpio_enable==0){
			GPIO_GPAR_ENABLE();
			first_gpio_enable=1;
		}

		
		buf=CMD_LIST[CMD_UARTNotification];
		CMD_NUM=buf[COMMAND];


		*(buf)= ((numOfDataInReadBuffer0)==TEST_SIZE)?'D':'C';
		*(buf+PARAMETER)=flag;
		*(buf+CHEACKSUM)=checksum(LENGTH_UARTNotification,(buf+2));

		
	
		CMD_LENGTH=2; //response cmd = (lead+checksum+len+cmd)+flag

		FLGA_UARTNotification_ENABLE=flag;
		
	
		//DBG_PRINTF("3");
		vu_SPI_WRITE(2,buf);
		FLAG_RESPONSE_PACKAGE=1;
		
}

void QueryUARTTXBuffer(UINT8 UART_port,VU_CMD_STATUS *vu)
{
	
		/*
			offset
					0:'C'		(leadcode:0x43)
					1:			(checksum) //length+command+parameters
					2:3 		(length)   //command+parameters
					3:0x04	(command)
					4:PORT  0:uart0/ 1:uar1/ 0xff:uart0&uart1
			
	 */
	char *buf;
		//if((FLAVG_RX_LEVEAL_PRIORITY>'R'))gpio_isr();
		while(_FLAG_RESPONSE_PACKAGE());
		//vu_cmd=vu;
	
		vu_cmd[CMD_QueryUARTTXbuffer].status=0xff; //wait

		buf=CMD_LIST[CMD_QueryUARTTXbuffer];
		CMD_NUM=buf[COMMAND];

		*(buf)= ((numOfDataInReadBuffer0)>=TEST_SIZE && (numOfDataInReadBuffer1)>=TEST_SIZE)?'F': 
						((numOfDataInReadBuffer0)>=TEST_SIZE && (numOfDataInReadBuffer1)<TEST_SIZE)? 'D':
						((numOfDataInReadBuffer0)<TEST_SIZE && (numOfDataInReadBuffer1)>=TEST_SIZE)? 'E':'C';
		*(buf+PARAMETER)=UART_port;
		*(buf+CHEACKSUM)=checksum(LENGTH_QueryUARTTXbuffer,(buf+2));

		if(UART_PORT_ENABLE==UART_ALL){
			if(UART_port==UART_PORT0){
				UART_PORT_ENABLE=UART_ALL_PORT0;
			}else if(UART_port==UART_PORT1){
				UART_PORT_ENABLE=UART_ALL_PORT1;
			}
		}
			

		CMD_LENGTH=2; //response cmd = (lead+checksum+len+cmd)+flag		
		//DBG_PRINTF("4");
		vu_SPI_WRITE(2,buf);
		FLAG_RESPONSE_PACKAGE=1;

}


void SendDatatoUARTTXBuffer(UINT8 UART_port,UINT UART0_len,UINT UART1_len,VU_CMD_STATUS *vu)
{
	
	/*
			offset
					0:'C'		(leadcode:0x43)
					1:			(checksum) //length+command+parameters
					2:n-2 	(length)   //command+parameters
					3:0x05	(command)
					4:PORT	
					5...n:Data

	 */

		char *buf;
	
		int i;
	
		while(_FLAG_RESPONSE_PACKAGE()){
		
		};


		vu_cmd[CMD_SendDatatoUARTTXBuffer].status=0xff; //wait

		buf=CMD_LIST[CMD_SendDatatoUARTTXBuffer];
		CMD_NUM=buf[COMMAND];
		*(buf)= 'C';
	if(UART_port!=UART_ALL){
		*(buf+LENGTH)=LENGTH_SendDatatoUARTTXBuffer+UART0_len;
		*(buf+PARAMETER)=UART_port;

		if(UART0_len<=TEST_SIZE){
			for(i=0;i<UART0_len;i++){
					if(numOfDataInWriteBuffer0>TEST_SIZE || numOfDataInWriteBuffer1>TEST_SIZE){
						sysprintf("!ERROR_TX_FULL\n");
						break;
					}else if(numOfDataInWriteBuffer0<0 || numOfDataInWriteBuffer1<0){
						sysprintf("ERROR_TX_EMPTY\n");
					}else{
						*(buf+PARAMETER+1+i)=deleteq_WriteBuffer(UART_port);
					}
					
			}
		}else{
			sysprintf("@ERROR_TX_FULL\n");
		}

		*(buf+CHEACKSUM)=checksum(LENGTH_SendDatatoUARTTXBuffer+UART0_len,(buf+2));
		CMD_LENGTH=2; //response cmd = (lead+checksum+len+cmd) +flag
		//sysprintf("5");
		vu_SPI_WRITE(1+(1+UART0_len+3)/4,buf); //(lead+checksum+len+cmd)
	}else{
	/*
			offset
					0:'C'		(leadcode:0x43)
					1:			(checksum) //length+command+parameters
					2:n-2 	(length)   //command+parameters
					3:0x05	(command)
					4:PORT
					4~9:..n: PORT0 Data	
					10~15:PORT1 Data	
	 */
			*(buf+LENGTH)=LENGTH_SendDatatoUARTTXBuffer+(UART0_len|UART1_len<<4);
			*(buf+PARAMETER)=UART_port;
			if(UART0_len<=5 && UART1_len<=5){
				for(i=0;i<UART0_len;i++){
						if(numOfDataInWriteBuffer0>TEST_SIZE || numOfDataInWriteBuffer1>TEST_SIZE){
							DBG_PRINTF("!ERROR_TX_FULL\n");
							break;
						}else if(numOfDataInWriteBuffer0<0 && numOfDataInWriteBuffer1<0){
							DBG_PRINTF("ERROR_TX_EMPTY\n");
						}else{
							*(buf+PARAMETER+1+i)=deleteq_WriteBuffer(UART_PORT0);
						}
				}
				for(i=0;i<UART1_len;i++){
						if(numOfDataInWriteBuffer0>TEST_SIZE || numOfDataInWriteBuffer1>TEST_SIZE){
							DBG_PRINTF("!ERROR_TX_FULL\n");
							break;
						}else if(numOfDataInWriteBuffer0<0 && numOfDataInWriteBuffer1<0){
							DBG_PRINTF("ERROR_TX_EMPTY\n");
						}else{
							*(buf+PARAMETER+6+i)=deleteq_WriteBuffer(UART_PORT1);
						}
				}
			}else{
				DBG_PRINTF("@ERROR_TX_FULL\n");
			}
			

			*(buf+CHEACKSUM)=checksum(13,(buf+2));

			CMD_LENGTH=2; //response cmd = (lead+checksum+len+cmd) +flag
		
			//DBG_PRINTF("5");
			vu_SPI_WRITE(4,buf); //(lead+checksum+len+cmd)

	}
		FLAG_RESPONSE_PACKAGE=1;
}
void QueryUARTRXBuffer(UINT8 UART_port)
{

	/*
			offset
					0:'C'		(leadcode:0x43)
					1:			(checksum) //length+command+parameters
					2:2 		(length)   //command+parameters
					3:0x06	(command)
					4:PORT	
	 */
	
		char *buf;
		while(vu_cmd[CMD_NUM].status==0xff||FLAG_RESPONSE_PACKAGE==1||FLAG_MAIN_BSY==1);

		vu_cmd[CMD_QueryUARTRXbuffer].status=0xff; //wait
		buf=CMD_LIST[CMD_QueryUARTRXbuffer];
		CMD_NUM=buf[COMMAND];

		*(buf)= ((numOfDataInReadBuffer0)>=TEST_SIZE && (numOfDataInReadBuffer1)>=TEST_SIZE)?'F': 
						((numOfDataInReadBuffer0)>=TEST_SIZE && (numOfDataInReadBuffer1)<TEST_SIZE)? 'D':
						((numOfDataInReadBuffer0)<TEST_SIZE && (numOfDataInReadBuffer1)>=TEST_SIZE)? 'E':'C';
		*(buf+PARAMETER)=UART_port;
		*(buf+CHEACKSUM)=checksum(LENGTH_QueryUARTRXbuffer,(buf+2));



		CMD_LENGTH=2; //response cmd = (lead+checksum+len+cmd)+flag
	
		//sysprintf("6");
		vu_SPI_WRITE(2,buf);
		FLAG_RESPONSE_PACKAGE=1;
		
}

void ReceiveDatafromUARTRXBuffer_Max(UINT8 UART_port,UINT8 UART0_Max,UINT8 UART1_Max)
{ 
	 /*
			offset
					0:'C'		(leadcode:0x43)
					1:			(checksum) //length+command+parameters
					2:4 		(length)   //command+parameters
					3:0x07	(command)
					4:PORT
					5:UART0_Max
					6:UART1_Max
	 */
	char *buf;   
  while(vu_cmd[CMD_NUM].status==0xff );

	vu_cmd[CMD_ReceiveDatafromUARTRXBuffer_Max].status=0xff; //wait
	buf=CMD_LIST[CMD_ReceiveDatafromUARTRXBuffer_Max];
	CMD_NUM=buf[COMMAND];

	*(buf)= 'C';
	*(buf+PARAMETER)=UART_port;
	*(buf+PARAMETER+1)=UART0_Max;
	*(buf+PARAMETER+2)=UART1_Max;
	*(buf+CHEACKSUM)=checksum(LENGTH_ReceiveDatafromUARTRXBuffer_Max+1,(buf+2));


	if(UART_PORT_ENABLE==UART_ALL)
		CMD_LENGTH=1+((TEST_SIZE+3)/4); //response cmd = (lead+checksum+len+cmd)+(max)
	else
		CMD_LENGTH=1+((UART0_Max+3)/4);
	//sysprintf("7");
	vu_SPI_WRITE(2,buf);
	FLAG_RESPONSE_PACKAGE=1;
}


void CloseUARTPort(UINT8 UART_port)
{ 
	 /*
			offset
					0:'C'		(leadcode:0x43)
					1:			(checksum) //length+command+parameters
					2:4 		(length)   //command+parameters
					3:0x09	(command)
					4:PORT
	 */
	char *buf;   
  while(vu_cmd[CMD_NUM].status==0xff);

	vu_cmd[CMD_CloseUARTPort].status=0xff; //wait
	buf=CMD_LIST[CMD_CloseUARTPort];
	CMD_NUM=buf[COMMAND];

	*(buf)='C';
	*(buf+PARAMETER)=UART_port;

	*(buf+CHEACKSUM)=checksum(LENGTH_CloseUARTPort,(buf+2));


		CMD_LENGTH=2;
	//sysprintf("9");
	vu_SPI_WRITE(2,buf);
	FLAG_RESPONSE_PACKAGE=1;
}


void ClearBuf(UINT8 UART_port)
{ 
	 /*
			offset
					0:'C'		(leadcode:0x43)
					1:			(checksum) //length+command+parameters
					2:4 		(length)   //command+parameters
					3:0x0B	(command)
					4:PORT
	 */
	char *buf;   
  while(vu_cmd[CMD_NUM].status==0xff);

	vu_cmd[CMD_ClearBuf].status=0xff; //wait
	buf=CMD_LIST[CMD_ClearBuf];
	CMD_NUM=buf[COMMAND];

	*(buf)='C';
	*(buf+PARAMETER)=UART_port;

	*(buf+CHEACKSUM)=checksum(LENGTH_ClearBuf,(buf+2));


		CMD_LENGTH=2;
	//sysprintf("9");
	vu_SPI_WRITE(2,buf);
	FLAG_RESPONSE_PACKAGE=1;
}

char GetNotification()
{
/*When interrupt occurs, get notification status
*/	
	char status=0;
	
	if(numOfDataInReadBuffer0>0)
		status=status|0x1;
	if(numOfDataInWriteBuffer0==0)
		status=status|0x2;
	if(numOfDataInReadBuffer1>0)
		status=status|0x4;
	if(numOfDataInWriteBuffer1==0)
		status=status|0x8;
	

	return status;
}

void vu_SPI_WRITE(int len,char *pSrc)
{

	outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) | 0x02);	// CS1 _LOW
	tmp_src[0]=*pSrc;
	tmp_src[1]=*(pSrc+1);
	tmp_src[2]=*(pSrc+2);
	tmp_src[3]=*(pSrc+3);
	spiWrite(SPI_port, SPI_32BIT, len, pSrc);
	outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & 0xfffffffc);	// CS1 _HIGH

}

void vu_SPI_READ(int len,char *pDst)
{

	outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) | 0x02);	// CS1 _LOW
	tmp_Dst[0]=*pDst;
	tmp_Dst[1]=*(pDst+1);
	tmp_Dst[2]=*(pDst+2);
	tmp_Dst[3]=*(pDst+3);
	spiRead(SPI_port, SPI_32BIT, len,  pDst);
	outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & 0xfffffffc);	// CS1 _HIGH

}

int _FLAG_RESPONSE_PACKAGE(void)
{
	return (FLAG_RESPONSE_PACKAGE==1||vu_cmd[CMD_NUM].status==0xff ||((FLAVG_RX_LEVEAL_PRIORITY>'R'))?1:0);
}

int ByteOfDataRxbuf(UINT8 port)
{
	return port==UART_PORT0?numOfDataInReadBuffer0:port==UART_PORT1?numOfDataInReadBuffer1:port==UART_ALL?((numOfDataInReadBuffer1<<4)|numOfDataInReadBuffer0):0;
}
int AvailableByteTxbuf(UINT8 port)
{
	return port==UART_PORT0?(TEST_SIZE-numOfDataInWriteBuffer0):port==UART_PORT1?(TEST_SIZE-numOfDataInWriteBuffer1):port==UART_ALL?((TEST_SIZE-numOfDataInWriteBuffer1)<<4)|(TEST_SIZE-numOfDataInWriteBuffer0):0;
}


int _sumofDataInReadBuffer0(int port)
{
	return port==UART_PORT0?numOfDataInReadBuffer0:port==UART_PORT1?numOfDataInReadBuffer1:port==UART_ALL?((numOfDataInReadBuffer1<<4)|numOfDataInReadBuffer0):0;
}

void _mainbsy(int bsy)
{
	
	FLAG_MAIN_BSY=bsy;
}

int _UART_ALL_WRITE_BUFFER_NUM(void)
{
	return (numOfDataInWriteBuffer0|numOfDataInWriteBuffer1<<4);
}
/****************************************************/
int IsEmpty_ReadBuffer(int port)
{
	if(port==UART_PORT0)
		return (numOfDataInReadBuffer0<=0)?1:numOfDataInReadBuffer0;
	else if(port==UART_PORT1)
		return (numOfDataInReadBuffer1<=0)?1:numOfDataInReadBuffer1;
	else DBG_PRINTF("ERROR_PORT\n");
	
	return 0;
}
int IsFull_ReadBuffer(int port)
{
	if(port==UART_PORT0)
		return(numOfDataInReadBuffer0==(TEST_SIZE))?1:0;
	else if(port==UART_PORT1)
		return(numOfDataInReadBuffer0==(TEST_SIZE))?1:0;
	else DBG_PRINTF("ERROR_PORT\n");
	
	return 0;
}


int addq_ReadBuffer(int port,UINT8 data)
{

	if(port==UART_PORT0)
	{
		if(numOfDataInReadBuffer0<TEST_SIZE)
		{
			rear_ReadBuffer0=(rear_ReadBuffer0+1)%TEST_SIZE;
			ReadBuffer0[rear_ReadBuffer0]=data;
			numOfDataInReadBuffer0=(numOfDataInReadBuffer0+1);
		}
		else
		{
			DBG_PRINTF("\nReadBuffer is FULL\n");
		}
	}else if(port==UART_PORT1)
	{
		if(numOfDataInReadBuffer1<TEST_SIZE)
		{
			rear_ReadBuffer1=(rear_ReadBuffer1+1)%TEST_SIZE;
			ReadBuffer1[rear_ReadBuffer1]=data;
			numOfDataInReadBuffer1=(numOfDataInReadBuffer1+1);
		}
		else
		{
			DBG_PRINTF("\nReadBuffer1 is FULL\n");
		}
	}

	return IsFull_ReadBuffer(port);
}

UINT8 deleteq_ReadBuffer(int port)
{

/*
Bit 0 : RX buffer has data
Bit 1 : TX buffer is empty
*/	
	if(port==UART_PORT0
	){
		if(numOfDataInReadBuffer0>0)
		{
			front_ReadBuffer0=((front_ReadBuffer0+1)%TEST_SIZE);
			numOfDataInReadBuffer0=(numOfDataInReadBuffer0-1);
			tmp_rx_cnt++;
			return ReadBuffer0[front_ReadBuffer0];
		}
		else
		{
			rear_ReadBuffer0=-1;
			front_ReadBuffer0=-1;
			DBG_PRINTF("\nReadBuffer is NULL\n");
			return IsEmpty_ReadBuffer(port);
		}	
		
	}
	else if(port==UART_PORT1)
	{
		if(numOfDataInReadBuffer1>0)
		{
			front_ReadBuffer1=((front_ReadBuffer1+1)%TEST_SIZE);
			numOfDataInReadBuffer1=(numOfDataInReadBuffer1-1);
			tmp_rx_cnt++;
			return ReadBuffer1[front_ReadBuffer1];
		}
		else
		{
			rear_ReadBuffer1=-1;
			front_ReadBuffer1=-1;
			DBG_PRINTF("\nReadBuffer1 is NULL\n");
			return IsEmpty_ReadBuffer(port);
		}	
	}
	return 0;
}
/****************************************************/
int IsFull_WriteBuffer(int port)
{
	if(port==UART_PORT0)
	{
		return(rear_WriteBuffer0==(TEST_SIZE-1))?1:0;
	}
	else if(port==UART_PORT1)
	{
		return(rear_WriteBuffer1==(TEST_SIZE-1))?1:0;
	}
	return 0;
}

int IsEmpty_WriteBuffer(int port)
{
	if(port==UART_PORT0)
	{
		return (front_WriteBuffer0==rear_WriteBuffer0)?1:0;
	}
	else if(port==UART_PORT1)
	{
		return (front_WriteBuffer1==rear_WriteBuffer1)?1:0;
	}
	return 0;
}

void clr_ReadBuffer(int port)
{
	if(port==UART_PORT0)
	{
		front_ReadBuffer0=-1;
		rear_ReadBuffer0=-1;
		numOfDataInReadBuffer0=0;
	}
	else if(port==UART_PORT1)
	{
		front_ReadBuffer1=-1;
		rear_ReadBuffer1=-1;
		numOfDataInReadBuffer1=0;
	}
	else if(port==UART_ALL)
	{
		front_ReadBuffer0=-1;
		rear_ReadBuffer0=-1;
		front_ReadBuffer1=-1;
		rear_ReadBuffer1=-1;
		numOfDataInReadBuffer0=0;
		numOfDataInReadBuffer1=0;
	}
}


void clr_WriteBuffer(int port)
{
	if(port==UART_PORT0)
	{
		front_WriteBuffer0=-1;
		rear_WriteBuffer0=-1;
		numOfDataInWriteBuffer0=0;
	}
	else if(port==UART_PORT1)
	{
		front_WriteBuffer1=-1;
		rear_WriteBuffer1=-1;
		numOfDataInWriteBuffer1=0;
	}
	else if(port==UART_ALL)
	{
		front_WriteBuffer0=-1;
		rear_WriteBuffer0=-1;
		front_WriteBuffer1=-1;
		rear_WriteBuffer1=-1;
		numOfDataInWriteBuffer0=0;
		numOfDataInWriteBuffer1=0;
	}
}
int addq_WriteBuffer(int port,UINT8 data)
{
	if(port==UART_PORT0)
	{
		if(numOfDataInWriteBuffer0<TEST_SIZE)
		{
			rear_WriteBuffer0=(rear_WriteBuffer0+1)%TEST_SIZE;
			WriteBuffer0[rear_WriteBuffer0]=data;
			numOfDataInWriteBuffer0=(numOfDataInWriteBuffer0+1);
			vu_cmd[CMD_SendDatatoUARTTXBuffer].count++;
	
			return WriteBuffer0[rear_WriteBuffer0];
		}
		else
		{
			DBG_PRINTF("\nWriteBuffer is FULL!!!!\n");
		}
	}
	else if(port==UART_PORT1)
	{
		if(numOfDataInWriteBuffer1<TEST_SIZE)
		{
			rear_WriteBuffer1=(rear_WriteBuffer1+1)%TEST_SIZE;
			WriteBuffer1[rear_WriteBuffer1]=data;
			numOfDataInWriteBuffer1=(numOfDataInWriteBuffer1+1);
			vu_cmd[CMD_SendDatatoUARTTXBuffer].count++;
			tmp_uart1_tx++;
			return WriteBuffer1[rear_WriteBuffer1];
		}
		else
		{
			DBG_PRINTF("\nWriteBuffer1 is FULL!!!!\n");
		}
	}
	return 0;
}

UINT8 deleteq_WriteBuffer(int port)
{
	if(port==UART_PORT0)
	{
		if(numOfDataInWriteBuffer0>0)
		{
			front_WriteBuffer0=((front_WriteBuffer0+1)%TEST_SIZE);
			numOfDataInWriteBuffer0=(numOfDataInWriteBuffer0-1);
			vu_cmd[CMD_SendDatatoUARTTXBuffer].count--;
			return WriteBuffer0[front_WriteBuffer0];
		}
		else
		{
			front_WriteBuffer0=-1;
			rear_WriteBuffer0=-1;
			DBG_PRINTF("\nWriteBuffer1 is NULL!!!\n");
			
			return 0;
		}	
	}
	else if(port==UART_PORT1)
	{
		if(numOfDataInWriteBuffer1>0)
		{
			front_WriteBuffer1=((front_WriteBuffer1+1)%TEST_SIZE);
			numOfDataInWriteBuffer1=(numOfDataInWriteBuffer1-1);
			vu_cmd[CMD_SendDatatoUARTTXBuffer].count--;
			return WriteBuffer1[front_WriteBuffer1];
		}
		else
		{
			front_WriteBuffer1=-1;
			rear_WriteBuffer1=-1;
			DBG_PRINTF("\nWriteBuffer1 is NULL!!!\n");
			return 0;
		}	

	}
	return 0;
}

void GPIO_GPAR_ENABLE()
{

		DBG_PRINTF("enable GPA0 as interrupt to notify (1)finish of TX (2)Data in buffer from controller\n");
		//set GPA0 input 'b0:input ; 'b1:output;
		outpw(REG_GPIOA_OMD,   inpw(REG_GPIOA_OMD & ( ~(0x1<<0) ) ));
		//set GPA0 Bit Pull-up Resistor Enable 'b0= disable ;'b1= enable
		outpw(REG_GPIOA_PUEN , inpw(REG_GPIOA_PUEN) | (0x1<<0) ); 
		
		/*
		IRQENGPA[31:16]:set GPA0~15 rising egde to trigger 
		IRQENGPA[15:0]:set GPA0~15 falling egde to trigger
		//set GPA4 falling edge trigger
		//outpw(REG_IRQENGPA , ((inpw(REG_IRQENGPA)&(~(0x1<<16<<4))) | ((0x1<<4))));
		*/
		//set GPA0 rising edge trigger
		outpw(REG_IRQENGPA , ((inpw(REG_IRQENGPA)&(~(0x1<<0))) | ((0x1<<16<<0))));
	
		//set GPA0 group IRQ0 interrupt 2'b00=IRQ0 2'b01=IRQ1 2'b10=IRQ2 2'b11=IRQ3
		outpw(REG_IRQSRCGPA , inpw(REG_IRQSRCGPA) & ~((0x3)<<0));
		//set GPA4 bounce(128, 1 << 0);
		/*
		Interrupt De-bounce Control
		DBNCECON[7:4] = DBCLKSEL
		4'd6 = sample interrupt input once per 64 clock
		4'd7 = sample interrupt input once per 128 clock
		4'd8 = sample interrupt input once per 2^8=256 clock
		DBNCECON[3:0] = DBEN
		de-bounce sampling enable for each IRQx, x = 0 ~ 3
		'b0=disable , 1'b1=enable
		*/
		outpw(REG_DBNCECON , (((0x7 << 0)| 0x0)));
	

		/*
		AIC Source Control Registers:AIC_SCR1~ AIC_SCR8
		IRQ_EXTINT0 = GPIO_INT0 -->AIC_SCR1[23:16]
		AIC_SCR1[23:22] = TYPE
			2'b00: low-active level triggered
			2'b01: high-active level triggered
			2'b10: low-active egde triggered
			2'b11: hiegh-active edge triggered
		AIC_SCR1[21:19] = RESERVED
		AIC_SCR1[18:16] = PRIORITY
			level 0 have the highest priority and the priority
			level 7 has the lowest.
		*/
		sysInstallISR(IRQ_LEVEL_7,IRQ_EXTINT0, (PVOID)gpio_isr);
		//sysSetInterruptType(IRQ_EXTINT0, HIGH_LEVEL_SENSITIVE);
		sysSetLocalInterrupt(ENABLE_IRQ);
		sysEnableInterrupt(IRQ_EXTINT0);
}
