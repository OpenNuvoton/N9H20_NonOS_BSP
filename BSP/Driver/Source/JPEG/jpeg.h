/**************************************************************************//**
 * @file     jpeg.h
 * @version  V3.00
 * @brief    N9H20 series JPEG driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#ifndef __JPEG_H__
#define __JPEG_H__

#include "N9H20_JPEG.h"

/* Define for Interrupt Status */
#define JPEG_EER_INTS  EER_INTS
#define JPEG_DER_INTS  DER_INTS
#define JPEG_DEC_INTS  DEC_INTS
#define JPEG_ENC_INTS  ENC_INTS
#define JPEG_DHE_INTS  DHE_INTS
#define JPEG_IPW_INTS  IPW_INTS

/* Define for Scaling */
#define JPEG_ENC_UPSCALE_MODE                         0
#define JPEG_DEC_PACKET_DOWNSCALE_MODE                1
#define JPEG_DEC_PLANAR_DOWNSCALE_MODE                2
#define JPEG_ENC_PLANAR_PRIMARY_DOWNSCALE_MODE        3
#define JPEG_ENC_PLANAR_THUMBNAIL_DOWNSCALE_MODE      4

/* Define for Interrupt Enable */
#define JPEG_EER_INTE  ERR_INTE
#define JPEG_DER_INTE  DER_INTE
#define JPEG_DEC_INTE  DEC_INTE
#define JPEG_ENC_INTE  ENC_INTE
#define JPEG_DHE_INTE  DHE_INTE
#define JPEG_IPW_INTE  IPW_INTE

/* N9H20 */
#define     REG_JMCR        JMCR
#define     REG_JHEADER     JHEADER
#define     REG_JITCR       JITCR
#define     REG_JPRIQC      JPRIQC
#define     REG_JTHBQC      JTHBQC
#define     REG_JPRIWH      JPRIWH
#define     REG_JTHBWH      JTHBWH
#define     REG_JPRST       JPRST
#define     REG_JTRST       JTRST
#define     REG_JDECWH      JDECWH
#define     REG_JINTCR      JINTCR
#define     REG_JTEST       JTEST
#define     REG_JWINDEC0    JWINDEC0
#define     REG_JWINDEC1    JWINDEC1
#define     REG_JWINDEC2    JWINDEC2
#define     REG_JMACR       JMACR
#define     REG_JPSCALU     JPSCALU
#define     REG_JPSCALD     JPSCALD
#define     REG_JTSCALD     JTSCALD
#define     REG_JDBCR       JDBCR
#define     REG_JRESERVE    JRESERVE
#define     REG_JOFFSET     JOFFSET
#define     REG_JFSTRIDE    JFSTRIDE
#define     REG_JYADDR0     JYADDR0
#define     REG_JUADDR0     JUADDR0
#define     REG_JVADDR0     JVADDR0
#define     REG_JYADDR1     JYADDR1
#define     REG_JUADDR1     JUADDR1
#define     REG_JVADDR1     JVADDR1
#define     REG_JYSTRIDE    JYSTRIDE
#define     REG_JUSTRIDE    JUSTRIDE
#define     REG_JVSTRIDE    JVSTRIDE
#define     REG_JIOADDR0    JIOADDR0
#define     REG_JIOADDR1    JIOADDR1
#define     REG_JPRI_SIZE   JPRI_SIZE
#define     REG_JTHB_SIZE   JTHB_SIZE
#define     REG_JUPRAT      JUPRAT
#define     REG_JBSFIFO     JBSFIFO
#define     REG_JSRCH       JSRCH
#define     REG_JQTAB0      JQTAB0
#define     REG_JQTAB1      JQTAB1
#define     REG_JQTAB2      JQTAB2

/* Export functions */
#define JPEG_SET_YADDR(u32Address)           outp32(REG_JYADDR0, u32Address)
#define JPEG_SET_UADDR(u32Address)           outp32(REG_JUADDR0, u32Address)
#define JPEG_SET_VADDR(u32Address)           outp32(REG_JVADDR0, u32Address)
#define JPEG_GET_YADDR()                     inp32(REG_JYADDR0)
#define JPEG_GET_UADDR()                     inp32(REG_JUADDR0)
#define JPEG_GET_VADDR()                     inp32(REG_JVADDR0)
#define JPEG_SET_YSTRIDE(u32Stride)          outp32(REG_JYSTRIDE, u32Stride)
#define JPEG_SET_USTRIDE(u32Stride)          outp32(REG_JUSTRIDE, u32Stride)
#define JPEG_SET_VSTRIDE(u32Stride)          outp32(REG_JVSTRIDE, u32Stride)
#define JPEG_GET_YSTRIDE()                   inp32(REG_JYSTRIDE)
#define JPEG_GET_USTRIDE()                   inp32(REG_JUSTRIDE)
#define JPEG_GET_VSTRIDE()                   inp32(REG_JVSTRIDE)
#define JPEG_SET_BITSTREAM_ADDR(u32Address)  outp32(REG_JIOADDR0,u32Address)
#define JPEG_GET_BITSTREAM_ADDR()            inp32(REG_JIOADDR0)
#define JPEG_SET_ENC_DEC(u8Mode)             outp32(REG_JMCR, (inp32(REG_JMCR) & ~ENC_DEC) | (u8Mode << 7));

