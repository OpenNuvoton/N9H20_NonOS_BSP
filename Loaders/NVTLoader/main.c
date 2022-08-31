/**************************************************************************//**
 * @file     main.c
 * @brief    Load conprog.bin code from SD or NAND device for next booting stage
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "N9H20.h"
#include "nvtloader.h"
#ifdef USB_HOST
#include "usb.h"
#endif


#ifdef __MASS_PARODUCT__	
void	EMU_MassProduction(void);
#endif

extern void loadKernelCont(int fd, int offset);
extern void playAni(int fd, char* pcString);
//extern void mass(NDISK_T *disk);
void mass(NDISK_T *disk, INT32 i32TotalSector);
extern void lcmFill2Dark(unsigned char*);
extern void initVPostShowLogo(void);

#if defined(__GNUC__)
unsigned char kbuf[CP_SIZE] __attribute__((aligned (32))); /* save first 16k of buffer. Copy to 0 after vector table is no longer needed */
UINT8 dummy_buffer[512] __attribute__((aligned (32)));
#else
__align(32) unsigned char kbuf[CP_SIZE];
__align(32) UINT8 dummy_buffer[512];
#endif



extern void AudioChanelControl(void);
extern void backLightEnable(void);
int kfd, mfd;
BOOL bIsIceMode = FALSE;	
	
int g_ibr_boot_sd_port=-1;     // indicate the SD port number which IBR boot from.		

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


static NDISK_T ptNDisk;
extern UINT16 u16Volume;
/* Volume config file locate in NAND disk */ 
void VolumeConfigFile(BOOL bIsFromNAND)
{
	INT8 path[64];
	/* Check if volume config file exists */
	if(bIsFromNAND==0)
		fsAsciiToUnicode(VOLUME_PATH, path, TRUE);
	else
		fsAsciiToUnicode(VOLUME_PATH_SD, path, TRUE);	
		
	kfd = fsOpenFile(path, 0, O_RDONLY);
	if(kfd > 0)
	{
		INT32		nStatus, nLen;		
		UINT8  		u8VolCfg[2];		
		nStatus = fsReadFile(kfd, u8VolCfg, 2, &nLen);
		if(nStatus>=0)
		{
			u16Volume = u8VolCfg[0]|(((UINT16)u8VolCfg[1])<<8); 
			u16Volume = u16Volume * 63/100;
			sysprintf("Volume = %d\n", u16Volume);	 
		}	
	}
}

BOOL udcDetection(void)
{  
	UINT32 volatile u32Delay = 0x100000;
	BOOL bIsUsbDet;	
	outp32(REG_AHBCLK, inp32(REG_AHBCLK) | USBD_CKE);
	outpw(REG_AHBIPRST, inpw(REG_AHBIPRST) | UDCRST);
	outpw(REG_AHBIPRST, inpw(REG_AHBIPRST) & ~UDCRST);	
	outp32(PHY_CTL, (0x20 | Phy_suspend));
	
	outp32(OPER, 0x0);		
	while(inp32(OPER) != 0x0);	
	while(u32Delay--);
	if(inp32(PHY_CTL) & Vbus_status) 
		bIsUsbDet = TRUE;
	else
		bIsUsbDet = FALSE;	
	outp32(REG_AHBCLK, inp32(REG_AHBCLK) & ~USBD_CKE);
	outpw(REG_AHBIPRST, inpw(REG_AHBIPRST) | UDCRST);
	outpw(REG_AHBIPRST, inpw(REG_AHBIPRST) & ~UDCRST);		
	return bIsUsbDet;
}

