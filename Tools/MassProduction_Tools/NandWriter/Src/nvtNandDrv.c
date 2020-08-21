/**************************************************************************//**
 * @file     nvtNandDrv.c
 * @brief    N9H20 series NAND driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "N9H20.h"
#include "writer.h"

extern UINT32 NAND_BACKUP_BASE;

#define OPT_MARK_BAD_BLOCK_WHILE_ERASE_FAIL

INT8 nIsSysImage;
int volatile gMarkCount=0;
UINT8 *pInfo;
UINT32 gNandBackupAddress=0;
UINT32 gNandBackupAddressTmp=0;
int volatile g_u32ExtraDataSize;
BOOL volatile gbSystemImage;
extern INT32 gNandLoaderSize;

#define NAND_EXTRA_512      16
#define NAND_EXTRA_2K       64
#define NAND_EXTRA_4K       128
#define NAND_EXTRA_8K       376

#define PRINTF  sysprintf

extern INT fmiSM_Reset(FMI_SM_INFO_T *pSM);
extern INT fmiSMCheckRB(FMI_SM_INFO_T *pSM);
extern INT fmiSMCheckStatus(FMI_SM_INFO_T *pSM);

/* Set BCH function & Page Size */
VOID nvtSM_Initial(FMI_SM_INFO_T *pSM)  /* OK */
{
    gNandBackupAddress = NAND_BACKUP_BASE ;
    gNandBackupAddressTmp = 0;
    outpw(REG_SMCSR, inpw(REG_SMCSR) & (~SMCR_PSIZE));

    if (pSM->nPageSize == NAND_PAGE_8KB)
    {
        outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~(SMCR_BCH_TSEL|SMCR_ECC_EN)) | (BCH_T12 | SMCR_ECC_EN | SMCR_ECC_3B_PROTECT|PSIZE_8K));
        g_u32ExtraDataSize = NAND_EXTRA_8K;
        outpw(REG_SMREAREA_CTL, g_u32ExtraDataSize);
        PRINTF("The Test NAND is 8KB page size\n");
    }
    else if (pSM->nPageSize == NAND_PAGE_4KB)
    {
        outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~(SMCR_BCH_TSEL|SMCR_ECC_EN)) | (BCH_T8 | SMCR_ECC_EN | SMCR_ECC_3B_PROTECT | PSIZE_4K));
        g_u32ExtraDataSize = NAND_EXTRA_4K;
        outpw(REG_SMREAREA_CTL, g_u32ExtraDataSize);
        PRINTF("The Test NAND is 4KB page size\n");
    }
    else if (pSM->nPageSize == NAND_PAGE_2KB)
    {
        outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~(SMCR_BCH_TSEL|SMCR_ECC_EN)) | (BCH_T4 | SMCR_ECC_EN | SMCR_ECC_3B_PROTECT | PSIZE_2K));
        g_u32ExtraDataSize = NAND_EXTRA_2K;
        outpw(REG_SMREAREA_CTL, g_u32ExtraDataSize);
        PRINTF("The Test NAND is 2KB page size\n");
    }
    else if (pSM->nPageSize == NAND_PAGE_512B)
    {
        outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~(SMCR_BCH_TSEL|SMCR_ECC_EN)) | (BCH_T4 | SMCR_ECC_EN | SMCR_ECC_3B_PROTECT | PSIZE_512));
        g_u32ExtraDataSize = NAND_EXTRA_512;
        outpw(REG_SMREAREA_CTL, g_u32ExtraDataSize);
        PRINTF("The Test NAND is 512B page size\n");
    }
}

// SM functions
/*-----------------------------------------------------------------------------
 * Read NAND chip ID from chip and then set pSM and NDISK by chip ID.
 *---------------------------------------------------------------------------*/
INT nvtSM_ReadID(FMI_SM_INFO_T *pSM)
{
    UINT32 tempID[5];

    fmiSM_Reset(pSM);
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
        case 0x79:  // 128M
            pSM->uSectorPerFlash = 255744;
            pSM->uBlockPerFlash = 8191;
            pSM->uPagePerBlock = 32;
            pSM->uSectorPerBlock = 32;
            pSM->bIsMulticycle = TRUE;
            pSM->nPageSize = NAND_PAGE_512B;
            pSM->bIsNandECC4 = TRUE;
            pSM->bIsMLCNand = FALSE;
            break;

        case 0x76:  // 64M
            pSM->uSectorPerFlash = 127872;
            pSM->uBlockPerFlash = 4095;
            pSM->uPagePerBlock = 32;
            pSM->uSectorPerBlock = 32;
            pSM->bIsMulticycle = TRUE;
            pSM->nPageSize = NAND_PAGE_512B;
            pSM->bIsNandECC4 = TRUE;
            pSM->bIsMLCNand = FALSE;
            break;

        case 0x75:  // 32M
            pSM->uSectorPerFlash = 63936;
            pSM->uBlockPerFlash = 2047;
            pSM->uPagePerBlock = 32;
            pSM->uSectorPerBlock = 32;
            pSM->bIsMulticycle = FALSE;
            pSM->nPageSize = NAND_PAGE_512B;
            pSM->bIsNandECC4 = TRUE;
            pSM->bIsMLCNand = FALSE;
            break;

        case 0x73:  // 16M
            pSM->uSectorPerFlash = 31968;
            pSM->uBlockPerFlash = 1023;
            pSM->uPagePerBlock = 32;
            pSM->uSectorPerBlock = 32;
            pSM->bIsMulticycle = FALSE;
            pSM->nPageSize = NAND_PAGE_512B;
            pSM->bIsNandECC4 = TRUE;
            pSM->bIsMLCNand = FALSE;
            break;

        case 0xf1:  // 128M
        case 0xd1:
            pSM->uBlockPerFlash = 1023;
            pSM->uPagePerBlock = 64;
            pSM->uSectorPerBlock = 256;
            pSM->uSectorPerFlash = 255744;
            pSM->bIsMulticycle = FALSE;
            pSM->nPageSize = NAND_PAGE_2KB;
            pSM->bIsNandECC4 = TRUE;
            pSM->bIsMLCNand = FALSE;

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

            if ((tempID[3] & 0x0C) == 0x04)
                pSM->bIsNandECC4 = TRUE;
            else if ((tempID[3] & 0x0C) == 0x08)
                pSM->bIsNandECC4 = FALSE;

            pSM->uSectorPerFlash = 511488;
            pSM->bIsMulticycle = TRUE;
            pSM->nPageSize = NAND_PAGE_2KB;
            break;

        case 0xdc:  // 512M
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

            if ((tempID[3] & 0x0C) == 0x04)
                pSM->bIsNandECC4 = TRUE;
            else if ((tempID[3] & 0x0C) == 0x08)
                pSM->bIsNandECC4 = FALSE;

            pSM->uSectorPerFlash = 1022976;
            pSM->bIsMulticycle = TRUE;
            pSM->nPageSize = NAND_PAGE_2KB;
            break;

        case 0xd3:
            // 2014/4/2, To support Samsung K9WAG08U1D 512MB NAND flash
            if ((tempID[0]==0xEC)&&(tempID[2]==0x51)&&(tempID[3]==0x95)&&(tempID[4]==0x58))
            {
                pSM->uBlockPerFlash  = 4095;        // block index with 0-base. = physical blocks - 1
                pSM->uPagePerBlock   = 64;
                pSM->nPageSize       = NAND_PAGE_2KB;
                pSM->uSectorPerBlock = pSM->nPageSize / 512 * pSM->uPagePerBlock;
                pSM->bIsMLCNand      = FALSE;
                pSM->bIsMulticycle   = TRUE;
                pSM->bIsNandECC8     = TRUE;
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
                pSM->uSectorPerBlock = 1024;
                pSM->nPageSize = NAND_PAGE_4KB;
                pSM->bIsMLCNand = TRUE;
            }
            else if ((tempID[3] & 0x33) == 0x11)
            {
                pSM->uBlockPerFlash = 8191;
                pSM->uPagePerBlock = 64;
                pSM->uSectorPerBlock = 256;
                pSM->nPageSize = NAND_PAGE_2KB;
                pSM->bIsMLCNand = FALSE;
            }
            else if ((tempID[3] & 0x33) == 0x21)
            {
                pSM->uBlockPerFlash = 4095;
                pSM->uPagePerBlock = 128;
                pSM->uSectorPerBlock = 512;
                pSM->nPageSize = NAND_PAGE_2KB;
                pSM->bIsMLCNand = TRUE;
            }

            if ((tempID[3] & 0x0C) == 0x04)
                pSM->bIsNandECC4 = TRUE;
            else if ((tempID[3] & 0x0C) == 0x08)
                pSM->bIsNandECC4 = FALSE;

            pSM->uSectorPerFlash = 2045952;
            pSM->bIsMulticycle = TRUE;
            break;

        case 0xd5:  // 2048M
            if ((tempID[3] & 0x33) == 0x32)
            {
                pSM->uBlockPerFlash = 4095;
                pSM->uPagePerBlock = 128;
                    pSM->uSectorPerBlock = 1024;
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

            if(tempID[0] == 0x98)
            {
                if ((tempID[3]&0x03) == 0x02)   // 8K page size
                {
                    pSM->uBlockPerFlash = 2000;
                    pSM->uPagePerBlock = 128;
                    pSM->uSectorPerBlock = 2048;    /* 128x8 */
                    pSM->nPageSize = NAND_PAGE_8KB;
                    pSM->bIsMLCNand = TRUE;
                }
            }

            if ((tempID[3] & 0x0C) == 0x04)
                pSM->bIsNandECC4 = TRUE;
            else if ((tempID[3] & 0x0C) == 0x08)
                pSM->bIsNandECC4 = FALSE;

            pSM->uSectorPerFlash = 4091904;
            pSM->bIsMulticycle = TRUE;
            break;

        default:
            // 2013/9/25, support MXIC MX30LF1208AA NAND flash
            if ((tempID[0]==0xC2)&&(tempID[1]==0xF0)&&(tempID[2]==0x80)&&(tempID[3]==0x1D))
            {
                pSM->uBlockPerFlash  = 511;     // block index with 0-base. = physical blocks - 1
                pSM->uPagePerBlock   = 64;
                pSM->nPageSize       = NAND_PAGE_2KB;
                pSM->uSectorPerBlock = pSM->nPageSize / 512 * pSM->uPagePerBlock;
                pSM->bIsMLCNand      = FALSE;
                pSM->bIsMulticycle   = FALSE;
                pSM->bIsNandECC4     = TRUE;    // IBR use BCH T4 for blocks in Reserved Area.
                pSM->uSectorPerFlash = pSM->uSectorPerBlock * 500 / 1000 * 999;
                break;
            }

            PRINTF("ERROR: SM ID not support!! [%02X][%02X][%02X][%02X]\n", tempID[0], tempID[1], tempID[2], tempID[3]);
            return -1;
    }

    PRINTF("nvtSM_ReadID: Found %s NAND, ID [%02X][%02X][%02X][%02X], page size %d, BCH T%d\n",
        pSM->bIsMLCNand ? "MLC" : "SLC",
        tempID[0], tempID[1], tempID[2], tempID[3],
        pSM->nPageSize,
        pSM->bIsNandECC4*4 + pSM->bIsNandECC8*8 + pSM->bIsNandECC12*12 + pSM->bIsNandECC15*15
        );
    return 0;
}


