/***********************************************************************************************
 *	Copyright (c) 2018, Matrix ComSec Pvt. Ltd.
 *
 * 	All right reserved. Matrix's source code is an unpublished work and the use
 *	of copyright notice does not imply otherwise.
 *
 *	This source code contains confidential, trade secret material of Matrix Telecom.
 *	Any attempt or participation in deciphering, decoding, reverse engineering or
 *	in any way altering the source code is strictly prohibited, unless the prior written
 *	consent of Matrix is obtained.
 *
 ***********************************************************************************************/

/***********************************************************************************************
* @INCLUDES
***********************************************************************************************/
#include "LiveMedia.h"
#include "ApplController.h"

/***********************************************************************************************
* @DEFINES
***********************************************************************************************/
/* frame header version */
#define FRAME_VERSION (2)

/* frame header validation magic code */
#define MAGIC_CODE (0x000001FF)

/*
 * store frames upto defined size (bytes).
 * !!! CAUTION !!!
 * take care before increasing this value as it may cause stack overflow.
 * increase LIVE_MEDIA_THREAD_STACK_SIZE in LiveMedia.h to accomodate more buffer size.
 */
#define VIDEO_BUFFER_TOTAL_SIZE_LIMIT (5ULL * MEGA_BYTE)

/* store max number of frames after which discard them */
#define VIDEO_BUFFER_NUM_FRAMES_LIMIT (120)

/* wait for data  on sokcet if not available try next time */
#define SOCKET_WAIT_DATA_TIMEOUT_MS (10)

/* buffering time before start playing */
#define VIDEO_BUFFERING_TIME_MS (100)

/* maximum ms time livemedia/decoder fsm should block (for debugging purpose only) */
#define FSM_PROCESSING_TIME_LIMIT_MS (150)

/* enable debug for decoder performance checking */
#define DECODER_PERFORMANCE_DEBUG 0
#define TRACK_DECODER_ID(decId) ((decId % MAX_WINDOWS) == 0)

/***********************************************************************************************
* @FUNCTION DEFINATION
***********************************************************************************************/
/**
 * @brief LiveMedia::LiveMedia
 * @param serverInfo
 * @param requestInfo
 * @param commandId
 * @param decoderId
 * @param strmId
 */
LiveMedia::LiveMedia(SERVER_INFO_t serverInfo, REQ_INFO_t &requestInfo, SET_COMMAND_e commandId, quint8 decoderId, quint8 strmId)
    : MediaRequest(serverInfo, requestInfo, commandId, decoderId)
{

    mServerSuccessRespPending = false;
    mLiveStreamId = strmId;
    mLiveMediaFsmState = LIVE_MEDIA_FSM_STATE_IDLE;
    mDecoderFsmState = DECODER_FSM_STATE_IDLE;
    setIsIframeRecive(false);
    mIsNewIframeReceived = false;
    mFrameReceiveCnt = 0;
    mVideoFrameCnt = 0;
    mAudioFrameCnt = 0;
    mDiscardFrame = false;
    mVideoLossStatus = NO_VIDEO_LOSS;
    mIsNewFrame = true;
    mHeaderBufferBegin = nullptr;
    mFrameBufferBegin = nullptr;
    mFrameBufferReader = nullptr;
    mFrameBufferWriter = nullptr;
    mFrameStartReference = nullptr;
    mHeaderBufferReader = 0;
    mHeaderBufferWriter = 0;
    mFrameLengthOffset = 0;
    mConsecutiveFramesTimeDiff = 0;
    mLastPlayedFrameTimestampMs = 0;
    mDecoderFsmTimeReference = 0;
    mFrameSize = 0;
    mHeaderList.clear();
    mFrameList.clear();
    memset(&mFrameHeader, 0x00, sizeof(FRAME_HEADER_t));
    mDecoderExecTimeMs = 0;
    mHeaderWritePtr = (char*)&mFrameHeader;
    mHeaderLengthOffset = 0;

    /* get live view type from app controller */
    mLiveViewType = ApplController::getInstance()->GetLiveViewType();
}

/**
 * @brief LiveMedia::~LiveMedia
 */
LiveMedia::~LiveMedia(void)
{

}

/**
 * @brief LiveMedia::run
 */
void LiveMedia::run(void)
{
    /* get the tcp socket object for communication with server */
    QTcpSocket tcpSocket;

    LiveMediaError_e ret = LIVE_MEDIA_NO_ERROR;

    /* array of structures to hold headers of defined max frames */
    FRAME_HEADER_t headerBuffer[VIDEO_BUFFER_NUM_FRAMES_LIMIT];

    /* local circular memory buffer to store video frames */
    char frameBuffer[VIDEO_BUFFER_TOTAL_SIZE_LIMIT];

    /* initialize all pointers */
    mHeaderBufferBegin = &headerBuffer[0];
    mFrameBufferBegin = &frameBuffer[0];

    mHeaderBufferReader = 0;
    mHeaderBufferWriter = 0;

    mFrameBufferReader = mFrameBufferBegin;
    mFrameBufferWriter = mFrameBufferBegin;

    /* thread name */
    SetThreadName();

    do
    {
        DPRINT(GUI_LIVE_MEDIA, "start stream request: [streamId=%d], [decId=%d], [viewType=%d]", mLiveStreamId, decId, mLiveViewType);

        /* Connect to server */
        if (LIVE_MEDIA_NO_ERROR != DoServerHandshake(tcpSocket))
        {
            break;
        }

        /* set fsm state to receive and feed media frames */
        mLiveMediaFsmState = LIVE_MEDIA_FSM_STATE_GET_VIDEO_HEADER;
        mDecoderFsmState = DECODER_FSM_STATE_BUFFERING;

        /*
         * QElapsedTimer utilizes monotonic time in NVR platform rather than epoch time. so safe to use.
         * check return value of fsmTimer.isMonotonic() for monotonic clock.
         */
        QElapsedTimer fsmTimer;

        /* start timer to calculate time taken by livemedia fsm & decoder fsm */
        fsmTimer.start();

        /* start timer to exit if no data available from scoket for predefined time */
        mSocketDataTimer.start();

        /*
        * ------------------------------------------------------------------------
        * loop till run flag is true
        * ------------------------------------------------------------------------
        */
        while (getRunFlag() == true)
        {
            /* Process fsm to receive header + frame from tcp socket */
            ret = ProcessMediaFsm(tcpSocket);

            /* Restart timer to calculate decoder fsm time */
            fsmTimer.restart();

            if ((LIVE_MEDIA_NO_ERROR != ret) && (LIVE_MEDIA_ERROR_OPERATION_IN_PROGRESS != ret))
            {
                EPRINT(GUI_LIVE_MEDIA, "stop stream: live media error: [streamId=%d], [decId=%d], [ret=%d]", mLiveStreamId, decId, ret);
                setRunFlag(false);
                break;
            }

            /* don't feed frames to decoder when stop flag is true */
            if (getStopFlag() == true)
            {
                DPRINT(GUI_LIVE_MEDIA, "stop stream: stop flag set: [streamId=%d], [decId=%d]", mLiveStreamId, decId);
                continue;
            }

            /* Process fsm to feed frame to decoder */
            ret = ProcessDecoderFsm();

            mDecoderExecTimeMs = (quint64)fsmTimer.restart();

            #if DECODER_PERFORMANCE_DEBUG
            if (TRACK_DECODER_ID(decId))
            #endif
            {
                if (mDecoderExecTimeMs > FSM_PROCESSING_TIME_LIMIT_MS)
                {
                    EPRINT(GUI_LIVE_MEDIA, "DecoderFsm exec time: [streamId=%d], [decId=%d], [time=%llu ms]", mLiveStreamId, decId, mDecoderExecTimeMs);
                }
            }

            if ((LIVE_MEDIA_NO_ERROR != ret) && (LIVE_MEDIA_ERROR_OPERATION_IN_PROGRESS != ret))
            {
                EPRINT(GUI_LIVE_MEDIA, "stop stream: decoding error: [streamId=%d], [decId=%d], [ret=%d]", mLiveStreamId, decId, ret);
                setRunFlag(false);
                break;
            }
        }

    } while(0);

    /* close socket if open */
    if (tcpSocket.isOpen())
    {
        /* close socket */
        tcpSocket.close();
    }

    /* if stop stream flag is true change status according to */
    if (getStopFlag() == true)
    {
        statusId = CMD_STREAM_STOPPED;
    }

    /* emit signal */
    emit sigMediaResponse(request.requestId, cmdId, statusId, request.payload);
}

