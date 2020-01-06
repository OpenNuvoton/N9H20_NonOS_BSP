#include <string.h>
#include "wblib.h"
#include "turbowriter.h"
#include "N9H20_VPOST.h"

// define DATE CODE and show it when running to make maintaining easy.
#ifdef _S605_
    #define DATE_CODE   "20191218 for S605"
#else
    #define DATE_CODE   "20191218"
#endif

/* global variable */
typedef struct sd_info
{
    unsigned int startSector;
    unsigned int endSector;
    unsigned int fileLen;
    unsigned int executeAddr;
} NVT_SD_INFO_T;
extern void spuDacOn(UINT8 level);


INT MoveData(NVT_SD_INFO_T *image, BOOL IsExecute)
{
    int volatile sector_count;
    void    (*fw_func)(void);

    sysprintf("Load file length 0x%x, execute address 0x%x\n", image->fileLen, image->executeAddr);

    // read SD card
    sector_count = image->fileLen / 512;
    if ((image->fileLen % 512) != 0)
        sector_count++;

    fmiSD_Read(image->startSector, sector_count, (UINT32)image->executeAddr);

    if (IsExecute == TRUE)
    {
        outpw(REG_SDISR, 0x0000FFFF);
        outpw(REG_FMIIER, 0);
        outpw(REG_SDIER, 0);
        outpw(REG_FMICR, 0);

        // SD-0 pin dis-selected
        outpw(REG_GPEFUN, inpw(REG_GPEFUN)&(~0x0000FFF0));  // SD0_CLK/CMD/DAT0_3 pins dis-selected

        // disable SD Card Host Controller operation and driving clock.
        outpw(REG_AHBCLK, inpw(REG_AHBCLK) & ~SD_CKE);
        outpw(REG_AHBCLK, inpw(REG_AHBCLK) & ~SIC_CKE);

        sysprintf("SD Boot Loader exit. Jump to execute address 0x%x ...\n", image->executeAddr);

        fw_func = (void(*)(void))(image->executeAddr);
        fw_func();
    }
    return 0;
}

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
        outp32(REG_SDTIME, 0x21667525);     // for S605 with Winbond 32MB DDR2 under 96MHz HCLK
    }
    else if((inp32(REG_CHIPCFG) & SDRAMSEL) == 0x30)     // Power On Setting SDRAM type is DDR
    {
    #ifdef __DDR_75__
        outp32(REG_SDTIME, 0x098E7549); // DDR Speed grade-75
    #endif
    #ifdef __DDR_6__
        //outp32(REG_SDTIME, 0x094E7425); // DDR Speed grade-6, for N9H20K5 with 32MB DDR-6 under 96MHz HCLK
        // For N9H20K1/N9H20K3 must use
		outp32(REG_SDTIME, 0x29AC8525);
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
#endif  // end of __UPLL_192__
}


#ifdef _S605_   // support S605 chip
//----------------------------------------------------------------------------
// Initail GPIO pins GPE10 and GPE11 to output mode for S605 module.
//----------------------------------------------------------------------------
void S605_init_gpio()
{
    sysprintf("Initial GPIO pins for S605 module.\n");

    // initial GPE10 to output mode and pull low it.
    outpw(REG_GPEFUN, inpw(REG_GPEFUN) & (~MF_GPE10));      // set GPE10 as GPIO pin
    outpw(REG_GPIOE_PUEN, inpw(REG_GPIOE_PUEN) | BIT10);    // set GPE10 internal resistor to pull up
    outpw(REG_GPIOE_DOUT, inpw(REG_GPIOE_DOUT) & (~BIT10)); // output 0 to GPE10
    outpw(REG_GPIOE_OMD,  inpw(REG_GPIOE_OMD)  | BIT10);    // set GPE10 to OUTPUT mode

    // initial GPE11 to output mode and pull low it.
    outpw(REG_GPEFUN, inpw(REG_GPEFUN) & (~MF_GPE11));      // set GPE11 as GPIO pin
    outpw(REG_GPIOE_PUEN, inpw(REG_GPIOE_PUEN) | BIT11);    // set GPE11 internal resistor to pull up
    outpw(REG_GPIOE_DOUT, inpw(REG_GPIOE_DOUT) & (~BIT11)); // output 0 to GPE11
    outpw(REG_GPIOE_OMD,  inpw(REG_GPIOE_OMD)  | BIT11);    // set GPE11 to OUTPUT mode
}