INT nvtSM_Write_512(FMI_SM_INFO_T *pSM, UINT32 uSector, UINT32 ucColAddr, UINT32 uSAddr)    //Modified
{
    int temp;

    // set the spare area configuration
    if (nIsSysImage != 0xFF)
    {
        if (((gMarkCount >> 8) +1) * pSM->nPageSize >= gNandLoaderSize)
        {
            outpw(REG_SMRA_0, 0x00A55AFF);      // Stop Marker
            gMarkCount += 0x100;
        }
        else
        {
            temp = (((nIsSysImage + gMarkCount) & 0xFFFF) << 8) + 0x000000FF;
            outpw(REG_SMRA_0, temp);
            gMarkCount += 0x100;
            if(gMarkCount == 0x1D)  /* Next page is 0x1D (page 29) */
                gMarkCount = 0xA500;
        }
    }
    else
    {
        outpw(REG_SMRA_0, 0x0000FFFF);
        gMarkCount = 0;
    }

    outpw(REG_FMICR, inpw(REG_FMICR)|FMI_SM_EN);
    fmiSM_Reset(pSM);
    /* set Page Size = 512 bytes */
    outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~(SMCR_BCH_TSEL|SMCR_ECC_EN)) | (BCH_T4 | SMCR_ECC_EN | SMCR_ECC_3B_PROTECT | PSIZE_512));

    outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_REDUN_AUTO_WEN);            // enable Auto Write
    outpw(REG_SMCSR, inpw(REG_SMCSR) & ~SMCR_REDUN_REN);                // enable Read
    /* enable DMAC */
    while(inpw(REG_DMACCSR) & FMI_BUSY);                // wait DMAC FMI ready;
    outpw(REG_DMACCSR, inpw(REG_DMACCSR) | DMAC_EN);

    /* set DMA Transfer Starting Address */
    outpw(REG_DMACSAR, uSAddr);

    /* set SM columm & page address */
    outpw(REG_SMCMD, 0x80);                             // Program setup command
    outpw(REG_SMADDR, ucColAddr & 0xFF);                // CA0 - CA7
    outpw(REG_SMADDR, uSector & 0xFF);                  // PA0 - PA7

    if (!pSM->bIsMulticycle)
        outpw(REG_SMADDR, ((uSector >> 8) & 0xFF) | EOA_SM);    // PA8 - PA15
    else
    {
        outpw(REG_SMADDR, (uSector >> 8) & 0xFF);               // PA8 - PA15
        outpw(REG_SMADDR, ((uSector >> 16) & 0xFF) | EOA_SM);   // PA16 - PA17
    }

    /* begin DMA write transfer */
    outpw(REG_SMISR, SMISR_DMA_IF);         // clear DMA flag
    outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_DWR_EN);

    while(!(inpw(REG_SMISR) & SMISR_DMA_IF));           // wait to finish DMAC transfer.
    outpw(REG_SMISR, SMISR_DMA_IF);                     // clear DMA flag

    outpw(REG_SMCMD, 0x10);                             // Progeam command

    if (!fmiSMCheckRB(pSM))
        return -1;

    if (fmiSMCheckStatus(pSM) != 0)
    {
        PRINTF("nvtSM_Write_512: write data error on sector [%d]!!\n", uSector);
        return -1;  // Program Fail
    }
    return 0;
}


INT nvtSM_Write_2K(FMI_SM_INFO_T *pSM, UINT32 uSector, UINT32 ucColAddr, UINT32 uSAddr) //Modified
{
    int temp;

    if (nIsSysImage != 0xFF)
    {
        if (((gMarkCount >> 8) +1) * pSM->nPageSize >= gNandLoaderSize)
        {
            outpw(REG_SMRA_0, 0x00A55AFF);      // Stop Marker
            gMarkCount += 0x100;
        }
        else
        {
            temp = (((nIsSysImage + gMarkCount) & 0xFFFF) << 8) + 0x000000FF;
            outpw(REG_SMRA_0, temp);
            gMarkCount += 0x100;
        }
    }
    else
    {
        outpw(REG_SMRA_0, 0x0000FFFF);
        gMarkCount = 0;
    }

    outpw(REG_FMICR, inpw(REG_FMICR)|FMI_SM_EN);

    fmiSM_Reset(pSM);

    /* set Page Size = 2048 bytes */
    if(gbSystemImage)
        outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~(SMCR_BCH_TSEL|SMCR_ECC_EN)) | (BCH_T4 | SMCR_ECC_EN | SMCR_ECC_3B_PROTECT | PSIZE_2K));
    else
        outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~(SMCR_BCH_TSEL|SMCR_ECC_EN)) | (BCH_T8 | SMCR_ECC_EN | SMCR_ECC_3B_PROTECT | PSIZE_2K));

    outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_REDUN_AUTO_WEN);            // enable Auto Write

    /* enable DMAC */
    while(inpw(REG_DMACCSR) & FMI_BUSY);    // wait DMAC FMI ready;
    outpw(REG_DMACCSR, inpw(REG_DMACCSR) | DMAC_EN);

    /* Set DMA Transfer Starting Address */
    outpw(REG_DMACSAR, uSAddr);

    outpw(REG_SMCMD, 0x80);                 // Program setup command

    outpw(REG_SMADDR, ucColAddr & 0xFF);
    outpw(REG_SMADDR, (ucColAddr >> 8) & 0xFF);
    outpw(REG_SMADDR, uSector & 0xFF);

    if (!pSM->bIsMulticycle)
        outpw(REG_SMADDR, ((uSector >> 8) & 0xFF) | EOA_SM);    // PA8 - PA15
    else
    {
        outpw(REG_SMADDR, (uSector >> 8) & 0xFF);               // PA8 - PA15
        outpw(REG_SMADDR, ((uSector >> 16) & 0xFF) | EOA_SM);   // PA16 - PA17
    }

    /* begin DMA write transfer */
    outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_DWR_EN);

    while(!(inpw(REG_SMISR) & SMISR_DMA_IF));       // wait to finish DMAC transfer.
    outpw(REG_SMISR, SMISR_DMA_IF);                 // clear DMA flag

    outpw(REG_SMCMD, 0x10);                         // Progeam command

    if (!fmiSMCheckRB(pSM))
        return -1;

    if (fmiSMCheckStatus(pSM) != 0)
    {
        PRINTF("nvtSM_Write_2K: write data error on sector [%d]!!\n", uSector);
        return -1;  // Program Fail
    }
    return 0;       // Program Success
}


