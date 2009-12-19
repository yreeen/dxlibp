#include "../fileio.h"

int FileRead_gets(char *buffer,int buffersize,int filehandle)
{
	if(filehandle < 0 || filehandle >= DXP_BUILDOPTION_FILEHANDLE_MAX)return -1;
	if(!dxpFileioData.handleArray[filehandle].used)return -1;
	if(dxpFileioData.reopen)dxpFileioReopenAll();

	int i;
	for(i = 0;i < buffersize;++i)
	{
		if(sceIoRead(dxpFileioData.handleArray[filehandle].fd,buffer + i,1) != 1)break;
		if(buffer[i] == '\0' || buffer[i] == '\n')break;
	}
	return i;
}