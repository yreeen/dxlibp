/*
*	DXライブラリPortable	GPU制御部	Ver1.20
*	製作者	：憂煉
*	備考	：グラフィックの読み込み関数は別ファイルにする予定
*	300KByte弱を占有してます。グラフィックが読まれると一気にメモリが減るので注意
*/

/*
メモ
pspDebugScreen系関数は使うと泣きを見るので使用禁止。VRAMの先頭数千バイトが使えなくなります。
*/
#include <pspgu.h>
#include <pspgum.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include "dxlibp.h"
#include "dxpstatic.h"
#include <malloc.h>
#include <string.h>
#include "zenkaku.h"
#include <fastmath.h>
/*定数定義*/
#define GULIST_LEN	262144	/*(256 * 1024) 1MByte*/
#define GULIST_SIZE (GULIST_LEN << 2)
/*マクロ定義*/

/*大域変数定義部*/
DXPGPUSETTING gusettings = 
{
	{{GU_PSM_8888,NULL,NULL,NULL,NULL,NULL,480,272,512,480,272,0x00000000,1,0,1,0,0},{GU_PSM_8888,NULL,NULL,NULL,NULL,NULL,480,272,512,480,272,0x00000000,1,0,1,0,0}},								/*フロントバッファ*/
	{GU_PSM_4444},									/*深度バッファのポインタ*/
	NULL,									/*描画先グラフィック*/
	NULL,										/*セットされているテクスチャ*/
//	GU_PSM_8888,							/*ディスプレーバッファの色設定*/
//	GU_PSM_4444,							/*深度バッファの色設定*/
	0,										/*クリアカラー*/
	0,										/*クリア深度*/
	0,										/*クリアステンシル*/
	{GPUSETTINGFLAGS_0_CREATEVRAMGRAPH | GPUSETTINGFLAGS_0_CREATESWIZZLEDGRAPH},/*各種フラグ*/
	0x00000000,								/*カラーキー*/
	//0,										/*テクスチャの頂点色に使う値*/
	DX_BLENDMODE_NOBLEND,					/*ブレンドモード*/
	255,									/*ブレンドパラメータ*/
	255,255,255,255,						/*RGBA*/
	0,										/*2D描画時に使うZ値*/
	64,										/*何バイトでsliceするか*/
	0,										/*frontbufferのどちらが表示されているのか*/
	{0,0,480,272},								/*シザー領域の設定値*/
	0,
	0,
	{1,-1,0,0,0,0,0,-1,0}						/*GPU設定値の保持*/
};
u32 __attribute__((aligned(16))) gulist[GULIST_LEN];/*GPUに送る命令を溜め込むためのバッファ　とりあえず1MByte*/

u8 psm2bytex2table[]={
	4,
	4,
	4,
	8,
	1,
	2,
};


/*関数定義部*/
//static int ApplyBrightAndBlendMode();

void GuListSafety()
{
	if(sceGuCheckList() > GULIST_SIZE * 0.8f)
	{
		GUFINISH
		GUSTART
	}
}

DXPTEXTURE2* MakeTexture(int x,int y,int format)
{
	int size;
	int height,width,pitch;
	DXPTEXTURE2 *ptr;
	if((ptr = (DXPTEXTURE2*)MALLOC(sizeof(DXPTEXTURE2))) == NULL)return NULL;
	//テクスチャのサイズ計算
	x = MIN(x,512);
	y = MIN(y,512);
	if(x <= 0 || y <= 0)return NULL;
	height = width = 1;
	while(height < y)height <<= 1;
//	height = (y + 15) & 0xfffffff0;
	while(width < x)width <<= 1;
	pitch = width;
	switch(format)
	{
	case GU_PSM_T4:
		if(pitch < 32)pitch = 32;
		ptr->ppalette = MEMALIGN(16,sizeof(DXPPALETTE));
		break;
	case GU_PSM_T8:
		if(pitch < 16)pitch = 16;
		ptr->ppalette = MEMALIGN(16,sizeof(DXPPALETTE));
		break;
	case GU_PSM_5650:
	case GU_PSM_5551:
	case GU_PSM_4444:
		if(pitch < 8)pitch = 8;
		ptr->ppalette = NULL;
		break;
	case GU_PSM_8888:
		if(pitch < 4)pitch = 4;
		ptr->ppalette = NULL;
		break;
	default:
		FREE(ptr);
		return NULL;
	}
	if((size = GraphSize2DataSize(pitch,y,format)) == -1)
	{
		FREE(ptr->ppalette);
		FREE(ptr);
		return NULL;
	}

	//テクスチャのメモリ確保
	if((ptr->pmemory = MEMALIGN(16,size)) == NULL)
	{
		FREE(ptr->ppalette);
		FREE(ptr);
		return NULL;
	}
	memset(ptr->pmemory,0x00,size);
	ptr->pvram		= NULL;
	ptr->height		= height;
	ptr->width		= width;
	ptr->pitch		= pitch;
	ptr->umax		= x;
	ptr->vmax		= y;
	ptr->psm		= format;
	ptr->vramflag	= 0;
	ptr->swizzledflag= 0;
	ptr->reloadflag	= 1;
	ptr->alphabit	= 0;
	ptr->colorkey	= gusettings.colorkey;
	ptr->refcount	= 0;
	TextureList_PushFront(ptr);
	return ptr;
	
}

