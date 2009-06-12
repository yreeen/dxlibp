// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		アーカイブ制御プログラム部
// 
// 				Ver 2.25b
// 
// -------------------------------------------------------------------------------


// ＤＸライブラリ作成時用定義
#define __DX_MAKE

// インクルード部-----------------------------------------------------------------
//#include "DxLib.h"
#include "dxlibp.h"
#include "dxparchive.h"
#include <stdio.h>
#include <string.h>

//#define DXPDEFARG(ARG) =(ARG)
//#ifndef DX_NON_DXA

// マクロ定義部 ------------------------------------------------------------------

#define DXARCD						DX_ArchiveDirData
#define CHECKMULTIBYTECHAR(CP)		(( (unsigned char)*(CP) >= 0x81 && (unsigned char)*(CP) <= 0x9F ) || ( (unsigned char)*(CP) >= 0xE0 && (unsigned char)*(CP) <= 0xFC ))	// TRUE:２バイト文字  FALSE:１バイト文字
#define CHARUP(C)					((C) >= 'a' && (C) <= 'z' ? (C) - 'a' + 'A' : (C))

#define DXARC_FILEHEAD_VER1_SIZE	(40)			// Ver0x0001 の DXARC_FILEHEAD 構造体のサイズ
#define DXARC_FILEHEAD_VER2_SIZE	(44)			// Ver0x0002 の DXARC_FILEHEAD 構造体のサイズ


// 構造体定義部 ------------------------------------------------------------------

// DXA_DIR_FindFirst 等の処理で使用する構造体
typedef struct tagDXA_DIR_FINDDATA
{
	int							UseArchiveFlag;					// アーカイブファイルを使用しているかフラグ
	int							UseArchiveIndex;				// アーカイブを使用している場合、使用しているアーカイブファイルデータのインデックス
	int							FindHandle;						// ファイル検索用ハンドル
} DXA_DIR_FINDDATA;

// DXA_FindFirst 等の処理で使用する構造体
typedef struct tagDXA_FINDDATA
{
	DXARC						*Container;						// 検索対象のＤＸＡファイル
	char						SearchStr[MAX_PATH];			// 検索文字列
	DXARC_DIRECTORY				*Directory;						// 検索対象のディレクトリ
	DWORD						ObjectCount;					// 次に渡すディレクトリ内オブジェクトのインデックス
} DXA_FINDDATA;

// 内部大域変数宣言部-------------------------------------------------------------

// アーカイブをディレクトリに見立てる為のデータ
DXARC_DIR DX_ArchiveDirData ;

// 関数プロトタイプ宣言-----------------------------------------------------------

static DXARC_FILEHEAD *DXA_GetFileInfo( DXARC *DXA, const char *FilePath ) ;				// ファイルの情報を得る
static int DXA_ChangeCurrentDirectoryFast( DXARC *DXA, DXARC_SEARCHDATA *SearchData ) ;			// アーカイブ内のディレクトリパスを変更する( 0:成功  -1:失敗 )
static int DXA_DIR_GetArchive( const char *FilePath ) ;										// 既に開かれているアーカイブのハンドルを取得する( 戻り値: -1=無かった 0以上:ハンドル )
static int DXA_DIR_CloseArchive( int ArchiveHandle ) ;										// アーカイブファイルを閉じる
static void DXA_DIR_CloseWaitArchive( void ) ;												// 使用されるのを待っているアーカイブファイルを全て閉じる
static int DXA_DIR_ConvertFullPath( const char *Src, char *Dest ) ;							// 全ての英字小文字を大文字にしながら、フルパスに変換する
//static int DXA_DIR_AnalysisFileNameAndDirPath( const char *Src, char *FileName = 0, char *DirPath = 0 ) ;
static int DXA_DIR_AnalysisFileNameAndDirPath( const char *Src, char *FileName DXPDEFARG(0), char *DirPath DXPDEFARG(0) ) ;
// ファイル名も一緒になっていると分かっているパス中からファイル名とディレクトリパスを分割する。フルパスである必要は無い、ファイル名だけでも良い、DirPath の終端に ¥ マークは付かない
static int DXA_DIR_FileNameCmp( const char *Src, const char *CmpStr );						// CmpStr の条件に Src が適合するかどうかを調べる( 0:適合する  -1:適合しない )
static int DXA_DIR_OpenTest( const char *FilePath, int *ArchiveIndex, char *ArchivePath, char *ArchiveFilePath );	// アーカイブファイルをフォルダに見立ててファイルを開く時の情報を得る( -1:アーカイブとしては存在しなかった  0:存在した )
//dxparc_decode.c
int		DXA_Decode( void *Src, void *Dest );
// プログラム部-------------------------------------------------------------------

/** 文字列を検索用のデータに変換( ヌル文字か \ があったら終了 )
 *
 *
 */
int		DXA_ConvSearchData( DXARC_SEARCHDATA *Dest, const char *Src, int *Length )
{
	int i, StringLength ;
	WORD ParityData ;
	ParityData = 0 ;
	for( i = 0 ; Src[i] != '\0' && Src[i] != '\\' ; )
	{
		if( CHECKMULTIBYTECHAR( &Src[i] ) == TRUE )
		{
			// ２バイト文字の場合はそのままコピー
			*((WORD *)&Dest->FileName[i]) = *((WORD *)&Src[i]) ;
			ParityData += (BYTE)Src[i] + (BYTE)Src[i+1] ;
			i += 2 ;
		}
		else
		{
			// 小文字の場合は大文字に変換
			if( Src[i] >= 'a' && Src[i] <= 'z' )	Dest->FileName[i] = (BYTE)Src[i] - 'a' + 'A' ;
			else									Dest->FileName[i] = Src[i] ;
			ParityData += (BYTE)Dest->FileName[i] ;
			i ++ ;
		}
	}
	// 文字列の長さを保存
	if( Length != NULL ) *Length = i ;
	// ４の倍数の位置まで０を代入
	StringLength = ( ( i + 1 ) + 3 ) / 4 * 4 ;
	memset( &Dest->FileName[i], 0, StringLength - i ) ;
	// パリティデータの保存
	Dest->Parity = ParityData ;
	// パックデータ数の保存
	Dest->PackNum = StringLength / 4 ;
	// 正常終了
	return 0 ;
}
//未使用ぽいのでとりあえずコメント化
/** データを反転させる関数
 *
 *
 */
/*void	DXA_NotConv( void *Data , int Size )
{
	int DwordNum ;
	int ByteNum ;
	DwordNum	= Size / 4 ;
	ByteNum		= Size - DwordNum * 4 ;
	//{
		DWORD *dd;
		dd = ( DWORD * )Data ;
		if( DwordNum != 0 )
		{
			do
			{
				*dd = ~*dd;
				dd++;
			}while( --DwordNum ) ;
		}
		if( ByteNum != 0 )
		{
			do
			{
				*((BYTE *)dd) = ~*((BYTE *)dd) ;
				dd = (DWORD *)(((BYTE *)dd) + 1) ;
			}while( --ByteNum ) ;
		}
	//}
}*/

/** データを反転させてファイルから読み込む関数
 *
 *
 */
/*void	DXA_NotConvFileRead( void *Data, int Size, DWORD FilePointer )
{
	// 読み込む
	//WinFileAccessRead( Data, Size, 1, FilePointer ) ;
	// データを反転
	DXA_NotConv( Data, Size ) ;
}*/

/** 鍵文字列を作成
 *
 *
 */
void	DXA_KeyCreate( const char *Source, unsigned char *Key )
{
	int Len ;
	if( Source == NULL )
	{
		memset( Key, 0xaa, DXA_KEYSTR_LENGTH ) ;
	}
	else
	{
		Len = strlen( Source ) ;
		if( Len > DXA_KEYSTR_LENGTH )
		{
			memcpy( Key, Source, DXA_KEYSTR_LENGTH ) ;
		}
		else
		{
			// 鍵文字列が DXA_KEYSTR_LENGTH より短かったらループする
			int i ;
			for( i = 0 ; i + Len <= DXA_KEYSTR_LENGTH ; i += Len )
				memcpy( Key + i, Source, Len ) ;
			if( i < DXA_KEYSTR_LENGTH )
				memcpy( Key + i, Source, DXA_KEYSTR_LENGTH - i ) ;
		}
	}
	Key[0] = ~Key[0] ;
	Key[1] = ( Key[1] >> 4 ) | ( Key[1] << 4 ) ;
	Key[2] = Key[2] ^ 0x8a ;
	Key[3] = ~( ( Key[3] >> 4 ) | ( Key[3] << 4 ) ) ;
	Key[4] = ~Key[4] ;
	Key[5] = Key[5] ^ 0xac ;
	Key[6] = ~Key[6] ;
	Key[7] = ~( ( Key[7] >> 3 ) | ( Key[7] << 5 ) ) ;
	Key[8] = ( Key[8] >> 5 ) | ( Key[8] << 3 ) ;
	Key[9] = Key[9] ^ 0x7f ;
	Key[10] = ( ( Key[10] >> 4 ) | ( Key[10] << 4 ) ) ^ 0xd6 ;
	Key[11] = Key[11] ^ 0xcc ;
}

/** 鍵文字列を使用して Xor 演算( Key は必ず DXA_KEYSTR_LENGTH の長さがなければならない )
 *
 *
 */
void DXA_KeyConv( void *Data, int Size, int Position, unsigned char *Key )
{
	int i, j ;
	j = Position ;
	for( i = 0 ; i < Size ; i ++ )
	{
		((u8 *)Data)[i] ^= Key[j] ;
		j ++ ;
		if( j == DXA_KEYSTR_LENGTH ) j = 0 ;
	}
}

