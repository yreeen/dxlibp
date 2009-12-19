#include "dxlibp.h"
#include <pspmp3.h>
#include <psputility.h>

#define DXP_BUILDOPTION_SOUNDHANDLE_MAX 32

#define DXP_SOUNDCMD_NONE 0
#define DXP_SOUNDCMD_PLAY 1
#define DXP_SOUNDCMD_STOP 2
#define DXP_SOUNDCMD_EXIT 3

#define DXP_SOUNDFMT_MP3 1

typedef struct DXPAVCODEC_BUFFER
{
	u32 reserved0[6];
	u8* datIn;
	u32 frameSize0;
	u16* pcmOut;
	u32 decodeByte;//set 4608 (= 1152[sample per frame] * 2[byte per sample] * 2[channel])
	u32 frameSize1;
	u32 reserved1[54];
}DXPAVCODEC_BUFFER;

typedef struct DXPSOUNDHANDLE
{
	//ステータス
	unsigned used : 1;
	SceUID threadId;
	//メインスレッドからの指示
	unsigned loop : 1;
	unsigned cmd : 2;//DXP_SOUNDCMD_XXXX
	int gotoPos;
	int loopPos[2];//A-B in sample num.
	u8 volume;
	int pan;
	char filename[256];
	int soundDataType;

	//再生スレッドからの情報
	int length;
	int currentPos;
	int sampleRate;
	int id3v1;
	int id3v2;
	unsigned playing : 1;

	//デコード関連
	u8 format;
	u16* pcmOut;
	int pcmOutSize;
	union{
		struct
		{
			int handle;
			DXPAVCODEC_BUFFER *avBuf;
			u8 *mp3Buf;
			u32 mp3BufSize;
		}mp3;
	};
}DXPSOUNDHANDLE;

typedef struct DXPSOUNDDATA
{
	u8 init;
	u8 createSoundDataType;
}DXPSOUNDDATA;

extern DXPSOUNDHANDLE dxpSoundArray[];
extern DXPSOUNDDATA dxpSoundData;

int dxpSoundInit();
int dxpSoundTerm();
int dxpSoundReserveHandle();
int dxpSoundReleaseHandle(int handle);
int dxpSoundThread(SceSize datLen,void* datPtr);


int dxpSoundMp3Init(DXPSOUNDHANDLE *pHnd,int fh);
int dxpSoundMp3Seek(DXPSOUNDHANDLE *pHnd,int fh,int sample);
int dxpSoundMp3Decode(DXPSOUNDHANDLE *pHnd,int fh);
int dxpSoundMp3End(DXPSOUNDHANDLE *pHnd,int fh);
