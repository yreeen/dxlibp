//
//DXライブラリPortable 文字表示サンプル
//
//作成日	:2009/04/18
//作成者	:シューティング大好き
//最終更新日:2009/04/18
//
//変更履歴
//2009/04/18 本サンプルの公開
//
//	int	InitString();
//		フォントの初期化を行います。
//		初期化を忘れた場合も文字描画系関数で最初に呼ばれた段階で初期化を行います。
//		ただしゲームなどのリアルタイム性が求められる場合はなるべく明示的に初期化を
//		行った方がいいと思います。
//	int	EndString();
//		フォントの破棄を行います。
//
//	int	DrawString( int x, int y, const char *String, int Color);
//		文字列を表示します。
//
//	int	DrawFormatString(int x,int y,int color,const char *String,...);
//		書式を指定して文字列を表示します。
//
//	int	SetFontSize(int FontSize);
//		フォントサイズをドットで指定できます。
//		デフォルトドットは16です。
//		ただし、文字のバックに色設定無しの状態でも行間の隙間は無い状態に
//		なりますので注意が必要です。
//
//	int	SetFontSizeF(float FontSize);
//		フォントサイズをスケールで指定できます。
//		デフォルトスケールは1.0fです。
//
//	int	SetFontBackgroundColor(int Color);
//		フォントの背後の陰の部分を指定できます。
//		フォントの文字の色は文字描画系関数で指定してください。
//		代表的な色をDXライブラリPortableで用意しております。
//			名称						カラー
//			DXP_FONT_COLOR_NONE			0x00000000
//			DXP_FONT_COLOR_BLACK		0xff000000
//			DXP_FONT_COLOR_RED 			0xff0000ff
//			DXP_FONT_COLOR_GREEN		0xff00ff00
//			DXP_FONT_COLOR_BLUE			0xffff0000
//			DXP_FONT_COLOR_WHITE		0xffffffff
//			DXP_FONT_COLOR_LITEGRAY		0xffbfbfbf
//			DXP_FONT_COLOR_GRAY			0xff7f7f7f
//			DXP_FONT_COLOR_DARKGRAY		0xff3f3f3f
//		直接カラーを指定することで上記以外の色も利用可能です。
//
//	int	SetFontAlignment(int Position,int Width);
//		フォントの配置を指定できます。
//		第二引数はDXP_FONT_WIDTH_FIXのみ使用します。
//		その他指定時は無視されますので任意の数字を指定してください。
//		DXP_FONT_ALIGN_LEFT		左寄せ
//		DXP_FONT_ALIGN_CENTER		中央
//		DXP_FONT_ALIGN_RIGHT		右寄せ
//		DXP_FONT_WIDTH_FIX		等幅
//		DXP_FONT_ALIGN_DEFAULT	デフォルト(左寄せ)
//		＊DXP_FONT_WIDTH_FIXについて
//			1～255の範囲で指定可能です。
//			0以下が指定された場合は1、256以上は255として範囲内に収めます。
//
//未実装関数(関数を呼び出しても何も処理をしません。リターンコードは常に0)
//CreateFontToHandle
//DrawFormatStringToHandle


#define GLOBAL_INSTANCE 
#include "../../DXLibralyPortable/dxlibp.h"

PSP_MODULE_INFO("FontSample", 0, 1, 1);		//モジュール情報を設定
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);		//ユーザーモードに設定
PSP_MAIN_THREAD_STACK_SIZE_KB(1024);		//スタックサイズを明示する場合に使用

int SetBaseColor(u32 color);

int ProcessLoop(){
	if(ProcessMessage()!=0)		return -1;	//プロセス処理がエラーなら-1を返す
	return 0;
}

int main(void)
{
	if (DxLib_Init() != 0)		return	0;	//DxLibの初期化
	if(ClearDrawScreen()!=0)	return	-1;	//画面クリア処理がエラーなら-1を返す
	if(InitString() != 0)		return	-1;	//フォントの初期化
	int y;
	SetFontBackgroundColor(DXP_FONT_COLOR_NONE);	//デフォルトはバックカラーありなので無しにする
	while(ProcessLoop()==0){				//メインループ
		ClearDrawScreen();
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND,255);
		DrawBox(0,0,480,272,0xffffffff,1);
		SetDrawBlendMode(DX_BLENDMODE_SUB,255);
		y = 26;

		SetFontSizeF(1.5f);
		DrawFormatString(  0,  y,DXP_FONT_COLOR_RED,"1文字のサンプルです。%d",y);
		y += 16;
		SetFontSizeF(1.0f);
		DrawFormatString(  0,  y,DXP_FONT_COLOR_LITEGRAY,"2文字のサンプルです。%d",y);
		y += 20;
		SetFontSizeF(0.5f);
		DrawFormatString(  0,  y,DXP_FONT_COLOR_GREEN,"3文字のサンプルです。%d",y);
		y += 25;

		SetFontSize(DXP_FONT_DEFAULT_SIZE*1.5);
		DrawFormatString(  0,  y,DXP_FONT_COLOR_DARKGRAY,"4文字のサンプルです。%d",y);
		y += 25;
		SetFontSize(DXP_FONT_DEFAULT_SIZE*1.0);
		SetFontAlignment(DXP_FONT_ALIGN_CENTER,0);
		DrawFormatString(240,  y,DXP_FONT_COLOR_BLUE,"5文字のサンプルです。%d",y);
		y += 25;
		SetFontSize(DXP_FONT_DEFAULT_SIZE*0.5);
		SetFontAlignment(DXP_FONT_ALIGN_RIGHT,0);
		DrawFormatString(479,  y,DXP_FONT_COLOR_GRAY,"6文字のサンプルです。%d",y);
		SetFontAlignment(DXP_FONT_ALIGN_LEFT,0);
		y += 25;
		ScreenFlip();						//裏画面反映
	}
	EndString();
	DxLib_End();							//ＤＸライブラリ終了処理
	return 0;
}
