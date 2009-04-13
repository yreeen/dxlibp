/*
*	DXライブラリPortable	画像読み込み、変換部	Ver1.10
*	製作者	：憂煉
*	備考	：けっこう色々と処理があります。Swizzle処理やVRAMへの転送はここで行っています。
*	
*/

#include <pspgu.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <png.h>
#include "dxlibp.h"
#include "dxpstatic.h"
#include <pspjpeg.h>
/*
画像の大きさ
実際に読み出す領域
ピッチの計算
確保
読み出し
登録

*/
#define GU_PSM_5650		(0) /* Display, Texture, Palette */
#define GU_PSM_5551		(1) /* Display, Texture, Palette */
#define GU_PSM_4444		(2) /* Display, Texture, Palette */
#define GU_PSM_8888		(3) /* Display, Texture, Palette */

DXPTEXTURE2* LoadJpegImage(const char *FileName);
DXPTEXTURE2* LoadPngImage(const char *FileName);



int LoadGraph(const char *FileName)
{
	DXPGRAPHDATA *gptr = (DXPGRAPHDATA*)MALLOC(sizeof(DXPGRAPHDATA));
	if(gptr == NULL)return -1;
	gptr->tex = LoadPngImage(FileName);
	if(gptr->tex == NULL)gptr->tex = LoadJpegImage(FileName);
	if(gptr->tex == NULL)
	{
		FREE(gptr);
		return -1;
	}
	++gptr->tex->refcount;
	gptr->handle = GenerateGraphHandle();
	gptr->u0 = 0;
	gptr->v0 = 0;
	gptr->u1 = gptr->tex->umax;
	gptr->v1 = gptr->tex->vmax;
	GraphDataList_PushFront(gptr);
	if(gusettings.flags[0] & GPUSETTINGFLAGS_0_CREATESWIZZLEDGRAPH)
		SwizzleGraph(gptr->handle);
	if(gusettings.flags[0] & GPUSETTINGFLAGS_0_CREATEVRAMGRAPH)
		MoveGraphToVRAM(gptr->handle);
	return gptr->handle;
}

int DerivationGraph( int SrcX, int SrcY,int Width, int Height, int src )
{
	if(SrcX < 0 || SrcY < 0)return -1;
	DXPGRAPHDATA *texptr = GraphHandle2Ptr(src);
	if(texptr == NULL)return -1;
	if(texptr->tex == NULL)return -1;
	if(SrcX + Width > texptr->tex->width || SrcY + Height > texptr->tex->height)return -1;	
	DXPGRAPHDATA *res = (DXPGRAPHDATA*)MALLOC(sizeof(DXPGRAPHDATA));
	if(res == NULL)return -1;
	res->u0 = SrcX;
	res->v0 = SrcY;
	res->u1 = SrcX + Width;
	res->v1 = SrcY + Height;
	res->tex = texptr->tex;
	++res->tex->refcount;
	res->handle = GenerateGraphHandle();
	GraphDataList_PushFront(res);
	return res->handle;
}
//#define	CODE090405hvhkrhIMnI
#ifdef	CODE090405hvhkrhIMnI
//シューティング大好き氏のコード。大規模仕様変更のため使用不能になりました。
u16	TextureGetColor16(DXPTEXTURE *texptr,int Sx,int Sy)
{
	if(texptr == NULL)return 0;
	u16* TEXRAM = texptr->vramflag ? sceGeEdramGetAddr() + texptr->pvram->flags : texptr->pmemory;
	return TEXRAM[texptr->pitch * Sy + Sx];
}

//32bitアルファ有りフォーマット固定
u32	TextureGetColor32(DXPTEXTURE *texptr,int Sx,int Sy)
{
	if(texptr == NULL)return 0;
	u32* TEXRAM = texptr->vramflag ? sceGeEdramGetAddr() + texptr->pvram->flags : texptr->pmemory;
	return TEXRAM[texptr->pitch * Sy + Sx];
}

//指定したテクスチャの任意座標へ指定した色情報をセット
void	TextureSetColor16(DXPTEXTURE *texptr,int Sx,int Sy,u16 color)
{
	if(texptr == NULL)return 0;
	u16* TEXRAM = texptr->vramflag ? sceGeEdramGetAddr() + texptr->pvram->flags : texptr->pmemory;
	TEXRAM[texptr->pitch * Sy + Sx] = color;
}
//32bitアルファ有りフォーマット固定
void	TextureSetColor32(DXPTEXTURE *texptr,int Sx,int Sy,u32 color)
{
	if(texptr == NULL)return 0;
	u32* TEXRAM = texptr->vramflag ? sceGeEdramGetAddr() + texptr->pvram->flags : texptr->pmemory;
	TEXRAM[texptr->pitch * Sy + Sx] = color;
}

//範囲コピー用 Swizzleされていないこと前提
int	TextureCopy16(int TextureNo1,int TextureNo2,int No1Sx,int No1Sy,int No2Sx,int No2Sy, int width, int height)
{
	//if (texarray[TextureNo1].flags != TEXTUREFLAGS_XXXX ) return -1;
	//if (texarray[TextureNo2].flags != TEXTUREFLAGS_XXXX ) return -2;
	DXPTEXTURE *texptr[2];
	texptr[0] = GraphHandle2Ptr(TextureNo1);
	texptr[1] = GraphHandle2Ptr(TextureNo2);
	if(texptr[0] == NULL)return -1;
	if(texptr[1] == NULL)return -1;
	int i;
	int j;
	for(i=0;i<height;i++) {
		for(j=0;j<width;j++) {
			TextureSetColor16(texptr[0],No1Sx + j,No1Sy + i,TextureGetColor16(texptr[1],No2Sx + j,No2Sy + i));
		}
	}
	return 0;
}
int	TextureCopy32(int TextureNo1,int TextureNo2,int No1Sx,int No1Sy,int No2Sx,int No2Sy, int width, int height)
{
	//if (texarray[TextureNo1].flags != TEXTUREFLAGS_XXXX ) return -1;
	//if (texarray[TextureNo2].flags != TEXTUREFLAGS_XXXX ) return -2;
	DXPTEXTURE *texptr[2];
	texptr[0] = GraphHandle2Ptr(TextureNo1);
	texptr[1] = GraphHandle2Ptr(TextureNo2);
	if(texptr[0] == NULL)return -1;
	if(texptr[1] == NULL)return -1;
	int i;
	int j;
	for(i=0;i<height;i++) {
		for(j=0;j<width;j++) {
			TextureSetColor32(texptr[0],No1Sx + j,No1Sy + i,TextureGetColor32(texptr[1],No2Sx + j,No2Sy + i));
		}
	}
	return 0;
}

