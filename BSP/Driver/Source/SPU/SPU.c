/**************************************************************************//**
 * @file     SPU.c
 * @version  V3.00
 * @brief    N9H20 series SPU driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/
 
#include "string.h"
#include "stdlib.h"
#include "wblib.h"
#include "N9H20_SPU.h"

/* buffer */
UINT8	*_pucPlayAudioBuff;
#if defined (__GNUC__) && !(__CC_ARM)
__attribute__ ((aligned (256))) UINT8 playbuffer[FRAG_SIZE];
#else
__align(256) UINT8 playbuffer[FRAG_SIZE];
#endif

/* function declaration */
ERRCODE DrvSPU_InstallCallBack(
	E_DRVSPU_CHANNEL eChannel,
	UINT32 eIntSource,
	PFN_DRVSPU_INT_CALLBACK	pfnCallback,
	PFN_DRVSPU_INT_CALLBACK *pfnOldCallback );

ERRCODE DrvSPU_Open(void);
void DrvSPU_Close(void);
void DrvSPU_ISR(void);
ERRCODE DrvSPU_ChannelOpen(E_DRVSPU_CHANNEL eChannel);
ERRCODE	DrvSPU_ChannelClose(E_DRVSPU_CHANNEL eChannel);
BOOL DrvSPU_IsChannelEnabled(E_DRVSPU_CHANNEL eChannel);	
ERRCODE	DrvSPU_EnableInt(E_DRVSPU_CHANNEL eChannel, UINT32 eInt, PFN_DRVSPU_CB_FUNC* pfnCallBack);
BOOL DrvSPU_IsIntEnabled(E_DRVSPU_CHANNEL eChannel, UINT32 eInt);
ERRCODE	DrvSPU_DisableInt(E_DRVSPU_CHANNEL eChannel, UINT32 eInt);
ERRCODE	DrvSPU_ClearInt(E_DRVSPU_CHANNEL eChannel, UINT32 eInt);
ERRCODE DrvSPU_PollInt(E_DRVSPU_CHANNEL eChannel, UINT32 eInt);
ERRCODE DrvSPU_SetBaseAddress(E_DRVSPU_CHANNEL eChannel, UINT32 u32Address);
ERRCODE DrvSPU_SetThresholdAddress(E_DRVSPU_CHANNEL eChannel, UINT32 u32Address);
ERRCODE DrvSPU_SetEndAddress(E_DRVSPU_CHANNEL eChannel, UINT32 u32Address);
ERRCODE DrvSPU_SetPauseAddress(E_DRVSPU_CHANNEL eChannel, UINT32 u32Address);
UINT32 DrvSPU_GetBaseAddress(UINT32 u32Channel);
UINT32 DrvSPU_GetThresholdAddress(UINT32 u32Channel);
UINT32 DrvSPU_GetCurrentAddress(UINT32 u32Channel);
ERRCODE DrvSPU_GetLoopStartAddress(E_DRVSPU_CHANNEL eChannel, UINT32* pu32Address);
UINT32 DrvSPU_GetEndAddress(UINT32 u32Channel);
ERRCODE DrvSPU_GetPauseAddress(E_DRVSPU_CHANNEL eChannel, UINT32* pu32Address);
ERRCODE DrvSPU_GetUserEventIndex(E_DRVSPU_CHANNEL eChannel, UINT8* pu8EventIndex, UINT8* pu8SubIndex);
#ifdef OPT_DIRECT_SET_DFA
ERRCODE DrvSPU_SetDFA(E_DRVSPU_CHANNEL eChannel, UINT16 u16DFA);
#else
ERRCODE DrvSPU_SetDFA(E_DRVSPU_CHANNEL eChannel, UINT16 u16SrcSampleRate, UINT16 u16OutputSampleRate);	
#endif	
ERRCODE DrvSPU_GetDFA(E_DRVSPU_CHANNEL eChannel, UINT16* pu16DFA);
ERRCODE DrvSPU_SetPAN(E_DRVSPU_CHANNEL eChannel, UINT16 u16PAN);	
ERRCODE DrvSPU_GetPAN(E_DRVSPU_CHANNEL eChannel, UINT16* pu16PAN);
ERRCODE DrvSPU_SetSrcType(E_DRVSPU_CHANNEL eChannel, E_DRVSPU_FORMAT eDataFormat);
ERRCODE	DrvSPU_GetSrcType(E_DRVSPU_CHANNEL eChannel, UINT16* pu16SrcType);
ERRCODE	DrvSPU_SetChannelVolume(E_DRVSPU_CHANNEL eChannel, UINT8 u8Volume);
UINT8 DrvSPU_GetChannelVolume(UINT32 u32Channel);
ERRCODE DrvSPU_SetChannelTone(E_DRVSPU_CHANNEL eChannel, S_TONE_CTRL* psToneCtrl);
ERRCODE DrvSPU_GetChannelTone(E_DRVSPU_CHANNEL eChannel, S_TONE_CTRL* psToneCtrl);
void DrvSPU_EqOpen(E_DRVSPU_EQ_BAND eEQBAND, E_DRVSPU_EQ_GAIN eEQGAIN);
void DrvSPU_EqClose(void);
void DrvSPU_SetVolume(UINT16 u16Volume);
void DrvSPU_GetVolume(UINT16* pu16Volume);
void DrvSPU_StartPlay(void);
void DrvSPU_StopPlay(void);
BOOL DrvSPU_IsSPUPlaying(void);
UINT32 DrvSPU_SetSampleRate(UINT32 u32SystemClock, UINT32 SampleRate);
ERRCODE	DrvSPU_UploadChannelContents(E_DRVSPU_CHANNEL eChannel);
ERRCODE DrvSPU_ChannelCtrl(S_CHANNEL_CTRL *psChannelCtrl);	
ERRCODE	DrvSPU_ChannelPause(E_DRVSPU_CHANNEL eChannel);
ERRCODE	DrvSPU_ChannelResume(E_DRVSPU_CHANNEL eChannel);	
ERRCODE DrvSPU_SetToneAmp(E_DRVSPU_CHANNEL eChannel, UINT32 u32Amp);
ERRCODE DrvSPU_SetTonePulse(E_DRVSPU_CHANNEL eChannel, UINT32 u32Pulse);
UINT8 DrvSPU_ReadDACReg(UINT8 DACRegIndex);
VOID DrvSPU_WriteDACReg(UINT8 DACRegIndex, UINT8 DACRegData);
void DrvSPU_IntHandler(void);



