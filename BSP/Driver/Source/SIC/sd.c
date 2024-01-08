/**************************************************************************//**
 * @file     sd.c
 * @version  V3.00
 * @brief    N9H20 series SIC driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#ifdef ECOS
    #include "drv_api.h"
    #include "diag.h"
    #include "wbtypes.h"
    #include "wbio.h"
#else
    #include "wblib.h"
#endif

#include "N9H20_SIC.h"
#include "fmi.h"
#include "N9H20_NVTFAT.h"

// define DATE CODE and show it when running to make maintaining easy.
#define SD_DATE_CODE    FMI_DATE_CODE

#define SD_BLOCK_SIZE   512

#define FMI_SD_INITCOUNT    2000
#define FMI_TICKCOUNT       1000

#define FMI_TYPE_UNKNOWN    0
#define FMI_TYPE_SD_HIGH    1
#define FMI_TYPE_SD_LOW     2
#define FMI_TYPE_MMC        3

// global variables
// For response R3 (such as ACMD41, CRC-7 is invalid; but SD controller will still
//      calculate CRC-7 and get an error result, software should ignore this error and clear SDISR [CRC_IF] flag
//      _fmi_uR3_CMD is the flag for it. 1 means software should ignore CRC-7 error
UINT32 _fmi_uR3_CMD=0;
UINT32 _fmi_uR7_CMD=0;

#if defined (__GNUC__)
    UCHAR _fmi_ucSDHCBuffer[512] __attribute__((aligned (4096)));
#else
    __align(4096) UCHAR _fmi_ucSDHCBuffer[512];
#endif

UINT8 *_fmi_pSDHCBuffer;

//--- 2014/3/27, check the sector number is valid or not for current SD card.
unsigned int g_max_valid_sector;    // The max valid sector number for current SD card.
INT fmiSDCheckSector(UINT32 uSector, UINT32 uBufcnt)
{
    if ((uSector + uBufcnt - 1) > g_max_valid_sector)
    {
        sysprintf("ERROR: Fail to access invalid sector number %d from SD card !!\n", uSector+uBufcnt-1);
        sysprintf("       The max valid sector number for current SD card is %d.\n", g_max_valid_sector);
        return FMI_SD_SELECT_ERROR; // invalid sector
    }
    return 0;   // valid sector
}


void fmiCheckRB()
{
    while(1)
    {
        outpw(REG_SDCR, inpw(REG_SDCR)|SDCR_8CLK_OE);
        while(inpw(REG_SDCR) & SDCR_8CLK_OE);
        if (inpw(REG_SDISR) & SDISR_SD_DATA0)
            break;
    }
}

INT fmiSDCommand(FMI_SD_INFO_T *pSD, UINT8 ucCmd, UINT32 uArg)
{
    outpw(REG_SDARG, uArg);
    outpw(REG_SDCR, (inpw(REG_SDCR)&(~SDCR_CMD_CODE))|(ucCmd << 8)|(SDCR_CO_EN));

    while(inpw(REG_SDCR) & SDCR_CO_EN)
    {
        if (pSD == pSD0)
            fmiSD_CardStatus();
        if (pSD->bIsCardInsert == FALSE)
            return FMI_NO_SD_CARD;
    }

    return Successful;
}


INT fmiSDCmdAndRsp(FMI_SD_INFO_T *pSD, UINT8 ucCmd, UINT32 uArg, INT ntickCount)
{
    outpw(REG_SDARG, uArg);
    outpw(REG_SDCR, (inpw(REG_SDCR)&(~SDCR_CMD_CODE))|(ucCmd << 8)|(SDCR_CO_EN | SDCR_RI_EN));

    if (ntickCount > 0)
    {
        while(inpw(REG_SDCR) & SDCR_RI_EN)
        {
            if(ntickCount-- == 0) {
                //--- 2023/10/6, Reset SD controller and DMAC to keep clean status for next access.
                // Reset DMAC engine and interrupt satus
                outpw(REG_DMACCSR, inpw(REG_DMACCSR) | DMAC_SWRST | DMAC_EN);
                while(inpw(REG_DMACCSR) & DMAC_SWRST);
                outpw(REG_DMACCSR, inpw(REG_DMACCSR) | DMAC_EN);
                outpw(REG_DMACISR, WEOT_IF | TABORT_IF);    // clear all interrupt flag
            
                // Reset FMI engine and interrupt status
                outpw(REG_FMICR, FMI_SWRST);
                while(inpw(REG_FMICR) & FMI_SWRST);
                outpw(REG_FMIISR, FMI_DAT_IF);              // clear all interrupt flag
            
                // Reset SD engine and interrupt status
                outpw(REG_FMICR, FMI_SD_EN);
                outpw(REG_SDCR, inpw(REG_SDCR) | SDCR_SWRST);
                while(inpw(REG_SDCR) & SDCR_SWRST);
                outpw(REG_SDISR, 0xFFFFFFFF);               // clear all interrupt flag

                return FMI_SD_INIT_TIMEOUT;
            }
            if (pSD == pSD0)
                fmiSD_CardStatus();
            if (pSD->bIsCardInsert == FALSE)
                return FMI_NO_SD_CARD;

            if (inpw(REG_SDISR) & SDISR_RITO_IF)
            {
                outpw(REG_SDISR, SDISR_RITO_IF);
                return FMI_SD_RITO_ERROR;
            }
        }
    }
    else
    {
        while(inpw(REG_SDCR) & SDCR_RI_EN)
        {
            if (pSD == pSD0)
                fmiSD_CardStatus();
            if (pSD->bIsCardInsert == FALSE)
                return FMI_NO_SD_CARD;

            if (inpw(REG_SDISR) & SDISR_RITO_IF)
            {
                outpw(REG_SDISR, SDISR_RITO_IF);
                return FMI_SD_RITO_ERROR;
            }
        }
    }

    if (_fmi_uR7_CMD)
    {
        if (((inpw(REG_SDRSP1) & 0xff) != 0x55) && ((inpw(REG_SDRSP0) & 0xf) != 0x01))
        {
            _fmi_uR7_CMD = 0;
            return FMI_SD_CMD8_ERROR;
        }
    }

    if (!_fmi_uR3_CMD)
    {
        if (inpw(REG_SDISR) & SDISR_CRC_7)      // check CRC7
            return Successful;
        else
        {
#ifdef DEBUG
            sysprintf("response error [%d]!\n", ucCmd);
#endif
            return FMI_SD_CRC7_ERROR;
        }
    }
    else    // ignore CRC error for R3 case
    {
        _fmi_uR3_CMD = 0;
        outpw(REG_SDISR, SDISR_CRC_IF);
        return Successful;
    }
}


// Get 16 bytes CID or CSD
INT fmiSDCmdAndRsp2(FMI_SD_INFO_T *pSD, UINT8 ucCmd, UINT32 uArg, UINT *puR2ptr)
{
    unsigned int i;
    unsigned int tmpBuf[5];

    outpw(REG_SDARG, uArg);
    outpw(REG_SDCR, (inpw(REG_SDCR)&(~SDCR_CMD_CODE))|(ucCmd << 8)|(SDCR_CO_EN | SDCR_R2_EN));

    while(inpw(REG_SDCR) & SDCR_R2_EN)
    {
        if (pSD == pSD0)
            fmiSD_CardStatus();
        if (pSD->bIsCardInsert == FALSE)
            return FMI_NO_SD_CARD;

        if (inpw(REG_SDISR) & SDISR_RITO_IF)
        {
            outpw(REG_SDISR, SDISR_RITO_IF);
            return FMI_SD_RITO_ERROR;
        }
    }

    if (inpw(REG_SDISR) & SDISR_CRC_7)
    {
        for (i=0; i<5; i++)
            tmpBuf[i] = Swap32(inpw(REG_FB_0+i*4));

        for (i=0; i<4; i++)
            *puR2ptr++ = ((tmpBuf[i] & 0x00ffffff)<<8) | ((tmpBuf[i+1] & 0xff000000)>>24);
        return Successful;
    }
    else
        return FMI_SD_CRC7_ERROR;
}


INT fmiSDCmdAndRspDataIn(FMI_SD_INFO_T *pSD, UINT8 ucCmd, UINT32 uArg)
{
    outpw(REG_SDARG, uArg);
    outpw(REG_SDCR, (inpw(REG_SDCR)&(~SDCR_CMD_CODE))|(ucCmd << 8)|(SDCR_CO_EN | SDCR_RI_EN | SDCR_DI_EN));

    while (inpw(REG_SDCR) & SDCR_RI_EN)
    {
        if (pSD == pSD0)
            fmiSD_CardStatus();
        if (pSD->bIsCardInsert == FALSE)
            return FMI_NO_SD_CARD;

        if (inpw(REG_SDISR) & SDISR_RITO_IF)
        {
            outpw(REG_SDISR, SDISR_RITO_IF);
            return FMI_SD_RITO_ERROR;
        }
    }

    while (inpw(REG_SDCR) & SDCR_DI_EN)
    {
        if (pSD == pSD0)
            fmiSD_CardStatus();
        if (pSD->bIsCardInsert == FALSE)
            return FMI_NO_SD_CARD;

        if (inpw(REG_SDISR) & SDISR_DITO_IF)
        {
            outpw(REG_SDISR, SDISR_DITO_IF);
            return FMI_SD_DITO_ERROR;
        }
    }

    if (!(inpw(REG_SDISR) & SDISR_CRC_7))       // check CRC7
    {
#ifdef DEBUG
        sysprintf("fmiSDCmdAndRspDataIn: response error [%d]!\n", ucCmd);
#endif
        return FMI_SD_CRC7_ERROR;
    }

    if (!(inpw(REG_SDISR) & SDISR_CRC_16))      // check CRC16
    {
#ifdef DEBUG
        sysprintf("fmiSDCmdAndRspDataIn: read data CRC16 error!\n");
#endif
        return FMI_SD_CRC16_ERROR;
    }
    return Successful;
}

// Initial
INT fmiSD_Init(FMI_SD_INFO_T *pSD)
{
    int volatile i, status, rate;
    unsigned int resp;
    unsigned int CIDBuffer[4];
    unsigned int volatile u32CmdTimeOut;
    int sdport;

    if (pSD == pSD0)
        sdport = 0;
    else if (pSD == pSD1)
        sdport = 1;
    else if (pSD == pSD2)
        sdport = 2;
    else
        return FMI_SD_INIT_ERROR;
    sysprintf("Initial SD NonOS Driver (%s) for SD port %d\n", SD_DATE_CODE, sdport);

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
        if (pSD == pSD0)
            fmiSD_CardStatus();
        if (pSD->bIsCardInsert == FALSE)
            return FMI_NO_SD_CARD;
    }

    fmiSDCommand(pSD, 0, 0);        // reset all cards
    for (i=0x100; i>0; i--);

    // initial SDHC
    _fmi_uR7_CMD = 1;
    u32CmdTimeOut = 5000;

    i = fmiSDCmdAndRsp(pSD, 8, 0x00000155, u32CmdTimeOut);
    _fmi_uR7_CMD = 0;   // Disable R7 checking for commands that are not CMD8
    if (i == Successful)
    {
        // SD 2.0
        fmiSDCmdAndRsp(pSD, 55, 0x00, u32CmdTimeOut);
        _fmi_uR3_CMD = 1;
        fmiSDCmdAndRsp(pSD, 41, 0x40ff8000, u32CmdTimeOut); // 2.7v-3.6v
        resp = inpw(REG_SDRSP0);

        while (!(resp & 0x00800000))        // check if card is ready
        {
            fmiSDCmdAndRsp(pSD, 55, 0x00, u32CmdTimeOut);
            _fmi_uR3_CMD = 1;
            fmiSDCmdAndRsp(pSD, 41, 0x40ff8000, u32CmdTimeOut); // 3.0v-3.4v
            resp = inpw(REG_SDRSP0);
        }
        if (resp & 0x00400000)
            pSD->uCardType = FMI_TYPE_SD_HIGH;
        else
            pSD->uCardType = FMI_TYPE_SD_LOW;
    }
    else
    {
        // SD 1.1 or MMC
        fmiSDCommand(pSD, 0, 0);        // reset all cards
        for (i=0x100; i>0; i--);

        i = fmiSDCmdAndRsp(pSD, 55, 0x00, u32CmdTimeOut);
        if (i == FMI_SD_INIT_TIMEOUT)     // MMC memory
        {
            fmiSDCommand(pSD, 0, 0);        // reset
            for (i=0x100; i>0; i--);

            _fmi_uR3_CMD = 1;
            // 2014/8/6, to support eMMC v4.4, the argument of CMD1 should be 0x40ff8000 to support both MMC plus and eMMC cards.
            if (fmiSDCmdAndRsp(pSD, 1, 0x40ff8000, u32CmdTimeOut) != FMI_SD_INIT_TIMEOUT)   // MMC memory
            {
                resp = inpw(REG_SDRSP0);
                while (!(resp & 0x00800000))        // check if card is ready
                {
                    _fmi_uR3_CMD = 1;
                    fmiSDCmdAndRsp(pSD, 1, 0x40ff8000, u32CmdTimeOut);  // high voltage
                    resp = inpw(REG_SDRSP0);
                }
                // MMC card is ready. Check the access mode of MMC card.
                if (resp & 0x00400000)
                    pSD->uCardType = FMI_TYPE_MMC_SECTOR_MODE;
                else
                    pSD->uCardType = FMI_TYPE_MMC;
            }
            else
            {
                pSD->uCardType = FMI_TYPE_UNKNOWN;
                return FMI_ERR_DEVICE;
            }
        }
        else if (i == Successful)    // SD Memory
        {
            _fmi_uR3_CMD = 1;
            fmiSDCmdAndRsp(pSD, 41, 0x00ff8000, u32CmdTimeOut); // 3.0v-3.4v
            resp = inpw(REG_SDRSP0);
            while (!(resp & 0x00800000))        // check if card is ready
            {
                fmiSDCmdAndRsp(pSD, 55, 0x00,u32CmdTimeOut);
                _fmi_uR3_CMD = 1;
                fmiSDCmdAndRsp(pSD, 41, 0x00ff8000, u32CmdTimeOut); // 3.0v-3.4v
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
            return FMI_SD_INIT_ERROR;
        }
    }

    // CMD2, CMD3
    if (pSD->uCardType != FMI_TYPE_UNKNOWN)
    {
        fmiSDCmdAndRsp2(pSD, 2, 0x00, CIDBuffer);
        if ((pSD->uCardType == FMI_TYPE_MMC) || (pSD->uCardType == FMI_TYPE_MMC_SECTOR_MODE))
        {
            // Increase RCA for next MMC card.
            // The RCA value 0 is reserved to set all cards with CMD7.
            // The default value is 1.
            pSD->uRCA = (pSD->uRCA + 0x10000) & 0xFFFF0000;   // RCA is 16-bit value at MSB
            if (pSD->uRCA == 0)
                pSD->uRCA = 0x10000;

            if ((status = fmiSDCmdAndRsp(pSD, 3, pSD->uRCA, 0)) != Successful)  // set RCA for MMC
                return status;
        }
        else
        {
            if ((status = fmiSDCmdAndRsp(pSD, 3, 0x00, 0)) != Successful)       // get RCA for SD
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
    }
#endif

    // set data transfer clock
    return Successful;
}


INT fmiSwitchToHighSpeed(FMI_SD_INFO_T *pSD)
{
    int volatile status=0;
    UINT16 current_comsumption, busy_status0, busy_status1;
    // UINT16 fun1_info, switch_status;

    outpw(REG_DMACSAR, (UINT32)_fmi_pSDHCBuffer);   // set DMA transfer starting address
    outpw(REG_SDBLEN, 63);  // 512 bit

    if ((status = fmiSDCmdAndRspDataIn(pSD, 6, 0x00ffff01)) != Successful)
        return Fail;

    current_comsumption = _fmi_pSDHCBuffer[0]<<8 | _fmi_pSDHCBuffer[1];
    if (!current_comsumption)
        return Fail;

    // fun1_info =  _fmi_pSDHCBuffer[12]<<8 | _fmi_pSDHCBuffer[13];
    // switch_status =  _fmi_pSDHCBuffer[16] & 0xf;
    busy_status0 = _fmi_pSDHCBuffer[28]<<8 | _fmi_pSDHCBuffer[29];

    if (!busy_status0)  // function ready
    {
        outpw(REG_DMACSAR, (UINT32)_fmi_pSDHCBuffer);   // set DMA transfer starting address
        outpw(REG_SDBLEN, 63);  // 512 bit

        if ((status = fmiSDCmdAndRspDataIn(pSD, 6, 0x80ffff01)) != Successful)
            return Fail;

        // function change timing: 8 clocks
        outpw(REG_SDCR, inpw(REG_SDCR)|SDCR_8CLK_OE);
        while(inpw(REG_SDCR) & SDCR_8CLK_OE);

        current_comsumption = _fmi_pSDHCBuffer[0]<<8 | _fmi_pSDHCBuffer[1];
        if (!current_comsumption)
            return Fail;

        busy_status1 = _fmi_pSDHCBuffer[28]<<8 | _fmi_pSDHCBuffer[29];
        if (!busy_status1)
            sysprintf("switch into high speed mode !!!\n");

        return Successful;
    }
    else
        return Fail;
}


INT fmiSelectCard(FMI_SD_INFO_T *pSD)
{
    int volatile status=0, i;
    int rate;
    UINT32 arg;

    if ((status = fmiSDCmdAndRsp(pSD, 7, pSD->uRCA, 0)) != Successful)
        return status;

    fmiCheckRB();

    // if SD card set 4bit
    if (pSD->uCardType == FMI_TYPE_SD_HIGH)
    {
        _fmi_pSDHCBuffer = (UINT8 *)((UINT32)_fmi_ucSDHCBuffer | 0x80000000);
        outpw(REG_DMACSAR, (UINT32)_fmi_pSDHCBuffer);   // set DMA transfer starting address
        outpw(REG_SDBLEN, 7);   // 64 bit

        if ((status = fmiSDCmdAndRsp(pSD, 55, pSD->uRCA, 0)) != Successful)
            return status;
        if ((status = fmiSDCmdAndRspDataIn(pSD, 51, 0x00)) != Successful)
            return status;

        if ((_fmi_ucSDHCBuffer[0] & 0xf) == 0x2)
        {
            // support SD spec v2.0
            status = fmiSwitchToHighSpeed(pSD);
            if (status == Successful)
            {
                /* divider */
                rate = _fmi_uFMIReferenceClock / SDHC_FREQ;
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

        if ((status = fmiSDCmdAndRsp(pSD, 55, pSD->uRCA, 0)) != Successful)
            return status;
        if ((status = fmiSDCmdAndRsp(pSD, 6, 0x02, 0)) != Successful)   // set bus width to 4-bit mode for SD card
            return status;

        outpw(REG_SDCR, inpw(REG_SDCR)|SDCR_DBW);   // set bus width to 4-bit mode for SD host controller
    }
    else if (pSD->uCardType == FMI_TYPE_SD_LOW)
    {
#if 0
        _fmi_pSDHCBuffer = (UINT8 *)((UINT32)_fmi_ucSDHCBuffer | 0x80000000);
        outpw(REG_DMACSAR, (UINT32)_fmi_pSDHCBuffer);   // set DMA transfer starting address
        outpw(REG_SDBLEN, 7);   // 64 bit

        if ((status = fmiSDCmdAndRsp(pSD, 55, pSD->uRCA, 0)) != Successful)
            return status;
        if ((status = fmiSDCmdAndRspDataIn(pSD, 51, 0x00)) != Successful)
            return status;
#endif
        if ((status = fmiSDCmdAndRsp(pSD, 55, pSD->uRCA, 0)) != Successful)
            return status;
        if ((status = fmiSDCmdAndRsp(pSD, 6, 0x02, 0)) != Successful)   // set bus width to 4-bit mode for SD card
            return status;

        outpw(REG_SDCR, inpw(REG_SDCR)|SDCR_DBW);   // set bus width to 4-bit mode for SD host controller
    }
    else if ((pSD->uCardType == FMI_TYPE_MMC) || (pSD->uCardType == FMI_TYPE_MMC_SECTOR_MODE))
    {
        //--- sent CMD6 to MMC card to set bus width to 4 bits mode
        // set CMD6 argument Access field to 3, Index to 183, Value to 1 (4-bit mode)
        arg = (3 << 24) | (183 << 16) | (1 << 8);
        if ((status = fmiSDCmdAndRsp(pSD, 6, arg, 0)) != Successful)
            return status;
        fmiCheckRB();

        outpw(REG_SDCR, inpw(REG_SDCR)|SDCR_DBW);   // set bus width to 4-bit mode for SD host controller
    }

    if ((status = fmiSDCmdAndRsp(pSD, 16, SD_BLOCK_SIZE, 0)) != Successful) // set block length
        return status;

    fmiSDCommand(pSD, 7, 0);

    // According to SD spec v2.0 chapter 4.4,
    // "After the last SD Memory Card bus transaction, the host is required,
    //  to provide 8 (eight) clock cycles for the card to complete the
    //  operation before shutting down the clock."
    outpw(REG_SDCR, inpw(REG_SDCR)|SDCR_8CLK_OE);