int LoadDivGraph(const char *FileName , int AllNum , int XNum , int YNum , int XSize , int YSize , int *HandleBuf )
{
	int res = -1;

	if(res == -1)res = LoadPngImage(FileName);
	if(res == -1)res = LoadJpegImage(FileName);
	//PSPのメモリの少なさから考えると先にDiv後の領域（XSizeｘYSizeｘカラーフォーマットｘAllNum）
	//を確保したほうが歯抜けになりにくい
	//でもとりあえずは読み込まないと画像のフォーマットがわからない
	//妥協案としては
	//（速度が速い方）
	//LoadPngImageもしくはLoadJpegImageロードしたあと
	//Div後の領域を確保してDivした後元の画像を破棄
	//（速度が遅い方）
	//LoadPngImageもしくはLoadJpegImageロードしたあと
	//カラーフォーマットだけ取得した後一旦画像を破棄
	//Div後の領域を確保して再度画像を読み込みdivしてから破棄

	if(res == -1)return -1;
	//int Wpsm = texarray[res].psm;
	int i;
	int j;
	//for(i=0;i<AllNum;i++)
	//{
	//	HandleBuf[i] = MakeGraph(XSize,YSize,GU_PSM_8888);
	//この関数でべき乗に確保してるようなので縦横サイズをそのまま渡してる
	//}
	//安全策はとりあえず省略（メモリ取れなかったら・・・
	//まあPSP上でPC用素材をそのまま使うというのがそもそもだし
	int k;
	int wres;
	for(i=0;i<YNum;i++)
	{
		for(j=0;j<XNum;j++)
		{
			k = i*XNum + j;
			wres = MakeGraph(XSize,YSize,GU_PSM_8888);
			TextureCopy32(wres,res,0,0,j*XSize,i*YSize,XSize,YSize);
			if(gusettings.flags[0] & GPUSETTINGFLAGS_0_CREATESWIZZLEDGRAPH)
				SwizzleGraph(wres);
			if(gusettings.flags[0] & GPUSETTINGFLAGS_0_CREATEVRAMGRAPH)
				MoveGraphToVRAM(wres);
			HandleBuf[k] = wres;
		}
	}
	DeleteGraph(res);
	sceKernelDcacheWritebackAll();
	return res == -1 ? -1 : 0;
}

#else
int LoadDivGraph( const char *FileName, int AllNum, int XNum, int YNum, int XSize, int YSize, int *HandleBuf)
{
	DXPTEXTURE2 *tex = NULL;
	int i;
	if(HandleBuf == NULL)return -1;
	if(AllNum > XNum * YNum)AllNum = XNum * YNum;
	for(i = 0;i < AllNum;++i)HandleBuf[i] = -1;
	if(AllNum <= 0 || XNum <= 0 || YNum <= 0 || XSize <= 0 || YSize <= 0)return -1;
	if(tex == NULL)tex = LoadPngImage(FileName);
	if(tex == NULL)tex = LoadJpegImage(FileName);
	if(tex == NULL)return -1;
	for(i = 0;i < AllNum;++i)
	{
		int x,y;
		x = XSize * (i % XNum);
		y = YSize * (i / XNum);
		if(x > tex->umax || y > tex->vmax || x + XSize > tex->umax || y + YSize > tex->vmax)continue;
		DXPGRAPHDATA *gptr = (DXPGRAPHDATA*)MALLOC(sizeof(DXPGRAPHDATA));
		if(gptr == NULL)break;
		gptr->tex = tex;
		++tex->refcount;
		gptr->u0 = x;
		gptr->v0 = y;
		gptr->u1 = x + XSize;
		gptr->v1 = y + YSize;
		HandleBuf[i] = gptr->handle = GenerateGraphHandle();
		GraphDataList_PushFront(gptr);
	}
	if(i == 0)
	{
		TextureList_Remove(tex);
		FREE(tex->pmemory);
		FREE(tex->ppalette);
		FreeVRAM(tex->pvram);
		FREE(tex);
		return -1;
	}
	if(gusettings.flags[0] & GPUSETTINGFLAGS_0_CREATESWIZZLEDGRAPH)
		SwizzleGraph(HandleBuf[0]);
	if(gusettings.flags[0] & GPUSETTINGFLAGS_0_CREATEVRAMGRAPH)
		MoveGraphToVRAM(HandleBuf[0]);
	return 0;
}
#endif

int PSM2BYTEx2(int psm)
{
	switch(psm)
	{
	case GU_PSM_T4:
		return 1;
	case GU_PSM_T8:
		return 2;
	case GU_PSM_4444:
	case GU_PSM_5551:
	case GU_PSM_5650:
		return 4;
	case GU_PSM_8888:
		return 8;
	default:
		return 0;
	}
}

void swizzle_fast(u8* out, const u8* in, unsigned int width, unsigned int height)
{
   unsigned int blockx, blocky;
   unsigned int j;
 
   unsigned int width_blocks = (width / 16);
   unsigned int height_blocks = (height / 8);
 
   unsigned int src_pitch = (width-16)/4;
   unsigned int src_row = width * 8;
 
   const u8* ysrc = in;
   u32* dst = (u32*)out;
 
   for (blocky = 0; blocky < height_blocks; ++blocky)
   {
      const u8* xsrc = ysrc;
      for (blockx = 0; blockx < width_blocks; ++blockx)
      {
         const u32* src = (u32*)xsrc;
         for (j = 0; j < 8; ++j)//16byte幅で高さ8の情報を線形に転送
         {
            *(dst++) = *(src++);
            *(dst++) = *(src++);
            *(dst++) = *(src++);
            *(dst++) = *(src++);
            src += src_pitch;
         }
         xsrc += 16;
     }
     ysrc += src_row;
   }
}
void unswizzle_fast(u8* out, const u8* in, unsigned int width, unsigned int height)
{
   unsigned int blockx, blocky;
   unsigned int j;
 
   unsigned int width_blocks = (width / 16);
   unsigned int height_blocks = (height / 8);
 
   unsigned int src_pitch = (width-16)/4;
   unsigned int src_row = width * 8;
 
   u8* ysrc = out;
   u32* dst = (u32*)in;
 
   for (blocky = 0; blocky < height_blocks; ++blocky)
   {
      u8* xsrc = ysrc;
      for (blockx = 0; blockx < width_blocks; ++blockx)
      {
         u32* src = (u32*)xsrc;
         for (j = 0; j < 8; ++j)
         {
            *(src++) = *(dst++);
            *(src++) = *(dst++);
            *(src++) = *(dst++);
            *(src++) = *(dst++);
            src += src_pitch;
         }
         xsrc += 16;
     }
     ysrc += src_row;
   }
}

