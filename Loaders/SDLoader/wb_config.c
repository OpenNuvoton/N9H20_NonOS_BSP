
/***************************************************************************
 *                                                                         *
 * Copyright (c) 2008 Nuvoton Technolog. All rights reserved.              *
 *                                                                         *
 ***************************************************************************/
/****************************************************************************
 *
 * FILENAME : wb_config.c
 *
 * VERSION  : 1.1
 *
 * DESCRIPTION :
 *               PLL control functions of Nuvoton ARM9 MCU
 *
 * HISTORY
 *
 *
 *      IBR set clocks default value
 *          UPLL= 240MHz
 *          SYS = 120MHz
 *          CPU = 60MHz
 *          HCLK = 60MHz
 *
 *
 *
 *
 ****************************************************************************/
#include <string.h>
#include "wblib.h"
#if 0
WB_PLL_T            _sysClock;
WB_CLKFREQ_T    _sysFreq;  //MHz, added on [2009/03/11]
#endif
//static            _sysClockIniFlag = FALSE;
static UINT32 g_u32SysClkSrc;
static UINT32 g_u32UpllKHz = 240000, g_u32ApllKHz=240000, g_u32SysKHz = 120000, g_u32CpuKHz = 60000, g_u32HclkKHz = 60000;
static UINT32 g_u32Hclk1KHz, g_u32ApbKHz;
static UINT32 g_u32REG_APLL, g_u32REG_UPLL;
static UINT32 g_u32SysDiv, g_u32CpuDiv, g_u32ApbDiv;
BOOL bIsCpuOver2xHclk = FALSE;
static UINT32 g_u32ExtClk = 27000;

//extern UINT8  _tmp_buf[];
UINT8  _tmp_buf[PD_RAM_SIZE];

//#define DBG_PRINTF        sysprintf
#define DBG_PRINTF(...)
void InitDelay(void)
{

}
void sysInitDDR(void)
{
    UINT32 u32Delay;
    outp32( 0xb0000224, 0x00000E6E);
    outp32( 0xb0000220, 0x1008CE6E);
    outp32( 0xb000020C, 0x00000019);
    outp32( 0xb0003030, 0x00001010);
    outp32( 0xb0003010, 0x00000005);
    outp32( 0xb0003004, 0x00000021);
    outp32( 0xb0003004, 0x00000023);
    outp32( 0xb0003004, 0x00000027);
    while(inp32(0xb0003004) & BIT2);
    outp32( 0xb000301C, 0x00001002);
    outp32( 0xb0003018, 0x00000122);
    outp32( 0xb0003004, 0x00000027);
    while(inp32(0xb0003004) & BIT2);
    outp32( 0xb0003004, 0x0000002B);
    while(inp32(0xb0003004) & BIT3);
    outp32( 0xb0003004, 0x0000002B);
    while(inp32(0xb0003004) & BIT3);
    outp32( 0xb0003018, 0x00000022);
    u32Delay=250;
    while(u32Delay--);
    outp32( 0xb0003004, 0x00000020);
    outp32( 0xb0003034, 0x00AAAA00);
    outp32( 0xb0003008, 0x000080C0);
    outp32( 0xb00000A0, 0x00000000);
}
void sysInitDDRStart(void)
{
    UINT32   vram_base, aic_status = 0;
    VOID    (*wb_func)(void);
    aic_status = inpw(REG_AIC_IMR);                 //Disable interrupt
    outpw(REG_AIC_MDCR, 0xFFFFFFFF);
/*
void sysInitDDR(void)
*/
    vram_base = PD_RAM_BASE;
    memcpy((char *)_tmp_buf, (char *)vram_base, PD_RAM_SIZE);                   //Backup RAM content
    memcpy((VOID *)vram_base,(VOID *)sysInitDDR, PD_RAM_SIZE);      //


    // Entering clock switch function
    wb_func = (void(*)(void)) vram_base;
    DBG_PRINTF("Jump to SRAM\n");
    wb_func();
    //wb_func(eSrcClk, u32Hclk, bIsUp2Hclk3X, u32SysDiv);
    //Restore VRAM
    memcpy((VOID *)vram_base, (VOID *)_tmp_buf, PD_RAM_SIZE);

    outpw(REG_AIC_MDCR, 0xFFFFFFFF);        // Disable all interrupt
    outpw(REG_AIC_MECR, aic_status);        // Restore AIC setting
}
/*-----------------------------------------------------------------------------------------------------------
 *
 * Function : sysInitMemory
 *
 * DESCRIPTION :
 *               According to the external clock and expected PLL output clock to get the content of PLL controll register
 *
 * Parameters
 *              eSysPll : eSYS_APLL or eSYS_UPLL
 *          u32FinKHz: External clock. Unit: KHz
 * Return
 *          PLL clock.
 * HISTORY
 *               2010-07-15
 *
-----------------------------------------------------------------------------------------------------------*/
UINT32 sysInitMemory(void)
{
    sysInitDDRStart();
    return 0;
}
/*-----------------------------------------------------------------------------------------------------------
 *
 * Function : sysGetPLLOutputKhz
 *
 * DESCRIPTION :
 *               According to the external clock and expected PLL output clock to get the content of PLL controll register
 *
 * Parameters
 *              eSysPll : eSYS_APLL or eSYS_UPLL
 *          u32FinKHz: External clock. Unit: KHz
 * Return
 *          PLL clock.
 * HISTORY
 *               2010-07-15
 *
-----------------------------------------------------------------------------------------------------------*/
UINT32 sysGetPLLOutputKhz(
    E_SYS_SRC_CLK eSysPll,
    UINT32 u32FinKHz
    )
{
    UINT32 u32Freq, u32PllCntlReg;
    UINT32 NF, NR, NO;
    UINT8 au8Map[4] = {1, 2, 2, 4};


    if(eSysPll==eSYS_APLL)
        u32PllCntlReg = inp32(REG_APLLCON);
    else if(eSysPll==eSYS_UPLL)
        u32PllCntlReg = inp32(REG_UPLLCON);

    if(u32PllCntlReg&0x10000)           //PLL power down.
        return 0;

    NF = (u32PllCntlReg&FB_DV)+2;
    NR = ((u32PllCntlReg & IN_DV)>>9)+2;
    NO = au8Map[((u32PllCntlReg&OUT_DV)>>14)];
    DBG_PRINTF("PLL regster = 0x%x\n", u32PllCntlReg);
    DBG_PRINTF("NF = %d\n", NF);
    DBG_PRINTF("NR = %d\n", NR);
    DBG_PRINTF("NO = %d\n", NO);

    u32Freq = u32FinKHz*NF/NR/NO;
    DBG_PRINTF("PLL Freq = %d\n", u32Freq);
    return u32Freq;
}