int MakeGraph(int x,int y,int format)
{
	int size;
	int height,width,pitch;
	DXPGRAPHDATA *ptr = GenerateGraphHandle();
	if(ptr == NULL) return -1;
	if((ptr->tex = (DXPTEXTURE2*)MALLOC(sizeof(DXPTEXTURE2))) == NULL)
	{
		GraphHandleFree(ptr->handle);
		return -1;
	}
	//テクスチャのサイズ計算
	x = MIN(x,512);
	y = MIN(y,512);
	if(x <= 0 || y <= 0)return -1;
	height = width = 1;
	while(height < y)height <<= 1;
//	height = (y + 15) & 0xfffffff0;
	while(width < x)width <<= 1;
	pitch = width;
	switch(format)
	{
	case GU_PSM_T4:
		if(pitch < 32)pitch = 32;
		ptr->tex->ppalette = MEMALIGN(16,sizeof(DXPPALETTE));
		break;
	case GU_PSM_T8:
		if(pitch < 16)pitch = 16;
		ptr->tex->ppalette = MEMALIGN(16,sizeof(DXPPALETTE));
		break;
	case GU_PSM_5650:
	case GU_PSM_5551:
	case GU_PSM_4444:
		if(pitch < 8)pitch = 8;
		ptr->tex->ppalette = NULL;
		break;
	case GU_PSM_8888:
		if(pitch < 4)pitch = 4;
		ptr->tex->ppalette = NULL;
		break;
	default:
		FREE(ptr->tex);
		GraphHandleFree(ptr->handle);
		return -1;
	}
	if((size = GraphSize2DataSize(pitch,y,format)) == -1)
	{
		FREE(ptr->tex->ppalette);
		FREE(ptr->tex);
		GraphHandleFree(ptr->handle);
		return -1;
	}

	//テクスチャのメモリ確保
	if((ptr->tex->pmemory = MEMALIGN(16,size)) == NULL)
	{
		FREE(ptr->tex->ppalette);
		FREE(ptr->tex);
		GraphHandleFree(ptr->handle);
		return -1;
	}
	memset(ptr->tex->pmemory,0x00,size);
	ptr->tex->pvram		= NULL;
	ptr->tex->height		= height;
	ptr->tex->width		= width;
	ptr->tex->pitch		= pitch;
	ptr->tex->umax		= x;
	ptr->tex->vmax		= y;
	ptr->tex->psm		= format;
	ptr->u0			= 0;
	ptr->v0			= 0;
	ptr->u1			= x;
	ptr->v1			= y;
	ptr->tex->vramflag	= 0;
	ptr->tex->swizzledflag= 0;
	ptr->tex->reloadflag	= 1;
	ptr->tex->alphabit	= 0;
	ptr->tex->colorkey	= gusettings.colorkey;
	ptr->tex->refcount	= 1;
	TextureList_PushFront(ptr->tex);
	return ptr->handle;
}

int GetColor(int Red,int Green,int Blue)/*現在のカラーフォーマットで色を返す*/
{
	return 0xff000000 | (Blue & 0x000000ff) << 16 | (Green & 0x000000ff) << 8 | (Red & 0x000000ff);
}

int GraphSize2DataSize(int width,int height,int psm)
{
	switch (psm)
	{
		case GU_PSM_T4:
			return (width * height) >> 1;
		case GU_PSM_T8:
			return width * height;
		case GU_PSM_5650:
		case GU_PSM_5551:
		case GU_PSM_4444:
		case GU_PSM_T16:
			return 2 * width * height;
		case GU_PSM_8888:
		case GU_PSM_T32:
			return 4 * width * height;
		default:
			return -1;
	}
}

int SetCreateSwizzledGraphFlag(int Flag)
{
	if(Flag != 0)
		gusettings.flags[0] |= GPUSETTINGFLAGS_0_CREATESWIZZLEDGRAPH;
	else
		gusettings.flags[0] &= (~GPUSETTINGFLAGS_0_CREATESWIZZLEDGRAPH);
	return 0;
}

