/**************************************************************************//**
 * @file     N9H20_MSC.h
 * @version  V3.00
 * @brief    N9H20 series MSC driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
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

