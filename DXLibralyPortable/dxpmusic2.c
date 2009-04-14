/*
*	DXライブラリPortable	音楽再生制御部	Ver1.0.1
*	製作者	：憂煉、シューティング大好き
*	備考	：
*/

/*
データ読み込み　失敗したら再オープンを試みる
デコード

//モノラルの場合でもかならずステレオで出力する。


デコード系関数おおかたできあがり。
再生スレッドとかを作成中。


ストリームのオープン
再生前の調整

再生位置確認、修正
デコード
再生
ループ

終了前処理
ストリームのクローズ



通常再生
	同時再生可能

ストリーム再生
	同時再生不可能
	省メモリ

再生コンテキストを使う方法に切り替える。
全体のボリューム調整は・・・本体の機能でいいか

ちょっとDXライブラリから仕様がかわるけど…いっか　DXライブラリのは機能追加の繰り返しでわけわかんないし




ラッパー関数
PlaySoundFile
CheckSoundFile		
StopSoundFile
SetVolumeSoundFile

*/

//インクルード

#include "dxpmusic2.h"

//宣言

int musicthread_normal(SceSize arglen,void* argp);
int musicthread_stream(SceSize arglen,void *argp);
int decodeprepare(DXP_MUSICDECODECONTEXT *context);
int decode(DXP_MUSICDECODECONTEXT *context);
int decodefinish(DXP_MUSICDECODECONTEXT *context);
//変数定義

MUSICDATA *musicdataroot = NULL;

//20090410
//音ファイル管理領域の変更に伴い追加
#define MusicTableMAX 32
//20090412
//管理に関するエリアをメモリ断片化を防ぐためにmallocで
//確保しない方式に変更
//static MUSICDATA* MusicTable[MusicTableMAX];
static MUSICDATA MusicTable[MusicTableMAX];

//関数定義

inline static int sample_per_frame(MUSICFILE_TYPE type)
{
	switch(type)
	{
	case DXPMFT_WAVE:
		return 1024;
	case DXPMFT_MP3:
		return 1152;
	default:
		return -1;
	}
}

MUSICDATA* Handle2MusicDataPtr(int handle)
{
	if ( handle <				0 ) return NULL;
	if ( handle >=	MusicTableMAX ) return NULL;
	//20090410
	//音ファイル管理用域の変更
	/*MUSICDATA *ptr = musicdataroot;
	while(ptr != NULL)
	{
		if(ptr->handle == handle)return ptr;
		ptr = ptr->next;
	}
	return ptr;*/
	//20090412
	//管理に関するエリアをメモリ断片化を防ぐためにmallocで
	//確保しない方式に変更
	//return MusicTable[handle];
	return &MusicTable[handle];
}

