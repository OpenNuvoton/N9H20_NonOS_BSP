/*************************************************************************
 * Nuvoton Electronics Corporation confidential
 *
 * Copyright (c) 2008 by Nuvoton Electronics Corporation
 * All rights reserved
 * 
 * FILENAME
 *     Demo_CommandShell.c
 *
 * VERSION
 *     1.0
 *
 * DESCRIPTION
 *     Command shell like demo program
 *
 * HISTORY
 *     2008.06.25		Created
 *
 * REMARK
 *     None
 **************************************************************************/

#ifdef ECOS 
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "drv_api.h"
#include "wbtypes.h"
#include "wbio.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "N9H20.h"
#endif


#define SYSTEM_CLOCK     	27000000
#define UART_BAUD_RATE		115200
//#define RESERVED_SIZE		(0)//(10240*1024)		//10MB
#define RESERVED_SIZE		(10240*1024)			//10MB

#ifdef ENABLE_GNAND
static NDISK_T ptNDisk;
#endif
// select use UART output or MULTI-ICE 
#undef getchar
#ifdef ECOS
#define getchar		diag_getc
#define printf		diag_printf
#else
#define getchar		sysGetChar
#define printf		sysprintf
#endif

#ifdef ECOS
#define get_timer_ticks()   cyg_current_time()
#define sysprintf	diag_printf
#else
#define get_timer_ticks()	sysGetTicks(TIMER0)
#endif

#ifdef ENABLE_GNAND
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
#endif


UINT32 g_u32TotalSize;
extern void  fsGetErrorDescription(INT nErrCode, CHAR *szDescription, INT bIsPrint);
extern void  FAT_dump_sector_cache(void);

#ifdef ENABLE_RAM
extern INT  InitRAMDisk(UINT32 uStartAddr, UINT32 uDiskSize);
extern INT32 RemoveRAMDisk(void);
extern void FormatRamDisk(void);
#define RAM_DISK_SIZE 	((1024*1024))		/* Fit for N9H20K1/N9H20K3/N9H20K5 */
#endif

/* imported from WBFILE_DISK.C */
//extern PDISK_T		*_fs_ptPDiskList;

#define DUMMY_BUFFER_SIZE		(64 * 1024)


#if defined(__GNUC__)
#ifdef ENABLE_RAM
INT8 i8RamDisk[RAM_DISK_SIZE] __attribute__((aligned (32)));
#endif
static UINT8  _pucDummy[DUMMY_BUFFER_SIZE] __attribute__((aligned (32)));
#else
#ifdef ENABLE_RAM
__align(32) INT8 i8RamDisk[RAM_DISK_SIZE];
#endif
__align(32) static UINT8  _pucDummy[DUMMY_BUFFER_SIZE];
#endif


static CHAR *_pcFileCommads[] = 
{
	"?",    "HELP", "DIR",    "TYPE",  "CD",    "DEL", 
	"COPY", "MKDIR","MD",     "RMDIR", "RD",	"RENAME",
	"CMP",	"DS", 	"RDTST",  "WRTST", "APTST", "RAWRD",
	"DISK",	"DF", 	"FORMAT", "DELA",  "DIRS",
	"RAWWD",
	NULL
};

enum  EnumerateCommandList 
{
	QQ=0,   HELP,   DIR,    TYPE,   CD,     DEL,
	COPY,   MKDIR,  MD,     RMDIR,  RD,		RENAME,
	CMP,	DUMP,	RDTST,  WRTST,	APTST,	RAWRD,
	DISK,	DF,		FORMAT, DELA,   DIRS,	RAWWD,	
	CHANGE
};

static CHAR _szCommandLine[512];

static CHAR _pszCommandShellHelp[] = 
{
	"WBFile Command Shell Command List -\n"
	"    <HELP/?>    command help\n"
	"    <DIR>       list files under the current directory\n"
	"    <TYPE>      print file contents on console\n"
	"    <CD>        change directory\n"
	"    <DEL>       delete a file\n"
	"    <COPY>      copy a file\n"
	"    <MKDIR/MD>  build an empty directory\n"
	"    <RMDIR/RD>  remove an empty directory\n"
	"    <RENAME>    rename a file\n"
	"    <CMP>       compare two file\n"
	"    <DS>        dump current disk sector\n"
	"    <RDTST>     read speed test\n"
	"    <WRTST>     write speed test\n"
	"    <APTST>     file append test\n"
	"    <DISK>      print disk information\n"
	"    <x:>        change working drive, where x is the drive character\n"
	"\n\n"
};


static INT   _nCurrentDrive;
static CHAR  _pszWorkingDirectory[24][300] =
{
	"C:",
	"D:",
	"E:",
	"F:",
	"G:",
	"H:",
	"I:",
	"J:",
	"K:",
	"L:",
	"M:",
	"N:",
	"O:",
	"P:",
	"Q:",
	"R:",
	"S:",
	"T:",
	"U:",
	"V:",
	"W:",
	"X:",
	"Y:",
	"Z:",
};

static volatile BOOL _bIsCardInserted = FALSE;

#if defined(ENABLE_SD_ONE_PART)||defined(ENABLE_SD_TWO_PART)||defined(ENABLE_SD_FOUR_PART)
void sd_removed(VOID *ptr)
{
    PDISK_T	 *ptPDisk = (PDISK_T *)ptr;
	sysprintf("\nSD card removed! (%d)\n", (INT)ptPDisk);
	fsPhysicalDiskDisconnected(ptPDisk);
}


void sd_inserted()
{
	sysprintf("\nSD card instered!\n");
	_bIsCardInserted = TRUE;
}
#endif



static INT  get_command_code(CHAR *szCmd)
{
	INT     nIdx = 0;
	
	if ((szCmd[1] == ':') && (szCmd[2] == 0))
		return CHANGE;		/* change drive */
	
	while (1)
	{
		if (_pcFileCommads[nIdx] == NULL)
			return -1;      /* command not found */
					
		if (!strcmp(_pcFileCommads[nIdx], szCmd))
			return nIdx;
			
		nIdx++;
	}
}


static CHAR  *get_token(CHAR *szString, CHAR *pcSepCharList, INT nMaxLen)
{
	CHAR    *pcParsePtr = szString;
	CHAR    *pcPtr; 
	INT     nLen = 0;

	if (pcParsePtr == NULL)
		return NULL;
	
	while (*pcParsePtr)
	{
		pcPtr = pcSepCharList;
		while (*pcPtr)
		{
			if (*pcParsePtr == *pcPtr)
			{
				*pcParsePtr++ = 0;
				return pcParsePtr;
			}
			pcPtr++;
		}
		pcParsePtr++;
		if (++nLen >= nMaxLen)
			return NULL;        /* string too long!! */
	}
	
	return pcParsePtr;        /* reach end of the input string */
}



static VOID  accept_string(CHAR *pcString, INT nLength)
{
	CHAR    chr;
	INT		nCount;

	nCount = 0;

	while (1)
	{
		chr = getchar();
		if (chr == 0)               /* control character pressed */
		{
			chr = getchar();
			if (chr == 0x4b)        /* left arrow key */
			{
				if (nCount > 0)      /* have characters in pcString */
				{
					nCount--;        /* delete a character */
					sysprintf("%c%c%c", 0x08, 0x20, 0x08);
				}
			}
		}
		else if (chr == 0x08)       /* backspace */
		{
			if (nCount > 0)          /* have characters in pcString */
			{
				nCount--;            /* delete a character */
				sysprintf("%c%c%c", 0x08, 0x20, 0x08);
			}
		}
		else if (chr == 10)
		{
			;
		}
		else
		{
			if ((chr == 0xd) || (chr == '!'))
			{
				if (nCount == 0)
					sysprintf("\n");
				pcString[nCount] = 0;
				//return;
				break;
			}
		  
			sysprintf("%c", chr);
			pcString[nCount] = chr;            // read in a character
			nCount++;
			if (nCount >= nLength)
			{
				nCount--;
				sysprintf("%c", 0x08);            // backspace
			}
		}
	}
}