/**
 * @brief LiveMedia::SetThreadName
 */
void LiveMedia::SetThreadName(void)
{
    /* thread name */
    char threadName[16];

    /* set thread name */
    snprintf(threadName, sizeof(threadName), "LIVE_VIEW_%d", mLiveStreamId);
    prctl(PR_SET_NAME, threadName, 0, 0, 0);
}

/**
 * @brief LiveMedia::DoServerHandshake
 * @return
 */
LiveMediaError_e LiveMedia::DoServerHandshake(QTcpSocket &tcpSocket)
{
    bool retval = false;

    /* connect to server */
    if (connectToServer(tcpSocket) == false)
    {
        EPRINT(GUI_LIVE_MEDIA, "cannot connect to server: [streamId=%d], [decId=%d], [statusId=%d]", mLiveStreamId, decId, statusId);
        return (LIVE_MEDIA_ERROR_IN_CONNECTION);
    }

    /* send request to server */
    if (sendRequest(tcpSocket) == false)
    {
        EPRINT(GUI_LIVE_MEDIA, "cannot start stream: fail to send request to server: [streamId=%d], [decId=%d], [statusId=%d]", mLiveStreamId, decId, statusId);
        return (LIVE_MEDIA_ERROR_OPERATION_FAILED);
    }

    /* receive response from server */
    retval = receiveResponse(tcpSocket, STREAM_RESPONSE_SIZE);

    DPRINT(GUI_LIVE_MEDIA, "start stream response: [streamId=%d], [decId=%d], [response=%d], [statusId=%d]", mLiveStreamId, decId, retval, statusId);

    /* check for failure */
    if ((retval != true) || (statusId != CMD_SUCCESS))
    {
        EPRINT(GUI_LIVE_MEDIA, "cannot start stream: fail to connect to server: [streamId=%d], [decId=%d], [statusId=%d]", mLiveStreamId, decId, statusId);
        return (LIVE_MEDIA_ERROR_OPERATION_FAILED);
    }

    /*
     * send success response only after first frame is accepted by decoder
     * In case there is no more decoder capacity to play video, success response
     * should not be sent and only failure response with decoder error should be sent.
     * Here GUI has to show the decoder limit icon on the camera window.
     * If success is returned from here and after that failre is sent, there will be glitch on UI
     * between blank screen and decoder limit icon.
     */
    mServerSuccessRespPending = true;

    return (LIVE_MEDIA_NO_ERROR);
}

/**
 * @brief LiveMedia::ProcessMediaFsm
 * @return
 */
LiveMediaError_e LiveMedia::ProcessMediaFsm(QTcpSocket &tcpSocket)
{
    LiveMediaError_e ret = LIVE_MEDIA_NO_ERROR;

    switch (mLiveMediaFsmState)
    {

    /* get header from tcp socket */
    case LIVE_MEDIA_FSM_STATE_GET_VIDEO_HEADER:
        ret = LiveMedia_ProcessHeader(tcpSocket);
        break;

    /* get frame from tcp socket */
    case LIVE_MEDIA_FSM_STATE_GET_VIDEO_FRAME:
        ret = LiveMedia_ProcessFrame(tcpSocket);
        break;

    default:
        break;

    }

    return (ret);
}

/**
 * @brief LiveMedia::LiveMedia_ProcessHeader
 * @return
 */