#ifdef _SIC_USE_INT_
    outpw(REG_SDIER, inpw(REG_SDIER)|SDIER_BLKD_IEN);
#endif  //_SIC_USE_INT_

    return Successful;
}

/*-----------------------------------------------------------------------------
 * fmiSD_Read_in(), To read data with default black size SD_BLOCK_SIZE
 *---------------------------------------------------------------------------*/
INT fmiSD_Read_in(FMI_SD_INFO_T *pSD, UINT32 uSector, UINT32 uBufcnt, UINT32 uDAddr)
{
    BOOL volatile bIsSendCmd=FALSE;
    unsigned int volatile reg;
    int volatile i, loop, status;

    //--- check input parameters
    status = fmiSDCheckSector(uSector, uBufcnt);
    if (status < 0)
        return status;  // invalid sector

    if (uBufcnt == 0)
    {
        sysprintf("ERROR: fmiSD_Read_in(): uBufcnt cannot be 0!!\n");
        return FMI_SD_SELECT_ERROR;
    }

    if ((status = fmiSDCmdAndRsp(pSD, 7, pSD->uRCA, 0)) != Successful)
        return status;
    fmiCheckRB();

    outpw(REG_SDBLEN, SD_BLOCK_SIZE - 1);

    if ((pSD->uCardType == FMI_TYPE_SD_HIGH) || (pSD->uCardType == FMI_TYPE_MMC_SECTOR_MODE))
        outpw(REG_SDARG, uSector);
    else
        outpw(REG_SDARG, uSector * SD_BLOCK_SIZE);

    outpw(REG_DMACSAR, uDAddr);

    loop = uBufcnt / 255;
    for (i=0; i<loop; i++)
    {
#ifdef _SIC_USE_INT_
        _fmi_bIsSDDataReady = FALSE;
#endif  //_SIC_USE_INT_

        reg = inpw(REG_SDCR) & ~SDCR_CMD_CODE;
        reg = reg | 0xff0000;   // set BLK_CNT to 255
        if (bIsSendCmd == FALSE)
        {
            outpw(REG_SDCR, reg|(18<<8)|(SDCR_CO_EN | SDCR_RI_EN | SDCR_DI_EN));
            bIsSendCmd = TRUE;
        }
        else
            outpw(REG_SDCR, reg | SDCR_DI_EN);

#ifdef _SIC_USE_INT_
        while(!_fmi_bIsSDDataReady)
#else
        while(1)
#endif  //_SIC_USE_INT_
        {
#ifndef _SIC_USE_INT_
            if ((inpw(REG_SDISR) & SDISR_BLKD_IF) && (!(inpw(REG_SDCR) & SDCR_DI_EN)))
            {
                outpw(REG_SDISR, SDISR_BLKD_IF);
                break;
            }
#endif
            if (pSD == pSD0)
                fmiSD_CardStatus();
            if (pSD->bIsCardInsert == FALSE)
                return FMI_NO_SD_CARD;

            if (inpw(REG_SDISR) & SDISR_DITO_IF)
            {
                outpw(REG_SDISR, SDISR_DITO_IF);
                return FMI_SD_DITO_ERROR;
            }
            if (inpw(REG_SDISR) & SDISR_RITO_IF)
            {
                outpw(REG_SDISR, SDISR_RITO_IF);
                return FMI_SD_RITO_ERROR;
            }

            /* Call schedule() to release CPU power to other tasks during waiting SIC/DMA completed. */
            schedule();
        }

        if (!(inpw(REG_SDISR) & SDISR_CRC_7))       // check CRC7
        {
#ifdef DEBUG
            sysprintf("fmiSD_Read_in(): response error!\n");
#endif
            return FMI_SD_CRC7_ERROR;
        }

        if (!(inpw(REG_SDISR) & SDISR_CRC_16))      // check CRC16
        {
#ifdef DEBUG
            sysprintf("fmiSD_Read_in() :read data CRC16 error!\n");
#endif
            return FMI_SD_CRC16_ERROR;
        }
    }

    loop = uBufcnt % 255;
    if (loop != 0)
    {
#ifdef _SIC_USE_INT_
        _fmi_bIsSDDataReady = FALSE;
#endif  //_SIC_USE_INT_

        reg = inpw(REG_SDCR) & (~SDCR_CMD_CODE);
        reg = reg & (~SDCR_BLKCNT);
        reg |= (loop << 16);    // setup SDCR_BLKCNT

        if (bIsSendCmd == FALSE)
        {
            outpw(REG_SDCR, reg|(18<<8)|(SDCR_CO_EN | SDCR_RI_EN | SDCR_DI_EN));
            bIsSendCmd = TRUE;
        }
        else
            outpw(REG_SDCR, reg | SDCR_DI_EN);

#ifdef _SIC_USE_INT_
        while(!_fmi_bIsSDDataReady)
#else
        while(1)
#endif  //_SIC_USE_INT_
        {
#ifndef _SIC_USE_INT_
            if ((inpw(REG_SDISR) & SDISR_BLKD_IF) && (!(inpw(REG_SDCR) & SDCR_DI_EN)))
            {
                outpw(REG_SDISR, SDISR_BLKD_IF);
                break;
            }
#endif
            if (pSD == pSD0)
                fmiSD_CardStatus();
            if (pSD->bIsCardInsert == FALSE)
                return FMI_NO_SD_CARD;

            if (inpw(REG_SDISR) & SDISR_DITO_IF)
            {
                outpw(REG_SDISR, SDISR_DITO_IF);
                return FMI_SD_DITO_ERROR;
            }
            if (inpw(REG_SDISR) & SDISR_RITO_IF)
            {
                outpw(REG_SDISR, SDISR_RITO_IF);
                return FMI_SD_RITO_ERROR;
            }

            /* Call schedule() to release CPU power to other tasks during waiting SIC/DMA completed. */
            schedule();
        }

        if (!(inpw(REG_SDISR) & SDISR_CRC_7))       // check CRC7
        {
#ifdef DEBUG
            sysprintf("fmiSD_Read_in(): response error!\n");
#endif
            return FMI_SD_CRC7_ERROR;
        }

        if (!(inpw(REG_SDISR) & SDISR_CRC_16))      // check CRC16
        {
#ifdef DEBUG
            sysprintf("fmiSD_Read_in(): read data CRC16 error!\n");
#endif
            return FMI_SD_CRC16_ERROR;
        }
    }

    if (fmiSDCmdAndRsp(pSD, 12, 0, 0))      // stop command
    {
#ifdef DEBUG
        sysprintf("stop command fail !!\n");
#endif
        return FMI_SD_CRC7_ERROR;
    }
    fmiCheckRB();

    fmiSDCommand(pSD, 7, 0);

    // According to SD spec v2.0 chapter 4.4,
    // "After the last SD Memory Card bus transaction, the host is required,
    //  to provide 8 (eight) clock cycles for the card to complete the
    //  operation before shutting down the clock."
    outpw(REG_SDCR, inpw(REG_SDCR)|SDCR_8CLK_OE);

    return Successful;
}