INT  fsFourPartAndFormatAll(PDISK_T *ptPDisk, INT firstPartSize, INT secondPartSize,
						INT threePartSize, INT fourPartSize)
{
	PARTITION_T *ptPartition;
	UINT32		uSizeList[5];
	INT			nStatus = 0;

	/*
     * Create two partitions for ptPDisk
	 */
	uSizeList[0] = firstPartSize;   // K Bytes;   
	uSizeList[1] = secondPartSize;  //  K Bytes; 
	uSizeList[2] = threePartSize;
	uSizeList[3] = fourPartSize;
	uSizeList[4] = 0;
	nStatus = fsCreateDiskPartition(ptPDisk, uSizeList, 4);
	if (nStatus < 0)
		return nStatus;
		
	ptPartition = ptPDisk->ptPartList;

	/*
     * Format all partitions of ptPDisk
	 */
	while (ptPartition != NULL)
	{	
		nStatus = fs_fat_format_partition(ptPDisk, ptPartition, 0);
		if (nStatus < 0)
		    return -1;
		ptPartition  = ptPartition->ptNextPart;
	} 
	return FS_OK;
}

static INT  Action_DIR(CHAR *suDirName)
{
	INT				i, nStatus;
	CHAR			szMainName[12], szExtName[8], *pcPtr;
	CHAR			szLongName[MAX_FILE_NAME_LEN/2];
	FILE_FIND_T  	tFileInfo;
	UINT32 			uBlockSize, uFreeSize, uDiskSize;

	memset((UINT8 *)&tFileInfo, 0, sizeof(tFileInfo));
	nStatus = fsFindFirst(suDirName, NULL, &tFileInfo);
	if (nStatus < 0)
		return nStatus;

	do 
	{
		pcPtr = tFileInfo.szShortName;
		if ((tFileInfo.ucAttrib & A_DIR) && 
			(!strcmp(pcPtr, ".") || !strcmp(pcPtr, "..")))
			strcat(tFileInfo.szShortName, ".");

		memset(szMainName, 0x20, 9);
		szMainName[8] = 0;
		memset(szExtName, 0x20, 4);
		szExtName[3] = 0;
		i = 0;
		while (*pcPtr && (*pcPtr != '.'))
			szMainName[i++] = *pcPtr++;
		if (*pcPtr++)
		{
			i = 0;
			while (*pcPtr)
				szExtName[i++] = *pcPtr++;
		}

		fsUnicodeToAscii(tFileInfo.suLongName, szLongName, 1);
		
		if (tFileInfo.ucAttrib & A_DIR){
			#if 0
			sysprintf("%s %s      <DIR>  %02d-%02d-%04d  %02d:%02d  %s\n",
						szMainName, szExtName, 
						tFileInfo.ucWDateMonth, tFileInfo.ucWDateDay, (tFileInfo.ucWDateYear+80)%100 ,
						tFileInfo.ucWTimeHour, tFileInfo.ucWTimeMin, szLongName);
			#else	/* There is an C library issue on Keil V5.24 */
			sysprintf("%s %s     <DIR>     ",
						szMainName, szExtName);
			sysprintf(" %02d-%02d-%04d ",
						tFileInfo.ucWDateMonth, tFileInfo.ucWDateDay, (tFileInfo.ucWDateYear+80)%100);
			
			sysprintf("%02d:%02d  %s\n",
						tFileInfo.ucWTimeHour, tFileInfo.ucWTimeMin, szLongName);
			#endif
		}				
		else{
			volatile UINT32 u32Var=0;	
			#if 0
			sysprintf("%s %s %10d  %02d-%02d-%04d  %02d:%02d  %s\n",
						szMainName, szExtName, tFileInfo.n64FileSize,
						tFileInfo.ucWDateMonth, tFileInfo.ucWDateDay, (tFileInfo.ucWDateYear+80)%100 ,
						tFileInfo.ucWTimeHour, tFileInfo.ucWTimeMin, szLongName);
			#else	/* There is an C library issue on Keil V5.24 */
			sysprintf("%s %s    ",
						szMainName, szExtName);
		#if 0	
			sysprintf("%10d ", tFileInfo.n64FileSize);
		#else
			u32Var = tFileInfo.n64FileSize;
			sysprintf("%10d ", u32Var);
		#endif		
			
			sysprintf(" %02d-%02d-%04d ",
						tFileInfo.ucWDateMonth, tFileInfo.ucWDateDay, (tFileInfo.ucWDateYear+80)%100);
			
			sysprintf("%02d:%02d  %s\n",
						tFileInfo.ucWTimeHour, tFileInfo.ucWTimeMin, szLongName);
			#endif		
		}		
	} while (!fsFindNext(&tFileInfo));
	
	fsFindClose(&tFileInfo);
	
	fsDiskFreeSpace(suDirName[0], &uBlockSize, &uFreeSize, &uDiskSize);
	
	sysprintf("\nDisk Size: %d Kbytes, Free Space: %d KBytes\n", (INT)uDiskSize, (INT)uFreeSize);

	return 0;
}


static INT  Action_DIRS(CHAR *suDirName)
{
	INT		nFileCnt, nDirCnt; 
	UINT64 	uTotalSize;
	INT   	nStatus;
	
	nStatus = fsGetDirectoryInfo(suDirName, NULL, &nFileCnt, &nDirCnt, &uTotalSize, TRUE);
	sysprintf("File = %d, Dir = %d, Total Size = %d\n", nFileCnt, nDirCnt, (INT)uTotalSize);
	return nStatus;
}


static INT  Action_DIR2(CHAR *suDirName)
{
	INT				i, nStatus;
	CHAR			szMainName[12], szExtName[8], *pcPtr;
	CHAR			szLongName[MAX_FILE_NAME_LEN/2];
	CHAR			suLongName[MAX_FILE_NAME_LEN];
	CHAR			suSlash[6] = { '\\', 0, 0, 0 };
	FILE_FIND_T  	tFileInfo;
	UINT32 			uBlockSize, uFreeSize, uDiskSize;

	memset((UINT8 *)&tFileInfo, 0, sizeof(tFileInfo));
	sysprintf("DIR => %s\n", fsDebugUniStr(suDirName));
	nStatus = fsFindFirst(suDirName, NULL, &tFileInfo);
	if (nStatus < 0)
	{
		sysprintf("fsFindFirst <%s> error = %x\n\n\n", fsDebugUniStr(suDirName), nStatus);
		return nStatus;
	}

	do 
	{
		pcPtr = tFileInfo.szShortName;
		if ((tFileInfo.ucAttrib & A_DIR) && 
			(!strcmp(pcPtr, ".") || !strcmp(pcPtr, "..")))
			strcat(tFileInfo.szShortName, ".");

		memset(szMainName, 0x20, 9);
		szMainName[8] = 0;
		memset(szExtName, 0x20, 4);
		szExtName[3] = 0;
		i = 0;
		while (*pcPtr && (*pcPtr != '.'))
			szMainName[i++] = *pcPtr++;
		if (*pcPtr++)
		{
			i = 0;
			while (*pcPtr)
				szExtName[i++] = *pcPtr++;
		}

		fsUnicodeToAscii(tFileInfo.suLongName, szLongName, 1);
		
		if (tFileInfo.ucAttrib & A_DIR)
		{
			sysprintf("%s %s      <DIR>  %02d-%02d-%04d  %02d:%02d  %s\n",
						szMainName, szExtName, 
						tFileInfo.ucWDateMonth, tFileInfo.ucWDateDay, (tFileInfo.ucWDateYear+80)%100 ,
						tFileInfo.ucWTimeHour, tFileInfo.ucWTimeMin, szLongName);
			fsUnicodeCopyStr(suLongName, suDirName);
			if (fsUnicodeStrLen(suDirName) > 6)
				fsUnicodeStrCat(suLongName, suSlash);	/* not C:\ */
			fsUnicodeStrCat(suLongName, tFileInfo.suLongName);
			if ((tFileInfo.szShortName[0] != '.') &&
				(strncmp(tFileInfo.szShortName, "RECYCLED", 8)))
				Action_DIR2(suLongName);
		}
#if 0		
		else
			sysprintf("%s %s %10d  %02d-%02d-%04d  %02d:%02d  %s\n",
						szMainName, szExtName, tFileInfo.n64FileSize,
						tFileInfo.ucWDateMonth, tFileInfo.ucWDateDay, (tFileInfo.ucWDateYear+80)%100 ,
						tFileInfo.ucWTimeHour, tFileInfo.ucWTimeMin, szLongName);
#endif						
	} while (!fsFindNext(&tFileInfo));
	
	fsFindClose(&tFileInfo);
	
	fsDiskFreeSpace(suDirName[0], &uBlockSize, &uFreeSize, &uDiskSize);
	
	sysprintf("Disk Size: %d Kbytes, Free Space: %d KBytes\n", (INT)uDiskSize, (INT)uFreeSize);

	return 0;
}


