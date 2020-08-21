/**************************************************************************//**
 * @file     N9H20_VPOST.h
 * @version  V3.00
 * @brief    N9H20 series VPOST driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/
 
#ifndef _N9H20_VPOST_H_
#define _N9H20_VPOST_H_

#include "wbtypes.h"
#include "wbio.h"
#include "wblib.h"

//#define PRINTF(...)
#define PRINTF	sysprintf

#define LCM_ERR_ID				0xFFF06000	/* LCM library ID */

/* error code */
#define ERR_NULL_BUF 			(LCM_ERR_ID | 0x04)	//error memory location
#define ERR_NO_DEVICE			(LCM_ERR_ID | 0x05)	//error no device
#define ERR_BAD_PARAMETER       (LCM_ERR_ID | 0x06) //error for bad parameter
#define ERR_POWER_STATE			(LCM_ERR_ID | 0x07)	//error power state control

// Frame buffer data selection
#define DRVVPOST_FRAME_RGB555     0x0   // RGB555
#define DRVVPOST_FRAME_RGB565     0x1   // RGB565
#define DRVVPOST_FRAME_RGBx888    0x2   // Dummy+888
#define DRVVPOST_FRAME_RGB888x    0x3   // 888+Dummy
#define DRVVPOST_FRAME_CBYCRY     0x4   // Cb0  Y0  Cr0  Y1
#define DRVVPOST_FRAME_YCBYCR     0x5   // Y0  Cb0  Y1  Cr0
#define DRVVPOST_FRAME_CRYCBY     0x6   // Cr0  Y0  Cb0  Y1
#define DRVVPOST_FRAME_YCRYCB     0x7   // Y0  Cr0  Y1  Cb0

// LCD and TV source selection
#define	DRVVPOST_COLOR_LINEBUFFER       0x0 // Line Buffer
#define	DRVVPOST_COLOR_FRAMEBUFFER      0x1 // Frame Buffer
#define	DRVVPOST_COLOR_REGISTERSETTING  0x2 // Register Setting Color
#define	DRVVPOST_COLOR_COLORBAR         0x3 // Internal Color Bar

#define DRVVPOST_CLK_13_5MHZ	0x0       // TV Color Modulation Method (Using 13.5MHz)
#define DRVVPOST_CLK_27MHZ      0x1       // TV Color Modulation Method (Using 27MHz)
#define DRVVPOST_CLK_PLL        0x4       // TV Color Modulation Method (Using PLL output)

// GPU size setting
#define DRVVPOST_VGA_SIZE			0x1   
#define DRVVPOST_QVGA_SIZE			0x0

// Frame Buffer size setting
#define DRVVPOST_FRAME_BUFFER_D1_SIZE		0x3   
#define DRVVPOST_FRAME_BUFFER_VGA_SIZE		0x2   
#define DRVVPOST_FRAME_BUFFER_HVGA_SIZE		0x1   
#define DRVVPOST_FRAME_BUFFER_QVGA_SIZE		0x0   

// LCD output path selection
#define DRVVPOST_DATA_2_EBI					0x00   
#define DRVVPOST_DATA_2_LCD					0x01   
#define DRVVPOST_DATA_2_EBI_PLUS_LCD		0x02   

// MPU Command & Display Mode selection
#define DRVVPOST_MPU_CMD_MODE				0x1
#define DRVVPOST_MPU_NORMAL_MODE			0x0

// Data Format Transfer selection
#define DRVVPOST_DataFormatTransfer_RGB888	0x1
#define DRVVPOST_DataFormatTransfer_YCbCr	0x0

// VPOST interrupt Flag
#define DRVVPOST_HINT						0x1
#define DRVVPOST_VINT						0x2
#define DRVVPOST_TVFIELDINT					0x4
#define DRVVPOST_MPUCPLINT					0x10

