//
//DX���C�u����Portable �����\���T���v��
//
//�쐬��	:2009/04/18
//�쐬��	:�V���[�e�B���O��D��
//�ŏI�X�V��:2009/04/18
//
//�ύX����
//2009/04/18 �{�T���v���̌��J
//
//	int	InitString();
//		�t�H���g�̏��������s���܂��B
//		��������Y�ꂽ�ꍇ�������`��n�֐��ōŏ��ɌĂ΂ꂽ�i�K�ŏ��������s���܂��B
//		�������Q�[���Ȃǂ̃��A���^�C���������߂���ꍇ�͂Ȃ�ׂ������I�ɏ�������
//		�s�������������Ǝv���܂��B
//	int	EndString();
//		�t�H���g�̔j�����s���܂��B
//
//	int	DrawString( int x, int y, const char *String, int Color);
//		�������\�����܂��B
//
//	int	DrawFormatString(int x,int y,int color,const char *String,...);
//		�������w�肵�ĕ������\�����܂��B
//
//	int	SetFontSize(int FontSize);
//		�t�H���g�T�C�Y���h�b�g�Ŏw��ł��܂��B
//		�f�t�H���g�h�b�g��16�ł��B
//		�������A�����̃o�b�N�ɐF�ݒ薳���̏�Ԃł��s�Ԃ̌��Ԃ͖�����Ԃ�
//		�Ȃ�܂��̂Œ��ӂ��K�v�ł��B
//
//	int	SetFontSizeF(float FontSize);
//		�t�H���g�T�C�Y���X�P�[���Ŏw��ł��܂��B
//		�f�t�H���g�X�P�[����1.0f�ł��B
//
//	int	SetFontBackgroundColor(int Color);
//		�t�H���g�̔w��̉A�̕������w��ł��܂��B
//		�t�H���g�̕����̐F�͕����`��n�֐��Ŏw�肵�Ă��������B
//		��\�I�ȐF��DX���C�u����Portable�ŗp�ӂ��Ă���܂��B
//			����						�J���[
//			DXP_FONT_COLOR_NONE			0x00000000
//			DXP_FONT_COLOR_BLACK		0xff000000
//			DXP_FONT_COLOR_RED 			0xff0000ff
//			DXP_FONT_COLOR_GREEN		0xff00ff00
//			DXP_FONT_COLOR_BLUE			0xffff0000
//			DXP_FONT_COLOR_WHITE		0xffffffff
//			DXP_FONT_COLOR_LITEGRAY		0xffbfbfbf
//			DXP_FONT_COLOR_GRAY			0xff7f7f7f
//			DXP_FONT_COLOR_DARKGRAY		0xff3f3f3f
//		���ڃJ���[���w�肷�邱�Ƃŏ�L�ȊO�̐F�����p�\�ł��B
//
//	int	SetFontAlignment(int Position,int Width);
//		�t�H���g�̔z�u���w��ł��܂��B
//		��������DXP_FONT_WIDTH_FIX�̂ݎg�p���܂��B
//		���̑��w�莞�͖�������܂��̂ŔC�ӂ̐������w�肵�Ă��������B
//		DXP_FONT_ALIGN_LEFT		����
//		DXP_FONT_ALIGN_CENTER		����
//		DXP_FONT_ALIGN_RIGHT		�E��
//		DXP_FONT_WIDTH_FIX		����
//		DXP_FONT_ALIGN_DEFAULT	�f�t�H���g(����)
//		��DXP_FONT_WIDTH_FIX�ɂ���
//			1�`255�͈̔͂Ŏw��\�ł��B
//			0�ȉ����w�肳�ꂽ�ꍇ��1�A256�ȏ��255�Ƃ��Ĕ͈͓��Ɏ��߂܂��B
//
//�������֐�(�֐����Ăяo���Ă��������������܂���B���^�[���R�[�h�͏��0)
//CreateFontToHandle
//DrawFormatStringToHandle


#define GLOBAL_INSTANCE 
#include "../../DXLibralyPortable/dxlibp.h"

PSP_MODULE_INFO("FontSample", 0, 1, 1);		//���W���[������ݒ�
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);		//���[�U�[���[�h�ɐݒ�
PSP_MAIN_THREAD_STACK_SIZE_KB(1024);		//�X�^�b�N�T�C�Y�𖾎�����ꍇ�Ɏg�p

int SetBaseColor(u32 color);

int ProcessLoop(){
	if(ProcessMessage()!=0)		return -1;	//�v���Z�X�������G���[�Ȃ�-1��Ԃ�
	return 0;
}

int main(void)
{
	if (DxLib_Init() != 0)		return	0;	//DxLib�̏�����
	if(ClearDrawScreen()!=0)	return	-1;	//��ʃN���A�������G���[�Ȃ�-1��Ԃ�
	if(InitString() != 0)		return	-1;	//�t�H���g�̏�����
	int y;
	SetFontBackgroundColor(DXP_FONT_COLOR_NONE);	//�f�t�H���g�̓o�b�N�J���[����Ȃ̂Ŗ����ɂ���
	while(ProcessLoop()==0){				//���C�����[�v
		ClearDrawScreen();
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND,255);
		DrawBox(0,0,480,272,0xffffffff,1);
		SetDrawBlendMode(DX_BLENDMODE_SUB,255);
		y = 26;

		SetFontSizeF(1.5f);
		DrawFormatString(  0,  y,DXP_FONT_COLOR_RED,"1�����̃T���v���ł��B%d",y);
		y += 16;
		SetFontSizeF(1.0f);
		DrawFormatString(  0,  y,DXP_FONT_COLOR_LITEGRAY,"2�����̃T���v���ł��B%d",y);
		y += 20;
		SetFontSizeF(0.5f);
		DrawFormatString(  0,  y,DXP_FONT_COLOR_GREEN,"3�����̃T���v���ł��B%d",y);
		y += 25;

		SetFontSize(DXP_FONT_DEFAULT_SIZE*1.5);
		DrawFormatString(  0,  y,DXP_FONT_COLOR_DARKGRAY,"4�����̃T���v���ł��B%d",y);
		y += 25;
		SetFontSize(DXP_FONT_DEFAULT_SIZE*1.0);
		SetFontAlignment(DXP_FONT_ALIGN_CENTER,0);
		DrawFormatString(240,  y,DXP_FONT_COLOR_BLUE,"5�����̃T���v���ł��B%d",y);
		y += 25;
		SetFontSize(DXP_FONT_DEFAULT_SIZE*0.5);
		SetFontAlignment(DXP_FONT_ALIGN_RIGHT,0);
		DrawFormatString(479,  y,DXP_FONT_COLOR_GRAY,"6�����̃T���v���ł��B%d",y);
		SetFontAlignment(DXP_FONT_ALIGN_LEFT,0);
		y += 25;
		ScreenFlip();						//����ʔ��f
	}
	EndString();
	DxLib_End();							//�c�w���C�u�����I������
	return 0;
}
