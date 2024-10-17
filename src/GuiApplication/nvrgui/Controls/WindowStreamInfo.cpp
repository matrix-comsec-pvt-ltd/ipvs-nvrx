#include "WindowStreamInfo.h"

WindowStreamInfo::WindowStreamInfo()
{
    clearWindowInfo();

    m_startRequestPending = false;
    m_stopRequestPending = false;
    m_replacingChannel = false;
    m_startInstantPlayback = false;
    m_stopInstantPlayback = false;
    m_pbOver = false;
    eventType = 0;
    timeStamp = 0;
}

void WindowStreamInfo::operator =(const WindowStreamInfo & newObject)
{
    m_deviceName = newObject.m_deviceName;
    m_cameraId = newObject.m_cameraId;
    m_streamId = newObject.m_streamId;
    m_videoType = newObject.m_videoType;
    m_videoStatus = newObject.m_videoStatus;
    m_audioStatus = newObject.m_audioStatus;
	m_microPhoneStatus = newObject.m_microPhoneStatus;
    m_errorType = newObject.m_errorType;
    m_streamType = newObject.m_streamType;
    m_streamRequestType = newObject.m_streamRequestType;
    m_startRequestPending = newObject.m_startRequestPending;
    m_startInstantPlayback = newObject.m_stopRequestPending;
    m_stopInstantPlayback = newObject.m_startRequestPending;
    m_stopRequestPending = newObject.m_stopRequestPending;
    m_replacingChannel = newObject.m_replacingChannel;

    //playback related parameters
    m_referenceFrameNo = newObject.m_referenceFrameNo;
    m_pbStreamId = newObject.m_pbStreamId;
    m_pbStartTime = newObject.m_pbStartTime;
    m_pbEndTime = newObject.m_pbEndTime;
    m_pbCurrentTime = newObject.m_pbCurrentTime;
    m_pbCurrTimeStr = newObject.m_pbCurrTimeStr;
    m_pbToolbarVisible = newObject.m_pbToolbarVisible;

    m_pbCurrCmd = newObject.m_pbCurrCmd;
    m_pbNextCmd = newObject.m_pbNextCmd;

    m_pbStreamState = newObject.m_pbStreamState;
    m_pbDirection = newObject.m_pbDirection;
    m_pbSpeed = newObject.m_pbSpeed;
}

void WindowStreamInfo::clearWindowInfo()
{
    m_deviceName = "";
    m_cameraId = INVALID_CAMERA_INDEX;
    m_streamId = MAX_STREAM_SESSION;
    m_videoType = VIDEO_TYPE_NONE;
    m_videoStatus = VIDEO_STATUS_NONE;
    m_audioStatus = false;
	m_microPhoneStatus = false;
    m_errorType = VIDEO_ERROR_NONE;
    m_streamType = MAX_LIVE_STREAM_TYPE;
    m_windowId = MAX_WINDOWS;
    m_streamRequestType = MAX_STREAM_REQUEST_TYPE;
    m_replacingChannel = false;

    //playback related parameters
    m_referenceFrameNo = 0;
    m_pbStreamId = MAX_STREAM_SESSION;
    m_pbStartTime = 0;
    m_pbEndTime = 0;
    m_pbCurrentTime = 0;
    m_pbCurrTimeStr = "";
    m_pbToolbarVisible = false;

    m_pbCurrCmd = PB_MAX_PLAYBACK_STATE;
    m_pbNextCmd = PB_MAX_PLAYBACK_STATE;

    m_pbStreamState = PB_MAX_PLAYBACK_STATE;
    m_pbDirection = MAX_PLAY_DIRECTION;
    m_pbSpeed = 0;
    m_iFrameNeeded = true;
    prevPlayState = PB_MAX_PLAYBACK_STATE;
}
