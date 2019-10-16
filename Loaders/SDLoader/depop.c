#include <string.h>
#include "wblib.h"
#include "turbowriter.h"

//#if defined(__IFLYTEK__) || defined(__NUVOTON_V4__)
//====================================================================
//
//	__IFLYTEK__ use GPIOA 4 to control depop circuit
//	GPIOA_4 = H, Force earphone to ground (depop)
//	GPIOA_4 = L, No-depop
//
//
//	__NUVOTON_V4__ use GPIOD 0 to control depop circuit
//	GPIOD_0 = H, Force earphone to ground (depop)
//	GPIOD_0 = L, No-depop
//
//====================================================================

UINT8 DacOnOffLevel;
void _sysDelay(UINT32 k)
{
	volatile UINT32 j=k*10000;
	while(j--);
}
#if 1
void spuDacOn(UINT8 level)
{
	outp32(REG_AHBCLK, inp32(REG_AHBCLK) &(~( ADO_CKE | SPU_CKE | HCLK4_CKE)));	// disable SPU engine clock
	outp32(REG_AHBCLK, inp32(REG_AHBCLK) | ADO_CKE | SPU_CKE | HCLK4_CKE);		// enable SPU engine clock

	outp32(REG_SPU_CTRL, inp32(REG_SPU_CTRL) & ~SPURST);

	outp32(REG_SPU_CTRL, inp32(REG_SPU_CTRL) | SPURST);
	outp32(REG_SPU_CTRL, inp32(REG_SPU_CTRL) & ~SPURST);

	DacOnOffLevel = level;

	outpw(REG_SPU_DAC_PAR, inpw(REG_SPU_DAC_PAR) | 0x30);		//disable
	if(level == 3)
		outpw(REG_SPU_DAC_PAR, inpw(REG_SPU_DAC_PAR) & ~0x30);	//delay time, p0=3s
	else if(level == 1)
		outpw(REG_SPU_DAC_PAR, inpw(REG_SPU_DAC_PAR) & ~0x20);	//delay time, p0=0.5-1s
	else if(level == 2)
		outpw(REG_SPU_DAC_PAR, inpw(REG_SPU_DAC_PAR) & ~0x10);	//delay time, p0=2s
	else
	{
		outpw(REG_SPU_DAC_VOL, inpw(REG_SPU_DAC_VOL) & ~0x00FF0000);	//P7
		outpw(REG_SPU_DAC_PAR, inpw(REG_SPU_DAC_PAR) | 0x30);		//disable
		return;
	}

	outpw(REG_SPU_DAC_VOL, inpw(REG_SPU_DAC_VOL) & ~0x0800000);	//P7
	_sysDelay(1);
	outpw(REG_SPU_DAC_VOL, inpw(REG_SPU_DAC_VOL) & ~0x0400000);	//P6
	_sysDelay(1);
	outpw(REG_SPU_DAC_VOL, inpw(REG_SPU_DAC_VOL) & ~0x01e0000);	//P1-4
	_sysDelay(1);
	outpw(REG_SPU_DAC_VOL, inpw(REG_SPU_DAC_VOL) & ~0x0200000);	//P5
	_sysDelay(1);

	outpw(REG_SPU_DAC_VOL, inpw(REG_SPU_DAC_VOL) & ~0x00010000);	//P0
#if 0
	if(level == 3)		//modify this delay time to meet the product request
		_sysDelay(300);
	else if(level == 1)
		_sysDelay(70);
	else if(level == 2)
		_sysDelay(200);
#endif
}
#else
void spuDacOn(UINT8 level)
{
	outp32(REG_AHBCLK, inp32(REG_AHBCLK) &(~( ADO_CKE | SPU_CKE | HCLK4_CKE)));	// disable SPU engine clock
	outp32(REG_AHBCLK, inp32(REG_AHBCLK) | ADO_CKE | SPU_CKE | HCLK4_CKE);		// enable SPU engine clock

	outp32(REG_SPU_CTRL, inp32(REG_SPU_CTRL) & ~SPURST);

	outp32(REG_SPU_CTRL, inp32(REG_SPU_CTRL) | SPURST);
	outp32(REG_SPU_CTRL, inp32(REG_SPU_CTRL) & ~SPURST);

	DacOnOffLevel = level;

	outpw(REG_SPU_DAC_PAR, inpw(REG_SPU_DAC_PAR) | 0x30);		//disable
	if(level == 3)
		outpw(REG_SPU_DAC_PAR, inpw(REG_SPU_DAC_PAR) & ~0x30);	//delay time, p0=3s
	else if(level == 1)
		outpw(REG_SPU_DAC_PAR, inpw(REG_SPU_DAC_PAR) & ~0x20);	//delay time, p0=0.5-1s
	else if(level == 2)
		outpw(REG_SPU_DAC_PAR, inpw(REG_SPU_DAC_PAR) & ~0x10);	//delay time, p0=2s
	else
	{
		outpw(REG_SPU_DAC_VOL, inpw(REG_SPU_DAC_VOL) & ~0x00FF0000);	//P7
		outpw(REG_SPU_DAC_PAR, inpw(REG_SPU_DAC_PAR) | 0x30);		//disable
		return;
	}

	outpw(REG_SPU_DAC_VOL, inpw(REG_SPU_DAC_VOL) & ~0x0800000);	//P7
	_sysDelay(1);
	outpw(REG_SPU_DAC_VOL, inpw(REG_SPU_DAC_VOL) & ~0x0400000);	//P6
	_sysDelay(1);
	outpw(REG_SPU_DAC_VOL, inpw(REG_SPU_DAC_VOL) & ~0x01e0000);	//P1-4
	_sysDelay(1);
	outpw(REG_SPU_DAC_VOL, inpw(REG_SPU_DAC_VOL) & ~0x0200000);	//P5
	_sysDelay(1);

	outpw(REG_SPU_DAC_VOL, inpw(REG_SPU_DAC_VOL) & ~0x00010000);	//P0

	if(level == 3)		//modify this delay time to meet the product request
		_sysDelay(300);
	else if(level == 1)
		_sysDelay(70);
	else if(level == 2)
		_sysDelay(200);
}


