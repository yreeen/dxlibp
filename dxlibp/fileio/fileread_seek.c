#include "../fileio.h"

int	FileRead_seek(int filehandle,int offset,int origin)
{
	if(filehandle < 0 || filehandle >= DXP_BUILDOPTION_FILEHANDLE_MAX)return -1;
	if(!dxpFileioData.handleArray[filehandle].used)return -1;
	if(dxpFileioData.reopen)dxpFileioReopenAll();

	return sceIoLseek32(dxpFileioData.handleArray[filehandle].fd,offset,origin);
}