UINT32 u32TimerChannel = 0;
void Timer0_300msCallback(void)
{
#ifdef __BACKLIGHT__
	backLightEnable();
#endif	
	sysClearTimerEvent(TIMER0, u32TimerChannel);
}
void init(void)
{
	WB_UART_T 	uart;
	UINT32 		u32ExtFreq;	    	    	
	UINT32 u32Cke = inp32(REG_AHBCLK);
	
	/* Reset SIC engine to fix USB update kernel and mvoie file */
	outp32(REG_AHBCLK, u32Cke  | (SIC_CKE | NAND_CKE | SD_CKE)); 
	outp32(REG_AHBIPRST, inp32(REG_AHBIPRST )|SICRST ); 
	outp32(REG_AHBIPRST, 0); 
	
	outp32(REG_APBIPRST, TMR0RST | TMR1RST); 
	outp32(REG_APBIPRST, 0); 
	
	outp32(REG_AHBCLK,u32Cke);	
	sysEnableCache(CACHE_WRITE_BACK);
	
	/* init timer */	
	u32ExtFreq = sysGetExternalClock();    	/* KHz unit */	
	sysSetTimerReferenceClock (TIMER0, 
								u32ExtFreq*1000);	/* Hz unit */
	sysStartTimer(TIMER0, 
					100, 
					PERIODIC_MODE);
  u32TimerChannel = sysSetTimerEvent(TIMER0, 
    									30,
    									(PVOID)Timer0_300msCallback);
    
  /* enable UART */
  sysUartPort(1);
	uart.uiFreq = u32ExtFreq*1000;					/* Hz unit */	
	uart.uiBaudrate = 115200;
	uart.uiDataBits = WB_DATA_BITS_8;
	uart.uiStopBits = WB_STOP_BITS_1;
	uart.uiParity = WB_PARITY_NONE;
	uart.uiRxTriggerLevel = LEVEL_1_BYTE;
	sysInitializeUART(&uart);    
	sysprintf("NVT Loader start\n");
#ifndef __CC__
    kpi_init();
	kpi_open(3); // use nIRQ0 as external interrupt source
	bIsIceMode = kpi_read(KPI_NONBLOCK);
	if(bIsIceMode!=FALSE)
		bIsIceMode=TRUE;
#endif 	
	sysSetLocalInterrupt(ENABLE_IRQ);
			
}


typedef struct sd_info
{
	unsigned int startSector;
	unsigned int endSector;
	unsigned int fileLen;
	unsigned int executeAddr;
}NVT_SD_INFO_T;

unsigned char *buf;
unsigned int *pImageList;

/* read image information */
UINT32 ParsingReservedArea(void)
{
	UINT32 u32ReservedSector=0;
	NVT_SD_INFO_T image;
	int count, i;
	
	buf = (UINT8 *)((UINT32)dummy_buffer | 0x80000000);
	pImageList=((unsigned int *)(((unsigned int)dummy_buffer)|0x80000000));
#if 1
	if (g_ibr_boot_sd_port == 0){
		sicSdRead0(33, 1, (UINT32)dummy_buffer);	
	}else if (g_ibr_boot_sd_port == 1){
		sicSdRead1(33, 1, (UINT32)dummy_buffer);	
	}else if (g_ibr_boot_sd_port == 2){
		sicSdRead2(33, 1, (UINT32)dummy_buffer);	
	}
#else	
#ifdef __ENABLE_SD_CARD_0__
	sicSdRead0(33, 1, (UINT32)dummy_buffer);	
#endif	
#ifdef __ENABLE_SD_CARD_1__
	sicSdRead1(33, 1, (UINT32)dummy_buffer);	
#endif	
#endif
	pImageList=((unsigned int *)(((unsigned int)dummy_buffer)|0x80000000));
	if( (*(pImageList+2)) !=0xFFFFFFFF)
	{
		sysprintf("Turbo writter reserved area %dKB\n",u32ReservedSector/2); 
		u32ReservedSector = (*(pImageList+2));
	}	
	if (((*(pImageList+0)) == 0x574255aa) && ((*(pImageList+3)) == 0x57425963))
	{
		count = *(pImageList+1);

		pImageList = pImageList+4;
		for (i=0; i<count; i++)
		{
			if (((*(pImageList) >> 16) & 0xffff) < 4)
			{
				image.startSector = *(pImageList + 1) & 0xffff;
				image.endSector = (*(pImageList + 1) & 0xffff0000) >> 16;
				if(image.endSector>u32ReservedSector)
					u32ReservedSector = image.endSector;
				image.executeAddr = *(pImageList + 2);
				image.fileLen = *(pImageList + 3);			
			}
			/* pointer to next image */
			pImageList = pImageList+12;
		}
	}
	return (u32ReservedSector+1);
}	