/** ファイルから読み込んだデータを鍵文字列を使用して Xor 演算する関数( Key は必ず DXA_KEYSTR_LENGTH の長さがなければならない )
 *
 *
 */
void	DXA_KeyConvFileRead( void *Data, int Size, DWORD FilePointer, unsigned char *Key, int Position )
{
	int pos ;
	// ファイルの位置を取得しておく
	if( Position == -1 ) pos = Position ;//WinFileAccessTell( FilePointer ) ;
	else                 pos = Position ;
	// 読み込む
	//WinFileAccessRead( Data, Size, 1, FilePointer ) ;
	//while( WinFileAccessIdleCheck( FilePointer ) == FALSE ) Sleep(0);
	// データを鍵文字列を使って Xor 演算
	DXA_KeyConv( Data, Size, pos, Key ) ;
}

/** 条件に適合するオブジェクトを検索する(検索対象は ObjectCount をインデックスとしたところから)(戻り値 -1:エラー 0:成功)
 *
 *
 */
int		DXA_FindProcess( DXA_FINDDATA *FindData, FILEINFO *FileInfo )
{
	DXARC_DIRECTORY *dir;
	DXARC_FILEHEAD *file;
	BYTE *nameTable;
	DXARC *DXA;
	int i, num, addnum;
	char *str, *name;
	DWORD fileHeadSize;
	DXA = FindData->Container;
	dir = FindData->Directory;
	str = FindData->SearchStr;
	num = dir->FileHeadNum;
	nameTable = DXA->Table.NameTable;
	addnum = dir->ParentDirectoryAddress == 0xffffffff ? 1 : 2;
	fileHeadSize = DXA->Head.Version >= 0x0002 ? DXARC_FILEHEAD_VER2_SIZE : DXARC_FILEHEAD_VER1_SIZE ;
	if( FindData->ObjectCount == (DWORD)( num + addnum ) ) return -1;
	file = ( DXARC_FILEHEAD * )( DXA->Table.FileTable + dir->FileHeadAddress + fileHeadSize * ( FindData->ObjectCount - addnum ) ) ;
	for( i = FindData->ObjectCount; i < num; i ++, file = (DXARC_FILEHEAD *)( (BYTE *)file + fileHeadSize ) )
	{
		if( i < addnum )
		{
			     if( i == 0 ){ if( DXA_DIR_FileNameCmp( ".",  str ) == 0 ) break; }
			else if( i == 1 ){ if( DXA_DIR_FileNameCmp( "..", str ) == 0 ) break; }
		}
		else
		{
			name = (char *)( nameTable + file->NameAddress + 4 );
			if( DXA_DIR_FileNameCmp( name, str ) == 0 ) break;
		}
	}
	FindData->ObjectCount = i;
	if( i == num + addnum ) return -1;
	if( FileInfo )
	{
		if( i < addnum )
		{
			switch( i )
			{
			case 0: strcpy( FileInfo->Name, "."  ); break;
			case 1: strcpy( FileInfo->Name, ".." ); break;
			}
			FileInfo->DirFlag = 1;
			FileInfo->Size    = 0;
			memset( &FileInfo->CreationTime,  0, sizeof( FileInfo->CreationTime  ) );
			memset( &FileInfo->LastWriteTime, 0, sizeof( FileInfo->LastWriteTime ) );
		}
		else
		{
			name = (char *)( nameTable + file->NameAddress );
			strcpy( FileInfo->Name, name + ((WORD *)name)[0] * 4 + 4 );
			FileInfo->DirFlag = (file->Attributes & FILE_ATTRIBUTE_DIRECTORY) != 0 ? TRUE : FALSE;
			FileInfo->Size = (LONGLONG)file->DataSize;
			//_FileTimeToLocalDateData( (FILETIME *)&file->Time.Create,    &FileInfo->CreationTime  );
			//_FileTimeToLocalDateData( (FILETIME *)&file->Time.LastWrite, &FileInfo->LastWriteTime );
		}
	}
	return 0;
}

/** アーカイブファイルを扱う為の構造体を初期化する
 *
 *
 */
int		DXA_Initialize( DXARC *DXA )
{
	memset( DXA, 0, sizeof( DXARC ) ) ;
	return 0 ;
}

/** アーカイブファイルを扱う為の構造体の後始末をする
 *
 *
 */
int		DXA_Terminate( DXARC *DXA )
{
	DXA_CloseArchive( DXA ) ;
	memset( DXA, 0, sizeof( DXARC ) ) ;
	return 0 ;
}

/** メモリ上にあるアーカイブファイルイメージを開く( 0:成功  -1:失敗 )
 *
 *
 */
int		DXA_OpenArchiveFromMem( DXARC *DXA, void *ArchiveImage, int ArchiveSize, const char *KeyString )
{
	BYTE *datp ;
	// 既になんらかのアーカイブを開いていた場合はエラー
	if( DXA->FilePointer != DXA_FILE_NO_SET ) return -1;
	// 鍵の作成
	DXA_KeyCreate( KeyString, DXA->Key ) ;
	// 最初にヘッダの部分を反転する
	memcpy( &DXA->Head, ArchiveImage, sizeof( DXARC_HEAD ) ) ;
	DXA_KeyConv( &DXA->Head, sizeof( DXARC_HEAD ), 0, DXA->Key ) ;
	// ＩＤが違う場合はバージョン２以前か調べる
	if( DXA->Head.Head != DXAHEAD )
	{
		// バージョン２以前か調べる
		memset( DXA->Key, 0xff, DXA_KEYSTR_LENGTH ) ;
		memcpy( &DXA->Head, ArchiveImage, sizeof( DXARC_HEAD ) ) ;
		DXA_KeyConv( &DXA->Head, sizeof( DXARC_HEAD ), 0, DXA->Key ) ;
		// バージョン２以前でもない場合はエラー
		if( DXA->Head.Head != DXAHEAD )	goto ERR ;
	}
	// すべてのデータを反転する
	DXA_KeyConv( ArchiveImage, ArchiveSize, 0, DXA->Key ) ;
	// ポインタを保存
	DXA->FilePointer = (DWORD)ArchiveImage ;
	datp = (BYTE *)ArchiveImage ;
	// ヘッダを解析する
	{
		memcpy( &DXA->Head, datp, sizeof( DXARC_HEAD ) ) ;
		datp += sizeof( DXARC_HEAD ) ;
		// ＩＤの検査
		if( DXA->Head.Head != DXAHEAD ) goto ERR ;
		// バージョン検査
		if( DXA->Head.Version > DXAVER ) goto ERR ;
		// 情報テーブルのアドレスをセットする
		DXA->Table.Top				= (BYTE *)DXA->FilePointer + DXA->Head.FileNameTableStartAddress ;
		DXA->Table.NameTable		= DXA->Table.Top ;
		DXA->Table.FileTable		= DXA->Table.NameTable + DXA->Head.FileTableStartAddress ;
		DXA->Table.DirectoryTable	= DXA->Table.NameTable + DXA->Head.DirectoryTableStartAddress ;
	}
	// カレントディレクトリのセット
	DXA->CurrentDirectory = ( DXARC_DIRECTORY * )DXA->Table.DirectoryTable ;
	DXA->MemoryOpenFlag					= TRUE ;			// メモリイメージから開いているフラグを立てる
	DXA->UserMemoryImageFlag			= TRUE ;			// ユーザーのイメージから開いたフラグを立てる
	DXA->MemoryImageSize				= ArchiveSize ;		// サイズも保存しておく
	return 0 ;
ERR :
	// 反転したデータを元に戻す
	DXA_KeyConv( ArchiveImage, ArchiveSize, 0, DXA->Key ) ;
	return -1 ;
}

/** アーカイブファイルを扱う準備が整ったかを得る( TRUE:整っている  FALSE:整っていない )
 *
 *
 */
int		DXA_CheckIdle( DXARC *DXA )
{
	// 非同期オープン中ではなければ特にやることはない
	if( DXA->ASyncOpenFlag == FALSE ) return TRUE ;
	// ファイル読み込みが完了しているか調べる
	//if( WinFileAccessIdleCheck( DXA->ASyncOpenFilePointer ) == FALSE ) return FALSE ;
	// ファイルを閉じる
	//WinFileAccessClose( DXA->ASyncOpenFilePointer ) ;
	DXA->ASyncOpenFilePointer = 0;
	// すべてのデータを反転する
	DXA_KeyConv( (void *)DXA->FilePointer, DXA->MemoryImageSize, 0, DXA->Key ) ;
	// 非同期オープン中フラグを倒す
	DXA->ASyncOpenFlag = FALSE ;
	return TRUE ;
}

/** アーカイブファイルを開き最初にすべてメモリ上に読み込んでから処理する( 0:成功  -1:失敗 )
 *
 *
 */
