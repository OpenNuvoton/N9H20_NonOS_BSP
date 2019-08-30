#include <stdio.h>
#include "N9H20.h"


#define DBG_PRINTF			sysprintf
#define AUDIO_REC_SEC		10

static volatile INT8 g_i8PcmReady = FALSE;
//__align(4) static INT16 g_pi16SampleBuf[16000*AUDIO_REC_SEC];		/* Keep max 16K sample rate */

#if defined (__GNUC__)
INT16 g_pi16SampleBuf[16000*AUDIO_REC_SEC]  __attribute__ ((aligned (32)));		/* Keep max 16K sample rate */

char WaveHeader[]  __attribute__ ((aligned (4))) = {'R', 'I', 'F', 'F', 0x00, 0x00, 0x00, 0x00,	   //ForthCC code+(RAW-data-size+0x24)
					'W', 'A', 'V', 'E', 'f', 'm', 't', ' ',
					0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,//Chunk-size, audio format, and NUMChannel
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//Sample-Rate and Byte-Count-Per-Sec
					0x02, 0x00, 0x10, 0x00,						   //Align and Bits-per-sample.
					'd', 'a', 't', 'a', 0x00, 0x00, 0x00, 0x00};   //"data"and RAW-data-size
#else
__align(32) INT16 g_pi16SampleBuf[16000*AUDIO_REC_SEC];		/* Keep max 16K sample rate */

__align(4) char WaveHeader[]= {'R', 'I', 'F', 'F', 0x00, 0x00, 0x00, 0x00,	   //ForthCC code+(RAW-data-size+0x24)	
					'W', 'A', 'V', 'E', 'f', 'm', 't', ' ',			
					0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,//Chunk-size, audio format, and NUMChannel
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//Sample-Rate and Byte-Count-Per-Sec 
					0x02, 0x00, 0x10, 0x00,						   //Align and Bits-per-sample.
					'd', 'a', 't', 'a', 0x00, 0x00, 0x00, 0x00};   //"data"and RAW-data-size		
#endif
INT32 AudioWriteFile(char* szAsciiName, 
					PUINT16 pu16BufAddr, 
					UINT32 u32Length,
					UINT32 u32SampleRate)
{
	INT hFile, i32Errcode, u32WriteLen;
	char suFileName[256];
	UINT32 *pu32ptr; 
	
	strcat(szAsciiName, ".wav");
	fsAsciiToUnicode(szAsciiName, suFileName, TRUE);
	hFile = fsOpenFile(suFileName, NULL, O_CREATE);
	if (hFile < 0)
	{
		DBG_PRINTF("Fail to creating file \n");					
		return hFile;
	}	
	else
		DBG_PRINTF("Succeed to creating file \n");	
	pu32ptr =  (UINT32*)(WaveHeader+4);
	*pu32ptr = u32Length+0x24;		
	pu32ptr = (UINT32*)(WaveHeader+0x28); //(UINT32*)WaveHeader[0x28];
	*pu32ptr = u32Length;
	pu32ptr = (UINT32*)(WaveHeader+0x18);//(UINT32*)WaveHeader[0x10];
	*pu32ptr = u32SampleRate;
	pu32ptr = (UINT32*)(WaveHeader+0x1C);//(UINT32*)WaveHeader[0x14];
	*pu32ptr = u32SampleRate*2;				//16bits 
	i32Errcode = fsWriteFile(hFile, 
							(UINT8 *)WaveHeader, 
							0x2C, 
							&u32WriteLen);	
	if (i32Errcode < 0)
		return i32Errcode;											
	i32Errcode = fsWriteFile(hFile, 
							(UINT8 *)pu16BufAddr, 
							u32Length, 
							&u32WriteLen);
	if (i32Errcode < 0)
		return i32Errcode;	
		
	fsCloseFile(hFile);
	return Successful;	
}

