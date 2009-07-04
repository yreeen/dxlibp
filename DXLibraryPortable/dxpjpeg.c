#include "dxlibp.h"
#include "dxpstatic.h"
#include <pspjpeg.h>
#include <malloc.h>
#include <string.h>

//#define	DXP_NOUSE_LIBJPEG
#ifndef DXP_NOUSE_LIBJPEG

// (殆ど jdatasrc.c の流用)
//#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"

// ＪＰＥＧ読みこみエラー時処理ルーチン用構造体
typedef struct my_error_mgr {
	struct jpeg_error_mgr pub;	// 標準エラーデータ

	jmp_buf setjmp_buffer;		// ロングジャンプ用構造体
} *my_error_ptr ;

// エラー時に呼ばれる関数
void my_error_exit( j_common_ptr cinfo )
{
	// cinfo->errが示す標準エラーデータの先頭アドレスをmy_error_mgr構造体の先頭アドレスに変換
	my_error_ptr myerr = (my_error_ptr) cinfo->err;
	// すいませんよくわかりません、とりあえずエラーメッセージ標準関数？
	(*cinfo->err->output_message) (cinfo);
	// ユーザーが決めたエラー処理コードの位置まで飛ぶ
	longjmp( myerr->setjmp_buffer, 1 ) ;
}


// 汎用データ形式の転送用構造体
typedef struct
{
	struct jpeg_source_mgr pub;		/* public fields */

	JOCTET *buffer;					/* start of buffer */
	boolean start_of_file;			/* have we gotten any data yet? */

	STREAMDATA *Data ;				// 汎用データ形式読み込み処理用構造体
	int DataSize ;					// データのサイズ
} my_source_general_mgr;

typedef my_source_general_mgr	*my_src_general_ptr;

#define INPUT_BUF_SIZE		(4096)	// 作業用バッファのサイズ

// 読み込むソースを初期化する関数
METHODDEF(void)
init_source_general( j_decompress_ptr cinfo )
{
	my_src_general_ptr src = (my_src_general_ptr) cinfo->src;

	// ファイル読み込み開始のフラグを立てる
	src->start_of_file = TRUE;
}

// データバッファにデータを転送する
METHODDEF(boolean)
fill_input_buffer_general (j_decompress_ptr cinfo)
{
	my_src_general_ptr src = (my_src_general_ptr) cinfo->src;
	size_t nbytes;

	// 転送するデータの量をコピーする
	nbytes = ( src->DataSize - STTELL( src->Data ) < INPUT_BUF_SIZE ) ?
			 src->DataSize - STTELL( src->Data ) : INPUT_BUF_SIZE ;
	if( nbytes != 0 )
	{
		STREAD( src->buffer, nbytes, 1, src->Data ) ;
	}

	// 読み込みに失敗したらエラー
	if( nbytes <= 0 )
	{
		if (src->start_of_file)	/* Treat empty input file as fatal error */
			ERREXIT(cinfo, JERR_INPUT_EMPTY);
		WARNMS(cinfo, JWRN_JPEG_EOF);

		/* Insert a fake EOI marker */
	    src->buffer[0] = (JOCTET) 0xFF;
		src->buffer[1] = (JOCTET) JPEG_EOI;
		nbytes = 2;
	}

	// その他の処理
	src->pub.next_input_byte = src->buffer;
	src->pub.bytes_in_buffer = nbytes;
	src->start_of_file = FALSE;

	return TRUE;
}

// 指定されたサイズのデータをスキップする処理
METHODDEF(void)
skip_input_data_general( j_decompress_ptr cinfo, long num_bytes)
{
	my_src_general_ptr src = (my_src_general_ptr) cinfo->src;

	// データスキップ処理
	if( num_bytes > 0 )
	{
		while( num_bytes > (long) src->pub.bytes_in_buffer )
		{
			num_bytes -= (long) src->pub.bytes_in_buffer;
			(void) fill_input_buffer_general( cinfo ) ;
		}
		src->pub.next_input_byte += (size_t) num_bytes;
		src->pub.bytes_in_buffer -= (size_t) num_bytes;
	}
}