#endif
#if 0
	int gpioOpen(unsigned char port)
	{

		switch (port) {
			case GPIO_PORTC:
				rPINFUN2 |= (1 << 13);
				break;
			case GPIO_PORTD:
				rPINFUN |= (1 << 16);
				break;
			case GPIO_PORTA:
			case GPIO_PORTB:
			case GPIO_PORTE:
				break;
			default:
				return(-1);

		}
		return(0);
	}

	int gpioReadport(unsigned char port, unsigned short *val)
	{
		switch (port) {

			case GPIO_PORTA:
				*val = (rGPIOA_PIN & 0xffff);
				break;
			case GPIO_PORTB:
				*val = (rGPIOB_PIN & 0xffff);
				break;
			case GPIO_PORTC:
				*val = (rGPIOC_PIN & 0xff);
				break;
			case GPIO_PORTD:
				*val = (rGPIOD_PIN & 0xf);
				break;
			case GPIO_PORTE:
				*val = (rGPIOE_PIN & 0xffff);
				break;
			default:
				return(-1);

		}
		return(0);
	}

	int gpioSetportdir(unsigned char port, unsigned short mask, unsigned short dir)
	{
		switch (port) {

			case GPIO_PORTA:
				rGPIOA_OMD &= ~(mask & (mask ^ dir));
				rGPIOA_OMD |= (mask & dir);
				break;
			case GPIO_PORTB:
				rGPIOB_OMD &= ~(mask & (mask ^ dir));
				rGPIOB_OMD |= (mask & dir);
				break;
			case GPIO_PORTC:
				rGPIOC_OMD &= ~(mask & (mask ^ dir));
				rGPIOC_OMD |= (mask & dir);
				break;
			case GPIO_PORTD:
				rGPIOD_OMD &= ~(mask & (mask ^ dir));
				rGPIOD_OMD |= (mask & dir);
				break;
			case GPIO_PORTE:
				rGPIOE_OMD &= ~(mask & (mask ^ dir));
				rGPIOE_OMD |= (mask & dir);
				break;
			default:
				return(-1);

		}
		return(0);


	}

	int gpioSetportval(unsigned char port, unsigned short mask, unsigned short val)
	{
		switch (port) {

			case GPIO_PORTA:
				rGPIOA_DOUT &= ~(mask & (mask ^ val));
				rGPIOA_DOUT |= (mask & val);
				break;
			case GPIO_PORTB:
				rGPIOB_DOUT &= ~(mask & (mask ^ val));
				rGPIOB_DOUT |= (mask & val);
				break;
			case GPIO_PORTC:
				rGPIOC_DOUT &= ~(mask & (mask ^ val));
				rGPIOC_DOUT |= (mask & val);
				break;
			case GPIO_PORTD:
				rGPIOD_DOUT &= ~(mask & (mask ^ val));
				rGPIOD_DOUT |= (mask & val);
				break;
			case GPIO_PORTE:
				rGPIOE_DOUT &= ~(mask & (mask ^ val));
				rGPIOE_DOUT |= (mask & val);
				break;
			default:
				return(-1);

		}
		return(0);

	}

	int gpioSetportpull(unsigned char port, unsigned short mask, unsigned short pull)
	{
		switch (port) {

			case GPIO_PORTA:
				rGPIOA_PUEN &= ~(mask & (mask ^ pull));
				rGPIOA_PUEN |= (mask & pull);
				break;
			case GPIO_PORTB:
				rGPIOB_PUEN &= ~(mask & (mask ^ pull));
				rGPIOB_PUEN |= (mask & pull);
				break;
			case GPIO_PORTC:
				rGPIOC_PUEN &= ~(mask & (mask ^ pull));
				rGPIOC_PUEN |= (mask & pull);
				break;
			case GPIO_PORTE:
				rGPIOE_PUEN &= ~(mask & (mask ^ pull));
				rGPIOE_PUEN |= (mask & pull);
				break;
			default:
				return(-1);

		}
		return(0);

	}

	void dePopStart(void)
	{
	#if defined(__IFLYTEK__)
		gpioOpen(GPIO_PORTA);
		gpioSetportval(GPIO_PORTA, 0x10, 0x10);        //GPIOA4 to H
		gpioSetportpull(GPIO_PORTA, 0x10, 0x10);		//GPIOA4 pull high
		gpioSetportdir(GPIO_PORTA, 0x10, 0x10);		//GPIOA4 output
	#endif
	#if defined(__NUVOTON_V4__)
		gpioOpen(GPIO_PORTD);
		gpioSetportval(GPIO_PORTD, 0x1, 0x1);        	//GPIOD0 to H
		gpioSetportpull(GPIO_PORTD, 0x1, 0x1);			//GPIOD0 pull high
		gpioSetportdir(GPIO_PORTD, 0x1, 0x1);			//GPIOD0 output
	#endif
	}

	void dePopEnd(void)
	{
	#if defined(__IFLYTEK__)
		gpioSetportval(GPIO_PORTA, 0x10, 0x00);        //GPIOA4 to L
	#endif
	#if defined(__NUVOTON_V4__)
		gpioSetportval(GPIO_PORTD, 0x1, 0x00);        //GPIOD0 to L
	#endif
	}
#endif
