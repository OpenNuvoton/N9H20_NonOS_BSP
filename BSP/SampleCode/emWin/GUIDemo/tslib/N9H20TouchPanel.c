/**************************************************************************//**
 * @file     N9H20TouchPanel.c
 * @brief    N9H20 series emWin touch flow
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include "GUI.h"
#include "N9H20.h"
#include "N9H20TouchPanel.h"

INT32 g_u16X, g_u16Y;
volatile BOOL g_bXY;

extern int ts_phy2log(int *sumx, int *sumy);
int Init_TouchPanel(void)
{
    adc_init();
    adc_open(ADC_TS_4WIRE, XSIZE_PHYS, YSIZE_PHYS);  //320x240
    return 1;
}

int Read_TouchPanel(int *x, int *y)
{
//return 0 fai;ure, 1 success.
    unsigned short adc_x, adc_y;
    if ( adc_read(ADC_NONBLOCK, (unsigned short *)&adc_x, (unsigned short *)&adc_y) == 1)
    {
        *x = adc_x;
        *y = adc_y;
        g_bXY = TRUE;
        return 1;
    }
    g_bXY = FALSE;
    return 0;
}

int Uninit_TouchPanel(void)
{
    adc_close();
    return 1;
}

int Check_TouchPanel(void)
{
    if ( (inp32(REG_ADC_TSC) & ADC_UD) == ADC_UD)
        return 1;   //Pen down;
    else
        return 0;   //Pen up;
}

void TouchTask(void)
{
    static U16 xOld;
    static U16 yOld;
    static U8  PressedOld = 0;
    int x, y, xDiff, yDiff;
    BOOL  Pressed;




    Read_TouchPanel(&g_u16X, &g_u16Y);
    Pressed = g_bXY;// TBD: Insert function which returns:
    //      1, if the touch screen is pressed
    //      0, if the touch screen is released
    //
    // Touch screen is pressed
    //
    if (Pressed)
    {
        x =  g_u16X;// TBD: Insert function which reads current x value
        y =  g_u16Y;// TBD: Insert function which reads current y value
        ts_phy2log(&x, &y);
        //
        // The touch has already been pressed
        //
        if (PressedOld == 1)
        {
            //
            // Calculate difference between new and old position
            //
            xDiff = (x > xOld) ? (x - xOld) : (xOld - x);
            yDiff = (y > yOld) ? (y - yOld) : (yOld - y);
            //
            // Store state if new position differs significantly from old position
            //
            if (xDiff + yDiff > 2)
            {
                xOld = x;
                yOld = y;
                GUI_TOUCH_StoreState(x, y);
                /*
                                    sysprintf("X=%d, Y=%d\n", x,y);
                                    State.x = x;
                                    State.y = y;
                                    State.Layer = 0;
                                    State.Pressed = 1;
                                    GUI_PID_StoreState(&State);
                                    */
            }
        }
        //
        // The touch was previously released
        // Store state regardless position
        //
        else
        {
            if ((x != 0) && (y != 0))
            {
//                  if ( g_u32Release > 1200 )
//                      return;
                xOld = x;
                yOld = y;
                PressedOld = 1;
                GUI_TOUCH_StoreState(x, y);
                /*
                                    sysprintf("X=%d, Y=%d\n", x,y);
                                    State.x = x;
                                    State.y = y;
                                    State.Layer = 0;
                                    State.Pressed = 1;
                                    GUI_PID_StoreState(&State);
                                    */
            }
        }
    }
    else
    {
        if (PressedOld == 1)
        {
            PressedOld = 0;
            GUI_TOUCH_StoreState(-1, -1);
            /*
                            sysprintf("X=-1, Y=-1\n");
                                State.x = 0;
                                State.y = 0;
                                State.Layer = 0;
                                State.Pressed = 0;
                                GUI_PID_StoreState(&State);
                             */
        }
    }
    //
    // Make sure
    //
}
