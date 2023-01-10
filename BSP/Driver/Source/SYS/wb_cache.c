/**************************************************************************//**
 * @file     wb_cache.c
 * @version  V3.00
 * @brief    N9H20 series SYS driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
 
/****************************************************************************
* FILENAME
*   wb_cache.c
*
* VERSION
*   1.0
*
* DESCRIPTION
*   The cache related functions of Nuvoton ARM9 MCU
*
* HISTORY
*   2008-06-25  Ver 1.0 draft by Min-Nan Cheng
*   	
* REMARK
*   None
 **************************************************************************/
#include "wblib.h"

BOOL volatile _sys_IsCacheOn = FALSE;
INT32 volatile _sys_CacheMode;
extern void sys_flush_and_clean_dcache(void);
//extern void sysDisableDCache(void);
//extern void sysDisableICache(void);
extern int sysInitMMUTable(int);


INT32 sysGetSdramSizebyMB()
{
	unsigned int reg, totalsize=0;

	reg = inpw(REG_SDSIZE0) & 0x07;
	switch(reg)
	{
		case 1:
			totalsize += 2;
			break;

		case 2:
			totalsize += 4;
			break;

		case 3:
			totalsize += 8;
			break;

		case 4:
			totalsize += 16;
			break;

		case 5:
			totalsize += 32;
			break;

		case 6:
			totalsize += 64;
			break;
	}

	reg = inpw(REG_SDSIZE1) & 0x07;
	switch(reg)
	{
		case 1:
			totalsize += 2;
			break;

		case 2:
			totalsize += 4;
			break;

		case 3:
			totalsize += 8;
			break;

		case 4:
			totalsize += 16;
			break;

		case 5:
			totalsize += 32;
			break;

		case 6:
			totalsize += 64;
			break;
	}

	if (totalsize != 0)
		return totalsize;
	else
		return Fail;	
}


INT32 sysEnableCache(UINT32 uCacheOpMode)
{
	sysInitMMUTable(uCacheOpMode);
	_sys_IsCacheOn = TRUE;
	_sys_CacheMode = uCacheOpMode;
	
	return 0;
}

#if defined (__GNUC__) && !defined(__CC_ARM)
#pragma GCC push_options
#pragma GCC optimize ("O0")
void sys_flush_and_clean_dcache(void)
{
	register int reg2;
	asm volatile(
    " tci_loop100:                              \n"
    "     MRC p15, #0, r15, c7, c14, #3         \n"     /* test clean and invalidate */
    "     BNE tci_loop100                       \n"
    : : : "memory");
}
#pragma GCC pop_options
#else
__asm void sys_flush_and_clean_dcache(void)
{
tci_loop
    MRC p15, 0, r15, c7, c14, 3 // test clean and invalidate
    BNE tci_loop
    BX  lr
}
#endif

VOID sysDisableCache()
{
	int temp;

	sys_flush_and_clean_dcache();
#if defined (__GNUC__) && !defined(__CC_ARM)
	asm volatile
    (
        /*----- flush I, D cache & write buffer -----*/
        "MOV %0, #0x0				\n\t"
        "MCR p15, 0, %0, c7, c5, #0 	\n\t" /* flush I cache */
        "MCR p15, 0, %0, c7, c6, #0  \n\t" /* flush D cache */
        "MCR p15, 0, %0, c7, c10,#4  \n\t" /* drain write buffer */

        /*----- disable Protection Unit -----*/
        "MRC p15, 0, %0, c1, c0, #0   \n\t" /* read Control register */
        "BIC %0, %0, #0x01            \n\t"
        "MCR p15, 0, %0, c1, c0, #0   \n\t" /* write Control register */
		: :"r"(temp) : "memory"
    );
#else
    __asm
    {
        /*----- flush I, D cache & write buffer -----*/
        MOV temp, 0x0
        MCR p15, 0, temp, c7, c5, 0 /* flush I cache */
        MCR p15, 0, temp, c7, c6, 0 /* flush D cache */
        MCR p15, 0, temp, c7, c10,4 /* drain write buffer */

        /*----- disable Protection Unit -----*/
        MRC p15, 0, temp, c1, c0, 0     /* read Control register */
        BIC temp, temp, 0x01
        MCR p15, 0, temp, c1, c0, 0     /* write Control register */
    }
#endif
	
	_sys_IsCacheOn = FALSE;
	_sys_CacheMode = CACHE_DISABLE;
	
}