void SPU_SET_SAMPLE_RATE(UINT32 u32sysclk, UINT32 u32SampleRate)
{	
	DrvSPU_SetSampleRate(u32sysclk, u32SampleRate);
}

void SPU_SET_BASE_ADDRESS(UINT32 u32BaseAddress)
{
	DrvSPU_SetBaseAddress(0, u32BaseAddress);
	DrvSPU_SetBaseAddress(1, u32BaseAddress);			
}

UINT32 SPU_GET_BASE_ADDRESS(void)
{
	return DrvSPU_GetBaseAddress(0);
}

void SPU_SET_TH1_ADDRESS(UINT32 u32TH1Address)
{
	DrvSPU_SetThresholdAddress(0, u32TH1Address);	
	DrvSPU_SetThresholdAddress(1, u32TH1Address);		
}

UINT32 SPU_GET_TH1_ADDRESS(void)
{
	return DrvSPU_GetThresholdAddress(0);
}

void SPU_SET_TH2_ADDRESS(UINT32 u32TH2Address)
{
	DrvSPU_SetEndAddress(0, u32TH2Address);	
	DrvSPU_SetEndAddress(1, u32TH2Address);		
}

UINT32 SPU_GET_TH2_ADDRESS(void)
{
	return DrvSPU_GetEndAddress(0);
}

UINT32 SPU_GET_CUR_ADDRESS(void)
{
	return DrvSPU_GetCurrentAddress(0);
}

void SPU_STEREO(void)
{
	DrvSPU_SetSrcType(0, 0x06);		// Stereo PCM16 left	
	DrvSPU_SetSrcType(1, 0x07);		// Stereo PCM16 Right			
}

void SPU_MONO(void)
{
	DrvSPU_SetSrcType(0, 0x05);		// Mono PCM16	
}

BOOL SPU_ISMONO(void)
{
	return 0;
}

