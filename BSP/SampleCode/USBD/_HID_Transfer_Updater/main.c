#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wblib.h"
#include "N9H20.h"
#include "N9H20_USBD.h"
#include "HID.h"

extern UINT32 volatile u32Exit;
#define SPI_PAGE_SIZE 0x100


VOID SPI_OpenSPI(VOID);

#if defined (__GNUC__)
UINT8  image_buffer[512] __attribute__((aligned(32)));
#else
UINT8 __align(32) image_buffer[512];
#endif

unsigned char *imagebuf;
unsigned int *pImageList;
extern unsigned int g_UpdateStartBlock, g_EndTagBlock;
extern unsigned int g_Version;

void GetInfo(void)
{
    unsigned int startBlock,endBlock;
    unsigned int fileLen;
    unsigned int executeAddr;
    unsigned int start_addr, size;
    int count, i, j;

    imagebuf = (UINT8 *)((UINT32)image_buffer | 0x80000000);
    pImageList=((unsigned int *)(((unsigned int)image_buffer)|0x80000000));

    /* Initial DMAC and NAND interface */
    SPI_OpenSPI();

    memset(imagebuf, 0, 32);
    sysprintf("Load Image ");
    /* read image information */
    SPIReadFast(0, 63*1024, 512, (UINT32*)imagebuf);  /* offset, len, address */

    if (((*(pImageList+0)) == 0xAA554257) && ((*(pImageList+3)) == 0x63594257))
    {
        count = *(pImageList+1);

        pImageList=((unsigned int*)(((unsigned int)image_buffer)|0x80000000));
        startBlock = fileLen = executeAddr = 0;

        /* load execution file */
        pImageList = pImageList+4;
        for (i=0; i<count; i++)
        {
            if (((*(pImageList) >> 16) & 0xffff) == 1)    /* execute */
            {
                startBlock = *(pImageList + 1) & 0xffff;       
                endBlock =  ((*(pImageList + 1) >> 16) & 0xffff);
                g_UpdateStartBlock = endBlock + 1;
                executeAddr = *(pImageList + 2);
                fileLen = *(pImageList + 3);

                sysprintf("Default Firmware : %d ~ %d (%d)\n", startBlock, endBlock,fileLen);

                g_EndTagBlock = 0;

                SPIReadFast(0, g_UpdateStartBlock * BLOCK_SIZE, 512, (UINT32*)pImageList);  /* offset, len, address */

                if((*(pImageList + 0) == 0x4E565420) && (*(pImageList + 3) == 0x2054564E))
                {
                    int size = *(pImageList + 2);
                    int ver =  *(pImageList + 1);
                    int offset = (((size + 16) + (SPI_PAGE_SIZE - 1)) / SPI_PAGE_SIZE) * SPI_PAGE_SIZE;
                    g_EndTagBlock = (g_UpdateStartBlock * BLOCK_SIZE + offset) / BLOCK_SIZE;

                    SPIReadFast(0, g_UpdateStartBlock * BLOCK_SIZE + offset, 4, (UINT32*)pImageList);  /* offset, len, address */

                    if(*pImageList == 0xA55AA55A)
                    {
                        g_Version = ver;
                        sysprintf("New Firmware (Ver. %d): %d Bytes\n",g_Version, size);
                    }
                }
                break;
            }
            /* pointer to next image */
            pImageList = pImageList+12;
        }
    }
}

int main(void)
{
    WB_UART_T uart;
    UINT32 u32ExtFreq;
    sysUartPort(1);
    u32ExtFreq = sysGetExternalClock();    /* Hz unit */
    uart.uiFreq = u32ExtFreq*1000;
    uart.uiBaudrate = 115200;
    uart.uiDataBits = WB_DATA_BITS_8;    
    uart.uiStopBits = WB_STOP_BITS_1;
    uart.uiParity = WB_PARITY_NONE;
    uart.uiRxTriggerLevel = LEVEL_1_BYTE;
    sysInitializeUART(&uart);	
    sysEnableCache(CACHE_WRITE_BACK);

    sysprintf("Sample code Start\n");	

    /* Enable USB */
    udcOpen();  

    spiFlashInit();
    GetInfo();
    usiCheckBusy();
    hidInit();
    udcInit();

    while(!u32Exit);
    sysprintf("Exit\n");
    while(1);
}