UINT32 NVT_LoadKernelFromSD(void)
{
	INT8 path[64];
	INT32 i32ErrorCode;
	INT found_kernel = 0;
	INT found_avi = 0;
	PDISK_T		*pDiskList;
	UINT32 block_size, free_size, disk_size; 
	UINT32 u32TotalSize;
	INT32 i32BootSDTotalSector;	
	void	(*_jump)(void);
	
	DBG_PRINTF("Loader will load conprog.bin in SD card.\n");
	fsAssignDriveNumber('X', DISK_TYPE_SD_MMC, 0, 1); 
	fsAssignDriveNumber('Y', DISK_TYPE_SD_MMC, 0, 2);
	     
	
	/*-----------------------------------------------------------------------*/
	/*  Init SD card                                                         */
	/*-----------------------------------------------------------------------*/
	sicOpen();
	sicIoctl(SIC_SET_CLOCK, 192000, 0, 0);
	
#if 1	
	if (g_ibr_boot_sd_port == 0){
		sysprintf("Load code from SD0\n");
		i32BootSDTotalSector = sicSdOpen0();	/* Total sector or error code */
		if(i32BootSDTotalSector < 0){
	        	sicSdClose0();
	        	exit(1);
	        }	
		sysprintf("total SD0 sectors number (%x)\n", i32BootSDTotalSector);
	}
	else if (g_ibr_boot_sd_port == 1){
		sysprintf("Load code from SD1\n");
		i32BootSDTotalSector = sicSdOpen1();	/* Total sector or error code */
		if(i32BootSDTotalSector < 0){
	   	     sicSdClose1();
	   	     exit(1);
	   	}
		sysprintf("total SD1 sectors (%x)\n", i32BootSDTotalSector);
	}
	else if (g_ibr_boot_sd_port == 2){
		sysprintf("Load code from SD2\n");
		i32BootSDTotalSector = sicSdOpen2();	/* Total sector or error code */
		if(i32BootSDTotalSector < 0){
	  	      sicSdClose2();
	  	      exit(1);
	  	} 
		sysprintf("total SD2 sectors (%x)\n", i32BootSDTotalSector);
	}
#else		
#ifdef __ENABLE_SD_CARD_0__ 	
	sysprintf("Load code from SD 0\n");	
	i32ErrorCode = sicSdOpen0();	/* Total sector or error code */
	sysprintf("total SD 0 sectors (%x)\n", sicSdOpen0());	
#endif 	
#ifdef __ENABLE_SD_CARD_1__ 	
	sysprintf("Load code from SD 1\n");	
	i32ErrorCode = sicSdOpen1();	/* Total sector or error code */
	sysprintf("total SD 1 sectors (%x)\n", sicSdOpen1());	
#endif
#endif	
	/* In here for USB VBus stable. Othwise, USB library can not judge VBus correct  */ 
	sysprintf("UDC open\n");
	udcOpen();	 
	
	/*? DBG_PRINTF("total SD sectors (%x)\n", sicSdOpen());	?*/
	
	/* Get SD disk information*/ 
	pDiskList = fsGetFullDiskInfomation();
	sysprintf("Total Disk Size = %dMB\n", pDiskList->uDiskSize/1024);
	/* Format NAND if necessery */
	if ((fsDiskFreeSpace('X', &block_size, &free_size, &disk_size) < 0) || 
	    (fsDiskFreeSpace('Y', &block_size, &free_size, &disk_size) < 0))
	{   
		UINT32 u32Reserved;
		u32Reserved = ParsingReservedArea();
		sysprintf("unknow disk type, format device ...Reserved Area= %dKB \n", u32Reserved/2);
		fsSetReservedArea(u32Reserved);	

	#if 1
		u32TotalSize = (i32ErrorCode-u32Reserved)*512; 
	#endif 	
				
    	if (fsTwoPartAndFormatAll((PDISK_T *)pDiskList->ptSelf, SD1_1_SIZE*1024, (u32TotalSize- SD1_1_SIZE*1024)) < 0) 
    	{
			sysprintf("Format failed\n");					
			goto sd_halt;
		}
    	fsSetVolumeLabel('X', "SD1-1\n", strlen("SD1-1"));
		fsSetVolumeLabel('Y', "SD1-2\n", strlen("SD1-2"));	
	}			
	/* Read volume config file */ 
	VolumeConfigFile(1);		/* Read volume config file from SD */
	/* Detect USB */ 
	if(udcIsAttached())
	{
		//for mass's issue. sicSdClose();
		sysprintf("Detect USB plug in\n");		
		//mass(&ptNDisk, i32ErrorCode);				/* ptNDisk is useless for SD mass-storage*/
	  mass(&ptNDisk, i32BootSDTotalSector);					/* ptNDisk is useless for SD mass-storage*/
		sysprintf("USB plug out\n");
	}				
#ifdef __WIFI_NO_LCM__	 /* Make sure ADO, VPOST and SPU clock has been turn off */	
	outp32(REG_AHBCLK, inp32(REG_AHBCLK) & ~(ADO_CKE|VPOST_CKE|SPU_CKE));
#else		
	// Check if movie & Kernel exists.
	fsAsciiToUnicode(MOVIE_PATH_SD, path, TRUE);
	mfd = fsOpenFile(path, 0, O_RDONLY);
	if(mfd > 0) 
	{
		found_avi = 1;
		fsCloseFile(mfd);
		sysprintf("animation file found\n");
	}
#endif	
	fsAsciiToUnicode(KERNEL_PATH_SD, path, TRUE);
	kfd = fsOpenFile(path, 0, O_RDONLY);
	if(kfd > 0) 
	{			
		found_kernel = 1;
		sysprintf("kernel found\n");
	}	
	
	#if defined(__IXL_WINTEK__) || defined(__GWMT9360A__) || defined(__GWMT9615A__)	
	AudioChanelControl();
	#endif

#ifdef __WIFI_NO_LCM__	

#else	
	/* Initial SPU in advance for linux set volume issue */ 
#if defined(N9H20K3) || defined(N9H20K5)		
	spuOpen(eDRVSPU_FREQ_8000);
	//spuDacOn(1);
	spuIoctl(SPU_IOCTL_SET_VOLUME, u16Volume, u16Volume);
#endif
#endif

#ifdef __BAT_DET__
	BatteryDetection(FALSE);
#endif		
	if(found_avi) 
	{ 
#ifdef __WIFI_NO_LCM__	

#else						
  #if defined(N9H20K3) || defined(N9H20K5)	
		char ucSring[64]= MOVIE_PATH_SD;
		playAni(kfd, ucSring);
  #endif	
#endif	
	} 
	else 
	{
		if(found_kernel)				
			loadKernelCont(kfd, 0);
	}
	
	if(kfd > 0)
	{	
		unsigned int i = 0;
		fsCloseFile(kfd);	//Close kernel file 
#if 1
	        if (g_ibr_boot_sd_port == 0)
	        	sicSdClose0();
	        else if (g_ibr_boot_sd_port == 1)
	        	sicSdClose1();
	        else if (g_ibr_boot_sd_port == 2)
	        	sicSdClose2();
		sicClose();
#else				
#ifdef __ENABLE_SD_CARD_0__ 		
		sicSdClose0();
#endif
#ifdef __ENABLE_SD_CARD_1__ 		
		sicSdClose1();
#endif		
		sicClose();
#endif
		sysSetGlobalInterrupt(DISABLE_ALL_INTERRUPTS);
		sysSetLocalInterrupt(DISABLE_FIQ_IRQ);		
		// Invalid and disable cache
		sysDisableCache();
		sysInvalidCache();
				
		memcpy((unsigned char *)i, kbuf, CP_SIZE);
		
		// JUMP to kernel
		sysprintf("Jump to kernel\n");
		//lcmFill2Dark((char *)(FB_ADDR | 0x80000000));	
		outp32(REG_AHBIPRST, JPGRST | SICRST |UDCRST | EDMARST);
		outp32(REG_AHBIPRST, 0);
		outp32(REG_APBIPRST, UART1RST | UART0RST | TMR1RST | TMR0RST );
		outp32(REG_APBIPRST, 0);
		
		sysFlushCache(I_D_CACHE);	   	
		_jump = (void(*)(void))(0x0); // Jump to 0x0 and execute kernel
		_jump();	
		
		while(1);
		//return(0); // avoid compilation warning
	}
	else
	{
		DBG_PRINTF("Cannot find conprog.bin in SD card.(err=0x%x)\n", kfd);
		DBG_PRINTF("Try load conprog.bin in NAND\n\n");
	}
	return Successful;
sd_halt:	
	sysprintf("systen exit\n");
	while(1); // never return		
}	

