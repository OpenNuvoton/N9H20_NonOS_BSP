/**************************************************************************//**
 * @file     main.c
 * @brief    USB Host integration test program source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#ifdef ECOS
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "drv_api.h"
#include "diag.h"
#include "wbtypes.h"
#include "wbio.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wbtypes.h"
#include "wblib.h"
#endif

#include "N9H20.h"
/* USB */
#include "usbvideo.h"
#include "usbkbd.h"

#define SYSTEM_CLOCK        12000000
#define UART_BAUD_RATE        115200

#define DUMMY_BUFFER_SIZE        (64 * 1024)

#ifdef ECOS
#define sysGetTicks(TIMER0)   cyg_current_time()
#endif

UINT8  _JpegImage[256 * 1024] __attribute__((aligned(32)));
UINT8  _JpegImageR[256 * 1024] __attribute__((aligned(32)));

extern UINT32    _QueuedSize;

extern INT  W99683_HasImageQueued(VOID);

void  GetJpegImage(UINT8 *image, UINT32 *len, INT interval)
{
    UINT8   *bufPTR;
    INT     bufLEN;
    UINT32  idx = 0;

    *len = 0;
    /* Skip frames */
    while (1)
    {
        if (W99683_GetFramePiece(&bufPTR, &bufLEN) < 0) 
            ;           /* W99683 is not enabled, we wait */

        if ((bufPTR[0] == 0xFF) && (bufPTR[1] == 0xD8))
        {
            if (interval == 0)
                break;
            interval--;
        }
    }

    while (1)
    {
        memcpy(&image[idx], bufPTR, bufLEN);
        idx += bufLEN;
        *len += bufLEN;

        if (W99683_GetFramePiece(&bufPTR, &bufLEN) < 0) 
            continue;        /* W99683 is not enabled, we wait */

        if ((bufPTR[0] == 0xFF) && (bufPTR[1] == 0xD8))
            return;
    }
}

static INT  Action_Compare(CHAR *suFileName1, CHAR *szAsciiName1,
                           CHAR *suFileName2, CHAR *szAsciiName2)
{
    INT  hFile1, hFile2;
    INT  nLen1, nLen2, nCount, nStatus1, nStatus2;
    UINT8  buffer1[8192], buffer2[8192];
    UINT32  nJpegLen;
    
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

        GetJpegImage(_JpegImage, &nJpegLen, 0);
        
        if ((nStatus1 == ERR_FILE_EOF) && (nStatus2 == ERR_FILE_EOF))
        {
            sysprintf("\ncompare ok!\n");
            fsCloseFile(hFile1);
            fsCloseFile(hFile2);
            return 0;
        }

        if (nLen1 != nLen2)
            break;

        if (memcmp(buffer1, buffer2, 1024))
            break;
        
        nCount++;
//         if ((nCount % 1024) == 0)
//             sysprintf("%d KB    \r", nCount);
    }

    sysprintf("\nFile Compare failed at offset %x\n", nCount * 1024);

    fsCloseFile(hFile1);
    fsCloseFile(hFile2);
    return -1;
}

INT  copy_file(CHAR *suSrcName, CHAR *szSrcAsciiName,
               CHAR *suDstName, CHAR *szDstAsciiName)
{
    INT  hFileSrc, hFileDst, nByteCnt, nStatus;
    UINT8  *pucBuff;
    UINT32  nJpegLen;

    pucBuff = (UINT8 *)malloc(4096 + MAX_FILE_NAME_LEN + 512);
    if (pucBuff == NULL)
        return ERR_NO_FREE_MEMORY;


    hFileSrc = fsOpenFile(suSrcName, szSrcAsciiName, O_RDONLY);
    if (hFileSrc < 0)
    {
        free(pucBuff);
        return hFileSrc;
    }

    hFileDst = fsOpenFile(suDstName, szDstAsciiName, O_RDONLY);
    if (hFileDst > 0)
    {
        fsCloseFile(hFileSrc);
        fsCloseFile(hFileDst);
        free(pucBuff);
        return ERR_FILE_EXIST;
    }

    hFileDst = fsOpenFile(suDstName, szDstAsciiName, O_CREATE);
    if (hFileDst < 0)
    {
        fsCloseFile(hFileSrc);
        free(pucBuff);
        return hFileDst;
    }

    while (1)
    {
        nStatus = fsReadFile(hFileSrc, pucBuff, 4096, &nByteCnt);
        if (nStatus < 0)
            break;

        nStatus = fsWriteFile(hFileDst, pucBuff, nByteCnt, &nByteCnt);
        if (nStatus < 0)
            break;

        GetJpegImage(_JpegImage, &nJpegLen, 0);
    }
    fsCloseFile(hFileSrc);
    fsCloseFile(hFileDst);
    free(pucBuff);

    if (nStatus == ERR_FILE_EOF)
        nStatus = 0;
    return nStatus;
}

