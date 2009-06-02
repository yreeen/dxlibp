
#include "../DXLibraryPortable/dxlibp.h"



PSP_MODULE_INFO("Test", 0, 1, 1); //モジュール情報を設定
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER); //ユーザーモードに設定

	
int main()
{
	if(DxLib_Init() == -1)return -1;
	int t = LoadSoundMem("cs.wav");
	int gh = LoadGraph("test.png");
	UnswizzleGraph(gh);
	SetDisplayFormat(DXP_FMT_5650);
	ConvertGraphFormat(gh,DXP_FMT_5650);
	printfDx("%d",t);
	ScreenFlip();
	PlaySoundMem(t,DX_PLAYTYPE_BACK);
	int t1 = 1,t2 = 1;
	while(ProcessMessage() != -1)
	{
		t1 = GetNowCount();
		ClearDrawScreen();
		DrawFormatString(0,100,0xffffffff,"%d",t2);
		for(int i = 0;i < 50000;++i)DrawGraph(200,100,gh,0);

		ScreenFlip();
		t2 = GetNowCount() - t1;
	}
	DxLib_End() ; // ＤＸライブラリ使用の終了処理
	return 0 ; // ソフトの終了
}



















































































































































