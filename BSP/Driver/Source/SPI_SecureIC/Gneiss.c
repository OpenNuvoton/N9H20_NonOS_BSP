/**
  ******************************************************************************
  * @file    Gneiss.c
  * @author  Winbond FAE Steam Lin
  * @version V1.1.0
  * @date    09-December-2015
  * @brief   This code provide the low level RPMC hardware operate function based on STM32F205.
  *            
  * COPYRIGHT 2015 Winbond Electronics Corporation.
*/ 

#include "wblib.h"
#include "SecureIC.h"
#include "N9H20_SPI.h"

/* public array use for RPMC algorithm */
unsigned char message[16];     /* Message data use for insturction input */
unsigned char counter[4];      /* counter data (32bit) */
unsigned char tag[12];         /* Tag data use for increase counter */
unsigned char signature[32];   /* Signature data use for every instruction output */

INT32 RPMC_ReadJEDECID(PUINT8 data)
{
    outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) | 0x01);    /* CS0 */

    /* command 8 bit */
    outpw(REG_SPI0_TX0, 0x9F);
    spiTxLen(0, 0, 8);
    spiActive(0);

    /* data 8 bit */
    outpw(REG_SPI0_TX0, 0xff);
    spiTxLen(0, 0, 8);
    spiActive(0);
    *data++ = inpw(REG_SPI0_RX0) & 0xFF;

    /* data 8 bit */
    outpw(REG_SPI0_TX0, 0xff);
    spiTxLen(0, 0, 8);
    spiActive(0);
    *data++ = inpw(REG_SPI0_RX0) & 0xFF;

    /* data 8 bit */
    outpw(REG_SPI0_TX0, 0xff);
    spiTxLen(0, 0, 8);
    spiActive(0);
    *data++ = inpw(REG_SPI0_RX0) & 0xFF;

    outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & 0xfe);    /* CS0 */

    return Successful;
}
INT16 RPMC_ReadUID(PUINT8 data)
{
    int i;
    outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) | 0x01);    /* CS0 */

    /* command 8 bit */
    outpw(REG_SPI0_TX0, 0x4B);
    spiTxLen(0, 0, 8);
    spiActive(0);

    /* Dummy 32 bit */
    outpw(REG_SPI0_TX0, 0x000000);
    spiTxLen(0, 0, 32);
    spiActive(0);

    for(i=0;i< 8;i++)
    {
        /* data 8 bit */
        outpw(REG_SPI0_TX0, 0xff);
        spiTxLen(0, 0, 8);
        spiActive(0);
        *data++ = inpw(REG_SPI0_RX0) & 0xFF;
    }

    outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & 0xfe);    /* CS0 */

    return Successful;
}

/**************************************************************************************************
  Function: /CS goes low
  Argument:
  return:
  date: 2015/8/12
 **************************************************************************************************/
void CS_LOW()
{
    outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) | 0x01);    /* CS0 */
}

/****************************************************************************
  Function: /CS goes high
  Argument:
  return:
  date: 2015/8/12
 **************************************************************************************************/
void CS_HIGH()
{
    outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & 0xfe);    /* CS0 */
}

/**************************************************************************************************
  Function: SPI Input/output
  Argument: DI:
      SPI DI data
  return:    SPI DO data
  date: 2015/8/12
 **************************************************************************************************/
unsigned char SPIin(unsigned char DI)
{
    unsigned char DO;
    outpw(REG_SPI0_TX0, DI);
    spiTxLen(0, 0, 8);
    spiActive(0);    
    DO = inpw(REG_SPI0_RX0) & 0xFF;
    return DO;
}

/**************************************************************************************************
  Function: Read counter
  Argument:
  return:    counter number
  date: 2015/8/12
 **************************************************************************************************/
unsigned int RPMC_ReadCounterData()
{
    return (((((counter[0]*0x100)+counter[1])*0x100)+counter[2])*0x100)+counter[3];
}

/**************************************************************************************************
  Function: RPMC read RPMC status
  Argument: checkall
      checkall = 0: only read out RPMC status
      checkall = 1: Read out counter data, tag, signature information
    
  return: RPMC status 
  date: 2015/8/12
 **************************************************************************************************/
