/**************************************************************************//**
 * @file     N9H20_SIC.h
 * @version  V3.00
 * @brief    N9H20 series SIC driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#ifndef _SIC_H_
#define _SIC_H_

//#define OPT_SW_WP
#define OPT_N9H20
#define OPT_MARK_BAD_BLOCK_WHILE_ERASE_FAIL

#define FMI_SD_CARD     0
#define FMI_SM_CARD     1

#define FMI_ERR_ID  0xFFFF0100

#define FMI_TIMEOUT             (FMI_ERR_ID|0x01)
#define FMI_NO_MEMORY           (FMI_ERR_ID|0x02)
/* SD error */
#define FMI_NO_SD_CARD          (FMI_ERR_ID|0x10)
#define FMI_ERR_DEVICE          (FMI_ERR_ID|0x11)
#define FMI_SD_INIT_TIMEOUT     (FMI_ERR_ID|0x12)
#define FMI_SD_SELECT_ERROR     (FMI_ERR_ID|0x13)
#define FMI_SD_WRITE_PROTECT    (FMI_ERR_ID|0x14)
#define FMI_SD_INIT_ERROR       (FMI_ERR_ID|0x15)
#define FMI_SD_CRC7_ERROR       (FMI_ERR_ID|0x16)
#define FMI_SD_CRC16_ERROR      (FMI_ERR_ID|0x17)
#define FMI_SD_CRC_ERROR        (FMI_ERR_ID|0x18)
#define FMI_SD_CMD8_ERROR       (FMI_ERR_ID|0x19)
#define FMI_SD_DITO_ERROR       (FMI_ERR_ID|0x1A)
#define FMI_SD_RITO_ERROR       (FMI_ERR_ID|0x1B)

/* NAND error */
#define FMI_SM_INIT_ERROR       (FMI_ERR_ID|0x20)
#define FMI_SM_RB_ERR           (FMI_ERR_ID|0x21)
#define FMI_SM_STATE_ERROR      (FMI_ERR_ID|0x22)
#define FMI_SM_ECC_ERROR        (FMI_ERR_ID|0x23)
#define FMI_SM_STATUS_ERR       (FMI_ERR_ID|0x24)
#define FMI_SM_ID_ERR           (FMI_ERR_ID|0x25)
#define FMI_SM_INVALID_BLOCK    (FMI_ERR_ID|0x26)
#define FMI_SM_MARK_BAD_BLOCK_ERR   (FMI_ERR_ID|0x27)

/* MS error */
#define FMI_NO_MS_CARD          (FMI_ERR_ID|0x30)
#define FMI_MS_INIT_ERROR       (FMI_ERR_ID|0x31)
#define FMI_MS_INT_TIMEOUT      (FMI_ERR_ID|0x32)
#define FMI_MS_BUSY_TIMEOUT     (FMI_ERR_ID|0x33)
#define FMI_MS_CRC_ERROR        (FMI_ERR_ID|0x34)
#define FMI_MS_INT_CMDNK        (FMI_ERR_ID|0x35)
#define FMI_MS_INT_ERR          (FMI_ERR_ID|0x36)
#define FMI_MS_INT_BREQ         (FMI_ERR_ID|0x37)
#define FMI_MS_INT_CED_ERR      (FMI_ERR_ID|0x38)
#define FMI_MS_READ_PAGE_ERROR  (FMI_ERR_ID|0x39)
#define FMI_MS_COPY_PAGE_ERR    (FMI_ERR_ID|0x3a)
#define FMI_MS_ALLOC_ERR        (FMI_ERR_ID|0x3b)
#define FMI_MS_WRONG_SEGMENT    (FMI_ERR_ID|0x3c)
#define FMI_MS_WRONG_PHYBLOCK   (FMI_ERR_ID|0x3d)
#define FMI_MS_WRONG_TYPE       (FMI_ERR_ID|0x3e)
#define FMI_MS_WRITE_DISABLE    (FMI_ERR_ID|0x3f)

#define NAND_TYPE_SLC       0x01
#define NAND_TYPE_MLC       0x00

#define NAND_PAGE_512B      512
#define NAND_PAGE_2KB       2048
#define NAND_PAGE_4KB       4096
#define NAND_PAGE_8KB       8192

