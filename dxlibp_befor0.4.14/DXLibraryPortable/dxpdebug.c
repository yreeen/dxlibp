/*
*	DXライブラリPortable	デバッグ用コード部	Ver1.00
*	製作者	：夢瑞憂煉
*
*	備考	：とくになし
*/
#include <libc/stdio.h>
#include <pspdebug.h>
#include "dxpstatic.h"
#include "dxlibp.h"
#include "zenkaku.h"
#define LOGFILENAME	"psplog.txt"

#define STRBUF_X	80
#define STRBUF_Y	23
static char strbuf[STRBUF_Y][STRBUF_X + 1];	/*表示文字列バッファ*/
static char vfsbuf[2048];
static unsigned char cx = 0,cy = 0;		/*カーソルらしきもの*/
static unsigned char l1 = 0;
static char LogFileDisable = 1;



int IsZenkaku(int c)	//なぜか引数をintにした上で0xffと&してあげないと正常動作しない。謎だ…
{
	c &= 0xff;
	if((0x81 <= c && c <= 0x9f) || (0xe0 <= c && c <= 0xfc))return 1;
	return 0;
}


static void br()
{
	strbuf[cy][cx] = '\0';
	cx = 0;
	cy += 1;
	cy %= STRBUF_Y;
	if(cy == 0)l1 = STRBUF_Y;
	if(l1)l1 = (l1 + 1) % STRBUF_Y + STRBUF_Y;
}

void printfDx(const char *format,...)
{
	va_list varg;
	va_start(varg,format);
	vsnprintf(vfsbuf,2047,format,varg);
	va_end(varg);
	vfsbuf[2046] = '\0';
	vfsbuf[2047] = '\0';
	//vfsbufに表示内容格納
	int i;
	for(i = 0;vfsbuf[i] != '\0' && i < 2048;++i)
	{
		if(IsZenkaku(vfsbuf[i]))
		{
			if(cx >= 79)br();
			strbuf[cy][cx] = vfsbuf[i];
			strbuf[cy][cx + 1] = vfsbuf[i + 1];
			++i;
			cx += 2;
		}
		else
		{
			if(vfsbuf[i] == '\n')
			{
				br();
				continue;
			}
			if(cx >= 80)br();
			strbuf[cy][cx] = vfsbuf[i];
			cx += 1;
		}
	}
	strbuf[cy][cx] = '\0';
	DrawDebugScreen();
//	ScreenCopy();
}

void DrawDebugScreen()
{
	int i;
	for(i = 0;i < STRBUF_Y;++i)
	{
		mh_print(0,FONTHEIGHT * i,strbuf[(i + l1) % STRBUF_Y],0xffffffff);
	}
}











void InitDebugScreen()
{
	clsDx();
}

//int poe(const char *format,...)	//㍊に頼まれたセンタリング描画　エラーチェック入ってないから怖い怖いｗｗｗ
//{
//	char buf[512];
//	va_list	args;
//	va_start(args,format);
//	vsnprintf(buf,256,format,args);
//	va_end(args);
//	int len = strlen(format);
//	int spacenum = (80 - len) / 2;
//	memmove(buf + spacenum,buf,len + 1);
//	printfDx("%s\n",buf);
//	return spacenum;
//}

//void printfDx(const char *format,...)
//{
//	int i;
//	static char	buf[2048];
//	va_list	args;
//	va_start(args,format);
//	vsnprintf(buf,2048,format,args);
//	va_end(args);
//	/*bufに文字列格納終了*/
//
//	for(i = 0;1;++i)
//	{
//		if(buf[i] == '\n' || cx == STRBUF_X)
//		{
//			strbuf[cy][cx] = '\0';
//			cx = 0;
//			cy += 1;
//			cy %= STRBUF_Y;
//			++i;
//			//if(l1 - cy == 1 || cy == STRBUF_Y)
//			//{
//			//	++l1;
//			//	l1 %= STRBUF_Y;
//			//}
//			if(buf[i] == '\n')continue;
//		}
//		strbuf[cy][cx] = buf[i];
//		if(buf[i] == '\0')break;
//		++cx;
//	}
//}
//
void clsDx()
{
	int i;
	for(i = 0;i < STRBUF_Y;++i)
	{
		strbuf[i][0] = '\0';
	}
	cx = cy = 0;
	l1 = 0;
}
//void DrawDebugScreen()
//{
//	int i;
//	for(i = 0;i < STRBUF_Y;++i)
//	{
//		strbuf[i][STRBUF_X - 1] = '\0';
//	}
//	for(i = 0;i < STRBUF_Y;++i)
//	{
//		mh_print(0,FONTHEIGHT * i,strbuf[(l1 + i) % STRBUF_Y],0xffffffff);
//	}
//	//if(GetInputState() & DXP_INPUT_SELECT)
//	//{
//	//	FILE *fp = fopen("debugscreen.txt","w");
//	//	for(i = 0;i < STRBUF_Y;++i)
//	//	{
//	//		fputs(strbuf[i],fp);
//	//		fputc('\n',fp);
//	//	}
//	//	fclose(fp);
//	//}
//}
//
//

#define DEBUGFILE_ENABLE
//#undef	DEBUGFILE_ENABLE

void AppLogAdd(const char *Format,...)
{
	if(LogFileDisable)return;
	char	buf[256];
	va_list args;
	va_start(args,Format);
	vsnprintf(buf,256,Format,args);
#ifdef DEBUGFILE_ENABLE
	FILE *fp = fopen(LOGFILENAME,"a");
	vfprintf(fp,Format,args);
	fprintf(fp,"\n");
	fclose(fp);
#endif
	va_end(args);
//	printfDx(buf);
//	printfDx("\n");
}


int AppLogAdd_GraphicData(int gh)/*グラフィックハンドルの情報を書き出す。*/
{
	DXPTEXTURE2 *ptr = GraphHandle2TexPtr(gh);
	if(ptr == NULL)
	{
		AppLogAdd2("無効なハンドルです。\n");
		return 0;
	}

	if(ptr->vramflag)
	{
		AppLogAdd2(
			"VRAM上に配置されたデータです。\n"
			"管理番号		：%d\n"
			"テクスチャ形式	：%d\n"
			"有効サイズ		：%d,%d\n"
			"メモリ上サイズ	：%d,%d,%d\n"
			"VRAMオフセット	：%#x\n",
			gh,
			ptr->psm,
			ptr->umax,ptr->vmax,
			ptr->width,ptr->height,ptr->pitch,
			ptr->pvram->offset
			);
		return 0;
	}
	else
	{
		AppLogAdd2(
			"DDR上に配置されたデータです。\n"
			"管理番号		：%d\n"
			"テクスチャ形式	：%d\n"
			"有効サイズ		：%d,%d\n"
			"メモリ上サイズ	：%d,%d,%d\n"
			"ポインタ		：%#x\n",
			gh,
			ptr->psm,
			ptr->umax,ptr->vmax,
			ptr->width,ptr->height,ptr->pitch,
			ptr->pmemory
			);
		return 0;
	}

}
