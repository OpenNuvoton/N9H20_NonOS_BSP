/****************************************************************************
*                                                                           *
* Copyright (c) 2009 Nuvoton Tech. Corp. All rights reserved.               *
*                                                                           *
*****************************************************************************/

/****************************************************************************
* FILENAME
*   adc.c
*
* VERSION
*   1.0
*
* DESCRIPTION
*   ADC sample application using ADC library
*
* DATA STRUCTURES
*   None
*
* FUNCTIONS
*
* HISTORY
*
* REMARK
*   None
****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "N9H20.h"

extern INT16 g_pi16SampleBuf[];

extern void Smpl_TscPanel_WT_Powerdown_Wakeup(void);
extern void AudioRecord(UINT32 u32SampleRate);

/* Every 10ms call back */
UINT32 u32Flag_10ms = 0;
void Timer0_1_Callback(void) 
{/* LVD check */
	UINT16 u16Vol;
	u32Flag_10ms = u32Flag_10ms+1;
	if(u32Flag_10ms>=300)
	{		
		if(adc_normalread(2, &u16Vol)==Successful)
		{
			sysprintf("Battery voltage = %x\n", u16Vol);
		}	
		u32Flag_10ms = 0;
	}	
}

UINT32 u32Flag_10s = 0;
void Timer0_2_Callback(void)
{
	u32Flag_10s = 1;
}
int main(void)
{
	UINT32 u32ExtFreq, u32Item, u32PllFreq, u32SampleRate;
	INT32 i32TimerChanel1, i32TimerChanel2;
	UINT16 x, y;
	WB_UART_T uart;

	
	u32ExtFreq = sysGetExternalClock();
	
	uart.uiFreq = u32ExtFreq*1000;	//use APB clock
    uart.uiBaudrate = 115200;
	uart.uiDataBits = WB_DATA_BITS_8;
	uart.uiStopBits = WB_STOP_BITS_1;
	uart.uiParity = WB_PARITY_NONE;
	uart.uiRxTriggerLevel = LEVEL_1_BYTE;
	sysUartPort(1);
	sysInitializeUART(&uart);
	u32PllFreq = sysGetPLLOutputKhz(eSYS_UPLL, u32ExtFreq);
	
	sysprintf("External clock = %dKHz\n", u32ExtFreq);    	
	sysprintf("PLL clock = %d KHz\n", u32PllFreq);	
	
	fsInitFileSystem();
	
	sicIoctl(SIC_SET_CLOCK, 192000, 0, 0);      /* clock from PLL */
	sicIoctl(SIC_SET_CLOCK, u32PllFreq, 0, 0);	
	sicOpen();	
	if (sicSdOpen0()<=0)
	{
		sysprintf("Error in initializing SD card !! \n");						
		while(1);
	}			
	fsAssignDriveNumber('C', DISK_TYPE_SD_MMC, 0, 1);		
	
	/* For 30 sec: Low battery checking from channel 2*/
	sysSetTimerReferenceClock (TIMER0, u32ExtFreq*1000);
	sysStartTimer(TIMER0, 100, PERIODIC_MODE);	
	sysEnableCache(CACHE_WRITE_BACK);
	
	sysSetLocalInterrupt(ENABLE_IRQ);	
	do
	{
		sysprintf("==================================================================\n");
		sysprintf("[1] Power Down Wake Up by TSC  (Test 20 sec if nonblocking) \n");
		sysprintf("[2] Position and Low battery \n");	
		sysprintf("[3] Audio Recording to SD card \n");	
		sysprintf("==================================================================\n");

		u32Item = sysGetChar();
		
		switch(u32Item) 
		{
			case '1': 
				adc_init();
				adc_open(ADC_TS_4WIRE, 320, 240);		
				Smpl_TscPanel_WT_Powerdown_Wakeup();	break;
			case '2':
				adc_init();
				adc_open(ADC_TS_4WIRE, 320, 240);	
				i32TimerChanel1 = sysSetTimerEvent(TIMER0, 1, (PVOID)Timer0_1_Callback);	/* For  battery detection */
				if(i32TimerChanel1==0)
				{
					sysprintf("Out of timer channel\n");
					exit(-1);
				}
				
				u32Flag_10s = 0;		
				i32TimerChanel2 = sysSetTimerEvent(TIMER0, 1000, (PVOID)Timer0_2_Callback);	/* For  escape touch panel test 10 sec*/				
				if(i32TimerChanel2==0)
				{
					sysprintf("Out of timer channel\n");
					exit(-1);
				}											
				while(1) 
				{

				/* Touch pannel */		
	#ifdef _ADC_NONBLOCK_
					if(adc_read(ADC_NONBLOCK, &x, &y))
	#else
					if(adc_read(ADC_BLOCK, &x, &y))
	#endif
						sysprintf("x = %d, y = %d\n", x, y);
					else
						;//sysprintf("pen up");
					if(u32Flag_10s==1)
						break;					
				}
				sysClearTimerEvent(TIMER0, i32TimerChanel1); 
				sysClearTimerEvent(TIMER0, i32TimerChanel2); 	
			break;
			case '3':
				sysprintf("!!!!Remember that touch panel and voltage detection will be invalid if enable audio recording\n");
				sysprintf("[1] Sample Rate 16K \n");
				sysprintf("[2] Sample Rate 12K \n");	
				sysprintf("[3] Sample Rate 8K \n");	
				
				u32Item = sysGetChar();
				/* Use APLL for audio recording from ADC. The sample rate support 8K and 12K samples/s */
				if(u32Item=='1')
				{
					sysCheckPllConstraint(FALSE);						
					#if 0
					sysSetPllClock(eSYS_APLL, 	//Another PLL,	APLL meet the clock constraint. 
								184320);		//UINT32 u32PllKHz, 	
					#else
					outp32(REG_APLLCON, 0x113D);		//184300000Hz
					#endif 			
					sysCheckPllConstraint(TRUE);
					u32SampleRate = 16000;		
					AudioRecord(u32SampleRate);		
				}else if(u32Item=='2')
				{
					sysCheckPllConstraint(FALSE);	
					sysSetPllClock(eSYS_APLL, 	//Another PLL,	
								153600);		//UINT32 u32PllKHz, 										
					sysCheckPllConstraint(TRUE);				
					u32SampleRate = 12000;
					AudioRecord(u32SampleRate);	
				}else if(u32Item=='3')
				{
					sysCheckPllConstraint(FALSE);	
					sysSetPllClock(eSYS_APLL, 	//Another PLL,	
								153600);		//UINT32 u32PllKHz, 										
					sysCheckPllConstraint(TRUE);				
					u32SampleRate = 8000;
					AudioRecord(u32SampleRate);	
				}				
											
				break;
			case	'Q':
			case 'q': 
				u32Item = 'Q';
				sysprintf("quit adc test...\n");
				adc_close();
				break;	
			}
	}while(u32Item!='Q');		
}

