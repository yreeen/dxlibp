#ifndef	DXLIBP_PC_H__
#define	DXLIBP_PC_H__
/*
これはPCの座標系をPSP用に強引に改変するラッパー関数郡です。
Ver0.0.1
*/

//#define DXPPC_DONT_GRAPHRESIZE	//画像サイズの調整しなくていいときはコメントアウト外してください。

#include "dxlibp.h"
inline float pc2psp_x(float x){return x * 9 / 16 + 60;}
inline float pc2psp_y(float y){return y * 9 / 16;}
inline int	PCDrawPixel( int x, int y, int Color){x = pc2psp_x(x);y = pc2psp_y(y);return DrawPixel(x,y,Color);}
inline int	PCDrawLine( int x1, int y1, int x2, int y2, int Color){x1 = pc2psp_x(x1);y1 = pc2psp_y(y1);x2 = pc2psp_x(x2);y2 = pc2psp_y(y2);return DrawLine(x1,y1,x2,y2,Color);}
inline int	PCDrawTriangle(int x1,int y1,int x2,int y2,int x3,int y3,int color,int fill){x1 = pc2psp_x(x1);y1 = pc2psp_y(y1);x2 = pc2psp_x(x2);y2 = pc2psp_y(y2);x3 = pc2psp_x(x3);y3 = pc2psp_y(y3);return DrawTriangle(x1,y1,x2,y2,x3,y3,color,fill);}
inline int	PCDrawBox(int x1,int y1,int x2,int y2,int color,int fillflag){x1 = pc2psp_x(x1);y1 = pc2psp_y(y1);
#ifndef	DXPPC_DONT_GRAPHRESIZE
	x2 = pc2psp_x(x2);y2 = pc2psp_y(y2);
#endif
	return DrawBox(x1,y1,x2,y2,color,fillflag);}
inline int	PCDrawCircle( int x, int y, int r, int Color,int fill){x = pc2psp_x(x);y = pc2psp_y(y);r = pc2psp_y(r);return DrawCircle(x,y,r,Color,fill);}
inline int	PCDrawGraph(int x,int y,int gh,int trans){int sx,sy;if(GetGraphSize(gh,&sx,&sy) == -1)return -1;x = pc2psp_x(x);y = pc2psp_y(y);sx = pc2psp_y(sx);sy = pc2psp_y(sy);return DrawExtendGraph(x,y,x + sx,y + sy,gh,trans);}
inline int	PCDrawExtendGraph(int x1,int y1,int x2,int y2,int gh,int trans){x1 = pc2psp_x(x1);y1 = pc2psp_y(y1);x2 = pc2psp_x(x2);y2 = pc2psp_y(y2);return DrawExtendGraph(x1,y1,x2,y2,gh,trans);}
inline int	PCDrawModiGraph(int x1,int y1,int x2,int y2,int x3,int y3,int x4,int y4,int gh,int trans){x1 = pc2psp_x(x1);y1 = pc2psp_y(y1);x2 = pc2psp_x(x2);y2 = pc2psp_y(y2);x3 = pc2psp_x(x3);y3 = pc2psp_y(y3);x4 = pc2psp_x(x4);y4 = pc2psp_y(y4);return DrawModiGraph(x1,y1,x2,y2,x3,y3,x4,y4,gh,trans);}
inline int	PCDrawTurnGraph(int x,int y,int gh,int trans){int sx,sy;if(GetGraphSize(gh,&sx,&sy) == -1)return -1;x = pc2psp_x(x);y = pc2psp_y(y);sx = pc2psp_y(sx);sy = pc2psp_y(sy);return DrawExtendGraph(x + sx,y,x,y + sy,gh,trans);}
inline int	PCDrawRotaGraph(int x,int y,double ExtRate,double Angle,int gh,int trans,int turn DXPDEFARG(0)){x = pc2psp_x(x);y = pc2psp_y(y);
#ifndef	DXPPC_DONT_GRAPHRESIZE
ExtRate = pc2psp_y(ExtRate);
#endif
	return DrawRotaGraph(x,y,ExtRate,Angle,gh,trans,turn);}
