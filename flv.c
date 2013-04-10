//
// flv.c
//
// Dump Adobe flv file Info per frame.
//
// Author:Chen Gang
// Blog: http://blog.yikuyiku.com
// Corp:SINA
// At 2013/4/10 Beijing
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PRT printf
#define OK			0
#define READ_ERROR	6

#define FLV_SIZE_HEADER				9
#define FLV_SIZE_PREVIOUSTAGSIZE	4
#define FLV_SIZE_TAGHEADER			11

#define FLV_TAG_AUDIO		8
#define FLV_TAG_VIDEO		9
#define FLV_TAG_SCRIPTDATA	18

#define FLV_UI32(x) (unsigned int)(((*(x)) << 24) + ((*(x + 1)) << 16) + ((*(x + 2)) << 8) + (*(x + 3)))
#define FLV_UI24(x) (unsigned int)(((*(x)) << 16) + ((*(x + 1)) << 8) + (*(x + 2)))
#define FLV_UI16(x) (unsigned int)(((*(x)) << 8) + (*(x + 1)))
#define FLV_UI8(x) (unsigned int)(*(x))
#define FLV_TIMESTAMP(x) (int)(((*(x + 3)) << 24) + ((*(x)) << 16) + ((*(x + 1)) << 8) + (*(x + 2)))

typedef struct
{
	off_t offset;
	unsigned int tagtype;
	int datasize;
	int timestamp;
	short keyframe;
	int tagsize;
} FLVTag_t;

int readFLVTag (FLVTag_t * flvTag, int *tagID, FILE * fp)
{
	int rv;
	unsigned char buf[FLV_SIZE_TAGHEADER];
	unsigned char flags;

	(*tagID)++;
	rv = fread (buf, FLV_SIZE_TAGHEADER, 1, fp);
	if (rv < 1)
	{
		return READ_ERROR;
	}
	flvTag->tagtype = FLV_UI8 (buf);
	flvTag->datasize = FLV_UI24 (&buf[1]);
	flvTag->timestamp = FLV_TIMESTAMP (&buf[4]);
	flvTag->tagsize = FLV_SIZE_TAGHEADER + flvTag->datasize;

	PRT ("TagID: %d\t", *tagID);
	PRT ("TagType: %d\t", flvTag->tagtype);
	PRT ("DataSize: %d\t", flvTag->datasize);
	PRT ("TimeStamp: %d\t", flvTag->timestamp);
	PRT ("TagSize: %d\t", flvTag->tagsize);

	int if_avc = 0;

	fread (&flags, 1, 1, fp);
	if (flvTag->tagtype == FLV_TAG_AUDIO)
	{
		int codecid, samplerate, samplesize, stereo;

		codecid = (flags >> 4) & 0xf;
		samplerate = (flags >> 2) & 0x3;
		samplesize = (flags >> 1) & 0x1;
		stereo = flags & 0x1;

		PRT ("audio\t");
		PRT ("codecId: %d\t", codecid);
		PRT ("sampleRate: %d\t", samplerate);
		PRT ("sampleSize: %d\t", samplesize);
		//PRT ("stereo: %d\t", stereo);
	}
	else if (flvTag->tagtype == FLV_TAG_VIDEO)
	{
		int frameType, codecid;

		frameType = (flags >> 4) & 0xf;
		codecid = flags & 0xf;
		PRT ("video\t");
		PRT ("codecId: %d\t", codecid);
		PRT ("frameType: %d\t", frameType);
		if (codecid == 7)
		{
			if_avc = 1;
			unsigned char avcPacketType;

			fread (&avcPacketType, 1, 1, fp);
			PRT ("avcPacketType: %d\t", avcPacketType);
		}
	}
	else
	{
		PRT ("script\t");
	}

	if (if_avc)
	{
		fseek (fp, flvTag->datasize - 2, SEEK_CUR);
	}
	else
	{
		fseek (fp, flvTag->datasize - 1, SEEK_CUR);
	}
	fseek (fp, FLV_SIZE_PREVIOUSTAGSIZE, SEEK_CUR);

	PRT ("\n");
	return OK;
}

int main (int argc, char *argv[])
{
	unsigned char buf[FLV_SIZE_HEADER + FLV_SIZE_PREVIOUSTAGSIZE];
	int fileSize;
	FLVTag_t flvTag;

	FILE *fp = NULL;

	fp = fopen (argv[1], "r");
	PRT ("FileName: %s\n", argv[1]);

	fseek (fp, 0, SEEK_END);
	fileSize = ftell (fp);
	PRT ("FileSize: %d\n", fileSize);
	rewind (fp);

	fread (buf, FLV_SIZE_HEADER + FLV_SIZE_PREVIOUSTAGSIZE, 1, fp);
	PRT ("FileHeader: %c%c%c\n", buf[0], buf[1], buf[2]);
	PRT ("Version: %d\n", FLV_UI8 (&buf[3]));
	PRT ("HeaderSize: %d\n", FLV_UI32 (&buf[5]));
	PRT ("PreviousTagSize0: %d\n", FLV_UI32 (&buf[FLV_SIZE_HEADER]));

	int tagID = 0;

	while (readFLVTag (&flvTag, &tagID, fp) == OK)
	{
	}
	fclose (fp);

	PRT ("## \n");
	PRT ("## \n");
	PRT ("## TagType:\t8=>audio, 9=>video, 18=>script\n");
	PRT ("## audioCodecId:\t0=>uncompressed, 1=>ADPCM, 10=>AAC, 11=>Speex, 14=>MP3_8kHz, 15=>Device-specific_sound, 2=>MP3, 3=>Linear_PCM_little_endian, 4=>Nellymoser_16kHz_mono, 5=>Nellymoser_8kHz_mono, 6=>Nellymoser, 7=>G.711_A-law, 8=>G.711_mu-law\n");
	PRT ("## videoCodecId:\t1=>JPEG, 2=>Sorenson_H.263, 3=>Screen_video, 4=>On2_VP6, 5=>On2_VP6_alpha, 6=>Screen_video_v2, 7=>AVC\n");
	PRT ("## sampleRate:\t0=>5518Hz, 1=>11025Hz, 2=>22050Hz, 3=>44100Hz\n");
	PRT ("## sampleSize:\t0=>8bit, 1=>16bit\n");
	PRT ("## frameType:\t1=>keyframe, 2=>interframe, 3=>disposable_interframe, 4=>generated_keyframe, 5=>video_info/command_frame\n");
	PRT ("## avcPacketType:\t0=>avc_seq_header, 1=>avc_nalu, 2=>avc_seq_end\n");
	PRT ("## \n");

	return 0;
}
