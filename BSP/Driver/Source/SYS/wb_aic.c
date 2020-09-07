/**************************************************************************//**
 * @file     wb_aic.c
 * @version  V3.00
 * @brief    N9H20 series SYS driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
 
/****************************************************************************
* FILENAME
*   wb_aic.c
*
* VERSION
*   1.0
*
* DESCRIPTION
*   The AIC related functions of Nuvoton ARM9 MCU
*
* HISTORY
*   2008-06-25  Ver 1.0 draft by Min-Nan Cheng
*
* REMARK
*   None
 **************************************************************************/
#include <stdio.h>
#include "wblib.h"
#define WB_MIN_INT_SOURCE  1
#define WB_MAX_INT_SOURCE  31
#define WB_NUM_OF_AICREG   32

/* Global variables */
BOOL volatile _sys_bIsAICInitial = FALSE;
BOOL volatile _sys_bIsHWMode = TRUE;

/* declaration the function prototype */
extern VOID WB_Interrupt_Shell(VOID);

/* SCR and SVR register access function */
#define AIC_REG_OFFSET    						0x4
#define AICRegRead(RegBase, RegNum)				inpw(RegBase+RegNum*AIC_REG_OFFSET)
#define AICRegWrite(RegBase, RegNum, Value)			outpw(RegBase+RegNum*AIC_REG_OFFSET, Value)

/* Interrupt Handler Table */
typedef void (*sys_pvFunPtr)();   /* function pointer */
sys_pvFunPtr sysIrqHandlerTable[] = { 0,                /* 0 */
                                  WB_Interrupt_Shell,	/* 1 */
                                  WB_Interrupt_Shell,	/* 2 */
                                  WB_Interrupt_Shell,	/* 3 */
                                  WB_Interrupt_Shell,	/* 4 */
                                  WB_Interrupt_Shell,	/* 5 */
                                  WB_Interrupt_Shell,	/* 6 */
                                  WB_Interrupt_Shell,	/* 7 */
                                  WB_Interrupt_Shell,	/* 8 */
                                  WB_Interrupt_Shell,	/* 9 */
                                  WB_Interrupt_Shell,	/* 10 */
                                  WB_Interrupt_Shell,	/* 11 */
                                  WB_Interrupt_Shell,	/* 12 */
                                  WB_Interrupt_Shell,	/* 13 */
                                  WB_Interrupt_Shell,	/* 14 */
                                  WB_Interrupt_Shell,	/* 15 */
                                  WB_Interrupt_Shell,	/* 16 */
                                  WB_Interrupt_Shell,	/* 17 */
                                  WB_Interrupt_Shell,	/* 18 */
                                  WB_Interrupt_Shell,	/* 19 */
                                  WB_Interrupt_Shell,	/* 20 */
                                  WB_Interrupt_Shell,	/* 21 */
                                  WB_Interrupt_Shell,	/* 22 */
                                  WB_Interrupt_Shell,	/* 23 */
                                  WB_Interrupt_Shell,	/* 24 */
                                  WB_Interrupt_Shell,	/* 25 */
                                  WB_Interrupt_Shell,	/* 26 */
                                  WB_Interrupt_Shell,	/* 27 */
                                  WB_Interrupt_Shell,	/* 28 */
                                  WB_Interrupt_Shell,	/* 29 */
                                  WB_Interrupt_Shell,	/* 30 */
                                  WB_Interrupt_Shell	/* 31 */
                                };