INT Test()
{
    INT  nStatus;
    CHAR  szSrcA[24] = "C:\\1.mp4";
    CHAR  szDstA[24] = "C:\\copya";
    CHAR  suFileName1[64], suFileName2[64];
    UINT32  nJpegLen;

    while (1)
    {
        sysprintf("Delete file: %s\n", szDstA);
        fsAsciiToUnicode(szDstA, suFileName1, TRUE);
        nStatus = fsDeleteFile(suFileName1, NULL);
        if (nStatus < 0)
            sysprintf("Failed, status = %x\n", nStatus);

        while (_QueuedSize > 16*1024)
        {
            GetJpegImage(_JpegImage, &nJpegLen, 0);
            sysprintf(".");
        }

        sysprintf("Copy file: %s\n", szSrcA);
        fsAsciiToUnicode(szSrcA, suFileName1, TRUE);
        fsAsciiToUnicode(szDstA, suFileName2, TRUE);
        nStatus = copy_file(suFileName1, NULL, suFileName2, NULL);
        if (nStatus < 0)
        {
            sysprintf("Failed, status = %x\n", nStatus);
            exit(0);
        }

        sysprintf("Compare file: %s and %s\n", szSrcA, szDstA);
        fsAsciiToUnicode(szSrcA, suFileName1, TRUE);
        fsAsciiToUnicode(szDstA, suFileName2, TRUE);
        
        if (Action_Compare(suFileName1, NULL, suFileName2, NULL) < 0)
            break;
    }
    return 0;
}


void  Isochronous_Test()
{
    CHAR  szFileName[32] = {'C',0,':',0,'\\',0,'1',0,0,0 };
    CHAR  suFileName[128];
    INT  nIdx = 0;
    UINT32  nJpegLen;
    INT  hFile;
    INT  nWriteBytes;

    W99683Cam_Init();

    while (!W99683Cam_IsConnected())
#ifdef ECOS
        Hub_CheckIrqEvent(0);
#else    
        Hub_CheckIrqEvent();
#endif

    if (W99683Cam_Open() < 0)
    {
        sysprintf("Failed to open W99683 device!\n");
        return;           /* _W99683_Camera is freed also */
    }

    while (!W99683Cam_IsStreaming())
        ;

    /* Drop 5 pictures */
    for (nIdx = 0; nIdx < 10; nIdx++)
    {
        sysprintf("%d GetJpegImage...\n", nIdx);
        GetJpegImage(_JpegImage, &nJpegLen, 0);
        sysprintf("ImageSize: %d, _QueuedSize: %d\n", nJpegLen, _QueuedSize);
    }
    
    for (nIdx = 0; nIdx < 30; nIdx++)
    {
reget:
        GetJpegImage(_JpegImage, &nJpegLen, 0);
        if (_QueuedSize > 200000)
        {
            goto reget;
        }
        sysprintf("ImageSize: %d, _QueuedSize: %d\n", nJpegLen, _QueuedSize);
        
        /* Open a new file for writing */
        sprintf(szFileName, "C:\\%04d.jpg", nIdx);
        fsAsciiToUnicode(szFileName, suFileName, 1);
        hFile = fsOpenFile(suFileName, NULL, O_CREATE);
        if (hFile > 0)
            sysprintf("Opene file:[%s], file handle:%d\n", szFileName, hFile);
        else
        {
            sysprintf("Failed to open file: %s (%x)\n", szFileName, hFile);
            continue;
        }
        if ((fsWriteFile(hFile, _JpegImage, nJpegLen, &nWriteBytes) < 0) ||
            (nWriteBytes != nJpegLen))
            sysprintf("File write error! %d %d\n", nWriteBytes);
        else
            sysprintf("%d bytes\n", nWriteBytes);
        fsCloseFile(hFile);
    }
    
    while (1)
        GetJpegImage(_JpegImage, &nJpegLen, 0);
}


