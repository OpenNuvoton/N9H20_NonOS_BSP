
#define NAND_PAGE_512B		512
#define NAND_PAGE_2KB		2048
#define NAND_PAGE_4KB		4096

/* F/W update information */
typedef struct fw_update_info_t
{
	UINT16	imageNo;
	UINT16	imageFlag;
	UINT16	startBlock;
	UINT16	endBlock;
	UINT32	executeAddr;
	UINT32	fileLen;
	CHAR	imageName[16];
} FW_UPDATE_INFO_T;

typedef struct fw_nand_image_t
{
	UINT32	actionFlag;
	UINT32	fileLength;
	UINT32	imageNo;
	CHAR	imageName[16];
	UINT32	imageType;
	UINT32	executeAddr;	// endblock
	UINT32	blockNo;		// startblock
	UINT32	dummy;
} FW_NAND_IMAGE_T;

typedef struct fw_nor_image_t
{
	UINT32	actionFlag;
	UINT32	fileLength;
	UINT32	imageNo;
	CHAR	imageName[16];
	UINT32	imageType;
	UINT32	executeAddr;
	UINT32	flashOffset;
	UINT32	endAddr;
} FW_NOR_IMAGE_T;

typedef struct fw_normal_t
{
	UINT32	actionFlag;
	UINT32	length;
	UINT32	address;
} FW_NORMAL_T;

typedef struct fmi_sm_info_t
{
	UINT32	uSectorPerFlash;
	UINT32	uBlockPerFlash;
	UINT32	uPagePerBlock;
	UINT32	uSectorPerBlock;
	UINT32	uLibStartBlock;
	UINT32	nPageSize;
	UINT32	uBadBlockCount;
	BOOL	bIsMulticycle;
	BOOL	bIsMLCNand;
	BOOL	bIsNandECC4;
	BOOL	bIsNandECC8;
	BOOL	bIsNandECC12;
	BOOL	bIsNandECC15;			
	BOOL	bIsCheckECC;
} FMI_SM_INFO_T;

typedef struct fmi_sd_info_t
{
	UINT32	totalSectorN;
	UINT32	uCardType;		// sd2.0, sd1.1, or mmc
	UINT32	uRCA;			// relative card address
	BOOL	bIsCardInsert;
} FMI_SD_INFO_T;

typedef struct nvt_pt_rec		/* partition record */
{
	UINT8       ucState;			/* Current state of partition */
	UINT8       uStartHead;			/* Beginning of partition head */
	UINT8       ucStartSector;		/* Beginning of partition sector */
	UINT8       ucStartCylinder;	/* Beginning of partition cylinder */
	UINT8       ucPartitionType;	/* Partition type, refer to the subsequent definitions */
	UINT8       ucEndHead;			/* End of partition - head */
	UINT8       ucEndSector;		/* End of partition - sector */
	UINT8       ucEndCylinder;		/* End of partition - cylinder */
	UINT32      uFirstSec;			/* Number of Sectors Between the MBR and the First Sector in the Partition */
	UINT32		uNumOfSecs;			/* Number of Sectors in the Partition */
}	NVT_PT_REC_T;

/*****************************/
extern FMI_SD_INFO_T *pSD;
extern FMI_SM_INFO_T *pSM0, *pSM1;

extern BOOL bIsMatch;
extern UINT32 *pUpdateImage;
extern UINT8 *pInfo;
extern UINT32 infoBuf;
extern INT8 nIsSysImage;
extern UINT32 volatile gRxLen;
extern UINT8 *gRxPtr;
extern INT8 gCurType;
extern INT32 gCurBlock, gCurPage;
extern UINT32 NandBlockSize;

/* extern flags */
extern UINT8 g_u8BulkState;
extern BOOL udcOnLine;
extern BOOL g_bIsFirstRead;
extern UINT32 usbDataReady;
extern UINT8 g_u8UsbState;
extern UINT8 g_u8Address;
extern UINT8 g_u8Config;
extern UINT8 g_u8Flag;
extern UINT8 g_au8SenseKey[];
extern UINT32 g_u32Address;
extern UINT32 g_u32Length;
extern UINT32 g_u32OutToggle;	// for Error Detection
extern UINT8 g_u8Size;
extern BOOL g_bCBWInvalid;
extern struct CBW g_sCBW;
extern struct CSW g_sCSW;
extern BOOL volatile bIsSdInit;



/*****************************/
INT		sicSMpread(INT chipSel, INT PBA, INT page, UINT8 *buff);
INT		sicSMpwrite(INT PBA, INT page, UINT8 *buff);
INT		sicSMblock_erase(INT PBA);
INT		fmiMarkBadBlock(UINT32 block);
INT		ChangeNandImageType(UINT32 imageNo, UINT32 imageType);
INT		sicSMchip_erase(UINT32 startBlcok, UINT32 endBlock);
INT		DelNandImage(UINT32 imageNo);
VOID	fmiInitDevice(void);
INT		sicSMInit(void);
INT		nvtSetNandImageInfo(FW_UPDATE_INFO_T *pFW);
INT		searchImageInfo(FW_UPDATE_INFO_T *pFW, UINT32 *ptr);
INT		nvtGetNandImageInfo(unsigned int *image);
INT		CheckBadBlockMark(UINT32 block);
INT		CheckBadBlockMark_512(UINT32 block);
INT		fmiInitSDDevice(void);
INT		SdChipErase(void);
INT		ChangeSdImageType(UINT32 imageNo, UINT32 imageType);
INT		DelSdImage(UINT32 imageNo);
INT		nvtGetSdImageInfo(unsigned int *image);
INT		fmiSD_Write(UINT32 uSector, UINT32 uBufcnt, UINT32 uSAddr);
INT		fmiSD_Read(UINT32 uSector, UINT32 uBufcnt, UINT32 uDAddr);
INT		nvtSetSdImageInfo(FW_UPDATE_INFO_T *pFW);
void	nvtSdFormat(UINT32 firstSector, UINT32 totalSector);
/*****************************/
void 		apuDacOn(UINT8 level);