// LCD data interface selection
#define DRVVPOST_LCDDATA_YUV422   0x0   // YUV422(CCIR601)
#define DRVVPOST_LCDDATA_RGB      0x1   // Dummy serial (R, G, B Dummy) (default)
#define DRVVPOST_LCDDATA_CCIR656  0x2   // CCIR656 interface
#define DRVVPOST_LCDDATA_RGBGBR   0x4   // Odd line data is RGB, even line is GBR
#define DRVVPOST_LCDDATA_BGRRBG   0x5   // Odd line data is BGR, even line is RBG
#define DRVVPOST_LCDDATA_GBRRGB   0x6   // Odd line data is GBR, even line is RGB
#define DRVVPOST_LCDDATA_RBGBGR   0x7   // Odd line data is RBG, even line is BGR

// LCD device type
#define DRVVPOST_LCD_HIGHRESOLUTION  0x0  // High Resolution mod
#define DRVVPOST_LCD_SYNCTYPE_TFT    0x1  // Sync-type TFT LCD
#define DRVVPOST_LCD_STNCTYPE_STN    0x2  // Sync-type Color STN LCD
#define DRVVPOST_LCD_MPUTYPE         0x3  // MPU-type LCD

#define OPT_MAXIMUM_LCM_REG		256
// LCD information structure
typedef struct
{
    UINT32 ucVASrcFormat;         //User input Display source format
    UINT32 nScreenWidth;          //Driver output,LCD width
    UINT32 nScreenHeight;         //Driver output,LCD height
    UINT32 nFrameBufferSize;      //Driver output,Frame buffer size(malloc by driver)
    UINT8 ucROT90;                //Rotate 90 degree or not
}LCDFORMATEX,*PLCDFORMATEX;


typedef enum {
				eDRVVPOST_HINT = 0x01, 
				eDRVVPOST_VINT = 0x02,
				eDRVVPOST_TVFIELDINT = 0x04,
				eDRVVPOST_MPUCPLINT = 0x10
											} E_DRVVPOST_INT;


typedef enum {
				eDRVVPOST_SYNC_TV = 0, 
				eDRVVPOST_ASYNC_TV
											} E_DRVVPOST_TIMING_TYPE;
											
typedef enum {
				eDRVVPOST_RESERVED = 0, 
				eDRVVPOST_FRAME_BUFFER,
				eDRVVPOST_REGISTER_SETTING,				
				eDRVVPOST_COLOR_BAR
											} E_DRVVPOST_IMAGE_SOURCE;
											
											
typedef enum {
				eDRVVPOST_DUPLICATED = 0, 
				eDRVVPOST_INTERPOLATION
											} E_DRVVPOST_IMAGE_SCALING;
											
typedef enum {
				eDRVVPOST_HIGH_RESOLUTINO_SYNC = 0,	// 16/18/24-bit sync type LCM
				eDRVVPOST_SYNC = 1, 				// 8-bit sync type LCM				
				eDRVVPOST_MPU = 3					// 8/9/16/18/24-bit MPU type LCM

											} E_DRVVPOST_LCM_TYPE;

typedef enum {
				eDRVVPOST_I80 = 0,	
				eDRVVPOST_M68
											} E_DRVVPOST_MPU_TYPE;
											
typedef enum {
				eDRVVPOST_SRGB_YUV422 = 0,	// CCIR601
				eDRVVPOST_SRGB_RGBDUMMY, 	// RGB Dummy Mode
				eDRVVPOST_SRGB_CCIR656,		// CCIR656
				eDRVVPOST_SRGB_RGBTHROUGH	// RGB ThroughMode				
											} E_DRVVPOST_8BIT_SYNCLCM_INTERFACE;

typedef enum {
				eDRVVPOST_CCIR656_360 = 0,	// CCIR656 360 mode
				eDRVVPOST_CCIR656_320 		// CCIR656 320 mode
											} E_DRVVPOST_CCIR656_MODE;

typedef enum {
				eDRVVPOST_YUV_BIG_ENDIAN = 0,	// Bigh endian
				eDRVVPOST_YUV_LITTLE_ENDIAN 	// Little endian
											} E_DRVVPOST_ENDIAN;

typedef enum {
				eDRVVPOST_SRGB_RGB = 0,		
				eDRVVPOST_SRGB_BGR, 		
				eDRVVPOST_SRGB_GBR,			
				eDRVVPOST_SRGB_RBG		
											} E_DRVVPOST_SERAIL_SYNCLCM_COLOR_ORDER;

