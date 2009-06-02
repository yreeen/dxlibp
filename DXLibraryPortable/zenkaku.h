/*
	mediumgauge氏作成の全角文字表示ライブラリのＤＸライブラリ適合版
	詳細はzenkaku.cの内部にあります。
*/

#ifndef	ZENKAKU_H__
#define	ZENKAKU_H__
#include "dxlibp.h"

#define BUF_WIDTH	(512)	/*一ラインあたりのピクセル数*/
#define SCR_WIDTH	(480)	/*画面横ピクセル数*/
#define SCR_HEIGHT	(272)	/*画面縦ピクセル数*/
#define	FONTWIDTH	6
#define	FONTHEIGHT	12

void	mh_print(int x,int y,const char *str,int col);
void	mh_displayoffset(int offset);
#endif