volatile BOOL bIsCheckPllConstraint = FALSE;
void sysCheckPllConstraint(BOOL bIsCheck)
{
    bIsCheckPllConstraint = bIsCheck;
}

/*-----------------------------------------------------------------------------------------------------------
 *
 * Function : sysGetPLLControlRegister
 *
 * DESCRIPTION :
 *               According to the external clock and expected PLL output clock to get the content of PLL controll register
 *
 * Parameters
 *              u32FinKHz : External clock.  Unit:KHz
 *          u32TargetKHz: PLL output clock. Unit:KHz
 * Return
 *                  0 : No any fit value for the specified PLL output clock
 *          PLL control register.
 * HISTORY
 *               2010-07-15
 *
-----------------------------------------------------------------------------------------------------------*/
UINT32 sysGetPLLControlRegister(UINT32 u32FinKHz, UINT32 u32TargetKHz)
{
    unsigned int u32OUT_DV, u32IN_DV, u32FB_DV;
    unsigned int u32NR, u32NF, u32NO;
    unsigned int au32Array[4] = {1 , 2, 2, 4};
    unsigned u32Target;
    unsigned int u23Register=0;

    do
    {
        for(u32OUT_DV =0 ; u32OUT_DV<4; u32OUT_DV=u32OUT_DV+1)
        {
            for(u32IN_DV =0 ; u32IN_DV<32; u32IN_DV=u32IN_DV+1)
            {
                for(u32FB_DV =0 ; u32FB_DV<512; u32FB_DV=u32FB_DV+1)
                {
                    u32NR =  (2 * (u32IN_DV + 2));
                    u32NF = (2 * (u32FB_DV + 2));
                    u32NO = au32Array[u32OUT_DV];
                    if( (u32FinKHz/u32NR)<1000 )
                        continue;
                    if( (u32FinKHz/u32NR)>15000)
                        continue;
                    if( ((u32FinKHz/u32NR)*u32NF) <100000)
                        continue;
                    if( ((u32FinKHz/u32NR)*u32NF) >500000)
                        continue;
                    u32Target = u32FinKHz*u32NF/u32NR/u32NO;
                    if(u32TargetKHz==u32Target)
                    {
                        u23Register = (u32OUT_DV<<14) | (u32IN_DV<<9) | (u32FB_DV);
                        return u23Register;
                    }
                }
            }
        }
        u32TargetKHz = u32TargetKHz -4000;      //- 4MHz
    }while((u23Register==0));

    return 0;
}
/*-----------------------------------------------------------------------------------------------------------
* Function: sysSetPLLControlRegister
*
* Parameters:
*              u32PllValue - [in], PLL setting value
*
* Returns:
*      None
*
* Description:
*              To set the PLL control register.
*
-----------------------------------------------------------------------------------------------------------*/
void
sysSetPLLControlRegister(
    E_SYS_SRC_CLK eSysPll,
    UINT32 u32PllValue
    )
{
    if(eSysPll==eSYS_APLL)
        outp32(REG_APLLCON, u32PllValue);
    else if(eSysPll==eSYS_APLL)
        outp32(REG_UPLLCON, u32PllValue);
}
/*
    1. Refresh rate base on the target
    Check the low freq bit in SDRAM controller whether need set ot not for DDR. ???????????
    refresh rate = REPEAT/Fmclk
*/
void sysExternalClock(void)
{
    /*Refresh rate is always from external clock */
    //UINT32 u32Repeat;
    //u32Repeat = 8*100;                                        //Max HCLK is 100MHz
    //outp32(REG_SDREF, (inp32(REG_SDREF) & ~REFRATE) | u32Repeat); //Set new refresh rate

    outp32(REG_PWRCON, (inp32(REG_PWRCON) & ~UP2HCLK3X));       //Clear anyway no matter CPU/HCLK>2x
    outp32(REG_CLKDIV0, inp32(REG_CLKDIV0) & ~(SYSTEM_N1 | SYSTEM_S | SYSTEM_N0)); //System clock from external and divider as 0.
    outp32(REG_CLKDIV4, inp32(REG_CLKDIV4) & ~(CPU_N) );
}
/*-----------------------------------------------------------------------------------------------------------
* Function: sysSetSystemClock
*
* Parameters:
*              u32PllValue - [in], PLL setting value
*
* Returns:
*      None
*
* Description:
*              To set the PLL control register.
*
*Note:
*       Switch systetm clock to external clock first. Remember to adjust the refresh rate before switch to external clock
*
*       refresh rate = REPEAT/Fmclk
*
*       1. Switch to external clock
*           a. Change refresh base on PLL value. Although the value more power sumption
*
*
*
*
*
*
-----------------------------------------------------------------------------------------------------------*/
void sysClockSwitch(register E_SYS_SRC_CLK eSrcClk,
                        register UINT32 u32PllReg,
                        register UINT32 u32HclkKHz,
                        //register BOOL bIsUp2Hclk3X,
                        register UINT32 u32SysDiv,
                        register UINT32 u32CpuDiv,
                        register UINT32 u32ApbDiv)
{
    UINT32 u32IntTmp;
    // disable interrupt (I will recovery it after clock changed)
    u32IntTmp = inp32(REG_AIC_IMR);
    outp32(REG_AIC_MDCR, 0xFFFFFFFE);

    //It is necessasy to Enable/Disable Low Freq prior to self-refresh mode
    if( (inp32(REG_CHIPCFG)&SDRAMSEL) == 0x20){//DDR2 will always disable DLL. Due to DLL enable only HCLK>133MHz.
            outp32(REG_SDEMR, inp32(REG_SDEMR)  | DLLEN); //DDR2 will always disable DLL due to HCLK always less than 133MHz.
            if(u32HclkKHz<96000)
                outp32(REG_SDOPM, inp32(REG_SDOPM)  | LOWFREQ); //Enable Low Freq
            else
                outp32(REG_SDOPM, inp32(REG_SDOPM)  & ~LOWFREQ); //Enable Low Freq

    }

    __asm
    {
        mov         r2, #100
        mov     r1, #0
        mov     r0, #1
    loop0:  add         r1, r1, r0
        cmp         r1, r2
        bne     loop0
    }


    outp32(REG_SDCMD, inp32(REG_SDCMD) | (AUTOEXSELFREF| REF_CMD));//DRAM enter self refresh mode

    if(eSrcClk==eSYS_EXT)
    {
        //Disable DLL of SDRAM device /*05-17*/
        outp32(REG_CLKDIV0, (inp32(REG_CLKDIV0) & ~SYSTEM_N0) | u32SysDiv );

        if( (inp32(REG_CHIPCFG)&SDRAMSEL) != 00)
        {//SDRAM unnecessary
            outp32(REG_SDEMR, inp32(REG_SDEMR)  | DLLEN);
            outp32(REG_SDOPM, inp32(REG_SDOPM) | LOWFREQ);
        }
        outp32(REG_CLKDIV0, (inp32(REG_CLKDIV0) & (~0xFF))); //Switch to external clock and divider to 0
    }
    else if(eSrcClk==eSYS_UPLL)
    {


        //outp32(REG_UPLLCON, u32PllReg);
        outp32(REG_CLKDIV0,  (inp32(REG_CLKDIV0) | 0x02));  //Safe consider
        outp32(REG_UPLLCON, u32PllReg);
        __asm
        {
            mov         r2, #1000
            mov     r1, #0
            mov     r0, #1
        loop1:  add         r1, r1, r0
            cmp         r1, r2
            bne     loop1
        }
        outp32(REG_CLKDIV0, (inp32(REG_CLKDIV0) & (~0xF1F)) | ((eSrcClk<<3) | u32SysDiv));
        //outp32(0xb0000200, inp32(0xb0000200) | ((bIsUp2Hclk3X&1)<<5));
        //outp32(REG_CLKDIV4,  (inp32(REG_CLKDIV4) & ~0x0F)| u32CpuDiv );
        outp32(REG_CLKDIV4,  (inp32(REG_CLKDIV4) & ~0xF0F)| (u32CpuDiv| (u32ApbDiv<<8) ));
        __asm
        {
            mov         r2, #1000
            mov     r1, #0
            mov     r0, #1
        loop2:  add         r1, r1, r0
            cmp         r1, r2
            bne     loop2
        }

    }
    else if (eSrcClk==eSYS_APLL)
    {
        if ((inp32(REG_CLKDIV0) & (0x18))==0x18)
        {//From UPLL -->APLL

            //outp32(REG_CLKDIV0, (inp32(REG_CLKDIV0) | (0x02))); //Safe consider
            outp32(REG_CLKDIV0, (inp32(REG_CLKDIV0) & (~0xFF))); //Switch to external clock and divider to 0

            //APB = CPU/2
            outp32(REG_CLKDIV4, 0x00000100);
            // enable APLL
            outp32(REG_APLLCON, 0x00F8801C);    //outp32(0xb0000220, 0x00F0801C);
            __asm
            {
                mov         r2, #500
                mov     r1, #0
                mov     r0, #1
            loop3:  add         r1, r1, r0
                cmp         r1, r2
                bne     loop3
            }
            #if 0
            for (u32Count=0; u32Count<50; u32Count=u32Count+1)
            {
                outp32(REG_APLLCON, (0x00F88000 | u32PllReg));
            }
            #else
            __asm
            {
                mov     r0, #0xb0000000
                mov     r1, #0x00F80000
                orr     r1, r1, u32PllReg
                mov     r2, #5
            loop3_1:
                str     r1, [r0, 0x220]
                sub     r2, r2, #1
                cmp     r2, #0
                bne     loop3_1
            }
            #endif


            __asm
            {
                mov         r2, #500
                mov     r1, #0
                mov     r0, #1
            loop4:  add         r1, r1, r0
                cmp         r1, r2
                bne     loop4
            }

            //outp32(REG_CLKDIV0, 0x12);

            __asm
            {
                mov         r2, #1000
                mov     r1, #0
                mov     r0, #1
            loop5:  add         r1, r1, r0
                cmp         r1, r2
                bne     loop5
            }
        }
        else
        {//From APLL -->APLL

            outp32(REG_CLKDIV0, (inp32(REG_CLKDIV0) & (~0xFF))); //Switch to external clock and divider to 0
            outp32(REG_APLLCON, (0x00F80000 | u32PllReg));
            __asm
            {
                mov         r2, #1000
                mov     r1, #0
                mov     r0, #1
            loop6:  add         r1, r1, r0
                cmp         r1, r2
                bne     loop6
            }
        }
        outp32(REG_CLKDIV0, (inp32(REG_CLKDIV0) & (~0xF1F)) | ((eSrcClk<<3) | u32SysDiv));
        //outp32(REG_PWRCON, inp32(REG_PWRCON) | ((bIsUp2Hclk3X&1)<<5));
        outp32(REG_CLKDIV4,  (inp32(REG_CLKDIV4) & ~0xF0F)| (u32CpuDiv| (u32ApbDiv<<8) ));
    }
    __asm
    {
        mov         r2, #1000
        mov     r1, #0
        mov     r0, #1
    loop7:  add         r1, r1, r0
        cmp         r1, r2
        bne     loop7
    }
    outp32(REG_SDCMD, inp32(REG_SDCMD) & ~REF_CMD);             //DRAM escape self refresh mode
    __asm
    {
        mov         r2, #1000
        mov     r1, #0
        mov     r0, #1
        loop8:  add         r1, r1, r0
        cmp         r1, r2
        bne     loop8
    }
    // enable interrupt (recovery origianl interrupt maask)
    outp32(REG_AIC_MECR, u32IntTmp);
}
void sysClockSwitchStart(E_SYS_SRC_CLK eSrcClk,
                        UINT32 u32PllReg,
                        UINT32 u32Hclk,
                        //BOOL bIsUp2Hclk3X,
                        UINT32 u32SysDiv,
                        UINT32 u32CpuDiv,
                        UINT32 u32ApbDiv)
{
    UINT32   vram_base, aic_status = 0;
    VOID    (*wb_func)(E_SYS_SRC_CLK,
                    UINT32,
                    UINT32,
                    //BOOL,
                    UINT32,
                    UINT32,
                    UINT32);
    aic_status = inpw(REG_AIC_IMR);                 //Disable interrupt
    outpw(REG_AIC_MDCR, 0xFFFFFFFF);

    vram_base = PD_RAM_BASE;
    memcpy((char *)((UINT32)_tmp_buf | 0x80000000),
            (char *)((UINT32)PD_RAM_BASE | 0x80000000),
            PD_RAM_SIZE);                   //Backup RAM content
    memcpy((VOID *)((UINT32)vram_base | 0x80000000),
            //(VOID *)((UINT32)sysClockSwitch | 0x80000000),
            (VOID *)( ((UINT32)sysClockSwitch -(PD_RAM_START-PD_RAM_BASE)) | 0x80000000),
            PD_RAM_SIZE);                   //
    vram_base = PD_RAM_START;
    wb_func = (void(*)(E_SYS_SRC_CLK,
                    UINT32,
                    UINT32,
                    UINT32,
                    UINT32,
                    UINT32)) vram_base;

    DBG_PRINTF("SYS_DIV = %x\n", u32SysDiv);
    DBG_PRINTF("CPU_DIV = %x\n", u32CpuDiv);
    DBG_PRINTF("APB_DIV = %x\n", u32ApbDiv);
    DBG_PRINTF("Jump to SRAM\n");
    wb_func(eSrcClk,
            u32PllReg,
            u32Hclk,
            u32SysDiv,
            u32CpuDiv,
            u32ApbDiv);


    //Restore VRAM
    vram_base = PD_RAM_BASE;
    memcpy((VOID *)((UINT32)vram_base | 0x80000000),
            (VOID *)((UINT32)_tmp_buf | 0x80000000),
            PD_RAM_SIZE);

    outpw(REG_AIC_MDCR, 0xFFFFFFFF);        // Disable all interrupt
    outpw(REG_AIC_MECR, aic_status);        // Restore AIC setting
}

