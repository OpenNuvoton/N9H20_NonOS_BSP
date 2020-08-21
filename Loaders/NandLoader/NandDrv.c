/**************************************************************************//**
 * @file     NandDrv.c
 * @brief    NandLoader source code for NAND driver.
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "wblib.h"
#include "turbowriter.h"

#define OPT_FIRST_4BLOCKS_ECC4
#define OPT_SUPPORT_H27UAG8T2A

/* functions */
INT fmiSMCheckRB()
{
    int timeout;

    timeout = 0;
    while(1)
    {
        if (inpw(REG_SMISR) & SMISR_RB0_IF)
        {
            outpw(REG_SMISR, SMISR_RB0_IF);
            return 1;
        }

        if (++timeout > 0x80000)
            return 0;
    }
}

// SM functions
INT fmiSM_Reset(VOID)
{
    UINT32 volatile i;

    outpw(REG_SMCMD, 0xff);
    for (i=100; i>0; i--);

    if (!fmiSMCheckRB())
        return -1;
    return 0;
}


VOID fmiSM_Initial(FMI_SM_INFO_T *pSM)
{
    if (pSM->nPageSize == NAND_PAGE_2KB)
    {
        outp32(REG_SMCSR, (inp32(REG_SMCSR) & ~SMCR_PSIZE) | PSIZE_2K);     // 2k page size
        outp32(REG_SMCSR, (inp32(REG_SMCSR) & ~SMCR_BCH_TSEL) | BCH_T4);    // BCH_T4 is selected
        outp32(REG_SMREAREA_CTL, 64);                           // redundant area size

#ifdef DEBUG
        printf("The Test NAND is 2KB page size\n");
#endif
    }
    else if (pSM->nPageSize == NAND_PAGE_512B)
    {
        outp32(REG_SMCSR, (inp32(REG_SMCSR) & ~SMCR_PSIZE));                // 512 page size
        outp32(REG_SMCSR, (inp32(REG_SMCSR) & ~SMCR_BCH_TSEL) | BCH_T4);    // BCH_T4 is selected
        outp32(REG_SMREAREA_CTL, 16);                           // redundant area size

#ifdef DEBUG
        printf("The Test NAND is 512B page size\n");
#endif
    }
    else    /* 4KB */
    {
    #ifdef OPT_SUPPORT_H27UAG8T2A

        if (pSM->bIsNandECC12 == TRUE)
        {
            outp32(REG_SMCSR, (inp32(REG_SMCSR) & ~SMCR_PSIZE) | PSIZE_4K);     // 4k page size
            outp32(REG_SMCSR, (inp32(REG_SMCSR) & ~SMCR_BCH_TSEL) | BCH_T12);   // BCH_T8 is selected
            outpw(REG_SMREAREA_CTL, (inpw(REG_SMREAREA_CTL) & ~SMRE_REA128_EXT) | 224);;  // Redundant area size
        }
        else
        {
            outp32(REG_SMCSR, (inp32(REG_SMCSR) & ~SMCR_PSIZE) | PSIZE_4K);     // 4k page size
            outp32(REG_SMCSR, (inp32(REG_SMCSR) & ~SMCR_BCH_TSEL) | BCH_T8);    // BCH_T8 is selected
            outp32(REG_SMREAREA_CTL, 128);                      // redundant area size
        }
    #else
        outp32(REG_SMCSR, (inp32(REG_SMCSR) & ~SMCR_PSIZE) | PSIZE_4K);     // 4k page size
        outp32(REG_SMCSR, (inp32(REG_SMCSR) & ~SMCR_BCH_TSEL) | BCH_T8);    // BCH_T8 is selected
        outp32(REG_SMREAREA_CTL, 128);                          // redundant area size
    #endif

#ifdef DEBUG
        printf("The Test NAND is 4KB page size, ECC8\n");
#endif
    }
}


