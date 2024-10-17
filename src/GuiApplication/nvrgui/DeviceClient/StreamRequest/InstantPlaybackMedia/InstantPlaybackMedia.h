#ifndef INSTANTPLAYBACKMEDIA_H
#define INSTANTPLAYBACKMEDIA_H

///////////////////////////////////////////////////////////////////////////
//
//   MMM     MMM       AAA       TTTTTTTTTT  RRRRRR    IIIIIIII  XX   XX
//   MMMM   MMMM      AA AA          TT      RR   RR      II      XX XX
//   MM MM MM MM     AA   AA         TT      RR    RR     II       XXX
//   MM  MM   MM    AAAAAAAAA        TT      RRRRRRR      II       XXX
//   MM       MM   AA       AA       TT      RR  RR       II      XX XX
//   MM       MM  AA         AA      TT      RR   RR   IIIIIIII  XX   XX
//
//   Company      : Matrix Telecom Pvt. Ltd., Baroda, India.
//   Project      : NVR [Network Video Recorder]
//   Owner        : Aekam Parmar
//   File         : PlaybackMedia.h
//   Description  : This module provides APIs for playback media streaming.
//                  It has a thread function which sends request to server
//                  and if the response is success, receives stream and stores
//                  in the buffer.
//
/////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QString>
#include <QReadWriteLock>
#include <QWaitCondition>
#include <QMutex>
#include <QTimer>

#include "InstantPlaybackFrameReceiver.h"

#define MAX_PLAYBACK_SESSION        (MAX_DEC_DISP_CHN_PLAYBACK)
#define MAX_MSGQUE_LENGTH           5

typedef struct
{
    SET_COMMAND_e command;
    QString payload;
}INSTATNT_PLAYBACK_MSG_QUEUE_t;

class InstantPlaybackMedia : public QThread
{
    Q_OBJECT

public:
    InstantPlaybackMedia(SERVER_INFO_t serverInfo,
                         REQ_INFO_t &requestInfo,
                         SET_COMMAND_e commandId,
                         quint8 decoderId);
    ~InstantPlaybackMedia();

    void run();

    void setRunFlag(bool flag);
    void setFrameReceiveFlag(bool flag);
    bool getFrameReceiveFlag();
    void setPlaybackId(QString playbackId);
    bool isValidPlaybackId();
    bool setInstantPlaybackCommand(SET_COMMAND_e commandId,
                                   QString payloadStr);
    void sigToRecvPauseFalse();

    bool getRunFlag();

protected:
    SERVER_INFO_t m_serverInfo;
    QString m_sessionId;
    SET_COMMAND_e m_commandId;
    quint8 m_timeout;

    quint8 m_msgQueueIndex;
    INSTATNT_PLAYBACK_MSG_QUEUE_t m_msgQueue[MAX_MSGQUE_LENGTH];
    QMutex m_msgQueueLock;

    bool m_isRecvPauseSigSend;
    QMutex m_recvPauseSigLock;
    QWaitCondition m_recvPauseSig;

    QString m_playBackId;
    bool m_isValidPlaybackId;
    bool m_runFlag;
    QReadWriteLock m_runFlagLock;

    CommandRequest *m_instantPlaybackCommand;
    InstantPlaybackFrameReceiver *m_frameReceiver;

    bool createFrameReceiver(SERVER_INFO_t serverInfo,
                             REQ_INFO_t &requestInfo,
                             SET_COMMAND_e commandId,
                             quint8 decoderId);
    void deleteFrameReceiver();
    bool createInstantPbCommand(SET_COMMAND_e commandId, QString payload);
    void deleteInstantPbCommand();

signals:
    void sigMediaResponse(REQ_MSG_ID_e requestId,
                          SET_COMMAND_e commandId,
                          DEVICE_REPLY_TYPE_e statusId,
                          QString payload);

public slots:
    void slotBufferThreshold(BUFFER_THRESHOLD_e threshold);
    void slotMediaResponse(REQ_MSG_ID_e requestId,
                           SET_COMMAND_e commandId,
                           DEVICE_REPLY_TYPE_e statusId,
                           QString payload);
};

#endif // INSTANTPLAYBACKMEDIA_H