sys_pvFunPtr sysFiqHandlerTable[] = { 0,
                                  WB_Interrupt_Shell,	/* 1 */
                                  WB_Interrupt_Shell,	/* 2 */
                                  WB_Interrupt_Shell,	/* 3 */
                                  WB_Interrupt_Shell,	/* 4 */
                                  WB_Interrupt_Shell,	/* 5 */
                                  WB_Interrupt_Shell,	/* 6 */
                                  WB_Interrupt_Shell,	/* 7 */
                                  WB_Interrupt_Shell,	/* 8 */
                                  WB_Interrupt_Shell,	/* 9 */
                                  WB_Interrupt_Shell,	/* 10 */
                                  WB_Interrupt_Shell,	/* 11 */
                                  WB_Interrupt_Shell,	/* 12 */
                                  WB_Interrupt_Shell,	/* 13 */
                                  WB_Interrupt_Shell,	/* 14 */
                                  WB_Interrupt_Shell,	/* 15 */
                                  WB_Interrupt_Shell,	/* 16 */
                                  WB_Interrupt_Shell,	/* 17 */
                                  WB_Interrupt_Shell,	/* 18 */
                                  WB_Interrupt_Shell,	/* 19 */
                                  WB_Interrupt_Shell,	/* 20 */
                                  WB_Interrupt_Shell,	/* 21 */
                                  WB_Interrupt_Shell,	/* 22 */
                                  WB_Interrupt_Shell,	/* 23 */
                                  WB_Interrupt_Shell,	/* 24 */
                                  WB_Interrupt_Shell,	/* 25 */
                                  WB_Interrupt_Shell,	/* 26 */
                                  WB_Interrupt_Shell,	/* 27 */
                                  WB_Interrupt_Shell,	/* 28 */
                                  WB_Interrupt_Shell,	/* 29 */
                                  WB_Interrupt_Shell,	/* 30 */
                                  WB_Interrupt_Shell	/* 31 */
                                };

/* Interrupt Handler */
#ifdef __FreeRTOS__
VOID sysIrqHandler(UINT32 _mIPER, UINT32 _mISNR)
{
	_mIPER = _mIPER >> 2;
	if (_mISNR != 0)
		if (_mIPER == _mISNR)
			(*sysIrqHandlerTable[_mIPER])();
	outpw(REG_AIC_EOSCR, 1);
}
#else
#if defined (__GNUC__) && !(__CC_ARM)
static void __attribute__ ((interrupt("IRQ"))) sysIrqHandler(void)
#else
__irq VOID sysIrqHandler()
#endif
{
	if (_sys_bIsHWMode)
	{
		UINT32 volatile _mIPER, _mISNR;

		_mIPER = inpw(REG_AIC_IPER) >> 2;
		//outpw(REG_AIC_IPER,0); //For test mode
		
		_mISNR = inpw(REG_AIC_ISNR);
		if (_mISNR != 0)
			if (_mIPER == _mISNR)
				(*sysIrqHandlerTable[_mIPER])();
		outpw(REG_AIC_EOSCR, 1);					
	}
	else
	{
		UINT32 volatile _mISR, i;

		_mISR = inpw(REG_AIC_ISR);
		for (i=1; i<WB_NUM_OF_AICREG; i++)
			if (_mISR & (1 << i))
				(*sysIrqHandlerTable[i])();
	}
}
#endif
#if defined (__GNUC__) && !(__CC_ARM)
static void __attribute__ ((interrupt("FIQ"))) sysFiqHandler(void)
#else
__irq VOID sysFiqHandler()
#endif
{
	if (_sys_bIsHWMode)
	{
		UINT32 volatile _mIPER, _mISNR;

		_mIPER = inpw(REG_AIC_IPER) >> 2;
		//outpw(REG_AIC_IPER,0); //For test mode
		_mISNR = inpw(REG_AIC_ISNR);
		if (_mISNR != 0)
			if (_mIPER == _mISNR)
				(*sysFiqHandlerTable[_mIPER])();
		outpw(REG_AIC_EOSCR, 1);	

	}
	else
	{
		UINT32 volatile _mISR, i;

		_mISR = inpw(REG_AIC_ISR);
		for (i=1; i<WB_NUM_OF_AICREG; i++)
			if (_mISR & (1 << i))
				(*sysFiqHandlerTable[i])();
	}
}

VOID WB_Interrupt_Shell()
{

}
#pragma diag_suppress 191
VOID sysInitializeAIC()
{
	//PVOID _mOldIrqVect, _mOldFiqVect;

	*((unsigned volatile *)0x18) = 0xe59ff018;
	*((unsigned volatile *)0x1C) = 0xe59ff018;   

#ifndef __FreeRTOS__
	//_mOldIrqVect = *(PVOID volatile *)0x38;
	*(PVOID volatile *)0x38 = (PVOID volatile)sysIrqHandler;
#endif

	//_mOldFiqVect = *(PVOID volatile *)0x3C;
	*(PVOID volatile *)0x3C = (PVOID volatile)sysFiqHandler;

	if (sysGetCacheState() == TRUE)
		sysFlushCache(I_CACHE);
}