inline int	PCDrawRotaGraph2(int x,int y,int cx,int cy,double ExtRate,double Angle,int gh,int trans,int turn DXPDEFARG(0)){x = pc2psp_x(x);y = pc2psp_y(y);
#ifndef	DXPPC_DONT_GRAPHRESIZE
ExtRate = pc2psp_y(ExtRate);
#endif
	return DrawRotaGraph2(x,y,cx,cy,ExtRate,Angle,gh,trans,turn);}
inline int	PCDrawRotaGraphF(float x,float y,double ExtRate,double Angle,int gh,int trans,int turn DXPDEFARG(0)){x = pc2psp_x(x);y = pc2psp_y(y);
#ifndef	DXPPC_DONT_GRAPHRESIZE
ExtRate = pc2psp_y(ExtRate);
#endif
	return DrawRotaGraphF(x,y,ExtRate,Angle,gh,trans,turn);}
inline int	PCDrawRotaGraph2F(float x,float y,float cx,float cy,double ExtRate,double Angle,int gh,int trans,int turn DXPDEFARG(0)){x = pc2psp_x(x);y = pc2psp_y(y);ExtRate = pc2psp_y(ExtRate);return DrawRotaGraph2F(x,y,cx,cy,ExtRate,Angle,gh,trans,turn);}
inline int	PCDrawModiGraphF(float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4,int gh,int trans){x1 = pc2psp_x(x1);y1 = pc2psp_y(y1);x2 = pc2psp_x(x2);y2 = pc2psp_y(y2);x3 = pc2psp_x(x3);y3 = pc2psp_y(y3);x4 = pc2psp_x(x4);y4 = pc2psp_y(y4);return DrawModiGraphF(x1,y1,x2,y2,x3,y3,x4,y4,gh,trans);}
inline int	PCSetDrawArea(int x1,int y1,int x2,int y2){x1 = pc2psp_x(x1);y1 = pc2psp_y(y1);x2 = pc2psp_x(x2);y2 = pc2psp_y(y2);return SetDrawArea(x1,y1,x2,y2);}
inline int	PCDrawString(int x,int y,const char *str,int color,int edge DXPDEFARG(0)){x = pc2psp_x(x);y = pc2psp_y(y);return DrawString(x,y,str,color,edge);}
inline int	PCSetFontSize( int FontSize ){ return SetFontSize( FontSize * 9 / 16);}
inline int	PCSetFontSizeF( float FontSize ){ return SetFontSizeF( pc2psp_y(FontSize));}

#define	DrawFormatString(X,Y,COL,STR,...)	DrawFormatString(pc2psp_x(X),pc2psp_y(Y),COL,STR,##__VA_ARGS__)

#define	DrawPixel		PCDrawPixel
#define	DrawLine		PCDrawLine
#define	DrawTriangle	PCDrawTriangle
#define	DrawBox			PCDrawBox
#define	DrawCircle		PCDrawCircle
#define	DrawGraph		PCDrawGraph
#define	DrawTurnGraph	PCDrawTurnGraph
#define	DrawExtendGraph	PCDrawExtendGraph
#define	DrawModiGraph	PCDrawModiGraph
#define	DrawModiGraphF	PCDrawModiGraphF
#define DrawRotaGraph	PCDrawRotaGraph
#define DrawRotaGraph2	PCDrawRotaGraph2
#define DrawRotaGraphF	PCDrawRotaGraphF
#define DrawRotaGraph2F	PCDrawRotaGraph2F

#define SetDrawArea		PCSetDrawArea

#define DrawString		PCDrawString
#define SetFontSize		PCSetFontSize
#define SetFontSizeF	PCSetFontSizeF

#endif
