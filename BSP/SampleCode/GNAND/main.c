/***************************************************************************
 * Copyright (c) 2013 Nuvoton Technology. All rights reserved.
 *
 * FILENAME
 *     main.c
 * DESCRIPTION
 *     The main file for GNAND demo code.
 **************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "N9H20.h"

/*-----------------------------------------------------------------------------
 * for system configuration
 *---------------------------------------------------------------------------*/
// define DBG_PRINTF to sysprintf to show more information about emulation testing
#define DBG_PRINTF    sysprintf
//#define DBG_PRINTF(...)

//#define TEST_NAND_CARD

#define ERROR   -1
#define PASS    1
#define FAIL    0
#define OK      TRUE

#define ALL_BLOCK   0xFFFE

/*-----------------------------------------------------------------------------
 * define global variables
 *---------------------------------------------------------------------------*/
NDISK_T *ptMassNDisk;
NDISK_T MassNDisk;
NDISK_T *ptMassNDisk1;
NDISK_T MassNDisk1;

//UINT32 u32ExtFreq, u32UPllHz, u32SysHz, u32CpuHz, u32Hclk1Hz, u32ApbHz, u32APllHz;

/*-----------------------------------------------------------------------------
 * for NAND testing
 *---------------------------------------------------------------------------*/
// define number and size for internal buffer
#define SECTOR_SIZE         512
#define SECTOR_MAX_COUNT    1024
#define BUF_SIZE    (SECTOR_SIZE * SECTOR_MAX_COUNT)
__align (32) UINT8 g_ram0[BUF_SIZE];
__align (32) UINT8 g_ram1[BUF_SIZE];
UINT8 *ptr_g_ram0;
UINT8 *ptr_g_ram1;

/*-----------------------------------------------------------------------------
 * for GNAND testing
 *---------------------------------------------------------------------------*/
NDRV_T _nandDiskDriver0 =
{
    nandInit0,
    nandpread0,
    nandpwrite0,
    nand_is_page_dirty0,
    nand_is_valid_block0,
    nand_ioctl,
    nand_block_erase0,
    nand_chip_erase0,
    0
};

NDRV_T _nandDiskDriver1 =
{
    nandInit1,
    nandpread1,
    nandpwrite1,
    nand_is_page_dirty1,
    nand_is_valid_block1,
    nand_ioctl,
    nand_block_erase1,
    nand_chip_erase1,
    0
};

/*-----------------------------------------------------------------------------
 * show data by hex format
 *---------------------------------------------------------------------------*/
void show_hex_data(unsigned char *ptr, unsigned int length)
{
    unsigned int line_len = 8;
    unsigned int jj;

    for (jj=0; jj<length; jj++)
    {
        if (jj % line_len == 0)
            DBG_PRINTF("        ");
        DBG_PRINTF("0x%02x ", *(ptr+jj));
        if (jj % line_len == line_len-1)
            DBG_PRINTF("\n");
    }
    if (jj % line_len != 0)
        DBG_PRINTF("\n");
}


/*-----------------------------------------------------------------------------
 * 2011/3/7, To show basic information for NAND chip
 *  Parameters:
 *      ptMassNDisk:    pointer to NAND disk information block
 *  Return: None
 *---------------------------------------------------------------------------*/
void show_NAND_info(NDISK_T *ptMassNDisk)
{
    DBG_PRINTF("--- NAND Chip Information -----------------\n");
    DBG_PRINTF("    Vendor ID      : 0x%02X\n", ptMassNDisk->vendor_ID);
    DBG_PRINTF("    Device ID      : 0x%02X\n", ptMassNDisk->device_ID);
    DBG_PRINTF("    Page Size      : %d\n",     ptMassNDisk->nPageSize);
    DBG_PRINTF("    Pages per Block: %d\n",     ptMassNDisk->nPagePerBlock);
    DBG_PRINTF("    Blocks per chip: %d\n",     ptMassNDisk->nBlockPerZone);
    DBG_PRINTF("    Logical Blocks : %d\n",     ptMassNDisk->nLBPerZone);
    DBG_PRINTF("    Hidden Blocks  : %d\n",     ptMassNDisk->nStartBlock);
}


