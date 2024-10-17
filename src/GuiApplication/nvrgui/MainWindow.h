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
//   Project      : NVR
//   Owner        : Shruti Sahni
//   File         : MainWindow.h
//   Description  : This is main window file for multiNvrClient.
/////////////////////////////////////////////////////////////////////////////
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <QTranslator>

#include "EnumFile.h"
#include "ApplController.h"
#include "Layout/Layout.h"
#include "Toolbar/Toolbar.h"
#include "Settings.h"
#include "BuzzerControl.h"
#include "UsbControl.h"
#include "VolumeControl.h"
#include "SystemDetail.h"
#include "LiveEvent.h"
#include "ApplicationMode.h"
#include "AboutUs.h"
#include "ManagePages.h"
#include "PlaybackSearch.h"
#include "SyncPlayback.h"
#include "LogIn.h"
#include "DisplaySetting.h"
#include "EventLogSearch.h"
#include "Controls/MessageBanner.h"
#include "Controls/PTZControl.h"
#include "DisplayMode.h"
#include "Controls/InfoPage.h"
#include "PayloadLib.h"
#include "Controls/MessageAlert.h"
#include "OsdUpdate.h"
#include "Controls/TextLabel.h"
#include "ValidationMessage.h"
#include "MxAutoAddCam.h"
#include "Controls/MxPopUpAlert.h"
#include "Controls/MxReporTable.h"
#include "MxQuickBackup.h"
#include "Controls/MxTimelineSlider.h"
#include "Wizard/SetupWizard.h"

typedef enum
{
    TIMEZONE_INDEX,
    VIDEO_STD,
    COSEC_INT,
    DATE_FMT,
    MAX_AUTO_TIMEZONE_PARAM
}AUTO_TIMEZONE_PARAM_e;

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    bool                                m_isAutoCnfgChecked;
    bool                                m_isAutoTmZnUpdtRunning;
    quint8                              m_reportCurPageNo;
    bool                                m_isClntAudStartedInInstPB;
    bool                                m_audioStatusBeforeClntAud;
    QMap<AUTO_TIMEZONE_PARAM_e,quint8>  m_currAutoTmZnParam;
    QMap<AUTO_TIMEZONE_PARAM_e,quint8>  m_detectedAutoTmZnParam;
    ApplController*                     m_applController;
    PayloadLib*                         m_payloadLib;
    AboutUs*                            m_aboutUs;
    Layout*                             m_layout;
    Toolbar*                            m_toolbar;
    DisplayMode*                        m_displayMode;
    Settings*                           m_settings;
    BuzzerControl*                      m_buzzerControl;
    UsbControl*                         m_usbControl;
    VolumeControl*                      m_volumeControl;
    SystemDetail*                       m_systemDetail;
    LiveEvent*                          m_liveEvent;
    ManagePages*                        m_managePages;
    PlaybackSearch *                    m_playbackSearch;
    SyncPlayback*                       m_syncPlayback;
    LogIn*                              m_logIn;
    DisplaySetting*                     m_displaySetting;
    EventLogSearch*                     m_eventLogSearch;
    QWidget*                            m_toolbarPage;
    MessageBanner*                      m_messageBanner;
    ValidationMessage*                  m_validationMessage;
    ApplicationMode*                    m_applicationMode;
    InfoPage*                           m_infoPage;
    InfoPage*                           m_autoTmznRebootInfoPage;
    InfoPage*                           m_startLiveViewInfoPage;
    InfoPage*                           m_autoAddCamInfoPage;
    InfoPage*                           m_multiLanguageInfoPage;
    Rectangle*                          m_initDeInitMsgBox;
    TextLabel*                          m_initDeInitMsg;
    MessageAlert*                       m_messageAlert;
    OsdUpdate*                          m_osdUpdate;
    MxAutoAddCam*                       m_autoAddCam;
    QTimer*                             m_hddCleanInfoTimer;
    quint8                              m_hddTimerCnt;
    MxPopUpAlert*                       m_popUpAlert;
    MxReportTable*                      m_reportTable;
    MxQuickBackup*                      m_quickBackup;
    MX_AUTO_ADD_CAM_SEARCH_ACTION_e     m_autoAddCamAction;
    QTimer*                             m_clientAudInclTimer;
    quint8                              m_clientAudInclretryCnt;
    bool                                m_isUnloadAutoAddCam;
    quint8                              m_clntAudStreamId;
    bool                                m_isStartLiveView;
    bool                                m_isAutoAddCamTimeout;
    quint8                              m_totalRecord;
    PlaybackRecordData                  m_prevRecordData[MAX_REC_ALLOWED];
    SetupWizard*                        m_setupWizard;
    bool                                m_setupWizardPendingF;
    InvisibleWidgetCntrl*               m_inVisibleWidget;
    bool                                m_preStartLiveViewF;
    TEMP_PLAYBACK_REC_DATA_t            m_pbSearchPrevData;
    bool                                m_userLanguagePendingF;
    bool                                m_showVideoPopupF;
    QUICK_BACKUP_ELEMENTS_e             m_quickBackupActions;
    QTranslator                         m_languageTranslator;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void createLayout();
    void createToolbar();
    void updateUsbStatus();
    void updateAudioStatus();
    void createInitDeinitBox(QString msgDisplay);
    void deleteInitDeInitBox();
    void mouseMoveEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent *event);
    void getAutoCnfgFlag();
    void processDeviceResponse(DevCommParam *param, QString deviceName);
    void getAutoUpdtRegFlg();
    void getVideoStdDtFrmt();
    void setAutoTimeZoneIndex(bool isAutoUpdtRegSetting);
    void sendClntAudCmd(SET_COMMAND_e cmdId, DEVICE_REPLY_TYPE_e deviceReply = CMD_MAX_DEVICE_REPLY);
    void processClntStrmReqRes(SET_COMMAND_e commandId, DEVICE_REPLY_TYPE_e statusId);
    void getUserPreferredLanguage(void);

