//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		InstantPlaybackMediaStreamer.c
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
/* OS Include */
#include <stdint.h>

/* Application Includes */
#include "InstantPlaybackMediaStreamer.h"
#include "Utils.h"
#include "SysTimer.h"
#include "DebugLog.h"
#include "NetworkManager.h"
#include "DiskController.h"
#include "NetworkCommand.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define MAX_INSTANT_PLAYBACK_SESSION		(getMaxCameraForCurrentVariant())
#define SEARCH_STEP_FOR_INSTANT_PLAY		(10)

// As per SAD how many minutes backward.
#define BACK_MINUTE_TO_START_PLAY			(5)

#define INST_PLAY_THREAD_STACK_SZ           (2*MEGA_BYTE)

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
}INSTANT_CAM_PLAY_INFO_t;

typedef struct
{
    UINT8 					cameraIndex;
	PLAYBACK_CMD_e 			direction;
	EVENT_e					eventType;
	UINT8					audioChannel;
	UINT8					frmSyncNum;
	UINT8					speed;
	NW_FRAME_TYPE_e 		frameType;
}INSTANT_PLAY_PARAMS_t;

typedef struct
{
	BOOL 					request;
	INSTANT_PLAY_PARAMS_t	instantPlayParams;
	time_t					startTimeInSec;
	time_t					endTimeInSec;
	struct tm				startTime;
	struct tm				endTime;
	BOOL					firstFrame;
    UINT8                   clientCbType;
	UINT8					pbClientIndex;
	UINT8			 		clientSessId;
	VOIDPTR					callback;
	INT32					clientSockId;
	pthread_mutex_t 		playBackListMutex;
	PLAYBACK_MSG_QUE_t		pbMagQue;
	pthread_mutex_t 		streamSignalLock;
	pthread_cond_t 			streamSignal;
	UINT8PTR 				frameBuffer;
	INSTANT_CAM_PLAY_INFO_t	camListInfo;
}INSTANT_PLAYBACK_INFO_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static INSTANT_PLAYBACK_INFO_t		instantPlaybackList[MAX_CAMERA];
static pthread_mutex_t 				playbackListLock;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static VOIDPTR instantPlaybackStreamerThread(void *threadArg);
//-------------------------------------------------------------------------------------------------
static BOOL writePbMesg(UINT8 pbIndex, PLAYBACK_MSG_t * pbMsgPtr);
//-------------------------------------------------------------------------------------------------
static BOOL readyFrameToSend(UINT8 camIndex, INSTANT_PLAY_PARAMS_t *pPlayParams, INSTANT_PLAYBACK_INFO_t *pPlayInfo,
                             UINT32PTR length, MEDIA_STATUS_e *mediaStatus);
//-------------------------------------------------------------------------------------------------
static BOOL fillRecordByHour(INSTANT_PLAY_PARAMS_t *playParams, INSTANT_PLAYBACK_INFO_t *playInfo,
                             struct tm *startTime, struct tm *endTime, UINT8 cameraSearch);
//-------------------------------------------------------------------------------------------------
static void updateRefFrameTime(INSTANT_PLAY_PARAMS_t *playParams, INSTANT_PLAYBACK_INFO_t *playInfo, UINT64PTR updatedTime);
//-------------------------------------------------------------------------------------------------
static BOOL setInstantPlayPosition(INSTANT_PLAY_PARAMS_t *playParams, INSTANT_PLAYBACK_INFO_t *playInfo, struct tm *stepTime);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e openPlaySessionAndSetPosition(SEARCH_DATA_t *searchData, BOOL setPlayPosition, BOOL audioEnbl, UINT8 speed,
                                                      PLAYBACK_CMD_e direction, UINT64PTR nextFrameTime, PLAY_SESSION_ID *playId);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e openNextRecord(UINT8 camIndex, INSTANT_PLAY_PARAMS_t *pPlayParams, INSTANT_PLAYBACK_INFO_t *pPlayInfo);
