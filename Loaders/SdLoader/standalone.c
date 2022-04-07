/*
 * standalone.c - minimal bootstrap for C library
 * Copyright (C) 2000 ARM Limited.
 * All rights reserved.
 */

/*
 * RCS $Revision: 1 $
 * Checkin $Date: 07/07/05 2:54p $ 0
 * Revising $Author: Sjlu0 $
 */

/*
 * This code defines a run-time environment for the C library.
 * Without this, the C startup code will attempt to use semi-hosting
 * calls to get environment information.
 */
extern unsigned int Image$$ZI$$Limit;

void _sys_exit(int return_code)
{
label:  goto label; /* endless loop */
}

void _ttywrch(int ch)
{
    char tempch = (char)ch;
    (void)tempch;
}

void dummy_func(void)
{
    extern void Vector_Table(void);
    unsigned int temp;
    __asm
    {
    	mov temp, 0x38
    	sub	temp, temp, 0x38
    }
	if( temp )Vector_Table();
}

__value_in_regs struct R0_R3 {unsigned heap_base, stack_base, heap_limit, stack_limit;} 
    __user_initial_stackheap(unsigned int R0, unsigned int SP, unsigned int R2, unsigned int SL)
{
    struct R0_R3 config;

	extern unsigned int bottom_of_heap;     /* defined in heap.s */
	extern unsigned int top_of_stacks;		/* defined in stack.s */
   // struct __initial_stackheap config;
    
    config.heap_base = (unsigned int)&bottom_of_heap; // defined in heap.s                                                      // placed by scatterfile   
    config.heap_limit = config.heap_base + 0x10000;                                                      
    config.stack_base = (unsigned int)&top_of_stacks;   // inherit SP from the execution environment
	
	dummy_func(); // This dummy function was used to ensure the vector.s could be linked


    return config;
}


/* end of file standalone.c */
