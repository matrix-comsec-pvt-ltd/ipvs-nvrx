#ifndef LAYOUT_H
#define LAYOUT_H

#include <QWidget>
#include <QTimer>

#include "EnumFile.h"
#include "Controls/WindowStreamInfo.h"
#include "ApplController.h"
#include "DataStructure.h"
#include "Controls/PlaybackRecordData.h"
#include "PayloadLib.h"
#include "ConfigPages/CameraSettings/ImageAppearenceSettings.h"
#include "ConfigPages/CameraSettings/PrivacyMaskSettings.h"
#include "ConfigPages/CameraSettings/MotionDetectionSettings.h"
#include "ConfigPages/CameraSettings/IpMotionDetectionSettings.h"
#include "Controls/LayoutCreator.h"
#include "PlaybackSearch.h"
#include "Controls/PlaybackControl/PbToolbar.h"
#include "Controls/MenuButtonList.h"
#include "Controls/ZoomFeatureControl.h"
#include "Controls/PTZControl.h"
#include "Controls/InfoPage.h"
#include "CosecVideoPopupUser.h"
#include "Controls/ViewCamera/ViewCamera.h"
#include "Controls/InvisibleWidgetCntrl.h"
#include "Controls/IntantPlaybackControl/InstantPlayback.h"
#include "Controls/LiveViewToolbar/LiveViewToolbar.h"
#include "Controls/TextLabel.h"
#include "Controls/PTZControl/MxAnalogPresetMenu.h"

typedef struct _ANALOG_FEATURE_VARIABLE_t
{
    quint16                 m_windowIndexForFeature;
    quint16                 m_previousSelectedWindow;
    LAYOUT_TYPE_e           m_previousSelectedLayout;
    LAYOUT_TYPE_e           m_previousBeforeOneXOne;
    quint8                  m_cameraIndexForFeature;
    QString                 m_deviceNameForFeature;
    WINDOW_INFO_t           m_backupWindowInfo;
    CAMERA_FEATURE_TYPE_e   m_featureType;
    FEATURE_STATE_e         m_currentFeatureState;
    void*                   m_featureData;
    void*                   m_featureConfigData;

    _ANALOG_FEATURE_VARIABLE_t() : m_windowIndexForFeature(0), m_previousSelectedWindow(0), m_previousSelectedLayout(MAX_LAYOUT),
        m_previousBeforeOneXOne(MAX_LAYOUT), m_cameraIndexForFeature(0), m_deviceNameForFeature(""), m_featureType(MAX_CAMERA_FEATURE),
        m_currentFeatureState(FEATURE_STATE_NONE), m_featureData(NULL), m_featureConfigData(NULL)
    {

    }
}ANALOG_FEATURE_VARIABLE_t;

typedef struct _COSEC_POPUP_FEATURE_VARIABLE_t
{
    quint16             m_windowIndexForFeature;
    quint16             m_previousSelectedWindow;
    WINDOW_INFO_t       m_backupWindowInfo;
    FEATURE_STATE_e     m_currentFeatureState;
    LAYOUT_TYPE_e       m_previousSelectedLayout;
    LAYOUT_TYPE_e       m_previousBeforeOneXOne;
    bool                m_nextCosecPending;
    bool                m_isCosecHandlingStarted;
    QString             m_userName;
    quint32             m_eventPopUpTime;
    QString             m_doorName;
    quint8              m_eventStatusCode;
    quint8              m_cameraIndexForFeature;
    QString             m_userId;
    QString             m_nextUserName;
    quint32             m_nextEventPopUpTime;
    QString             m_nextDoorName;
    quint8              m_nextEventStatusCode;
    quint8              m_nextCameraIndexForFeature;

    _COSEC_POPUP_FEATURE_VARIABLE_t() : m_windowIndexForFeature(0), m_previousSelectedWindow(0), m_currentFeatureState(FEATURE_STATE_NONE),
        m_previousSelectedLayout(MAX_LAYOUT), m_previousBeforeOneXOne(MAX_LAYOUT), m_nextCosecPending(false), m_isCosecHandlingStarted(false),
        m_userName(""), m_eventPopUpTime(0), m_doorName(""), m_eventStatusCode(0), m_cameraIndexForFeature(0), m_userId(""), m_nextUserName(""),
        m_nextEventPopUpTime(0), m_nextDoorName(""), m_nextEventStatusCode(0), m_nextCameraIndexForFeature(0)
    {

    }
}COSEC_POPUP_FEATURE_VARIABLE_t;