UINT32 NVT_LoadKernelFromNAND(void)
{
	INT found_kernel = 0;
	INT found_avi = 0;
	UINT32 block_size, free_size, disk_size; 
	UINT32 u32TotalSize;
	void	(*_jump)(void);
	INT8 path[64];

    fsAssignDriveNumber('C', DISK_TYPE_SMART_MEDIA, 0, 1);
    fsAssignDriveNumber('D', DISK_TYPE_SMART_MEDIA, 0, 2);                      
	     	
	
   	/* For detect VBUS stable */ 
   	sicOpen();
    sicIoctl(SIC_SET_CLOCK, 192000, 0, 0);  /* clock from PLL */   	
	
	/* In here for USB VBus stable. Othwise, USB library can not judge VBus correct  */ 	
#if 1
	udcOpen();	 
#endif	  	
	/* Initialize GNAND */
	if(GNAND_InitNAND(&_nandDiskDriver0, &ptNDisk, TRUE) < 0) 
	{
		sysprintf("GNAND_InitNAND error\n");
		goto halt;
	}	
	
	if(GNAND_MountNandDisk(&ptNDisk) < 0) 
	{
		sysprintf("GNAND_MountNandDisk error\n");
		goto halt;		
	}
	
	/* Get NAND disk information*/ 
	u32TotalSize = ptNDisk.nZone* ptNDisk.nLBPerZone*ptNDisk.nPagePerBlock*ptNDisk.nPageSize;
	sysprintf("Total Disk Size %d\n", u32TotalSize);	
	/* Format NAND if necessery */
	if ((fsDiskFreeSpace('C', &block_size, &free_size, &disk_size) < 0) || 
	    (fsDiskFreeSpace('D', &block_size, &free_size, &disk_size) < 0)) 			    
	    	{   
	    		sysprintf("unknow disk type, format device .....\n");	
		    	if (fsTwoPartAndFormatAll((PDISK_T *)ptNDisk.pDisk, NAND1_1_SIZE*1024, (u32TotalSize- NAND1_1_SIZE*1024)) < 0) {
					sysprintf("Format failed\n");					
				goto halt;
	    	}
	    	fsSetVolumeLabel('C', "NAND1-1\n", strlen("NAND1-1"));
			fsSetVolumeLabel('D', "NAND1-2\n", strlen("NAND1-2"));	
	}
	
	/* Detect USB */ 
	if(udcIsAttached())
	{
		sysprintf("USB plug in\n");		
		mass(&ptNDisk, FMI_NO_SD_CARD);	/* Total sector in ptNDisk. parameter 2 is useless for NAND*/
		sysprintf("USB plug out\n");
	}				
	/* Read volume config file to same as linux kernel */ 
	VolumeConfigFile(0);			/* Read volume config file from NAND */
	
#ifdef __WIFI_NO_LCM__	 /* Make sure ADO, VPOST and SPU clock has been turn off */	
	outp32(REG_AHBCLK, inp32(REG_AHBCLK) & ~(ADO_CKE|VPOST_CKE|SPU_CKE));
#else
	/* Check if movie & Kernel exists */
	sysprintf("%s\n",MOVIE_PATH);
	fsAsciiToUnicode(MOVIE_PATH, path, TRUE);
	kfd = fsOpenFile(path, 0, O_RDONLY);
	if(kfd > 0) 
	{
		found_avi = 1;
		fsCloseFile(kfd);
		sysprintf("animation file found\n");
	}
#endif	
	fsAsciiToUnicode(KERNEL_PATH, path, TRUE);
	kfd = fsOpenFile(path, 0, O_RDONLY);
	if(kfd > 0) 
	{			
		found_kernel = 1;
		sysprintf("kernel found\n");
	}	
	
	#if defined(__IXL_WINTEK__) || defined(__GWMT9360A__) || defined(__GWMT9615A__)
	AudioChanelControl();
	#endif	
#ifdef __WIFI_NO_LCM__	

#else	
	/* Initial SPU in advance for linux set volume issue */ 	
	spuOpen(eDRVSPU_FREQ_8000);
	//spuDacOn(1);
	spuIoctl(SPU_IOCTL_SET_VOLUME, u16Volume, u16Volume);	
#endif		
#ifdef __BAT_DET__
	BatteryDetection(FALSE);
#endif	
	if(found_avi) 
	{ 
#ifdef __WIFI_NO_LCM__	

#else	
#if defined(N9H20K3) || defined(N9H20K5)
		char ucSring[64]= MOVIE_PATH;				
		playAni(kfd, ucSring);
#endif		
#endif	
	} 
	else 
	{
		if(found_kernel)				
			loadKernelCont(kfd, 0);
	}

	if(found_kernel) 
	{
		unsigned int i = 0;
		GNAND_UnMountNandDisk(&ptNDisk);
       	fmiSMClose(0);		
#ifdef __ENABLE_SD_CARD_0__		
		sicSdClose0();
#endif
#ifdef __ENABLE_SD_CARD_1__		
		sicSdClose1();
#endif					
		sicClose();
		/* Disable interrupt */
		sysSetGlobalInterrupt(DISABLE_ALL_INTERRUPTS);
		sysSetLocalInterrupt(DISABLE_FIQ_IRQ);		
		/* Invalid and disable cache */
		sysDisableCache();
		sysInvalidCache();
		
		/* Reset IPs */	
		sysprintf("Jump to kernel\n");
		//outp32(REG_AHBIPRST, JPGRST | SICRST |UDCRST );		
		outp32(REG_AHBIPRST, JPGRST | SICRST |UDCRST | EDMARST);
		outp32(REG_AHBIPRST, 0);
		outp32(REG_APBIPRST, UART1RST | UART0RST | TMR1RST | TMR0RST );
		outp32(REG_APBIPRST, 0);				
			
		memcpy((unsigned char *)i, kbuf, CP_SIZE);
    	sysFlushCache(I_D_CACHE);	   
     
		_jump = (void(*)(void))(0x0); /* Jump to 0x0 and execute kernel */
		_jump();	
	} 
	else 
	{
		sysprintf("Cannot find conprog.bin");
	}	
halt:
	sysprintf("systen exit\n");
	while(1); // never return
}

