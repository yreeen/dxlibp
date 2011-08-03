#ifndef __TINY_MUTEX_H__
#define __TINY_MUTEX_H__
#include <pspkernel.h>
static inline int tmCreate(const char* name)
{
	return sceKernelCreateSema(name,0,1,1,0);
}

static inline int tmDelete(int id)
{
	return sceKernelDeleteSema(id);
}

static inline int tmLock(int id)
{
	return sceKernelWaitSema(id,1,0);
}

static inline int tmUnlock(int id)
{
	return sceKernelSignalSema(id,1);
}

#endif
