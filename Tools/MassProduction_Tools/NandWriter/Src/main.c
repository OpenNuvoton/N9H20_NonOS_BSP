/*----------------------------------------------------------------------------------*/
/* Copyright (c) 2013 by Nuvoton Technology Corporation.  All rights reserved.      */
/*                                                                                  */
/* Description: The main program of NandWriter.                                       */
/*----------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "N9H20.h"
#include "writer.h"

//#define SUPPORT_PRE_SCAN_NAND
//#define SUPPORT_POST_FILE_COMPARE

//--- Define UPDATE_BOOT_CODE_OPTIONAL_SETTING to parse TurboWriter.ini and config Boot Code Optional Setting.
#define UPDATE_BOOT_CODE_OPTIONAL_SETTING

//--- 2014/11/19, new version IBR that support User-Define Setting in Boot Code Header
#ifdef UPDATE_BOOT_CODE_OPTIONAL_SETTING
    #define IBR_NEW     1
    #define IBR_OLD     0
    INT32 Chip_flag = IBR_OLD;
    IBR_BOOT_STRUCT_T_NEW BootCodeMark_new;
#endif

extern INT fmiMarkBadBlock(UINT32 block);
extern INT nvtSMMarkBadBlock_WhileEraseFail(FMI_SM_INFO_T *pSM, UINT32 BlockNo);

extern UINT16 FrameBuffer[];
extern S_DEMO_FONT s_sDemo_Font;
extern INI_INFO_T Ini_Writer;

int font_x=0, font_y=16;
UINT32 u32SkipX;

#if 1
#define dbgprintf sysprintf
#else
#define dbgprintf(...)
#endif

//======================================================
// GNAND used
//======================================================
NDRV_T _nandDiskDriver0 =
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

#if defined (__GNUC__)
    UINT8 infoBufArray[0x40000] __attribute__((aligned (32)));
    UINT8 StorageBufferArray[0x50000] __attribute__((aligned (32)));
    UINT8 CompareBufferArray[0x50000] __attribute__((aligned (32)));
#else
    __align(32) UINT8 infoBufArray[0x40000];
    __align(32) UINT8 StorageBufferArray[0x50000];
    __align(32) UINT8 CompareBufferArray[0x50000];
#endif

UINT32 infoBuf, StorageBuffer, CompareBuffer, BufferSize=0;
UINT32 NAND_BACKUP_BASE;
UINT8 *tmpBackPtr;


CHAR suNvtFullName[512], suNvtTargetFullName[512];
static INT hNvtFile = -1;
INT32 gCurBlock=0, gCurPage=0;
FW_UPDATE_INFO_T FWInfo[3];
int volatile bIsAbort = FALSE;
IBR_BOOT_STRUCT_T NandMark;
INT32 gNandLoaderSize;


/**********************************/
void nvtVerifyNand(UINT32 len)
{
    int volatile page_count, block_count=0, page;
    int volatile i, j;
    UINT8 *ptr = (UINT8 *)CompareBuffer;

    memset((UINT8 *)CompareBuffer, 0xff, len);

    page_count = len / pNvtSM0->nPageSize;
    if ((len % pNvtSM0->nPageSize) != 0)
        page_count++;

    if (gCurPage > 0)
    {
        page = Minimum(pNvtSM0->uPagePerBlock - gCurPage, page_count);
        for (i=0; i<page; i++)
        {
            nvtSMpread(gCurBlock, i+gCurPage, (UINT8 *)ptr);
            ptr += pNvtSM0->nPageSize;
            page_count--;
        }
        if ((gCurPage+page) == pNvtSM0->uPagePerBlock)
        {
            gCurBlock++;
            gCurPage = 0;
        }
        else
            gCurPage += page;
    }

    block_count = page_count / pNvtSM0->uPagePerBlock;

    j=0;
    while(1)
    {
        if (j >= block_count)
            break;
        if (CheckBadBlockMark(gCurBlock) == Successful)
        {
            for (i=0; i<pNvtSM0->uPagePerBlock; i++)
            {
                nvtSMpread(gCurBlock, i, (UINT8 *)ptr);
                ptr += pNvtSM0->nPageSize;
            }
            j++;
        }
        gCurBlock++;
        gCurPage = 0;
    }

    if ((page_count % pNvtSM0->uPagePerBlock) != 0)
    {
        page_count = page_count - block_count * pNvtSM0->uPagePerBlock;
_read_:
        if (CheckBadBlockMark(gCurBlock) == Successful)
        {
            for (i=0; i<page_count; i++)
            {
                nvtSMpread(gCurBlock, i, (UINT8 *)ptr);
                ptr += pNvtSM0->nPageSize;
            }
            gCurPage = i;
        }
        else
        {
            gCurBlock++;
            goto _read_;
        }
    }
}


void nvtBackUpNand(UINT32 pageCount, UINT32 oldBlock)
{
    int volatile i, status=0;

    pInfo = (UINT8 *)((UINT32)infoBuf | 0x80000000);

    // block erase
_erase:
    status = nvtSMblock_erase(gCurBlock);
    if (status < 0)
    {
        fmiMarkBadBlock(gCurBlock);
        gCurBlock++;
        goto _erase;
    }

    for (i=0; i<pageCount; i++)
    {
        nvtSMpread(oldBlock, i, pInfo);
        status = nvtSMpwrite(gCurBlock, i, pInfo);
        if (status < 0)
        {
            fmiMarkBadBlock(gCurBlock);
            gCurBlock++;
            goto _erase;
        }
    }
}


void nvtWriteNand(UINT32 len)
{
    int volatile status = 0;
    int volatile page_count, block_count=0, page;
    int volatile i, j;
    UINT8 *ptr = (UINT8 *)StorageBuffer;

    page_count = len / pNvtSM0->nPageSize;
    if ((len % pNvtSM0->nPageSize) != 0)
        page_count++;

    if (gCurPage > 0)
    {
        page = Minimum(pNvtSM0->uPagePerBlock - gCurPage, page_count);
        for (i=0; i<page; i++)
        {
            status = nvtSMpwrite(gCurBlock, i+gCurPage, (UINT8 *)ptr);
            if (status < 0)
            {
                fmiMarkBadBlock(gCurBlock);
                gCurBlock++;
                nvtBackUpNand(i, gCurBlock-1);
            }
            ptr += pNvtSM0->nPageSize;
            page_count--;
        }
        if ((gCurPage+page) == pNvtSM0->uPagePerBlock)
        {
            gCurPage = 0;
            gCurBlock++;
        }
        else
            gCurPage += page;
    }

    // erase needed blocks
    block_count = page_count / pNvtSM0->uPagePerBlock;

    for (j=0; j<block_count; j++)
    {
        // block erase
_retry_erase:
        status = nvtSMblock_erase(gCurBlock);
        if (status < 0)
        {
            fmiMarkBadBlock(gCurBlock);
            gCurBlock++;
            goto _retry_erase;
        }

        // write block
        for (i=0; i<pNvtSM0->uPagePerBlock; i++)
        {
            status = nvtSMpwrite(gCurBlock, i, (UINT8 *)ptr);
            if (status < 0)
            {
                fmiMarkBadBlock(gCurBlock);
                gCurBlock++;
                nvtBackUpNand(i, gCurBlock-1);
            }
            ptr += pNvtSM0->nPageSize;
        }
        gCurBlock++;
        gCurPage = 0;
    }

    if ((page_count % pNvtSM0->uPagePerBlock) != 0)
    {
        page_count = page_count - block_count * pNvtSM0->uPagePerBlock;
        // erase block
_retry_remain:
        status = nvtSMblock_erase(gCurBlock);
        if (status < 0)
        {
            fmiMarkBadBlock(gCurBlock);
            gCurBlock++;
            goto _retry_remain;
        }

        // write block
        for (i=0; i<page_count; i++)
        {
            status = nvtSMpwrite(gCurBlock, i, (UINT8 *)ptr);
            if (status < 0)
            {
                fmiMarkBadBlock(gCurBlock);
                gCurBlock++;
                nvtBackUpNand(i, gCurBlock-1);
            }
            ptr += pNvtSM0->nPageSize;
        }
        gCurPage = i;
    }
}


