#include "../sound.h"
#include "../fileio.h"
#include <string.h>
#include <pspaudio.h>
#include "../general.h"
#include "../safealloc.h"

DXPSOUNDHANDLE dxpSoundArray[DXP_BUILDOPTION_SOUNDHANDLE_MAX];
DXPSOUNDDATA dxpSoundData = {0,DX_SOUNDDATATYPE_MEMNOPRESS};

int dxpSoundInit()
{
	if(dxpSoundData.init)return 0;
	int ret;
	ret = sceUtilityLoadAvModule(PSP_AV_MODULE_AVCODEC);
	if(ret < 0)return -1;
	memset(&dxpSoundArray,0,sizeof(dxpSoundArray));
	dxpSoundData.init = 1;
	int i;
	return 0;
}

int dxpSoundTerm()
{
	if(!dxpSoundData.init)return 0;
	//全部に対してStopをかける
	int i;
	for(i = 0;i < DXP_BUILDOPTION_SOUNDHANDLE_MAX;++i)
		DeleteSoundMem(i);
	sceUtilityUnloadAvModule(PSP_MODULE_AV_AVCODEC);
	dxpSoundData.init = 0;
	return 0;
}

int dxpSoundReserveHandle()
{
	if(!dxpSoundData.init)return -1;
	int i;
	DXPSOUNDHANDLE *pHnd;
	for(i = 0;i < DXP_BUILDOPTION_SOUNDHANDLE_MAX && dxpSoundArray[i].used;++i);
	if(i >= DXP_BUILDOPTION_SOUNDHANDLE_MAX)return -1;
	pHnd = dxpSoundArray + i;
	memset(pHnd,0,sizeof(DXPSOUNDHANDLE));
	pHnd->volume = 255;
	pHnd->used = 1;
	return i;
}

int dxpSoundReleaseHandle(int handle)
{
	if(!dxpSoundData.init)return -1;
	DXPSOUNDHANDLE *pHnd;
	if(handle < 0 || handle >= DXP_BUILDOPTION_SOUNDHANDLE_MAX)return -1;
	pHnd = dxpSoundArray + handle;
	if(!pHnd->used)return 0;
	pHnd->cmd = DXP_SOUNDCMD_EXIT;
	pHnd->used = 0;
	return 0;
}


int dxpSoundCodecInit(DXPSOUNDHANDLE *pHnd)
{
	int status;
	if(!dxpSoundData.init)return -1;
	status = dxpSoundMp3Init(&pHnd->avContext);
	if(!(status < 0))return 0;
	return -1;
}

int dxpSoundCodecGetSampleLength(DXPSOUNDHANDLE *pHnd)
{
	if(!dxpSoundData.init)return -1;
	switch(pHnd->avContext.format)
	{
	case DXP_SOUNDFMT_MP3:
		return dxpSoundMp3GetSampleLength(&pHnd->avContext);
	default:
		return -1;
	}
}

int dxpSoundCodecSeek(DXPSOUNDHANDLE *pHnd,int sample)
{
	if(!dxpSoundData.init)return -1;
	switch(pHnd->avContext.format)
	{
	case DXP_SOUNDFMT_MP3:
		return dxpSoundMp3Seek(&pHnd->avContext,sample);
	default:
		return -1;
	}
}

int dxpSoundCodecDecode(DXPSOUNDHANDLE *pHnd)
{
	if(!dxpSoundData.init)return -1;
	switch(pHnd->avContext.format)
	{
	case DXP_SOUNDFMT_MP3:
		return dxpSoundMp3Decode(&pHnd->avContext);
	default:
		return -1;
	}
}

int dxpSoundCodecEnd(DXPSOUNDHANDLE *pHnd)
{
	if(!dxpSoundData.init)return -1;
	switch(pHnd->avContext.format)
	{
	case DXP_SOUNDFMT_MP3:
		return dxpSoundMp3End(&pHnd->avContext);
	default:
		return -1;
	}
}

