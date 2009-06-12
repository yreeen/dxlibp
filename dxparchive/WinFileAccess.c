#include "dxlibp.h"
#include "dxparchive.h"
typedef void *HANDLE;
// ファイルアクセス専用スレッド用構造体
typedef struct tagFILEACCESSTHREAD
{
	DWORD					ThreadID ;							// スレッドＩＤ
	HANDLE					ThreadHandle ;						// スレッドハンドル
	HANDLE					Handle ;							// ファイルアクセス用ハンドル
	HANDLE					FuncEvent ;							// 指令受け取り用ハンドル
	HANDLE					CompEvent ;							// 指令完了用ハンドル
	int						Function ;							// 指令( FILEACCESSTHREAD_FUNCTION_OPEN 等 )

	int						EndFlag ;							// 終了したか、フラグ
	int						ErrorFlag ;							// エラー発生フラグ

	char					FilePath[MAX_PATH] ;				// ファイルパス
	void					*ReadBuffer ;						// 読み込むデータを格納するバッファー
	DWORD					ReadPosition ;						// 読み込むデータの位置
	DWORD					ReadSize ;							// 読み込むデータのサイズ(読み込めたデータのサイズ)
	long					SeekPoint ;							// ファイルポインタを移動する位置 

	BYTE					*CacheBuffer ;						// キャッシュバッファへのポインタ
	DWORD					CachePosition ;						// キャッシュしているファイル位置
	int						CacheSize ;							// キャッシュバッファの有効データサイズ
} FILEACCESSTHREAD ;

// ファイルアクセス処理用構造体
typedef struct tagWINFILEACCESS
{
	HANDLE					Handle ;							// ファイルアクセス用ハンドル
	int						UseThread ;							// スレッドを使用するかどうか
	int						UseCacheFlag ;						// キャッシュを使用するかどうか
	int						UseASyncReadFlag ;					// 非同期読み込みを行うかどうか
	int						EofFlag ;							// 終端チェックフラグ
	DWORD					Position ;							// アクセス位置
	DWORD					Size ;								// サイズ

	FILEACCESSTHREAD		ThreadData ;						// 読み込み専用スレッドのデータ
} WINFILEACCESS ;

extern char *_STRCHR( const char *Str1, char Char );
extern void _MEMSET( void *Memory, unsigned char Char, int Size );
extern void _MEMCPY( void *Buffer1, const void *Buffer2, int Size );
extern void _STRCPY( char *Dest, const char *Src );
extern void _STRCAT( char *Dest, const char *Src );
extern int _STRCMP( const char *Str1, const char *Str2 );
extern char *_STRUPR( char *Str );
extern int _STRNCMP( const char *Str1, const char *Str2, int Size );



