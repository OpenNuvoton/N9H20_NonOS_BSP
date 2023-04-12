/**************************************************************************//**
 * @file     N9H20_JPEG.h
 * @version  V3.00
 * @brief    N9H20 series JPEG driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#ifndef __JPEGCODEC_H__
#define __JPEGCODEC_H__

/* Include header file */
#include "wbio.h"
#include "wblib.h"

#define E_JPEG_FAIL            0
#define E_JPEG_SUCCESS         1
#define E_JPEG_INVALID_PARAM   2
#define E_JPEG_TIMEOUT         3

/* Define Constant */
#define JPEG_MAJOR_NUM         3
#define JPEG_MINOR_NUM         30
#define JPEG_BUILD_NUM         1

#define JPEG_ENC_PRIMARY       0
#define JPEG_ENC_THUMBNAIL     1

/* Define for Encode input Format */
#define JPEG_ENC_SOURCE_PLANAR    0
#define JPEG_ENC_SOURCE_PACKET    1

/* Define for Decode Output Format */
#define JPEG_DEC_PRIMARY_PLANAR_YUV       0x08021  /* (PLANAR_ON | PDHTAB | DHEND) */
#define JPEG_DEC_PRIMARY_PACKET_YUV422    0x00021  /* (PDHTAB | DHEND) */
#define JPEG_DEC_PRIMARY_PACKET_RGB555    0x04021  /* (PDHTAB | DHEND | ORDER) */
#define JPEG_DEC_PRIMARY_PACKET_RGB565    0x06021  /* (PDHTAB | DHEND | RGB555_565 | ORDER) */
#define JPEG_DEC_PRIMARY_PACKET_RGB888    0x14021  /* (PDHTAB | DHEND | ORDER | ARGB888) */
#define JPEG_DEC_THUMBNAIL_PLANAR_YUV     0x08031  /* (PLANAR_ON | DTHB | PDHTAB) */
#define JPEG_DEC_THUMBNAIL_PACKET_YUV422  0x00031  /* (DTHB | PDHTAB | DHEND) */
#define JPEG_DEC_THUMBNAIL_PACKET_RGB555  0x04031  /* (DTHB | PDHTAB | DHEND | ORDER) */

/* Define for Encode Image Format */
#define JPEG_ENC_PRIMARY_YUV420           0xA0
#define JPEG_ENC_PRIMARY_YUV422           0xA8
#define JPEG_ENC_PRIMARY_GRAY             0xA1
#define JPEG_ENC_THUMBNAIL_YUV420         0x90
#define JPEG_ENC_THUMBNAIL_YUV422         0x98
#define JPEG_ENC_THUMBNAIL_GRAY           0x91

/* Define for Decode Image Format */
#define JPEG_DEC_YUV420        0x000
#define JPEG_DEC_YUV422        0x100
#define JPEG_DEC_YUV444        0x200
#define JPEG_DEC_YUV411        0x300
#define JPEG_DEC_GRAY          0x400
#define JPEG_DEC_YUV422T       0x500

/* Define for Encode Image Header */
#define JPEG_ENC_PRIMARY_DRI     0x10 /*P_DRI*/
#define JPEG_ENC_PRIMARY_QTAB    0x20 /*P_QTAB*/
#define JPEG_ENC_PRIMARY_HTAB    0x40 /*P_HTAB*/
#define JPEG_ENC_PRIMARY_JFIF    0x80 /*P_JFIF*/
#define JPEG_ENC_THUMBNAIL_DRI   0x1 /*T_DRI*/
#define JPEG_ENC_THUMBNAIL_QTAB  0x2 /*T_QTAB*/
#define JPEG_ENC_THUMBNAIL_HTAB  0x4 /*T_HTAB*/
#define JPEG_ENC_THUMBNAIL_JFIF  0x8 /*T_JFIF*/

