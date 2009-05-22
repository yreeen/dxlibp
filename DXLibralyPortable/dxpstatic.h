#ifndef	DXPSTATIC_H__
#define	DXPSTATIC_H__
/*
*	DXライブラリPortable	ライブラリ内部用ヘッダ
*	製作者	：憂煉
*/

#include <pspkernel.h>

/*DXPのコンパイル時オプション*/

#define DXP_NOUSE_LIBJPEG		/*コメントアウトすると512x512以上のサイズのJpegファイルを読み込めるようになります。ただし、実行ファイルのサイズが80KBほど大きくなります。*/
#define DXP_NOUSE_MTRAND		/*コメントアウトすると乱数生成にメルセンヌ・ツイスターを使います。デフォルトではLFSR方式改となります。*/
/*#define DXP_NON_ZENKAKU*/		/*全角文字のデバッグスクリーンを使わない場合はコメントアウトを外してください。*/
//#define DXP_NOUSE_FLTVERTEX_WITH_ROTA	/*コメントアウトするとDrawRotaGraph、DrawRotaGraph2の描画に不動小数点数型頂点を使うようになります。*/
/*DXPのコンパイル時オプション終わり*/



/*マクロ*/

//#define PTR2HND(PTR)	((int)(PTR) - 1)
//#define HND2PTR(HND)	((void*)((int)HND + 1))
#define PTR2HND(PTR)	((PTR) - 1)
#define HND2PTR(HND)	(HND + 1)

#define MIN(A,B)		((A) > (B) ? (B) : (A))
#define MAX(A,B)		((A) > (B) ? (A) : (B))

/*ライブラリ本体の設定とか*/
#define	DXPDATAFLAGS_0_INITIALIZED	0x00000001
#define	DXPDATAFLAGS_0_INITIALIZING	0x00000002

typedef	struct
{
	u32			TPS;		/*ティック　パー　セコンド　RTC（リアルタイムクロック）が幾つで秒単位になるのかを記録　PSP-2000では1000000だった*/
	u32			flags[1];
}DXPDATA;

/*VRAM上のアドレスを管理するための構造体*/
#define	VRAMCTRLFLAGS_EXIST		0x01
#define	VRAMCTRLFLAGS_STATIC	0x02
typedef struct tagDXPVRAMCTRL
{
	union
	{
		u32		offset;
		void*	address;
	};
	u32			size;
	u8			flags;
}DXPVRAMCTRL;

/*パレットデータ*/
typedef struct tagDXPPALETTE	/*PSMはGU_PSM_8888で固定です。*/
{
	u32			data[256];
}DXPPALETTE;

/*テクスチャ管理構造体*/
//#define TEXTURE_MAX	(1024)		/*同時に利用可能なグラフィックハンドルの最大数*/
//#define	TEXTUREFLAGS_VRAM		0x01
//#define	TEXTUREFLAGS_SWIZZLED	0x02
//#define	TEXTUREFLAGS_EXIST		0x04
//#define TEXTUREFLAGS_RELOAD		0x08	/*テクスチャの強制リロードが必要な場合に設定される。一度ロードされたらフラグは倒される。*/
//typedef struct tagDXPTEXTURE
//{
//	DXPPALETTE	*ppalette;				/*パレットデータへのポインタ*/
//	DXPVRAMCTRL *pvram;					/*VRAMデータへのポインタ*/
//	void		*pmemory;				/*RAM上のポインタ*/
//	int			psm;					/*テクスチャフォーマット*/
//	u16			width;					/*テクスチャの横サイズ*/
//	u16			height;					/*テクスチャの縦サイズ*/
//	u16			u0,v0;					/*テクスチャの実際に使われる領域のサイズ*/
//	u16			mu,mv;					/*テクスチャの実際に使われる領域のサイズ*/
//	u16			pitch;					/*テクスチャの幅。byte単位だと思う*/
//	u32			colorkey;				/*カラーキー*/
//	unsigned	vramflag:1;				/*VRAM上に存在しているかどうかのフラグ。ビットフィールドです。*/
//	unsigned	swizzledflag:1;			/*Swizzleされているかどうか*/
//	unsigned	existflag:1;			/*存在しているかどうか*/
//	unsigned	reloadflag:1;			/*強制リロードするかどうか*/
//	struct tagDXPTEXTURE	*next,*prev;
//	int			handle;
//	int			refhandle;
//	int			refcount;
//}DXPTEXTURE;

