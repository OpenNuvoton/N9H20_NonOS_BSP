/****************************************************************************
 * @file     spiflash.c
 * @version  V1.00
 * $Revision: 4 $
 * $Date: 18/04/25 11:43a $
 * @brief    SpiWriter source file
 *
 * @note
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#include <string.h>
#include "N9H20.h"

//#define __DEBUG__

#ifdef __DEBUG__
    #define PRINTF sysprintf
#else
    #define PRINTF(...)
#endif

#define SPIFLASH_WINBOND        0xEF
#define SPIFLASH_SST            0xBF
#define SPIFLASH_MXIC           0xC2
#define SPIFLASH_GIGADEVICE     0xC8
#define SPIFLASH_PAGE_SIZE      256
#define SPIFLASH_SECTOR_SIZE    0x1000
#define SPIFLASH_BLOCK_SIZE     0x10000
#define SPIFLASH_W25P           0x10    /* W25P series */
#define SPIFLASH_W25X           0x40    /* W25X series */
#define SPIFLASH_W25Q           0x50    /* W25X series */
#define SPIFLASH_MX25L160       0x15    /* 16M-bits    */
#define SPIFLASH_MX25L320       0x16    /* 32M-bits    */
#define SPIFLASH_MX25L640       0x17    /* 64M-bits    */
#define SPIFLASH_MX25L256       0x19    /* 256M-bits   */

UINT32 volatile g_SPI_SIZE = 0;
BOOL volatile g_4byte_adderss = FALSE;
UINT8 volatile  SPI_ID[4] = {0};

typedef struct
{
    char PID;
    int (*SpiFlashWrite)(UINT32 address, UINT32 len, UINT8 *data);
} spiflash_t;

int wbSpiWrite(UINT32 addr, UINT32 len, UINT8 *buf);
int sstSpiWrite(UINT32 addr, UINT32 len, UINT8 *buf);

int gSpiReadCMD = 0x03;

spiflash_t spiflash[]={
    {0xEF, wbSpiWrite},
    {0xBF, sstSpiWrite},
    {0xC2, wbSpiWrite},
    {0xC8, wbSpiWrite},
    {0x00, wbSpiWrite}
};

/*****************************************/

INT32 volatile _spi_type = -1;

int spiActive(int port)
{
    outpw(REG_SPI0_CNTRL, inpw(REG_SPI0_CNTRL)|0x01);
    while(inpw(REG_SPI0_CNTRL) & 0x01);
    return Successful;
}

int spiTxLen(int port, int count, int bitLen)
{
    UINT32 reg;

    reg = inpw(REG_SPI0_CNTRL);

    if ((count < 0) || (count > 3))
        return -1;

    if ((bitLen <= 0) || (bitLen > 32))
        return -1;

    if (bitLen == 32)
        reg = reg & 0xffffff07;
    else
        reg = (reg & 0xffffff07) | (bitLen << 3);
    reg = (reg & 0xfffffcff) | (count << 8);

    outpw(REG_SPI0_CNTRL, reg);

    return Successful;
}

int usiCheckBusy()
{
    /* check status */
    outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) | 0x01); /* CS0 */

    /* status command */
    outpw(REG_SPI0_TX0, 0x05);
    spiTxLen(0, 0, 8);
    spiActive(0);

    /* get status */
    while(1)
    {
        outpw(REG_SPI0_TX0, 0xff);
        spiTxLen(0, 0, 8);
        spiActive(0);
        if (((inpw(REG_SPI0_RX0) & 0xff) & 0x01) != 0x01)
            break;
    }

    outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & 0xfe); /* CS0 */

    return Successful;
}

void Enter4ByteMode(void)
{

    outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) | 0x01);  /* CS0 */

    /* erase command */
    outpw(REG_SPI0_TX0, 0xB7);
    spiTxLen(0, 0, 8);
    spiActive(0);

    outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & 0xfe);  /* CS0 */

    /* check status */
    usiCheckBusy();
}

void Exit4ByteMode(void)
{
    outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) | 0x01);  /* CS0 */

    /* erase command */
    outpw(REG_SPI0_TX0, 0xE9);
    spiTxLen(0, 0, 8);
    spiActive(0);

    outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & 0xfe);  /* CS0 */

    /* check status */
    usiCheckBusy();  
}