BOOL bIsAPLLInitialize = FALSE;

UINT32 sysGetExternalClock(void)
{
    if( sysGetChipVersion() == 'G')
        g_u32ExtClk = 12000;
    else
    {
        if((inp32(REG_CHIPCFG) & 0xC) == 0x8)
            g_u32ExtClk = 12000;
        else
            g_u32ExtClk = 27000;
    }
    return g_u32ExtClk;
}
/*-----------------------------------------------------------------------------------------------------------
*   The Function set the relative system clock. PLL, SYS,CPU, HCLK, APB
*   And if specified PLL not meet some costraint, the funtion will search the near frequency and not over the specified frequency
*
*   Paramter:
*       eSrcClk: eSYS_UPLL or eSYS_APLL
*       u32PllKHz: Target PLL clock as system clock source. The frequency should not over 200MHz/240Mhz.
*       u32SysKHz: System clock. The frequency should not over 200MHz/240MHz. Designer suggestion set it below 200MHz.
*       u32CpuKHz: The Frequency must equal to HCLK clock or max equal to 2HCLK.
*       u32HclkKHz: The Frequency must equal to CPU clock or equal to CPU/2. It is the memory clock. Max to 133MHz
*       u32ApbKHz: APB clock.
*   Return:
*       Error code or Successful
-----------------------------------------------------------------------------------------------------------*/
UINT32
sysSetSystemClock(
    E_SYS_SRC_CLK eSrcClk,
    UINT32 u32PllKHz,
    UINT32 u32SysKHz,
    UINT32 u32CpuKHz,
    UINT32 u32HclkKHz,
    UINT32 u32ApbKHz
    )
{
    INT32 i32Div;
    UINT32 u32SysSrc;
    INT32 i32Idx;
    UINT32 u32RegPll;
    register UINT32 u32SysDiv, u32CpuDiv;

    g_u32ExtClk = sysGetExternalClock();

    if(eSrcClk != eSYS_EXT)
    {
        i32Div = u32PllKHz%u32SysKHz;
        i32Div = u32PllKHz/u32SysKHz-1+(i32Div?1:0);
        if(u32SysKHz>u32PllKHz)
        {
            DBG_PRINTF("SYS clock > PLL ckock\n");
            return E_ERR_CLK;
        }
        if(u32CpuKHz>u32SysKHz)
        {
            DBG_PRINTF("CPU clock > SYS ckock\n");
            return E_ERR_CLK;
        }
        if(u32HclkKHz>u32SysKHz)
        {
            DBG_PRINTF("HCLK clock > SYS ckock\n");
            return E_ERR_CLK;
        }
        if(u32ApbKHz>u32HclkKHz)
        {
            DBG_PRINTF("APB clock > HCLK ckock\n");
            return E_ERR_CLK;
        }

        if(u32CpuKHz <= (u32HclkKHz*2))
        {
            bIsCpuOver2xHclk = 0;
            DBG_PRINTF("CPU/HCLK clock rate <2\n");
        }
        else
        {
            bIsCpuOver2xHclk = 1;
            DBG_PRINTF("2< CPU/HCLK clock rate <3\n");
        }
        i32Div =  u32PllKHz%u32SysKHz;
        i32Div =  u32PllKHz/u32SysKHz-1+(i32Div?1:0);
        g_u32SysDiv = i32Div;
        g_u32SysKHz = u32PllKHz/ (g_u32SysDiv+1);                   //SYS clock
        DBG_PRINTF("SYS clock = %d\n", g_u32SysKHz);

        if(bIsCpuOver2xHclk==1)
        {
            g_u32HclkKHz = g_u32SysKHz/6;                       //HCLK clock
            DBG_PRINTF("HCLK clock = %d\n", g_u32HclkKHz);

            i32Div =  g_u32SysKHz%u32CpuKHz;
            i32Div =  g_u32SysKHz/u32CpuKHz-1 + (i32Div ? 1:0);
            if(i32Div==0)                                       //i32Div must be 1 at least if bIsCpuOver2xHclk=1
                i32Div = 1;
            if(i32Div>5)
                i32Div = 5;                                 //i32Div must be 5 at most if bIsCpuOver2xHclk=1
            g_u32CpuDiv = i32Div;
            g_u32CpuKHz = g_u32SysKHz/(g_u32CpuDiv+1);          //CPU clock
            DBG_PRINTF("CPU clock = %d\n", g_u32CpuKHz);

            i32Div =  6;                                        //HCLK1's DIV = 6 if    bIsCpuOver2xHclk=1
            g_u32Hclk1KHz = g_u32SysKHz/i32Div;                 //Remember HCLK1 DIV does not plus 1. ?????
            DBG_PRINTF("HCLK1 clock = %d\n", g_u32Hclk1KHz);

            i32Idx = g_u32SysKHz/u32ApbKHz;
            g_u32ApbDiv = i32Idx/i32Div;
            if(g_u32ApbDiv>0)
                g_u32ApbDiv= g_u32ApbDiv-1;

            g_u32ApbKHz = g_u32SysKHz/((g_u32ApbDiv+1)*i32Div);
            DBG_PRINTF("APB clock = %d\n", g_u32ApbKHz);
        }
        else
        {
            g_u32HclkKHz = g_u32SysKHz/2;                       //HCLK clock
            DBG_PRINTF("HCLK clock = %d\n", g_u32HclkKHz);

            i32Div =  g_u32SysKHz%u32CpuKHz;
            i32Div =  g_u32SysKHz/u32CpuKHz-1 + (i32Div ? 1:0);

            g_u32CpuDiv = i32Div;

            //Note
            if(g_u32CpuDiv>1)
            {
                DBG_PRINTF("CPU divider greate than 1\n");
                return E_ERR_CLK;
            }

            g_u32CpuKHz = g_u32SysKHz/(i32Div+1);               //CPU clock
            DBG_PRINTF("CPU clock = %d\n", g_u32CpuKHz);

            i32Div =  (i32Div > 2)?i32Div:2;                        //HCLK1's DIV = MAX(CPU_N, 2)
            i32Idx = g_u32SysKHz/u32ApbKHz;
            g_u32ApbDiv = i32Idx/i32Div-1;
            g_u32ApbKHz = g_u32SysKHz/((g_u32ApbDiv+1)*i32Div);
            DBG_PRINTF("APB clock = %d\n", g_u32ApbKHz);

        }
    }

    switch(eSrcClk)
    {
        case eSYS_EXT:
            g_u32SysClkSrc =  eSYS_EXT;
            u32SysSrc = g_u32ExtClk;
             DBG_PRINTF("Switch to external \n");
             break;
        case eSYS_X32K:
             g_u32SysClkSrc = eSYS_X32K;
             u32SysSrc = 32;
             break;
        case eSYS_APLL:
            g_u32SysClkSrc = eSYS_APLL;
            u32SysSrc = u32PllKHz;
            g_u32ApllKHz = u32PllKHz;
            g_u32REG_APLL = sysGetPLLControlRegister(g_u32ExtClk, g_u32ApllKHz);
            DBG_PRINTF("APLL register = 0x%x\n", g_u32REG_APLL);
            break;
        case eSYS_UPLL:
            g_u32SysClkSrc = eSYS_UPLL;
            u32SysSrc = u32PllKHz;
            g_u32UpllKHz = u32PllKHz;
            g_u32REG_UPLL = sysGetPLLControlRegister(g_u32ExtClk, g_u32UpllKHz);
            DBG_PRINTF("UPLL register = 0x%x\n", g_u32REG_UPLL);
            break;
        default:
            return E_ERR_CLK;
    }
    if(eSrcClk == eSYS_EXT)
    {
        u32RegPll = 0;
        g_u32HclkKHz = 27000;
        bIsCpuOver2xHclk = 0;
        g_u32SysDiv =1;
        g_u32CpuDiv =1;
        g_u32ApbDiv = 1;
    }
    else
    {
        u32SysDiv = u32PllKHz / g_u32SysKHz-1;
        u32CpuDiv = g_u32SysKHz / g_u32CpuKHz-1;
    }
    /*

    */
    if(eSrcClk==eSYS_UPLL)
    {
        u32RegPll = g_u32REG_UPLL;
        DBG_PRINTF("HCLK clock from UPLL = %d\n", g_u32HclkKHz);
        bIsAPLLInitialize = FALSE;
    }
    else if(eSrcClk==eSYS_APLL)
    {
        u32RegPll = g_u32REG_APLL;
        DBG_PRINTF("HCLK clock from APLL = %d\n", g_u32HclkKHz);
    }
    sysClockSwitchStart(eSrcClk,
                    u32RegPll,
                    g_u32HclkKHz,
                    //bIsCpuOver2xHclk,
                    g_u32SysDiv,
                    g_u32CpuDiv,
                    g_u32ApbDiv);
    return Successful;
}