int InitGUEngine()
{
//	AppLogAdd2("GPU関連の初期化を開始します。");
AppLogAdd2("");
	gusettings.rendertarget		= NULL;
	gusettings.texture			= NULL;
	//gusettings.displaypsm		= GU_PSM_8888;
	//gusettings.depthpsm			= GU_PSM_4444;
	gusettings.clearcolor		= GetColor(0,0,0);
	gusettings.cleardepth		= 0;
	gusettings.clearstencil		= 0;
	gusettings.flags[0]			= GPUSETTINGFLAGS_0_CREATEVRAMGRAPH | GPUSETTINGFLAGS_0_CREATESWIZZLEDGRAPH;
//gusettings.flags[0]			= 0x00000000;
	gusettings.colorkey			= 0x00000000;
//	gusettings.texcolor			= 0;
	gusettings.blendmode		= DX_BLENDMODE_NOBLEND;
	gusettings.blendparam		= 255;
	gusettings.red				= 255;
	gusettings.green			= 255;
	gusettings.blue				= 255;
	gusettings.alpha			= 255;
	gusettings.z_2d				= 0x0000;
	gusettings.backbuffer		= 0;
	gusettings.bc.forceupdate	= 1;
	//texlist = NULL;
	AppLogAdd2("テクスチャ管理データを初期化しました。");

	InitVRAM();	/*VRAM領域の初期化*/
AppLogAdd2("");
	/*フロントバッファとバックバッファの取得*/
	gusettings.displaybuffer[0].colorkey	= 0;
	gusettings.displaybuffer[0].height		= 272;
	gusettings.displaybuffer[0].next		= NULL;
	gusettings.displaybuffer[0].pitch		= 512;
	gusettings.displaybuffer[0].pmemory		= NULL;
	gusettings.displaybuffer[0].ppalette	= NULL;
	gusettings.displaybuffer[0].prev		= NULL;
//	gusettings.displaybuffer[0].psm			= GU_PSM_8888;
	gusettings.displaybuffer[0].pvram		= AllocVRAM(GraphSize2DataSize(512,272,gusettings.displaybuffer[0].psm),1);
	gusettings.displaybuffer[0].refcount	= 0xffffffff;
	gusettings.displaybuffer[0].reloadflag	= 1;
	gusettings.displaybuffer[0].swizzledflag= 1;
	gusettings.displaybuffer[0].umax		= 480;
	gusettings.displaybuffer[0].vmax		= 272;
	gusettings.displaybuffer[0].vramflag	= 1;
	gusettings.displaybuffer[0].width		= 480;
AppLogAdd2("");

	gusettings.displaybuffer[1].colorkey	= 0;
	gusettings.displaybuffer[1].height		= 272;
	gusettings.displaybuffer[1].next		= NULL;
	gusettings.displaybuffer[1].pitch		= 512;
	gusettings.displaybuffer[1].pmemory		= NULL;
	gusettings.displaybuffer[1].ppalette	= NULL;
	gusettings.displaybuffer[1].prev		= NULL;
//	gusettings.displaybuffer[1].psm			= GU_PSM_8888;
	gusettings.displaybuffer[1].pvram		= AllocVRAM(GraphSize2DataSize(512,272,gusettings.displaybuffer[1].psm),1);
	gusettings.displaybuffer[1].refcount	= 0xffffffff;
	gusettings.displaybuffer[1].reloadflag	= 1;
	gusettings.displaybuffer[1].swizzledflag= 1;
	gusettings.displaybuffer[1].umax		= 480;
	gusettings.displaybuffer[1].vmax		= 272;
	gusettings.displaybuffer[1].vramflag	= 1;
	gusettings.displaybuffer[1].width		= 480;
AppLogAdd2("");

	if(gusettings.displaybuffer[0].pvram == NULL)return -1;
	if(gusettings.displaybuffer[1].pvram == NULL)return -1;

AppLogAdd2("");

//	gusettings.frontbuffer[0] = AllocVRAM(GraphSize2DataSize(512,272,gusettings.displaypsm),1);
//	gusettings.frontbuffer[1] = AllocVRAM(GraphSize2DataSize(512,272,gusettings.displaypsm),1);
//	memset(sceGeEdramGetAddr() + gusettings.frontbuffer[1]->offset,0x88,557056);
//	if(gusettings.flags[0] & GPUSETTINGFLAGS_0_DEPTHENABLE)
	{//DepthBufferの確保
		memset(&gusettings.depthbuffer,0,sizeof(DXPTEXTURE2));
		gusettings.depthbuffer.height		= 272;
		gusettings.depthbuffer.pitch		= 512;
//		gusettings.depthbuffer.psm			= GU_PSM_4444;
		gusettings.depthbuffer.pvram		= gusettings.flags[0] & GPUSETTINGFLAGS_0_DEPTHENABLE ? AllocVRAM(GraphSize2DataSize(512,272,gusettings.depthbuffer.psm),1) : NULL;
		gusettings.depthbuffer.refcount		= 0xffffffff;
		gusettings.depthbuffer.umax			= 480;
		gusettings.depthbuffer.vmax			= 272;
		gusettings.depthbuffer.vramflag		= 1;
		gusettings.depthbuffer.width		= 480;
		if(gusettings.depthbuffer.pvram == NULL)gusettings.flags[0] &=~ GPUSETTINGFLAGS_0_DEPTHENABLE;
	}
	gusettings.rendertarget	= gusettings.displaybuffer;

	/*GPUの初期化開始*/
	sceGuInit();
	GUSTART
	sceGuDrawBuffer(gusettings.displaybuffer[0].psm,gusettings.displaybuffer[0].pvram->address,512);
	sceGuDispBuffer(480,272,gusettings.displaybuffer[1].pvram->address,512);
	if(gusettings.flags[0] & GPUSETTINGFLAGS_0_DEPTHENABLE)
	{
		sceGuDepthBuffer(gusettings.depthbuffer.pvram->address,512);
		sceGuEnable(GU_DEPTH_TEST);
	}else sceGuDisable(GU_DEPTH_TEST);

	sceGuOffset(2048 - (480/2),2048 - (272/2));
	sceGuViewport(2048,2048,480,272);
	sceGuScissor(
		gusettings.scissor[0],
		gusettings.scissor[1],
		gusettings.scissor[2],
		gusettings.scissor[3]
	);

	sceGuDepthRange(65535,0);
//	sceGuScissor(0,0,480,272);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuFrontFace(GU_CW);
	sceGuEnable(GU_TEXTURE_2D);
	//sceGuEnable(GU_FRAGMENT_2X);
	sceGuDepthMask(0xffff);
	sceGuShadeModel(GU_FLAT);
	sceGuTexScale(1.0f,1.0f);
	sceGuTexOffset(0.0f,0.0f);
		sceGuAmbientColor(0xffffffff);

	sceGumMatrixMode(GU_PROJECTION);
	sceGumLoadIdentity();
	sceGumOrtho(0,480,0,272,1,1000);
	//{
	//	ScePspFMatrix4 m;
	//	m.x.x = 1;	m.x.y = 0;	m.x.z = 0;	m.x.w = 0;
	//	m.y.x = 0;	m.y.y = 1;	m.y.z = 0;	m.y.w = 0;
	//	m.z.x = 0;	m.z.y = 0;	m.z.z = 1;	m.z.w = 0;
	//	m.w.x = 0;	m.w.y = 0;	m.w.z = 0;	m.w.w = 1;
	//	sceGumLoadMatrix(&m);
	//}
//	sceGumPerspective(90.0f,16.0f/9.0f,-1000.0f,1000.0f);

	sceGumMatrixMode(GU_VIEW);
	sceGumLoadIdentity();
	{
		ScePspFMatrix4 m;
		m.x.x = 1;	m.x.y = 0;	m.x.z = 0;	m.x.w = 0;
		m.y.x = 0;	m.y.y = 1;	m.y.z = 0;	m.y.w = 0;
		m.z.x = 0;	m.z.y = 0;	m.z.z = 1;	m.z.w = 0;
		m.w.x = 0;	m.w.y = 0;	m.w.z = 0;	m.w.w = 1;
		sceGumLoadMatrix(&m);
//		ScePspFVector3 pos = { -240.0f, -136.0f, 0.0f };
//		sceGumTranslate(&pos);
	}

	sceGumMatrixMode(GU_MODEL);
	sceGumLoadIdentity();


/*	sceGuShadeModel(GU_SMOOTH);*/
//	ApplyBrightAndBlendMode();
	SetDrawMode(DX_DRAWMODE_NEAREST);
	GUFINISH
	DxpGuAdminInit();
	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);
	ScreenFlip();
	return 0;
}