/*
    addr: memory address 
    len: byte count
    buf: buffer to put the read back data
*/
int usiRead(UINT32 addr, UINT32 len, UINT8 *buf)
{
    int volatile i;
    UINT32 u32Tmp,u32Count;
    PUINT32 u32Buf;
    if(!g_4byte_adderss)
    {
        if(addr & 0xFF000000)
        {
            Enter4ByteMode();
            g_4byte_adderss = TRUE;
            PRINTF("SPI 4 Byte Address Mode Enable (R)\n");
        }
        else if ((addr+len-1) & 0xFF000000)
        {
            Enter4ByteMode();
            g_4byte_adderss = TRUE;
            PRINTF("SPI 4 Byte Address Mode Enable (W1)\n");
        }
    }

    outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) | 0x01);  /* CS0 */

    /* read command */
    outpw(REG_SPI0_TX0, gSpiReadCMD);
    spiTxLen(0, 0, 8);
    spiActive(0);

    /* address */
    outpw(REG_SPI0_TX0, addr);
    if(g_4byte_adderss)
        spiTxLen(0, 0, 32);
    else
        spiTxLen(0, 0, 24);
    spiActive(0);

#if 1
    u32Buf = (UINT32 *)buf; 
    u32Count = len/4;
    if(len % 4)
        u32Count++;
    /* data */
    for (i=0; i<u32Count; i++)
    {
        outp32(REG_SPI0_TX0, 0xffffffff);
        spiTxLen(0, 0, 32);
        spiActive(0);
        u32Tmp = inp32(REG_SPI0_RX0);
        *u32Buf++ = ((u32Tmp & 0xFF) << 24) | ((u32Tmp & 0xFF00) << 8) | ((u32Tmp & 0xFF0000) >> 8)| ((u32Tmp & 0xFF000000) >> 24);
    }
#else
    /* data */
    for (i=0; i<len; i++)
    {
        outpw(REG_SPI0_TX0, 0xff);
        spiTxLen(0, 0, 8);
        spiActive(0);
        *buf++ = inpw(REG_SPI0_RX0) & 0xff;
    }
#endif
    outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & 0xfe);  /* CS0 */
    return Successful;
}


int usiWriteEnable()
{
    outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) | 0x01);  /* CS0 */

    outpw(REG_SPI0_TX0, 0x06);
    spiTxLen(0, 0, 8);
    spiActive(0);

    outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & 0xfe);  /* CS0 */

    return Successful;
}

int usiWriteDisable()
{
    outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) | 0x01);  /* CS0 */

    outpw(REG_SPI0_TX0, 0x04);
    spiTxLen(0, 0, 8);
    spiActive(0);

    outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & 0xfe);  /* CS0 */

    return Successful;
}

/*
    addr: memory address
    len: byte count
    buf: buffer with write data
*/
int usiWrite(UINT32 addr, UINT32 len, UINT8 *buf)
{
    return spiflash[_spi_type].SpiFlashWrite(addr, len, buf);
}

int usiEraseSector(UINT32 addr, UINT32 secCount)
{
    int volatile i;

    if ((addr % (64*1024)) != 0)
        return -1;

    if(!g_4byte_adderss)
    {
        if(addr & 0xFF000000)
        {
            Enter4ByteMode();
            g_4byte_adderss = TRUE;
            PRINTF("SPI 4 Byte Address Mode Enable (R)\n");
        }
        else if ((addr+secCount*64*1024-1) & 0xFF000000)
        {
            Enter4ByteMode();
            g_4byte_adderss = TRUE;
            PRINTF("SPI 4 Byte Address Mode Enable (W1)\n");
        }
    }

    for (i=0; i<secCount; i++)
    {
        usiWriteEnable();

        outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) | 0x01);  /* CS0 */

        /* erase command */
        outpw(REG_SPI0_TX0, 0xd8);
        spiTxLen(0, 0, 8);
        spiActive(0);

        /* address */
        outpw(REG_SPI0_TX0, addr+i*64*1024);
        if(g_4byte_adderss)
            spiTxLen(0, 0, 32);
        else
            spiTxLen(0, 0, 24);
        spiActive(0);

        outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & 0xfe);  /* CS0 */

        /* check status */
        usiCheckBusy();
    }
    return Successful;
}

int usiWaitEraseReady(void)
{
    outpw(REG_SPI0_TX0, 0xff);
    spiTxLen(0, 0, 8);
    spiActive(0);
    if (((inpw(REG_SPI0_RX0) & 0xff) & 0x01) != 0x01)
    {
        outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & 0xfe);  /* CS0 */
        return 0;       /* ready */
    }
    else
        return 0xFF;    /* busy */
}