typedef struct _VIDEO_POPUP_FEATURE_VARIABLE_t
{
    QString     m_deviceName;
    quint16     m_eventType;
    quint16     m_windowIndexForFeature;
    quint8      m_cameraIndexForFeature;
    quint32     m_timerCounter;
    quint16     m_videoPopUpTimeDuration;
    bool        m_replaceWindowVideoPopup;

    _VIDEO_POPUP_FEATURE_VARIABLE_t() : m_deviceName(""), m_eventType(0), m_windowIndexForFeature(0), m_cameraIndexForFeature(0),
        m_timerCounter(0), m_videoPopUpTimeDuration(0), m_replaceWindowVideoPopup(false)
    {

    }
}VIDEO_POPUP_FEATURE_VARIABLE_t;

typedef enum
{
    STATE_OTHER_MODE_NONE,
    STATE_OTHER_MODE_PROCESSING_START_REQUEST,
    STATE_OTHER_MODE_PROCESSING_STOP_REQUEST,
    STATE_OTHER_MODE_PROCESSING_REPLACE_REQUEST,
    STATE_APPLY_NEW_STYLE,
    STATE_CHANGE_PAGE,
    STATE_REFRESH_PAGE,
    STATE_RETRY_PAGE,
    STATE_SYNC_PLAYBACK_START_MODE,
    STATE_SYNC_PLAYBACK_STOP_MODE,
    STATE_SYNC_PLAYBACK_MODE,
    STATE_ANALOG_FEATURE,
    STATE_COSEC_FEATURE,
    STATE_EXPAND_WINDOW_START_MODE,
    STATE_COLLAPSE_WINDOW_START_MODE,
    STATE_EXPAND_WINDOW,
    STATE_INSTANT_PLAYBACK_START_MODE,
    STATE_VIDEO_POPUP_FEATURE,
    STATE_LOCAL_DECODING,
    MAX_STATE_LAYOUT
}STATE_TYPE_FOR_OTHER_MODE_e;

typedef struct _CAMEVT_VIDEO_POPUP_t
{
    VIDEO_POPUP_FEATURE_STATE_e         m_currentFeatureState;
    quint16                             m_previousSelectedWindow;
    LAYOUT_TYPE_e                       m_previousSelectedLayout;
    LAYOUT_TYPE_e                       m_previousBeforeOneXOne;
    qint32                              m_timerIdForVideoPopup;
    bool                                m_stopAllStream;
    quint16                             m_windowIndexForFeatureCurrentPage;
    WINDOW_INFO_t                       m_backupWindowInfo[MAX_WINDOWS];
    LAYOUT_TYPE_e                       m_oldLayoutId;
    bool                                m_wasExpandMode;
    STATE_TYPE_FOR_OTHER_MODE_e         m_previousMainState;
    quint8                              m_pendindingVideoPopupCount;
    bool                                m_videoPopupStartFlag;
    bool                                m_changeLayoutForVideoPopup;
    VIDEO_POPUP_FEATURE_VARIABLE_t      cameraEvtVideoPopData[MAX_POPUP_WINDOWS];
    VIDEO_POPUP_FEATURE_VARIABLE_t      next_cameraEvtVideoPopData[MAX_POPUP_WINDOWS];
    bool                                m_isRetryingMode;
    quint16                             m_startAudioInWindow;

    _CAMEVT_VIDEO_POPUP_t() : m_currentFeatureState(VIDEO_POPUP_STATE_NONE), m_previousSelectedWindow(0), m_previousSelectedLayout(MAX_LAYOUT),
        m_previousBeforeOneXOne(MAX_LAYOUT), m_timerIdForVideoPopup(0), m_stopAllStream(false),m_windowIndexForFeatureCurrentPage(0),
        m_oldLayoutId(MAX_LAYOUT), m_wasExpandMode(false), m_previousMainState(MAX_STATE_LAYOUT), m_pendindingVideoPopupCount(0),
        m_videoPopupStartFlag(false), m_changeLayoutForVideoPopup(false), m_isRetryingMode(false), m_startAudioInWindow(0)
    {

    }
}CAMEVT_VIDEO_POPUP_t;

typedef struct
{
    WINDOW_INFO_t   backupWindowInfo;
    CAM_INFO_t      currentCamInfo;
}INSTANT_PLAYBACK_DATA_t;