typedef enum {
				eDRVVPOST_PRGB_16BITS = 0,		
				eDRVVPOST_PRGB_18BITS, 		
				eDRVVPOST_PRGB_24BITS			
											} E_DRVVPOST_PARALLEL_SYNCLCM_INTERFACE;

typedef enum {
				eDRVVPOST_SYNC_8BITS = 0,		
				eDRVVPOST_SYNC_9BITS,
				eDRVVPOST_SYNC_16BITS,				
				eDRVVPOST_SYNC_18BITS,		
				eDRVVPOST_SYNC_24BITS						
											} E_DRVVPOST_SYNCLCM_DATABUS;

typedef enum {
				eDRVVPOST_MPU_8_8 = 0,
				eDRVVPOST_MPU_2_8_8,
				eDRVVPOST_MPU_6_6_6,
				eDRVVPOST_MPU_8_8_8,
				eDRVVPOST_MPU_9_9,
				eDRVVPOST_MPU_16,
				eDRVVPOST_MPU_16_2,
				eDRVVPOST_MPU_2_16,
				eDRVVPOST_MPU_16_8,
				eDRVVPOST_MPU_18,
				eDRVVPOST_MPU_18_6,
				eDRVVPOST_MPU_24
											} E_DRVVPOST_MPULCM_DATABUS;


typedef enum {
				eDRVVPOST_FRAME_RGB555 = 0,		
				eDRVVPOST_FRAME_RGB565, 		
				eDRVVPOST_FRAME_RGBx888,
				eDRVVPOST_FRAME_RGB888x,
				eDRVVPOST_FRAME_CBYCRY,
				eDRVVPOST_FRAME_YCBYCR,
				eDRVVPOST_FRAME_CRYCBY,
				eDRVVPOST_FRAME_YCRYCB
											} E_DRVVPOST_FRAME_DATA_TYPE;

typedef struct {
				UINT8 u8PulseWidth;  	 	// Horizontal sync pulse width
				UINT8 u8BackPorch;    		// Horizontal back porch
				UINT8 u8FrontPorch;    		// Horizontal front porch
											} S_DRVVPOST_SYNCLCM_HTIMING;

typedef struct {
				UINT8 u8PulseWidth;   		// Vertical sync pulse width
				UINT8 u8BackPorch;    		// Vertical back porch
				UINT8 u8FrontPorch;    		// Vertical front porch
											} S_DRVVPOST_SYNCLCM_VTIMING;

typedef struct {
				UINT16 u16ClockPerLine;		// Specify the number of pixel clock in each line or row of screen
				UINT16 u16LinePerPanel;    	// Specify the number of active lines per screen
				UINT16 u16PixelPerLine;   	// Specify the number of pixel in each line or row of screen
											} S_DRVVPOST_SYNCLCM_WINDOW;

typedef enum {
				eDRVVPOST_DATA_8BITS = 0,		
				eDRVVPOST_DATA_9BITS,
				eDRVVPOST_DATA_16BITS,			
				eDRVVPOST_DATA_18BITS,					
				eDRVVPOST_DATA_24BITS		
											} E_DRVVPOST_DATABUS;

typedef struct {
				BOOL bIsVsyncActiveLow;		// Vsync polarity
				BOOL bIsHsyncActiveLow;		// Hsync polarity
				BOOL bIsVDenActiveLow;		// VDEN polarity				
				BOOL bIsDClockRisingEdge;	// pixel clock polarity								
											} S_DRVVPOST_SYNCLCM_POLARITY;

// for MVsync & FrameMark signal
typedef struct {
				UINT16 u16LinePerPanel;    	// Specify the number of active lines per screen
				UINT16 u16PixelPerLine;   	// Specify the number of pixel in each line or row of screen
											} S_DRVVPOST_MPULCM_WINDOW; 	

typedef struct {
				UINT8  u8CSnF2DCt;			// CSn fall edge to Data change clock counter
				UINT8  u8WRnR2CSnRt;		// WRn rising edge to CSn rising  clock counter
				UINT8  u8WRnLWt;			// WR Low pulse clock counter
				UINT8  u8CSnF2WRnFt;		// Csn fall edge To WR falling edge clock counter
											} S_DRVVPOST_MPULCM_TIMING;