int		DXA_OpenArchiveFromFileUseMem( DXARC *DXA, const char *ArchivePath, const char *KeyString , int ASync )
{
	// 既になんらかのアーカイブを開いていた場合はエラー
	if( DXA->FilePointer != DXA_FILE_NO_SET) return -1 ;
	// 鍵の作成
	DXA_KeyCreate( KeyString, DXA->Key ) ;
	// ヘッダ部分だけ先に読み込む
	DXA->ASyncOpenFilePointer = 0;
	DXA->FilePointer = 0;
	//DXA->ASyncOpenFilePointer = WinFileAccessOpen( ArchivePath, FALSE, TRUE, TRUE ) ;
	if( DXA->ASyncOpenFilePointer == 0) return -1 ;
	// ファイルのサイズを取得する
	//WinFileAccessSeek( DXA->ASyncOpenFilePointer, 0L, SEEK_END ) ;
	//DXA->MemoryImageSize = WinFileAccessTell( DXA->ASyncOpenFilePointer ) ;
	//WinFileAccessSeek( DXA->ASyncOpenFilePointer, 0L, SEEK_SET ) ;
	// ファイルの内容を全てメモリに読み込む為のメモリ領域の確保
	DXA->FilePointer = (DWORD)DXALLOC( DXA->MemoryImageSize ) ;
	// ヘッダを解析する
	{
		DXA_KeyConvFileRead( &DXA->Head, sizeof( DXARC_HEAD ), DXA->ASyncOpenFilePointer, DXA->Key ,-1) ;
		// ＩＤの検査
		if( DXA->Head.Head != DXAHEAD )
		{
			// バージョン２以前か調べる
			memset( DXA->Key, 0xff, DXA_KEYSTR_LENGTH ) ;
			//WinFileAccessSeek( DXA->ASyncOpenFilePointer, 0L, SEEK_SET ) ;
			DXA_KeyConvFileRead( &DXA->Head, sizeof( DXARC_HEAD ), DXA->ASyncOpenFilePointer, DXA->Key,-1) ;
			// バージョン２以前でもない場合はエラー
			if( DXA->Head.Head != DXAHEAD )	goto ERR ;
		}
		// バージョン検査
		if( DXA->Head.Version > DXAVER ) goto ERR ;
		// 情報テーブルのサイズ分のメモリを確保する
		DXA->Table.Top = ( BYTE * )DXALLOC( DXA->Head.HeadSize ) ;
		if( DXA->Table.Top == NULL ) goto ERR ;
		// 情報テーブルをメモリに読み込む
		//WinFileAccessSeek( DXA->ASyncOpenFilePointer, DXA->Head.FileNameTableStartAddress, SEEK_SET ) ;
		DXA_KeyConvFileRead( DXA->Table.Top, DXA->Head.HeadSize, DXA->ASyncOpenFilePointer, DXA->Key,-1 ) ;
		// 情報テーブルのアドレスをセットする
		DXA->Table.NameTable		= DXA->Table.Top ;
		DXA->Table.FileTable		= DXA->Table.NameTable + DXA->Head.FileTableStartAddress ;
		DXA->Table.DirectoryTable	= DXA->Table.NameTable + DXA->Head.DirectoryTableStartAddress ;
	}
	// カレントディレクトリのセット
	DXA->CurrentDirectory = ( DXARC_DIRECTORY * )DXA->Table.DirectoryTable ;
	// 改めてファイルを丸ごと読み込む
	//WinFileAccessSeek( DXA->ASyncOpenFilePointer, 0L, SEEK_SET ) ;
	//WinFileAccessRead( (void *)DXA->FilePointer, DXA->MemoryImageSize, 1, DXA->ASyncOpenFilePointer );
	// ファイル非同期オープン中だということをセットしておく
	DXA->ASyncOpenFlag = TRUE ;
	// 同期オープンの場合はここで開き終わるのを待つ
	if( ASync == FALSE )
	{
		while( DXA_CheckIdle( DXA ) == FALSE ) Sleep(0);
	}
/*
	// ファイルの内容を全てメモリに読み込む
	{
		fp = WinFileAccessOpen( ArchivePath, FALSE, TRUE, FALSE ) ;
		if( fp == NULL ) return -1 ;
		WinFileAccessSeek( fp, 0L, SEEK_END ) ;
		DXA->MemoryImageSize = WinFileAccessTell( fp ) ;
		WinFileAccessSeek( fp, 0L, SEEK_SET ) ;
		DXA->FilePointer = (DWORD)DXALLOC( ArchiveSize ) ;
		if( DXA->FilePointer == 0 )
		{
			WinFileAccessClose( fp ) ;
			return -1 ;
		}
		WinFileAccessRead( (void *)DXA->FilePointer, ArchiveSize, 1, fp ) ;
		WinFileAccessClose( fp ) ;
	}
	// 最初にヘッダの部分を反転する
	memcpy( &DXA->Head, (void *)DXA->FilePointer, sizeof( DXARC_HEAD ) ) ;
	DXA_KeyConv( &DXA->Head, sizeof( DXARC_HEAD ), 0, DXA->Key ) ;
	// ＩＤが違う場合はバージョン２以前か調べる
	if( DXA->Head.Head != DXAHEAD )
	{
		// バージョン２以前か調べる
		memset( DXA->Key, 0xff, DXA_KEYSTR_LENGTH ) ;
		memcpy( &DXA->Head, (void *)DXA->FilePointer, sizeof( DXARC_HEAD ) ) ;
		DXA_KeyConv( &DXA->Head, sizeof( DXARC_HEAD ), 0, DXA->Key ) ;
		// バージョン２以前でもない場合はエラー
		if( DXA->Head.Head != DXAHEAD )
			goto ERR ;
	}
	// すべてのデータを反転する
	DXA_KeyConv( (void *)DXA->FilePointer, ArchiveSize, 0, DXA->Key ) ;
	// ヘッダを解析する
	{
		memcpy( &DXA->Head, (BYTE *)DXA->FilePointer, sizeof( DXARC_HEAD ) ) ;
		
		// ＩＤの検査
		if( DXA->Head.Head != DXAHEAD ) goto ERR ;
		
		// バージョン検査
		if( DXA->Head.Version > DXAVER ) goto ERR ;
		// 情報テーブルのアドレスをセットする
		DXA->Table.Top				= (BYTE *)DXA->FilePointer + DXA->Head.FileNameTableStartAddress ;
		DXA->Table.NameTable		= DXA->Table.Top ;
		DXA->Table.FileTable		= DXA->Table.NameTable + DXA->Head.FileTableStartAddress ;
		DXA->Table.DirectoryTable	= DXA->Table.NameTable + DXA->Head.DirectoryTableStartAddress ;
	}
	// カレントディレクトリのセット
	DXA->CurrentDirectory = ( DXARC_DIRECTORY * )DXA->Table.DirectoryTable ;
*/
	DXA->MemoryOpenFlag					= TRUE ;			// メモリイメージから開いているフラグを立てる
	DXA->UserMemoryImageFlag			= FALSE ;			// ユーザーのイメージから開いたわけではないのでフラグを倒す
	return 0 ;
ERR :
	if( DXA->ASyncOpenFilePointer )
	{
		//WinFileAccessClose( DXA->ASyncOpenFilePointer );
		DXA->ASyncOpenFilePointer = 0;
	}
	if( DXA->FilePointer )
	{
		DXFREE( (void *)DXA->FilePointer );
		DXA->FilePointer = 0;
	}
	DXA->ASyncOpenFlag = FALSE ;
	return -1 ;
}

/** アーカイブファイルを開く( 0:成功  -1:失敗 )
 *
 *
 */
int		DXA_OpenArchiveFromFile( DXARC *DXA, const char *ArchivePath, const char *KeyString )
{
	// 既になんらかのアーカイブを開いていた場合はエラー
	if( DXA->FilePointer != DXA_FILE_NO_SET) return -1 ;
	// アーカイブファイルを開こうと試みる
	//DXA->FilePointer = WinFileAccessOpen( ArchivePath, FALSE, TRUE, TRUE ) ;
	if( DXA->FilePointer == DXA_FILE_NO_SET ) return -1 ;
	// 鍵文字列の作成
	DXA_KeyCreate( KeyString, DXA->Key ) ;
	// ヘッダを解析する
	{
		DXA_KeyConvFileRead( &DXA->Head, sizeof( DXARC_HEAD ), DXA->FilePointer, DXA->Key,-1) ;
		// ＩＤの検査
		if( DXA->Head.Head != DXAHEAD )
		{
			// バージョン２以前か調べる
			memset( DXA->Key, 0xff, DXA_KEYSTR_LENGTH ) ;
			//WinFileAccessSeek( DXA->FilePointer, 0L, SEEK_SET ) ;
			DXA_KeyConvFileRead( &DXA->Head, sizeof( DXARC_HEAD ), DXA->FilePointer, DXA->Key,-1 ) ;
			// バージョン２以前でもない場合はエラー
			if( DXA->Head.Head != DXAHEAD )	goto ERR ;
		}
		// バージョン検査
		if( DXA->Head.Version > DXAVER ) goto ERR ;
		// 情報テーブルのサイズ分のメモリを確保する
		DXA->Table.Top = ( BYTE * )DXALLOC( DXA->Head.HeadSize ) ;
		if( DXA->Table.Top == NULL ) goto ERR ;
		// 情報テーブルをメモリに読み込む
		//WinFileAccessSeek( DXA->FilePointer, DXA->Head.FileNameTableStartAddress, SEEK_SET ) ;
		DXA_KeyConvFileRead( DXA->Table.Top, DXA->Head.HeadSize, DXA->FilePointer, DXA->Key,-1 ) ;
		// 情報テーブルのアドレスをセットする
		DXA->Table.NameTable		= DXA->Table.Top ;
		DXA->Table.FileTable		= DXA->Table.NameTable + DXA->Head.FileTableStartAddress ;
		DXA->Table.DirectoryTable	= DXA->Table.NameTable + DXA->Head.DirectoryTableStartAddress ;
	}
	// カレントディレクトリのセット
	DXA->CurrentDirectory = ( DXARC_DIRECTORY * )DXA->Table.DirectoryTable ;
	DXA->MemoryOpenFlag					= FALSE ;			// メモリイメージから開いているフラグを倒す
	DXA->UserMemoryImageFlag			= FALSE ;			// ユーザーのイメージから開いたわけではないのでフラグを倒す
	return 0 ;
ERR :
	if( DXA->FilePointer != DXA_FILE_NO_SET )
	{
		//WinFileAccessClose( DXA->FilePointer ) ;
		DXA->FilePointer = DXA_FILE_NO_SET ;
	}
	if( DXA->Table.Top != NULL )
	{
		DXFREE( DXA->Table.Top ) ;
		DXA->Table.Top = NULL ;
	}
	return -1 ;
}

