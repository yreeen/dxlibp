#include <pspaudio.h>
#include "../tiny_mutex.h"
#include "../sound.h"

typedef struct DXPSOUND2SOUNDDATA
{
	int Mutex;
	int RefCount;
	int Type;
	union{
		struct
		{
			void* Data;
		}Mem;
		struct
		{
			DXPAVCONTEXT Context;
		}Stream;
	}Data;
	void (*Prefetch)(int NextPos);
	void (*GetData)();
}DXPSOUND2SOUNDDATA;

typedef struct DXPSOUND2HANDLE
{
	int Mutex;
	int RefCount;
	int Length;
	DXPSOUND2SOUNDDATA *MainData;
	DXPSOUND2SOUNDDATA *LoopData;
	int Volume;
	int Pan;
	int LoopPos[2];
	int NextOnlyVolume;
	int NextOnlyPan;
	int CurrentPos;
};

typedef struct DXPSOUND2CONTROL
{
	SceUID MessagePipeID;
	DXPSOUND2HANDLE HandleArray[DXP_BUILDOPTION_SOUNDHANDLE_MAX];
	DXPSOUND2SOUNDDATA SoundArray[DXP_BUILDOPTION_SOUNDHANDLE_MAX * 2];
}DXPSOUND2CONTROL;

typedef enum DXPSOUND2MESSAGETYPE
{
	DS2M_PLAY,
	DS2M_STOP,
	DS2M_ALLSTOP,
	DS2M_EXIT,
}DXPSOUND2MESSAGETYPE;

typedef struct DXPSOUND2MESSAGE
{
	DXPSOUND2MESSAGETYPE Type;
	int Handle;
	int Volume;
	int Pan;
	int StartPos;
};

extern DXPSOUND2CONTROL dxpSound2Control;


//------------------------------------------------

DXPSOUND2CONTROL dxpSound2Control;

typedef struct DXPSOUND2CHANNEL
{
}DXPSOUND2CHANNEL;

int dxpSoundThreadFunc(SceSize len,void* ptr)
{
	DXPSOUND2CHANNEL ChannelArray[PSP_AUDIO_CHANNEL_MAX];
}