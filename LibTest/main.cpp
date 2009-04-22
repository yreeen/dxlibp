#include "../DXLibralyPortable/dxlibp.h"

PSP_MODULE_INFO("Test", 0, 1, 1); //モジュール情報を設定
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER); //ユーザーモードに設定

int main()
{
//変数宣言
int GHandle[ 10 ] ;
int i ;

if( DxLib_Init() == -1 ) // ＤＸライブラリ初期化処理
{
return -1; // エラーが起きたら直ちに終了
}

// ＢＭＰ画像のメモリへの分割読み込み
LoadDivGraph( "test2.bmp" , 10 , 4 , 3 , 48 , 56 , GHandle ) ;

i = 0 ;

//メッセージ処理
while(ProcessMessage() != -1)
{
// ロードしたグラフィックのアニメーション
// グラフィックの描画(『DrawGraph』使用)
DrawGraph( 0 , 0 , GHandle[ i ] , FALSE ) ;

// アニメーションパターンナンバーを変更
i ++ ;
if( i == 10 ) i = 0 ;

// 一定時間待つ(『WaitTimer』使用)
WaitTimer( 100 ) ;

}

DxLib_End() ; // ＤＸライブラリ使用の終了処理

return 0 ; // ソフトの終了
}
