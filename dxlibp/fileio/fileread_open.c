#include "../fileio.h"
#include <string.h>
#include <pspkerror.h>
#include <unistd.h>

int FileRead_open(const char* filename,int async)
{
	DXPFILEIOHANDLE *pHnd;
	int i;
	if(!filename)return -1;
	if(strlen(filename) + 1 >= DXP_BUILDOPTION_FILENAMELENGTH_MAX)return -1;
	if(!dxpFileioData.init)dxpFileioInit();
	for(i = 0;i < DXP_BUILDOPTION_FILEHANDLE_MAX;++i)
		if(!dxpFileioData.handleArray[i].used)break;
	if(i >= DXP_BUILDOPTION_FILEHANDLE_MAX)return -1;
	pHnd = &dxpFileioData.handleArray[i];
	strncpy(pHnd->filename,filename,DXP_BUILDOPTION_FILENAMELENGTH_MAX);
	pHnd->used = 1;
	pHnd->onmemory = 0;
	pHnd->pos = 0;
	if(dxpFileioReopen(i) < 0)
	{
		pHnd->used = 0;
		return -1;
	}
	SceIoStat stat;
	sceIoGetstat(pHnd->filename,&stat);
	pHnd->size = stat.st_size;
	return i;
}