/**********************************/
int File_Copy(CHAR *suSrcName, CHAR *suDstName)
{
    INT     hFileSrc, hFileDst, nByteCnt, nStatus;

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
            break;

        nStatus = fsWriteFile(hFileDst, (UINT8 *)StorageBuffer, nByteCnt, &nByteCnt);
        if (nStatus < 0)
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

int nandCopyContent(CHAR *suDirName, CHAR *suTargetName)
{
    INT             nLenS, nLenD, nStatus, nRetry;
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
        dbgprintf("ERROR: No %s Folder, return=0x%08X\n", FolderName, nStatus);
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
                dbgprintf("ERROR: Unable to Create Directory %s, return=0x%08X\n", suDstLongName, nStatus);
                return nStatus;
            }

            nStatus = nandCopyContent(suSrcLongName, suDstLongName);
            if (nStatus < 0)
            {
                sysprintf("===> 22\n");
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
            nRetry = 0;
_retry_filecopy:
            dbgprintf("Copying file %s\n", tFileInfo.szShortName);
            sprintf(Array1, "Copying %s", tFileInfo.szShortName);

            nStatus = File_Copy(suSrcLongName, suDstLongName);
            if (nStatus < 0)
            {
                if (nRetry >= 3)
                {
                    dbgprintf("ERROR: Copying file %s fail, return=0x%08X.\n", tFileInfo.szShortName, nStatus);
                    Draw_CurrentOperation(Array1, nStatus);
                    return nStatus;
                }
                else
                {
                    nRetry ++;
                    dbgprintf("WARNING: Copying file %s fail, return=0x%08X. Retry %d times.\n", tFileInfo.szShortName, nStatus, nRetry);
                    goto _retry_filecopy;
                }
            }
            else
                Draw_CurrentOperation(Array1,nStatus);

            sprintf(Array1, "Comparing %s", tFileInfo.szShortName);
            nStatus = File_Compare(suSrcLongName, suDstLongName);
            if (nStatus < 0)
            {
                if (nRetry >= 3)
                {
                    dbgprintf("ERROR: Comparing file %s fail, return=0x%08X.\n", tFileInfo.szShortName, nStatus);
                    Draw_CurrentOperation(Array1, nStatus);
                    return nStatus;
                }
                else
                {
                    nRetry ++;
                    dbgprintf("WARNING: Comparing file %s fail, return=0x%08X. Retry %d times.\n", tFileInfo.szShortName, nStatus, nRetry);
                    goto _retry_filecopy;
                }
            }

            Draw_CurrentOperation(Array1,nStatus);
        }
    } while (!fsFindNext(&tFileInfo));

    fsFindClose(&tFileInfo);
    return 0;
}


#ifdef SUPPORT_POST_FILE_COMPARE
//--------------------------------------------------------------
// Compare every files in folders suDirName and suTargetName.
// Return: = 0 : folder identical
//         < 0 : folder different and return error code
//--------------------------------------------------------------
int nandCompareContent(CHAR *suDirName, CHAR *suTargetName)
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
        dbgprintf("ERROR: No %s Folder, return=0x%08X\n", FolderName, nStatus);
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

            nStatus = nandCompareContent(suSrcLongName, suDstLongName);
            if (nStatus < 0)
            {
                sysprintf("===> 22\n");
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

            dbgprintf("Comparing file %s ... ", tFileInfo.szShortName, suSrcLongName, suDstLongName);
            sprintf(Array1, "Comparing %s", tFileInfo.szShortName);
            nStatus = File_Compare(suSrcLongName, suDstLongName);
            if (nStatus < 0)
            {
                Draw_CurrentOperation(Array1,nStatus);
                dbgprintf("\nWARNING: Comparing file %s fail, return=0x%08X. Try again.\n", tFileInfo.szShortName, nStatus);
                return nStatus;
            }
            else
                dbgprintf("OK.\n");
            Draw_CurrentOperation(Array1,nStatus);
        }
    } while (!fsFindNext(&tFileInfo));

    fsFindClose(&tFileInfo);
    return 0;
}
#endif

