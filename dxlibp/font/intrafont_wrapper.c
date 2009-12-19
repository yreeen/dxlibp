#include <libc/stdio.h>
#include <libc/malloc.h>
#include <pspgu.h>
#include "intraFont.h"
#include "../graphics.h"
#include <fastmath.h>

#define	DXP_FONT_BASE_SIZE	22.627416997969522f	//SetFontSizeの計算用16x16ドットを1.0fとするため
#define	DXP_FONT_SQRT_SIZE	1.4142135623730951f	//SetFontSizeの計算用

typedef struct
{
	int dat[3];
}GUCONTEXT_FORSTRING;

int InitString();
int EndString();
int SetFontSizeF( float FontSize );

static int gustatus;

static void StringStart(GUCONTEXT_FORSTRING *ptr)
{
	InitString();
	GUSTART;
	if(dxpGraphicsData.drawstate != DXP_DRAWSTATE_INTRAFONT)
	{
		gustatus = sceGuGetAllStatus();
		dxpGraphicsData.drawstate = DXP_DRAWSTATE_INTRAFONT;
		dxpGraphicsData.forceupdate = 1;
	}
	dxpGraphicsData.texture = NULL;
	int op;
	int src,dest;
	unsigned int srcfix;
	unsigned int destfix;	 
	switch(dxpGraphicsData.blendmode)
	{
	case DX_BLENDMODE_NOBLEND:
	case DX_BLENDMODE_ALPHA:
		op = GU_ADD;
		src = GU_SRC_ALPHA;
		dest = GU_ONE_MINUS_SRC_ALPHA;
		srcfix = 0;
		destfix = 0;
		break;
	case DX_BLENDMODE_ADD:
		op = GU_ADD;
		src = GU_SRC_ALPHA;
		dest = GU_FIX;
		srcfix = 0xffffffff;
		destfix = 0xffffffff;
		break;
	case DX_BLENDMODE_SUB:
		op = GU_REVERSE_SUBTRACT;
		src = GU_SRC_ALPHA;
		dest = GU_FIX;
		srcfix = 0xffffffff;
		destfix = 0xffffffff;
		break;
	case DX_BLENDMODE_MUL:
		op = GU_ADD;
		src = GU_DST_COLOR;
		dest = GU_FIX;
		srcfix = 0xffffffff;
		destfix = 0xffffffff;
		break;
	case DX_BLENDMODE_DESTCOLOR:
		op = GU_ADD;
		src = GU_FIX;
		dest = GU_FIX;
		srcfix = 0;
		destfix = 0xffffffff;
		break;
	case DX_BLENDMODE_INVDESTCOLOR:
		op = GU_ADD;
		src = GU_ONE_MINUS_DST_COLOR;
		dest = GU_FIX;
		srcfix = 0;
		destfix = 0;
		break;
	case DX_BLENDMODE_INVSRC:
		op = GU_ADD;
		src = GU_SRC_ALPHA;
		dest = GU_ONE_MINUS_SRC_ALPHA;
		srcfix = 0;
		destfix = 0;
		break;
	default:
		return;
	}
	GUENABLE(GU_BLEND);
	sceGuBlendFunc(op,src,dest,srcfix,destfix);
	unsigned int color = dxpGraphicsData.color;
	switch(dxpGraphicsData.blendmode)
	{
	case DX_BLENDMODE_NOBLEND:
	case DX_BLENDMODE_MUL:
	case DX_BLENDMODE_DESTCOLOR:
		color |= 0xff000000;
		break;
	case DX_BLENDMODE_INVSRC:
		color = (color & 0xff000000) | (~color & 0x00ffffff);
		break;
	}
	sceGuColor(color);
}																					

static void StringFinish(GUCONTEXT_FORSTRING *ptr)
{
	sceGuSetAllStatus(gustatus);
	sceGuScissor(
		dxpGraphicsData.intrafont_scissor[0],
		dxpGraphicsData.intrafont_scissor[1],
		dxpGraphicsData.intrafont_scissor[2],
		dxpGraphicsData.intrafont_scissor[3]
	);
}
	
//構造体定義
typedef struct DXP_FONTDATA__
{
	struct DXP_FONTDATA__ *next;
		intraFont	*ifp;
			float	scale;
	unsigned int	edgecolor_default;
			int		Alignment;
			int		handle;
}DXP_FONTDATA;
int DrawString( int x, int y, const char *String, int Color, int EdgeColor);
//大域変数定義
static u8 intrafont_init = 0;
static DXP_FONTDATA fontarray = {NULL,NULL,1.0f,DXP_COLOR_DARKGRAY,DXP_FONT_ALIGN_DEFAULT,-1};
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


int DrawStringToHandle(int x,int y,const char *String,int color,int handle, int EdgeColor)
{
	GUCONTEXT_FORSTRING gc;
	DXP_FONTDATA *ptr = FontHandle2Ptr(handle);
	if(ptr == NULL)return -1;
	StringStart(&gc);
	intraFontSetStyle(ptr->ifp,ptr->scale,(u32)color,EdgeColor ? (u32)EdgeColor : ptr->edgecolor_default,ptr->Alignment);
	intraFontPrint(ptr->ifp,x,y,String);
	StringFinish(&gc);
	return 0;
}
int DrawString(int x,int y,const char *String,int color, int EdgeColor)
{
	return DrawStringToHandle(x,y,String,color,-1,EdgeColor);
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

int SetFontSize( int FontSize )
{
	if(FontSize < 0) return -1;
	float size;
	if(FontSize == DXP_FONT_DEFAULT_SIZE)
	{
		size = 1.0f;
	}
	else
	{
		size = FontSize*DXP_FONT_SQRT_SIZE/DXP_FONT_BASE_SIZE;
	}
	return SetFontSizeF(size);
}

int SetFontSizeF( float FontSize )
{
	if(FontSize < 0.0f) return -1;
	if(FontSize > 2.0f) return -1;
	DXP_FONTDATA *ptr = FontHandle2Ptr(-1);
	if(ptr == NULL)return -1;
	ptr->scale = FontSize;
	return 0;
}

int SetFontBackgroundColor(int Color)
{
	DXP_FONTDATA *ptr = FontHandle2Ptr(-1);
	if(ptr == NULL)return -1;
	ptr->edgecolor_default = Color;
	return 0;
}

int SetFontAlignment(int Position,int Width)
{
	switch(Position){
	case DXP_FONT_ALIGN_LEFT:
	case DXP_FONT_ALIGN_CENTER:
	case DXP_FONT_ALIGN_RIGHT:
		break;
	case DXP_FONT_WIDTH_FIX:
		if(Width <= 0){
			Width = 1;
		}else if(Width > 255) {
			Width = 255;
		}
		Position |= Width;
		break;
	default:
		Position = DXP_FONT_ALIGN_DEFAULT;
		break;
	}
	DXP_FONTDATA *ptr = FontHandle2Ptr(-1);
	if(ptr == NULL)return -1;
	ptr->Alignment = Position;
	return 0;
}

int	DrawFormatStringToHandle( int x, int y, int color, int FontHandle, const char *fstr, ... )
{
	char str[1024];
	va_list arg;
	va_start(arg,fstr);
	vsnprintf(str,1024,fstr,arg);
	va_end(arg);
	return DrawStringToHandle(x,y,str,color,FontHandle,0);
}

//int	CreateFontToHandle( const char *FontName, int Size, int Thick, int FontType , int CharSet , int EdgeSize , int Italic  , int DataIndex , int ID )
//{
//	return 0;
//}
