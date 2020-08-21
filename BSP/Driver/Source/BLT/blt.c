/**************************************************************************//**
 * @file     blt.c
 * @version  V3.00
 * @brief    N9H20 series BLT driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <stdio.h>
#include "N9H20.h"

static PFN_BLT_CALLBACK _bltCompCb          =   NULL;

static void _blt_intr_hdlr (void)
{
    UINT32 bltIntStat = inp32 (REG_BLTINTCR);
    
    if (bltIntStat & BLT_INTS) {    // BLT complete
        // clear interrupt (write 1 to clear)
        outp32 (REG_BLTINTCR, bltIntStat & BLT_INTE | BLT_INTS);
        if (_bltCompCb) {
            _bltCompCb ();
        }
    }
}

int /* ERRCODE */ bltOpen(void)
{
    // 1.Check I/O pins. If I/O pins are used by other IPs, return error code.
    // 2.Enable IP¡¦s clock
    outp32 (REG_AHBCLK, inp32 (REG_AHBCLK) | BLT_CKE | HCLK4_CKE);
    // 3.Reset IP
    outp32 (REG_AHBIPRST, inp32 (REG_AHBIPRST) | BLTRST);
    outp32 (REG_AHBIPRST, inp32 (REG_AHBIPRST) & ~BLTRST);
    // 4.Configure IP according to inputted arguments.
    // 5.Enable IP I/O pins
    // 6.Register ISR.
    sysInstallISR (IRQ_LEVEL_1, IRQ_BLT, (void *) _blt_intr_hdlr);                      
    sysEnableInterrupt (IRQ_BLT);
    
    // 7.Return 0 to present success
    return Successful;
}


void bltClose(void)
{
    // 1.Disable IP I/O pins
    // 2.Disable IP¡¦s clock
    outp32 (REG_AHBCLK, inp32 (REG_AHBCLK) & ~BLT_CKE);
}

//  Set Transform Matrix | a c |
//                       | b d |
void bltSetTransformMatrix(
    S_DRVBLT_MATRIX sMatrix     // [in] Transformation Matrix 
)
{
    outp32(REG_ELEMENTA, sMatrix.a);
    outp32(REG_ELEMENTB, sMatrix.b);
    outp32(REG_ELEMENTC, sMatrix.c);
    outp32(REG_ELEMENTD, sMatrix.d);    
}

// Get Transform Matrix
void bltGetTransformMatrix(
    S_DRVBLT_MATRIX* psMatrix       // [out] Transformation Matrix 
)
{
    psMatrix->a = inp32(REG_ELEMENTA);
    psMatrix->b = inp32(REG_ELEMENTB);
    psMatrix->c = inp32(REG_ELEMENTC);
    psMatrix->d = inp32(REG_ELEMENTD);  
}

//  Set Pixel Format of Source Image
int /* ERRCODE */
bltSetSrcFormat(
    E_DRVBLT_BMPIXEL_FORMAT eSrcFmt // [in] Source Image Format 
)
{
    if (eSrcFmt & ~0x3F)
        return ERR_BLT_INVALID_SRCFMT;
        
    outp32(REG_SFMT, eSrcFmt);
    return  Successful; 
}

// Get Pixel Format of Source Image
E_DRVBLT_BMPIXEL_FORMAT
bltGetSrcFormat(void)
{
    return (E_DRVBLT_BMPIXEL_FORMAT) (inp32(REG_SFMT) & 0x3F);
}

//  Set Pixel Format of Display 
int /* ERRCODE */
bltSetDisplayFormat(
    E_DRVBLT_DISPLAY_FORMAT eDisplayFmt // [in] Display Format 
)
{
    if (eDisplayFmt & ~0x07)
        return ERR_BLT_INVALID_DSTFMT;
        
    outp32(REG_DFMT, eDisplayFmt);        
    return  Successful; 
}

//  Get Pixel Format of Display 
E_DRVBLT_DISPLAY_FORMAT 
bltGetDisplayFormat(void)
{
    return (E_DRVBLT_DISPLAY_FORMAT) (inp32(REG_DFMT) & 0x07);
}

void 
bltEnableInt(
    E_BLT_INT_TYPE eIntType
)
{
    switch (eIntType) {
    case BLT_INT_CMPLT:
        outp32 (REG_BLTINTCR, inp32 (REG_BLTINTCR) | BLT_INTE); 
        break;
    }
}   