MUSICDATA* AddMusicData()
{
	//20090412
	//管理に関するエリアをメモリ断片化を防ぐためにmallocで
	//確保しない方式に変更
	//MUSICDATA *ptr = (MUSICDATA*)malloc(sizeof(MUSICDATA));
	//if(ptr == NULL)return NULL;
	//20090410
	//音ファイル管理用域の変更
	//MUSICDATA *ptr2 = musicdataroot;
	//ptr->handle = 0;
	//while(ptr2 != NULL)
	//{
	//	if(ptr->handle < ptr2->handle)ptr->handle = ptr2->handle + 1;
	//	ptr2 = ptr2->next;
	//}
	int i;
	//20090412
	//管理に関するエリアをメモリ断片化を防ぐためにmallocで
	//確保しない方式に変更
	//for(i=0;i<MusicTableMAX;i++)
	//{
	//	if(MusicTable[i] == NULL) break;
	//}
	for(i=0;i<MusicTableMAX;i++)
	{
		if(MusicTable[i].useflg == 0) break;
	}
	if(i >= MusicTableMAX) return NULL;
	MUSICDATA *ptr = &MusicTable[i];
	//下の行は将来的に削除の方向で？
	memset(&MusicTable[i],0x00,sizeof(MusicTable[0]));
	ptr->handle = i;
	//ptr->apos = ptr->bpos = 0;
	//ptr->flag = 0;
	//ptr->pcm = NULL;
	//ptr->pcmlen = 0;
	//下の行は将来的に削除の方向で
	//ptr->next = NULL;//musicdataroot;
	ptr->volume[0] = ptr->volume[1] = ptr->volume[2] = 100;
	//ptr->count = 0;
	ptr->useflg = 1;
	//musicdataroot = ptr;
	//20090412
	//管理に関するエリアをメモリ断片化を防ぐためにmallocで
	//確保しない方式に変更
	//MusicTable[i] = ptr;
	return ptr;
}
int AddMusicDataH()
{
	MUSICDATA *ptr = AddMusicData();
	if(ptr == NULL)return -1;
	return ptr->handle;
}
int RemoveMusicDataH(int handle)
{
	if ( handle <				0 ) return -1;
	if ( handle >=	MusicTableMAX ) return -2;
	//20090412
	//管理に関するエリアをメモリ断片化を防ぐためにmallocで
	//確保しない方式に変更
	//if ( MusicTable[handle] != NULL) goto rm;
	if ( MusicTable[handle].useflg == 0) return -3;
	//20090410
	//音ファイル管理用域の変更
	//MUSICDATA *ptr = musicdataroot;
	//MUSICDATA *ptr2 = NULL;
	//while(ptr != NULL)
	//{
	//	if(ptr->handle == handle)goto rm;
	//	ptr2 = ptr;
	//	ptr = ptr->next;
	//}
	//return -3;
//rm:
	//20090410
	//音ファイル管理用域の変更
	//if(ptr2 != NULL)ptr2->next = ptr->next;
	//else musicdataroot = ptr->next;
	//free(ptr->pcm);
	//free(ptr);
	//free(MusicTable[handle]->pcm);
	free(MusicTable[handle].pcm);
	//20090412
	//管理に関するエリアをメモリ断片化を防ぐためにmallocで
	//確保しない方式に変更
	//free(MusicTable[handle]);
	//MusicTable[handle] = NULL;
	MusicTable[handle].useflg = 0;
	return 0;
}
int RemoveMusicData(MUSICDATA *ptr)
{
	if(ptr == NULL)return -1;
	return RemoveMusicDataH(ptr->handle);
}

int LoadSoundMem(const char* filename)
{
	MUSICDATA *md = NULL;
	int length = 0;
	u16 *data = NULL;
	//デコード処理
	DXP_MUSICDECODECONTEXT context;
	STREAMDATA Src;
	context.src = &Src;
	SetupSTREAMDATA(filename,context.src);
	if(decodeprepare_mp3(&context) != -1)goto mp3dec;
	
	return -1;
mp3dec:
	length = mp3len(context.src);
	if(length <= 0)
	{
		decodefinish_mp3(&context);
		return -1;
	}
	data = (u16*)memalign(64,length * 2 * 2);
	if(data == NULL)
	{
		decodefinish_mp3(&context);
		return -1;
	}
	int i;
	STSEEK(context.src,0,SEEK_SET);
	for(i = 0;i < length / 1152;++i)
	{
		context.out = data + i * 2 * 1152;
		if(decode_mp3(&context) == -1)
		{
			break;
		}
	}
	decodefinish_mp3(&context);
	goto lad;
	return -1;
lad:
	md = AddMusicData();
	if(md == NULL)
	{
		free(data);
		return -1;
	}
	md->pcm = data;
	md->bpos = md->pcmlen = length;
	return md->handle;
}

int PlaySoundMem(int handle,int PlayType)
{
	MUSICDATA *md = Handle2MusicDataPtr(handle);
	if(md == NULL)return -1;
	md->flag &= ~DXP_MUSICCOMMAND_STOP;
	md->flag |= DXP_MUSICCOMMAND_PLAY;
	MUSICDATA_THREADPARAM param;
	param.ptr = md;
	param.flag = 0;
	if(PlayType == DX_PLAYTYPE_NORMAL)
	{
		//スレッドの処理タイプを設定
		//20090410
		param.flag |= DXP_MUSICTHREADPARAMFLAG_NORMAL;
		return musicthread_normal(sizeof(MUSICDATA_THREADPARAM),&param);
	}
	//フラグの変更
	//20090410
	if(PlayType == DX_PLAYTYPE_LOOP)param.flag |= DXP_MUSICTHREADPARAMFLAG_LOOP;
	//PSP SDK書いてる人たちが作ってるサウンド系ライブラリが
	//スレッドプライオリティを0x11にしてるのでそれを参考に 20090410
	int thid = sceKernelCreateThread("soundthread",musicthread_normal,0x11,0x4000,THREAD_ATTR_USER,NULL);
	if(0 > thid)return -1;
	sceKernelStartThread(thid,sizeof(MUSICDATA_THREADPARAM),&param);
	return 0;
}

