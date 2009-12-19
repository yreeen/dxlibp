#include "../fileio.h"
#include <string.h>
#include <pspkerror.h>
#include <unistd.h>

int FileRead_open(const char* filename,int async)
{
	char name[DXP_BUILDOPTION_FILENAMELENGTH_MAX];
	int i;
	if(!filename)return -1;
	if(strlen(filename) + 1 >= DXP_BUILDOPTION_FILENAMELENGTH_MAX)return -1;
	strcpy(name,filename);
	if(!dxpFileioData.init)dxpFileioInit();
	for(i = 0;i < DXP_BUILDOPTION_FILEHANDLE_MAX;++i)
		if(!dxpFileioData.handleArray[i].used)break;
	if(i >= DXP_BUILDOPTION_FILEHANDLE_MAX)return -1;

	dxpFileioData.handleArray[i].fd = sceIoOpen(name,PSP_O_RDONLY,0777);
	if(dxpFileioData.handleArray[i].fd < 0)
	{
		if(dxpFileioData.handleArray[i].fd != SCE_KERNEL_ERROR_NOCWD)return -1;
		int len;
		getcwd(name,260);
		len = strlen(name);
		name[len] = '/';
		strncpy(name + len + 1,filename,DXP_BUILDOPTION_FILENAMELENGTH_MAX - len - 1);
		dxpFileioData.handleArray[i].fd = sceIoOpen(name,PSP_O_RDONLY,0777);
		if(dxpFileioData.handleArray[i].fd < 0)return -1;
	}
	dxpFileioData.handleArray[i].used = 1;
	dxpFileioData.handleArray[i].pos = 0;
	strcpy(dxpFileioData.handleArray[i].filename,name);
	return i;
}

