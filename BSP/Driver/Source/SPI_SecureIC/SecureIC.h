/**
  ******************************************************************************
  * @file    SecureIC.h
  * @author  Winbond FAE Steam Lin
  * @version V1.1.0
  * @date    09-December-2015
  * @brief   This code provide the low level RPMC hardware operate function based on STM32F205.
  *            
  * COPYRIGHT 2015 Winbond Electronics Corporation.
*/ 
#ifndef __SPI_SECUREIC_H
#define __SPI_SECUREIC_H

#ifdef __cplusplus
 extern "C" {
#endif

/*****************************************************************************************
 * RMPC Functions (13/04/23)
 *****************************************************************************************/
#define uchar unsigned char   /* 8-bit byte */

#if !defined (__GNUC__)
#define uint unsigned long    /* 32-bit word */
#endif
/* DBL_INT_ADD treats two unsigned ints a and b as one 64-bit integer and adds c to it */
#define DBL_INT_ADD(a,b,c) if (a > 0xffffffff - (c)) ++b; a += c;
#define ROTLEFT(a,b) (((a) << (b)) | ((a) >> (32-(b))))
#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))
#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))
#define IPAD 0x36
#define OPAD 0x5C

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

typedef struct {
    uchar data[64];
    uint datalen;
    uint bitlen[2];
    uint state[8];
} SHA256_CTX;

/* SIC Functions */
void sha256(unsigned char *text1 ,unsigned char *output,unsigned int length);
void sha256_init(SHA256_CTX *ctx);
void sha256_update(SHA256_CTX *ctx, uchar data[], uint len);
void sha256_final(SHA256_CTX *ctx, uchar hash[]);
void sha256_transform(SHA256_CTX *ctx, uchar data[]);
unsigned int hmacsha256(unsigned char *key, unsigned int  key_len, unsigned char *text, unsigned int  text_len, unsigned char *digest);
unsigned int RPMC_ReadRPMCstatus(unsigned int checkall);
void RPMC_ReqCounter(unsigned int cadr, unsigned char *hmackey,unsigned char *tag);
#ifdef __cplusplus
 }
#endif
 
#endif /* __SPI_SECUREIC_H */
 
