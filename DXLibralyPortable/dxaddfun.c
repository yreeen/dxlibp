#include "dxlibp.h"
#include <pspgu.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include "dxpstatic.h"
#include <pspctrl.h>

//#include "c_common.h"
#include <pspkernel.h>
#include <stdlib.h>

#define	FILE_READ_MAX_BYTE	30000
	char	Linewrk[6][FILE_READ_MAX_BYTE];

	SceUID fd;

//以下は龍神録さん所のサンプルソース 50章で使用している未実装関数になります。
int	DrawFormatStringToHandle( int x, int y, int Color, int FontHandle, const char *FormatString, ... )
{
	return 0;
}

int	CreateFontToHandle( const char *FontName, int Size, int Thick, int FontType , int CharSet , int EdgeSize , int Italic  , int DataIndex , int ID )
{
	return 0;
}

//PCのようにメモリが十分にないのでこの関数の存在意義はないかも・・・
int	SetCreateSoundDataType( int SoundDataType )
{
	return 0;
}

/*int WaveFileChk( const char *FileData)
{
	
	return 0;
}
*/

//以下の関数はPSP向けに最適化機能を実装するために追加した関数
void*	TsFileLoad(const char* filename,int* FileSize)
{
	void *wfp;
	if (!(fd = sceIoOpen((char *)filename, PSP_O_RDONLY, 0777))) return NULL;
	unsigned long size = sceIoLseek32(fd, 0, PSP_SEEK_END);
	wfp = malloc(size);
	if ( wfp == NULL) {
		sceIoClose(fd);
		return NULL;
	}
	sceIoLseek32(fd, 0, PSP_SEEK_SET);
	sceIoRead(fd,wfp, size);
	sceIoClose(fd);
	if (FileSize != NULL) *FileSize = size;
	return wfp;
}

int	TsFileSave(const char* filename,unsigned int FileSize,const char* writedata)
{
	if (!(fd = sceIoOpen((char *)filename, PSP_O_CREAT | PSP_O_WRONLY, 0777))) return -1;
	sceIoWrite(fd,writedata, FileSize);
	sceIoClose(fd);
	return 0;
}