INT fmiSM_ReadID(FMI_SM_INFO_T *pSM)
{
    UINT32 tempID[5];

    fmiSM_Reset();
    outpw(REG_SMCMD, 0x90);     // read ID command
    outpw(REG_SMADDR, EOA_SM);  // address 0x00

    tempID[0] = inpw(REG_SMDATA);
    tempID[1] = inpw(REG_SMDATA);
    tempID[2] = inpw(REG_SMDATA);
    tempID[3] = inpw(REG_SMDATA);
    tempID[4] = inpw(REG_SMDATA);

    if (tempID[0] == 0xC2)
        pSM->bIsCheckECC = FALSE;
    else
        pSM->bIsCheckECC = TRUE;

    pSM->bIsNandECC4 = FALSE;
    pSM->bIsNandECC8 = FALSE;
    pSM->bIsNandECC12 = FALSE;
    pSM->bIsNandECC15 = FALSE;

    switch (tempID[1])
    {
        /* page size 512B */
        case 0x79:  // 128M
            pSM->uSectorPerFlash = 255744;
            pSM->uBlockPerFlash = 8191;
            pSM->uPagePerBlock = 32;
            pSM->uSectorPerBlock = 32;
            pSM->bIsMulticycle = TRUE;
            pSM->nPageSize = NAND_PAGE_512B;
            pSM->bIsNandECC4 = TRUE;
            break;

        case 0x76:  // 64M
            pSM->uSectorPerFlash = 127872;
            pSM->uBlockPerFlash = 4095;
            pSM->uPagePerBlock = 32;
            pSM->uSectorPerBlock = 32;
            pSM->bIsMulticycle = TRUE;
            pSM->nPageSize = NAND_PAGE_512B;
            pSM->bIsNandECC4 = TRUE;
            break;

        case 0x75:  // 32M
            pSM->uSectorPerFlash = 63936;
            pSM->uBlockPerFlash = 2047;
            pSM->uPagePerBlock = 32;
            pSM->uSectorPerBlock = 32;
            pSM->bIsMulticycle = FALSE;
            pSM->nPageSize = NAND_PAGE_512B;
            pSM->bIsNandECC4 = TRUE;
            break;

        case 0x73:  // 16M
            pSM->uSectorPerFlash = 31968;   // max. sector no. = 999 * 32
            pSM->uBlockPerFlash = 1023;
            pSM->uPagePerBlock = 32;
            pSM->uSectorPerBlock = 32;
            pSM->bIsMulticycle = FALSE;
            pSM->nPageSize = NAND_PAGE_512B;
            pSM->bIsNandECC4 = TRUE;
            break;

        /* page size 2KB */
        case 0xf1:  // 128M
        case 0xd1:  // 128M
            pSM->uBlockPerFlash = 1023;
            pSM->uPagePerBlock = 64;
            pSM->uSectorPerBlock = 256;
            pSM->uSectorPerFlash = 255744;
            pSM->bIsMulticycle = FALSE;
            pSM->nPageSize = NAND_PAGE_2KB;
            pSM->bIsNandECC8 = TRUE;

            // 2013/10/22, support MXIC MX30LF1G08AA NAND flash
            // 2015/06/22, support MXIC MX30LF1G18AC NAND flash
            if ( ((tempID[0]==0xC2)&&(tempID[1]==0xF1)&&(tempID[2]==0x80)&&(tempID[3]==0x1D)) ||
                 ((tempID[0]==0xC2)&&(tempID[1]==0xF1)&&(tempID[2]==0x80)&&(tempID[3]==0x95)&&(tempID[4]==0x02)) )
            {
                // The first ID of this NAND is 0xC2 BUT it is NOT NAND ROM (read only)
                // So, we MUST change pSM->bIsCheckECC to TRUE to enable ECC feature.
                pSM->bIsCheckECC = TRUE;
            }
            break;

        case 0xda:  // 256M
            if ((tempID[3] & 0x33) == 0x11)
            {
                pSM->uBlockPerFlash = 2047;
                pSM->uPagePerBlock = 64;
                pSM->uSectorPerBlock = 256;
                pSM->bIsMLCNand = FALSE;
            }
            else if ((tempID[3] & 0x33) == 0x21)
            {
                pSM->uBlockPerFlash = 1023;
                pSM->uPagePerBlock = 128;
                pSM->uSectorPerBlock = 512;
                pSM->bIsMLCNand = TRUE;
            }

            pSM->uSectorPerFlash = 511488;
            pSM->bIsMulticycle = TRUE;
            pSM->nPageSize = NAND_PAGE_2KB;
            pSM->bIsNandECC8 = TRUE;
            break;

        case 0xdc:  // 512M
            // 2017/9/19, To support both Maker Founder MP4G08JAA
            //                        and Toshiba TC58NVG2S0HTA00 512MB NAND flash
            if ((tempID[0]==0x98)&&(tempID[2]==0x90)&&(tempID[3]==0x26)&&(tempID[4]==0x76))
            {
                pSM->uBlockPerFlash  = 2047;        // block index with 0-base. = physical blocks - 1
                pSM->uPagePerBlock   = 64;
                pSM->nPageSize       = NAND_PAGE_4KB;
                pSM->uSectorPerBlock = pSM->nPageSize / 512 * pSM->uPagePerBlock;
                pSM->bIsMLCNand      = FALSE;
                pSM->bIsMulticycle   = TRUE;
                pSM->bIsNandECC8     = TRUE;
                pSM->uSectorPerFlash = 1022976;
                break;
            }

            if ((tempID[3] & 0x33) == 0x11)
            {
                pSM->uBlockPerFlash = 4095;
                pSM->uPagePerBlock = 64;
                pSM->uSectorPerBlock = 256;
                pSM->bIsMLCNand = FALSE;
            }
            else if ((tempID[3] & 0x33) == 0x21)
            {
                pSM->uBlockPerFlash = 2047;
                pSM->uPagePerBlock = 128;
                pSM->uSectorPerBlock = 512;
                pSM->bIsMLCNand = TRUE;
            }

            pSM->uSectorPerFlash = 1022976;
            pSM->bIsMulticycle = TRUE;
            pSM->nPageSize = NAND_PAGE_2KB;
            pSM->bIsNandECC8 = TRUE;
            break;

        case 0xd3:  // 1024M
            pSM->bIsMLCNand = FALSE;
            pSM->bIsMulticycle = TRUE;
            pSM->bIsNandECC8 = TRUE;
            pSM->uSectorPerFlash = 2045952;

            // 2014/4/2, To support Samsung K9WAG08U1D 512MB NAND flash
            if ((tempID[0]==0xEC)&&(tempID[2]==0x51)&&(tempID[3]==0x95)&&(tempID[4]==0x58))
            {
                pSM->uBlockPerFlash  = 4095;        // block index with 0-base. = physical blocks - 1
                pSM->uPagePerBlock   = 64;
                pSM->nPageSize       = NAND_PAGE_2KB;
                pSM->uSectorPerBlock = pSM->nPageSize / 512 * pSM->uPagePerBlock;
                pSM->uSectorPerFlash = 1022976;
                break;
            }

            // 2016/9/29, support MXIC MX60LF8G18AC NAND flash
            if ((tempID[0]==0xC2)&&(tempID[2]==0xD1)&&(tempID[3]==0x95)&&(tempID[4]==0x5A))
            {
                // The first ID of this NAND is 0xC2 BUT it is NOT NAND ROM (read only)
                // So, we MUST change pSM->bIsCheckECC to TRUE to enable ECC feature.
                pSM->bIsCheckECC = TRUE;
            }

            if ((tempID[3] & 0x33) == 0x32)
            {
                pSM->uBlockPerFlash = 2047;
                pSM->uPagePerBlock = 128;
                pSM->uSectorPerBlock = 1024;    /* 128x8 */
                pSM->nPageSize = NAND_PAGE_4KB;
                pSM->bIsMLCNand = TRUE;
            }
            else if ((tempID[3] & 0x33) == 0x11)
            {
                pSM->uBlockPerFlash = 8191;
                pSM->uPagePerBlock = 64;
                pSM->uSectorPerBlock = 256;
                pSM->nPageSize = NAND_PAGE_2KB;
            }
            else if ((tempID[3] & 0x33) == 0x21)
            {
                pSM->uBlockPerFlash = 4095;
                pSM->uPagePerBlock = 128;
                pSM->uSectorPerBlock = 512;
                pSM->nPageSize = NAND_PAGE_2KB;
                pSM->bIsMLCNand = TRUE;
            }
            else if ((tempID[3] & 0x33) == 0x22)
            {
                pSM->uBlockPerFlash = 4095;
                pSM->uPagePerBlock = 64;
                pSM->uSectorPerBlock = 512; /* 64x8 */
                pSM->nPageSize = NAND_PAGE_4KB;
            }
            break;

        case 0xd5:  // 2048M
    #ifdef OPT_SUPPORT_H27UAG8T2A

            if ((tempID[0]==0xAD)&&(tempID[2] == 0x94)&&(tempID[3] == 0x25))
            {
                pSM->uBlockPerFlash = 4095;
                pSM->uPagePerBlock = 128;
                pSM->uSectorPerBlock = 1024;    /* 128x8 */
                pSM->nPageSize = NAND_PAGE_4KB;
                pSM->bIsMLCNand = TRUE;
                pSM->uSectorPerFlash = 4091904;
                pSM->bIsMulticycle = TRUE;
                pSM->bIsNandECC12 = TRUE;
                break;
            }
            else
            {
                if ((tempID[3] & 0x33) == 0x32)
                {
                    pSM->uBlockPerFlash = 4095;
                    pSM->uPagePerBlock = 128;
                    pSM->uSectorPerBlock = 1024;    /* 128x8 */
                    pSM->nPageSize = NAND_PAGE_4KB;
                    pSM->bIsMLCNand = TRUE;
                }
                else if ((tempID[3] & 0x33) == 0x11)
                {
                    pSM->uBlockPerFlash = 16383;
                    pSM->uPagePerBlock = 64;
                    pSM->uSectorPerBlock = 256;
                    pSM->nPageSize = NAND_PAGE_2KB;
                    pSM->bIsMLCNand = FALSE;
                }
                else if ((tempID[3] & 0x33) == 0x21)
                {
                    pSM->uBlockPerFlash = 8191;
                    pSM->uPagePerBlock = 128;
                    pSM->uSectorPerBlock = 512;
                    pSM->nPageSize = NAND_PAGE_2KB;
                    pSM->bIsMLCNand = TRUE;
                }

                pSM->uSectorPerFlash = 4091904;
                pSM->bIsMulticycle = TRUE;
                pSM->bIsNandECC8 = TRUE;
                break;
            }
    #else

            if ((tempID[3] & 0x33) == 0x32)
            {
                pSM->uBlockPerFlash = 4095;
                pSM->uPagePerBlock = 128;
                pSM->uSectorPerBlock = 1024;    /* 128x8 */
                pSM->nPageSize = NAND_PAGE_4KB;
                pSM->bIsMLCNand = TRUE;
            }
            else if ((tempID[3] & 0x33) == 0x11)
            {
                pSM->uBlockPerFlash = 16383;
                pSM->uPagePerBlock = 64;
                pSM->uSectorPerBlock = 256;
                pSM->nPageSize = NAND_PAGE_2KB;
                pSM->bIsMLCNand = FALSE;
            }
            else if ((tempID[3] & 0x33) == 0x21)
            {
                pSM->uBlockPerFlash = 8191;
                pSM->uPagePerBlock = 128;
                pSM->uSectorPerBlock = 512;
                pSM->nPageSize = NAND_PAGE_2KB;
                pSM->bIsMLCNand = TRUE;
            }

            pSM->uSectorPerFlash = 4091904;
            pSM->bIsMulticycle = TRUE;
            pSM->bIsNandECC8 = TRUE;
            break;
    #endif
        default:
            // 2013/9/25, support MXIC MX30LF1208AA NAND flash
            if ((tempID[0]==0xC2)&&(tempID[1]==0xF0)&&(tempID[2]==0x80)&&(tempID[3]==0x1D))
            {
                pSM->uBlockPerFlash  = 511;         // block index with 0-base. = physical blocks - 1
                pSM->uPagePerBlock   = 64;
                pSM->nPageSize       = NAND_PAGE_2KB;
                pSM->uSectorPerBlock = pSM->nPageSize / 512 * pSM->uPagePerBlock;
                pSM->bIsMLCNand      = FALSE;
                pSM->bIsMulticycle   = FALSE;
                pSM->bIsNandECC8     = TRUE;
                pSM->uSectorPerFlash = pSM->uSectorPerBlock * 500 / 1000 * 999;

                // The first ID of this NAND is 0xC2 BUT it is NOT NAND ROM.
                // So, we MUST change pSM->bIsCheckECC to TRUE.
                pSM->bIsCheckECC = TRUE;

                break;
            }

            sysprintf("ERROR: SM ID not support!! [%02x][%02x][%02x][%02x][%02x]\n", tempID[0], tempID[1], tempID[2], tempID[3], tempID[4]);
            return -1;
    }

    sysprintf("SM ID [%02x][%02x][%02x][%02x][%02x]\n", tempID[0], tempID[1], tempID[2], tempID[3], tempID[4]);
    return 0;
}


