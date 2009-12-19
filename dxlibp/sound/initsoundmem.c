#include "../sound.h"

int InitSoundMem()
{
	if(!dxpSoundData.init)dxpSoundInit();
	if(!dxpSoundData.init)return -1;
	int i;
	for(i = 0;i < DXP_BUILDOPTION_SOUNDHANDLE_MAX;++i)
		DeleteSoundMem(i);
	return 0;
}