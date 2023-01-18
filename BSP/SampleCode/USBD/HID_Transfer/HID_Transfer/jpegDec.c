#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wbtypes.h"
#include "wblib.h"
#include "N9H20_JPEG.h"
#include "Common.h"

/*-----------------------------------------------------------------------*/
/*  Header Decode Complete Callback function                             */
/*-----------------------------------------------------------------------*/
BOOL JpegDecHeaderComplete(void)
{
    JPEG_INFO_T jpegInfo;
    UINT32 u32TargetWidth,u32TargetHeight;
    UINT32 u32OutputOffset = 0;

    /* Get the JPEG information(jpeg_width, jpeg_height, and yuvformat are valid) */
    jpegGetInfo(&jpegInfo);

    if(jpegInfo.jpeg_width == 0 || jpegInfo.jpeg_height == 0)	
        return FALSE;

    /* DownScale size control */
    if(jpegInfo.jpeg_width > jpegInfo.jpeg_height)
    {
        if((jpegInfo.jpeg_width > PANEL_WIDTH || jpegInfo.jpeg_height > PANEL_HEIGHT))
        {
            /* Set Downscale to QVGA */
            jpegIoctl(JPEG_IOCTL_SET_DECODE_DOWNSCALE, PANEL_HEIGHT, PANEL_WIDTH);
            u32TargetHeight = PANEL_HEIGHT;
            u32TargetWidth = PANEL_WIDTH;	
        }
        else
        {
            u32TargetHeight = jpegInfo.jpeg_height;
            u32TargetWidth = jpegInfo.jpeg_width;
        }
    }
    else
    {
        if((jpegInfo.jpeg_width > PANEL_WIDTH || jpegInfo.jpeg_height > PANEL_HEIGHT))
        {
            UINT32 ratio;
            ratio = jpegInfo.jpeg_height / PANEL_HEIGHT + 1;
            /* Set Downscale to QVGA */
            jpegIoctl(JPEG_IOCTL_SET_DECODE_DOWNSCALE, jpegInfo.jpeg_height / ratio, jpegInfo.jpeg_width / ratio);
            u32TargetHeight = jpegInfo.jpeg_height / ratio;
            u32TargetWidth = jpegInfo.jpeg_width / ratio;	
        }
        else
        {
            u32TargetHeight = jpegInfo.jpeg_height;
            u32TargetWidth = jpegInfo.jpeg_width;
        }
    }

    /* Set Decode Stride to Panel width */
    jpegIoctl(JPEG_IOCTL_SET_DECODE_STRIDE, PANEL_WIDTH, 0);

    /* The pixel offset for putting the image at the center of Frame Buffer */
    u32OutputOffset = (UINT32)((PANEL_WIDTH * ((UINT32)(PANEL_HEIGHT - u32TargetHeight) / 2))) + (UINT32)((PANEL_WIDTH - u32TargetWidth) / 2);

    /* Change Raw data Output address (Let JPEG engine output data to the center of Panel Buffer) */
    jpegIoctl(JPEG_IOCTL_SET_YADDR, (UINT32) g_pu8FrameBuffer + u32OutputOffset * 2, 0);

    return TRUE;    /* Return TRUE to continue Decode operation, Otherwise, Stop Decode operation */
}
/*-----------------------------------------------------------------------*/
/*  Decode Function                                                      */
/*-----------------------------------------------------------------------*/  
INT JpegDec(UINT32 u32SrcAddr, UINT32 u32DestAddr)
{
    /* JPEG Init */
    jpegInit();

    /* Set Bit stream Address */   
    jpegIoctl(JPEG_IOCTL_SET_BITSTREAM_ADDR,(UINT32) u32SrcAddr, 0);
             
    /* Decode mode */	
    jpegIoctl(JPEG_IOCTL_SET_DECODE_MODE, JPEG_DEC_PRIMARY_PACKET_RGB565, 0);

    /* Set JPEG Header Decode End Call Back Function */
    jpegIoctl(JPEG_IOCTL_SET_HEADERDECODE_CALBACKFUN, (UINT32) JpegDecHeaderComplete, 0);

    /* Trigger JPEG decoder */
    jpegIoctl(JPEG_IOCTL_DECODE_TRIGGER, 0, 0);  
        
    /* Wait for complete */
//    if(!jpegWait())
//    {
//        sysprintf("JPEG Decode Error!!\n");
//        return 1;
//    }

    return 0;
}