typedef struct
{
    bool        pendingRequestForLoalDecoding;
    qint32      m_timerToShowBannerMessage;
    quint8      m_preLocalDecodingF;
    QString     m_userName;
}LOCAL_DECODING_DATA_t;

typedef struct _AUTO_PLAY_FEATURE_VARIABLE_t
{
    quint8      m_timerCount;
    bool        m_isApToolbarVisible;
    bool        m_isPrevEnable;
    bool        m_isNextEnable;

    _AUTO_PLAY_FEATURE_VARIABLE_t() : m_timerCount(0), m_isApToolbarVisible(false), m_isPrevEnable(0), m_isNextEnable(0)
    {

    }
}AUTO_PLAY_FEATURE_VARIABLE_t;

typedef struct _AUTO_PLAY_DATA_t
{
    qint32                          m_timerIdForAutoPlay;
    AUTO_PLAY_FEATURE_VARIABLE_t    autoPlayFeatureDataType[MAX_CHANNEL_FOR_SEQ];

    _AUTO_PLAY_DATA_t() : m_timerIdForAutoPlay(0)
    {

    }
}AUTO_PLAY_DATA_t;

class Layout : public QWidget
{
    Q_OBJECT
private:    
    ImageAppearenceSettings*                    m_imageAppearenceSettings;
    PrivacyMaskSettings*                        m_privackMaskSettings;
    MotionDetectionSettings*                    m_motionDetectionSettings;
    IpMotionDetectionSettings*                  m_ipMotionDetectionSettings;
    ZoomFeatureControl*                         m_zoomFeatureControl;
    PbToolbar*                                  m_pbToolbar;
    CosecVideoPopupUser*                        m_cosecVideoPopupUser;
    InvisibleWidgetCntrl*                       m_inVisibleWidget;
    PTZControl*                                 m_ptzControl;
    Rectangle*                                  m_cosecMsgBannerRect;
    TextLabel*                                  m_cosecPopupMsg;
    InfoPage*                                   m_infoPage;
    ViewCamera*                                 m_viewCamera;
    InstantPlayback*                            m_instantpbToolbar;
    LiveViewToolbar*                            m_liveViewToolbar;
    MxAnalogPresetMenu*                         m_analogPresetMenu;

    STYLE_TYPE_e                                m_nextStyleNoToBeApplied[MAX_DISPLAY_TYPE];
    TOOLBAR_BUTTON_TYPE_e                       m_nextToolbarPage;
    LAYOUT_TYPE_e                               m_previousLayout[MAX_DISPLAY_TYPE];
    LAYOUT_TYPE_e                               m_syncBackupLayout;
    LAYOUT_TYPE_e                               m_previousSyncBeforeOneXOne;
    LAYOUT_TYPE_e                               previouslayoutId;
    AP_TOOLBAR_BTN_e                            pendingRequestForNxtPbRec[MAX_CHANNEL_FOR_SEQ];

    WINDOW_INFO_t                               m_syncBackupWinInfo[MAX_SYNC_PB_SESSION];
    DISPLAY_CONFIG_t                            m_nextDisplayConfig[MAX_DISPLAY_TYPE];
    INSTANT_PLAYBACK_DATA_t                     m_instantPlaybackData[MAX_CHANNEL_FOR_SEQ];
    DISPLAY_CONFIG_t                            prevDisplayConfig;

    QString                                     m_AudioEnableDevice;
    qint8                                       m_changePageOffset[MAX_DISPLAY_TYPE];
    quint8                                      m_AudioEnableCamId;
    quint8                                      m_pendingRequestCount[MAX_DISPLAY_TYPE];
    quint8                                      m_focusCntrlIndex;
    quint16                                     m_audioStopForWindow;
    quint16                                     m_previousSelectedWindow;
    quint16                                     m_syncBackupSelectedWindow;
    quint16                                     m_instantAudioWindow;
    quint16                                     m_audIndexToPlay;
    qint32                                      m_timerIdForRetry;
    qint32                                      m_timerIdForSequencing[MAX_DISPLAY_TYPE];
    qint32                                      m_timerIdForCosecPopup;
    qint32                                      m_timerIdForCosecBanner;
    qint32                                      m_timerIdForWindowSequence[MAX_DISPLAY_TYPE][MAX_CHANNEL_FOR_SEQ];