int SwizzleGraph(int gh)	/*指定されたグラフィックをSwizzleする。*/
{
	GUFINISH
	void *buf;
	s32 size;
	DXPGRAPHDATA *gptr = GraphHandle2Ptr(gh);
	if(gptr == NULL)return -1;
	if(gptr->tex == NULL)return -1;
	if(gptr->tex->swizzledflag){return 0;}
	if(gptr->tex == gusettings.rendertarget)return -1;
	if((size = GraphSize2DataSize(gptr->tex->pitch,gptr->tex->height,gptr->tex->psm)) <= 0){return -1;}
	if((buf = MEMALIGN(16,size)) == NULL){return -1;}
	if(gptr->tex->vramflag)
	{
		swizzle_fast(buf,sceGeEdramGetAddr() + gptr->tex->pvram->offset, PSM2BYTEx2(gptr->tex->psm) * gptr->tex->pitch / 2,gptr->tex->height);
		memcpy(sceGeEdramGetAddr() + gptr->tex->pvram->offset,buf,size);
		FREE(buf);
	}else
	{
		swizzle_fast(buf,gptr->tex->pmemory, PSM2BYTEx2(gptr->tex->psm) * gptr->tex->pitch >> 1,gptr->tex->height);
		FREE(gptr->tex->pmemory);
		gptr->tex->pmemory = buf;
	}
	gptr->tex->swizzledflag = 1;
	sceKernelDcacheWritebackAll();
	gptr->tex->reloadflag = 1;
	return 0;
}
int UnswizzleGraph(int gh)	/*指定されたグラフィックをUnswizzleする。*/
{
	GUFINISH
	void *buf;
	s32 size;
	DXPGRAPHDATA *gptr = GraphHandle2Ptr(gh);
	if(gptr == NULL)return -1;
	if(gptr->tex == NULL)return -1;
	if(!gptr->tex->swizzledflag){return 0;}
	if((size = GraphSize2DataSize(gptr->tex->pitch,gptr->tex->height,gptr->tex->psm)) <= 0){return -1;}
	if((buf = MEMALIGN(16,size)) == NULL){return -1;}
	if(gptr->tex->vramflag)
	{
		unswizzle_fast(buf,sceGeEdramGetAddr() + gptr->tex->pvram->offset, PSM2BYTEx2(gptr->tex->psm) * gptr->tex->pitch / 2,gptr->tex->height);
		memcpy(sceGeEdramGetAddr() + gptr->tex->pvram->offset,buf,size);
		FREE(buf);
	}else
	{
		unswizzle_fast(buf,gptr->tex->pmemory, PSM2BYTEx2(gptr->tex->psm) * gptr->tex->pitch >> 1,gptr->tex->height);
		FREE(gptr->tex->pmemory);
		gptr->tex->pmemory = buf;
	}
	gptr->tex->swizzledflag = 0;
	sceKernelDcacheWritebackAll();
	gptr->tex->reloadflag = 1;
	return 0;
}

int MoveGraphToVRAM(int gh)	/*グラフィックをVRAMに移動する。VRAM不足の場合はなにもしない。*/
{
	GUFINISH
	s32 size;
	DXPGRAPHDATA *gptr = GraphHandle2Ptr(gh);
	if(gptr == NULL)return -1;
	if(gptr->tex == NULL)return -1;
	if(gptr->tex->vramflag)return 0;
	if((size = GraphSize2DataSize(gptr->tex->pitch,gptr->tex->height,gptr->tex->psm)) <= 0)return -1;
	if((gptr->tex->pvram = AllocVRAM(size,0)) == NULL)return -1;
	memcpy(sceGeEdramGetAddr() + gptr->tex->pvram->offset,gptr->tex->pmemory,size);
	sceKernelDcacheWritebackAll();
	FREE(gptr->tex->pmemory);
	gptr->tex->pmemory = NULL;
	gptr->tex->vramflag = 1;
	gptr->tex->reloadflag = 1;
	return 0;
}

int MoveGraphToDDR(int gh)	/*グラフィックをメインメモリに移動する*/
{
	GUFINISH
	s32 size;
	DXPGRAPHDATA *gptr = GraphHandle2Ptr(gh);
	if(gptr == NULL)return -1;
	if(gptr->tex == NULL)return -1;
	if(!gptr->tex->vramflag)return 0;
	if(gptr->tex == gusettings.rendertarget)return -1;
	if((size = GraphSize2DataSize(gptr->tex->pitch,gptr->tex->height,gptr->tex->psm)) <= 0)return -1;
	if((gptr->tex->pmemory = MEMALIGN(16,size)) == NULL)return -1;
	
	memcpy(gptr->tex->pmemory,sceGeEdramGetAddr() + gptr->tex->pvram->offset,size);
	sceKernelDcacheWritebackAll();
	FreeVRAM(gptr->tex->pvram);
	gptr->tex->pvram = NULL;
	gptr->tex->vramflag = 0;
	gptr->tex->reloadflag = 1;
	return 0;
}

