/**************************************************************************//**
 * @file     main.c
 * @brief    SpiLoader source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "N9H20.h"
#include "SpiLoader.h"
#include "tag.h"

#define E_CLKSKEW  0x00888800

#define __No_RTC__
#define __UPLL_192__

//#define _S605_

#ifdef _S605_
    #ifdef __Security__
    #define DATE_CODE   "20230228 - gzip with Security for S605"
    #else
    #define DATE_CODE   "20230228 - gzip for S605"
    #endif
#else
    #ifdef __Security__
    #define DATE_CODE   "20230228 - gzip with Security"
    #else
    #define DATE_CODE   "20230228 - gzip"
    #endif
#endif

#define __DAC_ON__

#define LOAD_IMAGE          0
#define CHECK_HEADER_ONLY   1
#ifdef _N9H20K5_
#define IMAGE_BUFFER        0xA00000
#elif defined _N9H20K3_
#define IMAGE_BUFFER        0x500000
#else
#define IMAGE_BUFFER        0x100000
#endif

#if defined (__GNUC__)
UINT8  image_buffer[1024] __attribute__((aligned(32)));
#else
UINT8 __align(32) image_buffer[1024];
#endif

unsigned char *imagebuf;
unsigned int *pImageList;

int do_bootm (UINT32 u32SrcAddress,UINT32 u32DestAddress, UINT32 u32Mode);

extern void lcmFill2Dark(unsigned char*);
//unsigned char kbuf[CP_SIZE]; /* save first 16k of buffer. Copy to 0 after vector table is no longer needed */
extern void AudioChanelControl(void);
extern void backLightEnable(void);
int kfd, mfd;
#ifdef __Security__
#define KEY_INDEX 1
void RPMC_CreateRootKey(unsigned char *u8uid, unsigned int id_len, unsigned char *rootkey);
#endif

UINT32 u32TimerChannel = 0;
void Timer0_300msCallback(void)
{
#ifdef __BACKLIGHT__
    backLightEnable();
#endif
    sysClearTimerEvent(TIMER0, u32TimerChannel);
}

#define E_CLKSKEW   0x00888800
#define __DDR_6__

void init(void)
{
    WB_UART_T  uart;
    UINT32  u32ExtFreq;
    UINT32 u32Cke = inp32(REG_AHBCLK);

    /* Reset SIC engine to fix USB update kernel and movie file */
    outp32(REG_AHBCLK, u32Cke  | (SIC_CKE | NAND_CKE | SD_CKE)); 
    outp32(REG_AHBIPRST, inp32(REG_AHBIPRST )|SICRST ); 
    outp32(REG_AHBIPRST, 0); 
    outp32(REG_APBIPRST, TMR0RST | TMR1RST); 
    outp32(REG_APBIPRST, 0); 
    outp32(REG_AHBCLK,u32Cke);
    sysEnableCache(CACHE_WRITE_BACK);

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
    if((inp32(REG_CHIPCFG) & SDRAMSEL) == 0x20)     /* Power On Setting SDRAM type is DDR2 */
    {
        outp32(REG_SDMR, 0x432);
        outp32(REG_DQSODS, 0x00001010);
        outp32(REG_MISCPCR,0x00000001);             /* Driving strength */
        outp32(REG_SDTIME, 0x21667525);
    }
    else if((inp32(REG_CHIPCFG) & SDRAMSEL) == 0x30)     /* Power On Setting SDRAM type is DDR */
    {
    #ifdef __DDR_75__
        outp32(REG_SDTIME, 0x098E7549); /* DDR Speed grade-75 */
    #endif
    #ifdef __DDR_6__
        outp32(REG_SDTIME, 0x094E7425); /* DDR Speed grade-6 */
    #endif
    #ifdef __DDR_5__
        outp32(REG_SDTIME, 0x094E6425); /* DDR Speed grade-5 */
    #endif
        outp32(REG_SDMR, 0x22);         /* Cas Latency = 2 */
    }

    sysSetSystemClock(eSYS_UPLL,    /* E_SYS_SRC_CLK eSrcClk, */
                    192000,         /* UINT32 u32PllKHz, */
                    192000,         /* UINT32 u32SysKHz, */
                    192000,         /* UINT32 u32CpuKHz, */
                    192000/2,       /* UINT32 u32HclkKHz, */
                    192000/4);      /* UINT32 u32ApbKHz */
#endif

    /* enable UART */
    sysUartPort(1);
    uart.uiFreq = u32ExtFreq*1000;    /* Hz unit */
    uart.uiBaudrate = 115200;
    uart.uiDataBits = WB_DATA_BITS_8;
    uart.uiStopBits = WB_STOP_BITS_1;
    uart.uiParity = WB_PARITY_NONE;
    uart.uiRxTriggerLevel = LEVEL_1_BYTE;
    sysInitializeUART(&uart);
    sysprintf("SPI Loader start (%s).\n", DATE_CODE);
    sysSetLocalInterrupt(ENABLE_IRQ);
    sysFlushCache(I_D_CACHE);
}

