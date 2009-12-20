#include "dxlibp.h"
#include <malloc.h>
#include <stdio.h>

//macros ---

//stream
#define STREAM_SEEKTYPE_SET							(PSP_SEEK_SET)
#define STREAM_SEEKTYPE_END							(PSP_SEEK_END)
#define STREAM_SEEKTYPE_CUR							(PSP_SEEK_CUR)

//#define STTELL( st )								((st)->ReadShred.Tell( (st)->DataPoint ))
//#define STSEEK( st, pos, type )						((st)->ReadShred.Seek( (st)->DataPoint, (pos), (type) ))
//#define STREAD( buf, length, num, st )				((st)->ReadShred.Read( (buf), (length), (num), (st)->DataPoint ))
//#define STWRITE( buf, length, num, st )				((st)->ReadShred.Write( (buf), (length), (num), (st)->DataPoint ))
//#define STEOF( st )									((st)->ReadShred.Eof( (st)->DataPoint ))
//#define STCLOSE( st )								((st)->ReadShred.Close( (st)->DataPoint ))


//structures ----

typedef struct DXPFILEIOHANDLE__
{
	unsigned used : 1;
	unsigned onmemory : 1;

	char filename[DXP_BUILDOPTION_FILENAMELENGTH_MAX];

	u32 pos;
	u32 size;

	union
	{
		SceUID fd;
		void *dat;
	};
}DXPFILEIOHANDLE;

typedef struct DXPFILEIODATA__
{
	unsigned init : 1;
	unsigned sleep : 1;
	DXPFILEIOHANDLE handleArray[DXP_BUILDOPTION_FILEHANDLE_MAX];
}DXPFILEIODATA;
//variables ----

extern DXPFILEIODATA dxpFileioData;

//local functions ----



void dxpFileioInit();
int dxpFileioReopen(int handle);