VOID SpiReset(VOID)
{
    outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) | 0x01);  /* CS0 */

    outpw(REG_SPI0_TX0, 0x66);
    spiTxLen(0, 0, 8);
    spiActive(0);

    outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & 0xfe);  /* CS0 */

    outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) | 0x01);  /* CS0 */

    outpw(REG_SPI0_TX0, 0x99);
    spiTxLen(0, 0, 8);
    spiActive(0);

    outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & 0xfe);  /* CS0 */
}
int usiEraseAll(void)
{
    usiWriteEnable();

    outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) | 0x01);  /* CS0 */

    outpw(REG_SPI0_TX0, 0xc7);
    spiTxLen(0, 0, 8);
    spiActive(0);

    outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & 0xfe);  /* CS0 */

    /* check status */
    outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) | 0x01);  /* CS0 */

    outpw(REG_SPI0_TX0, 0x05);
    spiTxLen(0, 0, 8);
    spiActive(0);

    /* get status */
    outpw(REG_SPI0_TX0, 0xff);
    spiTxLen(0, 0, 8);
    spiActive(0);
    if (((inpw(REG_SPI0_RX0) & 0xff) & 0x01) != 0x01)
    {
        outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & 0xfe);  /* CS0 */
        return 0;       /* ready */
    }
    else
        return 0xFF;    /* busy */
}


INT16 usiReadID()
{
    UINT32 volatile id;
    int volatile i;
    UINT8 u8DeviceID, u8Capaticy, u8MemType;

    outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) | 0x01);  /* CS0 */

    /* command 8 bit */
    outpw(REG_SPI0_TX0, 0x9F);
    spiTxLen(0, 0, 8);
    spiActive(0);

    /* data 16 bit */
    outpw(REG_SPI0_TX0, 0xFFFFFFFF);
    spiTxLen(0, 0, 24);
    spiActive(0);
    id = inpw(REG_SPI0_RX0);
    outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & 0xfe);  /* CS0 */

    u8DeviceID = (id & 0xff0000) >> 16;
    u8MemType = (id & 0xff00) >> 8;
    u8Capaticy = id & 0xFF;

    SPI_ID[0] = u8Capaticy;
    SPI_ID[1] = u8MemType;
    SPI_ID[2] = u8DeviceID;
    SPI_ID[3] = 0;
    /* find spi flash */
    i=0;
    while(spiflash[i].PID)
    {
        if( spiflash[i].PID == u8DeviceID)
        {
            _spi_type=i;
            break;
        }
        i++;
    }
    if(_spi_type == -1)
        _spi_type = 4;

    sysprintf("SPI ID 0x%X\n",id & 0xFFFFFF);
    if((u8DeviceID == 0x00) || (u8DeviceID == 0xFF))
        g_SPI_SIZE = 0;
    /* Calculate Size */
    if( u8DeviceID  == SPIFLASH_WINBOND)   /* Winbond */
    {
        if(u8MemType == 0x20)
            u8DeviceID = u8Capaticy-1;
        else
            u8DeviceID = u8MemType+(u8Capaticy-1);

        switch(u8DeviceID & 0xF0)
        {
            case SPIFLASH_W25P:
                sysprintf("Winbond SPI Flash W25P\n");
                g_SPI_SIZE = (0x1 << (u8DeviceID & 0xF)) * 0x200;
                break;
            case SPIFLASH_W25X:
            case SPIFLASH_W25Q:
                sysprintf("Winbond SPI Flash W25X/W25Q\n");
                g_SPI_SIZE = (0x1 << (u8DeviceID & 0xF)) * 0x200;
                break;
            default:
                return 0x0;
        }
        if(g_SPI_SIZE != 0)
            g_SPI_SIZE = g_SPI_SIZE * 256;

        if(u8MemType == 0x40 && u8Capaticy == 0x19)
        {
            gSpiReadCMD = 0x13;
            sysprintf("Use 0x13 for 4-Byte Address Read Command\n");
        }
    }
    else if (u8DeviceID == SPIFLASH_SST)
    {
        sysprintf("SST SPI Flash\n");
    }
    else if(u8DeviceID  == SPIFLASH_GIGADEVICE)
    {
        sysprintf("GigaDevice SPI Flash\n");
        g_SPI_SIZE = 1 << (u8Capaticy );
    }
    else if (u8DeviceID == SPIFLASH_MXIC)
    {
        switch(u8Capaticy)
        {
            case SPIFLASH_MX25L160:    /* 16M-bits */
                sysprintf("MXIC SPI Flash MX25L160\n");
                g_SPI_SIZE = 8192;
                break;
            case SPIFLASH_MX25L320:    /* 32M-bits */
                sysprintf("MXIC SPI Flash MX25L320\n");
                g_SPI_SIZE = 16384;
                break;
            case SPIFLASH_MX25L640:    /* 64M-bits */
                sysprintf("MXIC SPI Flash MX25L640\n");
                g_SPI_SIZE = 32768;
                break;
            case SPIFLASH_MX25L256:    /* 256M-bits */
                sysprintf("MXIC SPI Flash MX25L256\n");
                g_SPI_SIZE = 131072;
                break;
        }
        if(g_SPI_SIZE != 0)
            g_SPI_SIZE = g_SPI_SIZE * 256;
    }
    if(g_SPI_SIZE > 0x1000000)
    {
        sysprintf("Spiflash is larger than 16MB --> Enter 4 Byte address mode\n");
        g_4byte_adderss = TRUE;
        Enter4ByteMode();
    }       
    return id;
}


