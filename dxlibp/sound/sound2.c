#include <pspaudio.h>
#include "../tiny_mutex.h"
#include "../sound.h"

int dxpSoundCodec2InitMP3(DXPAVCONTEXT *context)
{
	if(!context)return -1;
	return dxpSoundMp3Init(&pHnd->avContext);
}

int dxpSoundCodec2GetSampleLength(DXPAVCONTEXT *context)
{
	if(!context)return -1;
	switch(context->format)
	{
	case DXP_SOUNDFMT_MP3:
		return dxpSoundMp3GetSampleLength(&pHnd->avContext);
	default:
		return -1;
	}
}

int dxpSoundCodec2Seek(DXPAVCONTEXT *context,int sample)
{
	if(!context)return -1;
	switch(context->format)
	{
	case DXP_SOUNDFMT_MP3:
		return dxpSoundMp3Seek(context,sample);
	default:
		return -1;
	}
}

int dxpSoundCodec2Decode(DXPAVCONTEXT *context)
{
	if(!context)return -1;
	switch(context->format)
	{
	case DXP_SOUNDFMT_MP3:
		return dxpSoundMp3Decode(context);
	default:
		return -1;
	}
}

int dxpSoundCodec2End(DXPAVCONTEXT *context)
{
	if(!context)return -1;
	switch(context->format)
	{
	case DXP_SOUNDFMT_MP3:
		return dxpSoundMp3End(context);
	default:
		return -1;
	}
}


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

int dxpSound2PrefetchFromHandle(DXPSOUND2HANDLE *ptr,int next)
{
	DXPSOUND2SOUNDDATA *sdata = NULL;
	if(next >= ptr->Length)return -1;
	if(next >= ptr->MainData->Length)
	{
		if(!ptr->LoopData)return -1;
		sdata = ptr->LoopData;
		next -= ptr->MainData->Length;
	}else
		sdata = ptr->MainData;
	switch(sdata->Type)
	{
	case DX_SOUNDDATATYPE_FILE:
		sdata->Data.Stream.Context.
		break;
	case DX_SOUNDDATATYPE_MEMNOPRESS:
		break;
	default:
		return -1;
	}
	return sdata->SampleElement;
}
int dxpSound2GetDataFromHandle(DXPSOUND2HANDLE *ptr,int next);



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
	int a,b;
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

			if(ChannelArray[ci].PlayType == DX_PLAYTYPE_LOOP)
			{
				if(dxpSound2Control.HandleArray[ChannelArray[ci].Handle].LoopData)
				{
					if(ChannelArray[ci].NextPos >= dxpSound2Control.HandleArray[ChannelArray[ci].Handle].Length)
						ChannelArray[ci].NextPos = dxpSound2Control.HandleArray[ChannelArray[ci].Handle].MainData->Length;
				}else
				{
					
					if(dxpSound2Control.HandleArray[ChannelArray[ci].Handle].LoopPos[1] < 0)
						b = dxpSound2Control.HandleArray[ChannelArray[ci].Handle].Length;
					else 
						b = dxpSound2Control.HandleArray[ChannelArray[ci].Handle].LoopPos[1];
					if(dxpSound2Control.HandleArray[ChannelArray[ci].Handle].LoopPos[0] < 0)
						a = 0;
					else 
						a = dxpSound2Control.HandleArray[ChannelArray[ci].Handle].LoopPos[0];
					if(ChannelArray[ci].NextPos >= b)
						ChannelArray[ci].NextPos = a;
				}
			}
			else
			{
				if(ChannelArray[ci].NextPos >= dxpSound2Control.HandleArray[ChannelArray[ci].Handle].Length)
					ChannelArray[ci].StopFlag = 1;
			}
			//プリフェッチ


			tmUnlock(dxpSound2Control.HandleArray[ChannelArray[ci].Handle].Mutex);
		}
	}
	sceKernelExitDeleteThread(0);
	return 0;
}