    bool                                        m_expandMode;
    bool                                        m_videoPopupMode;
    bool                                        m_instantPlayback;
    bool                                        m_refreshPageLaterFlag[MAX_DISPLAY_TYPE];
    bool                                        m_changePageLaterFlag[MAX_DISPLAY_TYPE];
    bool                                        m_applyNewStyleLaterFlag[MAX_DISPLAY_TYPE];
    bool                                        m_syncPlaybackStartFlag;
    bool                                        m_cosecHandleLaterFlag;
    bool                                        m_pbSliderChange;
    bool                                        m_doubleClickModeFlag;
    bool                                        m_stepIsRunning;
    bool                                        m_processPendingForWindow[MAX_DISPLAY_TYPE];
    bool                                        m_instantPlaybackRequestSent[MAX_CHANNEL_FOR_SEQ];
    bool                                        m_isPreviousPageNaviagtion;
    bool                                        m_isCurrStyleSelected[MAX_DISPLAY_TYPE];
    bool                                        m_isRetryingMode;
    bool                                        m_isRetryingModeinExpand;

public:
    static PayloadLib*                          payloadLib;
    static ApplController*                      applController;
    static LayoutCreator*                       layoutCreator;
    static WindowStreamInfo                     streamInfoArray[MAX_DISPLAY_TYPE][MAX_CHANNEL_FOR_SEQ];
    static PlaybackRecordData                   playbackRecordData[MAX_CHANNEL_FOR_SEQ];

    static ANALOG_FEATURE_VARIABLE_t            analogFeatureDataType;
    static COSEC_POPUP_FEATURE_VARIABLE_t       cosecPopupEvtData;
    static CAMEVT_VIDEO_POPUP_t                 videoPopupData;
    static DISPLAY_CONFIG_t                     currentDisplayConfig[MAX_DISPLAY_TYPE];
    static AUTO_PLAY_DATA_t                     autoPlayData;

    static STATE_TYPE_FOR_OTHER_MODE_e          currentModeType[MAX_DISPLAY_TYPE];
    static STATE_TYPE_FOR_OTHER_MODE_e          previousModeType[MAX_DISPLAY_TYPE];

    static quint8                               maxSupportedPrivacyMaskWindow;
    static quint8                               pendingInstantRequestCount[MAX_DISPLAY_TYPE][MAX_CHANNEL_FOR_SEQ];
    static quint16                              startX[MAX_DISPLAY_TYPE];
    static quint16                              startY[MAX_DISPLAY_TYPE];
    static quint16                              screenWidth[MAX_DISPLAY_TYPE];
    static quint16                              screenHeight[MAX_DISPLAY_TYPE];
    static quint16                              startAudioInWindow;
    static quint16                              lastEnableAudioWindow;
    static quint32                              tvAdjustParam;

    static bool                                 isRedraw;
    static bool                                 playbackStopRequest;
    static bool                                 m_isClientAudProcessRunning;

    quint16                                     m_currentRecNo[MAX_CHANNEL_FOR_SEQ];
    quint16                                     m_searchBtnClickRecNo[MAX_CHANNEL_FOR_SEQ];
    quint16                                     m_activeMicroPhoneWindow;

public:
    explicit Layout(QWidget *parent = 0);
    ~Layout();

    bool getScreenParam(LAYOUT_TYPE_e, DISPLAY_TYPE_e displayType);
    STYLE_TYPE_e readDefaultStyle(DISPLAY_TYPE_e displayIndex);
    void readDefaultLayout(DISPLAY_TYPE_e displayIndex);
    void setDefaultLayout();
    void setDefaultResolution();
    void setDefaultDisplay(QString deviceName);
    void setDefaultStreamInfo(QString deviceName);
    void startDefaultStreaming(QString deviceName);
    void changeLayout(DISPLAY_TYPE_e displayIndex, LAYOUT_TYPE_e layoutIndex, quint16 windowIndex, bool ifActualWindowIndex = false,
                      bool ifUpdateCurrentPage = true);
    void updateLayoutUiData(DISPLAY_TYPE_e displayId);
    void setupUIData(DISPLAY_TYPE_e displayType, QString deviceName);
    void setupUIData(DISPLAY_TYPE_e displayType, QString deviceName, quint8 camId);
    void mouseDoubleClicked(quint16 windowIndex);
    void closeButtonClicked(quint16 windowIndex);
    void addCameraButtonClicked(quint16 windowIndex);
    void expandWindow(quint16 windowIndex);
    void collapseWindow();
    void openToolbarButtonClicked(quint16 windowIndex);
    void changePage(DISPLAY_TYPE_e displayType, NAVIGATION_TYPE_e navigationType);
    void changeWindowChannel(DISPLAY_TYPE_e displayType, quint16 windowIndex);
    void refreshCurrentPage(DISPLAY_TYPE_e displayType);
    static void selectWindow(DISPLAY_TYPE_e displayType, quint16 windowIndex);
    static void giveFocusToWindow(quint16 windowIndex);
    static bool isWindowWiseSequeningRunning(DISPLAY_TYPE_e displayType = MAIN_DISPLAY);
    void disconnectDeviceNotify(QString devName);
    void processDeviceResponse(DevCommParam* param, QString deviceName);
    void processDeviceResponseForAnalogCameraFeature();
    void processDeviceResponseForCosecPopUp();
    void processDeviceResponseForVideoPopUp();
    void processStreamRequestResponse(STREAM_COMMAND_TYPE_e streamCommandType, StreamRequestParam *streamRequestParam,
                                      DEVICE_REPLY_TYPE_e deviceReply);
    void processStreamObjectDelete(DISPLAY_TYPE_e *displayTypeForDelete,quint16 *actualWindowIdForDelete);

