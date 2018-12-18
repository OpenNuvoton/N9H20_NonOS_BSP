#ifndef __N9H20TOUCHPANEL_H__
#define __N9H20TOUCHPANEL_H__

/*********************************************************************
Constraints on LCD panel N9H20_VPOST_FW050TFT_800x480.lib:

1. Resolution is 800x480. (N9H20K1 has no such target because of the DRAM size limitation)
2. VPOST OSD function must disable. (VPOST H/W limitation)
3. PLL clock is 192 MHz, the pixel clock is 192/6 = 32 MHz.
4. 32,000,000/1056/525 = 57.7 FPS (frame per second)
5. If the bus bandwidth condition is too busy it may causes blinking result.
*/

#ifdef __800x480__
#define XSIZE_PHYS  800
#define YSIZE_PHYS  480
#else
#ifdef __480x272__
#define XSIZE_PHYS  480
#define YSIZE_PHYS  272
#else
#define XSIZE_PHYS  320
#define YSIZE_PHYS  240
#endif
#endif
#define LCD_XSIZE       XSIZE_PHYS
#define LCD_YSIZE       YSIZE_PHYS


int Init_TouchPanel(void);
int Read_TouchPanel(int *x, int *y);
int Uninit_TouchPanel(void);
int Check_TouchPanel(void);
int Wait_PressDown(void);
#endif
