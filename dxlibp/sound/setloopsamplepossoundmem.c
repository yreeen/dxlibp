#include "../sound.h"

int SetLoopSamplePosSoundMem(int looppos,int handle)
{
	if(looppos < 0)return -1;
	if(!dxpSoundData.init)return -1;
	if(handle < 0 || handle >= DXP_BUILDOPTION_SOUNDHANDLE_MAX)return -1;
	DXPSOUNDHANDLE *pHnd = dxpSoundArray + handle;
	if(!pHnd->used)return -1;
	if(pHnd->threadId == -1)
	{//何かの都合でスレッドが強制終了した
		dxpSoundReleaseHandle(handle);
		return -1;
	}

	if(looppos > pHnd->length)return -1;
	pHnd->loopPos[0] = looppos;
	return 0;
}