/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright (c) 2010 Nuvoton Technology Corp. All rights reserved.                                        */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "HIDTransferTest.h"
#include "HID.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define HID_PACKET_SIZE 64
#define USB_VID         0x0416  /* Vendor ID */
#define USB_PID         0x5020  /* Product ID */

#define HID_CMD_SIGNATURE   0x43444948

/* HID Transfer Commands */
#define HID_CMD_NONE			0x00
#define HID_CMD_ERASE			0x71
#define HID_CMD_READ			0xD2
#define HID_CMD_GET_VERSION  	0xD3
#define HID_CMD_GET_STATUS		0xD4
#define HID_CMD_GET_START_BLOCK 0xD5
#define HID_CMD_WRITE			0xC3
#define HID_CMD_UPDATE          0xB0
#define HID_CMD_EXIT			0xB1
#define HID_CMD_TEST			0xB4
#define HID_CMD_IMAGE_WRITE		0xC4
#define HID_CMD_SET_PARAM		0xC5
#define HID_CMD_GET_PARAM       0xD6


#define PAGE_SIZE       256
#define BLOCK_SIZE      0x10000
#define JPEG_SIZE      0x20000
#define END_TAG_SIZE     4
#define START_TAG_SIZE  16

#define WRITE_PAGESE   (BLOCK_SIZE / PAGE_SIZE)
#define FILENAME_SIZE  200

#define USB_TIME_OUT    100

// 僅有的一個應用程式物件

CWinApp theApp;

using namespace std;

int main(int argc, TCHAR* argv[], TCHAR* envp[]);

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
    int nRetCode = 0;

    // 初始化 MFC 並於失敗時列印錯誤
    if(!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
    {
        // TODO: 配合您的需要變更錯誤碼
        _tprintf(_T("嚴重錯誤: MFC 初始化失敗\n"));
        nRetCode = 1;
    }
    else
    {
        // TODO: 在此撰寫應用程式行為的程式碼。
        main(argc, argv, envp);




    }

    return nRetCode;
}

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

typedef struct
{
    unsigned char cmd;
    unsigned char len;
    unsigned int arg1;
    unsigned int arg2;
    unsigned int signature;
    unsigned int checksum;
} CMD_T;

#pragma pack(pop)   /* restore original alignment from stack */



unsigned int CalCheckSum(unsigned char *buf, unsigned int size)
{
    unsigned int sum;
    int i;

    i = 0;
    sum = 0;
    while(size--)
    {
        sum += buf[i++];
    }

    return sum;

}

/*
    This function is used to read data through USB HID.

    pReadBuf - [in ] The read buffer to store the data from USB HID. User must make sure its size is enough.
    startPage- [out] The start page to read. The page size should be dependent on SPI Flash.
    pages    - [out] The number of pages to read.

    return value is the total read bytes.
*/
int GetParameter(unsigned char *pReadBuf,unsigned int argv1,unsigned int argv2)
{
    CHidCmd io;
    CMD_T cmd;
    unsigned long length;
    BOOL bRet;
    unsigned long readBytes;
    bool isDeviceOpened;

    readBytes = 0;
    isDeviceOpened = 0;
    if(!io.OpenDevice(USB_VID, USB_PID))
    {
        printf("Can't Open HID Device\n");
        goto lexit;
    }
    else
    {
        isDeviceOpened = TRUE;
        memset((void *)&cmd , 0xFF, sizeof(CMD_T));
        cmd.cmd = HID_CMD_GET_PARAM;
        cmd.len = 0x0E; /* Not include checksum */
        cmd.arg1 = argv1;
        cmd.arg2 = argv2;
        cmd.signature = HID_CMD_SIGNATURE;
        cmd.checksum = CalCheckSum((unsigned char *)&cmd, cmd.len);

        bRet = io.WriteFile((unsigned char *)&cmd, sizeof(cmd), &length, USB_TIME_OUT);
        if(!bRet)
        {
            printf("ERROR: Send read command error!\n");
            goto lexit;
        }
        bRet = io.ReadFile(pReadBuf, 64, &length, USB_TIME_OUT);
        if(!bRet)
        {
            printf("ERROR: Read fail!\n");
            goto lexit;
        }
    }


lexit:

    if(isDeviceOpened)
        io.CloseDevice();

    return readBytes;
}



