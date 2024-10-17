#include "StreamRequest.h"
#include "Layout/Layout.h"
#include "SyncPlayback.h"
#include "unistd.h"
#include "../DecoderLib/include/DecDispLib.h"
#include "ValidationMessage.h"
#include "DebugLog.h"

QMutex StreamRequest::audioDecoderLock;
quint8 StreamRequest::audioStreamIndex = MAX_STREAM_SESSION;
quint8 StreamRequest::audioIndex = MAX_WINDOWS;

quint8 StreamRequest::m_queueReadIndex[MAX_STREAM_SESSION] = {0};
quint8 StreamRequest::m_queueWriteIndex[MAX_STREAM_SESSION] = {0};
STREAM_REQUEST_QUEUE_t StreamRequest::m_requestQueue[MAX_STREAM_SESSION][MAX_STREAM_COMMAND_REQUEST];
QMutex StreamRequest::m_queueLock[MAX_STREAM_SESSION];


StreamRequest::StreamRequest(quint8 streamIndex)
{
    setObjectName ("StreamRequest[" + QString("%1").arg (streamIndex) + "]");

    SERVER_SESSION_INFO_t info;
    info.serverInfo.ipAddress = "";
    info.serverInfo.tcpPort = 0;
    info.sessionInfo.sessionId = "";
    info.sessionInfo.timeout = 0;
    setCurrentServerInfo(info);

    m_streamId = streamIndex;
    m_decoderId = INVALID_DEC_DISP_PLAY_ID;
    m_deviceName = "";
    m_channelIndex = MAX_CAMERAS;

    m_queueLock[m_streamId].lock ();
    m_queueReadIndex[m_streamId] = 0;
    m_queueWriteIndex[m_streamId] = 0;
    m_queueLock[m_streamId].unlock ();

    m_displayType = m_duplicateDisplayType = MAX_DISPLAY_TYPE;

    for(quint8 displayIndex = 0; displayIndex < MAX_DISPLAY_TYPE; displayIndex++)
    {
        m_displayTypeForDelete[displayIndex] = MAX_DISPLAY_TYPE;
        m_windowId[displayIndex] = MAX_WINDOWS;
        m_actualWindowId[displayIndex] = MAX_CHANNEL_FOR_SEQ;
        m_actualWindowIdForDelete[displayIndex] = MAX_CHANNEL_FOR_SEQ;
        m_timeStamp[displayIndex] = 0;
    }

    for(quint8 index = 0; index < MAX_SYNC_PB_SESSION; index++)
    {
        m_syncDecoderIdSet[index] = INVALID_DEC_DISP_PLAY_ID;
    }

    m_streamStatus = STREAM_NONE;
    m_streamType = MAX_STREAM_REQUEST_TYPE;
    m_liveStreamType = MAX_LIVE_STREAM_TYPE;
    setDecoderIntanceCreated(false);
    m_reRequest = false;

    setClearFlag(false);
    setFreeFlag(false);
    m_deleteAfterMediaControlDeleteFlag = false;
    setMediaPauseForReplaceFlag(false);
    m_replaceFlag = false;
    m_deletionProcess = false;

    INIT_OBJ(m_liveMedia);
    INIT_OBJ(m_analogMedia);
    INIT_OBJ(m_playbackMedia);
    INIT_OBJ(m_syncPlaybackMedia);
    INIT_OBJ(m_instantPlaybackMedia);
    INIT_OBJ(m_mediaControl);
    INIT_OBJ(m_clientMedia);
    INIT_OBJ(m_audioIn);
    INIT_OBJ(m_nextRequestParam);
}

StreamRequest::~StreamRequest()
{

}

void StreamRequest::deleteStreamRequest()
{
    deleteMediaControl();
    freeStreamRequest();
}

void StreamRequest::delMedControl()
{
    deleteMediaControl();
}

void StreamRequest::processRequest(STREAM_COMMAND_TYPE_e streamCommandType,
                                   SERVER_SESSION_INFO_t serverInfo,
                                   StreamRequestParam *streamRequestParam)
{
    if (!(getDeletionProcessFlag()))
    {
        // This signal is connected to StreamRequest::slotProcessRequest [Note : 'Queued Connection']
        emit sigProcessRequest(streamCommandType,
                               serverInfo,
                               streamRequestParam);
    }
    else
    {
        emit sigStreamRequestResponse(streamCommandType,
                                      streamRequestParam,
                                      CMD_MAX_DEVICE_REPLY);
    }
}

void StreamRequest::processRequest(STREAM_COMMAND_TYPE_e streamCommandType,
                                   SERVER_SESSION_INFO_t serverInfo,
                                   StreamRequestParam *streamRequestParam,
                                   SERVER_SESSION_INFO_t nextServerInfo,
                                   StreamRequestParam *nextStreamRequestParam)
{
    emit sigProcessRequest(streamCommandType,
                           serverInfo,
                           streamRequestParam,
                           nextServerInfo,
                           nextStreamRequestParam);
}

bool StreamRequest::getClearFlag()
{
    bool flag = false;

    m_clearFlagAccess.lock();
    flag = m_clearFlag;
    m_clearFlagAccess.unlock();

    return flag;
}

void StreamRequest::setClearFlag(bool flag)
{
    m_clearFlagAccess.lock();
    m_clearFlag = flag;
    m_clearFlagAccess.unlock();
}

bool StreamRequest::getFreeFlag()
{
    bool flag = false;

    m_freeFlagAccess.lock();
    flag = m_freeFlag;
    m_freeFlagAccess.unlock();

    return flag;
}

void StreamRequest::setFreeFlag(bool flag)
{
    m_freeFlagAccess.lock();
    m_freeFlag = flag;
    m_freeFlagAccess.unlock();
}

bool StreamRequest::getSetFreeFlag()
{
    bool flag = false;

    m_freeFlagAccess.lock();

    if(m_freeFlag == false)
    {
        m_freeFlag = true;
    }
    else
    {
        flag = true;
    }
    m_freeFlagAccess.unlock();

    return flag;
}

bool StreamRequest::getReplaceFlag()
{
    bool flag = false;

    m_replaceFlagAccess.lock();
    flag = m_replaceFlag;
    m_replaceFlagAccess.unlock();

    return flag;
}

void StreamRequest::setReplaceFlag(bool flag)
{
    m_replaceFlagAccess.lock();
    m_replaceFlag = flag;
    m_replaceFlagAccess.unlock();
}

void StreamRequest :: getStreamWindowInfo(DISPLAY_TYPE_e *displayType, quint16 *windowIndex)
{
    m_streamParamMutex.lock();
    memcpy(displayType,m_displayTypeForDelete,sizeof(m_displayTypeForDelete));
    memcpy(windowIndex,m_actualWindowIdForDelete,sizeof(m_actualWindowIdForDelete));
    m_streamParamMutex.unlock();
}

QString StreamRequest::getStreamDeviceName()
{
    QString deviceName;

    m_streamParamMutex.lock();
    deviceName = m_deviceName;
    m_streamParamMutex.unlock();

    return deviceName;
}

bool StreamRequest::getDeletionProcessFlag()
{
    bool flag = false;

    m_deletionProcessLock.lock ();
    flag = m_deletionProcess;
    m_deletionProcessLock.unlock ();

    return flag;
}

void StreamRequest::setDeletionProcessFlag (bool flag)
{

    m_deletionProcessLock.lock ();
    m_deletionProcess = flag;
    m_deletionProcessLock.unlock ();
}

void StreamRequest::setDecoderIntanceCreated(bool flag)
{
    m_decoderInstanceLock.lock();
    m_isDecInstanceCreated = flag;
    m_decoderInstanceLock.unlock();
}

bool StreamRequest::getDecoderIntanceCreated()
{
    bool ret;
    m_decoderInstanceLock.lock();
    ret = m_isDecInstanceCreated;
    m_decoderInstanceLock.unlock();
    return ret;
}



void StreamRequest::getWindowInfo(DISPLAY_TYPE_e displayType,
                                  quint8 &decoderWindow,
                                  quint16 &actualWindow,
                                  qint64 &timeStamp)
{
    m_streamParamMutex.lock ();
    if(displayType < MAX_DISPLAY_TYPE)
    {
        decoderWindow = m_windowId[displayType];
        actualWindow = m_actualWindowId[displayType];
        timeStamp = m_timeStamp[displayType];
    }
    m_streamParamMutex.unlock ();
}

void StreamRequest::setWindowInfo(DISPLAY_TYPE_e displayType,
                                  quint8 decoderWindow,
                                  quint16 actualWindow,
                                  qint64 timeStamp)
{
    m_streamParamMutex.lock();
    if(displayType < MAX_DISPLAY_TYPE)
    {
        m_windowId[displayType] = decoderWindow;
        if(timeStamp > 0)
        {
            m_timeStamp[displayType] = timeStamp;
        }

        if(actualWindow < MAX_CHANNEL_FOR_SEQ)
        {
            m_actualWindowId[displayType] = actualWindow;
        }
    }
    m_streamParamMutex.unlock();
}

void StreamRequest::swapWindowInfo(DISPLAY_TYPE_e displayType,
                                   StreamRequest *firstRequest,
                                   StreamRequest *secondRequest)
{
    quint8 firstDecoderWindow, secondDecoderWindow;
    quint16 firstActualWindow, secondActualWindow;
    qint64 firstTimeStamp, secondtTimeStamp;

    firstRequest->getWindowInfo(displayType,
                                firstDecoderWindow,
                                firstActualWindow,
                                firstTimeStamp);

    secondRequest->getWindowInfo(displayType,
                                 secondDecoderWindow,
                                 secondActualWindow,
                                 secondtTimeStamp);

    firstRequest->setWindowInfo(displayType,
                                secondDecoderWindow,
                                secondActualWindow,
                                secondtTimeStamp);

    secondRequest->setWindowInfo(displayType,
                                 firstDecoderWindow,
                                 firstActualWindow,
                                 firstTimeStamp);
}

quint8 StreamRequest::getAudioIndex()
{
    quint8 audioIdx;
    QMutexLocker locker(&audioDecoderLock);
    audioIdx = audioIndex;

    return audioIdx;
}

void StreamRequest::setAudioIndex(const quint8 &value)
{
    QMutexLocker locker(&audioDecoderLock);
    audioIndex = value;
}

quint8 StreamRequest::getAudioStreamIndex()
{
    quint8 strIndex;
    QMutexLocker locker(&audioDecoderLock);
    strIndex = audioStreamIndex;

    return strIndex;
}


void StreamRequest::setAudioStreamIndex(const quint8 &value)
{
    QMutexLocker locker(&audioDecoderLock);
    audioStreamIndex = value;
}

bool StreamRequest::createNewStream(SERVER_SESSION_INFO_t serverSessionInfo,
                                    StreamRequestParam *streamRequestParam,
                                    bool &isDecoderError,
                                    bool createMediaFlag)
{
    bool status = false;
    SET_COMMAND_e commandId = MAX_NET_COMMAND;
    isDecoderError = false;

    if(streamRequestParam != NULL)
    {
        if(streamRequestParam->displayType >= MAX_DISPLAY_TYPE)
        {
            EPRINT(STREAM_REQ,"Max Display Type Error");
            return status;
        }

        if(m_decoderId != INVALID_DEC_DISP_PLAY_ID)
        {
            EPRINT(STREAM_REQ,"DecoderId must be Invalid");
        }

        m_streamParamMutex.lock ();
        m_deviceName = streamRequestParam->deviceName;
        m_channelIndex = streamRequestParam->channelId;
        m_displayType = streamRequestParam->displayType;
        m_actualWindowId[m_displayType] = streamRequestParam->actualWindowId;
        m_windowId[m_displayType] = streamRequestParam->windowId;
        m_streamType = streamRequestParam->streamRequestType;
        m_liveStreamType = streamRequestParam->liveStreamType;
        m_timeStamp[m_displayType] = streamRequestParam->timeStamp;
        m_streamParamMutex.unlock ();

        switch(m_streamType)
        {
            case LIVE_STREAM_REQUEST:
            {
                // Assign Free Decoder channel, return Decoder ID in [m_decoderId]
                commandId = SRT_LV_STRM;
                status = StartChannelView((DISPLAY_DEV_e)m_displayType, m_windowId[m_displayType], &m_decoderId);
            }
            break;

            case LIVE_ANALOG_REQUEST:
            {
                commandId = SRT_LV_STRM;
                m_liveStreamType = LIVE_STREAM_TYPE_MAIN;
            }
            break;

            case PLAYBACK_STREAM_REQUEST:
            {
                commandId = PLY_RCD_STRM;
                status = StartChannelView((DISPLAY_DEV_e)m_displayType, m_windowId[m_displayType], &m_decoderId);
            }
            break;

            case SYNC_PLAYBACK_REQUEST:
            {
                commandId = PLYBCK_RCD;
                status = true;
                for(quint8 index = 0; index < MAX_SYNC_PB_SESSION; index++)
                {
                    //Find free decoder instance
                    StartChannelView((DISPLAY_DEV_e)m_displayType, index, &m_syncDecoderIdSet[index]);
                }
            }
            break;

            case INSTANT_PLAYBACK_STREAM_REQUEST:
            {
                commandId = STRT_INSTANT_PLY;
                status = StartChannelView((DISPLAY_DEV_e)m_displayType, m_windowId[m_displayType], &m_decoderId);
            }
            break;

            default:
            {
                /* Nothing to do */
            }
            break;
        }

        if(status == true)
        {
            isDecoderError = false;
            setDecoderIntanceCreated(true);
            DPRINT(GUI_LIVE_MEDIA,"[StreamId=%d] : [Decoder=%d] assigned to [wind=%d]", m_streamId,m_decoderId,m_windowId[m_displayType]);
            if(createMediaFlag == true)
            {
                // Create new Media request for communicating to server
                status = createMediaRequest(serverSessionInfo, commandId, streamRequestParam);
            }
            else
            {
                DELETE_OBJ (streamRequestParam);
            }
        }
        else
        {
             EPRINT(STREAM_REQ,"createNewStream Stream Error [%d]",m_streamId);
        }
    }
    return status;
}

