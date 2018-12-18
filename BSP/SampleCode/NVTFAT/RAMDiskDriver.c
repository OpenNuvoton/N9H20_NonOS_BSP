#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef W90N740
#include "supports.h"
#else
#include "N9H20.h"
#endif


UINT32 _RAMDiskBase;


static INT  ram_disk_init(PDISK_T *ptPDisk)
{
	return 0;
}


static INT  ram_disk_ioctl(PDISK_T *ptPDisk, INT control, VOID *param)
{
	return 0;
}


static INT  ram_disk_read(PDISK_T *ptPDisk, UINT32 uSecNo, 
								INT nSecCnt, UINT8 *pucBuff)
{
	memcpy(pucBuff, (UINT8 *)(_RAMDiskBase + uSecNo * 512), nSecCnt * 512);
	return FS_OK;
}


static INT  ram_disk_write(PDISK_T *ptPDisk, UINT32 uSecNo, 
								INT nSecCnt, UINT8 *pucBuff, BOOL bWait)
{
	memcpy((UINT8 *)(_RAMDiskBase + uSecNo * 512), pucBuff, nSecCnt * 512);
	return FS_OK;
}


STORAGE_DRIVER_T  _RAMDiskDriver = 
{
	ram_disk_init,
	ram_disk_read,
	ram_disk_write,
	ram_disk_ioctl,
};

static PDISK_T		*ptRAMDisk;
INT32 RemoveRAMDisk(void)
{
	fsPhysicalDiskDisconnected(ptRAMDisk);
	return 0;
}
INT32  InitRAMDisk(UINT32 uStartAddr, UINT32 uDiskSize)
{
	_RAMDiskBase = uStartAddr;
	ptRAMDisk = malloc(sizeof(PDISK_T));

	memset(ptRAMDisk, 0, sizeof(PDISK_T));
	memcpy(&ptRAMDisk->szManufacture,"NUVOTON ",sizeof("NUVOTON "));
	memcpy(&ptRAMDisk->szProduct,"RAMDISK",sizeof("RAMDISK"));
		
	ptRAMDisk->nDiskType = DISK_TYPE_HARD_DISK | DISK_TYPE_DMA_MODE;
	ptRAMDisk->uTotalSectorN = uDiskSize / 512;
	ptRAMDisk->nSectorSize = 512;
	ptRAMDisk->uDiskSize = uDiskSize/1024;
	ptRAMDisk->ptDriver = &_RAMDiskDriver;
	fsPhysicalDiskConnected(ptRAMDisk);
	return 0;
}