void IntegrationTest(void)
{
    INT  t0;
    UINT32  uBlockSize, uFreeSize, uDiskSize;

    sysprintf("Please plug in pen driver, key board and W99683 camera in advance\n");

    InitUsbSystem();
    UMAS_InitUmasDriver();
    USBKeyboardInit();
    /* wait hard disk ready */
    t0 = sysGetTicks(TIMER0);
    while (sysGetTicks(TIMER0) - t0 < 300)
        ;
    
    if (fsDiskFreeSpace('C', &uBlockSize, &uFreeSize, &uDiskSize) == 0)
        sysprintf("Disk size = %d KB, Free speace = %d KB\n", uDiskSize, uFreeSize);
    else
        sysprintf("fsDiskFreeSpace failed!!\n");
    
    Isochronous_Test();
}
/* Pen driver Connect/disconnect  Test */
volatile UINT32 i32MsConnect = 0;
volatile UINT32 i32MsDisConnect = 1;
volatile UINT32 i32AccessDone = 0;
VOID MassStotrageConnection(void* umas)
{
    volatile int i=0x20000;
    sysprintf("Umas driver connect  0x%x\n", (UINT32)umas);
    i32MsConnect = 1;
    i32MsDisConnect =0;
    while(i--);
}
VOID MassStotrageDisconnection(void* umas)
{
    sysprintf("Umas driver disconnect 0x%x\n", (UINT32)umas);
    i32MsConnect = 0;
    i32MsDisConnect = 1;
    i32AccessDone = 0;
}

/*************************************************************************************
 *  Test Condition:
 *   Plug in a pen driver to USBH port 1 or port 0. 
 *
 *************************************************************************************/
void PenDriverAccess(UINT32 u32Count)
{
    CHAR szFileName[32] = {'C',0,':',0,'\\',0,'1',0,0,0 };
    CHAR szAsciiStr[32]={0};
    CHAR suFileName[128];
    INT  nIdx = 0, nIdy=0;
    UINT32  nJpegLen= 256*1024;
    INT  hFile;
    INT  nWriteBytes;

    sprintf(szAsciiStr, "C:\\Test");
    fsAsciiToUnicode(szAsciiStr, suFileName, TRUE);
    fsMakeDirectory(suFileName, NULL);
    for (nIdx = 0; nIdx < u32Count; nIdx++)
    {
        for (nIdy = 0; nIdy < (nJpegLen); nIdy=nIdy+1)
            _JpegImage[nIdy] = (nIdy+(nIdy>>8)+(nIdy>>16))+nIdx;

        sysprintf("ImageSize: %d\n", nJpegLen);
        /* Open a new file for writing */
        sprintf(szFileName, "C:\\Test\\%07d.jpg", nIdx);
        fsAsciiToUnicode(szFileName, suFileName, 1);
        hFile = fsOpenFile(suFileName, NULL, O_CREATE);
        if (hFile > 0)
            sysprintf("Opene file:[%s], file handle:%d\n", szFileName, hFile);
        else
        {
            sysprintf("Failed to open file: %s (%x)\n", szFileName, hFile);
            continue;
        }
        if ((fsWriteFile(hFile, _JpegImage, nJpegLen, &nWriteBytes) < 0) ||
            (nWriteBytes != nJpegLen))
            sysprintf("File write error! %d %d\n", nWriteBytes);
        else
            sysprintf("%d bytes\n", nWriteBytes);
        fsCloseFile(hFile);
        hFile = fsOpenFile(suFileName, NULL, O_RDONLY );
        if (hFile > 0)
            sysprintf("Opene file:[%s], file handle:%d\n", szFileName, hFile);
        else
        {
            sysprintf("Opene file Error\n");
            while(1);
        }
        if ((fsReadFile(hFile, _JpegImageR, nJpegLen, &nWriteBytes) < 0) ||
            (nWriteBytes != nJpegLen))
            sysprintf("File read error! %d %d\n", nWriteBytes);
        fsCloseFile(hFile);
            
        if(memcmp(_JpegImage, _JpegImageR, nJpegLen)  != 0 )
        {
            sysprintf("Compare file error\n");
            while(1);
        }

    }
    sysprintf("Done\n");
}