//void dxpSoundThreadExit(DXPSOUNDHANDLE *pHnd)
//{
//	pHnd->threadId = -1;
//	sceKernelExitDeleteThread(0);
//}
//
//void dxpSoundThread_memnopress(DXPSOUNDHANDLE *pHnd,int fh)
//{
//	u32 *pcmBuf;
//	if(dxpSoundCodecInit(pHnd,fh) < 0)dxpSoundThreadExit(pHnd);
//	pcmBuf = (u32*) dxpSafeAlloc(pHnd->length * 2 * 2);
//	if(!pcmBuf)dxpSoundThreadExit(pHnd);
//	memset(pcmBuf,0,pHnd->length * 2 * 2);
//	pHnd->pcmOut = (u16*)pcmBuf;
//	while(pHnd->currentPos < pHnd->length)
//	{
//		if(dxpSoundCodecDecode(pHnd,fh) < 0)
//		{
//			dxpSafeFree(pcmBuf);
//			dxpSoundCodecEnd(pHnd,fh);
//			dxpSoundThreadExit(pHnd);
//		}
//		pHnd->pcmOut = (u16*)(pcmBuf + pHnd->currentPos);
//		sceKernelDelayThread(1000);
//	}
//	dxpSoundMp3End(pHnd,fh);
//	pHnd->loadstatus = 1;
//
//	pHnd->currentPos = 0;
//
//	int channel = -1;
//
//	while(1)
//	{
//		if(dxpGeneralData.exit_called)
//			pHnd->cmd = DXP_SOUNDCMD_EXIT;
//		switch(pHnd->cmd)
//		{
//		case DXP_SOUNDCMD_NONE:
//			break;
//		case DXP_SOUNDCMD_PLAY:
//			pHnd->playing = 1;
//			break;
//		case DXP_SOUNDCMD_STOP:
//			pHnd->playing = 0;
//			break;
//		case DXP_SOUNDCMD_EXIT:
//			if(channel >= 0)
//			{
//				while(sceAudioGetChannelRestLen(channel) > 0)sceKernelDelayThread(1000);
//				sceAudioChRelease(channel);
//			}
//			dxpSafeFree(pcmBuf);
//			dxpSoundThreadExit(pHnd);
//			break;
//		}
//		pHnd->cmd = DXP_SOUNDCMD_NONE;
//		if(pHnd->gotoPos != -1)pHnd->currentPos = pHnd->gotoPos;
//		pHnd->gotoPos = -1;
//
//		if(pHnd->playing)
//		{
//			if(channel < 0)
//			{
//				channel = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL,pHnd->pcmOutSize / 4,PSP_AUDIO_FORMAT_STEREO);
//				if(channel < 0)
//				{
//					pHnd->playing = 0;
//					continue;
//				}
//			}
//			if(pHnd->currentPos > pHnd->length)
//			{
//				if(!pHnd->loop)
//				{
//					pHnd->playing = 0;
//					pHnd->currentPos = 0;
//					continue;
//				}
//				pHnd->currentPos = (pHnd->loopPos[0] / (pHnd->pcmOutSize / 4)) * (pHnd->pcmOutSize / 4);
//			}
//			if(pHnd->loop && pHnd->loopPos[1] && pHnd->currentPos > pHnd->loopPos[1])
//			{
//				pHnd->currentPos = (pHnd->loopPos[0] / (pHnd->pcmOutSize / 4)) * (pHnd->pcmOutSize / 4);
//			}
//			sceAudioSetChannelDataLen(channel,pHnd->pcmOutSize / 4);
//			sceAudioOutputPannedBlocking(channel,
//				PSP_AUDIO_VOLUME_MAX * (pHnd->pan > 0 ? 1.0f - pHnd->pan / 10000.0f : 1.0f) * pHnd->volume / 255.0f,
//				PSP_AUDIO_VOLUME_MAX * (pHnd->pan < 0 ? 1.0f + pHnd->pan / 10000.0f : 1.0f) * pHnd->volume / 255.0f,
//				pcmBuf + pHnd->currentPos);
//			pHnd->currentPos += pHnd->pcmOutSize / 4;
//		}else
//		{
//			if(channel >= 0)
//			{
//				while(sceAudioGetChannelRestLen(channel) > 0)sceKernelDelayThread(3000);
//				sceAudioChRelease(channel);
//				channel = -1;
//			}
//			sceKernelDelayThread(500);
//		}
//	}
//}
//
//void dxpSoundThread_file(DXPSOUNDHANDLE *pHnd,int fh)
//{
//	u8 pcm = 0;
//	u32 *pcmBuf[2];
//	int channel = -1;
//	if(dxpSoundCodecInit(pHnd,fh) < 0)dxpSoundThreadExit(pHnd);
//	pHnd->loadstatus = 1;
//	pcmBuf[0] = dxpSafeAlloc(pHnd->pcmOutSize);
//	pcmBuf[1] = dxpSafeAlloc(pHnd->pcmOutSize);
//	if(!pcmBuf[0] || !pcmBuf[1])
//	{
//		dxpSafeFree(pcmBuf[0]);
//		dxpSafeFree(pcmBuf[1]);
//		dxpSoundCodecEnd(pHnd,fh);
//		dxpSoundThreadExit(pHnd);
//	}
//	while(1)
//	{
//		if(dxpGeneralData.exit_called)
//			pHnd->cmd = DXP_SOUNDCMD_EXIT;
//		switch(pHnd->cmd)
//		{
//		case DXP_SOUNDCMD_NONE:
//			break;
//		case DXP_SOUNDCMD_PLAY:
//			pHnd->playing = 1;
//			break;
//		case DXP_SOUNDCMD_STOP:
//			pHnd->playing = 0;
//			break;
//		case DXP_SOUNDCMD_EXIT:
//			if(channel >= 0)
//			{
//				while(sceAudioGetChannelRestLen(channel) > 0)sceKernelDelayThread(3000);
//				sceAudioChRelease(channel);
//			}
//			dxpSafeFree(pcmBuf[0]);
//			dxpSafeFree(pcmBuf[1]);
//			dxpSoundCodecEnd(pHnd,fh);
//			dxpSoundThreadExit(pHnd);
//			break;
//		}
//		pHnd->cmd = DXP_SOUNDCMD_NONE;
//		if(pHnd->gotoPos != -1)dxpSoundCodecSeek(pHnd,fh,pHnd->gotoPos);
//		pHnd->gotoPos = -1;
//
//		if(pHnd->playing)
//		{
//			if(channel < 0)
//			{
//				channel = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL,pHnd->pcmOutSize / 4,PSP_AUDIO_FORMAT_STEREO);
//				if(channel < 0)
//				{
//					pHnd->playing = 0;
//					continue;
//				}
//			}
//
//			if(pHnd->currentPos > pHnd->length)
//			{
//				if(!pHnd->loop)
//				{
//					pHnd->playing = 0;
//					dxpSoundCodecSeek(pHnd,fh,0);
//					continue;
//				}
//				dxpSoundCodecSeek(pHnd,fh, (pHnd->loopPos[0] / (pHnd->pcmOutSize / 4)) * (pHnd->pcmOutSize / 4));
//			}
//			if(pHnd->loop && pHnd->loopPos[1] && pHnd->currentPos > pHnd->loopPos[1])
//			{
//				dxpSoundCodecSeek(pHnd,fh,pHnd->loopPos[0] / (pHnd->pcmOutSize / 4) * (pHnd->pcmOutSize / 4));
//			}
//
//			pHnd->pcmOut = (u16*)pcmBuf[pcm];
//			if(dxpSoundCodecDecode(pHnd,fh) < 0)
//			{
//				//pHnd->cmd = DXP_SOUNDCMD_STOP;
//				continue;
//			}
//			while(sceAudioGetChannelRestLen(channel) > 0)sceKernelDelayThread(3000);
//			sceAudioSetChannelDataLen(channel,pHnd->pcmOutSize / 4);
//			sceAudioOutputPanned(channel,
//				PSP_AUDIO_VOLUME_MAX * (pHnd->pan > 0 ? 1.0f - pHnd->pan / 10000.0f : 1.0f) * pHnd->volume / 255.0f,
//				PSP_AUDIO_VOLUME_MAX * (pHnd->pan < 0 ? 1.0f + pHnd->pan / 10000.0f : 1.0f) * pHnd->volume / 255.0f,
//				pcmBuf[pcm]);
//			pcm ^= 1;
//		}else
//		{
//			if(channel >= 0)
//			{
//				while(sceAudioGetChannelRestLen(channel) > 0)sceKernelDelayThread(1000);
//				sceAudioChRelease(channel);
//				channel = -1;
//			}
//			sceKernelDelayThread(500);
//		}
//	}
//}
//
//int dxpSoundThread(SceSize datLen,void* datPtr)
//{
//	if(datLen != 4 || !datPtr)sceKernelExitDeleteThread(0);
//	DXPSOUNDHANDLE *pHnd;
//	pHnd = *(DXPSOUNDHANDLE**)datPtr;
//	int fh = FileRead_open(pHnd->filename,0);
//	if(fh < 0)dxpSoundThreadExit(pHnd);
//	switch(pHnd->soundDataType)
//	{
//	case DX_SOUNDDATATYPE_MEMNOPRESS:
//		dxpSoundThread_memnopress(pHnd,fh);
////	case DX_SOUNDDATATYPE_MEMPRESS: break;
//	case DX_SOUNDDATATYPE_FILE:
//		dxpSoundThread_file(pHnd,fh);
//
//	default: dxpSoundThreadExit(pHnd);
//	}
//
//	dxpSoundThreadExit(pHnd);
//	return -1;
//}
//
//
//
//

