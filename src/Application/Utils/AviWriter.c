//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		AviWriter.c
@brief      File containing the defination of different API functions to handle AVI file.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "Utils.h"
#include "VideoParser.h"
#include "AudioParser.h"
#include "AviWriter.h"
#include "DebugLog.h"
#include "CameraDatabase.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define AVIF_HASINDEX           		0x00000010  // Index at end of file?
#define AVIF_MUSTUSEINDEX       		0x00000020
#define AVIF_ISINTERLEAVED      		0x00000100
#define AVIF_TRUSTCKTYPE        		0x00000800  // Use CKType to find key frames?
#define AVIF_WASCAPTUREFILE     		0x00010000
#define AVIIF_KEYFRAME      			0x00000010L // this frame is a key frame
#define SIZE_OF_FOURCC					4
#define DFLT_AUDIO_SAMPLE_FREQ			8000
#define AVI_COPY_FOURCC(dest, fourcc)	memcpy(dest, fourcc, SIZE_OF_FOURCC)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef struct
{
	CHAR	fcc[SIZE_OF_FOURCC];
	UINT32	cb;
	UINT32	dwMicroSecPerFrame;
	UINT32	dwMaxBytesPerSec;
	UINT32	dwPaddingGranularity;
	UINT32	dwFlags;
	UINT32	dwTotalFrames;
	UINT32	dwInitialFrames;
	UINT32	dwStreams;
	UINT32	dwSuggestedBufferSize;
	UINT32	dwWidth;
	UINT32	dwHeight;
	UINT32	dwReserved[4];

}AVI_MAIN_HEADER_t;

typedef struct
{
	CHAR	fcc[SIZE_OF_FOURCC];
	UINT32	cb;
	CHAR	fccType[SIZE_OF_FOURCC];
	CHAR	fccHandler[SIZE_OF_FOURCC];
	UINT32	dwFlags;
	UINT16	wPriority;
	UINT16	wLanguage;
	UINT32	dwInitialFrames;
	UINT32	dwScale;
	UINT32	dwRate;
	UINT32	dwStart;
	UINT32	dwLength;
	UINT32	dwSuggestedBufferSize;
	UINT32	dwQuality;
	UINT32	dwSampleSize;

	struct
	{
		UINT16	left;
		UINT16	top;
		UINT16	right;
		UINT16	bottom;

	}rcFrame;

}AVI_STREAM_HEADER_t;

typedef struct
{
	UINT32	biSize;
	UINT32	biWidth;
	UINT32	biHeight;
	UINT16	biPlanes;
	UINT16	biBitCount;
	CHAR	biCompression[SIZE_OF_FOURCC];
	UINT32	biSizeImage;
	UINT32	biXPelsPerMeter;
	UINT32	biYPelsPerMeter;
	UINT32	biClrUsed;
	UINT32	biClrImportant;

}BITMAP_INFO_HEADER_t;

typedef struct
{
	UINT16	wFormatTag;
	UINT16	nChannels;
	UINT32	nSamplesPerSec;
	UINT32	nAvgBytesPerSec;
	UINT16	nBlockAlign;
	UINT16	wBitsPerSample;
	UINT16	cbSize;
	UINT16	extraData;

}WAVE_FORMAT_t;

typedef struct
{
	CHAR 	ckId[SIZE_OF_FOURCC];	// chunk ID
	UINT32	flags;					// Flags
	UINT32	offset;
	UINT32	ckSize;

}AVI_INDEX_t;

typedef struct
{
	UINT8	codec;
	UINT32	sampleFreq;
    UINT32	totalFrames;
    BOOL	headerFilled;
    UINT16	aacConfig;

}AUDIO_PARAM_t;

typedef struct
{
	UINT8		codec;
	UINT8		resolution;
	BOOL		headerFilled;
    UINT32		totalFrames;
    LocalTime_t	firstFrameTime;
    LocalTime_t	lastFrameTime;
    UINT16		width;
    UINT16		height;

}VIDEO_PARAM_t;

typedef struct
{
	INT32			fd;				// fd of AVI File
	// This is to Keep track That avi file writing is abnormally terminated somewhere
	BOOL			abTerminationFlag;
	AUDIO_PARAM_t	audioParam;
	VIDEO_PARAM_t	videoParam;
	UINT32 			moviListEnd;	// MOVI LIST end
	UINT32 			riffEnd;		// RIFF LIST end
    UINT32			maxIndex;		// max AVI Indexes
    AVI_INDEX_t 	*indexPtr;		// AVI Index Pointer

}AVI_PRIV_t;

