#include "dxlibp.h"
#include <pspgu.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include "dxpstatic.h"
#include <pspctrl.h>
#include <stdlib.h>

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

void*	TsFileLoad(const char* filename,int* FileSize)
{
	STREAMDATA src;
	STREAMDATA *Src = &src;
	if(SetupSTREAMDATA(filename,Src) == -1)
	{
		return NULL;
	}
	void *wfp;
	unsigned long size = FileRead_size(filename);
	wfp = malloc(size);
	if ( wfp == NULL) {
		STCLOSE(Src);
		return NULL;
	}
	//20090415 このSEEK_SETってどこにもってるの？
	//STSEEK(Src,0,SEEK_SET);
	STSEEK(Src,0,PSP_SEEK_SET);
	STREAD(wfp,size,1,Src);
	STCLOSE(Src);
	if (FileSize != NULL) *FileSize = size;
	return wfp;
}

int	TsFileSave(const char* filename,unsigned int FileSize,const char* writedata)
{
	STREAMDATA src;
	STREAMDATA *Src = &src;
	//暫定実装
	//書き込みで使う場合はファイルが存在しない時に作成できず
    //終了してしまう。
	if(SetupSTREAMDATA(filename,Src) == -1)
	{
		return -1;
	}
	STWRITE(&writedata,FileSize,1,Src);
	STCLOSE(Src);
	return 0;
}