INT fmiSM2BufferM(FMI_SM_INFO_T *pSM, UINT32 uSector, UINT8 ucColAddr)
{
    /* clear R/B flag */
    while(!(inpw(REG_SMISR) & SMISR_RB0));
    outpw(REG_SMISR, SMISR_RB0_IF);

    outpw(REG_SMCMD, 0x00);     // read command
    outpw(REG_SMADDR, ucColAddr);       // CA0 - CA7
    outpw(REG_SMADDR, uSector & 0xff);  // PA0 - PA7
    if (!pSM->bIsMulticycle)
        outpw(REG_SMADDR, ((uSector >> 8) & 0xff)|0x80000000);      // PA8 - PA15
    else
    {
        outpw(REG_SMADDR, (uSector >> 8) & 0xff);                   // PA8 - PA15
        outpw(REG_SMADDR, ((uSector >> 16) & 0xff)|0x80000000);     // PA16 - PA17
    }

    if (!fmiSMCheckRB())
        return -1;

    return 0;
}


INT fmiSMECCErrCount(UINT32 ErrSt, BOOL bIsECC8)
{
    unsigned int volatile errCount;

    if ((ErrSt & 0x02) || (ErrSt & 0x200) || (ErrSt & 0x20000) || (ErrSt & 0x2000000))
    {
#ifdef DEBUG
        printf("uncorrectable!![0x%x]\n", ErrSt);
#endif
//      return FMI_SM_ECC_ERROR;
        return -1;
    }
    if (ErrSt & 0x01)
    {
        errCount = (ErrSt >> 2) & 0xf;
#ifdef DEBUG
        if (bIsECC8)
            printf("Field 5 have %d error!!\n", errCount);
        else
            printf("Field 1 have %d error!!\n", errCount);
#endif
    }
    if (ErrSt & 0x100)
    {
        errCount = (ErrSt >> 10) & 0xf;
#ifdef DEBUG
        if (bIsECC8)
            printf("Field 6 have %d error!!\n", errCount);
        else
            printf("Field 2 have %d error!!\n", errCount);
#endif
    }
    if (ErrSt & 0x10000)
    {
        errCount = (ErrSt >> 18) & 0xf;
#ifdef DEBUG
        if (bIsECC8)
            printf("Field 7 have %d error!!\n", errCount);
        else
            printf("Field 3 have %d error!!\n", errCount);
#endif
    }
    if (ErrSt & 0x1000000)
    {
        errCount = (ErrSt >> 26) & 0xf;
#ifdef DEBUG
        if (bIsECC8)
            printf("Field 8 have %d error!!\n", errCount);
        else
            printf("Field 4 have %d error!!\n", errCount);
#endif
    }
    return errCount;
}

