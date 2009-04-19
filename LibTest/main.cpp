#include "../DXLibralyPortable/dxlibp.h"

PSP_MODULE_INFO("Test", 0, 1, 1); //モジュール情報を設定
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER); //ユーザーモードに設定
int main(void)
{
	DxLib_Init();//DXPを初期化
	int mh[100];
	int i;
	for(i = 0;i < 100;++i)
	{
		mh[i] = MakeGraph(128,128);
		if(MoveGraphToVRAM(mh[i]) == -1)break;
	}
	printfDx("%d\n",GetDisplayFormat());
	SetDisplayFormat(DXP_FMT_5650);
	printfDx("%d\n",GetDisplayFormat());
	for(;i < 100;++i)
	{
		mh[i] = MakeGraph(128,128);
		if(MoveGraphToVRAM(mh[i]) == -1)break;
	}
	SetDisplayFormat(DXP_FMT_8888);
	printfDx("%d\n",GetDisplayFormat());

	int sh = LoadStreamSound("test.mp3");
	PlayStreamSound(sh,DX_PLAYTYPE_LOOP);
	while(ProcessMessage() != -1)//この関数がエラーを返すまでループする
	{
		ClearDrawScreen();
		DrawBox(0,0,240,272,0xffffffff,1);
		ScreenFlip();
	}
	DxLib_End(); //ライブラリを終了
	return 0;
}