/*-----------------------------------------------------------------------------
 * 2011/3/7, To show basic information for GNAND
 *  Parameters:
 *      ptMassNDisk:    pointer to NAND disk information block
 *  Return: None
 *---------------------------------------------------------------------------*/
void show_GNAND_info(NDISK_T *ptMassNDisk)
{
    DBG_PRINTF("--- GNAND Information (PBA = GPBA + Hidden Blocks) -----------------\n");
    DBG_PRINTF("    P2LN block GPBA = %d\n", ptMassNDisk->p2ln_block);
    DBG_PRINTF("    OP block   GPBA = %d\n", ptMassNDisk->op_block);
    DBG_PRINTF("    Need 2 P2LN blocks ? %s !\n", ptMassNDisk->need2P2LN ? "Yes" : "No");
}


/*-----------------------------------------------------------------------------
 * Convert LBA code to readable string
 *  INPUT:
 *      lba:    LBA code.
 *  OUTPUT:
 *      buf:    pointer to readable string for LBA code
 *  Return: None.
 *---------------------------------------------------------------------------*/
void lba_string(UINT16 lba, char *buf)
{
    switch (lba)
    {
        case 0xFFFF: strcpy(buf, "FREE_BLOCK"); break;
        case 0xFFF0: strcpy(buf, "BAD_BLOCK");  break;
        case 0xFFAA: strcpy(buf, "OP_BLOCK");   break;
        case 0xFF55: strcpy(buf, "P2LN_BLOCK"); break;
        default:     sprintf(buf, "%d", lba);   break;
    }
}


/*-----------------------------------------------------------------------------
 * 2012/2/9, To show GPBA/LBA mapping for specific LBA
 *  Parameters:
 *      ptMassNDisk:    pointer to NAND disk information block
 *  Return: None
 *---------------------------------------------------------------------------*/
void show_GNAND_lba_map(NDISK_T *ptMassNDisk, UINT16 lba)
{
    int i;
    char buf[24];

    sysprintf("--- show_GNAND_lba_map(), show mapping for lba = 0x%x -----------\n", lba);
    for (i = 0; i < ptMassNDisk->nZone * ptMassNDisk->nBlockPerZone; i++)
    {
        if ((lba == ALL_BLOCK) || (lba == ptMassNDisk->p2lm[i].lba))
        {
            lba_string(ptMassNDisk->p2lm[i].lba, buf);
            sysprintf("    GPBA[%d] (age=%d) -> LBA[%s]\n", i, ptMassNDisk->p2lm[i].age, buf);
            if (lba == 0xFFAA)  // OP block
                GNAND_show_op(ptMassNDisk);
        }
    }
    sysprintf("--- End -----------\n\n", lba);
}


/*-----------------------------------------------------------------------------
 * 2012/2/9, To show GPBA/LBA mapping for specific GPBA
 *  INPUT:
 *      ptMassNDisk:    pointer to NAND disk information block
 *  Return: None
 *---------------------------------------------------------------------------*/
void show_GNAND_gpba_map(NDISK_T *ptMassNDisk, UINT16 gpba)
{
    char buf[24];
    sysprintf("--- show_GNAND_gpba_map(), show mapping for gpba = %d -----------\n", gpba);
    lba_string(ptMassNDisk->p2lm[gpba].lba, buf);
    sysprintf("    GPBA[%d] -> LBA[%s]\n", gpba, buf);
}


/*-----------------------------------------------------------------------------
 * To parse FAT table
 *  Parameters:
 *      ptMassNDisk:    pointer to NAND disk information block
 *  Return: None
 *---------------------------------------------------------------------------*/
void parse_FAT(NDISK_T *ptNDisk)
{
    int step = 1;
    DBG_PRINTF("--- Parse FAT Table on NAND -----------------\n");
    DBG_PRINTF("    --- Step %d: LBA 0 mapping to GPBA %d\n", step++, ptNDisk->l2pm[0].pba);

    DBG_PRINTF("    --- Step %d: Dump Partition table at LBA 0 page 0 offset 446...\n", step++);
    memset(ptr_g_ram1, 0x5a, BUF_SIZE);

    if (ptNDisk == ptMassNDisk)
        nandpread0(ptNDisk->l2pm[0].pba, 0, ptr_g_ram1);

    show_hex_data(ptr_g_ram1+446, 66);  // 66 = 512 - 446
}


