/*-----------------------------------------------------------------------------------*/
/* Nuvoton Technology Corporation confidential                                       */
/*                                                                                   */
/* Copyright (c) 2008 by Nuvoton Technology Corporation                              */
/* All rights reserved                                                               */
/*                                                                                   */
/*-----------------------------------------------------------------------------------*/
#include <string.h>
#include "wblib.h"
#include "turbowriter.h"

//#define _fmi_uFMIReferenceClock   240000
int volatile _fmi_uFMIReferenceClock;

#define SD_FREQ     24000
#define SDHC_FREQ   24000
#define MMC_FREQ    20000

FMI_SD_INFO_T SDInfo, *pSD;
extern UINT8 dummy_buffer[];

//--- define type of SD card or MMC
#define FMI_TYPE_UNKNOWN                0
#define FMI_TYPE_SD_HIGH                1
#define FMI_TYPE_SD_LOW                 2
#define FMI_TYPE_MMC                    3   // MMC access mode: Byte mode for capacity <= 2GB
#define FMI_TYPE_MMC_SECTOR_MODE        4   // MMC access mode: Sector mode for capacity > 2GB

// global variables
UINT32 _fmi_uR3_CMD=0;
UINT32 _fmi_uR7_CMD=0;

void fmiCheckRB()
{
    while(1)
    {
        outpw(REG_SDCR, inpw(REG_SDCR)|0x40);
        while(inpw(REG_SDCR) & 0x40);
        if (inpw(REG_SDISR) & 0x80)
            break;
    }
}

INT fmiSDCommand(UINT8 ucCmd, UINT32 uArg)
{
    outpw(REG_SDARG, uArg);
    outpw(REG_SDCR, (inpw(REG_SDCR)&0xffffc080)|(ucCmd << 8)|(0x01));

    while(inpw(REG_SDCR) & 0x01)
    {
        if (pSD->bIsCardInsert == FALSE)
            return -1;
    }

    return Successful;
}


INT fmiSDCmdAndRsp(UINT8 ucCmd, UINT32 uArg, INT ntickCount)
{
    unsigned int reg;
    int volatile count=0;

    outpw(REG_SDARG, uArg);
    reg = inpw(REG_SDCR) & 0xffffc080;
    outpw(REG_SDCR, reg | (ucCmd << 8)|(0x03));

    if (ntickCount > 0)
    {
        while(inpw(REG_SDCR) & 0x02)
        {
            if (count++ > ntickCount)
            {
                outpw(REG_SDCR, inpw(REG_SDCR)|0x4000); // reset SD engine
                return 2;
            }

            if (pSD->bIsCardInsert == FALSE)
                return -1;
        }
    }
    else
    {
        while(inpw(REG_SDCR) & 0x02)
        {
            if (pSD->bIsCardInsert == FALSE)
                return -1;
        }
    }

    if (_fmi_uR7_CMD)
    {
        if (((inpw(REG_SDRSP1) & 0xff) != 0x55) && ((inpw(REG_SDRSP0) & 0xf) != 0x01))
        {
            _fmi_uR7_CMD = 0;
            return -1;
        }
    }

    if (!_fmi_uR3_CMD)
    {
        if (inpw(REG_SDISR) & 0x04)     // check CRC7
            return Successful;
        else
        {
#ifdef DEBUG
            sysprintf("response error [%d]!\n", ucCmd);
#endif
            return -1;
        }
    }
    else
    {
        _fmi_uR3_CMD = 0;
        outpw(REG_SDISR, 0x02);
        return Successful;
    }
}


// Get 16 bytes CID or CSD
INT fmiSDCmdAndRsp2(UINT8 ucCmd, UINT32 uArg, UINT *puR2ptr)
{
    unsigned int i;
    unsigned int tmpBuf[5];
    unsigned int reg;

    outpw(REG_SDARG, uArg);
    reg = inpw(REG_SDCR) & 0xffffc080;
    outpw(REG_SDCR, reg | (ucCmd << 8)|(0x11));

    while(inpw(REG_SDCR) & 0x10)
    {
        if (pSD->bIsCardInsert == FALSE)
            return -1;
    }

    if (inpw(REG_SDISR) & 0x04)
    {
        for (i=0; i<5; i++)
            tmpBuf[i] = Swap32(inpw(REG_FB_0+i*4));

        for (i=0; i<4; i++)
            *puR2ptr++ = ((tmpBuf[i] & 0x00ffffff)<<8) | ((tmpBuf[i+1] & 0xff000000)>>24);
        return Successful;
    }
    else
        return -1;
}


