/**************************************************************************//**
 * @file     wb_mmu.c
 * @version  V3.00
 * @brief    N9H20 series SYS driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

/****************************************************************************
 * 
 * FILENAME
 *     wb_mmu.c
 *
 * VERSION
 *     1.0
 *
 * DESCRIPTION
 *     This file implement mmu functions.
 *
  * HISTORY
 *     2008-06-25  Ver 1.0 draft by Min-Nan Cheng
 *
 * REMARK
 *     None
 **************************************************************************/
#include "wblib.h"

#if defined ( __CC_ARM )
#pragma push
#pragma O2
#endif

#define USING_SECTION_TABLE

#define  _CoarsePageSize 	64  //MB

typedef struct _coarse_table
{
	unsigned int page[256];
} _CTable;

#ifdef USING_SECTION_TABLE
#if defined (__GNUC__)
    unsigned int _mmuSectionTable[4096] __attribute__((aligned (0x4000)));
#else
    __align(0x4000) unsigned int _mmuSectionTable[4096];
#endif
#else
__align(0x4000) unsigned int _mmuSectionTable[4096];
__align(1024) static _CTable _mmuCoarsePageTable[_CoarsePageSize];          // maximum 64MB for coarse pages
__align(1024) static _CTable _mmuCoarsePageTable_NonCache[_CoarsePageSize]; // Shadow SDRAM area for non-cacheable
#endif


static BOOL _IsInitMMUTable = FALSE;

//extern INT32 sysGetSdramSizebyMB(void);
extern void sysSetupCP15(unsigned int);

#if defined (__GNUC__) && !(__CC_ARM)
void sysSetupCP15(unsigned int addr)
{
	register int reg1, reg0;
	reg0 = addr;
    asm volatile(
    "MOV     %0, %1                \n" // _mmuSectionTable
    "MCR     p15, #0, %0, c2, c0, #0  \n" // write translation table base register c2

    "MOV     %0, #0x40000000   \n"
    "MCR     p15, #0, %0, c3, c0, #0  \n"  // domain access control register c3

    "MRC     p15, #0, %0, c1, c0, #0 \n" // read control register c1
    "ORR     %0, %0, #0x1000 \n"       // enable I cache bit
    "ORR     %0, %0, #0x5     \n"      // enable D cache and MMU bits
    "MCR     p15, #0, %0, c1, c0, #0  \n" // write control register c1
    : : "r"(reg1), "r"(reg0) :"memory"
    );
}
#else
__asm void sysSetupCP15(unsigned int addr)
{
    MOV     r1, r0                 // _mmuSectionTable
    MCR     p15, 0, r1, c2, c0, 0  // write translation table base register c2

    MOV     r1, #0x40000000
    MCR     p15, 0, r1, c3, c0, 0  // domain access control register c3

    MRC     p15, 0, r1, c1, c0, 0  // read control register c1
    ORR     r1, r1, #0x1000        // enable I cache bit
    ORR     r1, r1, #0x5           // enable D cache and MMU bits
    MCR     p15, 0, r1, c1, c0, 0  // write control register c1
    BX      lr
}
#endif

#ifndef USING_SECTION_TABLE
static int _MMUMappingMode = MMU_DIRECT_MAPPING;
unsigned int sysGetPhyPageAddr(unsigned int vaddr)
{
	int table_num, page_num;
	unsigned int base_addr, page_base, page_offset, phy_addr;
	volatile _CTable *PageTabPtr;
	
	if (vaddr & 0x80000000)
		PageTabPtr = (_CTable *) _mmuCoarsePageTable_NonCache; //non-cacheable virtual address
	else
		PageTabPtr = (_CTable *) _mmuCoarsePageTable;	//cache-able virtual address
		
	if (sysGetCacheState() == TRUE)
		PageTabPtr = (_CTable *) ((unsigned int)PageTabPtr | 0x80000000); //If cache is enable, must write page tables directly into SDRAM		
	
	base_addr = vaddr & 0x7FFFF000;
	table_num = base_addr / 0x100000;
	page_num = (base_addr & 0xFF000) >> 12;
	
	page_base = (*(PageTabPtr+table_num)).page[page_num] & 0xFFFFF000;
	page_offset = vaddr & 0xFFF;
	phy_addr = page_base + page_offset;

	return phy_addr;

} /* end sysGetPHYAddr */

