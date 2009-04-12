#include <libc/stdio.h>
#include <libc/malloc.h>
#include <pspgu.h>
#include "dxlibp.h"
#include "dxpstatic.h"
#include "intraFont.h"

/*
シザー領域の設定値をどこかに保存しとかないといけないみたい…gusettingsに追加するかｗ
*/
typedef struct
{
	int dat[3];
}GUCONTEXT_FORSTRING;


static void StringStart(GUCONTEXT_FORSTRING *ptr)
{
	InitString();
	GUSTART
	ptr->dat[0] = sceGuGetAllStatus();
	ptr->dat[1] = gusettings.blendmode;
	ptr->dat[2] = gusettings.blendparam;
	SetTexture(-1,0);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA,255);
	//gusettings.texture = -1;
	//sceGuEnable(GU_BLEND);
	//sceGuBlendFunc(GU_ADD,GU_SRC_ALPHA,GU_ONE_MINUS_SRC_ALPHA,0,0);
	//sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA);
	//printfDx("test");
}																					

static void StringFinish(GUCONTEXT_FORSTRING *ptr)
{
	sceGuSetAllStatus(ptr->dat[0]);
	sceGuScissor(
		gusettings.scissor[0],
		gusettings.scissor[1],
		gusettings.scissor[2],
		gusettings.scissor[3]
	);
	SetDrawBlendMode(ptr->dat[1],ptr->dat[2]);
}
	

//構造体定義
typedef struct DXP_FONTDATA__
{
	struct DXP_FONTDATA__ *next;
	intraFont *ifp;
	float scale;
	unsigned int edgecolor_default;
	int handle;
}DXP_FONTDATA;
//関数宣言
int InitString();
int EndString();
int DrawString( int x, int y, const char *String, int Color, int EdgeColor);
//大域変数定義
static u8 intrafont_init = 0;
static DXP_FONTDATA fontarray = {NULL,NULL,1.0f,0xFF3F3F3F,-1};
//関数定義
static DXP_FONTDATA* FontHandle2Ptr(int handle)
{
	DXP_FONTDATA *ptr = &fontarray;
	for(;ptr != NULL;ptr = ptr->next)
		if(ptr->handle == handle)return ptr;
	return NULL;
}

static DXP_FONTDATA* FontHandle2PrevPtr(int handle)
{
	DXP_FONTDATA *ptr = &fontarray;
	for(;ptr != NULL && ptr->next != NULL;ptr = ptr->next)
		if(ptr->next->handle == handle)return ptr;
	return NULL;
}
int InitString()
{
	if(intrafont_init)return 0;
	if(intraFontInit() != 1)return -1;
	fontarray.ifp = intraFontLoad("flash0:/font/jpn0.pgf",INTRAFONT_STRING_SJIS);
	intrafont_init = 1;
	return 0;
}

int EndString()
{
//	int i;
	DXP_FONTDATA *p = &fontarray,*p2 = NULL;
	if(!intrafont_init)return 0;
	while(p != NULL)
	{
		intraFontUnload(p->ifp);
		p2 = p;
		p = p->next;
		free(p2);
	}
	intrafont_init = 0;
	return 0;
}


int DrawStringWithHandle(int x,int y,const char *String,int color,int handle, int EdgeColor)
{
	GUCONTEXT_FORSTRING gc;
	DXP_FONTDATA *ptr = FontHandle2Ptr(handle);
	if(ptr == NULL)return -1;
	StringStart(&gc);
	intraFontSetStyle(ptr->ifp,ptr->scale,(u32)color,EdgeColor ? (u32)EdgeColor : ptr->edgecolor_default,0);
	intraFontPrint(ptr->ifp,x,y,String);
	StringFinish(&gc);
	return 0;
}
int DrawString(int x,int y,const char *String,int color, int EdgeColor)
{
	return DrawStringWithHandle(x,y,String,color,-1,EdgeColor);
}

int DrawFormatString(int x,int y,int color,const char *String,...)
{
	char str[1024];
	va_list arg;
	va_start(arg,String);
	vsnprintf(str,1024,String,arg);
	va_end(arg);
	return DrawString(x,y,str,color,0);
}

float GetDrawStringWidthWithHandleF(const char *String, int StrLen, int FontHandle)
{
	DXP_FONTDATA *ptr = FontHandle2Ptr(FontHandle);
	if(ptr == NULL)return -1;
	return intraFontMeasureTextEx(ptr->ifp,String,StrLen);
}
int GetDrawStringWidthWithHandle(const char *String,int StrLen,int FontHandle)
{
	return (int)GetDrawStringWidthWithHandleF(String,StrLen,FontHandle);
}
int LoadFont(const char *font,int CharSet)
{
	DXP_FONTDATA *newdata = (DXP_FONTDATA*)malloc(sizeof(DXP_FONTDATA)),*tmp;
	if(newdata == NULL)return -1;
	if((newdata->ifp = intraFontLoad(font,((u32)CharSet) << 16)) == NULL)
	{
		free(newdata);
		return -1;
	}
	newdata->handle = -1;
	newdata->scale = 1.0f;
	newdata->edgecolor_default = 0xFF3F3F3F;
	for(tmp = &fontarray;tmp != NULL;tmp = tmp->next)
		if(newdata->handle <= tmp->handle)newdata->handle = tmp->handle + 1;
	newdata->next = fontarray.next;
	fontarray.next = newdata;
	return newdata->handle;
}


int DeleteFont(int handle)
{
	DXP_FONTDATA *prev = FontHandle2PrevPtr(handle),*ptr;
	if(prev == NULL)return -1;
	ptr = prev->next;
	prev->next = prev->next->next;
	intraFontUnload(ptr->ifp);
	free(ptr);
	return 0;
}