//int LoadRAWData(const char *FileName,int SizeX,int SizeY,int Format)
//{
//	int i;
//	int fh;
//	int gh;
//	int depth;
//	if((fh = FileRead_open(FileName)) == -1)return -1;
//	if((gh = MakeGraph(SizeX,SizeY,Format)) == -1)
//	{
//		FileRead_close(fh);
//		return -1;
//	}
//	DXPTEXTURE *texptr = GraphHandle2Ptr(gh);
//	if(texptr == NULL)
//	{
//		DeleteGraph(gh);
//		FileRead_close(fh);
//		return -1;
//	}
//	depth = Format == GU_PSM_8888 ? 4 : 2;
//	for(i = 0;i < texptr->mv;++i)
//	{
//		FileRead_seek(fh,i * SizeX * depth,SEEK_SET);
//		FileRead_read(texptr->pmemory + i * texptr->pitch * depth,i * texptr->pitch * depth,fh);
//	}
////	FileRead_read(texarray[gh].pmemory,texarray[gh].mu * texarray[gh].mv * depth,fh);
//	FileRead_close(fh);
//	sceKernelDcacheWritebackAll();
////	sceKernelDcacheWritebackRange(texarray[gh].pmemory,texarray[gh].pitch * texarray[gh].height * depth);
//
//	/*必要があり、かつ可能ならばswizzleする*/
//	if(gusettings.flags[0] & GPUSETTINGFLAGS_0_CREATESWIZZLEDGRAPH)
//	{
//		//swizzleする
//		SwizzleGraph(gh);
//	}
//	/*必要があり、かつ可能ならばVRAMに転送する*/
//	if(gusettings.flags[0] & GPUSETTINGFLAGS_0_CREATEVRAMGRAPH)
//	{
//		MoveGraphToVRAM(gh);
//	}
//	return gh;
//}

int ConvertGraphFormat(int gh,int psm)
/*
DDR上のSwizzleなしグラフィックにする。
処理
VRAMグラフィックだったりSwizzleグラフィックだったりなら元に戻す。

スーパービット演算タイム。デバッグする気がおきませんｗ
*/
{
	int x,y;
	char swizzleflag = 0;
	char vramflag = 0;
	int size;
	void* buf;
	//GraphicHandleCheck(gh);
	DXPTEXTURE2 *texptr = GraphHandle2TexPtr(gh);
	if(texptr == NULL)return -1;
	if(psm == texptr->psm)return 0;
	if(psm < 0 || psm > 3 || texptr->psm < 0 || texptr->psm > 3)return -1;	/*変換元と変換先のどちらかが4444,5551,5650,8888以外のフォーマットなら失敗*/
	size = GraphSize2DataSize(texptr->pitch,texptr->height,psm);
	if((buf = MEMALIGN(16,size)) == NULL)return -1;
	if(texptr->vramflag)
	{
		MoveGraphToDDR(gh);
		vramflag = 1;
	}
	if(texptr->swizzledflag)
	{
		UnswizzleGraph(gh);
		swizzleflag = 1;
	}
	switch(texptr->psm)
	{
	case GU_PSM_4444:
		goto SRC4444;
	case GU_PSM_5551:
		goto SRC5551;
	case GU_PSM_5650:
		goto SRC5650;
	case GU_PSM_8888:
		goto SRC8888;
	default:
		return -1;
	}
SRC4444:
	switch(psm)
	{
	case GU_PSM_5551:
		for(y = 0;y < texptr->height;++y)
		{
			for(x = 0;x < texptr->pitch;++x)
			{
				u16 color = *(((u16*)texptr->pmemory) + y * texptr->pitch + x);
				color = (color & 0x8000) | ((color & 0x0f00) << 3) | ((color & 0x00f0) << 2) | ((color & 0x000f) << 1)
					| ((color & 0x0800) >> 1) | ((color & 0x0080) >> 2) | ((color & 0x0008) >> 3);
				*(((u16*)buf) + y * texptr->pitch + x) = color;
			}
		}
		goto ENDPROCESS;
	case GU_PSM_5650:
		for(y = 0;y < texptr->height;++y)
		{
			for(x = 0;x < texptr->pitch;++x)
			{
				u16 color = *(((u16*)texptr->pmemory) + y * texptr->pitch + x);
				color = ((color & 0x0f00) << 4) | ((color & 0x00f0) << 3) | ((color & 0x000f) << 1)
					| (color & 0x0800) | ((color & 0x00c0) >> 1) | ((color & 0x0008) << 1);
				*(((u16*)buf) + y * texptr->pitch + x) = color;
			}
		}
		texptr->alphabit = 0;
		goto ENDPROCESS;
	case GU_PSM_8888:
		for(y = 0;y < texptr->height;++y)
		{
			for(x = 0;x < texptr->pitch;++x)
			{
				u32 color = *(((u16*)texptr->pmemory) + y * texptr->pitch + x);
				color = ((color & 0x0000f000) << 16) | ((color & 0x0000f000) << 12)
					| ((color & 0x00000f00) << 12) | ((color & 0x00000f00) << 8)
					| ((color & 0x000000f0) << 8) | ((color & 0x000000f0) << 4)
					| ((color & 0x0000000f) << 4) | (color & 0x0000000f);
				*(((u32*)buf) + y * texptr->pitch + x) = color;
			}
		}
		goto ENDPROCESS;
	}
SRC5551:
	switch(psm)
	{
	case GU_PSM_4444:
		for(y = 0;y < texptr->height;++y)
		{
			for(x = 0;x < texptr->pitch;++x)
			{
				u16 color = *(((u16*)texptr->pmemory) + y * texptr->pitch + x);
				color = (color & 0x8000) | ((color & 0x8000) >> 1) | ((color & 0x8000) >> 2) | ((color & 0x8000) >> 3)
					| ((color & 0x7800) >> 3) | ((color & 0x03c0) >> 2) | ((color & 0x001e) >> 1);
				*(((u16*)buf) + y * texptr->pitch + x) = color;
			}
		}
		goto ENDPROCESS;
	case GU_PSM_5650:
		for(y = 0;y < texptr->height;++y)
		{
			for(x = 0;x < texptr->pitch;++x)
			{
				u16 color = *(((u32*)texptr->pmemory) + y * texptr->pitch + x);
				color = ((color & 0x7ef0) << 1) | (color & 0x01f)
					| ((color & 0x0200) >> 4);
				*(((u16*)buf) + y * texptr->pitch + x) = color;
			}
		}
		texptr->alphabit = 0;
		goto ENDPROCESS;
	case GU_PSM_8888:
		for(y = 0;y < texptr->height;++y)
		{
			for(x = 0;x < texptr->pitch;++x)
			{
				u32 color = *(((u16*)texptr->pmemory) + y * texptr->pitch + x);
				color = ((color & 0x8000) << 16) | ((color & 0x8000) << 15) | ((color & 0x8000) << 14) | ((color & 0x8000) << 13)
					 | ((color & 0x8000) << 12) | ((color & 0x8000) << 11) | ((color & 0x8000) << 10) | ((color & 0x8000) << 9)
					 | ((color & 0x7c00) << 9) | ((color & 0x03e0) << 6) | ((color & 0x001f) << 3)
					 | ((color & 0x7000) << 4) | ((color & 0x0380) << 1) | ((color & 0x001c) >> 2);
				*(((u32*)buf) + y * texptr->pitch + x) = color;
			}
		}
		goto ENDPROCESS;
	}
SRC5650:
	switch(psm)
	{
	case GU_PSM_4444:
		for(y = 0;y < texptr->height;++y)
		{
			for(x = 0;x < texptr->pitch;++x)
			{
				u16 color = *(((u16*)texptr->pmemory) + y * texptr->pitch + x);
				color = 0xf000 | ((color & 0xf000) >> 4) | ((color & 0x0780) >> 3) | ((color & 0x001e) >> 1);
				*(((u16*)buf) + y * texptr->pitch + x) = color;
			}
		}
		goto ENDPROCESS;
	case GU_PSM_5551:
		for(y = 0;y < texptr->height;++y)
		{
			for(x = 0;x < texptr->pitch;++x)
			{
				u16 color = *(((u16*)texptr->pmemory) + y * texptr->pitch + x);
				color = 0x8000 | ((color & 0xffc0) >> 1) | (color & 0x001f);
				*(((u16*)buf) + y * texptr->pitch + x) = color;
			}
		}
		goto ENDPROCESS;
	case GU_PSM_8888:
		for(y = 0;y < texptr->height;++y)
		{
			for(x = 0;x < texptr->pitch;++x)
			{
				u32 color = *(((u16*)texptr->pmemory) + y * texptr->pitch + x);
				color = 0xff000000 | ((color & 0x00008f00) << 8) | ((color & 0x07e0) << 5) | ((color & 0x0000001f) << 3)
					| ((color & 0x0000e000) << 3) | ((color & 0x00000600) >> 1) | ((color & 0x0000001c) >> 2);
				*(((u32*)buf) + y * texptr->pitch + x) = color;
			}
		}
		goto ENDPROCESS;
	}
SRC8888:
	switch(psm)
	{
	case GU_PSM_4444:
		for(y = 0;y < texptr->height;++y)
		{
			for(x = 0;x < texptr->pitch;++x)
			{
				u32 color = *(((u32*)texptr->pmemory) + y * texptr->pitch + x);
				color = ((color & 0xf0000000) >> 16) | ((color & 0x00f00000) >> 12) | ((color & 0x0000f000) >> 8) | ((color & 0x000000f0) >> 4);
				*(((u16*)buf) + y * texptr->pitch + x) = (u16)(color & 0x0000ffff);
			}
		}
		goto ENDPROCESS;
	case GU_PSM_5551:
		for(y = 0;y < texptr->height;++y)
		{
			for(x = 0;x < texptr->pitch;++x)
			{
				u32 color = *(((u32*)texptr->pmemory) + y * texptr->pitch + x);
				color = ((color & 0x80000000) >> 16) | ((color & 0x00f80000) >> 9) | ((color & 0x0000f800) >> 6) | ((color & 0x000000f8) >> 3);
				*(((u16*)buf) + y * texptr->pitch + x) = (u16)(color & 0x0000ffff);
			}
		}
		goto ENDPROCESS;
	case GU_PSM_5650:
		for(y = 0;y < texptr->height;++y)
		{
			for(x = 0;x < texptr->pitch;++x)
			{
				u32 color = *(((u32*)texptr->pmemory) + y * texptr->pitch + x);
				color = ((color & 0x00f80000) >> 8) | ((color & 0x0000fc00) >> 5) | ((color & 0x000000f8) >> 3);
				*(((u16*)buf) + y * texptr->pitch + x) = (u16)(color & 0x0000ffff);
			}
		}
		texptr->alphabit = 0;
		goto ENDPROCESS;
	}
ENDPROCESS:
	FREE(texptr->pmemory);
	texptr->pmemory = buf;
	texptr->psm = psm;

	if(swizzleflag)
	{
		SwizzleGraph(gh);
	}
	if(vramflag)
	{
		MoveGraphToVRAM(gh);
	}
	return 0;
}

