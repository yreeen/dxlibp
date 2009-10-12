#include "dxlibp.h"
#include "dxparchive.h"
#include <stdio.h>
#include <string.h>

// 非同期読み込み状態
#define DXARC_STREAM_ASYNCSTATE_IDLE			(0)				// 何もしていない
#define DXARC_STREAM_ASYNCSTATE_PRESSREAD		(1)				// 圧縮されたファイルを読み込み中
#define DXARC_STREAM_ASYNCSTATE_READ			(2)				// データを読み込み中

//dxparc_decode.c
int		DXA_Decode( void *Src, void *Dest );

/** ファイルポインタを変更する
 *
 *
 */
int		DXA_STREAM_Seek( DXARC_STREAM *DXAStream, int SeekPoint, int SeekMode )
{
	// 非同期読み込みで状態がまだ待機状態ではなかったら待機状態になるまで待つ
	if( DXAStream->UseASyncReadFlag == TRUE && DXAStream->ASyncState != DXARC_STREAM_ASYNCSTATE_IDLE )
	{
		while( DXA_STREAM_IdleCheck( DXAStream ) == FALSE ) Sleep(1);
	}
	// シークタイプによって処理を分岐
	switch( SeekMode )
	{
	case SEEK_SET : break ;		
	case SEEK_CUR : SeekPoint += DXAStream->FilePoint ; break ;
	case SEEK_END :	SeekPoint = DXAStream->FileHead->DataSize + SeekPoint ; break ;
	}
	// 補正
	if( SeekPoint > (int)DXAStream->FileHead->DataSize ) SeekPoint = DXAStream->FileHead->DataSize ;
	if( SeekPoint < 0 ) SeekPoint = 0 ;
	// セット
	DXAStream->FilePoint = SeekPoint ;
	// EOFフラグを倒す
	DXAStream->EOFFlag = FALSE ;
	return 0 ;
}

/** 現在のファイルポインタを得る
 *
 *
 */
int		DXA_STREAM_Tell( DXARC_STREAM *DXAStream )
{
	// 非同期読み込みで状態がまだ待機状態ではなかったら待機状態になるまで待つ
	if( DXAStream->UseASyncReadFlag == TRUE && DXAStream->ASyncState != DXARC_STREAM_ASYNCSTATE_IDLE )
	{
		while( DXA_STREAM_IdleCheck( DXAStream ) == FALSE ) Sleep(1);
	}
	return DXAStream->FilePoint ;
}

/** ファイルの終端に来ているか、のフラグを得る
 *
 *
 */
int		DXA_STREAM_Eof( DXARC_STREAM *DXAStream )
{
	// 非同期読み込みで状態がまだ待機状態ではなかったら待機状態になるまで待つ
	if( DXAStream->UseASyncReadFlag == TRUE && DXAStream->ASyncState != DXARC_STREAM_ASYNCSTATE_IDLE )
	{
		while( DXA_STREAM_IdleCheck( DXAStream ) == FALSE ) Sleep(1);
	}
	return DXAStream->EOFFlag ? EOF : 0 ;
}

/** 読み込み処理が完了しているかどうかを調べる
 *
 *
 */
int		DXA_STREAM_IdleCheck( DXARC_STREAM *DXAStream )
{
	// 非同期読み込みではない場合は何もせず TRUE を返す
	if( DXAStream->UseASyncReadFlag == FALSE ) return TRUE ;
	// 状態によって処理を分岐
	switch( DXAStream->ASyncState )
	{
	case DXARC_STREAM_ASYNCSTATE_IDLE:			// 待機状態
		return TRUE;
	case DXARC_STREAM_ASYNCSTATE_PRESSREAD:		// 圧縮データ読み込み待ち
		// 読み込み終了待ち
		//if( WinFileAccessIdleCheck( DXAStream->Archive->FilePointer ) == FALSE ) return FALSE;
		// 読み込み終わったらまず鍵を外す
		DXA_KeyConv( DXAStream->DecodeTempBuffer, DXAStream->FileHead->PressDataSize, DXAStream->ASyncReadFileAddress, DXAStream->Archive->Key ) ;
		// 解凍
		DXA_Decode( DXAStream->DecodeTempBuffer, DXAStream->DecodeDataBuffer ) ;
		// メモリの解放
		DXFREE( DXAStream->DecodeTempBuffer ) ;
		DXAStream->DecodeTempBuffer = NULL ;
		// 状態を待機状態にする
		DXAStream->ASyncState = DXARC_STREAM_ASYNCSTATE_IDLE;
		return TRUE;
	case DXARC_STREAM_ASYNCSTATE_READ:			// 読み込み待ち
		// 読み込み終了待ち
		//if( WinFileAccessIdleCheck( DXAStream->Archive->FilePointer ) == FALSE ) return FALSE;
		// 読み込み終わったら鍵を外す
		DXA_KeyConv( DXAStream->ReadBuffer, DXAStream->ReadSize, DXAStream->ASyncReadFileAddress, DXAStream->Archive->Key ) ;
		// 状態を待機状態にする
		DXAStream->ASyncState = DXARC_STREAM_ASYNCSTATE_IDLE;
		return TRUE;
	}
	return TRUE ;
}

