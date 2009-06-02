
#include "../DXLibralyPortable/dxlibp.h"



PSP_MODULE_INFO("Test", 0, 1, 1); //モジュール情報を設定
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER); //ユーザーモードに設定

	
int main()
{
	int gh;
	if(DxLib_Init() == -1)return -1;


	gh = LoadGraph("test.png");
	ConvertGraphFormat(gh,DXP_FMT_5650);
	SetDisplayFormat(DXP_FMT_5650);
	int n = 10000;
while(ProcessMessage() != -1)
{
	clsDx();
//	printfDx("%d",GetCpuUsage());
	if(GetInputState() & DXP_INPUT_LTRIGGER)UnswizzleGraph(gh);
	if(GetInputState() & DXP_INPUT_RTRIGGER)SwizzleGraph(gh);
	if(GetInputState() & DXP_INPUT_UP)n += 1000;
	if(GetInputState() & DXP_INPUT_DOWN)n -= 1000;
	if(GetInputState() & DXP_INPUT_TRIANGLE)n += 100;
	if(GetInputState() & DXP_INPUT_CROSS)n -= 100;
	clsDx();
	printfDx("%d",n);
	ClearDrawScreen();
	for(int i = 0;i < n;++i)DrawGraph(100,100,gh,0) ;
	ScreenFlip();




	SetDisplayFormat(DXP_FMT_5650);
	int n = 10000;
	int t = LoadSoundMem("cs.wav");
	printfDx("%d",t);
	ScreenFlip();
	PlaySoundMem(t,DX_PLAYTYPE_BACK);
while(ProcessMessage() != -1)
{
//	clsDx();
////	printfDx("%d",GetCpuUsage());
//	if(GetInputState() & DXP_INPUT_LTRIGGER)UnswizzleGraph(gh);
//	if(GetInputState() & DXP_INPUT_RTRIGGER)SwizzleGraph(gh);
//	if(GetInputState() & DXP_INPUT_UP)n += 1000;
//	if(GetInputState() & DXP_INPUT_DOWN)n -= 1000;
//	if(GetInputState() & DXP_INPUT_TRIANGLE)n += 100;
//	if(GetInputState() & DXP_INPUT_CROSS)n -= 100;
//	clsDx();
//	printfDx("%d",n);
//	ClearDrawScreen();
//	for(int i = 0;i < n;++i)DrawGraph(100,100,gh,0) ;
//	ScreenFlip();
}

DxLib_End() ; // ＤＸライブラリ使用の終了処理

return 0 ; // ソフトの終了
}



















































































































































