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