static	UINT8  _ucLine[128];
static INT  Action_TYPE(CHAR *suFileName, CHAR *szAsciiName)
{
	INT	 	hFile;
	INT     nLen, nIdx, nStatus;

	hFile = fsOpenFile(suFileName, szAsciiName, O_RDONLY);
	if (hFile < 0)
		return hFile;

	nStatus = 0;
	while (1)
	{
		nStatus = fsReadFile(hFile, _ucLine, 128, &nLen);
		if (nStatus < 0)
			break;
			
		for (nIdx = 0; nIdx < nLen; nIdx++)
		{
			if (_ucLine[nIdx] < 127)
				sysprintf("%c", _ucLine[nIdx]);
		}
	}

	fsCloseFile(hFile);

	return nStatus;
}



static INT  Action_ReadSpeedTest(CHAR *suFileName, CHAR *szAsciiName)
{
	INT	 	 hFile;
	INT      nReadLen, nTime0, nStatus;
	UINT32   uKBCnt;

	uKBCnt = 0;
	hFile = fsOpenFile(suFileName, szAsciiName, O_RDONLY);
	if (hFile < 0)
	{
		sysprintf("Open file fail\n");
		return hFile;
	}
	nStatus = 0;
	nTime0 = get_timer_ticks();
	while (1)
	{
		nReadLen = 0;
		nStatus = fsReadFile(hFile, (UINT8 *)_pucDummy, DUMMY_BUFFER_SIZE, &nReadLen);
		if ((nStatus < 0) || (nReadLen == 0))
			break;
		
		uKBCnt += nReadLen / 1024;
		
		if (uKBCnt % 8192 == 0)	//Print for every 8MB
			sysprintf("%d MB\n", uKBCnt/1024);
	}
	nTime0 = get_timer_ticks() - nTime0;
	if (nTime0 == 0) nTime0 = 1;

	sysprintf("Read speed: %d KB/sec\n", (INT)(uKBCnt * 100) / nTime0);
	
	fsCloseFile(hFile);
	return nStatus;
}



static INT  Action_WriteSpeedTest(CHAR *suFileName, CHAR *szAsciiName)
{
	INT	 	 hFile;
	INT      i, j, nWriteLen, nTime0, nStatus;
	UINT32   uKBCnt = 0;

	hFile = fsOpenFile(suFileName, szAsciiName, O_CREATE | O_TRUNC);
	if (hFile < 0)
	{
		sysprintf("Open file fail\n");
		return hFile;
	}

	for (i = 0, j = 0; i < DUMMY_BUFFER_SIZE; i += 2, j++)
	{
		_pucDummy[i] = (j >> 8) & 0xff;
		_pucDummy[i + 1] = j & 0xff;
	}

	memset((UINT8 *)_pucDummy + 0x7e0, 0x97, 32);
	nStatus = 0;
	nTime0 = get_timer_ticks();

	while (1)
	{
		nStatus = fsWriteFile(hFile, (UINT8 *)_pucDummy, DUMMY_BUFFER_SIZE, &nWriteLen);
		if (nStatus < 0)
			break;

		uKBCnt += nWriteLen / 1024;
		
		if (uKBCnt % 1024 == 0)
			sysprintf("%d MB\r", uKBCnt);
		if (uKBCnt >= 40*1024)
			break;
	}
	nTime0 = get_timer_ticks() - nTime0;
	if (nTime0 == 0) nTime0 = 1;

	sysprintf("Write speed: %d KB/sec\n", (INT)(uKBCnt * 100) / nTime0);
	
	fsCloseFile(hFile);

	return nStatus;
}


static INT  Action_FileAppendTest(CHAR *suFileName, CHAR *szAsciiName)
{
	INT	 	hFile;
	CHAR	szPattern[128];
	INT		i, nWriteLen, nStatus;
	INT  	nWriteCnt = 15 * 1024;

	hFile = fsOpenFile(suFileName, szAsciiName, O_APPEND);
	if (hFile < 0)
		return hFile;
		
	for (i =0 ; i < 26; i++)
		szPattern[i] = 'A' + i;
		
	while (nWriteCnt > 0)
	{
		nStatus = fsWriteFile(hFile, (UINT8 *)szPattern, 26, &nWriteLen);
		if (nStatus < 0)
		{
			fsGetErrorDescription(nStatus, "", 1);
			break;
		}
		nWriteCnt -= nWriteLen;
	}
	nStatus = fsCloseFile(hFile);
	return nStatus;
}



static INT  Action_Compare(CHAR *suFileName1, CHAR *szAsciiName1, 
							CHAR *suFileName2, CHAR *szAsciiName2)
{
	INT		hFile1, hFile2;
	INT		nLen1, nLen2, nCount, nStatus1, nStatus2;
	UINT8	buffer1[8192], buffer2[8192];
	
	hFile1 = fsOpenFile(suFileName1, szAsciiName1, O_RDONLY);
	if (hFile1 < 0)
		return hFile1;
	
	hFile2 = fsOpenFile(suFileName2, szAsciiName2, O_RDONLY);
	if (hFile2 < 0)
		return hFile2;
	
	sysprintf("\nComparing file ...\n");
	nCount = 0;
	while (1)
	{
		nStatus1 = fsReadFile(hFile1, buffer1, 1024, &nLen1);
		nStatus2 = fsReadFile(hFile2, buffer2, 1024, &nLen2);
		
		if ((nStatus1 == ERR_FILE_EOF) && (nStatus2 == ERR_FILE_EOF))
		{			
			sysprintf("\nFile Compare length %x pass\n", nCount * 1024+nLen1);
			fsCloseFile(hFile1);
			fsCloseFile(hFile2);
			return Successful;
		}
		
		if (nLen1 != nLen2)
		{
			sysprintf("Compare different length \n");
			return -1;
		}	
		if (memcmp(buffer1, buffer2, nLen1))
		{
			sysprintf("File Compare failed at offset %x\n", nCount * 1024 + nLen1);
			return -1;
		}	
		if(nLen1<1024)	
		{
			sysprintf("\nFile Compare length %x pass\n", nCount * 1024+nLen1);
			break;
		}	
		if(nLen1==1024)
			nCount++;
		//if ((nCount % 1024) == 0)
		//	sysprintf("%d KB    \r", nCount);
	}	
	

	fsCloseFile(hFile1);
	fsCloseFile(hFile2);
	return -1;
}



