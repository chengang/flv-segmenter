// 
// flv.c
// 
// Dump Adobe flv file Info per frame.
// 
// Author:Chen Gang
// Blog: http://blog.yikuyiku.com
// Corp:SINA
// At 2013/4/11 Beijing
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

#define FLV_VIDEO_AVC	7
#define FLV_AUDIO_AAC	10

#define KEY_FRAME 1

#define FLV_UI24(x) (unsigned int)(((*(x)) << 16) + ((*(x + 1)) << 8) + (*(x + 2)))
#define FLV_UI8(x) (unsigned int)(*(x))

int tagId = 0;
int videoTagId = 0;
int audioTagId = 0;
int if_avc = 0;
int if_aac = 0;
int outId = 0;
char outFileName[50];
FILE *out = NULL;
char inFileName[50];
int avc_0_frame_size;
int aac_0_frame_size;
unsigned char *avc_0_frame;
unsigned char *aac_0_frame;
unsigned char FLVHeader[FLV_SIZE_HEADER + FLV_SIZE_PREVIOUSTAGSIZE];

typedef struct
{
    unsigned int tagType;
    int dataSize;
    short keyframe;
} FLVTag_t;

int readFLVTag(FILE * fp)
{
    int rv;
    FLVTag_t flvTag;

    unsigned char tagHeader[FLV_SIZE_TAGHEADER];
    unsigned char preTagSize[FLV_SIZE_PREVIOUSTAGSIZE];

    rv = fread(tagHeader, FLV_SIZE_TAGHEADER, 1, fp);
    if (rv < 1)
    {
        return READ_ERROR;
    }

    tagId++;

    flvTag.tagType = FLV_UI8(tagHeader);
    flvTag.dataSize = FLV_UI24(&tagHeader[1]);

    unsigned char data[flvTag.dataSize];
    int frameType, codecId;

    fread(&data, flvTag.dataSize, 1, fp);
    if (flvTag.tagType == FLV_TAG_AUDIO)
    {
        audioTagId++;
        codecId = (data[0] >> 4) & 0xf;
        if (codecId == FLV_AUDIO_AAC)
        {
            if_aac = 1;
            if (audioTagId == 1)
            {
                aac_0_frame_size =
                    FLV_SIZE_TAGHEADER + flvTag.dataSize +
                    FLV_SIZE_PREVIOUSTAGSIZE;
                aac_0_frame =
                    malloc(FLV_SIZE_TAGHEADER + flvTag.dataSize +
                           FLV_SIZE_PREVIOUSTAGSIZE);
                memset(aac_0_frame, 0, aac_0_frame_size);
                memcpy(aac_0_frame, tagHeader, FLV_SIZE_TAGHEADER);
                memcpy(aac_0_frame + FLV_SIZE_TAGHEADER, data,
                       flvTag.dataSize);
                memcpy(aac_0_frame + FLV_SIZE_TAGHEADER + flvTag.dataSize,
                       preTagSize, FLV_SIZE_PREVIOUSTAGSIZE);
            }
        }
    }
    else if (flvTag.tagType == FLV_TAG_VIDEO)
    {
        videoTagId++;
        frameType = (data[0] >> 4) & 0xf;
        codecId = data[0] & 0xf;
        if (codecId == FLV_VIDEO_AVC)
        {
            if_avc = 1;
            if (videoTagId == 1)
            {
                avc_0_frame_size =
                    FLV_SIZE_TAGHEADER + flvTag.dataSize +
                    FLV_SIZE_PREVIOUSTAGSIZE;
                avc_0_frame =
                    malloc(FLV_SIZE_TAGHEADER + flvTag.dataSize +
                           FLV_SIZE_PREVIOUSTAGSIZE);
                memset(avc_0_frame, 0, avc_0_frame_size);
                memcpy(avc_0_frame, tagHeader, FLV_SIZE_TAGHEADER);
                memcpy(avc_0_frame + FLV_SIZE_TAGHEADER, data,
                       flvTag.dataSize);
                memcpy(avc_0_frame + FLV_SIZE_TAGHEADER + flvTag.dataSize,
                       preTagSize, FLV_SIZE_PREVIOUSTAGSIZE);
            }
        }
    }

    fread(preTagSize, FLV_SIZE_PREVIOUSTAGSIZE, 1, fp);

    if (!if_avc || tagId > 3)
    {
        if (flvTag.tagType == FLV_TAG_VIDEO && frameType == KEY_FRAME)
        {
            outId++;
            if (out != NULL)
            {
                fclose(out);
            }
            sprintf(outFileName, "%s_%d.flv", inFileName, outId);
            out = fopen(outFileName, "w");
            fwrite(FLVHeader, FLV_SIZE_HEADER + FLV_SIZE_PREVIOUSTAGSIZE, 1,
                   out);
            if (if_avc)
            {
                fwrite(avc_0_frame, avc_0_frame_size, 1, out);
            }
            if (if_aac)
            {
                fwrite(aac_0_frame, aac_0_frame_size, 1, out);
            }

        }
        if (out != NULL)
        {
            fwrite(tagHeader, FLV_SIZE_TAGHEADER, 1, out);
            fwrite(data, flvTag.dataSize, 1, out);
            fwrite(preTagSize, FLV_SIZE_PREVIOUSTAGSIZE, 1, out);
        }
    }

    return OK;
}

int main(int argc, char *argv[])
{
    FILE *fp = NULL;
    strcpy(inFileName, argv[1]);

    fp = fopen(argv[1], "r");
    fread(FLVHeader, FLV_SIZE_HEADER + FLV_SIZE_PREVIOUSTAGSIZE, 1, fp);

    while (readFLVTag(fp) == OK)
    {
    }
    fclose(fp);
    fclose(out);

    free(avc_0_frame);
    free(aac_0_frame);

    return OK;
}
