/**************************************************************************//**
 * @file     N9H20_VPOST_TIANMA_TM022HDH31.c
 * @brief    Panel driver file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#include "stdio.h"
#include "stdlib.h"
#include "N9H20_VPOST.h"
#include "TM022HDH31.h"

extern void LCDDelay(unsigned int nCount);

#if defined(__HAVE_TIANMA_TM022HDH31__)

static UINT32 g_nScreenWidth;
static UINT32 g_nScreenHeight;

static unsigned char LandFlag = 0;

typedef enum{
	xInc_yInc=0,
	xInc_yDec,
	xDec_yInc,
	xDec_yDec
}LCD_INC_MODE;

#define GPIO_PORTA		1
#define GPIO_PORTB		2
#define GPIO_PORTC		4
#define GPIO_PORTD		8
#define GPIO_PORTE		16


static void delay_loop(UINT32 u32delay)
{
	volatile UINT32 ii, jj;
	for (jj=0; jj<u32delay; jj++)
		for (ii=0; ii<200; ii++);
}

void LCD_DelayMs(UINT32 Ms)
{
  UINT32 i;
	for(; Ms; Ms--)
		for(i=1000;i;i--);
}

typedef enum 
{
	eEXT 	= 0,
	eX32K 	= 1,
	eAPLL  	= 2,
	eUPLL  	= 3
}E_CLK;

static UINT32 GetPLLOutputKhz(
	E_CLK eSysPll,
	UINT32 u32FinKHz
	)
{
	UINT32 u32Freq, u32PllCntlReg;
	UINT32 NF, NR, NO;
	
	UINT8 au8Map[4] = {1, 2, 2, 4};
	if(eSysPll==eSYS_APLL)
		u32PllCntlReg = inp32(REG_APLLCON);
	else if(eSysPll==eSYS_UPLL)	
		u32PllCntlReg = inp32(REG_UPLLCON);		
	
	NF = (u32PllCntlReg&FB_DV)+2;
	NR = ((u32PllCntlReg & IN_DV)>>9)+2;
	NO = au8Map[((u32PllCntlReg&OUT_DV)>>14)];
//	sysprintf("PLL regster = 0x%x\n", u32PllCntlReg);	
//	sysprintf("NF = %d\n", NF);
//	sysprintf("NR = %d\n", NR);
//	sysprintf("NOr = %d\n", NO);
		
	u32Freq = u32FinKHz*NF/NR/NO;
//	sysprintf("PLL Freq = %d\n", u32Freq);
	return u32Freq;
}


static int gpio_setportdir(unsigned char port, unsigned short mask, unsigned short dir)
{
	switch (port) {

		case GPIO_PORTA:
			outp32(REG_GPIOA_OMD, inp32(REG_GPIOA_OMD) & ~(mask & (mask ^ dir)));
			outp32(REG_GPIOA_OMD, inp32(REG_GPIOA_OMD) | (mask & dir));			
			break;
		case GPIO_PORTB:
			outp32(REG_GPIOB_OMD, inp32(REG_GPIOB_OMD) & ~(mask & (mask ^ dir)));
			outp32(REG_GPIOB_OMD, inp32(REG_GPIOB_OMD) | (mask & dir));	
			break;			
		case GPIO_PORTC:
			outp32(REG_GPIOC_OMD, inp32(REG_GPIOC_OMD) & ~(mask & (mask ^ dir)));
			outp32(REG_GPIOC_OMD, inp32(REG_GPIOC_OMD) | (mask & dir));	
			break;		
		case GPIO_PORTD:
			outp32(REG_GPIOD_OMD, inp32(REG_GPIOD_OMD) & ~(mask & (mask ^ dir)));
			outp32(REG_GPIOD_OMD, inp32(REG_GPIOD_OMD) | (mask & dir));	
			break;	
		case GPIO_PORTE:
			outp32(REG_GPIOE_OMD, inp32(REG_GPIOE_OMD) & ~(mask & (mask ^ dir)));
			outp32(REG_GPIOE_OMD, inp32(REG_GPIOE_OMD) | (mask & dir));	
			break;		
		default:
			return(-1);
	}
	return(0);
}

static int gpio_setportval(unsigned char port, unsigned short mask, unsigned short val)
{
	switch (port) {
	
		case GPIO_PORTA:
			outp32(REG_GPIOA_DOUT, inp32(REG_GPIOA_DOUT) & ~(mask & (mask ^ val)));
			outp32(REG_GPIOA_DOUT, inp32(REG_GPIOA_DOUT) | (mask & val));			
			break;
		case GPIO_PORTB:
			outp32(REG_GPIOB_DOUT, inp32(REG_GPIOB_DOUT) & ~(mask & (mask ^ val)));
			outp32(REG_GPIOB_DOUT, inp32(REG_GPIOB_DOUT) | (mask & val));
			break;			
		case GPIO_PORTC:
			outp32(REG_GPIOC_DOUT, inp32(REG_GPIOC_DOUT) & ~(mask & (mask ^ val)));
			outp32(REG_GPIOC_DOUT, inp32(REG_GPIOC_DOUT) | (mask & val));
			break;		
		case GPIO_PORTD:
			outp32(REG_GPIOD_DOUT, inp32(REG_GPIOD_DOUT) & ~(mask & (mask ^ val)));
			outp32(REG_GPIOD_DOUT, inp32(REG_GPIOD_DOUT) | (mask & val));
			break;	
		case GPIO_PORTE:
			outp32(REG_GPIOE_DOUT, inp32(REG_GPIOE_DOUT) & ~(mask & (mask ^ val)));
			outp32(REG_GPIOE_DOUT, inp32(REG_GPIOE_DOUT) | (mask & val));
			break;		
		default:
			return(-1);
	}
	return(0);
}

static int gpio_setportpull(unsigned char port, unsigned short mask, unsigned short pull)
{
	switch (port) {
	
		case GPIO_PORTA:
			outp32(REG_GPIOA_PUEN, inp32(REG_GPIOA_PUEN) & ~(mask & (mask ^ pull)));
			outp32(REG_GPIOA_PUEN, inp32(REG_GPIOA_PUEN) | (mask & pull));			
			break;
		case GPIO_PORTB:
			outp32(REG_GPIOB_PUEN, inp32(REG_GPIOB_PUEN) & ~(mask & (mask ^ pull)));
			outp32(REG_GPIOB_PUEN, inp32(REG_GPIOB_PUEN) | (mask & pull));	
			break;			
		case GPIO_PORTC:
			outp32(REG_GPIOC_PUEN, inp32(REG_GPIOC_PUEN) & ~(mask & (mask ^ pull)));
			outp32(REG_GPIOC_PUEN, inp32(REG_GPIOC_PUEN) | (mask & pull));	
			break;	
		case GPIO_PORTD:
			outp32(REG_GPIOD_PUEN, inp32(REG_GPIOD_PUEN) & ~(mask & (mask ^ pull)));
			outp32(REG_GPIOD_PUEN, inp32(REG_GPIOD_PUEN) | (mask & pull));	
			break;
		case GPIO_PORTE:
			outp32(REG_GPIOE_PUEN, inp32(REG_GPIOE_PUEN) & ~(mask & (mask ^ pull)));
			outp32(REG_GPIOE_PUEN, inp32(REG_GPIOE_PUEN) | (mask & pull));	
			break;		
		default:
			return(-1);
	}
	return(0);
}

static int gpio_readport(unsigned char port, unsigned int *val)
{
	switch (port) {
	
		case GPIO_PORTA:
			*val = (inp32(REG_GPIOA_PIN) & 0x0fff);
			break;
		case GPIO_PORTB:
			*val = (inp32(REG_GPIOB_PIN) & 0xffff);
			break;			
		case GPIO_PORTC:
			*val = (inp32(REG_GPIOC_PIN) & 0xffff);
			break;		
		case GPIO_PORTD:
			*val = (inp32(REG_GPIOD_PIN) & 0xffff);
			break;	
		case GPIO_PORTE:
			*val = (inp32(REG_GPIOE_PIN) & 0x0fff);
			break;		
		default:
			return(-1);

	}
	return(0);
}

#define	SET_TFT_BL()	gpio_setportval(GPIO_PORTE, 0x1<<0, 0x1<<0)
#define	CLR_TFT_BL()	gpio_setportval(GPIO_PORTE, 0x1<<0, 0x0<<0)

#define	SET_TFT_RST()	gpio_setportval(GPIO_PORTE, 0x1<<1, 0x1<<1)
#define	CLR_TFT_RST()	gpio_setportval(GPIO_PORTE, 0x1<<1, 0x0<<1)

#define	SET_TFT_CS()	gpio_setportval(GPIO_PORTB, 0x1<<15, 0x1<<15)
#define	CLR_TFT_CS()	gpio_setportval(GPIO_PORTB, 0x1<<15, 0x0<<15)

#define	SET_TFT_ADDR()	gpio_setportval(GPIO_PORTD, 0x1<<11, 0x1<<11)
#define	CLR_TFT_ADDR()	gpio_setportval(GPIO_PORTD, 0x1<<11, 0x0<<11)

#define	SET_TFT_WR()	gpio_setportval(GPIO_PORTD, 0x1<<9, 0x1<<9)
#define	CLR_TFT_WR()	gpio_setportval(GPIO_PORTD, 0x1<<9, 0x0<<9)

#define	SET_TFT_RD()	gpio_setportval(GPIO_PORTD, 0x1<<10, 0x1<<10)
#define	CLR_TFT_RD()	gpio_setportval(GPIO_PORTD, 0x1<<10, 0x0<<10)

#define	OUT_TFT_DATA()	gpio_setportdir(GPIO_PORTC, 0xFF<<0, 0xFF<<0)
#define	IN_TFT_DATA()	gpio_setportdir(GPIO_PORTC, 0xFF<<0, 0x00<<0)


static void DispBackLight(unsigned char cMode)
{
  	if(cMode == 1)
		SET_TFT_BL();	
	else
		CLR_TFT_BL(); 	
}

void TFT_WriteData(char Data)
{
	gpio_setportval(GPIO_PORTC, 0xFF<<0, (int)(Data<<0));
}

char TFT_ReadData(void)
{
	int wData;

	gpio_readport(GPIO_PORTC, (unsigned int *)&wData);

	return (char)(wData>>0);
}

static void LCD_WriteIndex(unsigned char index)
{
	//configure data bus as OUTPUT
	OUT_TFT_DATA();

	//CS low, chipselect sensor
	CLR_TFT_CS();
	
	SET_TFT_RD();

	//A0 low, means to write cmd
	CLR_TFT_ADDR();

	// set address register
	TFT_WriteData(index);

	//WR low, enable write
	CLR_TFT_WR();

	//Wait for stable
	//LCD_Sleep(4);

	//WR high, disable write
	SET_TFT_WR();
	
	//CS high, deselect
	SET_TFT_CS();

	//Wait for stable
	//LCD_Sleep(10);
}

static void LCD_WriteData(unsigned char val)
{
	//configure data bus as OUTPUT
	OUT_TFT_DATA();

	//CS low, chipselect
	CLR_TFT_CS();
	
	SET_TFT_RD();

	//A0 high means to write dat
	SET_TFT_ADDR();

	// set address register
	TFT_WriteData(val);

	//WR low, enable write
	CLR_TFT_WR();

	//Wait for stable
	//LCD_Sleep(4);

	//WR high, disable write
	SET_TFT_WR();

	//CS high, deselect
	SET_TFT_CS();

	//Wait for stable
	//LCD_Sleep(10);
}

char LCD_ReadData(void)
{
	char dat;
		
	//configure data bus as OUTPUT
	IN_TFT_DATA();

	//CS low, chipselect
	CLR_TFT_CS();
	
	SET_TFT_WR();

	//A0 high means to read dat
	SET_TFT_ADDR();

	//RD low, enable read
	CLR_TFT_RD();

	//Wait for stable
	//LCD_Sleep(1);
	
	//Read Data from TFT
	dat = TFT_ReadData();

	//RD high, disable read
	SET_TFT_RD();
	
	//CS high, deselect
	SET_TFT_CS();

	//Wait for stable
	//LCD_Sleep(10);
	
	return dat;
}


static void LCD_SetAddrIncMode(LCD_INC_MODE xyDirection, unsigned char yIncFirst)
{
	unsigned int Mode;

	LCD_WriteIndex(RDDMADCTL);
	LCD_ReadData();
	Mode = LCD_ReadData();
	Mode &=0x1F;
	switch(xyDirection)
	{
		case xInc_yInc:
			Mode|=0x00;
			break;
		case xInc_yDec:
			Mode|=0x80;
			break;
		case xDec_yInc:
			Mode|=0x40;
			break;
		case xDec_yDec:
			Mode|=0xC0;
			break;	
	}
	if(yIncFirst)
		Mode |=0x20;
	LCD_WriteIndex(MADCTL);
	LCD_WriteData(Mode);	
}

volatile int s_mpu_init = 0;

static void TFT_ConfigGPIO(void)
{

	if (s_mpu_init==0)
	{
//		s_mpu_init = 1;

	//TFT_WE
	gpio_setportpull(GPIO_PORTD, 0x1<<9, 0x1<<9);
	gpio_setportdir(GPIO_PORTD, 0x1<<9, 0x1<<9);
	gpio_setportval(GPIO_PORTD, 0x1<<9, 0x1<<9);
	}	
	//TFT_OE
	gpio_setportpull(GPIO_PORTD, 0x1<<10, 0x1<<10);
	gpio_setportdir(GPIO_PORTD, 0x1<<10, 0x1<<10);
	gpio_setportval(GPIO_PORTD, 0x1<<10, 0x1<<10);
	//TFT_A0
	gpio_setportpull(GPIO_PORTD, 0x1<<11, 0x1<<11);
	gpio_setportdir(GPIO_PORTD, 0x1<<11, 0x1<<11);
	gpio_setportval(GPIO_PORTD, 0x1<<11, 0x1<<11);
	//TFT_CS
	gpio_setportpull(GPIO_PORTB, 0x1<<15, 0x1<<15);
	gpio_setportdir(GPIO_PORTB, 0x1<<15, 0x1<<15);
	gpio_setportval(GPIO_PORTB, 0x1<<15, 0x1<<15);	

	
	//TFT_RST
	gpio_setportpull(GPIO_PORTE, 0x1<<1, 0x1<<1);
	gpio_setportdir(GPIO_PORTE, 0x1<<1, 0x1<<1);
	gpio_setportval(GPIO_PORTE, 0x1<<1, 0x1<<1);
	//TFT_BL
	gpio_setportpull(GPIO_PORTE, 0x1<<0, 0x1<<0);
	gpio_setportdir(GPIO_PORTE, 0x1<<0, 0x1<<0);
	gpio_setportval(GPIO_PORTE, 0x1<<0, 0x1<<0);

	//TCS_DATA
	gpio_setportpull(GPIO_PORTC, 0xFF<<0, 0xFF<<0);
	gpio_setportdir(GPIO_PORTC, 0xFF<<0, 0xFF<<0);
	gpio_setportval(GPIO_PORTC, 0xFF<<0, 0xFF<<0);
}

void LCD_Identification(void)
{
    char tmp=0xAA;
    LCD_WriteIndex(RDDIDIF);
	tmp=LCD_ReadData();
    tmp=LCD_ReadData();
    tmp=LCD_ReadData();
    tmp=LCD_ReadData();
}

void LCD_DisplayPwrMode(void)
{
    char tmp=0xAA;
    LCD_WriteIndex(RDDPM);
  
	tmp=LCD_ReadData();
    tmp=LCD_ReadData();
}

void LCD_LandModeOn(void)
{
	LCD_SetAddrIncMode(xInc_yDec,TRUE);
	LandFlag = 1;
}


void LCD_DisplayStatus(void)
{
    char tmp=0xAA;
    LCD_WriteIndex(RDDST);
 
	tmp=LCD_ReadData();
    tmp=LCD_ReadData();
    tmp=LCD_ReadData();
    tmp=LCD_ReadData();
    tmp=LCD_ReadData();
}

void LCD_Reset(void)
{
	CLR_TFT_CS();
    LCD_DelayMs(50);					   
	SET_TFT_RST();    
    LCD_DelayMs(50);					   
    CLR_TFT_RST();		 	 
	LCD_DelayMs(100);
    SET_TFT_RST();	
    LCD_DelayMs(50);	
	SET_TFT_CS();    				       
}

void LCD_PowerOn(void)
{	
    LCD_Reset();

    LCD_Identification();
    LCD_DisplayStatus();
    LCD_DisplayPwrMode();
    
	LCD_WriteIndex(PWCTLB);//0xCF Power control B
	LCD_WriteData(0x00);//0x00;
	LCD_WriteData(0x81);//0x99;0x83
	LCD_WriteData(0x30);//0x30;0xb0
    LCD_DelayMs(10);
	LCD_WriteIndex(PWONSCTL);//0xED,Power on sequence control
	LCD_WriteData(0x64);
	LCD_WriteData(0x03);
	LCD_WriteData(0x12);
	LCD_WriteData(0x81);
    LCD_DelayMs(10);
	LCD_WriteIndex(DTIMCTLA);//0xE8,Driver timing control A
	LCD_WriteData(0x85);
	LCD_WriteData(0x10);
	LCD_WriteData(0x78);
    LCD_DelayMs(10);
	LCD_WriteIndex(PWCTLA);//0xCB,Power control A
	LCD_WriteData(0x39);
	LCD_WriteData(0x2C);
	LCD_WriteData(0x00);
	LCD_WriteData(0x34);
	LCD_WriteData(0x02);
    LCD_DelayMs(10);
	LCD_WriteIndex(PRCTL);//0xF7,Pump ratio control
	LCD_WriteData(0x20);
    LCD_DelayMs(10);
	LCD_WriteIndex(DTIMCTLB);//0xEA,Driver timing control B
	LCD_WriteData(0x00);
	LCD_WriteData(0x00);
    LCD_DelayMs(10);
    LCD_WriteIndex(FRMCTR1);//0xB1,Frame Rate Control (In Normal Mode / Full colors
	LCD_WriteData(0x00);
	LCD_WriteData(0x1B);//0x1b,70Hz;0x19   
    LCD_DelayMs(10);    	
	LCD_WriteIndex(DISCTRL);//0xB6,Display Function Control
	LCD_WriteData(0x0A);
	LCD_WriteData(0xA2);
    LCD_DelayMs(10);
	LCD_WriteIndex(PWCTRL1);//0xC0,Power Control 1
	LCD_WriteData(0x22);//0x35,5.50V;0x22,4.55V
    LCD_DelayMs(10);
	LCD_WriteIndex(PWCTRL2);//0xC1,Power Control 2
	LCD_WriteData(0x11);//0x11,0x01
	LCD_WriteIndex(VMCTRL1);//0xC5,VCOM Control 1
	LCD_WriteData(0x5C);//0x45,4.425V;0x5c,5.000V
	LCD_WriteData(0x4C);//0x45,-0.775V;0x4c,-0.600V
    LCD_DelayMs(10);
	LCD_WriteIndex(VMCTRL2);//0xC7,VCOM Control 2
	LCD_WriteData(0xA2);//0xa2,VM-30;0x8f,VM-49
    LCD_DelayMs(10);
	LCD_WriteIndex(EN3G);//0xF2,Enable_3G
	LCD_WriteData(0x00);
    LCD_DelayMs(10);
	LCD_WriteIndex(GAMSET);//0x26,Gamma Set
	LCD_WriteData(0x01);
    LCD_DelayMs(10);
	LCD_WriteIndex(PGAMCTRL);//0xE0 Positive Gamma Control
	LCD_WriteData(0x0F);//0x0f;0x0f
	LCD_WriteData(0x21);//0x28;0x21
	LCD_WriteData(0x21);//0x24;0x21
	LCD_WriteData(0x0a);//0x0b;0x0a
	LCD_WriteData(0x10);//0x0e;0x10
	LCD_WriteData(0x0b);//0x08;0x0b
	LCD_WriteData(0x4e);//0x50;0x4e
	LCD_WriteData(0xe5);//0xb8;0xe5
	LCD_WriteData(0x3c);//0x3e;0x3c
	LCD_WriteData(0x09);//0x07;0x09
	LCD_WriteData(0x14);//0x15;0x14
	LCD_WriteData(0x09);//0x08;0x09
	LCD_WriteData(0x18);//0x1b;0x18
	LCD_WriteData(0x0b);//0x0d;0x0b
	LCD_WriteData(0x08);//0x08;0x08
    LCD_DelayMs(10);
	LCD_WriteIndex(NGAMCTRL);//0xE1 Negative Gamma Correction
	LCD_WriteData(0x00);//0x08;0x00
	LCD_WriteData(0x1e);//0x17;0x1e
	LCD_WriteData(0x1e);//0x1b;0x1e
	LCD_WriteData(0x05);//0x04;0x05
	LCD_WriteData(0x0f);//0x11;0x0f
	LCD_WriteData(0x04);//0x07;0x04
	LCD_WriteData(0x31);//0x2f;0x31
	LCD_WriteData(0x73);//0x74;0x73
	LCD_WriteData(0x43);//0x41;0x43
	LCD_WriteData(0x06);//0x08;0x06
	LCD_WriteData(0x0b);//0x0a;0x0b
	LCD_WriteData(0x06);//0x07;0x06
	LCD_WriteData(0x27);//0x24;0x27
	LCD_WriteData(0x34);//0x32;0x34
	LCD_WriteData(0x0f);//0x0f;0x0f
	LCD_WriteData(0x06);
	LCD_WriteData(0x27);//0x30,0x27
	LCD_WriteData(0x34);//0x38,0x34
	LCD_WriteData(0x0F);
    LCD_DelayMs(10);
    LCD_WriteIndex(MADCTL);//0x36,Memory Access Control
	LCD_WriteData(0x08);//0x08,48
    LCD_DelayMs(10);
    LCD_WriteIndex(CASET);//0x2A,Column Address Set 
	LCD_WriteData(0x00);
	LCD_WriteData(0x00);
	LCD_WriteData(0x00);
	LCD_WriteData(0xEF);
    LCD_DelayMs(10);
	LCD_WriteIndex(PASET);//0x2B,Page Address Set 
	LCD_WriteData(0x00);
	LCD_WriteData(0x00);
	LCD_WriteData(0x01);
	LCD_WriteData(0x3F);
    LCD_DelayMs(10);
    LCD_WriteIndex(PIXSET);//0x3A,Pixel Format Set 
	LCD_WriteData(0x05);//0x05,0x66
    LCD_DelayMs(10);
    LCD_WriteIndex(SLPOUT);//0x11,Sleep Out
	LCD_DelayMs(200);
	LCD_WriteIndex(DISPON);//0x29,Display ON
    LCD_DelayMs(200);		
    LCD_WriteIndex(RAMWR);//0x2C,Memory Write  
    LCD_DelayMs(200);
}

static void LCD_SetRegion(int x_start,int y_start,int x_end,int y_end)
{		
	LCD_WriteIndex(CASET);  
	LCD_WriteData(x_start >> 8);     
	LCD_WriteData(x_start & 0xff);
	LCD_WriteData(x_end >> 8);     
	LCD_WriteData(x_end & 0xff);

	LCD_WriteIndex(PASET);  
	LCD_WriteData(y_start >> 8);     
	LCD_WriteData(y_start & 0xff);
	LCD_WriteData(y_end >> 8);     
	LCD_WriteData(y_end & 0xff);
}

static void LCD_BulkWriteDataStart(void)
{
	LCD_WriteIndex(RAMWRC);
    LCD_WriteIndex(RAMWR);
}

void LCD_SetDisplayRegion(int x,int y,int w,int h)
{
	LCD_SetRegion(x,y,x+w-1,y+h-1);
	LCD_BulkWriteDataStart();
}


void LCD_FillArea(int x,int y,int w,int h,int Color)
{
	int Count,i;
	Count=w*h;
	
	LCD_SetRegion(x,y,x+w-1,y+h-1);
	LCD_BulkWriteDataStart();
	
	// dummy write 
//	LCD_WriteData(0x00);
	
	// write data
	for(i=0;i<Count;i++)
    {
		LCD_WriteData(Color>>8);
        LCD_WriteData(Color&0xFF);
    }
}

static void TIANMA_TM022HDH31_Init(void)  
{  

	// set GPC[7:0] to GPIO mode (LCD data bus)
	outp32(REG_GPCFUN, inp32(REG_GPCFUN) & 0xFFFF0000);

	// set GPD[11] to GPIO mode (LCD RS)
	outp32(REG_GPDFUN, inp32(REG_GPDFUN) & ~MF_GPD11);

	// set GPB[15] to GPIO mode (LCD CS)
	outp32(REG_GPBFUN, inp32(REG_GPBFUN) & ~MF_GPB15);

	// set GPE[1] to GPIO mode (LCD RST)
	outp32(REG_GPEFUN, inp32(REG_GPEFUN) & ~MF_GPE1);
	
	// set GPE[0] to GPIO mode (LCD BL, backlight)
	outp32(REG_GPEFUN, inp32(REG_GPEFUN) & ~MF_GPE0);

	// set GPD[10] to GPIO mode (LCD RD)
	outp32(REG_GPDFUN, inp32(REG_GPDFUN) & ~MF_GPD10);

	// set GPD[9] to GPIO mode (LCD WR)
	outp32(REG_GPDFUN, inp32(REG_GPDFUN) & ~MF_GPD9);
	
	LCD_DelayMs(100);

	TFT_ConfigGPIO();
	DispBackLight(1);

	LCD_PowerOn();
	LCD_LandModeOn();
	
	// dummy write
	LCD_WriteData(0x00);	
	LCD_SetDisplayRegion(0,0,320,240);
	
//   	LCD_FillArea(0,0,320,240,0xF800);	// red
//   	LCD_FillArea(0,0,320,240,0x07E0);   // green	
//   	LCD_FillArea(0,0,320,240,0x001F);	// blue
}


INT vpostLCMInit_TIANMA_TM022HDH31(PLCDFORMATEX plcdformatex, UINT32 *pFramebuf)
{
	volatile S_DRVVPOST_MPULCM_CTRL sMPU;
	volatile S_DRVVPOST_MPULCM_TIMING sTiming = {1,1,1,1};
	volatile S_DRVVPOST_MPULCM_WINDOW sWindow = {240, 320};

	UINT32 nBytesPixel, u32PLLclk, u32ClockDivider;

	// VPOST clock control
	outp32(REG_AHBCLK, inp32(REG_AHBCLK) | VPOST_CKE | HCLK4_CKE);
	outp32(REG_AHBIPRST, inp32(REG_AHBIPRST) | VPOSTRST);
	delay_loop(2);			
	outp32(REG_AHBIPRST, inp32(REG_AHBIPRST) & ~VPOSTRST);	
	delay_loop(2);				
	
	u32PLLclk = GetPLLOutputKhz(eUPLL, 12000);			// CLK_IN = 12 MHz
	u32ClockDivider = u32PLLclk / 40000;
	u32ClockDivider --;
	
	outp32(REG_CLKDIV1, inp32(REG_CLKDIV1) & ~VPOST_N0);						
	outp32(REG_CLKDIV1, (inp32(REG_CLKDIV1) & ~VPOST_N1) | ((u32ClockDivider & 0xFF) << 8));						
	outp32(REG_CLKDIV1, inp32(REG_CLKDIV1) & ~VPOST_S);
	outp32(REG_CLKDIV1, inp32(REG_CLKDIV1) | (3<<3));		// VPOST clock from UPLL		

    sysprintf("Begin MPU LCM initial !!!\n");	

	// stop MPU trigger
	vpostVAStopTriggerMPU();	
    
	// MPU LCM initial 	
	TIANMA_TM022HDH31_Init();

	// Enable VPOST function pins
	vpostSetDataBusPin(eDRVVPOST_DATA_8BITS);	
		  
	// LCD image source select
	vpostSetLCM_ImageSource(eDRVVPOST_FRAME_BUFFER);
	
	// configure LCD interface
	vpostSetLCM_TypeSelect(eDRVVPOST_MPU);	

	// configure LCD timing sync or async with TV timing	
	vpostsetLCM_TimingType(eDRVVPOST_ASYNC_TV);
	
	// set MPU bus mode
	vpostSetMPULCM_BusModeSelect(eDRVVPOST_MPU_8_8);
	
    // set MPU timing 
    vpostSetMPULCM_TimingConfig((S_DRVVPOST_MPULCM_TIMING *)&sTiming);

    // set MPU LCM window 
	vpostSetMPULCM_ImageWindow((S_DRVVPOST_MPULCM_WINDOW *)&sWindow);
    
    // set frambuffer base address
    if(pFramebuf != NULL) {
		vpostAllocVABufferFromAP(pFramebuf);
	} else {
    	if( vpostAllocVABuffer(plcdformatex, nBytesPixel)==FALSE)
    		return ERR_NULL_BUF;
    }
	
	// set frame buffer data format
	vpostSetFrameBuffer_DataType(plcdformatex->ucVASrcFormat);
	
	vpostSetYUVEndianSelect(eDRVVPOST_YUV_LITTLE_ENDIAN);
	
	// enable MPU LCD controller
	outp32(REG_LCM_MPUCMD, inp32(REG_LCM_MPUCMD)|MPUCMD_WR_RS|MPUCMD_MPU_CS);			
	vpostVAStartTrigger_MPUContinue();

//	BacklightControl(TRUE);			
	return 0;
}

INT32 vpostLCMDeinit_TIANMA_TM022HDH31(VOID)
{
	vpostVAStopTrigger();
	vpostFreeVABuffer();
	outp32(REG_AHBCLK, inp32(REG_AHBCLK) & ~VPOST_CKE);	
	return 0;
}

VOID vpostEnaBacklight(void)
{
	/* set BL_EN (GPE1) to High */
	outpw(REG_GPEFUN, inpw(REG_GPEFUN) & ~MF_GPE1);
	outpw(REG_GPIOE_OMD, inpw(REG_GPIOE_OMD) | 0x00000002);
	outpw(REG_GPIOE_DOUT, inpw(REG_GPIOE_DOUT) | 0x00000002);

	/* set BL_CTL (GPD0) to Low */
	outpw(REG_GPDFUN, inpw(REG_GPDFUN) & ~MF_GPD0);
	outpw(REG_GPIOD_OMD, inpw(REG_GPIOD_OMD) | 0x00000001);
	outpw(REG_GPIOD_DOUT, inpw(REG_GPIOD_DOUT) & ~0x00000001);
}
#endif    //HAVE_TIANMA_TM022HDH31
