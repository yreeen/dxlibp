#include "../fileio.h"
#include <psppower.h>

//variables ----

DXPFILEIODATA dxpFileioData = 
{
	0,
	0,
};

//local functions ----


static int dxpPowerCallback(int unk0,int flag,void* arg)
{
	if(flag & PSP_POWER_CB_RESUMING)dxpFileioData.reopen = 1;
	return 0;
}

static int dxpPowerCallbackThread(SceSize args, void *argp)
{
    int cbid;
	cbid = sceKernelCreateCallback("dxp power callback", dxpPowerCallback, NULL);
    scePowerRegisterCallback(-1, cbid);
    sceKernelSleepThreadCB();
	return 0;
}

static int dxpPowerSetupCallback(void)
{
    int thid = 0;
	thid = sceKernelCreateThread("update_thread", dxpPowerCallbackThread, 0x11, 0xFA0, 0, 0);
    if (thid >= 0)
	sceKernelStartThread(thid, 0, 0);
    return thid;
}




void dxpFileioInit()
{
	int i;
	if(dxpFileioData.init)return;
	for(i = 0;i < DXP_BUILDOPTION_FILEHANDLE_MAX;++i)
	{
		dxpFileioData.handleArray[i].used = 0;
	}
	dxpPowerSetupCallback();
	dxpFileioData.init = 1;
}

void dxpFileioReopenAll()
{
	int i;
	if(!dxpFileioData.init)return;
	for(i = 0;i < DXP_BUILDOPTION_FILEHANDLE_MAX;++i)
	{
		if(!dxpFileioData.handleArray[i].used)continue;
		if(dxpFileioData.handleArray[i].filename[0] == '\0')continue;
		sceIoClose(dxpFileioData.handleArray[i].fd);
		dxpFileioData.handleArray[i].fd = sceIoOpen(dxpFileioData.handleArray[i].filename,PSP_O_RDONLY,0777);
		sceIoLseek32((SceUID)dxpFileioData.handleArray[i].fd,dxpFileioData.handleArray[i].pos,SEEK_SET);
	}
	dxpFileioData.reopen = 0;
}

//STREAMDATA* dxpFileHandle2StreamDataPtr(int filehandle)
//{
//	if(filehandle < 0 || filehandle >= DXP_BUILDOPTION_FILEHANDLE_MAX)return NULL;
//	return &dxpFileioData.handleArray[filehandle].src;
//}

//int dxpFileClose(void *ptr)
//{
//	int fd = (int)ptr;
//	sceIoClose(fd);
//	return 0;
//}
//
//int dxpFileEof(void *ptr)
//{
//	int fd = (int)ptr;
//	char buf[1];
//	if(sceIoRead(fd,buf,1) != 1)return 1;
//	sceIoLseek32(fd,PSP_SEEK_CUR,-1);
//	return 0;
//} 
//
//unsigned int dxpFileRead(void* buf,size_t size,size_t num,void* ptr)
//{
//	int fd = (int)ptr,res;
//	if(!buf)return 0;
//
//	res = sceIoRead(fd,buf,size * num) / size;
//	pos = dxpFileTell(ptr);
//	return res;
//}
//
//int dxpFileSeek(void* ptr,long offset,int origin)
//{
//	int fd = (int)ptr;
//	return sceIoLseek(fd,offset,origin);
//}
//
//long dxpFileTell(void* ptr)
//{
//	return dxpFileSeek(ptr,0,PSP_SEEK_CUR);
//}
//
//int dxpFileIdleCheck(void* ptr)
//{
//	return 1;
//}