void sysGetSystemClock(E_SYS_SRC_CLK* peSrcClk, // System clock frol UPLL or APLL or external clock
                    PUINT32 pu32PllKHz,         //
                    PUINT32 pu32SysKHz,
                    PUINT32 pu32CpuKHz,
                    PUINT32 pu32HclkKHz,
                    PUINT32 pu32ApbKHz)
{

    *peSrcClk = g_u32SysClkSrc;

    if(g_u32SysClkSrc==eSYS_UPLL)
        *pu32PllKHz = g_u32UpllKHz;
    else if(g_u32SysClkSrc==eSYS_APLL)
        *pu32PllKHz = g_u32ApllKHz;

    *pu32SysKHz = g_u32SysKHz;
    *pu32CpuKHz = g_u32CpuKHz;
    *pu32HclkKHz = g_u32HclkKHz;
    *pu32ApbKHz = g_u32ApbKHz;
}
/*-----------------------------------------------------------------------------------------------------------
*   The Function is used to set the other PLL which is not the system clock source.
*   If system clock source come from eSYS_UPLL. The eSrcClk only can be eSYS_APLL
*   And if specified PLL not meet some costraint, the funtion will search the near frequency and not over the specified frequency
*
*   Paramter:
*       eSrcClk: eSYS_UPLL or eSYS_APLL
*       u32TargetKHz: The specified frequency. Unit:Khz.
*
*   Return:
*       The specified PLL output frequency really.
-----------------------------------------------------------------------------------------------------------*/
UINT32 sysSetPllClock(E_SYS_SRC_CLK eSrcClk, UINT32 u32TargetKHz)
{
    UINT32 u32PllReg, u32PllOutFreqKHz, u32FinKHz;

    u32FinKHz = sysGetExternalClock();

    //Specified clock is system clock,  return working frequency directly.
    if( (inp32(REG_CLKDIV0) & SYSTEM_S)== 0x18 )
    {//System from UPLL
        if(eSrcClk==eSYS_UPLL)
        {
            u32PllOutFreqKHz = sysGetPLLOutputKhz(eSrcClk, u32FinKHz);
            return u32PllOutFreqKHz;
        }
    }
    if( (inp32(REG_CLKDIV0) & SYSTEM_S)== 0x10 )
    {//System from APLL
        if(eSrcClk==eSYS_APLL)
        {
            u32PllOutFreqKHz = sysGetPLLOutputKhz(eSrcClk, u32FinKHz);
            return u32PllOutFreqKHz;
        }
    }
    //Specified clock is not system clock,
    u32PllReg = sysGetPLLControlRegister(u32FinKHz, u32TargetKHz);
    if(eSrcClk == eSYS_APLL)
        outp32(REG_APLLCON, u32PllReg);
    else if(eSrcClk == eSYS_UPLL)
        outp32(REG_UPLLCON, u32PllReg);
    if((eSrcClk == eSYS_APLL) || (eSrcClk == eSYS_UPLL))
    {
        u32PllOutFreqKHz = sysGetPLLOutputKhz(eSrcClk, u32FinKHz);
        return u32PllOutFreqKHz;
    }
    else
        return 0;

}