    //function related to analog camera feature
    void showAnalogCameraFeature(quint8 cameraIndex, CAMERA_FEATURE_TYPE_e featureType, void* param = NULL,
                                 QString deviceName = LOCAL_DEVICE_NAME, void* configParam = NULL);
    void startAnalogCameraFeature();
    void stopAnalogCameraFeature();
    void resetAnalogCameraFeature();
    void hideAnalogCameraFeature();
    void closeAnalogCameraFeature(bool applyOldLayoutFlag = true);
    void takeActionForCameraMenuSetting(quint16 window);

    //function related to cosec integration feature
    void showCosecPopupFeature(quint8 cameraIndex, QString userName, quint32 popUpTime, QString userId, QString doorName,
                               quint8 eventStausCode);
    void startCosecPopupFeature();
    void stopCosecPopupFeature();
    void closeCosecPopupFeature();
    void resetCosecPopupFeature();
    void updateCosecPopupParam(bool saveLayoutParamFlag = true);
    void deleteCosecPopUpBanner();
    void deleteCosecPopupObject();
    void unloadAndHideOptionsInCosecPopup();
    void loadAndShowOptionsInCosecPopup();
    void updateNextParamToCurrent();
    void updateNextToolbarPageIndex(TOOLBAR_BUTTON_TYPE_e Index);

