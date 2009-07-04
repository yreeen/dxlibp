#include <png.h>
#include <malloc.h>
#include <pspgu.h>
#include "dxlibp.h"
#include "dxpstatic.h"

#ifndef DX_NON_PNGREAD

// 汎用データ読み込み処理からの読み込みをするためのデータ型
typedef struct tagPNGGENERAL
{
	STREAMDATA *Data ;
	unsigned int DataSize ;
} PNGGENERAL ;

// 汎用データ読み込み処理からデータを読み込むコールバック関数
static void png_general_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	PNGGENERAL *PGen ;

	PGen = (PNGGENERAL *)CVT_PTR(png_ptr->io_ptr) ;

	// 残りのサイズが足りなかったらエラー
	if( PGen->DataSize - STTELL( PGen->Data ) < length )
	{
		png_error(png_ptr, "Read Error");
	}
	else
	{
		STREAD( data, length, 1, PGen->Data ) ;
	}
}

// 汎用データ読み込み処理からの読み込みを設定する関数
int png_general_read_set( png_structp png_ptr, PNGGENERAL *PGen, STREAMDATA *Data )
{
	PGen->Data = Data ;

	// 現在のファイルポインタから終端までのサイズを取得する
	{
		long pos ;
		pos = STTELL( PGen->Data ) ;
		STSEEK( PGen->Data, 0, STREAM_SEEKTYPE_END ) ;
		PGen->DataSize = STTELL( PGen->Data ) - pos ;
		STSEEK( PGen->Data, pos, STREAM_SEEKTYPE_SET ) ;
	}

	// コールバック関数のセット
	png_set_read_fn( png_ptr, PGen, png_general_read_data ) ;

	// 終了
	return 0 ;
}

// ＰＮＧ画像の読みこみ
DXPTEXTURE2* LoadPngImage(const char *FileName)
{
//	BASEIMAGE *Image ;
	STREAMDATA src;
	STREAMDATA *Src = &src;
	if(SetupSTREAMDATA(FileName,Src) == -1)
	{
		STCLOSE(Src);
		return NULL;
	}
	png_bytep *row_pointers;
	unsigned int row, rowbytes ;
	u8 Expand ;
	PNGGENERAL PGen ;
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int sig_read = 0;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;
	int i ;
	png_bytep BufPoint ;

	Expand = 0;

	// ＰＮＧ管理情報の作成
	if( ( png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL ) ) == NULL )
	{
		STCLOSE(Src);
		return NULL ;
	}
	// 画像情報構造体の作成
	if( ( info_ptr = png_create_info_struct( png_ptr ) ) == NULL ) 
	{
		STCLOSE(Src);
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		return NULL ;
	}

	// エラー時の処理セットアップおよび処理部
	if( setjmp( png_jmpbuf( png_ptr ) ) )
	{
		STCLOSE(Src);
		png_destroy_read_struct( &png_ptr, &info_ptr, (png_infopp)NULL );
		return NULL ;
	}

	// 汎用データ読み込み処理から読み込む場合の設定を行う
	png_general_read_set( png_ptr, &PGen, Src ) ;

	// 設定処理郡
	png_set_sig_bytes(		png_ptr, sig_read ) ;												// よくわから無い処理(汗)
	png_read_info(			png_ptr, info_ptr );												// 画像情報を得る
	png_get_IHDR(			png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,		// 画像の基本ステータスを取得する
							&interlace_type, NULL, NULL ) ;
	png_set_strip_16(		png_ptr ) ;															// １６ビットの画像でも８ビットで収納するように設定
//	if( BmpFlag == TRUE )										png_set_strip_alpha( png_ptr ) ;// アルファチャンネル無視
	png_set_packing(		png_ptr ) ;															// １バイト以下のパレット画像をバイト単位で展開するよう指定
	png_set_packswap(		png_ptr ) ;															// よくわから無い処理

//	if( color_type == PNG_COLOR_TYPE_PALETTE )					png_set_expand( png_ptr ) ;		// パレット使用画像データの自動展開指定

	// tRNS(一種のカラーキー)付き画像か８ビット以下のグレースケール画像の場合は
	// 出力画像のピクセルフォーマットをフルカラーにする
	//ただし、パレット画像でtRNSの場合はパレット情報とtRNS情報を合成する。
	if( ( color_type == PNG_COLOR_TYPE_GRAY && bit_depth <= 8 ) ||
		(color_type != PNG_COLOR_TYPE_PALETTE && png_get_valid( png_ptr, info_ptr, PNG_INFO_tRNS ) ))
	{
		png_set_expand( png_ptr );		
		Expand = 1 ;
	}
	png_set_gray_to_rgb(png_ptr);
	png_set_add_alpha(png_ptr,0xff,PNG_FILLER_AFTER);			/*アルファ情報が無い場合は付加*/