#ifdef SUPPORT_PRE_SCAN_NAND
//-----------------------------------------------------------------
// Pre scan all blocks and mark invalid blocks to "bad block".
// Return: = 0 : scan completed
//         < 0 : found invalid block but fail to mark it to bad block.
//-----------------------------------------------------------------
int preScanNAND(unsigned char pattern)
{
    int block, page;
    int result;
    sysprintf("NAND flash block pre-scanning by pattern 0x%02X ...\n", pattern);
    for (block = 0; block < pNvtSM0->uBlockPerFlash; block++)
    {
        if (fmiNormalCheckBlock(pNvtSM0, block) != 0)
        {
            sysprintf("    Pre Scan: Detect bad block %d. Keep it to bad block.\n", block);
            continue;
        }

        nvtSMblock_erase(block);
        for (page = 0; page < pNvtSM0->uPagePerBlock; page++)
        {
            memset((UINT8 *)StorageBuffer, 0x00, pNvtSM0->nPageSize);
            result = nvtSMpwrite(block, page, (UINT8 *)StorageBuffer);
            if (result < 0)
            {
                sysprintf("    Pre Scan: block %d page %d write fail ! Mark it to BAD BLOCK !\n", block, page);
                nvtSMMarkBadBlock_WhileEraseFail(pNvtSM0, block);
                if (fmiNormalCheckBlock(pNvtSM0, block) == 0)
                {
                    // still is valid block after mark bad block.
                    sysprintf("    ERROR: fail to mark bad block %d !\n", block);
                    return -1;
                }
                break;
            }

            memset((UINT8 *)CompareBuffer, 0xF0, pNvtSM0->nPageSize);
            result = nvtSMpread(block, page, (UINT8 *)CompareBuffer);
            if (result < 0)
            {
                sysprintf("    Pre Scan: block %d page %d read fail ! Mark it to BAD BLOCK !\n", block, page);
                nvtSMMarkBadBlock_WhileEraseFail(pNvtSM0, block);
                if (fmiNormalCheckBlock(pNvtSM0, block) == 0)
                {
                    // still is valid block after mark bad block.
                    sysprintf("    ERROR: fail to mark bad block %d !\n", block);
                    return -1;
                }
                break;
            }

            if (memcmp((UINT8 *)StorageBuffer, (UINT8 *)CompareBuffer, pNvtSM0->nPageSize))
            {
                sysprintf("    Pre Scan: block %d page %d data compare fail ! Mark it to BAD BLOCK !\n", block, page);
                nvtSMMarkBadBlock_WhileEraseFail(pNvtSM0, block);
                if (fmiNormalCheckBlock(pNvtSM0, block) == 0)
                {
                    // still is valid block after mark bad block.
                    sysprintf("    ERROR: fail to mark bad block %d !\n", block);
                    return -1;
                }
                break;
            }
        }
        if (block % 100 == 0)
            sysprintf("    block %d pre-scanning done.\n", block);
    }
    sysprintf("    All %d blocks pre-scanning done.\n", block);
    return 0;
}
#endif


/**********************************/
int main()
{
    DateTime_T ltime;

    NDISK_T *ptNDisk;
    PDISK_T *pDisk_nand = NULL;
    int status=0, nReadLen;
    CHAR szNvtFullName[64];

    LDISK_T *ptLDisk;
    PDISK_T *ptPDisk;
    PARTITION_T *ptPart;
    int LogicSectorD=-1, LogicSectorE=-1, NandFlag, i;

    char Array1[64];
    UINT  Next_Font_Height;
    INT Disk1Size, FileInfoIdx=0;

    UINT32 uBlockSize=0, uFreeSize=0, uDiskSize=0;

#ifdef UPDATE_BOOT_CODE_OPTIONAL_SETTING
    int optional_ini_size = 0;
    extern IBR_BOOT_OPTIONAL_STRUCT_T optional_ini_file;
#endif

    //--- Reset SIC engine to make sure it under normal status.
    outp32(REG_AHBCLK, inp32(REG_AHBCLK) | (SIC_CKE | NAND_CKE | SD_CKE));
    outp32(REG_AHBIPRST, inp32(REG_AHBIPRST) | SICRST);     // SIC engine reset is avtive
    outp32(REG_AHBIPRST, inp32(REG_AHBIPRST) & ~SICRST);    // SIC engine reset is no active. Reset completed.

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
    sysSetTimerReferenceClock(TIMER0, 12000000);
    sysStartTimer(TIMER0, 100, PERIODIC_MODE);

    sysprintf("\n=====> N9H20 NandWriter (v%d.%d) Begin [%d] <=====\n", MAJOR_VERSION_NUM, MINOR_VERSION_NUM, sysGetTicks(0));

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
    fsAssignDriveNumber('X', DISK_TYPE_SD_MMC, 0, 1);           // SD0, single partition
    fsAssignDriveNumber('D', DISK_TYPE_SMART_MEDIA, 0, 1);      // NAND1-1, 2 partitions
    fsAssignDriveNumber('E', DISK_TYPE_SMART_MEDIA, 0, 2);      // NAND1-2, 2 partitions

    Draw_Init();
    font_y = g_Font_Height;
    Next_Font_Height = g_Font_Height-6;

    Draw_Font(COLOR_RGB16_WHITE,  &s_sDemo_Font, font_x, font_y, "Mount SD Card:");
    u32SkipX = 14;

    sicIoctl(SIC_SET_CLOCK, 192000, 0, 0);
    sicOpen();
    status = sicSdOpen0();
    if (status < 0)
    {
        Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
        sysprintf("===> 1 (No SD Card)\n");
        bIsAbort = TRUE;
        goto _end_;
    }

    // Get the NandWriter setting from INI file (NandWriter.ini)
    status = ProcessINI("X:\\NandWriter.ini");
    if (status < 0)
    {
        Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
        sysprintf("===> 1.1 (Wrong INI file)\n");
        bIsAbort = TRUE;
        goto _end_;
    }
    Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Successful);
    font_y += Next_Font_Height;

