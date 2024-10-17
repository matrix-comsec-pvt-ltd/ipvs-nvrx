//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		LiveMediaStreamer.c
@brief      This Module provide live stream data to client when client request for live stream of
            particular camera.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <sys/resource.h>

/* Application Includes */
#include "SysTimer.h"
#include "CameraInterface.h"
#include "LiveMediaStreamer.h"
#include "DebugLog.h"
#include "MediaStreamer.h"
#include "Utils.h"
#include "UtilCommon.h"
#include "NetworkCommand.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
/* Live media stream timeout in seconds */
#define	LIVE_STREAM_TIMEOUT				10

/* Live media stream queue read interval during frame processing in seconds */
#define	LIVE_STREAM_QUEUE_READ_TIME		1

/* Live media streamer thread stack size */
#define LIVE_STREAM_THREAD_STACK_SIZE   ((2 * MEGA_BYTE) + MAX_LIVE_STRM_BUFFER_SIZE)

/* Total live stream supported */
#define LIVE_STREAM_MAX					(getMaxCameraForCurrentVariant() * 4)

#define MAX_LS_QUEUE_SIZE				(200)

#define MAX_LS_FRAME_SEND_TIME			5 // Seconds

/* If difference between  last frame sys tick and the configured video loss duration sys tick is less than 2 sys tick(200 ms)
 * then declare video loss, This need to be done because if live streamer thread do not have any frame to process then it goes to
 * sleep for 10 sec due to which video loss configured in the multiple of 10 are sometimes getting delayed with 10 sec this case
 * happens only when live view of only one camera is on */
#define EARLY_VIDEO_LOSS_DECLARE_SYS_TICK           2

// Check for Queue roll-over, If occurs, Because it's a circular Queue assign 0th index
#define UPDATE_RW_INDEX(qPtr, rwIndex)				\
qPtr.rwIndex++;										\
if (qPtr.rwIndex >= MAX_LS_QUEUE_SIZE)				\
{													\
    qPtr.rwIndex = 0;								\
}													\

#define QUEUE_CLEANUP(qPtr)                             \
UINT8 idxCnt;                                           \
for (idxCnt = 0; idxCnt < MAX_LS_QUEUE_SIZE; idxCnt++)  \
{                                                       \
    FREE_MEMORY(qPtr.entryPtr[idxCnt]);                 \
}                                                       \
qPtr.readIndex = 0;                                     \
qPtr.writeIndex = 0;

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
    LS_TRG_START_STREAM,
    LS_TRG_FIRST_FRAME,
    LS_TRG_STOP_STREAM,
    LS_TRG_CHANGE_STREAM,
    LS_TRG_AUDIO_STATE,
    LS_TRG_FORCE_STOP,
    MAX_LS_TRG
}LS_TRG_e;

typedef enum
{
    LS_STREAM_ALL,
    LS_STREAM_I_FRAME,
    LS_STREAM_ADAPTIVE,
}LS_STREAM_TYPE_e;

typedef enum
{
    LS_STREAM_MJPG_ALL,
    LS_STREAM_MJPG_PERTICULAR,
    LS_STREAM_MJPG_ADAPTIVE,

}LS_STREAM_MPJEG_TYPE_e;

typedef struct
{
    LS_TRG_e				type;
    UINT8					camIndex;
    VIDEO_TYPE_e 			streamType;
    BOOL					audioState;
    INT32 					connId;
    NW_CMD_REPLY_CB			callBackFunc;
    NET_CMD_STATUS_e		nwCmdStatus;
    LS_STREAM_TYPE_e		frameType;
    LS_STREAM_MPJEG_TYPE_e	frameTypeMJPG;
    UINT8					fpsMJPG;
}LS_TRG_PARAM_t;

typedef struct
{
    UINT16					readIndex;
    UINT16					writeIndex;
    VOIDPTR					entryPtr[MAX_LS_QUEUE_SIZE];
}LS_QUEUE_t;

typedef struct
{
    BOOL					threadRunStatus;
    UINT8					clientIdx;
    BOOL					frameAvailable[MAX_CAMERA][MAX_STREAM];
    LS_QUEUE_t				lsQueue;
    pthread_mutex_t 		dataMutex;
    pthread_cond_t 			condSignal;
    CLIENT_CB_TYPE_e        clientCbType;

}LS_CLIENT_PUBLIC_t;

typedef struct
{
    INT32 					connId[MAX_CAMERA][MAX_STREAM];
    NW_CMD_REPLY_CB			callBackFunc[MAX_CAMERA][MAX_STREAM];
    BOOL 					includeAudio[MAX_CAMERA][MAX_STREAM];
    BOOL					streamSwitch[MAX_CAMERA][MAX_STREAM];
    BOOL					firstIframeSent[MAX_CAMERA][MAX_STREAM];
    UINT8					totalCamera;
    FRAME_HEADER_t 			frmHeader[MAX_CAMERA][MAX_STREAM];
    STREAM_STATUS_INFO_t	*streamStatusInfo;
    UINT8PTR 				streamBuffPtr;
    UINT32 					streamDataLen;
    UINT32					lastFrameTime[MAX_CAMERA][MAX_STREAM];
    LS_STREAM_TYPE_e		reqframeType[MAX_CAMERA][MAX_STREAM];
    LS_STREAM_MPJEG_TYPE_e	reqframeTypeMJPG[MAX_CAMERA][MAX_STREAM];
    UINT8					reqfpsMJPG[MAX_CAMERA][MAX_STREAM];
    UINT8					sendFrameCountMPJPG[MAX_CAMERA][MAX_STREAM];
    BOOL                    localFramePending[MAX_CAMERA][MAX_STREAM];
    BOOL                    firstCallBackGiven[MAX_CAMERA][MAX_STREAM];

}LS_CLIENT_PRIVATE_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static UINT32				liveStreamCnt;
static pthread_mutex_t		liveStreamCntLock;
static LS_CLIENT_PUBLIC_t	lsClientPublic[MAX_NW_CLIENT];
static BOOL 				camMotionState[MAX_CAMERA];
static CHARPTR              streamTypeStr[MAX_STREAM] = {"MAIN", "SUB"};

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static VOIDPTR liveStreamThread(VOIDPTR);
//-------------------------------------------------------------------------------------------------
static void liveStreamCallback(const CI_STREAM_RESP_PARAM_t *respParam);
//-------------------------------------------------------------------------------------------------
static BOOL isFrameAvailableForAnyCamera(BOOL frameStatusBuff[MAX_CAMERA][MAX_STREAM]);
//-------------------------------------------------------------------------------------------------
static BOOL addToLsQueue(UINT8 clientIdx, LS_TRG_PARAM_t *lsTrg);
//-------------------------------------------------------------------------------------------------
static BOOL setLiveStreamCnt(BOOL status);
//-------------------------------------------------------------------------------------------------
static BOOL isFrameNeedTosend(UINT8 cameraIndex, UINT8 streamType, LS_CLIENT_PRIVATE_t *lsClientData);
//-------------------------------------------------------------------------------------------------
static void cleanupLiveMediaStream(UINT8 camIndex, VIDEO_TYPE_e streamType, LS_CLIENT_PRIVATE_t *lsPrivate, LS_CLIENT_PUBLIC_t *lsPublic);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This api use intialize Live Media Streamer module.Create all thread at intialize.
 */
