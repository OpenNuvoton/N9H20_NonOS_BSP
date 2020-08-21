/**************************************************************************//**
 * @file     N9H20_nau8822.c
 * @brief    N9H20 series I2S demo code in NAU8822 codec
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#include <stdio.h>
#include "wbio.h"
#include "wblib.h"
#include "wbtypes.h"
#include "N9H20_I2S.h"
#include "DrvI2CH.h"

#define	WM_8000		(5<<1)
#define	WM_12000	(4<<1)
#define	WM_16000	(3<<1)
#define	WM_24000	(2<<1)	
#define	WM_32000	(1<<1)
#define	WM_48000	0
           
#define	WM_11025	(4<<1)
#define	WM_22050	(2<<1)
#define	WM_44100	0

int uSamplingRate = eDRVI2S_FREQ_44100;

void NAU8822_Init(void);

int bNAU8822Inited = 0;

//static unsigned short normal_addr[] = { 0x1A, I2C_CLIENT_END };

static void Delay(int nCnt)
{
	 int volatile loop;
	for (loop=0; loop<nCnt*10; loop++);
}

static BOOL i2c_Write_8bitSlaveAddr_8bitReg_8bitData(UINT8 uAddr, UINT8 uRegAddr, UINT8 uData)
{
	// 3-Phase(ID address, regiseter address, data(8bits)) write transmission
	volatile int u32Delay = 0x100;
	
	while(u32Delay--);		
	if ( (DrvI2CH_WriteByte(TRUE, uAddr, TRUE, FALSE)!=Successful) ||			// Write ID address to sensor
		 (DrvI2CH_WriteByte(FALSE, uRegAddr, TRUE, FALSE)!=Successful) ||	// Write register address to sensor
		 (DrvI2CH_WriteByte(FALSE, uData, TRUE, FALSE)!=Successful) )		// Write data to sensor
	{
		DrvI2CH_SendStop();
		return FALSE;
	}
	DrvI2CH_SendStop();
	if (uRegAddr==0x12 && (uData&0x80)!=0)
	{
		Delay(1000);			
	}
	return TRUE;
}

static int i2c_Read_8bitSlaveAddr_8bitReg_9bitData(UINT8 uAddr, UINT8 uRegAddr)
{
	UINT8 u8Data[2];
	int data;
	
	// 2-Phase(ID address, register address) write transmission
	DrvI2CH_SendStart();
	DrvI2CH_WriteByte(FALSE, uAddr, TRUE, FALSE);		// Write ID address to sensor
	DrvI2CH_WriteByte(FALSE, uRegAddr, TRUE, FALSE);	// Write register address to sensor

	// 2-Phase(ID-address, data(8bits)) read transmission
	DrvI2CH_SendStart();
	DrvI2CH_ReadByte(FALSE, &u8Data[1], TRUE, FALSE);		// Read high-byte data
	DrvI2CH_ReadByte(FALSE, &u8Data[0], TRUE, FALSE);		// Read low-byte data
	DrvI2CH_SendStop();
	data = (u8Data[1]<<8) | u8Data[0];
	return data;
}

static void i2C_WriteData(char data1, char data2)
{
	i2c_Write_8bitSlaveAddr_8bitReg_8bitData(0x34, data1, data2);
}

static int i2C_ReadData(char data1)
{
	return i2c_Read_8bitSlaveAddr_8bitReg_9bitData(0x34, data1);
}

static int NAU8822_ReadData(char addr)
{
	return i2C_ReadData(addr);
}

static int NAU8822_WriteData(char addr, unsigned short data)
{
	char i2cdata[2];
	
	//printk("(I2C)%d:0x%x\n", addr, data);
	i2cdata[0] = ((addr << 1)  | (data >> 8));		//addr(7bit) + data(first bit)
	i2cdata[1] = (char)(data & 0x00FF);			//data(8bit)
	i2C_WriteData(i2cdata[0], i2cdata[1]);	

	return 1;
}

static int NAU8822_SetDACVolume(unsigned char ucLeftVol, unsigned char ucRightVol)
{	//0~31
	
	/* R11 DACOUTL R12 DACOUTR, R52 HPOUTL R53 HPOUTR, R54 SPOUTL R55 SPOUTR */
	unsigned short data;
	if (ucLeftVol!=0)
		data = (1<<8) | (ucLeftVol*4 + 130);
	else
		data = (1<<8);
	NAU8822_WriteData(11, data);
		
	if (ucRightVol!=0)
		data = (1<<8) | (ucRightVol*4 + 130);
	else
		data = (1<<8);
	NAU8822_WriteData(12, data);
	
	//HeadPhone
	if (ucLeftVol!=0)	//0~127
		data = (1<<8) | (ucLeftVol + 32);
	else
		data = (1<<8);
	NAU8822_WriteData(52, data);
		
	if (ucRightVol!=0)
		data = (1<<8) | (ucRightVol + 32);
	else
		data = (1<<8);
	NAU8822_WriteData(53, data);
	
	//Speaker	
	if (ucLeftVol!=0)
		data = (1<<8) | (ucLeftVol + 32);
	else
		data = (1<<8);
	data = 0x13f - 6;
	NAU8822_WriteData(54, data);
		
	if (ucRightVol!=0)
		data = (1<<8) | (ucRightVol + 32);
	else
		data = (1<<8);
		
	data = 0x13f - 6;
	NAU8822_WriteData(55, data);
		
	return 0;
}	