#ifdef _S605_   /* support S605 chip */
/*----------------------------------------------------------------------------
  Initail GPIO pins GPE10 and GPE11 to output mode for S605 module.
  ----------------------------------------------------------------------------*/
void S605_init_gpio()
{
    sysprintf("Initial GPIO pins for S605 module.\n");

    /* initial GPE10 to output mode and pull low it. */
    outpw(REG_GPEFUN, inpw(REG_GPEFUN) & (~MF_GPE10));      /* set GPE10 as GPIO pin */
    outpw(REG_GPIOE_PUEN, inpw(REG_GPIOE_PUEN) | BIT10);    /* set GPE10 internal resistor to pull up */
    outpw(REG_GPIOE_DOUT, inpw(REG_GPIOE_DOUT) & (~BIT10)); /* output 0 to GPE10 */
    outpw(REG_GPIOE_OMD,  inpw(REG_GPIOE_OMD)  | BIT10);    /* set GPE10 to OUTPUT mode */

    /* initial GPE11 to output mode and pull low it. */
    outpw(REG_GPEFUN, inpw(REG_GPEFUN) & (~MF_GPE11));      /* set GPE11 as GPIO pin */
    outpw(REG_GPIOE_PUEN, inpw(REG_GPIOE_PUEN) | BIT11);    /* set GPE11 internal resistor to pull up */
    outpw(REG_GPIOE_DOUT, inpw(REG_GPIOE_DOUT) & (~BIT11)); /* output 0 to GPE11 */
    outpw(REG_GPIOE_OMD,  inpw(REG_GPIOE_OMD)  | BIT11);    /* set GPE11 to OUTPUT mode */
}


/*----------------------------------------------------------------------------
 * Control GPE10 and GPE11 to power on S605 module.
 *      When power on, pull low both RST (GPE10 pin) and PMU (GPE11 pin);
 *      And then, pull high PMU;
 *      Wait 3ms at least, pull high RST;
 *      Keep status 12ms at least;
 *----------------------------------------------------------------------------*/
void S605_power_on()
{
    UINT32 u32Delay;

    sysprintf("Power on S605 module.\n");
    outpw(REG_GPIOE_DOUT, inpw(REG_GPIOE_DOUT) & (~BIT10)); /* pull low RST */
    outpw(REG_GPIOE_DOUT, inpw(REG_GPIOE_DOUT) & (~BIT11)); /* pull low PMU */
    u32Delay = 20000;
    while(u32Delay--);  /* delay 10000 loop ~= 5ms */

    outpw(REG_GPIOE_DOUT, inpw(REG_GPIOE_DOUT) | BIT11);    /* pull high PMU */
    u32Delay = 20000;
    while(u32Delay--);  /* delay 10000 loop ~= 5ms */

    outpw(REG_GPIOE_DOUT, inpw(REG_GPIOE_DOUT) | BIT10);    /* pull hiigh RST */
    u32Delay = 40000;
    while(u32Delay--);  /* delay 10000 loop ~= 5ms */
}
#endif