typedef struct {
				BOOL  	bIsSyncWithTV;				
				BOOL  	bIsVsyncSignalOut;
				BOOL  	bIsFrameMarkSignalIn;				
				E_DRVVPOST_IMAGE_SOURCE		eSource;
				E_DRVVPOST_LCM_TYPE			eType;
				E_DRVVPOST_MPU_TYPE			eMPUType;				
				E_DRVVPOST_MPULCM_DATABUS	eBus;
				S_DRVVPOST_MPULCM_WINDOW*	psWindow;			
				S_DRVVPOST_MPULCM_TIMING*	psTiming;
				
												} S_DRVVPOST_MPULCM_CTRL;

#if 1
#define DRVVPOST_CLK_LOW			 0x0  		// clock active low
#define DRVVPOST_CLK_HIGH		     0x1  		// clock active high

#define DRVVPOST_REG_NO_INDEX		0xFFCC	 	// need not send RegIndex for LCD register setting
#define DRVVPOST_REG_NO_DATA		0xFFCD	 	// need not send RegData for LCD register setting
#define DRVVPOST_REG_DELAY			0xFFDD	 	// delay (ms) ID for LCD register setting
//#define DRVVPOST_REG_END			0xFFFF	 	// end ID for LCD register setting

typedef enum {
				eDRVVPOST_SPI = 0,		
				eDRVVPOST_I2C,
				eDRVVPOST_NONE
											} E_DRVVPOST_CONFIG_INTERFACE;

typedef struct {
				UINT32	u32PixelPerLine;
				UINT32	u32LinePerPanel;
											} S_DRVVPOST_LCM_WINDOW;
											
typedef enum {
				eDRVVPOST_GPA = 0,		
				eDRVVPOST_GPB,
				eDRVVPOST_GPC,
				eDRVVPOST_GPD,
				eDRVVPOST_GPE,
				eDRVVPOST_GPF,
				eDRVVPOST_GPG,
				eDRVVPOST_GPH
											} E_DRVVPOST_PORT_SELECT;

typedef enum {
				eDRVVPOST_PIN_0 = 0,		
				eDRVVPOST_PIN_1,
				eDRVVPOST_PIN_2,
				eDRVVPOST_PIN_3,
				eDRVVPOST_PIN_4,
				eDRVVPOST_PIN_5,
				eDRVVPOST_PIN_6,
				eDRVVPOST_PIN_7,
				eDRVVPOST_PIN_8,
				eDRVVPOST_PIN_9,
				eDRVVPOST_PIN_10,
				eDRVVPOST_PIN_11,
				eDRVVPOST_PIN_12,
				eDRVVPOST_PIN_13,
				eDRVVPOST_PIN_14,
				eDRVVPOST_PIN_15
											} E_DRVVPOST_PIN_SELECT;

typedef struct {
				UINT32	u32RegIndex;		// regsiter index
				UINT32	u32RegData;			// register data
											} S_DRVVPOST_LCM_REG;

typedef enum {
				eDRVVPOST_PIN_LOW = 0,		
				eDRVVPOST_PIN_HI
											} E_DRVVPOST_PIN_STATE;
typedef enum {
             eDRVVPOST_SPI_RW_ADDR_DATA = 0,
             eDRVVPOST_SPI_ADDR_RW_DATA,
             eDRVVPOST_SPI_RESERVED
                       } E_DRVVPOST_SPI_TYPE;
											
typedef struct {
				E_DRVVPOST_PORT_SELECT	eCSPort;			// CS port	
				E_DRVVPOST_PIN_SELECT	eCSPin;				// CS pin
				E_DRVVPOST_PORT_SELECT	eClkPort;			// clock port	
				E_DRVVPOST_PIN_SELECT	eClkPin;			// clock pin				
				E_DRVVPOST_PORT_SELECT	eDataPort;			// data port	
				E_DRVVPOST_PIN_SELECT	eDataPin;			// data pin
				E_DRVVPOST_SPI_TYPE		eSpiType;			// reserved
				UINT8	u8RegIndexWidth;					// regsiter index width, 8/16/24
				UINT8	u8RegDataWidth;						// regsiter data width, 8/16/24
											} S_DRVVPOST_SPI_CONFIGURE;