// データを閉じるときの処理
METHODDEF(void)
term_source_general( j_decompress_ptr cinfo )
{
  /* no work necessary here */
}

// 汎用データ読み込み処理からデータを読み込むようにする実際の設定を行う関数
GLOBAL(void)
jpeg_general_src (j_decompress_ptr cinfo, STREAMDATA *Data )
{
	my_src_general_ptr src;

	// まだＪＰＥＧデータを一時的に格納するバッファを確保していなかったら確保する
	if (cinfo->src == NULL)
	{
		/* first time for this JPEG object? */
		cinfo->src = (struct jpeg_source_mgr *)
						(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
						sizeof(my_source_general_mgr));
		src = (my_src_general_ptr) cinfo->src;
		src->buffer = (JOCTET *)
					(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
					INPUT_BUF_SIZE * sizeof(JOCTET));
	}

	// 関数ポインタなどをセットする
	src = (my_src_general_ptr) cinfo->src;
	src->pub.init_source			= init_source_general ;
	src->pub.fill_input_buffer		= fill_input_buffer_general ;
	src->pub.skip_input_data		= skip_input_data_general ;
	src->pub.resync_to_restart		= jpeg_resync_to_restart ; /* use default method */
	src->pub.term_source			= term_source_general ;

	src->Data = Data ;

	// 現在のファイルポインタから終端までのサイズを取得する
	{
		long pos ;
		pos = STTELL( src->Data ) ;
		STSEEK( src->Data, 0, STREAM_SEEKTYPE_END ) ;
		src->DataSize = STTELL( src->Data ) - pos ;
		STSEEK( src->Data, pos, STREAM_SEEKTYPE_SET ) ;
	}

	src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
	src->pub.next_input_byte = NULL; /* until buffer loaded */
}

#ifdef DX_TEST	// テスト用
extern LONGLONG time2;
#endif




extern int LoadJpegImage(const char *FileName)
{
	STREAMDATA src;
	STREAMDATA *Src = &src;
	if(SetupSTREAMDATA(FileName,Src) == -1)
	{
		return -1;
	}
	struct jpeg_decompress_struct cinfo ;
	struct my_error_mgr jerr ;
	JSAMPARRAY buffer ;
	int InPitch ;
	int i ;

	// 通常ＪＰＥＧエラールーチンのセットアップ
	memset( &cinfo, 0, sizeof( cinfo ) );
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	if( setjmp( jerr.setjmp_buffer ) )
	{
		jpeg_destroy_decompress( &cinfo );
		STCLOSE(Src);
		return -1;
	}

#ifdef DX_TEST	// テスト用
	time2 = ST_GetNowHiPerformanceCount();
#endif
	// cinfo情報のアロケートと初期化を行う
	jpeg_create_decompress(&cinfo);

	// 汎用データ読み込み処理からデータを読み込む設定を行う
	jpeg_general_src( &cinfo, Src ) ;

	// ＪＰＥＧファイルのパラメータ情報の読みこみ
	(void)jpeg_read_header(&cinfo, TRUE);

	// ＪＰＥＧファイルの解凍の開始
	(void)jpeg_start_decompress(&cinfo);

	if(cinfo.output_components != 1 && cinfo.out_color_components != 3)
	{
		jpeg_destroy_decompress( &cinfo );
		STCLOSE(Src);
		return -1 ;
	}
	/*グラフィックハンドルを作る。*/
	int gh = MakeGraph(cinfo.output_width,cinfo.output_height,GU_PSM_8888);
	if(gh == -1)
	{
		//エラー処理
		jpeg_destroy_decompress( &cinfo );
		STCLOSE(Src);
		return -1 ;
	}

	// １ライン当たりのデータバイト数を計算
	InPitch = cinfo.output_width * cinfo.output_components ;

	// データバッファの確保
	buffer = (*cinfo.mem->alloc_sarray)
				((j_common_ptr) &cinfo, JPOOL_IMAGE, InPitch, 1 );
	
	// 画像データの読みこみ
	int row = 0;
	while( cinfo.output_scanline < cinfo.output_height && row < texarray[gh].mv)
	{
		(void) jpeg_read_scanlines(&cinfo, buffer, 1);

		// データを出力データに変換して、またはそのまま転送
		for(i = 0;i < texarray[gh].mu;++i)
		{
			if(cinfo.output_components == 1)
			{/*グレースケール*/
				*((unsigned char*)(texarray[gh].pmemory) + texarray[gh].pitch * row * 4 + i * 4 + 0) = 0xff;
				*((unsigned char*)(texarray[gh].pmemory) + texarray[gh].pitch * row * 4 + i * 4 + 1) = *(buffer[0] + i);
				*((unsigned char*)(texarray[gh].pmemory) + texarray[gh].pitch * row * 4 + i * 4 + 2) = *(buffer[0] + i);
				*((unsigned char*)(texarray[gh].pmemory) + texarray[gh].pitch * row * 4 + i * 4 + 3) = *(buffer[0] + i);
			}
			else
			{/*ABGR*/
				*((unsigned char*)(texarray[gh].pmemory) + texarray[gh].pitch * row * 4 + i * 4 + 3) = 0xff;					//A
				*((unsigned char*)(texarray[gh].pmemory) + texarray[gh].pitch * row * 4 + i * 4 + 2) = *(buffer[0] + i * 3 + 2);//B
				*((unsigned char*)(texarray[gh].pmemory) + texarray[gh].pitch * row * 4 + i * 4 + 1) = *(buffer[0] + i * 3 + 1);//G
				*((unsigned char*)(texarray[gh].pmemory) + texarray[gh].pitch * row * 4 + i * 4 + 0) = *(buffer[0] + i * 3 + 0);//R
			}
		}
		++row;
	}

	// 解凍処理の終了
	(void) jpeg_finish_decompress(&cinfo);
	// cinfo構造体の解放
	jpeg_destroy_decompress(&cinfo);
#ifdef DX_TEST	// テスト用
	time2 = ST_GetNowHiPerformanceCount() - time2 ;
#endif

	// 終了
//	GraphicNormalize(texi);/*グラフィックを正規化（っていうの？）*/
	STCLOSE(Src);
	return gh ;
}