/*
    The following codes are added to support Linux tag list [2007/03/21]
    currently, only compressed romfs's size and physical address are supported!
*/
int  TAG_create(unsigned int addr, unsigned int size)
{
    static struct tag *tlist;

    tlist = (struct tag *) 0x100; /* will destroy BIB_ShowInfo() */

    //sysprintf("tlist->hdr.tag = ATAG_CORE;\n");
    tlist->hdr.tag = ATAG_CORE;
    tlist->hdr.size = tag_size (tag_core);
    tlist = tag_next (tlist);

    tlist->hdr.tag = ATAG_INITRD2;
    tlist->hdr.size = tag_size (tag_initrd);
    tlist->u.initrd.start = addr;  /* omfs starting address */
    tlist->u.initrd.size = size;   /* romfs size */
    tlist = tag_next (tlist);

//     tag-list node
//     tlist->hdr.tag = ATAG_MACADDR;
//     tlist->hdr.size = tag_size (tag_macaddr);
//     memcpy(&tlist->u.macaddr.mac[0], &_HostMAC[0], 6);
    /*    
    uprintf("===>%02x %02x %02x %02x %02x %02x\n", tlist->u.macaddr.mac[0],
                                                   tlist->u.macaddr.mac[1],
                                                   tlist->u.macaddr.mac[2],
                                                   tlist->u.macaddr.mac[3],
                                                   tlist->u.macaddr.mac[4],
                                                   tlist->u.macaddr.mac[5],
                                                   tlist->u.macaddr.mac[6]);
    */                                                   
    tlist = tag_next (tlist);
    tlist->hdr.tag = ATAG_NONE;
    tlist->hdr.size = 0;
    return 0;
}

volatile int tag_flag = 0, tagaddr,tagsize;
#ifndef __No_LCM__
static UINT32 bIsInitVpost=FALSE;
LCDFORMATEX lcdInfo;

void initVPostShowLogo(void)
{
    if(bIsInitVpost==FALSE)
    {
        bIsInitVpost = TRUE;
        //lcdInfo.ucVASrcFormat = DRVVPOST_FRAME_YCBYCR;
        lcdInfo.ucVASrcFormat = DRVVPOST_FRAME_RGB565;
        lcdInfo.nScreenWidth = PANEL_WIDTH;
        lcdInfo.nScreenHeight = PANEL_HEIGHT;
        vpostLCMInit(&lcdInfo, (UINT32*)FB_ADDR);
        //backLightEnable();
    }
}
#endif
void spuDacOnLoader(UINT8 level)
{
    outp32(REG_AHBCLK, inp32(REG_AHBCLK) &(~( ADO_CKE | SPU_CKE | HCLK4_CKE)));  /* disable SPU engine clock */
    outp32(REG_AHBCLK, inp32(REG_AHBCLK) | ADO_CKE | SPU_CKE | HCLK4_CKE);       /* enable SPU engine clock */

    outp32(REG_SPU_CTRL, inp32(REG_SPU_CTRL) & ~SPURST);

    outp32(REG_SPU_CTRL, inp32(REG_SPU_CTRL) | SPURST);
    outp32(REG_SPU_CTRL, inp32(REG_SPU_CTRL) & ~SPURST);

    outpw(REG_SPU_DAC_PAR, inpw(REG_SPU_DAC_PAR) | 0x30);       /* disable */
    if(level == 3)
        outpw(REG_SPU_DAC_PAR, inpw(REG_SPU_DAC_PAR) & ~0x30);  /* delay time, p0=3s */
    else if(level == 1)
        outpw(REG_SPU_DAC_PAR, inpw(REG_SPU_DAC_PAR) & ~0x20);  /* delay time, p0=0.5-1s */
    else if(level == 2)
        outpw(REG_SPU_DAC_PAR, inpw(REG_SPU_DAC_PAR) & ~0x10);  /* delay time, p0=2s */
    else
    {
        outpw(REG_SPU_DAC_VOL, inpw(REG_SPU_DAC_VOL) & ~0x00FF0000);  /* P7 */
        outpw(REG_SPU_DAC_PAR, inpw(REG_SPU_DAC_PAR) | 0x30);   /* disable */
        return;
    }
#if 0
    if(level == 3)    /* modify this delay time to meet the product request */
        _sysDelay(300);
    else if(level == 1)
        _sysDelay(70);
    else if(level == 2)
        _sysDelay(200);
#endif
}