unsigned int RPMC_ReadRPMCstatus(unsigned int checkall)
{
    unsigned int i;
    unsigned char RPMCstatus;
    CS_LOW();
    SPIin(0x96);                       /* read RPMC status command */
    SPIin(0x00);
    RPMCstatus = SPIin(0x00);          /* status was readout, read RPMC status don't need signature input */
    if(checkall ==0)
    {
        CS_HIGH();
    }
    else
    {                                  /* After signature matched reqeust counter instruction, counter data can readout as follow. */
        for(i=0;i<12;i++)
        {
            tag[i]=SPIin(0x00);        /* tag information repeat */
        }
        for(i=0;i<4;i++)
        {
            counter[i] = SPIin(0x00);  /* counter data readout */
        }
        for(i=0;i<32;i++)
        {
            signature[i]=SPIin(0x00);  /* signature repeat */
        }
        CS_HIGH();
      } /* end else */
    return RPMCstatus;
}

/**************************************************************************************************
  Function: RPMC request counter data
  Argument: 
      cadr: selected Counter address, from 1~4
      hmackey: 32 byte HMACKEY which is generated by RPMC_UpHMACkey()
      input_tag: 12 byte input Tag data, which can be time stamp, serial number or random number. 
      these data would repeat after success RPMC_ReqCounter() operation
  return: 
  date: 2015/8/12
 **************************************************************************************************/
void RPMC_ReqCounter(unsigned int cadr, unsigned char *hmackey,unsigned char *input_tag)
{
    unsigned int i;
    message[0]=0x9B;
    message[1]=0x03;
    message[2]=cadr-1;
    message[3]=0x00;
    for(i=0;i<12;i++)
    {
        message[i+4]=*(input_tag+i);
    }
  
    hmacsha256(hmackey,32,message,16,signature);  /* caculate signature by SHA256 */

    CS_LOW();
    for(i=0;i<16;i++)
    {
        SPIin(message[i]);
    }
    for(i=0;i<32;i++)
    {
        SPIin(signature[i]);
    }
    CS_HIGH();
    return;
}

/**************************************************************************************************
  Function: RPMC write rootkey
  Argument: 
      cadr: selected Counter address, from 1~4
      rootkey: 32 byte rootkey infomration.
  return: RPMC status
  date: 2015/8/12
 **************************************************************************************************/
unsigned int RPMC_WrRootKey(unsigned int cadr,unsigned char *rootkey)
{
    unsigned int i;

    memset(message, 0x00, sizeof(message[0])*8);
    message[0]=0x9B;
    message[1]=0x00;
    message[2]=cadr-1;  /* counter address 0~3,so -1 */
    message[3]=0x00; 
  
    hmacsha256(rootkey,32,message,4,signature);    /* caculate signature by SHA256 */
  
    CS_LOW();
    SPIin(message[0]);
    SPIin(message[1]);
    SPIin(message[2]);
    SPIin(message[3]);
    for(i=0;i<32;i++)
    {
        SPIin(*(rootkey+i));
    }
    for(i=0;i<28;i++){
        SPIin(signature[i+4]);
    }
    CS_HIGH();
  
    while(RPMC_ReadRPMCstatus(0)&0x01==0x01)
    {
        /* wait until RPMC operation done */
    }  
    return RPMC_ReadRPMCstatus(0);  
}

/**************************************************************************************************
  Function: RPMC Update HMAC key, this function should call in every Gneiss power on
  Argument: 
      cadr: selected Counter address, from 1~4
      rootkey: rootkey use for generate HMAC key
      hmac4: 4 byte input hmac message data, which can be time stamp, serial number or random number. 
      hmackey: 32 byte HMACKEY, which would be use for increase/request counter after RPMC_UpHMACkey() operation success
  return: 
  date: 2015/8/12
 **************************************************************************************************/