//----------------------------------------------------------------------------
// Control GPE10 and GPE11 to power on S605 module.
//      When power on, pull low both RST (GPE10 pin) and PMU (GPE11 pin);
//      And then, pull high PMU;
//      Wait 3ms at least, pull high RST;
//      Keep status 12ms at least;
//----------------------------------------------------------------------------
void S605_power_on()
{
    UINT32 u32Delay;

    sysprintf("Power on S605 module.\n");
    outpw(REG_GPIOE_DOUT, inpw(REG_GPIOE_DOUT) & (~BIT10)); // pull low RST
    outpw(REG_GPIOE_DOUT, inpw(REG_GPIOE_DOUT) & (~BIT11)); // pull low PMU
    u32Delay = 20000;
    while(u32Delay--);  // delay 10000 loop ~= 5ms

    outpw(REG_GPIOE_DOUT, inpw(REG_GPIOE_DOUT) | BIT11);    // pull high PMU
    u32Delay = 20000;
    while(u32Delay--);  // delay 10000 loop ~= 5ms

    outpw(REG_GPIOE_DOUT, inpw(REG_GPIOE_DOUT) | BIT10);    // pull hiigh RST
    u32Delay = 40000;
    while(u32Delay--);  // delay 10000 loop ~= 5ms
}
#endif


UINT8 dummy_buffer[512];
#if defined (__GNUC__)
    UINT8 dummy_buffer[512] __attribute__((aligned (32)));
#else
    __align(32) UINT8 dummy_buffer[512];
#endif

unsigned char *buf;
unsigned int *pImageList;
int main()
{
    NVT_SD_INFO_T image;
    int count, i;
    WB_UART_T uart;
    E_SYS_SRC_CLK eSrcClk;
    UINT32 u32PllKHz, u32SysKHz, u32CpuKHz, u32HclkKHz, u32ApbKHz;
    int ibr_boot_sd_port;

    /* Clear Boot Code Header in SRAM to avoid booting fail issue */
    outp32(0xFF000000, 0);

    spuDacOn(2);

    initClock();
#ifdef _DEBUG_AIC_
    initTimer();
#endif

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
		sysUartPort(1);
    uart.uiFreq = 12000000; //use APB clock
    uart.uiBaudrate = 115200;
    uart.uiDataBits = WB_DATA_BITS_8;
    uart.uiStopBits = WB_STOP_BITS_1;
    uart.uiParity = WB_PARITY_NONE;
    uart.uiRxTriggerLevel = LEVEL_1_BYTE;
    sysInitializeUART(&uart);

    sysprintf("N9H20 SD Boot Loader entry (%s).\n", DATE_CODE);

#ifdef __DISABLE_RTC__
    sysprintf("Disable RTC feature.\n");
#endif

    sysGetSystemClock(&eSrcClk, &u32PllKHz, &u32SysKHz, &u32CpuKHz, &u32HclkKHz, &u32ApbKHz);
    sysprintf("System clock = %dKHz\nAHB clock = %dKHz\nREG_SDTIME = 0x%08X\n",
               u32SysKHz, u32HclkKHz, inp32(REG_SDTIME));

#ifdef _S605_   // support S605 chip
    S605_init_gpio();
    S605_power_on();
#endif

    buf = (UINT8 *)((UINT32)dummy_buffer | 0x00000000);
    pImageList=((unsigned int *)(((unsigned int)dummy_buffer)|0x80000000));

    // IBR keep the booting SD port number on register SDCR.
    // SDLoader should load image from same SD port.
    ibr_boot_sd_port = (inpw(REG_SDCR) & SDCR_SDPORT) >> 29;

    /* Initial DMAC and NAND interface */
    fmiInitDevice();

    sysprintf("Boot from SD%d ...\n", ibr_boot_sd_port);
    i = fmiInitSDDevice(ibr_boot_sd_port);
    if (i < 0)
        sysprintf("SD%d init fail <%d>\n", ibr_boot_sd_port, i);

    memset((char *)&image, 0, sizeof(NVT_SD_INFO_T));

    /* read image information */
    fmiSD_Read(33, 1, (UINT32)buf);

    if (((*(pImageList+0)) == 0x574255aa) && ((*(pImageList+3)) == 0x57425963))
    {
        count = *(pImageList+1);

        /* logo */
        pImageList = pImageList+4;
        for (i=0; i<count; i++)
        {
            if (((*(pImageList) >> 16) & 0xffff) == 4)  // logo
            {
                image.startSector = *(pImageList + 1) & 0xffff;
                image.endSector = (*(pImageList + 1) & 0xffff0000) >> 16;
                image.executeAddr = *(pImageList + 2);
                image.fileLen = *(pImageList + 3);
                MoveData(&image, FALSE);
                break;
            }
            /* pointer to next image */
            pImageList = pImageList+12;
        }

        pImageList = ((unsigned int*)(((unsigned int)dummy_buffer)|0x80000000));
        memset((char *)&image, 0, sizeof(NVT_SD_INFO_T));
        /* execution file */
        pImageList = pImageList+4;
        for (i=0; i<count; i++)
        {
            if (((*(pImageList) >> 16) & 0xffff) == 1)  // execute
            {
                image.startSector = *(pImageList + 1) & 0xffff;
                image.endSector = (*(pImageList + 1) & 0xffff0000) >> 16;
                image.executeAddr = *(pImageList + 2);
                image.fileLen = *(pImageList + 3);
                MoveData(&image, TRUE);
                break;
            }
            /* pointer to next image */
            pImageList = pImageList+12;
        }
        while(1);
    }
    return 0;
}
