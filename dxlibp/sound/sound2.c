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
	int SampleElement;//1024とか1152とか
	int Length;
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
}DXPSOUND2HANDLE;



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
	int PlayType;
	int Volume;
	int Pan;
	int StartPos;
}DXPSOUND2MESSAGE;

extern DXPSOUND2CONTROL dxpSound2Control;


//------------------------------------------------

DXPSOUND2CONTROL dxpSound2Control;

typedef struct DXPSOUND2CHANNEL
{
	int Channel;
	int Handle;
	int Volume;
	int Pan;
	int PlayType;
	int CurrentPos;
	int NextPos;
	int StopFlag;
	int IsHandleCurrentPosResponsible;
}DXPSOUND2CHANNEL;

int dxpSoundThreadFunc(SceSize len,void* ptr)
{
	DXPSOUND2CHANNEL ChannelArray[PSP_AUDIO_CHANNEL_MAX];
	int ci;
	int vol,pan;
	for(ci = 0;ci < PSP_AUDIO_CHANNEL_MAX;++ci)
		ChannelArray[ci].Channel = -1;

	while(1)
	{
		DXPSOUND2MESSAGE msg;
		while(sceKernelTryReceiveMsgPipe(dxpSound2Control.MessagePipeID,&msg,sizeof(msg),0,0) >= 0)
		{
			switch(msg.Type)
			{
			case DS2M_PLAY:
				for(ci = 0;ci < PSP_AUDIO_CHANNEL_MAX;++ci)
					if(ChannelArray[ci].Channel < 0)break;
				ChannelArray[ci].Channel = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL,1024,PSP_AUDIO_FORMAT_STEREO);
				if(ChannelArray[ci].Channel < 0)break;
				ChannelArray[ci].CurrentPos = msg.StartPos;
				ChannelArray[ci].NextPos = msg.StartPos;
				ChannelArray[ci].Handle = msg.Handle;
				ChannelArray[ci].Pan = msg.Pan;
				ChannelArray[ci].PlayType = msg.PlayType;
				ChannelArray[ci].Volume = msg.Volume;
				ChannelArray[ci].StopFlag = 0;
				tmLock(dxpSound2Control.HandleArray[ChannelArray[ci].Handle].Mutex);
				dxpSound2Control.HandleArray[ChannelArray[ci].Handle].RefCount += 1;
				ChannelArray[ci].IsHandleCurrentPosResponsible = dxpSound2Control.HandleArray[ChannelArray[ci].Handle].RefCount == 1 ? 1 : 0;
				if(dxpSound2Control.HandleArray[ChannelArray[ci].Handle].MainData != NULL)
				{
					tmLock(dxpSound2Control.HandleArray[ChannelArray[ci].Handle].MainData->Mutex);
					//プリフェッチを行う。
					tmUnlock(dxpSound2Control.HandleArray[ChannelArray[ci].Handle].MainData->Mutex);
				}
				tmUnlock(dxpSound2Control.HandleArray[ChannelArray[ci].Handle].Mutex);
				break;
			case DS2M_STOP:
				for(ci = 0;ci < PSP_AUDIO_CHANNEL_MAX;++ci)
					if(ChannelArray[ci].Handle == msg.Handle)
						ChannelArray[ci].StopFlag = 1;
				break;
			case DS2M_ALLSTOP:
				for(ci = 0;ci < PSP_AUDIO_CHANNEL_MAX;++ci)
					ChannelArray[ci].StopFlag = 1;
				break;
			case DS2M_EXIT:
				for(ci = 0;ci < PSP_AUDIO_CHANNEL_MAX;++ci)
				{
					if(ChannelArray[ci].Channel < 0)continue;
					while(sceAudioGetChannelRestLen(ChannelArray[ci].Channel) > 0)sceKernelDelayThread(1000);
					sceAudioChRelease(ChannelArray[ci].Channel);
					tmLock(dxpSound2Control.HandleArray[ChannelArray[ci].Handle].Mutex);
					dxpSound2Control.HandleArray[ChannelArray[ci].Handle].RefCount -= 1;
					tmUnlock(dxpSound2Control.HandleArray[ChannelArray[ci].Handle].Mutex);
				}
				sceKernelExitDeleteThread(0);
			default:
				break;
			}
		}

		for(ci = 0;ci < PSP_AUDIO_CHANNEL_MAX;++ci)
		{
			//このループの流れ
			//チャンネルのバッファが空でないならcontinue
			//終了すべきか？なら解放
			//次のデータを取ってきて再生
			//次のデータの位置を探して、必要ならPrefetch
			if(sceAudioGetChannelRestLength(ChannelArray[ci].Channel) > 0)continue;
			if(ChannelArray[ci].StopFlag)
			{
				sceAudioChRelease(ChannelArray[ci].Channel);
				ChannelArray[ci].Channel = -1;
				tmLock(dxpSound2Control.HandleArray[ChannelArray[ci].Handle].Mutex);
				dxpSound2Control.HandleArray[ChannelArray[ci].Handle].RefCount -= 1;
				tmUnlock(dxpSound2Control.HandleArray[ChannelArray[ci].Handle].Mutex);
				continue;
			}
			tmLock(dxpSound2Control.HandleArray[ChannelArray[ci].Handle].Mutex);
			vol = ChannelArray[ci].Volume >= 0 ? ChannelArray[ci].Volume : dxpSound2Control.HandleArray[ChannelArray[ci].Handle].Volume;
			pan = -10000 <= ChannelArray[ci].Pan && ChannelArray[ci].Pan <= 10000 ? ChannelArray[ci].Pan : dxpSound2Control.HandleArray[ChannelArray[ci].Handle].Pan;
			sceAudioOutputPanned(
				ChannelArray[ci].Channel,
				PSP_AUDIO_VOLUME_MAX * (pan > 0 ? 1.0f - pan / 10000.0f : 1.0f) * vol / 255.0f,
				PSP_AUDIO_VOLUME_MAX * (pan < 0 ? 1.0f + pan / 10000.0f : 1.0f) * vol / 255.0f,
				/*データもってこい*/0);
			ChannelArray[ci].CurrentPos = ChannelArray[ci].NextPos;
			if(ChannelArray[ci].IsHandleCurrentPosResponsible)
				dxpSound2Control.HandleArray[ChannelArray[ci].Handle].CurrentPos = ChannelArray[ci].CurrentPos;


			
			ChannelArray[ci].NextPos += 1024;//ここ要修正。SampleElement


			//ループとかどうすんのかここに書く

			//プリフェッチ


			tmUnlock(dxpSound2Control.HandleArray[ChannelArray[ci].Handle].Mutex);
		}
	}
	sceKernelExitDeleteThread(0);
	return 0;
}