#ifdef UPDATE_BOOT_CODE_OPTIONAL_SETTING
    if (Chip_flag == IBR_NEW)
    {
        // Get the Boot Code Optional Setting from INI file (TurboWriter.ini) to optional_ini_file
        status = ProcessOptionalINI("X:\\TurboWriter.ini");
        if (status < 0)
        {
            sysprintf("===> 1.2 (Wrong TurboWriter INI file)\n");
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

    nvtSMInit();    // initial NAND controller and pNvtSM0

    sprintf(Array1, "Nand:%d(Blk)*%d(Pg)*%d(Size)",
            pNvtSM0->uBlockPerFlash+1, pNvtSM0->uPagePerBlock, pNvtSM0->nPageSize);
    Draw_Font(COLOR_RGB16_WHITE, &s_sDemo_Font, 0,  _LCM_HEIGHT_ -1-g_Font_Height,Array1);

    BufferSize = pNvtSM0->uPagePerBlock * pNvtSM0->nPageSize;

    tmpBackPtr = malloc(BufferSize+32);
    NAND_BACKUP_BASE = (UINT32) (tmpBackPtr+31) & ~0x1F;
    if (NAND_BACKUP_BASE ==0)
    {
        sysprintf("Malloc fail for size=0x%x\n",BufferSize);
        goto _end_;
    }

    if (BufferSize >= 0x50000)
        BufferSize = 0x40000;

#ifdef SUPPORT_PRE_SCAN_NAND
    // pre scan NAND to mark invliad blocks to "bad block".
    status = preScanNAND(0x5A);
    if (status < 0)
    {
        sysprintf("ERROR: Pre scan fail !\n");
        bIsAbort = -1;  // -1 to show "UMRK" in Draw_FinalStatus()
        goto _end_;
    }
    status = preScanNAND(0xA5);
    if (status < 0)
    {
        sysprintf("ERROR: Pre scan fail !\n");
        bIsAbort = -1;  // -1 to show "UMRK" in Draw_FinalStatus()
        goto _end_;
    }
    status = preScanNAND(0x00);
    if (status < 0)
    {
        sysprintf("ERROR: Pre scan fail !\n");
        bIsAbort = -1;  // -1 to show "UMRK" in Draw_FinalStatus()
        goto _end_;
    }
#endif

    //--- erase NAND chip if need to partition disk for NAND1-1 or NAND1-2.
    if ((Ini_Writer.NAND1_1_FAT >= 0) || (Ini_Writer.NAND1_1_FAT >= 0))
    {
        sysprintf("=====> Erase NAND chip on CS0 [%d] <=====\n", sysGetTicks(0));
        Draw_Font(COLOR_RGB16_WHITE, &s_sDemo_Font, font_x, font_y, "Erase NAND:");
        u32SkipX = 11;
        nvtSMchip_erase(0, pNvtSM0->uBlockPerFlash);
        Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Successful);
        font_y += Next_Font_Height;
    }

    /*******************/
    /* copy NandLoader */
    /*******************/
    if (strlen(Ini_Writer.NandLoader) ==0)
        goto WriteLogo;

    sysprintf("=====> copy and verify nandloader [%d] <=====\n", sysGetTicks(0));

    Draw_Font(COLOR_RGB16_WHITE, &s_sDemo_Font, font_x, font_y, "Writing NandLoader:");
    u32SkipX = 19;

    sprintf(szNvtFullName, "X:\\%s",Ini_Writer.NandLoader);
    fsAsciiToUnicode(szNvtFullName, suNvtFullName, TRUE);
    hNvtFile = fsOpenFile(suNvtFullName, NULL, O_RDONLY);

    sprintf(Array1, "Open %s", szNvtFullName);
    if (hNvtFile < 0)
    {
        Draw_CurrentOperation(Array1,hNvtFile);
        Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
        sysprintf("===> 2.1 open file name [%s], return = 0x%x\n", suNvtFullName, hNvtFile);
        bIsAbort = TRUE;
        goto _end_;
    }
    else
        Draw_CurrentOperation(Array1,hNvtFile);

    /* nand information */
    nIsSysImage = 0x5A;

    FWInfo[FileInfoIdx].imageNo = FileInfoIdx;
    FWInfo[FileInfoIdx].imageFlag = 3;
    FWInfo[FileInfoIdx].startBlock = 0;
    FWInfo[FileInfoIdx].endBlock = 3;
    FWInfo[FileInfoIdx].executeAddr = 0x900000;
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
    gNandLoaderSize = FWInfo[FileInfoIdx].fileLen;
    memcpy(&FWInfo[FileInfoIdx].imageName[0], Ini_Writer.NandLoader, 32);
    gCurPage = 0;
    gCurBlock = 0;

    // initial Boot Code Mark for NandLoader
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

        //--- The gNandLoaderSize MUST include 32 bytes for Boot Code Marker Header
        //    and User Defined Option size.
        gNandLoaderSize += (32 + optional_ini_size);
    }
    else
    {
        NandMark.BootCodeMarker = 0x57425AA5;
#if defined(N9H20K5)
        NandMark.ExeAddr = 0x900000;
#endif
#if defined(N9H20K3)
        NandMark.ExeAddr = 0x700000;
#endif
#if defined(N9H20K1)
        NandMark.ExeAddr = 0x180000;
#endif
        NandMark.ImageSize = FWInfo[FileInfoIdx].fileLen;
        NandMark.SkewMarker = 0xA55A4257;
        NandMark.DQSODS = 0x00001010;
        NandMark.CLKQSDS = 0x00888800;
        NandMark.DRAMMarker = 0x0;      // No DRMA Size Marker
        NandMark.DRAMSize = 0x0;
    }

    //Duplicate NandLoader in block 0 ~ 3
    for (i=0;i<4;i++)
    {
        gCurPage = 0;
        gCurBlock = i;
        NandFlag = 1;

        // ignore bad block
        if (fmiNormalCheckBlock(pNvtSM0, gCurBlock) != 0)
        {
            sysprintf("Detect bad block %d. Ignore it for NandLoader programming.\n", gCurBlock);
            continue;
        }

        fsFileSeek(hNvtFile, 0, SEEK_SET);
        while(1)
        {
            memset((UINT8 *)StorageBuffer, 0xff, BufferSize);
            if (NandFlag)
            {   //Write 1st Page with NAND Marker
#ifdef UPDATE_BOOT_CODE_OPTIONAL_SETTING
                // write 1st sector with Boot Code Mark and optional header for SD
                if (Chip_flag == IBR_NEW)
                {
                    memcpy((UINT8 *)StorageBuffer, (UINT8 *)&BootCodeMark_new, sizeof(IBR_BOOT_STRUCT_T_NEW));
                    memcpy((UINT8 *)(StorageBuffer+sizeof(IBR_BOOT_STRUCT_T_NEW)), (UINT8 *)&optional_ini_file, optional_ini_size);
                    status = fsReadFile(hNvtFile, (UINT8 *)(StorageBuffer+sizeof(IBR_BOOT_STRUCT_T_NEW)+optional_ini_size),
                                        pNvtSM0->nPageSize - sizeof(IBR_BOOT_STRUCT_T_NEW) - optional_ini_size, &nReadLen);
                    nvtWriteNand(nReadLen + sizeof(IBR_BOOT_STRUCT_T_NEW) + optional_ini_size);
                }
                else
                {
                    memcpy((UINT8 *)StorageBuffer, (UINT8 *)&NandMark, sizeof(IBR_BOOT_STRUCT_T));
                    status = fsReadFile(hNvtFile, (UINT8 *)(StorageBuffer+sizeof(IBR_BOOT_STRUCT_T)),
                                        pNvtSM0->nPageSize - sizeof(IBR_BOOT_STRUCT_T), &nReadLen);
                    nvtWriteNand(nReadLen + sizeof(IBR_BOOT_STRUCT_T));
                }
#else
                memcpy((UINT8 *)StorageBuffer, (UINT8 *)&NandMark, sizeof(IBR_BOOT_STRUCT_T));
                status = fsReadFile(hNvtFile, (UINT8 *)(StorageBuffer+sizeof(IBR_BOOT_STRUCT_T)),
                                    pNvtSM0->nPageSize - sizeof(IBR_BOOT_STRUCT_T), &nReadLen);
                nvtWriteNand(nReadLen + sizeof(IBR_BOOT_STRUCT_T));
#endif
                NandFlag = 0;
            }
            else
            {
                status = fsReadFile(hNvtFile, (UINT8 *)StorageBuffer, BufferSize, &nReadLen);
                nvtWriteNand(nReadLen);
            }
            if (status == ERR_FILE_EOF)
                break;
            else if (status < 0)
            {
                Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
                sysprintf("===> 2.3 fail to read NandLoader file\n");
                bIsAbort = TRUE;
                goto _end_;
            }
        }

        /* verify NandLoader */
        gCurPage = 0;
        gCurBlock = i;
        fsFileSeek(hNvtFile, 0, SEEK_SET);

        NandFlag = 1;

        while(1)
        {
            memset((UINT8 *)StorageBuffer, 0xff, BufferSize);
            if (NandFlag)
            {
#ifdef UPDATE_BOOT_CODE_OPTIONAL_SETTING
                // verify 1st sector with Boot Code Mark and optional header for SD
                if (Chip_flag == IBR_NEW)
                {
                    memcpy((UINT8 *)StorageBuffer, (UINT8 *)&BootCodeMark_new, sizeof(IBR_BOOT_STRUCT_T_NEW));
                    memcpy((UINT8 *)(StorageBuffer+sizeof(IBR_BOOT_STRUCT_T_NEW)), (UINT8 *)&optional_ini_file, optional_ini_size);
                    status = fsReadFile(hNvtFile, (UINT8 *)(StorageBuffer+sizeof(IBR_BOOT_STRUCT_T_NEW)+optional_ini_size),
                                        pNvtSM0->nPageSize - sizeof(IBR_BOOT_STRUCT_T_NEW) - optional_ini_size, &nReadLen);
                    nvtVerifyNand(nReadLen + sizeof(IBR_BOOT_STRUCT_T_NEW) + optional_ini_size);
                }
                else
                {
                    memcpy((UINT8 *)StorageBuffer, (UINT8 *)&NandMark, sizeof(IBR_BOOT_STRUCT_T));
                    memcpy((UINT8 *)(StorageBuffer+sizeof(IBR_BOOT_STRUCT_T)), (UINT8 *)&optional_ini_file, optional_ini_size);
                    status = fsReadFile(hNvtFile, (UINT8 *)(StorageBuffer+sizeof(IBR_BOOT_STRUCT_T)+optional_ini_size),
                                        pNvtSM0->nPageSize - sizeof(IBR_BOOT_STRUCT_T) - optional_ini_size, &nReadLen);
                    nvtVerifyNand(nReadLen + sizeof(IBR_BOOT_STRUCT_T) + optional_ini_size);
                }
#else
                memcpy((UINT8 *)StorageBuffer, (UINT8 *)&NandMark, sizeof(IBR_BOOT_STRUCT_T));
                status = fsReadFile(hNvtFile, (UINT8 *)(StorageBuffer+sizeof(IBR_BOOT_STRUCT_T)),
                                    pNvtSM0->nPageSize - sizeof(IBR_BOOT_STRUCT_T), &nReadLen);
                nvtVerifyNand(nReadLen+sizeof(IBR_BOOT_STRUCT_T));
#endif
                NandFlag = 0;
            }
            else
            {
                status = fsReadFile(hNvtFile, (UINT8 *)StorageBuffer, BufferSize, &nReadLen);
                nvtVerifyNand(nReadLen);
            }

            if (memcmp((UINT8 *)StorageBuffer, (UINT8 *)CompareBuffer, nReadLen))
            {
                Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
                sysprintf("===> 2.4 fail to verify NandLoader file\n");
                bIsAbort = TRUE;
                goto _end_;
            }

            if (status == ERR_FILE_EOF)
                break;
            else if (status < 0)
            {
                Draw_Status(font_x+ 20*g_Font_Step, font_y, Fail);
                sysprintf("===> 2.5 fail to read NandLoader file\n");
                bIsAbort = TRUE;
                goto _end_;
            }
        }
    }

    Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Successful);
    font_y += Next_Font_Height;

    fsCloseFile(hNvtFile);
    hNvtFile = -1;

    FileInfoIdx++;

    nIsSysImage = 0xFF;

    /*************/
    /* copy logo */
    /*************/