//#define	DXP_NOUSE_LIBJPEG
#ifndef DXP_NOUSE_LIBJPEG

// (殆ど jdatasrc.c の流用)
//#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"

// ＪＰＥＧ読みこみエラー時処理ルーチン用構造体
typedef struct my_error_mgr {
	struct jpeg_error_mgr pub;	// 標準エラーデータ

	jmp_buf setjmp_buffer;		// ロングジャンプ用構造体
} *my_error_ptr ;

// エラー時に呼ばれる関数
void my_error_exit( j_common_ptr cinfo )
{
	// cinfo->errが示す標準エラーデータの先頭アドレスをmy_error_mgr構造体の先頭アドレスに変換
	my_error_ptr myerr = (my_error_ptr) cinfo->err;
	// すいませんよくわかりません、とりあえずエラーメッセージ標準関数？
	(*cinfo->err->output_message) (cinfo);
	// ユーザーが決めたエラー処理コードの位置まで飛ぶ
	longjmp( myerr->setjmp_buffer, 1 ) ;
}


// 汎用データ形式の転送用構造体
typedef struct
{
	struct jpeg_source_mgr pub;		/* public fields */

	JOCTET *buffer;					/* start of buffer */
	boolean start_of_file;			/* have we gotten any data yet? */

	STREAMDATA *Data ;				// 汎用データ形式読み込み処理用構造体
	int DataSize ;					// データのサイズ
} my_source_general_mgr;

typedef my_source_general_mgr	*my_src_general_ptr;

#define INPUT_BUF_SIZE		(4096)	// 作業用バッファのサイズ

// 読み込むソースを初期化する関数
METHODDEF(void)
init_source_general( j_decompress_ptr cinfo )
{
	my_src_general_ptr src = (my_src_general_ptr) cinfo->src;

	// ファイル読み込み開始のフラグを立てる
	src->start_of_file = TRUE;
}

