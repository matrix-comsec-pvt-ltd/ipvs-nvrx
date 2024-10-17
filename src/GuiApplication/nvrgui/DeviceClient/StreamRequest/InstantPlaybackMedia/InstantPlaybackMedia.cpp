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
//   Project      : NVR ( Network Video Recorder)
//   Owner        : Aekam Parmar
//   File         : PlaybackMedia.cpp
//   Description  : This module provides APIs for playback media streaming.
//                  It has a thread function which sends request to server
//                  and if the response is success, receives stream and stores
//                  in the buffer.
//                  It also sends throttle requests [pause / resume] in response
//                  to buffer threshold cross, [maximum / minimum].
//
/////////////////////////////////////////////////////////////////////////////
#include "InstantPlaybackMedia.h"
#include <QStringList>
#include <sys/prctl.h>

InstantPlaybackMedia::InstantPlaybackMedia(SERVER_INFO_t serverInfo,
                                           REQ_INFO_t &requestInfo,
                                           SET_COMMAND_e commandId,
                                           quint8 decoderId)
       :m_serverInfo(serverInfo), m_sessionId(requestInfo.sessionId), m_commandId(commandId), m_timeout(requestInfo.timeout),
        m_msgQueueIndex(0)
{
    m_instantPlaybackCommand = NULL;
    m_frameReceiver = NULL;
    m_runFlag = true;
    m_isValidPlaybackId = false;
    m_isRecvPauseSigSend = false;

    createFrameReceiver(serverInfo, requestInfo, commandId, decoderId);
}

InstantPlaybackMedia::~InstantPlaybackMedia()
{

}

void InstantPlaybackMedia::run()
{
    bool status = true;
    QString payload, devicereplyString;
    QStringList payloadList;
    SET_COMMAND_e command = MAX_NET_COMMAND;
    DEVICE_REPLY_TYPE_e deviceResponse;
    bool emitSignal = true, processCommand = true;

    prctl(PR_SET_NAME, "INSTANT_PB_MED", 0, 0, 0);

    while(getRunFlag () == true)
    {
        m_msgQueueLock.lock();
        if(m_msgQueueIndex <= 0)
        {
            m_msgQueueLock.unlock();

            m_recvPauseSigLock.lock();
            if(m_isRecvPauseSigSend == false)
            {
                m_recvPauseSig.wait(&m_recvPauseSigLock);
            }
            m_isRecvPauseSigSend = false;
            m_recvPauseSigLock.unlock();
        }
        else
        {
            if(m_msgQueueIndex < MAX_MSGQUE_LENGTH)
            {
                command = m_msgQueue[m_msgQueueIndex].command;
                payload = m_msgQueue[m_msgQueueIndex].payload;
                m_msgQueueIndex--;
                processCommand = true;
            }
            else
            {
                processCommand = false;
            }
            m_msgQueueLock.unlock();

            if(processCommand == true)
            {
                if((command == PAUSE_INSTANT_PLY_BUFFER) || (command == RSM_INSTANT_PLY))
                {
                    emitSignal = false;
                }
                else
                {
                    emitSignal = true;
                }

                switch(command)
                {
                case PAUSE_INSTANT_PLY_BUFFER:
                    m_frameReceiver->setFrameReceiveFlag(false);
                    break;

                case PAUSE_INSTANT_PLY:
                    m_frameReceiver->setFrameFeedFlag(false);
                    m_frameReceiver->setFrameReceiveFlag(false);
                    break;

                case STOP_INSTANT_PLY:
                    m_frameReceiver->setFrameReceiveFlag(false);
                    break;

                case SEEK_INSTANT_PLY:
                    payloadList = payload.split(FSP);
                    m_frameReceiver->setReferenceFrameNo((quint8)payloadList.at(4).toUInt());
                    break;

                default:
                    break;
                }

                status = createInstantPbCommand(((command == PAUSE_INSTANT_PLY_BUFFER)
                                                 ? PAUSE_INSTANT_PLY : command),
                                                payload);

                if(status == true)
                {
                    m_instantPlaybackCommand->getBlockingRes(devicereplyString,
                                                             deviceResponse);
                    deleteInstantPbCommand();
                }
                else
                {
                    deviceResponse = CMD_RESOURCE_LIMIT;
                }

                if((emitSignal == true) || (deviceResponse != CMD_SUCCESS))
                {
                    emit sigMediaResponse(MSG_SET_CMD,
                                          command,
                                          deviceResponse,
                                          devicereplyString);
                }

                if(deviceResponse == CMD_SUCCESS)
                {
                    switch(command)
                    {
                    case RSM_INSTANT_PLY:
                        m_frameReceiver->setFrameReceiveFlag(true);
                        break;

                    case SEEK_INSTANT_PLY:
                        m_frameReceiver->setFrameReceiveFlag(true);
                        break;

                    default:
                        break;
                    }
                }
            }
        }
    }

    deleteFrameReceiver();
}

bool InstantPlaybackMedia::createFrameReceiver(SERVER_INFO_t serverInfo,
                                               REQ_INFO_t &requestInfo,
                                               SET_COMMAND_e commandId,
                                               quint8 decoderId)
{
    bool status = false;
    if(m_frameReceiver == NULL)
    {
        /* PARASOFT: Memory Deallocated in delete Frame Receiver */
        m_frameReceiver = new InstantPlaybackFrameReceiver(serverInfo,
                                                           requestInfo,
                                                           commandId,
                                                           decoderId);
        if(m_frameReceiver != NULL)
        {
            status = true;
            connect(m_frameReceiver,
                    SIGNAL(sigBufferThreshold(BUFFER_THRESHOLD_e)),
                    this,
                    SLOT(slotBufferThreshold(BUFFER_THRESHOLD_e)));
            connect(m_frameReceiver,
                    SIGNAL(sigMediaResponse(REQ_MSG_ID_e,SET_COMMAND_e,DEVICE_REPLY_TYPE_e,QString)),
                    this,
                    SLOT(slotMediaResponse(REQ_MSG_ID_e,SET_COMMAND_e,DEVICE_REPLY_TYPE_e,QString)));
            m_frameReceiver->start();
        }
    }
    return status;
}

