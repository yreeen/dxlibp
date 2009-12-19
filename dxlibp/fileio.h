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

////function-pointers for stream data control
//typedef struct STREAMDATASHRED__
//{
//	long					(*Tell)( void *StreamDataPoint ) ;
//	int						(*Seek)( void *StreamDataPoint, long SeekPoint, int SeekType ) ;
//	size_t					(*Read)( void *Buffer, size_t BlockSize, size_t DataNum, void *StreamDataPoint ) ;
////	size_t					(*Write)( void *Buffer, size_t BlockSize, size_t DataNum, void *StreamDataPoint ) ;
//	int						(*Eof)( void *StreamDataPoint ) ;
//	int						(*IdleCheck)( void *StreamDataPoint ) ;
//	int						(*Close)( void *StreamDataPoint ) ;
//} STREAMDATASHRED;
//
////stream data control
//typedef struct STREAMDATA__
//{
//	STREAMDATASHRED			ReadShred ;
//	void					*DataPoint ;
//} STREAMDATA ;


typedef struct DXPFILEIOHANDLE__
{
	unsigned used : 1;
	char filename[DXP_BUILDOPTION_FILENAMELENGTH_MAX];
	int pos;
	SceUID fd;
}DXPFILEIOHANDLE;

typedef struct DXPFILEIODATA__
{
	unsigned init : 1;
	unsigned reopen : 1;
	DXPFILEIOHANDLE handleArray[DXP_BUILDOPTION_FILEHANDLE_MAX];
}DXPFILEIODATA;
//variables ----

extern DXPFILEIODATA dxpFileioData;

//local functions ----



void dxpFileioInit();
void dxpFileioReopenAll();