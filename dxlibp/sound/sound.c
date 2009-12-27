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
	int i;
	for(i = 0;i < PSP_AUDIO_CHANNEL_MAX;++i)
	{
		dxpSoundThreads[i].used = 0;
		dxpSoundThreads[i].running = 0;
		dxpSoundThreads[i].pHnd = NULL;
		dxpSoundThreads[i].threadId = sceKernelCreateThread("dxpsound",dxpSoundThreadFunc,0x11,0x4000,PSP_THREAD_ATTR_USER,0);
		sceKernelStartThread(dxpSoundThreads[i].threadId,4,&i);
	}

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
	for(i = 0;i < PSP_AUDIO_CHANNEL_MAX;++i)
		dxpSoundReleaseThread(i);
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
	pHnd->used = 0;
	return 0;
}


int LoadSoundMem(const char *filename)
{
	if(!dxpSoundData.init)return -1;
	int i;
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
		if(!pHnd->memnopress.pcmBuf)
		{
			free(pHnd->memnopress.pcmBuf);
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
	case DX_SOUNDDATATYPE_FILE:
		pHnd->file.gotoPos = -1;
		return 0;
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
	int thnd,i;
	DXPSOUNDHANDLE *pHnd;
	DXPSOUNDTHREAD *pThd;

	SHND2PTR(handle,pHnd);
	pHnd->cmd = DXP_SOUNDCMD_NONE;
	switch(pHnd->soundDataType)
	{
	case DX_SOUNDDATATYPE_MEMNOPRESS:
		thnd = dxpSoundReserveThread();
		if(thnd < 0)return -1;
		pThd = dxpSoundThreads + thnd;
		pThd->pHnd = pHnd;
		pThd->loop = playtype == DX_PLAYTYPE_LOOP ? 1 : 0;
		sceKernelWakeupThread(pThd->threadId);
		if(playtype == DX_PLAYTYPE_NORMAL)
			while(pThd->used)sceKernelDelayThread(100);
		break;
	case DX_SOUNDDATATYPE_FILE:
		pThd = NULL;
		for(i = 0;i < PSP_AUDIO_CHANNEL_MAX;++i)
			if(dxpSoundThreads[i].pHnd == pHnd)
			{
				pThd = dxpSoundThreads + i;
				break;
			}
		if(!pThd)
		{
			thnd = dxpSoundReserveThread();
			if(thnd < 0)return -1;
			pThd = dxpSoundThreads + thnd;
			pThd->pHnd = pHnd;
		}
		pThd->loop = playtype == DX_PLAYTYPE_LOOP ? 1 : 0;
		sceKernelWakeupThread(pThd->threadId);
		if(playtype == DX_PLAYTYPE_NORMAL)
			while(pThd->used)sceKernelDelayThread(100);
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
		pHnd->cmd = DXP_SOUNDCMD_STOP;
		while(1)
		{
			int i,flag = 1;
			for(i = 0;i < PSP_AUDIO_CHANNEL_MAX;++i)
				if(dxpSoundThreads[i].pHnd == pHnd && dxpSoundThreads[i].running)
					flag = 0;
			if(flag)break;
			sceKernelDelayThread(100);
		}
		free(pHnd->memnopress.pcmBuf);
		dxpSoundReleaseHandle(handle);
		return 0;
		break;
	case DX_SOUNDDATATYPE_FILE:
		pHnd->cmd = DXP_SOUNDCMD_STOP;
		while(1)
		{
			int i,flag = 1;
			for(i = 0;i < PSP_AUDIO_CHANNEL_MAX;++i)
				if(dxpSoundThreads[i].pHnd == pHnd && dxpSoundThreads[i].running)
					flag = 0;
			if(flag)break;
			sceKernelDelayThread(100);
		}
		dxpSoundReleaseHandle(handle);

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
		{
			int i,flag = 0;
			for(i = 0;i < PSP_AUDIO_CHANNEL_MAX;++i)
			if(dxpSoundThreads[i].pHnd == pHnd && dxpSoundThreads[i].running)
				flag = 1;
			return flag;
		}
	case DX_SOUNDDATATYPE_FILE:
		{
			int i,flag = 0;
			for(i = 0;i < PSP_AUDIO_CHANNEL_MAX;++i)
			if(dxpSoundThreads[i].pHnd == pHnd && dxpSoundThreads[i].running)
				flag = 1;
			return flag;
		}
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

