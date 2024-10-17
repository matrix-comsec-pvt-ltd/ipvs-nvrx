//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		RtspClient.h
@brief      RTSP client streaming application who communicates with NVR server application and
            provides interface for Live555 client
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "UtilCommon.h"
#include "RtspClient.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
/* RTSP client binary process name with number */
#define RTSP_CLIENT_NAME    "RTSP_CLIENT"

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static INT32 rtspClientMediaFd  = INVALID_CONNECTION;
static UINT8 clientId           = RTSP_CLIENT_APPL_COUNT;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static BOOL CreateServerSocket(INT32 *sockFd);
//-------------------------------------------------------------------------------------------------
static void ProcessRegisterMsg(INT32 connFd, RtspClientMsgInfo_t *pRtspClientMsg);
//-------------------------------------------------------------------------------------------------
static void ProcessDebugConfigMsg(INT32 connFd, RtspClientMsgInfo_t *pRtspClientMsg);
//-------------------------------------------------------------------------------------------------
static void ProcessStartStreamMsg(INT32 connFd, RtspClientMsgInfo_t *pRtspClientMsg);
//-------------------------------------------------------------------------------------------------
static void ProcessStopStreamMsg(INT32 connFd, RtspClientMsgInfo_t *pRtspClientMsg);
//-------------------------------------------------------------------------------------------------
static void MediaFrameCb(MediaFrameResp_e mediaResp, RTSP_HANDLE mediaHandle,
                         MEDIA_FRAME_INFO_t *frameInfo, UINT8 camIndex, UINT32 headSize, UINT32 offset);
//-------------------------------------------------------------------------------------------------
static BOOL RecvMsgResp(INT32 connFd, RtspClientMsgId_e msgId, UINT8 camIndex, void *pData, UINT32 dataLenMax);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   Entry level of code
 * @param   argc
 * @param   argv
 * @return  exit code of application
 */