LiveMediaError_e LiveMedia::LiveMedia_ProcessHeader(QTcpSocket &tcpSocket)
{
    quint64 requestBytesFromSocket = 0;
    quint64 bytesRead = 0;
    quint16 timeoutMs = 0;

    /* get socket wait timeout based on time available to sleep */
    timeoutMs = GetSocketTimeoutWaitTimeMs();

    /* request remaining bytes header. If new header is requested value of mHeaderLengthOffset will be 0. */
    requestBytesFromSocket = (sizeof(FRAME_HEADER_t) - mHeaderLengthOffset);

    #if DECODER_PERFORMANCE_DEBUG
    if (TRACK_DECODER_ID(decId))
    {
        DPRINT(GUI_LIVE_MEDIA, "get header: [streamId=%d], [decId=%d], [frameCount=%d], [sockData=%llu bytes], [timeOut=%d ms]",
               mLiveStreamId, decId, mFrameList.count(), tcpSocket.bytesAvailable(), timeoutMs);
    }
    #endif

    /* receive full header in single socket read */
    if (false == receiveHeader_v2(mHeaderWritePtr,requestBytesFromSocket, tcpSocket, timeoutMs, &bytesRead))
    {
        /* check if for how long data is not available in socket.*/
        if (mSocketDataTimer.elapsed() > ((qint64)request.timeout * 1000))
        {
            EPRINT(GUI_LIVE_MEDIA, "no stream data available since [%lld ms]: [streamId=%d], [decId=%d]", mSocketDataTimer.elapsed(), mLiveStreamId, decId);
            return (LIVE_MEDIA_ERROR_IN_CONNECTION);
        }

        if (statusId == CMD_REQUEST_IN_PROGRESS)
        {
            return (LIVE_MEDIA_ERROR_OPERATION_IN_PROGRESS);
        }

        EPRINT(GUI_LIVE_MEDIA, "fail to receive header: [streamId=%d], [decId=%d], [statusId=%d]", mLiveStreamId, decId, statusId);
        return (LIVE_MEDIA_ERROR_IN_CONNECTION);
    }

    /* update total bytes received for current header */
    mHeaderLengthOffset += bytesRead;

    /* move write pointer */
    mHeaderWritePtr += bytesRead;

    /* if recevied bytes are not sufficient, then send status 'in progress' and read remaining bytes again */
    if (mHeaderLengthOffset < sizeof(FRAME_HEADER_t))
    {
        return (LIVE_MEDIA_ERROR_OPERATION_IN_PROGRESS);
    }

    /* complete header is received, reset write pointer */
    mHeaderLengthOffset = 0;
    mHeaderWritePtr = (char*)&mFrameHeader;

//    DPRINT(GUI_LIVE_MEDIA, "header received: [received=%llu bytes]", bytesRead);

    /* reset timer as we have received some data */
    mSocketDataTimer.restart();

    /* by default don't discard frame */
    mDiscardFrame = false;

    /* validate magic code in the header */
    if (mFrameHeader.magicCode != MediaRequest::magicCode)
    {
        EPRINT(GUI_LIVE_MEDIA, "invalid frame header magic code: [streamId=%d], [decId=%d]", mLiveStreamId, decId);
        return (LIVE_MEDIA_ERROR_INVALID_DATA);
    }

    /* ----------------------------------------------------------------------------
     * stream status validation (e.g. requested to stop stream)
     * ----------------------------------------------------------------------------
     */
    if (mFrameHeader.streamStatus != STREAM_NORMAL)
    {
        /* header status is not normal */
        switch (mFrameHeader.streamStatus)
        {

        case STREAM_CONFIG_CHANGE:
            EPRINT(GUI_LIVE_MEDIA, "configuration changed: [streamId=%d], [decId=%d]", mLiveStreamId, decId);
            statusId = CMD_CONFIG_CHANGED;
            break;

        default:
            EPRINT(GUI_LIVE_MEDIA, "other stream status: [streamId=%d], [decId=%d], [streamStatus=%d]", mLiveStreamId, decId, mFrameHeader.streamStatus);
            statusId = CMD_PROCESS_ERROR;
            break;
        }

        return (LIVE_MEDIA_ERROR_IN_CONNECTION);
    }

    /* ----------------------------------------------------------------------------
     * stream type & codec validation
     * ----------------------------------------------------------------------------
     */
    if (mFrameHeader.streamType == STREAM_TYPE_AUDIO)
    {
        /* audio codec */
        if (mFrameHeader.codecType >= MAX_AUDIO_CODEC)
        {
            statusId = CMD_PROCESS_ERROR;
            EPRINT(GUI_LIVE_MEDIA, "invalid audio codec: [streamId=%d], [decId=%d], [codecType=%d]", mLiveStreamId, decId, mFrameHeader.codecType);
            return (LIVE_MEDIA_ERROR_INVALID_DATA);
        }
    }
    else if (mFrameHeader.streamType == STREAM_TYPE_VIDEO)
    {
        /* video codec */
        if (mFrameHeader.codecType >= MAX_VIDEO_CODEC)
        {
            statusId = CMD_PROCESS_ERROR;
            EPRINT(GUI_LIVE_MEDIA, "invalid video codec: [streamId=%d], [decId=%d], [codecType=%d]", mLiveStreamId, decId, mFrameHeader.codecType);
            return (LIVE_MEDIA_ERROR_INVALID_DATA);
        }
    }
    else
    {
        statusId = CMD_PROCESS_ERROR;
        return (LIVE_MEDIA_ERROR_INVALID_DATA);
    }

    /* ----------------------------------------------------------------------------
     * video loss declared by server
     * ----------------------------------------------------------------------------
     */
    if (mFrameHeader.streamType == STREAM_TYPE_VIDEO)
    {
        if (mFrameHeader.videoLoss != mVideoLossStatus)
        {
            /* video loss status has been updated */
            mVideoLossStatus = mFrameHeader.videoLoss;

            /* set status id */
            if (mFrameHeader.videoLoss == VIDEO_LOSS)
            {
                EPRINT(GUI_LIVE_MEDIA, "video loss declared: [streamId=%d], [decId=%d]", mLiveStreamId, decId);
                statusId = CMD_STREAM_VIDEO_LOSS;
            }
            else
            {
                EPRINT(GUI_LIVE_MEDIA, "video loss recovered: [streamId=%d], [decId=%d]", mLiveStreamId, decId);
                statusId = CMD_STREAM_NO_VIDEO_LOSS;
            }

            /* emit signal to update stream status */
            emit sigMediaResponse(request.requestId, cmdId, statusId, request.payload);
        }

        /* discard this header and wait for next header as no frame will be received till video loss is not recovered */
        if (mFrameHeader.videoLoss == VIDEO_LOSS)
        {
            setIsIframeRecive(false);
            return (LIVE_MEDIA_ERROR_OPERATION_IN_PROGRESS);
        }
    }

    /* check for valid frame size */
    if (mFrameHeader.frameSize <= sizeof(FRAME_HEADER_t))
    {
        EPRINT(GUI_LIVE_MEDIA, "invalid frame size: [streamId=%d], [decId=%d], [size=%u]", mLiveStreamId, decId, mFrameHeader.frameSize);
        statusId = CMD_PROCESS_ERROR;
        return (LIVE_MEDIA_ERROR_INVALID_DATA);
    }

    /* ----------------------------------------------------------------------------
     * max allowed frame size validation
     * ----------------------------------------------------------------------------
     */
    if (((quint64)mFrameHeader.frameSize - sizeof(FRAME_HEADER_t)) > VIDEO_BUFFER_TOTAL_SIZE_LIMIT)
    {
        statusId = CMD_PROCESS_ERROR;
        EPRINT(GUI_LIVE_MEDIA, "frame size is too big to fit in buffer: [streamId=%d], [decId=%d]", mLiveStreamId, decId);
        return (LIVE_MEDIA_ERROR_FRAME_BUFFER_LIMIT);
    }

    /* ----------------------------------------------------------------------------
     * header buffer full (e.g. VIDEO_BUFFER_NUM_FRAMES_LIMIT frames received)
     * ----------------------------------------------------------------------------
     */
    if (true == IsHeaderBufferFull())
    {
        EPRINT(GUI_LIVE_MEDIA, "flush all frames: no space in header buffer: [streamId=%d], [decId=%d]", mLiveStreamId, decId);

        /* we don't have enough space to store header, so clear all saved frames */
        ClearAllFrames();
    }

    /* ----------------------------------------------------------------------------
     * frame buffer full (e.g. VIDEO_BUFFER_TOTAL_SIZE_LIMIT size of frame data received)
     * ----------------------------------------------------------------------------
     */
    if (((quint64)mFrameHeader.frameSize - sizeof(FRAME_HEADER_t)) >= GetFrameBufferFreeSpace())
    {
        EPRINT(GUI_LIVE_MEDIA, "flush all frames: no space in frame buffer: [streamId=%d], [decId=%d]", mLiveStreamId, decId);

        /* we don't have enough space to store frame, so clear whole buffer */
        ClearAllFrames();
    }

    /* ----------------------------------------------------------------------------
     * waiting for first I-frame
     * ----------------------------------------------------------------------------
     */
    if (getIsIframeRecive() == false)
    {
        /* we have to clear all frames if frames are available in buffer but we are waiting for fresh i-frame.
         * It happens when we switch the stream. Main to Sub and Sub to Main */
        quint32 frameCount = GetHeaderFrameList();
        if (frameCount > 0)
        {
            /* flush all frames */
            ClearAllFrames();
            WPRINT(GUI_LIVE_MEDIA, "flush all frames: requested: [streamId=%d], [decId=%d], [frameCount=%d]", mLiveStreamId, decId, frameCount);
        }

        /* start frame flush logic after receiving first i-frame */
        mIsNewIframeReceived = false;

        if (mFrameHeader.frameType == (FRAME_TYPE_e)I_FRAME)
        {
            setIsIframeRecive(true);
            DPRINT(GUI_LIVE_MEDIA, "stream started: receiving first I-frame: [streamId=%d], [decId=%d], [viewType=%d]", mLiveStreamId, decId, mLiveViewType);
        }
        else
        {
            /* read and discard frame */
            mDiscardFrame = true;
            //EPRINT(GUI_LIVE_MEDIA, "discard frame: waiting for first I-frame: [streamId=%d], [decId=%d]", mLiveStreamId, decId);
        }
    }

    /* mark flag to receive new frame */
    mIsNewFrame = true;

    /* change fsm state to receive frame */
    mLiveMediaFsmState = LIVE_MEDIA_FSM_STATE_GET_VIDEO_FRAME;

    return (LIVE_MEDIA_NO_ERROR);
}

