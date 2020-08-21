/**************************************************************************//**
 * @file     nau8822.h
 * @version  V3.00
 * @brief    NAU8822 codec header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

void NAU8822_DACSetup(void);
void NAU8822_ADCSetup(void);
void NAU8822_ADC_DAC_Setup(void);
void NAU8822_Init(void);
int  NAU8822_SetPlayVolume(unsigned int ucVolL, unsigned int ucVolR);
int  NAU8822_SetRecVolume(unsigned int ucVolL, unsigned int ucVolR);
void NAU8822_SetSampleFreq(int uSample_Rate);