/*
    This function is used to program data to target device through USB HID.

    pWriteBuf- [in ] The buffer of programming data. User must make sure its size is enough.
    startPage- [out] The start page to program SPI Flash. The page size should be dependent on SPI Flash.
    pages    - [out] The number of pages to program.

    return value is the total programming bytes.
*/
int SetParameter(unsigned char *pWriteBuf, unsigned int argv1, unsigned int argv2)
{
    CHidCmd io;
    CMD_T cmd;
    unsigned long length;
    BOOL bRet;
    unsigned long writeBytes;
    bool isDeviceOpened;

    writeBytes = 0;
    isDeviceOpened = 0;
    if(!io.OpenDevice(USB_VID, USB_PID))
    {
        printf("Can't Open HID Device\n");
        goto lexit;
    }
    else
    {
        isDeviceOpened = TRUE;
        printf(">>> Set Parameter %d - %d = %d\n", argv1, argv2,*pWriteBuf);
        memset((void *)&cmd , 0xFF, sizeof(CMD_T));
        cmd.cmd = HID_CMD_SET_PARAM;
        cmd.len = 0x0E; /* Not include checksum */
        cmd.arg1 = argv1;
        cmd.arg2 = argv2;
        cmd.signature = HID_CMD_SIGNATURE;
        cmd.checksum = CalCheckSum((unsigned char *)&cmd, cmd.len);

        bRet = io.WriteFile((unsigned char *)&cmd, sizeof(cmd), &length, USB_TIME_OUT);
        if(!bRet)
        {
            printf("ERROR: Send read command error!\n");
            goto lexit;
        }
	//	Sleep(20);

        bRet = io.WriteFile(pWriteBuf, 64, &length, USB_TIME_OUT);
        if(!bRet)
        {
            printf("ERROR: Write fail!\n");
            goto lexit;
        }
    }

lexit:

    if(isDeviceOpened)
        io.CloseDevice();

    return length;
}

/*
    This function is used to read data through USB HID.

    pReadBuf - [in ] The read buffer to store the data from USB HID. User must make sure its size is enough.
    startPage- [out] The start page to read. The page size should be dependent on SPI Flash.
    pages    - [out] The number of pages to read.

    return value is the total read bytes.
*/
int GetVersion(unsigned char *pReadBuf)
{
    CHidCmd io;
    CMD_T cmd;
    unsigned long length;
    BOOL bRet;
    unsigned long readBytes;
    bool isDeviceOpened;

    readBytes = 0;
    isDeviceOpened = 0;
    if(!io.OpenDevice(USB_VID, USB_PID))
    {
        printf("Can't Open HID Device\n");
        goto lexit;
    }
    else
    {
        isDeviceOpened = TRUE;
        memset((void *)&cmd , 0xFF, sizeof(CMD_T));
        cmd.cmd = HID_CMD_GET_VERSION;
        cmd.len = 0x0E; /* Not include checksum */
        cmd.arg1 = 0;
        cmd.arg2 = 0;
        cmd.signature = HID_CMD_SIGNATURE;
        cmd.checksum = CalCheckSum((unsigned char *)&cmd, cmd.len);

        bRet = io.WriteFile((unsigned char *)&cmd, sizeof(cmd), &length, USB_TIME_OUT);
        if(!bRet)
        {
            printf("ERROR: Send read command error!\n");
            goto lexit;
        }
        bRet = io.ReadFile(pReadBuf, 64, &length, USB_TIME_OUT);
        if(!bRet)
        {
            printf("ERROR: Read fail!\n");
            goto lexit;
        }
    }


lexit:

    if(isDeviceOpened)
        io.CloseDevice();

    return readBytes;
}

