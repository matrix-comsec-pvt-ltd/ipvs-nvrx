#if !defined VIDEO_PARSER_H
#define VIDEO_PARSER_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		VideoParser.h
@brief      Video codec header paser (e.g NAL header parser etc.)
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
#ifdef __cplusplus
extern "C" {
#endif

/* OS Includes */
#include <sys/time.h>

/* Application Includes */
#include "MxTypedef.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
typedef struct
{
	FRAME_TYPE_e			frameType;
	UINT16					width;
	UINT16					height;
	UINT8					noOfRefFrame;

}VIDEO_INFO_t;

typedef struct
{
	STREAM_CODEC_TYPE_e		codecType;
	UINT32 					sampleRate;
	UINT32 					len;
	VIDEO_INFO_t			videoInfo;
	struct timeval			avPresentationTime;
    /* Added to check is callback in Camera Interface is from RTSP or HTTP according to callback in CI we take timestamp */
	UINT8 					isRTSP;
}MEDIA_FRAME_INFO_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
BOOL GetJpegSize(UINT8PTR data, UINT32 dataSize, VIDEO_INFO_t *videoInfo);
//-------------------------------------------------------------------------------------------------
BOOL GetH264Info(UINT8PTR frameBuf, UINT32 frameSize, VIDEO_INFO_t *videoInfo, UINT8PTR configPresent);
//-------------------------------------------------------------------------------------------------
BOOL GetH265Info(UINT8PTR frameBuf, UINT32 frameSize, VIDEO_INFO_t *videoInfo, UINT8PTR firstSlice);
//-------------------------------------------------------------------------------------------------
BOOL GetMpeg4Info(UINT8PTR frameBuf, UINT32 frameSize, VIDEO_INFO_t *videoInfo, UINT8PTR configPresent);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#ifdef __cplusplus
}
#endif
#endif // VIDEO_PARSER_H