typedef struct DXPSOUND_THREADPARAM
{
	DXPSOUNDHANDLE *pHnd;
	union
	{
		struct
		{
			int channel;
		}memnopress;
	};
}DXPSOUND_THREADPARAM;

void dxpSoundThreadFunc_memnopress(DXPSOUND_THREADPARAM *thp)
{
	DXPSOUNDHANDLE *pHnd;
	pHnd = thp->pHnd;
	int pos = 0,loop;
	sceKernelWaitSema(pHnd->memnopress.sema,1,0);
	loop = pHnd->loop ? 1 : 0;
	++pHnd->memnopress.refCount;
	sceKernelSignalSema(pHnd->memnopress.sema,1);
	while(1)
	{
		if(pHnd->cmd == DXP_SOUNDCMD_STOP)break;
		if(pHnd->cmd == DXP_SOUNDCMD_EXIT)break;
		if(pos >= pHnd->memnopress.length)
		{
			if(!loop)break;
			pos = pHnd->loopResumePos;
		}
		sceAudioOutputPannedBlocking(thp->memnopress.channel,
			PSP_AUDIO_VOLUME_MAX * (pHnd->pan > 0 ? 1.0f - pHnd->pan / 10000.0f : 1.0f) * pHnd->volume / 255.0f,
			PSP_AUDIO_VOLUME_MAX * (pHnd->pan < 0 ? 1.0f + pHnd->pan / 10000.0f : 1.0f) * pHnd->volume / 255.0f,
			pHnd->memnopress.pcmBuf + pos);
		pos += pHnd->avContext.outSampleNum;
	}
	sceAudioChRelease(thp->memnopress.channel);
	sceKernelWaitSema(pHnd->memnopress.sema,1,0);
	--pHnd->memnopress.refCount;
	sceKernelSignalSema(pHnd->memnopress.sema,1);
}

