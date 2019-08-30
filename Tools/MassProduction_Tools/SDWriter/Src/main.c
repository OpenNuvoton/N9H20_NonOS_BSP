#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "N9H20.h"
#include "writer.h"

//--- Define UPDATE_BOOT_CODE_OPTIONAL_SETTING to parse TurboWriter.ini and config Boot Code Optional Setting.
#define UPDATE_BOOT_CODE_OPTIONAL_SETTING

//--- 2014/11/19, new version IBR that support User-Define Setting in Boot Code Header
#ifdef UPDATE_BOOT_CODE_OPTIONAL_SETTING
    #define IBR_NEW     1
    #define IBR_OLD     0
    INT32 Chip_flag = IBR_OLD;
    IBR_BOOT_STRUCT_T_NEW BootCodeMark_new;
#endif

extern S_DEMO_FONT s_sDemo_Font;
extern INI_INFO_T Ini_Writer;

int font_x=0, font_y=16;
UINT32 u32SkipX;

/*-----------------------------------------------------------------------------
 * Define message display level
 *---------------------------------------------------------------------------*/
// show large messages for debug
#define DBG_PRINTF  sysprintf
//#define DBG_PRINTF(...)
// show improtant information for normal operation
#define INF_PRINTF  sysprintf
// show error message
#define ERR_PRINTF  sysprintf

//--- global variables defined at SD driver (SDGlue.c)
extern PDISK_T *pDisk_SD0;
extern PDISK_T *pDisk_SD1;
extern PDISK_T *pDisk_SD2;

/**********************************/
#if defined (__GNUC__)
    UINT8 infoBufArray[0x50000] __attribute__((aligned (32)));
    UINT8 StorageBufferArray[0x50000] __attribute__((aligned (32)));
    UINT8 CompareBufferArray[0x50000] __attribute__((aligned (32)));
#else
    __align(32) UINT8 infoBufArray[0x50000];
    __align(32) UINT8 StorageBufferArray[0x50000];
    __align(32) UINT8 CompareBufferArray[0x50000];
#endif

UINT32 infoBuf, StorageBuffer, CompareBuffer, BufferSize=0;

UINT8 *pInfo;

CHAR suNvtFullName[512], suNvtTargetFullName[512];
static INT hNvtFile = -1;
INT32 gCurSector=0;
FW_UPDATE_INFO_T FWInfo[3];
BOOL volatile bIsAbort = FALSE;
IBR_BOOT_STRUCT_T BootCodeMark;
INT32 gSDLoaderSize;


/**********************************/
int File_Copy(CHAR *suSrcName, CHAR *suDstName)
{
	INT		hFileSrc, hFileDst, nByteCnt, nStatus, nStatusW, nByteCntW;

    hFileSrc = fsOpenFile(suSrcName, NULL, O_RDONLY);
    if (hFileSrc < 0)
        return hFileSrc;

    hFileDst = fsOpenFile(suDstName, NULL, O_CREATE);
    if (hFileDst < 0)
    {
        fsCloseFile(hFileSrc);
        return hFileDst;
    }

    while (1)
    {
        Draw_Wait_Status(font_x+ u32SkipX*g_Font_Step, font_y);

        nStatus = fsReadFile(hFileSrc, (UINT8 *)StorageBuffer, BufferSize, &nByteCnt);
        if (nStatus < 0)
        {
            if (nStatus != ERR_FILE_EOF)
            {
                ERR_PRINTF("ERROR in File_Copy(): Read file %s error ! Reture = 0x%x !!\n", suSrcName, nStatus);
       		}
            break;
   		}

		nStatusW = fsWriteFile(hFileDst, (UINT8 *)StorageBuffer, nByteCnt, &nByteCntW);
   		if (nStatusW < 0)
   			break;

    	if ((nByteCnt < BufferSize) || (nByteCnt != nByteCntW))
            break;
    }
    fsCloseFile(hFileSrc);
    fsCloseFile(hFileDst);

    if (nStatus == ERR_FILE_EOF)
        nStatus = 0;
    return nStatus;
}


int File_Compare(CHAR *suFileNameS, CHAR *suFileNameD)
{
    INT     hFileS, hFileD;
    INT     nLenS, nLenD, nStatusS, nStatusD;

    hFileS = fsOpenFile(suFileNameS, NULL, O_RDONLY);
    if (hFileS < 0)
        return hFileS;

    hFileD = fsOpenFile(suFileNameD, NULL, O_RDONLY);
    if (hFileD < 0)
        return hFileD;

    while (1)
    {
        Draw_Wait_Status(font_x+ u32SkipX*g_Font_Step, font_y);
        nStatusS = fsReadFile(hFileS, (UINT8 *)StorageBuffer, BufferSize, &nLenS);
        nStatusD = fsReadFile(hFileD, (UINT8 *)CompareBuffer, BufferSize, &nLenD);

        if ((nStatusS == ERR_FILE_EOF) && (nStatusD == ERR_FILE_EOF))
        {
            fsCloseFile(hFileS);
            fsCloseFile(hFileD);
            return 0;
        }

        if (nLenS != nLenD)
            break;

        if (memcmp((UINT8 *)StorageBuffer, (UINT8 *)CompareBuffer, nLenS))
            break;
    }

    fsCloseFile(hFileS);
    fsCloseFile(hFileD);

    return -1;
}


