#include "../DXLibralyPortable/dxlibp.h"

PSP_MODULE_INFO("Test", 0, 1, 1); //モジュール情報を設定
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER); //ユーザーモードに設定
int main(void)
{
	DxLib_Init();//DXPを初期化
	int sh = LoadStreamSound("test.mp3");
	PlayStreamSound(sh,DX_PLAYTYPE_LOOP);
	while(ProcessMessage() != -1)//この関数がエラーを返すまでループする
	{
	}
	DxLib_End(); //ライブラリを終了
	return 0;
}
