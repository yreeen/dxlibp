#include "dxpmusic2.h"
#include "dxpstatic.h"
#define TRACE		printfDx("%s::%d\n",__FILE__,__LINE__);

int decodeprepare_mp3(DXP_MUSICDECODECONTEXT *context)
{
	//sceUtilityLoadAvModule(PSP_AV_MODULE_AVCODEC);
	if(DxpAvModuleInit() != 1) return -2;
	if(context == NULL)return -1;
	if(context->src == NULL)return -1;
	context->nextframe = -1;
	u8 headerbuf[4];
	STSEEK(context->src,0,SEEK_SET);
	if(id3skip(context->src) == -1)return -1;
	if(STREAD(headerbuf,1,4,context->src) != 4)return -1;
	int header = headerbuf[0];
	header = (header << 8) | headerbuf[1];
	header = (header << 8) | headerbuf[2];
	header = (header << 8) | headerbuf[3];
	if((header & 0xFFE00000) != 0xFFE00000)
	{
		return -1;	//同期ヘッダの存在確認
	}
	context->filetype = DXPMFT_MP3;
	if((context->mp3stream.codec_buffer = (unsigned long*)memalign(64,65 * sizeof(unsigned long))) == NULL)return -1;
	memset(context->mp3stream.codec_buffer,0,sizeof(unsigned long) * 65);
	if(sceAudiocodecCheckNeedMem(context->mp3stream.codec_buffer,0x1002) < 0)
	{
		free(context->mp3stream.codec_buffer);
		return -1;
	}
	if(sceAudiocodecGetEDRAM(context->mp3stream.codec_buffer,0x1002) < 0)
	{
		free(context->mp3stream.codec_buffer);
		return -1;
	}
	if(sceAudiocodecInit(context->mp3stream.codec_buffer,0x1002) < 0){
		sceAudiocodecReleaseEDRAM(context->mp3stream.codec_buffer);
		free(context->mp3stream.codec_buffer);
		return -1;
	}
	STSEEK(context->src,0,SEEK_SET);
	return 0;
}

int decode_mp3(DXP_MUSICDECODECONTEXT *context)
{
	u8 headerbuf[4];
	if(context->filetype != DXPMFT_MP3)return -1;
	if(context->src == NULL)return -1;
	if(context->nextframe != -1)
	{
		mp3seek(context->src,context->nextframe);
		context->nextframe = -1;
	}
	if(id3skip(context->src) == -1)return -1;
	if(STREAD(headerbuf,1,4,context->src) != 4)return -1;
	int header = headerbuf[0];
	header = (header << 8) | headerbuf[1];
	header = (header << 8) | headerbuf[2];
	header = (header << 8) | headerbuf[3];
	if((header & 0xFFE00000) != 0xFFE00000)return -1;	//同期ヘッダの存在確認
	int framesize = mp3framesize(header);
	u8 *databuf = (u8*)memalign(64,framesize);
	if(databuf == NULL)return -1;
	if(STREAD(databuf + 4,1,framesize - 4,context->src) != framesize - 4)
	{
		free(databuf);
		return -1;
	}
	memcpy(databuf,headerbuf,4);

	context->mp3stream.codec_buffer[6] = (unsigned long)databuf;
	context->mp3stream.codec_buffer[8] = (unsigned long)context->out;
	context->mp3stream.codec_buffer[7] = context->mp3stream.codec_buffer[10] = framesize;
	context->mp3stream.codec_buffer[9] = 4608;	//1152*4
	if(sceAudiocodecDecode(context->mp3stream.codec_buffer,0x1002) < 0)
	{
		free(databuf);
		return -1;
	}
	free(databuf);
	return 0;
}

int decodefinish_mp3(DXP_MUSICDECODECONTEXT *context)
{
	if(context->src == NULL)return -1;
	STCLOSE(context->src);
	context->src = NULL;
	sceAudiocodecReleaseEDRAM(context->mp3stream.codec_buffer);
	free(context->mp3stream.codec_buffer);
	return 0;
}


int mp3framesize(int header)
{
	const int bitrates[2][15]
	= {
		{0,	32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320 },	//MPEG1
		{0,	8,	16,	24,	32,	40,	48,	56,	64,	 80,  96,  112,	128, 144, 160 },	//MPEG2
	};
	const int samplerates[4] = {44100,48000,32000,2};
	int version = (header & 0x180000) >> 19;
	switch(version)
	{
	case 3:
		version = 0;
		break;
	case 2:
		version = 1;
		break;
	default:
		return -1;
	}
	return 144000 * bitrates[version][(header & 0xf000) >> 12] / (samplerates[(header & 0xc00) >> 10] / (version + 1)) + ((header & 0x200) >> 9);
}

int mp3seek(STREAMDATA *src,int frame)	//指定フレーム目の先頭に移動
{
	if(src == NULL)return -1;
	STSEEK(src,0,SEEK_SET);
	int i;
	u8 headerbuf[4];
	for(i = 0;i < frame;++i)
	{
		if(id3skip(src) == -1)return -1;
		if(STREAD(headerbuf,1,4,src) != 4)return -1;
		int header = headerbuf[0];
		header = (header << 8) | headerbuf[1];
		header = (header << 8) | headerbuf[2];
		header = (header << 8) | headerbuf[3];
		int framesize = mp3framesize(header);
		if(framesize == -1)return -1;
		STSEEK(src,framesize - 4,SEEK_CUR);
	}
	if(id3skip(src) == -1)return -1;
	return 0;
}

int id3skip(STREAMDATA *src)
/*
Param:
	filestream ptr
Return:
	-1:eof
others:skip size in bytes
*/
{
	char header[10];
	if(src == NULL)return -1;
	int pos = STTELL(src);
	STREAD(header,1,10,src);
	if(header[0] == 'T' && header[1] == 'A' && header[2] == 'G')
	{
		STSEEK(src,pos,SEEK_SET);
		return -1;
	}
	if(!strncmp(header,"ID3",3) || !strncmp(header,"ea3",3))
	{
		u32 tagsize;
		tagsize = (u8)header[6];
		tagsize <<= 7;
		tagsize |= (u8)header[7];
		tagsize <<= 7;
		tagsize |= (u8)header[8];
		tagsize <<= 7;
		tagsize |= (u8)header[9];

		if(header[5] & 0x10)
			tagsize += 10;

		STSEEK(src,tagsize,SEEK_CUR);
		return tagsize + 10;
	}
	STSEEK(src,pos,SEEK_SET);
	return 0;
}

int mp3len(STREAMDATA *src)//MP3ファイル全体のサンプル数を返す。
{
	int i;
	if(src == NULL)return -1;
	int pos = STTELL(src);
	STSEEK(src,0,SEEK_SET);
	id3skip(src);
	u8 readbuf[4];
	for(i = 0;;++i)
	{
		if(id3skip(src) == -1)break;
		if(STREAD(readbuf,1,4,src) != 4)break;
		u32 header;
		header = readbuf[0];
		header = (header << 8) | readbuf[1];
		header = (header << 8) | readbuf[2];
		header = (header << 8) | readbuf[3];
		int framesize = mp3framesize(header);
		if(framesize == -1)
		{
			STSEEK(src,pos,SEEK_SET);
			return -1;
		}
		STSEEK(src,framesize - 4,SEEK_CUR);
	}
	STSEEK(src,pos,SEEK_SET);
	return i * 1152;
}