INT nvtSM_Write_4K(FMI_SM_INFO_T *pSM, UINT32 uSector, UINT32 ucColAddr, UINT32 uSAddr) //Modified
{
    int temp;

    if (nIsSysImage != 0xFF)
    {
        if (((gMarkCount >> 8) +1) * pSM->nPageSize >= gNandLoaderSize)
        {
            outpw(REG_SMRA_0, 0x00A55AFF);      // Stop Marker
            gMarkCount += 0x100;
        }
        else
        {
            temp = (((nIsSysImage + gMarkCount) & 0xFFFF) << 8) + 0x000000FF;
            outpw(REG_SMRA_0, temp);
            gMarkCount += 0x100;
        }
    }
    else
    {
        outpw(REG_SMRA_0, 0x0000FFFF);
        gMarkCount = 0;
    }

    outpw(REG_FMICR, inpw(REG_FMICR)|FMI_SM_EN);
    fmiSM_Reset(pSM);
    outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~(SMCR_BCH_TSEL|SMCR_ECC_EN)) | (BCH_T8 | SMCR_ECC_EN | SMCR_ECC_3B_PROTECT | PSIZE_4K));
    outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_REDUN_AUTO_WEN);            // enable Auto Write

    /* enable DMAC */
    while(inpw(REG_DMACCSR) & FMI_BUSY);    // wait DMAC FMI ready;
    outpw(REG_DMACCSR, inpw(REG_DMACCSR) | DMAC_EN);

    /* Set DMA Transfer Starting Address */
    outpw(REG_DMACSAR, uSAddr);

    outpw(REG_SMCMD, 0x80);                 // Program setup command

    outpw(REG_SMADDR, ucColAddr & 0xFF);
    outpw(REG_SMADDR, (ucColAddr >> 8) & 0xFF);
    outpw(REG_SMADDR, uSector & 0xFF);

    if (!pSM->bIsMulticycle)
    {
        outpw(REG_SMADDR, ((uSector >> 8) & 0xFF) | EOA_SM);    // PA8 - PA15
    }
    else
    {
        outpw(REG_SMADDR, (uSector >> 8) & 0xFF);               // PA8 - PA15
        outpw(REG_SMADDR, ((uSector >> 16) & 0xFF) | EOA_SM);   // PA16 - PA17
    }

    /* begin DMA write transfer */
    outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_DWR_EN);

    while(!(inpw(REG_SMISR) & SMISR_DMA_IF));   // wait to finish DMAC transfer.
    outpw(REG_SMISR, SMISR_DMA_IF);             // clear DMA flag

    outpw(REG_SMCMD, 0x10);     // auto program command

    if (!fmiSMCheckRB(pSM))
        return -1;

    if (fmiSMCheckStatus(pSM) != 0)
    {
        PRINTF("nvtSM_Write_4K: write data error on sector [%d]!!\n", uSector);
        return -1;  // Program Fail
    }

    return 0;
}

INT nvtSM_Write_8K(FMI_SM_INFO_T *pSM, UINT32 uSector, UINT32 ucColAddr, UINT32 uSAddr) //Added
{
    int temp;

    if (nIsSysImage != 0xFF)
    {
        if (((gMarkCount >> 8) +1) * pSM->nPageSize >= gNandLoaderSize)
        {
            outpw(REG_SMRA_0, 0x00A55AFF);      // Stop Marker
            gMarkCount += 0x100;
        }
        else
        {
            temp = (((nIsSysImage + gMarkCount) & 0xFFFF) << 8) + 0x000000FF;
            outpw(REG_SMRA_0, temp);
            gMarkCount += 0x100;
        }
    }
    else
    {
        outpw(REG_SMRA_0, 0x0000FFFF);
        gMarkCount = 0;
    }

    outpw(REG_FMICR, inpw(REG_FMICR)|FMI_SM_EN);
    fmiSM_Reset(pSM);
    outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~(SMCR_BCH_TSEL|SMCR_ECC_EN)) | (BCH_T12 | SMCR_ECC_EN | SMCR_ECC_3B_PROTECT|PSIZE_8K));
    outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_REDUN_AUTO_WEN);            // enable Auto Write

    /* enable DMAC */
    while(inpw(REG_DMACCSR) & FMI_BUSY);    // wait DMAC FMI ready;
    outpw(REG_DMACCSR, inpw(REG_DMACCSR) | DMAC_EN);

    /* Set DMA Transfer Starting Address */
    outpw(REG_DMACSAR, uSAddr);

    outpw(REG_SMCMD, 0x80);                 // Program setup command

    outpw(REG_SMADDR, ucColAddr & 0xFF);
    outpw(REG_SMADDR, (ucColAddr >> 8) & 0xFF);
    outpw(REG_SMADDR, uSector & 0xFF);

    if (!pSM->bIsMulticycle)
    {
        outpw(REG_SMADDR, ((uSector >> 8) & 0xFF) | EOA_SM);    // PA8 - PA15
    }
    else
    {
        outpw(REG_SMADDR, (uSector >> 8) & 0xFF);               // PA8 - PA15
        outpw(REG_SMADDR, ((uSector >> 16) & 0xFF) | EOA_SM);   // PA16 - PA17
    }

    /* begin DMA write transfer */
    outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_DWR_EN);

    while(!(inpw(REG_SMISR) & SMISR_DMA_IF));   // wait to finish DMAC transfer.
    outpw(REG_SMISR, SMISR_DMA_IF);             // clear DMA flag

    outpw(REG_SMCMD, 0x10);     // auto program command

    if (!fmiSMCheckRB(pSM))
        return -1;

    if (fmiSMCheckStatus(pSM) != 0)
    {
        PRINTF("nvtSM_Write_8K: write data error on sector [%d]!!\n", uSector);
        return -1;  // Program Fail
    }
    return 0;
}

/* BCH Correction */
static void nvtSM_CorrectData_BCH(UINT32 u32PageSize, UINT8 ucFieidIndex, UINT8 ucErrorCnt, UINT8* pDAddr)  //Added
{
    UINT32 uaData[16], uaAddr[16];
    UINT32 uaErrorData[4];
    UINT8   ii;

    for(ii=0; ii<4; ii++)
    {
        uaErrorData[ii] = inpw(REG_BCH_ECC_DATA0 + ii*4);
    }

    for(ii=0; ii<4; ii++)
    {
        uaData[ii*4+0] = uaErrorData[ii] & 0xff;
        uaData[ii*4+1] = (uaErrorData[ii]>>8) & 0xff;
        uaData[ii*4+2] = (uaErrorData[ii]>>16) & 0xff;
        uaData[ii*4+3] = (uaErrorData[ii]>>24) & 0xff;
    }

    for(ii=0; ii<8; ii++)
    {
        uaAddr[ii*2+0] = inpw(REG_BCH_ECC_ADDR0 + ii*4) & 0x1fff;
        uaAddr[ii*2+1] = (inpw(REG_BCH_ECC_ADDR0 + ii*4)>>16) & 0x1fff;
    }

    pDAddr += (ucFieidIndex-1)*0x200;

    for(ii=0; ii<ucErrorCnt; ii++)
    {
        if (u32PageSize == 8192)
        {
            switch(inpw(REG_SMCSR) & SMCR_BCH_TSEL)
            {
                case BCH_T4:    // 4K+128
                default:
                    if (uaAddr[ii] < 512)
                        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];
                    else
                    {
                        uaAddr[ii] = 543 - uaAddr[ii];
                        uaAddr[ii] = 7 - uaAddr[ii];
                        uaAddr[ii] += 8*(ucFieidIndex-1);   // field offset
                        *((UINT8*)REG_SMRA_16+uaAddr[ii]) ^=  uaData[ii];
                    }
                    break;

                case BCH_T8:    // 4K+128
                    if (uaAddr[ii] < 512)
                        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];
                    else
                    {
                        uaAddr[ii] = 543 - uaAddr[ii];
                        uaAddr[ii] = 14 - uaAddr[ii];
                        uaAddr[ii] += 15*(ucFieidIndex-1);  // field offset
                        *((UINT8*)REG_SMRA_2+uaAddr[ii]) ^=  uaData[ii];
                    }

                    break;

                case BCH_T12:   // 4K+216
                    if (uaAddr[ii] < 512)
                        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];
                    else
                    {
                        uaAddr[ii] = 543 - uaAddr[ii];
                        uaAddr[ii] = 22 - uaAddr[ii];
                        uaAddr[ii] += 23*(ucFieidIndex-1);  // field offset
                        *((UINT8*)REG_SMRA_2+uaAddr[ii]) ^=  uaData[ii];
                    }

                    break;
            }
        }
        else if (u32PageSize == 4096)
        {
            switch(inpw(REG_SMCSR) & SMCR_BCH_TSEL)
            {
                case BCH_T4:    // 4K+128
                default:
                    if (uaAddr[ii] < 512)
                        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];
                    else
                    {
                        uaAddr[ii] = 543 - uaAddr[ii];
                        uaAddr[ii] = 7 - uaAddr[ii];
                        uaAddr[ii] += 8*(ucFieidIndex-1);   // field offset
                        *((UINT8*)REG_SMRA_16+uaAddr[ii]) ^=  uaData[ii];
                    }
                    break;

                case BCH_T8:    // 4K+128
                    if (uaAddr[ii] < 512)
                        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];
                    else
                    {
                        uaAddr[ii] = 543 - uaAddr[ii];
                        uaAddr[ii] = 14 - uaAddr[ii];
                        uaAddr[ii] += 15*(ucFieidIndex-1);  // field offset
                        *((UINT8*)REG_SMRA_2+uaAddr[ii]) ^=  uaData[ii];
                    }

                    break;

                case BCH_T12:   // 4K+216
                    if (uaAddr[ii] < 512)
                        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];
                    else
                    {
                        uaAddr[ii] = 543 - uaAddr[ii];
                        uaAddr[ii] = 22 - uaAddr[ii];
                        uaAddr[ii] += 23*(ucFieidIndex-1);  // field offset
                        *((UINT8*)REG_SMRA_8+uaAddr[ii]) ^=  uaData[ii];
                    }

                    break;
            }
        }
        else if (u32PageSize == 2048)
        {
            switch(inpw(REG_SMCSR) & SMCR_BCH_TSEL)
            {
                case BCH_T4:
                default:
                    if (uaAddr[ii] < 512)
                        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];
                    else
                    {
                        uaAddr[ii] = 543 - uaAddr[ii];
                        uaAddr[ii] = 7 - uaAddr[ii];
                        uaAddr[ii] += 8*(ucFieidIndex-1);   // field offset
                        *((UINT8*)REG_SMRA_8+uaAddr[ii]) ^=  uaData[ii];
                    }
                    break;

                case BCH_T8:
                    if (uaAddr[ii] < 512)
                        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];
                    else
                    {
                        uaAddr[ii] = 543 - uaAddr[ii];
                        uaAddr[ii] = 14 - uaAddr[ii];
                        uaAddr[ii] += 15*(ucFieidIndex-1);  // field offset
                        *((UINT8*)REG_SMRA_1+uaAddr[ii]) ^=  uaData[ii];
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
                uaAddr[ii] = 543 - uaAddr[ii];
                uaAddr[ii] = 7 - uaAddr[ii];
                *((UINT8*)REG_SMRA_2+uaAddr[ii]) ^=  uaData[ii];
            }

        }

    }
}