/*-----------------------------------------------------------------------------
 * fmiSD_Write_in(), To write data with static black size SD_BLOCK_SIZE
 *---------------------------------------------------------------------------*/
INT fmiSD_Write_in(FMI_SD_INFO_T *pSD, UINT32 uSector, UINT32 uBufcnt, UINT32 uSAddr)
{
    BOOL volatile bIsSendCmd=FALSE;
    unsigned int volatile reg;
    int volatile i, loop, status;

    //--- check input parameters
    status = fmiSDCheckSector(uSector, uBufcnt);
    if (status < 0)
        return status;  // invalid sector

    if (uBufcnt == 0)
    {
        sysprintf("ERROR: fmiSD_Write_in(): uBufcnt cannot be 0!!\n");
        return FMI_SD_SELECT_ERROR;
    }

    if ((status = fmiSDCmdAndRsp(pSD, 7, pSD->uRCA, 0)) != Successful)
        return status;
    fmiCheckRB();

    // According to SD Spec v2.0, the write CMD block size MUST be 512, and the start address MUST be 512*n.
    outpw(REG_SDBLEN, SD_BLOCK_SIZE - 1);           // set the block size

    if ((pSD->uCardType == FMI_TYPE_SD_HIGH) || (pSD->uCardType == FMI_TYPE_MMC_SECTOR_MODE))
        outpw(REG_SDARG, uSector);
    else
        outpw(REG_SDARG, uSector * SD_BLOCK_SIZE);  // set start address for SD CMD

    outpw(REG_DMACSAR, uSAddr);
    loop = uBufcnt / 255;   // the maximum block count is 0xFF=255 for register SDCR[BLK_CNT]
    for (i=0; i<loop; i++)
    {
#ifdef _SIC_USE_INT_
        _fmi_bIsSDDataReady = FALSE;
#endif  //_SIC_USE_INT_

        reg = inpw(REG_SDCR) & 0xff00c080;
        reg = reg | 0xff0000;   // set BLK_CNT to 0xFF=255
        if (!bIsSendCmd)
        {
            outpw(REG_SDCR, reg|(25<<8)|(SDCR_CO_EN | SDCR_RI_EN | SDCR_DO_EN));
            bIsSendCmd = TRUE;
        }
        else
            outpw(REG_SDCR, reg | SDCR_DO_EN);

#ifdef _SIC_USE_INT_
        while(!_fmi_bIsSDDataReady)
#else
        while(1)
#endif  //_SIC_USE_INT_
        {
#ifndef _SIC_USE_INT_
            if ((inpw(REG_SDISR) & SDISR_BLKD_IF) && (!(inpw(REG_SDCR) & SDCR_DO_EN)))
            {
                outpw(REG_SDISR, SDISR_BLKD_IF);
                break;
            }
#endif
            if (pSD == pSD0)
                fmiSD_CardStatus();
            if (pSD->bIsCardInsert == FALSE)
                return FMI_NO_SD_CARD;

            if (inpw(REG_SDISR) & SDISR_RITO_IF)
            {
                outpw(REG_SDISR, SDISR_RITO_IF);
                return FMI_SD_RITO_ERROR;
            }

            /* Call schedule() to release CPU power to other tasks during waiting SIC/DMA completed. */
            schedule();
        }

        if ((inpw(REG_SDISR) & SDISR_CRC_IF) != 0)      // check CRC
        {
#ifdef DEBUG
            sysprintf("1. fmiSD_Write:write data error [SDISR = 0x%08X]\n", inpw(REG_SDISR));
#endif
            outpw(REG_SDISR, SDISR_CRC_IF);
            return FMI_SD_CRC_ERROR;
        }
    }

    loop = uBufcnt % 255;
    if (loop != 0)
    {
#ifdef _SIC_USE_INT_
        _fmi_bIsSDDataReady = FALSE;
#endif  //_SIC_USE_INT_

        reg = (inpw(REG_SDCR) & 0xff00c080) | (loop << 16);
        if (!bIsSendCmd)
        {
            outpw(REG_SDCR, reg|(25<<8)|(SDCR_CO_EN | SDCR_RI_EN | SDCR_DO_EN));
            bIsSendCmd = TRUE;
        }
        else
            outpw(REG_SDCR, reg | SDCR_DO_EN);

#ifdef _SIC_USE_INT_
        while(!_fmi_bIsSDDataReady)
#else
        while(1)
#endif  //_SIC_USE_INT_
        {
#ifndef _SIC_USE_INT_
            if ((inpw(REG_SDISR) & SDISR_BLKD_IF) && (!(inpw(REG_SDCR) & SDCR_DO_EN)))
            {
                outpw(REG_SDISR, SDISR_BLKD_IF);
                break;
            }
#endif
            if (pSD == pSD0)
                fmiSD_CardStatus();
            if (pSD->bIsCardInsert == FALSE)
                return FMI_NO_SD_CARD;

            if (inpw(REG_SDISR) & SDISR_RITO_IF)
            {
                outpw(REG_SDISR, SDISR_RITO_IF);
                return FMI_SD_RITO_ERROR;
            }

            /* Call schedule() to release CPU power to other tasks during waiting SIC/DMA completed. */
            schedule();
        }

        if ((inpw(REG_SDISR) & SDISR_CRC_IF) != 0)      // check CRC
        {
#ifdef DEBUG
            sysprintf("2. fmiSD_Write:write data error [SDISR = 0x%08X]\n", inpw(REG_SDISR));
#endif
            outpw(REG_SDISR, SDISR_CRC_IF);
            return FMI_SD_CRC_ERROR;
        }
    }
    outpw(REG_SDISR, SDISR_CRC_IF);

    if (fmiSDCmdAndRsp(pSD, 12, 0, 0))      // stop command
    {
#ifdef DEBUG
        sysprintf("stop command fail !!\n");
#endif
        return FMI_SD_CRC7_ERROR;
    }
    fmiCheckRB();

    fmiSDCommand(pSD, 7, 0);

    // According to SD spec v2.0 chapter 4.4,
    // "After the last SD Memory Card bus transaction, the host is required,
    //  to provide 8 (eight) clock cycles for the card to complete the
    //  operation before shutting down the clock."
    outpw(REG_SDCR, inpw(REG_SDCR)|SDCR_8CLK_OE);

    return Successful;
}


