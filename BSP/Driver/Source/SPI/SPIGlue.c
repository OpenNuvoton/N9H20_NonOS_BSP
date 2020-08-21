/**************************************************************************//**
 * @file     SPIGlue.c
 * @version  V3.00
 * @brief    N9H20 series SPI driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wblib.h"

#include "N9H20_SPI.h"
#include "N9H20_NVTFAT.h"
#include "SPIGlue.h"


#define	SECTOR_NUM	512				// One Sector = 512B for MSC
#define READ_ID_CMD	0x9F			// Change it if the SPI read ID commmand is not 0x9F

DISK_DATA_T SPI_DiskInfo0;
PDISK_T *pDisk_SPI = NULL;

static int spi_ok=0;
int spi_FAT_offset=0;

extern UINT8 g_u8Is4ByteMode;

//Below sector size and erase command maybe changed depend on the used SPI flash spec
//Default use command 0x20 to erase 4K Sector
#define		SPI_SECTOR_ERASE_SIZE	4*1024
#define		SPI_ERASE_CMD			0x20

#if defined(__GNUC__)
UINT8 SPI_BackupBuf[SPI_SECTOR_ERASE_SIZE] __attribute__((aligned(32)));	// Backup Buffer to save SPI content
#else
__align(32) UINT8 SPI_BackupBuf[SPI_SECTOR_ERASE_SIZE];			
#endif

extern INT  fsPhysicalDiskConnected(PDISK_T *pDisk);
extern int usiWriteEnable(void);
extern int usiCheckBusy(void);

INT spiFlashEraseSector4Backup(UINT32 addr, UINT32 secCount);


INT32 usiReadID_9F()
{

	UINT32 volatile id;

	outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) | 0x01);	// CS0

	// command 8 bit
	outpw(REG_SPI0_TX0, READ_ID_CMD);
	spiTxLen(0, 0, 8);
	spiActive(0);

	// data 24 bit
	outpw(REG_SPI0_TX0, 0xffff);
	spiTxLen(0, 0, 24);
	spiActive(0);
	id = inp32(REG_SPI0_RX0) & 0xFFFFFF;

	outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & 0xfe);	// CS0

	return id;
}

static INT  SpiFlash_disk_init(PDISK_T  *lDisk)
{
	return 0;
}


static INT  SpiFlash_disk_ioctl(PDISK_T *lDisk, INT control, VOID *param)
{
	return 0;
}

static INT  SpiFlash_disk_read(PDISK_T *pDisk, UINT32 sector_no, INT number_of_sector, UINT8 *buff)
{
	int status;	

	status = spiFlashRead(sector_no*SECTOR_NUM+spi_FAT_offset, number_of_sector*SECTOR_NUM, (UINT32 *)buff);

	if (status != Successful)
		return status;

	return FS_OK;
}

static INT  SpiFlash_disk_write(PDISK_T *pDisk, UINT32 sector_no, INT number_of_sector, UINT8 *buff, BOOL IsWait)
{
	int status;
	int SPI_addr, SPI_AligneSectorAddr;
	int SPI_writeSize,SPI_tmpSize,SPI_TargetSize,SPI_RemainingSize,SPI_MaxWritetSize4Sector;
	UINT32 size_mask;
	
	size_mask = ~(SPI_SECTOR_ERASE_SIZE-1);
	SPI_addr = sector_no*SECTOR_NUM+spi_FAT_offset;
	
	do {
		// Get the 4K alignment SPI flash address
		SPI_AligneSectorAddr = SPI_addr & size_mask;
		
		// Calculate the max available update SPI data size for current SPI address
		if (SPI_addr != SPI_AligneSectorAddr)
			SPI_MaxWritetSize4Sector = SPI_SECTOR_ERASE_SIZE - (SPI_addr - SPI_AligneSectorAddr);
		else	
			SPI_MaxWritetSize4Sector = SPI_SECTOR_ERASE_SIZE;
		
		// Get the real write data size in this sector
		if (number_of_sector * SECTOR_NUM >= SPI_MaxWritetSize4Sector)
			SPI_writeSize = SPI_MaxWritetSize4Sector;
		else 	
			SPI_writeSize = number_of_sector * SECTOR_NUM;
			
		// Read the original Sector content
		status = spiFlashRead(SPI_AligneSectorAddr, SPI_SECTOR_ERASE_SIZE, (UINT32 *)((UINT32)SPI_BackupBuf|0x80000000));
		if (status != Successful)
			return status;

		// Erase the spicified Sector before write
		status = spiFlashEraseSector4Backup(SPI_AligneSectorAddr,1 );	// Erase 1 sector size
		if (status != Successful)
		{
			sysprintf("Erase SPI sector fail !!\n");
			return status;	
		}			

		// Backup the data before Target address for backup
		SPI_tmpSize = 0;
		if (SPI_addr != SPI_AligneSectorAddr)
		{
			SPI_tmpSize = SPI_addr - SPI_AligneSectorAddr;
			status = spiFlashWrite(SPI_AligneSectorAddr, SPI_tmpSize, (UINT32 *)((UINT32)SPI_BackupBuf|0x80000000));
			
			if (status != Successful)
				return status;		
		}

		// Write the target SPI data	
		SPI_TargetSize = SPI_writeSize;
		status = spiFlashWrite(SPI_addr, SPI_TargetSize, (UINT32 *)buff);
		
		if (status != Successful)
			return status;

		SPI_addr = SPI_addr + SPI_TargetSize;	
		buff += SPI_TargetSize;	
			
		number_of_sector -= (SPI_TargetSize/512);
			
		// Write the remaining data after target data if the size is less than on sector
		if (SPI_tmpSize + SPI_TargetSize < SPI_SECTOR_ERASE_SIZE)
		{
			SPI_RemainingSize = SPI_SECTOR_ERASE_SIZE - SPI_writeSize - SPI_tmpSize;
			status = spiFlashWrite(SPI_addr, SPI_RemainingSize, (UINT32 *)((UINT32)&SPI_BackupBuf[SPI_writeSize + SPI_tmpSize]|0x80000000));
			
			if (status != Successful)
				return status;
		}	
		
	} while (number_of_sector >0);			

	return FS_OK;
}



STORAGE_DRIVER_T  _SPIDiskDriver = 
{
	SpiFlash_disk_init,
	SpiFlash_disk_read,
	SpiFlash_disk_write,
	SpiFlash_disk_ioctl
};

INT  SpiFlash_CardSel(void)
{
	return 0;		
}

INT  SPI_Read(UINT32 uSector, UINT32 uBufcnt, UINT32 uDAddr)
{
    INT status=0;

    status = SpiFlash_disk_read(NULL, uSector, uBufcnt, (UINT8 *)uDAddr);

    return status;
}

INT  SPI_Write(UINT32 uSector, UINT32 uBufcnt, UINT32 uSAddr)
{
    int status=0;

    status = SpiFlash_disk_write(NULL, uSector, uBufcnt, (UINT8 *)uSAddr, 0);

    return status;
}

// Added SPI list here to expand support list
INT MapFlashID(UINT32 spiID, DISK_DATA_T* pInfo)
{
	switch(spiID)
	{
		case 0x1C3018:
			strcpy(pInfo->vendor,"Eon Device");		//EN25Q128
			pInfo->totalSectorN = 16*1024*2;	// Unit : K (16MB)
			pInfo->diskSize = pInfo->totalSectorN/2 - (spi_FAT_offset / 1024);	
			break;
		case 0x1C7019:
			strcpy(pInfo->vendor,"Eon Device");		//EN25QH256
			pInfo->totalSectorN = 32*1024*2;	// Unit : K (32MB)
			pInfo->diskSize = pInfo->totalSectorN/2 - (spi_FAT_offset / 1024);	
			break;
		case 0xC84016:
			strcpy(pInfo->vendor,"Giga Device");	//GD25Q32B
			pInfo->totalSectorN = 4*1024*2;	// Unit : K (4MB)
			pInfo->diskSize = pInfo->totalSectorN/2 - (spi_FAT_offset / 1024);	
			break;
		case 0xC84017:
			strcpy(pInfo->vendor,"Giga Device");	//GD25Q64B
			pInfo->totalSectorN = 8*1024*2;	// Unit : K (8MB)
			pInfo->diskSize = pInfo->totalSectorN/2 - (spi_FAT_offset / 1024);
			break;
		case 0xC84018:
			strcpy(pInfo->vendor,"Giga Device");	//GD25Q128B
			pInfo->totalSectorN = 16*1024*2;	// Unit : K (16MB)
			pInfo->diskSize = pInfo->totalSectorN/2 - (spi_FAT_offset / 1024);
			break;	
		case 0xC22014:
			strcpy(pInfo->vendor,"Macronix");	//MX25L8006E
			pInfo->totalSectorN = 1*1024*2;	// Unit : K (1MB)
			pInfo->diskSize = pInfo->totalSectorN/2 - (spi_FAT_offset / 1024);
			break;		
		case 0xC22019:
			strcpy(pInfo->vendor,"Macronix");	//MX25L25635E
			pInfo->totalSectorN = 32*1024*2;	// Unit : K (32MB)
			pInfo->diskSize = pInfo->totalSectorN/2 - (spi_FAT_offset / 1024);
			break;
		case 0xEF3013:
			strcpy(pInfo->vendor,"Winbond");
			pInfo->totalSectorN = 1024;	// Unit : K (512KB)
			pInfo->diskSize = pInfo->totalSectorN/2 - (spi_FAT_offset / 1024);
			break;	
		case 0xEF4015:
			strcpy(pInfo->vendor,"Winbond");	//W25Q16CV
			pInfo->totalSectorN = 2*1024*2;	// Unit : K (2MB)
			pInfo->diskSize = pInfo->totalSectorN/2 - (spi_FAT_offset / 1024);
			break;	
		case 0xEF4016:
			strcpy(pInfo->vendor,"Winbond");	//W25Q32FV
			pInfo->totalSectorN = 4*1024*2;	// Unit : K (4MB)
			pInfo->diskSize = pInfo->totalSectorN/2 - (spi_FAT_offset / 1024);
			break;
		case 0xEF4018:
			strcpy(pInfo->vendor,"Winbond");	//W25Q128FV
			pInfo->totalSectorN = 16*1024*2;	// Unit : K (16MB)
			pInfo->diskSize = pInfo->totalSectorN/2 - (spi_FAT_offset / 1024);
			break;
		case 0xEF4019:
			strcpy(pInfo->vendor,"Winbond");	//W25Q256FV
			pInfo->totalSectorN = 32*1024*2;	// Unit : K (32MB)
			pInfo->diskSize = pInfo->totalSectorN/2 - (spi_FAT_offset / 1024);
			break;				
		default :
			sysprintf("Unknown SPI ID\n");
			return 0;
	}
	
	pInfo->totalSectorN = pInfo->totalSectorN - spi_FAT_offset/512;	
	sysprintf("Vendor = %s, Total sector = %d for Disk, Total size = %dK, FAT offset =%dK\n",pInfo->vendor, pInfo->totalSectorN, pInfo->diskSize,spi_FAT_offset/1024);
	
	return 0;
}

INT  SpiFlashInitDevice(void)   
{
	PDISK_T *pDisk;
	DISK_DATA_T* pSPIDisk;
	INT32 FlashID;	

	
	if(spi_ok == 1)
		return(0);
		
	pSPIDisk	= &SPI_DiskInfo0;		

	if (spiFlashInit() < 0)
		return SPI_INIT_TIMEOUT;
		
	/* 
	 * Create physical disk descriptor 
	 */
	pDisk = malloc(sizeof(PDISK_T));
	if (pDisk == NULL)
		return SPI_NO_MEMORY;
	memset((char *)pDisk, 0, sizeof(PDISK_T));
	
	FlashID = usiReadID_9F();
	MapFlashID(FlashID, pSPIDisk);

	if(pSPIDisk->totalSectorN > 16*1024*2)
		spiFlashEnter4ByteMode();

	/* read Disk information */
	pDisk->szManufacture[0] = '\0';
	strcpy(pDisk->szManufacture, pSPIDisk->vendor);
	strcpy(pDisk->szProduct, (char *)pSPIDisk->product);
	strcpy(pDisk->szSerialNo, (char *)pSPIDisk->serial);


	pDisk->nDiskType = DISK_TYPE_SD_MMC;

	pDisk->nPartitionN = 0;
	pDisk->ptPartList = NULL;
	
	pDisk->nSectorSize = SECTOR_NUM;
	pDisk->uTotalSectorN = pSPIDisk->totalSectorN;
	pDisk->uDiskSize = pSPIDisk->diskSize;

	pDisk->ptDriver = &_SPIDiskDriver;

	pDisk_SPI = pDisk;

	fsPhysicalDiskConnected(pDisk);

	spi_ok = 1;	
	
	return pDisk->uTotalSectorN;
} 



