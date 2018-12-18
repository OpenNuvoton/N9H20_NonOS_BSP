#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "N9H20.h"

static INT32 g_PdmaCh = 0;

#define E_UART_BUF 32

__align(32) UINT8 g_UARTBuf[E_UART_BUF];

volatile BOOL bIsBufferDone=0;
volatile BOOL bIsUARTDone=FALSE;

void PdmaCallback_UART(UINT32 u32WrapStatus)
{
	if(u32WrapStatus==256)
	{
		bIsBufferDone = 1;				
		bIsUARTDone = TRUE;
	}	
	else if(u32WrapStatus==1024)
	{		
		bIsBufferDone = 2;			
	}
}

void UARTTest(void)
{	
	UINT32 i;

	sysprintf("UART1 will receive 32 char and print input char on terminal\n");
	
	g_PdmaCh = PDMA_FindandRequest(); 

	EDMA_SetAPB(g_PdmaCh,			//int channel, 
						eDRVEDMA_UART1,			//E_DRVEDMA_APB_DEVICE eDevice, 						
						eDRVEDMA_READ_APB,		//E_DRVEDMA_APB_RW eRWAPB, 
						eDRVEDMA_WIDTH_8BITS);	//E_DRVEDMA_TRANSFER_WIDTH eTransferWidth	

	EDMA_SetupHandlers(g_PdmaCh, 		//int channel
						eDRVEDMA_WAR, 			//int interrupt,	
						PdmaCallback_UART, 				//void (*irq_handler) (void *),
						NULL);					//void *data

	EDMA_SetWrapINTType(g_PdmaCh , 
								eDRVEDMA_WRAPAROUND_EMPTY | 
								eDRVEDMA_WRAPAROUND_HALF);	//int channel, WR int type

	EDMA_SetDirection(g_PdmaCh , eDRVEDMA_DIRECTION_FIXED, eDRVEDMA_DIRECTION_WRAPAROUND);


	EDMA_SetupSingle(g_PdmaCh,		// int channel, 
								0xB8008100,		// unsigned int src_addr,  (ADC data port physical address) 								
								(UINT32)g_UARTBuf |NON_CACHE_BIT, //phaddrrecord,		// unsigned int dest_addr,
								E_UART_BUF);	// unsigned int dma_length /* Lenth equal 2 half buffer */

	outp32(0xb8008104, inp32(0xb8008104) | EDMA_RX_EN);
	
	EDMA_Trigger(g_PdmaCh);

	while(bIsUARTDone != TRUE);

	outp32(0xb8008104, inp32(0xb8008104) & ~EDMA_RX_EN);
	EDMA_Free(g_PdmaCh);
	bIsUARTDone = FALSE;

	for(i=0;i<E_UART_BUF;i++)
		sysprintf("%c", g_UARTBuf[i]);

	sysprintf("\n\n");
	
}