typedef struct {
				UINT8	u8I2C_ID;							// I2C ID address
				E_DRVVPOST_PORT_SELECT	eClkPort;			// clock port	
				E_DRVVPOST_PIN_SELECT	eClkPin;			// clock pin				
				E_DRVVPOST_PORT_SELECT	eDataPort;			// data port	
				E_DRVVPOST_PIN_SELECT	eDataPin;			// data pin
				UINT8	u8RegIndexWidth;					// regsiter index width, 8/16/24
				UINT8	u8RegDataWidth;						// regsiter data width, 8/16/24
											} S_DRVVPOST_I2C_CONFIGURE;

typedef struct {
				E_DRVVPOST_CONFIG_INTERFACE eInterface;		// SPI, I2C or don't configure (eDRVVPOST_NONE)
				S_DRVVPOST_SPI_CONFIGURE* psSpiConfig;		// SPI configuration
				S_DRVVPOST_I2C_CONFIGURE* psI2cConfig;		// I2C configuration
				S_DRVVPOST_LCM_REG* psLcmReg;				// LCM regsiter table
				UINT32	u32RegNum;							// total regsiters number
											} S_DRVVPOST_CONFIGURE;

typedef struct {
				BOOL	bDoNeedEnaPort;						// check whether need to control backlight ENA pin
				E_DRVVPOST_PORT_SELECT	eEnaPort;			// CS port	
				E_DRVVPOST_PIN_SELECT	eEnaPin;			// CS pin
				E_DRVVPOST_PIN_STATE	eEnaStae;
				BOOL	bDoNeedPWMPort;						// check whether need to control backlight PWM pin
				E_DRVVPOST_PORT_SELECT	ePWMPort;			// clock port	
				E_DRVVPOST_PIN_SELECT	ePWMPin;			// clock pin
				E_DRVVPOST_PIN_STATE	ePWMState;				
											} S_DRVVPOST_BACKLIGHT_CTRL;
											
typedef struct {
				BOOL	bDoNeedReset;						// check whether need to reset LCM
				E_DRVVPOST_PORT_SELECT	eResetPort;			// RESET port	
				E_DRVVPOST_PIN_SELECT	eResetPin;			// RESET pin
				E_DRVVPOST_PIN_STATE	eResetStae;
											} S_DRVVPOST_RESET_CTRL;
											
typedef struct {
				BOOL bHClock;				// Hsync polarity
				BOOL bVClock;				// Vsync polarity
				BOOL bVDenClock;			// VDEN polarity				
				BOOL bPixelClock;			// pixel clock polarity
											} S_DRVVPOST_CLK_POLARITY;

typedef enum {
				eDRVVPOST_RGB565 = 0,
				eDRVVPOST_RGB666,
				eDRVVPOST_RGB888,
				eDRVVPOST_CCIR601,
				eDRVVPOST_RGBDummy,
				eDRVVPOST_CCIR656,
				eDRVVPOST_RGBThrough
											} E_DRVVPOST_SYNCLCM_INTERFACE;

typedef enum {
				eDRVVPOST_HSYNC_VSYNC_DE = 0,	// LCM needs Hsync, Vsync and DE pins.
				eDRVVPOST_HSYNC_VSYNC_ONLY,		// LCM only needs Hsync and Vsync pins
				eDRVVPOST_DE_ONLY				// LCM only needs DE pin
											} E_DRVVPOST_HSYNC_VSYNC_DE_MODE;

