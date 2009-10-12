#include <pspkernel.h>
#include <psputility.h>
typedef struct
{ 
	int	AvModule;
} mflag;

mflag	MFLAG = {0,};

int		DxpAvModuleInit()
{
	if(MFLAG.AvModule != 0) return MFLAG.AvModule;
	if(sceUtilityLoadAvModule(PSP_AV_MODULE_AVCODEC) == 0)
		MFLAG.AvModule = 1;
	else
		MFLAG.AvModule = -1;
	return MFLAG.AvModule;
}

int		DxpAvModuleFree()
{
	if(MFLAG.AvModule != 1) return -1;
	sceUtilityUnloadAvModule(PSP_AV_MODULE_AVCODEC);
	MFLAG.AvModule = 0;
	return 0;
}
