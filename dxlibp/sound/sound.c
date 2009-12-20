#include "../sound.h"
#include "../fileio.h"
#include <string.h>
#include <pspaudio.h>
#include "../general.h"
#include "../safealloc.h"

DXPSOUNDHANDLE dxpSoundArray[DXP_BUILDOPTION_SOUNDHANDLE_MAX];
DXPSOUNDDATA dxpSoundData = {0,0};

int dxpSoundInit()
{
	if(dxpSoundData.init)return 0;
	int ret;
	ret = sceUtilityLoadAvModule(PSP_AV_MODULE_AVCODEC);
	if(ret < 0)return -1;
	memset(&dxpSoundArray,0,sizeof(dxpSoundArray));
	dxpSoundData.init = 1;
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
	if(!dxpSoundData.init)dxpSoundInit();
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
	while(pHnd->threadId != -1)sceKernelDelayThread(100);
	pHnd->used = 0;
	return 0;
}


int dxpSoundCodecInit(DXPSOUNDHANDLE *pHnd,int fh)
{
	int status;
	if(!dxpSoundData.init)return -1;
	status = dxpSoundMp3Init(pHnd,fh);
	if(!(status < 0))return 0;
	return -1;
}

int dxpSoundCodecSeek(DXPSOUNDHANDLE *pHnd,int fh,int sample)
{
	if(!dxpSoundData.init)return -1;
	switch(pHnd->format)
	{
	case DXP_SOUNDFMT_MP3:
		return dxpSoundMp3Seek(pHnd,fh,sample);
	default:
		return -1;
	}
}

int dxpSoundCodecDecode(DXPSOUNDHANDLE *pHnd,int fh)
{
	if(!dxpSoundData.init)return -1;
	switch(pHnd->format)
	{
	case DXP_SOUNDFMT_MP3:
		return dxpSoundMp3Decode(pHnd,fh);
	default:
		return -1;
	}
}

int dxpSoundCodecEnd(DXPSOUNDHANDLE *pHnd,int fh)
{
	if(!dxpSoundData.init)return -1;
	switch(pHnd->format)
	{
	case DXP_SOUNDFMT_MP3:
		return dxpSoundMp3End(pHnd,fh);
	default:
		return -1;
	}
}

void dxpSoundThreadExit(DXPSOUNDHANDLE *pHnd)
{
	pHnd->threadId = -1;
	sceKernelExitDeleteThread(0);
}

void dxpSoundThread_memnopress(DXPSOUNDHANDLE *pHnd,int fh)
{
	u32 *pcmBuf;
	if(dxpSoundCodecInit(pHnd,fh) < 0)dxpSoundThreadExit(pHnd);
	pcmBuf = (u32*) dxpSafeAlloc(pHnd->length * 2 * 2);
	if(!pcmBuf)dxpSoundThreadExit(pHnd);
	memset(pcmBuf,0,pHnd->length * 2 * 2);
	pHnd->pcmOut = (u16*)pcmBuf;
	while(pHnd->currentPos < pHnd->length)
	{
		if(dxpSoundCodecDecode(pHnd,fh) < 0)
		{
			dxpSafeFree(pcmBuf);
			dxpSoundCodecEnd(pHnd,fh);
			dxpSoundThreadExit(pHnd);
		}
		pHnd->pcmOut = (u16*)(pcmBuf + pHnd->currentPos);
		sceKernelDelayThread(1);
	}
	dxpSoundMp3End(pHnd,fh);
	pHnd->loadstatus = 1;

	pHnd->currentPos = 0;

	int channel = -1;

	while(1)
	{
		if(dxpGeneralData.exit_called)
			pHnd->cmd = DXP_SOUNDCMD_EXIT;
		switch(pHnd->cmd)
		{
		case DXP_SOUNDCMD_NONE:
			break;
		case DXP_SOUNDCMD_PLAY:
			pHnd->playing = 1;
			break;
		case DXP_SOUNDCMD_STOP:
			pHnd->playing = 0;
			break;
		case DXP_SOUNDCMD_EXIT:
			if(channel >= 0)
			{
				while(sceAudioGetChannelRestLen(channel) > 0)sceKernelDelayThread(100);
				sceAudioChRelease(channel);
			}
			dxpSafeFree(pcmBuf);
			dxpSoundThreadExit(pHnd);
			break;
		}
		pHnd->cmd = DXP_SOUNDCMD_NONE;
		if(pHnd->gotoPos != -1)pHnd->currentPos = pHnd->gotoPos;
		pHnd->gotoPos = -1;

		if(pHnd->playing)
		{
			if(channel < 0)
			{
				channel = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL,pHnd->pcmOutSize / 4,PSP_AUDIO_FORMAT_STEREO);
				if(channel < 0)
				{
					pHnd->playing = 0;
					continue;
				}
			}
			if(pHnd->currentPos > pHnd->length)
			{
				if(!pHnd->loop)
				{
					pHnd->playing = 0;
					pHnd->currentPos = 0;
					continue;
				}
				pHnd->currentPos = (pHnd->loopPos[0] / (pHnd->pcmOutSize / 4)) * (pHnd->pcmOutSize / 4);
			}
			if(pHnd->loop && pHnd->loopPos[1] && pHnd->currentPos > pHnd->loopPos[1])
			{
				pHnd->currentPos = (pHnd->loopPos[0] / (pHnd->pcmOutSize / 4)) * (pHnd->pcmOutSize / 4);
			}
			while(sceAudioGetChannelRestLen(channel) > 0)sceKernelDelayThread(100);
			sceAudioSetChannelDataLen(channel,pHnd->pcmOutSize / 4);
			sceAudioOutputPanned(channel,
				PSP_AUDIO_VOLUME_MAX * (pHnd->pan > 0 ? 1.0f - pHnd->pan / 10000.0f : 1.0f) * pHnd->volume / 255.0f,
				PSP_AUDIO_VOLUME_MAX * (pHnd->pan < 0 ? 1.0f + pHnd->pan / 10000.0f : 1.0f) * pHnd->volume / 255.0f,
				pcmBuf + pHnd->currentPos);
			pHnd->currentPos += pHnd->pcmOutSize / 4;
		}else
		{
			if(channel >= 0)
			{
				while(sceAudioGetChannelRestLen(channel) > 0)sceKernelDelayThread(100);
				sceAudioChRelease(channel);
				channel = -1;
			}
			sceKernelDelayThread(100);
		}
	}
}

