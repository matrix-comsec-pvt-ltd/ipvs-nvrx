//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		PlaybackMediaStreamer.c
@brief      This module provides play-back streaming functionalities. User can select a record from
            the record list returned. This selected record is given an ID and play-back session in
            disk manager is opened. When play command is received, a thread, which actually does the
            streaming, is signaled. This thread keeps on sending the frames to user until pause or
            stop command is not received. Upon receipt of pause or stop command, streaming loop is
            broken and streaming is stopped. After stop, if resume command is received then streaming
            is started from the same time instance, where it was stopped or if new play command is
            received then streaming is started from new time instance specified. Record can also be
            played in steps, i.e. frame by frame, one at a time. When streaming needs to be closed,
            all parameter stored against stream ID is flushed and stream ID is freed. Any use of
            stream ID without assigning value to it will be invalid.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "PlaybackMediaStreamer.h"
#include "Utils.h"
#include "SysTimer.h"
#include "DebugLog.h"
#include "NetworkManager.h"
#include "NetworkCommand.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
/* Use Default Stack Size*/
#define PLAYBACK_THREAD_STACK_SZ    (0*MEGA_BYTE)
#define PB_PKT_SEND_TIMEOUT_IN_US   1000

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef struct
{
    BOOL 					request;
    BOOL					audio;
    UINT8					speed;
    UINT8					clientCbType;
    UINT8					pbClientIndex;
    UINT8 					cameraIndex;
    UINT8			 		clientSessId;
    PLAY_SESSION_ID 		dmPlaySessId;
    PLAYBACK_CMD_e 			direction;
    NW_FRAME_TYPE_e 		frameType;
    pthread_mutex_t 		playBackListMutex;
    PLAYBACK_MSG_QUE_t		pbMagQue;
    pthread_mutex_t 		streamSignalLock;
    pthread_cond_t 			streamSignal;
    UINT8PTR 				frameBuffer;
}PLAYBACK_INFO_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static PLAYBACK_INFO_t playbackList[MAX_CAMERA];
static pthread_mutex_t playbackListLock;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static VOIDPTR playbackStreamerThread(void *threadArg);
//-------------------------------------------------------------------------------------------------
static BOOL writePbMesg(UINT8 pbIndex, PLAYBACK_MSG_t * pbMsgPtr);
//-------------------------------------------------------------------------------------------------
static MEDIA_STATUS_e readyFrameToSend(PLAYBACK_INFO_t * infoPtr, UINT32PTR length);
//-------------------------------------------------------------------------------------------------
static BOOL sendPlaybackFrameHeader(PLAYBACK_INFO_t *pAsyncPbInfo, INT32 sockFd, MEDIA_STATUS_e mediaStatus);
//-------------------------------------------------------------------------------------------------
static void sendPlaybackCmdResp(PLAYBACK_MSG_t *pPbMsg, NET_CMD_STATUS_e cmdResp, BOOL closeSockF);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API initializes the play-back media streamer module.
 */