INT32 AudioWriteFileHead(char* szAsciiName, 
					UINT32 u32Length,
					UINT32 u32SampleRate)
{
	INT hFile, i32Errcode=0, u32WriteLen;
	char suFileName[256];
	UINT32 *pu32ptr; 
	strcat(szAsciiName, ".wav");
	fsAsciiToUnicode(szAsciiName, suFileName, TRUE);
	hFile = fsOpenFile(suFileName, NULL, O_CREATE);
	if (hFile < 0)
	{
		DBG_PRINTF("Fail to creating file \n");					
		return hFile;
	}	
	else
		DBG_PRINTF("Succeed to creating file \n");	
#if 0		
	pu32ptr =  (UINT32*)(WaveHeader+4);
	*pu32ptr = u32Length+0x24;		
	pu32ptr = (UINT32*)(WaveHeader+0x28); //(UINT32*)WaveHeader[0x28];
	*pu32ptr = u32Length;
	pu32ptr = (UINT32*)(WaveHeader+0x18);//(UINT32*)WaveHeader[0x10];
	*pu32ptr = u32SampleRate;
	pu32ptr = (UINT32*)(WaveHeader+0x1C);//(UINT32*)WaveHeader[0x14];
	*pu32ptr = u32SampleRate*2;				//16bits
#else
	pu32ptr =  (UINT32*) ((UINT32)(WaveHeader+4) | 0x80000000);
	*pu32ptr = u32Length+0x24;		
	pu32ptr = (UINT32*)((UINT32)(WaveHeader+0x28) | 0x80000000) ; //(UINT32*)WaveHeader[0x28];
	*pu32ptr = u32Length;
	pu32ptr = (UINT32*)((UINT32)(WaveHeader+0x18) | 0x80000000);//(UINT32*)WaveHeader[0x10];
	*pu32ptr = u32SampleRate;
	pu32ptr = (UINT32*)((UINT32)(WaveHeader+0x1C) | 0x80000000);//(UINT32*)WaveHeader[0x14];
	*pu32ptr = u32SampleRate*2;				//16bits 
#endif	
	i32Errcode = fsWriteFile(hFile, 
							(UINT8 *)((UINT32)WaveHeader | 0x80000000), 
							0x2C, 
							&u32WriteLen);
	if(i32Errcode < 0)
		sysprintf("Write bitstream fail\n");
	return hFile;						
}							
INT32 AudioWriteFileData(INT hFile, UINT16* pu16BufAddr, UINT32 u32Length)
{
	INT i32Errcode, u32WriteLen;
	i32Errcode = fsWriteFile(hFile, 
						(UINT8 *)pu16BufAddr, 
						u32Length, 
						&u32WriteLen);
	if(i32Errcode<0)
		return i32Errcode;
	else
		return Successful;						
}
INT32 AudioWriteFileClose(INT hFile)
{
	fsCloseFile(hFile);
	return Successful;	
}
static void pfnRecordCallback(void)
{
	g_i8PcmReady = TRUE;
}
volatile BOOL bIsBufferDone=0;
void edmaCallback(UINT32 u32WrapStatus)
{
	UINT32 u32Period, u32Attack, u32Recovery, u32Hold;
	if(u32WrapStatus==256)
	{
		bIsBufferDone = 1;
		//sysprintf("I %d\n\n", bIsBufferDone);
	}	
	else if(u32WrapStatus==1024)
	{
		
		bIsBufferDone = 2;		
		//sysprintf("I %d\n\n", bIsBufferDone);
	}
	/* AGC response speed */
	audio_GetAutoGainTiming(&u32Period, &u32Attack, &u32Recovery, &u32Hold);
	if(u32Period<128)
	{		
		u32Period = u32Period+16;
		audio_SetAutoGainTiming(u32Period, u32Attack, u32Recovery, u32Hold);		
	}
}
/*
	The EDMA will move audio data to start address = g_pi16SampleBuf[0] with range u32Length. 
	The EDMA will issue interrupt in data reach to half of u32Length and u32Length. 	
	Then repeat to filled audio data to  g_pi16SampleBuf[0].
	Programmer must write audio data ASAP. 
*/
#pragma diag_suppress 188
void InitEDMA(UINT32 u32Length)
{
	S_DRVEDMA_CH_ADDR_SETTING pSrc, pDest;
	PFN_DRVEDMA_CALLBACK *pfnOldcallback;  
	
	pSrc.u32Addr = REG_AUDIO_BUF0;
	pSrc.eAddrDirection = eDRVEDMA_DIRECTION_FIXED;
	pDest.u32Addr = (unsigned int)g_pi16SampleBuf | 0x80000000;
	pDest.eAddrDirection = eDRVEDMA_DIRECTION_WRAPAROUND;
	DrvEDMA_Open();

	DrvEDMA_EnableCH(eDRVEDMA_CHANNEL_1,eDRVEDMA_ENABLE);
	DrvEDMA_SetAPBTransferWidth(eDRVEDMA_CHANNEL_1,eDRVEDMA_WIDTH_16BITS);	// Mode 0 : 16 Bit
//	DrvEDMA_SetAPBTransferWidth(eDRVEDMA_CHANNEL_1,eDRVEDMA_WIDTH_32BITS);	// Mode 1 : 32 bit	
	DrvEDMA_SetCHForAPBDevice(eDRVEDMA_CHANNEL_1, eDRVEDMA_ADC, eDRVEDMA_READ_APB);
	//DrvEDMA_SetTransferSetting(eDRVEDMA_CHANNEL_1, &pSrc, &pDest,16*8000 ); //16K 
	DrvEDMA_SetTransferSetting(eDRVEDMA_CHANNEL_1, &pSrc, &pDest, u32Length); //16K
	DrvEDMA_SetWrapIntType((E_DRVEDMA_CHANNEL_INDEX)eDRVEDMA_CHANNEL_1, 
							((E_DRVEDMA_WRAPAROUND_SELECT)eDRVEDMA_WRAPAROUND_HALF |
							(E_DRVEDMA_WRAPAROUND_SELECT)eDRVEDMA_WRAPAROUND_EMPTY));
	DrvEDMA_EnableInt(eDRVEDMA_CHANNEL_1,eDRVEDMA_WAR);	
	
	DrvEDMA_InstallCallBack(eDRVEDMA_CHANNEL_1, 
						eDRVEDMA_WAR,
						edmaCallback,
						pfnOldcallback);	
	
}	
/*=================================================================
	Convert 10 bit ADC data to 16 bit sign audio data
	
	ADC RAW 10 bit from 0 ~ 1023. 
	Generally, the audio from miscrophone is 20 ~ 30 mv. It is very small. 	
	1. Force the DC level to middle of 3.3 v. ==>Add 512. (The analog part add the DC bise) 
=================================================================*/
static INT32 nIdx=1;
#define E_AUD_BUF 16000
INT32 AudioRecord(UINT32 u32SampleRate)
{	
    CHAR    szFileName[128];	
    PFN_ADC_CALLBACK    pfnOldCallback;
    INT32   hFile;
    UINT32 u32Length=0;


    audio_Open(eSYS_APLL, u32SampleRate/1000);
    adc_disableInt(eADC_AUD_INT);//(PVOID)pfnRecordCallback, 0);
    adc_installCallback(eADC_AUD_INT,
			pfnRecordCallback,	/* Invalid if EDMA enable */
			&pfnOldCallback);													
    InitEDMA(E_AUD_BUF);
    audio_StartRecord();
    sprintf(szFileName, "C:\\Audio_%04d", nIdx);
    hFile = AudioWriteFileHead(szFileName,							
			        (u32SampleRate*2*AUDIO_REC_SEC),
				u32SampleRate);
    DrvEDMA_CHEnablelTransfer(eDRVEDMA_CHANNEL_1);
    bIsBufferDone = 0;
    do
    {
        if(bIsBufferDone==1)
        {		
            AudioWriteFileData(hFile,
				(UINT16*)(((UINT32)g_pi16SampleBuf+E_AUD_BUF/2) |0x80000000),
				E_AUD_BUF/2);							
            u32Length = u32Length+E_AUD_BUF/2;		
            bIsBufferDone = 0;
	}
	else if(bIsBufferDone==2)
	{		
            AudioWriteFileData(hFile,
                               (PUINT16)((UINT32)(&g_pi16SampleBuf) | 0x80000000),
                               E_AUD_BUF/2);						
            u32Length = u32Length+E_AUD_BUF/2;	
            bIsBufferDone = 0;
        }		
    }while(u32Length<(u32SampleRate*2*AUDIO_REC_SEC));
    AudioWriteFileClose(hFile);
    audio_StopRecord();   
    DrvEDMA_Close();
    sysprintf("Close File\n");
    nIdx = nIdx+1;			    
    return Successful;
}