/** アーカイブファイルを閉じる
 *
 *
 */
int		DXA_CloseArchive( DXARC *DXA )
{
	// 既に閉じていたら何もせず終了
	if( DXA->FilePointer == DXA_FILE_NO_SET) return 0 ;
	// 非同期同期オープン中の場合はここで開き終わるのを待つ
	if( DXA->ASyncOpenFlag == TRUE )
	{
		while( DXA_CheckIdle( DXA ) == FALSE ) Sleep(0);
	}
	// メモリから開いているかどうかで処理を分岐
	if( DXA->MemoryOpenFlag == TRUE )
	{
		// アーカイブプログラムがメモリに読み込んだ場合とそうでない場合で処理を分岐
		if( DXA->UserMemoryImageFlag == TRUE )
		{
			// ユーザーから渡されたデータだった場合
			// 反転したデータを元に戻す
			DXA_KeyConv( (void *)DXA->FilePointer, DXA->MemoryImageSize, 0, DXA->Key ) ;
		}
		else
		{
			// アーカイブプログラムがメモリに読み込んだ場合
			// 確保していたメモリを開放する
			DXFREE( (void *)DXA->FilePointer ) ;
		}
	}
	else
	{
		// アーカイブファイルを閉じる
		//WinFileAccessClose( DXA->FilePointer ) ;
		// 情報テーブルを格納していたメモリ領域も解放
		DXFREE( DXA->Table.Top ) ;
	}
	memset( DXA, 0, sizeof( DXARC ) ) ;// 初期化
	return 0 ;
}

/** アーカイブ内のディレクトリパスを変更する( 0:成功  -1:失敗 )
 *
 *
 */
