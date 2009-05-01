/*
*	DXライブラリPortable	入力管理部	Ver1.00
*	製作者	：夢瑞憂煉
*
*	備考	：
*	占有メモリの目安はおよそ40Byteです。
*/
#include <pspctrl.h>
#include "dxlibp.h"
#include <string.h>

int InitInput();		/*初期化*/
int RenewInput();		/*更新*/
int GetInputState();	/*取得*/
int GetAnalogInput(int *XBuf,int *YBuf);/*アナログデータの取得*/

static struct SceCtrlData CtrlData;		/*入力データ　RenewInputで更新される*/


//モード制御
int	PspCtrlMode		= 0;// 0) マイクロソフトサイドワインダーパッド準拠モード
						// 1) キーボードモード
int PspAnalogMode	= 1;// 0) PSPアナログパッドを十字キーの結果に合成する
						//    マイクロソフトサイドワインダーパッドに
						//    アナログ機能無しとして振舞う
						// 1) PSPアナログパッドを十字キーの結果に合成しない
						//    マイクロソフトサイドワインダーパッドに
						//    アナログ機能有りとして振舞う
						//注) PspCtrlModeのモードに関係なく振舞う

//PSPのダメアナログパッド(笑)のブレを吸収するための数字
#define ANALOG_BOUNDARY_MIN  85
#define ANALOG_BOUNDARY_MAX 170

int InitInput()
{
	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
	return 0;
}
int RenewInput()
{
	return sceCtrlReadBufferPositive(&CtrlData,1) < 0 ? -1 : 0;
}
int GetInputState()
{
	return CtrlData.Buttons;
}
int GetAnalogInput(int *XBuf,int *YBuf)
{
	//アナログモード判定
	// 0) PSPアナログパッドを十字キーの結果に合成するため数字を返さない(デフォルト
	// 1) PSPアナログパッドを十字キーの結果に合成しないため数字を返す
	//    (面倒なので0でないときは合成しない処理にしてます
	if(PspAnalogMode == 0)
	{
		*XBuf = 0;
		*YBuf = 0;
	}
	else
	{
		*XBuf = CtrlData.Lx * 7.8125 - 1000;
		*YBuf = CtrlData.Ly * 7.8125 - 1000;
	}
	return 0;
}
int GetHitKeyStateAll( char *KeyStateBuf )
{
	if(KeyStateBuf == NULL)return -1;
	memset(KeyStateBuf,0x00,256);
	//PADエミュレーション時は処理をスキップ
	if(PspCtrlMode == 0) return 0;
	KeyStateBuf[KEY_INPUT_UP] = CtrlData.Buttons & DXP_INPUT_UP ? 1 : 0;
	KeyStateBuf[KEY_INPUT_DOWN] = CtrlData.Buttons & DXP_INPUT_DOWN ? 1 : 0;
	KeyStateBuf[KEY_INPUT_LEFT] = CtrlData.Buttons & DXP_INPUT_LEFT ? 1 : 0;
	KeyStateBuf[KEY_INPUT_RIGHT] = CtrlData.Buttons & DXP_INPUT_RIGHT ? 1 : 0;
	KeyStateBuf[KEY_INPUT_Z] = CtrlData.Buttons & DXP_INPUT_CROSS ? 1 : 0;
	KeyStateBuf[KEY_INPUT_X] = CtrlData.Buttons & DXP_INPUT_CIRCLE ? 1 : 0;
	KeyStateBuf[KEY_INPUT_A] = CtrlData.Buttons & DXP_INPUT_SQUARE ? 1 : 0;
	KeyStateBuf[KEY_INPUT_S] = CtrlData.Buttons & DXP_INPUT_TRIANGLE ? 1 : 0;
	KeyStateBuf[KEY_INPUT_Q] = CtrlData.Buttons & DXP_INPUT_LTRIGGER ? 1 : 0;
	KeyStateBuf[KEY_INPUT_W] = CtrlData.Buttons & DXP_INPUT_RTRIGGER ? 1 : 0;
	KeyStateBuf[KEY_INPUT_ESCAPE] = CtrlData.Buttons & DXP_INPUT_START ? 1 : 0;
	KeyStateBuf[KEY_INPUT_SPACE] = CtrlData.Buttons & DXP_INPUT_SELECT ? 1 : 0;
	return 0;
}

