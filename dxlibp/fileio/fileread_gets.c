#include "../fileio.h"

int FileRead_gets(char *buffer,int buffersize,int filehandle)
{
	int i,c;
	for(i = 0;i < buffersize;++i)
	{
		c = FileRead_getc(filehandle);
		if(c == -1)return i;
		if(c == '\0' || c == '\n')
		{
			FileRead_seek(filehandle,-1,SEEK_CUR);
			return i;
		}
		buffer[i] = c;
	}
	return i;
}