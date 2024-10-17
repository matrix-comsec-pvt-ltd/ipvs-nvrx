#include "InstantPlaybackFrameReceiver.h"
#include <sys/prctl.h>

#define PAUSE_POLL_TIME                         100

static const QString streamStateString[MAX_STREAM_STATUS] = { "STREAM_NORMAL",
                                                              "STREAM_FILE_ERROR",
                                                              "STREAM_SYNC_START_INDICATOR",
                                                              "STREAM_PLAYBACK_PROCESS_ERROR",
                                                              "STREAM_RESERVED_4",
                                                              "STREAM_HDD_FORMAT",
                                                              "STREAM_CONFIG_CHANGE",
                                                              "STREAM_PLAYBACK_OVER",
                                                              "STREAM_PLAYBACK_SESSION_NOT_AVAIL",
                                                              "STREAM_PLAYBACK_CAM_PREVILAGE",
                                                              "MEDIA_REC_DRIVE_CONFIG_CHANGE",
                                                              "MEDIA_OTHER_ERROR"};

InstantPlaybackFrameReceiver::InstantPlaybackFrameReceiver(SERVER_INFO_t serverInfo,
                                                           REQ_INFO_t &requestInfo,
                                                           SET_COMMAND_e commandId,
                                                           quint8 decoderId)
    : MediaRequest(serverInfo, requestInfo, commandId, decoderId), m_referenceFrameNo(0)
{
    m_feeder = NULL;
    m_frameReceiveFlag = false;
    m_flushBufferFlag = false;
}

InstantPlaybackFrameReceiver::~InstantPlaybackFrameReceiver()
{

}

void InstantPlaybackFrameReceiver::run()
{
    bool status = true;
    QTcpSocket tcpSocket;
    FRAME_HEADER_t header;
    QByteArray frame;
    bool writeFrame = false;

    prctl(PR_SET_NAME, "INSTANT_PLY_FRAME_REC", 0, 0, 0);

    status = createBufferFeeder();
    if(status == false)
    {
        emit sigMediaResponse(request.requestId,
                              cmdId,
                              CMD_INTERNAL_RESOURCE_LIMIT,
                              request.payload);
    }
    else
    {
        status = connectToServer(tcpSocket);
        if(status == true)
        {
            status = sendRequest(tcpSocket);
            if(status == true)
            {
                receiveResponse(tcpSocket, 51);
            }

            if((status == true)
                    && (statusId == CMD_SUCCESS))
            {
                emit sigMediaResponse(request.requestId,
                                      cmdId,
                                      statusId,
                                      request.payload);

                while(getRunFlag () == true)
                {
                    if(getFrameReceiveFlag() == true)
                    {
                        if((receiveHeader(header, tcpSocket) == false)
                                || (receiveFrame(frame, (header.frameSize - sizeof(FRAME_HEADER_t)), tcpSocket) == false))
                        {
                            if(getFrameReceiveFlag() == true)
                            {
                                statusId = CMD_SERVER_NOT_RESPONDING;
                                setRunFlag(false);
                            }
                        }
                        else
                        {
                            statusId = CMD_SUCCESS;
                            if((header.streamStatus == STREAM_SYNC_START_INDICATOR)
                                    && (header.syncFrameNum == getReferenceFrameNo()))
                            {
                                if(m_feeder != NULL)
                                {
                                    m_feeder->resetBufData();
                                }
                                setFrameFeedFlag(true);
                            }
                            else
                            {
                                if((header.streamStatus != STREAM_NORMAL)
                                        && (header.syncFrameNum == getReferenceFrameNo()))
                                {
                                    setFrameReceiveFlag(false);
                                }

                                if((header.syncFrameNum != getReferenceFrameNo())
                                        && (header.streamStatus != STREAM_NORMAL))
                                {
                                    writeFrame = false;
                                }
                                else
                                {
                                    writeFrame = true;
                                }

                                if((writeFrame == true) && (m_feeder != NULL))
                                {
                                    m_feeder->writeFrame(frame, header);
                                }
                            }
                        }
                    }
                    else
                    {
                        usleep(1000);
                    }
                }
            }
        }

        emit sigMediaResponse(request.requestId, cmdId, statusId, "");
        deleteBufferFeeder();
    }
}

