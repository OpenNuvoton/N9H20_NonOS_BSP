/**************************************************************************//**
 * @file     mscd.h
 * @version  V3.00
 * @brief    N9H20 series Mass Storage Device driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include "wblib.h"

/* MSC Class Command */
#define BULK_ONLY_MASS_STORAGE_RESET      0xFF
#define GET_MAX_LUN                       0xFE

/* UFI Command */
#define UFI_TEST_UNIT_READY               0x00
#define UFI_REQUEST_SENSE                 0x03
#define UFI_INQUIRY                       0x12
#define UFI_MODE_SELECT_6                 0x15
#define UFI_MODE_SENSE_6                  0x1A
#define UFI_START_STOP                    0x1B
#define UFI_PREVENT_ALLOW_MEDIUM_REMOVAL  0x1E
#define UFI_READ_FORMAT_CAPACITY          0x23
#define UFI_READ_CAPACITY                 0x25
#define UFI_READ_10                       0x28
#define UFI_READ_12                       0xA8
#define UFI_WRITE_10                      0x2A
#define UFI_WRITE_12                      0xAA
#define UFI_VERIFY_10                     0x2F
#define UFI_MODE_SELECT_10                0x55
#define UFI_MODE_SENSE_10                 0x5A
#define CDROM_COMMAND_43                  0x43
#define CDROM_COMMAND_46                  0x46
#define CDROM_COMMAND_51                  0x51
#define CDROM_COMMAND_4A                  0x4A
#define CDROM_COMMAND_A4                  0xA4

/* length of descriptors */
#define MSC_DEVICE_DSCPT_LEN              0x12
#define MSC_CONFIG_DSCPT_LEN              0x20
#define MSC_STR0_DSCPT_LEN                0x04
#define MSC_STR1_DSCPT_LEN                0x10
#define MSC_STR2_DSCPT_LEN                0x10
#define MSC_STR3_DSCPT_LEN                0x1a

#define MSC_QUALIFIER_DSCPT_LEN           0x0a
#define MSC_OSCONFIG_DSCPT_LEN            0x20

/* for mass storage */
/* flash format */
#define RAMDISK_1M        1
#define RAMDISK_4M        2
#define RAMDISK_8M        3
#define RAMDISK_16M       4
#define RAMDISK_32M       5
#define RAMDISK_64M       6
#define RAMDISK_128M      7