int main(void)
{
    unsigned int startBlock;
    unsigned int fileLen;
    unsigned int executeAddr,loadAddr;
    unsigned int start_addr, size;

#ifdef __Security__
    UINT8  u8UID[8];
    unsigned char ROOTKey[32];      /* Rootkey array */
    unsigned char HMACKey[32];      /* HMACkey array */
    unsigned char HMACMessage[4];   /* HMAC message data, use for update HMAC key */
    unsigned char Input_tag[12];    /* Input tag data for request content */
    unsigned char RPMCStatus;
#endif
    int count, i, j;
    void  (*fw_func)(void);

    outp32(0xFF000000, 0);

    if(sysGetChipVersion() == 'G')
        outp32(REG_CLKDIV4, inp32(REG_CLKDIV4)| 0x100);

    spuDacOnLoader(2);

    outp32(REG_APBCLK, inp32(REG_APBCLK) | RTC_CKE);

#ifdef __No_RTC__
    sysprintf("* Not Config RTC\n");
    outp32(REG_APBCLK, inp32(REG_APBCLK) & ~RTC_CKE);
#else
    if(inp32(INIR) & 0x1)
    {
        sysprintf("* Enable HW Power Off\n");

        count = 0;

        outp32(REG_APBCLK, inp32(REG_APBCLK) | RTC_CKE);

        outp32(AER,0x0000a965);

        while(1)
        {
            if((inp32(AER) & 0x10000) == 0x10000)
                break;
            if(count > 1000000)
            {
                sysprintf("Write RTC Fail!!\n");
                break;
            }
            count++;
        }
        outp32(PWRON, 0x60005);   /* Press Power Key during 6 sec to Power off (0x'6'0005) */
        outp32(RIIR,0x4);
    }
    else
    {
        if((inp32(INIR) & 0x1) == 0)
        sysprintf("RTC is in-active!!\n");
    }
 
#endif
    init();

#ifdef _S605_   /* support S605 chip */
    S605_init_gpio();
    S605_power_on();
#endif

#ifndef __No_LCM__
    initVPostShowLogo();
#endif
    imagebuf = (UINT8 *)((UINT32)image_buffer | 0x80000000);
    pImageList=((unsigned int *)(((unsigned int)image_buffer)|0x80000000));

    /* Initial DMAC and NAND interface */
    SPI_OpenSPI();
#ifdef __Security__
    if ((RPMC_ReadUID(u8UID)) == -1)
    {
        sysprintf("read id error !!\n");
        return -1;
    }

    sysprintf("SPI flash uid [0x%02X%02X%02X%02X%02X%02X%02X%02X]\n",u8UID[0], u8UID[1],u8UID[2], u8UID[3],u8UID[4], u8UID[5],u8UID[6], u8UID[7]);
  
    /* first stage, initial rootkey */
    RPMC_CreateRootKey((unsigned char *)u8UID,8, ROOTKey);  /* caculate ROOTKey with UID & ROOTKeyTag by SHA256 */

    /* Second stage, update HMACKey after ever power on. without update HMACkey, Gneiss would not function*/
    HMACMessage[0] = rand()%0x100;        /* Get random data for HMAC message, it can also be serial number, RTC information and so on. */
    HMACMessage[1] = rand()%0x100;
    HMACMessage[2] = rand()%0x100;
    HMACMessage[3] = rand()%0x100;

    /* 
        Update HMAC key and get new HMACKey. 
        HMACKey is generated by SW using Rootkey and HMACMessage.
        RPMC would also generate the same HMACKey by HW 
     */
    RPMCStatus = RPMC_UpHMACkey(KEY_INDEX, ROOTKey, HMACMessage, HMACKey);
    if(RPMCStatus == 0x80)
    {
        /* update HMACkey success */
        sysprintf("RPMC_UpHMACkey Success - 0x%02X!!\n",RPMCStatus );
    }
    else
    {
        /* write HMACkey fail, check datasheet for the error bit */
        sysprintf("RPMC_UpHMACkey Fail - 0x%02X!!\n",RPMCStatus );
    }

    /* 
        Third stage, increase RPMC counter
        input tag is send in to RPMC, it could be time stamp, serial number and so on
    */
    for(i= 0; i<12;i++)
        Input_tag[i] = u8UID[i%8];

    RPMCStatus = RPMC_IncCounter(KEY_INDEX, HMACKey, Input_tag);
    if(RPMCStatus == 0x80)
    {
        /* increase counter success */
        sysprintf("RPMC_IncCounter Success - 0x%02X!!\n",RPMCStatus );
    }
    else
    {
        /* increase counter fail, check datasheet for the error bit */
        sysprintf("RPMC_IncCounter Fail - 0x%02X!!\n",RPMCStatus );
        while(1);
    }

    if(RPMC_Challenge(KEY_INDEX, HMACKey, Input_tag)!=0)
    {
        sysprintf("RPMC_Challenge Fail!!\n" );
        /* return signature miss-match */
        while(1);
    }
    else
        sysprintf("RPMC_Challenge Pass!!\n" );
#endif
    memset(imagebuf, 0, 32);
    sysprintf("Load Image ");
    /* read image information */
    SPIReadFast(0, 63*1024, 1024, (UINT32*)imagebuf);  /* offset, len, address */

    if (((*(pImageList+0)) == 0xAA554257) && ((*(pImageList+3)) == 0x63594257))
    {
        count = *(pImageList+1);

        pImageList=((unsigned int*)(((unsigned int)image_buffer)|0x80000000));
        startBlock = fileLen = executeAddr = 0;

        /* load logo first */
        pImageList = pImageList+4;
        for (i=0; i<count; i++)
        {
            if (((*(pImageList) >> 16) & 0xffff) == 4)    /* logo */
            {
                startBlock = *(pImageList + 1) & 0xffff;
                executeAddr = *(pImageList + 2);
                fileLen = *(pImageList + 3);
                SPIReadFast(0, startBlock * 0x10000, fileLen, (UINT32*)executeAddr);
                break;
            }
            /* pointer to next image */
            pImageList = pImageList+12;
        }

        pImageList=((unsigned int*)(((unsigned int)image_buffer)|0x80000000));
        startBlock = fileLen = executeAddr = 0;

        /* load romfs file */
        pImageList = pImageList+4;
        for (i=0; i<count; i++)
        {
            if (((*(pImageList) >> 16) & 0xffff) == 2)    /* RomFS */
            {
                startBlock = *(pImageList + 1) & 0xffff;
                executeAddr = *(pImageList + 2);
                fileLen = *(pImageList + 3);
                SPIReadFast(0, startBlock * 0x10000, fileLen, (UINT32*)executeAddr);
                tag_flag = 1;
                tagaddr = executeAddr;
                tagsize = fileLen;
                break;
            }
            /* pointer to next image */
            pImageList = pImageList+12;
        }

        pImageList=((unsigned int*)(((unsigned int)image_buffer)|0x80000000));
        startBlock = fileLen = executeAddr = 0;

        /* load execution file */
        pImageList = pImageList+4;
        for (i=0; i<count; i++)
        {
            if (((*(pImageList) >> 16) & 0xffff) == 1)    /* execute */
            {
                UINT32 u32Result;
                startBlock = *(pImageList + 1) & 0xffff;
                executeAddr = *(pImageList + 2);
                fileLen = *(pImageList + 3);
                SPIReadFast(0, startBlock * 0x10000, 64, (UINT32*)IMAGE_BUFFER);
                u32Result = do_bootm(IMAGE_BUFFER, 0, CHECK_HEADER_ONLY);

                sysSetGlobalInterrupt(DISABLE_ALL_INTERRUPTS);
                sysSetLocalInterrupt(DISABLE_FIQ_IRQ);
#ifdef __DAC_ON__

                outpw(REG_SPU_DAC_VOL, inpw(REG_SPU_DAC_VOL) & ~0x0800000);  /* P7 */

                start_addr = startBlock * 0x10000;

                size = (fileLen >> 10) << 8;

                if(u32Result)
                    loadAddr = executeAddr;                
                else
                    loadAddr = IMAGE_BUFFER;

                for(j=0;j<4;j++)
                {
                    SPIReadFast(0, start_addr + size * j, size, (UINT32*) (loadAddr + size * j));

                    switch(j)
                    {
                        case 0:;
                            outpw(REG_SPU_DAC_VOL, inpw(REG_SPU_DAC_VOL) & ~0x0400000);    /* P6 */
                            break;
                        case 1:
                            outpw(REG_SPU_DAC_VOL, inpw(REG_SPU_DAC_VOL) & ~0x01e0000);    /* P1-4 */
                            break;
                        case 2:
                            outpw(REG_SPU_DAC_VOL, inpw(REG_SPU_DAC_VOL) & ~0x0200000);    /* P5 */
                            break;
                    }
                }
                fileLen = fileLen - size * 4;
                if(fileLen)
                    SPIReadFast(0, start_addr + size * 4, fileLen, (UINT32*) (loadAddr + size * 4));

                if(u32Result == 0)
                    do_bootm(IMAGE_BUFFER, loadAddr, LOAD_IMAGE);

                outpw(REG_SPU_DAC_VOL, inpw(REG_SPU_DAC_VOL) & ~0x00010000);   /* P0 */

                outpw(REG_SPU_DAC_VOL, inpw(REG_SPU_DAC_VOL) | 0x00001F1F);

                outp32(REG_AHBCLK, inp32(REG_AHBCLK) | ADO_CKE | SPU_CKE | HCLK4_CKE);    /* enable SPU engine clock */
                /* Initial SPU in advance for linux set volume issue */
                spuOpen(eDRVSPU_FREQ_8000);
#else
                if(u32Result)    /* Not compressed */
                    SPIReadFast(0, startBlock * 0x10000, fileLen, (UINT32*)executeAddr);
                else    /* compressed */
                {
                    SPIReadFast(0, startBlock * 0x10000, fileLen, (UINT32*)IMAGE_BUFFER);
                    do_bootm(IMAGE_BUFFER, executeAddr, LOAD_IMAGE);
                }
#endif
                sysSetGlobalInterrupt(DISABLE_ALL_INTERRUPTS);
                sysSetLocalInterrupt(DISABLE_FIQ_IRQ);
                /* Invalid and disable cache */
                sysDisableCache();
                sysInvalidCache();

                if(tag_flag)
                {
                    sysprintf("Create Tag - Address 0x%08X, Size 0x%08X\n",tagaddr,tagsize );
                    TAG_create(tagaddr,tagsize);
                }

                /* JUMP to kernel */
                sysprintf("Jump to kernel\n\n\n");

                //lcmFill2Dark((char *)(FB_ADDR | 0x80000000));
                outp32(REG_AHBIPRST, JPGRST | SICRST |UDCRST | EDMARST);
                outp32(REG_AHBIPRST, 0);
                outp32(REG_APBIPRST, UART1RST | UART0RST | TMR1RST | TMR0RST );
                outp32(REG_APBIPRST, 0);
                sysFlushCache(I_D_CACHE);
                fw_func = (void(*)(void))(executeAddr);
                fw_func();
                break;
            }
            /* pointer to next image */
            pImageList = pImageList+12;
        }
    }

    return(0); /* avoid compilation warning */
}