typedef struct
{

    CHAR					riff[SIZE_OF_FOURCC];   // Signature "RIFF"
    UINT32					headerSize;             // Data size (little-endian, low byte first)
    CHAR					avi[SIZE_OF_FOURCC];    // Signature "AVI " for AVI chunk

    CHAR					list1[SIZE_OF_FOURCC];  // RIFF list ("LIST")
    UINT32					list1Size;              // hdrl + main header + (list2 + size + strl + stream header + strf + size + bitmap header) + (list3 + size + strl + stream header + strf + size + wave header)

    CHAR					hdrl[SIZE_OF_FOURCC];   // Header list ("hdrl")
    AVI_MAIN_HEADER_t		aviMainHeader;          // AVI main header

    CHAR					list2[SIZE_OF_FOURCC];  // Video header list ("LIST")
    UINT32					list2Size;              // List size: "strl" + stream header + "strf" + size + video info header
    CHAR					strlVid[SIZE_OF_FOURCC];// Video stream List ("strl")
    AVI_STREAM_HEADER_t		streamHeaderVid;        // Video stream header
    CHAR					strf1[SIZE_OF_FOURCC];  // Video stream format ("strf")
    UINT32					strf1Size;              // Video info header size
    BITMAP_INFO_HEADER_t	bitmapInfoHeader;       // Video info header

    CHAR					list3[SIZE_OF_FOURCC];  // Audio header list ("LIST")
    UINT32					list3Size;              // List size: "strl" + stream header + "strf" + size + audio info header
    CHAR					strlAud[SIZE_OF_FOURCC];// Audio stream List ("strl")
    AVI_STREAM_HEADER_t		streamHeaderAud;        // Audio stream header
    CHAR					strf2[SIZE_OF_FOURCC];  // Audio stream format ("strf")
    UINT32					strf2Size;              // Audio info header size
    WAVE_FORMAT_t			waveHeader;             // Audio info header

    CHAR					list4[SIZE_OF_FOURCC];  // "movi" list ("LIST")
    UINT32					list4Size;              // Actual data length

    CHAR					movi[SIZE_OF_FOURCC];   // Tag "movi"

}AVI_HEADER_t; //328 bytes header

//#################################################################################################
// @GLOBAL VARIABLES
//#################################################################################################
static const CHARPTR videoCodecTag[MAX_VIDEO_CODEC] =
{
	"    ",
	"MJPG",
	"h264",
	"MP4V",
	"MJPG",
	"h265",
};

static const CHARPTR streamFourCC[MAX_STREAM_TYPE] =
{
	"00dc",
	"01wb"
};

static const UINT16 audioCodecTag[MAX_AUDIO_CODEC] =
{
	0x0000,		//	AUDIO_CODEC_NONE = 0,
    0x0007,		//	AUDIO_G711_ULAW
	0x0045,		//	AUDIO_G726_8
	0x0045,		//	AUDIO_G726_16
	0x0045,		//	AUDIO_G726_24
	0x0045,		//	AUDIO_G726_32
	0x0045,		//	AUDIO_G726_40
	0x00ff,		//	AUDIO_AAC
	0x0001,		//	AUDIO_PCM
	0x0001,		//	AUDIO_PCM
    0x0006,     //  AUDIO_G711_ALAW
};

static const UINT16 audioCodecBlockAlignVal[MAX_AUDIO_CODEC] =
{
	0,		//	AUDIO_CODEC_NONE = 0,
    1,		//	AUDIO_G711_ULAW
	1,		//	AUDIO_G726_8
	1,		//	AUDIO_G726_16
	1,		//	AUDIO_G726_24
	1,		//	AUDIO_G726_32
	1,		//	AUDIO_G726_40
	4,		//	AUDIO_AAC
	2,		//	AUDIO_PCM
    2,      //	AUDIO_PCM
    1      //  AUDIO_G711_ALAW
};

