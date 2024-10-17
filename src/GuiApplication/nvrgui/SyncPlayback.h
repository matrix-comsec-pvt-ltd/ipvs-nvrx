#ifndef SYNCPLAYBACK_H
#define SYNCPLAYBACK_H

#include <QWidget>
#include <QTimer>
#include <Controls/Rectangle.h>
#include <Controls/Closebuttton.h>
#include <Controls/SyncPlayback/SyncPlaybackToolbar.h>
#include <Controls/SyncPlayback/SyncPlaybackTimeLine.h>
#include <Controls/SyncPlayback/SyncPlaybackCalender.h>
#include <Controls/DropDown.h>
#include <Controls/ElementHeading.h>
#include <Controls/OptionSelectButton.h>
#include <Controls/ControlButton.h>
#include <Controls/ElementHeading.h>
#include <Controls/SyncPlayback/SyncPlaybackCropAndBackup.h>
#include <Controls/SyncPlayback/SyncPlayBackLoadingText.h>
#include <Controls/ZoomFeatureControl.h>
#include <Controls/ScrollBar.h>
#include <Controls/TextLabel.h>
#include <ApplController.h>
#include <PayloadLib.h>
#include "KeyBoard.h"
#include "Controls/ToolTip.h"

/* Max Camera List Count shown in Scrollbar */
#define MAX_SCROLL_CAM_LIST_CNT				(16)
/* Max Camera List Sperator Count */
#define MAX_CAM_LIST_SEPARATOR_CNT			(MAX_SCROLL_CAM_LIST_CNT + 1)
/* Max Camera List Count including Select Maximum */
#define MAX_CAM_LIST_CNT					(MAX_CAMERAS + 1)


typedef enum
{
    SYNC_PLAYBACK_CLOSEBUTTON,
    SYNC_PLAYBACK_DEVICENAME_SPINBOX,
    SYNC_PLAYBACK_REC_DRIVE_DROPBOX,
    SYNC_PLAYBACK_CALENDER,
    SYNC_PLAYBACK_RECORDINGTYPE_CHECKBOX,
    SYNC_PLAYBACK_SEARCH_CONTROLBUTTON  = (SYNC_PLAYBACK_RECORDINGTYPE_CHECKBOX + 4),
    SYNC_PLAYBACK_CAMERASELECT_CHECKBOX,
	SYNCPB_SCROLLBAR =(SYNC_PLAYBACK_CAMERASELECT_CHECKBOX + MAX_CAM_LIST_CNT),
    SYNC_PLAYBACK_TOOLBAR,
    SYNC_PLAYBACK_HOURTYPE_RADIOBUTTON,
    SYNC_PLAYBACK_TIMELINE = (SYNC_PLAYBACK_HOURTYPE_RADIOBUTTON + 4),
    MAX_SYNC_PLAYBACK_ELEMENT
}SYNC_PLAYBACK_ELEMENT_e;

typedef enum
{
    ALARM_RECORDING,
    MANUAL_RECORDING,
    SCHEDULE_RECORDING,
    COSEC_RECORDING,
    MAX_RECORDING_TYPE
}RECORDING_TYPE_e;

typedef enum
{
    STEP_FORWARD,
    STEP_BACKWARD
}SYNC_PLAYBACK_STEP_DIRECTION_e;

typedef enum
{
    SYNC_PLAYBACK_ZOOM_IN,
    SYNC_PLAYBACK_ZOOM_OUT
}SYNC_PLAYBACK_ZOOM_STATE_e;

class SyncPlayback : public KeyBoard
{
    Q_OBJECT
private:
    Rectangle*                      m_mainRightRectangle;
    Rectangle*                      m_mainBottomRectangle;
    CloseButtton*                   m_closeButton;
    SyncPlaybackToolbar*            m_toolbar;
    SyncPlaybackTimeLine*           m_timeLine;
    SyncPlaybackCalender*           m_calender;
    DropDown*                       m_deviceNameDropDownBox;
    ElementHeading*                 m_camerasHeading;
    OptionSelectButton*             m_cameraCheckbox[MAX_CAMERAS + 1];
    CnfgButton*                     m_searchButton;
    ElementHeading*                 m_hoursHeading;
    ElementHeading*                 m_typesHeading;
    OptionSelectButton*             m_hoursRadioButtion[MAX_HOUR_FORMAT_TYPE];
    OptionSelectButton*             m_recordingTypeCheckbox[MAX_RECORDING_TYPE];
    TextLabel*                      m_recordingInitialLabel[MAX_RECORDING_TYPE];
    TextLabel*                      m_recordingLabel[MAX_RECORDING_TYPE];
    SyncPlaybackCropAndBackup*      m_syncPlaybackCropAndBackup;
    SyncPlayBackLoadingText*        m_syncPlaybackLoadingText;
    ZoomFeatureControl*             m_zoomFeatureControl;
    ScrollBar*                      m_syncScrollbar;
    QWidget*                        m_inVisibleWidget;
    DropDown*                       m_recDriveListDropBox;