void SPU_SET_VOLUME(UINT16 u16CHRVolume, UINT16 u16CHLVolume)
{
	UINT16 u16Voluem;
	
	// u16CHRVolume: 0x0(Mute) ~ 0x3F(Maximum)
		u16Voluem = (u16CHRVolume & 0x3F) | ((u16CHLVolume & 0x3F) << 8);
		DrvSPU_SetVolume(u16Voluem);
}

UINT8 SPU_GET_VOLUME(UINT8 u8Channel)
{
	return DrvSPU_GetChannelVolume(u8Channel);
}

void SPU_SET_POWER_DOWN(UINT16 u16PowerDown)
{
	outp32(REG_SPU_DAC_VOL, (inp32(REG_SPU_DAC_VOL) & ~ANA_PD) | ((u16PowerDown & 0xFFF)<<16));
}

UINT16 SPU_GET_POWER_DOWN(void)
{
	return (inp32(REG_SPU_DAC_VOL) & ~ANA_PD) >> 16;
}

UINT8 DacOnOffLevel;

//must use this function before calling spuStartPlay()
VOID spuDacOn(UINT8 level)
{
	DacOnOffLevel = level;	
	
	outp32(REG_SPU_DAC_PAR, inp32(REG_SPU_DAC_PAR) | 0x30);		//disable
	if(level == 3)
		outp32(REG_SPU_DAC_PAR, inp32(REG_SPU_DAC_PAR) & ~0x30);	//delay time, p0=3s
	else if(level == 1)
		outp32(REG_SPU_DAC_PAR, inp32(REG_SPU_DAC_PAR) & ~0x20);	//delay time, p0=0.5-1s
	else if(level == 2)
		outp32(REG_SPU_DAC_PAR, inp32(REG_SPU_DAC_PAR) & ~0x10);	//delay time, p0=2s
	else 	
	{	
		outp32(REG_SPU_DAC_VOL, inp32(REG_SPU_DAC_VOL) & ~0x03FF0000);	//P7
		outp32(REG_SPU_DAC_PAR, inp32(REG_SPU_DAC_PAR) | 0x30);		//disable
		return;
	}

	outp32(REG_SPU_DAC_VOL, inp32(REG_SPU_DAC_VOL) & ~0x0800000);	//P7	
	sysDelay(1);
	outp32(REG_SPU_DAC_VOL, inp32(REG_SPU_DAC_VOL) & ~0x0400000);	//P6
	sysDelay(1);
	outp32(REG_SPU_DAC_VOL, inp32(REG_SPU_DAC_VOL) & ~0x01e0000);	//P1-4
	sysDelay(1);
	outp32(REG_SPU_DAC_VOL, inp32(REG_SPU_DAC_VOL) & ~0x0200000);	//P5	
	sysDelay(1);
	
	outp32(REG_SPU_DAC_VOL, inp32(REG_SPU_DAC_VOL) & ~0x00010000);	//P0			
	
	if(level == 3)		//modify this delay time to meet the product request
		sysDelay(220);
	else if(level == 1)
		sysDelay(70);
	else if(level == 2)
		sysDelay(30);
		
}

//must use this function after calling spuStopPlay()
VOID spuDacOff(VOID)
{		
	sysDelay(1);
	outp32(REG_SPU_DAC_VOL, inp32(REG_SPU_DAC_VOL) | 0x10000);	//P0
	
	if(DacOnOffLevel == 3)		//modify this delay time to meet the product request
		sysDelay(150);
	else if(DacOnOffLevel == 1)
		sysDelay(70);
	else if(DacOnOffLevel == 2)
		sysDelay(40);
		
	outp32(REG_SPU_DAC_VOL, inp32(REG_SPU_DAC_VOL) | 0x200000);	//P5
	sysDelay(1);
				
	outp32(REG_SPU_DAC_VOL, inp32(REG_SPU_DAC_VOL) | 0x1e0000);	//P1-4
	sysDelay(1);
	outp32(REG_SPU_DAC_VOL, inp32(REG_SPU_DAC_VOL) | 0x400000);	//P6
	sysDelay(1);
	outp32(REG_SPU_DAC_VOL, inp32(REG_SPU_DAC_VOL) | 0x800000);	//P7	
	sysDelay(1);

	outp32(REG_SPU_DAC_PAR, inp32(REG_SPU_DAC_PAR) | 0x30);  //disable
}

