/**************************************************************************//**
 * @file     DrvSPU.h
 * @version  V3.00
 * @brief    N9H20 series VPOST driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#ifndef _DRVSPU_H_
#define _DRVSPU_H_

//#define OPT_FPGA_DEBUG
#define OPT_DIRECT_SET_DFA

#include "wbio.h"

#ifdef  __cplusplus
extern "C"
{
#endif

#define E_DRVSPU_WRONG_CHANNEL     -1 /* Wrong channel */
#define E_DRVSPU_WRONG_INTERRUPT   -2 /* Wrong interrupt setting */

#define DRVSPU_USER_INT			EV_USR_FG		// User event interrupt enable bit
#define DRVSPU_SILENT_INT		EV_SLN_FG		// Silent event interrupt enable bit
#define DRVSPU_LOOPSTART_INT	EV_LP_FG		// Loop start event interrupt enable bit
#define DRVSPU_END_INT			EV_END_FG		// End event interrupt enable bit
#define DRVSPU_ENDADDRESS_INT	END_FG			// End Address event interrupt enable bit
#define DRVSPU_THADDRESS_INT	TH_FG			// Threshold Address event interrupt enable bit
#define DRVSPU_ALL_INT			EV_USR_FG+EV_SLN_FG+EV_LP_FG+EV_END_FG+END_FG+TH_FG	

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


typedef enum {
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
				eDRVSPU_EQBAND_10	} E_DRVSPU_EQ_BAND;
				
typedef enum {
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
				eDRVSPU_CHANNEL_31	} E_DRVSPU_CHANNEL;
				

typedef enum {
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
				eDRVSPU_EQGAIN_P8DB	} E_DRVSPU_EQ_GAIN;

typedef enum {
				eDRVSPU_FREQ_48000 = 48000, 
				eDRVSPU_FREQ_44100 = 44100, 
				eDRVSPU_FREQ_32000 = 32000, 
				eDRVSPU_FREQ_24000 = 24000, 
				eDRVSPU_FREQ_22050 = 22050, 
				eDRVSPU_FREQ_16000 = 16000, 
				eDRVSPU_FREQ_12000 = 12000, 
				eDRVSPU_FREQ_11025 = 11025, 
				eDRVSPU_FREQ_8000  = 8000	} E_DRVSPU_SAMPLING;


typedef struct {
				UINT32 u32ChannelIndex;
				UINT8  u8ChannelVolume;
				UINT16  u16PAN;
				UINT8  u8DataFormat;				
				UINT16 u16DFA;				
//				UINT8  u8SubIndex;				
//				UINT8  u8EventIndex;								
				UINT32 u32SrcBaseAddr;
				UINT32 u32SrcThresholdAddr;				
				UINT32 u32SrcEndAddr;								
//				UINT32 u32CurrentAddr;				
//				UINT32 u32LoopStartAddr;								
//				UINT32 u32LoopPlayByteCnt;												
				UINT16 u16SrcSampleRate;												
				UINT16 u16OutputSampleRate;																
											} S_CHANNEL_CTRL;

typedef int (PFN_DRVSPU_CB_FUNC)(UINT8 *);

#define	ERRCODE		INT32

void DrvSPU_IntHandler(void);

ERRCODE
DrvSPU_Open(void);
void DrvSPU_Close(void);

ERRCODE
DrvSPU_ChannelOpen(
	UINT32 u32Channel
);

ERRCODE	
DrvSPU_ChannelClose(
	UINT32 u32Channel
);

BOOL
DrvSPU_IsChannelEnabled(
	E_DRVSPU_CHANNEL e32Channel
);	
	
ERRCODE	
DrvSPU_EnableInt(
	UINT32 u32Channel, 
	UINT32 u32InterruptFlag, 
	PFN_DRVSPU_CB_FUNC* pfnCallBack
);

BOOL
DrvSPU_IsIntEnabled(
	UINT32 u32Channel, 
	UINT32 u32InterruptFlag
);

ERRCODE 
DrvSPU_DisableInt(
	UINT32 u32Channel, 
	UINT32 u32InterruptFlag
);