int EndGUEngine()
{
	GUFINISH;
	sceGuTerm();
	DxpGuAdminEnd();
	FreeVRAM(gusettings.displaybuffer[0].pvram);
	FreeVRAM(gusettings.displaybuffer[1].pvram);
	InitVRAM();
	return 0;
}

void	dxpDisplayWaitVblankStart()
{
	while(1)
	{
		int	newVcount = sceDisplayGetVcount();
		if (newVcount >= (gusettings.vsynccount + gusettings.flipmode))
		{
			gusettings.vsynccount = newVcount;
			return;
		}
	}
	return;
}

void	WaitDrawProcess()
{
	if(gusettings.flags[0] & GPUSETTINGFLAGS_0_GUSTART)		
	{														
		sceGuFinish();										
		gusettings.flags[0] &= (~GPUSETTINGFLAGS_0_GUSTART);
		sceGuSync(3,0);										
	}	
	return;
}

int		ScreenFlipMode(int Mode)
{
	if(Mode < 0) return -1;
	if(Mode > 4) return -1;
	gusettings.flipmode		= Mode;
	gusettings.vsynccount	= sceDisplayGetVcount();
	return 0;
}

int		ScreenFlip()
{
	void *p;
	GUFINISH
	if(gusettings.flipmode == 0)
	{
		sceDisplayWaitVblankStart();//垂直同期を待つ
	}
	else
	{
		dxpDisplayWaitVblankStart();
	}
	//スワップするときにデバッグ用のスクリーンのオフセットを指定する。
	DrawDebugScreen();
	p = sceGuSwapBuffers();
	mh_displayoffset((u32)(p) & 0x00ffffff);
	gusettings.backbuffer ^= 1;
	if(gusettings.rendertarget == &gusettings.displaybuffer[0] || gusettings.rendertarget == &gusettings.displaybuffer[1])
		gusettings.rendertarget = &gusettings.displaybuffer[gusettings.backbuffer];
	return 0;
}

