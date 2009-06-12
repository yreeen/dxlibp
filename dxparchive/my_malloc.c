#include <stddef.h>
// メモリ確保系関数

// メモリ確保の情報
typedef struct tagALLOCMEM
{
	char					Name[16] ;							// ファイルパス
	unsigned short			ID ;								// ＩＤ
	unsigned short			Line ;								// 行番号
	unsigned int			Size ;								// 確保サイズ
	struct tagALLOCMEM		*Back, *Next ;						// 次と前の確保メモリ情報へのポインタ
} ALLOCMEM, *LPALLOCMEM ;

#define DXADDRESS	(64)

// メモリを確保する
extern void *ST_DxAlloc( size_t AllocSize, const char *File, int Line )
{
/*	ALLOCMEM *mem ;

	// メモリの確保
	mem = (ALLOCMEM *)AllocWrap( AllocSize + DXADDRESS ) ;
	if( mem == NULL )
	{
		// メモリが足りなかったらその時のメモリをダンプする
		ST_DxDumpAlloc() ;

		// エラー情報も出力する
		ST_DxErrorCheckAlloc() ;

		return NULL ;
	}
		
	// メモリ情報追加処理
	{
		// 情報の保存
		mem->Size = AllocSize ;
		if( File != NULL )
		{
			int len = lstrlen( File ) ;
			_STRCPY( mem->Name, &File[ len < 15 ? 0 : len - 15 ] ) ;
		}
		else
		{
			mem->Name[0] = '\0' ;
		}
		mem->Line = ( unsigned short )Line ;

		mem->ID = WinData.AllocMemoryID ;
		WinData.AllocMemoryID ++ ;

		// リストに追加
		if( WinData.AllocMemoryFirst == NULL )
			WinData.AllocMemoryFirst = &WinData.AllocMemoryAnchor ;
		WinData.AllocMemoryFirst->Next = mem ;
		mem->Back = WinData.AllocMemoryFirst ;
		WinData.AllocMemoryFirst = mem ;
		mem->Next = NULL ;

		// 複製を保存
		DxCopyAlloc( mem ) ;
		DxCopyAlloc( mem->Back ) ;

		// 確保したメモリの総量と総数を加算する
		WinData.AllocMemorySize += AllocSize ;
		WinData.AllocMemoryNum ++ ;
	}

	// 条件が揃っている場合はログを出力する
	if( (int)WinData.AllocTrapSize < 0 || WinData.AllocTrapSize == AllocSize || WinData.AllocMemoryPrintFlag == TRUE )
	{
		DXST_ERRORLOG_ADD( "mem alloc  " ) ;
		DxPrintAlloc( mem ) ;
	}

	// 確保しているメモリの総量を出力する
	if( WinData.AllocMemorySizeOutFlag == TRUE )
		DxPrintAllocSize() ;

	// メモリ破壊のチェック
	if( WinData.AllocMemoryErrorCheckFlag == TRUE )
		ST_DxErrorCheckAlloc() ;
	
	// メモリアドレスを返す
	return (char *)mem + DXADDRESS ;
	*/
	return NULL;
}

// メモリを確保して０で初期化する
extern void *ST_DxCalloc( size_t AllocSize, const char *File, int Line )
{
/*	void *buf ;

	// メモリの確保
	buf = ST_DxAlloc( AllocSize, File, Line ) ;
	if( buf == NULL ) return NULL ;
	
	// 確保したメモリを初期化
	_MEMSET( buf, 0, AllocSize ) ;

	// 確保したメモリのアドレスを返す
	return buf ;
	*/
	return NULL;
}

