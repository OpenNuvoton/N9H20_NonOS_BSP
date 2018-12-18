/****************************************************************************
 * @file     Gneiss.h
 * @version  V1.00
 * $Revision: 4 $
 * $Date: 18/04/25 11:43a $
 * @brief    SPI Securic driver header file
 *
 * @note
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#include "wblib.h"
unsigned int RPMC_ReadCounterData(void);
unsigned int RPMC_ReadRPMCstatus(unsigned int checkall);
unsigned int RPMC_WrRootKey(unsigned int cadr,unsigned char *rootkey);
unsigned int RPMC_UpHMACkey(unsigned int cadr,unsigned char *rootkey,unsigned char *hmac4,unsigned char *hmackey);
unsigned int RPMC_IncCounter(unsigned int cadr,unsigned char *hmackey,unsigned char *input_tag);
unsigned char RPMC_Challenge(unsigned int cadr,unsigned char *hmackey,unsigned char *input_tag);
void RPMC_ReqCounter(unsigned int cadr, unsigned char *hmackey,unsigned char *tag);
INT32 RPMC_ReadJEDECID(PUINT8 data);
INT16 RPMC_ReadUID(PUINT8 data);


