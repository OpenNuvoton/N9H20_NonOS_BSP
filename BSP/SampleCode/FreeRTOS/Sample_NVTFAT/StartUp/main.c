/* Standard includes. */
#include <stdlib.h>

/* Nuvoton includes. */
#include "wblib.h"
#include "N9H20.h"
#include <inttypes.h>

#include "FreeRTOS.h"
#include "task.h"


#define DEF_WORKER_NUMBER	4
//#define STORAGE_SD

//#ifdef __GNUC__
#define printf sysprintf
//#endif

#ifdef STORAGE_SD

#define ENABLE_SD_ONE_PART
#define ENABLE_SD_0             
//#define ENABLE_SD_1
//#define ENABLE_SD_2
//#define ENABLE_SDIO_1

#define DEF_DISK_VOLUME_STR_SIZE	20
static char s_szDiskVolume[DEF_DISK_VOLUME_STR_SIZE] = {0x00};

UINT32 StorageForSD(void)
{
	uint32_t u32ExtFreqKHz;
	uint32_t u32PllOutKHz;
	uint32_t u32BlockSize, u32FreeSize, u32DiskSize;

	u32ExtFreqKHz = sysGetExternalClock();
	u32PllOutKHz = sysGetPLLOutputKhz(eSYS_UPLL, u32ExtFreqKHz);
	memset(s_szDiskVolume, 0x00, DEF_DISK_VOLUME_STR_SIZE);
	
#if defined(ENABLE_SD_ONE_PART)
	sicIoctl(SIC_SET_CLOCK, u32PllOutKHz, 0, 0);	
#if defined(ENABLE_SDIO_1)
	sdioOpen();	
	if (sdioSdOpen1()<=0)			//cause crash on Doorbell board SDIO slot. conflict with sensor pin
	{
		printf("Error in initializing SD card !! \n");						
		s_bFileSystemInitDone = TRUE;
		return -1;
	}	
#elif defined(ENABLE_SD_1)
	sicOpen();
	if (sicSdOpen1()<=0)
	{
		printf("Error in initializing SD card !! \n");						
		s_bFileSystemInitDone = TRUE;
		return -1;
	}
#elif defined(ENABLE_SD_2)
	sicOpen();
	if (sicSdOpen2()<=0)
	{
		printf("Error in initializing SD card !! \n");						
		s_bFileSystemInitDone = TRUE;
		return -1;
	}
#elif defined(ENABLE_SD_0)
	sicOpen();
	if (sicSdOpen0()<=0)
	{
		printf("Error in initializing SD card !! \n");						
		return -1;
	}
#endif
#endif			

	sprintf(s_szDiskVolume, "C");

#if defined(ENABLE_SD_ONE_PART)
	fsAssignDriveNumber(s_szDiskVolume[0], DISK_TYPE_SD_MMC, 0, 1);
#endif

	if(fsDiskFreeSpace(s_szDiskVolume[0], &u32BlockSize, &u32FreeSize, &u32DiskSize) != FS_OK){
		return -2;
	}

	printf("SD card block_size = %d\n", u32BlockSize);   
	printf("SD card free_size = %dkB\n", u32FreeSize);   
	printf("SD card disk_size = %dkB\n", u32DiskSize);   

	return 0;
}
#else
#define NAND_2      1   // comment to use 1 disk for NAND, uncomment to use 2 disk
UINT32 StorageForNAND(void);

static NDISK_T ptNDisk;
static NDRV_T _nandDiskDriver0 =
{
    nandInit0,
    nandpread0,
    nandpwrite0,
    nand_is_page_dirty0,
    nand_is_valid_block0,
    nand_ioctl,
    nand_block_erase0,
    nand_chip_erase0,
    0
};

#define NAND1_1_SIZE     32 /* MB unit */