void StreamRequest::startMedia(SET_COMMAND_e commandId)
{

    StreamRequestParam *streamRequestParam=NULL;
    switch(m_streamType)
    {
    case LIVE_STREAM_REQUEST:
        if (m_displayType >= MAX_DISPLAY_TYPE)
        {
            break;
        }

        streamRequestParam = new StreamRequestParam();
        if(IS_VALID_OBJ(streamRequestParam))
        {
            m_streamParamMutex.lock ();
            streamRequestParam->streamId = m_streamId;
            streamRequestParam->windowId = m_windowId[m_displayType];
            streamRequestParam->actualWindowId = m_actualWindowId[m_displayType];
            streamRequestParam->channelId = m_channelIndex;
            streamRequestParam->deviceName = m_deviceName;
            streamRequestParam->displayType = m_displayType;
            streamRequestParam->streamRequestType = m_streamType;
            streamRequestParam->liveStreamType = m_liveStreamType;
            streamRequestParam->timeStamp = m_timeStamp[m_displayType];
            m_streamParamMutex.unlock ();

            // signal for changing state of widow to 'Stream Connecting'
            emit sigStreamRequestResponse(START_STREAM_COMMAND,
                                          streamRequestParam,
                                          CMD_STREAM_CONNECTING);
            // start live media thread
            if(IS_VALID_OBJ(m_liveMedia))
            {
                // start live media thread => void LiveMedia::run ()
                m_liveMedia->start();
                DPRINT(GUI_LIVE_MEDIA, "live media started: [streamId=%d], [cameraId=%d]", m_streamId, m_channelIndex);
            }
            else
            {
                EPRINT(STREAM_REQ, "live media object null: [streamId=%d]", m_streamId);
            }
        }
        else
        {
            EPRINT(STREAM_REQ,"Error in creating Stream Request Parameter class [%d]", m_streamId);
        }
        break;

    case LIVE_ANALOG_REQUEST:
        if (m_displayType >= MAX_DISPLAY_TYPE)
        {
            EPRINT(STREAM_REQ,"Invalid Display Type [%d]", m_streamId);
            break;
        }

        streamRequestParam = new StreamRequestParam();
        if(IS_VALID_OBJ(streamRequestParam))
        {
            m_streamParamMutex.lock ();
            streamRequestParam->streamId = m_streamId;
            streamRequestParam->windowId = m_windowId[m_displayType];
            streamRequestParam->actualWindowId = m_actualWindowId[m_displayType];
            streamRequestParam->channelId = m_channelIndex;
            streamRequestParam->deviceName = m_deviceName;
            streamRequestParam->displayType = m_displayType;
            streamRequestParam->streamRequestType = m_streamType;
            streamRequestParam->liveStreamType = m_liveStreamType;
            streamRequestParam->timeStamp = m_timeStamp[m_displayType];
            m_streamParamMutex.unlock ();

            emit sigStreamRequestResponse(START_STREAM_COMMAND,
                                          streamRequestParam,
                                          CMD_STREAM_CONNECTING);
            if(IS_VALID_OBJ(m_analogMedia))
            {
                m_analogMedia->start();
            }
            else
            {
                EPRINT(STREAM_REQ,"Analog media object NULL [%d]",m_streamId);
            }
        }
        else
        {
            EPRINT(STREAM_REQ,"No Stream Object Initalize");
        }
        break;

    case PLAYBACK_STREAM_REQUEST:
        if(commandId == PLY_RCD_STRM)
        {
            if(m_displayType >= MAX_DISPLAY_TYPE)
            {
                EPRINT(STREAM_REQ,"Invalid Display Type [%d]",m_streamId);
                break;
            }

            streamRequestParam = new StreamRequestParam();
            if(IS_VALID_OBJ(streamRequestParam))
            {
                m_streamParamMutex.lock ();
                streamRequestParam->streamId = m_streamId;
                streamRequestParam->windowId = m_windowId[m_displayType];
                streamRequestParam->actualWindowId = m_actualWindowId[m_displayType];
                streamRequestParam->channelId = m_channelIndex;
                streamRequestParam->deviceName = m_deviceName;
                streamRequestParam->displayType = m_displayType;
                streamRequestParam->streamRequestType = m_streamType;
                streamRequestParam->liveStreamType = m_liveStreamType;
                streamRequestParam->timeStamp = m_timeStamp[m_displayType];
                m_streamParamMutex.unlock ();

                emit sigStreamRequestResponse(PLAY_PLABACK_STREAM_COMMAND,
                                              streamRequestParam,
                                              CMD_STREAM_CONNECTING);
            }
            else
            {
               EPRINT(STREAM_REQ,"No Stream Object Initalize");
            }
        }

        if(IS_VALID_OBJ(m_playbackMedia))
        {
            m_playbackMedia->start();
        }
        break;

    case INSTANT_PLAYBACK_STREAM_REQUEST:
        if(IS_VALID_OBJ(m_instantPlaybackMedia))
        {
            m_instantPlaybackMedia->start();
        }
        break;

    case SYNC_PLAYBACK_REQUEST:
        if(IS_VALID_OBJ(m_syncPlaybackMedia))
        {
            m_syncPlaybackMedia->start();
        }
        break;

    default:
        break;
    }
}

void StreamRequest::restartMedia()
{
    switch(m_streamType)
    {
    case LIVE_ANALOG_REQUEST:
        if(IS_VALID_OBJ(m_analogMedia))
        {
            m_analogMedia->setRunFlag(true);
            m_analogMedia->setStopFlag(false);
            m_analogMedia->start();
        }
        break;

    case LIVE_STREAM_REQUEST:
        if(IS_VALID_OBJ(m_liveMedia))
        {
            m_liveMedia->setRunFlag(true);
            m_liveMedia->setStopFlag(false);
            m_liveMedia->start();
        }
        break;

    default:
        break;
    }
}

void StreamRequest::stopMedia(bool deleteDecoder, bool isDecoderError)
{
    bool sendSignal = false;
    bool status = false;

    switch(m_streamType)
    {
    case LIVE_STREAM_REQUEST:
        if(IS_VALID_OBJ(m_liveMedia))
        {
            m_liveMedia->setStopFlag(true);
            m_liveMedia->setRunFlag(false);
            if(m_liveMedia->isRunning())
            {
                m_liveMedia->wait();
            }
            status = true;
        }
        sendSignal = true;
        break;

    case LIVE_ANALOG_REQUEST:
        if(IS_VALID_OBJ(m_analogMedia))
        {
            m_analogMedia->setStopFlag(true);
            m_analogMedia->setRunFlag(false);
            if(m_analogMedia->isRunning())
            {
                m_analogMedia->wait();
            }
            status = true;
            sendSignal = true;
        }
        break;

    case PLAYBACK_STREAM_REQUEST:
        if(IS_VALID_OBJ(m_playbackMedia))
        {
            m_playbackMedia->setStopFlag(true);
            m_playbackMedia->setRunFlag(false);
            if(m_playbackMedia->isRunning())
            {
                m_playbackMedia->wait ();
            }
            status = true;
        }
        break;

    default:
        break;
    }

    if(status == true)
    {
        if(deleteDecoder == true)
        {
            deleteDecoderInstance();
        }
        else
        {
            pauseDecoderInstance();
        }
    }

    if((sendSignal == true) && (isDecoderError == false))
    {
        if(m_displayType < MAX_DISPLAY_TYPE)
        {
            StreamRequestParam *streamRequestParam = new StreamRequestParam();
            if(IS_VALID_OBJ(streamRequestParam))
            {
                m_streamParamMutex.lock ();
                streamRequestParam->streamId = m_streamId;
                streamRequestParam->windowId = m_windowId[m_displayType];
                streamRequestParam->actualWindowId = m_actualWindowId[m_displayType];
                streamRequestParam->channelId = m_channelIndex;
                streamRequestParam->deviceName = m_deviceName;
                streamRequestParam->displayType = m_displayType;
                streamRequestParam->streamRequestType = m_streamType;
                streamRequestParam->liveStreamType = m_liveStreamType;
                streamRequestParam->timeStamp = m_timeStamp[m_displayType];
                m_streamParamMutex.unlock ();

                emit sigStreamRequestResponse(STOP_STREAM_COMMAND,
                                              streamRequestParam,
                                              CMD_SUCCESS);
            }
        }
        else
        {
            EPRINT(STREAM_REQ,"Maximun Display Type [%d]", m_streamId);
        }
    }
}

bool StreamRequest::stopStream(StreamRequestParam *streamRequestParam,
                               bool deleteDecoder, bool isDecoderError,bool *sameReq)
{

    bool status = false;
    if(sameReq != NULL)
    {
        *sameReq = false;
    }

    SERVER_SESSION_INFO_t serverSessionInfo;

    getCurrentServerInfo(serverSessionInfo);

    switch(m_streamType)
    {
    case LIVE_STREAM_REQUEST:
        if(IS_VALID_OBJ(m_liveMedia))
        {
            m_liveMedia->setRunFlag(false);
            m_liveMedia->setStopFlag (true);
        }
        // create new media control object , Request : {MSG_SET_CMD,STP_LV_STRM,..}
        status = createMediaControl(serverSessionInfo,
                                    STP_LV_STRM,
                                    streamRequestParam->payload);
        if(status == true)
        {
            m_mediaControl->start();
        }
        else
        {
            if(m_mediaControl != NULL)
            {
                if(m_mediaControl->getCommandOfCommandReq() == STP_LV_STRM)
                {
                    if(sameReq != NULL)
                    {
                        *sameReq = true;
                    }
                }
            }
            return status;
        }
        break;

    case LIVE_ANALOG_REQUEST:
        status = true;
        break;

    case PLAYBACK_STREAM_REQUEST:
        status = createMediaControl(serverSessionInfo,
                                    STP_RCD_STRM,
                                    streamRequestParam->payload);
        if(status == true)
        {
            m_mediaControl->start();
        }
        break;

    default:
        return false;
    }

     stopMedia(deleteDecoder,isDecoderError);

    return status;
}

void StreamRequest::deleteDecoderInstance()
{
    if(getDecoderIntanceCreated() == true)
    {
        switch(m_streamType)
        {
            case LIVE_STREAM_REQUEST:
            case PLAYBACK_STREAM_REQUEST:
            case INSTANT_PLAYBACK_STREAM_REQUEST:
            {
                StopChannelView(m_decoderId);
            }
            break;

            case LIVE_ANALOG_REQUEST:
            {
            }
            break;

            case SYNC_PLAYBACK_REQUEST:
            {
                for(quint8 index = 0; index < MAX_SYNC_PB_SESSION; index++)
                {
                    StopChannelView(m_syncDecoderIdSet[index]);
                    m_syncDecoderIdSet[index] = INVALID_DEC_DISP_PLAY_ID;
                }
            }
            break;

        default:
            break;
        }

        setDecoderIntanceCreated(false);
        m_decoderId = INVALID_DEC_DISP_PLAY_ID;
    }
}

void StreamRequest::pauseDecoderInstance(bool stopAnalogFlag)
{
    if(m_displayType >= MAX_DISPLAY_TYPE)
    {
        EPRINT(STREAM_REQ,"MaxDisplay Type Error");//cppcheckChange
        return;
    }

    switch(m_streamType)
    {
        case LIVE_ANALOG_REQUEST:
            if((stopAnalogFlag == true) && (getDecoderIntanceCreated() == true))
            {

            }
            break;

        case LIVE_STREAM_REQUEST:
        case PLAYBACK_STREAM_REQUEST:
        case INSTANT_PLAYBACK_STREAM_REQUEST:
            if(getDecoderIntanceCreated() == true)
            {
                UpdateDecoderStatusOnVideoLoss(m_decoderId, (DISPLAY_DEV_e)m_displayType, m_windowId[m_displayType]);
            }
            break;

        case SYNC_PLAYBACK_REQUEST:
            for(quint8 index = 0; index < MAX_SYNC_PB_SESSION; index++)
            {
                if(m_syncDecoderIdSet[index] != INVALID_DEC_DISP_PLAY_ID)
                {
                    UpdateDecoderStatusOnVideoLoss(m_syncDecoderIdSet[index], MAIN_VIDEO_DISPLAY, index);
                }
            }
            break;

        default:
            break;
    }
}

void StreamRequest::resumeDecoderInstance()
{
    if(m_displayType >= MAX_DISPLAY_TYPE)
    {
        EPRINT(STREAM_REQ,"MaxDisplay Type Error");//cppcheckChange
        return;
    }
}

void StreamRequest::freeStreamRequest()
{

    if(getSetFreeFlag () == false)
    {
        excludeAudio ();
        deleteMediaRequest();
        deleteDecoderInstance();
        clearStreamInfo();
   }
}

