/**************************************************************************//**
 * @file     N9H20_SPU.h
 * @version  V3.00
 * @brief    N9H20 series SPU driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/
 
#ifndef _N9H20_SPU_H_
#define _N9H20_SPU_H_

#include "wbio.h"
#include "wblib.h"
#include "wbtypes.h"

#define OPT_DIRECT_SET_DFA
/*---------------------------------*/
/*		Constant Declaration	   */
/*---------------------------------*/
#define E_DRVSPU_WRONG_CHANNEL     		-1		/* Wrong channel */
#define E_DRVSPU_WRONG_INTERRUPT   		-2 		/* Wrong interrupt setting */

#define DRVSPU_USER_INT					EV_USR_FG		// User event interrupt enable bit
#define DRVSPU_SILENT_INT				EV_SLN_FG		// Silent event interrupt enable bit
#define DRVSPU_LOOPSTART_INT			EV_LP_FG		// Loop start event interrupt enable bit
#define DRVSPU_END_INT					EV_END_FG		// End event interrupt enable bit
#define DRVSPU_ENDADDRESS_INT			END_FG			// End Address event interrupt enable bit
#define DRVSPU_THADDRESS_INT			TH_FG			// Threshold Address event interrupt enable bit
#define DRVSPU_ALL_INT					EV_USR_FG+EV_SLN_FG+EV_LP_FG+EV_END_FG+END_FG+TH_FG	

#define DRVSPU_LOAD_SELECTED_CHANNEL	0x01	// load selected channel
#define DRVSPU_UPDATE_ALL_SETTINGS		0x02	// update all registers settings in selected channel
#define DRVSPU_UPDATE_PARTIAL_SETTINGS	0x03	// update partial registers settings in selected channel
#define DRVSPU_UPDATE_IRQ_PARTIAL		0x80	// update Interrupt partial
#define DRVSPU_UPDATE_DFA_PARTIAL		0x40	// update DFA partial
#define DRVSPU_UPDATE_PAN_PARTIAL		0x20	// update PAN partial
#define DRVSPU_UPDATE_VOL_PARTIAL		0x10	// update Volume partial
#define DRVSPU_UPDATE_PAUSE_PARTIAL		0x08	// update PAUSE partial
#define DRVSPU_UPDATE_ALL_PARTIALS		0xF0	// update ALL partials

#define DRVSPU_MDPCM_FORMAT				0x00	// source format is MDPCM
#define DRVSPU_LP8_FORMAT				0x01	// source format is LP8
#define DRVSPU_PCM16_FORMAT				0x03	// source format is PCM16
#define DRVSPU_TONE_FORMAT				0x04	// source format is Tone
#define DRVSPU_STEREO_PCM16_LEFT		0x06	// stereo PCM16 left channel [15:0]
#define DRVSPU_STEREO_PCM16_RIGHT		0x07	// stereo PCM16 left channel [31:16]

#define FRAG_SIZE						(32 * 1024)
#define HALF_FRAG_SIZE					(FRAG_SIZE/2)

#define	MIXED_PCM_BUFFER_NUMBER			4
#define MIXER_BUSY						0x0001
#define	MIXER_STEREO					0x0002
#define MIXER_SPU_RAMP_ZERO				0x0008
#define	MIXER_CH_BUSY					0x0001
#define	MIXER_CH_STEREO					0x0002
#define	MIXER_CH_DECAY					0x0004
#define MIXER_CH_PAUSE					0x0008
#define MIXER_CH_FILL_RAW_BUF_AT_CRITICAL	0x0010	// 1-indicate has filled raw buffer when raw buffer ptr is rawbufsize-1
#define S_SPU_PARCON_MODE				BIT16
#define S_SPU_PARCON_STEREO				0xFFFFFFFE
#define S_SPU_PARCON_MONO				0x00000001


#define MIXER_CH_FILL_RAW_BUF_AT_CRITICAL	0x0010	// 1-indicate has filled raw buffer when raw buffer ptr is rawbufsize-1

