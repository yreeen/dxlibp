#include "../sound.h"

int PlaySoundMem(int handle,int playtype,int rewindflag)
{
	if(!dxpSoundData.init)return -1;
	if(handle < 0 || handle >= DXP_BUILDOPTION_SOUNDHANDLE_MAX)return -1;
	DXPSOUNDHANDLE *pHnd = dxpSoundArray + handle;
	if(!pHnd->used)return -1;
	if(pHnd->threadId == -1)
	{//何かの都合でスレッドが強制終了した
		dxpSoundReleaseHandle(handle);
		return -1;
	}
	if(pHnd->playing)return 0;
	if(rewindflag)pHnd->gotoPos = 0;
	switch(playtype)
	{
	case DX_PLAYTYPE_NORMAL:
		pHnd->loop = 0;
		pHnd->cmd = DXP_SOUNDCMD_PLAY;
		while(pHnd->playing)if(pHnd->threadId == -1)break;
		break;
	case DX_PLAYTYPE_BACK:
		pHnd->loop = 0;
		pHnd->cmd = DXP_SOUNDCMD_PLAY;
		break;
	case DX_PLAYTYPE_LOOP:
		pHnd->loop = 1;
		pHnd->cmd = DXP_SOUNDCMD_PLAY;
		break;
	default:
		return -1;
	}
	return 0;
}