void StreamRequest::replaceStreamRequest()
{
    replaceMediaRequest();
    clearStreamInfo();

    setCurrentServerInfo(m_nextServerInfo);

    m_streamParamMutex.lock ();
    m_deviceName = m_nextRequestParam->deviceName;
    m_channelIndex = m_nextRequestParam->channelId;
    m_displayType = m_nextRequestParam->displayType;
    m_actualWindowId[m_displayType] = m_nextRequestParam->actualWindowId;
    m_windowId[m_displayType] = m_nextRequestParam->windowId;
    m_streamType = m_nextRequestParam->streamRequestType;
    m_liveStreamType = m_nextRequestParam->liveStreamType;
    m_streamParamMutex.unlock ();

    REQ_INFO_t requestInfo;
    SERVER_INFO_t serverInfo;

    serverInfo.ipAddress = m_nextServerInfo.serverInfo.ipAddress;
    serverInfo.tcpPort = m_nextServerInfo.serverInfo.tcpPort;

    requestInfo.bytePayload = NULL;
    requestInfo.payload = m_nextRequestParam->payload;
    requestInfo.requestId = MSG_SET_CMD;
    requestInfo.sessionId = m_nextServerInfo.sessionInfo.sessionId;
    requestInfo.timeout = m_nextServerInfo.sessionInfo.timeout;

    switch(m_streamType)
    {
    case LIVE_ANALOG_REQUEST:
        if(m_analogMedia != NULL)
        {
            m_analogMedia->setServerSessionInfo(serverInfo,
                                                requestInfo,
                                                m_channelIndex);
        }
        break;

    case LIVE_STREAM_REQUEST:
        if(m_liveMedia != NULL)
        {
            m_liveMedia->setServerSessionInfo(serverInfo, requestInfo);
        }
        break;

    default:
        break;
    }

    resumeDecoderInstance();
    restartMedia();

    m_nextServerInfo.serverInfo.ipAddress = "";
    m_nextServerInfo.serverInfo.tcpPort = 0;
    m_nextServerInfo.sessionInfo.sessionId = "";
    m_nextServerInfo.sessionInfo.timeout = 0;

    DELETE_OBJ(m_nextRequestParam);
}

void StreamRequest::clearStreamInfo()
{
    if(m_displayType >= MAX_DISPLAY_TYPE)
    {
        EPRINT(STREAM_REQ,"MaxDisplay Type Error");//cppcheckChange
        return;
    }

    m_streamStatus = STREAM_NONE;
    m_streamParamMutex.lock ();
    if(m_displayType < MAX_DISPLAY_TYPE)
    {
        m_windowId[m_displayType] = MAX_WINDOWS;
        m_actualWindowId[m_displayType] = MAX_CHANNEL_FOR_SEQ;
        m_timeStamp[m_displayType] = 0;
        m_displayType = MAX_DISPLAY_TYPE;
    }
    else
    {
      EPRINT(STREAM_REQ,"Invalid Display Type [%d]",m_streamId);
    }
    m_deviceName = "";
    m_channelIndex = MAX_CAMERAS;
    m_streamStatus = STREAM_NONE;
    m_streamType = MAX_STREAM_REQUEST_TYPE;
    m_liveStreamType = MAX_LIVE_STREAM_TYPE;
    m_playbackId = "";
    m_streamParamMutex.unlock ();

    SERVER_SESSION_INFO_t info;
    info.serverInfo.ipAddress = "";
    info.serverInfo.tcpPort = 0;
    info.sessionInfo.sessionId = "";
    info.sessionInfo.timeout = 0;
    setCurrentServerInfo(info);
}
/* Create new liveMedia Object for given streamType
 * 1)Live Stream , 2)Analog Stream, 3)Playback, 4)Sync PB, 5)Instant PB, 6)Client Audio
 * */
bool StreamRequest::createMediaRequest(SERVER_SESSION_INFO_t serverSessionInfo,
                                       SET_COMMAND_e commandId,
                                       StreamRequestParam *streamRequestParam)
{

    bool status = false;
    REQ_INFO_t requestInfo;

    SERVER_INFO_t serverInfo;

    serverInfo.ipAddress = serverSessionInfo.serverInfo.ipAddress;
    serverInfo.tcpPort = serverSessionInfo.serverInfo.tcpPort;

    requestInfo.bytePayload = NULL;
    requestInfo.payload = streamRequestParam->payload;
    requestInfo.requestId = MSG_SET_CMD;
    requestInfo.sessionId = serverSessionInfo.sessionInfo.sessionId;
    requestInfo.timeout = serverSessionInfo.sessionInfo.timeout;

    if(!getDeletionProcessFlag ())
    {
        switch(m_streamType)
        {
        case LIVE_STREAM_REQUEST:
            if((m_liveMedia == NULL) && (m_streamId < MAX_STREAM_SESSION))
            {
                /* PARASOFT: Memory Deallocated in delete Media Request */
                m_liveMedia = new LiveMedia(serverInfo,
                                            requestInfo,
                                            commandId,
                                            m_decoderId,
                                            m_streamId);

                if(IS_VALID_OBJ(m_liveMedia))
                {
                    /* set stack size */
                    m_liveMedia->setStackSize(LIVE_MEDIA_THREAD_STACK_SIZE);

                    connect(m_liveMedia,
                            SIGNAL(sigMediaResponse(REQ_MSG_ID_e,
                                                    SET_COMMAND_e,
                                                    DEVICE_REPLY_TYPE_e,
                                                    QString)),
                            this,
                            SLOT(slotStreamResponse(REQ_MSG_ID_e,
                                                    SET_COMMAND_e,
                                                    DEVICE_REPLY_TYPE_e,
                                                    QString)),
                            Qt::QueuedConnection);
                    status = true;
                }
            }
            else
            {
                DPRINT(STREAM_REQ,"LiveMedia not created for streamId [%d]",m_streamId);
            }
            break;

        case LIVE_ANALOG_REQUEST:
            if(m_analogMedia == NULL)
            {
                /* PARASOFT: Memory Deallocated in delete Media Request */
                m_analogMedia = new AnalogMedia(serverInfo,
                                                requestInfo,
                                                commandId,
                                                m_channelIndex);
                if(IS_VALID_OBJ(m_analogMedia))
                {
                    connect(m_analogMedia,
                            SIGNAL(sigAnalogMediaResponse(REQ_MSG_ID_e,
                                                          SET_COMMAND_e,
                                                          DEVICE_REPLY_TYPE_e,
                                                          QString)),
                            this,
                            SLOT(slotStreamResponse(REQ_MSG_ID_e,
                                                    SET_COMMAND_e,
                                                    DEVICE_REPLY_TYPE_e,
                                                    QString)),
                            Qt::QueuedConnection);
                    status = true;
                }
            }
            else
            {
                DPRINT(STREAM_REQ,"AnalogMedia not created for streamId [%d]",m_streamId);
            }
            break;

        case PLAYBACK_STREAM_REQUEST:

            if(IS_VALID_OBJ(m_playbackMedia))
            {
                deleteMediaRequest ();
                m_reRequest = true;
            }
            else
            {
                m_reRequest = false;
            }

            if(m_playbackMedia == NULL)
            {
                /* PARASOFT: Memory Deallocated in delete Media Request */
                m_playbackMedia = new PlaybackMedia(serverInfo,
                                                    requestInfo,
                                                    commandId,
                                                    m_playbackId,
                                                    m_decoderId);
                if(IS_VALID_OBJ(m_playbackMedia))
                {
                    connect(m_playbackMedia,
                            SIGNAL(sigMediaResponse(REQ_MSG_ID_e,
                                                    SET_COMMAND_e,
                                                    DEVICE_REPLY_TYPE_e,
                                                    QString)),
                            this,
                            SLOT(slotStreamResponse(REQ_MSG_ID_e,
                                                    SET_COMMAND_e,
                                                    DEVICE_REPLY_TYPE_e,
                                                    QString)),
                            Qt::QueuedConnection);
                    status = true;
                }
            }
            else
            {
                 DPRINT(STREAM_REQ,"PlaybackMedia not created for streamId [%d]",m_streamId);
            }
            break;

            case SYNC_PLAYBACK_REQUEST:
            {
                if(IS_VALID_OBJ(m_syncPlaybackMedia))
                {
                    DPRINT(STREAM_REQ, "SyncPbMedia is already created: [streamId=%d]", m_streamId);
                    break;
                }

                quint8 syncChIdSet[MAX_SYNC_PB_SESSION];
                for(quint8 index = 0; index < MAX_SYNC_PB_SESSION; index++)
                {
                    syncChIdSet[index] = Layout::getCameraId(index);
                }

                /* PARASOFT: Memory Deallocated in delete Media Request */
                m_syncPlaybackMedia = new SyncPbMedia(serverInfo,
                                                      requestInfo,
                                                      commandId,
                                                      INVALID_DEC_DISP_PLAY_ID,
                                                      m_syncDecoderIdSet,
                                                      syncChIdSet);
                connect(m_syncPlaybackMedia,
                        SIGNAL(sigSyncPbResponse(REQ_MSG_ID_e,
                                                 SET_COMMAND_e,
                                                 DEVICE_REPLY_TYPE_e,
                                                 QString)),
                        this,
                        SLOT(slotStreamResponse(REQ_MSG_ID_e,
                                                SET_COMMAND_e,
                                                DEVICE_REPLY_TYPE_e,
                                                QString)),
                        Qt::QueuedConnection);
                status = true;
            }
            break;

        case INSTANT_PLAYBACK_STREAM_REQUEST:
            if(m_instantPlaybackMedia == NULL)
            {
                /* PARASOFT: Memory Deallocated in delete Media Request */
                m_instantPlaybackMedia = new InstantPlaybackMedia(serverInfo,
                                                                  requestInfo,
                                                                  commandId,
                                                                  m_decoderId);
                if(m_instantPlaybackMedia != NULL)
                {
                    connect(m_instantPlaybackMedia,
                            SIGNAL(sigMediaResponse(REQ_MSG_ID_e,
                                                    SET_COMMAND_e,
                                                    DEVICE_REPLY_TYPE_e,
                                                    QString)),
                            this,
                            SLOT(slotStreamResponse(REQ_MSG_ID_e,
                                                    SET_COMMAND_e,
                                                    DEVICE_REPLY_TYPE_e,
                                                    QString)));
                    status = true;
                }
            }
            break;

        case CLIENT_AUDIO_REQUEST:
            if(m_clientMedia == NULL)
            {
                /* PARASOFT: Memory Deallocated in delete Media Request */
                m_clientMedia = new ClientMedia(serverInfo,
                                            requestInfo,
                                            commandId,
                                            m_decoderId,
                                            m_streamId);

                if(IS_VALID_OBJ(m_clientMedia))
                {
                    connect(m_clientMedia,
                            SIGNAL(sigMediaResponse(REQ_MSG_ID_e,
                                                    SET_COMMAND_e,
                                                    DEVICE_REPLY_TYPE_e,
                                                    QString)),
                            this,
                            SLOT(slotStreamResponse(REQ_MSG_ID_e,
                                                    SET_COMMAND_e,
                                                    DEVICE_REPLY_TYPE_e,
                                                    QString)));
                    status = true;
                }
            }
            else
            {
                DPRINT(STREAM_REQ,"m_clientMedia not created for streamId [%d]",m_streamId);
            }
            break;

        default:
            status = false;
            break;
        }

        if(status == true)
        {
            DELETE_OBJ(streamRequestParam);
            setCurrentServerInfo(serverSessionInfo);
        }
        else
        {
            EPRINT(STREAM_REQ,"Create Media Request Failed [%d]",m_streamId);
        }
    }
    return status;
}
/* Create new media control object
 * 1. Prepare MSG_SET_SMD reuqest with Payload
 * 2. sigCommandResponse <=> slotStreamResponse
 * */
bool StreamRequest::createMediaControl(SERVER_SESSION_INFO_t serverSessionInfo,
                                       SET_COMMAND_e commandType,
                                       QString payloadString)
{
    bool status = false;

    SERVER_INFO_t   serverInfo;
    REQ_INFO_t      requestInfo;

    serverInfo.ipAddress = serverSessionInfo.serverInfo.ipAddress;
    serverInfo.tcpPort = serverSessionInfo.serverInfo.tcpPort;

    requestInfo.bytePayload = NULL;
    requestInfo.payload = payloadString;
    requestInfo.requestId = MSG_SET_CMD;
    requestInfo.sessionId = serverSessionInfo.sessionInfo.sessionId;
    requestInfo.timeout = serverSessionInfo.sessionInfo.timeout;
    requestInfo.windowId = MAX_WIN_ID;

    if(m_mediaControl == NULL)
    {
        /* PARASOFT: Memory Deallocated in delete Media Control */
        m_mediaControl = new CommandRequest(serverInfo,
                                            requestInfo,
                                            commandType,
                                            STRM_RELAT_CMD_SES_ID);

        if(IS_VALID_OBJ(m_mediaControl))
        {
            status = true;

            connect(m_mediaControl,
                    SIGNAL(sigCommandResponse(REQ_MSG_ID_e,
                                              SET_COMMAND_e,
                                              DEVICE_REPLY_TYPE_e,
                                              QString,
                                              quint8)),
                    this,
                    SLOT(slotStreamResponse(REQ_MSG_ID_e,
                                            SET_COMMAND_e,
                                            DEVICE_REPLY_TYPE_e,
                                            QString)),
                    Qt::QueuedConnection);
        }
        else
        {
            EPRINT(STREAM_REQ,"Create Media Control Failed [%d]",m_streamId);
        }
    }
    else
    {
        EPRINT(STREAM_REQ,"MediaControl not created for streamId [%d]",m_streamId);
    }

    return status;
}

