/**************************************************************************//**
 * @file     user_define_func.c
 * @brief    NandLoader source code for user define function.
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#ifdef __USER_DEFINE_FUNC

#include "user_define_func.h"

void initVPostShowLogo(void);
INT32 vpostLCMInit(PLCDFORMATEX plcdformatex, UINT32 *pFramebuf);
LCDFORMATEX lcdInfo;

/*-----------------------------------------------------------------------------------*/
/* The entry point of User Define Function that called by NandLoader.                */
/*-----------------------------------------------------------------------------------*/
void user_define_func()
{
    //--- This is a sample code for user define function.
    //--- This sample code will show Logo to panel on Nuvoton N9H20 Demo Board.
    initVPostShowLogo();
}


static UINT32 bIsInitVpost=FALSE;
void initVPostShowLogo(void)
{
    if(bIsInitVpost==FALSE)
    {
        bIsInitVpost = TRUE;
        lcdInfo.ucVASrcFormat = DRVVPOST_FRAME_RGB565;
        lcdInfo.nScreenWidth = PANEL_WIDTH;
        lcdInfo.nScreenHeight = PANEL_HEIGHT;
        vpostLCMInit(&lcdInfo, (UINT32*)FB_ADDR);

        /* If backlight control signal is different from nuvoton's demo board,
           please don't call this function and must implement another similar one to enable LCD backlight. */
        vpostEnaBacklight();
    }
}

#else

/*-----------------------------------------------------------------------------------*/
/* The entry point of User Define Function that called by NandLoader.                */
/*-----------------------------------------------------------------------------------*/
void user_define_func()
{
    //--- Keep empty if user define nothing for User Define Function.
}

#endif  // end of #ifdef __USER_DEFINE_FUNC
