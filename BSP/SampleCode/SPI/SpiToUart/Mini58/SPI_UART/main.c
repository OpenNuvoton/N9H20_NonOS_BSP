
#include <stdio.h>
#include "Mini58Series.h"

/******************************************************************************
 * @file     main.c
 * @version  V0.10
 * $Revision: 2 $
 * $Date: 15/05/27 2:06p $
 * @brief    Demonstrate SPI(N9H20) to Uart(mini58) 
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include "Mini58Series.h"

#define PLLCON_SETTING      SYSCLK_PLLCON_50MHz_HIRC
#define PLL_CLOCK           50000000
void SYS_Init(void);
void UART_Init(int UART_PORT);
void SPI_Init(void);
void cal_out_checksum(unsigned char len);
int cal_in_checksum(unsigned char len,volatile unsigned char *ptr);
void SPI_SLAVE_SEND(unsigned char length);
void SPI_RETURN_PACKAGE(unsigned char length,unsigned char command, unsigned char report);
void SPI_Command_Parser(void);
void gpio_INT(void);
void UART_Receive(char UART_PORT);
void UART_Transmit(char UART_PORT);
//void UART1_Receive(void);
//void UART1_Transmit(void);
void UART_clr_ReadBuffer(char UART_PORT);
void UART_clr_WriteBuffer(char UART_PORT);

int UART_RX_overflow(char UART_PORT);
//int UART1_RX_overflow(void);
void _SPI_SET_DATA_WIDTH(SPI_T *spi, uint32_t u32Width);
void CMD_32BITS_DIVIDEINTO_8BITS(int *buf,int cnt);
void SPI_Receive(void);

#define RXBUFSIZE 64
#define BUFSIZE 64
#define MASTER_DATA_SIZE 0xa

volatile unsigned char  SPI_IN_COMMAND[RXBUFSIZE]={0};
volatile unsigned char  SPI_OUT_COMMAND[RXBUFSIZE]={0};
volatile int com0Rbytes = 0;
volatile int com0Tbytes = 0;
volatile int com1Rbytes = 0;
volatile int com1Tbytes = 0;

volatile unsigned char  UART0_IN_BUFFER[RXBUFSIZE]={0};
volatile unsigned char  UART0_OUT_BUFFER[RXBUFSIZE]={0};
volatile unsigned char  UART1_IN_BUFFER[RXBUFSIZE]={0};
volatile unsigned char  UART1_OUT_BUFFER[RXBUFSIZE]={0};


volatile unsigned int total_cnt=0;
volatile unsigned int total_cnt_write=0;

volatile unsigned char  in_checksum=0;
volatile unsigned char  out_checksum=0;

volatile unsigned char  m=0;
volatile unsigned int tmp_baudrate=0;
volatile unsigned char SPI_Send_Flag=0;
volatile unsigned char SOFTWARE_FLOW_CONTROL_Flag=0;
volatile unsigned char SOFTWARE_Notification_Flag=0;
volatile unsigned char tmp_SOFTWARE_Notification_Flag=0;
volatile unsigned char SPI_IN_COUNT=0;
volatile unsigned char SPI_OUT_COUNT=0;
volatile unsigned int  temp_count=0;
volatile unsigned char temp_val=0;
volatile unsigned char tmp_uart0_cnt=0;
volatile unsigned char tmp_over_BUFSIZE=0;
volatile unsigned char Receive_UART0_FIFO = 15;
volatile unsigned char SPI_BSY=0;
volatile unsigned char main_bsy=0;
volatile unsigned char event=0;
volatile unsigned char FLAG_SPI_CMD_PARSER=0;
volatile unsigned char FLAG_UART0_RECEIVE=0;
volatile unsigned char FLAG_UART1_RECEIVE=0;
volatile unsigned char FLAG_GPIO_INT=0;
volatile unsigned char FLAG_UART0_TRANSMIT=0;
volatile unsigned char FLAG_UART1_TRANSMIT=0;
volatile unsigned char FLAG_UART0_RX_OVERFLOW=0;
volatile unsigned char FLAG_UART1_RX_OVERFLOW=0;
volatile unsigned char FLAG_SPI_Receive=0;
volatile unsigned char FLAG_RECEIVE_C=0;
volatile unsigned char tmp_SPI_IN_COUNT=0;
volatile unsigned char NUM_SPI_IN_32BIT=0;
volatile unsigned char LAST_CMD=0;

volatile unsigned char SPI_TANSIST_COUNT=0;
volatile unsigned char SPI_TANSIST_TOTAL=0;
volatile unsigned char FLAG_SPI_RESPONSE=0;
volatile unsigned char FLAG_MASTER_RX_FULL=0;
volatile unsigned char FLAG_SLAVE_RX_ISNOT_EMPTY=0;
volatile unsigned char FLAVG_RX_LEVEAL_PRIORITY='R';
volatile unsigned char FLAG_SPI_RX_HAVE_RECEIVED=1;
volatile unsigned char  FLAG_UART0_FIFO_TXOVIF=0;
volatile unsigned char  FLAG_UART1_FIFO_TXOVIF=1;
volatile unsigned char  FLAG_UART_CLOSE=0;
volatile unsigned char TMP_LENGTH=0;
volatile unsigned char TMP_INT_FALG=0;
volatile unsigned char TMP_CMD_NUM=0;
volatile int tmp_cnt_tx_tx=0;
volatile int tmp_cnt_int_delay=0;
volatile unsigned char FLAG_ENABLE_UART_PORT=0xff;
int  CMD_32bit[8];
int tmp_quertrx=0;
volatile int tmp_cnt_int=0;
volatile int tmp_response_int=0;
volatile unsigned int tmp_FIFO_TABLE=0;
volatile unsigned int tmp_FIFO_TABLE2=0;
volatile unsigned int tmp_function_tx=0;
volatile unsigned int tmp_function_rx=0;
#define UINT8 unsigned char

//uart command
#define RESET_UART 0
#define RESET_UART_Length 2

#define CONFIG_UART 1
#define CONFIG_UART_Length 9

#define HW_Flow_Control 2
#define HW_Flow_Control_Length 2

#define Notification 3
#define Notification_Length 2

#define Query_TX_buffer 4
#define Query_TX_buffer_Length 2

#define Send_TX_buffer 5


#define Query_RX_buffer 6
#define Query_RX_buffer_Length 2

#define Receive_RX_buffer 7

#define Get_Notification 8
#define Get_Notification_Length 1

#define CLOSE_UART 9
#define Get_CLOSE_UART 2

#define OPEN_UART 0xA
#define OPEN_UART_LENGTH 2

#define CLEAR_BUF 0xB
#define CLEAR_BUF_LENGTH 2


//const define
#define C_CMD 0x03
#define C_Length 0x02
#define C_CHECKSUM 0x01
#define OK 0x0
#define CRS 2 //COMMON RETURN SIZE
#define PORT 4
#define FLAG 4
#define UART_PORT_0 0
#define UART_PORT_1 1
#define ALL_UART 0xFF
#define ALL_UART_0 2
#define ALL_UART_1 3
#define Master_RX_BUF_FULL 0x6
#define SLAVE_RX_ISNOT_EMPTY 0x6


#define Clear_COMMAND SPI_OUT_COUNT=0;//SSPI_IN_COUNT=0;SPI_OUT_COUNT=0;SPI_IN_COMMAND[C_Length]=0;
#define UART_RXPTR(uart)    ((uart->FIFOSTS & UART_FIFOSTS_RXPTR_Msk)>>UART_FIFOSTS_RXPTR_Pos)
#define UART_TXPTR(uart)    ((uart->FIFOSTS & UART_FIFOSTS_TXPTR_Msk)>>UART_FIFOSTS_TXPTR_Pos)
#define SW_INT P32
#define test_gpio P15


#define UART0_FIFO_RXOVIF(uart) ((uart->FIFOSTS & (UART_FIFOSTS_RXOVIF_Msk)))
#define UART0_FIFO_TXOVIF(uart) ((uart->FIFOSTS & (UART_FIFOSTS_TXFULL_Msk))>>UART_FIFOSTS_TXFULL_Pos)
#define UART0_FIFO_RXOVIF_clr(uart) ((uart->FIFOSTS | (UART_FIFOSTS_RXOVIF_Msk)))
#define UART0_FIFO(uart) ((uart->FIFOSTS))
#define SPI_CHIP_SELECT ((GPIO_GET_IN_DATA(P0)&0x10)>>4)
#define UART_FIFO_RXRST(uart) (uart->FIFO|(UART_FIFO_RXRST_Msk))
#define UART_FIFO_TXRST(uart) (uart->FIFO|(UART_FIFO_TXRST_Msk))

#define SPI_CMD_PARSER 1
#define UART0_RECEIVE 2
#define GPIO_INT 3
#define UART0_TRANSMIT 4
#define UART0_RX_OVERFLOW 5


 volatile int UART0_rear_ReadBuffer=-1;
 volatile int UART0_front_ReadBuffer=-1;
 volatile int UART0_rear_WriteBuffer=-1;
 volatile int UART0_front_WriteBuffer=-1;
 
UINT8 UART_addq_WriteBuffer(char UART_PORT,UINT8 data);
UINT8 UART_deleteq_WriteBuffer(char UART_PORT);
UINT8 UART_addq_ReadBuffer(char UART_PORT,UINT8 data);
UINT8 UART_deleteq_ReadBuffer(char UART_PORT);

 
 volatile int UART1_rear_ReadBuffer=-1;
 volatile int UART1_front_ReadBuffer=-1;
 volatile int UART1_rear_WriteBuffer=-1;
 volatile int UART1_front_WriteBuffer=-1;
 




void SYS_Init(void)
{
  /* Unlock protected registers */
    SYS_UnlockReg();

    /* Read User Config to select internal high speed RC */
    //SystemInit();
	
	  CLK->PWRCTL &= ~CLK_PWRCTL_XTLEN_Msk; //[1:0]00 = XT1_IN and XT1_OUT are GPIO, disable both LXT & HXT (default).
    CLK->PWRCTL |= CLK_PWRCTL_HIRCEN_Msk; // HIRC Enabled
	  CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);
    /* Waiting for 12MHz clock ready */
    while(!(CLK->STATUS & CLK_STATUS_HIRCSTB_Msk));