void StreamRequest::deleteMediaRequest()
{

    switch(m_streamType)
    {
    case LIVE_STREAM_REQUEST:
        if(IS_VALID_OBJ(m_liveMedia))
        {
            m_liveMedia->setRunFlag(false);
            m_liveMedia->setStopFlag (true);
            if(m_liveMedia->isRunning())
            {
                m_liveMedia->wait();
            }

            disconnect(m_liveMedia,
                       SIGNAL(sigMediaResponse(REQ_MSG_ID_e,
                                               SET_COMMAND_e,
                                               DEVICE_REPLY_TYPE_e,
                                               QString)),
                       this,
                       SLOT(slotStreamResponse(REQ_MSG_ID_e,
                                               SET_COMMAND_e,
                                               DEVICE_REPLY_TYPE_e,
                                               QString)));

            DELETE_OBJ(m_liveMedia);

        }
        break;

    case LIVE_ANALOG_REQUEST:
        if(IS_VALID_OBJ(m_analogMedia))
        {
            m_analogMedia->setRunFlag(false);
            if(m_analogMedia->isRunning())
            {
                m_analogMedia->wait();
            }

            disconnect(m_analogMedia,
                       SIGNAL(sigAnalogMediaResponse(REQ_MSG_ID_e,
                                                     SET_COMMAND_e,
                                                     DEVICE_REPLY_TYPE_e,
                                                     QString)),
                       this,
                       SLOT(slotStreamResponse(REQ_MSG_ID_e,
                                               SET_COMMAND_e,
                                               DEVICE_REPLY_TYPE_e,
                                               QString)));

            DELETE_OBJ(m_analogMedia);

        }
        break;

    case PLAYBACK_STREAM_REQUEST:
        if(IS_VALID_OBJ(m_playbackMedia))
        {
            m_playbackMedia->setStopFlag (true);
            m_playbackMedia->setRunFlag(false);
            m_playbackMedia->setPauseFlag(false);

            if(m_playbackMedia->isRunning())
            {
                m_playbackMedia->wait();
            }

            if(m_playbackMedia != NULL)
            {
                disconnect(m_playbackMedia,
                           SIGNAL(sigMediaResponse(REQ_MSG_ID_e,
                                                   SET_COMMAND_e,
                                                   DEVICE_REPLY_TYPE_e,
                                                   QString)),
                           this,
                           SLOT(slotStreamResponse(REQ_MSG_ID_e,
                                                   SET_COMMAND_e,
                                                   DEVICE_REPLY_TYPE_e,
                                                   QString)));
                m_playbackMedia->deleteLater();
                m_playbackMedia = NULL;
            }
        }
        break;

    case SYNC_PLAYBACK_REQUEST:
        if(IS_VALID_OBJ(m_syncPlaybackMedia))
        {
            m_syncPlaybackMedia->setRunFlag(false);
            m_syncPlaybackMedia->sigToRecvPauseFalse();

            if(m_syncPlaybackMedia->isRunning())
            {
                m_syncPlaybackMedia->wait();
            }
            disconnect(m_syncPlaybackMedia,
                       SIGNAL(sigSyncPbResponse(REQ_MSG_ID_e,
                                                SET_COMMAND_e,
                                                DEVICE_REPLY_TYPE_e,
                                                QString)),
                       this,
                       SLOT(slotStreamResponse(REQ_MSG_ID_e,
                                               SET_COMMAND_e,
                                               DEVICE_REPLY_TYPE_e,
                                               QString)));
            DELETE_OBJ(m_syncPlaybackMedia);
        }
        break;

    case INSTANT_PLAYBACK_STREAM_REQUEST:
        if(m_instantPlaybackMedia != NULL)
        {
            m_instantPlaybackMedia->setRunFlag(false);
            m_instantPlaybackMedia->sigToRecvPauseFalse();
            if(m_instantPlaybackMedia->isRunning())
            {
                m_instantPlaybackMedia->wait();
            }

            disconnect(m_instantPlaybackMedia,
                       SIGNAL(sigMediaResponse(REQ_MSG_ID_e,
                                               SET_COMMAND_e,
                                               DEVICE_REPLY_TYPE_e,
                                               QString)),
                       this,
                       SLOT(slotStreamResponse(REQ_MSG_ID_e,
                                               SET_COMMAND_e,
                                               DEVICE_REPLY_TYPE_e,
                                               QString)));
            DELETE_OBJ(m_instantPlaybackMedia);
        }
        break;

    case CLIENT_AUDIO_REQUEST:
        if(IS_VALID_OBJ(m_clientMedia))
        {
            m_clientMedia->setRunFlag(false);
            m_clientMedia->setStopFlag (true);
            if(m_clientMedia->isRunning())
            {
                m_clientMedia->wait();
            }

            disconnect(m_clientMedia,
                       SIGNAL(sigMediaResponse(REQ_MSG_ID_e,
                                               SET_COMMAND_e,
                                               DEVICE_REPLY_TYPE_e,
                                               QString)),
                       this,
                       SLOT(slotStreamResponse(REQ_MSG_ID_e,
                                               SET_COMMAND_e,
                                               DEVICE_REPLY_TYPE_e,
                                               QString)));

            DELETE_OBJ(m_clientMedia);

            DPRINT(STREAM_REQ,"ClientMedia deleted [streamId %d]",m_streamId);
        }
        else
        {
            DPRINT(STREAM_REQ,"ClientMedia already deleted for streamId [%d]",m_streamId);
        }
        break;

    default:
        break;
    }
}

void StreamRequest::deleteMediaControl()
{
    if(IS_VALID_OBJ(m_mediaControl))
    {
        while((m_mediaControl->isRunning()) == true)
        {
            m_mediaControl->wait(750);
        }

        disconnect(m_mediaControl,
                   SIGNAL(sigCommandResponse(REQ_MSG_ID_e,
                                             SET_COMMAND_e,
                                             DEVICE_REPLY_TYPE_e,
                                             QString,
                                             quint8)),
                   this,
                   SLOT(slotStreamResponse(REQ_MSG_ID_e,
                                           SET_COMMAND_e,
                                           DEVICE_REPLY_TYPE_e,
                                           QString)));
        DELETE_OBJ(m_mediaControl);
    }
}

void StreamRequest::replaceMediaRequest()
{

    switch(m_streamType)
    {
    case LIVE_STREAM_REQUEST:
        if(m_liveMedia != NULL)
        {
            m_liveMedia->setRunFlag(false);
            m_liveMedia->setStopFlag (true);
            m_liveMedia->wait();

            DPRINT(STREAM_REQ, "LiveMedia replaced with streamId [%d]", m_streamId);
        }
        break;

    case LIVE_ANALOG_REQUEST:
        if(m_analogMedia != NULL)
        {
            m_analogMedia->setRunFlag(false);
            m_analogMedia->wait();

            DPRINT(STREAM_REQ, "AnalogMedia replaced with streamId [%d]", m_streamId);
        }
        break;

    default:
        break;
    }
}

/* Change Live Stream from MAIN->SUB or vice versa
 * */
bool StreamRequest::changeStreamType(StreamRequestParam *streamRequestParam)
{
    bool status = false;
    SERVER_SESSION_INFO_t serverSessionInfo;
    getCurrentServerInfo(serverSessionInfo);

    switch(m_streamType)
    {
    case LIVE_STREAM_REQUEST:
        status = createMediaControl(serverSessionInfo,
                                    CNG_LV_STRM,
                                    streamRequestParam->payload);

        if(status == true)
        {
            DELETE_OBJ(streamRequestParam);
            m_mediaControl->start();
        }
        break;

    case LIVE_ANALOG_REQUEST:
        status = true;
        emit sigStreamRequestResponse(CHANGE_STREAM_TYPE_COMMAND,
                                      streamRequestParam,
                                      CMD_SUCCESS);
        break;

    default:
        break;
    }

    return status;
}

bool StreamRequest::includeAudio(StreamRequestParam *streamRequestParam)
{
    bool status = false;
    SERVER_SESSION_INFO_t serverSessionInfo;
    getCurrentServerInfo(serverSessionInfo);
    QString payloadString, deviceResponseString;
    DEVICE_REPLY_TYPE_e deviceResponse = CMD_INTERNAL_RESOURCE_LIMIT;
    QStringList responseList;
    bool videoAudioPriv;
    bool audioStatus;

    switch(m_streamType)
    {
    case LIVE_STREAM_REQUEST:
        status = createMediaControl(serverSessionInfo,
                                    INC_LV_AUD,
                                    streamRequestParam->payload);

        if(status == true)
        {
            DELETE_OBJ(streamRequestParam);
            m_mediaControl->start();
        }
        break;

    case LIVE_ANALOG_REQUEST:
        payloadString = QString("%1").arg(m_channelIndex);
        payloadString.append(FSP);
        status = createMediaControl(serverSessionInfo,
                                    CHK_PRIV,
                                    payloadString);

        if(status == true)
        {
            m_mediaControl->getBlockingRes(deviceResponseString, deviceResponse);
            deleteMediaControl();

            if(deviceResponse == CMD_SUCCESS)
            {
                responseList = deviceResponseString.split(FSP);

                videoAudioPriv = (bool)responseList.at(2).toUInt();
                audioStatus = (bool)responseList.at(3).toUInt();

                if(videoAudioPriv == false)
                {
                    deviceResponse = CMD_NO_PRIVILEGE;
                }
                else if(audioStatus == false)
                {
                    deviceResponse = CMD_AUDIO_DISABLED;
                }
                else
                {
                    status = IncludeAudio(0);
                    if(status == false)
                    {
                        deviceResponse = CMD_DECODER_ERROR;
                    }
                }
            }

            status = true;
            emit sigStreamRequestResponse(INCLUDE_AUDIO_COMMAND,
                                          streamRequestParam,
                                          deviceResponse);
        }
        break;

    default:
        break;
    }

    if (status == false)
    {
        EPRINT(STREAM_REQ, "INCLUDE_AUDIO_COMMAND failed to send request");
    }

    return status;
}

bool StreamRequest::excludeAudio(StreamRequestParam *streamRequestParam)
{
    bool status = false;
    SERVER_SESSION_INFO_t serverSessionInfo;
    getCurrentServerInfo(serverSessionInfo);

    switch(m_streamType)
    {
    case LIVE_STREAM_REQUEST:
        status = createMediaControl(serverSessionInfo,
                                    EXC_LV_AUD,
                                    streamRequestParam->payload);

        if(status == true)
        {
            DELETE_OBJ(streamRequestParam);
            m_mediaControl->start();
        }
        break;

    case LIVE_ANALOG_REQUEST:

        if(status == false)
        {
            emit sigStreamRequestResponse(EXCLUDE_AUDIO_COMMAND,
                                          streamRequestParam,
                                          CMD_DECODER_ERROR);
        }
        else
        {
            emit sigStreamRequestResponse(EXCLUDE_AUDIO_COMMAND,
                                          streamRequestParam,
                                          CMD_SUCCESS);
        }
        status = true;
        break;

    default:
        break;
    }

    if (status == false)
    {
        EPRINT(STREAM_REQ, "EXCLUDE_AUDIO_COMMAND failed to send request");
    }

    return status;
}

void StreamRequest::excludeAudio()
{
    if(getAudioStreamIndex () == m_streamId)
    {
        ExcludeAudio (getAudioIndex ());
        setAudioStreamIndex(MAX_STREAM_SESSION);
    }
}

/**
 * @brief create media request for sending to server based to 'audio in' status.
          create object of AudioIn and start audioin thread. 			
 * @param streamRequestParam 
 * @return
 */
bool StreamRequest::includeExculdeAudioIn(StreamRequestParam *streamRequestParam)
{
    bool status = false;
    SERVER_SESSION_INFO_t   serverSessionInfo;
    REQ_INFO_t              requestInfo;
    SERVER_INFO_t           serverInfo;

    /* get server information */
    getCurrentServerInfo(serverSessionInfo);

    /*  if request is for include audio in */
    if (streamRequestParam->audioStatus == true)
    {
        DPRINT(STREAM_REQ, "AudioIn: SND_AUDIO request to device");

        if (m_audioIn == NULL)
        {
            /* set server info */
            serverInfo.ipAddress = serverSessionInfo.serverInfo.ipAddress;
            serverInfo.tcpPort = serverSessionInfo.serverInfo.tcpPort;

            /* set request info */
            requestInfo.bytePayload = NULL;
            requestInfo.payload = streamRequestParam->payload;
            requestInfo.requestId = MSG_SET_CMD;
            requestInfo.sessionId = serverSessionInfo.sessionInfo.sessionId;
            requestInfo.timeout = serverSessionInfo.sessionInfo.timeout;

            /* PARASOFT: Memory Deallocated in STP_AUDIO command */
            m_audioIn = new AudioIn(serverInfo, requestInfo, SND_AUDIO);

            if (IS_VALID_OBJ(m_audioIn))
            {
                connect(m_audioIn,
                        SIGNAL(sigMediaResponse(REQ_MSG_ID_e,
                                                SET_COMMAND_e,
                                                DEVICE_REPLY_TYPE_e,
                                                QString)),
                        this,
                        SLOT(slotStreamResponse(REQ_MSG_ID_e,
                                                SET_COMMAND_e,
                                                DEVICE_REPLY_TYPE_e,
                                                QString)));
                status = true;
            }
        }
        else
        {
            EPRINT(STREAM_REQ, "AudioIn: failed to create m_audioIn in object");
        }

        if(status == true)
        {
            /* delete stream request param */
            DELETE_OBJ(streamRequestParam);
            m_audioIn->start();
  			DPRINT(STREAM_REQ, "AudioIn: audio in thread started");
        }
        else
        {			
            emit sigStreamRequestResponse(INCL_EXCL_AUDIO_IN_COMMAND,
                                          streamRequestParam,
                                          CMD_AUD_SND_REQ_FAIL);
        }
    }
    else /*  if request is for exclude audio in */
    {
		
		DPRINT(LAYOUT, "AudioIn: STP_AUDIO request to device");
        status = createMediaControl(serverSessionInfo,
                                    STP_AUDIO,
                                    streamRequestParam->payload);
        if(status == true)
        {
            /* delete stream request param */
            DELETE_OBJ(streamRequestParam);
            m_mediaControl->start();
        }
        else
        {
            emit sigStreamRequestResponse(INCL_EXCL_AUDIO_IN_COMMAND,
                                          streamRequestParam,
                                          CMD_REQUEST_NOT_PROCESSED);
        }

        /* delete audioin in object */
        if (IS_VALID_OBJ(m_audioIn))
        {
            m_audioIn->setRunFlag(false);

            if (m_audioIn->isRunning())
            {
                m_audioIn->wait();
                DPRINT(STREAM_REQ, "AudioIn: audio in thread exited");
            }

            disconnect(m_audioIn,
                       SIGNAL(sigMediaResponse(REQ_MSG_ID_e,
                                               SET_COMMAND_e,
                                               DEVICE_REPLY_TYPE_e,
                                               QString)),
                       this,
                       SLOT(slotStreamResponse(REQ_MSG_ID_e,
                                               SET_COMMAND_e,
                                               DEVICE_REPLY_TYPE_e,
                                               QString)));

            DELETE_OBJ(m_audioIn);            
        }        
    }

    return status;
}