VOID fmiGet_SD_info(FMI_SD_INFO_T *pSD, DISK_DATA_T *_info)
{
    unsigned int i;
    unsigned int R_LEN, C_Size, MULT, size;
    unsigned int Buffer[4];
    unsigned char *ptr;
    int volatile status;

    fmiSDCmdAndRsp2(pSD, 9, pSD->uRCA, Buffer);

#ifdef DEBUG
    sysprintf("max. data transfer rate [%x][%08x]\n", Buffer[0]&0xff, Buffer[0]);
    sysprintf("CSD = 0x%08X 0x%08X 0x%08X 0x%08X\n", Buffer[0], Buffer[1], Buffer[2], Buffer[3]);
#endif

    if ((pSD->uCardType == FMI_TYPE_MMC) || (pSD->uCardType == FMI_TYPE_MMC_SECTOR_MODE))
    {
        // for MMC/eMMC card
        if ((Buffer[0] & 0xc0000000) == 0xc0000000)
        {
            // CSD_STRUCTURE [127:126] is 3
            // CSD version depend on EXT_CSD register in eMMC v4.4 for card size > 2GB
            fmiSDCmdAndRsp(pSD, 7, pSD->uRCA, 0);

            _fmi_pSDHCBuffer = (UINT8 *)((UINT32)_fmi_ucSDHCBuffer | 0x80000000);
            outpw(REG_DMACSAR, (UINT32)_fmi_pSDHCBuffer);   // set DMA transfer starting address
            outpw(REG_SDBLEN, 511);     // read 512 bytes for EXT_CSD
            if ((status = fmiSDCmdAndRspDataIn(pSD, 8, 0x00)) != Successful)
                return;

            fmiSDCommand(pSD, 7, 0);

            // According to SD spec v2.0 chapter 4.4,
            // "After the last SD Memory Card bus transaction, the host is required,
            //  to provide 8 (eight) clock cycles for the card to complete the
            //  operation before shutting down the clock."
            outpw(REG_SDCR, inpw(REG_SDCR)|SDCR_8CLK_OE);

            _info->totalSectorN = (*(UINT32 *)(_fmi_pSDHCBuffer+212));
            _info->diskSize = _info->totalSectorN / 2;
        }
        else
        {
            // CSD version v1.0/1.1/1.2 in eMMC v4.4 spec for card size <= 2GB
            R_LEN = (Buffer[1] & 0x000f0000) >> 16;
            C_Size = ((Buffer[1] & 0x000003ff) << 2) | ((Buffer[2] & 0xc0000000) >> 30);
            MULT = (Buffer[2] & 0x00038000) >> 15;
            size = (C_Size+1) * (1<<(MULT+2)) * (1<<R_LEN);

            _info->diskSize = size / 1024;
            _info->totalSectorN = size / 512;
        }
    }
    else
    {
        // for SD/SDHC card
        if ((Buffer[0] & 0xc0000000) && (pSD->uCardType != FMI_TYPE_MMC))
        {
            // CSD version 2.0 in SD v2.0 spec for SDHC card
            C_Size = ((Buffer[1] & 0x0000003f) << 16) | ((Buffer[2] & 0xffff0000) >> 16);
            size = (C_Size+1) * 512;    // Kbytes
    
            _info->diskSize = size;
            _info->totalSectorN = size << 1;
        }
        else
        {
            // CSD version 1.0 in SD v2.0 spec for SD card
            R_LEN = (Buffer[1] & 0x000f0000) >> 16;
            C_Size = ((Buffer[1] & 0x000003ff) << 2) | ((Buffer[2] & 0xc0000000) >> 30);
            MULT = (Buffer[2] & 0x00038000) >> 15;
            size = (C_Size+1) * (1<<(MULT+2)) * (1<<R_LEN);
    
            _info->diskSize = size / 1024;
            _info->totalSectorN = size / 512;
        }
    }
    _info->sectorSize = 512;

    g_max_valid_sector = _info->totalSectorN;

    fmiSDCmdAndRsp2(pSD, 10, pSD->uRCA, Buffer);

    _info->vendor[0] = (Buffer[0] & 0xff000000) >> 24;
    ptr = (unsigned char *)Buffer;
	ptr = (unsigned char *)Buffer + 3;
    for (i=0; i<5; i++)
        _info->product[i] = *ptr++;
	ptr = (unsigned char *)Buffer + 9;
    for (i=0; i<4; i++)
        _info->serial[i] = *ptr++;

#ifdef DEBUG
    sysprintf("SD card CID is %08X-%08X-%08X-%08X\n", Buffer[0], Buffer[1], Buffer[2], Buffer[3]);
#endif
}
