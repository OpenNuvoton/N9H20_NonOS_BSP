/**************************************************************************//**
 * @file     N9H20_KPI.h
 * @version  V3.00
 * @brief    N9H20 series keypad driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wblib.h"

#define KPI_NONBLOCK	0
#define KPI_BLOCK		1

#define KEY_COUNT		6

#define KEY_UP		1
#define KEY_DOWN	2
#define KEY_LEFT	4
#define KEY_RIGHT	8
#define KEY_ENTER	16
#define KEY_ESC		32

extern void kpi_init(void);
extern int kpi_open(unsigned int src);
extern void kpi_close(void);
extern int kpi_read(unsigned char mode);
