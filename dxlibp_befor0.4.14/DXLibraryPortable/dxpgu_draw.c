/*
ソースコードが肥大化してるので一寸分割。
DXライブラリPortable　描画関数群
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

#define USE_OPTIMIZED_VERTEXBUFFER090410	//コメントアウトで頂点を最適化しない描画にする。0.5.xにバージョンをあげる頃まで様子を見て、安定しているようなら古いコードは削除。


inline static int StaticDrawExtendGraph(int x1,int y1,int x2,int y2,DXPGRAPHDATA* gptr,int trnas);
inline static int StaticDrawModiGraph( int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4,DXPGRAPHDATA* gptr, int TransFlag );
inline static int StaticDrawModiGraphF( float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4,DXPGRAPHDATA* gptr, int TransFlag );


int DrawExtendGraph(int x1,int y1,int x2,int y2,int gh,int trans)
{

	return StaticDrawExtendGraph(x1,y1,x2,y2,GraphHandle2Ptr(gh),trans);
}


int DrawModiGraph( int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int gh, int TransFlag )
{
	return StaticDrawModiGraph(x1,y1,x2,y2,x3,y3,x4,y4,GraphHandle2Ptr(gh),TransFlag);
}



int DrawModiGraphF( float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4, int gh, int TransFlag )
{
	return StaticDrawModiGraphF(x1,y1,x2,y2,x3,y3,x4,y4,GraphHandle2Ptr(gh),TransFlag);
}



int DrawGraph(int x,int y,int gh,int trans)
{
	DXPGRAPHDATA *gptr = GraphHandle2Ptr(gh);
	if(gptr == NULL)return -1;
	return StaticDrawExtendGraph(x,y,x + gptr->u1 - gptr->u0,y + gptr->v1 - gptr->v0,gptr,trans);
}
int DrawTurnGraph(int x,int y,int gh,int trans)
{
	DXPGRAPHDATA *gptr = GraphHandle2Ptr(gh);
	if(gptr == NULL)return -1;
	return StaticDrawExtendGraph(x + gptr->u1 - gptr->u0,y,x,y + gptr->v1 - gptr->v0,gptr,trans);	
}

int	DrawRotaGraphCompatible(int x,int y,double ExtRate,double Angle,int gh,int trans,int turn)
{
	return DrawRotaGraph(x,y,ExtRate,Angle,gh,trans,turn);
}

int	DrawRotaGraph(int x,int y,float ExtRate,float Angle,int gh,int trans,int turn)
{
#ifdef DXP_NOUSE_FLTVERTEX_WITH_ROTA
	DXPGRAPHDATA* gptr = GraphHandle2Ptr(gh);
	if(gptr == NULL)return -1;
	register float x1,x2,x3,x4,y1,y2,y3,y4;
	register float x1_,x2_,x3_,x4_,y1_,y2_,y3_,y4_;
	x2 = x3 = (gptr->u1 - gptr->u0) / 2;
	x1 = x4 = -x3;
	y3 = y4 = (gptr->v1 - gptr->v0) / 2;
	y1 = y2 = -y3;
	register float extrate = ExtRate;
	x1 *= extrate;
	x2 *= extrate;
	x3 *= extrate;
	x4 *= extrate;
	y1 *= extrate;
	y2 *= extrate;
	y3 *= extrate;
	y4 *= extrate;
	float s,c;
	s = sinf(Angle);
	c = cosf(Angle);

#define XROT(VARNUM)	\
	{	\
	x##VARNUM##_ = x##VARNUM * c - y##VARNUM * s + x;	\
	y##VARNUM##_ = x##VARNUM * s + y##VARNUM * c + y;	\
	}

	XROT(1)
	XROT(2)
	XROT(3)
	XROT(4)
#undef XROT
	if(turn)return StaticDrawModiGraph(x2_,y2_,x1_,y1_,x4_,y4_,x3_,y3_,gptr,trans);
	return StaticDrawModiGraph(x1_,y1_,x2_,y2_,x3_,y3_,x4_,y4_,gptr,trans);
#else
	return DrawRotaGraphF(x,y,ExtRate,Angle,gh,trans,turn);
#endif
}

int	DrawRotaGraphFCompatible(float x,float y,double ExtRate,double Angle,int gh,int trans,int turn)
{
	return DrawRotaGraphF(x,y,ExtRate,Angle,gh,trans,turn);
}

int	DrawRotaGraphF(float x,float y,float ExtRate,float Angle,int gh,int trans,int turn)
{
	DXPGRAPHDATA* gptr = GraphHandle2Ptr(gh);
	if(!gptr)return -1;
	register float x1,x2,x3,x4,y1,y2,y3,y4;
	register float s,c;
	register float eus,euc,evs,evc;
	s = sinf(Angle);
	c = cosf(Angle);
	eus = ((gptr->u1 - gptr->u0) >> 1) * ExtRate;
	evs = ((gptr->v1 - gptr->v0) >> 1) * ExtRate;
	euc = eus * c;
	evc = evs * c;
	eus *= s;
	evs *= s;
	x3 = -(x1 = evs - euc);
	x4 = -(x2 = euc + evs);
	y1 = -(y3 = eus + evc);
	y4 = -(y2 = eus - evc);
	x1 += x;
	x2 += x;
	x3 += x;
	x4 += x;
	y1 += y;
	y2 += y;
	y3 += y;
	y4 += y;
	if(turn)return StaticDrawModiGraphF(x2,y2,x1,y1,x4,y4,x3,y3,gptr,trans);
	return StaticDrawModiGraphF(x1,y1,x2,y2,x3,y3,x4,y4,gptr,trans);
//ｘ’＝ｘcosθ-ysinθ
//ｙ’＝ｘsinθ+ycosθ
}

int DrawRotaGraph2Compatible(int x,int y,int cx,int cy,double ExtRate,double Angle,int gh,int trans,int turn)
{
	return DrawRotaGraph2(x,y,cx,cy,ExtRate,Angle,gh,trans,turn);
}

int DrawRotaGraph2(int x,int y,int cx,int cy,float ExtRate,float Angle,int gh,int trans,int turn)
{
#ifdef DXP_NOUSE_FLTVERTEX_WITH_ROTA
	DXPGRAPHDATA* gptr = GraphHandle2Ptr(gh);
	if(gptr == NULL)return -1;
	register float x1,x2,x3,x4,y1,y2,y3,y4;
	register float x1_,x2_,x3_,x4_,y1_,y2_,y3_,y4_;
	x2 = x3 = (gptr->u1 - gptr->u0) - cx;
	x1 = x4 = -cx;
	y3 = y4 = (gptr->v1 - gptr->v0) - cy;
	y1 = y2 = -cy;
	register float extrate = ExtRate;
	x1 *= extrate;
	x2 *= extrate;
	x3 *= extrate;
	x4 *= extrate;
	y1 *= extrate;
	y2 *= extrate;
	y3 *= extrate;
	y4 *= extrate;
	float s,c;
	s = sinf(Angle);
	c = cosf(Angle);

#define XROT(VARNUM)	\
	{	\
	x##VARNUM##_ = x##VARNUM * c - y##VARNUM * s + x;	\
	y##VARNUM##_ = x##VARNUM * s + y##VARNUM * c + y;	\
	}

	XROT(1)
	XROT(2)
	XROT(3)
	XROT(4)
#undef XROT
	if(turn)return StaticDrawModiGraph(x2_,y2_,x1_,y1_,x4_,y4_,x3_,y3_,gptr,trans);
	return StaticDrawModiGraph(x1_,y1_,x2_,y2_,x3_,y3_,x4_,y4_,gptr,trans);
#else
	return DrawRotaGraph2F(x,y,cx,cy,ExtRate,Angle,gh,trans,turn);
#endif
}

int	DrawRotaGraph2FCompatible(float x,float y,float cx,float cy,double ExtRate,double Angle,int gh,int trans,int turn)
{
	return DrawRotaGraph2F(x,y,cx,cy,ExtRate,Angle,gh,trans,turn);
}

int	DrawRotaGraph2F(float x,float y,float cx,float cy,float ExtRate,float Angle,int gh,int trans,int turn)
{
	DXPGRAPHDATA* gptr = GraphHandle2Ptr(gh);
	if(!gptr)return -1;
	register float x1,x2,x3,x4,y1,y2,y3,y4;
	register float x1_,x2_,x3_,x4_,y1_,y2_,y3_,y4_;
	x2 = x3 = ((gptr->u1 - gptr->u0) - cx) * ExtRate;
	x1 = x4 = -cx * ExtRate;
	y3 = y4 = ((gptr->v1 - gptr->v0) - cy) * ExtRate;
	y1 = y2 = -cy * ExtRate;
	register float s,c;
	s = sinf(Angle);
	c = cosf(Angle);

#define XROT(VARNUM)	\
	{	\
	x##VARNUM##_ = x##VARNUM * c - y##VARNUM * s + x;	\
	y##VARNUM##_ = x##VARNUM * s + y##VARNUM * c + y;	\
	}

	XROT(1)
	XROT(2)
	XROT(3)
	XROT(4)
#undef XROT
	if(turn)return StaticDrawModiGraphF(x2_,y2_,x1_,y1_,x4_,y4_,x3_,y3_,gptr,trans);
	return StaticDrawModiGraphF(x1_,y1_,x2_,y2_,x3_,y3_,x4_,y4_,gptr,trans);
//ｘ’＝ｘcosθ-ysinθ
//ｙ’＝ｘsinθ+ycosθ
}

int	DrawLine( int x1, int y1, int x2, int y2, int Color)
{
	GUSTART;
	SetTexture(-1,0);
	SetBaseColor(Color);
	DXPVERTEX_2D *vtxbuf = sceGuGetMemory(sizeof(DXPVERTEX_2D) * 2);
	if(vtxbuf == NULL)return -1;
	vtxbuf[0].x = x1;
	vtxbuf[0].y = y1;
	vtxbuf[0].z = gusettings.z_2d;
	vtxbuf[1].x = x2;
	vtxbuf[1].y = y2;
	vtxbuf[1].z = gusettings.z_2d;
//	sceKernelDcacheWritebackRange(vtxbuf,sizeof(DXPVERTEX_2D) * 2);
	sceGuDrawArray(GU_LINES,DXP_VTYPE_2D | GU_TRANSFORM_2D,2,0,vtxbuf);
	return 0;
}

int DrawBox(int x1,int y1,int x2,int y2,int color,int fillflag)
{
	GUSTART
	SetTexture(-1,0);
	SetBaseColor(color);
	if(fillflag)
	{
		DXPVERTEX_2D *vtxbuf = sceGuGetMemory(sizeof(DXPVERTEX_2D) * 2);
		if(vtxbuf == NULL)return -1;
		vtxbuf[0].x = x1;
		vtxbuf[0].y = y1;
		vtxbuf[0].z = gusettings.z_2d;
		vtxbuf[1].x = x2;
		vtxbuf[1].y = y2;
		vtxbuf[1].z = gusettings.z_2d;
		sceGuDrawArray(GU_SPRITES,DXP_VTYPE_2D | GU_TRANSFORM_2D,2,0,vtxbuf);
	}
	else
	{
		DXPVERTEX_2D *vtxbuf = sceGuGetMemory(sizeof(DXPVERTEX_2D) * 5);
		if(vtxbuf == NULL)return -1;
		vtxbuf[0].x = x1;
		vtxbuf[0].y = y1;
		vtxbuf[0].z = gusettings.z_2d;
		vtxbuf[1].x = x2;
		vtxbuf[1].y = y1;
		vtxbuf[1].z = gusettings.z_2d;
		vtxbuf[2].x = x2;
		vtxbuf[2].y = y2;
		vtxbuf[2].z = gusettings.z_2d;
		vtxbuf[3].x = x1;
		vtxbuf[3].y = y2;
		vtxbuf[3].z = gusettings.z_2d;
		vtxbuf[4].x = x1;
		vtxbuf[4].y = y1;
		vtxbuf[4].z = gusettings.z_2d;
		sceGuDrawArray(GU_LINE_STRIP,DXP_VTYPE_2D | GU_TRANSFORM_2D,5,0,vtxbuf);
	}
	return 0;
}

int	DrawPixel( int x, int y, int Color)
{
	GUSTART
	SetTexture(-1,0);
	SetBaseColor(Color);
	DXPVERTEX_2D *vtxbuf = sceGuGetMemory(sizeof(DXPVERTEX_2D) * 2);
	if(vtxbuf == NULL)return -1;
	vtxbuf[0].x = x;
	vtxbuf[0].y = y;
	vtxbuf[0].z = gusettings.z_2d;
	vtxbuf[1].x = x + 1;
	vtxbuf[1].y = y + 1;
	vtxbuf[1].z = gusettings.z_2d;
	sceGuDrawArray(GU_LINES,DXP_VTYPE_2D | GU_TRANSFORM_2D,2,0,vtxbuf);
	return 0;
}

#define DXPOVAL_DIV	128
int	DrawCircle( int x, int y, int r, int Color,int fill)
{
	GUSTART;
	SetTexture(-1,0);
	SetBaseColor(Color);
	DXPVERTEX_2D *vtxbuf = sceGuGetMemory(sizeof(DXPVERTEX_2D) * (DXPOVAL_DIV + 2));
	if(vtxbuf == NULL)return -1;
	int i;
	vtxbuf[0].x = x;
	vtxbuf[0].y = y;
	vtxbuf[0].z = gusettings.z_2d;

	for(i = 1;i <= DXPOVAL_DIV + 1;++i)
	{
		vtxbuf[i].x = x + r * cosf(M_PI * 2 / DXPOVAL_DIV * i);
		vtxbuf[i].y = y + r * sinf(M_PI * 2 / DXPOVAL_DIV * i);
		vtxbuf[i].z = gusettings.z_2d;
	}

	if(fill)
		sceGuDrawArray(GU_TRIANGLE_FAN,DXP_VTYPE_2D | GU_TRANSFORM_2D,DXPOVAL_DIV + 2,0,vtxbuf);
	else
		sceGuDrawArray(GU_LINE_STRIP,DXP_VTYPE_2D | GU_TRANSFORM_2D,DXPOVAL_DIV + 1,0,vtxbuf + 1);
	return 0;
}

int DrawTriangle(int x1,int y1,int x2,int y2,int x3,int y3,int color,int fill)
{
	GUSTART
	SetTexture(-1,0);
	SetBaseColor(color);
	if(fill)
	{
		DXPVERTEX_2D *vtxbuf = sceGuGetMemory(sizeof(DXPVERTEX_2D) * 3);
		if(vtxbuf == NULL)return -1;
		vtxbuf[0].x = x1;
		vtxbuf[0].y = y1;
		vtxbuf[0].z = gusettings.z_2d;
		vtxbuf[1].x = x2;
		vtxbuf[1].y = y2;
		vtxbuf[1].z = gusettings.z_2d;
		vtxbuf[2].x = x3;
		vtxbuf[2].y = y3;
		vtxbuf[2].z = gusettings.z_2d;
		sceGuDrawArray(GU_TRIANGLES,DXP_VTYPE_2D | GU_TRANSFORM_2D,3,0,vtxbuf);
	}
	else
	{
		DXPVERTEX_2D *vtxbuf = sceGuGetMemory(sizeof(DXPVERTEX_2D) * 4);
		if(vtxbuf == NULL)return -1;
		vtxbuf[0].x = x1;
		vtxbuf[0].y = y1;
		vtxbuf[0].z = gusettings.z_2d;
		vtxbuf[1].x = x2;
		vtxbuf[1].y = y2;
		vtxbuf[1].z = gusettings.z_2d;
		vtxbuf[2].x = x3;
		vtxbuf[2].y = y3;
		vtxbuf[2].z = gusettings.z_2d;
		vtxbuf[3].x = x1;
		vtxbuf[3].y = y1;
		vtxbuf[3].z = gusettings.z_2d;
		sceGuDrawArray(GU_LINE_STRIP,DXP_VTYPE_2D | GU_TRANSFORM_2D,4,0,vtxbuf);
	}
	return 0;
}

int DrawPolygon3D(VERTEX_3D *Vertex,int PolygonNum,int gh,int trans)
{
	GUSTART
	DXPGRAPHDATA *pg = GraphHandle2Ptr(gh);
	if(pg == NULL)return -1;
	SetTexture2(pg->tex,trans);
	sceGuShadeModel(GU_SMOOTH);
	DXPVERTEX_3DTEX_F *vtxbuf = sceGuGetMemory(sizeof(DXPVERTEX_3DTEX_F) * PolygonNum * 3);
	if(vtxbuf == NULL)return -1;
	int i,j;
	for(i = 0;i < PolygonNum;++i)
	{
		for(j = 0;j < 3;++j)
		{
			vtxbuf[i * 3 + j].u = (pg->u0 + (pg->u1 - pg->u0) * Vertex[i * 3 + j].u) / pg->tex->width;
			vtxbuf[i * 3 + j].v = (pg->v0 + (pg->v1 - pg->v0) * Vertex[i * 3 + j].v) / pg->tex->height;
			vtxbuf[i * 3 + j].color = ((u32)Vertex[i * 3 + j].a << 24) | ((u32)Vertex[i * 3 + j].b << 16) | ((u32)Vertex[i * 3 + j].g << 8) | ((u32)Vertex[i * 3 + j].r);
			vtxbuf[i * 3 + j].x = Vertex[i * 3 + j].pos.x;
			vtxbuf[i * 3 + j].y = Vertex[i * 3 + j].pos.y;
			vtxbuf[i * 3 + j].z = Vertex[i * 3 + j].pos.z;
		}
	}
	sceGumDrawArray(GU_TRIANGLES,DXP_VTYPE_3DTEX_F | GU_TRANSFORM_3D,PolygonNum * 3,0,vtxbuf);
	sceGuShadeModel(GU_FLAT);
	GuListSafety();
	return 0;
}

inline static int StaticDrawExtendGraph(int x1,int y1,int x2,int y2,DXPGRAPHDATA *gptr,int trans)
{
	if((x2 < x1 || y2 < y1) && !(x2 < x1 && y2 < y1))return StaticDrawModiGraph(x1,y1,x2,y1,x2,y2,x1,y2,gptr,trans);//X方向反転画像がうまく動作しないので（GU_SPRITESを使ったのが原因と思われる）強引に回避
	if(!gptr)return -1;
	if(!gptr->tex)return -1;
	if(SetTexture2(gptr->tex,trans) == -1)return -1;
	int sw = gusettings.slice * 2 / PSM2BYTEx2(gptr->tex->psm);	/*何ピクセルごとにsliceするか*/
	int u0,u1;
	u0 = gptr->u0;
	int count = (gptr->u1 - gptr->u0 + sw - 1) / sw;
	DXPVERTEX_2DTEX *vtxbuf = (DXPVERTEX_2DTEX*)sceGuGetMemory(sizeof(DXPVERTEX_2DTEX) * 2 * count);
	int i = 0;
	register float invu1_u0 = 1.0f / (gptr->u1 - gptr->u0);
	while(u0 < gptr->u1)
	{
		u1 = MIN(u0 + sw,gptr->u1);
		if(vtxbuf == NULL)return -1;
		vtxbuf[i<<1].u = u0;
		vtxbuf[i<<1].v = gptr->v0;
		vtxbuf[i<<1].x = x1 + (float)(x2 - x1) * (u0 - gptr->u0) * invu1_u0;
		vtxbuf[i<<1].y = y1;
		vtxbuf[i<<1].z = gusettings.z_2d;
		vtxbuf[(i<<1)+1].u = u1;
		vtxbuf[(i<<1)+1].v = gptr->v1;
		vtxbuf[(i<<1)+1].x = x1 + (float)(x2 - x1) * (u1 - gptr->u0) * invu1_u0;
		vtxbuf[(i<<1)+1].y = y2;
		vtxbuf[(i<<1)+1].z = gusettings.z_2d;
		u0 += sw;
		++i;
	}
	sceGuDrawArray(GU_SPRITES,DXP_VTYPE_2DTEX | GU_TRANSFORM_2D,2 * count,NULL,vtxbuf);
	GuListSafety();
	return 0;
}