#ifdef CLK_SOURCE_HIRC
    /* Disable External XTAL (4~24 MHz) */
  
   /* Switch HCLK clock source to HIRC clock for safe */
    CLK->CLKSEL0 &= ~CLK_CLKSEL0_HCLKSEL_Msk;
		CLK->CLKSEL0 |= CLK_CLKSEL0_HCLKSEL_HIRC;
	  CLK->CLKDIV &= (~CLK_CLKDIV_HCLKDIV_Msk);
		

#else //CLK_SOURCE_PLL
 		CLK->CLKSEL0 &= ~CLK_CLKSEL0_HCLKSEL_Msk;
		CLK->CLKSEL0 |= CLK_CLKSEL0_HCLKSEL_PLL;
		CLK_SetCoreClock(PLL_CLOCK);
 
#endif /* CLK_SOURCE_HIRC*/


    /* Enable IP clock */
    CLK->APBCLK |= CLK_APBCLK_UART0CKEN_Msk; // UART0 Clock Enable
    CLK->APBCLK |= CLK_APBCLK_UART1CKEN_Msk; // UART1 Clock Enable

    /* Select IP clock source */
    CLK->CLKSEL1 &= ~CLK_CLKSEL1_UARTSEL_Msk;
    CLK->CLKSEL1 = CLK->CLKSEL1|(0x2 << 24);// Clock source from HIRC

		CLK->CLKSEL1 &= ~CLK_CLKSEL1_SPISEL_Msk;
		CLK->CLKSEL1 |= (0x1 << CLK_CLKSEL1_SPISEL_Pos); // Clock source from HCLK
		
  		/*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set P1 multi-function pins for UART0 RXD, TXD */
		SYS->P5_MFP|=(0x000000303)  ;//SYS_MFP_P50_UART0_TXD//SYS_MFP_P51_UART0_RXD
		
		/* Set P2 multi-function pins for UART1 RXD, TXD */
		SYS->P2_MFP &= ~(SYS_MFP_P24_Msk | SYS_MFP_P25_Msk);
    SYS->P2_MFP |= (SYS_MFP_P24_UART1_RXD | SYS_MFP_P25_UART1_TXD);


		/* Set P0 multi-function pins for SPI0 (MOSI,MISO,CLK,SS)*/
		SYS->P0_MFP &= ~(SYS_MFP_P04_Msk | SYS_MFP_P05_Msk| SYS_MFP_P06_Msk| SYS_MFP_P07_Msk);
		SYS->P0_MFP |= (SYS_MFP_P04_SPI0_SS | SYS_MFP_P05_SPI0_MOSI | SYS_MFP_P06_SPI0_MISO |SYS_MFP_P07_SPI0_CLK);
		
		/*Set P3_2 multi-function pins for SPI0 INT  */
		GPIO_SetMode(P3, BIT2, GPIO_MODE_OUTPUT);
		
    /* Lock protected registers */
    SYS->REGLCTL = 0;

    /* Update System Core Clock */
    SystemCoreClockUpdate();
}
int i =0;

void SPI_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init SPI                                                                                                */
    /*---------------------------------------------------------------------------------------------------------*/
    CLK->APBCLK |= CLK_APBCLK_SPICKEN_Msk;
    SPI->CTL = SPI_SLAVE | SPI_MODE_1;
		SPI_Open(SPI, SPI_SLAVE, SPI_MODE_1, 32, 16000000);
    SPI->CLKDIV = 0;


		SPI->SSCTL|=SPI_SSCTL_SSLTEN_Msk;
}
void UART_Init(int UART_PORT)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Reset IP */

	if(UART_PORT==UART_PORT_0){
		SYS_ResetModule(UART0_RST);
		UART_Open(UART0, 115200);
	}else if(UART_PORT==UART_PORT_0){
		SYS_ResetModule(UART1_RST);
		UART_Open(UART1, 115200);
	}else{
		SYS_ResetModule(UART0_RST);
		SYS_ResetModule(UART1_RST);
		UART_Open(UART0, 115200);
		UART_Open(UART1, 115200);	
	}
}

