
// ＤＸアーカイブ関連

/*
	データマップ
		
	DXARC_HEAD
	ファイル実データ
	ファイル名テーブル
	DXARC_FILEHEAD テーブル
	DXARC_DIRECTORY テーブル


	ファイル名のデータ形式
	2byte:文字列の長さ(バイトサイズ÷４)
	2byte:文字列のパリティデータ(全ての文字の値を足したもの)
	英字は大文字に変換されたファイル名のデータ(４の倍数のサイズ)
	英字が大文字に変換されていないファイル名のデータ
*/

//とりあえずvc++互換
//と思ったけどアンダースコア付きでもだめなのでとりあえず定数指定
//#define MAX_PATH					_MAX_PATH
#define MAX_PATH					260
#define DXA_FILE_NO_SET				0

#define bool						int
#define false						FALSE
#define true						TRUE			
#define FILE_ATTRIBUTE_DIRECTORY	0x01	//数値は後で解析するのでとりあえずダミー

#define FILEACCESSTHREAD_DEFAULT_CACHESIZE	(128 * 1024)	// デフォルトのキャッシュサイズ

#define DXAHEAD						*((WORD *)"DX")		// ヘッダ
#define DXAVER						(0x0003)			// バージョン
#define DXA_DIR_MAXARCHIVENUM		(512)				// 同時に開いておけるアーカイブファイルの数
#define DXA_DIR_MAXFILENUM			(512)				// 同時に開いておけるファイルの数
#define DXA_KEYSTR_LENGTH			(12)				// 鍵文字列の長さ
#define DXA_MAXDRIVENUM				(64)				// 対応するドライブの最大数
// ＤＸアーカイブ関連(DxArchive.cpp)
#define ULONGLONG u64
// アーカイブデータの最初のヘッダ
typedef struct tagDXARC_HEAD
{
	WORD						Head ;							// ＩＤ
	WORD						Version ;						// バージョン
	DWORD						HeadSize ;						// ヘッダ情報の DXARC_HEAD を抜いた全サイズ
	DWORD						DataStartAddress ;				// 最初のファイルのデータが格納されているデータアドレス(ファイルの先頭アドレスをアドレス０とする)
	DWORD						FileNameTableStartAddress ;		// ファイル名テーブルの先頭アドレス(ファイルの先頭アドレスをアドレス０とする)
	DWORD						FileTableStartAddress ;			// ファイルテーブルの先頭アドレス(メンバ変数 FileNameTableStartAddress のアドレスを０とする)
	DWORD						DirectoryTableStartAddress ;	// ディレクトリテーブルの先頭アドレス(メンバ変数 FileNameTableStartAddress のアドレスを０とする)
																// アドレス０から配置されている DXARC_DIRECTORY 構造体がルートディレクトリ
} DXARC_HEAD ;

// ファイルの時間情報
typedef struct tagDXARC_FILETIME
{
	ULONGLONG					Create ;						// 作成時間
	ULONGLONG					LastAccess ;					// 最終アクセス時間
	ULONGLONG					LastWrite ;						// 最終更新時間
} DXARC_FILETIME ;

// ファイル名データ構造体
typedef struct tagDXARC_FILENAME
{
	WORD						Length ;						// 文字列の長さ÷４
	WORD						Parity ;						// パリティ情報
} DXARC_FILENAME ;

// ファイル格納情報(Ver0x0001 用)
typedef struct tagDXARC_FILEHEAD_VER1
{
	DWORD						NameAddress ;					// ファイル名が格納されているアドレス( ARCHIVE_HEAD構造体 のメンバ変数 FileNameTableStartAddress のアドレスをアドレス０とする) 
	DWORD						Attributes ;					// ファイル属性
	DXARC_FILETIME				Time ;							// 時間情報
	DWORD						DataAddress ;					// ファイルが格納されているアドレス
																//			ファイルの場合：DXARC_HEAD構造体 のメンバ変数 DataStartAddress が示すアドレスをアドレス０とする
																//			ディレクトリの場合：DXARC_HEAD構造体 のメンバ変数 DirectoryTableStartAddress のが示すアドレスをアドレス０とする
	DWORD						DataSize ;						// ファイルのデータサイズ
} DXARC_FILEHEAD_VER1 ;