INT fmiSDCmdAndRspDataIn(UINT8 ucCmd, UINT32 uArg)
{
    unsigned int reg;

    outpw(REG_SDARG, uArg);
    reg = inpw(REG_SDCR) & 0xffffc080;
    outpw(REG_SDCR, reg | (ucCmd << 8)|(0x07));

    while (inpw(REG_SDCR) & 0x02)
    {
        if (pSD->bIsCardInsert == FALSE)
            return -1;
    }

    while (inpw(REG_SDCR) & 0x04)
    {
        if (pSD->bIsCardInsert == FALSE)
            return -1;
    }

    /* According to comment in Linux driver mmc_sd_init_card() in core/sd.c, 
       "some SDHC cards are not able to provide valid CRCs for non-512-byte blocks",
       SDLoader don't check CRC-16 to improve compatibility.
     */
    if (inpw(REG_SDISR) & 0x04)     // check CRC7
        return Successful;
    else
    {
#ifdef DEBUG
        sysprintf("fmiSDCmdAndRspDataIn: response error [%d]!\n", ucCmd);
#endif
        return -1;
    }

//    if (!(inpw(REG_SDISR) & 0x08))      // check CRC16
//    {
//#ifdef DEBUG
//        sysprintf("fmiSDCmdAndRspDataIn: read data error!\n");
//#endif
//        return -1;
//    }
//    return Successful;
}