void StreamRequest::forceStopAudioIn()
{
    /* delete audioin in object */
    if (IS_VALID_OBJ(m_audioIn))
    {
        m_audioIn->setRunFlag(false);

        if (m_audioIn->isRunning())
        {
            m_audioIn->wait();
            DPRINT(STREAM_REQ, "AudioIn: audio in thread exited");
        }

        disconnect(m_audioIn,
                   SIGNAL(sigMediaResponse(REQ_MSG_ID_e,
                                           SET_COMMAND_e,
                                           DEVICE_REPLY_TYPE_e,
                                           QString)),
                   this,
                   SLOT(slotStreamResponse(REQ_MSG_ID_e,
                                           SET_COMMAND_e,
                                           DEVICE_REPLY_TYPE_e,
                                           QString)));

        DELETE_OBJ(m_audioIn);
    }    
}

void StreamRequest::getPlaybackId(SERVER_SESSION_INFO_t serverInfo,
                                  StreamRequestParam *streamRequestParam)
{
    bool status = false;
    bool isDecoderError;
    QString payloadString = streamRequestParam->payload;
    StreamRequestParam *replyParam=NULL;

    if(!getDeletionProcessFlag ())
    {
        status = createNewStream(serverInfo,
                                 streamRequestParam,
                                 isDecoderError,
                                 false);
        if(status == true)
        {
            status = createMediaControl(serverInfo,
                                        GET_PLY_STRM_ID,
                                        payloadString);
            if(status == true)
            {
                m_mediaControl->start();
            }
            else
            {
                replyParam = new StreamRequestParam();

                if(IS_VALID_OBJ(replyParam))
                {
                    if(m_displayType < MAX_DISPLAY_TYPE)
                    {
                        m_streamParamMutex.lock ();
                        replyParam->streamId = m_streamId;
                        replyParam->windowId = m_windowId[m_displayType];
                        replyParam->actualWindowId = m_actualWindowId[m_displayType];
                        replyParam->channelId = m_channelIndex;
                        replyParam->deviceName = m_deviceName;
                        replyParam->displayType = m_displayType;
                        replyParam->streamRequestType = m_streamType;
                        replyParam->liveStreamType = m_liveStreamType;
                        replyParam->timeStamp = m_timeStamp[m_displayType];
                        m_streamParamMutex.unlock ();
                    }

                    emit sigStreamRequestResponse(GET_PLAYBACK_ID_COMMAND,
                                                  replyParam,
                                                  CMD_INTERNAL_RESOURCE_LIMIT);
                    setDeletionProcessFlag (true);
                    freeStreamRequest();
                    emit sigDeleteStreamRequest(m_streamId);
                }
            }
        }
        else if(isDecoderError == true)
        {
            emit sigStreamRequestResponse(GET_PLAYBACK_ID_COMMAND,
                                          streamRequestParam,
                                          CMD_DECODER_ERROR);
            setDeletionProcessFlag (true);
            freeStreamRequest();
            emit sigDeleteStreamRequest(m_streamId);
        }
        else
        {
            emit sigStreamRequestResponse(GET_PLAYBACK_ID_COMMAND,
                                          streamRequestParam,
                                          CMD_INTERNAL_RESOURCE_LIMIT);
            setDeletionProcessFlag (true);
            freeStreamRequest();
            emit sigDeleteStreamRequest(m_streamId);
        }
    }
}

void StreamRequest::getPlaybackIdForInstatPlayback(SERVER_SESSION_INFO_t serverInfo,
                                                   StreamRequestParam *streamRequestParam)
{
    bool status = false;
    bool isDecoderError;

    if(!getDeletionProcessFlag ())
    {
        status = createNewStream(serverInfo,
                                 streamRequestParam,
                                 isDecoderError);
        if(status == true)
        {
            startMedia();
        }
        else if(isDecoderError == true)
        {
            emit sigStreamRequestResponse(START_INSTANTPLAYBACK_COMMAND,
                                          streamRequestParam,
                                          CMD_DECODER_ERROR);
            setDeletionProcessFlag (true);
            freeStreamRequest();
            emit sigDeleteStreamRequest(m_streamId);
        }
        else
        {
            emit sigStreamRequestResponse(START_INSTANTPLAYBACK_COMMAND,
                                          streamRequestParam,
                                          CMD_INTERNAL_RESOURCE_LIMIT);
            setDeletionProcessFlag (true);
            freeStreamRequest();
            emit sigDeleteStreamRequest(m_streamId);
        }
    }
}

void StreamRequest::setAudioAction(STREAM_COMMAND_TYPE_e streamCommandType,
                                   StreamRequestParam *streamRequestParam)
{
    quint8 decoderId = INVALID_DEC_DISP_PLAY_ID, windowIndex;
    quint8 status = FAIL;
    DEVICE_REPLY_TYPE_e deviceResponse = CMD_INTERNAL_RESOURCE_LIMIT;
    quint8 winId = m_decoderId;
	quint8 tryCount = 0;

    if(streamRequestParam->audioStatus == true)
    {
        if(m_streamType == SYNC_PLAYBACK_REQUEST)
        {
            if(m_syncPlaybackMedia != NULL)
            {
                m_syncPlaybackMedia->GetDecIdForCh(streamRequestParam->channelId,
                                                   decoderId,
                                                   windowIndex);
                winId = decoderId;
            }

            if(getAudioIndex () != winId)
            {
                excludeAudio ();
            }
        }
        if(m_streamType == INSTANT_PLAYBACK_STREAM_REQUEST)
        {
            if(GetCurrentAudioChannel () != INVALID_DEC_DISP_PLAY_ID)
            {
                ExcludeAudio (GetCurrentAudioChannel ());
            }
        }

        do
        {
            status = IncludeAudio(winId);
            if(status != SUCCESS)
            {
                usleep(5000);
            }
            else
            {
                break;
            }
        }while(tryCount++ <= 3);

        if(status == SUCCESS)
        {
            setAudioIndex (winId);
            setAudioStreamIndex(m_streamId);
        }

      emit sigChangeAudState();
    }
    else
    {
        excludeAudio ();
        status = SUCCESS;
    }

    if(status == SUCCESS)
    {
        deviceResponse = CMD_SUCCESS;
    }

    emit sigStreamRequestResponse(streamCommandType,
                                  streamRequestParam,
                                  deviceResponse);
}

void StreamRequest::setSyncPlaybackAction(STREAM_COMMAND_TYPE_e streamCommandType,
                                          StreamRequestParam *streamRequestParam)
{
    bool status = false;
    SET_COMMAND_e commandId = MAX_NET_COMMAND;

    if(m_syncPlaybackMedia != NULL)
    {
        switch(streamCommandType)
        {
        case SYNCPLAY_SYNCPLABACK_STREAM_COMMAND:
            commandId = SYNC_PLYBCK_RCD;
            break;

		case SET_SPEED_SYNCPLABACK_STREAM_COMMAND:
			commandId = SYNC_PLYBCK_SPEED;
			break;

        case STEPFORWARD_SYNCPLABACK_STREAM_COMMAND:
            commandId = SYNC_STEP_FORWARD;
            break;

        case STEPBACKWARD_SYNCPLABACK_STREAM_COMMAND:
            commandId = SYNC_STEP_REVERSE;
            break;

        case PAUSE_SYNCPLABACK_STREAM_COMMAND:
            commandId = PAUSE_RCD;
            break;

        case STOP_SYNCPLABACK_STREAM_COMMAND:
            excludeAudio ();
            commandId = STP_RCD;
            break;

        case CLEAR_SYNCPLABACK_STREAM_COMMAND:
            excludeAudio ();
            commandId = CLR_RCD;
            break;

        default:
            break;
        }

        if(commandId != MAX_NET_COMMAND)
        {
            status = true;
            m_syncPlaybackMedia->setActionOnControlCmd(commandId,
                                                       streamRequestParam->payload);
        }
    }

    if(status == true)
    {
        DELETE_OBJ(streamRequestParam);
    }
    else
    {
        emit sigStreamRequestResponse(streamCommandType,
                                      streamRequestParam,
                                      CMD_INTERNAL_RESOURCE_LIMIT);
    }
}

void StreamRequest::sendInstatPlaybackAction(SERVER_SESSION_INFO_t,
                                             STREAM_COMMAND_TYPE_e streamCommandType,
                                             StreamRequestParam *streamRequestParam)
{
    bool status = false;
    SET_COMMAND_e commandId = MAX_NET_COMMAND;

    switch(streamCommandType)
    {
    case PAUSE_INSTANTPLAYBACK_COMMAND:
        commandId = PAUSE_INSTANT_PLY;
        break;

    case SEEK_INSTANTPLAYBACK_COMMAND:
        commandId = SEEK_INSTANT_PLY;
        break;

    case STOP_INSTANTPLAYBACK_COMMAND:
        excludeAudio ();
        commandId = STOP_INSTANT_PLY;
        break;

    default:
        break;
    }

    if(m_instantPlaybackMedia != NULL)
    {
        status = m_instantPlaybackMedia->setInstantPlaybackCommand(commandId,
                                                                   streamRequestParam->payload);
    }
    if(status == true)
    {
        DELETE_OBJ(streamRequestParam);
    }
    else
    {
        emit sigStreamRequestResponse(streamCommandType,
                                      streamRequestParam,
                                      CMD_RESOURCE_LIMIT);
    }
}

bool StreamRequest::getDeleteAfterMediaControlDeleteFlag()
{
    bool flag = false;

    m_deleteAfterMediaControlDeleteFlagAccess.lock();
    flag = m_deleteAfterMediaControlDeleteFlag;
    m_deleteAfterMediaControlDeleteFlagAccess.unlock();
    return flag;
}

void StreamRequest::setDeleteAfterMediaControlDeleteFlag(bool flag)
{
    m_deleteAfterMediaControlDeleteFlagAccess.lock();
    m_deleteAfterMediaControlDeleteFlag = flag;
    m_deleteAfterMediaControlDeleteFlagAccess.unlock();
}


void StreamRequest::getCurrentServerInfo(SERVER_SESSION_INFO_t &info)
{
    m_serverInfoAccess.lock();
    info.serverInfo.ipAddress = m_currentServerInfo.serverInfo.ipAddress;
    info.serverInfo.tcpPort = m_currentServerInfo.serverInfo.tcpPort;
    info.sessionInfo.sessionId = m_currentServerInfo.sessionInfo.sessionId;
    info.sessionInfo.timeout = m_currentServerInfo.sessionInfo.timeout;
   m_serverInfoAccess.unlock();
}

void StreamRequest::setCurrentServerInfo(SERVER_SESSION_INFO_t info)
{
    m_serverInfoAccess.lock();
    m_currentServerInfo.serverInfo.ipAddress = info.serverInfo.ipAddress;
    m_currentServerInfo.serverInfo.tcpPort = info.serverInfo.tcpPort;
    m_currentServerInfo.sessionInfo.sessionId = info.sessionInfo.sessionId;
    m_currentServerInfo.sessionInfo.timeout = info.sessionInfo.timeout;
    m_serverInfoAccess.unlock();
}

bool StreamRequest::getMediaPauseForReplaceFlag()
{
    bool flag = false;
    m_mediaPauseForReplaceFlagAccess.lock();
    flag = m_mediaPauseForReplaceFlag;
    m_mediaPauseForReplaceFlagAccess.unlock();

    return flag;
}

void StreamRequest::setMediaPauseForReplaceFlag(bool flag)
{
    m_mediaPauseForReplaceFlagAccess.lock();
    m_mediaPauseForReplaceFlag = flag;
    m_mediaPauseForReplaceFlagAccess.unlock();
}

bool StreamRequest::getMediaRequestStopFlag ()
{
    bool flag = false;
    if(m_analogMedia != NULL)
    {
        flag = m_analogMedia->getStopFlag ();
    }
    else if(m_liveMedia != NULL)
    {
        flag = m_liveMedia->getStopFlag();
    }
    else if(m_playbackMedia != NULL)
    {
        flag = m_playbackMedia->getStopFlag();
    }
    return flag;
}

bool StreamRequest::isMediaControlActive()
{
    return ((m_mediaControl == NULL) ? false : true);
}

bool StreamRequest::isQueueEmpty()
{
    bool status=false;
    quint8 streamId;
    m_streamParamMutex.lock ();
    streamId = m_streamId;
    m_streamParamMutex.unlock ();
    m_queueLock[streamId].lock();
    status = (m_queueReadIndex[streamId] == m_queueWriteIndex[streamId]);
    m_queueLock[streamId].unlock();

    return status;
}

