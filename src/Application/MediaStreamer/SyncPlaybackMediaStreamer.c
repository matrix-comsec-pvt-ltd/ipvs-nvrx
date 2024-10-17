//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		SyncPlaybackMediaStreamer.c
@brief      This module provides sync playback streaming functionalities. User can select a record
            from the record list returned. This selected record is given an ID and play-back session
            in disk manager is opened. When play command is received, a thread, which actually does
            the streaming, is signaled. This thread keeps on sending the frames to user until pause
            or stop command is not received. Upon receipt of pause or stop command, streaming loop
            is broken and streaming is stopped. After stop, if resume command is received then
            streaming is started from the same time instance, where it was stopped or if new play
            command is received then streaming is started from new time instance specified. Record
            can also be played in steps, i.e. frame by frame, one at a time. When streaming needs to
            be closed, all parameter stored against stream ID is flushed and stream ID is freed. Any
            use of stream ID without assigning value to it will be invalid.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Include */
#include <stdint.h>

/* Application Includes */
#include "SyncPlaybackMediaStreamer.h"
#include "MediaStreamer.h"
#include "Utils.h"
#include "DebugLog.h"
#include "NetworkManager.h"
#include "DiskController.h"
#include "NetworkCommand.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define MAX_SYNC_PLAYBACK_SESSION			(getMaxCameraForCurrentVariant() / 4)
#define SEARCH_STEP_FOR_SYNC_PLAY			(10)

/* Use Default Stack Size*/
#define SYNC_PLAYBK_STRM_STACK_SZ           (0*MEGA_BYTE)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef struct
{
    UINT64					nextFrameTime;
    PLAYBACK_FILE_READ_e	playbackStatus;
    PLAYBACK_FILE_READ_e	curPlaybackStatus;
    UINT32					searchListLen;
    UINT32					curRecFromList;
    UINT16					srchRsltListLen;
    PLAY_SESSION_ID 		dmPlaySessId;
    SEARCH_DATA_t			*searchListInfo;
}SYNC_CAM_PLAY_INFO_t;