int GetStartBlock(unsigned char *pReadBuf)
{
    CHidCmd io;
    CMD_T cmd;
    unsigned long length;
    BOOL bRet;
    unsigned long readBytes;
    bool isDeviceOpened;

    readBytes = 0;
    isDeviceOpened = 0;
    if(!io.OpenDevice(USB_VID, USB_PID))
    {
        printf("Can't Open HID Device\n");
        goto lexit;
    }
    else
    {
        isDeviceOpened = TRUE;
        memset((void *)&cmd , 0xFF, sizeof(CMD_T));
        cmd.cmd = HID_CMD_GET_START_BLOCK;
        cmd.len = 0x0E; /* Not include checksum */
        cmd.arg1 = 0;
        cmd.arg2 = 0;
        cmd.signature = HID_CMD_SIGNATURE;
        cmd.checksum = CalCheckSum((unsigned char *)&cmd, cmd.len);

        bRet = io.WriteFile((unsigned char *)&cmd, sizeof(cmd), &length, USB_TIME_OUT);
        if(!bRet)
        {
            printf("ERROR: Send read command error!\n");
            goto lexit;
        }
        bRet = io.ReadFile(pReadBuf, 64, &length, USB_TIME_OUT);
        if(!bRet)
        {
            printf("ERROR: Read fail!\n");
            goto lexit;
        }
    }


lexit:

    if(isDeviceOpened)
        io.CloseDevice();

    return readBytes;
}

/*
    This function is used to read data through USB HID.

    pReadBuf - [in ] The read buffer to store the data from USB HID. User must make sure its size is enough.
    startPage- [out] The start page to read. The page size should be dependent on SPI Flash.
    pages    - [out] The number of pages to read.

    return value is the total read bytes.
*/
int GetStatus(unsigned char *pReadBuf)
{
    CHidCmd io;
    CMD_T cmd;
    unsigned long length;
    BOOL bRet;
    unsigned long readBytes;
    bool isDeviceOpened;

    readBytes = 0;
    isDeviceOpened = 0;
    if(!io.OpenDevice(USB_VID, USB_PID))
    {
        printf("Can't Open HID Device\n");
        goto lexit;
    }
    else
    {
        isDeviceOpened = TRUE;
		memset((void *)&cmd , 0xFF, sizeof(CMD_T));
        cmd.cmd = HID_CMD_GET_STATUS;
        cmd.len = 0x0E; /* Not include checksum */
        cmd.arg1 = 0;
        cmd.arg2 = 0;
        cmd.signature = HID_CMD_SIGNATURE;
        cmd.checksum = CalCheckSum((unsigned char *)&cmd, cmd.len);

        bRet = io.WriteFile((unsigned char *)&cmd, sizeof(cmd), &length, USB_TIME_OUT);
        if(!bRet)
        {
            printf("ERROR: Send read command error!\n");
            goto lexit;
        }
        bRet = io.ReadFile(pReadBuf, 64, &length, USB_TIME_OUT);
        if(!bRet)
        {
            printf("ERROR: Read fail!\n");
            goto lexit;
        }
    }
lexit:

    if(isDeviceOpened)
        io.CloseDevice();
    return readBytes;
}


/*
    This function is used to read data through USB HID.

    pReadBuf - [in ] The read buffer to store the data from USB HID. User must make sure its size is enough.
    startPage- [out] The start page to read. The page size should be dependent on SPI Flash.
    pages    - [out] The number of pages to read.

    return value is the total read bytes.
*/
int ReadPages(unsigned char *pReadBuf, unsigned int startPage, unsigned int pages)
{
    CHidCmd io;
    CMD_T cmd;
    unsigned long length;
    BOOL bRet;
    unsigned long readBytes;
    bool isDeviceOpened;

    readBytes = 0;
    isDeviceOpened = 0;
    if(!io.OpenDevice(USB_VID, USB_PID))
    {
        printf("Can't Open HID Device\n");
        goto lexit;
    }
    else
    {
        isDeviceOpened = TRUE;
        printf(">>> Read pages: %d - %d\n", startPage, startPage + pages - 1);
        memset((void *)&cmd , 0xFF, sizeof(CMD_T));
        cmd.cmd = HID_CMD_READ;
        cmd.len = 0x0E; /* Not include checksum */
        cmd.arg1 = startPage;
        cmd.arg2 = pages;
        cmd.signature = HID_CMD_SIGNATURE;
        cmd.checksum = CalCheckSum((unsigned char *)&cmd, cmd.len);

        bRet = io.WriteFile((unsigned char *)&cmd, sizeof(cmd), &length, USB_TIME_OUT);
        if(!bRet)
        {
            printf("ERROR: Send read command error!\n");
            goto lexit;
        }

        while(1)
        {
            if(readBytes >= cmd.arg2 * PAGE_SIZE)
            {
                break;
            }

            bRet = io.ReadFile(pReadBuf + readBytes, 256, &length, USB_TIME_OUT);
            if(!bRet)
            {
                printf("ERROR: Read fail!\n");
                goto lexit;
            }
            readBytes += length;
        }

    }


lexit:

    if(isDeviceOpened)
        io.CloseDevice();

    return readBytes;
}