int ScreenCopy()
{
	if(ScreenFlip() != 0)return -1;
	void *src = sceGeEdramGetAddr(),*dst;
	GUFINISH
	dst = src;
	src += gusettings.displaybuffer[gusettings.backbuffer^1].pvram->offset;
	dst += gusettings.displaybuffer[gusettings.backbuffer].pvram->offset;
	memcpy(dst,src,GraphSize2DataSize(512,272,gusettings.displaybuffer[0].psm));
	sceKernelDcacheWritebackAll();
	return 0;
}

int ClearDrawScreen()
{
	GUSTART;
	sceGuClearColor(gusettings.clearcolor);
	if(gusettings.flags[0] & GPUSETTINGFLAGS_0_DEPTHENABLE && gusettings.flags[0] & GPUSETTINGFLAGS_0_CLEARDEPTH)
		sceGuClearDepth(gusettings.cleardepth);
	if(gusettings.flags[0] & GPUSETTINGFLAGS_0_STENCILENABLE && gusettings.flags[0] & GPUSETTINGFLAGS_0_CLEARSTENCIL)
		sceGuClearStencil(gusettings.clearstencil);
	sceGuClear(
		GU_COLOR_BUFFER_BIT |
		((gusettings.flags[0] & GPUSETTINGFLAGS_0_DEPTHENABLE && gusettings.flags[0] & GPUSETTINGFLAGS_0_CLEARDEPTH) ? GU_DEPTH_BUFFER_BIT : 0) |
		((gusettings.flags[0] & GPUSETTINGFLAGS_0_STENCILENABLE && gusettings.flags[0] & GPUSETTINGFLAGS_0_CLEARSTENCIL) ? GU_STENCIL_BUFFER_BIT : 0)
		);
	return 0;
}

DXPGRAPHDATA *bptr = NULL;
 
