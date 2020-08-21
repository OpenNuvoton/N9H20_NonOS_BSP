/**************************************************************************//**
 * @file     PWM.c
 * @version  V3.00
 * @brief    N9H20 series PWM driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/


/*---------------------------------------------------------------------------------------------------------*/
/* Includes of system headers                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wbio.h"
#include "wblib.h"
#include "wbtypes.h"
#include "N9H20_PWM.h"


/*---------------------------------------------------------------------------------------------------------*/
/* Macro, type and constant definitions                                                                    */
/*---------------------------------------------------------------------------------------------------------*/
#define PWM_GLOBALS


/*---------------------------------------------------------------------------------------------------------*/
/* Global file scope (static) variables                                                                    */
/*---------------------------------------------------------------------------------------------------------*/
static PWM_CALLBACK_T pfnPWMHandler = {FALSE};

/*---------------------------------------------------------------------------------------------------------*/
/* Function:     <PWM0_ISR>                                                                                */
/*                                                                                                         */
/* Parameter:                                                                                              */
/*               VOID                                                                                      */
/* Returns:                                                                                                */
/*               None                                                                                      */
/* Side effects:                                                                                           */
/*                                                                                                         */
/* Description:                                                                                            */
/*               ISR to handle PWM0 interrupt event           		                                       */
/*---------------------------------------------------------------------------------------------------------*/
static VOID PWM_ISR (VOID)
{ 
    UINT32 volatile u32RegPIIR,u32RegCCR0,u32RegCCR1;

    u32RegPIIR = inp32(PIIR);
    u32RegCCR0 = inp32(CCR0);
    u32RegCCR1 = inp32(CCR1);    
    
    if (u32RegPIIR & 0xF )    
    {
		if(u32RegPIIR & PIIR0)
    	{
    		outp32(PIIR, PIIR0);    
			if (pfnPWMHandler.pfnPWM0CallBack != NULL)                           
            	pfnPWMHandler.pfnPWM0CallBack();	
    	} 	    	    	
		if(u32RegPIIR & PIIR1)
    	{
    		outp32(PIIR, PIIR1);    
			if (pfnPWMHandler.pfnPWM1CallBack != NULL)                           
            	pfnPWMHandler.pfnPWM1CallBack();	
    	}   
		if(u32RegPIIR & PIIR2)
    	{
    		outp32(PIIR, PIIR2);    
			if (pfnPWMHandler.pfnPWM2CallBack != NULL)                           
            	pfnPWMHandler.pfnPWM2CallBack();	
    	} 
		if(u32RegPIIR & PIIR3)
    	{
    		outp32(PIIR, PIIR3);    
			if (pfnPWMHandler.pfnPWM3CallBack != NULL)                           
            	pfnPWMHandler.pfnPWM3CallBack();	
    	}    
    }

    if (u32RegCCR0 & CIIR0) 
    {
    	outp32(CCR0, inp32(CCR0) & ~CIIR0);
        if (pfnPWMHandler.pfnCAP0CallBack != NULL)
        {
        	pfnPWMHandler.pfnCAP0CallBack();
        }
	}

    if (u32RegCCR0 & CIIR1) 
    {
    	outp32(CCR0, inp32(CCR0) & ~CIIR1);
        if (pfnPWMHandler.pfnCAP1CallBack != NULL)
        {
        	pfnPWMHandler.pfnCAP1CallBack();
        }
	}		

    if (u32RegCCR1 & CIIR2) 
    {
    	outp32(CCR1, inp32(CCR1) & ~CIIR2);
        if (pfnPWMHandler.pfnCAP2CallBack != NULL)
        {
        	pfnPWMHandler.pfnCAP2CallBack();
        }
	}		 
    if (u32RegCCR1 & CIIR3) 
    {
    	outp32(CCR1, inp32(CCR1) & ~CIIR3);
        if (pfnPWMHandler.pfnCAP3CallBack != NULL)
        {
        	pfnPWMHandler.pfnCAP3CallBack();
        }
	}			
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function: PWM_IsTimerEnabled                                                                            */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*               u8Timer - [in]		PWM_TIMER0/PWM_TIMER1/PWM_TIMER2/PWM_TIMER3         			       */
/*                                                                                                         */
/* Returns:                                                                                                */
/*               0		disable                                                                            */
/*               1		enable	                                                                           */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*               This function is used to get PWM specified timer enable/disable state  	               */
/*---------------------------------------------------------------------------------------------------------*/
BOOL PWM_IsTimerEnabled(UINT8 u8Timer)
{
	return ((inp32(PCR) & (1 << ((u8Timer & 0x7) << 3))) ? TRUE : FALSE);
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function: PWM_SetTimerCounter                                                                           */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*               u8Timer 		- [in]		PWM_TIMER0/PWM_TIMER1/PWM_TIMER2/PWM_TIMER3   				   */
/*               u16Counter     - [in]      Timer counter : 0~65535                                        */
/* Returns:                                                                                                */
/*               0		disable                                                                            */
/*               1		enable	                                                                           */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*               This function is used to set the PWM specified timer counter 			           		   */
/*---------------------------------------------------------------------------------------------------------*/
VOID PWM_SetTimerCounter(UINT8 u8Timer, UINT16 u16Counter)
{
	outp32(CNR0 + (u8Timer & 0x07) * 12, u16Counter);		
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function: PWM_GetTimerCounter                                                                           */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*               u8Timer 		- [in]		PWM_TIMER0/PWM_TIMER1/PWM_TIMER2/PWM_TIMER3  			       */
/*              											                                        	   */
/* Returns:                                                                                                */
/*               The specified timer counter value                                                         */
/*              											                                        	   */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*               This function is used to get the PWM specified timer counter value 		               */
/*---------------------------------------------------------------------------------------------------------*/
UINT32 PWM_GetTimerCounter(UINT8 u8Timer)
{
	return (inp32(PDR0 + 0x0C * (u8Timer & 0x07)) & 0xFFFF);
}



/*---------------------------------------------------------------------------------------------------------*/
/* Function: DrvPWM_InstallCallBack		                                                                   */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*               u8Timer 		- [in]		DRVPWM_TIMER0/DRVPWM_TIMER1/DRVPWM_TIMER2/DRVPWM_TIMER3        */
/*         									DRVPWM_CAP0/DRVPWM_CAP1/DRVPWM_CAP2/DRVPWM_CAP3        		   */
/*               pfncallback	- [in]		The call back function for specified timer / capture		   */
/*              											                                        	   */
/* Returns:                                                                                                */
/*               None                                                                            		   */
/*               			                                                                          	   */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*               This function is used to iNnstall the PWM timer/capture interrupt call back function      */
/*---------------------------------------------------------------------------------------------------------*/
VOID PWM_InstallCallBack(
	UINT8 u8Timer,
	PFN_PWM_CALLBACK pfncallback,
	PFN_PWM_CALLBACK *pfnOldcallback
)
{
	switch(u8Timer)
	{	
		case PWM_TIMER0:
		case PWM_CAP0:			
		    if(pfncallback != NULL)
		    {
		    	if(u8Timer & 0x10)
		    	{
		    		*pfnOldcallback = pfnPWMHandler.pfnCAP0CallBack;
		       	    pfnPWMHandler.pfnCAP0CallBack = pfncallback;
		       	}
		       	else
		       	{
		       		*pfnOldcallback = pfnPWMHandler.pfnPWM0CallBack;
		       		pfnPWMHandler.pfnPWM0CallBack = pfncallback;
		       	}
	    	}
			break;
		case PWM_TIMER1:
		case PWM_CAP1:		
		    if(pfncallback != NULL)
		    {
		    	if(u8Timer & 0x10)
		    	{
		    		*pfnOldcallback = pfnPWMHandler.pfnCAP1CallBack; 
		       	    pfnPWMHandler.pfnCAP1CallBack = pfncallback;
		       	}
		       	else
		       	{
		       		*pfnOldcallback = pfnPWMHandler.pfnPWM1CallBack;
		       		pfnPWMHandler.pfnPWM1CallBack = pfncallback;
		       	}
	    	}		
			break;
		case PWM_TIMER2:
		case PWM_CAP2:
		    if(pfncallback != NULL)
		    {
		    	if(u8Timer & 0x10)
		    	{
		    		*pfnOldcallback = pfnPWMHandler.pfnCAP2CallBack;
		       	    pfnPWMHandler.pfnCAP2CallBack = pfncallback;
		       	}
		       	else
		       	{
		       		*pfnOldcallback = pfnPWMHandler.pfnPWM2CallBack;
		       		pfnPWMHandler.pfnPWM2CallBack = pfncallback;
		       	}
	    	}				
			break;
		case PWM_TIMER3:
		case PWM_CAP3:
		    if(pfncallback != NULL)
		    {
		    	if(u8Timer & 0x10)
		    	{
		    		*pfnOldcallback = pfnPWMHandler.pfnCAP3CallBack;
		       	    pfnPWMHandler.pfnCAP3CallBack = pfncallback;
		       	}
		       	else
		       	{
		       		*pfnOldcallback = pfnPWMHandler.pfnPWM3CallBack; 
		       		pfnPWMHandler.pfnPWM3CallBack = pfncallback;
		       	}
	    	}				

			
			break;									
	}
	
	sysInstallISR(IRQ_LEVEL_1, IRQ_PWM, (PVOID)PWM_ISR);	
	sysSetLocalInterrupt(ENABLE_IRQ);
	sysEnableInterrupt(IRQ_PWM);   	
}
/*---------------------------------------------------------------------------------------------------------*/
/* Function: PWM_EnableInt		                                                                     	   */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*               u8Timer 		- [in]		PWM_TIMER0/PWM_TIMER1/PWM_TIMER2/PWM_TIMER3    				   */
/*         									PWM_CAP0/PWM_CAP1/PWM_CAP2/PWM_CAP3       		 			   */
/*               u8Int	 		- [in]		PWM_CAP_RISING_INT/PWM_CAP_FALLING_INT/PWM_CAP_ALL_INT		   */
/*              					        (The parameter is valid only when capture function)	       	   */
/*               pfncallback	- [in]		The call back function for specified timer / capture		   */
/*              											                                        	   */
/* Returns:                                                                                                */
/*               None                                                                            		   */
/*               			                                                                          	   */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*               This function is used to Enable the PWM timer/capture interrupt 		                   */
/*---------------------------------------------------------------------------------------------------------*/
VOID PWM_EnableInt(UINT8 u8Timer, UINT8 u8Int)
{
	if(u8Timer & 0x10)
	{		
		if(u8Timer & 0x02)		
			outp32(CCR1, (inp32(CCR1) & ~((0x06 << ((u8Timer & 0x01) << 4 )) | (CFLRD3 | CRLRD3 | CIIR3 | CFLRD2 | CRLRD2 |CIIR2))) |	((u8Int & 0x03) << (((u8Timer & 0x01) <<4) + 1)));					
		else
			outp32(CCR0, (inp32(CCR0) & ~((0x06 << ((u8Timer & 0x01) << 4 )) | (CFLRD1 | CRLRD1 | CIIR1 | CFLRD0 | CRLRD0 |CIIR0))) |	((u8Int & 0x03) << (((u8Timer & 0x01) <<4) + 1)));					
	}
	else
		outp32(PIER,  inp32(PIER) | (1<<u8Timer));
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function: PWM_DisableInt		  		                                                                   */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*               u8Timer 		- [in]		PWM_TIMER0/PWM_TIMER1/PWM_TIMER2/PWM_TIMER3      			   */
/*         									PWM_CAP0/PWM_CAP1/PWM_CAP2/PWM_CAP3        					   */
/*               u8Int	 		- [in]		PWM_CAP_RISING_INT/PWM_CAP_FALLING_INT/PWM_CAP_ALL_INT	 	   */
/*              											                                        	   */
/* Returns:                                                                                                */
/*               None                                                                            		   */
/*               			                                                                          	   */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*               This function is used to clear the PWM timer/capture interrupt 		                   */
/*---------------------------------------------------------------------------------------------------------*/
VOID PWM_DisableInt(UINT8 u8Timer, UINT8 u8Int)
{   
	if(u8Timer & 0x10)
	{
		if(u8Timer & 0x02)		
			outp32(CCR1, inp32(CCR1) & ~(((u8Int & 0x03) << (((u8Timer & 0x01) <<4) + 1)) | (CFLRD3 | CRLRD3 | CIIR3 | CFLRD2 | CRLRD2 |CIIR2)));					
		else
			outp32(CCR0, inp32(CCR0) & ~(((u8Int & 0x03) << (((u8Timer & 0x01) <<4) + 1)) | (CFLRD1 | CRLRD1 | CIIR1 | CFLRD0 | CRLRD0 |CIIR0)));					
		
	}
	else
	{		    
  		outp32(PIER,  inp32(PIER) & ~(1 << (u8Timer & 0x03)));
   		outp32(PIIR,  inp32(PIIR) & ~(1 << (u8Timer & 0x03)));
   	} 
	switch(u8Timer)
	{	
		case PWM_TIMER0:
		case PWM_CAP0:			
       	    pfnPWMHandler.pfnCAP0CallBack = NULL;
       		pfnPWMHandler.pfnPWM0CallBack = NULL;	
			break;
		case PWM_TIMER1:
		case PWM_CAP1:		
       	    pfnPWMHandler.pfnCAP1CallBack = NULL;
       		pfnPWMHandler.pfnPWM1CallBack = NULL;	
			break;
		case PWM_TIMER2:
		case PWM_CAP2:
       	    pfnPWMHandler.pfnCAP2CallBack = NULL;
       		pfnPWMHandler.pfnPWM2CallBack = NULL;	
			break;
		case PWM_TIMER3:
		case PWM_CAP3:
       	    pfnPWMHandler.pfnCAP3CallBack = NULL;
       		pfnPWMHandler.pfnPWM3CallBack = NULL;	
			break;									
	}
	
	
	if(((inp32(PIER) & 0xF) == 0) && ((inp32(CCR0) & 0x60006) == 0) && ((inp32(CCR1) & 0x60006) == 0))
		sysDisableInterrupt(IRQ_PWM);   		 
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function: PWM_ClearInt		                                                                      	   */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*               u8Timer 		- [in]		PWM_TIMER0/PWM_TIMER1/PWM_TIMER2/PWM_TIMER3       			   */
/*         									PWM_CAP0/PWM_CAP1/PWM_CAP2/PWM_CAP3        		 			   */
/*              											                                        	   */
/* Returns:                                                                                                */
/*               None                                                                            		   */
/*               			                                                                          	   */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*               This function is used to clear the PWM timer/capture interrupt 		                   */
/*---------------------------------------------------------------------------------------------------------*/
VOID PWM_ClearInt(UINT8 u8Timer)
{
	if(u8Timer & 0x10)
	{
		if(u8Timer & 0x02)
			outp32(CCR1, inp32(CCR1) & ~((1 << (((u8Timer & 0x01) << 4) + 4)) | (CFLRD3 | CRLRD3 | CIIR3 | CFLRD2 | CRLRD2 |CIIR2)));
		else
			outp32(CCR0, inp32(CCR0) & ~((1 << (((u8Timer & 0x01) << 4) + 4)) | (CFLRD3 | CRLRD3 | CIIR3 | CFLRD2 | CRLRD2 |CIIR2)));			
	}
	else
		outp32(PIIR,  inp32(PIIR) & ~(1<<(u8Timer & 0x03)));
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function: PWM_GetIntFlag				                                                         	       */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*               u8Timer 		- [in]		PWM_TIMER0/PWM_TIMER1/PWM_TIMER2/PWM_TIMER3  			       */
/*         									PWM_CAP0/PWM_CAP1/PWM_CAP2/PWM_CAP3        					   */
/*              											                                        	   */
/* Returns:                                                                                                */
/*               0		FALSE                                                                              */
/*               1		TRUE		                                                                       */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*               This function is used to get the PWM timer/capture interrupt flag 		                   */
/*---------------------------------------------------------------------------------------------------------*/
BOOL PWM_GetIntFlag(UINT8 u8Timer)
{
	if(u8Timer & 0x10)
	{
		if(u8Timer & 0x02)
		{	
			if(inp32(CCR1) & (1 << (((u8Timer & 0x01) << 4) + 4)))
				return TRUE;
			else
				return FALSE;
		}
		else
		{  	
	 		if(inp32(CCR0) & (1 << (((u8Timer & 0x01) << 4) + 4)))
				return TRUE;
			else
				return FALSE;
		}		
	}
	else
	{
		if(inp32(PIIR) & (1<<(u8Timer & 0x03)))
			return TRUE;
		else
			return FALSE;
	}		
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function: PWM_GetRisingCounter				    	                                                   */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*               u8Capture 		- [in]		PWM_CAP0/PWM_CAP1/PWM_CAP2/PWM_CAP3	      			 		   */
/*              											                                        	   */
/* Returns:                                                                                                */
/*               The value which latches the counter when there・s a rising transition                      */
/*               							                                                               */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*               This function is used to get value which latches the counter when there・s a rising 	   */		
/*				 transition		                   														   */
/*---------------------------------------------------------------------------------------------------------*/
UINT16 PWM_GetRisingCounter(UINT8 u8Capture)
{
	return inp32(CRLR0 + ((u8Capture & 0x7) << 3));
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function: PWM_GetFallingCounter				                     	                                   */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*               u8Capture 		- [in]		PWM_CAP0/PWM_CAP1/PWM_CAP2/PWM_CAP3	    			   		   */
/*              											                                        	   */
/* Returns:                                                                                                */
/*               The value which latches the counter when there・s a falling transition                     */
/*               							                                                               */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*               This function is used to get value which latches the counter when there・s a falling 	   */		
/*				 transition		                   														   */
/*---------------------------------------------------------------------------------------------------------*/
UINT16 PWM_GetFallingCounter(UINT8 u8Capture)
{
	return inp32(CFLR0 + ((u8Capture & 0x7)<< 3));
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function: PWM_GetCaptureIntStatus				                                                       */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*               u8Capture 		- [in]		PWM_CAP0/PWM_CAP1/PWM_CAP2/PWM_CAP3	       					   */
/*               u8IntType 		- [in]		PWM_CAP_RISING_FLAG/PWM_CAP_FALLING_FLAG	       			   */
/*              											                                        	   */
/* Returns:                                                                                                */
/*               Check if there・s a rising / falling transition                    				     	   */
/*               							                                                               */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*               This function is used to get the rising / falling transition interrupt flag			   */		
/*---------------------------------------------------------------------------------------------------------*/
BOOL PWM_GetCaptureIntStatus(UINT8 u8Capture, UINT8 u8IntType)
{	
	if(u8Capture & 0x02)
	{
		if(inp32(CCR1) &  (1 << (((u8Capture & 0x01) << 4) + u8IntType)))
			return TRUE;
		else 
			return FALSE;
    }
    else
    {
		if(inp32(CCR0) &  (1 << (((u8Capture & 0x01) << 4) + u8IntType)))
			return TRUE;
		else 
			return FALSE;
    }
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function: PWM_ClearCaptureIntStatus				                                                 	   */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*               u8Capture 		- [in]		PWM_CAP0/PWM_CAP1/PWM_CAP2/PWM_CAP3	       		 			   */
/*               u8IntType 		- [in]		PWM_CAP_RISING_FLAG/PWM_CAP_FALLING_FLAG	       			   */
/*              											                                        	   */
/* Returns:                                                                                                */
/*               Clear the rising / falling transition interrupt flag	                    		       */
/*               							                                                               */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*               This function is used to clear the rising / falling transition interrupt flag		 	   */		
/*---------------------------------------------------------------------------------------------------------*/
VOID PWM_ClearCaptureIntStatus(UINT8 u8Capture, UINT8 u8IntType)
{
	if(u8Capture & 0x02)
		outp32(CCR1, (inp32(CCR1) & ~(CFLRD3 | CRLRD3 | CIIR3 | CFLRD2 | CRLRD2 |CIIR2)) | (1 << (((u8Capture & 0x01) << 4) + u8IntType)));
	else
		outp32(CCR0, (inp32(CCR0) & ~(CFLRD1 | CRLRD1 | CIIR1 | CFLRD0 | CRLRD0 |CIIR0)) | (1 << (((u8Capture & 0x01) << 4) + u8IntType)));
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function: PWM_Open				                                                   					   */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*               None             											                               */
/*              											                                        	   */
/* Returns:                                                                                                */
/*               None 											                    		       		   */
/*               							                                                               */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*               Enable PWM engine clock and reset PWM											 	   	   */		
/*---------------------------------------------------------------------------------------------------------*/
VOID PWM_Open(VOID)
{ 
	outp32(REG_APBCLK, inp32(REG_APBCLK)|PWM_CKE);
	outp32(REG_APBIPRST, PWMRST);
	outp32(REG_APBIPRST, inp32(REG_APBIPRST) & ~PWMRST);
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function: PWM_Close				                                                   					   */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*               None             											                               */
/*              											                                        	   */
/* Returns:                                                                                                */
/*               None 											                    		       		   */
/*               							                                                               */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*               Disable PWM engine clock and the I/O enable											   */		
/*---------------------------------------------------------------------------------------------------------*/
VOID PWM_Close(VOID)
{
	outp32(POE, 0);
	outp32(CAPENR,0x0);
   	sysDisableInterrupt(IRQ_PWM);
	outp32(REG_APBCLK, inp32(REG_APBCLK) & ~PWM_CKE);
	
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function: PWM_EnableDeadZone				                               		              		       */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*               u8Timer 		- [in]		PWM_TIMER0/PWM_TIMER1/PWM_TIMER2/PWM_TIMER3     			   */
/*               u8Length 		- [in]		Dead Zone Length : 0~255				       		   		   */
/*               bEnableDeadZone- [in]		Enable DeadZone (TRUE) / Diasble DeadZone (FALSE)     		   */
/*              											                                        	   */
/* Returns:                                                                                                */
/*               None											                    		               */
/*               							                                                               */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*               This function is used to set the dead zone length and enable/disable Dead Zone function   */		
/*---------------------------------------------------------------------------------------------------------*/
VOID PWM_EnableDeadZone(UINT8 u8Timer, UINT8 u8Length,BOOL bEnableDeadZone)
{
	if(u8Timer & 0x02)
	{
		outp32(PPR, (inp32(PPR) & ~DZI1) | ((u8Length & 0xFF) << 24));
		outp32(PCR, (inp32(PCR) & ~DZEN1) | (bEnableDeadZone?DZEN1:0));			
	}
	else
	{
		outp32(PPR, (inp32(PPR) & ~DZI0) | ((u8Length & 0xFF) << 16));				
		outp32(PCR, (inp32(PCR) & ~DZEN0) | (bEnableDeadZone?DZEN0:0));		
	}	
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function: PWM_Enable				                            		                 			       */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*               u8Timer 		- [in]		PWM_TIMER0/PWM_TIMER1/PWM_TIMER2/PWM_TIMER3     			   */
/*         									PWM_CAP0/PWM_CAP1/PWM_CAP2/PWM_CAP3        					   */
/*               bEnable		- [in]		Enable  (TRUE) / Disable  (FALSE)     					       */
/*              											                                        	   */
/* Returns:                                                                                                */
/*               None											                    		               */
/*               							                                                               */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*               This function is used to enable PWM timer / capture function					 		   */		
/*---------------------------------------------------------------------------------------------------------*/
VOID PWM_Enable(UINT8 u8Timer,BOOL bEnable)
{
	outp32(PCR, (inp32(PCR)& ~(1 << ((u8Timer & 0x07) << 3) )) | (bEnable?(1 << ((u8Timer & 0x07) << 3) ):0) );
	
	if(u8Timer & 0x10)
	{
		if(u8Timer & 0x02)
			outp32(CCR1, (inp32(CCR1) & ~((1 << (((u8Timer & 0x01) << 4) + 3))) | (CFLRD3 | CRLRD3 | CIIR3 | CFLRD2 | CRLRD2 |CIIR2)) | (bEnable?(1 << (((u8Timer & 0x01) << 4) + 3)):0));	
		else
			outp32(CCR0, (inp32(CCR0) & ~((1 << (((u8Timer & 0x01) << 4) + 3))) | (CFLRD1 | CRLRD1 | CIIR1 | CFLRD0 | CRLRD0 |CIIR0)) | (bEnable?(1 << (((u8Timer & 0x01) << 4) + 3)):0));
	}
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function: PWM_SetTimerClk					                                             		       */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*               u8Timer 						PWM_TIMER0/PWM_TIMER1/PWM_TIMER2/PWM_TIMER3   	  		   */
/*         										PWM_CAP0/PWM_CAP1/PWM_CAP2/PWM_CAP3        				   */
/*				 PWM_TIME_DATA_T *sPt																	   */
/*               	u8Frequency					Frequency   					       					   */
/*               	u8HighPulseRatio			High Pulse Ratio    					       			   */
/*               	u8Mode						PWM_ONE_SHOT_MODE / PWM_TOGGLE_MODE 					   */
/*               	bInverter					Inverter Enable  (TRUE) / Inverter Disable  (FALSE)        */
/*               	u8ClockSelector				Clock Selector											   */
/*              								PWM_CLOCK_DIV_1/PWM_CLOCK_DIV_2/PWM_CLOCK_DIV_4   		   */
/*              								PWM_CLOCK_DIV_8/PWM_CLOCK_DIV_16						   */
/*												(The parameter takes effect when u8Frequency = 0)		   */
/*               	u8PreScale					Prescale (2 ~ 256)										   */ 
/*												(The parameter takes effect when u8Frequency = 0)		   */
/*               	u32Duty						Pulse duty										           */                             
/*												(The parameter takes effect when u8Frequency = 0 or		   */
/*												u8Timer = PWM_CAP0/PWM_CAP1/PWM_CAP2/PWM_CAP3)			   */	
/*              											                                        	   */
/* Returns:                                                                                                */
/*               The actual frequency											                           */
/*               							                                                               */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*               This function is used to configure the frequency/pulse/mode/inverter function		       */		
/*---------------------------------------------------------------------------------------------------------*/
FLOAT PWM_SetTimerClk(UINT8 u8Timer, PWM_TIME_DATA_T *sPt)
{
	FLOAT 	fFrequency;
	UINT16	u16Duty;
	UINT32 	u32ApbHz;
	
	u32ApbHz = sysGetAPBClock() * 1000;
						
	if(u8Timer & 0x10)	
	{
		if(u8Timer & 0x02)
			outp32(CCR1, (inp32(CCR1) & ~((1 << (((u8Timer & 0x01) << 4)))) | (CFLRD3 | CRLRD3 | CIIR3 | CFLRD2 | CRLRD2 |CIIR2)) | (sPt->bInverter?(1 << ((u8Timer & 0x01) << 4)):0));
		else
			outp32(CCR0, (inp32(CCR0) & ~((1 << (((u8Timer & 0x01) << 4)))) | (CFLRD1 | CRLRD1 | CIIR1 | CFLRD0 | CRLRD0 |CIIR0)) | (sPt->bInverter?(1 << ((u8Timer & 0x01) << 4)):0));
	}
	else
		outp32(PCR, (inp32(PCR) & ~(1 << ((((u8Timer & 0x07)) << 3) + 2) )) |(sPt->bInverter?(1 << (((u8Timer & 0x07) << 3) + 2)):0));	
	
	outp32(PCR, (inp32(PCR) & ~(1 <<(((u8Timer & 0x07) << 3) + 3))) | ( sPt->u8Mode? (1 <<(((u8Timer & 0x07) << 3) + 3)):0));
	
	if(sPt->fFrequency == 0)
	{	
		UINT8	u8Divider;
		
		if(u8Timer & 0x02)	
			outp32(PPR, (inp32(PPR) & ~CP1) | (((sPt->u8PreScale - 1) & 0xFF) << 8));
		else	
			outp32(PPR, (inp32(PPR) & ~CP0) | ((sPt->u8PreScale - 1) & 0xFF));
			
		u8Divider = 1;	
		switch(sPt->u8ClockSelector)
		{
			case PWM_CLOCK_DIV_1:
				u8Divider = 4;
				break;
			case PWM_CLOCK_DIV_2:
				u8Divider = 0;			
				break;			
			case PWM_CLOCK_DIV_4:
				u8Divider = 1;			
				break;			
			case PWM_CLOCK_DIV_8:
				u8Divider = 2;			
				break;			
			case PWM_CLOCK_DIV_16: 
				u8Divider = 3;			
				break;		
		}				
			
		outp32(PWM_CSR, (inp32(PWM_CSR) & ~(0x7 << ((u8Timer & 0x07)<<2) )) | ((u8Divider & 0x7) << ((u8Timer & 0x07)<<2)));	
		outp32(CNR0 + (u8Timer & 0x07) * 12, (sPt->u32Duty - 1));
		outp32(CMR0 + (u8Timer & 0x07) * 12, (sPt->u32Duty * sPt->u8HighPulseRatio / 100 - 1));			
				
		fFrequency = (FLOAT)u32ApbHz / sPt->u8PreScale / sPt->u8ClockSelector / sPt->u32Duty;
	}
	else
	{
		UINT8	u8Divider;
		UINT16	u16PreScale;
		
		fFrequency = (FLOAT)  u32ApbHz / sPt->fFrequency;		
		
		if(fFrequency > 0x10000000)
			return 0;
			
		u8Divider = 1;			
			
		if(fFrequency < 0x20000)
			u16PreScale = 2;	
		else
		{
			u16PreScale = fFrequency / 65536;	
							
			if(fFrequency / u16PreScale > 65536)
				u16PreScale++;
			
			if(u16PreScale > 256)
			{
				UINT8 u8i;
				u16PreScale = 256;
				fFrequency = fFrequency / u16PreScale;
				
				u8Divider = fFrequency / 65536;				
				
				if(fFrequency / u8Divider > 65536)
					u8Divider++;				
				
				u8i = 0;
				while(1)	
				{
					if((1 << u8i++) > u8Divider)
						break;
				}
				
				u8Divider = 1 << (u8i - 1);
				
				if(u8Divider > 16)
					return 0;	
					
				fFrequency = fFrequency * u16PreScale;						
			}		
					
		}
		u16Duty = (UINT16 )(fFrequency/u16PreScale/u8Divider);
		
		fFrequency = ((FLOAT) u32ApbHz / u16PreScale / u8Divider) / u16Duty;	
				
		if(u8Timer & 0x02)	
			outp32(PPR, (inp32(PPR) & ~CP1) | (((u16PreScale - 1) & 0xFF) << 8));
		else	
			outp32(PPR, (inp32(PPR) & ~CP0) | ((u16PreScale - 1) & 0xFF));	
				
		sPt->u8ClockSelector = u8Divider;	
				
		switch(u8Divider)
		{
			case 1:
				u8Divider = 4;
				break;
			case 2:
				u8Divider = 0;			
				break;			
			case 4:
				u8Divider = 1;			
				break;			
			case 8:
				u8Divider = 2;			
				break;			
			case 16:
				u8Divider = 3;			
				break;		
		}		
		
		sPt->u8PreScale = u16PreScale;		
							
		outp32(PWM_CSR, (inp32(PWM_CSR) & ~(0x7 << ((u8Timer & 0x07)<<2) )) | ((u8Divider & 0x7) << ((u8Timer & 0x07)<<2)));	
		
		if(u8Timer & 0x10)
		{			
			if(sPt->u32Duty != 0)
			{
				outp32(CNR0 + (u8Timer & 0x07) * 12, (sPt->u32Duty - 1));
				outp32(CMR0 + (u8Timer & 0x07) * 12, (sPt->u32Duty * sPt->u8HighPulseRatio / 100 - 1));			
			}
			else
			{
				outp32(CNR0 + (u8Timer & 0x07) * 12, (u16Duty - 1));
				outp32(CMR0 + (u8Timer & 0x07) * 12, (u16Duty * sPt->u8HighPulseRatio / 100 - 1));				
			}
		}
		else
		{	
			outp32(CNR0 + (u8Timer & 0x07) * 12, (u16Duty - 1));			
			outp32(CMR0 + (u8Timer & 0x07) * 12, (u16Duty * sPt->u8HighPulseRatio / 100 - 1));			
		}
	}
	
	return fFrequency;

}


/*---------------------------------------------------------------------------------------------------------*/
/* Function: PWM_SetTimerIO					                                             		           */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*               u8Timer 		- [in]		PWM_TIMER0/PWM_TIMER1/PWM_TIMER2/PWM_TIMER3   				   */
/*         									PWM_CAP0/PWM_CAP1/PWM_CAP2/PWM_CAP3        		 			   */
/*               bEnable		- [in]		Enable  (TRUE) / Diasble  (FALSE)     					       */
/*              											                                        	   */
/* Returns:                                                                                                */
/*               None											                    		               */
/*               							                                                               */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*               This function is used to enable/disable PWM timer / capture I/O function				   */		
/*---------------------------------------------------------------------------------------------------------*/
VOID PWM_SetTimerIO(UINT8 u8Timer, BOOL bEnable)
{
	
	if(bEnable)
	{
		if(u8Timer & 0x10)
			outp32(CAPENR, inp32(CAPENR)  | (1 << (u8Timer & 0x07)));			
		else
			outp32(POE, inp32(POE)  | (1 << (u8Timer & 0x07)));	
			
		outp32(REG_GPDFUN, (inp32(REG_GPDFUN) & ~(0x3 << ((u8Timer & 0x07) * 2))) | (0x2 << ((u8Timer & 0x07) * 2)));
			
	}
	else
	{
		if(u8Timer & 0x10)			
			outp32(CAPENR, inp32(CAPENR) & ~(1 << (u8Timer & 0x07)));			
		else
			outp32(POE, inp32(POE) & ~(1 << (u8Timer & 0x07)));
		outp32(REG_GPDFUN, (inp32(REG_GPDFUN) & ~(0x3 << ((u8Timer & 0x07) * 2))));	
	}
}

