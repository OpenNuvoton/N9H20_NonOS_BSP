/****************************************************************
 *                                                              *
 * Copyright (c) Nuvoton Technology Corp. All rights reserved.  *
 *                                                              *
 ****************************************************************/
     
#include "wblib.h"

#define PANEL_WIDTH   480    /* PANEL Width (Raw data output width for Panel Test) */
#define PANEL_HEIGHT  272    /* PANEL Height (Raw data output height for Panel Test) */

INT JpegDec(UINT32 u32SrcAddr, UINT32 u32DestAddr);
/*  Header Decode Complete Callback function */
BOOL JpegDecHeaderComplete(void);

extern CHAR g_pu8FrameBuffer[PANEL_WIDTH * PANEL_HEIGHT * 2]; 
