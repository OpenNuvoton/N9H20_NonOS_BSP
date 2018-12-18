/****************************************************************************
 * @file     writer.h
 * @version  V1.00
 * $Revision: 4 $
 * $Date: 18/04/25 11:43a $
 * @brief    SpiWriter header file
 *
 * @note
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#define	MAJOR_VERSION_NUM  1
#define	MINOR_VERSION_NUM  4

//#define __DEBUG__

#ifdef __DEBUG__
  #define PRINTF sysprintf
#else
  #define PRINTF(...)
#endif

extern UINT	g_Font_Height, g_Font_Width, g_Font_Step;

#define	COLOR_RGB16_RED      0xF800
#define	COLOR_RGB16_WHITE    0xFFFF
#define	COLOR_RGB16_GREEN    0x07E0
#define	COLOR_RGB16_BLACK    0x0000

#define NVT_SM_INFO_T FMI_SM_INFO_T

#define START_BURN           0
#define BURN_PASS            1
#define BURN_FAIL            2

/* F/W update information */
typedef struct fw_update_info_t
{
    UINT16  imageNo;
    UINT16  imageFlag;
    UINT16  startBlock;
    UINT16  endBlock;
    UINT32  executeAddr;
    UINT32  fileLen;
    CHAR  imageName[32];
} FW_UPDATE_INFO_T;

typedef struct IBR_boot_struct_t
{
    UINT32  BootCodeMarker;
    UINT32  ExeAddr;
    UINT32  ImageSize;
    UINT32  SkewMarker;
    UINT32  DQSODS;
    UINT32  CLKQSDS;
    UINT32  DRAMMarker;
    UINT32  DRAMSize;
} IBR_BOOT_STRUCT_T;

typedef struct IBR_boot_struct_t_new
{
    UINT32  BootCodeMarker;
    UINT32  ExeAddr;
    UINT32  ImageSize;
    UINT32  Reserved;
} IBR_BOOT_STRUCT_T_NEW;



typedef struct USER_IMAGE_Info {
    char  FileName[1024];
    int  StartBlock;
    int  address;
} INI_USER_IMAGE_T;

/* Boot Code Optional Setting */
typedef struct IBR_boot_optional_pairs_struct_t
{
    UINT32  address;
    UINT32  value;
} IBR_BOOT_OPTIONAL_PAIRS_STRUCT_T;

#define IBR_BOOT_CODE_OPTIONAL_MARKER       0xAA55AA55
#define IBR_BOOT_CODE_OPTIONAL_MAX_NUMBER   63  /* support maximum 63 pairs optional setting */
typedef struct IBR_boot_optional_struct_t
{
    UINT32  OptionalMarker;
    UINT32  Counter;
    IBR_BOOT_OPTIONAL_PAIRS_STRUCT_T Pairs[IBR_BOOT_CODE_OPTIONAL_MAX_NUMBER];
} IBR_BOOT_OPTIONAL_STRUCT_T;



typedef struct INI_Info {
    INI_USER_IMAGE_T RawData;
    INI_USER_IMAGE_T SpiLoader;
    INI_USER_IMAGE_T Logo;
    INI_USER_IMAGE_T Execute;
    INI_USER_IMAGE_T UserImage[22];
    int ChipErase;
    int FourByteAddressMode;
    int DataVerify;
    int SoundPlay;
    int SoundPlayVolume;
    INI_USER_IMAGE_T SoundPlayStart;
    INI_USER_IMAGE_T SoundPlayFail;
    INI_USER_IMAGE_T SoundPlayPass;
    int RootKey;
} INI_INFO_T;

#define SPIFLASH_WINBOND      0xEF
#define SPIFLASH_SST          0xBF
#define SPIFLASH_MXIC         0xC2
#define SPIFLASH_GIGADEVICE   0xC8

/* extern parameters */
extern UINT32 infoBuf;
extern UINT8 *pInfo;
extern UINT32 volatile g_SPI_SIZE;
void Draw_Font(UINT16 u16RGB,S_DEMO_FONT* ptFont,UINT32	u32x,UINT32 u32y,PCSTR	pszString);
void Draw_Status(UINT32	u32x,UINT32	u32y,int Status);
void Draw_Clear(int xStart, int yStart, int xEnd, int yEnd, UINT16 color);
void Draw_CurrentOperation(PCSTR pszString, int Retcode);
void Draw_FinalStatus(int bIsAbort);
void Draw_Init(void);
void Draw_Wait_Status(UINT32 u32x, UINT32 u32y);
void Draw_Clear_Wait_Status(UINT32 u32x, UINT32 u32y);

int sound_init(UINT32 SoundPlayVolume);
int sound_play(UINT32 u32select,UINT32 u32Buffer,UINT32 u32Size);
int ProcessINI(char *fileName);
int ProcessOptionalINI(char *fileName);
extern UINT32 u32GpioPort_Start, u32GpioPort_Pass, u32GpioPort_Fail;
extern UINT32 u32GpioPin_Start, u32GpioPin_Pass, u32GpioPin_Fail;
extern UINT32 u32GpioLevel_Start, u32GpioLevel_Pass, u32GpioLevel_Fail;
extern UINT32 u32ImageCount, u32UserImageCount;
extern UINT8 volatile  SPI_ID[];

void SpiReset(void);
void Enter4ByteMode(void);
int usiInit(void);
int usiEraseSector(UINT32 addr, UINT32 secCount);
int usiWaitEraseReady(void);
int usiEraseAll(void);
int usiWrite(UINT32 addr, UINT32 len, UINT8 *buf);
int usiRead(UINT32 addr, UINT32 len, UINT8 *buf);
ERRCODE DrvSPU_SetPauseAddress_PCM16(UINT32 u32Channel, UINT32 u32Address);
