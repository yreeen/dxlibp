#include "../fileio.h"

int FileRead_getc(int filehandle)
{
	if(filehandle < 0 || filehandle >= DXP_BUILDOPTION_FILEHANDLE_MAX)return -1;
	if(!dxpFileioData.handleArray[filehandle].used)return -1;
	if(dxpFileioData.reopen)dxpFileioReopenAll();

	u8 res;
	sceIoRead(dxpFileioData.handleArray[filehandle].fd,&res,1);
	return res;
}