void 
bltDisableInt(
    E_BLT_INT_TYPE eIntType
)
{
    switch (eIntType) {
    case BLT_INT_CMPLT:
        outp32 (REG_BLTINTCR, inp32 (REG_BLTINTCR) & ~BLT_INTE);    
        break;
    }
}

//  Get Interrupt enable/disable status
BOOL bltIsIntEnabled (E_BLT_INT_TYPE eIntType)
{
    BOOL enabled = FALSE;
    
    switch (eIntType) {
    case BLT_INT_CMPLT:
        enabled = !! (inp32 (REG_BLTINTCR) & BLT_INTE);
        break;
    }
    
    return enabled;
}

//  Polling BitBlt Interrupt status
BOOL bltPollInt(E_BLT_INT_TYPE eIntType)
{
    BOOL status = FALSE;
    
    switch (eIntType) {
    case BLT_INT_CMPLT:
        status = !! (inp32 (REG_BLTINTCR) & BLT_INTS);
        break;
    }
    
    return status;
}

void bltInstallCallback (E_BLT_INT_TYPE eIntType, PFN_BLT_CALLBACK pfnCallback, PFN_BLT_CALLBACK* pfnOldCallback)
{
    if (eIntType == BLT_INT_CMPLT) {    // BLT complete
        if (pfnOldCallback) {
            *pfnOldCallback = _bltCompCb;
        }
        _bltCompCb = pfnCallback;
    }
}

// Set A/R/G/B Color Multiplier
void bltSetColorMultiplier(
    S_DRVBLT_ARGB16 sARGB16 // [in] ARGB Multiplier 
)
{
/*
    if ((sARGB16.i16Blue & 0xFFFF0000) || (sARGB16.i16Green & 0xFFFF0000) 
        || (sARGB16.i16Red & 0xFFFF0000) || (sARGB16.i16Alpha & 0xFFFF0000))
        return E_DRVBLT_FALSE_INPUT;    
*/        
    outp32(REG_MLTB, (inp32(REG_MLTB)  & 0xFFFF0000 ) | (sARGB16.i16Blue  & 0x0FFFF));
    outp32(REG_MLTG, (inp32(REG_MLTG)  & 0xFFFF0000 ) | (sARGB16.i16Green & 0x0FFFF));
    outp32(REG_MLTR, (inp32(REG_MLTR)  & 0xFFFF0000 ) | (sARGB16.i16Red   & 0x0FFFF));          
    outp32(REG_MLTA, (inp32(REG_MLTA)  & 0xFFFF0000 ) | (sARGB16.i16Alpha & 0x0FFFF));
}

// Get A/R/G/B Color Multiplier
void bltGetColorMultiplier(
    S_DRVBLT_ARGB16* psARGB16   // [out] ARGB Multiplier 
)
{
    psARGB16->i16Blue   = inp32(REG_MLTB) & 0xFFFF;
    psARGB16->i16Green  = inp32(REG_MLTG) & 0xFFFF;
    psARGB16->i16Red    = inp32(REG_MLTR) & 0xFFFF;
    psARGB16->i16Alpha  = inp32(REG_MLTA) & 0xFFFF;
}

// Set A/R/G/B Color Offset
void bltSetColorOffset(
    S_DRVBLT_ARGB16 sARGB16 // [in] ARGB offset 
)
{

    outp32(REG_MLTB, (inp32(REG_MLTB)  & 0x0000FFFF ) | ((UINT32)(sARGB16.i16Blue  & 0x0FFFF) << 16));
    outp32(REG_MLTG, (inp32(REG_MLTG)  & 0x0000FFFF ) | ((UINT32)(sARGB16.i16Green & 0x0FFFF) << 16));
    outp32(REG_MLTR, (inp32(REG_MLTR)  & 0x0000FFFF ) | ((UINT32)(sARGB16.i16Red   & 0x0FFFF) << 16));          
    outp32(REG_MLTA, (inp32(REG_MLTA)  & 0x0000FFFF ) | ((UINT32)(sARGB16.i16Alpha & 0x0FFFF) << 16));
      
}

// Get A/R/G/B Color Offset
void bltGetColorOffset(
    S_DRVBLT_ARGB16* psARGB16   // [out] ARGB offset 
)
{

    psARGB16->i16Blue   = inp32(REG_MLTB) >> 16;
    psARGB16->i16Green  = inp32(REG_MLTG) >> 16;
    psARGB16->i16Red    = inp32(REG_MLTR) >> 16;
    psARGB16->i16Alpha  = inp32(REG_MLTA) >> 16;
}