void InstantPlaybackFrameReceiver::setFrameReceiveFlag(bool flag)
{
    m_frameReceiveFlagLock.lockForWrite();
    m_frameReceiveFlag = flag;
    m_frameReceiveFlagLock.unlock();
}

bool InstantPlaybackFrameReceiver::getFrameReceiveFlag()
{
    bool flag = false;

    m_frameReceiveFlagLock.lockForRead();
    flag = m_frameReceiveFlag;
    m_frameReceiveFlagLock.unlock();

    return flag;
}

void InstantPlaybackFrameReceiver::setFlushBufferFlag(bool flag)
{
    m_flushBufferFlagLock.lockForWrite();
    m_flushBufferFlag = flag;
    m_flushBufferFlagLock.unlock();
}

bool InstantPlaybackFrameReceiver::getFlushBufferFlag()
{
    bool flag = false;

    m_flushBufferFlagLock.lockForRead();
    flag = m_flushBufferFlag;
    m_flushBufferFlagLock.unlock();

    return flag;
}

void InstantPlaybackFrameReceiver::setFrameFeedFlag(bool flag)
{
    if(m_feeder != NULL)
    {
        m_feeder->setFeedFrameFlag(flag);
    }
}

bool InstantPlaybackFrameReceiver::getFrameFeedFlag()
{
    if(m_feeder != NULL)
    {
        return m_feeder->getFeedFrameFlag();
    }
    else
    {
        return false;
    }
}

void InstantPlaybackFrameReceiver::setReferenceFrameNo(quint8 frameNo)
{
    m_referenceFrameNoLock.lockForWrite();
    m_referenceFrameNo = frameNo;
    m_referenceFrameNoLock.unlock();
}

quint8 InstantPlaybackFrameReceiver::getReferenceFrameNo()
{
    quint8 frameNo;
    m_referenceFrameNoLock.lockForRead();
    frameNo = m_referenceFrameNo;
    m_referenceFrameNoLock.unlock();
    return frameNo;
}

bool InstantPlaybackFrameReceiver::createBufferFeeder()
{
    bool status = false;

    if(m_feeder == NULL)
    {
        /* PARASOFT: Memory Deallocated in delete Buffer Feeder */
        m_feeder = new InstantBufferFeeder(decId);

        if(m_feeder != NULL)
        {
            status = true;
            connect(m_feeder,
                    SIGNAL(sigBufferThreshold(BUFFER_THRESHOLD_e)),
                    this,
                    SLOT(slotBufferThreshold(BUFFER_THRESHOLD_e)));
            connect(m_feeder,
                    SIGNAL(sigFeederResponse(DEVICE_REPLY_TYPE_e,QString)),
                    this,
                    SLOT(slotFeederResponse(DEVICE_REPLY_TYPE_e,QString)));
            m_feeder->start();
        }
    }

    return status;
}

void InstantPlaybackFrameReceiver::deleteBufferFeeder()
{
    if(m_feeder != NULL)
    {
        m_feeder->setRunFlag(false);
        m_feeder->wait();

        disconnect(m_feeder,
                   SIGNAL(sigBufferThreshold(BUFFER_THRESHOLD_e)),
                   this,
                   SLOT(slotBufferThreshold(BUFFER_THRESHOLD_e)));
        disconnect(m_feeder,
                   SIGNAL(sigFeederResponse(DEVICE_REPLY_TYPE_e,QString)),
                   this,
                   SLOT(slotFeederResponse(DEVICE_REPLY_TYPE_e,QString)));
        delete m_feeder;
        m_feeder = NULL;
    }
}

void InstantPlaybackFrameReceiver::slotBufferThreshold(BUFFER_THRESHOLD_e threshold)
{
    emit sigBufferThreshold(threshold);
}

void InstantPlaybackFrameReceiver::slotFeederResponse(DEVICE_REPLY_TYPE_e tStatusId,
                                                      QString responsePayload)
{
    if((tStatusId == CMD_STREAM_PLAYBACK_OVER)
            || (tStatusId == CMD_STREAM_FILE_ERROR)
            || (tStatusId == CMD_STREAM_HDD_FORMAT)
            || (tStatusId == CMD_STREAM_CONFIG_CHANGE)
            || (tStatusId == CMD_PROCESS_ERROR))
    {
        setRunFlag(false);
    }

    emit sigMediaResponse(request.requestId,
                          cmdId,
                          tStatusId,
                          responsePayload);
}