int main(void)
{
    uint32_t u32DataCount,i=0;
	
    /* Init System, IP clock and multi-function I/O */
    SYS_Init();

		SPI_Init();
		SPI_EnableFIFO(SPI0, 0, 0);
		SPI_EnableInt(SPI0, SPI_FIFO_RX_INTEN_MASK);
    NVIC_EnableIRQ(SPI_IRQn);
		UART_ENABLE_INT(UART0, UART_INTEN_RDAIEN_Msk);
		NVIC_EnableIRQ(UART0_IRQn);
		UART_ENABLE_INT(UART1, UART_INTEN_RDAIEN_Msk);
		NVIC_EnableIRQ(UART1_IRQn);
   	SW_INT =0;
 while(1){

		
			if(FLAG_SPI_Receive==1){			
				SPI_Command_Parser();		
			}
			if(FLAG_SPI_Receive==0){
				if(FLAG_ENABLE_UART_PORT==UART_PORT_0){
					
					if(FLAG_UART0_RECEIVE==1){
						if(FLAG_UART0_RX_OVERFLOW==1){
								UART_RX_overflow(UART_PORT_0);
						}else{
							UART_Receive(UART_PORT_0);	
							UART_ENABLE_INT(UART0, UART_INTEN_RDAIEN_Msk);
						}
						FLAG_UART0_RECEIVE=0;
					}
					if(FLAG_UART0_TRANSMIT==1){
						UART_Transmit(UART_PORT_0);
						FLAG_UART0_TRANSMIT=0;	
					}
		
					if(FLAVG_RX_LEVEAL_PRIORITY>'R'){

					}else{
						if(SPI_BSY==0&& FLAG_SPI_RESPONSE!=1 &&com0Rbytes!=0 && FLAG_MASTER_RX_FULL==0 && (SPI_CHIP_SELECT==1) ){
							gpio_INT();		
							TMP_INT_FALG=5;			
							tmp_cnt_int++;																
						}
					}
					if(FLAG_UART0_RX_OVERFLOW==1){UART_RX_overflow(UART_PORT_0);}
					if(UART0_FIFO_TXOVIF(UART0)==1)FLAG_UART0_FIFO_TXOVIF=1; else FLAG_UART0_FIFO_TXOVIF=0;
					if(com0Rbytes<(RXBUFSIZE)&&SPI_BSY==0 &&(FLAG_UART_CLOSE!=0x1)&& (((UART0->INTEN)&UART_INTEN_RDAIEN_Msk)==0))UART_ENABLE_INT(UART0, UART_INTEN_RDAIEN_Msk);
					if(com0Tbytes>0 && UART0_FIFO_TXOVIF(UART0)==0)FLAG_UART0_TRANSMIT=1;
					if(FLAG_ENABLE_UART_PORT==ALL_UART_0)FLAG_ENABLE_UART_PORT=ALL_UART;
				}else if(FLAG_ENABLE_UART_PORT==UART_PORT_1){
					if(FLAG_UART1_RECEIVE==1){
						if(FLAG_UART1_RX_OVERFLOW==1){
								UART_RX_overflow(UART_PORT_1);
						}else{
							UART_Receive(UART_PORT_1);	
							UART_ENABLE_INT(UART1, UART_INTEN_RDAIEN_Msk);
						}
						FLAG_UART1_RECEIVE=0;
					}
					if(FLAG_UART1_TRANSMIT==1){
						UART_Transmit(UART_PORT_1);
						FLAG_UART1_TRANSMIT=0;	
					}		

					if(FLAVG_RX_LEVEAL_PRIORITY>'R'){
						
					}else{
						if(SPI_BSY==0&& FLAG_SPI_RESPONSE!=1 &&com1Rbytes!=0 && FLAG_MASTER_RX_FULL==0 && (SPI_CHIP_SELECT==1) ){
							gpio_INT();		
							TMP_INT_FALG=4;			
							tmp_cnt_int++;																
						}
					}
	
					if(FLAG_UART1_RX_OVERFLOW==1){UART_RX_overflow(UART_PORT_1);}
					if(UART0_FIFO_TXOVIF(UART1)==1)FLAG_UART1_FIFO_TXOVIF=1; else FLAG_UART1_FIFO_TXOVIF=0;
					if(com1Rbytes<(RXBUFSIZE)&&SPI_BSY==0 &&(FLAG_UART_CLOSE!=0x2)&& (((UART1->INTEN)&UART_INTEN_RDAIEN_Msk)==0))UART_ENABLE_INT(UART1, UART_INTEN_RDAIEN_Msk);
					if(com1Tbytes>0 && UART0_FIFO_TXOVIF(UART1)==0)FLAG_UART1_TRANSMIT=1;
					if(FLAG_ENABLE_UART_PORT==ALL_UART_1)FLAG_ENABLE_UART_PORT=ALL_UART;
			  }else if(FLAG_ENABLE_UART_PORT==ALL_UART){
					
		
					if(FLAG_UART0_RECEIVE==1 || UART_RXPTR(UART0)>=8){
						if(FLAG_UART0_RX_OVERFLOW==1){
								UART_RX_overflow(UART_PORT_0);
						}else{
							UART_Receive(UART_PORT_0);	
							UART_ENABLE_INT(UART0, UART_INTEN_RDAIEN_Msk);
						}
							FLAG_UART0_RECEIVE=0;
					}					
					if(FLAG_UART1_RECEIVE==1){
						if(FLAG_UART1_RX_OVERFLOW==1){
								UART_RX_overflow(UART_PORT_1);
						}else{
							UART_Receive(UART_PORT_1);	
							UART_ENABLE_INT(UART1, UART_INTEN_RDAIEN_Msk);
						}
						FLAG_UART1_RECEIVE=0;
					}
					if(FLAG_UART1_TRANSMIT==1){
						UART_Transmit(UART_PORT_1);
						FLAG_UART1_TRANSMIT=0;	
					}	
					if(FLAG_UART0_TRANSMIT==1){
						UART_Transmit(UART_PORT_0);
						FLAG_UART0_TRANSMIT=0;	
					}		
					
					if(FLAVG_RX_LEVEAL_PRIORITY>'R'){
			
					}else{
						if(SPI_BSY==0&& FLAG_SPI_RESPONSE!=1&&(com0Rbytes+com1Rbytes>0) && FLAG_MASTER_RX_FULL==0 /*&& LAST_CMD!=Query_RX_buffer&& LAST_CMD!=Query_TX_buffer&& TMP_INT_FALG==1*/&& (SPI_CHIP_SELECT==1)){
								gpio_INT();
								tmp_cnt_int_delay=0;			
								TMP_INT_FALG=6;
								tmp_cnt_int++;		
						}
					}
	
					if(FLAG_UART1_RX_OVERFLOW==1){UART_RX_overflow(UART_PORT_1);}
					if(UART0_FIFO_TXOVIF(UART1)==1)FLAG_UART1_FIFO_TXOVIF=1; else FLAG_UART1_FIFO_TXOVIF=0;
					if(com1Rbytes<(RXBUFSIZE)&&SPI_BSY==0 &&(FLAG_UART_CLOSE!=0x3)&&(((UART1->INTEN)&UART_INTEN_RDAIEN_Msk)==0))UART_ENABLE_INT(UART1, UART_INTEN_RDAIEN_Msk);
					if(com1Tbytes>0 && UART0_FIFO_TXOVIF(UART1)==0)FLAG_UART1_TRANSMIT=1;
					if(FLAG_UART0_RX_OVERFLOW==1){UART_RX_overflow(UART_PORT_0);}
					if(UART0_FIFO_TXOVIF(UART0)==1)FLAG_UART0_FIFO_TXOVIF=1; else FLAG_UART0_FIFO_TXOVIF=0;
					if(com0Rbytes<(RXBUFSIZE)&&SPI_BSY==0 &&(FLAG_UART_CLOSE!=0x3)&& (((UART0->INTEN)&UART_INTEN_RDAIEN_Msk)==0))UART_ENABLE_INT(UART0, UART_INTEN_RDAIEN_Msk);
					if(com0Tbytes>0 && UART0_FIFO_TXOVIF(UART0)==0)FLAG_UART0_TRANSMIT=1;
				}//else if(FLAG_ENABLE_UART_PORT==ALL_UART)
			}//	if(FLAG_SPI_Receive==0)
	}//while(1)
}//main}