int copyContent(CHAR *suDirName, CHAR *suTargetName)
{
    INT             nLenS, nLenD, nStatus;
    CHAR            suSrcLongName[MAX_FILE_NAME_LEN];
    CHAR            suDstLongName[MAX_FILE_NAME_LEN];
    CHAR            suSlash[6] = { '\\', 0, 0, 0 };
    FILE_FIND_T     tFileInfo;
    CHAR            Array1[64], FolderName[64];

    fsUnicodeStrCat(suDirName, suSlash);    /* append '\' */
    fsUnicodeStrCat(suTargetName, suSlash); /* append '\' */

    memset((UINT8 *)&tFileInfo, 0, sizeof(tFileInfo));

    nStatus = fsFindFirst(suDirName, NULL, &tFileInfo);
    if (nStatus < 0)
    {
        fsUnicodeToAscii(suDirName,FolderName, TRUE);
        sprintf(Array1, "No %s Folder", FolderName);
        Draw_CurrentOperation(Array1,nStatus);
        return nStatus;
    }

    do
    {
        if (tFileInfo.ucAttrib & A_DIR)
        {
            if ((strcmp(tFileInfo.szShortName, "..") == 0) ||
                (strcmp(tFileInfo.szShortName, ".") == 0))
                continue;

            fsUnicodeCopyStr(suSrcLongName, suDirName);
            fsUnicodeCopyStr(suDstLongName, suTargetName);
            nLenS = fsUnicodeStrLen(suDirName);
            nLenD = fsUnicodeStrLen(suTargetName);
            if ( !((suDirName[nLenS-2] == '\\') && (suDirName[nLenS-1] == 0)) )
                fsUnicodeStrCat(suSrcLongName, suSlash);    /* append '\' */
            if ( !((suTargetName[nLenD-2] == '\\') && (suTargetName[nLenD-1] == 0)) )
                fsUnicodeStrCat(suDstLongName, suSlash);    /* append '\' */
            fsUnicodeStrCat(suSrcLongName, tFileInfo.suLongName);
            fsUnicodeStrCat(suDstLongName, tFileInfo.suLongName);

            nStatus = fsMakeDirectory(suDstLongName, NULL);
            if (nStatus < 0)
            {
                Draw_CurrentOperation("Unable to Create Directory",nStatus);
                return nStatus;
            }

            nStatus = copyContent(suSrcLongName, suDstLongName);
            if (nStatus < 0)
            {
                ERR_PRINTF("===> ERROR in copyContent(): Copy file %s fail ! Return = 0x%x !!\n", suSrcLongName, nStatus);
                bIsAbort = TRUE;
                return nStatus;
            }
        }
        else
        {
            fsUnicodeCopyStr(suSrcLongName, suDirName);
            fsUnicodeCopyStr(suDstLongName, suTargetName);
            fsUnicodeStrCat(suSrcLongName, tFileInfo.suLongName);
            fsUnicodeStrCat(suDstLongName, tFileInfo.suLongName);

            INF_PRINTF("Copying   file %s\n", tFileInfo.szShortName);
            sprintf(Array1, "Copying %s", tFileInfo.szShortName);
            nStatus = File_Copy(suSrcLongName, suDstLongName);
            if (nStatus < 0)
            {
                INF_PRINTF("Copying   file %s fail (0x%08X)\n", tFileInfo.szShortName, nStatus);
                Draw_CurrentOperation(Array1,nStatus);
                return nStatus;
            }
            else
                Draw_CurrentOperation(Array1,nStatus);

            INF_PRINTF("Comparing file %s\n", tFileInfo.szShortName);
            sprintf(Array1, "Comparing %s", tFileInfo.szShortName);
            nStatus = File_Compare(suSrcLongName, suDstLongName);
            if (nStatus < 0)
            {
                INF_PRINTF("Comparing file %s fail (0x%08X)\n", tFileInfo.szShortName, nStatus);
                Draw_CurrentOperation(Array1,nStatus);
                return nStatus;
            }
            Draw_CurrentOperation(Array1,nStatus);
        }

    } while (!fsFindNext(&tFileInfo));

    fsFindClose(&tFileInfo);
    return 0;
}


/*-----------------------------------------------------------------------------
 * Write StorageBuffer to sector gCurSector in SD card 1.
 *---------------------------------------------------------------------------*/
void nvtWriteSD(UINT32 len)
{
    int volatile status = 0;
    int volatile sector_count;

    sector_count = len / pDisk_SD1->nSectorSize;
    if ((len % pDisk_SD1->nSectorSize) != 0)
        sector_count++;

    status = sicSdWrite1(gCurSector, sector_count, (INT32)StorageBuffer);
    if (status < 0)
    {
        ERR_PRINTF("ERROR in nvtWriteSD(): Write SD error ! Return = 0x%x !!\n", status);
    }

#if 0
    // read data back to comfirm it
    status = sicSdRead1(gCurSector, sector_count, (INT32)CompareBuffer);
    if (memcmp((UINT8 *)StorageBuffer, (UINT8 *)CompareBuffer, len)==0)
        DBG_PRINTF("SD1 write OK from sector %d, count %d.\n", gCurSector, sector_count);
    else
        ERR_PRINTF("SD1 write FAIL from sector %d, count %d.\n", gCurSector, sector_count);
#endif

    gCurSector += sector_count;
}


/*-----------------------------------------------------------------------------
 * Write StorageBuffer to sector gCurSector in SD card 2.
 *---------------------------------------------------------------------------*/
void nvtWriteSD2(UINT32 len)
{
    int volatile status = 0;
    int volatile sector_count;

    sector_count = len / pDisk_SD2->nSectorSize;
    if ((len % pDisk_SD2->nSectorSize) != 0)
        sector_count++;

    status = sicSdWrite2(gCurSector, sector_count, (INT32)StorageBuffer);
    if (status < 0)
    {
        ERR_PRINTF("ERROR in nvtWriteSD2(): Write SD error ! Return = 0x%x !!\n", status);
    }

#if 0
    // read data back to comfirm it
    status = sicSdRead2(gCurSector, sector_count, (INT32)CompareBuffer);
    if (memcmp((UINT8 *)StorageBuffer, (UINT8 *)CompareBuffer, len)==0)
        DBG_PRINTF("SD2 write OK from sector %d, count %d.\n", gCurSector, sector_count);
    else
        ERR_PRINTF("SD2 write FAIL from sector %d, count %d.\n", gCurSector, sector_count);
#endif

    gCurSector += sector_count;
}


