#include "../DXLibralyPortable/dxlibp.h"

PSP_MODULE_INFO("Test", 0, 1, 1); //モジュール情報を設定
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER); //ユーザーモードに設定
int main(void)
{
	DxLib_Init();//DXPを初期化
	int buf[4];
	int gh = LoadDivGraph("test.png",3,2,2,32,32,buf);//64x64くらいが吉。VRAM足りないと後で困る。
	while(ProcessMessage() != -1)//この関数がエラーを返すまでループする
	{
		ClearDrawScreen();

		SetDrawBlendMode(DX_BLENDMODE_NOBLEND,255);
		DrawBox(0,0,480,272,0xff808080,1);

		//SetDrawBlendMode(DX_BLENDMODE_ADD,255);
		//DrawGraph(0,0,buf[0],0);
		//DrawGraph(100,0,buf[0],1);

		SetDrawBlendMode(DX_BLENDMODE_ADD,128);
		DrawGraph(0,100,buf[0],0);
		DrawGraph(100,100,buf[0],1);

		ScreenFlip(); //表画面と裏画面を交換
	}
	DxLib_End(); //ライブラリを終了
	return 0;
}
