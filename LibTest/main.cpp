#include "../DXLibralyPortable/dxlibp.h"

PSP_MODULE_INFO("Test", 0, 1, 1); //���W���[������ݒ�
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER); //���[�U�[���[�h�ɐݒ�
int main(void)
{
	DxLib_Init();//DXP��������
	int buf[4];
	int gh = LoadDivGraph("test.png",3,2,2,32,32,buf);//64x64���炢���g�BVRAM����Ȃ��ƌ�ō���B
	while(ProcessMessage() != -1)//���̊֐����G���[��Ԃ��܂Ń��[�v����
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

		ScreenFlip(); //�\��ʂƗ���ʂ�����
	}
	DxLib_End(); //���C�u�������I��
	return 0;
}
