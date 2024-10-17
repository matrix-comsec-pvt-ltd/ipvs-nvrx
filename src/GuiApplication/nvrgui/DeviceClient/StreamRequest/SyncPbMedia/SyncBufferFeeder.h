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
 * @file         SyncBufferFeeder.h
 * @brief        This module provides buffer to store media frames and their headers.
 *               It has APIs to write frame into and read frame from the buffer. It also notifies a
 *               signal when buffer size crosses minimum and maximum threshold. It provides a feeder
 *               thread which is useful to feed frames to decoder module, maintaining time delay between consecutive frames.
 */

#ifndef SYNCBUFFERFEEDER_H
#define SYNCBUFFERFEEDER_H

/***********************************************************************************************
* @INCLUDES
***********************************************************************************************/
#include <QObject>
#include <QByteArray>
#include <QThread>
#include <QReadWriteLock>
#include <QWaitCondition>
#include <QMutex>
#include "EnumFile.h"
#include "DeviceDefine.h"
#include "DeviceClient/StreamRequest/FrameHeader.h"
#include "../VideoStreamParser.h"

/***********************************************************************************************
* @DEFINES
***********************************************************************************************/

/* Stack size of buffer feeder thread */
#define SYNC_PB_DECODER_THREAD_STACK_SIZE   (1 * ONE_MEGABYTE)

/* Sync playback common buffer size
 * To store 2 seconds of data for 16 camera with 16 Mbps bitrate
 * (2 sec.) * (16 camera) * (16Mbps or 2 MB) = 64 MB
 * But usually higher bitrate like 16Mbps used with higher resolution like 5MP/8MP
 * And due to decoder limit we can not play 16 cameras of 5MP.
 * So keeping buufer size to 48MB.
 */
#define TOTAL_BUFFER_SIZE                   (48 * ONE_MEGABYTE)

/***********************************************************************************************
* @TYPEDEF
***********************************************************************************************/
typedef enum
{
    STEP_STATE_FALSE,
    STEP_STATE_TRUE,
    MAX_STEP_STATE
}STEP_STATE_e;

/***********************************************************************************************
* @CLASSES
***********************************************************************************************/
class SyncBufferFeeder : public QThread
{
    Q_OBJECT

public:

    SyncBufferFeeder(quint8 decoderId, quint8 sesId);
    ~SyncBufferFeeder(void);

    /* feeder thread which feeds frame to decoder module, maintaining delay between consecutive frames */
    void run(void);

    void SetThreadName(void);
    void setRunFlag(bool flag);
    bool getRunFlag(void);

    void SetPbSpeedBufFeeder(PB_SPEED_e speed, quint8 dir);
    void SendStepSig(void);
    bool readNextFrameTime(quint64 &nextFrameTime, quint8 &nextFrameStatus);
    quint64 GetNextFrameDelay(void);
    void setFeedFrameFlag(bool flag);
    bool getFeedFrameFlag(void);
    void resetBufData(void);
    void clearBufData(void);
    void setStepFlag(STEP_STATE_e flagState);
	bool getStepFlag(void);
    void storeLocationBuf(char *location);
    static void setMostReadPos(char *setPos);
    static void getMostReadPos(char **readPos);
    static void resetMostReadPos(void);
    static void setPlaybackReferenceTime(quint64);
    static quint64 getPlaybackReferenceTime(void);
    quint64 GetRequiredDecodingCapacity(PB_SPEED_e speed, quint8 playbackDirection);
    bool GetVideoResolution(FRAME_HEADER_t *header, char *frameData, UINT16 &frameWidth, UINT16 &frameHeight);

protected:

    bool isFirstIFrameReceived;
    bool isForward;
    STEP_STATE_e stepFlag;
    bool runFlag;
    bool feedFrame;
    quint8 decId;
    quint8 bufFeedId;
    quint64 currFrameTime;
	bool isFirstFramePlayed;
	bool restartWithIFrame;

    QMutex feedFrameLock;
    QMutex stepSigLock;
    QWaitCondition stepSig;

    /* buffer to store frame location in buffer */
    QList<char *> locationBuff;
    QReadWriteLock bufferRdWr;

    /* read write access lock for feeder run flag */
    QReadWriteLock runFlagLock;

    /* speed to maintain delay */
    PB_SPEED_e pbSpeed;
    quint32 syncPbSpeed;
    quint8 direction;

    static quint64 playbackReferenceTime;
    static QMutex  referenceTimeMutexLock;

    /* static read position, which updated by all object of feeder */
    static char *bufStartPos;
    static char *mostReadPos;
    static QMutex mostReadPosAccess;

    /* read function to retrieve frame from buffer */
    bool readFrame(char **header, quint64 &frameDelay);
    void getFrameRateForPlaySpeed(UINT16 &fps);

private:
     quint16 fCurrFPS;
     quint16 fheight;
     quint16 fwidth;

signals:

    /* signal to convey the threshold command response */
    void sigFeederResponse(DEVICE_REPLY_TYPE_e statusId, quint32 payload, quint8 feederId);
    void sigSetLowestTime(quint8 feederId, bool feedFrame, quint64 frameTime);
};

#endif // SYNCBUFFERFEEDER_H
/***********************************************************************************************
* @END OF FILE
***********************************************************************************************/