// Bits per sample values for all audio codecs
static const UINT16 audioCodecBitsPerSampleVal[MAX_AUDIO_CODEC] =
{
	0,		//	AUDIO_CODEC_NONE = 0,
    8,		//	AUDIO_G711_ULAW
	4,		//	AUDIO_G726_8
	2,		//	AUDIO_G726_16
	3,		//	AUDIO_G726_24
	4,		//	AUDIO_G726_32
	5,		//	AUDIO_G726_40
	16,		//	AUDIO_AAC
	16,		//	AUDIO_PCM
    16,     //	AUDIO_PCM
    8,      // AUDIO_G711_ALAW
};

// Audio Sampling Frequency
static const UINT16 audioCodecByteRateVal[MAX_AUDIO_CODEC] =
{
       0,		//	AUDIO_CODEC_NONE = 0,
    8000,		//	AUDIO_G711_ULAW
	1000,		//	AUDIO_G726_8
	2000,		//	AUDIO_G726_16
	3000,		//	AUDIO_G726_24
	4000,		//	AUDIO_G726_32
	5000,		//	AUDIO_G726_40
    8000,		//	AUDIO_AAC (NOTE: this table is not used for AAC)
	8000,		//	AUDIO_PCM
    8000,		//	AUDIO_PCM
    8000        //  AUDIO_G711_ALAW
};

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static BOOL AviWriteHeader(AVI_PRIV_t *aviHandle);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   Open AVI file for writting and initialize related variables
 * @param   aviFileName - Pointer to AVI file name
 * @return  Return AVI handle
 */
