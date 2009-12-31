#include "../graphics.h"
#include <pspdisplay.h>
#include <string.h>
int ScreenCopy()
{
	GUINITCHECK;
	GUSYNC;
	register void *front,*back;
	front = &dxpGraphicsData.displaybuffer[&dxpGraphicsData.displaybuffer[0] == dxpGraphicsData.displaybuffer_back ? 1 : 0];
	back = dxpGraphicsData.displaybuffer_back;
	memcpy(front,back,dxpGraphicsData.display_psm == GU_PSM_8888 ? 557056 : 278528);
	sceKernelDcacheWritebackAll();
	if(dxpGraphicsData.debugScreenCallback)dxpGraphicsData.debugScreenCallback();
	sceGuSwapBuffers();
	dxpGraphicsWaitVSync();
	dxpGraphicsData.displaybuffer_back = front;
	if(dxpGraphicsData.rendertarget == &dxpGraphicsData.displaybuffer[0] || dxpGraphicsData.rendertarget == &dxpGraphicsData.displaybuffer[1])
		dxpGraphicsData.rendertarget = front;
	return 0;
}