VOID spuStartPlay(PFN_DRVSPU_CB_FUNC *fnCallBack, UINT8 *data)
{	
	DrvSPU_EnableInt(eDRVSPU_CHANNEL_0, DRVSPU_THADDRESS_INT, (PFN_DRVSPU_CB_FUNC*) fnCallBack);
	DrvSPU_EnableInt(eDRVSPU_CHANNEL_0, DRVSPU_ENDADDRESS_INT, (PFN_DRVSPU_CB_FUNC*) fnCallBack);	
	
	memcpy(playbuffer, data, FRAG_SIZE);

	DrvSPU_StartPlay();	
}

VOID spuStopPlay(VOID)
{
	int ii;     
    for (ii=0; ii<32; ii++)
    {
 		DrvSPU_DisableInt(ii, DRVSPU_ENDADDRESS_INT); 
        DrvSPU_DisableInt(ii, DRVSPU_THADDRESS_INT);
    }
	DrvSPU_StopPlay();	
	sysDisableInterrupt(IRQ_SPU);	
}

VOID spuIoctl(UINT32 cmd, UINT32 arg0, UINT32 arg1)
{
	switch(cmd)
	{		
		case SPU_IOCTL_SET_VOLUME:
			SPU_SET_VOLUME(arg0, arg1);
			break;
			
		case SPU_IOCTL_SET_MONO:
			SPU_MONO();
			break;
			
		
		case SPU_IOCTL_SET_STEREO:
			SPU_STEREO();
			break;			
			
		case SPU_IOCTL_GET_FRAG_SIZE:
			*((UINT32 *)arg0) = FRAG_SIZE;
			break;
			
		default:
			break;
	}		
}

VOID spuOpen(UINT32 u32SampleRate)
{
	UINT32 u32SystemClock, u32ExtClock;
	
	_pucPlayAudioBuff = (UINT8 *)((UINT32)playbuffer | 0x80000000);
	memset(_pucPlayAudioBuff, 0, FRAG_SIZE);

	SPU_DISABLE();	// SPU must be disabled before to enable again
	
	//sysGetClockFreq(&pllcfg);
	u32ExtClock= sysGetExternalClock();
	if (inp32(REG_CHIPCFG) & 0x01)
		u32SystemClock = sysGetPLLOutputKhz(eSYS_UPLL, u32ExtClock)*1000;
	else
		u32SystemClock = sysGetPLLOutputKhz(eSYS_UPLL, u32ExtClock)*1000;

	// 1.Check I/O pins. If I/O pins are used by other IPs, return error code.
	// 2.Enable IP¡¦s clock
	// 3.Reset IP			
	
	DrvSPU_Open();	
	
	DrvSPU_SetPAN(0, 0x1f1f);	
	
	// Channel volume 0x4F for earphone and 0x3F for speaker
	DrvSPU_SetChannelVolume(0, 0x4F);	//	
	DrvSPU_SetChannelVolume(1, 0x4F);	//0408
	
	DrvSPU_ChannelOpen(0);		
	
	DrvSPU_SetSampleRate(u32SystemClock, u32SampleRate);

	DrvSPU_SetSrcType(0, 0x06);		// Stereo PCM16 left

	SPU_SET_BASE_ADDRESS((UINT32)_pucPlayAudioBuff);
	SPU_SET_TH1_ADDRESS((UINT32)_pucPlayAudioBuff + HALF_FRAG_SIZE);	
	SPU_SET_TH2_ADDRESS((UINT32)_pucPlayAudioBuff + FRAG_SIZE);

	sysInstallISR(IRQ_LEVEL_1, IRQ_SPU, (PVOID)DrvSPU_IntHandler);	
	sysSetLocalInterrupt(ENABLE_IRQ);
	sysEnableInterrupt(IRQ_SPU);
	
}

VOID spuClose (VOID)
{
	sysDisableInterrupt(IRQ_SPU);
	DrvSPU_Close();
}

VOID spuEqOpen (E_DRVSPU_EQ_BAND eEqBand, E_DRVSPU_EQ_GAIN eEqGain)
{
	DrvSPU_EqOpen(eEqBand, eEqGain);
}

VOID spuEqClose (VOID)
{
	DrvSPU_EqClose();
}



