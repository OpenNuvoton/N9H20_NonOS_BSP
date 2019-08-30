
#include <stdio.h>
#include "wbio.h"
#include "wblib.h"
#include "wbtypes.h"

#include "N9H20.h"
#include "nau8822.h"

#define OPT_CACHE_ON

#define printf 	sysprintf
volatile S_DRVI2S_PLAY g_sPlay;
volatile S_DRVI2S_RECORD g_sRrecord;

#define 	BUFSIZE		0x100000

#if defined (__GNUC__) && !(__CC_ARM)
__attribute__ ((aligned (32))) char g_baAudioBuf[BUFSIZE];
#else
__align (32) char g_baAudioBuf[BUFSIZE];
#endif

int  audio_RecordCallBack(UINT8 *pu8Buff, UINT32 uDataLen)
{
	printf(" I2S current recording address = %6x \n", inp32(REG_I2S_ACTL_RDSTC));	    
	printf(" value of R_DMA_RIA_SN = %1x \n", inp32(REG_I2S_ACTL_RSR) & 0xE0);	    			
	return 0;
}


int  audio_PlayCallBack(UINT8 *pu8Buff, UINT32 uDataLen)
{
	printf(" I2S current playing address = %6x \n", inp32(REG_I2S_ACTL_PDSTC));	    
	printf(" value of P_DMA_RIA_SN = %1x \n", inp32(REG_I2S_ACTL_PSR) & 0xE0);	    			
	return 0;
}

//====================================================================================
//		main program
//====================================================================================
int main(void)
{
	UINT32 uWriteAddr, uReadAddr;
	int rtval;

#ifdef OPT_CACHE_ON
	sysDisableCache(); 	
	sysFlushCache(I_D_CACHE);		
	sysEnableCache(CACHE_WRITE_THROUGH);		
#endif

#ifdef OPT_CACHE_ON
	uWriteAddr = (UINT32) g_baAudioBuf | 0x80000000;
	uReadAddr = (UINT32) g_baAudioBuf | 0x80000000;
#else
	uWriteAddr = (UINT32) g_baAudioBuf;
	uReadAddr = (UINT32) g_baAudioBuf;
#endif 	

	uWriteAddr = (UINT32) g_baAudioBuf;
	uReadAddr = (UINT32) g_baAudioBuf;

	DrvI2S_Open();

	/* set up nau8822 */
	i2cInit();	
	rtval = i2cOpen();
	if(rtval < 0)
	{
		printf("Open I2C error!\n");
		goto exit_demo;
	}	
	i2cIoctl(I2C_IOC_SET_SPEED, 100, 0);

	NAU8822_ADC_DAC_Setup();
	NAU8822_SetPlayVolume(20, 20);
	NAU8822_SetRecVolume(20, 20);	

#define 	OPT_I2S_INT_TEST	
#ifdef 		OPT_I2S_INT_TEST
	sysSetLocalInterrupt(DISABLE_FIQ_IRQ);		
	DrvI2S_EnableInt(DRVI2S_IRQ_RECORD, audio_RecordCallBack);	
	DrvI2S_EnableInt(DRVI2S_IRQ_PLAYBACK, audio_PlayCallBack);	
	sysEnableInterrupt(IRQ_I2S);	
	sysSetLocalInterrupt(ENABLE_FIQ_IRQ);						
#endif	

	// set record	
	g_sRrecord.u32BufferAddr = uWriteAddr;	
	g_sRrecord.u32BufferLength = BUFSIZE;		
	g_sRrecord.eSampleRate = eDRVI2S_FREQ_44100;
	g_sRrecord.eChannel = eDRVI2S_RECORD_STEREO;						
	g_sRrecord.eFormat = eDRVI2S_I2S;	
	DrvI2S_StartRecord((S_DRVI2S_RECORD*) &g_sRrecord);	
	printf(" I2S start Playing stereo in 44.1 kHz sampling rate \n\n");

	// set playback
	g_sPlay.u32BufferAddr = (UINT32) uReadAddr;	
	g_sPlay.u32BufferLength = BUFSIZE;		
	g_sPlay.eSampleRate = eDRVI2S_FREQ_44100;
	g_sPlay.eChannel = eDRVI2S_PLAY_STEREO;						
	g_sPlay.eFormat = eDRVI2S_I2S;			
	DrvI2S_StartPlay((S_DRVI2S_PLAY*) &g_sPlay);		
	printf(" I2S start Recording stereo in 44.1 kHz sampling rate \n\n");
	
	while(1)
	{
#ifndef OPT_I2S_INT_TEST		
	    while (inp32(REG_I2S_ACTL_RSR) & R_DMA_RIA_IRQ)
	    {
			outp32(REG_I2S_ACTL_RSR, inp32(REG_I2S_ACTL_RSR));
			printf(" I2S current recording address = %6x \n", inp32(REG_I2S_ACTL_RDSTC));	    
			printf(" value of R_DMA_RIA_SN = %1x \n", inp32(REG_I2S_ACTL_RSR) & 0xE0);	    			
	    }

	    while (inp32(REG_I2S_ACTL_PSR) & P_DMA_RIA_IRQ)
	    {
			outp32(REG_I2S_ACTL_PSR, inp32(REG_I2S_ACTL_PSR));	    
			printf(" I2S current playing address = %6x \n", inp32(REG_I2S_ACTL_PDSTC));	    
			printf(" value of P_DMA_RIA_SN = %1x \n", inp32(REG_I2S_ACTL_PSR) & 0xE0);	    			
	    }
#endif
	}	    

exit_demo:
	while(1);	
}