extern DISK_DATA_T SPI_DiskInfo0;

void delay(UINT32 u32Tick)
{
	UINT32 btime, etime; 
	btime = sysGetTicks(TIMER0);
	while (1)
	{			
		etime = sysGetTicks(TIMER0);
		if ((etime - btime) >= u32Tick)
		{
			break; 
		}
	}
}	

UINT32 ParsingSPI_ReservedArea(void)
{
		UINT32 u32Buf[16];
	  UINT32 i, addr; 
	  UINT32* u32Ptr; 
	
	  if (spiFlashInit() < 0)
			return 1024*1024;
		for(i=0; i<128; i=i+1)
		{
			addr = i*512; 	
			spiFlashRead(addr, 16, u32Buf);
			u32Ptr = (UINT32*)u32Buf;
			//sysprintf("u32Addr = 0x%x, Data = 0x%x\n", addr, *u32Ptr);
			if((*u32Ptr == 0xAA554257))
			{
				 sysprintf("u32Addr = 0x%x, Data = 0x%x\n", addr, *u32Ptr);
				 sysprintf("Data = 0x%x\n", *(u32Ptr+1));
				 sysprintf("Data = 0x%x\n", *(u32Ptr+2));
				 sysprintf("Data = 0x%x\n", *(u32Ptr+3));
				 //addr = *(u32Ptr+2)*512;  /* ==> Sector offset to byte offset */
			   //return addr;
				 return *(u32Ptr+2); 
			}
		}
}