    SYNCPB_TOOLBAR_MODE_TYPE_e      m_currentMode, m_modeBeforeZoomAction;
    HOURS_FORMAT_TYPE_e             m_currentHourFormat;
    SYNC_PLAYBACK_STATE_e           m_currentPlaybackState, m_previousPlaybackState;
    PB_SPEED_e                      m_playBackSpeed;
    PB_DIRECTION_e                  m_playBackDirection;
    QList<CROP_AND_BACKUP_DATA_t>   m_cropAndBackupData;
    SYNC_PLAYBACK_ZOOM_STATE_e      m_currentZoomState;
    Rectangle*						m_cameraListSeparator[MAX_CAM_LIST_SEPARATOR_CNT];
	TextLabel*						m_cameraRecType[MAX_CAMERAS][MAX_RECORDING_TYPE];
	TextLabel*						m_errorMsgTxt;
	LAYOUT_TYPE_e					m_prevLayout;
	quint8							m_cameraListCount;
	quint8							m_selectedCamCnt;
    quint16                         m_prevSelectedWindowIndex;
    CAMERA_BIT_MASK_t               m_cameraValueForPlayRecord;

    int                             m_currentElement;
    NavigationControl*              m_elementList[MAX_SYNC_PLAYBACK_ELEMENT];
    ApplController*                 m_applController;
    PayloadLib*                     m_payloadLib;

    QRect                       m_layoutArea;
    QStringList                 m_cameraNameList;
    quint8                      m_moveCount, m_recordValueForSearch;
    CAMERA_BIT_MASK_t           m_cameraValueForSearch, m_audioValueForPlayRecord;
    bool                        m_isFirstTimePlay, m_isSliderPositionChangedbyUser, m_isAudioCommandSend;
    bool                        m_isRequestPendingForSliderPositionChanged;
    quint16                     m_windowIndexForAudio;
    quint16                     m_windowIndexForAudioOld;
    quint16                     m_currentWindowIndex;
    quint64                     m_lastFrameSecond;
    bool                        m_isPreProcessingIsDone, m_isSearchDateRequired;
    QString                     m_currentDeviceName;
    quint8                      m_syncPlaybackStreamId;
    bool                        m_isSyncRequestPending;
    quint8                      m_firstCameraIndex, m_lastCameraIndex;
    bool                        m_isGetDateTimeRequestPending, m_isSearchDateRequestPending, m_isSearchHoursRequestPending;
    quint8                      m_searchAttemptsForDate, m_searchAttemptsForHours;

    QTimer*                     m_retryTimer;
    QRect                       m_windowArea[MAX_SYNC_PB_SESSION];
    quint8                      m_cameraIndex[MAX_CAMERAS];
    bool                        m_isDeviceConnected;
    bool                        m_isQuickBackupon;
    bool                        m_isClientAudioProcessing;
    quint16                     m_audCamIndexAfterClntAud;
    bool                        m_isCloseButtonClicked;

public:
    SyncPlayback(quint16 layoutWidth, quint16 layoutHeight, bool isPreProcessingDone, QWidget* parent = 0);
    ~SyncPlayback();