// Initial
INT fmiSD_Init()
{
    int volatile i, status, rate;
    unsigned int resp;
    unsigned int CIDBuffer[4];

#if 1
    // set the clock to 200KHz
    /* divider */
    rate = _fmi_uFMIReferenceClock / 200;
    if ((_fmi_uFMIReferenceClock % 200) == 0)
        rate = rate - 1;
#else
    // set the clock to 400KHz
    /* divider */
    rate = _fmi_uFMIReferenceClock / 400;
    if ((_fmi_uFMIReferenceClock % 400) == 0)
        rate = rate - 1;
#endif

    for(i=0; i<100; i++);
    outpw(REG_CLKDIV2, (inpw(REG_CLKDIV2) & ~SD_S) | (0x03 << 19));     // SD clock from UPLL

    outpw(REG_CLKDIV2, (inpw(REG_CLKDIV2) & ~SD_N0) | (0x07 << 16));    // SD clock divided by 8
    rate /= 8;
    rate &= 0xFF;
    if (rate) rate--;
    outpw(REG_CLKDIV2, (inpw(REG_CLKDIV2) & ~SD_N1) | (rate << 24));    // SD clock divider

    for(i=0; i<1000; i++);

    // power ON 74 clock
    outpw(REG_SDCR, inpw(REG_SDCR) | SDCR_74CLK_OE);

    while(inpw(REG_SDCR) & SDCR_74CLK_OE)
    {
        if (pSD->bIsCardInsert == FALSE)
            return -1;
    }

    fmiSDCommand(0, 0);     // reset all cards
    for (i=0x100; i>0; i--);

    // initial SDHC
    _fmi_uR7_CMD = 1;
    i = fmiSDCmdAndRsp(8, 0x00000155, 4000);
    _fmi_uR7_CMD = 0;   // Disable R7 checking for commands that are not CMD8
    if (i == Successful)
    {
        // SD 2.0
        fmiSDCmdAndRsp(55, 0x00, 0);
        _fmi_uR3_CMD = 1;
        fmiSDCmdAndRsp(41, 0x40ff8000, 0);  // 2.7v-3.6v
        resp = inpw(REG_SDRSP0);
        while (!(resp & 0x00800000))        // check if card is ready
        {
            fmiSDCmdAndRsp(55, 0x00, 0);
            _fmi_uR3_CMD = 1;
            fmiSDCmdAndRsp(41, 0x40ff8000, 0);  // 3.0v-3.4v
            resp = inpw(REG_SDRSP0);
        }
        if (resp & 0x00400000)
            pSD->uCardType = FMI_TYPE_SD_HIGH;
        else
            pSD->uCardType = FMI_TYPE_SD_LOW;
    }
    else
    {
        // SD 1.1
        i = fmiSDCmdAndRsp(55, 0x00, 6000);
        if (i == 2)     // MMC memory
        {
            fmiSDCommand(0, 0);     // reset
            for (i=0x100; i>0; i--);

            _fmi_uR3_CMD = 1;
            // 2014/8/6, to support eMMC v4.4, the argument of CMD1 should be 0x40ff8000 to support both MMC plus and eMMC cards.
            if (fmiSDCmdAndRsp(1, 0x40ff8000, 5000) != 2)   // MMC memory
            {
                resp = inpw(REG_SDRSP0);
                while (!(resp & 0x00800000))    // check if card is ready
                {
                    _fmi_uR3_CMD = 1;
                    fmiSDCmdAndRsp(1, 0x40ff8000, 0);   // high voltage
                    resp = inpw(REG_SDRSP0);
                }
                // MMC card is ready. Check the access mode of MMC card.
                if (resp & 0x00400000)
                {
                    pSD->uCardType = FMI_TYPE_MMC_SECTOR_MODE;
                    //sysprintf("--> MMC card report Ready and Sector Mode (>2GB) !\n");
                }
                else
                {
                    pSD->uCardType = FMI_TYPE_MMC;
                    //sysprintf("--> MMC card report Ready and Byte Mode (<=2GB) !\n");
                }
            }
            else
            {
                pSD->uCardType = FMI_TYPE_UNKNOWN;
                return -1;
            }
        }
        else if (i == 0)    // SD Memory
        {
            _fmi_uR3_CMD = 1;
            fmiSDCmdAndRsp(41, 0x00ff8000, 0);  // 3.0v-3.4v
            resp = inpw(REG_SDRSP0);
            while (!(resp & 0x00800000))        // check if card is ready
            {
                fmiSDCmdAndRsp(55, 0x00, 0);
                _fmi_uR3_CMD = 1;
                fmiSDCmdAndRsp(41, 0x00ff8000, 0);  // 3.0v-3.4v
                resp = inpw(REG_SDRSP0);
            }
            pSD->uCardType = FMI_TYPE_SD_LOW;
        }
        else
        {
            pSD->uCardType = FMI_TYPE_UNKNOWN;
#ifdef DEBUG
            sysprintf("CMD55 CRC error !!\n");
#endif
            return -1;
        }
    }

    // CMD2, CMD3
    if (pSD->uCardType != FMI_TYPE_UNKNOWN)
    {
        fmiSDCmdAndRsp2(2, 0x00, CIDBuffer);
        if (pSD->uCardType == FMI_TYPE_MMC)
        {
            if ((status = fmiSDCmdAndRsp(3, 0x10000, 0)) != Successful)     // set RCA
                return status;
            pSD->uRCA = 0x10000;
        }
        else
        {
            if ((status = fmiSDCmdAndRsp(3, 0x00, 0)) != Successful)        // get RCA
                return status;
            else
                pSD->uRCA = (inpw(REG_SDRSP0) << 8) & 0xffff0000;
        }
    }

#ifdef DEBUG
    switch (pSD->uCardType)
    {
        case FMI_TYPE_SD_HIGH:
            sysprintf("This is high capacity SD memory card\n");       break;
        case FMI_TYPE_SD_LOW:
            sysprintf("This is standard capacity SD memory card\n");   break;
        case FMI_TYPE_MMC:
            sysprintf("This is standard capacity MMC memory card\n");  break;
        case FMI_TYPE_MMC_SECTOR_MODE:
            sysprintf("This is high capacity MMC memory card\n");      break;
        default:
            sysprintf("ERROR: Unknown card type !!\n");                break;
    }
#endif

    // set data transfer clock

    return Successful;
}