void InitPlaybackStreamer(void)
{
    UINT8 sessionIndex;

    // Initialize locks and signals
    MUTEX_INIT(playbackListLock, NULL);

    for(sessionIndex = 0; sessionIndex < MAX_PLAYBACK_SESSION; sessionIndex++)
    {
        // Initialize stream info parameter
        playbackList[sessionIndex].cameraIndex = getMaxCameraForCurrentVariant();
        playbackList[sessionIndex].clientSessId = MAX_NW_CLIENT;
        playbackList[sessionIndex].clientCbType = CLIENT_CB_TYPE_MAX;
        playbackList[sessionIndex].pbClientIndex = sessionIndex;
        playbackList[sessionIndex].request = FREE;
        playbackList[sessionIndex].pbMagQue.readIdx = 0;
        playbackList[sessionIndex].pbMagQue.writeIdx = 0;

        MUTEX_INIT(playbackList[sessionIndex].playBackListMutex, NULL);
        MUTEX_INIT(playbackList[sessionIndex].streamSignalLock, NULL);
        pthread_cond_init(&playbackList[sessionIndex].streamSignal, NULL);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API adds request to the play-back stream list, if free any, and outputs the streamID.
 * @param   startTime
 * @param   endTime
 * @param   cameraIndex
 * @param   sessionId
 * @param   eventType
 * @param   overlapIndex
 * @param   diskId
 * @param   recStorageType
 * @param   pPlaybackId
 * @return
 */
NET_CMD_STATUS_e AddPlaybackStream(CLIENT_CB_TYPE_e clientCbType, struct tm startTime, struct tm endTime, UINT8 cameraIndex,  UINT8 sessionId,
                                   EVENT_e eventType, UINT8 overlapIndex, UINT8 diskId, RECORD_ON_DISK_e recStorageType, PLAYBACK_ID *pPlaybackId)
{
    BOOL                retVal;
    UINT8               sessionIndex;
    PLAYBACK_MSG_t      pbMessg;
    PLAY_CNTRL_INFO_t   playCntrlInfo;

    /* Validate input parameters */
    if ((cameraIndex >= getMaxCameraForCurrentVariant()) || (NULL == pPlaybackId))
    {
        return CMD_PROCESS_ERROR;
    }

    /* Allocate playback id */
    MUTEX_LOCK(playbackListLock);
    for(sessionIndex = 0; sessionIndex < MAX_PLAYBACK_SESSION; sessionIndex++)
    {
        if(playbackList[sessionIndex].request == FREE)
        {
            /* Occupy the session & break */
            playbackList[sessionIndex].request = BUSY;
            break;
        }
    }
    MUTEX_UNLOCK(playbackListLock);

    if (sessionIndex >= MAX_PLAYBACK_SESSION)
    {
        EPRINT(PLAYBACK_MEDIA, "max async pb session reached: [camera=%d], [clientId=%d], [clientType=%d]", cameraIndex, sessionId, clientCbType);
        return CMD_MAX_STREAM_LIMIT;
    }

    // Store play-back information to buffer
    playCntrlInfo.startTime = startTime;
    playCntrlInfo.stopTime = endTime;

    // Play back use camera index 1 to 16
    playCntrlInfo.camNo = GET_CAMERA_NO(cameraIndex);
    playCntrlInfo.eventType	= eventType;
    playCntrlInfo.overlapFlg = overlapIndex;
    playCntrlInfo.diskId = diskId;
    playCntrlInfo.recStorageType = recStorageType;

    do
    {
        /* Open playback session in disk manager */
        retVal = OpenPlaySession(&playCntrlInfo, PLAYBACK_PLAY, &playbackList[sessionIndex].dmPlaySessId);
        if(retVal != CMD_SUCCESS)
        {
            EPRINT(PLAYBACK_MEDIA, "fail to open dm play session: [camera=%d], [session=%d], [retVal=%d]", cameraIndex, sessionIndex, retVal);
            break;
        }

        pbMessg.pbState = PS_GET_PLAYBACK_ID;
        pbMessg.cmdRespCallback = NULL;
        pbMessg.cmdRespFd = INVALID_CONNECTION;
        if(writePbMesg(sessionIndex, &pbMessg) == FAIL)
        {
            EPRINT(PLAYBACK_MEDIA, "fail to write async pb msg in queue: [session=%d]", sessionIndex);
            retVal = CMD_RESOURCE_LIMIT;
            break;
        }

        if (FAIL == Utils_CreateThread(NULL, playbackStreamerThread, &playbackList[sessionIndex], DETACHED_THREAD, PLAYBACK_THREAD_STACK_SZ))
        {
            EPRINT(PLAYBACK_MEDIA, "fail to create async pb thread: [session=%d]", sessionIndex);
            ClosePlaySession(playbackList[sessionIndex].dmPlaySessId);
            retVal = CMD_RESOURCE_LIMIT;
            break;
        }

        playbackList[sessionIndex].clientSessId = sessionId;
        playbackList[sessionIndex].cameraIndex = cameraIndex;
        playbackList[sessionIndex].clientCbType = clientCbType;
        *pPlaybackId = sessionIndex;
        DPRINT(PLAYBACK_MEDIA, "%s: [camera=%d], [eventType=%d], [overlap=%d], [diskId=%d], [storage=%d], [session=%d], [clientId=%d], "
                               "[clientType=%d], [startTime=%02d/%02d/%04d %02d:%02d:%02d], [stopTime=%02d/%02d/%04d %02d:%02d:%02d]",
               playbackSeqStr[pbMessg.pbState], cameraIndex, eventType, overlapIndex, diskId, recStorageType, sessionIndex, sessionId,
               clientCbType, startTime.tm_mday, startTime.tm_mon + 1, startTime.tm_year, startTime.tm_hour, startTime.tm_min, startTime.tm_sec,
                endTime.tm_mday, endTime.tm_mon + 1, endTime.tm_year, endTime.tm_hour, endTime.tm_min, endTime.tm_sec);
        return CMD_SUCCESS;

    }while(0);

    MUTEX_LOCK(playbackListLock);
    playbackList[sessionIndex].request = FREE;
    MUTEX_UNLOCK(playbackListLock);
    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API removes all playback session for all session upon user logout.
 */
void RemovePlayBackForAllSessionId(void)
{
    UINT8 sessionIdx;

    for(sessionIdx = 0; sessionIdx < MAX_NW_CLIENT; sessionIdx++)
    {
        RemovePlayBackFromSessionId(sessionIdx);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API removes all playback session related to this client session ID.
 * @param   sessionId
 */
void RemovePlayBackFromSessionId(UINT8 sessionId)
{
    UINT8 playBackId;

    for(playBackId = 0; playBackId < MAX_PLAYBACK_SESSION; playBackId++)
    {
        /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
        MUTEX_LOCK(playbackListLock);
        if(playbackList[playBackId].request == FREE)
        {
            MUTEX_UNLOCK(playbackListLock);
            continue;
        }
        MUTEX_UNLOCK(playbackListLock);

        /* Remove playback stream for session */
        RemovePlaybackStream(playBackId, sessionId, NULL, INVALID_CONNECTION);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API removes camera playback session related to this client session ID. This API is
 *          called when user privillegde for particular camera is changed.
 * @param   sessionId
 * @param   camIndex
 */
void RemovePlayBackForSessCamId(UINT8 sessionId, UINT8 camIndex)
{
    UINT8 playBackId;

    for(playBackId = 0; playBackId < MAX_PLAYBACK_SESSION; playBackId++)
    {
        /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
        MUTEX_LOCK(playbackListLock);
        if ((playbackList[playBackId].request == FREE) || (playbackList[playBackId].cameraIndex != camIndex))
        {
            MUTEX_UNLOCK(playbackListLock);
            continue;
        }
        MUTEX_UNLOCK(playbackListLock);

        /* Remove playback stream for session */
        RemovePlaybackStream(playBackId, sessionId, NULL, INVALID_CONNECTION);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Pause OR Resume Playback From SessionId
 * @param   sessionId
 * @param   isPausePlaybackF - TRUE=Pause & FALSE=Resume
 */
void PauseResumePlaybackFromSessionId(UINT8 sessionId, BOOL isPausePlaybackF)
{
    UINT8 playBackId;

    /* Find out async playback id from client session id */
    for(playBackId = 0; playBackId < MAX_PLAYBACK_SESSION; playBackId++)
    {
        /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
        MUTEX_LOCK(playbackListLock);
        if(playbackList[playBackId].request == FREE)
        {
            MUTEX_UNLOCK(playbackListLock);
            continue;
        }
        MUTEX_UNLOCK(playbackListLock);

        /* Pause or resume playback */
        if (isPausePlaybackF)
        {
            /* We have to pause playback */
            if (SUCCESS == PausePlaybackStream(playBackId, sessionId, NULL, INVALID_CONNECTION))
            {
                /* We have paused required session */
                break;
            }
        }
        else
        {
            /* We have to resume playback */
            if (SUCCESS == ResumePlaybackStream(playBackId, sessionId, NULL, INVALID_CONNECTION))
            {
                /* We have resumed required session */
                break;
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API closes the play-back session opened in disk manager and removes the request
 *          from the play-back stream list.
 * @param   streamId
 * @param   sessionId
 * @param   callback
 * @param   streamFd
 * @return
 */
BOOL RemovePlaybackStream(PLAYBACK_ID streamId, UINT8 sessionId, NW_CMD_REPLY_CB callback, INT32 streamFd)
{
    PLAYBACK_MSG_t pbMessg;

    // IF any of the input / output parameter is invalid
    if ((streamId >= MAX_PLAYBACK_SESSION) || (playbackList[streamId].clientSessId != sessionId))
    {
        return FAIL;
    }

    pbMessg.pbState = PS_CLEAR_PLAYBACK_ID;
    pbMessg.cmdRespCallback = callback;
    pbMessg.cmdRespFd = streamFd;
    return writePbMesg(streamId, &pbMessg);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API receives the streamId and other parameters. It sets the record at the time
 *          instance specified. and then sends signal to thread to start streaming record frame to user
 * @param   streamId
 * @param   sessionId
 * @param   direction
 * @param   timeInstance
 * @param   audio
 * @param   speed
 * @param   callback
 * @param   streamFd
 * @return
 */
BOOL StartPlaybackStream(PLAYBACK_ID streamId, UINT8 sessionId, PLAYBACK_CMD_e direction,
                         struct tm timeInstance, BOOL audio, UINT8 speed, NW_CMD_REPLY_CB callback, INT32 streamFd)
{
    PLAYBACK_MSG_t pbMessg;

    // IF any of the input / output parameter is invalid
    if ((streamId >= MAX_PLAYBACK_SESSION) || (playbackList[streamId].clientSessId != sessionId))
    {
        return FAIL;
    }

    MUTEX_LOCK(playbackList[streamId].playBackListMutex);
    playbackList[streamId].speed = speed;
    playbackList[streamId].direction = direction;
    if ((direction == PLAY_FORWARD) && (speed == PLAY_1X))
    {
        playbackList[streamId].audio = audio;
    }
    else
    {
        playbackList[streamId].audio = FALSE;
    }
    if ((direction == PLAY_FORWARD) && (speed < PLAY_8X))
    {
        playbackList[streamId].frameType = NW_ANY_FRAME;
    }
    else
    {
        playbackList[streamId].frameType = NW_I_FRAME;
    }
    MUTEX_UNLOCK(playbackList[streamId].playBackListMutex);

    pbMessg.stepTimeSec = timeInstance;
    pbMessg.pbState = PS_PLAY_STREAM;
    pbMessg.cmdRespCallback = callback;
    pbMessg.cmdRespFd = streamFd;

    DPRINT(PLAYBACK_MEDIA, "%s: [camera=%d], [session=%d], [direction=%d], [speed=%d], [audio=%d], [frameType=%d], [client=%d], "
                           "[time=%02d/%02d/%04d %02d:%02d:%02d]",
           playbackSeqStr[pbMessg.pbState], playbackList[streamId].cameraIndex, streamId, direction, speed, audio, playbackList[streamId].frameType,
            sessionId, timeInstance.tm_mday, timeInstance.tm_mon + 1, timeInstance.tm_year, timeInstance.tm_hour, timeInstance.tm_min, timeInstance.tm_sec);
    return writePbMesg(streamId, &pbMessg);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API sets the stream break flag, which in turn stops the stream of the record.
 * @param   streamId
 * @param   sessionId
 * @param   callback
 * @param   streamFd
 * @return
 */
BOOL StopPlaybackStream(PLAYBACK_ID streamId, UINT8 sessionId, NW_CMD_REPLY_CB callback, INT32 streamFd)
{
    PLAYBACK_MSG_t pbMessg;

    // IF any of the input / output parameter is invalid
    if ((streamId >= MAX_PLAYBACK_SESSION) || (playbackList[streamId].clientSessId != sessionId))
    {
        return FAIL;
    }

    pbMessg.pbState = PS_STOP_STREAM;
    pbMessg.cmdRespCallback = callback;
    pbMessg.cmdRespFd = streamFd;
    return writePbMesg(streamId, &pbMessg);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API sets the stream break flag, which in turn stops the stream of the record.
 * @param   streamId
 * @param   sessionId
 * @param   callback
 * @param   streamFd
 * @return
 */
BOOL PausePlaybackStream(PLAYBACK_ID streamId, UINT8 sessionId, NW_CMD_REPLY_CB callback, INT32 streamFd)
{
    PLAYBACK_MSG_t pbMessg;

    if ((streamId >= MAX_PLAYBACK_SESSION) || (playbackList[streamId].clientSessId != sessionId))
    {
        return FAIL;
    }

    pbMessg.pbState = PS_PAUSE_STREAM;
    pbMessg.cmdRespCallback = callback;
    pbMessg.cmdRespFd = streamFd;
    return writePbMesg(streamId, &pbMessg);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API resumes the streaming of specified paused stream.
 * @param   streamId
 * @param   sessionId
 * @param   callback
 * @param   streamFd
 * @return
 */
BOOL ResumePlaybackStream(PLAYBACK_ID streamId, UINT8 sessionId, NW_CMD_REPLY_CB callback, INT32 streamFd)
{
    PLAYBACK_MSG_t pbMessg;

    if ((streamId >= MAX_PLAYBACK_SESSION) || (playbackList[streamId].clientSessId != sessionId))
    {
        return FAIL;
    }

    pbMessg.pbState = PS_RESUME_STREAM;
    pbMessg.cmdRespCallback = callback;
    pbMessg.cmdRespFd = streamFd;
    return writePbMesg(streamId, &pbMessg);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API resumes the streaming of specified paused stream.
 * @param   streamId
 * @param   sessionId
 * @param   direction
 * @param   frameType
 * @param   timeInstance
 * @param   mSec
 * @param   callback
 * @param   streamFd
 * @return
 */
BOOL StepPlaybackStream(PLAYBACK_ID streamId, UINT8 sessionId, PLAYBACK_CMD_e direction, NW_FRAME_TYPE_e frameType,
                        struct tm timeInstance, UINT16 mSec, NW_CMD_REPLY_CB callback, INT32 streamFd)
{
    PLAYBACK_MSG_t pbMessg;

    if ((streamId >= MAX_PLAYBACK_SESSION) || (playbackList[streamId].clientSessId != sessionId))
    {
        return FAIL;
    }

    MUTEX_LOCK(playbackList[streamId].playBackListMutex);
    playbackList[streamId].audio = FALSE;
    playbackList[streamId].direction = direction;
    playbackList[streamId].frameType = NW_I_FRAME;
    MUTEX_UNLOCK(playbackList[streamId].playBackListMutex);

    pbMessg.stepTimeSec = timeInstance;
    pbMessg.stepTimemSec = mSec;
    pbMessg.pbState = PS_STEP_STREAM;
    pbMessg.cmdRespCallback = callback;
    pbMessg.cmdRespFd = streamFd;

    DPRINT(PLAYBACK_MEDIA, "%s: [camera=%d], [session=%d], [direction=%d], [frameType=%d], [client=%d], [time=%02d/%02d/%04d %02d:%02d:%02d:%d]",
           playbackSeqStr[pbMessg.pbState], playbackList[streamId].cameraIndex, streamId, direction, frameType, sessionId,
           timeInstance.tm_mday, timeInstance.tm_mon + 1, timeInstance.tm_year, timeInstance.tm_hour, timeInstance.tm_min, timeInstance.tm_sec, mSec);
    return writePbMesg(streamId, &pbMessg);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API actually starts streaming to user. This thread is created during the initialization
 *          of the module and it then waits for the signal to start streaming. Upon receipt of the play
 *          record or step record signal is sent to this thread, it starts reading frames from disk manager
 *          and sends it to user. When stop stream or pause stream is received, stream break flag is set,
 *          which breaks the streaming loop and thread again waits for the signal.
 * @param   threadArg
 * @return
 */
static VOIDPTR playbackStreamerThread(void *threadArg)
{
    BOOL					isThreadRunning = TRUE;
    PLAYBACK_INFO_t 		*pAsyncPbInfo = (PLAYBACK_INFO_t *)threadArg;
    UINT32 					frameLen = 0, sentByte = 0, readIdx;
    INT32                   sentLen;
    PLAYBACK_MSG_t			pbMessage = { .pbState = PS_PLAYBACK_SEQUENCE_MAX };
    INT32 					pbStreamFd = INVALID_CONNECTION;
    NET_CMD_STATUS_e		nwCmdResp;
    MEDIA_STATUS_e			pbMediaStatus = MEDIA_STATUS_MAX;

    // IF any of the input / output parameter is invalid
    THREAD_START_INDEX("ASYNC_PB", pAsyncPbInfo->pbClientIndex);

    while (isThreadRunning)
    {
        MUTEX_LOCK(pAsyncPbInfo->streamSignalLock);
        if(pAsyncPbInfo->pbMagQue.readIdx == pAsyncPbInfo->pbMagQue.writeIdx)
        {
            if ((pbMessage.pbState == PS_GET_PLAYBACK_ID) || (pbMessage.pbState == PS_PAUSE_STREAM)
                    || (pbMessage.pbState == PS_STEP_STREAM) || (pbMessage.pbState == PS_STOP_STREAM))
            {
                DPRINT(PLAYBACK_MEDIA, "async playback sleep: [camera=%d], [state=%s], [session=%d]",
                       pAsyncPbInfo->cameraIndex, playbackSeqStr[pbMessage.pbState], pAsyncPbInfo->pbClientIndex);
                pthread_cond_wait(&pAsyncPbInfo->streamSignal, &pAsyncPbInfo->streamSignalLock);
                DPRINT(PLAYBACK_MEDIA, "async playback run: [camera=%d], [state=%s], [session=%d]",
                       pAsyncPbInfo->cameraIndex, playbackSeqStr[pbMessage.pbState], pAsyncPbInfo->pbClientIndex);
            }
            MUTEX_UNLOCK(pAsyncPbInfo->streamSignalLock);
        }
        else
        {
            readIdx = pAsyncPbInfo->pbMagQue.readIdx + 1;
            if(readIdx >= PB_MSG_QUEUE_MAX)
            {
                readIdx = 0;
            }

            memcpy(&pbMessage, &pAsyncPbInfo->pbMagQue.pbMsg[readIdx], sizeof(PLAYBACK_MSG_t));
            pAsyncPbInfo->pbMagQue.readIdx = readIdx;
            MUTEX_UNLOCK(pAsyncPbInfo->streamSignalLock);

            DPRINT(PLAYBACK_MEDIA, "async playback cmd: [camera=%d], [state=%s], [fd=%d], [session=%d]",
                   pAsyncPbInfo->cameraIndex, (pbMessage.pbState < PS_PLAYBACK_SEQUENCE_MAX) ? playbackSeqStr[pbMessage.pbState] : "INVALID",
                    pbMessage.cmdRespFd, pAsyncPbInfo->pbClientIndex);
        }

        switch(pbMessage.pbState)
        {
            case PS_GET_PLAYBACK_ID:
            {
                /* Wait for other command */
            }
            break;

            case PS_RESUME_STREAM:
            {
                /* Send playback response to client and close the socket */
                sendPlaybackCmdResp(&pbMessage, CMD_SUCCESS, TRUE);
            }

            /* FALL THROUGH */
            case PS_PLAY_STREAM:
            {
                if(pbMessage.cmdRespCallback != NULL)
                {
                    if(pbMessage.pbState == PS_PLAY_STREAM)
                    {
                        /* Set play position for playback */
                        if (SetPlayPosition(&pbMessage.stepTimeSec, 0, pAsyncPbInfo->audio, pAsyncPbInfo->direction,
                                            pAsyncPbInfo->speed, pAsyncPbInfo->dmPlaySessId) == SUCCESS)
                        {
                            nwCmdResp = CMD_SUCCESS;
                        }
                        else
                        {
                            nwCmdResp = CMD_PROCESS_ERROR;
                        }

                        /* Close the fd if it is already opened */
                        if (pbStreamFd != INVALID_CONNECTION)
                        {
                            EPRINT(PLAYBACK_MEDIA, "async pb play cmd without stop: [camera=%d], [session=%d]",
                                   pAsyncPbInfo->cameraIndex, pAsyncPbInfo->pbClientIndex);
                            closeConnCb[pAsyncPbInfo->clientCbType](&pbStreamFd);
                        }

                        /* Send playback response to client but don't close the socket */
                        sendPlaybackCmdResp(&pbMessage, nwCmdResp, FALSE);

                        /* Store the fd for playback streaming */
                        pbStreamFd = pbMessage.cmdRespFd;
                    }
                }

                /* If frame length is not zero means previous frame send is pending.
                 * It may be full frame or chunk of frame. Before reading new frame, send it first. */
                if (frameLen == 0)
                {
                    sentByte = 0;
                    usleep(PB_PKT_SEND_TIMEOUT_IN_US);
                    pbMediaStatus = readyFrameToSend(pAsyncPbInfo, &frameLen);
                    if (pbMediaStatus != MEDIA_NORMAL)
                    {
                        sendPlaybackFrameHeader(pAsyncPbInfo, pbStreamFd, pbMediaStatus);
                        pbMessage.pbState = PS_STOP_STREAM;
                        break;
                    }
                }

                if (frameLen)
                {
                    sentLen = sendClientDataCb[pAsyncPbInfo->clientCbType](pbStreamFd, &pAsyncPbInfo->frameBuffer[sentByte], frameLen, PB_PKT_SEND_TIMEOUT_IN_US);
                    if (sentLen < 0)
                    {
                        /* Fail to send required data. Remote client is slow to receive data. */
                        pbMessage.pbState = PS_STOP_STREAM;
                        break;
                    }

                    /* Update sent data bytes. It may full length or partial frame sent bytes */
                    sentByte += sentLen;
                    frameLen -= sentLen;
                }
            }
            break;

            case PS_PAUSE_STREAM:
            {
                /* Send playback response to client and close the socket */
                sendPlaybackCmdResp(&pbMessage, CMD_SUCCESS, TRUE);
            }
            break;

            case PS_STEP_STREAM:
            {
                /* Don't procced if callback is not set */
                if(pbMessage.cmdRespCallback == NULL)
                {
                    break;
                }

                /* Set play position for step play */
                if (SetPlayPosition(&pbMessage.stepTimeSec, pbMessage.stepTimemSec, DISABLE,
                                    pAsyncPbInfo->direction, PLAY_1X, pAsyncPbInfo->dmPlaySessId) != SUCCESS)
                {
                    /* Send playback response to client and close the socket */
                    sendPlaybackCmdResp(&pbMessage, CMD_PROCESS_ERROR, TRUE);
                    break;
                }

                /* Send playback response to client but don't close the socket */
                sendPlaybackCmdResp(&pbMessage, CMD_SUCCESS, FALSE);

                /* Get the frame and send to client */
                pbMediaStatus = readyFrameToSend(pAsyncPbInfo, &frameLen);
                if (pbMediaStatus != MEDIA_NORMAL)
                {
                    /* Send error frame header */
                    sendPlaybackFrameHeader(pAsyncPbInfo, pbMessage.cmdRespFd, pbMediaStatus);
                }
                else
                {
                    /* Send frame data */
                    sendDataCb[pAsyncPbInfo->clientCbType](pbMessage.cmdRespFd, pAsyncPbInfo->frameBuffer, frameLen, SEND_PACKET_TIMEOUT);
                }

                /* Close the socket */
                closeConnCb[pAsyncPbInfo->clientCbType](&pbMessage.cmdRespFd);
                frameLen = 0;
            }
            break;

            case PS_STOP_STREAM:
            case PS_CLEAR_PLAYBACK_ID:
            {
                /* Close the streaming socket */
                closeConnCb[pAsyncPbInfo->clientCbType](&pbStreamFd);

                /* Send playback response to client and close the socket */
                sendPlaybackCmdResp(&pbMessage, CMD_SUCCESS, TRUE);

                if (pbMessage.pbState != PS_CLEAR_PLAYBACK_ID)
                {
                    /* Don't clear session on stop command */
                    break;
                }

                DPRINT(PLAYBACK_MEDIA, "clear async pb session: [camera=%d], [session=%d]", pAsyncPbInfo->cameraIndex, pAsyncPbInfo->pbClientIndex);
                ClosePlaySession(pAsyncPbInfo->dmPlaySessId);
                pAsyncPbInfo->cameraIndex = getMaxCameraForCurrentVariant();
                pAsyncPbInfo->clientSessId = MAX_NW_CLIENT;

                /* Exit from async playback thread */
                isThreadRunning = FALSE;
            }
            break;

            default:
            {
                EPRINT(PLAYBACK_MEDIA, "invld async pb state: [camera=%d], [session=%d] [state=%d]",
                       pAsyncPbInfo->cameraIndex, pAsyncPbInfo->pbClientIndex, pbMessage.pbState);
            }
            break;
        }
    }

    MUTEX_LOCK(playbackListLock);
    pAsyncPbInfo->request = FREE;
    MUTEX_UNLOCK(playbackListLock);
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   writePbMesg
 * @param   pbIndex
 * @param   pbMsgPtr
 * @return
 */
static BOOL writePbMesg(UINT8 pbIndex, PLAYBACK_MSG_t * pbMsgPtr)
{
    MUTEX_LOCK(playbackList[pbIndex].streamSignalLock);
    UINT32 writeIdx = playbackList[pbIndex].pbMagQue.writeIdx + 1;
    if(writeIdx >= PB_MSG_QUEUE_MAX)
    {
        writeIdx = 0;
    }

    if (writeIdx == playbackList[pbIndex].pbMagQue.readIdx)
    {
        MUTEX_UNLOCK(playbackList[pbIndex].streamSignalLock);
        return FAIL;
    }

    memcpy(&playbackList[pbIndex].pbMagQue.pbMsg[writeIdx], pbMsgPtr, sizeof(PLAYBACK_MSG_t));
    playbackList[pbIndex].pbMagQue.writeIdx = writeIdx;
    pthread_cond_signal(&playbackList[pbIndex].streamSignal);
    MUTEX_UNLOCK(playbackList[pbIndex].streamSignalLock);
    DPRINT(PLAYBACK_MEDIA, "async pb msg written: [session=%d], [msg=%s]", pbIndex, playbackSeqStr[pbMsgPtr->pbState]);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function send stream to stream socket
 * @param   infoPtr
 * @param   length
 * @return
 */
static MEDIA_STATUS_e readyFrameToSend(PLAYBACK_INFO_t * infoPtr, UINT32PTR length)
{
    PLAYBACK_CMD_e 			direction;
    NW_FRAME_TYPE_e 		frameType;
    PLAYBACK_FILE_READ_e	playbackStatus = MAX_PLAYBACK_FILE_STATE;
    FSH_INFO_t 				fshInfo = { 0 };
    FRAME_HEADER_t 			*frameHeader;
    UINT64					nextFrameTime = 0;

    *length = FRAME_HEADER_LEN_MAX;
    MUTEX_LOCK(infoPtr->playBackListMutex);
    direction = infoPtr->direction;
    frameType = infoPtr->frameType;
    MUTEX_UNLOCK(infoPtr->playBackListMutex);

    ReadRecordFrame(infoPtr->dmPlaySessId, direction, frameType, &fshInfo, &infoPtr->frameBuffer, length, &playbackStatus, &nextFrameTime);
    if (playbackStatus == PLAYBACK_FILE_READ_NORMAL)
    {
        frameHeader = (FRAME_HEADER_t *)infoPtr->frameBuffer;
        frameHeader->magicCode 		= MAGIC_CODE;
        frameHeader->headerVersion 	= HEADER_VERSION;
        frameHeader->productType 	= PRODUCT_TYPE;
        frameHeader->mediaFrmLen	= (UINT32)*length;
        frameHeader->chNo		    = GET_CAMERA_NO(infoPtr->cameraIndex);
        frameHeader->localTime		= fshInfo.localTime;
        frameHeader->streamType		= (UINT8)fshInfo.mediaType;
        frameHeader->codecType		= (UINT8)fshInfo.codecType;
        frameHeader->fps			= (UINT8)fshInfo.fps;
        frameHeader->frmType		= (UINT8)fshInfo.vop;
        frameHeader->vidResolution	= (UINT8)fshInfo.resolution;
        frameHeader->noOfRefFrame	= (UINT8)fshInfo.noOfRefFrame;
        frameHeader->mediaStatus    = (UINT8)MEDIA_NORMAL;
        frameHeader->vidLoss        = ((fshInfo.mediaType == STREAM_TYPE_VIDEO) && (frameHeader->mediaFrmLen == FRAME_HEADER_LEN_MAX)) ? TRUE : FALSE;
        frameHeader->audSampleFrq	= (UINT16)fshInfo.fps;
        frameHeader->vidFormat		= 1;
        frameHeader->scanType		= 0;
        return MEDIA_NORMAL;
    }

    *length = 0;
    if (playbackStatus == PLAYBACK_FILE_READ_ERROR)
    {
        return MEDIA_FILE_IO_ERROR;
    }
    else if (playbackStatus == PLAYBACK_FILE_READ_HDD_STOP)
    {
        return MEDIA_HDD_FORMAT;
    }
    else if (playbackStatus == PLAYBACK_FILE_READ_OVER)
    {
        return MEDIA_PLAYBACK_OVER;
    }
    else
    {
        return MEDIA_OTHER_ERROR;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   sendPlaybackFrameHeader
 * @param   pAsyncPbInfo
 * @param   sockFd
 * @param   mediaStatus
 * @return
 */
static BOOL sendPlaybackFrameHeader(PLAYBACK_INFO_t *pAsyncPbInfo, INT32 sockFd, MEDIA_STATUS_e mediaStatus)
{
    FRAME_HEADER_t frameHeader;

    frameHeader.magicCode           = MAGIC_CODE;
    frameHeader.headerVersion       = HEADER_VERSION;
    frameHeader.productType         = PRODUCT_TYPE;
    frameHeader.mediaFrmLen         = FRAME_HEADER_LEN_MAX;
    frameHeader.chNo                = GET_CAMERA_NO(pAsyncPbInfo->cameraIndex);
    frameHeader.streamType          = (UINT8)MAX_STREAM_TYPE;
    frameHeader.codecType           = (UINT8)MAX_VIDEO_CODEC;
    frameHeader.fps                 = 30;
    frameHeader.frmType             = (UINT8)MAX_FRAME_TYPE;
    frameHeader.vidResolution       = (UINT8)MAX_RESOLUTION;
    frameHeader.noOfRefFrame        = 0;
    frameHeader.vidLoss             = 0;
    frameHeader.audSampleFrq        = 8000;
    frameHeader.vidFormat           = 1;
    frameHeader.scanType            = 0;
    frameHeader.mediaStatus         = (UINT8)mediaStatus;
    frameHeader.localTime.totalSec  = 0;
    frameHeader.localTime.mSec		= 0;
    frameHeader.frmSyncNum          = 0;
    frameHeader.preReserverMediaLen = 0;

    if (SUCCESS != sendDataCb[pAsyncPbInfo->clientCbType](sockFd, (UINT8PTR)&frameHeader, FRAME_HEADER_LEN_MAX, SEND_PACKET_TIMEOUT))
    {
        EPRINT(PLAYBACK_MEDIA, "fail to send async pb frame header: [mediaStatus=%d]", mediaStatus);
        return FALSE;
    }

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send async playback command response to client
 * @param   pPbMsg
 * @param   cmdResp
 * @param   closeSockF
 */
static void sendPlaybackCmdResp(PLAYBACK_MSG_t *pPbMsg, NET_CMD_STATUS_e cmdResp, BOOL closeSockF)
{
    /* Don't procced if callback is not set */
    if(pPbMsg->cmdRespCallback == NULL)
    {
        return;
    }

    /* Send response to client and dereference callback */
    pPbMsg->cmdRespCallback(cmdResp, pPbMsg->cmdRespFd, closeSockF);
    pPbMsg->cmdRespCallback = NULL;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
