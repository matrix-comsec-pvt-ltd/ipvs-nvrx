#if !defined INSTANT_PLAYBACKMEDIASTREAMER_H
#define INSTANT_PLAYBACKMEDIASTREAMER_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		InstantPlaybackMediaStreamer.h
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
#include "MediaStreamer.h"

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitInstantPlaybackStreamer(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e AddInstantPlaybackStream(CLIENT_CB_TYPE_e clientCbType, struct tm startTime, UINT32 cameraIndex, UINT8 sessionId,
                                          NW_CMD_REPLY_CB callBack, INT32 clientSocket, INSTANT_PLAYBACK_ID *playbackId);
//-------------------------------------------------------------------------------------------------
void RemoveInstantPlayBackForAllSessionId(void);
//-------------------------------------------------------------------------------------------------
void RemoveInstantPlayBackFromSessionId(UINT8 sessionId);
//-------------------------------------------------------------------------------------------------
void RemoveInstantPlayBackForSessCamId(UINT8 sessionId, UINT8 camIndex);
//-------------------------------------------------------------------------------------------------
void PauseResumeInstantPlayBackFromSessionId(UINT8 sessionId, BOOL isPausePlaybackF);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e PauseInstantPlaybackStream(INSTANT_PLAYBACK_ID instantPlayId, UINT8 sessionId, NW_CMD_REPLY_CB callback, INT32 streamFd);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e ResumeInstantPlaybackStream(INSTANT_PLAYBACK_ID instantPlayId, UINT8 sessionId, NW_CMD_REPLY_CB callback, INT32 streamFd);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e StopInstantPlaybackStream(INSTANT_PLAYBACK_ID instantPlayId, UINT8 sessionId, NW_CMD_REPLY_CB callback, INT32 streamFd);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e SetInstantPlaybackPosition(INSTANT_PLAYBACK_ID instantPlayId, UINT8 sessionId, NW_CMD_REPLY_CB callBack, INT32 streamFd,
                                            struct tm startTime, PLAYBACK_CMD_e direction, UINT32 audioChannel, UINT8 frmSyncNum);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif  // INSTANT_PLAYBACKMEDIASTREAMER_H