inline static int StaticDrawModiGraph( int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4,DXPGRAPHDATA* gptr, int TransFlag )
{
	if(!gptr)return -1;
	if(!gptr->tex)return -1;
	GUSTART;
	if(SetTexture2(gptr->tex,TransFlag) == -1)return -1;
	int sw = gusettings.slice * 2 / PSM2BYTEx2(gptr->tex->psm);	/*何ピクセルごとにsliceするか*/
	int count = (gptr->u1 - gptr->u0 + sw - 1) / sw;
	int u = gptr->u0,i = 1;
	DXPVERTEX_2DTEX *vtxbuf = (DXPVERTEX_2DTEX*)sceGuGetMemory(sizeof(DXPVERTEX_2DTEX) * 2 * (count + 1));
	if(vtxbuf == NULL)return -1;
	register float invu1_u0 = 1.0f / (gptr->u1 - gptr->u0);

	vtxbuf[0].u = u;
	vtxbuf[0].v = gptr->v0;
	vtxbuf[0].x = x1 + (x2 - x1) * (u - gptr->u0) * invu1_u0;
	vtxbuf[0].y = y1 + (y2 - y1) * (u - gptr->u0) * invu1_u0;
	vtxbuf[0].z = gusettings.z_2d;
	vtxbuf[1].u = u;
	vtxbuf[1].v = gptr->v1;
	vtxbuf[1].x = x4 + (x3 - x4) * (u - gptr->u0) * invu1_u0;
	vtxbuf[1].y = y4 + (y3 - y4) * (u - gptr->u0) * invu1_u0;
	vtxbuf[1].z = gusettings.z_2d;
	while(u < gptr->u1)
	{
		u = MIN(u + sw,gptr->u1);
		vtxbuf[i<<1].u = u;
		vtxbuf[i<<1].v = gptr->v0;
		vtxbuf[i<<1].x = x1 + (x2 - x1) * (u - gptr->u0) * invu1_u0;
		vtxbuf[i<<1].y = y1 + (y2 - y1) * (u - gptr->u0) * invu1_u0;
		vtxbuf[i<<1].z = gusettings.z_2d;
		vtxbuf[(i<<1)+1].u = u;
		vtxbuf[(i<<1)+1].v = gptr->v1;
		vtxbuf[(i<<1)+1].x = x4 + (x3 - x4) * (u - gptr->u0) * invu1_u0;
		vtxbuf[(i<<1)+1].y = y4 + (y3 - y4) * (u - gptr->u0) * invu1_u0;
		vtxbuf[(i<<1)+1].z = gusettings.z_2d;
		++i;
	}
	sceGuDrawArray(GU_TRIANGLE_STRIP,DXP_VTYPE_2DTEX | GU_TRANSFORM_2D,2 * (count + 1),NULL,vtxbuf);
	GuListSafety();
	return 0;
}