/**********************************/
int main()
{
    DateTime_T ltime;
    int status=0, nReadLen;
    CHAR szNvtFullName[64];

    PARTITION_T *ptPart;
    int LogicSector1=-1, LogicSector2=-1;
    int BootCodeMarkFlag, i;
    int /*srcDrive1 = 0,*/ dstDrive1 = 0, dstDrive2 = 0;    // the disk drive number for src and dst SD card
    int driveNumber;
    UINT32 uBlockSize, uFreeSize, uDiskSize;

    char Array1[64];
    UINT Next_Font_Height;
    INT  Disk1Size, FileInfoIdx=0;
    int  partition_size, sector_size;

#ifdef UPDATE_BOOT_CODE_OPTIONAL_SETTING
    int optional_ini_size = 0;
    extern IBR_BOOT_OPTIONAL_STRUCT_T optional_ini_file;
#endif

    sysSetSystemClock(eSYS_UPLL,    //E_SYS_SRC_CLK eSrcClk,
                        192000,     //UINT32 u32PllKHz,
                        192000,     //UINT32 u32SysKHz,
                         96000,     //UINT32 u32CpuKHz,
                         96000,     //UINT32 u32HclkKHz,
                         48000);    //UINT32 u32ApbKHz

    /* enable cache */
    sysDisableCache();
    sysInvalidCache();
    sysEnableCache(CACHE_WRITE_BACK);

    /* check SDRAM size and buffer address */
    infoBuf = (UINT32) &infoBufArray[0] | 0x80000000;
    StorageBuffer = (UINT32)&StorageBufferArray[0] | 0x80000000;
    CompareBuffer = (UINT32)&CompareBufferArray[0] | 0x80000000;
    pInfo = (UINT8 *)((UINT32)infoBuf | 0x80000000);

    /* configure Timer0 for FMI library */
    sysSetTimerReferenceClock(TIMER0, 12000000);    // External Crystal
    sysStartTimer(TIMER0, 100, PERIODIC_MODE);      // 100 ticks/per sec ==> 1tick/10ms

	sysprintf("=====> N9H20 SDWriter (v%d.%d) Begin [%d] <=====\n", MAJOR_VERSION_NUM, MINOR_VERSION_NUM, sysGetTicks(0));

    //--- 2014/11/19, check out IBR version
    if(inp8(0xFFFF3EDC) == 0x032)
    {
        Chip_flag = IBR_NEW;
        sysprintf("The Chip used latest version IBR -> Support User-Define Setting in Boot Code Header\n");
    }
    else
    {
        Chip_flag = IBR_OLD;
        sysprintf("The Chip used old version IBR -> Not Support User-Define Setting in Boot Code Header\n");
    }

	ltime.year = 2011;
	ltime.mon  = 10;
	ltime.day  = 31;
	ltime.hour = 8;
	ltime.min  = 40;
    ltime.sec  = 0;
    sysSetLocalTime(ltime);

    fsInitFileSystem();

    // The NVTFAT limitation, don't need to assign drive number to SD.
    //      The NVTFAT will auto assign 'C' to SD card partition 1 on SD port 0

    //--- initial panel
    Draw_Init();
    font_y = g_Font_Height;
    Next_Font_Height = g_Font_Height-6;

    sicIoctl(SIC_SET_CLOCK, 192000, 0, 0);
    sicOpen();

    //--- mount SD card on port 0
    Draw_Font(COLOR_RGB16_WHITE,  &s_sDemo_Font, font_x, font_y, "Mount SD Card 0:");
    u32SkipX = 16;
    status = sicSdOpen0();
    if (status < 0)
    {
        Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
        ERR_PRINTF("===> 1 (No source SD Card on port 0)\n");
        bIsAbort = TRUE;
        goto _end_;
    }
    INF_PRINTF("Detect SD0: %d sectors * %d Bytes = %d KBytes\n",
                pDisk_SD0->uTotalSectorN, pDisk_SD0->nSectorSize, pDisk_SD0->uDiskSize);

    // Get the SDWriter setting from INI file (SDWriter.ini)
    status = ProcessINI("C:\\SDWriter.ini");
	if (status < 0)
	{
		Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
        ERR_PRINTF("===> 1.1 (Wrong INI file)\n");
		bIsAbort = TRUE;
		goto _end_;
	}
    Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Successful);
    font_y += Next_Font_Height;

#ifdef UPDATE_BOOT_CODE_OPTIONAL_SETTING
    if (Chip_flag == IBR_NEW)
    {
        // Get the Boot Code Optional Setting from INI file (TurboWriter.ini) to optional_ini_file
        status = ProcessOptionalINI("C:\\TurboWriter.ini");
        if (status < 0)
        {
            ERR_PRINTF("===> 1.2 (Wrong TurboWriter INI file)\n");
            bIsAbort = TRUE;
            goto _end_;
        }

        if (optional_ini_file.Counter == 0)
            optional_ini_size = 0;
        else
        {
            optional_ini_size = 8 * (optional_ini_file.Counter + 1);
            if (optional_ini_file.Counter % 2 == 0)
                optional_ini_size += 8;     // for dummy pair to make sure 16 bytes alignment.
        }
    }
#endif

    //--- mount SD card on port 1 or 2
    if (Ini_Writer.Target_SD_Port == 2)
    {
        Draw_Font(COLOR_RGB16_WHITE,  &s_sDemo_Font, font_x, font_y, "Mount SD Card 2:");
        status = sicSdOpen2();
    }
    else
    {
        Draw_Font(COLOR_RGB16_WHITE,  &s_sDemo_Font, font_x, font_y, "Mount SD Card 1:");
        status = sicSdOpen1();
    }

    u32SkipX = 16;
    if (status < 0)
    {
        Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
        if (Ini_Writer.Target_SD_Port == 2)
            ERR_PRINTF("===> 1.2 (No destination SD Card on port 2)\n");
        else
            ERR_PRINTF("===> 1.2 (No destination SD Card on port 1)\n");
        bIsAbort = TRUE;
        goto _end_;
    }
    Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Successful);
    font_y += Next_Font_Height;

    if (Ini_Writer.Target_SD_Port == 2)
    {
        INF_PRINTF("Detect SD2: %d sectors * %d Bytes = %d KBytes\n",
                    pDisk_SD2->uTotalSectorN, pDisk_SD2->nSectorSize, pDisk_SD2->uDiskSize);
        sprintf(Array1, "SD2:%d(Sec)*%dB=%dMB",
                pDisk_SD2->uTotalSectorN, pDisk_SD2->nSectorSize, pDisk_SD2->uDiskSize/1024);
    }
    else
    {
        INF_PRINTF("Detect SD1: %d sectors * %d Bytes = %d KBytes\n",
                    pDisk_SD1->uTotalSectorN, pDisk_SD1->nSectorSize, pDisk_SD1->uDiskSize);
        sprintf(Array1, "SD1:%d(Sec)*%dB=%dMB",
                pDisk_SD1->uTotalSectorN, pDisk_SD1->nSectorSize, pDisk_SD1->uDiskSize/1024);
    }
    Draw_Font(COLOR_RGB16_WHITE, &s_sDemo_Font, 0,  _LCM_HEIGHT_ -1-g_Font_Height,Array1);

    BufferSize = pDisk_SD0->nSectorSize * 128;
    if (BufferSize >= 0x50000)
        BufferSize = 0x40000;

    /*******************/
    /* copy SDLoader   */
    /*******************/
    if (strlen(Ini_Writer.SDLoader) ==0)
        goto WriteLogo;

    INF_PRINTF("=====> copy and verify SDLoader [%d] <=====\n", sysGetTicks(0));

    Draw_Font(COLOR_RGB16_WHITE, &s_sDemo_Font, font_x, font_y, "Writing SDLoader:");
    u32SkipX = 17;

    sprintf(szNvtFullName, "C:\\%s",Ini_Writer.SDLoader);
    fsAsciiToUnicode(szNvtFullName, suNvtFullName, TRUE);
    hNvtFile = fsOpenFile(suNvtFullName, NULL, O_RDONLY);

    sprintf(Array1, "Open %s", szNvtFullName);
    if (hNvtFile < 0)
    {
        Draw_CurrentOperation(Array1,hNvtFile);
        Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
        ERR_PRINTF("===> 2.1 Open file name [%s] fail ! Return = 0x%x !!\n", suNvtFullName, hNvtFile);
        bIsAbort = TRUE;
        goto _end_;
    }
    else
        Draw_CurrentOperation(Array1,hNvtFile);

    /* image information */

    FWInfo[FileInfoIdx].imageNo = FileInfoIdx;
    FWInfo[FileInfoIdx].imageFlag = 3;      // image type: system image
    FWInfo[FileInfoIdx].startBlock = 1;
    // FWInfo[FileInfoIdx].endBlock = 3;
