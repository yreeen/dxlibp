/*
*	DXライブラリPortable	全体管理部	Ver1.00
*	製作者	：夢瑞憂煉
*
*	備考	：とくになし
*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pspkernel.h>
#include <psputility.h>
#include <psprtc.h>
#include "dxlibp.h"
#include "dxpstatic.h"

PSP_HEAP_SIZE_KB(-1);/*ヒープメモリを限界まで使えるようにする。*/

/*大域変数*/

DXPDATA dxpdata = 
{
	10000,
	{0}
};
/*時間*/
int GetNowCount()/*milis単位での時間*/
/*TODO 現在の仕様だとRTCを使っていて遅いかもしれない。代替できるものがあれば移行。*/
/*1回あたり35マイクロ秒しかかかっていないのでこのままでいいですｗ*/
{
	u64 tick;
	if(sceRtcGetCurrentTick(&tick) != 0)return -1;
	return (int)(tick * 1000 / dxpdata.TPS);
}
u64	GetNowHiPerformanceCount()/*micros単位*/
{
	u64 tick;
	double tmp;
	if(sceRtcGetCurrentTick(&tick) != 0)return -1;
	tmp = tick;
	tmp *= 1000000;
	tmp /= dxpdata.TPS;
	return (u64)tmp;
}

/*HOMEボタン監視*/
static int exit_callback(int arg1, int arg2, void *common)
{

	sceKernelExitGame();
	return 0;
}

static int ExitCallbackThread(SceSize args, void *argp)
{
	sceKernelRegisterExitCallback(sceKernelCreateCallback("Exit Callback", exit_callback, NULL));	
	sceKernelSleepThreadCB();//よくわかんない
	return 0;
}

static int SetExitCallback(void)
{
	int thid = 0;
	
	thid = sceKernelCreateThread("update_thread", ExitCallbackThread, 0x11, 0xFA0, 0, 0);
	if (thid >= 0) {
		sceKernelStartThread(thid, 0, 0);
	}
	atexit(sceKernelExitGame);
	return thid;
}
/*乱数*/
#ifndef DXP_NOUSE_MTRAND
static SceKernelUtilsMt19937Context RandData;	/*乱数計算のためのデータ*/
int SRand(int seed)
{
	return sceKernelUtilsMt19937Init(&RandData,seed) < 0 ? -1 : 0;
}

int GetRand(int maxnum)
{
	if(maxnum <= 0)return 0;
	return sceKernelUtilsMt19937UInt(&RandData) % maxnum;
}
#else
static unsigned int randdata;
int SRand(int seed)
{
	randdata = seed;
	return 0;
}

static void lfsr()
{
	randdata = (randdata >> 1) ^ (-(randdata & 1u) & 0x80200003u); /* taps 32 31 29 1 */
}

int GetRand(int maxnum)
{
	if(maxnum == 0)return 0;
	lfsr();
	//138
	//62
	//34
	unsigned int i = randdata % 34u;
	for(i += 1;i > 0;--i)lfsr();

	return (s32)(randdata % (u32)maxnum);
}
#endif

/*必須関数*/
/*初期化*/
int DxLib_IsInit()
{
	return dxpdata.flags[0] & DXPDATAFLAGS_0_INITIALIZED;
}
int DxLib_Init()
{
	if(DxLib_IsInit())return -1;
	dxpdata.flags[0] |= DXPDATAFLAGS_0_INITIALIZING;
	sceUtilityLoadAvModule(PSP_AV_MODULE_AVCODEC);//MP3やJpegのデコードにつかうモジュールをロードする。

	SetExitCallback();/*HOMEボタンコールバック関数のセット*/
	InitInput();/*入力の初期化*/
	RenewInput();
	InitGUEngine();/*GPU周りの初期化*/
	SRand(time(NULL));/*乱数初期化*/

	dxpdata.TPS = sceRtcGetTickResolution();

	dxpdata.flags[0] &= (~DXPDATAFLAGS_0_INITIALIZING);
	dxpdata.flags[0] |= DXPDATAFLAGS_0_INITIALIZED;
	return 0;
}
int DxLib_End()
{
//	StopMusic();
	dxpdata.flags[0] &= (~DXPDATAFLAGS_0_INITIALIZED);
	sceUtilityUnloadAvModule(PSP_AV_MODULE_AVCODEC);
	EndGUEngine();
	return 0;
}

int ProcessMessage()
{
	RenewInput();
//	ProcessAudio();
	return 0;
}



int WaitTimer(int time)
{
	int i = 0;
	int t0 = GetNowCount();
	while(GetNowCount() - t0 < time)
	{
		sceKernelDelayThread(1000);
		if(i % 30 == 0)ProcessMessage();
		++i;
	}
	return 0;
}
int Sleep(int time)
{
	sceKernelDelayThread(time * 1000);
	return 0;
}




void InitProfiler(void); 
void ExitProfiler(void); 
void ClrProfileRegs(void); 
void GetProfileRegs(PspDebugProfilerRegs *dest);
//int GetCpuUsage()
//{
	//static SceUInt mod = -1;
	//PspDebugProfilerRegs regs;
	//if(mod < 0)
	//{
	//	mod = pspSdkLoadStartModule("dxpkernel.prx", PSP_MEMORY_PARTITION_KERNEL); 
	//	if(mod < 0)return -1;
	//	InitProfiler(); 
	//	ClrProfileRegs();
	//}
	//GetProfileRegs(&regs);
	//ClrProfileRegs();
	//int res = 100 * (regs.cpuck - regs.sleep) / regs.cpuck; 
	//return res;
	//return -1;
//}

int SetupSTREAMDATA(const char *FileName,STREAMDATA *DataPtr)
/*DXライブラリのSTREAMDATAを設定する。多少強引なのは仕様*/
{
	if(DataPtr == NULL)return -1;
	if((int)(DataPtr->DataPoint = (void*)FileRead_open(FileName)) == -1)return -1;
	DataPtr->ReadShred.Close	= (int(*)(void*))FileRead_close;
	DataPtr->ReadShred.Eof		= (int(*)(void*))FileRead_eof;
	DataPtr->ReadShred.IdleCheck= (int(*)(void*))FileRead_idle_chk;
	DataPtr->ReadShred.Write	= (unsigned int(*)(void*,size_t,size_t,void*))FileRead_write;
	DataPtr->ReadShred.Read		= (unsigned int(*)(void*,size_t,size_t,void*))FileRead_stdread;
	DataPtr->ReadShred.Seek		= (int(*)(void*,long,int))FileRead_seek;
	DataPtr->ReadShred.Tell		= (long(*)(void*))FileRead_tell;
	//STSEEK(DataPtr,0,SEEK_SET);
	return 0;
}