inline static int StaticDrawModiGraphF( float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4,DXPGRAPHDATA* gptr, int TransFlag )
{
	if(gptr == NULL)return -1;
	if(gptr->tex == NULL)return -1;
	GUSTART;
	if(SetTexture2(gptr->tex,TransFlag) == -1)return -1;
	int sw = gusettings.slice * 2 / PSM2BYTEx2(gptr->tex->psm);	/*何ピクセルごとにsliceするか*/
	int count = (gptr->u1 - gptr->u0 + sw - 1) / sw;
	int u = gptr->u0,i = 1;
	DXPVERTEX_2DTEX_F *vtxbuf = (DXPVERTEX_2DTEX_F*)sceGuGetMemory(sizeof(DXPVERTEX_2DTEX_F) * 2 * (count + 1));
	if(vtxbuf == NULL)return -1;
	register float invu1_u0 = 1.0f / (gptr->u1 - gptr->u0);

	vtxbuf[0].u = u;
	vtxbuf[0].v = gptr->v0;
	vtxbuf[0].x = x1 + (x2 - x1) * (u - gptr->u0) * invu1_u0;
	vtxbuf[0].y = y1 + (y2 - y1) * (u - gptr->u0) * invu1_u0;
	vtxbuf[0].z = gusettings.z_2d;
	vtxbuf[1].u = u;
	vtxbuf[1].v = gptr->v1;
	vtxbuf[1].x = x4 + (x3 - x4) * (u - gptr->u0) * invu1_u0;
	vtxbuf[1].y = y4 + (y3 - y4) * (u - gptr->u0) * invu1_u0;
	vtxbuf[1].z = gusettings.z_2d;
	while(u < gptr->u1)
	{
		u = MIN(u + sw,gptr->u1);
		vtxbuf[i<<1].u = u;
		vtxbuf[i<<1].v = gptr->v0;
		vtxbuf[i<<1].x = x1 + (x2 - x1) * (u - gptr->u0) * invu1_u0;
		vtxbuf[i<<1].y = y1 + (y2 - y1) * (u - gptr->u0) * invu1_u0;
		vtxbuf[i<<1].z = gusettings.z_2d;
		vtxbuf[(i<<1)+1].u = u;
		vtxbuf[(i<<1)+1].v = gptr->v1;
		vtxbuf[(i<<1)+1].x = x4 + (x3 - x4) * (u - gptr->u0) * invu1_u0;
		vtxbuf[(i<<1)+1].y = y4 + (y3 - y4) * (u - gptr->u0) * invu1_u0;
		vtxbuf[(i<<1)+1].z = gusettings.z_2d;
		++i;
	}
	sceGuDrawArray(GU_TRIANGLE_STRIP,DXP_VTYPE_2DTEX_F | GU_TRANSFORM_2D,2 * (count + 1),NULL,vtxbuf);
	GuListSafety();
	return 0;
}