static int NAU8822_SetADCVolume(unsigned char ucLeftVol, unsigned char ucRightVol)
{
	unsigned short data=0;
	
	if (ucLeftVol<=8)
		data = 0x100 | ucLeftVol;
	else if (ucLeftVol<=16&&ucLeftVol>8)
		data = 0x100 | ucLeftVol * 4;
	else if (ucLeftVol<=31&&ucLeftVol>16)
		data = 0x100 | (ucLeftVol * 8 + 7);		
		
	data = 0x1ff;
	NAU8822_WriteData(15, data);
			
	if (ucRightVol<=8)
		data = 0x100 | ucRightVol;
	else if (ucRightVol<=16&&ucRightVol>8)
		data = 0x100 | ucRightVol * 4;
	else if (ucRightVol<=31&&ucRightVol>16)
		data = 0x100 | (ucRightVol * 8 + 7);	
	
	data = 0x1ff;
	NAU8822_WriteData(16, data);
	
	return 0;
}
//======================================================================
//======================================================================
void NAU8822_DACSetup(void)
{	
//  printk("==NAU8822_DAC_Setup==\n");
	NAU8822_WriteData(0, 0x000);
	Delay(0x100);
	NAU8822_WriteData(1, 0x01F);//R1 OUT4MIXEN OUT3MIXEN MICBEN BIASEN BUFIOEN VMIDSEL
	NAU8822_WriteData(2, 0x1BF);//R2 power up ROUT1 LOUT1 
	NAU8822_WriteData(3, 0x06F);//R3 OUT4EN OUT3EN SPKNEN SPKPEN RMIXEN LMIXEN DACENR DACENL		
	NAU8822_WriteData(4, 0x010);//R4 select audio format(I2S format) and word length (16bits)
	NAU8822_WriteData(5, 0x000);//R5 companding ctrl and loop back mode (all disable)
	NAU8822_WriteData(6, 0x000);//R6 clock Gen ctrl(slave mode)
	NAU8822_WriteData(10, 0x008);//R10 DAC control (softmute disable, oversample select 64x (lowest power) )
	NAU8822_WriteData(43, 0x010);//For differential speaker
	NAU8822_WriteData(45, 0x139);//R10 DAC control (softmute disable, oversample select 64x (lowest power) )
	NAU8822_WriteData(50, 0x001);//R50 DACL2LMIX
	NAU8822_WriteData(51, 0x001);//R51 DACR2RMIX
	NAU8822_WriteData(49, 0x002);
}

void NAU8822_ADCSetup(void)
{	
//  printk("==NAU8822_ADC_Setup==\n");
	NAU8822_WriteData(0, 0x000);
	Delay(0x100);
	NAU8822_WriteData(1, 0x01F);//R1 MICBEN BIASEN BUFIOEN VMIDSEL
	NAU8822_WriteData(2, 0x03F);//R2 power up ROUT1 LOUT1 
	NAU8822_WriteData(4, 0x010);//R4 select audio format(I2S format) and word length (16bits)
	NAU8822_WriteData(5, 0x000);//R5 companding ctrl and loop back mode (all disable)
	NAU8822_WriteData(6, 0x000);//R6 clock Gen ctrl(slave mode)
	NAU8822_WriteData(35, 0x008);//R35 enable noise gate		
	NAU8822_WriteData(3, 0x00C); //R3 bypass		
	NAU8822_WriteData(45, 0x13f);//R45 PGA
	NAU8822_WriteData(46, 0x13f);//R46 PGA
}