WriteLogo:
    if (strlen(Ini_Writer.Logo) ==0)
        goto WriteNVTLoader;

    sysprintf("=====> copy and verify logo [%d] <=====\n", sysGetTicks(0));

    Draw_Font(COLOR_RGB16_WHITE, &s_sDemo_Font, font_x, font_y, "Writing Logo:");
    u32SkipX = 12;

    sprintf(szNvtFullName, "X:\\%s",Ini_Writer.Logo);
    fsAsciiToUnicode(szNvtFullName, suNvtFullName, TRUE);
    hNvtFile = fsOpenFile(suNvtFullName, NULL, O_RDONLY);

    sprintf(Array1, "Open %s", szNvtFullName);
    if (hNvtFile < 0)
    {
        Draw_CurrentOperation(Array1,hNvtFile);
        Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
        sysprintf("===> 3.1 open file name [%s], return = 0x%x\n", suNvtFullName, hNvtFile);
        bIsAbort = TRUE;
        goto _end_;
    }
    else
        Draw_CurrentOperation(Array1,hNvtFile);

    /* nand information */
    FWInfo[FileInfoIdx].imageNo = FileInfoIdx;
    FWInfo[FileInfoIdx].imageFlag = 4;
    FWInfo[FileInfoIdx].startBlock = FWInfo[FileInfoIdx-1].endBlock + 1;
#if defined(N9H20K5) || defined(N9H20K3)
    FWInfo[FileInfoIdx].executeAddr = 0x500000;
#endif
#if defined(N9H20K1)
    FWInfo[FileInfoIdx].executeAddr = 0x190000; //Although didn't support logo in 2MB solution. But we still defined it.
#endif
    FWInfo[FileInfoIdx].fileLen = (UINT32)fsGetFileSize(hNvtFile);
    memcpy(&FWInfo[FileInfoIdx].imageName[0], Ini_Writer.Logo, 32);
    gCurPage = 0;
    gCurBlock = 4;

    while(1)
    {
        status = fsReadFile(hNvtFile, (UINT8 *)StorageBuffer, BufferSize, &nReadLen);
        nvtWriteNand(nReadLen);
        if (status == ERR_FILE_EOF)
            break;
        else if (status < 0)
        {
            Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
            sysprintf("===> 3.2 fail to read Logo file\n");
            bIsAbort = TRUE;
            goto _end_;
        }
    }

    if (gCurPage == 0)
        FWInfo[FileInfoIdx].endBlock = gCurBlock - 1;
    else
        FWInfo[FileInfoIdx].endBlock = gCurBlock;

    /* verify logo */
    gCurPage = 0;
    gCurBlock = 4;
    fsFileSeek(hNvtFile, 0, SEEK_SET);

    while(1)
    {
        status = fsReadFile(hNvtFile, (UINT8 *)StorageBuffer, BufferSize, &nReadLen);
        nvtVerifyNand(nReadLen);
        if (memcmp((UINT8 *)StorageBuffer, (UINT8 *)CompareBuffer, nReadLen))
        {
            Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
            sysprintf("===> 3.3 fail to verify Logo file\n");
            bIsAbort = TRUE;
            goto _end_;
        }
        if (status == ERR_FILE_EOF)
            break;
        else if (status < 0)
        {
            Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
            sysprintf("===> 3.4 fail to read Logo file\n");
            bIsAbort = TRUE;
            goto _end_;
        }
    }
    Draw_Status(font_x+ 14*g_Font_Step, font_y, Successful);
    font_y += Next_Font_Height;

    fsCloseFile(hNvtFile);
    hNvtFile = -1;

    FileInfoIdx++;

    /******************/
    /* copy nvtloader */
    /******************/