//-------------------------------------------------------------------------------------------------
static void sendPlaybackFrameHeader(CLIENT_CB_TYPE_e clientCbType, UINT8 camIndex, INT32 sockFd, MEDIA_STATUS_e mediaStatus, UINT8 frameSyncNum);
//-------------------------------------------------------------------------------------------------
static BOOL	sortSearchData(SEARCH_DATA_t *searchData, SEARCH_DATA_t *searchRslt, UINT16 srchDataLen, UINT16 maxSortLen, UINT16PTR resltSort);
//-------------------------------------------------------------------------------------------------
static void cleanupInstantDataInfo(UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API initializes the play-back media streamer module.
 */
void InitInstantPlaybackStreamer(void)
{
    UINT8 sessionIndex;

	// Initialize locks and signals
    MUTEX_INIT(playbackListLock, NULL);

	for(sessionIndex = 0; sessionIndex < MAX_INSTANT_PLAYBACK_SESSION; sessionIndex++)
	{
        MUTEX_INIT(instantPlaybackList[sessionIndex].playBackListMutex, NULL);
        MUTEX_INIT(instantPlaybackList[sessionIndex].streamSignalLock, NULL);
		pthread_cond_init(&instantPlaybackList[sessionIndex].streamSignal, NULL);
		cleanupInstantDataInfo(sessionIndex);
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API initializes the play-back media streamer date structures.
 * @param   sessionIndex
 * @return
 */
static void cleanupInstantDataInfo(UINT8 sessionIndex)
{
    MUTEX_LOCK(instantPlaybackList[sessionIndex].playBackListMutex);
	instantPlaybackList[sessionIndex].instantPlayParams.cameraIndex = 0;
	instantPlaybackList[sessionIndex].instantPlayParams.eventType = 0;
	instantPlaybackList[sessionIndex].instantPlayParams.audioChannel = 0;
	instantPlaybackList[sessionIndex].instantPlayParams.direction = PLAY_FORWARD;
	instantPlaybackList[sessionIndex].instantPlayParams.speed = PLAY_1X;
    MUTEX_UNLOCK(instantPlaybackList[sessionIndex].playBackListMutex);

    instantPlaybackList[sessionIndex].clientCbType = CLIENT_CB_TYPE_MAX;
    instantPlaybackList[sessionIndex].pbClientIndex = sessionIndex;
	instantPlaybackList[sessionIndex].clientSessId = MAX_NW_CLIENT;
	instantPlaybackList[sessionIndex].request = FREE;
	instantPlaybackList[sessionIndex].pbMagQue.readIdx = 0;
	instantPlaybackList[sessionIndex].firstFrame = FALSE;
	instantPlaybackList[sessionIndex].pbMagQue.writeIdx = 0;

	instantPlaybackList[sessionIndex].camListInfo.curRecFromList = 0;
	instantPlaybackList[sessionIndex].camListInfo.dmPlaySessId = INVALID_PLAY_SESSION_HANDLE;
	instantPlaybackList[sessionIndex].camListInfo.nextFrameTime = 0;
	instantPlaybackList[sessionIndex].camListInfo.searchListInfo = NULL;
	instantPlaybackList[sessionIndex].camListInfo.searchListLen = SEARCH_STEP_FOR_INSTANT_PLAY;
	instantPlaybackList[sessionIndex].camListInfo.srchRsltListLen = 0;
	instantPlaybackList[sessionIndex].camListInfo.curPlaybackStatus = PLAYBACK_FILE_READ_NORMAL;
	instantPlaybackList[sessionIndex].camListInfo.playbackStatus = PLAYBACK_FILE_READ_NORMAL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API adds request to the play-back stream list, if free any, and outputs the streamID.
 * @param   clientCbType
 * @param   startTime
 * @param   cameraIndex
 * @param   direction
 * @param   audioEnable
 * @param   sessionId
 * @param   userIndex
 * @param   callBack
 * @param   clientSocket
 * @param   playbackId
 * @return
 */
NET_CMD_STATUS_e AddInstantPlaybackStream(CLIENT_CB_TYPE_e clientCbType, struct tm startTime, UINT32 cameraIndex, UINT8 sessionId,
                                          NW_CMD_REPLY_CB callBack, INT32 clientSocket, INSTANT_PLAYBACK_ID *playbackId)
{
    PLAYBACK_STATE_e    playState = PS_PLAY_STREAM;
    UINT8               sessionIndex = 0;
    PLAYBACK_MSG_t      pbMessg;
    struct tm           endTime;
    time_t              startTimeInSec = 0;
    time_t              endTimeInSec = 0;

    /* Get the disk status and check */
    if (FALSE == IsStorageOperationalForRead(MAX_RECORDING_MODE))
    {
        NET_CMD_STATUS_e retVal = GiveMediaStatus(MAX_RECORDING_MODE);
        EPRINT(PLAYBACK_MEDIA, "disk is not operational: [diskStatus=%d]", retVal);
        return retVal;
    }

    MUTEX_LOCK(playbackListLock);
    for (sessionIndex = 0; sessionIndex < MAX_INSTANT_PLAYBACK_SESSION; sessionIndex ++)
    {
        if(instantPlaybackList[sessionIndex].request == FREE)
        {
            instantPlaybackList[sessionIndex].request = BUSY;
            break;
        }
    }
    MUTEX_UNLOCK(playbackListLock);

    if(sessionIndex >= MAX_INSTANT_PLAYBACK_SESSION)
    {
        EPRINT(PLAYBACK_MEDIA, "instant pb session not available");
        return CMD_MAX_STREAM_LIMIT;
    }

    endTime = startTime;
    ConvertLocalTimeInSec(&endTime, &endTimeInSec);
    startTimeInSec = endTimeInSec - (SEC_IN_ONE_MIN * BACK_MINUTE_TO_START_PLAY);
    ConvertLocalTimeInBrokenTm(&startTimeInSec, &startTime);

    MUTEX_LOCK(instantPlaybackList[sessionIndex].playBackListMutex);
    instantPlaybackList[sessionIndex].clientCbType = clientCbType;
    instantPlaybackList[sessionIndex].instantPlayParams.cameraIndex = cameraIndex;
    instantPlaybackList[sessionIndex].instantPlayParams.direction = PLAY_FORWARD;
    instantPlaybackList[sessionIndex].startTime = startTime;
    instantPlaybackList[sessionIndex].startTimeInSec = startTimeInSec;
    instantPlaybackList[sessionIndex].endTime = endTime;
    instantPlaybackList[sessionIndex].endTimeInSec = endTimeInSec;
    instantPlaybackList[sessionIndex].instantPlayParams.eventType = DM_ANY_EVENT;
    instantPlaybackList[sessionIndex].instantPlayParams.speed = PLAY_1X;
    instantPlaybackList[sessionIndex].instantPlayParams.audioChannel = FALSE;
    instantPlaybackList[sessionIndex].clientSessId = sessionId;
    instantPlaybackList[sessionIndex].instantPlayParams.frameType = NW_ANY_FRAME;
    MUTEX_UNLOCK(instantPlaybackList[sessionIndex].playBackListMutex);

    pbMessg.stepTimeSec = startTime;
    pbMessg.pbState = playState;
    pbMessg.cmdRespCallback = callBack;
    pbMessg.cmdRespFd = clientSocket;

    if(writePbMesg(sessionIndex, &pbMessg) == FAIL)
    {
        EPRINT(PLAYBACK_MEDIA, "fail to write msg in queue");
        return CMD_RESOURCE_LIMIT;
    }

    if(pbMessg.pbState != PS_PLAY_STREAM)
    {
        return CMD_RESOURCE_LIMIT;
    }

    if (FAIL == Utils_CreateThread(NULL, instantPlaybackStreamerThread, &instantPlaybackList[sessionIndex], DETACHED_THREAD, INST_PLAY_THREAD_STACK_SZ))
    {
        EPRINT(PLAYBACK_MEDIA, "fail to create instant playback: [session=%d]", sessionIndex);
        MUTEX_LOCK(playbackListLock);
        instantPlaybackList[sessionIndex].request = FREE;
        MUTEX_UNLOCK(playbackListLock);
        return CMD_RESOURCE_LIMIT;
    }

    *playbackId = sessionIndex;
    DPRINT(PLAYBACK_MEDIA, "%s: [camera=%d], [direction=%d], [eventType=%d], [speed=%d], [audio=%d], [session=%d], [clientId=%d], "
                           "[clientType=%d], [startTime=%02d/%02d/%04d %02d:%02d:%02d], [stopTime=%02d/%02d/%04d %02d:%02d:%02d]",
           playbackSeqStr[pbMessg.pbState], cameraIndex, PLAY_FORWARD, DM_ANY_EVENT, PLAY_1X, FALSE, sessionIndex, sessionId,
           clientCbType, startTime.tm_mday, startTime.tm_mon + 1, startTime.tm_year, startTime.tm_hour, startTime.tm_min, startTime.tm_sec,
            endTime.tm_mday, endTime.tm_mon + 1, endTime.tm_year, endTime.tm_hour, endTime.tm_min, endTime.tm_sec);
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API removes all playback session for all session upon user logout.
 */
void RemoveInstantPlayBackForAllSessionId(void)
{
    UINT8 sessionIdx;

	for(sessionIdx = 0; sessionIdx < MAX_NW_CLIENT; sessionIdx++)
	{
		RemoveInstantPlayBackFromSessionId(sessionIdx);
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API removes all playback session related to this client session ID.
 * @param   sessionId
 */
void RemoveInstantPlayBackFromSessionId(UINT8 sessionId)
{
    PLAYBACK_MSG_t      pbMessg;
    INSTANT_PLAYBACK_ID instantPlayId;

    for (instantPlayId = 0; instantPlayId < MAX_INSTANT_PLAYBACK_SESSION; instantPlayId++)
    {
        /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
        MUTEX_LOCK(playbackListLock);
        if ((instantPlaybackList[instantPlayId].clientSessId != sessionId) || (instantPlaybackList[instantPlayId].request != BUSY))
        {
            MUTEX_UNLOCK(playbackListLock);
            continue;
        }
        MUTEX_UNLOCK(playbackListLock);

        pbMessg.pbState = PS_STOP_STREAM;
        pbMessg.cmdRespCallback = NULL;
        pbMessg.cmdRespFd = INVALID_CONNECTION;

        if (writePbMesg(instantPlayId, &pbMessg) == FAIL)
        {
            EPRINT(PLAYBACK_MEDIA, "fail to add cmd in queue: [session=%d], [client=%d]", instantPlayId, sessionId);
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API removes camera playback session related to this client session ID.
 * @param   sessionId
 * @param   camIndex
 */
void RemoveInstantPlayBackForSessCamId(UINT8 sessionId, UINT8 camIndex)
{
    PLAYBACK_MSG_t      pbMessg;
    INSTANT_PLAYBACK_ID instantPlayId;

    for (instantPlayId = 0; instantPlayId < MAX_INSTANT_PLAYBACK_SESSION; instantPlayId++)
    {
        /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
        MUTEX_LOCK(playbackListLock);
        if ((instantPlaybackList[instantPlayId].clientSessId != sessionId) || (instantPlaybackList[instantPlayId].request != BUSY) ||
                (instantPlaybackList[instantPlayId].instantPlayParams.cameraIndex != camIndex))
        {
            MUTEX_UNLOCK(playbackListLock);
            continue;
        }
        MUTEX_UNLOCK(playbackListLock);

        pbMessg.pbState = PS_STOP_STREAM;
        pbMessg.cmdRespCallback = NULL;
        pbMessg.cmdRespFd = INVALID_CONNECTION;

        if (writePbMesg(instantPlayId, &pbMessg) == FAIL)
        {
            EPRINT(PLAYBACK_MEDIA, "fail to add cmd in queue: [session=%d], [client=%d]", instantPlayId, sessionId);
        }

        /* We can play only one instant playback of one camera */
        break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Pause OR Resume Instant Playback From SessionId
 * @param   sessionId
 * @param   isPausePlaybackF - TRUE=Pause & FALSE=Resume
 */
void PauseResumeInstantPlayBackFromSessionId(UINT8 sessionId, BOOL isPausePlaybackF)
{
    INSTANT_PLAYBACK_ID instantPlayId;

    MUTEX_LOCK(playbackListLock);
    for (instantPlayId = 0; instantPlayId < MAX_INSTANT_PLAYBACK_SESSION; instantPlayId++)
    {
        if ((instantPlaybackList[instantPlayId].clientSessId == sessionId) && (instantPlaybackList[instantPlayId].request == BUSY))
        {
            break;
        }
    }
    MUTEX_UNLOCK(playbackListLock);

    /* Check instant playback is running or not */
    if (instantPlayId >= MAX_INSTANT_PLAYBACK_SESSION)
    {
        /* Instant playback is not running */
        return;
    }

    /* Instant PB is running; Hence, Pause or resume playback */
    if (isPausePlaybackF)
    {
        /* We have to pause playback */
        PauseInstantPlaybackStream(instantPlayId, sessionId, NULL, INVALID_CONNECTION);
    }
    else
    {
        /* We have to resume playback */
        ResumeInstantPlaybackStream(instantPlayId, sessionId, NULL, INVALID_CONNECTION);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API sets play position to given time and direction.
 * @param   instantPlayId
 * @param   sessionId
 * @param   callBack
 * @param   streamFd
 * @param   startTime
 * @param   direction
 * @param   audioChannel
 * @param   frameNo
 * @return
 */
NET_CMD_STATUS_e SetInstantPlaybackPosition(INSTANT_PLAYBACK_ID instantPlayId, UINT8 sessionId, NW_CMD_REPLY_CB callBack, INT32 streamFd,
                                            struct tm startTime, PLAYBACK_CMD_e direction, UINT32 audioChannel, UINT8 frmSyncNum)
{
    PLAYBACK_MSG_t pbMessg;

    if ((instantPlayId >= MAX_INSTANT_PLAYBACK_SESSION) || (instantPlaybackList[instantPlayId].clientSessId != sessionId))
	{
        return CMD_PROCESS_ERROR;
    }

    MUTEX_LOCK(instantPlaybackList[instantPlayId].playBackListMutex);
    instantPlaybackList[instantPlayId].instantPlayParams.direction = direction;
    instantPlaybackList[instantPlayId].instantPlayParams.frmSyncNum = frmSyncNum;
    if (direction == PLAY_FORWARD)
    {
        instantPlaybackList[instantPlayId].instantPlayParams.audioChannel = audioChannel;
    }
    else
    {
        instantPlaybackList[instantPlayId].instantPlayParams.audioChannel = FALSE;
    }
    MUTEX_UNLOCK(instantPlaybackList[instantPlayId].playBackListMutex);

    pbMessg.stepTimeSec = startTime;
    pbMessg.pbState = PS_SET_PLAY_POSITION;
    pbMessg.cmdRespCallback = callBack;
    pbMessg.cmdRespFd = streamFd;

    if (writePbMesg(instantPlayId, &pbMessg) == FAIL)
    {
        EPRINT(PLAYBACK_MEDIA, "fail to add cmd in queue: [session=%d], [client=%d]", instantPlayId, sessionId);
        return CMD_PROCESS_ERROR;
    }

    DPRINT(PLAYBACK_MEDIA, "%s: [session=%d], [direction=%d], [frmSyncNum=%d], [audio=%d], [client=%d], [time=%02d/%02d/%04d %02d:%02d:%02d]",
           playbackSeqStr[pbMessg.pbState], instantPlayId, direction, frmSyncNum, audioChannel, sessionId,
           startTime.tm_mday, startTime.tm_mon + 1, startTime.tm_year, startTime.tm_hour, startTime.tm_min, startTime.tm_sec);
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API sets the stream break flag, which in turn stops the stream of the record.
 * @param   instantPlayId
 * @param   sessionId
 * @param   callback
 * @param   streamFd
 * @return
 */
NET_CMD_STATUS_e PauseInstantPlaybackStream(INSTANT_PLAYBACK_ID instantPlayId, UINT8 sessionId, NW_CMD_REPLY_CB callback, INT32 streamFd)
{
    PLAYBACK_MSG_t pbMessg;

    if ((instantPlayId >= MAX_INSTANT_PLAYBACK_SESSION) || (instantPlaybackList[instantPlayId].clientSessId != sessionId))
    {
        return CMD_PROCESS_ERROR;
    }

    pbMessg.pbState = PS_PAUSE_STREAM;
    pbMessg.cmdRespCallback = callback;
    pbMessg.cmdRespFd = streamFd;

    if (writePbMesg(instantPlayId, &pbMessg) == FAIL)
    {
        EPRINT(PLAYBACK_MEDIA, "fail to add cmd in queue: [session=%d], [client=%d]", instantPlayId, sessionId);
        return CMD_PROCESS_ERROR;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API resumes the streaming of specified paused stream.
 * @param   instantPlayId
 * @param   sessionId
 * @param   callback
 * @param   streamFd
 * @return
 */
NET_CMD_STATUS_e ResumeInstantPlaybackStream(INSTANT_PLAYBACK_ID instantPlayId, UINT8 sessionId, NW_CMD_REPLY_CB callback, INT32 streamFd)
{
    PLAYBACK_MSG_t pbMessg;

    if ((instantPlayId >= MAX_INSTANT_PLAYBACK_SESSION) || (instantPlaybackList[instantPlayId].clientSessId != sessionId))
    {
        return CMD_PROCESS_ERROR;
    }

    pbMessg.pbState = PS_RESUME_STREAM;
    pbMessg.cmdRespCallback = callback;
    pbMessg.cmdRespFd = streamFd;

    if (writePbMesg(instantPlayId, &pbMessg) == FAIL)
    {
        EPRINT(PLAYBACK_MEDIA, "fail to add cmd in queue: [session=%d], [client=%d]", instantPlayId, sessionId);
        return CMD_PROCESS_ERROR;

    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API sets the stream break flag, which in turn stops the stream of the record.
 * @param   instantPlayId
 * @param   sessionId
 * @param   callback
 * @param   streamFd
 * @return
 */
NET_CMD_STATUS_e StopInstantPlaybackStream(INSTANT_PLAYBACK_ID instantPlayId, UINT8 sessionId, NW_CMD_REPLY_CB callback, INT32 streamFd)
{
    PLAYBACK_MSG_t pbMessg;

    if ((instantPlayId >= MAX_INSTANT_PLAYBACK_SESSION) || (instantPlaybackList[instantPlayId].clientSessId != sessionId))
    {
        return CMD_PROCESS_ERROR;
    }

    pbMessg.pbState = PS_STOP_STREAM;
    pbMessg.cmdRespCallback = callback;
    pbMessg.cmdRespFd = streamFd;

    if (writePbMesg(instantPlayId, &pbMessg) == FAIL)
    {
        EPRINT(PLAYBACK_MEDIA, "fail to add cmd in queue: [session=%d], [client=%d]", instantPlayId, sessionId);
        return CMD_PROCESS_ERROR;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API fill next hour record from given time.
 * @param   playParams
 * @param   playInfo
 * @param   startTime
 * @param   endTime
 * @param   cameraSearch
 * @return
 */
static BOOL fillRecordByHour(INSTANT_PLAY_PARAMS_t * playParams, INSTANT_PLAYBACK_INFO_t *playInfo,
                             struct tm *startTime, struct tm *endTime, UINT8 cameraSearch)
{
	BOOL 					retVal = SUCCESS;
	BOOL					status;
	UINT16					sortRsltCnt = 0;
	UINT8					moreData = FALSE, overlapIndicator = FALSE;
	SEARCH_CRITERIA_t		srchCriteria;
	INSTANT_CAM_PLAY_INFO_t	*camInfo;
	SEARCH_DATA_t			*syncSearchData = NULL;

    srchCriteria.startTime = *startTime;
    srchCriteria.endTime = *endTime;
	srchCriteria.eventType = playParams->eventType;
	srchCriteria.channelNo = GET_CAMERA_NO(cameraSearch);
	srchCriteria.searchRecStorageType = MAX_RECORDING_MODE;

	camInfo = &playInfo->camListInfo;
	camInfo->srchRsltListLen = 0;
	camInfo->curRecFromList = 0;
	camInfo->nextFrameTime = 0;

	do
	{
		srchCriteria.noOfRecord = camInfo->searchListLen;
		if(camInfo->searchListInfo == NULL)
		{
            camInfo->searchListInfo = (SEARCH_DATA_t *)malloc(camInfo->searchListLen * (sizeof(SEARCH_DATA_t)));
		}

        if(camInfo->searchListInfo == NULL)
		{
            EPRINT(PLAYBACK_MEDIA, "fail to alloc memory: [camera=%d]", cameraSearch);
            camInfo->srchRsltListLen = 0;
            break;
        }

        // Check Only One event is present for playback
        status = SearchCamAllEventForSync(&srchCriteria, camInfo->searchListInfo, &camInfo->srchRsltListLen, &moreData, &overlapIndicator);
        if (status == FAIL)
        {
            EPRINT(PLAYBACK_MEDIA, "search failed: [camera=%d]", cameraSearch);
            FREE_MEMORY(camInfo->searchListInfo);
            camInfo->searchListLen = SEARCH_STEP_FOR_INSTANT_PLAY;
            camInfo->srchRsltListLen = 0;
            retVal = FAIL;
            break;
        }

        if (moreData == TRUE)
        {
            camInfo->searchListLen += SEARCH_STEP_FOR_INSTANT_PLAY;
            FREE_MEMORY(camInfo->searchListInfo);
            moreData = FALSE;
            continue;
        }

        if(camInfo->srchRsltListLen == 0)
        {
            camInfo->nextFrameTime = ((playParams->direction == PLAY_FORWARD) || (playParams->direction == STEP_FORWARD)) ? UINT64_MAX : 0;
            break;
        }

        syncSearchData = (SEARCH_DATA_t *)malloc((sizeof(SEARCH_DATA_t) * camInfo->searchListLen));
        if(syncSearchData == NULL)
        {
            EPRINT(PLAYBACK_MEDIA, "fail to alloc memory: [camera=%d]", cameraSearch);
            camInfo->srchRsltListLen = 0;
            camInfo->nextFrameTime = ((playParams->direction == PLAY_FORWARD) || (playParams->direction == STEP_FORWARD)) ? UINT64_MAX : 0;
            break;
        }

        // Here we need to sort search result
        sortRsltCnt = 0;
        sortSearchData(camInfo->searchListInfo, syncSearchData, camInfo->srchRsltListLen, camInfo->searchListLen, &sortRsltCnt);
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
            camInfo->nextFrameTime = ((UINT64)camInfo->searchListInfo[0].stopTime * MSEC_IN_ONE_SEC);
            camInfo->curRecFromList = (camInfo->srchRsltListLen - 1);
        }

        // Initialise first frame time
        DPRINT(PLAYBACK_MEDIA,"record search and sorting done: [camera=%d], [nextFrameTime=%lld], [srchRsltListLen=%d]",
               cameraSearch, camInfo->nextFrameTime, camInfo->srchRsltListLen);
        FREE_MEMORY(syncSearchData);
        break;

    } while(retVal == SUCCESS);

	if(camInfo->srchRsltListLen == 0)
	{
		camInfo->playbackStatus = PLAYBACK_FILE_READ_OVER;
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
 * @return
 */
static void updateRefFrameTime(INSTANT_PLAY_PARAMS_t * playParams, INSTANT_PLAYBACK_INFO_t * playInfo, UINT64PTR updatedTime)
{
    UINT64 refFrameTime = ((playParams->direction == PLAY_FORWARD) || (playParams->direction == STEP_FORWARD)) ? UINT64_MAX : 0;

    if((playParams->direction == PLAY_FORWARD) || (playParams->direction == STEP_FORWARD))
    {
        if(refFrameTime >= playInfo->camListInfo.nextFrameTime)
        {
            refFrameTime = playInfo->camListInfo.nextFrameTime;
        }
    }
    else
    {
        if(refFrameTime <= playInfo->camListInfo.nextFrameTime)
        {
            refFrameTime = playInfo->camListInfo.nextFrameTime;
        }
    }

	if(refFrameTime)
	{
		*updatedTime = refFrameTime;
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API actually starts streaming to user. This thread is created during the initialization
 *          of the module and it then waits for the signal to start streaming. Upon receipt of the play
 *          record or step record signal is sent to this thread, it starts reading frames from disk
 *          manager and sends it to user. When stop stream or pause stream is received, stream break
 *          flag is set, which breaks the streaming loop and thread again waits for the signal.
 * @param   threadArg
 * @return
 */
static VOIDPTR instantPlaybackStreamerThread(void *threadArg)
{
    BOOL					isThreadRunning = TRUE;
    UINT64					refFrameTime = 0;
    INSTANT_PLAYBACK_INFO_t *playInfo = ((INSTANT_PLAYBACK_INFO_t *)threadArg);
	UINT32 					length = 0, readIdx;
	INT32 					pbStreamFd;
	NET_CMD_STATUS_e		nwCmdResp = CMD_SUCCESS;
	PLAYBACK_MSG_t			instPbMessage;
	MEDIA_STATUS_e			pbMediaStatus = MEDIA_NORMAL;
	INSTANT_PLAY_PARAMS_t	instantPlayParams;
	struct tm				startTime, endTime;
	CHAR					cmdRespData[60];

    THREAD_START_INDEX("INSTANT_PB", playInfo->pbClientIndex);

    refFrameTime = ((UINT64)playInfo->startTimeInSec * MSEC_IN_ONE_SEC);
	playInfo->camListInfo.dmPlaySessId = INVALID_PLAY_SESSION_HANDLE;
	playInfo->camListInfo.curRecFromList = 0;
	playInfo->camListInfo.searchListLen = SEARCH_STEP_FOR_INSTANT_PLAY;
	playInfo->camListInfo.srchRsltListLen = 0;

    instPbMessage.cmdRespCallback = NULL;
    instPbMessage.cmdRespFd = INVALID_CONNECTION;
    instPbMessage.pbState = PS_STOP_STREAM;
    instPbMessage.responseCode = MEDIA_STATUS_MAX;
    memset(&instPbMessage.stepTimeSec, 0, sizeof instPbMessage.stepTimeSec);

    MUTEX_LOCK(playInfo->playBackListMutex);
    memcpy(&instantPlayParams, &playInfo->instantPlayParams, sizeof(INSTANT_PLAY_PARAMS_t));
    MUTEX_UNLOCK(playInfo->playBackListMutex);

    if (fillRecordByHour(&instantPlayParams, playInfo, &playInfo->startTime, &playInfo->endTime, instantPlayParams.cameraIndex) == FAIL)
	{
        EPRINT(PLAYBACK_MEDIA, "no record available: [camera=%d], [time=%02d/%02d/%04d %02d:%02d:%02d]",
               instantPlayParams.cameraIndex, playInfo->startTime.tm_mday, (playInfo->startTime.tm_mon + 1),
               playInfo->startTime.tm_year, playInfo->startTime.tm_hour, playInfo->startTime.tm_min, playInfo->startTime.tm_sec);

        MUTEX_LOCK(playInfo->streamSignalLock);
		readIdx = playInfo->pbMagQue.readIdx + 1;
		if(readIdx >= PB_MSG_QUEUE_MAX)
		{
			readIdx = 0;
		}
		memcpy(&instPbMessage, &playInfo->pbMagQue.pbMsg[readIdx], sizeof(PLAYBACK_MSG_t));
		playInfo->pbMagQue.readIdx = readIdx;
        MUTEX_UNLOCK(playInfo->streamSignalLock);
		instPbMessage.cmdRespCallback(CMD_INSTANT_PLAYBACK_FAIL, instPbMessage.cmdRespFd, TRUE);
        isThreadRunning = FALSE;
	}

    while(isThreadRunning)
    {
        MUTEX_LOCK(playInfo->streamSignalLock);
        if(playInfo->pbMagQue.readIdx == playInfo->pbMagQue.writeIdx)
        {
            if ((instPbMessage.pbState == PS_PAUSE_STREAM) || (instPbMessage.pbState == PS_STOP_STREAM))
            {
                DPRINT(PLAYBACK_MEDIA, "instant playback sleep: [camera=%d], [state=%s], [session=%d]",
                       instantPlayParams.cameraIndex, playbackSeqStr[instPbMessage.pbState], playInfo->pbClientIndex);
                pthread_cond_wait(&playInfo->streamSignal, &playInfo->streamSignalLock);
                DPRINT(PLAYBACK_MEDIA, "instant playback run: [camera=%d], [state=%s], [session=%d]",
                       instantPlayParams.cameraIndex, playbackSeqStr[instPbMessage.pbState], playInfo->pbClientIndex);
            }
            MUTEX_UNLOCK(playInfo->streamSignalLock);
        }
        else
        {
            readIdx = playInfo->pbMagQue.readIdx + 1;
            if(readIdx >= PB_MSG_QUEUE_MAX)
            {
                readIdx = 0;
            }
            memcpy(&instPbMessage, &playInfo->pbMagQue.pbMsg[readIdx], sizeof(PLAYBACK_MSG_t));
            playInfo->pbMagQue.readIdx = readIdx;
            MUTEX_UNLOCK(playInfo->streamSignalLock);

            DPRINT(PLAYBACK_MEDIA, "instant playback cmd: [camera=%d], [state=%s], [fd=%d], [session=%d]",
                   instantPlayParams.cameraIndex, playbackSeqStr[instPbMessage.pbState], instPbMessage.cmdRespFd, playInfo->pbClientIndex);
        }

        MUTEX_LOCK(playInfo->playBackListMutex);
        memcpy(&instantPlayParams, &playInfo->instantPlayParams, sizeof(INSTANT_PLAY_PARAMS_t));
        MUTEX_UNLOCK(playInfo->playBackListMutex);

        switch(instPbMessage.pbState)
        {
            case PS_RESUME_STREAM:
            {
                if(instPbMessage.cmdRespCallback != NULL)
                {
                    instPbMessage.cmdRespCallback(CMD_SUCCESS, instPbMessage.cmdRespFd, TRUE);
                    instPbMessage.cmdRespCallback = NULL;
                }
            }

            /* Fall through */
            case PS_PLAY_STREAM:
            {
                if(instPbMessage.cmdRespCallback != NULL)
                {
                    if(instPbMessage.pbState == PS_PLAY_STREAM)
                    {
                        pbStreamFd = instPbMessage.cmdRespFd;
                        if (playInfo->startTimeInSec > playInfo->camListInfo.searchListInfo[0].startTime)
                        {
                            startTime = playInfo->startTime;
                        }
                        else
                        {
                            ConvertLocalTimeInBrokenTm(&playInfo->camListInfo.searchListInfo[0].startTime, &startTime);
                        }

                        if (setInstantPlayPosition(&instantPlayParams, playInfo, &startTime) == SUCCESS)
                        {
                            ConvertLocalTimeInBrokenTm(&playInfo->camListInfo.searchListInfo[(playInfo->camListInfo.srchRsltListLen -1)].stopTime, &endTime);
                            snprintf(cmdRespData, sizeof(cmdRespData),
                                    "%c%s%c%03d%c%c%d%c%02d%c%02d%02d%04d%02d%02d%02d%c%02d%02d%04d%02d%02d%02d%c%c%c",
                                    SOM, headerReq[RPL_CMD], FSP, CMD_SUCCESS,
                                    FSP, SOI, 1, FSP, playInfo->pbClientIndex, FSP,
                                    startTime.tm_mday, (startTime.tm_mon + 1), startTime.tm_year, startTime.tm_hour, startTime.tm_min, startTime.tm_sec, FSP,
                                    endTime.tm_mday, (endTime.tm_mon + 1), endTime.tm_year, endTime.tm_hour, endTime.tm_min, endTime.tm_sec, FSP, EOI, EOM);

                            if (sendDataCb[playInfo->clientCbType](pbStreamFd, (UINT8PTR)cmdRespData, strlen(cmdRespData), MESSAGE_REPLY_TIMEOUT) != SUCCESS)
                            {
                                /* Stop instant playback thread */
                                closeConnCb[playInfo->clientCbType](&pbStreamFd);
                                isThreadRunning = FALSE;
                                break;
                            }
                            instPbMessage.cmdRespCallback = NULL;
                        }
                        else
                        {
                            EPRINT(PLAYBACK_MEDIA, "fail to set play position: [camera=%d]", instantPlayParams.cameraIndex);
                            instPbMessage.pbState = PS_STOP_STREAM;
                            instPbMessage.cmdRespCallback(CMD_PROCESS_ERROR, pbStreamFd, TRUE);
                            instPbMessage.cmdRespCallback = NULL;
                            writePbMesg(playInfo->pbClientIndex, &instPbMessage);
                        }
                    }
                }

                if (playInfo->firstFrame == TRUE)
                {
                    playInfo->firstFrame = FALSE;
                    DPRINT(PLAYBACK_MEDIA, "send media sync header: [camera=%d], [frmSyncNum=%d]", instantPlayParams.cameraIndex, instantPlayParams.frmSyncNum);
                    sendPlaybackFrameHeader(playInfo->clientCbType, instantPlayParams.cameraIndex, pbStreamFd,
                                            MEDIA_SYNC_START_INDICATOR, instantPlayParams.frmSyncNum);
                }

                if(playInfo->camListInfo.curPlaybackStatus == PLAYBACK_FILE_READ_OVER)
                {
                    DPRINT(PLAYBACK_MEDIA, "try to open next record: [camera=%d]", instantPlayParams.cameraIndex);
                    nwCmdResp = openNextRecord(instantPlayParams.cameraIndex, &instantPlayParams, playInfo);
                    if(nwCmdResp == CMD_SUCCESS)
                    {
                        updateRefFrameTime(&instantPlayParams, playInfo, &refFrameTime);
                    }
                    else if(nwCmdResp == CMD_MAX_STREAM_LIMIT)
                    {
                        sendPlaybackFrameHeader(playInfo->clientCbType, instantPlayParams.cameraIndex, pbStreamFd, MEDIA_PLAYBACK_SESSION_NOT_AVAIL, 0);
                    }

                    if (playInfo->camListInfo.playbackStatus == PLAYBACK_FILE_READ_OVER)
                    {
                        /* All channel play back over. so, wait for next command */
                        sendPlaybackFrameHeader(playInfo->clientCbType, instantPlayParams.cameraIndex, pbStreamFd, MEDIA_PLAYBACK_OVER, 0);
                        instPbMessage.pbState = PS_PAUSE_STREAM;
                        DPRINT(PLAYBACK_MEDIA, "instant playback over: [camera=%d]", instantPlayParams.cameraIndex);
                        break;
                    }
                }
                else
                {
                    // read record Frame
                    if (TRUE == readyFrameToSend(instantPlayParams.cameraIndex, &instantPlayParams, playInfo, &length, &pbMediaStatus))
                    {
                        if (sendDataCb[playInfo->clientCbType](pbStreamFd, playInfo->frameBuffer, length, SEND_PACKET_TIMEOUT) == FAIL)
                        {
                            EPRINT(PLAYBACK_MEDIA, "fail to send frame: [camera=%d], [sessionIdx=%d], [length=%d]", instantPlayParams.cameraIndex, playInfo->pbClientIndex, length);
                            instPbMessage.pbState = PS_STOP_STREAM;
                            instPbMessage.cmdRespCallback = NULL;
                            break;
                        }
                    }
                    else if (pbMediaStatus != MEDIA_PLAYBACK_OVER)
                    {
                        sendPlaybackFrameHeader(playInfo->clientCbType, instantPlayParams.cameraIndex, pbStreamFd, pbMediaStatus, 0);
                        instPbMessage.pbState = PS_STOP_STREAM;
                        instPbMessage.cmdRespCallback = NULL;
                        break;
                    }
                }
                usleep(10000);
            }
            break;

            case PS_SET_PLAY_POSITION:
            {
                DPRINT(PLAYBACK_MEDIA, "set play position: [camera=%d], [direction=%d], [frameType=%d], [eventType=%d] [time=%02d/%02d/%04d %02d:%02d:%02d]",
                       instantPlayParams.cameraIndex, instantPlayParams.direction, instantPlayParams.frameType, instantPlayParams.eventType,
                       instPbMessage.stepTimeSec.tm_mday, instPbMessage.stepTimeSec.tm_mon + 1, instPbMessage.stepTimeSec.tm_year, instPbMessage.stepTimeSec.tm_hour,
                       instPbMessage.stepTimeSec.tm_min, instPbMessage.stepTimeSec.tm_sec);

                if (setInstantPlayPosition(&instantPlayParams, playInfo, &instPbMessage.stepTimeSec) == SUCCESS)
                {
                    nwCmdResp = CMD_SUCCESS;
                    playInfo->firstFrame  = TRUE;
                    instPbMessage.pbState = PS_PLAY_STREAM;
                }
                else
                {
                    nwCmdResp = CMD_PROCESS_ERROR;
                    instPbMessage.pbState = PS_STOP_STREAM;
                    instPbMessage.cmdRespCallback = NULL;
                    EPRINT(PLAYBACK_MEDIA, "fail to set play position: [camera=%d]", instantPlayParams.cameraIndex);
                }

                if(instPbMessage.cmdRespCallback != NULL)
                {
                    instPbMessage.cmdRespCallback(nwCmdResp, instPbMessage.cmdRespFd, TRUE);
                    instPbMessage.cmdRespCallback = NULL;
                }
            }
            break;

            case PS_PAUSE_STREAM:
            {
                if(instPbMessage.cmdRespCallback != NULL)
                {
                    instPbMessage.cmdRespCallback(CMD_SUCCESS, instPbMessage.cmdRespFd, TRUE);
                    instPbMessage.cmdRespCallback = NULL;
                }
            }
            break;

            case PS_STOP_STREAM:
            {
                if(instPbMessage.cmdRespCallback != NULL)
                {
                    instPbMessage.cmdRespCallback(CMD_SUCCESS, instPbMessage.cmdRespFd, TRUE);
                    instPbMessage.cmdRespCallback = NULL;
                }

                if (playInfo->camListInfo.dmPlaySessId != INVALID_PLAY_SESSION_HANDLE)
                {
                    ClosePlaySession(playInfo->camListInfo.dmPlaySessId);
                    playInfo->camListInfo.dmPlaySessId = INVALID_PLAY_SESSION_HANDLE;
                }

                playInfo->camListInfo.srchRsltListLen = 0;
                closeConnCb[playInfo->clientCbType](&pbStreamFd);

                /* Exit from instant playback thread */
                isThreadRunning = FALSE;
                MUTEX_LOCK(playInfo->streamSignalLock);
                playInfo->pbMagQue.readIdx = 0;
                playInfo->pbMagQue.writeIdx = 0;
                MUTEX_UNLOCK(playInfo->streamSignalLock);
            }
            break;

            default:
            {
                /* Nothing to do */
                EPRINT(PLAYBACK_MEDIA, "invld instant playback state: [clientSession=%d] [state=%d]", playInfo->pbClientIndex, instPbMessage.pbState);
            }
            break;
        }
    }

    FREE_MEMORY(playInfo->camListInfo.searchListInfo);
	cleanupInstantDataInfo(playInfo->pbClientIndex);

    MUTEX_LOCK(playbackListLock);
	playInfo->request = FREE;
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
    UINT32  writeIdx;

    MUTEX_LOCK(instantPlaybackList[pbIndex].streamSignalLock);
	writeIdx = instantPlaybackList[pbIndex].pbMagQue.writeIdx + 1;
	if(writeIdx >= PB_MSG_QUEUE_MAX)
	{
		writeIdx = 0;
	}

    if(writeIdx == instantPlaybackList[pbIndex].pbMagQue.readIdx)
	{
        MUTEX_UNLOCK(instantPlaybackList[pbIndex].streamSignalLock);
        return FAIL;
    }
    memcpy(&instantPlaybackList[pbIndex].pbMagQue.pbMsg[writeIdx], pbMsgPtr, sizeof(PLAYBACK_MSG_t));
    instantPlaybackList[pbIndex].pbMagQue.writeIdx = writeIdx;
    pthread_cond_signal(&instantPlaybackList[pbIndex].streamSignal);
    MUTEX_UNLOCK(instantPlaybackList[pbIndex].streamSignalLock);
    DPRINT(PLAYBACK_MEDIA, "write instant pb msg: [session=%d], [msg=%s], [fd=%d]", pbIndex, playbackSeqStr[pbMsgPtr->pbState], pbMsgPtr->cmdRespFd);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function send stream to stream socket
 * @param   camIndex
 * @param   pPlayParams
 * @param   pPlayInfo
 * @param   length
 * @param   mediaStatus
 * @return
 */
static BOOL readyFrameToSend(UINT8 camIndex, INSTANT_PLAY_PARAMS_t *pPlayParams,
                             INSTANT_PLAYBACK_INFO_t *pPlayInfo, UINT32PTR length, MEDIA_STATUS_e * mediaStatus)
{
	PLAYBACK_FILE_READ_e	playbackStatus = MAX_PLAYBACK_FILE_STATE;
	FSH_INFO_t 				fshInfo = { 0 };
	FRAME_HEADER_t 			*frameHeader;

    *length = FRAME_HEADER_LEN_MAX;
    ReadRecordFrame(pPlayInfo->camListInfo.dmPlaySessId, pPlayParams->direction, pPlayParams->frameType,
                    &fshInfo, &pPlayInfo->frameBuffer, length, &playbackStatus, &pPlayInfo->camListInfo.nextFrameTime);

    pPlayInfo->camListInfo.curPlaybackStatus = playbackStatus;
    if (playbackStatus == PLAYBACK_FILE_READ_NORMAL)
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

        if ((fshInfo.mediaType == STREAM_TYPE_VIDEO) && (frameHeader->mediaFrmLen == FRAME_HEADER_LEN_MAX))
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

        if (pPlayInfo->instantPlayParams.direction == PLAY_FORWARD)
        {
            if (fshInfo.localTime.totalSec > (UINT32)pPlayInfo->endTimeInSec)
            {
                playbackStatus = PLAYBACK_FILE_READ_OVER;
            }
            else
            {
                *mediaStatus = MEDIA_NORMAL;
                return TRUE;
            }
        }
        else
        {
            if (fshInfo.localTime.totalSec < (UINT32)pPlayInfo->startTimeInSec)
            {
                playbackStatus = PLAYBACK_FILE_READ_OVER;
            }
            else
            {
                *mediaStatus = MEDIA_NORMAL;
                return TRUE;
            }
        }
    }


    if (playbackStatus == PLAYBACK_FILE_READ_ERROR)
    {
        DPRINT(PLAYBACK_MEDIA, "file io error: [camera=%d]", camIndex);
        ClosePlaySession(pPlayInfo->camListInfo.dmPlaySessId);
        pPlayInfo->camListInfo.dmPlaySessId = INVALID_PLAY_SESSION_HANDLE;
        *mediaStatus = MEDIA_FILE_IO_ERROR;
    }
    else if (playbackStatus == PLAYBACK_FILE_READ_HDD_STOP)
    {
        ClosePlaySession(pPlayInfo->camListInfo.dmPlaySessId);
        pPlayInfo->camListInfo.dmPlaySessId = INVALID_PLAY_SESSION_HANDLE;
        pPlayInfo->camListInfo.nextFrameTime = (pPlayParams->direction == PLAY_FORWARD) ? UINT64_MAX : 0;
        *mediaStatus = MEDIA_HDD_FORMAT;
    }
    else if (playbackStatus == PLAYBACK_FILE_READ_OVER)
    {
        *mediaStatus = MEDIA_PLAYBACK_OVER;
        DPRINT(PLAYBACK_MEDIA, "file read over: [camera=%d]", camIndex);
    }
    else
    {
        *mediaStatus = MEDIA_OTHER_ERROR;
    }

    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function sets all playback position to given time for given direction
 * @param   playParams
 * @param   playInfo
 * @param   stepTime
 * @return
 */
static BOOL setInstantPlayPosition(INSTANT_PLAY_PARAMS_t *playParams, INSTANT_PLAYBACK_INFO_t *playInfo, struct tm *stepTime)
{
    BOOL        retVal = SUCCESS;
    BOOL        rollOverCnt = FALSE;
    BOOL        setPosition = FALSE, openPlaySession = FALSE;
    UINT8       camCnt = 0;
    time_t      localTime;
    struct tm   setPositionTime = *stepTime;

	ConvertLocalTimeInSec(stepTime, &localTime);
	camCnt = playParams->cameraIndex;

	do
	{
        DPRINT(PLAYBACK_MEDIA, "search: [camera=%d], [searchList=%d], [currentList=%d], [playSession=%d], [localTime=%ld], [startTime=%ld], [stopTime=%ld], [direction=%d], [audio=%d]",
               camCnt, playInfo->camListInfo.srchRsltListLen, playInfo->camListInfo.curRecFromList, playInfo->camListInfo.dmPlaySessId,
               localTime, playInfo->camListInfo.searchListInfo[playInfo->camListInfo.curRecFromList].startTime,
                playInfo->camListInfo.searchListInfo[playInfo->camListInfo.curRecFromList].stopTime, playParams->direction, playParams->audioChannel);

		// Now, we need to check new position is in between current record
        if ((playInfo->camListInfo.srchRsltListLen > 0)
                && (playInfo->camListInfo.searchListInfo[playInfo->camListInfo.curRecFromList].startTime <= localTime)
                && (playInfo->camListInfo.searchListInfo[playInfo->camListInfo.curRecFromList].stopTime >= localTime))
		{
			if(playInfo->camListInfo.dmPlaySessId == INVALID_PLAY_SESSION_HANDLE)
			{
                DPRINT(PLAYBACK_MEDIA, "open play dm session: [camera=%d]", camCnt);
				openPlaySession = TRUE;
			}
			else
			{
                playInfo->camListInfo.nextFrameTime = (UINT64)((UINT64)localTime * MSEC_IN_ONE_SEC);
				setPosition = TRUE;
			}
			break;
		}
		else
		{
			if(playInfo->camListInfo.dmPlaySessId != INVALID_PLAY_SESSION_HANDLE)
			{
				// New position is not in between current record so closed this play session
				ClosePlaySession(playInfo->camListInfo.dmPlaySessId);
                DPRINT(PLAYBACK_MEDIA, "close play dm session: [camera=%d], [dmSession=%d]", camCnt, playInfo->camListInfo.dmPlaySessId);
				playInfo->camListInfo.dmPlaySessId = INVALID_PLAY_SESSION_HANDLE;
			}
		}

        playInfo->camListInfo.curRecFromList++;
        if((playInfo->camListInfo.curRecFromList >= playInfo->camListInfo.srchRsltListLen) && (rollOverCnt == FALSE))
		{
			playInfo->camListInfo.curRecFromList = 0;
			rollOverCnt = TRUE;
		}
	}
	while(playInfo->camListInfo.curRecFromList < playInfo->camListInfo.srchRsltListLen);

    if((openPlaySession == FALSE) && (setPosition == FALSE))
	{
        EPRINT(PLAYBACK_MEDIA, "no session open for given time or set position: [camera=%d], [dmSession=%d]", camCnt, playInfo->camListInfo.dmPlaySessId);
		playInfo->camListInfo.curRecFromList = 0;

		do
		{
			// Now, we need to check new position is in between current record
            if((playInfo->camListInfo.srchRsltListLen > 0)
                && (playInfo->camListInfo.searchListInfo[playInfo->camListInfo.curRecFromList].startTime >= localTime))
			{
				if(playInfo->camListInfo.dmPlaySessId == INVALID_PLAY_SESSION_HANDLE)
				{
                    DPRINT(PLAYBACK_MEDIA, "open play dm session: [camera=%d]", camCnt);
					openPlaySession = TRUE;
				}
				else
				{
                    playInfo->camListInfo.nextFrameTime = (UINT64)((UINT64)localTime * MSEC_IN_ONE_SEC);
					setPosition = TRUE;
				}
				break;
			}
			else
			{
				if(playInfo->camListInfo.dmPlaySessId != INVALID_PLAY_SESSION_HANDLE)
				{
					// New position is not in between current record so closed this play session
					ClosePlaySession(playInfo->camListInfo.dmPlaySessId);
                    DPRINT(PLAYBACK_MEDIA, "close play dm session: [camera=%d], [dmSession=%d]", camCnt, playInfo->camListInfo.dmPlaySessId);
					playInfo->camListInfo.dmPlaySessId = INVALID_PLAY_SESSION_HANDLE;
				}
			}

            playInfo->camListInfo.curRecFromList++;
		}
		while(playInfo->camListInfo.curRecFromList < playInfo->camListInfo.srchRsltListLen);
	}

	if(openPlaySession == TRUE)
	{
		openPlaySession = FALSE;
        if(openPlaySessionAndSetPosition(&playInfo->camListInfo. searchListInfo[playInfo->camListInfo.curRecFromList],
                                         FALSE, playParams->audioChannel, playParams->speed, playParams->direction,
                                         &playInfo->camListInfo.nextFrameTime, &playInfo->camListInfo.dmPlaySessId) == CMD_SUCCESS)
		{
			// Check this record is in between given set time position
            setPosition = TRUE;
            if ((playInfo->camListInfo.searchListInfo[playInfo->camListInfo.curRecFromList].startTime <= localTime) &&
                    (playInfo->camListInfo.searchListInfo[playInfo->camListInfo.curRecFromList].stopTime >= localTime))
			{
                setPositionTime = *stepTime;
			}
			else
			{
				if(playParams->direction == PLAY_FORWARD)
				{
                    ConvertLocalTimeInBrokenTm(&playInfo->camListInfo.searchListInfo[playInfo->camListInfo.curRecFromList].startTime, &setPositionTime);
				}
				else if(playParams->direction == PLAY_REVERSE)
				{
                    ConvertLocalTimeInBrokenTm(&playInfo->camListInfo.searchListInfo[playInfo->camListInfo.curRecFromList].stopTime, &setPositionTime);
				}
			}
		}
		else
		{
			retVal = FAIL;
            EPRINT(PLAYBACK_MEDIA, "fail to open play session: [camera=%d], [NextFrameTime=%lld]", camCnt, playInfo->camListInfo.nextFrameTime);
		}
	}

	if(setPosition == TRUE)
	{
		setPosition = FALSE;

        DPRINT(PLAYBACK_MEDIA, "set play position: [camera=%d], [time=%02d:%02d], [audio=%d], [speed=%d]",
               camCnt, setPositionTime.tm_hour, setPositionTime.tm_min, playParams->audioChannel, playParams->speed);

        if (SetPlayPosition(&setPositionTime, 0, playParams->audioChannel, playParams->direction,
                            playParams->speed, playInfo->camListInfo.dmPlaySessId) == FAIL)
		{
            EPRINT(PLAYBACK_MEDIA, "fail to set play position: [camera=%d]", camCnt);
			retVal = FAIL;
		}
		else
		{
			playInfo->camListInfo.curPlaybackStatus = PLAYBACK_FILE_READ_NORMAL;
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
 * @param   playId
 * @return
 */
static NET_CMD_STATUS_e openPlaySessionAndSetPosition(SEARCH_DATA_t *searchData, BOOL setPlayPosition, BOOL audioEnbl,
                                                      UINT8 speed, PLAYBACK_CMD_e direction, UINT64PTR nextFrameTime, PLAY_SESSION_ID * playId)
{
	NET_CMD_STATUS_e	retVal = CMD_SUCCESS;
	PLAY_CNTRL_INFO_t	playCntrlInfo = { .startTime = { 0 }, .stopTime = { 0 }};
	struct tm			*timeStamp;

	playCntrlInfo.camNo = searchData->camNo;
	playCntrlInfo.diskId = searchData->diskId;

	playCntrlInfo.eventType = searchData->eventType;
	playCntrlInfo.overlapFlg = searchData->overlapFlg;
	playCntrlInfo.recStorageType = MAX_RECORDING_MODE;

	ConvertLocalTimeInBrokenTm(&searchData->startTime, &playCntrlInfo.startTime);
	ConvertLocalTimeInBrokenTm(&searchData->stopTime, &playCntrlInfo.stopTime);

    DPRINT(PLAYBACK_MEDIA, "instant play session: [camera=%d], [diskId=%d], [eventType=%d], [overlapFlg=%d], "
           "[startTime=%02d/%02d/%04d %02d:%02d:%02d], [stopTime=%02d/%02d/%04d %02d:%02d:%02d]",
           playCntrlInfo.camNo, playCntrlInfo.diskId, playCntrlInfo.eventType, playCntrlInfo.overlapFlg,
           playCntrlInfo.startTime.tm_mday, playCntrlInfo.startTime.tm_mon,
           playCntrlInfo.startTime.tm_year, playCntrlInfo.startTime.tm_hour,
           playCntrlInfo.startTime.tm_min, playCntrlInfo.startTime.tm_sec,
           playCntrlInfo.stopTime.tm_mday, playCntrlInfo.stopTime.tm_mon,
           playCntrlInfo.stopTime.tm_year, playCntrlInfo.stopTime.tm_hour,
           playCntrlInfo.stopTime.tm_min, playCntrlInfo.stopTime.tm_sec);

	retVal = OpenPlaySession(&playCntrlInfo, PLAYBACK_PLAY, playId);
    if(retVal != CMD_SUCCESS)
	{
        EPRINT(PLAYBACK_MEDIA, "fail to open play session: [camera=%d]", playCntrlInfo.camNo);
        if(retVal != CMD_MAX_STREAM_LIMIT)
        {
            *nextFrameTime = (direction == PLAY_FORWARD) ? UINT64_MAX : 0;
        }

        return retVal;
    }

    DPRINT(PLAYBACK_MEDIA, "instant play dm session opened: [camera=%d], [dmSession=%dd]", playCntrlInfo.camNo, *playId);
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

    if(setPlayPosition == FALSE)
    {
        return CMD_SUCCESS;
    }

    if (SetPlayPosition(timeStamp, 0, audioEnbl, direction, speed, *playId) == FAIL)
    {
        EPRINT(PLAYBACK_MEDIA, "fail to set play position: [camera=%d]", playCntrlInfo.camNo);
        *nextFrameTime = (direction == PLAY_FORWARD) ? UINT64_MAX : 0;
        return CMD_PROCESS_ERROR;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function open disk manager session for next consecutive record for particular camera.
 * @param   camIndex
 * @param   pPlayParams
 * @param   pPlayInfo
 * @return
 */
static NET_CMD_STATUS_e openNextRecord(UINT8 camIndex, INSTANT_PLAY_PARAMS_t *pPlayParams, INSTANT_PLAYBACK_INFO_t *pPlayInfo)
{
	BOOL					openPlaySession = FALSE, setPlayPos = FALSE;
	UINT16					searchNo;
	NET_CMD_STATUS_e		cmdStatus = CMD_SUCCESS;
	struct tm				brokenTime;

    ClosePlaySession(pPlayInfo->camListInfo.dmPlaySessId);
    DPRINT(PLAYBACK_MEDIA, "close play dm session: [camera=%d], [dmSession=%d]", camIndex, pPlayInfo->camListInfo.dmPlaySessId);
    pPlayInfo->camListInfo.dmPlaySessId = INVALID_PLAY_SESSION_HANDLE;

    if(pPlayParams->direction == PLAY_FORWARD)
	{
        pPlayInfo->camListInfo.curRecFromList++;
        DPRINT(PLAYBACK_MEDIA, "play forward records: [camera=%d], [current=%d], [total=%d]",
               camIndex, pPlayInfo->camListInfo.curRecFromList, pPlayInfo->camListInfo.srchRsltListLen);

        if(pPlayInfo->camListInfo.curRecFromList < pPlayInfo->camListInfo.srchRsltListLen)
		{
			openPlaySession = TRUE;
		}
		else
		{
            pPlayInfo->camListInfo.curRecFromList--;
		}
	}
	else
	{
        if(pPlayInfo->camListInfo.curRecFromList > 0)
		{
            pPlayInfo->camListInfo.curRecFromList--;
			openPlaySession = TRUE;
            DPRINT(PLAYBACK_MEDIA, "play reverse records: [camera=%d], [current=%d], [total=%d]",
                   camIndex, pPlayInfo->camListInfo.curRecFromList, pPlayInfo->camListInfo.srchRsltListLen);
		}
	}

    if(openPlaySession == FALSE)
	{
        pPlayInfo->camListInfo.curPlaybackStatus = PLAYBACK_FILE_READ_OVER;
        pPlayInfo->camListInfo.playbackStatus = PLAYBACK_FILE_READ_OVER;
        return CMD_SUCCESS;
    }

    searchNo = pPlayInfo->camListInfo.curRecFromList;
    cmdStatus = openPlaySessionAndSetPosition(&pPlayInfo->camListInfo.searchListInfo[searchNo], TRUE, pPlayParams->audioChannel,
                                              pPlayParams->speed, pPlayParams->direction, &pPlayInfo->camListInfo.nextFrameTime,
                                              &pPlayInfo->camListInfo.dmPlaySessId);
    if(cmdStatus != CMD_SUCCESS)
    {
        if(cmdStatus != CMD_MAX_STREAM_LIMIT)
        {
            pPlayInfo->camListInfo.nextFrameTime = (pPlayParams->direction == PLAY_FORWARD) ? UINT64_MAX : 0;
            pPlayInfo->camListInfo.curPlaybackStatus = PLAYBACK_FILE_READ_OVER;
            pPlayInfo->camListInfo.playbackStatus = PLAYBACK_FILE_READ_OVER;
        }
        return cmdStatus;
    }

    DPRINT(PLAYBACK_MEDIA, "record search: [camera=%d], [searchNo=%d], [total=%d], [direction=%d]",
           camIndex, searchNo, pPlayInfo->camListInfo.srchRsltListLen, pPlayParams->direction);
    if((searchNo != 0) && (pPlayParams->direction == PLAY_FORWARD))
    {
        if(pPlayInfo->camListInfo.searchListInfo[searchNo].startTime < pPlayInfo->camListInfo.searchListInfo[searchNo - 1].stopTime)
        {
            setPlayPos = TRUE;
            if(pPlayInfo->camListInfo.searchListInfo[searchNo - 1].stopTime > pPlayInfo->camListInfo.searchListInfo[searchNo].stopTime)
            {
                ConvertLocalTimeInBrokenTm(&pPlayInfo->camListInfo.searchListInfo[searchNo].startTime, &brokenTime);
            }
            else
            {
                ConvertLocalTimeInBrokenTm(&pPlayInfo->camListInfo.searchListInfo[searchNo - 1].stopTime, &brokenTime);
            }
        }
    }
    else if((searchNo != pPlayInfo->camListInfo.srchRsltListLen) && (pPlayParams->direction == PLAY_REVERSE))
    {
        if(((pPlayInfo->camListInfo.srchRsltListLen - searchNo) > 1) &&
                (pPlayInfo->camListInfo.searchListInfo[searchNo].stopTime > pPlayInfo->camListInfo.searchListInfo[searchNo + 1].startTime))
        {
            setPlayPos = TRUE;
            ConvertLocalTimeInBrokenTm(&pPlayInfo->camListInfo.searchListInfo[searchNo + 1].startTime, &brokenTime);
        }
        else
        {
            ConvertLocalTimeInBrokenTm(&pPlayInfo->camListInfo.searchListInfo[searchNo].stopTime, &brokenTime);
        }
    }

    if(setPlayPos == TRUE)
    {
        if (SetPlayPosition(&brokenTime, 0, pPlayParams->audioChannel, pPlayParams->direction, pPlayParams->speed, pPlayInfo->camListInfo.dmPlaySessId) == FAIL)
        {
            EPRINT(PLAYBACK_MEDIA, "fail to set play position: [camera=%d]", camIndex);
            cmdStatus = CMD_PROCESS_ERROR;
            pPlayInfo->camListInfo.nextFrameTime = (pPlayParams->direction == PLAY_FORWARD) ? UINT64_MAX : 0;
        }
    }

    pPlayInfo->camListInfo.curPlaybackStatus = PLAYBACK_FILE_READ_NORMAL;
    pPlayInfo->camListInfo.playbackStatus = PLAYBACK_FILE_READ_NORMAL;
	return cmdStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function generates frame header for given type of error.
 * @param   clientCbType
 * @param   camIndex
 * @param   sockFd
 * @param   mediaStatus
 * @param   frameSyncNum
 */
static void sendPlaybackFrameHeader(CLIENT_CB_TYPE_e clientCbType, UINT8 camIndex, INT32 sockFd, MEDIA_STATUS_e mediaStatus, UINT8 frameSyncNum)
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
    frameHeader.localTime.totalSec  = 0;
    frameHeader.localTime.mSec		= 0;
    frameHeader.preReserverMediaLen = 0;

    if (SUCCESS == sendDataCb[clientCbType](sockFd, (UINT8PTR)&frameHeader, FRAME_HEADER_LEN_MAX, SEND_PACKET_TIMEOUT))
    {
        DPRINT(PLAYBACK_MEDIA, "instant playback frame header sent: [camera=%d], [mediaStatus=%d]", camIndex, mediaStatus);
    }
    else
    {
        EPRINT(PLAYBACK_MEDIA, "failed to send instant playback frame header: [camera=%d], [mediaStatus=%d]", camIndex, mediaStatus);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   sortSearchData
 * @param   searchData
 * @param   searchRslt
 * @param   srchDataLen
 * @param   maxSortLen
 * @param   resltSort
 * @return
 */
static BOOL	sortSearchData(SEARCH_DATA_t * searchData, SEARCH_DATA_t *searchRslt, UINT16 srchDataLen, UINT16 maxSortLen, UINT16PTR resltSort)
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

//#################################################################################################
// @END OF FILE
//#################################################################################################