int DrawExtendGraphCommon(int x1,int y1,int x2,int y2,DXPGRAPHDATA* gptr)
{
	if(gptr == NULL)return -1;
	if(gptr->tex == NULL)return -1;
	int sw = gusettings.slice * 2 / PSM2BYTEx2(gptr->tex->psm);	/*何ピクセルごとにsliceするか*/
	int u[2];
	u[0] = gptr->u0;
	int count = (gptr->u1 - gptr->u0 + sw - 1) / sw;
	DXPVERTEX_2DTEX *vtxbuf = (DXPVERTEX_2DTEX*)sceGuGetMemory(sizeof(DXPVERTEX_2DTEX) * 2 * count);
	if(vtxbuf == NULL)return -1;
	int i = 0;
	while(u[0] < gptr->u1)
	{
		u[1] = MIN(u[0] + sw,gptr->u1);
		vtxbuf[(i<<1)+0].u = u[0];
		vtxbuf[(i<<1)+0].v = gptr->v0;
		vtxbuf[(i<<1)+0].x = x1 + (float)(x2 - x1) * (u[0] - gptr->u0) / (gptr->u1 - gptr->u0);
		vtxbuf[(i<<1)+0].y = y1;
		vtxbuf[(i<<1)+0].z = gusettings.z_2d;
		vtxbuf[(i<<1)+1].u = u[1];
		vtxbuf[(i<<1)+1].v = gptr->v1;
		vtxbuf[(i<<1)+1].x = x1 + (float)(x2 - x1) * (u[1] - gptr->u0) / (gptr->u1 - gptr->u0);
		vtxbuf[(i<<1)+1].y = y2;
		vtxbuf[(i<<1)+1].z = gusettings.z_2d;
		u[0] += sw;
		++i;
	}
	sceGuDrawArray(GU_SPRITES,DXP_VTYPE_2DTEX | GU_TRANSFORM_2D,2 * count,NULL,vtxbuf);
	GuListSafety();
	return 0;
}

