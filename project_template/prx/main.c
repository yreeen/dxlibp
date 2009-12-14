#include "main.h"
/* kernel mode*/
PSP_MODULE_INFO("template_prx_kernel",PSP_MODULE_KERNEL, 1, 0);
PSP_MAIN_THREAD_ATTR(0);
/* user mode
PSP_MODULE_INFO("template_prx_user",0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
*/

int function(void)
{
	return 0;
}

int module_start(SceSize args, void *argp)
{
	return 0;
}

int module_stop(void)
{
	return 0;
}