UINT32 NVT_LoadKernelFromSPI(void)
{
    /* For detect VBUS stable */ 
	
    UINT32 block_size, free_size, disk_size, reserved_size;
    INT32 i32BootSDTotalSector;

		INT found_kernel = 0;
		INT found_avi = 0;
		void	(*_jump)(void);
		INT8 path[64];


    fsInitFileSystem();
		fsAssignDriveNumber('C', DISK_TYPE_SD_MMC, 0, 1);
#if 1
		reserved_size = ParsingSPI_ReservedArea();
	  if( reserved_size == 0xFFFFFFFF )
				reserved_size = 1024*1024;
		else
				reserved_size *= 512;
#else	
	  reserved_size = 1024*1024;        /* SPI reserved size before FAT = 1M */
#endif		
		i32BootSDTotalSector = SpiFlashOpen(reserved_size);
		sysprintf("SPI Total SPI sector size = 0x%x\n", i32BootSDTotalSector);

	/* In here for USB VBus stable. Otherwise, USB library can not judge VBus correct  */ 	
#if 1
	   udcOpen();	 
#endif	  	

	/* Get SPI disk information*/
	if (fsDiskFreeSpace('C', &block_size, &free_size, &disk_size) < 0)  
	{
			UINT32 u32BlockSize, u32FreeSize, u32DiskSize;
			PDISK_T  *pDiskList;

			//printf("Total SPI size = %d KB\n", u32TotalSectorSize/2);

			fsSetReservedArea(reserved_size/512);
			pDiskList = fsGetFullDiskInfomation();
			fsFormatFlashMemoryCard(pDiskList);

	    fsSetVolumeLabel('C', "SPI1-1\n", strlen("SPI1-1"));
		
			fsReleaseDiskInformation(pDiskList);
			fsDiskFreeSpace('C', &u32BlockSize, &u32FreeSize, &u32DiskSize);   
			sysprintf("block_size = %d\n", u32BlockSize);
			sysprintf("free_size = %d\n", u32FreeSize);
			sysprintf("disk_size = %d\n", u32DiskSize);
	}
	sysprintf("block_size = %d\n", block_size);
	sysprintf("free_size = %d\n", free_size);
	sysprintf("disk_size = %d\n", disk_size);
	
	/* Detect USB */
	delay(8); 
	if(udcIsAttached())
	{
		sysprintf("USB plug in\n");		
		mass(NULL, i32BootSDTotalSector);	
		sysprintf("USB plug out\n");
	}				
	/* Read volume config file to same as linux kernel */ 
	VolumeConfigFile(0);			/* Read volume config file from NAND */
	
#ifdef __WIFI_NO_LCM__	 /* Make sure ADO, VPOST and SPU clock has been turn off */	
	outp32(REG_AHBCLK, inp32(REG_AHBCLK) & ~(ADO_CKE|VPOST_CKE|SPU_CKE));
#else
	/* Check if movie & Kernel exists */
	sysprintf("%s\n",MOVIE_PATH);
	fsAsciiToUnicode(MOVIE_PATH, path, TRUE);
	kfd = fsOpenFile(path, 0, O_RDONLY);
	if(kfd > 0) 
	{
		found_avi = 1;
		fsCloseFile(kfd);
		sysprintf("animation file found\n");
	}
#endif	
	fsAsciiToUnicode(KERNEL_PATH, path, TRUE);
	sysprintf("Find Kernel\n"); 
	kfd = fsOpenFile(path, 0, O_RDONLY);
	if(kfd > 0) 
	{			
		found_kernel = 1;
		sysprintf("kernel found\n");
	}	
	
	#if defined(__IXL_WINTEK__) || defined(__GWMT9360A__) || defined(__GWMT9615A__)
	AudioChanelControl();
	#endif	
#ifdef __WIFI_NO_LCM__	

#else	
	/* Initial SPU in advance for linux set volume issue */ 	
	spuOpen(eDRVSPU_FREQ_8000);
	//spuDacOn(1);
	spuIoctl(SPU_IOCTL_SET_VOLUME, u16Volume, u16Volume);	
#endif		
#ifdef __BAT_DET__
	BatteryDetection(FALSE);
#endif	
	if(found_avi) 
	{ 
#ifdef __WIFI_NO_LCM__	

#else	
#if defined(N9H20K3) || defined(N9H20K5)
		char ucSring[64]= MOVIE_PATH;				
		playAni(kfd, ucSring);
#endif		
#endif	
	} 
	else 
	{
		if(found_kernel)				
			loadKernelCont(kfd, 0);
	}

	if(found_kernel) 
	{
		unsigned int i = 0;
		
		/* Disable interrupt */
		sysSetGlobalInterrupt(DISABLE_ALL_INTERRUPTS);
		sysSetLocalInterrupt(DISABLE_FIQ_IRQ);		
		/* Invalid and disable cache */
		sysDisableCache();
		sysInvalidCache();
		
		/* Reset IPs */	
		sysprintf("Jump to kernel\n");
		//outp32(REG_AHBIPRST, JPGRST | SICRST |UDCRST );		
		outp32(REG_AHBIPRST, JPGRST | SICRST |UDCRST | EDMARST);
		outp32(REG_AHBIPRST, 0);
		outp32(REG_APBIPRST, UART1RST | UART0RST | TMR1RST | TMR0RST | SPI0RST | SPI1RST);
		outp32(REG_APBIPRST, 0);				
			
		memcpy((unsigned char *)i, kbuf, CP_SIZE);
    sysFlushCache(I_D_CACHE);	   
     
		_jump = (void(*)(void))(0x0); /* Jump to 0x0 and execute kernel */
		_jump();	
	} 
	else 
	{
		sysprintf("Cannot find conprog.bin");
	}	
//halt:
	sysprintf("systen exit\n");
	while(1); // never return
}

