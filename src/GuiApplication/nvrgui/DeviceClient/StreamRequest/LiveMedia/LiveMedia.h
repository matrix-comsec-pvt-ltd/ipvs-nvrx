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

/**
 * @file         LiveMedia.h
 * @brief        This class provides interface to receive, buffer and play video using h/w decoder.
 */

#ifndef LIVEMEDIA_H
#define LIVEMEDIA_H

/***********************************************************************************************
* @INCLUDES
***********************************************************************************************/
#include <QObject>
#include <QElapsedTimer>

#include <sys/prctl.h>
#include "../MediaRequest.h"
#include "../VideoStreamParser.h"

/***********************************************************************************************
* @DEFINES
***********************************************************************************************/

/* live media thread stack size */
#define LIVE_MEDIA_THREAD_STACK_SIZE (6 * MEGA_BYTE)

/***********************************************************************************************
* @ENUMS
***********************************************************************************************/
/* live media fsm states */
typedef enum
{
    LIVE_MEDIA_FSM_STATE_IDLE,
    LIVE_MEDIA_FSM_STATE_GET_VIDEO_HEADER,
    LIVE_MEDIA_FSM_STATE_GET_VIDEO_FRAME

}LiveMediaFsmState_e;

/* decoder fsm states */
typedef enum
{
    DECODER_FSM_STATE_IDLE,
    DECODER_FSM_STATE_BUFFERING,
    DECODER_FSM_STATE_PLAYING

}DecoderFsmState_e;

/* live media error codes */
typedef enum
{
    LIVE_MEDIA_NO_ERROR,
    LIVE_MEDIA_ERROR_UNKNOWN,
    LIVE_MEDIA_ERROR_IN_CONNECTION,
    LIVE_MEDIA_ERROR_OPERATION_IN_PROGRESS,
    LIVE_MEDIA_ERROR_OPERATION_FAILED,
    LIVE_MEDIA_ERROR_INVALID_DATA,
    LIVE_MEDIA_ERROR_FRAME_BUFFER_LIMIT,
    LIVE_MEDIA_ERROR_VIDEO_LOSS,
    LIVE_MEDIA_ERROR_DECODER

}LiveMediaError_e;

/***********************************************************************************************
* @CLASSES
***********************************************************************************************/

class LiveMedia : public MediaRequest
{
    Q_OBJECT

private:

    /* server response status */
    bool mServerSuccessRespPending;

    /* stream id */
    quint8 mLiveStreamId;

    /* live media & decoder fsm state */
    LiveMediaFsmState_e mLiveMediaFsmState;
    DecoderFsmState_e mDecoderFsmState;

    /* flag to indicate that first I-frame is received */
    QMutex mIframeLock;
    bool mIsFirstIframeReceived;

    /* flag to indicate that new I-frame is received */
    bool mIsNewIframeReceived;
    quint8 mFrameReceiveCnt;

    /* audio and video frame count in header list */
    quint8 mVideoFrameCnt;
    quint8 mAudioFrameCnt;

    /* frame time difference to wait before feeding next frame to decoder */
    quint64 mLastPlayedFrameTimestampMs;
    quint64 mConsecutiveFramesTimeDiff;

    /* flag to indicate to discard frame due to errors */
    bool mDiscardFrame;

    /* video loss status */
    bool mVideoLossStatus;

    /* flag to indicate frame is being receving */
    bool mIsNewFrame;
    quint64 mFrameLengthOffset;
    quint64 mFrameSize;
    char *mFrameStartReference;

    /* buffer to store header temporary */
    FRAME_HEADER_t mFrameHeader;
    char *mHeaderWritePtr;
    quint64 mHeaderLengthOffset;

    /* pointer to start of header buffer */
    FRAME_HEADER_t *mHeaderBufferBegin;

    /* Reader & writer location of array to manager header buffer */
    quint8 mHeaderBufferReader;
    quint8 mHeaderBufferWriter;

    /* list to store starting address of each header */
    QList<FRAME_HEADER_t *> mHeaderList;

    /* pointer to start & end of frame buffer */
    char *mFrameBufferBegin;

    /* Reader & writer pointer to manager frame buffer */
    char *mFrameBufferReader;
    char *mFrameBufferWriter;

    /* list to store starting address of each frame */
    QList<char *> mFrameList;

    /* frame information */
    FRAME_INFO_t mFrameInfo;

    /* timer used to escape if no data available in socket for predefined time */
    QElapsedTimer mSocketDataTimer;

    /* time in millisec that decoder have used in last fsm call */
    quint64 mDecoderExecTimeMs;

    /* time reference used for deriving socket timeout for no data available */
    quint64 mDecoderFsmTimeReference;

    /* live view type */
    LIVE_VIEW_TYPE_e mLiveViewType;

public:
    /* initializes object with server info, request info command id and windowId */
    LiveMedia(SERVER_INFO_t serverInfo, REQ_INFO_t &requestInfo, SET_COMMAND_e commandId,
              quint8 decoderId, quint8 streamId);

    /* destructor */
    ~LiveMedia();

    /* system thread which requests for live media, receives frame and feeds it to decoder */
    void run(void);

    /* function to set/get I-frame received flag */
    void setIsIframeRecive(bool truefalse);
    bool getIsIframeRecive(void);

private:
    /* set thread name */
    void SetThreadName(void);

    /* initial handshaking with server */
    LiveMediaError_e DoServerHandshake(QTcpSocket &tcpSocket);

    /* receive headers & frames in non-blocking manner */
    LiveMediaError_e ProcessMediaFsm(QTcpSocket &tcpSocket);
    LiveMediaError_e LiveMedia_ProcessHeader(QTcpSocket &tcpSocket);
    LiveMediaError_e LiveMedia_ProcessFrame(QTcpSocket &tcpSocket);

    /* feed frames to decoder */
    LiveMediaError_e ProcessDecoderFsm(void);
    LiveMediaError_e Decoder_WaitForBuffering(void);
    LiveMediaError_e Decoder_FeedFrames(void);

    bool GetVideoResolution(FRAME_HEADER_t *header, char *frameData, quint16 &frameWidth, quint16 &frameHeight);

    quint64 BufferedFrameTimeDiffTotal(void);
    quint64 BufferedFrameTimeDiffConsecutive(void);

    quint8 GetHeaderFrameList(STREAM_TYPE_e streamType = MAX_STREAM_TYPE);
    void SaveHeadertoBuffer(void);

    bool IsHeaderBufferFull(void);
    bool IsHeaderBufferEmpty(void);

    bool IsFrameBufferFull(void);
    bool IsFrameBufferEmpty(void);

    quint64 GetFrameBufferOccupiedSize(void);
    quint64 GetFrameBufferFreeSpace(void);

    bool ReadSingleHeaderFromBuffer(FRAME_HEADER_t **pHeader);
    bool ReadSingleFrameFromBuffer(bool flushFrame, QByteArray &frame, quint64 frameLen);

    quint16 GetSocketTimeoutWaitTimeMs(void);

    bool ClearFrameFromBuffer(void);
    void ClearAllFrames(void);

    quint64 GetMonotonicTimeInMiliSec(void);
};

#endif // LIVEMEDIA_H
/***********************************************************************************************
* @END OF FILE
***********************************************************************************************/

