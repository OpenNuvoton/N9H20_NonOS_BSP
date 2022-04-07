/**************************************************************************//**
 * @file     N9H20_GNAND.h
 * @version  V3.00
 * @brief    N9H20 series GNAND driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#ifndef _GNAND_H_
#define _GNAND_H_

#define GNAND_OK                    0
#define GNERR_READ_L2P_MISS         1   /* read block not found in L2P        */
#define GNAND_ERR                   0xFFFFC000

/* GENERAL ERRORs */
#define GNERR_GENERAL               (GNAND_ERR+1)       /* general error                      */
#define GNERR_MEMORY_OUT            (GNAND_ERR+0x5)     /* memory not enough                  */
#define GNERR_GNAND_FORMAT          (GNAND_ERR+0x10)    /* not GNAND format                   */
#define GNERR_FAT_FORMAT            (GNAND_ERR+0x15)    /* NAND disk was not formatted as FAT */
#define GNERR_BLOCK_OUT             (GNAND_ERR+0x20)    /* there's no available free blocks   */
#define GNERR_P2LN_SYNC             (GNAND_ERR+0x25)
#define GNERR_READONLY_NAND         (GNAND_ERR+0x26)    /* XtraROM */

/* for NAND driver return value */
#define GNERR_IO_ERR                (GNAND_ERR+0x30)    /* NAND read/write/erase access failed*/
#define GNERR_NAND_NOT_FOUND        (GNAND_ERR+0x40)    /* NAND driver cannot find NAND disk  */
#define GNERR_UNKNOW_ID             (GNAND_ERR+0x42)    /* Not supported NAND ID              */


/*-------------------------------------------------------------------*/
/*  NAND driver function set                                         */
/*-------------------------------------------------------------------*/
#define NDISK_T     struct ndisk_t
#define NDRV_T      struct ndrv_t


typedef struct p2lm_t
{
    UINT16  lba;            /* logical block address                 */
    UINT16  age;            /* times this block has been used        */
}  P2LM_T;


typedef struct l2pm_t
{
    UINT16  pba;            /* physical block address                */
    UINT16  reserved;       /* reserved for future used              */
}  L2PM_T;


// Define constant for sturct ndisk_t.NAND_type
#define NAND_TYPE_MLC       0x00
#define NAND_TYPE_SLC       0x01

// Define constant for sturct ndisk_t.write_page_in_seq
#define NAND_TYPE_PAGE_OUT_SEQ  0   // SLC NAND that can program page out of sequence
#define NAND_TYPE_PAGE_IN_SEQ   1   // SLC NAND that have to program page in sequence

/*-------------------------------------------------------------------*/
/*  NAND disk infotmation. This information was provided             */
/*  by NAND driver.                                                  */
/*-------------------------------------------------------------------*/
struct ndisk_t
{
    INT     vendor_ID;
    INT     device_ID;
    INT     NAND_type;          /* NAND_TYPE_MLC / NAND_TYPE_SLC         */
    INT     nZone;              /* number of zones                       */
    INT     nBlockPerZone;      /* blocks per zone                       */
    INT     nPagePerBlock;      /* pages per block                       */
    INT     nLBPerZone;         /* logical blocks per zone               */
    INT     nPageSize;
    INT     nStartBlock;        /* available start block                 */
    INT     nBadBlockCount;     /* bad block count                       */
    NDRV_T  *driver;            /* NAND driver to work on this NAND disk */
    INT     nNandNo;
    VOID    *pDisk;
    INT     write_page_in_seq;  /* NAND_TYPE_PAGE_OUT_SEQ / NAND_TYPE_PAGE_IN_SEQ */
    INT     reserved[59];
    INT     need2P2LN;
    INT     p2ln_block1;

    /* for GNAND internal used */
    P2LM_T  *p2lm;
    L2PM_T  *l2pm;
    UINT8   *dp_tbl;        /* dirty page bit map */
    UINT16  db_idx[16];     /* data block search index, up to 8 zone */
    UINT16  p2ln_block;     /* P2LN block No. */
    UINT16  op_block;       /* OP block No. */
    INT     op_offset;      /* operation index */
    UINT8   last_op[32];    /* the last op code in op table */

    INT     err_sts;
    struct ndisk_t  *next;
};


struct ndrv_t
{
    INT  (*init)(NDISK_T *NDInfo);
    INT  (*pread)(INT nPBlockAddr, INT nPageNo, UINT8 *buff);
    INT  (*pwrite)(INT nPBlockAddr, INT nPageNo, UINT8 *buff);
    INT  (*is_page_dirty)(INT nPBlockAddr, INT nPageNo);
    INT  (*is_valid_block)(INT nPBlockAddr);
    INT  (*ioctl)(INT param1, INT param2, INT param3, INT param4);
    INT  (*block_erase)(INT nPBlockAddr);
    INT  (*chip_erase)(void);
    VOID *next;
} ;


/*-------------------------------------------------------------------*/
/* Export functions                                                  */
/*-------------------------------------------------------------------*/
INT  GNAND_InitNAND(NDRV_T *ndriver, NDISK_T *ptNDisk, BOOL bEraseIfNotGnandFormat);
INT  GNAND_MountNandDisk(NDISK_T *ptNDisk);
INT  GNAND_read(NDISK_T *ptNDisk, UINT32 nSectorNo, INT nSectorCnt, UINT8 *buff);
INT  GNAND_write(NDISK_T *ptNDisk, UINT32 nSectorNo, INT nSectorCnt, UINT8 *buff);
INT  GNAND_block_erase(NDISK_T *ptNDisk, INT pba);
INT  GNAND_chip_erase(NDISK_T *ptNDisk);
VOID GNAND_UnMountNandDisk(NDISK_T *ptNDisk);

#endif  /* _GNAND_H_ */