static INT  Action_PrintDiskInfo()
{
	PDISK_T		*pDiskList, *ptPDiskPtr;
	PARTITION_T	*ptPartition;
	INT			nDiskIdx = 0;
	INT			nPartIdx;
	
	ptPDiskPtr = pDiskList = fsGetFullDiskInfomation();
	
	while (ptPDiskPtr != NULL)
	{
		sysprintf("\n\n=== Disk %d (%s) ======================\n", nDiskIdx++, (ptPDiskPtr->nDiskType & DISK_TYPE_USB_DEVICE) ? "USB" : "IDE");
		sysprintf("    name:     [%s%s]\n", ptPDiskPtr->szManufacture, ptPDiskPtr->szProduct);
		sysprintf("    head:     [%d]\n", ptPDiskPtr->nHeadNum);
		sysprintf("    sector:   [%d]\n", ptPDiskPtr->nSectorNum);
		sysprintf("    cylinder: [%d]\n", ptPDiskPtr->nCylinderNum);
		sysprintf("    size:     [%d MB]\n", ((INT)ptPDiskPtr->uDiskSize / 1024));
		
		ptPartition = ptPDiskPtr->ptPartList;
		nPartIdx = 1;
		while (ptPartition != NULL)
		{
			sysprintf("\n    --- Partition %d --------------------\n", nPartIdx++);
			sysprintf("        active: [%s]\n", (ptPartition->ucState & 0x80) ? "Yes" : "No");
			sysprintf("        size:   [%d MB]\n", (INT)(ptPartition->uTotalSecN / 1024) / 2);
			sysprintf("        start:  [%d]\n", (INT)ptPartition->uStartSecN);
			sysprintf("        type:   ");
			if (ptPartition->ptLDisk == NULL)
				sysprintf("[Unknown]\n");
			else 
			{
				switch (ptPartition->ptLDisk->ucFileSysType)
				{
					case FILE_SYSTEM_FAT12:
						sysprintf("[FAT12]\n");
						break;
					case FILE_SYSTEM_FAT16:
						sysprintf("[FAT16]\n");
						break;
					case FILE_SYSTEM_FAT32:
						sysprintf("[FAT32]\n");
						break;
					case FILE_SYSTEM_NTFS:
						sysprintf("[NTFS]\n");
						break;
					default:
						sysprintf("[???????]\n");
						break;
				}
				sysprintf("        drive:  [%c]\n", ptPartition->ptLDisk->nDriveNo);
			}
			ptPartition = ptPartition->ptNextPart;
		}
		ptPDiskPtr = ptPDiskPtr->ptPDiskAllLink;
	}
	fsReleaseDiskInformation(pDiskList);
	return 0;
}



INT  build_full_path(CHAR *szFullPath, CHAR *szPath)
{
	INT		nDriveNo;
	CHAR	*pcPtr;
	
	if (szPath[1] == ':')
	{
		nDriveNo = szPath[0];
		if ((nDriveNo >= 'a') && (nDriveNo <= 'z'))
			nDriveNo -= ('a' - 'A');
		pcPtr = szPath + 2;
	}
	else
	{
		nDriveNo = _nCurrentDrive;
		pcPtr = szPath;
	}
	
	if ((nDriveNo < 'C') || (nDriveNo > 'Z'))
		return -1;
		
	if (pcPtr[0] == '\\')
		strcpy(szFullPath, szPath);
	else
	{
		strcpy(szFullPath, _pszWorkingDirectory[nDriveNo - 'C']);
		strcat(szFullPath, "\\");
		strcat(szFullPath, pcPtr);
	}
	
	return 0;
}


void Create_File_Test(void)
{
	CHAR  	szAsciiStr[128], suFileName[512];
	INT		nIdx, hFile, nWriteLen;
	INT		t0, t_create, t_write, t_close;

	memset(szAsciiStr, 0, sizeof(szAsciiStr));
	sprintf(szAsciiStr, "C:\\Test");
	fsAsciiToUnicode(szAsciiStr, suFileName, TRUE);
	fsMakeDirectory(suFileName, NULL);
	
	for (nIdx = 1; nIdx < 1000; nIdx++)
	{
		sprintf(szAsciiStr, "C:\\Test\\%04d", nIdx);
		fsAsciiToUnicode(szAsciiStr, suFileName, TRUE);
		
		t0 = get_timer_ticks();
		hFile = fsOpenFile(suFileName, NULL, O_CREATE | O_IOC_VER2);
		t_create = get_timer_ticks() - t0;
		
		t0 = get_timer_ticks();
		fsWriteFile(hFile, (UINT8 *)_pucDummy, 1024, &nWriteLen);	
		t_write = get_timer_ticks() - t0;
		
		t0 = get_timer_ticks();
		fsCloseFile(hFile);
		t_close = get_timer_ticks() - t0;

		sysprintf("%04d --- %d, %d, %d\n", nIdx, t_create, t_write, t_close);
	}
	fsFlushIOCache();
}


void  delay_ticks(int ticks)
{
	int  t0;
	
	t0 = get_timer_ticks();
	while (get_timer_ticks() - t0 < ticks)
		;
}


#ifdef SW_REMOVE
void USB_PhyPowerDown_Test()
{
	LDISK_T	 *ptLDisk;
	PDISK_T  *ptPDisk;
	int   	 t_start, t;
	int   	 i, tloop, sec;

	if (get_vdisk('C', &ptLDisk) < 0)
	{
		sysprintf("Disk C not found, USB test aborted!!\n");
		return;
	}
	ptPDisk = ptLDisk->ptPDisk;
	
	t_start = get_timer_ticks();
	
	// 24 hours test loop
	//
	for (tloop = 0; tloop < 24*60*60; tloop++)
	{
		sec = (get_timer_ticks() - t_start) / 100;
		sysprintf("**** Test loop %d elapsed: %02d:%02d:%02d\n", tloop, sec/3600, (sec/60) % 60, sec % 60);
		
		sysprintf("Before suspend: %x\n", inpw(UPSCR1));
				
		outpw(UPSCR1, inpw(UPSCR1) | PORT_SUSPEND);
		for (i = 0; i < 0x10000; i++);
		sysprintf("After set suspend: %x\n", inpw(UPSCR1));
				
		outpw(UCMDR, (inpw(UCMDR) & ~CMD_RUN));
		sysprintf("After clear RUN: %x\n", inpw(UPSCR1));
		sysprintf("After suspend completed: %x\n", inpw(UPSCR1));

		outpw(0xB00050C4, 0xEC);
		outpw(0xB00050C8, 0xEC);
		
		// wait 1 minute
		//
		t = get_timer_ticks();
		while (get_timer_ticks() - t < 6000) ;
		
		outpw(0xB00050C4, 0x160);				
		outpw(0xB00050C8, 0x520);
		
		outpw(UCMDR, (inpw(UCMDR) | CMD_RUN));
		for (i = 0; i < 0x10000; i++);
		sysprintf("After enable RUN: %x\n", inpw(UPSCR1));
				
		outpw(UPSCR1, inpw(UPSCR1) | PORT_RESUME);
		for (i = 0; i < 0x100000; i++);
		sysprintf("After set resume: %x\n", inpw(UPSCR1));
				
		outpw(UPSCR1, inpw(UPSCR1) &~ (PORT_RESUME | PORT_SUSPEND));
		for (i = 0; i < 0x10000; i++);
		sysprintf("After resume completed: %x\n", inpw(UPSCR1));

		// read 128 sectors from disk
		if (ptPDisk->ptDriver->read(ptPDisk, 0, 128, (UINT8 *)_pucDummy) < 0)
		{
			sysprintf("Read disk failed!!\n");
			return;
		}
		else
			sysprintf("Test passed.\n");
	}
}
#endif

