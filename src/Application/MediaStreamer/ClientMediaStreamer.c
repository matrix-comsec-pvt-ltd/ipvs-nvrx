// #################################################################################################
//  FILE BRIEF
// #################################################################################################
/**
@file		ClientMediaStreamer.c
@brief      This module used to play audio from any remote client to either device (local client)
            or camera(which supports two way audio).
*/
// #################################################################################################
//  @INCLUDES
// #################################################################################################
/* Library Includes */
#include <curl/curl.h>

/* Application Includes */
#include "ClientMediaStreamer.h"
#include "DebugLog.h"
#include "HttpClient.h"
#include "MediaStreamer.h"
#include "NetworkCommand.h"
#include "P2pInterface.h"
#include "SysTimer.h"
#include "UrlRequest.h"
#include "Utils.h"

// #################################################################################################
//  @DEFINES
// #################################################################################################
#define MAX_CLNT_AUD_STRM_BUF_SIZE (1 * MEGA_BYTE)
#define MAX_DATA_SZ                (1280)
#define MAX_FRAME_SZ               (FRAME_HEADER_LEN_MAX + MAX_DATA_SZ)
#define MAX_FRAME_IN_BUFFER        (780)  // 1MB / MAX_FRAME_SZ = ~794 Frames
#define MAX_FRAME_WAIT_TIME        (5)    // In second
#define MAX_FRAME_SEND_TIME        (5)    // In second
#define HTTP_TIMEOUT               (5)
#define TCP_REQ_TIMEOUT            (5)
#define SND_AUDIO_CMD_TIMEOUT      (8)  // In second

#define CLNT_AUDIO_XFER_STACK_SZ   (500 * KILO_BYTE)

#define CLOSE_P2P_DATA_XFER_FD(clientCbType)                  \
    if ((clientCbType) == CLIENT_CB_TYPE_P2P)                 \
    {                                                         \
        CloseP2pDataXferFd(P2P_CLIENT_FD_TYPE_TWO_WAY_AUDIO); \
    }

// #################################################################################################
//  @DATA TYPES
// #################################################################################################
// Frame information
typedef struct
{
    UINT8PTR framePtr;
    UINT32   frameLen;
} FRAME_INFO_t;

// This provide current Read / Write pointer inside buffer.
typedef struct
{
    INT16            rdPos;
    INT16            wrPos;
    INT16            maxWriteIndex;
    pthread_rwlock_t writeIndexLock;
    pthread_mutex_t  writeBuffLock;
    FRAME_INFO_t     frameInfo[MAX_FRAME_IN_BUFFER];
} BUFFER_MARKER_t;

// This provide current client stream information.
typedef struct
{
    UINT8PTR        bufferPtr;    // buffer pointer where stream of data to be stored
    UINT32          bufferSize;   // size of the frame buffer
    BUFFER_MARKER_t frameMarker;  // manages read / write index and relative frame info
} CLNT_STREAM_INFO_t;

// Provides information related to thread
typedef struct
{
    BOOL  runFlag;
    INT32 recvFd;
    INT32 sendFd;
} AUDIO_THREAD_INFO_t;

// #################################################################################################
//  @STATIC VARIABLES
// #################################################################################################
static UINT8               clientAudioBuffer[MAX_CLNT_AUD_STRM_BUF_SIZE];
static CLNT_STREAM_INFO_t  streamInfo;
static AUDIO_THREAD_INFO_t audioThreadInfo;
static pthread_mutex_t     dataLock;
static CLIENT_AUD_INFO_t   clientAudInfo;
static AUD_TO_CAM_INFO_t   audToCamInfo;

// #################################################################################################
//  @PROTOTYPES
// #################################################################################################
//-------------------------------------------------------------------------------------------------
static void stopClientMediaStreamer(void);
//-------------------------------------------------------------------------------------------------
static void resetClientMediaStreamer(void);
//-------------------------------------------------------------------------------------------------
static void writeToClientAudioBuf(UINT8PTR frameBuf, UINT32 frameLen);
//-------------------------------------------------------------------------------------------------
static UINT32 readFromClientAudioBuf(UINT8PTR *streamData, UINT32PTR streamDataLen);
//-------------------------------------------------------------------------------------------------
static VOIDPTR clientAudioTransferThread(VOIDPTR arg);
//-------------------------------------------------------------------------------------------------
static BOOL recvFrame(INT32 connFd, CHARPTR rcvMsg, UINT32PTR rcvLen, UINT32 maxData, UINT32 timeout);
//-------------------------------------------------------------------------------------------------
static BOOL sendFrame(INT32 connFd, UINT8PTR sndBuff, UINT32 sndLen, UINT32 timeout);
//-------------------------------------------------------------------------------------------------
static BOOL sendTcpReq(UINT8 urlNoToSend);
//-------------------------------------------------------------------------------------------------
static void httpCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static BOOL sendHttpReq(UINT8 userData, HTTP_HANDLE *httpHandle);
//-------------------------------------------------------------------------------------------------
static BOOL cameraSetupToSendAudio(UINT8 urlNoToSend);
//-------------------------------------------------------------------------------------------------
// #################################################################################################
// @FUNCTIONS
// #################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   Initialization of client media streamer for two way audio
 */
