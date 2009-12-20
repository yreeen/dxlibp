#include "../sound.h"
#include <string.h>
#include <stdio.h>
#include <psputility.h>
#include <pspaudiocodec.h>
#include <malloc.h>
#include "../safealloc.h"

int dxpSoundGetID3v1Size(int fh);
int dxpSoundGetID3v2Size(int fh);
int dxpSoundMp3CheckFrameHeader(u8 *buf);
int dxpSoundMp3GetSampleRate(u8 *buf);

int dxpSoundMp3Init(DXPSOUNDHANDLE *pHnd,int fh);
int dxpSoundMp3Seek(DXPSOUNDHANDLE *pHnd,int fh,int sample);
int dxpSoundMp3Decode(DXPSOUNDHANDLE *pHnd,int fh);
int dxpSoundMp3End(DXPSOUNDHANDLE *pHnd,int fh);

int dxpSoundGetID3v1Size(int fh)
{
	int pos,ret = 0;
	char buf[3];
	if(fh < 0)return -1;
	pos = FileRead_tell(fh);
	FileRead_seek(fh,128,SEEK_END);
	FileRead_read(buf,3,fh);
	if(!strncmp(buf,"TAG",3))ret = 128;
	FileRead_seek(fh,pos,SEEK_SET);
	return ret;
}

int dxpSoundGetID3v2Size(int fh)
{
	char header[10];
	int pos,ret = 0;
	if(fh < 0)return -1;
	pos = FileRead_tell(fh);
	FileRead_seek(fh,0,SEEK_SET);
	FileRead_read(header,10,fh);
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
		ret = tagsize + 10;
	}
	FileRead_seek(fh,pos,SEEK_SET);
	return ret;
}

