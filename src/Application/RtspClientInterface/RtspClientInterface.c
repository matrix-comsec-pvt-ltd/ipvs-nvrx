//#################################################################################################
// @FILE BRIEF
//#################################################################################################
/**
  * @file       RtspClientInterface.c
  * @brief      This Module is used for interfacing RTSP client applications. RTSP_CLIENT_APPL_COUNT
  *             number of RTSP client application and same number of RTSP interface thread will be
  *             started. RTSP_MEDIA_SESSION_MAX will be equaly divided into all the independent
  *             RTSP clients.
  */
//#################################################################################################
// @INCLUDES
//#################################################################################################
#include "RtspClientInterface.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
/* Path of script executing RTSP client application */
#define RTSP_CLIENT_APPL_PATH       BIN_DIR_PATH "/rtspClient.bin"

/* Stack size for media process thread */
#define MEDIA_PRO_THRD_STACK_SIZE   (1 * MEGA_BYTE)

/* Maximum file name of size of shared memory */
#define SHM_MEM_FILE_NAME_MAX       20

//#################################################################################################
// @STRUCT
//#################################################################################################
typedef struct
{
    INT32       shmFd;
    UINT8PTR    shmBaseAddr;
}SharedMemInfo_t;

typedef struct
{
    UINT8           clientId;
    BOOL            runFlag;
    INT32           connFd;
    pthread_t       threadId;
    RTSP_CALLBACK   rtspCb;
    SharedMemInfo_t shmInfo[MEDIA_SESSION_PER_RTSP_APPL][MAX_STREAM_TYPE];
}RtspClientInfo_t;

//#################################################################################################
// @VARIABLES
//#################################################################################################
/* Used to store RTSP client information such as clientId, registration status and shared memory info */
static RtspClientInfo_t clientInfo[RTSP_CLIENT_APPL_COUNT];

//#################################################################################################
// @FUNCTION PROTOTYPE
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void *mediaProcessThread(void *threadInfo);
//-------------------------------------------------------------------------------------------------
static void initRtspClientInfo(RtspClientInfo_t *pRtspClientInfo);
//-------------------------------------------------------------------------------------------------
static void deinitRtspClientInfo(RtspClientInfo_t *pRtspClientInfo);
//-------------------------------------------------------------------------------------------------
static void startRtspClientAppl(UINT8 clientId);
//-------------------------------------------------------------------------------------------------
static void stopRtspClientAppl(UINT8 clientId);
//-------------------------------------------------------------------------------------------------
static void restartRtspClientConnection(RtspClientInfo_t *pRtspClientInfo);
//-------------------------------------------------------------------------------------------------
static void ProcessMediaFrameMsg(RtspClientInfo_t *pRtspClientInfo, RtspClientMsgInfo_t *pRtspClientMsg);
//-------------------------------------------------------------------------------------------------
static void ProcessAllocRtpPortMsg(RtspClientInfo_t *pRtspClientInfo, RtspClientMsgInfo_t *pRtspClientMsg);
//-------------------------------------------------------------------------------------------------
static void ProcessDeallocRtpPortMsg(RtspClientInfo_t *pRtspClientInfo, RtspClientMsgInfo_t *pRtspClientMsg);
//-------------------------------------------------------------------------------------------------
static BOOL connectToRtspClient(UINT8 clientId, INT32PTR connFd);
//-------------------------------------------------------------------------------------------------
static BOOL RegisterToRtspClient(RtspClientInfo_t *pRtspClientInfo);
//-------------------------------------------------------------------------------------------------
static BOOL createSharedMemory(RTSP_HANDLE mediaHandle, STREAM_TYPE_e streamType, RtspClientInfo_t *pRtspClientInfo);
//-------------------------------------------------------------------------------------------------
static BOOL destroySharedMemory(RTSP_HANDLE mediaHandle, STREAM_TYPE_e streamType, RtspClientInfo_t *pRtspClientInfo);
//-------------------------------------------------------------------------------------------------

//#################################################################################################
// @FUNCTION DEFINATIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief This function initalize RTSP client interface. It will execute multiple
 *        RTSP client application instances defined by RTSP_CLIENT_APPL_COUNT.Also, coressponding
 *        mediaProcessThread() started from here.
 * @return
 */
