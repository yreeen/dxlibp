#include "../fileio.h"

int FileRead_eof(int filehandle)
{
	DXPFILEIOHANDLE *pHnd;
	FHANDLE2PTR(pHnd,filehandle);
	if(pHnd->pos >= pHnd->size - 1)
	{
		pHnd->pos = pHnd->size - 1;
		return 1;
	}
	return 0;
}