typedef struct
{
    BOOL 					request;
    SYNC_PLAY_PARAMS_t		syncPlayParams;
    UINT8					pbSessionId;
    UINT8			 		clientCbType;
    UINT8			 		pbClientId;
    UINT8			 		userId;
    VOIDPTR					callback;
    pthread_mutex_t 		playBackListMutex;
    PLAYBACK_MSG_QUE_t		pbMagQue;
    pthread_mutex_t 		streamSignalLock;
    pthread_cond_t 			streamSignal;
    UINT8PTR 				frameBuffer;
    SYNC_CAM_PLAY_INFO_t	camListInfo[MAX_CAMERA];

}SYNC_PLAYBACK_INFO_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static SYNC_PLAYBACK_INFO_t	syncPlaybackList[MAX_CAMERA / 4];
static pthread_mutex_t 		playbackListLock;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void *SyncPlaybackStreamerThread(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
static BOOL WriteSyncPlaybackMsg(UINT8 pbIndex, PLAYBACK_MSG_t *pbMsgPtr);
//-------------------------------------------------------------------------------------------------
static BOOL GetRecordFrame(UINT8 camIndex, SYNC_PLAY_PARAMS_t *pPlayParams, SYNC_PLAYBACK_INFO_t *pPlayInfo,
                           UINT32PTR length, MEDIA_STATUS_e *mediaStatus);
//-------------------------------------------------------------------------------------------------
static BOOL FillRecordByHour(SYNC_PLAY_PARAMS_t *playParams, SYNC_PLAYBACK_INFO_t *playInfo, struct tm *brokenTime, UINT8 cameraSearchIndex);
//-------------------------------------------------------------------------------------------------
static void UpdateReferenceFrameTime(SYNC_PLAY_PARAMS_t *playParams, SYNC_PLAYBACK_INFO_t *playInfo, UINT64PTR updatedTime);
//-------------------------------------------------------------------------------------------------
static BOOL SetSyncPlayPosition(SYNC_PLAY_PARAMS_t *playParams, SYNC_PLAYBACK_INFO_t *playInfo, struct tm *stepTime);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e OpenPlaySessionAndSetPosition(SEARCH_DATA_t *searchData, BOOL setPlayPosition, BOOL audioEnbl, UINT8 speed,
                                                      PLAYBACK_CMD_e direction, UINT64PTR nextFrameTime, PLAY_SESSION_ID *playId);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e OpenNextRecord(UINT8 camIndex, SYNC_PLAY_PARAMS_t *playParams, SYNC_PLAYBACK_INFO_t *infoPtr);
//-------------------------------------------------------------------------------------------------
static BOOL IsAllPlaybackOver(SYNC_PLAY_PARAMS_t *playParams, SYNC_PLAYBACK_INFO_t *playInfo);
//-------------------------------------------------------------------------------------------------
static BOOL SendPlaybackFrameHeader(CLIENT_CB_TYPE_e clientCbType, UINT8 camIndex, INT32 sockFd,
                                    MEDIA_STATUS_e mediaStatus, UINT8 frameSyncNum, UINT64 frameTime);
//-------------------------------------------------------------------------------------------------
static BOOL	SortSearchData(SEARCH_DATA_t *searchData, SEARCH_DATA_t *searchRslt, UINT16 srchDataLen, UINT16 maxSortLen, UINT16PTR resltSort);
//-------------------------------------------------------------------------------------------------
static void CleanupSyncDataInfo(UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static UINT8 GetSyncPlaybackSessionIndex(UINT8 clientSessionId);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e GetSyncPlaybackFailCause(void);
//-------------------------------------------------------------------------------------------------
static void StoreCameraSyncPlaybackPrivilege(SYNC_PLAYBACK_INFO_t *pPlayInfo, SYNC_PLAY_PARAMS_t *pSyncPlayParams, BOOL *pCameraNoPrivilegeRespF);
//-------------------------------------------------------------------------------------------------
static void ClearSyncPlaybackDmSession(SYNC_PLAYBACK_INFO_t *pPlayInfo);
//-------------------------------------------------------------------------------------------------
static void SendSyncPlaybackCmdResp(PLAYBACK_MSG_t *pPlaybackMsg, NET_CMD_STATUS_e response, BOOL closeSockF);
//-------------------------------------------------------------------------------------------------
static void SendPbProcErrRespFrameHeader(INT32 *pbSockFd, SYNC_PLAY_PARAMS_t *playParams, UINT8 clientCbType, BOOL closeConnF);
//-------------------------------------------------------------------------------------------------
static BOOL GetInitialRecordByHour(struct tm *pStepTimeSec, SYNC_PLAYBACK_INFO_t *pPlayInfo, SYNC_PLAY_PARAMS_t *pSyncPlayParams,
                                   UINT64 *pRefFrameTime, BOOL *pCameraNoPrivilegeRespF);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API initializes the play-back media streamer module.
 */
void InitSyncPlaybackStreamer(void)
{
    UINT8 sessionIndex;

    // Initialize locks and signals
    MUTEX_INIT(playbackListLock, NULL);

    for(sessionIndex = 0; sessionIndex < MAX_SYNC_PLAYBACK_SESSION; sessionIndex++)
    {
        MUTEX_INIT(syncPlaybackList[sessionIndex].playBackListMutex, NULL);
        MUTEX_INIT(syncPlaybackList[sessionIndex].streamSignalLock, NULL);
        pthread_cond_init(&syncPlaybackList[sessionIndex].streamSignal, NULL);
        CleanupSyncDataInfo(sessionIndex);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API initializes the play-back media streamer date structures.
 * @param   sessionIndex
 * @return
 */
static void CleanupSyncDataInfo(UINT8 sessionIndex)
{
    UINT8 camIndex;

    MUTEX_LOCK(syncPlaybackList[sessionIndex].playBackListMutex);
    memset(&syncPlaybackList[sessionIndex].syncPlayParams, 0, sizeof(syncPlaybackList[sessionIndex].syncPlayParams));
    MUTEX_UNLOCK(syncPlaybackList[sessionIndex].playBackListMutex);

    syncPlaybackList[sessionIndex].pbSessionId = sessionIndex;
    syncPlaybackList[sessionIndex].clientCbType = CLIENT_CB_TYPE_MAX;
    syncPlaybackList[sessionIndex].pbClientId = MAX_NW_CLIENT;
    syncPlaybackList[sessionIndex].request = FREE;
    syncPlaybackList[sessionIndex].pbMagQue.readIdx = 0;
    syncPlaybackList[sessionIndex].pbMagQue.writeIdx = 0;

    for (camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
    {
        syncPlaybackList[sessionIndex].camListInfo[camIndex].curRecFromList = 0;
        syncPlaybackList[sessionIndex].camListInfo[camIndex].dmPlaySessId = INVALID_PLAY_SESSION_HANDLE;
        syncPlaybackList[sessionIndex].camListInfo[camIndex].nextFrameTime = 0;
        syncPlaybackList[sessionIndex].camListInfo[camIndex].searchListInfo = NULL;
        syncPlaybackList[sessionIndex].camListInfo[camIndex].searchListLen = SEARCH_STEP_FOR_SYNC_PLAY;
        syncPlaybackList[sessionIndex].camListInfo[camIndex].srchRsltListLen = 0;
        syncPlaybackList[sessionIndex].camListInfo[camIndex].curPlaybackStatus = PLAYBACK_FILE_READ_NORMAL;
        syncPlaybackList[sessionIndex].camListInfo[camIndex].playbackStatus = PLAYBACK_FILE_READ_NORMAL;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API adds request to the play-back stream list, if free any, and outputs the streamID.
 * @param   pSyncPlayParam
 * @param   clientCbType
 * @param   startTime
 * @param   sessionId
 * @param   userIndex
 * @param   callBack
 * @param   clientSocket
 * @return
 */
NET_CMD_STATUS_e AddSyncPlaybackStream(SYNC_PLAY_PARAMS_t *pSyncPlayParam, struct tm *startTime, CLIENT_CB_TYPE_e clientCbType,
                                       UINT8 sessionId, UINT8 userIndex, NW_CMD_REPLY_CB callBack, INT32 clientSocket)
{
    UINT8           syncPlayIndex;
    PLAYBACK_MSG_t  pbMessg;

    if (GetAvailablPlaySession() == 0)
    {
        EPRINT(PLAYBACK_MEDIA, "no disk manager session available for sync playback: [sessionId=%d]", sessionId);
        return CMD_MAX_STREAM_LIMIT;
    }

    /* Get sync playback session if already allocated */
    syncPlayIndex = GetSyncPlaybackSessionIndex(sessionId);
    if (syncPlayIndex >= MAX_SYNC_PLAYBACK_SESSION)
    {
        /* Session not allocated to current playback client */
        MUTEX_LOCK(playbackListLock);
        for(syncPlayIndex = 0; syncPlayIndex < MAX_SYNC_PLAYBACK_SESSION; syncPlayIndex++)
        {
            if(syncPlaybackList[syncPlayIndex].request == FREE)
            {
                // Occupy the session
                syncPlaybackList[syncPlayIndex].request = BUSY;
                pbMessg.pbState = PS_PLAY_STREAM;
                break;
            }
        }
        MUTEX_UNLOCK(playbackListLock);

        if (syncPlayIndex >= MAX_SYNC_PLAYBACK_SESSION)
        {
            EPRINT(PLAYBACK_MEDIA, "no free session available for sync playback: [client=%d]", sessionId);
            return CMD_MAX_STREAM_LIMIT;
        }
    }
    else
    {
        /* Session already allocated to client */
        pbMessg.pbState = PS_CLEAR_AND_RESTART_PLAYBACK;
    }

    MUTEX_LOCK(syncPlaybackList[syncPlayIndex].playBackListMutex);
    if ((pSyncPlayParam->direction == PLAY_FORWARD) && (pSyncPlayParam->speed < PLAY_8X))
    {
        pSyncPlayParam->frameType = NW_ANY_FRAME;
    }
    else
    {
        pSyncPlayParam->frameType = NW_I_FRAME;
    }
    syncPlaybackList[syncPlayIndex].syncPlayParams = *pSyncPlayParam;
    syncPlaybackList[syncPlayIndex].clientCbType = clientCbType;
    syncPlaybackList[syncPlayIndex].pbClientId = sessionId;
    syncPlaybackList[syncPlayIndex].userId = userIndex;
    MUTEX_UNLOCK(syncPlaybackList[syncPlayIndex].playBackListMutex);

    pbMessg.stepTimeSec = *startTime;
    pbMessg.cmdRespCallback = callBack;
    pbMessg.cmdRespFd = clientSocket;

    if (WriteSyncPlaybackMsg(syncPlayIndex, &pbMessg) != SUCCESS)
    {
        return CMD_RESOURCE_LIMIT;
    }

    DPRINT(PLAYBACK_MEDIA, "%s: [totalCamera=%d], [direction=%d], [speed=%d], [eventType=%d], [storage=%d], [frameType=%d], [time=%02d/%02d/%04d %02d:%02d:%02d]",
           playbackSeqStr[pbMessg.pbState], pSyncPlayParam->totalCamera, pSyncPlayParam->direction, pSyncPlayParam->speed, pSyncPlayParam->eventType,
           pSyncPlayParam->recStorageDrive, syncPlaybackList[syncPlayIndex].syncPlayParams.frameType,
           startTime->tm_mday, startTime->tm_mon + 1, startTime->tm_year, startTime->tm_hour, startTime->tm_min, startTime->tm_sec);

    if (pbMessg.pbState != PS_PLAY_STREAM)
    {
        return CMD_SUCCESS;
    }

    if (FAIL == Utils_CreateThread(NULL, SyncPlaybackStreamerThread, &syncPlaybackList[syncPlayIndex], DETACHED_THREAD, SYNC_PLAYBK_STRM_STACK_SZ))
    {
        EPRINT(PLAYBACK_MEDIA, "fail to create sync playback thread: [session=%d], [client=%d]", syncPlayIndex, sessionId);
        MUTEX_LOCK(playbackListLock);
        syncPlaybackList[syncPlayIndex].request = FREE;
        MUTEX_UNLOCK(playbackListLock);
        return CMD_RESOURCE_LIMIT;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API removes all playback session for all session upon user logout.
 * @param   diskStatus
 */
void RemoveSyncPlayBackForAllSessionId(DISK_ACT_e diskStatus)
{
    UINT8 sessionIdx;

    for(sessionIdx = 0; sessionIdx < MAX_NW_CLIENT; sessionIdx++)
    {
        RemoveSyncPlayBackFromSessionId(sessionIdx, diskStatus);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API removes all playback session related to this client session ID.
 * @param   sessionId
 * @param   diskStatus
 */
void RemoveSyncPlayBackFromSessionId(UINT8 sessionId, DISK_ACT_e diskStatus)
{
    RemoveSyncPlaybackStream(sessionId, NULL, INVALID_CONNECTION, diskStatus);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Pause OR Resume Sync Playback From SessionId
 * @param   sessionId
 * @param   isPausePlaybackF - TRUE=Pause & FALSE=Resume
 */
void PauseResumeSyncPlayBackFromSessionId(UINT8 sessionId, BOOL isPausePlaybackF)
{
    UINT8 syncPlayIndex;

    /* Get sync playback session */
    syncPlayIndex = GetSyncPlaybackSessionIndex(sessionId);
    if (syncPlayIndex >= MAX_SYNC_PLAYBACK_SESSION)
    {
        /* Sync PB is not running */
        return;
    }

    /* Sync PB is running; Hence, Pause or resume playback */
    if (isPausePlaybackF)
    {
        /* We have to pause playback */
        PauseSyncPlaybackStream(sessionId, NULL, INVALID_CONNECTION);
    }
    else
    {
        /* We have to resume playback */
        ResumeSyncPlaybackStream(sessionId, NULL, INVALID_CONNECTION);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API sets play position to given time and direction.
 * @param   pSyncPlayParam
 * @param   startTime
 * @param   sessionId
 * @param   callBack
 * @param   streamFd
 * @return
 */
NET_CMD_STATUS_e SetSyncPlaybackPosition(SYNC_PLAY_PARAMS_t *pSyncPlayParam, struct tm *startTime, UINT8 sessionId, NW_CMD_REPLY_CB callBack, INT32 clientSocket)
{
    UINT8           syncPlayIndex;
    PLAYBACK_MSG_t  pbMessg;

    /* Get sync playback session */
    syncPlayIndex = GetSyncPlaybackSessionIndex(sessionId);
    if (syncPlayIndex >= MAX_SYNC_PLAYBACK_SESSION)
    {
        return GetSyncPlaybackFailCause();
    }

    MUTEX_LOCK(syncPlaybackList[syncPlayIndex].playBackListMutex);
    if ((pSyncPlayParam->direction == PLAY_FORWARD) && (pSyncPlayParam->speed < PLAY_8X))
    {
        pSyncPlayParam->frameType = NW_ANY_FRAME;
    }
    else
    {
        pSyncPlayParam->frameType = NW_I_FRAME;
    }
    pSyncPlayParam->recStorageDrive = syncPlaybackList[syncPlayIndex].syncPlayParams.recStorageDrive;
    syncPlaybackList[syncPlayIndex].syncPlayParams = *pSyncPlayParam;
    MUTEX_UNLOCK(syncPlaybackList[syncPlayIndex].playBackListMutex);

    pbMessg.stepTimeSec = *startTime;
    pbMessg.pbState = PS_SET_PLAY_POSITION;
    pbMessg.cmdRespCallback = callBack;
    pbMessg.cmdRespFd = clientSocket;

    if (WriteSyncPlaybackMsg(syncPlayIndex, &pbMessg) != SUCCESS)
    {
        return CMD_RESOURCE_LIMIT;
    }

    DPRINT(PLAYBACK_MEDIA, "%s: [totalCamera=%d], [direction=%d], [speed=%d], [eventType=%d], [frmSyncNum=%d], [time=%02d/%02d/%04d %02d:%02d:%02d]",
           playbackSeqStr[pbMessg.pbState], pSyncPlayParam->totalCamera, pSyncPlayParam->direction,
           pSyncPlayParam->speed, pSyncPlayParam->eventType, pSyncPlayParam->frmSyncNum,
           startTime->tm_mday, startTime->tm_mon + 1, startTime->tm_year, startTime->tm_hour, startTime->tm_min, startTime->tm_sec);
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API sets the stream break flag, which in turn stops the stream of the record.
 * @param   sessionId
 * @param   callback
 * @param   streamFd
 * @return
 */
NET_CMD_STATUS_e PauseSyncPlaybackStream(UINT8 sessionId, NW_CMD_REPLY_CB callback, INT32 clientSocket)
{
    UINT8           syncPlayIndex = 0;
    PLAYBACK_MSG_t  pbMessg;

    /* Get sync playback session */
    syncPlayIndex = GetSyncPlaybackSessionIndex(sessionId);
    if (syncPlayIndex >= MAX_SYNC_PLAYBACK_SESSION)
    {
        return GetSyncPlaybackFailCause();
    }

    pbMessg.pbState = PS_PAUSE_STREAM;
    pbMessg.cmdRespCallback = callback;
    pbMessg.cmdRespFd = clientSocket;

    if (WriteSyncPlaybackMsg(syncPlayIndex, &pbMessg) != SUCCESS)
    {
        return CMD_RESOURCE_LIMIT;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API resumes the streaming of specified paused stream.
 * @param   sessionId
 * @param   callback
 * @param   streamFd
 * @return
 */
NET_CMD_STATUS_e ResumeSyncPlaybackStream(UINT8 sessionId, NW_CMD_REPLY_CB callback, INT32 clientSocket)
{
    UINT8           syncPlayIndex = 0;
    PLAYBACK_MSG_t  pbMessg;

    /* Get sync playback session */
    syncPlayIndex = GetSyncPlaybackSessionIndex(sessionId);
    if (syncPlayIndex >= MAX_SYNC_PLAYBACK_SESSION)
    {
        return GetSyncPlaybackFailCause();
    }

    pbMessg.pbState = PS_RESUME_STREAM;
    pbMessg.cmdRespCallback = callback;
    pbMessg.cmdRespFd = clientSocket;

    if (WriteSyncPlaybackMsg(syncPlayIndex, &pbMessg) != SUCCESS)
    {
        return CMD_RESOURCE_LIMIT;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API sets the stream break flag, which in turn stops the stream of the record.
 * @param   sessionId
 * @param   callback
 * @param   streamFd
 * @return
 */
NET_CMD_STATUS_e StopSyncPlaybackStream(UINT8 sessionId, NW_CMD_REPLY_CB callback, INT32 clientSocket)
{
    UINT8           syncPlayIndex = 0;
    PLAYBACK_MSG_t  pbMessg;

    /* Get sync playback session */
    syncPlayIndex = GetSyncPlaybackSessionIndex(sessionId);
    if (syncPlayIndex >= MAX_SYNC_PLAYBACK_SESSION)
    {
        return GetSyncPlaybackFailCause();
    }

    pbMessg.pbState = PS_STOP_STREAM;
    pbMessg.cmdRespCallback = callback;
    pbMessg.cmdRespFd = clientSocket;

    if (WriteSyncPlaybackMsg(syncPlayIndex, &pbMessg) != SUCCESS)
    {
        return CMD_RESOURCE_LIMIT;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API closes the play-back session opened in disk manager and removes the request from the play-back stream list.
 * @param   sessionId
 * @param   callback
 * @param   streamFd
 * @param   diskStatus
 * @return
 */
NET_CMD_STATUS_e RemoveSyncPlaybackStream(UINT8 sessionId, NW_CMD_REPLY_CB callback, INT32 clientSocket, DISK_ACT_e diskStatus)
{
    UINT8           syncPlayIndex = 0;
    PLAYBACK_MSG_t  pbMessg;

    /* Get sync playback session */
    syncPlayIndex = GetSyncPlaybackSessionIndex(sessionId);
    if (syncPlayIndex >= MAX_SYNC_PLAYBACK_SESSION)
    {
        return GetSyncPlaybackFailCause();
    }

    SetStorageDriveStatus(ALL_DRIVE, diskStatus);
    switch(diskStatus)
    {
        case DISK_ACT_NORMAL:
            pbMessg.responseCode = MEDIA_NORMAL;
            break;

        case DISK_ACT_CONFIG_CHANGE:
            pbMessg.responseCode = MEDIA_REC_DRIVE_CONFIG_CHANGE;
            break;

        case DISK_ACT_FORMAT:
            pbMessg.responseCode = MEDIA_HDD_FORMAT;
            break;

        default:
            pbMessg.responseCode = MEDIA_OTHER_ERROR;
            break;
    }

    pbMessg.pbState = PS_CLEAR_PLAYBACK_ID;
    pbMessg.cmdRespCallback = callback;
    pbMessg.cmdRespFd = clientSocket;

    if (WriteSyncPlaybackMsg(syncPlayIndex, &pbMessg) != SUCCESS)
    {
        return CMD_RESOURCE_LIMIT;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API fill next hour record from given time.
 * @param   playParams
 * @param   playInfo
 * @param   brokenTime
 * @param   cameraSearchIndex
 * @return
 */
static BOOL FillRecordByHour(SYNC_PLAY_PARAMS_t *playParams, SYNC_PLAYBACK_INFO_t *playInfo, struct tm *brokenTime, UINT8 cameraSearchIndex)
{
    BOOL 					retVal = SUCCESS;
    BOOL					recordFoundF = FALSE;
    UINT8					camIndex;
    UINT16					sortRsltCnt = 0;
    INT8					hourCnt = 0;
    UINT8					moreDataSearch, overlapIndicator = FALSE;
    UINT8					cameraStartIndex, cameraEndIndex, totalCamera;
    SEARCH_CRITERIA_t		srchCriteria;
    SYNC_CAM_PLAY_INFO_t	*camInfo;
    SEARCH_DATA_t			*syncSearchData = NULL;

    if (cameraSearchIndex >= MAX_CAMERA)
    {
        cameraStartIndex = 0;
        cameraEndIndex = getMaxCameraForCurrentVariant();
        totalCamera = playParams->totalCamera;
    }
    else
    {
        cameraStartIndex = cameraSearchIndex;
        cameraEndIndex = cameraSearchIndex + 1;
        totalCamera = 1;
    }

    for (camIndex = cameraStartIndex; camIndex < cameraEndIndex; camIndex++)
    {
        if (cameraSearchIndex >= MAX_CAMERA)
        {
            if (FALSE == playParams->cameraRecordF[camIndex])
            {
                continue;
            }
        }

        srchCriteria.startTime = *brokenTime;
        srchCriteria.endTime = *brokenTime;
        srchCriteria.eventType = playParams->eventType;
        srchCriteria.searchRecStorageType = playParams->recStorageDrive;
        hourCnt = brokenTime->tm_hour;

        do
        {
            srchCriteria.channelNo = GET_CAMERA_NO(camIndex);
            camInfo = &playInfo->camListInfo[camIndex];
            camInfo->srchRsltListLen = 0;
            camInfo->curRecFromList = 0;
            camInfo->nextFrameTime = 0;

            if ((playParams->direction == PLAY_FORWARD) || (playParams->direction == STEP_FORWARD))
            {
                if(hourCnt >= HOUR_IN_ONE_DAY)
                {
                    camInfo->nextFrameTime = UINT64_MAX;
                    break;
                }

                srchCriteria.startTime.tm_hour = hourCnt;
                srchCriteria.startTime.tm_min = 0;
                srchCriteria.startTime.tm_sec = 0;

                if(hourCnt == (HOUR_IN_ONE_DAY - 1))
                {
                    srchCriteria.endTime.tm_hour = (HOUR_IN_ONE_DAY - 1);
                    srchCriteria.endTime.tm_min = (MIN_IN_ONE_HOUR - 1);
                    srchCriteria.endTime.tm_sec = (SEC_IN_ONE_MIN - 1);
                }
                else
                {
                    srchCriteria.endTime.tm_hour = (hourCnt + 1);
                    srchCriteria.endTime.tm_min = 0;
                    srchCriteria.endTime.tm_sec = 0;
                }
            }
            else
            {
                if(hourCnt <= 0)
                {
                    camInfo->nextFrameTime = 0;
                    break;
                }

                srchCriteria.startTime.tm_hour = (hourCnt - 1);
                srchCriteria.startTime.tm_min = 0;
                srchCriteria.startTime.tm_sec = 0;

                if(hourCnt >= HOUR_IN_ONE_DAY)
                {
                    srchCriteria.endTime.tm_hour = (HOUR_IN_ONE_DAY - 1);
                    srchCriteria.endTime.tm_min = (MIN_IN_ONE_HOUR - 1);
                    srchCriteria.endTime.tm_sec = (SEC_IN_ONE_MIN - 1);
                }
                else
                {
                    srchCriteria.endTime.tm_hour = hourCnt;
                    srchCriteria.endTime.tm_min = 0;
                    srchCriteria.endTime.tm_sec = 0;
                }
            }

            do
            {
                srchCriteria.noOfRecord = camInfo->searchListLen;
                if(camInfo->searchListInfo == NULL)
                {
                    camInfo->searchListInfo = (SEARCH_DATA_t *)malloc(camInfo->searchListLen * (sizeof(SEARCH_DATA_t)));
                }

                if(camInfo->searchListInfo == NULL)
                {
                    EPRINT(PLAYBACK_MEDIA, "fail to alloc memory: [camera=%d]", camIndex);
                    continue;
                }

                // Check Only One event is present for playback
                moreDataSearch = FALSE;
                retVal = SearchCamAllEventForSync(&srchCriteria, camInfo->searchListInfo, &camInfo->srchRsltListLen, &moreDataSearch, &overlapIndicator);
                if (FAIL == retVal)
                {
                    EPRINT(PLAYBACK_MEDIA, "record search failed: [camera:%d]", camIndex);
                    FREE_MEMORY(camInfo->searchListInfo);
                    camInfo->searchListLen = SEARCH_STEP_FOR_SYNC_PLAY;
                    camInfo->srchRsltListLen = 0;
                    break;
                }

                /* Do we have to search more?? */
                if (moreDataSearch == TRUE)
                {
                    camInfo->searchListLen += SEARCH_STEP_FOR_SYNC_PLAY;
                    FREE_MEMORY(camInfo->searchListInfo);
                    continue;
                }

                if (camInfo->srchRsltListLen > 0)
                {
                    syncSearchData = (SEARCH_DATA_t *)malloc((sizeof(SEARCH_DATA_t) * camInfo->searchListLen));
                    if(syncSearchData != NULL)
                    {
                        sortRsltCnt = 0;
                        // Here we need to sort search result
                        SortSearchData(camInfo->searchListInfo, syncSearchData, camInfo->srchRsltListLen, camInfo->searchListLen, &sortRsltCnt);

                        camInfo->srchRsltListLen = sortRsltCnt;
                        memcpy(camInfo->searchListInfo, syncSearchData, (sizeof(SEARCH_DATA_t) * sortRsltCnt));

                        if((playParams->direction == PLAY_FORWARD) || (playParams->direction == STEP_FORWARD))
                        {
                            // which record should play
                            camInfo->curRecFromList = 0;
                            camInfo->nextFrameTime = ((UINT64)camInfo->searchListInfo[0].startTime * MSEC_IN_ONE_SEC);
                        }
                        else
                        {
                            // because we have fetch it from array
                            camInfo->curRecFromList = (camInfo->srchRsltListLen - 1);
                            camInfo->nextFrameTime = ((UINT64)camInfo->searchListInfo[camInfo->curRecFromList].stopTime * MSEC_IN_ONE_SEC);
                        }

                        // Initialise first frame time
                        DPRINT(PLAYBACK_MEDIA, "sorting done: [camera=%d] [record=%d] [next_frame_time=%lld]",
                               camIndex, camInfo->srchRsltListLen, camInfo->nextFrameTime);
                        FREE_MEMORY(syncSearchData);
                    }
                    else
                    {
                        EPRINT(PLAYBACK_MEDIA, "memory allocation failed: [camera=%d]", camIndex);
                        camInfo->srchRsltListLen = 0;
                        camInfo->nextFrameTime = ((playParams->direction == PLAY_FORWARD) || (playParams->direction == STEP_FORWARD)) ? UINT64_MAX : 0;
                    }
                }
                else
                {
                    camInfo->nextFrameTime = ((playParams->direction == PLAY_FORWARD) || (playParams->direction == STEP_FORWARD)) ? UINT64_MAX : 0;
                }
                break;

            }while(retVal != FAIL);

            if(retVal == FAIL)
            {
                break;
            }

            if(camInfo->srchRsltListLen > 0)
            {
                break;
            }

            if ((hourCnt <= HOUR_IN_ONE_DAY) && ((playParams->direction == PLAY_FORWARD) || (playParams->direction == STEP_FORWARD)))
            {
                hourCnt++;
                if(hourCnt >= HOUR_IN_ONE_DAY)
                {
                    camInfo->nextFrameTime = UINT64_MAX;
                    break;
                }
            }
            else if ((hourCnt >= 0) && ((playParams->direction == PLAY_REVERSE) || (playParams->direction == STEP_REVERSE)))
            {
                hourCnt--;
                if(hourCnt <= 0)
                {
                    camInfo->nextFrameTime = 0;
                    break;
                }
            }

            srchCriteria.endTime.tm_min = 0;
            srchCriteria.endTime.tm_sec = 0;

        } while(retVal != FAIL);

        if (camInfo->srchRsltListLen == 0)
        {
            camInfo->playbackStatus = PLAYBACK_FILE_READ_OVER;
        }
        else
        {
            recordFoundF = TRUE;
        }

        if (retVal == FAIL)
        {
            break;
        }
    }

    if ((retVal != FAIL) && (recordFoundF == FALSE))
    {
        WPRINT(PLAYBACK_MEDIA, "no records found for any cameras: [totalCamera=%d]", totalCamera);
        retVal = FAIL;
    }

    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API update referance frame time from list of camera in which recording is going on.
 * @param   playParams
 * @param   playInfo
 * @param   updatedTime
 */
static void UpdateReferenceFrameTime(SYNC_PLAY_PARAMS_t *playParams, SYNC_PLAYBACK_INFO_t *playInfo, UINT64PTR updatedTime)
{
    UINT8   camIndex;
    UINT64  refFrameTime = ((playParams->direction == PLAY_FORWARD) || (playParams->direction == STEP_FORWARD)) ? UINT64_MAX : 0;

    //Find out next frame time which is less than all.
    for (camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
    {
        if (FALSE == playParams->cameraRecordF[camIndex])
        {
            continue;
        }

        if ((playParams->direction == PLAY_FORWARD) || (playParams->direction == STEP_FORWARD))
        {
            if (refFrameTime >= playInfo->camListInfo[camIndex].nextFrameTime)
            {
                refFrameTime = playInfo->camListInfo[camIndex].nextFrameTime;
            }
        }
        else
        {
            if (refFrameTime <= playInfo->camListInfo[camIndex].nextFrameTime)
            {
                refFrameTime = playInfo->camListInfo[camIndex].nextFrameTime;
            }
        }
    }

    if (refFrameTime)
    {
        *updatedTime = refFrameTime;
    }
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
static void *SyncPlaybackStreamerThread(VOIDPTR threadArg)
{
    BOOL					isThreadRunning = TRUE;
    BOOL                    noCameraFrameAvailF = FALSE;
    UINT8					camIndex;
    UINT64					refFrameTime = 0;
    SYNC_PLAYBACK_INFO_t 	*playInfo = (SYNC_PLAYBACK_INFO_t *)threadArg;
    UINT32 					frameLength = 0;
    INT32 					pbStreamFd = INVALID_CONNECTION;
    NET_CMD_STATUS_e		nwCmdResp = CMD_SUCCESS;
    PLAYBACK_MSG_t			pbMessage = { .pbState = PS_PLAYBACK_SEQUENCE_MAX };
    MEDIA_STATUS_e			pbMediaStatus = MEDIA_STATUS_MAX;
    SYNC_PLAY_PARAMS_t		syncPlayParams;
    BOOL					commandRespGiven = FALSE;
    BOOL                    cameraNoPrivilegeRespF[MAX_CAMERA];

    THREAD_START_INDEX("SYNC_PB", playInfo->pbSessionId);

    for (camIndex = 0; camIndex < MAX_CAMERA; camIndex++)
    {
        cameraNoPrivilegeRespF[camIndex] = FALSE;
        playInfo->camListInfo[camIndex].searchListLen = SEARCH_STEP_FOR_SYNC_PLAY;
        playInfo->camListInfo[camIndex].dmPlaySessId = INVALID_PLAY_SESSION_HANDLE;
    }

    while (isThreadRunning)
    {
        MUTEX_LOCK(playInfo->playBackListMutex);
        memcpy(&syncPlayParams, &playInfo->syncPlayParams, sizeof(SYNC_PLAY_PARAMS_t));
        MUTEX_UNLOCK(playInfo->playBackListMutex);

        MUTEX_LOCK(playInfo->streamSignalLock);
        if (playInfo->pbMagQue.readIdx == playInfo->pbMagQue.writeIdx)
        {
            if ((pbMessage.pbState == PS_PAUSE_STREAM) || (pbMessage.pbState == PS_STOP_STREAM))
            {
                DPRINT(PLAYBACK_MEDIA, "sync playback sleep: [state=%s], [session=%d]", playbackSeqStr[pbMessage.pbState], playInfo->pbSessionId);
                pthread_cond_wait(&playInfo->streamSignal, &playInfo->streamSignalLock);
                DPRINT(PLAYBACK_MEDIA, "sync playback run: [state=%s], [session=%d]", playbackSeqStr[pbMessage.pbState], playInfo->pbSessionId);
            }
            MUTEX_UNLOCK(playInfo->streamSignalLock);
        }
        else
        {
            playInfo->pbMagQue.readIdx++;
            if(playInfo->pbMagQue.readIdx >= PB_MSG_QUEUE_MAX)
            {
                playInfo->pbMagQue.readIdx = 0;
            }

            memcpy(&pbMessage, &playInfo->pbMagQue.pbMsg[playInfo->pbMagQue.readIdx], sizeof(PLAYBACK_MSG_t));
            MUTEX_UNLOCK(playInfo->streamSignalLock);

            DPRINT(PLAYBACK_MEDIA, "sync pb cmd: [state=%s], [fd=%d], [session=%d], [client=%d]",
                   (pbMessage.pbState < PS_PLAYBACK_SEQUENCE_MAX) ? playbackSeqStr[pbMessage.pbState] : "INVALID",
                    pbMessage.cmdRespFd, playInfo->pbSessionId, playInfo->pbClientId);

            /* For initial play message only */
            if (pbMessage.pbState == PS_PLAY_STREAM)
            {
                /* Get initial recording for play */
                if (FALSE == GetInitialRecordByHour(&pbMessage.stepTimeSec, playInfo, &syncPlayParams, &refFrameTime, cameraNoPrivilegeRespF))
                {
                    /* Error found. So no need to start play */
                    SendPbProcErrRespFrameHeader(&pbMessage.cmdRespFd, &syncPlayParams, playInfo->clientCbType, TRUE);
                    EPRINT(PLAYBACK_MEDIA, "fail to start sync playback, exiting: [session=%d]", playInfo->pbSessionId);
                    break;
                }
            }
        }

        switch(pbMessage.pbState)
        {
            case PS_RESUME_STREAM:
            {
                SendSyncPlaybackCmdResp(&pbMessage, CMD_SUCCESS, TRUE);
            }

            /* Fall through */
            case PS_PLAY_STREAM:
            {
                noCameraFrameAvailF = TRUE;
                for (camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
                {
                    if (TRUE == syncPlayParams.cameraRecordF[camIndex])
                    {
                        // check next frame is available for camera
                        if (((syncPlayParams.direction == PLAY_FORWARD) && (playInfo->camListInfo[camIndex].nextFrameTime <= refFrameTime))
                                || ((syncPlayParams.direction == PLAY_REVERSE) && (playInfo->camListInfo[camIndex].nextFrameTime >= refFrameTime)))
                        {
                            if ((pbMessage.cmdRespCallback != NULL) && (pbMessage.pbState == PS_PLAY_STREAM))
                            {
                                pbMessage.cmdRespCallback = NULL;
                                DPRINT(PLAYBACK_MEDIA, "set playback position: [camera=%d], [timestamp=%02d:%02d]",
                                       camIndex, pbMessage.stepTimeSec.tm_hour, pbMessage.stepTimeSec.tm_min);

                                if (SetSyncPlayPosition(&syncPlayParams, playInfo, &pbMessage.stepTimeSec) == SUCCESS)
                                {
                                    pbStreamFd = pbMessage.cmdRespFd;
                                    commandRespGiven = TRUE;
                                }
                                else
                                {
                                    EPRINT(PLAYBACK_MEDIA, "fail to set play position: [camera=%d]", camIndex);
                                    SendPbProcErrRespFrameHeader(&pbMessage.cmdRespFd, &syncPlayParams, playInfo->clientCbType, TRUE);

                                    /* Clear all sync playback DM session */
                                    ClearSyncPlaybackDmSession(playInfo);
                                    pbMessage.pbState = PS_STOP_STREAM;
                                    break;
                                }
                            }

                            if (playInfo->camListInfo[camIndex].curPlaybackStatus == PLAYBACK_FILE_READ_OVER)
                            {
                                noCameraFrameAvailF = FALSE;
                                DPRINT(PLAYBACK_MEDIA, "try to open next hour record: [camera=%d]", camIndex);
                                nwCmdResp = OpenNextRecord(camIndex, &syncPlayParams, playInfo);
                                if (nwCmdResp == CMD_SUCCESS)
                                {
                                    /* Update frame reference time */
                                    UpdateReferenceFrameTime(&syncPlayParams, playInfo, &refFrameTime);
                                }
                                else if(nwCmdResp == CMD_MAX_STREAM_LIMIT)
                                {
                                    SendPlaybackFrameHeader(playInfo->clientCbType, camIndex, pbStreamFd, MEDIA_PLAYBACK_SESSION_NOT_AVAIL, 0, 0);
                                }

                                if (TRUE == IsAllPlaybackOver(&syncPlayParams, playInfo))
                                {
                                    /* All channel play back over so, wait for next command */
                                    SendPlaybackFrameHeader(playInfo->clientCbType, camIndex, pbStreamFd, MEDIA_PLAYBACK_OVER, 0, 0);

                                    /* Clear all sync playback DM session */
                                    ClearSyncPlaybackDmSession(playInfo);
                                    pbMessage.pbState = PS_STOP_STREAM;
                                    DPRINT(PLAYBACK_MEDIA, "send all camera playback over: [camera=%d]", camIndex);
                                    break;
                                }
                            }
                            else
                            {
                                if (playInfo->camListInfo[camIndex].dmPlaySessId >= MAX_PLAYBACK_DISK_SESSION)
                                {
                                    EPRINT(PLAYBACK_MEDIA, "invalid playback id: [camera=%d]", camIndex);
                                }
                                else
                                {
                                    /* Read record frame for playback */
                                    noCameraFrameAvailF = FALSE;
                                    if (FALSE == GetRecordFrame(camIndex, &syncPlayParams, playInfo, &frameLength, &pbMediaStatus))
                                    {
                                        if (pbMediaStatus != MEDIA_PLAYBACK_OVER)
                                        {
                                            /* Clear all sync playback DM session */
                                            SendPlaybackFrameHeader(playInfo->clientCbType, camIndex, pbStreamFd, pbMediaStatus, 0, 0);
                                            ClearSyncPlaybackDmSession(playInfo);
                                            pbMessage.pbState = PS_STOP_STREAM;
                                            break;
                                        }
                                    }
                                    else if (frameLength)
                                    {
                                        if (sendDataCb[playInfo->clientCbType](pbStreamFd, playInfo->frameBuffer, frameLength, SEND_PACKET_TIMEOUT) == FAIL)
                                        {
                                            EPRINT(PLAYBACK_MEDIA,"fail to send sync playback stream: [camera=%d], [session=%d]", camIndex, playInfo->pbSessionId);

                                            /* Clear all sync playback DM session */
                                            ClearSyncPlaybackDmSession(playInfo);
                                            pbMessage.pbState = PS_STOP_STREAM;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        if ((commandRespGiven == TRUE) && (TRUE == cameraNoPrivilegeRespF[camIndex]))
                        {
                            noCameraFrameAvailF = FALSE;
                            cameraNoPrivilegeRespF[camIndex] = FALSE;
                            DPRINT(PLAYBACK_MEDIA, "send no previlage frame header: [camera=%d]", camIndex);
                            SendPlaybackFrameHeader(playInfo->clientCbType, camIndex, pbStreamFd, MEDIA_PLAYBACK_CAM_PREVILAGE, 0, refFrameTime);
                        }
                    }
                }

                /* If error occurred then stop playback */
                if (pbMessage.pbState == PS_STOP_STREAM)
                {
                    break;
                }

                if (TRUE == noCameraFrameAvailF)
                {
                    /* Update frame reference time */
                    UpdateReferenceFrameTime(&syncPlayParams, playInfo, &refFrameTime);

                    if (((refFrameTime == UINT64_MAX) && (syncPlayParams.direction == PLAY_FORWARD))
                            || ((refFrameTime == 0) && (syncPlayParams.direction == PLAY_REVERSE)))
                    {
                        DPRINT(PLAYBACK_MEDIA, "no frames available for any cameras: [camera=%d]", camIndex);
                        for (camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
                        {
                            if (TRUE == syncPlayParams.cameraRecordF[camIndex])
                            {
                                SendPlaybackFrameHeader(playInfo->clientCbType, camIndex, pbStreamFd, MEDIA_PLAYBACK_OVER, 0, 0);
                                break;
                            }
                        }

                        /* Clear all sync playback DM session */
                        ClearSyncPlaybackDmSession(playInfo);
                        pbMessage.pbState = PS_STOP_STREAM;
                    }
                }
                else if(camIndex >= getMaxCameraForCurrentVariant())
                {
                    UpdateReferenceFrameTime(&syncPlayParams, playInfo, &refFrameTime);
                }

                /* Device client sends 1 camera in clip export. We added some delay to slow down the process */
                if (syncPlayParams.totalCamera == 1)
                {
                    usleep(2000);
                }
            }
            break;

            case PS_SET_PLAY_POSITION:
            {
                /* Check and store camera privilege for playback */
                StoreCameraSyncPlaybackPrivilege(playInfo, &syncPlayParams, cameraNoPrivilegeRespF);

                if (SetSyncPlayPosition(&syncPlayParams, playInfo, &pbMessage.stepTimeSec) == SUCCESS)
                {
                    commandRespGiven = TRUE;
                    UpdateReferenceFrameTime(&syncPlayParams, playInfo, &refFrameTime);
                    pbMessage.pbState = PS_PLAY_STREAM;

                    for (camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
                    {
                        if (TRUE == syncPlayParams.cameraRecordF[camIndex])
                        {
                            SendPlaybackFrameHeader(playInfo->clientCbType, camIndex, pbStreamFd, MEDIA_SYNC_START_INDICATOR, syncPlayParams.frmSyncNum, 0);
                            break;
                        }
                    }
                }
                else
                {
                    EPRINT(PLAYBACK_MEDIA, "fail to set play position, check all playback: [session=%d]", playInfo->pbSessionId);

                    /* Error found. So no need to start play */
                    SendPbProcErrRespFrameHeader(&pbStreamFd, &syncPlayParams, playInfo->clientCbType, FALSE);

                    /* Clear all sync playback DM session */
                    ClearSyncPlaybackDmSession(playInfo);
                    pbMessage.pbState = PS_STOP_STREAM;
                }
            }
            break;

            case PS_PAUSE_STREAM:
            {
                SendSyncPlaybackCmdResp(&pbMessage, CMD_SUCCESS, TRUE);
            }
            break;

            case PS_STOP_STREAM:
            {
                SendSyncPlaybackCmdResp(&pbMessage, CMD_SUCCESS, TRUE);

                /* Clear all sync playback DM session */
                ClearSyncPlaybackDmSession(playInfo);
            }
            break;

            case PS_CLEAR_PLAYBACK_ID:
            {
                if (pbStreamFd != INVALID_CONNECTION)
                {
                    if (pbMessage.responseCode != MEDIA_NORMAL)
                    {
                        for (camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
                        {
                            if (TRUE == syncPlayParams.cameraRecordF[camIndex])
                            {
                                SendPlaybackFrameHeader(playInfo->clientCbType, camIndex, pbStreamFd, pbMessage.responseCode, 0, 0);
                                break;
                            }
                        }
                    }
                    closeConnCb[playInfo->clientCbType](&pbStreamFd);
                }

                /* Clear all sync playback DM session */
                ClearSyncPlaybackDmSession(playInfo);

                /* Exit from sync playback thread */
                isThreadRunning = FALSE;
                MUTEX_LOCK(playInfo->streamSignalLock);
                playInfo->pbMagQue.readIdx = 0;
                playInfo->pbMagQue.writeIdx = 0;
                MUTEX_UNLOCK(playInfo->streamSignalLock);
            }
            break;

            case PS_CLEAR_AND_RESTART_PLAYBACK:
            {
                /* Clear all sync playback DM session */
                ClearSyncPlaybackDmSession(playInfo);
                closeConnCb[playInfo->clientCbType](&pbStreamFd);

                /* Get initial recording for play */
                if (FALSE == GetInitialRecordByHour(&pbMessage.stepTimeSec, playInfo, &syncPlayParams, &refFrameTime, cameraNoPrivilegeRespF))
                {
                    /* Error found. So no need to start play */
                    SendPbProcErrRespFrameHeader(&pbMessage.cmdRespFd, &syncPlayParams, playInfo->clientCbType, TRUE);
                    EPRINT(PLAYBACK_MEDIA, "fail to start sync playback, waiting: [session=%d]", playInfo->pbSessionId);
                    pbMessage.pbState = PS_STOP_STREAM;
                }
                else
                {
                    pbMessage.pbState = PS_PLAY_STREAM;
                }
            }
            break;

            default:
            {
                /* Nothing to do */
                EPRINT(PLAYBACK_MEDIA, "invld sync playback state: [session=%d], [state=%d]", playInfo->pbSessionId, pbMessage.pbState);
            }
            break;
        }
    }

    /* Free allocated memory for cameras */
    for (camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
    {
        FREE_MEMORY(playInfo->camListInfo[camIndex].searchListInfo);
    }

    /* Reset all params */
    CleanupSyncDataInfo(playInfo->pbSessionId);

    //Free the session
    MUTEX_LOCK(playbackListLock);
    playInfo->request = FREE;
    MUTEX_UNLOCK(playbackListLock);
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   WriteSyncPlaybackMsg
 * @param   pbIndex
 * @param   pbMsgPtr
 * @return
 */
static BOOL WriteSyncPlaybackMsg(UINT8 pbIndex, PLAYBACK_MSG_t *pbMsgPtr)
{
    UINT32 writeIdx;

    MUTEX_LOCK(syncPlaybackList[pbIndex].streamSignalLock);
    writeIdx = syncPlaybackList[pbIndex].pbMagQue.writeIdx + 1;
    if(writeIdx >= PB_MSG_QUEUE_MAX)
    {
        writeIdx = 0;
    }

    if (writeIdx == syncPlaybackList[pbIndex].pbMagQue.readIdx)
    {
        MUTEX_UNLOCK(syncPlaybackList[pbIndex].streamSignalLock);
        return FAIL;
    }

    memcpy(&syncPlaybackList[pbIndex].pbMagQue.pbMsg[writeIdx], pbMsgPtr, sizeof(PLAYBACK_MSG_t));
    syncPlaybackList[pbIndex].pbMagQue.writeIdx = writeIdx;
    pthread_cond_signal(&syncPlaybackList[pbIndex].streamSignal);
    MUTEX_UNLOCK(syncPlaybackList[pbIndex].streamSignalLock);
    DPRINT(PLAYBACK_MEDIA, "write playback message: [session=%d] [message=%s]", pbIndex, playbackSeqStr[pbMsgPtr->pbState]);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function get the stream from disk manager
 * @param   camIndex
 * @param   pPlayParams
 * @param   pPlayInfo
 * @param   length
 * @param   mediaStatus
 */
static BOOL GetRecordFrame(UINT8 camIndex, SYNC_PLAY_PARAMS_t *pPlayParams,
                           SYNC_PLAYBACK_INFO_t *pPlayInfo, UINT32PTR length, MEDIA_STATUS_e *mediaStatus)
{
    PLAYBACK_FILE_READ_e	playbackStatus = MAX_PLAYBACK_FILE_STATE;
    FSH_INFO_t 				fshInfo = { 0 };
    FRAME_HEADER_t 			*frameHeader;

    *length = FRAME_HEADER_LEN_MAX;

    ReadRecordFrame(pPlayInfo->camListInfo[camIndex].dmPlaySessId, pPlayParams->direction, pPlayParams->frameType,
                    &fshInfo, &pPlayInfo->frameBuffer, length, &playbackStatus, &pPlayInfo->camListInfo[camIndex].nextFrameTime);

    pPlayInfo->camListInfo[camIndex].curPlaybackStatus = playbackStatus;

    switch(playbackStatus)
    {
        case PLAYBACK_FILE_READ_NORMAL:
        {
            frameHeader = (FRAME_HEADER_t *)pPlayInfo->frameBuffer;
            frameHeader->magicCode 		= MAGIC_CODE;
            frameHeader->headerVersion 	= HEADER_VERSION;
            frameHeader->productType 	= PRODUCT_TYPE;
            frameHeader->mediaFrmLen	= (UINT32)*length;
            frameHeader->chNo		    = GET_CAMERA_NO(camIndex);
            frameHeader->localTime		= fshInfo.localTime;
            frameHeader->streamType		= (UINT8)fshInfo.mediaType;
            frameHeader->codecType		= (UINT8)fshInfo.codecType;
            frameHeader->fps			= (UINT8)fshInfo.fps;
            frameHeader->frmType		= (UINT8)fshInfo.vop;
            frameHeader->vidResolution	= (UINT8)fshInfo.resolution;
            frameHeader->noOfRefFrame	= (UINT8)fshInfo.noOfRefFrame;
            frameHeader->frmSyncNum		= (UINT8)pPlayParams->frmSyncNum;
            frameHeader->mediaStatus    = (UINT8)MEDIA_NORMAL;

            if((fshInfo.mediaType == STREAM_TYPE_VIDEO) && (frameHeader->mediaFrmLen == FRAME_HEADER_LEN_MAX))
            {
                frameHeader->vidLoss = TRUE;
            }
            else
            {
                frameHeader->vidLoss = FALSE;
            }

            frameHeader->audSampleFrq	= (UINT32)fshInfo.fps;
            frameHeader->vidFormat		= 1;
            frameHeader->scanType		= 0;
            *mediaStatus = MEDIA_NORMAL;
        }
        return TRUE;

        case PLAYBACK_FILE_READ_ERROR:
        {
            EPRINT(PLAYBACK_MEDIA, "file io error: [camera=%d]", camIndex);
            ClosePlaySession(pPlayInfo->camListInfo[camIndex].dmPlaySessId);
            pPlayInfo->camListInfo[camIndex].dmPlaySessId = INVALID_PLAY_SESSION_HANDLE;
            *mediaStatus = MEDIA_FILE_IO_ERROR;
        }
        return FALSE;

        case PLAYBACK_FILE_READ_HDD_STOP:
        {
            ClosePlaySession(pPlayInfo->camListInfo[camIndex].dmPlaySessId);
            pPlayInfo->camListInfo[camIndex].dmPlaySessId = INVALID_PLAY_SESSION_HANDLE;
            pPlayInfo->camListInfo[camIndex].nextFrameTime = (pPlayParams->direction == PLAY_FORWARD) ? UINT64_MAX : 0;
            *mediaStatus = MEDIA_HDD_FORMAT;
        }
        return FALSE;

        case PLAYBACK_FILE_READ_OVER:
        {
            *mediaStatus = MEDIA_PLAYBACK_OVER;
            DPRINT(PLAYBACK_MEDIA, "file read over: [camera=%d]", camIndex);
        }
        return FALSE;

        default:
        {
            *mediaStatus = MEDIA_OTHER_ERROR;
        }
        return FALSE;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function sets all playback position to given time for given direction
 * @param   playParams
 * @param   playInfo
 * @param   stepTime
 * @return
 */
static BOOL SetSyncPlayPosition(SYNC_PLAY_PARAMS_t *playParams, SYNC_PLAYBACK_INFO_t *playInfo, struct tm *stepTime)
{
    BOOL				retVal = SUCCESS;
    NET_CMD_STATUS_e	cmdStatus = CMD_PROCESS_ERROR;
    BOOL				setPosition, openPlaySession;
    BOOL				trySearch = FALSE;
    UINT8				camIndex = 0;
    time_t				localTime;
    UINT8               noCameraRecordCnt = 0;
    BOOL                findNearestRecord;
    UINT16				index = 0;
    struct tm			setPositionTime;

    ConvertLocalTimeInSec(stepTime, &localTime);

    for (camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
    {
        setPosition = openPlaySession = FALSE;
        setPositionTime = *stepTime;
        if (TRUE == playParams->cameraRecordF[camIndex])
        {
            do
            {
                DPRINT(PLAYBACK_MEDIA, "search: [camera=%d], [searchList=%d], [currentList=%d]",
                       camIndex, playInfo->camListInfo[camIndex].srchRsltListLen, playInfo->camListInfo[camIndex].curRecFromList);

                /* Now, we need to check new position is in between current record */
                if((playInfo->camListInfo[camIndex].srchRsltListLen > 0)
                    && (playInfo->camListInfo[camIndex].searchListInfo[playInfo->camListInfo[camIndex].curRecFromList].startTime <= localTime)
                    && (playInfo->camListInfo[camIndex].searchListInfo[playInfo->camListInfo[camIndex].curRecFromList].stopTime >= localTime))
                {
                    if(playInfo->camListInfo[camIndex].dmPlaySessId == INVALID_PLAY_SESSION_HANDLE)
                    {
                        DPRINT(PLAYBACK_MEDIA, "open record session: [camera=%d]", camIndex);
                        openPlaySession = TRUE;
                    }
                    else
                    {
                        playInfo->camListInfo[camIndex].nextFrameTime = (UINT64)((UINT64)localTime * MSEC_IN_ONE_SEC);
                        setPosition = TRUE;
                    }

                    trySearch = FALSE;
                    break;
                }

                if(playInfo->camListInfo[camIndex].dmPlaySessId != INVALID_PLAY_SESSION_HANDLE)
                {
                    /* New position is not in between current record so closed this play session */
                    ClosePlaySession(playInfo->camListInfo[camIndex].dmPlaySessId);
                    DPRINT(PLAYBACK_MEDIA, "close playback session: [camera=%d] [dmPlaySessId=%d]", camIndex, playInfo->camListInfo[camIndex].dmPlaySessId);
                    playInfo->camListInfo[camIndex].dmPlaySessId = INVALID_PLAY_SESSION_HANDLE;
                }

                /* try to search record for given time duration */
                trySearch = TRUE;
                playInfo->camListInfo[camIndex].curRecFromList++;

            } while(playInfo->camListInfo[camIndex].curRecFromList < playInfo->camListInfo[camIndex].srchRsltListLen);

            if(trySearch == TRUE)
            {
                trySearch = FALSE;

                if(playParams->direction == PLAY_REVERSE)
                {
                    setPositionTime.tm_hour = setPositionTime.tm_hour + 1;
                    setPositionTime.tm_min = 0;
                    setPositionTime.tm_sec = 0;
                }

                if (FillRecordByHour(playParams, playInfo, &setPositionTime, camIndex) == SUCCESS)
                {
                    if((playParams->direction == PLAY_FORWARD) || (playParams->direction == STEP_FORWARD))
                    {
                        index = 0;
                    }
                    else
                    {
                        index = (playInfo->camListInfo[camIndex].srchRsltListLen - 1);
                    }

                    /* Find whether current time is present in any record */
                    findNearestRecord = TRUE;

                    do
                    {
                        if ((playInfo->camListInfo[camIndex].searchListInfo[index].startTime <= localTime) &&
                                (playInfo->camListInfo[camIndex].searchListInfo[index].stopTime >= localTime))
                        {
                            playInfo->camListInfo[camIndex].curRecFromList = index;
                            openPlaySession = TRUE;
                            findNearestRecord = FALSE;
                            playInfo->camListInfo[camIndex].nextFrameTime = localTime;
                            DPRINT(PLAYBACK_MEDIA, "open record session: [camera=%d], [record=%d]", camIndex, index);
                            break;
                        }

                        if((playParams->direction == PLAY_FORWARD)	|| (playParams->direction == STEP_FORWARD))
                        {
                            if (index >= (playInfo->camListInfo[camIndex].srchRsltListLen - 1))
                            {
                                break;
                            }

                            index++;
                        }
                        else
                        {
                            if (index <= 0)
                            {
                                break;
                            }

                            index--;
                        }

                    }while(TRUE);

                    /* Find Nearest Record */
                    if (TRUE == findNearestRecord)
                    {
                        if((playParams->direction == PLAY_FORWARD)	|| (playParams->direction == STEP_FORWARD))
                        {
                            index = 0;
                        }
                        else
                        {
                            index = (playInfo->camListInfo[camIndex].srchRsltListLen - 1);
                        }

                        /* Find whether current time is present in any record */
                        do
                        {
                            if((playParams->direction == PLAY_FORWARD)	|| (playParams->direction == STEP_FORWARD))
                            {
                                if(playInfo->camListInfo[camIndex].searchListInfo[index].startTime > localTime)
                                {
                                    playInfo->camListInfo[camIndex].curRecFromList=index;
                                    openPlaySession = TRUE;
                                    playInfo->camListInfo[camIndex].nextFrameTime = playInfo->camListInfo[camIndex].searchListInfo[index].startTime;
                                    DPRINT(PLAYBACK_MEDIA, "open record session: [camera=%d]", camIndex);
                                    break;
                                }

                                if (index >= (playInfo->camListInfo[camIndex].srchRsltListLen - 1))
                                {
                                    break;
                                }

                                index++;
                            }
                            else
                            {
                                if(playInfo->camListInfo[camIndex].searchListInfo[index].stopTime < localTime)
                                {
                                    playInfo->camListInfo[camIndex].curRecFromList=index;
                                    openPlaySession = TRUE;
                                    playInfo->camListInfo[camIndex].nextFrameTime = playInfo->camListInfo[camIndex].searchListInfo[index].stopTime;
                                    DPRINT(PLAYBACK_MEDIA, "open record session: [camera=%d]", camIndex);
                                    break;
                                }

                                if (index <= 0)
                                {
                                    break;
                                }

                                index--;
                            }

                        }while(TRUE);
                    }
                }
                else
                {
                    EPRINT(PLAYBACK_MEDIA, "no record found onward: [camera=%d], [time=%02d/%02d/%04d %02d:%02d:%02d]",
                           camIndex, setPositionTime.tm_mday, setPositionTime.tm_mon + 1,
                           setPositionTime.tm_year, setPositionTime.tm_hour, setPositionTime.tm_min, setPositionTime.tm_sec);
                }
            }

            if((openPlaySession == FALSE) && (setPosition == FALSE))
            {
                EPRINT(PLAYBACK_MEDIA, "no record found onward: [camera=%d], [time=%02d/%02d/%04d %02d:%02d:%02d]",
                       camIndex, setPositionTime.tm_mday, setPositionTime.tm_mon + 1,
                       setPositionTime.tm_year, setPositionTime.tm_hour, setPositionTime.tm_min, setPositionTime.tm_sec);

                playInfo->camListInfo[camIndex].nextFrameTime = (playParams->direction == PLAY_FORWARD) ? UINT64_MAX : 0;
                playInfo->camListInfo[camIndex].playbackStatus = PLAYBACK_FILE_READ_OVER;
                noCameraRecordCnt++;
            }

            if (openPlaySession == TRUE)
            {
                openPlaySession = FALSE;
                cmdStatus = OpenPlaySessionAndSetPosition(&playInfo->camListInfo[camIndex].searchListInfo[playInfo->camListInfo[camIndex].curRecFromList],
                        FALSE, playParams->cameraAudioF[camIndex], playParams->speed, playParams->direction,
                        &playInfo->camListInfo[camIndex].nextFrameTime, &playInfo->camListInfo[camIndex].dmPlaySessId);
                if(cmdStatus == CMD_SUCCESS)
                {
                    /* Check this record is in between given set time position */
                    setPosition = TRUE;
                    if ((playInfo->camListInfo[camIndex].searchListInfo[playInfo->camListInfo[camIndex].curRecFromList].startTime <= localTime) &&
                            (playInfo->camListInfo[camIndex].searchListInfo[playInfo->camListInfo[camIndex].curRecFromList].stopTime >= localTime))
                    {
                        setPositionTime = *stepTime;
                    }
                    else
                    {
                        if(playParams->direction == PLAY_FORWARD)
                        {
                            ConvertLocalTimeInBrokenTm(&playInfo->camListInfo[camIndex].
                                searchListInfo[playInfo->camListInfo[camIndex].curRecFromList].startTime, &setPositionTime);
                        }
                        else if(playParams->direction == PLAY_REVERSE)
                        {
                            ConvertLocalTimeInBrokenTm(&playInfo->camListInfo[camIndex].
                                searchListInfo[playInfo->camListInfo[camIndex].curRecFromList].stopTime, &setPositionTime);
                        }
                    }
                }
                else
                {
                    if(cmdStatus == CMD_MAX_STREAM_LIMIT)
                    {
                        playInfo->camListInfo[camIndex].curPlaybackStatus = PLAYBACK_FILE_READ_OVER;
                        retVal = SUCCESS;
                    }
                    else
                    {
                        retVal = FAIL;
                    }
                    EPRINT(PLAYBACK_MEDIA, "play session not open: [camera=%d], [nextFrameTime=%lld]",
                           camIndex, playInfo->camListInfo[camIndex].nextFrameTime);
                }
            }

            if(setPosition == TRUE)
            {
                setPosition = FALSE;

                DPRINT(PLAYBACK_MEDIA, "set play position: [camera=%d], [time=%02d:%02d:%02d], [isAudio=%d], [speed=%d]",
                       camIndex, setPositionTime.tm_hour, setPositionTime.tm_min, setPositionTime.tm_sec, playParams->cameraAudioF[camIndex], playParams->speed);

                if (setPositionTime.tm_hour == HOUR_IN_ONE_DAY)
                {
                    setPositionTime.tm_hour = (HOUR_IN_ONE_DAY - 1);
                    setPositionTime.tm_min = (MIN_IN_ONE_HOUR - 1);
                    setPositionTime.tm_sec = (SEC_IN_ONE_MIN - 1);
                }

                if (SetPlayPosition(&setPositionTime, 0, playParams->cameraAudioF[camIndex], playParams->direction,
                                    playParams->speed, playInfo->camListInfo[camIndex].dmPlaySessId) == FAIL)
                {
                    EPRINT(PLAYBACK_MEDIA, "fail to set play position: [camera=%d]", camIndex);
                    retVal = FAIL;
                    break;
                }
                else
                {
                    playInfo->camListInfo[camIndex].curPlaybackStatus = PLAYBACK_FILE_READ_NORMAL;
                }
            }
        }
        else
        {
            if(playInfo->camListInfo[camIndex].dmPlaySessId != INVALID_PLAY_SESSION_HANDLE)
            {
                ClosePlaySession(playInfo->camListInfo[camIndex].dmPlaySessId);
                playInfo->camListInfo[camIndex].dmPlaySessId = INVALID_PLAY_SESSION_HANDLE;
                playInfo->camListInfo[camIndex].curRecFromList = 0;
                playInfo->camListInfo[camIndex].srchRsltListLen = 0;
                playInfo->camListInfo[camIndex].nextFrameTime = (playParams->direction == PLAY_FORWARD) ? UINT64_MAX : 0;
            }
        }

        if (noCameraRecordCnt == playParams->totalCamera)
        {
            retVal = FAIL;
            EPRINT(PLAYBACK_MEDIA, "no record found for given time position: [totalCamera=%d]", playParams->totalCamera);
            break;
        }
    }

    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function open disk manager session for given search result.
 * @param   searchData
 * @param   setPlayPosition
 * @param   audioEnbl
 * @param   speed
 * @param   direction
 * @param   nextFrameTime
 * @param   recStorageDrive
 * @param   playId
 * @return
 */
static NET_CMD_STATUS_e OpenPlaySessionAndSetPosition(SEARCH_DATA_t *searchData, BOOL setPlayPosition, BOOL audioEnbl, UINT8 speed,
                                                      PLAYBACK_CMD_e direction, UINT64PTR nextFrameTime, PLAY_SESSION_ID *playId)
{
    NET_CMD_STATUS_e	retVal = CMD_SUCCESS;
    PLAY_CNTRL_INFO_t	playCntrlInfo = { .startTime = { 0 }, .stopTime = { 0 }};
    struct tm			*timeStamp;

    playCntrlInfo.camNo = searchData->camNo;
    playCntrlInfo.diskId = searchData->diskId;
    playCntrlInfo.eventType = searchData->eventType;
    playCntrlInfo.overlapFlg = searchData->overlapFlg;
    playCntrlInfo.recStorageType = searchData->recStorageType;
    ConvertLocalTimeInBrokenTm(&searchData->startTime, &playCntrlInfo.startTime);
    ConvertLocalTimeInBrokenTm(&searchData->stopTime, &playCntrlInfo.stopTime);

    DPRINT(PLAYBACK_MEDIA, "play session: [camera=%d], [diskId=%d], [eventType=%d], [overlapFlg=%d], "
           "[startTime=%02d/%02d/%04d %02d:%02d:%02d], [stopTime=%02d/%02d/%04d %02d:%02d:%02d]",
           playCntrlInfo.camNo-1, playCntrlInfo.diskId, playCntrlInfo.eventType, playCntrlInfo.overlapFlg,
           playCntrlInfo.startTime.tm_mday, playCntrlInfo.startTime.tm_mon+1,
           playCntrlInfo.startTime.tm_year, playCntrlInfo.startTime.tm_hour,
           playCntrlInfo.startTime.tm_min, playCntrlInfo.startTime.tm_sec,
           playCntrlInfo.stopTime.tm_mday, playCntrlInfo.stopTime.tm_mon+1,
           playCntrlInfo.stopTime.tm_year, playCntrlInfo.stopTime.tm_hour,
           playCntrlInfo.stopTime.tm_min, playCntrlInfo.stopTime.tm_sec);

    retVal = OpenPlaySession(&playCntrlInfo, PLAYBACK_PLAY, playId);
    if(retVal == CMD_SUCCESS)
    {
        DPRINT(PLAYBACK_MEDIA, "sync playback DM session opened: [camera=%d], [dmSession=%d]", playCntrlInfo.camNo-1, *playId);

        if(direction == PLAY_FORWARD)
        {
            timeStamp = &playCntrlInfo.startTime;
            *nextFrameTime = ((UINT64)searchData->startTime * MSEC_IN_ONE_SEC);
        }
        else
        {
            timeStamp = &playCntrlInfo.stopTime;
            *nextFrameTime = ((UINT64)searchData->stopTime * MSEC_IN_ONE_SEC);
        }

        if(setPlayPosition == TRUE)
        {
            if (SetPlayPosition(timeStamp, 0, audioEnbl, direction, speed, *playId) == FAIL)
            {
                EPRINT(PLAYBACK_MEDIA, "fail to set play position: [camera=%d]", playCntrlInfo.camNo-1);
                retVal = CMD_PROCESS_ERROR;
                *nextFrameTime = (direction == PLAY_FORWARD) ? UINT64_MAX : 0;
            }
        }
    }
    else
    {
        EPRINT(PLAYBACK_MEDIA, "fail to open playback DM session: [camera=%d]", playCntrlInfo.camNo-1);
        if(retVal != CMD_MAX_STREAM_LIMIT)
        {
            *nextFrameTime = (direction == PLAY_FORWARD) ? UINT64_MAX : 0;
        }
    }
    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function open disk manager session for next consecutive record for particular camera.
 * @param   camIndex
 * @param   playParams
 * @param   infoPtr
 * @return
 */
static NET_CMD_STATUS_e OpenNextRecord(UINT8 camIndex, SYNC_PLAY_PARAMS_t *playParams, SYNC_PLAYBACK_INFO_t *infoPtr)
{
    BOOL					openPlaySession = FALSE, setPlayPos = FALSE;
    UINT16					searchNo;
    NET_CMD_STATUS_e		cmdStatus = CMD_SUCCESS;
    struct tm				brokenTime = { 0 };

    ClosePlaySession(infoPtr->camListInfo[camIndex].dmPlaySessId);
    DPRINT(PLAYBACK_MEDIA, "close playback session: [camera=%d] [dmPlaySessId=%d]", camIndex, infoPtr->camListInfo[camIndex].dmPlaySessId);
    infoPtr->camListInfo[camIndex].dmPlaySessId = INVALID_PLAY_SESSION_HANDLE;

    if (playParams->direction == PLAY_FORWARD)
    {
        infoPtr->camListInfo[camIndex].curRecFromList++;
        DPRINT(PLAYBACK_MEDIA, "playback record: [current=%d], [total=%d]",
               infoPtr->camListInfo[camIndex].curRecFromList, infoPtr->camListInfo[camIndex].srchRsltListLen);

        if(infoPtr->camListInfo[camIndex].curRecFromList < infoPtr->camListInfo[camIndex].srchRsltListLen)
        {
            openPlaySession = TRUE;
        }
        else
        {
            infoPtr->camListInfo[camIndex].curRecFromList--;
        }
    }
    else
    {
        if(infoPtr->camListInfo[camIndex].curRecFromList > 0)
        {
            infoPtr->camListInfo[camIndex].curRecFromList--;
            openPlaySession = TRUE;
            DPRINT(PLAYBACK_MEDIA, "playback record: [current=%d], [total=%d]",
                   infoPtr->camListInfo[camIndex].curRecFromList, infoPtr->camListInfo[camIndex].srchRsltListLen);
        }
    }

    if (FALSE == openPlaySession)
    {
        // Hard Coded First Record
        ConvertLocalTimeInBrokenTm(&infoPtr->camListInfo[camIndex].searchListInfo[infoPtr->camListInfo[camIndex].curRecFromList].startTime, &brokenTime);

        brokenTime.tm_min = 0;
        brokenTime.tm_sec = 0;

        if(playParams->direction == PLAY_FORWARD)
        {
            brokenTime.tm_hour++;
        }

        if(FillRecordByHour(playParams, infoPtr, &brokenTime, camIndex) == SUCCESS)
        {
            DPRINT(PLAYBACK_MEDIA, "next frame: [camera=%d], [time=%lld]", camIndex, infoPtr->camListInfo[camIndex].nextFrameTime);
            openPlaySession = TRUE;
        }
        else
        {
            EPRINT(PLAYBACK_MEDIA, "no record found onward: [camera=%d], [hour=%02d]", camIndex, brokenTime.tm_hour);
            infoPtr->camListInfo[camIndex].nextFrameTime = (playParams->direction == PLAY_FORWARD) ? UINT64_MAX : 0;
        }
    }

    if (FALSE == openPlaySession)
    {
        return CMD_SUCCESS;
    }

    searchNo = infoPtr->camListInfo[camIndex].curRecFromList;
    cmdStatus = OpenPlaySessionAndSetPosition(&infoPtr->camListInfo[camIndex].searchListInfo[searchNo], TRUE,
                                              playParams->cameraAudioF[camIndex], playParams->speed,
                                              playParams->direction, &infoPtr->camListInfo[camIndex].nextFrameTime,
                                              &infoPtr->camListInfo[camIndex].dmPlaySessId);
    if (cmdStatus != CMD_SUCCESS)
    {
        if(cmdStatus != CMD_MAX_STREAM_LIMIT)
        {
            infoPtr->camListInfo[camIndex].nextFrameTime = (playParams->direction == PLAY_FORWARD) ? UINT64_MAX : 0;
            infoPtr->camListInfo[camIndex].curPlaybackStatus = PLAYBACK_FILE_READ_OVER;
            infoPtr->camListInfo[camIndex].playbackStatus = PLAYBACK_FILE_READ_OVER;
        }
        return cmdStatus;
    }

    DPRINT(PLAYBACK_MEDIA, "search result: [camera=%d], [searchNo=%d], [resultLength=%d], [direction=%d]",
           camIndex, searchNo, infoPtr->camListInfo[camIndex].srchRsltListLen, playParams->direction);
    if((searchNo != 0) && (playParams->direction == PLAY_FORWARD))
    {
        if(infoPtr->camListInfo[camIndex].searchListInfo[searchNo].startTime < infoPtr->camListInfo[camIndex].searchListInfo[searchNo - 1].stopTime)
        {
            setPlayPos = TRUE;
            if (infoPtr->camListInfo[camIndex].searchListInfo[searchNo - 1].stopTime > infoPtr->camListInfo[camIndex].searchListInfo[searchNo].stopTime)
            {
                ConvertLocalTimeInBrokenTm(&infoPtr->camListInfo[camIndex].searchListInfo[searchNo].startTime, &brokenTime);
            }
            else
            {
                ConvertLocalTimeInBrokenTm(&infoPtr->camListInfo[camIndex].searchListInfo[searchNo - 1].stopTime, &brokenTime);
            }
        }
    }
    else if ((searchNo != infoPtr->camListInfo[camIndex].srchRsltListLen) && (playParams->direction == PLAY_REVERSE))
    {
        if (((infoPtr->camListInfo[camIndex].srchRsltListLen - searchNo) > 1) &&
                (infoPtr->camListInfo[camIndex].searchListInfo[searchNo].stopTime > infoPtr->camListInfo[camIndex].searchListInfo[searchNo + 1].startTime))
        {
            setPlayPos = TRUE;
            ConvertLocalTimeInBrokenTm(&infoPtr->camListInfo[camIndex].searchListInfo[searchNo + 1].startTime, &brokenTime);
        }
        else
        {
            ConvertLocalTimeInBrokenTm(&infoPtr->camListInfo[camIndex].searchListInfo[searchNo].stopTime, &brokenTime);
        }
    }

    if (setPlayPos == TRUE)
    {
        if (SetPlayPosition(&brokenTime, 0, playParams->cameraAudioF[camIndex], playParams->direction,
                            playParams->speed, infoPtr->camListInfo[camIndex].dmPlaySessId) == FAIL)
        {
            EPRINT(PLAYBACK_MEDIA, "set play position failed: [camera=%d]", camIndex);
            cmdStatus = CMD_PROCESS_ERROR;
            infoPtr->camListInfo[camIndex].nextFrameTime = (playParams->direction == PLAY_FORWARD) ? UINT64_MAX : 0;
        }
    }

    infoPtr->camListInfo[camIndex].curPlaybackStatus = PLAYBACK_FILE_READ_NORMAL;
    infoPtr->camListInfo[camIndex].playbackStatus = PLAYBACK_FILE_READ_NORMAL;
    return cmdStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function check all camera has completed it playback.
 * @param   playParams
 * @param   playInfo
 * @return
 */
static BOOL IsAllPlaybackOver(SYNC_PLAY_PARAMS_t *playParams, SYNC_PLAYBACK_INFO_t *playInfo)
{
    UINT8 camIndex;

    for (camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
    {
        if (TRUE == playParams->cameraRecordF[camIndex])
        {
            if (playInfo->camListInfo[camIndex].playbackStatus != PLAYBACK_FILE_READ_OVER)
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function generates frame header for given type of error.
 * @param   clientCbType
 * @param   camIndex
 * @param   sockFd
 * @param   mediaStatus
 * @param   frameSyncNum
 * @param   frameTime
 * @return
 */
static BOOL SendPlaybackFrameHeader(CLIENT_CB_TYPE_e clientCbType, UINT8 camIndex, INT32 sockFd,
                                    MEDIA_STATUS_e mediaStatus, UINT8 frameSyncNum, UINT64 frameTime)
{
    FRAME_HEADER_t frameHeader;

    frameHeader.magicCode           = MAGIC_CODE;
    frameHeader.headerVersion       = HEADER_VERSION;
    frameHeader.productType         = PRODUCT_TYPE;
    frameHeader.mediaFrmLen         = FRAME_HEADER_LEN_MAX;
    frameHeader.chNo                = GET_CAMERA_NO(camIndex);
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
    frameHeader.frmSyncNum          = frameSyncNum;
    frameHeader.preReserverMediaLen = 0;
    if (frameTime)
    {
        frameHeader.localTime.totalSec  = (frameTime / 1000);
        frameHeader.localTime.mSec      = (frameTime % 1000);
    }
    else
    {
        frameHeader.localTime.totalSec  = 0;
        frameHeader.localTime.mSec		= 0;
    }

    if (SUCCESS != sendDataCb[clientCbType](sockFd, (UINT8PTR)&frameHeader, FRAME_HEADER_LEN_MAX, SEND_PACKET_TIMEOUT))
    {
        EPRINT(PLAYBACK_MEDIA, "fail to send sync playback frame header: [camera=%d], [mediaStatus=%d]", camIndex, mediaStatus);
        return FALSE;
    }

    DPRINT(PLAYBACK_MEDIA, "sync playback frame header sent: [camera=%d], [mediaStatus=%d]", camIndex, mediaStatus);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   SortSearchData
 * @param   searchData
 * @param   searchRslt
 * @param   srchDataLen
 * @param   maxSortLen
 * @param   resltSort
 * @return
 */
static BOOL	SortSearchData(SEARCH_DATA_t *searchData, SEARCH_DATA_t *searchRslt, UINT16 srchDataLen, UINT16 maxSortLen, UINT16PTR resltSort)
{
    UINT16  searchCnt, index, lowestId = 0;
    time_t  startTime, defRefTime;

    /* Set max default value of time_t variable */
    memset(&defRefTime, 0xFF, sizeof(defRefTime));
    for (searchCnt = 0; searchCnt < srchDataLen; searchCnt++)
    {
        if(*resltSort >= maxSortLen)
        {
            break;
        }

        /* Init with default on starting of sort operation */
        startTime = defRefTime;
        for(index = 0; index < srchDataLen; index++)
        {
            if (searchData[index].startTime == defRefTime)
            {
                continue;
            }

            if ((startTime == defRefTime) || (startTime > searchData[index].startTime))
            {
                startTime = searchData[index].startTime;
                lowestId = index;
            }
        }

        memcpy(&searchRslt[*resltSort], &searchData[lowestId], sizeof(SEARCH_DATA_t));
        searchData[lowestId].startTime = defRefTime;
        (*resltSort)++;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It provides playback session if allocated to current playback client.
 * @param   clientSessionId
 * @return  Returns valid session if allocated to client else returns invalid (max) session
 */
static UINT8 GetSyncPlaybackSessionIndex(UINT8 clientSessionId)
{
    UINT8 syncPlayIndex;

    MUTEX_LOCK(playbackListLock);
    for(syncPlayIndex = 0; syncPlayIndex < MAX_SYNC_PLAYBACK_SESSION; syncPlayIndex++)
    {
        /* Check session is already allocated or not to current playback client */
        if ((syncPlaybackList[syncPlayIndex].pbClientId == clientSessionId) && (syncPlaybackList[syncPlayIndex].request == BUSY))
        {
            /* Session already allocated to client */
            break;
        }
    }
    MUTEX_UNLOCK(playbackListLock);

    return syncPlayIndex;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It provides sync playback session fail cause based on storage drive status
 * @return  Returns status based on storage drive status
 */
static NET_CMD_STATUS_e GetSyncPlaybackFailCause(void)
{
    DISK_ACT_e status = GetStorageDriveStatus(MAX_RECORDING_MODE);
    switch (status)
    {
        case DISK_ACT_NORMAL:
            return CMD_PROCESS_ERROR;

        case DISK_ACT_CONFIG_CHANGE:
            return CMD_REC_DRIVE_CONFIG_CHANGE;

        case DISK_ACT_FORMAT:
            return CMD_FORMAT_IN_PROCESS;

        default:
            return CMD_REC_MEDIA_ERR;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It provides camera privilege for sync playback for given cameras and also store in data structures
 * @param   pPlayInfo
 * @param   pSyncPlayParams
 * @param   pCameraNoPrivilegeRespF
 */
static void StoreCameraSyncPlaybackPrivilege(SYNC_PLAYBACK_INFO_t *pPlayInfo, SYNC_PLAY_PARAMS_t *pSyncPlayParams, BOOL *pCameraNoPrivilegeRespF)
{
    UINT8                   camIndex;
    USER_ACCOUNT_CONFIG_t   userAccountConfig;

    ReadSingleUserAccountConfig(pPlayInfo->userId, &userAccountConfig);
    for (camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
    {
        pCameraNoPrivilegeRespF[camIndex] = FALSE;
        if (FALSE == pSyncPlayParams->cameraRecordF[camIndex])
        {
            continue;
        }

        if (userAccountConfig.userPrivilege[camIndex].privilegeBitField.playback == DISABLE)
        {
            pCameraNoPrivilegeRespF[camIndex] = TRUE;
            pSyncPlayParams->cameraRecordF[camIndex] = FALSE;
        }
    }

    MUTEX_LOCK(pPlayInfo->playBackListMutex);
    memcpy(pPlayInfo->syncPlayParams.cameraRecordF, pSyncPlayParams->cameraRecordF, sizeof(pSyncPlayParams->cameraRecordF));
    MUTEX_UNLOCK(pPlayInfo->playBackListMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It clears disk manager sessions for all cameras which are allocated for this playback session
 * @param   pPlayInfo
 */
static void ClearSyncPlaybackDmSession(SYNC_PLAYBACK_INFO_t *pPlayInfo)
{
    UINT8 camIndex;

    for (camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
    {
        pPlayInfo->camListInfo[camIndex].srchRsltListLen = 0;
        if(pPlayInfo->camListInfo[camIndex].dmPlaySessId != INVALID_PLAY_SESSION_HANDLE)
        {
            ClosePlaySession(pPlayInfo->camListInfo[camIndex].dmPlaySessId);
            DPRINT(PLAYBACK_MEDIA, "close playback session: [camera=%d] [dmPlaySessId=%d]", camIndex, pPlayInfo->camListInfo[camIndex].dmPlaySessId);
            pPlayInfo->camListInfo[camIndex].dmPlaySessId = INVALID_PLAY_SESSION_HANDLE;
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It sends sync playback command's response to playback client
 * @param   pPlaybackMsg
 * @param   response
 * @param   closeSockF
 */
static void SendSyncPlaybackCmdResp(PLAYBACK_MSG_t *pPlaybackMsg, NET_CMD_STATUS_e response, BOOL closeSockF)
{
    if (pPlaybackMsg->cmdRespCallback != NULL)
    {
        pPlaybackMsg->cmdRespCallback(response, pPlaybackMsg->cmdRespFd, closeSockF);
        pPlaybackMsg->cmdRespCallback = NULL;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It sends frame header with stream process error code
 * @param   pbSockFd
 * @param   playParams
 * @param   clientCbType
 * @param   closeConnF
 */
static void SendPbProcErrRespFrameHeader(INT32 *pbSockFd, SYNC_PLAY_PARAMS_t *playParams, UINT8 clientCbType, BOOL closeConnF)
{
    UINT8 camIndex;

    /* Check for all the cameras */
    for (camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
    {
        if (TRUE == playParams->cameraRecordF[camIndex])
        {
            /* Error found. So no need to start play */
            SendPlaybackFrameHeader(clientCbType, camIndex, *pbSockFd, MEDIA_PLAYBACK_PROCESS_ERROR, 0, 0);
            break;
        }
    }

    /* Close socket if required */
    if (TRUE == closeConnF)
    {
        /* Close the socket */
        closeConnCb[clientCbType](pbSockFd);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It provides initial recording for playback
 * @param   pStepTimeSec
 * @param   pPlayInfo
 * @param   pSyncPlayParams
 * @param   pRefFrameTime
 * @param   pCameraNoPrivilegeRespF
 * @return
 */
static BOOL GetInitialRecordByHour(struct tm *pStepTimeSec, SYNC_PLAYBACK_INFO_t *pPlayInfo, SYNC_PLAY_PARAMS_t *pSyncPlayParams,
                                   UINT64 *pRefFrameTime, BOOL *pCameraNoPrivilegeRespF)
{
    time_t      localTime;
    struct tm   brokenTime;

    ConvertLocalTimeInSec(pStepTimeSec, &localTime);
    *pRefFrameTime = ((UINT64)localTime * MSEC_IN_ONE_SEC);

    /* Check and store camera privilege for playback */
    StoreCameraSyncPlaybackPrivilege(pPlayInfo, pSyncPlayParams, pCameraNoPrivilegeRespF);

    memcpy(&brokenTime, pStepTimeSec, sizeof(struct tm));

    if (pSyncPlayParams->direction == PLAY_REVERSE)
    {
        brokenTime.tm_hour = (pStepTimeSec->tm_hour + 1);
        brokenTime.tm_min = 0;
        brokenTime.tm_sec = 0;
    }
    else
    {
        brokenTime.tm_hour = pStepTimeSec->tm_hour;
        brokenTime.tm_min = 0;
        brokenTime.tm_sec = 0;
    }

    //Search record for given time in storage device
    if (FillRecordByHour(pSyncPlayParams, pPlayInfo, &brokenTime, MAX_CAMERA) == FAIL)
    {
        EPRINT(PLAYBACK_MEDIA, "no playback record available: [time=%02d/%02d/%04d %02d:%02d]",
               brokenTime.tm_mday, (brokenTime.tm_mon + 1), brokenTime.tm_year, brokenTime.tm_hour, brokenTime.tm_min);
        return FALSE;
    }

    return TRUE;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
