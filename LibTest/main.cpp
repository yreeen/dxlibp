#include "../DXLibralyPortable/dxlibp.h"

PSP_MODULE_INFO("Test", 0, 1, 1); //���W���[������ݒ�
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER); //���[�U�[���[�h�ɐݒ�

int main()
{
//�ϐ��錾
int GHandle[ 10 ] ;
int i ;

if( DxLib_Init() == -1 ) // �c�w���C�u��������������
{
return -1; // �G���[���N�����璼���ɏI��
}

// �a�l�o�摜�̃������ւ̕����ǂݍ���
LoadDivGraph( "test2.bmp" , 10 , 4 , 3 , 48 , 56 , GHandle ) ;

i = 0 ;

//���b�Z�[�W����
while(ProcessMessage() != -1)
{
// ���[�h�����O���t�B�b�N�̃A�j���[�V����
// �O���t�B�b�N�̕`��(�wDrawGraph�x�g�p)
DrawGraph( 0 , 0 , GHandle[ i ] , FALSE ) ;

// �A�j���[�V�����p�^�[���i���o�[��ύX
i ++ ;
if( i == 10 ) i = 0 ;

// ��莞�ԑ҂�(�wWaitTimer�x�g�p)
WaitTimer( 100 ) ;

}

DxLib_End() ; // �c�w���C�u�����g�p�̏I������

return 0 ; // �\�t�g�̏I��
}
