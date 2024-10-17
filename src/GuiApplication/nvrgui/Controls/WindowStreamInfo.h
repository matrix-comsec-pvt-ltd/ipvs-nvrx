#ifndef WINDOWSTREAMINFO_H
#define WINDOWSTREAMINFO_H

#include "EnumFile.h"

class WindowStreamInfo
{
public:
    QString m_deviceName;
    quint8 m_cameraId;
    quint8 m_streamId;
    VIDEO_STREAM_TYPE_e m_videoType;
    VIDEO_STATUS_TYPE_e m_videoStatus;
    bool m_audioStatus;
	bool m_microPhoneStatus;
    VIDEO_ERROR_TYPE_e m_errorType;
    LIVE_STREAM_TYPE_e m_streamType;
    STREAM_REQUEST_TYPE_e m_streamRequestType;
    quint8 m_windowId;
    bool m_startRequestPending;
    bool m_stopRequestPending;
    bool m_replacingChannel;
    bool m_startInstantPlayback;
    bool m_stopInstantPlayback;
    qint64 timeStamp;

    //playback related parameters
    quint8 m_referenceFrameNo;
    quint8 m_pbStreamId;
    quint64 m_pbStartTime;
    quint64 m_pbEndTime;
    quint64 m_pbCurrentTime;
    QString m_pbCurrTimeStr;
    bool m_pbToolbarVisible;

    PB_STREAM_STATE_e m_pbCurrCmd;
    PB_STREAM_STATE_e m_pbNextCmd;

    PB_STREAM_STATE_e m_pbStreamState;
    PB_DIRECTION_e m_pbDirection;
    quint8 m_pbSpeed;
    bool m_iFrameNeeded;
    quint16 eventType;

    // if record is < 1sec, then there is looping in between step & playback over
    // to avoid this condition
    bool m_pbOver;
    PB_STREAM_STATE_e prevPlayState;

public:
    WindowStreamInfo();
    void operator =(const WindowStreamInfo &);

    void clearWindowInfo();
};

#endif // WINDOWSTREAMINFO_H