#if defined(N9H20K5)
    FWInfo[FileInfoIdx].executeAddr = 0x900000;
#endif
#if defined(N9H20K3)
    FWInfo[FileInfoIdx].executeAddr = 0x700000;
#endif
#if defined(N9H20K1)
    FWInfo[FileInfoIdx].executeAddr = 0x180000;
#endif


    FWInfo[FileInfoIdx].fileLen = (UINT32)fsGetFileSize(hNvtFile);
    gSDLoaderSize = FWInfo[FileInfoIdx].fileLen;
    memcpy(&FWInfo[FileInfoIdx].imageName[0], Ini_Writer.SDLoader, 512);

    // initial Boot Code Mark for SDLoader
    if (Chip_flag == IBR_NEW)
    {
        BootCodeMark_new.BootCodeMarker = 0x57425AA5;
#if defined(N9H20K5)
        BootCodeMark_new.ExeAddr = 0x900000;
#endif
#if defined(N9H20K3)
        BootCodeMark_new.ExeAddr = 0x700000;
#endif
#if defined(N9H20K1)
        BootCodeMark_new.ExeAddr = 0x180000;
#endif
        BootCodeMark_new.ImageSize = FWInfo[FileInfoIdx].fileLen;
        BootCodeMark_new.Reserved = 0x0000000;
    }
    else
    {
        BootCodeMark.BootCodeMarker = 0x57425AA5;
#if defined(N9H20K5)
        BootCodeMark.ExeAddr = 0x900000;
#endif
#if defined(N9H20K3)
        BootCodeMark.ExeAddr = 0x700000;
#endif
#if defined(N9H20K1)
        BootCodeMark.ExeAddr = 0x180000;
#endif
        BootCodeMark.ImageSize = FWInfo[FileInfoIdx].fileLen;
        BootCodeMark.SkewMarker = 0xA55A4257;
        BootCodeMark.DQSODS = 0x00001010;
        BootCodeMark.CLKQSDS = 0x00888800;
        BootCodeMark.DRAMMarker = 0x0;      // No DRMA Size Marker
        BootCodeMark.DRAMSize = 0x0;
    }

    gCurSector = 1;         // write SDLoader.bin to reserve area sector 1, NOT secter 0
    BootCodeMarkFlag = 1;

    fsFileSeek(hNvtFile, 0, SEEK_SET);
    while(1)
    {
        if (BootCodeMarkFlag)
        {
#ifdef UPDATE_BOOT_CODE_OPTIONAL_SETTING
            // write 1st sector with Boot Code Mark and optional header for SD
            if (Chip_flag == IBR_NEW)
            {
                memcpy((UINT8 *)StorageBuffer, (UINT8 *)&BootCodeMark_new, sizeof(IBR_BOOT_STRUCT_T_NEW));
                memcpy((UINT8 *)(StorageBuffer+sizeof(IBR_BOOT_STRUCT_T_NEW)), (UINT8 *)&optional_ini_file, optional_ini_size);
                status = fsReadFile(hNvtFile, (UINT8 *)(StorageBuffer+sizeof(IBR_BOOT_STRUCT_T_NEW)+optional_ini_size),
                                    BufferSize - sizeof(IBR_BOOT_STRUCT_T_NEW) - optional_ini_size, &nReadLen);

                if (Ini_Writer.Target_SD_Port == 2)
                    nvtWriteSD2(nReadLen+sizeof(IBR_BOOT_STRUCT_T_NEW)+optional_ini_size);
                else
                    nvtWriteSD(nReadLen+sizeof(IBR_BOOT_STRUCT_T_NEW)+optional_ini_size);
            }
            else
            {
                memcpy((UINT8 *)StorageBuffer, (UINT8 *)&BootCodeMark, sizeof(IBR_BOOT_STRUCT_T));
                status = fsReadFile(hNvtFile, (UINT8 *)(StorageBuffer+sizeof(IBR_BOOT_STRUCT_T)),
                                    BufferSize - sizeof(IBR_BOOT_STRUCT_T), &nReadLen);
                if (Ini_Writer.Target_SD_Port == 2)
                    nvtWriteSD2(nReadLen+sizeof(IBR_BOOT_STRUCT_T));
                else
                    nvtWriteSD(nReadLen+sizeof(IBR_BOOT_STRUCT_T));
            }
#else
            // write 1st sector with Boot Code Mark for SD
            memcpy((UINT8 *)StorageBuffer, (UINT8 *)&BootCodeMark, sizeof(IBR_BOOT_STRUCT_T));
            status = fsReadFile(hNvtFile, (UINT8 *)(StorageBuffer+sizeof(IBR_BOOT_STRUCT_T)),
                                BufferSize - sizeof(IBR_BOOT_STRUCT_T), &nReadLen);
            if (Ini_Writer.Target_SD_Port == 2)
                nvtWriteSD2(nReadLen+sizeof(IBR_BOOT_STRUCT_T));
            else
                nvtWriteSD(nReadLen+sizeof(IBR_BOOT_STRUCT_T));
#endif
            BootCodeMarkFlag = 0;   // boot code mark had written
        }
        else
        {
            status = fsReadFile(hNvtFile, (UINT8 *)StorageBuffer, BufferSize, &nReadLen);
            if (status == ERR_FILE_EOF)
                break;
            if (Ini_Writer.Target_SD_Port == 2)
                nvtWriteSD2(nReadLen);  // write StorageBuffer to SD card
            else
                nvtWriteSD(nReadLen);   // write StorageBuffer to SD card
        }
        if (status == ERR_FILE_EOF)
            break;
        else if (status < 0)
        {
            Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
            ERR_PRINTF("===> 2.3 fsReadFile() fail for SDLoader ! Return = 0x%x !!\n", status);
            bIsAbort = TRUE;
            goto _end_;
        }
    }

    Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Successful);
    font_y += Next_Font_Height;

    fsCloseFile(hNvtFile);
    hNvtFile = -1;

    FWInfo[FileInfoIdx].endBlock = gCurSector - 1;
    gCurSector = 0x22;      // 0x21 is the System Info sector, the next image MUST begin from 0x22
    FileInfoIdx++;

    /*************/
    /* copy logo */
    /*************/
