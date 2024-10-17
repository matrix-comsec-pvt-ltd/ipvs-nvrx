#include <QApplication>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QTimerEvent>

#include "Layout.h"
#include "ApplicationMode.h"
#include "ValidationMessage.h"
#include "Controls/MessageBanner.h"

#define COSEC_BANNER_INTERVAL       3000            //  3 Secs
#define RETRY_INTERVAL              5000            //  5 Secs
#define SYSTEM_RESTART_INTERVAL     (3 * 60000)     //  3 min
#define AUDIO_OUT_CNFG_FRM_INDEX    1
#define AUDIO_OUT_CNFG_TO_INDEX     1
#define AUDIO_OUT_CNFG_FRM_FIELD    1
#define AUDIO_OUT_MAX_FIELD_NO      1

typedef enum
{
    COSEC_POPUP_USR_ALLOWED,
    COSEC_POPUP_USR_DENIED,
    COSEC_POPUP_NOT_IDENTIFY,
    COSEC_POPUP_AUX_CHNANGE,
    COSEC_POPUP_DURESS_DETECT,
    COSEC_POPUP_DEAD_MAN_TMR_EXPR,
    COSEC_POPUP_PANIC_ALARM,
    COSEC_POPUP_DOOR_ABNORMAL,
    COSEC_POPUP_DOOR_FORCE_OPEN,
    COSEC_POPUP_DOOR_TEMPAR_ALARM,
    COSEC_POPUP_INTERCOM_PANIC_ALARM,
    COSEC_POPUP_AUX2_CHANGE,
    COSEC_POPUP_AUX3_CHANGE,
    COSEC_POPUP_AUX4_CHANGE,
    COSEC_POPUP_AUX5_CHANGE,
    COSEC_POPUP_AUX6_CHANGE,
    COSEC_POPUP_AUX7_CHANGE,
    COSEC_POPUP_AUX8_CHANGE,
    MAX_COSEC_POPUP_STATUS
}COSEC_POPUP_STAUSCODE_e;

static QString modString[] =
{
    "STATE_OTHER_MODE_NONE",
    "STATE_OTHER_MODE_PROCESSING_START_REQUEST",
    "STATE_OTHER_MODE_PROCESSING_STOP_REQUEST",
    "STATE_OTHER_MODE_PROCESSING_REPLACE_REQUEST",
    "STATE_APPLY_NEW_STYLE",
    "STATE_CHANGE_PAGE",
    "STATE_REFRESH_PAGE",
    "STATE_RETRY_PAGE",
    "STATE_SYNC_PLAYBACK_START_MODE",
    "STATE_SYNC_PLAYBACK_STOP_MODE",
    "STATE_SYNC_PLAYBACK_MODE",
    "STATE_ANALOG_FEATURE",
    "STATE_COSEC_FEATURE",
    "STATE_EXPAND_WINDOW_START_MODE",
    "STATE_COLLAPSE_WINDOW_START_MODE",
    "STATE_EXPAND_WINDOW",
    "STATE_INSTANT_PLAYBACK_START_MODE",
    "STATE_VIDEO_POPUP_FEATURE",
    "STATE_LOCAL_DECODING"
};

PayloadLib*                     Layout::payloadLib = new PayloadLib();
ApplController*                 Layout::applController = NULL;
LayoutCreator*                  Layout::layoutCreator = NULL;
WindowStreamInfo                Layout::streamInfoArray[MAX_DISPLAY_TYPE][MAX_CHANNEL_FOR_SEQ];
PlaybackRecordData              Layout::playbackRecordData[MAX_CHANNEL_FOR_SEQ];

ANALOG_FEATURE_VARIABLE_t       Layout::analogFeatureDataType;
COSEC_POPUP_FEATURE_VARIABLE_t  Layout::cosecPopupEvtData;
CAMEVT_VIDEO_POPUP_t            Layout::videoPopupData;
DISPLAY_CONFIG_t                Layout::currentDisplayConfig[MAX_DISPLAY_TYPE];
AUTO_PLAY_DATA_t                Layout::autoPlayData;

STATE_TYPE_FOR_OTHER_MODE_e     Layout::currentModeType[MAX_DISPLAY_TYPE] = {STATE_OTHER_MODE_NONE};
STATE_TYPE_FOR_OTHER_MODE_e     Layout::previousModeType[MAX_DISPLAY_TYPE] = {STATE_OTHER_MODE_NONE};

quint8                          Layout::pendingInstantRequestCount[MAX_DISPLAY_TYPE][MAX_CHANNEL_FOR_SEQ] = {0};
quint16                         Layout::startX[MAX_DISPLAY_TYPE] = {0};
quint16                         Layout::startY[MAX_DISPLAY_TYPE] = {0};
quint16                         Layout::screenWidth[MAX_DISPLAY_TYPE] = {0};
quint16                         Layout::screenHeight[MAX_DISPLAY_TYPE] = {0};
quint16                         Layout::startAudioInWindow = MAX_CHANNEL_FOR_SEQ;
quint16                         Layout::lastEnableAudioWindow = MAX_CHANNEL_FOR_SEQ;
quint32                         Layout::tvAdjustParam = 0;

bool                            Layout::isRedraw = false;
bool                            Layout::playbackStopRequest = false;
bool                            Layout::m_isClientAudProcessRunning = false;

static LOCAL_DECODING_DATA_t    localDecodingFeature;

Layout::Layout(QWidget *parent) : QWidget(parent)
{
    setObjectName("Layout");
    m_audIndexToPlay = 0;
    INIT_OBJ(m_inVisibleWidget);
    m_isRetryingModeinExpand = false;

    m_nextToolbarPage = MAX_TOOLBAR_BUTTON;
    m_previousSelectedWindow = 0;

    m_syncBackupLayout = MAX_LAYOUT;
    previouslayoutId = MAX_LAYOUT;
    applController = ApplController::getInstance();

    if (getScreenParam(ONE_X_ONE, MAIN_DISPLAY))
    {
        this->setGeometry(startX[MAIN_DISPLAY], startY[MAIN_DISPLAY], screenWidth[MAIN_DISPLAY], screenHeight[MAIN_DISPLAY]);
    }

    layoutCreator = new LayoutCreator(0, 0, this->width(), this->height(), this);
    connect(layoutCreator,
            SIGNAL(sigWindowSelected(quint16)),
            this,
            SLOT(slotWindowSelected(quint16)));
    connect(layoutCreator,
            SIGNAL(sigLoadMenuListOptions(quint16)),
            this,
            SLOT(slotLoadMenuListOptions(quint16)));
    connect(layoutCreator,
            SIGNAL(sigWindowDoubleClicked(quint16)),
            this,
            SLOT(slotWindowDoubleClicked(quint16)));
    connect(layoutCreator,
            SIGNAL(sigSwapWindows(quint16, quint16)),
            this,
            SLOT(slotSwapWindows(quint16, quint16)));
    connect(layoutCreator,
            SIGNAL(sigDragStartStopEvent(bool)),
            this,
            SLOT(slotDragStartStopEvent(bool)));
    connect(layoutCreator,
            SIGNAL(sigChangePageFromLayout()),
            this,
            SLOT(slotChangePageFromLayout()));
    connect(layoutCreator,
            SIGNAL(sigEnterKeyPressed(quint16)),
            this,
            SLOT(slotEnterKeyPressed(quint16)));
    connect(layoutCreator,
            SIGNAL(sigWindowImageHover(WINDOW_IMAGE_TYPE_e, quint16, bool)),
            this,
            SLOT(slotWindowImageHover(WINDOW_IMAGE_TYPE_e, quint16, bool)));
    connect(layoutCreator,
            SIGNAL(sigWindowImageClicked(WINDOW_IMAGE_TYPE_e, quint16)),
            this,
            SLOT(slotWindowImageClicked(WINDOW_IMAGE_TYPE_e, quint16)));
    connect(layoutCreator,
            SIGNAL(sigAPCenterBtnClicked(quint8, quint16)),
            this,
            SLOT(slotAPCenterBtnClicked(quint8, quint16)));

    m_motionDetectionSettings = NULL;
    m_privackMaskSettings = NULL;
    m_imageAppearenceSettings = NULL;
    m_ipMotionDetectionSettings = NULL;
    m_pbToolbar = NULL;
    m_cosecVideoPopupUser = NULL;
    m_zoomFeatureControl = NULL;
    m_ptzControl = NULL;
    m_infoPage = NULL;
    m_viewCamera = NULL;
    m_instantpbToolbar = NULL;
    m_liveViewToolbar = NULL;
    m_cosecPopupMsg = NULL;
    m_cosecMsgBannerRect = NULL;
    startAudioInWindow = MAX_CHANNEL_FOR_SEQ;
    lastEnableAudioWindow = MAX_CHANNEL_FOR_SEQ;
	m_activeMicroPhoneWindow = MAX_CHANNEL_FOR_SEQ;
    m_audioStopForWindow = MAX_CHANNEL_FOR_SEQ;
    m_AudioEnableCamId = MAX_CAMERAS;
    m_AudioEnableDevice = "";
    m_expandMode = false;
    m_videoPopupMode = false;
    m_instantPlayback = false;
    m_isPreviousPageNaviagtion = false;
    m_focusCntrlIndex = 255;
    m_analogPresetMenu = NULL;
    m_previousSyncBeforeOneXOne = MAX_LAYOUT;
    m_isClientAudProcessRunning = false;

    for (quint16 windowIndex = 0; windowIndex < MAX_CHANNEL_FOR_SEQ; windowIndex++)
    {
        pendingRequestForNxtPbRec[windowIndex] = MAX_AP_TOOLBAR_BTN;
        m_currentRecNo[windowIndex] = MAX_REC_ALLOWED;
        m_searchBtnClickRecNo[windowIndex] = MAX_REC_ALLOWED;
    }

    for (quint16 windowIndex = 0; windowIndex < MAX_CHANNEL_FOR_SEQ; windowIndex++)
    {
        m_instantPlaybackRequestSent[windowIndex] = false;
        m_instantPlaybackData[windowIndex].currentCamInfo.defChannel = INVALID_CAMERA_INDEX;
        m_instantPlaybackData[windowIndex].currentCamInfo.deviceName[0] = '\0';
        memset(&m_instantPlaybackData[windowIndex].backupWindowInfo, 0, sizeof(WINDOW_INFO_t));

        for (quint8 channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
        {
            m_instantPlaybackData[windowIndex].backupWindowInfo.camInfo[channelIndex].deviceName[0] = '\0';
            m_instantPlaybackData[windowIndex].backupWindowInfo.camInfo[channelIndex].defChannel = INVALID_CAMERA_INDEX;
        }
    }

    for (quint16 windowIndex = 0; windowIndex < MAX_CHANNEL_FOR_SEQ; windowIndex++)
    {
        resetAutoPlayFeatureInWindow(windowIndex);
    }

    memset(&m_syncBackupWinInfo, 0, sizeof(m_syncBackupWinInfo));
    for (quint8 windowIndex = 0; windowIndex < MAX_SYNC_PB_SESSION; windowIndex++)
    {
        for (quint8 channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
        {
            m_syncBackupWinInfo[windowIndex].camInfo[channelIndex].deviceName[0] = '\0';
            m_syncBackupWinInfo[windowIndex].camInfo[channelIndex].defChannel = INVALID_CAMERA_INDEX;
        }
    }

    for (quint8 displayIndex = 0; displayIndex < MAX_DISPLAY_TYPE; displayIndex++)
    {
        m_timerIdForSequencing[displayIndex] = 0;
        m_isCurrStyleSelected[displayIndex] = false;
        m_refreshPageLaterFlag[displayIndex] = false;
        m_changePageLaterFlag[displayIndex] = false;
        m_applyNewStyleLaterFlag[displayIndex] = false;
        m_nextStyleNoToBeApplied[displayIndex] = MAX_STYLE_TYPE;
        m_changePageOffset[displayIndex] = 0;
        m_previousLayout[displayIndex] = MAX_LAYOUT;
        m_pendingRequestCount[displayIndex] = 0;
        m_processPendingForWindow[displayIndex] = false;

        for (quint16 windowIndex = 0; windowIndex < MAX_CHANNEL_FOR_SEQ; windowIndex++)
        {
            m_timerIdForWindowSequence[displayIndex][windowIndex] = 0;
            pendingInstantRequestCount[displayIndex][windowIndex] = 0;
        }

        memset(&currentDisplayConfig[displayIndex], 0, sizeof(DISPLAY_CONFIG_t));
        memset(&m_nextDisplayConfig[displayIndex], 0, sizeof(DISPLAY_CONFIG_t));
    }

    m_instantAudioWindow = MAX_CHANNEL_FOR_SEQ;
    m_stepIsRunning = false;
    m_doubleClickModeFlag = false;
    m_syncBackupSelectedWindow = MAX_CHANNEL_FOR_SEQ;
    m_syncPlaybackStartFlag = false;
    m_cosecHandleLaterFlag = false;
    m_pbSliderChange = false;
    m_isRetryingMode = false;
    m_timerIdForCosecPopup = 0;
    m_timerIdForCosecBanner = 0;
    memset(&prevDisplayConfig, 0, sizeof(DISPLAY_CONFIG_t));

    m_infoPage = new InfoPage(0, 0, parent->width(), parent->height(), MAX_INFO_PAGE_TYPE, parent, false, false);
    connect(m_infoPage,
            SIGNAL(sigInfoPageCnfgBtnClick(int)),
            this,
            SLOT(slotInfoPageButtonClicked(int)));

    /* start timer for retry */
    m_timerIdForRetry = this->startTimer(RETRY_INTERVAL);
    resetAnalogCameraFeature();
    resetCosecPopupFeature();
    resetVideoPopupFeature();
    resetLocalDecordingData();
    this->setEnabled(true);
    this->setMouseTracking(true);
    this->show();
}

Layout::~Layout()
{
    disconnect(m_infoPage,
               SIGNAL(sigInfoPageCnfgBtnClick(int)),
               this,
               SLOT(slotInfoPageButtonClicked(int)));
    DELETE_OBJ(m_infoPage);

    if (autoPlayData.m_timerIdForAutoPlay != 0)
    {
        killTimer(autoPlayData.m_timerIdForAutoPlay);
        autoPlayData.m_timerIdForAutoPlay = 0;
    }

    disconnect(layoutCreator,
               SIGNAL(sigWindowSelected(quint16)),
               this,
               SLOT(slotWindowSelected(quint16)));
    disconnect(layoutCreator,
               SIGNAL(sigLoadMenuListOptions(quint16)),
               this,
               SLOT(slotLoadMenuListOptions(quint16)));
    disconnect(layoutCreator,
               SIGNAL(sigWindowDoubleClicked(quint16)),
               this,
               SLOT(slotWindowDoubleClicked(quint16)));
    disconnect(layoutCreator,
               SIGNAL(sigSwapWindows(quint16, quint16)),
               this,
               SLOT(slotSwapWindows(quint16, quint16)));
    disconnect(layoutCreator,
               SIGNAL(sigDragStartStopEvent(bool)),
               this,
               SLOT(slotDragStartStopEvent(bool)));
    disconnect(layoutCreator,
               SIGNAL(sigChangePageFromLayout()),
               this,
               SLOT(slotChangePageFromLayout()));
    disconnect(layoutCreator,
               SIGNAL(sigEnterKeyPressed(quint16)),
               this,
               SLOT(slotEnterKeyPressed(quint16)));
    disconnect(layoutCreator,
               SIGNAL(sigWindowImageHover(WINDOW_IMAGE_TYPE_e, quint16, bool)),
               this,
               SLOT(slotWindowImageHover(WINDOW_IMAGE_TYPE_e, quint16, bool)));
    disconnect(layoutCreator,
               SIGNAL(sigWindowImageClicked(WINDOW_IMAGE_TYPE_e, quint16)),
               this,
               SLOT(slotWindowImageClicked(WINDOW_IMAGE_TYPE_e, quint16)));
    disconnect(layoutCreator,
            SIGNAL(sigAPCenterBtnClicked(quint8,quint16)),
            this,
            SLOT(slotAPCenterBtnClicked(quint8,quint16)));
    DELETE_OBJ(layoutCreator);
}

bool Layout::getScreenParam(LAYOUT_TYPE_e layoutIndex, DISPLAY_TYPE_e displayType)
{
    bool status = false;

    if (layoutIndex >= ONE_X_ONE_PLAYBACK)
    {
       VALID_SCREEN_INFO_t screenInfo;
       status = GetValidScreenInfoPlayBack((DISPLAY_DEV_e)displayType, &screenInfo, (WIND_LAYOUT_ID_e)layoutIndex);
       if (status == true)
       {
           startX[displayType] = screenInfo.actualStartX;
           startY[displayType] = screenInfo.actualStartY;
           screenWidth[displayType] = screenInfo.actualWidth;
           screenHeight[displayType] = screenInfo.actualHeight;
       }
    }
    else
    {
        status = applController->getOriginofScreen(layoutIndex, displayType);
        if (status == true)
        {
            startX[displayType] = ApplController::getXPosOfScreen();
            startY[displayType] = ApplController::getYPosOfScreen();
            screenWidth[displayType] = ApplController::getWidthOfScreen();
            screenHeight[displayType] = ApplController::getHeightOfScreen();
        }
    }

    return status;
}

STYLE_TYPE_e Layout::readDefaultStyle(DISPLAY_TYPE_e displayIndex)
{
    STYLE_TYPE_e styleType = MAX_STYLE_TYPE;
    QList<QVariant> paramList;
    paramList.insert(SUB_ACTIVITY_TYPE, READ_DFLTSTYLE_ACTIVITY);
    paramList.insert(DISPLAY_ID, displayIndex);
    applController->processActivity(DISPLAY_SETTING, paramList, &styleType);
    return styleType;
}

void Layout::readDefaultLayout(DISPLAY_TYPE_e displayIndex)
{
    STYLE_TYPE_e  defaultStyleId = readDefaultStyle(displayIndex);
    if (defaultStyleId >= MAX_STYLE_TYPE)
    {
        return;
    }

    QList<QVariant> paramList;
    paramList.append(READ_DISP_ACTIVITY);
    paramList.append(displayIndex);
    paramList.append(defaultStyleId);
    applController->processActivity(DISPLAY_SETTING, paramList, &currentDisplayConfig[displayIndex]);

    for (quint16 windowIndex = 0; windowIndex < MAX_CHANNEL_FOR_SEQ; windowIndex++)
    {
        if (false == isMultipleChannelAssigned(displayIndex,windowIndex))
        {
            currentDisplayConfig[displayIndex].windowInfo[windowIndex].sequenceStatus = false;
            currentDisplayConfig[displayIndex].windowInfo[windowIndex].sequenceInterval = DEFAULT_WINDOW_SEQ_INTERVAL;
        }
    }

    paramList.clear();
}

void Layout::setDefaultLayout()
{
    for (quint8 displayIndex = 0; displayIndex < deviceRespInfo.maxDisplayOutput; displayIndex++)
    {
        applController->readTVApperanceParameters(tvAdjustParam);

        for (quint16 windowIndex = 0; windowIndex < MAX_CHANNEL_FOR_SEQ; windowIndex++)
        {
            for (quint8 channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
            {
                if ((strcmp(currentDisplayConfig[displayIndex].windowInfo[windowIndex].camInfo[channelIndex].deviceName, LOCAL_DEVICE_NAME) == 0) &&
                        (currentDisplayConfig[displayIndex].windowInfo[windowIndex].camInfo[channelIndex].defChannel > deviceRespInfo.maxCameras))
                {
                    currentDisplayConfig[displayIndex].windowInfo[windowIndex].camInfo[channelIndex].deviceName[0] = '\0';
                    currentDisplayConfig[displayIndex].windowInfo[windowIndex].camInfo[channelIndex].defChannel = INVALID_CAMERA_INDEX;
                }
            }
        }

        //update parameters and apply layout
        changeLayout((DISPLAY_TYPE_e)displayIndex, currentDisplayConfig[displayIndex].layoutId, currentDisplayConfig[displayIndex].selectedWindow);

        if (currentDisplayConfig[displayIndex].seqStatus == true)
        {
            startSequencing((DISPLAY_TYPE_e)displayIndex, true);
        }
        else
        {
            startWindowSequencing((DISPLAY_TYPE_e)displayIndex);
        }

        if ((ApplicationMode::getApplicationMode() == IDLE_MODE) && (displayIndex == MAIN_DISPLAY))
        {
            giveFocusToWindow(currentDisplayConfig[displayIndex].selectedWindow);
        }
    }
}

void Layout::setDefaultResolution()
{
        // set appearance parameters for HDMI on boot
        DISPLAY_PARAM_t param;
        applController->readDisplayParameters(HDMI, param);
        applController->setDisplayParameters(HDMI, PHYSICAL_DISPLAY_BRIGHTNESS, param.brighteness);
        applController->setDisplayParameters(HDMI, PHYSICAL_DISPLAY_CONTRAST, param.contrast);
        applController->setDisplayParameters(HDMI, PHYSICAL_DISPLAY_SATURATION, param.saturation);
        applController->setDisplayParameters(HDMI, PHYSICAL_DISPLAY_HUE, param.hue);

        // adjust TV Parameter
        applController->readTVApperanceParameters(Layout::tvAdjustParam);
        applController->setTVApperanceParameters(Layout::tvAdjustParam);
}

void Layout::setDefaultDisplay(QString deviceName)
{
    if ((currentModeType[MAIN_DISPLAY] == STATE_OTHER_MODE_NONE) && (isAnyWindowReplacing(MAIN_DISPLAY) == false))
    {
        if (deviceName == LOCAL_DEVICE_NAME)
        {
            for (quint8 disIndex = 0; disIndex < MAX_DISPLAY_TYPE; disIndex++)
            {
                for (quint16 winIndex = 0; winIndex < MAX_CHANNEL_FOR_SEQ; winIndex++)
                {
                    for (quint8 channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
                    {
                        if ((strcmp(currentDisplayConfig[disIndex].windowInfo[winIndex].camInfo[channelIndex].deviceName, LOCAL_DEVICE_NAME) == 0)
                                && (currentDisplayConfig[disIndex].windowInfo[winIndex].camInfo[channelIndex].defChannel > deviceRespInfo.maxCameras))
                        {
                            currentDisplayConfig[disIndex].windowInfo[winIndex].camInfo[channelIndex].deviceName[0] = '\0';
                            currentDisplayConfig[disIndex].windowInfo[winIndex].camInfo[channelIndex].defChannel = INVALID_CAMERA_INDEX;
                        }
                    }
                }
            }
        }

        setDefaultStreamInfo(deviceName);
        DEV_TABLE_INFO_t deviceInfo;
        memset(&deviceInfo, 0, sizeof(deviceInfo));
        applController->GetDeviceInfo(LOCAL_DEVICE_NAME, deviceInfo);

        if (deviceInfo.startLiveView == true)
        {
            startDefaultStreaming(deviceName);
        }
    }
}

void Layout::setDefaultStreamInfo(QString deviceName)
{
    quint16 windowLimit[2] = {0,MAX_CHANNEL_FOR_SEQ};
    quint8 channelIndex;

    for (quint8 disIndex = 0; disIndex < deviceRespInfo.maxDisplayOutput; disIndex++)
    {
        if (disIndex >= MAX_DISPLAY_TYPE)
        {
            break;
        }

        getFirstAndLastWindow(currentDisplayConfig[disIndex].currPage, currentDisplayConfig[disIndex].layoutId, windowLimit);

        for (quint16 windowIndex = windowLimit[0]; windowIndex <= windowLimit[1]; windowIndex++)
        {
            channelIndex = currentDisplayConfig[disIndex].windowInfo[windowIndex].currentChannel;

            if ((currentDisplayConfig[disIndex].windowInfo[windowIndex].camInfo[channelIndex].deviceName[0] == '\0')
                    && (currentDisplayConfig[disIndex].windowInfo[windowIndex].camInfo[channelIndex].defChannel == INVALID_CAMERA_INDEX))
            {
                for (quint8 index = 0; index < MAX_WIN_SEQ_CAM; index++)
                {
                    if ((currentDisplayConfig[disIndex].windowInfo[windowIndex].camInfo[index].deviceName[0] != '\0')
                            && (currentDisplayConfig[disIndex].windowInfo[windowIndex].camInfo[index].defChannel != INVALID_CAMERA_INDEX))
                    {
                        channelIndex = index;
                        currentDisplayConfig[disIndex].windowInfo[windowIndex].currentChannel = channelIndex;
                        break;
                    }
                }
            }

            if ((deviceName == currentDisplayConfig[disIndex].windowInfo[windowIndex].camInfo[channelIndex].deviceName)
                    || ((deviceName == LOCAL_DEVICE_NAME)
                        && (currentDisplayConfig[disIndex].windowInfo[windowIndex].camInfo[channelIndex].deviceName[0] != '\0')))
            {
                if (streamInfoArray[disIndex][windowIndex].m_deviceName == "")
                {
                    streamInfoArray[disIndex][windowIndex].m_deviceName = currentDisplayConfig[disIndex].windowInfo[windowIndex].camInfo[channelIndex].deviceName;
                    streamInfoArray[disIndex][windowIndex].m_cameraId = currentDisplayConfig[disIndex].windowInfo[windowIndex].camInfo[channelIndex].defChannel;
                    streamInfoArray[disIndex][windowIndex].m_videoType = VIDEO_TYPE_LIVESTREAM_AWAITING;
                }
            }
        }
    }
}

void Layout::startDefaultStreaming(QString deviceName)
{
    quint16 windowLimit[2] = {0,MAX_CHANNEL_FOR_SEQ};

    for (quint8 displayType = 0; displayType < deviceRespInfo.maxDisplayOutput; displayType++)
    {
        if (displayType >= MAX_DISPLAY_TYPE)
        {
            break;
        }

        getFirstAndLastWindow(currentDisplayConfig[displayType].currPage, currentDisplayConfig[displayType].layoutId, windowLimit);
        for (quint16 windowIndex = windowLimit[0]; windowIndex <= windowLimit[1]; windowIndex++)
        {
            if (windowIndex >= MAX_CHANNEL_FOR_SEQ)
            {
                break;
            }

            if ((streamInfoArray[displayType][windowIndex].m_deviceName == deviceName)
                    && (streamInfoArray[displayType][windowIndex].m_videoType == VIDEO_TYPE_LIVESTREAM_AWAITING))
            {
                startLiveStream((DISPLAY_TYPE_e)displayType, streamInfoArray[displayType][windowIndex].m_deviceName,
                                streamInfoArray[displayType][windowIndex].m_cameraId, windowIndex);
            }
        }
    }
}

void Layout::changeLayout(DISPLAY_TYPE_e displayIndex, LAYOUT_TYPE_e layoutIndex, quint16 windowIndex, bool ifActualWindowIndex, bool ifUpdateCurrentPage)
{
    qint32 currentPage;
    quint8 decoderWindowId = windowIndex;

    if ((layoutIndex >= 0) && (layoutIndex < MAX_LAYOUT) && (!ifActualWindowIndex))
    {
        decoderWindowId = windowIndex % windowPerPage[layoutIndex];
    }

    if (false == ChangeWindowLayout((DISPLAY_DEV_e)displayIndex, (WIND_LAYOUT_ID_e)layoutIndex, decoderWindowId))
    {
        return;
    }

    if (false == getScreenParam(layoutIndex, (DISPLAY_TYPE_e)displayIndex))
    {
        return;
    }

    if (ifUpdateCurrentPage)
    {
        currentPage = (windowIndex / windowPerPage[layoutIndex]);
        currentDisplayConfig[displayIndex].currPage = currentPage;
    }

    m_previousLayout[displayIndex] = currentDisplayConfig[displayIndex].layoutId;
    currentDisplayConfig[displayIndex].layoutId = layoutIndex;
    this->setGeometry(startX[MAIN_DISPLAY], startY[MAIN_DISPLAY], screenWidth[MAIN_DISPLAY], screenHeight[MAIN_DISPLAY]);
    layoutCreator->setGeometry(0, 0, this->width(), this->height());
    updateLayoutUiData(displayIndex);
}

void Layout::updateLayoutUiData(DISPLAY_TYPE_e displayType)
{
    quint16 winLim[2] = {0,MAX_CHANNEL_FOR_SEQ};

    if ((displayType >= MAX_DISPLAY_TYPE) || (displayType < MAIN_DISPLAY))
    {
        return;
    }

    layoutCreator->setLayoutStyle(currentDisplayConfig[displayType].layoutId, currentDisplayConfig[displayType].selectedWindow);
    getFirstAndLastWindow(currentDisplayConfig[displayType].currPage, currentDisplayConfig[displayType].layoutId, winLim);

    for (quint16 windowIndex = winLim[0]; windowIndex <= winLim[1]; windowIndex++)
    {
        layoutCreator->updateWindowData(windowIndex);
    }
}

void Layout::setupUIData(DISPLAY_TYPE_e displayType, QString deviceName)
{
    quint8 totalCamera = 0;
    applController->GetTotalCamera(deviceName, totalCamera);

    for (quint8 cameraIndex = 0; cameraIndex < totalCamera; cameraIndex++)
    {
        setupUIData(displayType, deviceName, (cameraIndex + 1));
    }

    /* When device name is changed, update the device name in async-playback window and instant playback window */
    for (quint16 index = 0; index < MAX_CHANNEL_FOR_SEQ; index++)
    {
        if ((streamInfoArray[MAIN_DISPLAY][index].m_videoType == VIDEO_TYPE_PLAYBACKSTREAM)
                || (streamInfoArray[MAIN_DISPLAY][index].m_videoType == VIDEO_TYPE_INSTANTPLAYBACKSTREAM))
        {
            quint16 window[2] = {0,MAX_CHANNEL_FOR_SEQ};
            getFirstAndLastWindow(currentDisplayConfig[displayType].currPage, currentDisplayConfig[displayType].layoutId, window);

            if ((index >= window[0]) && (index <= window[1]))
            {
                layoutCreator->updateWindowData(index);
            }
        }
    }
}

void Layout::setupUIData(DISPLAY_TYPE_e displayType, QString deviceName, quint8 camId)
{
    quint16 windowIndex;
    windowIndex = findWindowOfLiveStream(displayType, deviceName, camId);
    if (windowIndex >=  MAX_CHANNEL_FOR_SEQ)
    {
        return;
    }

    quint16 window[2] = {0,MAX_CHANNEL_FOR_SEQ};
    getFirstAndLastWindow(currentDisplayConfig[displayType].currPage, currentDisplayConfig[displayType].layoutId, window);

    if ((windowIndex >= window[0]) && (windowIndex <= window[1]))
    {
        layoutCreator->updateWindowData(windowIndex);
    }
}

void Layout::mouseDoubleClicked(quint16 windowIndex)
{
    quint16 tempMaxWindow;

    applController->readMaxWindowsForDisplay(tempMaxWindow);
    if (windowIndex >= tempMaxWindow)
    {
        EPRINT(LAYOUT, "invld window index: [windowIndex=%d]", windowIndex);
        return;
    }

    if (true == currentDisplayConfig[MAIN_DISPLAY].seqStatus)
    {
        if ((ApplicationMode::getApplicationMode() == IDLE_MODE) && (currentModeType[MAIN_DISPLAY] != STATE_LOCAL_DECODING))
        {
            MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SEQUENCING_ON_MSG));
        }
        return;
    }

    if (((ApplicationMode::getApplicationMode() == IDLE_MODE) || (ApplicationMode::getApplicationMode() == ASYNC_PLAYBACK_TOOLBAR_MODE))
            && (currentModeType[MAIN_DISPLAY] == STATE_OTHER_MODE_NONE))
    {
        emit sigUnloadToolbar();
        closeDeletePbToolBar();
        closeDeleteInstantPbToolBar();
        setSelectedWindow(windowIndex);

        switch (streamInfoArray[MAIN_DISPLAY][windowIndex].m_videoType)
        {
            case VIDEO_TYPE_LIVESTREAM:
            case VIDEO_TYPE_PLAYBACKSTREAM:
            case VIDEO_TYPE_INSTANTPLAYBACKSTREAM:
            {
                if ((m_doubleClickModeFlag == false) && (currentDisplayConfig[MAIN_DISPLAY].layoutId != ONE_X_ONE))
                {
                    DISPLAY_CONFIG_t displayConfig;
                    memcpy(&displayConfig, &currentDisplayConfig[MAIN_DISPLAY], sizeof(DISPLAY_CONFIG_t));
                    displayConfig.layoutId = ONE_X_ONE;
                    m_previousSelectedWindow = displayConfig.selectedWindow;
                    applyNewStyle(MAIN_DISPLAY, displayConfig);
                    m_doubleClickModeFlag = true;
                }
                else if ((m_doubleClickModeFlag == true) && (m_previousLayout[MAIN_DISPLAY] != MAX_LAYOUT))
                {
                    DISPLAY_CONFIG_t displayConfig;
                    memcpy(&displayConfig, &currentDisplayConfig[MAIN_DISPLAY], sizeof(DISPLAY_CONFIG_t));
                    displayConfig.layoutId = m_previousLayout[MAIN_DISPLAY];
                    m_doubleClickModeFlag = false;

                    if (m_previousSelectedWindow != MAX_CHANNEL_FOR_SEQ)
                    {
                        displayConfig.selectedWindow = m_previousSelectedWindow;
                    }

                    m_previousSelectedWindow = MAX_CHANNEL_FOR_SEQ;
                    applyNewStyle(MAIN_DISPLAY, displayConfig);
                }
            }
            break;

            default:
            {
                /* Nothing to do */
            }
            break;
        }

        emit sigToolbarStyleChnageNotify(MAX_STYLE_TYPE);
    }
}

void Layout::closeButtonClicked(quint16 windowIndex)
{
    setSelectedWindow(windowIndex);

    if (currentModeType[MAIN_DISPLAY] == STATE_COSEC_FEATURE)
    {
        closeCosecPopupFeature();
        return;
    }

    if (currentModeType[MAIN_DISPLAY] == STATE_VIDEO_POPUP_FEATURE)
    {
        closeVideoPopupFeatureWindow(windowIndex);
        return;
    }

    if ((ApplicationMode::getApplicationMode() == IDLE_MODE) || (ApplicationMode::getApplicationMode() == ASYNC_PLAYBACK_TOOLBAR_MODE))
    {
        if (streamInfoArray[MAIN_DISPLAY][windowIndex].m_videoType == VIDEO_TYPE_INSTANTPLAYBACKSTREAM)
        {
            m_instantPlaybackRequestSent[windowIndex] = true;
            stopInstantPlaybackFeature(windowIndex);
            return;
        }

        if ((m_liveViewToolbar != NULL) && (m_liveViewToolbar->getWindowIndex() != windowIndex))
        {
            slotCloseLiveViewToolbar();
            return;
        }

        if (streamInfoArray[MAIN_DISPLAY][windowIndex].m_videoType == VIDEO_TYPE_LIVESTREAM)
        {
            m_audioStopForWindow = windowIndex;
            if (m_timerIdForWindowSequence[MAIN_DISPLAY][windowIndex] != 0)
            {
                killTimer(m_timerIdForWindowSequence[MAIN_DISPLAY][windowIndex]);
                m_timerIdForWindowSequence[MAIN_DISPLAY][windowIndex] = 0;
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(LAYOUT_SEQ_STOP_FOR_MAIN_WINDOW));
            }

            currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].sequenceStatus = false;
            currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].sequenceInterval = DEFAULT_WINDOW_SEQ_INTERVAL;
        }
        else if (streamInfoArray[MAIN_DISPLAY][windowIndex].m_videoType == VIDEO_TYPE_PLAYBACKSTREAM)
        {
            m_audioStopForWindow = windowIndex;
            m_currentRecNo[windowIndex] = MAX_REC_ALLOWED;
            m_searchBtnClickRecNo[windowIndex] = MAX_REC_ALLOWED;
        }

        freeWindow(MAIN_DISPLAY, windowIndex, true);
        emit sigToolbarStyleChnageNotify(MAX_STYLE_TYPE);
    }
}

//add camera button clicked
void Layout::addCameraButtonClicked(quint16 windowIndex)
{
    if (true == currentDisplayConfig[MAIN_DISPLAY].seqStatus)
    {
        if ((ApplicationMode::getApplicationMode() == IDLE_MODE) &&(currentModeType[MAIN_DISPLAY] != STATE_LOCAL_DECODING))
        {
            MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SEQUENCING_ON_MSG));
        }
        return;
    }

    if((ApplicationMode::getApplicationMode() == IDLE_MODE) && (currentModeType[MAIN_DISPLAY] == STATE_OTHER_MODE_NONE))
    {
        emit sigUnloadToolbar();
        setSelectedWindow(windowIndex);

        if (m_viewCamera == NULL)
        {
            /* PARASOFT: Memory Deallocated in slot ViewCameraPageClose */
            m_inVisibleWidget = new InvisibleWidgetCntrl(this->window());

            /* PARASOFT: Memory Deallocated in slot ViewCameraPageClose */
            m_viewCamera = new ViewCamera(this->window(), windowIndex);
            connect(m_viewCamera,
                    SIGNAL(sigSwapWindows(quint16, quint16)),
                    this,
                    SLOT(slotSwapWindows(quint16, quint16)));
            connect(m_viewCamera,
                    SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                    this,
                    SLOT(slotViewCameraPageClose(TOOLBAR_BUTTON_TYPE_e)));
            connect(m_viewCamera,
                    SIGNAL(sigStartStreamInWindow(DISPLAY_TYPE_e, QString, quint8, quint16)),
                    this,
                    SLOT(slotStartStreamInWindow(DISPLAY_TYPE_e, QString, quint8, quint16)));
        }
    }
}

void Layout::expandWindow(quint16 windowIndex)
{
    if (isPlaybackRunning())
    {
        MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(PB_RUNNING_MESSAGE));
    }
    else if ((ApplicationMode::getApplicationMode() == IDLE_MODE) && (currentModeType[MAIN_DISPLAY] == STATE_OTHER_MODE_NONE)
            && (currentDisplayConfig[MAIN_DISPLAY].seqStatus == false) && (isAnyWindowReplacing(MAIN_DISPLAY) == false))
    {
        setSelectedWindow(windowIndex);
        DISPLAY_CONFIG_t displayConfig;
        memset(&displayConfig, 0, sizeof(DISPLAY_CONFIG_t));
        memcpy(&prevDisplayConfig, &currentDisplayConfig[MAIN_DISPLAY], sizeof(DISPLAY_CONFIG_t));

        if ((m_doubleClickModeFlag == true) && (m_previousLayout[MAIN_DISPLAY] != MAX_LAYOUT))
        {
            previouslayoutId = m_previousLayout[MAIN_DISPLAY];
        }

        quint8 configWindow = 0;

        for (quint8 index = 0; index < MAX_WIN_SEQ_CAM; index++)
        {
            if ((currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[index].deviceName[0] != '\0')
                    && (currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[index].defChannel != INVALID_CAMERA_INDEX))
            {
                displayConfig.windowInfo[configWindow].camInfo[0].defChannel
                        = currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[index].defChannel;
                snprintf(displayConfig.windowInfo[configWindow].camInfo[0].deviceName, MAX_DEVICE_NAME_SIZE, "%s",
                        currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[index].deviceName);
                configWindow++;
            }
        }

        currentDisplayConfig[MAIN_DISPLAY].seqStatus = false;
        currentDisplayConfig[MAIN_DISPLAY].seqInterval = 0;
        memcpy(&currentDisplayConfig[MAIN_DISPLAY].windowInfo, &displayConfig.windowInfo, sizeof(currentDisplayConfig[MAIN_DISPLAY].windowInfo));
        shiftWindows(MAIN_DISPLAY, 0, FOUR_X_FOUR);
        changeCurrentMode(STATE_EXPAND_WINDOW_START_MODE, MAIN_DISPLAY);
    }
}

void Layout::collapseWindow()
{
    m_expandMode = false;
    currentDisplayConfig[MAIN_DISPLAY].seqStatus = prevDisplayConfig.seqStatus;
    currentDisplayConfig[MAIN_DISPLAY].seqInterval = prevDisplayConfig.seqInterval;
    memcpy(&currentDisplayConfig[MAIN_DISPLAY].windowInfo, &prevDisplayConfig.windowInfo, sizeof(currentDisplayConfig[MAIN_DISPLAY].windowInfo));
    shiftWindows(MAIN_DISPLAY, prevDisplayConfig.selectedWindow, prevDisplayConfig.layoutId);

    if ((m_doubleClickModeFlag == true) && (m_previousLayout[MAIN_DISPLAY] != MAX_LAYOUT))
    {
        m_previousLayout[MAIN_DISPLAY] = previouslayoutId;
    }

    changeCurrentMode(STATE_COLLAPSE_WINDOW_START_MODE, MAIN_DISPLAY);
    resumeWindowSequencing(MAIN_DISPLAY);

    // Only for 0 window ID start Audio here because not getting stream again on collapse if expand for
    // window id 0 camera, And another window start after retry so put on start stream response
    if ((lastEnableAudioWindow == AUDIO_ON_PAGE_CHANGE) && (m_isClientAudProcessRunning == false))
    {
        includeExcludeAudio(lastEnableAudioWindow,true);
    }
}

void Layout::openToolbarButtonClicked(quint16 windowIndex)
{
    if ((ApplicationMode::getApplicationMode() == IDLE_MODE) && (currentModeType[MAIN_DISPLAY] == STATE_OTHER_MODE_NONE)
            && (streamInfoArray[MAIN_DISPLAY][windowIndex].m_videoType == VIDEO_TYPE_LIVESTREAM)
       && (streamInfoArray[MAIN_DISPLAY][windowIndex].m_errorType != VIDEO_ERROR_DISABLECAMERA))
    {
        if (m_liveViewToolbar == NULL)
        {
            setSelectedWindow(windowIndex);
            WIN_DIMENSION_INFO_t windowInfo = {0,0,0,0};
            layoutCreator->getWindowDimensionInfo(windowIndex, &windowInfo);

            /* PARASOFT: Memory Deallocated in slot CloseLiveViewToolbar */
            m_liveViewToolbar = new LiveViewToolbar((windowInfo.winStartx + SCALE_WIDTH(2)), (windowInfo.winStarty + SCALE_HEIGHT(2)),
                                                    windowInfo.winWidth, windowInfo.winHeight, windowIndex,
                                                    layoutCreator->getWindowIconType(windowIndex), this);
            connect(m_liveViewToolbar,
                    SIGNAL(sigCloseLiveViewToolbar()),
                    this,
                    SLOT(slotCloseLiveViewToolbar()));
            connect(m_liveViewToolbar,
                    SIGNAL(sigToolbarButtonClicked(LIVEVIEW_TOOLBAR_BUTTON_TYPE_e,STATE_TYPE_e,quint16)),
                    this,
                    SLOT(slotLiveViewToolbarButtonClicked(LIVEVIEW_TOOLBAR_BUTTON_TYPE_e,STATE_TYPE_e,quint16)));
            ApplicationMode::setApplicationMode(ASYNC_PLAYBACK_TOOLBAR_MODE);
        }
    }
    else if ((currentDisplayConfig[MAIN_DISPLAY].seqStatus == true) && (ApplicationMode::getApplicationMode() == IDLE_MODE)
            && (currentModeType[MAIN_DISPLAY] != STATE_LOCAL_DECODING))
    {
        MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SEQUENCING_ON_MSG));
    }
}

void Layout::changePage(DISPLAY_TYPE_e displayType, NAVIGATION_TYPE_e navigationType)
{
    if (isNextPageAvailable(displayType, currentDisplayConfig[displayType].currPage, navigationType))
    {
        slotCloseLiveViewToolbar();
        pauseSequencing(displayType);
        pauseWindowSequencing(displayType);

        if (((currentModeType[displayType] == STATE_OTHER_MODE_NONE) && (isAnyWindowReplacing(displayType) == false))
                || (currentModeType[displayType] == STATE_LOCAL_DECODING))
        {
            m_previousSelectedWindow = MAX_CHANNEL_FOR_SEQ;
            changeCurrentMode(STATE_CHANGE_PAGE, displayType);
        }
        else
        {
            if (isAnyWindowReplacing(displayType) == true)
            {
                m_processPendingForWindow[displayType] = true;
            }

            m_changePageLaterFlag[displayType] = true;

            if (navigationType == NEXT_PAGE_NAVIGATION)
            {
                m_changePageOffset[displayType]++;
            }
            else
            {
                m_changePageOffset[displayType]--;
            }
        }

        if (displayType == MAIN_DISPLAY)
        {
            stopAutoPlayFeature();
        }
    }
    else if ((displayType == MAIN_DISPLAY) && (m_isPreviousPageNaviagtion))
    {
        m_isPreviousPageNaviagtion = false;
    }
}

void Layout::changeWindowChannel(DISPLAY_TYPE_e displayType, quint16 windowIndex)
{
    DPRINT(LAYOUT, "change window: [startRequestPending=%d], [stopRequestPending=%d], [currentModeType=%s], [streamId=%d]",
           streamInfoArray[displayType][windowIndex].m_startRequestPending, streamInfoArray[displayType][windowIndex].m_stopRequestPending,
           modString[currentModeType[displayType]].toUtf8().constData(), streamInfoArray[displayType][windowIndex].m_streamId);

    if ((streamInfoArray[displayType][windowIndex].m_startRequestPending == false)
            && (streamInfoArray[displayType][windowIndex].m_stopRequestPending == false)
            && (currentModeType[displayType] == STATE_OTHER_MODE_NONE))
    {
        quint8 channelIndex = currentDisplayConfig[displayType].windowInfo[windowIndex].currentChannel;
        if (false == isNextChannelAvailableForWindow(displayType, windowIndex, channelIndex))
        {
            return;
        }

        if (m_timerIdForWindowSequence[displayType][windowIndex] != 0)
        {
            killTimer(m_timerIdForWindowSequence[displayType][windowIndex]);
            m_timerIdForWindowSequence[displayType][windowIndex] = 0;
        }

        channelIndex = updateCurrentChannel(displayType, windowIndex, channelIndex);
        if (channelIndex >= MAX_WIN_SEQ_CAM)
        {
            return;
        }

        currentDisplayConfig[displayType].windowInfo[windowIndex].currentChannel = channelIndex;
        if (streamInfoArray[displayType][windowIndex].m_streamId != MAX_STREAM_SESSION)
        {
             replaceLiveStream(displayType, windowIndex, currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[channelIndex].deviceName,
                               currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[channelIndex].defChannel, windowIndex);
        }
        else
        {
            deAllocateWindow(displayType, windowIndex);
            startLiveStream(displayType, currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[channelIndex].deviceName,
                            currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[channelIndex].defChannel, windowIndex);
        }

        processNextActionForWindow(displayType, windowIndex);
    }
}

void Layout::refreshCurrentPage(DISPLAY_TYPE_e displayType)
{
    //stop default sequencing
    pauseSequencing(displayType);

    //stop Window sequencing
    pauseWindowSequencing(displayType);

    if ((displayType < MAIN_DISPLAY) || (displayType >= MAX_DISPLAY_TYPE))
    {
        return;
    }

    if (((currentModeType[displayType] == STATE_OTHER_MODE_NONE)||(currentModeType[displayType] == STATE_LOCAL_DECODING))
            && (isAnyWindowReplacing(displayType) == false))
    {
        changeCurrentMode(STATE_REFRESH_PAGE, displayType);
    }
    else
    {
        if (isAnyWindowReplacing(displayType) == true)
        {
            m_processPendingForWindow[displayType] = true;
        }

        m_refreshPageLaterFlag[displayType] = true;
    }
}

void Layout::selectWindow(DISPLAY_TYPE_e displayType, quint16 windowIndex)
{
    currentDisplayConfig[displayType].selectedWindow = windowIndex;
    layoutCreator->changeSelectedWindow(windowIndex);
}

void Layout::giveFocusToWindow(quint16 windowIndex)
{
    if (windowIndex < MAX_CHANNEL_FOR_SEQ)
    {
        layoutCreator->changeSelectedWindow(windowIndex, true);
    }
}

void Layout::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    switch (param->msgType)
    {
        case MSG_SET_CMD:
        {
            switch (param->cmdType)
            {
                case STP_MAN_PTZ_TOUR:
                case SRT_MAN_PTZ_TOUR:
                case SETPANTILT:
                case SETZOOM:
                case SETFOCUS:
                case SETIRIS:
                case CALLPRESET:
                case RESUME_PTZ_TOUR:
                case PTZ_TOUR_STATUS:
                {
                    if (m_ptzControl != NULL)
                    {
                        m_ptzControl->processDeviceResponse(param, deviceName);
                    }
                }
                break;

                case SET_PRESET:
                {
                    if (m_analogPresetMenu != NULL)
                    {
                        m_analogPresetMenu->processDeviceResponse(param, deviceName);
                    }
                }
                break;

                case SRT_MAN_REC:
                {
                    if (param->deviceStatus == CMD_SUCCESS)
                    {
                        MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(LAYOUT_MANUAL_REC_START));
                    }
                    else
                    {
                        MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                    }
                }
                break;

                case STP_MAN_REC:
                {
                    if (param->deviceStatus == CMD_SUCCESS)
                    {
                        MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(LAYOUT_MANUAL_REC_STOP));
                    }
                    else
                    {
                        MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                    }
                }
                break;

                case COSEC_VIDEO_POPUP:
                {
                    if (m_cosecVideoPopupUser != NULL)
                    {
                        m_cosecVideoPopupUser->processDeviceResponse(param, deviceName);
                    }
                }
                break;

                case OTHR_SUP:
                {
                    if (param->deviceStatus == CMD_SUCCESS)
                    {
                        payloadLib->parseDevCmdReply(true, param->payload);

                        if (payloadLib->getCnfgArrayAtIndex(2).toBool())
                        {
                            createPTZControl();
                        }
                        else
                        {
                            m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LAYOUT_PTZ_NOT_SUPPORTED));
                        }
                    }
                    else
                    {
                        m_infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                    }
                }
                break;

                case CHK_PRIV:
                {
                    if (param->deviceStatus == CMD_SUCCESS)
                    {
                        bool audioInRight;
                        bool audioStatus;
                        payloadLib->parseDevCmdReply(true,param->payload);
                        audioInRight = payloadLib->getCnfgArrayAtIndex(1).toBool();
                        audioStatus = payloadLib->getCnfgArrayAtIndex(2).toBool();

                        if (audioInRight == false)
                        {
                            MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(CMD_NO_PRIVILEGE));
                        }
                        else if (audioStatus == false)
                        {
                            MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(CMD_AUDIO_DISABLED));
                        }
                        else
                        {
                            lastEnableAudioWindow = m_audIndexToPlay;
                            emit sigStopClntAud(CMD_AUD_SND_STP_PRO_LCL_CLNT_REQ);
                        }

                        m_audIndexToPlay = MAX_CHANNEL_FOR_SEQ;
                    }
                }
                break;

                default:
                {
                    /* Nothing to do */
                }
                break;
            }
        }
        break;

        case MSG_SET_CFG:
        {
            if (m_ptzControl != NULL)
            {
                m_ptzControl->processDeviceResponse(param, deviceName);
            }
        }
        break;

        case MSG_GET_CFG:
        {
            if (m_ptzControl != NULL)
            {
                m_ptzControl->processDeviceResponse(param, deviceName);
                break;
            }

            if (deviceName != LOCAL_DEVICE_NAME)
            {
                break;
            }

            /* Stop client audio and Play camera audio if priority is of camera in audio out page */
            payloadLib->parsePayload (param->msgType, param->payload);
            if (payloadLib->getcnfgTableIndex(0) == AUDIO_OUT_TABLE_INDEX)
            {
                if (payloadLib->getCnfgArrayAtIndex(0).toUInt() == CAMERA_AUDIO_PRIORITY)
                {
                    checkUserPriviledge();
                }
                else
                {
                    m_audIndexToPlay = MAX_CHANNEL_FOR_SEQ;
                    MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(CMD_REQ_FAIL_CHNG_AUD_OUT_PRI));
                }
            }
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void Layout::processDeviceResponseForAnalogCameraFeature()
{
    if (previousModeType[MAIN_DISPLAY] == STATE_OTHER_MODE_PROCESSING_STOP_REQUEST)
    {
        if (analogFeatureDataType.m_currentFeatureState == FEATURE_STATE_STOP_OLD_WINDOW)
        {
            if (m_cosecHandleLaterFlag == true)
            {
                cosecPopupEvtData.m_previousSelectedWindow = analogFeatureDataType.m_previousSelectedWindow;
                cosecPopupEvtData.m_previousSelectedLayout = analogFeatureDataType.m_previousSelectedLayout;
                closeAnalogCameraFeature(false);
                m_cosecHandleLaterFlag = false;
                updateCosecPopupParam(false);
            }
            else
            {
                analogFeatureDataType.m_currentFeatureState = FEATURE_STATE_START_VIDEO;
                startStreamForCurrentPage(MAIN_DISPLAY);
            }
        }
        else if (analogFeatureDataType.m_currentFeatureState == FEATURE_STATE_STOP_VIDEO)
        {
            analogFeatureDataType.m_currentFeatureState = FEATURE_STATE_START_OLD_WINDOW;
            startStreamForCurrentPage(MAIN_DISPLAY);
        }
    }
    else if (previousModeType[MAIN_DISPLAY] == STATE_OTHER_MODE_PROCESSING_START_REQUEST)
    {
        if (analogFeatureDataType.m_currentFeatureState == FEATURE_STATE_START_VIDEO)
        {
            if (m_cosecHandleLaterFlag == true)
            {
                cosecPopupEvtData.m_previousSelectedWindow = analogFeatureDataType.m_previousSelectedWindow;
                cosecPopupEvtData.m_previousSelectedLayout = analogFeatureDataType.m_previousSelectedLayout;
                closeAnalogCameraFeature(false);
                m_cosecHandleLaterFlag = false;
                updateCosecPopupParam(false);
            }
            else
            {
                analogFeatureDataType.m_currentFeatureState = FEATURE_STATE_VIDEO_ON;
                startAnalogCameraFeature();
            }
        }
        else if (analogFeatureDataType.m_currentFeatureState == FEATURE_STATE_START_OLD_WINDOW)
        {
            analogFeatureDataType.m_currentFeatureState = FEATURE_STATE_NONE;
            stopAnalogCameraFeature();
            changeCurrentMode(STATE_OTHER_MODE_NONE, MAIN_DISPLAY);
        }
    }
}

void Layout::processDeviceResponseForCosecPopUp()
{
    if (previousModeType[MAIN_DISPLAY] == STATE_OTHER_MODE_PROCESSING_STOP_REQUEST)
    {
        if (cosecPopupEvtData.m_currentFeatureState == FEATURE_STATE_STOP_OLD_WINDOW)
        {
            if (cosecPopupEvtData.m_nextCosecPending == true)
            {
                updateNextParamToCurrent();
            }

            cosecPopupEvtData.m_currentFeatureState = FEATURE_STATE_START_VIDEO;
            startStreamForCurrentPage(MAIN_DISPLAY);
        }
        else if (cosecPopupEvtData.m_currentFeatureState == FEATURE_STATE_STOP_VIDEO)
        {
            cosecPopupEvtData.m_currentFeatureState = FEATURE_STATE_START_OLD_WINDOW;
            startStreamForCurrentPage(MAIN_DISPLAY);
        }
    }
    else if (previousModeType[MAIN_DISPLAY] == STATE_OTHER_MODE_PROCESSING_START_REQUEST)
    {
        if (cosecPopupEvtData.m_currentFeatureState == FEATURE_STATE_START_VIDEO)
        {
            if (cosecPopupEvtData.m_nextCosecPending == true)
            {
                updateNextParamToCurrent();
                cosecPopupEvtData.m_currentFeatureState = FEATURE_STATE_STOP_OLD_WINDOW;
                changeCurrentMode(STATE_COSEC_FEATURE, MAIN_DISPLAY);
            }
            else
            {
                cosecPopupEvtData.m_currentFeatureState = FEATURE_STATE_VIDEO_ON;
                startCosecPopupFeature();
            }
        }
        else if (cosecPopupEvtData.m_currentFeatureState == FEATURE_STATE_START_OLD_WINDOW)
        {
            cosecPopupEvtData.m_currentFeatureState = FEATURE_STATE_NONE;
            changeCurrentMode(STATE_OTHER_MODE_NONE, MAIN_DISPLAY);
            stopCosecPopupFeature();
        }
    }
}

void Layout::processDeviceResponseForVideoPopUp()
{
    quint16 windowLimits[2] = {0,MAX_CHANNEL_FOR_SEQ};

    if (previousModeType[MAIN_DISPLAY] == STATE_OTHER_MODE_PROCESSING_STOP_REQUEST)
    {
        if (videoPopupData.m_currentFeatureState == VIDEO_POPUP_STATE_STOP_OLD_WINDOW_PROCESSING)
        {
            videoPopupData.m_currentFeatureState = VIDEO_POPUP_STATE_START_VIDEO;
            startStreamForCurrentPage(MAIN_DISPLAY);
            videoPopupData.m_currentFeatureState = VIDEO_POPUP_STATE_START_VIDEO_WAIT;
        }
        else if (videoPopupData.m_currentFeatureState == VIDEO_POPUP_STATE_STOP_VIDEO)
        {
            m_videoPopupMode = false;
            updateCurrentConfigurationForVideoPopup();
            videoPopupData.m_currentFeatureState = VIDEO_POPUP_STATE_START_OLD_WINDOW;
            shiftWindows(MAIN_DISPLAY, videoPopupData.m_previousSelectedWindow, videoPopupData.m_previousSelectedLayout);

            if (!videoPopupData.m_wasExpandMode)
            {
                selectWindow(MAIN_DISPLAY, videoPopupData.m_previousSelectedWindow);
            }

            changeCurrentMode(STATE_OTHER_MODE_PROCESSING_START_REQUEST, MAIN_DISPLAY);
            getFirstAndLastWindow(currentDisplayConfig[MAIN_DISPLAY].currPage, currentDisplayConfig[MAIN_DISPLAY].layoutId, windowLimits);

            for (quint16 windowIndex = windowLimits[0]; windowIndex <= windowLimits[1]; windowIndex++)
            {
                quint8 channelIndex = currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].currentChannel;

                if ((currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[channelIndex].deviceName[0] != '\0')
                        && (streamInfoArray[MAIN_DISPLAY][windowIndex].m_videoType == VIDEO_TYPE_NONE))
                {
                     startLiveStream(MAIN_DISPLAY, currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[channelIndex].deviceName,
                                     currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[channelIndex].defChannel, windowIndex);
                }
            }

            processNextActionForOtherMode(MAIN_DISPLAY);
        }
    }
    else if (previousModeType[MAIN_DISPLAY] == STATE_OTHER_MODE_PROCESSING_START_REQUEST)
    {
        if (videoPopupData.m_currentFeatureState == VIDEO_POPUP_STATE_START_VIDEO
                || (videoPopupData.m_currentFeatureState == VIDEO_POPUP_STATE_START_VIDEO_WAIT))
        {
            videoPopupData.m_currentFeatureState = VIDEO_POPUP_STATE_VIDEO_ON;
            startVideoPopupFeature();
        }
        else if (videoPopupData.m_currentFeatureState == VIDEO_POPUP_STATE_START_OLD_WINDOW)
        {
            videoPopupData.m_currentFeatureState = VIDEO_POPUP_STATE_NONE;
            if (!videoPopupData.m_wasExpandMode)
            {
                emit sigShowHideStyleAndPageControl(true, STATE_VIDEO_POPUP_FEATURE);
                changeCurrentMode(STATE_OTHER_MODE_NONE, MAIN_DISPLAY);
            }
            else
            {
                changeCurrentMode(STATE_EXPAND_WINDOW, MAIN_DISPLAY);
            }

            if (videoPopupData.m_startAudioInWindow != MAX_CHANNEL_FOR_SEQ)
            {
                startAudioInWindow = videoPopupData.m_startAudioInWindow;
                videoPopupData.m_startAudioInWindow = MAX_CHANNEL_FOR_SEQ;
            }

            if (videoPopupData.m_previousBeforeOneXOne != MAX_LAYOUT)
            {
                 m_doubleClickModeFlag = true;
                 m_previousLayout[MAIN_DISPLAY] = videoPopupData.m_previousBeforeOneXOne;
            }

            if ((localDecodingFeature.pendingRequestForLoalDecoding == true) && (videoPopupData.m_wasExpandMode == true))
            {
                collapseWindow();
            }

            resetVideoPopupFeature();
            loadOptionsInVideoPopup();
        }
    }
}

void Layout::processStreamRequestResponse(STREAM_COMMAND_TYPE_e streamCommandType, StreamRequestParam *streamRequestParam, DEVICE_REPLY_TYPE_e deviceReply)
{
    DISPLAY_TYPE_e  displayType = streamRequestParam->displayType;
    QString         deviceName = streamRequestParam->deviceName;
    quint16         windowIndex = streamRequestParam->actualWindowId;
    quint8          channelId = streamRequestParam->channelId;
    quint8          streamId = streamRequestParam->streamId;
    quint64         playbackTime;
    qint64          timeStamp = streamRequestParam->timeStamp;
    bool            isInstancePlayBackStopRequire = false;
    QString         m_getUserNameforAudio;

    DPRINT(LAYOUT, "stream command response: [device=%s], [windowIndex=%d], [streamId=%d], [cameraId=%d], [cmd=%s], [resp=%d]",
           deviceName.toUtf8().constData(), windowIndex, streamId, channelId, streamCmdString[streamCommandType].toUtf8().constData(), deviceReply);

    if (windowIndex >= MAX_CHANNEL_FOR_SEQ)
    {
        EPRINT(LAYOUT, "invld window index: [windowIndex=%d]", windowIndex);
        return;
    }

    switch (streamCommandType)
    {
        case START_STREAM_COMMAND:
        {
            if (QString::localeAwareCompare(streamInfoArray[displayType][windowIndex].m_deviceName, deviceName) != 0)
            {
                break;
            }

            if ((streamInfoArray[displayType][windowIndex].m_cameraId != channelId) || (streamInfoArray[displayType][windowIndex].timeStamp != timeStamp))
            {
                break;
            }

            switch (deviceReply)
            {
                case CMD_STREAM_CONNECTING:
                {
                    changeStreamStateOfWindow(displayType, windowIndex, deviceName, channelId, streamId, VIDEO_TYPE_LIVESTREAM,
                                              VIDEO_STATUS_CONNECTING, VIDEO_ERROR_NONE);
                }
                break;

                case CMD_SUCCESS:
                case CMD_STREAM_NO_VIDEO_LOSS:
                {
                    changeStreamStateOfWindow(displayType, windowIndex, deviceName, channelId, streamId, VIDEO_TYPE_LIVESTREAM,
                                              VIDEO_STATUS_RUNNING, VIDEO_ERROR_NONE);
                    audioOnAfterPageChanges(windowIndex);
                }
                break;

                case CMD_STREAM_VIDEO_LOSS:
                {
                    if (windowIndex == m_activeMicroPhoneWindow)
                    {
                        streamInfoArray[MAIN_DISPLAY][windowIndex].m_microPhoneStatus = false;
                        m_activeMicroPhoneWindow = MAX_CHANNEL_FOR_SEQ;
                        layoutCreator->updateWindowData(windowIndex);
                    }

                    changeStreamStateOfWindow(displayType, windowIndex, deviceName, channelId, streamId, VIDEO_TYPE_LIVESTREAM,
                                              VIDEO_STATUS_VIDEOLOSS, VIDEO_ERROR_VIDEOLOSS);
                    changeWinAudioStatus(windowIndex);
                }
                break;

                case CMD_NO_PRIVILEGE:
                {
                    changeStreamStateOfWindow(displayType, windowIndex, deviceName, channelId, MAX_STREAM_SESSION, VIDEO_TYPE_LIVESTREAM,
                                              VIDEO_STATUS_ERROR, VIDEO_ERROR_NOUSERRIGHTS);
                }
                break;

                case CMD_CHANNEL_DISABLED:
                {
                    changeStreamStateOfWindow(displayType, windowIndex, deviceName, channelId, MAX_STREAM_SESSION, VIDEO_TYPE_LIVESTREAM,
                                              VIDEO_STATUS_EVENTWAIT, VIDEO_ERROR_DISABLECAMERA);
                }
                break;

                case CMD_CAM_DISCONNECTED:
                {
                    changeStreamStateOfWindow(displayType, windowIndex, deviceName, channelId, MAX_STREAM_SESSION, VIDEO_TYPE_LIVESTREAM,
                                              VIDEO_STATUS_EVENTWAIT, VIDEO_ERROR_CAMERADISCONNECTED);
                }
                break;

                case CMD_DECODER_ERROR:
                {
                    if (windowIndex == m_activeMicroPhoneWindow)
                    {
                        streamInfoArray[MAIN_DISPLAY][windowIndex].m_microPhoneStatus = false;
                        m_activeMicroPhoneWindow = MAX_CHANNEL_FOR_SEQ;
                        layoutCreator->updateWindowData(windowIndex);
                    }

                    changeStreamStateOfWindow(displayType, windowIndex, deviceName, channelId, MAX_STREAM_SESSION, VIDEO_TYPE_LIVESTREAM,
                                              VIDEO_STATUS_RETRY, VIDEO_ERROR_OTHERERROR);
                    changeWinAudioStatus(windowIndex);
                }
                break;

                case CMD_DECODER_CAPACITY_ERROR:
                {
                    if (windowIndex == m_activeMicroPhoneWindow)
                    {
                        streamInfoArray[MAIN_DISPLAY][windowIndex].m_microPhoneStatus = false;
                        m_activeMicroPhoneWindow = MAX_CHANNEL_FOR_SEQ;
                        layoutCreator->updateWindowData(windowIndex);
                    }
                    changeStreamStateOfWindow(displayType, windowIndex, deviceName, channelId, MAX_STREAM_SESSION, VIDEO_TYPE_LIVESTREAM,
                                              VIDEO_STATUS_RETRY, VIDEO_ERROR_NO_DECODING_CAP);
                    changeWinAudioStatus(windowIndex);
                }
                break;

                case CMD_MAX_STREAM_LIMIT:
                {
                    changeStreamStateOfWindow(displayType, windowIndex, deviceName, channelId, MAX_STREAM_SESSION, VIDEO_TYPE_LIVESTREAM,
                                              VIDEO_STATUS_RETRY, VIDEO_ERROR_OTHERERROR);
                    MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(CMD_MAX_STREAM_LIMIT));
                }
                break;

                case CMD_INTERNAL_RESOURCE_LIMIT:
                {
                    changeStreamStateOfWindow(displayType, windowIndex, deviceName, channelId, MAX_STREAM_SESSION, VIDEO_TYPE_LIVESTREAM,
                                              VIDEO_STATUS_RETRY, VIDEO_ERROR_OTHERERROR);
                    MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(CMD_INTERNAL_RESOURCE_LIMIT));
                }
                break;

                case CMD_STREAM_STOPPED:
                {
                    changeStreamStateOfWindow(displayType, windowIndex, "", INVALID_CAMERA_INDEX, MAX_STREAM_SESSION, VIDEO_TYPE_NONE,
                                              VIDEO_STATUS_NONE, VIDEO_ERROR_NONE);
                }
                break;

                default:
                {
                    changeStreamStateOfWindow(displayType, windowIndex, deviceName, channelId, MAX_STREAM_SESSION, VIDEO_TYPE_LIVESTREAM,
                                              VIDEO_STATUS_RETRY, VIDEO_ERROR_OTHERERROR);
                }
                break;
            }

            if (streamInfoArray[displayType][windowIndex].m_startRequestPending == true)
            {
                streamInfoArray[displayType][windowIndex].m_startRequestPending = false;
                if (pendingInstantRequestCount[displayType][windowIndex] > 0)
                {
                    m_processPendingForWindow[MAIN_DISPLAY] = false;
                    pendingInstantRequestCount[displayType][windowIndex]--;
                    if (pendingInstantRequestCount[displayType][windowIndex] == 0)
                    {
                        processNextActionForWindow(displayType, windowIndex, false, true);
                    }
                }
            }

            emit sigWindowResponseToDisplaySettingsPage(displayType, deviceName, channelId, windowIndex);
        }
        break;

        case CHANGE_STREAM_TYPE_COMMAND:
        {
            if (deviceReply == CMD_SUCCESS)
            {
                toggleStreamType(windowIndex, displayType);
            }
            else
            {
                MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(deviceReply));
            }
        }
        break;

        case INCLUDE_AUDIO_COMMAND:
        {
            if (deviceReply == CMD_SUCCESS)
            {
                audioResponse(true, streamId);
            }
            else
            {
                startAudioInWindow = MAX_CHANNEL_FOR_SEQ;
                MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(deviceReply));
                audioResponse(false, streamId);
            }
        }
        break;

        case EXCLUDE_AUDIO_COMMAND:
        {
            if (deviceReply == CMD_SUCCESS)
            {
                audioResponse(false, streamId);
            }
            else
            {
                if (startAudioInWindow != MAX_CHANNEL_FOR_SEQ)
                {
                    includeDevAudio(startAudioInWindow);
                }
                else
                {
                    MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(deviceReply));
                }
            }
        }
        break;

        case INCL_EXCL_AUDIO_IN_COMMAND:
        {
            if (deviceReply == CMD_SUCCESS)
            {
                if (true == streamRequestParam->audioStatus)
                {
                    DPRINT(LAYOUT, "microphone status changed to true");
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_microPhoneStatus = true;

                    /* update window icon */
                    layoutCreator->updateWindowData(windowIndex);
                }
                else
                {
                    if (MAX_CHANNEL_FOR_SEQ != m_activeMicroPhoneWindow)
                    {
                        sendIncludeExcludeDevAudioInCmd(m_activeMicroPhoneWindow, true);
                    }
                }
            }
            else
            {
                /* Reset Status and Active WindowId */
                m_activeMicroPhoneWindow = MAX_CHANNEL_FOR_SEQ;
                streamInfoArray[MAIN_DISPLAY][windowIndex].m_microPhoneStatus = false;

                /* update window icon */
                layoutCreator->updateWindowData(windowIndex);

                /* If audio channel is stopped from server side due to any reason
                 * then CMD_PROCESS_ERROR is received.  Don't show response message in this case */
                if (deviceReply != CMD_PROCESS_ERROR)
                {
                    MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(deviceReply));
                }
            }
        }
        break;

        case GET_PLAYBACK_ID_COMMAND:
        {
            switch (deviceReply)
            {
                case CMD_SUCCESS:
                {
                    payloadLib->parseDevCmdReply(true, streamRequestParam->payload);
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamId = payloadLib->getCnfgArrayAtIndex(0).toUInt();
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbCurrCmd = PB_PLAY_STATE;
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbNextCmd = PB_MAX_PLAYBACK_STATE;
                    streamInfoArray[MAIN_DISPLAY][windowIndex].prevPlayState = PB_PLAY_STATE;
                    changeStreamStateOfWindow(MAIN_DISPLAY, windowIndex, deviceName, channelId, streamId, VIDEO_TYPE_PLAYBACKSTREAM,
                                              VIDEO_STATUS_CONNECTING, VIDEO_ERROR_NONE);
                    //send play request
                    sendPbRequest(windowIndex);
                }
                break;

                case CMD_NO_PRIVILEGE:
                {
                    changeStreamStateOfWindow(MAIN_DISPLAY, windowIndex, deviceName, channelId, MAX_STREAM_SESSION,
                                              VIDEO_TYPE_PLAYBACKSTREAM, VIDEO_STATUS_ERROR, VIDEO_ERROR_NOUSERRIGHTS);
                }
                break;

                case CMD_DECODER_CAPACITY_ERROR:
                case CMD_DECODER_ERROR:
                {
                    playbackRecordData[windowIndex].clearPlaybackInfo();
                    deAllocateWindow(displayType, windowIndex);
                }
                break;

                case CMD_INTERNAL_RESOURCE_LIMIT:
                {
                    playbackRecordData[windowIndex].clearPlaybackInfo();
                    deAllocateWindow(displayType, windowIndex);
                    MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(LAYOUT_STREAM_LIMIT_ERROR_MESSAGE));
                }
                break;

                case CMD_MAX_STREAM_LIMIT:
                {
                    playbackRecordData[windowIndex].clearPlaybackInfo();
                    deAllocateWindow(displayType, windowIndex);
                    MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(deviceReply));
                }
                break;

                default:
                {
                    changeStreamStateOfWindow(MAIN_DISPLAY, windowIndex, deviceName, channelId, MAX_STREAM_SESSION,
                                              VIDEO_TYPE_PLAYBACKSTREAM, VIDEO_STATUS_ERROR, VIDEO_ERROR_OTHERERROR);
                }
                break;
            }

            if (streamInfoArray[displayType][windowIndex].m_startRequestPending == true)
            {
                streamInfoArray[displayType][windowIndex].m_startRequestPending = false;
                if (m_pendingRequestCount[displayType] > 0)
                {
                    m_pendingRequestCount[displayType]--;
                    if (m_pendingRequestCount[displayType] == 0)
                    {
                        processNextActionForOtherMode(displayType);
                    }
                }
            }
        }
        break;

        case PLAY_PLABACK_STREAM_COMMAND:
        {
            if ((streamInfoArray[MAIN_DISPLAY][windowIndex].m_streamId != streamId) || (deviceName == ""))
            {
                break;
            }

            switch (deviceReply)
            {
                case CMD_STREAM_CONNECTING:
                {
                    changeStreamStateOfWindow(MAIN_DISPLAY, windowIndex, deviceName, channelId, streamId,
                                              VIDEO_TYPE_PLAYBACKSTREAM, VIDEO_STATUS_CONNECTING, VIDEO_ERROR_NONE);
                }
                break;

                case CMD_SUCCESS:
                {
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbOver = false;
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamState = PB_PLAY_STATE;
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbCurrCmd = streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbNextCmd;
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbNextCmd = PB_MAX_PLAYBACK_STATE;
                    changeStreamStateOfWindow(MAIN_DISPLAY, windowIndex, deviceName, channelId, streamId,
                                              VIDEO_TYPE_PLAYBACKSTREAM, VIDEO_STATUS_RUNNING, VIDEO_ERROR_NONE);

                    if ((m_pbToolbar != NULL) && (m_pbToolbar->getPbToolbarWinId() == windowIndex))
                    {
                        if (m_stepIsRunning == false)
                        {
                            if (streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbDirection == FORWARD_PLAY)
                            {
                                m_pbToolbar->changeStateOfButton(PB_TOOLBAR_ELE_PLAY_BTN, false);
                                m_pbToolbar->changeStateOfButton(PB_TOOLBAR_ELE_REV_PLAY_BTN, true, false);

                                if (streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbSpeed == PB_SPEED_NORMAL)
                                {
                                    m_pbToolbar->changeButtonEnableState(PB_TOOLBAR_ELE_MUTE_BTN, true);
                                }
                                else
                                {
                                    m_pbToolbar->changeButtonEnableState(PB_TOOLBAR_ELE_MUTE_BTN, false);
                                }
                            }
                            else
                            {
                                m_pbToolbar->changeStateOfButton(PB_TOOLBAR_ELE_PLAY_BTN, true);
                                m_pbToolbar->changeStateOfButton(PB_TOOLBAR_ELE_REV_PLAY_BTN, false, false);
                                m_pbToolbar->changeButtonEnableState(PB_TOOLBAR_ELE_MUTE_BTN, false);
                            }
                        }
                    }

                    sendPbRequest(windowIndex);

                    //Audio get resume in case of Asynchronous playback and page changes
                    if (lastEnableAudioWindow == windowIndex)
                    {
                        if (m_isClientAudProcessRunning == false)
                        {
                            includeExcludeAudio(lastEnableAudioWindow);
                        }
                    }
                }
                break;

                case CMD_PLAYBACK_TIME:
                {
                    playbackTime = streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbCurrentTime;
                    if (m_pbSliderChange == false)
                    {
                        playbackTime = streamRequestParam->payload.toULongLong();
                        streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbCurrentTime = playbackTime;
                    }
                    else
                    {
                        m_pbSliderChange = false;
                    }

                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbCurrTimeStr = mSecToString(playbackTime);

                    if ((m_pbToolbar != NULL) && (m_pbToolbar->getPbToolbarWinId() == windowIndex))
                    {
                        m_pbToolbar->showPbSpeedDateTime(windowIndex);
                        m_pbToolbar->updateCurrTime(playbackTime);
                    }
                }
                break;

                case CMD_STREAM_NO_VIDEO_LOSS:
                {
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamState = PB_PLAY_STATE;
                    changeStreamStateOfWindow(MAIN_DISPLAY, windowIndex, deviceName, channelId, streamId,
                                              VIDEO_TYPE_PLAYBACKSTREAM, VIDEO_STATUS_RUNNING, VIDEO_ERROR_NONE);
                }
                break;

                case CMD_STREAM_VIDEO_LOSS:
                {
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamState = PB_PLAY_STATE;
                    changeStreamStateOfWindow(MAIN_DISPLAY, windowIndex, deviceName, channelId, streamId,
                                              VIDEO_TYPE_PLAYBACKSTREAM, VIDEO_STATUS_VIDEOLOSS, VIDEO_ERROR_VIDEOLOSS);
                }
                break;

                case CMD_STREAM_PLAYBACK_OVER:
                {
                    // For update next-prev button in Async Playback
                    if (m_currentRecNo[windowIndex] != MAX_REC_ALLOWED)
                    {
                        /* While changing style, playback is running and its stop stream req sent, then if in between if play playback stream
                         * response came then it will change next cmd to PB_MAX_PLAYBACK_STATE so clear playback command will not send and
                         * subsequently pending count was not decreased. */
                        closeDeletePbToolBar();
                        streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbCurrentTime = streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStartTime;
                        if (previousModeType[MAIN_DISPLAY] != STATE_APPLY_NEW_STYLE)
                        {
                            streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamState = PB_STEP_STATE;
                            streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbCurrCmd = PB_STEP_STATE;
                            streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbNextCmd = PB_MAX_PLAYBACK_STATE;
                            streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbDirection = FORWARD_PLAY;
                            streamInfoArray[MAIN_DISPLAY][windowIndex].m_iFrameNeeded = true;
                            streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbToolbarVisible = true;
                        }

                        emit sigGetNextPrevRecord(m_currentRecNo[windowIndex], windowIndex, streamInfoArray[MAIN_DISPLAY][windowIndex].m_cameraId);
                        autoPlayData.autoPlayFeatureDataType[windowIndex].m_isApToolbarVisible = true;
                        autoPlayData.autoPlayFeatureDataType[windowIndex].m_timerCount = 0;
                        if (autoPlayData.m_timerIdForAutoPlay == 0)
                        {
                            autoPlayData.m_timerIdForAutoPlay = startTimer(1000);
                        }

                        changeStreamStateOfWindow(MAIN_DISPLAY, windowIndex, deviceName, channelId, streamId,
                                                  VIDEO_TYPE_PLAYBACKSTREAM, VIDEO_STATUS_RUNNING, VIDEO_ERROR_NONE);
                    }
                    else
                    {
                        if ((m_pbToolbar != NULL) && (m_pbToolbar->getPbToolbarWinId() == windowIndex))
                        {
                            m_pbToolbar->updateCurrTime(streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStartTime);
                        }

                        streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbCurrentTime = streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStartTime;
                        streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamState = PB_STEP_STATE;
                        streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbCurrCmd = PB_STEP_STATE;
                        streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbNextCmd = PB_MAX_PLAYBACK_STATE;
                        streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbDirection = FORWARD_PLAY;
                        streamInfoArray[MAIN_DISPLAY][windowIndex].m_iFrameNeeded = true;
                        streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbOver = false;
                        streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbToolbarVisible = false;
                        autoPlayData.autoPlayFeatureDataType[windowIndex].m_isApToolbarVisible = false;
                        sendPbRequest(windowIndex);
                    }

                    if (startAudioInWindow == windowIndex)
                    {
                        lastEnableAudioWindow = startAudioInWindow;
                        includeExcludeAudio(lastEnableAudioWindow, true);
                    }
                }
                break;

                case CMD_STREAM_STOPPED:
                {
                    /* While changing style,playback is running and its stop stream req sent,then if in between if play playback stream
                     * response came then it will change next cmd to PB_MAX_PLAYBACK_STATE so clear playback command will not send and
                     * subsequently pending count was not decreased. */
                    if (previousModeType[MAIN_DISPLAY] == STATE_APPLY_NEW_STYLE)
                    {
                        break;
                    }

                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamState = PB_PAUSE_STATE;
                    if ((m_pbToolbar != NULL) && (m_pbToolbar->getPbToolbarWinId() == windowIndex))
                    {
                        if (m_stepIsRunning == false)
                        {
                            if (streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbDirection == FORWARD_PLAY)
                            {
                                m_pbToolbar->changeStateOfButton(PB_TOOLBAR_ELE_PLAY_BTN, true);
                            }
                            else
                            {
                                m_pbToolbar->changeStateOfButton(PB_TOOLBAR_ELE_REV_PLAY_BTN, true);
                            }
                        }
                        else
                        {
                            if (!m_pbToolbar->getStateOfButton(PB_TOOLBAR_ELE_PLAY_BTN))
                            {
                                m_pbToolbar->changeStateOfButton(PB_TOOLBAR_ELE_PLAY_BTN, true);
                            }

                            if (!m_pbToolbar->getStateOfButton(PB_TOOLBAR_ELE_REV_PLAY_BTN))
                            {
                                m_pbToolbar->changeStateOfButton(PB_TOOLBAR_ELE_REV_PLAY_BTN, true);
                            }
                        }
                    }

                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbCurrCmd = streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbNextCmd;
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbNextCmd = PB_MAX_PLAYBACK_STATE;
                    sendPbRequest(windowIndex);
                }
                break;

                case CMD_DECODER_CAPACITY_ERROR:
                case CMD_DECODER_ERROR:
                /* FALLS THROUGH */
                default:
                {
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamState = PB_PAUSE_STATE;
                    if ((m_pbToolbar != NULL) && (m_pbToolbar->getPbToolbarWinId() == windowIndex))
                    {
                        if (streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbDirection == FORWARD_PLAY)
                        {
                            m_pbToolbar->changeStateOfButton(PB_TOOLBAR_ELE_PLAY_BTN, true);
                        }
                        else
                        {
                            m_pbToolbar->changeStateOfButton(PB_TOOLBAR_ELE_REV_PLAY_BTN, true);
                        }
                    }

                    VIDEO_ERROR_TYPE_e tVideoError = VIDEO_ERROR_OTHERERROR;
                    if (CMD_DECODER_CAPACITY_ERROR == deviceReply)
                    {
                        tVideoError = VIDEO_ERROR_NO_DECODING_CAP;
                    }

                    changeStreamStateOfWindow(MAIN_DISPLAY, windowIndex, deviceName, channelId, streamId,
                                              VIDEO_TYPE_PLAYBACKSTREAM, VIDEO_STATUS_ERROR, tVideoError);
                }
                break;
            }
        }
        break;

        case STEP_PLABACK_STREAM_COMMAND:
        {
            if (streamInfoArray[MAIN_DISPLAY][windowIndex].m_streamId != streamId)
            {
                break;
            }

            streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamState = PB_STEP_STATE;
            streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbSpeed = PB_SPEED_NORMAL;
            switch (deviceReply)
            {
                case CMD_STREAM_CONNECTING:
                {
                    changeStreamStateOfWindow(MAIN_DISPLAY, windowIndex, deviceName, channelId, streamId,
                                              VIDEO_TYPE_PLAYBACKSTREAM, VIDEO_STATUS_CONNECTING, VIDEO_ERROR_NONE);
                }
                break;

                case CMD_SUCCESS:
                {
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_iFrameNeeded = false;
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbCurrCmd = streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbNextCmd;
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbNextCmd = PB_MAX_PLAYBACK_STATE;
                    changeStreamStateOfWindow(MAIN_DISPLAY, windowIndex, deviceName, channelId, streamId,
                                              VIDEO_TYPE_PLAYBACKSTREAM, VIDEO_STATUS_RUNNING, VIDEO_ERROR_NONE);

                    if ((m_pbToolbar != NULL) && (m_pbToolbar->getPbToolbarWinId() == windowIndex))
                    {
                        m_pbToolbar->changeButtonEnableState(PB_TOOLBAR_ELE_MUTE_BTN, false);
                        if (m_stepIsRunning == false)
                        {
                            if (streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbDirection == FORWARD_PLAY)
                            {
                                m_pbToolbar->changeStateOfButton(PB_TOOLBAR_ELE_PLAY_BTN, false);
                                m_pbToolbar->changeStateOfButton(PB_TOOLBAR_ELE_REV_PLAY_BTN, true, false);
                            }
                            else
                            {
                                m_pbToolbar->changeStateOfButton(PB_TOOLBAR_ELE_PLAY_BTN, true);
                                m_pbToolbar->changeStateOfButton(PB_TOOLBAR_ELE_REV_PLAY_BTN, false, false);
                            }
                        }
                    }

                    sendPbRequest(windowIndex);
                }
                break;

                case CMD_PLAYBACK_TIME:
                {
                    playbackTime = streamRequestParam->payload.toULongLong();
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbCurrentTime = playbackTime;
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbCurrTimeStr = mSecToString(playbackTime);

                    if ((m_pbToolbar != NULL) && (m_pbToolbar->getPbToolbarWinId() == windowIndex))
                    {
                        m_pbToolbar->showPbSpeedDateTime(windowIndex);
                        m_pbToolbar->updateCurrTime(playbackTime);
                    }
                }
                break;

                case CMD_STREAM_NO_VIDEO_LOSS:
                {
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamState = PB_STEP_STATE;
                    changeStreamStateOfWindow(MAIN_DISPLAY, windowIndex, deviceName, channelId, streamId,
                                              VIDEO_TYPE_PLAYBACKSTREAM, VIDEO_STATUS_RUNNING, VIDEO_ERROR_NONE);
                }
                break;

                case CMD_STREAM_VIDEO_LOSS:
                {
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamState = PB_STEP_STATE;
                    changeStreamStateOfWindow(MAIN_DISPLAY, windowIndex, deviceName, channelId, streamId,
                                              VIDEO_TYPE_PLAYBACKSTREAM, VIDEO_STATUS_VIDEOLOSS, VIDEO_ERROR_VIDEOLOSS);
                }
                break;

                case CMD_STREAM_PLAYBACK_OVER:
                {
                    if ((m_pbToolbar != NULL) && (m_pbToolbar->getPbToolbarWinId() == windowIndex))
                    {
                        m_pbToolbar->updateCurrTime(streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStartTime);
                    }

                    if (streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbOver == false)
                    {
                        streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbCurrentTime = streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStartTime;
                        streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamState = PB_STEP_STATE;
                        streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbCurrCmd = PB_STEP_STATE;
                        streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbNextCmd = PB_MAX_PLAYBACK_STATE;
                        streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbDirection = FORWARD_PLAY;
                        streamInfoArray[MAIN_DISPLAY][windowIndex].m_iFrameNeeded = true;
                        streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbOver = true;
                        sendPbRequest(windowIndex);
                    }
                }
                break;

                case CMD_STREAM_STOPPED:
                {
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamState = PB_PAUSE_STATE;
                    if ((m_pbToolbar != NULL) && (m_pbToolbar->getPbToolbarWinId() == windowIndex))
                    {
                        if (m_stepIsRunning == false)
                        {
                            if (streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbDirection == FORWARD_PLAY)
                            {
                                m_pbToolbar->changeStateOfButton(PB_TOOLBAR_ELE_PLAY_BTN, true);
                            }
                            else
                            {
                                m_pbToolbar->changeStateOfButton(PB_TOOLBAR_ELE_REV_PLAY_BTN, true);
                            }
                        }
                        else
                        {
                            if (!m_pbToolbar->getStateOfButton(PB_TOOLBAR_ELE_PLAY_BTN))
                            {
                                m_pbToolbar->changeStateOfButton(PB_TOOLBAR_ELE_PLAY_BTN, true);
                            }

                            if (!m_pbToolbar->getStateOfButton(PB_TOOLBAR_ELE_REV_PLAY_BTN))
                            {
                                m_pbToolbar->changeStateOfButton (PB_TOOLBAR_ELE_REV_PLAY_BTN, true);
                            }
                        }
                    }

                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbCurrCmd = streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbNextCmd;
                    if (streamInfoArray[displayType][windowIndex].prevPlayState == PB_MAX_PLAYBACK_STATE)
                    {
                        streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbNextCmd = PB_MAX_PLAYBACK_STATE;
                    }
                    else
                    {
                        streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbNextCmd = PB_PLAY_STATE;
                    }
                    sendPbRequest(windowIndex);
                }
                break;

                case CMD_INTERNAL_RESOURCE_LIMIT:
                {
                    /* Nothing to do */
                }
                break;

                default:
                {
                    changeStreamStateOfWindow(MAIN_DISPLAY, windowIndex, deviceName, channelId, streamId,
                                              VIDEO_TYPE_PLAYBACKSTREAM, VIDEO_STATUS_ERROR, VIDEO_ERROR_OTHERERROR);
                }
                break;
            }
        }
        break;

        case AUDIO_PLABACK_STREAM_COMMAND:
        {
            if (streamInfoArray[MAIN_DISPLAY][windowIndex].m_audioStatus == true)
            {
                if (deviceReply == CMD_SUCCESS)
                {
                    audioResponse(false, streamId);
                    if ((m_pbToolbar != NULL) && (m_pbToolbar->getPbToolbarWinId() == windowIndex))
                    {
                        m_pbToolbar->changeStateOfButton(PB_TOOLBAR_ELE_MUTE_BTN, true);
                    }
                }
                else
                {
                    MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(deviceReply));
                }
            }
            else
            {
                if (deviceReply == CMD_SUCCESS)
                {
                    audioResponse(true, streamId);
                    if ((m_pbToolbar != NULL) && (m_pbToolbar->getPbToolbarWinId() == windowIndex))
                    {
                        m_pbToolbar->changeStateOfButton(PB_TOOLBAR_ELE_MUTE_BTN, false);
                    }
                }
                else
                {
                    startAudioInWindow = MAX_CHANNEL_FOR_SEQ;
                    MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(deviceReply));
                }
            }
        }
        break;

        case STOP_PLABACK_STREAM_COMMAND:
        {
            // if Audio start and page change then Audio restart when comes on previous page
            if (streamInfoArray[MAIN_DISPLAY][windowIndex].m_audioStatus == true)
            {
                lastEnableAudioWindow = windowIndex;
                if (m_isClientAudProcessRunning == false)
                {
                    includeExcludeAudio(lastEnableAudioWindow,true);
                }

                // If directly closed window and Audio start on that window then remove Audio
                if (m_audioStopForWindow == windowIndex)
                {
                    lastEnableAudioWindow = MAX_CHANNEL_FOR_SEQ;
                }
            }

            m_audioStopForWindow = MAX_CHANNEL_FOR_SEQ;

            // On Log out,stop the Audio if already ON state
            applController->getUsernameFrmDev(LOCAL_DEVICE_NAME, m_getUserNameforAudio);
            if (m_getUserNameforAudio == DEFAULT_LOGIN_USER)
            {
                lastEnableAudioWindow = MAX_CHANNEL_FOR_SEQ;
            }

            if ((streamInfoArray[MAIN_DISPLAY][windowIndex].m_streamId == streamId))
            {
                if (streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbNextCmd == PB_STOP_STATE)
                {
                    closeDeletePbToolBar();
                }
                else
                {
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamState = PB_PAUSE_STATE;
                }

                if ((m_pbToolbar != NULL) && (m_pbToolbar->getPbToolbarWinId() == windowIndex))
                {
                    if (!m_pbToolbar->getStateOfButton(PB_TOOLBAR_ELE_PLAY_BTN))
                    {
                        m_pbToolbar->changeStateOfButton(PB_TOOLBAR_ELE_PLAY_BTN, true);
                    }

                    if (!m_pbToolbar->getStateOfButton(PB_TOOLBAR_ELE_REV_PLAY_BTN))
                    {
                        m_pbToolbar->changeStateOfButton (PB_TOOLBAR_ELE_REV_PLAY_BTN, true);
                    }
                }

                streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbCurrCmd = streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbNextCmd;
                streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbNextCmd = PB_MAX_PLAYBACK_STATE;
                sendPbRequest(windowIndex);
            }
        }
        break;

        case CLEAR_PLAYBACK_ID_COMMAND:
        {
            if ((streamInfoArray[MAIN_DISPLAY][windowIndex].m_streamId == streamId))
            {
                changeStreamStateOfWindow(MAIN_DISPLAY, windowIndex, "", INVALID_CAMERA_INDEX, MAX_STREAM_SESSION,
                                          VIDEO_TYPE_NONE, VIDEO_STATUS_NONE, VIDEO_ERROR_NONE);
                closeDeletePbToolBar();

                if (streamInfoArray[displayType][windowIndex].m_stopRequestPending == true)
                {
                    streamInfoArray[displayType][windowIndex].m_stopRequestPending = false;
                    if (m_pendingRequestCount[displayType] > 0)
                    {
                        m_pendingRequestCount[displayType]--;
                        if (m_pendingRequestCount[displayType] == 0)
                        {
                            processNextActionForOtherMode(displayType);
                        }
                    }
                }
            }

            if (pendingRequestForNxtPbRec[windowIndex] == AP_TOOLBAR_PREVIOUS)
            {
                //get previous play
                emit(sigFindPrevRecord(m_currentRecNo[windowIndex], windowIndex));
            }
            else if (pendingRequestForNxtPbRec[windowIndex] == AP_TOOLBAR_NEXT)
            {
                //get next play
                emit(sigFindNextRcd(m_currentRecNo[windowIndex], windowIndex));
            }
        }
        break;

        case START_INSTANTPLAYBACK_COMMAND:
        {
            switch (deviceReply)
            {
                case CMD_SUCCESS:
                {
                    payloadLib->parseDevCmdReply(true, streamRequestParam->payload);
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamId = payloadLib->getCnfgArrayAtIndex(0).toUInt();
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStartTime = stringToMSec(payloadLib->getCnfgArrayAtIndex(1).toString());
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbEndTime = stringToMSec(payloadLib->getCnfgArrayAtIndex(2).toString());
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamState = PB_PLAY_STATE;
                    changeStreamStateOfWindow(MAIN_DISPLAY, windowIndex, deviceName, channelId, streamId,
                                              VIDEO_TYPE_INSTANTPLAYBACKSTREAM, VIDEO_STATUS_RUNNING, VIDEO_ERROR_NONE);

                    // For Automatic Audio start if in Live view start
                    if ((lastEnableAudioWindow == windowIndex) && (m_instantPlayback == true))
                    {
                        m_instantPlayback = false;
                        if (m_isClientAudProcessRunning == false)
                        {
                            includeExcludeAudio(lastEnableAudioWindow, true);
                        }
                    }
                }
                break;

                case CMD_STREAM_NO_VIDEO_LOSS:
                {
                    changeStreamStateOfWindow(MAIN_DISPLAY, windowIndex, deviceName, channelId, streamId,
                                              VIDEO_TYPE_INSTANTPLAYBACKSTREAM, VIDEO_STATUS_RUNNING, VIDEO_ERROR_NONE);

                    // For Automatic Audio start if in Live view start
                    if ((lastEnableAudioWindow == windowIndex) && (m_instantPlayback == true))
                    {
                        m_instantPlayback = false;
                        if (m_isClientAudProcessRunning == false)
                        {
                            includeExcludeAudio(lastEnableAudioWindow,true);
                        }
                    }
                }
                break;

                case CMD_STREAM_VIDEO_LOSS:
                {
                    changeStreamStateOfWindow(MAIN_DISPLAY, windowIndex, deviceName, channelId, streamId,
                                              VIDEO_TYPE_INSTANTPLAYBACKSTREAM, VIDEO_STATUS_VIDEOLOSS, VIDEO_ERROR_VIDEOLOSS);
                }
                break;

                case CMD_NO_PRIVILEGE:
                {
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamState = PB_MAX_PLAYBACK_STATE;
                    MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(deviceReply));
                    isInstancePlayBackStopRequire = true;
                }
                break;

                case CMD_INTERNAL_RESOURCE_LIMIT:
                {
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamState = PB_MAX_PLAYBACK_STATE;
                    MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(LAYOUT_STREAM_LIMIT_ERROR_MESSAGE));
                    isInstancePlayBackStopRequire = true;
                }
                break;

                case CMD_PROCESS_ERROR:
                {
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamState = PB_MAX_PLAYBACK_STATE;
                    MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(deviceReply));
                    isInstancePlayBackStopRequire = true;
                }
                break;

                case CMD_PLAYBACK_TIME:
                {
                    //update time in timeline
                    playbackTime = streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbCurrentTime;
                    if (m_pbSliderChange == false)
                    {
                        playbackTime = streamRequestParam->payload.toULongLong();
                        streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbCurrentTime = playbackTime;
                    }
                    else
                    {
                        m_pbSliderChange = false;
                    }

                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbCurrTimeStr = mSecToString(playbackTime);
                    if ((m_instantpbToolbar != NULL) && (m_instantpbToolbar->getPbToolbarWinId() == windowIndex))
                    {
                        m_instantpbToolbar->showInstantPbDateTime();
                        m_instantpbToolbar->updateCurrTime(playbackTime);
                    }
                }
                break;

                case CMD_STREAM_PLAYBACK_OVER:
                {
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamState = PB_MAX_PLAYBACK_STATE;
                    isInstancePlayBackStopRequire = true;
                }
                break;

                case CMD_INSTANT_PLAYBACK_FAILED:
                {
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamState = PB_MAX_PLAYBACK_STATE;
                    MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(deviceReply));
                    isInstancePlayBackStopRequire = true;
                }
                break;

                case CMD_DECODER_CAPACITY_ERROR:
                {
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamState = PB_MAX_PLAYBACK_STATE;
                    changeStreamStateOfWindow(displayType, windowIndex, deviceName, channelId, streamId,
                                              VIDEO_TYPE_INSTANTPLAYBACKSTREAM, VIDEO_STATUS_ERROR, VIDEO_ERROR_NO_DECODING_CAP);
                }
                break;

                case CMD_DECODER_ERROR:
                {
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamState = PB_MAX_PLAYBACK_STATE;
                    isInstancePlayBackStopRequire = true;
                }
                break;

                default:
                {
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamState = PB_MAX_PLAYBACK_STATE;
                    MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(deviceReply));
                    isInstancePlayBackStopRequire = true;
                }
                break;
            }

            if (streamInfoArray[displayType][windowIndex].m_startRequestPending == true)
            {
                streamInfoArray[displayType][windowIndex].m_startRequestPending = false;
                streamInfoArray[displayType][windowIndex].m_startInstantPlayback = false;

                if(pendingInstantRequestCount[displayType][windowIndex] > 0)
                {
                    m_processPendingForWindow[MAIN_DISPLAY] = false;
                    pendingInstantRequestCount[displayType][windowIndex]--;
                    if (pendingInstantRequestCount[displayType][windowIndex] == 0)
                    {
                        processNextActionForWindow(displayType, windowIndex, false, true);
                    }
                }
            }

            if (isInstancePlayBackStopRequire)
            {
                m_instantPlaybackRequestSent[windowIndex] = true;
                stopInstantPlaybackFeature(windowIndex);
            }
        }
        break;

        case PAUSE_INSTANTPLAYBACK_COMMAND:
        {
            m_instantPlaybackRequestSent[windowIndex] = false;
            if (streamInfoArray[MAIN_DISPLAY][windowIndex].m_streamId != streamId)
            {
                break;
            }

            switch (deviceReply)
            {
                case CMD_SUCCESS:
                {
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamState = PB_PAUSE_STATE;
                    if ((m_instantpbToolbar != NULL) && (m_instantpbToolbar->getPbToolbarWinId() == windowIndex))
                    {
                        if (streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbDirection == FORWARD_PLAY)
                        {
                            m_instantpbToolbar->changeStateOfButton(INST_PB_TOOLBAR_ELE_PLAY_BTN, true);
                            m_instantpbToolbar->changeStateOfButton(INST_PB_TOOLBAR_ELE_REV_PLAY_BTN, true, false);
                        }
                        else
                        {
                            m_instantpbToolbar->changeStateOfButton(INST_PB_TOOLBAR_ELE_PLAY_BTN, true);
                            m_instantpbToolbar->changeStateOfButton(INST_PB_TOOLBAR_ELE_REV_PLAY_BTN, true, false);
                        }
                    }
                }
                break;

                case CMD_PROCESS_ERROR:
                case CMD_RESOURCE_LIMIT:
                {
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamState = PB_MAX_PLAYBACK_STATE;
                    changeStreamStateOfWindow(MAIN_DISPLAY, windowIndex, deviceName, channelId, MAX_STREAM_SESSION,
                                              VIDEO_TYPE_INSTANTPLAYBACKSTREAM, VIDEO_STATUS_ERROR, VIDEO_ERROR_OTHERERROR);
                }
                break;

                default:
                {
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamState = PB_MAX_PLAYBACK_STATE;
                    changeStreamStateOfWindow(MAIN_DISPLAY, windowIndex, deviceName, channelId, streamId,
                                              VIDEO_TYPE_INSTANTPLAYBACKSTREAM, VIDEO_STATUS_ERROR, VIDEO_ERROR_OTHERERROR);
                }
                break;
            }
        }
        break;

        case STOP_INSTANTPLAYBACK_COMMAND:
        {
            m_instantPlaybackRequestSent[windowIndex] = false;
            if (startAudioInWindow == windowIndex)
            {
                streamInfoArray[MAIN_DISPLAY][windowIndex].m_audioStatus = false;
                startAudioInWindow = MAX_CHANNEL_FOR_SEQ;
            }

            if (streamInfoArray[MAIN_DISPLAY][windowIndex].m_streamId != streamId)
            {
                break;
            }

            changeStreamStateOfWindow(displayType, windowIndex, "", INVALID_CAMERA_INDEX, MAX_STREAM_SESSION,
                                      VIDEO_TYPE_NONE, VIDEO_STATUS_NONE, VIDEO_ERROR_NONE);

            if (streamInfoArray[displayType][windowIndex].m_stopRequestPending == true)
            {
                streamInfoArray[displayType][windowIndex].m_stopRequestPending = false;
                if (pendingInstantRequestCount[displayType][windowIndex] > 0)
                {
                    m_processPendingForWindow[MAIN_DISPLAY] = false;
                    pendingInstantRequestCount[displayType][windowIndex]--;
                    if (pendingInstantRequestCount[displayType][windowIndex] == 0)
                    {
                        processNextActionForWindow(displayType, windowIndex, false, true);
                    }
                }
            }
        }
        break;

        case SEEK_INSTANTPLAYBACK_COMMAND:
        {
            m_instantPlaybackRequestSent[windowIndex] = false;
            if ((streamInfoArray[MAIN_DISPLAY][windowIndex].m_streamId != streamId)
                    || (streamInfoArray[MAIN_DISPLAY][windowIndex].m_videoType != VIDEO_TYPE_INSTANTPLAYBACKSTREAM))
            {
                break;
            }

            switch (deviceReply)
            {
                case CMD_SUCCESS:
                {
                    if (m_pbSliderChange == true)
                    {
                        m_pbSliderChange = false;
                    }

                    if (m_instantAudioWindow == windowIndex)
                    {
                        m_instantAudioWindow = MAX_CHANNEL_FOR_SEQ;

                        if (lastEnableAudioWindow == windowIndex)
                        {
                            includeExcludeAudio(windowIndex,true);
                        }
                        else
                        {
                            includeExcludeAudio(windowIndex);
                        }
                    }

                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamState = PB_PLAY_STATE;
                    if ((m_instantpbToolbar != NULL) && (m_instantpbToolbar->getPbToolbarWinId() == windowIndex))
                    {
                        if (streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbDirection == FORWARD_PLAY)
                        {
                            m_instantpbToolbar->changeStateOfButton(INST_PB_TOOLBAR_ELE_PLAY_BTN, false);
                            m_instantpbToolbar->changeStateOfButton(INST_PB_TOOLBAR_ELE_REV_PLAY_BTN, true, false);
                        }
                        else
                        {
                            m_instantpbToolbar->changeStateOfButton(INST_PB_TOOLBAR_ELE_PLAY_BTN, true);
                            m_instantpbToolbar->changeStateOfButton(INST_PB_TOOLBAR_ELE_REV_PLAY_BTN, false, false);
                        }
                    }
                }
                break;

                case CMD_PROCESS_ERROR:
                case CMD_RESOURCE_LIMIT:
                {
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamState = PB_MAX_PLAYBACK_STATE;
                    changeStreamStateOfWindow(MAIN_DISPLAY, windowIndex, deviceName, channelId, MAX_STREAM_SESSION,
                                              VIDEO_TYPE_INSTANTPLAYBACKSTREAM, VIDEO_STATUS_ERROR, VIDEO_ERROR_OTHERERROR);
                }
                break;

                default:
                {
                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamState = PB_MAX_PLAYBACK_STATE;
                    changeStreamStateOfWindow(MAIN_DISPLAY, windowIndex, deviceName, channelId, streamId,
                                              VIDEO_TYPE_INSTANTPLAYBACKSTREAM, VIDEO_STATUS_ERROR, VIDEO_ERROR_OTHERERROR);
                }
                break;
            }
        }
        break;

        case AUDIO_INSTANTPLAYBACK_COMMAND:
        {
            if (streamInfoArray[MAIN_DISPLAY][windowIndex].m_audioStatus == true)
            {
                if (deviceReply == CMD_SUCCESS)
                {
                    audioResponse(false, streamId);
                    if ((m_instantpbToolbar != NULL) && (m_instantpbToolbar->getPbToolbarWinId() == windowIndex))
                    {
                        m_instantpbToolbar->changeStateOfButton(INST_PB_TOOLBAR_ELE_MUTE_BTN, true);
                    }
                }
                else
                {
                    MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(deviceReply));
                }
            }
            else
            {
                if (deviceReply == CMD_SUCCESS)
                {
                    audioResponse(true, streamId);
                    if ((m_instantpbToolbar != NULL) && (m_instantpbToolbar->getPbToolbarWinId() == windowIndex))
                    {
                        m_instantpbToolbar->changeStateOfButton(INST_PB_TOOLBAR_ELE_MUTE_BTN, false);
                    }
                }
                else
                {
                    startAudioInWindow = MAX_CHANNEL_FOR_SEQ;
                    MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(deviceReply));
                }
            }
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void Layout :: processStreamObjectDelete(DISPLAY_TYPE_e *displayTypeForDelete, quint16 *actualWindowIdForDelete)
{
    for(quint8 displayType = 0; displayType < MAX_DISPLAY_TYPE; displayType++)
    {
        DISPLAY_TYPE_e tempDisp = displayTypeForDelete[displayType];
        if ((tempDisp < MAIN_DISPLAY) || (tempDisp >= MAX_DISPLAY_TYPE))
        {
            continue;
        }

        quint16 windIndx = actualWindowIdForDelete[displayType];
        if (windIndx >= MAX_CHANNEL_FOR_SEQ)
        {
            continue;
        }

        if((streamInfoArray[tempDisp][windIndx].m_startRequestPending == true) && (streamInfoArray[tempDisp][windIndx].m_startInstantPlayback == false))
        {
            streamInfoArray[tempDisp][windIndx].m_startRequestPending = false;
            changeStreamStateOfWindow(tempDisp,
                                      windIndx,
                                      streamInfoArray[tempDisp][windIndx].m_deviceName,
                                      streamInfoArray[tempDisp][windIndx].m_cameraId,
                                      MAX_STREAM_SESSION,
                                      VIDEO_TYPE_LIVESTREAM,
                                      VIDEO_STATUS_RETRY,
                                      VIDEO_ERROR_OTHERERROR);

            if (pendingInstantRequestCount[tempDisp][windIndx] != 0)
            {
                m_processPendingForWindow[MAIN_DISPLAY] = false;
                pendingInstantRequestCount[tempDisp][windIndx]--;
                if(pendingInstantRequestCount[tempDisp][windIndx] == 0)
                {
                    processNextActionForWindow(tempDisp, windIndx, false, true);
                }
            }
        }

        if((streamInfoArray[tempDisp][windIndx].m_stopRequestPending == true) && (streamInfoArray[tempDisp][windIndx].m_stopInstantPlayback == false))
        {
            streamInfoArray[tempDisp][windIndx].m_stopRequestPending = false;
            if(streamInfoArray[tempDisp][windIndx].m_replacingChannel == false)
            {
                changeStreamStateOfWindow(tempDisp,
                                          windIndx,
                                          "",
                                          INVALID_CAMERA_INDEX,
                                          MAX_STREAM_SESSION,
                                          VIDEO_TYPE_NONE,
                                          VIDEO_STATUS_NONE,
                                          VIDEO_ERROR_NONE);
            }
            else
            {
                streamInfoArray[tempDisp][windIndx].m_replacingChannel = false;
                streamInfoArray[tempDisp][windIndx].m_audioStatus = false;

                quint16 window[2] = {0,MAX_CHANNEL_FOR_SEQ};
                getFirstAndLastWindow(currentDisplayConfig[tempDisp].currPage, currentDisplayConfig[tempDisp].layoutId, window);

                if((windIndx >= window[0]) && (windIndx <= window[1]))
                {
                    layoutCreator->updateWindowData(windIndx);
                }
            }

            if(pendingInstantRequestCount[tempDisp][windIndx] != 0)
            {
                m_processPendingForWindow[MAIN_DISPLAY] = false;
                pendingInstantRequestCount[tempDisp][windIndx]--;
                if(pendingInstantRequestCount[tempDisp][windIndx] == 0)
                {
                    processNextActionForWindow((DISPLAY_TYPE_e)tempDisp, windIndx, false, true);
                }
            }
        }
    }
}

void Layout::disconnectDeviceNotify(QString devName)
{
    applController->deleteStreamRequestForDevice(devName);

    if((analogFeatureDataType.m_deviceNameForFeature == devName) && (analogFeatureDataType.m_cameraIndexForFeature != INVALID_CAMERA_INDEX))
    {
        hideAnalogCameraFeature();
        closeAnalogCameraFeature();
    }

    if((m_ptzControl != NULL) && (devName == (m_ptzControl->currentDevName())))
    {
        disconnect (m_ptzControl,
                    SIGNAL(sigObjectDelete()),
                    this,
                    SLOT(slotSubObjectDelete()));
        DELETE_OBJ(m_ptzControl);
        DELETE_OBJ(m_inVisibleWidget);
    }
}

void Layout::showAnalogCameraFeature(quint8 cameraIndex, CAMERA_FEATURE_TYPE_e featureType, void* param, QString deviceName, void* configParam)
{
    //stop default sequencing
    pauseSequencing(MAIN_DISPLAY);

    //stop Window sequencing
    pauseWindowSequencing(MAIN_DISPLAY);

    m_previousSelectedWindow = MAX_CHANNEL_FOR_SEQ;

    analogFeatureDataType.m_featureData = param;
    analogFeatureDataType.m_featureConfigData = configParam;
    analogFeatureDataType.m_cameraIndexForFeature = cameraIndex;
    analogFeatureDataType.m_deviceNameForFeature = deviceName;
    analogFeatureDataType.m_featureType = featureType;
    analogFeatureDataType.m_previousSelectedWindow = currentDisplayConfig[MAIN_DISPLAY].selectedWindow;

    if((currentDisplayConfig[MAIN_DISPLAY].layoutId == ONE_X_ONE) && (analogFeatureDataType.m_featureType == ZOOM_FEATURE)
            && (analogFeatureDataType.m_previousBeforeOneXOne == MAX_LAYOUT) && (m_doubleClickModeFlag == true))
    {
        analogFeatureDataType.m_previousBeforeOneXOne = m_previousLayout[MAIN_DISPLAY];
    }
    else
    {
        analogFeatureDataType.m_previousBeforeOneXOne = MAX_LAYOUT;
    }
    m_doubleClickModeFlag = false;
    analogFeatureDataType.m_previousSelectedLayout = currentDisplayConfig[MAIN_DISPLAY].layoutId;

    quint16 windowIndex = findWindowIndexIfAssignOnCurrentPage(MAIN_DISPLAY, deviceName, cameraIndex);
    if(windowIndex == MAX_CHANNEL_FOR_SEQ)
    {
        windowIndex = currentDisplayConfig[MAIN_DISPLAY].currPage * windowPerPage[currentDisplayConfig[MAIN_DISPLAY].layoutId];
    }

    memcpy(&analogFeatureDataType.m_backupWindowInfo, &currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex], sizeof(WINDOW_INFO_t));
    memset(&currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex], 0, sizeof(WINDOW_INFO_t));

    for(quint8 channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
    {
        currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[channelIndex].deviceName[0] = '\0';
        currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[channelIndex].defChannel = INVALID_CAMERA_INDEX;
    }

    //change style and layout
    analogFeatureDataType.m_windowIndexForFeature = windowIndex;
    snprintf(currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[0].deviceName, MAX_DEVICE_NAME_SIZE, "%s", deviceName.toUtf8().constData());
    currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[0].defChannel = cameraIndex;

    shiftWindows(MAIN_DISPLAY, windowIndex, ONE_X_ONE);
    analogFeatureDataType.m_currentFeatureState = FEATURE_STATE_STOP_OLD_WINDOW;
    changeCurrentMode(STATE_ANALOG_FEATURE, MAIN_DISPLAY);
}

void Layout::startAnalogCameraFeature()
{
    CAMERA_TYPE_e cameraType;
    switch(analogFeatureDataType.m_featureType)
    {
        case IMAGE_APPEARENCE_FEATURE:
        {
            if(m_imageAppearenceSettings == NULL)
            {
                /* PARASOFT: Memory Deallocated in hide and close analogCameraFeature */
                m_imageAppearenceSettings = new ImageAppearenceSettings(analogFeatureDataType.m_featureData,
                                                                        analogFeatureDataType.m_featureConfigData,
                                                                        analogFeatureDataType.m_cameraIndexForFeature,
                                                                        parentWidget());
                connect(m_imageAppearenceSettings,
                        SIGNAL(destroyed(QObject*)),
                        this,
                        SLOT(slotCloseCameraFeature(QObject*)));
            }
        }
        break;

        case PRIVACY_MASK_FEATURE:
        {
            cameraType = applController->GetCameraType(analogFeatureDataType.m_deviceNameForFeature, (analogFeatureDataType.m_cameraIndexForFeature-1));
            if(m_privackMaskSettings == NULL)
            {
                /* PARASOFT: Memory Deallocated in hide and close analogCameraFeature */
                m_privackMaskSettings = new PrivacyMaskSettings(analogFeatureDataType.m_featureData,
                                                                analogFeatureDataType.m_deviceNameForFeature,
                                                                analogFeatureDataType.m_cameraIndexForFeature,
                                                                cameraType,
                                                                Layout::maxSupportedPrivacyMaskWindow,
                                                                parentWidget());
                connect(m_privackMaskSettings,
                        SIGNAL(destroyed(QObject*)),
                        this,
                        SLOT(slotCloseCameraFeature(QObject*)));
                connect (m_privackMaskSettings,
                         SIGNAL(sigLoadProcessBar()),
                         this,
                         SLOT(slotLoadProcessBar()));
            }
        }
        break;

        case MOTION_DETECTION_FEATURE:
        {
            cameraType = applController->GetCameraType(analogFeatureDataType.m_deviceNameForFeature, (analogFeatureDataType.m_cameraIndexForFeature-1));
            if(cameraType == ANALOG_CAMERA)
            {
                if (m_motionDetectionSettings == NULL)
                {
                    /* PARASOFT: Memory Deallocated in hide and close analogCameraFeature */
                    m_motionDetectionSettings = new MotionDetectionSettings(analogFeatureDataType.m_featureData,
                                                                            analogFeatureDataType.m_deviceNameForFeature,
                                                                            parentWidget());
                    connect(m_motionDetectionSettings,
                            SIGNAL(destroyed(QObject*)),
                            this,
                            SLOT(slotCloseCameraFeature(QObject*)));
                }
            }
            else if((cameraType == IP_CAMERA) || (cameraType == AUTO_ADD_IP_CAMERA))
            {
                if (m_ipMotionDetectionSettings == NULL)
                {
                    /* PARASOFT: Memory Deallocated in hide and close analogCameraFeature */
                    m_ipMotionDetectionSettings = new IpMotionDetectionSettings(analogFeatureDataType.m_featureData,
                                                                                analogFeatureDataType.m_deviceNameForFeature,
                                                                                analogFeatureDataType.m_cameraIndexForFeature,
                                                                                parentWidget());
                    connect(m_ipMotionDetectionSettings,
                            SIGNAL(destroyed(QObject*)),
                            this,
                            SLOT(slotCloseCameraFeature(QObject*)));
                    connect (m_ipMotionDetectionSettings,
                             SIGNAL(sigLoadProcessBar()),
                             this,
                             SLOT(slotLoadProcessBar()));
                }
            }
            else
            {
                EPRINT(LAYOUT, "invld camera type in motion detection settings: [cameraType=%d]", cameraType);
            }
        }
        break;

        case ZOOM_FEATURE:
        {
            /* PARASOFT: Memory Deallocated in hide and close analogCameraFeature */
            m_zoomFeatureControl = new ZoomFeatureControl(analogFeatureDataType.m_deviceNameForFeature,
                                                          analogFeatureDataType.m_windowIndexForFeature,
                                                          parentWidget());
            connect(m_zoomFeatureControl,
                    SIGNAL(sigExitFromZoomFeature()),
                    this,
                    SLOT(slotExitFromZoomFeature()));
            connect(m_zoomFeatureControl,
                    SIGNAL(sigChangeLayout(LAYOUT_TYPE_e,DISPLAY_TYPE_e,quint16,bool,bool)),
                    this,
                    SLOT(slotChangeLayout(LAYOUT_TYPE_e,DISPLAY_TYPE_e,quint16,bool,bool)));
            connect(m_zoomFeatureControl,
                    SIGNAL(destroyed(QObject*)),
                    this,
                    SLOT(slotCloseCameraFeature(QObject*)));
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }

    emit sigRaiseWidgetOnTopOfAll();
}

void Layout::stopAnalogCameraFeature()
{
    selectWindow(MAIN_DISPLAY, analogFeatureDataType.m_previousSelectedWindow);

    if((m_previousLayout[MAIN_DISPLAY] == ONE_X_ONE) && (analogFeatureDataType.m_featureType == ZOOM_FEATURE)
            && (analogFeatureDataType.m_previousBeforeOneXOne != MAX_LAYOUT))
    {
         m_doubleClickModeFlag = true;
         m_previousLayout[MAIN_DISPLAY] = analogFeatureDataType.m_previousBeforeOneXOne;
    }

    resetAnalogCameraFeature();
    if(m_cosecHandleLaterFlag == false)
    {
        emit sigCloseAnalogCameraFeature();
    }
}

void Layout::resetAnalogCameraFeature()
{
    analogFeatureDataType.m_windowIndexForFeature = MAX_CHANNEL_FOR_SEQ;
    analogFeatureDataType.m_cameraIndexForFeature = INVALID_CAMERA_INDEX;
    analogFeatureDataType.m_deviceNameForFeature = "";
    analogFeatureDataType.m_featureType = MAX_CAMERA_FEATURE;
    memset(&analogFeatureDataType.m_backupWindowInfo, 0, sizeof(WINDOW_INFO_t));

    for(quint8 channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
    {
        analogFeatureDataType.m_backupWindowInfo.camInfo[channelIndex].deviceName[0] = '\0';
        analogFeatureDataType.m_backupWindowInfo.camInfo[channelIndex].defChannel = INVALID_CAMERA_INDEX;
    }

    analogFeatureDataType.m_previousSelectedWindow = MAX_CHANNEL_FOR_SEQ;
    analogFeatureDataType.m_previousSelectedLayout = MAX_LAYOUT;
    analogFeatureDataType.m_previousBeforeOneXOne = MAX_LAYOUT;
    analogFeatureDataType.m_currentFeatureState = FEATURE_STATE_NONE;
}

void Layout::hideAnalogCameraFeature()
{
    if (analogFeatureDataType.m_currentFeatureState != FEATURE_STATE_VIDEO_ON)
    {
        return;
    }

    switch(analogFeatureDataType.m_featureType)
    {
        case IMAGE_APPEARENCE_FEATURE:
        {
            DELETE_OBJ(m_imageAppearenceSettings);
        }
        break;

        case PRIVACY_MASK_FEATURE:
        {
            if(m_privackMaskSettings != NULL)
            {
                disconnect (m_privackMaskSettings,
                            SIGNAL(sigLoadProcessBar()),
                            this,
                            SLOT(slotLoadProcessBar()));
                DELETE_OBJ(m_privackMaskSettings);
            }
        }
        break;

        case MOTION_DETECTION_FEATURE:
        {
            DELETE_OBJ(m_motionDetectionSettings);
            if(m_ipMotionDetectionSettings != NULL)
            {
                disconnect (m_ipMotionDetectionSettings,
                            SIGNAL(sigLoadProcessBar()),
                            this,
                            SLOT(slotLoadProcessBar()));
                DELETE_OBJ(m_ipMotionDetectionSettings);
            }
        }
        break;

        case ZOOM_FEATURE:
        {
            m_zoomFeatureControl->exitAction();
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void Layout::closeAnalogCameraFeature(bool applyOldLayoutFlag)
{

    quint16 windowIndex = analogFeatureDataType.m_windowIndexForFeature;
    memcpy(&currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex], &analogFeatureDataType.m_backupWindowInfo, sizeof(WINDOW_INFO_t));

    if(applyOldLayoutFlag)
    {
        analogFeatureDataType.m_currentFeatureState = FEATURE_STATE_STOP_VIDEO;
        shiftWindows(MAIN_DISPLAY, currentDisplayConfig[MAIN_DISPLAY].selectedWindow, analogFeatureDataType.m_previousSelectedLayout);

        // resume sequencing
        resumeSequencing(MAIN_DISPLAY);
        if(currentDisplayConfig[MAIN_DISPLAY].seqStatus == false)
        {
            // resume window Sequencing
            resumeWindowSequencing(MAIN_DISPLAY);
        }

        changeCurrentMode(STATE_ANALOG_FEATURE, MAIN_DISPLAY);
    }
    else
    {
        stopAnalogCameraFeature();
        changeCurrentMode(STATE_OTHER_MODE_NONE, MAIN_DISPLAY, false);
    }
}

void Layout::processQueuedActionForVideoPopup()
{
    if (false == videoPopupData.m_videoPopupStartFlag)
    {
        return;
    }

    videoPopupData.m_videoPopupStartFlag = false;
    for(quint8 evtCnt = 0; evtCnt < videoPopupData.m_pendindingVideoPopupCount; evtCnt++)
    {
        if(videoPopupData.next_cameraEvtVideoPopData[evtCnt].m_cameraIndexForFeature != INVALID_CAMERA_INDEX)
        {
            showVideoPopupFeature(videoPopupData.next_cameraEvtVideoPopData[evtCnt].m_cameraIndexForFeature,
                                  videoPopupData.next_cameraEvtVideoPopData[evtCnt].m_deviceName,
                                  videoPopupData.next_cameraEvtVideoPopData[evtCnt].m_eventType,
                                  videoPopupData.next_cameraEvtVideoPopData[evtCnt].m_videoPopUpTimeDuration);
        }

        videoPopupData.next_cameraEvtVideoPopData[evtCnt].m_deviceName = "";
        videoPopupData.next_cameraEvtVideoPopData[evtCnt].m_windowIndexForFeature = MAX_CHANNEL_FOR_SEQ;
        videoPopupData.next_cameraEvtVideoPopData[evtCnt].m_videoPopUpTimeDuration = 10;
        videoPopupData.next_cameraEvtVideoPopData[evtCnt].m_eventType = 0;
        videoPopupData.next_cameraEvtVideoPopData[evtCnt].m_cameraIndexForFeature = INVALID_CAMERA_INDEX;
        videoPopupData.next_cameraEvtVideoPopData[evtCnt].m_timerCounter = 0;
    }
    videoPopupData.m_pendindingVideoPopupCount = MAX_POPUP_WINDOWS;}

void Layout::showVideoPopupFeature(quint8 cameraIndex, QString deviceName, quint8 eventType, quint8 videoPopUpDuration)
{
    quint16 windowIndex;

    if (true == isAnyWindowReplacing(MAIN_DISPLAY))
    {
        return;
    }

    m_previousSelectedWindow = MAX_CHANNEL_FOR_SEQ;
    if((currentDisplayConfig[MAIN_DISPLAY].layoutId == ONE_X_ONE) && (videoPopupData.m_previousBeforeOneXOne == MAX_LAYOUT)
            && (m_doubleClickModeFlag == true))
    {
        videoPopupData.m_previousBeforeOneXOne = m_previousLayout[MAIN_DISPLAY];
    }
    else
    {
        videoPopupData.m_previousBeforeOneXOne = MAX_LAYOUT;
    }
    m_doubleClickModeFlag = false;

    //if camera already present then update video poup duration and event type
    for(windowIndex = 0; windowIndex < MAX_POPUP_WINDOWS; windowIndex++)
    {
        if((videoPopupData.cameraEvtVideoPopData[windowIndex].m_cameraIndexForFeature == cameraIndex) &&
                (videoPopupData.cameraEvtVideoPopData[windowIndex].m_deviceName == deviceName))
        {
            videoPopupData.cameraEvtVideoPopData[windowIndex].m_timerCounter = 0;
            videoPopupData.cameraEvtVideoPopData[windowIndex].m_videoPopUpTimeDuration = videoPopUpDuration;
            streamInfoArray[MAIN_DISPLAY][videoPopupData.cameraEvtVideoPopData[windowIndex].m_windowIndexForFeature].eventType = eventType;
            videoPopupData.cameraEvtVideoPopData[windowIndex].m_eventType = eventType;
            layoutCreator->updateWindowData(videoPopupData.cameraEvtVideoPopData[windowIndex].m_windowIndexForFeature);
            return;
        }
    }

    //Find window index
    for(windowIndex = 0; windowIndex < MAX_POPUP_WINDOWS; windowIndex++)
    {
        if((videoPopupData.cameraEvtVideoPopData[windowIndex].m_cameraIndexForFeature == INVALID_CAMERA_INDEX))
        {
            break;
        }
    }

    //if max window assign then check which camera is about to die and replace the window
    bool replaceFlag = false;
    if(windowIndex == MAX_POPUP_WINDOWS)
    {
        quint32 maxTimerCount = 0;
        quint32 timerCount;

        for(quint8 popupWindow = 0; popupWindow < MAX_POPUP_WINDOWS; popupWindow++)
        {
            timerCount = videoPopupData.cameraEvtVideoPopData[popupWindow].m_timerCounter;
            if(maxTimerCount < timerCount)
            {
                maxTimerCount = timerCount;
                windowIndex = popupWindow;
                replaceFlag = true;
            }
        }
    }

    if(windowIndex == MAX_POPUP_WINDOWS)
    {
        windowIndex = 0;
        replaceFlag = true;
    }

    if(replaceFlag == true)
    {
        videoPopupData.cameraEvtVideoPopData[windowIndex].m_replaceWindowVideoPopup = true;
        videoPopupData.cameraEvtVideoPopData[windowIndex].m_timerCounter = 0;
    }

    if((videoPopupData.m_previousSelectedWindow == MAX_CHANNEL_FOR_SEQ) && (videoPopupData.m_previousSelectedLayout == MAX_LAYOUT))
    {
        videoPopupData.m_currentFeatureState = VIDEO_POPUP_STATE_NONE;
    }

    switch(videoPopupData.m_currentFeatureState)
    {
        case VIDEO_POPUP_STATE_NONE:
        {
            if((currentModeType[MAIN_DISPLAY] == STATE_OTHER_MODE_NONE) || (currentModeType[MAIN_DISPLAY] == STATE_ANALOG_FEATURE) ||
                    (currentModeType[MAIN_DISPLAY] == STATE_COSEC_FEATURE) || (currentModeType[MAIN_DISPLAY] == STATE_EXPAND_WINDOW))
            {
                videoPopupData.cameraEvtVideoPopData[windowIndex].m_cameraIndexForFeature = cameraIndex;
                videoPopupData.cameraEvtVideoPopData[windowIndex].m_deviceName = deviceName;
                videoPopupData.cameraEvtVideoPopData[windowIndex].m_eventType = eventType;
                videoPopupData.cameraEvtVideoPopData[windowIndex].m_videoPopUpTimeDuration = videoPopUpDuration;
            }

            switch(currentModeType[MAIN_DISPLAY])
            {
                case STATE_OTHER_MODE_NONE:
                {
                    pauseSequencing(MAIN_DISPLAY);
                    pauseWindowSequencing(MAIN_DISPLAY);
                    m_videoPopupMode = true;
                    if ((startAudioInWindow != MAX_CHANNEL_FOR_SEQ) && (videoPopupData.m_startAudioInWindow == MAX_CHANNEL_FOR_SEQ))
                    {
                        excludeDevAudio(startAudioInWindow);
                        if(streamInfoArray[MAIN_DISPLAY][startAudioInWindow].m_videoType == VIDEO_TYPE_LIVESTREAM)
                        {
                            videoPopupData.m_startAudioInWindow = startAudioInWindow;
                            lastEnableAudioWindow = startAudioInWindow;
                        }
                        else
                        {
                            videoPopupData.m_startAudioInWindow = MAX_CHANNEL_FOR_SEQ;
                        }
                        startAudioInWindow = MAX_CHANNEL_FOR_SEQ;
                    }

                    // Microphone On then stop Audio-In on Video Pop-Up event
                    if(MAX_CHANNEL_FOR_SEQ != m_activeMicroPhoneWindow)
                    {
                        sendIncludeExcludeDevAudioInCmd(m_activeMicroPhoneWindow, false);
                    }

                    //close async playback
                    for(quint16 index = 0; index < MAX_CHANNEL_FOR_SEQ; index++)
                    {
                        playbackRecordData[index].clearPlaybackInfo();
                    }

                    //Close live view tool bar
                    if((m_liveViewToolbar != NULL))
                    {
                        slotCloseLiveViewToolbar();
                    }

                    unloadAndHideOptionsInVideoPopup();
                    updateVideoPopupParam(true,windowIndex);
                }
                break;

                case STATE_ANALOG_FEATURE:
                {
                    videoPopupData.m_previousSelectedLayout = analogFeatureDataType.m_previousSelectedLayout;
                    if((analogFeatureDataType.m_previousBeforeOneXOne != MAX_LAYOUT) && (videoPopupData.m_previousBeforeOneXOne == MAX_LAYOUT))
                    {
                       videoPopupData.m_previousBeforeOneXOne = analogFeatureDataType.m_previousBeforeOneXOne;
                    }

                    videoPopupData.m_previousSelectedWindow = analogFeatureDataType.m_previousSelectedWindow;
                    videoPopupData.m_previousMainState = STATE_ANALOG_FEATURE;
                    hideAnalogCameraFeature();
                    updateVideoPopupParam(false, windowIndex);
                    resetAnalogCameraFeature();
                }
                break;

                case STATE_COSEC_FEATURE:
                {
                    m_cosecHandleLaterFlag = false;
                    videoPopupData.m_previousSelectedLayout = cosecPopupEvtData.m_previousSelectedLayout;
                    videoPopupData.m_previousSelectedWindow = cosecPopupEvtData.m_previousSelectedWindow;
                    deleteCosecPopUpBanner();
                    deleteCosecPopupObject();
                    updateVideoPopupParam(false,windowIndex);
                    resetCosecPopupFeature();
                }
                break;

                case STATE_EXPAND_WINDOW:
                {
                    videoPopupData.m_wasExpandMode = true;
                    updateVideoPopupParam(true,windowIndex);
                }
                break;

                default:
                {
                    m_cosecHandleLaterFlag = false;
                    if(videoPopupData.m_pendindingVideoPopupCount == MAX_POPUP_WINDOWS)
                    {
                        videoPopupData.m_pendindingVideoPopupCount = 0;
                    }

                    videoPopupData.next_cameraEvtVideoPopData[videoPopupData.m_pendindingVideoPopupCount].m_cameraIndexForFeature = cameraIndex;
                    videoPopupData.next_cameraEvtVideoPopData[videoPopupData.m_pendindingVideoPopupCount].m_deviceName = deviceName;
                    videoPopupData.next_cameraEvtVideoPopData[videoPopupData.m_pendindingVideoPopupCount].m_eventType = eventType;
                    videoPopupData.next_cameraEvtVideoPopData[videoPopupData.m_pendindingVideoPopupCount].m_videoPopUpTimeDuration = videoPopUpDuration;
                    videoPopupData.m_pendindingVideoPopupCount++;
                    videoPopupData.m_videoPopupStartFlag = true;
                }
                break;
            }
        }
        break;

        case VIDEO_POPUP_STATE_START_VIDEO:
        case VIDEO_POPUP_STATE_VIDEO_ON:
        case VIDEO_POPUP_STATE_START_VIDEO_WAIT:
        {
            videoPopupData.cameraEvtVideoPopData[windowIndex].m_cameraIndexForFeature = cameraIndex;
            videoPopupData.cameraEvtVideoPopData[windowIndex].m_deviceName = deviceName;
            videoPopupData.cameraEvtVideoPopData[windowIndex].m_eventType = eventType;
            videoPopupData.cameraEvtVideoPopData[windowIndex].m_videoPopUpTimeDuration = videoPopUpDuration;
            updateVideoPopupParam(false, windowIndex);
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void Layout::startVideoPopupFeature()
{
    if (videoPopupData.m_timerIdForVideoPopup == 0)
    {
        videoPopupData.m_timerIdForVideoPopup = startTimer(1000);
    }
}

void Layout::closeVideoPopupFeatureWindow(quint16 windowIndex)
{
    if (videoPopupData.cameraEvtVideoPopData[windowIndex].m_cameraIndexForFeature == INVALID_CAMERA_INDEX)
    {
        return;
    }

    quint8 pendingCameraCount = 0;
    for(quint8 popupWindow = 0; popupWindow < MAX_POPUP_WINDOWS; popupWindow++)
    {
        if(videoPopupData.cameraEvtVideoPopData[popupWindow].m_cameraIndexForFeature != INVALID_CAMERA_INDEX)
        {
            pendingCameraCount++;
        }
    }

    quint16 windowActual = videoPopupData.cameraEvtVideoPopData[windowIndex].m_windowIndexForFeature;
    freeWindow(MAIN_DISPLAY, windowActual, true);

    videoPopupData.cameraEvtVideoPopData[windowIndex].m_deviceName = "";
    videoPopupData.cameraEvtVideoPopData[windowIndex].m_windowIndexForFeature = MAX_POPUP_WINDOWS;
    videoPopupData.cameraEvtVideoPopData[windowIndex].m_eventType = 0;
    videoPopupData.cameraEvtVideoPopData[windowIndex].m_cameraIndexForFeature = INVALID_CAMERA_INDEX;
    videoPopupData.cameraEvtVideoPopData[windowIndex].m_timerCounter = 0;
    videoPopupData.cameraEvtVideoPopData[windowIndex].m_replaceWindowVideoPopup = false;

    if (pendingCameraCount > 1)
    {
        return;
    }

    videoPopupData.m_currentFeatureState = VIDEO_POPUP_STATE_STOP_VIDEO;
    killTimer(videoPopupData.m_timerIdForVideoPopup);
    videoPopupData.m_timerIdForVideoPopup = 0;

    quint16 windowLimit[2] = {0,MAX_CHANNEL_FOR_SEQ};
    getFirstAndLastWindow(currentDisplayConfig[MAIN_DISPLAY].currPage, currentDisplayConfig[MAIN_DISPLAY].layoutId, windowLimit);

    quint16 windowDiff = windowLimit[1] - windowLimit[0];
    for(quint8 index = 0; index <= windowDiff; index++)
    {
        layoutCreator->changeWindowType(index, WINDOW_TYPE_LAYOUT);
    }

    changeCurrentMode(STATE_OTHER_MODE_PROCESSING_STOP_REQUEST, MAIN_DISPLAY, false);
    processNextActionForOtherMode(MAIN_DISPLAY);
}

void Layout::stopVideoPopupFeature()
{
    videoPopupData.m_currentFeatureState = VIDEO_POPUP_STATE_STOP_VIDEO;
    changeCurrentMode(STATE_VIDEO_POPUP_FEATURE, MAIN_DISPLAY);
}

void Layout::resetVideoPopupFeature()
{
    videoPopupData.m_timerIdForVideoPopup = 0;
    videoPopupData.m_currentFeatureState = VIDEO_POPUP_STATE_NONE;
    videoPopupData.m_previousSelectedWindow = MAX_CHANNEL_FOR_SEQ;
    videoPopupData.m_previousSelectedLayout = MAX_LAYOUT;
    videoPopupData.m_stopAllStream = false;
    videoPopupData.m_windowIndexForFeatureCurrentPage = MAX_CHANNEL_FOR_SEQ;
    videoPopupData.m_currentFeatureState = VIDEO_POPUP_STATE_NONE;
    videoPopupData.m_previousSelectedLayout = MAX_LAYOUT;
    videoPopupData.m_oldLayoutId = MAX_LAYOUT;
    videoPopupData.m_previousBeforeOneXOne = MAX_LAYOUT;
    videoPopupData.m_wasExpandMode = false;
    videoPopupData.m_previousMainState = MAX_STATE_LAYOUT;
    videoPopupData.m_pendindingVideoPopupCount = MAX_POPUP_WINDOWS;
    videoPopupData.m_videoPopupStartFlag = false;
    videoPopupData.m_changeLayoutForVideoPopup = true;
    videoPopupData.m_isRetryingMode = false;
    videoPopupData.m_startAudioInWindow = MAX_CHANNEL_FOR_SEQ;

    for(quint8 windowActual = 0; windowActual < MAX_WINDOWS; windowActual++)
    {
        memset(&videoPopupData.m_backupWindowInfo[windowActual], 0, sizeof(WINDOW_INFO_t));
        for(quint8 window = 0; window < MAX_WIN_SEQ_CAM; window++)
        {
            videoPopupData.m_backupWindowInfo[windowActual].camInfo[window].deviceName[0] = '\0';
            videoPopupData.m_backupWindowInfo[windowActual].camInfo[window].defChannel = INVALID_CAMERA_INDEX;
        }
    }

    for(quint8 channelIndex = 0; channelIndex < MAX_POPUP_WINDOWS; channelIndex++)
    {
        videoPopupData.cameraEvtVideoPopData[channelIndex].m_windowIndexForFeature = MAX_CHANNEL_FOR_SEQ;
        videoPopupData.cameraEvtVideoPopData[channelIndex].m_videoPopUpTimeDuration = 10;
        videoPopupData.cameraEvtVideoPopData[channelIndex].m_eventType = 0;
        videoPopupData.cameraEvtVideoPopData[channelIndex].m_cameraIndexForFeature = INVALID_CAMERA_INDEX;
        videoPopupData.cameraEvtVideoPopData[channelIndex].m_timerCounter = 0;
        videoPopupData.cameraEvtVideoPopData[channelIndex].m_replaceWindowVideoPopup = false;
    }
}

void Layout::backupCurrentConfigurationForVideoPopup()
{
    quint16 windowLimit[2];
    quint16 windowIndexActual;
    quint16 windowDiff;

    if((currentModeType[MAIN_DISPLAY] == STATE_COSEC_FEATURE) || (currentModeType[MAIN_DISPLAY] == STATE_ANALOG_FEATURE))
    {
        quint16 windowLimitCosecAnalogPopup[2];
        quint16 currentPage = ((videoPopupData.m_previousSelectedWindow) / (windowPerPage[videoPopupData.m_previousSelectedLayout]));
        getFirstAndLastWindow(currentPage, videoPopupData.m_previousSelectedLayout, windowLimitCosecAnalogPopup);

        if(windowLimitCosecAnalogPopup[0] == cosecPopupEvtData.m_windowIndexForFeature)
        {
            memcpy(&currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowLimitCosecAnalogPopup[0]],
                    &cosecPopupEvtData.m_backupWindowInfo, sizeof(WINDOW_INFO_t));
        }
        else if(windowLimitCosecAnalogPopup[0] == analogFeatureDataType.m_windowIndexForFeature)
        {
            memcpy(&videoPopupData.m_backupWindowInfo[windowLimitCosecAnalogPopup[0]],
                   &analogFeatureDataType.m_backupWindowInfo, sizeof(WINDOW_INFO_t));
        }
    }

    windowLimit[0] = 0;
    windowLimit[1] = 15;
    windowDiff = windowLimit[1] - windowLimit[0];
    windowIndexActual = windowLimit[0];

    for(quint16 windowIndex = 0; windowIndex <= windowDiff; windowIndex++)
    {
        if((currentModeType[MAIN_DISPLAY] == STATE_OTHER_MODE_NONE) || (currentModeType[MAIN_DISPLAY] == STATE_EXPAND_WINDOW) ||
                (currentModeType[MAIN_DISPLAY] == STATE_COSEC_FEATURE) || (currentModeType[MAIN_DISPLAY] == STATE_ANALOG_FEATURE))
        {
            memcpy(&videoPopupData.m_backupWindowInfo[windowIndex],
                   &currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndexActual], sizeof(WINDOW_INFO_t));
        }

        memset(&currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndexActual], 0, sizeof(WINDOW_INFO_t));
        for(quint8 channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
        {
            currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndexActual].camInfo[channelIndex].deviceName[0] = '\0';
            currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndexActual].camInfo[channelIndex].defChannel = INVALID_CAMERA_INDEX;
        }
        windowIndexActual++;
    }
}

void Layout::updateCurrentConfigurationForVideoPopup()
{
    quint16 windowLimit[2];
    quint16 windowIndexActual;

    windowLimit[0] = 0;
    windowLimit[1] = 15;

    quint16 windowDiff = windowLimit[1] - windowLimit[0];
    windowIndexActual = windowLimit[0];

    for(quint16 windowIndex = 0; windowIndex <= windowDiff; windowIndex++)
    {
        memset(&currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndexActual], 0, sizeof(WINDOW_INFO_t));
        for(quint8 channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
        {
            currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndexActual].camInfo[channelIndex].deviceName[0] = '\0';
            currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndexActual].camInfo[channelIndex].defChannel = INVALID_CAMERA_INDEX;
        }
        windowIndexActual++;
    }

    windowIndexActual = windowLimit[0];
    for(quint16 windowIndex = 0; windowIndex <= windowDiff; windowIndex++)
    {
        memcpy(&currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndexActual],
               &videoPopupData.m_backupWindowInfo[windowIndex], sizeof(WINDOW_INFO_t));
        windowIndexActual++;
    }

    for(quint8 index = 0; index <= windowDiff ; index++)
    {
        layoutCreator->changeWindowType(index, WINDOW_TYPE_LAYOUT);
    }
}

quint16 Layout::getVideoPopupLiveCam()
{
    quint8 pendingCameraCount = 0;
    for(quint8 windowIndex = 0; windowIndex < MAX_POPUP_WINDOWS; windowIndex++)
    {
        if(videoPopupData.cameraEvtVideoPopData[windowIndex].m_cameraIndexForFeature != INVALID_CAMERA_INDEX)
        {
            pendingCameraCount++;
        }
    }
    return pendingCameraCount;
}

void Layout::updateVideoPopupParam(bool saveLayoutParamFlag,quint16 windowIndex)
{
    //save present layout
    if(saveLayoutParamFlag)
    {
        videoPopupData.m_previousSelectedWindow = currentDisplayConfig[MAIN_DISPLAY].selectedWindow;
        videoPopupData.m_previousSelectedLayout = currentDisplayConfig[MAIN_DISPLAY].layoutId;
    }

    quint16 windowIndexActual = 0;
    if(videoPopupData.m_currentFeatureState == VIDEO_POPUP_STATE_NONE)
    {
        videoPopupData.m_currentFeatureState = VIDEO_POPUP_BACKUP;
        windowIndexActual = windowIndex;
        videoPopupData.cameraEvtVideoPopData[windowIndex].m_windowIndexForFeature = windowIndexActual;
        videoPopupData.m_windowIndexForFeatureCurrentPage = windowIndexActual;
        backupCurrentConfigurationForVideoPopup();
    }
    else
    {
        videoPopupData.cameraEvtVideoPopData[windowIndex].m_windowIndexForFeature = videoPopupData.m_windowIndexForFeatureCurrentPage + windowIndex;
        windowIndexActual = videoPopupData.cameraEvtVideoPopData[windowIndex].m_windowIndexForFeature;
    }

    snprintf(currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndexActual].camInfo[0].deviceName, MAX_DEVICE_NAME_SIZE, "%s",
            videoPopupData.cameraEvtVideoPopData[windowIndex].m_deviceName.toUtf8().constData());
    currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndexActual].camInfo[0].defChannel =
            videoPopupData.cameraEvtVideoPopData[windowIndex].m_cameraIndexForFeature;

    if(videoPopupData.m_changeLayoutForVideoPopup == true)
    {
        quint16 liveCamVideoPopup = getVideoPopupLiveCam();
        if(liveCamVideoPopup != 0)
        {
            //shifting window
            if(liveCamVideoPopup == 1)
            {
                shiftWindows(MAIN_DISPLAY, windowIndexActual, ONE_X_ONE);
            }
            else if(liveCamVideoPopup < 5)
            {
                shiftWindows(MAIN_DISPLAY, windowIndexActual, TWO_X_TWO);
            }
            else if(liveCamVideoPopup < 10)
            {
                videoPopupData.m_changeLayoutForVideoPopup = false;
                shiftWindows(MAIN_DISPLAY, windowIndexActual, THREE_X_THREE);
            }
        }
        else
        {
            DPRINT(LAYOUT, "camera found for live stream in video popup");
        }
    }

     videoPopupImageReplace();

    streamInfoArray[MAIN_DISPLAY][windowIndexActual].eventType = videoPopupData.cameraEvtVideoPopData[windowIndex].m_eventType;
    if(videoPopupData.m_stopAllStream == false)
    {
        //when changed mode toolbar and stream stoping is done in its handling
        videoPopupData.m_stopAllStream = true;
        videoPopupData.m_currentFeatureState = VIDEO_POPUP_STATE_STOP_OLD_WINDOW;
        changeCurrentMode(STATE_VIDEO_POPUP_FEATURE, MAIN_DISPLAY);
    }
    else if(videoPopupData.m_currentFeatureState == VIDEO_POPUP_STATE_VIDEO_ON
            || (videoPopupData.m_currentFeatureState == VIDEO_POPUP_STATE_START_VIDEO_WAIT))
    {
        if(videoPopupData.cameraEvtVideoPopData[windowIndex].m_replaceWindowVideoPopup == false)
        {
                startLiveStream(MAIN_DISPLAY, videoPopupData.cameraEvtVideoPopData[windowIndex].m_deviceName,
                                videoPopupData.cameraEvtVideoPopData[windowIndex].m_cameraIndexForFeature,
                                videoPopupData.cameraEvtVideoPopData[windowIndex].m_windowIndexForFeature);
        }
        else
        {
            if(streamInfoArray[MAIN_DISPLAY][windowIndex].m_streamId != MAX_STREAM_SESSION)
            {
                videoPopupData.cameraEvtVideoPopData[windowIndex].m_replaceWindowVideoPopup = false;
                replaceLiveStream(MAIN_DISPLAY,
                                  windowIndex,
                                  videoPopupData.cameraEvtVideoPopData[windowIndex].m_deviceName,
                                  videoPopupData.cameraEvtVideoPopData[windowIndex].m_cameraIndexForFeature,
                                  windowIndex);
            }
        }
    }
}

void Layout::videoPopupImageReplace()
{
    if (videoPopupData.m_oldLayoutId != currentDisplayConfig[MAIN_DISPLAY].layoutId)
    {
        quint16 windowLimit[2] = {0, MAX_CHANNEL_FOR_SEQ};
        quint8 windowDiff;
        getFirstAndLastWindow(currentDisplayConfig[MAIN_DISPLAY].currPage, currentDisplayConfig[MAIN_DISPLAY].layoutId, windowLimit);

        windowDiff = windowLimit[1] - windowLimit[0];
        for(quint8 index = 0; index <= windowDiff ; index++)
        {
            layoutCreator->changeWindowType(index, WINDOW_TYPE_VIDEO_POP_UP);
            layoutCreator->updateWindowData(index);
        }
    }
    videoPopupData.m_oldLayoutId = currentDisplayConfig[MAIN_DISPLAY].layoutId;
}

void Layout::unloadAndHideOptionsInVideoPopup()
{
    //if ptz control is open , then hide it
    if(m_ptzControl != NULL)
    {
        m_inVisibleWidget->setVisible(false);
        m_ptzControl->setVisible(false);
    }

    //if view camera control is open , then hide it
    if(m_viewCamera != NULL)
    {
        m_inVisibleWidget->setVisible(false);
        m_viewCamera->setVisible(false);
    }

    if(m_infoPage->isVisible())
    {
        m_infoPage->unloadInfoPage();
    }

    if(m_analogPresetMenu != NULL)
    {
        m_inVisibleWidget->setVisible(false);
        m_analogPresetMenu->setVisible(false);
    }
}

void Layout::loadOptionsInVideoPopup()
{
    //if ptz control is open, then show it
    if(m_ptzControl != NULL)
    {
        m_inVisibleWidget->setVisible(true);
        m_ptzControl->setVisible(true);
    }

    //if view camera control is open, then show it
    if(m_viewCamera != NULL)
    {
        m_inVisibleWidget->setVisible(true);
        m_viewCamera->setVisible(true);
    }

    if(m_analogPresetMenu != NULL)
    {
        m_inVisibleWidget->setVisible(true);
        m_analogPresetMenu->setVisible(true);
    }
}

void Layout::showCosecPopupFeature(quint8 cameraIndex, QString userName, quint32 popUpTime, QString userId, QString doorName, quint8 eventStausCode)
{
    if((currentDisplayConfig[MAIN_DISPLAY].layoutId == ONE_X_ONE) && (cosecPopupEvtData.m_previousBeforeOneXOne == MAX_LAYOUT)
            && (m_doubleClickModeFlag == true))
    {
        cosecPopupEvtData.m_previousBeforeOneXOne = m_previousLayout[MAIN_DISPLAY];
    }
    else
    {
        cosecPopupEvtData.m_previousBeforeOneXOne = MAX_LAYOUT;
    }

    m_doubleClickModeFlag = false;
    m_previousSelectedWindow = MAX_CHANNEL_FOR_SEQ;
    m_cosecHandleLaterFlag = false;

    bool updateOld = false;
    switch(cosecPopupEvtData.m_currentFeatureState)
    {
        case FEATURE_STATE_NONE:
            updateOld = true;
            break;

        case FEATURE_STATE_STOP_OLD_WINDOW:
            updateOld = true;
            break;

        case FEATURE_STATE_START_VIDEO:
        case FEATURE_STATE_VIDEO_ON:
            if(cameraIndex == cosecPopupEvtData.m_cameraIndexForFeature)
            {
                updateOld = true;
            }
            break;

        default:
            break;
    }

    if(updateOld == true)
    {
        cosecPopupEvtData.m_cameraIndexForFeature = cameraIndex;
        cosecPopupEvtData.m_doorName = doorName;
        cosecPopupEvtData.m_eventStatusCode = eventStausCode;
        cosecPopupEvtData.m_eventPopUpTime = popUpTime;
        cosecPopupEvtData.m_userName = userName;
        cosecPopupEvtData.m_userId = userId;

        cosecPopupEvtData.m_nextCameraIndexForFeature = INVALID_CAMERA_INDEX;
        cosecPopupEvtData.m_nextDoorName = "";
        cosecPopupEvtData.m_nextEventStatusCode = 255;
        cosecPopupEvtData.m_nextEventPopUpTime = 0;
        cosecPopupEvtData.m_nextUserName = "";
    }
    else
    {
        cosecPopupEvtData.m_nextCosecPending = true;
        cosecPopupEvtData.m_nextCameraIndexForFeature = cameraIndex;
        cosecPopupEvtData.m_nextDoorName = doorName;
        cosecPopupEvtData.m_nextEventStatusCode = eventStausCode;
        cosecPopupEvtData.m_nextEventPopUpTime = popUpTime;
        cosecPopupEvtData.m_nextUserName = userName;
        cosecPopupEvtData.m_userId = userId;
    }

    quint16 windowIndex;
    switch(cosecPopupEvtData.m_currentFeatureState)
    {
        case FEATURE_STATE_NONE:
        {
            switch(currentModeType[MAIN_DISPLAY])
            {
                case STATE_OTHER_MODE_NONE:
                {
                    pauseSequencing(MAIN_DISPLAY);
                    pauseWindowSequencing(MAIN_DISPLAY);
                    if((m_liveViewToolbar != NULL))
                    {
                        slotCloseLiveViewToolbar();
                    }
                    updateCosecPopupParam();
                }
                break;

                case STATE_ANALOG_FEATURE:
                {
                    m_cosecHandleLaterFlag = true;
                    if((analogFeatureDataType.m_previousBeforeOneXOne != MAX_LAYOUT) && (cosecPopupEvtData.m_previousBeforeOneXOne == MAX_LAYOUT))
                    {
                        cosecPopupEvtData.m_previousBeforeOneXOne = analogFeatureDataType.m_previousBeforeOneXOne;
                    }
                    else
                    {
                        cosecPopupEvtData.m_previousBeforeOneXOne = MAX_LAYOUT;
                    }
                    hideAnalogCameraFeature();
                }
                break;

                case STATE_VIDEO_POPUP_FEATURE:
                {
                    m_cosecHandleLaterFlag = false;
                    resetCosecPopupFeature();
                }
                break;

                default:
                {
                    m_cosecHandleLaterFlag = true;
                }
                break;
            }
        }
        break;

        case FEATURE_STATE_STOP_OLD_WINDOW:
        {
            windowIndex = cosecPopupEvtData.m_windowIndexForFeature;
            snprintf(currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[0].deviceName, MAX_DEVICE_NAME_SIZE, LOCAL_DEVICE_NAME);
            currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[0].defChannel = cosecPopupEvtData.m_cameraIndexForFeature;
        }
        break;

        case FEATURE_STATE_VIDEO_ON:
        {
            if(cameraIndex != cosecPopupEvtData.m_cameraIndexForFeature)
            {
                windowIndex = cosecPopupEvtData.m_windowIndexForFeature;
                currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[0].deviceName[0] = '\0';
                currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[0].defChannel = INVALID_CAMERA_INDEX;
                cosecPopupEvtData.m_currentFeatureState = FEATURE_STATE_STOP_OLD_WINDOW;
                changeCurrentMode(STATE_COSEC_FEATURE, MAIN_DISPLAY);
            }
            else
            {
                startCosecPopupFeature();
            }
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void Layout::updateCosecPopupParam(bool saveLayoutParamFlag)
{
    if(saveLayoutParamFlag)
    {
        cosecPopupEvtData.m_previousSelectedWindow = currentDisplayConfig[MAIN_DISPLAY].selectedWindow;
        cosecPopupEvtData.m_previousSelectedLayout = currentDisplayConfig[MAIN_DISPLAY].layoutId;
    }

    quint16 windowIndex = findWindowIndexIfAssignOnCurrentPage(MAIN_DISPLAY, LOCAL_DEVICE_NAME, cosecPopupEvtData.m_cameraIndexForFeature);
    if ((currentDisplayConfig[MAIN_DISPLAY].layoutId >= 0) && (currentDisplayConfig[MAIN_DISPLAY].layoutId < MAX_LAYOUT)
            && (windowIndex == MAX_CHANNEL_FOR_SEQ))
    {
        windowIndex = currentDisplayConfig[MAIN_DISPLAY].currPage * windowPerPage[currentDisplayConfig[MAIN_DISPLAY].layoutId];
    }

    if (windowIndex < MAX_CHANNEL_FOR_SEQ)
    {
        memcpy(&cosecPopupEvtData.m_backupWindowInfo, &currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex], sizeof(WINDOW_INFO_t));
        memset(&currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex], 0, sizeof(WINDOW_INFO_t));

        for(quint8 channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
        {
            currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[channelIndex].deviceName[0] = '\0';
            currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[channelIndex].defChannel = INVALID_CAMERA_INDEX;
        }

        //change style and layout
        cosecPopupEvtData.m_windowIndexForFeature = windowIndex;
        snprintf(currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[0].deviceName, MAX_DEVICE_NAME_SIZE, LOCAL_DEVICE_NAME);
        currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[0].defChannel = cosecPopupEvtData.m_cameraIndexForFeature;
        shiftWindows(MAIN_DISPLAY, windowIndex, ONE_X_ONE);
    }

    cosecPopupEvtData.m_currentFeatureState = FEATURE_STATE_STOP_OLD_WINDOW;
    changeCurrentMode(STATE_COSEC_FEATURE, MAIN_DISPLAY);

    switch(ApplicationMode::getApplicationMode())
    {
        case PAGE_MODE:
            emit sigHideToolbarPage();
            break;

        case PAGE_WITH_TOOLBAR_MODE:
        case TOOLBAR_MODE:
            emit sigUnloadToolbar();
            break;

        case IDLE_MODE:
            emit sigUnloadToolbar();
            unloadAndHideOptionsInCosecPopup();
            break;

        default:
            break;
    }

    cosecPopupEvtData.m_isCosecHandlingStarted = true;
}

void Layout::startCosecPopupFeature()
{
    QString tempMsgStr;
    quint16 textWidth = 0, textHeight = 0;
    QFont textFont = TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE);
    MessageBanner::flushQueueOfMsgBanner();

    switch(cosecPopupEvtData.m_eventStatusCode)
    {
        case COSEC_POPUP_USR_ALLOWED:
            tempMsgStr = Multilang("Access allowed to") + QString(" ") +
                    cosecPopupEvtData.m_userName +  QString(" ") + Multilang("on Door") + QString(" - ") + cosecPopupEvtData.m_doorName;
            break;

        case COSEC_POPUP_USR_DENIED:
            tempMsgStr = Multilang("Access denied to") + QString(" ") +
                    cosecPopupEvtData.m_userName +  QString(" ") + Multilang("on Door") + QString(" - ") + cosecPopupEvtData.m_doorName;
            break;

        case COSEC_POPUP_NOT_IDENTIFY:
            tempMsgStr = Multilang("Access denied on Door -") + QString(" ") +  cosecPopupEvtData.m_doorName;
            break;

        case COSEC_POPUP_AUX_CHNANGE:
            tempMsgStr = Multilang("Status of Aux In-1 on Door -") + QString(" ") + cosecPopupEvtData.m_doorName + QString(" ") + Multilang("has changed");
            break;

        case COSEC_POPUP_DURESS_DETECT:
            tempMsgStr = Multilang("Duress Alarm activated by") + QString(" ") +
                    cosecPopupEvtData.m_userName +  QString(" ") + Multilang("on Door") + QString(" - ") + cosecPopupEvtData.m_doorName;
            break;

        case COSEC_POPUP_DEAD_MAN_TMR_EXPR:
            tempMsgStr = Multilang("Dead Man timer on Door -") + QString(" ") + cosecPopupEvtData.m_doorName + QString(" ") +
                    Multilang("expired for") + QString(" ") + cosecPopupEvtData.m_userName;
            break;

        case COSEC_POPUP_PANIC_ALARM:
            tempMsgStr = Multilang("Panic Alarm detected on Door") + QString(" - ") + cosecPopupEvtData.m_doorName;
            break;

        case COSEC_POPUP_DOOR_ABNORMAL:
            tempMsgStr = Multilang("Door -") + QString(" ") + cosecPopupEvtData.m_doorName +  QString(" ") +Multilang("detected abnormal");
            break;

        case COSEC_POPUP_DOOR_FORCE_OPEN:
            tempMsgStr = Multilang("Door -") + QString(" ") + cosecPopupEvtData.m_doorName + QString(" ") + Multilang("was forced opened");
            break;

        case COSEC_POPUP_DOOR_TEMPAR_ALARM:
            tempMsgStr = Multilang("Tamper Alarm detected on Door -") + QString(" ") + cosecPopupEvtData.m_doorName;
            break;

        case COSEC_POPUP_INTERCOM_PANIC_ALARM:
            tempMsgStr = Multilang("Intercom Panic Alarm detected on door-") + cosecPopupEvtData.m_doorName + QString(" ") +
                    Multilang("by Extension Number-") + cosecPopupEvtData.m_userId;
            break;

        case COSEC_POPUP_AUX2_CHANGE:
            tempMsgStr = Multilang("Status of Aux In-2 on Door -") + cosecPopupEvtData.m_doorName + QString(" ") + Multilang("has changed");
            break;

        case COSEC_POPUP_AUX3_CHANGE:
            tempMsgStr = Multilang("Status of Aux In-3 on Door -") + cosecPopupEvtData.m_doorName + QString(" ") + Multilang("has changed");
            break;

        case COSEC_POPUP_AUX4_CHANGE:
            tempMsgStr = Multilang("Status of Aux In-4 on Door -") + cosecPopupEvtData.m_doorName + QString(" ") + Multilang("has changed");
            break;

        case COSEC_POPUP_AUX5_CHANGE:
            tempMsgStr = Multilang("Status of Aux In-5 on Door -") + cosecPopupEvtData.m_doorName + QString(" ") + Multilang("has changed");
            break;

        case COSEC_POPUP_AUX6_CHANGE:
            tempMsgStr = Multilang("Status of Aux In-6 on Door -") + cosecPopupEvtData.m_doorName + QString(" ") + Multilang("has changed");
            break;

        case COSEC_POPUP_AUX7_CHANGE:
            tempMsgStr = Multilang("Status of Aux In-7 on Door -") + cosecPopupEvtData.m_doorName + QString(" ") + Multilang("has changed");
            break;

        case COSEC_POPUP_AUX8_CHANGE:
            tempMsgStr = Multilang("Status of Aux In-8 on Door -") + cosecPopupEvtData.m_doorName + " " + Multilang("has changed");
            break;

        default:
            break;
    }

    textWidth = QFontMetrics(textFont).width(tempMsgStr) + SCALE_WIDTH(20);
    textHeight = QFontMetrics(textFont).height() + SCALE_FONT(15);

    deleteCosecPopUpBanner();
    deleteCosecPopupObject();

    /* PARASOFT: Memory Deallocated in Delete cosecPopup banner */
    m_cosecMsgBannerRect = new Rectangle((this->width() - textWidth)/2,
                                         SCALE_HEIGHT(5),
                                         textWidth,
                                         textHeight,
                                         0,
                                         WINDOW_GRID_COLOR,
                                         CLICKED_BKG_COLOR,
                                         this, SCALE_WIDTH(2));

    /* PARASOFT: Memory Deallocated in Delete cosecPopup banner */
    m_cosecPopupMsg = new TextLabel(m_cosecMsgBannerRect->x() + m_cosecMsgBannerRect->width()/2,
                                    m_cosecMsgBannerRect->y() + m_cosecMsgBannerRect->height()/2,
                                    NORMAL_FONT_SIZE,
                                    tempMsgStr, this,
                                    NORMAL_FONT_COLOR,
                                    NORMAL_FONT_FAMILY,
                                    ALIGN_CENTRE_X_CENTER_Y, 0, false, (textWidth + SCALE_WIDTH(50)));

    if(m_timerIdForCosecBanner != 0)
    {
        killTimer(m_timerIdForCosecBanner);
        m_timerIdForCosecBanner = 0;
    }
    m_timerIdForCosecBanner = startTimer(COSEC_BANNER_INTERVAL);

    if(m_timerIdForCosecPopup != 0)
    {
        killTimer(m_timerIdForCosecPopup);
        m_timerIdForCosecPopup = 0;
    }
    m_timerIdForCosecPopup = startTimer(cosecPopupEvtData.m_eventPopUpTime * 1000);
}

void Layout::stopCosecPopupFeature()
{
    if(cosecPopupEvtData.m_nextCosecPending == true)
    {
        cosecPopupEvtData.m_nextCosecPending = false;
        showCosecPopupFeature(cosecPopupEvtData.m_nextCameraIndexForFeature, cosecPopupEvtData.m_nextUserName,
                              cosecPopupEvtData.m_eventPopUpTime, cosecPopupEvtData.m_userId,
                              cosecPopupEvtData.m_doorName, cosecPopupEvtData.m_eventStatusCode);
    }
    else
    {
        if(cosecPopupEvtData.m_previousBeforeOneXOne != MAX_LAYOUT)
        {
            m_doubleClickModeFlag = true;
            m_previousLayout[MAIN_DISPLAY] = cosecPopupEvtData.m_previousBeforeOneXOne;
        }

        //reset to default state
        selectWindow(MAIN_DISPLAY, cosecPopupEvtData.m_previousSelectedWindow);
        resetCosecPopupFeature();
        emit sigCloseCosecPopup();
    }
}

void Layout::closeCosecPopupFeature()
{
    deleteCosecPopUpBanner();
    deleteCosecPopupObject();

    quint16 windowIndex = cosecPopupEvtData.m_windowIndexForFeature;
    memcpy(&currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex], &cosecPopupEvtData.m_backupWindowInfo, sizeof(WINDOW_INFO_t));
    shiftWindows(MAIN_DISPLAY, currentDisplayConfig[MAIN_DISPLAY].selectedWindow, cosecPopupEvtData.m_previousSelectedLayout);
    cosecPopupEvtData.m_currentFeatureState = FEATURE_STATE_STOP_VIDEO;
    changeCurrentMode(STATE_COSEC_FEATURE, MAIN_DISPLAY);
}

void Layout::resetCosecPopupFeature()
{
    cosecPopupEvtData.m_isCosecHandlingStarted = false;
    cosecPopupEvtData.m_previousSelectedLayout = MAX_LAYOUT;
    cosecPopupEvtData.m_previousBeforeOneXOne = MAX_LAYOUT;
    cosecPopupEvtData.m_windowIndexForFeature = MAX_CHANNEL_FOR_SEQ;
    memset(&cosecPopupEvtData.m_backupWindowInfo, 0, sizeof(WINDOW_INFO_t));

    for(quint8 channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
    {
        cosecPopupEvtData.m_backupWindowInfo.camInfo[channelIndex].deviceName[0] = '\0';
        cosecPopupEvtData.m_backupWindowInfo.camInfo[channelIndex].defChannel = INVALID_CAMERA_INDEX;
    }
    cosecPopupEvtData.m_previousSelectedWindow = MAX_CHANNEL_FOR_SEQ;
    cosecPopupEvtData.m_currentFeatureState = FEATURE_STATE_NONE;

    cosecPopupEvtData.m_nextCameraIndexForFeature = INVALID_CAMERA_INDEX;
    cosecPopupEvtData.m_nextEventStatusCode = 255;
    cosecPopupEvtData.m_nextEventPopUpTime = 0;
    cosecPopupEvtData.m_nextUserName = "";

    cosecPopupEvtData.m_cameraIndexForFeature = INVALID_CAMERA_INDEX;
    cosecPopupEvtData.m_doorName = "";
    cosecPopupEvtData.m_eventStatusCode = 255;
    cosecPopupEvtData.m_eventPopUpTime = 0;
    cosecPopupEvtData.m_userName = "";
    cosecPopupEvtData.m_userId = "";
}

void Layout::deleteCosecPopUpBanner()
{
    if(m_timerIdForCosecBanner != 0)
    {
        killTimer(m_timerIdForCosecBanner);
        m_timerIdForCosecBanner = 0;

        DELETE_OBJ(m_cosecPopupMsg);
        DELETE_OBJ(m_cosecMsgBannerRect);
    }
}

void Layout::deleteCosecPopupObject()
{
    if(m_timerIdForCosecPopup != 0)
    {
        killTimer(m_timerIdForCosecPopup);
        m_timerIdForCosecPopup = 0;
    }

    DELETE_OBJ(m_cosecVideoPopupUser);
}

void Layout::unloadAndHideOptionsInCosecPopup()
{
    //if ptz control is open , then hide it
    if(m_ptzControl != NULL)
    {
        m_inVisibleWidget->setVisible(false);
        m_ptzControl->setVisible(false);
    }

    //if view camera control is open , then hide it
    if(m_viewCamera != NULL)
    {
        m_inVisibleWidget->setVisible(false);
        m_viewCamera->setVisible(false);
    }

    if(m_infoPage->isVisible())
    {
        m_infoPage->unloadInfoPage();
    }

    if(m_analogPresetMenu != NULL)
    {
        m_inVisibleWidget->setVisible(false);
        m_analogPresetMenu->setVisible(false);
    }
}

void Layout::loadAndShowOptionsInCosecPopup()
{
    //if ptz control is open, then show it
    if(m_ptzControl != NULL)
    {
        m_inVisibleWidget->setVisible(true);
        m_ptzControl->setVisible(true);
    }

    //if view camera control is open, then show it
    if(m_viewCamera != NULL)
    {
        m_inVisibleWidget->setVisible(true);
        m_viewCamera->setVisible(true);
    }

    if(m_analogPresetMenu != NULL)
    {
        m_inVisibleWidget->setVisible(true);
        m_analogPresetMenu->setVisible(true);
    }
}

void Layout::updateNextParamToCurrent()
{
    quint16 windowIndex = cosecPopupEvtData.m_windowIndexForFeature;
    cosecPopupEvtData.m_nextCosecPending = false;
    cosecPopupEvtData.m_cameraIndexForFeature = cosecPopupEvtData.m_nextCameraIndexForFeature;
    cosecPopupEvtData.m_doorName = cosecPopupEvtData.m_nextDoorName;
    cosecPopupEvtData.m_eventStatusCode = cosecPopupEvtData.m_nextEventStatusCode;
    cosecPopupEvtData.m_eventPopUpTime = cosecPopupEvtData.m_nextEventPopUpTime;
    cosecPopupEvtData.m_userName = cosecPopupEvtData.m_nextUserName;
    cosecPopupEvtData.m_userId = cosecPopupEvtData.m_userId;

    snprintf(currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[0].deviceName, MAX_DEVICE_NAME_SIZE, LOCAL_DEVICE_NAME);
    currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[0].defChannel = cosecPopupEvtData.m_cameraIndexForFeature;
}

void Layout::setSelectedWindow(quint16 windowIndex)
{
    currentDisplayConfig[MAIN_DISPLAY].selectedWindow = windowIndex;
    layoutCreator->changeSelectedWindow(windowIndex, true);

    if(streamInfoArray[MAIN_DISPLAY][windowIndex].m_videoType != VIDEO_TYPE_PLAYBACKSTREAM)
    {
        closeDeletePbToolBar();
    }

    if(streamInfoArray[MAIN_DISPLAY][windowIndex].m_videoType != VIDEO_TYPE_INSTANTPLAYBACKSTREAM)
    {
        closeDeleteInstantPbToolBar();
    }
}

void Layout::startSequencing(DISPLAY_TYPE_e displayType, bool forceStart)
{
    if((currentDisplayConfig[displayType].seqStatus == false) || (forceStart == true))
    {
        if((displayType == MAIN_DISPLAY) && (isPlaybackRunning()))
        {
            m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LAYOUT_PB_ERROR_MESSAGE));
        }
        else if(isWindowWiseSequeningRunning (displayType))
        {
            m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LAYOUT_WINDOW_SEQENC_ERROR_MESSAGE));
        }
        else
        {
            currentDisplayConfig[displayType].seqStatus = true;
            if(m_timerIdForSequencing[displayType] == 0)
            {
                m_timerIdForSequencing[displayType] = startTimer(currentDisplayConfig[displayType].seqInterval * 1000);
            }

            MessageBanner::addMessageInBanner("Auto Page Navigation start for MAIN Display");
            for(quint16 index = 0; index < MAX_CHANNEL_FOR_SEQ; index++)
            {
                playbackRecordData[index].clearPlaybackInfo();
            }

            emit sigChangeToolbarButtonState(SEQUENCE_BUTTON, STATE_2);
        }
    }
}

void Layout::stopSequencing(DISPLAY_TYPE_e displayType)
{
    if ((displayType < MAIN_DISPLAY) || (displayType >= MAX_DISPLAY_TYPE))
    {
        return;
    }

    if (currentDisplayConfig[displayType].seqStatus == true)
    {
        currentDisplayConfig[displayType].seqStatus = false;
        if(m_timerIdForSequencing[displayType] != 0)
        {
            killTimer(m_timerIdForSequencing[displayType]);
            m_timerIdForSequencing[displayType] = 0;
        }

        MessageBanner::addMessageInBanner("Auto Page Navigation stopped for MAIN Display");
        emit sigChangeToolbarButtonState(SEQUENCE_BUTTON, STATE_1);
    }
}

void Layout::pauseSequencing(DISPLAY_TYPE_e displayType)
{
    if(m_timerIdForSequencing[displayType] != 0)
    {
        killTimer(m_timerIdForSequencing[displayType]);
        m_timerIdForSequencing[displayType] = 0;
    }
}

void Layout::resumeSequencing(DISPLAY_TYPE_e displayType)
{
    if(currentDisplayConfig[displayType].seqStatus == true)
    {
        if(m_timerIdForSequencing[displayType] == 0)
        {
            m_timerIdForSequencing[displayType] = startTimer(currentDisplayConfig[displayType].seqInterval * 1000);

            if(displayType == MAIN_DISPLAY)
            {
                emit sigChangeToolbarButtonState(SEQUENCE_BUTTON, STATE_2);
            }
        }
    }
}

void Layout::startWindowSequencing(DISPLAY_TYPE_e displayType)
{
    quint16 windowLimit[2] = {0,MAX_CHANNEL_FOR_SEQ};

    if (true == currentDisplayConfig[displayType].seqStatus)
    {
        MessageBanner::addMessageInBanner (ValidationMessage::getValidationMessage(SEQUENCING_ON_MSG));
        return;
    }

    getFirstAndLastWindow(currentDisplayConfig[displayType].currPage, currentDisplayConfig[displayType].layoutId, windowLimit);
    for(quint16 windowIndex = windowLimit[0]; windowIndex <= windowLimit[1]; windowIndex++)
    {
        if(((currentDisplayConfig[displayType].windowInfo[windowIndex].sequenceStatus == true)
                 || (currentDisplayConfig[displayType].windowInfo[windowIndex].lastSequenceStatus == true))
                && (isMultipleChannelAssigned (displayType, windowIndex))
                && (currentDisplayConfig[displayType].windowInfo[windowIndex].sequenceInterval > 0))
        {
            currentDisplayConfig[displayType].windowInfo[windowIndex].lastSequenceStatus = false;
            if(m_timerIdForWindowSequence[displayType][windowIndex] == 0)
            {
                m_timerIdForWindowSequence[displayType][windowIndex] = startTimer(currentDisplayConfig[displayType].windowInfo[windowIndex].sequenceInterval * 1000);
            }

            if(m_timerIdForWindowSequence[MAIN_DISPLAY][windowIndex] <= 0)
            {
                EPRINT(LAYOUT, "fail to start window sequencing: [windowIndex=%d]", windowIndex);
            }
        }
        else
        {
            currentDisplayConfig[displayType].windowInfo[windowIndex].sequenceStatus = false;
            currentDisplayConfig[displayType].windowInfo[windowIndex].sequenceInterval = DEFAULT_WINDOW_SEQ_INTERVAL;
        }
    }
}

void Layout::stopWindowSequencing(DISPLAY_TYPE_e displayType)
{
    if ((displayType < MAIN_DISPLAY) || (displayType >= MAX_DISPLAY_TYPE))
    {
        return;
    }

    for(quint16 windowIndex = 0; windowIndex < MAX_CHANNEL_FOR_SEQ; windowIndex++)
    {
        if (m_timerIdForWindowSequence[displayType][windowIndex] != 0)
        {
            killTimer(m_timerIdForWindowSequence[displayType][windowIndex]);
            m_timerIdForWindowSequence[displayType][windowIndex] = 0;
            currentDisplayConfig[displayType].windowInfo[windowIndex].sequenceStatus = false;
            MessageBanner::addMessageInBanner("Window wise sequencing stopped for Main Display");
        }
    }
}

void Layout::pauseWindowSequencing(DISPLAY_TYPE_e displayType)
{
    quint16 windowLimit[2] = {0,MAX_CHANNEL_FOR_SEQ};

    if ((displayType < MAIN_DISPLAY) || (displayType >= MAX_DISPLAY_TYPE))
    {
        return;
    }

    getFirstAndLastWindow(currentDisplayConfig[displayType].currPage, currentDisplayConfig[displayType].layoutId, windowLimit);
    for(quint16 windowIndex = windowLimit[0]; windowIndex <= windowLimit[1]; windowIndex++)
    {
        if(m_timerIdForWindowSequence[displayType][windowIndex] != 0)
        {
            killTimer(m_timerIdForWindowSequence[displayType][windowIndex]);
            m_timerIdForWindowSequence[displayType][windowIndex] = 0;

            currentDisplayConfig[displayType].windowInfo[windowIndex].sequenceStatus = false;
            currentDisplayConfig[displayType].windowInfo[windowIndex].lastSequenceStatus = true;
        }
    }
}

void Layout::resumeWindowSequencing(DISPLAY_TYPE_e displayType)
{
    quint16 windowLimit[2] = {0,MAX_CHANNEL_FOR_SEQ};

    if (true == currentDisplayConfig[displayType].seqStatus)
    {
        if(currentModeType[MAIN_DISPLAY] != STATE_LOCAL_DECODING)
        {
            MessageBanner::addMessageInBanner (ValidationMessage::getValidationMessage(SEQUENCING_ON_MSG));
        }
        return;
    }

    getFirstAndLastWindow(currentDisplayConfig[displayType].currPage, currentDisplayConfig[displayType].layoutId, windowLimit);
    for(quint16 windowIndex = windowLimit[0]; windowIndex <= windowLimit[1]; windowIndex++)
    {
        if((((currentDisplayConfig[displayType].windowInfo[windowIndex].lastSequenceStatus == true) &&
            (m_timerIdForWindowSequence[displayType][windowIndex] == 0)) ||
                ((currentDisplayConfig[displayType].windowInfo[windowIndex].sequenceStatus == true) &&
                 (m_timerIdForWindowSequence[displayType][windowIndex] == 0))) && (isMultipleChannelAssigned (displayType, windowIndex)))
        {
            currentDisplayConfig[displayType].windowInfo[windowIndex].lastSequenceStatus = false;
            currentDisplayConfig[displayType].windowInfo[windowIndex].sequenceStatus = true;

            if(m_timerIdForWindowSequence[displayType][windowIndex] == 0)
            {
                m_timerIdForWindowSequence[displayType][windowIndex] = startTimer(currentDisplayConfig[displayType].windowInfo[windowIndex].sequenceInterval * 1000);
            }
        }
    }
}

void Layout::processingBeforeSyncPlayback()
{
    //stop default sequencing
    pauseSequencing(MAIN_DISPLAY);

    //stop Window sequencing
    pauseWindowSequencing(MAIN_DISPLAY);

    //clearplaybackdata
    for(quint16 index = 0; index < MAX_CHANNEL_FOR_SEQ; index++)
    {
        playbackRecordData[index].clearPlaybackInfo();
    }

    stopAutoPlayFeature();
    if((currentDisplayConfig[MAIN_DISPLAY].layoutId == ONE_X_ONE) && (m_doubleClickModeFlag == true))
    {
        m_previousSyncBeforeOneXOne = m_previousLayout[MAIN_DISPLAY];
    }
    else
    {
        m_previousSyncBeforeOneXOne = MAX_LAYOUT;
    }

    m_doubleClickModeFlag = false;
    m_previousSelectedWindow = MAX_CHANNEL_FOR_SEQ;

    //take backup
    m_syncBackupLayout = currentDisplayConfig[MAIN_DISPLAY].layoutId;
    m_syncBackupSelectedWindow = currentDisplayConfig[MAIN_DISPLAY].selectedWindow;

	/* Take Backup of Window(Live View) data as same window is used SyncPlayback */
	for(quint8 windowIndex = 0; windowIndex < MAX_SYNC_PB_SESSION; windowIndex++)
    {
        layoutCreator->changeWindowType(windowIndex, WINDOW_TYPE_SYNCPLAYBACK);
        memcpy(&m_syncBackupWinInfo[windowIndex], &currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex], sizeof(WINDOW_INFO_t));
        memset(&currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex], 0, sizeof(WINDOW_INFO_t));

        //Intialize current widow with default values
        for(quint8 channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
        {
            currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[channelIndex].deviceName[0] = '\0';
            currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[channelIndex].defChannel = INVALID_CAMERA_INDEX;
        }
    }

    //Shift window in sync playback
    shiftWindows(MAIN_DISPLAY, 0, ONE_X_ONE_PLAYBACK);

    if(((currentModeType[MAIN_DISPLAY] == STATE_OTHER_MODE_NONE) && (isAnyWindowReplacing(MAIN_DISPLAY) == false))
            || ((currentModeType[MAIN_DISPLAY] == STATE_LOCAL_DECODING)))
    {
        m_isCurrStyleSelected[MAIN_DISPLAY] = false;
        changeCurrentMode(STATE_SYNC_PLAYBACK_START_MODE, MAIN_DISPLAY);
        if(localDecodingFeature.m_timerToShowBannerMessage != 0)
        {
            killTimer (localDecodingFeature.m_timerToShowBannerMessage);
        }
    }
    else
    {
        if(isAnyWindowReplacing(MAIN_DISPLAY) == true)
        {
            m_processPendingForWindow[MAIN_DISPLAY] = true;
        }
        m_syncPlaybackStartFlag = true;
    }
}

void Layout::processingAfterSyncPlayback()
{
	/* Restore Window(Live View) data from Backup taken when SyncPlayback Page opened */
    for(quint8 windowIndex = 0; windowIndex < MAX_SYNC_PB_SESSION; windowIndex++)
    {
        memcpy(&currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex], &m_syncBackupWinInfo[windowIndex], sizeof(WINDOW_INFO_t));
        memset(&m_syncBackupWinInfo[windowIndex], 0, sizeof(WINDOW_INFO_t));

        //Intilized with default values
        for(quint8 channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
        {
            m_syncBackupWinInfo[windowIndex].camInfo[channelIndex].deviceName[0] = '\0';
            m_syncBackupWinInfo[windowIndex].camInfo[channelIndex].defChannel = INVALID_CAMERA_INDEX;
        }
    }

    //Back in previous window
    shiftWindows(MAIN_DISPLAY, m_syncBackupSelectedWindow, m_syncBackupLayout);

    /* Here the window type depends on current layout type and which is changed after the shiftwindow function */
    for(quint8 windowIndex = 0; windowIndex < MAX_SYNC_PB_SESSION; windowIndex++)
    {
        layoutCreator->changeWindowType(windowIndex, WINDOW_TYPE_LAYOUT);
    }

    DEV_TABLE_INFO_t devTableInfo;
    applController->GetDeviceInfo(LOCAL_DEVICE_NAME, devTableInfo);
    if(devTableInfo.startLiveView == false)
    {
        localDecodingFeature.pendingRequestForLoalDecoding = false;
        changeCurrentMode( STATE_LOCAL_DECODING, MAIN_DISPLAY, false);
        if(localDecodingFeature.m_timerToShowBannerMessage == 0)
        {
            localDecodingFeature.m_timerToShowBannerMessage = startTimer(10000);
        }
    }

    changeCurrentMode(STATE_SYNC_PLAYBACK_STOP_MODE, MAIN_DISPLAY);
    if(m_previousSyncBeforeOneXOne != MAX_LAYOUT)
    {
        m_doubleClickModeFlag = true;
        m_previousLayout[MAIN_DISPLAY] = m_previousSyncBeforeOneXOne;
        m_previousSyncBeforeOneXOne = MAX_LAYOUT;
    }
}

void Layout::changeCurrentMode(STATE_TYPE_FOR_OTHER_MODE_e mode, DISPLAY_TYPE_e displayType, bool isNextProcessingNeeded)
{
    if ((displayType >= MAX_DISPLAY_TYPE) || (displayType < MAIN_DISPLAY))
    {
        return;
    }

    previousModeType[displayType] = currentModeType[displayType];
    currentModeType[displayType] = mode;

    if(isNextProcessingNeeded)
    {
        doProcessingAfterChangingMode(displayType);
    }
}

void Layout::doProcessingAfterChangingMode(DISPLAY_TYPE_e displayType)
{
    if ((displayType >= MAX_DISPLAY_TYPE) || (displayType < MAIN_DISPLAY))
    {
        return;
    }

    switch(currentModeType[displayType])
    {
        case STATE_OTHER_MODE_NONE:
        {
            processNextActionForOtherMode(displayType);
        }
        break;

        case STATE_SYNC_PLAYBACK_MODE:
        {
            emit sigPreProcessingDoneForSyncPlayback();
        }
        break;

        case STATE_LOCAL_DECODING:
        {
            stopStreamsForNewStyleApply(displayType);
        }
        break;

        case STATE_APPLY_NEW_STYLE:
        case STATE_SYNC_PLAYBACK_START_MODE:
        case STATE_SYNC_PLAYBACK_STOP_MODE:
        case STATE_ANALOG_FEATURE:
        case STATE_COSEC_FEATURE:
        case STATE_EXPAND_WINDOW_START_MODE:
        case STATE_COLLAPSE_WINDOW_START_MODE:
        case STATE_INSTANT_PLAYBACK_START_MODE:
        {
            if(((previousModeType[MAIN_DISPLAY] == STATE_LOCAL_DECODING) && (currentModeType[displayType] == STATE_SYNC_PLAYBACK_STOP_MODE))
                    || ((previousModeType[MAIN_DISPLAY] == STATE_LOCAL_DECODING) && (currentModeType[displayType] == STATE_APPLY_NEW_STYLE)))
            {
                changeCurrentMode(STATE_LOCAL_DECODING, displayType, false);
            }
            stopStreamsForNewStyleApply(displayType);
        }
        break;

        case STATE_CHANGE_PAGE:
        {
            if(previousModeType[MAIN_DISPLAY] == STATE_LOCAL_DECODING)
            {
                changeCurrentMode(STATE_LOCAL_DECODING, displayType, false);
            }
            DPRINT(LAYOUT, "page sequencing: replace live stream for current page");
            replaceLiveStreamForCurrentPage(displayType);
        }
        break;

        case STATE_REFRESH_PAGE:
        {
            if(previousModeType[MAIN_DISPLAY] == STATE_LOCAL_DECODING)
            {
                changeCurrentMode(STATE_LOCAL_DECODING, displayType, false);
            }
            stopLiveStreamInAllWindows(displayType);
        }
        break;

        case STATE_RETRY_PAGE:
        {
            startStreamsForPageInRetry(displayType);
        }
        break;

        case STATE_EXPAND_WINDOW:
        {
            emit sigShowHideStyleAndPageControl(false,STATE_EXPAND_WINDOW);
            if(videoPopupData.m_videoPopupStartFlag == true)
            {
                videoPopupData.m_wasExpandMode = true;
                changeCurrentMode(STATE_OTHER_MODE_NONE,displayType);
            }
        }
        break;

        case STATE_VIDEO_POPUP_FEATURE:
        {
            if(videoPopupData.m_wasExpandMode)
            {
                emit sigShowHideStyleAndPageControl(true, STATE_EXPAND_WINDOW);
            }

            emit sigShowHideStyleAndPageControl(false, STATE_VIDEO_POPUP_FEATURE);
            if(videoPopupData.m_currentFeatureState == VIDEO_POPUP_STATE_STOP_OLD_WINDOW)
            {
                videoPopupData.m_currentFeatureState = VIDEO_POPUP_STATE_STOP_OLD_WINDOW_PROCESSING;
                stopStreamsForNewStyleApply(displayType);
            }
            else if(videoPopupData.m_currentFeatureState == VIDEO_POPUP_STATE_STOP_VIDEO)
            {
                changeCurrentMode(STATE_OTHER_MODE_PROCESSING_STOP_REQUEST, displayType, false);
                processNextActionForOtherMode(displayType);
            }
            else
            {
                startStreamForCurrentPage(displayType);
            }
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

quint16 Layout::updateCurrentPage(DISPLAY_TYPE_e displayType, quint16 currentPage, NAVIGATION_TYPE_e navigationType)
{
    quint16 newPage = currentPage;
    quint16 totalPage = 0;

    if ((displayType >= MAX_DISPLAY_TYPE) || (displayType < MAIN_DISPLAY))
    {
        return newPage;
    }

    if((currentDisplayConfig[displayType].layoutId < MAX_LAYOUT))
    {
        totalPage = (MAX_CHANNEL_FOR_SEQ / windowPerPage[currentDisplayConfig[displayType].layoutId]);
        if((MAX_CHANNEL_FOR_SEQ % windowPerPage[currentDisplayConfig[displayType].layoutId]) != 0)
        {
            totalPage++;
        }
    }

    if(totalPage > 0)
    {
        if(navigationType == PREVIOUS_PAGE_NAVIGATION)
        {
            newPage = (currentPage - 1 + totalPage) % totalPage;
        }
        else
        {
            newPage = (currentPage + 1) % totalPage;
        }
    }

    if(newPage == currentDisplayConfig[displayType].currPage)
    {
        return newPage;
    }

    if(!isAnyWindowAssignedOnPage(displayType, newPage))
    {
        return updateCurrentPage(displayType, newPage, navigationType);
    }

    return newPage;
}

bool Layout::isNextPageAvailable(DISPLAY_TYPE_e displayType, quint16 currentPage, NAVIGATION_TYPE_e navigationType)
{
    quint16 newPage = currentPage;
    quint16 totalPage = 0;

    if ((displayType >= MAX_DISPLAY_TYPE) || (displayType < MAIN_DISPLAY))
    {
        return false;
    }

    if(currentDisplayConfig[displayType].layoutId < MAX_LAYOUT)
    {
        totalPage = (MAX_CHANNEL_FOR_SEQ / windowPerPage[currentDisplayConfig[displayType].layoutId]);
        if((MAX_CHANNEL_FOR_SEQ % windowPerPage[currentDisplayConfig[displayType].layoutId]) != 0)
        {
            totalPage++;
        }
    }

    if (totalPage > 0)
    {
        if(navigationType == PREVIOUS_PAGE_NAVIGATION)
        {
            newPage = (currentPage - 1 + totalPage) % totalPage;
        }
        else
        {
            newPage = (currentPage + 1) % totalPage;
        }
    }

    if(newPage == currentDisplayConfig[displayType].currPage)
    {
        return false;
    }

    if(!isAnyWindowAssignedOnPage(displayType, newPage))
    {
        return isNextPageAvailable(displayType, newPage, navigationType);
    }

    return true;
}

bool Layout::isAnyWindowAssignedOnPage(DISPLAY_TYPE_e displayType, quint16 pageId)
{
    quint16 windowLimit[2] = {0,MAX_CHANNEL_FOR_SEQ};
    quint8 channelIndex;

    if ((displayType >= MAX_DISPLAY_TYPE) || (displayType < MAIN_DISPLAY))
    {
        return false;
    }

    getFirstAndLastWindow(pageId, currentDisplayConfig[displayType].layoutId, windowLimit);

    for(quint16 windowIndex = windowLimit[0]; windowIndex <= windowLimit[1]; windowIndex++)
    {
        if (windowIndex >= MAX_CHANNEL_FOR_SEQ)
        {
            break;
        }

        channelIndex = currentDisplayConfig[displayType].windowInfo[windowIndex].currentChannel;
        if((currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[channelIndex].deviceName[0] == '\0')
                || ((displayType == MAIN_DISPLAY) && (playbackRecordData[windowIndex].deviceName != "")))
        {
            for(quint8 index = 0; index < MAX_WIN_SEQ_CAM; index++)
            {
                if((currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[index].deviceName[0] != '\0')
                        || ((displayType == MAIN_DISPLAY) && (playbackRecordData[windowIndex].deviceName != "")))
                {
                    channelIndex = index;
                    currentDisplayConfig[displayType].windowInfo[windowIndex].currentChannel = channelIndex;
                    break;
                }
            }
        }

        if((currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[channelIndex].deviceName[0] != '\0')
                || ((displayType == MAIN_DISPLAY) && (playbackRecordData[windowIndex].deviceName != "")))
        {
            return true;
        }
    }

    return false;
}

bool Layout::isNextChannelAvailableForWindow(DISPLAY_TYPE_e displayType, quint16 windowIndex, quint8 currentChannel)
{
    if ((displayType >= MAX_DISPLAY_TYPE) || (windowIndex >= MAX_CHANNEL_FOR_SEQ))
    {
        return false;
    }

    quint8 newChannel = (currentChannel + 1) % MAX_WIN_SEQ_CAM ;
    if(newChannel == currentDisplayConfig[displayType].windowInfo[windowIndex].currentChannel)
    {
        return false;
    }

    if((currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[newChannel].deviceName[0] != '\0')
        && (currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[newChannel].defChannel != INVALID_CAMERA_INDEX))
    {
        return true;
    }

    return isNextChannelAvailableForWindow(displayType, windowIndex, newChannel);
}

quint8 Layout::updateCurrentChannel(DISPLAY_TYPE_e displayType, quint16 windowIndex, quint8 currentChannel)
{
    if ((displayType >= MAX_DISPLAY_TYPE) || (windowIndex >= MAX_CHANNEL_FOR_SEQ))
    {
        return MAX_WIN_SEQ_CAM;
    }

    quint8 newChannel = (currentChannel + 1) % MAX_WIN_SEQ_CAM;
    if(newChannel == currentDisplayConfig[displayType].windowInfo[windowIndex].currentChannel)
    {
        return newChannel;
    }

    if((currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[newChannel].deviceName[0] != '\0')
        && (currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[newChannel].defChannel != INVALID_CAMERA_INDEX))
    {
        return newChannel;
    }

    return updateCurrentChannel(displayType, windowIndex, newChannel);
}

void Layout::deleteDeviceFromCurrentStyle(DISPLAY_TYPE_e displayType, QString deviceName)
{
    if((displayType >= MAX_DISPLAY_TYPE) || (displayType < MAIN_DISPLAY))
    {
        return;
    }

    for(quint16 index = 0; index < MAX_CHANNEL_FOR_SEQ; index++)
    {
        for(quint8 channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
        {
            if(strcmp(currentDisplayConfig[displayType].windowInfo[index].camInfo[channelIndex].deviceName, deviceName.toUtf8().constData()) == 0)
            {
                currentDisplayConfig[displayType].windowInfo[index].camInfo[channelIndex].deviceName[0] = '\0';
                currentDisplayConfig[displayType].windowInfo[index].camInfo[channelIndex].defChannel = INVALID_CAMERA_INDEX;
            }
        }

        if(playbackRecordData[index].deviceName == deviceName)
        {
            playbackRecordData[index].clearPlaybackInfo();
        }
        else if(strcmp(m_instantPlaybackData[index].currentCamInfo.deviceName, deviceName.toUtf8().data()) == 0)
        {
            memset(&m_instantPlaybackData[index], 0, sizeof(INSTANT_PLAYBACK_DATA_t));
        }
    }
}

void Layout::processNextActionForOtherMode(DISPLAY_TYPE_e displayType)
{
    if ((displayType >= MAX_DISPLAY_TYPE) || (displayType < MAIN_DISPLAY))
    {
        return;
    }

    switch(currentModeType[displayType])
    {
        case STATE_OTHER_MODE_NONE:
        {
            //emit signal to display settings page
            switch(previousModeType[displayType])
            {
                case STATE_APPLY_NEW_STYLE:
                    emit sigDisplaySettingApplyChangesNotify(displayType, m_isCurrStyleSelected[displayType]);
                    m_isCurrStyleSelected[displayType] = false;
                    break;

                case STATE_COLLAPSE_WINDOW_START_MODE:
                    emit sigShowHideStyleAndPageControl(true, STATE_COLLAPSE_WINDOW_START_MODE);
                    ApplicationMode::setApplicationMode(IDLE_MODE);
                    break;

                case STATE_VIDEO_POPUP_FEATURE:
                    emit sigShowHideStyleAndPageControl(true, STATE_VIDEO_POPUP_FEATURE);
                    ApplicationMode::setApplicationMode(IDLE_MODE);
                    break;

                default:
                    break;
            }
            processQueuedAction(displayType);
        }
        break;

        case STATE_OTHER_MODE_PROCESSING_START_REQUEST:
        {
            switch(previousModeType[displayType])
            {
                case STATE_APPLY_NEW_STYLE:
                case STATE_COLLAPSE_WINDOW_START_MODE:
                    changeCurrentMode(previousModeType[displayType], displayType, false);
                    changeCurrentMode(STATE_OTHER_MODE_NONE, displayType);
                    break;

                case STATE_EXPAND_WINDOW_START_MODE:
                    changeCurrentMode(STATE_EXPAND_WINDOW, displayType);
                    break;

                case STATE_SYNC_PLAYBACK_STOP_MODE:
                case STATE_CHANGE_PAGE:
                case STATE_REFRESH_PAGE:
                case STATE_RETRY_PAGE:
                    changeCurrentMode(STATE_OTHER_MODE_NONE, displayType);
                    break;

                case STATE_ANALOG_FEATURE:
                    changeCurrentMode(STATE_ANALOG_FEATURE, displayType, false);
                    processDeviceResponseForAnalogCameraFeature();
                    break;

                case STATE_COSEC_FEATURE:
                    changeCurrentMode(STATE_COSEC_FEATURE, displayType, false);
                    processDeviceResponseForCosecPopUp();
                    break;

                case STATE_INSTANT_PLAYBACK_START_MODE:
                    changeCurrentMode(STATE_OTHER_MODE_NONE, displayType);
                    break;

                case STATE_VIDEO_POPUP_FEATURE:
                    changeCurrentMode(STATE_VIDEO_POPUP_FEATURE, displayType,false);
                    processDeviceResponseForVideoPopUp();
                    break;

                case STATE_LOCAL_DECODING:
                    changeCurrentMode(STATE_LOCAL_DECODING, displayType, false);
                    break;

                default:
                    break;
            }
        }
        break;

        case STATE_OTHER_MODE_PROCESSING_REPLACE_REQUEST:
        {
            switch(previousModeType[displayType])
            {
                case STATE_CHANGE_PAGE:
                    if(m_syncPlaybackStartFlag == true)
                    {
                        changeCurrentMode(STATE_OTHER_MODE_NONE, displayType);
                    }
                    else
                    {
                        changeCurrentMode(STATE_CHANGE_PAGE, displayType, false);
                        startStreamForCurrentPage(displayType);
                    }
                    break;

                case STATE_LOCAL_DECODING:
                    changeCurrentMode(STATE_LOCAL_DECODING, displayType, false);
                    startPlayBackForCurrentPage (displayType);
                    updateWindowDataForLocalDecording();
                    break;

                default:
                    break;
            }
        }
        break;

        case STATE_OTHER_MODE_PROCESSING_STOP_REQUEST:
        {
            switch(previousModeType[displayType])
            {
                case STATE_LOCAL_DECODING:
                    changeCurrentMode(STATE_LOCAL_DECODING, displayType,false);
                    startPlayBackForCurrentPage (displayType);
                    updateWindowDataForLocalDecording();
                    break;

                case STATE_APPLY_NEW_STYLE:
                case STATE_EXPAND_WINDOW_START_MODE:
                case STATE_COLLAPSE_WINDOW_START_MODE:
                case STATE_REFRESH_PAGE:
                case STATE_SYNC_PLAYBACK_STOP_MODE:
                    if((m_syncPlaybackStartFlag == true) || (m_nextToolbarPage == SYNC_PLAYBACK_BUTTON))
                    {
                        m_nextToolbarPage = MAX_TOOLBAR_BUTTON;
                        changeCurrentMode(STATE_OTHER_MODE_NONE, displayType);
                    }
                    else
                    {
                        changeCurrentMode(previousModeType[displayType], displayType, false);
                        startStreamForCurrentPage(displayType);

                        if((previousModeType[displayType] == STATE_APPLY_NEW_STYLE)
                                && (currentDisplayConfig[displayType].seqStatus == true) && (m_timerIdForSequencing[displayType] == 0))
                        {
                            startSequencing(displayType, true);
                        }
                    }
                    break;

                case STATE_SYNC_PLAYBACK_START_MODE:
                    changeCurrentMode(STATE_SYNC_PLAYBACK_MODE, displayType);
                    break;

                case STATE_ANALOG_FEATURE:
                    changeCurrentMode(STATE_ANALOG_FEATURE, displayType, false);
                    processDeviceResponseForAnalogCameraFeature();
                    break;

                case STATE_COSEC_FEATURE:
                    changeCurrentMode(STATE_COSEC_FEATURE, displayType, false);
                    processDeviceResponseForCosecPopUp();
                    break;

                case STATE_INSTANT_PLAYBACK_START_MODE:
                    changeCurrentMode(STATE_INSTANT_PLAYBACK_START_MODE, displayType, false);
                    break;

                case STATE_VIDEO_POPUP_FEATURE:
                    if( videoPopupData.m_currentFeatureState != VIDEO_POPUP_STATE_VIDEO_ON)
                    {
                        changeCurrentMode(STATE_VIDEO_POPUP_FEATURE, displayType,false);
                        processDeviceResponseForVideoPopUp();
                    }
                    else
                    {
                        stopVideoPopupFeature();
                    }
                    break;

                default:
                    break;
            }
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void Layout::processQueuedAction(DISPLAY_TYPE_e displayType)
{
    DEV_TABLE_INFO_t devTableInfo;
    memset(&devTableInfo, 0, sizeof(devTableInfo));
    if (true != applController->GetDeviceInfo(LOCAL_DEVICE_NAME, devTableInfo))
    {
        EPRINT(LAYOUT, "fail to get device Info");
    }

    if((devTableInfo.startLiveView == false) && (localDecodingFeature.pendingRequestForLoalDecoding == true))
    {
        for(quint8 displayType=MAIN_DISPLAY; displayType<MAX_DISPLAY_TYPE; displayType++)
        {
            startLocalDecodingFeature((DISPLAY_TYPE_e)displayType);
        }
    }
    else if((displayType == MAIN_DISPLAY) &&(videoPopupData.m_pendindingVideoPopupCount != MAX_POPUP_WINDOWS))
    {
        processQueuedActionForVideoPopup();
    }
    else if((displayType == MAIN_DISPLAY) && (m_cosecHandleLaterFlag == true))
    {
        showCosecPopupFeature(cosecPopupEvtData.m_cameraIndexForFeature, cosecPopupEvtData.m_userName,
                              cosecPopupEvtData.m_eventPopUpTime, cosecPopupEvtData.m_userId,
                              cosecPopupEvtData.m_doorName, cosecPopupEvtData.m_eventStatusCode);
    }
    else if((displayType == MAIN_DISPLAY) && (m_syncPlaybackStartFlag == true))
    {
        m_syncPlaybackStartFlag = false;
        changeCurrentMode(STATE_SYNC_PLAYBACK_START_MODE, displayType);
    }
    else if(m_changePageLaterFlag[displayType] == true)
    {
        m_changePageLaterFlag[displayType] = false;
        if((displayType == MAIN_DISPLAY) && (m_isPreviousPageNaviagtion))
        {
            changePage(displayType, PREVIOUS_PAGE_NAVIGATION);
        }
        else
        {
            changePage(displayType, NEXT_PAGE_NAVIGATION);
        }
    }
    else if(m_applyNewStyleLaterFlag[displayType] == true)
    {
        m_applyNewStyleLaterFlag[displayType] = false;
        applyNewStyle(displayType, m_nextDisplayConfig[displayType], m_nextStyleNoToBeApplied[displayType]);
        m_nextStyleNoToBeApplied[displayType] = MAX_STYLE_TYPE;
    }
    else if(m_refreshPageLaterFlag[displayType] == true)
    {
        m_refreshPageLaterFlag[displayType] = false;
        refreshCurrentPage(displayType);
    }
    else
    {
        if(!videoPopupData.m_isRetryingMode)
        {
            resumeSequencing(displayType);
        }

        if(!m_isRetryingMode)
        {
            if(currentDisplayConfig[displayType].seqStatus == false)
            {
                resumeWindowSequencing(displayType);
            }
        }
        else
        {
            m_isRetryingMode = false;
        }

        if(m_isRetryingModeinExpand)
        {
            changeCurrentMode(STATE_EXPAND_WINDOW, (DISPLAY_TYPE_e)MAIN_DISPLAY,false);
            m_isRetryingModeinExpand = false;
        }

        if(videoPopupData.m_isRetryingMode)
        {
            changeCurrentMode(STATE_VIDEO_POPUP_FEATURE, (DISPLAY_TYPE_e)MAIN_DISPLAY,false);
            videoPopupData.m_isRetryingMode = false;
        }
    }
}

void Layout::processNextActionForWindow(DISPLAY_TYPE_e displayType, quint16 windowIndex, bool windowSequencingFlag, bool instantPlaybackFlag)
{
    if ((displayType < MAIN_DISPLAY) || (displayType >= MAX_DISPLAY_TYPE) || (windowIndex >= MAX_CHANNEL_FOR_SEQ))
    {
        return;
    }

    if(windowSequencingFlag == true)
    {
        if((m_timerIdForWindowSequence[displayType][windowIndex] == 0)
                && (currentDisplayConfig[displayType].windowInfo[windowIndex].sequenceStatus == true)
                && (isMultipleChannelAssigned (displayType, windowIndex)))
        {
            m_timerIdForWindowSequence[displayType][windowIndex] = startTimer(currentDisplayConfig[displayType].windowInfo[windowIndex].sequenceInterval * 1000);
            if(m_timerIdForWindowSequence[displayType][windowIndex] <= 0)
            {
                EPRINT(LAYOUT, "fail to start window sequencing: [windowIndex=%d]", windowIndex);
            }
        }
    }
    else if(instantPlaybackFlag == true)
    {
        if(streamInfoArray[MAIN_DISPLAY][windowIndex].m_startInstantPlayback == true)
        {
            getInstantPlaybackId(windowIndex);
        }
        else if(streamInfoArray[MAIN_DISPLAY][windowIndex].m_stopInstantPlayback == true)
        {
            streamInfoArray[MAIN_DISPLAY][windowIndex].m_stopInstantPlayback = false;
            clearInstantPlaybackId(windowIndex);
        }
        else if(currentDisplayConfig[displayType].windowInfo[windowIndex].sequenceStatus != false)
        {
            if(m_timerIdForWindowSequence[displayType][windowIndex] == 0)
            {
                m_timerIdForWindowSequence[displayType][windowIndex] = startTimer(currentDisplayConfig[displayType].windowInfo[windowIndex].sequenceInterval * 1000);
            }

            if(m_timerIdForWindowSequence[displayType][windowIndex] <= 0)
            {
                EPRINT(LAYOUT, "fail to start window sequencing: [windowIndex=%d]", windowIndex);
            }
        }
    }
}

void Layout::applyNewStyle(DISPLAY_TYPE_e displayType, DISPLAY_CONFIG_t displayConfig, STYLE_TYPE_e styleNo)
{
    if ((displayType < MAIN_DISPLAY) || (displayType >= MAX_DISPLAY_TYPE))
    {
        return;
    }

    if ((currentModeType[displayType] == STATE_OTHER_MODE_NONE) && (isAnyWindowReplacing(displayType) == false))
    {
        if(styleNo == MAX_STYLE_TYPE)
        {
            m_isCurrStyleSelected[displayType] = true;
        }
        else
        {
            m_isCurrStyleSelected[displayType] = false;
            for(quint16 index = 0; index < MAX_CHANNEL_FOR_SEQ; index++)
            {
                playbackRecordData[index].clearPlaybackInfo();
            }
        }

        //stop default sequencing
        pauseSequencing(displayType);

        //stop Window sequencing
        pauseWindowSequencing(displayType);

        if(displayConfig.seqStatus == true)
        {
            stopWindowSequencing(displayType);
        }
        else
        {
            quint16 windowLimit[2] = {0,MAX_CHANNEL_FOR_SEQ};

            stopSequencing(displayType);
            getFirstAndLastWindow(displayConfig.currPage, displayConfig.layoutId, windowLimit);
            for(quint16 windowIndex = windowLimit[0]; windowIndex <= windowLimit[1]; windowIndex++)
            {
                if((displayConfig.windowInfo[windowIndex].sequenceStatus == true))
                {
                    displayConfig.windowInfo[windowIndex].lastSequenceStatus = true;
                }
            }
        }

        currentDisplayConfig[displayType].seqStatus = displayConfig.seqStatus;
        currentDisplayConfig[displayType].seqInterval = displayConfig.seqInterval;

        memcpy(&currentDisplayConfig[displayType].windowInfo, &displayConfig.windowInfo, sizeof(currentDisplayConfig[displayType].windowInfo));
        shiftWindows(displayType, displayConfig.selectedWindow, displayConfig.layoutId);
        changeCurrentMode(STATE_APPLY_NEW_STYLE, displayType);
    }
    else
    {
        if(isAnyWindowReplacing(displayType) == true)
        {
            m_processPendingForWindow[displayType] = true;
        }

        m_applyNewStyleLaterFlag[displayType] = true;
        m_nextStyleNoToBeApplied[displayType] = styleNo;
        memcpy(&m_nextDisplayConfig[displayType], &displayConfig, sizeof(DISPLAY_CONFIG_t));
    }
}

void Layout::getOSDStatus(DISPLAY_TYPE_e dispType, quint16 windowId, DEV_CAM_INFO_t &camInfo)
{
    applController->GetCameraInfoOfDevice(streamInfoArray[dispType][windowId].m_deviceName, streamInfoArray[dispType][windowId].m_cameraId, camInfo);
}

void Layout::getSingleHealthParamStatus(DISPLAY_TYPE_e displayType, quint16 windowId, HEALTH_STS_PARAM_e healthStatuParam, quint8 &status)
{
    status = 0;
    applController->GetHealtStatusSingleParamSingleCamera(streamInfoArray[displayType][windowId].m_deviceName, status,
                                                          healthStatuParam, (streamInfoArray[displayType][windowId].m_cameraId - 1));
}

void Layout::getRecordingStatus(DISPLAY_TYPE_e displayType, quint16 windowId, quint8 &recordingStatus)
{
    quint8 status = EVENT_NORMAL;

    recordingStatus = 0;
    quint8 cameraNo = (streamInfoArray[displayType][windowId].m_cameraId - 1);
    QString deviceName = streamInfoArray[displayType][windowId].m_deviceName;

    applController->GetHealtStatusSingleParamSingleCamera(deviceName, status, MANUAL_RECORDING_STS, cameraNo);
    if(status == EVENT_START)
    {
        recordingStatus |= status;
    }

    applController->GetHealtStatusSingleParamSingleCamera(deviceName, status, ALARM_RECORDING_STS, cameraNo);
    if(status == EVENT_START)
    {
        recordingStatus |= status;
    }

    applController->GetHealtStatusSingleParamSingleCamera(deviceName, status, SCHEDULE_RECORDING_STS, cameraNo);
    if(status == EVENT_START)
    {
        recordingStatus |= status;
    }

    applController->GetHealtStatusSingleParamSingleCamera(deviceName, status, COSEC_RECORDING_STS, cameraNo);
    if(status == EVENT_START)
    {
        recordingStatus |= status;
    }
}

void Layout::getFirstAndLastWindow(quint16 pageId, LAYOUT_TYPE_e layoutType, quint16 *window)
{
    if((layoutType >= 0) && (layoutType < MAX_LAYOUT))
    {
        window[0] = pageId * windowPerPage[layoutType];
        window[1] = (((pageId + 1) * windowPerPage[layoutType]) - 1);
    }

    if(window[1] >= MAX_CHANNEL_FOR_SEQ)
    {
        window[1] = (MAX_CHANNEL_FOR_SEQ - 1);
    }
}

void Layout::changeStreamStateOfWindow(DISPLAY_TYPE_e displayType, quint16 windowIndex, QString deviceName, quint8 cameraId, int streamId,
                                       VIDEO_STREAM_TYPE_e videoType, VIDEO_STATUS_TYPE_e videoStatus, VIDEO_ERROR_TYPE_e errorStatus)
{
    if ((displayType < MAIN_DISPLAY) || (displayType >= MAX_DISPLAY_TYPE) || (windowIndex >= MAX_CHANNEL_FOR_SEQ))
    {
        return;
    }

    streamInfoArray[displayType][windowIndex].m_videoType = videoType;
    streamInfoArray[displayType][windowIndex].m_deviceName = deviceName;
    streamInfoArray[displayType][windowIndex].m_cameraId = cameraId;
    streamInfoArray[displayType][windowIndex].m_streamId = streamId;
    streamInfoArray[displayType][windowIndex].m_videoStatus = videoStatus;
    streamInfoArray[displayType][windowIndex].m_errorType = errorStatus;

    if ((videoType == VIDEO_TYPE_NONE) && (!((streamInfoArray[displayType][windowIndex].m_streamRequestType == PLAYBACK_STREAM_REQUEST)
                                             && (streamInfoArray[displayType][windowIndex].m_startRequestPending == true))))
    {
        streamInfoArray[displayType][windowIndex].clearWindowInfo();
        streamInfoArray[displayType][windowIndex].timeStamp = 0;
    }

    quint16 window[2] = {0,MAX_CHANNEL_FOR_SEQ};
    getFirstAndLastWindow(currentDisplayConfig[displayType].currPage, currentDisplayConfig[displayType].layoutId, window);

    if((windowIndex >= window[0]) && (windowIndex <= window[1]))
    {
        layoutCreator->updateWindowData(windowIndex);
    }
}

CAMERA_STATE_TYPE_e Layout::changeVideoStatusToCameraStatus(VIDEO_STATUS_TYPE_e videoStatus)
{
    CAMERA_STATE_TYPE_e camStatus = CAM_STATE_NONE;
    switch(videoStatus)
    {
        case VIDEO_STATUS_NONE:
            camStatus = CAM_STATE_NONE;
            break;

        case VIDEO_STATUS_CONNECTING:
            camStatus = CAM_STATE_CONNECTING;
            break;

        case VIDEO_STATUS_RUNNING:
        case VIDEO_STATUS_VIDEOLOSS:
            camStatus = CAM_STATE_LIVE_STREAM;
            break;

        case VIDEO_STATUS_EVENTWAIT:
        case VIDEO_STATUS_RETRY:
        case VIDEO_STATUS_ERROR:
            camStatus = CAM_STATE_RETRY;
            break;
    }

    return camStatus;
}

QString Layout::getCameraNameOfDevice(QString deviceName, quint8 cameraId)
{
    return applController->GetCameraNameOfDevice(applController->GetRealDeviceName(deviceName), cameraId);
}

quint16 Layout::findWindowOfLiveStream(DISPLAY_TYPE_e displayType, QString deviceName, quint8 cameraId)
{
    for(quint16 windowIndex = 0; windowIndex < MAX_CHANNEL_FOR_SEQ; windowIndex++)
    {
        if((streamInfoArray[displayType][windowIndex].m_deviceName == deviceName) && (streamInfoArray[displayType][windowIndex].m_cameraId == cameraId)
                && (streamInfoArray[displayType][windowIndex].m_videoType == VIDEO_TYPE_LIVESTREAM))
        {
            return windowIndex;
        }
    }

    return MAX_CHANNEL_FOR_SEQ;
}

quint16 Layout::findWindowOfSyncPlaybackStream(DISPLAY_TYPE_e displayType, QString deviceName, quint8 cameraId)
{
    for(quint16 windowIndex = 0; windowIndex < MAX_CHANNEL_FOR_SEQ; windowIndex++)
    {
        if((streamInfoArray[displayType][windowIndex].m_deviceName == deviceName) && (streamInfoArray[displayType][windowIndex].m_cameraId == cameraId)
                && (streamInfoArray[displayType][windowIndex].m_videoType == VIDEO_TYPE_SYNCPLAYBAKSTREAM))
        {
            return windowIndex;
        }
    }

    return MAX_CHANNEL_FOR_SEQ;
}

quint8 Layout::getCameraId(quint16 windowIndex)
{
    if(windowIndex >= MAX_CHANNEL_FOR_SEQ)
	{
        EPRINT(LAYOUT, "invld window index: [windowIndex=%d]", windowIndex);
		return (INVALID_CAMERA_INDEX);
	}

	/* Retrun CameraId based on received WindowId */
    return (streamInfoArray[MAIN_DISPLAY][windowIndex].m_cameraId);
}

void Layout::updateWindowDataForSyncPB(quint16 windowIndex)
{
    layoutCreator->updateWindowData(windowIndex);
}

quint16 Layout::findEmptyWindowInCurrentStyle(DISPLAY_TYPE_e displayType)
{
    quint8 channelIndex;

    for(quint16 windowIndex = 0; windowIndex < MAX_CHANNEL_FOR_SEQ; windowIndex++)
    {
        channelIndex = currentDisplayConfig[displayType].windowInfo[windowIndex].currentChannel;
        if(playbackRecordData[windowIndex].deviceName != "")
        {
            continue;
        }

        for(quint8 chnIdx = 0; chnIdx < MAX_WIN_SEQ_CAM; chnIdx++)
        {
            if(currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[chnIdx].deviceName[0] != '\0')
            {
                channelIndex = chnIdx;
                break;
            }
        }

        if((currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[channelIndex].deviceName[0] == '\0')
                && (playbackRecordData[windowIndex].deviceName == ""))
        {
            return windowIndex;
        }
    }
    return MAX_CHANNEL_FOR_SEQ;
}

quint16 Layout::findEmptyWindowOnCurrentPageInCurrentStyle(DISPLAY_TYPE_e displayType)
{
    quint16 windowLimit[2] = {0,MAX_CHANNEL_FOR_SEQ};
    quint8 channelIndex;

    getFirstAndLastWindow(currentDisplayConfig[displayType].currPage, currentDisplayConfig[displayType].layoutId, windowLimit);
    for(quint16 windowIndex = windowLimit[0]; windowIndex <= windowLimit[1]; windowIndex++)
    {
        channelIndex = currentDisplayConfig[displayType].windowInfo[windowIndex].currentChannel;
        if(playbackRecordData[windowIndex].deviceName != "")
        {
            continue;
        }

        for(quint8 chnIdx = 0; chnIdx < MAX_WIN_SEQ_CAM; chnIdx++)
        {
            if(currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[chnIdx].deviceName[0] != '\0')
            {
                channelIndex = chnIdx;
                break;
            }
        }

        if((currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[channelIndex].deviceName[0] == '\0')
                && (playbackRecordData[windowIndex].deviceName == ""))
        {
            return windowIndex;
        }
    }
    return MAX_CHANNEL_FOR_SEQ;
}

quint16 Layout::findWindowIndexIfAssignOnCurrentPage(DISPLAY_TYPE_e displayType, QString deviceName, quint8 cameraId)
{
    quint16 windowLimit[2] = {0,MAX_CHANNEL_FOR_SEQ};
    quint8 channelIndex;

    getFirstAndLastWindow(currentDisplayConfig[displayType].currPage, currentDisplayConfig[displayType].layoutId, windowLimit);
    for(quint16 windowIndex = windowLimit[0]; windowIndex <= windowLimit[1]; windowIndex++)
    {
        channelIndex = currentDisplayConfig[displayType].windowInfo[windowIndex].currentChannel;
        if((cameraId == currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[channelIndex].defChannel)
                && (strcmp(deviceName.toUtf8().constData(), currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[channelIndex].deviceName) == 0))
        {
            return windowIndex;
        }
    }
    return MAX_CHANNEL_FOR_SEQ;
}

bool Layout::findWindowIndexIfAssignOnAnyPage(DISPLAY_TYPE_e displayType, QString deviceName, quint8 cameraId, quint16 &windowIndex, quint8 &channelIndex)
{
    for(windowIndex = 0; windowIndex < MAX_CHANNEL_FOR_SEQ; windowIndex++)
    {
        for(channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
        {
            if((strcmp(deviceName.toUtf8().constData(), currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[channelIndex].deviceName) == 0)
                    && (cameraId == currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[channelIndex].defChannel))
            {
                return true;
            }
        }
    }
    return false;
}

bool Layout::isMultipleChannelAssigned(DISPLAY_TYPE_e displayType, quint16 windowIndex)
{
    quint8 assignCount = 0;

    for(quint8 channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
    {
        if((currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[channelIndex].deviceName[0] != '\0')
                && (currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[channelIndex].defChannel != INVALID_CAMERA_INDEX))
        {
            assignCount++;
        }

        if (assignCount >= 2)
        {
            return true;
        }
    }
    return false;
}

bool Layout::isAnyWindowReplacing(DISPLAY_TYPE_e displayType)
{
    if ((displayType < MAIN_DISPLAY) || (displayType >= MAX_DISPLAY_TYPE))
    {
        return false;
    }

    for(quint16 windowIndex = 0; windowIndex < MAX_CHANNEL_FOR_SEQ; windowIndex++)
    {
        if (pendingInstantRequestCount[displayType][windowIndex] != 0)
        {
            return true;
        }
    }
    return false;
}

void Layout::deAllocateWindow(DISPLAY_TYPE_e displayType, quint16 windowIndex)
{
    changeStreamStateOfWindow(displayType,
                              windowIndex,
                              "",
                              INVALID_CAMERA_INDEX,
                              MAX_STREAM_SESSION,
                              VIDEO_TYPE_NONE,
                              VIDEO_STATUS_NONE,
                              VIDEO_ERROR_NONE);

    if ((displayType >= MAIN_DISPLAY) && (displayType < MAX_DISPLAY_TYPE) && (windowIndex < MAX_CHANNEL_FOR_SEQ))
    {
        emit sigWindowResponseToDisplaySettingsPage(displayType, streamInfoArray[displayType][windowIndex].m_deviceName,
                                                    streamInfoArray[displayType][windowIndex].m_cameraId, windowIndex);
    }
}

void Layout::actionWindowforDeviceStatechange(QString deviceName, DEVICE_STATE_TYPE_e devState)
{
    DPRINT(LAYOUT, "device state changed: [device=%s], [state=%d]", deviceName.toUtf8().constData(), devState);

    for(quint8 displayIndex = 0; displayIndex < deviceRespInfo.maxDisplayOutput; displayIndex++)
    {
        for(quint16 windowIndex = 0; windowIndex < MAX_CHANNEL_FOR_SEQ; windowIndex++)
        {
            if (deviceName != streamInfoArray[displayIndex][windowIndex].m_deviceName)
            {
                continue;
            }

            bool replaceFlag =  false;
            switch(streamInfoArray[displayIndex][windowIndex].m_videoType)
            {
                case VIDEO_TYPE_LIVESTREAM:
                {
                    if((devState == DISCONNECTED) || (devState == LOGGED_OUT))
                    {
                        changeStreamStateOfWindow((DISPLAY_TYPE_e)displayIndex,
                                                  windowIndex,
                                                  deviceName,
                                                  streamInfoArray[displayIndex][windowIndex].m_cameraId,
                                                  MAX_STREAM_SESSION,
                                                  VIDEO_TYPE_LIVESTREAM_AWAITING,
                                                  VIDEO_STATUS_NONE,
                                                  VIDEO_ERROR_NONE);
                    }
                    else if((devState == CONFLICT) || (devState == DELETED))
                    {
                        if(false == isNextChannelAvailableForWindow((DISPLAY_TYPE_e)displayIndex, windowIndex,
                                                                    currentDisplayConfig[displayIndex].windowInfo[windowIndex].currentChannel))
                        {
                            changeStreamStateOfWindow((DISPLAY_TYPE_e)displayIndex,
                                                      windowIndex,
                                                      "",
                                                      INVALID_CAMERA_INDEX,
                                                      MAX_STREAM_SESSION,
                                                      VIDEO_TYPE_NONE,
                                                      VIDEO_STATUS_NONE,
                                                      VIDEO_ERROR_NONE);

                            currentDisplayConfig[displayIndex].windowInfo[windowIndex].sequenceStatus = false;
                            currentDisplayConfig[displayIndex].windowInfo[windowIndex].sequenceInterval = DEFAULT_WINDOW_SEQ_INTERVAL;
                        }
                        else
                        {
                            if(false == isMultipleChannelAssigned((DISPLAY_TYPE_e)displayIndex,windowIndex))
                            {
                                currentDisplayConfig[displayIndex].windowInfo[windowIndex].sequenceStatus = false;
                                currentDisplayConfig[displayIndex].windowInfo[windowIndex].sequenceInterval = DEFAULT_WINDOW_SEQ_INTERVAL;
                            }
                            replaceFlag = true;
                        }
                    }

                    if(streamInfoArray[displayIndex][windowIndex].m_startRequestPending == true)
                    {
                        streamInfoArray[displayIndex][windowIndex].m_startRequestPending = false;
                        if(pendingInstantRequestCount[displayIndex][windowIndex] != 0)
                        {
                            m_processPendingForWindow[MAIN_DISPLAY] = false;
                            pendingInstantRequestCount[displayIndex][windowIndex]--;
                            if(pendingInstantRequestCount[displayIndex][windowIndex] == 0)
                            {
                                processNextActionForWindow((DISPLAY_TYPE_e)displayIndex, windowIndex, false, true);
                            }
                        }
                    }

                    if(streamInfoArray[displayIndex][windowIndex].m_stopRequestPending == true)
                    {
                        streamInfoArray[displayIndex][windowIndex].m_stopRequestPending = false;

                        if(pendingInstantRequestCount[displayIndex][windowIndex] != 0)
                        {
                            m_processPendingForWindow[MAIN_DISPLAY] = false;
                            pendingInstantRequestCount[displayIndex][windowIndex]--;
                            if(pendingInstantRequestCount[displayIndex][windowIndex] == 0)
                            {
                                processNextActionForWindow((DISPLAY_TYPE_e)displayIndex, windowIndex, false, true);
                            }
                        }
                    }

                    if(replaceFlag)
                    {
                        changeWindowChannel((DISPLAY_TYPE_e)displayIndex,windowIndex);
                    }
                }
                break;

                case VIDEO_TYPE_LIVESTREAM_AWAITING:
                {
                    if((devState == CONFLICT) || (devState == DELETED))
                    {
                        changeStreamStateOfWindow((DISPLAY_TYPE_e)displayIndex,
                                                  windowIndex,
                                                  "",
                                                  INVALID_CAMERA_INDEX,
                                                  MAX_STREAM_SESSION,
                                                  VIDEO_TYPE_NONE,
                                                  VIDEO_STATUS_NONE,
                                                  VIDEO_ERROR_NONE);
                    }
                }
                break;

                case VIDEO_TYPE_PLAYBACKSTREAM:
                {
                    if((devState == CONFLICT) || (devState == DELETED))
                    {
                        changeStreamStateOfWindow((DISPLAY_TYPE_e)displayIndex,
                                                  windowIndex,
                                                  "",
                                                  INVALID_CAMERA_INDEX,
                                                  MAX_STREAM_SESSION,
                                                  VIDEO_TYPE_NONE,
                                                  VIDEO_STATUS_NONE,
                                                  VIDEO_ERROR_NONE);
                    }
                    if(streamInfoArray[displayIndex][windowIndex].m_startRequestPending == true)
                    {
                        streamInfoArray[displayIndex][windowIndex].m_startRequestPending = false;
                        if(m_pendingRequestCount[displayIndex] > 0)
                        {
                            m_pendingRequestCount[displayIndex]--;
                            if(m_pendingRequestCount[displayIndex] == 0)
                            {
                                processNextActionForOtherMode((DISPLAY_TYPE_e)displayIndex);
                            }
                        }
                    }

                    if(streamInfoArray[displayIndex][windowIndex].m_stopRequestPending == true)
                    {
                        streamInfoArray[displayIndex][windowIndex].m_stopRequestPending = false;
                        if(m_pendingRequestCount[displayIndex] > 0)
                        {
                            m_pendingRequestCount[displayIndex]--;
                            if(m_pendingRequestCount[displayIndex] == 0)
                            {
                                processNextActionForOtherMode((DISPLAY_TYPE_e)displayIndex);
                            }
                        }
                    }
                }
                break;

                case VIDEO_TYPE_INSTANTPLAYBACKSTREAM:
                {
                    if((devState == CONFLICT) || (devState == DELETED))
                    {
                        changeStreamStateOfWindow((DISPLAY_TYPE_e)displayIndex,
                                                  windowIndex,
                                                  "",
                                                  INVALID_CAMERA_INDEX,
                                                  MAX_STREAM_SESSION,
                                                  VIDEO_TYPE_NONE,
                                                  VIDEO_STATUS_NONE,
                                                  VIDEO_ERROR_NONE);
                    }

                    if(streamInfoArray[displayIndex][windowIndex].m_startRequestPending == true)
                    {
                        streamInfoArray[displayIndex][windowIndex].m_startRequestPending = false;
                        if(pendingInstantRequestCount[displayIndex][windowIndex] != 0)
                        {
                            m_processPendingForWindow[MAIN_DISPLAY] = false;
                            pendingInstantRequestCount[displayIndex][windowIndex]--;
                            if(pendingInstantRequestCount[displayIndex][windowIndex] == 0)
                            {
                                processNextActionForWindow((DISPLAY_TYPE_e)displayIndex, windowIndex, false, true);
                            }
                        }
                    }

                    if(streamInfoArray[displayIndex][windowIndex].m_stopRequestPending == true)
                    {
                        streamInfoArray[displayIndex][windowIndex].m_stopRequestPending = false;
                        if(pendingInstantRequestCount[displayIndex][windowIndex] != 0)
                        {
                            m_processPendingForWindow[MAIN_DISPLAY] = false;
                            pendingInstantRequestCount[displayIndex][windowIndex]--;
                            if(pendingInstantRequestCount[displayIndex][windowIndex] == 0)
                            {
                                processNextActionForWindow((DISPLAY_TYPE_e)displayIndex, windowIndex, false, true);
                            }
                        }
                    }
                }
                break;

                default:
                {
                    /* Nothing to do */
                }
                break;
            }
        }

        if((devState == CONFLICT) || (devState == DELETED))
        {
            deleteDeviceFromCurrentStyle((DISPLAY_TYPE_e)displayIndex, deviceName);
        }
    }
}

quint8 Layout::startLiveStream(DISPLAY_TYPE_e displayIndex, QString devName, quint8 cameraId, quint16 windowIndex)
{
    quint8 totalCam = 0, channelIndex, pendingStreamRequest = 0;
    quint8 streamType = MAX_LIVE_STREAM_TYPE;
    CAMERA_TYPE_e cameraType = MAX_CAMERA_TYPE;
    STREAM_REQUEST_TYPE_e streamRequestType = MAX_STREAM_REQUEST_TYPE;
    SERVER_SESSION_INFO_t serverSessionInfo;
    StreamRequestParam *streamRequestParam = NULL;
    qint64 timeStamp = 0;

    if(streamInfoArray[displayIndex][windowIndex].m_startRequestPending == true)
    {
        WPRINT(LAYOUT, "window is currently busy: [displayIndex=%d], [windowIndex=%d], [streamType=%d], [cameraId=%d]",
               displayIndex, windowIndex, streamType, cameraId);
        freeWindow(displayIndex, windowIndex);
    }

    if(devName == LOCAL_DEVICE_NAME)
    {
        cameraType = applController->GetCameraType(LOCAL_DEVICE_NAME, (cameraId - 1));
        if(cameraType == ANALOG_CAMERA)
        {
            changeStreamStateOfWindow(displayIndex,
                                      windowIndex,
                                      devName,
                                      cameraId,
                                      MAX_STREAM_SESSION,
                                      VIDEO_TYPE_LIVESTREAM,
                                      VIDEO_STATUS_EVENTWAIT,
                                      VIDEO_ERROR_DISABLECAMERA);
            streamInfoArray[displayIndex][windowIndex].m_windowId = (windowIndex % windowPerPage[currentDisplayConfig[displayIndex].layoutId]);
            return pendingStreamRequest;
        }
    }

    if (false == applController->GetTotalCamera(devName, totalCam))
    {
        //device is not enabled so make it live stream awaiting
        changeStreamStateOfWindow(displayIndex,
                                  windowIndex,
                                  devName,
                                  cameraId,
                                  MAX_STREAM_SESSION,
                                  VIDEO_TYPE_LIVESTREAM_AWAITING,
                                  VIDEO_STATUS_NONE,
                                  VIDEO_ERROR_NONE);
        return pendingStreamRequest;
    }

    if (cameraId > totalCam)
    {
        changeStreamStateOfWindow(displayIndex,
                                  windowIndex,
                                  "",
                                  INVALID_CAMERA_INDEX,
                                  MAX_STREAM_SESSION,
                                  VIDEO_TYPE_NONE,
                                  VIDEO_STATUS_NONE,
                                  VIDEO_ERROR_NONE);
        channelIndex = currentDisplayConfig[displayIndex].windowInfo[windowIndex].currentChannel;
        currentDisplayConfig[displayIndex].windowInfo[windowIndex].camInfo[channelIndex].deviceName[0] = '\0';
        currentDisplayConfig[displayIndex].windowInfo[windowIndex].camInfo[channelIndex].defChannel = INVALID_CAMERA_INDEX;
        return pendingStreamRequest;
    }

    bool streamStartFlag = false;
    quint16 window = findWindowOfLiveStream(displayIndex, devName, cameraId);
    if (window == MAX_CHANNEL_FOR_SEQ)
    {
        streamStartFlag = true;
        window = windowIndex;
    }
    else
    {
        if((streamInfoArray[displayIndex][window].m_videoStatus == VIDEO_STATUS_EVENTWAIT)
                || (streamInfoArray[displayIndex][window].m_videoType == VIDEO_TYPE_LIVESTREAM_AWAITING))
        {
            DEV_CAM_INFO_t cameraInfo;
            if(applController->GetCameraInfoOfDevice(devName, cameraId, cameraInfo))
            {
                if (false == cameraInfo.camStatus)
                {
                    return pendingStreamRequest;
                }

                streamStartFlag = true;
            }
        }
        else if(streamInfoArray[displayIndex][window].m_videoStatus == VIDEO_STATUS_RETRY)
        {
            streamStartFlag = true;
        }
    }

    if (false == streamStartFlag)
    {
        return pendingStreamRequest;
    }

    if(false == applController->getLiveStreamTypeFrmDev(devName, streamType))
    {
        changeStreamStateOfWindow(displayIndex,
                                  windowIndex,
                                  devName,
                                  cameraId,
                                  MAX_STREAM_SESSION,
                                  VIDEO_TYPE_LIVESTREAM_AWAITING,
                                  VIDEO_STATUS_NONE,
                                  VIDEO_ERROR_NONE);
        return pendingStreamRequest;
    }

    if(streamType == MAX_LIVE_STREAM_TYPE)
    {
        if((currentDisplayConfig[displayIndex].layoutId != ONE_X_ONE) && (currentDisplayConfig[displayIndex].layoutId < ONE_X_ONE_PLAYBACK))
        {
            streamType = LIVE_STREAM_TYPE_SUB;
        }
        else
        {
            streamType = LIVE_STREAM_TYPE_MAIN;
        }
    }

    if (streamInfoArray[displayIndex][window].m_errorType == VIDEO_ERROR_NO_DECODING_CAP)
    {
        if (applController->readBandwidthOptFlag() == true)
        {
            streamType = LIVE_STREAM_TYPE_SUB;
        }
        else
        {
            streamType = (streamInfoArray[displayIndex][window].m_streamType == LIVE_STREAM_TYPE_SUB) ? LIVE_STREAM_TYPE_MAIN : LIVE_STREAM_TYPE_SUB;
            DPRINT(LAYOUT, "changing live view stream type due to decoder capacity: [cameraId=%d], [newStreamType=%s]", cameraId,
                   (streamType == LIVE_STREAM_TYPE_MAIN) ? "MAIN" : "SUB");
        }
    }

    if(false == applController->GetServerSessionInfo(devName, serverSessionInfo))
    {
        changeStreamStateOfWindow(displayIndex,
                                  windowIndex,
                                  devName,
                                  cameraId,
                                  MAX_STREAM_SESSION,
                                  VIDEO_TYPE_LIVESTREAM_AWAITING,
                                  VIDEO_STATUS_NONE,
                                  VIDEO_ERROR_NONE);
        return pendingStreamRequest;
    }

    /* Update live stream video loss timeout duration */
    applController->GetPreVideoLossDuration(devName, serverSessionInfo.sessionInfo.timeout);
    if(devName == LOCAL_DEVICE_NAME)
    {
        cameraType = applController->GetCameraType(devName, (cameraId - 1));
        streamRequestType = (cameraType == ANALOG_CAMERA) ? LIVE_ANALOG_REQUEST : LIVE_STREAM_REQUEST;
    }
    else
    {
        streamRequestType = LIVE_STREAM_REQUEST;
    }

    timeStamp = QDateTime::currentMSecsSinceEpoch() + cameraId + windowIndex;
    payloadLib->setCnfgArrayAtIndex(0, cameraId);
    payloadLib->setCnfgArrayAtIndex(1, streamType);

    streamRequestParam = new StreamRequestParam();
    streamRequestParam->windowId = (windowIndex % windowPerPage[currentDisplayConfig[displayIndex].layoutId]);
    streamRequestParam->actualWindowId = windowIndex;
    streamRequestParam->channelId = cameraId;
    streamRequestParam->deviceName = devName;
    streamRequestParam->displayType = displayIndex;
    streamRequestParam->streamRequestType = streamRequestType;
    streamRequestParam->liveStreamType = (LIVE_STREAM_TYPE_e)streamType;
    streamRequestParam->timeStamp = timeStamp;
    streamRequestParam->payload = payloadLib->createDevCmdPayload(2);
    streamRequestParam->streamId =  streamInfoArray[displayIndex][windowIndex].m_streamId;

    pendingStreamRequest = applController->processStreamActivity(START_STREAM_COMMAND,
                                                                 serverSessionInfo,
                                                                 streamRequestParam,
                                                                 &streamInfoArray[displayIndex][windowIndex].m_streamId);
    if(pendingStreamRequest == 0)
    {
        EPRINT(LAYOUT, "fail to send start stream: [camera=%d]", streamRequestParam->channelId);
        DELETE_OBJ(streamRequestParam);

        changeStreamStateOfWindow(displayIndex,
                                  windowIndex,
                                  devName,
                                  cameraId,
                                  MAX_STREAM_SESSION,
                                  VIDEO_TYPE_LIVESTREAM,
                                  VIDEO_STATUS_RETRY,
                                  VIDEO_ERROR_OTHERERROR);
        streamInfoArray[displayIndex][windowIndex].timeStamp = 0;
    }
    else
    {
        streamInfoArray[displayIndex][windowIndex].m_deviceName = devName;
        streamInfoArray[displayIndex][windowIndex].m_cameraId = cameraId;
        streamInfoArray[displayIndex][windowIndex].m_videoType = VIDEO_TYPE_LIVESTREAM;
        streamInfoArray[displayIndex][windowIndex].m_errorType = VIDEO_ERROR_NONE;
        streamInfoArray[displayIndex][windowIndex].m_streamType = (LIVE_STREAM_TYPE_e)streamType;
        streamInfoArray[displayIndex][windowIndex].m_windowId = (windowIndex % windowPerPage[currentDisplayConfig[displayIndex].layoutId]);
        streamInfoArray[displayIndex][windowIndex].m_startRequestPending = true;
        streamInfoArray[displayIndex][windowIndex].timeStamp = timeStamp;

        changeStreamStateOfWindow(displayIndex,
                                  windowIndex,
                                  devName,
                                  cameraId,
                                  streamInfoArray[displayIndex][windowIndex].m_streamId,
                                  VIDEO_TYPE_LIVESTREAM,
                                  VIDEO_STATUS_CONNECTING,
                                  VIDEO_ERROR_NONE);
    }

    return pendingStreamRequest;
}

quint8 Layout::startLiveStream(DISPLAY_TYPE_e displayIndex, QString devName, quint8 cameraId, quint16 windowIndex, LIVE_STREAM_TYPE_e streamType)
{
    quint8 totalCam = 0, pendingStreamRequest = 0;
    CAMERA_TYPE_e cameraType = MAX_CAMERA_TYPE;
    STREAM_REQUEST_TYPE_e streamRequestType = MAX_STREAM_REQUEST_TYPE;
    SERVER_SESSION_INFO_t serverSessionInfo;
    quint8 channelIndex = currentDisplayConfig[displayIndex].windowInfo[windowIndex].currentChannel;
    StreamRequestParam *streamRequestParam = NULL;
    qint64 timeStamp = 0;

    if(streamInfoArray[displayIndex][windowIndex].m_startRequestPending == true)
    {
        WPRINT(LAYOUT, "window is currently busy: [displayIndex=%d], [windowIndex=%d], [streamType=%d], [cameraId=%d]",
               displayIndex, windowIndex, streamType, cameraId);
        freeWindow(displayIndex, windowIndex);
    }

    if(devName == LOCAL_DEVICE_NAME)
    {
        cameraType = applController->GetCameraType(LOCAL_DEVICE_NAME, (cameraId - 1));
        if(cameraType == ANALOG_CAMERA)
        {
            DPRINT(LAYOUT,"Disable Camera status of devName [%s] cameraId [%d] streamType [%d]", devName.toUtf8().constData(), cameraId, streamType);
            changeStreamStateOfWindow(displayIndex,
                                      windowIndex,
                                      devName,
                                      cameraId,
                                      MAX_STREAM_SESSION,
                                      VIDEO_TYPE_LIVESTREAM,
                                      VIDEO_STATUS_EVENTWAIT,
                                      VIDEO_ERROR_DISABLECAMERA);
            streamInfoArray[displayIndex][windowIndex].m_windowId = (windowIndex % windowPerPage[currentDisplayConfig[displayIndex].layoutId]);
            return pendingStreamRequest;
        }
    }

    //if cameraNumber is invalid
    if (false == applController->GetTotalCamera(devName, totalCam))
    {
        //device is not enabled so make it live stream awaiting
        changeStreamStateOfWindow(displayIndex,
                                  windowIndex,
                                  devName,
                                  cameraId,
                                  MAX_STREAM_SESSION,
                                  VIDEO_TYPE_LIVESTREAM_AWAITING,
                                  VIDEO_STATUS_NONE,
                                  VIDEO_ERROR_NONE);
        return pendingStreamRequest;
    }

    if(cameraId > totalCam)
    {
        changeStreamStateOfWindow(displayIndex,
                                  windowIndex,
                                  "",
                                  INVALID_CAMERA_INDEX,
                                  MAX_STREAM_SESSION,
                                  VIDEO_TYPE_NONE,
                                  VIDEO_STATUS_NONE,
                                  VIDEO_ERROR_NONE);
        currentDisplayConfig[displayIndex].windowInfo[windowIndex].camInfo[channelIndex].deviceName[0] = '\0';
        currentDisplayConfig[displayIndex].windowInfo[windowIndex].camInfo[channelIndex].defChannel = INVALID_CAMERA_INDEX;
        return pendingStreamRequest;
    }

    bool streamStartFlag = false;
    quint16 window = findWindowOfLiveStream(displayIndex, devName, cameraId);
    if(window == MAX_CHANNEL_FOR_SEQ)
    {
        streamStartFlag = true;
    }
    else
    {
        if((streamInfoArray[displayIndex][window].m_videoStatus == VIDEO_STATUS_RETRY)
                || (streamInfoArray[displayIndex][window].m_videoStatus == VIDEO_STATUS_EVENTWAIT)
                || (streamInfoArray[displayIndex][window].m_videoType == VIDEO_TYPE_LIVESTREAM_AWAITING))
        {
            DEV_CAM_INFO_t cameraInfo;
            if(applController->GetCameraInfoOfDevice(devName, cameraId, cameraInfo))
            {
                if (false == cameraInfo.camStatus)
                {
                    return pendingStreamRequest;
                }

                streamStartFlag = true;
            }
        }
    }


    if(false == streamStartFlag)
    {
        return pendingStreamRequest;
    }

    if(false == applController->GetServerSessionInfo(devName, serverSessionInfo))
    {
        EPRINT(LAYOUT, "fail to get server session info");
        changeStreamStateOfWindow(displayIndex,
                                  windowIndex,
                                  devName,
                                  cameraId,
                                  MAX_STREAM_SESSION,
                                  VIDEO_TYPE_LIVESTREAM_AWAITING,
                                  VIDEO_STATUS_NONE,
                                  VIDEO_ERROR_NONE);
        return pendingStreamRequest;
    }

    /* Update live stream video loss timeout duration */
    applController->GetPreVideoLossDuration(devName, serverSessionInfo.sessionInfo.timeout);
    if(devName == LOCAL_DEVICE_NAME)
    {
        cameraType = applController->GetCameraType(devName, (cameraId - 1));
        streamRequestType = (cameraType == ANALOG_CAMERA) ? LIVE_ANALOG_REQUEST : LIVE_STREAM_REQUEST;
    }
    else
    {
        streamRequestType = LIVE_STREAM_REQUEST;
    }

    timeStamp = QDateTime::currentMSecsSinceEpoch() + cameraId + windowIndex;
    payloadLib->setCnfgArrayAtIndex(0, cameraId);
    payloadLib->setCnfgArrayAtIndex(1, streamType);

    streamRequestParam = new StreamRequestParam();
    streamRequestParam->windowId = (windowIndex % windowPerPage[currentDisplayConfig[displayIndex].layoutId]);
    streamRequestParam->actualWindowId = windowIndex;
    streamRequestParam->channelId = cameraId;
    streamRequestParam->deviceName = devName;
    streamRequestParam->displayType = displayIndex;
    streamRequestParam->streamRequestType = streamRequestType;
    streamRequestParam->liveStreamType = streamType;
    streamRequestParam->timeStamp = timeStamp; // current local timestamp
    streamRequestParam->payload = payloadLib->createDevCmdPayload(2);

    // send Start Stream Command to particular server with provided stream request parameters
    pendingStreamRequest = applController->processStreamActivity(START_STREAM_COMMAND,
                                                                 serverSessionInfo,
                                                                 streamRequestParam);
    if(pendingStreamRequest == 0)
    {
        EPRINT(LAYOUT, "fail to send start stream: [camera=%d]", streamRequestParam->channelId);
        DELETE_OBJ(streamRequestParam);

        changeStreamStateOfWindow(displayIndex,
                                  windowIndex,
                                  devName,
                                  cameraId,
                                  MAX_STREAM_SESSION,
                                  VIDEO_TYPE_LIVESTREAM,
                                  VIDEO_STATUS_RETRY,
                                  VIDEO_ERROR_NONE);
        streamInfoArray[displayIndex][windowIndex].timeStamp = 0;
    }
    else
    {

        streamInfoArray[displayIndex][windowIndex].m_deviceName = devName;
        streamInfoArray[displayIndex][windowIndex].m_cameraId = cameraId;
        streamInfoArray[displayIndex][windowIndex].m_videoType = VIDEO_TYPE_LIVESTREAM;
        streamInfoArray[displayIndex][windowIndex].m_errorType = VIDEO_ERROR_NONE;;
        streamInfoArray[displayIndex][windowIndex].m_streamType = streamType;
        streamInfoArray[displayIndex][windowIndex].m_windowId = (windowIndex % windowPerPage[currentDisplayConfig[displayIndex].layoutId]);
        streamInfoArray[displayIndex][windowIndex].m_startRequestPending = true;
        streamInfoArray[displayIndex][windowIndex].timeStamp = timeStamp;
    }

    return pendingStreamRequest;
}

quint8 Layout::stopLiveStream(DISPLAY_TYPE_e displayType, quint16 windowIndex)
{
    quint8 pendingStreamRequest = 0;
    SERVER_SESSION_INFO_t serverSessionInfo;
    StreamRequestParam *streamRequestParam = NULL;
    QString m_getUserNameforAudio;

    applController->GetServerSessionInfo(streamInfoArray[displayType][windowIndex].m_deviceName, serverSessionInfo);

    payloadLib->setCnfgArrayAtIndex(0, streamInfoArray[displayType][windowIndex].m_cameraId);
    payloadLib->setCnfgArrayAtIndex(1, streamInfoArray[displayType][windowIndex].m_streamType);

    streamRequestParam = new StreamRequestParam();
    streamRequestParam->streamId = streamInfoArray[displayType][windowIndex].m_streamId;
    streamRequestParam->windowId = streamInfoArray[displayType][windowIndex].m_windowId;
    streamRequestParam->actualWindowId = windowIndex;
    streamRequestParam->channelId = streamInfoArray[displayType][windowIndex].m_cameraId;
    streamRequestParam->deviceName = streamInfoArray[displayType][windowIndex].m_deviceName;
    streamRequestParam->displayType = displayType;
    streamRequestParam->payload = payloadLib->createDevCmdPayload(2);

    // send STOP_STREAM request to the server
    pendingStreamRequest = applController->processStreamActivity(STOP_STREAM_COMMAND,
                                                                 serverSessionInfo,
                                                                 streamRequestParam);
    if (pendingStreamRequest == 0)
    {
        EPRINT(LAYOUT, "fail to send stop stream: [camera=%d]", streamRequestParam->channelId);
        DELETE_OBJ(streamRequestParam);
    }
    else
    {
        streamInfoArray[displayType][windowIndex].m_startRequestPending = false;
        streamInfoArray[displayType][windowIndex].timeStamp = 0;
    }

    if(streamInfoArray[MAIN_DISPLAY][windowIndex].m_audioStatus == true)
    {
        m_AudioEnableDevice = streamInfoArray[displayType][windowIndex].m_deviceName;
        m_AudioEnableCamId = streamInfoArray[displayType][windowIndex].m_cameraId;
        lastEnableAudioWindow = windowIndex;

        //If directly closed window and Audio start on that window then remove Audio
        if(m_audioStopForWindow == windowIndex)
        {
            lastEnableAudioWindow = MAX_CHANNEL_FOR_SEQ;
        }
    }

	// Microphone On then stop Audio-In on LiveView Stop event
	// No Need to send command to stop Audio-In separately. It will taken care by bottem layer
    m_audioStopForWindow = MAX_CHANNEL_FOR_SEQ;
	if(windowIndex == m_activeMicroPhoneWindow)
	{
		streamInfoArray[MAIN_DISPLAY][windowIndex].m_microPhoneStatus = false;
		m_activeMicroPhoneWindow = MAX_CHANNEL_FOR_SEQ;
	}

    // ON Log out page stop the Audio if already ON state
    applController->getUsernameFrmDev(LOCAL_DEVICE_NAME, m_getUserNameforAudio);
    if(m_getUserNameforAudio == DEFAULT_LOGIN_USER)
    {
        lastEnableAudioWindow = MAX_CHANNEL_FOR_SEQ;
    }

    if(startAudioInWindow == windowIndex)
    {
        streamInfoArray[MAIN_DISPLAY][windowIndex].m_audioStatus = false;
        startAudioInWindow = MAX_CHANNEL_FOR_SEQ;
    }

    if(streamInfoArray[displayType][windowIndex].m_replacingChannel == false)
    {
        changeStreamStateOfWindow(displayType,
                                  windowIndex,
                                  "",
                                  INVALID_CAMERA_INDEX,
                                  MAX_STREAM_SESSION,
                                  VIDEO_TYPE_NONE,
                                  VIDEO_STATUS_NONE,
                                  VIDEO_ERROR_NONE);
    }
    else
    {
        streamInfoArray[displayType][windowIndex].m_replacingChannel = false;
        streamInfoArray[displayType][windowIndex].m_audioStatus = false;

        quint16 window[2] = {0, MAX_CHANNEL_FOR_SEQ};
        getFirstAndLastWindow(currentDisplayConfig[displayType].currPage, currentDisplayConfig[displayType].layoutId, window);
        if((windowIndex >= window[0]) && (windowIndex <= window[1]))
        {
            if(displayType == MAIN_DISPLAY)
            {
                layoutCreator->updateWindowData(windowIndex);
            }
        }
    }

    emit sigWindowResponseToDisplaySettingsPage(displayType, streamInfoArray[displayType][windowIndex].m_deviceName,
                                                streamInfoArray[displayType][windowIndex].m_cameraId, windowIndex);

    if((pendingStreamRequest > 0) && (streamInfoArray[MAIN_DISPLAY][windowIndex].m_startInstantPlayback == true))
    {
        processNextActionForWindow(MAIN_DISPLAY, windowIndex, false, true);
    }

    return 0;
}

// This Function is invoked when replacing stream on particular window.
quint8 Layout::replaceLiveStream(DISPLAY_TYPE_e displayType, quint16 windowIndex, QString deviceName, quint8 cameraId, quint16 newWindowIndex)
{
    quint8 pendingStreamRequest = 0, processRequest = 0;
    CAMERA_TYPE_e cameraType = MAX_CAMERA_TYPE;
    STREAM_REQUEST_TYPE_e streamRequestType = MAX_STREAM_REQUEST_TYPE;
    quint8 streamType = MAX_LIVE_STREAM_TYPE;
    SERVER_SESSION_INFO_t serverSessionInfo, nextServerSessionInfo;
    StreamRequestParam *streamRequestParam = NULL;
    StreamRequestParam *nextStreamRequestParam = NULL;

    if(true == applController->getLiveStreamTypeFrmDev(deviceName,streamType))
    {
        if(streamType == MAX_LIVE_STREAM_TYPE)
        {
            if((currentDisplayConfig[displayType].layoutId != ONE_X_ONE) && (currentDisplayConfig[displayType].layoutId < ONE_X_ONE_PLAYBACK))
            {
                streamType = LIVE_STREAM_TYPE_SUB;
            }
            else
            {
                streamType = LIVE_STREAM_TYPE_MAIN;
            }
        }
    }
    else
    {
        streamType = streamInfoArray[displayType][windowIndex].m_streamType;
    }

    /* Get server info and update live stream video loss timeout duration */
    applController->GetServerSessionInfo(streamInfoArray[displayType][windowIndex].m_deviceName, serverSessionInfo);
    applController->GetPreVideoLossDuration(streamInfoArray[displayType][windowIndex].m_deviceName, serverSessionInfo.sessionInfo.timeout);

    /* Get server info and update live stream video loss timeout duration */
    applController->GetServerSessionInfo(deviceName, nextServerSessionInfo);
    applController->GetPreVideoLossDuration(deviceName, nextServerSessionInfo.sessionInfo.timeout);

    payloadLib->setCnfgArrayAtIndex(0, streamInfoArray[displayType][windowIndex].m_cameraId);
    payloadLib->setCnfgArrayAtIndex(1, streamInfoArray[displayType][windowIndex].m_streamType);

    streamRequestParam = new StreamRequestParam();
    streamRequestParam->streamId = streamInfoArray[displayType][windowIndex].m_streamId;
    streamRequestParam->windowId = streamInfoArray[displayType][windowIndex].m_windowId;
    streamRequestParam->actualWindowId = windowIndex;
    streamRequestParam->channelId = streamInfoArray[displayType][windowIndex].m_cameraId;
    streamRequestParam->deviceName = streamInfoArray[displayType][windowIndex].m_deviceName;
    streamRequestParam->displayType = displayType;
    streamRequestParam->payload = payloadLib->createDevCmdPayload(2);

    if(deviceName == LOCAL_DEVICE_NAME)
    {
        cameraType = applController->GetCameraType(deviceName, (cameraId - 1));
        streamRequestType = (cameraType == ANALOG_CAMERA) ? LIVE_ANALOG_REQUEST : LIVE_STREAM_REQUEST;
    }
    else
    {
        streamRequestType = LIVE_STREAM_REQUEST;
    }

    payloadLib->setCnfgArrayAtIndex(0, cameraId);
    payloadLib->setCnfgArrayAtIndex(1, streamType);

    nextStreamRequestParam = new StreamRequestParam();
    nextStreamRequestParam->windowId = (newWindowIndex % windowPerPage[currentDisplayConfig[displayType].layoutId]);
    nextStreamRequestParam->actualWindowId = newWindowIndex;
    nextStreamRequestParam->channelId = cameraId;
    nextStreamRequestParam->deviceName = deviceName;
    nextStreamRequestParam->displayType = displayType;
    nextStreamRequestParam->streamRequestType = streamRequestType;
    nextStreamRequestParam->liveStreamType = (LIVE_STREAM_TYPE_e)streamType;
    nextStreamRequestParam->payload = payloadLib->createDevCmdPayload(2);

    processRequest = applController->processStreamActivity(REPLACE_STREAM_COMMAND,
                                                           serverSessionInfo,
                                                           streamRequestParam,
                                                           nextServerSessionInfo,
                                                           nextStreamRequestParam);
    DELETE_OBJ(streamRequestParam);
    DELETE_OBJ(nextStreamRequestParam);

    switch(processRequest)
    {
        case NO_STOP_NO_START:
        {
            changeStreamStateOfWindow(displayType,
                                      windowIndex,
                                      "",
                                      INVALID_CAMERA_INDEX,
                                      MAX_STREAM_SESSION,
                                      VIDEO_TYPE_NONE,
                                      VIDEO_STATUS_NONE,
                                      VIDEO_ERROR_NONE);

            changeStreamStateOfWindow(displayType,
                                      newWindowIndex,
                                      deviceName,
                                      cameraId,
                                      MAX_STREAM_SESSION,
                                      VIDEO_TYPE_LIVESTREAM,
                                      VIDEO_STATUS_RETRY,
                                      VIDEO_ERROR_OTHERERROR);
        }
        break;

        case ONLY_SECOND_START:
        {
            pendingStreamRequest = startLiveStream(displayType, deviceName, cameraId, newWindowIndex);
            DPRINT(LAYOUT, "page sequence: replace stream: ONLY_SECOND_START");
        }
        break;

        case ONLY_FIRST_STOP:
        {
            if(newWindowIndex == windowIndex)
            {
                streamInfoArray[displayType][windowIndex].m_replacingChannel = true;
            }

            pendingStreamRequest = stopLiveStream(displayType, windowIndex);
            DPRINT(LAYOUT, "page sequence: replace stream: ONLY_FIRST_STOP");
        }
        break;

        case FIRST_STOP_SECOND_START:
        case SECOND_REPLACE_FIRST:
        {
            if(processRequest == FIRST_STOP_SECOND_START)
            {
                DPRINT(LAYOUT, "page sequence: replace stream: FIRST_STOP_SECOND_START");
            }
            else
            {
                DPRINT(LAYOUT, "page sequence: replace stream: SECOND_REPLACE_FIRST");
            }

            if(newWindowIndex == windowIndex)
            {
                streamInfoArray[displayType][windowIndex].m_replacingChannel = true;
            }

            stopLiveStream(displayType, windowIndex);
            pendingStreamRequest = startLiveStream(displayType, deviceName, cameraId, newWindowIndex);
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }

    if(pendingStreamRequest == 0)
    {
        DPRINT(LAYOUT, "fail to replace stream: [windowIndex=%d], [newWindowIndex=%d]", windowIndex, newWindowIndex);
    }

    return pendingStreamRequest;
}

quint8 Layout::freeWindow(DISPLAY_TYPE_e displayType, quint16 windowIndex, bool isManuallyStop)
{
    quint8 pendingStreamRequest = 0;
    if ((displayType < MAIN_DISPLAY) || (displayType >= MAX_DISPLAY_TYPE) || (windowIndex >= MAX_CHANNEL_FOR_SEQ))
    {
        return pendingStreamRequest;
    }

    switch(streamInfoArray[displayType][windowIndex].m_videoType)
    {
        case VIDEO_TYPE_LIVESTREAM:
        {
            switch(streamInfoArray[displayType][windowIndex].m_videoStatus)
            {
                case VIDEO_STATUS_CONNECTING:
                case VIDEO_STATUS_RUNNING:
                case VIDEO_STATUS_VIDEOLOSS:
                    pendingStreamRequest = stopLiveStream(displayType, windowIndex);
                    break;

                case VIDEO_STATUS_RETRY:
                case VIDEO_STATUS_EVENTWAIT:
                case VIDEO_STATUS_ERROR:
                    DPRINT(LAYOUT, "[camera=%d], [videoType=%d], [videoStatus=%d]", streamInfoArray[displayType][windowIndex].m_cameraId,
                           streamInfoArray[displayType][windowIndex].m_videoType, streamInfoArray[displayType][windowIndex].m_videoStatus);
                    deAllocateWindow(displayType, windowIndex);
                    break;

                default:
                    break;
            }

            if(isManuallyStop)
            {
                for(quint8 channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
                {
                    currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[channelIndex].deviceName[0] = '\0';
                    currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[channelIndex].defChannel = INVALID_CAMERA_INDEX;
                }
            }
        }
        break;

        case VIDEO_TYPE_PLAYBACKSTREAM:
        {
            closeDeletePbToolBar();
            pendingStreamRequest = stopPlaybackStream(windowIndex);
            if(pendingStreamRequest == 0)
            {
                deAllocateWindow(displayType, windowIndex);
            }

            if(isManuallyStop)
            {
                playbackRecordData[windowIndex].clearPlaybackInfo();
            }
        }
        break;

        case VIDEO_TYPE_INSTANTPLAYBACKSTREAM:
        {
            closeDeleteInstantPbToolBar();
            pendingStreamRequest = stopInstantPlayBackStream(windowIndex);
            if(pendingStreamRequest == 0)
            {
                deAllocateWindow(displayType, windowIndex);
            }

            if(isManuallyStop)
            {
                memset(&m_instantPlaybackData[windowIndex], 0, sizeof(INSTANT_PLAYBACK_DATA_t));
            }
        }
        break;

        case VIDEO_TYPE_LIVESTREAM_AWAITING:
        {
            deAllocateWindow(displayType, windowIndex);
            if((displayType < MAX_DISPLAY_TYPE) && (windowIndex < MAX_CHANNEL_FOR_SEQ) && (isManuallyStop))
            {
                for(quint8 channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
                {
                    currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[channelIndex].deviceName[0] = '\0';
                    currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[channelIndex].defChannel = INVALID_CAMERA_INDEX;
                }
            }
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }

    return pendingStreamRequest;
}

void Layout::replaceLiveStreamForCurrentPage(DISPLAY_TYPE_e displayType)
{
    if ((displayType >= MAX_DISPLAY_TYPE) || (displayType < MAIN_DISPLAY))
    {
        return;
    }

    if (currentDisplayConfig[displayType].layoutId < MAX_LAYOUT)
    {
        changeCurrentMode(STATE_OTHER_MODE_PROCESSING_REPLACE_REQUEST, displayType, false);

        NAVIGATION_TYPE_e navigationType = NEXT_PAGE_NAVIGATION;
        quint8 pendingStreamRequest = 0;
        quint16 previousPage = currentDisplayConfig[displayType].currPage;
        quint16 totalPage = (MAX_CHANNEL_FOR_SEQ / windowPerPage[currentDisplayConfig[displayType].layoutId]);

        m_pendingRequestCount[displayType] = 0;
        if((MAX_CHANNEL_FOR_SEQ % windowPerPage[currentDisplayConfig[displayType].layoutId]) != 0)
        {
            totalPage++;
        }

        if((displayType == MAIN_DISPLAY) && (m_isPreviousPageNaviagtion))
        {
            m_isPreviousPageNaviagtion = false;
            navigationType = PREVIOUS_PAGE_NAVIGATION;
        }

        if(m_changePageLaterFlag[displayType] == true)
        {
            m_changePageLaterFlag[displayType] = false;
            if(m_changePageOffset[displayType] < 0)
            {
                currentDisplayConfig[displayType].currPage = (currentDisplayConfig[displayType].currPage - (~(m_changePageOffset[displayType]) + 1) + totalPage) % totalPage;
            }
            else if(m_changePageOffset[displayType] >= 0)
            {
                currentDisplayConfig[displayType].currPage = (currentDisplayConfig[displayType].currPage + m_changePageOffset[displayType]) % totalPage;
            }

            currentDisplayConfig[displayType].currPage += m_changePageOffset[displayType];
            m_changePageOffset[displayType] = 0;
        }

        currentDisplayConfig[displayType].currPage = updateCurrentPage(displayType, currentDisplayConfig[displayType].currPage, navigationType);
        currentDisplayConfig[displayType].selectedWindow = currentDisplayConfig[displayType].currPage * windowPerPage[currentDisplayConfig[displayType].layoutId];

        quint8  channelIndex;
        quint16 windowLimit[2], newWindowLimit[2];
        getFirstAndLastWindow(previousPage, currentDisplayConfig[displayType].layoutId, windowLimit);
        getFirstAndLastWindow(currentDisplayConfig[displayType].currPage, currentDisplayConfig[displayType].layoutId, newWindowLimit);

        for(quint16 windowIndex = windowLimit[0], nextWindow = newWindowLimit[0]; windowIndex <= windowLimit[1]; windowIndex++, nextWindow++)
        {
            if((streamInfoArray[displayType][windowIndex].m_videoType == VIDEO_TYPE_LIVESTREAM)
                    && (streamInfoArray[displayType][windowIndex].m_streamId != MAX_STREAM_SESSION))
            {
                channelIndex = currentDisplayConfig[displayType].windowInfo[nextWindow].currentChannel;
                if((currentDisplayConfig[displayType].windowInfo[nextWindow].camInfo[channelIndex].deviceName[0] == '\0')
                        && (currentDisplayConfig[displayType].windowInfo[nextWindow].camInfo[channelIndex].defChannel == INVALID_CAMERA_INDEX))
                {
                    for(quint8 index = 0; index < MAX_WIN_SEQ_CAM; index++)
                    {
                        if((currentDisplayConfig[displayType].windowInfo[nextWindow].camInfo[index].deviceName[0] != '\0')
                                && (currentDisplayConfig[displayType].windowInfo[nextWindow].camInfo[index].defChannel != INVALID_CAMERA_INDEX))
                        {
                            channelIndex = index;
                            currentDisplayConfig[displayType].windowInfo[nextWindow].currentChannel = channelIndex;
                            break;
                        }
                    }
                }

                if((currentDisplayConfig[displayType].windowInfo[nextWindow].camInfo[channelIndex].deviceName[0] != '\0')
                        && (currentDisplayConfig[displayType].windowInfo[nextWindow].camInfo[channelIndex].defChannel != INVALID_CAMERA_INDEX))
                {
                    // replace live stream in-loop for all windows in current page
                    DPRINT(LAYOUT, "page sequence: replace live stream called: [currWin=%d], [nextWin=%d]", windowIndex, nextWindow);
                    replaceLiveStream(displayType,
                                      windowIndex,
                                      currentDisplayConfig[displayType].windowInfo[nextWindow].camInfo[channelIndex].deviceName,
                                      currentDisplayConfig[displayType].windowInfo[nextWindow].camInfo[channelIndex].defChannel,
                                      nextWindow);
                }
                else
                {
                    freeWindow(displayType, windowIndex);
                }
            }
            else
            {
                pendingStreamRequest = freeWindow(displayType, windowIndex,
                                                  (streamInfoArray[displayType][windowIndex].m_videoType == VIDEO_TYPE_INSTANTPLAYBACKSTREAM));
                if(streamInfoArray[displayType][windowIndex].m_videoType == VIDEO_TYPE_PLAYBACKSTREAM)
                {
                    m_pendingRequestCount[displayType] += pendingStreamRequest;
                }
            }
        }
    }

    updateLayoutUiData(displayType);
    if( m_pendingRequestCount[displayType] == 0)
    {
        processNextActionForOtherMode(displayType);
    }
}

void Layout::stopLiveStreamInAllWindows(DISPLAY_TYPE_e displayType)
{
    quint16 windowLimit[2] = {0,MAX_CHANNEL_FOR_SEQ};
    quint8 pendingStreamRequest = 0;

    if ((displayType >= MAX_DISPLAY_TYPE) || (displayType < MAIN_DISPLAY))
    {
        return;
    }

    changeCurrentMode(STATE_OTHER_MODE_PROCESSING_STOP_REQUEST, displayType, false);
    getFirstAndLastWindow(currentDisplayConfig[displayType].currPage, currentDisplayConfig[displayType].layoutId, windowLimit);

    for(quint16 windowIndex = windowLimit[0]; windowIndex <= windowLimit[1]; windowIndex++)
    {
        pendingStreamRequest = freeWindow(displayType, windowIndex);
        if(streamInfoArray[displayType][windowIndex].m_videoType == VIDEO_TYPE_PLAYBACKSTREAM)
        {
            m_pendingRequestCount[displayType] += pendingStreamRequest;
        }
    }

    if(m_pendingRequestCount[displayType] == 0)
    {
        processNextActionForOtherMode(displayType);
    }
}

void Layout::stopStreamsExceptCurrentPage(DISPLAY_TYPE_e displayType)
{
    quint16 windowLimit[2] = {0,MAX_CHANNEL_FOR_SEQ};
    quint8 pendingStreamRequest = 0;

    if ((displayType < MAIN_DISPLAY) || (displayType >= MAX_DISPLAY_TYPE))
    {
        return;
    }

    getFirstAndLastWindow(currentDisplayConfig[displayType].currPage, currentDisplayConfig[displayType].layoutId, windowLimit);

    for(quint16 windowIndex = 0; windowIndex < MAX_CHANNEL_FOR_SEQ; windowIndex++)
    {
        if((windowIndex < windowLimit[0]) || (windowIndex > windowLimit[1]))
        {
            pendingStreamRequest = freeWindow(displayType, windowIndex,
                                              (streamInfoArray[displayType][windowIndex].m_videoType == VIDEO_TYPE_INSTANTPLAYBACKSTREAM));
            if(streamInfoArray[displayType][windowIndex].m_videoType == VIDEO_TYPE_PLAYBACKSTREAM)
            {
                m_pendingRequestCount[displayType] += pendingStreamRequest;
            }
        }
    }
}

void Layout::stopChangedStreamsInCurrentPage(DISPLAY_TYPE_e displayType)
{
    quint16 windowLimit[2] = {0,MAX_CHANNEL_FOR_SEQ};
    quint8 channelIndex;
    quint8 pendingStreamRequest;

    if ((displayType < MAIN_DISPLAY) || (displayType >= MAX_DISPLAY_TYPE))
    {
        return;
    }

    getFirstAndLastWindow(currentDisplayConfig[displayType].currPage, currentDisplayConfig[displayType].layoutId, windowLimit);
    for(quint16 windowIndex = windowLimit[0]; windowIndex <= windowLimit[1]; windowIndex++)
    {
        pendingStreamRequest = 0;
        channelIndex = currentDisplayConfig[displayType].windowInfo[windowIndex].currentChannel;
        switch(streamInfoArray[displayType][windowIndex].m_videoType)
        {
            case VIDEO_TYPE_PLAYBACKSTREAM:
            {
                if((m_isCurrStyleSelected[displayType] == false)
                        || (currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[channelIndex].deviceName[0] != '\0'))
                {
                    pendingStreamRequest = freeWindow(displayType, windowIndex, true);
                }
            }
            break;

            case VIDEO_TYPE_INSTANTPLAYBACKSTREAM:
            {
                if((strcmp(currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[channelIndex].deviceName,
                           m_instantPlaybackData[windowIndex].currentCamInfo.deviceName) != 0)
                        || (currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[channelIndex].defChannel
                            != m_instantPlaybackData[windowIndex].currentCamInfo.defChannel))
                {
                    freeWindow(displayType, windowIndex, true);
                }
            }
            break;

            case VIDEO_TYPE_LIVESTREAM:
            {
                if((QString(currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[channelIndex].deviceName)
                    != streamInfoArray[displayType][windowIndex].m_deviceName)
                        || (currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[channelIndex].defChannel
                            != streamInfoArray[displayType][windowIndex].m_cameraId))
                {
                    freeWindow(displayType, windowIndex);
                }
            }
            break;

            default:
            {
                freeWindow(displayType, windowIndex);
            }
            break;
        }

        m_pendingRequestCount[displayType] += pendingStreamRequest;
    }
}

void Layout::stopStreamsForNewStyleApply(DISPLAY_TYPE_e displayType)
{
    m_pendingRequestCount[displayType] = 0;
    changeCurrentMode(STATE_OTHER_MODE_PROCESSING_STOP_REQUEST, displayType, false);
    stopStreamsExceptCurrentPage(displayType);
    stopChangedStreamsInCurrentPage(displayType);

    if(m_pendingRequestCount[displayType] == 0)
    {
        processNextActionForOtherMode(displayType);
    }
}

void Layout::startStreamForCurrentPage(DISPLAY_TYPE_e displayType)
{
    quint16 windowLimit[2] = {0, MAX_CHANNEL_FOR_SEQ};
    quint8 channelIndex;
    quint8 pendingStreamRequest = 0, streamType;

    if ((displayType < MAIN_DISPLAY) || (displayType >= MAX_DISPLAY_TYPE))
    {
        return;
    }

    m_pendingRequestCount[displayType] = 0;
    changeCurrentMode(STATE_OTHER_MODE_PROCESSING_START_REQUEST, displayType, false);
    getFirstAndLastWindow(currentDisplayConfig[displayType].currPage, currentDisplayConfig[displayType].layoutId, windowLimit);

    for(quint16 windowIndex = windowLimit[0]; windowIndex <= windowLimit[1]; windowIndex++)
    {
        if (streamInfoArray[displayType][windowIndex].m_videoType != VIDEO_TYPE_LIVESTREAM)
        {
            continue;
        }

        if (false == applController->getLiveStreamTypeFrmDev(streamInfoArray[displayType][windowIndex].m_deviceName, streamType))
        {
            continue;
        }

        if (streamType != MAX_LIVE_STREAM_TYPE)
        {
            continue;
        }

        if(streamInfoArray[displayType][windowIndex].m_streamType == LIVE_STREAM_TYPE_MAIN)
        {
            if((currentDisplayConfig[displayType].layoutId != ONE_X_ONE) && (currentDisplayConfig[displayType].layoutId < ONE_X_ONE_PLAYBACK))
            {
                changeLiveStreamType(windowIndex, displayType);
            }
        }
        else if(streamInfoArray[displayType][windowIndex].m_streamType == LIVE_STREAM_TYPE_SUB)
        {
            if(currentDisplayConfig[displayType].layoutId == ONE_X_ONE)
            {
                changeLiveStreamType(windowIndex, displayType);
            }
        }
    }

    for(quint16 windowIndex = windowLimit[0]; windowIndex <= windowLimit[1]; windowIndex++)
    {
        if ((displayType != MAIN_DISPLAY) || (windowIndex >= MAX_CHANNEL_FOR_SEQ))
        {
            break;
        }

        pendingStreamRequest = 0;
        if ((streamInfoArray[displayType][windowIndex].m_videoType == VIDEO_TYPE_NONE)
                && (m_instantPlaybackData[windowIndex].currentCamInfo.defChannel != INVALID_CAMERA_INDEX)
                && (m_instantPlaybackData[windowIndex].currentCamInfo.deviceName[0] != '\0'))
        {
            startInstantPlayBackStream(windowIndex, QString(m_instantPlaybackData[windowIndex].currentCamInfo.deviceName),
                                       m_instantPlaybackData[windowIndex].currentCamInfo.defChannel);
        }
        else if((playbackRecordData[windowIndex].deviceName != "") && (streamInfoArray[MAIN_DISPLAY][windowIndex].m_videoType == VIDEO_TYPE_NONE))
        {
            pendingStreamRequest = getPlaybackId(&playbackRecordData[windowIndex], windowIndex);
            if(pendingStreamRequest == 0)
            {
                changeStreamStateOfWindow(MAIN_DISPLAY,
                                          windowIndex,
                                          playbackRecordData[windowIndex].deviceName,
                                          playbackRecordData[windowIndex].camNo,
                                          MAX_STREAM_SESSION,
                                          VIDEO_TYPE_PLAYBACKSTREAM,
                                          VIDEO_STATUS_RETRY,
                                          VIDEO_ERROR_OTHERERROR);
            }
        }
        else
        {
            channelIndex = currentDisplayConfig[displayType].windowInfo[windowIndex].currentChannel;
            if(((currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[channelIndex].deviceName[0] == '\0')
                && (currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[channelIndex].defChannel == INVALID_CAMERA_INDEX)))
            {
                for(quint8 index = 0; index < MAX_WIN_SEQ_CAM; index++)
                {
                    if((currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[index].deviceName[0] != '\0')
                            && (currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[index].defChannel != INVALID_CAMERA_INDEX))
                    {
                        channelIndex = index;
                        currentDisplayConfig[displayType].windowInfo[windowIndex].currentChannel = channelIndex;
                        break;
                    }
                }
            }

            if((currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[channelIndex].deviceName[0] != '\0')
                    && (streamInfoArray[displayType][windowIndex].m_videoType == VIDEO_TYPE_NONE))
            {
                startLiveStream(displayType, currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[channelIndex].deviceName,
                                currentDisplayConfig[displayType].windowInfo[windowIndex].camInfo[channelIndex].defChannel, windowIndex);
            }
        }

        m_pendingRequestCount[displayType] += pendingStreamRequest;
    }

    updateLayoutUiData(displayType);

    if(m_pendingRequestCount[displayType] == 0)
    {
        processNextActionForOtherMode(displayType);
    }
}

void Layout::startPlayBackForCurrentPage(DISPLAY_TYPE_e displayType)
{
    quint16 windowLimit[2] = {0,MAX_CHANNEL_FOR_SEQ};
    quint8 pendingStreamRequest = 0;

    if ((displayType < MAIN_DISPLAY) || (displayType >= MAX_DISPLAY_TYPE))
    {
        return;
    }

    m_pendingRequestCount[displayType] = 0;
    changeCurrentMode(STATE_OTHER_MODE_PROCESSING_START_REQUEST, displayType, false);
    getFirstAndLastWindow(currentDisplayConfig[displayType].currPage, currentDisplayConfig[displayType].layoutId, windowLimit);

    for(quint16 windowIndex = windowLimit[0]; windowIndex <= windowLimit[1]; windowIndex++)
    {
        if ((displayType != MAIN_DISPLAY) || (windowIndex >= MAX_CHANNEL_FOR_SEQ))
        {
            break;
        }

        pendingStreamRequest = 0;
        if((playbackRecordData[windowIndex].deviceName != "") && (streamInfoArray[MAIN_DISPLAY][windowIndex].m_videoType == VIDEO_TYPE_NONE))
        {
            pendingStreamRequest = getPlaybackId(&playbackRecordData[windowIndex], windowIndex);
            if(pendingStreamRequest == 0)
            {
                changeStreamStateOfWindow(MAIN_DISPLAY,
                                          windowIndex,
                                          playbackRecordData[windowIndex].deviceName,
                                          playbackRecordData[windowIndex].camNo,
                                          MAX_STREAM_SESSION,
                                          VIDEO_TYPE_PLAYBACKSTREAM,
                                          VIDEO_STATUS_RETRY,
                                          VIDEO_ERROR_OTHERERROR);
            }
        }

        m_pendingRequestCount[displayType] += pendingStreamRequest;
    }

    if(m_pendingRequestCount[displayType] == 0)
    {
        processNextActionForOtherMode(displayType);
    }
}

void Layout::startStreamsForPageInRetry(DISPLAY_TYPE_e displayType)
{
    quint16 windowLimit[2] = {0,MAX_CHANNEL_FOR_SEQ};
    quint8  pendingStreamRequest = 0;

    if ((displayType < MAIN_DISPLAY) || (displayType >= MAX_DISPLAY_TYPE))
    {
        return;
    }

    changeCurrentMode(STATE_OTHER_MODE_PROCESSING_START_REQUEST, displayType, false);
    getFirstAndLastWindow(currentDisplayConfig[displayType].currPage, currentDisplayConfig[displayType].layoutId, windowLimit);

    for(quint16 windowIndex = windowLimit[0]; windowIndex <= windowLimit[1]; windowIndex++)
    {
        pendingStreamRequest = 0;
        if (streamInfoArray[displayType][windowIndex].m_videoStatus == VIDEO_STATUS_RETRY)
        {
            if(streamInfoArray[displayType][windowIndex].m_videoType == VIDEO_TYPE_LIVESTREAM)
            {
                startLiveStream(displayType, streamInfoArray[displayType][windowIndex].m_deviceName,
                                streamInfoArray[displayType][windowIndex].m_cameraId, windowIndex);
            }
            else if((streamInfoArray[displayType][windowIndex].m_videoType == VIDEO_TYPE_PLAYBACKSTREAM)
                    && (playbackRecordData[windowIndex].deviceName != ""))
            {
                pendingStreamRequest = getPlaybackId(&playbackRecordData[windowIndex], windowIndex);
                m_pendingRequestCount[displayType] += pendingStreamRequest;
            }
        }
    }

    // Do next process by so that restart timer kills and Don't send isNextProcessingNeeded false
    changeCurrentMode(STATE_OTHER_MODE_NONE, displayType);
}

void Layout::shiftWindows(DISPLAY_TYPE_e displayType, quint16 newSelectedWindow, LAYOUT_TYPE_e newLayout)
{
    quint16 newPage = newSelectedWindow / windowPerPage[newLayout];
    quint16 oldWindowLimits[2] = {0,MAX_CHANNEL_FOR_SEQ}, newWindowLimits[2] = {0,MAX_CHANNEL_FOR_SEQ};

    getFirstAndLastWindow(newPage, newLayout, newWindowLimits);
    if(displayType < MAX_DISPLAY_TYPE)
    {
        getFirstAndLastWindow(currentDisplayConfig[displayType].currPage, currentDisplayConfig[displayType].layoutId, oldWindowLimits);
        currentDisplayConfig[displayType].selectedWindow = newSelectedWindow;
    }

    if((newWindowLimits[1] < oldWindowLimits[0]) || (oldWindowLimits[1] < newWindowLimits[0]))
    {
        changeLayout(displayType, newLayout, newSelectedWindow);
        return;
    }

    //which window to drop in current page
    if((newLayout >= 0) && (displayType < MAX_DISPLAY_TYPE) && (newLayout < MAX_LAYOUT) &&
            (currentDisplayConfig[displayType].layoutId >=0 ) && (currentDisplayConfig[displayType].layoutId < MAX_LAYOUT))
    {
        qint8 offset = (currentDisplayConfig[displayType].currPage *
                        windowPerPage[currentDisplayConfig[displayType].layoutId]) - (newPage * windowPerPage[newLayout]);
        if(false == applController->ShiftWindows(displayType, offset, (newSelectedWindow % windowPerPage[newLayout]), newLayout))
        {
            return;
        }

        quint16 arrayIndex = oldWindowLimits[0];
        quint8 off = (offset >= 0) ? offset : ((~offset) + 1);
        if(offset >= 0) // downShift
        {
            for(quint8 windowIndex = 0, tmpShift = 0; windowIndex < MAX_WINDOWS; windowIndex++, tmpShift++, arrayIndex++)
            {
                if(streamInfoArray[displayType][arrayIndex].m_streamId != MAX_STREAM_SESSION)
                {
                    if((windowIndex + off) < MAX_WINDOWS)
                    {
                        streamInfoArray[displayType][arrayIndex].m_windowId = (windowIndex + off);
                    }
                    else
                    {
                        streamInfoArray[displayType][arrayIndex].m_windowId = tmpShift;
                    }
                }
            }
        }
        else // upshif
        {
            for(quint8 windowIndex = 0, tmpShift = (MAX_WINDOWS - off); windowIndex < MAX_WINDOWS; windowIndex++, tmpShift++, arrayIndex++)
            {
                if(streamInfoArray[displayType][arrayIndex].m_windowId != MAX_WINDOWS)
                {
                    if(windowIndex < off)
                    {
                        streamInfoArray[displayType][arrayIndex].m_windowId = tmpShift;
                    }
                    else
                    {
                        streamInfoArray[displayType][arrayIndex].m_windowId = (windowIndex - off);
                    }
                }
            }
        }
    }

    if (true == getScreenParam(newLayout, displayType))
    {
        currentDisplayConfig[displayType].currPage = (newSelectedWindow / windowPerPage[newLayout]);
        m_previousLayout[displayType] = currentDisplayConfig[displayType].layoutId;
        currentDisplayConfig[displayType].layoutId = newLayout;

        this->setGeometry(startX[MAIN_DISPLAY], startY[MAIN_DISPLAY], screenWidth[MAIN_DISPLAY], screenHeight[MAIN_DISPLAY]);
        layoutCreator->setGeometry(0, 0, this->width(), this->height());

        updateLayoutUiData(displayType);
        emit sigUpdateUIGeometry(isRedraw);
        isRedraw = false;
    }
}

void Layout::closeDeletePbToolBar()
{
    if(m_pbToolbar != NULL)
    {
        disconnect (m_pbToolbar,
                    SIGNAL(sigSliderValueChanged(quint64,quint16)),
                    this,
                    SLOT(slotPbToolbarSliderChanged(quint64,quint16)));
        disconnect (m_pbToolbar,
                    SIGNAL(sigPbToolbarBtnClick(int,quint16,bool)),
                    this,
                    SLOT(slotPbToolbarHandling(int,quint16,bool)));
        DELETE_OBJ(m_pbToolbar);
        ApplicationMode::setApplicationMode(IDLE_MODE);
    }
}

void Layout::createPbToolbar(quint16 windowIndex)
{
    bool toolbarCreation = false;
    bool pbStatusPlay = true;
    WIN_DIMENSION_INFO_t windowInfo = {0,0,0,0};

    if(m_pbToolbar != NULL)
    {
        if(m_pbToolbar->getPbToolbarWinId() != windowIndex)
        {
            closeDeletePbToolBar();
            toolbarCreation = true;
        }
    }
    else
    {
        toolbarCreation = true;
    }

    if (false == toolbarCreation)
    {
        return;
    }

    if(m_pbToolbar == NULL)
    {
        layoutCreator->getWindowDimensionInfo(windowIndex, &windowInfo);

        /* PARASOFT: Memory Deallocated in close Delete PbToolbar */
        m_pbToolbar = new PbToolbar(windowInfo.winStartx,
                                    windowInfo.winStarty,
                                    windowInfo.winWidth,
                                    windowInfo.winHeight,
                                    windowIndex,
                                    layoutCreator->getPbtoolbarSize (windowIndex),
                                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStartTime,
                                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbEndTime,
                                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbCurrentTime,
                                    this);

        if(streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamState == PB_PLAY_STATE)
        {
            pbStatusPlay = false;
        }

        if(streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbDirection == FORWARD_PLAY)
        {
            m_pbToolbar->changeStateOfButton (PB_TOOLBAR_ELE_PLAY_BTN, pbStatusPlay);
        }
        else
        {
            m_pbToolbar->changeStateOfButton (PB_TOOLBAR_ELE_REV_PLAY_BTN, pbStatusPlay);
        }

        if(streamInfoArray[MAIN_DISPLAY][windowIndex].m_audioStatus == true)
        {
            m_pbToolbar->changeStateOfButton (PB_TOOLBAR_ELE_MUTE_BTN, false);
        }
        else
        {
            m_pbToolbar->changeStateOfButton (PB_TOOLBAR_ELE_MUTE_BTN, true);
        }

        connect (m_pbToolbar,
                 SIGNAL(sigPbToolbarBtnClick(int,quint16,bool)),
                 this,
                 SLOT(slotPbToolbarHandling(int,quint16,bool)));

        connect (m_pbToolbar,
                 SIGNAL(sigSliderValueChanged(quint64,quint16)),
                 this,
                 SLOT(slotPbToolbarSliderChanged(quint64,quint16)));

        ApplicationMode::setApplicationMode(ASYNC_PLAYBACK_TOOLBAR_MODE);
    }
}

bool Layout::isPlaybackRunning()
{
    for(quint16 index = 0; index < MAX_CHANNEL_FOR_SEQ; index++)
    {
        if((streamInfoArray[MAIN_DISPLAY][index].m_videoType == VIDEO_TYPE_PLAYBACKSTREAM)
                || (streamInfoArray[MAIN_DISPLAY][index].m_videoType == VIDEO_TYPE_INSTANTPLAYBACKSTREAM)
                || (streamInfoArray[MAIN_DISPLAY][index].m_videoType == VIDEO_TYPE_SYNCPLAYBAKSTREAM))
        {
            return true;
        }
    }
    return false;
}

quint8 Layout::getPlaybackId(PlaybackRecordData *recData, quint16 windowIndex)
{
    quint8 pendingStreamRequest = 0, streamId = MAX_STREAM_SESSION;
    SERVER_SESSION_INFO_t serverSessionInfo;
    StreamRequestParam *streamRequestParam = NULL;

    applController->GetServerSessionInfo(recData->deviceName,serverSessionInfo);

    payloadLib->setCnfgArrayAtIndex(0, recData->startTime);
    payloadLib->setCnfgArrayAtIndex(1, recData->endTime);
    payloadLib->setCnfgArrayAtIndex(2, recData->camNo);
    payloadLib->setCnfgArrayAtIndex(3, recData->evtType);
    payloadLib->setCnfgArrayAtIndex(4, recData->overlap);
    payloadLib->setCnfgArrayAtIndex(5, recData->hddIndicator);
    payloadLib->setCnfgArrayAtIndex(6, recData->partionIndicator);
    payloadLib->setCnfgArrayAtIndex(7, recData->recDriveIndex);

    streamRequestParam = new StreamRequestParam();
    streamRequestParam->windowId = (windowIndex % windowPerPage[currentDisplayConfig[MAIN_DISPLAY].layoutId]);
    streamRequestParam->actualWindowId = windowIndex;
    streamRequestParam->channelId = recData->camNo;
    streamRequestParam->deviceName = recData->deviceName;
    streamRequestParam->displayType = MAIN_DISPLAY;
    streamRequestParam->streamRequestType = PLAYBACK_STREAM_REQUEST;
    streamRequestParam->payload = payloadLib->createDevCmdPayload(8);

    pendingStreamRequest = applController->processStreamActivity(GET_PLAYBACK_ID_COMMAND,
                                                                 serverSessionInfo,
                                                                 streamRequestParam,
                                                                 &streamId);
    if (pendingStreamRequest == 0)
    {
        DELETE_OBJ(streamRequestParam);
    }
    else
    {
        streamInfoArray[MAIN_DISPLAY][windowIndex].m_videoType = VIDEO_TYPE_PLAYBACKSTREAM;
        streamInfoArray[MAIN_DISPLAY][windowIndex].m_errorType = VIDEO_ERROR_NONE;
        streamInfoArray[MAIN_DISPLAY][windowIndex].m_windowId = (windowIndex % windowPerPage[currentDisplayConfig[MAIN_DISPLAY].layoutId]);
        streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStartTime = stringToMSec(recData->startTime);
        streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbCurrentTime = stringToMSec(recData->startTime);
        streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbEndTime = stringToMSec(recData->endTime);
        streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbDirection = FORWARD_PLAY;
        streamInfoArray[MAIN_DISPLAY][windowIndex].m_audioStatus = false;
        streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbSpeed = PB_SPEED_NORMAL;
        streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbCurrCmd = PB_MAX_PLAYBACK_STATE;
        streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbNextCmd = PB_MAX_PLAYBACK_STATE;
        streamInfoArray[MAIN_DISPLAY][windowIndex].m_streamId = streamId;
        streamInfoArray[MAIN_DISPLAY][windowIndex].m_startRequestPending = true;

        changeStreamStateOfWindow(MAIN_DISPLAY,
                                  windowIndex,
                                  recData->deviceName,
                                  recData->camNo,
                                  streamId,
                                  VIDEO_TYPE_PLAYBACKSTREAM,
                                  VIDEO_STATUS_CONNECTING,
                                  VIDEO_ERROR_NONE);
    }

    return pendingStreamRequest;
}

quint8 Layout::sendPlyRcdStrm(quint16 windowId)
{
    quint8 pendingStreamRequest = 0;
    SERVER_SESSION_INFO_t serverSessionInfo;
    StreamRequestParam *streamRequestParam = NULL;

    applController->GetServerSessionInfo(streamInfoArray[MAIN_DISPLAY][windowId].m_deviceName, serverSessionInfo);

    payloadLib->setCnfgArrayAtIndex(0, streamInfoArray[MAIN_DISPLAY][windowId].m_pbStreamId);
    payloadLib->setCnfgArrayAtIndex(1, streamInfoArray[MAIN_DISPLAY][windowId].m_pbDirection);
    payloadLib->setCnfgArrayAtIndex(2, mSecToString(streamInfoArray[MAIN_DISPLAY][windowId].m_pbCurrentTime));
    payloadLib->setCnfgArrayAtIndex(3, 1); // Audio
    payloadLib->setCnfgArrayAtIndex(4, streamInfoArray[MAIN_DISPLAY][windowId].m_pbSpeed);

    streamRequestParam = new StreamRequestParam();
    streamRequestParam->streamId = streamInfoArray[MAIN_DISPLAY][windowId].m_streamId;
    streamRequestParam->windowId = streamInfoArray[MAIN_DISPLAY][windowId].m_windowId;
    streamRequestParam->actualWindowId = windowId;
    streamRequestParam->channelId = streamInfoArray[MAIN_DISPLAY][windowId].m_cameraId;
    streamRequestParam->deviceName = streamInfoArray[MAIN_DISPLAY][windowId].m_deviceName;
    streamRequestParam->displayType = MAIN_DISPLAY;
    streamRequestParam->payload = payloadLib->createDevCmdPayload(5);

    pendingStreamRequest = applController->processStreamActivity(PLAY_PLABACK_STREAM_COMMAND,
                                                                 serverSessionInfo,
                                                                 streamRequestParam);
    if (pendingStreamRequest == 0)
    {
        DELETE_OBJ(streamRequestParam);
    }

    return pendingStreamRequest;
}

quint8 Layout::sendStepRcdStrm(quint16 windowId)
{
    quint8 pendingStreamRequest = 0;
    SERVER_SESSION_INFO_t serverSessionInfo;
    StreamRequestParam *streamRequestParam = NULL;

    applController->GetServerSessionInfo(streamInfoArray[MAIN_DISPLAY][windowId].m_deviceName, serverSessionInfo);

    payloadLib->setCnfgArrayAtIndex(0, streamInfoArray[MAIN_DISPLAY][windowId].m_pbStreamId);

    if(playbackStopRequest)
    {
        payloadLib->setCnfgArrayAtIndex(1, FORWARD_PLAY);
        payloadLib->setCnfgArrayAtIndex(2, mSecToString(streamInfoArray[MAIN_DISPLAY][windowId].m_pbStartTime, true));
        streamInfoArray[MAIN_DISPLAY][windowId].m_iFrameNeeded = true;
        playbackStopRequest = false;
    }
    else
    {
        payloadLib->setCnfgArrayAtIndex(1, streamInfoArray[MAIN_DISPLAY][windowId].m_pbDirection);
        payloadLib->setCnfgArrayAtIndex(2, mSecToString(streamInfoArray[MAIN_DISPLAY][windowId].m_pbCurrentTime, true));
    }

    if((streamInfoArray[MAIN_DISPLAY][windowId].m_pbDirection == BACKWARD_PLAY) || (streamInfoArray[MAIN_DISPLAY][windowId].m_iFrameNeeded == true))
    {
        payloadLib->setCnfgArrayAtIndex(3, 0);  // only I- frames
    }
    else
    {
        payloadLib->setCnfgArrayAtIndex(3, 1); // any frame
    }

    streamRequestParam = new StreamRequestParam();
    streamRequestParam->streamId = streamInfoArray[MAIN_DISPLAY][windowId].m_streamId;
    streamRequestParam->windowId = streamInfoArray[MAIN_DISPLAY][windowId].m_windowId;
    streamRequestParam->actualWindowId = windowId;
    streamRequestParam->channelId = streamInfoArray[MAIN_DISPLAY][windowId].m_cameraId;
    streamRequestParam->deviceName = streamInfoArray[MAIN_DISPLAY][windowId].m_deviceName;
    streamRequestParam->displayType = MAIN_DISPLAY;
    streamRequestParam->payload = payloadLib->createDevCmdPayload(4);

    pendingStreamRequest = applController->processStreamActivity(STEP_PLABACK_STREAM_COMMAND,
                                                                 serverSessionInfo,
                                                                 streamRequestParam);
    if (pendingStreamRequest == 0)
    {
        DELETE_OBJ(streamRequestParam);
    }

    return pendingStreamRequest;
}

quint8 Layout::sendStpRcdStrm(quint16 windowId)
{
    quint8 pendingStreamRequest = 0;
    SERVER_SESSION_INFO_t serverSessionInfo;
    StreamRequestParam *streamRequestParam = NULL;

    DPRINT(GUI_PB_MEDIA, "send step playback: [window=%d], [pbStreamId=%d], [pbStreamState=%d]",
           windowId, streamInfoArray[MAIN_DISPLAY][windowId].m_pbStreamId, streamInfoArray[MAIN_DISPLAY][windowId].m_pbStreamState);
    if(false == applController->GetServerSessionInfo(streamInfoArray[MAIN_DISPLAY][windowId].m_deviceName, serverSessionInfo))
    {
        EPRINT(GUI_PB_MEDIA, "fail to get server session info");
    }

    payloadLib->setCnfgArrayAtIndex(0, streamInfoArray[MAIN_DISPLAY][windowId].m_pbStreamId);

    streamRequestParam = new StreamRequestParam();
    streamRequestParam->streamId = streamInfoArray[MAIN_DISPLAY][windowId].m_streamId;
    streamRequestParam->windowId = streamInfoArray[MAIN_DISPLAY][windowId].m_windowId;
    streamRequestParam->actualWindowId = windowId;
    streamRequestParam->channelId = streamInfoArray[MAIN_DISPLAY][windowId].m_cameraId;
    streamRequestParam->deviceName = streamInfoArray[MAIN_DISPLAY][windowId].m_deviceName;
    streamRequestParam->displayType = MAIN_DISPLAY;
    streamRequestParam->payload = payloadLib->createDevCmdPayload(1);

    pendingStreamRequest = applController->processStreamActivity(STOP_PLABACK_STREAM_COMMAND,
                                                                 serverSessionInfo,
                                                                 streamRequestParam);
    if (pendingStreamRequest == 0)
    {
        DELETE_OBJ(streamRequestParam);
    }

    return pendingStreamRequest;
}

quint8 Layout::sendClrPlyStrmId(quint16 windowId)
{
    quint8 pendingStreamRequest = 0;
    SERVER_SESSION_INFO_t serverSessionInfo;
    StreamRequestParam *streamRequestParam = NULL;

    DPRINT(GUI_PB_MEDIA, "send clear playback: [window=%d], [pbStreamId=%d], [pbStreamState=%d]",
           windowId, streamInfoArray[MAIN_DISPLAY][windowId].m_pbStreamId, streamInfoArray[MAIN_DISPLAY][windowId].m_pbStreamState);
    if(false == applController->GetServerSessionInfo(streamInfoArray[MAIN_DISPLAY][windowId].m_deviceName, serverSessionInfo))
    {
        EPRINT(LAYOUT, "fail to get server session info");
    }

    payloadLib->setCnfgArrayAtIndex(0, streamInfoArray[MAIN_DISPLAY][windowId].m_pbStreamId);

    streamRequestParam = new StreamRequestParam();
    streamRequestParam->streamId = streamInfoArray[MAIN_DISPLAY][windowId].m_streamId;
    streamRequestParam->windowId = streamInfoArray[MAIN_DISPLAY][windowId].m_windowId;
    streamRequestParam->actualWindowId = windowId;
    streamRequestParam->channelId = streamInfoArray[MAIN_DISPLAY][windowId].m_cameraId;
    streamRequestParam->deviceName = streamInfoArray[MAIN_DISPLAY][windowId].m_deviceName;
    streamRequestParam->displayType = MAIN_DISPLAY;
    streamRequestParam->payload = payloadLib->createDevCmdPayload(1);

    pendingStreamRequest = applController->processStreamActivity(CLEAR_PLAYBACK_ID_COMMAND,
                                                                 serverSessionInfo,
                                                                 streamRequestParam);
    if (pendingStreamRequest == 0)
    {
        DELETE_OBJ(streamRequestParam);
    }
    else
    {
        streamInfoArray[MAIN_DISPLAY][windowId].m_stopRequestPending = true;
    }

    return pendingStreamRequest;
}

quint8 Layout::sendPbRequest(quint16 windowId)
{
    quint8 pendingStreamRequest = 0;

    if (streamInfoArray[MAIN_DISPLAY][windowId].m_pbStreamId == MAX_STREAM_SESSION)
    {
        return pendingStreamRequest;
    }

    switch(streamInfoArray[MAIN_DISPLAY][windowId].m_pbCurrCmd)
    {
        case PB_PLAY_STATE:
            pendingStreamRequest = sendPlyRcdStrm(windowId);
            break;

        case PB_PAUSE_STATE:
            pendingStreamRequest = sendStpRcdStrm(windowId);
            break;

        case PB_STOP_STATE:
            pendingStreamRequest = sendClrPlyStrmId(windowId);
            break;

        case PB_STEP_STATE:
            pendingStreamRequest = sendStepRcdStrm(windowId);
            break;

        default:
            break;
    }
    return pendingStreamRequest;
}

bool Layout::startPlaybackStream(quint16 windowId, PB_DIRECTION_e direction, quint8 speed)
{
    streamInfoArray[MAIN_DISPLAY][windowId].m_pbDirection = direction;
    streamInfoArray[MAIN_DISPLAY][windowId].m_pbSpeed = speed;

    if(streamInfoArray[MAIN_DISPLAY][windowId].m_pbStreamState != PB_PLAY_STATE)
    {
        streamInfoArray[MAIN_DISPLAY][windowId].m_pbCurrCmd = PB_PLAY_STATE;
        streamInfoArray[MAIN_DISPLAY][windowId].m_pbNextCmd = PB_MAX_PLAYBACK_STATE;
        streamInfoArray[MAIN_DISPLAY][windowId].prevPlayState = PB_PLAY_STATE;
    }
    else
    {
        streamInfoArray[MAIN_DISPLAY][windowId].m_pbCurrCmd = PB_PAUSE_STATE;
        streamInfoArray[MAIN_DISPLAY][windowId].m_pbNextCmd = PB_PLAY_STATE;
    }
    return sendPbRequest(windowId);
}

bool Layout::stopPlaybackStream(quint16 windowId)
{
    DPRINT(GUI_PB_MEDIA, "stop playback: [window=%d], [pbStreamState=%d]", windowId, streamInfoArray[MAIN_DISPLAY][windowId].m_pbStreamState);
    if(streamInfoArray[MAIN_DISPLAY][windowId].m_pbStreamState == PB_PLAY_STATE)
    {
        streamInfoArray[MAIN_DISPLAY][windowId].m_pbCurrCmd = PB_PAUSE_STATE;
        streamInfoArray[MAIN_DISPLAY][windowId].m_pbNextCmd = PB_STOP_STATE;
    }
    else
    {
        streamInfoArray[MAIN_DISPLAY][windowId].m_pbCurrCmd = PB_STOP_STATE;
        streamInfoArray[MAIN_DISPLAY][windowId].m_pbNextCmd = PB_MAX_PLAYBACK_STATE;
    }

    return sendPbRequest (windowId);
}

bool Layout::slowPlaybackStream(quint16 windowId)
{
    if (streamInfoArray[MAIN_DISPLAY][windowId].m_pbStreamState != PB_PLAY_STATE)
    {
        return false;
    }

    if ((streamInfoArray[MAIN_DISPLAY][windowId].m_pbSpeed - 1) >= 0)
    {
        streamInfoArray[MAIN_DISPLAY][windowId].m_pbSpeed--;
        streamInfoArray[MAIN_DISPLAY][windowId].m_pbCurrCmd = PB_PAUSE_STATE;
        streamInfoArray[MAIN_DISPLAY][windowId].m_pbNextCmd = PB_PLAY_STATE;
        return sendPbRequest (windowId);
    }

    return false;
}

bool Layout::fastPlaybackStream(quint16 windowId)
{
    if (streamInfoArray[MAIN_DISPLAY][windowId].m_pbStreamState != PB_PLAY_STATE)
    {
        return false;
    }

    if ((streamInfoArray[MAIN_DISPLAY][windowId].m_pbSpeed + 1) < MAX_PB_SPEED)
    {
        streamInfoArray[MAIN_DISPLAY][windowId].m_pbSpeed++;
        streamInfoArray[MAIN_DISPLAY][windowId].m_pbCurrCmd = PB_PAUSE_STATE;
        streamInfoArray[MAIN_DISPLAY][windowId].m_pbNextCmd = PB_PLAY_STATE;
        return sendPbRequest (windowId);
    }

    return false;
}

bool Layout::stepPlaybackStream(quint16 windowId, PB_DIRECTION_e direction)
{
    streamInfoArray[MAIN_DISPLAY][windowId].m_pbDirection = direction;
    if (streamInfoArray[MAIN_DISPLAY][windowId].m_pbStreamState == PB_PLAY_STATE)
    {
        streamInfoArray[MAIN_DISPLAY][windowId].m_pbCurrCmd = PB_PAUSE_STATE;
        streamInfoArray[MAIN_DISPLAY][windowId].m_pbNextCmd = PB_STEP_STATE;
    }
    else
    {
        streamInfoArray[MAIN_DISPLAY][windowId].m_pbCurrCmd = PB_STEP_STATE;
        streamInfoArray[MAIN_DISPLAY][windowId].m_pbNextCmd = PB_MAX_PLAYBACK_STATE;
    }
    return sendPbRequest(windowId);
}

bool Layout::pausePlaybackStream(quint16 windowId)
{
    if (streamInfoArray[MAIN_DISPLAY][windowId].m_pbStreamState == PB_PLAY_STATE)
    {
        streamInfoArray[MAIN_DISPLAY][windowId].m_pbCurrCmd = PB_PAUSE_STATE;
        streamInfoArray[MAIN_DISPLAY][windowId].m_pbNextCmd = PB_MAX_PLAYBACK_STATE;
    }
    else
    {
        streamInfoArray[MAIN_DISPLAY][windowId].m_pbCurrCmd = PB_MAX_PLAYBACK_STATE;
        streamInfoArray[MAIN_DISPLAY][windowId].m_pbNextCmd = PB_MAX_PLAYBACK_STATE;
    }
    return sendPbRequest (windowId);
}

void Layout::pausePlaybackStreamOfCurrentPage()
{
    quint16 windowLimit[2] = {0,MAX_CHANNEL_FOR_SEQ};

    getFirstAndLastWindow(currentDisplayConfig[MAIN_DISPLAY].currPage, currentDisplayConfig[MAIN_DISPLAY].layoutId, windowLimit);
    for(quint16 index = windowLimit[0]; index <= windowLimit[1]; index++)
    {
        if(streamInfoArray[MAIN_DISPLAY][index].m_videoType == VIDEO_TYPE_PLAYBACKSTREAM)
        {
            pausePlaybackStream(index);
        }
        else if(streamInfoArray[MAIN_DISPLAY][index].m_videoType == VIDEO_TYPE_INSTANTPLAYBACKSTREAM)
        {
            pauseInstantPlaybackStream (index);
        }
    }
}

QString Layout::mSecToString(quint64 currTime, bool msecNeeded)
{
    QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(currTime);
    if(msecNeeded == false)
    {
        return dateTime.toString("ddMMyyyyHHmmss");
    }
    else
    {
        return dateTime.toString("ddMMyyyyHHmmsszzz");
    }
}

quint64 Layout::stringToMSec(QString currTimeStr)
{
    QDateTime dateTime = QDateTime::fromString(currTimeStr, "ddMMyyyyHHmmss");
    return dateTime.toMSecsSinceEpoch();
}

void Layout::sendEventToPtz(QString deviceName, quint8 camNum, LOG_EVENT_STATE_e evtState)
{
    if(m_ptzControl != NULL)
    {
        m_ptzControl->updateManualTourStatus(deviceName, camNum, evtState);
    }
}

void Layout::startStopManualRecording(quint16 windowIndex)
{
    quint8 manualRecordingStatus = 0;

    getSingleHealthParamStatus(MAIN_DISPLAY, windowIndex, MANUAL_RECORDING_STS, manualRecordingStatus);

    payloadLib->setCnfgArrayAtIndex(0, streamInfoArray[MAIN_DISPLAY][windowIndex].m_cameraId);
    if(manualRecordingStatus == EVENT_START)
    {
        createCmdPayload(STP_MAN_REC, 1);
    }
    else
    {
        createCmdPayload(SRT_MAN_REC, 1);
    }
}

void Layout::takeSnapshot(quint16 windowIndex)
{
    payloadLib->setCnfgArrayAtIndex(0, streamInfoArray[MAIN_DISPLAY][windowIndex].m_cameraId);
    createCmdPayload(SNP_SHT, 1);
}

void Layout::changeLiveStreamType(quint16 windowIndex, DISPLAY_TYPE_e displayType)
{
    LIVE_STREAM_TYPE_e streamType = streamInfoArray[displayType][windowIndex].m_streamType;
    SERVER_SESSION_INFO_t serverSessionInfo;
    StreamRequestParam *streamRequestParam = NULL;
    quint8 processRequest = 0;

    /* Change stream type */
    streamType = (streamType == LIVE_STREAM_TYPE_MAIN) ? LIVE_STREAM_TYPE_SUB : LIVE_STREAM_TYPE_MAIN;

    /* Get server info and update live stream video loss timeout duration */
    applController->GetServerSessionInfo(streamInfoArray[displayType][windowIndex].m_deviceName, serverSessionInfo);
    applController->GetPreVideoLossDuration(streamInfoArray[displayType][windowIndex].m_deviceName, serverSessionInfo.sessionInfo.timeout);

    payloadLib->setCnfgArrayAtIndex(0, streamInfoArray[displayType][windowIndex].m_cameraId);
    payloadLib->setCnfgArrayAtIndex(1, streamType);

    streamRequestParam = new StreamRequestParam();
    streamRequestParam->streamId = streamInfoArray[displayType][windowIndex].m_streamId;
    streamRequestParam->channelId = streamInfoArray[displayType][windowIndex].m_cameraId;
    streamRequestParam->liveStreamType = streamType;
    streamRequestParam->windowId = streamInfoArray[displayType][windowIndex].m_windowId;
    streamRequestParam->actualWindowId = windowIndex;
    streamRequestParam->deviceName = streamInfoArray[displayType][windowIndex].m_deviceName;
    streamRequestParam->displayType = displayType;
    streamRequestParam->payload = payloadLib->createDevCmdPayload(2);

    processRequest = applController->processStreamActivity(CHANGE_STREAM_TYPE_COMMAND,
                                                           serverSessionInfo,
                                                           streamRequestParam);
    if (processRequest == 0)
    {
        DELETE_OBJ(streamRequestParam);
    }
}

void Layout::toggleStreamType(quint16 windowIndex, DISPLAY_TYPE_e dispId)
{
    if (streamInfoArray[dispId][windowIndex].m_streamType == LIVE_STREAM_TYPE_MAIN)
    {
        streamInfoArray[dispId][windowIndex].m_streamType = LIVE_STREAM_TYPE_SUB;
    }
    else
    {
        streamInfoArray[dispId][windowIndex].m_streamType = LIVE_STREAM_TYPE_MAIN;
    }

    /* If audio was enabled for previous stream then again include it to update the status.
     * Either audio will remain as it is if configured in stream settings else it will get disabled */
    if (true == streamInfoArray[dispId][windowIndex].m_audioStatus)
    {
        includeDevAudio(windowIndex);
    }
}

void Layout::includeExcludeAudio(quint16 windowIndex, bool status)
{
    if (windowIndex >= MAX_CHANNEL_FOR_SEQ)
    {
        EPRINT(LAYOUT, "invld window index: [windowIndex=%d], [status=%d]", windowIndex, status);
        return;
    }

    if(streamInfoArray[MAIN_DISPLAY][windowIndex].m_audioStatus)
    {
        excludeDevAudio(windowIndex);
        startAudioInWindow = MAX_CHANNEL_FOR_SEQ;
        if(status == false)
        {
            lastEnableAudioWindow = MAX_CHANNEL_FOR_SEQ;
        }
    }
    else
    {
        if(startAudioInWindow == MAX_CHANNEL_FOR_SEQ)
        {
            includeDevAudio(windowIndex);
            if(status == false)
            {
                lastEnableAudioWindow = MAX_CHANNEL_FOR_SEQ;
            }
        }
        else
        {
            excludeDevAudio(startAudioInWindow);
            if(status == false)
            {
                lastEnableAudioWindow = MAX_CHANNEL_FOR_SEQ;
            }
        }
        startAudioInWindow = windowIndex;
    }
}

void Layout::includeDevAudio(quint16 audioWindowIndex)
{
    STREAM_COMMAND_TYPE_e streamCommandType = MAX_STREAM_COMMAND;
    StreamRequestParam *streamRequestParam = NULL;
    quint8 processRequest = 0;

    if(audioWindowIndex < MAX_CHANNEL_FOR_SEQ)
    {
        switch(streamInfoArray[MAIN_DISPLAY][audioWindowIndex].m_videoType)
        {
            case VIDEO_TYPE_LIVESTREAM:
                streamCommandType = INCLUDE_AUDIO_COMMAND;
                break;

            case VIDEO_TYPE_PLAYBACKSTREAM:
                streamCommandType = AUDIO_PLABACK_STREAM_COMMAND;
                break;

            case VIDEO_TYPE_INSTANTPLAYBACKSTREAM:
                streamCommandType = AUDIO_INSTANTPLAYBACK_COMMAND;
                break;

            default:
                break;
        }
    }

    if(streamCommandType == MAX_STREAM_COMMAND)
    {
        return;
    }

    SERVER_SESSION_INFO_t serverSessionInfo;

    applController->GetServerSessionInfo(streamInfoArray[MAIN_DISPLAY][audioWindowIndex].m_deviceName, serverSessionInfo);

    payloadLib->setCnfgArrayAtIndex(0, streamInfoArray[MAIN_DISPLAY][audioWindowIndex].m_cameraId);
    payloadLib->setCnfgArrayAtIndex(1, streamInfoArray[MAIN_DISPLAY][audioWindowIndex].m_streamType);

    streamRequestParam = new StreamRequestParam();
    streamRequestParam->streamId = streamInfoArray[MAIN_DISPLAY][audioWindowIndex].m_streamId;
    streamRequestParam->channelId = streamInfoArray[MAIN_DISPLAY][audioWindowIndex].m_cameraId;
    streamRequestParam->windowId = streamInfoArray[MAIN_DISPLAY][audioWindowIndex].m_windowId;
    streamRequestParam->actualWindowId = audioWindowIndex;
    streamRequestParam->deviceName = streamInfoArray[MAIN_DISPLAY][audioWindowIndex].m_deviceName;
    streamRequestParam->displayType = MAIN_DISPLAY;
    streamRequestParam->audioStatus = true;
    streamRequestParam->payload = payloadLib->createDevCmdPayload(2);

    processRequest = applController->processStreamActivity(streamCommandType,
                                                           serverSessionInfo,
                                                           streamRequestParam);
    if (processRequest == 0)
    {
        DELETE_OBJ(streamRequestParam);
    }
}

void Layout::excludeDevAudio(quint16 audioWindowIndex)
{
    STREAM_COMMAND_TYPE_e streamCommandType = MAX_STREAM_COMMAND;
    quint8 processRequest = 0;
    StreamRequestParam *streamRequestParam = NULL;

    if(audioWindowIndex < MAX_CHANNEL_FOR_SEQ)
    {
        switch(streamInfoArray[MAIN_DISPLAY][audioWindowIndex].m_videoType)
        {
            case VIDEO_TYPE_LIVESTREAM:
                streamCommandType = EXCLUDE_AUDIO_COMMAND;
                break;

            case VIDEO_TYPE_PLAYBACKSTREAM:
                streamCommandType = AUDIO_PLABACK_STREAM_COMMAND;
                break;

            case VIDEO_TYPE_INSTANTPLAYBACKSTREAM:
                streamCommandType = AUDIO_INSTANTPLAYBACK_COMMAND;
                break;

            default:
                break;
        }
    }

    if(streamCommandType == MAX_STREAM_COMMAND)
    {
        return;
    }

    SERVER_SESSION_INFO_t serverSessionInfo;

    applController->GetServerSessionInfo(streamInfoArray[MAIN_DISPLAY][audioWindowIndex].m_deviceName, serverSessionInfo);

    payloadLib->setCnfgArrayAtIndex(0, streamInfoArray[MAIN_DISPLAY][audioWindowIndex].m_cameraId);
    payloadLib->setCnfgArrayAtIndex(1, streamInfoArray[MAIN_DISPLAY][audioWindowIndex].m_streamType);

    streamRequestParam = new StreamRequestParam();
    streamRequestParam->streamId = streamInfoArray[MAIN_DISPLAY][audioWindowIndex].m_streamId;
    streamRequestParam->channelId = streamInfoArray[MAIN_DISPLAY][audioWindowIndex].m_cameraId;
    streamRequestParam->windowId = streamInfoArray[MAIN_DISPLAY][audioWindowIndex].m_windowId;
    streamRequestParam->actualWindowId = audioWindowIndex;
    streamRequestParam->deviceName = streamInfoArray[MAIN_DISPLAY][audioWindowIndex].m_deviceName;
    streamRequestParam->displayType = MAIN_DISPLAY;
    streamRequestParam->audioStatus = false;
    streamRequestParam->payload = payloadLib->createDevCmdPayload(2);

    processRequest = applController->processStreamActivity(streamCommandType,
                                                           serverSessionInfo,
                                                           streamRequestParam);
    if (processRequest == 0)
    {
        DELETE_OBJ(streamRequestParam);
    }
}

void Layout::excludeAudioExcept(quint16 audioWindowIndex, bool status)
{
    for(quint16 index = 0; index < MAX_CHANNEL_FOR_SEQ; index++)
    {
        streamInfoArray[MAIN_DISPLAY][index].m_audioStatus = false;
    }

    if(status)
    {
        streamInfoArray[MAIN_DISPLAY][audioWindowIndex].m_audioStatus = true;
    }
}

void Layout::createCmdPayload(SET_COMMAND_e cmdType, quint8 totalFields, DISPLAY_TYPE_e displayType)
{
    quint16 windowIndex = currentDisplayConfig[displayType].selectedWindow;

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = cmdType;
    param->payload = payloadLib->createDevCmdPayload(totalFields);

    applController->processActivity(streamInfoArray[displayType][windowIndex].m_deviceName, DEVICE_COMM, param);
}

void Layout::audioResponse(bool isAudioInc, quint8 streamId)
{
    quint16 audioIndex;

    for(audioIndex = 0; audioIndex < MAX_CHANNEL_FOR_SEQ; audioIndex++)
    {
        if(streamInfoArray[MAIN_DISPLAY][audioIndex].m_streamId == streamId)
        {
            break;
        }
    }

    if(audioIndex != MAX_CHANNEL_FOR_SEQ)
    {
        excludeAudioExcept(audioIndex, isAudioInc);
    }

    if(isAudioInc == false)
    {
        if(startAudioInWindow != MAX_CHANNEL_FOR_SEQ)
        {
            includeDevAudio(startAudioInWindow);
        }
    }

    layoutCreator->updateWindowData(audioIndex);
}

void Layout::takeActionForCameraMenuSetting(quint16 windowIndex)
{
    if(m_analogPresetMenu ==  NULL)
    {
        /* PARASOFT: Memory Deallocated in slot SubObjectDelete */
        m_inVisibleWidget = new InvisibleWidgetCntrl(this->window());

        /* PARASOFT: Memory Deallocated in slot SubObjectDelete */
        m_analogPresetMenu = new MxAnalogPresetMenu(SCALE_WIDTH(800), SCALE_HEIGHT(800),
                                                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_deviceName,
                                                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_cameraId, this->window());
        connect (m_analogPresetMenu,
                 SIGNAL(sigObjectDelete()),
                 this,
                 SLOT(slotSubObjectDelete()));
    }
}

void Layout::takeActionForPTZControl(quint16 windowIndex)
{
    if (true == Layout::currentDisplayConfig[MAIN_DISPLAY].seqStatus)
    {
        if((ApplicationMode::getApplicationMode() == IDLE_MODE) && (currentModeType[MAIN_DISPLAY] != STATE_LOCAL_DECODING))
        {
            MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SEQUENCING_ON_MSG));
        }
        return;
    }

    if(m_ptzControl == NULL)
    {
        CAMERA_TYPE_e currentcamtype = applController->GetCameraType(streamInfoArray[MAIN_DISPLAY][windowIndex].m_deviceName,
                                                                     streamInfoArray[MAIN_DISPLAY][windowIndex].m_cameraId -1);
        if((currentcamtype == IP_CAMERA) || (currentcamtype == AUTO_ADD_IP_CAMERA))
        {
            payloadLib->setCnfgArrayAtIndex(0, streamInfoArray[MAIN_DISPLAY][windowIndex].m_cameraId);
            createCmdPayload(OTHR_SUP, 1);
        }
        else if(currentcamtype == ANALOG_CAMERA)
        {
            createPTZControl();
        }
    }
}

void Layout::createPTZControl()
{
    if(m_ptzControl == NULL)
    {
        quint16 windowIndex = currentDisplayConfig[MAIN_DISPLAY].selectedWindow;
        /* PARASOFT: Memory Deallocated in slot SubObjectDelete */
        m_inVisibleWidget = new InvisibleWidgetCntrl(this->window());

        /* PARASOFT: Memory Deallocated in slot SubObjectDelete */
        m_ptzControl = new PTZControl(SCALE_WIDTH(200), SCALE_HEIGHT(100),
                                      streamInfoArray[MAIN_DISPLAY][windowIndex].m_deviceName,
                                      streamInfoArray[MAIN_DISPLAY][windowIndex].m_cameraId, this->window());
        connect (m_ptzControl,
                 SIGNAL(sigObjectDelete()),
                 this,
                 SLOT(slotSubObjectDelete()));

        if(m_focusCntrlIndex != 255)
        {
            giveFocusToPTZControl(m_focusCntrlIndex);
        }
    }
}

void Layout::giveFocusToPTZControl(quint8 focusCntrlIndex)
{
    if(m_ptzControl != NULL)
    {
        m_ptzControl->giveFocusToControl(focusCntrlIndex);
        m_focusCntrlIndex = 255;
    }
    else
    {
        m_focusCntrlIndex = focusCntrlIndex;
    }
}

void Layout::updateDeviceState(QString deviceName, DEVICE_STATE_TYPE_e devState)
{
    if(m_viewCamera != NULL)
    {
        m_viewCamera->updateDeviceState(deviceName, devState);
    }
}

void Layout::startInstantPlayBackFeature(quint16 windowIndex)
{
    quint8 pendingRequest = 0;
    bool camRights = true;

    applController->GetCameraRights(streamInfoArray[MAIN_DISPLAY][windowIndex].m_deviceName,
                                    streamInfoArray[MAIN_DISPLAY][windowIndex].m_cameraId-1,
                                    PLAYBACK_CAM_LIST_TYPE, camRights);
    if(camRights == false)
    {
        MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(CMD_NO_PRIVILEGE));
        EPRINT(LAYOUT, "fail to start instant playback: no privilege to camera");
        return;
    }

    if((streamInfoArray[MAIN_DISPLAY][windowIndex].m_startRequestPending == false)
            && (currentModeType[MAIN_DISPLAY] == STATE_OTHER_MODE_NONE)
            && (m_processPendingForWindow[MAIN_DISPLAY] == false))
    {
        memcpy(&m_instantPlaybackData[windowIndex].backupWindowInfo, &currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex], sizeof(WINDOW_INFO_t));
        m_instantPlaybackData[windowIndex].currentCamInfo.defChannel = streamInfoArray[MAIN_DISPLAY][windowIndex].m_cameraId;
        snprintf(m_instantPlaybackData[windowIndex].currentCamInfo.deviceName, MAX_DEVICE_NAME_SIZE, "%s",
                 streamInfoArray[MAIN_DISPLAY][windowIndex].m_deviceName.toUtf8().constData());

        streamInfoArray[MAIN_DISPLAY][windowIndex].m_startInstantPlayback = true;

        if((streamInfoArray[MAIN_DISPLAY][windowIndex].m_videoType == VIDEO_TYPE_LIVESTREAM)
                && ((streamInfoArray[MAIN_DISPLAY][windowIndex].m_videoStatus == VIDEO_STATUS_RETRY)
                    || (streamInfoArray[MAIN_DISPLAY][windowIndex].m_videoStatus == VIDEO_STATUS_EVENTWAIT)
                    || (streamInfoArray[MAIN_DISPLAY][windowIndex].m_videoStatus == VIDEO_STATUS_ERROR)))
        {
            deAllocateWindow(MAIN_DISPLAY, windowIndex);
            processNextActionForWindow(MAIN_DISPLAY, windowIndex, false, true);
        }
        else
        {
            pendingRequest = freeWindow(MAIN_DISPLAY, windowIndex);
        }

        pendingInstantRequestCount[MAIN_DISPLAY][windowIndex] += pendingRequest;
    }
    else
    {
        EPRINT(LAYOUT, "fail to start instant playback");
    }
}

void Layout::stopInstantPlaybackFeature(quint16 windowIndex)
{
    quint8 pendingRequest = 0;

    if(windowIndex >= MAX_CHANNEL_FOR_SEQ)
    {
        return;
    }

    streamInfoArray[MAIN_DISPLAY][windowIndex].m_stopInstantPlayback = true;
    pendingRequest = freeWindow(MAIN_DISPLAY, windowIndex);
    pendingInstantRequestCount[MAIN_DISPLAY][windowIndex] += pendingRequest;
    if(pendingInstantRequestCount[MAIN_DISPLAY][windowIndex] == 0)
    {
        processNextActionForWindow(MAIN_DISPLAY, windowIndex, false, true);
    }
}

void Layout::getInstantPlaybackId(quint16 windowIndex)
{
    quint8 pendingStreamRequest = 0;

    memcpy(&currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[0], &m_instantPlaybackData[windowIndex].currentCamInfo, sizeof(CAM_INFO_t));
    pendingStreamRequest = startInstantPlayBackStream(windowIndex, QString(m_instantPlaybackData[windowIndex].currentCamInfo.deviceName),
                                                      m_instantPlaybackData[windowIndex].currentCamInfo.defChannel);
    pendingInstantRequestCount[MAIN_DISPLAY][windowIndex] += pendingStreamRequest;
    if(pendingInstantRequestCount[MAIN_DISPLAY][windowIndex] == 0)
    {
        processNextActionForWindow(MAIN_DISPLAY, windowIndex, false, true);
    }
}

void Layout::clearInstantPlaybackId(quint16 windowIndex)
{
    // If Audio start in Instant PB and come back then if Previous audio started then start Audio for live view
    if(lastEnableAudioWindow == windowIndex)
    {
        streamInfoArray[MAIN_DISPLAY][windowIndex].m_audioStatus = false;
    }

    if((m_instantpbToolbar != NULL) && (m_instantpbToolbar->getPbToolbarWinId() == windowIndex))
    {
        closeDeleteInstantPbToolBar();
    }

    memcpy(&currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex], &m_instantPlaybackData[windowIndex].backupWindowInfo, sizeof(WINDOW_INFO_t));
    memset(&m_instantPlaybackData[windowIndex].currentCamInfo, 0, sizeof(CAM_INFO_t));

    quint8 channelIndex = currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].currentChannel;
    startLiveStream(MAIN_DISPLAY, QString(currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[channelIndex].deviceName),
                    currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[channelIndex].defChannel, windowIndex);
    if(pendingInstantRequestCount[MAIN_DISPLAY][windowIndex] == 0)
    {
        processNextActionForWindow(MAIN_DISPLAY, windowIndex, false, true);
    }
}

quint8 Layout::startInstantPlayBackStream(quint16 windowIndex, QString deviceName, quint8 cameraId)
{
    quint8 totalCam = 0, pendingStreamRequest = 0;
    SERVER_SESSION_INFO_t serverSessionInfo;
    StreamRequestParam *streamRequestParam = NULL;

    if (false == applController->GetTotalCamera(deviceName, totalCam))
    {
        changeStreamStateOfWindow(MAIN_DISPLAY,
                                  windowIndex,
                                  deviceName,
                                  cameraId,
                                  MAX_STREAM_SESSION,
                                  VIDEO_TYPE_INSTANTPLAYBACKSTREAM,
                                  VIDEO_STATUS_ERROR,
                                  VIDEO_ERROR_DEVICEDISCONNECTED);
        return pendingStreamRequest;
    }

    if(cameraId > totalCam)
    {
        changeStreamStateOfWindow(MAIN_DISPLAY,
                                  windowIndex,
                                  deviceName,
                                  cameraId,
                                  MAX_STREAM_SESSION,
                                  VIDEO_TYPE_INSTANTPLAYBACKSTREAM,
                                  VIDEO_STATUS_ERROR,
                                  VIDEO_ERROR_DEVICEDISCONNECTED);
        return pendingStreamRequest;
    }

    if(false == applController->GetServerSessionInfo(deviceName, serverSessionInfo))
    {
        changeStreamStateOfWindow(MAIN_DISPLAY,
                                  windowIndex,
                                  deviceName,
                                  cameraId,
                                  MAX_STREAM_SESSION,
                                  VIDEO_TYPE_INSTANTPLAYBACKSTREAM,
                                  VIDEO_STATUS_ERROR,
                                  VIDEO_ERROR_DEVICEDISCONNECTED);
        return pendingStreamRequest;
    }

    payloadLib->setCnfgArrayAtIndex(0, cameraId);

    streamRequestParam = new StreamRequestParam();
    streamRequestParam->windowId = (windowIndex % windowPerPage[currentDisplayConfig[MAIN_DISPLAY].layoutId]);
    streamRequestParam->actualWindowId = windowIndex;
    streamRequestParam->channelId = cameraId;
    streamRequestParam->deviceName = deviceName;
    streamRequestParam->displayType = MAIN_DISPLAY;
    streamRequestParam->streamRequestType = INSTANT_PLAYBACK_STREAM_REQUEST;
    streamRequestParam->payload = payloadLib->createDevCmdPayload(1);

    pendingStreamRequest = applController->processStreamActivity(START_INSTANTPLAYBACK_COMMAND,
                                                                 serverSessionInfo,
                                                                 streamRequestParam);
    if(pendingStreamRequest != 0)
    {
        streamInfoArray[MAIN_DISPLAY][windowIndex].m_deviceName = deviceName;
        streamInfoArray[MAIN_DISPLAY][windowIndex].m_cameraId = cameraId;
        streamInfoArray[MAIN_DISPLAY][windowIndex].m_videoType = VIDEO_TYPE_INSTANTPLAYBACKSTREAM;
        streamInfoArray[MAIN_DISPLAY][windowIndex].m_errorType = VIDEO_ERROR_NONE;
        streamInfoArray[MAIN_DISPLAY][windowIndex].m_windowId = (windowIndex % windowPerPage[currentDisplayConfig[MAIN_DISPLAY].layoutId]);
        streamInfoArray[MAIN_DISPLAY][windowIndex].m_startRequestPending = true;
        streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbDirection = FORWARD_PLAY;
        streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamState = PB_MAX_PLAYBACK_STATE;
        streamInfoArray[MAIN_DISPLAY][windowIndex].m_audioStatus = false;

        changeStreamStateOfWindow(MAIN_DISPLAY,
                                  windowIndex,
                                  deviceName,
                                  cameraId,
                                  MAX_STREAM_SESSION,
                                  VIDEO_TYPE_INSTANTPLAYBACKSTREAM,
                                  VIDEO_STATUS_CONNECTING,
                                  VIDEO_ERROR_NONE);
    }
    else
    {
        DELETE_OBJ(streamRequestParam);
        changeStreamStateOfWindow(MAIN_DISPLAY,
                                  windowIndex,
                                  deviceName,
                                  cameraId,
                                  MAX_STREAM_SESSION,
                                  VIDEO_TYPE_INSTANTPLAYBACKSTREAM,
                                  VIDEO_STATUS_ERROR,
                                  VIDEO_ERROR_DEVICEDISCONNECTED);
    }

    return pendingStreamRequest;
}

quint8 Layout::stopInstantPlayBackStream(quint16 windowIndex)
{
    quint8 pendingStreamRequest = 0;
    StreamRequestParam *streamRequestParam  = NULL;

    if(streamInfoArray[MAIN_DISPLAY][windowIndex].m_streamId == MAX_STREAM_SESSION)
    {
        return pendingStreamRequest;
    }

    SERVER_SESSION_INFO_t serverSessionInfo;
    applController->GetServerSessionInfo(m_instantPlaybackData[windowIndex].currentCamInfo.deviceName, serverSessionInfo);

    payloadLib->setCnfgArrayAtIndex(0, streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamId);

    streamRequestParam = new StreamRequestParam();
    streamRequestParam->streamId = streamInfoArray[MAIN_DISPLAY][windowIndex].m_streamId;
    streamRequestParam->windowId = streamInfoArray[MAIN_DISPLAY][windowIndex].m_windowId;
    streamRequestParam->actualWindowId = windowIndex;
    streamRequestParam->channelId = m_instantPlaybackData[windowIndex].currentCamInfo.defChannel;
    streamRequestParam->deviceName = QString(m_instantPlaybackData[windowIndex].currentCamInfo.deviceName);
    streamRequestParam->displayType = MAIN_DISPLAY;
    streamRequestParam->payload = payloadLib->createDevCmdPayload(1);

    pendingStreamRequest = applController->processStreamActivity(STOP_INSTANTPLAYBACK_COMMAND,
                                                                 serverSessionInfo,
                                                                 streamRequestParam);
    if(pendingStreamRequest == 0)
    {
        DELETE_OBJ(streamRequestParam);
    }
    else
    {
        streamInfoArray[MAIN_DISPLAY][windowIndex].m_stopRequestPending = true;
    }

    return pendingStreamRequest;
}

quint8 Layout::pauseInstantPlaybackStream(quint16 windowIndex)
{
    quint8 pendingStreamRequest = 0;
    StreamRequestParam *streamRequestParam = NULL;
    if(streamInfoArray[MAIN_DISPLAY][windowIndex].m_streamId == MAX_STREAM_SESSION)
    {
        return pendingStreamRequest;
    }

    SERVER_SESSION_INFO_t serverSessionInfo;
    applController->GetServerSessionInfo(m_instantPlaybackData[windowIndex].currentCamInfo.deviceName, serverSessionInfo);

    payloadLib->setCnfgArrayAtIndex(0, streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamId);

    streamRequestParam = new StreamRequestParam();
    streamRequestParam->streamId = streamInfoArray[MAIN_DISPLAY][windowIndex].m_streamId;
    streamRequestParam->windowId = streamInfoArray[MAIN_DISPLAY][windowIndex].m_windowId;
    streamRequestParam->actualWindowId = windowIndex;
    streamRequestParam->channelId = m_instantPlaybackData[windowIndex].currentCamInfo.defChannel;
    streamRequestParam->deviceName = QString(m_instantPlaybackData[windowIndex].currentCamInfo.deviceName);
    streamRequestParam->displayType = MAIN_DISPLAY;
    streamRequestParam->payload = payloadLib->createDevCmdPayload(1);

    pendingStreamRequest = applController->processStreamActivity(PAUSE_INSTANTPLAYBACK_COMMAND,
                                                                 serverSessionInfo,
                                                                 streamRequestParam);
    if(pendingStreamRequest == 0)
    {
        DELETE_OBJ(streamRequestParam);
    }

    return pendingStreamRequest;
}

quint8 Layout::seekInsantPlaybackStream(quint16 windowIndex)
{
    quint8 pendingStreamRequest = 0;
    StreamRequestParam *streamRequestParam = NULL;

    if(streamInfoArray[MAIN_DISPLAY][windowIndex].m_streamId == MAX_STREAM_SESSION)
    {
        return pendingStreamRequest;
    }

    SERVER_SESSION_INFO_t serverSessionInfo;
    applController->GetServerSessionInfo(m_instantPlaybackData[windowIndex].currentCamInfo.deviceName, serverSessionInfo);

    payloadLib->setCnfgArrayAtIndex(0, streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamId);
    payloadLib->setCnfgArrayAtIndex(1, mSecToString(streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbCurrentTime));
    payloadLib->setCnfgArrayAtIndex(2, 1); /* Audio */
    payloadLib->setCnfgArrayAtIndex(3, streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbDirection);
    payloadLib->setCnfgArrayAtIndex(4, ++streamInfoArray[MAIN_DISPLAY][windowIndex].m_referenceFrameNo);

    streamRequestParam = new StreamRequestParam();
    streamRequestParam->streamId = streamInfoArray[MAIN_DISPLAY][windowIndex].m_streamId;
    streamRequestParam->windowId = streamInfoArray[MAIN_DISPLAY][windowIndex].m_windowId;
    streamRequestParam->actualWindowId = windowIndex;
    streamRequestParam->channelId = m_instantPlaybackData[windowIndex].currentCamInfo.defChannel;
    streamRequestParam->deviceName = QString(m_instantPlaybackData[windowIndex].currentCamInfo.deviceName);
    streamRequestParam->displayType = MAIN_DISPLAY;
    streamRequestParam->payload = payloadLib->createDevCmdPayload(5);

    pendingStreamRequest = applController->processStreamActivity(SEEK_INSTANTPLAYBACK_COMMAND,
                                                                 serverSessionInfo,
                                                                 streamRequestParam);
    if(pendingStreamRequest == 0)
    {
        DELETE_OBJ(streamRequestParam);
    }

    return pendingStreamRequest;
}

void Layout::createInstantPbToolbar(quint16 windowIndex)
{
    bool toolbarCreation = false;
    WIN_DIMENSION_INFO_t windowInfo = {0,0,0,0};

    if(m_instantpbToolbar != NULL)
    {
        if(m_instantpbToolbar->getPbToolbarWinId() != windowIndex)
        {
            closeDeleteInstantPbToolBar();
            toolbarCreation = true;
        }
    }
    else
    {
        toolbarCreation = true;
    }

    if (false == toolbarCreation)
    {
        return;
    }

    if(m_instantpbToolbar == NULL)
    {
        layoutCreator->getWindowDimensionInfo(windowIndex, &windowInfo);

        /* PARASOFT: Memory Deallocated in close Delete Instant PbToolBar */
        m_instantpbToolbar = new InstantPlayback(windowInfo.winStartx,
                                                 windowInfo.winStarty,
                                                 windowInfo.winWidth,
                                                 windowInfo.winHeight,
                                                 windowIndex,
                                                 layoutCreator->getPbtoolbarSize(windowIndex),
                                                 streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStartTime,
                                                 streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbEndTime,
                                                 streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbCurrentTime,
                                                 this);

        if(streamInfoArray[MAIN_DISPLAY][windowIndex].m_audioStatus == true)
        {
            m_instantpbToolbar->changeStateOfButton(INST_PB_TOOLBAR_ELE_MUTE_BTN, false);
        }
        else
        {
            m_instantpbToolbar->changeStateOfButton(INST_PB_TOOLBAR_ELE_MUTE_BTN, true);
        }

        if(streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbStreamState == PB_PLAY_STATE)
        {
            if(streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbDirection == FORWARD_PLAY)
            {
                m_instantpbToolbar->changeStateOfButton(INST_PB_TOOLBAR_ELE_PLAY_BTN, false);
            }
            else if(streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbDirection == BACKWARD_PLAY)
            {
                m_instantpbToolbar->changeStateOfButton(INST_PB_TOOLBAR_ELE_REV_PLAY_BTN, false);
            }
        }
        else
        {
            m_instantpbToolbar->changeStateOfButton(INST_PB_TOOLBAR_ELE_PLAY_BTN, true);
            m_instantpbToolbar->changeStateOfButton(INST_PB_TOOLBAR_ELE_REV_PLAY_BTN, true);
        }

        connect(m_instantpbToolbar,
                SIGNAL(sigInstantPbToolbarBtnClick(int,quint16,bool)),
                this,
                SLOT(slotInstantPbToolbarHandling(int,quint16,bool)));
        connect(m_instantpbToolbar,
                SIGNAL(sigSliderValueChanged(quint64,quint16)),
                this,
                SLOT(slotInstantPbToolbarSliderChanged(quint64,quint16)));
        ApplicationMode::setApplicationMode(ASYNC_PLAYBACK_TOOLBAR_MODE);
    }
}

void Layout::closeDeleteInstantPbToolBar()
{
    if(m_instantpbToolbar != NULL)
    {
        disconnect(m_instantpbToolbar,
                   SIGNAL(sigSliderValueChanged(quint64,quint16)),
                   this,
                   SLOT(slotInstantPbToolbarSliderChanged(quint64,quint16)));
        disconnect(m_instantpbToolbar,
                   SIGNAL(sigInstantPbToolbarBtnClick(int,quint16,bool)),
                   this,
                   SLOT(slotInstantPbToolbarHandling(int,quint16,bool)));
        DELETE_OBJ(m_instantpbToolbar);
        ApplicationMode::setApplicationMode(IDLE_MODE);
    }
}

void Layout::timerEvent(QTimerEvent * event)
{
    if(event->timerId() == m_timerIdForCosecBanner)
    {
        deleteCosecPopUpBanner();
    }
    else if(event->timerId() == m_timerIdForCosecPopup)
    {
        closeCosecPopupFeature();
    }
    else if(event->timerId() == localDecodingFeature.m_timerToShowBannerMessage)
    {
        if(localDecodingFeature.m_userName != "")
        {
           MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(LOCAL_DECODING_DISABLED) +
                                             " " + "by" + " " + localDecodingFeature.m_userName);
        }
        else
        {
           MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(LOCAL_DECODING_DISABLED));
        }
    }
    else if(event->timerId() == m_timerIdForRetry)
    {
        for(quint8 displayIndex = 0; displayIndex < deviceRespInfo.maxDisplayOutput; displayIndex++)
        {
            if((displayIndex < MAX_DISPLAY_TYPE) && (((currentModeType[displayIndex] == STATE_OTHER_MODE_NONE) ||
                 (currentModeType[displayIndex] == STATE_VIDEO_POPUP_FEATURE) ||
                 (currentModeType[displayIndex] == STATE_EXPAND_WINDOW))) && (isAnyWindowReplacing((DISPLAY_TYPE_e)displayIndex) == false))
            {
                m_isRetryingMode = true;
                if(currentModeType[displayIndex] == STATE_EXPAND_WINDOW)
                {
                    m_isRetryingModeinExpand = true;
                }
                if(currentModeType[displayIndex] == STATE_VIDEO_POPUP_FEATURE)
                {
                   videoPopupData.m_isRetryingMode = true;
                }
                changeCurrentMode(STATE_RETRY_PAGE, (DISPLAY_TYPE_e)displayIndex);
            }
        }
    }
    else if (event->timerId() == videoPopupData.m_timerIdForVideoPopup)
    {
        for(quint8 windowIndex = 0; windowIndex < MAX_POPUP_WINDOWS; windowIndex++)
        {
            if(videoPopupData.cameraEvtVideoPopData[windowIndex].m_cameraIndexForFeature == INVALID_CAMERA_INDEX)
            {
                continue;
            }

            if(videoPopupData.cameraEvtVideoPopData[windowIndex].m_timerCounter < videoPopupData.cameraEvtVideoPopData[windowIndex].m_videoPopUpTimeDuration)
            {
                videoPopupData.cameraEvtVideoPopData[windowIndex].m_timerCounter++;
                continue;
            }

            if(videoPopupData.m_isRetryingMode == false)
            {
                closeVideoPopupFeatureWindow(windowIndex);
            }
        }
    }
    else if(event->timerId() == autoPlayData.m_timerIdForAutoPlay)
    {
        quint16 windowIndexRange[2] = {0,MAX_CHANNEL_FOR_SEQ};
        bool    stopAutoPlayTimer = true;

        getFirstAndLastWindow(currentDisplayConfig[MAIN_DISPLAY].currPage, currentDisplayConfig[MAIN_DISPLAY].layoutId, windowIndexRange);
        for(quint16 windowIndex = windowIndexRange[0]; windowIndex <= windowIndexRange[1]; windowIndex++)
        {
            if((autoPlayData.autoPlayFeatureDataType[windowIndex].m_isApToolbarVisible == true) &&
                    (autoPlayData.autoPlayFeatureDataType[windowIndex].m_isNextEnable == true))
            {
                autoPlayData.autoPlayFeatureDataType[windowIndex].m_timerCount++;
                if(autoPlayData.autoPlayFeatureDataType[windowIndex].m_timerCount < MAX_AP_TIMER_COUNT)
                {
                    stopAutoPlayTimer = false;
                    layoutCreator->updateAPNextVideoText(windowIndex);
                }
                else
                {
                    slotAPCenterBtnClicked(AP_TOOLBAR_NEXT, windowIndex);
                }
            }
        }

        if(stopAutoPlayTimer)
        {
            if(autoPlayData.m_timerIdForAutoPlay != 0)
            {
                killTimer(autoPlayData.m_timerIdForAutoPlay);
                autoPlayData.m_timerIdForAutoPlay = 0;
            }
        }
    }
    else
    {
        for(quint8 displayIndex = 0; displayIndex < deviceRespInfo.maxDisplayOutput; displayIndex++)
        {
            if(m_timerIdForSequencing[displayIndex] == event->timerId())
            {
                changePage((DISPLAY_TYPE_e)displayIndex, NEXT_PAGE_NAVIGATION);
                return;
            }

            quint16 windowLimits[2] = {0,MAX_CHANNEL_FOR_SEQ};
            getFirstAndLastWindow(currentDisplayConfig[displayIndex].currPage, currentDisplayConfig[displayIndex].layoutId, windowLimits);

            for(quint16 windowIndex = windowLimits[0]; windowIndex <= windowLimits[1]; windowIndex++)
            {
                if(m_timerIdForWindowSequence[displayIndex][windowIndex] == event->timerId())
                {
                    if((m_liveViewToolbar != NULL) && (m_liveViewToolbar->getWindowIndex() == windowIndex))
                    {
                        slotCloseLiveViewToolbar();
                    }
                    changeWindowChannel((DISPLAY_TYPE_e)displayIndex, windowIndex);
                    return;
                }
            }
        }
    }
}

quint16 Layout::getStartAudioInWindow()
{
    return startAudioInWindow;
}

void Layout::getAudioOutConfig()
{
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                             AUDIO_OUT_TABLE_INDEX,
                                                             AUDIO_OUT_CNFG_FRM_INDEX,
                                                             AUDIO_OUT_CNFG_TO_INDEX,
                                                             AUDIO_OUT_CNFG_FRM_FIELD,
                                                             AUDIO_OUT_MAX_FIELD_NO,
                                                             AUDIO_OUT_MAX_FIELD_NO);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_GET_CFG;
    param->payload = payloadString;
    applController->processActivity(LOCAL_DEVICE_NAME, DEVICE_COMM, param);
}

void Layout::checkUserPriviledge()
{
    quint8  cameraId = streamInfoArray[MAIN_DISPLAY][m_audIndexToPlay].m_cameraId;
    QString deviceName = streamInfoArray[MAIN_DISPLAY][m_audIndexToPlay].m_deviceName;

    payloadLib->setCnfgArrayAtIndex(0, cameraId);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = CHK_PRIV;
    param->payload = payloadLib->createDevCmdPayload(1);
    applController->processActivity(deviceName, DEVICE_COMM, param);
}

void Layout::setlastEnableAudioWindow(quint16 windowIndex)
{
    lastEnableAudioWindow = windowIndex;
}

quint16 Layout::getlastEnableAudioWindow()
{
    return lastEnableAudioWindow;
}

bool Layout::isWindowWiseSequeningRunning (DISPLAY_TYPE_e displayType)
{
    for(quint16 windowIndex = 0; windowIndex < MAX_CHANNEL_FOR_SEQ; windowIndex++)
    {
        if((currentDisplayConfig[displayType].windowInfo[windowIndex].sequenceStatus) && (isMultipleChannelAssigned(displayType, windowIndex)))
        {
            return true;
        }
    }
    return false;
}

void Layout::slotWindowSelected(quint16 windowIndex)
{
    slotCloseLiveViewToolbar();

    if((currentModeType[MAIN_DISPLAY] == STATE_COSEC_FEATURE)
            && (cosecPopupEvtData.m_currentFeatureState == FEATURE_STATE_VIDEO_ON) && (m_cosecVideoPopupUser == NULL))
    {
        /* PARASOFT: Memory Deallocated in delete Cosec Popup Object */
        m_cosecVideoPopupUser = new CosecVideoPopupUser(this);
        connect (m_cosecVideoPopupUser,
                 SIGNAL(destroyed()),
                 this,
                 SLOT(slotCosecPopupSubObjDelete()));
        return;
    }

    if (true == currentDisplayConfig[MAIN_DISPLAY].seqStatus)
    {
        if ((ApplicationMode::getApplicationMode() == IDLE_MODE) && (currentModeType[MAIN_DISPLAY] != STATE_LOCAL_DECODING))
        {
            MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SEQUENCING_ON_MSG));
        }
        return;
    }

    if(((ApplicationMode::getApplicationMode() == IDLE_MODE) || (ApplicationMode::getApplicationMode() == PAGE_WITH_TOOLBAR_MODE)
        || (ApplicationMode::getApplicationMode() == ASYNC_PLAYBACK_TOOLBAR_MODE)) && (currentModeType[MAIN_DISPLAY] != STATE_EXPAND_WINDOW))
    {
        emit sigUnloadToolbar();

        setSelectedWindow(windowIndex);
        if(streamInfoArray[MAIN_DISPLAY][windowIndex].m_videoType == VIDEO_TYPE_PLAYBACKSTREAM)
        {
            if((m_pbToolbar == NULL) || (m_pbToolbar->getPbToolbarWinId() != windowIndex))
            {
                if(streamInfoArray[MAIN_DISPLAY][windowIndex].m_pbToolbarVisible == false)
                {
                    createPbToolbar(windowIndex);
                }
            }
        }
        else if(streamInfoArray[MAIN_DISPLAY][windowIndex].m_videoType == VIDEO_TYPE_INSTANTPLAYBACKSTREAM)
        {
            if((m_instantpbToolbar == NULL) || (m_instantpbToolbar->getPbToolbarWinId() != windowIndex))
            {
                createInstantPbToolbar(windowIndex);
            }
        }
    }
}

void Layout::slotLoadMenuListOptions(quint16 windowIndex)
{
    if((currentModeType[MAIN_DISPLAY] == STATE_COSEC_FEATURE)
            && (cosecPopupEvtData.m_currentFeatureState == FEATURE_STATE_VIDEO_ON) && (m_cosecVideoPopupUser == NULL))
    {
        /* PARASOFT: Memory Deallocated in delete Cosec Popup Object */
        m_cosecVideoPopupUser = new CosecVideoPopupUser(this);
        connect(m_cosecVideoPopupUser,
                SIGNAL(destroyed()),
                this,
                SLOT(slotCosecPopupSubObjDelete()));
        return;
    }

    if (true == currentDisplayConfig[MAIN_DISPLAY].seqStatus)
    {
        if ((ApplicationMode::getApplicationMode() == IDLE_MODE) && (currentModeType[MAIN_DISPLAY] != STATE_LOCAL_DECODING))
        {
            MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SEQUENCING_ON_MSG));
        }
        return;
    }

    if(((ApplicationMode::getApplicationMode() == IDLE_MODE) || (ApplicationMode::getApplicationMode() == PAGE_WITH_TOOLBAR_MODE)
        || (ApplicationMode::getApplicationMode() == ASYNC_PLAYBACK_TOOLBAR_MODE)) && (currentModeType[MAIN_DISPLAY] == STATE_OTHER_MODE_NONE))
    {
        emit sigUnloadToolbar();
        setSelectedWindow(windowIndex);
        if((m_liveViewToolbar != NULL) && (m_liveViewToolbar->getWindowIndex() != windowIndex))
        {
            slotCloseLiveViewToolbar();
        }

        openToolbarButtonClicked(windowIndex);
    }
}

void Layout::slotWindowDoubleClicked(quint16 windowIndex)
{
    mouseDoubleClicked(windowIndex);
}

void Layout::slotEnterKeyPressed(quint16 windowIndex)
{
    if((streamInfoArray[MAIN_DISPLAY][windowIndex].m_videoType == VIDEO_TYPE_LIVESTREAM)
            && (streamInfoArray[MAIN_DISPLAY][windowIndex].m_videoStatus == VIDEO_STATUS_RUNNING))
    {
        mouseDoubleClicked(windowIndex);
    }
    else if(streamInfoArray[MAIN_DISPLAY][windowIndex].m_videoType == VIDEO_TYPE_NONE)
    {
        addCameraButtonClicked(windowIndex);
    }
}

void Layout::slotWindowImageClicked(WINDOW_IMAGE_TYPE_e imageType, quint16 windowIndex)
{
    switch(imageType)
    {
        case WINDOW_CLOSE_BUTTON:
            closeButtonClicked(windowIndex);
            break;

        case WINDOW_ADD_CAMERA_BUTTON:
            if(streamInfoArray[MAIN_DISPLAY][windowIndex].m_videoType != VIDEO_TYPE_INSTANTPLAYBACKSTREAM)
                addCameraButtonClicked(windowIndex);
            break;

        case WINDOW_OPEN_TOOLBAR_BUTTON:
            if((m_liveViewToolbar != NULL) && (m_liveViewToolbar->getWindowIndex() != windowIndex))
            {
                slotCloseLiveViewToolbar();
            }
            openToolbarButtonClicked(windowIndex);
            break;

        default:
            break;
    }

    emit sigRaiseWidgetOnTopOfAll();
}

void Layout::slotAPCenterBtnClicked(quint8 index, quint16 windowId)
{
    resetAutoPlayFeatureInWindow(windowId);

    /* Play recording as per selection of toolba. If click previous or next then first free window and send req to server */
    switch(index)
    {
        case AP_TOOLBAR_RELOAD:
            streamInfoArray[MAIN_DISPLAY][windowId].m_pbToolbarVisible = false;
            startPlaybackStream(windowId, FORWARD_PLAY, PB_SPEED_NORMAL);
            break;

        case AP_TOOLBAR_PREVIOUS:
            streamInfoArray[MAIN_DISPLAY][windowId].m_pbToolbarVisible = false;
            pendingRequestForNxtPbRec[windowId] = AP_TOOLBAR_PREVIOUS;
            freeWindow(MAIN_DISPLAY, windowId, true);
            emit sigToolbarStyleChnageNotify(MAX_STYLE_TYPE);
            break;

        case AP_TOOLBAR_NEXT:
            streamInfoArray[MAIN_DISPLAY][windowId].m_pbToolbarVisible = false;
            pendingRequestForNxtPbRec[windowId] = AP_TOOLBAR_NEXT;
            freeWindow(MAIN_DISPLAY, windowId, true);
            emit sigToolbarStyleChnageNotify(MAX_STYLE_TYPE);
            break;

        default:
            break;
    }
}

void Layout::slotWindowImageHover(WINDOW_IMAGE_TYPE_e imageType, quint16 windowIndex, bool isHover)
{
    switch(imageType)
    {
        case WINDOW_CLOSE_BUTTON:
        {
            if (true == currentDisplayConfig[MAIN_DISPLAY].seqStatus)
            {
                if ((ApplicationMode::getApplicationMode() == IDLE_MODE) && (currentModeType[MAIN_DISPLAY] != STATE_LOCAL_DECODING))
                {
                    MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SEQUENCING_ON_MSG));
                }
                break;
            }

            if(((currentModeType[MAIN_DISPLAY] == STATE_COSEC_FEATURE) && (cosecPopupEvtData.m_currentFeatureState == FEATURE_STATE_VIDEO_ON))
                    ||((currentModeType[MAIN_DISPLAY] == STATE_VIDEO_POPUP_FEATURE)
                       && ((videoPopupData.m_currentFeatureState ==VIDEO_POPUP_STATE_START_VIDEO)
                        || (videoPopupData.m_currentFeatureState == VIDEO_POPUP_STATE_VIDEO_ON)
                        || (videoPopupData.m_currentFeatureState == VIDEO_POPUP_STATE_START_VIDEO_WAIT)))
                    || (((ApplicationMode::getApplicationMode() == IDLE_MODE) || (ApplicationMode::getApplicationMode() == PAGE_WITH_TOOLBAR_MODE)
                         || (ApplicationMode::getApplicationMode() == ASYNC_PLAYBACK_TOOLBAR_MODE))
                        && (currentModeType[MAIN_DISPLAY] == STATE_OTHER_MODE_NONE) && (m_liveViewToolbar == NULL)))
            {
                layoutCreator->updateImageMouseHover(imageType, windowIndex, isHover);
            }
        }
        break;

        case WINDOW_ADD_CAMERA_BUTTON:
        {
            if (true == currentDisplayConfig[MAIN_DISPLAY].seqStatus)
            {
                break;
            }

            if((ApplicationMode::getApplicationMode() == IDLE_MODE) && (currentModeType[MAIN_DISPLAY] == STATE_OTHER_MODE_NONE))
            {
                if(streamInfoArray[MAIN_DISPLAY][windowIndex].m_videoType != VIDEO_TYPE_INSTANTPLAYBACKSTREAM)
                {
                    layoutCreator->updateImageMouseHover(imageType, windowIndex, isHover);
                }
            }
        }
        break;

        case WINDOW_SEQUENCE_BUTTON:
        {
            if (true == currentDisplayConfig[MAIN_DISPLAY].seqStatus)
            {
                if (ApplicationMode::getApplicationMode() == IDLE_MODE)
                {
                    MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SEQUENCING_ON_MSG));
                }
                break;
            }

            if((ApplicationMode::getApplicationMode() == IDLE_MODE)
                    && ((currentModeType[MAIN_DISPLAY] == STATE_OTHER_MODE_NONE) || (currentModeType[MAIN_DISPLAY] == STATE_EXPAND_WINDOW)))
            {
                layoutCreator->updateImageMouseHover(imageType, windowIndex, isHover);
            }
        }
        break;

        case WINDOW_OPEN_TOOLBAR_BUTTON:
        {
            if (true == currentDisplayConfig[MAIN_DISPLAY].seqStatus)
            {
                break;
            }

            if((ApplicationMode::getApplicationMode() == IDLE_MODE) && (currentModeType[MAIN_DISPLAY] == STATE_OTHER_MODE_NONE)
                    && (streamInfoArray[MAIN_DISPLAY][windowIndex].m_videoType == VIDEO_TYPE_LIVESTREAM))
            {
                layoutCreator->updateImageMouseHover(imageType, windowIndex, isHover);
            }
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void Layout::slotSwapWindows(quint16 firstWindow, quint16 secondWindow)
{
    if(((ApplicationMode::getApplicationMode() == IDLE_MODE) || (ApplicationMode::getApplicationMode() == PAGE_WITH_TOOLBAR_MODE)
        || (ApplicationMode::getApplicationMode() == ASYNC_PLAYBACK_TOOLBAR_MODE))
            && (currentModeType[MAIN_DISPLAY] == STATE_OTHER_MODE_NONE) && (isAnyWindowReplacing(MAIN_DISPLAY) == false)
            && (currentDisplayConfig[MAIN_DISPLAY].seqStatus == false)
            && (currentDisplayConfig[MAIN_DISPLAY].windowInfo[firstWindow].sequenceStatus == false)
            && (currentDisplayConfig[MAIN_DISPLAY].windowInfo[secondWindow].sequenceStatus == false))
    {
        closeDeletePbToolBar();
        closeDeleteInstantPbToolBar();
        currentDisplayConfig[MAIN_DISPLAY].selectedWindow = secondWindow;
        layoutCreator->changeSelectedWindow(secondWindow);

        quint16 tempMaxWindow;
        applController->readMaxWindowsForDisplay(tempMaxWindow);
        if(secondWindow >= tempMaxWindow)
        {
            tempMaxWindow = tempMaxWindow + (secondWindow - tempMaxWindow) + 1;
            applController->writeMaxWindowsForDisplay(tempMaxWindow);
        }

        applController->readMaxWindowsForDisplay(tempMaxWindow);
        if(firstWindow >= tempMaxWindow)
        {
            tempMaxWindow = tempMaxWindow + (firstWindow - tempMaxWindow) + 1;
            applController->writeMaxWindowsForDisplay(tempMaxWindow);
        }

        WINDOW_INFO_t tempConfig;
        StreamRequestParam firstParam, secondParam;
        INSTANT_PLAYBACK_DATA_t tempInstantData;

        if(startAudioInWindow == firstWindow)
        {
            startAudioInWindow = secondWindow;
        }
        else if (startAudioInWindow == secondWindow)
        {
            startAudioInWindow = firstWindow;
        }

        if(lastEnableAudioWindow != MAX_CHANNEL_FOR_SEQ)
        {
            if(lastEnableAudioWindow == firstWindow)
            {
                lastEnableAudioWindow = secondWindow;
            }
            else if(lastEnableAudioWindow == secondWindow)
            {
                lastEnableAudioWindow = firstWindow;
            }
        }

		// Swap Microphone Active Window on Window swapping
		if(MAX_CHANNEL_FOR_SEQ != m_activeMicroPhoneWindow)
		{
			if(m_activeMicroPhoneWindow == firstWindow)
			{
				m_activeMicroPhoneWindow = secondWindow;
			}
			else if (m_activeMicroPhoneWindow == secondWindow)
			{
				m_activeMicroPhoneWindow = firstWindow;
			}
		}

        // For swapping window when AsyncPB started then for automatic start AsyncPB, Apply swap logic for Index swap
        if((m_currentRecNo[firstWindow] != MAX_REC_ALLOWED) && (m_currentRecNo[secondWindow] != MAX_REC_ALLOWED))
        {
            quint16 tempIndex = m_currentRecNo[firstWindow];
            m_currentRecNo[firstWindow] = m_currentRecNo[secondWindow];
            m_currentRecNo[secondWindow] = tempIndex;
        }
        else if(m_currentRecNo[firstWindow] != MAX_REC_ALLOWED)
        {
            m_currentRecNo[secondWindow] = m_currentRecNo[firstWindow];
            m_currentRecNo[firstWindow] = MAX_REC_ALLOWED;
        }
        else if(m_currentRecNo[secondWindow] != MAX_REC_ALLOWED)
        {
            m_currentRecNo[firstWindow] = m_currentRecNo[secondWindow];
            m_currentRecNo[secondWindow] = MAX_REC_ALLOWED;
        }

        if((m_searchBtnClickRecNo[firstWindow] != MAX_REC_ALLOWED) && (m_searchBtnClickRecNo[secondWindow] != MAX_REC_ALLOWED))
        {
            quint16 tempIndex = m_searchBtnClickRecNo[firstWindow];
            m_searchBtnClickRecNo[firstWindow] = m_searchBtnClickRecNo[secondWindow];
            m_searchBtnClickRecNo[secondWindow] = tempIndex;
        }
        else if(m_searchBtnClickRecNo[firstWindow] != MAX_REC_ALLOWED)
        {
            m_searchBtnClickRecNo[secondWindow] = m_searchBtnClickRecNo[firstWindow];
            m_searchBtnClickRecNo[firstWindow] = MAX_REC_ALLOWED;
        }
        else if(m_currentRecNo[secondWindow] != MAX_REC_ALLOWED)
        {
            m_searchBtnClickRecNo[firstWindow] = m_searchBtnClickRecNo[secondWindow];
            m_searchBtnClickRecNo[secondWindow] = MAX_REC_ALLOWED;
        }

        firstParam.displayType = MAIN_DISPLAY;
        firstParam.streamId = streamInfoArray[MAIN_DISPLAY][firstWindow].m_streamId;
        firstParam.actualWindowId = firstWindow;
        if(streamInfoArray[MAIN_DISPLAY][firstWindow].m_streamId != MAX_STREAM_SESSION)
        {
            firstParam.windowId = streamInfoArray[MAIN_DISPLAY][firstWindow].m_windowId;
        }
        else
        {
            firstParam.windowId = (firstWindow % windowPerPage[currentDisplayConfig[MAIN_DISPLAY].layoutId]);
            streamInfoArray[MAIN_DISPLAY][firstWindow].m_windowId = firstParam.windowId;
        }

        secondParam.displayType = MAIN_DISPLAY;
        secondParam.streamId = streamInfoArray[MAIN_DISPLAY][secondWindow].m_streamId;
        secondParam.actualWindowId = secondWindow;
        if(streamInfoArray[MAIN_DISPLAY][secondWindow].m_streamId != MAX_STREAM_SESSION)
        {
            secondParam.windowId = streamInfoArray[MAIN_DISPLAY][secondWindow].m_windowId;
        }
        else
        {
            secondParam.windowId = (secondWindow % windowPerPage[currentDisplayConfig[MAIN_DISPLAY].layoutId]);
            streamInfoArray[MAIN_DISPLAY][secondWindow].m_windowId = secondParam.windowId;
        }

        if(applController->swapWindows(MAIN_DISPLAY, firstParam, secondParam) == true)
        {
            WindowStreamInfo tempStreamInfo = streamInfoArray[MAIN_DISPLAY][firstWindow];
            WindowStreamInfo tempStreamInfo2 = streamInfoArray[MAIN_DISPLAY][secondWindow];
            PlaybackRecordData tempPbDtata = playbackRecordData[firstWindow];

            playbackRecordData[firstWindow] = playbackRecordData[secondWindow];
            playbackRecordData[secondWindow] = tempPbDtata;
            streamInfoArray[MAIN_DISPLAY][firstWindow] = tempStreamInfo2;
            streamInfoArray[MAIN_DISPLAY][secondWindow] = tempStreamInfo;

            memcpy(&tempConfig, &currentDisplayConfig[MAIN_DISPLAY].windowInfo[firstWindow], sizeof(WINDOW_INFO_t));
            memcpy(&currentDisplayConfig[MAIN_DISPLAY].windowInfo[firstWindow], &currentDisplayConfig[MAIN_DISPLAY].windowInfo[secondWindow], sizeof(WINDOW_INFO_t));
            memcpy(&currentDisplayConfig[MAIN_DISPLAY].windowInfo[secondWindow], &tempConfig, sizeof(WINDOW_INFO_t));

            memcpy(&tempInstantData, &m_instantPlaybackData[firstWindow], sizeof(INSTANT_PLAYBACK_DATA_t));
            memcpy(&m_instantPlaybackData[firstWindow], &m_instantPlaybackData[secondWindow], sizeof(INSTANT_PLAYBACK_DATA_t));
            memcpy(&m_instantPlaybackData[secondWindow], &tempInstantData, sizeof(INSTANT_PLAYBACK_DATA_t));

            layoutCreator->updateWindowData(firstWindow);
            layoutCreator->updateWindowData(secondWindow);

            emit sigToolbarStyleChnageNotify(MAX_STYLE_TYPE);
        }

        if((QApplication::overrideCursor() != NULL) && (QApplication::overrideCursor()->shape() == Qt::OpenHandCursor))
        {
            QApplication::setOverrideCursor(Qt::ArrowCursor);
        }
    }
    else if((currentDisplayConfig[MAIN_DISPLAY].seqStatus == true)
            && (ApplicationMode::getApplicationMode() == IDLE_MODE) && (currentModeType[MAIN_DISPLAY] != STATE_LOCAL_DECODING))
    {
        MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SEQUENCING_ON_MSG));
    }
    else if (((Layout::isMultipleChannelAssigned(MAIN_DISPLAY, firstWindow) == true)
              && (currentDisplayConfig[MAIN_DISPLAY].windowInfo[firstWindow].sequenceStatus == true)) ||
             ((Layout::isMultipleChannelAssigned(MAIN_DISPLAY, secondWindow) == true)
              && (currentDisplayConfig[MAIN_DISPLAY].windowInfo[secondWindow].sequenceStatus == true)))
    {
        MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(WINDOW_SEQUENCING_ON_MSG));
    }

    if((QApplication::overrideCursor() != NULL) && (QApplication::overrideCursor()->shape() == Qt::OpenHandCursor))
    {
        QApplication::setOverrideCursor(Qt::ArrowCursor);
    }
}

void Layout::slotDragStartStopEvent(bool isStart)
{
    if (true == currentDisplayConfig[MAIN_DISPLAY].seqStatus)
    {
        if((ApplicationMode::getApplicationMode() == IDLE_MODE) && (currentModeType[MAIN_DISPLAY] != STATE_LOCAL_DECODING))
        {
            MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SEQUENCING_ON_MSG));
        }
        return;
    }

    if(((ApplicationMode::getApplicationMode() == IDLE_MODE) || (ApplicationMode::getApplicationMode() == PAGE_WITH_TOOLBAR_MODE)
        || (ApplicationMode::getApplicationMode() == ASYNC_PLAYBACK_TOOLBAR_MODE))
            && (currentModeType[MAIN_DISPLAY] == STATE_OTHER_MODE_NONE) && (isAnyWindowReplacing(MAIN_DISPLAY) == false))
    {
        if(isStart)
        {
            QApplication::setOverrideCursor(Qt::OpenHandCursor);
        }
        else
        {
            QApplication::setOverrideCursor(Qt::ArrowCursor);
        }
    }
}

void Layout::slotChangeLayout(LAYOUT_TYPE_e index, DISPLAY_TYPE_e displayType, quint16 windowIndex, bool ifActualWindow, bool ifUpdateCurrentPage)
{
    changeLayout(displayType, index, windowIndex, ifActualWindow, ifUpdateCurrentPage);
}

void Layout::slotCloseCameraFeature(QObject * object)
{
    if(object == m_imageAppearenceSettings)
    {
        disconnect(m_imageAppearenceSettings,
                   SIGNAL(destroyed(QObject*)),
                   this,
                   SLOT(slotCloseCameraFeature(QObject*)));
        m_imageAppearenceSettings = NULL;
    }
    else if(object == m_privackMaskSettings)
    {
        disconnect(m_privackMaskSettings,
                   SIGNAL(destroyed()),
                   this,
                   SLOT(slotCloseCameraFeature(QObject*)));
        m_privackMaskSettings = NULL;
    }
    else if(object == m_motionDetectionSettings)
    {
        disconnect(m_motionDetectionSettings,
                   SIGNAL(destroyed(QObject*)),
                   this,
                   SLOT(slotCloseCameraFeature(QObject*)));
        m_motionDetectionSettings = NULL;
    }
    else if(object == m_ipMotionDetectionSettings)
    {
        disconnect(m_ipMotionDetectionSettings,
                   SIGNAL(destroyed(QObject*)),
                   this,
                   SLOT(slotCloseCameraFeature(QObject*)));
        m_ipMotionDetectionSettings = NULL;
    }
    else if(object == m_zoomFeatureControl)
    {
        disconnect(m_zoomFeatureControl,
                   SIGNAL(destroyed(QObject*)),
                   this,
                   SLOT(slotCloseCameraFeature(QObject*)));
        m_zoomFeatureControl = NULL;
    }

    if((videoPopupData.m_previousMainState == MAX_STATE_LAYOUT))
    {
        if(m_cosecHandleLaterFlag == true)
        {
            cosecPopupEvtData.m_previousSelectedWindow = analogFeatureDataType.m_previousSelectedWindow;
            cosecPopupEvtData.m_previousSelectedLayout = analogFeatureDataType.m_previousSelectedLayout;
            closeAnalogCameraFeature(false);
            m_cosecHandleLaterFlag = false;
            updateCosecPopupParam(false);
        }
        else
        {
            closeAnalogCameraFeature();
        }
    }
}

void Layout::slotPbToolbarHandling(int index, quint16 windowId, bool state)
{
    m_stepIsRunning = false;
    switch(index)
    {
        case PB_TOOLBAR_ELE_PLAY_BTN:
        {
            if(state == true)
            {
                startPlaybackStream(windowId, FORWARD_PLAY, PB_SPEED_NORMAL);
            }
            else
            {
                if(streamInfoArray[MAIN_DISPLAY][windowId].prevPlayState == PB_PLAY_STATE)
                {
                    streamInfoArray[MAIN_DISPLAY][windowId].prevPlayState=PB_MAX_PLAYBACK_STATE;
                }
                pausePlaybackStream(windowId);
            }
        }
        break;

        case PB_TOOLBAR_ELE_STOP_BTN:
        {
            if(startAudioInWindow != MAX_CHANNEL_FOR_SEQ)
            {
                includeExcludeAudio(startAudioInWindow);
            }

            freeWindow(MAIN_DISPLAY, windowId, true);
            emit sigToolbarStyleChnageNotify(MAX_STYLE_TYPE);
        }
        break;

        case PB_TOOLBAR_ELE_REV_PLAY_BTN:
        {
            if(state == true)
            {
                startPlaybackStream(windowId, BACKWARD_PLAY, PB_SPEED_NORMAL);
            }
            else
            {
                if(streamInfoArray[MAIN_DISPLAY][windowId].prevPlayState == PB_PLAY_STATE)
                {
                    streamInfoArray[MAIN_DISPLAY][windowId].prevPlayState = PB_MAX_PLAYBACK_STATE;
                }
                pausePlaybackStream(windowId);
            }
        }
        break;

        case PB_TOOLBAR_ELE_SLOW_BTN:
        {
            slowPlaybackStream(windowId);
        }
        break;

        case PB_TOOLBAR_ELE_FAST_BTN:
        {
            fastPlaybackStream(windowId);
        }
        break;

        case PB_TOOLBAR_ELE_PREVIOUS_BTN:
        {
            m_stepIsRunning = true;
            if(streamInfoArray[MAIN_DISPLAY][windowId].prevPlayState == PB_PLAY_STATE)
            {
                streamInfoArray[MAIN_DISPLAY][windowId].prevPlayState = PB_MAX_PLAYBACK_STATE;
            }
            stepPlaybackStream(windowId, BACKWARD_PLAY);
        }
        break;

        case PB_TOOLBAR_ELE_NEXT_BTN:
        {
            m_stepIsRunning = true;
            if(streamInfoArray[MAIN_DISPLAY][windowId].prevPlayState == PB_PLAY_STATE)
            {
                streamInfoArray[MAIN_DISPLAY][windowId].prevPlayState = PB_MAX_PLAYBACK_STATE;
            }
            stepPlaybackStream(windowId, FORWARD_PLAY);
        }
        break;

        case PB_TOOLBAR_ELE_MUTE_BTN:
        {
            if(m_isClientAudProcessRunning == true)
            {
                 MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(TWO_WAY_PLYBK_AUD_REQ_FAIL_PRO_CLNT_AUD));
            }
            else if(streamInfoArray[MAIN_DISPLAY][windowId].m_pbStreamId != MAX_STREAM_SESSION)
            {
                includeExcludeAudio(windowId);
            }
        }
        break;

        case PB_TOOLBAR_ELE_CLOSE:
        {
            closeDeletePbToolBar();
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void Layout::slotPlaybackPlayClick(PlaybackRecordData recData, QString devName, bool asyncPlayBackClick)
{
    quint16 windowIndex = currentDisplayConfig[MAIN_DISPLAY].selectedWindow;
    quint8 channelIndex = currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].currentChannel;
    quint16 tempMaxWindow;

    applController->readMaxWindowsForDisplay(tempMaxWindow);

    if((currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[channelIndex].deviceName[0] == '\0')
            && (playbackRecordData[windowIndex].deviceName == ""))
    {
        //stop default sequencing
        stopSequencing(MAIN_DISPLAY);

        //stop Window sequencing
        stopWindowSequencing(MAIN_DISPLAY);

        if(windowIndex >= tempMaxWindow)
        {
            applController->writeMaxWindowsForDisplay(windowIndex + 2);
        }

        // take flag into structure because in asyncPB and event search play button click comes here, so for Auto Play in AsyncPB take it
        m_currentRecNo[windowIndex] = (asyncPlayBackClick == true) ? recData.asyncPbIndex : MAX_REC_ALLOWED;
        getPlaybackId(&recData, windowIndex);
        playbackRecordData[windowIndex] = recData;
    }
    else
    {
        windowIndex = findEmptyWindowOnCurrentPageInCurrentStyle(MAIN_DISPLAY);
        if(windowIndex != MAX_CHANNEL_FOR_SEQ)
        {
            currentDisplayConfig[MAIN_DISPLAY].selectedWindow = windowIndex;
            layoutCreator->changeSelectedWindow(windowIndex, false);
            if(windowIndex < MAX_CHANNEL_FOR_SEQ)
            {
                if(windowIndex >= tempMaxWindow)
                {
                    applController->writeMaxWindowsForDisplay(windowIndex + 2);
                }

                playbackRecordData[windowIndex] = recData;

                //stop default sequencing
                stopSequencing(MAIN_DISPLAY);

                //stop Window sequencing
                stopWindowSequencing(MAIN_DISPLAY);

                // take flag into structure because in asyncPB and event search play button click comes here, so for Auto Play in AsyncPB take it
                m_currentRecNo[windowIndex] = (asyncPlayBackClick == true) ? recData.asyncPbIndex : MAX_REC_ALLOWED;
                getPlaybackId(&recData, windowIndex);
            }
        }
        else
        {
            windowIndex = findEmptyWindowInCurrentStyle(MAIN_DISPLAY);
            if(windowIndex != MAX_CHANNEL_FOR_SEQ)
            {
                if(windowIndex >= tempMaxWindow)
                {
                    applController->writeMaxWindowsForDisplay(windowIndex + 2);
                }

                playbackRecordData[windowIndex] = recData;

                //stop default sequencing
                stopSequencing(MAIN_DISPLAY);

                //stop Window sequencing
                stopWindowSequencing(MAIN_DISPLAY);

                // take flag into structure because in asyncPB and event search play button click comes here, so for Auto Play in AsyncPB take it
                m_currentRecNo[windowIndex] = (asyncPlayBackClick == true) ? recData.asyncPbIndex : MAX_REC_ALLOWED;
                shiftWindows(MAIN_DISPLAY, windowIndex, currentDisplayConfig[MAIN_DISPLAY].layoutId);
                changeCurrentMode(STATE_APPLY_NEW_STYLE, MAIN_DISPLAY);
            }
            else
            {
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(LAYOUT_NO_FREE_WINDOW_ERROR_MESSAGE));
            }
        }
    }

    m_searchBtnClickRecNo[windowIndex] = m_currentRecNo[windowIndex]; //  for updated variable value
    Q_UNUSED(devName);
}

void Layout::slotPbToolbarSliderChanged(quint64 currTime, quint16 winId)
{
    m_pbSliderChange = true;
    streamInfoArray[MAIN_DISPLAY][winId].m_pbCurrentTime = currTime;
    streamInfoArray[MAIN_DISPLAY][winId].m_iFrameNeeded = true;
    if(streamInfoArray[MAIN_DISPLAY][winId].m_pbStreamState == PB_PLAY_STATE)
    {
        startPlaybackStream(winId, streamInfoArray[MAIN_DISPLAY][winId].m_pbDirection, streamInfoArray[MAIN_DISPLAY][winId].m_pbSpeed);
    }
    else
    {
        stepPlaybackStream(winId, streamInfoArray[MAIN_DISPLAY][winId].m_pbDirection);
    }
}

void Layout::slotExitFromZoomFeature()
{
    disconnect(m_zoomFeatureControl,
               SIGNAL(sigExitFromZoomFeature()),
               this,
               SLOT(slotExitFromZoomFeature()));
    disconnect(m_zoomFeatureControl,
               SIGNAL(sigChangeLayout(LAYOUT_TYPE_e,DISPLAY_TYPE_e,quint16,bool,bool)),
               this,
               SLOT(slotChangeLayout(LAYOUT_TYPE_e,DISPLAY_TYPE_e,quint16,bool,bool)));
}

void Layout::slotChangePageFromLayout()
{
    emit sigChangePageFromLayout();
}

void Layout::slotSubObjectDelete()
{
    if(IS_VALID_OBJ(m_ptzControl))
    {
        disconnect (m_ptzControl,
                    SIGNAL(sigObjectDelete()),
                    this,
                    SLOT(slotSubObjectDelete()));
        DELETE_OBJ(m_ptzControl);
        DELETE_OBJ(m_inVisibleWidget);
    }

    if(IS_VALID_OBJ(m_analogPresetMenu))
    {
        disconnect (m_analogPresetMenu,
                    SIGNAL(sigObjectDelete()),
                    this,
                    SLOT(slotSubObjectDelete()));
        DELETE_OBJ(m_analogPresetMenu);
        DELETE_OBJ(m_inVisibleWidget);
    }
}

void Layout::slotInfoPageButtonClicked(int)
{
    layoutCreator->giveFocusToWindow();
}

void Layout::slotCosecPopupSubObjDelete()
{
    if(m_cosecVideoPopupUser != NULL)
    {
        disconnect (m_cosecVideoPopupUser,
                    SIGNAL(destroyed()),
                    this,
                    SLOT(slotCosecPopupSubObjDelete()));
        m_cosecVideoPopupUser = NULL;
    }
}

void Layout::slotChangePage(NAVIGATION_TYPE_e navigationType)
{
    if(navigationType == PREVIOUS_PAGE_NAVIGATION)
    {
        m_isPreviousPageNaviagtion = true;
    }

    changePage(MAIN_DISPLAY, navigationType);
}

void Layout::slotDispSettingApplyChanges(DISPLAY_TYPE_e displayType, DISPLAY_CONFIG_t displayConfig, STYLE_TYPE_e styleNo)
{
    DPRINT(LAYOUT, "display settings: [currWindow=%d], [newWindow=%d], [currPage=%d], [newPage=%d], [currLayout=%d], [newLayout=%d]",
           currentDisplayConfig[displayType].selectedWindow, displayConfig.selectedWindow,
           currentDisplayConfig[displayType].currPage, displayConfig.currPage, currentDisplayConfig[displayType].layoutId, displayConfig.layoutId);

    m_doubleClickModeFlag = false;
    m_previousSelectedWindow = MAX_CHANNEL_FOR_SEQ;

    applyNewStyle(displayType, displayConfig, styleNo);
}

void Layout::slotViewCameraPageClose(TOOLBAR_BUTTON_TYPE_e)
{
    if(m_viewCamera != NULL)
    {
        disconnect(m_viewCamera,
                   SIGNAL(sigSwapWindows(quint16,quint16)),
                   this,
                   SLOT(slotSwapWindows(quint16,quint16)));
        disconnect(m_viewCamera,
                   SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                   this,
                   SLOT(slotViewCameraPageClose(TOOLBAR_BUTTON_TYPE_e)));
        disconnect(m_viewCamera,
                   SIGNAL(sigStartStreamInWindow(DISPLAY_TYPE_e,QString,quint8,quint16)),
                   this,
                   SLOT(slotStartStreamInWindow(DISPLAY_TYPE_e,QString,quint8,quint16)));
        DELETE_OBJ(m_viewCamera);
        DELETE_OBJ(m_inVisibleWidget);

        quint16 windowIndex = currentDisplayConfig[MAIN_DISPLAY].selectedWindow;
        quint8 channelIndex = currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].currentChannel;

        if (currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[channelIndex].deviceName[0] != '\0')
        {
            playbackRecordData[currentDisplayConfig[MAIN_DISPLAY].selectedWindow].clearPlaybackInfo();
            emit sigToolbarStyleChnageNotify(MAX_STYLE_TYPE);
        }
    }
}

void Layout::slotStartStreamInWindow(DISPLAY_TYPE_e displayType, QString deviceName, quint8 channelId, quint16 windowId)
{
    quint16 tempMaxWindow;
    applController->readMaxWindowsForDisplay(tempMaxWindow);
    if(windowId >= tempMaxWindow)
    {
        tempMaxWindow = tempMaxWindow + (windowId - tempMaxWindow) + 1;
        applController->writeMaxWindowsForDisplay(tempMaxWindow);
    }

    for(quint16 index = 0; index < MAX_CHANNEL_FOR_SEQ; index++)
    {
        if(lastEnableAudioWindow == index)
        {
            if((m_AudioEnableCamId == channelId) && (m_AudioEnableDevice == deviceName))
            {
                lastEnableAudioWindow = MAX_CHANNEL_FOR_SEQ;
            }
        }
    }

    startLiveStream(displayType, deviceName, channelId, windowId);
}

void Layout::slotInstantPbToolbarHandling(int index, quint16 windowId, bool state)
{
    switch(index)
    {
        case INST_PB_TOOLBAR_ELE_PLAY_BTN:
        {
            if((m_instantPlaybackRequestSent[windowId] == false)
                    && (streamInfoArray[MAIN_DISPLAY][windowId].m_stopInstantPlayback == false)
                    && (streamInfoArray[MAIN_DISPLAY][windowId].m_startInstantPlayback == false))
            {
                m_instantPlaybackRequestSent[windowId] = true;
                if(state == false)
                {
                    pauseInstantPlaybackStream(windowId);
                }
                else if((streamInfoArray[MAIN_DISPLAY][windowId].m_pbStreamState == PB_PLAY_STATE)
                        || (streamInfoArray[MAIN_DISPLAY][windowId].m_pbStreamState == PB_PAUSE_STATE))
                {
                    streamInfoArray[MAIN_DISPLAY][windowId].m_pbDirection = FORWARD_PLAY;
                    seekInsantPlaybackStream(windowId);
                }
            }
        }
        break;

        case INST_PB_TOOLBAR_ELE_REV_PLAY_BTN:
        {
            if((m_instantPlaybackRequestSent[windowId] == false)
                    && (streamInfoArray[MAIN_DISPLAY][windowId].m_stopInstantPlayback == false)
                    && (streamInfoArray[MAIN_DISPLAY][windowId].m_startInstantPlayback == false))
            {
                m_instantPlaybackRequestSent[windowId] = true;
                if(state == false)
                {
                    pauseInstantPlaybackStream(windowId);
                }
                else if((streamInfoArray[MAIN_DISPLAY][windowId].m_pbStreamState == PB_PLAY_STATE)
                        || (streamInfoArray[MAIN_DISPLAY][windowId].m_pbStreamState == PB_PAUSE_STATE))
                {
                    streamInfoArray[MAIN_DISPLAY][windowId].m_pbDirection = BACKWARD_PLAY;
                    seekInsantPlaybackStream(windowId);
                }
            }
        }
        break;

        case INST_PB_TOOLBAR_ELE_STOP_BTN:
        {
            if((m_instantPlaybackRequestSent[windowId] == false)
                    && (streamInfoArray[MAIN_DISPLAY][windowId].m_stopInstantPlayback == false)
                    && (streamInfoArray[MAIN_DISPLAY][windowId].m_startInstantPlayback == false))
            {
                m_instantPlaybackRequestSent[windowId] = true;
                streamInfoArray[MAIN_DISPLAY][windowId].m_pbStreamState = PB_STOP_STATE;
                stopInstantPlaybackFeature(windowId);
            }
        }
        break;

        case INST_PB_TOOLBAR_ELE_MUTE_BTN:
        {
            if(m_isClientAudProcessRunning == true)
            {
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(TWO_WAY_PLYBK_AUD_REQ_FAIL_PRO_CLNT_AUD));
            }
            else if((streamInfoArray[MAIN_DISPLAY][windowId].m_pbStreamState == PB_PLAY_STATE)
                    && (m_instantPlaybackRequestSent[windowId] == false))
            {
                m_instantPlaybackRequestSent[windowId] = true;
                m_instantAudioWindow = windowId;
                seekInsantPlaybackStream(windowId);
            }
        }
        break;

        case INST_PB_TOOLBAR_ELE_CLOSE:
        {
            closeDeleteInstantPbToolBar();
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void Layout::slotInstantPbToolbarSliderChanged(quint64 currTime, quint16 windowId)
{
    if (true == m_instantPlaybackRequestSent[windowId])
    {
        return;
    }

    m_pbSliderChange = true;
    streamInfoArray[MAIN_DISPLAY][windowId].m_pbCurrentTime = currTime;

    if(streamInfoArray[MAIN_DISPLAY][windowId].m_pbStreamState == PB_PLAY_STATE)
    {
        m_instantPlaybackRequestSent[windowId] = true;
        seekInsantPlaybackStream(windowId);
    }
}

void Layout::slotCloseLiveViewToolbar()
{
    if(m_liveViewToolbar != NULL)
    {
        disconnect(m_liveViewToolbar,
                   SIGNAL(sigCloseLiveViewToolbar()),
                   this,
                   SLOT(slotCloseLiveViewToolbar()));
        disconnect(m_liveViewToolbar,
                   SIGNAL(sigToolbarButtonClicked(LIVEVIEW_TOOLBAR_BUTTON_TYPE_e,STATE_TYPE_e,quint16)),
                   this,
                   SLOT(slotLiveViewToolbarButtonClicked(LIVEVIEW_TOOLBAR_BUTTON_TYPE_e,STATE_TYPE_e,quint16)));
        DELETE_OBJ(m_liveViewToolbar);
        ApplicationMode::setApplicationMode(IDLE_MODE);
    }
}

void Layout::slotLiveViewToolbarButtonClicked(LIVEVIEW_TOOLBAR_BUTTON_TYPE_e index, STATE_TYPE_e state, quint16 windowIndex)
{
    if(index != LIVEVIEW_CLOSE_BUTTON)
    {
        if (currentDisplayConfig[MAIN_DISPLAY].seqStatus == true)
        {
            MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SEQUENCING_ON_MSG));
            slotCloseLiveViewToolbar();
            return;
        }

        if(index < LIVEVIEW_SEQUENCING_BUTTON)
        {
            if((Layout::isMultipleChannelAssigned(MAIN_DISPLAY, windowIndex) == true)
                    && (currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].sequenceStatus == true))
            {
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(WINDOW_SEQUENCING_ON_MSG));
                slotCloseLiveViewToolbar();
                return;
            }
        }
        else if (index == LIVEVIEW_EXPAND_BUTTON)
        {
            stopSequencing(MAIN_DISPLAY);
        }
    }

    slotCloseLiveViewToolbar();

    switch(index)
    {
        case LIVEVIEW_STREAMTYPE_BUTTON:
        {
            if(streamInfoArray[MAIN_DISPLAY][windowIndex].m_streamId == MAX_STREAM_SESSION)
            {
                LIVE_STREAM_TYPE_e streamType = (state == STATE_1) ? LIVE_STREAM_TYPE_MAIN : LIVE_STREAM_TYPE_SUB;
                startLiveStream(MAIN_DISPLAY,
                                streamInfoArray[MAIN_DISPLAY][windowIndex].m_deviceName,
                                streamInfoArray[MAIN_DISPLAY][windowIndex].m_cameraId,
                                windowIndex,
                                streamType);
            }
            else
            {
                changeLiveStreamType(windowIndex);
            }
        }
        break;

        case LIVEVIEW_PTZCONTROL_BUTTON:
        {
            takeActionForPTZControl(windowIndex);
        }
        break;

        case LIVEVIEW_SNAPSHOT_BUTTON:
        {
            takeSnapshot(windowIndex);
        }
        break;

        case LIVEVIEW_ZOOM_BUTTON:
        {
            showAnalogCameraFeature(streamInfoArray[MAIN_DISPLAY][windowIndex].m_cameraId,
                                    ZOOM_FEATURE, NULL, streamInfoArray[MAIN_DISPLAY][windowIndex].m_deviceName);
        }
        break;

        case LIVEVIEW_RECORDING_BUTTON:
        {
            startStopManualRecording(windowIndex);
        }
        break;

        case LIVEVIEW_AUDIO_BUTTON:
        {
            if(m_isClientAudProcessRunning == true)
            {
                m_audIndexToPlay = windowIndex;
                getAudioOutConfig();
            }
            else
            {
                includeExcludeAudio(windowIndex);
            }
        }
        break;

        case LIVEVIEW_MICROPHONE_BUTTON:
        {
            if(LOCAL_DEVICE_NAME != streamInfoArray[MAIN_DISPLAY][windowIndex].m_deviceName)
            {
                MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(CMD_AUD_SND_REQ_FAIL));
                break;
            }

            if(true == streamInfoArray[MAIN_DISPLAY][windowIndex].m_microPhoneStatus)
            {
                sendIncludeExcludeDevAudioInCmd(windowIndex, false);
                break;
            }

            if(MAX_CHANNEL_FOR_SEQ != m_activeMicroPhoneWindow)
            {
                /* Update m_activeMicroPhoneWindow to send Include Audio-IN command once response received */
                sendIncludeExcludeDevAudioInCmd(m_activeMicroPhoneWindow, false);
                m_activeMicroPhoneWindow = windowIndex;
            }
            else
            {
                sendIncludeExcludeDevAudioInCmd(windowIndex, true);
            }
        }
        break;

        case LIVEVIEW_INSTANTPLAYBACK_BUTTON:
        {
            startInstantPlayBackFeature(windowIndex);
        }
        break;

        case LIVEVIEW_SEQUENCING_BUTTON:
        {
            if(isPlaybackRunning())
            {
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(PB_RUNNING_MESSAGE));
                break;
            }

            if(state == STATE_1)
            {
                currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].sequenceStatus = true;
                currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].lastSequenceStatus = false;

                if(m_timerIdForWindowSequence[MAIN_DISPLAY][windowIndex] == 0)
                {
                    m_timerIdForWindowSequence[MAIN_DISPLAY][windowIndex] =
                            startTimer(currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].sequenceInterval * 1000);
                }

                if(m_timerIdForWindowSequence[MAIN_DISPLAY][windowIndex] <= 0)
                {
                    DPRINT(LAYOUT, "fail to start window sequencing from live toolbar");
                }
            }
            else
            {
                currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].sequenceStatus = false;
                currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].lastSequenceStatus = false;

                if(m_timerIdForWindowSequence[MAIN_DISPLAY][windowIndex] != 0)
                {
                    killTimer(m_timerIdForWindowSequence[MAIN_DISPLAY][windowIndex]);
                    m_timerIdForWindowSequence[MAIN_DISPLAY][windowIndex] = 0;
                }

                if(m_timerIdForWindowSequence[MAIN_DISPLAY][windowIndex] <= 0)
                {
                    DPRINT(LAYOUT, "stop window sequencing from live toolbar");
                }
            }

            // Audio On then stop Audio on Window Sequencing
            if((currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].sequenceStatus == true)
                    && (streamInfoArray[MAIN_DISPLAY][windowIndex].m_audioStatus == true))
            {
                startAudioInWindow = MAX_CHANNEL_FOR_SEQ;
                excludeDevAudio(windowIndex);
            }

            // Microphone On then stop Audio-In on Window Sequencing
            lastEnableAudioWindow = MAX_CHANNEL_FOR_SEQ;
            if((currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].sequenceStatus == true) && (m_activeMicroPhoneWindow == windowIndex))
            {
                sendIncludeExcludeDevAudioInCmd(windowIndex, false);
            }
        }
        break;

        case LIVEVIEW_EXPAND_BUTTON:
        {
            if(isPlaybackRunning())
            {
                MessageBanner::addMessageInBanner (ValidationMessage::getValidationMessage(PB_RUNNING_MESSAGE));
                break;
            }

            pauseWindowSequencing(MAIN_DISPLAY);
            expandWindow(windowIndex);
            if(startAudioInWindow != MAX_CHANNEL_FOR_SEQ)
            {
                m_expandMode = true;
                lastEnableAudioWindow = startAudioInWindow;
                includeExcludeAudio(lastEnableAudioWindow,true);
            }
        }
        break;

        case LIVEVIEW_MENUSETTINGS_BUTTON:
        {
            takeActionForCameraMenuSetting(windowIndex);
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void Layout::slotLoadProcessBar()
{
    emit sigLoadProcessBar();
}

void Layout::slotExitVideoPopupState()
{
    for(quint8 windowIndex = 0 ;windowIndex < MAX_POPUP_WINDOWS ;windowIndex++)
    {
        if(videoPopupData.cameraEvtVideoPopData[windowIndex].m_cameraIndexForFeature != INVALID_CAMERA_INDEX)
        {
            closeVideoPopupFeatureWindow(windowIndex);
        }
    }
}

void Layout::audioOnAfterPageChanges(quint16 windowIndex)
{
    if(true == m_isClientAudProcessRunning)
    {
        return;
    }

    if((lastEnableAudioWindow == windowIndex) && (currentModeType[MAIN_DISPLAY] != STATE_VIDEO_POPUP_FEATURE) &&
            (currentModeType[MAIN_DISPLAY] != STATE_EXPAND_WINDOW) && (m_expandMode == false) && (m_videoPopupMode == false))
    {
        /* because when comes from expand mode some time stream retry and some time directley comes,
         * so if already ON from collapse mode the not started again */
        if((lastEnableAudioWindow < MAX_CHANNEL_FOR_SEQ) && (streamInfoArray[MAIN_DISPLAY][lastEnableAudioWindow].m_audioStatus != true))
        {
            includeExcludeAudio(lastEnableAudioWindow);
        }
    }
}

void Layout::slotliveViewAudiostop()
{
    quint16 windowIndex;

    /* Here flag became false because after changes position stream become stop and then start again */
    if (startAudioInWindow < MAX_CHANNEL_FOR_SEQ)
    {
        windowIndex = startAudioInWindow;
        includeExcludeAudio(startAudioInWindow);
        windowIndex = (startAudioInWindow < MAX_CHANNEL_FOR_SEQ) ? startAudioInWindow : windowIndex;
        streamInfoArray[MAIN_DISPLAY][windowIndex].m_audioStatus = false;
    }
}

void Layout::startLocalDecoding(bool localDecodingF)
{
    if(localDecodingFeature.m_preLocalDecodingF == localDecodingF)
    {
        return;
    }

    localDecodingFeature.m_preLocalDecodingF = localDecodingF;
    if(localDecodingFeature.m_timerToShowBannerMessage != 0)
    {
        killTimer(localDecodingFeature.m_timerToShowBannerMessage);
        localDecodingFeature.m_timerToShowBannerMessage = 0;
        localDecodingFeature.m_userName = "";
    }

    for(quint8 displayType=MAIN_DISPLAY; displayType<MAX_DISPLAY_TYPE; displayType++)
    {
        resumeSequencing((DISPLAY_TYPE_e)displayType);
        resumeWindowSequencing((DISPLAY_TYPE_e)displayType);

        if (currentModeType[displayType] == STATE_LOCAL_DECODING)
        {
            changeCurrentMode(STATE_CHANGE_PAGE,(DISPLAY_TYPE_e)displayType,false);
            startStreamForCurrentPage ((DISPLAY_TYPE_e)displayType);
            updateWindowDataForLocalDecording();
        }
    }
}

void Layout::stopLocalDecoding(bool localDecodingF, QString userName)
{
    if(localDecodingFeature.m_preLocalDecodingF == localDecodingF)
    {
        return;
    }

    localDecodingFeature.m_userName = userName;
    localDecodingFeature.m_preLocalDecodingF = localDecodingF;
    if(m_liveViewToolbar != NULL)
    {
        slotCloseLiveViewToolbar();
    }

    if(m_viewCamera!=NULL)
    {
        slotViewCameraPageClose(MAX_TOOLBAR_BUTTON);
    }

    slotSubObjectDelete();

    if (localDecodingF == true)
    {
        return;
    }

    for(quint8 displayType = MAIN_DISPLAY; displayType < MAX_DISPLAY_TYPE; displayType++)
    {
        switch(currentModeType[displayType])
        {
            case STATE_OTHER_MODE_NONE:
                startLocalDecodingFeature((DISPLAY_TYPE_e)displayType);
                break;

            case STATE_COSEC_FEATURE:
                m_cosecHandleLaterFlag = false;
                closeCosecPopupFeature();
                localDecodingFeature.pendingRequestForLoalDecoding = true;
                break;

            case STATE_ANALOG_FEATURE:
                hideAnalogCameraFeature();
                localDecodingFeature.pendingRequestForLoalDecoding = true;
                break;

            case STATE_VIDEO_POPUP_FEATURE:
                localDecodingFeature.pendingRequestForLoalDecoding = true;
                slotExitVideoPopupState();
                break;

            case STATE_EXPAND_WINDOW:
                collapseWindow();
                localDecodingFeature.pendingRequestForLoalDecoding = true;
                break;

            default:
                localDecodingFeature.pendingRequestForLoalDecoding = true;
                break;
        }
    }
}

void Layout::updateWindowDataForLocalDecording()
{
    quint16 windowLimit[2] = {0,MAX_CHANNEL_FOR_SEQ};
    getFirstAndLastWindow(currentDisplayConfig[MAIN_DISPLAY].currPage, currentDisplayConfig[MAIN_DISPLAY].layoutId, windowLimit);

    for(quint16 index = windowLimit[0]; index <= windowLimit[1] ; index++)
    {
        if((Layout::currentDisplayConfig[MAIN_DISPLAY].windowInfo[index].camInfo[0].defChannel != INVALID_CAMERA_INDEX)
                && (Layout::streamInfoArray[MAIN_DISPLAY][index].m_deviceName == ""))
        {
            layoutCreator->changeWindowType(index, WINDOW_TYPE_LOCAL_DECORDING);
            layoutCreator->updateWindowData (index);
            QString cameraName = applController->GetCameraNameOfDevice(Layout::currentDisplayConfig[MAIN_DISPLAY].windowInfo[index].camInfo[0].deviceName,
                    (Layout::currentDisplayConfig[MAIN_DISPLAY].windowInfo[index].camInfo[0].defChannel - 1));
            layoutCreator->updateWindowHeaderText(index, (applController->GetDispDeviceName(Layout::currentDisplayConfig[MAIN_DISPLAY].windowInfo[index].camInfo[0].deviceName)
                                                  + ":" + INT_TO_QSTRING(Layout::currentDisplayConfig[MAIN_DISPLAY].windowInfo[index].camInfo[0].defChannel)
                                                  + ":" + cameraName));
        }
        else
        {
            layoutCreator->changeWindowType(index, WINDOW_TYPE_LAYOUT);
            layoutCreator->updateWindowData (index);
        }
    }
}

void Layout::resetLocalDecordingData()
{
    localDecodingFeature.m_preLocalDecodingF = 255;
    localDecodingFeature.pendingRequestForLoalDecoding = false;
    localDecodingFeature.m_timerToShowBannerMessage = 0;
}

void Layout::startLocalDecodingFeature(DISPLAY_TYPE_e displayType)
{
    //Close live view tool bar
    if((m_liveViewToolbar != NULL))
    {
        slotCloseLiveViewToolbar();
    }

    pauseSequencing((DISPLAY_TYPE_e)displayType);
    pauseWindowSequencing((DISPLAY_TYPE_e)displayType);

    localDecodingFeature.pendingRequestForLoalDecoding = false;
    changeCurrentMode(STATE_LOCAL_DECODING,(DISPLAY_TYPE_e)displayType,false);
    stopLiveStreamLocalDecoding((DISPLAY_TYPE_e)displayType);

    if(localDecodingFeature.m_timerToShowBannerMessage == 0)
    {
        localDecodingFeature.m_timerToShowBannerMessage = startTimer(10000);
    }
}

void Layout::stopLiveStreamLocalDecoding(DISPLAY_TYPE_e displayType)
{
    quint16 windowLimit[2] = {0,MAX_CHANNEL_FOR_SEQ};
    quint8 pendingStreamRequest = 0;

    if ((displayType < MAIN_DISPLAY) || (displayType >= MAX_DISPLAY_TYPE))
    {
        return;
    }

    changeCurrentMode(STATE_OTHER_MODE_PROCESSING_STOP_REQUEST, displayType, false);
    getFirstAndLastWindow(currentDisplayConfig[displayType].currPage, currentDisplayConfig[displayType].layoutId, windowLimit);

    for(quint16 windowIndex = windowLimit[0]; windowIndex <= windowLimit[1]; windowIndex++)
    {
        pendingStreamRequest = freeWindow(displayType, windowIndex, (streamInfoArray[displayType][windowIndex].m_videoType == VIDEO_TYPE_INSTANTPLAYBACKSTREAM));
        if(streamInfoArray[displayType][windowIndex].m_videoType == VIDEO_TYPE_PLAYBACKSTREAM)
        {
            m_pendingRequestCount[displayType] += pendingStreamRequest;
        }
    }

    if(m_pendingRequestCount[displayType] == 0)
    {
        processNextActionForOtherMode(displayType);
    }
}

void Layout::playNextRecord(PlaybackRecordData NextReco, quint16 windowIndex,quint16 curRec)
{
    pendingRequestForNxtPbRec[windowIndex] = MAX_AP_TOOLBAR_BTN;
    m_currentRecNo[windowIndex] = curRec;
    m_searchBtnClickRecNo[windowIndex] = curRec;
    playbackRecordData[windowIndex] = NextReco;
    getPlaybackId(&NextReco, windowIndex);
}

void Layout::clearsearchBtnClickResult()
{
    for(quint16 windowIndex = 0; windowIndex < MAX_CHANNEL_FOR_SEQ; windowIndex++ )
    {
       m_searchBtnClickRecNo[windowIndex] = MAX_REC_ALLOWED;
    }
}

void Layout::UpdateAsyncCenterBtn(bool prevFlag,bool nextFlag, quint16 windowIndex)
{
    autoPlayData.autoPlayFeatureDataType[windowIndex].m_isPrevEnable = prevFlag;
    autoPlayData.autoPlayFeatureDataType[windowIndex].m_isNextEnable = nextFlag;
    layoutCreator->updateApToolbar(windowIndex);
}

void Layout::stopAutoPlayFeature()
{
    quint16 windowIndexRange[2] = {0,MAX_CHANNEL_FOR_SEQ};

    if(autoPlayData.m_timerIdForAutoPlay != 0)
    {
        killTimer(autoPlayData.m_timerIdForAutoPlay);
        autoPlayData.m_timerIdForAutoPlay = 0;
    }

    getFirstAndLastWindow(currentDisplayConfig[MAIN_DISPLAY].currPage, currentDisplayConfig[MAIN_DISPLAY].layoutId, windowIndexRange);

    for(quint16 windowIndex = windowIndexRange[0]; windowIndex <= windowIndexRange[1]; windowIndex++)
    {
        resetAutoPlayFeatureInWindow(windowIndex);
    }
}

void Layout::resetAutoPlayFeatureInWindow(quint16 windowIndex)
{
    autoPlayData.autoPlayFeatureDataType[windowIndex].m_timerCount = 0;
    autoPlayData.autoPlayFeatureDataType[windowIndex].m_isApToolbarVisible = false;
    autoPlayData.autoPlayFeatureDataType[windowIndex].m_isPrevEnable = true;
    autoPlayData.autoPlayFeatureDataType[windowIndex].m_isNextEnable = true;

    layoutCreator->updateApToolbar(windowIndex);
}

void Layout::navigationKeyPressed(QKeyEvent *event)
{
    layoutCreator->navigationKeyPressed (event);
}

void Layout::updateNextToolbarPageIndex(TOOLBAR_BUTTON_TYPE_e Index)
{
    m_nextToolbarPage = Index;
}

void Layout::changeWinAudioStatus(quint16 windowIndex)
{
    if(streamInfoArray[MAIN_DISPLAY][windowIndex].m_audioStatus ==true)
    {
        lastEnableAudioWindow = windowIndex;
        streamInfoArray[MAIN_DISPLAY][windowIndex].m_audioStatus =false;
    }
}

void Layout::sendIncludeExcludeDevAudioInCmd(quint16 windowIndex, bool includeExcludeDevAudioFlag)
{
    quint8 pendingStreamRequest = 0;
	StreamRequestParam *streamRequestParam = NULL;
	SERVER_SESSION_INFO_t serverSessionInfo;

    if(streamInfoArray[MAIN_DISPLAY][windowIndex].m_microPhoneStatus == includeExcludeDevAudioFlag)
	{
        EPRINT(LAYOUT, "ignore audio-in cmd: [windowIndex=%d], [microPhoneStatus=%d]", windowIndex, streamInfoArray[MAIN_DISPLAY][windowIndex].m_microPhoneStatus);
		return;
	}

    if((streamInfoArray[MAIN_DISPLAY][windowIndex].m_videoStatus != VIDEO_STATUS_RUNNING) && (true == includeExcludeDevAudioFlag))
    {
        EPRINT(LAYOUT, "ignore audio-in cmd: [windowIndex=%d], [videoStatus=%d]", windowIndex, streamInfoArray[MAIN_DISPLAY][windowIndex].m_videoStatus);
        m_activeMicroPhoneWindow = MAX_CHANNEL_FOR_SEQ;
        return;
    }

    DPRINT(LAYOUT, "send audio-in command: [windowIndex=%d], [microPhoneStatus=%d]", windowIndex, includeExcludeDevAudioFlag);

    /* Update Microphone status FALSE by default */
    for(quint16 index = 0; index < MAX_CHANNEL_FOR_SEQ; index++)
    {
        streamInfoArray[MAIN_DISPLAY][index].m_microPhoneStatus = false;
    }

    /* update window icon */
    layoutCreator->updateWindowData(windowIndex);

    m_activeMicroPhoneWindow = (false == includeExcludeDevAudioFlag) ? MAX_CHANNEL_FOR_SEQ : windowIndex;
    applController->GetServerSessionInfo(streamInfoArray[MAIN_DISPLAY][windowIndex].m_deviceName, serverSessionInfo);

    payloadLib->setCnfgArrayAtIndex(0, 1); /* 0-send to device, 1-send to camera */
    payloadLib->setCnfgArrayAtIndex(1, streamInfoArray[MAIN_DISPLAY][windowIndex].m_cameraId);

	streamRequestParam = new StreamRequestParam();
    streamRequestParam->streamId = streamInfoArray[MAIN_DISPLAY][windowIndex].m_streamId;
    streamRequestParam->channelId = streamInfoArray[MAIN_DISPLAY][windowIndex].m_cameraId;
    streamRequestParam->windowId = streamInfoArray[MAIN_DISPLAY][windowIndex].m_windowId;
    streamRequestParam->actualWindowId = windowIndex;
    streamRequestParam->deviceName = streamInfoArray[MAIN_DISPLAY][windowIndex].m_deviceName;
    streamRequestParam->displayType = MAIN_DISPLAY;
    streamRequestParam->audioStatus = includeExcludeDevAudioFlag;
    streamRequestParam->payload = payloadLib->createDevCmdPayload(2);

    pendingStreamRequest = applController->processStreamActivity(INCL_EXCL_AUDIO_IN_COMMAND, serverSessionInfo, streamRequestParam);
    if (pendingStreamRequest == 0)
    {
        DELETE_OBJ(streamRequestParam);
    }
}