typedef struct {
				UINT32	u32BufAddr;			// buffer starting address
				UINT16  u16HSize;			// pixel number per line
				UINT16  u16VSize;    		// lines number per screen
				UINT32	u32PixClock;		// pixel clock freq (KHz), for example, 9000 (9MHz)
				E_DRVVPOST_FRAME_DATA_TYPE	eDataFormat;	// eDRVVPOST_FRAME_RGB555, ...
				S_DRVVPOST_SYNCLCM_HTIMING* psHTiming;		// {H_sync. H_BackPorch, H_FrontPorch}
				S_DRVVPOST_SYNCLCM_VTIMING* psVTiming;		// {V_sync. V_BackPorch, V_FrontPorch}
				S_DRVVPOST_CLK_POLARITY* psClockPolarity;
				E_DRVVPOST_SYNCLCM_INTERFACE eSyncInterface;// eDRVVPOST_CCIR601, ...
				BOOL	bDoNeedConfigLcm;					// check whether need to configure LCM internal registers
				S_DRVVPOST_CONFIGURE*		psConfig;		// configure LCM related registers
				BOOL	bDoNeedCtrlBacklight;				// check whether need to control LCM backlight
				S_DRVVPOST_BACKLIGHT_CTRL*	psBacklight;	// control LCM backlight
				S_DRVVPOST_RESET_CTRL*		psReset;		// control Reset pin
				E_DRVVPOST_HSYNC_VSYNC_DE_MODE eHVDeMode;	// Hsync, Vsync and DE pins selection
									} S_DRVVPOST_SYNCLCM_INIT;

typedef struct {
				UINT32	u32BufAddr;			// buffer starting address
				UINT16  u16HSize;			// pixel number per line
				UINT16  u16VSize;    		// lines number per screen
				UINT32	u32PixClock;		// pixel clock freq (KHz), for example, 9000 (9MHz)
				E_DRVVPOST_FRAME_DATA_TYPE	eDataFormat;	// eDRVVPOST_FRAME_RGB555, ...
				E_DRVVPOST_MPULCM_DATABUS	eBusType;		// MPU data bus type, 8+8, 16+0, ...
				BOOL	bDoNeedCtrlBacklight;				// check whether need to control LCM backlight
				S_DRVVPOST_BACKLIGHT_CTRL*	psBacklight;	// control LCM backlight
				S_DRVVPOST_RESET_CTRL*		psReset;		// control Reset pin	
	
				S_DRVVPOST_LCM_REG* psLcmReg;				// LCM regsiter table
				UINT32	u32RegNum;							// total regsiters number

									} S_DRVVPOST_MPULCM_INIT;

typedef enum {
				eDRVVPOST_LCM_TYPE_OFFSET = 0,
				eDRVVPOST_RESOLUTION_OFFSET = 1,
				eDRVVPOST_CLOCK_FREQ_OFFSET = 3,
				eDRVVPOST_DATA_TYPE_OFFSET = 4,
				eDRVVPOST_HTIMING_OFFSET = 5,
				eDRVVPOST_VTIMING_OFFSET = 8,
				eDRVVPOST_SYNC_INTERFACE_OFFSET = 9,
				eDRVVPOST_MPU_INTERFACE_OFFSET = 10,
				eDRVVPOST_POLARITY_OFFSET = 11,
				eDRVVPOST_CONFIG_INTERFACE_OFFSET = 12,
				eDRVVPOST_SPI_CONFIG_OFFSET = 13,
				eDRVVPOST_I2C_CONFIG_OFFSET = 16,
				eDRVVPOST_BACKLIGHT_CTRL_OFFSET = 18,
				eDRVVPOST_RESET_CTRL_OFFSET = 20,
				eDRVVPOST_HSYNC_VSYNC_DE_OFFSET = 21,
				eDRVVPOST_REG_NUM_OFFSET = 52,
				eDRVVPOST_REG_OFFSET = 53
									} E_DRVVPOST_INIT_OFFSET;
											
#endif												

typedef struct 
{
	UINT16 u16VSize;			// Specify OSD vertical size
	UINT16 u16HSize;    		// Specify OSD horizontal size

} S_DRVVPOST_OSD_SIZE;

