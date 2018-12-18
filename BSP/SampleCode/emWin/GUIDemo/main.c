#include "N9H20.h"

#include "GUI.h"
#include "LCDconf.h"

#include "WM.h"
#include "TEXT.h"

#include "N9H20TouchPanel.h"

/*********************************************************************
Constraints on LCD panel N9H20_VPOST_FW050TFT_800x480.lib:

1. Resolution is 800x480. (N9H20K1 has no such target because of the DRAM size limitation)
2. VPOST OSD function must disable. (VPOST H/W limitation)
3. PLL clock is 192 MHz, the pixel clock is 192/6 = 32 MHz.
4. 32,000,000/1056/525 = 57.7 FPS (frame per second)
5. If the bus bandwidth condition is too busy it may causes blinking result.
*/

__align(32) UINT8 u8FrameBuf[XSIZE_PHYS*YSIZE_PHYS*2];

UINT8 *u8FrameBufPtr;

extern volatile int OS_TimeMS;
volatile int g_enable_Touch;

extern int ts_writefile(int hFile);
extern int ts_readfile(int hFile);
extern int ts_calibrate(int xsize, int ysize);
extern void TouchTask(void);

#ifndef STORAGE_SD
#define NAND_2      1   // comment to use 1 disk foor NAND, uncomment to use 2 disk
UINT32 StorageForNAND(void);

static NDISK_T ptNDisk;
static NDRV_T _nandDiskDriver0 =
{
    nandInit0,
    nandpread0,
    nandpwrite0,
    nand_is_page_dirty0,
    nand_is_valid_block0,
    nand_ioctl,
    nand_block_erase0,
    nand_chip_erase0,
    0
};

#define NAND1_1_SIZE     32 /* MB unit */

UINT32 StorageForNAND(void)
{

    UINT32 block_size, free_size, disk_size;
    UINT32 u32TotalSize;

    fsAssignDriveNumber('C', DISK_TYPE_SMART_MEDIA, 0, 1);
#ifdef NAND_2
    fsAssignDriveNumber('D', DISK_TYPE_SMART_MEDIA, 0, 2);
#endif

    sicOpen();

    /* Initialize GNAND */
    if(GNAND_InitNAND(&_nandDiskDriver0, &ptNDisk, TRUE) < 0)
    {
        sysprintf("GNAND_InitNAND error\n");
        goto halt;
    }

    if(GNAND_MountNandDisk(&ptNDisk) < 0)
    {
        sysprintf("GNAND_MountNandDisk error\n");
        goto halt;
    }

    /* Get NAND disk information*/
    u32TotalSize = ptNDisk.nZone* ptNDisk.nLBPerZone*ptNDisk.nPagePerBlock*ptNDisk.nPageSize;
    sysprintf("Total Disk Size %d\n", u32TotalSize);
    /* Format NAND if necessery */
#ifdef NAND_2
    if ((fsDiskFreeSpace('C', &block_size, &free_size, &disk_size) < 0) ||
            (fsDiskFreeSpace('D', &block_size, &free_size, &disk_size) < 0))
    {
        sysprintf("unknow disk type, format device .....\n");
        if (fsTwoPartAndFormatAll((PDISK_T *)ptNDisk.pDisk, NAND1_1_SIZE*1024, (u32TotalSize- NAND1_1_SIZE*1024)) < 0)
        {
            sysprintf("Format failed\n");
            goto halt;
        }
        fsSetVolumeLabel('C', "NAND1-1\n", strlen("NAND1-1"));
        fsSetVolumeLabel('D', "NAND1-2\n", strlen("NAND1-2"));
    }
#endif

halt:
    sysprintf("systen exit\n");
    return 0;

}
#endif

/*********************************************************************
*
*       TMR0_IRQHandler
*/
void TMR0_IRQHandler(void)
{
    OS_TimeMS++;
    if ( OS_TimeMS % 20 == 0 )
    {
        if ( g_enable_Touch == 1 )
        {
            TouchTask();
        }
    }
}

