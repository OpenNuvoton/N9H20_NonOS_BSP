/**************************************************************************//**
 * @file     NandLoader.c
 * @brief    NandLoader source code.
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <string.h>
#include "wblib.h"
#include "turbowriter.h"
#include "N9H20_VPOST.h"

// define DATE CODE and show it when running to make maintaining easy.
#define DATE_CODE   "20201214"

/* global variable */
typedef struct nand_info
{
    unsigned int startBlock;
    unsigned int endBlock;
    unsigned int fileLen;
    unsigned int executeAddr;
} NVT_NAND_INFO_T;
extern void spuDacOn(UINT8 level);

#ifdef __USER_DEFINE_FUNC
    extern VOID user_define_func(void);
#endif

INT MoveData(NVT_NAND_INFO_T *image, BOOL IsExecute)
{
    unsigned int page_count, block_count, curBlock, addr;
    int volatile i, j;
    void    (*fw_func)(void);

    sysprintf("Load file length 0x%x, execute address 0x%x\n", image->fileLen, image->executeAddr);

    page_count = image->fileLen / pSM0->nPageSize;
    if ((image->fileLen % pSM0->nPageSize) != 0)
        page_count++;

    block_count = page_count / pSM0->uPagePerBlock;

    curBlock = image->startBlock;
    addr = image->executeAddr;
    j=0;
    while(1)
    {
        if (j >= block_count)
            break;
        if (CheckBadBlockMark(curBlock) == Successful)
        {
            for (i=0; i<pSM0->uPagePerBlock; i++)
            {
                sicSMpread(0, curBlock, i, (UINT8 *)addr);
                addr += pSM0->nPageSize;
            }
            j++;
        }
        curBlock++;
    }

    if ((page_count % pSM0->uPagePerBlock) != 0)
    {
        page_count = page_count - block_count * pSM0->uPagePerBlock;
_read_:
        if (CheckBadBlockMark(curBlock) == Successful)
        {
            for (i=0; i<page_count; i++)
            {
                sicSMpread(0, curBlock, i, (UINT8 *)addr);
                addr += pSM0->nPageSize;
            }
        }
        else
        {
            curBlock++;
            goto _read_;
        }
    }

    if (IsExecute == TRUE)
    {
        /* disable NAND control pin used */
        outpw(REG_GPDFUN, inpw(REG_GPDFUN) & ~0x0003FC00);      // disable NAND NWR/NRD/RB0/RB1 pins
        outpw(REG_GPEFUN, inpw(REG_GPEFUN) & ~0x00FF0000);      // disable NAND ALE/CLE/CS0/CS1 pins

        // disable SD Card Host Controller operation and driving clock.
        outpw(REG_GPEFUN, inpw(REG_GPEFUN)&(~0x0000FFF0));      // disable SD0_CLK/CMD/DAT0_3 pins selected

        fw_func = (void(*)(void))(image->executeAddr);
        fw_func();
    }
    return 0;
}


#if defined (__GNUC__)
    UINT8 image_buffer[4096] __attribute__((aligned (32)));
#else
    __align(32) UINT8 image_buffer[4096];
#endif

unsigned char *imagebuf;
unsigned int *pImageList;
UINT32 g_u32ExtClk;


#ifdef _DEBUG_AIC_
UINT32 u3210ms=0;
void timer0_callback(void)
{
    u3210ms = u3210ms +1;
    sysprintf("u3210ms = %d\n", u3210ms);
}

void initTimer(void)
{
    UINT32 u32ExtFreq;
    u32ExtFreq = sysGetExternalClock();     /* KHz unit */
    sysSetTimerReferenceClock (TIMER0, u32ExtFreq*1000);    /* Hz unit */
    sysStartTimer(TIMER0, 100, PERIODIC_MODE);              /* 100 tick/1s ==> 1 tick = 10ms*/
    sysSetTimerEvent(TIMER0, 1, (PVOID)timer0_callback);    /* Each 10 ticks = 100ms call back */
}
#endif

#define E_CLKSKEW   0x00888800

#ifdef N9H20K5
    #define __DDR2__	// N9H20K5 use DDR2
#else
    #define __DDR_6__
