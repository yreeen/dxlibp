#include "../fileio.h"

int	FileRead_seek(int filehandle,int offset,int origin)
{
	DXPFILEIOHANDLE *pHnd;
	if(origin < 0 || origin > 2)return -1;
	FHANDLE2PTR(pHnd,filehandle);
	if(pHnd->onmemory)
	{
		int target = 0;
		switch(origin)
		{
		case SEEK_CUR:
			target = pHnd->pos + offset;
			break;
		case SEEK_SET:
			target = offset;
			break;
		case SEEK_END:
			target = pHnd->size - offset;
		}
		if(target < 0)target = 0;
		if(target > pHnd->size)target = pHnd->size;
		return pHnd->pos = target;
	}
	if(dxpFileioData.sleep)
		if(dxpFileioReopen(filehandle) < 0)return -1;
	return pHnd->pos = sceIoLseek32(pHnd->fd,offset,origin);
}
