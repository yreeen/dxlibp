#include "../DXLibralyPortable/dxlibp.h"

PSP_MODULE_INFO("Test", 0, 1, 1); //モジュール情報を設定
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER); //ユーザーモードに設定
int main(void)
{
	DxLib_Init();//DXPを初期化
	while(ProcessMessage() != -1)//この関数がエラーを返すまでループする
	{
		ClearDrawScreen();
		DrawCircle(100,100,50,0xffffffff,0);
		ScreenFlip();
	}
	DxLib_End(); //ライブラリを終了
	return 0;
}
