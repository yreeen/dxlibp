#include <pspaudio.h>
#include "../tiny_mutex.h"
#include "../sound.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#define DXP_BUILDOPTION_SOUNDDATA_MAX (DXP_BUILDOPTION_SOUNDHANDLE_MAX * 2)

#define for_each_c(ITR,ARRAY) for(ITR = ARRAY;ITR < ARRAY + (sizeof(ARRAY) / sizeof(ARRAY[0]));++ITR)



int dxpSoundCodec2Init(DXPAVCONTEXT *context)
{
	int status;
	if(!context)return -1;
	status = dxpSoundMp3Init(context);
	if(!(status < 0))return 0;
	return -1;
}

int dxpSoundCodec2GetSampleLength(DXPAVCONTEXT *context)
{
	if(!context)return -1;
	switch(context->format)
	{
	case DXP_SOUNDFMT_MP3:
		return dxpSoundMp3GetSampleLength(context);
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
	int used;
	int Mutex;
	int RefCount;
	int Type;
	union{
		struct
		{
			u32* Data;
		}Mem;
		struct
		{
			DXPAVCONTEXT Context;
			u32* OutputBuf[2];//dxpSafeAllocで確保
			int OutputDecoding;
		}Stream;
	}Data;
	int SampleElement;//1024とか1152とか
	int Length;
}DXPSOUND2SOUNDDATA;

typedef struct DXPSOUND2HANDLE
{
	int used;
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
		if(sdata->Data.Stream.Context.nextPos != next)
			dxpSoundCodec2Seek(&sdata->Data.Stream.Context,next);
		sdata->Data.Stream.OutputDecoding ^= 1;
		sdata->Data.Stream.Context.pcmOut = sdata->Data.Stream.OutputBuf[sdata->Data.Stream.OutputDecoding];
		dxpSoundCodec2Decode(&sdata->Data.Stream.Context);
		sdata->SampleElement = sdata->Data.Stream.Context.outSampleNum;
		break;
	case DX_SOUNDDATATYPE_MEMNOPRESS:
		sdata->SampleElement = 1024;
		break;
	default:
		return -1;
	}
	return sdata->SampleElement;
}
void* dxpSound2GetDataFromHandle(DXPSOUND2HANDLE *ptr,int next)
{
	DXPSOUND2SOUNDDATA *sdata = NULL;
	if(next >= ptr->Length)return NULL;
	if(next >= ptr->MainData->Length)
	{
		if(!ptr->LoopData)return NULL;
		sdata = ptr->LoopData;
		next -= ptr->MainData->Length;
	}else
		sdata = ptr->MainData;
	switch(sdata->Type)
	{
	case DX_SOUNDDATATYPE_FILE:
		dxpSoundCodec2Decode(&sdata->Data.Stream.Context);
		break;
	case DX_SOUNDDATATYPE_MEMNOPRESS:
		return ((u32*)sdata->Data.Mem.Data) + next;
		break;
	default:
		return NULL;
	}
	return NULL;
}