// Set Source Image
void bltSetSrcImage(
    S_DRVBLT_SRC_IMAGE sSrcImage    // [in] Source Image Setting
)
{

    outp32(REG_SADDR,    sSrcImage.u32SrcImageAddr);    // Set Source Image Start Addr
    outp32(REG_SSTRIDE,  sSrcImage.i32Stride);          // Set Source Image Stride  
    outp32(REG_OFFSETSX, sSrcImage.i32XOffset);         // Set X offset into the source to start rendering from
    outp32(REG_OFFSETSY, sSrcImage.i32YOffset);         // Set Y offset into the source to start rendering from
    outp32(REG_SWIDTH,   sSrcImage.i16Width);           // Set width of source image
    outp32(REG_SHEIGHT,  sSrcImage.i16Height);          // Set height of source image
}

// Set Destination Frame Buffer
void bltSetDestFrameBuf(
    S_DRVBLT_DEST_FB sFrameBuf  // [in] Frame Buffer Setting
)
{
    outp32(REG_DADDR,    sFrameBuf.u32FrameBufAddr);    // Set Frame Buffer Start Addr
    //outp32(OFFSETDX, sFrameBuf.i32XOffset);           // X offset into the frame buffer in pixels
    //outp32(OFFSETDY, sFrameBuf.i32YOffset);           // Y offset into the frame buffer in pixels
    outp32(REG_DSTRIDE,  sFrameBuf.i32Stride);          // Set Frame Buffer Stride
    outp32(REG_DWIDTH,   sFrameBuf.i16Width);           // Set width of Frame Buffer
    outp32(REG_DHEIGHT,  sFrameBuf.i16Height);          // Set height of Frame Buffer
}

//  Set ARGB color for Fill Operation
void bltSetARGBFillColor(
    S_DRVBLT_ARGB8 sARGB8   // [in] ARGB value for fill operation
)
{
    UINT32 u32ARGB;
    
    u32ARGB = ((UINT32)sARGB8.u8Alpha << 24) | ((UINT32)sARGB8.u8Red << 16) 
              | ((UINT32)sARGB8.u8Green << 8) | sARGB8.u8Blue;  
    outp32(REG_FILLARGB, u32ARGB);            
}

//  Get ARGB color for Fill Operation
void bltGetARGBFillColor(
    S_DRVBLT_ARGB8* psARGB8 // [out] ARGB value for fill operation
)
{
    UINT32 u32ARGB;
    
    u32ARGB = inp32(REG_FILLARGB);
    psARGB8->u8Alpha    = (u32ARGB >> 24) & 0xFF;
    psARGB8->u8Red      = (u32ARGB >> 16) & 0xFF;
    psARGB8->u8Green    = (u32ARGB >> 8) & 0xFF;
    psARGB8->u8Blue     = u32ARGB & 0xFF;   
}

//  Get BitBlt engine busy(rendering) status
BOOL 
bltGetBusyStatus(void)
{
    return !! (inp32(REG_SET2DA) & BLIT_EN);
}

// Set Fill Blending bit
void bltSetFillAlpha(BOOL bEnable)
{
    outp32(REG_SET2DA, (inp32(REG_SET2DA) & ~FILL_BLEND) | (bEnable << 2));
}

// Get Fill Blending bit
BOOL 
bltGetFillAlpha(void)
{
    return !! ((inp32(REG_SET2DA) & FILL_BLEND) >> 2);
}


//  Set Transform Flag
void bltSetTransformFlag(
    UINT32 u32TransFlag         // [in] A combination of the enum E_DRVBLT_TRANSFORM_FLAG
)
{
    outp32(REG_SET2DA, (inp32(REG_SET2DA) & ~TRANS_FLAG) | (u32TransFlag << 4));
}

//  Get Transform Flag
UINT32 
bltGetTransformFlag(void)
{
    return (inp32(REG_SET2DA)  & TRANS_FLAG) >> 4;
}

// Set Palette Endian
void bltSetPaletteEndian(
    E_DRVBLT_PALETTE_ORDER eEndian  // [in] Palette Endian Type
)
{
    outp32(REG_SET2DA, (inp32(REG_SET2DA) & ~L_ENDIAN) | (eEndian << 1));
}

// Get Palette Endian
E_DRVBLT_PALETTE_ORDER 
bltGetPaletteEndian(void)
{
    return (E_DRVBLT_PALETTE_ORDER) ((inp32(REG_SET2DA) & L_ENDIAN) >> 1);
}