int StopSoundMem(int handle)
{
	MUSICDATA *md = Handle2MusicDataPtr(handle);
	if(md == NULL)return -1;
	md->flag &=~ DXP_MUSICCOMMAND_PLAY;
	md->flag |= DXP_MUSICCOMMAND_STOP;
	return 0;
}

#define SAMPLECOUNT_DEFAULT		1024
int musicthread_normal(SceSize arglen,void* argp)
{
	MUSICDATA_THREADPARAM *param = (MUSICDATA_THREADPARAM*)argp;
	if(param == NULL)return -1;
	MUSICDATA *ptr = param->ptr;
	if(ptr == NULL)return -1;
	if(ptr->flag & DXP_MUSICFLAG_STREAM)return -1;
	u16 *buf = (u16*)memalign(64,SAMPLECOUNT_DEFAULT * 4);
	if(buf == NULL)return -1;
	++ptr->count;
	param->ptr->flag |= DXP_MUSICFLAG_PLAYING;
	int pos = 0;
	int reservedchannel = -1;
	int play = 1;

	reservedchannel = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL,SAMPLECOUNT_DEFAULT,PSP_AUDIO_FORMAT_STEREO);
	while(play && reservedchannel >= 0 && !(ptr->flag & DXP_MUSICCOMMAND_STOP))//ストップコマンドが出されていなければ再生続行
	{
		float vleft,vright;
		vleft = vright = PSP_AUDIO_VOLUME_MAX;
		vleft *= ptr->volume[0] / 100.0f;
		vright *= ptr->volume[0] / 100.0f;
		vleft *= ptr->volume[1] / 100.0f;
		vright *= ptr->volume[2] / 100.0f;

		int i;
		for(i = 0;i < SAMPLECOUNT_DEFAULT;++i)
		{
			if(pos >= ptr->pcmlen || (ptr->bpos != 0 && pos >= ptr->bpos))
			{
				if(param->flag & DXP_MUSICTHREADPARAMFLAG_LOOP)
					pos = ptr->apos;
				else
					break;
			}
			buf[i * 2] = ptr->pcm[pos * 2];
			buf[i * 2 + 1] = ptr->pcm[pos * 2 + 1];
			++pos;
		}
		if(i < SAMPLECOUNT_DEFAULT)
		{
			play = 0;
			sceAudioSetChannelDataLen(reservedchannel,i);
		}
		else
			sceAudioSetChannelDataLen(reservedchannel,SAMPLECOUNT_DEFAULT);
		sceAudioOutputPannedBlocking(reservedchannel,vleft,vright,buf);
	}
	if(reservedchannel >= 0)sceAudioChRelease(reservedchannel);
	reservedchannel = -1;

	--ptr->count;
	if(ptr->count <= 0)ptr->flag &=~ DXP_MUSICFLAG_PLAYING;
	free(buf);
	//本関数がスレッドとして起動されていた場合に自スレッドを破棄する処理を追加 20090410
	if(ptr->flag & DXP_MUSICTHREADPARAMFLAG_NORMAL) return 0;
	sceKernelExitDeleteThread(0);
	return 0;
}


int LoadStreamSound(const char *filename,int SetPcmLen,int* AnsPcmLen)
{
	MUSICDATA_THREADPARAM_STREAM param,*p;
	p = &param;
	if((param.md = AddMusicData()) == NULL)return -1;
	if(SetupSTREAMDATA(filename,param.context.src = &param.md->Src) == -1)
	{
		RemoveMusicData(param.md);
		return -1;
	}
	param.md->flag |= DXP_MUSICFLAG_STREAM;
	if(decodeprepare_mp3(&param.context) != -1)goto mp3setup;
	STCLOSE(param.context.src);
	RemoveMusicData(param.md);
	return -1;	
mp3setup:
	if(SetPcmLen <= 0)
	//Pcmの長さがセットされてない(デフォルトは-1)場合は
	//自前でmp3フォーマットを調べてpcm時の長さを解析
	//AnsPcmLenにアドレスがあった場合はPcmの長さをセットする
	//Pcmの長さがセットされていれば解析をせず入力された数字
	//をそのままセットする。
	{
		//通常の処理
		param.md->bpos = param.md->pcmlen = mp3len(param.context.src);
		if(AnsPcmLen !=NULL) *AnsPcmLen = param.md->pcmlen;
	}
	else
	{
		param.md->bpos = param.md->pcmlen = SetPcmLen;
	}
	//param.md->pcmlen = mp3len(param.context.src); 2回同じ処理をしてるのでコメント 20090410

	//PSP SDK書いてる人たちが作ってるサウンド系ライブラリが
	//スレッドプライオリティを0x11にしてるのでそれを参考に 20090410
	int thid = sceKernelCreateThread("soundthreads",musicthread_stream,0x11,0x4000,THREAD_ATTR_USER,NULL);
	if(0 > thid)
	{
		decodefinish(&param.context);
		RemoveMusicData(param.md);
		return -1;
	}
	sceKernelStartThread(thid,sizeof(MUSICDATA_THREADPARAM_STREAM),p);
	return param.md->handle;
}