    //function related to Video Pop-up feature
    void showVideoPopupFeature(quint8 cameraIndex, QString deviceName, quint8  eventType, quint8 videoPopUpDuration);
    void processQueuedActionForVideoPopup();
    void startVideoPopupFeature();
    void closeVideoPopupFeatureWindow(quint16 windowIndex);
    void updateVideoPopupParam(bool saveLayoutParamFlag = true, quint16 windowIndex=0);
    void resetVideoPopupFeature();
    void stopVideoPopupFeature();
    void unloadAndHideOptionsInVideoPopup();
    void loadOptionsInVideoPopup();
    void backupCurrentConfigurationForVideoPopup();
    void updateCurrentConfigurationForVideoPopup();
    quint16 getVideoPopupLiveCam();
    void setSelectedWindow(quint16 windowIndex);
    void startSequencing(DISPLAY_TYPE_e displayType, bool forceStart = false);
    void stopSequencing(DISPLAY_TYPE_e displayType);
    void pauseSequencing(DISPLAY_TYPE_e displayType);
    void resumeSequencing(DISPLAY_TYPE_e displayType);
    void startWindowSequencing(DISPLAY_TYPE_e displayType);
    void stopWindowSequencing(DISPLAY_TYPE_e displayType);
    void pauseWindowSequencing (DISPLAY_TYPE_e displayType);
    void resumeWindowSequencing (DISPLAY_TYPE_e displayType);
    void processingBeforeSyncPlayback();
    void processingAfterSyncPlayback();
    void changeCurrentMode(STATE_TYPE_FOR_OTHER_MODE_e mode, DISPLAY_TYPE_e displayType, bool isNextProcessingNeeded = true);
    void doProcessingAfterChangingMode(DISPLAY_TYPE_e displayType);
    quint16 updateCurrentPage(DISPLAY_TYPE_e displayType, quint16 currentPage, NAVIGATION_TYPE_e navigationType);
    bool isNextPageAvailable(DISPLAY_TYPE_e displayType, quint16 currentPage, NAVIGATION_TYPE_e navigationType);
    bool isAnyWindowAssignedOnPage(DISPLAY_TYPE_e displayType, quint16 pageId);
    bool isNextChannelAvailableForWindow(DISPLAY_TYPE_e displayType, quint16 windowIndex, quint8 currentChannel);
    quint8 updateCurrentChannel(DISPLAY_TYPE_e displayType, quint16 windowIndex, quint8 currentChannel);
    void deleteDeviceFromCurrentStyle(DISPLAY_TYPE_e displayType, QString deviceName);
    void processNextActionForOtherMode(DISPLAY_TYPE_e displayType);
    void processQueuedAction(DISPLAY_TYPE_e displayType);
    void processNextActionForWindow(DISPLAY_TYPE_e displayType, quint16 windowIndex, bool windowSequencingFlag = true, bool instantPlaybackFlag = false);
    void applyNewStyle(DISPLAY_TYPE_e displayType, DISPLAY_CONFIG_t displayConfig, STYLE_TYPE_e styleNo = MAX_STYLE_TYPE);
    static void getOSDStatus(DISPLAY_TYPE_e dispType, quint16 windowId, DEV_CAM_INFO_t &camInfo);
    static void getSingleHealthParamStatus(DISPLAY_TYPE_e dispType, quint16 windowId, HEALTH_STS_PARAM_e healthStatuParam, quint8 &status);
    static void getRecordingStatus(DISPLAY_TYPE_e dispType, quint16 windowId, quint8 &recordingStatus);
    static void getFirstAndLastWindow(quint16 pageId, LAYOUT_TYPE_e layoutType, quint16 *window);
    static void changeStreamStateOfWindow(DISPLAY_TYPE_e displayId, quint16 windowIndex, QString deviceName, quint8 cameraId,
                                          int streamId, VIDEO_STREAM_TYPE_e videoType, VIDEO_STATUS_TYPE_e videoStatus,
                                          VIDEO_ERROR_TYPE_e errorStatus);
    static CAMERA_STATE_TYPE_e changeVideoStatusToCameraStatus(VIDEO_STATUS_TYPE_e videoStatus);
    static QString getCameraNameOfDevice(QString deviceName, quint8 cameraId);
    static quint16 findWindowOfLiveStream(DISPLAY_TYPE_e displayId, QString deviceName, quint8 cameraId);
    static quint16 findWindowOfSyncPlaybackStream(DISPLAY_TYPE_e displayId, QString deviceName, quint8 cameraId);
    static quint8 getCameraId(quint16 windowIndex);
    static void updateWindowDataForSyncPB(quint16 windowIndex);
    static quint16 findEmptyWindowInCurrentStyle(DISPLAY_TYPE_e displayId);
    static quint16 findEmptyWindowOnCurrentPageInCurrentStyle(DISPLAY_TYPE_e displayId);
    static quint16 findWindowIndexIfAssignOnCurrentPage(DISPLAY_TYPE_e displayType, QString deviceName, quint8 cameraId);
    static bool findWindowIndexIfAssignOnAnyPage(DISPLAY_TYPE_e displayType, QString deviceName, quint8 cameraId,
                                                 quint16 &windowIndex, quint8 &channelIndex);
    static bool isMultipleChannelAssigned(DISPLAY_TYPE_e displayType, quint16 windowIndex);
    static bool isAnyWindowReplacing(DISPLAY_TYPE_e displayType);
    void deAllocateWindow(DISPLAY_TYPE_e displayId, quint16 windowIndex);
    void actionWindowforDeviceStatechange(QString deviceName, DEVICE_STATE_TYPE_e devState);

    //function related to livestream
    quint8 startLiveStream(DISPLAY_TYPE_e displayIndex, QString devName, quint8 cameraId, quint16 windowIndex);
    quint8 startLiveStream(DISPLAY_TYPE_e displayIndex, QString devName, quint8 cameraId, quint16 windowIndex, LIVE_STREAM_TYPE_e streamType);
    quint8 stopLiveStream(DISPLAY_TYPE_e displayId, quint16 windowIndex);
    quint8 replaceLiveStream(DISPLAY_TYPE_e displayType, quint16 windowIndex, QString deviceName, quint8 cameraId, quint16 newWindowIndex);
    quint8 freeWindow(DISPLAY_TYPE_e displayId, quint16 windowIndex, bool isManuallyStop = false);
    void replaceLiveStreamForCurrentPage(DISPLAY_TYPE_e displayType);
    void stopLiveStreamInAllWindows(DISPLAY_TYPE_e displayType);
    void stopStreamsExceptCurrentPage(DISPLAY_TYPE_e displayType);
    void stopChangedStreamsInCurrentPage(DISPLAY_TYPE_e displayType);
    void stopStreamsForNewStyleApply(DISPLAY_TYPE_e displayType);
    void startStreamForCurrentPage(DISPLAY_TYPE_e displayType);
    void startPlayBackForCurrentPage(DISPLAY_TYPE_e displayType);
    void startStreamsForPageInRetry(DISPLAY_TYPE_e displayType);
    void shiftWindows(DISPLAY_TYPE_e displayType, quint16 newSelectedWindow, LAYOUT_TYPE_e newLayout);
    void videoPopupImageReplace();

