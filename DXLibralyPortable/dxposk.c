#include "dxlibp.h"
#include <psputility.h>
#include <pspgu.h>
#include "dxpstatic.h"
/*
*	DXライブラリPortable	OSK管理部	Ver1.00
*	製作者	：夢瑞憂煉
*
*	備考	：とくになし
*/
#include <malloc.h>
#include <string.h>
/*帰ってくる文字コードがSHIFT-JISじゃない（UTF-8？）ので解析しないと…*/
/*返ってくる文字コードがUTF-16だと判明。iconvで変換を試みるもリンクがうまくいかないｗ　自前の変換コードでも書くか…*/

int GetTextOSK(char *buf,int buflen,int inputmode,const char *title,const char *init)
{
	if(buf == NULL)return -1;

	u16 *winit,*wtitle,*wresult;
	char nulstr[] = "";
	if(init == NULL)init = nulstr;
	if(title == NULL)title = nulstr;
	winit = malloc((strlen(init) + 1) * 2);
	wtitle = malloc((strlen(title) + 1) * 2);
	wresult = malloc((buflen + 1) * 2);
	if(winit == NULL || wtitle == NULL || wresult == NULL)
	{
		free(winit);
		free(wtitle);
		free(wresult);
		return -1;
	}
	sjisstr2unicodestr(init,winit);
	sjisstr2unicodestr(title,wtitle);

	SceUtilityOskData data;
	memset(&data, 0, sizeof(SceUtilityOskData));
	sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &data.language);
	data.unk_00 = 1;	//漢字
	data.lines = 1;
	data.unk_24 = 1;
	data.inputtype = inputmode; // Allow all input types
	data.desc = wtitle;
	data.intext = winit;
	data.outtextlength = buflen;
	data.outtextlimit = buflen; // Limit input to 32 characters
	data.outtext = wresult;

	SceUtilityOskParams params;
	memset(&params,0x00,sizeof(params));
	params.base.size = sizeof(params);
	sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE,&params.base.language);
	sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN,&params.base.buttonSwap);
	params.base.graphicsThread = 0x11;
	params.base.accessThread = 0x13;
	params.base.fontThread = 0x12;
	params.base.soundThread = 0x10;
	params.datacount = 1;
	params.data = &data;

	sceUtilityOskInitStart(&params);

	int done = 0;
	while(ProcessMessage() != -1 && !done)
	{
		ClearDrawScreen();
		GUSYNC
		switch(sceUtilityOskGetStatus())
		{
			case PSP_UTILITY_DIALOG_INIT:
				break;
			
			case PSP_UTILITY_DIALOG_VISIBLE:
				sceUtilityOskUpdate(1);
				break;
			
			case PSP_UTILITY_DIALOG_QUIT:
				sceUtilityOskShutdownStart();
				break;
			
			case PSP_UTILITY_DIALOG_FINISHED:
				break;
				
			case PSP_UTILITY_DIALOG_NONE:
				done = 1;
				
			default :
				break;
		}
		ScreenFlip();
	}

	unicodestr2sjisstr(wresult,buf);

	free(winit);
	free(wtitle);
	free(wresult);
	
	return 0;
}
