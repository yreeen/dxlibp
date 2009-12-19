#include "../sound.h"

int DeleteSoundMem(int handle)
{
	if(!dxpSoundData.init)return -1;
	if(handle < 0 || handle >= DXP_BUILDOPTION_SOUNDHANDLE_MAX)return -1;
	DXPSOUNDHANDLE *pHnd = dxpSoundArray + handle;
	if(!pHnd->used)return -1;
	if(pHnd->threadId == -1)
	{
		dxpSoundReleaseHandle(handle);
		return 0;
	}
	pHnd->cmd = DXP_SOUNDCMD_EXIT;
	while(pHnd->threadId != -1)sceKernelDelayThread(100);
	dxpSoundReleaseHandle(handle);
	return 0;
}