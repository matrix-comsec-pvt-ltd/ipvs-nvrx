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
 * @file         SyncPbMedia.h
 * @brief        This module provides APIs for Synchronous playback media streaming.
 *               It has a thread function which process thre requested command from que.
 *               Also provide api to device thread to put request in que.
 *               If any request in que, then process it, otherwise it will go in condition wait.
 */

#ifndef SYNCPBMEDIA_H
#define SYNCPBMEDIA_H

/***********************************************************************************************
* @INCLUDES
***********************************************************************************************/
#include "SyncPbFrameRecv.h"

/***********************************************************************************************
* @DEFINES
***********************************************************************************************/
#define MAX_SYNC_PB_MSGQUE_LENGTH           25

/***********************************************************************************************
* @TYPEDEF
***********************************************************************************************/
typedef enum
{
    PLAY_STATE,
    PAUSE_STATE,
    PAUSE_BY_BUFFER_STATE,
    STEP_STATE,
    STOP_STATE,
    RECV_STATE,

    MAX_PB_STATE
}PB_STATE_e;

/***********************************************************************************************
* @CLASSES
***********************************************************************************************/
class SyncPbMedia : public QThread
{
    Q_OBJECT

public:
    // This API is constructor of class PlaybackMedia
    SyncPbMedia(SERVER_INFO_t serverInfo,
                REQ_INFO_t &requestInfo,
                SET_COMMAND_e commandId,
                quint8 decoderId,
                const quint8 *pDecIdSet,
                const quint8 *pChnlIdSet);

    // This API is destructor of class PlaybackMedia
    ~SyncPbMedia();

    // run function, which is executed in system created thread.
    // it requests for playback media, receives frame and stores it in buffer.
    void run(void);

    // this api put command request in que of media thread
    void setActionOnControlCmd(SET_COMMAND_e commandId,
                               QString payloadStr);


    // this api returns decoder Id for Channel Id.
    void GetDecIdForCh(quint8 chnlId, quint8 &decoderId, quint8 &indx);
    void setRunFlag(bool flag);
    // this api send signal to thread which is in condition wait
    void sigToRecvPauseFalse(void);

    void updateDecInstances(const quint8 *idSet);

protected:
    // run flag, which breaks the media loop if set to false
    bool runFlag;

    // response timeout [used for control command]
    quint8 timeout;

	quint8 msgIdQue[MAX_SYNC_PB_MSGQUE_LENGTH];
    quint8 msgQueIndex;

    // server information, like ip address and tcp port
    SERVER_INFO_t srvrInfo;
    // active session id of NVR [used for control command]
    QString sessionId;

    // read write access lock for run flag
    QReadWriteLock runFlagLock;

    // playback state
    PB_STATE_e pbState;

    // message que related data, in which index, command & paylod included
    QMutex msgQueLock;

	QString msgQuePayload[MAX_SYNC_PB_MSGQUE_LENGTH];

    // condition wait mutex
    bool m_isRecvPauseSigSend;
    QMutex recvPauseSigLock;
    QWaitCondition recvPauseSig;

    // control command pointer [used for all sync command]
    CommandRequest *syncCommand;
    // frame receiver object pointer
    SyncPbFrameRecv *syncPbFrameRecv;

    bool createSyncFrameRecv(SERVER_INFO_t serverInfo,
                             REQ_INFO_t &requestInfo,
                             SET_COMMAND_e commandId,
                             quint8 decoderId,
                             const quint8 *pDecIdSet,
                             const quint8 *pChnlIdSet);

    bool deleteSyncFrameRecv(void);

    bool createSyncCommandReq(SET_COMMAND_e commandId,
                              QString reqInfoPayload);

    bool deleteSyncCommandReq(void);

private:

signals:
    // signal to send media info to streamReq
    void sigSyncPbResponse (REQ_MSG_ID_e requestId,
                            SET_COMMAND_e commandId,
                            DEVICE_REPLY_TYPE_e statusId,
                            QString payload);

public slots:
    // slot to catch buffer threshold cross signal
    void slotPbFrameRecvThreshold (BUFFER_THRESHOLD_e threshold);

    // slot to catch frameReceive response
    void slotFrameRecvResponse (REQ_MSG_ID_e requestId,
                                SET_COMMAND_e commandId,
                                DEVICE_REPLY_TYPE_e statusId,
                                QString payload);


};
#endif // SYNCPBMEDIA_H