/* Read Extra Data */
int nvtSM_ReadOnePage_ExtraData(FMI_SM_INFO_T *pSM, UINT32 uPageIndex)  //Added
{
    UINT32 ii;
    UINT8 *uDAddr;

    outpw(REG_FMICR, inpw(REG_FMICR)|FMI_SM_EN);

    fmiSM_Reset(pSM);

    /* set Page Size = 8192 bytes */
    if(pSM->nPageSize == 512)
    {
        outpw(REG_SMCSR, inpw(REG_SMCSR) & (~SMCR_PSIZE));      //uExtraNo->16
        /* set READ command */
        outpw(REG_SMCMD, 0x50);             // READ command 1 (0x50), read from extra area address (area C)
        outpw(REG_SMADDR, 0x00);
    }
    else
    {
        switch(pSM->nPageSize)
        {
            case 2048:
                outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~SMCR_PSIZE)|PSIZE_2K); //uExtraNo->64
                /* set READ command */
                outpw(REG_SMCMD, 0x00);     // READ 1st cycle command
                outpw(REG_SMADDR, 0x00);
                outpw(REG_SMADDR, 0x08);
                break;
            case 4096:
                outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~SMCR_PSIZE)|PSIZE_4K);
                /* set READ command */
                outpw(REG_SMCMD, 0x00);     // READ 1st cycle command
                outpw(REG_SMADDR, 0x00);
                outpw(REG_SMADDR, 0x10);

                break;
            case 8192:
                outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~SMCR_PSIZE)|PSIZE_8K);
                /* set READ command */
                outpw(REG_SMCMD, 0x00);     // READ 1st cycle command
                outpw(REG_SMADDR, 0x00);
                outpw(REG_SMADDR, 0x20);
                break;
        }
    }

    outpw(REG_SMADDR, uPageIndex & 0xFF);

    if (!pSM->bIsMulticycle)
        outpw(REG_SMADDR, ((uPageIndex >> 8) & 0xFF) | EOA_SM);     // PA8 - PA15
    else
    {
        outpw(REG_SMADDR, (uPageIndex >> 8) & 0xFF);                // PA8 - PA15
        outpw(REG_SMADDR, ((uPageIndex >> 16) & 0xFF) | EOA_SM);    // PA16 - PA17
    }

    if(pSM->nPageSize != 512)
        outpw(REG_SMCMD, 0x30);         // READ 2nd cycle command

    if (!fmiSMCheckRB(pSM))
        return -1;

    uDAddr = (UINT8*) REG_SMRA_0;
    for (ii=0; ii<g_u32ExtraDataSize; ii++)
    {
        *(UINT8*) uDAddr = inpb(REG_SMDATA);

        uDAddr++;
    }

    return 0;
}

/* Read Extra Data */
int fmiSM_ReadBlockStatus(FMI_SM_INFO_T *pSM, UINT32 uPageIndex, PUINT8 pu8Status)  //Added
{
    unsigned char data, data517;

    outpw(REG_FMICR, inpw(REG_FMICR)|FMI_SM_EN);

    fmiSM_Reset(pSM);

    /* set Page Size = 8192 bytes */
    if(pSM->nPageSize == 512)
    {
        outpw(REG_SMCSR, inpw(REG_SMCSR) & (~SMCR_PSIZE));      //uExtraNo->16
        /* set READ command */
        outpw(REG_SMCMD, 0x50);             // READ command 1 (0x50), read from extra area address (area C)
        outpw(REG_SMADDR, 0x00);
    }
    else
    {
        switch(pSM->nPageSize)
        {
            case 2048:
                outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~SMCR_PSIZE)|PSIZE_2K); //uExtraNo->64
                /* set READ command */
                outpw(REG_SMCMD, 0x00);     // READ 1st cycle command
                outpw(REG_SMADDR, 0x00);
                outpw(REG_SMADDR, 0x08);
                break;
            case 4096:
                outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~SMCR_PSIZE)|PSIZE_4K);
                /* set READ command */
                outpw(REG_SMCMD, 0x00);     // READ 1st cycle command
                outpw(REG_SMADDR, 0x00);
                outpw(REG_SMADDR, 0x10);

                break;
            case 8192:
                outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~SMCR_PSIZE)|PSIZE_8K);
                /* set READ command */
                outpw(REG_SMCMD, 0x00);     // READ 1st cycle command
                outpw(REG_SMADDR, 0x00);
                outpw(REG_SMADDR, 0x20);
                break;
        }
    }

    outpw(REG_SMADDR, uPageIndex & 0xFF);

    if (!pSM->bIsMulticycle)
        outpw(REG_SMADDR, ((uPageIndex >> 8) & 0xFF) | EOA_SM);     // PA8 - PA15
    else
    {
        outpw(REG_SMADDR, (uPageIndex >> 8) & 0xFF);                // PA8 - PA15
        outpw(REG_SMADDR, ((uPageIndex >> 16) & 0xFF) | EOA_SM);    // PA16 - PA17
    }

    if(pSM->nPageSize != 512)
        outpw(REG_SMCMD, 0x30);         // READ 2nd cycle command

    if (!fmiSMCheckRB(pSM))
        return -1;

    if(pSM->nPageSize == 512)
    {
        data = inpw(REG_SMDATA) & 0xff;
        data517 = inpw(REG_SMDATA);
        data517 = inpw(REG_SMDATA);
        data517 = inpw(REG_SMDATA);
        data517 = inpw(REG_SMDATA);
        data517 = inpw(REG_SMDATA) & 0xff;
        if ((data == 0xFF) && (data517 == 0xFF))
            *pu8Status = 0xFF;
        else
            *pu8Status = 0xF0;
    }
    else
    {
        *pu8Status = inpb(REG_SMDATA);
    }
    return 0;
}