AVI_HANDLE AviOpen(CHARPTR aviFileName)
{
    AVI_PRIV_t      *priv;
    AVI_HEADER_t    aviHeader;

	// malloc memory to store AVI file information
	priv = (AVI_PRIV_t *)malloc(sizeof(AVI_PRIV_t));
    if (priv == NULL)
	{
        EPRINT(UTILS, "fail to alloc memory: [path=%s]", aviFileName);
        return NULL;
    }

    do
    {
        priv->indexPtr = NULL;
        priv->fd = open(aviFileName, CREATE_WRITE_MODE | O_TRUNC, USR_RW_GRP_RW_OTH_RW);
        if (priv->fd == INVALID_FILE_FD)
        {
            EPRINT(UTILS, "fail to open file: [path=%s], [err=%s]", aviFileName, STR_ERR);
            break;
        }

        priv->maxIndex = KILO_BYTE;
        priv->indexPtr = (AVI_INDEX_t *)malloc(sizeof(AVI_INDEX_t) * priv->maxIndex);
        if (priv->indexPtr == NULL)
        {
            EPRINT(UTILS, "fail to alloc memory: [path=%s]", aviFileName);
            break;
        }

        /* Write AVI header with 0 and will update actual value later on */
        memset(&aviHeader, 0, sizeof(aviHeader));

        // Write Default Header to AVI File
        if (write(priv->fd, &aviHeader, sizeof(AVI_HEADER_t)) != sizeof(AVI_HEADER_t))
        {
            EPRINT(UTILS, "fail to write default avi header in file: [path=%s], [err=%s]", aviFileName, STR_ERR);
            break;
        }

        priv->abTerminationFlag = FALSE;
        priv->audioParam.codec = AUDIO_CODEC_NONE;
        priv->audioParam.sampleFreq = DFLT_AUDIO_SAMPLE_FREQ;
        priv->audioParam.totalFrames = 0;
        priv->audioParam.headerFilled = FALSE;
        priv->videoParam.totalFrames = 0;
        priv->videoParam.headerFilled = FALSE;
        DPRINT(UTILS, "avi file created successfully: [path=%s]", aviFileName);
        return (VOIDPTR)priv;

    }while(0);

    /* Free memory and file resources if allocated */
    FREE_MEMORY(priv->indexPtr);
    CloseFileFd(&priv->fd);
    FREE_MEMORY(priv);
    return INVALID_AVI_HANDLE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Write Audio and Video frames in AVI file
 * @param   handle - Pointer to AVI handle
 * @param   fshData - Pointer to frame header
 * @param   frameData - Pointer to frame date
 * @param   frameLen - Frame data length
 * @return  Returns success and fail
 */
BOOL AviWrite(AVI_HANDLE handle, FSH_INFO_t *fshData, UINT8PTR frameData, UINT32 frameLen)
{
    AVI_PRIV_t      *priv = NULL;
    UINT32          currPos;
    VOIDPTR         tmpPtr;
    UINT32          flags;
    UINT32          tmpAacConfig = 0;
    VIDEO_INFO_t    videoInfo;
    UINT8           configPresent;

    do
    {
        if ((handle == NULL) || (fshData == NULL) || (frameData == NULL) || (fshData->mediaType >= MAX_STREAM_TYPE))
        {
            break;
        }

        // Sort- of Initialisation
        priv = (AVI_PRIV_t *)handle;
        currPos = lseek(priv->fd, 0, SEEK_CUR);
        if (fshData->mediaType == STREAM_TYPE_VIDEO)
        {
            if (priv->videoParam.headerFilled == FALSE)
            {
                // First frame must be an I frame for AVI file
                if ((fshData->codecType == VIDEO_H264) || (fshData->codecType == VIDEO_MPEG4) || (fshData->codecType == VIDEO_H265))
                {
                    if (fshData->vop != I_FRAME)
                    {
                        /* We have to discard this frame */
                        return SUCCESS;
                    }
                }

                if (0 == fshData->resolution)
                {
                    videoInfo.width = 0;
                    videoInfo.height = 0;
                    if(fshData->vop == I_FRAME)
                    {
                        switch(fshData->codecType)
                        {
                            case VIDEO_H264:
                                GetH264Info(frameData,frameLen, &videoInfo, &configPresent);
                                break;

                            case VIDEO_MPEG4:
                                GetMpeg4Info(frameData, frameLen, &videoInfo, &configPresent);
                                break;

                            case VIDEO_H265:
                                GetH265Info(frameData, frameLen, &videoInfo, &configPresent);
                                break;

                            case VIDEO_MJPG:
                                GetJpegSize(frameData, frameLen, &videoInfo);
                                break;

                            default:
                                break;
                        }
                    }

                    priv->videoParam.width = videoInfo.width;
                    priv->videoParam.height = videoInfo.height;
                }

                priv->videoParam.codec = fshData->codecType;
                priv->videoParam.resolution = fshData->resolution;
                priv->videoParam.firstFrameTime = fshData->localTime;
                priv->videoParam.headerFilled = TRUE;
            }

            if (fshData->codecType == VIDEO_MJPG)
            {
                flags = AVIIF_KEYFRAME;
            }
            else if (fshData->vop == I_FRAME)
            {
                flags = AVIIF_KEYFRAME;
            }
            else
            {
                flags = 0;
            }

            priv->videoParam.totalFrames++;
        }
        else
        {
            if (priv->audioParam.headerFilled == FALSE)
            {
                priv->audioParam.codec = fshData->codecType;
                priv->audioParam.sampleFreq = fshData->fps;
                if (fshData->codecType == AUDIO_AAC)
                {
                    GetAACAudioConfig(frameData, frameLen, &tmpAacConfig);
                    priv->audioParam.aacConfig = 0;
                    priv->audioParam.aacConfig |= ((tmpAacConfig & 0x0ff00) >> 8);
                    priv->audioParam.aacConfig |= ((tmpAacConfig & 0xff) << 8);
                }
                priv->audioParam.headerFilled = TRUE;
            }
            flags = AVIIF_KEYFRAME;
            priv->audioParam.totalFrames++;
        }

        // Fill Index chunk in Memory
        if ((priv->audioParam.totalFrames + priv->videoParam.totalFrames) >= priv->maxIndex)
        {
            priv->maxIndex += KILO_BYTE;
            tmpPtr = realloc((VOIDPTR)priv->indexPtr, (sizeof(AVI_INDEX_t) * priv->maxIndex));
            if (tmpPtr == NULL)
            {
                EPRINT(UTILS, "fail to alloc memory for avi indices");
                break;
            }

            priv->indexPtr = (AVI_INDEX_t *)tmpPtr;
        }

        // Fill Index Struct
        UINT32 frameCnt = priv->audioParam.totalFrames + priv->videoParam.totalFrames - 1;
        AVI_COPY_FOURCC(priv->indexPtr[frameCnt].ckId, streamFourCC[fshData->mediaType]);
        priv->indexPtr[frameCnt].flags = flags;
        priv->indexPtr[frameCnt].ckSize = frameLen;
        priv->indexPtr[frameCnt].offset = currPos;

        // write frame type Audio / Video
        if (write(priv->fd, streamFourCC[fshData->mediaType], SIZE_OF_FOURCC) != SIZE_OF_FOURCC)
        {
            break;
        }

        if (frameLen == 0)
        {
            EPRINT(UTILS, "invld frame found");
            break;
        }

        // write frame length
        if (write(priv->fd, &frameLen, sizeof(UINT32)) != (INT32)sizeof(UINT32))
        {
            break;
        }

        // Need to swap data in case of AUDIO_PCM_L
        if ((fshData->mediaType == STREAM_TYPE_AUDIO) && (fshData->codecType == AUDIO_PCM_L))
        {
            UINT8 tmpData;
            UINT32 loop;

            // For Odd Frame Length , shouldn't happen
            frameLen /= 2;
            frameLen *= 2;

            for (loop = 0; loop < frameLen; loop += 2)
            {
                if(frameData != NULL)
                {
                    tmpData = frameData[loop];
                    frameData[loop] = frameData[loop + 1];
                    frameData[loop + 1] = tmpData;
                }
            }
        }

        // write frame
        if (write(priv->fd, frameData, frameLen) != (INT32)frameLen)
        {
            break;
        }

        // frameLen must be even number of bytes
        if (frameLen & 0x01)
        {
            // pad 1 Byte
            if (write(priv->fd, "0", 1) == 1)
            {
                priv->videoParam.lastFrameTime = fshData->localTime;
                return SUCCESS;
            }
        }
        else
        {
            priv->videoParam.lastFrameTime = fshData->localTime;
            return SUCCESS;
        }

    }while(0);

    if (priv != NULL)
    {
        priv->abTerminationFlag = TRUE;
        EPRINT(UTILS, "fail to write frame to file");
    }
    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Close AVI file
 * @param   handle - Pointer to AVI handle
 * @return  Returns success and fail
 */
BOOL AviClose(AVI_HANDLE handle)
{
	AVI_PRIV_t	*priv;
	BOOL		retVal = FAIL;
	UINT32		wCount;

    if (handle == NULL)
	{
        return FAIL;
    }

    priv = (AVI_PRIV_t *)handle;

    // Check if there was an error in file writing
    if (priv->abTerminationFlag == FALSE)
    {
        priv->moviListEnd = lseek(priv->fd, 0, SEEK_CUR);

        // Write AVI indexes
        if (write(priv->fd, "idx1", SIZE_OF_FOURCC) == SIZE_OF_FOURCC)
        {
            // Get Size of Indexes
            wCount = (sizeof(AVI_INDEX_t) * (priv->audioParam.totalFrames + priv->videoParam.totalFrames));
            if (write(priv->fd, &wCount, 4) < 4)
            {
                EPRINT(UTILS, "fail to write file: [err=%s]", STR_ERR);
            }
            else if (write(priv->fd, priv->indexPtr, wCount) < (INT32)wCount)
            {
                EPRINT(UTILS, "fail to write indices to file: [err=%s]", STR_ERR);
            }
            else
            {
                priv->riffEnd = lseek(priv->fd, 0, SEEK_CUR);

                // dump Avi File Header
                if (AviWriteHeader(priv) == SUCCESS)
                {
                    retVal = SUCCESS;
                    DPRINT(UTILS, "avi file closed successfully");
                }
            }
        }
        else
        {
            EPRINT(UTILS, "fail to write avi indx1 to file: [err=%s]", STR_ERR);
        }
    }
    else
    {
        DPRINT(UTILS, "closing avi file without dumping header & index");
    }

    // Sync file Before Closing
    fsync(priv->fd);

    // close AVi File
    close(priv->fd);

    // Free Allocated Memory for Storing Avi Indexes
    FREE_MEMORY(priv->indexPtr);

    // Free Session related data
    free(priv);
	return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Write AVI header on file close
 * @param   priv - Pointer to AVI handle
 * @return  Returns success and fail
 */
static BOOL AviWriteHeader(AVI_PRIV_t *priv)
{
    BOOL            status = FAIL;
    UINT16          width = 0, height = 0;
    CHAR 			resolution[MAX_RESOLUTION_NAME_LEN];
    AVI_HEADER_t    aviHeader;

    if (FALSE == priv->videoParam.headerFilled)
	{
        EPRINT(UTILS, "no streams found for avi header");
        return FAIL;
    }

    /* It must contain tag RIFF at the beginning of the file */
    AVI_COPY_FOURCC(aviHeader.riff, "RIFF");

    // Group Size
    aviHeader.headerSize = (priv->riffEnd - (SIZE_OF_FOURCC * 2));

    // Type
    AVI_COPY_FOURCC(aviHeader.avi, "AVI ");

    // LIST : 'hdrl'
    AVI_COPY_FOURCC(aviHeader.list1, "LIST");

    // Group Size ; AVI Main + Video + Audio
    aviHeader.list1Size = sizeof(AVI_HEADER_t) - 32;

    // Type
    AVI_COPY_FOURCC(aviHeader.hdrl, "hdrl");

    // avi
    AVI_COPY_FOURCC(aviHeader.aviMainHeader.fcc, "avih");
    aviHeader.aviMainHeader.cb = (sizeof(AVI_MAIN_HEADER_t) - (SIZE_OF_FOURCC * 2));
    aviHeader.aviMainHeader.dwMicroSecPerFrame = 0;
    aviHeader.aviMainHeader.dwMaxBytesPerSec = -1;
    aviHeader.aviMainHeader.dwPaddingGranularity = 0;
    aviHeader.aviMainHeader.dwFlags = (AVIF_HASINDEX | AVIF_TRUSTCKTYPE);
    aviHeader.aviMainHeader.dwTotalFrames = (priv->audioParam.totalFrames + priv->videoParam.totalFrames);
    aviHeader.aviMainHeader.dwInitialFrames = 0;
    aviHeader.aviMainHeader.dwStreams = 2;		// Streams : 2(Video & Audio)
    aviHeader.aviMainHeader.dwSuggestedBufferSize = MEGA_BYTE;

    /* Get resolution width and height */
    if ((priv->videoParam.resolution != 0) && (priv->videoParam.resolution < MAX_RESOLUTION))
    {
        /* Get resolution string from id */
        GetResolutionString(resolution, priv->videoParam.resolution);

        /* Get resolution height and width from string */
        status = GetResolutionHeightWidth(resolution, &height, &width);
    }

    if (status == SUCCESS)
    {
        aviHeader.aviMainHeader.dwHeight = height;
        aviHeader.aviMainHeader.dwWidth = width;
    }
    else
    {
        aviHeader.aviMainHeader.dwHeight = priv->videoParam.height;
        aviHeader.aviMainHeader.dwWidth  = priv->videoParam.width;
    }

    aviHeader.aviMainHeader.dwReserved[0] = 0;
    aviHeader.aviMainHeader.dwReserved[1] = 0;
    aviHeader.aviMainHeader.dwReserved[2] = 0;
    aviHeader.aviMainHeader.dwReserved[3] = 0;

    //  LIST : 'strl' - Video Stream Info
    AVI_COPY_FOURCC(aviHeader.list2, "LIST");

    aviHeader.list2Size = ((SIZE_OF_FOURCC * 3) + sizeof(AVI_STREAM_HEADER_t) + sizeof(BITMAP_INFO_HEADER_t));

    // List Type
    AVI_COPY_FOURCC(aviHeader.strlVid, "strl");

    // stream header : VIDEO
    // strh's fourcc
    AVI_COPY_FOURCC(aviHeader.streamHeaderVid.fcc, "strh");
    // strh's size
    aviHeader.streamHeaderVid.cb = (sizeof(AVI_STREAM_HEADER_t) - (SIZE_OF_FOURCC * 2));

    // Identifier : vids
    AVI_COPY_FOURCC(aviHeader.streamHeaderVid.fccType, "vids");

    AVI_COPY_FOURCC(aviHeader.streamHeaderVid.fccHandler, videoCodecTag[priv->videoParam.codec]);

    aviHeader.streamHeaderVid.dwFlags = 0;
    aviHeader.streamHeaderVid.wPriority = 0;
    aviHeader.streamHeaderVid.wLanguage = 0;
    aviHeader.streamHeaderVid.dwInitialFrames = 0;
    aviHeader.streamHeaderVid.dwScale = ((((priv->videoParam.lastFrameTime.totalSec * 1000) +  priv->videoParam.lastFrameTime.mSec))
                                         - (((priv->videoParam.firstFrameTime.totalSec * 1000) + priv->videoParam.firstFrameTime.mSec)));
    aviHeader.streamHeaderVid.dwRate = (priv->videoParam.totalFrames * 1000);
    aviHeader.streamHeaderVid.dwStart = 0;
    aviHeader.streamHeaderVid.dwLength = priv->videoParam.totalFrames;
    aviHeader.streamHeaderVid.dwSuggestedBufferSize = MEGA_BYTE;
    aviHeader.streamHeaderVid.dwQuality = -1;
    aviHeader.streamHeaderVid.dwSampleSize = 0;
    aviHeader.streamHeaderVid.rcFrame.top = 0;
    aviHeader.streamHeaderVid.rcFrame.left = 0;

    if (status == SUCCESS)
    {
        aviHeader.streamHeaderVid.rcFrame.bottom = height;
        aviHeader.streamHeaderVid.rcFrame.right = width;
    }
    else
    {
        aviHeader.streamHeaderVid.rcFrame.bottom = priv->videoParam.width;
        aviHeader.streamHeaderVid.rcFrame.right  = priv->videoParam.height;
    }

    // stream format : Bitmap
    AVI_COPY_FOURCC(aviHeader.strf1, "strf");

    // 'strf' size
    aviHeader.strf1Size = sizeof(BITMAP_INFO_HEADER_t);

    aviHeader.bitmapInfoHeader.biSize = sizeof(BITMAP_INFO_HEADER_t);

    if (status == SUCCESS)
    {
        aviHeader.bitmapInfoHeader.biWidth = width;
        aviHeader.bitmapInfoHeader.biHeight = height;
    }
    else
    {
        aviHeader.bitmapInfoHeader.biWidth = priv->videoParam.width;
        aviHeader.bitmapInfoHeader.biHeight  = priv->videoParam.height;
    }

    aviHeader.bitmapInfoHeader.biPlanes = 1;
    aviHeader.bitmapInfoHeader.biBitCount = 24;

    AVI_COPY_FOURCC(aviHeader.bitmapInfoHeader.biCompression, videoCodecTag[priv->videoParam.codec]);

    if (status == SUCCESS)
    {
        aviHeader.bitmapInfoHeader.biSizeImage = (width * height * 3);
    }
    else
    {
        aviHeader.bitmapInfoHeader.biSizeImage = (priv->videoParam.width * priv->videoParam.height * 3);
    }
    aviHeader.bitmapInfoHeader.biXPelsPerMeter = 0;
    aviHeader.bitmapInfoHeader.biYPelsPerMeter = 0;
    aviHeader.bitmapInfoHeader.biClrUsed = 0;
    aviHeader.bitmapInfoHeader.biClrImportant = 0;

    /* To apply all audio parameters accurate, we should derive them from SDP rather than take them
     * hardcodec. e.g.: Number of channels, Sampling frequency etc. */
    // List
    AVI_COPY_FOURCC(aviHeader.list3, "LIST");

    // List Total size
    aviHeader.list3Size = ((SIZE_OF_FOURCC * 3) + sizeof(AVI_STREAM_HEADER_t) + sizeof(WAVE_FORMAT_t));

    // List Type
    AVI_COPY_FOURCC(aviHeader.strlAud, "strl");

    // stream header : AUDIO
    AVI_COPY_FOURCC(aviHeader.streamHeaderAud.fcc, "strh");
    aviHeader.streamHeaderAud.cb = (sizeof(AVI_STREAM_HEADER_t) - (SIZE_OF_FOURCC * 2));

    // stream header 'auds'
    AVI_COPY_FOURCC(aviHeader.streamHeaderAud.fccType, "auds");

    aviHeader.streamHeaderAud.fccHandler[3] = 0x00;
    aviHeader.streamHeaderAud.fccHandler[2] = 0x00;
    aviHeader.streamHeaderAud.fccHandler[1] = 0x00;
    aviHeader.streamHeaderAud.fccHandler[0] = 0x01;

    aviHeader.streamHeaderAud.dwFlags = 0;
    aviHeader.streamHeaderAud.wPriority = 0;
    aviHeader.streamHeaderAud.wLanguage = 0;
    aviHeader.streamHeaderAud.dwInitialFrames = 0;

    if (priv->audioParam.codec == AUDIO_AAC)
    {
        /* AAC is not working properly with VLC and WMP but it is working with format factory */
        aviHeader.streamHeaderAud.dwScale = 8;
        aviHeader.streamHeaderAud.dwRate = priv->audioParam.sampleFreq / 128;
        aviHeader.streamHeaderAud.dwSampleSize = 0;
    }
    else if ((priv->audioParam.codec >= AUDIO_G726_8) && (priv->audioParam.codec <= AUDIO_G726_40))
    {
        aviHeader.streamHeaderAud.dwScale = audioCodecBlockAlignVal[priv->audioParam.codec];
        aviHeader.streamHeaderAud.dwRate = audioCodecByteRateVal[priv->audioParam.codec];
        aviHeader.streamHeaderAud.dwSampleSize = audioCodecBlockAlignVal[priv->audioParam.codec];
    }
    else if ((priv->audioParam.codec == AUDIO_PCM_B) || (priv->audioParam.codec == AUDIO_PCM_L))
    {
        aviHeader.streamHeaderAud.dwScale = audioCodecBlockAlignVal[priv->audioParam.codec];
        aviHeader.streamHeaderAud.dwRate = audioCodecBlockAlignVal[priv->audioParam.codec]
                                                       * priv->audioParam.sampleFreq;
        aviHeader.streamHeaderAud.dwSampleSize = audioCodecBlockAlignVal[priv->audioParam.codec];
    }
    else
    {
        aviHeader.streamHeaderAud.dwScale = audioCodecBlockAlignVal[priv->audioParam.codec] * 8;
        aviHeader.streamHeaderAud.dwRate = priv->audioParam.sampleFreq * 8;
        aviHeader.streamHeaderAud.dwSampleSize = audioCodecBlockAlignVal[priv->audioParam.codec];
    }

    // below are parameters with default values
    aviHeader.streamHeaderAud.dwStart = 0;
    aviHeader.streamHeaderAud.dwLength = priv->audioParam.totalFrames;
    aviHeader.streamHeaderAud.dwSuggestedBufferSize = 12 * KILO_BYTE;
    aviHeader.streamHeaderAud.dwQuality = -1;
    aviHeader.streamHeaderAud.rcFrame.top = 0;
    aviHeader.streamHeaderAud.rcFrame.left = 0;
    aviHeader.streamHeaderAud.rcFrame.right = 0;
    aviHeader.streamHeaderAud.rcFrame.bottom = 0;

    //  stream format : WAVE
    AVI_COPY_FOURCC(aviHeader.strf2, "strf");

    // 'strf' size
    aviHeader.strf2Size = sizeof(WAVE_FORMAT_t);

    aviHeader.waveHeader.wFormatTag = audioCodecTag[priv->audioParam.codec];
    aviHeader.waveHeader.nSamplesPerSec = priv->audioParam.sampleFreq;
    aviHeader.waveHeader.wBitsPerSample = audioCodecBitsPerSampleVal[priv->audioParam.codec];

    // Fill extra data for AAC Codec
    if (priv->audioParam.codec == AUDIO_AAC)
    {
        aviHeader.waveHeader.nChannels = 2;
        aviHeader.waveHeader.nAvgBytesPerSec = priv->audioParam.sampleFreq / audioCodecBlockAlignVal[priv->audioParam.codec];
        aviHeader.waveHeader.nBlockAlign = 768 * aviHeader.waveHeader.nChannels;
        aviHeader.waveHeader.cbSize = 2;
        aviHeader.waveHeader.extraData = priv->audioParam.aacConfig;
    }
    else
    {
        aviHeader.waveHeader.nChannels = 1;
        aviHeader.waveHeader.nAvgBytesPerSec = audioCodecByteRateVal[priv->audioParam.codec];
        aviHeader.waveHeader.nBlockAlign = audioCodecBlockAlignVal[priv->audioParam.codec];
        aviHeader.waveHeader.cbSize = 0;
        aviHeader.waveHeader.extraData = 0;
    }

    // List
    AVI_COPY_FOURCC(aviHeader.list4, "LIST");

    // File 'movi' list size
    aviHeader.list4Size = (priv->moviListEnd - sizeof(AVI_HEADER_t) + SIZE_OF_FOURCC);

    // List Type : movi
    AVI_COPY_FOURCC(aviHeader.movi, "movi");

    lseek(priv->fd, 0, SEEK_SET);

    if (write(priv->fd, &aviHeader, sizeof(AVI_HEADER_t)) != (INT32)sizeof(AVI_HEADER_t))
    {
        EPRINT(UTILS, "fail to write header to avi file: [err=%s]", STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