/*-----------------------------------------------------------------------------
 * To do test for GNAND access. Write, read, and compare data.
 *---------------------------------------------------------------------------*/
int gnand_access_test(NDISK_T *ptMassNDisk)
{
    int ii;
    UINT32  result;
    UINT32  count, sector_num;
    UINT32  max_logic_sector_count;

    //--- initial random data, select random sector_num
    for(ii=0; ii<BUF_SIZE; ii++)
    {
        ptr_g_ram0[ii] = rand() & 0xFF;
    }

    max_logic_sector_count = ptMassNDisk->nPageSize / SECTOR_SIZE * ptMassNDisk->nPagePerBlock * ptMassNDisk->nLBPerZone;
    sector_num = rand() % max_logic_sector_count;   // random sector: 0 ~ max_logic_sector_count - 1
    count = (rand() % SECTOR_MAX_COUNT) + 1;        // random count: 1 ~ SECTOR_MAX_COUNT
    if (sector_num + count > max_logic_sector_count)
        count = 1;

    //--- do write and read back test
    result = GNAND_write(ptMassNDisk, sector_num, count, ptr_g_ram0);
    DBG_PRINTF("    Write g_ram0 to GNAND, result = 0x%x\n", result);
    show_hex_data(ptr_g_ram0, 16);

    memset(ptr_g_ram1, 0x5a, BUF_SIZE);
    result = GNAND_read(ptMassNDisk, sector_num, count, ptr_g_ram1);
    DBG_PRINTF("    Read GNAND to g_ram1,  result = 0x%x\n", result);
    show_hex_data(ptr_g_ram1, 16);

    //--- compare data
    if(memcmp(ptr_g_ram0, ptr_g_ram1, SECTOR_SIZE*count) == 0)
    {
        DBG_PRINTF("    Data compare OK at sector %d (LBA=%d), count %d\n", sector_num, sector_num*SECTOR_SIZE/(ptMassNDisk->nPageSize * ptMassNDisk->nPagePerBlock), count);
        result = OK;
    }
    else
    {
        DBG_PRINTF("    ERROR: Data compare ERROR at sector %d (LBA=%d), count %d\n\n", sector_num, sector_num*SECTOR_SIZE/(ptMassNDisk->nPageSize * ptMassNDisk->nPagePerBlock), count);
        show_GNAND_lba_map(ptMassNDisk, sector_num*SECTOR_SIZE/(ptMassNDisk->nPageSize * ptMassNDisk->nPagePerBlock));
        result = FAIL;

         for(ii=0; ii<BUF_SIZE; ii++)
        {
            if (ptr_g_ram0[ii] != ptr_g_ram1[ii])
                DBG_PRINTF("    ERROR byte write 0x%02x, read 0x%02x at offset %d\n", ptr_g_ram0[ii], ptr_g_ram1[ii], ii);
        }
    }
    return result;
}


/*-----------------------------------------------------------------------------
 * Initial UART0.
 *---------------------------------------------------------------------------*/
void init_UART()
{
    UINT32 u32ExtFreq;
    WB_UART_T uart;

    u32ExtFreq = sysGetExternalClock()*1000;
    sysUartPort(1);
    uart.uiFreq = u32ExtFreq;   //use APB clock
    uart.uiBaudrate = 115200;
    uart.uiDataBits = WB_DATA_BITS_8;
    uart.uiStopBits = WB_STOP_BITS_1;
    uart.uiParity = WB_PARITY_NONE;
    uart.uiRxTriggerLevel = LEVEL_1_BYTE;
    uart.uart_no = WB_UART_1;
    sysInitializeUART(&uart);
}


/*-----------------------------------------------------------------------------
 * Initial Timer0 interrupt for system tick.
 *---------------------------------------------------------------------------*/
void init_timer()
{
    sysSetTimerReferenceClock(TIMER0, sysGetExternalClock()*1000);  // External Crystal
    sysStartTimer(TIMER0, 100, PERIODIC_MODE);                      // 100 ticks/per sec ==> 1tick/10ms
    sysSetLocalInterrupt(ENABLE_IRQ);
}