// データバッファにデータを転送する
METHODDEF(boolean)
fill_input_buffer_general (j_decompress_ptr cinfo)
{
	my_src_general_ptr src = (my_src_general_ptr) cinfo->src;
	size_t nbytes;

	// 転送するデータの量をコピーする
	nbytes = ( src->DataSize - STTELL( src->Data ) < INPUT_BUF_SIZE ) ?
			 src->DataSize - STTELL( src->Data ) : INPUT_BUF_SIZE ;
	if( nbytes != 0 )
	{
		STREAD( src->buffer, nbytes, 1, src->Data ) ;
	}

	// 読み込みに失敗したらエラー
	if( nbytes <= 0 )
	{
		if (src->start_of_file)	/* Treat empty input file as fatal error */
			ERREXIT(cinfo, JERR_INPUT_EMPTY);
		WARNMS(cinfo, JWRN_JPEG_EOF);

		/* Insert a fake EOI marker */
	    src->buffer[0] = (JOCTET) 0xFF;
		src->buffer[1] = (JOCTET) JPEG_EOI;
		nbytes = 2;
	}

	// その他の処理
	src->pub.next_input_byte = src->buffer;
	src->pub.bytes_in_buffer = nbytes;
	src->start_of_file = FALSE;

	return TRUE;
}

// 指定されたサイズのデータをスキップする処理
METHODDEF(void)
skip_input_data_general( j_decompress_ptr cinfo, long num_bytes)
{
	my_src_general_ptr src = (my_src_general_ptr) cinfo->src;

	// データスキップ処理
	if( num_bytes > 0 )
	{
		while( num_bytes > (long) src->pub.bytes_in_buffer )
		{
			num_bytes -= (long) src->pub.bytes_in_buffer;
			(void) fill_input_buffer_general( cinfo ) ;
		}
		src->pub.next_input_byte += (size_t) num_bytes;
		src->pub.bytes_in_buffer -= (size_t) num_bytes;
	}
}

// データを閉じるときの処理
METHODDEF(void)
term_source_general( j_decompress_ptr cinfo )
{
  /* no work necessary here */
}

// 汎用データ読み込み処理からデータを読み込むようにする実際の設定を行う関数
GLOBAL(void)
jpeg_general_src (j_decompress_ptr cinfo, STREAMDATA *Data )
{
	my_src_general_ptr src;

	// まだＪＰＥＧデータを一時的に格納するバッファを確保していなかったら確保する
	if (cinfo->src == NULL)
	{
		/* first time for this JPEG object? */
		cinfo->src = (struct jpeg_source_mgr *)
						(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
						sizeof(my_source_general_mgr));
		src = (my_src_general_ptr) cinfo->src;
		src->buffer = (JOCTET *)
					(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
					INPUT_BUF_SIZE * sizeof(JOCTET));
	}

	// 関数ポインタなどをセットする
	src = (my_src_general_ptr) cinfo->src;
	src->pub.init_source			= init_source_general ;
	src->pub.fill_input_buffer		= fill_input_buffer_general ;
	src->pub.skip_input_data		= skip_input_data_general ;
	src->pub.resync_to_restart		= jpeg_resync_to_restart ; /* use default method */
	src->pub.term_source			= term_source_general ;

	src->Data = Data ;

	// 現在のファイルポインタから終端までのサイズを取得する
	{
		long pos ;
		pos = STTELL( src->Data ) ;
		STSEEK( src->Data, 0, STREAM_SEEKTYPE_END ) ;
		src->DataSize = STTELL( src->Data ) - pos ;
		STSEEK( src->Data, pos, STREAM_SEEKTYPE_SET ) ;
	}

	src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
	src->pub.next_input_byte = NULL; /* until buffer loaded */
}

#ifdef DX_TEST	// テスト用
extern LONGLONG time2;
#endif




extern int LoadJpegImage(const char *FileName)
{
	STREAMDATA src;
	STREAMDATA *Src = &src;
	if(SetupSTREAMDATA(FileName,Src) == -1)
	{
		return -1;
	}
	struct jpeg_decompress_struct cinfo ;
	struct my_error_mgr jerr ;
	JSAMPARRAY buffer ;
	int InPitch ;
	int i ;

	// 通常ＪＰＥＧエラールーチンのセットアップ
	memset( &cinfo, 0, sizeof( cinfo ) );
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	if( setjmp( jerr.setjmp_buffer ) )
	{
		jpeg_destroy_decompress( &cinfo );
		STCLOSE(Src);
		return -1;
	}

#ifdef DX_TEST	// テスト用
	time2 = ST_GetNowHiPerformanceCount();
#endif
	// cinfo情報のアロケートと初期化を行う
	jpeg_create_decompress(&cinfo);

	// 汎用データ読み込み処理からデータを読み込む設定を行う
	jpeg_general_src( &cinfo, Src ) ;

	// ＪＰＥＧファイルのパラメータ情報の読みこみ
	(void)jpeg_read_header(&cinfo, TRUE);

	// ＪＰＥＧファイルの解凍の開始
	(void)jpeg_start_decompress(&cinfo);

	if(cinfo.output_components != 1 && cinfo.out_color_components != 3)
	{
		jpeg_destroy_decompress( &cinfo );
		STCLOSE(Src);
		return -1 ;
	}
	/*グラフィックハンドルを作る。*/
	int gh = MakeGraph(cinfo.output_width,cinfo.output_height,GU_PSM_8888);
	if(gh == -1)
	{
		//エラー処理
		jpeg_destroy_decompress( &cinfo );
		STCLOSE(Src);
		return -1 ;
	}

	// １ライン当たりのデータバイト数を計算
	InPitch = cinfo.output_width * cinfo.output_components ;

	// データバッファの確保
	buffer = (*cinfo.mem->alloc_sarray)
				((j_common_ptr) &cinfo, JPOOL_IMAGE, InPitch, 1 );
	
	// 画像データの読みこみ
	int row = 0;
	while( cinfo.output_scanline < cinfo.output_height && row < texarray[gh].mv)
	{
		(void) jpeg_read_scanlines(&cinfo, buffer, 1);

		// データを出力データに変換して、またはそのまま転送
		for(i = 0;i < texarray[gh].mu;++i)
		{
			if(cinfo.output_components == 1)
			{/*グレースケール*/
				*((unsigned char*)(texarray[gh].pmemory) + texarray[gh].pitch * row * 4 + i * 4 + 0) = 0xff;
				*((unsigned char*)(texarray[gh].pmemory) + texarray[gh].pitch * row * 4 + i * 4 + 1) = *(buffer[0] + i);
				*((unsigned char*)(texarray[gh].pmemory) + texarray[gh].pitch * row * 4 + i * 4 + 2) = *(buffer[0] + i);
				*((unsigned char*)(texarray[gh].pmemory) + texarray[gh].pitch * row * 4 + i * 4 + 3) = *(buffer[0] + i);
			}
			else
			{/*ABGR*/
				*((unsigned char*)(texarray[gh].pmemory) + texarray[gh].pitch * row * 4 + i * 4 + 3) = 0xff;					//A
				*((unsigned char*)(texarray[gh].pmemory) + texarray[gh].pitch * row * 4 + i * 4 + 2) = *(buffer[0] + i * 3 + 2);//B
				*((unsigned char*)(texarray[gh].pmemory) + texarray[gh].pitch * row * 4 + i * 4 + 1) = *(buffer[0] + i * 3 + 1);//G
				*((unsigned char*)(texarray[gh].pmemory) + texarray[gh].pitch * row * 4 + i * 4 + 0) = *(buffer[0] + i * 3 + 0);//R
			}
		}
		++row;
	}

	// 解凍処理の終了
	(void) jpeg_finish_decompress(&cinfo);
	// cinfo構造体の解放
	jpeg_destroy_decompress(&cinfo);
#ifdef DX_TEST	// テスト用
	time2 = ST_GetNowHiPerformanceCount() - time2 ;
#endif

	// 終了
//	GraphicNormalize(texi);/*グラフィックを正規化（っていうの？）*/
	STCLOSE(Src);
	return gh ;
}