int dxpSoundThreadFunc(SceSize datLen,void* datPtr)
{
	if(datLen != sizeof(DXPSOUND_THREADPARAM) || !datPtr)sceKernelExitDeleteThread(0);
	DXPSOUND_THREADPARAM *thp;
	thp = (DXPSOUND_THREADPARAM*)datPtr;
	if(!thp)sceKernelExitDeleteThread(0);
	switch(thp->pHnd->soundDataType)
	{
	case DX_SOUNDDATATYPE_MEMNOPRESS:
		dxpSoundThreadFunc_memnopress(thp);
		break;
	}
	sceKernelExitDeleteThread(0);
	return 0;
}


int LoadSoundMem(const char *filename)
{
	if(!dxpSoundData.init)return -1;
	int i,testchannel;
	int fileSize = FileRead_size(filename);
	if(fileSize <= 0)return -1;
	int fileHandle = FileRead_open(filename,0);
	if(fileHandle < 0)return -1;
	int handle = dxpSoundReserveHandle();
	if(handle < 0)
	{
		FileRead_close(fileHandle);
		return -1;
	}
	DXPSOUNDHANDLE *pHnd = dxpSoundArray + handle;
	pHnd->avContext.fileHandle = fileHandle;
	pHnd->avContext.fileSize = fileSize;
	if(dxpSoundCodecInit(pHnd) < 0)
	{
		dxpSoundReleaseHandle(handle);
		FileRead_close(fileHandle);
		return -1;
	}
	pHnd->soundDataType = dxpSoundData.createSoundDataType;
	switch(dxpSoundData.createSoundDataType)
	{
	case DX_SOUNDDATATYPE_MEMNOPRESS:
		pHnd->memnopress.length = dxpSoundCodecGetSampleLength(pHnd);
		pHnd->memnopress.pcmBuf = memalign(64,pHnd->memnopress.length * 4);
		pHnd->memnopress.sema = sceKernelCreateSema("dxp sound",0,1,1,0);
		if(!pHnd->memnopress.pcmBuf || pHnd->memnopress.sema < 0)
		{
			free(pHnd->memnopress.pcmBuf);
			if(pHnd->memnopress.sema >= 0)sceKernelDeleteSema(pHnd->memnopress.sema);
			dxpSoundCodecEnd(pHnd);
			dxpSoundReleaseHandle(handle);
			FileRead_close(fileHandle);
			return -1;
		}
		memset(pHnd->memnopress.pcmBuf,0,pHnd->memnopress.length);
		pHnd->avContext.nextPos = 0;
		for(i = 0;1;++i)
		{
			if(pHnd->avContext.nextPos > pHnd->memnopress.length)break;
			pHnd->avContext.pcmOut = pHnd->memnopress.pcmBuf + pHnd->avContext.nextPos;
			if(dxpSoundCodecDecode(pHnd) < 0)break;
		}
		dxpSoundCodecEnd(pHnd);
		FileRead_close(fileHandle);
		break;
	default:
		dxpSoundCodecEnd(pHnd);
		dxpSoundReleaseHandle(handle);
		FileRead_close(fileHandle);
		return -1;
	}
	return handle;
}