int DrawGraphBoost(int x,int y)
{
	if(bptr == NULL)return -1;
	DrawExtendGraphCommon(x,y,x + bptr->u1 - bptr->u0,y + bptr->v1 - bptr->v0,bptr);
	return 0;
}

int DrawGraphCentre(int x,int y,int gh,int trans)
{
	DXPGRAPHDATA *gptr = GraphHandle2Ptr(gh);
	if(gptr == NULL)	return -1;
	int	vx = (gptr->u1 - gptr->u0) / 2;
	int	vy = (gptr->v1 - gptr->v0) / 2;
	return StaticDrawExtendGraph(x - vx ,y - vy,x + vx,y + vy,gptr,trans);
}

int DrawGraphCentreBoost(int x,int y)
{
	if(bptr == NULL)return -1;
	int	vx = (bptr->u1 - bptr->u0) / 2;
	int	vy = (bptr->v1 - bptr->v0) / 2;
	DrawExtendGraphCommon(x - vx ,y - vy,x + vx,y + vy,bptr);
	return 0;
}

int DrawExtendGraphBoost(int x1,int y1,int x2,int y2)
{
	DrawExtendGraphCommon(x1,y1,x2,y2,bptr);
	return 0;
}


int DrawModiGraphFCommon( float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4, DXPGRAPHDATA* gptr)
{
	if(gptr == NULL)return -1;
	if(gptr->tex == NULL)return -1;
	int sw = gusettings.slice * 2 / PSM2BYTEx2(gptr->tex->psm);	/*何ピクセルごとにsliceするか*/
	int count = (gptr->u1 - gptr->u0 + sw - 1) / sw;
	int u = gptr->u0,i = 1;
	DXPVERTEX_2DTEX_F *vtxbuf = (DXPVERTEX_2DTEX_F*)sceGuGetMemory(sizeof(DXPVERTEX_2DTEX_F) * 2 * (count + 1));
	if(vtxbuf == NULL)return -1;

	vtxbuf[0].u = u;
	vtxbuf[0].v = gptr->v0;
	vtxbuf[0].x = x1 + (float)(x2 - x1) * (u - gptr->u0) / (gptr->u1 - gptr->u0);
	vtxbuf[0].y = y1 + (float)(y2 - y1) * (u - gptr->u0) / (gptr->u1 - gptr->u0);
	vtxbuf[0].z = gusettings.z_2d;
	vtxbuf[1].u = u;
	vtxbuf[1].v = gptr->v1;
	vtxbuf[1].x = x4 + (float)(x3 - x4) * (u - gptr->u0) / (gptr->u1 - gptr->u0);
	vtxbuf[1].y = y4 + (float)(y3 - y4) * (u - gptr->u0) / (gptr->u1 - gptr->u0);
	vtxbuf[1].z = gusettings.z_2d;
	while(u < gptr->u1)
	{
		u = MIN(u + sw,gptr->u1);
		vtxbuf[(i<<1)+0].u = u;
		vtxbuf[(i<<1)+0].v = gptr->v0;
		vtxbuf[(i<<1)+0].x = x1 + (float)(x2 - x1) * (u - gptr->u0) / (gptr->u1 - gptr->u0);
		vtxbuf[(i<<1)+0].y = y1 + (float)(y2 - y1) * (u - gptr->u0) / (gptr->u1 - gptr->u0);
		vtxbuf[(i<<1)+0].z = gusettings.z_2d;
		vtxbuf[(i<<1)+1].u = u;
		vtxbuf[(i<<1)+1].v = gptr->v1;
		vtxbuf[(i<<1)+1].x = x4 + (float)(x3 - x4) * (u - gptr->u0) / (gptr->u1 - gptr->u0);
		vtxbuf[(i<<1)+1].y = y4 + (float)(y3 - y4) * (u - gptr->u0) / (gptr->u1 - gptr->u0);
		vtxbuf[(i<<1)+1].z = gusettings.z_2d;
		++i;
	}
	sceGuDrawArray(GU_TRIANGLE_STRIP,DXP_VTYPE_2DTEX_F | GU_TRANSFORM_2D,2 * (count + 1),NULL,vtxbuf);
	GuListSafety();
	return 0;
}

