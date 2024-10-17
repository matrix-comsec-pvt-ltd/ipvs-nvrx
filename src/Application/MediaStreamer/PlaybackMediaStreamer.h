#if !defined PLAYBACKMEDIASTREAMER_H
#define PLAYBACKMEDIASTREAMER_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		PlaybackMediaStreamer.h
@brief      This module provides playback streaming functionality. Once the record to be played is
            selected; play, pause, resume, stop commands can be operated; to play the selected
            record, to pause the running stream, to resume paused stream, and to stop the stream,
            respectively. Record can be played in forward direction as well as in backward direction.
            User can also ask for single frame matching to specified time or very next or previous to it.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "DiskManager.h"
#include "MediaStreamer.h"

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitPlaybackStreamer(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e AddPlaybackStream(CLIENT_CB_TYPE_e clientCbType, struct tm startTime, struct tm endTime, UINT8 cameraIndex,  UINT8 sessionId,
                                   EVENT_e eventType, UINT8 overlapIndex, UINT8 diskId, RECORD_ON_DISK_e recStorageType, PLAYBACK_ID *pPlaybackId);
//-------------------------------------------------------------------------------------------------
void RemovePlayBackForAllSessionId(void);
//-------------------------------------------------------------------------------------------------
void RemovePlayBackFromSessionId(UINT8 sessionId);
//-------------------------------------------------------------------------------------------------
void RemovePlayBackForSessCamId(UINT8 sessionId, UINT8 camIndex);
//-------------------------------------------------------------------------------------------------
void PauseResumePlaybackFromSessionId(UINT8 sessionId, BOOL isPausePlaybackF);
//-------------------------------------------------------------------------------------------------
BOOL RemovePlaybackStream(PLAYBACK_ID streamId, UINT8 sessionId, NW_CMD_REPLY_CB callback, INT32 streamFd);
//-------------------------------------------------------------------------------------------------
BOOL StartPlaybackStream(PLAYBACK_ID streamId, UINT8 sessionId, PLAYBACK_CMD_e direction, struct tm timeInstance,
                         BOOL audio, UINT8 speed, NW_CMD_REPLY_CB callback, INT32 streamFd);
//-------------------------------------------------------------------------------------------------
BOOL StopPlaybackStream(PLAYBACK_ID streamId, UINT8 sessionId, NW_CMD_REPLY_CB callback, INT32 streamFd);
//-------------------------------------------------------------------------------------------------
BOOL PausePlaybackStream(PLAYBACK_ID streamId, UINT8 sessionId, NW_CMD_REPLY_CB callback, INT32 streamFd);
//-------------------------------------------------------------------------------------------------
BOOL ResumePlaybackStream(PLAYBACK_ID streamId, UINT8 sessionId, NW_CMD_REPLY_CB callback, INT32 streamFd);
//-------------------------------------------------------------------------------------------------
BOOL StepPlaybackStream(PLAYBACK_ID streamId, UINT8 sessionId, PLAYBACK_CMD_e direction, NW_FRAME_TYPE_e frameType,
                        struct tm timeInstance, UINT16 mSec, NW_CMD_REPLY_CB callback, INT32 streamFd);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif  // PLAYBACKMEDIASTREAMER_H
