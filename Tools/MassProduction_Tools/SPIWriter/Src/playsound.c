/****************************************************************************
 * @file     playsound.c
 * @version  V1.00
 * $Revision: 4 $
 * $Date: 18/04/25 11:43a $
 * @brief    SpiWriter source file
 *
 * @note
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "N9H20.h" 
#include "writer.h"
#include "pcm.h"

static UINT16 u16IntCount = 2;
static UINT32 u32FragSize;
static volatile UINT8 bPlaying = TRUE;

UINT8* SPU_SOURCE;
UINT32 volatile u32PcmLen;

#define START_BURN  0
#define BURN_PASS   1
#define BURN_FAIL   2

int playCallBack(UINT8 * pu8Buffer)
{
#if 1
    UINT32 u32Offset = 0;
    UINT32 len = u32PcmLen;

    u32Offset = ( u32FragSize / 2) * u16IntCount;

    if (u32Offset >= u32PcmLen)    /* Reach the end of the song, restart from beginning */
    {
        if (u16IntCount >2 )
        {
            u16IntCount--;
            u32Offset = ( u32FragSize / 2) * u16IntCount;
            len -= u32Offset;
        }
        DrvSPU_SetPauseAddress_PCM16(0, (UINT32)pu8Buffer + len);
        u16IntCount = 2;        
    }
    else
    {
        memcpy(pu8Buffer, &SPU_SOURCE[u32Offset], u32FragSize/2);
        u16IntCount++;
    }
    return TRUE;
#else
    if(bPlaying)
    {
        UINT32 u32Offset = 0;

        u32Offset = ( u32FragSize / 2) * u16IntCount;
        if (u32Offset >= u32PcmLen)    /* Reach the end of the song, restart from beginning */
        {
            bPlaying = FALSE;
            if(u32PcmLen  >= u32FragSize)
            {
                if((u16IntCount & 1) == 0)
                    DrvSPU_SetPauseAddress_PCM16(0, (UINT32)pu8Buffer + u32FragSize);
                else
                    DrvSPU_SetPauseAddress_PCM16(0, (UINT32)pu8Buffer + u32FragSize/2);
                sysprintf("-----------------><2>\n");
            }
            sysprintf("-----------------><0>\n");
            return TRUE;
        }
        sysprintf("-----------------><1>\n");
        memcpy(pu8Buffer, &SPU_SOURCE[u32Offset], u32FragSize/2);
        u16IntCount++;
    }
    return FALSE;
#endif
}



int sound_init(UINT32 SoundPlayVolume)
{
    spuOpen(eDRVSPU_FREQ_8000);
    spuIoctl(SPU_IOCTL_SET_VOLUME, SoundPlayVolume, SoundPlayVolume);
    spuIoctl(SPU_IOCTL_GET_FRAG_SIZE, (UINT32)&u32FragSize, 0);   
    spuEqOpen(eDRVSPU_EQBAND_2, eDRVSPU_EQGAIN_P7DB);
    spuDacOn(1);
    return(0);
}
int sound_play(UINT32 u32select,UINT32 u32Buffer,UINT32 u32Size)
{
    bPlaying = TRUE;
    u16IntCount =2;
    if(u32Buffer)
    {
        SPU_SOURCE = (UINT8 *)u32Buffer;
        u32PcmLen = u32Size;
    }
    else
    {
        switch(u32select)
        {
            case START_BURN:
                SPU_SOURCE = (UINT8 *)pcm_start;
                u32PcmLen = sizeof(pcm_start);
                break;
            case BURN_PASS:
                SPU_SOURCE = (UINT8 *) pcm_pass;
                u32PcmLen = sizeof(pcm_pass);
                break;
            case BURN_FAIL:
                SPU_SOURCE = (UINT8 *) pcm_fail;
                u32PcmLen = sizeof(pcm_fail);
                break;
        }
    }
    spuStartPlay((PFN_DRVSPU_CB_FUNC *) playCallBack, (UINT8 *)SPU_SOURCE);

    while(bPlaying == TRUE)
    {
        if (inp32(REG_SPU_CH_PAUSE))
            break;
    }

    return(0);
}