int main(void)
{
	// IBR and SDLoader keep the booting SD port number on register SDCR.
	// NVTLoader should load image from same SD port.
#if defined(__ENABLE_SD_CARD_0__)||defined(__ENABLE_SD_CARD_1__)||defined(__ENABLE_SD_CARD_2__)		
	outpw(REG_AHBCLK, inpw(REG_AHBCLK) | SD_CKE);   // SDLoader disable SIC/SD clock before jump to NVTLoader.
	outpw(REG_AHBCLK, inpw(REG_AHBCLK) | SIC_CKE);  // Now, enable clock to read SD register.
	g_ibr_boot_sd_port = (inpw(REG_SDCR) & SDCR_SDPORT) >> 29;
	outpw(REG_AHBCLK, inpw(REG_AHBCLK) & ~SD_CKE);
	outpw(REG_AHBCLK, inpw(REG_AHBCLK) & ~SIC_CKE);
#endif	
	
	sysDisableCache(); 	
	sysFlushCache(I_D_CACHE);		
#ifdef __BAT_DET__
	adc_init();
	adc_open(ADC_TS_4WIRE, 320, 240);
	BatteryDetection(FALSE);
#endif	
#ifdef __WIFI_NO_LCM__

#else	
#if defined(N9H20K3) || defined(N9H20K5)
	initVPostShowLogo();
#endif	
#endif
	init();			
			
	fsInitFileSystem();			
						
#if defined(__ENABLE_SD_CARD_0__)||defined(__ENABLE_SD_CARD_1__)||defined(__ENABLE_SD_CARD_2__)	
	sysprintf("NVT Loader: g_ibr_boot_sd_port = %d\n", g_ibr_boot_sd_port);
	NVT_LoadKernelFromSD();
#endif
#if defined(__SPI_ONLY__)
	sysprintf("NVT Loader SPI");
	NVT_LoadKernelFromSPI();
#endif

	sysprintf("Load code from NAND\n");
	NVT_LoadKernelFromNAND();
	
	return(0); // avoid compilation warning
}