/* Identifier Language */
#define LANGID_Afrikaans                 0x04360304
#define LANGID_Albanian                  0x041c0304
#define LANGID_Arabic_Saudi_Arabia       0x04010304
#define LANGID_Arabic_Iraq               0x08010304
#define LANGID_Arabic_Egypt              0x0c010304
#define LANGID_Arabic_Libya              0x10010304
#define LANGID_Arabic_Algeria            0x14010304
#define LANGID_Arabic_Morocco            0x18010304
#define LANGID_Arabic_Tunisia            0x1c010304
#define LANGID_Arabic_Oman               0x20010304
#define LANGID_Arabic_Yemen              0x24010304
#define LANGID_Arabic_Syria              0x28010304
#define LANGID_Arabic_Jordan             0x2c010304
#define LANGID_Arabic_Lebanon            0x30010304
#define LANGID_Arabic_Kuwait             0x34010304
#define LANGID_Arabic_UAE                0x38010304
#define LANGID_Arabic_Bahrain            0x3c010304
#define LANGID_Arabic_Qatar              0x40010304
#define LANGID_Armenian                  0x042b0304
#define LANGID_Assamese                  0x044d0304
#define LANGID_Azeri_Latin               0x042c0304
#define LANGID_Azeri_Cyrillic            0x082c0304
#define LANGID_Basque                    0x042d0304
#define LANGID_Belarussian               0x04230304
#define LANGID_Bengali                   0x04450304
#define LANGID_Bulgarian                 0x04020304
#define LANGID_Burmese                   0x04550304
#define LANGID_Catalan                   0x04030304
#define LANGID_Chinese_Taiwan            0x04040304
#define LANGID_Chinese_PRC               0x08040304
#define LANGID_Chinese_HongKong_SAR      0x0c040304
#define LANGID_Chinese_Singapore         0x10040304
#define LANGID_Chinese_Macau_SAR         0x14040304
#define LANGID_Croatian                  0x041a0304
#define LANGID_Czech                     0x04050304
#define LANGID_Danish                    0x04060304
#define LANGID_Dutch_Netherlands         0x04130304
#define LANGID_Dutch_Belgium             0x08130304
#define LANGID_English_UnitedStates      0x04090304
#define LANGID_English_UnitedKingdom     0x08090304
#define LANGID_English_Australian        0x0c090304
#define LANGID_English_Canadian          0x10090304
#define LANGID_English_NewZealand        0x14090304
#define LANGID_English_Ireland           0x18090304
#define LANGID_English_SouthAfrica       0x1c090304
#define LANGID_English_Jamaica           0x20090304
#define LANGID_English_Caribbean         0x24090304
#define LANGID_English_Belize            0x28090304
#define LANGID_English_Trinidad          0x2c090304
#define LANGID_English_Zimbabwe          0x30090304
#define LANGID_English_Philippines       0x34090304
#define LANGID_Estonian                  0x04250304
#define LANGID_Faeroese                  0x04380304
#define LANGID_Farsi                     0x04290304
#define LANGID_Finnish                   0x040b0304
#define LANGID_French_Standard           0x040c0304
#define LANGID_French_Belgian            0x080c0304
#define LANGID_French_Canadian           0x0c0c0304
#define LANGID_French_Switzerland        0x100c0304
#define LANGID_French_Luxembourg         0x140c0304
#define LANGID_French_Monaco             0x180c0304
#define LANGID_Georgian                  0x04370304
#define LANGID_German_Standard           0x04070304
#define LANGID_German_Switzerland        0x08070304
#define LANGID_German_Austria            0x0c070304
#define LANGID_German_Luxembourg         0x10070304
#define LANGID_German_Liechtenstein      0x14070304
#define LANGID_Greek                     0x04080304
#define LANGID_Gujarati                  0x04470304
#define LANGID_Hebrew                    0x040d0304
#define LANGID_Hindi                     0x04390304
#define LANGID_Hungarian                 0x040e0304
#define LANGID_Icelandic                 0x040f0304
#define LANGID_Indonesian                0x04210304
#define LANGID_Italian_Standard          0x04100304
#define LANGID_Italian_Switzerland       0x08100304
#define LANGID_Japanese                  0x04110304
#define LANGID_Kannada                   0x044b0304
#define LANGID_Kashmiri_India            0x08600304
#define LANGID_Kazakh                    0x043f0304
#define LANGID_Konkani                   0x04570304
#define LANGID_Korean                    0x04120304
#define LANGID_Korean_Johab              0x08120304
#define LANGID_Latvian                   0x04260304
#define LANGID_Lithuanian                0x04270304
#define LANGID_Lithuanian_Classic        0x08270304
#define LANGID_Macedonian                0x042f0304
#define LANGID_Malay_Malaysian           0x043e0304
#define LANGID_Malay_BruneiDarussalam    0x083e0304
#define LANGID_Malayalam                 0x044c0304
#define LANGID_Manipuri                  0x04580304
#define LANGID_Marathi                   0x044e0304
#define LANGID_Nepali_India              0x08610304
#define LANGID_Norwegian_Bokmal          0x04140304
#define LANGID_Norwegian_Nynorsk         0x08140304
#define LANGID_Oriya                     0x04480304
#define LANGID_Polish                    0x04150304
#define LANGID_Portuguese_Brazil         0x04160304
#define LANGID_Portuguese_Standard       0x08160304
#define LANGID_Punjabi                   0x04460304
#define LANGID_Romanian                  0x04180304
#define LANGID_Russian                   0x04190304
#define LANGID_Sanskrit                  0x044f0304
#define LANGID_Serbian_Cyrillic          0x0c1a0304
#define LANGID_Serbian_Latin             0x081a0304
#define LANGID_Sindhi                    0x04590304
#define LANGID_Slovak                    0x041b0304
#define LANGID_Slovenian                 0x04240304
#define LANGID_Spanish_TraditionalSort   0x040a0304
#define LANGID_Spanish_Mexican           0x080a0304
#define LANGID_Spanish_ModernSort        0x0c0a0304
#define LANGID_Spanish_Guatemala         0x100a0304
#define LANGID_Spanish_CostaRica         0x140a0304
#define LANGID_Spanish_Panama            0x180a0304
#define LANGID_Spanish_DominicanRepublic 0x1c0a0304
#define LANGID_Spanish_Venezuela         0x200a0304
#define LANGID_Spanish_Colombia          0x240a0304
#define LANGID_Spanish_Peru              0x280a0304
#define LANGID_Spanish_Argentina         0x2c0a0304
#define LANGID_Spanish_Ecuador           0x300a0304
#define LANGID_Spanish_Chile             0x340a0304
#define LANGID_Spanish_Uruguay           0x380a0304
#define LANGID_Spanish_Paraguay          0x3c0a0304
#define LANGID_Spanish_Bolivia           0x400a0304
#define LANGID_Spanish_ElSalvador        0x440a0304
#define LANGID_Spanish_Honduras          0x480a0304
#define LANGID_Spanish_Nicaragua         0x4c0a0304
#define LANGID_Spanish_PuertoRico        0x500a0304
#define LANGID_Sutu                      0x04300304
#define LANGID_Swahili_Kenya             0x04410304
#define LANGID_Swedish                   0x041d0304
#define LANGID_Swedish_Finland           0x081d0304
#define LANGID_Tamil                     0x04490304
#define LANGID_Tatar_Tatarstan           0x04440304
#define LANGID_Telugu                    0x044a0304
#define LANGID_Thai                      0x041e0304
#define LANGID_Turkish                   0x041f0304
#define LANGID_Ukrainian                 0x04220304
#define LANGID_Urdu_Pakistan             0x04200304
#define LANGID_Urdu_India                0x08200304
#define LANGID_Uzbek_Latin               0x04430304
#define LANGID_Uzbek_Cyrillic            0x08430304


