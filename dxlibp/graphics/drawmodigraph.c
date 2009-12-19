#include "../graphics.h"
int DrawModiGraph( int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int gh, int trans )
{//Sliceを更に発展させてみた。列ごとに描画版
	DXPGRAPHICSHANDLE* gptr;
	GHANDLE2GPTR(gptr,gh);
	GUSTART;
	trans = trans ? 1 : 0;
	if(dxpGraphicsSetup2DTex(gptr->tex,trans) < 0)return -1;
	int sw = dxpPsm2SliceSize[gptr->tex->psm][0];
	int sh = dxpPsm2SliceSize[gptr->tex->psm][1];
	int swn = (gptr->u1 - gptr->u0 + sw - 1) / sw;
	int shn = (gptr->v1 - gptr->v0 + sh - 1) / sh;
	DXP_FVF_2DTEX *buf = (DXP_FVF_2DTEX*)dxpGuGetMemory(sizeof(DXP_FVF_2DTEX) * (swn * 2 + 2) * shn);
	DXP_FVF_2DTEX *vtxbuf = buf;
	int u[2],v[2];
	float u1_0 = (gptr->u1 - gptr->u0),v1_0 = (gptr->v1 - gptr->v0);
	u1_0 = 1.0f / u1_0;
	v1_0 = 1.0f / v1_0;

	v[0] = gptr->v0;
	while(v[0] < gptr->v1)
	{
		u[0] = gptr->u0;
		v[1] = MIN(v[0] + sh,gptr->v1);
		buf = vtxbuf;

		vtxbuf->u = u[0];
		vtxbuf->v = v[0];
		vtxbuf->x = (x1 + (u[0] - gptr->u0) * (x2 - x1) * u1_0) * ((gptr->v1 - v[0]) * v1_0) +
					(x4 + (u[0] - gptr->u0) * (x3 - x4) * u1_0) * ((v[0] - gptr->v0) * v1_0);
		vtxbuf->y = (y1 + (v[0] - gptr->v0) * (y4 - y1) * v1_0) * ((gptr->u1 - u[0]) * u1_0) +
					(y2 + (v[0] - gptr->v0) * (y3 - y2) * v1_0) * ((u[0] - gptr->u0) * u1_0);
		vtxbuf->z = dxpGraphicsData.z_2d;
		++vtxbuf;

		vtxbuf->u = u[0];
		vtxbuf->v = v[1];
		vtxbuf->x = (x1 + (u[0] - gptr->u0) * (x2 - x1) * u1_0) * ((gptr->v1 - v[1]) * v1_0) +
					(x4 + (u[0] - gptr->u0) * (x3 - x4) * u1_0) * ((v[1] - gptr->v0) * v1_0);
		vtxbuf->y = (y1 + (v[1] - gptr->v0) * (y4 - y1) * v1_0) * ((gptr->u1 - u[0]) * u1_0) +
					(y2 + (v[1] - gptr->v0) * (y3 - y2) * v1_0) * ((u[0] - gptr->u0) * u1_0);
		vtxbuf->z = dxpGraphicsData.z_2d;
		++vtxbuf;
		while(u[0] < gptr->u1)
		{
			u[1] = MIN(u[0] + sw,gptr->u1);			

			vtxbuf->u = u[1];
			vtxbuf->v = v[0];
			vtxbuf->x = (x1 + (u[1] - gptr->u0) * (x2 - x1) * u1_0) * ((gptr->v1 - v[0]) * v1_0) +
						(x4 + (u[1] - gptr->u0) * (x3 - x4) * u1_0) * ((v[0] - gptr->v0) * v1_0);
			vtxbuf->y = (y1 + (v[0] - gptr->v0) * (y4 - y1) * v1_0) * ((gptr->u1 - u[1]) * u1_0) +
						(y2 + (v[0] - gptr->v0) * (y3 - y2) * v1_0) * ((u[1] - gptr->u0) * u1_0);
			vtxbuf->z = dxpGraphicsData.z_2d;
			++vtxbuf;

			vtxbuf->u = u[1];
			vtxbuf->v = v[1];
			vtxbuf->x = (x1 + (u[1] - gptr->u0) * (x2 - x1) * u1_0) * ((gptr->v1 - v[1]) * v1_0) +
						(x4 + (u[1] - gptr->u0) * (x3 - x4) * u1_0) * ((v[1] - gptr->v0) * v1_0);
			vtxbuf->y = (y1 + (v[1] - gptr->v0) * (y4 - y1) * v1_0) * ((gptr->u1 - u[1]) * u1_0) +
						(y2 + (v[1] - gptr->v0) * (y3 - y2) * v1_0) * ((u[1] - gptr->u0) * u1_0);
			vtxbuf->z = dxpGraphicsData.z_2d;
			++vtxbuf;

			u[0] += sw;
		}
		sceGuDrawArray(GU_TRIANGLE_STRIP,DXP_VTYPE_2DTEX | GU_TRANSFORM_2D,2 * swn + 2,0,buf);
		v[0] += sh;
	}
	return 0;
}