// Set the Color Palette used in BitBlt
// The format pointer by pu32ARGB are [31:24] -> Alpha, [23:16] -> Red, [15:8] -> Green, [7:0] -> Blue
void bltSetColorPalette(
    UINT32 u32PaletteInx,       // [in] Color Palette Start index
    UINT32 u32Num,              // [in] Color Palette number to set
    S_DRVBLT_ARGB8* psARGB          // [in] pointer for Color palette from u32PaletteInx
)
{
    UINT32 u32PaletteStartInx;
    UINT32 u32ARGB;
    int i;
    
    u32PaletteStartInx = REG_PALETTE + u32PaletteInx;
    for(i=0;i<u32Num;i++)
    {
        u32ARGB = ((UINT32)(psARGB-> u8Alpha) << 24) | ((UINT32)(psARGB-> u8Red) << 16) | 
                  ((UINT32)(psARGB-> u8Green) << 8) | psARGB-> u8Blue; 

        outp32(u32PaletteStartInx+i*4, u32ARGB);
        psARGB++;
    }
}

// Enable/Disable Fill Operation
void bltSetFillOP(
    E_DRVBLT_FILLOP eOP     // [in] Enable/Disable FillOP
)
{
    outp32(REG_SET2DA,(inp32(REG_SET2DA) &  ~FILL_OP) | (eOP<<11));
}

// Get Fill or Blit Operation 
BOOL 
bltGetFillOP(void)
{
    return !! ((inp32(REG_SET2DA) & FILL_OP) >> 11);
}

// Set Fill stype 
void bltSetFillStyle(
    E_DRVBLT_FILL_STYLE eStyle      // [in] Fill Style for Operation
)
{
    outp32(REG_SET2DA,(inp32(REG_SET2DA) &  ~FILL_STYLE) | (eStyle <<8));
}

// Get Fill stype 
E_DRVBLT_FILL_STYLE 
bltGetFillStyle(void)
{
    return (E_DRVBLT_FILL_STYLE) ((inp32(REG_SET2DA) & FILL_STYLE) >> 8);
}

// Enable/Disable Pre-multiply with alpha
void bltSetRevealAlpha(
    E_DRVBLT_REVEAL_ALPHA eAlpha        // [in] need / no need un-multiply alpha on source image
)
{
    outp32(REG_SET2DA,(inp32(REG_SET2DA) &  ~S_ALPHA) | (eAlpha << 3));
}

// Get Pre-multiply status
BOOL 
bltGetRevealAlpha(void)
{
    return !! ((inp32(REG_SET2DA) & S_ALPHA) >> 3);
}


//  Trigger BitBlt engine to render image
void bltTrigger(void)
{
    outp32(REG_SET2DA,inp32(REG_SET2DA) | BLIT_EN);
}

void bltSetRGB565TransparentColor(
    UINT16 u16RGB565    // [in] RGB565 Transparent Color
)
{
    outp32(REG_TRCOLOR, u16RGB565 & TRCOLOR);
    
}

UINT16 
bltGetRGB565TransparentColor(void)
{
    return inp32(REG_TRCOLOR) & TRCOLOR;
}


void bltSetRGB565TransparentCtl(
BOOL bEnable
)
{
    outp32(REG_SET2DA, (inp32(REG_SET2DA) & ~TRCOLOR_E) | (bEnable << 7));
}

BOOL 
bltGetRGB565TransparentCtl(void)
{
    return !! (inp32(REG_SET2DA) & TRCOLOR_E);
}

void bltFlush (void)
{
    while (bltGetBusyStatus() !=0 );
}

