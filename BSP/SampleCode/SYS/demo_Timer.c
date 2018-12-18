/***************************************************************************
 *                                                                         									     *
 * Copyright (c) 2008 Nuvoton Technolog. All rights reserved.              					     *
 *                                                                         									     *
 ***************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include "N9H20.h"
#include "demo.h"


volatile unsigned int count=0, test1_flag=0, test3_flag=0;
volatile unsigned int IsWdtTimeOut=FALSE, IsClearWdt=FALSE, IsWdtResetFlagSet=FALSE;


void WatchDog_ISR()
{
    	sysClearWatchDogTimerInterruptStatus();
	
	IsWdtTimeOut = TRUE;

	if (IsClearWdt == TRUE)
	{
		sysClearWatchDogTimerCount();
		sysprintf("C\n");	
	}
	else
		sysprintf("NC\n");	
}

void Test()
{
	DBG_PRINTF("test -> %d\n", ++count);
}

void Test1()
{
	DBG_PRINTF("Hello World!\n");
}

void Test2()
{
	DBG_PRINTF("timer 1 test\n");
}


void Test3()
{
	DBG_PRINTF("Hello Timer1!\n");
	if (test3_flag == 0)
	{
		sysClearTimerEvent(TIMER0, 2);
		test3_flag = 1;
	}
}


void DemoAPI_Timer0(void)
{
	//	unsigned int volatile i;
	volatile unsigned int btime, etime, tmp, tsec;
	volatile UINT32 u32TimeOut = 0;
	UINT32 u32ExtFreq;
	
	u32ExtFreq = sysGetExternalClock()*1000;
	DBG_PRINTF("Timer 0 Test...\n");	
	sysSetTimerReferenceClock(TIMER0, u32ExtFreq); //External Crystal
	
	sysStartTimer(TIMER0, 100, PERIODIC_MODE);			/* 100 ticks/per sec ==> 1tick/10ms */
	
	tmp = sysSetTimerEvent(TIMER0, 100, (PVOID)Test);	/* 100 ticks = 1s call back */
	DBG_PRINTF("No. of Event [%d]\n", tmp);

	tmp = sysSetTimerEvent(TIMER0, 300, (PVOID)Test1);	/* 300 ticks/per sec */
	DBG_PRINTF("No. of Event [%d]\n", tmp);	
	sysSetLocalInterrupt(ENABLE_IRQ);

	btime = sysGetTicks(TIMER0);
	tsec = 0;
	tmp = btime;
	while (1)
	{			
		etime = sysGetTicks(TIMER0);
		if ((etime - btime) >= 100)
		{
			DBG_PRINTF("tick = %d\n", ++tsec);			
			btime = etime;
			u32TimeOut = u32TimeOut +1;
			if(u32TimeOut==10)			/* 10s Time out  */
				break;			
		}
	}
	DBG_PRINTF("Finish timer 0 testing...\n");
	sysStopTimer(TIMER0);
}

void DemoAPI_Timer1(void)
{
	volatile unsigned int btime, etime, tmp, tsec;
	volatile UINT32 u32TimeOut = 0;
	UINT32 u32ExtFreq;
	
	u32ExtFreq = sysGetExternalClock()*1000;
	DBG_PRINTF("Timer 1 Test...\n");
	
	sysSetTimerReferenceClock(TIMER1, u32ExtFreq); 		//External Crystal
	sysStartTimer(TIMER1, 100, PERIODIC_MODE);			/* 100 ticks/per sec ==> 1tick/10ms */
	tmp = sysSetTimerEvent(TIMER1, 100, (PVOID)Test);	/* 100 ticks = 1s call back */
	DBG_PRINTF("No. of Event [%d]\n", tmp);
	tmp = sysSetTimerEvent(TIMER1, 300, (PVOID)Test1);	/* 300 ticks/per sec */
	DBG_PRINTF("No. of Event [%d]\n", tmp);
	sysSetLocalInterrupt(ENABLE_IRQ);
	btime = sysGetTicks(TIMER1);
	tsec = 0;
	tmp = btime;
	while (1)
	{
		etime = sysGetTicks(TIMER1);	
		if ((etime - btime) >= 100)
		{
			DBG_PRINTF("tick = %d\n", ++tsec);
			btime = etime;
			u32TimeOut = u32TimeOut +1;
			if(u32TimeOut==10)			/* 10s Time out  */
				break;	
		}
		
	}
	DBG_PRINTF("Finish timer 1 testing...\n");

	//	for (i=0; i<0x5000; i++);

	sysStopTimer(TIMER1);
}

void DemoAPI_WDT(void)
{
	unsigned int btime, etime;
	UINT32 u32ExtFreq, tmp;
	
	u32ExtFreq = sysGetExternalClock()*1000;
	DBG_PRINTF("Watchdog Timer Test...\n");
	/* start timer 1 */
	tmp = sysSetTimerEvent(TIMER0, 1, (PVOID)Test);	/* 1 ticks = 10ms call back */
	sysSetTimerReferenceClock(TIMER0, u32ExtFreq);
	sysStartTimer(TIMER0, 100, PERIODIC_MODE); /* SW Modify sysStartTimer(TIMER0, 0, PERIODIC_MODE);*/

	/* set up watch dog timer */
	//sysSetWatchDogTimerInterval(HALF_MINUTES); 
	sysInstallWatchDogTimerISR(IRQ_LEVEL_1, (PVOID)WatchDog_ISR);
	sysEnableWatchDogTimer();
	sysEnableWatchDogTimerReset();

	IsClearWdt = TRUE;
	IsWdtTimeOut = FALSE;
	btime = sysGetTicks(TIMER0);

	while (IsWdtResetFlagSet == FALSE)
	{
		if (IsWdtTimeOut == TRUE)
		{
			IsWdtTimeOut = FALSE;
			etime =sysGetTicks(TIMER0);
			if ((etime - btime) > 3)
			 	IsClearWdt = FALSE;
		}
		if (inpw(REG_WTCR) & 0x04)
		  	IsWdtResetFlagSet = TRUE; /* check WDT reset flag bit */
	}
	sysClearTimerEvent(TIMER0, tmp);
	DBG_PRINTF("Finish watch dog timer testing...\n");
}