typedef struct Disk_Par_Inf {
    UINT32  partition_size,
    data_location,
    bpb_location,
    fat_location,
    rootdir_location,
    free_size;
    UINT16  rootdirentryno,
    totalcluster,
    NumCyl;
    UINT8  NumHead,
    NumSector,
    capacity,
    fatcopyno,
    fatsectorno,
    fat16flg;
} Disk_Par_Info;



typedef struct{
    UINT32  Mass_Base_Addr;
    UINT32  Storage_Base_Addr;
    UINT32  Storage_Base_Addr_RAMDISK;
    UINT32  SizePerSector;
    UINT32  _usbd_DMA_Dir;
    UINT32  _usbd_Less_MPS;
    UINT32  USBD_DMA_LEN;
    UINT32  bulkonlycmd;
    UINT32  dwUsedBytes;
    UINT32  dwResidueLen;
    UINT32  dwCBW_flag;
    UINT32  TxedFlag;
    INT32  gTotalSectors_RAM;
    INT32  gTotalSectors_SD;
    INT32  gTotalSectors_SM;
    INT32  gTotalSectors_SPI;
    UINT32  Card_STS;
    UINT32  preventflag;
    UINT32  Bulk_First_Flag;
    UINT32  card_remove_flag;
    UINT32  card_Insert_flag;
    UINT32  no_card_flag;
    UINT32  MB_Invalid_Cmd_Flg;
    Disk_Par_Info DDB;
    UINT8  SenseKey ;
    UINT8  ASC;
    UINT8  ASCQ;
    UINT32  Mass_LUN;
    UINT32  F_RAM_LUN;
    UINT32  F_SD_LUN;
    UINT32  F_SM_LUN;
    UINT32  F_SPI_LUN;
    UINT32 F_CDROM_LUN;
    UINT32 CD_Tracks_Base;
    INT32  gTotalSectors_CDROM;
}MSC_INFO_T;