/* ================================================
Change system clock without DDR entering self-refresh.
The code will be copied to SRAM by function-sysClockDivSwitchStart()
==================================================*/
void sysClockDiv(register UINT32 u32HCLK, register UINT32 u32SysDiv)
{
    register UINT32 u32IntTmp, u32Delay=100;
    UINT32 u32HCLKLimit;
/*Disable interrupts to ensure no any DDR request during change clock */
    u32IntTmp = inp32(REG_AIC_IMR);
    outp32(REG_AIC_MDCR, 0xFFFFFFFE);

    if( (inp32(REG_CHIPCFG)&SDRAMSEL) == 0x20){//DDR2 will always disable DLL. Due to DLL enable only HCLK>133MHz.
        outp32(REG_SDEMR, inp32(REG_SDEMR)  | DLLEN); //Disable DLL.
        u32HCLKLimit = 96000;
        if(u32HCLK<u32HCLKLimit)
            outp32(REG_SDOPM, inp32(REG_SDOPM)  | LOWFREQ); //Enable Low Freq
        else
            outp32(REG_SDOPM, inp32(REG_SDOPM)  & ~LOWFREQ); //Enable Low Freq
    }else{
        u32HCLKLimit = 64000;
    }

    if(u32HCLK<u32HCLKLimit)
    {
        //Disable DLL of SDRAM device
        outp32(REG_CLKDIV0, (inp32(REG_CLKDIV0) & ~SYSTEM_N0) | u32SysDiv );

        if( (inp32(REG_CHIPCFG)&SDRAMSEL) != 00)
        {//SDRAM unnecessary
            outp32(REG_SDEMR, inp32(REG_SDEMR)  | DLLEN);
            outp32(REG_SDOPM, inp32(REG_SDOPM) | LOWFREQ);
        }
    }
    else
    {
        //Enable DLL of SDRAM device
        outp32(REG_CLKDIV0, (inp32(REG_CLKDIV0) & ~SYSTEM_N0) | u32SysDiv );
        outp32(REG_SDOPM, inp32(REG_SDOPM) & ~LOWFREQ);
        while(u32Delay--);/*Must*/
        if( (inp32(REG_CHIPCFG)&SDRAMSEL) == 0x20){//DDR2 will always disable DLL. Due to DLL enable only HCLK>133MHz.
            outp32(REG_SDEMR, inp32(REG_SDEMR)  | DLLEN); //Disable DLL.
        }else   {//Other memory type
            if( (inp32(REG_CHIPCFG)&SDRAMSEL) != 0x00)  //if memory != SDRAM type
            outp32(REG_SDEMR, inp32(REG_SDEMR)  & ~DLLEN);
        }
    }
    outp32(REG_AIC_MECR, u32IntTmp);
    u32IntTmp = 0x1000;
    while(u32IntTmp--);
}
/* ================================================
Copy function- sysClockDiv() to RAM,
then jump to SRAM to execute the code.
==================================================*/
INT32 sysClockDivSwitchStart(UINT32 u32SysDiv)
{
    UINT32   vram_base, aic_status = 0;
    UINT32 u32PllOutKHz, u32ExtFreq;
    VOID    (*wb_func)(UINT32, UINT32);
    if(u32SysDiv>7)
        return -1;
    /* Judge system clock come from */
    if((inp32(REG_CLKDIV0) & SYSTEM_S)==0)
        u32PllOutKHz = sysGetExternalClock();
    else{
        u32ExtFreq = sysGetExternalClock();
        if((inp32(REG_CLKDIV0) & SYSTEM_S) == 0x18)
            u32PllOutKHz = sysGetPLLOutputKhz(eSYS_UPLL, u32ExtFreq);
           else
                u32PllOutKHz = sysGetPLLOutputKhz(eSYS_APLL, u32ExtFreq);
    }


    aic_status = inpw(REG_AIC_IMR); //Disable all interrupts
    outpw(REG_AIC_MDCR, 0xFFFFFFFF);
    vram_base = PD_RAM_BASE;
    memcpy((char *)((UINT32)_tmp_buf | 0x80000000),
            (char *)((UINT32)PD_RAM_BASE | 0x80000000),
            PD_RAM_SIZE);

    memcpy((VOID *)((UINT32)vram_base | 0x80000000),
            (VOID *) (((UINT32)sysClockDiv-(PD_RAM_START-PD_RAM_BASE)) | 0x80000000),
            PD_RAM_SIZE);
    sysprintf("system divider = %d\n", u32SysDiv);

    vram_base = PD_RAM_START;
    wb_func = (void(*)(UINT32, UINT32)) vram_base;
    sysprintf("Jump to SRAM\n");
    wb_func(u32PllOutKHz/(u32SysDiv+1)/2 , u32SysDiv);

    //Restore VRAM
    vram_base = PD_RAM_BASE;
    memcpy((VOID *)((UINT32)vram_base | 0x80000000),
            (VOID *)((UINT32)_tmp_buf | 0x80000000),
            PD_RAM_SIZE);

    outpw(REG_AIC_MDCR, 0xFFFFFFFF);    // Disable all interrupts
    outpw(REG_AIC_MECR, aic_status);        // Restore AIC setting
    return Successful;
}

