#include "dxpmusic2.h"
#define LINETRACE 	DrawFormatString(0,0,0xffffffff,"%d\n",__LINE__);ScreenFlip();
int decodeprepare_wave(DXP_MUSICDECODECONTEXT *context)
{
	u8 headerbuf[16];
	int csize;
	LINETRACE
	STSEEK(context->src,0,SEEK_SET);
	if(STREAD(headerbuf,1,8,context->src) != 8)return -1;
	LINETRACE
	if(strncmp((char*)headerbuf,"RIFF",4))return -1;
	LINETRACE
	if(STREAD(headerbuf,1,4,context->src) != 4)return -1;
	LINETRACE
	if(strncmp((char*)headerbuf,"WAVE",4))return -1;
	u8 fmt = 0;//fmtチャンクを取得したかどうかのフラグ
	LINETRACE
	while(1)
	{
		if(STREAD(headerbuf,1,8,context->src) != 8)return -1;
		if(!strncmp((char*)headerbuf,"fmt ",4))
		{
			fmt = 1;
			u16 buf2;
			u32 buf4;
			if(STREAD(&buf2,2,1,context->src) != 1)return -1;	//フォーマットタグを取得
			if(buf2 != 1)return -1;		//フォーマットタグが1以外の場合は何かしらの圧縮がなされているので再生できない
			if(STREAD(&buf2,2,1,context->src) != 1)return -1;	//チャンネル数を取得
			if(buf2 == 1)context->wavestream.monoflag = 1;
			else if(buf2 == 2)context->wavestream.monoflag = 0;
			else return -1;
			if(STREAD(&buf4,4,1,context->src) != 1)return -1;	//サンプリングレートを取得
			if(buf4 != 44100)return -1;
			STSEEK(context->src,6,SEEK_CUR);
			if(STREAD(&buf2,2,1,context->src) != 1)return -1;	//１サンプル数あたりのビット数を取得
			if(buf2 == 8)context->wavestream.s8bitflag = 1;
			else if(buf2 == 16)context->wavestream.s8bitflag = 0;
			else return -1;
			csize = headerbuf[4];
			csize = (csize << 8) | headerbuf[5];
			csize = (csize << 8) | headerbuf[6];
			csize = (csize << 8) | headerbuf[7];
			STSEEK(context->src,csize - 16,SEEK_CUR);
		}
		else if(!strncmp((char*)headerbuf,"data",4))
		{
			context->filetype = DXPMFT_WAVE;
			return fmt - 1;
		}
		else
		{
			csize = headerbuf[4];
			csize = (csize << 8) | headerbuf[5];
			csize = (csize << 8) | headerbuf[6];
			csize = (csize << 8) | headerbuf[7];
			STSEEK(context->src,csize,SEEK_CUR);
	LINETRACE
			return 0;
		}
	}
	return -1;
}

s16 s8tos16(u8 dat)
{
	s16 ret = dat;
	ret -= 128;
	ret *= 256;
	return ret;
}

int decode_wave(DXP_MUSICDECODECONTEXT *context)
{
	if(context == NULL || context->src == NULL)return -1;
	if(context->filetype != DXPMFT_WAVE)return -1;
	if(context->nextframe != -1)waveseek(context);

	int readsize = (context->wavestream.s8bitflag + 1) * (context->wavestream.monoflag + 1) * sample_per_frame(DXPMFT_WAVE);
	if(STREAD(context->out,1,readsize,context->src) != readsize){return -1;}
	if(context->wavestream.s8bitflag)
	{
		if(context->wavestream.monoflag)
		{
			s16 *out = (s16*)context->out + (sample_per_frame(DXPMFT_WAVE) - 1);
			u8 *in = (u8*)context->out + (sample_per_frame(DXPMFT_WAVE) - 1);
			while((u16*)in >= context->out)*out-- = s8tos16(*in--);
		}else
		{
			s16 *out = (s16*)context->out + (sample_per_frame(DXPMFT_WAVE) - 1) * 2;
			u8 *in = (u8*)context->out + (sample_per_frame(DXPMFT_WAVE) - 1) * 2;
			while((u16*)in >= context->out)*out-- = s8tos16(*in--);
		}
	}
	if(context->wavestream.monoflag)
	{
		s16 *out = (s16*)context->out + (sample_per_frame(DXPMFT_WAVE) - 1);
		s16 *in = (s16*)context->out + (sample_per_frame(DXPMFT_WAVE) - 1);
		while((u16*)in >= context->out)
		{
			*out-- = *in;
			*out-- = *in--;
		}
	}
	return 0;
}

int decodefinish_wave(DXP_MUSICDECODECONTEXT *context)
{
	if(context->src == NULL)return -1;
	STCLOSE(context->src);
	context->src = NULL;
	return 0;
}

int waveseek(DXP_MUSICDECODECONTEXT *context)
{
	if(context == NULL)return -1;
	char buf[8];
	while(1)
	{
		if(STREAD(buf,1,8,context->src) != 8)return -1;
		if(strncmp(buf,"data",4) == 0)break;
		if(strncmp(buf,"RIFF",4) == 0)STSEEK(context->src,4,SEEK_CUR);
		else STSEEK(context->src,(*(int*)(buf + 4)),SEEK_CUR);
	}
	STSEEK(context->src,(context->wavestream.s8bitflag + 1) * (context->wavestream.monoflag + 1) * sample_per_frame(DXPMFT_WAVE),SEEK_CUR) * context->nextframe;
	return 0;
}