ERRCODE 
DrvSPU_ClearInt(
	UINT32 u32Channel, 
	UINT32 u32InterruptFlag
);

BOOL
DrvSPU_PollInt(
	UINT32 u32Channel, 
	UINT32 u32InterruptFlag
);

ERRCODE 
DrvSPU_SetPauseAddress_PCM16(
	UINT32 u32Channel, 
	UINT32 u32Address
);

UINT32
DrvSPU_GetPauseAddress_PCM16(
	UINT32 u32Channel
);

ERRCODE 
DrvSPU_SetBaseAddress(
	UINT32 u32Channel, 
	UINT32 u32Address
);

ERRCODE 
DrvSPU_SetThresholdAddress(
	UINT32 u32Channel, 
	UINT32 u32Address
);

ERRCODE 
DrvSPU_SetEndAddress(
	UINT32 u32Channel, 
	UINT32 u32Address
);

UINT32
DrvSPU_GetCurrentAddress(
	UINT32 u32Channel
);

UINT32 	
DrvSPU_GetBaseAddress(
	UINT32 u32Channel
);

UINT32 	
DrvSPU_GetThresholdAddress(
	UINT32 u32Channel
);

UINT32 	
DrvSPU_GetEndAddress(
	UINT32 u32Channel
);	

//	ERRCODE DrvSPU_SetDFA(UINT32 u32Channel, UINT16 u16DFA);

#ifdef OPT_DIRECT_SET_DFA
	ERRCODE 
	DrvSPU_SetDFA(
		UINT32 u32Channel, 
		UINT16 u16DFA
	);
#else
	ERRCODE 
		DrvSPU_SetDFA(
			UINT32 u32Channel, 
			UINT16 u16SrcSampleRate, 
			UINT16 u16OutputSampleRate
		);	
#endif	

UINT16 	
DrvSPU_GetDFA(
	UINT32 u32Channel
);

ERRCODE 
DrvSPU_SetPAN(
	UINT32 u32Channel, 		// MSB 8-bit = right channel; LSB 8-bit = left channel
	UINT16 u16PAN
);	

UINT16 	
DrvSPU_GetPAN(
	UINT32 u32Channel
);

ERRCODE 
DrvSPU_SetSrcType(
	UINT32 u32Channel, 
	UINT8 u8DataFormat
);

UINT16	
DrvSPU_GetSrcType(
	UINT32 u32Channel
);

ERRCODE	
DrvSPU_SetChannelVolume(
	UINT32 u32Channel, 
	UINT8 u8Volume
);

UINT8 	
DrvSPU_GetChannelVolume(
	UINT32 u32Channel
);

void DrvSPU_EqOpen(
	E_DRVSPU_EQ_BAND eEQBAND, 
	E_DRVSPU_EQ_GAIN eEQGAIN
);

void DrvSPU_EqClose(void);

void DrvSPU_SetVolume(
	UINT16 u16Volume
);

UINT16 	
DrvSPU_GetVolume(void);

void DrvSPU_StartPlay(void);
void DrvSPU_StopPlay(void);

UINT32 	
DrvSPU_SetSampleRate(
	UINT32  u32SystemClock,
	UINT32 SampleRate
);	

ERRCODE	
DrvSPU_UploadChannelContents(
	UINT32 u32Channel
);

ERRCODE 
DrvSPU_ChannelCtrl(
	S_CHANNEL_CTRL *psChannelCtrl
);	

ERRCODE	
DrvSPU_ChannelPause(
	UINT32 u32Channel
);

ERRCODE	
DrvSPU_ChannelResume(
	UINT32 u32Channel
);	

ERRCODE 
DrvSPU_SetToneAmp(
	UINT32 u32Channel, 
	UINT32 u32Amp
);

ERRCODE 
DrvSPU_SetTonePulse(
	UINT32 u32Channel, 
	UINT32 u32Pulse
);

//	UINT32 DrvSPU_GetSampleRate (void);


#ifdef __cplusplus
}
#endif

#endif	/*_DRVNAND_H_*/