/**
 * @brief LiveMedia::LiveMedia_ProcessFrame
 * @return
 */
LiveMediaError_e LiveMedia::LiveMedia_ProcessFrame(QTcpSocket &tcpSocket)
{
    quint64 requestBytesFromSocket = 0;
    quint64 bytesRead = 0;
    quint16 timeoutMs = 0;

    /* ----------------------------------------------------------------------------
     * initialization while starting new frame
     * ----------------------------------------------------------------------------
     */
    if (true == mIsNewFrame)
    {
        /* get length of frame size */
        mFrameLengthOffset = 0;
        mFrameSize = ((quint64)mFrameHeader.frameSize - sizeof(FRAME_HEADER_t));
        mFrameStartReference = mFrameBufferWriter;
        mIsNewFrame = false;
    }

    /* derive length of data to be received from socket */
    requestBytesFromSocket = (mFrameSize - mFrameLengthOffset);

    /* we cannot read full frame in one go if frame is bigger than circular buffer boundary */
    if ((mFrameBufferWriter + requestBytesFromSocket) > (mFrameBufferBegin + VIDEO_BUFFER_TOTAL_SIZE_LIMIT))
    {
        /* we cannot read full frame in one go due to frame is bigger than circular buffer boundary */
        requestBytesFromSocket = (((quint64)mFrameBufferBegin + VIDEO_BUFFER_TOTAL_SIZE_LIMIT) - (quint64)mFrameBufferWriter);
    }

    /* get socket wait timeout based on time available to sleep */
    timeoutMs = GetSocketTimeoutWaitTimeMs();

    #if DECODER_PERFORMANCE_DEBUG
    if (TRACK_DECODER_ID(decId))
    {
        DPRINT(GUI_LIVE_MEDIA, "get frame: [streamId=%d], [decId=%d], [frameCount=%d], [sockData=%llu bytes], [frameSize=%lld], [timeOut=%d ms]",
               mLiveStreamId, decId, mFrameList.count(), tcpSocket.bytesAvailable(), mFrameSize, timeoutMs);
    }
    #endif

    /* receive frame chunk */
    if (false == receiveFrame_v2(mFrameBufferWriter, requestBytesFromSocket, tcpSocket, timeoutMs, &bytesRead))
    {
        /* check if for how long ime data is not available in socket */
        if (mSocketDataTimer.elapsed() > ((qint64)request.timeout * 1000))
        {
            EPRINT(GUI_LIVE_MEDIA, "no stream data available since [%lld ms]: [streamId=%d], [decId=%d]", mSocketDataTimer.elapsed(), mLiveStreamId, decId);
            return (LIVE_MEDIA_ERROR_IN_CONNECTION);
        }

        if (CMD_REQUEST_IN_PROGRESS == statusId)
        {
            return (LIVE_MEDIA_ERROR_OPERATION_IN_PROGRESS);
        }

        EPRINT(GUI_LIVE_MEDIA, "fail to receive frame: [streamId=%d], [decId=%d]", mLiveStreamId, decId);
        return (LIVE_MEDIA_ERROR_IN_CONNECTION);
    }

//    DPRINT(GUI_LIVE_MEDIA, "frame received: [received=%llu bytes]", bytesRead);

    /* reset timer as we have received some data */
    mSocketDataTimer.restart();

    /* update total bytes received for current frame */
    mFrameLengthOffset += bytesRead;

    /* move write pointer */
    mFrameBufferWriter += bytesRead;

    /* reset write pointer if overflow considering circular buffer */
    if (mFrameBufferWriter >= (mFrameBufferBegin + VIDEO_BUFFER_TOTAL_SIZE_LIMIT))
    {
        mFrameBufferWriter = mFrameBufferBegin;
    }

    /* check if full frame is received */
    if (mFrameLengthOffset < mFrameSize)
    {
        return (LIVE_MEDIA_ERROR_OPERATION_IN_PROGRESS);
    }

    /* this should not happen logically */
    if (mFrameLengthOffset > mFrameSize)
    {
        EPRINT(GUI_LIVE_MEDIA, "frame buffer mis-alignment: [streamId=%d], [decId=%d]", mLiveStreamId, decId);
    }

    /* if frame is to be discarded, reset write pointer and don't save header */
    if (true == mDiscardFrame)
    {
        /* reset writer pointer position */
        mFrameBufferWriter = mFrameStartReference;
    }
    else
    {
        /* save header to the list */
        SaveHeadertoBuffer();

        /* save frame start address to frame list */
        mFrameList.append(mFrameStartReference);

        /* if stream type is video and frame type is i-frame then we can clear the extra delayed frames from buffer */
        if ((mFrameHeader.streamType == STREAM_TYPE_VIDEO) && (mFrameHeader.codecType != VIDEO_MJPG))
        {
            /* if it is i-frame then enable the clear delay frame logic */
            if (mFrameHeader.frameType == I_FRAME)
            {
                /* get latest live view type from app controller */
                LIVE_VIEW_TYPE_e liveViewType = ApplController::getInstance()->GetLiveViewType();
                if (liveViewType != mLiveViewType)
                {
                    DPRINT(GUI_LIVE_MEDIA, "live view type changed: [streamId=%d], [decId=%d], [liveViewType=%d --> %d], [frameCnt=%d]",
                           mLiveStreamId, decId, mLiveViewType, liveViewType, GetHeaderFrameList());
                    mLiveViewType = liveViewType;
                }

                mIsNewIframeReceived = true;
                mFrameReceiveCnt = 0;
            }

            /* Increament frame counter on each video frame */
            if (true == mIsNewIframeReceived)
            {
                mFrameReceiveCnt++;
            }
        }
    }

//    DPRINT(GUI_LIVE_MEDIA, "frame saved: [size=%llu], [received=%llu]", mFrameSize, mFrameLengthOffset);

    /* change fsm state to receive next header */
    mLiveMediaFsmState = LIVE_MEDIA_FSM_STATE_GET_VIDEO_HEADER;

    return (LIVE_MEDIA_NO_ERROR);
}

