#include "../DXLibralyPortable/dxlibp.h"

PSP_MODULE_INFO("Test", 0, 1, 1); //���W���[������ݒ�
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER); //���[�U�[���[�h�ɐݒ�
int main(void)
{
	DxLib_Init();//DXP��������
	while(ProcessMessage() != -1)//���̊֐����G���[��Ԃ��܂Ń��[�v����
	{
		ClearDrawScreen();
		DrawCircle(100,100,50,0xffffffff,0);
		ScreenFlip();
	}
	DxLib_End(); //���C�u�������I��
	return 0;
}