bool StreamRequest::isQueueFull()
{
    bool status=false;
    quint8 streamId;
    m_streamParamMutex.lock ();
    streamId = m_streamId;
    m_streamParamMutex.unlock ();
    m_queueLock[streamId].lock();
    status = (m_queueWriteIndex[streamId] == (MAX_STREAM_COMMAND_REQUEST - 1));
    m_queueLock[streamId].unlock();

    return status;
}

bool StreamRequest::putRequestInQueue(STREAM_REQUEST_QUEUE_t &request)
{
    EPRINT(STREAM_REQ,"PutinQueue as occured");
    bool status = false;
    quint8 streamId;
    if(isQueueFull() == false)
    {
        status = true;
        m_streamParamMutex.lock ();
        streamId = m_streamId;
        m_streamParamMutex.unlock ();

        if(streamId >= MAX_STREAM_SESSION)
        {
            EPRINT(STREAM_REQ,"streamId Not Valid");
            return status;
        }

        m_queueLock[streamId].lock ();
        memcpy(&m_requestQueue[streamId][m_queueWriteIndex[streamId]],
               &request,
               sizeof(STREAM_REQUEST_QUEUE_t));
        m_queueWriteIndex[streamId]++;
        m_queueLock[streamId].unlock ();
    }
    else
    {
        EPRINT(STREAM_REQ, "Queue is full");
    }
    return status;
}


bool StreamRequest::getRequestFromQueue(STREAM_REQUEST_QUEUE_t *request)
{
    bool status = false;
    quint8 streamId;
    if(isQueueEmpty() == false)
    {
        status = true;
        m_streamParamMutex.lock ();
        streamId = m_streamId;
        m_streamParamMutex.unlock ();

        if(streamId >= MAX_STREAM_SESSION)
        {
            EPRINT(STREAM_REQ,"streamId Not Valid");
            return status;
        }

        m_queueLock[streamId].lock ();
        memcpy(request,
               &m_requestQueue[streamId][m_queueReadIndex[streamId]],
               sizeof(STREAM_REQUEST_QUEUE_t));
        m_queueReadIndex[streamId]++;
        m_queueLock[streamId].unlock ();
    }
    return status;
}

/*
 * Slot for processing Stream reuqests
 * Signal emitted by processRequest()
 * Queued Connection with sigProcessRequest()
 * */
void StreamRequest::slotProcessRequest(STREAM_COMMAND_TYPE_e streamCommandType,
                                       SERVER_SESSION_INFO_t serverInfo,
                                       StreamRequestParam *streamRequestParam)
{

    bool status = false;
    bool isDecoderError = false;
    DEVICE_REPLY_TYPE_e deviceResponse = CMD_INTERNAL_RESOURCE_LIMIT;
    SERVER_SESSION_INFO_t serverSessionInfo;

    SET_COMMAND_e commandId;
    bool sameReq;

    if(!getDeletionProcessFlag ())
    {
        switch(streamCommandType)
        {
        case START_STREAM_COMMAND:
            m_streamParamMutex.lock ();
            if(m_deviceName == "")
            {
                m_streamParamMutex.unlock ();

                // 1)Assign Free Decoder channel
                // 2)Create new liveMedia Object for given Stream Type
                status = createNewStream(serverInfo,
                                         streamRequestParam,
                                         isDecoderError);
                if(status == true)
                {
                    startMedia();
                }
                else if(isDecoderError == true)
                {
                    DPRINT(STREAM_REQ, "deviceName [%s] CameraIndex [%d] Response statusId [%d] actualWindowId [%d] isDecoderError [%d] streamCommandType[%d]",
                           m_deviceName.toUtf8().constData(), m_channelIndex, m_streamId, m_actualWindowId[m_displayType], isDecoderError, streamCommandType);

                    emit sigStreamRequestResponse(streamCommandType,
                                                  streamRequestParam,
                                                  CMD_DECODER_ERROR);
                    setDeletionProcessFlag (true);
                    freeStreamRequest();
                    emit sigDeleteStreamRequest(m_streamId);
                }
                else
                {
                    DPRINT(STREAM_REQ, "deviceName [%s] CameraIndex [%d] Response statusId [%d] actualWindowId [%d] isDecoderError [%d] streamCommandType[%d]",
                           m_deviceName.toUtf8().constData(), m_channelIndex, m_streamId, m_actualWindowId[m_displayType], isDecoderError, streamCommandType);

                    emit sigStreamRequestResponse(streamCommandType,
                                                  streamRequestParam,
                                                  deviceResponse);
                    setDeletionProcessFlag (true);
                    freeStreamRequest();
                    emit sigDeleteStreamRequest(m_streamId);
                }
            }
            else
            {
                if((streamRequestParam->deviceName == m_deviceName) &&
                        (streamRequestParam->channelId == m_channelIndex))
                {
                    if(((streamRequestParam->displayType == m_displayType)
                        && (streamRequestParam->actualWindowId == m_actualWindowId[m_displayType]))
                            || ((streamRequestParam->displayType == m_duplicateDisplayType)
                                && (streamRequestParam->actualWindowId == m_actualWindowId[m_duplicateDisplayType])))
                    {
                        m_streamParamMutex.unlock ();

                        DPRINT(STREAM_REQ, "Already duplicate deviceName [%s] cameraIndex [%d] streamId [%d]  streamStatus [%d] actualWindowId [%d]",
                               m_deviceName.toUtf8().constData(), m_channelIndex, m_streamId, m_streamStatus, m_actualWindowId[m_displayType]);

                        streamRequestParam->streamId = m_streamId;

                        if(m_streamStatus == STREAM_RUN)
                        {
                            emit sigStreamRequestResponse(START_STREAM_COMMAND,
                                                          streamRequestParam,
                                                          CMD_STREAM_NO_VIDEO_LOSS);
                        }
                        else if(m_streamStatus == STREAM_VL)
                        {
                            emit sigStreamRequestResponse(START_STREAM_COMMAND,
                                                          streamRequestParam,
                                                          CMD_STREAM_VIDEO_LOSS);
                        }
                        else
                        {
                            emit sigStreamRequestResponse(START_STREAM_COMMAND,
                                                          streamRequestParam,
                                                          CMD_INTERNAL_RESOURCE_LIMIT);
                        }
                    }
                    else
                    {
                        m_streamParamMutex.unlock ();
                        emit sigStreamRequestResponse(streamCommandType,
                                                      streamRequestParam,
                                                      CMD_MAX_DEVICE_REPLY);
                    }
                }
                else
                {
                    m_streamParamMutex.unlock ();
                    emit sigStreamRequestResponse(START_STREAM_COMMAND,
                                                  streamRequestParam,
                                                  CMD_INTERNAL_RESOURCE_LIMIT);
                }
            }
            break;

        case STOP_STREAM_COMMAND:
            if(getFreeFlag() == false)
            {
                setClearFlag(true);

                status = stopStream(streamRequestParam,true,false,&sameReq);
                if(status == true)
                {
                    excludeAudio();
                    forceStopAudioIn();
                    DELETE_OBJ(streamRequestParam);
                }
                else
                {
                    setClearFlag(false);
                    if(sameReq)
                    {
                        emit sigStreamRequestResponse(STOP_STREAM_COMMAND,
                                                      streamRequestParam,
                                                      CMD_SUCCESS);
                    }
                    else
                    {

                        STREAM_REQUEST_QUEUE_t streamRequest;

                        streamRequest.serverInfo.serverInfo.ipAddress = serverInfo.serverInfo.ipAddress;
                        streamRequest.serverInfo.serverInfo.tcpPort = serverInfo.serverInfo.tcpPort;
                        streamRequest.serverInfo.sessionInfo.sessionId = serverInfo.sessionInfo.sessionId;
                        streamRequest.serverInfo.sessionInfo.timeout = serverInfo.sessionInfo.timeout;
                        streamRequest.commandType = streamCommandType;
                        streamRequest.requestParam = streamRequestParam;

                        putRequestInQueue(streamRequest);
                    }
                }
            }
            else
            {
                emit sigStreamRequestResponse(STOP_STREAM_COMMAND,
                                              streamRequestParam,
                                              CMD_SUCCESS);
            }
            break;

        case CHANGE_STREAM_TYPE_COMMAND:
            status = changeStreamType(streamRequestParam);

            if(status == false)
            {
                emit sigStreamRequestResponse(streamCommandType,
                                              streamRequestParam,
                                              deviceResponse);
            }
            break;

        case INCLUDE_AUDIO_COMMAND:
            status = includeAudio(streamRequestParam);

            if(status == false)
            {
                emit sigStreamRequestResponse(streamCommandType,
                                              streamRequestParam,
                                              deviceResponse);
            }
            break;

        case EXCLUDE_AUDIO_COMMAND:
            status = excludeAudio(streamRequestParam);

            if(status == false)
            {
                emit sigStreamRequestResponse(streamCommandType,
                                              streamRequestParam,
                                              deviceResponse);
            }
            break;

		case INCL_EXCL_AUDIO_IN_COMMAND:
            includeExculdeAudioIn(streamRequestParam);
			break;

        case GET_PLAYBACK_ID_COMMAND:
            getPlaybackId(serverInfo, streamRequestParam);
            break;

        case PLAY_PLABACK_STREAM_COMMAND:
        case STEP_PLABACK_STREAM_COMMAND:
            if(streamCommandType == PLAY_PLABACK_STREAM_COMMAND)
            {
                commandId = PLY_RCD_STRM;
            }
            else
            {
                commandId = STEP_RCD_STRM;
            }

            if(getClearFlag ())
            {
                setClearFlag (false);
            }

            status = createMediaRequest(serverInfo,
                                        commandId,
                                        streamRequestParam);
            if(status == true)
            {
                startMedia(commandId);
            }
            else
            {
                emit sigStreamRequestResponse(streamCommandType,
                                              streamRequestParam,
                                              deviceResponse);
            }
            break;

        case STOP_PLABACK_STREAM_COMMAND:
            setClearFlag(true);

            status = stopStream(streamRequestParam,false,false,&sameReq);
            if(status == true)
            {
                DELETE_OBJ(streamRequestParam);
            }
            else
            {
                setClearFlag(false);
                if(sameReq)
                {
                    emit sigStreamRequestResponse(STOP_STREAM_COMMAND,
                                                  streamRequestParam,
                                                  CMD_SUCCESS);
                }
                else
                {

                    STREAM_REQUEST_QUEUE_t streamRequest;

                    streamRequest.serverInfo.serverInfo.ipAddress = serverInfo.serverInfo.ipAddress;
                    streamRequest.serverInfo.serverInfo.tcpPort = serverInfo.serverInfo.tcpPort;
                    streamRequest.serverInfo.sessionInfo.sessionId = serverInfo.sessionInfo.sessionId;
                    streamRequest.serverInfo.sessionInfo.timeout = serverInfo.sessionInfo.timeout;
                    streamRequest.commandType = streamCommandType;
                    streamRequest.requestParam = streamRequestParam;

                    putRequestInQueue(streamRequest);
                }
            }
            break;

        case AUDIO_PLABACK_STREAM_COMMAND:
        case AUDIO_SYNCPLABACK_STREAM_COMMAND:
        case AUDIO_INSTANTPLAYBACK_COMMAND:
            setAudioAction(streamCommandType, streamRequestParam);
            break;

        case CLEAR_PLAYBACK_ID_COMMAND:
            excludeAudio();
            getCurrentServerInfo(serverSessionInfo);
            status = createMediaControl(serverSessionInfo,
                                        CLR_PLY_STRM_ID,
                                        streamRequestParam->payload);
            if(status == true)
            {
                DELETE_OBJ(streamRequestParam);
                m_mediaControl->start();
            }
            else
            {
                STREAM_REQUEST_QUEUE_t streamRequest;

                streamRequest.serverInfo.serverInfo.ipAddress = serverInfo.serverInfo.ipAddress;
                streamRequest.serverInfo.serverInfo.tcpPort = serverInfo.serverInfo.tcpPort;
                streamRequest.serverInfo.sessionInfo.sessionId = serverInfo.sessionInfo.sessionId;
                streamRequest.serverInfo.sessionInfo.timeout = serverInfo.sessionInfo.timeout;
                streamRequest.commandType = streamCommandType;
                streamRequest.requestParam = streamRequestParam;
                putRequestInQueue(streamRequest);
            }
            break;

        case PLAY_SYNCPLABACK_STREAM_COMMAND:
            //Allocate free decoder channel and create media req
            status = createNewStream(serverInfo, streamRequestParam, isDecoderError);

            if(status == true)
            {
                //Run media req thread
                startMedia();
            }
            else
            {
                emit sigStreamRequestResponse(streamCommandType,
                                              streamRequestParam,
                                              deviceResponse);
                setDeletionProcessFlag (true);
                freeStreamRequest();
                emit sigDeleteStreamRequest(m_streamId);
            }
            break;

        case SYNCPLAY_SYNCPLABACK_STREAM_COMMAND:
		case SET_SPEED_SYNCPLABACK_STREAM_COMMAND:
        case STEPFORWARD_SYNCPLABACK_STREAM_COMMAND:
        case STEPBACKWARD_SYNCPLABACK_STREAM_COMMAND:
        case PAUSE_SYNCPLABACK_STREAM_COMMAND:
        case STOP_SYNCPLABACK_STREAM_COMMAND:
        case CLEAR_SYNCPLABACK_STREAM_COMMAND:
            setSyncPlaybackAction(streamCommandType, streamRequestParam);
            break;

        case START_INSTANTPLAYBACK_COMMAND:
            getPlaybackIdForInstatPlayback(serverInfo, streamRequestParam);
            break;

        case PAUSE_INSTANTPLAYBACK_COMMAND:
        case SEEK_INSTANTPLAYBACK_COMMAND:
        case STOP_INSTANTPLAYBACK_COMMAND:
            sendInstatPlaybackAction(serverInfo,
                                     streamCommandType,
                                     streamRequestParam);
            break;

        case STRT_CLNT_AUDIO_COMMAND:
            m_streamParamMutex.lock ();
            m_decoderId = CLIENT_AUDIO_DECODER_ID;
            m_streamType = CLIENT_AUDIO_REQUEST;
            m_displayType = MAIN_DISPLAY;
            m_streamParamMutex.unlock ();
            commandId = STRT_CLNT_AUDIO;
            status = createMediaRequest(serverInfo,
                                        commandId,
                                        streamRequestParam);
            if(status == true)
            {
                m_clientMedia->start();
            }
            else
            {
                emit sigStreamRequestResponse(streamCommandType,
                                              streamRequestParam,
                                              CMD_AUD_SND_REQ_FAIL);
            }
            break;

        case STOP_CLNT_AUDIO_COMMAND:
            commandId = STOP_CLNT_AUDIO;
            status = createMediaControl(serverSessionInfo,
                                        commandId,
                                        streamRequestParam->payload);
            if(status == true)
            {
                m_mediaControl->start();
            }
            else
            {
                emit sigStreamRequestResponse(streamCommandType,
                                              streamRequestParam,
                                              CMD_REQUEST_NOT_PROCESSED);
            }
            break;


        default:
            break;
        }
    }
    else
    {
        emit sigStreamRequestResponse(streamCommandType,
                                      streamRequestParam,
                                      CMD_MAX_DEVICE_REPLY);
    }
}