#ifdef ECOS
void CommandShell(cyg_addrword_t data)
#else
void  CommandShell()
#endif
{
	INT     	nCmdCode;
	CHAR    	*szCmd, *szArgum1, *szArgum2, *pcPtr; // *szArgum3
	CHAR		szPath1[MAX_PATH_LEN], szPath2[MAX_PATH_LEN];
	CHAR 		suFullName1[MAX_PATH_LEN], suFullName2[MAX_PATH_LEN];
	INT			hFile, nStatus;

#if 0
	/* USB open close test */
	while (1)
	{
		delay_ticks(100);

		#ifndef ECOS
		Hub_CheckIrqEvent();
		delay_ticks(20);
		Hub_CheckIrqEvent();
		#endif
		
		sysprintf("\n\nClose =>\n\n");
		
		DeInitUsbSystem();
		
		sysprintf("\n\nClose done.\n\n");

		#ifndef ECOS
		Hub_CheckIrqEvent();
		delay_ticks(20);
		Hub_CheckIrqEvent();
		#endif

		delay_ticks(100);

		#ifndef ECOS
		Hub_CheckIrqEvent();
		delay_ticks(20);
		Hub_CheckIrqEvent();
		#endif

		sysprintf("\n\nOpen =>\n\n");
		
		InitUsbSystem();
		
		sysprintf("\n\nOpen done.\n\n");

		#ifndef ECOS
		Hub_CheckIrqEvent();
		delay_ticks(20);
		Hub_CheckIrqEvent();
		#endif
		
		sysprintf("Free - %d\n", USB_available_memory());
		delay_ticks(50);
		
	}
#endif

	//Create_File_Test();

	_nCurrentDrive = 'C';
	
	while (1)
	{
#if	defined(ENABLE_USB_HOST) && !defined(ECOS)
		Hub_CheckIrqEvent();
		//UMAS_ScanAllDevice();
#endif		
		//sysprintf("memory: %d\n", USB_available_memory());
		
		strcpy(szPath1, _pszWorkingDirectory[_nCurrentDrive - 'C']);
		if (strlen(_pszWorkingDirectory[_nCurrentDrive - 'C']) == 2)
			sysprintf("%s\\>", szPath1);
		else
			sysprintf("%s>", szPath1);
			
		
		accept_string(_szCommandLine, 256);
		
		if (strlen(_szCommandLine) == 0)
			continue;
	
		sysprintf("\n");
		
		/*- get command tokens */
		szCmd = &_szCommandLine[0];
		szArgum1 = get_token(szCmd, " \t\r\n", 8);
		szArgum2 = get_token(szArgum1, " \t\r\n", 128);
		//szArgum3 = get_token(szArgum2, " \t\r\n", 128);
			
		//sysprintf("Command:<%s> <%s> <%s>\n", szCmd, szArgum1, szArgum2);
		
		fsAsciiToUpperCase(szCmd);
		
		nCmdCode = get_command_code(szCmd);
		
		if ((szArgum1 == NULL) && (szCmd[1] == ':') && (szCmd[2] == 0))
			nCmdCode = CHANGE;
		
		if (nCmdCode < 0)
		{
			sysprintf("Command not found!\n");
			continue;
		}
		
		build_full_path(szPath1, szArgum1);
		fsAsciiToUnicode(szPath1, suFullName1, 1);
		build_full_path(szPath2, szArgum2);
		fsAsciiToUnicode(szPath2, suFullName2, 1);
		
		nStatus = 0;

		switch (nCmdCode)
		{
			case QQ:
			case HELP:
				sysprintf("%s", _pszCommandShellHelp);
				break;
				
			case DIR:
				nStatus = Action_DIR(suFullName1);
				//nStatus = Action_DIR2(suFullName1);
				break;

			case DIRS:
				nStatus = Action_DIRS(suFullName1);
				break;
		  
			case TYPE:
				nStatus = Action_TYPE(suFullName1, szArgum1);
				break;
				
			case CD:
				if (!strcmp(szArgum1, "\\"))
				{
					_pszWorkingDirectory[_nCurrentDrive - 'C'][2] = '\0';
					break;
				}
			
				if (!strcmp(szArgum1, ".."))
				{
					if (strlen(_pszWorkingDirectory[_nCurrentDrive - 'C']) == 2)
						break;
						
					strcpy(szPath1, _pszWorkingDirectory[_nCurrentDrive - 'C']);
					
					nStatus = 0;
					pcPtr = szPath1 + strlen(szPath1);
					while (pcPtr > szPath1)
					{
						if (*pcPtr == '\\')
							break;
						pcPtr--;
					}
					*pcPtr = '\0';
					strcpy(_pszWorkingDirectory[_nCurrentDrive - 'C'], szPath1);
					break;
				}
				
				fsAsciiToUnicode(szPath1, suFullName1, 1);
				hFile = fsOpenFile(suFullName1, szArgum1, O_DIR);
				if (hFile < 0)
					nStatus = hFile;
				else
				{
					strcpy(_pszWorkingDirectory[_nCurrentDrive - 'C'], szPath1);
					fsCloseFile(hFile);
					nStatus = 0;
				}
				break;
				
			case DEL:
				nStatus = fsDeleteFile(suFullName1, szArgum1);
				break;
				
			case DELA:
				nStatus = fsDeleteDirTree(suFullName1, NULL);
				break;
				
			case COPY:
				nStatus = fsCopyFile(suFullName1, szArgum1, suFullName2, szArgum2);
				break;
				
			case MKDIR:
			case MD:
				nStatus = fsMakeDirectory(suFullName1, szArgum1);
				break;
				
			case RMDIR:
			case RD:
				nStatus = fsRemoveDirectory(suFullName1, szArgum1);
				break;
				
			case RENAME:
				if ((nStatus = fsRenameFile(suFullName1, szArgum1, suFullName2, szArgum2, 0)) == ERR_FILE_IS_DIRECTORY)
					nStatus = fsRenameFile(suFullName1, szArgum1, suFullName2, szArgum2, 1);
				break;
				
			case CMP:
				nStatus = Action_Compare(suFullName1, szArgum1, suFullName2, szArgum2);
				break;
				
			case CHANGE:
				if ((szCmd[0] >= 'C') && (szCmd[0] <= 'Z'))
					_nCurrentDrive = szCmd[0];
				continue;
				
			//case DUMP:
			//	{
			//		UINT32	uSectorNo = 0;
			//		
			//		while (*szArgum1)
			//			uSectorNo = uSectorNo * 10 + (*szArgum1++ - '0');
			//		fsDumpDiskSector(uSectorNo, 1);
			//	}
			//	continue;
				
			case RDTST:
				//_fs_uReadSecCnt = _fs_uWriteSecCnt = 0;
				nStatus = Action_ReadSpeedTest(suFullName1, szArgum1);
				//sysprintf("\n\n\nTotal: Read: %d, Write: %d\n", _fs_uReadSecCnt, _fs_uWriteSecCnt);
				break;

			case WRTST:
				//_fs_uReadSecCnt = _fs_uWriteSecCnt = 0;
				//nStatus = Action_WriteSpeedTest(suFullName1, szArgum1);
				nStatus = Action_WriteSpeedTest(suFullName1, NULL);
				sysprintf("wrtst [%x]\n", nStatus);
				//sysprintf("\n\n\nTotal: Read: %d, Write: %d\n", _fs_uReadSecCnt, _fs_uWriteSecCnt);
				break;
				
			case APTST:
				nStatus = Action_FileAppendTest(suFullName1, szArgum1);
				break;
				
			case RAWRD:
				{
					LDISK_T	 *ptLDisk;
					PDISK_T  *ptPDisk;
					INT  nTime0;
					INT  nCount, nStatus;
					
					nStatus = get_vdisk('C', &ptLDisk);
					if (nStatus < 0)
						break;
					ptPDisk = ptLDisk->ptPDisk;
					
					nTime0 = get_timer_ticks();
					/* 40MB read test */
					for (nCount = 0; nCount < 40960; nCount += 64)
					{
						nStatus = ptPDisk->ptDriver->read(ptPDisk, 1024, 128, (UINT8 *)_pucDummy);
						if (nStatus < 0)
							break;
					}
					nTime0 = get_timer_ticks() - nTime0;
					if (nTime0 == 0) nTime0 = 1;
					sysprintf("Raw Read speed: %d KB/sec\n", (INT)(nCount * 100) / nTime0);
				}	
				break;

			case RAWWD:
				{
					LDISK_T	 *ptLDisk;
					PDISK_T  *ptPDisk;
					INT  nTime0;
					INT  nCount, nStatus;
					
					nStatus = get_vdisk('C', &ptLDisk);
					if (nStatus < 0)
						break;
					ptPDisk = ptLDisk->ptPDisk;
					
					nTime0 = get_timer_ticks();
					/* 40MB write test */
					for (nCount = 0; nCount < 40960; nCount += 64)
					{
						nStatus = ptPDisk->ptDriver->write(ptPDisk, 1024, 128, (UINT8 *)_pucDummy, BLOCKING_WRITE);
						if (nStatus < 0)
							break;
					}
					nTime0 = get_timer_ticks() - nTime0;
					if (nTime0 == 0) nTime0 = 1;
					sysprintf("Raw write speed: %d KB/sec\n", (INT)(nCount * 100) / nTime0);
				}	
				break;
				
			case DISK:
				Action_PrintDiskInfo();
				break;
				
			case DF:
				FAT_dump_sector_cache();
		  		break;

			case FORMAT:
				{
				#if defined(ENABLE_GNAND)||defined(ENABLE_SD_ONE_PART)||defined(ENABLE_SD_TWO_PART)||defined(ENABLE_SD_FOUR_PART)
					UINT32 u32BlockSize, u32FreeSize, u32DiskSize;
				#endif
				#if defined(ENABLE_SD_TWO_PART)||defined(ENABLE_SD_FOUR_PART) 	
					UINT32 u32RemainingSize;
				#endif	
					char chr;
					#if defined(ENABLE_SD_ONE_PART)||defined(ENABLE_SD_TWO_PART)||defined(ENABLE_SD_FOUR_PART) 	
					PDISK_T		*pDiskList;
						
					fsSetReservedArea(RESERVED_SIZE/512);	//Reserved 20480 sector=10MB.
					#endif
					
					
					sysprintf("The operation should cause whole disk data lost, Do you want to continue? [Y/N]\n");
					chr = getchar();
					if( (chr == 'Y') || (chr == 'y') )
					{
					#ifdef ENABLE_SD_ONE_PART 					
						pDiskList = fsGetFullDiskInfomation();
						fsFormatFlashMemoryCard(pDiskList);
						fsReleaseDiskInformation(pDiskList);
						fsDiskFreeSpace('C', &u32BlockSize, &u32FreeSize, &u32DiskSize);   
						sysprintf("block_size = %d\n", u32BlockSize);   
						sysprintf("free_size = %d\n", u32FreeSize);   
						sysprintf("disk_size = %d\n", u32DiskSize);  
					#endif
					#ifdef ENABLE_SD_TWO_PART										
						pDiskList = fsGetFullDiskInfomation();
						sysprintf("Total Disk Size = %dMB\n", pDiskList->uDiskSize/1024);
						u32RemainingSize =  pDiskList->uDiskSize - (RESERVED_SIZE/1024); //rest disk size = total disk size - reserved size 
						sysprintf("RemainingSize = %dMB\n", u32RemainingSize/1024);
						if (fsTwoPartAndFormatAll((PDISK_T *)pDiskList->ptSelf, 
												128*1024, 
												//((PDISK_T *)(ptNDisk.pDisk))->uDiskSize - 16*1024) < 0)    
												(u32RemainingSize-128*1024))<0)
						{   
							sysprintf("Format failed\n");      
							exit(-1);
						}																		
						fsSetVolumeLabel('C', "SD1-1\n", strlen("SD1-1"));
						fsSetVolumeLabel('D', "SD1-2\n", strlen("SD1-2")); 						
						pDiskList = fsGetFullDiskInfomation();  	
						fsDiskFreeSpace('C', &u32BlockSize, &u32FreeSize, &u32DiskSize);   
						sysprintf("block_size = %d\n", u32BlockSize);   
						sysprintf("free_size = %d\n", u32FreeSize);   
						sysprintf("disk_size = %d\n", u32DiskSize);  
						fsDiskFreeSpace('D', &u32BlockSize, &u32FreeSize, &u32DiskSize);    
						sysprintf("block_size = %d\n", u32BlockSize);   
						sysprintf("free_size = %d\n", u32FreeSize);   
						sysprintf("disk_size = %d\n", u32DiskSize);   	
					#endif		
					#ifdef ENABLE_SD_FOUR_PART										
						pDiskList = fsGetFullDiskInfomation();
						sysprintf("Total Disk Size = %dMB\n", pDiskList->uDiskSize/1024);
						u32RemainingSize =  pDiskList->uDiskSize - (RESERVED_SIZE/1024); //rest disk size = total disk size - reserved size 
						sysprintf("RemainingSize = %dMB\n", u32RemainingSize/1024);						
						if (fsFourPartAndFormatAll((PDISK_T *)pDiskList->ptSelf, 128*1024, 128*1024,
						    								128*1024,	u32RemainingSize - 3*128*1024) < 0) 
						{
							sysprintf("Format failed\n");	
							exit(-1);
						}
																				
						fsSetVolumeLabel('C', "SD1-1\n", strlen("SD1-1"));
						fsSetVolumeLabel('D', "SD1-2\n", strlen("SD1-2")); 	
						fsSetVolumeLabel('E', "SD1-3\n", strlen("SD1-3"));
						fsSetVolumeLabel('F', "SD1-4\n", strlen("SD1-4")); 						
						pDiskList = fsGetFullDiskInfomation();  	
						fsDiskFreeSpace('C', &u32BlockSize, &u32FreeSize, &u32DiskSize);   
						sysprintf("block_size = %d\n", u32BlockSize);   
						sysprintf("free_size = %d\n", u32FreeSize);   
						sysprintf("disk_size = %d\n", u32DiskSize);  
						fsDiskFreeSpace('D', &u32BlockSize, &u32FreeSize, &u32DiskSize);    
						sysprintf("block_size = %d\n", u32BlockSize);   
						sysprintf("free_size = %d\n", u32FreeSize);   
						sysprintf("disk_size = %d\n", u32DiskSize);   	
						fsDiskFreeSpace('E', &u32BlockSize, &u32FreeSize, &u32DiskSize);    
						sysprintf("block_size = %d\n", u32BlockSize);   
						sysprintf("free_size = %d\n", u32FreeSize);   
						sysprintf("disk_size = %d\n", u32DiskSize);   	
						fsDiskFreeSpace('F', &u32BlockSize, &u32FreeSize, &u32DiskSize);    
						sysprintf("block_size = %d\n", u32BlockSize);   
						sysprintf("free_size = %d\n", u32FreeSize);   
						sysprintf("disk_size = %d\n", u32DiskSize);   	
					#endif						
					#ifdef ENABLE_GNAND
						sysprintf("Disk Size = %d\n", ((PDISK_T *)(ptNDisk.pDisk))->uDiskSize);
						if (fsTwoPartAndFormatAll((PDISK_T *)ptNDisk.pDisk, 
												32*1024, 
												//((PDISK_T *)(ptNDisk.pDisk))->uDiskSize - 16*1024) < 0)    
												g_u32TotalSize-32*1024)<0)
						{   
							sysprintf("Format failed\n");      
							exit(-1);
						}
						fsSetVolumeLabel('C', "NAND1-1\n", strlen("NAND1-1"));
						fsSetVolumeLabel('D', "NAND1-2\n", strlen("NAND1-2"));   	
						fsDiskFreeSpace('C', &u32BlockSize, &u32FreeSize, &u32DiskSize);   
						sysprintf("block_size = %d\n", u32BlockSize);   
						sysprintf("free_size = %d\n", u32FreeSize);   
						sysprintf("disk_size = %d\n", u32DiskSize);  
						fsDiskFreeSpace('D', &u32BlockSize, &u32FreeSize, &u32DiskSize);    
						sysprintf("block_size = %d\n", u32BlockSize);   
						sysprintf("free_size = %d\n", u32FreeSize);   
						sysprintf("disk_size = %d\n", u32DiskSize);   	
					#endif 	
						
          #ifdef ENABLE_RAM	
            RemoveRAMDisk();
            memset(&i8RamDisk, 0, RAM_DISK_SIZE);
            InitRAMDisk((UINT32)&i8RamDisk, RAM_DISK_SIZE)	;
            FormatRamDisk();
            fsSetVolumeLabel('D', "RAM", strlen("RAM")); 
            //fsAssignDriveNumber('D', DISK_TYPE_HARD_DISK, 1, 1);
          #endif 	
					}						
				}
				break;

			default:
			   sysprintf("Unknown command!\n");
			   break;
	   	} /* end of switch */
	   
		if (_bIsCardInserted)
		{
			_bIsCardInserted = FALSE;
#ifdef DEVICE_SD
			fmiInitSDDevice();
#endif
		}

	   	if (nStatus != ERR_FILE_EOF)
	  		fsGetErrorDescription(nStatus, "", 1);
	} 
}