// ファイル格納情報
typedef struct tagDXARC_FILEHEAD
{
	DWORD						NameAddress ;					// ファイル名が格納されているアドレス( ARCHIVE_HEAD構造体 のメンバ変数 FileNameTableStartAddress のアドレスをアドレス０とする) 
	DWORD						Attributes ;					// ファイル属性
	DXARC_FILETIME				Time ;							// 時間情報
	DWORD						DataAddress ;					// ファイルが格納されているアドレス
																//			ファイルの場合：DXARC_HEAD構造体 のメンバ変数 DataStartAddress が示すアドレスをアドレス０とする
																//			ディレクトリの場合：DXARC_HEAD構造体 のメンバ変数 DirectoryTableStartAddress のが示すアドレスをアドレス０とする
	DWORD						DataSize ;						// ファイルのデータサイズ
	DWORD						PressDataSize ;					// 圧縮後のデータのサイズ( 0xffffffff:圧縮されていない ) ( Ver0x0002 で追加された )
} DXARC_FILEHEAD ;

// ディレクトリ格納情報
typedef struct tagDXARC_DIRECTORY
{
	DWORD						DirectoryAddress ;				// 自分の DXARC_FILEHEAD が格納されているアドレス( DXARC_HEAD 構造体 のメンバ変数 FileTableStartAddress が示すアドレスをアドレス０とする)
	DWORD						ParentDirectoryAddress ;		// 親ディレクトリの DXARC_DIRECTORY が格納されているアドレス( DXARC_HEAD構造体 のメンバ変数 DirectoryTableStartAddress が示すアドレスをアドレス０とする)
	DWORD						FileHeadNum ;					// ディレクトリ内のファイルの数
	DWORD						FileHeadAddress ;				// ディレクトリ内のファイルのヘッダ列が格納されているアドレス( DXARC_HEAD構造体 のメンバ変数 FileTableStartAddress が示すアドレスをアドレス０とする) 
} DXARC_DIRECTORY ;



// ファイル名検索用データ構造体
typedef struct tagDXARC_SEARCHDATA
{
	BYTE						FileName[1024] ;				// ファイル名
	WORD						Parity ;						// パリティ情報
	WORD						PackNum ;						// 文字列の長さ÷４
} DXARC_SEARCHDATA ;

// 情報テーブル構造体
typedef struct tagDXARC_TABLE
{
	BYTE						*Top ;							// 情報テーブルの先頭ポインタ
	BYTE						*FileTable ;					// ファイル情報テーブルへのポインタ
	BYTE						*DirectoryTable ;				// ディレクトリ情報テーブルへのポインタ
	BYTE						*NameTable ;					// 名前情報テーブルへのポインタ
} DXARC_TABLE ;

// アーカイブ処理用情報構造体
typedef struct tagDXARC
{
	DXARC_HEAD					Head ;							// アーカイブのヘッダ
	DWORD						FilePointer ;					// アーカイブファイルのポインタ	
	DXARC_TABLE					Table ;							// 各テーブルへの先頭アドレスが格納された構造体
	DXARC_DIRECTORY				*CurrentDirectory ;				// カレントディレクトリデータへのポインタ

	unsigned char				Key[DXA_KEYSTR_LENGTH] ;		// 鍵文字列
	int							MemoryOpenFlag ;				// メモリ上のファイルを開いているか、フラグ
	int							UserMemoryImageFlag ;			// ユーザーが展開したメモリイメージを使用しているか、フラグ
	int							MemoryImageSize ;				// メモリ上のファイルから開いていた場合のイメージのサイズ

	int							ASyncOpenFlag ;					// 非同期読み込み中かフラグ( TRUE:非同期読み込み中 FALSE:違う )
	DWORD						ASyncOpenFilePointer ;			// 非同期オープン処理に使用するファイルのポインタ
} DXARC ;

