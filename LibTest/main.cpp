#include "../DXLibralyPortable/dxlibp.h"

PSP_MODULE_INFO("Test", 0, 1, 1); //���W���[������ݒ�
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER); //���[�U�[���[�h�ɐݒ�
int main(void)
{
	DxLib_Init();//DXP��������
	int sh = LoadStreamSound("test.mp3");
	PlayStreamSound(sh,DX_PLAYTYPE_LOOP);
	while(ProcessMessage() != -1)//���̊֐����G���[��Ԃ��܂Ń��[�v����
	{
	}
	DxLib_End(); //���C�u�������I��
	return 0;
}