void NAU8822_ADC_DAC_Setup(void)
{	
//  printk("==NAU8822_ADC_DAC_Setup==\n");
	// ADC		
	NAU8822_WriteData(0, 0x000);
	Delay(0x100);
	NAU8822_WriteData(1, 0x01F);//R1 MICBEN BIASEN BUFIOEN VMIDSEL
	NAU8822_WriteData(4, 0x010);//R4 select audio format(I2S format) and word length (16bits)
	NAU8822_WriteData(5, 0x000);//R5 companding ctrl and loop back mode (all disable)
	NAU8822_WriteData(6, 0x000);//R6 clock Gen ctrl(slave mode)
	NAU8822_WriteData(35, 0x000);//R35 disable noise gate		
	NAU8822_WriteData(46, 0x13f);//R46 PGA
	
//#define OPT_LINE_IN
#ifdef OPT_LINE_IN	
	NAU8822_WriteData(47, 0x106);//left AUX line in, -3dB
	NAU8822_WriteData(48, 0x106);//right AUX line in, -3dB
#endif	

	// DAC
	NAU8822_WriteData(2, 0x1BF);//R2 power up ROUT1 LOUT1 
	NAU8822_WriteData(3, 0x06F);//R3 OUT4EN OUT3EN SPKNEN SPKPEN RMIXEN LMIXEN DACENR DACENL		
	NAU8822_WriteData(10, 0x008);//R10 DAC control (softmute disable, oversample select 64x (lowest power) )
	NAU8822_WriteData(43, 0x010);//For differential speaker
	NAU8822_WriteData(45, 0x139);//R10 DAC control (softmute disable, oversample select 64x (lowest power) )
	NAU8822_WriteData(50, 0x001);//R50 DACL2LMIX
	NAU8822_WriteData(51, 0x001);//R51 DACR2RMIX
	NAU8822_WriteData(49, 0x002);
}

void NAU8822_Init(void)
{
	// open I2C pins in GPIO mode
	outp32(REG_GPBFUN, inp32(REG_GPBFUN) & ~(MF_GPB14+MF_GPB13));	// set GPB_13/GPB_14 to GPIO pins
	outp32(REG_GPIOB_OMD, inp32(REG_GPIOB_OMD)|(BIT13 + BIT14));	// set GPB_13/GPB_14 to output mode
	outp32(REG_GPIOB_PUEN, inp32(REG_GPIOB_PUEN)|(BIT13 + BIT14));	// set GPB_13/GPB_14 pullup pin enabled
	outp32(REG_GPIOB_DOUT, inp32(REG_GPIOB_DOUT)|(BIT13 + BIT14));	// set GPB_13/GPB_14 to high
}

int  NAU8822_SetPlayVolume(unsigned int ucVolL, unsigned int ucVolR)
{    
//  printk("NAU8822SetPlayVolume = L:%d R:%d\n", ucVolL, ucVolR);
	NAU8822_SetDACVolume((unsigned char)ucVolL, (unsigned char)ucVolR);
	return 0;
}

int  NAU8822_SetRecVolume(unsigned int ucVolL, unsigned int ucVolR)
{
//  printk("NAU8822SetRecVolume = L:%d R:%d\n", ucVolL, ucVolR);
    NAU8822_SetADCVolume((unsigned char)ucVolL, (unsigned char)ucVolR);
	return 0;
}

void NAU8822_SetSampleFreq(int uSample_Rate)
{
	unsigned short data=0;
	
	switch (uSample_Rate)
	{
		case eDRVI2S_FREQ_8000:							//8KHz
			data =WM_8000 ;
			NAU8822_WriteData(6,0x20);	//only support 256fs
			break;
		case eDRVI2S_FREQ_11025:					//11.025KHz
			data = WM_11025 ;
			NAU8822_WriteData(6,0x20);	//only support 256fs
			break;
		case eDRVI2S_FREQ_16000:						//16KHz
			data = WM_16000 ;
			NAU8822_WriteData(6,0x20);	//only support 256fs
			break;
		case eDRVI2S_FREQ_22050:					//22.05KHz
			data = WM_22050;
			break;
		case eDRVI2S_FREQ_24000:						//24KHz
			data = WM_24000;
			break;
		case eDRVI2S_FREQ_32000:						//32KHz
			data = WM_32000 ;
			NAU8822_WriteData(6,0x20);	//only support 256fs
			break;
		case eDRVI2S_FREQ_44100:					//44.1KHz
			data = WM_44100 ;
			NAU8822_WriteData(6,0x20);	//only support 256fs
			break;
		case eDRVI2S_FREQ_48000:						//48KHz
			data = WM_48000;
			break;
		default:break;
	}
	NAU8822_WriteData(7,data|1); 	
}