static VOID fmiSM_CorrectData_BCH(UINT8 ucFieidIndex, UINT8 ucErrorCnt, UINT8* pDAddr)
{
    UINT32 uaData[16], uaAddr[16];
    UINT32 uaErrorData[4];
    UINT8   ii, jj;
    UINT32 uPageSize;

    uPageSize = inpw(REG_SMCSR) & SMCR_PSIZE;

    jj = ucErrorCnt/4;
    jj ++;
    if (jj > 4) jj = 4;

    for(ii=0; ii<jj; ii++)
    {
        uaErrorData[ii] = inpw(REG_BCH_ECC_DATA0 + ii*4);
    }

    for(ii=0; ii<jj; ii++)
    {
        uaData[ii*4+0] = uaErrorData[ii] & 0xff;
        uaData[ii*4+1] = (uaErrorData[ii]>>8) & 0xff;
        uaData[ii*4+2] = (uaErrorData[ii]>>16) & 0xff;
        uaData[ii*4+3] = (uaErrorData[ii]>>24) & 0xff;
    }

    jj = ucErrorCnt/2;
    jj ++;
    if (jj > 8) jj = 8;

    for(ii=0; ii<jj; ii++)
    {
        uaAddr[ii*2+0] = inpw(REG_BCH_ECC_ADDR0 + ii*4) & 0x1fff;
        uaAddr[ii*2+1] = (inpw(REG_BCH_ECC_ADDR0 + ii*4)>>16) & 0x1fff;
    }

    pDAddr += (ucFieidIndex-1)*0x200;

    for(ii=0; ii<ucErrorCnt; ii++)
    {
        if (uPageSize == PSIZE_8K)
        {
            switch(inpw(REG_SMCSR) & SMCR_BCH_TSEL)
            {
                case BCH_T4:    // 8K+256
                default:
                    if (uaAddr[ii] < 512)
                        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];
                    else
                    {
                        if (uaAddr[ii] < 515)
                        {
                            uaAddr[ii] -= 512;
                            uaAddr[ii] += 8*(ucFieidIndex-1);   // field offset, only Field-0
                            *((UINT8*)REG_SMRA_0+uaAddr[ii]) ^=  uaData[ii];
                        }
                        else
                        {
                            uaAddr[ii] = 543 - uaAddr[ii];
                            uaAddr[ii] = 7 - uaAddr[ii];
                            uaAddr[ii] += 8*(ucFieidIndex-1);   // field offset
                            *((UINT8*)REG_SMRA_32+uaAddr[ii]) ^=  uaData[ii];
                        }
                    }
                    break;

                case BCH_T8:    // 8K+256
                    if (uaAddr[ii] < 512)
                        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];
                    else
                    {
                        if (uaAddr[ii] < 515)
                        {
                            uaAddr[ii] -= 512;
                            uaAddr[ii] += 15*(ucFieidIndex-1);  // field offset
                            *((UINT8*)REG_SMRA_0+uaAddr[ii]) ^=  uaData[ii];
                        }
                        else
                        {
                            uaAddr[ii] = 543 - uaAddr[ii];
                            uaAddr[ii] = 14 - uaAddr[ii];
                            uaAddr[ii] += 15*(ucFieidIndex-1);  // field offset
                            *((UINT8*)REG_SMRA_4+uaAddr[ii]) ^=  uaData[ii];
                        }
                    }
                    break;

                case BCH_T12:   // 8K+376
                    if (uaAddr[ii] < 512)
                        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];
                    else
                    {
                        if (uaAddr[ii] < 515)
                        {
                            uaAddr[ii] -= 512;
                            uaAddr[ii] += 23*(ucFieidIndex-1);  // field offset
                            *((UINT8*)REG_SMRA_0+uaAddr[ii]) ^=  uaData[ii];
                        }
                        else
                        {
                            uaAddr[ii] = 543 - uaAddr[ii];
                            uaAddr[ii] = 22 - uaAddr[ii];
                            uaAddr[ii] += 23*(ucFieidIndex-1);  // field offset
                            *((UINT8*)REG_SMRA_2+uaAddr[ii]) ^=  uaData[ii];
                        }
                    }
                    break;

                case BCH_T15:
                    break;
            }
        }
        else if (uPageSize == PSIZE_4K)
        {
            switch(inpw(REG_SMCSR) & SMCR_BCH_TSEL)
            {
                case BCH_T4:    // 4K+128
                default:
                    if (uaAddr[ii] < 512)
                        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];
                    else
                    {
                        if (uaAddr[ii] < 515)
                        {
                            uaAddr[ii] -= 512;
                            uaAddr[ii] += 8*(ucFieidIndex-1);   // field offset, only Field-0
                            *((UINT8*)REG_SMRA_0+uaAddr[ii]) ^=  uaData[ii];
                        }
                        else
                        {
                            uaAddr[ii] = 543 - uaAddr[ii];
                            uaAddr[ii] = 7 - uaAddr[ii];
                            uaAddr[ii] += 8*(ucFieidIndex-1);   // field offset
                            *((UINT8*)REG_SMRA_16+uaAddr[ii]) ^=  uaData[ii];
                        }
                    }
                    break;

                case BCH_T8:    // 4K+128
                    if (uaAddr[ii] < 512)
                        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];
                    else
                    {
                        if (uaAddr[ii] < 515)
                        {
                            uaAddr[ii] -= 512;
                            uaAddr[ii] += 15*(ucFieidIndex-1);  // field offset
                            *((UINT8*)REG_SMRA_0+uaAddr[ii]) ^=  uaData[ii];
                        }
                        else
                        {
                            uaAddr[ii] = 543 - uaAddr[ii];
                            uaAddr[ii] = 14 - uaAddr[ii];
                            uaAddr[ii] += 15*(ucFieidIndex-1);  // field offset
                            *((UINT8*)REG_SMRA_2+uaAddr[ii]) ^=  uaData[ii];
                        }
                    }
                    break;

                case BCH_T12:   // 4K+216
                    if (uaAddr[ii] < 512)
                        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];
                    else
                    {
                        if (uaAddr[ii] < 515)
                        {
                            uaAddr[ii] -= 512;
                            uaAddr[ii] += 23*(ucFieidIndex-1);  // field offset
                            *((UINT8*)REG_SMRA_0+uaAddr[ii]) ^=  uaData[ii];
                        }
                        else
                        {
                            uaAddr[ii] = 543 - uaAddr[ii];
                            uaAddr[ii] = 22 - uaAddr[ii];
                            uaAddr[ii] += 23*(ucFieidIndex-1);  // field offset
                            *((UINT8*)REG_SMRA_8+uaAddr[ii]) ^=  uaData[ii];
                        }
                    }
                    break;
            }
        }
        else if (uPageSize == PSIZE_2K)
        {
            switch(inpw(REG_SMCSR) & SMCR_BCH_TSEL)
            {
                case BCH_T4:
                default:
                    if (uaAddr[ii] < 512)
                        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];
                    else
                    {
                        if (uaAddr[ii] < 515)
                        {
                            uaAddr[ii] -= 512;
                            uaAddr[ii] += 8*(ucFieidIndex-1);   // field offset, only Field-0
                            *((UINT8*)REG_SMRA_0+uaAddr[ii]) ^=  uaData[ii];
                        }
                        else
                        {
                            uaAddr[ii] = 543 - uaAddr[ii];
                            uaAddr[ii] = 7 - uaAddr[ii];
                            uaAddr[ii] += 8*(ucFieidIndex-1);   // field offset
                            *((UINT8*)REG_SMRA_8+uaAddr[ii]) ^=  uaData[ii];
                        }
                    }
                    break;

                case BCH_T8:
                    if (uaAddr[ii] < 512)
                        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];
                    else
                    {
                        if (uaAddr[ii] < 515)
                        {
                            uaAddr[ii] -= 512;
                            uaAddr[ii] += 15*(ucFieidIndex-1);  // field offset
                            *((UINT8*)REG_SMRA_0+uaAddr[ii]) ^=  uaData[ii];
                        }
                        else
                        {
                            uaAddr[ii] = 543 - uaAddr[ii];
                            uaAddr[ii] = 14 - uaAddr[ii];
                            uaAddr[ii] += 15*(ucFieidIndex-1);  // field offset
                            *((UINT8*)REG_SMRA_1+uaAddr[ii]) ^=  uaData[ii];
                        }
                    }
                    break;
            }
        }
        else
        {
            if (uaAddr[ii] < 512)
                *(pDAddr+uaAddr[ii]) ^=  uaData[ii];
            else
            {
                if (uaAddr[ii] < 515)
                {
                    uaAddr[ii] -= 512;
                    *((UINT8*)REG_SMRA_0+uaAddr[ii]) ^=  uaData[ii];
                }
                else
                {
                    uaAddr[ii] = 543 - uaAddr[ii];
                    uaAddr[ii] = 7 - uaAddr[ii];
                    *((UINT8*)REG_SMRA_2+uaAddr[ii]) ^=  uaData[ii];
                }
            }
        }
    }
}