extern int WinFileAccessOpen( const char *Path, int UseCacheFlag, int BlockReadFlag, int UseASyncReadFlag )
{
	WINFILEACCESS *FileAccess ;
	DWORD Code;
	
//	UseCacheFlag = UseCacheFlag ;
	BlockReadFlag = BlockReadFlag ;

	FileAccess = (WINFILEACCESS *)DXALLOC( sizeof( WINFILEACCESS ) ) ;
	//if( FileAccess == NULL ) return NULL ;
	if( FileAccess == NULL ) return -1;

	_MEMSET( FileAccess, 0, sizeof( WINFILEACCESS ) ) ;

//	// キャッシュを使用するかどうかをスレッドを使用するかどうかにしてしまう
//	FileAccess->UseThread = UseCacheFlag ;

	// キャッシュを使用するかどうかのフラグをセット
	FileAccess->UseCacheFlag = UseCacheFlag ;
	FileAccess->ThreadData.CacheBuffer = NULL;

	// 非同期読み書きフラグをセット
	FileAccess->UseASyncReadFlag = UseASyncReadFlag ;

	// キャッシュ、若しくは非同期読み書きを行う場合はスレッドを使用する
	FileAccess->UseThread = FileAccess->UseCacheFlag || FileAccess->UseASyncReadFlag ;

	// スレッドを使用する場合としない場合で処理を分岐
	if( FileAccess->UseThread == TRUE )
	{
		// スレッドを使用する場合はファイルアクセス専用スレッドを立てる

		// 最初にファイルを開けるかどうか確かめておく
		//FileAccess->Handle = CreateFile( Path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ;
		//if( FileAccess->Handle == INVALID_HANDLE_VALUE )
		{
			DXFREE( FileAccess ) ;
			//return NULL ;
			return -1;
		}
		//FileAccess->Size = GetFileSize( FileAccess->Handle, NULL ) ;
		//CloseHandle( FileAccess->Handle ) ;
		FileAccess->Handle = NULL ;

		// キャッシュ用メモリの確保
		if( FileAccess->UseCacheFlag )
		{
			FileAccess->ThreadData.CacheBuffer = (BYTE *)DXALLOC( FILEACCESSTHREAD_DEFAULT_CACHESIZE );
			if( FileAccess->ThreadData.CacheBuffer == NULL )
			{
				DXFREE( FileAccess->ThreadData.CacheBuffer ) ;
				DXFREE( FileAccess ) ;
				DXST_ERRORLOG_ADD( "ファイル読み込みキャッシュ用メモリの確保に失敗しました\n" ) ;
				//return NULL ;
				return -1;
			}
		}

		// 専用スレッドデータを初期化する
		FileAccess->ThreadData.Handle = NULL ;
		FileAccess->ThreadData.ThreadHandle = NULL ;
		FileAccess->ThreadData.FuncEvent = CreateEvent( NULL, TRUE, FALSE, NULL ) ;
		FileAccess->ThreadData.CompEvent = CreateEvent( NULL, TRUE, TRUE, NULL ) ;

		FileAccess->ThreadData.ThreadHandle = CreateThread(
												NULL,
												0,
												(LPTHREAD_START_ROUTINE)FileAccessThreadFunction, 
												&FileAccess->ThreadData,
												0,
												&FileAccess->ThreadData.ThreadID ) ;
		if( FileAccess->ThreadData.ThreadHandle == NULL )
		{
			if( FileAccess->ThreadData.CacheBuffer ) DXFREE( FileAccess->ThreadData.CacheBuffer ) ;
			CloseHandle( FileAccess->ThreadData.FuncEvent ) ;
			CloseHandle( FileAccess->ThreadData.CompEvent ) ;
			DXFREE( FileAccess ) ;
			DXST_ERRORLOG_ADD( "ファイルアクセス専用スレッドの作成に失敗しました\n" ) ;
			//return NULL ;
			return -1;
		}

		// ファイルオープン指令はここで完了してしまう
		FileAccess->ThreadData.Function = FILEACCESSTHREAD_FUNCTION_OPEN ;
		_STRCPY( FileAccess->ThreadData.FilePath, Path ) ;
		ResetEvent( FileAccess->ThreadData.CompEvent ) ;
		SetEvent( FileAccess->ThreadData.FuncEvent ) ;

		// 指令が終了するまで待つ
		WaitForSingleObject( FileAccess->ThreadData.CompEvent, INFINITE ) ;
		if( FileAccess->ThreadData.ErrorFlag == TRUE )
		{
			if( FileAccess->ThreadData.CacheBuffer ) DXFREE( FileAccess->ThreadData.CacheBuffer ) ;
			CloseHandle( FileAccess->ThreadData.FuncEvent ) ;
			CloseHandle( FileAccess->ThreadData.CompEvent ) ;
			do
			{
				Sleep(0);
				GetExitCodeThread( FileAccess->ThreadData.ThreadHandle, &Code );
			}while( Code == STILL_ACTIVE );
			CloseHandle( FileAccess->ThreadData.ThreadHandle ) ;
			DXFREE( FileAccess ) ;
			DXST_ERRORLOG_ADD( "ファイルのオープンに失敗しました\n" ) ;
			//return NULL ;
			return -1;
		}
	}
	else
	{
		// スレッドを使用しない場合はこの場でファイルを開く
		FileAccess->Handle = CreateFile( Path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ;
		//if( FileAccess->Handle == INVALID_HANDLE_VALUE )
		{
			DXFREE( FileAccess ) ;
			//return NULL ;
			return -1;
		}
		//FileAccess->Size = GetFileSize( FileAccess->Handle, NULL ) ;
	}
	FileAccess->EofFlag = FALSE ;
	FileAccess->Position = 0 ;

	return (int)FileAccess ;
}

extern int WinFileAccessClose( int Handle )
{
	WINFILEACCESS *FileAccess = ( WINFILEACCESS * )Handle ;
	BOOL Result;
	DWORD Code ;

	// スレッドを使用する場合としない場合で処理を分岐
	if( FileAccess->UseThread == TRUE )
	{
		// これ以前の指令が出ていた場合の為に指令完了イベントがシグナル状態になるまで待つ
		WaitForSingleObject( FileAccess->ThreadData.CompEvent, INFINITE ) ;

		// スレッドに終了指令を出す
		FileAccess->ThreadData.Function = FILEACCESSTHREAD_FUNCTION_EXIT ;
		ResetEvent( FileAccess->ThreadData.CompEvent ) ;
		SetEvent( FileAccess->ThreadData.FuncEvent ) ;

		// 指令が終了するまで待つ
		WaitForSingleObject( FileAccess->ThreadData.CompEvent, INFINITE ) ;

		// スレッドが終了するのを待つ
		do
		{
			Sleep(0);
			GetExitCodeThread( FileAccess->ThreadData.ThreadHandle, &Code );
		}while( Code == STILL_ACTIVE );

		// キャッシュを使用していた場合はキャッシュ用メモリを開放する
		if( FileAccess->ThreadData.CacheBuffer )
			DXFREE( FileAccess->ThreadData.CacheBuffer ) ;

		// イベントやスレッドを閉じる
		CloseHandle( FileAccess->ThreadData.ThreadHandle ) ;
		CloseHandle( FileAccess->ThreadData.CompEvent ) ;
		CloseHandle( FileAccess->ThreadData.FuncEvent ) ;
		Result = 0 ;
	}
	else
	{
		// 使用していない場合はこの場でハンドルを閉じて終了
		Result = CloseHandle( FileAccess->Handle ) ;
	}
	DXFREE( FileAccess ) ;

	return Result != 0 ? 0 : -1/*EOF*/ ;
}

extern long WinFileAccessTell( int Handle )
{
	WINFILEACCESS *FileAccess = ( WINFILEACCESS * )Handle ;

	return FileAccess->Position ;
}

extern int WinFileAccessSeek( int Handle, long SeekPoint, int SeekType )
{
	WINFILEACCESS *FileAccess = ( WINFILEACCESS * )Handle ;
	LONG Pos ;
	DWORD Result ;

	switch( SeekType )
	{
	case SEEK_CUR : Pos = ( LONG )FileAccess->Position + SeekPoint ; break ;
	case SEEK_END : Pos = ( LONG )FileAccess->Size + SeekPoint ; break ;
	case SEEK_SET : Pos = SeekPoint ; break ;
	}

	// スレッドを使用しているかどうかで処理を分岐
	if( FileAccess->UseThread == TRUE )
	{
		// これ以前の指令が出ていた場合の為に指令完了イベントがシグナル状態になるまで待つ
		WaitForSingleObject( FileAccess->ThreadData.CompEvent, INFINITE ) ;

		// スレッドにファイル位置変更指令を出す
		FileAccess->ThreadData.Function = FILEACCESSTHREAD_FUNCTION_SEEK ;
		FileAccess->ThreadData.SeekPoint = Pos ;
		ResetEvent( FileAccess->ThreadData.CompEvent ) ;
		SetEvent( FileAccess->ThreadData.FuncEvent ) ;
	}
	else
	{
		// ファイルアクセス位置を変更する
		Result = SetFilePointer( FileAccess->Handle, Pos, NULL, FILE_BEGIN ) ;
		if( Result == 0xFFFFFFFF ) return -1 ;
	}

	// 位置を保存しておく
	FileAccess->Position = (DWORD)Pos ;

	// 終端チェックフラグを倒す
	FileAccess->EofFlag = FALSE ;

	// 終了
	return 0 ;
}

extern size_t WinFileAccessRead( void *Buffer, size_t BlockSize, size_t DataNum, int Handle )
{
	WINFILEACCESS *FileAccess = ( WINFILEACCESS * )Handle ;
	DWORD Result, BytesRead ;

	if( BlockSize == 0 ) return 0 ;

	// 終端チェック
	if( FileAccess->Position == FileAccess->Size )
	{
		FileAccess->EofFlag = TRUE ;
		return 0 ;
	}

	// 項目数調整
	if( BlockSize * DataNum + FileAccess->Position > FileAccess->Size )
	{
		DataNum = ( FileAccess->Size - FileAccess->Position ) / BlockSize ;
	}
	
	if( DataNum == 0 )
	{
		FileAccess->EofFlag = TRUE ;
		return 0 ;
	}

	// スレッドを使用しているかどうかで処理を分岐
	if( FileAccess->UseThread == TRUE )
	{
		// これ以前の指令が出ていた場合の為に指令完了イベントがシグナル状態になるまで待つ
		WaitForSingleObject( FileAccess->ThreadData.CompEvent, INFINITE ) ;

		// スレッドにファイル読み込み指令を出す
		FileAccess->ThreadData.Function = FILEACCESSTHREAD_FUNCTION_READ ;
		FileAccess->ThreadData.ReadBuffer = Buffer ;
		FileAccess->ThreadData.ReadPosition = FileAccess->Position ;
		FileAccess->ThreadData.ReadSize = BlockSize * DataNum ;
		ResetEvent( FileAccess->ThreadData.CompEvent ) ;
		SetEvent( FileAccess->ThreadData.FuncEvent ) ;

		// 非同期かどうかで処理を分岐
		if( FileAccess->UseASyncReadFlag == FALSE )
		{
			// 同期読み込みの場合は指令が完了するまで待つ
			WaitForSingleObject( FileAccess->ThreadData.CompEvent, INFINITE ) ;
		}

		BytesRead = BlockSize * DataNum ;
		Result = 1 ;
	}
	else
	{
		Result = ReadFile( FileAccess->Handle, Buffer, BlockSize * DataNum, &BytesRead, NULL ) ;
	}

	FileAccess->Position += DataNum * BlockSize ;
	return Result != 0 ? BytesRead / BlockSize : 0 ;
}

extern int WinFileAccessEof( int Handle )
{
	WINFILEACCESS *FileAccess = (WINFILEACCESS *)Handle ;

	return FileAccess->EofFlag ? EOF : 0 ;
}

extern int WinFileAccessIdleCheck( int Handle )
{
	WINFILEACCESS *FileAccess = (WINFILEACCESS *)Handle ;

	if( FileAccess->UseThread == TRUE )
	{
		return WaitForSingleObject( FileAccess->ThreadData.CompEvent, 0 ) == WAIT_TIMEOUT ? FALSE : TRUE ;
	}
	else
	{
		return TRUE ;
	}
}

extern int WinFileAccessChDir( const char *Path )
{
	return SetCurrentDirectory( Path ) ;
}

extern int WinFileAccessGetDir( char *Buffer )
{
	return GetCurrentDirectory( MAX_PATH, Buffer ) ;
}

static void _WIN32_FIND_DATA_To_FILEINFO( WIN32_FIND_DATA *FindData, FILEINFO *FileInfo )
{
	// ファイル名のコピー
	_STRCPY( FileInfo->Name, FindData->cFileName );

	// ディレクトリかどうかのフラグをセット
	FileInfo->DirFlag = (FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 ? TRUE : FALSE;

	// ファイルのサイズをセット
	((DWORD *)&FileInfo->Size)[0] = FindData->nFileSizeLow;
	((DWORD *)&FileInfo->Size)[1] = FindData->nFileSizeHigh;

	// ファイルタイムを保存
	_FileTimeToLocalDateData( &FindData->ftCreationTime, &FileInfo->CreationTime );
	_FileTimeToLocalDateData( &FindData->ftLastWriteTime, &FileInfo->LastWriteTime );
}

// 戻り値: -1=エラー  -1以外=FindHandle
extern int WinFileAccessFindFirst( const char *FilePath, FILEINFO *Buffer )
{
	WIN32_FIND_DATA FindData;
	HANDLE FindHandle;

	FindHandle = FindFirstFile( FilePath, &FindData );
	if( FindHandle == INVALID_HANDLE_VALUE ) return -1;

	if( Buffer ) _WIN32_FIND_DATA_To_FILEINFO( &FindData, Buffer );

	return (int)FindHandle;
}

// 戻り値: -1=エラー  0=成功
extern int WinFileAccessFindNext( int FindHandle, FILEINFO *Buffer )
{
	WIN32_FIND_DATA FindData;

	if( FindNextFile( (HANDLE)FindHandle, &FindData ) == 0 ) return -1;

	if( Buffer ) _WIN32_FIND_DATA_To_FILEINFO( &FindData, Buffer );

	return 0;
}

// 戻り値: -1=エラー  0=成功
extern int WinFileAccessFindClose( int FindHandle )
{
	// ０以外が返ってきたら成功
	return FindClose( (HANDLE)FindHandle ) != 0 ? 0 : -1;
}



// テンポラリファイルを作成する
extern HANDLE CreateTemporaryFile( char *TempFileNameBuffer )
{
	char String1[MAX_PATH], String2[MAX_PATH] ;
	HANDLE FileHandle ;
	int Length ;

	// テンポラリファイルのディレクトリパスを取得する
	if( GetTempPath( 256, String1 ) == NULL ) return NULL ;

	// 文字列の最後に¥マークをつける
	Length = lstrlen( String1 ) ;
	if( String1[Length-1] != '\\' ) 
	{
		String1[Length] = '\\' ;
		String1[Length+1] = '\0' ;
	}

	// テンポラリファイルのファイル名を作成する
	if( GetTempFileName( String1, "tmp", 0, String2 ) == 0 ) return NULL ;

	// フルパスに変換
	_ConvertFullPath( String2, String1, NULL ) ;

	// テンポラリファイルを開く
	DeleteFile( String1 ) ;
	FileHandle = CreateFile( String1, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL ) ;
	if( FileHandle == NULL ) return NULL ;

	// テンポラリファイル名を保存
	if( TempFileNameBuffer != NULL ) lstrcpy( TempFileNameBuffer, String1 ) ;

	// ハンドルを返す
	return FileHandle ;
}

// ファイルアクセス専用スレッド用関数
DWORD WINAPI FileAccessThreadFunction( void *FileAccessThreadData )
{
	FILEACCESSTHREAD *dat = (FILEACCESSTHREAD *)FileAccessThreadData ;
	DWORD res, ReadSize ;

	for(;;)
	{
		for(;;)
		{
			// キャッシュを使用すかどうかで処理を分岐
			if( dat->CacheBuffer )
			{
				// 指令が来るまでちょっと待つ
				res = WaitForSingleObject( dat->FuncEvent, 100 ) ;

				// 指令が来てい無い場合でファイルが開いている場合はキャッシング処理を行う
				if( res == WAIT_TIMEOUT && dat->Handle != NULL )
				{
					// もしキャッシュが一杯だったら何もしない
					if( dat->CacheSize != FILEACCESSTHREAD_DEFAULT_CACHESIZE )
					{
						// 読み込み開始位置セット
						SetFilePointer( dat->Handle, dat->CachePosition + dat->CacheSize, NULL, FILE_BEGIN ) ;

						// 読み込み
						ReadFile( dat->Handle, &dat->CacheBuffer[dat->CacheSize], FILEACCESSTHREAD_DEFAULT_CACHESIZE - dat->CacheSize, &ReadSize, NULL ) ;
						
						// 有効なサイズを増やす
						dat->CacheSize += ReadSize ;
					}
				}
				else
				{
					break ;
				}
			}
			else
			{
				// 指令が来るまで待つ
				res = WaitForSingleObject( dat->FuncEvent, INFINITE ) ;
				if( res == WAIT_TIMEOUT && dat->Handle != NULL ) continue;
				break;
			}
		}

//		WaitForSingleObject( dat->FuncEvent, INFINITE ) ;

		// イベントのシグナル状態を解除する
		ResetEvent( dat->FuncEvent ) ;
		ResetEvent( dat->CompEvent ) ;

		// 指令が来たら判断する
		switch( dat->Function )
		{
		case FILEACCESSTHREAD_FUNCTION_OPEN :
			dat->Handle = CreateFile( dat->FilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ;
			if( dat->Handle == INVALID_HANDLE_VALUE )
			{
				dat->ErrorFlag = TRUE ;
				goto END ;
			}
			break ;

		case FILEACCESSTHREAD_FUNCTION_CLOSE :
			CloseHandle( dat->Handle ) ;
			dat->Handle = NULL ;
			break ;

		case FILEACCESSTHREAD_FUNCTION_READ :
			// キャッシュと読み込み位置が一致している場合はキャッシュからデータを転送する
			if( dat->CacheBuffer && dat->ReadPosition == dat->CachePosition && dat->CacheSize != 0 )
			{
				DWORD MoveSize ;

				// 転送するサイズを調整
				MoveSize = dat->ReadSize ;
				if( MoveSize > (DWORD)dat->CacheSize ) MoveSize = dat->CacheSize ;

				// 転送
				_MEMCPY( dat->ReadBuffer, dat->CacheBuffer, MoveSize ) ;

				// 読み込みサイズと読み込み位置を移動する
				dat->ReadBuffer = (void *)( (BYTE *)dat->ReadBuffer + MoveSize ) ;
				dat->ReadPosition += MoveSize ;
				dat->ReadSize -= MoveSize ;
				
				// キャッシュの情報も更新
				dat->CachePosition += MoveSize ;
				dat->CacheSize     -= MoveSize ;
				if( dat->CacheSize != 0 ) _MEMMOVE( &dat->CacheBuffer[0], &dat->CacheBuffer[MoveSize], dat->CacheSize ) ;
			}

			// 希望のデータが全て読めていない場合は更にファイルから読み込む
			if( dat->ReadSize != 0 )
			{
				SetFilePointer( dat->Handle, dat->ReadPosition, NULL, FILE_BEGIN ) ;
				ReadFile( dat->Handle, dat->ReadBuffer, dat->ReadSize, &dat->ReadSize, NULL ) ;

				// キャッシュを初期化する
				if( dat->CacheBuffer )
				{
					dat->CachePosition = dat->ReadPosition + dat->ReadSize ;
					dat->CacheSize = 0 ;
				}
			}
			break ;

		case FILEACCESSTHREAD_FUNCTION_SEEK :
			SetFilePointer( dat->Handle, dat->SeekPoint, NULL, FILE_BEGIN ) ;

			// キャッシュを初期化する
			if( dat->CacheBuffer )
			{
				dat->CachePosition = (DWORD)dat->SeekPoint ;
				dat->CacheSize = 0 ;
			}
			break ;

		case FILEACCESSTHREAD_FUNCTION_EXIT :
			if( dat->Handle != NULL ) CloseHandle( dat->Handle ) ;
			dat->Handle = NULL ;
			goto END ;
		}

		// 指令が完了したら完了イベントをシグナル状態にする
		SetEvent( dat->CompEvent ) ;
	}

END:
	// エラー時の為に完了イベントをシグナル状態にする
	SetEvent( dat->CompEvent ) ;
	dat->EndFlag = TRUE ;
	ExitThread( 1 ) ;

	return 0 ;
}
