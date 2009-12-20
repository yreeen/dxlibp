#include "../fileio.h"
#include <stdio.h>
int FileRead_getc(int filehandle)
{
	if(filehandle < 0 || filehandle >= DXP_BUILDOPTION_FILEHANDLE_MAX)return -1;
	DXPFILEIOHANDLE *pHnd;
	pHnd = &dxpFileioData.handleArray[filehandle];
	if(!pHnd->used)return 0;
	if(pHnd->onmemory)
	{
		if(pHnd->pos >= pHnd->size)
		{
			pHnd->pos = pHnd->size;
			return -1;
		}
		return ((u8*)pHnd->dat)[pHnd->pos++];
	}
	if(dxpFileioData.sleep)
		if(dxpFileioReopen(filehandle) < 0)return -1;

	u8 res;
	pHnd->pos += sceIoRead(dxpFileioData.handleArray[filehandle].fd,&res,1);
	
	return res;
}