void cal_out_checksum(unsigned char len)
{
volatile unsigned char  i=0;	
out_checksum=0;
TMP_LENGTH=len;

//SPI_OUT_COMMAND[1]=0;//SPI_OUT_COMMAND checksum for return spi 
for(i=2;i<(2+len+1);i++) //2:leading+checksum 
{
out_checksum=out_checksum+SPI_OUT_COMMAND[i];
}

SPI_OUT_COMMAND[1]=out_checksum;//for return spi

}

int cal_in_checksum(unsigned char len,volatile unsigned char *ptr)
{
	volatile unsigned char  i=0;
	in_checksum=0;

	for(i=0;i<=len;i++) //2:leading+checksum 
	{
	in_checksum=in_checksum+*(ptr+i);
	}
	
	return in_checksum;
}

void SPI_SLAVE_SEND(unsigned char length)
{
	unsigned char temp,j,k;
	int *tmp;
	volatile unsigned char  i=0;
	
	while(FLAG_SPI_RX_HAVE_RECEIVED==0);
	FLAG_SPI_RESPONSE=1;
	SPI_ClearTxFIFO(SPI);
	j=0,k=0;

	SW_INT=0;
	
	length=(length+3)/4;
	SPI_TANSIST_TOTAL=length;
	if(SPI_TANSIST_TOTAL<=0 || length>4){
		length=0;
	}
	tmp=(int*) SPI_OUT_COMMAND;

	

	FLAG_SPI_RX_HAVE_RECEIVED=0;
	while(length>0){
		
		if(length<=4){
			for(i=0;i<length;i++){
				while(SPI_GET_TX_FIFO_FULL_FLAG(SPI)==1);
				SPI_WRITE_TX(SPI,*(tmp+j));	
				j++;
			}
			length=length-length;
		}else{
			length=0xff;//error
		}
		//while(FLAG_SPI_RX_HAVE_RECEIVED!=1);
		while(((GPIO_GET_IN_DATA(P0)&0x10)>>4)==0); 
		SW_INT=1;
		tmp_response_int++;
		while(((GPIO_GET_IN_DATA(P0)&0x10)>>4)==0);  //CS=0 --> master is fetching
		//test_gpio=1;
		SW_INT=0;
		TMP_INT_FALG=1;
		//test_gpio=0;
		
	}
	while(SPI_GET_TX_FIFO_EMPTY_FLAG(SPI)==0);
	while(FLAG_SPI_RX_HAVE_RECEIVED==0);
}


void SPI_RETURN_PACKAGE(unsigned char length,unsigned char command, unsigned char report)
{
		if(com0Rbytes>=(RXBUFSIZE) && com1Rbytes<(RXBUFSIZE)){
			SPI_OUT_COMMAND[0]='S';
			//FLAVG_RX_LEVEAL_PRIORITY++;
		}else if(com0Rbytes<(RXBUFSIZE)&&com1Rbytes>=(RXBUFSIZE)){
			SPI_OUT_COMMAND[0]='T';
		}else if(com0Rbytes>=(RXBUFSIZE)&&com1Rbytes>=(RXBUFSIZE)){
			SPI_OUT_COMMAND[0]='U';
		}else{
			SPI_OUT_COMMAND[0]='R';
		}
		
    SPI_OUT_COMMAND[2]=length;
    SPI_OUT_COMMAND[3]=command;
    SPI_OUT_COMMAND[4]=report;
}