int sysSetCachePages(unsigned int vaddr, int size, int cache_flag)
{
	int i, cnt, table_num, page_num, cache_mode;
	unsigned volatile int baseaddr, temp;
	volatile _CTable *PageTabPtr;
	
	if (vaddr & 0x80000000)
		PageTabPtr = (_CTable *) _mmuCoarsePageTable_NonCache; //non-cacheable virtual address
	else
		PageTabPtr = (_CTable *) _mmuCoarsePageTable;	//cache-able virtual address
		
	if (sysGetCacheState() == TRUE)
		PageTabPtr = (_CTable *) ((unsigned int)PageTabPtr | 0x80000000); //If cache is enable, must write page tables directly into SDRAM	
	
	vaddr &= 0x7FFFFFFF;	//ignore the non-cacheable bit 31	
	//if ( _IsInitMMUTable == FALSE ) return -1;	
	if ((vaddr + size) > (_CoarsePageSize << 20)) return -1;
	
	if (vaddr & 0xFFF) 	return -1;  /* MUST 4K Boundary */
	if (size % 4096)	return -1;  /* MUST 4K multiple size */		
		
	/* for flat mapping address */
	cnt = size / 4096;	

	if (cache_flag == CACHE_WRITE_BACK) /* write back mode */
		cache_mode = 0x0C; 
	else if (cache_flag == CACHE_WRITE_THROUGH) /* write through mode */
		cache_mode = 0x08; 
	else
		cache_mode = 0; /* Non-cacheable, non-buffered */
	
	for (i=0; i<cnt; i++)
	{
		baseaddr = vaddr + i * 4096;
		table_num = baseaddr / 0x100000;
		page_num =  (baseaddr & 0xFF000) >> 12; /* bits [19:12] for level two table index */
	
		temp = (*(PageTabPtr+table_num)).page[page_num] & 0xFFFFFFF3;			
		temp |= cache_mode; /* cache mode */			
		(*(PageTabPtr+table_num)).page[page_num] = temp;
	} 
	
	//sysFlushCache(D_CACHE);
	
	return 0;
	
} /* end sysSetCachePages */



int sysInitPageTable(unsigned int vaddr, unsigned int phy_addr, int size, int cache_flag, int rev_flag)
{
	int i, cnt, table_num, page_num, cache_mode, addr_offset;
	unsigned volatile int phy_base_addr, vbase_addr, temp;
	volatile _CTable *PageTabPtr;
	
	if (vaddr & 0x80000000)
		PageTabPtr = (_CTable *) _mmuCoarsePageTable_NonCache; //non-cacheable virtual address
	else
		PageTabPtr = (_CTable *) _mmuCoarsePageTable;	//cache-able virtual address
		
	if (sysGetCacheState() == TRUE)
		PageTabPtr = (_CTable *) ((unsigned int)PageTabPtr | 0x80000000); //If cache is enable, must write page tables directly into SDRAM		
	
	//if ( _IsInitMMUTable == FALSE ) return -1;	
	vaddr &= 0x7FFFFFFF;	//ignore the non-cacheable bit 31
	if ((vaddr + size) > (_CoarsePageSize << 20)) return -1;	
	if (vaddr & 0xFFFFF) 	return -1;  /* MUST 1M Boundary */
	if (size % 4096)	    return -1;  /* MUST 4K multiple size */		
						
	/* Pages count */
	cnt = size / 4096;	

	if (cache_flag == CACHE_WRITE_BACK) /* write back mode */
		cache_mode = 0x0C; 
	else if (cache_flag == CACHE_WRITE_THROUGH) /* write through mode */
		cache_mode = 0x08; 
	else
		cache_mode = 0; /* Non-cacheable, non-buffered */
		
		
	if (rev_flag == MMU_DIRECT_MAPPING)
		phy_base_addr = phy_addr;
	else
		phy_base_addr = phy_addr + size - 4096;		
	
	addr_offset = 4096;
	for (i=0; i<cnt; i++)
	{				
		vbase_addr = vaddr + i * 4096;
		table_num = vbase_addr / 0x100000;
		page_num =  (vbase_addr & 0xFF000) >> 12; /* bits [19:12] for level two table index */
	
		temp = phy_base_addr & 0xFFFFF000;
		temp |= 0xFF0; /* access permission, 11 for read/write */
		temp |= cache_mode; /* cache mode */
		temp |= 0x02;  /* small page */
		
		(*(PageTabPtr+table_num)).page[page_num] = temp;
		
		if (rev_flag == MMU_DIRECT_MAPPING)
			phy_base_addr += addr_offset;
		else
			phy_base_addr -= addr_offset;
	} 
		
	return 0;
	
} /* end sysInitPageTable */
#endif