INT fmiSwitchToHighSpeed()
{
    UINT8 *Info;
    int volatile status=0;
    UINT16 current_comsumption, /*fun1_info, switch_status,*/ busy_status0;
#ifdef DEBUG
    UINT16 busy_status1;
#endif

    Info = (UINT8 *)((UINT32)dummy_buffer | 0x80000000);
    outpw(REG_DMACSAR, (UINT32)Info);   // set DMA transfer starting address
    outpw(REG_SDBLEN, 63);  // 512 bit

    if ((status = fmiSDCmdAndRspDataIn(6, 0x00ffff01)) != Successful)
        return Fail;

    current_comsumption = dummy_buffer[0]<<8 | dummy_buffer[1];
    if (!current_comsumption)
        return Fail;

    //fun1_info =  dummy_buffer[12]<<8 | dummy_buffer[13];

    //switch_status =  dummy_buffer[16] & 0xf;

    busy_status0 = dummy_buffer[28]<<8 | dummy_buffer[29];

    if (!busy_status0)  // function ready
    {
        outpw(REG_DMACSAR, (UINT32)Info);   // set DMA transfer starting address
        outpw(REG_SDBLEN, 63);  // 512 bit

        if ((status = fmiSDCmdAndRspDataIn(6, 0x80ffff01)) != Successful)
            return Fail;

        // function change timing: 8 clocks
        outpw(REG_SDCR, inpw(REG_SDCR)|0x40);
        while(inpw(REG_SDCR) & 0x40);

        current_comsumption = dummy_buffer[0]<<8 | dummy_buffer[1];
        if (!current_comsumption)
            return Fail;

#ifdef DEBUG
        busy_status1 = dummy_buffer[28]<<8 | dummy_buffer[29];
        if (!busy_status1)
            sysprintf("switch into high speed mode !!!\n");
#endif
        return Successful;
    }
    else
        return Fail;
}