typedef struct tagDXPTEXTURE2
{
	int			psm;
	struct tagDXPTEXTURE2 *next,*prev;
	DXPPALETTE	*ppalette;
	DXPVRAMCTRL	*pvram;
	void*		pmemory;
	u16			width;					/*テクスチャの横サイズ*/
	u16			height;					/*テクスチャの縦サイズ*/
	u16			pitch;					/*テクスチャの横サイズ*/
	u16			umax,vmax;				/*有効なUV座標の最大値*/
	u32			colorkey;				/*カラーキー*/
	unsigned	vramflag:1;				/*VRAM上に存在しているかどうかのフラグ。ビットフィールドです。*/
	unsigned	swizzledflag:1;
	unsigned	reloadflag:1;
	unsigned	alphabit:1;				/*読み込んだ画像にα情報が存在したかどうか*/
	int			refcount;
}DXPTEXTURE2;

typedef	struct	tagDXPGRAPHDATA
{
	struct tagDXPGRAPHDATA	*next,*prev;
	DXPTEXTURE2	*tex;
	u16			u0,v0,u1,v1;
	int			handle;
}DXPGRAPHDATA;
extern DXPGRAPHDATA	*graphlist;

/*描画設定構造体*/
#define	GPUSETTINGFLAGS_0_GUSTART		0x00000001	/*GuStart〜GuFinishの間である*/
#define	GPUSETTINGFLAGS_0_DEPTHENABLE	0x00000002	/*深度バッファが有効になっている*/
#define GPUSETTINGFLAGS_0_STENCILENABLE	0x00000004	/*ステンシルバッファが有効になっている*/
#define	GPUSETTINGFLAGS_0_CLEARDEPTH	0x00000008	/*ClearDrawScreenで深度バッファを初期化する*/
#define GPUSETTINGFLAGS_0_CLEARSTENCIL	0x00000010	/*ClearDrawScreenでステンシルバッファを初期化する*/
#define GPUSETTINGFLAGS_0_BILINEAR		0x00000020	/*バイリニアモードでテクスチャを使う。立っていなければ最近点モード*/
#define	GPUSETTINGFLAGS_0_CREATESWIZZLEDGRAPH	0x00000040	/*Swizzleしているグラフィックを作る。*/
#define	GPUSETTINGFLAGS_0_CREATEVRAMGRAPH	0x00000080	/*VRAM上にグラフィックを作るか*//*デフォルトでON*/
typedef struct tagDXPGPUSETTING
{
	DXPTEXTURE2	displaybuffer[2];
	DXPTEXTURE2	depthbuffer;
	DXPTEXTURE2	*rendertarget;
//	DXPVRAMCTRL	*frontbuffer[2];/*画面バッファ*/
//	DXPVRAMCTRL	*depthbuffer;/*深度バッファ*/
	//int		rendertarget;/*描画先のテクスチャのハンドル　バックバッファの場合はDXP_SCREEN_BACK*/
	DXPTEXTURE2	*texture;/*セットされているテクスチャのハンドル　セットされていなければ-1*/
//	int			displaypsm;/*ディスプレーの色設定*/
//	int			depthpsm;/*深度バッファの色（？）設定*/
	u32			clearcolor;/*ClearDrawScreen時に使う背景色*/
	u32			cleardepth;/*ClearDrawScreen時に使う深度*/
	u32			clearstencil;/*ClearDrawScreen時に使うステンシル値*/
	u32			flags[1];	/*各種フラグデータ。できるだけメモリは食いたくないのでbit管理*/
	u32			colorkey;	/*カラーキーのデフォルト値*/
//	int			texcolor;	/*テクスチャの頂点色として指定する。一部のブレンドモード専用*/
	int			blendmode;	/*SetDrawBlendModeで指定される値*/
	int			blendparam;
	u8			red,green,blue,alpha;/*SetDrawBrightとSetDrawBlendModeで指定された値から割り出される数値*/
	u16			z_2d;		/*2D描画時に用いるZ値。*/
	u8			slice;		/*何バイトでsliceするか*/
	u8			backbuffer;/*displaybufferのどちらが裏画面とされているのか*/
	int			scissor[4];	/*sceGuScissorの設定値を保存。DrawString系関数のために使用する。*/
	struct BLENDANDCOLOR
	{
		u8	forceupdate;//強制設定更新
		u32	color;
		int op;
		int src;
		int dest;
		u32 srcfix;
		u32 destfix;
		int tfx,tcc;
	}bc;
}DXPGPUSETTING;
extern DXPGPUSETTING gusettings;