#else
// ＪＰＥＧ画像の読みこみ PSP内部のライブラリを使うバージョン
DXPTEXTURE2* LoadJpegImage(const char *FileName)
{
	if(sceJpegInitMJpeg() < 0)return NULL;
	if(sceJpegCreateMJpeg(512,512) < 0)
	{
		sceJpegFinishMJpeg();
		return NULL;
	}
	int size = FileRead_size(FileName);
	u8* rgbabuf = (u8*)malloc(4*512*512);
	u8* databuf = (u8*)malloc(size);
	if(databuf == NULL || rgbabuf == NULL)
	{
		free(databuf);
		free(rgbabuf);
		sceJpegDeleteMJpeg();
		sceJpegFinishMJpeg();
		return NULL;
	}
	int fh = FileRead_open(FileName);
	if(fh == -1)
	{
		free(databuf);
		free(rgbabuf);
		sceJpegDeleteMJpeg();
		sceJpegFinishMJpeg();
		return NULL;
	}
	FileRead_read(databuf,size,fh);
	FileRead_close(fh);
	int res = sceJpegDecodeMJpeg(databuf,size,rgbabuf,0);
	free(databuf);
	sceJpegDeleteMJpeg();
	sceJpegFinishMJpeg();
	if(res < 0)
	{
		free(rgbabuf);
		return NULL;
	}
	int height = res >> 16;
	int width = res & 0x0000ffff;
	DXPTEXTURE2 *texptr = MakeTexture(width,height,DXP_FMT_8888);
	if(texptr == NULL)
	{
		free(rgbabuf);
		return NULL;
	}
//	memcpy(texptr->pmemory,rgbabuf,4 * height * width);//この1行なぜ存在するのか理解不能。消し忘れかもｗ動作に影響なければ削除。
	int i;
	for(i = 0;i < height;++i)
		memcpy(((u32*)texptr->pmemory) + texptr->pitch * i,((u32*)rgbabuf) + 512 * i,width * 4);
	free(rgbabuf);
	return texptr;
}
#endif


#ifndef DX_NON_PNGREAD

// 汎用データ読み込み処理からの読み込みをするためのデータ型
typedef struct tagPNGGENERAL
{
	STREAMDATA *Data ;
	unsigned int DataSize ;
} PNGGENERAL ;

// 汎用データ読み込み処理からデータを読み込むコールバック関数
static void png_general_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	PNGGENERAL *PGen ;

	PGen = (PNGGENERAL *)CVT_PTR(png_ptr->io_ptr) ;

	// 残りのサイズが足りなかったらエラー
	if( PGen->DataSize - STTELL( PGen->Data ) < length )
	{
		png_error(png_ptr, "Read Error");
	}
	else
	{
		STREAD( data, length, 1, PGen->Data ) ;
	}
}

// 汎用データ読み込み処理からの読み込みを設定する関数
int png_general_read_set( png_structp png_ptr, PNGGENERAL *PGen, STREAMDATA *Data )
{
	PGen->Data = Data ;

	// 現在のファイルポインタから終端までのサイズを取得する
	{
		long pos ;
		pos = STTELL( PGen->Data ) ;
		STSEEK( PGen->Data, 0, STREAM_SEEKTYPE_END ) ;
		PGen->DataSize = STTELL( PGen->Data ) - pos ;
		STSEEK( PGen->Data, pos, STREAM_SEEKTYPE_SET ) ;
	}

	// コールバック関数のセット
	png_set_read_fn( png_ptr, PGen, png_general_read_data ) ;

	// 終了
	return 0 ;
}

// ＰＮＧ画像の読みこみ
DXPTEXTURE2* LoadPngImage(const char *FileName)
{
//	BASEIMAGE *Image ;
	STREAMDATA src;
	STREAMDATA *Src = &src;
	if(SetupSTREAMDATA(FileName,Src) == -1)
	{
		STCLOSE(Src);
		return NULL;
	}
	png_bytep *row_pointers;
	unsigned int row, rowbytes ;
	u8 Expand ;
	PNGGENERAL PGen ;
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int sig_read = 0;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;
	int i ;
	png_bytep BufPoint ;

	Expand = 0;

	// ＰＮＧ管理情報の作成
	if( ( png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL ) ) == NULL )
	{
		STCLOSE(Src);
		return NULL ;
	}
	// 画像情報構造体の作成
	if( ( info_ptr = png_create_info_struct( png_ptr ) ) == NULL ) 
	{
		STCLOSE(Src);
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		return NULL ;
	}

	// エラー時の処理セットアップおよび処理部
	if( setjmp( png_jmpbuf( png_ptr ) ) )
	{
		STCLOSE(Src);
		png_destroy_read_struct( &png_ptr, &info_ptr, (png_infopp)NULL );
		return NULL ;
	}

	// 汎用データ読み込み処理から読み込む場合の設定を行う
	png_general_read_set( png_ptr, &PGen, Src ) ;

	// 設定処理郡
	png_set_sig_bytes(		png_ptr, sig_read ) ;												// よくわから無い処理(汗)
	png_read_info(			png_ptr, info_ptr );												// 画像情報を得る
	png_get_IHDR(			png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,		// 画像の基本ステータスを取得する
							&interlace_type, NULL, NULL ) ;
	png_set_strip_16(		png_ptr ) ;															// １６ビットの画像でも８ビットで収納するように設定