/* Interrupt library functions */
INT32 sysDisableInterrupt(INT_SOURCE_E eIntNo)
{
	if ((eIntNo > WB_MAX_INT_SOURCE) || (eIntNo < WB_MIN_INT_SOURCE))
	  	return Fail;

	outpw(REG_AIC_MDCR, (1 << eIntNo));
	return Successful;
}

#if 0
INT32 sysDisableGroupInterrupt(INT_SOURCE_GROUP_E eIntNo)
{
	outpw(REG_AIC_GEN, inpw(REG_AIC_GEN) & ~eIntNo);
	return Successful;
}
#endif

INT32 sysEnableInterrupt(INT_SOURCE_E eIntNo)
{
	if ((eIntNo > WB_MAX_INT_SOURCE) || (eIntNo < WB_MIN_INT_SOURCE))
	  	return Fail;

	outpw(REG_AIC_MECR, (1 << eIntNo));
	return Successful;
}

#if 0
INT32 sysEnableGroupInterrupt(INT_SOURCE_GROUP_E eIntNo)
{
	outpw(REG_AIC_GEN, inpw(REG_AIC_GEN) | eIntNo);
	return Successful;
}

UINT32 sysGetGroupInterruptStatus()
{
	UINT32 status;

	status = inpw(REG_AIC_GASR);
	return status;
}
#endif

PVOID sysInstallExceptionHandler(INT32 nExceptType, PVOID pvNewHandler)
{
	PVOID _mOldVect;

	switch (nExceptType)
	{
		case WB_SWI:
			_mOldVect = *(PVOID volatile *)0x28;
			*(PVOID volatile *)0x28 = pvNewHandler;
		break;

		case WB_D_ABORT:
			_mOldVect = *(PVOID volatile *)0x30;
			*(PVOID volatile *)0x30 = pvNewHandler;
		break;

		case WB_I_ABORT:
			_mOldVect = *(PVOID volatile *)0x2C;
			*(PVOID volatile *)0x2C = pvNewHandler;
		break;

		case WB_UNDEFINE:
			_mOldVect = *(PVOID volatile *)0x24;
			*(PVOID volatile *)0x24 = pvNewHandler;
		break;

		default:
		   ;
	}
	return _mOldVect;
}

PVOID sysInstallFiqHandler(PVOID pvNewISR)
{
	PVOID _mOldVect;

	_mOldVect = *(PVOID volatile *)0x3C;
	*(PVOID volatile *)0x3C = pvNewISR;
	return _mOldVect;
}

PVOID sysInstallIrqHandler(PVOID pvNewISR)
{
	PVOID _mOldVect;

	_mOldVect = *(PVOID volatile *)0x38;
	*(PVOID volatile *)0x38 = pvNewISR;
	return _mOldVect;
}