/**
 * @brief LiveMedia::ProcessDecoderFsm
 * @return
 */
LiveMediaError_e LiveMedia::ProcessDecoderFsm(void)
{
    LiveMediaError_e ret = LIVE_MEDIA_NO_ERROR;

    switch (mDecoderFsmState)
    {

    /* wait for some frame buffering */
    case DECODER_FSM_STATE_BUFFERING:
        ret = Decoder_WaitForBuffering();
        break;

    /* feed frames to decoder */
    case DECODER_FSM_STATE_PLAYING:
        ret = Decoder_FeedFrames();
        break;

    default:
        break;

    }

    return (ret);
}

/**
 * @brief LiveMedia::Decoder_WaitForBuffering
 * @return
 */
LiveMediaError_e LiveMedia::Decoder_WaitForBuffering(void)
{
    /* check total frames. minimum two video frames are required for smooth operation */
    if (GetHeaderFrameList(STREAM_TYPE_VIDEO) < 2)
    {
        return (LIVE_MEDIA_ERROR_OPERATION_IN_PROGRESS);
    }

    /* tune VIDEO_BUFFERING_TIME_MS for initial buffering of video */
    if (BufferedFrameTimeDiffTotal() < VIDEO_BUFFERING_TIME_MS)
    {
        return (LIVE_MEDIA_ERROR_OPERATION_IN_PROGRESS);
    }

    //DPRINT(GUI_LIVE_MEDIA, "play stream: buffering complete: [streamId=%d], [decId=%d]", mLiveStreamId, decId);

    /* start playing */
    mDecoderFsmState = DECODER_FSM_STATE_PLAYING;

    return (LIVE_MEDIA_NO_ERROR);
}

/**
 * @brief LiveMedia::Decoder_FeedFrames
 * @return
 */