/* Read One Page */
int fmiSM_Read(FMI_SM_INFO_T *pSM, UINT32 uPageIndex,UINT32 uDAddr) //Added
{
    volatile UINT32 uStatus, uErrorCnt;
    volatile UINT32 uF1_status, uF2_status;
    volatile UINT8 ii, jj,EccLoop;

    outpw(REG_FMICR, inpw(REG_FMICR)|FMI_SM_EN);

    fmiSM_Reset(pSM);

    outpw(REG_SMCSR, inpw(REG_SMCSR) & (~SMCR_PSIZE));

    switch(pSM->nPageSize)
    {
        case 512:
            outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~(SMCR_BCH_TSEL|SMCR_ECC_EN)) | (BCH_T4 | SMCR_ECC_EN | SMCR_ECC_3B_PROTECT | PSIZE_512));
            break;
        case 2048:
            if(gbSystemImage)
                outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~(SMCR_BCH_TSEL|SMCR_ECC_EN)) | (BCH_T4 | SMCR_ECC_EN | SMCR_ECC_3B_PROTECT | PSIZE_2K));
            else
                outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~(SMCR_BCH_TSEL|SMCR_ECC_EN)) | (BCH_T8 | SMCR_ECC_EN | SMCR_ECC_3B_PROTECT | PSIZE_2K));

            break;
        case 4096:
            outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~(SMCR_BCH_TSEL|SMCR_ECC_EN)) | (BCH_T8 | SMCR_ECC_EN | SMCR_ECC_3B_PROTECT | PSIZE_4K));
            break;

        case 8192:
            outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~(SMCR_BCH_TSEL|SMCR_ECC_EN)) | (BCH_T12 | SMCR_ECC_EN | SMCR_ECC_3B_PROTECT|PSIZE_8K));
            break;
    }

    outpw(REG_SMREAREA_CTL, g_u32ExtraDataSize);

    outpw(REG_SMCSR, inpw(REG_SMCSR) & ~SMCR_REDUN_REN);

    /* enable DMAC */
    while(inpw(REG_DMACCSR) & FMI_BUSY);    // wait DMAC FMI ready;
    outpw(REG_DMACCSR, inpw(REG_DMACCSR) | DMAC_EN);


    /* Set DMA Transfer Starting Address */
    outpw(REG_DMACSAR, uDAddr);


    outpw(REG_SMCMD, 0x00);     // READ 1st cycle command

    outpw(REG_SMADDR, 0);

    if(pSM->nPageSize != 512)
        outpw(REG_SMADDR, 0);

    outpw(REG_SMADDR, uPageIndex & 0xFF);

    if (!pSM->bIsMulticycle)
    {
        outpw(REG_SMADDR, ((uPageIndex >> 8) & 0xFF) | EOA_SM);     // PA8 - PA15
    }
    else
    {
        outpw(REG_SMADDR, (uPageIndex >> 8) & 0xFF);                // PA8 - PA15
        outpw(REG_SMADDR, ((uPageIndex >> 16) & 0xFF) | EOA_SM);    // PA16 - PA17
    }

    if(pSM->nPageSize != 512)
        outpw(REG_SMCMD, 0x30);     // READ 2nd cycle command

    if (!fmiSMCheckRB(pSM))
        return -1;

    uF1_status = 0;
    uF2_status = 0;
    uStatus = 0;
    /* begin DMA read transfer */
    outpw(REG_SMISR, SMISR_DMA_IF); // clear DMA flag
    outpw(REG_SMISR, inpw(REG_SMISR)| SMISR_DMA_IF| SMISR_ECC_FIELD_IF);    // clear DMA flag
    outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_DRD_EN);

    while(1)
    {
        if (inpw(REG_SMISR) & SMISR_ECC_FIELD_IF)
        {
            switch(pSM->nPageSize)
            {
                case 512:
                {
                    uF1_status = inpw(REG_SM_ECC_ST0);
                    uF1_status &= 0x3f;
                    if ((uF1_status & 0x03)==0x01)  // correctable error in 1st field
                    {
                        uErrorCnt = uF1_status >> 2;
                        nvtSM_CorrectData_BCH(pSM->nPageSize, 1, uErrorCnt, (UINT8*)uDAddr);
                    }
                    else if (((uF1_status & 0x03)==0x02)
                          ||((uF1_status & 0x03)==0x03)) // uncorrectable error or ECC error in 1st field
                    {
                      uStatus = 1;
                      PRINTF("Page %d - Uncorrectable Error 0x%X\n",uPageIndex, inpw(REG_SM_ECC_ST0));
                    }
                    break;
                }
                case 2048:
                {
                    uF1_status = inpw(REG_SM_ECC_ST0);
                    for (ii=1; ii<5; ii++)
                    {
                        if ((uF1_status & 0x03)==0x01)  // correctable error in 1st field
                        {
                            uErrorCnt = uF1_status >> 2;
                            nvtSM_CorrectData_BCH(pSM->nPageSize, ii, uErrorCnt, (UINT8*)uDAddr);
                            break;
                        }
                        else if (((uF1_status & 0x03)==0x02)
                              ||((uF1_status & 0x03)==0x03)) // uncorrectable error or ECC error in 1st field
                        {
                            uStatus = 1;
                            PRINTF("Page %d - Uncorrectable Error 0x%X\n",uPageIndex,inpw(REG_SM_ECC_ST0));
                            break;
                        }
                        uF1_status >>= 8;
                    }

                    break;
                }
                case 4096:
                case 8192:
                {
                    if(pSM->nPageSize == 4096)
                        EccLoop = 2;
                    else
                        EccLoop = 4;

                    for (jj=0; jj<EccLoop; jj++)
                    {
                        uF1_status = inpw(REG_SM_ECC_ST0+jj*4);
                        for (ii=1; ii<5; ii++)
                        {
                            if ((uF1_status & 0x03)==0x01)  // correctable error in 1st field
                            {
                                uErrorCnt = uF1_status >> 2;
                                nvtSM_CorrectData_BCH(pSM->nPageSize, ii+jj*4, uErrorCnt, (UINT8*)uDAddr);
                                break;
                            }
                            else if (((uF1_status & 0x03)==0x02)
                                  ||((uF1_status & 0x03)==0x03)) // uncorrectable error or ECC error in 1st field
                            {
                                uStatus = 1;
                                PRINTF("Page %d - Uncorrectable Error 0x%X\n",uPageIndex, inpw(REG_SM_ECC_ST0));
                                break;
                            }
                            uF1_status >>= 8;
                        }
                    }
                }
                break;
            }

            outpw(REG_SMISR, SMISR_ECC_FIELD_IF);       // clear ECC_FLD_Error
        }

        if (inpw(REG_SMISR) & SMISR_DMA_IF)      // wait to finish DMAC transfer.
        {
            if ( !(inpw(REG_SMISR) & SMISR_ECC_FIELD_IF) )
                break;
        }
    }

    outpw(REG_SMISR, SMISR_DMA_IF);         // clear DMA flag

    outpw(REG_SMISR, SMISR_ECC_FIELD_IF);   // clear ECC flag

    if (uStatus)
        return -1;

    return 0;
}

/* function pointer */
BOOL volatile bIsNandInit = FALSE;
FMI_SM_INFO_T nvtSMInfo, *pNvtSM0, *pNvtSMInfo;

#ifdef __Bad_Block_Check__
extern volatile BOOL _fmi_bIsNandFirstAccess;
extern INT sicSMChangeBadBlockMark(INT chipSel);

INT fmiSMCheckHeader(FMI_SM_INFO_T* pSM)
{
    int i, status;

    pInfo = (UINT8 *)((UINT32)infoBuf | 0x80000000);

    /* check specific mark */
    if (pSM->nPageSize != NAND_PAGE_512B)
    {
        /* read physical block 0 - image information */
        if ((status = nvtSMpread(0, pSM0->uPagePerBlock-1, pInfo)) < 0)
            return -1;

        if ((*(pInfo + pSM->nPageSize - 6) != '5') && (*(pInfo + pSM->nPageSize - 5) != '5') &&
            (*(pInfo + pSM->nPageSize - 4) != '0') && (*(pInfo + pSM->nPageSize - 3) != '0') &&
            (*(pInfo + pSM->nPageSize - 2) != '9') && (*(pInfo + pSM->nPageSize - 1) != '1'))
        {
            if ((status = sicSMChangeBadBlockMark(0)) < 0)
                return -1;
            _fmi_bIsNandFirstAccess = FALSE;
        }
        else
        {
            _fmi_bIsNandFirstAccess = FALSE;
        }
    }
    return 0;
}
#endif

/* 512 Page - Read Spare area */
#if 0
INT fmiSM2BufferM_RA(FMI_SM_INFO_T *pSM, UINT32 uSector, UINT8 ucColAddr)   //Added
{
    /* clear R/B flag */
    while(!(inpw(REG_SMISR) & 0x40000));
    outpw(REG_SMISR, 0x400);

    outpw(REG_SMCMD, 0x50);     /* Read C area (Spare area) */
    outpw(REG_SMADDR, ucColAddr);       // CA0 - CA7
    outpw(REG_SMADDR, uSector & 0xff);  // PA0 - PA7
    if (!pSM->bIsMulticycle)
        outpw(REG_SMADDR, ((uSector >> 8) & 0xff)|0x80000000);      // PA8 - PA15
    else
    {
        outpw(REG_SMADDR, (uSector >> 8) & 0xff);                   // PA8 - PA15
        outpw(REG_SMADDR, ((uSector >> 16) & 0xff)|0x80000000);     // PA16 - PA17
    }

    if (!fmiSMCheckRB(pSM))
        return -1;

    return 0;
}
#endif

#if 1
/* Check if Bad Block */
INT fmiNormalCheckBlock(FMI_SM_INFO_T *pSM, UINT32 BlockNo)  //Modified
{
    int pageNo;
    UINT8 u8Status;

    // enable SM
    outpw(REG_FMICR, FMI_SM_EN);

#if 1
    if (pSM->bIsMLCNand == TRUE)
        pageNo = (BlockNo+1) * pSM->uPagePerBlock - 1;
    else
        pageNo = BlockNo * pSM->uPagePerBlock;
#else
    pageNo = BlockNo * pSM->uPagePerBlock;
#endif

    if (fmiSM_ReadBlockStatus(pSM, pageNo,&u8Status))
        return 1;   // No response


    if (u8Status == 0xFF)
    {
        fmiSM_ReadBlockStatus(pSM, pageNo+1,&u8Status);
        if (u8Status != 0xFF)
        {
            PRINTF("fmiNormalCheckBlock Bad Block %d - STATUS 0x%X\n", BlockNo, u8Status);
            return 1;   // invalid block
        }
    }
    else
    {
        PRINTF("fmiNormalCheckBlock Bad Block %d - STATUS 0x%X\n", BlockNo, u8Status);
        return 1;   // invalid block
    }
    return 0;
}
#endif

INT nvtSMInit(void)     // Modified
{
#if 0
    if (!bIsNandInit)
    {
        // Enable SD Card Host Controller operation and driving clock.
        outpw(REG_AHBCLK, inpw(REG_AHBCLK) | (NAND_CKE|SIC_CKE|HCLK3_CKE));

        outpw(0xB000008C, inpw(0xB000008C) | 0x3FC00);      // enable NAND NWR/NRD/BR0/BR1 pins
        outpw(0xB0000090, inpw(0xB0000090) | 0x00FF0000);   // enable NAND CS0/CS1 pins

        /* SM Enable */
        outpw(REG_FMICR, FMI_SM_EN);

        /* reset DMAC */
        outpw(REG_DMACCSR, inpw(REG_DMACCSR) | DMAC_SWRST);

        /* FMI software reset */
        outpw(REG_FMICR, inpw(REG_FMICR) | FMI_SWRST);
        sysDelay(0x1);
        outpw(REG_FMICR, (inpw(REG_FMICR) & (~FMI_SWRST)));

        /* SM software reset */
        outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_SM_SWRST);

        /* set page size = 512B (default) */
        outpw(REG_SMCSR, inpw(REG_SMCSR) & (~SMCR_PSIZE));          // 0: 512-byte, 1: 2048-byte

        /* Disable Auto-Write */
        outpw(REG_SMCSR, inpw(REG_SMCSR) & (~SMCR_REDUN_AUTO_WEN)); // 0: 512-byte, 1: 2048-byte

        /* SM chip select */
        outpw(REG_SMCSR, inpw(REG_SMCSR) & ~ SMCR_CS0);             // CS0 pin low
        outpw(REG_SMCSR, inpw(REG_SMCSR) |   SMCR_CS1);             // CS1 pin high

        /* clear Interrupt Status Flag */
        outpw(REG_SMISR, 0xFFFF);

        /* SM Interrupt setting */
        outpw(REG_SMIER, 0x00);

        /* set CLE/ALE setup and hold time width */
        outpw(REG_SMTCR, 0x00010204);          // CLE/ALE=0x01, R/W_h=0x02, R/W_l=0x04

        pNvtSM0 = (FMI_SM_INFO_T *)malloc(sizeof(NVT_SM_INFO_T));
        memset((char *)pNvtSMInfo, 0, sizeof(NVT_SM_INFO_T));
        pSM0= pNvtSM0;

        if (nvtSM_ReadID(pSM0) < 0)
            return -1;
        nvtSM_Initial(pSM0);

        bIsNandInit = TRUE;
    }

    return 0;