typedef struct 
{
	UINT16 u16VStart_1st;		// Specify 1st OSD bar Vertical START position
	UINT16 u16VEnd_1st;			// Specify 1st OSD bar Vertical END position
	UINT16 u16VOffset_2nd;		// Specify 2st OSD bar Vertiacal OFFSET position (2nd_Start - 1st_End)
	UINT16 u16HStart_1st;		// Specify 1st OSD bar Horizontal START position
	UINT16 u16HEnd_1st;			// Specify 1st OSD bar Horizontal END position
	UINT16 u16HOffset_2nd;		// Specify 2st OSD bar Horizontal OFFSET position (2nd_Start - 1st_End)

} S_DRVVPOST_OSD_POS;

typedef enum 
{
	eDRVVPOST_OSD_RGB555 = 8,		
	eDRVVPOST_OSD_RGB565 = 9, 		
	eDRVVPOST_OSD_RGBx888 = 10,
	eDRVVPOST_OSD_RGB888x = 11,
	eDRVVPOST_OSD_ARGB888 = 12,	
	eDRVVPOST_OSD_CB0Y0CR0Y1 = 0,
	eDRVVPOST_OSD_Y0CB0Y1CR0 = 1,	
	eDRVVPOST_OSD_CR0Y0CB0Y1 = 2,
	eDRVVPOST_OSD_Y0CR0Y1CB0 = 3,	
	eDRVVPOST_OSD_Y1CR0Y0CB0 = 4,		
	eDRVVPOST_OSD_CR0Y1CB0Y0 = 5,			
	eDRVVPOST_OSD_Y1CB0Y0CR0 = 6,		
	eDRVVPOST_OSD_CB0Y1CR0Y0 = 7
	
} E_DRVVPOST_OSD_DATA_TYPE;

typedef struct 
{
	BOOL	bIsOSDEnabled;
	UINT32	u32Address;
	E_DRVVPOST_OSD_DATA_TYPE 	eType;
	S_DRVVPOST_OSD_SIZE* 		psSize;
	S_DRVVPOST_OSD_POS* 		psPos;
				
} S_DRVVPOST_OSD_CTRL;

typedef enum 
{
	eDRVVPOST_OSD_TRANSPARENT_RGB565 = 0, 		
	eDRVVPOST_OSD_TRANSPARENT_YUV,
	eDRVVPOST_OSD_TRANSPARENT_RGB888
	
} E_DRVVPOST_OSD_TRANSPARENT_DATA_TYPE;

typedef struct 
{
	E_DRVVPOST_OSD_DATA_TYPE 	eType;
	S_DRVVPOST_OSD_SIZE* 		psSize;
	S_DRVVPOST_OSD_POS* 		psPos;
} S_DRVVPOST_OSD;

typedef int (*PFN_DRVVPOST_INT_CALLBACK)(UINT8*, UINT32);

/***********************************
* Function Prototype List  *
***********************************/
extern VOID* g_VAFrameBuf;
extern VOID* g_VAOrigFrameBuf;

VOID vpostSetFrameBuffer(UINT32 pFramebuf);
VOID *vpostGetFrameBuffer(void);
INT32 vpostLCMDeinit(void);
INT32 vpostLCMInit(PLCDFORMATEX plcdformatex, UINT32 *pFramebuf);
VOID vpostEnaBacklight(void);
void vpostSetOSD_Enable(void);
void vpostSetOSD_Disable(void);
void vpostSetOSD_Size(S_DRVVPOST_OSD_SIZE* psSize);
void vpostSetOSD_Pos(S_DRVVPOST_OSD_POS* psPos);
void vpostSetOSD_DataType(E_DRVVPOST_OSD_DATA_TYPE eType);
void vpostSetOSD_Transparent_Enable(void);
void vpostSetOSD_Transparent_Disable(void);
int vpostSetOSD_Transparent(E_DRVVPOST_OSD_TRANSPARENT_DATA_TYPE eType, UINT32 u32Pattern);
void vpostSetOSD_BaseAddress(UINT32 u32BaseAddress);
void N9HxxLCMInit(UINT32* Vpost_Frame, UINT8* Lcm_Data);
void vpostSyncLCMInit(S_DRVVPOST_SYNCLCM_INIT* pLCM);
void vpostMpuLCMInit(S_DRVVPOST_MPULCM_INIT* pMpuLCM);