LiveMediaError_e LiveMedia::Decoder_FeedFrames(void)
{
    QByteArray framePayload;
    FRAME_HEADER_t *pHeader = nullptr;

    /* if we don't have frames to play then wait for the frames */
    if (mHeaderList.isEmpty())
    {
        //WPRINT(GUI_LIVE_MEDIA, "pause stream: buffering: [streamId=%d], [decId=%d]", mLiveStreamId, decId);

        /* start buffering as we don't have enough data */
        mDecoderFsmState = DECODER_FSM_STATE_BUFFERING;

        /* reset last frame timestamp reference */
        mLastPlayedFrameTimestampMs = 0;
        mDecoderFsmTimeReference = 0;
        mConsecutiveFramesTimeDiff = 0;

        return (LIVE_MEDIA_ERROR_OPERATION_IN_PROGRESS);
    }

    /* save this time reference to derive tcp socket timeout when data is not available */
    mDecoderFsmTimeReference = GetMonotonicTimeInMiliSec();

    /* check if time arrived to feed frame */
    if ((mDecoderFsmTimeReference - mLastPlayedFrameTimestampMs) < mConsecutiveFramesTimeDiff)
    {
        return (LIVE_MEDIA_ERROR_OPERATION_IN_PROGRESS);
    }

    if (0 == mLastPlayedFrameTimestampMs)
    {
        /* save timestamp of when frame is played */
        mLastPlayedFrameTimestampMs = mDecoderFsmTimeReference;
    }
    else
    {
        /* save timestamp of when frame is played */
        mLastPlayedFrameTimestampMs += mConsecutiveFramesTimeDiff;
    }

    /* minimum new frame required in buffer since last i-frame received before flushing old frames */
    if ((true == mIsNewIframeReceived) && (mFrameReceiveCnt >= 6))
    {
        /* get the number of video frames in buffer */
        quint8 frameInBuff = GetHeaderFrameList(STREAM_TYPE_VIDEO);
        bool flushFrame = false;

        if (mLiveViewType == LIVE_VIEW_TYPE_REAL_TIME)
        {
            /* maximum old frame allow in buffer since last i-frame received */
            if (frameInBuff > mFrameReceiveCnt)
            {
                /* flush old frame from buffer */
                flushFrame = true;
            }

            /* disable frame flush logic till next i-frame reception */
            mIsNewIframeReceived = false;
        }
        else
        {
            /* maximum old frames flush count from buffer after last i-frame received */
            if (frameInBuff == (mFrameReceiveCnt + 2))
            {
                /* flush old frame from buffer */
                flushFrame = true;

                /* disable frame flush logic till next i-frame reception */
                mIsNewIframeReceived = false;
            }
        }

        /* more frames available to be played even after new i-frame and few p-frames received then clear all older delayed frames */
        if (true == flushFrame)
        {
            quint32 frameCnt;

            /* Get actual frame count from list */
            frameInBuff = GetHeaderFrameList();
            for (frameCnt = 0; frameCnt < frameInBuff; frameCnt++)
            {
                /* get first frame header and check frame type. clear frames till i-frame found */
                pHeader = (FRAME_HEADER_t *)mHeaderList.at(0);
                if ((pHeader->streamType == STREAM_TYPE_VIDEO) && (pHeader->frameType == I_FRAME))
                {
                    /* now we will play from this i-frame */
                    break;
                }

                /* clear frame (header + payload) from buffer */
                if (false == ClearFrameFromBuffer())
                {
                    statusId = CMD_PROCESS_ERROR;
                    EPRINT(GUI_LIVE_MEDIA, "fail to flush frame: [streamId=%d], [decId=%d]", mLiveStreamId, decId);
                    return (LIVE_MEDIA_ERROR_OPERATION_FAILED);
                }
            }

//            WPRINT(GUI_LIVE_MEDIA, "frame(s) flushed from buffer: [streamId=%d], [decId=%d], [frameInBuff=%d], [flushFrameCnt=%d], [viewType=%d]",
//                   mLiveStreamId, decId, frameInBuff, frameCnt, mLiveViewType);
        }
    }

    /* save time to wait before playing next frame */
    mConsecutiveFramesTimeDiff = BufferedFrameTimeDiffConsecutive();

    /* pick up a header & frame reference from buffer */
    if (false == ReadSingleHeaderFromBuffer(&pHeader))
    {
        statusId = CMD_PROCESS_ERROR;
        EPRINT(GUI_LIVE_MEDIA, "fail to process frame header: [streamId=%d], [decId=%d]", mLiveStreamId, decId);
        return (LIVE_MEDIA_ERROR_OPERATION_FAILED);
    }

    /* get single frame payload from buffer */
    if (false == ReadSingleFrameFromBuffer(false, framePayload, pHeader->frameSize - sizeof(FRAME_HEADER_t)))
    {
        statusId = CMD_PROCESS_ERROR;
        EPRINT(GUI_LIVE_MEDIA, "fail to process frame: [streamId=%d], [decId=%d]", mLiveStreamId, decId);
        return (LIVE_MEDIA_ERROR_OPERATION_FAILED);
    }

    /* prepare frame info structure for decoder */
    mFrameInfo.framePayload = framePayload.data();
    mFrameInfo.frameSize = (pHeader->frameSize - sizeof(FRAME_HEADER_t));
    mFrameInfo.mediaType = (STREAM_TYPE_e)pHeader->streamType;
    mFrameInfo.codecType = (STREAM_CODEC_TYPE_e)pHeader->codecType;
    mFrameInfo.frameType = (FRAME_TYPE_e)pHeader->frameType;
    mFrameInfo.frameTimeSec = pHeader->seconds;
    mFrameInfo.frameTimeMSec = pHeader->mSec;
    mFrameInfo.noOfReferanceFrame = pHeader->noOfRefFrame;

    /* After Header version 2 frameRate is actual configured FPS of camera stream parameter */
    if ((pHeader->version >= FRAME_VERSION) && (pHeader->frameRate != 0))
    {
        mFrameInfo.frameRate = pHeader->frameRate;
    }
    else
    {
        mFrameInfo.frameRate = DEFAULT_FRAME_RATE;
    }

    /* get frame resolution */
    if (false == GetVideoResolution(pHeader, mFrameInfo.framePayload, mFrameInfo.frameWidth, mFrameInfo.frameHeight))
    {
        EPRINT(GUI_LIVE_MEDIA, "fail to get frame resolution: [streamId=%d], [decId=%d]", mLiveStreamId, decId);

        statusId = CMD_PROCESS_ERROR;

        return (LIVE_MEDIA_ERROR_INVALID_DATA);
    }

    #if DECODER_PERFORMANCE_DEBUG
    if (TRACK_DECODER_ID(decId))
    {
        DPRINT(GUI_LIVE_MEDIA, "frame play: [streamId=%d], [decId=%d], [frameCount=%d], [frameDiff=%llu ms]", mLiveStreamId, decId, mFrameList.count(), mConsecutiveFramesTimeDiff);
    }
    #endif

    DECODER_ERROR_e decError = MAX_DEC_ERROR;

    /* feed frame to decoder */
    if (false == DecodeDispFrame(decId, &mFrameInfo, &decError))
    {
        if(DEC_ERROR_NO_CAPACITY == decError)
        {
            statusId = CMD_DECODER_CAPACITY_ERROR;
        }
        else
        {
            statusId = CMD_DECODER_ERROR;
        }
        EPRINT(GUI_LIVE_MEDIA, "decoder error: [streamId=%d], [decId=%d], [decError=%d]", mLiveStreamId, decId, decError);
        return (LIVE_MEDIA_ERROR_DECODER);
    }

    /* In normal case if video is started successfully, send success response back to server */
    if (true == mServerSuccessRespPending)
    {
        /* emit signal */
        statusId = CMD_SUCCESS;
        emit sigMediaResponse(request.requestId, cmdId, statusId, request.payload);

        /* response should be sent only once when stream is starting */
        mServerSuccessRespPending = false;
    }

    return (LIVE_MEDIA_NO_ERROR);
}

/**
 * @brief LiveMedia::GetVideoResolution
 * @return
 */
bool LiveMedia::GetVideoResolution(FRAME_HEADER_t *header, char *frameData, quint16 &frameWidth, quint16 &frameHeight)
{
    /* verify audio parameters */
    if (header->streamType != STREAM_TYPE_VIDEO)
    {
        return (true);
    }

    /* video frame resolution */
    if (I_FRAME == (FRAME_TYPE_e)header->frameType)
    {
        /* parse frame header to extract frame resolution if actual resolution not provided */
        if (header->resolution == 0)
        {
            if (false == (VideoStreamParser::getHeightWidth(frameData, header, frameWidth, frameHeight)))
            {
                return (false);
            }

            /* validate resloution */
            if ((frameWidth == 0) || (frameHeight == 0))
            {
                return (false);
            }
        }
        /* if reslution is provided get width and height */
        else
        {
            if (header->resolution >= MAX_RESOLUTION)
            {
                EPRINT(GUI_LIVE_MEDIA, "invalid video frame resolution: [streamId=%d], [decId=%d], [resolution=%dx%d]",
                       mLiveStreamId, decId, frameWidth, frameHeight);

                return (false);
            }

            frameWidth = frameResolution[header->resolution][MediaRequest::FRAME_WIDTH];
            frameHeight = frameResolution[header->resolution][MediaRequest::FRAME_HEIGHT];
        }
    }

    return (true);
}

/**
 * @brief LiveMedia::BufferedFrameTimeDiffTotal
 * @return
 */
quint64 LiveMedia::BufferedFrameTimeDiffTotal(void)
{
    quint64 firstFrameTimeMs = 0, lastFrameTimeMs = 0;
    FRAME_HEADER_t *pHeader = nullptr;

    if (GetHeaderFrameList() < 2)
    {
        return (0);
    }

    /* get first frame timestamp */
    pHeader = (FRAME_HEADER_t *)mHeaderList.first();
    firstFrameTimeMs = (pHeader->seconds * 1000) + (quint64)(pHeader->mSec);

    /* get last frame timestamp */
    pHeader = (FRAME_HEADER_t *)mHeaderList.last();
    lastFrameTimeMs = (pHeader->seconds * 1000) + (quint64)(pHeader->mSec);

    /* this should not happen */
    if (lastFrameTimeMs < firstFrameTimeMs)
    {
        return (0);
    }

    return (lastFrameTimeMs - firstFrameTimeMs);
}

