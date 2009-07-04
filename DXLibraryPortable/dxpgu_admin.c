#include <string.h>
#include <malloc.h>
#include "dxlibp.h"
#include "dxpstatic.h"

//DXPTEXTURE texarray[TEXTURE_MAX];
DXPTEXTURE2		*texlist	= NULL;
//DXPGRAPHDATA	*graphlist	= NULL;
#define 	GRAPHLIST_MAX	512
DXPGRAPHDATA	graphlist[GRAPHLIST_MAX];

void	DxpGuAdminInit()
{
	texlist	= NULL;
	memset(graphlist,0x00,sizeof(DXPGRAPHDATA)*GRAPHLIST_MAX);
	return;
}

void	DxpGuAdminEnd()
{
	DXPTEXTURE2 *tptr;
	while(texlist != NULL)
	{
		FREE(texlist->pmemory);
		FREE(texlist->ppalette);
		FreeVRAM(texlist->pvram);
		tptr = texlist;
		texlist = texlist->next;
		FREE(tptr);
	}
	texlist = NULL;
	return;
}

void TextureList_PushFront(DXPTEXTURE2 *ptr)
{
	ptr->next = texlist;
	ptr->prev = NULL;
	if(texlist != NULL)texlist->prev = ptr;
	texlist = ptr;
}
void TextureList_Remove(DXPTEXTURE2 *ptr)
{
	if(ptr->next != NULL)ptr->next->prev = ptr->prev;
	if(ptr->prev != NULL)ptr->prev->next = ptr->next;
	else texlist = ptr->next;
	ptr->next = ptr->prev = NULL;
}

void	GraphHandleFree(int handle)
{
	if(handle <		0)					return;
	if(handle >=	GRAPHLIST_MAX)		return;
	if(graphlist[handle].useflag == 0)	return;
	graphlist[handle].useflag = 0;
	return;
}

DXPGRAPHDATA* GenerateGraphHandle()//ハンドルの番号を生成する。
{
	int i;
	for(i=0;i<GRAPHLIST_MAX;i++)
	{
		if(graphlist[i].useflag == 0)
		{
			graphlist[i].useflag	= 1;
			graphlist[i].handle		= i;
			return &graphlist[i];
		}
	}
	return NULL;
}

DXPGRAPHDATA* GraphHandle2Ptr(int handle)
{
	if(handle <		0)					return NULL;
	if(handle >=	GRAPHLIST_MAX)		return NULL;
	if(graphlist[handle].useflag == 0)	return NULL;
	return &graphlist[handle];
}

DXPTEXTURE2* GraphHandle2TexPtr(int handle)
{
	if(handle == DXP_SCREEN_BACK)return &gusettings.displaybuffer[gusettings.backbuffer];
	DXPGRAPHDATA *ptr = GraphHandle2Ptr(handle);
	if(ptr != NULL) return ptr->tex;
	return NULL;
}
