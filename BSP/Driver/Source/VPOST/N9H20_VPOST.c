/**************************************************************************//**
 * @file     N9H20_VPOST.c
 * @version  V3.00
 * @brief    N9H20 series VPOST driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#include "wblib.h"
#include "N9H20_VPOST.h"
#include <stdio.h>

extern INT vpostLCMInit_FW043TFT_480x272(PLCDFORMATEX plcdformatex, UINT32 *pFramebuf);
extern INT vpostLCMInit_FW050TFT_800x480(PLCDFORMATEX plcdformatex, UINT32 *pFramebuf);
extern INT vpostLCMInit_GIANTPLUS_GPM1006D0(PLCDFORMATEX plcdformatex, UINT32 *pFramebuf);
extern INT vpostLCMInit_GOWORLD_GW8973(PLCDFORMATEX plcdformatex, UINT32 *pFramebuf);
extern INT vpostLCMInit_TIANMA_TM022HDH31(PLCDFORMATEX plcdformatex, UINT32 *pFramebuf);

extern INT32 vpostLCMDeinit_FW043TFT_480x272(VOID);
extern INT32 vpostLCMDeinit_FW050TFT_800x480(VOID);
extern INT32 vpostLCMDeinit_GIANTPLUS_GPM1006D0(VOID);
extern INT32 vpostLCMDeinit_GOWORLD_GW8973(VOID);
extern INT32 vpostLCMDeinit_TIANMA_TM022HDH31(VOID);

VOID * g_VAFrameBuf = NULL;
VOID * g_VAOrigFrameBuf = NULL;

INT32 vpostLCMInit(PLCDFORMATEX plcdformatex, UINT32 *pFramebuf)
{

#ifdef __HAVE_SHARP_LQ035Q1DH02__
	return vpostLCMInit_SHARP_LQ035Q1DH02(plcdformatex, pFramebuf);
#endif

#ifdef __HAVE_WINTEK_WMF3324__
	return vpostLCMInit_WINTEK_WMF3324(plcdformatex, pFramebuf);
#endif

#ifdef __HAVE_GIANTPLUS_GPM1006D0__
	return  vpostLCMInit_GIANTPLUS_GPM1006D0(plcdformatex, pFramebuf);
#endif

#ifdef __HAVE_HANNSTAR_HSD043I9W1__
	return vpostLCMInit_HANNSTAR_HSD043I9W1(plcdformatex, pFramebuf);
#endif

#ifdef __HAVE_GOWORLD_GWMTF9406A__
	return vpostLCMInit_GOWORLD_GWMTF9406A(plcdformatex, pFramebuf);
#endif

#ifdef __HAVE_GOWORLD_GWMTF9360A__
	return vpostLCMInit_GOWORLD_GWMTF9360A(plcdformatex, pFramebuf);
#endif

#ifdef __HAVE_GOWORLD_GWMTF9615A__
	return vpostLCMInit_GOWORLD_GWMTF9615A(plcdformatex, pFramebuf);
#endif
#ifdef __HAVE_GOWORLD_GWMTF9360A_MODIFY__
	return vpostLCMInit_GOWORLD_GWMTF9360A_modify(plcdformatex, pFramebuf);
#endif

#ifdef __HAVE_GOWORLD_GW8973__
	return vpostLCMInit_GOWORLD_GW8973(plcdformatex, pFramebuf);
#endif

#ifdef __HAVE_VG680__
	return vpostLCMInit_VG680(plcdformatex, pFramebuf);
#endif

#ifdef __HAVE_TVOUT_720x480__
	return vpostLCMInit_TVOUT_720x480(plcdformatex, pFramebuf);
#endif

#ifdef __HAVE_TVOUT_640x480__
	return vpostLCMInit_TVOUT_640x480(plcdformatex, pFramebuf);
#endif

#ifdef __HAVE_TVOUT_320x240__
	return vpostLCMInit_TVOUT_320x240(plcdformatex, pFramebuf);
#endif

#ifdef __HAVE_AMPIRE_800x600__
	return vpostLCMInit_AMPIRE_800x600(plcdformatex, pFramebuf);
#endif

#ifdef __HAVE_HANNSTAR_HSD070IDW1__
	return vpostLCMInit_HANNSTAR_HSD070IDW1(plcdformatex, pFramebuf);
#endif

#ifdef __HAVE_AMPIRE_800x480__
	return vpostLCMInit_AMPIRE_800x480(plcdformatex, pFramebuf);
#endif

#ifdef __HAVE_GOWORLD_GW07000GNWV__
	return vpostLCMInit_GOWORLD_GW07000GNWV(plcdformatex, pFramebuf);
#endif

#ifdef __HAVE_KD070D10_800x480__
	return vpostLCMInit_KD070D10_800x480(plcdformatex, pFramebuf);
#endif

#ifdef __HAVE_HIMAX_HX8346__
	return vpostLCMInit_HIMAX_HX8346(plcdformatex, pFramebuf);
#endif

#ifdef __HAVE_HIMAX_HX8357__
	return vpostLCMInit_HIMAX_HX8357(plcdformatex, pFramebuf);
#endif

#ifdef __HAVE_ILITEK_ILI9481DS__
	return vpostLCMInit_ILITEK_ILI9481DS(plcdformatex, pFramebuf);
#endif

#ifdef __HAVE_TECHTRON_R61505V__
	return vpostLCMInit_TECHTRON_R61505V(plcdformatex, pFramebuf);
#endif

#ifdef __HAVE_RISETECH_OTA5182A__
	return vpostLCMInit_RISETECH_OTA5182A(plcdformatex, pFramebuf);
#endif

#ifdef __HAVE_RISETECH_OTA5182A_CCIR656__
	return vpostLCMInit_RISETECH_OTA5182A_CCIR656(plcdformatex, pFramebuf);
#endif

#ifdef __HAVE_TOPPOLY_TD028THEA1__
	return vpostLCMInit_TOPPOLY_TD028THEA1(plcdformatex, pFramebuf);
#endif	

#ifdef __HAVE_SITRONIX_ST7567__
	return vpostLCMInit_SITRONIX_ST7567(plcdformatex, pFramebuf);
#endif	

#ifdef __HAVE_TIANMA_TM022HDH31__
	return vpostLCMInit_TIANMA_TM022HDH31(plcdformatex, pFramebuf);
#endif

#ifdef __HAVE_PARISUN_640x480__
    return vpostLCMInit_PARISUN_800x480(plcdformatex, pFramebuf);
#endif

#ifdef __HAVE_FW050TFT_800x480__
    return vpostLCMInit_FW050TFT_800x480(plcdformatex, pFramebuf);
#endif

#ifdef __HAVE_FW043TFT_480x272__
    return vpostLCMInit_FW043TFT_480x272(plcdformatex, pFramebuf);
#endif
}

INT32 vpostLCMDeinit(void)
{
#ifdef __HAVE_SHARP_LQ035Q1DH02__
	return vpostLCMDeinit_SHARP_LQ035Q1DH02();
#endif

#ifdef __HAVE_WINTEK_WMF3324__
	return vpostLCMDeinit_WINTEK_WMF3324();
#endif

#ifdef __HAVE_GIANTPLUS_GPM1006D0__
	return vpostLCMDeinit_GIANTPLUS_GPM1006D0();
#endif

#ifdef __HAVE_HANNSTAR_HSD043I9W1__
	return vpostLCMDeinit_HANNSTAR_HSD043I9W1();
#endif

#ifdef __HAVE_GOWORLD_GW8973__
	return vpostLCMDeinit_GOWORLD_GW8973();
#endif

#ifdef __HAVE_GOWORLD_GWMTF9406A__
	return vpostLCMDeinit_GOWORLD_GWMTF9406A();
#endif

#ifdef __HAVE_GOWORLD_GWMTF9360A__
	return vpostLCMDeinit_GOWORLD_GWMTF9360A();
#endif

#ifdef __HAVE_GOWORLD_GWMTF9615A__
	return vpostLCMDeinit_GOWORLD_GWMTF9615A();
#endif
#ifdef __HAVE_GOWORLD_GWMTF9360A_MODIFY__
	return vpostLCMDeinit_GOWORLD_GWMTF9360A_modify();
#endif

#ifdef __HAVE_VG680__
	return vpostLCMDeinit_VG680();
#endif

#ifdef __HAVE_TVOUT_720x480__
	return vpostLCMDeinit_TVOUT_720x480();
#endif

#ifdef __HAVE_TVOUT_640x480__
	return vpostLCMDeinit_TVOUT_640x480();
#endif

#ifdef __HAVE_TVOUT_320x240__
	return vpostLCMDeinit_TVOUT_320x240();
#endif

#ifdef __HAVE_AMPIRE_800x600__
	return vpostLCMDeinit_AMPIRE_800x600();
#endif

#ifdef __HAVE_AMPIRE_800x480__
	return vpostLCMDeinit_AMPIRE_800x480();
#endif

#ifdef __HAVE_GOWORLD_GW07000GNWV__
	return vpostLCMDeinit_GOWORLD_GW07000GNWV();
#endif

#ifdef __HAVE_KD070D10_800x480__
	return vpostLCMDeinit_KD070D10_800x480();
#endif

#ifdef __HAVE_HANNSTAR_HSD070IDW1__
	return vpostLCMDeinit_HANNSTAR_HSD070IDW1();
#endif

#ifdef __HAVE_HIMAX_HX8346__
	return vpostLCMDeinit_HIMAX_HX8346();
#endif

#ifdef __HAVE_HIMAX_HX8357__
	return vpostLCMDeinit_HIMAX_HX8357();
#endif

#ifdef __HAVE_ILITEK_ILI9481DS__
	return vpostLCMDeinit_ILITEK_ILI9481DS();
#endif

#ifdef __HAVE_TECHTRON_R61505V__
	return vpostLCMDeinit_TECHTRON_R61505V();
#endif

#ifdef __HAVE_RISETECH_OTA5182A__
	return vpostLCMDeinit_RISETECH_OTA5182A();
#endif

#ifdef __HAVE_RISETECH_OTA5182A_CCIR656__
	return vpostLCMDeinit_RISETECH_OTA5182A_CCIR656();
#endif

#ifdef __HAVE_TOPPOLY_TD028THEA1__
	return vpostLCMDeinit_TOPPOLY_TD028THEA1();
#endif	

#ifdef __HAVE_SITRONIX_ST7567__
	return vpostLCMDeinit_SITRONIX_ST7567();
#endif	

#ifdef __HAVE_TIANMA_TM022HDH31__
	return vpostLCMDeinit_TIANMA_TM022HDH31();
#endif

#ifdef __HAVE_PARISUN_640x480__
    return vpostLCMDeinit_PARISUN_800x480();
#endif

#ifdef __HAVE_FW050TFT_800x480__
    return vpostLCMDeinit_FW050TFT_800x480();
#endif

#ifdef __HAVE_FW043TFT_480x272__
    return vpostLCMDeinit_FW043TFT_480x272();
#endif
}

VOID* vpostGetFrameBuffer(void)
{
    return g_VAFrameBuf;
}

VOID vpostSetFrameBuffer(UINT32 pFramebuf)
{ 
	g_VAFrameBuf = (VOID *)pFramebuf;
	g_VAFrameBuf = (VOID*)((UINT32)g_VAFrameBuf | 0x80000000);
    outpw(REG_LCM_FSADDR, (UINT32)pFramebuf);
}


void LCDDelay(unsigned int nCount)
{
	unsigned volatile int i;
		
	for(;nCount!=0;nCount--)
//		for(i=0;i<100;i++);
		for(i=0;i<10;i++);
}
