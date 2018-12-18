/****************************************************************************
 * @file     cmd_bootm.c
 * @version  V1.00
 * $Revision: 4 $
 * $Date: 18/04/25 11:43a $
 * @brief    SpiLoader_gzip source file
 *
 * @note
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/
#include ".\ziplib\bzlib.h"
#include ".\ziplib\zlib.h"
#include ".\ziplib\image.h"
#include "string.h"
#include "stdlib.h"

 #define ntohl(x)  (((x & 0xFF) << 24) | ((x & 0xFF00) << 8) | ((x & 0xFF0000) >> 8)| ((x & 0xFF000000) >> 24))
 
int  gunzip (void *, int, unsigned char *, unsigned long *);

static void *zalloc(void *, unsigned, unsigned);
static void zfree(void *, void *, unsigned);

#define ulong unsigned long
#define uchar unsigned char

#ifdef _N9H20K5_
#define CFG_MALLOC_LEN 8192 * 1024 /* 192*1024 */


#ifndef CFG_BOOTM_LEN
#define CFG_BOOTM_LEN  0x800000  /* use 8MByte as default max gunzip size */
#endif
#endif

#ifdef _N9H20K3_
#define CFG_MALLOC_LEN 4096 * 1024 /* 192*1024 */


#ifndef CFG_BOOTM_LEN
#define CFG_BOOTM_LEN  0x400000  /* use 8MByte as default max gunzip size */
#endif
#endif

#ifdef _N9H20K1_
#define CFG_MALLOC_LEN 1024 * 1024 /* 192*1024 */


#ifndef CFG_BOOTM_LEN
#define CFG_BOOTM_LEN  0x100000  /* use 8MByte as default max gunzip size */
#endif
#endif

image_header_t header;

int do_bootm (UINT32 u32SrcAddress,UINT32 u32DestAddress, UINT32 u32Mode)
{
    unsigned long  addr;
    unsigned long  data, len;
//     unsigned long  *len_ptr;
    unsigned int  unc_len = CFG_BOOTM_LEN;
    image_header_t *hdr = &header;

    addr  =u32SrcAddress;

    sysprintf ("## Booting image at 0x%08x ...\n", addr);

    /* Copy header so we can blank CRC field for re-calculation */
    memcpy (&header, (char *)addr, sizeof(image_header_t));

    if (ntohl(hdr->ih_magic) != IH_MAGIC)
    {
        sysprintf ("Bad Magic Number\n");
        return 1;
    }
    else
    {
        sysprintf ("Get Magic Number\n");
        if(u32Mode)
            return 0;
    }

    data = (ulong)&header;
    len  = sizeof(image_header_t);

    hdr->ih_hcrc = 0;

    data = addr + sizeof(image_header_t);
    len  = ntohl(hdr->ih_size);

//     len_ptr = (ulong *)data;
    
    /*
     * We have reached the point of no return: we are going to
     * overwrite all exception vector code, so we cannot easily
     * recover from any failures any more...
     */
    if(hdr->ih_comp ==IH_COMP_GZIP) 
    {
        sysprintf ("   Gzip Uncompressing to 0x%X ... ",u32DestAddress);
        if (gunzip ((void *)u32DestAddress, unc_len, (uchar *)data, &len) != 0) 
        {
            sysprintf ("GUNZIP ERROR\n");
        }
    }
    else
    {
        sysprintf ("Unimplemented compression type %d\n", hdr->ih_comp);
        return 1;
    }
    sysprintf ("OK\n");

    return 1;
}

#define  ZALLOC_ALIGNMENT  16

static void *zalloc(void *x, unsigned items, unsigned size)
{
    void *p;

    size *= items;
    size = (size + ZALLOC_ALIGNMENT - 1) & ~(ZALLOC_ALIGNMENT - 1);

    p = (void *)malloc (size);

    return (p);
}

static void zfree(void *x, void *addr, unsigned nb)
{
    free (addr);
}

#define HEAD_CRC      2
#define EXTRA_FIELD   4
#define ORIG_NAME     8
#define COMMENT    0x10
#define RESERVED   0xe0

#define DEFLATED      8

int gunzip(void *dst, int dstlen, unsigned char *src, unsigned long *lenp)
{
    z_stream s;
    int r, i, flags;

    /* skip header */
    i = 10;
    flags = src[3];
    if (src[2] != DEFLATED || (flags & RESERVED) != 0)
    {
        sysprintf ("Error: Bad gzipped data\n");
        return (-1);
    }
    if ((flags & EXTRA_FIELD) != 0)
        i = 12 + src[10] + (src[11] << 8);
    if ((flags & ORIG_NAME) != 0)
        while (src[i++] != 0)
            ;
    if ((flags & COMMENT) != 0)
        while (src[i++] != 0)
            ;
    if ((flags & HEAD_CRC) != 0)
        i += 2;
    if (i >= *lenp)
    {
        sysprintf ("Error: gunzip out of data in header\n");
        return (-1);
    }

    s.zalloc = zalloc;
    s.zfree = zfree;

    s.outcb = Z_NULL;

    r = inflateInit2(&s, -MAX_WBITS);
    if (r != Z_OK) 
    {
        sysprintf ("Error: inflateInit2() returned %d\n", r);
        return (-1);
    }
    s.next_in = src + i;
    s.avail_in = *lenp - i;
    s.next_out = dst;
    s.avail_out = dstlen;
    r = inflate(&s, Z_FINISH);
    if (r != Z_OK && r != Z_STREAM_END) 
    {
        sysprintf ("Error: inflate() returned %d\n", r);
        return (-1);
    }
    *lenp = s.next_out - (unsigned char *) dst;
    inflateEnd(&s);

    return (0);
}


#ifdef CONFIG_BZIP2
void bz_internal_error(int errcode)
{
    sysprintf ("BZIP2 internal error %d\n", errcode);
}
#endif /* CONFIG_BZIP2 */