//	png_set_bgr(			png_ptr ) ;															// カラー配列をＲＧＢからＢＧＲに反転
	
	// 変更した設定を反映させる
	png_read_update_info(	png_ptr, info_ptr ) ;

	// １ラインあたりに必要なデータ量を得る
	rowbytes = png_get_rowbytes( png_ptr, info_ptr ) ;

	// グラフィックデータを格納するメモリ領域を作成する
	{
		png_bytep BufP ;

		row_pointers = ( png_bytep * )malloc( height * sizeof( png_bytep * ) ) ;
		if( ( BufPoint = ( png_bytep )png_malloc( png_ptr, rowbytes * height ) ) == NULL )	return NULL ;
		BufP = BufPoint ;
		for (row = 0; row < height; row++, BufP += rowbytes )
			row_pointers[row] = BufP ;
	}

	// 画像データの読み込み
	png_read_image( png_ptr, row_pointers );


	DXPTEXTURE2 *texptr;
	// 情報をセットする
	{
		// カラー情報をセットする
		if( color_type == PNG_COLOR_TYPE_PALETTE && Expand == 0)
		{
			//パレットカラーである
			if(bit_depth == 8)
texptr = MakeTexture(width,height,GU_PSM_T8);
			else
texptr = MakeTexture(width,height,GU_PSM_T4);
			if(texptr == NULL)
			{
				//失敗処理
				free( row_pointers ) ;
				png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
				STCLOSE(Src);
				return NULL;
			}

			//画像を実際に使えるようにする
			int bn = 8 / bit_depth;
			for(i = 0;i < texptr->vmax;++i)
				memcpy(((u8*)texptr->pmemory) + texptr->pitch * i / bn,row_pointers[i],texptr->umax / bn);
			png_colorp SrcPalette ;
			int PaletteNum ;
			// パレットを取得
			png_get_PLTE( png_ptr, info_ptr, &SrcPalette, &PaletteNum ) ;

			// パレットの数が２５６以上だった場合は２５６に補正
			if( PaletteNum < 256 ) PaletteNum = 256 ;

			for(i = 0;i < PaletteNum;++i)
			{
				texptr->ppalette->data[i] = 0xff000000 |
					(((u32)SrcPalette[i].blue & 0x000000ff) << 16) |
					(((u32)SrcPalette[i].green & 0x000000ff) << 8) |
					(((u32)SrcPalette[i].red & 0x000000ff) << 0);
			}

			//tRNS情報取得
			if(png_get_valid(png_ptr,info_ptr,PNG_INFO_tRNS))
			{
				png_bytep trans;
				int num_trans;
				png_color_16p trans_values;
				png_get_tRNS(png_ptr,info_ptr,&trans,&num_trans,&trans_values);
				if(num_trans > 256)num_trans = 256;
				for(i = 0;i < num_trans;++i)
				{
					texptr->ppalette->data[i] &= (((u32)trans[i]) << 24) | 0x00ffffff;
				}
			}
		}
		else
		if( color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA )
		{
			texptr = MakeTexture(width,height,GU_PSM_8888);
			if(texptr == NULL)
			{
				//失敗処理
				free( row_pointers ) ;
				png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
				STCLOSE(Src);
				return NULL;
			}
			//画像を実際に使えるようにする
			int i;
			int bn = 8 / bit_depth;
			for(i = 0;i < texptr->vmax;++i)
				memcpy(((u8*)texptr->pmemory) + texptr->pitch * 4 * i / bn,row_pointers[i],texptr->umax  * 4 / bn);
		}
		else
		{
			//フルカラーである
			texptr = MakeTexture(width,height,GU_PSM_8888);
			if(texptr == NULL)
			{
				//失敗処理
				free( row_pointers ) ;
				png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
				STCLOSE(Src);
				return NULL;
			}
			//画像を実際に使えるようにする
			int i;
			int bn = 8 / bit_depth;
			for(i = 0;i < texptr->vmax;++i)
				memcpy(((u8*)texptr->pmemory) + texptr->pitch * 4 * i / bn,row_pointers[i],texptr->umax * 4 / bn);
		}
	}

	// メモリの解放
	png_free( png_ptr, BufPoint ) ;
	free( row_pointers ) ;

	// 読み込み処理の終了
	png_read_end(png_ptr, info_ptr);

	// 読み込み処理用構造体の破棄
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

	STCLOSE(Src);
	// 終了
	sceKernelDcacheWritebackAll();//全部書き込んじゃえ
	texptr->alphabit = 1;
	return texptr;
}
#else
int LoadPngImage(const char *FileName)
{
	return -1;
}

#endif // DX_NON_PNGREAD