void bltFIBlit(S_FI_BLITOP sBlitOp)
{
    S_DRVBLT_DEST_FB sNewDestFB;
    UINT32 u32BytePerPixel;    
       
    bltFlush ();
    
    bltSetFillOP((E_DRVBLT_FILLOP) FALSE);    
    
    if (sBlitOp.sFISrcImage.psARGB8 != 0)
        bltSetColorPalette(0, 256, sBlitOp.sFISrcImage.psARGB8);
        
#if 1
    // Src Offset is 16.16 formt
    sBlitOp.sFISrcImage.sDrvSrcImage.i32XOffset = sBlitOp.sFISrcImage.sDrvSrcImage.i32XOffset << 16;        
    sBlitOp.sFISrcImage.sDrvSrcImage.i32YOffset = sBlitOp.sFISrcImage.sDrvSrcImage.i32YOffset << 16;    
#endif          
    bltSetSrcImage(sBlitOp.sFISrcImage.sDrvSrcImage);    

    // Remove OFFSETDX and OFFSETDY
    if (inp32(REG_DFMT) & D_ARGB8888)
        u32BytePerPixel = 4;
    else
        u32BytePerPixel = 2;
        
    sNewDestFB.i32Stride       = sBlitOp.sDestFB.i32Stride;  
    sNewDestFB.i16Width        = sBlitOp.sDestFB.i16Width; 
    sNewDestFB.i16Height       = sBlitOp.sDestFB.i16Height; 

    sNewDestFB.u32FrameBufAddr = sBlitOp.sDestFB.u32FrameBufAddr + sBlitOp.sDestFB.i32YOffset * sBlitOp.sDestFB.i32Stride 
                                 +  sBlitOp.sDestFB.i32XOffset *u32BytePerPixel;
                                 
    sNewDestFB.i32XOffset      = 0; 
    sNewDestFB.i32YOffset      = 0;  
    
    bltSetDestFrameBuf(sNewDestFB);
    
    if (sBlitOp.psTransform != NULL)
    {
        bltSetTransformMatrix(sBlitOp.psTransform->sMatrix);    
        bltSetSrcFormat(sBlitOp.psTransform->eSrcFmt);
        bltSetDisplayFormat(sBlitOp.psTransform->eDestFmt);     
        bltSetTransformFlag(sBlitOp.psTransform->i32Flag);
        bltSetColorMultiplier(sBlitOp.psTransform->sColorMultiplier);  
        bltSetColorOffset(sBlitOp.psTransform->sColorOffset);           
        bltSetFillStyle(sBlitOp.psTransform->eFilStyle);
    }   
    else {
        // Nuvoton CCLi8 (2012.08.06)
        // FIXME
        //defaultSetting();
        //outp32(REG_MLTA, 0x00000100);
        //outp32(REG_MLTR, 0x00000100);
        //outp32(REG_MLTG, 0x00000100);
        //outp32(REG_MLTB, 0x00000100);

        //outp32(REG_ELEMENTA, 0x00010000); 
        //outp32(REG_ELEMENTB, 0x00000000);
        //outp32(REG_ELEMENTC, 0x00000000);
        //outp32(REG_ELEMENTD, 0x00010000); 
        
        S_DRVBLT_MATRIX xform_mx = {0x00010000, 0x00000000, 0x00000000, 0x00010000};
        bltSetTransformMatrix (xform_mx);
        bltSetTransformFlag (0);
        bltSetFillStyle ((E_DRVBLT_FILL_STYLE) 0);
    }
    
    bltTrigger(); 
    
    bltFlush ();
}


void bltFIFill(S_FI_FILLOP sFillOp)
{
    S_DRVBLT_DEST_FB sDestFB;
    S_DRVBLT_ARGB8   sARGB8;
    UINT32 u32BytePerPixel;
    
    
    bltFlush ();
    
    bltSetFillOP((E_DRVBLT_FILLOP) TRUE);
    
    sARGB8.u8Blue   = sFillOp.sARGB8.u8Blue;
    sARGB8.u8Green  = sFillOp.sARGB8.u8Green;
    sARGB8.u8Red    = sFillOp.sARGB8.u8Red; 
    sARGB8.u8Alpha  = sFillOp.sARGB8.u8Alpha;             
    bltSetARGBFillColor(sARGB8);
    
    if (inp32(REG_DFMT) & D_ARGB8888)
        u32BytePerPixel = 4;
    else
        u32BytePerPixel = 2;
        
    sDestFB.i32Stride       = sFillOp.i32Stride;  
    sDestFB.i16Width        = sFillOp.sRect.i16Xmax - sFillOp.sRect.i16Xmin; 
    sDestFB.i16Height       = sFillOp.sRect.i16Ymax - sFillOp.sRect.i16Ymin; 

    sDestFB.u32FrameBufAddr = sFillOp.u32FBAddr + sFillOp.sRect.i16Ymin*sFillOp.i32Stride +  sFillOp.sRect.i16Xmin *u32BytePerPixel; 

    sDestFB.i32XOffset      = 0; 
    sDestFB.i32YOffset      = 0;     
    
    bltSetDestFrameBuf(sDestFB);
    
    bltSetDisplayFormat(sFillOp.eDisplayFmt);
    
    bltSetFillAlpha(sFillOp.i32Blend);
    
    bltTrigger();
    
    bltFlush ();
}
