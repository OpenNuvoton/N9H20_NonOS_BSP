/**************************************************************************//**
 * @file     DrvI2CH.c
 * @version  V3.00
 * @brief    N9H20 series I2C driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include "wblib.h"

#include "DrvI2CH.h"

volatile PFN_DRVI2CH_INT_CALLBACK g_pfnI2CHCallback = {0};

/*---------------------------------------------------------------------------------------------------------*/
/* Function: DrvI2CH_ISR  		                                                                           */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*      None																		                       */
/*                                                                                                         */
/* Returns:                                                                                                */
/*      None 						                                                                       */
/*                                                                                                         */
/* Description:                                                                                            */
/*      I2CH Interrupt Service Routine                                                                      */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
void DrvI2CH_ISR(void)
{
	if (g_pfnI2CHCallback != 0)
	{
		if (DrvI2CH_IsIntEnabled() == TRUE)	
		{
			if (DrvI2CH_PollInt() == TRUE)
				g_pfnI2CHCallback(0,0);				
		}				
	}					

	DrvI2CH_ClearInt();	
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function: DrvI2CH_InstallCallBack                                                                        */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*           																	                           */
/*                                                                                                         */
/* Returns:                                                                                                */
/*      ERRCODE							                                                                   */
/*                                                                                                         */
/* Description:                                                                                            */
/*      Install I2CH interrupt Callback Function                                                            */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
ERRCODE
DrvI2CH_InstallCallBack(
	PFN_DRVI2CH_INT_CALLBACK  pfnCallback,
	PFN_DRVI2CH_INT_CALLBACK *pfnOldCallback
)
{
	*pfnOldCallback = g_pfnI2CHCallback; 	// return previous installed callback function pointer
	g_pfnI2CHCallback = pfnCallback;        // install current callback function

	sysInstallISR(IRQ_LEVEL_1, IRQ_I2C, (PVOID)DrvI2CH_ISR);	
	sysEnableInterrupt(IRQ_I2C);	
	
	return Successful;
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function: DrvI2CH_InitSDASCK                                                                             */
/*                                                                                                         */
/* Parameters:  None                                                                                       */
/*                                                                                                         */
/*                                                                                                         */
/* Returns:     None                                                                                       */
/*                                                                                                         */
/* Description:                                                                                            */
/*               This function is used to initial SDA and SDK of I2C                                       */
/*---------------------------------------------------------------------------------------------------------*/
void DrvI2CH_InitSDASCK(void)
{
	outp32 (REG_I2C_CSR, inp32(REG_I2C_CSR)&(~I2C_EN)&(~TX_NUM) );	        /* I2C core disable, Tx_NUM=0 */
	outp32 (REG_I2C_CSR, inp32(REG_I2C_CSR)| CSR_IF | CSR_IE | I2C_EN );    /* I2C core enable            */
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function: DrvI2CH_IsBusy                                                                                 */
/*                                                                                                         */
/* Parameters:  None                                                                                       */
/*                                                                                                         */
/*                                                                                                         */
/* Returns:                                                                                                */
/*              Return TRUE if the I2C is in busy.                                                         */
/*                                                                                                         */
/* Description:                                                                                            */
/*               This function is used to check if I2C is in busy.                                         */
/*----  -----------------------------------------------------------------------------------------------------*/
BOOL 
DrvI2CH_IsBusy(void)
{	
    return ((inp32(REG_I2C_CSR)&I2C_TIP)?TRUE:FALSE);
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function: DrvI2CH_IsBusBusy                                                                              */
/*                                                                                                         */
/* Parameters:  None                                                                                       */
/*                                                                                                         */
/*                                                                                                         */
/* Returns:                                                                                                */
/*              Return TRUE if the I2C bus is in busy.                                                     */
/*                                                                                                         */
/* Description:                                                                                            */
/*               This function is used to check if I2C bus is in busy. The function will return TRUE if    */
/*               there is START signal on bus and it will return FALSE until STOP signal.                  */
/*---------------------------------------------------------------------------------------------------------*/
BOOL 
DrvI2CH_IsBusBusy(void)
{
    return ((inp32(REG_I2C_CSR)&I2C_BUSY)?TRUE:FALSE);
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function: DrvI2CH_IsArbitLost                                                                            */
/*                                                                                                         */
/* Parameters:  None                                                                                       */
/*                                                                                                         */
/*                                                                                                         */
/* Returns:                                                                                                */
/*              Return TRUE if arbitration error on I2C bus.                                               */
/*                                                                                                         */
/* Description:                                                                                            */
/*               If the arbitration error is found on I2C bus, this function will return TRUE and send     */
/*               START and STOP command to the bus to try to recover the error.                            */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
BOOL 
DrvI2CH_IsArbitLost(void)
{	
	if( (inp32(REG_I2C_CSR)&I2C_AL) ==0x0)	
		return FALSE;						/* No arbitration lose */
	else
	{	
	    /* Send clocks out when arbitration error to try to re-cover the device status */
		outp32(REG_I2C_CMDR, 0x10);					/* Send START signal to I2C bus */
		outp32(REG_I2C_CMDR, 0x08);					/* Send STOP signal to I2C bus  */
		
		return TRUE;						/* Arbitration error            */
	}	
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function: DrvI2CH_SetBurstCnt                                                                            */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*              u8BurstCnt  - [in], The sequential transfer number. It could be 1~4.                       */
/*                                                                                                         */
/* Returns:                                                                                                */
/*              E_SUCCESS                                                                                  */
/*              E_DRVI2CH_WRONG_LENGTH   Invalid sequential transfer number.                                */
/*                                                                                                         */
/* Description:                                                                                            */
/*              To set the burst transfer count. The I2C could be up to 4 transfers when trigger to        */
/*              send/get data                                                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
ERRCODE 
DrvI2CH_SetBurstCnt(
	UINT8 u8BurstCnt
)
{
	if( (u8BurstCnt>0) && (u8BurstCnt<=4) )
	{
		outp32(REG_I2C_CSR, inp32(REG_I2C_CSR)&(~TX_NUM) );		
		outp32(REG_I2C_CSR, inp32(REG_I2C_CSR)|((u8BurstCnt-1)<<4) );
	}	
	else 
		return 	E_DRVI2CH_WRONG_LENGTH;
	return 	Successful;
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function: DrvI2CH_SetTxData                                                                              */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*              u32TxData   - [in], The data to send.                                                      */
/*                                                                                                         */
/* Returns:                                                                                                */
/*              None                                                                                       */
/*                                                                                                         */
/* Description:                                                                                            */
/*              Set the data to send                                                                       */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
void DrvI2CH_SetTxData(
	UINT32 u32TxData
)
{
	outp32(REG_I2C_TxR, u32TxData);
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function: DrvI2CH_SendCmd                                                                               */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*              eCmd   - [in], The I2C command. It could be eDRVI2CH_ACK, eDRVI2CH_WRITE, eDRVI2CH_READ,   */
/*                            eDRVI2CH_STOP, eDRVI2CH_START. It is also able to send two commands at 	   */	
/* 							  the same time by "OR",i.e eDRVI2CH_START | eDRVI2CH_READ | eDRVI2CH_ACK |    */
/*							  eDRVI2CH_STOP. The sequences of the I2C actions are START => READ => ACK =>  */
/* 							  STOP when these commands are send at the same time.          			       */
/*                                                                                                         */
/*                                                                                                         */
/* Returns:                                                                                                */
/*              None                                                                                       */
/*                                                                                                         */
/* Description:                                                                                            */
/*              Send the I2C command to the hardware to do the relative action.                            */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
void DrvI2CH_SendCmd(
	E_DRVI2CH_CMD eCmd
)
{
	outp32 (REG_I2C_CMDR, eCmd);		
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function: DrvI2CH_IsACK                                                                                  */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*              None                                                                                       */
/*                                                                                                         */
/* Returns:                                                                                                */
/*              Return TRUE if ACK received from slave, otherwize return FALSE                             */
/*                                                                                                         */
/* Description:                                                                                            */
/*              This function is used to check if ACK received from slave after sending data to slave      */
/*              device.                                                                                    */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
BOOL 
DrvI2CH_IsACK(void)
{
	if( (inp32(REG_I2C_CSR)&I2C_RXACK)==0 )
		return TRUE;
	else
		return FALSE;
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function: DrvI2CH_GetRxData                                                                              */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*              None                                                                                       */
/*                                                                                                         */
/* Returns:                                                                                                */
/*              Return the last byte which is received from I2C bus                                        */
/*                                                                                                         */
/* Description:                                                                                            */
/*              To read the last byte which is received from I2C bus                                       */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
UINT8 
DrvI2CH_GetRxData(void)
{
	return ( inp32(REG_I2C_RxR)&0xff );
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function: DrvI2CH_EnableInt                                                                              */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*              bIsInterrupt - [in], TRUE = Enable the interrupt, FASE = Disable the I2C interrupt         */
/*                                                                                                         */
/* Returns:                                                                                                */
/*              None                                                                                       */
/*                                                                                                         */
/* Description:                                                                                            */
/*              To enable the I2C interrupt.                                                               */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
void DrvI2CH_EnableInt(void)
{
	outp32( REG_I2C_CSR, inp32(REG_I2C_CSR) | CSR_IE);
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function: DrvI2CH_DisableInt                                                                            */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*              None																					   */
/*                                                                                                         */
/* Returns:                                                                                                */
/*              None                                                                                       */
/*                                                                                                         */
/* Description:                                                                                            */
/*              To enable the I2C interrupt.                                                               */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
void DrvI2CH_DisableInt(void)
{
	outp32( REG_I2C_CSR, inp32(REG_I2C_CSR) & ~CSR_IE);
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function: DrvI2CH_IsIntEnabled                                                                           */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*              None                                                                                       */
/*                                                                                                         */
/* Returns:                                                                                                */
/*              TRUE = I2C Interrupt enabled. FALSE = I2C Interrupt disabled.                              */
/*                                                                                                         */
/* Description:                                                                                            */
/*              Get the interrupt enable or disable status.                                                */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
BOOL 
DrvI2CH_IsIntEnabled(void)
{
	return ((inp32(REG_I2C_CSR)&(CSR_IE)) ? TRUE:FALSE);
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function: DrvI2CH_PollInt                                                                               */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*              None                                                                                       */
/*                                                                                                         */
/* Returns:                                                                                                */
/*              True/False                                                                                       */
/*                                                                                                         */
/* Description:                                                                                            */
/*              To return I2C interrupt flag.                                                           */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
BOOL
DrvI2CH_PollInt(void)
{
	return ((inp32(REG_I2C_CSR)&(CSR_IF)) ? TRUE:FALSE);	
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function: DrvI2CH_ClearInt                                                                               */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*              None                                                                                       */
/*                                                                                                         */
/* Returns:                                                                                                */
/*              None                                                                                       */
/*                                                                                                         */
/* Description:                                                                                            */
/*              To clear the I2C interrupt flag.                                                           */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
void DrvI2CH_ClearInt(void)
{
	outp32( REG_I2C_CSR, inp32(REG_I2C_CSR) |  CSR_IF); 
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function: DrvI2CH_Open                                                                                   */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*              u32I2cClock - [in], To configure the I2C bus clock. its unit is Hz.                        */
/*                                                                                                         */
/* Returns:                                                                                                */
/*              E_SUCCESS                                                                                  */
/*                                                                                                         */
/* Description:                                                                                            */
/*              To open the I2C hardware and configure the I2C bus clock.                                  */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
ERRCODE 
DrvI2CH_Open(
	UINT32 u32I2cClock
)
{	
	UINT32 u32ApbKHz;
	INT32 n32Divider;
	
	// enable I2C pin function (switch pin is removed to GPIO driver)
//	outp32(PINFUN1, inp32(PINFUN1) & ~(PF_ISDA|PF_ISCK));
//	outp32(PINFUN1, inp32(PINFUN1) | (0x0F << 16));

	// enable I2C engine clock 
	outp32(REG_APBCLK, inp32(REG_APBCLK) | I2C_CKE);	

	// reset I2C engine 
	outp32(REG_APBIPRST, inp32(REG_APBIPRST) | I2CRST);
	outp32(REG_APBIPRST, inp32(REG_APBIPRST) & ~I2CRST);	

	u32ApbKHz = sysGetAPBClock();

	n32Divider = u32ApbKHz/(5*u32I2cClock)-1;
	if(n32Divider<=1)
		n32Divider = 1;
	
	outp32(REG_I2C_DIVIDER,n32Divider);								

    outp32(REG_I2C_CSR, (inp32(REG_I2C_CSR)&(~TX_NUM)) | I2C_EN);		
	return Successful;
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function: DrvI2CH_Close                                                                                  */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*              None                                                                                       */
/*                                                                                                         */
/* Returns:                                                                                                */
/*              None                                                                                       */
/*                                                                                                         */
/* Description:                                                                                            */
/*              To close the I2C hardware.                                                                 */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
void DrvI2CH_Close(void)
{
	outp32(REG_APBIPRST, inp32(REG_APBIPRST) | I2CRST);
	outp32(REG_APBIPRST, inp32(REG_APBIPRST) & ~I2CRST);	

	outp32(REG_APBCLK, inp32(REG_APBCLK) & ~I2C_CKE);	

}

/*---------------------------------------------------------------------------------------------------------*/
/* Function: DrvI2CH_Delay                                                                                  */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*              nCount - [in], The delay count in micro-second.                                            */
/*                                                                                                         */
/* Returns:                                                                                                */
/*              None                                                                                       */
/*                                                                                                         */
/* Description:                                                                                            */
/*              This is a delay function                                                                   */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
static void DrvI2CH_Delay(
	UINT32 u32Count
)
{
	UINT32 ii, jj;
	
	for(ii=0; ii<u32Count; ii++)
		for (jj=0; jj<10; jj++){}

//    DrvSYS_RoughDelay(nCount);
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function: DrvI2CH_WaitReady                                                                              */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*              None                                                                                       */
/*                                                                                                         */
/* Returns:                                                                                                */
/*              E_SUCCESS             Success                                                              */
/*              E_DRVI2CH_ARBIT_LOSE   Bus arbitration error                                                */
/*              E_DRVI2CH_TIME_OUT     I2C time out                                                         */
/*                                                                                                         */
/* Description:                                                                                            */
/*              Wait for I2C hardware ready.                                                               */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
ERRCODE 
DrvI2CH_WaitReady(void)
{	
	UINT32 u32ErrCode=Successful;
	UINT32 u32DelayCount=0;
	
	if (DrvI2CH_IsArbitLost()==TRUE)	
		return E_DRVI2CH_ARBIT_LOSE;
		
	while(DrvI2CH_IsBusy() && u32DelayCount!=500)	
	{
		DrvI2CH_Delay(10);
		u32DelayCount++;
	}
	
	if(u32DelayCount==500)
		return E_DRVI2CH_TIME_OUT;		
	
	return u32ErrCode;
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function: DrvI2CH_SendStart                                                                              */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*              None                                                                                       */
/*                                                                                                         */
/* Returns:                                                                                                */
/*              E_SUCCESS             Success                                                              */
/*              E_DRVI2CH_ARBIT_LOSE   Bus arbitration error                                                */
/*              E_DRVI2CH_TIME_OUT     I2C time out                                                         */
/*                                                                                                         */
/* Description:                                                                                            */
/*              To send START signal to I2C bus.                                                           */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
ERRCODE 
DrvI2CH_SendStart(void)
{	
	UINT32 u32ErrCode;						
	DrvI2CH_SendCmd(eDRVI2CH_START);
	u32ErrCode=DrvI2CH_WaitReady();
	return u32ErrCode;
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function: DrvI2CH_SendStop                                                                               */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*              None                                                                                       */
/*                                                                                                         */
/* Returns:                                                                                                */
/*              E_SUCCESS             Success                                                              */
/*              E_DRVI2CH_ARBIT_LOSE   Bus arbitration error                                                */
/*              E_DRVI2CH_TIME_OUT     I2C time out                                                         */
/*                                                                                                         */
/* Description:                                                                                            */
/*              To send STOP signal to I2C bus.                                                            */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
ERRCODE 
DrvI2CH_SendStop(void)
{	
	UINT32 u32ErrCode;						
	DrvI2CH_SendCmd(eDRVI2CH_STOP);
	u32ErrCode=DrvI2CH_WaitReady();
	return u32ErrCode;
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function: DrvI2CH_WriteByte                                                                              */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*              bStart - [in], Enable to send START signal before send data.                               */
/*              u8Data - [in], The byte to send through I2C bus.                                           */
/*              bCheckAck - [in], Enable to check ACK after send data.                                     */
/*              bStop - [in], Enable to send STOP signal at the end.                                       */
/*                                                                                                         */
/* Returns:                                                                                                */
/*              E_SUCCESS             Success                                                              */
/*              E_DRVI2CH_ARBIT_LOSE   Bus arbitration error                                                */
/*              E_DRVI2CH_TIME_OUT     I2C time out                                                         */
/*              E_DRVI2CH_NACK         NACK received                                                        */
/*                                                                                                         */
/* Description:                                                                                            */
/*              Send a byte to I2C bus.                                                                    */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
ERRCODE 
DrvI2CH_WriteByte(
	BOOL bStart, 
	UINT8 u8Data, 
	BOOL bCheckAck, 
	BOOL bStop
)
{
    UINT32 u32ErrCode;
    E_DRVI2CH_CMD eCmd;
    
	if (DrvI2CH_IsArbitLost())
		return	E_DRVI2CH_ARBIT_LOSE;
			
    DrvI2CH_SetBurstCnt(1);
	DrvI2CH_SetTxData(u8Data);
	eCmd = eDRVI2CH_WRITE;
	if(bStart)
	    eCmd = (E_DRVI2CH_CMD)(eCmd | eDRVI2CH_START);
	if(bStop)
	    eCmd = (E_DRVI2CH_CMD)(eCmd | eDRVI2CH_STOP);
    DrvI2CH_SendCmd(eCmd);

	
	u32ErrCode=DrvI2CH_WaitReady();
	if (u32ErrCode==Successful)	
	{		
	    if(bCheckAck)
	    {
    		if(DrvI2CH_IsACK() == FALSE)
    			return	E_DRVI2CH_NACK;
	    }
	}	
		
	return	u32ErrCode;
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function: DrvI2CH_ReadByte                                                                               */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*              bStart - [in], Enable to send START signal before send data.                               */
/*              pu8ReadData - [in], The byte to read through I2C bus.                                      */
/*              bSendAck - [in], Enable to send ACK after read data.                                       */
/*              bStop - [in], Enable to send STOP signal at the end.                                       */
/*                                                                                                         */
/* Returns:                                                                                                */
/*              E_SUCCESS             Success                                                              */
/*              E_DRVI2CH_ARBIT_LOSE   Bus arbitration error                                                */
/*              E_DRVI2CH_TIME_OUT     I2C time out                                                         */
/*                                                                                                         */
/* Description:                                                                                            */
/*              Read a byte from I2C bus.                                                                    */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
ERRCODE 
DrvI2CH_ReadByte(
	BOOL bStart, 
	PUINT8 pu8ReadData, 
	BOOL bSendAck, 
	BOOL bStop
)
{
	UINT32 u32ErrCode = Successful;
	
	DrvI2CH_SendCmd((E_DRVI2CH_CMD)(((bStart)?eDRVI2CH_START:0) | eDRVI2CH_READ | ((bSendAck)?0:eDRVI2CH_ACK) | ((bStop)?eDRVI2CH_STOP:0)));
	
	u32ErrCode = DrvI2CH_WaitReady();
	if(u32ErrCode == Successful)
        *pu8ReadData=DrvI2CH_GetRxData();
			
	return u32ErrCode;				
}

                