WriteNVTLoader:

    if (strlen(Ini_Writer.NVTLoader) ==0)
        goto WriteSysteInfo;

    sysprintf("=====> copy and verify nvtloader [%d] <=====\n", sysGetTicks(0));

    Draw_Font(COLOR_RGB16_WHITE, &s_sDemo_Font, font_x, font_y, "Writing NvtLoader:");
    u32SkipX = 18;

    sprintf(szNvtFullName, "X:\\%s",Ini_Writer.NVTLoader);
    fsAsciiToUnicode(szNvtFullName, suNvtFullName, TRUE);
    hNvtFile = fsOpenFile(suNvtFullName, NULL, O_RDONLY);

    sprintf(Array1, "Open %s", szNvtFullName);
    if (hNvtFile < 0)
    {
        Draw_CurrentOperation(Array1,hNvtFile);
        Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
        sysprintf("===> 4.1 open file name [%s], return = 0x%x\n", suNvtFullName, hNvtFile);
        bIsAbort = TRUE;
        goto _end_;
    }
    else
        Draw_CurrentOperation(Array1,hNvtFile);

    /* nand information */
    FWInfo[FileInfoIdx].imageNo = FileInfoIdx;
    FWInfo[FileInfoIdx].imageFlag = 1;
    FWInfo[FileInfoIdx].startBlock = FWInfo[FileInfoIdx-1].endBlock + 1;
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
    memcpy(&FWInfo[FileInfoIdx].imageName[0], Ini_Writer.NVTLoader, 32);
    gCurPage = 0;
    gCurBlock = FWInfo[FileInfoIdx].startBlock;
    while(1)
    {
        status = fsReadFile(hNvtFile, (UINT8 *)StorageBuffer, BufferSize, &nReadLen);
        nvtWriteNand(nReadLen);
        if (status == ERR_FILE_EOF)
            break;
        else if (status < 0)
        {
            Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
            sysprintf("===> 4.2 fail to read NvtLoader file\n");
            bIsAbort = TRUE;
            goto _end_;
        }
    }

    if (gCurPage == 0)
        FWInfo[FileInfoIdx].endBlock = gCurBlock - 1;
    else
        FWInfo[FileInfoIdx].endBlock = gCurBlock;

    /* verify nvtloader */
    gCurPage = 0;
    gCurBlock = FWInfo[FileInfoIdx].startBlock;
    fsFileSeek(hNvtFile, 0, SEEK_SET);
    while(1)
    {
        status = fsReadFile(hNvtFile, (UINT8 *)StorageBuffer, BufferSize, &nReadLen);
        nvtVerifyNand(nReadLen);

        if (memcmp((UINT8 *)StorageBuffer, (UINT8 *)CompareBuffer, nReadLen))
        {
            Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
            sysprintf("===> 4.3 fail to verify NvtLoader file\n");
            bIsAbort = TRUE;
            goto _end_;
        }

        if (status == ERR_FILE_EOF)
            break;
        else if (status < 0)
        {
            Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
            sysprintf("===> 4.4 fail to read NvtLoader file\n");
            bIsAbort = TRUE;
            goto _end_;
        }
    }

    Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Successful);
    font_y += Next_Font_Height;

    fsCloseFile(hNvtFile);

    hNvtFile = -1;