#define JPEG_IOCTL_SET_YADDR                                  0
#define JPEG_IOCTL_SET_YSTRIDE                                1
#define JPEG_IOCTL_SET_USTRIDE                                2
#define JPEG_IOCTL_SET_VSTRIDE                                3
#define JPEG_IOCTL_SET_BITSTREAM_ADDR                         4
#define JPEG_IOCTL_SET_SOURCE_IMAGE_HEIGHT                    5
#define JPEG_IOCTL_ENC_SET_HEADER_CONTROL                     6
#define JPEG_IOCTL_SET_DEFAULT_QTAB                           7
#define JPEG_IOCTL_SET_DECODE_MODE                            8
#define JPEG_IOCTL_SET_ENCODE_MODE                            9
#define JPEG_IOCTL_SET_DIMENSION                             10
#define JPEG_IOCTL_ENCODE_TRIGGER                            11
#define JPEG_IOCTL_DECODE_TRIGGER                            12
#define JPEG_IOCTL_WINDOW_DECODE                             13
#define JPEG_IOCTL_SET_DECODE_STRIDE                         14
#define JPEG_IOCTL_SET_DECODE_DOWNSCALE                      15
#define JPEG_IOCTL_SET_ENCODE_UPSCALE                        16
#define JPEG_IOCTL_SET_HEADERDECODE_CALBACKFUN               17
#define JPEG_IOCTL_SET_DECINPUTWAIT_CALBACKFUN               18
#define JPEG_IOCTL_ADJUST_QTAB                               19
#define JPEG_IOCTL_ENC_RESERVED_FOR_SOFTWARE                 20
#define JPEG_IOCTL_SET_UADDR                                 21
#define JPEG_IOCTL_SET_VADDR                                 22
#define JPEG_IOCTL_SET_ENCODE_PRIMARY_RESTART_INTERVAL       23
#define JPEG_IOCTL_SET_ENCODE_THUMBNAIL_RESTART_INTERVAL     24
#define JPEG_IOCTL_GET_ENCODE_PRIMARY_RESTART_INTERVAL       25
#define JPEG_IOCTL_GET_ENCODE_THUMBNAIL_RESTART_INTERVAL     26
#define JPEG_IOCTL_SET_THUMBNAIL_DIMENSION                   27
#define JPEG_IOCTL_SET_ENCODE_SW_OFFSET                      28
#define JPEG_IOCTL_GET_THUMBNAIL_DIMENSION                   29
#define JPEG_IOCTL_GET_ENCODE_SW_OFFSET                      30
#define JPEG_IOCTL_SET_ENCODE_PRIMARY_DOWNSCALE              31
#define JPEG_IOCTL_SET_ENCODE_THUMBNAIL_DOWNSCALE            32
#define JPEG_IOCTL_SET_ENCODE_PRIMARY_ROTATE_RIGHT           33
#define JPEG_IOCTL_SET_ENCODE_PRIMARY_ROTATE_LEFT            34
#define JPEG_IOCTL_SET_ENCODE_PRIMARY_ROTATE_NORMAL          35
#define JPEG_IOCTL_GET_WINDOW_DECODE_SIZE                    36

typedef BOOL (*PFN_JPEG_HEADERDECODE_CALLBACK)(VOID);
typedef BOOL (*PFN_JPEG_DECINPUTWAIT_CALLBACK)(UINT32 u32Address,UINT32 u32Size);

typedef struct{
    /* decode information */
    UINT32  yuvformat;        /*for decode*/
    UINT32  width;            /*for decode*/
    UINT32  height;           /*for decode*/
    UINT32  jpeg_width;       /*for decode*/
    UINT32  jpeg_height;      /*for decode*/        
    UINT32  stride;           /*for decode*/    
    /* encode information */
    UINT32  bufferend;
    UINT32  image_size[2];    /* image size after encoded */
}JPEG_INFO_T;

typedef struct{
    UINT16  u16StartMCUX;     /* Start X MCU */
    UINT16  u16StartMCUY;     /* Horizontal Scaling Factor */
    UINT16  u16EndMCUX;       /* Vertical Scaling Factor */
    UINT16  u16EndMCUY;       /* Horizontal Scaling Factor */
    UINT32  u32Stride;        /* Decode Output Stride */
}JPEG_WINDOW_DECODE_T;

/* Define inline function */
INT jpegOpen(VOID);
VOID jpegClose(VOID);
VOID jpegInit(VOID);
VOID jpegGetInfo(JPEG_INFO_T *info);
BOOL jpegIsReady(VOID);

INT jpegSetQTAB(
    PUINT8 puQTable0,
    PUINT8 puQTable1,
    PUINT8 puQTable2,
    UINT8 u8num
);

INT jpegWait(VOID);
VOID jpegIoctl(UINT32 cmd, UINT32 arg0, UINT32 arg1);

#endif
