/**************************************************************************//**
 * @file     main.c
 * @brief    The main file for SIC/NAND demo code
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "N9H20.h"

/*-----------------------------------------------------------------------------
 * For test configuration
 *---------------------------------------------------------------------------*/
#define OK      TRUE
#define FAIL    FALSE

/* RAM buffer size for one NAND flash access in this test. */
#define BUF_SIZE    (8192)

/*-----------------------------------------------------------------------------
 * For global variables
 *---------------------------------------------------------------------------*/
NDISK_T *ptMassNDisk;
NDISK_T MassNDisk;

#if defined (__GNUC__)
    UINT8 g_ram0[BUF_SIZE] __attribute__((aligned (32)));
    UINT8 g_ram1[BUF_SIZE] __attribute__((aligned (32)));
#else
    __align (32) UINT8 g_ram0[BUF_SIZE];
    __align (32) UINT8 g_ram1[BUF_SIZE];
#endif


/*-----------------------------------------------------------------------------
 * Show data by hex format
 *---------------------------------------------------------------------------*/
void show_hex_data(unsigned char *ptr, unsigned int length)
{
    unsigned int line_len = 8;
    unsigned int i;

    for (i=0; i<length; i++)
    {
        if (i % line_len == 0)
            sysprintf("        ");
        sysprintf("0x%02x ", *(ptr+i));
        if (i % line_len == line_len-1)
            sysprintf("\n");
    }
    if (i % line_len != 0)
        sysprintf("\n");
}


/*-----------------------------------------------------------------------------
 * To do test for NAND access. Write, read, and compare data.
 *---------------------------------------------------------------------------*/
int nand_access_test(int nand_port)
{
    int ii;
    UINT32  result;
    UINT32  block, page;
    UINT8   *ptr_g_ram0, *ptr_g_ram1;

    ptr_g_ram0 = (UINT8 *)((UINT32)g_ram0 | 0x80000000);    // non-cache
    ptr_g_ram1 = (UINT8 *)((UINT32)g_ram1 | 0x80000000);    // non-cache

    //--- initial random data, select random block index and page index
    for(ii=0; ii<ptMassNDisk->nPageSize; ii++)
    {
        ptr_g_ram0[ii] = rand() & 0xFF;
    }
    // select random block except first 4 blocks.
    //      First 4 blocks is booting block and control by IBR. Don't touch it.
    block = (rand() % (ptMassNDisk->nBlockPerZone - 4)) + 4;
    page  = rand() % ptMassNDisk->nPagePerBlock;

    //--- do write and read back test
    switch (nand_port)
    {
        case 0: result = nand_block_erase0(block);  break;
        case 1: result = nand_block_erase1(block);  break;
    }
    if (result != 0)
    {
        // Erase block fail. Could be bad block. Ignore it.
        sysprintf("    Ignore since nand_block_erase0() fail, block = %d, page = %d, result = 0x%x\n", block, page, result);
        return OK;
    }

    switch (nand_port)
    {
        case 0: result = nandpwrite0(block, page, ptr_g_ram0);  break;
        case 1: result = nandpwrite1(block, page, ptr_g_ram0);  break;
    }
    sysprintf("    Write g_ram0 to NAND, result = 0x%x\n", result);
    show_hex_data(ptr_g_ram0, 16);

    memset(ptr_g_ram1, 0x5a, BUF_SIZE);
    switch (nand_port)
    {
        case 0: result = nandpread0(block, page, ptr_g_ram1);   break;
        case 1: result = nandpread1(block, page, ptr_g_ram1);   break;
    }
    sysprintf("    Read NAND to g_ram1,  result = 0x%x\n", result);
    show_hex_data(ptr_g_ram1, 16);

    //--- compare data
    if(memcmp(ptr_g_ram0, ptr_g_ram1, ptMassNDisk->nPageSize) == 0)
    {
        sysprintf("    Data compare OK at block %d page %d\n", block, page);
        return OK;
    }
    else
    {
        sysprintf("    ERROR: Data compare ERROR at block %d page %d\n", block, page);
        return FAIL;
    }
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
    int result, i, pass, fail;
    int target_port = 0;

    init_UART();
    init_timer();

    sysprintf("\n=====> N9H20 Non-OS SIC/NAND Driver Sample Code [tick %d] <=====\n", sysGetTicks(0));

    //--- enable cache feature
    sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);

    //--- initial SIC/NAND driver for target_port
    sicOpen();
    ptMassNDisk = (NDISK_T*)&MassNDisk;

    switch (target_port)
    {
        case 0: result = nandInit0((NDISK_T *)ptMassNDisk);   break;
        case 1: result = nandInit1((NDISK_T *)ptMassNDisk);   break;
        default: sysprintf("ERRROR: Wrong NAND port number %d\n", target_port); break;
    }
    if (result)
    {
        sysprintf("ERROR: NAND port %d initial fail !!\n", target_port);
    }
    else
    {
        //--- do basic NAND access test
        sysprintf("Do basic NAND access test on port %d ...\n", target_port);
        i = 0;
        pass = 0;
        fail = 0;
        while(i<10)
        {
            i++;
            sysprintf("=== Loop %d ...\n", i);
            result = nand_access_test(target_port);
            if (result == OK)
            {
                sysprintf("NAND access test is SUCCESSFUL!!!\n");
                pass++;
            }
            else
            {
                sysprintf("NAND access test is FAIL!!!\n");
                fail++;
            }
            sysprintf("=== Pass %d, Fail %d\n", pass, fail);
        }
    }

    sysprintf("\n===== THE END ===== [tick %d]\n", sysGetTicks(0));
    while(1);
}
