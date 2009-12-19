#include "../sound.h"
#include <string.h>
int LoadSoundMem(const char *filename)
{
	if(!dxpSoundData.init)dxpSoundInit();
	if(!filename)return -1;
	if(strlen(filename) > 256 - 1)return -1;
	int fsize = FileRead_size(filename);
	if(fsize <= 0)return -1;
	DXPSOUNDHANDLE *pHnd;
	int handle = dxpSoundReserveHandle();
	if(handle < 0)return -1;
	pHnd = dxpSoundArray + handle;
	pHnd->threadId = sceKernelCreateThread("dxpSoundThread",dxpSoundThread,0x11,0x4000,THREAD_ATTR_USER,0);
	if(pHnd->threadId < 0)
	{
		dxpSoundReleaseHandle(handle);
		return -1;
	}
	strncpy(pHnd->filename,filename,256);
	pHnd->soundDataType = dxpSoundData.createSoundDataType;
	sceKernelStartThread(pHnd->threadId,4,&pHnd);
	return handle;
}