// アーカイブ内のファイルを通常のファイル読み込みのように処理する為の構造体
typedef struct tagDXARC_STREAM
{
	DXARC						*Archive ;						// アーカイブデータへのポインタ
	DXARC_FILEHEAD				*FileHead ;						// ファイル情報へのポインタ
	void						*DecodeDataBuffer ;				// 解凍したデータが格納されているメモリ領域へのポインタ( ファイルが圧縮データだった場合のみ有効 )
	void						*DecodeTempBuffer ;				// 圧縮データ一時保存用メモリ領域へのポインタ

	int							EOFFlag ;						// EOFフラグ
	DWORD						FilePoint ;						// ファイルポインタ
	int							UseASyncReadFlag ;				// 非同期読み込みフラグ
	int							ASyncState ;					// 非同期読み込み状態( DXARC_STREA_ASYNCSTATE 系 )
	int							ASyncReadFileAddress ;			// 非同期読み込み時のファイルポインタ

	void						*ReadBuffer;					// 非同期読み込み時の引数に渡されたバッファへのポインタ
	int							ReadSize;						// 非同期読み込み時の引数に渡された読み込みサイズへのポインタ
} DXARC_STREAM ;

// アーカイブファイルをディレクトリに見立てる処理用の開いているアーカイブファイルの情報
typedef struct tagDXARC_DIR_ARCHIVE
{
	int							UseCounter ;					// このアーカイブファイルが使用されている数
	DXARC						Archive ;						// アーカイブファイルデータ
	char						Path[256] ;						// アーカイブファイルのパス
} DXARC_DIR_ARCHIVE ;

// アーカイブファイルをディレクトリに見立てる処理用の開いているアーカイブファイル中のファイルの情報
typedef struct tagDXARC_DIR_FILE
{
	int							UseArchiveFlag ;				// アーカイブファイルを使用しているかフラグ
	DWORD						FilePointer ;					// アーカイブを使用していない場合の、ファイルポインタ
	DWORD						UseArchiveIndex ;				// アーカイブを使用している場合、使用しているアーカイブファイルデータのインデックス
	DXARC_STREAM				DXAStream ;						// アーカイブファイルを使用している場合のファイルアクセス用データ
} DXARC_DIR_FILE ;

// アーカイブをディレクトリに見立てる処理用の構造体
typedef struct tagDXARC_DIR
{
	DXARC_DIR_ARCHIVE			*Archive[DXA_DIR_MAXARCHIVENUM] ;	// 使用しているアーカイブファイルのデータ
	DXARC_DIR_FILE				*File[DXA_DIR_MAXFILENUM];			// 開いているファイルのデータ
	char						ArcExt[64];							// アーカイブファイルの拡張子
	int							ArcExtLen;							// アーカイブファイルの拡張子の長さ

	int							ValidKeyString ;					// KeyString が有効かどうか
	char						KeyString[DXA_KEYSTR_LENGTH + 1 ] ;	// 鍵文字列

	int							ArchiveNum ;					// 使用しているアーカイブファイルの数
	int							FileNum ;						// 開いているファイルの数

	int							BUArcIndex;						// 前回使用したアーカイブのインデックス
	char						BUDir[256];						// 前回使用したディレクトリパス
	int							BUDirPathLen;					// 前回使用したディレクトリパスの長さ
} DXARC_DIR ;

// タイムデータ型
typedef struct tagDATEDATA
{
	int						Year ;							// 年
	int						Mon ;							// 月
	int						Day ;							// 日
	int						Hour ;							// 時間
	int						Min ;							// 分
	int						Sec ;							// 秒
} DATEDATA, *LPDATEDATA ;

