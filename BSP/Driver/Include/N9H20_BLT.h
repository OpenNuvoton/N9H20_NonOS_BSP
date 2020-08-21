/**************************************************************************//**
 * @file     N9H20_BLT.h
 * @version  V3.00
 * @brief    N9H20 series BLT driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
 
#ifndef __BLT_H__
#define __BLT_H__

#ifdef  __cplusplus
extern "C"
{
#endif

#include "wblib.h"

#define ERR_BLT_INVALID_INT         (BLT_ERR_ID | 0x01)
#define ERR_BLT_INVALID_SRCFMT      (BLT_ERR_ID | 0x02)
#define ERR_BLT_INVALID_DSTFMT      (BLT_ERR_ID | 0x03)

typedef void (*PFN_BLT_CALLBACK) (void);

typedef enum {
    BLT_INT_CMPLT       = 1
} E_BLT_INT_TYPE;

typedef enum
{
    eDRVBLT_DISABLE,      
    eDRVBLT_ENABLE     
} E_DRVBLT_FILLOP;

typedef enum
{
    eDRVBLT_EFFECTIVE,      
    eDRVBLT_NO_EFFECTIVE     
} E_DRVBLT_REVEAL_ALPHA;

typedef enum
{
    eDRVBLT_NONTRANSPARENCY,     
    eDRVBLT_HASTRANSPARENCY=1,     
    eDRVBLT_HASCOLORTRANSFORM=2, 
    eDRVBLT_HASALPHAONLY=4           
} E_DRVBLT_TRANSFORM_FLAG;

typedef enum
{
    eDRVBLT_SRC_ARGB8888=1,     
    eDRVBLT_SRC_RGB565, 
    eDRVBLT_SRC_1BPP=4,             
    eDRVBLT_SRC_2BPP=8,    
    eDRVBLT_SRC_4BPP=16,     
    eDRVBLT_SRC_8BPP=32
} E_DRVBLT_BMPIXEL_FORMAT;

typedef enum
{
    eDRVBLT_DEST_ARGB8888=1,    
    eDRVBLT_DEST_RGB565, 
    eDRVBLT_DEST_RGB555=4         
} E_DRVBLT_DISPLAY_FORMAT;

typedef enum
{
    eDRVBLT_CLIP_TO_EDGE=1,    
    eDRVBLT_NOTSMOOTH,    
    eDRVBLT_NONE_FILL=4
} E_DRVBLT_FILL_STYLE;

typedef enum
{
    eDRVBLT_BIG_ENDIAN,     
    eDRVBLT_LITTLE_ENDIAN  
} E_DRVBLT_PALETTE_ORDER;


typedef struct {
    INT32 a;
    INT32 b;
    INT32 c;
    INT32 d;
} S_DRVBLT_MATRIX;

typedef struct {
    INT16  i16Blue;
    INT16  i16Green;
    INT16  i16Red;
    INT16  i16Alpha;
} S_DRVBLT_ARGB16;

typedef struct {
    INT16   i16Xmin;
    INT16   i16Xmax;
    INT16   i16Ymin;
    INT16   i16Ymax;
} S_DRVBLT_RECT;

typedef struct {
    UINT8   u8Blue;
    UINT8   u8Green;
    UINT8   u8Red;
    UINT8   u8Alpha;
} S_DRVBLT_ARGB8;

typedef struct {
//    S_DRVBLT_ARGB8*  pSARGB8;
    UINT32              u32SrcImageAddr;
    INT32               i32Stride;    
    INT32               i32XOffset;
    INT32               i32YOffset;
    INT16               i16Width;
    INT16               i16Height;   
} S_DRVBLT_SRC_IMAGE;    

typedef struct {
    UINT32              u32FrameBufAddr;
    INT32               i32XOffset;
    INT32               i32YOffset;
    INT32               i32Stride;     
    INT16               i16Width;
    INT16               i16Height;      
} S_DRVBLT_DEST_FB;    


// APIs declaration

int /* ERRCODE */ bltOpen (void);
void bltClose (void);
void bltSetTransformMatrix (S_DRVBLT_MATRIX sMatrix);

void bltGetTransformMatrix (S_DRVBLT_MATRIX* psMatrix);

int /* ERRCODE */ bltSetSrcFormat (E_DRVBLT_BMPIXEL_FORMAT eSrcFmt);

E_DRVBLT_BMPIXEL_FORMAT 
bltGetSrcFormat(void);

int /* ERRCODE */
bltSetDisplayFormat(
    E_DRVBLT_DISPLAY_FORMAT eDisplayFmt // [in] Display Format 
);

E_DRVBLT_DISPLAY_FORMAT 
bltGetDisplayFormat(void);