#ifdef ECOS
static cyg_handle_t  	thread_handle;
static cyg_thread 		thread;
#define STACKSIZE		(128*1024)
static UINT8            _Statck[STACKSIZE];
#endif

#if 0
void	FormatDisk(void)
{
#if 1
	PDISK_T      *ptPDiskList, *ptPDisk;
	PARTITION_T  *ptPartition;
#else	
	
#endif	
	UINT32 u32BlockSize, u32FreeSize, u32DiskSize;
	
	fsAssignDriveNumber('C', DISK_TYPE_SMART_MEDIA, 0, 1);
#if 0
	fsDiskFreeSpace('C', &u32BlockSize, &u32FreeSize, &u32DiskSize);
	
	sysprintf("Disk size %d KBytes\n", u32DiskSize);
	sysprintf("Free size %d KBytes\n", u32FreeSize);
	sysprintf("Block size %d KBytes\n", u32BlockSize);
	sysprintf("Do you want to format NAND flash? [Y/N]\n");
	chr = getchar();
	if( (chr == 'Y') || (chr == 'y') )
	{
		pDiskList = fsGetFullDiskInfomation();
		fsFormatFlashMemoryCard(pDiskList);
		fsReleaseDiskInformation(pDiskList);
		if (fsTwoPartAndFormatAll((PDISK_T *)ptNDisk.pDisk, 
							16*1024, 
							((PDISK_T *)(ptNDisk.pDisk))->uDiskSize - 16*1024) < 0) 
		{
			sysprintf("Format failed\n");	
			exit(-1);
		}	
	}			
#else
	/* Get complete disk information */
	ptPDiskList = fsGetFullDiskInfomation();

	/* Format the first physical disk */
	ptPartition = (PARTITION_T*)ptPDiskList;
	ptPDisk = ptPDiskList;      /* format the first physical disk */
	fsTwoPartAndFormatAll(ptPDisk, 16384, u32DiskSize-16384);
		
	/* Release allocated memory */
	fsReleaseDiskInformation(ptPDiskList);
#endif	
}