int sysSetMMUMappingMethod(int mode)
{
#ifndef USING_SECTION_TABLE
	_MMUMappingMode = mode;
#endif	
	return 0;

} /* end sysSetMMUMappingMethod */


int sysInitMMUTable(int cache_mode)
{
	int i;
#ifndef USING_SECTION_TABLE	
    unsigned volatile int temp;
    int size, ramsize;
#endif
    if (_IsInitMMUTable == FALSE) {
#ifndef USING_SECTION_TABLE
		ramsize = sysGetSdramSizebyMB();
		
		//flat mapping for 4GB, 4096 section table, each size is 1MB
		temp = 0xC00;   /* (11:10) access permission, R/W */
		temp |= 0x1E0;  /* (8:5) domain 15 */
		temp |= 0x10;   /* bit 4 must be 1 */
		temp |= 0x00;   /* bit 3:2 for cache control bits, cache disabled */
		temp |= 0x02;   /* set as 1Mb section */
  
		for (i=0; i<4096; i++)
		{
			_mmuSectionTable[i] = (unsigned int)(temp | (i << 20));    
		}
  
        //Inside SDRAM, divide each section into 256 small pages, each page size is 4KB
        if (ramsize > _CoarsePageSize) 
            size = _CoarsePageSize;	//maximum 64MB
        else						   
            size = ramsize;
	
		/* first 1M always direct mapping */
		sysInitPageTable(0, 0, 0x100000, cache_mode, MMU_DIRECT_MAPPING);
		temp = ((unsigned int)_mmuCoarsePageTable  & 0xFFFFFC00); /*  coarse table base address */		
		temp |= 0x1E0;  /* (8:5) domain 15 */
		temp |= 0x10;   /* bit 4 must be 1 */
		temp |= 0x01;   /* Coarse page table */
		_mmuSectionTable[0] = temp;
		
		/* Create a shadow area at 0x80000000 for non-cacheable region */
		sysInitPageTable(0x80000000, 0x0, 0x100000, CACHE_DISABLE, MMU_DIRECT_MAPPING);
		temp = ((unsigned int)_mmuCoarsePageTable_NonCache  & 0xFFFFFC00); /*  coarse table base address */
		temp |= 0x1E0;  /* (8:5) domain 15 */
		temp |= 0x10;   /* bit 4 must be 1 */
		temp |= 0x01;   /* Coarse page table */
		_mmuSectionTable[0x800] = temp;
		
		/* Mapping the other memory */
		for (i=1; i< size; i++)
		{
			temp = (((unsigned int)_mmuCoarsePageTable + (unsigned int)i*1024) & 0xFFFFFC00); /*  coarse table base address */
			//temp = ((unsigned int)(0x604000 + i*1024) & 0xFFFFFC00); /* coarse table base address */
			temp |= 0x1E0;  /* (8:5) domain 15 */
			temp |= 0x10;   /* bit 4 must be 1 */
			temp |= 0x01;   /* Coarse page table */

			if (_MMUMappingMode == MMU_DIRECT_MAPPING)		
				sysInitPageTable((i << 20), (i << 20), 0x100000, cache_mode, MMU_DIRECT_MAPPING); /* direct mapping */
			else
				sysInitPageTable((i << 20), (i << 20), 0x100000, cache_mode, MMU_INVERSE_MAPPING); /* inverse mapping for each 1MB area */
				
			_mmuSectionTable[i] = temp;			
		}
				
		//Create shadow non-cacheabel region
		for (i=1; i< size; i++)
		{
			temp = (((unsigned int)_mmuCoarsePageTable_NonCache + (unsigned int)i*1024) & 0xFFFFFC00); /*  coarse table base address */
			//temp = ((unsigned int)(0x604000 + i*1024) & 0xFFFFFC00); /* coarse table base address */
			temp |= 0x1E0;  /* (8:5) domain 15 */
			temp |= 0x10;   /* bit 4 must be 1 */
			temp |= 0x01;   /* Coarse page table */

			if (_MMUMappingMode == MMU_DIRECT_MAPPING)		
				sysInitPageTable(((i << 20) | 0x80000000), (i << 20), 0x100000, CACHE_DISABLE, MMU_DIRECT_MAPPING); /* direct mapping */
			else
				sysInitPageTable(((i << 20) | 0x80000000), (i << 20), 0x100000, CACHE_DISABLE, MMU_INVERSE_MAPPING); /* inverse mapping for each 1MB area */
				
			_mmuSectionTable[0x800+i] = temp;			
		}
#else
		for (i=0; i< 64; i++)
		{
            _mmuSectionTable[i] = (i<<20) | 0xDFE;            /* Cacheable, Bufferable */
		}
		for (i=64; i< 2048; i++)
		{
            _mmuSectionTable[i] = (i<<20) | 0xDF2;            /* Non-acheable, Non-Bufferable */
		}
		for (i=2048; i< 2112; i++)
		{
            _mmuSectionTable[i] = ((i-2048)<<20) | 0xDF2;     /* Non-acheable, Non-Bufferable */
		}
		for (i=2112; i< 4096; i++)
		{
            _mmuSectionTable[i] = (i<<20) | 0xDF2;            /* Non-acheable, Non-Bufferable */
		}
#endif										
		_IsInitMMUTable = TRUE;
 	}
 	
 	//moved here by cmn [2007/01/27]
 	//set CP15 registers
	sysSetupCP15((unsigned int)_mmuSectionTable);
 	
 	return 0;
	
} /* end sysInitMMUTable */


