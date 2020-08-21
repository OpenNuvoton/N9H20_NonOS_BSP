/**************************************************************************//**
 * @file     SpiRead.c
 * @brief    SPI functions for Spiloader
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include "N9H20.h"

#define  SPICMD_DUMMY         0x00
#define  SPICMD_READ_DATA     0x03

#define CACHE_BIT31  BIT31

int spiActive(int port)
{
    outp32(REG_SPI0_CNTRL, inp32(REG_SPI0_CNTRL)|0x01);
    while(inp32(REG_SPI0_CNTRL) & 0x01);
    return 0;
}

int spiTxLen(int port, int count, int bitLen)
{
    UINT32 reg;

    reg = inp32(REG_SPI0_CNTRL);

    if ((count < 0) || (count > 3))
        return 1;

    if ((bitLen <= 0) || (bitLen > 32))
        return 1;

    if (bitLen == 32)
        reg = reg & 0xffffff07;
    else
        reg = (reg & 0xffffff07) | (bitLen << 3);

    reg = (reg & 0xfffffcff) | (count << 8);

    outp32(REG_SPI0_CNTRL, reg);

    return 0;
}

int usiCheckBusy()
{
    /* check status */
    outp32(REG_SPI0_SSR, inp32(REG_SPI0_SSR) | 1);    /* CS0 */

    /* status command */
    outp32(REG_SPI0_TX0, 0x05);
    spiTxLen(0, 0, 8);
    spiActive(0);

    /* get status */
    while(1)
    {
        outp32(REG_SPI0_TX0, 0xff);
        spiTxLen(0, 0, 8);
        spiActive(0);
        if (((inp32(REG_SPI0_RX0) & 0xff) & 0x01) != 0x01)
            break;
    }

    outp32(REG_SPI0_SSR, inp32(REG_SPI0_SSR) & 0xfe);  /* CS0 */

    return 0;
}

void EDMAMasterRead(UINT32 u32DestAddr, UINT32 u32Length)
{
    UINT32 u32SFRCSR,u32Value,u32SPIBitLen;

    /* Set Byte Endian */
    outp32(REG_SPI0_CNTRL, inp32(REG_SPI0_CNTRL) | 1<<20);

    u32SPIBitLen=0;

    outp32(REG_SPI0_CNTRL, (inp32(REG_SPI0_CNTRL) & ~Tx_BIT_LEN) | u32SPIBitLen<<3);    

    u32SFRCSR = REG_PDMA_CSR1;

    outp32(REG_PDMA_SAR1, REG_SPI0_RX0);             /* EDMA Source */
    outp32(REG_PDMA_DAR1,(u32DestAddr| 0x80000000)); /* EDMA Destination */
    outp32(REG_PDMA_BCR1, u32Length);                /* EDMA Byte Count */
    u32Value = 0x00000005;                           /* Src:Fix, Dest:Inc, IP2Mem, EDMA enable */
    //u32Value = u32Value | (eTransferWidth<<19);    /* Change APB_TWS */

    outp32(u32SFRCSR,u32Value);                      /* EDMA Transer width */

    outp32(u32SFRCSR,inp32(u32SFRCSR) | TRIG_EN);    /* Enable EDMA Trig_En */

    outp32(REG_SPI0_EDMA,0x03);                      /* Enable SPI EDMA Read & Start */
    outp32(REG_SPI0_CNTRL, inp32(REG_SPI0_CNTRL) |1);    /* master start */
    
    while(inp32(u32SFRCSR) & TRIG_EN);
    
}

int SPIReadFast(BOOL bEDMAread, UINT32 addr, UINT32 len, UINT32 *buf)
{
    int volatile i, j;
    UINT32 u32Count;
    //sysprintf("Load file length 0x%x, execute address 0x%x\n", len, (UINT32)buf);

    outp32(REG_SPI0_CNTRL, inp32(REG_SPI0_CNTRL) & ~(1<<20));    /* disabe BYTE endian */

    outp32(REG_SPI0_SSR, inp32(REG_SPI0_SSR) | 0x01);    /* CS0 */

    buf = (UINT32 *) ((UINT32)buf | (UINT32)CACHE_BIT31);

    /* read command */
    outp32(REG_SPI0_TX0, 0x0B);
    spiTxLen(0, 0, 8);
    spiActive(0);

    /* address */
    outp32(REG_SPI0_TX0, addr);
    spiTxLen(0, 0, 24);
    spiActive(0);

    /* dummy byte */
    outp32(REG_SPI0_TX0, 0xff);
    spiTxLen(0, 0, 8);
    spiActive(0);
    if(bEDMAread)
    {
        EDMAMasterRead((UINT32)buf, len);
    }
    else
    {
#if  0
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
            *buf++ = ((u32Tmp & 0xFF) << 24) | ((u32Tmp & 0xFF00) << 8) | ((u32Tmp & 0xFF0000) >> 8)| ((u32Tmp & 0xFF000000) >> 24);
        }
#else
        u32Count = len/16;
        if(len % 16)
            u32Count++;

        outpw(REG_SPI0_CNTRL, inpw(REG_SPI0_CNTRL) |BYTE_ENDIN);
        spiTxLen(0, 3, 32);

        // data
        for (i=0; i<u32Count; i++)
        {
            //spiTxLen(0, 3, 32);
            outpw(REG_SPI0_TX0, 0xffffffff);
            outpw(REG_SPI0_TX1, 0xffffffff);
            outpw(REG_SPI0_TX2, 0xffffffff);
            outpw(REG_SPI0_TX3, 0xffffffff);
            spiActive(0);
            for(j=0; j<4; j++)
            {
                *buf++ = inp32(REG_SPI0_RX0 + j*4);
            }
        }

        outpw(REG_SPI0_CNTRL, inpw(REG_SPI0_CNTRL) & ~BYTE_ENDIN);
        spiTxLen(0, 0, 32);
#endif
    }
    outp32(REG_SPI0_SSR, inp32(REG_SPI0_SSR) & 0xfe);    /* CS0 */

    return 0;
}

VOID SPI_OpenSPI(VOID)
{
    outp32(REG_APBCLK, inp32(REG_APBCLK) | SPIMS0_CKE);
    outp32(REG_GPDFUN, inp32(REG_GPDFUN) | (MF_GPD15|MF_GPD14|MF_GPD13|MF_GPD12));
    outp32(REG_SPI0_DIVIDER, 0x00);
}

VOID SPI_CloseSPI(VOID)
{
    outp32(REG_APBCLK, inp32(REG_APBCLK) & ~SPIMS0_CKE);
    /* Disable Pin function */
    outp32(REG_GPDFUN, inp32(REG_GPDFUN) & ~(MF_GPD15|MF_GPD14|MF_GPD13|MF_GPD12));
}