int dxpSoundMp3CheckFrameHeader(u8 *buf)
{
	u32 header,version;
	const int bitrates[2][15]
	= {
		{0,	32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320 },	//MPEG1
		{0,	8,	16,	24,	32,	40,	48,	56,	64,	 80,  96,  112,	128, 144, 160 },	//MPEG2
	};
	const int samplerates[4] = {44100,48000,32000,2};

	header = buf[0];
	header = (header << 8) | buf[1];
	header = (header << 8) | buf[2];
	header = (header << 8) | buf[3];
	if((header & 0xFFE00000) != 0xFFE00000)return -1;

	version = (header & 0x180000) >> 19;
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

int dxpSoundMp3GetSampleRate(u8 *buf)
{
	u32 header;
	const int samplerates[4] = {44100,48000,32000,2};

	header = buf[0];
	header = (header << 8) | buf[1];
	header = (header << 8) | buf[2];
	header = (header << 8) | buf[3];
	return samplerates[(header & 0xc00) >> 10];
}


int dxpSoundMp3Init(DXPSOUNDHANDLE *pHnd,int fh)
{
	u8 buf[4];
	int filesize;
	int status;
	filesize = FileRead_seek(fh,0,SEEK_END);
	pHnd->id3v1 = dxpSoundGetID3v1Size(fh);
	pHnd->id3v2 = dxpSoundGetID3v2Size(fh);
	FileRead_seek(fh,pHnd->id3v2,SEEK_SET);
	if(FileRead_read(buf,4,fh) != 4)return -1;
	status = dxpSoundMp3CheckFrameHeader(buf);
	if(status == -1)return -1;
	pHnd->format = DXP_SOUNDFMT_MP3;
	pHnd->sampleRate = dxpSoundMp3GetSampleRate(buf);

	for(pHnd->length = 0;(status = dxpSoundMp3CheckFrameHeader(buf)) != -1;pHnd->length += 1152)
	{
		FileRead_seek(fh,status - 4,SEEK_CUR);
		if(FileRead_read(buf,4,fh) != 4)break;
		if(!strncmp(buf,"TAG",3))break;
	}
	FileRead_seek(fh,pHnd->id3v2,SEEK_SET);
	pHnd->mp3.avBuf = (DXPAVCODEC_BUFFER*)dxpSafeAlloc(sizeof(DXPAVCODEC_BUFFER));
	if(!pHnd->mp3.avBuf)return -1;
	memset(pHnd->mp3.avBuf,0,sizeof(DXPAVCODEC_BUFFER));
	status = sceAudiocodecCheckNeedMem((unsigned long*)pHnd->mp3.avBuf,PSP_CODEC_MP3);
	if(status < 0)
	{
		free(pHnd->mp3.avBuf);
		return -1;
	}
	status = sceAudiocodecGetEDRAM((unsigned long*)pHnd->mp3.avBuf,PSP_CODEC_MP3);
	if(status < 0)
	{
		free(pHnd->mp3.avBuf);
		return -1;
	}
	status = sceAudiocodecInit((unsigned long*)pHnd->mp3.avBuf,PSP_CODEC_MP3);
	if(status < 0)
	{
		sceAudiocodecReleaseEDRAM((unsigned long*)pHnd->mp3.avBuf);
		free(pHnd->mp3.avBuf);
		return -1;
	}
	pHnd->mp3.mp3Buf = NULL;
	pHnd->mp3.mp3BufSize = 0;
	pHnd->currentPos = 0;
	pHnd->pcmOutSize = 1152 * 2 * 2;
	return 0;
}

int dxpSoundMp3Seek(DXPSOUNDHANDLE *pHnd,int fh,int sample)
{
	if(pHnd->length < sample)return -1;
	int frame = sample / 1152,i;
	int frameLen;
	u8 buf[4];
	FileRead_seek(fh,pHnd->id3v2,SEEK_SET);
	for(i = 0;i < frame;++i)
	{
		FileRead_read(buf,4,fh);
		frameLen = dxpSoundMp3CheckFrameHeader(buf);
		if(frameLen == -1)return -1;
		FileRead_seek(fh,frameLen - 4,SEEK_CUR);
	}
	pHnd->currentPos = frame * 1152;
	return 0;
}

int dxpSoundMp3Decode(DXPSOUNDHANDLE *pHnd,int fh)
{
	int frameLen;
	u8 headerBuf[4] = {0,0,0,0};
	int status;
	if(pHnd->format != DXP_SOUNDFMT_MP3)return -1;
	FileRead_read(headerBuf,4,fh);
	if(headerBuf[0] == 'T' && headerBuf[1] == 'A' && headerBuf[2] == 'G')return -1;
	frameLen = dxpSoundMp3CheckFrameHeader(headerBuf);
	if(frameLen < 0)return -1;
	if(pHnd->mp3.mp3BufSize < frameLen)
	{
		free(pHnd->mp3.mp3Buf);
		pHnd->mp3.mp3Buf = dxpSafeAlloc(frameLen);
		if(!pHnd->mp3.mp3Buf)return -1;
		pHnd->mp3.mp3BufSize = frameLen;
	}
	FileRead_read(pHnd->mp3.mp3Buf + 4,frameLen - 4,fh);
	memcpy(pHnd->mp3.mp3Buf,headerBuf,4);
	pHnd->mp3.avBuf->datIn = pHnd->mp3.mp3Buf;
	pHnd->mp3.avBuf->decodeByte = 1152 * 2 * 2;
	pHnd->mp3.avBuf->frameSize0 =
		pHnd->mp3.avBuf->frameSize1 = frameLen;
	pHnd->mp3.avBuf->pcmOut = pHnd->pcmOut;
	status = sceAudiocodecDecode((unsigned long*)pHnd->mp3.avBuf,PSP_CODEC_MP3);
	if(status < 0)return -1;
	pHnd->currentPos += 1152;
	return 0;
}

int dxpSoundMp3End(DXPSOUNDHANDLE *pHnd,int fh)
{
	if(pHnd->format != DXP_SOUNDFMT_MP3)return -1;
	FileRead_close(fh);
	sceAudiocodecReleaseEDRAM((unsigned long*)pHnd->mp3.avBuf);
	dxpSafeFree(pHnd->mp3.avBuf);
	dxpSafeFree(pHnd->mp3.mp3Buf);
	return 0;
}