void SPI_Command_Parser(void)
{
	
	volatile unsigned char  i=0,j=0;
	if(SPI_IN_COMMAND[C_CMD]==OPEN_UART){
		 if(SPI_IN_COMMAND[PORT]==UART_PORT_0){
			UART_Init(UART_PORT_0);
			UART_ENABLE_INT(UART0, UART_INTEN_RDAIEN_Msk);
			com0Rbytes=0;
			com0Tbytes=0;
			TMP_CMD_NUM=0;
			FLAG_ENABLE_UART_PORT=UART_PORT_0;
			FLAG_UART_CLOSE=FLAG_UART_CLOSE&0x2;
		}else if(SPI_IN_COMMAND[PORT]==UART_PORT_1){
			UART_Init(UART_PORT_1);
			UART_ENABLE_INT(UART1, UART_INTEN_RDAIEN_Msk);
			com1Rbytes=0;
			com1Tbytes=0;
			FLAG_ENABLE_UART_PORT=UART_PORT_1;
			TMP_CMD_NUM=2;
			FLAG_UART_CLOSE=FLAG_UART_CLOSE&0x1;
		}else if(SPI_IN_COMMAND[PORT]==ALL_UART){
			UART_Init(ALL_UART);
			com0Rbytes=0;
			com0Tbytes=0;
			com1Rbytes=0;
			com1Tbytes=0;
			UART_ENABLE_INT(UART0, UART_INTEN_RDAIEN_Msk);
			UART_ENABLE_INT(UART1, UART_INTEN_RDAIEN_Msk);
			FLAG_ENABLE_UART_PORT=ALL_UART;
			TMP_CMD_NUM=3;
			FLAG_UART_CLOSE=0;
		}
		
		
		SPI_RETURN_PACKAGE(CRS,OPEN_UART,OK); 	
		cal_out_checksum(2);	 	 		
		SPI_SLAVE_SEND(5);
		Clear_COMMAND;
		//FLAG_SPI_Receive=0;
		
	}else if(SPI_IN_COMMAND[C_CMD]==RESET_UART){
		if(SPI_IN_COMMAND[PORT]==UART_PORT_0){
			com0Rbytes=0;
			com0Tbytes=0;
			UART_clr_WriteBuffer(UART_PORT_0);
			UART_clr_ReadBuffer(UART_PORT_0);
			UART_Init(UART_PORT_0);
			UART_FIFO_RXRST(UART0);
			UART_FIFO_TXRST(UART0);
		}else if(SPI_IN_COMMAND[PORT]==UART_PORT_1){
			com1Rbytes=0;
			com1Tbytes=0;
			UART_clr_WriteBuffer(UART_PORT_1);
			UART_clr_ReadBuffer(UART_PORT_1);
			UART_Init(UART_PORT_1);
			UART_FIFO_RXRST(UART1);
			UART_FIFO_TXRST(UART1);
		}else if(SPI_IN_COMMAND[PORT]==ALL_UART){
			com0Rbytes=0;
			com0Tbytes=0;
			com1Rbytes=0;
			com1Tbytes=0;
			UART_clr_WriteBuffer(UART_PORT_0);
			UART_clr_ReadBuffer(UART_PORT_0);				
			UART_clr_WriteBuffer(UART_PORT_1);
			UART_clr_ReadBuffer(UART_PORT_1);
			UART_Init(ALL_UART);
			UART_FIFO_RXRST(UART0);
			UART_FIFO_RXRST(UART1);
			UART_FIFO_TXRST(UART0);
			UART_FIFO_TXRST(UART);
		}else if(SPI_IN_COMMAND[PORT]==0xfe){ //SPI reset
			SPI_Init();
			SPI_EnableFIFO(SPI0, 0, 0);
			SPI_EnableInt(SPI0, SPI_FIFO_RX_INTEN_MASK);
			
		}

		SPI_RETURN_PACKAGE(CRS,RESET_UART,OK); 	
		cal_out_checksum(2);	 	 		
		SPI_SLAVE_SEND(5);
		Clear_COMMAND;
	}else if(SPI_IN_COMMAND[C_CMD]==CONFIG_UART){
		tmp_baudrate=	(((unsigned int)SPI_IN_COMMAND[5])+
		(((unsigned int)SPI_IN_COMMAND[6])<<8)+
		(((unsigned int)SPI_IN_COMMAND[7])<<(8*2))+
		(((unsigned int)SPI_IN_COMMAND[8])<<(8*3)));
		
		if(SPI_IN_COMMAND[PORT]==UART_PORT_0){       
			UART_Open(UART0, tmp_baudrate);	
		}else if(SPI_IN_COMMAND[PORT]==UART_PORT_1){
			UART_Open(UART1, tmp_baudrate);	
		}
	
		SPI_RETURN_PACKAGE(CRS,CONFIG_UART,OK);
		cal_out_checksum(2);	 	 
		SPI_SLAVE_SEND(5);
		TMP_CMD_NUM=4;
		Clear_COMMAND;
		//FLAG_SPI_Receive=0;
	}else if(SPI_IN_COMMAND[C_CMD]==Notification){    
		//Enable/Disable UART Notification
		if(SPI_IN_COMMAND[FLAG]==0){
			SOFTWARE_Notification_Flag=0;	
			tmp_SOFTWARE_Notification_Flag=0;
			FLAG_GPIO_INT=0;
		}else{
			SOFTWARE_Notification_Flag=1;
			tmp_SOFTWARE_Notification_Flag=1;
			FLAG_GPIO_INT=1;
		}
		//return Status packet	
		SPI_RETURN_PACKAGE(CRS,Notification,OK);
		cal_out_checksum(2);	 	 
		SPI_SLAVE_SEND(5);
		TMP_CMD_NUM=5;
		Clear_COMMAND;
		//FLAG_SPI_Receive=0;
	}else if(SPI_IN_COMMAND[C_CMD]== Query_TX_buffer){
			//Query UART TX buffer
	 if(SPI_IN_COMMAND[PORT]==UART_PORT_0 || SPI_IN_COMMAND[PORT]==UART_PORT_1 || SPI_IN_COMMAND[PORT]==ALL_UART){
			SPI_OUT_COMMAND[0]=(com0Rbytes>RXBUFSIZE&&com1Rbytes>RXBUFSIZE)?'V':com1Rbytes>RXBUFSIZE? 'T': com0Rbytes>RXBUFSIZE?'S':'R';
			SPI_OUT_COMMAND[2]=3;
			SPI_OUT_COMMAND[3]=Query_TX_buffer;
			SPI_OUT_COMMAND[4]=(RXBUFSIZE-(com0Tbytes));
			SPI_OUT_COMMAND[5]=(RXBUFSIZE-(com1Tbytes));
		}
			/*if(FLAG_ENABLE_UART_PORT==ALL_UART){
				if(SPI_IN_COMMAND[PORT]==UART_PORT_0)FLAG_ENABLE_UART_PORT=ALL_UART_0;
				else if(SPI_IN_COMMAND[PORT]==UART_PORT_1)FLAG_ENABLE_UART_PORT=ALL_UART_1;
			}*/
			cal_out_checksum(3);	 	 
			SPI_SLAVE_SEND(6);
			TMP_CMD_NUM=7;
			TMP_CMD_NUM=6;
			Clear_COMMAND;
		
	}else if(SPI_IN_COMMAND[C_CMD]== Send_TX_buffer){
		if(SPI_IN_COMMAND[PORT]==UART_PORT_0){
			for(i=5;i<(5+SPI_IN_COMMAND[C_Length]-2);i++){
				FLAG_UART0_TRANSMIT=1;
				UART_addq_WriteBuffer(UART_PORT_0,SPI_IN_COMMAND[i]);	
				UART_Transmit(UART_PORT_0);
			}
		
			TMP_CMD_NUM=8;
		}else if(SPI_IN_COMMAND[PORT]==UART_PORT_1){
			for(i=5;i<(5+SPI_IN_COMMAND[C_Length]-2);i++)	{
					UART_addq_WriteBuffer(UART_PORT_1,SPI_IN_COMMAND[i]);	
					FLAG_UART1_TRANSMIT=1;
					UART_Transmit(UART_PORT_1);
			}
			TMP_CMD_NUM=9;
		}else if(SPI_IN_COMMAND[PORT]==ALL_UART){
			for(i=0;i<((SPI_IN_COMMAND[C_Length]&0xf)-2);i++)	{
					UART_addq_WriteBuffer(UART_PORT_0,SPI_IN_COMMAND[5+i]);	
					FLAG_UART0_TRANSMIT=1;
					UART_Transmit(UART_PORT_0);
			}
			for(i=0;i<((SPI_IN_COMMAND[C_Length]&0xf0)>>4);i++){
					UART_addq_WriteBuffer(UART_PORT_1,SPI_IN_COMMAND[10+i]);	
					FLAG_UART1_TRANSMIT=1;
					UART_Transmit(UART_PORT_1);
			}
			TMP_CMD_NUM=19;
		}
		SPI_RETURN_PACKAGE(CRS,Send_TX_buffer,OK);
		cal_out_checksum(2);	 	 
		SPI_SLAVE_SEND(5);
		Clear_COMMAND;
		//FLAG_SPI_Receive=0;

	}else if(SPI_IN_COMMAND[C_CMD]==Query_RX_buffer){

		if(SPI_IN_COMMAND[PORT]==UART_PORT_0 || SPI_IN_COMMAND[PORT]==UART_PORT_1 || SPI_IN_COMMAND[PORT]==ALL_UART){
			SPI_OUT_COMMAND[0]=(com0Rbytes>RXBUFSIZE&&com1Rbytes>RXBUFSIZE)?'U':com1Rbytes>RXBUFSIZE? 'T': com0Rbytes>RXBUFSIZE?'S':'R';
			SPI_OUT_COMMAND[2]=3;
			SPI_OUT_COMMAND[3]=Query_RX_buffer;
			SPI_OUT_COMMAND[4]=com0Rbytes;
			SPI_OUT_COMMAND[5]=com1Rbytes;
		}
		
			FLAVG_RX_LEVEAL_PRIORITY='R';
			cal_out_checksum(3);	 	 
			SPI_SLAVE_SEND(6);
			TMP_CMD_NUM=10;
			Clear_COMMAND;
			//FLAG_SPI_Receive=0;
	}else if(SPI_IN_COMMAND[C_CMD]==Receive_RX_buffer){
		if(SPI_IN_COMMAND[5]>MASTER_DATA_SIZE)FLAG_MASTER_RX_FULL=1;
			if(SPI_IN_COMMAND[PORT]==UART_PORT_0){
				if(com0Rbytes==0){
						SPI_OUT_COMMAND[0]=com0Rbytes>=RXBUFSIZE?'S':'R';
						SPI_OUT_COMMAND[1]=8;
						SPI_OUT_COMMAND[2]=1;
						SPI_OUT_COMMAND[3]=7;
						SPI_SLAVE_SEND(4);
					TMP_CMD_NUM=11;
				}else{
					if(SPI_IN_COMMAND[C_Length]==4){ //MAX
						SPI_OUT_COMMAND[0]='R'; 
						SPI_OUT_COMMAND[2]=SPI_IN_COMMAND[5]+1; 
						SPI_OUT_COMMAND[3]=7; //cmd
						for(i=4;i<4+SPI_IN_COMMAND[5];i++){
							SPI_OUT_COMMAND[i]= UART_deleteq_ReadBuffer(UART_PORT_0);				
						}
						TMP_CMD_NUM=12;					
						cal_out_checksum(SPI_OUT_COMMAND[2]);         
						SPI_SLAVE_SEND(4+SPI_IN_COMMAND[5]);	
					
					}//if(SPI_IN_COMMAND[C_Length]==4)			
				}//if(com0Rbytes==0)
			}else if(SPI_IN_COMMAND[PORT]==UART_PORT_1){
				if(com1Rbytes==0){
						SPI_OUT_COMMAND[0]=com1Rbytes>=RXBUFSIZE?'T':'R';
						SPI_OUT_COMMAND[1]=8;
						SPI_OUT_COMMAND[2]=1;
						SPI_OUT_COMMAND[3]=7;
						SPI_SLAVE_SEND(4);
					TMP_CMD_NUM=13;
				}else{
					if(SPI_IN_COMMAND[C_Length]==4){ //MAX
						SPI_OUT_COMMAND[0]= 'R';
						SPI_OUT_COMMAND[2]=SPI_IN_COMMAND[5]+1; 
						SPI_OUT_COMMAND[3]=7; //cmd
						for(i=4;i<4+SPI_IN_COMMAND[5];i++){
							SPI_OUT_COMMAND[i]= UART_deleteq_ReadBuffer(UART_PORT_1);	
						}
						TMP_CMD_NUM=14;
						cal_out_checksum(SPI_OUT_COMMAND[2]);         
						SPI_SLAVE_SEND(4+SPI_IN_COMMAND[5]);	
					TMP_CMD_NUM=15;
					}//if(SPI_IN_COMMAND[C_Length]==4)			
				}//if(com1Rbytes==0)
			}else	if(SPI_IN_COMMAND[PORT]==ALL_UART){
				if(com0Rbytes==0 && com1Rbytes==0){
						SPI_OUT_COMMAND[0]='R';
						SPI_OUT_COMMAND[1]=8;
						SPI_OUT_COMMAND[2]=1;
						SPI_OUT_COMMAND[3]=7;
						SPI_SLAVE_SEND(4);
					TMP_CMD_NUM=16;
				}else{
					if(SPI_IN_COMMAND[C_Length]==4){ //MAX
						SPI_OUT_COMMAND[0]=(com0Rbytes>RXBUFSIZE&&com1Rbytes>RXBUFSIZE)?'U':com1Rbytes>RXBUFSIZE? 'T': com0Rbytes>RXBUFSIZE?'S':'R'; 
						SPI_OUT_COMMAND[2]=((SPI_IN_COMMAND[6]<<4)|SPI_IN_COMMAND[5]); 
						SPI_OUT_COMMAND[3]=7; //cmd
						for(i=4;i<4+SPI_IN_COMMAND[5];i++){
							SPI_OUT_COMMAND[i]= UART_deleteq_ReadBuffer(UART_PORT_0);	
							j++;
						}
						for(i=9;i<9+SPI_IN_COMMAND[6];i++){
							SPI_OUT_COMMAND[i]= UART_deleteq_ReadBuffer(UART_PORT_1);
							j++;							
						}
						cal_out_checksum(2+MASTER_DATA_SIZE);         
						SPI_SLAVE_SEND(4+MASTER_DATA_SIZE);	
					TMP_CMD_NUM=17;
					}//if(SPI_IN_COMMAND[C_Length]==4)			
				}//if(com1Rbytes==0)
			}//lse	if(SPI_IN_COMMAND[PORT]==ALL_UART)
		Clear_COMMAND;
		//FLAG_SPI_Receive=0;
	}else if(SPI_IN_COMMAND[C_CMD]==CLOSE_UART){

		if(SPI_IN_COMMAND[PORT]==UART_PORT_0){
			UART_DISABLE_INT(UART0, UART_INTEN_RDAIEN_Msk);
			FLAG_UART_CLOSE=0x1;
		}else if(SPI_IN_COMMAND[PORT]==UART_PORT_1){
			UART_DISABLE_INT(UART1, UART_INTEN_RDAIEN_Msk);
			FLAG_UART_CLOSE=0x2;
		}else if(SPI_IN_COMMAND[PORT]==ALL_UART){
			UART_DISABLE_INT(UART0, UART_INTEN_RDAIEN_Msk);
			UART_DISABLE_INT(UART1, UART_INTEN_RDAIEN_Msk);
			FLAG_UART_CLOSE=0x3;
		}
		
		SPI_RETURN_PACKAGE(CRS,CLOSE_UART,OK); 	
		cal_out_checksum(2);	 	 		
		SPI_SLAVE_SEND(5);
		Clear_COMMAND;
		TMP_CMD_NUM=11;
		Clear_COMMAND;
			//FLAG_SPI_Receive=0;
	}else if(SPI_IN_COMMAND[C_CMD]==CLEAR_BUF){

		if(SPI_IN_COMMAND[PORT]==UART_PORT_0){
			com0Rbytes=0;
			com0Tbytes=0;
			UART_clr_WriteBuffer(UART_PORT_0);
			UART_clr_ReadBuffer(UART_PORT_0);		
			UART_FIFO_RXRST(UART0);
		}else if(SPI_IN_COMMAND[PORT]==UART_PORT_1){
			com1Rbytes=0;
			com1Tbytes=0;
			UART_clr_WriteBuffer(UART_PORT_1);
			UART_clr_ReadBuffer(UART_PORT_1);
			UART_FIFO_RXRST(UART1);
		}else if(SPI_IN_COMMAND[PORT]==ALL_UART){
			com0Rbytes=0;
			com0Tbytes=0;
			com1Rbytes=0;
			com1Tbytes=0;
			UART_clr_WriteBuffer(UART_PORT_0);
			UART_clr_ReadBuffer(UART_PORT_0);				
			UART_clr_WriteBuffer(UART_PORT_1);
			UART_clr_ReadBuffer(UART_PORT_1);
			UART_FIFO_RXRST(UART0);
			UART_FIFO_RXRST(UART1);
		}
		
		SPI_RETURN_PACKAGE(CRS,CLEAR_BUF,OK); 	
		cal_out_checksum(2);	 	 		
		SPI_SLAVE_SEND(5);
		Clear_COMMAND;
		TMP_CMD_NUM=11;
		Clear_COMMAND;
	}
}//SPI_Command_Parser





