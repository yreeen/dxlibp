#include "../fileio.h"

int FileRead_eof(int filehandle)
{
	if(filehandle < 0 || filehandle >= DXP_BUILDOPTION_FILEHANDLE_MAX)return -1;
	if(!dxpFileioData.handleArray[filehandle].used)return -1;
	if(dxpFileioData.reopen)dxpFileioReopenAll();
	int cur = sceIoLseek32(dxpFileioData.handleArray[filehandle].fd,0,SEEK_CUR);
	int next = sceIoLseek32(dxpFileioData.handleArray[filehandle].fd,1,SEEK_CUR);
	if(cur == next)return 1;
	sceIoLseek32(dxpFileioData.handleArray[filehandle].fd,cur,SEEK_SET);
	return 0;
}