void dxpSoundThread_file(DXPSOUNDHANDLE *pHnd,int fh)
{
	u8 pcm = 0;
	u32 *pcmBuf[2];
	int channel = -1;
	if(dxpSoundCodecInit(pHnd,fh) < 0)dxpSoundThreadExit(pHnd);
	pHnd->loadstatus = 1;
	pcmBuf[0] = dxpSafeAlloc(pHnd->pcmOutSize);
	pcmBuf[1] = dxpSafeAlloc(pHnd->pcmOutSize);
	if(!pcmBuf[0] || !pcmBuf[1])
	{
		dxpSafeFree(pcmBuf[0]);
		dxpSafeFree(pcmBuf[1]);
		dxpSoundCodecEnd(pHnd,fh);
		dxpSoundThreadExit(pHnd);
	}
	while(1)
	{
		if(dxpGeneralData.exit_called)
			pHnd->cmd = DXP_SOUNDCMD_EXIT;
		switch(pHnd->cmd)
		{
		case DXP_SOUNDCMD_NONE:
			break;
		case DXP_SOUNDCMD_PLAY:
			pHnd->playing = 1;
			break;
		case DXP_SOUNDCMD_STOP:
			pHnd->playing = 0;
			break;
		case DXP_SOUNDCMD_EXIT:
			if(channel >= 0)
			{
				while(sceAudioGetChannelRestLen(channel) > 0)sceKernelDelayThread(100);
				sceAudioChRelease(channel);
			}
			dxpSafeFree(pcmBuf[0]);
			dxpSafeFree(pcmBuf[1]);
			dxpSoundCodecEnd(pHnd,fh);
			dxpSoundThreadExit(pHnd);
			break;
		}
		pHnd->cmd = DXP_SOUNDCMD_NONE;
		if(pHnd->gotoPos != -1)dxpSoundCodecSeek(pHnd,fh,pHnd->gotoPos);
		pHnd->gotoPos = -1;

		if(pHnd->playing)
		{
			if(channel < 0)
			{
				channel = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL,pHnd->pcmOutSize / 4,PSP_AUDIO_FORMAT_STEREO);
				if(channel < 0)
				{
					pHnd->playing = 0;
					continue;
				}
			}

			if(pHnd->currentPos > pHnd->length)
			{
				if(!pHnd->loop)
				{
					pHnd->playing = 0;
					dxpSoundCodecSeek(pHnd,fh,0);
					continue;
				}
				dxpSoundCodecSeek(pHnd,fh, (pHnd->loopPos[0] / (pHnd->pcmOutSize / 4)) * (pHnd->pcmOutSize / 4));
			}
			if(pHnd->loop && pHnd->loopPos[1] && pHnd->currentPos > pHnd->loopPos[1])
			{
				dxpSoundCodecSeek(pHnd,fh,pHnd->loopPos[0] / (pHnd->pcmOutSize / 4) * (pHnd->pcmOutSize / 4));
			}

			pHnd->pcmOut = (u16*)pcmBuf[pcm];
			if(dxpSoundCodecDecode(pHnd,fh) < 0)
			{
				//pHnd->cmd = DXP_SOUNDCMD_STOP;
				continue;
			}
			while(sceAudioGetChannelRestLen(channel) > 0)sceKernelDelayThread(100);
			sceAudioSetChannelDataLen(channel,pHnd->pcmOutSize / 4);
			sceAudioOutputPanned(channel,
				PSP_AUDIO_VOLUME_MAX * (pHnd->pan > 0 ? 1.0f - pHnd->pan / 10000.0f : 1.0f) * pHnd->volume / 255.0f,
				PSP_AUDIO_VOLUME_MAX * (pHnd->pan < 0 ? 1.0f + pHnd->pan / 10000.0f : 1.0f) * pHnd->volume / 255.0f,
				pcmBuf[pcm]);
			pcm ^= 1;
		}else
		{
			if(channel >= 0)
			{
				while(sceAudioGetChannelRestLen(channel) > 0)sceKernelDelayThread(100);
				sceAudioChRelease(channel);
				channel = -1;
			}
			sceKernelDelayThread(100);
		}
	}
}

int dxpSoundThread(SceSize datLen,void* datPtr)
{
	if(datLen != 4 || !datPtr)sceKernelExitDeleteThread(0);
	DXPSOUNDHANDLE *pHnd;
	pHnd = *(DXPSOUNDHANDLE**)datPtr;
	int fh = FileRead_open(pHnd->filename,0);
	if(fh < 0)dxpSoundThreadExit(pHnd);
	switch(pHnd->soundDataType)
	{
	case DX_SOUNDDATATYPE_MEMNOPRESS:
		dxpSoundThread_memnopress(pHnd,fh);
//	case DX_SOUNDDATATYPE_MEMPRESS: break;
	case DX_SOUNDDATATYPE_FILE:
		dxpSoundThread_file(pHnd,fh);

	default: dxpSoundThreadExit(pHnd);
	}

	dxpSoundThreadExit(pHnd);
	return -1;
}