void StreamRequest::slotProcessRequest(STREAM_COMMAND_TYPE_e streamCommandType,
                                       SERVER_SESSION_INFO_t serverInfo,
                                       StreamRequestParam *streamRequestParam,
                                       SERVER_SESSION_INFO_t nextServerInfo,
                                       StreamRequestParam *nextStreamRequestParam)
{

    bool status = false;
    bool sameReq = false;

    if(!getDeletionProcessFlag ())
    {
        switch(streamCommandType)
        {
        case REPLACE_STREAM_COMMAND:
            setReplaceFlag(true);

            m_streamParamMutex.lock ();
            m_nextServerInfo.serverInfo.ipAddress = nextServerInfo.serverInfo.ipAddress;
            m_nextServerInfo.serverInfo.tcpPort = nextServerInfo.serverInfo.tcpPort;
            m_nextServerInfo.sessionInfo.sessionId = nextServerInfo.sessionInfo.sessionId;
            m_nextServerInfo.sessionInfo.timeout = nextServerInfo.sessionInfo.timeout;

            m_nextRequestParam = nextStreamRequestParam;
            m_streamParamMutex.unlock ();

            status = stopStream(streamRequestParam,false,false,&sameReq);
            if(status == true)
            {
                DELETE_OBJ(streamRequestParam);
            }
            else
            {
                setReplaceFlag(false);
                if(sameReq)
                {
                      m_nextRequestParam = NULL;
                      setReplaceFlag(false);
                      emit sigStreamRequestResponse(STOP_STREAM_COMMAND,
                                                    streamRequestParam,
                                                    CMD_SUCCESS);
                      emit sigStreamRequestResponse(START_STREAM_COMMAND,
                                                    nextStreamRequestParam,
                                                    CMD_MAX_DEVICE_REPLY);
                }
                else
                {

                    STREAM_REQUEST_QUEUE_t streamRequest;
                    streamRequest.serverInfo.serverInfo.ipAddress = serverInfo.serverInfo.ipAddress;
                    streamRequest.serverInfo.serverInfo.tcpPort = serverInfo.serverInfo.tcpPort;
                    streamRequest.serverInfo.sessionInfo.sessionId = serverInfo.sessionInfo.sessionId;
                    streamRequest.serverInfo.sessionInfo.timeout = serverInfo.sessionInfo.timeout;
                    streamRequest.commandType = streamCommandType;
                    streamRequest.requestParam = streamRequestParam;

                    streamRequest.nextServerInfo.serverInfo.ipAddress = nextServerInfo.serverInfo.ipAddress;
                    streamRequest.nextServerInfo.serverInfo.tcpPort =   nextServerInfo.serverInfo.tcpPort;
                    streamRequest.nextServerInfo.sessionInfo.sessionId = nextServerInfo.sessionInfo.sessionId;
                    streamRequest.nextServerInfo.sessionInfo.timeout =   nextServerInfo.sessionInfo.timeout;
                    streamRequest.nextRequestParam = nextStreamRequestParam;

                    putRequestInQueue(streamRequest);
                }
            }
            break;

        default:
            break;
        }
    }
    else
    {
        emit sigStreamRequestResponse(STOP_STREAM_COMMAND,
                                      streamRequestParam,
                                      CMD_SUCCESS);

        emit sigStreamRequestResponse(START_STREAM_COMMAND,
                                      nextStreamRequestParam,
                                      CMD_MAX_DEVICE_REPLY);

    }
}
/* 1)connected to sigCommandResponse emmited from CommandRequest::slotGenericResponse
 * 2)connected to sigMediaResponse emmited from LiveMedia::run ()
 * */
