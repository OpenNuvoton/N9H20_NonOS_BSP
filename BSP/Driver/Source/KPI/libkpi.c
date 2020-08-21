/**************************************************************************//**
 * @file     libkpi.c
 * @version  V3.00
 * @brief    N9H20 series keypad driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include "N9H20_GPIO.h"
#include "N9H20_KPI.h"

// latest key pressed recorded by ISR, might _not_ be the most updated status
static unsigned int _key;
// interrupt number for kpi
static unsigned char _int;
static unsigned char _opened = 0;

int key_map[KEY_COUNT] = {
        KEY_UP, KEY_LEFT, KEY_ESC, KEY_DOWN, KEY_RIGHT, KEY_ENTER
};

static unsigned int readkey(void)
{
	unsigned int i, read0, read1, new_key=0;
	gpio_readport(GPIO_PORTA, (unsigned short *)&read0);
	read0 &= 0x1C;
	if(read0 == 0x1C)
		return(0);
	gpio_setportval(GPIO_PORTA, (1 << 7), (1 << 7));
	gpio_readport(GPIO_PORTA, (unsigned short *)&read1);	
	read1 &= 0x1C;
	if(read1 == 0x1C)
		read1 = (read0 ^ read1)>>2;
	else
		read1 = ((~read0 & 0x1C)<<1);
	
	gpio_setportval(GPIO_PORTA, (1 << 7), 0);

	for(i=0; i<KEY_COUNT; i++)
	{
		if (read1 & (1 << i))
		{
			new_key |= key_map[i];
		}
	}	
	
	return new_key;
}

static void kpi_isr(void)
{	
	_key = readkey();
	//(*(unsigned int volatile *)REG_AIC_SCCR) = (1 << (_int + 2));
	gpio_cleartriggersrc(GPIO_PORTA);
	
	return;

}

void kpi_init(void)
{

	_opened = 1;
	
	// PORTA[2-4]
	gpio_setportdir(GPIO_PORTA, ((1 << 2) | (1 << 3) | (1 << 4)), 0);
	gpio_setportpull(GPIO_PORTA, ((1 << 2) | (1 << 3) | (1 << 4)), 0);	
	gpio_setintmode(GPIO_PORTA, ((1 << 2) | (1 << 3) | (1 << 4)), 
					((1 << 2) | (1 << 3) | (1 << 4)), ((1 << 2) | (1 << 3) | (1 << 4)));
        
	// PORTA[7]	
	gpio_setportval(GPIO_PORTA, (1 << 7), 0);
	gpio_setportpull(GPIO_PORTA, (1 << 7), 0);
	gpio_setportdir(GPIO_PORTA, (1 << 7), (1 << 7));

    return;    
}

int kpi_open(unsigned int src)
{
	if(_opened == 0)
		kpi_init();
	if(_opened != 1)
		return(-1);
	
	_opened = 2;
	if(src > 3)
		return(-1);
		
	_int = src;
	
	gpio_setsrcgrp(GPIO_PORTA, ((1 << 2) | (1 << 3) | (1 << 4)), src);

	gpio_setdebounce(128, 1 << src);
	gpio_setlatchtrigger(1 << src);

	sysInstallISR(IRQ_LEVEL_7, (INT_SOURCE_E)(src + 2), (PVOID)kpi_isr);	
		
	sysSetInterruptType((INT_SOURCE_E)(src + 2), HIGH_LEVEL_SENSITIVE);
	sysSetLocalInterrupt(ENABLE_IRQ);	

    return(0);    
}

void kpi_close(void)
{
	if(_opened != 2)
		return;
	_opened = 1;
	sysDisableInterrupt((INT_SOURCE_E)(_int + 2));  
	return;
}

#ifdef __GNUC__
__attribute__((optimize("O0")))
#endif
int kpi_read(unsigned char mode)
{
	// add this var in case key released right before return.
	int volatile k = 0;
	
	if(_opened != 2)
		return(-1);
	
	if(mode != KPI_NONBLOCK && mode != KPI_BLOCK) {
		//sysDisableInterrupt(_int + 2);  
		return(-1);
	}
	sysEnableInterrupt((INT_SOURCE_E)(_int + 2));
	
	if(_key == 0) {
		// not pressed, non blocking, return immediately
		if(mode == KPI_NONBLOCK) {
			sysDisableInterrupt((INT_SOURCE_E)(_int + 2));  
			return(0);
		}
		// not pressed, non blocking, wait for key pressed
#ifndef __GNUC__		
#pragma O0
#endif
// ARMCC is tooooo smart to compile this line correctly, so ether set O0 or use pulling....
		while((k = _key) == 0);
	} else {
		// read latest key(s) and return.
		sysDisableInterrupt((INT_SOURCE_E)(_int + 2));
		do {		
			k = readkey();		
		} while(k == 0 && mode != KPI_NONBLOCK);
	}
	return(k);
}