    //function related to playback
    void closeDeletePbToolBar();
    void createPbToolbar(quint16 windowIndex);
    static bool isPlaybackRunning();
    static quint8 getPlaybackId(PlaybackRecordData *recData, quint16 windowIndex);
    static quint8 sendPlyRcdStrm(quint16 windowId);
    static quint8 sendStepRcdStrm(quint16 windowId);
    static quint8 sendStpRcdStrm(quint16 windowId);
    static quint8 sendClrPlyStrmId(quint16 windowId);
    static quint8 sendPbRequest(quint16 windowId);
    static bool startPlaybackStream(quint16 windowId, PB_DIRECTION_e direction, quint8 speed);
    static bool stopPlaybackStream(quint16 windowId);
    static bool slowPlaybackStream(quint16 windowId);
    static bool fastPlaybackStream(quint16 windowId);
    static bool stepPlaybackStream(quint16 windowId, PB_DIRECTION_e direction);
    static bool pausePlaybackStream(quint16 windowId);
    static QString mSecToString(quint64 currTime, bool msecNeeded = false);
    static quint64 stringToMSec(QString currTimeStr);
    void pausePlaybackStreamOfCurrentPage();
    void createPTZControl();
    void takeActionForPTZControl(quint16 window);
    void giveFocusToPTZControl(quint8 focusCntrlIndex);

    //function related to rightClick
    void sendEventToPtz(QString deviceName, quint8 camNum, LOG_EVENT_STATE_e evtState);
    void startStopManualRecording(quint16 windowIndex);
    void takeSnapshot(quint16 windowIndex);
    static void changeLiveStreamType(quint16 windowIndex, DISPLAY_TYPE_e displayId = MAIN_DISPLAY);
    void toggleStreamType(quint16 windowIndex, DISPLAY_TYPE_e dispId);
    void includeExcludeAudio(quint16 windowIndex, bool status = false);
    void excludeAudioExcept(quint16 audioWindowIndex, bool status);
    void includeDevAudio(quint16 audioWindowIndex);
    void excludeDevAudio(quint16 audioWindowIndex);
    void sendIncludeExcludeDevAudioInCmd(quint16 windowIndex, bool includeExcludeDevAudioFlag);
    void audioResponse(bool isAudioInc, quint8 streamId);
    static void createCmdPayload(SET_COMMAND_e cmdType, quint8 totalFields, DISPLAY_TYPE_e displayId = MAIN_DISPLAY);
    void updateDeviceState(QString deviceName, DEVICE_STATE_TYPE_e devState);

    //function related to Instant playback
    void startInstantPlayBackFeature(quint16 windowIndex);
    void stopInstantPlaybackFeature(quint16 windowIndex);
    void getInstantPlaybackId(quint16 windowIndex);
    void clearInstantPlaybackId(quint16 windowIndex);
    quint8 startInstantPlayBackStream(quint16 windowIndex, QString deviceName, quint8 cameraId);
    quint8 stopInstantPlayBackStream(quint16 windowIndex);
    quint8 pauseInstantPlaybackStream(quint16 windowIndex);
    quint8 seekInsantPlaybackStream(quint16 windowIndex);
    void createInstantPbToolbar(quint16 windowIndex);
    void closeDeleteInstantPbToolBar();
    void timerEvent(QTimerEvent *);

    //functions used for two way audio
    quint16 getStartAudioInWindow();
    void getAudioOutConfig();
    void checkUserPriviledge();
    void setlastEnableAudioWindow(quint16 windowIndex);
    quint16 getlastEnableAudioWindow();
    void audioOnAfterPageChanges(quint16 windowIndex);
    void changeWinAudioStatus(quint16 windowIndex);

