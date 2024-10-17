#ifndef INSTANTPLAYBACKFRAMERECEIVER_H
#define INSTANTPLAYBACKFRAMERECEIVER_H

#include <QObject>
#include <QString>
#include <QStringList>

#include "../MediaRequest.h"
#include "InstantBufferFeeder.h"
#include "EnumFile.h"

class InstantPlaybackFrameReceiver : public MediaRequest
{
    Q_OBJECT
public:
    InstantPlaybackFrameReceiver(SERVER_INFO_t serverInfo,
                                 REQ_INFO_t &requestInfo,
                                 SET_COMMAND_e commandId,
                                 quint8 decoderId);
    ~InstantPlaybackFrameReceiver();

    void run();

    void setFrameReceiveFlag(bool flag);
    bool getFrameReceiveFlag();
    void setFlushBufferFlag(bool flag);
    bool getFlushBufferFlag();
    void setFrameFeedFlag(bool flag);
    bool getFrameFeedFlag();
    void setReferenceFrameNo(quint8 frameNo);
    quint8 getReferenceFrameNo();

protected:
    InstantBufferFeeder *m_feeder;
    bool m_frameReceiveFlag, m_flushBufferFlag;
    QReadWriteLock m_frameReceiveFlagLock, m_flushBufferFlagLock;
    quint8 m_referenceFrameNo;
    QReadWriteLock m_referenceFrameNoLock;

    bool createBufferFeeder();
    void deleteBufferFeeder();

signals:
    void sigBufferThreshold(BUFFER_THRESHOLD_e thresholds);

public slots:
    void slotBufferThreshold(BUFFER_THRESHOLD_e threshold);
    void slotFeederResponse(DEVICE_REPLY_TYPE_e tStatusId,
                            QString responsePayload);
};

#endif // INSTANTPLAYBACKFRAMERECEIVER_H