VOID SpiClose(void)
{
	spi_ok = 0;

	if (pDisk_SPI != NULL)
	{
		fsUnmountPhysicalDisk(pDisk_SPI);
		free(pDisk_SPI);
		pDisk_SPI = NULL;
	}
}


INT SpiFlashOpen(UINT32 FATOffset)
{
	spi_FAT_offset = FATOffset;
	return SpiFlashInitDevice();	
}


INT spiFlashEraseSector4Backup(UINT32 addr, UINT32 secCount)
{
	// Winbon W25Q128(16MB)
	// 4KB sector erase command : 0x20
	// 32KB Block erase command : 0x52
	// 64KB Block rease command : 0xD8
	int volatile i;

	if ((addr % (SPI_SECTOR_ERASE_SIZE)) != 0)
		return -1;

	for (i=0; i<secCount; i++)
	{
		usiWriteEnable();

		outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) | 0x01);	// CS0

		// erase command
		outpw(REG_SPI0_TX0, SPI_ERASE_CMD);
		spiTxLen(0, 0, 8);
		spiActive(0);

		// address
		if(g_u8Is4ByteMode == TRUE)
		{
			outpw(REG_SPI0_TX0, addr+i*SPI_SECTOR_ERASE_SIZE);
			spiTxLen(0, 0, 32);
			spiActive(0);
		}
		else
		{
			outpw(REG_SPI0_TX0, addr+i*SPI_SECTOR_ERASE_SIZE);
			spiTxLen(0, 0, 24);
			spiActive(0);
		}

		outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & 0xfe);	// CS0

		// check status
		usiCheckBusy();
	}

	return Successful;
}

	




