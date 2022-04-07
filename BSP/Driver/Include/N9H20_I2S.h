/**************************************************************************//**
 * @file     N9H20_I2S.h
 * @version  V3.00
 * @brief    N9H20 series I2S driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/
 
#ifndef __DRVI2S_H__
#define __DRVI2S_H__

/* Constant Definition */
#define DRVI2S_MAJOR_NUM 2
#define DRVI2S_MINOR_NUM 0
#define DRVI2S_BUILD_NUM 1

#ifdef __GNUC__
//	_SYSINFRA_int_DEF (DRVI2S, PIN_UNAVAILABLE, TRUE, MODULE_ID_DRVI2S, 0);
//	_SYSINFRA_int_DEF (DRVI2S, DISABLED, TRUE, MODULE_ID_DRVI2S, 0);
//	_SYSINFRA_int_DEF (DRVI2S, PIN_UNAVAILABLE, 1, MODULE_ID_DRVI2S, 0);
//	_SYSINFRA_int_DEF (DRVI2S, DISABLED, 1, MODULE_ID_DRVI2S, 0);
#endif	

/* Play & Record Settings */
#define DRVI2S_PLAY					0
#define DRVI2S_RECORD				1

/* FIFO level */
#define DRVI2S_4_Level				0x1
#define DRVI2S_8_Level				0x0

/* I2S Function Flags */
#define DRVI2S_PRESCALE_1			0x0
#define DRVI2S_PRESCALE_2			0x1
#define DRVI2S_PRESCALE_3			0x2
#define DRVI2S_PRESCALE_4			0x3
#define DRVI2S_PRESCALE_5			0x4
#define DRVI2S_PRESCALE_6			0x5
#define DRVI2S_PRESCALE_7			0x6
#define DRVI2S_PRESCALE_8			0x7
#define DRVI2S_PRESCALE_10			0x9
#define DRVI2S_PRESCALE_12			0xB
#define DRVI2S_PRESCALE_14			0xD
#define DRVI2S_PRESCALE_16			0xF
#define DRVI2S_CLK_DIV8				0x0
#define DRVI2S_CLK_DIV12			0x1
#define DRVI2S_FREQ_SEL32			~FS_SEL
#define DRVI2S_FREQ_SEL48			FS_SEL

#define DRVI2S_MCLK_PRS				~MCLK_SEL
#define DRVI2S_MCLK_PLL				MCLK_SEL

#define DRVI2S_IRQ_DMA_COUNTER		IRQ_DMA_CNTER_EN
#define DRVI2S_IRQ_DMA_DATA_ZERO	IRQ_DMA_DATA_ZERO_EN
#define DRVI2S_IRQ_RECORD			R_DMA_IRQ_EN
#define DRVI2S_IRQ_PLAYBACK			P_DMA_IRQ_EN

#define DRVI2S_PLAY_DMA_COUNTER		DMA_CNTER_IRQ
#define DRVI2S_PLAY_DMA_DATA_ZERO	DMA_DATA_ZERO_IRQ
#define DRVI2S_PLAY_FIFO_EMPTY		P_FIFO_EMPTY
#define DRVI2S_PLAY_DMA_END			P_DMA_END_IRQ
#define DRVI2S_PLAY_DMA_MIDDLE		P_DMA_MIDDLE_IRQ

#define DRVI2S_REC_FIFO_FULL		R_FIFO_FULL
#define DRVI2S_REC_DMA_END			R_DMA_END_IRQ
#define DRVI2S_REC_DMA_MIDDLE		R_DMA_MIDDLE_IRQ

typedef int (PFN_DRVI2S_CB_FUNC)(UINT8 *, UINT32);

typedef enum {
				eDRVI2S_FREQ_96000 = 96000, 
				eDRVI2S_FREQ_88200 = 88200, 
				eDRVI2S_FREQ_64000 = 64000, 
				eDRVI2S_FREQ_48000 = 48000, 
				eDRVI2S_FREQ_44100 = 44100, 
				eDRVI2S_FREQ_32000 = 32000, 
				eDRVI2S_FREQ_24000 = 24000, 
				eDRVI2S_FREQ_22050 = 22050, 
				eDRVI2S_FREQ_16000 = 16000, 
				eDRVI2S_FREQ_12000 = 12000, 
				eDRVI2S_FREQ_11025 = 11025, 
				eDRVI2S_FREQ_8000  = 8000	} E_DRVI2S_SAMPLING;
				
typedef enum {
				eDRVI2S_I2S = 0,
				eDRVI2S_MSB_JUSTIFIED		} E_DRVI2S_FORMAT; 

typedef enum {
				eDRVI2S_256FS = 0, 		// MCLK = 256*FS is selected
				eDRVI2S_384FS, 			// MCLK = 384*FS is selected	
				eDRVI2S_512FS			} DRVI2S_SYS_CLOCK_E; 


typedef enum {
                eDRVI2S_PLAY_STEREO = (3 << 12),
                eDRVI2S_PLAY_MONO = (2 << 12),
                eDRVI2S_RECORD_STEREO = (3 << 14),
                eDRVI2S_RECORD_LEFT = (1 << 14),
                eDRVI2S_RECORD_RIGHT = (2 << 14)  } E_DRVI2S_CHANNEL;

typedef struct {
				UINT32 u32BufferAddr;
				UINT32 u32BufferLength;
				E_DRVI2S_SAMPLING eSampleRate;  
			    E_DRVI2S_CHANNEL eChannel;
			    E_DRVI2S_FORMAT eFormat;} S_DRVI2S_PLAY;

typedef struct {
				UINT32 u32BufferAddr;
				UINT32 u32BufferLength;
				E_DRVI2S_SAMPLING eSampleRate;  
			    E_DRVI2S_CHANNEL eChannel;
			    E_DRVI2S_FORMAT eFormat;} S_DRVI2S_RECORD;

VOID DrvI2S_Open(VOID);
VOID DrvI2S_Close(VOID);
VOID DrvI2S_EnableInt(UINT32 u32InterruptFlag, PFN_DRVI2S_CB_FUNC* pfnCallBack);
VOID DrvI2S_DisableInt(UINT32 u32InterruptFlag);
VOID DrvI2S_ClearInt(UINT32	u32InterruptFlag);
UINT32 DrvI2S_PollInt(UINT32 u32InterruptFlag);
VOID DrvI2S_StartPlay (S_DRVI2S_PLAY* psPlayStruct);
VOID DrvI2S_StopPlay (VOID);
VOID DrvI2S_StartRecord (S_DRVI2S_RECORD* psRecord);
VOID DrvI2S_StopRecord (VOID);
INT DrvI2S_SetSampleRate(E_DRVI2S_SAMPLING  eSampleRate);
#endif	// __DRVI2S_H__



