int main (int argc, char **argv)
{
    UINT32              dataLen;
    INT32 				serverFd;
    INT32				clientConnFd;
    RtspClientMsgInfo_t rtspClientMsg;

    InitDebugLog();

    /* If client id is not provided in argument then exit */
    if(argc < 2)
    {
        EPRINT(RTSP_CLIENT, "insufficient args provided");
        exit(0);
    }

    /*get client id*/
    clientId = atoi(argv[1]);
    if (clientId >= RTSP_CLIENT_APPL_COUNT)
    {
        EPRINT(RTSP_CLIENT, "invld client id for application: [client=%d]", clientId);
        exit(0);
    }

    /*set thread name RTSP_CLIENT_X*/
    THREAD_START_INDEX(RTSP_CLIENT_NAME, clientId)

    DPRINT(RTSP_CLIENT, "rtsp client started: [client=%d]", clientId);

    InitRtspClient();

    if (FALSE == CreateServerSocket(&serverFd))
    {
        return -1;
    }

    while (TRUE)
    {
        /* Wait for client to connected */
        clientConnFd = accept(serverFd, NULL, NULL);
        if (clientConnFd < STATUS_OK)
        {
            //Message failed to try again
            EPRINT(RTSP_CLIENT, "fail to accept connection: [client=%d], [err=%s]", clientId, strerror(errno));
            sleep(1);
            continue;
        }

        /* Read data from TCP socket */
        dataLen = 0;
        if (SUCCESS != RecvUnixSockSeqPkt(clientConnFd, &rtspClientMsg, sizeof(RtspClientMsgInfo_t), UNIX_SOCK_TIMEOUT_IN_SEC, &dataLen))
        {
            EPRINT(RTSP_CLIENT, "fail to recv msg request: [client=%d]", clientId);
            CloseSocket(&clientConnFd);
            continue;
        }

        switch (rtspClientMsg.header.msgId)
        {
            case RTSP_CLIENT_MSG_ID_REGISTER:
            {
                ProcessRegisterMsg(clientConnFd, &rtspClientMsg);
            }
            break;

            case RTSP_CLIENT_MSG_ID_DEBUG_CONFIG:
            {
                ProcessDebugConfigMsg(clientConnFd, &rtspClientMsg);
            }
            break;

            case RTSP_CLIENT_MSG_ID_START_STREAM:
            {
                ProcessStartStreamMsg(clientConnFd, &rtspClientMsg);
            }
            break;

            case RTSP_CLIENT_MSG_ID_STOP_STREAM:
            {
                ProcessStopStreamMsg(clientConnFd, &rtspClientMsg);
            }
            break;

            default:
            {
                CloseSocket(&clientConnFd);
            }
            break;
        }
    }

    CloseSocket(&rtspClientMediaFd);
    return 0;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gets the free RTP Port for UDP stream.
 * @param   rtpPortPtr
 * @return
 */
BOOL AllocateRtpPort(UINT16PTR rtpPortPtr, UINT8 camIndex)
{
    UINT32              dataLen;
    RtspClientMsgInfo_t rtspClientMsg;

    if (rtspClientMediaFd == INVALID_CONNECTION)
    {
        EPRINT(RTSP_CLIENT, "rtsp interface not registered: [client=%d], [camera=%d]", clientId, camIndex);
        return FAIL;
    }

    rtspClientMsg.header.msgId = RTSP_CLIENT_MSG_ID_ALLOC_RTP_PORT;
    rtspClientMsg.header.msgType = MSG_TYPE_REQUEST;
    rtspClientMsg.header.status = CMD_SUCCESS;
    dataLen = sizeof(RtspClientHeader_t);

    if (SUCCESS != SendToSocket(rtspClientMediaFd, (UINT8PTR)&rtspClientMsg, dataLen, UNIX_SOCK_TIMEOUT_IN_SEC))
    {
        EPRINT(RTSP_CLIENT, "fail to send alloc rtp port request: [client=%d], [camera=%d]", clientId, camIndex);
        return FAIL;
    }

    /* Receive required message response */
    if (SUCCESS != RecvMsgResp(rtspClientMediaFd, RTSP_CLIENT_MSG_ID_ALLOC_RTP_PORT, camIndex, &rtspClientMsg, sizeof(RtspClientMsgInfo_t)))
    {
        EPRINT(RTSP_CLIENT, "fail to alloc rtp port: [client=%d], [camera=%d], [status=%d]", clientId, camIndex, rtspClientMsg.header.status);
        return FAIL;
    }

    *rtpPortPtr = rtspClientMsg.payload.rtpPort;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function clears the RTP Port for UDP stream.
 * @param   rtpPortPtr
 */
void DeallocRtpPort(UINT16PTR rtpPortPtr, UINT8 camIndex)
{
    UINT32              dataLen;
    RtspClientMsgInfo_t rtspClientMsg;

    if (rtspClientMediaFd == INVALID_CONNECTION)
    {
        EPRINT(RTSP_CLIENT, "rtsp interface not registered: [client=%d], [camera=%d]", clientId, camIndex);
        return;
    }

    rtspClientMsg.header.msgId = RTSP_CLIENT_MSG_ID_DEALLOC_RTP_PORT;
    rtspClientMsg.header.msgType = MSG_TYPE_REQUEST;
    rtspClientMsg.header.status = CMD_SUCCESS;
    rtspClientMsg.payload.rtpPort = *rtpPortPtr;
    dataLen = sizeof(RtspClientHeader_t) + sizeof(rtspClientMsg.payload.rtpPort);

    if (SUCCESS != SendToSocket(rtspClientMediaFd, (UINT8PTR)&rtspClientMsg, dataLen, UNIX_SOCK_TIMEOUT_IN_SEC))
    {
        EPRINT(RTSP_CLIENT, "fail to send dealloc rtp port request: [client=%d], [camera=%d]", clientId, camIndex);
        return;
    }

    /* Receive required message response */
    if (SUCCESS != RecvMsgResp(rtspClientMediaFd, RTSP_CLIENT_MSG_ID_DEALLOC_RTP_PORT, camIndex, &rtspClientMsg, sizeof(RtspClientMsgInfo_t)))
    {
        EPRINT(RTSP_CLIENT, "fail to dealloc rtp port: [client=%d], [camera=%d], [status=%d]", clientId, camIndex, rtspClientMsg.header.status);
        return;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   CreateSharedMemory
 * @param   mediaHandle
 * @param   shmFd
 * @param   shmBaseAddr
 * @param   streamType
 * @return
 */
BOOL CreateSharedMemory(RTSP_HANDLE mediaHandle, INT32PTR shmFd, UINT8PTR *shmBaseAddr, STREAM_TYPE_e streamType)
{
    UINT8   shmMemId;
    CHAR    name[NAME_MAX];
    size_t  memorySize;

    /* Get shared memory index for client */
    shmMemId = clientId + (mediaHandle * RTSP_CLIENT_APPL_COUNT);

    /* prepare name of shared memory file & its size */
    if (STREAM_TYPE_VIDEO == streamType)
    {
        snprintf(name, sizeof(name), SHM_NAME_VIDEO"_%d", shmMemId);
        memorySize = VIDEO_BUF_SIZE;
    }
    else
    {
        snprintf(name, sizeof(name), SHM_NAME_AUDIO"_%d", shmMemId);
        memorySize = AUDIO_BUF_SIZE;
    }

    /* open shared memory in read/write mode */
    return Utils_OpenSharedMemory(name, memorySize, shmFd, shmBaseAddr, FALSE);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   DestroySharedMemory
 * @param   mediaHandle
 * @param   shmFd
 * @param   shmBaseAddr
 * @param   streamType
 * @return
 */
BOOL DestroySharedMemory(RTSP_HANDLE mediaHandle, INT32PTR shmFd, UINT8PTR *shmBaseAddr, STREAM_TYPE_e streamType)
{
    UINT8   shmMemId;
    CHAR    name[NAME_MAX];
    size_t  memorySize;

    /* Get shared memory index for client */
    shmMemId = clientId + (mediaHandle * RTSP_CLIENT_APPL_COUNT);

    /* prepare name of shared memory file & its size */
    if (STREAM_TYPE_VIDEO == streamType)
    {
        snprintf(name, sizeof(name), SHM_NAME_VIDEO"_%d", shmMemId);
        memorySize = VIDEO_BUF_SIZE;
    }
    else
    {
        snprintf(name, sizeof(name), SHM_NAME_AUDIO"_%d", shmMemId);
        memorySize = AUDIO_BUF_SIZE;
    }

    /* open shared memory in read/write mode */
    Utils_DestroySharedMemory(name, memorySize, shmFd, shmBaseAddr);

    *shmBaseAddr = NULL;
    *shmFd = INVALID_FILE_FD;
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   CreateServerSocket
 * @param   sockFd
 * @return
 */
static BOOL CreateServerSocket(INT32 *sockFd)
{
    struct sockaddr_un internalRtspServerAddr;

    //Create local server to send rtsp frames to main application
    *sockFd = socket(AF_UNIX, (SOCK_SEQPACKET | SOCK_CLOEXEC), 0);
    if (*sockFd == INVALID_CONNECTION)
    {
        EPRINT(RTSP_CLIENT, "fail to open socket: [client=%d], [err=%s]", clientId, strerror(errno));
        return FALSE;
    }

    memset(&internalRtspServerAddr, 0, sizeof(internalRtspServerAddr));
    snprintf(internalRtspServerAddr.sun_path, sizeof(internalRtspServerAddr.sun_path), RTSP_CLIENT_UNIX_SOCKET"_%d", clientId);
    internalRtspServerAddr.sun_family = AF_UNIX;
    unlink(internalRtspServerAddr.sun_path);

    if (bind(*sockFd, (struct sockaddr*)&internalRtspServerAddr, sizeof(internalRtspServerAddr)) == -1)
    {
        EPRINT(RTSP_CLIENT, "fail to bind socket: [client=%d], [err=%s]", clientId, strerror(errno));
        CloseSocket(sockFd);
        return FALSE;
    }

    //Changing permision of socket file
    chmod(internalRtspServerAddr.sun_path, 0777);

    if (listen(*sockFd, MAX_CAMERA) == -1)
    {
        EPRINT(RTSP_CLIENT, "fail to listen socket: [client=%d], [err=%s]", clientId, strerror(errno));
        CloseSocket(sockFd);
        return FALSE;
    }

    DPRINT(RTSP_CLIENT, "rtsp client server socket created: [client=%d]", clientId);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   ProcessRegisterMsg
 * @param   connFd
 * @param   pRtspClientMsg
 */
static void ProcessRegisterMsg(INT32 connFd, RtspClientMsgInfo_t *pRtspClientMsg)
{
    UINT32 dataLen;

    CloseSocket(&rtspClientMediaFd);
    rtspClientMediaFd = connFd;

    pRtspClientMsg->header.msgType = MSG_TYPE_RESPONSE;
    pRtspClientMsg->header.status = CMD_SUCCESS;
    dataLen = sizeof(RtspClientHeader_t);

    if (SUCCESS != SendToSocket(connFd, (UINT8PTR)pRtspClientMsg, dataLen, UNIX_SOCK_TIMEOUT_IN_SEC))
    {
        EPRINT(RTSP_CLIENT, "fail to send register resp: [client=%d]", clientId);
        return;
    }

    DPRINT(RTSP_CLIENT, "rtsp interface registered: [client=%d]", clientId);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   ProcessDebugConfigMsg
 * @param   connFd
 * @param   pRtspClientMsg
 */
static void ProcessDebugConfigMsg(INT32 connFd, RtspClientMsgInfo_t *pRtspClientMsg)
{
    UINT32 dataLen;

    ReadDebugConfigFile();
    DPRINT(RTSP_CLIENT, "debug config updated successfully: [client=%d]", clientId);

    pRtspClientMsg->header.msgType = MSG_TYPE_RESPONSE;
    pRtspClientMsg->header.status = CMD_SUCCESS;
    dataLen = sizeof(RtspClientHeader_t);

    if (SUCCESS != SendToSocket(connFd, (UINT8PTR)pRtspClientMsg, dataLen, UNIX_SOCK_TIMEOUT_IN_SEC))
    {
        EPRINT(RTSP_CLIENT, "fail to send debug config resp: [client=%d]", clientId);
    }

    CloseSocket(&connFd);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   ProcessStartStreamMsg
 * @param   connFd
 * @param   pRtspClientMsg
 */
static void ProcessStartStreamMsg(INT32 connFd, RtspClientMsgInfo_t *pRtspClientMsg)
{
    UINT32 dataLen;

    pRtspClientMsg->header.msgType = MSG_TYPE_RESPONSE;
    pRtspClientMsg->header.status = StartRtspMedia(pRtspClientMsg->payload.startStreamInfo.mediaHandle,
                                                   &pRtspClientMsg->payload.startStreamInfo.rtspStreamInfo,
                                                   MediaFrameCb);
    dataLen = sizeof(RtspClientHeader_t);

    if (SUCCESS != SendToSocket(connFd, (UINT8PTR)pRtspClientMsg, dataLen, UNIX_SOCK_TIMEOUT_IN_SEC))
    {
        EPRINT(RTSP_CLIENT, "fail to send start stream resp: [client=%d]", clientId);
    }

    CloseSocket(&connFd);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   ProcessStopStreamMsg
 * @param   connFd
 * @param   pRtspClientMsg
 */
static void ProcessStopStreamMsg(INT32 connFd, RtspClientMsgInfo_t *pRtspClientMsg)
{
    UINT32 dataLen;

    pRtspClientMsg->header.msgType = MSG_TYPE_RESPONSE;
    pRtspClientMsg->header.status = StopRtspMedia(pRtspClientMsg->payload.mediaHandle);
    dataLen = sizeof(RtspClientHeader_t);

    if (SUCCESS != SendToSocket(connFd, (UINT8PTR)pRtspClientMsg, dataLen, UNIX_SOCK_TIMEOUT_IN_SEC))
    {
        EPRINT(RTSP_CLIENT, "fail to send stop stream resp: [client=%d]", clientId);
    }

    CloseSocket(&connFd);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   MediaFrameCb
 * @param   mediaResp
 * @param   mediaHandle
 * @param   frameInfo
 * @param   cameraIndex
 * @param   headSize
 * @param   offset
 * @return
 */
static void MediaFrameCb(MediaFrameResp_e mediaResp, RTSP_HANDLE mediaHandle, MEDIA_FRAME_INFO_t *frameInfo,
                         UINT8 camIndex, UINT32 headSize, UINT32 offset)
{
    UINT32              dataLen;
    RtspClientMsgInfo_t rtspClientMsg;

    if (rtspClientMediaFd == INVALID_CONNECTION)
    {
        EPRINT(RTSP_CLIENT, "rtsp interface not registered: [client=%d]", clientId);
        StopRtspMedia(mediaHandle);
        return;
    }

    rtspClientMsg.header.msgId = RTSP_CLIENT_MSG_ID_MEDIA_FRAME;
    rtspClientMsg.header.msgType = MSG_TYPE_REQUEST;
    rtspClientMsg.header.status = CMD_SUCCESS;
    rtspClientMsg.payload.mediaFrameInfo.response = mediaResp;
    rtspClientMsg.payload.mediaFrameInfo.camIndex = camIndex;
    rtspClientMsg.payload.mediaFrameInfo.mediaHandle = mediaHandle;
    rtspClientMsg.payload.mediaFrameInfo.headSize = headSize;
    rtspClientMsg.payload.mediaFrameInfo.offset = offset;

    dataLen = sizeof(RtspClientHeader_t) + sizeof(rtspClientMsg.payload.mediaFrameInfo);

    if (SUCCESS != SendToSocket(rtspClientMediaFd, (UINT8PTR)&rtspClientMsg, dataLen, UNIX_SOCK_TIMEOUT_IN_SEC))
    {
        EPRINT(RTSP_CLIENT, "fail to send media frame request: [client=%d], [camera=%d]", clientId, camIndex);
        return;
    }

    /* Receive required message response */
    if (SUCCESS != RecvMsgResp(rtspClientMediaFd, RTSP_CLIENT_MSG_ID_MEDIA_FRAME, camIndex, &rtspClientMsg, sizeof(RtspClientMsgInfo_t)))
    {
        EPRINT(RTSP_CLIENT, "negative media frame resp: [client=%d], [camera=%d], [status=%d]",
               clientId, camIndex, rtspClientMsg.header.status);
        return;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   RecvMsgResp
 * @param   connFd
 * @param   msgId
 * @param   camIndex
 * @param   pData
 * @param   dataLenMax
 * @return
 */
static BOOL RecvMsgResp(INT32 connFd, RtspClientMsgId_e msgId, UINT8 camIndex, void *pData, UINT32 dataLenMax)
{
    UINT32              retryCnt = 0;
    UINT32              dataLen;
    RtspClientMsgId_e   recvMsgId;

    do
    {
        if (SUCCESS != RecvUnixSockSeqPkt(connFd, pData, dataLenMax, UNIX_SOCK_TIMEOUT_IN_SEC, &dataLen))
        {
            EPRINT(RTSP_CLIENT, "fail to recv media frame resp: [client=%d], [camera=%d], [retryCnt=%d]", clientId, camIndex, ++retryCnt);
            continue;
        }

        /* Validate data length */
        if (dataLen < sizeof(RtspClientHeader_t))
        {
            EPRINT(RTSP_CLIENT, "invld data length recv: [client=%d], [camera=%d], [length=%d], [retryCnt=%d]", clientId, camIndex, dataLen, ++retryCnt);
            continue;
        }

        recvMsgId = ((RtspClientHeader_t *)pData)->msgId;
        if ((recvMsgId != msgId) || (((RtspClientHeader_t *)pData)->msgType != MSG_TYPE_RESPONSE))
        {
            EPRINT(RTSP_CLIENT, "invld resp recv: [client=%d], [recvMsgId=%d], [expMsgId=%dd], [camera=%d], [retryCnt=%d]",
                   clientId, recvMsgId, msgId, camIndex, ++retryCnt);
            continue;
        }

        /* Successfully received required message */
        if (((RtspClientHeader_t *)pData)->status != CMD_SUCCESS)
        {
            /* Failed response received against request */
            return FAIL;
        }

        /* Success response received against request */
        return SUCCESS;

    } while(TRUE);
}

//#################################################################################################
// END OF FILE
//#################################################################################################