#endif
void initClock(void)
{
    UINT32 u32ExtFreq;

    u32ExtFreq = sysGetExternalClock();     /* KHz unit */
    outp32(REG_DQSODS, 0x1010);
    outp32(REG_CKDQSDS, E_CLKSKEW);
    if(u32ExtFreq==12000)
    {
        outp32(REG_SDREF, 0x805A);
    }
    else
    {
        outp32(REG_SDREF, 0x80C0);
    }


#ifdef __UPLL_192__
    if((inp32(REG_CHIPCFG) & SDRAMSEL) == 0x20)     // Power On Setting SDRAM type is DDR2
    {
        outp32(REG_SDMR, 0x432);
        outp32(REG_DQSODS, 0x00001010);
        outp32(REG_MISCPCR,0x00000001);     // Driving strength
        outp32(REG_SDTIME, 0x21667525);
    }
    else if((inp32(REG_CHIPCFG) & SDRAMSEL) == 0x30)     // Power On Setting SDRAM type is DDR
    {
    #ifdef __DDR_75__
        outp32(REG_SDTIME, 0x098E7549); // DDR Speed grade-75
    #endif
    #ifdef __DDR_6__
        outp32(REG_SDTIME, 0x094E7425); // DDR Speed grade-6
    #endif
    #ifdef __DDR_5__
        outp32(REG_SDTIME, 0x094E6425); // DDR Speed grade-5
    #endif
        outp32(REG_SDMR, 0x22);         // Cas Latency = 2
    }

    sysSetSystemClock(eSYS_UPLL,    //E_SYS_SRC_CLK eSrcClk,
                    192000,         //UINT32 u32PllKHz,
                    192000,         //UINT32 u32SysKHz,
                    192000,         //UINT32 u32CpuKHz,
                    192000/2,       //UINT32 u32HclkKHz,
                    192000/4);      //UINT32 u32ApbKHz
#endif


#ifdef __UPLL_96__

    #ifdef __DDR_75__
    outp32(REG_SDTIME, 0x098E7549); // DDR Speed grade-75
    #endif
    #ifdef __DDR_6__
    outp32(REG_SDTIME, 0x094E7425); // DDR Speed grade-6
    #endif
    #ifdef __DDR_5__
    outp32(REG_SDTIME, 0x094E6425); // DDR Speed grade-5
    #endif
    #ifdef __DDR2__
    outp32(REG_SDMR, 0x32);         //Cas Latency = 3
    outp32(REG_DQSODS, 0x00001212);
    outp32(REG_MISCPCR,0x00000001); //Driving strength
    outp32(REG_SDTIME, 0x2BDE9649);
    #else
    outp32(REG_SDMR, 0x22);         //Cas Latency = 2
    #endif

    sysSetSystemClock(eSYS_UPLL,    //E_SYS_SRC_CLK eSrcClk,
                    96000,          //UINT32 u32PllKHz,
                    96000,          //UINT32 u32SysKHz,
                    96000,          //UINT32 u32CpuKHz,
                    96000/2,        //UINT32 u32HclkKHz,
                    96000/4);       //UINT32 u32ApbKHz
#endif


#ifdef __UPLL_120__
    #ifdef __DDR_75__
    outp32(REG_SDTIME, 0x0A129649); // DDR Speed grade-75
    #endif
    #ifdef __DDR_6__
    outp32(REG_SDTIME, 0x0A149529); // DDR Speed grade-6
    #endif
    #ifdef __DDR_5__
    outp32(REG_SDTIME, 0x09928525); // DDR Speed grade-5
    #endif
    outp32(REG_SDMR, 0x32);         //Cas Latency = 3
    sysSetSystemClock(eSYS_UPLL,    //E_SYS_SRC_CLK eSrcClk,
                    120000,         //UINT32 u32PllKHz,
                    120000,         //UINT32 u32SysKHz,
                    120000,         //UINT32 u32CpuKHz,
                    120000/2,       //UINT32 u32HclkKHz,
                    120000/4);      //UINT32 u32ApbKHz
#endif
}