// ファイル情報構造体
typedef struct tagFILEINFO
{
	char					Name[260] ;			// オブジェクト名
	int						DirFlag ;			// ディレクトリかどうか( TRUE:ディレクトリ  FALSE:ファイル )
	LONGLONG				Size ;				// サイズ
	DATEDATA				CreationTime ;		// 作成時刻
	DATEDATA				LastWriteTime ;		// 最終更新時刻
} FILEINFO, *LPFILEINFO ;

typedef struct _FILETIME {
	DWORD dwLowDateTime;
	DWORD dwHighDateTime;
} FILETIME; 


// ＤＸアーカイブ関連(DxArchive.cpp)
//DXPDEFARG
extern	int	DXA_Initialize( DXARC *DXA ) ;												// アーカイブファイルを扱う為の構造体を初期化する
extern	int DXA_Terminate( DXARC *DXA ) ;												// アーカイブファイルを扱う為の構造体の後始末をする
//extern	int	DXA_OpenArchiveFromFile( DXARC *DXA, const char *ArchivePath, const char *KeyString = NULL ) ;
extern	int	DXA_OpenArchiveFromFile( DXARC *DXA, const char *ArchivePath, const char *KeyString DXPDEFARG(NULL) ) ;
// アーカイブファイルを開く( 0:成功  -1:失敗 )
//extern	int	DXA_OpenArchiveFromFileUseMem( DXARC *DXA, const char *ArchivePath, const char *KeyString = NULL , int ASync = FALSE ) ;
extern	int	DXA_OpenArchiveFromFileUseMem( DXARC *DXA, const char *ArchivePath, const char *KeyString DXPDEFARG(NULL) , int ASync DXPDEFARG(FALSE) ) ;
// アーカイブファイルを開き最初にすべてメモリ上に読み込んでから処理する( 0:成功  -1:失敗 )
//extern	int	DXA_OpenArchiveFromMem( DXARC *DXA, void *ArchiveImage, int ArchiveSize, const char *KeyString = NULL ) ;
extern	int	DXA_OpenArchiveFromMem( DXARC *DXA, void *ArchiveImage, int ArchiveSize, const char *KeyString DXPDEFARG(NULL) ) ;
// メモリ上にあるアーカイブファイルイメージを開く( 0:成功  -1:失敗 )
extern	int DXA_CheckIdle( DXARC *DXA ) ;												// アーカイブファイルを扱う準備が整ったかを得る( TRUE:整っている  FALSE:整っていない )
extern	int	DXA_CloseArchive( DXARC *DXA ) ;											// アーカイブファイルを閉じる

extern	int	DXA_LoadFile( DXARC *DXA, const char *FilePath, void *Buffer, unsigned int BufferSize ) ;	// アーカイブファイル中の指定のファイルをメモリに読み込む( -1:エラー 0以上:ファイルサイズ )
extern	void *DXA_GetFileImage( DXARC *DXA ) ;											// アーカイブファイルをメモリに読み込んだ場合のファイルイメージが格納されている先頭アドレスを取得する( DXA_OpenArchiveFromFileUseMem 若しくは DXA_OpenArchiveFromMem で開いた場合に有効、データが圧縮されている場合は注意 )
//extern	int	DXA_GetFileInfo( DXARC *DXA, const char *FilePath, int *Position, int *Size ) ;	// アーカイブファイル中の指定のファイルのファイル内の位置とファイルの大きさを得る( -1:エラー )
extern	int	DXA_ChangeCurrentDir( DXARC *DXA, const char *DirPath ) ;					// アーカイブ内のカレントディレクトリを変更する( 0:成功  -1:失敗 )
extern	int DXA_GetCurrentDir( DXARC *DXA, char *DirPathBuffer, int BufferSize ) ;		// アーカイブ内のカレントディレクトリを取得する
extern	int DXA_FindFirst( DXARC *DXA, const char *FilePath, FILEINFO *Buffer ) ;		// アーカイブ内のオブジェクトを検索する( -1:エラー -1以外:DXA検索ハンドル )
extern	int DXA_FindNext( int DxaFindHandle, FILEINFO *Buffer ) ;						// アーカイブ内のオブジェクトを検索する( -1:エラー 0:成功 )
extern	int DXA_FindClose( int DxaFindHandle ) ;											// アーカイブ内のオブジェクト検索を終了する

