;/*****************************************************************************
; * @file     GCC.ld
; * @brief    NUC980 link file
; *
; * @copyright (C) 2019 Nuvoton Technology Corp. All rights reserved.
; *****************************************************************************/
#if defined(N9H20K5_32MB)
    #define MEMSIZE 0x2000000
    #define EXEADDR 0x900000
#endif 
#if defined(N9H20K3_8MB)
    #define MEMSIZE 0x800000
    #define EXEADDR 0x700000
#endif 
#if defined(N9H20K1_2MB)
    #define MEMSIZE 0x200000
    #define EXEADDR 0x180000
#endif 

ENTRY(__vector)
/*
	We allocate stack size 0x9000 in Startup_GCC.S
*/
/* define stack size and heap size here */
stack_size = 0x9000;

/* define beginning and ending of stack */
_stack_start = ORIGIN(RAM)+LENGTH(RAM);
_stack_end = _stack_start - stack_size;


MEMORY
{
    RAM  (rwx)   : ORIGIN = EXEADDR, LENGTH = MEMSIZE
}



PROVIDE ( _Heap_Begin = __heap_start__ ) ;
PROVIDE ( _Heap_Limit = __heap_end__ ) ;

SECTIONS
{
    .text :
    {
        PROVIDE(__image_start = .);
        PROVIDE(__text_start = .);

        PROVIDE(__vectors_start = .);
        *(.vectors);
        . = ALIGN(4);
        PROVIDE(__vectors_end = .);
        *(.init);
        . = ALIGN(4);
        *(.text);
        . = ALIGN(4);
        *(.rodata);
        . = ALIGN(4);
        *(.rodata*);
        . = ALIGN(4);

        /* etext = .; */

        PROVIDE(__text_end = .);
    } > RAM

    . = ALIGN(4);
    _etext = .;
    .data : AT (_etext)
    {
		__data_start__ = .;
		/* *(vtable) */
		/* *(.data*)*/
	
		. = ALIGN(4);
		/* preinit data */
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP(*(.preinit_array))
		PROVIDE_HIDDEN (__preinit_array_end = .);
	
		. = ALIGN(4);
		/* init data */
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP(*(SORT(.init_array.*)))
		KEEP(*(.init_array))
		PROVIDE_HIDDEN (__init_array_end = .);
	
	
		. = ALIGN(4);
		/* finit data */
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP(*(SORT(.fini_array.*)))
		KEEP(*(.fini_array))
		PROVIDE_HIDDEN (__fini_array_end = .);
	
		KEEP(*(.jcr*))
		. = ALIGN(4);
		/* All data end */
		__data_end__ = .;
	
    } > RAM

    /* Uninitialized data section */
    . = ALIGN(4);
    .bss :
    {
    /*  Used by the startup in order to initialize the .bss secion */
        _sbss = .;         /* define a global symbol at bss start */
        __bss_start__ = _sbss;
        *(.bss)
        *(.bss*)
        *(COMMON)

        . = ALIGN(4);
        _ebss = .;         /* define a global symbol at bss end */
        __bss_end__ = _ebss;
    } >RAM
    bss_size = __bss_end__ - __bss_start__;
    heap_size = _stack_start - __bss_end__;
    . = ALIGN(4);
    free_memory_start = .;
    .heap :
    {
    	__heap_start__ = .; 
        . = . + heap_size;
        __heap_end__ = .;  
    } > RAM

}