/** ファイルのサイズを取得する
 *
 *
 */
int		DXA_STREAM_Size( DXARC_STREAM *DXAStream )
{
	return DXAStream->FileHead->DataSize ;
}

/** ファイルの内容を読み込む
 *
 *
 */
int		DXA_STREAM_Read( DXARC_STREAM *DXAStream, void *Buffer, int ReadLength )
{
	int ReadSize ;
	// 非同期読み込みで状態がまだ待機状態ではなかったら待機状態になるまで待つ
	if( DXAStream->UseASyncReadFlag == TRUE && DXAStream->ASyncState != DXARC_STREAM_ASYNCSTATE_IDLE )
	{
		while( DXA_STREAM_IdleCheck( DXAStream ) == FALSE ) Sleep(1);
	}
	// EOF フラグが立っていたら０を返す
	if( DXAStream->EOFFlag == TRUE ) return 0 ;
	// EOF 検出
	if( DXAStream->FileHead->DataSize == DXAStream->FilePoint )
	{
		DXAStream->EOFFlag = TRUE ;
		return 0 ;
	}
	// データを読み込む量を設定する
	ReadSize = ReadLength < (int)( DXAStream->FileHead->DataSize - DXAStream->FilePoint ) ? ReadLength : DXAStream->FileHead->DataSize - DXAStream->FilePoint ;
	// データが圧縮されていたかどうかで処理を分岐
	if( DXAStream->DecodeDataBuffer != NULL )
	{
		// データをコピーする
		memcpy( Buffer, (BYTE *)DXAStream->DecodeDataBuffer + DXAStream->FilePoint, ReadSize ) ;
	}
	else
	{
		// メモリ上にデータがあるかどうかで処理を分岐
		if( DXAStream->Archive->MemoryOpenFlag == TRUE )
		{
			// メモリ上にある場合
			// データをコピーする
			memcpy( Buffer, (BYTE *)DXAStream->Archive->FilePointer + DXAStream->Archive->Head.DataStartAddress + DXAStream->FileHead->DataAddress + DXAStream->FilePoint, ReadSize ) ;
		}
		else
		{
			// ファイルから読み込んでいる場合
			// アーカイブファイルポインタと、仮想ファイルポインタが一致しているか調べる
			// 一致していなかったらアーカイブファイルポインタを移動する
			DXAStream->ASyncReadFileAddress = (int)( DXAStream->FileHead->DataAddress + DXAStream->Archive->Head.DataStartAddress + DXAStream->FilePoint );
			//if( WinFileAccessTell( DXAStream->Archive->FilePointer ) != DXAStream->ASyncReadFileAddress )
			{
			//	WinFileAccessSeek( DXAStream->Archive->FilePointer, DXAStream->ASyncReadFileAddress, SEEK_SET ) ;
			}
			// 非同期読み込みの場合と同期読み込みの場合で処理を分岐
			if( DXAStream->UseASyncReadFlag )
			{
				// ファイルから読み込み
				//WinFileAccessRead( Buffer, ReadSize, 1, DXAStream->Archive->FilePointer ) ;
				DXAStream->ReadBuffer = Buffer;
				DXAStream->ReadSize = ReadSize;
				DXAStream->ASyncState = DXARC_STREAM_ASYNCSTATE_READ ;
			}
			else
			{
				// データを読み込む
				DXA_KeyConvFileRead( Buffer, ReadSize, DXAStream->Archive->FilePointer, DXAStream->Archive->Key,-1 ) ;
			}
		}
	}
	// EOF フラグを倒す
	DXAStream->EOFFlag = FALSE ;
	// 読み込んだ分だけファイルポインタを移動する
	DXAStream->FilePoint += ReadSize ;
	return ReadSize ;// 読み込んだ容量を返す
}
	
