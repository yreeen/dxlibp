#include <pspkernel.h>
#include <libc/stdio.h>
#include "dxlibp.h"
#include "dxpstatic.h"

#include <pspmscm.h>
/*
*	DXライブラリPortable	ファイルの読み込み関連	Ver1.01
*	製作者	：憂煉
*	最終更新：2008/03/30
*	備考	：現在は標準入出力関数を使っています。Ver2.00以降でDXアーカイブに対応させる予定です。
*/





//static u8 FileReadMode = 0x00000000;
//
//int FileRead_open(const char* FileName)
//{
//	return PTR2HND(fopen(FileName,"rb"));
//}
//int FileRead_close(int FileHandle)
//{
//	if(FileHandle == -1)return -1;
//	fclose(HND2PTR(FileHandle));
//	return 0;
//}
//int FileRead_size(const char *FileName)
//{
//	FILE *fp;
//	char tmp;
//	int tmp2;
//	int Result = 0;
//	fp = fopen(FileName,"rb");
//	if(fp == NULL)return -1;
//	while(0 != (tmp2 = fread(&tmp,1024,1,fp)))Result += tmp2;
//	fclose(fp);
//	return Result;
//}
//int FileRead_eof(int FileHandle)
//{
//	if(FileHandle == -1)return -1;
//	return feof((FILE*)HND2PTR(FileHandle));
//}
//int FileRead_getc(int FileHandle)
//{
//	char buf;
//	if(FileHandle == -1)return -1;
//	if(fread(&buf,1,1,HND2PTR(FileHandle)) != 1)return -1;
//	if(buf == 0x0d)
//	{
//		if(FileReadMode & DXP_FILEREAD_DOS)
//		{
//			if(fread(&buf,1,1,HND2PTR(FileHandle)) != 1)return -1;
//			if(buf == 0x0a)return '\n';
//			fseek(HND2PTR(FileHandle),-1,SEEK_CUR);
//			return 0x0d;
//		}
//	}
//	return buf;
//}
//int FileRead_gets(char *Buffer,int Num,int FileHandle)
//{
//	if(FileHandle == -1)return -1;
//	int i;
//	for(i = 0;i < Num;++i)
//	{
//		Buffer[i] = FileRead_getc(FileHandle);
//		if(Buffer[i] == '\n')
//		{
//			Buffer[i] = '\n';
//			return i;
//		}
//	}
//	return i + 1;
//}
//int FileRead_tell(int FileHandle)
//{
//	if(FileHandle == -1)return -1;
//	return ftell(HND2PTR(FileHandle));
//}
//int FileRead_seek(int FileHandle,int Offset,int Origin)
//{
//	if(FileHandle == -1)return -1;
//	return fseek(HND2PTR(FileHandle),Offset,Origin);
//}
//int FileRead_stdread(void * Buffer,int Size,int Num,int FileHandle)
//{
//	if(FileHandle == -1)return -1;
//	return fread(Buffer,Size,Num,HND2PTR(FileHandle));
//}
//int FileRead_read(void * Buffer,int Size,int FileHandle)
//{
//	if(FileHandle == -1)return -1;
//	return fread(Buffer,Size,1,HND2PTR(FileHandle));
//}
//int FileRead_scanf(int FileHandle,const char* Format,...)
//{
//	int res;
//	if(FileHandle == -1)return -1;
//	va_list arglist;
//	va_start(arglist,Format);
//	res = vfscanf(HND2PTR(FileHandle),Format,arglist);
//	va_end(arglist);
//	return res;
//}
//int FileRead_idle_chk( int FileHandle )
//{
//	return 1;
//}
static u8 FileReadMode = 0x00000000;

int FileRead_open(const char* FileName)
{
	return PTR2HND(sceIoOpen(FileName,PSP_O_RDWR,0777));
//	return PTR2HND(fopen(FileName,"rb"));
}
int FileRead_close(int FileHandle)
{
	if(FileHandle == -1)return -1;
	sceIoClose(HND2PTR(FileHandle));
//	fclose(HND2PTR(FileHandle));
	return 0;
}
int FileRead_size(const char *FileName)
{
	SceIoStat stat;
	if(sceIoGetstat(FileName,&stat) < 0) return -1;
	if(stat.st_size > 0x00000000ffffffff)return 0xffffffff;
	return stat.st_size;
}
int FileRead_eof(int FileHandle)
{
	char tmp;
	if(FileHandle == -1)return -1;
	if(sceIoRead(HND2PTR(FileHandle),&tmp,1) != 1)return 1;
	return 0;
}

int FileRead_getc(int FileHandle)
{
	char buf;
	if(FileHandle == -1)return -1;
	if(sceIoRead(HND2PTR(FileHandle),&buf,1) != 1)return -1;
	if(buf == 0x0d)
	{
		if(FileReadMode & DXP_FILEREAD_DOS)
		{
			if(sceIoRead(HND2PTR(FileHandle),&buf,1) != 1)return -1;
			if(buf == 0x0a)return '\n';
			sceIoLseek32(HND2PTR(FileHandle),-1,SEEK_CUR);
			return 0x0d;
		}
	}
	return buf;
}
int FileRead_gets(char *Buffer,int Num,int FileHandle)
{
	if(FileHandle == -1)return -1;
	int i;
	for(i = 0;i < Num;++i)
	{
		Buffer[i] = FileRead_getc(FileHandle);
		if(Buffer[i] == '\n')
		{
			Buffer[i] = '\n';
			return i;
		}
	}
	return i + 1;
}
int FileRead_tell(int FileHandle)
{
	if(FileHandle == -1)return -1;
	return sceIoLseek32(HND2PTR(FileHandle),0,SEEK_CUR);
}
int FileRead_seek(int FileHandle,int Offset,int Origin)
{
	if(FileHandle == -1)return -1;
	return sceIoLseek32(HND2PTR(FileHandle),Offset,Origin);
}
int FileRead_stdread(void * Buffer,int Size,int Num,int FileHandle)
{
	if(FileHandle == -1)return -1;
	if(Size == 0 || Num == 0)return 0;
	return sceIoRead(HND2PTR(FileHandle),Buffer,Num * Size) / Size;
}
int FileRead_read(void * Buffer,int Size,int FileHandle)
{
	if(FileHandle == -1)return -1;
	return sceIoRead(HND2PTR(FileHandle),Buffer,Size);
}
int FileRead_write(void *Buffer,int Size,int Num,int FileHandle)
{
	if(FileHandle == -1)return -1;
	if(Size == 0 || Num == 0)return 0;
	return sceIoWrite(HND2PTR(FileHandle),Buffer,Num * Size) / Size;
}

//int FileRead_scanf(int FileHandle,const char* Format,...)
//{
//	/*解析ルーチン組むしかない。*/
//	int res;
//	if(FileHandle == -1)return -1;
//	va_list arglist;
//	va_start(arglist,Format);
//
//	//int i = 0;
//	//while(Format[i]:
//
//
////	res = vfscanf(HND2PTR(FileHandle),Format,arglist);
//	va_end(arglist);
//	return res;
//}
int FileRead_idle_chk( int FileHandle )
{
	return 1;
}