/*-----------------------------------------------------------------------------*/
int main(void)
{
    UINT32  u32Item, u32Item2;

    init_UART();
    init_timer();

    sysprintf("\n=====> N9H20 Non-OS GNAND Library Sampe Code [tick %d] <=====\n", sysGetTicks(0));

    ptr_g_ram0 = (UINT8 *)((UINT32)g_ram0 | 0x80000000);    // non-cache
    ptr_g_ram1 = (UINT8 *)((UINT32)g_ram1 | 0x80000000);    // non-cache

    //--- enable system cache feature
    sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    DBG_PRINTF("    ** enable cache to CACHE_WRITE_BACK mode.\n");

    //--- initialize SIC/FMI
    sicOpen();

    //--- initial GNAND
    ptMassNDisk = (NDISK_T*)&MassNDisk;
    if (GNAND_InitNAND(&_nandDiskDriver0, (NDISK_T *)ptMassNDisk, TRUE))
    {
        sysprintf("ERROR: GNAND CS 0 initial fail !!\n");
        goto __exit__;
    }

#ifdef TEST_NAND_CARD
    // for NAND card
    ptMassNDisk1 = (NDISK_T*)&MassNDisk1;
    if (GNAND_InitNAND(&_nandDiskDriver1, (NDISK_T *)ptMassNDisk1, FALSE))
    {
        sysprintf("ERROR: GNAND CS 1 initial fail !!\n");
        goto __exit__;
    }
#endif

    /*----- Menu for emulation items ----------*/
    do
    {
        sysprintf("\n");
        sysprintf("==================================================\n");
        sysprintf("    0. Erase whole NAND and exit\n");
        sysprintf("    1. Show NAND/GNAND Information\n");
        sysprintf("    2. Show GNAND LBA mapping (include age)\n");
        sysprintf("    3. Show GNAND P2LN/OP block\n");
        sysprintf("    4. GNAND access test\n");
        sysprintf("    5. Parsing FAT Table\n");
        sysprintf("    Q. Exit\n");
        sysprintf("Select one action: ");
        u32Item = sysGetChar();
        sysprintf("%c\n\n", u32Item);
        switch(u32Item)
        {
            case '0':
                sysprintf("All data will lost after Erase NAND. Are you sure? <y/n> : ");
                u32Item2 = sysGetChar();
                sysprintf("%c\n\n", u32Item2);
                if ((u32Item2 == 'y') || (u32Item2 == 'Y'))
                {
                    sysprintf("Begin to erase NAND CS0 ...\n");
                    GNAND_chip_erase(ptMassNDisk);
                    goto __exit__;
                }
                else
                    sysprintf("Do nothing !!\n");
                break;

            case '1':
                show_NAND_info(ptMassNDisk);
                show_GNAND_info(ptMassNDisk);

#ifdef TEST_NAND_CARD
                // for NAND card
                show_NAND_info(ptMassNDisk1);
                show_GNAND_info(ptMassNDisk1);
#endif
                break;

            case '2':
                sysprintf("Show CS0 all mapping table\n");
                show_GNAND_lba_map(ptMassNDisk, ALL_BLOCK);
                break;

            case '3':
                sysprintf("Show P2LN_BLOCK mapping\n");
                show_GNAND_lba_map(ptMassNDisk, P2LN_BLOCK);

                sysprintf("Show OP_BLOCK mapping\n");
                show_GNAND_lba_map(ptMassNDisk, OP_BLOCK);

                sysprintf("Show BAD_BLOCK mapping\n");
                show_GNAND_lba_map(ptMassNDisk, BAD_BLOCK);
                break;

            case '4':
                sysprintf("CS0 GNAND access testing...\n");
                gnand_access_test(ptMassNDisk);
                break;

            case '5':
                sysprintf("CS0 Paring FAT table...\n");
                parse_FAT(ptMassNDisk);
                break;

            case 'Q':
            case 'q':
                break;
            default:
                sysprintf("Wrong Item %d!\n", u32Item);
                break;
        }
    } while((u32Item != 'q') && (u32Item != 'Q'));

__exit__:
    sysprintf("\n===== THE END =====\n\n");
    return 0;
}