typedef struct fmi_sm_info_t
{
    UINT32  uSectorPerFlash;
    UINT32  uBlockPerFlash;
    UINT32  uPagePerBlock;
    UINT32  uSectorPerBlock;
    UINT32  uLibStartBlock;
    UINT32  nPageSize;
    UINT32  uBadBlockCount;
    BOOL    bIsMulticycle;
    BOOL    bIsMLCNand;
    BOOL    bIsNandECC4;
    BOOL    bIsNandECC8;
    BOOL    bIsNandECC12;
    BOOL    bIsNandECC15;
    BOOL    bIsCheckECC;
} FMI_SM_INFO_T;

extern FMI_SM_INFO_T *pSM0, *pSM1;


typedef struct fmi_sd_info_t
{
    UINT32  uCardType;      // sd2.0, sd1.1, or mmc
    UINT32  uRCA;           // relative card address
    BOOL    bIsCardInsert;
} FMI_SD_INFO_T;

extern FMI_SD_INFO_T *pSD0;
extern FMI_SD_INFO_T *pSD1;
extern FMI_SD_INFO_T *pSD2;


/* extern function */
#define SIC_SET_CLOCK       0
#define SIC_SET_CALLBACK    1
#define SIC_GET_CARD_STATUS 2
#define SIC_SET_CARD_DETECT 3
extern INT32 g_SD0_card_detect;

// MMC run 20MHz
#define MMC_FREQ    20000
#define EMMC_FREQ   26000

#define SD_FREQ     24000
#define SDHC_FREQ   24000


void sicOpen(void);
void sicClose(void);
INT sicSdOpen(void);
VOID sicSdClose(void);

INT sicSdOpen0(void);
INT sicSdOpen1(void);
INT sicSdOpen2(void);
VOID sicSdClose0(void);
VOID sicSdClose1(void);
VOID sicSdClose2(void);

VOID sicIoctl(INT32 sicFeature, INT32 sicArg0, INT32 sicArg1, INT32 sicArg2);

INT sicSdRead(INT32 sdSectorNo, INT32 sdSectorCount, INT32 sdTargetAddr);
INT sicSdRead0(INT32 sdSectorNo, INT32 sdSectorCount, INT32 sdTargetAddr);
INT sicSdRead1(INT32 sdSectorNo, INT32 sdSectorCount, INT32 sdTargetAddr);
INT sicSdRead2(INT32 sdSectorNo, INT32 sdSectorCount, INT32 sdTargetAddr);

INT sicSdWrite(INT32 sdSectorNo, INT32 sdSectorCount, INT32 sdSourceAddr);
INT sicSdWrite0(INT32 sdSectorNo, INT32 sdSectorCount, INT32 sdSourceAddr);
INT sicSdWrite1(INT32 sdSectorNo, INT32 sdSectorCount, INT32 sdSourceAddr);
INT sicSdWrite2(INT32 sdSectorNo, INT32 sdSectorCount, INT32 sdSourceAddr);


VOID fmiSMClose(INT chipSel);

/* gnand use */
#include "N9H20_GNAND.h"

INT nandInit0(NDISK_T *NDISK_info);
INT nandpread0(INT PBA, INT page, UINT8 *buff);
INT nandpwrite0(INT PBA, INT page, UINT8 *buff);
INT nand_is_page_dirty0(INT PBA, INT page);
INT nand_is_valid_block0(INT PBA);
INT nand_block_erase0(INT PBA);
INT nand_chip_erase0(void);
INT nand_ioctl(INT param1, INT param2, INT param3, INT param4);

INT nandInit1(NDISK_T *NDISK_info);
INT nandpread1(INT PBA, INT page, UINT8 *buff);
INT nandpwrite1(INT PBA, INT page, UINT8 *buff);
INT nand_is_page_dirty1(INT PBA, INT page);
INT nand_is_valid_block1(INT PBA);
INT nand_block_erase1(INT PBA);
INT nand_chip_erase1(void);


/* Declare callback function in waiting loop of SD driver */
#if defined (__GNUC__)
    __attribute__((weak)) void schedule(void);
#else
    __weak void schedule(void);
#endif

#endif /* _SIC_H_ */
