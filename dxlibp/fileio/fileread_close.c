#include "../fileio.h"

int FileRead_close(int filehandle)
{
	if(filehandle < 0 || filehandle >= DXP_BUILDOPTION_FILEHANDLE_MAX)return -1;
	if(!dxpFileioData.handleArray[filehandle].used)return -1;
	if(dxpFileioData.reopen)dxpFileioReopenAll();
	sceIoClose(dxpFileioData.handleArray[filehandle].fd);
	dxpFileioData.handleArray[filehandle].used = 0;
	return 0;
}