VOID sysFlushCache(INT32 nCacheType)
{
	int temp;

	    switch (nCacheType)
	    {
	    case I_CACHE:

	#if defined (__GNUC__) && !defined(__CC_ARM)
	        asm volatile
	        (
	            /*----- flush I-cache -----*/
	            "MOV %0, #0x0  \n\t"
	            "MCR p15, #0, %0, c7, c5, 0  \n\t" /* invalidate I cache */
	            : "=r"(temp)
	            : "0" (temp)
	            : "memory"
	        );
	#else
	        __asm
	        {
	            /*----- flush I-cache -----*/
	            MOV temp, 0x0
	            MCR p15, 0, temp, c7, c5, 0 /* invalidate I cache */
	        }
	#endif
	        break;

	    case D_CACHE:
	        sys_flush_and_clean_dcache();

	#if defined (__GNUC__) && !defined(__CC_ARM)
	        asm volatile
	        (
	            /*----- flush D-cache & write buffer -----*/
	            "MOV %0, #0x0 \n\t"
	            "MCR p15, #0, %0, c7, c10, #4 \n\t" /* drain write buffer */
	            :"=r" (temp)
	            :"0"  (temp)
	            :"memory"
	        );
	#else
	        __asm
	        {
	            /*----- flush D-cache & write buffer -----*/
	            MOV temp, 0x0
	            MCR p15, 0, temp, c7, c10, 4 /* drain write buffer */
	        }
	#endif
	        break;

	    case I_D_CACHE:
	        sys_flush_and_clean_dcache();
	#if defined(__GNUC__) && !defined(__CC_ARM)
	        asm volatile
	        (
	            /*----- flush I, D cache & write buffer -----*/
	            "MOV %0, #0x0  \n\t"
	            "MCR p15, #0, %0, c7, c5, #0  \n\t" /* invalidate I cache */
	            "MCR p15, #0, %0, c7, c10, #4 \n\t" /* drain write buffer */
	            :"=r" (temp)
	            :"0"  (temp)
	            :"memory"
	        );
	#else
	        __asm
	        {
	            /*----- flush I, D cache & write buffer -----*/
	            MOV temp, 0x0
	            MCR p15, 0, temp, c7, c5, 0 /* invalidate I cache */
	            MCR p15, 0, temp, c7, c10, 4 /* drain write buffer */
	        }
	#endif
	        break;

	    default:
	        ;
	    }
}

VOID sysInvalidCache()
{
	int temp;

#if defined (__GNUC__) && !defined(__CC_ARM)
    asm volatile
    (
        "MOV %0, #0x0 \n\t"
        "MCR p15, #0, %0, c7, c7, #0 \n\t" /* invalidate I and D cache */
        :"=r" (temp)
        :"0" (temp)
        :"memory"
    );
#else
    __asm
    {
        MOV temp, 0x0
        MCR p15, 0, temp, c7, c7, 0 /* invalidate I and D cache */
    }
#endif
}

BOOL sysGetCacheState()
{
	return _sys_IsCacheOn;
}


INT32 sysGetCacheMode()
{
	return _sys_CacheMode;
}


INT32 _sysLockCode(UINT32 addr, INT32 size)
{
    int i, cnt, temp;

#if defined (__GNUC__) && !defined(__CC_ARM)
    asm volatile
    (
        /* use way3 to lock instructions */
        "MRC p15, #0, %0, c9, c0, #1 \n\t"
        "ORR %0, %0, #0x07 \n\t"
        "MCR p15, #0, %0, c9, c0, #1 \n\t"
        :"=r" (temp)
        :"0" (temp)
        :"memory"
    );
#else
    __asm
    {
        /* use way3 to lock instructions */
        MRC p15, 0, temp, c9, c0, 1 ;
        ORR temp, temp, 0x07 ;
        MCR p15, 0, temp, c9, c0, 1 ;
    }
#endif

    if (size % 16)  cnt = (size/16) + 1;
    else            cnt = size / 16;

    for (i=0; i<cnt; i++)
    {
#if defined (__GNUC__) && !defined(__CC_ARM)
        asm volatile
        (
            "MCR p15, #0, r0, c7, c13, #1 \n\t"
        );
#else
        __asm
        {
            MCR p15, 0, addr, c7, c13, 1;
        }
#endif

        addr += 16;
    }

#if defined (__GNUC__) && !defined(__CC_ARM)
    asm volatile
    (
        /* use way3 to lock instructions */
        "MRC p15, #0, %0, c9, c0, #1 \n\t"
        "BIC %0, %0, #0x07  \n\t"
        "ORR %0, %0, #0x08 \n\t"
        "MCR p15, #0, %0, c9, c0, #1 \n\t"
        :"=r" (temp)
        :"0"  (temp)
        :"memory"
    );
#else
    __asm
    {
        /* use way3 to lock instructions */
        MRC p15, 0, temp, c9, c0, 1 ;
        BIC temp, temp, 0x07 ;
        ORR temp, temp, 0x08 ;
        MCR p15, 0, temp, c9, c0, 1 ;
    }
#endif

    return 0;

}


INT32 _sysUnLockCode()
{
    int temp;

    /* unlock I-cache way 3 */
#if defined (__GNUC__) && !defined(__CC_ARM)
    asm volatile
    (
        "MRC p15, #0, %0, c9, c0, #1  \n"
        "BIC %0, %0, #0x08 \n"
        "MCR p15, #0, %0, c9, c0, #1  \n"
        :"=r" (temp)
        :"0"  (temp)
        :"memory"
    );
#else
    __asm
    {
        MRC p15, 0, temp, c9, c0, 1;
        BIC temp, temp, 0x08 ;
        MCR p15, 0, temp, c9, c0, 1;
    }
#endif

    return 0;
}


