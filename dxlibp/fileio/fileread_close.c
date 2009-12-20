#include "../fileio.h"

int FileRead_close(int filehandle)
{
	DXPFILEIOHANDLE *pHnd;
	if(filehandle < 0 || filehandle >= DXP_BUILDOPTION_FILEHANDLE_MAX)return -1;
	pHnd = &dxpFileioData.handleArray[filehandle];
	if(!pHnd->used)return -1;
	if(!pHnd->onmemory)
	{
		sceIoClose(pHnd->fd);
	}
	pHnd->used = 0;
	return 0;
}