#if defined (__GNUC__)
#if 1
extern char __heap_start__;
extern char __heap_end__;
unsigned char * HeapSize=0;
unsigned char * _sbrk ( int incr )
{
	  //extern char _Heap_Begin; // Defined by the linker.
	  //extern char _Heap_Limit; // Defined by the linker.

	  static char* current_heap_end = 0;
	  char* current_block_address;

	  if (current_heap_end == 0)
	  {
	      current_heap_end = &__heap_start__;
	  }

	  current_block_address = current_heap_end;

	  // Need to align heap to word boundary, else will get
	  // hard faults on Cortex-M0. So we assume that heap starts on
	  // word boundary, hence make sure we always add a multiple of
	  // 4 to it.
	  incr = (incr + 3) & (~3); // align value to 4
	  if (current_heap_end + incr > &__heap_end__)
	  {
	      // Some of the libstdc++-v3 tests rely upon detecting
	      // out of memory errors, so do not abort here.
	#if 0
	      extern void abort (void);

	      _write (1, "_sbrk: Heap and stack collision\n", 32);

	      abort ();
	#else
	      // Heap has overflowed
	      // errno = ENOMEM;
	      return (unsigned char*) - 1;
	#endif
	  }

	  current_heap_end += incr;

	  return (unsigned char*) current_block_address;
}
#endif
#endif

#if defined ( __CC_ARM )
#pragma pop
#endif
