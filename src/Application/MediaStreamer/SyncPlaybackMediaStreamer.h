#if !defined SYNC_PLAYBACKMEDIASTREAMER_H
#define SYNC_PLAYBACKMEDIASTREAMER_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		SyncPlaybackMediaStreamer.h
@brief      This module provides playback streaming functionality. Once the record to be played is
            selected; play, pause, resume, stop commands can be operated; to play the selected record,
            to pause the running stream, to resume paused stream, and to stop the stream, respectively.
            Record can be played in forward direction as well as in backward direction. User can also
            ask for single frame matching to specified time or very next or previous to it.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "DiskManager.h"

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef struct
{
    UINT8               totalCamera;
    BOOL				cameraRecordF[MAX_CAMERA];
    BOOL				cameraAudioF[MAX_CAMERA];
    PLAYBACK_CMD_e      direction;
    EVENT_e             eventType;
    UINT8               frmSyncNum;
    UINT8               speed;
    NW_FRAME_TYPE_e     frameType;
    RECORD_ON_DISK_e    recStorageDrive;
}SYNC_PLAY_PARAMS_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitSyncPlaybackStreamer(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e AddSyncPlaybackStream(SYNC_PLAY_PARAMS_t *pSyncPlayParam, struct tm *startTime, CLIENT_CB_TYPE_e clientCbType,
                                       UINT8 sessionId, UINT8 userIndex, NW_CMD_REPLY_CB callBack, INT32 clientSocket);
//-------------------------------------------------------------------------------------------------
void RemoveSyncPlayBackForAllSessionId(DISK_ACT_e diskStatus);
//-------------------------------------------------------------------------------------------------
void RemoveSyncPlayBackFromSessionId(UINT8 sessionId, DISK_ACT_e diskStatus);
//-------------------------------------------------------------------------------------------------
void PauseResumeSyncPlayBackFromSessionId(UINT8 sessionId, BOOL isPausePlaybackF);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e PauseSyncPlaybackStream(UINT8 sessionId, NW_CMD_REPLY_CB callback, INT32 clientSocket);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e ResumeSyncPlaybackStream(UINT8 sessionId, NW_CMD_REPLY_CB callback, INT32 clientSocket);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e StopSyncPlaybackStream(UINT8 sessionId, NW_CMD_REPLY_CB callback, INT32 clientSocket);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e SetSyncPlaybackPosition(SYNC_PLAY_PARAMS_t *pSyncPlayParam, struct tm *startTime,
                                         UINT8 sessionId, NW_CMD_REPLY_CB callBack, INT32 clientSocket);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e RemoveSyncPlaybackStream(UINT8 sessionId, NW_CMD_REPLY_CB callback, INT32 clientSocket, DISK_ACT_e diskStatus);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif  // PLAYBACKMEDIASTREAMER_H
