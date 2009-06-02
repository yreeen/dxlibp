#ifndef DXPMUSIC_H__
#define DXPMUSIC_H__
#include "dxlibp.h"
#include <pspaudiocodec.h>
#include <pspaudio.h>
#include <psputility.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>

/*
データを保持する場所
	ハンドル管理配列
	スレッド内

データ項目
	ファイルストリーム
	ファイルタイプ
	スレッドID
	再生チャネル
	パン、ボリューム
	ストリームか通常か
	再生バッファ
	コーデックバッファ


通常再生の流れ
	ファイル情報取得
	全部解凍
	スレッド生成
	スレッドが必要な部分を使用
	データ破棄には参照カウンタを使う

ストリーム再生の流れ
	ファイル情報取得
	スレッド生成
	スレッドが必要な部分を解凍しつつ使用

データ破棄方法
	破棄フラグを立てる
	参照カウンタが０になった時点でメインスレッドが破棄処理を行う

作る関数リスト
LoadSoundMem		//通常再生向けにファイルをメモリに読み込む
LoadStreamSoundMem	//ストリーム再生向けにファイルをオープンする。
PlaySoundMem		//再生開始
DeleteSoundMem		//ハンドルを消去する（ためのフラグを立てる
GetPlayNumSoundMem	//いまいくつ再生されているのか取得する
CheckSoundMem		//再生されているかどうか取得する。GetPlayNumSoundMemが非ゼロかどうかというだけ

デコードはデコード用コンテクストを使う。
通常再生とストリーム再生で関数を分ける。

*/

typedef enum
{
	DXPMFT_WAVE = 0,
	DXPMFT_MP3,

	DXPMFT_MAX
}MUSICFILE_TYPE;



typedef struct
{
	unsigned long *codec_buffer;
}DXP_MUSICUNION_MP3STREAM;

typedef struct
{
	//44.1kHz前提とする。それ以外は弾く。
	unsigned monoflag:1;
	unsigned s8bitflag:1;
}DXP_MUSICUNION_WAVESTREAM;

#define MUSICHANDLE_MAX	64
#define SOUNDDATA_INDEX MUSICHANDLE_MAX

#define MUSICFLAG_USED		0x0001	//使用中


typedef struct
{
	MUSICFILE_TYPE filetype;
	int nextframe;
//	STREAMDATA Src;
	STREAMDATA *src;
	u16 *pcm;
	union
	{
		DXP_MUSICUNION_WAVESTREAM	wavestream;
		DXP_MUSICUNION_MP3STREAM	mp3stream;
	};
	u16 *out;
}DXP_MUSICDECODECONTEXT;



//20090410 フラグ関連の変更
/*旧
#define DXP_MUSICFLAG_STREAM	0x00000001
#define DXP_MUSICFLAG_PLAYING	0x00000002
#define DXP_MUSICFLAG_LOOP		0x00000004
#define DXP_MUSICCOMMAND_PLAY	0x80000000
#define DXP_MUSICCOMMAND_STOP	0x40000000	//とまれ！
#define DXP_MUSICCOMMAND_QUIT	0x20000000
*/
//新
#define DXP_MUSICFLAG_STREAM			0x00000001
#define DXP_MUSICFLAG_PLAYING			0x00000002
#define DXP_MUSICFLAG_LOOP				0x00000004

#define DXP_MUSICCOMMAND_PLAY			0x80000000
#define DXP_MUSICCOMMAND_STOP			0x40000000	//とまれ！
#define DXP_MUSICCOMMAND_QUIT			0x20000000
#define DXP_MUSICCOMMAND_TOP			0x08000000 //20090425add

#define DXP_MUSICTHREADPARAMFLAG_NORMAL	0x00000010
#define DXP_MUSICTHREADPARAMFLAG_BACK	0x00000020
#define DXP_MUSICTHREADPARAMFLAG_LOOP	0x00000040

typedef struct MUSICDATA__	//音楽データのハンドル用構造体
{
	//20090412
	//管理に関するエリアをメモリ断片化を防ぐためにmallocで
	//確保しない方式に変更
	//struct MUSICDATA__ *next;
	unsigned useflg:1;		//0)空き 1)割り当て済み
	int apos,bpos;	//リピート位置。A-Bリピートに対応させる。
	u32 flag;		//通常/ストリーム リピート 再生/停止
	int handle;
	int count;		//参照カウンタ
	u16 *pcm;		//通常ストリームの場合のみ非NULL;
	int pcmlen;		//何サンプルあるのか
	u8 volume[4];	//ボリューム 全体、左、右の順 最大100
	STREAMDATA Src;
}MUSICDATA;

//20090410
//上の修正部分に移動
//#define DXP_MUSICTHREADPARAMFLAG_LOOP	0x01
typedef struct
{
	MUSICDATA *ptr;
	u8 flag;
}MUSICDATA_THREADPARAM;

typedef struct
{
	MUSICDATA *md;
	u8 flag;
	DXP_MUSICDECODECONTEXT context;
}MUSICDATA_THREADPARAM_STREAM;

int getfreehandle();//空きハンドルの探索
int reserveoutchannel(int handle);//空き出力チャンネルの取得
int releaseoutchannel(int handle);
int musicthread(SceSize arglen,void* argp);
int id3skip(STREAMDATA *src);

int waveseek(DXP_MUSICDECODECONTEXT *context);
int decodeprepare_wave(DXP_MUSICDECODECONTEXT *context);
int decode_wave(DXP_MUSICDECODECONTEXT *context);
int decodefinish_wave(DXP_MUSICDECODECONTEXT *context);

int mp3seek(STREAMDATA *src,int frame);
int mp3len(STREAMDATA *src);
int mp3framesize(int header);
int decodeprepare_mp3(DXP_MUSICDECODECONTEXT *context);
int decode_mp3(DXP_MUSICDECODECONTEXT *context);
int decodefinish_mp3(DXP_MUSICDECODECONTEXT *context);


int readframe(DXP_MUSICDECODECONTEXT *context);

inline int sample_per_frame(MUSICFILE_TYPE type);

#endif