void SPI_IRQHandler(void)
{
		int tmp=0;

		if(FLAG_SPI_RESPONSE==0){
			SPI_Receive();
		}else if(FLAG_SPI_RESPONSE==1){
			SPI_TANSIST_COUNT++;
			SPI_READ_RX(SPI);

			
			if(SPI_TANSIST_TOTAL<6){
				if(SPI_TANSIST_COUNT==(SPI_TANSIST_TOTAL)){
					
					SPI_TANSIST_TOTAL=0;
					SPI_TANSIST_COUNT=0;
					FLAG_SPI_RESPONSE=0;
					FLAG_SPI_Receive=0;
					FLAG_SPI_RX_HAVE_RECEIVED=1;
					SPI_BSY=0;
				}
				
			}else{
				FLAG_SPI_RESPONSE=0xff;
					//error
			}
		}
}



void gpio_INT(void){
	if(SPI_BSY==0&& FLAG_SPI_RESPONSE!=1 && (SPI_CHIP_SELECT==1)){
		SW_INT=0;
		if(com0Rbytes>0 || com1Rbytes>0){
			SW_INT=1;
		}else{
			SW_INT=0;		
		}
		SW_INT=0;	
	}	
}





void UART0_IRQHandler(void)
{
		FLAG_UART0_RECEIVE	=1;		
		UART_DISABLE_INT(UART0, UART_INTEN_RDAIEN_Msk);
}

