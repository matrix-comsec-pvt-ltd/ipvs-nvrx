#ifndef MEDIASTREAMER_H_
#define MEDIASTREAMER_H_
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		MediaStreamer.h
@brief      This is header file for live and playback stream module.
            This module provides playback streaming functionality. Once the record to be played is
            selected; play, pause, resume, stop commands can be operated; to play the selected record,
            to pause the running stream, to resume paused stream, and to stop the stream, respectively.
            Record can be played in forward direction as well as in backward direction. User can also
            ask for single frame matching to specified time or very next or previous to it.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "DateTime.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define MAGIC_CODE				(0x000001FF)
/*
 *Header version 1 : default
 *Header version 2 : Change in fps field,
 *                   Now FPS will be current configured FPS of camera in case of VIDEO stream
 */
#define HEADER_VERSION			(2)
#define PRODUCT_TYPE			(1)
#define STREAM_REPLY_TIMEOUT 	(10)
#define FRAME_HEADER_LEN_MAX    sizeof(FRAME_HEADER_t)

#define	SEND_PACKET_TIMEOUT     5
#define	PB_MSG_QUEUE_MAX        10

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef UINT8 PLAYBACK_ID;
typedef UINT8 INSTANT_PLAYBACK_ID;

typedef enum
{
    PS_GET_PLAYBACK_ID = 0,
    PS_PLAY_STREAM,
    PS_SET_PLAY_POSITION,
    PS_PAUSE_STREAM,
    PS_RESUME_STREAM,
    PS_STEP_STREAM,
    PS_STOP_STREAM,
    PS_CLEAR_PLAYBACK_ID,
    PS_CLEAR_AND_RESTART_PLAYBACK,
    PS_PLAYBACK_SEQUENCE_MAX,
}PLAYBACK_STATE_e;

typedef enum
{
	MEDIA_NORMAL = 0,
	MEDIA_FILE_IO_ERROR,
	MEDIA_SYNC_START_INDICATOR,
    MEDIA_PLAYBACK_PROCESS_ERROR,
	MEDIA_FILE_RESERVED_4,
	MEDIA_HDD_FORMAT,
	MEDIA_CFG_CHANGE,
	MEDIA_PLAYBACK_OVER,
	MEDIA_PLAYBACK_SESSION_NOT_AVAIL,
	MEDIA_PLAYBACK_CAM_PREVILAGE,
	MEDIA_REC_DRIVE_CONFIG_CHANGE,
	MEDIA_OTHER_ERROR,
	MEDIA_STATUS_MAX
}MEDIA_STATUS_e;

typedef struct __attribute__((packed)) FRAME_HEADER_t
{
	UINT32 		magicCode;
	UINT8 		headerVersion;
	UINT8 		productType;
	UINT32 		mediaFrmLen;
	LocalTime_t localTime;
	UINT8 		chNo;				// Channel no
	UINT8 		streamType;			// Video / Audio
	UINT8 		codecType;			// Stream Codec type
    UINT8 		fps;				// FPS of current camera in video and sampline rate in audio
	UINT8 		frmType;			// Video frame type (I, P, B)
	UINT8 		vidResolution;		// Video resolution
	UINT8 		vidFormat;			// Video format
	UINT8 		scanType;			// Scan type
	UINT8 		mediaStatus;		// media status
	UINT8 		vidLoss;			// video loss is true/false
	UINT16 		audSampleFrq;		// audio sample freq
	UINT8 		noOfRefFrame;		// number of reference frame
	UINT8 		frmSyncNum;			// Frame Synchronisation number
	UINT8 		reserveByte[6];
	UINT32 		preReserverMediaLen;
}FRAME_HEADER_t;

typedef struct
{
    PLAYBACK_STATE_e		pbState;
    INT32 					cmdRespFd;
    MEDIA_STATUS_e		    responseCode;
    NW_CMD_REPLY_CB 		cmdRespCallback;
    struct tm 				stepTimeSec;
    UINT16 					stepTimemSec;
}PLAYBACK_MSG_t;

typedef struct
{
    UINT32					readIdx;
    UINT32					writeIdx;
    PLAYBACK_MSG_t			pbMsg[PB_MSG_QUEUE_MAX];
}PLAYBACK_MSG_QUE_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static const CHARPTR playbackSeqStr[PS_PLAYBACK_SEQUENCE_MAX] =
{
    "GET_PLAYBACK_ID",
    "PLAY_STREAM",
    "SET_PLAY_POSITION",
    "PAUSE_STREAM",
    "RESUME_STREAM",
    "STEP_STREAM",
    "STOP_STREAM",
    "CLEAR_PLAYBACK_ID",
    "CLEAR_AND_RESTART_PLAYBACK",
};

//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif /* MEDIASTREAMER_H_ */