/*
    This function is used to erase sectors of target device.

    startSector- [out] The start sector to erase. The sector size should be dependent on SPI Flash.
    sectors    - [out] The number of sectors to erase.

    return value is the total erased sec.
*/
int EraseBlock(unsigned int startSector, unsigned int sectors)
{
    CHidCmd io;
    CMD_T cmd;
    unsigned long length;
    BOOL bRet;
    bool isDeviceOpened;
    unsigned int eraseCnt;

    eraseCnt = 0;
    isDeviceOpened = 0;
    if(!io.OpenDevice(USB_VID, USB_PID))
    {
        printf("Can't Open HID Device\n");
        goto lexit;
    }
    else
    {
        isDeviceOpened = TRUE;
        printf(">>> Erase Blocks: %d - %d\n", startSector, startSector + sectors - 1);
        memset((void *)&cmd , 0xFF, sizeof(CMD_T));
        cmd.cmd = HID_CMD_ERASE;
        cmd.len = 0x0E; /* Not include checksum */
        cmd.arg1 = startSector;
        cmd.arg2 = sectors;
        cmd.signature = HID_CMD_SIGNATURE;
        cmd.checksum = CalCheckSum((unsigned char *)&cmd, cmd.len);

        bRet = io.WriteFile((unsigned char *)&cmd, sizeof(cmd), &length, USB_TIME_OUT);
        if(!bRet)
        {
            printf("ERROR: Send erase command error!\n");
            goto lexit;
        }
        eraseCnt = sectors;
    }


lexit:

    if(isDeviceOpened)
        io.CloseDevice();

    return eraseCnt;
}


/*
    This function is used to program data to target device through USB HID.

    pWriteBuf- [in ] The buffer of programming data. User must make sure its size is enough.
    startPage- [out] The start page to program SPI Flash. The page size should be dependent on SPI Flash.
    pages    - [out] The number of pages to program.

    return value is the total programming bytes.
*/
int WritePages(unsigned char *pWriteBuf, unsigned int startPage, unsigned int pages)
{
    CHidCmd io;
    CMD_T cmd;
    unsigned long length;
    BOOL bRet;
    unsigned long writeBytes;
    bool isDeviceOpened;

    writeBytes = 0;
    isDeviceOpened = 0;
    if(!io.OpenDevice(USB_VID, USB_PID))
    {
        printf("Can't Open HID Device\n");
        goto lexit;
    }
    else
    {
        isDeviceOpened = TRUE;
        printf(">>> Write pages: %d - %d\n", startPage, startPage + pages - 1);
        memset((void *)&cmd , 0xFF, sizeof(CMD_T));
        cmd.cmd = HID_CMD_WRITE;
        cmd.len = 0x0E; /* Not include checksum */
        cmd.arg1 = startPage;
        cmd.arg2 = pages;
        cmd.signature = HID_CMD_SIGNATURE;
        cmd.checksum = CalCheckSum((unsigned char *)&cmd, cmd.len);

        bRet = io.WriteFile((unsigned char *)&cmd, sizeof(cmd), &length, USB_TIME_OUT);
        if(!bRet)
        {
            printf("ERROR: Send read command error!\n");
            goto lexit;
        }

        while(1)
        {
            if(writeBytes >= cmd.arg2 * PAGE_SIZE)
            {
                break;
            }

            bRet = io.WriteFile(pWriteBuf + writeBytes, HID_PACKET_SIZE, &length, USB_TIME_OUT);
            if(!bRet)
            {
                printf("ERROR: Write fail!\n");
                goto lexit;
            }
            writeBytes += length;
        }

    }


lexit:

    if(isDeviceOpened)
        io.CloseDevice();

    return writeBytes;
}

