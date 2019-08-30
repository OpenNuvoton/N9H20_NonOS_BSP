/****************************************************************************
 * @file     mass_storage_class.h
 * @version  V1.00
 * $Revision: 4 $
 * $Date: 18/04/25 11:43a $
 * @brief    Mass Storage Device driver header file
 *
 * @note
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#include "N9H20_GNAND.h"
#include "wblib.h"

typedef void (*PFN_MSCD_CDROM_CALLBACK)(PUINT32 pu32address, UINT32 u32Offset, UINT32 u32LengthInByte);

/* extern functions */
void mscdInit(VOID);
void mscdDeinit(VOID);
UINT8 mscdFlashInit(NDISK_T *pDisk, INT SDsector);
UINT8 mscdFlashInitCDROM(NDISK_T *pDisk, INT SDsector, PFN_MSCD_CDROM_CALLBACK pfnCallBack ,INT CdromSizeInByte);
void mscdMassEvent(PFN_USBD_EXIT_CALLBACK* callback_func);
void mscdSdPortSelect(UINT32 u32Port);
void mscdBlcokModeEnable(BOOL bEnable);

