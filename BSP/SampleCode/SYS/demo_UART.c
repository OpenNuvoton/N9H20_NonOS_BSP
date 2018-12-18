/***************************************************************************
 *                                                                         									     *
 * Copyright (c) 2008 Nuvoton Technolog. All rights reserved.              					     *
 *                                                                         									     *
 ***************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include "N9H20.h"
#include "demo.h"

void DemoAPI_UART(void)
{
	DBG_PRINTF("\nUART Test...\n");
#if 1
	DBG_PRINTF("Type 'y' to finish UART test...\n");
	while(1)
	{
		if (sysGetChar() == 'y')   break;
	}
#endif
	DBG_PRINTF("Finish UART Testing...\n");
}