int GetJoypadInputState( int InputType )
{
	//キーボードエミュレーション時は処理をスキップ
	if(PspCtrlMode == 1) return 0;
	int ret = 0;
	//PSPアナログパッドを十字キーの結果に合成するモードの処理
	if (PspAnalogMode == 0) {
		if (CtrlData.Lx <= ANALOG_BOUNDARY_MIN) CtrlData.Buttons |= PSP_CTRL_LEFT;
		if (CtrlData.Lx >= ANALOG_BOUNDARY_MAX) CtrlData.Buttons |= PSP_CTRL_RIGHT;
		if (CtrlData.Ly <= ANALOG_BOUNDARY_MIN) CtrlData.Buttons |= PSP_CTRL_UP;
		if (CtrlData.Ly >= ANALOG_BOUNDARY_MAX) CtrlData.Buttons |= PSP_CTRL_DOWN;
	}
	if(CtrlData.Buttons & PSP_CTRL_UP)		 ret |= PAD_INPUT_UP;
	if(CtrlData.Buttons & PSP_CTRL_DOWN)	 ret |= PAD_INPUT_DOWN;
	if(CtrlData.Buttons & PSP_CTRL_LEFT)	 ret |= PAD_INPUT_LEFT;
	if(CtrlData.Buttons & PSP_CTRL_RIGHT)	 ret |= PAD_INPUT_RIGHT;
	if(CtrlData.Buttons & PSP_CTRL_CROSS)	 ret |= PAD_INPUT_A;
	if(CtrlData.Buttons & PSP_CTRL_CIRCLE)	 ret |= PAD_INPUT_B;
	//if(CtrlData.Buttons & PSP_CTRL_)		 ret |= PAD_INPUT_C;
	if(CtrlData.Buttons & PSP_CTRL_SQUARE)	 ret |= PAD_INPUT_X;
	if(CtrlData.Buttons & PSP_CTRL_TRIANGLE) ret |= PAD_INPUT_Y;
	//if(CtrlData.Buttons & PSP_CTRL_)		 ret |= PAD_INPUT_Z;
	if(CtrlData.Buttons & PSP_CTRL_LTRIGGER) ret |= PAD_INPUT_L;
	if(CtrlData.Buttons & PSP_CTRL_RTRIGGER) ret |= PAD_INPUT_R;
	if(CtrlData.Buttons & PSP_CTRL_START)	 ret |= PAD_INPUT_START;
	if(CtrlData.Buttons & PSP_CTRL_SELECT)	 ret |= PAD_INPUT_M;
	return ret;
}

int GetJoypadNum()
{
	return 1;
}
int GetJoypadAnalogInput(int *px,int *py,int inputtype)
{
	return GetAnalogInput(px,py);
}
int GetJoypadAnalogInputRight(int *px,int *py,int inputtype)
{
	if(px != NULL)*px = 0;
	if(py != NULL)*py = 0;
	return 0;
}
//SetJoypadInputToKeyInput 　ジョイパッドの入力に対応したキーボードの入力を設定する 
int StartJoypadVibration(int a,int b,int c){return 0;}// 　ジョイパッドの振動を開始する 
int StopJoypadVibration(int a){return 0;}

//以下はPSPの独自関数
//当面は非公開で
//要望があったら公開
int	SetPspCtrlMode(int Mode)
{
	if(Mode & PSPCTRLMODE_MSWP)
	{
		PspCtrlMode = 0;
	}
	else
	{
		PspCtrlMode = 1;
	}
	return 0;
}

int	SetPspAnalogMode(int Mode)
{
	if(Mode & PSPANALOGMODE_CKEY)
	{
		PspAnalogMode = 0;
	}
	else
	{
		PspAnalogMode = 1;
	}
	return 0;
}