/**
 * @brief LiveMedia::BufferedFrameTimeDiffConsecutive
 * @return
 */
quint64 LiveMedia::BufferedFrameTimeDiffConsecutive(void)
{
    FRAME_HEADER_t *pHeader;
    quint64 firstFrameTimeMs, secondFrameTimeMs;

    quint32 frameCnt = GetHeaderFrameList();
    if (frameCnt < 2)
    {
        return (0);
    }

    /* get second frame timestamp */
    pHeader = (FRAME_HEADER_t *)mHeaderList.at(1);
    secondFrameTimeMs = (pHeader->seconds * 1000) + (quint64)(pHeader->mSec);

    /* get first frame timestamp */
    pHeader = (FRAME_HEADER_t *)mHeaderList.at(0);
    firstFrameTimeMs = (pHeader->seconds * 1000) + (quint64)(pHeader->mSec);

    /* in case latest frame time is lower than older frame then play next frame immediately */
    if (secondFrameTimeMs < firstFrameTimeMs)
    {
        return (0);
    }

    /* if frame difference is 0 or more than 1 sec then play next frame immediately */
    quint32 frameDiff = (secondFrameTimeMs - firstFrameTimeMs);
    if ((frameDiff == 0) || (frameDiff > 1000))
    {
        return (0);
    }

    /* Do not alter frame time difference if diff is less than 300ms and less frame available in buffer */
    if ((frameCnt <= 2) || (frameDiff < 300))
    {
        return (frameDiff);
    }

    /* if frame rate available in frame header then calculate frame time and compare with frame difference */
    if ((pHeader->streamType == STREAM_TYPE_VIDEO) && (pHeader->version >= FRAME_VERSION) && (pHeader->frameRate != 0))
    {
        /* get ideal frame difference based on configured fps */
        quint32 fpsDiff = qCeil((float)1000/pHeader->frameRate);

        /* if frame difference is more 300ms then cap to fps difference.  */
        if (fpsDiff < 300)
        {
            WPRINT(GUI_LIVE_MEDIA, "high frame diff found: [streamId=%d], [decId=%d], [frameCnt=%d], [fpsDiff=%d], [frameDiff=%d], [viewType=%d]",
                   mLiveStreamId, decId, frameCnt, fpsDiff, frameDiff, mLiveViewType);
            frameDiff = fpsDiff;
        }
    }

    /* provide frame difference */
    return (frameDiff);
}

/**
 * @brief LiveMedia::GetHeaderFrameList
 * @param streamType
 * @return
 */
quint8 LiveMedia::GetHeaderFrameList(STREAM_TYPE_e streamType)
{
    quint8 frameInBuff = mHeaderList.count();

    /* match actual frame count with derived frame count. Ideally it should matched */
    if (frameInBuff != (mVideoFrameCnt + mAudioFrameCnt))
    {
        WPRINT(GUI_LIVE_MEDIA, "frame header count mismatched: [streamId=%d], [decId=%d], [frameInBuff=%d], [frameCnt=%d]",
               mLiveStreamId, decId, frameInBuff, (mVideoFrameCnt + mAudioFrameCnt));
    }

    /* provide frame count as per stream type */
    if (streamType == STREAM_TYPE_VIDEO)
    {
        return mVideoFrameCnt;
    }
    else if (streamType == STREAM_TYPE_AUDIO)
    {
        return mAudioFrameCnt;
    }
    else
    {
        return frameInBuff;
    }
}

/**
 * @brief LiveMedia::SaveHeadertoBuffer
 */
void LiveMedia::SaveHeadertoBuffer(void)
{
    quint8 locationOffset = 0;

    /* write to bufer */
    memcpy((mHeaderBufferBegin + mHeaderBufferWriter), &mFrameHeader, sizeof(FRAME_HEADER_t));

    /* save structure address to header list */
    mHeaderList.append(mHeaderBufferBegin + mHeaderBufferWriter);

    locationOffset = mHeaderBufferWriter;

    /* circular array of structure */
    if (++locationOffset >= VIDEO_BUFFER_NUM_FRAMES_LIMIT)
    {
        locationOffset = 0;
    }

    /* update writer pointer position */
    mHeaderBufferWriter = locationOffset;

    /* update frame count as per stream type */
    if (mFrameHeader.streamType == STREAM_TYPE_VIDEO)
    {
        mVideoFrameCnt++;
    }
    else
    {
        mAudioFrameCnt++;
    }
}

/**
 * @brief LiveMedia::IsHeaderBufferFull
 * @return
 */
bool LiveMedia::IsHeaderBufferFull(void)
{
    quint8 locationOffset = mHeaderBufferWriter;

    if (++locationOffset >= VIDEO_BUFFER_NUM_FRAMES_LIMIT)
    {
        locationOffset = 0;
    }

    /* reader & writer are on same location */
    if (locationOffset == mHeaderBufferReader)
    {
        return (true);
    }

    return (false);
}

/**
 * @brief LiveMedia::IsHeaderBufferEmpty
 * @return
 */
bool LiveMedia::IsHeaderBufferEmpty(void)
{
    /* reader & writer are on same location */
    if (mHeaderBufferReader == mHeaderBufferWriter)
    {
        return (true);
    }

    return (false);
}

/**
 * @brief LiveMedia::IsFrameBufferFull
 * @return
 */
bool LiveMedia::IsFrameBufferFull(void)
{
    char *locationOffset = mFrameBufferWriter;

    if (++locationOffset >= (mFrameBufferBegin + VIDEO_BUFFER_TOTAL_SIZE_LIMIT))
    {
        locationOffset = mFrameBufferBegin;
    }

    /* reader & writer are on same location */
    if (locationOffset == mFrameBufferReader)
    {
        return (true);
    }

    return (false);
}

/**
 * @brief LiveMedia::IsFrameBufferEmpty
 * @return
 */
bool LiveMedia::IsFrameBufferEmpty(void)
{
    /* reader & writer are on same location */
    if (mFrameBufferWriter == mFrameBufferReader)
    {
        return (true);
    }

    return (false);
}

/**
 * @brief LiveMedia::GetFrameBufferOccupiedSize
 * @return
 */
quint64 LiveMedia::GetFrameBufferOccupiedSize(void)
{
    if (mFrameBufferWriter >= mFrameBufferReader)
    {
        return ((quint64)mFrameBufferWriter - (quint64)mFrameBufferReader);
    }

    return ((quint64)VIDEO_BUFFER_TOTAL_SIZE_LIMIT - (mFrameBufferReader - mFrameBufferWriter));
}

/**
 * @brief LiveMedia::GetFrameBufferFreeSpace
 * @return
 */