int main()
{
    NVT_NAND_INFO_T image;
    int count, i;
    WB_UART_T uart;
    E_SYS_SRC_CLK eSrcClk;
    UINT32 u32PllKHz, u32SysKHz, u32CpuKHz, u32HclkKHz, u32ApbKHz;

    /* Clear Boot Code Header in SRAM to avoid booting fail issue */
    outp32(0xFF000000, 0);

    spuDacOn(2);
    /* PLL clock setting */
    initClock();
#ifdef _DEBUG_AIC_
    initTimer();
#endif
//  sysSetLocalInterrupt(ENABLE_IRQ);

#ifndef __DISABLE_RTC__
    // RTC H/W Power Off Function Configuration */
    outp32(AER,0x0000a965);
    while(1)
    {
        if((inp32(AER) & 0x10000) ==0x10000)
            break;
    }
    outp32(PWRON, 0x60005);     /* Press Power Key during 6 sec to Power off (0x'6'0005) */
    outp32(RIIR,0x4);
#endif

    uart.uiFreq = sysGetExternalClock()*1000;       /*Use external clock*/
    uart.uiBaudrate = 115200;
    uart.uiDataBits = WB_DATA_BITS_8;
    uart.uiStopBits = WB_STOP_BITS_1;
    uart.uiParity = WB_PARITY_NONE;
    uart.uiRxTriggerLevel = LEVEL_1_BYTE;
    sysInitializeUART(&uart);
    sysprintf("N9H20 Nand Boot Loader entry (%s).\n", DATE_CODE);

#ifdef __DISABLE_RTC__
    sysprintf("Disable RTC feature.\n");
#endif

    sysGetSystemClock(&eSrcClk, &u32PllKHz, &u32SysKHz, &u32CpuKHz, &u32HclkKHz, &u32ApbKHz);
    sysprintf("System clock = %dKHz\nAHB clock = %dKHz\nREG_SDTIME = 0x%08X\n",
               u32SysKHz, u32HclkKHz, inp32(REG_SDTIME));

    imagebuf = (UINT8 *)((UINT32)image_buffer | 0x80000000);
    pImageList=((unsigned int *)(((unsigned int)image_buffer)|0x80000000));

    /* Initial DMAC and NAND interface */
    fmiInitDevice();
    sicSMInit();

    memset((char *)&image, 0, sizeof(NVT_NAND_INFO_T));

    /* read physical block 0 - image information */
#if 1
    for (i=0; i<4; i++)
    {
        if (!sicSMpread(0, i, pSM0->uPagePerBlock-2, imagebuf))
        {
            if (((*(pImageList+0)) == 0x574255aa) && ((*(pImageList+3)) == 0x57425963))
            {
                sysprintf("Get NANDLoader image from block 0x%x ..\n", i);
                break;
            }
        }
    }
#else
    sicSMpread(0, 0, pSM0->uPagePerBlock-2, imagebuf);
#endif


    if (((*(pImageList+0)) == 0x574255aa) && ((*(pImageList+3)) == 0x57425963))
    {
        count = *(pImageList+1);

        /* load logo first */
        pImageList = pImageList+4;
        for (i=0; i<count; i++)
        {
            if (((*(pImageList) >> 16) & 0xffff) == 4)  // logo
            {
                image.startBlock = *(pImageList + 1) & 0xffff;
                image.endBlock = (*(pImageList + 1) & 0xffff0000) >> 16;
                image.executeAddr = *(pImageList + 2);
                image.fileLen = *(pImageList + 3);
                MoveData(&image, FALSE);
                break;
            }
            /* pointer to next image */
            pImageList = pImageList+12;
        }

        pImageList=((unsigned int*)(((unsigned int)image_buffer)|0x80000000));
        memset((char *)&image, 0, sizeof(NVT_NAND_INFO_T));

#ifdef __USER_DEFINE_FUNC
        //--- call user define function before jump to next application.
        user_define_func();
#endif

        /* load execution file */
        pImageList = pImageList+4;
        for (i=0; i<count; i++)
        {
            if (((*(pImageList) >> 16) & 0xffff) == 1)  // execute
            {
                image.startBlock = *(pImageList + 1) & 0xffff;
                image.endBlock = (*(pImageList + 1) & 0xffff0000) >> 16;
                image.executeAddr = *(pImageList + 2);
                image.fileLen = *(pImageList + 3);
                MoveData(&image, TRUE);
                break;
            }
            /* pointer to next image */
            pImageList = pImageList+12;
        }
    }
    return 0;
}