WriteSysteInfo:
    /* set information to block 0 @(last page -1) */
    {
        unsigned int *ptr;
        pInfo = (UINT8 *)((UINT32)infoBuf | 0x80000000);
        ptr = (unsigned int *)((UINT32)infoBuf | 0x80000000);

        memset(pInfo, 0xff,  pNvtSM0->nPageSize);
        /* update image information */
        *(ptr+0) = 0x574255AA;
        *(ptr+1) = FileInfoIdx+1;
        *(ptr+3) = 0x57425963;
        memcpy(pInfo+16, (char *)&FWInfo, pNvtSM0->nPageSize);
        nvtSMpwrite(0, pNvtSM0->uPagePerBlock - 2, pInfo);

        /* Verify information */
        nvtSMpread(0, pNvtSM0->uPagePerBlock - 2, (UINT8 *)CompareBuffer);
        if (memcmp((UINT8 *)pInfo, (UINT8 *)CompareBuffer, 112))
        {
            sysprintf("===> 4.5 fail to verify System Information\n");
            bIsAbort = TRUE;
            goto _end_;
        }
    }

    /*******************************************************/
    /* set system reserved area - 8MB                      */
    /* 4K page must >=2MB if NVTloader + Logo is necessary */
    /*******************************************************/
    if (Ini_Writer.SystemReservedMegaByte < 0)
        Ini_Writer.SystemReservedMegaByte = 8;  // default is 8MBytes for system area
    sysprintf("System Reserved Size = %d MByte\n", Ini_Writer.SystemReservedMegaByte);

    //--- check the System Reserved area size is enough or not ?
    if ((FWInfo[FileInfoIdx].endBlock+1) * pNvtSM0->nPageSize * pNvtSM0->uPagePerBlock >
        Ini_Writer.SystemReservedMegaByte * 1024 * 1024)
    {
        sysprintf("%d system image need %d reserved blocks (%d KByte).\n", FileInfoIdx+1, FWInfo[FileInfoIdx].endBlock+1,
            (FWInfo[FileInfoIdx].endBlock+1) * pNvtSM0->nPageSize * pNvtSM0->uPagePerBlock / 1024);
        sysprintf("ERROR: Partition fail since System Reserved Size too small !!\n");
        status = -1;
        Draw_CurrentOperation("Partition",status);
        Draw_Status(font_x+ 12*g_Font_Step, font_y, Fail);
        goto _end_;
    }

    setNandPartition(Ini_Writer.SystemReservedMegaByte);

    fmiSMClose(0);

    //--- goto end if don't need to program both NAND1-1 and NAND1-2.
    if ((Ini_Writer.NAND1_1_FAT == FAT_MODE_SKIP) || (Ini_Writer.NAND1_1_FAT == FAT_MODE_SKIP))
        goto _end_;

    Draw_Font(COLOR_RGB16_WHITE, &s_sDemo_Font, font_x, font_y, "Mount GNAND:");
    u32SkipX = 12;
    sysprintf("\n=====> Create GNAND for NAND on CS0 [%d] <=====\n", sysGetTicks(0));
    ptNDisk = (NDISK_T *)malloc(sizeof(NDISK_T));
    GNAND_InitNAND(&_nandDiskDriver0, ptNDisk, TRUE);
    status = GNAND_MountNandDisk(ptNDisk);
    if (status)
    {
        Draw_CurrentOperation("Mount GNAND",status);
        Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
    }
    else
        Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Successful);
    font_y += Next_Font_Height;

    pDisk_nand = (PDISK_T *)ptNDisk->pDisk;

    /* partition and format Nand1-1 and Nand1-2 */
    Draw_Font(COLOR_RGB16_WHITE, &s_sDemo_Font, font_x, font_y, "Format Nand:");
    u32SkipX = 12;

    // Partition all disk to two partitions
    if ((Ini_Writer.NAND1_1_SIZE < pDisk_nand->uDiskSize/1024) && (Ini_Writer.NAND1_1_SIZE > 0))
        Disk1Size = Ini_Writer.NAND1_1_SIZE;
    else
        Disk1Size = 16;     // default size for 1st partition is 16MB
    sysprintf("Nand1-1 Partition Size = %d\n", Disk1Size);

    sysprintf("=====> partition and format [%d] <=====\n", sysGetTicks(0));
    // 2011/11/3, partition 2 size should subtract system reserved area size
    fsSetReservedArea(Ini_Writer.SystemReservedMegaByte*1024*2);    // set reserved sector number for fsTwoPartAndFormatAll()
    status = fsTwoPartAndFormatAll((PDISK_T *)pDisk_nand, Disk1Size*1024,
                                   pDisk_nand->uDiskSize - (Disk1Size + Ini_Writer.SystemReservedMegaByte)*1024);

    if (status < 0)
    {
        Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
        sysprintf("===> 5 (Format NAND fail)\n");
        bIsAbort = TRUE;
        goto _end_;
    }
    else
    {
        Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Successful);
        fsSetVolumeLabel('D', "NAND1-1\n", strlen("NAND1-1"));
        fsSetVolumeLabel('E', "NAND1-2\n", strlen("NAND1-2"));
    }
    font_y += Next_Font_Height;

    // Get disk information
    uBlockSize=0, uFreeSize=0, uDiskSize=0;
    fsDiskFreeSpace('X', &uBlockSize, &uFreeSize, &uDiskSize);
    sysprintf("\nDisk X Size: %d Kbytes, Free Space: %d KBytes\n", (INT)uDiskSize, (INT)uFreeSize);

    uBlockSize=0, uFreeSize=0, uDiskSize=0;
    fsDiskFreeSpace('D', &uBlockSize, &uFreeSize, &uDiskSize);
    sysprintf("Disk D Size: %d Kbytes, Free Space: %d KBytes\n", (INT)uDiskSize, (INT)uFreeSize);

    uBlockSize=0, uFreeSize=0, uDiskSize=0;
    fsDiskFreeSpace('E', &uBlockSize, &uFreeSize, &uDiskSize);
    sysprintf("Disk E Size: %d Kbytes, Free Space: %d KBytes\n", (INT)uDiskSize, (INT)uFreeSize);

    // Get the Start sector for D & E partition
    ptLDisk = (LDISK_T *)malloc(sizeof(LDISK_T));
    if (get_vdisk('D', &ptLDisk) <0)
    {
        sysprintf(" ===> 6 (vdisk fail)\n");
        bIsAbort = TRUE;
        goto _end_;
    }

    ptPDisk = ptLDisk->ptPDisk; // get the physical disk structure pointer of NAND disk
    ptPart = ptPDisk->ptPartList;   // Get the partition of NAND disk
    while (ptPart != NULL)
    {
        sysprintf("Driver %c -- Start sector : %d, Total sector : %d\n",
                ptPart->ptLDisk->nDriveNo, ptPart->uStartSecN, ptPart->uTotalSecN);
        if  (ptPart->ptLDisk->nDriveNo == 'D')
        {
            LogicSectorD = ptPart->uStartSecN;
        }

        if  (ptPart->ptLDisk->nDriveNo == 'E')
        {
            LogicSectorE = ptPart->uStartSecN;
        }
        ptPart = ptPart->ptNextPart;
    }

    /*********************************/
    /* copy first partition content  */
    /*********************************/
    if (Ini_Writer.NAND1_1_FAT >= 0)
    {
        sysprintf("\n=====> copy First Partition Content [%d] <=====\n", sysGetTicks(0));

        if (Ini_Writer.NAND1_1_FAT == 0)
        {
            // Copy File Through FAT like Binary ISO
            Draw_Font(COLOR_RGB16_WHITE, &s_sDemo_Font, font_x, font_y, "Copying NAND1-1:");
            u32SkipX = 16;

            if (LogicSectorD == -1)
            {
                Draw_CurrentOperation("LogicSectorD :",hNvtFile);
                sysprintf("===> 7.1 (Wrong start sector for NAND1-1)\n");
                bIsAbort = TRUE;
                goto _end_;
            }

            strcpy(szNvtFullName, "X:\\NAND1-1\\content.bin");
            fsAsciiToUnicode(szNvtFullName, suNvtFullName, TRUE);
            hNvtFile = fsOpenFile(suNvtFullName, NULL, O_RDONLY);
            sysprintf("Copying file %s\n", szNvtFullName);
            sprintf(Array1, "Open %s", szNvtFullName);
            if (hNvtFile < 0)
            {
                Draw_CurrentOperation("Open X:\\NAND1-1\\content.bin",hNvtFile);
                Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
                sysprintf("===> 7.2 (Open content.bin fail)\n");
                bIsAbort = TRUE;
                goto _end_;
            }
            else
                Draw_CurrentOperation(Array1,hNvtFile);

            while(1)
            {
                sysprintf(".");
                Draw_Wait_Status(font_x+ u32SkipX*g_Font_Step, font_y);
                status = fsReadFile(hNvtFile, (UINT8 *)StorageBuffer, BufferSize, &nReadLen);
                GNAND_write(ptNDisk, LogicSectorD,nReadLen/512 ,(UINT8 *)StorageBuffer);

                // add for image verify
                GNAND_read(ptNDisk, LogicSectorD, nReadLen/512, (UINT8 *)CompareBuffer);
                if (memcmp((UINT8 *)StorageBuffer, (UINT8 *)CompareBuffer, nReadLen))
                {
                    Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
                    sysprintf("===> 7.5 (Verify NAND1-1 image fail)\n");
                    bIsAbort = TRUE;
                    goto _end_;
                }

                LogicSectorD += nReadLen/512;

                if (status == ERR_FILE_EOF)
                    break;
                else if (status < 0)
                {
                    Draw_Clear_Wait_Status(font_x+ u32SkipX*g_Font_Step, font_y);
                    Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
                    sysprintf("\n===> 7.3 (Read content.bin fail) [0x%x]\n", status);
                    bIsAbort = TRUE;
                    goto _end_;
                }
            }
            sysprintf("\n");
            Draw_Clear_Wait_Status(font_x+ u32SkipX*g_Font_Step, font_y);
            Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Successful);
            font_y += Next_Font_Height;
        }
        else
        {
            // Copy File through FAT
            Draw_Font(COLOR_RGB16_WHITE, &s_sDemo_Font, font_x, font_y, "Copying NAND1-1:");
            u32SkipX = 16;

            strcpy(szNvtFullName, "D:");
            fsAsciiToUnicode(szNvtFullName, suNvtTargetFullName, TRUE);

            strcpy(szNvtFullName, "X:\\NAND1-1");
            fsAsciiToUnicode(szNvtFullName, suNvtFullName, TRUE);

            status = nandCopyContent(suNvtFullName, suNvtTargetFullName);
            Draw_Clear_Wait_Status(font_x+ u32SkipX*g_Font_Step, font_y);
            if (status < 0)
            {
                Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
                sysprintf("===> 7.4 (Copy files in NAND1-1 fail) [0x%x]\n", status);
                bIsAbort = TRUE;
                goto _end_;
            }
            Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Successful);
            font_y += Next_Font_Height;
        }
    }

    /**********************************/
    /* copy Second Partition content  */
    /**********************************/
    if (Ini_Writer.NAND1_2_FAT >= 0)
    {
        sysprintf("=====> Copy Second Partition Content [%d] <=====\n", sysGetTicks(0));

        if (Ini_Writer.NAND1_2_FAT == 0)
        {
            // Copy File Through FAT like Binary ISO
            Draw_Font(COLOR_RGB16_WHITE, &s_sDemo_Font, font_x, font_y, "Copying NAND1-2:");
            u32SkipX = 16;

            if (LogicSectorE == -1)
            {
                Draw_CurrentOperation("LogicSectorE:",hNvtFile);

                Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
                sysprintf("===> 8.1 (Wrong start sector for NAND1-2)\n");
                bIsAbort = TRUE;
                goto _end_;
            }

            strcpy(szNvtFullName, "X:\\NAND1-2\\content.bin");
            fsAsciiToUnicode(szNvtFullName, suNvtFullName, TRUE);

            hNvtFile = fsOpenFile(suNvtFullName, NULL, O_RDONLY);

            sprintf(Array1, "Open %s", szNvtFullName);
            if (hNvtFile < 0)
            {
                Draw_CurrentOperation("Open X:\\NAND1-2\\content.bin",hNvtFile);
                Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
                sysprintf("===> 8.2 (Open content.bin fail)\n");
                bIsAbort = TRUE;
                goto _end_;
            }
            else
                Draw_CurrentOperation(Array1,hNvtFile);

            while(1)
            {
                sysprintf(".");
                Draw_Wait_Status(font_x+ u32SkipX*g_Font_Step, font_y);
                status = fsReadFile(hNvtFile, (UINT8 *)StorageBuffer, BufferSize, &nReadLen);
                GNAND_write(ptNDisk, LogicSectorE,nReadLen/512 ,(UINT8 *)StorageBuffer);

                // add for image verify
                GNAND_read(ptNDisk, LogicSectorE, nReadLen/512, (UINT8 *)CompareBuffer);
                if (memcmp((UINT8 *)StorageBuffer, (UINT8 *)CompareBuffer, nReadLen))
                {
                    Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
                    sysprintf("===> 8.6 (Verify NAND1-2 image fail)\n");
                    bIsAbort = TRUE;
                    goto _end_;
                }

                LogicSectorE += nReadLen/512;

                if (status == ERR_FILE_EOF)
                    break;
                else if (status < 0)
                {
                    Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
                    sysprintf("\n===> 8.3 (Read content.bin fail) [0x%x]\n", status);
                    bIsAbort = TRUE;
                    goto _end_;
                }
            }
            sysprintf("\n");
            Draw_Clear_Wait_Status(font_x+ u32SkipX*g_Font_Step, font_y);
            Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Successful);
            font_y += Next_Font_Height;

#ifdef SUPPORT_POST_FILE_COMPARE
            sysprintf(" Disk image copy completed. Compare all files one by one ...\n");
            // compare files after program disk image
            GNAND_UnMountNandDisk(ptNDisk);
            fmiSMClose(0);
            GNAND_InitNAND(&_nandDiskDriver0, ptNDisk, TRUE);
            GNAND_MountNandDisk(ptNDisk);
            pDisk_nand = (PDISK_T *)ptNDisk->pDisk;

            strcpy(szNvtFullName, "E:");
            fsAsciiToUnicode(szNvtFullName, suNvtTargetFullName, TRUE);
            strcpy(szNvtFullName, "X:\\NAND1-2G");
            fsAsciiToUnicode(szNvtFullName, suNvtFullName, TRUE);

            status = nandCompareContent(suNvtFullName, suNvtTargetFullName);
            Draw_Clear_Wait_Status(font_x+ u32SkipX*g_Font_Step, font_y);
            if (status < 0)
            {
                Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
                sysprintf("===> 8.5 (Compare files in NAND1-2G fail) [0x%x]\n", status);
                bIsAbort = TRUE;
                goto _end_;
            }
            sysprintf("Compare disk image with golden files successful !!\n");
            Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Successful);
            font_y += Next_Font_Height;