quint64 LiveMedia::GetFrameBufferFreeSpace(void)
{
    return (VIDEO_BUFFER_TOTAL_SIZE_LIMIT - GetFrameBufferOccupiedSize());
}

/**
 * @brief LiveMedia::ClearFrameFromBuffer
 * @return
 */
bool LiveMedia::ClearFrameFromBuffer(void)
{
    FRAME_HEADER_t  *pHeader = nullptr;
    QByteArray      payload;

    /* remove first frame from  buffer */
    if (false == ReadSingleHeaderFromBuffer(&pHeader))
    {
        return (false);
    }

    /* remove first frame payload from buffer */
    if (false == ReadSingleFrameFromBuffer(true, payload, pHeader->frameSize - sizeof(FRAME_HEADER_t)))
    {
        return (false);
    }

    return (true);
}

/**
 * @brief LiveMedia::ReadSingleHeaderFromBuffer
 * @param pHeader
 * @return
 */
bool LiveMedia::ReadSingleHeaderFromBuffer(FRAME_HEADER_t **pHeader)
{
    quint8 headerOffset = 0;

    if (true == mHeaderList.isEmpty())
    {
        return (false);
    }

    /* get address of first header */
    *pHeader = (mHeaderBufferBegin + mHeaderBufferReader);

    headerOffset = mHeaderBufferReader;

    /* check for circular buffer */
    if (++headerOffset >= VIDEO_BUFFER_NUM_FRAMES_LIMIT)
    {
        headerOffset = 0;
    }

    /* update reader pointer */
    mHeaderBufferReader = headerOffset;

    /* remove header from list */
    mHeaderList.removeFirst();

    /* update frame count as per stream type */
    if ((*pHeader)->streamType == STREAM_TYPE_VIDEO)
    {
        if (mVideoFrameCnt)
        {
            mVideoFrameCnt--;
        }
    }
    else
    {
        if (mAudioFrameCnt)
        {
            mAudioFrameCnt--;
        }
    }

    return (true);
}

/**
 * @brief LiveMedia::ReadSingleFrameFromBuffer
 * @param flushFrame
 * @param frame
 * @param frameLen
 */
bool LiveMedia::ReadSingleFrameFromBuffer(bool flushFrame, QByteArray &frame, quint64 frameLen)
{
    if (mFrameList.isEmpty())
    {
        return (false);
    }

    /*
     * take care when circular buffer when half payload of
     * frame is at end and half at the starting of buffer
     */
    if ((mFrameBufferReader + frameLen) > (mFrameBufferBegin + VIDEO_BUFFER_TOTAL_SIZE_LIMIT))
    {
        /* get length of first part of frame from the end of frame buffer */
        quint64 postLen = ((quint64)mFrameBufferBegin + VIDEO_BUFFER_TOTAL_SIZE_LIMIT - (quint64)mFrameBufferReader);

        /* get length of remaining frame from start of the buffer */
        quint64 preLen = frameLen - postLen;

        /* do not store address in header if frame flush needed from buffer */
        if (false == flushFrame)
        {
            /* read first part of frame from the end of frame buffer */
            frame.append(mFrameBufferReader, postLen);

            /* read remaining frame from start of the buffer */
            frame.append(mFrameBufferBegin, preLen);
        }

        /* update reader pointer */
        mFrameBufferReader = mFrameBufferBegin + preLen;
    }
    else
    {
        /* do not store address in header if frame flush needed from buffer */
        if (false == flushFrame)
        {
            /* full frame is in conitunous memory */
            frame.append(mFrameBufferReader, frameLen);
        }

        /* update reader pointer */
        mFrameBufferReader += frameLen;
    }

    /* remove frame reference from list */
    mFrameList.removeFirst();

    return (true);
}

/**
 * @brief LiveMedia::GetSocketTimeoutWaitTimeMs
 * @return
 */
quint16 LiveMedia::GetSocketTimeoutWaitTimeMs(void)
{
    qint64 timeout = 0;

    /* set tcp socket wait according to time difference between two frames & time taken by decoder to play last frame */

    /* if frame play is not started at all, wait for default timeout */
    if ((0 == mDecoderFsmTimeReference) || (0 == mConsecutiveFramesTimeDiff))
    {
        timeout = SOCKET_WAIT_DATA_TIMEOUT_MS;
    }
    else
    {
        /* if decoder has already spent lot of time (e.g. more than frame time difference) then don't wait on TCP socket */
        timeout = (mLastPlayedFrameTimestampMs + mConsecutiveFramesTimeDiff) - (mDecoderFsmTimeReference + mDecoderExecTimeMs);

        if (timeout < 0)
        {
            #if DECODER_PERFORMANCE_DEBUG
            if (TRACK_DECODER_ID(decId))
            {
                DPRINT(GUI_LIVE_MEDIA, "running late: [streamId=%d], [decId=%d], [time=%lld ms]", mLiveStreamId, decId, -timeout);
            }
            #endif

            timeout = 0;
        }
    }

    return ((quint16)timeout);
}

/**
 * @brief LiveMedia::ClearAllFrames
 */
void LiveMedia::ClearAllFrames(void)
{
    mHeaderBufferReader = 0;
    mHeaderBufferWriter = 0;

    mFrameBufferReader = mFrameBufferBegin;
    mFrameBufferWriter = mFrameBufferBegin;

    mHeaderList.clear();
    mFrameList.clear();

    mConsecutiveFramesTimeDiff = 0;
    mLastPlayedFrameTimestampMs = 0;

    /* we need to wait for next I-frame after clearing all frames */
    setIsIframeRecive(false);
    mIsNewIframeReceived = false;
    mFrameReceiveCnt = 0;
    mVideoFrameCnt = 0;
    mAudioFrameCnt = 0;
}

/**
 * @brief LiveMedia::GetMonotonicTimeInMiliSec
 * @return
 */
quint64 LiveMedia::GetMonotonicTimeInMiliSec(void)
{
   struct timespec ts;

   /* get time in form of sec & nanosec */
   clock_gettime(CLOCK_MONOTONIC, &ts);

   return (quint64) ((((quint64)ts.tv_sec) * 1000) + (ts.tv_nsec / 1000000LL));
}

/**
 * @brief LiveMedia::setIsIframeRecive
 * @param truefalse
 */
void LiveMedia::setIsIframeRecive(bool truefalse)
{
    mIframeLock.lock();
    mIsFirstIframeReceived = truefalse;
    mIframeLock.unlock();
}

/**
 * @brief LiveMedia::getIsIframeRecive
 * @return
 */
bool LiveMedia::getIsIframeRecive(void)
{
    mIframeLock.lock();
    bool value = mIsFirstIframeReceived;
    mIframeLock.unlock();

    return (value);
}

/***********************************************************************************************
* @END OF FILE
***********************************************************************************************/