void GetDiskInformation(void)
{
	PDISK_T       *pDiskList, *ptPDiskPtr;
	PARTITION_T   *ptPartition;
	INT           nDiskIdx = 0;
	INT           nPartIdx;

	ptPDiskPtr = pDiskList = fsGetFullDiskInfomation();
	while (ptPDiskPtr != NULL)
	{
		sysprintf("\n\n=== Disk %d (%s) ======================\n", 
		nDiskIdx++, (ptPDiskPtr->nDiskType & 
		DISK_TYPE_USB_DEVICE) ? "USB" : "IDE");
		sysprintf("    name:     [%s%s]\n", ptPDiskPtr->szManufacture, 
		ptPDiskPtr->szProduct);
		sysprintf("    head:     [%d]\n", ptPDiskPtr->nHeadNum);
		sysprintf("    sector:   [%d]\n", ptPDiskPtr->nSectorNum);
		sysprintf("    cylinder: [%d]\n", ptPDiskPtr->nCylinderNum);
		sysprintf("    size:     [%d MB]\n", ptPDiskPtr->uDiskSize / 1024);
				
		ptPartition = ptPDiskPtr->ptPartList;
		nPartIdx = 1;
		while (ptPartition != NULL)
		{
			sysprintf("\n    --- Partition %d --------------------\n", 
			nPartIdx++);
			sysprintf("        active: [%s]\n", 
			(ptPartition->ucState & 0x80) ? "Yes" : "No");
			sysprintf("        size:   [%d MB]\n", 
			(ptPartition->uTotalSecN / 1024) / 2);
			sysprintf("        start:  [%d]\n", ptPartition->uStartSecN);
			sysprintf("        type:   ");
			ptPartition = ptPartition->ptNextPart;
		}
		ptPDiskPtr = ptPDiskPtr->ptPDiskAllLink;
	}
	fsReleaseDiskInformation(pDiskList);  
	
}



#endif
/*
void fscopyAndCompare(void)
{
	
	
	

}
*/

void FormatRamDisk(void)
{
	PDISK_T       *pDiskList, *ptPDiskPtr;
	PARTITION_T   *ptPartition;
	INT           nDiskIdx = 0;
	INT           nPartIdx;
	ptPDiskPtr = pDiskList = fsGetFullDiskInfomation();
	while (ptPDiskPtr != NULL)
	{
		sysprintf("\n\n=== Disk %d (%s) ======================\n", 
		nDiskIdx++, (ptPDiskPtr->nDiskType & 
		DISK_TYPE_USB_DEVICE) ? "USB" : "IDE");
		sysprintf("    name:     [%s%s]\n", ptPDiskPtr->szManufacture, 
		ptPDiskPtr->szProduct);
		sysprintf("    head:     [%d]\n", ptPDiskPtr->nHeadNum);
		sysprintf("    sector:   [%d]\n", ptPDiskPtr->nSectorNum);
		sysprintf("    cylinder: [%d]\n", ptPDiskPtr->nCylinderNum);
		sysprintf("    size:     [%d MB]\n", ptPDiskPtr->uDiskSize / 1024);
				
		ptPartition = ptPDiskPtr->ptPartList;
		nPartIdx = 1;
		while (ptPartition != NULL)
		{
			sysprintf("\n    --- Partition %d --------------------\n", 
			nPartIdx++);
			sysprintf("        active: [%s]\n", 
			(ptPartition->ucState & 0x80) ? "Yes" : "No");
			sysprintf("        size:   [%d MB]\n", 
			(ptPartition->uTotalSecN / 1024) / 2);
			sysprintf("        start:  [%d]\n", ptPartition->uStartSecN);
			sysprintf("        type:   ");
			ptPartition = ptPartition->ptNextPart;
		}
		ptPDiskPtr = ptPDiskPtr->ptPDiskAllLink;
		fsFormatFlashMemoryCard(ptPDiskPtr);
	}	
	fsReleaseDiskInformation(pDiskList);  
}

