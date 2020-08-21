/**************************************************************************//**
 * @file     SPIGlue.h
 * @version  V3.00
 * @brief    N9H20 series SPI driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>

#include "wbio.h"


#define TIMER0  0

//-- function return value
#define	   Successful  0
#define	   Fail        1


#define STOR_STRING_LEN	32

/* we allocate one of these for every device that we remember */

typedef struct disk_data_t
{
    struct disk_data_t  *next;           /* next device */

    /* information about the device -- always good */
	unsigned int  totalSectorN;			/* SPI flash size in Kbyte*/
	unsigned int  diskSize;				/* disk size in Kbytes */
	int           sectorSize;
    char          vendor[STOR_STRING_LEN];
    char          product[STOR_STRING_LEN];
    char          serial[STOR_STRING_LEN];
} DISK_DATA_T;


#define SPI_NO_MEMORY			(SPI_ERR_ID|0x02)
#define SPI_INIT_TIMEOUT		(SPI_ERR_ID|0x12)