void InitClientMediaStreamer(void)
{
    pthread_rwlock_init(&streamInfo.frameMarker.writeIndexLock, NULL);
    MUTEX_INIT(streamInfo.frameMarker.writeBuffLock, NULL);
    MUTEX_INIT(dataLock, NULL);

    MUTEX_LOCK(dataLock);
    audioThreadInfo.sendFd = INVALID_CONNECTION;
    audioThreadInfo.recvFd = INVALID_CONNECTION;
    MUTEX_UNLOCK(dataLock);
    resetClientMediaStreamer();
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function deinitialize client media streamer and do proper shut down of two way audio.
 */
void DeinitClientMediaStreamer(void)
{
    MUTEX_LOCK(dataLock);
    audioThreadInfo.runFlag = FALSE;
    MUTEX_UNLOCK(dataLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Terminate two way audio client media streamer process
 * @param   cameraIndex
 * @param   sessionIndex
 */
void TerminateClientMediaStreamer(UINT8 cameraIndex, UINT8 sessionIndex)
{
    /* Get Camera index on which Two way audio is running */
    MUTEX_LOCK(dataLock);
    if ((clientAudInfo.cameraIndex != cameraIndex) || (clientAudInfo.destination == CLIENT_AUDIO_DESTINATION_DEVICE) ||
        ((sessionIndex != INVALID_SESSION_INDEX) && (sessionIndex != clientAudInfo.sessionIndex)))
    {
        MUTEX_UNLOCK(dataLock);
        return;
    }
    MUTEX_UNLOCK(dataLock);

    /* Disable two audio only if it is affected by configuration change */
    DPRINT(LIVE_MEDIA_STREAMER, "stop two way audio: [camera=%d]", cameraIndex);
    MUTEX_LOCK(dataLock);
    audioThreadInfo.runFlag = FALSE;
    MUTEX_UNLOCK(dataLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function stops client media streamer.
 */
static void stopClientMediaStreamer(void)
{
    CHAR        details[MAX_EVENT_DETAIL_SIZE] = {0};
    CHAR        advncDetail[MAX_EVENT_ADVANCE_DETAIL_SIZE] = {0};
    HTTP_HANDLE httpHandle;
    UINT8       urlCnt = audToCamInfo.noOfUrlReq - audToCamInfo.noOfStopReq;

    MUTEX_LOCK(dataLock);
    if (audioThreadInfo.sendFd == INVALID_CONNECTION)
    {
        MUTEX_UNLOCK(dataLock);
        DPRINT(LIVE_MEDIA_STREAMER, "audio transfer already closed: [camera=%d]", clientAudInfo.cameraIndex);
        resetClientMediaStreamer();
        return;
    }
    MUTEX_UNLOCK(dataLock);

    if (clientAudInfo.destination == CLIENT_AUDIO_DESTINATION_CAMERA)
    {
        if (AUTO_ADDED_CAMERA == clientAudInfo.camType)
        {
            StopSendAudioToCameraRequest(clientAudInfo.cameraIndex);
        }
        else
        {
            if (audToCamInfo.noOfStopReq == 0)
            {
                return;
            }

            for (; urlCnt < audToCamInfo.noOfUrlReq; urlCnt++)
            {
                if (CAM_REQ_CONTROL == audToCamInfo.url[urlCnt].requestType)
                {
                    sendHttpReq(urlCnt, &httpHandle);
                    audToCamInfo.httpHandle[urlCnt] = httpHandle;
                }
            }
        }
    }
    else
    {
        snprintf(details, sizeof(details), "%02d", GET_CAMERA_NO(clientAudInfo.cameraIndex));
        MUTEX_LOCK(dataLock);
        snprintf(advncDetail, sizeof(advncDetail), "%d", clientAudInfo.streamerStatusCode);
        MUTEX_UNLOCK(dataLock);
        WriteEvent(LOG_SYSTEM_EVENT, LOG_TWO_WAY_AUDIO, details, advncDetail, EVENT_STOP);
    }

    /* PARASOFT: BD-TRS-DIFCS Variable used in multiple critical sections */
    MUTEX_LOCK(dataLock);
    DPRINT(LIVE_MEDIA_STREAMER, "audio transfer request close: [camera=%d], [fd=%d]", clientAudInfo.cameraIndex, audioThreadInfo.recvFd);
    MUTEX_UNLOCK(dataLock);
    resetClientMediaStreamer();
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function reset all the variables used for client media streamer.
 */
static void resetClientMediaStreamer(void)
{
    clientAudInfo.cameraIndex = INVALID_CAMERA_INDEX;
    clientAudInfo.destination = CLIENT_AUDIO_DESTINATION_MAX;
    clientAudInfo.sessionIndex = INVALID_SESSION_INDEX;
    clientAudInfo.camType = MAX_CAMERA_TYPE;

    streamInfo.bufferPtr = clientAudioBuffer;
    streamInfo.bufferSize = MAX_CLNT_AUD_STRM_BUF_SIZE;
    streamInfo.frameMarker.rdPos = 0;

    pthread_rwlock_wrlock(&streamInfo.frameMarker.writeIndexLock);
    streamInfo.frameMarker.wrPos = 0;
    streamInfo.frameMarker.maxWriteIndex = 1;
    pthread_rwlock_unlock(&streamInfo.frameMarker.writeIndexLock);
    streamInfo.frameMarker.frameInfo[0].framePtr = clientAudioBuffer;

    MUTEX_LOCK(dataLock);
    clientAudInfo.streamerStatusCode = CMD_SUCCESS;
    memset(audToCamInfo.httpHandle, 0, sizeof(audToCamInfo.httpHandle));
    MUTEX_UNLOCK(dataLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It writes into the client audio buffer used for two way audio.
 * @param   frameBuf
 * @param   frameLen
 */
static void writeToClientAudioBuf(UINT8PTR frameBuf, UINT32 frameLen)
{
    INT16    writeIndex, maxWriteIndex;
    UINT8PTR nextFramePtr;

    MUTEX_LOCK(streamInfo.frameMarker.writeBuffLock);
    pthread_rwlock_rdlock(&streamInfo.frameMarker.writeIndexLock);
    writeIndex = streamInfo.frameMarker.wrPos;
    maxWriteIndex = streamInfo.frameMarker.maxWriteIndex;
    pthread_rwlock_unlock(&streamInfo.frameMarker.writeIndexLock);

    /* if we have written 780 frames(1MB) already in buffer, assign starting of buf to framePtr again */
    if (writeIndex >= MAX_FRAME_IN_BUFFER)
    {
        maxWriteIndex = MAX_FRAME_IN_BUFFER;
        writeIndex = 0;
        streamInfo.frameMarker.frameInfo[writeIndex].framePtr = streamInfo.bufferPtr;
    }

    /* streamInfo->bufferPtr is a starting position of buffer of 1Mb.
     * so if current position - starting pos + current frame length > max buf size then our buf is overflowed */
    if ((streamInfo.frameMarker.frameInfo[writeIndex].framePtr - streamInfo.bufferPtr + frameLen) >= streamInfo.bufferSize)
    {
        maxWriteIndex = writeIndex;
        writeIndex = 0;
        streamInfo.frameMarker.frameInfo[writeIndex].framePtr = streamInfo.bufferPtr;
    }

    if ((frameBuf != NULL) && (frameLen > 0))
    {
        // Store the received frame to frame buffer at current write pointer
        memcpy((streamInfo.frameMarker.frameInfo[writeIndex].framePtr), frameBuf, frameLen);
    }

    streamInfo.frameMarker.frameInfo[writeIndex].frameLen = frameLen;

    // Set next frame pointer to current write pointer plus current frame length
    nextFramePtr = (streamInfo.frameMarker.frameInfo[writeIndex].framePtr + frameLen);

    // Increment write index by one
    writeIndex++;
    if (writeIndex < MAX_FRAME_IN_BUFFER)
    {
        streamInfo.frameMarker.frameInfo[writeIndex].framePtr = nextFramePtr;
    }

    if (maxWriteIndex < writeIndex)
    {
        maxWriteIndex = writeIndex;
    }

    pthread_rwlock_wrlock(&streamInfo.frameMarker.writeIndexLock);
    streamInfo.frameMarker.wrPos = writeIndex;
    streamInfo.frameMarker.maxWriteIndex = maxWriteIndex;
    pthread_rwlock_unlock(&streamInfo.frameMarker.writeIndexLock);
    MUTEX_UNLOCK(streamInfo.frameMarker.writeBuffLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function used to read a frame from the frame buffer. Puts frame address into streamData
 *          pointer and frame length in streamDataLen pointer and returns no of remaining frames in buffer.
 * @param   streamData
 * @param   streamDataLen
 * @return  No. of remaining frames to read in buffer
 */
static UINT32 readFromClientAudioBuf(UINT8PTR *streamData, UINT32PTR streamDataLen)
{
    INT16            maxWriteIdx, writeIndex, readIndex;
    UINT32           framesInBuff = 0;
    BUFFER_MARKER_t *frameMarkPtr = &streamInfo.frameMarker;

    pthread_rwlock_rdlock(&frameMarkPtr->writeIndexLock);
    writeIndex = frameMarkPtr->wrPos;
    maxWriteIdx = frameMarkPtr->maxWriteIndex;
    pthread_rwlock_unlock(&frameMarkPtr->writeIndexLock);

    readIndex = frameMarkPtr->rdPos;

    if ((writeIndex != readIndex) && (writeIndex < MAX_FRAME_IN_BUFFER))
    {
        if (readIndex >= maxWriteIdx)
        {
            readIndex = 0;
        }

        framesInBuff = (UINT32)((maxWriteIdx - (readIndex - writeIndex)) % maxWriteIdx);
        *streamData = frameMarkPtr->frameInfo[readIndex].framePtr;
        *streamDataLen = frameMarkPtr->frameInfo[readIndex].frameLen;
        frameMarkPtr->rdPos = (readIndex + 1);
    }
    return framesInBuff;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Checks if a free audio channel is available for use.
 * @return  TRUE if audio transfer thread is currently running and the audio channel is busy;
 *          FALSE if an audio transfer thread is not running and the channel is free.
 */
BOOL IsAudioTransferInProcess(void)
{
    MUTEX_LOCK(dataLock);
    BOOL status = audioThreadInfo.runFlag;
    MUTEX_UNLOCK(dataLock);
    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function sets up the required state and configurations for the audio transfer process,
 *          and subsequently launches a thread dedicated for handling the transfer of audio frames.
 * @param   clientAudRxInfo
 *
 * @return  SUCCESS if the initialization and thread start are successful;
 *          FAIL otherwise.
 */
BOOL StartClientAudioTransfer(CLIENT_AUD_INFO_t *clientAudRxInfo)
{
    CLIENT_AUD_INFO_t *pClientAudInfo = malloc(sizeof(CLIENT_AUD_INFO_t));
    if (NULL == pClientAudInfo)
    {
        EPRINT(LIVE_MEDIA_STREAMER, "fail to alloc memory");
        return FAIL;
    }

    /* Get local copy of input information */
    *pClientAudInfo = *clientAudRxInfo;

    /* PARASOFT: The release of dynamically allocated memory is handled within the thread function */
    if (FAIL == Utils_CreateThread(NULL, clientAudioTransferThread, pClientAudInfo, DETACHED_THREAD, CLNT_AUDIO_XFER_STACK_SZ))
    {
        EPRINT(LIVE_MEDIA_STREAMER, "fail to create client audio transfer thread");
        FREE_MEMORY(pClientAudInfo);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Thread function to read audio frames from the sender socket and transfer them
 *          to the specified destination based on user selection.
 * @param   arg
 * @return  None
 */
static VOIDPTR clientAudioTransferThread(VOIDPTR pThreadArg)
{
    CHAR               rcvFrameBuf[MAX_FRAME_SZ];
    UINT32             rcvLen;
    UINT32             recvFrameLen = 0;
    BOOL               isFirstFrameSended = FALSE;
    FRAME_HEADER_t    *pFrameHeader;
    UINT32             sendFrameLen;
    UINT8              skipDataLen;
    UINT8             *sendFramePtr;
    UINT32             framesInBuffer;
    BOOL               terminateSessionF = FALSE;
    BOOL               sendFdRecvdF = FALSE;
    CLIENT_AUD_INFO_t *pClientAudInfo = (CLIENT_AUD_INFO_t *)pThreadArg;
    static BOOL        threadExitFlag = TRUE;

    THREAD_START("CLNT_AUD_XFER");

    /* Previous thread is already running? */
    while (threadExitFlag == FALSE)
    {
        /* Wait to exit previous thread */
        usleep(10000);
    }

    /* PARASOFT: BD-TRS-DIFCS Variable used in multiple critical sections */
    MUTEX_LOCK(dataLock);
    audioThreadInfo.runFlag = TRUE;
    MUTEX_UNLOCK(dataLock);

    /* Start audio process for current session */
    resetClientMediaStreamer();
    threadExitFlag = FALSE;
    clientAudInfo = *pClientAudInfo;
    FREE_MEMORY(pClientAudInfo);

    do
    {
        /* For P2P, we will create two socket fds: one for sending data (received from P2P client) and other for receiving
         * (data sent by sending socket). We will connect these sockets with each other. Audio receive FD will be closed by
         * two way audio module same as native type */
        if (clientAudInfo.clientCbType == CLIENT_CB_TYPE_P2P)
        {
            MUTEX_LOCK(dataLock);
            if (FALSE == GetP2pDataXferFd(P2P_CLIENT_FD_TYPE_TWO_WAY_AUDIO, clientAudInfo.clientSockFd, &audioThreadInfo.recvFd))
            {
                MUTEX_UNLOCK(dataLock);
                EPRINT(LIVE_MEDIA_STREAMER, "fail to get audio recv fd for two way audio in p2p: [session=%d]", clientAudInfo.sessionIndex);
                terminateSessionF = TRUE;
                break;
            }
            MUTEX_UNLOCK(dataLock);
        }
        else
        {
            MUTEX_LOCK(dataLock);
            audioThreadInfo.recvFd = clientAudInfo.clientSockFd;
            MUTEX_UNLOCK(dataLock);
        }

        if (clientAudInfo.destination == CLIENT_AUDIO_DESTINATION_DEVICE)
        {
            AUDIO_OUT_CONFIG_t audioOutConfig;
            CHAR               camIndexStr[MAX_EVENT_DETAIL_SIZE] = {0};
            CHAR               audioOutPriorityStr[MAX_EVENT_ADVANCE_DETAIL_SIZE] = {0};

            ReadAudioOutConfig(&audioOutConfig);
            snprintf(camIndexStr, sizeof(camIndexStr), "%02d", GET_CAMERA_NO(clientAudInfo.cameraIndex));
            snprintf(audioOutPriorityStr, sizeof(audioOutPriorityStr), "%d", audioOutConfig.priority);
            WriteEvent(LOG_SYSTEM_EVENT, LOG_TWO_WAY_AUDIO, camIndexStr, audioOutPriorityStr, EVENT_START);
        }
        else
        {
            if (AUTO_ADDED_CAMERA == clientAudInfo.camType)
            {
                if (CMD_SUCCESS != StartSendAudioToCameraRequest(clientAudInfo.cameraIndex))
                {
                    EPRINT(LIVE_MEDIA_STREAMER, "fail to start CI camera audio: [camera=%d]", clientAudInfo.cameraIndex);
                    terminateSessionF = TRUE;
                    CLOSE_P2P_DATA_XFER_FD(clientAudInfo.clientCbType);
                    break;
                }
            }
            else
            {
                if (FAIL == cameraSetupToSendAudio(CAM_REQ_CONTROL))
                {
                    EPRINT(LIVE_MEDIA_STREAMER, "fail to start BM camera audio: [camera=%d]", clientAudInfo.cameraIndex);
                    terminateSessionF = TRUE;
                    CLOSE_P2P_DATA_XFER_FD(clientAudInfo.clientCbType);
                    break;
                }
            }
        }

    } while (0);

    /* Is session failed? */
    if (TRUE == terminateSessionF)
    {
        /* Send fail response to client and exit from thread */
        EPRINT(LIVE_MEDIA_STREAMER, "audio session terminated due to failure: [camera=%d]", clientAudInfo.cameraIndex);
        clientCmdRespCb[clientAudInfo.clientCbType](CMD_AUD_SND_REQ_FAIL, clientAudInfo.clientSockFd, TRUE);
        threadExitFlag = TRUE;
        MUTEX_LOCK(dataLock);
        audioThreadInfo.runFlag = FALSE;
        MUTEX_UNLOCK(dataLock);
        pthread_exit(NULL);
    }

    /* Send response call back to client. In case of P2P, We have to close this client callback fd because
     * we have allocated new streaming fd. In case of native, everything will be send/receive on same socket fd */
    clientCmdRespCb[clientAudInfo.clientCbType](clientAudInfo.streamerStatusCode, clientAudInfo.clientSockFd,
                                                (clientAudInfo.clientCbType == CLIENT_CB_TYPE_P2P) ? TRUE : FALSE);
    clientAudInfo.clientSockFd = INVALID_CONNECTION;

    /* Send frame header if destination is local client else skip it */
    skipDataLen = (clientAudInfo.destination == CLIENT_AUDIO_DESTINATION_DEVICE) ? 0 : FRAME_HEADER_LEN_MAX;

    while (TRUE)
    {
        /* Need to stop audio session? */
        if (FALSE == IsAudioTransferInProcess())
        {
            break;
        }

        do
        {
            /* In case of P2P, socket will be UDP. Hence we will get header and frame in same packet */
            recvFrameLen = 0;
            if (clientAudInfo.clientCbType == CLIENT_CB_TYPE_P2P)
            {
                /* Read data from UDP socket */
                if (SUCCESS != recvFrame(audioThreadInfo.recvFd, rcvFrameBuf, &rcvLen, MAX_FRAME_SZ, MAX_FRAME_WAIT_TIME))
                {
                    /* If packet not received with in timeout then stop whole process. If frame transmit started then
                     * stop transmit thread and wait for join, otherwise directly stop receiving thread. */
                    EPRINT(LIVE_MEDIA_STREAMER, "two way audio transfer thread failed in data receive");
                    terminateSessionF = TRUE;
                    break;
                }

                /* If header was not proper, read next frame */
                if (rcvLen < FRAME_HEADER_LEN_MAX)
                {
                    EPRINT(LIVE_MEDIA_STREAMER, "incomplete header received: [rcvLen=%d]", rcvLen);
                    break;
                }

                /* Read frame length from frame header and it should be same as received data */
                pFrameHeader = (FRAME_HEADER_t *)rcvFrameBuf;
                if (pFrameHeader->mediaFrmLen != rcvLen)
                {
                    EPRINT(LIVE_MEDIA_STREAMER, "incomplete frame received: [rcvLen=%d]", rcvLen);
                    break;
                }

                /* Update frame length */
                recvFrameLen = pFrameHeader->mediaFrmLen;
            }
            else
            {
                /* Read header from TCP socket */
                if (SUCCESS != recvFrame(audioThreadInfo.recvFd, rcvFrameBuf, &rcvLen, FRAME_HEADER_LEN_MAX, MAX_FRAME_WAIT_TIME))
                {
                    /* If packet not received with in timeout then stop whole process. If frame transmit started then
                     * stop transmit thread and wait for join, otherwise directly stop receiving thread. */
                    DPRINT(LIVE_MEDIA_STREAMER, "two way audio transfer thread failed in header receive");
                    terminateSessionF = TRUE;
                    break;
                }

                /* If header was not proper, read next frame */
                if (rcvLen != FRAME_HEADER_LEN_MAX)
                {
                    EPRINT(LIVE_MEDIA_STREAMER, "incomplete header received: [rcvLen=%d]", rcvLen);
                    break;
                }

                /* Read frame length from frame header */
                pFrameHeader = (FRAME_HEADER_t *)rcvFrameBuf;
                if (pFrameHeader->magicCode != MAGIC_CODE)
                {
                    EPRINT(LIVE_MEDIA_STREAMER, "invalid frame header: [magicCode=0x%X]", pFrameHeader->magicCode);
                    break;
                }

                recvFrameLen = pFrameHeader->mediaFrmLen - FRAME_HEADER_LEN_MAX;
                if (recvFrameLen > MAX_DATA_SZ)
                {
                    EPRINT(LIVE_MEDIA_STREAMER, "invalid frame length: [recvFrameLen=%d]", recvFrameLen);
                    recvFrameLen = 0;
                    break;
                }

                /* Read frame data from TCP socket */
                if (SUCCESS != recvFrame(audioThreadInfo.recvFd, (rcvFrameBuf + FRAME_HEADER_LEN_MAX), &rcvLen, recvFrameLen, MAX_FRAME_WAIT_TIME))
                {
                    /* If packet not received with in timeout then stop whole process. If frame transmit started then
                     * stop transmit thread and wait for join, otherwise directly stop receiving thread. */
                    DPRINT(LIVE_MEDIA_STREAMER, "two way audio transfer thread failed in data receive");
                    terminateSessionF = TRUE;
                    break;
                }

                /* If data len is zero that discard that frame */
                if (rcvLen == 0)
                {
                    EPRINT(LIVE_MEDIA_STREAMER, "no frame data received: [rcvLen=%d]", rcvLen);
                    recvFrameLen = 0;
                    break;
                }

                /* Calculate frame length including header */
                recvFrameLen = rcvLen + FRAME_HEADER_LEN_MAX;
            }

        } while (0);

        /* Terminate audio transfer thread if recv failed */
        if (terminateSessionF == TRUE)
        {
            break;
        }

        /* Check if audio send fd is received */
        if (sendFdRecvdF == FALSE)
        {
            MUTEX_LOCK(dataLock);
            if (audioThreadInfo.sendFd == INVALID_CONNECTION)
            {
                MUTEX_UNLOCK(dataLock);
                continue;
            }
            MUTEX_UNLOCK(dataLock);

            sendFdRecvdF = TRUE;
        }

        /* Write received frame into the buffer */
        if (recvFrameLen)
        {
            writeToClientAudioBuf((UINT8PTR)rcvFrameBuf, recvFrameLen);
        }

        /* Read frame from audio buffer */
        framesInBuffer = readFromClientAudioBuf(&sendFramePtr, &sendFrameLen);
        if (framesInBuffer == 0)
        {
            continue;
        }

        /* P2P flavor not required, as it always sends audio to device (local client) or camera */
        if (SUCCESS != sendFrame(audioThreadInfo.sendFd, sendFramePtr + skipDataLen, (sendFrameLen - skipDataLen), MAX_FRAME_SEND_TIME))
        {
            EPRINT(LIVE_MEDIA_STREAMER, "two way audio transfer thread failed in data transmit");
            if ((clientAudInfo.destination == CLIENT_AUDIO_DESTINATION_CAMERA) && (isFirstFrameSended == FALSE))
            {
                MUTEX_LOCK(dataLock);
                clientAudInfo.streamerStatusCode = CMD_AUD_CHANNEL_BUSY;
                MUTEX_UNLOCK(dataLock);
            }
            break;
        }

        if (isFirstFrameSended == FALSE)
        {
            isFirstFrameSended = TRUE;
        }
    }

    /* Stop device or camera audio */
    stopClientMediaStreamer();

    CLOSE_P2P_DATA_XFER_FD(clientAudInfo.clientCbType);
    MUTEX_LOCK(dataLock);
    CloseSocket(&audioThreadInfo.recvFd);
    CloseSocket(&audioThreadInfo.sendFd);
    audioThreadInfo.runFlag = FALSE;
    MUTEX_UNLOCK(dataLock);

    threadExitFlag = TRUE;
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function stops the client audio receive thread.
 * @param   sessionIndex
 */
void StopClientAudioReception(UINT8 sessionIndex)
{
    if (clientAudInfo.sessionIndex != sessionIndex)
    {
        return;
    }

    MUTEX_LOCK(dataLock);
    clientAudInfo.streamerStatusCode = CMD_SUCCESS;
    audioThreadInfo.runFlag = FALSE;
    MUTEX_UNLOCK(dataLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is used to read data from TCP socket.It reads data from socket provided in
 *          connFd, reads data upto maxData and wait for time period provided in timeout.It directly
 *          receives data at address provided by rcvMsg and puts recived data length at location
 *          provided by rcvLen.
 * @param   connFd
 * @param   rcvMsg
 * @param   rcvLen
 * @param   maxData
 * @param   timeout
 * @return  SUCCESS/FAIL
 */
static BOOL recvFrame(INT32 connFd, CHARPTR rcvMsg, UINT32PTR rcvLen, UINT32 maxData, UINT32 timeout)
{
    BOOL   chkMsgStat = FAIL;
    UINT8  pollSts;
    INT32  cnt;
    UINT32 recvCntSeg = 0;
    UINT64 prevTimeMs;
    INT16  pollRevent;
    UINT32 index = 0;
    UINT16 iterTimeoutMs = 100;
    UINT32 totalIterations = (timeout * MILLI_SEC_PER_SEC) / iterTimeoutMs;

    *rcvLen = 0;
    if (connFd == INVALID_CONNECTION)
    {
        return FAIL;
    }

    while (index < totalIterations)
    {
        index++;

        /* Is process terminated */
        if (FALSE == IsAudioTransferInProcess())
        {
            return FAIL;
        }

        /* Reset timestamp to start new iteration with new timeout */
        prevTimeMs = 0;

        /* Receive entire message upto timeout with max try */
        while (TRUE)
        {
            /* Wait for data or timeout occur */
            pollSts = GetSocketPollEvent(connFd, (POLLRDNORM | POLLRDHUP), GetRemainingPollTime(&prevTimeMs, iterTimeoutMs), &pollRevent);

            /* if poll failed */
            if (pollSts == FAIL)
            {
                EPRINT(LIVE_MEDIA_STREAMER, "poll failed: [fd=%d], [err=%s]", connFd, STR_ERR);
                break;
            }

            /* if poll timeout */
            if (pollSts == TIMEOUT)
            {
                break;
            }

            if ((pollRevent & POLLRDHUP) == POLLRDHUP)
            {
                EPRINT(LIVE_MEDIA_STREAMER, "remote connection closed: [fd=%d], [rcvLen=%d]", connFd, *rcvLen);
                return REFUSE;
            }

            /* if other than read event */
            if ((pollRevent & POLLRDNORM) != POLLRDNORM)
            {
                EPRINT(LIVE_MEDIA_STREAMER, "invalid poll event:[0x%x]", pollRevent);
                break;
            }

            cnt = recv(connFd, (rcvMsg + recvCntSeg), (maxData - recvCntSeg), MSG_DONTWAIT);
            if (cnt > 0)
            {
                recvCntSeg += cnt;
                chkMsgStat = SUCCESS;
                *rcvLen = recvCntSeg;
                if (recvCntSeg >= maxData)
                {
                    break;
                }
            }
            else
            {
                EPRINT(LIVE_MEDIA_STREAMER, "fail to recv data: [fd=%d], [status=%d], [rcvLen=%d], [err=%s]", connFd, cnt, *rcvLen, STR_ERR);
                break;
            }
        }

        if (recvCntSeg >= maxData)
        {
            break;
        }
    }

    if (index >= totalIterations)
    {
        EPRINT(LIVE_MEDIA_STREAMER, "recv timeout: [fd=%d], [rcvLen=%d]", connFd, *rcvLen);
    }

    if ((chkMsgStat == FAIL) && (errno != EWOULDBLOCK))
    {
        DPRINT(LIVE_MEDIA_STREAMER, "recv data error: [fd=%d], [err=%s]", connFd, STR_ERR);
    }

    return chkMsgStat;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function set connection fd of destination to send audio
 * @param   connFd
 */
void UpdateAudioSendFd(INT32 connFd)
{
    if (FALSE == IsAudioTransferInProcess())
    {
        EPRINT(LIVE_MEDIA_STREAMER, "audio session already stopped: [fd=%d]", connFd);
        CloseSocket(&connFd);
        return;
    }

    MUTEX_LOCK(dataLock);
    audioThreadInfo.sendFd = connFd;
    MUTEX_UNLOCK(dataLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function stops the audio transmitting to device and set stop reason code used to send in stop event.
 * @param   stopReasonCode
 */
void StopClientAudioTransmission(NET_CMD_STATUS_e stopReasonCode)
{
    /* This command is used for client audio to local client */
    if (clientAudInfo.destination != CLIENT_AUDIO_DESTINATION_DEVICE)
    {
        return;
    }

    MUTEX_LOCK(dataLock);
    clientAudInfo.streamerStatusCode = stopReasonCode;
    audioThreadInfo.runFlag = FALSE;
    MUTEX_UNLOCK(dataLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function send frame data to device or camera
 * @param   connFd
 * @param   sndBuff
 * @param   sndLen
 * @param   timeout
 * @return  SUCCESS/FAIL
 */
BOOL sendFrame(INT32 connFd, UINT8PTR sndBuff, UINT32 sndLen, UINT32 timeout)
{
    BOOL   sndStatus = FAIL;
    UINT8  pollSts;
    INT32  sndCnt = 0;
    UINT32 totalSend = 0;
    UINT32 index = 0;
    UINT64 prevTimeMs;
    INT16  pollRevent;
    UINT16 iterTimeoutMs = 100;
    UINT32 totalIterations = (timeout * MILLI_SEC_PER_SEC) / iterTimeoutMs;

    if (connFd == INVALID_CONNECTION)
    {
        DPRINT(LIVE_MEDIA_STREAMER, "invld fd found");
        return FAIL;
    }

    while (index < totalIterations)
    {
        index++;

        /* Is process terminated */
        if (FALSE == IsAudioTransferInProcess())
        {
            return FAIL;
        }

        /* Reset timestamp to start new iteration with new timeout */
        prevTimeMs = 0;

        while (TRUE)
        {
            pollSts = GetSocketPollEvent(connFd, (POLLWRNORM | POLLRDHUP), GetRemainingPollTime(&prevTimeMs, iterTimeoutMs), &pollRevent);

            if (TIMEOUT == pollSts)
            {
                EPRINT(LIVE_MEDIA_STREAMER, "send timeout: [fd=%d], [packet=%d], [err=%s]", connFd, sndCnt, STR_ERR);
                break;
            }

            if (FAIL == pollSts)
            {
                EPRINT(LIVE_MEDIA_STREAMER, "poll failed: [fd=%d], [packet=%d], [err=%s]", connFd, sndCnt, STR_ERR);
                break;
            }

            /* Is remote connection closed event? */
            if ((pollRevent & POLLRDHUP) == POLLRDHUP)
            {
                EPRINT(LIVE_MEDIA_STREAMER, "remote connection closed: [fd=%d], [packet=%d]", connFd, sndCnt);
                return REFUSE;
            }

            /* if other than read event */
            if ((pollRevent & POLLWRNORM) != POLLWRNORM)
            {
                EPRINT(LIVE_MEDIA_STREAMER, "invalid poll event:[0x%x], [fd=%d], [packet=%d],", pollRevent, connFd, sndCnt);
                break;
            }

            sndCnt = send(connFd, sndBuff + totalSend, (sndLen - totalSend), MSG_NOSIGNAL);
            if (sndCnt > 0)
            {
                totalSend = totalSend + sndCnt;
                if (totalSend >= sndLen)
                {
                    sndStatus = SUCCESS;
                    break;
                }
            }
            else
            {
                if (!(errno == EAGAIN || errno == EWOULDBLOCK || errno == ENOBUFS))
                {
                    EPRINT(LIVE_MEDIA_STREAMER, "send failed: [fd=%d], [err=%s]", connFd, STR_ERR);
                    break;
                }
            }
        }

        if (totalSend >= sndLen)
        {
            sndStatus = SUCCESS;
            break;
        }

        if (errno == EPIPE)
        {
            break;
        }
    }

    return sndStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is used to send http request for set up of audio channel i.e. opening
 *          and closing audio channel.
 * @param   userData
 * @return  SUCCESS/FAIL
 */
static BOOL sendHttpReq(UINT8 userData, HTTP_HANDLE *httpHandle)
{
    HTTP_INFO_t httpInfo;

    memset(&httpInfo, 0, sizeof(httpInfo));
    httpInfo.authMethod = audToCamInfo.url[userData].authMethod;
    httpInfo.maxConnTime = HTTP_TIMEOUT;
    httpInfo.maxFrameTime = HTTP_TIMEOUT;
    httpInfo.port = audToCamInfo.httpPort;
    httpInfo.userAgent = audToCamInfo.httpUserAgent;
    httpInfo.interface = MAX_HTTP_INTERFACE;

    if (audToCamInfo.url[userData].httpRequestType == PUT_REQUEST)
    {
        snprintf(httpInfo.fileForPutReq, MAX_FILE_NAME_LENGTH, "%s", audToCamInfo.url[userData].fileForPutReq);
        httpInfo.sizeOfPutFile = audToCamInfo.url[userData].sizeOfPutFile;
        httpInfo.contentType = audToCamInfo.url[userData].httpContentType;
    }

    snprintf(httpInfo.httpUsrPwd.username, MAX_CAMERA_USERNAME_WIDTH, "%s", audToCamInfo.userName);
    snprintf(httpInfo.httpUsrPwd.password, MAX_CAMERA_PASSWORD_WIDTH, "%s", audToCamInfo.password);
    snprintf(httpInfo.ipAddress, MAX_CAMERA_ADDRESS_WIDTH, "%s", audToCamInfo.ipAddr);
    snprintf(httpInfo.relativeUrl, MAX_RELATIVE_URL_WIDTH, "%s", audToCamInfo.url[userData].relativeUrl);

    return StartHttp(audToCamInfo.url[userData].httpRequestType, &httpInfo, httpCb, (UINT32)userData, httpHandle);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   HTTP header read callback. It is added to identify old or new implementation of camera
 *          for two way audio. In old implementation, camera sends malformed http header, due to
 *          that curl doesn't recognise the 200 OK properly and it doesn't stop the session.
 *          Hence, if we receive malformed header ("Content-Length 0") then it is older
 *          implementation and we have to stop the current session and need to use legacy method.
 * @param   curlBuffer
 * @param   row
 * @param   column
 * @param   index
 * @return  Number of byte processed
 */
static size_t headerReadCallback(VOIDPTR curlBuffer, size_t row, size_t column, VOIDPTR index)
{
    size_t sizeToReturn = (row * column);

    /* If malformed header found then stop session and use legacy method */
    if (strstr((CHARPTR)curlBuffer, "Content-Length 0") != NULL)
    {
        /* Camera have older implementation and use legacy method */
        WPRINT(LIVE_MEDIA_STREAMER, "camera have old two way audio implementation");
        return 0;
    }

    /* Return the number of bytes read */
    return sizeToReturn;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is used to open a audio channel socket connection with camera and audio
 *          data was send on this socket.
 * @param   urlNoToSend
 * @return  SUCCESS/FAIL
 * @note    Matrix camera uses POST request and OEM camera uses PUT request.
 */
static BOOL sendTcpReq(UINT8 urlNoToSend)
{
    INT32              clientFd = INVALID_CONNECTION;
    INT32              connStatus;
    struct sockaddr_in clientParam;
    CURL              *pCurlHandle = NULL;
    CURLcode           curlResp = CURLE_OK;
    curl_socket_t      curlSockFd;
    CHAR               reqUrl[MAX_CAMERA_URI_WIDTH];
    CHAR               absoluteUrl[MAX_CAMERA_URI_WIDTH];
    struct curl_slist *slist = NULL;

    do
    {
        /* Matrix camera supports only PUT request for AudioIn */
        if (audToCamInfo.url[urlNoToSend].httpRequestType != POST_REQUEST)
        {
            /* No need to handle OEM camera as of now. We will see in the future. */
            break;
        }

        /* Create curl session */
        pCurlHandle = curl_easy_init();
        if (pCurlHandle == NULL)
        {
            EPRINT(LIVE_MEDIA_STREAMER, "fail to init curl for two way audio");
            return FAIL;
        }

        /* Inform curl to stop all progress meter */
        curl_easy_setopt(pCurlHandle, CURLOPT_NOPROGRESS, 1L);

        /* Option to inform curl to do not use signal */
        curl_easy_setopt(pCurlHandle, CURLOPT_NOSIGNAL, 1L);

        /* Set HTTP version none */
        curl_easy_setopt(pCurlHandle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_NONE);

        /* Disable verification of peer ssl certificate */
        curl_easy_setopt(pCurlHandle, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(pCurlHandle, CURLOPT_SSL_VERIFYHOST, 0L);

        /* Tcp connection timeout */
        curl_easy_setopt(pCurlHandle, CURLOPT_CONNECTTIMEOUT, TCP_REQ_TIMEOUT);

        /* Low speed limit & time */
        curl_easy_setopt(pCurlHandle, CURLOPT_LOW_SPEED_LIMIT, 1);
        curl_easy_setopt(pCurlHandle, CURLOPT_LOW_SPEED_TIME, TCP_REQ_TIMEOUT);

        /* Configure authentication credentials */
        curl_easy_setopt(pCurlHandle, CURLOPT_USERNAME, audToCamInfo.userName);
        curl_easy_setopt(pCurlHandle, CURLOPT_PASSWORD, audToCamInfo.password);

        /* Configure authentication method */
        curl_easy_setopt(pCurlHandle, CURLOPT_HTTPAUTH, CURLAUTH_ANY);

        /* Set the header read callback function */
        curl_easy_setopt(pCurlHandle, CURLOPT_HEADERFUNCTION, headerReadCallback);

        /* Search request type string in url */
        CHARPTR searchStart = strstr(audToCamInfo.url[urlNoToSend].relativeUrl, "POST");
        if (searchStart == NULL)
        {
            EPRINT(LIVE_MEDIA_STREAMER, "request type str not found in request url: [url=%s]", audToCamInfo.url[urlNoToSend].relativeUrl);
            break;
        }

        /* Skip request type and space */
        searchStart += strlen("POST") + 1;

        /* Search HTTP to get actual url */
        CHARPTR searchEnd = strstr(searchStart, "HTTP");
        if (searchEnd == NULL)
        {
            EPRINT(LIVE_MEDIA_STREAMER, "HTTP str not found in request url: [url=%s]", audToCamInfo.url[urlNoToSend].relativeUrl);
            break;
        }

        /* Prepare actual url */
        snprintf(reqUrl, MIN((UINT32)sizeof(reqUrl), (UINT32)(searchEnd - searchStart)), "%s", searchStart);

        /* Configure curl URL */
        snprintf(absoluteUrl, MAX_CAMERA_URI_WIDTH, "http://%s:%d%s", audToCamInfo.ipAddr, audToCamInfo.httpPort, reqUrl);
        curl_easy_setopt(pCurlHandle, CURLOPT_URL, absoluteUrl);

        /* Set HTTP POST request */
        curl_easy_setopt(pCurlHandle, CURLOPT_POST, 1L);

        /* Append content type in http header */
        slist = curl_slist_append(slist, "Content-Type: " AUDIO_MIME_TYPE_STR);
        if (slist == NULL)
        {
            EPRINT(LIVE_MEDIA_STREAMER, "fail to set content type in curl header: [url=%s]", absoluteUrl);
            break;
        }

        /* Bind http headers to curl session */
        curl_easy_setopt(pCurlHandle, CURLOPT_HTTPHEADER, slist);

        /* Curl easy perform */
        DPRINT(LIVE_MEDIA_STREAMER, "two way audio http session started: [urlNoToSend=%d], [url=%s]", urlNoToSend, absoluteUrl);
        curlResp = curl_easy_perform(pCurlHandle);
        if (CURLE_OK != curlResp)
        {
            EPRINT(LIVE_MEDIA_STREAMER, "fail to execute curl_easy_perform for two way audio: [curlResp=%d]", curlResp);
            break;
        }

        /* Verify HTTP response code */
        long respCode = CURLE_OK;

        /* last arg must be long ptr or ptr to char* */
        curlResp = curl_easy_getinfo(pCurlHandle, CURLINFO_RESPONSE_CODE, &respCode);
        if (CURLE_OK != curlResp)
        {
            EPRINT(LIVE_MEDIA_STREAMER, "fail to execute curl_easy_getinfo: [curlResp=%d]", curlResp);
            break;
        }

        /* Is it success response? */
        if (respCode != 200)
        {
            EPRINT(LIVE_MEDIA_STREAMER, "two way audio http session auth error: [respCode=%d]", (UINT32)respCode);
            break;
        }

        /* Get the curl socket file descriptor */
        curlResp = curl_easy_getinfo(pCurlHandle, CURLINFO_LASTSOCKET, &curlSockFd);
        if ((CURLE_OK != curlResp) || (curlSockFd == INVALID_CONNECTION))
        {
            EPRINT(LIVE_MEDIA_STREAMER, "fail to get curl socket for two way audio: [curlResp=%d], [curlSockFd=%d]", curlResp, curlSockFd);
            break;
        }

        clientFd = dup(curlSockFd);
        if (clientFd == INVALID_CONNECTION)
        {
            EPRINT(LIVE_MEDIA_STREAMER, "fail to duplicate the curl socket for two way audio: [err=%s]", STR_ERR);
            break;
        }

        /* Now we can use duplicated socket fd for further communication */
        DPRINT(LIVE_MEDIA_STREAMER, "curl socket duplicated for communication: [sockfd=%d]", clientFd);

    } while (0);

    /* Free slist */
    if (NULL != slist)
    {
        curl_slist_free_all(slist);
    }

    /* Free curl handle */
    if (pCurlHandle != NULL)
    {
        curl_easy_cleanup(pCurlHandle);
    }

    do
    {
        /* Is valid socket fd acquired? */
        if (clientFd != INVALID_CONNECTION)
        {
            DPRINT(LIVE_MEDIA_STREAMER, "two way audio http session done with success: [urlNoToSend=%d], [url=%s]", urlNoToSend, absoluteUrl);
            break;
        }

        /* Create socket */
        DPRINT(LIVE_MEDIA_STREAMER, "trying legacy method for two way audio http session: [urlNoToSend=%d], [url=%s]", urlNoToSend, absoluteUrl);
        clientFd = socket(AF_INET, TCP_SOCK_OPTIONS, 0);
        if (clientFd == INVALID_CONNECTION)
        {
            DPRINT(LIVE_MEDIA_STREAMER, "fail to create two way audio socket: [err=%s]", STR_ERR);
            return FAIL;
        }

        /* Socket parameters to connect with brand-model camera thorugh URL */
        memset(&clientParam, 0, sizeof(clientParam));
        clientParam.sin_family = AF_INET;
        clientParam.sin_addr.s_addr = inet_addr(audToCamInfo.ipAddr);
        clientParam.sin_port = htons(audToCamInfo.httpPort);

        /* Connecting to server */
        connStatus = connect(clientFd, (struct sockaddr *)&clientParam, sizeof(clientParam));
        if (connStatus < 0)
        {
            DPRINT(LIVE_MEDIA_STREAMER, "fail to connect with camera: [err=%s]", STR_ERR);
            CloseSocket(&clientFd);
            return FAIL;
        }

        if (SUCCESS != SendToSocket(clientFd, (UINT8PTR)audToCamInfo.url[urlNoToSend].relativeUrl, strlen(audToCamInfo.url[urlNoToSend].relativeUrl),
                                    TCP_REQ_TIMEOUT))
        {
            CloseSocket(&clientFd);
            return FAIL;
        }

    } while (0);

    UpdateAudioSendFd(clientFd);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is used to as callback of http request.It is used to take further action
 *          based on response received.
 * @param   httpHandle
 * @param   dataInfo
 */
static void httpCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo)
{
    UINT8 urlCnt;

    // checking that http callback is for present request or previous one. If from previous one that discard it.
    MUTEX_LOCK(dataLock);
    for (urlCnt = 0; urlCnt < MAX_SEND_AUDIO_URL; urlCnt++)
    {
        if (audToCamInfo.httpHandle[urlCnt] == 0)
        {
            break;
        }
    }
    MUTEX_UNLOCK(dataLock);

    if (urlCnt >= MAX_SEND_AUDIO_URL)
    {
        return;
    }

    switch (dataInfo->httpResponse)
    {
        case HTTP_CLOSE_ON_SUCCESS:
        default:
            break;

        case HTTP_SUCCESS:
            if ((dataInfo->frameSize > 0) && (dataInfo->storagePtr != NULL))
            {
                DPRINT(LIVE_MEDIA_STREAMER, "two way audio: [userdata=%d], [info=%s]", dataInfo->userData, (char *)dataInfo->storagePtr);
            }
            cameraSetupToSendAudio((UINT8)(dataInfo->userData + 1));
            break;

        case HTTP_ERROR:
        case HTTP_CLOSE_ON_ERROR:
            if ((dataInfo->frameSize > 0) && (dataInfo->storagePtr != NULL))
            {
                DPRINT(LIVE_MEDIA_STREAMER, "two way audio: [info=%s]", (char *)dataInfo->storagePtr);
            }
            MUTEX_LOCK(dataLock);
            clientAudInfo.streamerStatusCode = CMD_AUD_SND_REQ_FAIL;
            audioThreadInfo.runFlag = FALSE;
            MUTEX_UNLOCK(dataLock);
            break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is used to check conditions needed to satisfy before processing further
 *          for sending audio to camera.
 * @param   clientAudRxInfo
 * @return  NET_CMD_STATUS_e
 */
NET_CMD_STATUS_e ProcessClientAudioToCamera(CLIENT_AUD_INFO_t *clientAudRxInfo)
{
    return GenerateAudioToCameraRequest(clientAudRxInfo->cameraIndex, &clientAudRxInfo->camType, &audToCamInfo);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is used to send url one by one from the bunch.
 * @param   urlNoToSend
 * @return  Status
 */
BOOL cameraSetupToSendAudio(UINT8 urlNoToSend)
{
    BOOL        retVal = SUCCESS;
    UINT8       noOfStartReqUrl = audToCamInfo.noOfUrlReq - audToCamInfo.noOfStopReq;
    HTTP_HANDLE httpHandle;

    if (urlNoToSend >= noOfStartReqUrl)
    {
        return retVal;
    }

    switch (audToCamInfo.url[urlNoToSend].requestType)
    {
        case CAM_REQ_CONTROL:
            retVal = sendHttpReq(urlNoToSend, &httpHandle);
            audToCamInfo.httpHandle[urlNoToSend] = httpHandle;
            break;

        case CAM_REQ_MEDIA:
            retVal = sendTcpReq(urlNoToSend);
            break;

        default:
            break;
    }

    if (retVal == FAIL)
    {
        DPRINT(LIVE_MEDIA_STREAMER, "camera setup failed to send url: [camera=%d], [urlNo=%d]", clientAudInfo.cameraIndex, urlNoToSend);
        ProcTxAudioToCameraSetupFailure(CMD_AUD_SND_REQ_FAIL);
    }

    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It will process on Faiure during setup
 * @param   statusCode
 */
void ProcTxAudioToCameraSetupFailure(NET_CMD_STATUS_e statusCode)
{
    DPRINT(LIVE_MEDIA_STREAMER, "stop send audio to camera process: [camera=%d]", clientAudInfo.cameraIndex);
    MUTEX_LOCK(dataLock);
    clientAudInfo.streamerStatusCode = statusCode;
    audioThreadInfo.runFlag = FALSE;
    MUTEX_UNLOCK(dataLock);
}

// #################################################################################################
//  @END OF FILE
// #################################################################################################