void InstantPlaybackMedia::deleteFrameReceiver()
{
    if(m_frameReceiver != NULL)
    {
        m_frameReceiver->setRunFlag(false);
        m_frameReceiver->wait();

        disconnect(m_frameReceiver,
                   SIGNAL(sigBufferThreshold(BUFFER_THRESHOLD_e)),
                   this,
                   SLOT(slotBufferThreshold(BUFFER_THRESHOLD_e)));
        disconnect(m_frameReceiver,
                   SIGNAL(sigMediaResponse(REQ_MSG_ID_e,SET_COMMAND_e,DEVICE_REPLY_TYPE_e,QString)),
                   this,
                   SLOT(slotMediaResponse(REQ_MSG_ID_e,SET_COMMAND_e,DEVICE_REPLY_TYPE_e,QString)));

        delete m_frameReceiver;
        m_frameReceiver = NULL;
    }
}

bool InstantPlaybackMedia::createInstantPbCommand(SET_COMMAND_e commandId,
                                                  QString payload)
{
    bool status = false;
    REQ_INFO_t requestInfo;

    requestInfo.sessionId = m_sessionId;
    requestInfo.requestId = MSG_SET_CMD;
    requestInfo.timeout = m_timeout;
    requestInfo.payload = payload;
    requestInfo.bytePayload = NULL;
    requestInfo.windowId = MAX_WIN_ID;

    if(m_instantPlaybackCommand == NULL)
    {
        /* PARASOFT: Memory Deallocated in delete Instant Pb Command */
        m_instantPlaybackCommand = new CommandRequest(m_serverInfo,
                                                      requestInfo,
                                                      commandId,
                                                      STRM_RELAT_CMD_SES_ID);

        if(m_instantPlaybackCommand != NULL)
        {
            status = true;
        }
    }

    return status;
}

void InstantPlaybackMedia::deleteInstantPbCommand()
{
    if(m_instantPlaybackCommand != NULL)
    {
        m_instantPlaybackCommand->wait();
        delete m_instantPlaybackCommand;
        m_instantPlaybackCommand = NULL;
    }
}

void InstantPlaybackMedia::sigToRecvPauseFalse()
{
    m_recvPauseSigLock.lock();
    m_isRecvPauseSigSend = true;
    m_recvPauseSig.wakeAll();
    m_recvPauseSigLock.unlock();
}

bool InstantPlaybackMedia::getRunFlag()
{
    bool isRunFlag = false;

    m_runFlagLock.lockForRead ();
    isRunFlag = m_runFlag;
    m_runFlagLock.unlock ();

    return isRunFlag;
}

void InstantPlaybackMedia::setRunFlag(bool flag)
{
    m_runFlagLock.lockForWrite();
    m_runFlag = flag;
    m_runFlagLock.unlock();
}

void InstantPlaybackMedia::setFrameReceiveFlag(bool flag)
{
    m_frameReceiver->setFrameReceiveFlag(flag);
}

bool InstantPlaybackMedia::getFrameReceiveFlag()
{
    return m_frameReceiver->getFrameReceiveFlag();
}

void InstantPlaybackMedia::setPlaybackId(QString playbackId)
{
    m_playBackId = playbackId;
    m_isValidPlaybackId = true;
}

bool InstantPlaybackMedia::isValidPlaybackId()
{
    return m_isValidPlaybackId;
}

bool InstantPlaybackMedia::setInstantPlaybackCommand(SET_COMMAND_e commandId,
                                                     QString payloadStr)
{
    bool status = false;
    m_msgQueueLock.lock();
    if(m_msgQueueIndex < (MAX_MSGQUE_LENGTH))
    {
        status = true;
        m_msgQueueIndex++;
        m_msgQueue[m_msgQueueIndex].command = commandId;
        m_msgQueue[m_msgQueueIndex].payload = payloadStr;
    }
    m_msgQueueLock.unlock();

    sigToRecvPauseFalse();
    return status;
}

void InstantPlaybackMedia::slotBufferThreshold(BUFFER_THRESHOLD_e threshold)
{
    m_msgQueueLock.lock();
    if(m_msgQueueIndex < (MAX_MSGQUE_LENGTH))
    {
        m_msgQueueIndex++;

        if(threshold == CROSSED_MINIMUM)
        {
            m_msgQueue[m_msgQueueIndex].command = RSM_INSTANT_PLY;
        }
        else
        {
            m_msgQueue[m_msgQueueIndex].command = PAUSE_INSTANT_PLY_BUFFER;
        }
        m_msgQueue[m_msgQueueIndex].payload = m_playBackId.append(FSP);
    }
    m_msgQueueLock.unlock();

    sigToRecvPauseFalse();
}

void InstantPlaybackMedia::slotMediaResponse(REQ_MSG_ID_e requestId,
                                             SET_COMMAND_e commandId,
                                             DEVICE_REPLY_TYPE_e statusId,
                                             QString payload)
{
    emit sigMediaResponse(requestId,
                          commandId,
                          statusId,
                          payload);
}