#if 0   /* ignore by SDLoader */
/*
    The function is used to power down unused PLL if system come from external clock
    Please remember that don't power down the PLL is used by some IPs.
*/
INT32 sysPowerDownPLL(E_SYS_SRC_CLK eSrcClk, BOOL bIsPowerDown)
{
    if(bIsPowerDown==TRUE){/* If power down, need to check whether the PLL is system clock source */
        if( (inp32(REG_CLKDIV0)&SYSTEM_S) == (eSrcClk<<3) )
            return -1;
    }
    if(eSrcClk == eSYS_APLL){
        if(bIsPowerDown==TRUE)
            outp32(REG_APLLCON, (inp32(REG_APLLCON)|PD) );
        else
            outp32(REG_APLLCON, (inp32(REG_APLLCON)&~PD) );
    }else if(eSrcClk == eSYS_UPLL){
        if(bIsPowerDown==TRUE)
            outp32(REG_UPLLCON, (inp32(REG_UPLLCON)|PD) );
        else
            outp32(REG_UPLLCON, (inp32(REG_UPLLCON)&~PD) );
    }
    if(bIsPowerDown!=TRUE)
        sysDelay(1);    /* If not power down, need wait 500us for PLL stable */
    return Successful;
}
#endif

#if 0
void sysFirstAdjustAPLL(UINT32 u32ApllClockKHz)
{
    DBG_PRINTF("System will switch to external\n");
    //sysExternalClock();
    sysSetSystemClock(eSYS_EXT,
                    27000,
                    27000,
                    27000,
                    27000/2,
                    27000/4);

    DBG_PRINTF("System switch to external successful\n");
    DBG_PRINTF("Will first Program APLL\n");
    sysSetSystemClock(eSYS_APLL,    //E_SYS_SRC_CLK eSrcClk,
                        u32ApllClockKHz,        //UINT32 u32PllKHz,
                        u32ApllClockKHz/2,      //UINT32 u32SysKHz,
                        u32ApllClockKHz/4,      //UINT32 u32CpuKHz,
                        u32ApllClockKHz/4,      //UINT32 u32HclkKHz,
                         u32ApllClockKHz/8);        //UINT32 u32ApbKHz
    DBG_PRINTF("First Program APLL successful\n");
    bIsAPLLInitialize = TRUE;
}




int sysSetPLLConfig(WB_PLL_T *sysClk)
{
    sysClk->pll0 = 0;
    sysClk->cpu_src = 0;
    sysClk->ahb_clk = 0;
    sysClk->apb_clk = 0;
    _sysClockIniFlag = TRUE;
    return 0;
} /* end sysSetPLLConfig */

int sysGetPLLConfig(WB_PLL_T *sysClk)
{
    sysClk->pll0 = 0;
    sysClk->cpu_src = 0;
    sysClk->ahb_clk = 0;
    sysClk->apb_clk = 0;
    return 0;
} /* end sysGetPLLConfig */

int sysGetClockFreq(WB_CLKFREQ_T *sysFreq)
{
    _sysFreq.pll_clk_freq = 0;
    _sysFreq.cpu_clk_freq = 0;
    _sysFreq.ahb_clk_freq = 0;
    _sysFreq.apb_clk_freq = 0;
    return 0;
}
#endif