INT fmiSM_Read_512(FMI_SM_INFO_T *pSM, UINT32 uSector, UINT32 uDAddr)
{
    int volatile ret=0;
    volatile UINT32 uStatus, uError;
    UINT32 uErrorCnt;

    outpw(REG_DMACSAR, uDAddr);
    ret = fmiSM2BufferM(pSM, uSector, 0);
    if (ret < 0)
        return ret;

    outpw(REG_SMISR, SMISR_DMA_IF);                 // clear DMA flag
    outpw(REG_SMISR, SMISR_ECC_FIELD_IF);           // clear ECC_FIELD flag
    outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_DRD_EN);

    uError = 0;
    while(1)
    {
        if (!(inpw(REG_SMCSR) & SMCR_DRD_EN))
            break;
    }

    if (pSM->bIsCheckECC)
    {
        while(1)
        {
            if (inpw(REG_SMISR) & SMISR_ECC_FIELD_IF)
            {
                if (inpw(REG_SMCSR) & BCH_T4)   // BCH_ECC selected
                {
                    uStatus = inpw(REG_SM_ECC_ST0);
                    uStatus &= 0x3f;

                    if ((uStatus & 0x03)==0x01)         // correctable error in 1st field
                    {
                        uErrorCnt = uStatus >> 2;
                        fmiSM_CorrectData_BCH(1, uErrorCnt, (UINT8*)uDAddr);

                #ifdef DEBUG
                        printf("Field 1 have %d error!!\n", uErrorCnt);
                #endif
                    }
                    else if (((uStatus & 0x03)==0x02)
                          ||((uStatus & 0x03)==0x03))   // uncorrectable error or ECC error
                    {
                #ifdef DEBUG
                        printf("SM uncorrectable error is encountered, %4x !!\n", uStatus);
                #endif
                        uError = 1;
                    }
                }
                else
                {
                #ifdef DEBUG
                    printf("Wrong BCH setting for page-512 NAND !!\n");
                #endif
                }

                outpw(REG_SMISR, SMISR_ECC_FIELD_IF);       // clear ECC_FLD_Error
            }

            if (inpw(REG_SMISR) & SMISR_DMA_IF)      // wait to finish DMAC transfer.
            {
                if ( !(inpw(REG_SMISR) & SMISR_ECC_FIELD_IF) )
                    break;
            }
        }
    }
    else
        outpw(REG_SMISR, SMISR_ECC_FIELD_IF);

    outpw(REG_SMISR, SMISR_DMA_IF);                 // clear DMA flag
    if (uError)
        return -1;
    return 0;
}

INT fmiSM_Read_2K(FMI_SM_INFO_T *pSM, UINT32 uPage, UINT32 uDAddr)
{
    volatile UINT32 uStatus, uError;
    UINT32 uErrorCnt, ii;

//  fmiSM_Reset();

    while(inpw(REG_DMACCSR) & FMI_BUSY);    // wait DMAC FMI ready;
    outpw(REG_DMACCSR, inpw(REG_DMACCSR) | DMAC_EN);
    outpw(REG_DMACSAR, uDAddr); // set DMA transfer starting address

    /* clear R/B flag */
    while(!(inpw(REG_SMISR) & SMISR_RB0));
    outpw(REG_SMISR, SMISR_RB0_IF);

    if (inpw(REG_SMISR) & SMISR_ECC_FIELD_IF)   /* ECC_FLD_IF */
    {
        //printf("read: ECC error!!\n");
        outpw(REG_SMISR, SMISR_ECC_FIELD_IF);
    }

    outpw(REG_SMCMD, 0x00); // read command
    outpw(REG_SMADDR, 0);   // CA0 - CA7
    outpw(REG_SMADDR, 0);   // CA8 - CA11
    outpw(REG_SMADDR, uPage & 0xff);    // PA0 - PA7
    if (!pSM->bIsMulticycle)
        outpw(REG_SMADDR, ((uPage >> 8) & 0xff)|EOA_SM);    // PA8 - PA15
    else
    {
        outpw(REG_SMADDR, (uPage >> 8) & 0xff);             // PA8 - PA15
        outpw(REG_SMADDR, ((uPage >> 16) & 0xff)|EOA_SM);   // PA16 - PA17
    }
    outpw(REG_SMCMD, 0x30);     // read command

    if (!fmiSMCheckRB())
    //  return FMI_SM_RB_ERR;
        return -1;

    outpw(REG_SMISR, SMISR_DMA_IF);                 // clear DMA flag
    outpw(REG_SMISR, SMISR_ECC_FIELD_IF);           // clear ECC_FIELD flag
    outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_DRD_EN);

    uError = 0;
//  if ((pSM->bIsCheckECC) || (inpw(REG_SMCSR)&SMCR_ECC_CHK) )
    if (pSM->bIsCheckECC)
    {
        while(1)
        {
            if (inpw(REG_SMISR) & SMISR_ECC_FIELD_IF)
            {
                uStatus = inpw(REG_SM_ECC_ST0);
                for (ii=1; ii<5; ii++)
                {
                #if 0
                    if (!(uStatus & 0x03))
                    {
                        uStatus >>= 8;
                        continue;
                    }
                #endif

                    if ((uStatus & 0x03)==0x01)  // correctable error in 1st field
                    {
                        uErrorCnt = uStatus >> 2;
                        fmiSM_CorrectData_BCH(ii, uErrorCnt, (UINT8*)uDAddr);
                #ifdef DEBUG
                        printf("Field %d have %d error!!\n", ii, uErrorCnt);
                #endif
                        break;
                    }
                    else if (((uStatus & 0x03)==0x02)
                          ||((uStatus & 0x03)==0x03)) // uncorrectable error or ECC error in 1st field
                    {
                #ifdef DEBUG
                        printf("SM uncorrectable error is encountered, %4x !!\n", uStatus);
                #endif
                        uError = 1;
                        break;
                    }
                    uStatus >>= 8;
                }

                outpw(REG_SMISR, SMISR_ECC_FIELD_IF);       // clear ECC_FLD_Error
            }

            if (inpw(REG_SMISR) & SMISR_DMA_IF)      // wait to finish DMAC transfer.
            {
                if ( !(inpw(REG_SMISR) & SMISR_ECC_FIELD_IF) )
                    break;
            }
        }
    }
    else
    {
        while(1)
        {
            outpw(REG_SMISR, SMISR_ECC_FIELD_IF);
            if (inpw(REG_SMISR) & SMISR_DMA_IF)
            {
                outpw(REG_SMISR, SMISR_DMA_IF);                 // clear DMA flag
                break;
            }
        }

    }

    outpw(REG_SMISR, SMISR_DMA_IF);                 // clear DMA flag
    if (uError)
        return -1;

    return 0;
}

