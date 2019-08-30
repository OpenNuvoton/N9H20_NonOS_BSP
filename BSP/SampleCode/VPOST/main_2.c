/****************************************************************************
 *
 **************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wbio.h"
#include "wbtypes.h"
#include "N9H20.h"


#if defined (__GNUC__) && !(__CC_ARM)
__attribute__ ((aligned (32))) UINT8 Lcm_InitData[]=
#else
__align(32) UINT8 Lcm_InitData[]=
#endif
{
#ifdef __LCD_320x240__	
	#include "export_GPM1006D_320x240.txt"		// generated from LCM Test Tool
#endif
	
#ifdef __LCD_320x480__	
	#include "export_ILI9325_mpu_320x480.txt"		// generated from LCM Test Tool
#endif
	
#ifdef __LCD_480x272__	
	#include "export_FW043TFT_480x272.txt"		// generated from LCM Test Tool
#endif

#ifdef __LCD_800x480__	
	#include "export_FW050TFT_800x480.txt"		// generated from LCM Test Tool
#endif
};

extern UINT8 Vpost_Frame[];

int main_forLcmTestTool(void)
{
	N9HxxLCMInit((UINT32*)Vpost_Frame, (UINT8*)Lcm_InitData);
}