typedef struct DXPSOUND2CONTROL
{
	int init;
	SceUID MessagePipeID;
	DXPSOUND2HANDLE HandleArray[DXP_BUILDOPTION_SOUNDHANDLE_MAX];
	DXPSOUND2SOUNDDATA SoundArray[DXP_BUILDOPTION_SOUNDDATA_MAX];
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

int dxpSound2ReserveHandle()
{
	if(!dxpSound2Control.init)return -1;
	int i;
	DXPSOUND2HANDLE *pHnd;
	for(i = 0;i < DXP_BUILDOPTION_SOUNDHANDLE_MAX;++i)
	{
		if(dxpSound2Control.HandleArray[i].used)continue;
		pHnd = dxpSound2Control.HandleArray + i;
		memset(pHnd,0,sizeof(DXPSOUND2HANDLE));
		pHnd->MainData = NULL;
		pHnd->LoopData = NULL;
		pHnd->Volume = 255;
		pHnd->used = 1;
		return i;
	}
	return -1;
}

int dxpSound2ReleaseHandle(int handle)
{
	if(!dxpSound2Control.init)return -1;
	DXPSOUND2HANDLE *pHnd;
	if(handle < 0 || handle >= DXP_BUILDOPTION_SOUNDHANDLE_MAX)return -1;
	pHnd = dxpSound2Control.HandleArray + handle;
	pHnd->used = 0;
	return 0;
}

DXPSOUND2HANDLE* dxpSound2Handle2Ptr(int handle)
{
	if(!dxpSound2Control.init)return NULL;
	DXPSOUND2HANDLE *pHnd;
	if(handle < 0 || handle >= DXP_BUILDOPTION_SOUNDHANDLE_MAX)return NULL;
	pHnd = dxpSound2Control.HandleArray + handle;
	if(!pHnd->used)return NULL;
	return pHnd;
}

DXPSOUND2SOUNDDATA* dxpSound2ReserveData()
{
	if(!dxpSound2Control.init)return NULL;
	int i;
	DXPSOUND2SOUNDDATA *pData;
	for(i = 0;i < DXP_BUILDOPTION_SOUNDDATA_MAX;++i)
	{
		if(dxpSound2Control.SoundArray[i].used)continue;
		pData = dxpSound2Control.SoundArray + i;
		memset(pData,0,sizeof(DXPSOUND2SOUNDDATA));
		pData->used = 1;
		return pData;
	}
	return NULL;
}

void dxpSound2ReleaseData(DXPSOUND2SOUNDDATA* ptr)
{
	if(!dxpSound2Control.init)return;
	if(ptr == NULL)return;
	ptr->used = 0;
}


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
	int NextOutputLength;
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
					ChannelArray[ci].NextOutputLength = dxpSound2PrefetchFromHandle(dxpSound2Control.HandleArray + ChannelArray[ci].Handle,ChannelArray[ci].NextPos);
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
			sceAudioSetChannelDataLen(ChannelArray[ci].Channel,ChannelArray[ci].NextOutputLength);
			sceAudioOutputPanned(
				ChannelArray[ci].Channel,
				PSP_AUDIO_VOLUME_MAX * (pan > 0 ? 1.0f - pan / 10000.0f : 1.0f) * vol / 255.0f,
				PSP_AUDIO_VOLUME_MAX * (pan < 0 ? 1.0f + pan / 10000.0f : 1.0f) * vol / 255.0f,
				dxpSound2GetDataFromHandle(dxpSound2Control.HandleArray + ChannelArray[ci].Handle,ChannelArray[ci].NextPos));
			ChannelArray[ci].CurrentPos = ChannelArray[ci].NextPos;
			if(ChannelArray[ci].IsHandleCurrentPosResponsible)
				dxpSound2Control.HandleArray[ChannelArray[ci].Handle].CurrentPos = ChannelArray[ci].CurrentPos;
			ChannelArray[ci].NextPos += ChannelArray[ci].NextOutputLength;

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

			if(!ChannelArray[ci].StopFlag)
				ChannelArray[ci].NextOutputLength = dxpSound2PrefetchFromHandle(dxpSound2Control.HandleArray + ChannelArray[ci].Handle,ChannelArray[ci].NextPos);

			tmUnlock(dxpSound2Control.HandleArray[ChannelArray[ci].Handle].Mutex);
		}
	}
	sceKernelExitDeleteThread(0);
	return 0;
}

int LoadSoundMem_TEST(const char* filename)
{
	int i = 0;
	int fh = 0;
	DXPSOUND2HANDLE *pHnd = NULL;
	DXPAVCONTEXT context;
	if(!dxpSound2Control.init)return -1;
	int fsize = FileRead_size(filename);
	if(fsize <= 0)return -1;
	fh = FileRead_open(filename,0);
	if(fh == 0)return -1;
	int handle = dxpSound2ReserveHandle();
	pHnd = dxpSound2Handle2Ptr(handle);
	if(pHnd == NULL)goto err;
	
	pHnd->MainData = dxpSound2ReserveData();
	if(pHnd->MainData == NULL)goto err;
	pHnd->LoopData = NULL;
	pHnd->MainData->Type = dxpSoundData.createSoundDataType;
	switch(pHnd->MainData->Type)
	{
	case DX_SOUNDDATATYPE_MEMNOPRESS:
		context.fileHandle = fh;
		context.fileSize = fsize;
		if(dxpSoundCodec2Init(&context) < 0)
			goto err;
		pHnd->MainData->Length = dxpSoundCodec2GetSampleLength(&context);
		pHnd->MainData->Data.Mem.Data = memalign(64,pHnd->MainData->Length * 4);
		if(!pHnd->MainData->Data.Mem.Data)
		{
			dxpSoundCodec2End(&context);
			goto err;
		}
		memset(pHnd->MainData->Data.Mem.Data,0,pHnd->MainData->Length * 4);
		dxpSoundCodec2Seek(&context,0);
		for(i = 0;1;++i)
		{
			if(context.nextPos > pHnd->MainData->Length)break;
			context.pcmOut = pHnd->MainData->Data.Mem.Data + context.nextPos;
			if(dxpSoundCodec2Decode(&context) < 0)break;
		}
		dxpSoundCodec2End(&context);
		FileRead_close(fh);
		break;
	case DX_SOUNDDATATYPE_FILE:
		//未作成
		break;
	default:
		goto err;
	}

	return handle;
err:
	if(pHnd != NULL)
		dxpSound2ReleaseData(pHnd->MainData);
	dxpSound2ReleaseHandle(handle);
	FileRead_close(fh);
	return -1;
}

int DeleteSoundMem_TEST(int shandle)
{
	return -1;
}