int SetTexture2(DXPTEXTURE2 *texptr,int TransFlag)
{
	GUSTART;
	bptr = NULL;
	if(texptr == NULL)
	{
		/*テクスチャを解除する。*/
		gusettings.texture = NULL;
		sceGuDisable(GU_TEXTURE_2D);
		return 0;
	}
	if(!sceGuGetStatus(GU_TEXTURE_2D))sceGuEnable(GU_TEXTURE_2D);
	if(gusettings.texture != texptr || texptr->reloadflag )/*同じテクスチャがセットされている場合はなにもしない。パフォーマンス落ちちゃうからｗ*/
	{
		texptr->reloadflag = 0;
		/*テクスチャのフォーマットを審査し、必要ならパレットを設定する*/
		switch(texptr->psm)
		{
		case GU_PSM_4444:/*通常のテクスチャ*/
		case GU_PSM_5551:
		case GU_PSM_5650:
		case GU_PSM_8888:
		case GU_PSM_DXT1:/*圧縮されたテクスチャ*/
		case GU_PSM_DXT3:
		case GU_PSM_DXT5:
			break;
		case GU_PSM_T4:/*たぶんパレットだと思うんだよね…よくわかんないケド*/
		case GU_PSM_T8:
	//	case GU_PSM_T16:
	//	case GU_PSM_T32:
			if(texptr->ppalette == NULL)
			{
				return -1;
			}
			sceGuClutMode(GU_PSM_8888,0,0xff,0);
			sceGuClutLoad(256 / 8,texptr->ppalette->data);
			break;
		default:
			return -1;
		}

		sceGuTexMode(texptr->psm,0,0,texptr->swizzledflag ? GU_TRUE : GU_FALSE);
		/*テクスチャをセット*/
		if(texptr->vramflag)
		{
			sceGuTexImage(0,texptr->width,texptr->height,texptr->pitch,sceGeEdramGetAddr() + texptr->pvram->offset);
		}
		else
		{
			sceGuTexImage(0,texptr->width,texptr->height,texptr->pitch,texptr->pmemory);
		}
		gusettings.texture = texptr;
	}

//ここから色設定やブレンドモード設定等を行う。
//なにを設定するか調査
	u8	ColorKey	= 0;
	u8	AlphaChannel= 0;
	if(TransFlag)
	{
		if(texptr->alphabit)
		{
			AlphaChannel = 1;
		}
		else
		{
			ColorKey = 1;
		}
	}

//カラーキーを設定する
	if(ColorKey)
	{
		if(!sceGuGetStatus(GU_COLOR_TEST))sceGuEnable(GU_COLOR_TEST);
		sceGuColorFunc(GU_NOTEQUAL,texptr->colorkey,0x00fefefe);
	}
	else
	{
		if(sceGuGetStatus(GU_COLOR_TEST))sceGuDisable(GU_COLOR_TEST);
	}
//ブレンディングの方法を設定する
	int op;
	int src,dest;
	unsigned int srcfix;
	unsigned int destfix;	 
	switch(gusettings.blendmode)
	{
	case DX_BLENDMODE_NOBLEND:
		op = GU_ADD;
		src = GU_FIX;
		dest = GU_FIX;
		srcfix = 0xffffffff;
		destfix = 0;
		if(!AlphaChannel)break;
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
		return -1;
	}

	if(gusettings.blendmode == DX_BLENDMODE_NOBLEND && !AlphaChannel)
	{
		if(sceGuGetStatus(GU_BLEND))sceGuDisable(GU_BLEND);
	}
	else
	{
		if(!sceGuGetStatus(GU_BLEND))sceGuEnable(GU_BLEND);
		if(gusettings.bc.forceupdate
			|| gusettings.bc.op != op
			|| gusettings.bc.src != src
			|| gusettings.bc.dest != dest
			|| gusettings.bc.srcfix != srcfix
			|| gusettings.bc.destfix != destfix
		){
			sceGuBlendFunc(op,src,dest,srcfix,destfix);
			gusettings.bc.op = op;
			gusettings.bc.src = src;
			gusettings.bc.dest = dest;
			gusettings.bc.srcfix = srcfix;
			gusettings.bc.destfix = destfix;
		}
	}
//色を設定
	unsigned int color;
	int tfx,tcc;
	color = (u32)(gusettings.blendmode == DX_BLENDMODE_NOBLEND ? 255 : gusettings.alpha) << 24 | (u32)(gusettings.blue) << 16 | (u32)(gusettings.green) << 8 | (u32)(gusettings.red);
	if(gusettings.bc.color != color || gusettings.bc.forceupdate)
	{
		sceGuColor(color);
		gusettings.bc.color = color;
	}

	switch(gusettings.blendmode)
	{
	case DX_BLENDMODE_NOBLEND:
	case DX_BLENDMODE_MUL:
	case DX_BLENDMODE_DESTCOLOR:
		tcc = GU_TCC_RGB;
		tfx = GU_TFX_MODULATE;
		if(AlphaChannel)tcc = GU_TCC_RGBA;
	break;
	case DX_BLENDMODE_ALPHA:
	case DX_BLENDMODE_ADD:
	case DX_BLENDMODE_SUB:
	case DX_BLENDMODE_INVDESTCOLOR:
		tcc = GU_TCC_RGBA;
		tfx = GU_TFX_MODULATE;
		break;
	case DX_BLENDMODE_INVSRC:
		sceGuTexEnvColor(0x00000000);
		tcc = GU_TCC_RGBA;
		tfx = GU_TFX_BLEND;
		break;
	default:
		return -1;
	}
	if(gusettings.bc.forceupdate || gusettings.bc.tfx != tfx || gusettings.bc.tcc)
	{
		sceGuTexFunc(tfx,tcc);
		gusettings.bc.tfx = tfx;
		gusettings.bc.tcc = tcc;
	}
	gusettings.bc.forceupdate = 0;
	return 0;
}