signals:
    void sigClosePage(TOOLBAR_BUTTON_TYPE_e index);
    void sigNotifyStatusIcon(TOOLBAR_BUTTON_TYPE_e index, bool state);
    void sigChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e index, STATE_TYPE_e currentState);
    void sigChangeToolbarBtnState(TOOLBAR_BUTTON_TYPE_e index, bool state);

public slots:
    void slotOpenToolbarPage(TOOLBAR_BUTTON_TYPE_e index);
    void slotCloseToolbarPage(TOOLBAR_BUTTON_TYPE_e index);
    void slotDevCommGui(QString deviceName, DevCommParam* param);
    void slotEventToGui(QString deviceName, quint8 tEventType, quint8 eventSubType, quint8 eventIndex, quint8 eventState,
                        QString eventAdvanceDetail, bool isLiveEvent);
    void slotPopUpEvtToGui(QString deviceName, quint8 cameraIndex, QString userName, quint32 popUpTime, QString userId,
                           QString doorName, quint8 eventCodeIndex);
    void slotStreamRequestResponse(STREAM_COMMAND_TYPE_e streamCommandType, StreamRequestParam *streamRequestParam, DEVICE_REPLY_TYPE_e deviceReply);
    void slotOpenCameraFeature(void *param, CAMERA_FEATURE_TYPE_e featureType, quint8 cameraInde, void *configParam, QString devName);
    void slotCloseAnalogCameraFeature();
    void slotApplicationModeChanged();
    void slotCloseCosecPopUp();
    void slotUnloadToolbar();
    void slotHideToolbarPage();
    void slotInfoPageButtonClicked(int index);
    void slotDeviceListChangeToGui();
    void slotLoadProcessBar();
    void slotStreamObjectDelete(DISPLAY_TYPE_e *displayTypeForDelete, quint16 *actualWindowIdForDelete);
    void slotMessageAlertClose();
    void slotRaiseWidget();
    void slotAutoAddClose();
    void slotHddCleanInfoTimeout();
    void slotAutoAddCamPopUpAction(MX_AUTO_ADD_CAM_SEARCH_ACTION_e action);
    void slotPopUpAlertClose(int index, bool isQueueEmpty);
    void slotReportTableClose(quint8 pageNo);
    void slotAutoTmznRbtInfoPgBtnClick(int index);
    void slotHdmiInfoPage(bool isHdmiInfoShow);
    void slotLanguageCfgModified(QString langStr);
    void slotUpdateQuickbackupFlag(bool flag);
    void slotClientAudInclude();
    void slotStopClntAud(DEVICE_REPLY_TYPE_e statusId);
    void slotStartLiveViewInfoPageClick(int index);
    void slotPrevRecords(PlaybackRecordData* tempRecord, quint8 totalTempRecord, TEMP_PLAYBACK_REC_DATA_t *tempData);
    void slotFindNextRecord(quint16 curRec, quint16 windIndx);
    void slotFindPrevRecord(quint16 curRec, quint16 windIndx);
    void slotGetNextPrevRecord(quint16 curRec, quint16 windowIndex, quint16 camId);
    void slotUpdateUIGeometry(bool isRedraw);
    void slotChangeAudButtonState();
    void slotAdvanceOption(QUICK_BACKUP_ELEMENTS_e index);
    void slotQuitSetupWiz();
    void slotOpenWizard();

private:
    void sendCommand(QString deviceName, SET_COMMAND_e cmdType, int totalfeilds = 0);
};

#endif // MAINWINDOW_H