/*
    This function is used to program data to target device through USB HID.

    pWriteBuf- [in ] The buffer of programming data. User must make sure its size is enough.
    startPage- [out] The start page to program SPI Flash. The page size should be dependent on SPI Flash.
    pages    - [out] The number of pages to program.

    return value is the total programming bytes.
*/
int WriteImage(unsigned char *pWriteBuf, unsigned int size)
{
    CHidCmd io;
    CMD_T cmd;
    unsigned long length;
    BOOL bRet;
    unsigned long writeBytes;
    bool isDeviceOpened;

    writeBytes = 0;
    isDeviceOpened = 0;
    if(!io.OpenDevice(USB_VID, USB_PID))
    {
        printf("Can't Open HID Device\n");
        goto lexit;
    }
    else
    {
        isDeviceOpened = TRUE;
        printf(">>> Write Image: %d Bytes\n", size);
        memset((void *)&cmd , 0xFF, sizeof(CMD_T));
        cmd.cmd = HID_CMD_IMAGE_WRITE;
        cmd.len = 0x0E; /* Not include checksum */
        cmd.arg1 = size;
        cmd.arg2 = 0;
        cmd.signature = HID_CMD_SIGNATURE;
        cmd.checksum = CalCheckSum((unsigned char *)&cmd, cmd.len);

        bRet = io.WriteFile((unsigned char *)&cmd, sizeof(cmd), &length, USB_TIME_OUT);
        if(!bRet)
        {
            printf("ERROR: Send read command error!\n");
            goto lexit;
        }
		//Sleep(20);

        while(1)
        {
            if(writeBytes >= cmd.arg1)
            {
                break;
            }

            bRet = io.WriteFile(pWriteBuf + writeBytes, 64, &length, USB_TIME_OUT);
            if(!bRet)
            {
                printf("ERROR: Write fail!\n");
                goto lexit;
            }
            writeBytes += length;
			
		  //  Sleep(2);
        }

    }


lexit:

    if(isDeviceOpened)
        io.CloseDevice();

    return writeBytes;
}
/*
    This function is used to be an simple demo of send command. User may use it as an example to add new command.
*/
int SendTestCmd(void)
{
    CHidCmd io;
    CMD_T cmd;
    unsigned long length;
    BOOL bRet;
    bool isDeviceOpened;


    isDeviceOpened = 0;
    if(!io.OpenDevice(USB_VID, USB_PID))
    {
        printf("Can't Open HID Device\n");
        goto lexit;
    }
    else
    {
        isDeviceOpened = TRUE;
        printf(">>> Test command\n");
        memset((void *)&cmd , 0xFF, sizeof(CMD_T));
        cmd.cmd = HID_CMD_TEST;
        cmd.len = 0x0E; /* Not include checksum */
        cmd.arg1 = 0x12345678;
        cmd.arg2 = 0xabcdef01;
        cmd.signature = HID_CMD_SIGNATURE;
        cmd.checksum = CalCheckSum((unsigned char *)&cmd, cmd.len);

        bRet = io.WriteFile((unsigned char *)&cmd, sizeof(cmd), &length, USB_TIME_OUT);
        if(!bRet)
        {
            printf("ERROR: Send test command error!\n");
            goto lexit;
        }
    }


lexit:

    if(isDeviceOpened)
        io.CloseDevice();

    return 0;
}


int SendExitCmd(void)
{
    CHidCmd io;
    CMD_T cmd;
    unsigned long length;
    BOOL bRet;
    bool isDeviceOpened;

    isDeviceOpened = 0;
    if(!io.OpenDevice(USB_VID, USB_PID))
    {
        printf("Can't Open HID Device\n");
        goto lexit;
    }
    else
    {
        isDeviceOpened = TRUE;
        printf(">>> Exit command\n");
        memset((void *)&cmd , 0xFF, sizeof(CMD_T));
        cmd.cmd = HID_CMD_EXIT;
        cmd.len = 0x0E; /* Not include checksum */
        cmd.arg1 = 0x00000000;
        cmd.arg2 = 0x00000000;
        cmd.signature = HID_CMD_SIGNATURE;
        cmd.checksum = CalCheckSum((unsigned char *)&cmd, cmd.len);

        bRet = io.WriteFile((unsigned char *)&cmd, sizeof(cmd), &length, USB_TIME_OUT);
        if(!bRet)
        {
            printf("ERROR: Send test command error!\n");
            goto lexit;
        }
    }


lexit:

    if(isDeviceOpened)
        io.CloseDevice();

    return 0;
}