#define S_SPU_PARCON_MODE				BIT16
#define S_SPU_PARCON_STEREO				0xFFFFFFFE
#define S_SPU_PARCON_MONO				0x00000001

/* ioctls */
#define SPU_IOCTL_SET_BASE_ADDRESS 		0
#define SPU_IOCTL_SET_TH1_ADDRESS		1
#define SPU_IOCTL_SET_TH2_ADDRESS		2
#define SPU_IOCTL_SET_VOLUME			3
#define SPU_IOCTL_SET_MONO				4
#define SPU_IOCTL_GET_FRAG_SIZE			5
#define SPU_IOCTL_SET_STEREO			6

typedef int (PFN_DRVSPU_CB_FUNC)(UINT8 *);
typedef int (*PFN_DRVSPU_INT_CALLBACK)(UINT8*, UINT32);
typedef enum 
{
	eDRVSPU_MDPCM = 0,
	eDRVSPU_LP8,
	eDRVSPU_PCM16 = 3,
	eDRVSPU_TONE_FORMAT = 4,
	eDRVSPU_MONO_PCM16 = 5,
	eDRVSPU_STEREO_PCM16_LEFT = 6,
	eDRVSPU_STEREO_PCM16_RIGHT = 7	
	
} E_DRVSPU_FORMAT;

typedef enum 
{
	eDRVSPU_USER_INT = 0, 
	eDRVSPU_SILENT_INT,
	eDRVSPU_LOOPSTART_INT,
	eDRVSPU_END_INT, 
	eDRVSPU_ENDADDRESS_INT,
	eDRVSPU_THADDRESS_INT	
				
} E_DRVSPU_INT;

typedef enum 
{
	eDRVSPU_EQBAND_DC = 0, 
	eDRVSPU_EQBAND_1, 
	eDRVSPU_EQBAND_2, 
	eDRVSPU_EQBAND_3, 
	eDRVSPU_EQBAND_4, 
	eDRVSPU_EQBAND_5, 
	eDRVSPU_EQBAND_6, 
	eDRVSPU_EQBAND_7, 
	eDRVSPU_EQBAND_8, 
	eDRVSPU_EQBAND_9,
	eDRVSPU_EQBAND_10	
	
} E_DRVSPU_EQ_BAND;
				

typedef enum 
{
	eDRVSPU_CHANNEL_0 = 0, 
	eDRVSPU_CHANNEL_1, 
	eDRVSPU_CHANNEL_2, 
	eDRVSPU_CHANNEL_3, 
	eDRVSPU_CHANNEL_4, 
	eDRVSPU_CHANNEL_5, 
	eDRVSPU_CHANNEL_6, 
	eDRVSPU_CHANNEL_7, 
	eDRVSPU_CHANNEL_8, 
	eDRVSPU_CHANNEL_9,
	eDRVSPU_CHANNEL_10, 
	eDRVSPU_CHANNEL_11, 
	eDRVSPU_CHANNEL_12, 
	eDRVSPU_CHANNEL_13, 
	eDRVSPU_CHANNEL_14, 
	eDRVSPU_CHANNEL_15, 
	eDRVSPU_CHANNEL_16, 
	eDRVSPU_CHANNEL_17, 
	eDRVSPU_CHANNEL_18, 
	eDRVSPU_CHANNEL_19, 
	eDRVSPU_CHANNEL_20, 
	eDRVSPU_CHANNEL_21, 
	eDRVSPU_CHANNEL_22, 
	eDRVSPU_CHANNEL_23, 
	eDRVSPU_CHANNEL_24, 
	eDRVSPU_CHANNEL_25, 
	eDRVSPU_CHANNEL_26, 
	eDRVSPU_CHANNEL_27, 
	eDRVSPU_CHANNEL_28, 
	eDRVSPU_CHANNEL_29, 
	eDRVSPU_CHANNEL_30, 
	eDRVSPU_CHANNEL_31	
	
} E_DRVSPU_CHANNEL;
			
