/*
*	DXライブラリPortable	VRAM管理部	Ver1.10
*	製作者	：夢瑞憂煉
*
*	備考	：必ず16バイト境界に沿ったデータを返すようになっています。
*	占有メモリの目安はおよそ130KByteです。
*	Ver2.00あたりでVRAMのデフラグに対応させる予定です。
*/

#include <pspgu.h>
#include <pspdisplay.h>
#include "dxlibp.h"
#include "dxpstatic.h"

//#define HOGE

#define VRAMARRAYNUM	(2*1024)			/*確保できるVRAM領域の最大数（≒テクスチャの最大数）*/

static	u8			vramdata[2*1024*64 + 1];	/*VRAMの使われている場所を記憶する16byte単位。最後のは番兵*/
static	DXPVRAMCTRL	vramarray[VRAMARRAYNUM + 1];/*確保済みVRAMの情報保持*/
static	u32			vramfree;				/*あとどれくらい空いているのか*/

void InitVRAM();
DXPVRAMCTRL* AllocVRAM(int Size,int StaticFlag);
void FreeVRAM(DXPVRAMCTRL *VRamCtrl);
//static void DeflagVRAM(){}

void InitVRAM()
{
	AppLogAdd2("VRAMの管理データを初期化します。");

	int i;
	for(i = 0;i < 2 * 1024 * 64;++i)vramdata[i] = 0x00;
//	for(i = 0;i < 34816;++i)vramdata[i] = 0x01;

//	vramdata[0] = 0x01;
	vramdata[2 * 1024 * 64] = 0x01;
	for(i = 0;i < VRAMARRAYNUM;++i)
	{
		vramarray[i].address= 0;
		vramarray[i].flags	= 0;
		vramarray[i].size	= 0;
	}
	vramarray[VRAMARRAYNUM].flags = VRAMCTRLFLAGS_EXIST;
	vramfree = 2*1024*64;
	AppLogAdd2("VRAMの管理データを初期化しました。");
}
#ifdef	HOGE
DXPVRAMCTRL* AllocVRAM(int Size,int StaticFlag)
/*静的フラグが立っている場合は逆方向から検索する。若干の改良（？）*/
{
	int i,j,k,df;
	if(Size > vramfree << 4)return NULL;
	if(Size <= 0)return NULL;
	if(Size % 16)Size += 16 - Size % 16;/*確保するサイズが16で割り切れない場合は補正*/
	df = 0;

	if(!StaticFlag)
	{
		i = 0;
FREEAREASERCH:
		while(vramdata[i])++i;	/*使われていない場所を検索*/
		if(i >= 2 * 1024 * 64)return NULL;/*番兵にぶつかったか、みょんな力が働いたかのどちらか*/
		for(j = 0;i + j < 2 * 1024 * 64 && j < (Size >> 4);++j);/*連続して空いているのか確認*/
		if(i + j >= 2 * 1024 * 64)return NULL;
		if(j < (Size >> 4))goto FREEAREASERCH;/*連続してなかったら検索しなおす*/
	}
	else
	{
		i = 2 * 1024 * 64;
FREEAREASERCH_STATIC:
		while(vramdata[i])
		{
			--i;	/*使われていない場所を検索*/
			if(i < 0)
			{
				return NULL;
			}
		}
		for(j = 0;i - j >= 0 && j < (Size >> 4);++j);/*連続して空いているのか確認*/
		if(i - j < 0)return NULL;
		if(j < (Size >> 4))goto FREEAREASERCH_STATIC;/*連続してなかったら検索しなおす*/
		i = i + 1 - j;	/*iを場所の先頭にもってくる*/
	}
/*ここまでで空き領域検索終了。データ検索に入ります*/
	j = 0;
	while(vramarray[j].flags & VRAMCTRLFLAGS_EXIST)++j;
	if(j >= VRAMARRAYNUM)return NULL;
/*検索終了♪*/
	for(k = 0;k < (Size >> 4);++k)vramdata[i + k] = 0x01;
	vramarray[j].offset = i * 16;/*16バイト境界前提なのでｗ*/
	vramarray[j].size	= Size;
	vramarray[j].flags	= VRAMCTRLFLAGS_EXIST |
						(StaticFlag ? VRAMCTRLFLAGS_STATIC : 0)
						;
	if(StaticFlag)
	{
		AppLogAdd2("静的なVRAM領域が確保されました。");
		AppLogAdd2("オフセット：%#x\tサイズ：%d",i * 16,Size);
	}
	return vramarray + j;
}
#else
DXPVRAMCTRL* AllocVRAM(int Size,int StaticFlag)
/*静的フラグが立っている場合は逆方向から検索する。若干の改良（？）*/
{
	int i,j,k,df;
	if(Size > vramfree << 4)return NULL;
	if(Size <= 0)return NULL;
	if(Size % 16)Size += 16 - Size % 16;/*確保するサイズが16で割り切れない場合は補正*/
	df = 0;

	{
		i = 0;
FREEAREASERCH:
		while(vramdata[i])++i;	/*使われていない場所を検索*/
		if(i >= 2 * 1024 * 64)return NULL;/*番兵にぶつかったか、みょんな力が働いたかのどちらか*/
		for(j = 0;i + j < 2 * 1024 * 64 && j < (Size >> 4);++j);/*連続して空いているのか確認*/
		if(i + j >= 2 * 1024 * 64)return NULL;
		if(j < (Size >> 4))goto FREEAREASERCH;/*連続してなかったら検索しなおす*/
	}
/*ここまでで空き領域検索終了。データ検索に入ります*/
	j = 0;
	while(vramarray[j].flags & VRAMCTRLFLAGS_EXIST)++j;
	if(j >= VRAMARRAYNUM)return NULL;
/*検索終了♪*/
	for(k = 0;k < (Size >> 4);++k)vramdata[i + k] = 0x01;
	vramarray[j].offset = i * 16;/*16バイト境界前提なのでｗ*/
	vramarray[j].size	= Size;
	vramarray[j].flags	= VRAMCTRLFLAGS_EXIST |
						(StaticFlag ? VRAMCTRLFLAGS_STATIC : 0)
						;
	if(StaticFlag)
	{
		AppLogAdd2("静的なVRAM領域が確保されました。");
		AppLogAdd2("オフセット：%d\tサイズ：%d",i * 16,Size);
	}
	return vramarray + j;
}
#endif
void FreeVRAM(DXPVRAMCTRL *ptr)
{
	int i;
	if(ptr == NULL)return;
	if(!(ptr->flags & VRAMCTRLFLAGS_EXIST))return;
	if(ptr < vramarray || ptr >= vramarray + VRAMARRAYNUM)return;
	for(i = (ptr->offset >> 4);i < (ptr->size >> 4);++i)vramdata[i] = 0x00;
	ptr->address	= 0;
	ptr->flags		= 0;
	ptr->size		= 0;
}
