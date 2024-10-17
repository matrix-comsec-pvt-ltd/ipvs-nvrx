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
 * @file        SyncPbFrameRecv.h
 * @brief       This module provides APIs for Synchronous playback media streaming. It has a
 *              thread function which receive frame continuosly and store frame in buffer according to received channel id.
 *              It also provide api related to bufferFeeder thread control and parameter changes.
 */

#ifndef SYNCPBFRAMERECV_H
#define SYNCPBFRAMERECV_H

/***********************************************************************************************
* @INCLUDES
***********************************************************************************************/
#include <QObject>
#include <QString>
#include <QStringList>

#include "../MediaRequest.h"
#include "SyncBufferFeeder.h"
#include "EnumFile.h"

/***********************************************************************************************
* @DEFINES
***********************************************************************************************/
#define SYNC_PB_FRAME_RECV_THREAD_STACK_SIZE (1 * ONE_MEGABYTE)

/***********************************************************************************************
* @CLASSES
***********************************************************************************************/
class SyncPbFrameRecv : public MediaRequest
{
    Q_OBJECT
public:
    // This API is constructor of class SyncPbFrameRecv
    SyncPbFrameRecv(SERVER_INFO_t serverInfo, REQ_INFO_t &requestInfo, SET_COMMAND_e commandId,
                    quint8 decoderId, const quint8 *pDecIdSet);

    // This API is destructor of class SyncPbFrameRecv
    ~SyncPbFrameRecv(void);

    // run function, which is executed in system created thread.
    // which receive frame continuosly and store frame in buffer
    // according to received channel id.
    void run(void);

    // this api set speed of all feeder
	void SetPbSpeed(PB_SPEED_e pbkSpeed, quint8 dir);
	PB_SPEED_e getPbSpeed(void);

    // this api set feed frame flag for feeding frame to decoder
    void setFeedFrameFlagToAll(bool flag);

    bool createAllSyncBufFeeder(void);
    bool deleteAllSyncBufFeeder(void);

    // this api reset previous frame time & store current direction
    void resetprevFrameTimeDirection(quint8 direction);
    void getDirection(quint8 &dir);

    // this api set frameStoreFlag , which is decide whether this received frame
    // should be stored or not
    void setFrameStoreFlag(bool flag);
    bool getFrameStoreFlag(void);
    void updateChIdSet(const quint8 *pChnlIdSet);

    // this api give decoder Id for particular channel id.
    void getDecIdForChannel(quint8 chId, quint8 &decodeId, quint8 &indx);
    //This api update decoder Id set after stop
    // becuse by swap dec Id may be changed
    void updateDecoderInstances(const quint8 *decIdSet);
    // this api reset all buffer data
    void resetAllBuffer(void);
    // this api set step flag of all feeder
    void setStepFlagAll(bool flag);
    // this api send condition signal to thread which is in wait in step process
    void sendStepWakeSigToAll(void);

    void setRecvPauseFlag(bool flag);
    bool getRecvPauseFlag(void);
    void setSyncFrameNum(quint8 num);
    void setThrottleFlag(bool flag);
    bool getThrottleFlag(void);

protected:

    bool recvPauseFlag;
    bool frameStoreFlag;

    // store channel id
    quint8 chIdSet[MAX_SYNC_PB_SESSION];
    //max channel to be played in sync pb
    quint8 maxCh;

    quint8 syncFrameNo;
    quint8 direction;

    QMutex syncFrameNoAccess;
    // pointer to feeder object
    SyncBufferFeeder *feeder[MAX_SYNC_PB_SESSION];

    // platyback speed
    PB_SPEED_e pbSpeed;

    QMutex prevTimeDirAccess;

    //previous frame time to compare current frame time
    quint32 prevFrameTime;

    QReadWriteLock recvPauseFlagAccess;
    QReadWriteLock frameStoreFlagAccess;

    char * startBufPtr;
    char * endBufPtr;
    bool throttle;
    QMutex throttleAccess;

    // This API creates buffer to store playback stream
    // and feeder thread to feed frames to decoder
    bool createSyncBufferFeeder (quint8 sesId);
    // This API deletes buffer feeder
    bool deleteSyncBufferFeeder (quint8 sesId);

private:
    quint64         m_currentFrameFeedTime[MAX_SYNC_PB_SESSION];
    QReadWriteLock  m_feedInfoLock;

signals:
    // signal to indicate threshold crossed
    void sigPbFrameRecvThreshold(BUFFER_THRESHOLD_e thresholds);

public slots:

    // slot ot catch any bufferFeeder response
    void slotPbMediaResponse (DEVICE_REPLY_TYPE_e tStatusId, quint32 frameTime, quint8 feederId);
    void slotSetLowestTime(quint8 feederId,bool feedFrameFlag,quint64 frameTime);

};

#endif // SYNCPBFRAMERECV_H
/***********************************************************************************************
* @END OF FILE
***********************************************************************************************/
