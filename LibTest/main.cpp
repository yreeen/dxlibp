#include "../DXLibralyPortable/dxlibp.h"
#include <pspprof.h>
PSP_MODULE_INFO("Test", 0, 1, 1); //���W���[������ݒ�
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER); //���[�U�[���[�h�ɐݒ�
int main()
{
	int GrHandle ;
	float Z, ZAdd ; 
	VERTEX_3D Vertex[6] ;	// �|���S���Q���Ȃ̂ŁA���_�͂U��

	// �c�w���C�u��������������
	if( DxLib_Init() == -1 )	
		 return -1 ;	// �G���[���N�����璼���ɏI��

	// �`���𗠉�ʂɂ���
	SetDrawScreen( DXP_SCREEN_BACK ) ;

	// �e�N�X�`���̓ǂݍ���
	int hbuf[5];
	GrHandle = LoadDivGraph( "test.png",5,5,1,23,23,hbuf ) ;

	// �y�l�̏�����
	Z = 0.0F ;

	// �y�l�̉��Z�l��������(�ŏ��͋߂Â���)
	ZAdd = -1.0F ;

	int cnt = 0;
	// �����L�[���������܂Ń��[�v
	while(! (GetInputState() & DXP_INPUT_CROSS) )
	{
		;
		// ���b�Z�[�W����
		if( ProcessMessage() != 0 ) break ;

		// ��ʂ̏�����
		ClearDrawScreen() ;
		DrawGraph(0,0,GrHandle = hbuf[(cnt++) % 5],0);
		//DrawBox(0,0,240,272,0xffffffff,GrHandle != -1 ? 1 : 0 );

		// �y�l�̏���
//		Z += ZAdd ;

		// ���̃��C�����z���Ă�����i�s�����𔽓]����
//		if( Z < -300.0F || Z > 300.0F ) ZAdd = -ZAdd ;
		Z = -200;
		// ���_���̃Z�b�g
		{
			// ��ʂ̒����ɕ��E����100�ŕ`��
			Vertex[0].pos.x = 0.0F - 20.0F ;	Vertex[0].pos.y = 0.0F + 20.0F ;	Vertex[0].pos.z = Z ;
			Vertex[0].u = 0.0F ;
			Vertex[0].v = 0.0F ;

			Vertex[1].pos.x = 0.0F + 20.0F ;	Vertex[1].pos.y = 0.0F + 20.0F ;	Vertex[1].pos.z = Z ;
			Vertex[1].u = 1.0F ;
			Vertex[1].v = 0.0F ;
	
			Vertex[2].pos.x = 0.0F - 20.0F ;	Vertex[2].pos.y = 0.0F - 20.0F ;	Vertex[2].pos.z = Z ;
			Vertex[2].u = 0.0F ;
			Vertex[2].v = 1.0F ;

			Vertex[3].pos.x = 0.0F + 20.0F ;	Vertex[3].pos.y = 0.0F - 20.0F ;	Vertex[3].pos.z = Z ;
			Vertex[3].u = 1.0F ;
			Vertex[3].v = 1.0F ;

			Vertex[4].pos.x = 0.0F - 20.0F ;	Vertex[4].pos.y = 0.0F - 20.0F ;	Vertex[4].pos.z = Z ;
			Vertex[4].u = 0.0F ;
			Vertex[4].v = 1.0F ;

			Vertex[5].pos.x = 0.0F + 20.0F ;	Vertex[5].pos.y = 0.0F + 20.0F ;	Vertex[5].pos.z = Z ;
			Vertex[5].u = 1.0F ;
			Vertex[5].v = 0.0F ;

			// �P�x�͑S�v�f100%
			Vertex[0].r = Vertex[0].g = Vertex[0].b = 255 ;
			Vertex[1].r = Vertex[1].g = Vertex[1].b = 255 ;
			Vertex[2].r = Vertex[2].g = Vertex[2].b = 255 ;
			Vertex[3].r = Vertex[3].g = Vertex[3].b = 255 ;
			Vertex[4].r = Vertex[4].g = Vertex[4].b = 255 ;
			Vertex[5].r = Vertex[5].g = Vertex[5].b = 255 ;

			// a ���ő�l
			Vertex[0].a = 255 ;
			Vertex[1].a = 255 ;
			Vertex[2].a = 255 ;
			Vertex[3].a = 255 ;
			Vertex[4].a = 255 ;
			Vertex[5].a = 255 ;
		}

		//// ���_���̃Z�b�g
		//{
		//	// ��ʂ̒����ɕ��E����100�ŕ`��
		//	Vertex[0].pos.x = 320.0F - 50.0F ;	Vertex[0].pos.y = 240.0F + 50.0F ;	Vertex[0].pos.z = Z ;
		//	Vertex[0].u = 0.0F ;
		//	Vertex[0].v = 0.0F ;

		//	Vertex[1].pos.x = 320.0F + 50.0F ;	Vertex[1].pos.y = 240.0F + 50.0F ;	Vertex[1].pos.z = Z ;
		//	Vertex[1].u = 1.0F ;
		//	Vertex[1].v = 0.0F ;
	
		//	Vertex[2].pos.x = 320.0F - 50.0F ;	Vertex[2].pos.y = 240.0F - 50.0F ;	Vertex[2].pos.z = Z ;
		//	Vertex[2].u = 0.0F ;
		//	Vertex[2].v = 1.0F ;

		//	Vertex[3].pos.x = 320.0F + 50.0F ;	Vertex[3].pos.y = 240.0F - 50.0F ;	Vertex[3].pos.z = Z ;
		//	Vertex[3].u = 1.0F ;
		//	Vertex[3].v = 1.0F ;

		//	Vertex[4].pos.x = 320.0F - 50.0F ;	Vertex[4].pos.y = 240.0F - 50.0F ;	Vertex[4].pos.z = Z ;
		//	Vertex[4].u = 0.0F ;
		//	Vertex[4].v = 1.0F ;

		//	Vertex[5].pos.x = 320.0F + 50.0F ;	Vertex[5].pos.y = 240.0F + 50.0F ;	Vertex[5].pos.z = Z ;
		//	Vertex[5].u = 1.0F ;
		//	Vertex[5].v = 0.0F ;

		//	// �P�x�͑S�v�f100%
		//	Vertex[0].r = Vertex[0].g = Vertex[0].b = 255 ;
		//	Vertex[1].r = Vertex[1].g = Vertex[1].b = 255 ;
		//	Vertex[2].r = Vertex[2].g = Vertex[2].b = 255 ;
		//	Vertex[3].r = Vertex[3].g = Vertex[3].b = 255 ;
		//	Vertex[4].r = Vertex[4].g = Vertex[4].b = 255 ;
		//	Vertex[5].r = Vertex[5].g = Vertex[5].b = 255 ;

		//	// a ���ő�l
		//	Vertex[0].a = 255 ;
		//	Vertex[1].a = 255 ;
		//	Vertex[2].a = 255 ;
		//	Vertex[3].a = 255 ;
		//	Vertex[4].a = 255 ;
		//	Vertex[5].a = 255 ;
		//}

		// �|���S���𓧉ߐF�����łQ���`��
		DrawPolygon3D( Vertex, 2, GrHandle, FALSE ) ;

		// ����ʂ̓��e��\��ʂɔ��f
		ScreenFlip() ;
	}

	// �c�w���C�u�����g�p�̏I������
	DxLib_End() ;

	// �\�t�g�̏I��
	gprof_cleanup();
	return 0 ;
}