void StreamRequest::slotStreamResponse(REQ_MSG_ID_e,
                                       SET_COMMAND_e commandId,
                                       DEVICE_REPLY_TYPE_e statusId,
                                       QString payload)
{

    bool freeRequestFlag = false;
    bool replaceFlag = false;
    bool processQueue = false;
    bool emitSignalFlag = true;
    bool streamParamCreate = true;
    QStringList payloadField;
    QString tempPayload;
    STREAM_COMMAND_TYPE_e streamCommandType = MAX_STREAM_COMMAND;
    StreamRequestParam *streamParam = NULL;
    quint8 decoderId = INVALID_DEC_DISP_PLAY_ID, windowIndex;
    bool requestPending = false;
    STREAM_REQUEST_QUEUE_t streamRequest;

    switch(commandId)
    {
    case SRT_LV_STRM:
        streamCommandType = START_STREAM_COMMAND;
        if( (statusId == CMD_SUCCESS) || (statusId == CMD_STREAM_NO_VIDEO_LOSS) )
        {
            m_streamStatus = STREAM_RUN;
        }
        else if(statusId == CMD_STREAM_VIDEO_LOSS)
        {
            m_streamStatus = STREAM_VL;
            pauseDecoderInstance(false);
            /* force stop audio in if video loss */
            forceStopAudioIn();
        }
        else if(!((statusId == CMD_SUCCESS)
             || (statusId == CMD_STREAM_VIDEO_LOSS)
             || (statusId == CMD_STREAM_NO_VIDEO_LOSS)))
        {
            m_streamStatus = STREAM_NONE;

            if(getMediaRequestStopFlag() == true)
            {
                statusId = CMD_STREAM_STOPPED;
            }

            if((getClearFlag() == true) || (getReplaceFlag() == true))
            {
                emitSignalFlag = false;
            }

            if(getReplaceFlag() == true)
            {
                replaceFlag = true;
            }
            else
            {
                freeRequestFlag = true;
            }

            if ((statusId == CMD_DECODER_ERROR) || (statusId == CMD_DECODER_CAPACITY_ERROR))
            {
                /* force stop audio in if decoder error */
                forceStopAudioIn();
            }

            if(statusId == CMD_DECODER_ERROR)
            {
                DELETE_OBJ(streamParam);

                streamParam = new StreamRequestParam();
                if(IS_VALID_OBJ(streamParam))
                {
                    if(m_displayType < MAX_DISPLAY_TYPE)
                    {
                        m_streamParamMutex.lock ();
                        streamParam->streamId = m_streamId;
                        streamParam->windowId = m_windowId[m_displayType];
                        streamParam->actualWindowId = m_actualWindowId[m_displayType];
                        streamParam->channelId = m_channelIndex;
                        streamParam->deviceName = m_deviceName;
                        streamParam->displayType = m_displayType;
                        streamParam->streamRequestType = m_streamType;
                        streamParam->liveStreamType = m_liveStreamType;
                        streamParam->timeStamp = m_timeStamp[m_displayType];
                        streamParam->payload = payload;
                        m_streamParamMutex.unlock ();
                        stopStream (streamParam,false,true);
                    }
                    else
                    {
                        EPRINT(STREAM_REQ,"Error in Display Type");
                    }
                }
                else
                {
                     EPRINT(STREAM_REQ,"Error in creating Stream Request Parameter class");
                }
            }
        }

        if(freeRequestFlag == true)
        {
            if(isMediaControlActive() == true)
            {
                freeRequestFlag = false;
                deleteMediaRequest();
                setDeleteAfterMediaControlDeleteFlag(true);
            }
        }
        else if(replaceFlag == true)
        {
            if(isMediaControlActive() == true)
            {
                replaceFlag = false;
                setMediaPauseForReplaceFlag(true);
            }
        }

        break;

    case STP_LV_STRM:
    {
        emitSignalFlag = false;

        if(getDeleteAfterMediaControlDeleteFlag() == true)
        {
            freeRequestFlag = true;
        }

        if(getMediaPauseForReplaceFlag() == true)
        {
            setMediaPauseForReplaceFlag(false);
            replaceFlag = true;
        }
        else
        {
            freeRequestFlag = true;
        }

    }
        break;

    case INC_LV_AUD:
        streamCommandType = INCLUDE_AUDIO_COMMAND;
        emit sigDelMedControl(m_streamId);

        if(statusId == CMD_SUCCESS)
        {
            quint8 status = FAIL;
            quint8 winId = m_decoderId;
            quint8 tryCount = 0;

            do
            {
                if(getAudioIndex () != winId)
                {
                    excludeAudio ();
                }
                status = IncludeAudio(winId);
               if(status != SUCCESS)
                {
                    usleep(5000);
                }
                else
                {
                    break;
                }
            }while(tryCount++ <= 3);

            if(status == SUCCESS)
            {
                setAudioIndex (winId);
                setAudioStreamIndex(m_streamId);
            }
             emit sigChangeAudState();
        }

        requestPending = getRequestFromQueue(&streamRequest);
        if(requestPending == true)
        {
            processQueue = true;
        }
        else if(getDeleteAfterMediaControlDeleteFlag() == true)
        {
            freeRequestFlag = true;
        }
        break;

    case EXC_LV_AUD:
        streamCommandType = EXCLUDE_AUDIO_COMMAND;
        emit sigDelMedControl(m_streamId);
        if(statusId == CMD_SUCCESS)
        {
            excludeAudio ();
        }

        requestPending = getRequestFromQueue(&streamRequest);
        if(requestPending == true)
        {
            processQueue = true;
        }
        else if(getDeleteAfterMediaControlDeleteFlag() == true)
        {
            freeRequestFlag = true;
        }
        break;

    case CNG_LV_STRM:
        streamCommandType = CHANGE_STREAM_TYPE_COMMAND;

        if(m_liveMedia != NULL)
        {
            m_liveMedia->setIsIframeRecive (false);
        }
        emit sigDelMedControl(m_streamId);

        requestPending = getRequestFromQueue(&streamRequest);
        if(requestPending == true)
        {
            processQueue = true;
        }
        else if(getDeleteAfterMediaControlDeleteFlag() == true)
        {
            freeRequestFlag = true;
        }
        break;

    case GET_PLY_STRM_ID:
        streamCommandType = GET_PLAYBACK_ID_COMMAND;
        emit sigDelMedControl(m_streamId);
        switch(statusId)
        {
        case CMD_SUCCESS:
            tempPayload = payload;
            payloadField = tempPayload.split(FSP);
            m_playbackId = payloadField.at(1);
            break;

        default:
            freeRequestFlag = true;
            break;
        }
        break;

    case PLY_RCD_STRM:
        streamCommandType = PLAY_PLABACK_STREAM_COMMAND;

        if((statusId == CMD_SUCCESS)
                || (statusId == CMD_STREAM_VIDEO_LOSS))
               // || (statusId == CMD_STREAM_PLAYBACK_OVER))
        {
            pauseDecoderInstance();
        }

        if (!((statusId == CMD_SUCCESS)
              || (statusId == CMD_PLAYBACK_TIME)
              || (statusId == CMD_STREAM_CONNECTING)
              || (statusId == CMD_STREAM_VIDEO_LOSS)
              || (statusId == CMD_STREAM_NO_VIDEO_LOSS)))
        {
            if(getMediaRequestStopFlag() == true)
            {
                statusId = CMD_STREAM_STOPPED;
            }

            if(m_reRequest == false)
                deleteMediaRequest();
        }

        if(getClearFlag() == true)
        {
            emitSignalFlag = false;
        }
        break;

    case STEP_RCD_STRM:
        streamCommandType = STEP_PLABACK_STREAM_COMMAND;
        if(getClearFlag() == true)
        {
            emitSignalFlag = false;
        }
        if (!((statusId == CMD_SUCCESS)
              || (statusId == CMD_PLAYBACK_TIME)
              || (statusId == CMD_STREAM_CONNECTING)
              || (statusId == CMD_STREAM_VIDEO_LOSS)
              || (statusId == CMD_STREAM_NO_VIDEO_LOSS)))
        {
            deleteMediaRequest();
        }
        break;

    case STP_RCD_STRM:
        streamCommandType = STOP_PLABACK_STREAM_COMMAND;
        deleteMediaControl();
        break;

    case CLR_PLY_STRM_ID:
        streamCommandType = CLEAR_PLAYBACK_ID_COMMAND;
        deleteMediaControl();
        freeRequestFlag = true;
        break;

    case PLYBCK_RCD:
        streamCommandType = PLAY_SYNCPLABACK_STREAM_COMMAND;

        switch(statusId)
        {
        case CMD_STREAM_VIDEO_LOSS:
            if(m_syncPlaybackMedia != NULL)
            {
                m_syncPlaybackMedia->GetDecIdForCh(payload.toUInt(),
                                                   decoderId,
                                                   windowIndex);
            }

            if(decoderId != INVALID_DEC_DISP_PLAY_ID)
            {
            }
            break;

        case CMD_STREAM_PLAYBACK_OVER:
            pauseDecoderInstance();
            break;

        default:
            break;
        }

        if(!((statusId == CMD_SUCCESS)
             || (statusId == CMD_PLAYBACK_TIME)
			 || (statusId == CMD_STREAM_PB_SPEED)
             || (statusId == CMD_STREAM_PLAYBACK_OVER)
             || (statusId == CMD_NO_PRIVILEGE)
             || (statusId == CMD_STREAM_HDD_FORMAT)
             || (statusId == CMD_STREAM_FILE_ERROR)
             || (statusId == CMD_STREAM_CONFIG_CHANGE)
             || (statusId == CMD_DECODER_ERROR)
			 || (statusId == CMD_DECODER_CAPACITY_ERROR)
             || (statusId == CMD_STREAM_NO_VIDEO_LOSS)
             || (statusId == CMD_STREAM_VIDEO_LOSS)))
        {
            freeRequestFlag = true;
        }
		else if((statusId == CMD_NO_PRIVILEGE) || (statusId == CMD_DECODER_ERROR)
				|| (statusId == CMD_DECODER_CAPACITY_ERROR))
        {
            if(payload.isEmpty() == true)
            {
                freeRequestFlag = true;
            }
        }
        break;

    case SYNC_PLYBCK_RCD:
        streamCommandType = SYNCPLAY_SYNCPLABACK_STREAM_COMMAND;
        if(statusId == CMD_SUCCESS)
        {
            pauseDecoderInstance();
        }
        else if((statusId == CMD_SERVER_NOT_RESPONDING)
                || (statusId == CMD_REC_DRIVE_CONFIG_CHANGES))
        {
            freeRequestFlag = true;
        }
        break;

	case SYNC_PLYBCK_SPEED:
		streamCommandType = SET_SPEED_SYNCPLABACK_STREAM_COMMAND;
		break;

    case STP_RCD:
        streamCommandType = STOP_SYNCPLABACK_STREAM_COMMAND;
        if(IS_VALID_OBJ(m_syncPlaybackMedia))
        {
            m_syncPlaybackMedia->updateDecInstances(m_syncDecoderIdSet);
        }

        pauseDecoderInstance();

        if((statusId == CMD_SERVER_NOT_RESPONDING)
                || (statusId == CMD_REC_DRIVE_CONFIG_CHANGES))
        {
            freeRequestFlag = true;
        }
        break;

    case SYNC_STEP_FORWARD:
        streamCommandType = STEPFORWARD_SYNCPLABACK_STREAM_COMMAND;
        if((statusId == CMD_SERVER_NOT_RESPONDING)
                || (statusId == CMD_REC_DRIVE_CONFIG_CHANGES))
        {
            freeRequestFlag = true;
        }
        break;

    case SYNC_STEP_REVERSE:
        streamCommandType = STEPBACKWARD_SYNCPLABACK_STREAM_COMMAND;
        if((statusId == CMD_SERVER_NOT_RESPONDING)
                || (statusId == CMD_REC_DRIVE_CONFIG_CHANGES))
        {
            freeRequestFlag = true;
        }
        break;

    case PAUSE_RCD:
        streamCommandType = PAUSE_SYNCPLABACK_STREAM_COMMAND;
        if((statusId == CMD_SERVER_NOT_RESPONDING)
                || (statusId == CMD_REC_DRIVE_CONFIG_CHANGES))
        {
            freeRequestFlag = true;
        }
        break;

    case CLR_RCD:
        streamCommandType = CLEAR_SYNCPLABACK_STREAM_COMMAND;
        freeRequestFlag = true;
        break;

    case STRT_INSTANT_PLY:
        streamCommandType = START_INSTANTPLAYBACK_COMMAND;
        if((m_instantPlaybackMedia != NULL)
                && (m_instantPlaybackMedia->isValidPlaybackId() == false))
        {
            if(statusId == CMD_SUCCESS)
            {
                tempPayload = payload;
                payloadField = tempPayload.split(FSP);
                m_playbackId = payloadField.at(1);
                m_instantPlaybackMedia->setPlaybackId(m_playbackId);
                m_instantPlaybackMedia->setFrameReceiveFlag(true);
            }
            else if((statusId == CMD_NO_PRIVILEGE)
                    || (statusId == CMD_INTERNAL_RESOURCE_LIMIT)
                    || (statusId == CMD_PROCESS_ERROR)
                    || (statusId == CMD_INSTANT_PLAYBACK_FAILED)
                    || (statusId == CMD_SERVER_NOT_RESPONDING)
                    || (statusId == CMD_NO_DISK_FOUND))
            {
                freeRequestFlag = true;
            }
        }
        else
        {
            if((statusId == CMD_SUCCESS)
                    || (statusId == CMD_STREAM_VIDEO_LOSS)
                    || (statusId == CMD_STREAM_PLAYBACK_OVER))
            {
                pauseDecoderInstance();
            }

            if((statusId == CMD_PROCESS_ERROR)
                    || (statusId == CMD_INSTANT_PLAYBACK_FAILED)
                    || (statusId == CMD_NO_DISK_FOUND))
            {
                freeRequestFlag = true;
            }
        }
        break;

    case STOP_INSTANT_PLY:
        streamCommandType = STOP_INSTANTPLAYBACK_COMMAND;
        freeRequestFlag = true;
        break;

    case SEEK_INSTANT_PLY:
        streamCommandType = SEEK_INSTANTPLAYBACK_COMMAND;
        if(statusId == CMD_SUCCESS)
        {
            pauseDecoderInstance();
        }
        else if(statusId == CMD_PROCESS_ERROR)
        {
            freeRequestFlag = true;
        }
        break;

    case PAUSE_INSTANT_PLY:
        streamCommandType = PAUSE_INSTANTPLAYBACK_COMMAND;

        if(statusId == CMD_PROCESS_ERROR)
        {
            freeRequestFlag = true;
        }
        break;

    case STRT_CLNT_AUDIO:
        streamCommandType = STRT_CLNT_AUDIO_COMMAND;
        if(statusId != CMD_SUCCESS)
        {
            freeRequestFlag = true;
        }
        break;

    case STOP_CLNT_AUDIO:
        streamCommandType = STOP_CLNT_AUDIO_COMMAND;
        deleteMediaControl();
        freeRequestFlag = true;
        break;

    case SND_AUDIO:
        {
            streamCommandType = INCL_EXCL_AUDIO_IN_COMMAND;
            DPRINT(STREAM_REQ, "AudioIn: SND_AUDIO resp: [statusId=%d]", statusId);

            /* if status is not success then delete m_audioIn object */
            if (statusId != CMD_SUCCESS)
            {
                /* delete audioin in object */
                if (IS_VALID_OBJ(m_audioIn))
                {
                    m_audioIn->setRunFlag(false);

                    if (m_audioIn->isRunning())
                    {
                        m_audioIn->wait();
                        DPRINT(STREAM_REQ, "AudioIn: audio in thread exited");
                    }

                    disconnect(m_audioIn,
                               SIGNAL(sigMediaResponse(REQ_MSG_ID_e,
                                                       SET_COMMAND_e,
                                                       DEVICE_REPLY_TYPE_e,
                                                       QString)),
                               this,
                               SLOT(slotStreamResponse(REQ_MSG_ID_e,
                                                       SET_COMMAND_e,
                                                       DEVICE_REPLY_TYPE_e,
                                                       QString)));

                    DELETE_OBJ(m_audioIn);
                }                
            }

            DELETE_OBJ(streamParam);
            streamParam = new StreamRequestParam();
            if(IS_VALID_OBJ(streamParam))
            {
                if(m_displayType < MAX_DISPLAY_TYPE)
                {
                    m_streamParamMutex.lock ();
                    streamParam->streamId = m_streamId;
                    streamParam->windowId = m_windowId[m_displayType];
                    streamParam->actualWindowId = m_actualWindowId[m_displayType];
                    streamParam->channelId = m_channelIndex;
                    streamParam->deviceName = m_deviceName;
                    streamParam->displayType = m_displayType;
                    streamParam->streamRequestType = m_streamType;
                    streamParam->liveStreamType = m_liveStreamType;
                    streamParam->payload = payload;
                    streamParam->timeStamp = m_timeStamp[m_displayType];
                    streamParam->audioStatus = true;
                    m_streamParamMutex.unlock ();
                }
                else
                {
                    EPRINT(STREAM_REQ,"Error in Dsiplay Type [%d]", m_streamId);
                }
            }
            streamParamCreate = false;
        }
        break;

    case STP_AUDIO:
        {
            streamCommandType = INCL_EXCL_AUDIO_IN_COMMAND;
            deleteMediaControl();
            DPRINT(STREAM_REQ, "AudioIn: STP_AUDIO resp: [statusId=%d]", statusId);

            DELETE_OBJ(streamParam);
            streamParam = new StreamRequestParam();
            if(IS_VALID_OBJ(streamParam))
            {
                if(m_displayType < MAX_DISPLAY_TYPE)
                {
                    m_streamParamMutex.lock ();
                    streamParam->streamId = m_streamId;
                    streamParam->windowId = m_windowId[m_displayType];
                    streamParam->actualWindowId = m_actualWindowId[m_displayType];
                    streamParam->channelId = m_channelIndex;
                    streamParam->deviceName = m_deviceName;
                    streamParam->displayType = m_displayType;
                    streamParam->streamRequestType = m_streamType;
                    streamParam->liveStreamType = m_liveStreamType;
                    streamParam->payload = payload;
                    streamParam->timeStamp = m_timeStamp[m_displayType];
                    streamParam->audioStatus = false;
                    m_streamParamMutex.unlock ();                    
                }
                else
                {
                    EPRINT(STREAM_REQ,"Error in Dsiplay Type [%d]", m_streamId);
                }
            }
            streamParamCreate = false;
        }
        break;

    default:
        break;
    }

    if(emitSignalFlag == true)
    {
        m_streamParamMutex.lock ();
        if(((m_streamType == LIVE_ANALOG_REQUEST) || (m_streamType == LIVE_STREAM_REQUEST))
                && (m_displayType == MAX_DISPLAY_TYPE))
        {
            m_streamParamMutex.unlock ();
            streamParamCreate = false;
        }
        else
        {
            m_streamParamMutex.unlock ();
        }

        if(streamParamCreate == true)
        {

            DELETE_OBJ(streamParam);
            streamParam = new StreamRequestParam();
            if(IS_VALID_OBJ(streamParam))
            {
                if(m_displayType < MAX_DISPLAY_TYPE)
                {
                    m_streamParamMutex.lock ();
                    streamParam->streamId = m_streamId;
                    streamParam->windowId = m_windowId[m_displayType];
                    streamParam->actualWindowId = m_actualWindowId[m_displayType];
                    streamParam->channelId = m_channelIndex;
                    streamParam->deviceName = m_deviceName;
                    streamParam->displayType = m_displayType;
                    streamParam->streamRequestType = m_streamType;
                    streamParam->liveStreamType = m_liveStreamType;
                    streamParam->payload = payload;
                    streamParam->timeStamp = m_timeStamp[m_displayType];
                    m_streamParamMutex.unlock ();
                }
                else
                {
                    EPRINT(STREAM_REQ,"Error in Dsiplay Type [%d]", m_streamId);
                }
            }
            else
            {
                EPRINT(STREAM_REQ,"Error in creating Stream Request Parameter class [%d]", m_streamId);
            }
        }
    }

    if(getDeletionProcessFlag())
    {
        freeRequestFlag = false;
    }

    if(freeRequestFlag == true)
    {
        setDeletionProcessFlag (true);

        m_streamParamMutex.lock ();
        if(m_displayType < MAX_DISPLAY_TYPE)
        {
            m_displayTypeForDelete[0] = m_displayType;
            m_actualWindowIdForDelete[0] = m_actualWindowId[m_displayType];
        }
        m_streamParamMutex.unlock ();

        freeStreamRequest();
    }
    else if(replaceFlag == true)
    {
        setReplaceFlag(false);
        m_streamParamMutex.lock ();
        if(m_nextRequestParam == NULL)
        {
            m_streamParamMutex.unlock ();
            EPRINT(STREAM_REQ, "accesing nextParam while its null");
        }
        else
        {
            if(m_streamType != m_nextRequestParam->streamRequestType)
            {
                m_streamParamMutex.unlock ();
                freeStreamRequest();
                setFreeFlag(false);
                slotProcessRequest(START_STREAM_COMMAND,
                                   m_nextServerInfo,
                                   m_nextRequestParam);
            }
            else
            {
                m_streamParamMutex.unlock ();
                replaceStreamRequest();
            }
        }
    }

    if((emitSignalFlag == true) && (IS_VALID_OBJ(streamParam)))
    {
        emit sigStreamRequestResponse(streamCommandType,
                                      streamParam,
                                      statusId);
    }

    if(freeRequestFlag == true)
    {
        setDeletionProcessFlag (true);
        emit sigDeleteStreamRequest(m_streamId);
    }
    else if(processQueue == true)
    {
        if(streamRequest.commandType == REPLACE_STREAM_COMMAND)
        {
            processRequest(streamRequest.commandType,
                           streamRequest.serverInfo,
                           streamRequest.requestParam,
                           streamRequest.nextServerInfo,
                           streamRequest.nextRequestParam);
        }
        else
        {
            processRequest(streamRequest.commandType,
                           streamRequest.serverInfo,
                           streamRequest.requestParam);
        }
    }
}