//OK
PVOID sysInstallISR(INT32 nIntTypeLevel, INT_SOURCE_E eIntNo, PVOID pvNewISR)
{
	PVOID  _mOldVect;
	UINT32 _mRegValue;

	if (!_sys_bIsAICInitial)
	{
		sysInitializeAIC();
		_sys_bIsAICInitial = TRUE;
	}

	_mRegValue = AICRegRead(REG_AIC_SCR1, (eIntNo/4));
	_mRegValue = _mRegValue & ( 0xFFFFFFF8<<((eIntNo%4)*8) | ( (1<<((eIntNo%4)*8))-1 ) );
	_mRegValue = _mRegValue | nIntTypeLevel<<((eIntNo%4)*8);
	AICRegWrite(REG_AIC_SCR1, (eIntNo/4), _mRegValue );

	switch (nIntTypeLevel)
	{
		case FIQ_LEVEL_0:
			_mOldVect = (PVOID) sysFiqHandlerTable[eIntNo];
			sysFiqHandlerTable[eIntNo] = (sys_pvFunPtr)pvNewISR;
		break;

		case IRQ_LEVEL_1:
		case IRQ_LEVEL_2:
		case IRQ_LEVEL_3:
		case IRQ_LEVEL_4:
		case IRQ_LEVEL_5:
		case IRQ_LEVEL_6:
		case IRQ_LEVEL_7:
		  	 _mOldVect = (PVOID) sysIrqHandlerTable[eIntNo];
		   	sysIrqHandlerTable[eIntNo] = (sys_pvFunPtr)pvNewISR;
		break;

		default:
		   ;
	}
	return _mOldVect;	
}
//OK
INT32 sysSetGlobalInterrupt(INT32 nIntState)
{
	switch (nIntState)
	{
		case ENABLE_ALL_INTERRUPTS:
		   	outpw(REG_AIC_MECR, 0xFFFFFFFF);
		break;

		case DISABLE_ALL_INTERRUPTS:
		   	outpw(REG_AIC_MDCR, 0xFFFFFFFF);
		break;

		default:
		   	;
   	}
  	return Successful;
}
//OK
INT32 sysSetInterruptPriorityLevel(INT_SOURCE_E eIntNo, UINT32 uIntLevel)
{
	UINT32 _mRegValue;

	if ((eIntNo > WB_MAX_INT_SOURCE) || (eIntNo < WB_MIN_INT_SOURCE))
	  	return Fail;
	
	_mRegValue = AICRegRead(REG_AIC_SCR1, (eIntNo/4));
	_mRegValue = _mRegValue & ( 0xFFFFFFF8<<((eIntNo%4)*8) | ( (1<<((eIntNo%4)*8))-1 ) );
	_mRegValue = _mRegValue | uIntLevel<<((eIntNo%4)*8);
	AICRegWrite(REG_AIC_SCR1, (eIntNo/4), _mRegValue);

	return Successful;		
}
//OK
/*
typedef enum 
{
	eDRVAIC_LOW_LEVEL, 
	eDRVAIC_HIGH_LEVEL, 
	eDRVAIC_FALLING, 
	eDRVAIC_RISING
}E_DRVAIC_INT_TYPE;
*/
INT32 sysSetInterruptType(INT_SOURCE_E eIntNo, UINT32 uIntSourceType)
{
	UINT32 _mRegValue;

	if ((eIntNo > WB_MAX_INT_SOURCE) || (eIntNo < WB_MIN_INT_SOURCE))
	  	return Fail;

	_mRegValue = AICRegRead(REG_AIC_SCR1, (eIntNo/4));
	_mRegValue = _mRegValue & ( 0xFFFFFF3F<<((eIntNo & 0x3)*8) | ( (1<<((eIntNo%4)*8))-1 ) );
	_mRegValue = _mRegValue | ( uIntSourceType<<((eIntNo & 0x3)*8 + 6) );
	AICRegWrite(REG_AIC_SCR1, (eIntNo >> 2), _mRegValue );
   
	return Successful;
}
//OK
INT32 sysSetLocalInterrupt(INT32 nIntState)
{
#if defined (__GNUC__) && !(__CC_ARM)

# else
    INT32 temp;
#endif

	switch (nIntState)
	{
		case ENABLE_IRQ:
		case ENABLE_FIQ:
		case ENABLE_FIQ_IRQ:
#if defined (__GNUC__) && !(__CC_ARM)
            asm
            (
                "mrs    r0, CPSR  \n"
                "bic    r0, r0, #0x80  \n"
                "msr    CPSR_c, r0  \n"
            );
#else
			__asm
			{
			   MRS    temp, CPSR
			   AND    temp, temp, nIntState
			   MSR    CPSR_c, temp
			}
#endif
		break;

		case DISABLE_IRQ:
		case DISABLE_FIQ:
		case DISABLE_FIQ_IRQ:
#if defined ( __GNUC__ ) && !(__CC_ARM)
            asm
            (
                "MRS    r0, CPSR  \n"
                "ORR    r0, r0, #0x80  \n"
                "MSR    CPSR_c, r0  \n"
            );
#else
			__asm
			{
			   MRS    temp, CPSR
			   ORR    temp, temp, nIntState
			   MSR    CPSR_c, temp
			}
#endif
		   break;

		default:
		   	;
	}
	return Successful;
}
//OK
INT32 sysSetAIC2SWMode()
{
	_sys_bIsHWMode = FALSE;
	return Successful;
}

//OK
UINT32	sysGetInterruptEnableStatus(VOID)
{
    	return (inpw(REG_AIC_IMR));
}

//OK
BOOL sysGetIBitState()
{
	INT32 temp;
#if defined (__GNUC__) && !(__CC_ARM)
    asm
    (
        "MRS %0, CPSR   \n"
        :"=r" (temp) : :
    );
#else
	__asm
	{
		MRS	temp, CPSR
	}
#endif
	if (temp & 0x80)
		return FALSE;
	else
		return TRUE;
}