unsigned int RPMC_UpHMACkey(unsigned int cadr,unsigned char *rootkey,unsigned char *hmac4,unsigned char *hmackey)
{
    unsigned int i;
    message[0]=0x9B;
    message[1]=0x01;
    message[2]=cadr - 1;
    message[3]=0x00;
    message[4]=*(hmac4+0);
    message[5]=*(hmac4+1);
    message[6]=*(hmac4+2);
    message[7]=*(hmac4+3);
  
    hmacsha256(rootkey,32,hmac4,4,hmackey);       /* use rootkey generate HMAC key by SHA256 */
    hmacsha256(hmackey,32,message,8,signature);   /* caculate signature by SHA256 */
  
    CS_LOW();
    for(i=0;i<8;i++)
    {
        SPIin(message[i]);
    }
    for(i=0;i<32;i++)
    {
        SPIin(signature[i]);
    }
    CS_HIGH();
    while(RPMC_ReadRPMCstatus(0)&0x01==0x01)
    {
        /* wait until RPMC operation done */
    }  
    return RPMC_ReadRPMCstatus(0); 
}

/**************************************************************************************************
  Function: RPMC request counter data
  Argument: 
      cadr: selected Counter address, from 1~4
      hmackey: 32 byte HMACKEY which is generated by RPMC_UpHMACkey()
      input_tag: 12 byte input Tag data, which can be time stamp, serial number or random number. 
      these data would repeat after success RPMC_ReqCounter() operation
  return: 
  date: 2015/8/12
 **************************************************************************************************/
unsigned int RPMC_IncCounter(unsigned int cadr,unsigned char *hmackey,unsigned char *input_tag)
{
    unsigned int i;

    RPMC_ReqCounter(cadr, hmackey, input_tag);
  
    while(RPMC_ReadRPMCstatus(0)&0x01==0x01)
    {
        /* wait until RPMC operation done */
    }  
    RPMC_ReadRPMCstatus(1);    /* Get counter information */
  
    message[0]=0x9B;
    message[1]=0x02;
    message[2]=cadr-1;
    message[3]=0x00;
    for(i=0;i<4;i++)
    {
        message[i+4]=counter[i];
    }  
  
    hmacsha256(hmackey,32,message,8,signature);    /* caculate signature by SHA256 */
  
    CS_LOW();
    for(i=0;i<8;i++)
    {
        SPIin(message[i]);
    }
    for(i=0;i<32;i++)
    {
        SPIin(signature[i]);
    }
    CS_HIGH();
    while(RPMC_ReadRPMCstatus(0)&0x01==0x01)
    {
        /* wait until RPMC operation done */
    }  
    return RPMC_ReadRPMCstatus(0); 
}

/**************************************************************************************************
  Function: RPMC Challenge signature. Main security operation
  Argument: 
      cadr: selected Counter address, from 1~4
      hmackey: 32 byte HMACKEY which is generated by RPMC_UpHMACkey()
      input_tag: 12 byte input Tag data, which can be time stamp, serial number or random number.
  return: 
      Challlenge result. if signature match, return 0.
  date: 2015/12/09
 **************************************************************************************************/
unsigned char RPMC_Challenge(unsigned int cadr, unsigned char *hmackey,unsigned char *input_tag)
{
    unsigned char Verify_signature[32];    /* signature for verification. should match signature[32] */
    unsigned int i;
    RPMC_ReqCounter(cadr, hmackey, input_tag);
    while(RPMC_ReadRPMCstatus(0)&0x01==0x01)
    {
        /* wait until RPMC operation done */
    }  
    RPMC_ReadRPMCstatus(1);            /* Get counter information. In this stage, tag[12], counter[4], signature[32] is updated.*/
    /* Comment: the message using for signature is tag[0:11]+count[0:3] data, you can also use memcpy to casecade these data */
  
    for(i = 0; i < 12; i++)
    {
        message[i] = tag[i];    /* message [0:11] = tag[0:11] */
    }
    for(i = 0; i < 4; i++)
    {
        message[12+i] = counter[i];    /* message [12:15] = counter[0:3] */
    }
    hmacsha256(hmackey,32,message,16,Verify_signature);    /* Verification signature should as same as security output */
    return memcmp(Verify_signature, signature, 32);        /* Compare Verification signature (computed by controllor) and internal signature (return from security Flash by request counter operation) */
}
