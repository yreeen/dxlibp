#include "../sound.h"
#include "../fileio.h"
#include <string.h>
#include <pspaudio.h>
#include "../general.h"
#include "../safealloc.h"

DXPSOUNDTHREAD dxpSoundThreads[PSP_AUDIO_CHANNEL_MAX];

int dxpSoundThreadFunc(SceSize size,void* argp)
{
	int channel = -1;
	int pos = 0;
	u32 *pcmBuf[2] = {NULL,NULL};
	u32 pcmBufSize[2] = {0,0};
	u8 pcm = 1;
	DXPSOUNDTHREAD *pth = dxpSoundThreads + *(int*)argp;
	if(!pth)sceKernelExitDeleteThread(0);
	while(1)
	{
		if(!pth->pHnd || !pth->used)
		{
			if(channel >= 0)
			{
				sceAudioChRelease(channel);
				channel = -1;
			}
			dxpSafeFree(pcmBuf[0]);
			dxpSafeFree(pcmBuf[1]);
			pcmBufSize[0] = 0;
			pcmBufSize[1] = 0;
			pos = 0;
			pth->running = 0;
			pth->used = 0;
			sceKernelSleepThread();
		}
		if(!pth->pHnd || !pth->used)continue;
		switch(pth->pHnd->cmd)
		{
		case DXP_SOUNDCMD_NONE:
			break;
		case DXP_SOUNDCMD_STOP:
			pth->pHnd = NULL;
			continue;
		case DXP_SOUNDCMD_EXIT:
			if(channel >= 0)
			{
				sceAudioChRelease(channel);
				channel = -1;
			}
			pth->running = 0;
			pth->used = 0;
			pth->threadId = -1;
			dxpSafeFree(pcmBuf[0]);
			dxpSafeFree(pcmBuf[1]);
			pcmBufSize[0] = 0;
			pcmBufSize[1] = 0;
			sceKernelExitDeleteThread(0);
		}
		pth->running = 1;
		if(channel < 0)
			channel = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL,pth->pHnd->avContext.outSampleNum,PSP_AUDIO_FORMAT_STEREO);
		switch(pth->pHnd->soundDataType)
		{
		case DX_SOUNDDATATYPE_MEMNOPRESS:
			if(pos >= pth->pHnd->memnopress.length)
			{
				if(!pth->loop)
				{
					pth->pHnd = NULL;
					continue;
				}
				pos = pth->pHnd->loopResumePos;
			}
			if(channel < 0)
			{
				pth->pHnd = NULL;
				continue;
			}
			sceAudioOutputPannedBlocking(channel,
				PSP_AUDIO_VOLUME_MAX * (pth->pHnd->pan > 0 ? 1.0f - pth->pHnd->pan / 10000.0f : 1.0f) * pth->pHnd->volume / 255.0f,
				PSP_AUDIO_VOLUME_MAX * (pth->pHnd->pan < 0 ? 1.0f + pth->pHnd->pan / 10000.0f : 1.0f) * pth->pHnd->volume / 255.0f,
				pth->pHnd->memnopress.pcmBuf + pos);
			pos += pth->pHnd->avContext.outSampleNum;
			break;
		case DX_SOUNDDATATYPE_FILE:
			if(channel < 0)
				continue;
			if(pth->pHnd->file.gotoPos >= 0)
			{
				dxpSoundCodecSeek(pth->pHnd,pth->pHnd->file.gotoPos);
				pth->pHnd->file.gotoPos = -1;
			}
			if(pcmBufSize[pcm] < pth->pHnd->avContext.outSampleNum)
			{
				dxpSafeFree(pcmBuf[pcm]);
				pcmBuf[pcm] = dxpSafeAlloc(pth->pHnd->avContext.outSampleNum * 4);
				if(!pcmBuf[pcm])
				{
					pcmBufSize[pcm] = 0;
					continue;
				}
				pcmBufSize[pcm] = pth->pHnd->avContext.outSampleNum;
			}
			pth->pHnd->avContext.pcmOut = pcmBuf[pcm];
			if(dxpSoundCodecDecode(pth->pHnd) < 0)
			{
				if(!pth->loop)
				{
					dxpSoundCodecSeek(pth->pHnd,0);
					pth->pHnd = NULL;
					continue;
				}
				dxpSoundCodecSeek(pth->pHnd,pth->pHnd->loopResumePos);
			}
			sceAudioOutputPannedBlocking(channel,
				PSP_AUDIO_VOLUME_MAX * (pth->pHnd->pan > 0 ? 1.0f - pth->pHnd->pan / 10000.0f : 1.0f) * pth->pHnd->volume / 255.0f,
				PSP_AUDIO_VOLUME_MAX * (pth->pHnd->pan < 0 ? 1.0f + pth->pHnd->pan / 10000.0f : 1.0f) * pth->pHnd->volume / 255.0f,
				pcmBuf[pcm]);
			pcm ^= 1;
			break;

		}
	}
	return 0;
}

int dxpSoundReserveThread()
{
	int i;
	for(i = 0;i < PSP_AUDIO_CHANNEL_MAX;++i)
	{
		if(dxpSoundThreads[i].used || dxpSoundThreads[i].running)continue;
		dxpSoundThreads[i].used = 1;
		if(dxpSoundThreads[i].threadId < 0)
		{
			dxpSoundThreads[i].running = 0;
			dxpSoundThreads[i].pHnd = NULL;
			dxpSoundThreads[i].threadId = sceKernelCreateThread("dxpsound",dxpSoundThreadFunc,0x11,0x4000,PSP_THREAD_ATTR_USER,0);
			sceKernelStartThread(dxpSoundThreads[i].threadId,4,&i);		
		}
		return i;
	}
	return -1;
}

int dxpSoundReleaseThread(int id)
{
	if(id < 0 || id >= PSP_AUDIO_CHANNEL_MAX)return -1;
	if(!dxpSoundThreads[id].used)return 0;
	dxpSoundThreads[id].used = 0;
	while(dxpSoundThreads[id].running || dxpSoundThreads[id].threadId >= 0)
		sceKernelDelayThread(100);
	return 0;
}