int usiStatusWrite(UINT8 data)
{
    usiWriteEnable();

    outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) | 0x01);  /* CS0 */

    /* status command */
    outpw(REG_SPI0_TX0, 0x01);
    spiTxLen(0, 0, 8);
    spiActive(0);

    /* write status */
    outpw(REG_SPI0_TX0, data);
    spiTxLen(0, 0, 8);
    spiActive(0);

    outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & 0xfe);  /* CS0 */

    return Successful;
}


BOOL volatile _usbd_bIsSPIInit = FALSE;
int usiInit(void)
{
    int volatile loop;
    if (_usbd_bIsSPIInit == FALSE)
    {
        outpw(REG_APBCLK, inpw(REG_APBCLK) | SPIMS0_CKE);  /* turn on SPI clk */
        /* Reset SPI controller first */
        outpw(REG_APBIPRST, SPI0RST);
        /* Delay */
        for (loop=0; loop<500; loop++);

        outpw(REG_APBIPRST, 0);

        /* Startup SPI0 multi-function features, chip select using SS0 */
        outpw(REG_GPDFUN, inpw(REG_GPDFUN) | (MF_GPD15|MF_GPD14|MF_GPD13|MF_GPD12));

        /* configure SPI divider, the sclk = 60 / 8 = 7.5Mhz */
        outpw(REG_SPI0_DIVIDER, 0x0000) ;       /* (3+1)*2=8, N=3 */

        outpw(REG_SPI0_SSR, 0x00);      /* CS active low */
        outpw(REG_SPI0_CNTRL, 0x04);    /* Tx: falling edge, Rx: rising edge */

        SpiReset();

        if (usiReadID() == -1)
        {
            sysprintf("read id error !! [0x%x]\n", spiflash[_spi_type].PID);
            return -1;
        }

        //sysprintf("SPI flash id [0x%x]\n", spiflash[_spi_type].PID);
        usiStatusWrite(0x00);  /* clear block protect */
        /* Delay */
        for (loop=0; loop<0x20000; loop++);

        _usbd_bIsSPIInit = TRUE;
    }
    return 0;
 
} /* end usiInit */
#if 0
int DelSpiImage(UINT32 imageNo)
{
    int i, count;
    unsigned int start, length;
    unsigned int *ptr;

    pInfo = (UINT8 *)((UINT32)infoBuf | 0x80000000);

    usiRead(0, 64*1024, (UINT8 *)pInfo);

    ptr = (unsigned int *)(pInfo + 63*1024);

    if (((*(ptr+0)) == 0xAA554257) && ((*(ptr+3)) == 0x63594257))
    {
        count = *(ptr+1);
        *(ptr+1) = count - 1;  /* del one image */

        /* pointer to image information */
        ptr += 4;
        for (i=0; i<count; i++)
        {
            if ((*(ptr) & 0xffff) == imageNo)
            {
                start = (*(ptr + 1) & 0xffff) * 0x10000;
                length = *(ptr + 3);
                break;
            }
            /* pointer to next image */
            ptr = ptr+12;
        }
        /* Shift the information after the deleted image */
        memcpy((char *)ptr, (char *)(ptr+12), (count-i-1)*48);
    }

    if (start == 0)
        memset(pInfo, 0xFF, length);

    usiEraseSector(0, 1);  /* erase sector 0 */

    usiWrite(0, 64*1024, pInfo);

    /* erase the sector */
    if (start != 0)
    {
        count = length / (64 * 1024);
        if ((length % (64 * 1024)) != 0)
            count++;   
        usiEraseSector(start, count);
    }
    return Successful;
}