void UART1_IRQHandler(void)
{
		FLAG_UART1_RECEIVE	=1;		
		UART_DISABLE_INT(UART1, UART_INTEN_RDAIEN_Msk);
}



void SPI_Receive(void){
	unsigned int len,tmp,tmp_send_tx_buf;

	while(SPI_GET_RX_FIFO_EMPTY_FLAG(SPI0) == 0){
		switch(FLAG_RECEIVE_C){
			case 1:
				SPI_IN_COUNT++;
			if(SPI_IN_COUNT>7){
				FLAG_RECEIVE_C=0;
				SPI_IN_COUNT=0;
				tmp_SPI_IN_COUNT=0;
				FLAG_SPI_Receive=0;

			}else{
				CMD_32bit[SPI_IN_COUNT]=SPI_READ_RX(SPI);
				if(((CMD_32bit[0]&0xf000000)>>24)==Send_TX_buffer && SPI_IN_COUNT==1){
					if((CMD_32bit[1]&0xff)!=0xff){
						NUM_SPI_IN_32BIT=(((((CMD_32bit[0]>>16)&0xff)-1)+3)/4);
					}
				}
	
				if((SPI_IN_COUNT-tmp_SPI_IN_COUNT)==NUM_SPI_IN_32BIT){
						CMD_32BITS_DIVIDEINTO_8BITS(CMD_32bit,NUM_SPI_IN_32BIT+1);			
						if(SPI_IN_COMMAND[C_CMD]==Send_TX_buffer &&((SPI_IN_COMMAND[PORT]==ALL_UART))){
							if(SPI_IN_COMMAND[C_CHECKSUM]==cal_in_checksum(13,&SPI_IN_COMMAND[C_Length])){
								LAST_CMD=SPI_IN_COMMAND[C_CMD];
								FLAG_SPI_Receive=1;
								FLAG_RECEIVE_C=0;
								SPI_BSY=1;
							}else{
									FLAG_SPI_Receive=0;//error
							}
						}else{
							if(SPI_IN_COMMAND[C_CHECKSUM]==cal_in_checksum(SPI_IN_COMMAND[C_Length],&SPI_IN_COMMAND[C_Length])){
								LAST_CMD=SPI_IN_COMMAND[C_CMD];
								FLAG_SPI_Receive=1;
								FLAG_RECEIVE_C=0;
								SPI_BSY=1;
							}else{
									FLAG_SPI_Receive=0;//error
							}
						}
				} 
				}
				break;
			default:
				tmp=SPI_READ_RX(SPI);
				if((tmp&0xff)=='C' || (tmp&0xff)=='D'|| (tmp&0xff)=='E'|| (tmp&0xff)=='F'){
					if((tmp&0xff)=='C'){
						FLAG_MASTER_RX_FULL=0;
					}else{
						FLAG_MASTER_RX_FULL=1;
					}

					SPI_IN_COUNT=0;
					CMD_32bit[SPI_IN_COUNT]=tmp;
					if((tmp&0xf000000)>>24!=Send_TX_buffer){
						NUM_SPI_IN_32BIT=(((((tmp>>16)&0xff)-1)+3)/4);
					}else{
						NUM_SPI_IN_32BIT=3;
					}
					tmp_SPI_IN_COUNT=SPI_IN_COUNT;
					FLAG_RECEIVE_C=1;
					
				}else{
					test_gpio=0;
					FLAG_RECEIVE_C=0;
					SPI_IN_COUNT=0;
					tmp_SPI_IN_COUNT=0;
					FLAG_SPI_Receive=0;

				}
		}//switch
	}//while
}//SPI_Receive(void)



void UART_Receive(char UART_PORT)
{
    //uint8_t u8InChar=0xFF;
if(LAST_CMD!=RESET_UART || LAST_CMD!=CLOSE_UART){
	if(UART_PORT==UART_PORT_0){
    uint32_t u32IntSts= UART0->INTSTS;
		tmp_function_rx++;
	  if(UART0->INTSTS & UART_INTSTS_RDAIF_Msk) {
      /* Get all the input characters */
			if(com0Rbytes>(RXBUFSIZE-1)){	
				UART_DISABLE_INT(UART0, UART_INTEN_RDAIEN_Msk);
				FLAG_UART0_RX_OVERFLOW=1;
			}else{ 						
				UART_addq_ReadBuffer(UART_PORT,UART_READ(UART0));	
				tmp_cnt_tx_tx++;
			}
		}		
	}else if(UART_PORT==UART_PORT_1){
    //uint8_t u8InChar=0xFF;
    uint32_t u32IntSts= UART1->INTSTS;
	    if(UART1->INTSTS & UART_INTSTS_RDAIF_Msk) {
        /* Get all the input characters */
						if(com1Rbytes>(RXBUFSIZE-1)){	
								UART_DISABLE_INT(UART1, UART_INTEN_RDAIEN_Msk);
								FLAG_UART1_RX_OVERFLOW=1;
						}else{ 						
								UART_addq_ReadBuffer(UART_PORT,UART_READ(UART1));	
						}
			}	
	}
}else{
			if(UART_PORT==UART_PORT_0){
				UART_FIFO_RXRST(UART0);
				UART_READ(UART0);
			}else if(UART_PORT==UART_PORT_1){
				UART_FIFO_RXRST(UART1);
				UART_READ(UART1);
			}
	}	
}


void UART_Transmit(char UART_PORT){
	if(UART_PORT==UART_PORT_0){
		if(com0Tbytes>0){
				if(UART_IS_TX_FULL(UART0)==1){
					//error
				}else{
					UART_WRITE(UART0,UART_deleteq_WriteBuffer(UART_PORT));
				}
		}
	}else if(UART_PORT==UART_PORT_1){
		if(com1Tbytes>0){
					if(UART_IS_TX_FULL(UART1)==1){
						//FLAG_SPI_Receive=0xff;//error
						//break;
					}else{
						UART_WRITE(UART1,UART_deleteq_WriteBuffer(UART_PORT));
					}
		}
	}
}


int UART_RX_overflow(char UART_PORT){
	if(UART_PORT==UART_PORT_0){
		if(com0Rbytes<RXBUFSIZE){
			FLAG_UART0_RX_OVERFLOW=0;
			UART_addq_ReadBuffer(UART_PORT,UART_READ(UART0));
			UART_ENABLE_INT(UART0, UART_INTEN_RDAIEN_Msk);
			return 0;
		}else{
			
			return 1;
		}
	}else if(UART_PORT==UART_PORT_1){
		if(com1Rbytes<RXBUFSIZE){
			FLAG_UART1_RX_OVERFLOW=0;
			UART_addq_ReadBuffer(UART_PORT,UART_READ(UART1));
			UART_ENABLE_INT(UART1, UART_INTEN_RDAIEN_Msk);
			return 0;
		}else{
			
			return 1;
		}
	}
}

    