int SetTexture(int handle,int TransFlag)//テクスチャを使う描画関数で呼ぶ。テクスチャ使わない描画関数ではSetBaseColorを使う事。
{
	DXPGRAPHDATA *gptr = GraphHandle2Ptr(handle);
	if(gptr == NULL)return -1;
	int res = SetTexture2(gptr->tex,TransFlag);
	bptr = res == 0 ? gptr : NULL;
	return res;
}
/*static*/ int SetBaseColor(u32 color)
/* DrawLine等のテクスチャ非使用関数で呼ぶ。色のセット*/
{
	GUSTART;
	gusettings.texture = NULL;
	bptr = NULL;
	sceGuDisable(GU_TEXTURE_2D);
	switch(gusettings.blendmode)
	{
	case DX_BLENDMODE_INVSRC:
		color = (color & 0xff000000) | ~(color & 0x00ffffff);
	case DX_BLENDMODE_NOBLEND:
	case DX_BLENDMODE_ALPHA:
	case DX_BLENDMODE_ADD:
	case DX_BLENDMODE_SUB:
	case DX_BLENDMODE_MUL:
	case DX_BLENDMODE_DESTCOLOR:
	case DX_BLENDMODE_INVDESTCOLOR:
	break;
	default:
		return -1;
	}
	u32 r,g,b,a;
	r = color & 0x000000ff;
	r *= gusettings.red;
	r /= 255;
	r &= 0x000000ff;
	g = (color & 0x0000ff00) >> 8;
	g *= gusettings.green;
	g /= 255;
	g &= 0x000000ff;
	b = (color & 0x00ff0000) >> 16;
	b *= gusettings.blue;
	b /= 255;
	b &= 0x000000ff;
	a = (color & 0xff000000) >> 24;
	a *= gusettings.alpha;
	a /= 255;
	a &= 0x000000ff;
	color = (a << 24) | (b << 16) | (g << 8) | r;
	if(gusettings.bc.color != color || gusettings.bc.forceupdate)
	{
		sceGuColor(color);
		gusettings.bc.color = color;
	}
//ブレンディングの方法を設定する
	int op;
	int src,dest;
	unsigned int srcfix;
	unsigned int destfix;	 
	switch(gusettings.blendmode)
	{
	case DX_BLENDMODE_NOBLEND:
		break;
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
		srcfix = 0;
		destfix = 0;
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
		return -1;
	}
	if(gusettings.blendmode == DX_BLENDMODE_NOBLEND)
	{
		if(sceGuGetStatus(GU_BLEND))sceGuDisable(GU_BLEND);
	}
	else
	{
		if(!sceGuGetStatus(GU_BLEND))sceGuEnable(GU_BLEND);
		if(gusettings.bc.forceupdate
			|| gusettings.bc.op != op
			|| gusettings.bc.src != src
			|| gusettings.bc.dest != dest
			|| gusettings.bc.srcfix != srcfix
			|| gusettings.bc.destfix != destfix
		){
			sceGuBlendFunc(op,src,dest,srcfix,destfix);
			gusettings.bc.op = op;
			gusettings.bc.src = src;
			gusettings.bc.dest = dest;
			gusettings.bc.srcfix = srcfix;
			gusettings.bc.destfix = destfix;
		}
	}
	gusettings.bc.forceupdate = 0;
	return 0;
}

int SetDrawMode(int Mode)
{
	GUSTART;
	switch(Mode)
	{
	case DX_DRAWMODE_NEAREST:
		sceGuTexFilter(GU_NEAREST,GU_NEAREST);
		gusettings.flags[0] &= (~GPUSETTINGFLAGS_0_BILINEAR);
		return 0;
	case DX_DRAWMODE_BILINEAR:
		sceGuTexFilter(GU_LINEAR,GU_LINEAR);
		gusettings.flags[0] |= GPUSETTINGFLAGS_0_BILINEAR;
		return 0;
	default:
		return -1;
	}
}

//static int ApplyBrightAndBlendMode();
int SetDrawBright(int Red,int Green,int Blue)
{
	gusettings.red		= Red & 0x000000ff;
	gusettings.green	= Green & 0x000000ff;
	gusettings.blue		= Blue & 0x000000ff;
	return 0;
}

int SetDrawBlendMode(int BlendMode,int Param)
{
	gusettings.blendmode = BlendMode;
	gusettings.blendparam= Param;
	gusettings.alpha = Param & 0x000000ff;
	return 0;
}