/*********************************************************************
*
*       _SYS_Init
*/
static void _SYS_Init(void)
{
    WB_UART_T uart;
    UINT32 u32ExtFreq;
    LCDFORMATEX lcdFormat;

    u32ExtFreq = sysGetExternalClock()*1000;
    sysUartPort(1);
    uart.uart_no = WB_UART_1;
    uart.uiFreq = u32ExtFreq;   //use APB clock
    uart.uiBaudrate = 115200;
    uart.uiDataBits = WB_DATA_BITS_8;
    uart.uiStopBits = WB_STOP_BITS_1;
    uart.uiParity = WB_PARITY_NONE;
    uart.uiRxTriggerLevel = LEVEL_1_BYTE;
    sysInitializeUART(&uart);
    sysSetLocalInterrupt(ENABLE_FIQ_IRQ);

    sysEnableCache(CACHE_WRITE_BACK);

    /* start timer 0 */
    OS_TimeMS = 0;

    sysSetTimerReferenceClock(TIMER0, u32ExtFreq);                  //External Crystal
    sysStartTimer(TIMER0, 1000, PERIODIC_MODE);                     /* 100 ticks/per sec ==> 1tick/10ms */
    sysSetTimerEvent(TIMER0, 1, (PVOID)TMR0_IRQHandler);        /* 1 ticks  call back */

    sysSetLocalInterrupt(ENABLE_IRQ);

    sysprintf("fsInitFileSystem.\n");
    fsInitFileSystem();

#ifdef STORAGE_SD
    /*-----------------------------------------------------------------------*/
    /*  Init SD card                                                         */
    /*-----------------------------------------------------------------------*/
    /* clock from PLL */
    sicIoctl(SIC_SET_CLOCK, sysGetPLLOutputKhz(eSYS_UPLL, u32ExtFreq/1000), 0, 0);
    sicOpen();
    sysprintf("total sectors (%x)\n", sicSdOpen0());
#else
    StorageForNAND();
#endif

// LCD initial
//  LCD_initial();
    lcdFormat.ucVASrcFormat = DRVVPOST_FRAME_RGB565;
    vpostLCMInit(&lcdFormat, (UINT32*)u8FrameBuf);
    u8FrameBufPtr  = (UINT8 *)((UINT32)u8FrameBuf | 0x80000000);
    /* If backlight control signal is different from nuvoton's demo board,
        please don't call this function and must implement another similar one to enable LCD backlight. */
    vpostEnaBacklight();
}

/*********************************************************************
*
*       main
*/
extern void MainTask(void);

int main(void)
{
    char szFileName[20];
    char szCalibrationFile[40];
    int hFile;

#ifdef GUI_SUPPORT_TOUCH
    g_enable_Touch = 0;
#endif

    _SYS_Init();

#ifdef GUI_SUPPORT_TOUCH
    Init_TouchPanel();

    sprintf(szFileName, "C:\\ts_calib");
    fsAsciiToUnicode(szFileName, szCalibrationFile, TRUE);
    hFile = fsOpenFile(szCalibrationFile, szFileName, O_RDONLY | O_FSEEK);
    sysprintf("file = %d\n", hFile);
    if (hFile < 0)
    {
        // file does not exists, so do calibration
        hFile = fsOpenFile(szCalibrationFile, szFileName, O_CREATE|O_RDWR | O_FSEEK);
        if ( hFile < 0 )
        {
            sysprintf("CANNOT create the calibration file\n");
            return -1;
        }
        GUI_Init();
        ts_calibrate(LCD_XSIZE, LCD_YSIZE);
        ts_writefile(hFile);
    }
    else
    {
        ts_readfile(hFile);
    }
    fsCloseFile(hFile);

#ifndef STORAGE_SD
    GNAND_UnMountNandDisk(&ptNDisk);
    sicClose();
#endif
    g_enable_Touch = 1;
#endif

    MainTask();
//    while(1);

//      Uninit_TouchPanel();
//      sysStopTimer(TIMER0);
//    sysDisableCache();
    return 0;
}

/*************************** End of file ****************************/