INT fmiSM_Read_RA(FMI_SM_INFO_T *pSM, UINT32 uPage, UINT32 ucColAddr)
{
    /* clear R/B flag */

//  fmiSM_Reset();

    while(!(inpw(REG_SMISR) & SMISR_RB0));
    outpw(REG_SMISR, SMISR_RB0_IF);

    outpw(REG_SMCMD, 0x00);     // read command
    outpw(REG_SMADDR, ucColAddr);               // CA0 - CA7
    outpw(REG_SMADDR, (ucColAddr >> 8) & 0xFF); // CA8 - CA12
    outpw(REG_SMADDR, uPage & 0xff);            // PA0 - PA7
    if (!pSM->bIsMulticycle)
        outpw(REG_SMADDR, ((uPage >> 8) & 0xff)|EOA_SM);        // PA8 - PA15
    else
    {
        outpw(REG_SMADDR, (uPage >> 8) & 0xff);                 // PA8 - PA15
        outpw(REG_SMADDR, ((uPage >> 16) & 0xff)|EOA_SM);       // PA16 - PA17
    }
    outpw(REG_SMCMD, 0x30);     // read command

    if (!fmiSMCheckRB())
        return -1;

    return 0;
}

INT fmiSM_Read_RA_512(FMI_SM_INFO_T *pSM, UINT32 uPage, UINT32 uColumm)
{
    /* clear R/B flag */
    while(!(inpw(REG_SMISR) & SMISR_RB0));
    outpw(REG_SMISR, SMISR_RB0_IF);

    outpw(REG_SMCMD, 0x50);     // read command
    outpw(REG_SMADDR, uColumm);
    outpw(REG_SMADDR, uPage & 0xff);    // PA0 - PA7
    if (!pSM->bIsMulticycle)
        outpw(REG_SMADDR, ((uPage >> 8) & 0xff)|EOA_SM);        // PA8 - PA15
    else
    {
        outpw(REG_SMADDR, (uPage >> 8) & 0xff);                 // PA8 - PA15
        outpw(REG_SMADDR, ((uPage >> 16) & 0xff)|EOA_SM);       // PA16 - PA17
    }

    if (!fmiSMCheckRB())
        return -1;

    return 0;
}

INT fmiSM_Read_4K(FMI_SM_INFO_T *pSM, UINT32 uPage, UINT32 uDAddr)
{
//  UINT32 detect_addr;
//  int volatile ret;
    volatile UINT32 uStatus, uError;
    UINT32 uErrorCnt, ii, jj;

    outpw(REG_DMACSAR, uDAddr); // set DMA transfer starting address

    /* clear R/B flag */
    while(!(inpw(REG_SMISR) & SMISR_RB0));
    outpw(REG_SMISR, SMISR_RB0_IF);

    outpw(REG_SMCMD, 0x00);     // read command
    outpw(REG_SMADDR, 0);   // CA0 - CA7
    outpw(REG_SMADDR, 0);   // CA8 - CA11
    outpw(REG_SMADDR, uPage & 0xff);    // PA0 - PA7
    if (!pSM->bIsMulticycle)
        outpw(REG_SMADDR, ((uPage >> 8) & 0xff)|EOA_SM);        // PA8 - PA15
    else
    {
        outpw(REG_SMADDR, (uPage >> 8) & 0xff);                 // PA8 - PA15
        outpw(REG_SMADDR, ((uPage >> 16) & 0xff)|EOA_SM);       // PA16 - PA17
    }
    outpw(REG_SMCMD, 0x30);     // read command

    if (!fmiSMCheckRB())
    //  return FMI_SM_RB_ERR;
        return -1;

    outpw(REG_SMISR, SMISR_DMA_IF);                 // clear DMA flag
    outpw(REG_SMISR, SMISR_ECC_FIELD_IF);           // clear ECC_FIELD flag
    outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_DRD_EN);
    uError = 0;

    if ((pSM->bIsCheckECC) || (inpw(REG_SMCSR)&SMCR_ECC_CHK) )
    {
        while(1)
        {
            if (inpw(REG_SMISR) & SMISR_ECC_FIELD_IF)
            {
                for (jj=0; jj<2; jj++)
                {
                    uStatus = inpw(REG_SM_ECC_ST0+jj*4);
                    if (!uStatus)
                        continue;

                    for (ii=1; ii<5; ii++)
                    {
                        if (!(uStatus & 0x03))
                        {
                            uStatus >>= 8;
                            continue;
                        }

                        if ((uStatus & 0x03)==0x01)  // correctable error in 1st field
                        {
                            uErrorCnt = uStatus >> 2;
                            fmiSM_CorrectData_BCH(jj*4+ii, uErrorCnt, (UINT8*)uDAddr);
                    #ifdef DEBUG
                            printf("Field %d have %d error!!\n", jj*4+ii, uErrorCnt);
                    #endif
                            break;
                        }
                        else if (((uStatus & 0x03)==0x02)
                              ||((uStatus & 0x03)==0x03)) // uncorrectable error or ECC error in 1st field
                        {
                    #ifdef _DEBUG
                            printf("SM uncorrectable BCH error is encountered !!\n");
                    #endif
                            uError = 1;
                            break;
                        }
                        uStatus >>= 8;
                    }
                }
                outpw(REG_SMISR, SMISR_ECC_FIELD_IF);       // clear ECC_FLD_Error
            }

    #ifdef _SIC_USE_INT_
            if (_fmi_bIsSMDataReady)
            {
                if ( !(inpw(REG_SMISR) & SMISR_ECC_FIELD_IF) )
                    break;
            }
    #else
            if (inpw(REG_SMISR) & SMISR_DMA_IF)      // wait to finish DMAC transfer.
            {
                if ( !(inpw(REG_SMISR) & SMISR_ECC_FIELD_IF) )
                    break;
            }
    #endif  //_SIC_USE_INT_

        }
    }
    else
    {
        while(1)
        {
            outpw(REG_SMISR, SMISR_ECC_FIELD_IF);
            if (inpw(REG_SMISR) & SMISR_DMA_IF)
            {
                outpw(REG_SMISR, SMISR_DMA_IF);                 // clear DMA flag
                break;
            }
        }

    }
    if (uError)
        return -1;
    return 0;
}


