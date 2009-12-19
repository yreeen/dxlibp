#include "../graphics.h"

int GetGraphSize(int gh,int *xbuf,int *ybuf)
{
	DXPGRAPHICSHANDLE *gptr;
	GUINITCHECK;
	GHANDLE2GPTR(gptr,gh);
	if(xbuf)*xbuf = gptr->u1 - gptr->u0;
	if(ybuf)*xbuf = gptr->v1 - gptr->v0;
	return 0;
}