//	if( BmpFlag == TRUE )										png_set_strip_alpha( png_ptr ) ;// アルファチャンネル無視
	png_set_packing(		png_ptr ) ;															// １バイト以下のパレット画像をバイト単位で展開するよう指定
	png_set_packswap(		png_ptr ) ;															// よくわから無い処理

//	if( color_type == PNG_COLOR_TYPE_PALETTE )					png_set_expand( png_ptr ) ;		// パレット使用画像データの自動展開指定

	// tRNS(一種のカラーキー)付き画像か８ビット以下のグレースケール画像の場合は
	// 出力画像のピクセルフォーマットをフルカラーにする
	//ただし、パレット画像でtRNSの場合はパレット情報とtRNS情報を合成する。
	if( ( color_type == PNG_COLOR_TYPE_GRAY && bit_depth <= 8 ) ||
		(color_type != PNG_COLOR_TYPE_PALETTE && png_get_valid( png_ptr, info_ptr, PNG_INFO_tRNS ) ))
	{
		png_set_expand( png_ptr );		
		Expand = 1 ;
	}
	png_set_gray_to_rgb(png_ptr);
	png_set_add_alpha(png_ptr,0xff,PNG_FILLER_AFTER);			/*アルファ情報が無い場合は付加*/
//	png_set_bgr(			png_ptr ) ;															// カラー配列をＲＧＢからＢＧＲに反転
	
	// 変更した設定を反映させる
	png_read_update_info(	png_ptr, info_ptr ) ;

	// １ラインあたりに必要なデータ量を得る
	rowbytes = png_get_rowbytes( png_ptr, info_ptr ) ;

	// グラフィックデータを格納するメモリ領域を作成する
	{
		png_bytep BufP ;

		row_pointers = ( png_bytep * )malloc( height * sizeof( png_bytep * ) ) ;
		if( ( BufPoint = ( png_bytep )png_malloc( png_ptr, rowbytes * height ) ) == NULL )	return NULL ;
		BufP = BufPoint ;
		for (row = 0; row < height; row++, BufP += rowbytes )
			row_pointers[row] = BufP ;
	}

	// 画像データの読み込み
	png_read_image( png_ptr, row_pointers );


	DXPTEXTURE2 *texptr;
	// 情報をセットする
	{
		// カラー情報をセットする
		if( color_type == PNG_COLOR_TYPE_PALETTE && Expand == 0)
		{
			//パレットカラーである
			if(bit_depth == 8)
texptr = MakeTexture(width,height,GU_PSM_T8);
			else
texptr = MakeTexture(width,height,GU_PSM_T4);
			if(texptr == NULL)
			{
				//失敗処理
				free( row_pointers ) ;
				png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
				STCLOSE(Src);
				return NULL;
			}

			//画像を実際に使えるようにする
			int bn = 8 / bit_depth;
			for(i = 0;i < texptr->vmax;++i)
				memcpy(((u8*)texptr->pmemory) + texptr->pitch * i / bn,row_pointers[i],texptr->umax / bn);
			png_colorp SrcPalette ;
			int PaletteNum ;
			// パレットを取得
			png_get_PLTE( png_ptr, info_ptr, &SrcPalette, &PaletteNum ) ;

			// パレットの数が２５６以上だった場合は２５６に補正
			if( PaletteNum < 256 ) PaletteNum = 256 ;

			for(i = 0;i < PaletteNum;++i)
			{
				texptr->ppalette->data[i] = 0xff000000 |
					(((u32)SrcPalette[i].blue & 0x000000ff) << 16) |
					(((u32)SrcPalette[i].green & 0x000000ff) << 8) |
					(((u32)SrcPalette[i].red & 0x000000ff) << 0);
			}

			//tRNS情報取得
			if(png_get_valid(png_ptr,info_ptr,PNG_INFO_tRNS))
			{
				png_bytep trans;
				int num_trans;
				png_color_16p trans_values;
				png_get_tRNS(png_ptr,info_ptr,&trans,&num_trans,&trans_values);
				if(num_trans > 256)num_trans = 256;
				for(i = 0;i < num_trans;++i)
				{
					texptr->ppalette->data[i] &= (((u32)trans[i]) << 24) | 0x00ffffff;
				}
			}
			ClearDrawScreen();printfDx("PLTE!\n");ScreenFlip();
		}
		else
		if( color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA )
		{
			texptr = MakeTexture(width,height,GU_PSM_8888);
			if(texptr == NULL)
			{
				//失敗処理
				free( row_pointers ) ;
				png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
				STCLOSE(Src);
				return NULL;
			}
			//画像を実際に使えるようにする
			int i;
			int bn = 8 / bit_depth;
			for(i = 0;i < texptr->vmax;++i)
				memcpy(((u8*)texptr->pmemory) + texptr->pitch * 4 * i / bn,row_pointers[i],texptr->umax  * 4 / bn);
		}
		else
		{
			//フルカラーである
			texptr = MakeTexture(width,height,GU_PSM_8888);
			if(texptr == NULL)
			{
				//失敗処理
				free( row_pointers ) ;
				png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
				STCLOSE(Src);
				return NULL;
			}
			//画像を実際に使えるようにする
			int i;
			int bn = 8 / bit_depth;
			for(i = 0;i < texptr->vmax;++i)
				memcpy(((u8*)texptr->pmemory) + texptr->pitch * 4 * i / bn,row_pointers[i],texptr->umax * 4 / bn);
		}
	}

	// メモリの解放
	png_free( png_ptr, BufPoint ) ;
	free( row_pointers ) ;

	// 読み込み処理の終了
	png_read_end(png_ptr, info_ptr);

	// 読み込み処理用構造体の破棄
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

	STCLOSE(Src);
	// 終了
	sceKernelDcacheWritebackAll();//全部書き込んじゃえ
	texptr->alphabit = 1;
	return texptr;
}
#else
int LoadPngImage(const char *FileName)
{
	return -1;
}

#endif // DX_NON_PNGREAD
