/**************************************************************************//**
 * @file     sys_cache_ex.c
 * @brief    Extend system cache operations.
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include "wblib.h"
#include "stdint.h"

/*----- Keil -----------------------------------------------------------------*/
#ifdef __CC_ARM

void sysCleanDcache(UINT32 buffer, UINT32 size)
{
    unsigned int ptr;

    ptr = buffer & ~(DEF_CACHE_LINE_SIZE - 1);

    while (ptr < buffer + size)
    {
        __asm volatile { MCR p15, 0, ptr, c7, c10, 1 }
        ptr += DEF_CACHE_LINE_SIZE;
    }
}

void sysInvalidateDcache(UINT32 buffer, UINT32 size)
{
    unsigned int ptr;

    ptr = buffer & ~(DEF_CACHE_LINE_SIZE - 1);

    while (ptr < buffer + size)
    {
        __asm volatile { MCR p15, 0, ptr, c7, c6, 1 }
        ptr += DEF_CACHE_LINE_SIZE;
    }
}

void sysInvalidateDcacheAll()
{
    register uint32_t value;

    value = 0;

    __asm volatile { mcr p15, 0, value, c7, c6, 0 }
}

void sysCleanInvalidatedDcache(UINT32 buffer, UINT32 size)
{
    unsigned int ptr;

    ptr = buffer & ~(DEF_CACHE_LINE_SIZE - 1);

    while (ptr < buffer + size)
    {
        __asm volatile { MCR p15, 0, ptr, c7, c14, 1 }
        ptr += DEF_CACHE_LINE_SIZE;
    }
}

#elif defined(__GNUC__)

void sysCleanDcache(UINT32 buffer, UINT32 size)
{
    unsigned int ptr;

    ptr = buffer & ~(DEF_CACHE_LINE_SIZE - 1);

    while (ptr < buffer + size)
    {
        asm volatile("mcr p15, 0, %0, c7, c10, 1": :"r"(ptr));

        ptr += DEF_CACHE_LINE_SIZE;
    }
}

void sysInvalidateDcache(UINT32 buffer, UINT32 size)
{
    unsigned int ptr;

    ptr = buffer & ~(DEF_CACHE_LINE_SIZE - 1);

    while (ptr < buffer + size)
    {
        asm volatile("mcr p15, 0, %0, c7, c6, 1": :"r"(ptr));

        ptr += DEF_CACHE_LINE_SIZE;
    }
}

void sysInvalidateDcacheAll()
{
    asm volatile("mcr p15, 0, %0, c7, c6, 0": :"r"(0));
}

void sysCleanInvalidatedDcache(UINT32 buffer, UINT32 size)
{
    unsigned int ptr;

    ptr = buffer & ~(DEF_CACHE_LINE_SIZE - 1);

    while (ptr < buffer + size)
    {
        asm volatile("mcr p15, 0, %0, c7, c14, 1": :"r"(ptr));

        ptr += DEF_CACHE_LINE_SIZE;
    }
}

#endif