#else
// ＪＰＥＧ画像の読みこみ PSP内部のライブラリを使うバージョン
DXPTEXTURE2* LoadJpegImage(const char *FileName)
{
	if(sceJpegInitMJpeg() < 0)return NULL;
	if(sceJpegCreateMJpeg(512,512) < 0)
	{
		sceJpegFinishMJpeg();
		return NULL;
	}
	int size = FileRead_size(FileName);
	u8* rgbabuf = (u8*)malloc(4*512*512);
	u8* databuf = (u8*)malloc(size);
	if(databuf == NULL || rgbabuf == NULL)
	{
		free(databuf);
		free(rgbabuf);
		sceJpegDeleteMJpeg();
		sceJpegFinishMJpeg();
		return NULL;
	}
	int fh = FileRead_open(FileName);
	if(fh == -1)
	{
		free(databuf);
		free(rgbabuf);
		sceJpegDeleteMJpeg();
		sceJpegFinishMJpeg();
		return NULL;
	}
	FileRead_read(databuf,size,fh);
	FileRead_close(fh);
	int res = sceJpegDecodeMJpeg(databuf,size,rgbabuf,0);
	free(databuf);
	sceJpegDeleteMJpeg();
	sceJpegFinishMJpeg();
	if(res < 0)
	{
		free(rgbabuf);
		return NULL;
	}
	int height = res >> 16;
	int width = res & 0x0000ffff;
	DXPTEXTURE2 *texptr = MakeTexture(width,height,DXP_FMT_8888);
	if(texptr == NULL)
	{
		free(rgbabuf);
		return NULL;
	}
//	memcpy(texptr->pmemory,rgbabuf,4 * height * width);//この1行なぜ存在するのか理解不能。消し忘れかもｗ動作に影響なければ削除。
	int i;
	for(i = 0;i < height;++i)
		memcpy(((u32*)texptr->pmemory) + texptr->pitch * i,((u32*)rgbabuf) + 512 * i,width * 4);
	free(rgbabuf);
	return texptr;
}
#endif