void InitLiveMediaStream(void)
{
    UINT8 clientIdx;

    liveStreamCnt = 0;
    MUTEX_INIT(liveStreamCntLock, NULL);

    /* Initially make all session invalid */
    for(clientIdx = 0; clientIdx < MAX_NW_CLIENT; clientIdx++)
    {
        MUTEX_INIT(lsClientPublic[clientIdx].dataMutex, NULL);
        pthread_cond_init(&lsClientPublic[clientIdx].condSignal, NULL);
        QUEUE_CLEANUP(lsClientPublic[clientIdx].lsQueue);
        lsClientPublic[clientIdx].threadRunStatus = INACTIVE;
        lsClientPublic[clientIdx].clientIdx = clientIdx;
        lsClientPublic[clientIdx].clientCbType = CLIENT_CB_TYPE_MAX;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This api use for start live stream from further module.It will create thread for receive stream.
 * @param   camIndex
 * @param   streamType
 * @param   clientIdx
 * @param   connId
 * @param   clientCbType
 * @param   reqFrameType
 * @param   reqFrameTypeForMPJEG
 * @param   reqfps
 * @return
 */
NET_CMD_STATUS_e AddLiveMediaStream(UINT8 camIndex, VIDEO_TYPE_e streamType, UINT8 clientIdx, INT32 connId,
                                    CLIENT_CB_TYPE_e clientCbType, UINT8 reqFrameType, UINT8 reqFrameTypeForMPJEG, UINT8 reqfps)
{
    LS_TRG_PARAM_t		*lsTrg;
    LS_CLIENT_PUBLIC_t	*lsClient;

    if (camIndex >= getMaxCameraForCurrentVariant())
    {
        EPRINT(LIVE_MEDIA_STREAMER, "max camera configured: [camera=%d], [sessionIdx=%d]", camIndex, clientIdx);
        return CMD_MAX_CAMERA_CONFIGURED;
    }

    if (clientIdx >= MAX_NW_CLIENT)
    {
        EPRINT(LIVE_MEDIA_STREAMER, "invld live client: [camera=%d], [sessionIdx=%d]", camIndex, clientIdx);
        return CMD_PROCESS_ERROR;
    }

    lsTrg = malloc(sizeof(LS_TRG_PARAM_t));
    if (lsTrg == NULL)
    {
        EPRINT(LIVE_MEDIA_STREAMER, "fail to alloc memory: [camera=%d], [sessionIdx=%d]", camIndex, clientIdx);
        return CMD_PROCESS_ERROR;
    }

    lsTrg->type = LS_TRG_START_STREAM;
    lsTrg->callBackFunc = clientCmdRespCb[clientCbType];
    lsTrg->camIndex = camIndex;
    lsTrg->connId = connId;
    lsTrg->streamType = streamType;
    lsTrg->frameType = (LS_STREAM_TYPE_e)reqFrameType;
    lsTrg->frameTypeMJPG = (LS_STREAM_MPJEG_TYPE_e)reqFrameTypeForMPJEG;
    lsTrg->fpsMJPG = reqfps;
    lsClient = &lsClientPublic[clientIdx];

    MUTEX_LOCK(lsClient->dataMutex);
    if (lsClient->lsQueue.entryPtr[lsClient->lsQueue.writeIndex] != NULL)
    {
        MUTEX_UNLOCK(lsClient->dataMutex);
        EPRINT(LIVE_MEDIA_STREAMER, "fail to start stream: [camera=%d], [sessionIdx=%d], [writeIndex=%d], [readIndex=%d]",
               camIndex, clientIdx, lsClient->lsQueue.writeIndex, lsClient->lsQueue.readIndex);
        free(lsTrg);
        return CMD_PROCESS_ERROR;
    }

    // Add new live stream object to LS queues
    lsClient->lsQueue.entryPtr[lsClient->lsQueue.writeIndex] = lsTrg;
    UPDATE_RW_INDEX(lsClient->lsQueue, writeIndex);

    /* If liveStreamThread in not running create new thread else return with success */
    DPRINT(LIVE_MEDIA_STREAMER, "start live stream: [camera=%d], [sessionIdx=%d], [stream=%s]", camIndex, clientIdx, streamTypeStr[streamType]);
    if (lsClient->threadRunStatus == ACTIVE)
    {
        /* Signal required when no frames are available from any cameras which are added in live view
         * otherwise this command executed after conditional wait timeout */
        pthread_cond_signal(&lsClient->condSignal);
        MUTEX_UNLOCK(lsClient->dataMutex);
        return CMD_SUCCESS;
    }
    lsClient->threadRunStatus = ACTIVE;
    memset(lsClient->frameAvailable, FALSE, sizeof(lsClient->frameAvailable));

    /* assigning callback type */
    lsClient->clientCbType = clientCbType;
    MUTEX_UNLOCK(lsClient->dataMutex);

    if (FALSE == Utils_CreateThread(NULL, liveStreamThread, &lsClientPublic[clientIdx], DETACHED_THREAD, LIVE_STREAM_THREAD_STACK_SIZE))
    {
        MUTEX_LOCK(lsClient->dataMutex);
        lsClient->threadRunStatus = INACTIVE;
        QUEUE_CLEANUP(lsClient->lsQueue);
        MUTEX_UNLOCK(lsClient->dataMutex);
        EPRINT(LIVE_MEDIA_STREAMER, "fail to create thread: [camera=%d], [sessionIdx=%d]", camIndex, clientIdx);
        free(lsTrg);
        return CMD_PROCESS_ERROR;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function stop  stream. it only set flag for stop stream. and wake up stream thread,
 *          stream thread function close thread if no more client want this stream.
 * @param   camIndex
 * @param   streamType
 * @param   clientIdx
 * @return
 */
BOOL RemoveLiveMediaStream(UINT8 camIndex, VIDEO_TYPE_e streamType, UINT8 clientIdx)
{
    BOOL            status;
    LS_TRG_PARAM_t  *lsTrg;

    if ((camIndex >= getMaxCameraForCurrentVariant()) || (clientIdx >= MAX_NW_CLIENT))
    {
        EPRINT(LIVE_MEDIA_STREAMER, "max camera configured or invld live client: [camera=%d], [sessionIdx=%d]", camIndex, clientIdx);
        return FAIL;
    }

    lsTrg = malloc(sizeof(LS_TRG_PARAM_t));
    if (lsTrg == NULL)
    {
        EPRINT(LIVE_MEDIA_STREAMER, "fail to alloc memory: [camera=%d], [sessionIdx=%d]", camIndex, clientIdx);
        return FAIL;
    }

    lsTrg->type = LS_TRG_STOP_STREAM;
    lsTrg->camIndex = camIndex;
    lsTrg->streamType = streamType;

    status = addToLsQueue(clientIdx, lsTrg);
    if (SUCCESS != status)
    {
        free(lsTrg);
        if (status == FAIL)
        {
            EPRINT(LIVE_MEDIA_STREAMER, "fail to stop stream: [camera=%d], [sessionIdx=%d], [stream=%s]", camIndex, clientIdx, streamTypeStr[streamType]);
        }
        return FAIL;
    }

    DPRINT(LIVE_MEDIA_STREAMER, "stop live stream: [camera=%d], [sessionIdx=%d], [stream=%s]", camIndex, clientIdx, streamTypeStr[streamType]);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function add audio to live media and add audio to client stream.
 * @param   camIndex
 * @param   streamType
 * @param   clientIdx
 * @param   state
 * @return
 */
BOOL ChangeLiveMediaAudioState(UINT8 camIndex, VIDEO_TYPE_e streamType, UINT8 clientIdx, BOOL state)
{
    LS_TRG_PARAM_t *lsTrg;
    BOOL            status;

    if ((camIndex >= getMaxCameraForCurrentVariant()) || (clientIdx >= MAX_NW_CLIENT))
    {
        EPRINT(LIVE_MEDIA_STREAMER, "max camera configured or invld live client: [camera=%d], [sessionIdx=%d]", camIndex, clientIdx);
        return FAIL;
    }

    lsTrg = malloc(sizeof(LS_TRG_PARAM_t));
    if (lsTrg == NULL)
    {
        EPRINT(LIVE_MEDIA_STREAMER, "fail to alloc memory: [camera=%d], [sessionIdx=%d]", camIndex, clientIdx);
        return FAIL;
    }

    lsTrg->type = LS_TRG_AUDIO_STATE;
    lsTrg->audioState = state;
    lsTrg->camIndex = camIndex;
    lsTrg->streamType = streamType;

    status = addToLsQueue(clientIdx, lsTrg);
    if (SUCCESS != status)
    {
        free(lsTrg);
        if (status == FAIL)
        {
            EPRINT(LIVE_MEDIA_STREAMER, "fail to change audio state for stream: [camera=%d], [sessionIdx=%d], [stream=%s]", camIndex, clientIdx, streamTypeStr[streamType]);
        }
        return FAIL;
    }

    DPRINT(LIVE_MEDIA_STREAMER, "change stream audio state: [camera=%d], [sessionIdx=%d], [stream=%s]", camIndex, clientIdx, streamTypeStr[streamType]);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function changes stream type from MAIN to SUB or SUB to MAIN
 * @param   camIndex
 * @param   streamType
 * @param   clientIdx
 * @param   connFd
 * @param   clientCbType
 * @param   reqFrameType
 * @param   reqFrameTypeForMPJEG
 * @param   reqfps
 * @return
 */
BOOL ChangeLiveMediaStream(UINT8 camIndex, VIDEO_TYPE_e streamType, UINT8 clientIdx, INT32 connFd,
                           CLIENT_CB_TYPE_e clientCbType, UINT8 reqFrameType, UINT8 reqFrameTypeForMPJEG, UINT8 reqfps)
{
    LS_TRG_PARAM_t *lsTrg;

    if ((camIndex >= getMaxCameraForCurrentVariant()) || (clientIdx >= MAX_NW_CLIENT) || (streamType >= MAX_STREAM))
    {
        EPRINT(LIVE_MEDIA_STREAMER, "invld input param: [camera=%d], [sessionIdx=%d], [stream=%d]", camIndex, clientIdx, streamType);
        return FAIL;
    }

    lsTrg = malloc(sizeof(LS_TRG_PARAM_t));
    if (lsTrg == NULL)
    {
        EPRINT(LIVE_MEDIA_STREAMER, "fail to alloc memory: [camera=%d], [sessionIdx=%d]", camIndex, clientIdx);
        return FAIL;
    }

    lsTrg->type = LS_TRG_CHANGE_STREAM;
    lsTrg->camIndex = camIndex;
    lsTrg->streamType = streamType;
    lsTrg->connId = connFd;
    lsTrg->callBackFunc = clientCmdRespCb[clientCbType];
    lsTrg->frameType = (LS_STREAM_TYPE_e)reqFrameType;
    lsTrg->frameTypeMJPG = (LS_STREAM_MPJEG_TYPE_e)reqFrameTypeForMPJEG;
    lsTrg->fpsMJPG = reqfps;

    if (SUCCESS != addToLsQueue(clientIdx, lsTrg))
    {
        free(lsTrg);
        EPRINT(LIVE_MEDIA_STREAMER, "fail to change stream type: [camera=%d], [sessionIdx=%d], [stream=%s]", camIndex, clientIdx, streamTypeStr[streamType]);
        return FAIL;
    }

    DPRINT(LIVE_MEDIA_STREAMER, "change live stream type: [camera=%d], [sessionIdx=%d], [stream=%s]", camIndex, clientIdx, streamTypeStr[streamType]);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is thread function ,pass as argument of thread create. It will schedule when processor
 *          get time.When live notify function send  signal to this thread function,it get next frame
 *          from camera interface.
 * @param   arg
 * @return
 */
static VOIDPTR liveStreamThread(VOIDPTR arg)
{
    UINT8 					camIndex, complexCamIdx, clientIdx, camCnt = 0;
    VIDEO_TYPE_e			streamType = MAX_STREAM, streamCnt = MAIN_STREAM, prevStreamType;
    LS_CLIENT_PUBLIC_t		*lsPublic = (LS_CLIENT_PUBLIC_t *)arg;
    LS_CLIENT_PRIVATE_t		lsPrivate;
    LS_TRG_PARAM_t			*lsTrg;
    struct timespec 		ts;
    BOOL					frameProcessStopF = FALSE;
    UINT32					totalFrameP;
    UINT32					framePushStartTime;
    GENERAL_CONFIG_t		generalConfig;
    UINT32                  frameLen;
    UINT8                   frameBuff[MAX_LIVE_STRM_BUFFER_SIZE];

    setpriority(PRIO_PROCESS, PRIO_PROCESS, 1);

    THREAD_START_INDEX("LIVE_MEDIA", lsPublic->clientIdx);

    for (camIndex = 0; camIndex < MAX_CAMERA; camIndex++)
    {
        for (streamType = MAIN_STREAM; streamType < MAX_STREAM; streamType++)
        {
            lsPrivate.connId[camIndex][streamType] = INVALID_CONNECTION;
            lsPrivate.callBackFunc[camIndex][streamType] = NULL;
            lsPrivate.frmHeader[camIndex][streamType].magicCode = MAGIC_CODE;
            lsPrivate.frmHeader[camIndex][streamType].headerVersion = HEADER_VERSION; //current version 2
            lsPrivate.frmHeader[camIndex][streamType].productType = PRODUCT_TYPE;
            lsPrivate.frmHeader[camIndex][streamType].chNo = GET_CAMERA_NO(camIndex);
            lsPrivate.frmHeader[camIndex][streamType].codecType = VIDEO_H264;
            lsPrivate.frmHeader[camIndex][streamType].vidResolution = RESOLUTION_640x480_VGA;
            lsPrivate.streamSwitch[camIndex][streamType] = FALSE;
            lsPrivate.firstIframeSent[camIndex][streamType] = FALSE;
            lsPrivate.reqframeType[camIndex][streamType] = LS_STREAM_ALL;
            lsPrivate.reqframeTypeMJPG[camIndex][streamType] = LS_STREAM_MJPG_ALL;
            lsPrivate.reqfpsMJPG[camIndex][streamType] = 0;
            lsPrivate.sendFrameCountMPJPG[camIndex][streamType] = 0;
            lsPrivate.firstCallBackGiven[camIndex][streamType] = FALSE;
            lsPrivate.localFramePending[camIndex][streamType] = FALSE;
        }
    }

    lsPrivate.totalCamera = 0;

    while (TRUE)
    {
        MUTEX_LOCK(lsPublic->dataMutex);

        // Part 1 - Execute Queue Tasks
        while (lsPublic->lsQueue.entryPtr[lsPublic->lsQueue.readIndex] != NULL)
        {
            lsTrg = lsPublic->lsQueue.entryPtr[lsPublic->lsQueue.readIndex];
            lsPublic->lsQueue.entryPtr[lsPublic->lsQueue.readIndex] = NULL;

            UPDATE_RW_INDEX(lsPublic->lsQueue, readIndex);

            MUTEX_UNLOCK(lsPublic->dataMutex);

            camIndex = lsTrg->camIndex;
            streamType = lsTrg->streamType;

            switch (lsTrg->type)
            {
                /* In AddLiveMediaStream() for new live stream request LS_TRG_START_STREAM is added in LS queue.
                 * When LIVE_MEDIA thread starts, first it will check LS queue and then do execution releated in start stream
                 * in LS_TRG_START_STREAM */
                case LS_TRG_START_STREAM:
                {
                    /* If stream already active then restart it.
                     * Ideally it should not happen because when client needs to restart stream then it will fisrt stop stream then
                     * it will send start stream command. In some rainy day scenarios, client may send start stream command for already
                     * active stream without stopping it. To handle this type of cases, we have restarted already active stream */
                    if (lsPrivate.connId[camIndex][streamType] != INVALID_CONNECTION)
                    {
                        WPRINT(LIVE_MEDIA_STREAMER, "stream already on, restarting: [camera=%d], [stream=%s] [sessionIdx=%d]",
                               camIndex, streamTypeStr[streamType], lsPublic->clientIdx);
                        cleanupLiveMediaStream(camIndex, streamType, &lsPrivate, lsPublic);
                    }

                    if (FAIL == setLiveStreamCnt(TRUE))
                    {
                        WPRINT(LIVE_MEDIA_STREAMER, "max stream limit reach: [camera=%d], [stream=%s] [sessionIdx=%d]",
                               camIndex, streamTypeStr[streamType], lsPublic->clientIdx);
                        lsTrg->callBackFunc(CMD_MAX_STREAM_LIMIT, lsTrg->connId, TRUE);
                        break;
                    }

                    complexCamIdx = GET_STREAM_MAPPED_CAMERA_ID(camIndex, lsTrg->streamType);
                    clientIdx = (CI_STREAM_CLIENT_LIVE_START + lsPublic->clientIdx);
                    InitStreamSession(complexCamIdx, clientIdx, CI_READ_LATEST_FRAME);

                    lsTrg->nwCmdStatus = StartStream(complexCamIdx, liveStreamCallback, clientIdx);
                    if (lsTrg->nwCmdStatus != CMD_SUCCESS)
                    {
                        WPRINT(LIVE_MEDIA_STREAMER, "fail to start stream: [camera=%d], [status=%d], [sessionIdx=%d]",
                               complexCamIdx, lsTrg->nwCmdStatus, lsPublic->clientIdx);
                        lsTrg->callBackFunc(lsTrg->nwCmdStatus, lsTrg->connId, TRUE);
                        setLiveStreamCnt(FALSE);
                        break;
                    }

                    lsPrivate.totalCamera++;
                    lsPrivate.connId[camIndex][streamType] = lsTrg->connId;
                    lsPrivate.callBackFunc[camIndex][streamType] = lsTrg->callBackFunc;
                    lsPrivate.includeAudio[camIndex][streamType] = DISABLE;
                    lsPrivate.lastFrameTime[camIndex][streamType] = GetSysTick();
                    lsPrivate.frmHeader[camIndex][streamType].vidLoss = FALSE;
                    lsPrivate.firstIframeSent[camIndex][streamType] = FALSE;
                    lsPrivate.reqframeType[camIndex][streamType] = lsTrg->frameType;
                    lsPrivate.reqframeTypeMJPG[camIndex][streamType] = lsTrg->frameTypeMJPG;
                    lsPrivate.reqfpsMJPG[camIndex][streamType] = lsTrg->fpsMJPG;
                    lsPrivate.sendFrameCountMPJPG[camIndex][streamType] = 0;
                }
                break;

                case LS_TRG_FIRST_FRAME:
                {
                    if (lsPrivate.callBackFunc[camIndex][streamType] == NULL)
                    {
                        DPRINT(LIVE_MEDIA_STREAMER, "live stream callback not register: [camera=%d], [sessionIdx=%d]", camIndex, lsPublic->clientIdx);
                        break;
                    }

                    if (lsPrivate.streamSwitch[camIndex][streamType] == TRUE)
                    {
                        lsPrivate.streamSwitch[camIndex][streamType] = FALSE;
                        break;
                    }

                    // reply command success to client and do not close connection
                    lsPrivate.callBackFunc[camIndex][streamType](lsTrg->nwCmdStatus, lsPrivate.connId[camIndex][streamType], FALSE);
                    lsPrivate.callBackFunc[camIndex][streamType] = NULL;
                    if (lsTrg->nwCmdStatus != CMD_SUCCESS)
                    {
                        // No need to Say STOP Stream to CI, but it won't harm
                        cleanupLiveMediaStream(camIndex, streamType, &lsPrivate, lsPublic);
                    }
                    else
                    {
                        DPRINT(LIVE_MEDIA_STREAMER, "live stream reply success: [camera=%d], [sessionIdx=%d]", camIndex, lsPublic->clientIdx);
                        lsPrivate.firstCallBackGiven[camIndex][streamType] = TRUE;
                    }
                }
                break;

                case LS_TRG_STOP_STREAM:
                {
                    if (lsPrivate.connId[camIndex][streamType] != INVALID_CONNECTION)
                    {
                        cleanupLiveMediaStream(camIndex, streamType, &lsPrivate, lsPublic);
                    }
                }
                break;

                case LS_TRG_CHANGE_STREAM:
                {
                    if (FALSE == lsPrivate.firstCallBackGiven[camIndex][!streamType])
                    {
                        lsTrg->nwCmdStatus = CMD_RESOURCE_LIMIT;
                        EPRINT(LIVE_MEDIA_STREAMER, "stream switch before previous response given: [camera=%d], [sessionIdx=%d]", camIndex, lsPublic->clientIdx);
                        lsTrg->callBackFunc(lsTrg->nwCmdStatus, lsTrg->connId, TRUE);
                        break;
                    }

                    if (lsPrivate.connId[camIndex][streamType] != INVALID_CONNECTION)
                    {
                        lsTrg->callBackFunc(CMD_STREAM_ALREADY_ON, lsTrg->connId, TRUE);
                        EPRINT(LIVE_MEDIA_STREAMER, "stream already on: [camera=%d], [stream=%s], [sessionIdx=%d]",
                               camIndex, streamTypeStr[streamType], lsPublic->clientIdx);
                        break;
                    }

                    complexCamIdx = GET_STREAM_MAPPED_CAMERA_ID(camIndex, streamType);
                    clientIdx = (CI_STREAM_CLIENT_LIVE_START + lsPublic->clientIdx);
                    InitStreamSession(complexCamIdx, clientIdx,CI_READ_LATEST_FRAME);
                    lsTrg->nwCmdStatus = StartStream(complexCamIdx, liveStreamCallback, clientIdx);
                    lsTrg->callBackFunc(lsTrg->nwCmdStatus, lsTrg->connId, TRUE);
                    if (lsTrg->nwCmdStatus != CMD_SUCCESS)
                    {
                        break;
                    }

                    prevStreamType = !streamType;
                    lsPrivate.connId[camIndex][streamType] = lsPrivate.connId[camIndex][prevStreamType];
                    lsPrivate.callBackFunc[camIndex][streamType] = lsPrivate.callBackFunc[camIndex][prevStreamType];
                    lsPrivate.includeAudio[camIndex][streamType] = lsPrivate.includeAudio[camIndex][prevStreamType];
                    lsPrivate.lastFrameTime[camIndex][streamType] = lsPrivate.lastFrameTime[camIndex][prevStreamType];

                    lsPrivate.streamSwitch[camIndex][streamType] = TRUE;
                    lsPrivate.firstIframeSent[camIndex][streamType] = FALSE;
                    lsPrivate.reqframeType[camIndex][streamType] = lsTrg->frameType;
                    lsPrivate.reqframeTypeMJPG[camIndex][streamType] = lsTrg->frameTypeMJPG;
                    lsPrivate.reqfpsMJPG[camIndex][streamType] = lsTrg->fpsMJPG;
                    lsPrivate.sendFrameCountMPJPG[camIndex][streamType] = 0;
                    lsPrivate.firstCallBackGiven[camIndex][streamType] = TRUE;
                    StopStream(GET_STREAM_MAPPED_CAMERA_ID(camIndex, prevStreamType), clientIdx);

                    lsPrivate.connId[camIndex][prevStreamType] = INVALID_CONNECTION;
                    lsPrivate.callBackFunc[camIndex][prevStreamType] = NULL;
                    lsPrivate.streamSwitch[camIndex][prevStreamType] = FALSE;
                    lsPrivate.includeAudio[camIndex][prevStreamType] = FALSE;
                    lsPrivate.firstIframeSent[camIndex][prevStreamType] = FALSE;
                    lsPrivate.sendFrameCountMPJPG[camIndex][prevStreamType] = 0;
                    lsPrivate.firstCallBackGiven[camIndex][prevStreamType] = FALSE;
                }
                break;

                case LS_TRG_AUDIO_STATE:
                {
                    lsPrivate.includeAudio[camIndex][streamType] = lsTrg->audioState;
                }
                break;

                case LS_TRG_FORCE_STOP:
                {
                    if (FALSE == lsPrivate.firstCallBackGiven[camIndex][streamType])
                    {
                        if (lsPrivate.callBackFunc[camIndex][streamType] != NULL)
                        {
                            /* NOTE: when start live stream command has been received and camera is in video loss
                             * and no frame would be received. This is handling for giving response to the client
                             * to show proper message that stream is not available. */
                            lsPrivate.callBackFunc[camIndex][streamType](CMD_CODEC_NOT_SUPPORTED, lsPrivate.connId[camIndex][streamType], FALSE);
                            lsPrivate.callBackFunc[camIndex][streamType] = NULL;
                        }
                    }

                    if (lsPrivate.connId[camIndex][streamType] != INVALID_CONNECTION)
                    {
                        lsPrivate.frmHeader[camIndex][streamType].mediaStatus = (UINT8)MEDIA_CFG_CHANGE;
                        lsPrivate.frmHeader[camIndex][streamType].vidLoss = FALSE;
                        GetLocalTime(&lsPrivate.frmHeader[camIndex][streamType].localTime);
                        lsPrivate.frmHeader[camIndex][streamType].mediaFrmLen = FRAME_HEADER_LEN_MAX;

                        /* send to socket */
                        sendDataCb[lsPublic->clientCbType](lsPrivate.connId[camIndex][streamType], (UINT8PTR)&lsPrivate.frmHeader[camIndex][streamType],
                                                           FRAME_HEADER_LEN_MAX, MAX_LS_FRAME_SEND_TIME);
                    }

                    cleanupLiveMediaStream(camIndex, streamType, &lsPrivate, lsPublic);
                }
                break;

                default:
                {
                    /* Nothing to do */
                }
                break;
            }

            free(lsTrg);
            MUTEX_LOCK(lsPublic->dataMutex);
        }

        /* Check camera is added or not for streaming */
        if (lsPrivate.totalCamera == 0)
        {
            /* There is no single camera for streaming. Hence live streaming thread is not required. Exit from thread */
            break;
        }

        /* Check frame is available or not for any camera */
        if (FALSE == isFrameAvailableForAnyCamera(lsPublic->frameAvailable))
        {
            /* No frame found for any camera. */
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += LIVE_STREAM_TIMEOUT;

            /* It will wakeup on timeout or on conditional signal */
            pthread_cond_timedwait(&lsPublic->condSignal, &lsPublic->dataMutex, &ts);

            if (lsPublic->lsQueue.entryPtr[lsPublic->lsQueue.readIndex] != NULL)
            {
                MUTEX_UNLOCK(lsPublic->dataMutex);
                continue;
            }
        }

        /* Get copy of frame status of all cameras */
        memcpy(lsPrivate.localFramePending, lsPublic->frameAvailable, sizeof(lsPrivate.localFramePending));
        MUTEX_UNLOCK(lsPublic->dataMutex);

        framePushStartTime = GetSysTick();
        ReadGeneralConfig(&generalConfig);

        /* Do processing of available frames for all cameras */
        do
        {
            for (camIndex = camCnt; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
            {
                for (streamType = streamCnt; streamType < MAX_STREAM; streamType++)
                {
                    /* Process only if connection is valid */
                    if (lsPrivate.connId[camIndex][streamType] == INVALID_CONNECTION)
                    {
                        /* Change status to frame is not available */
                        MUTEX_LOCK(lsPublic->dataMutex);
                        lsPublic->frameAvailable[camIndex][streamType] = FALSE;
                        MUTEX_UNLOCK(lsPublic->dataMutex);
                        continue;
                    }

                    /* When first frame response given to client and new frame is available from camera for client */
                    if ((TRUE == lsPrivate.firstCallBackGiven[camIndex][streamType]) && (TRUE == lsPrivate.localFramePending[camIndex][streamType]))
                    {
                        /* Get frame from stream buffer. For p2p client, start from next i-frame but other client, start from previous i-frame */
                        totalFrameP = GetNextFrameForLive(GET_STREAM_MAPPED_CAMERA_ID(camIndex, streamType), (CI_STREAM_CLIENT_LIVE_START + lsPublic->clientIdx),
                                                          &lsPrivate.streamStatusInfo, &lsPrivate.streamBuffPtr, &lsPrivate.streamDataLen,
                                                          (lsPublic->clientCbType == CLIENT_CB_TYPE_P2P) ? TRUE : lsPrivate.firstIframeSent[camIndex][streamType]);

                        /* We have 1 or more frame to send client */
                        if (totalFrameP >= 1)
                        {
                            if (((lsPrivate.streamStatusInfo->streamType == STREAM_TYPE_VIDEO)
                                 && ((lsPrivate.firstIframeSent[camIndex][streamType] == TRUE) || (lsPrivate.streamStatusInfo->streamPara.videoStreamType == I_FRAME)))
                                    || ((lsPrivate.streamStatusInfo->streamType == STREAM_TYPE_AUDIO) && (lsPrivate.includeAudio[camIndex][streamType] == TRUE)
                                        && (lsPrivate.firstIframeSent[camIndex][streamType] == TRUE)))
                            {
                                /* Check whether we have to send this frame to client or discard it */
                                if (isFrameNeedTosend(camIndex, streamType, &lsPrivate) == TRUE)
                                {
                                    // Decorate Frame Header !!!!
                                    lsPrivate.frmHeader[camIndex][streamType].streamType = lsPrivate.streamStatusInfo->streamType;

                                    // Check stream type is video
                                    if (lsPrivate.frmHeader[camIndex][streamType].streamType == STREAM_TYPE_VIDEO)
                                    {
                                        lsPrivate.frmHeader[camIndex][streamType].vidResolution = (UINT8)lsPrivate.streamStatusInfo->streamPara.resolution;
                                        lsPrivate.frmHeader[camIndex][streamType].frmType = (UINT8)lsPrivate.streamStatusInfo->streamPara.videoStreamType;
                                        lsPrivate.frmHeader[camIndex][streamType].fps = (UINT8)lsPrivate.streamStatusInfo->streamPara.sampleRate;
                                        lsPrivate.frmHeader[camIndex][streamType].noOfRefFrame = (UINT8)lsPrivate.streamStatusInfo->streamPara.noOfRefFrame;

                                        if (lsPrivate.streamStatusInfo->streamPara.videoStreamType == I_FRAME)
                                        {
                                            lsPrivate.firstIframeSent[camIndex][streamType] = TRUE;
                                        }
                                    }
                                    else
                                    {
                                        lsPrivate.frmHeader[camIndex][streamType].audSampleFrq = (UINT16)lsPrivate.streamStatusInfo->streamPara.sampleRate;
                                    }

                                    lsPrivate.frmHeader[camIndex][streamType].codecType = (UINT8)lsPrivate.streamStatusInfo->streamPara.streamCodecType;
                                    lsPrivate.frmHeader[camIndex][streamType].localTime = lsPrivate.streamStatusInfo->localTime;
                                    lsPrivate.frmHeader[camIndex][streamType].mediaStatus = (UINT8)MEDIA_NORMAL;
                                    lsPrivate.frmHeader[camIndex][streamType].mediaFrmLen = (lsPrivate.streamDataLen + FRAME_HEADER_LEN_MAX);

                                    if (lsPrivate.streamDataLen == 0)
                                    {
                                        WPRINT(LIVE_MEDIA_STREAMER, "camera gives video loss: [camera=%d], [sessionIdx=%d]", camIndex, lsPublic->clientIdx);
                                        lsPrivate.frmHeader[camIndex][streamType].vidLoss = TRUE;
                                        lsPrivate.firstIframeSent[camIndex][streamType] = FALSE;
                                    }
                                    else
                                    {
                                        lsPrivate.frmHeader[camIndex][streamType].vidLoss = FALSE;
                                    }

                                    /* Get total frame length including header */
                                    frameLen = FRAME_HEADER_LEN_MAX + lsPrivate.streamDataLen;
                                    if (frameLen > MAX_LIVE_STRM_BUFFER_SIZE)
                                    {
                                        /* We don't have memory to copy whole frame data in local buffer */
                                        EPRINT(LIVE_MEDIA_STREAMER, "frame length is greater, frame ignored: [camera=%d], [sessionIdx=%d], [length=%d]",
                                               camIndex, lsPublic->clientIdx, frameLen);
                                        continue;
                                    }

                                    /* Copy frame header in buffer */
                                    memcpy(frameBuff, &lsPrivate.frmHeader[camIndex][streamType], FRAME_HEADER_LEN_MAX);

                                    /* Copy frame if length is not 0 */
                                    if (lsPrivate.streamDataLen)
                                    {
                                        /* Copy frame data in buffer */
                                        memcpy(frameBuff+FRAME_HEADER_LEN_MAX, lsPrivate.streamBuffPtr, lsPrivate.streamDataLen);
                                    }

                                    /* Send received frame to client */
                                    if (sendDataCb[lsPublic->clientCbType](lsPrivate.connId[camIndex][streamType], frameBuff, frameLen, MAX_LS_FRAME_SEND_TIME) == SUCCESS)
                                    {
                                        lsPrivate.lastFrameTime[camIndex][streamType] = GetSysTick();
                                    }
                                    else
                                    {
                                        // Failed To Send Frame, Close The Connection
                                        WPRINT(LIVE_MEDIA_STREAMER, "fail to send live frame: [camera=%d], [sessionIdx=%d]", camIndex, lsPublic->clientIdx);
                                        cleanupLiveMediaStream(camIndex, streamType, &lsPrivate, lsPublic);
                                        if (lsPrivate.totalCamera == 0)
                                        {
                                            /* No camera is there for streaming. Hence frame processing not required */
                                            frameProcessStopF = TRUE;
                                            break;
                                        }
                                    }
                                }
                                else
                                {
                                    continue;
                                }
                            }
                            /* No frame available from camera. Hence send video loss if timeout occurred */
                            else if ((ElapsedTick(lsPrivate.lastFrameTime[camIndex][streamType]) + EARLY_VIDEO_LOSS_DECLARE_SYS_TICK)
                                     >= CONVERT_SEC_TO_TIMER_COUNT(generalConfig.preVideoLossDuration))
                            {
                                WPRINT(LIVE_MEDIA_STREAMER, "now declare video loss: [camera=%d], [sessionIdx=%d]", camIndex, lsPublic->clientIdx);

                                // Update time
                                lsPrivate.lastFrameTime[camIndex][streamType] = GetSysTick();
                                lsPrivate.sendFrameCountMPJPG[camIndex][streamType] = 0;

                                lsPrivate.frmHeader[camIndex][streamType].mediaStatus = (UINT8)MEDIA_NORMAL;
                                lsPrivate.frmHeader[camIndex][streamType].streamType = STREAM_TYPE_VIDEO;
                                lsPrivate.frmHeader[camIndex][streamType].frmType = I_FRAME;
                                lsPrivate.frmHeader[camIndex][streamType].vidLoss = TRUE;
                                GetLocalTime(&lsPrivate.frmHeader[camIndex][streamType].localTime);
                                lsPrivate.frmHeader[camIndex][streamType].mediaFrmLen = FRAME_HEADER_LEN_MAX;

                                if (sendDataCb[lsPublic->clientCbType](lsPrivate.connId[camIndex][streamType],
                                                                       (UINT8PTR)&lsPrivate.frmHeader[camIndex][streamType],
                                                                       FRAME_HEADER_LEN_MAX, MAX_LS_FRAME_SEND_TIME) == FAIL)
                                {
                                    WPRINT(LIVE_MEDIA_STREAMER, "fail to send video loss header: [camera=%d], [sessionIdx=%d]", camIndex, lsPublic->clientIdx);
                                    cleanupLiveMediaStream(camIndex, streamType, &lsPrivate, lsPublic);
                                    if (lsPrivate.totalCamera == 0)
                                    {
                                        /* No camera is there for streaming. Hence frame processing not required */
                                        frameProcessStopF = TRUE;
                                        break;
                                    }
                                }
                            }
                        }

                        /* There was only one frame or no frame was found */
                        if (totalFrameP < 2)
                        {
                            /* Change status to no frame available for this camera and stream */
                            MUTEX_LOCK(lsPublic->dataMutex);
                            lsPublic->frameAvailable[camIndex][streamType] = FALSE;
                            MUTEX_UNLOCK(lsPublic->dataMutex);
                        }

                        /* Check how much time we have spent in processing */
                        if (ElapsedTick(framePushStartTime) >= CONVERT_SEC_TO_TIMER_COUNT(LIVE_STREAM_QUEUE_READ_TIME))
                        {
                            /* We have spent more than 1 second. Hence wait for next tick */
                            frameProcessStopF = TRUE;
                            break;
                        }
                    }
                    /* When first frame response given to client and but no new frame available from camera for client */
                    else if (TRUE == lsPrivate.firstCallBackGiven[camIndex][streamType])
                    {
                        /* Send video loss only once till it does not get recover again */
                        if ((FALSE == lsPrivate.frmHeader[camIndex][streamType].vidLoss)
                                && ((ElapsedTick(lsPrivate.lastFrameTime[camIndex][streamType]) + EARLY_VIDEO_LOSS_DECLARE_SYS_TICK)
                                    >= CONVERT_SEC_TO_TIMER_COUNT(generalConfig.preVideoLossDuration)))
                        {
                            WPRINT(LIVE_MEDIA_STREAMER, "finally declare video loss: [camera=%d], [sessionIdx=%d]", camIndex, lsPublic->clientIdx);
                            // Update time
                            lsPrivate.lastFrameTime[camIndex][streamType] = GetSysTick();
                            lsPrivate.sendFrameCountMPJPG[camIndex][streamType] = 0;

                            lsPrivate.frmHeader[camIndex][streamType].mediaStatus = (UINT8)MEDIA_NORMAL;
                            lsPrivate.frmHeader[camIndex][streamType].streamType = STREAM_TYPE_VIDEO;
                            lsPrivate.frmHeader[camIndex][streamType].frmType = I_FRAME;
                            lsPrivate.frmHeader[camIndex][streamType].vidLoss = TRUE;
                            GetLocalTime(&lsPrivate.frmHeader[camIndex][streamType].localTime);
                            lsPrivate.frmHeader[camIndex][streamType].mediaFrmLen = FRAME_HEADER_LEN_MAX;

                            if (sendDataCb[lsPublic->clientCbType](lsPrivate.connId[camIndex][streamType],
                                                                   (UINT8PTR)&lsPrivate.frmHeader[camIndex][streamType],
                                                                   FRAME_HEADER_LEN_MAX, MAX_LS_FRAME_SEND_TIME) == FAIL)
                            {
                                WPRINT(LIVE_MEDIA_STREAMER, "fail to send video loss header: [camera=%d], [sessionIdx=%d]", camIndex, lsPublic->clientIdx);
                                cleanupLiveMediaStream(camIndex, streamType, &lsPrivate, lsPublic);
                                if (lsPrivate.totalCamera == 0)
                                {
                                    /* No camera is there for streaming. Hence frame processing not required */
                                    frameProcessStopF = TRUE;
                                    break;
                                }
                            }
                        }

                        /* Check how much time we have spent in processing */
                        if (ElapsedTick(framePushStartTime) >= CONVERT_SEC_TO_TIMER_COUNT(LIVE_STREAM_QUEUE_READ_TIME))
                        {
                            /* We have spent more than 1 second. Hence wait for next tick */
                            frameProcessStopF = TRUE;
                            break;
                        }
                    }
                    /* When new frame available from camera for client but first frame response not given to client */
                    else if (TRUE == lsPrivate.localFramePending[camIndex][streamType])
                    {
                        MUTEX_LOCK(lsPublic->dataMutex);
                        lsPublic->frameAvailable[camIndex][streamType] = FALSE;
                        MUTEX_UNLOCK(lsPublic->dataMutex);
                    }
                }

                /* Check frame processing need to continue or wait */
                if (frameProcessStopF == TRUE)
                {
                    /* Wait for some time for frame processing */
                    break;
                }
            }

            /* Check frame processing need to continue or wait */
            if (frameProcessStopF == FALSE)
            {
                MUTEX_LOCK(lsPublic->dataMutex);
                memcpy(lsPrivate.localFramePending, lsPublic->frameAvailable, sizeof(lsPrivate.localFramePending));
                MUTEX_UNLOCK(lsPublic->dataMutex);

                /* Start serving from main stream of first camera */
                camCnt = 0;
                streamCnt = MAIN_STREAM;
            }
            else
            {
                /* Wait for some time for frame processing */
                frameProcessStopF = FALSE;

                /* We will start again from here on next tick */
                camCnt = camIndex;
                streamCnt = streamType;
                break;
            }

        } while (TRUE == isFrameAvailableForAnyCamera(lsPrivate.localFramePending));
    }

    lsPublic->threadRunStatus = INACTIVE;
    QUEUE_CLEANUP(lsPublic->lsQueue);
    MUTEX_UNLOCK(lsPublic->dataMutex);
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function Remove audio from live stream of requested client
 * @param   respParam
 * @return
 */
static void liveStreamCallback(const CI_STREAM_RESP_PARAM_t * respParam)
{
    UINT8				camIndex = GET_STREAM_INDEX(respParam->camIndex);
    VIDEO_TYPE_e		streamType = GET_STREAM_TYPE(respParam->camIndex);
    LS_TRG_PARAM_t		*lsTrg = NULL;
    LS_CLIENT_PUBLIC_t	*lsClient;

    lsClient = &lsClientPublic[respParam->clientIndex];
    switch (respParam->respCode)
    {
        case CI_STREAM_RESP_MEDIA:
        {
            /* Check frame is available or not for any camera */
            MUTEX_LOCK(lsClient->dataMutex);
            if (FALSE == isFrameAvailableForAnyCamera(lsClient->frameAvailable))
            {
                /* Update frame status */
                lsClient->frameAvailable[camIndex][streamType] = TRUE;
                pthread_cond_signal(&lsClient->condSignal);
            }
            else
            {
                /* Update frame status */
                lsClient->frameAvailable[camIndex][streamType] = TRUE;
            }
            MUTEX_UNLOCK(lsClient->dataMutex);
        }
        break;

        case CI_STREAM_RESP_START:
        {
            DPRINT(LIVE_MEDIA_STREAMER, "live stream start resp: [camera=%d], [sessionIdx=%d], [status=%d]",
                   camIndex, respParam->clientIndex, respParam->cmdStatus);
            lsTrg = malloc(sizeof(LS_TRG_PARAM_t));
            if (lsTrg == NULL)
            {
                break;
            }

            lsTrg->type = LS_TRG_FIRST_FRAME;
            lsTrg->camIndex = camIndex;
            lsTrg->nwCmdStatus = respParam->cmdStatus;
            lsTrg->streamType = streamType;
            if (addToLsQueue(respParam->clientIndex, lsTrg) != SUCCESS)
            {
                free(lsTrg);
            }
        }
        break;

        case CI_STREAM_RESP_CLOSE:
        {
            DPRINT(LIVE_MEDIA_STREAMER, "live stream stop notify: [camera=%d], [sessionIdx=%d]", camIndex, respParam->clientIndex);
            lsTrg = malloc(sizeof(LS_TRG_PARAM_t));
            if (lsTrg == NULL)
            {
                break;
            }

            lsTrg->type = LS_TRG_FORCE_STOP;
            lsTrg->camIndex = camIndex;
            lsTrg->streamType = streamType;
            if (addToLsQueue(respParam->clientIndex, lsTrg) != SUCCESS)
            {
                free(lsTrg);
            }
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It provides status of availability of frame for any cameras. It checks status of all
            camera and its main and sub stream status.
 * @param   frameStatusBuff - Pointer to frame status buffer of all cameras
 * @return  Returns TRUE if frame available of any cameras else returns FALSE
 */
static BOOL isFrameAvailableForAnyCamera(BOOL frameStatusBuff[MAX_CAMERA][MAX_STREAM])
{
    UINT8           camIndex;
    VIDEO_TYPE_e    streamType;

    for (camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
    {
        for (streamType = MAIN_STREAM; streamType < MAX_STREAM; streamType++)
        {
            /* Check frame for both streams of camera */
            if (frameStatusBuff[camIndex][streamType])
            {
                /* Frame is available */
                return TRUE;
            }
        }
    }

    /* Frame not available */
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It clears the live media stream session for client
 * @param   camIndex - Camera Index (Same for main and sub streams)
 * @param   streamType - Stream type: Main or Sub stream
 * @param   lsPrivate - Pointer to client specific private information
 * @param   lsPublic - Pointer to client specific public information
 */
static void cleanupLiveMediaStream(UINT8 camIndex, VIDEO_TYPE_e streamType,
                                   LS_CLIENT_PRIVATE_t *lsPrivate, LS_CLIENT_PUBLIC_t *lsPublic)
{
    /* Give trigger to camera interface to stop stream for this client */
    StopStream(GET_STREAM_MAPPED_CAMERA_ID(camIndex, streamType), (CI_STREAM_CLIENT_LIVE_START + lsPublic->clientIdx));

    /* Close communication socket with client */
    closeConnCb[lsPublic->clientCbType](&lsPrivate->connId[camIndex][streamType]);

    /* Update stream information */
    lsPrivate->totalCamera--;
    lsPrivate->firstCallBackGiven[camIndex][streamType] = FALSE;
    lsPrivate->streamSwitch[camIndex][streamType] = FALSE;

    MUTEX_LOCK(lsPublic->dataMutex);
    lsPublic->frameAvailable[camIndex][streamType] = FALSE;
    MUTEX_UNLOCK(lsPublic->dataMutex);

    /* Remove stream from count */
    setLiveStreamCnt(FALSE);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function returns live stream count
 * @return
 */
UINT32 GetLiveStreamCnt(void)
{
    MUTEX_LOCK(liveStreamCntLock);
    UINT32 totalLiveStream = liveStreamCnt;
    MUTEX_UNLOCK(liveStreamCntLock);
    return totalLiveStream;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Add message in queue
 * @param   clientIdx
 * @param   lsTrg
 * @return
 */
static BOOL addToLsQueue(UINT8 clientIdx, LS_TRG_PARAM_t *lsTrg)
{
    LS_CLIENT_PUBLIC_t *lsClient = &lsClientPublic[clientIdx];

    MUTEX_LOCK(lsClient->dataMutex);
    if (lsClient->threadRunStatus == INACTIVE)
    {
        MUTEX_UNLOCK(lsClient->dataMutex);
        return REFUSE;
    }

    if (lsClient->lsQueue.entryPtr[lsClient->lsQueue.writeIndex] != NULL)
    {
        MUTEX_UNLOCK(lsClient->dataMutex);
        return FAIL;
    }

    lsClient->lsQueue.entryPtr[lsClient->lsQueue.writeIndex] = lsTrg;
    UPDATE_RW_INDEX(lsClient->lsQueue, writeIndex);
    pthread_cond_signal(&lsClient->condSignal);
    MUTEX_UNLOCK(lsClient->dataMutex);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function increment or decrement live stream count
 * @param   addStreamCnt
 * @return
 */
static BOOL setLiveStreamCnt(BOOL addStreamCnt)
{
    MUTEX_LOCK(liveStreamCntLock);
    if (addStreamCnt == TRUE)
    {
        if(liveStreamCnt >= LIVE_STREAM_MAX)
        {
            MUTEX_UNLOCK(liveStreamCntLock);
            return FAIL;
        }

        liveStreamCnt++;
    }
    else if(liveStreamCnt > 0)
    {
        liveStreamCnt--;
    }
    MUTEX_UNLOCK(liveStreamCntLock);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function verify whether frame send to client or not depending on parameter requested
 * @param   cameraIndex
 * @param   streamType
 * @param   private
 * @return
 */
static BOOL isFrameNeedTosend(UINT8 cameraIndex, UINT8 streamType, LS_CLIENT_PRIVATE_t *lsClientData)
{
    if (lsClientData->streamStatusInfo->streamPara.streamCodecType == VIDEO_MJPG)
    {
        if (lsClientData->reqframeTypeMJPG[cameraIndex][streamType] == LS_STREAM_MJPG_ALL)
        {
            lsClientData->sendFrameCountMPJPG[cameraIndex][streamType] = 0;
            return TRUE;
        }

        if ((lsClientData->reqframeTypeMJPG[cameraIndex][streamType] == LS_STREAM_MJPG_ADAPTIVE) && (camMotionState[cameraIndex] == ACTIVE))
        {
            lsClientData->sendFrameCountMPJPG[cameraIndex][streamType] = 0;
            return TRUE;
        }

        if (lsClientData->streamStatusInfo->streamType == STREAM_TYPE_AUDIO)
        {
            return FALSE;
        }

        UINT8 camframeRate = 0, gop = 0, skipCont;
        GetCameraFpsGop(cameraIndex, streamType, &camframeRate, &gop);
        /* PARASOFT : CERT_C-STR31-a - streamType boundary is validated prior to this function */
        if(camframeRate <= lsClientData->reqfpsMJPG[cameraIndex][streamType])
        {
            lsClientData->sendFrameCountMPJPG[cameraIndex][streamType] = 0;
            return TRUE;
        }

        if(lsClientData->reqfpsMJPG[cameraIndex][streamType] >= 10)
        {
            /* Derive the frame skip count */
            skipCont = camframeRate / (camframeRate - lsClientData->reqfpsMJPG[cameraIndex][streamType]);

            /* Send frame and skip frame on particular count */
            if(lsClientData->sendFrameCountMPJPG[cameraIndex][streamType] < skipCont)
            {
                lsClientData->sendFrameCountMPJPG[cameraIndex][streamType]++;
                return TRUE;
            }

            lsClientData->sendFrameCountMPJPG[cameraIndex][streamType] = 0;
        }
        else
        {
            /* Derive the frame skip count */
            skipCont = camframeRate / lsClientData->reqfpsMJPG[cameraIndex][streamType];

            /* Skip frames and send frame after particular count */
            lsClientData->sendFrameCountMPJPG[cameraIndex][streamType]++;
            if(lsClientData->sendFrameCountMPJPG[cameraIndex][streamType] >= skipCont)
            {
                lsClientData->sendFrameCountMPJPG[cameraIndex][streamType] = 0;
                return TRUE;
            }
        }
    }
    else
    {
        lsClientData->sendFrameCountMPJPG[cameraIndex][streamType] = 0;
        if (lsClientData->reqframeType[cameraIndex][streamType] == LS_STREAM_ALL)
        {
            return TRUE;
        }

        if ((lsClientData->reqframeType[cameraIndex][streamType] == LS_STREAM_ADAPTIVE) && (camMotionState[cameraIndex] == ACTIVE))
        {
            return TRUE;
        }

        if (lsClientData->streamStatusInfo->streamType == STREAM_TYPE_AUDIO)
        {
            return FALSE;
        }

        if (lsClientData->streamStatusInfo->streamPara.videoStreamType == I_FRAME)
        {
            return TRUE;
        }
    }

    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function updates motion state of camera
 * @param   cameraIndex
 * @param   evtState
 */
void CamMotionNotify(UINT8 camIndex, BOOL evtState)
{
    DPRINT(LIVE_MEDIA_STREAMER, "motion state changed: [camera=%d], [old=%d],  [new=%d]",  camIndex, camMotionState[camIndex], evtState);
    camMotionState[camIndex] = evtState;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