void PenDriverConnectTest(void)
{
    UINT32  uBlockSize, uFreeSize, uDiskSize;
    
    umass_register_connect(MassStotrageConnection);
    umass_register_disconnect(MassStotrageDisconnection);
    InitUsbSystem();
    UMAS_InitUmasDriver();
    sysDelay(30);            /* Delay 300 ms */
    while(1)
    {
    	Hub_CheckIrqEvent();
        if(i32MsConnect ==1)
        {
            if(i32AccessDone==0)
            {
                if (fsDiskFreeSpace('C', &uBlockSize, &uFreeSize, &uDiskSize) == 0)
                {
                    sysprintf("Disk size = %d KB, Free speace = %d KB\n", uDiskSize, uFreeSize);
                }
                else
                {
                    sysprintf("fsDiskFreeSpace failed!!\n");
                    sysprintf("Pen driver may plug out !\n");
                }
                PenDriverAccess(1);
                i32AccessDone = 1;
                sysprintf("Please plug out the pen driver for test\n");
            }
        }
        if(i32MsDisConnect == 0)
            ;
        if(i32MsConnect == 0)
        {
        	sysDelay(100);
            sysprintf("Please plug in the pen driver for test\n");
        }
        Hub_CheckIrqEvent();
    }
}

INT main()
{
    UINT32     u32Item, u32ExtFreq;
    UINT32     u32PllOutKHz;
    WB_UART_T uart;
#ifndef ECOS
	

    sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
        
	u32ExtFreq = sysGetExternalClock();	
    sysUartPort(1);
    uart.uiFreq = u32ExtFreq*1000;    /* use APB clock */
    uart.uiBaudrate = 115200;
    uart.uiDataBits = WB_DATA_BITS_8;
    uart.uiStopBits = WB_STOP_BITS_1;
    uart.uiParity = WB_PARITY_NONE;
    uart.uiRxTriggerLevel = LEVEL_1_BYTE;
    sysInitializeUART(&uart);                

    sysSetSystemClock(eSYS_UPLL,     /* E_SYS_SRC_CLK eSrcClk, */
                    192000,          /* UINT32 u32PllKHz, */
                    192000,          /* UINT32 u32SysKHz, */
                    192000,          /* UINT32 u32CpuKHz, */
                    192000/2,        /* UINT32 u32HclkKHz, */
                    192000/4);       /* UINT32 u32ApbKHz */
    u32PllOutKHz = sysGetPLLOutputKhz(eSYS_UPLL, u32ExtFreq);
    sysprintf("PLL out frequency %d Khz\n", u32PllOutKHz);

	sysSetTimerReferenceClock (TIMER0, u32ExtFreq*1000);
	sysStartTimer(TIMER0, 100, PERIODIC_MODE);
    outp32(REG_AHBCLK, inp32(REG_AHBCLK) & ~USBD_CKE);
    outp32(REG_APBCLK, 0xFFFFFFFF);
#endif

    fsInitFileSystem();

    do
    {
        sysprintf("====================================================================\n");
        sysprintf("Please select the USB host port through Transceiver ports or  GPIO\n");
        sysprintf("[1] Transceiver port 0\n");
        sysprintf("[2] Transceiver port 0 and 1\n");
        sysprintf("[3] Host-like port (GPB0 , GPB1)\n");
        sysprintf("[4] Host-like port (GPA3 , GPA4)\n");
        sysprintf("====================================================================\n");
        u32Item = sysGetChar();
        switch(u32Item)
        {
            case '1':    USB_PortInit(HOST_NORMAL_PORT0_ONLY);    break;
            case '2':    USB_PortInit(HOST_NORMAL_TWO_PORT);      break;
            case '3':    USB_PortInit(HOST_LIKE_PORT0);           break;
            case '4':    USB_PortInit(HOST_LIKE_PORT1);           break;
        }
    }while((u32Item> '4') || (u32Item< '0'));

    sysprintf("=============================================================================================\n");
    sysprintf("[1] Pen driver connect/disconect test\n");
    sysprintf("[2] Integration test\n");
    u32Item = sysGetChar();
    switch(u32Item)
    {
        case '1':
                PenDriverConnectTest();
                break;
        case '2':
                IntegrationTest();
                break;
    }

    return 0;
}