int main()
{
    WB_UART_T 	uart;
	UINT32 u32ExtFreq;
	UINT32 u32PllOutKHz;

#ifdef ENABLE_GNAND	
	char 		chr;	
	UINT32 u32BlockSize, u32FreeSize, u32DiskSize, u32TotalSize;
#endif
	
	/* CACHE_ON	*/
	sysEnableCache(CACHE_WRITE_BACK);
	u32ExtFreq = sysGetExternalClock();
	sysUartPort(1);
	uart.uiFreq = u32ExtFreq*1000;	//use APB clock
    uart.uiBaudrate = 115200;
    uart.uiDataBits = WB_DATA_BITS_8;
    uart.uiStopBits = WB_STOP_BITS_1;
    uart.uiParity = WB_PARITY_NONE;
    uart.uiRxTriggerLevel = LEVEL_1_BYTE;
    sysInitializeUART(&uart);

    sysDisableInterrupt(IRQ_UDC);	
   
    sysSetSystemClock(eSYS_UPLL, 	//E_SYS_SRC_CLK eSrcClk,
    						192000,		  //UINT32 u32PllKHz,
							192000,		  //UINT32 u32SysKHz,
							192000,		  //UINT32 u32CpuKHz,
							192000/2,	  //UINT32 u32HclkKHz,
							192000/4);    //UINT32 u32ApbKHz


    u32PllOutKHz = sysGetPLLOutputKhz(eSYS_UPLL, u32ExtFreq);
	sysprintf("PLL out frequency %d Khz\n", u32PllOutKHz);	


	sysSetTimerReferenceClock (TIMER0, u32ExtFreq*1000);
	sysStartTimer(TIMER0, 100, PERIODIC_MODE);

	sysprintf("fsInitFileSystem.\n");
	fsInitFileSystem();

#ifdef ENABLE_USB_HOST
	/* Depend on the board's layout */
	//USB_PortInit(HOST_NORMAL_PORT0_ONLY);
	//USB_PortInit(HOST_NORMAL_TWO_PORT);	
	//USB_PortInit(HOST_LIKE_PORT0);
	USB_PortInit(HOST_LIKE_PORT1);	

	//umass_register_connect(MassStotrageConnection);	
	//umass_register_disconnect(MassStotrageDisconnection);	
	InitUsbSystem();       
	UMAS_InitUmasDriver();
#endif	

#if defined(ENABLE_SD)||defined(ENABLE_SD_TWO_PART)
	sicIoctl(SIC_SET_CLOCK, u32PllOutKHz, 0, 0);	
	sicOpen();	
	if (sicSdOpen0()<=0)
	{
		sysprintf("Error in initializing SD card !! \n");						
		while(1);
	}	
	sysprintf("sicOpen done !! \n");
#endif			
#ifdef ENABLE_SD_ONE_PART	
	fsAssignDriveNumber('C', DISK_TYPE_SD_MMC, 0, 1);
#endif
#ifdef ENABLE_SD_TWO_PART
	fsAssignDriveNumber('C', DISK_TYPE_SD_MMC, 0, 1);
	fsAssignDriveNumber('D', DISK_TYPE_SD_MMC, 0, 2);
#endif
#ifdef ENABLE_SD_FOUR_PART
	fsAssignDriveNumber('C', DISK_TYPE_SD_MMC, 0, 1);
	fsAssignDriveNumber('D', DISK_TYPE_SD_MMC, 0, 2);
	fsAssignDriveNumber('E', DISK_TYPE_SD_MMC, 0, 3);
	fsAssignDriveNumber('F', DISK_TYPE_SD_MMC, 0, 4);
#endif

#ifdef ENABLE_RAM	
	memset(&i8RamDisk, 0, RAM_DISK_SIZE);
	InitRAMDisk((UINT32)&i8RamDisk, RAM_DISK_SIZE)	;
	FormatRamDisk();
	fsSetVolumeLabel('D', "RAM", strlen("RAM")); 
	//fsAssignDriveNumber('D', DISK_TYPE_HARD_DISK, 1, 1);
#endif 

#ifdef ENABLE_GNAND
	{	
		//fsAssignDriveNumber('C', DISK_TYPE_SMART_MEDIA, 0, 1);
	    	//fsAssignDriveNumber('D', DISK_TYPE_SMART_MEDIA, 0, 2);    
		
		sicOpen();
		
		sysprintf("ENABLE_GNAND.\n");		
		if(GNAND_InitNAND(&_nandDiskDriver0, &ptNDisk, TRUE) < 0) 
		{
			sysprintf("GNAND_InitNAND error\n");
			exit(-1);	
		}			
		if(GNAND_MountNandDisk(&ptNDisk) < 0) 
		{
			sysprintf("GNAND_MountNandDisk error\n");
			exit(-1);	
		}
		u32TotalSize = ptNDisk.nZone* ptNDisk.nBlockPerZone*ptNDisk.nPagePerBlock*ptNDisk.nPageSize;				
	}

	
		
	sysprintf("Do you want to format NAND flash? [Y/N]\n");
	chr = getchar();
	if( (chr == 'Y') || (chr == 'y') )
	{	
		sysprintf("Total NAND size = %d KB\n", u32TotalSize/1024);
		if( fsTwoPartAndFormatAll((PDISK_T *)ptNDisk.pDisk,
							32*1024,
							(u32TotalSize/1024-32*1024))<0 )						
		{
			sysprintf("Format failed\n");	
			exit(-1);
		}	
		
	}		
	fsSetVolumeLabel('C', "NAND1-1\n", strlen("NAND1-1"));
	fsSetVolumeLabel('D', "NAND1-2\n", strlen("NAND1-2"));   	
	fsDiskFreeSpace('C', &u32BlockSize, &u32FreeSize, &u32DiskSize);   
	sysprintf("block_size = %d\n", u32BlockSize);   
	sysprintf("free_size = %d\n", u32FreeSize);   
	sysprintf("disk_size = %d\n", u32DiskSize);   
	g_u32TotalSize = u32DiskSize;
	sysprintf("Total NAND size = %d KB\n", g_u32TotalSize);   
	fsDiskFreeSpace('D', &u32BlockSize, &u32FreeSize, &u32DiskSize);    
	
	sysprintf("block_size = %d\n", u32BlockSize);   
	sysprintf("free_size = %d\n", u32FreeSize);   
	sysprintf("disk_size = %d\n", u32DiskSize);   
	g_u32TotalSize = g_u32TotalSize+u32DiskSize;
	sysprintf("Total NAND size = %d KB\n", g_u32TotalSize);   	 
	
#endif
	


#ifdef ECOS	
 	cyg_thread_create(20, CommandShell, 0, "cs",
        				_Statck, STACKSIZE, &thread_handle, &thread);
	cyg_thread_resume(thread_handle);
#else




	CommandShell();
#endif

	return 0;
}



