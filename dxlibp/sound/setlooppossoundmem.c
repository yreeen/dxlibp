#include "../sound.h"

int SetLoopPosSoundMem(int looppos_s,int handle)
{
	if(looppos_s < 0)return -1;
	if(!dxpSoundData.init)return -1;
	if(handle < 0 || handle >= DXP_BUILDOPTION_SOUNDHANDLE_MAX)return -1;
	DXPSOUNDHANDLE *pHnd = dxpSoundArray + handle;
	if(!pHnd->used)return -1;
	if(pHnd->threadId == -1)
	{//何かの都合でスレッドが強制終了した
		dxpSoundReleaseHandle(handle);
		return -1;
	}

	int sample;
	sample = pHnd->sampleRate * looppos_s;
	if(sample > pHnd->length)return -1;
	pHnd->loopPos[0] = sample;
	return 0;
}