VOID vpostVAStartTrigger(void);
VOID vpostVAStopTrigger(void);
VOID vpostVAStartTrigger_MPUContinue(void);
VOID vpostVAStartTrigger_MPUSingle(void);
VOID vpostVAStopTriggerMPU(void);
BOOL vpostAllocVABuffer(PLCDFORMATEX plcdformatex,UINT32 nBytesPixel);
BOOL vpostAllocVABufferFromAP(UINT32 *pFramebuf);
BOOL vpostClearVABuffer(void);
BOOL vpostFreeVABuffer(void);
VOID vpostSetLCDEnable(BOOL bYUVBL, UINT8 ucVASrcType, BOOL bLCDRun);
VOID vpostSetLCDConfig(BOOL bLCDSynTv, UINT8 u8LCDDataSel, UINT8 u8LCDTYPE);
VOID vpostsetLCM_TimingType(E_DRVVPOST_TIMING_TYPE eTimingTpye);
VOID vpostSetLCM_TypeSelect(E_DRVVPOST_LCM_TYPE eType);
VOID vpostSetSerialSyncLCM_Interface(E_DRVVPOST_8BIT_SYNCLCM_INTERFACE eInterface);
VOID vpostSetSerialSyncLCM_ColorOrder(
	E_DRVVPOST_SERAIL_SYNCLCM_COLOR_ORDER eEvenLineOrder,
	E_DRVVPOST_SERAIL_SYNCLCM_COLOR_ORDER eOddLineOrder	
);
VOID vpostSetSerialSyncLCM_CCIR656ModeSelect(E_DRVVPOST_CCIR656_MODE eMode);
VOID vpostSetParalelSyncLCM_Interface(E_DRVVPOST_PARALLEL_SYNCLCM_INTERFACE eInterface);
VOID vpostSetFrameBuffer_DataType(E_DRVVPOST_FRAME_DATA_TYPE eType);
VOID vpostSetFrameBuffer_BaseAddress(UINT32 u32BufferAddress);
VOID vpostSetYUVEndianSelect(E_DRVVPOST_ENDIAN eEndian);
VOID vpostSetDataBusPin(E_DRVVPOST_DATABUS eDataBus);
VOID vpostSetDataBusPin_noDE(E_DRVVPOST_DATABUS eDataBus);
VOID vpostSetDataBusPin_onlyDE(E_DRVVPOST_DATABUS eDataBus);
VOID vpostSetSyncLCM_HTiming(S_DRVVPOST_SYNCLCM_HTIMING *psHTiming);
VOID vpostSetSyncLCM_VTiming(S_DRVVPOST_SYNCLCM_VTIMING *psVTiming);
VOID vpostSetSyncLCM_ImageWindow(S_DRVVPOST_SYNCLCM_WINDOW *psWindow);
VOID vpostSetSyncLCM_SignalPolarity(S_DRVVPOST_SYNCLCM_POLARITY *psPolarity);
VOID vpostSetLCM_ImageSource(E_DRVVPOST_IMAGE_SOURCE eSource);
VOID vpostMPULCDWriteAddr16Bit(unsigned short u16AddrIndex);
VOID vpostMPULCDWriteData16Bit(unsigned short  u16WriteData);
VOID vpostEnableInt(E_DRVVPOST_INT eInt);
VOID vpostDisableInt(E_DRVVPOST_INT eInt);
VOID vpostClearInt(E_DRVVPOST_INT eInt);
BOOL vpostIsIntEnabled(E_DRVVPOST_INT eInt);
int vpostInstallCallBack(
	E_DRVVPOST_INT eIntSource,
	PFN_DRVVPOST_INT_CALLBACK	pfnCallback,
	PFN_DRVVPOST_INT_CALLBACK 	*pfnOldCallback
);
VOID vpostSetMPULCM_ImageWindow(S_DRVVPOST_MPULCM_WINDOW *psWindow);
VOID vpostSetMPULCM_TimingConfig(S_DRVVPOST_MPULCM_TIMING *psTiming);
VOID vpostSetMPULCM_BusModeSelect(E_DRVVPOST_MPULCM_DATABUS eBusMode);
VOID vpostEnaBacklight(void);
							
#endif   /* _N9H20_VPOST_H_  */

