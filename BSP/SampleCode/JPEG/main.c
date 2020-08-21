/**************************************************************************//**
 * @file     main.c
 * @brief    Demo how to use JPEG driver to encode & decode JPEG file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "N9H20.h"
#include "jpegSample.h"

/*-----------------------------------------------------------------------*/
/*  JPEG Demo Code ReadMe                                                */
/*-----------------------------------------------------------------------*/
/*  Encode Operation                                                     */
/*  Please put the raw data file in SD card root folder, change the	     */ 
/*  definition by Item 7 for the encode size. Select Item 8 to do the    */
/*  Encode operation. In Item 8, please input the file name for encode   */ 
/*  source. After encode operation complete, the jpeg file will write to */
/*  the input file name with file name exetension ".jpg" in SD card root */
/*  folder                                                               */
/*-----------------------------------------------------------------------*/
/*  Decode Operation                                                     */
/*  Please put the bitstream file into SD card root folder. User can     */
/*  change decode properties by the following items                      */
/*      Item 0 Enable/Disable Panel Test                                 */
/*             Downscale to fit the Panel size, and display on Panel     */
/*      Item 1 Enable/Disable Input Wait function                        */
/*      Item 2 Set Decode Output format                                  */
/*      Item 3 Start Docode operation                                    */
/*             Do decode operation after input Jpeg file name            */
/*  After Decode operation complete, the decoded raw data file will      */
/*  write to the input filea name with file name exetension ".dat" into   */
/*  SD card root folder                                                  */ 
/*-----------------------------------------------------------------------*/

BOOL g_bDecPanelTest = FALSE, g_bDecIpwTest = FALSE, g_bEncUpTest = FALSE, g_bEncSwReserveTest = FALSE;

LCDFORMATEX lcdInfo;

