/**************************************************************************//**
 * @file     libgpio.c
 * @version  V3.00
 * @brief    N9H20 series GPIO driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wblib.h"
#include "N9H20_GPIO.h"

// accetiable debounce clock
static const unsigned int _clk[16] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 2*256, 4*256, 8*256, 16*256, 32*256, 64*256, 128*256};

int gpio_open(unsigned char port)
{

	switch (port) {			
		case GPIO_PORTA:
			outpw(REG_GPAFUN , inpw(REG_GPAFUN) &~ (0xF00000));
			break;			
		case GPIO_PORTD:
			outpw(REG_GPDFUN , inpw(REG_GPDFUN) &~ (0xFF0003FF));
			break;
		case GPIO_PORTE:
			outpw(REG_GPEFUN , inpw(REG_GPEFUN) &~ (0xFF0));
			break;	
		case GPIO_PORTB:				
		case GPIO_PORTC:			
			break;	
		default:
			return(-1);

	}
	return(0);

}

int gpio_configure(unsigned char port, unsigned short num)
{

	switch (port) {			
		case GPIO_PORTA:
			if(num<=11)
				outpw(REG_GPAFUN , inpw(REG_GPAFUN) &~ (0x3 << (num<<1)));
			else
				return(-1);
			break;	
		case GPIO_PORTB:
			outpw(REG_GPBFUN , inpw(REG_GPBFUN) &~ (0x3 << (num<<1)));
			break;
		case GPIO_PORTC:
			outpw(REG_GPCFUN , inpw(REG_GPCFUN) &~ (0x3 << (num<<1)));			
			break;
		case GPIO_PORTD:
			outpw(REG_GPDFUN , inpw(REG_GPDFUN) &~ (0x3 << (num<<1)));			
			break;
		case GPIO_PORTE:
			if(num<=11)
				outpw(REG_GPEFUN , inpw(REG_GPEFUN) &~ (0x3 << (num<<1)));		
			else
				return(-1);
			break;	
			
		default:
			return(-1);

	}
	return(0);

}

int gpio_readport(unsigned char port, unsigned short *val)
{
	switch (port) {
	
		case GPIO_PORTA:
			*val = (inpw(REG_GPIOA_PIN) & 0x0fff);
			break;
		case GPIO_PORTB:
			*val = (inpw(REG_GPIOB_PIN) & 0xffff);
			break;			
		case GPIO_PORTC:
			*val = (inpw(REG_GPIOC_PIN) & 0xffff);
			break;		
		case GPIO_PORTD:
			*val = (inpw(REG_GPIOD_PIN) & 0xffff);
			break;	
		case GPIO_PORTE:
			*val = (inpw(REG_GPIOE_PIN) & 0x0fff);
			break;		
		default:
			return(-1);

	}
	return(0);
}

int gpio_setportdir(unsigned char port, unsigned short mask, unsigned short dir)
{
	switch (port) {

		case GPIO_PORTA:
			outpw(REG_GPIOA_OMD , inpw(REG_GPIOA_OMD) & ~(mask & (mask ^ dir)));
			outpw(REG_GPIOA_OMD , inpw(REG_GPIOA_OMD) | (mask & dir));			
			break;
		case GPIO_PORTB:
			outpw(REG_GPIOB_OMD , inpw(REG_GPIOB_OMD) & ~(mask & (mask ^ dir)));
			outpw(REG_GPIOB_OMD , inpw(REG_GPIOB_OMD) | (mask & dir));	
			break;			
		case GPIO_PORTC:
			outpw(REG_GPIOC_OMD , inpw(REG_GPIOC_OMD) & ~(mask & (mask ^ dir)));
			outpw(REG_GPIOC_OMD , inpw(REG_GPIOC_OMD) | (mask & dir));	
			break;		
		case GPIO_PORTD:
			outpw(REG_GPIOD_OMD , inpw(REG_GPIOD_OMD) & ~(mask & (mask ^ dir)));
			outpw(REG_GPIOD_OMD , inpw(REG_GPIOD_OMD) | (mask & dir));	
			break;	
		case GPIO_PORTE:
			outpw(REG_GPIOE_OMD , inpw(REG_GPIOE_OMD) & ~(mask & (mask ^ dir)));
			outpw(REG_GPIOE_OMD , inpw(REG_GPIOE_OMD) | (mask & dir));	
			break;		
		default:
			return(-1);

	}
	return(0);
	

}

int gpio_setportval(unsigned char port, unsigned short mask, unsigned short val)
{
	switch (port) {
	
		case GPIO_PORTA:
			outpw(REG_GPIOA_DOUT , inpw(REG_GPIOA_DOUT) & ~(mask & (mask ^ val)));
			outpw(REG_GPIOA_DOUT , inpw(REG_GPIOA_DOUT) | (mask & val));			
			break;
		case GPIO_PORTB:
			outpw(REG_GPIOB_DOUT , inpw(REG_GPIOB_DOUT) & ~(mask & (mask ^ val)));
			outpw(REG_GPIOB_DOUT , inpw(REG_GPIOB_DOUT) | (mask & val));
			break;			
		case GPIO_PORTC:
			outpw(REG_GPIOC_DOUT , inpw(REG_GPIOC_DOUT) & ~(mask & (mask ^ val)));
			outpw(REG_GPIOC_DOUT , inpw(REG_GPIOC_DOUT) | (mask & val));
			break;		
		case GPIO_PORTD:
			outpw(REG_GPIOD_DOUT , inpw(REG_GPIOD_DOUT) & ~(mask & (mask ^ val)));
			outpw(REG_GPIOD_DOUT , inpw(REG_GPIOD_DOUT) | (mask & val));
			break;	
		case GPIO_PORTE:
			outpw(REG_GPIOE_DOUT , inpw(REG_GPIOE_DOUT) & ~(mask & (mask ^ val)));
			outpw(REG_GPIOE_DOUT , inpw(REG_GPIOE_DOUT) | (mask & val));
			break;		
		default:
			return(-1);

	}
	return(0);

}

int gpio_setportpull(unsigned char port, unsigned short mask, unsigned short pull)
{
	switch (port) {
	
		case GPIO_PORTA:
			outpw(REG_GPIOA_PUEN , inpw(REG_GPIOA_PUEN) & ~(mask & (mask ^ pull)));
			outpw(REG_GPIOA_PUEN , inpw(REG_GPIOA_PUEN) | (mask & pull));			
			break;
		case GPIO_PORTB:
			outpw(REG_GPIOB_PUEN , inpw(REG_GPIOB_PUEN) & ~(mask & (mask ^ pull)));
			outpw(REG_GPIOB_PUEN , inpw(REG_GPIOB_PUEN) | (mask & pull));	
			break;			
		case GPIO_PORTC:
			outpw(REG_GPIOC_PUEN , inpw(REG_GPIOC_PUEN) & ~(mask & (mask ^ pull)));
			outpw(REG_GPIOC_PUEN , inpw(REG_GPIOC_PUEN) | (mask & pull));	
			break;	
		case GPIO_PORTD:
			outpw(REG_GPIOD_PUEN , inpw(REG_GPIOD_PUEN) & ~(mask & (mask ^ pull)));
			outpw(REG_GPIOD_PUEN , inpw(REG_GPIOD_PUEN) | (mask & pull));	
			break;
		case GPIO_PORTE:
			outpw(REG_GPIOE_PUEN , inpw(REG_GPIOE_PUEN) & ~(mask & (mask ^ pull)));
			outpw(REG_GPIOE_PUEN , inpw(REG_GPIOE_PUEN) | (mask & pull));	
			break;		
		default:
			return(-1);

	}
	return(0);

}

int gpio_setdebounce(unsigned int clk, unsigned char src)
{
	int i;
	
	if(clk > 128*256 || src > 0xf)
		return(-1);
	
	// clk could only be 1, 2, 4, 8, ... 128*256
	for(i = 0 ; i < 16; i++) {
		if(_clk[i] == clk)
			break;
	}
	if(i == 16)
		return(-1);

	outpw(REG_DBNCECON , (i << 4 | src));
		
	return(0);				
}

void gpio_getdebounce(unsigned int *clk, unsigned char *src)
{
	*clk = _clk[(inpw(REG_DBNCECON) >> 4) & 0xf];
	*src = inpw(REG_DBNCECON) & 0xf;
	
	return;
}

int gpio_setsrcgrp(unsigned char port, unsigned short mask, unsigned char irq)
{

	const unsigned int _irq[4] = {0, 0x55555555, 0xaaaaaaaa, 0xffffffff};
	unsigned int _mask = 0;
	int i;
	
	
	if(irq > 3)
		return(-1);	
	
	if(mask > 0xffff)
		return(-1);
		
	for(i = 0; i < 16; i++) {
		if(mask & (1 << i))
			_mask += (3 << (i << 1));	
	}
		
	switch (port) {	
		case GPIO_PORTA:
			outpw(REG_IRQSRCGPA , inpw(REG_IRQSRCGPA) & ~_mask);
			outpw(REG_IRQSRCGPA , inpw(REG_IRQSRCGPA) | (_mask & _irq[irq]));			
			break;
		case GPIO_PORTB:
			outpw(REG_IRQSRCGPB , inpw(REG_IRQSRCGPB) & ~_mask);
			outpw(REG_IRQSRCGPB , inpw(REG_IRQSRCGPB) | (_mask & _irq[irq]));
			break;			
		case GPIO_PORTC:
			outpw(REG_IRQSRCGPC , inpw(REG_IRQSRCGPC) & ~_mask);
			outpw(REG_IRQSRCGPC , inpw(REG_IRQSRCGPC) | (_mask & _irq[irq]));
			break;	
		case GPIO_PORTD:
			outpw(REG_IRQSRCGPD , inpw(REG_IRQSRCGPD) & ~_mask);
			outpw(REG_IRQSRCGPD , inpw(REG_IRQSRCGPD) | (_mask & _irq[irq]));
			break;
		case GPIO_PORTE:
			outpw(REG_IRQSRCGPE , inpw(REG_IRQSRCGPE) & ~_mask);
			outpw(REG_IRQSRCGPE , inpw(REG_IRQSRCGPE) | (_mask & _irq[irq]));
			break;		
		default:
			return(-1);

	}
	return(0);
}

int gpio_getsrcgrp(unsigned char port, unsigned int *val)
{
	switch (port) {	
		case GPIO_PORTA:		
			*val = inpw(REG_IRQSRCGPA);
			break;
		case GPIO_PORTB:
			*val = inpw(REG_IRQSRCGPB);
			break;			
		case GPIO_PORTC:
			*val = inpw(REG_IRQSRCGPC);
			break;	
		case GPIO_PORTD:
			*val = inpw(REG_IRQSRCGPD);
			break;
		case GPIO_PORTE:			
			*val = inpw(REG_IRQSRCGPE);
			break;		
		default:
			return(-1);

	}
	return(0);

}

int gpio_setintmode(unsigned char port, unsigned short mask, unsigned short falling, unsigned short rising)
{
	
	if(mask > 0xffff)
		return(-1);
		
	switch (port) {	
		case GPIO_PORTA:	
			outpw(REG_IRQENGPA , inpw(REG_IRQENGPA) & ~((mask << 16) | mask));
			outpw(REG_IRQENGPA , inpw(REG_IRQENGPA) | (((mask & rising) << 16) | (mask & falling)));			
			break;
		case GPIO_PORTB:
			outpw(REG_IRQENGPB , inpw(REG_IRQENGPB) & ~((mask << 16) | mask));
			outpw(REG_IRQENGPB , inpw(REG_IRQENGPB) | (((mask & rising) << 16) | (mask & falling)));
			break;			
		case GPIO_PORTC:
			outpw(REG_IRQENGPC , inpw(REG_IRQENGPC) & ~((mask << 16) | mask));
			outpw(REG_IRQENGPC , inpw(REG_IRQENGPC) | (((mask & rising) << 16) | (mask & falling)));
			break;	
		case GPIO_PORTD:
			outpw(REG_IRQENGPD , inpw(REG_IRQENGPD) & ~((mask << 16) | mask));
			outpw(REG_IRQENGPD , inpw(REG_IRQENGPD) | (((mask & rising) << 16) | (mask & falling)));
			break;	
		case GPIO_PORTE:
			outpw(REG_IRQENGPE , inpw(REG_IRQENGPE) & ~((mask << 16) | mask));
			outpw(REG_IRQENGPE , inpw(REG_IRQENGPE) | (((mask & rising) << 16) | (mask & falling)));		
			break;		
		default:
			return(-1);

	}
	return(0);		
		
}


int gpio_getintmode(unsigned char port, unsigned short *falling, unsigned short *rising)
{

	switch (port) {	
		case GPIO_PORTA:		
			*rising = inpw(REG_IRQENGPA) >> 16;
			*falling = inpw(REG_IRQENGPA) & 0xffff;
			break;
		case GPIO_PORTB:
			*rising = inpw(REG_IRQENGPB) >> 16;
			*falling = inpw(REG_IRQENGPB) & 0xffff;
			break;			
		case GPIO_PORTC:
			*rising = inpw(REG_IRQENGPC) >> 16;
			*falling = inpw(REG_IRQENGPC) & 0xffff;
			break;	
		case GPIO_PORTD:
			*rising = inpw(REG_IRQENGPD) >> 16;
			*falling = inpw(REG_IRQENGPD) & 0xffff;
			break;
		case GPIO_PORTE:
			*rising = inpw(REG_IRQENGPE)  >> 16;
			*falling = inpw(REG_IRQENGPE) & 0xffff;		
			break;		
		default:
			return(-1);

	}
	return(0);
}


int gpio_setlatchtrigger(unsigned char src)
{
	if(src > 0xf)
		return(-1);

	outpw(REG_IRQLHSEL , src);		
	return(0);

}

void gpio_getlatchtrigger(unsigned char *src)
{

	*src = inpw(REG_IRQLHSEL) & 0xf;
	return;
}

int gpio_getlatchval(unsigned char port, unsigned short *val)
{

	switch (port) {	
		case GPIO_PORTA:		
			*val = inpw(REG_IRQLHGPA) & 0xffff;
			break;
		case GPIO_PORTB:
			*val = inpw(REG_IRQLHGPB) & 0xffff;
			break;			
		case GPIO_PORTC:
			*val = inpw(REG_IRQLHGPC) & 0xffff;
			break;	
		case GPIO_PORTD:
			*val = inpw(REG_IRQLHGPD) & 0xffff;
			break;
		case GPIO_PORTE:
			*val = inpw(REG_IRQLHGPE) & 0xffff;
			break;		
		default:
			return(-1);

	}

	return(0);
}


int gpio_gettriggersrc(unsigned char port, unsigned short *src)
{

	switch (port) {	
		case GPIO_PORTA:		
			*src = inpw(REG_IRQTGSRC0) & 0xffff;						
			break;
		case GPIO_PORTB:
			*src = (inpw(REG_IRQTGSRC0) & 0xffff0000) >> 16;						
			break;			
		case GPIO_PORTC:
			*src = inpw(REG_IRQTGSRC1) & 0xffff;						
			break;	
		case GPIO_PORTD:
			*src = (inpw(REG_IRQTGSRC1) & 0xffff0000) >> 16;						
			break;	
		case GPIO_PORTE:
			*src = inpw(REG_IRQTGSRC2) & 0xffff;						
			break;		
		default:
			return(-1);

	}

	return(0);
}

int gpio_cleartriggersrc(unsigned char port)
{

	switch (port) {	
		case GPIO_PORTA:					
			outpw(REG_IRQTGSRC0 , inpw(REG_IRQTGSRC0) & 0xffff);			
			break;
		case GPIO_PORTB:			
			outpw(REG_IRQTGSRC0 , inpw(REG_IRQTGSRC0) & 0xffff0000);			
			break;			
		case GPIO_PORTC:			
			outpw(REG_IRQTGSRC1 , inpw(REG_IRQTGSRC1) & 0xffff);			
			break;	
		case GPIO_PORTD:			
			outpw(REG_IRQTGSRC1 , inpw(REG_IRQTGSRC1) & 0xffff0000);			
			break;	
		case GPIO_PORTE:			
			outpw(REG_IRQTGSRC2 , inpw(REG_IRQTGSRC2) & 0xffff);			
			break;		
		default:
			return(-1);

	}

	return(0);
}