//Encode
#define JPEG_GET_ENC_PRIMARY_BITSTREAM_SIZE()    inp32(REG_JPRI_SIZE)
#define JPEG_GET_ENC_THUMBNAIL_BITSTREAM_SIZE()  inp32(REG_JTHB_SIZE)
#define JPEG_SET_SOURCE_IMAGE_HEIGHT(u16Size)    outp32(REG_JSRCH,u16Size)
#define JPEG_GET_SOURCE_IMAGE_HEIGHT()           inp32(REG_JSRCH)
#define JPEG_ENC_ENABLE_UPSCALING()              outp32(REG_JPSCALU,inp32(REG_JPSCALU) | JPSCALU_8X)
#define JPEG_ENC_DISABLE_UPSCALING()             outp32(REG_JPSCALU,inp32(REG_JPSCALU) & ~JPSCALU_8X)
#define JPEG_ENC_ISENABLE_UPSCALING()            ((inp32(REG_JPSCALU) & JPSCALU_8X) >> 6)
#define JPEG_ENC_SET_HEADER_CONTROL(u8Control)   outp32(REG_JHEADER, u8Control)
#define JPEG_ENC_GET_HEADER_CONTROL()            inp32(REG_JHEADER)
#define JPEG_ENC_SET_RDI_VALUE(u8Value)          outp32(REG_JPRST,u8Value)
#define JPEG_ENC_GET_RDI_VALUE()                 inp32(REG_JPRST)

//Decode
#define JPEG_DEC_ENABLE_DOWNSCALING()        outp32(REG_JPSCALD, inp32(REG_JPSCALD) | PSX_ON)
#define JPEG_DEC_ISENABLE_DOWNSCALING()      ((inp32(REG_JPSCALD) & PSX_ON) >> 15)
#define JPEG_DEC_DISABLE_DOWNSCALING()       outp32(REG_JPSCALD, inp32(REG_JPSCALD) & ~PSX_ON)
#define JPEG_DEC_GET_DECODED_IMAGE_FORMAT()  (inp32(REG_JITCR) & DYUV_MODE)
#define JPEG_DEC_ENABLE_LOW_PASS_FILTER()    outp32(REG_JPSCALD,inp32(REG_JPSCALD) | PS_LPF_ON) 
#define JPEG_DEC_DISABLE_LOW_PASS_FILTER()   outp32(REG_JPSCALD,inp32(REG_JPSCALD) & ~PS_LPF_ON) 
#define JPEG_DEC_ISENABLE_LOW_PASS_FILTER()  ((inp32(REG_JPSCALD) & PS_LPF_ON) >> 14)
#define JPEG_DEC_SET_INPUT_WAIT(u16Size)     outp32(REG_JMACR, 0x00400008 | ((u16Size & 0x3FF)<< 8) );
#define JPEG_DEC_RESUME_INPUT_WAIT()         outp32(REG_JMCR,inp32(REG_JMCR) | RESUMEI);
#define JPEG_DEC_DISABLE_WINDOWDECODE()      outp32(REG_JMCR, inp32(REG_JMCR) & ~(WIN_DEC));

/* Interrupt */
#define JPEG_INT_ENABLE(u32Intflag)    outp32(REG_JINTCR, u32Intflag)
#define JPEG_INT_DISABLE(u32Intflag)   outp32(REG_JINTCR, inp32 (REG_JINTCR) & ~(u32Intflag))
#define JPEG_GET_INT_STATUS()          (inp32(REG_JINTCR) & 0xFF)
#define JPEG_CLEAR_INT(u32Intflag)     outp32(REG_JINTCR, (inp32 (REG_JINTCR) & ~0xFF) | u32Intflag)

INT jpegSetEncodeMode(UINT8 u8SourceFormat, UINT16 u16JpegFormat);
INT jpegSetDecodeMode(UINT32 u8OutputFormat);
BOOL jpegPollInt(UINT32 u32Intflag);
VOID jpegEncodeTrigger(VOID);
VOID jpegDecodeTrigger(VOID);

VOID jpegGetDecodedDimension(
    PUINT16 pu16Height,    /* Decode/Encode Height */
    PUINT16 pu16Width      /* Decode/Encode Width */
);

VOID jpegSetDimension(
    UINT16 u16Height,      /* Decode/Encode Height */
    UINT16 u16Width        /* Decode/Encode Width */
);

VOID jpegGetDimension(
    PUINT16 pu16Height,    /* Decoded Height from bit stream */
    PUINT16 pu16Width      /* Decoded Width  from bit stream */
);

INT jpegSetWindowDecode(
    UINT16  u16StartMCUX,  /* Start X MCU */
    UINT16  u16StartMCUY,  /* Horizontal Scaling Factor */
    UINT16  u16EndMCUX,    /* Vertical Scaling Factor */
    UINT16  u16EndMCUY,    /* Horizontal Scaling Factor */
    UINT32  u32Stride      /* Decode Output Stride */
);
INT jpegCalScalingFactor(
    UINT8  u8Mode,              /* Up / Down Scaling */
    UINT16  u16Height,          /* Original Height */
    UINT16  u16Width,           /* Original Width */
    UINT16  u16ScalingHeight,   /* Scaled Height */
    UINT16  u16ScalingWidth,    /* Scaled Width */
    PUINT16  pu16RatioH,        /* Horizontal Ratio */
    PUINT16  pu16RatioW         /* Vertical Ratio */
);

INT jpegSetScalingFactor(
    UINT8  u8Mode,              /* Up / Down Scaling */
    UINT16  u16FactorH,         /* Vertical Scaling Factor */
    UINT16  u16FactorW          /* Horizontal Scaling Factor */
);

VOID jpegGetScalingFactor(
    UINT8  u8Mode,              /* Up / Down Scaling */
    PUINT16  pu16FactorH,       /* Vertical Scaling Factor */
    PUINT16  pu16FactorW        /* Horizontal Scaling Factor */
);

#endif
