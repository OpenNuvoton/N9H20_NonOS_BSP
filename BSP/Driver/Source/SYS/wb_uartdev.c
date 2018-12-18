#include "wblib.h"
extern UARTDEV_T nvt_uart0;
extern UARTDEV_T nvt_uart1;
INT32 register_uart_device(UINT32 u32port, UARTDEV_T* pUartDev)
{
	if(u32port==0)
		*pUartDev = nvt_uart0;
	else if(u32port==1)
		*pUartDev = nvt_uart1;
	else 	
		return -1;
	return Successful;	
}