int PlaySoundMem(int handle,int playtype,int rewindflag)
{
	int thid;
	DXPSOUND_THREADPARAM thp;
	SHND2PTR(handle,thp.pHnd);

	switch(thp.pHnd->soundDataType)
	{
	case DX_SOUNDDATATYPE_MEMNOPRESS:
		thp.pHnd->cmd = DXP_SOUNDCMD_NONE;
		thp.memnopress.channel = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL,thp.pHnd->avContext.outSampleNum,PSP_AUDIO_FORMAT_STEREO);
		if(thp.memnopress.channel < 0)return -1;
		thp.pHnd->loop = playtype == DX_PLAYTYPE_LOOP ? 1 : 0;
		if(playtype == DX_PLAYTYPE_NORMAL)
		{
			thp.pHnd->loop = 0;
			dxpSoundThreadFunc_memnopress(&thp);
			return 0;
		}
		thp.pHnd->cmd = DXP_SOUNDCMD_PLAY;
		thid = sceKernelCreateThread("dxp sound memnopress",dxpSoundThreadFunc,0x11,0x4000,THREAD_ATTR_USER,0);
		if(thid < 0)
		{
			sceAudioChRelease(thp.memnopress.channel);
			return -1;
		}
		if(sceKernelStartThread(thid,sizeof(DXPSOUND_THREADPARAM),&thp) < 0)
		{
			sceKernelDeleteThread(thid);
			sceAudioChRelease(thp.memnopress.channel);
			return -1;
		}
		break;
	default:
		return -1;
	}
	return 0;
}

int StopSoundMem(int handle)
{
	DXPSOUNDHANDLE *pHnd;
	SHND2PTR(handle,pHnd);
	pHnd->cmd = DXP_SOUNDCMD_STOP;
	return 0;
}

int DeleteSoundMem(int handle)
{
	DXPSOUNDHANDLE *pHnd;
	SHND2PTR(handle,pHnd);
	switch(pHnd->soundDataType)
	{
	case DX_SOUNDDATATYPE_MEMNOPRESS:
		pHnd->cmd = DXP_SOUNDCMD_EXIT;
		while(pHnd->memnopress.refCount > 0)sceKernelDelayThread(100);
		sceKernelDeleteSema(pHnd->memnopress.sema);
		free(pHnd->memnopress.pcmBuf);
		dxpSoundReleaseHandle(handle);
		return 0;
		break;
	default:
		return -1;
	}
}

int CheckSoundMem(int handle)
{
	DXPSOUNDHANDLE *pHnd;
	SHND2PTR(handle,pHnd);
	switch(pHnd->soundDataType)
	{
	case DX_SOUNDDATATYPE_MEMNOPRESS:
		return pHnd->memnopress.refCount > 0 ? 1 : 0;
	}
	return -1;
}

int InitSoundMem()
{
	if(!dxpSoundData.init)return -1;
	int i;
	for(i = 0;i < DXP_BUILDOPTION_SOUNDHANDLE_MAX;++i)
		DeleteSoundMem(i);
	return 0;
}