#else
    if (!bIsNandInit)
    {

        outpw(REG_DMACCSR, inpw(REG_DMACCSR) | DMAC_EN);
        // Reset DMAC
        outpw(REG_DMACCSR, inpw(REG_DMACCSR) | DMAC_SWRST);
        outpw(REG_DMACCSR, inpw(REG_DMACCSR) & ~DMAC_SWRST);
        // SM Enable
        outpw(REG_FMICR, FMI_SM_EN);

        /* set CLE/ALE setup and hold time width */
        outpw(REG_SMTCR, 0x20305);   // CLE/ALE=0x02, R/W_h=0x03, R/W_l=0x05
        outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~SMCR_PSIZE) | PSIZE_512);
        outpw(REG_SMCSR, inpw(REG_SMCSR) |  SMCR_ECC_3B_PROTECT);
        outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_ECC_CHK);

        /* init SM interface */
        outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_REDUN_AUTO_WEN);

        outpw(REG_GPDFUN, inpw(REG_GPDFUN) | 0x0003CC00);       // enable NAND NWR/NRD/RB0 pins
        outpw(REG_GPEFUN, inpw(REG_GPEFUN) | 0x00F30000);       // enable NAND ALE/CLE/CS0 pins

        outpw(REG_SMCSR, inpw(REG_SMCSR) & ~SMCR_CS0);
        outpw(REG_SMCSR, inpw(REG_SMCSR) |  SMCR_CS1);

        pNvtSMInfo = (FMI_SM_INFO_T *)malloc(sizeof(NVT_SM_INFO_T));
        memset((char *)pNvtSMInfo, 0, sizeof(NVT_SM_INFO_T));
        pSM0= pNvtSMInfo;
        pNvtSM0 = pSM0;

        if (nvtSM_ReadID(pNvtSM0) < 0)
            return -1;
        nvtSM_Initial(pNvtSM0);

        bIsNandInit = TRUE;
    }

    return 0;
#endif
}


INT nvtSMpread(INT PBA, INT page, UINT8 *buff)  //Modified
{
    FMI_SM_INFO_T *pSM;
    int pageNo;

    pSM = pNvtSM0;

    // enable SM
    outpw(REG_FMICR, FMI_SM_EN);
    if(PBA < 4)
        gbSystemImage = TRUE;
    else
        gbSystemImage = FALSE;

    pageNo = PBA * pSM->uPagePerBlock + page;

    nvtSM_ReadOnePage_ExtraData(pSM, pageNo);

    return (fmiSM_Read(pSM, pageNo, (UINT32)buff));
}


INT nvtSMpwrite(INT PBA, INT page, UINT8 *buff) // Modified
{
    FMI_SM_INFO_T *pSM;
    int pageNo;

    pSM = pNvtSM0;

    // enable SM
    outpw(REG_FMICR, 0x08);

    if (PBA > 3)
        nIsSysImage = 0xFF;     // NOT System Image (Loader), write 0xFFFF0000 to Redundancy Area
    else
    {
        if (page == 0)          // Reset Sequence Mark (0xFF5Axx00) to 0 in Redundancy Area
            gMarkCount = 0;
        else if (page > pSM->uPagePerBlock)
        {
            gMarkCount = 0;
            nIsSysImage = 0xFF;
        }
    }

    if(PBA < 4)
        gbSystemImage = TRUE;
    else
        gbSystemImage = FALSE;

    pageNo = PBA * pSM->uPagePerBlock + page;

    if (pSM->nPageSize == NAND_PAGE_2KB)        /* 2KB */
        return (nvtSM_Write_2K(pSM, pageNo, 0, (UINT32)buff));
    else if (pSM->nPageSize == NAND_PAGE_4KB)   /* 4KB */
        return (nvtSM_Write_4K(pSM, pageNo, 0, (UINT32)buff));
    else if (pSM->nPageSize == NAND_PAGE_8KB)   /* 8KB */
        return (nvtSM_Write_8K(pSM, pageNo, 0, (UINT32)buff));
    else    /* 512B */
        return (nvtSM_Write_512(pSM, pageNo, 0, (UINT32)buff));
}

#ifdef OPT_MARK_BAD_BLOCK_WHILE_ERASE_FAIL

INT nvtSMMarkBadBlock_WhileEraseFail(FMI_SM_INFO_T *pSM, UINT32 BlockNo)
{
    UINT32 uSector, ucColAddr;

    /* check if MLC NAND */
    if (pSM->bIsMLCNand == TRUE)
    {
        uSector = (BlockNo+1) * pSM->uPagePerBlock - 1; // write last page
        if (pSM->nPageSize == NAND_PAGE_2KB)
        {
            ucColAddr = 2048;       // write 2048th byte
        }
        else if (pSM->nPageSize == NAND_PAGE_4KB)
        {
            ucColAddr = 4096;       // write 4096th byte
        }

        // send command
        outpw(REG_SMCMD, 0x80);     // serial data input command
        outpw(REG_SMADDR, ucColAddr);               // CA0 - CA7
        outpw(REG_SMADDR, (ucColAddr >> 8) & 0xff); // CA8 - CA11
        outpw(REG_SMADDR, uSector & 0xff);          // PA0 - PA7
        if (!pSM->bIsMulticycle)
            outpw(REG_SMADDR, ((uSector >> 8) & 0xff)|EOA_SM);  // PA8 - PA15
        else
        {
            outpw(REG_SMADDR, (uSector >> 8) & 0xff);           // PA8 - PA15
            outpw(REG_SMADDR, ((uSector >> 16) & 0xff)|EOA_SM); // PA16 - PA17
        }
        outpw(REG_SMDATA, 0xf0);    // mark bad block (use 0xf0 instead of 0x00 to differ from Old (Factory) Bad Blcok Mark)
        outpw(REG_SMCMD, 0x10);

        if (! fmiSMCheckRB(pSM))
            return FMI_SM_RB_ERR;

        fmiSM_Reset(pSM);
        return 0;
    }
    /* SLC check the 2048 byte of 1st or 2nd page per block */
    else    // SLC
    {
        uSector = BlockNo * pSM->uPagePerBlock;     // write lst page
        if (pSM->nPageSize == NAND_PAGE_2KB)
        {
            ucColAddr = 2048;       // write 2048th byte
        }
        else if (pSM->nPageSize == NAND_PAGE_4KB)
        {
            ucColAddr = 4096;       // write 4096th byte
        }
        else if (pSM->nPageSize == NAND_PAGE_512B)
        {
            ucColAddr = 0;          // write 4096th byte
            goto _mark_512;
        }

        // send command
        outpw(REG_SMCMD, 0x80);     // serial data input command
        outpw(REG_SMADDR, ucColAddr);               // CA0 - CA7
        outpw(REG_SMADDR, (ucColAddr >> 8) & 0xff); // CA8 - CA11
        outpw(REG_SMADDR, uSector & 0xff);          // PA0 - PA7
        if (!pSM->bIsMulticycle)
            outpw(REG_SMADDR, ((uSector >> 8) & 0xff)|EOA_SM);  // PA8 - PA15
        else
        {
            outpw(REG_SMADDR, (uSector >> 8) & 0xff);           // PA8 - PA15
            outpw(REG_SMADDR, ((uSector >> 16) & 0xff)|EOA_SM); // PA16 - PA17
        }
        outpw(REG_SMDATA, 0xf0);    // mark bad block (use 0xf0 instead of 0x00 to differ from Old (Factory) Bad Blcok Mark)
        outpw(REG_SMCMD, 0x10);

        if (! fmiSMCheckRB(pSM))
            return FMI_SM_RB_ERR;

        fmiSM_Reset(pSM);
        return 0;

_mark_512:

        outpw(REG_SMCMD, 0x50);     // point to redundant area
        outpw(REG_SMCMD, 0x80);     // serial data input command
        outpw(REG_SMADDR, ucColAddr);       // CA0 - CA7
        outpw(REG_SMADDR, uSector & 0xff);  // PA0 - PA7
        if (!pSM->bIsMulticycle)
            outpw(REG_SMADDR, ((uSector >> 8) & 0xff)|EOA_SM);  // PA8 - PA15
        else
        {
            outpw(REG_SMADDR, (uSector >> 8) & 0xff);           // PA8 - PA15
            outpw(REG_SMADDR, ((uSector >> 16) & 0xff)|EOA_SM); // PA16 - PA17
        }

        outpw(REG_SMDATA, 0xf0);    // 512
        outpw(REG_SMDATA, 0xff);
        outpw(REG_SMDATA, 0xff);
        outpw(REG_SMDATA, 0xff);
        outpw(REG_SMDATA, 0xf0);    // 516
        outpw(REG_SMDATA, 0xf0);    // 517
        outpw(REG_SMCMD, 0x10);
        if (! fmiSMCheckRB(pSM))
            return FMI_SM_RB_ERR;

        fmiSM_Reset(pSM);
        return 0;
    }
}