int ChangeSpiImageType(UINT32 imageNo, UINT32 imageType)
{
    int volatile i, count;
    unsigned int *ptr;

    pInfo = (UINT8 *)((UINT32)infoBuf | 0x80000000);

    /* Read Entire Block 0 */
    usiRead(0, 64*1024, (UINT8 *)pInfo);
    /* Parse Block 64 to get image information */
    ptr = (unsigned int *)(pInfo + 63*1024);

    if (((*(ptr+0)) == 0xAA554257) && ((*(ptr+3)) == 0x63594257))
    {
        count = *(ptr+1);
        /* pointer to image information */
        ptr += 4;
        for (i=0; i<count; i++)
        {
            if ((*ptr & 0xffff) == imageNo)
            {
                *ptr = ((imageType & 0xffff) << 16) | (imageNo & 0xffff);
                break;
            }
            /* pointer to next image */
            ptr = ptr+12;
        }
    }

    if (usiEraseSector(0, 1) < 0)  /* erase sector 0 */
        return -1;

    usiWrite(0, 64*1024, pInfo);

    return Successful;
}


int SetSpiImageInfo(FW_UPDATE_INFO_T *spiImageInfo)
{
    int i, count=0;
    unsigned int *ptr, *pImage;

    pInfo = (UINT8 *)((UINT32)infoBuf | 0x80000000);

    usiRead(0, 64*1024, (UINT8 *)pInfo);
    ptr = (unsigned int *)(pInfo + 63*1024);

    pImage = ptr+4;

    if (((*(ptr+0)) == 0xAA554257) && ((*(ptr+3)) == 0x63594257))
    {
        count = *(ptr+1);

        /* pointer to image information */
        for (i=0; i<count; i++)
        {
            if ((*pImage & 0xffff) == spiImageInfo->imageNo)
            {
                pUpdateImage = pImage;
                bIsMatch = TRUE;
                break;
            }
            else
            {
                bIsMatch = FALSE;
                /* pointer to next image */
                pImage += 12;
            }
        }
    }

    /* update image information */
    *(ptr+0) = 0xAA554257;
    if (!bIsMatch)
    {
        *(ptr+1) = count+1;
        pUpdateImage = (ptr+4) + (count * 12);
    }
    *(ptr+3) = 0x63594257;
    bIsMatch = FALSE;

    *(pUpdateImage+0) = (spiImageInfo->imageNo & 0xffff) | ((spiImageInfo->imageFlag & 0xffff) << 16);  /* image number / flag */
    *(pUpdateImage+1) = (spiImageInfo->startBlock&0xffff) | ((spiImageInfo->endBlock&0xffff) << 16);
    *(pUpdateImage+2) = spiImageInfo->executeAddr;
    sysprintf("Image %d Execute address 0x%08X\n",spiImageInfo->imageNo,spiImageInfo->executeAddr);
    *(pUpdateImage+3) = spiImageInfo->fileLen;
    memcpy((char *)(pUpdateImage+4), spiImageInfo->imageName, 32);  /* image name */

    usiEraseSector(0, 1);  /* erase sector 0 */

    usiWrite(0, 64*1024, pInfo);

    return Successful;
}

#endif
/******************************************/
/* write function for different spi flash */
/******************************************/
int wbSpiWrite(UINT32 addr, UINT32 len, UINT8 *buf)
{
    int volatile count=0, page, i,j;
    PUINT32 u32Buf;
    UINT32 u32Tmp,u32Count;

    if(!g_4byte_adderss)
    {
        if(addr & 0xFF000000)
        {
            Enter4ByteMode();
            g_4byte_adderss = TRUE;
            PRINTF("SPI 4 Byte Address Mode Enable(W0)\n");
        }
        else if ((addr+len-1) & 0xFF000000)
        {
            Enter4ByteMode();
            g_4byte_adderss = TRUE;
            PRINTF("SPI 4 Byte Address Mode Enable (W1)\n");
        }
    }
    count = len / 256;
    if ((len % 256) != 0)
        count++;

    for (i=0; i<count; i++)
    {
        /* check data len */
        if (len >= 256)
        {
            page = 256;
            len = len - 256;
        }
        else
            page = len;

        usiWriteEnable();

        outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) | 0x01);  /* CS0 */

        /* write command */
        outpw(REG_SPI0_TX0, 0x02);
        spiTxLen(0, 0, 8);
        spiActive(0);

        /* address */
        outpw(REG_SPI0_TX0, addr+i*256);

        if(g_4byte_adderss)
            spiTxLen(0, 0, 32);
        else
            spiTxLen(0, 0, 24);
        spiActive(0);