int SetDrawScreen(int ghandle)
{
	GUSTART
	if(ghandle == -1)return -1;
	if(ghandle == DXP_SCREEN_BACK)
	{
		gusettings.rendertarget = &gusettings.displaybuffer[gusettings.backbuffer];
	}
	else
	{
		DXPTEXTURE2 *tmp = GraphHandle2TexPtr(ghandle);
		if(tmp != NULL)
		{
			if(tmp->vramflag && !tmp->swizzledflag)gusettings.rendertarget = tmp;
		}
	}
	if(gusettings.rendertarget == NULL)gusettings.rendertarget = &gusettings.displaybuffer[gusettings.backbuffer];
	sceGuTexFlush();
	sceGuDrawBufferList(gusettings.rendertarget->psm,gusettings.rendertarget->pvram->address,gusettings.rendertarget->pitch);
	sceGuOffset(2048 - (gusettings.rendertarget->width/2),2048 - (gusettings.rendertarget->height/2));
	sceGuViewport(2048,2048,gusettings.rendertarget->width,gusettings.rendertarget->height);

	return 0;
}

int SaveDrawScreen( int x1, int y1, int x2, int y2, char *FileName )
{
	return 0;
}

int SetDisplayFormat(int format)
{
	switch(format)
	{
	case GU_PSM_4444:
	case GU_PSM_5551:
	case GU_PSM_5650:
	case GU_PSM_8888:
		break;
	default:
		return -1;
	}
	if(format == gusettings.displaybuffer[0].psm)return 0;
	if(gusettings.displaybuffer[0].pvram == NULL)
	{
		gusettings.displaybuffer[0].psm = gusettings.displaybuffer[1].psm = format;
		return 0;
	}
	int size = GraphSize2DataSize(512,272,format);
	int pformat = gusettings.displaybuffer[0].psm;
	FreeVRAM(gusettings.displaybuffer[0].pvram);
	FreeVRAM(gusettings.displaybuffer[1].pvram);
	gusettings.displaybuffer[0].pvram = AllocVRAM(size,1);
	gusettings.displaybuffer[1].pvram = AllocVRAM(size,1);
	if(gusettings.displaybuffer[0].pvram == NULL || gusettings.displaybuffer[1].pvram == NULL)
	{
		FreeVRAM(gusettings.displaybuffer[0].pvram);
		FreeVRAM(gusettings.displaybuffer[1].pvram);
		size = GraphSize2DataSize(512,272,pformat);
		gusettings.displaybuffer[0].pvram = AllocVRAM(size,1);
		gusettings.displaybuffer[1].pvram = AllocVRAM(size,1);
		return -1;
	}
	gusettings.displaybuffer[0].psm = format;
	gusettings.displaybuffer[1].psm = format;
	return 0;
}

int GetDisplayFormat()
{
	return gusettings.displaybuffer[0].psm;
}


int DeleteGraph(int gh)
{
	DXPGRAPHDATA* gptr = GraphHandle2Ptr(gh);
	if(gptr == NULL)return -1;
	if(gptr->tex != NULL)
	{
		--gptr->tex->refcount;
		if(gptr->tex->refcount <= 0)
		{
			FREE(gptr->tex->pmemory);
			FREE(gptr->tex->ppalette);
			FreeVRAM(gptr->tex->pvram);
			TextureList_Remove(gptr->tex);
			FREE(gptr->tex);
		}
	}
	GraphHandleFree(gptr->handle);
	return 0;
}

int SetSliceSize(int size)
{
	switch(size)
	{
	case 16:
	case 32:
	case 64:
	case 128:
		break;
	default:
		return -1;
	}
	gusettings.slice = size;
	return 0;
}

int GetGraphSize(int gh,int *px,int *py)
{
	DXPGRAPHDATA* gptr = GraphHandle2Ptr(gh);
	if(gptr == NULL)return -1;
	if(px != NULL)*px = gptr->u1 - gptr->u0;
	if(py != NULL)*py = gptr->v1 - gptr->v0;
	return 0;
}

int SetTransColor(int red,int green,int blue)
{
	gusettings.colorkey = ((blue & 0x000000ff) << 16) | ((green & 0x000000ff) << 8) | (red & 0x000000ff);
	return 0;
}

void WaitGPUSync()
{
	GUFINISH
}

int SetDrawArea(int x1,int y1,int x2,int y2)
{
	GUSTART
	gusettings.scissor[0] = x1;
	gusettings.scissor[1] = y1;
	gusettings.scissor[2] = x2;
	gusettings.scissor[3] = y2;
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuScissor(gusettings.scissor[0],gusettings.scissor[1],gusettings.scissor[2],gusettings.scissor[3]);
	return 0;
}