INT fmiSelectCard()
{
    UINT8 *Info;
    int volatile status=0;
    UINT32 arg;

    if ((status = fmiSDCmdAndRsp(7, pSD->uRCA, 0)) != Successful)
        return status;
    fmiCheckRB();

    // if SD card set 4bit
    if (pSD->uCardType == FMI_TYPE_SD_HIGH)
    {
        Info = (UINT8 *)((UINT32)dummy_buffer | 0x80000000);
        outpw(REG_DMACSAR, (UINT32)Info);   // set DMA transfer starting address
        outpw(REG_SDBLEN, 7);   // 64 bit

        if ((status = fmiSDCmdAndRsp(55, pSD->uRCA, 0)) != Successful)
            return status;
        if ((status = fmiSDCmdAndRspDataIn(51, 0x00)) != Successful)
            return status;

        {
            int volatile rate, i;
            if ((dummy_buffer[0] & 0xf) == 0x2)
            {
                status = fmiSwitchToHighSpeed();
                if (status == Successful)
                {
                    /* divider */
                    rate = _fmi_uFMIReferenceClock / SDHC_FREQ;
            //      if ((_fmi_uFMIReferenceClock % SDHC_FREQ) == 0)
            //          rate = rate - 1;
                    if ((_fmi_uFMIReferenceClock % SDHC_FREQ) > 0)
                        rate ++;

                    for(i=0; i<100; i++);

                    outpw(REG_CLKDIV2, (inpw(REG_CLKDIV2) & ~SD_S) | (0x03 << 19));     // SD clock from UPLL
                    outpw(REG_CLKDIV2, (inpw(REG_CLKDIV2) & ~SD_N0) | (0x01 << 16));    // SD clock divided by 8

                    if (rate % 2)
                    {
                        rate /= 2;
                        rate &= 0xFF;
                    }
                    else
                    {
                        rate /= 2;
                        rate &= 0xFF;
                        rate--;
                    }

                    outpw(REG_CLKDIV2, (inpw(REG_CLKDIV2) & ~SD_N1) | (rate << 24));    // SD clock divider
                    for(i=0; i<1000; i++);
                }
            }
        }

        if ((status = fmiSDCmdAndRsp(55, pSD->uRCA, 0)) != Successful)
            return status;
        if ((status = fmiSDCmdAndRsp(6, 0x02, 0)) != Successful)    // set bus width
            return status;

        outpw(REG_SDCR, inpw(REG_SDCR)|0x8000);
    }
    else if (pSD->uCardType == FMI_TYPE_SD_LOW)
    {
        Info = (UINT8 *)((UINT32)dummy_buffer | 0x80000000);
        outpw(REG_DMACSAR, (UINT32)Info);   // set DMA transfer starting address
        outpw(REG_SDBLEN, 7);   // 64 bit
#if 0
        if ((status = fmiSDCmdAndRsp(55, pSD->uRCA, 0)) != Successful)
            return status;
        if ((status = fmiSDCmdAndRspDataIn(51, 0x00)) != Successful)
            return status;
#endif
        if ((status = fmiSDCmdAndRsp(55, pSD->uRCA, 0)) != Successful)
            return status;
        if ((status = fmiSDCmdAndRsp(6, 0x02, 0)) != Successful)    // set bus width
            return status;

        outpw(REG_SDCR, inpw(REG_SDCR)|0x8000);
    }
    else if ((pSD->uCardType == FMI_TYPE_MMC) || (pSD->uCardType == FMI_TYPE_MMC_SECTOR_MODE))
    {
#if 0
        //--- MMC card bus width is 1 bit mode by default
        outpw(REG_SDCR, inpw(REG_SDCR) & (~0x00008000); // set SD controller to 1 bit mode
#else
        //--- sent CMD6 to MMC card to set bus width to 4 bits mode
        // set CMD6 argument Access field to 3, Index to 183, Value to 1 (4-bit mode)
        arg = (3 << 24) | (183 << 16) | (1 << 8);
        if ((status = fmiSDCmdAndRsp(6, arg, 0)) != Successful)
            return status;
        fmiCheckRB();

        outpw(REG_SDCR, inpw(REG_SDCR)|0x00008000); // set bus width to 4-bit mode for SD host controller
#endif
    }

    if ((status = fmiSDCmdAndRsp(16, 512, 0)) != Successful)    // set block length
        return status;

    fmiSDCommand(7, 0);

    // According to SD spec v2.0 chapter 4.4,
    // "After the last SD Memory Card bus transaction, the host is required,
    //  to provide 8 (eight) clock cycles for the card to complete the
    //  operation before shutting down the clock."
    outpw(REG_SDCR, inpw(REG_SDCR)|0x40);

    outpw(REG_SDISR, 0xffff);
    outpw(REG_SDIER, inpw(REG_SDIER)|0x01);

    return Successful;
}


INT fmiSD_Read_in(UINT32 uSector, UINT32 uBufcnt, UINT32 uDAddr)
{
    BOOL volatile bIsSendCmd=FALSE;
    unsigned int volatile reg;
    int volatile i, loop, status;

    if ((status = fmiSDCmdAndRsp(7, pSD->uRCA, 0)) != Successful)
        return status;
    fmiCheckRB();

    outpw(REG_SDBLEN, 0x1ff);   // 512 bytes
    if ((pSD->uCardType == FMI_TYPE_SD_HIGH) || (pSD->uCardType == FMI_TYPE_MMC_SECTOR_MODE))
        outpw(REG_SDARG, uSector);
    else
        outpw(REG_SDARG, uSector * 512);

    outpw(REG_DMACSAR, uDAddr);

    loop = uBufcnt / 255;
    for (i=0; i<loop; i++)
    {
        reg = inpw(REG_SDCR) & 0xff00c080;
        reg = reg | 0xff0000;
        if (!bIsSendCmd)
        {
            outpw(REG_SDCR, reg|(18<<8)|0x07);
            bIsSendCmd = TRUE;
        }
        else
            outpw(REG_SDCR, reg | 0x04);

        while(1)
        {
            if ((inpw(REG_SDISR) & 0x01) && (!(inpw(REG_SDCR) & 0x04)))
            {
                outpw(REG_SDISR, 0x01);
                break;
            }
            if (pSD->bIsCardInsert == FALSE)
                return -1;
        }

        if (!(inpw(REG_SDISR) & 0x04))      // check CRC7
        {
#ifdef DEBUG
            sysprintf("fmiSD_Read: response error!\n");
#endif
            return -1;
        }

        if (!(inpw(REG_SDISR) & 0x08))      // check CRC16
        {
#ifdef DEBUG
            sysprintf("fmiSD_Read_in:read data error!\n");
#endif
            return -1;
        }
    }

    loop = uBufcnt % 255;
    if (loop != 0)
    {
        reg = (inpw(REG_SDCR) & 0xff00c080) | (loop << 16);
        if (!bIsSendCmd)
        {
            outpw(REG_SDCR, reg|(18<<8)|0x07);
            bIsSendCmd = TRUE;
        }
        else
            outpw(REG_SDCR, reg | 0x04);

        while(1)
        {
            if ((inpw(REG_SDISR) & 0x01) && (!(inpw(REG_SDCR) & 0x04)))
            {
                outpw(REG_SDISR, 0x01);
                break;
            }
            if (pSD->bIsCardInsert == FALSE)
                return -1;
        }

        if (!(inpw(REG_SDISR) & 0x04))      // check CRC7
        {
#ifdef DEBUG
            sysprintf("fmiSD_Read: response error!\n");
#endif
            return -1;
        }

        if (!(inpw(REG_SDISR) & 0x08))      // check CRC16
        {
#ifdef DEBUG
            sysprintf("fmiSD_Read_in:read data error!\n");
#endif
            return -1;
        }
    }

    if (fmiSDCmdAndRsp(12, 0, 0))       // stop command
    {
#ifdef DEBUG
        sysprintf("stop command fail !!\n");
#endif
        return -1;
    }
    fmiCheckRB();

    fmiSDCommand(7, 0);

    // According to SD spec v2.0 chapter 4.4,
    // "After the last SD Memory Card bus transaction, the host is required,
    //  to provide 8 (eight) clock cycles for the card to complete the
    //  operation before shutting down the clock."
    outpw(REG_SDCR, inpw(REG_SDCR)|0x40);

    return Successful;
}


INT fmiSD_Write_in(UINT32 uSector, UINT32 uBufcnt, UINT32 uSAddr)
{
    BOOL volatile bIsSendCmd=FALSE;
    unsigned int volatile reg;
    int volatile i, loop, status;

    if ((status = fmiSDCmdAndRsp(7, pSD->uRCA, 0)) != Successful)
        return status;
    fmiCheckRB();

    outpw(REG_SDBLEN, 0x1ff);   // 512 bytes

    if (pSD->uCardType == FMI_TYPE_SD_HIGH)
        outpw(REG_SDARG, uSector);
    else
        outpw(REG_SDARG, uSector * 512);

    outpw(REG_DMACSAR, uSAddr);
    loop = uBufcnt / 256;
    for (i=0; i<loop; i++)
    {
        reg = inpw(REG_SDCR) & 0xff00c080;
        if (!bIsSendCmd)
        {
            outpw(REG_SDCR, reg|(25<<8)|0x0b);
            bIsSendCmd = TRUE;
        }
        else
            outpw(REG_SDCR, reg | 0x08);

        while(1)
        {
            if (inpw(REG_SDISR) & 0x01)
            {
                outpw(REG_SDISR, 0x01);
                break;
            }
            if (pSD->bIsCardInsert == FALSE)
                return -1;
        }

        if ((inpw(REG_SDISR) & 0x02) != 0)      // check CRC
        {
#ifdef DEBUG
            sysprintf("1. fmiSD_Write:write data error [0x%x]\n", inpw(REG_SDISR));
#endif
            outpw(REG_SDISR, 0x02);
            return -1;
        }
    }

    loop = uBufcnt % 256;
    if (loop != 0)
    {
        reg = (inpw(REG_SDCR) & 0xff00c080) | (loop << 16);
        if (!bIsSendCmd)
        {
            outpw(REG_SDCR, reg|(25<<8)|0x0b);
            bIsSendCmd = TRUE;
        }
        else
            outpw(REG_SDCR, reg | 0x08);

        while(1)
        {
            if (inpw(REG_SDISR) & 0x01)
            {
                outpw(REG_SDISR, 0x01);
                break;
            }
            if (pSD->bIsCardInsert == FALSE)
                return -1;
        }

        if ((inpw(REG_SDISR) & 0x02) != 0)      // check CRC
        {
#ifdef DEBUG
            sysprintf("2. fmiSD_Write:write data error [0x%x]!\n", inpw(REG_SDISR));
#endif
            outpw(REG_SDISR, 0x02);
            return -1;
        }
    }
    outpw(REG_SDISR, 0x02);

    if (fmiSDCmdAndRsp(12, 0, 0))       // stop command
    {
#ifdef DEBUG
        sysprintf("stop command fail !!\n");
#endif
        return -1;
    }
    fmiCheckRB();

    fmiSDCommand(7, 0);

    // According to SD spec v2.0 chapter 4.4,
    // "After the last SD Memory Card bus transaction, the host is required,
    //  to provide 8 (eight) clock cycles for the card to complete the
    //  operation before shutting down the clock."
    outpw(REG_SDCR, inpw(REG_SDCR)|0x40);

    return Successful;
}


VOID fmiGet_SD_info()
{
    unsigned int R_LEN, C_Size, MULT, size;
    unsigned int Buffer[4];

    fmiSDCmdAndRsp2(9, pSD->uRCA, Buffer);

#ifdef DEBUG
    sysprintf("max. data transfer rate [%x][%08x]\n", Buffer[0]&0xff, Buffer[0]);
#endif

    if ((Buffer[0] & 0xc0000000) && (pSD->uCardType != FMI_TYPE_MMC))
    {
        C_Size = ((Buffer[1] & 0x0000003f) << 16) | ((Buffer[2] & 0xffff0000) >> 16);
        size = (C_Size+1) * 512;    // Kbytes

        pSD->totalSectorN = size << 1;
    }
    else
    {
        R_LEN = (Buffer[1] & 0x000f0000) >> 16;
        C_Size = ((Buffer[1] & 0x000003ff) << 2) | ((Buffer[2] & 0xc0000000) >> 30);
        MULT = (Buffer[2] & 0x00038000) >> 15;
        size = (C_Size+1) * (1<<(MULT+2)) * (1<<R_LEN);

        pSD->totalSectorN = size / 512;
    }
}

int SdChipErase()
{
    int status=0;
    status = fmiSDCmdAndRsp(32, 512, 6000);
    if (status < 0)
        return status;
    status = fmiSDCmdAndRsp(33, pSD->totalSectorN*512, 6000);
    if (status < 0)
        return status;
    status = fmiSDCmdAndRsp(38, 0, 6000);
    if (status < 0)
        return status;
    fmiCheckRB();
    return 0;
}

INT  fmiInitSDDevice(int cardSel)
{
    int volatile rate, i;

    //Reset FMI
    outpw(REG_FMICR, FMI_SWRST);        // Start reset FMI controller.
    while(inpw(REG_FMICR)&FMI_SWRST);

    // enable SD
    outpw(REG_FMICR, FMI_SD_EN);
    outpw(REG_SDCR, inpw(REG_SDCR) & ~0xFF);        // disable SD clock ouput

    // reset SD engine
    outpw(REG_SDCR, inpw(REG_SDCR) | SDCR_SWRST);   // SD software reset
    while(inpw(REG_SDCR) & SDCR_SWRST);

    //Enable SD I/O pins (SD-0/1/2 port)
    if (cardSel==0)
    {
        outpw(REG_GPEFUN, inpw(REG_GPEFUN)&(~0x0000FFF0) | 0x0000aaa0); // SD0_CLK/CMD/DAT0_3 pins selected
        outpw(REG_SDCR, inpw(REG_SDCR) & (~SDCR_SDPORT));               // SD_0 port selected
    }
    else if (cardSel==1)
    {
        outpw(REG_GPBFUN, inpw(REG_GPBFUN)&(~0x00000FFF) | 0x00000AAA); // SD1_CLK_CMD_DAT0_3 pins selected
        outpw(REG_SDCR, inpw(REG_SDCR) & (~0x60000000) | 0x20000000);   // SD_1 port selected
    }
    else if (cardSel==2)
    {
        outpw(REG_GPEFUN, inpw(REG_GPEFUN)&(~0x000F0000) | 0x00050000); // SD2_DAT0_1 pins selected
        outpw(REG_GPDFUN, inpw(REG_GPDFUN)&(~0x0003FC00) | 0x00015400); // SD2_CLK/CMD/DAT2_3 pins selected
        outpw(REG_SDCR, inpw(REG_SDCR) & (~0x60000000) | 0x40000000);   // SD_2 port selected
    }

    // Disable FMI/SD host interrupt
    outpw(REG_FMIIER, 0);

    // restore default register value
    outpw(REG_SDIER, 0x00000A00);
    outpw(REG_SDISR, 0x0000FFFF);
    outpw(REG_SDTMOUT, 0x00FFFFFF);

    pSD = &SDInfo;
    memset((char *)pSD, 0, sizeof(FMI_SD_INFO_T));

    pSD->bIsCardInsert = TRUE;

    if (fmiSD_Init() < 0)
        return -1;

    /* divider */
    if (pSD->uCardType == 3)
        rate = _fmi_uFMIReferenceClock / MMC_FREQ;
    else
        rate = _fmi_uFMIReferenceClock / SD_FREQ;

    if (pSD->uCardType == 3)
    {
        if ((_fmi_uFMIReferenceClock % MMC_FREQ) > 0)
            rate ++;
    }
    else
    {
        if ((_fmi_uFMIReferenceClock % SD_FREQ) > 0)
            rate ++;
    }

    for(i=0; i<100; i++);
    outpw(REG_CLKDIV2, (inpw(REG_CLKDIV2) & ~SD_S) | (0x03 << 19));     // SD clock from UPLL
    outpw(REG_CLKDIV2, (inpw(REG_CLKDIV2) & ~SD_N0) | (0x01 << 16));    // SD clock divided by 2
    if (rate % 2)
    {
        rate /= 2;
        rate &= 0xFF;
    }
    else
    {
        rate /= 2;
        rate &= 0xFF;
        rate--;
    }
    outpw(REG_CLKDIV2, (inpw(REG_CLKDIV2) & ~SD_N1) | (rate << 24));    // SD clock divider
    for(i=0; i<1000; i++);

    /* init SD interface */
    fmiGet_SD_info();

    if (fmiSelectCard())
        return -1;

    return 0;
}   /* end InitIDEDevice */

INT  fmiSD_Read(UINT32 uSector, UINT32 uBufcnt, UINT32 uDAddr)
{
    int status=0;

    // enable SD
    outpw(REG_FMICR, 0x02);
    status = fmiSD_Read_in(uSector, uBufcnt, uDAddr);

    return status;
}

INT  fmiSD_Write(UINT32 uSector, UINT32 uBufcnt, UINT32 uSAddr)
{
    int status=0;

    // enable SD
    outpw(REG_FMICR, 0x02);
    status = fmiSD_Write_in(uSector, uBufcnt, uSAddr);

    return status;
}

VOID fmiInitDevice()
{
    // Enable SD Card Host Controller operation and driving clock.
    outpw(REG_AHBCLK, inpw(REG_AHBCLK) | SIC_CKE);
    outpw(REG_AHBCLK, inpw(REG_AHBCLK) | SD_CKE);


    // DMAC Initial
    outpw(REG_DMACCSR, DMAC_EN | DMAC_SWRST);
    outpw(REG_DMACCSR, DMAC_EN);
    outpw(REG_DMACISR, TABORT_IF);
    outpw(REG_DMACIER, 0x00000000);

    outpw(REG_FMICR, FMI_SWRST);        // reset FMI engine

    // assign UPLL to reference clock of SD engine.
    _fmi_uFMIReferenceClock = sysGetPLLOutputKhz(eSYS_UPLL, sysGetExternalClock());
}
