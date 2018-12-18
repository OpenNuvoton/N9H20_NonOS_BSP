Constraints on LCD panel N9H20_VPOST_FW050TFT_800x480.lib:

1. Resolution is 800x480. (N9H20K1 has no such target because of the DRAM size limitation)
2. VPOST OSD function must disable. (VPOST H/W limitation)
3. PLL clock is 192 MHz, the pixel clock is 192/6 = 32 MHz.
4. 32,000,000/1056/525 = 57.7 FPS (frame per second)
5. If the bus bandwidth condition is too busy it may causes blinking result.