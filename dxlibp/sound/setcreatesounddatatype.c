#include "../sound.h"

int SetCreateSoundDataType(int type)
{
	switch(type)
	{
	case DX_SOUNDDATATYPE_MEMNOPRESS:
	case DX_SOUNDDATATYPE_FILE:
		dxpSoundData.createSoundDataType = type;
		return 0;
	}
	return -1;
}