#endif
        }
        else
        {
            // Copy File through FAT
            Draw_Font(COLOR_RGB16_WHITE, &s_sDemo_Font, font_x, font_y, "Copying NAND1-2:");
            u32SkipX = 16;

#if 1   // write partition 2 to root folder
            strcpy(szNvtFullName, "E:");
            fsAsciiToUnicode(szNvtFullName, suNvtTargetFullName, TRUE);
            strcpy(szNvtFullName, "X:\\NAND1-2");
            fsAsciiToUnicode(szNvtFullName, suNvtFullName, TRUE);
#else   // write partition 2 to Data folder
            strcpy(szNvtFullName, "E:\\Data");
            fsAsciiToUnicode(szNvtFullName, suNvtTargetFullName, TRUE);
            status = fsMakeDirectory(suNvtTargetFullName, NULL);
            if (status < 0)
            {
                Draw_CurrentOperation("Create E:\\Data",status);
                Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
                sysprintf("===> 8.4 (Create folder in NAND1-2 fail) [0x%x]\n", status);
                bIsAbort = TRUE;
                goto _end_;
            }
            else
                Draw_CurrentOperation("Create E:\\Data",status);

            strcpy(szNvtFullName, "X:\\NAND1-2");
            fsAsciiToUnicode(szNvtFullName, suNvtFullName, TRUE);
#endif

            status = nandCopyContent(suNvtFullName, suNvtTargetFullName);
            Draw_Clear_Wait_Status(font_x+ u32SkipX*g_Font_Step, font_y);
            if (status < 0)
            {
                Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
                sysprintf("===> 8.5 (Copy files in NAND1-2 fail) [0x%x]\n", status);
                bIsAbort = TRUE;
                goto _end_;
            }
            Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Successful);
            font_y += Next_Font_Height;
        }
    }


_end_:

    sysprintf("\n=====> Finish [%d] <=====\n", sysGetTicks(0));
    Draw_FinalStatus(bIsAbort);

    if (tmpBackPtr != NULL)
        free(tmpBackPtr);

    while(1);
}