int		DXA_ChangeCurrentDirectoryFast( DXARC *DXA, DXARC_SEARCHDATA *SearchData )
{
	DXARC_FILEHEAD *FileH ;
	int i, j, k, Num ;
	BYTE *NameData, *PathData ;
	WORD PackNum, Parity ;
	DWORD FileHeadSize ;
	// 非同期同期オープン中の場合はここで開き終わるのを待つ
	if( DXA->ASyncOpenFlag == TRUE )
	{
		while( DXA_CheckIdle( DXA ) == FALSE ) Sleep(0);
	}
	PackNum  = SearchData->PackNum ;
	Parity   = SearchData->Parity ;
	PathData = SearchData->FileName ;
	// カレントディレクトリから同名のディレクトリを探す
	FileH = ( DXARC_FILEHEAD * )( DXA->Table.FileTable + DXA->CurrentDirectory->FileHeadAddress ) ;
	Num = (int)DXA->CurrentDirectory->FileHeadNum ;
	FileHeadSize = DXA->Head.Version >= 0x0002 ? DXARC_FILEHEAD_VER2_SIZE : DXARC_FILEHEAD_VER1_SIZE ;
	for( i = 0 ; i < Num ; i ++, FileH = (DXARC_FILEHEAD *)( (BYTE *)FileH + FileHeadSize ) )
	{
		// ディレクトリチェック
		if( ( FileH->Attributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 ) continue ;
		// 文字列数とパリティチェック
		NameData = DXA->Table.NameTable + FileH->NameAddress ;
		if( PackNum != ((WORD *)NameData)[0] || Parity != ((WORD *)NameData)[1] ) continue ;
		// 文字列チェック
		NameData += 4 ;
		for( j = 0, k = 0 ; j < PackNum ; j ++, k += 4 )
			if( *((DWORD *)&PathData[k]) != *((DWORD *)&NameData[k]) ) break ;
		// 適合したディレクトリがあったらここで終了
		if( PackNum == j ) break ;
	}
	// 無かったらエラー
	if( i == Num ) return -1 ;
	// 在ったらカレントディレクトリを変更
	DXA->CurrentDirectory = ( DXARC_DIRECTORY * )( DXA->Table.DirectoryTable + FileH->DataAddress ) ;
	return 0 ;
}

/** アーカイブ内のディレクトリパスを変更する( 0:成功  -1:失敗 )
 *
 *
 */
int		DXA_ChangeCurrentDirectoryBase( DXARC *DXA, const char *DirectoryPath, bool ErrorIsDirectoryReset, DXARC_SEARCHDATA *LastSearchData )
{
	DXARC_DIRECTORY *OldDir ;
	DXARC_SEARCHDATA SearchData ;
	// 非同期同期オープン中の場合はここで開き終わるのを待つ
	if( DXA->ASyncOpenFlag == TRUE )
	{
		while( DXA_CheckIdle( DXA ) == FALSE ) Sleep(0);
	}
	// ここに留まるパスだったら無視
	if( strcmp( DirectoryPath, "." ) == 0 ) return 0 ;
	// 『\』だけの場合はルートディレクトリに戻る
	if( strcmp( DirectoryPath, "\\" ) == 0 )
	{
		DXA->CurrentDirectory = ( DXARC_DIRECTORY * )DXA->Table.DirectoryTable ;
		return 0 ;
	}
	// 下に一つ下がるパスだったら処理を分岐
	if( strcmp( DirectoryPath, ".." ) == 0 )
	{
		// ルートディレクトリに居たらエラー
		if( DXA->CurrentDirectory->ParentDirectoryAddress == 0xffffffff ) return -1 ;
		// 親ディレクトリがあったらそちらに移る
		DXA->CurrentDirectory = ( DXARC_DIRECTORY * )( DXA->Table.DirectoryTable + DXA->CurrentDirectory->ParentDirectoryAddress ) ;
		return 0 ;
	}
	// それ以外の場合は指定の名前のディレクトリを探す
	// 変更以前のディレクトリを保存しておく
	OldDir = DXA->CurrentDirectory ;
	// パス中に『\』があるかどうかで処理を分岐
	if(strchr( DirectoryPath, '\\' ) == NULL )
	{
		// ファイル名を検索専用の形式に変換する
		DXA_ConvSearchData( &SearchData, DirectoryPath, NULL ) ;
		// ディレクトリを変更
		if( DXA_ChangeCurrentDirectoryFast( DXA, &SearchData ) < 0 ) goto ERR ;
	}
	else
	{
		// \ がある場合は繋がったディレクトリを一つづつ変更してゆく
		int Point, StrLength ;
		Point = 0 ;
		// ループ
		for(;;)
		{
			// 文字列を取得する
			DXA_ConvSearchData( &SearchData, &DirectoryPath[Point], &StrLength ) ;
			Point += StrLength ;
			// もし初っ端が \ だった場合はルートディレクトリに落とす
			if( StrLength == 0 && DirectoryPath[Point] == '\\' )
			{
				DXA_ChangeCurrentDirectoryBase( DXA, "\\", false , NULL) ;
			}
			else
			{
				// それ以外の場合は普通にディレクトリ変更
				if( DXA_ChangeCurrentDirectoryFast( DXA, &SearchData ) < 0 )
				{
					// エラーが起きて、更にエラーが起きた時に元のディレクトリに戻せの
					// フラグが立っている場合は元のディレクトリに戻す
					if( ErrorIsDirectoryReset == true ) DXA->CurrentDirectory = OldDir ;
					// エラー終了
					goto ERR ;
				}
			}
			// もし終端文字で終了した場合はループから抜ける
			// 又はあと \ しかない場合もループから抜ける
			if( DirectoryPath[Point] == '\0' || ( DirectoryPath[Point] == '\\' && DirectoryPath[Point+1] == '\0' ) ) break ;
			Point ++ ;
		}
	}
	if( LastSearchData != NULL )
	{
		memcpy( LastSearchData->FileName, SearchData.FileName, SearchData.PackNum * 4 ) ;
		LastSearchData->Parity  = SearchData.Parity ;
		LastSearchData->PackNum = SearchData.PackNum ;
	}
	return 0 ;// 正常終了
ERR:
	if( LastSearchData != NULL )
	{
		memcpy( LastSearchData->FileName, SearchData.FileName, SearchData.PackNum * 4 ) ;
		LastSearchData->Parity  = SearchData.Parity ;
		LastSearchData->PackNum = SearchData.PackNum ;
	}
	return -1 ;// エラー終了
}

/** ファイルの情報を得る
 *
 *
 */
DXARC_FILEHEAD *DXA_GetFileInfo( DXARC *DXA, const char *FilePath )
{
	DXARC_DIRECTORY *OldDir ;
	DXARC_FILEHEAD *FileH ;
	DWORD FileHeadSize ;
	BYTE *NameData ;
	int i, j, k, Num ;
	DXARC_SEARCHDATA SearchData ;
	OldDir = DXA->CurrentDirectory ;		// 元のディレクトリを保存しておく
	if(strchr( FilePath, '\\' ) != NULL )	// ファイルパスに \ が含まれている場合、ディレクトリ変更を行う
	{
		// カレントディレクトリを目的のファイルがあるディレクトリに変更する
		if( DXA_ChangeCurrentDirectoryBase( DXA, FilePath, false, &SearchData ) >= 0 )
		{
			// エラーが起きなかった場合はファイル名もディレクトリだったことになるのでエラー
			goto ERR ;
		}
	}
	else
	{
		// ファイル名を検索用データに変換する
		DXA_ConvSearchData( &SearchData, FilePath, NULL ) ;
	}
	// 同名のファイルを探す
	FileHeadSize = DXA->Head.Version >= 0x0002 ? DXARC_FILEHEAD_VER2_SIZE : DXARC_FILEHEAD_VER1_SIZE ;
	FileH        = ( DXARC_FILEHEAD * )( DXA->Table.FileTable + DXA->CurrentDirectory->FileHeadAddress ) ;
	Num          = ( int )DXA->CurrentDirectory->FileHeadNum ;
	for( i = 0 ; i < Num ; i ++, FileH = (DXARC_FILEHEAD *)( (BYTE *)FileH + FileHeadSize ) )
	{
		// ディレクトリチェック
		if( ( FileH->Attributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 ) continue ;
		// 文字列数とパリティチェック
		NameData = DXA->Table.NameTable + FileH->NameAddress ;
		if( SearchData.PackNum != ((WORD *)NameData)[0] || SearchData.Parity != ((WORD *)NameData)[1] ) continue ;
		// 文字列チェック
		NameData += 4 ;
		for( j = 0, k = 0 ; j < SearchData.PackNum ; j ++, k += 4 )
			if( *((DWORD *)&SearchData.FileName[k]) != *((DWORD *)&NameData[k]) ) break ;
		// 適合したファイルがあったらここで終了
		if( SearchData.PackNum == j ) break ;
	}
	// 無かったらエラー
	if( i == Num ) goto ERR ;
	// ディレクトリを元に戻す
	DXA->CurrentDirectory = OldDir ;
	// 目的のファイルのアドレスを返す
	return FileH ;
ERR :
	// ディレクトリを元に戻す
	DXA->CurrentDirectory = OldDir ;
	return NULL ;// エラー終了
}

/** アーカイブ内のディレクトリパスを変更する( 0:成功  -1:失敗 )
 *
 *
 */
int		DXA_ChangeCurrentDir( DXARC *DXA, const char *DirPath )
{
	return DXA_ChangeCurrentDirectoryBase( DXA, DirPath, true ,NULL) ;
}

/** アーカイブ内のカレントディレクトリパスを取得する
 *
 *
 */
int		DXA_GetCurrentDir( DXARC *DXA, char *DirPathBuffer, int BufferSize )
{
	char DirPath[256] ;
	DXARC_DIRECTORY *Dir[200], *DirTempP ;
	int Depth, i ;
	// 非同期同期オープン中の場合はここで開き終わるのを待つ
	if( DXA->ASyncOpenFlag == TRUE )
	{
		while( DXA_CheckIdle( DXA ) == FALSE ) Sleep(0);
	}
	// ルートディレクトリに着くまで検索する
	Depth = 0 ;
	DirTempP = DXA->CurrentDirectory ;
	while( DirTempP->DirectoryAddress != 0xffffffff && DirTempP->DirectoryAddress != 0 )
	{
		Dir[Depth] = DirTempP ;
		DirTempP = ( DXARC_DIRECTORY * )( DXA->Table.DirectoryTable + DirTempP->ParentDirectoryAddress ) ;
		Depth ++ ;
	}
	// パス名を連結する
	DirPath[0] = '\0' ;
	for( i = Depth - 1 ; i >= 0 ; i -- )
	{
		strcat( DirPath, "\\" ) ;
		strcat( DirPath, (char *)DXA->Table.NameTable + ((DXARC_FILEHEAD *)( DXA->Table.FileTable + Dir[i]->DirectoryAddress ))->NameAddress ) ;
	}
	// バッファの長さが０か、長さが足りないときはディレクトリ名の長さを返す
	if( BufferSize == 0 || BufferSize < (int)strlen( DirPath ) )
	{
		return strlen( DirPath ) + 1 ;
	}
	else
	{
		// ディレクトリ名をバッファに転送する
		strcpy( DirPathBuffer, DirPath ) ;
	}
	return 0 ;
}

/** アーカイブ内のオブジェクトを検索する( -1:エラー -1以外:DXA検索ハンドル )
 *
 *
 */
int		DXA_FindFirst( DXARC *DXA, const char *FilePath, FILEINFO *Buffer )
{
	DXA_FINDDATA *find;
	char Dir[256], Name[256];
	// 非同期同期オープン中の場合はここで開き終わるのを待つ
	if( DXA->ASyncOpenFlag == TRUE )
	{
		while( DXA_CheckIdle( DXA ) == FALSE ) Sleep(0);
	}
	// メモリの確保
	find = (DXA_FINDDATA *)DXALLOC( sizeof( *find ) );
	if( find == NULL ) return -1;
	find->Container = DXA;
	DXA_DIR_AnalysisFileNameAndDirPath( FilePath, Name, Dir );
	// 全て大文字にする
	strupr( Dir );
	strupr( Name );
	// 検索対象のディレクトリを取得
	if( Dir[0] == '\0' )
	{
		find->Directory = DXA->CurrentDirectory;
	}
	else
	{
		DXARC_DIRECTORY *OldDir;
		OldDir = DXA->CurrentDirectory;
		// 指定のディレクトリが無い場合はエラー
		if( DXA_ChangeCurrentDirectoryBase( DXA, Dir, false ,NULL) == -1 )
		{
			DXFREE( find );
			return -1;
		}
		find->Directory = DXA->CurrentDirectory;
		DXA->CurrentDirectory = OldDir;
	}
	find->ObjectCount = 0;
	strcpy( find->SearchStr, Name );
	// 適合する最初のファイルを検索する
	if( DXA_FindProcess( find, Buffer ) == -1 )
	{
		DXFREE( find );
		return -1;
	}
	find->ObjectCount ++ ;
	return (int)find;// ハンドルを返す
}

/** アーカイブ内のオブジェクトを検索する( -1:エラー 0:成功 )
 *
 *
 */
int		DXA_FindNext( int DxaFindHandle, FILEINFO *Buffer )
{
	DXA_FINDDATA *find;
	find = (DXA_FINDDATA *)DxaFindHandle;
	if( DXA_FindProcess( find, Buffer ) == -1 ) return -1;
	find->ObjectCount ++ ;
	return 0;
}

/** アーカイブ内のオブジェクト検索を終了する
 *
 *
 */
int		DXA_FindClose( int DxaFindHandle )
{
	DXA_FINDDATA *find;
	find = (DXA_FINDDATA *)DxaFindHandle;
	DXFREE( find );
	return 0;
}

/** アーカイブファイル中の指定のファイルをメモリに読み込む( -1:エラー 0以上:ファイルサイズ )
 *
 *
 */
int		DXA_LoadFile( DXARC *DXA, const char *FilePath, void *Buffer, unsigned int BufferSize )
{
	DXARC_FILEHEAD *FileH ;
	// 非同期同期オープン中の場合はここで開き終わるのを待つ
	if( DXA->ASyncOpenFlag == TRUE )
	{
		while( DXA_CheckIdle( DXA ) == FALSE ) Sleep(0);
	}
	// 指定のファイルの情報を得る
	FileH = DXA_GetFileInfo( DXA, FilePath ) ;
	if( FileH == NULL ) return -1 ;
	// ファイルサイズが足りているか調べる、足りていないか、バッファ、又はサイズが０だったらサイズを返す
	if( BufferSize < FileH->DataSize || BufferSize == 0 || Buffer == NULL )
	{
		return ( int )FileH->DataSize ;
	}
	// 足りている場合はバッファーに読み込む
	// ファイルが圧縮されているかどうかで処理を分岐
	if( DXA->Head.Version >= 0x0002 && FileH->PressDataSize != 0xffffffff )
	{
		// 圧縮されている場合
		// メモリ上に読み込んでいるかどうかで処理を分岐
		if( DXA->MemoryOpenFlag == TRUE )
		{
			// メモリ上の圧縮データを解凍する
			DXA_Decode( (BYTE *)DXA->FilePointer + DXA->Head.DataStartAddress + FileH->DataAddress, Buffer ) ;
		}
		else
		{
			void *temp ;
			// 圧縮データをメモリに読み込んでから解凍する
			// 圧縮データが収まるメモリ領域の確保
			temp = DXALLOC( FileH->PressDataSize ) ;
			// 圧縮データの読み込み
			//WinFileAccessSeek( DXA->FilePointer, DXA->Head.DataStartAddress + FileH->DataAddress, SEEK_SET ) ;
			DXA_KeyConvFileRead( temp, FileH->PressDataSize, DXA->FilePointer, DXA->Key,-1 ) ;
			// 解凍
			DXA_Decode( temp, Buffer ) ;
			// メモリの解放
			DXFREE( temp ) ;
		}
	}
	else
	{
		if( DXA->MemoryOpenFlag == TRUE )
		{
			// コピー
			memcpy( Buffer, (BYTE *)DXA->FilePointer + DXA->Head.DataStartAddress + FileH->DataAddress, FileH->DataSize ) ;
		}
		else
		{
			// ファイルポインタを移動
			//WinFileAccessSeek( DXA->FilePointer, DXA->Head.DataStartAddress + FileH->DataAddress, SEEK_SET ) ;
			// 読み込み
			DXA_KeyConvFileRead( Buffer, FileH->DataSize, DXA->FilePointer, DXA->Key,-1 ) ;
		}
	}
	return 0 ;
}

/** アーカイブファイルをメモリに読み込んだ場合のファイルイメージが格納されている先頭アドレスを取得する( DXA_OpenArchiveFromFileUseMem 若しくは DXA_OpenArchiveFromMem で開いた場合に有効 )
 *
 *
 */
void	*DXA_GetFileImage( DXARC *DXA )
{
	// 非同期同期オープン中の場合はここで開き終わるのを待つ
	if( DXA->ASyncOpenFlag == TRUE )
	{
		while( DXA_CheckIdle( DXA ) == FALSE ) Sleep(0);
	}
	// メモリイメージから開いていなかったらエラー
	if( DXA->MemoryOpenFlag == FALSE ) return NULL ;
	return (void *)DXA->FilePointer ;// 先頭アドレスを返す
}

/** アーカイブファイル中の指定のファイルのファイル内の位置とファイルの大きさを得る( -1:エラー )
 *
 *
 */
/*int		DXA_GetFileInfo( DXARC *DXA, const char *FilePath, int *Position, int *Size )
{
	DXARC_FILEHEAD *FileH ;
	// 非同期同期オープン中の場合はここで開き終わるのを待つ
	if( DXA->ASyncOpenFlag == TRUE )
	{
		while( DXA_CheckIdle( DXA ) == FALSE ) Sleep(0);
	}
	// 指定のファイルの情報を得る
	FileH = DXA_GetFileInfo( DXA, FilePath ) ;
	if( FileH == NULL ) return -1 ;
	// ファイルのデータがある位置とファイルサイズを保存する
	if( Position != NULL ) *Position = DXA->Head.DataStartAddress + FileH->DataAddress ;
	if( Size     != NULL ) *Size     = FileH->DataSize ;
	return 0 ;
}*/

/** フルパスではないパス文字列をフルパスに変換する
 *
 *
 */
int		DXA_DIR_ConvertFullPath( const char *Src, char *Dest )
{
	int i, j, k ;
	char iden[256], CurrentDir[MAX_PATH] ;
	// カレントディレクトリを得る
	//GetCurrentDirectory( MAX_PATH, CurrentDir ) ;
	strupr( CurrentDir ) ;
	if( Src == NULL )
	{
		strcpy( Dest, CurrentDir ) ;
		goto END ;
	}
	i = 0 ;
	j = 0 ;
	k = 0 ;
	// 最初に『\』又は『/』が２回連続で続いている場合はネットワークを介していると判断
	if( ( Src[0] == '\\' && Src[1] == '\\' ) ||
		( Src[0] == '/'  && Src[1] == '/'  ) )
	{
		Dest[0] = '\\';
		Dest[1] = '\0';
		i += 2;
		j ++ ;
	}
	else
	// 最初が『\』又は『/』の場合はカレントドライブのルートディレクトリまで落ちる
	if( Src[0] == '\\' || Src[0] == '/' )
	{
		Dest[0] = CurrentDir[0] ;
		Dest[1] = CurrentDir[1] ;
		Dest[2] = '\0' ;

		i ++ ;
		j = 2 ;
	}
	else
	// ドライブ名が書かれていたらそのドライブへ
	if( Src[1] == ':' )
	{
		Dest[0] = CHARUP(Src[0]) ;
		Dest[1] = Src[1] ;
		Dest[2] = '\0' ;
		i = 2 ;
		j = 2 ;
		if( Src[i] == '\\' ) i ++ ;
	}
	else
	// それ以外の場合はカレントディレクトリ
	{
		strcpy( Dest, CurrentDir ) ;
		//j = lstrlen( Dest ) ;
		j = strlen( Dest ) ;
		if( Dest[j-1] == '\\' || Dest[j-1] == '/' )
		{
			Dest[j-1] = '\0' ;
			j -- ;
		}
	}
	for(;;)
	{
		switch( Src[i] )
		{
		case '\0' :
			if( k != 0 )
			{
				Dest[j] = '\\' ; j ++ ;
				strcpy( &Dest[j], iden ) ;
				j += k ;
				k = 0 ;
			}
			goto END ;

		case '\\' :
		case '/' :
			// 文字列が無かったらスキップ
			if( k == 0 )
			{
				i ++ ;
				break;
			}
			if( strcmp( iden, "." ) == 0 )
			{
				// なにもしない
			}
			else
			if( strcmp( iden, ".." ) == 0 )
			{
				// 一つ下のディレクトリへ
				j -- ;
				while( Dest[j] != '\\' && Dest[j] != '/' && Dest[j] != ':' ) j -- ;
				if( Dest[j] != ':' ) Dest[j] = '\0' ;
				else j ++ ;
			}
			else
			{
				Dest[j] = '\\' ; j ++ ;
				strcpy( &Dest[j], iden ) ;
				j += k ;
			}
			k = 0 ;
			i ++ ;
			break ;
		default :
			if( CHECKMULTIBYTECHAR( &Src[i] ) == FALSE  )
			{
				iden[k] = CHARUP(Src[i]) ;
				iden[k+1] = '\0' ; 
				k ++ ;
				i ++ ;
			}
			else
			{
				*((WORD *)&iden[k]) = *((WORD *)&Src[i]) ;
				iden[k+2] = '\0' ;
				k += 2 ;
				i += 2 ;
			}
			break ;
		}
	}
END :
	return 0 ;//正常終了
}

/** ファイル名も一緒になっていると分かっているパス中からファイル名とディレクトリパスを分割する
 * フルパスである必要は無い、ファイル名だけでも良い
 * DirPath の終端に ¥ マークは付かない
 */
int		DXA_DIR_AnalysisFileNameAndDirPath( const char *Src, char *FileName, char *DirPath )
{
	int i, Last ;
	// ファイル名を抜き出す
	i = 0 ;
	Last = -1 ;
	while( Src[i] != '\0' )
	{
		if( CHECKMULTIBYTECHAR( &Src[i] ) == FALSE )
		{
			if( Src[i] == '\\' || Src[i] == '/' || Src[i] == '\0' || Src[i] == ':' ) Last = i ;
			i ++ ;
		}
		else
		{
			i += 2 ;
		}
	}
	if( FileName != NULL )
	{
		if( Last != -1 ) strcpy( FileName, &Src[Last+1] ) ;
		else             strcpy( FileName, Src ) ;
	}
	// ディレクトリパスを抜き出す
	if( DirPath != NULL )
	{
		if( Last != -1 )
		{
			memcpy( DirPath, Src, Last ) ;
			DirPath[Last] = '\0' ;
		}
		else
		{
			DirPath[0] = '\0' ;
		}
	}
	return 0 ;
}

/** CmpStr の条件に Src が適合するかどうかを調べる( 0:適合する  -1:適合しない )
 *
 *
 */
int		DXA_DIR_FileNameCmp( const char *Src, const char *CmpStr )
{
	const char *s, *c;
	s = Src;
	c = CmpStr;
	while( *c != '\0' || *s != '\0' )
	{
		if( CHECKMULTIBYTECHAR( c ) == TRUE )
		{
			if( *((WORD *)s) != *((WORD *)c) ) return -1;
			c += 2;
			s += 2;
		}
		else
		{
			switch( *c )
			{
			case '?':
				c ++ ;
				s ++ ;
				break;

			case '*':
				while( *c == '*' ) c ++ ;
				if( *c == '\0' ) return 0;
				while( *s != '\0' && *s != *c ) s ++ ;
				if( *s == '\0' ) return -1;
				c ++ ;
				s ++ ;
				break;
			default:
				if( *c != *s ) return -1;
				c ++ ;
				s ++ ;
				break;
			}
		}
		if( ( *c == '\0' && *s != '\0' ) || ( *c != '\0' && *s == '\0' ) ) return -1;
	}
	return 0;
}

/** アーカイブファイルを開く
 *
 *
 */
int		DXA_DIR_OpenArchive( const char *FilePath, int ArchiveIndex, int OnMemory, int ASync )
{
	int i, index, newindex ;
	DXARC_DIR_ARCHIVE *arc ;
	DXARC temparc ;
	// アーカイブの指定がある場合はそのまま使用する
	if( ArchiveIndex != -1 )
	{
		if( DXARCD.Archive[ArchiveIndex] != NULL && strcmp( FilePath, DXARCD.Archive[ArchiveIndex]->Path ) == 0 )
		{
			DXARCD.Archive[ArchiveIndex]->UseCounter ++ ;
			return ArchiveIndex ;
		}
	}
	// 既に開かれているか調べる
	newindex = -1 ;
	index    = 0 ;
	for( i = 0 ; i < DXARCD.ArchiveNum ; index ++ )
	{
		arc = DXARCD.Archive[index] ;
		if( arc == NULL )
		{
			newindex = index ;
			continue ;
		}
		i ++ ;
		if( strcmp( arc->Path, FilePath ) == 0 )
		{
			// 既に開かれていた場合はそのインデックスを返す
			arc->UseCounter ++ ;
			return index ;
		}
	}
	// なかった場合は新規にデータを追加する
	// ハンドルが既に一杯の場合は使用されていないアーカイブハンドルを解放する
	if( DXARCD.ArchiveNum == DXA_DIR_MAXARCHIVENUM )
	{
		// 未使用のハンドルを解放
		DXA_DIR_CloseWaitArchive() ;
		// それでも一杯である場合はエラー
		if( DXARCD.ArchiveNum == DXA_DIR_MAXARCHIVENUM )
			return -1 ;
	} 
	if( newindex == -1 )
	{
		for( newindex = 0 ; DXARCD.Archive[newindex] != NULL ; newindex ++ ){}
	}
	// アーカイブファイルが存在しているか確認がてら初期化する
	DXA_Initialize( &temparc ) ;
	if( OnMemory == TRUE )
	{
		// メモリに読み込む場合
		if( DXA_OpenArchiveFromFileUseMem( &temparc, FilePath, DXARCD.ValidKeyString == TRUE ? DXARCD.KeyString : NULL, ASync ) < 0 )
			return -1 ;
	}
	else
	{
		// ファイルから読み込む場合
		if( DXA_OpenArchiveFromFile( &temparc, FilePath, DXARCD.ValidKeyString == TRUE ? DXARCD.KeyString : NULL ) < 0 )
			return -1 ;
	}
	// 新しいアーカイブデータ用のメモリを確保する
	arc = DXARCD.Archive[newindex] = (DXARC_DIR_ARCHIVE *)DXALLOC( sizeof( DXARC_DIR_ARCHIVE ) ) ;
	if( DXARCD.Archive[newindex] == NULL )
	{
		DXA_CloseArchive( &temparc ) ;
		DXA_Terminate( &temparc ) ;
		return -1 ;
	}
	// 情報セット
	memcpy( &arc->Archive, &temparc, sizeof( DXARC ) ) ;
	arc->UseCounter = 1 ;
	strcpy( arc->Path, FilePath ) ;
	// 使用中のアーカイブの数を増やす
	DXARCD.ArchiveNum ++ ;
	return newindex ;// インデックスを返す
}

/** 既に開かれているアーカイブのハンドルを取得する( 戻り値: -1=無かった 0以上:ハンドル )
 *
 *
 */
int		DXA_DIR_GetArchive( const char *FilePath )
{
	int i, index ;
	DXARC_DIR_ARCHIVE *arc ;
	index = 0 ;
	for( i = 0 ; i < DXARCD.ArchiveNum ; index ++ )
	{
		arc = DXARCD.Archive[index] ;
		if( arc == NULL ) continue ;
		i ++ ;
		if( strcmp( arc->Path, FilePath ) == 0 )
			return index ;
	}
	return -1 ;
}

/** アーカイブファイルを閉じる
 *
 *
 */
int		DXA_DIR_CloseArchive( int ArchiveHandle )
{
	DXARC_DIR_ARCHIVE *arc ;
	// 使用されていなかったら何もせず終了
	arc = DXARCD.Archive[ArchiveHandle] ;
	if( arc == NULL || arc->UseCounter == 0 ) return -1 ;
	// 参照カウンタを減らす
	arc->UseCounter -- ;
	return 0 ;
}

/** 使用されるのを待っているアーカイブファイルを全て閉じる
 *
 *
 */
void	DXA_DIR_CloseWaitArchive( void )
{
	int i, Num, index ;
	DXARC_DIR_ARCHIVE *arc ;
	
	Num = DXARCD.ArchiveNum ;
	for( i = 0, index = 0 ; i < Num ; arc ++, index ++ )
	{
		if( DXARCD.Archive[index] == NULL ) continue ;
		i ++ ;

		arc = DXARCD.Archive[index] ;

		// 使われていたら解放しない
		if( arc->UseCounter > 0 ) continue ;

		// 後始末
		DXA_CloseArchive( &arc->Archive ) ;
		DXA_Terminate( &arc->Archive ) ;
		DXFREE( arc ) ;
		DXARCD.Archive[index] = NULL ;
		
		// アーカイブの数を減らす
		DXARCD.ArchiveNum -- ;
	}
}

/** アーカイブをディレクトリに見立てる処理の初期化
 *
 *
 */
int		DXA_DIR_Initialize( void )
{
	memset( &DXARCD, 0, sizeof( DXARC_DIR ) ) ;
	return 0 ;
}

/** アーカイブをディレクトリに見立てる処理の後始末
 *
 *
 */
int		DXA_DIR_Terminate( void )
{
	// 使用されていないアーカイブファイルを解放する
	DXA_DIR_CloseWaitArchive() ;
	return 0 ;
}

/** アーカイブファイルの拡張子を設定する
 *
 *
 */
int		DXA_DIR_SetArchiveExtension( const char *Extension )
{
	int Length ;
	Length = strlen( Extension ) ;
	if( Length >= 64 || Extension == NULL || Extension[0] == '\0' )
	{
		DXARCD.ArchiveExtension[0] = 0 ;
		DXARCD.ArchiveExtensionLength = 0 ;
	}
	else
	{
		DXARCD.ArchiveExtensionLength = Length ;
		memcpy( DXARCD.ArchiveExtension, Extension, Length + 1 ) ;
	}
	return 0 ;
}

/** アーカイブファイルの鍵文字列を設定する
 *
 *
 */
int		DXA_DIR_SetKeyString( const char *KeyString )
{
	if( KeyString == NULL )
	{
		DXARCD.ValidKeyString = FALSE ;
	}
	else
	{
		DXARCD.ValidKeyString = TRUE ;
		if( strlen( KeyString ) > DXA_KEYSTR_LENGTH )
		{
			memcpy( DXARCD.KeyString, KeyString, DXA_KEYSTR_LENGTH ) ;
			DXARCD.KeyString[ DXA_KEYSTR_LENGTH ] = '\0' ;
		}
		else
		{
			strcpy( DXARCD.KeyString, KeyString ) ;
		}
	}
	return 0 ;
}

/** ファイルポインタの位置を取得する
 *
 *
 */
long	DXA_DIR_Tell( int Handle )
{
	DXARC_DIR_FILE *file = DXARCD.File[Handle & 0x0FFFFFFF] ;
	if( file == NULL ) return -1 ;
	//if( file->UseArchiveFlag == 0 )	return WinFileAccessTell( file->FilePointer ) ;
	//else							return DXA_STREAM_Tell( &file->DXAStream ) ;
	return 0;
}

/** ファイルの終端を調べる
 *
 *
 */
int		DXA_DIR_Eof( int Handle )
{
	DXARC_DIR_FILE *file = DXARCD.File[Handle & 0x0FFFFFFF] ;
	if( file == NULL ) return -1 ;
	//if( file->UseArchiveFlag == 0 ) return WinFileAccessEof( file->FilePointer ) ;
	//else							return DXA_STREAM_Eof( &file->DXAStream ) ;
	return 0;
}

/**
 *
 *
 */
int		DXA_DIR_ChDir( const char *Path )
{
	//::SetCurrentDirectory( Path ) ;
	return 0 ;
}

/**
 *
 *
 */
int		DXA_DIR_GetDir( char *Buffer )
{
	//::GetCurrentDirectory( 256, Buffer ) ;
	return 0 ;
}

/**
 *
 *
 */
int		DXA_DIR_IdleCheck( int Handle )
{
	DXARC_DIR_FILE *file = DXARCD.File[Handle & 0x0FFFFFFF] ;
	if( file == NULL ) return -1 ;
	//if( file->UseArchiveFlag == 0 )	return WinFileAccessIdleCheck( file->FilePointer ) ;
	//else							return DXA_STREAM_IdleCheck( &file->DXAStream ) ;
	return 0;
}

/** 戻り値: -1=エラー  -1以外=FindHandle
 *
 *
 */
int		DXA_DIR_FindFirst( const char *FilePath, FILEINFO *Buffer )
{
	DXA_DIR_FINDDATA *find;
	char nPath[256];
	// メモリの確保
	find = ( DXA_DIR_FINDDATA * )DXALLOC( sizeof( DXA_DIR_FINDDATA ) );
	if( find == NULL ) return -1;
	memset( find, 0, sizeof( *find ) );
	// 指定のオブジェクトがアーカイブファイル内か調べる
	if( DXA_DIR_OpenTest( FilePath, &find->UseArchiveIndex, NULL, nPath ) == -1 )
	{
		// アーカイブファイル内ではなかった場合はファイルから検索する
		find->UseArchiveFlag = 0;
		//find->FindHandle = WinFileAccessFindFirst( FilePath, Buffer );
	}
	else
	{
		// アーカイブファイル内の場合はアーカイブファイル内から検索する
		find->UseArchiveFlag = 1;
		find->FindHandle = DXA_FindFirst( &DXARCD.Archive[ find->UseArchiveIndex ]->Archive, nPath, Buffer );
	}
	// 検索ハンドルが取得できなかった場合はエラー
	if( find->FindHandle == -1 )
	{
		DXFREE( find );
		return -1;
	}
	return (int)find;	// ハンドルを返す
}

/** 戻り値: -1=エラー  0=成功
 *
 *
 */
int		DXA_DIR_FindNext( int FindHandle, FILEINFO *Buffer )
{
	DXA_DIR_FINDDATA *find;
	find = (DXA_DIR_FINDDATA *)FindHandle;
	if( find->UseArchiveFlag == 0 )
		return 0;//WinFileAccessFindNext( find->FindHandle, Buffer );
	else
		return DXA_FindNext( find->FindHandle, Buffer );
}

/** 戻り値: -1=エラー  0=成功
 *
 *
 */
int		DXA_DIR_FindClose( int FindHandle )
{
	DXA_DIR_FINDDATA *find;
	find = (DXA_DIR_FINDDATA *)FindHandle;
	if( find->UseArchiveFlag == 0 )
	{
		//WinFileAccessFindClose( find->FindHandle );
	}
	else
	{
		DXA_FindClose( find->FindHandle );
		DXA_DIR_CloseArchive( find->UseArchiveIndex ) ;
	}
	DXFREE( find );
	return 0;
}

/** 指定のＤＸＡファイルを丸ごとメモリに読み込む( 戻り値: -1=エラー  0=成功 )
 *
 *
 */
int		ST_DXArchivePreLoad( const char *FilePath , int ASync )
{
	char fullpath[256];
	// フルパスを得る(ついでに全ての文字を大文字にする)
	DXA_DIR_ConvertFullPath( FilePath, fullpath ) ;
	return DXA_DIR_OpenArchive( fullpath, -1, TRUE, ASync ) == -1 ? -1 : 0;
}

/** 指定のＤＸＡファイルの事前読み込みが完了したかどうかを取得する( 戻り値： TRUE=完了した FALSE=まだ )
 *
 *
 */
int		ST_DXArchiveCheckIdle( const char *FilePath )
{
	int handle;
	char fullpath[256];
	// フルパスを得る(ついでに全ての文字を大文字にする)
	DXA_DIR_ConvertFullPath( FilePath, fullpath ) ;
	// ファイルパスからハンドルを取得する
	handle = DXA_DIR_GetArchive( fullpath );
	if( handle == -1 ) return 0 ;
	// 準備が完了したかどうかを得る
	return DXA_CheckIdle( &DXARCD.Archive[handle]->Archive );
}

/** 指定のＤＸＡファイルをメモリから解放する
 *
 *
 */
int		ST_DXArchiveRelease( const char *FilePath )
{
	int handle;
	char fullpath[256];
	// フルパスを得る(ついでに全ての文字を大文字にする)
	DXA_DIR_ConvertFullPath( FilePath, fullpath ) ;
	// ファイルパスからハンドルを取得する
	handle = DXA_DIR_GetArchive( fullpath );
	if( handle == -1 ) return 0 ;
	DXA_DIR_CloseArchive( handle ) ;
	DXA_DIR_CloseWaitArchive() ;
	return 0 ;
}

/** ファイルを閉じる
 *
 *
 */
int		DXA_DIR_Close( int Handle )
{
	DXARC_DIR_FILE *file = DXARCD.File[Handle & 0x0FFFFFFF] ;
	if( file == NULL ) return -1;		// 使用されていなかったら何もせず終了
	if( file->UseArchiveFlag == FALSE )	// アーカイブを使用しているかどうかで分岐
	{
		//WinFileAccessClose( file->FilePointer ) ;
		file->FilePointer = DXA_FILE_NO_SET;
	}
	else
	{
		// アーカイブファイルの参照数を減らす
		DXA_DIR_CloseArchive( file->UseArchiveIndex ) ;
		// アーカイブファイルの後始末
		DXA_STREAM_Terminate( &file->DXAStream ) ;
	}
	DXFREE( file ) ;	// メモリを解放する
	DXARCD.File[Handle & 0x0FFFFFFF] = NULL ;
	DXARCD.FileNum -- ;	// 数を減らす
	return 0 ;
}

/* ファイルからデータを読み込む
 *
 *
 */
size_t	DXA_DIR_Read( void *Buffer, size_t BlockSize, size_t BlockNum, int Handle )
{
	DXARC_DIR_FILE *file = DXARCD.File[Handle & 0x0FFFFFFF] ;
	if( file == NULL ) return -1 ;
	//if( file->UseArchiveFlag == 0 )	return WinFileAccessRead( Buffer, BlockSize, BlockNum, file->FilePointer ) ;
	//else							return DXA_STREAM_Read( &file->DXAStream, Buffer, BlockSize * BlockNum ) ;
	return 0;
}

/** ファイルポインタの位置を変更する
 *
 *
 */
int		DXA_DIR_Seek( int Handle, long SeekPoint, int SeekType )
{
	DXARC_DIR_FILE *file = DXARCD.File[Handle & 0x0FFFFFFF] ;
	if( file == NULL ) return -1 ;
	//if( file->UseArchiveFlag == 0 )	return WinFileAccessSeek( file->FilePointer, SeekPoint, SeekType ) ;
	//else							return DXA_STREAM_Seek( &file->DXAStream, SeekPoint, SeekType ) ;
	return 0;
}

/** アーカイブファイルをフォルダに見立ててファイルを開く時の情報を得る( -1:アーカイブとしては存在しなかった  0:存在した )
 *
 *
 */
int		DXA_DIR_OpenTest( const char *FilePath, int *ArchiveIndex, char *ArchivePath, char *ArchiveFilePath )
{
	int i, len, arcindex ;
	char fullpath[256], path[256], temp[256], dir[256], *p ;
	// フルパスを得る(ついでに全ての文字を大文字にする)
	DXA_DIR_ConvertFullPath( FilePath, fullpath ) ;
	// 前回と使用するアーカイブのパスが同じ場合は同じアーカイブを使用する
	if( DXARCD.BUDirPathLen != 0 &&
		strncmp( fullpath, DXARCD.BUDir, DXARCD.BUDirPathLen ) == 0 &&
		( fullpath[DXARCD.BUDirPathLen] == '\\' ||
		fullpath[DXARCD.BUDirPathLen] == '/' ) )
	{
		// 前回使用したＤＸＡファイルを開く
		arcindex = DXA_DIR_OpenArchive( DXARCD.BUDir, DXARCD.BUArcIndex ,FALSE,FALSE ) ;
		if( arcindex == -1 ) return -1;
		// ＤＸＡファイルがある場所以降のパスを作成する
		p = &fullpath[ DXARCD.BUDirPathLen + 1 ] ;
	}
	else
	{
		// 前回とは違うパスの場合は一から調べる
		// ディレクトリを一つ一つ追って行く
		p = fullpath ;
		len = 0 ;
		for(;;)
		{
			// ネットワークを介していた場合の専用処理
			if( p - fullpath == 0 && fullpath[0] == '\\' && fullpath[1] == '\\' )
			{
				path[0] = '\\';
				path[1] = '\\';
				path[2] = '\0';
				len += 2;
				p += 2;
			}
			// ディレクトリを一つ取る
			for( i = 0 ; *p != '\0' && *p !=  '/' && *p != '\\' ; p ++, i ++ )
			{
				dir[i] = path[len+i] = *p ;
			}
			if( *p == '\0' || i == 0 ) return -1;
			p ++ ;
			dir[i] = path[len+i] = '\0' ;
			len += i ;
			// フォルダ名をDXアーカイブファイル名にする
			memcpy( temp, path, len ) ;
			temp[len] = '.' ;
			if( DXARCD.ArcExtLen == 0 )	memcpy( &temp[len+1], "DXA", 4 ) ;
			else						memcpy( &temp[len+1], DXARCD.ArcExt, DXARCD.ArcExtLen + 1 ) ;
			// ＤＸＡファイルとして開いてみる
			arcindex = DXA_DIR_OpenArchive( temp ,-1,FALSE,FALSE) ;
			if( arcindex != -1 ) break ;
			// 開けなかったら次の階層へ
			path[len] = '\\' ;
			len ++ ;
		}
		// 開けたら今回の情報を保存する
		if( DXARCD.ArcExtLen == 0 )
			memcpy( DXARCD.BUDir, temp, len + 3 + 2 ) ;
		else
			memcpy( DXARCD.BUDir, temp, len + DXARCD.ArcExtLen + 2 ) ;
		DXARCD.BUDirPathLen = len ;
		DXARCD.BUArcIndex   = arcindex ;
	}
	// 情報をセットする
	*ArchiveIndex = arcindex;
	if( ArchivePath     ) strcpy( ArchivePath,     DXARCD.BUDir );
	if( ArchiveFilePath ) strcpy( ArchiveFilePath, p                       );
	return 0;
}

/** ファイルを開く( エラー：-1  成功：ハンドル )
 *
 * FileRead_openと差しかえ
 */
int		DXA_DIR_Open( const char *FilePath)//, int UseCacheFlag, int BlockReadFlag, int UseASyncReadFlag )
{
	int index ;
	DXARC_DIR_FILE *file ;
	char DxaInFilePath[256];
	if( DXARCD.FileNum == DXA_DIR_MAXFILENUM )	return -1;// 空きデータが無い
	for( index = 0 ; DXARCD.File[index] != NULL ; index ++ ){}
	// メモリの確保
	DXARCD.File[index] = (DXARC_DIR_FILE *)DXALLOC( sizeof( DXARC_DIR_FILE ) ) ;
	if( DXARCD.File[index] == NULL )			return -1;//暫定的にハンドルを返す
	file = DXARCD.File[index] ;
	// アーカイブファイルが無いか調べる
	if( DXA_DIR_OpenTest( FilePath, (int *)&file->UseArchiveIndex, NULL, DxaInFilePath ) == 0 )
	{
		file->UseArchiveFlag = 1 ;		// アーカイブを使用しているフラグを立てる
		// ディレクトリ名と同名のＤＸＡファイルを開けたらその中から指定のファイルを読み込もうとする
		if( DXA_STREAM_Initialize( &file->DXAStream,
			&DXARCD.Archive[ file->UseArchiveIndex ]->Archive,
			DxaInFilePath/*, UseASyncReadFlag*/ ) < 0 )
		{
			DXA_DIR_CloseArchive( file->UseArchiveIndex ) ;
			goto ERR ;
		}
	}
	else
	{
		file->UseArchiveFlag = 0;		// 開いたら普通のファイルから読み込む設定を行う
		file->FilePointer = FileRead_open(FilePath);
		if( file->FilePointer == DXA_FILE_NO_SET ) goto ERR ;
	}
	DXARCD.FileNum ++ ;			// ハンドルの数を増やす
	return index | 0xF0000000 ;	// インデックスを返す
ERR:
	// メモリの解放
	if( DXARCD.File[index] != NULL ) DXFREE( DXARCD.File[index] ) ;
	DXARCD.File[index] = NULL ;
	return -1;
}

/** ファイルを丸ごと読み込む関数
 *
 *
 */
/*int		DXA_DIR_LoadFile( const char *FilePath, void *Buffer, int BufferSize )
{
	int siz ;
	int handle ;
	handle = DXA_DIR_Open( FilePath,FALSE,TRUE,FALSE ) ;
	if( handle == -1 ) return false ;
	DXA_DIR_Seek( handle, 0L, SEEK_END ) ;
	siz = DXA_DIR_Tell( handle ) ;
	DXA_DIR_Seek( handle, 0L, SEEK_SET ) ;
	if( siz <= BufferSize )	DXA_DIR_Read( Buffer, siz, 1, handle ) ;
	DXA_DIR_Close( handle ) ;
	return siz ;
}*/