void bltEnableInt (E_BLT_INT_TYPE eIntType);
void bltDisableInt (E_BLT_INT_TYPE eIntType);
BOOL bltIsIntEnabled (E_BLT_INT_TYPE eIntType);
BOOL bltPollInt (E_BLT_INT_TYPE eIntType);
void bltInstallCallback (E_BLT_INT_TYPE eIntType, PFN_BLT_CALLBACK pfnCallback, PFN_BLT_CALLBACK *pfnOldCallback);

void bltSetColorMultiplier(
    S_DRVBLT_ARGB16 sARGB16 // [in] ARGB Multiplier 
);

void bltGetColorMultiplier(
    S_DRVBLT_ARGB16* psARGB16   // [out] ARGB Multiplier 
);

void bltSetColorOffset(
    S_DRVBLT_ARGB16 sARGB16 // [in] ARGB offset 
);

void bltGetColorOffset(
    S_DRVBLT_ARGB16* psARGB16   // [out] ARGB offset 
);

void bltSetSrcImage(
    S_DRVBLT_SRC_IMAGE sSrcImage    // [in] Source Image Setting
);

void bltSetDestFrameBuf(
    S_DRVBLT_DEST_FB sFrameBuf  // [in] Frame Buffer Setting
);

void bltSetARGBFillColor(
    S_DRVBLT_ARGB8 sARGB8   // [in] ARGB value for fill operation
);

void bltGetARGBFillColor(
    S_DRVBLT_ARGB8* psARGB8 // [out] ARGB value for fill operation
);

BOOL 
bltGetBusyStatus(void);

void bltSetFillAlpha(
BOOL bEnable
);

BOOL 
bltGetFillAlpha(void);

void bltSetTransformFlag(
    UINT32 u32TransFlag         // [in] A combination of the enum E_DRVBLT_TRANSFORM_FLAG
);

UINT32 
bltGetTransformFlag(void);

void bltSetPaletteEndian(
    E_DRVBLT_PALETTE_ORDER eEndian  // [in] Palette Endian Type
);

E_DRVBLT_PALETTE_ORDER 
bltGetPaletteEndian(void);

void bltSetColorPalette(
    UINT32 u32PaletteInx,       // [in] Color Palette Start index
    UINT32 u32Num,              // [in] Color Palette number to set
    S_DRVBLT_ARGB8* psARGB  // [in] pointer for Color palette from u32PaletteInx
);

void bltSetFillOP(
    E_DRVBLT_FILLOP eOP     // [in] Enable/Disable FillOP
);

BOOL 
bltGetFillOP(void);

void bltSetFillStyle(
    E_DRVBLT_FILL_STYLE eStyle      // [in] Fill Style for Fill Operation
);

E_DRVBLT_FILL_STYLE 
bltGetFillStyle(void);

void bltSetRevealAlpha(
    E_DRVBLT_REVEAL_ALPHA eAlpha        // [in] need / no need un-multiply alpha on source image
);

BOOL 
bltGetRevealAlpha(void);

void bltTrigger(void);

void bltSetRGB565TransparentColor(
    UINT16 u16RGB565    // [in] RGB565 Transparent Color
);

UINT16 
bltGetRGB565TransparentColor(void);

void bltSetRGB565TransparentCtl(
BOOL bEnable
);

BOOL 
bltGetRGB565TransparentCtl(void);

void bltFlush (void);

// Flash Lite BLT I/F
typedef struct {
    S_DRVBLT_MATRIX          sMatrix;
    E_DRVBLT_BMPIXEL_FORMAT  eSrcFmt;
    E_DRVBLT_DISPLAY_FORMAT  eDestFmt;
    
    INT32                       i32Flag;
    S_DRVBLT_ARGB16          sColorMultiplier;
    S_DRVBLT_ARGB16          sColorOffset;
    E_DRVBLT_FILL_STYLE      eFilStyle;
} S_FI_BLITTRANSFORMATION;


typedef struct {
    S_DRVBLT_RECT            sRect;
    S_DRVBLT_ARGB8           sARGB8;
    UINT32                      u32FBAddr;
    INT32                       i32Stride;
    E_DRVBLT_DISPLAY_FORMAT  eDisplayFmt;
    INT32                       i32Blend;
} S_FI_FILLOP;

typedef struct {
    S_DRVBLT_ARGB8          *psARGB8;
    S_DRVBLT_SRC_IMAGE      sDrvSrcImage;
} S_FI_SRC_IMAGE;

typedef struct {
    S_FI_SRC_IMAGE              sFISrcImage;
    S_DRVBLT_DEST_FB         sDestFB;
    S_FI_BLITTRANSFORMATION     *psTransform;
} S_FI_BLITOP;

void bltFIBlit (S_FI_BLITOP sBiltOp);
void bltFIFill (S_FI_FILLOP sFillOp);

#ifdef  __cplusplus
}
#endif

#endif  // __BLT_H__






