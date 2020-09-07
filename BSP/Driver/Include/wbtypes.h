/**************************************************************************//**
 * @file     wbtypes.h
 * @version  V3.00
 * @brief    N9H20 series SYS driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
 
/****************************************************************************
 * 
 * FILENAME
 *     wbtypes.h
 *
 * VERSION
 *     3.0
 *
 * DESCRIPTION
 *     This file contains PreDefined data types for N9H20 software development
 *
 * DATA STRUCTURES
 *     None
 *
 * FUNCTIONS
 *     None
 *
 * HISTORY
 *     03/11/02		 Ver 1.0 Created by PC30 YCHuang
 *
 * REMARK
 *     None
 **************************************************************************/

#ifndef _WBTYPES_H
#define _WBTYPES_H

/* Nuvoton N9H20 coding standard draft 2.0 */
/* wbtypes.h Release 1.0 */
#ifndef CONST
#define CONST             const
#endif

#ifndef FALSE
#define FALSE             (0)
#endif

#ifndef TRUE
#define TRUE              (1)
#endif

#if defined (__GNUC__) && !(__CC_ARM)
typedef void              VOID;
#else
#define VOID              void
#endif

typedef void *            PVOID;

typedef char              BOOL;
typedef char *            PBOOL;

typedef char              INT8;
typedef char              CHAR;
typedef char *            PINT8;
typedef char *            PCHAR;
typedef unsigned char     UINT8;
typedef unsigned char     UCHAR;
typedef unsigned char *   PUINT8;
typedef unsigned char *   PUCHAR;
typedef char *            PSTR;
typedef const char *      PCSTR;

typedef short             SHORT;
typedef short *           PSHORT;
typedef unsigned short    USHORT;
typedef unsigned short *  PUSHORT;

typedef short             INT16;
typedef short *           PINT16;
typedef unsigned short    UINT16;
typedef unsigned short *  PUINT16;

typedef int               INT;
typedef int *             PINT;
typedef unsigned int      UINT;
typedef unsigned int *    PUINT;

typedef int               INT32;
typedef int *             PINT32;
typedef unsigned int      UINT32;
typedef unsigned int *    PUINT32;

#if defined (__GNUC__) && !(__CC_ARM)
typedef long long           INT64;
typedef unsigned long long  UINT64;
#else
typedef __int64           INT64;    ///< Define 64-bit signed data type
typedef unsigned __int64  UINT64;   ///< Define 64-bit unsigned data type
#endif

typedef float             FLOAT;
typedef float *           PFLOAT;

typedef double            DOUBLE;
typedef double *          PDOUBLE;

typedef int               SIZE_T;

typedef unsigned char     REG8;
typedef unsigned short    REG16;
typedef unsigned int      REG32;

#define ERRCODE		  INT32
#endif /* _WBTYPES_H */