UINT32 StorageForNAND(void)
{

    UINT32 block_size, free_size, disk_size;
    UINT32 u32TotalSize;

    fsAssignDriveNumber('C', DISK_TYPE_SMART_MEDIA, 0, 1);
#ifdef NAND_2
    fsAssignDriveNumber('D', DISK_TYPE_SMART_MEDIA, 0, 2);
#endif

    sicOpen();

    /* Initialize GNAND */
    if(GNAND_InitNAND(&_nandDiskDriver0, &ptNDisk, TRUE) < 0)
    {
        printf("GNAND_InitNAND error\n");
        goto halt;
    }

    if(GNAND_MountNandDisk(&ptNDisk) < 0)
    {
        printf("GNAND_MountNandDisk error\n");
        goto halt;
    }

    /* Get NAND disk information*/
    u32TotalSize = ptNDisk.nZone* ptNDisk.nLBPerZone*ptNDisk.nPagePerBlock*ptNDisk.nPageSize;
    printf("Total Disk Size %d\n", u32TotalSize);
    /* Format NAND if necessery */
#ifdef NAND_2
    if ((fsDiskFreeSpace('C', &block_size, &free_size, &disk_size) < 0) ||
            (fsDiskFreeSpace('D', &block_size, &free_size, &disk_size) < 0))
    {
        printf("unknow disk type, format device .....\n");
        if (fsTwoPartAndFormatAll((PDISK_T *)ptNDisk.pDisk, NAND1_1_SIZE*1024, (u32TotalSize- NAND1_1_SIZE*1024)) < 0)
        {
            printf("Format failed\n");
            goto halt;
        }
        fsSetVolumeLabel('C', "NAND1-1\n", strlen("NAND1-1"));
        fsSetVolumeLabel('D', "NAND1-2\n", strlen("NAND1-2"));
    }
#endif

halt:
    printf("systen exit\n");
    return 0;

}
#endif


#define DEF_BUF_SIZE 4096

static void worker_nvtfat ( void *pvArgs )
{
	int id = *((int*)pvArgs);
	int hFile = 0;
	int32_t i32Ret;
	int32_t num = 0;
	int i32Counter=0;
	int j;
	FILE_STAT_T sFileState;
	char szFileName[64];
	char szUnicodeFileName[128];
	char testBuf[DEF_BUF_SIZE];
		
	while(1)
	{
		int wsize = (rand()%DEF_BUF_SIZE)+1;

		// Open
		sprintf(szFileName, "C:\\%d_%d.txt", id, i32Counter);
		fsAsciiToUnicode(szFileName, szUnicodeFileName, TRUE);

		hFile = fsOpenFile(szUnicodeFileName, NULL, O_RDWR | O_CREATE);
		if (hFile < 0)
		{
			printf("Fail to open - %d.\n", id);
			goto exit_worker_nvtfat;
		}
		printf("opened %s.\n", szFileName);

		memset( testBuf, id, wsize );
		
		// Write
		num = 0;
		i32Ret = fsWriteFile(hFile, (UINT8 *)testBuf, wsize, (INT *)&num);
		if (i32Ret < 0 || (num != wsize) )
		{
			printf("Fail to write - %s %x.\n", szFileName, i32Ret);
			goto exit_worker_nvtfat;
		}
		printf("wrote %d to %s.\n", wsize, szFileName);

		// Close
		i32Ret = fsCloseFile(hFile);
		printf("closed %s. %x\n", szFileName, i32Ret);
	
		memset((void*)&sFileState, 0, sizeof(FILE_STAT_T) );
		
		// Stat
		if( ((i32Ret =fsGetFileStatus(-1, szUnicodeFileName, NULL, &sFileState)) < 0) ){
			printf("Fail to stat - %s %x\n", szFileName, i32Ret);
			goto exit_worker_nvtfat;
		}

		if ( sFileState.n64FileSize != wsize )
		{
			printf("wrong stat.size - %s %x\n", szFileName, i32Ret);
			goto exit_worker_nvtfat;
		}
//#ifdef __GNUC__
		printf("Size: %d%d bytes\n", (int)(sFileState.n64FileSize>>32), (int)sFileState.n64FileSize);
		printf("file name = %s\n",szFileName);
//#else
//		printf("Size: %" PRIu64 " bytes, %s\n", sFileState.n64FileSize, szFileName);
//#endif
		// Open
		hFile = fsOpenFile(szUnicodeFileName, NULL, O_RDWR);
		if (hFile < 0)
		{
			printf("Fail to open - %d.\n", id);
			goto exit_worker_nvtfat;
		}
		printf("opened %s.\n", szFileName);
		
		memset( testBuf, 0, wsize );
		
		// Read
		num = 0;
		i32Ret = fsReadFile(hFile, (void*)&testBuf[0], wsize, &num);
		if (i32Ret < 0 || (num != wsize))
		{
			printf("Fail to read - %s. %x\n", szFileName, i32Ret);
			goto exit_worker_nvtfat;
		}
		printf("readed %d from %s.\n", wsize, szFileName);

		// Verify
		for ( j=0; j<wsize; j++ )
			if ( (int)testBuf[j] != (id&0xff) )
			{
				printf("verify fail -> %d %d\n", id, testBuf[j]);
				goto exit_worker_nvtfat;
			}
		printf("Verified %s.\n", szFileName);

		// Close
		i32Ret = fsCloseFile(hFile);
		printf("closed %s %x.\n", szFileName, i32Ret);

		i32Counter++;
			
		// Delete
		i32Ret = fsDeleteFile(szUnicodeFileName, NULL);
		printf("Deleted %s. %x\n", szFileName, i32Ret);
	}

exit_worker_nvtfat:

	if (hFile)
		i32Ret = fsCloseFile(hFile);

	printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ Worker %d die. @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n", id );
	
	vTaskDelete(NULL);
}