int	DrawRotaGraphFBoost(float x,float y,float ExtRate,float Angle,int turn)
{
	if(!bptr)return -1;
	register float x1,x2,x3,x4,y1,y2,y3,y4;
	register float s,c;
	register float eus,euc,evs,evc;
	s = sinf(Angle);
	c = cosf(Angle);
	eus = ((bptr->u1 - bptr->u0) >> 1) * ExtRate;
	evs = ((bptr->v1 - bptr->v0) >> 1) * ExtRate;
	euc = eus * c;
	evc = evs * c;
	eus *= s;
	evs *= s;
	x3 = -(x1 = evs - euc);
	x4 = -(x2 = euc + evs);
	y1 = -(y3 = eus + evc);
	y4 = -(y2 = eus - evc);
	x1 += x;
	x2 += x;
	x3 += x;
	x4 += x;
	y1 += y;
	y2 += y;
	y3 += y;
	y4 += y;
	if(turn)
		DrawModiGraphFCommon(x2,y2,x1,y1,x4,y4,x3,y3,bptr);
	else
		DrawModiGraphFCommon(x1,y1,x2,y2,x3,y3,x4,y4,bptr);
	return 0;
//ｘ’＝ｘcosθ-ysinθ
//ｙ’＝ｘsinθ+ycosθ
}
/*
テクスチャ関連の関数概略
sceGuTexEnvColor	テクスチャブレンディングの定数を設定
sceGuTexFunc		テクスチャブレンド方法を設定
sceGuTexFilter		テクスチャフィルタリングの設定　ネアレストとバイリニアだけでいいかとｗ

sceGuTexFlush		GPU内部のテクスチャキャッシュを飛ばす
sceGuTexMapMode		テクスチャの張り方の指定

sceGuTexMode		テクスチャフォーマットとかの指定
sceGuTexImage		テクスチャを設定

sceGuTexWrap		テクスチャのUV座標が限界突破したときどうするか設定

sceGuTexOffset		テクスチャのUV座標に加算される値。2Dでは使えない
sceGuTexProjMapMode	テクスチャのUV座標系に何を使うか	多分呼び出すことは無い
sceGuTexScale		テクスチャのUV座標に乗算される値。2Dでは使えない
sceGuTexSlope		不明
sceGuTexSync		sceGuCopyImage()の終了を待つ
sceGuTexLevelMode	みっぷマップの設定　当分使わないつもり


ステータスの一覧
#define GU_ALPHA_TEST		(0)		アルファテスト
#define GU_DEPTH_TEST		(1)		深度テスト
#define GU_SCISSOR_TEST		(2)		シザーテスト（描画矩形内にピクセルが収まっているかの判定）
#define GU_STENCIL_TEST		(3)		ステンシルテスト	バッファの設定方法がよくわからない ディスプレイバッファのα成分を使うらしい
#define GU_BLEND		(4)			ブレンドを使用する。
#define GU_CULL_FACE		(5)		カリング	2Dでは使わない
#define GU_DITHER		(6)			減色		よくわからない
#define GU_FOG			(7)			フォグ		2Dではたぶｎつかわない
#define GU_CLIP_PLANES		(8)		不明		サンプルではEnableになってる
#define GU_TEXTURE_2D		(9)		テクスチャを張るかどうか
#define GU_LIGHTING		(10)		ライティング	使わない
#define GU_LIGHT0		(11)
#define GU_LIGHT1		(12)
#define GU_LIGHT2		(13)
#define GU_LIGHT3		(14)
#define GU_LINE_SMOOTH		(15)	斜めの線分の描画が綺麗になる。
#define GU_PATCH_CULL_FACE	(16)	不明
#define GU_COLOR_TEST		(17)	カラーテスト　カラーキーみたいなのができる？
#define GU_COLOR_LOGIC_OP	(18)	多分カラーテストで使うものだと
#define GU_FACE_NORMAL_REVERSE	(19)多分法線を反転して扱うのではないかと…
#define GU_PATCH_FACE		(20)	高次曲面パッチを使う。2Dでは使わないよ…
#define GU_FRAGMENT_2X		(21)	フラグメントとはピクセルのことらしい。それで…？

*/