#endif      // OPT_MARK_BAD_BLOCK_WHILE_ERASE_FAIL

INT nvtSMblock_erase(INT PBA)   //Modified
{
    FMI_SM_INFO_T *pSM;
    UINT32 page_no;

    pSM = pNvtSM0;

    // enable SM
    outpw(REG_FMICR, FMI_SM_EN);

    // Check Bad Block Status
    if(fmiNormalCheckBlock(pSM, PBA) != 1)
    {
        page_no = PBA * pSM->uPagePerBlock;     // get page address

        // clear R/B flag
        while(!(inpw(REG_SMISR) & 0x40000));
        outpw(REG_SMISR, 0x400);

        outpw(REG_SMCMD, 0x60);     // erase setup command

        outpw(REG_SMADDR, (page_no & 0xff));        // PA0 - PA7
        if (!pSM->bIsMulticycle)
            outpw(REG_SMADDR, ((page_no >> 8) & 0xff)|0x80000000);  // PA8 - PA15
        else
        {
            outpw(REG_SMADDR, ((page_no >> 8) & 0xff));             // PA8 - PA15
            outpw(REG_SMADDR, ((page_no >> 16) & 0xff)|0x80000000); // PA16 - PA17
        }

        outpw(REG_SMCMD, 0xd0);     // erase command

        if (!fmiSMCheckRB(pSM))
            return -1;

        if (fmiSMCheckStatus(pSM) != 0)
        {
            PRINTF("nvtSMblock_erase: erase block fail on block [%d]!!\n", PBA);

#ifdef OPT_MARK_BAD_BLOCK_WHILE_ERASE_FAIL
            nvtSMMarkBadBlock_WhileEraseFail(pSM,PBA);
#endif
            return -1;
        }
    }
    else
    {
        sysprintf("nvtSMblock_erase(): block %d is bad\n", PBA);
        return -1;
    }
    return 0;
}


INT nvtSMchip_erase(UINT32 startBlcok, UINT32 endBlock) //Modified
{
    int i, status=0;
    //FMI_SM_INFO_T *pSM;
    pInfo = (UINT8 *)((UINT32)infoBuf | 0x80000000);

    //pSM = pNvtSM0;

    // enable SM
    outpw(REG_FMICR, FMI_SM_EN);

    // erase all chip
    for (i=startBlcok; i<=endBlock; i++)
    {
        status = nvtSMblock_erase(i);
        if (status < 0)
            PRINTF("SM block erase fail <%d>!!\n", i);
    }
    return 0;
}

INT nvtBackupNandBlock0()   // OK
{
    unsigned char *pBackup;
    int volatile i;

    /* use 15MB SDRAM address to backup NAND block 0 */
    pBackup = (UINT8 *)(NAND_BACKUP_BASE);

    for (i=0; i<pNvtSM0->uPagePerBlock; i++)
    {
        nvtSMpread(0, i, pBackup);
        pBackup += pNvtSM0->nPageSize;
    }
    return 0;
}

INT nvtBackupNandBlockBackup(int block)/* Test Only */
{
    unsigned char *pBackup;
    int volatile i;

    pBackup = (UINT8 *)((UINT32)gNandBackupAddressTmp | 0x80000000);

    for (i=0; i<pSM0->uPagePerBlock; i++)
    {
        if (nandpread0(block, i, pBackup) < 0)
        {
            PRINTF("Backup Block Fail - page %d\n", i);
            return -1;
        }
        pBackup += pSM0->nPageSize;
    }
    return 0;
}

INT nvtRestoreNandBlock0()  //OK
{
    unsigned char *pBackup;
    int volatile i;

    /* use 15MB SDRAM address to backup NAND block 0 */
    pBackup = (UINT8 *)(NAND_BACKUP_BASE);

    for (i=0; i<pNvtSM0->uPagePerBlock; i++)
    {
        if (nvtSMpwrite(0, i, pBackup) <0)
            return -1;
        pBackup += pNvtSM0->nPageSize;
    }
    return 0;
}

/* Restore entire block 0 from gNandBackupAddress */
INT nvtRestoreNandBlockBackup(void) //OK
{
    unsigned char *pBackup;
    int volatile i,j;

    for(j=1;j<4;j++)
    {
        pBackup = (UINT8 *)(NAND_BACKUP_BASE);
        nvtSMblock_erase(j);
        for (i=0; i<pSM0->uPagePerBlock; i++)
        {
            if (nvtSMpwrite(j, i, pBackup) < 0)
            {
                PRINTF("Backup Write Fail\n");
                return -1;
            }
            pBackup += pSM0->nPageSize;
        }
    }
    return 0;
}

INT ChangeNandImageType(UINT32 imageNo, UINT32 imageType)   //OK
{
    int i, count;
    unsigned int volatile uPage;
    unsigned int *ptr;

    pInfo = (UINT8 *)((UINT32)infoBuf | 0x80000000);
    ptr = (unsigned int *)((UINT32)infoBuf | 0x80000000);

    uPage = pNvtSM0->uPagePerBlock - 2;

    if (nvtBackupNandBlock0() <0)
        return -1;

    /* search the image */
    if (nvtSMpread(0, uPage, pInfo) <0)
        return -1;

    if (((*(ptr+0)) == 0x574255AA) && ((*(ptr+3)) == 0x57425963))
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

    memcpy((char *)(NAND_BACKUP_BASE+uPage*pNvtSM0->nPageSize), pInfo, pNvtSM0->nPageSize);

    /* write back to block 0 */
    nvtSMblock_erase(0);

    nIsSysImage = 0x005A;
    gMarkCount = 0;

    if (nvtRestoreNandBlock0() <0)
        return -1;

    PRINTF("Restore Block 0 to Block 1~3\n");
    nvtRestoreNandBlockBackup();

    return Successful;
}

INT DelNandImage(UINT32 imageNo)
{
    int i, count;
    unsigned int startBlock, endBlock;
    unsigned int volatile uPage;
    unsigned int *ptr;

    pInfo = (UINT8 *)((UINT32)infoBuf | 0x80000000);
    ptr = (unsigned int *)((UINT32)infoBuf | 0x80000000);

    uPage = pNvtSM0->uPagePerBlock - 2;

    if (nvtBackupNandBlock0() <0)
        return -1;

    /* search the image */
    if (nvtSMpread(0, uPage, pInfo) < 0)
        return -1;

    if (((*(ptr+0)) == 0x574255AA) && ((*(ptr+3)) == 0x57425963))
    {
        count = *(ptr+1);
        *(ptr+1) = count - 1;   // del one image

        /* pointer to image information */
        ptr += 4;
        for (i=0; i<count; i++)
        {
            if ((*(ptr) & 0xffff) == imageNo)
            {
                startBlock = *(ptr + 1) & 0xffff;
                endBlock = (*(ptr + 1) & 0xffff0000) >> 16;
                break;
            }
            /* pointer to next image */
            ptr = ptr+12;
        }
        memcpy((char *)ptr, (char *)(ptr+12), (count-i-1)*48);
    }

    memcpy((char *)(NAND_BACKUP_BASE+uPage*pNvtSM0->nPageSize), pInfo, pNvtSM0->nPageSize);

    /* write back to block 0 */
    nvtSMblock_erase(0);

    nIsSysImage = 0x005A;
    gMarkCount = 0;

    /* If the image to delete is system image */
    /* Set the first 16K or 32K data in gNandBackupAddress buffer as 0xFF (system image) */
    if (startBlock == 0)
    {
        if (pSM0->nPageSize == NAND_PAGE_512B)
            memset((char *)(NAND_BACKUP_BASE), 0xFF, 16*1024);
        else
            memset((char *)(NAND_BACKUP_BASE), 0xFF, 32*1024);
    }

    if (nvtRestoreNandBlock0() <0)
        return -1;

    PRINTF("Restore Block 0 to Block 1~3\n");
    nvtRestoreNandBlockBackup();

    // erase the block
    if (startBlock != 0)
    {
        for (i=startBlock; i<=(endBlock-startBlock); i++)
            nvtSMblock_erase(i);
    }
    return Successful;
}

INT fmiMarkBadBlock(UINT32 block)
{
    UINT32 ucColAddr, uSector;

    uSector = block * pNvtSM0->uPagePerBlock;
    if (pNvtSM0->nPageSize != NAND_PAGE_512B)
    {
        // send command
        if (pNvtSM0->nPageSize == NAND_PAGE_2KB)
            ucColAddr = 2048;
        else if (pNvtSM0->nPageSize == NAND_PAGE_4KB)
            ucColAddr = 4096;
        else if (pNvtSM0->nPageSize == NAND_PAGE_8KB)
            ucColAddr = 8192;
        else
            return -1;

        outpw(REG_SMCMD, 0x80);     // serial data input command
        outpw(REG_SMADDR, ucColAddr);       // CA0 - CA7
        outpw(REG_SMADDR, (ucColAddr >> 8) & 0xff);
        outpw(REG_SMADDR, uSector & 0xff);  // PA0 - PA7
        if (!pNvtSM0->bIsMulticycle)
            outpw(REG_SMADDR, ((uSector >> 8) & 0xff)|0x80000000);  // PA8 - PA15
        else
        {
            outpw(REG_SMADDR, (uSector >> 8) & 0xff);               // PA8 - PA15
            outpw(REG_SMADDR, ((uSector >> 16) & 0xff)|0x80000000); // PA16 - PA17
        }
    }
    else
    {
        ucColAddr = 0;
        outpw(REG_SMCMD, 0x50);     // read RA command
        outpw(REG_SMCMD, 0x80);     // serial data input command
        outpw(REG_SMADDR, ucColAddr);       // CA0 - CA7
        outpw(REG_SMADDR, uSector & 0xff);  // PA0 - PA7
        if (!pNvtSM0->bIsMulticycle)
            outpw(REG_SMADDR, ((uSector >> 8) & 0xff)|0x80000000);  // PA8 - PA15
        else
        {
            outpw(REG_SMADDR, (uSector >> 8) & 0xff);               // PA8 - PA15
            outpw(REG_SMADDR, ((uSector >> 16) & 0xff)|0x80000000); // PA16 - PA17
        }
    }

    /* mark data */
    outpw(REG_SMDATA, 0x00);
    outpw(REG_SMDATA, 0x00);
    outpw(REG_SMDATA, 0x00);
    outpw(REG_SMDATA, 0x00);

    outpw(REG_SMCMD, 0x10);     // auto program command

    if (!fmiSMCheckRB(pSM0))
        return Fail;

    if (fmiSM_Reset(pSM0) < 0)
        return -1;

    return Successful;
}