    bool stopSyncBackupManual();
    void changeCameraElement();
    void ResponceOnDeviceDisconnected();
    void updateOnNewDeviceConnected();
    void resetGeometryForCameraOption();
    void getDateAndTime();
    bool searchNewRecord();
    void searchDatesHavingRecord();
    void searchHoursHavingRecord();
    void setCameraValueForSearch();
    void setRecordValueForSearch();
	void selectDeselectAllRecords(OPTION_STATE_TYPE_e iButtonState);
	void selectDeselectCameraCheckBox(OPTION_STATE_TYPE_e iButtonState, int indexInPage);
    bool isAllButtonChecked();
    void changeMode(SYNCPB_TOOLBAR_MODE_TYPE_e mode);
    void changeLayout(LAYOUT_TYPE_e layout);
    void changeCurrentPlaybackState(SYNC_PLAYBACK_STATE_e);
    void changeAudioStateOfAudioButton();
    void makeVisibleInvisibleAllElements(bool visibleFlag);
    void processDeviceResponse(DevCommParam* param, QString deviceName);
    void processStreamRequestResponse(STREAM_COMMAND_TYPE_e streamCommandType, StreamRequestParam *streamRequestParam, DEVICE_REPLY_TYPE_e deviceReply);
    void updateManualBackupStatusEventAction(QString deviceName, quint8 copiedPercentage);
    void updateManualBackupSystemEventAction(QString deviceName, LOG_EVENT_STATE_e eventState);
    bool createStreamCommand(STREAM_COMMAND_TYPE_e streamCommandType, QString payload);
    void startSyncPlaybackRecord();
    void pauseSyncPlaybackRecord();
	void SetSpeed(void);
    void stopSyncPlaybackRecord();
    void stepSyncPlaybackRecord(SYNC_PLAYBACK_STEP_DIRECTION_e stepDirection);
    void sendSyncPlaybackAudioCommand();
    void clearSyncPlaybackRecord();
    void processStopSyncPlaybackAction();
    void ClearSyncPlaybackWindow();
    void processClearSyncPlaybackAction();
    void clearRecords();
    void startClipMaking();
    void stopClipMaking();
    void setLayoutWindowArea();
    void enterInZoomAction();
    void exitFromZoomAction();
    void deviceDisconnectNotify(QString devName);
    void setQuickBackupFlag(bool flag);
	void CreateCameraList(void);
	void DeleteCameraList(void);
	void HideCameraList(void);

    void takeLeftKeyAction();
    void takeRightKeyAction();
    void takeUpKeyAction();
    void takeDownKeyAction();

    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);
    void setClientAudioRunningFlag(bool state);
    bool getSyncPBAudPlayStatus();
    void setAudCamIndexAfterClntAud();
    void stopCamAudAfterClntAud();
    void startCamAudAfterClntAud();
    void updateDeviceList(void);

    //ketboard functions
    void navigationKeyPressed(QKeyEvent *event);
    void backTab_KeyPressed(QKeyEvent *event);
    void tabKeyPressed(QKeyEvent *event);
    void insertKeyPressed(QKeyEvent *event);
    void enterKeyPressed(QKeyEvent *event);
    void escKeyPressed(QKeyEvent *event);
    void functionKeyPressed(QKeyEvent *event);

    static LAYOUT_TYPE_e m_currentLayout;

signals:
    void sigClosePage(TOOLBAR_BUTTON_TYPE_e index);
    void sigChangeLayout(LAYOUT_TYPE_e layout, DISPLAY_TYPE_e displayId, quint16 windowIndex, bool ifAtcualWindow, bool ifUpdatePage);

public slots:
    void slotCloseButtonClicked(int index);
    void slotUpdateCurrentElement(int index);
    void slotDeviceNameChanged(QString string, quint32);
    void slotRecDriveDropBoxChanged(QString string, quint32);
    void slotFocusChangedFromCurrentElement(bool isPreviousElement);
    void slotFetchNewSelectedDate();
    void slotFetchRecordForNewDate();
    void slotSearchButtonClicked(int index);
	void slotCameraCheckboxClicked(OPTION_STATE_TYPE_e iButtonState, int indexInPage);
	void slotRecTypeCheckboxClicked(OPTION_STATE_TYPE_e iButtonState, int indexInPage);
    void slotHourFormatChanged(OPTION_STATE_TYPE_e currentState, int indexInPage);
    void slotSliderPositionChanged();
    void slotSliderPositionChangedStart();
    void slotToolbarButtonClicked(SYNCPB_TOOLBAR_BUTTON_TYPE_e index, STATE_TYPE_e state);
    void slotCropAndBackupPageClosed(TOOLBAR_BUTTON_TYPE_e index);
    void slotPreProcessingDoneForSyncPlayback();
    void slotLayoutChanged();
    void slotExitFromZoomFeature();
    void slotScrollbarClick(int numberOfSteps);
    void slotRetryTimeOut();
	void slotchangeLayout(LAYOUT_TYPE_e iLayoutType);
};

#endif // SYNCPLAYBACK_H