int PlayStreamSound(int handle,int playtype)
{
	MUSICDATA *md = Handle2MusicDataPtr(handle);
	if(md == NULL)
	{
		return -1;
	}
	if(!md->flag & DXP_MUSICFLAG_STREAM)
	{
		return -1;
	}
	//if(playtype == DX_PLAYTYPE_LOOP)md->flag |= DXP_MUSICFLAG_LOOP;
	//else md->flag &=~ DXP_MUSICFLAG_LOOP;
	//フラグの種類を変更 20090410
	if(playtype == DX_PLAYTYPE_LOOP)md->flag |= DXP_MUSICTHREADPARAMFLAG_LOOP;
	else md->flag &=~ DXP_MUSICTHREADPARAMFLAG_LOOP;
	md->flag &=~ DXP_MUSICCOMMAND_STOP;
	md->flag |= DXP_MUSICCOMMAND_PLAY;
	if(playtype == DX_PLAYTYPE_NORMAL)
	{
		//ストリーム処理用タスクがスレッド休止から復活するまでの時間稼ぎを追加 20090410
		sceKernelDelayThread(100000);
		while(md->flag & DXP_MUSICFLAG_PLAYING)sceKernelDelayThread(1000);
	}
	return 0;
}


int musicthread_stream(SceSize arglen,void *argp)
{
	MUSICDATA_THREADPARAM_STREAM *ptr = (MUSICDATA_THREADPARAM_STREAM*)argp;
	if(ptr == NULL)
	{
		return -1;
	}
	int pos = 0;
	int reservedchannel = -1;
	int spf = sample_per_frame(ptr->context.filetype);
	u8 play = 0;
	u16 *playbuf,*outbuf,*buf;
	int bufsize = spf * 2 * sizeof(u16) * 2;
	if(bufsize <= 0)
	{
		decodefinish(&ptr->context);
		return -1;
	}
	buf = memalign(64,bufsize);
	if(buf == NULL)
	{
		decodefinish(&ptr->context);
		return -1;
	}
	reservedchannel = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL,spf,PSP_AUDIO_FORMAT_STEREO);
	playbuf = buf;
	outbuf = buf + spf * 2;
	++ptr->md->count;
	/*ClearDrawScreen();
	clsDx();
	printfDx("spf    = %d\n",spf);
	printfDx("count  = %d\n",ptr->md->count);
	printfDx("pos    = %d\n",pos);
	printfDx("apos   = %d\n",ptr->md->apos);
	printfDx("bpos   = %d\n",ptr->md->bpos);
	printfDx("pcmlen = %d\n",ptr->md->pcmlen);
	printfDx("next f = %d\n",ptr->context.nextframe);
	ScreenFlip();*/
	while( reservedchannel >= 0)
	{
		if(ptr->md->flag & DXP_MUSICCOMMAND_QUIT)break;
		if(play)
		{
			if(ptr->md->flag & DXP_MUSICCOMMAND_STOP)
			{
				play = 0;
				ptr->md->flag &=~ DXP_MUSICFLAG_PLAYING;
				continue;
			}
			ptr->context.nextframe = -1;
			if((pos >= ptr->md->bpos && ptr->md->bpos != 0) || (pos >= ptr->md->pcmlen && ptr->md->pcmlen != 0))
			{
				if(ptr->md->flag & DXP_MUSICTHREADPARAMFLAG_LOOP)
				{
					pos = ptr->md->apos - ptr->md->apos % spf;
					ptr->context.nextframe = pos / spf;
				}
				else
				{
					play = 0;
					ptr->md->flag &=~ DXP_MUSICFLAG_PLAYING;//20090410
															//これがないとDX_PLAYTYPE_NORMAL
															//で再生終了の際にメインタスクが次へ進めない
					continue;
				}
			}
			float vleft,vright;
			vleft = vright = PSP_AUDIO_VOLUME_MAX;
			vleft *= ptr->md->volume[0] / 100.0f;
			vright *= ptr->md->volume[0] / 100.0f;
			vleft *= ptr->md->volume[1] / 100.0f;
			vright *= ptr->md->volume[2] / 100.0f;

			ptr->context.out = outbuf;
			if(decode(&ptr->context) == -1)goto err;
			u16 *tmp = playbuf;
			playbuf = outbuf;
			//outbuf = playbuf; 報告のあったバグ部分の修正 20090410
			outbuf = tmp;
			while(sceAudioGetChannelRestLength(reservedchannel) > 0)sceKernelDelayThread(500);
			sceAudioOutputPanned(reservedchannel,(int)vleft,(int)vright,playbuf);
			pos += spf;
		}
		else
		{
			if(ptr->md->flag & DXP_MUSICCOMMAND_PLAY)
			{
				ptr->md->flag &=~ DXP_MUSICCOMMAND_PLAY;//20090410
														//これがないとデッドループに入る
				play = 1;
				ptr->md->flag |= DXP_MUSICFLAG_PLAYING;
				continue;
			}
			sceKernelDelayThread(3000);
		}
	}
	if(reservedchannel >= 0)sceAudioChRelease(reservedchannel);
	--ptr->md->count;
	decodefinish(&ptr->context);
	ptr->md->flag &=~ DXP_MUSICFLAG_PLAYING;
	//スレッドの破棄処理追加 20090410
	sceKernelExitDeleteThread(0);
	return 0;