/* function pointer */
BOOL volatile bIsNandInit = FALSE;
FMI_SM_INFO_T SMInfo, *pSM0, *pSM1;
INT sicSMInit()
{
    if (!bIsNandInit)
    {
        // enable SM
        /* select NAND control pin used */
        outpw(REG_GPDFUN, inpw(REG_GPDFUN) | 0x0003FC00);       // enable NAND NWR/NRD/RB0/RB1 pins
        outpw(REG_GPEFUN, inpw(REG_GPEFUN) | 0x00FF0000);       // enable NAND ALE/CLE/CS0/CS1 pins
        outpw(REG_SMCSR, inpw(REG_SMCSR) | 0x06000000);         // CS-0/CS-1 -> HIGH
        outpw(REG_SMCSR, inpw(REG_SMCSR) & ~0x02000000);        // CS-0      -> LOW
        outpw(REG_FMICR, FMI_SM_EN);

        /* init SM interface */
//      outpw(REG_SMCSR, (inpw(REG_SMCSR)&0xf8ffff80)|0x00000020);  // (512+16)*n
        outp32(REG_SMCSR, inp32(REG_SMCSR) | SMCR_SM_SWRST);
        outp32(REG_SMCSR, inp32(REG_SMCSR) & ~SMCR_SM_SWRST);

        outp32(REG_SMCSR, inp32(REG_SMCSR) & ~SMCR_PSIZE);                  // 0: 512-byte, 1: 2048-byte
        outp32(REG_SMCSR, inp32(REG_SMCSR) | SMCR_ECC_EN | SMCR_ECC_CHK | SMCR_REDUN_AUTO_WEN); /// enable ECC/ECC_CHK/Auto_Write
        outp32(REG_SMCSR, (inp32(REG_SMCSR) & ~SMCR_BCH_TSEL) | BCH_T4);    // BCH_T4 is selected

        outp32(REG_SMREAREA_CTL, inp32(REG_SMREAREA_CTL) & ~SMRE_MECC); // disable to mask ECC parithenable BCH ECC algorithm

        outp32(REG_SMCSR, inp32(REG_SMCSR) | SMCR_ECC_3B_PROTECT);  // BCH_T4 is selected

        /* SM chip select */
        outp32(REG_SMCSR, inp32(REG_SMCSR) & ~ SMCR_CS0);           // CS0 pin low
        outp32(REG_SMCSR, inp32(REG_SMCSR) |   SMCR_CS1);           // CS1 pin high

        /* set timing */
        outpw(REG_SMTCR, 0x3050b);	// 192MHz OK

        memset((char *)&SMInfo, 0, sizeof(FMI_SM_INFO_T));
        pSM0 = &SMInfo;

        if (fmiSM_ReadID(pSM0) < 0)
            return -1;
        fmiSM_Initial(pSM0);
        bIsNandInit = TRUE;
    }

    return 0;
}

static void sicSMselect(INT chipSel)
{
    if (chipSel == 0)
    {
        outpw(REG_GPDFUN, inpw(REG_GPDFUN) | 0x0003CC00);       // enable NAND NWR/NRD/RB0 pins
        outpw(REG_GPEFUN, inpw(REG_GPEFUN) | 0x00F30000);       // enable NAND ALE/CLE/CS0 pins
        outpw(REG_SMCSR, inpw(REG_SMCSR) & ~SMCR_CS0);
        outpw(REG_SMCSR, inpw(REG_SMCSR) |  SMCR_CS1);
    }
    else
    {
        outpw(REG_GPDFUN, inpw(REG_GPDFUN) | 0x0003F000);       // enable NAND NWR/NRD/RB1 pins
        outpw(REG_GPEFUN, inpw(REG_GPEFUN) | 0x00FC0000);       // enable NAND ALE/CLE/CS1 pins
        outpw(REG_SMCSR, inpw(REG_SMCSR) & ~SMCR_CS1);
        outpw(REG_SMCSR, inpw(REG_SMCSR) |  SMCR_CS0);
    }
}

INT sicSMpread(INT chipSel, INT PBA, INT page, UINT8 *buff)
{
    FMI_SM_INFO_T *pSM;
    int pageNo;
    int status=0;

    int i;
    char *ptr;

#ifdef OPT_SUPPORT_H27UAG8T2A
    int spareSize;
#endif

    sicSMselect(chipSel);
    if (chipSel == 0)
        pSM = pSM0;
    else
        pSM = pSM1;

    // enable SM
    outpw(REG_FMICR, FMI_SM_EN);
//  fmiSM_Initial(pSM);     //removed by mhuko

    PBA += pSM->uLibStartBlock;
    pageNo = PBA * pSM->uPagePerBlock + page;

#ifdef OPT_FIRST_4BLOCKS_ECC4
    if (PBA <= 3)
    {
    #ifdef OPT_SUPPORT_H27UAG8T2A
        // set to ECC8 for Block 0-3
        if (pSM->nPageSize == NAND_PAGE_4KB)    /* 4KB */
        {
            if (pSM->bIsNandECC12 == TRUE)
            {
                outpw(REG_SMCSR, inpw(REG_SMCSR) &  ~SMCR_BCH_TSEL);
                outpw(REG_SMCSR, inpw(REG_SMCSR) | BCH_T8);             // BCH_8 is selected
                outpw(REG_SMREAREA_CTL, (inpw(REG_SMREAREA_CTL) & ~SMRE_REA128_EXT) | 128);  // Redundant area size
            }
        }
        // set to ECC4 for Block 0-3
        else if (pSM->nPageSize == NAND_PAGE_2KB)   /* 2KB */
        {
            if (pSM->bIsNandECC8 == TRUE)
            {
                outpw(REG_SMCSR, inpw(REG_SMCSR) &  ~SMCR_BCH_TSEL);
                outpw(REG_SMCSR, inpw(REG_SMCSR) | BCH_T4);             // BCH_4 is selected
                outpw(REG_SMREAREA_CTL, (inpw(REG_SMREAREA_CTL) & ~SMRE_REA128_EXT) | 64);  // Redundant area size
            }
        }
    #else
        // set to ECC4 for Block 0-3
        if (pSM->nPageSize == NAND_PAGE_2KB)    /* 2KB */
        {
            if (pSM->bIsNandECC8 == TRUE)
            {
                outpw(REG_SMCSR, inpw(REG_SMCSR) &  ~SMCR_BCH_TSEL);
                outpw(REG_SMCSR, inpw(REG_SMCSR) | BCH_T4);             // BCH_4 is selected
                outpw(REG_SMREAREA_CTL, (inpw(REG_SMREAREA_CTL) & ~SMRE_REA128_EXT) | 64);  // Redundant area size
            }
        }
    #endif
    }
#endif

    if (pSM->nPageSize == NAND_PAGE_2KB)    /* 2KB */
    {
        ptr = (char *)REG_SMRA_0;
        fmiSM_Read_RA(pSM, pageNo, 2048);
        for (i=0; i<64; i++)
            *ptr++ = inpw(REG_SMDATA) & 0xff;

        status = fmiSM_Read_2K(pSM, pageNo, (UINT32)buff);
    }
    else if (pSM->nPageSize == NAND_PAGE_4KB)   /* 4KB */
    {
#ifdef OPT_SUPPORT_H27UAG8T2A
        spareSize = inpw(REG_SMREAREA_CTL) & SMRE_REA128_EXT;
        ptr = (char *)REG_SMRA_0;
        fmiSM_Read_RA(pSM, pageNo, 4096);

        for (i=0; i<spareSize; i++)
            *ptr++ = inpw(REG_SMDATA) & 0xff;
#else
        ptr = (char *)REG_SMRA_0;
        fmiSM_Read_RA(pSM, pageNo, 4096);
//      for (i=0; i<216; i++)
        for (i=0; i<128; i++)
            *ptr++ = inpw(REG_SMDATA) & 0xff;
#endif

        status = fmiSM_Read_4K(pSM, pageNo, (UINT32)buff);
    }
    else    /* 512B */
    {
        ptr = (char *)REG_SMRA_0;
        fmiSM_Read_RA_512(pSM, pageNo, 0);
        for (i=0; i<16; i++)
            *ptr++ = inpw(REG_SMDATA) & 0xff;

        status = fmiSM_Read_512(pSM, pageNo, (UINT32)buff);
    }

#ifdef DEBUG
    if (status)
        printf("read NAND page fail !!!\n");
#endif

#ifdef OPT_FIRST_4BLOCKS_ECC4
    if (PBA <= 3)
    {
    #ifdef OPT_SUPPORT_H27UAG8T2A
        // restore to ECC12
        if (pSM->nPageSize == NAND_PAGE_4KB)    /* 4KB */
        {
            if (pSM->bIsNandECC12 == TRUE)
            {
                outpw(REG_SMCSR, inpw(REG_SMCSR) &  ~SMCR_BCH_TSEL);
                outpw(REG_SMCSR, inpw(REG_SMCSR) | BCH_T12);            // BCH_8 is selected
                outpw(REG_SMREAREA_CTL, (inpw(REG_SMREAREA_CTL) & ~SMRE_REA128_EXT) | 224);  // Redundant area size
            }
        }
        // restore to ECC8
        else if (pSM->nPageSize == NAND_PAGE_2KB)   /* 2KB */
        {
            if (pSM->bIsNandECC8 == TRUE)
            {
                outpw(REG_SMCSR, inpw(REG_SMCSR) &  ~SMCR_BCH_TSEL);
                outpw(REG_SMCSR, inpw(REG_SMCSR) | BCH_T8);             // BCH_8 is selected
                outpw(REG_SMREAREA_CTL, (inpw(REG_SMREAREA_CTL) & ~SMRE_REA128_EXT) | 64);  // Redundant area size
            }
        }
    #else
        // restore to ECC8
        if (pSM->nPageSize == NAND_PAGE_2KB)    /* 2KB */
        {
            if (pSM->bIsNandECC8 == TRUE)
            {
                outpw(REG_SMCSR, inpw(REG_SMCSR) &  ~SMCR_BCH_TSEL);
                outpw(REG_SMCSR, inpw(REG_SMCSR) | BCH_T8);             // BCH_8 is selected
                outpw(REG_SMREAREA_CTL, (inpw(REG_SMREAREA_CTL) & ~SMRE_REA128_EXT) | 64);  // Redundant area size
            }
        }
    #endif
    }
#endif
    return status;
}