#if 1 
        u32Buf = (UINT32 *)(buf + i*256); 
        u32Count = page/4;
        if(page % 4)
            u32Count++;
        /* data */
        for (j=0; j<u32Count; j++)
        {
            u32Tmp = *u32Buf++;
            outpw(REG_SPI0_TX0,  ((u32Tmp & 0xFF) << 24) | ((u32Tmp & 0xFF00) << 8) | ((u32Tmp & 0xFF0000) >> 8)| ((u32Tmp & 0xFF000000) >> 24));
            spiTxLen(0, 0, 32);
            spiActive(0);
        }
#else
        /* write data */
        while (page-- > 0)
        {
            outpw(REG_SPI0_TX0, *buf++);
            spiTxLen(0, 0, 8);
            spiActive(0);
        }
#endif
        outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & 0xfe);  /* CS0 */

        /* check status */
        usiCheckBusy();
    }

    return Successful;
}

int sstSpiWrite(UINT32 addr, UINT32 len, UINT8 *buf)
{
    if(!g_4byte_adderss)
    {
        if(addr & 0xFF000000)
        {
            Enter4ByteMode();
            g_4byte_adderss = TRUE;
            PRINTF("SPI 4 Byte Address Mode Enable (W0)\n");
        }
        else if ((addr+len-1) & 0xFF000000)
        {
            Enter4ByteMode();
            g_4byte_adderss = TRUE;
            PRINTF("SPI 4 Byte Address Mode Enable (W1)\n");
        }
    }
    while (len > 0)
    {
        usiWriteEnable();

        outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) | 0x01);  /* CS0 */

        /* write command */
        outpw(REG_SPI0_TX0, 0x02);
        spiTxLen(0, 0, 8);
        spiActive(0);

        /* address */
        outpw(REG_SPI0_TX0, addr++);
        if(g_4byte_adderss)
            spiTxLen(0, 0, 32);
        else
            spiTxLen(0, 0, 24);
        spiActive(0);

        /* write data */
        outpw(REG_SPI0_TX0, *buf++);
        spiTxLen(0, 0, 8);
        spiActive(0);

        outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & 0xfe);  /* CS0 */

        /* check status */
        usiCheckBusy();

        len--;
    }

    return Successful;
}
#if 0
INT nvtGetSpiImageInfo(unsigned int *image)
{
    UINT32 volatile bmark, emark;
    int volatile i, imageCount=0;
    unsigned int *ptr;

    pInfo = (UINT8 *)((UINT32)infoBuf | 0x80000000);
    ptr = (unsigned int *)((UINT32)infoBuf | 0x80000000);
    memset(pInfo, 0, 1024);

    usiRead(63*1024, 1024, pInfo);  /* offset, len, address */

    bmark = *(ptr+0);
    emark = *(ptr+3);

    if ((bmark == 0xAA554257) && (emark == 0x63594257))
    {
        imageCount = *(ptr+1);

        /* pointer to image information */
        ptr = ptr+4;
        for (i=0; i<imageCount; i++)
        {
            /* fill into the image list buffer */
            *image = 0;      /* action flag, dummy */
            *(image+1) = 0;  /* file len, dummy */
            *(image+2) = *(ptr) & 0xffff;  /* imageNo */
            memcpy((char *)(image+3), (char *)(ptr+4), 32);  /* image name */
            *(image+11) = (*(ptr) >> 16) & 0xffff;     /* image flag */
            *(image+12) = (*(ptr+1) >> 16) & 0xffff;  /* end block */
            *(image+13) = *(ptr+1) & 0xffff; /* start block */
            *(image+14) = 0;

            /* pointer to next image */
            image += 15;
            ptr = ptr+12;
        }
    }
    else
        imageCount = 0;

    return imageCount;
}

#endif