extern	int DXA_STREAM_Initialize( DXARC_STREAM *DXAStream, DXARC *DXA, const char *FilePath/*, int UseASyncReadFlag*/ ) ;	// アーカイブファイル内のファイルを開く
extern	int DXA_STREAM_Terminate( DXARC_STREAM *DXAStream ) ;							// アーカイブファイル内のファイルを閉じる
extern	int DXA_STREAM_Read( DXARC_STREAM *DXAStream, void *Buffer, int ReadLength ) ;	// ファイルの内容を読み込む
extern	int DXA_STREAM_Seek( DXARC_STREAM *DXAStream, int SeekPoint, int SeekMode ) ;	// ファイルポインタを変更する
extern	int DXA_STREAM_Tell( DXARC_STREAM *DXAStream ) ;								// 現在のファイルポインタを得る
extern	int DXA_STREAM_Eof( DXARC_STREAM *DXAStream ) ;									// ファイルの終端に来ているか、のフラグを得る
extern	int	DXA_STREAM_IdleCheck( DXARC_STREAM *DXAStream ) ;							// 読み込み処理が完了しているかどうかを調べる
extern	int DXA_STREAM_Size( DXARC_STREAM *DXAStream ) ;								// ファイルのサイズを取得する


extern	int DXA_DIR_Initialize( void ) ;												// アーカイブをディレクトリに見立てる処理の初期化
extern	int DXA_DIR_Terminate( void ) ;													// アーカイブをディレクトリに見立てる処理の後始末

extern void *ST_DxAlloc( size_t AllocSize, const char *File, int Line );
extern void *ST_DxCalloc( size_t AllocSize, const char *File, int Line );
extern void *ST_DxRealloc( void *Memory, size_t AllocSize, const char *File, int Line );
extern void ST_DxFree( void *Memory );

// メモリ確保系関数
//#ifdef DX_USE_DXLIB_MEM_DUMP
#ifdef _DEBUG
	#define DXALLOC( size )			ST_DxAlloc( (size), __FILE__, __LINE__ )
	#define DXCALLOC( size )		ST_DxCalloc( (size), __FILE__, __LINE__ )
	#define DXREALLOC( mem, size )	ST_DxRealloc( (mem), (size), __FILE__, __LINE__ )
	#define DXFREE( mem )			ST_DxFree( (mem) )
#else
	#define DXALLOC( size )			ST_DxAlloc( (size), "", 0 )
	#define DXCALLOC( size )		ST_DxCalloc( (size), "", 0 )
	#define DXREALLOC( mem, size )	ST_DxRealloc( (mem), (size), "", 0 )
	#define DXFREE( mem )			ST_DxFree( (mem) )
#endif

//#ifdef	__cplusplus
//#define	DXPDEFARG(ARG)	=(ARG)		/*C++のデフォルト引数の機能はCではエラーになるので対策*/
//extern "C" {
//#else
//#define	DXPDEFARG(ARG)
//#endif

//static void DXA_KeyConvFileRead( void *Data, int Size, DWORD FilePointer, unsigned char *Key, int Position = -1 ) ;
//static void DXA_KeyConvFileRead( void *Data, int Size, DWORD FilePointer, unsigned char *Key, int Position DXPDEFARG(-1) ) ;
//void DXA_KeyConvFileRead( void *Data, int Size, DWORD FilePointer, unsigned char *Key, int Position DXPDEFARG(-1) ) ;

//#ifdef	__cplusplus
//};
//#endif
