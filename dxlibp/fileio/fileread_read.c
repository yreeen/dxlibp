#include "../fileio.h"

int	FileRead_read(void *buffer,int readsize,int filehandle)
{
	if(filehandle < 0 || filehandle >= DXP_BUILDOPTION_FILEHANDLE_MAX)return -1;
	if(!dxpFileioData.handleArray[filehandle].used)return -1;
	if(dxpFileioData.reopen)dxpFileioReopenAll();

	return sceIoRead(dxpFileioData.handleArray[filehandle].fd,buffer,readsize);
}