int SendUpdateCmd(void)
{
    CHidCmd io;
    CMD_T cmd;
    unsigned long length;
    BOOL bRet;
    bool isDeviceOpened;

    isDeviceOpened = 0;
    if(!io.OpenDevice(USB_VID, USB_PID))
    {
        printf("Can't Open HID Device\n");
        goto lexit;
    }
    else
    {
        isDeviceOpened = TRUE;
        printf(">>> Update command\n");
        memset((void *)&cmd , 0xFF, sizeof(CMD_T));
        cmd.cmd = HID_CMD_UPDATE;
        cmd.len = 0xE; /* Not include checksum */
        cmd.arg1 = 0x12345678;
        cmd.arg2 = 0xabcdef01;
        cmd.signature = HID_CMD_SIGNATURE;
        cmd.checksum = CalCheckSum((unsigned char *)&cmd, cmd.len);

        bRet = io.WriteFile((unsigned char *)&cmd, sizeof(cmd), &length, USB_TIME_OUT);
        if(!bRet)
        {
            printf("ERROR: Send test command error!\n");
            goto lexit;
        }
    }


lexit:

    if(isDeviceOpened)
        io.CloseDevice();

    return 0;
}

void WaitSpiReady(void)
{
	int u32Status;
	unsigned char Tmp[64];
	while(1)
	{
		GetStatus(Tmp);    
		u32Status = *(unsigned int *)Tmp;
		if(u32Status == 0)
		{
			break;
		}
//		Sleep(2);
	}
}
void WaitDeviceConnect(void)
{
    CHidCmd io;
	int status = 0;

    while(!io.OpenDevice(USB_VID, USB_PID))
    {
		switch(status & 0x03)
		{
			case 0:
				printf("\r-");
				break;
			case 1:
				printf("\r\\");
				break;
			case 2:
				printf("\r|");
				break;
			case 3:
				printf("\r/");
				break;
		}
		status++;
		Sleep(200);
    }
    printf("\r - Device Connected\n");
}
void EraseBlocks(unsigned int startBlock, unsigned int blocks)
{
    unsigned int i;
	for(i=0;i<blocks;i++)
	{
        EraseBlock(startBlock + i, 1);
	    WaitSpiReady();
	}
}