WriteLogo:
    if (strlen(Ini_Writer.Logo) ==0)
        goto WriteNVTLoader;

    INF_PRINTF("=====> copy and verify logo [%d] <=====\n", sysGetTicks(0));

    Draw_Font(COLOR_RGB16_WHITE, &s_sDemo_Font, font_x, font_y, "Writing Logo:");
    u32SkipX = 13;

    sprintf(szNvtFullName, "C:\\%s", Ini_Writer.Logo);
    fsAsciiToUnicode(szNvtFullName, suNvtFullName, TRUE);
    hNvtFile = fsOpenFile(suNvtFullName, NULL, O_RDONLY);

    sprintf(Array1, "Open %s", szNvtFullName);
    if (hNvtFile < 0)
    {
        Draw_CurrentOperation(Array1,hNvtFile);
        Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
        ERR_PRINTF("===> 3.1 Open file name [%s] fail ! Return = 0x%x !!\n", suNvtFullName, hNvtFile);
        bIsAbort = TRUE;
        goto _end_;
    }
    else
        Draw_CurrentOperation(Array1,hNvtFile);

    /* image information */
    FWInfo[FileInfoIdx].imageNo = FileInfoIdx;
    FWInfo[FileInfoIdx].imageFlag = 4;      // image type: logo
    FWInfo[FileInfoIdx].startBlock = gCurSector;
#if defined(N9H20K5) || defined(N9H20K3)
    FWInfo[FileInfoIdx].executeAddr = 0x500000;
#endif
#if defined(N9H20K1)
    FWInfo[FileInfoIdx].executeAddr = 0x190000;	//Although didn't support logo in 2MB solution. But we still defined it.
#endif
    FWInfo[FileInfoIdx].fileLen = (UINT32)fsGetFileSize(hNvtFile);
    memcpy(&FWInfo[FileInfoIdx].imageName[0], Ini_Writer.Logo, 512);

    while(1)
    {
        status = fsReadFile(hNvtFile, (UINT8 *)StorageBuffer, BufferSize, &nReadLen);
        if (status == ERR_FILE_EOF)
            break;
        else if (status < 0)
        {
            Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
            ERR_PRINTF("===> 3.2 fsReadFile() fail for Logo ! Return = 0x%x !!\n", status);
            bIsAbort = TRUE;
            goto _end_;
        }
        if (Ini_Writer.Target_SD_Port == 2)
            nvtWriteSD2(nReadLen);   // write StorageBuffer to SD card
        else
            nvtWriteSD(nReadLen);   // write StorageBuffer to SD card
    }

    /* verify logo */
    fsFileSeek(hNvtFile, 0, SEEK_SET);
    i = FWInfo[FileInfoIdx].startBlock;
    while(1)
    {
        if (Ini_Writer.Target_SD_Port == 2)
        {
            status = fsReadFile(hNvtFile, (UINT8 *)StorageBuffer, pDisk_SD2->nSectorSize, &nReadLen);
            sicSdRead2(i, 1, (INT32)CompareBuffer);
        }
        else
        {
            status = fsReadFile(hNvtFile, (UINT8 *)StorageBuffer, pDisk_SD1->nSectorSize, &nReadLen);
            sicSdRead1(i, 1, (INT32)CompareBuffer);
        }
        i++;

        if (memcmp((UINT8 *)StorageBuffer, (UINT8 *)CompareBuffer, nReadLen))
        {
            Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
            ERR_PRINTF("===> 3.3 Data verify fail for Logo ! Return = 0x%x !!\n", status);
            bIsAbort = TRUE;
            goto _end_;
        }

        if (status == ERR_FILE_EOF)
            break;
        else if (status < 0)
        {
            Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
            ERR_PRINTF("===> 3.4 fsReadFile() fail for Logo ! Return = 0x%x !!\n", status);
            bIsAbort = TRUE;
            goto _end_;
        }
    }
    Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Successful);
    font_y += Next_Font_Height;

    fsCloseFile(hNvtFile);
    hNvtFile = -1;

    FWInfo[FileInfoIdx].endBlock = gCurSector - 1;
    FileInfoIdx++;

    /******************/
    /* copy nvtloader */
    /******************/
WriteNVTLoader:
    if (strlen(Ini_Writer.NVTLoader) ==0)
        goto WriteSysteInfo;

    INF_PRINTF("=====> copy and verify nvtloader [%d] <=====\n", sysGetTicks(0));

    Draw_Font(COLOR_RGB16_WHITE, &s_sDemo_Font, font_x, font_y, "Writing NvtLoader:");
    u32SkipX = 18;

    sprintf(szNvtFullName, "C:\\%s",Ini_Writer.NVTLoader);
    fsAsciiToUnicode(szNvtFullName, suNvtFullName, TRUE);
    hNvtFile = fsOpenFile(suNvtFullName, NULL, O_RDONLY);

    sprintf(Array1, "Open %s", szNvtFullName);
    if (hNvtFile < 0)
    {
        Draw_CurrentOperation(Array1,hNvtFile);
        Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
        ERR_PRINTF("===> 4.1 Open file name [%s] fail ! Return = 0x%x !!\n", suNvtFullName, hNvtFile);
        bIsAbort = TRUE;
        goto _end_;
    }
    else
        Draw_CurrentOperation(Array1,hNvtFile);

    /* image information */
    FWInfo[FileInfoIdx].imageNo = FileInfoIdx;
    FWInfo[FileInfoIdx].imageFlag = 1;      // image type: execute
    FWInfo[FileInfoIdx].startBlock = gCurSector;


#if defined(N9H20K5)
    FWInfo[FileInfoIdx].executeAddr = 0x800000;
#endif
#if defined(N9H20K3)
    FWInfo[FileInfoIdx].executeAddr = 0x600000;
#endif
#if defined(N9H20K1)
    FWInfo[FileInfoIdx].executeAddr = 0x60000;