    //Function Related To Local Decoding
    void stopLocalDecoding(bool localDecodingF, QString userName ="");
    void startLocalDecoding(bool localDecodingF);
    void updateWindowDataForLocalDecording();
    void resetLocalDecordingData();
    void startLocalDecodingFeature(DISPLAY_TYPE_e displayType);
    void stopLiveStreamLocalDecoding(DISPLAY_TYPE_e displayType);
    void playNextRecord(PlaybackRecordData NextReco, quint16 windowIndex, quint16 curRec);
    void UpdateAsyncCenterBtn(bool prevFlag,bool nextFlag,quint16 windowIndex);
    void clearsearchBtnClickResult();
    void stopAutoPlayFeature();
    static void resetAutoPlayFeatureInWindow(quint16 windowIndex);
    void navigationKeyPressed(QKeyEvent* event);

signals:
    void sigChangePageFromLayout();
    void sigCloseAnalogCameraFeature();
    void sigCloseCosecPopup();
    void sigWindowResponseToDisplaySettingsPage(DISPLAY_TYPE_e displayId, QString deviceName, quint8 cameraId, quint16 windowId);
    void sigPreProcessingDoneForSyncPlayback();
    void sigChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e index, STATE_TYPE_e currentState);
    void sigDisplaySettingApplyChangesNotify(DISPLAY_TYPE_e displayId, bool isCurrStyle);
    void sigUnloadToolbar();
    void sigHideToolbarPage();
    void sigToolbarStyleChnageNotify(STYLE_TYPE_e styleNo);
    void sigShowHideStyleAndPageControl(bool toShow,quint8 type);
    void sigLoadProcessBar();
    void sigRaiseWidgetOnTopOfAll();
    void sigStopClntAud(DEVICE_REPLY_TYPE_e statusId);
    void sigFindNextRcd(quint16 curRec,quint16);
    void sigFindPrevRecord(quint16 curRec,quint16);
    void sigGetNextPrevRecord(quint16 curRec,quint16 windowIndex,quint16 camId);
    void sigUpdateUIGeometry(bool isRedraw);

public slots:
    void slotWindowSelected(quint16 index);
    void slotLoadMenuListOptions(quint16 index);
    void slotWindowDoubleClicked(quint16 windowIndex);
    void slotEnterKeyPressed(quint16 windowIndex);
    void slotWindowImageClicked(WINDOW_IMAGE_TYPE_e imageType, quint16 windowIndex);
    void slotWindowImageHover(WINDOW_IMAGE_TYPE_e imageType, quint16 windowIndex, bool isHover);
    void slotSwapWindows(quint16 firstWindow, quint16 secondWindow);
    void slotDragStartStopEvent(bool isStart);
    void slotChangeLayout(LAYOUT_TYPE_e index, DISPLAY_TYPE_e disId, quint16 windowIndex, bool ifActualWindow, bool ifUpdateCurrentPage);
    void slotCloseCameraFeature(QObject*);
    void slotPlaybackPlayClick(PlaybackRecordData recData, QString devName,bool status);
    void slotPbToolbarHandling(int index, quint16 windowId, bool state);
    void slotPbToolbarSliderChanged(quint64 currTime, quint16 winId);
    void slotExitFromZoomFeature();
    void slotChangePageFromLayout();
    void slotSubObjectDelete();
    void slotInfoPageButtonClicked(int index);
    void slotCosecPopupSubObjDelete();
    void slotChangePage(NAVIGATION_TYPE_e navigationType);
    void slotDispSettingApplyChanges(DISPLAY_TYPE_e displayType, DISPLAY_CONFIG_t displayConfig, STYLE_TYPE_e styleNo = MAX_STYLE_TYPE);
    void slotViewCameraPageClose(TOOLBAR_BUTTON_TYPE_e buttonIndex);
    void slotStartStreamInWindow(DISPLAY_TYPE_e displayType, QString deviceName, quint8 channelId, quint16 windowId);
    void slotInstantPbToolbarHandling(int index, quint16 windowId, bool state);
    void slotInstantPbToolbarSliderChanged(quint64 currTime, quint16 winId);
    void slotCloseLiveViewToolbar();
    void slotLiveViewToolbarButtonClicked(LIVEVIEW_TOOLBAR_BUTTON_TYPE_e index, STATE_TYPE_e state, quint16 windowIndex);
    void slotLoadProcessBar();
    void slotExitVideoPopupState();
    void slotliveViewAudiostop();
    void slotAPCenterBtnClicked(quint8 index, quint16 windowId);
};

#endif // LAYOUT_H