/** アーカイブファイル内のファイルを閉じる
 *
 *
 */
int		DXA_STREAM_Terminate( DXARC_STREAM *DXAStream )
{
	// 非同期読み込みで状態がまだ待機状態ではなかったら待機状態になるまで待つ
	if( DXAStream->UseASyncReadFlag == TRUE && DXAStream->ASyncState != DXARC_STREAM_ASYNCSTATE_IDLE )
	{
		while( DXA_STREAM_IdleCheck( DXAStream ) == FALSE ) Sleep(1);
	}
	// メモリの解放
	if( DXAStream->DecodeDataBuffer != NULL )
	{
		DXFREE( DXAStream->DecodeDataBuffer ) ;
		DXAStream->DecodeDataBuffer = NULL ;
	}
	if( DXAStream->DecodeTempBuffer != NULL )
	{
		DXFREE( DXAStream->DecodeTempBuffer ) ;
		DXAStream->DecodeTempBuffer = NULL ;
	}
	// ゼロ初期化
	memset( DXAStream, 0, sizeof( DXARC_STREAM ) ) ;
	return 0 ;
}

/** アーカイブファイル内のファイルを開く(ファイル閉じる作業は必要なし)
 *
 *
 */
int		DXA_STREAM_Initialize( DXARC_STREAM *DXAStream, DXARC *DXA, const char *FilePath)//, int UseASyncReadFlag )
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
	// データのセット
	DXAStream->Archive          = DXA ;
	DXAStream->FileHead         = FileH ;
	DXAStream->EOFFlag          = FALSE ;
	DXAStream->FilePoint        = 0 ;
	DXAStream->DecodeDataBuffer = NULL ;
	DXAStream->DecodeTempBuffer = NULL ;
	DXAStream->UseASyncReadFlag = UseASyncReadFlag ;
	DXAStream->ASyncState       = DXARC_STREAM_ASYNCSTATE_IDLE ;
	// ファイルが圧縮されている場合はここで読み込んで解凍してしまう
	if( DXA->Head.Version >= 0x0002 && FileH->PressDataSize != 0xffffffff )
	{
		// 解凍データが収まるメモリ領域の確保
		DXAStream->DecodeDataBuffer = DXALLOC( FileH->DataSize ) ;
		// メモリ上に読み込まれているかどうかで処理を分岐
		if( DXA->MemoryOpenFlag == TRUE )
		{
			// 解凍
			DXA_Decode( (BYTE *)DXA->FilePointer + DXA->Head.DataStartAddress + FileH->DataAddress, DXAStream->DecodeDataBuffer ) ;
		}
		else
		{
			// 圧縮データが収まるメモリ領域の確保
			DXAStream->DecodeTempBuffer = DXALLOC( FileH->PressDataSize ) ;
			// 圧縮データの読み込み
			DXAStream->ASyncReadFileAddress = DXA->Head.DataStartAddress + FileH->DataAddress;
			//WinFileAccessSeek( DXA->FilePointer, DXAStream->ASyncReadFileAddress, SEEK_SET ) ;
			// 非同期の場合は読み込みと鍵解除を別々に行う
			if( DXAStream->UseASyncReadFlag == TRUE )
			{
				// ファイルから読み込み
				//WinFileAccessRead( DXAStream->DecodeTempBuffer, FileH->PressDataSize, 1, DXA->FilePointer ) ;
				DXAStream->ASyncState = DXARC_STREAM_ASYNCSTATE_PRESSREAD ;
			}
			else
			{
				DXA_KeyConvFileRead( DXAStream->DecodeTempBuffer, FileH->PressDataSize, DXA->FilePointer, DXA->Key,-1 ) ;
				// 解凍
				DXA_Decode( DXAStream->DecodeTempBuffer, DXAStream->DecodeDataBuffer ) ;
				// メモリの解放
				DXFREE( DXAStream->DecodeTempBuffer ) ;
				DXAStream->DecodeTempBuffer = NULL ;
			}
		}
	}
	return 0 ;
}

