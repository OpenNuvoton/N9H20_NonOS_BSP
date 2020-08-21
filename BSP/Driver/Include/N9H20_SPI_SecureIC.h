/**
  ******************************************************************************
  * @file    N9H20_SPI_SecureIC.h
  * @author  Winbond FAE YY Huang, FAE Steam Lin
  * @version V1.0.0
  * @date    09-December-2015
  * @brief   This code provide the Demo code for RPMC operation. Please do not copy the rootkey generate method directly.
		     Rootkey generate method should be keep in secret and should not exposed.		 
			 
  *            
  * COPYRIGHT 2015 Winbond Electronics Corporation.
*/
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