typedef enum 
{
	eDRVSPU_EQGAIN_M7DB = 0, 
	eDRVSPU_EQGAIN_M6DB,
	eDRVSPU_EQGAIN_M5DB,
	eDRVSPU_EQGAIN_M4DB,
	eDRVSPU_EQGAIN_M3DB,
	eDRVSPU_EQGAIN_M2DB,
	eDRVSPU_EQGAIN_M1DB,
	eDRVSPU_EQGAIN_M0DB,
	eDRVSPU_EQGAIN_P1DB,
	eDRVSPU_EQGAIN_P2DB,
	eDRVSPU_EQGAIN_P3DB,
	eDRVSPU_EQGAIN_P4DB,
	eDRVSPU_EQGAIN_P5DB,
	eDRVSPU_EQGAIN_P6DB,
	eDRVSPU_EQGAIN_P7DB,
	eDRVSPU_EQGAIN_P8DB	
	
} E_DRVSPU_EQ_GAIN;

typedef enum 
{
	eDRVSPU_FREQ_48000 = 48000, 
	eDRVSPU_FREQ_44100 = 44100, 
	eDRVSPU_FREQ_32000 = 32000, 
	eDRVSPU_FREQ_24000 = 24000, 
	eDRVSPU_FREQ_22050 = 22050, 
	eDRVSPU_FREQ_16000 = 16000, 
	eDRVSPU_FREQ_12000 = 12000, 
	eDRVSPU_FREQ_11025 = 11025, 
	eDRVSPU_FREQ_8000  = 8000	

} E_DRVSPU_SAMPLING;

typedef struct 
{
	UINT32 u32ChannelIndex;
	UINT8  u8ChannelVolume;
	UINT16  u16PAN;
	UINT8  u8DataFormat;				
	UINT16 u16DFA;				
	UINT32 u32SrcBaseAddr;
	UINT32 u32SrcThresholdAddr;				
	UINT32 u32SrcEndAddr;								
	UINT16 u16SrcSampleRate;												
	UINT16 u16OutputSampleRate;																
} S_CHANNEL_CTRL;

typedef struct 
{
	UINT16 u16PositivePulse;
	UINT16 u16NegativePulse;	
	UINT16 u16PositiveAmp;
	UINT16 u16NegativeAmp;	
	
} S_TONE_CTRL;

typedef union tagFF16_16
{
	UINT32 m_u32Value;
	struct
	{
		UINT16 m_u16Fraction;
		UINT16 m_u16Integer;
	} m_sFF16;
} U_FF16_16;

/*---------------------------------*/
/*		Variable Declaration	   */
/*---------------------------------*/
extern UINT8	*_pucPlayAudioBuff;

/*---------------------------------*/
/*		Function Declaration	   */
/*---------------------------------*/
#define SPU_ENABLE()	outpw(REG_SPU_CTRL, inpw(REG_SPU_CTRL) | 0x01)	
#define SPU_DISABLE()	outpw(REG_SPU_CTRL, inpw(REG_SPU_CTRL) & ~0x01)	
#define SPU_RESET()		outpw(REG_SPU_CTRL, inpw(REG_SPU_CTRL) | 0x00010000); outpw(REG_SPU_CTRL, inpw(REG_SPU_CTRL) & ~0x00010000)
#define	SPU_ISENABLED()	(inpw(REG_SPU_CTRL) | 0x01)	

VOID spuDacOn(UINT8 level);
VOID spuSetDacSlaveMode(void);
VOID spuDacOff(void);
VOID spuStartPlay(PFN_DRVSPU_CB_FUNC *fnCallBack, UINT8 *data);
VOID spuStopPlay(void);
VOID spuIoctl(UINT32 cmd, UINT32 arg0, UINT32 arg1);
VOID spuOpen(UINT32 u32SampleRate);
VOID spuClose (void);
VOID spuEqOpen (E_DRVSPU_EQ_BAND eEqBand, E_DRVSPU_EQ_GAIN eEqGain);
VOID spuEqClose (void);

#endif