void InitRtspClientInterface(void)
{    
    UINT8 clientId;

    for (clientId = 0; clientId < RTSP_CLIENT_APPL_COUNT; clientId++)
    {
        clientInfo[clientId].clientId = clientId;
        clientInfo[clientId].runFlag = TRUE;

        if (FAIL == Utils_CreateThread(&clientInfo[clientId].threadId, mediaProcessThread,
                                       (void*)&clientInfo[clientId], JOINABLE_THREAD, MEDIA_PRO_THRD_STACK_SIZE))
        {
            EPRINT(RTSP_IFACE, "fail to create media process thread: [client=%d]", clientId);
            clientInfo[clientId].runFlag = FALSE;
        }
        else
        {
            DPRINT(RTSP_IFACE, "media process thread started: [client=%d]", clientId);
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief Set run flag. All mediaProcessThreads will exit.
 *        Disconnect all the client.
 * @return
 */
void DeinitRtspClientInterface(void)
{
    UINT8 clientId;

    /* Disconnect all RTSP client socket and destroy shared memory */
    for (clientId = 0; clientId < RTSP_CLIENT_APPL_COUNT; clientId++)
    {
        if (TRUE == clientInfo[clientId].runFlag)
        {
            clientInfo[clientId].runFlag = FALSE;
            pthread_join(clientInfo[clientId].threadId, NULL);
            DPRINT(RTSP_IFACE, "media process thread exited: [client=%d]", clientId);
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief Send RTSP_IFACE_MSG_START_STREAM to RTSP client.
 * @param rtspInfo
 * @param callBack
 * @param mediaHndl
 * @return
 */
NET_CMD_STATUS_e StartRtspStream(RtspStreamInfo_t *rtspInfo, RTSP_CALLBACK callBack, RTSP_HANDLE *rtspHandle)
{
    UINT8               clientId;
    INT32               clientFd = INVALID_CONNECTION;
    UINT32              dataLen;
    RtspClientMsgInfo_t rtspClientMsg;
    RTSP_HANDLE         mediaHandle;

    /*divide the cameras by modulo operation*/
    clientId = (rtspInfo->camIndex % RTSP_CLIENT_APPL_COUNT);

    if (clientInfo[clientId].connFd == INVALID_CONNECTION)
    {
        EPRINT(RTSP_IFACE, "rtsp interface not registered: [client=%d], [camera=%d]", clientId, rtspInfo->camIndex);
        return CMD_PROCESS_ERROR;
    }

    if (FALSE == connectToRtspClient(clientId, &clientFd))
    {
        return CMD_PROCESS_ERROR;
    }

    /* Get media handle */
    mediaHandle = (rtspInfo->camIndex / RTSP_CLIENT_APPL_COUNT);

    rtspClientMsg.header.msgId = RTSP_CLIENT_MSG_ID_START_STREAM;
    rtspClientMsg.header.msgType = MSG_TYPE_REQUEST;
    rtspClientMsg.header.status = CMD_SUCCESS;
    rtspClientMsg.payload.startStreamInfo.mediaHandle = mediaHandle;
    rtspClientMsg.payload.startStreamInfo.rtspStreamInfo = *rtspInfo;
    dataLen = sizeof(RtspClientHeader_t) + sizeof(rtspClientMsg.payload.startStreamInfo);

    if (SUCCESS != SendToSocket(clientFd, (UINT8PTR)&rtspClientMsg, dataLen, UNIX_SOCK_TIMEOUT_IN_SEC))
    {
        EPRINT(RTSP_IFACE, "fail to send start stream request: [client=%d], [camera=%d]", clientId, rtspInfo->camIndex);
        CloseSocket(&clientFd);
        return CMD_PROCESS_ERROR;
    }

    if (SUCCESS != RecvUnixSockSeqPkt(clientFd, &rtspClientMsg, sizeof(RtspClientMsgInfo_t), UNIX_SOCK_TIMEOUT_IN_SEC, &dataLen))
    {
        EPRINT(RTSP_IFACE, "fail to recv start stream resp: [client=%d], [camera=%d]", clientId, rtspInfo->camIndex);
        CloseSocket(&clientFd);
        return CMD_PROCESS_ERROR;
    }

    CloseSocket(&clientFd);

    if ((rtspClientMsg.header.msgId != RTSP_CLIENT_MSG_ID_START_STREAM) || (rtspClientMsg.header.msgType != MSG_TYPE_RESPONSE))
    {
        EPRINT(RTSP_IFACE, "invld start stream resp: [client=%d], [camera=%d], [msgId=%d], [msgType=%d]",
               clientId, rtspInfo->camIndex, rtspClientMsg.header.msgId, rtspClientMsg.header.msgType);
        return CMD_PROCESS_ERROR;
    }

    if (rtspClientMsg.header.status != CMD_SUCCESS)
    {
        EPRINT(RTSP_IFACE, "fail to start stream: [client=%d], [camera=%d], [status=%d]",
               clientId, rtspInfo->camIndex, rtspClientMsg.header.status);
        StopRtspStream(rtspInfo->camIndex, mediaHandle);
        return rtspClientMsg.header.status;
    }

    /* Derive media index from camera number and provide it to camera interface for further operation on stream */
    *rtspHandle = mediaHandle;

    /*register RTSP callback to camera interface*/
    clientInfo[clientId].rtspCb = callBack;
    DPRINT(RTSP_IFACE, "start rtsp stream: [client=%d], [camera=%d]", clientId, rtspInfo->camIndex);

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief Send RTSP_IFACE_MSG_START_STREAM to RTSP client
 * @param mediaHndl
 * @return
 */
NET_CMD_STATUS_e StopRtspStream(UINT8 camIndex, RTSP_HANDLE rtspHandle)
{
    UINT8               clientId;
    INT32               clientFd = INVALID_CONNECTION;
    UINT32              dataLen;
    RtspClientMsgInfo_t rtspClientMsg;

    /*divide the cameras by modulo operation*/
    clientId = (camIndex % RTSP_CLIENT_APPL_COUNT);

    /*Invalid media handle*/
    if (rtspHandle >= MEDIA_SESSION_PER_RTSP_APPL)
    {
        EPRINT(RTSP_IFACE, "invld rtsp handle: [client=%d], [handle=%d]", clientId, rtspHandle);
        return CMD_PROCESS_ERROR;
    }

    if (clientInfo[clientId].connFd == INVALID_CONNECTION)
    {
        EPRINT(RTSP_IFACE, "rtsp interface not registered: [client=%d], [camera=%d]", clientId, camIndex);
        return CMD_PROCESS_ERROR;
    }

    if (FALSE == connectToRtspClient(clientId, &clientFd))
    {
        return CMD_PROCESS_ERROR;
    }

    rtspClientMsg.header.msgId = RTSP_CLIENT_MSG_ID_STOP_STREAM;
    rtspClientMsg.header.msgType = MSG_TYPE_REQUEST;
    rtspClientMsg.header.status = CMD_SUCCESS;
    rtspClientMsg.payload.mediaHandle = rtspHandle;
    dataLen = sizeof(RtspClientHeader_t) + sizeof(rtspClientMsg.payload.mediaHandle);

    if (SUCCESS != SendToSocket(clientFd, (UINT8PTR)&rtspClientMsg, dataLen, UNIX_SOCK_TIMEOUT_IN_SEC))
    {
        EPRINT(RTSP_IFACE, "fail to send stop stream request: [client=%d], [camera=%d]", clientId, camIndex);
        CloseSocket(&clientFd);
        return CMD_PROCESS_ERROR;
    }

    if (SUCCESS != RecvUnixSockSeqPkt(clientFd, &rtspClientMsg, sizeof(RtspClientMsgInfo_t), UNIX_SOCK_TIMEOUT_IN_SEC, &dataLen))
    {
        EPRINT(RTSP_IFACE, "fail to recv stop stream resp: [client=%d], [camera=%d]", clientId, camIndex);
        CloseSocket(&clientFd);
        return CMD_PROCESS_ERROR;
    }

    CloseSocket(&clientFd);

    if ((rtspClientMsg.header.msgId != RTSP_CLIENT_MSG_ID_STOP_STREAM) || (rtspClientMsg.header.msgType != MSG_TYPE_RESPONSE))
    {
        EPRINT(RTSP_IFACE, "invld stop stream resp: [client=%d], [camera=%d], [msgId=%d], [msgType=%d]",
               clientId, camIndex, rtspClientMsg.header.msgId, rtspClientMsg.header.msgType);
        return CMD_PROCESS_ERROR;
    }

    if (rtspClientMsg.header.status != CMD_SUCCESS)
    {
        EPRINT(RTSP_IFACE, "fail to stop stream: [client=%d], [camera=%d], [status=%d]",
               clientId, camIndex, rtspClientMsg.header.status);
        return rtspClientMsg.header.status;
    }

    DPRINT(RTSP_IFACE, "stop rtsp stream: [client=%d], [camera=%d]", clientId, camIndex);

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief Send RTSP_IFACE_MSG_DEBUG_CONFIG command each to RTSP client
 * @return
 */
void SetRtspClientDebugConfig(void)
{
    UINT8               clientId;
    INT32               clientFd = INVALID_CONNECTION;
    UINT32              dataLen;
    RtspClientMsgInfo_t rtspClientMsg;

    for(clientId = 0; clientId < RTSP_CLIENT_APPL_COUNT; clientId++)
    {
        if (clientInfo[clientId].connFd == INVALID_CONNECTION)
        {
            EPRINT(RTSP_IFACE, "rtsp interface not registered: [client=%d]", clientId);
            continue;
        }

        if (FALSE == connectToRtspClient(clientId, &clientFd))
        {
            continue;
        }

        rtspClientMsg.header.msgId = RTSP_CLIENT_MSG_ID_DEBUG_CONFIG;
        rtspClientMsg.header.msgType = MSG_TYPE_REQUEST;
        rtspClientMsg.header.status = CMD_SUCCESS;
        dataLen = sizeof(RtspClientHeader_t);

        if (SUCCESS != SendToSocket(clientFd, (UINT8PTR)&rtspClientMsg, dataLen, UNIX_SOCK_TIMEOUT_IN_SEC))
        {
            EPRINT(RTSP_IFACE, "fail to send debug config request: [client=%d]", clientId);
            CloseSocket(&clientFd);
            continue;
        }

        if (SUCCESS != RecvUnixSockSeqPkt(clientFd, &rtspClientMsg, sizeof(RtspClientMsgInfo_t), UNIX_SOCK_TIMEOUT_IN_SEC, &dataLen))
        {
            EPRINT(RTSP_IFACE, "fail to recv debug config resp: [client=%d]", clientId);
            CloseSocket(&clientFd);
            continue;
        }

        CloseSocket(&clientFd);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief mediaProcessThread
 * @param threadInfo
 * @return
 */
static void *mediaProcessThread(void *threadInfo)
{    
    RtspClientInfo_t    *pRtspClientInfo = (RtspClientInfo_t*)threadInfo;
    RtspClientMsgInfo_t rtspClientMsg;
    UINT32              dataLen;
    BOOL                retVal;

    /* Set thread name RTSP_IFACE_x */
    THREAD_START_INDEX("RTSP_IFACE", pRtspClientInfo->clientId);

    /* Initialize RTSP client info */
    initRtspClientInfo(pRtspClientInfo);

    /* Execute RTSP client applications*/
    startRtspClientAppl(pRtspClientInfo->clientId);

    while (pRtspClientInfo->runFlag)
    {
        /*Try to connect UNIX server*/
        if (FAIL == connectToRtspClient(pRtspClientInfo->clientId, &pRtspClientInfo->connFd))
        {
            EPRINT(RTSP_IFACE, "fail to connect with rtsp client: [client=%d]", pRtspClientInfo->clientId);
            sleep(1);
            continue;
        }

        if (FAIL == RegisterToRtspClient(pRtspClientInfo))
        {
            EPRINT(RTSP_IFACE, "fail to register with rtsp client: [client=%d]", pRtspClientInfo->clientId);
            CloseSocket(&pRtspClientInfo->connFd);
            sleep(1);
            continue;
        }

        while (pRtspClientInfo->runFlag)
        {
            retVal = RecvUnixSockSeqPkt(pRtspClientInfo->connFd, &rtspClientMsg, sizeof(RtspClientMsgInfo_t), UNIX_SOCK_TIMEOUT_IN_SEC, &dataLen);
            if (retVal == FAIL)
            {
                // receive message timeout
                continue;
            }

            if (retVal == REFUSE)
            {
                EPRINT(RTSP_IFACE, "remote connection closed of rtsp client: [client=%d]", pRtspClientInfo->clientId);
                restartRtspClientConnection(pRtspClientInfo);
                break;
            }

            switch(rtspClientMsg.header.msgId)
            {
                case RTSP_CLIENT_MSG_ID_MEDIA_FRAME:
                {
                    ProcessMediaFrameMsg(pRtspClientInfo, &rtspClientMsg);
                }
                break;

                case RTSP_CLIENT_MSG_ID_ALLOC_RTP_PORT:
                {
                    ProcessAllocRtpPortMsg(pRtspClientInfo, &rtspClientMsg);
                }
                break;

                case RTSP_CLIENT_MSG_ID_DEALLOC_RTP_PORT:
                {
                    ProcessDeallocRtpPortMsg(pRtspClientInfo, &rtspClientMsg);
                }
                break;

                default:
                {
                    /* Nothing to do */
                    EPRINT(RTSP_IFACE, "invld msg recv: [client=%d], [msgId=%d]", pRtspClientInfo->clientId, rtspClientMsg.header.msgId);
                }
                break;
            }
        }
    }

    /* Kill RTSP client process */
    stopRtspClientAppl(pRtspClientInfo->clientId);

    /* Deinitialization of client information */
    deinitRtspClientInfo(pRtspClientInfo);

    DPRINT(RTSP_IFACE, "thread exited: [client=%d]", pRtspClientInfo->clientId);
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief initRtspClientInfo
 * @param pRtspClientInfo
 */
static void initRtspClientInfo(RtspClientInfo_t *pRtspClientInfo)
{
    RTSP_HANDLE mediaHandle;

    pRtspClientInfo->connFd = INVALID_CONNECTION;
    pRtspClientInfo->rtspCb = NULL;

    for (mediaHandle = 0; mediaHandle < MEDIA_SESSION_PER_RTSP_APPL; mediaHandle++)
    {
        /* shared memory for video stream */
        pRtspClientInfo->shmInfo[mediaHandle][STREAM_TYPE_VIDEO].shmBaseAddr = NULL;
        pRtspClientInfo->shmInfo[mediaHandle][STREAM_TYPE_VIDEO].shmFd = INVALID_CONNECTION;

        /* shared memory for audio stream */
        pRtspClientInfo->shmInfo[mediaHandle][STREAM_TYPE_AUDIO].shmBaseAddr = NULL;
        pRtspClientInfo->shmInfo[mediaHandle][STREAM_TYPE_AUDIO].shmFd = INVALID_CONNECTION;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief deinitRtspClientInfo
 * @param pRtspClientInfo
 */
static void deinitRtspClientInfo(RtspClientInfo_t *pRtspClientInfo)
{
    RTSP_HANDLE mediaHandle;
    UINT8       camIndex;

    /* close UNIX socket for client */
    CloseSocket(&pRtspClientInfo->connFd);

    /* Unlink shared memory */
    for (mediaHandle = 0; mediaHandle < MEDIA_SESSION_PER_RTSP_APPL; mediaHandle++)
    {
        if (pRtspClientInfo->shmInfo[mediaHandle][STREAM_TYPE_VIDEO].shmBaseAddr != NULL)
        {
            destroySharedMemory(mediaHandle, STREAM_TYPE_VIDEO, pRtspClientInfo);
        }

        if (pRtspClientInfo->shmInfo[mediaHandle][STREAM_TYPE_AUDIO].shmBaseAddr != NULL)
        {
            destroySharedMemory(mediaHandle, STREAM_TYPE_AUDIO, pRtspClientInfo);
        }

        /* give rtsp callback to camera interface */
        if (pRtspClientInfo->rtspCb != NULL)
        {
            camIndex = pRtspClientInfo->clientId + (mediaHandle * RTSP_CLIENT_APPL_COUNT);
            if (camIndex < (2*getMaxCameraForCurrentVariant()))
            {
                pRtspClientInfo->rtspCb(RTSP_RESP_CODE_CONNECT_FAIL, NULL, NULL, camIndex);
            }
        }
    }

    /* Reset rtsp callback */
    pRtspClientInfo->rtspCb = NULL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief Execute instance of RTSP client application
 * @param clientId
 */
static void startRtspClientAppl(UINT8 clientId)
{    
    CHAR cmdBuff[NAME_MAX];

    /* As we are executing multiple instance of RTSP application .we give application count using command line argument. */
    snprintf(cmdBuff, sizeof(cmdBuff), RTSP_CLIENT_APPL_PATH" %d &", clientId);
    if (FALSE == ExeSysCmd(TRUE, cmdBuff))
    {
        EPRINT(RTSP_IFACE, "fail to start rtsp client application: [client=%d]", clientId);
    }
    else
    {
        DPRINT(RTSP_IFACE, "rtsp client application started: [client=%d]", clientId);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief stopRtspClientAppl
 * @param clientId
 */
static void stopRtspClientAppl(UINT8 clientId)
{
    CHAR cmdBuff[NAME_MAX];

    /*Kill client application*/
    snprintf(cmdBuff, sizeof(cmdBuff), RTSP_CLIENT_APPL_PATH" %d", clientId);
    KillProcess(cmdBuff, KILL_SIG);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief first disconnect client then restart
 * @param clientInfo
 * @return
 */
static void restartRtspClientConnection(RtspClientInfo_t *pRtspClientInfo)
{
    /* clear client information and again init it */
    deinitRtspClientInfo(pRtspClientInfo);
    initRtspClientInfo(pRtspClientInfo);

    /*run client aplication*/
    startRtspClientAppl(pRtspClientInfo->clientId);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief createSharedMemory
 * @param mediaIndex
 * @param streamType
 * @param pRtspClientInfo
 * @return
 */
static BOOL createSharedMemory(RTSP_HANDLE mediaHandle, STREAM_TYPE_e streamType, RtspClientInfo_t *pRtspClientInfo)
{
    UINT8   shmMemId;
    CHAR    shmName[NAME_MAX];
    size_t  memorySize;

    /* Get shared memory index for client */
    shmMemId = pRtspClientInfo->clientId + (mediaHandle * RTSP_CLIENT_APPL_COUNT);

    /* set memory size according to stream type */
    if (STREAM_TYPE_VIDEO == streamType)
    {
        snprintf(shmName, sizeof(shmName), SHM_NAME_VIDEO"_%d", shmMemId);
        memorySize = VIDEO_BUF_SIZE;
    }
    else
    {
        snprintf(shmName, sizeof(shmName), SHM_NAME_AUDIO"_%d", shmMemId);
        memorySize = AUDIO_BUF_SIZE;
    }

    /* open shared memory */
    return Utils_OpenSharedMemory(shmName, memorySize, &pRtspClientInfo->shmInfo[mediaHandle][streamType].shmFd,
                                  &pRtspClientInfo->shmInfo[mediaHandle][streamType].shmBaseAddr, TRUE);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief destroySharedMemory
 * @param mediaIndex
 * @param streamType
 * @param pRtspClientInfo
 * @return
 */
static BOOL destroySharedMemory(RTSP_HANDLE mediaHandle, STREAM_TYPE_e streamType, RtspClientInfo_t *pRtspClientInfo)
{
    UINT8   shmMemId;
    CHAR    shmName[NAME_MAX];
    size_t  memorySize;

    /* Get shared memory index for client */
    shmMemId = pRtspClientInfo->clientId + (mediaHandle * RTSP_CLIENT_APPL_COUNT);

    /* set memory size according to stream type */
    if (STREAM_TYPE_VIDEO == streamType)
    {
        snprintf(shmName, sizeof(shmName), SHM_NAME_VIDEO"_%d", shmMemId);
        memorySize = VIDEO_BUF_SIZE;
    }
    else
    {
        snprintf(shmName, sizeof(shmName), SHM_NAME_AUDIO"_%d", shmMemId);
        memorySize = AUDIO_BUF_SIZE;
    }

    /* open shared memory */
    Utils_DestroySharedMemory(shmName, memorySize, &pRtspClientInfo->shmInfo[mediaHandle][streamType].shmFd,
                              &pRtspClientInfo->shmInfo[mediaHandle][streamType].shmBaseAddr);

    /* reset shared memory fd & address */
    pRtspClientInfo->shmInfo[mediaHandle][streamType].shmBaseAddr = NULL;
    pRtspClientInfo->shmInfo[mediaHandle][streamType].shmFd = INVALID_CONNECTION;

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief connectToRtspClient
 * @param client: [input] Id of RTSP client application
 * @param connFd: [output] connection FD
 * @return
 */
static BOOL connectToRtspClient(UINT8 clientId, INT32PTR connFd)
{
    struct sockaddr_un socketParam;

    *connFd = socket(AF_UNIX, (SOCK_SEQPACKET | SOCK_CLOEXEC), 0);
    if (*connFd == INVALID_CONNECTION)
    {
        EPRINT(RTSP_IFACE, "fail to create socket: [client=%d], [err=%s]", clientId, strerror(errno));
        return FAIL;
    }

    memset(&socketParam, 0, sizeof(socketParam));
    socketParam.sun_family = AF_UNIX;
    snprintf(socketParam.sun_path, sizeof(socketParam.sun_path), RTSP_CLIENT_UNIX_SOCKET"_%d", clientId);

    /*connect to server*/
    if (connect(*connFd, (struct sockaddr*)&socketParam, sizeof(socketParam)) == -1)
    {
        EPRINT(RTSP_IFACE, "fail to connect socket: [client=%d], [err=%s]", clientId, strerror(errno));
        CloseSocket(connFd);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief RegisterToRtspClient
 * @param pRtspClientInfo
 * @return
 */
static BOOL RegisterToRtspClient(RtspClientInfo_t *pRtspClientInfo)
{
    UINT32              dataLen;
    RtspClientMsgInfo_t rtspClientMsg;

    rtspClientMsg.header.msgId = RTSP_CLIENT_MSG_ID_REGISTER;
    rtspClientMsg.header.msgType = MSG_TYPE_REQUEST;
    rtspClientMsg.header.status = CMD_SUCCESS;
    dataLen = sizeof(RtspClientHeader_t);

    if (SUCCESS != SendToSocket(pRtspClientInfo->connFd, (UINT8PTR)&rtspClientMsg, dataLen, UNIX_SOCK_TIMEOUT_IN_SEC))
    {
        EPRINT(RTSP_IFACE, "fail to send register request: [client=%d]", pRtspClientInfo->clientId);
        return FAIL;
    }

    if (SUCCESS != RecvUnixSockSeqPkt(pRtspClientInfo->connFd, &rtspClientMsg, sizeof(RtspClientMsgInfo_t), UNIX_SOCK_TIMEOUT_IN_SEC, &dataLen))
    {
        EPRINT(RTSP_IFACE, "fail to recv register resp: [client=%d]", pRtspClientInfo->clientId);
        return FAIL;
    }

    if ((rtspClientMsg.header.msgId != RTSP_CLIENT_MSG_ID_REGISTER) || (rtspClientMsg.header.msgType != MSG_TYPE_RESPONSE))
    {
        EPRINT(RTSP_IFACE, "invld register resp: [client=%d], [msgId=%d], [msgType=%d]",
               pRtspClientInfo->clientId, rtspClientMsg.header.msgId, rtspClientMsg.header.msgType);
        return FAIL;
    }

    if (rtspClientMsg.header.status != CMD_SUCCESS)
    {
        EPRINT(RTSP_IFACE, "registration failed: [client=%d], [status=%d]", pRtspClientInfo->clientId, rtspClientMsg.header.status);
        return FAIL;
    }

    DPRINT(RTSP_IFACE, "registered with rtsp client: [client=%d]", pRtspClientInfo->clientId);

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief ProcessMediaFrameMsg
 * @param msgPtr
 * @param threadDesc
 */
static void ProcessMediaFrameMsg(RtspClientInfo_t *pRtspClientInfo, RtspClientMsgInfo_t *pRtspClientMsg)
{
    UINT32              dataLen;
    UINT32              offset;
    MEDIA_FRAME_INFO_t  *pMediaFrameInfo;
    UINT8               camIndex = pRtspClientMsg->payload.mediaFrameInfo.camIndex;
    RTSP_HANDLE         mediaHandle = pRtspClientMsg->payload.mediaFrameInfo.mediaHandle;

    pRtspClientMsg->header.msgType = MSG_TYPE_RESPONSE;
    pRtspClientMsg->header.status = CMD_PROCESS_ERROR;
    dataLen = sizeof(RtspClientHeader_t);

    /* validate media handle */
    if (mediaHandle >= MEDIA_SESSION_PER_RTSP_APPL)
    {
        EPRINT(RTSP_IFACE, "invalid media handle: [client=%d], [camera=%d], [handle=%d]", pRtspClientInfo->clientId, camIndex, mediaHandle);
        return;
    }

    switch (pRtspClientMsg->payload.mediaFrameInfo.response)
    {
        case RTSP_RESP_CODE_CONFIG_VIDEO_DATA:
        {
            if (createSharedMemory(mediaHandle, STREAM_TYPE_VIDEO, pRtspClientInfo) == FAIL)
            {
                EPRINT(RTSP_IFACE, "creating video shared memory failed: [client=%d], [camera=%d]", pRtspClientInfo->clientId, camIndex);
                break;
            }
        }

        /* FALL THROUGH */
        case RTSP_RESP_CODE_VIDEO_DATA:
        {
            /* check if shared memory is available or not */
            pMediaFrameInfo = (MEDIA_FRAME_INFO_t *)pRtspClientInfo->shmInfo[mediaHandle][STREAM_TYPE_VIDEO].shmBaseAddr;
            if (pMediaFrameInfo == NULL)
            {
                break;
            }

            /* Only pass other than I-Frame and P-Frame to camera interface */
            if ((pMediaFrameInfo->videoInfo.frameType != I_FRAME) && (pMediaFrameInfo->videoInfo.frameType != P_FRAME))
            {
                WPRINT(RTSP_IFACE, "invld frame type received: [client=%d], [camera=%d], [frameType=%d], [codecType=%d]",
                       pRtspClientInfo->clientId, camIndex, pMediaFrameInfo->videoInfo.frameType, pMediaFrameInfo->codecType);
                pRtspClientMsg->header.status = CMD_SUCCESS;
                break;
            }

            offset = sizeof(MEDIA_FRAME_INFO_t) + pRtspClientMsg->payload.mediaFrameInfo.offset;
            if (pMediaFrameInfo->videoInfo.frameType != I_FRAME)
            {
                /* If frame type is I-frame then include header also [FRAME_INFO|SPS|PPS|FRAME] otherwise exclude it */
                offset += pRtspClientMsg->payload.mediaFrameInfo.headSize;
            }

            pRtspClientInfo->rtspCb(pRtspClientMsg->payload.mediaFrameInfo.response, ((UINT8PTR)pMediaFrameInfo + offset),
                                    pMediaFrameInfo, camIndex);
            pRtspClientMsg->header.status = CMD_SUCCESS;
        }
        break;

        case RTSP_RESP_CODE_CONFIG_AUDIO_DATA:
        {
            if (createSharedMemory(mediaHandle, STREAM_TYPE_AUDIO, pRtspClientInfo) == FAIL)
            {
                EPRINT(RTSP_IFACE, "creating audio shared memory failed: [client=%d], [camera=%d]", pRtspClientInfo->clientId, camIndex);
                break;
            }
        }

        /*fall through*/
        case RTSP_RESP_CODE_AUDIO_DATA:
        {
            /*check if shared memory is available or not*/
            pMediaFrameInfo = (MEDIA_FRAME_INFO_t *)pRtspClientInfo->shmInfo[mediaHandle][STREAM_TYPE_AUDIO].shmBaseAddr;
            if (pMediaFrameInfo == NULL)
            {
                break;
            }

            pRtspClientInfo->rtspCb(pRtspClientMsg->payload.mediaFrameInfo.response, ((UINT8PTR)pMediaFrameInfo + sizeof(MEDIA_FRAME_INFO_t) +
                                    pRtspClientMsg->payload.mediaFrameInfo.headSize + pRtspClientMsg->payload.mediaFrameInfo.offset),
                                    pMediaFrameInfo, camIndex);
            pRtspClientMsg->header.status = CMD_SUCCESS;
        }
        break;

        case RTSP_RESP_CODE_CONNECT_FAIL:
        case RTSP_RESP_CODE_CONN_CLOSE:
        case RTSP_RESP_CODE_FRAME_TIMEOUT:
        {
            pRtspClientInfo->rtspCb(pRtspClientMsg->payload.mediaFrameInfo.response, NULL, NULL, camIndex);

            if (pRtspClientInfo->shmInfo[mediaHandle][STREAM_TYPE_VIDEO].shmBaseAddr != NULL)
            {
                destroySharedMemory(mediaHandle, STREAM_TYPE_VIDEO, pRtspClientInfo);
            }

            if (pRtspClientInfo->shmInfo[mediaHandle][STREAM_TYPE_AUDIO].shmBaseAddr != NULL)
            {
                destroySharedMemory(mediaHandle, STREAM_TYPE_AUDIO, pRtspClientInfo);
            }
            pRtspClientMsg->header.status = CMD_SUCCESS;
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;

    }

    if (SUCCESS != SendToSocket(pRtspClientInfo->connFd, (UINT8PTR)pRtspClientMsg, dataLen, UNIX_SOCK_TIMEOUT_IN_SEC))
    {
        EPRINT(RTSP_IFACE, "fail to send media frame resp: [client=%d], [camera=%d]", pRtspClientInfo->clientId, camIndex);
    }

    return;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief ProcessAllocRtpPortMsg
 * @param pRtspClientInfo
 * @param pRtspClientMsg
 */
static void ProcessAllocRtpPortMsg(RtspClientInfo_t *pRtspClientInfo, RtspClientMsgInfo_t *pRtspClientMsg)
{
    UINT32 dataLen;

    pRtspClientMsg->header.msgType = MSG_TYPE_RESPONSE;

    if (FindFreeRtpPort(&pRtspClientMsg->payload.rtpPort) == SUCCESS)
    {
        pRtspClientMsg->header.status = CMD_SUCCESS;
        dataLen = sizeof(RtspClientHeader_t) + sizeof(pRtspClientMsg->payload.rtpPort);
    }
    else
    {
        pRtspClientMsg->header.status = CMD_RESOURCE_LIMIT;
        dataLen = sizeof(RtspClientHeader_t);
    }

    if (SUCCESS != SendToSocket(pRtspClientInfo->connFd, (UINT8PTR)pRtspClientMsg, dataLen, UNIX_SOCK_TIMEOUT_IN_SEC))
    {
        EPRINT(RTSP_IFACE, "fail to send alloc rtp port resp: [client=%d]", pRtspClientInfo->clientId);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief ProcessDeallocRtpPortMsg
 * @param pRtspClientInfo
 * @param pRtspClientMsg
 */
static void ProcessDeallocRtpPortMsg(RtspClientInfo_t *pRtspClientInfo, RtspClientMsgInfo_t *pRtspClientMsg)
{
    UINT32 dataLen;

    ClearRtpPort(&pRtspClientMsg->payload.rtpPort);

    pRtspClientMsg->header.msgType = MSG_TYPE_RESPONSE;
    pRtspClientMsg->header.status = CMD_SUCCESS;
    dataLen = sizeof(RtspClientHeader_t);

    if (SUCCESS != SendToSocket(pRtspClientInfo->connFd, (UINT8PTR)pRtspClientMsg, dataLen, UNIX_SOCK_TIMEOUT_IN_SEC))
    {
        EPRINT(RTSP_IFACE, "fail to send dealloc rtp port resp: [client=%d]", pRtspClientInfo->clientId);
    }
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