// メモリの再確保を行う
extern void *ST_DxRealloc( void *Memory, size_t AllocSize, const char *File, int Line )
{
/*	ALLOCMEM *mem, *back ;
	void *Result ;

	// メモリの再確保
	back = (ALLOCMEM *)( (char *)Memory - DXADDRESS ) ;
	if( Memory == NULL )
	{
		Result = ST_DxAlloc( AllocSize + DXADDRESS, File, Line ) ;

		return Result ;
	}
	else
	{
		mem = (ALLOCMEM *)ReallocWrap( (char *)Memory - DXADDRESS, AllocSize + DXADDRESS ) ;
	}
	if( mem == NULL )
	{
		// メモリが足りなかったらその時のメモリをダンプする
		ST_DxDumpAlloc() ;

		return NULL ;
	}

	// 情報の修正
	{
		// 情報の保存
		if( File != NULL )
		{
			int len = lstrlen( File ) ;
			_STRCPY( mem->Name, &File[ len < 15 ? 0 : len - 15 ] ) ;
		}
		else
		{
			mem->Name[0] = '\0' ;
		}
		mem->Line = Line ;
		mem->ID = WinData.AllocMemoryID ;
		WinData.AllocMemoryID ++ ;

		// 前後の情報の更新
		mem->Back->Next = mem ;
		if( mem->Next != NULL ) mem->Next->Back = mem ;
		if( Memory != NULL && back == WinData.AllocMemoryFirst ) WinData.AllocMemoryFirst = mem ;

		// 確保したメモリの総量の修正
		WinData.AllocMemorySize -= mem->Size ;
		mem->Size = AllocSize ;
		WinData.AllocMemorySize += AllocSize ;

		// 複製を保存
		DxCopyAlloc( mem ) ;
		DxCopyAlloc( mem->Back ) ;
		if( mem->Next != NULL ) DxCopyAlloc( mem->Next ) ;
	}

	// 条件が揃っている場合はログを出力する
	if( (int)WinData.AllocTrapSize < 0 || WinData.AllocTrapSize == AllocSize || WinData.AllocMemoryPrintFlag == TRUE )
	{
		DXST_ERRORLOG_ADD( "mem realloc  " ) ;
		DxPrintAlloc( mem ) ;
	}

	// 確保しているメモリの総量を出力する
	if( WinData.AllocMemorySizeOutFlag == TRUE )
		DxPrintAllocSize() ;

	// メモリ破壊のチェック
	if( WinData.AllocMemoryErrorCheckFlag == TRUE )
		ST_DxErrorCheckAlloc() ;

	// 確保したメモリアドレスを返す
	return (char *)mem + DXADDRESS ;
	*/
	return NULL;
}

// メモリを解放する
extern void ST_DxFree( void *Memory )
{
/*	ALLOCMEM *mem ;

	// NULL が渡された場合は何もしない
	if( Memory == NULL ) return ;

	mem = (ALLOCMEM *)( (char *)Memory - DXADDRESS ) ;

	// 解放するメモリの分だけ確保したメモリの総量と数を減らす
	WinData.AllocMemorySize -= mem->Size ;
	WinData.AllocMemoryNum -- ;

	// メモリ破壊のチェック
	if( WinData.AllocMemoryErrorCheckFlag == TRUE )
		ST_DxErrorCheckAlloc() ;

	// 大域データアクセス
	{
		// リストから外す
		if( mem->Back != NULL )	mem->Back->Next = mem->Next ;
		if( mem->Next != NULL ) mem->Next->Back = mem->Back ;
		if( mem == WinData.AllocMemoryFirst ) WinData.AllocMemoryFirst = mem->Back ;

		// 複製を保存
		if( mem->Next != NULL )	DxCopyAlloc( mem->Next ) ;
		if( mem->Back != NULL ) DxCopyAlloc( mem->Back ) ;
	}
	
	// 条件が揃っている場合はログを出力する
	if( (int)WinData.AllocTrapSize < 0 || WinData.AllocTrapSize == mem->Size || WinData.AllocMemoryPrintFlag == TRUE )
	{
		DXST_ERRORLOG_ADD( "mem free  " ) ;
		DxPrintAlloc( mem ) ;
	}

	// 確保しているメモリの総量を出力する
	if( WinData.AllocMemorySizeOutFlag == TRUE )
		DxPrintAllocSize() ;
	
	// メモリの解放
	FreeWrap( mem ) ;
	*/
}

// 列挙対象にするメモリの確保容量をセットする
extern size_t ST_DxSetAllocSizeTrap( size_t Size )
{
	/*size_t trapsize ;

	trapsize = WinData.AllocTrapSize ;
	WinData.AllocTrapSize = Size ;
		
	return trapsize ;
	*/
	return 0;
}