int PlaySoundMem_TEST(int handle,int playtype,int rewindflag)
{
	DXPSOUND2MESSAGE msg;
	DXPSOUND2HANDLE *pHnd = dxpSound2Handle2Ptr(handle);
	if(pHnd == NULL)return -1;
	msg.PlayType = playtype;
	msg.Handle = handle;
	msg.Pan = -20000;
	if(-10000 <= pHnd->NextOnlyPan && pHnd->NextOnlyPan <= 10000)
	{
		msg.Pan = pHnd->NextOnlyPan;
		pHnd->NextOnlyPan = -20000;
	}
	msg.StartPos;
	msg.Volume = -1;
	if(pHnd->NextOnlyVolume > 0)
	{
		msg.Volume = pHnd->NextOnlyVolume;
		pHnd->NextOnlyVolume = -1;
	}
	msg.Type = DXPSOUND2MESSAGETYPE::DS2M_PLAY;
	
	sceKernelTrySendMsgPipe(dxpSound2Control.MessagePipeID,&msg,sizeof(msg),0,0);
//	DXPSOUNDHANDLE *pHnd;
//	SHND2PTR(handle,pHnd);
////	while(pHnd->cmd != DXP_SOUNDCMD_NONE)sceKernelDelayThread(100);
//	switch(pHnd->soundDataType)
//	{
//	case DX_SOUNDDATATYPE_MEMNOPRESS:
//		if(playtype == DX_PLAYTYPE_NORMAL)
//		{
//			int channel = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL,pHnd->avContext.outSampleNum,PSP_AUDIO_FORMAT_STEREO);
//			if(channel < 0)return 0;
//			int pos = 0;
//			while(pos < pHnd->memnopress.length)
//			{
//				sceAudioOutputPannedBlocking(channel,
//					PSP_AUDIO_VOLUME_MAX * (pHnd->pan > 0 ? 1.0f - pHnd->pan / 10000.0f : 1.0f) * pHnd->volume / 255.0f,
//					PSP_AUDIO_VOLUME_MAX * (pHnd->pan < 0 ? 1.0f + pHnd->pan / 10000.0f : 1.0f) * pHnd->volume / 255.0f,
//					pHnd->memnopress.pcmBuf + pos);
//				pos += pHnd->avContext.outSampleNum;
//			}
//			sceAudioChRelease(channel);
//			return 0;
//		}
//		while(pHnd->cmd != DXP_SOUNDCMD_NONE)sceKernelDelayThread(100);
//		pHnd->memnopress.cmdplaytype = playtype;
//		pHnd->cmd = DXP_SOUNDCMD_PLAY;
//		break;
//	case DX_SOUNDDATATYPE_FILE:
//		pHnd->file.loop = playtype == DX_PLAYTYPE_LOOP ? 1 : 0;
//		if(rewindflag)pHnd->file.gotoPos = 0;
//		while(pHnd->cmd != DXP_SOUNDCMD_NONE)sceKernelDelayThread(100);
//		pHnd->cmd = DXP_SOUNDCMD_PLAY;
//		if(playtype == DX_PLAYTYPE_NORMAL)
//		{
//			while(pHnd->cmd != DXP_SOUNDCMD_NONE)sceKernelDelayThread(100);
//			while(pHnd->playing)sceKernelDelayThread(100);
//		}
//		break;
//	default:
//		return -1;
//	}
//	return 0;
}

int dxpSound2Init()
{
	DXPSOUND2HANDLE *pHnd;
	sceUtilityLoadAvModule(PSP_AV_MODULE_AVCODEC);
	sceKernelStartThread(sceKernelCreateThread("dxpSound2Thread",dxpSoundThreadFunc,0x11,0x8000,PSP_THREAD_ATTR_USER,NULL),0,NULL);
	for_each_c(pHnd,dxpSound2Control.HandleArray)
	{
		char name[64];
		snprintf(name,63,"dxpSound2HandleMutex%p",pHnd);
		pHnd->used = 0;
		pHnd->CurrentPos = 0;
		pHnd->Length = 0;
		pHnd->LoopData = NULL;
		pHnd->LoopPos[0] = -1;
		pHnd->LoopPos[1] = -1;
		pHnd->MainData = NULL;
		pHnd->Mutex = tmCreate(name);
		pHnd->NextOnlyPan = -1000000;
		pHnd->NextOnlyVolume = -1;
		pHnd->Pan = 0;
		pHnd->RefCount = 0;
		pHnd->Volume = 255;
	}

	return 0;
}

int dxpSound2Term()
{
	DXPSOUND2HANDLE *pHnd;
	DXPSOUND2MESSAGE msg;
	msg.Type = DS2M_EXIT;
	sceKernelSendMsgPipe(dxpSound2Control.MessagePipeID,&msg,sizeof(msg),0,0,NULL);
	for_each_c(pHnd,dxpSound2Control.HandleArray)
	{
		if(!pHnd->used)continue;
		while(1)
		{
			int rc;
			tmLock(pHnd->Mutex);
			rc = pHnd->RefCount;
			tmUnlock(pHnd->Mutex);
			if(!rc)break;
		}
		tmDelete(pHnd->Mutex);
		pHnd->Mutex = 0;
		

	}
	return 0;
}