INT32 main()
{
    WB_UART_T uart;
    UINT32 u32ExtFreq;
    UINT8 u8Item;
    BOOL bLoop = TRUE;

    /* CACHE_ON	*/
    sysInvalidCache();
    sysEnableCache(CACHE_WRITE_THROUGH);
    sysFlushCache(I_D_CACHE);

    u32ExtFreq = sysGetExternalClock();    /* Hz unit */

    sysUartPort(1);
    uart.uiFreq = u32ExtFreq * 1000;
    uart.uiBaudrate = 115200;
    uart.uiDataBits = WB_DATA_BITS_8;
    uart.uiStopBits = WB_STOP_BITS_1;
    uart.uiParity = WB_PARITY_NONE;
    uart.uiRxTriggerLevel = LEVEL_1_BYTE;
    sysInitializeUART(&uart);

    sysSetSystemClock(eSYS_UPLL,    /* E_SYS_SRC_CLK eSrcClk */
                    192000,         /* UINT32 u32PllKHz */
                    192000,         /* UINT32 u32SysKHz */
                    192000,         /* UINT32 u32CpuKHz */
                    192000/2,       /* UINT32 u32HclkKHz */
                    192000/4);      /* UINT32 u32ApbKHz */
                    
    sysprintf ("\n/*-----------------------------------------------------------------------*/");
    sysprintf ("\n/*  JPEG Demo code                                                       */");
    sysprintf ("\n/*-----------------------------------------------------------------------*/\n");

    /*-----------------------------------------------------------------------*/
    /*  Init FAT file system                                                 */
    /*-----------------------------------------------------------------------*/
    fsInitFileSystem();

    sicIoctl(SIC_SET_CLOCK, 192000, 0, 0);    /* clock from PLL */

    sicOpen();
    if (sicSdOpen0()<=0)
    {
        sysprintf("Error in initializing SD card !!\n");
        while(1);
    }

    /* Init Panel */
    lcdInfo.ucVASrcFormat = DRVVPOST_FRAME_RGB565;
    lcdInfo.nScreenWidth = PANEL_WIDTH;
    lcdInfo.nScreenHeight = PANEL_HEIGHT;

    /* start timer 0 */
    sysSetTimerReferenceClock(TIMER0, u32ExtFreq * 1000);
    sysStartTimer(TIMER0, 1000, PERIODIC_MODE);

    /* JPEG Open */
    jpegOpen ();

    while(bLoop)
    {
        sysprintf("\nPlease Select Test Item\n");
        sysprintf("[*]Decode Test\n");
        sysprintf(" 0 : Panel Test ");g_bDecPanelTest?sysprintf("Disable\n"):sysprintf("Enable\n");
        sysprintf("     -> Decode Downscale to QVGA\n");
        sysprintf("     -> Decode Stride is %d\n",PANEL_WIDTH);
        sysprintf("     -> Output data size is %dx%d\n",PANEL_WIDTH,PANEL_HEIGHT);
        sysprintf(" 1 : Input Wait ");g_bDecIpwTest?sysprintf("Disable\n"):sysprintf("Enable\n");
        sysprintf(" 2 : Set Decode output format\n");
        sysprintf(" 3 : Start to Decode\n");
        sysprintf("     -> Decode output format is ");
        switch(g_u32DecFormat)
        {
            case JPEG_DEC_PRIMARY_PACKET_YUV422:
                sysprintf("PACKET YUV422\n");
                break;
            case JPEG_DEC_PRIMARY_PACKET_RGB555:
                sysprintf("PACKET RGB555\n");
                break;
            case JPEG_DEC_PRIMARY_PACKET_RGB565:
                sysprintf("PACKET RGB565\n");
                break;
            case JPEG_DEC_PRIMARY_PACKET_RGB888:
                sysprintf("PACKET RGB888\n");
                break;
            case JPEG_DEC_PRIMARY_PLANAR_YUV:
                sysprintf("PLANAR format\n");
                break;
        }
        sysprintf("[*]Encode Test\n");
        sysprintf(" 4 : Upscale ");g_bEncUpTest?sysprintf("Disable\n"):sysprintf("Enable\n");
        sysprintf(" 5 : Software Reserved ");g_bEncSwReserveTest?sysprintf("Disable\n"):sysprintf("Enable\n");
        sysprintf(" 6 : Set Encode Width & Height\n");
        sysprintf(" 7 : Start to Encode\n");
        sysprintf("     ->  Encode Size %dx%d\n",g_u32EncWidth,g_u32EncHeight);
        sysprintf(" 8 : Exit\n>");

        u8Item = sysGetChar();

        switch(u8Item)
        {
            case '0':
                if(g_u32DecFormat == JPEG_DEC_PRIMARY_PLANAR_YUV)
                    sysprintf("\n<Not support Planar format Panel Test>\n");
                else
                    g_bDecPanelTest ^= 1;
                break;
            case '1':
                g_bDecIpwTest ^= 1;
                break;
            case '2':
                sysprintf("\nPlease select Decode Output format\n");
                sysprintf(" 0: PACKET YUV422\n");
                sysprintf(" 1: PACKET RGB555\n");
                sysprintf(" 2: PACKET RGB565\n");
                sysprintf(" 3: PACKET RGB888\n");
                sysprintf(" 4: PLANAR format\n");
                sysprintf(">");
                u8Item = sysGetChar();

                switch(u8Item)
                {
                    case '0':
                        g_u32DecFormat = JPEG_DEC_PRIMARY_PACKET_YUV422;
                        break;
                    case '1':
                        g_u32DecFormat = JPEG_DEC_PRIMARY_PACKET_RGB555;
                        break;
                    case '2':
                        g_u32DecFormat = JPEG_DEC_PRIMARY_PACKET_RGB565;
                        break;
                    case '3':
                        g_u32DecFormat = JPEG_DEC_PRIMARY_PACKET_RGB888;
                        break;
                    case '4':
                        if(g_bDecPanelTest)
                            sysprintf("\n<Not support Planar format Panel Test>\n");
                        else
                            g_u32DecFormat = JPEG_DEC_PRIMARY_PLANAR_YUV;
                        break;
                    default:
                        sysprintf("Write Item\n");
                        break;
                }
                break;
            case '3':
                sysprintf("\n<Decode Test>\n");
                sysprintf("   -> Panel Test ");g_bDecPanelTest?sysprintf("Enabled\n"):sysprintf("Disabled\n");
                sysprintf("   -> Input Test ");g_bDecIpwTest?sysprintf("Enabled\n"):sysprintf("Disabled\n");
                JpegDecTest();
                break;
            case '4':
                g_bEncUpTest ^= 1;
                break;
            case '5':
                g_bEncSwReserveTest ^= 1;
                break;
            case '6':
                sysprintf("\nPlease input Encode Width\n");
                g_u32EncWidth = GetData();
                sysprintf("\nPlease input Encode Height\n");
                g_u32EncHeight = GetData();
                break;
            case '7':
                sysprintf("\n<Encode Test>\n");
                sysprintf("   Upscale ");g_bEncUpTest?sysprintf("Enabled\n"):sysprintf("Disabled\n");
                sysprintf("    -> Software Reserved ");g_bEncSwReserveTest?sysprintf("Enabled\n"):sysprintf("Disabled\n");
                sysprintf("    -> Encode Size %dx%d\n",g_u32EncWidth,g_u32EncHeight);
                JpegEncTest();
                break;
            case '8':
                bLoop = FALSE;
                break;
            default:
                sysprintf("Write Item\n");
                break;
        }
    }

    /* JPEG Close */
    jpegClose ();

    sysprintf ("\n/*-----------------------------------------------------------------------*/");
    sysprintf ("\n/*  JPEG Demo code End                                                   */");
    sysprintf ("\n/*-----------------------------------------------------------------------*/\n");

    return 0;

}  /* main */