VOID fmiInitDevice()
{
    // Enable NAND Card Host Controller operation and driving clock.
    outpw(REG_AHBCLK, inp32(REG_AHBCLK) | SIC_CKE | NAND_CKE);  // enable NAND engine clock
    outpw(REG_AHBCLK, inp32(REG_AHBCLK) & ~SD_CKE);             // disable SD engine clock

    // DMAC Initial
    outpw(REG_DMACCSR, 0x00000003);
    outpw(REG_DMACCSR, 0x00000001);

    outpw(REG_FMICR, 0x01);     // reset FMI engine
}


/*-----------------------------------------------------------------------------
 * To check if block is valid or not.
 * The block is GOOD only when all checked bytes are 0xFF as below:
 *      NAND type       check pages     check bytes in spare area
 *      ---------       --------------  -------------------------
 *      SLC 512         1st & 2nd       1st & 6th
 *      SLC 2K/4K/8K    1st & 2nd       1st
 *      MLC 2K/4K/8K    1st & last      1st
 * Return:
 *      0: valid block
 *      1: invalid block
 *---------------------------------------------------------------------------*/
INT CheckBadBlockMark(UINT32 BlockNo)
{
    int volatile status=0;
    unsigned int volatile sector;
    unsigned char volatile data512=0xff, data517=0xff, blockStatus=0xff;

    FMI_SM_INFO_T *pSM;

    if (BlockNo == 0)
        return 0;

    pSM = pSM0;

    //--- check first page ...
    sector = BlockNo * pSM->uPagePerBlock;  // first page
    if (pSM->nPageSize == NAND_PAGE_512B)
        status = fmiSM_Read_RA_512(pSM, sector, 0);
    else
        status = fmiSM_Read_RA(pSM, sector, pSM->nPageSize);

    if (status < 0)
    {
        sysprintf("ERROR: CheckBadBlockMark() read fail, for block %d, return 0x%x\n", BlockNo, status);
        return 1;
    }

    // for 512B page size NAND
    if (pSM->nPageSize == NAND_PAGE_512B)
    {
        data512 = inpw(REG_SMDATA) & 0xff;
        data517 = inpw(REG_SMDATA);
        data517 = inpw(REG_SMDATA);
        data517 = inpw(REG_SMDATA);
        data517 = inpw(REG_SMDATA);
        data517 = inpw(REG_SMDATA) & 0xff;
        if ((data512 == 0xFF) && (data517 == 0xFF)) // check byte 1 & 6
        {
            //--- first page PASS; check second page ...
            fmiSM_Reset();
            status = fmiSM_Read_RA_512(pSM, sector+1, 0);
            if (status < 0)
            {
                sysprintf("ERROR: CheckBadBlockMark() read fail, for block %d, return 0x%x\n", BlockNo, status);
                return 1;
            }
            data512 = inpw(REG_SMDATA) & 0xff;
            data517 = inpw(REG_SMDATA);
            data517 = inpw(REG_SMDATA);
            data517 = inpw(REG_SMDATA);
            data517 = inpw(REG_SMDATA);
            data517 = inpw(REG_SMDATA) & 0xff;
            if ((data512 != 0xFF) || (data517 != 0xFF)) // check byte 1 & 6
            {
                fmiSM_Reset();
                return 1;   // invalid block
            }
        }
        else
        {
            fmiSM_Reset();
            return 1;   // invalid block
        }
    }
    // for 2K/4K/8K page size NAND
    else
    {
        blockStatus = inpw(REG_SMDATA) & 0xff;
        if (blockStatus == 0xFF)    // check first byte
        {
            if (pSM->bIsMLCNand == TRUE)
                //--- first page PASS; check last page for MLC
                sector = (BlockNo+1) * pSM->uPagePerBlock - 1;  // last page
            else
                //--- first page PASS; check second page for SLC
                sector++;                                       // second page

            fmiSM_Reset();
            status = fmiSM_Read_RA(pSM, sector, pSM->nPageSize);
            if (status < 0)
            {
                sysprintf("ERROR: CheckBadBlockMark() read fail, for block %d, return 0x%x\n", BlockNo, status);
                return 1;
            }
            blockStatus = inpw(REG_SMDATA) & 0xff;
            if (blockStatus != 0xFF)    // check first byte
            {
                fmiSM_Reset();
                return 1;   // invalid block
            }
        }
        else
        {
            fmiSM_Reset();
            return 1;   // invalid block
        }
    }

    fmiSM_Reset();
    return 0;   // valid block
}