static void prvSetupHardware(void)
{
    WB_UART_T uart;
    UINT32 u32ExtFreq;

    u32ExtFreq = sysGetExternalClock();
    sysUartPort(1);
    uart.uart_no = WB_UART_1;
    uart.uiFreq = u32ExtFreq*1000;   //use APB clock
    uart.uiBaudrate = 115200;
    uart.uiDataBits = WB_DATA_BITS_8;
    uart.uiStopBits = WB_STOP_BITS_1;
    uart.uiParity = WB_PARITY_NONE;
    uart.uiRxTriggerLevel = LEVEL_1_BYTE;
    sysInitializeUART(&uart);

    sysEnableCache(CACHE_WRITE_BACK);

    printf("fsInitFileSystem.\n");
	fsInitFileSystem();
	
#ifdef STORAGE_SD
	printf("\nstart the SD NVTFAT FreeRTOS demo\n");
	StorageForSD();
#else
	printf("\nstart the NAND¡@NVTFAT FreeRTOS demo\n");
    StorageForNAND();
#endif

}

int main(void)
{
	int i=0;

	char szBuf[configMAX_TASK_NAME_LEN];
	TaskHandle_t  pxID_worker_nvtfat[DEF_WORKER_NUMBER];

	int i32Argv[DEF_WORKER_NUMBER];
		
	prvSetupHardware();

	for (i=0; i<DEF_WORKER_NUMBER; i++)
	{
		//pthread_create( &pxID_worker_nvtfat[i], &nvtfat_attr[i], worker_nvtfat, (void*)&i32Argv[i] );
	    /* Create the task, storing the handle. */
		i32Argv[i]=i+1;
		snprintf(szBuf, sizeof(szBuf), "worker-%d", i32Argv[i]);
        xTaskCreate( worker_nvtfat,        /* Function that implements the task. */
                 szBuf,     					 /* Text name for the task. */
                 8*1024,     					 /* Stack size in words, not bytes. */
                 (void*)&i32Argv[i],   /* Parameter passed into the task. */
                 configMAX_PRIORITIES-1,	/* Priority at which the task is created. */
				 //1,
                 &pxID_worker_nvtfat[i] );      		/* Used to pass out the created task's handle. */
	}
	
	vTaskStartScheduler();
	for(;;);
	
	fsCloseFileSystem();

#ifndef STORAGE_SD
	GNAND_UnMountNandDisk(&ptNDisk);
	sicClose();
#endif
	
	return 0;
}

/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	printf("%s\n", __func__);

	taskDISABLE_INTERRUPTS();
	for( ;; );
}

void vApplicationMallocFailedHook( void ){
	printf("%s\n", __func__);
}

// We need this when configSUPPORT_STATIC_ALLOCATION is enabled
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize ) {

// This is the static memory (TCB and stack) for the idle task
static StaticTask_t xIdleTaskTCB; // __attribute__ ((section (".rtos_heap")));
static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE]; // __attribute__ ((section (".rtos_heap"))) __attribute__((aligned (8)));


    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/*************************** End of file ****************************/
