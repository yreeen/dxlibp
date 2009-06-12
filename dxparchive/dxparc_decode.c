#include "dxlibp.h"
#include "dxparchive.h"
#include <stdio.h>
#include <string.h>

// データを解凍する( 戻り値:解凍後のデータサイズ )
#define MIN_COMPRESS		(4)						// 最低圧縮バイト数
#define MAX_SEARCHLISTNUM	(64)					// 最大一致長を探す為のリストを辿る最大数
#define MAX_SUBLISTNUM		(65536)					// 圧縮時間短縮のためのサブリストの最大数
#define MAX_COPYSIZE 		(0x1fff + MIN_COMPRESS)	// 参照アドレスからコピー出切る最大サイズ( 圧縮コードが表現できるコピーサイズの最大値 + 最低圧縮バイト数 )
#define MAX_ADDRESSLISTNUM	(1024 * 1024 * 1)		// スライド辞書の最大サイズ
#define MAX_POSITION		(1 << 24)				// 参照可能な最大相対アドレス( 16MB )

/**
 *
 *
 */
int		DXA_Decode( void *Src, void *Dest )
{
	DWORD srcsize, destsize, code, indexsize, keycode, conbo, index ;
	BYTE *srcp, *destp, *dp, *sp ;

	destp = (BYTE *)Dest ;
	srcp  = (BYTE *)Src ;
	// 解凍後のデータサイズを得る
	destsize = *((DWORD *)&srcp[0]) ;
	// 圧縮データのサイズを得る
	srcsize = *((DWORD *)&srcp[4]) - 9 ;
	// キーコード
	keycode = srcp[8] ;
	// 出力先がない場合はサイズだけ返す
	if( Dest == NULL )	return destsize ;
	// 展開開始
	sp  = srcp + 9 ;
	dp  = destp ;
	while( srcsize )
	{
		// キーコードか同かで処理を分岐
		if( sp[0] != keycode )
		{
			// 非圧縮コードの場合はそのまま出力
			*dp = *sp ;
			dp      ++ ;
			sp      ++ ;
			srcsize -- ;
			continue ;
		}
		// キーコードが連続していた場合はキーコード自体を出力
		if( sp[1] == keycode )
		{
			*dp = (BYTE)keycode ;
			dp      ++ ;
			sp      += 2 ;
			srcsize -= 2 ;
			
			continue ;
		}
		// 第一バイトを得る
		code = sp[1] ;
		// もしキーコードよりも大きな値だった場合はキーコード
		// とのバッティング防止の為に＋１しているので−１する
		if( code > keycode ) code -- ;
		sp      += 2 ;
		srcsize -= 2 ;
		// 連続長を取得する
		conbo = code >> 3 ;
		if( code & ( 0x1 << 2 ) )
		{
			conbo |= *sp << 5 ;
			sp      ++ ;
			srcsize -- ;
		}
		conbo += MIN_COMPRESS ;	// 保存時に減算した最小圧縮バイト数を足す
		// 参照相対アドレスを取得する
		index = 0;
		indexsize = code & 0x3 ;
		switch( indexsize )
		{
		case 0 :
			index = *sp ;
			sp      ++ ;
			srcsize -- ;
			break ;
			
		case 1 :
			index = *((WORD *)sp) ;
			sp      += 2 ;
			srcsize -= 2 ;
			break ;
			
		case 2 :
			index = *((WORD *)sp) | ( sp[2] << 16 ) ;
			sp      += 3 ;
			srcsize -= 3 ;
			break ;
		}
		index ++ ;		// 保存時に−１しているので＋１する
		// 展開
		if( index < conbo )
		{
			DWORD num ;
			num  = index ;
			while( conbo > num )
			{
				memcpy( dp, dp - num, num ) ;
				dp    += num ;
				conbo -= num ;
				num   += num ;
			}
			if( conbo != 0 )
			{
				memcpy( dp, dp - num, conbo ) ;
				dp += conbo ;
			}
		}
		else
		{
			memcpy( dp, dp - index, conbo ) ;
			dp += conbo ;
		}
	}
	// 解凍後のサイズを返す
	return (int)destsize ;
}