/*頂点構造体*/
#define	DXP_VTYPE_2D	GU_VERTEX_16BIT
typedef struct
{
	s16			x,y,z;
}DXPVERTEX_2D;

#define	DXP_VTYPE_2DTEX	(GU_VERTEX_16BIT | GU_TEXTURE_16BIT)
typedef struct
{
	u16			u,v;
	s16			x,y,z;
}DXPVERTEX_2DTEX;

#define	DXP_VTYPE_2DTEX_F	(GU_VERTEX_32BITF | GU_TEXTURE_16BIT)
typedef	struct
{
	u16			u,v;
	float		x,y,z;
}DXPVERTEX_2DTEX_F;

#define DXP_VTYPE_3DTEX_F	(GU_VERTEX_32BITF | GU_TEXTURE_16BIT | GU_COLOR_8888)
typedef	struct
{
	u16			u,v;
	u32			color;
	float		x,y,z;
}DXPVERTEX_3DTEX_F;


/*VRAM領域の取得、開放*/
void InitVRAM();
DXPVRAMCTRL* AllocVRAM(int Size,int StaticFlag);
void FreeVRAM(DXPVRAMCTRL *VRamCtrl);
extern u32 gulist[];
/*GPU系*/
#define GUSTART												\
{															\
	if(!(gusettings.flags[0] & GPUSETTINGFLAGS_0_GUSTART))	\
	{														\
		sceGuStart(GU_DIRECT,gulist);						\
		gusettings.flags[0] |= GPUSETTINGFLAGS_0_GUSTART;	\
	}														\
}
#define GUFINISH											\
{															\
	if(gusettings.flags[0] & GPUSETTINGFLAGS_0_GUSTART)		\
	{														\
		sceGuFinish();										\
		gusettings.flags[0] &= (~GPUSETTINGFLAGS_0_GUSTART);\
		sceGuSync(0,0);										\
	}														\
}

int InitGUEngine();
int EndGUEngine();
int SetTexture(int GrHandle,int TransFlag);
/*デバッグスクリーン用*/
void InitDebugScreen();
void DrawDebugScreen();

int LoadModule_AV_MP3();/*MP3のコーデックをロードする。*/
int ProcessAudio();
DXPGRAPHDATA* GraphHandle2Ptr(int handle);
DXPTEXTURE2* GraphHandle2TexPtr(int handle);
void TextureList_PushFront(DXPTEXTURE2 *ptr);
void TextureList_Remove(DXPTEXTURE2 *ptr);
void GraphDataList_PushFront(DXPGRAPHDATA *ptr);
DXPTEXTURE2* MakeTexture(int x,int y,int format);
int GenerateGraphHandle();//ハンドルの番号を生成する。
int PSM2BYTEx2(int psm);
#define MALLOC		malloc
#define	MEMALIGN	memalign
#define	FREE		free


#define LINETRACE 	ClearDrawScreen();printfDx("%s::%d \n",__FILE__,__LINE__);ScreenFlip();

#endif