#endif
    FWInfo[FileInfoIdx].fileLen = (UINT32)fsGetFileSize(hNvtFile);
    memcpy(&FWInfo[FileInfoIdx].imageName[0], Ini_Writer.NVTLoader, 512);

    while(1)
    {
        status = fsReadFile(hNvtFile, (UINT8 *)StorageBuffer, BufferSize, &nReadLen);
        if (status == ERR_FILE_EOF)
            break;
        else if (status < 0)
        {
            Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
            ERR_PRINTF("===> 4.2 fsReadFile() fail for NVTLoader ! Return = 0x%x !!\n", status);
            bIsAbort = TRUE;
            goto _end_;
        }
        if (Ini_Writer.Target_SD_Port == 2)
            nvtWriteSD2(nReadLen);
        else
            nvtWriteSD(nReadLen);
    }

    /* verify nvtloader */
    fsFileSeek(hNvtFile, 0, SEEK_SET);
    i = FWInfo[FileInfoIdx].startBlock;
    while(1)
    {
        if (Ini_Writer.Target_SD_Port == 2)
        {
            status = fsReadFile(hNvtFile, (UINT8 *)StorageBuffer, pDisk_SD2->nSectorSize, &nReadLen);
            sicSdRead2(i, 1, (INT32)CompareBuffer);
        }
        else
        {
            status = fsReadFile(hNvtFile, (UINT8 *)StorageBuffer, pDisk_SD1->nSectorSize, &nReadLen);
            sicSdRead1(i, 1, (INT32)CompareBuffer);
        }
        i++;
        if (memcmp((UINT8 *)StorageBuffer, (UINT8 *)CompareBuffer, nReadLen))
        {
            Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
            ERR_PRINTF("===> 4.3 Data verify fail for NVTLoader ! Return = 0x%x !!\n", status);
            bIsAbort = TRUE;
            goto _end_;
        }

        if (status == ERR_FILE_EOF)
            break;
        else if (status < 0)
        {
            Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
            ERR_PRINTF("===> 4.4 fsReadFile() fail for NVTLoader ! Return = 0x%x !!\n", status);
            bIsAbort = TRUE;
            goto _end_;
        }
    }

    Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Successful);
    font_y += Next_Font_Height;

    fsCloseFile(hNvtFile);
    hNvtFile = -1;

    FWInfo[FileInfoIdx].endBlock = gCurSector - 1;
    FileInfoIdx++;

