#include "../sound.h"

int SetPanSoundMem(int pan,int handle)
{
	if(pan < -10000 || pan > 10000)return -1;
	if(!dxpSoundData.init)return -1;
	if(handle < 0 || handle >= DXP_BUILDOPTION_SOUNDHANDLE_MAX)return -1;
	DXPSOUNDHANDLE *pHnd = dxpSoundArray + handle;
	if(!pHnd->used)return -1;
	if(pHnd->threadId == -1)
	{//何かの都合でスレッドが強制終了した
		dxpSoundReleaseHandle(handle);
		return -1;
	}

	pHnd->pan = pan;
	return 0;
}