int main(int argc, TCHAR* argv[], TCHAR* envp[])
{
    unsigned int filesize, ret, offset;
    int block_count = 0;
    int pages = 0;
    int size = 0;
	int mode;
	unsigned int u32CurVersion;
	unsigned int u32Parameter;
	unsigned int u32UpdateVersion;
	FILE *fp;
    unsigned char write_buf[BLOCK_SIZE];
    unsigned char *jpeg_buf;
    unsigned char read_buf[BLOCK_SIZE];
	unsigned char Tmp[512];
    char filename[FILENAME_SIZE];
    char modename[10];
    char fileVer[64];
    char argv1name[10];
    char argv2name[10];
    char argv3name[10];
    int argv1, argv2, argv3;
	unsigned int START_TAG[4] = {0x4E565420, 0x00000000, 0x00000000, 0x2054564E};
	unsigned char END_TAG[4] = {0x5A, 0xA5, 0x5A, 0xA5};

	if(argc < 2)
	{
		printf("Usage          : HID_Transfer [mode] ...\n");
		printf("Get Parmeter   : HID_Transfer    0   [arg1] [arg2]\n");
		printf("Set Parmeter   : HID_Transfer    1   [arg1] [arg2] [Vaule]\n");
		printf("Write Image    : HID_Transfer    2   [file name]\n");
		printf("Firmware Update: HID_Transfer    3   [Version] [file name]\n");
		return 0;	
	}
    sprintf(modename,"%s", argv[1]); 
    mode = atoi(modename);

	if(mode == 0)    /* Get Parmeter */
	{
    	if(argc < 4)
    	{
		    printf("Get Parmeter   : HID_Transfer 0 [arg1] [arg2].\n");
	    	return 0;	
	    }	
        sprintf(argv1name,"%s", argv[2]); 
        sprintf(argv2name,"%s", argv[3]); 
        argv1 = atoi(argv1name);
        argv2 = atoi(argv2name);
        if(argv1 > 50 || argv2 > 50)
        {
		    printf("arg1 & arg2 should be less than 50.\n");
	    	return 0;	
        }
        
		printf("Wait Device Connect - VID %04X PID %04X\n",USB_VID,USB_PID);
        WaitDeviceConnect();
        GetParameter(Tmp,argv1,argv2);
        u32Parameter = *(unsigned int *)Tmp;
	    printf(" - Get Parameter %d - %d : %d\n",argv1, argv2, u32Parameter);
	    return 0;	
   	}
	else if(mode == 1)    /* Set Parmeter */
	{		
    	if(argc < 5)
    	{
		printf("Set Parmeter   : HID_Transfer 1 [arg1] [arg2] [Vaule]...\n");
	    	return 0;	
	    }	
        sprintf(argv1name,"%s", argv[2]); 
        sprintf(argv2name,"%s", argv[3]); 
        sprintf(argv3name,"%s", argv[4]); 
        argv1 = atoi(argv1name);
        argv2 = atoi(argv2name);
        argv3 = atoi(argv3name);
        if(argv1 > 50 || argv2 > 50)
        {
		    printf("arg1 & arg2 should be less than 50.\n");
	    	return 0;	
        }
        
		printf("Wait Device Connect - VID %04X PID %04X\n",USB_VID,USB_PID);
        WaitDeviceConnect();
        *(unsigned int *)Tmp = argv3;
        SetParameter(Tmp,argv1,argv2);
	    printf(" - Set Parameter %d - %d : %d\n",argv1, argv2, argv3);
	    return 0;		
	}
	else if(mode == 2)    /* Write Image */
	{
        if(argc < 3)
    	{
		    printf("Write Image    : HID_Transfer 2 [file name].\n");
	    	return 0;	
	    }
		
    	if(_tcslen(argv[2])/sizeof(TCHAR) > FILENAME_SIZE)
	    {
	    	printf("File name is too long!!\n");
	    	return 0;
	    }
retry0:
	    sprintf(filename,"%s", argv[2]); 

	    fp = fopen(filename,"rb");

    	if(fp == NULL)
	    {
	    	printf("Can't Open File - %s!!\n",filename);
	    	return -1;
	    }
        else
        {
    	    fseek(fp, 0, SEEK_END);
	        filesize = ftell(fp);
	        fseek(fp, 0, SEEK_SET);

	        printf("Image Information\n");
	        printf(" - File Name : %s\n",filename);
	        printf(" - File Size : %d Bytes\n",filesize);
	        
    	    printf("Wait Device Connect - VID %04X PID %04X\n",USB_VID,USB_PID);
            WaitDeviceConnect();
            jpeg_buf = (unsigned char *) malloc(filesize);
            size = fread(jpeg_buf, 1, filesize, fp);
            
			size = (size + 0x1FF) & ~0x1FF;
			
            WriteImage(jpeg_buf,size);

	        fclose(fp);
		    free(jpeg_buf);
        }	
	}
	else if(mode == 3)    /* Update mode */
	{
    	if(argc < 3)
		{
		    printf("Wait Device Connect - VID %04X PID %04X\n",USB_VID,USB_PID);
            WaitDeviceConnect();
       	    GetStartBlock(Tmp);
            block_count = *(unsigned int *)Tmp;
	        printf(" - Start Block : %d\n",block_count);
	    	EraseBlocks(block_count, 1);  
	        WaitSpiReady();  /* Device will erase the end tag, so host needs to wait Erase CMD complete */
            SendUpdateCmd();

    	    WaitSpiReady();  /* Device will erase the end tag, so host needs to wait Erase CMD complete */

	    	return 0;	
    	}
    	else if(argc < 4)
    	{
	    	printf("Usage : HID_Transfer 1 [Version] [file name]\n");
	    	return 0;	
	    }
		
    	if(_tcslen(argv[3])/sizeof(TCHAR) > FILENAME_SIZE)
	    {
	    	printf("File name is too long!!\n");
	    	return 0;
	    }
retry:
	    sprintf(filename,"%s", argv[3]); 
	    sprintf(fileVer,"%s", argv[2]); 
    	u32UpdateVersion = atoi(fileVer);

	    fp = fopen(filename,"rb");

    	if(fp == NULL)
	    {
	    	printf("Can't Open File - %s!!\n",filename);
	    	return -1;
	    }
        else
        {
    	    fseek(fp, 0, SEEK_END);
	        filesize = ftell(fp);
	        fseek(fp, 0, SEEK_SET);

	        printf("Update Firmware Information\n");
	        printf(" - File Name : %s\n",filename);
	        printf(" - File Size : %d Bytes\n",filesize);
	        printf(" - File Version : %d\n",u32UpdateVersion);
    	    printf("Wait Device Connect - VID %04X PID %04X\n",USB_VID,USB_PID);
            WaitDeviceConnect();

        	START_TAG[1] = u32UpdateVersion;
        	START_TAG[2] = filesize;

	        printf("Current Firmware Information\n");
            GetVersion(Tmp);
            u32CurVersion = *(unsigned int *)Tmp;
	        printf(" - File Version : %d\n",u32CurVersion);

            if(u32CurVersion >= u32UpdateVersion)
	            printf("*The Update Firmware Version is older than Current Firmware\n");
	        else
	            printf("*The Update Firmware Version is newer than Current Firmware\n");

	    	Sleep(1000);

            GetStartBlock(Tmp);
            block_count = *(unsigned int *)Tmp;
	        printf(" - Start Block : %d\n",block_count);
    
	        WaitSpiReady();  /* Device will erase the end tag, so host needs to wait Erase CMD complete */

    	    offset = START_TAG_SIZE;  /* Start Tag */

            SendUpdateCmd();

	        WaitSpiReady();  /* Device will erase the end tag, so host needs to wait Erase CMD complete */

            while(filesize)
	        {
	        	if((filesize + offset) > BLOCK_SIZE)
	        	{
                    EraseBlocks(block_count, 1);   

		            fread(write_buf + offset, 1, BLOCK_SIZE - offset, fp);

			        filesize -= (BLOCK_SIZE - offset);

			        if(offset)
			        {
			    	    memcpy(write_buf, START_TAG, START_TAG_SIZE);
		    		    offset = 0;
			        }

                    WritePages(write_buf,block_count * WRITE_PAGESE, WRITE_PAGESE);

                    ReadPages(read_buf, block_count * WRITE_PAGESE, WRITE_PAGESE);

                    ret = memcmp(write_buf, read_buf, BLOCK_SIZE);

		        	if(ret)
		        	{
		    	    	printf("Compare Fail\n");
			        	return 0;
			        }

			        block_count++;
	        	}
	    	    else
		        {
                    EraseBlocks(block_count,1);   
			
	    	    	memset(write_buf, 0xFF, BLOCK_SIZE);	

		            fread(write_buf + offset, 1, filesize, fp);

		        	pages = (filesize + offset) / PAGE_SIZE;

                    if((filesize + offset) % PAGE_SIZE)
			        	pages++;	

		        	if(offset)
		        	{
			        	memcpy(write_buf, START_TAG, START_TAG_SIZE);
			        	offset = 0;
		        	}	

                    WritePages(write_buf, block_count * WRITE_PAGESE, pages);
                    ReadPages(read_buf, block_count * WRITE_PAGESE, pages);

                    ret = memcmp(write_buf, read_buf, filesize);
	    		    if(ret)
		        	{
			    		printf("Compare Fail\n");
				        return 0;
		        	}
			        filesize = 0;
		        }
        	}


            printf("Write END TAG\n");
            memset(write_buf, 0xFF, PAGE_SIZE);	
            memcpy(write_buf, END_TAG, END_TAG_SIZE);
	    	if(WRITE_PAGESE == pages)
                EraseBlocks(block_count + 1,1);   
		
            WritePages(write_buf, block_count * WRITE_PAGESE + pages, 1);
            ReadPages(read_buf, block_count * WRITE_PAGESE + pages , 1);
		
            ret = memcmp(write_buf, read_buf, PAGE_SIZE);
            if(ret)
            {
                printf("Compare Fail\n");
                return 0;
            }

	        fclose(fp);
	    	SendExitCmd();
        }
	}
    return 0;
}