void CMD_32BITS_DIVIDEINTO_8BITS(int *buf,int cnt){
	int k=0;
		for(k=0;k<cnt*4;k=k+4){
			SPI_IN_COMMAND[k]	 =buf[k/4]&0xff;
			SPI_IN_COMMAND[k+1]=(buf[k/4]>>8)&0xff;
			SPI_IN_COMMAND[k+2]=(buf[k/4]>>16)&0xff;
			SPI_IN_COMMAND[k+3]=(buf[k/4]>>24)&0xff;
		}
}



/******************************************************************/
char UART0_IsFull_WriteBuffer(void){
	return(UART0_rear_WriteBuffer==(BUFSIZE))?1:0;
}
char UART0_IsEmpty_WriteBuffer(void){

	return (UART0_rear_WriteBuffer==UART0_front_WriteBuffer) ? 1:0;
}

char UART0_IsEmpty_ReadBuffer(void){
	return (UART0_front_ReadBuffer==UART0_rear_ReadBuffer)?1:0;
}
char UART0_IsFull_ReadBuffer(void){
	return(UART0_rear_ReadBuffer==(BUFSIZE))?1:0;
}
void UART0_clr_ReadBuffer(void){
	UART0_front_ReadBuffer=-1;
	UART0_rear_ReadBuffer=-1;
}
void UART0_clr_WriteBuffer(void){
	UART0_front_WriteBuffer=-1;
	UART0_rear_WriteBuffer=-1;
}
void UART_clr_WriteBuffer(char UART_PORT){
	if(UART_PORT==UART_PORT_0){
		UART0_front_WriteBuffer=-1;
		UART0_rear_WriteBuffer=-1;
	}else if(UART_PORT==UART_PORT_1){
		UART1_front_WriteBuffer=-1;
		UART1_rear_WriteBuffer=-1;
	}
}

void UART_clr_ReadBuffer(char UART_PORT){
	if(UART_PORT==UART_PORT_0){
		UART0_front_ReadBuffer=-1;
		UART0_rear_ReadBuffer=-1;
	}else if(UART_PORT==UART_PORT_1){
		UART1_front_ReadBuffer=-1;
		UART1_rear_ReadBuffer=-1;
	}
}


char UART1_IsFull_WriteBuffer(void){
	return(UART1_rear_WriteBuffer==(BUFSIZE))?1:0;
}
char UART1_IsEmpty_WriteBuffer(void){

	return (UART1_rear_WriteBuffer==UART0_front_WriteBuffer) ? 1:0;
}

char UART1_IsEmpty_ReadBuffer(void){
	return (UART1_front_ReadBuffer==UART1_rear_ReadBuffer)?1:0;
}
char UART1_IsFull_ReadBuffer(void){
	return(UART1_rear_ReadBuffer==(BUFSIZE))?1:0;
}
void UART1_clr_ReadBuffer(void){
	UART1_front_ReadBuffer=-1;
	UART1_rear_ReadBuffer=-1;
}
void UART1_clr_WriteBuffer(void){
	UART1_front_WriteBuffer=-1;
	UART1_rear_WriteBuffer=-1;
}

UINT8  UART_addq_WriteBuffer(char UART_PORT,UINT8 data){
	if(UART_PORT==UART_PORT_0){
		if(com0Tbytes<RXBUFSIZE){
			UART0_rear_WriteBuffer=(UART0_rear_WriteBuffer+1)%BUFSIZE;
			UART0_OUT_BUFFER[UART0_rear_WriteBuffer]=data;
			com0Tbytes	++;	
		}else{
				FLAG_SPI_Receive=0xff;
		}
		return UART0_IsFull_WriteBuffer();
	}else	if(UART_PORT==UART_PORT_1){
		if(com1Tbytes<RXBUFSIZE){
			UART1_rear_WriteBuffer=(UART1_rear_WriteBuffer+1)%BUFSIZE;
			UART1_OUT_BUFFER[UART1_rear_WriteBuffer]=data;
			com1Tbytes++;	
		}else{
				FLAG_SPI_Receive=0xff;
		}
		return UART0_IsFull_WriteBuffer();
	}
}

UINT8  UART_deleteq_WriteBuffer(char UART_PORT){
	if(UART_PORT==UART_PORT_0){
		if(com0Tbytes>0){
			UART0_front_WriteBuffer=((UART0_front_WriteBuffer+1)%BUFSIZE);	
			total_cnt_write++;
			com0Tbytes--;
			return UART0_OUT_BUFFER[UART0_front_WriteBuffer];
		}else{
			FLAG_SPI_Receive=0xff;
			return NULL;
		}	
	}else if(UART_PORT==UART_PORT_1){
		if(com1Tbytes>0){
			UART1_front_WriteBuffer=((UART1_front_WriteBuffer+1)%BUFSIZE);	
			total_cnt_write++;
			com1Tbytes--;
			return UART1_OUT_BUFFER[UART1_front_WriteBuffer];
		}else{
			FLAG_SPI_Receive=0xff;
			return NULL;
		}	
	}
}


UINT8 UART_addq_ReadBuffer(char UART_PORT,UINT8 data){
	if(UART_PORT==UART_PORT_0){
		if(com0Rbytes<(RXBUFSIZE)){
			UART0_rear_ReadBuffer=(UART0_rear_ReadBuffer+1)%BUFSIZE;
			UART0_IN_BUFFER[UART0_rear_ReadBuffer]=data;
			com0Rbytes++;
		}else{
			FLAG_SPI_Receive=0xff;
		}
		total_cnt++;
		return UART0_IsFull_ReadBuffer();
	}else	if(UART_PORT==UART_PORT_1){
		if(com1Rbytes<(RXBUFSIZE)){
			UART1_rear_ReadBuffer=(UART1_rear_ReadBuffer+1)%BUFSIZE;
			UART1_IN_BUFFER[UART1_rear_ReadBuffer]=data;
			com1Rbytes++;

		}else{
				//error
			FLAG_SPI_Receive=0xff;
		}
		return UART1_IsFull_ReadBuffer();
	}
}

UINT8 UART_deleteq_ReadBuffer(char UART_PORT){

	/*
Bit 0 : RX buffer has data
Bit 1 : TX buffer is empty
*/	
	if(UART_PORT==UART_PORT_0){
		if(com0Rbytes>0){
				UART0_front_ReadBuffer=((UART0_front_ReadBuffer+1)%BUFSIZE);
				com0Rbytes--;	
				return UART0_IN_BUFFER[UART0_front_ReadBuffer];
		}else{
				UART0_rear_ReadBuffer=-1;
				UART0_front_ReadBuffer=-1;
			return NULL;
		}	
	}else if(UART_PORT==UART_PORT_1){
		if(com1Rbytes>0){
				UART1_front_ReadBuffer=((UART1_front_ReadBuffer+1)%BUFSIZE);
				com1Rbytes--;	
				return UART1_IN_BUFFER[UART1_front_ReadBuffer];
		}else{
				UART1_rear_ReadBuffer=-1;
				UART1_front_ReadBuffer=-1;
			return NULL;
		}	
	}
}