err:
	if(reservedchannel >= 0)sceAudioChRelease(reservedchannel);
	--ptr->md->count;
	decodefinish(&ptr->context);
	ptr->md->flag &=~ DXP_MUSICFLAG_PLAYING;
	//スレッドの破棄処理追加 20090410
	sceKernelExitDeleteThread(0);
	return -1;
}

int decodeprepare(DXP_MUSICDECODECONTEXT *context)
{
	if(context == NULL)return -1;
	if(context->src == NULL)return -1;
	if(decodeprepare_mp3(context) != -1)return 0;
	return -1;
}
int decode(DXP_MUSICDECODECONTEXT *context)
{
	if(context == NULL)return -1;
	switch(context->filetype)
	{
	case DXPMFT_MP3:
		return decode_mp3(context);
	default:
		return -1;
	}
}

int decodefinish(DXP_MUSICDECODECONTEXT *context)
{
	if(context == NULL)return -1;
	switch(context->filetype)
	{
	case DXPMFT_MP3:
		return decodefinish_mp3(context);
	default:
		return -1;
	}
}

int getRefCount(int sh)
{
	MUSICDATA *md = Handle2MusicDataPtr(sh);
	if(md == NULL)return -1;
	return md->count;
}

//関数の追加 20090410
int	CheckSoundMem( int handle )
{
	if( handle <	0				)	return -1;
	if( handle >=	MusicTableMAX	)	return -2;
	//20090412
	//管理に関するエリアをメモリ断片化を防ぐためにmallocで
	//確保しない方式に変更
	//if( MusicTable[handle]	== NULL)	return -3;
	//if( MusicTable[handle]->flag & DXP_MUSICFLAG_PLAYING) return 1;
	if( MusicTable[handle].useflg == 0)	return -3;
	if( MusicTable[handle].flag & DXP_MUSICFLAG_PLAYING) return 1;
	return 0;
}

//20090410
//以下枠だけとりあえず追加
//dxlibp.hにプロトタイプ宣言要追加
int	ChangeVolumeSoundMem( int VolumePal, int SoundHandle )
{
	return 0;
}

int	DeleteSoundMem( int SoundHandle, int LogOutFlag )
{
	return 0;
}

int	SetLoopPosSoundMem( int LoopTime, int SoundHandle )
{
	return 0;
}