WriteSysteInfo:

    /* set system reserved area */
    if (Ini_Writer.Target_SD_Port == 2)
        partition_size = pDisk_SD2->uDiskSize/1024;
    else
        partition_size = pDisk_SD1->uDiskSize/1024;
    if ((Ini_Writer.SystemReservedMegaByte > partition_size) || (Ini_Writer.SystemReservedMegaByte < 0))
        Ini_Writer.SystemReservedMegaByte = 8;  // default is 8MBytes for system area
    INF_PRINTF("System Reserved Size = %d MBytes\n", Ini_Writer.SystemReservedMegaByte);

    /* set information to sector 0x21 */
    {
        unsigned int *ptr;
        pInfo = (UINT8 *)((UINT32)infoBuf | 0x80000000);
        ptr = (unsigned int *)((UINT32)infoBuf | 0x80000000);

        if (Ini_Writer.Target_SD_Port == 2)
            memset(pInfo, 0xff,  pDisk_SD2->nSectorSize);
        else
            memset(pInfo, 0xff,  pDisk_SD1->nSectorSize);

        /* update image information for SD card */
        *(ptr+0) = 0x574255AA;      // magic number
        *(ptr+1) = FileInfoIdx;     // image count
        *(ptr+2) = Ini_Writer.SystemReservedMegaByte * 1024 * 2;    // sector number for systeam area
        *(ptr+3) = 0x57425963;      // magic number

        if (Ini_Writer.Target_SD_Port == 2)
        {
            memcpy(pInfo+16, (char *)&FWInfo, pDisk_SD2->nSectorSize - 16);
            status = sicSdWrite2(0x21, 1, (INT32)pInfo);
        }
        else
        {
            memcpy(pInfo+16, (char *)&FWInfo, pDisk_SD1->nSectorSize - 16);
            status = sicSdWrite1(0x21, 1, (INT32)pInfo);
        }
        if (status < 0)
        {
            ERR_PRINTF("ERROR(): Write System Info to SD card error ! Return = 0x%x !!\n", status);
        }

        /* Verify information */
        if (Ini_Writer.Target_SD_Port == 2)
        {
            status = sicSdRead2(0x21, 1, (INT32)CompareBuffer);
            sector_size = pDisk_SD2->nSectorSize;
        }
        else
        {
            status = sicSdRead1(0x21, 1, (INT32)CompareBuffer);
            sector_size = pDisk_SD1->nSectorSize;
        }
        if (memcmp((UINT8 *)pInfo, (UINT8 *)CompareBuffer, sector_size)==0)
            INF_PRINTF("System Info write OK on sector 33.\n");
        else
        {
            ERR_PRINTF("===> 4.5 System Info write FAIL on sector 33.\n");
            bIsAbort = TRUE;
            goto _end_;
        }
    }

    //--- partition SD card 1 to 2 partitions
    INF_PRINTF("\n=====> partition and format [%d] <=====\n", sysGetTicks(0));

    /* partition and format SD1-1 and SD1-2 */
    if (Ini_Writer.Target_SD_Port == 2)
    {
        Draw_Font(COLOR_RGB16_WHITE, &s_sDemo_Font, font_x, font_y, "Format SD Card 2:");
        u32SkipX = 17;

        if ((Ini_Writer.SD1_1_SIZE < pDisk_SD2->uDiskSize/1024) && (Ini_Writer.SD1_1_SIZE > 0))
            Disk1Size = Ini_Writer.SD1_1_SIZE;
        else
            Disk1Size = 16;     // default is 16MBytes for first partition
        INF_PRINTF("Set SD 2 Partition 1 Size = %d MBytes\n", Disk1Size);

        fsSetReservedArea(Ini_Writer.SystemReservedMegaByte*1024*2);    // set reserved sector number for fsTwoPartAndFormatAll()
        status = fsTwoPartAndFormatAll(pDisk_SD2, Disk1Size*1024,
                                       pDisk_SD2->uDiskSize - (Disk1Size + Ini_Writer.SystemReservedMegaByte)*1024);
    }
    else
    {
        Draw_Font(COLOR_RGB16_WHITE, &s_sDemo_Font, font_x, font_y, "Format SD Card 1:");
        u32SkipX = 17;

        if ((Ini_Writer.SD1_1_SIZE < pDisk_SD1->uDiskSize/1024) && (Ini_Writer.SD1_1_SIZE > 0))
            Disk1Size = Ini_Writer.SD1_1_SIZE;
        else
            Disk1Size = 16;     // default is 16MBytes for first partition
        INF_PRINTF("Set SD 1 Partition 1 Size = %d MBytes\n", Disk1Size);

        fsSetReservedArea(Ini_Writer.SystemReservedMegaByte*1024*2);    // set reserved sector number for fsTwoPartAndFormatAll()
        status = fsTwoPartAndFormatAll(pDisk_SD1, Disk1Size*1024,
                                       pDisk_SD1->uDiskSize - (Disk1Size + Ini_Writer.SystemReservedMegaByte)*1024);
    }
    if (status < 0)
    {
        Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
        ERR_PRINTF("===> 5 (Format disk fail) ! Return = 0x%x !!\n", status);
        bIsAbort = TRUE;
        goto _end_;
    }

    //--- Disk partition and format successfully. Now, scan logical disk and get disk information.
    Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Successful);
    font_y += Next_Font_Height;

    // scan source disk and get disk information
    uBlockSize=0, uFreeSize=0, uDiskSize=0;
    ptPart = pDisk_SD0->ptPartList;
    i = 0;
    while (ptPart != NULL)
    {
        driveNumber = ptPart->ptLDisk->nDriveNo;
//        if (i == 0)
//            srcDrive1 = driveNumber;
        fsDiskFreeSpace(driveNumber, &uBlockSize, &uFreeSize, &uDiskSize);
        INF_PRINTF("Source disk %c Size: %d Kbytes, Free Space: %d KBytes\n", driveNumber, (INT)uDiskSize, (INT)uFreeSize);
        ptPart = ptPart->ptNextPart;
        i++;
    }

    // scan dest disk and get disk information
    if (Ini_Writer.Target_SD_Port == 2)
        ptPart = pDisk_SD2->ptPartList;
    else
        ptPart = pDisk_SD1->ptPartList;
    i = 0;
    while (ptPart != NULL)
    {
        driveNumber = ptPart->ptLDisk->nDriveNo;
        if (i == 0)
        {
            dstDrive1 = driveNumber;
            LogicSector1 = ptPart->uStartSecN;
        }
        else if (i == 1)
        {
            dstDrive2 = driveNumber;
            LogicSector2 = ptPart->uStartSecN;
        }
        fsDiskFreeSpace(driveNumber, &uBlockSize, &uFreeSize, &uDiskSize);
        INF_PRINTF("Dest   disk %c Size: %d Kbytes, Free Space: %d KBytes\n", driveNumber, (INT)uDiskSize, (INT)uFreeSize);
        ptPart = ptPart->ptNextPart;
        i++;
    }

    // set volumn lable for dest disk
    fsSetVolumeLabel(dstDrive1, "SD1-1\n", strlen("SD1-1"));
    fsSetVolumeLabel(dstDrive2, "SD1-2\n", strlen("SD1-2"));

    /*********************************/
    /* copy first partition content  */
    /*********************************/
    if (Ini_Writer.SD1_1_FAT >= 0)
    {
        INF_PRINTF("\n=====> copy First Partition Content [%d] <=====\n", sysGetTicks(0));

        if (Ini_Writer.SD1_1_FAT == 0)
        {
            // Copy File Through FAT like Binary ISO
            Draw_Font(COLOR_RGB16_WHITE, &s_sDemo_Font, font_x, font_y, "Copying SD1-1:");
            u32SkipX = 14;

            if (LogicSector1 == -1)
            {
                Draw_CurrentOperation("LogicSector1 :",hNvtFile);
                ERR_PRINTF("===> 7.1 (Wrong start sector for SD1-1)\n");
                bIsAbort = TRUE;
                goto _end_;
            }

            strcpy(szNvtFullName, "C:\\SD1\\content.bin");
            fsAsciiToUnicode(szNvtFullName, suNvtFullName, TRUE);
            hNvtFile = fsOpenFile(suNvtFullName, NULL, O_RDONLY);
            INF_PRINTF("Copy %s\n", szNvtFullName);
            sprintf(Array1, "Copy %s", szNvtFullName);
            if (hNvtFile < 0)
            {
				sysprintf("===> 7.2 (Open SD1\\content.bin fail) [0x%x], try SD1-1 folder...\n", hNvtFile);
                strcpy(szNvtFullName, "C:\\SD1-1\\content.bin");
                fsAsciiToUnicode(szNvtFullName, suNvtFullName, TRUE);
                hNvtFile = fsOpenFile(suNvtFullName, NULL, O_RDONLY);
                INF_PRINTF("Copy %s\n", szNvtFullName);
                if (hNvtFile < 0)
                {
                    Draw_CurrentOperation("Copy C:\\SD1-1\\content.bin",hNvtFile);
                    Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
                    ERR_PRINTF("===> 7.2 (Open SD1-1\\content.bin fail) ! Return = 0x%x !!\n", hNvtFile);
                    bIsAbort = TRUE;
                    goto _end_;
                }
            }
            else
                Draw_CurrentOperation(Array1,hNvtFile);

            while(1)
            {
			    Draw_Wait_Status(font_x+ u32SkipX*g_Font_Step, font_y);
                status = fsReadFile(hNvtFile, (UINT8 *)StorageBuffer, BufferSize, &nReadLen);
                if (status == ERR_FILE_EOF)
                    break;
                else if (status < 0)
                {
				    Draw_Clear_Wait_Status(font_x+ u32SkipX*g_Font_Step, font_y);
                    Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
                    ERR_PRINTF("===> 7.3 (Read content.bin fail) ! Return = 0x%x !!\n", status);
                    bIsAbort = TRUE;
                    goto _end_;
                }
                if (Ini_Writer.Target_SD_Port == 2)
                    sicSdWrite2(LogicSector1, nReadLen/512, (INT32)StorageBuffer);
                else
                    sicSdWrite1(LogicSector1, nReadLen/512, (INT32)StorageBuffer);
                LogicSector1 += nReadLen/512;
            }
            Draw_Clear_Wait_Status(font_x+ u32SkipX*g_Font_Step, font_y);
            Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Successful);
            font_y += Next_Font_Height;
        }
        else
        {
            // Copy File through FAT
            Draw_Font(COLOR_RGB16_WHITE, &s_sDemo_Font, font_x, font_y, "Copying SD1-1:");
            u32SkipX = 14;

            szNvtFullName[0] = (char) dstDrive1;
            szNvtFullName[1] = ':';
            szNvtFullName[2] = 0;           // end of string
            fsAsciiToUnicode(szNvtFullName, suNvtTargetFullName, TRUE);

            strcpy(szNvtFullName, "C:\\SD1");
            fsAsciiToUnicode(szNvtFullName, suNvtFullName, TRUE);

            status = copyContent(suNvtFullName, suNvtTargetFullName);
            Draw_Clear_Wait_Status(font_x+ u32SkipX*g_Font_Step, font_y);
            if (status < 0)
            {
				sysprintf("===> 7.4 (Copy files in SD1 fail) [0x%x], try SD1-1 folder...\n", status);
                szNvtFullName[0] = (char) dstDrive1;
                szNvtFullName[1] = ':';
                szNvtFullName[2] = 0;           // end of string
                fsAsciiToUnicode(szNvtFullName, suNvtTargetFullName, TRUE);

                strcpy(szNvtFullName, "C:\\SD1-1");
                fsAsciiToUnicode(szNvtFullName, suNvtFullName, TRUE);

                status = copyContent(suNvtFullName, suNvtTargetFullName);
                if (status < 0)
                {
                    Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
                    ERR_PRINTF("===> 7.4 (Copy files in SD1-1 fail) ! Return = 0x%x !!\n", status);
                    bIsAbort = TRUE;
                    goto _end_;
                }
            }
            Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Successful);
            font_y += Next_Font_Height;
        }
    }

    /**********************************/
    /* copy Second Partition content  */
    /**********************************/
    if (Ini_Writer.SD1_2_FAT >= 0)
    {
        INF_PRINTF("\n=====> Copy Second Partition Content [%d] <=====\n", sysGetTicks(0));

        if (Ini_Writer.SD1_2_FAT == 0)
        {
            // Copy File Through FAT like Binary ISO
            Draw_Font(COLOR_RGB16_WHITE, &s_sDemo_Font, font_x, font_y, "Copying SD1-2:");
            u32SkipX = 14;

            if (LogicSector2 == -1)
            {
                Draw_CurrentOperation("LogicSector2:",hNvtFile);
                Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
                ERR_PRINTF("===> 8.1 (Wrong start sector for SD1-2)\n");
                bIsAbort = TRUE;
                goto _end_;
            }

            strcpy(szNvtFullName, "C:\\SD2\\content.bin");
            fsAsciiToUnicode(szNvtFullName, suNvtFullName, TRUE);
            hNvtFile = fsOpenFile(suNvtFullName, NULL, O_RDONLY);
            INF_PRINTF("Copy %s\n", szNvtFullName);
            sprintf(Array1, "Copy %s", szNvtFullName);
            if (hNvtFile < 0)
            {
				sysprintf("===> 8.2 (Open SD2\\content.bin fail) [0x%x], try SD1-2 folder...\n", hNvtFile);
                strcpy(szNvtFullName, "C:\\SD1-2\\content.bin");
                fsAsciiToUnicode(szNvtFullName, suNvtFullName, TRUE);
                hNvtFile = fsOpenFile(suNvtFullName, NULL, O_RDONLY);
                INF_PRINTF("Copy %s\n", szNvtFullName);
                if (hNvtFile < 0)
                {
                    Draw_CurrentOperation("Copy C:\\SD1-2\\content.bin",hNvtFile);
                    Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
                    ERR_PRINTF("===> 8.2 (Open SD1-2\\content.bin fail) ! Return = 0x%x !!\n", hNvtFile);
                    bIsAbort = TRUE;
                    goto _end_;
                }
            }
            else
                Draw_CurrentOperation(Array1,hNvtFile);

            while(1)
            {
			    Draw_Wait_Status(font_x+ u32SkipX*g_Font_Step, font_y);
                status = fsReadFile(hNvtFile, (UINT8 *)StorageBuffer, BufferSize, &nReadLen);
                if (status == ERR_FILE_EOF)
                    break;
                else if (status < 0)
                {
                    Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
                    ERR_PRINTF("===> 8.3 (Read content.bin fail) ! Return = 0x%x !!\n", status);
                    bIsAbort = TRUE;
                    goto _end_;
                }
                if (Ini_Writer.Target_SD_Port == 2)
                    sicSdWrite2(LogicSector2, nReadLen/512, (INT32)StorageBuffer);
                else
                    sicSdWrite1(LogicSector2, nReadLen/512, (INT32)StorageBuffer);
                LogicSector2 += nReadLen/512;
            }
            Draw_Clear_Wait_Status(font_x+ u32SkipX*g_Font_Step, font_y);
            Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Successful);
            font_y += Next_Font_Height;
        }
        else
        {
            // Copy File through FAT
            Draw_Font(COLOR_RGB16_WHITE, &s_sDemo_Font, font_x, font_y, "Copying SD1-2:");
            u32SkipX = 14;

            szNvtFullName[0] = (char) dstDrive2;
            szNvtFullName[1] = ':';
            szNvtFullName[2] = 0;           // end of string
            fsAsciiToUnicode(szNvtFullName, suNvtTargetFullName, TRUE);

            strcpy(szNvtFullName, "C:\\SD2");
            fsAsciiToUnicode(szNvtFullName, suNvtFullName, TRUE);

            status = copyContent(suNvtFullName, suNvtTargetFullName);
            Draw_Clear_Wait_Status(font_x+ u32SkipX*g_Font_Step, font_y);
            if (status < 0)
            {
				sysprintf("===> 8.4 (Copy files in SD2 fail) [0x%x], try SD1-2 folder...\n", status);
                szNvtFullName[0] = (CHAR) dstDrive2;
                szNvtFullName[1] = ':';
                szNvtFullName[2] = 0;           // end of string
                fsAsciiToUnicode(szNvtFullName, suNvtTargetFullName, TRUE);

                strcpy(szNvtFullName, "C:\\SD1-2");
                fsAsciiToUnicode(szNvtFullName, suNvtFullName, TRUE);

                status = copyContent(suNvtFullName, suNvtTargetFullName);
                if (status < 0)
                {
                    Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
                    ERR_PRINTF("===> 8.4 (Copy files in SD1-2 fail) ! Return = 0x%x !!\n", status);
                    bIsAbort = TRUE;
                    goto _end_;
                }
            }
            Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Successful);
            font_y += Next_Font_Height;
        }
    }

_end_:

    INF_PRINTF("\n=====> Finish [%d] <=====\n", sysGetTicks(0));
    Draw_FinalStatus(bIsAbort);

    while(1);
}
