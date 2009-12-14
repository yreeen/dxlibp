#include <pspuser.h>

PSP_MODULE_INFO("template_eboot", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

int main(int argc,void* argv[])
{
	return 0;
}
