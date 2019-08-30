/****************************************************************************
 * 
 * FILENAME
 *     NVTFAT_ERROR.c
 *
 * VERSION
 *     1.0
 *
 * DESCRIPTION
 *     Get szDescription of the error nErrCode
 *
 * DATA STRUCTURES
 *     None
 *
 * FUNCTIONS
 *     fsGetErrorDescription
 *
 *
 * HISTORY
 *     2004.01.12		Ver 0.9 Created by PC30 YCHuang
 *
 * REMARK
 *     None
 **************************************************************************/
#ifdef ECOS 
#include "stdio.h"
#include "string.h"
#include "drv_api.h"
#include "wbio.h"
#else
#include <stdio.h>
#include <string.h>
#include "N9H20.h"
#endif


#ifdef ECOS
#include "kapi.h"
#define printf  diag_printf
#else
#define printf  sysprintf
#endif

typedef struct fs_err_code_S
{
	INT		nErrorCode;
	CHAR	*szDescription;
}	FS_ERR_T;


static FS_ERR_T	 _ptErrorCodeList[] =
{
	ERR_FILE_EOF,                	"end of file",
	ERR_GENERAL_FILE_ERROR,      	"general file error",
	ERR_NO_FREE_MEMORY,          	"no available memory",
	ERR_NO_FREE_BUFFER,          	"no available sector buffer",
	ERR_NOT_SUPPORTED,				"operation was not supported",
	ERR_UNKNOWN_OP_CODE, 			"unrecognized operation nErrCode",
	ERR_INTERNAL_ERROR,				"file system internal error",
	ERR_SYSTEM_LOCK,				"file system locked by ScanDisk or Defragment",

	ERR_FILE_NOT_FOUND,          	"file not found",
	ERR_FILE_INVALID_NAME,       	"invalid file name",
	ERR_FILE_INVLAID_HANDLE,     	"invalid file handle",
	ERR_FILE_IS_DIRECTORY,       	"the file to be opened is a directory",
	ERR_FILE_IS_NOT_DIRECTORY,   	"the directory to be opened is a file",
	ERR_FILE_CREATE_NEW,         	"can not create new directory entry",
	ERR_FILE_OPEN_MAX_LIMIT,		"number of opened files has reached limitation",
	ERR_FILE_EXIST,					"file already exist",
	ERR_FILE_INVALID_OP,			"invalid file operation",
	ERR_FILE_INVALID_ATTR,			"invalid file attribute",
	ERR_FILE_INVALID_TIME,			"invalid time format",
	ERR_FILE_TRUNC_UNDER,			"truncate file underflow, size < pos", 
	ERR_FILE_IS_CORRUPT,			"file is corrupt",

	ERR_PATH_INVALID,            	"invalid path name",
	ERR_PATH_TOO_LONG,           	"path too long",
	ERR_PATH_NOT_FOUND,				"path not found",

	ERR_DRIVE_NOT_FOUND,         	"drive not found, the disk may have been unmounted",
	ERR_DRIVE_INVALID_NUMBER,    	"invalid drive number",
	ERR_DRIVE_NO_FREE_SLOT,			"can not mount more drive",

	ERR_DIR_BUILD_EXIST,         	"try to build an existent directory",
	ERR_DIR_REMOVE_MISS,         	"try to remove a nonexistent directory",
	ERR_DIR_REMOVE_ROOT,         	"try to remoe root directory",
	ERR_DIR_REMOVE_NOT_EMPTY,    	"try to remove a non-empty directory",
	ERR_DIR_DIFFERENT_DRIVE,     	"specified files on different drive",
	ERR_DIR_ROOT_FULL,           	"FAT12/FAT16 root directory full",
	ERR_DIR_SET_SIZE,				"try to set file size of a directory",

	ERR_READ_VIOLATE,               "user has no read privilege",
	ERR_WRITE_VIOLATE,				"user has no write privilege",
	ERR_ACCESS_VIOLATE,				"can not access",
	ERR_READ_ONLY,            		"try to open a read-only file with write access",
	ERR_WRITE_CAP,					"try to write file/directory which was opened with read-only",

	ERR_NO_DISK_MOUNT,				"there's no any disk mounted",
	ERR_DISK_CHANGE_DIRTY,       	"disk change, buffer is dirty",
	ERR_DISK_REMOVED,            	"portable disk has been removed",
	ERR_DISK_WRITE_PROTECT,			"disk is write-protected",
	ERR_DISK_FULL,               	"disk full",
	ERR_DISK_BAD_PARTITION,      	"bad partition",
	ERR_DISK_UNKNOWN_PARTITION,		"unknown or not supported partition type",
	ERR_DISK_UNKNOWN_FORMAT,		"unknown disk format",
	ERR_DISK_BAD_BPB,				"bad BPB, disk may not be formatted",
	ERR_DISK_IO,					"disk I/O failure",
	ERR_DISK_IO_TIMEOUT,			"disk I/O time-out",
	ERR_DISK_FAT_BAD_CLUS,			"bad cluster number in FAT table",
	ERR_DISK_INVALID_PARM,			"invalid parameter",
	ERR_DISK_CANNOT_LOCK,			"cannot lock disk, the disk was in-use or locked by other one",

	ERR_SEEK_SET_EXCEED,         	"file seek set exceed end-of-file",
	ERR_ACCESS_SEEK_WRITE,       	"try to seek a file which was opened for written",

	ERR_FILE_SYSTEM_NOT_INIT,    	"file system was not initialized",
	ERR_ILLEGAL_ATTR_CHANGE,     	"illegal file attribute change",

	0,								""
};




/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*      fsGetErrorDescription                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Get the human readable szDescription of error reason.            */
/*                                                                       */
/* INPUTS                                                                */
/*      nErrCode    The error code                                       */
/*      bIsPrint    Print the szDescription string on console or not     */
/*                                                                       */
/*                                                                       */
/* OUTPUTS                                                               */
/*      szDescription    The container to receive the szDescription      */
/*                       string. If it was passed as NULL, then this     */
/*                       parameter will be ignored. It it is not NULL,   */
/*                       its length must at least be 128 bytes long.     */                                   
/*                                                                       */
/* RETURN                                                                */
/*      0           Success                                              */
/*      Otherwise   Failed, use GET_LAST_ERROR to obtain the reason      */
/*                                                                       */
/*************************************************************************/
void  fsGetErrorDescription(INT nErrCode, CHAR *szDescription, INT bIsPrint)
{
	FS_ERR_T	*ptErrCodePtr = _ptErrorCodeList;
	
	if (!nErrCode)
		return;
		
	if (nErrCode == ERR_FILE_NO_MORE)
		return;
	
	while (ptErrCodePtr->nErrorCode != 0)
	{
		if (ptErrCodePtr->nErrorCode == nErrCode)
		{
			if (szDescription != NULL)
				strcpy(szDescription, ptErrCodePtr->szDescription);
			if (bIsPrint)
			{
				sysprintf("Error - %s\n", ptErrCodePtr->szDescription);
			}
			return;
		}
		ptErrCodePtr++;
	}
	sysprintf("fsGetErrorDescription - undefined error nErrCode: %d\n", nErrCode);
}