/* TRUE => match; FALSE => new image */
BOOL bIsMatch = FALSE;
UINT32 *pUpdateImage=0;
INT searchImageInfo(FW_UPDATE_INFO_T *pFW, UINT32 *ptr)
{
    int volatile i;
    UINT32 bmark, emark, totalCnt=0;
    UINT32 *pImage;

    /* image start pint */
    pImage = ptr+4;

    /* check mark */
    bmark = *ptr;
    emark = *(ptr+3);

    if ((bmark == 0x574255AA) || (emark == 0x57425963))
    {
        totalCnt = *(ptr+1);
        for (i=0; i<totalCnt; i++)
        {
            if ((*pImage & 0xffff) == pFW->imageNo)
            {
                pUpdateImage = pImage;
                bIsMatch = TRUE;
                break;
            }
            else
            {
                pImage += 12;
                bIsMatch = FALSE;
            }
        }
    }

    if (!bIsMatch)
    {
        pUpdateImage = (ptr+4) + (totalCnt * 12);
    }

    return totalCnt;
}

/* write image information into physical block 0 last 2 page */
INT nvtSetNandImageInfo(FW_UPDATE_INFO_T *pFW)
{
    unsigned int volatile i, uPage;
    unsigned int *ptr;

    pInfo = (UINT8 *)((UINT32)infoBuf | 0x80000000);
    ptr = (unsigned int *)((UINT32)infoBuf | 0x80000000);

//printf("\nNo[%d], Flag[%d], start[%d], end[%d], len[%d], exec[0x%x], name[%s]\n\n",
//  pFW->imageNo, pFW->imageFlag, pFW->startBlock, pFW->endBlock, pFW->fileLen, pFW->executeAddr, pFW->imageName);

    uPage = pNvtSM0->uPagePerBlock - 2;

    if (nvtBackupNandBlock0() < 0)
        return -1;

    /* search the image */
    if (nvtSMpread(0, uPage, pInfo) <0)
        return -1;

    i = searchImageInfo(pFW, ptr);  // return image total count

    /* update image information */
    *(ptr+0) = 0x574255AA;
    if (!bIsMatch)
        *(ptr+1) = i+1;
    *(ptr+3) = 0x57425963;
    bIsMatch = FALSE;

    *(pUpdateImage+0) = (pFW->imageNo & 0xffff) | ((pFW->imageFlag & 0xffff) << 16);    // image number / flag
    *(pUpdateImage+1) = (pFW->startBlock&0xffff) | ((pFW->endBlock&0xffff) << 16);
    *(pUpdateImage+2) = pFW->executeAddr;
    *(pUpdateImage+3) = pFW->fileLen;
    memcpy((char *)(pUpdateImage+4), pFW->imageName, 32);   // image name

    memcpy((char *)(NAND_BACKUP_BASE+uPage*pNvtSM0->nPageSize), pInfo, pNvtSM0->nPageSize);

    /* write back to block 0 */
    nvtSMblock_erase(0);

    nIsSysImage = 0x005A;
    gMarkCount = 0;

    if (nvtRestoreNandBlock0() < 0)
        return -1;
    PRINTF("Restore Block 0 to Block 1~3\n");
    if (nvtRestoreNandBlockBackup() < 0)
        return -1;

    {
        int x;
        for(x=1;x<4;x++)
        {
            nvtBackupNandBlockBackup(x);
            if(memcmp((UINT8*)gNandBackupAddressTmp, (UINT8*)gNandBackupAddress, (pSM0->nPageSize * pSM0->uPagePerBlock))!= 0)
                PRINTF("Backup error block %d\n",x);
        }
    }

    return Successful;
}


INT nvtGetNandImageInfo(unsigned int *image)
{
    UINT32 volatile bmark, emark;
    int volatile i, imageCount=0;
    unsigned int *ptr;

    pInfo = (UINT8 *)((UINT32)infoBuf | 0x80000000);
    ptr = (unsigned int *)((UINT32)infoBuf | 0x80000000);

    if (nvtSMpread(0, pNvtSM0->uPagePerBlock-2, pInfo) <0)
    {
        PRINTF("Read Image information ERROR\n");
        return -1;
    }

    bmark = *(ptr+0);
    emark = *(ptr+3);

    if ((bmark == 0x574255AA) && (emark == 0x57425963))
    {
        imageCount = *(ptr+1);

        /* pointer to image information */
        ptr = ptr+4;
        for (i=0; i<imageCount; i++)
        {
            /* fill into the image list buffer */
            *image = 0;     // action flag, dummy
            *(image+1) = 0; // file len, dummy
            *(image+2) = *(ptr) & 0xffff;                   // imageNo
            memcpy((char *)(image+3), (char *)(ptr+4), 32); // image name
            *(image+11) = (*(ptr) >> 16) & 0xffff;          // image flag
            *(image+12) = (*(ptr+1) >> 16) & 0xffff;        // end block
            *(image+13) = *(ptr+1) & 0xffff;                // start block
            *(image+14) = 0;

            PRINTF("    Image No: %d\n", *(image+2));
            PRINTF("    Image Start Block: %d\n", *(image+13));
            PRINTF("    Image End Block: %d\n", *(image+12));
            /* pointer to next image */
            image += 15;
            ptr = ptr+12;
        }
    }
    else
        imageCount = 0;

    return imageCount;
}

/* Check the 1th byte of spare */
INT CheckBadBlockMark(UINT32 block)
{
    pInfo = (UINT8 *)((UINT32)infoBuf | 0x80000000);

    if (nvtSMpread(block, 0, pInfo) < 0)
        return -1;

    if (inpw(REG_SMRA_0) == 0x0)
        return Fail;
    else
        return Successful;
}


/* set the partition information in block 0 last page */
INT setNandPartition(INT sysArea)
{
    unsigned int volatile uPage;
    unsigned int *ptr;
    unsigned int sysSector=0xFFFFFFFF;

    if (sysArea != 0)
    {
        pInfo = (UINT8 *)((UINT32)infoBuf | 0x80000000);
        ptr = (unsigned int *)((UINT32)infoBuf | 0x80000000);

        uPage = pNvtSM0->uPagePerBlock - 1;
        sysSector = sysArea * 1024 * 2; // sector count

        if (nvtBackupNandBlock0() < 0)
            return -1;

        memset((char *)pInfo, 0xFF, pNvtSM0->nPageSize);
        /* update image information */
        *(ptr+0) = 0x574255AA;
        *(ptr+1) = sysSector;
        *(ptr+2) = 0xffffffff;
        *(ptr+3) = 0x57425963;

        /* Copy information to the final page of buffer (NAND_BACKUP_BASE) */
        memcpy((char *)(NAND_BACKUP_BASE+uPage*pNvtSM0->nPageSize), pInfo, pNvtSM0->nPageSize);

        /* write back to block 0 */
        if (nvtSMblock_erase(0) <0)
            return -1;

        nIsSysImage = 0x005A;

        gMarkCount = 0;

        if (nvtRestoreNandBlock0() <0)
            return -1;
    }
    return Successful;
}

/* Get the System Area Size from the final page of Block 0 and Erase the blocks for System Area */
INT nvtSM_EraseSysArea()
{
    int volatile fmiNandSysArea=0, sysTotalBlock=0;
    int volatile status, i;
    unsigned int *ptr;

    pInfo = (UINT8 *)((UINT32)infoBuf | 0x80000000);
    ptr = (unsigned int *)((UINT32)infoBuf | 0x80000000);

    /* read physical block 0 - image information */
    status = nvtSMpread(0, pSM0->uPagePerBlock-1, pInfo);
    if (status < 0)
        return status;

    if (((*(ptr+0)) == 0x574255aa) && ((*(ptr+3)) == 0x57425963))
    {
        fmiNandSysArea = *(ptr+1);
    }

    if ((fmiNandSysArea != 0xFFFFFFFF) && (fmiNandSysArea != 0))
    {
        sysTotalBlock = (fmiNandSysArea / pSM0->uSectorPerBlock) + 1;
    }

    for (i=0; i<sysTotalBlock; i++)
    {
        status = nvtSMblock_erase(i);
        if (status < 0)
            return status;
    }

    /* write physical block 0 - image information */
    status = nvtSMpwrite(0, pSM0->uPagePerBlock-1, pInfo);
    if (status < 0)
        return status;

    return Successful;
}
