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
//   File         : MainWindow.cpp
//   Description  : This is main window file for multiNvrClient.
/////////////////////////////////////////////////////////////////////////////
#include <unistd.h>
#include <QMouseEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QFontDatabase>

#include "MainWindow.h"
#include "CommonApi.h"
#include "ValidationMessage.h"
#include "../DecoderLib/include/DecDispLib.h"

#define MAX_CLIENT_INC_AUD_RETRY_CNT    (5)

bool isMessageAlertLoaded;
bool isOnbootAuoCamSearchRunning;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setObjectName ("MAIN_WIN");

    QFontDatabase::addApplicationFont (":/fonts/Fonts/GOTHIC.TTF");
    QFontDatabase::addApplicationFont (":/fonts/Fonts/GOTHICB.TTF");
    QFontDatabase::addApplicationFont(":/fonts/Fonts/GOTHICBI.TTF");
    QFontDatabase::addApplicationFont(":/fonts/Fonts/GOTHICI.TTF");

    this->setEnabled(true);
    this->setMouseTracking(true);

    m_layout = NULL;
    m_toolbar = NULL;
    m_toolbarPage = NULL;
    m_settings = NULL;
    m_buzzerControl = NULL;
    m_usbControl = NULL;
    m_volumeControl = NULL;
    m_systemDetail = NULL;
    m_liveEvent = NULL;
    m_managePages = NULL;
    m_aboutUs = NULL;
    m_playbackSearch = NULL;
    m_syncPlayback = NULL;
    m_logIn = NULL;
    m_displaySetting = NULL;
    m_eventLogSearch = NULL;
    m_messageBanner = NULL;
    m_validationMessage = NULL;
    m_displayMode = NULL;
    m_initDeInitMsgBox = NULL;
    m_initDeInitMsg = NULL;
    m_messageAlert = NULL;
    m_popUpAlert = NULL;
    m_osdUpdate = NULL;
    m_autoAddCam = NULL;
    m_reportTable = NULL;
    m_quickBackup = NULL;
    m_autoTmznRebootInfoPage = NULL;
    m_isAutoCnfgChecked = false;
    m_userLanguagePendingF = false;
    m_hddTimerCnt = 60; /* Count down timer variable for HDD clean up info page. */
    m_autoAddCamAction =  MAX_MX_AUTO_ADD_CAM_SEARCH_ACTION;
    m_reportCurPageNo = 1;
    m_isAutoTmZnUpdtRunning = false;
    m_currAutoTmZnParam.clear();
    m_detectedAutoTmZnParam.clear();
    m_isClntAudStartedInInstPB = false;
    m_audioStatusBeforeClntAud = false;
    m_clientAudInclTimer = NULL;
    m_clientAudInclretryCnt = 0;
    m_clntAudStreamId = MAX_STREAM_SESSION;
    m_inVisibleWidget = NULL;
    isMessageAlertLoaded = false;
    isOnbootAuoCamSearchRunning = false;
    m_isUnloadAutoAddCam = false;
    m_preStartLiveViewF = false;
    m_isStartLiveView = false;
    m_isAutoAddCamTimeout = false;
    m_totalRecord = 0;
    m_startLiveViewInfoPage = NULL;
    m_showVideoPopupF = false;
    m_autoAddCamInfoPage = NULL;
    m_multiLanguageInfoPage = NULL;
    m_quickBackupActions = MAX_QUICK_BACKUP_ELEMENTS;
    m_setupWizard = NULL;
    m_setupWizardPendingF = false;

    m_applController = ApplController::getInstance();
    connect(m_applController,
            SIGNAL(sigDevCommGui(QString, DevCommParam*)),
            this,
            SLOT(slotDevCommGui(QString, DevCommParam*)));
    connect(m_applController,
            SIGNAL(sigEventToGui(QString, quint8, quint8, quint8, quint8, QString, bool)),
            this,
            SLOT(slotEventToGui(QString, quint8, quint8, quint8, quint8, QString, bool)));
    connect(m_applController,
            SIGNAL(sigPopUpEventToGui(QString, quint8, QString, quint32, QString, QString, quint8)),
            this,
            SLOT(slotPopUpEvtToGui(QString, quint8, QString, quint32, QString, QString, quint8)));
    connect(m_applController,
            SIGNAL(sigStreamRequestResponse(STREAM_COMMAND_TYPE_e, StreamRequestParam*, DEVICE_REPLY_TYPE_e)),
            this,
            SLOT(slotStreamRequestResponse(STREAM_COMMAND_TYPE_e, StreamRequestParam*, DEVICE_REPLY_TYPE_e)));
    connect(m_applController,
            SIGNAL(sigDeviceListChangeToGui()),
            this,
            SLOT(slotDeviceListChangeToGui()));
    connect(m_applController,
            SIGNAL(sigStreamObjectDelete(DISPLAY_TYPE_e*, quint16*)),
            this,
            SLOT(slotStreamObjectDelete(DISPLAY_TYPE_e*, quint16*)));
    connect(m_applController,
            SIGNAL(sigHdmiInfoPage(bool)),
            this,
            SLOT(slotHdmiInfoPage(bool)));
    connect(m_applController,
            SIGNAL(sigChangeAudButtonState()),
            this,
            SLOT(slotChangeAudButtonState()));
    connect(m_applController,
            SIGNAL(sigLanguageCfgModified(QString)),
            this,
            SLOT(slotLanguageCfgModified(QString)));

    /* Read Default Style and Layout index from display config file and get screen origin parameter
     * from decode display and set main window origin. */
    DISPLAY_CONFIG_t dispConfig;
    STYLE_TYPE_e styleType = MAX_STYLE_TYPE;
    memset(&dispConfig, 0, sizeof(DISPLAY_CONFIG_t));

    QList<QVariant> paramList;
    paramList.insert(SUB_ACTIVITY_TYPE, READ_DFLTSTYLE_ACTIVITY);
    paramList.insert(DISPLAY_ID, MAIN_DISPLAY);
    m_applController->processActivity(DISPLAY_SETTING, paramList, &styleType);
    paramList.clear();

    if (styleType != MAX_STYLE_TYPE)
    {
        paramList.append(READ_DISP_ACTIVITY);
        paramList.append(MAIN_DISPLAY);
        paramList.append(styleType);
        m_applController->processActivity(DISPLAY_SETTING, paramList, &dispConfig);
    }

    m_applController->getOriginofScreen(dispConfig.layoutId, MAIN_DISPLAY);
    this->setGeometry(QRect(ApplController::getXPosOfScreen(), ApplController::getYPosOfScreen(),
                            ApplController::getWidthOfScreen(), ApplController::getHeightOfScreen()));
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    QApplication::setActiveWindow(this);

    createLayout();

    m_messageBanner = MessageBanner::getInstance(this);
    m_validationMessage = ValidationMessage::getInstance();
    createInitDeinitBox(ValidationMessage::getValidationMessage(MAIN_WINDOW_INIT_MSG));
    m_applicationMode = ApplicationMode::getApplicationModeInstance(this);
    connect(m_applicationMode,
            SIGNAL(sigApplicationModeChanged()),
            this,
            SLOT(slotApplicationModeChanged()));
    ApplicationMode::setApplicationMode(IDLE_MODE);

    for (quint8 displayIndex = 0; displayIndex < MAX_DISPLAY_TYPE; displayIndex++)
    {
        m_layout->readDefaultLayout((DISPLAY_TYPE_e)displayIndex);
    }

    m_infoPage = new InfoPage(0, 0, this->width(), this->height(), MAX_INFO_PAGE_TYPE, this, true, false);
    connect(m_infoPage,
            SIGNAL(sigInfoPageCnfgBtnClick(int)),
            this,
            SLOT(slotInfoPageButtonClicked(int)));

    m_payloadLib = new PayloadLib();

    m_hddCleanInfoTimer = new QTimer();
    connect(m_hddCleanInfoTimer,
            SIGNAL(timeout()),
            this,
            SLOT(slotHddCleanInfoTimeout()));
    m_hddCleanInfoTimer->setInterval(1000);
    m_hddCleanInfoTimer->setSingleShot(false);

    m_clientAudInclTimer = new QTimer();
    connect(m_clientAudInclTimer,
            SIGNAL(timeout()),
            this,
            SLOT(slotClientAudInclude()));
    m_clientAudInclTimer->setInterval(200);
    m_clientAudInclTimer->setSingleShot(false);

    m_autoAddCamInfoPage = new InfoPage(0, 0, this->width(), this->height(), MAX_INFO_PAGE_TYPE, this);
    m_multiLanguageInfoPage = new InfoPage(0, 0, this->width(), this->height(), MAX_INFO_PAGE_TYPE, this);
    showFullScreen();
}

MainWindow::~MainWindow()
{
    if (IS_VALID_OBJ(m_toolbar))
    {
        disconnect(m_toolbar,
                   SIGNAL(sigOpenToolbarPage(TOOLBAR_BUTTON_TYPE_e)),
                   this,
                   SLOT(slotOpenToolbarPage(TOOLBAR_BUTTON_TYPE_e)));
        disconnect(m_toolbar,
                   SIGNAL(sigCloseToolbarPage(TOOLBAR_BUTTON_TYPE_e)),
                   this,
                   SLOT(slotCloseToolbarPage(TOOLBAR_BUTTON_TYPE_e)));
        disconnect(this,
                   SIGNAL(sigNotifyStatusIcon(TOOLBAR_BUTTON_TYPE_e, bool)),
                   m_toolbar,
                   SLOT(slotNotifyStatusIcon(TOOLBAR_BUTTON_TYPE_e, bool)));
        disconnect(this,
                   SIGNAL(sigChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e, STATE_TYPE_e)),
                   m_toolbar,
                   SLOT(slotChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e, STATE_TYPE_e)));
        disconnect(this,
                   SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                   m_toolbar,
                   SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
        disconnect(m_layout,
                   SIGNAL(sigChangePageFromLayout()),
                   m_toolbar,
                   SLOT(slotChangePageFromLayout()));
        disconnect(m_layout,
                   SIGNAL(sigChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e, STATE_TYPE_e)),
                   m_toolbar,
                   SLOT(slotChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e, STATE_TYPE_e)));
        disconnect(m_toolbar,
                   SIGNAL(sigChangePage(NAVIGATION_TYPE_e)),
                   m_layout,
                   SLOT(slotChangePage(NAVIGATION_TYPE_e)));
        disconnect(m_toolbar,
                   SIGNAL(sigToolbarApplyStyle(DISPLAY_TYPE_e, DISPLAY_CONFIG_t, STYLE_TYPE_e)),
                   m_layout,
                   SLOT(slotDispSettingApplyChanges(DISPLAY_TYPE_e, DISPLAY_CONFIG_t, STYLE_TYPE_e)));
        disconnect(m_layout,
                   SIGNAL(sigToolbarStyleChnageNotify(STYLE_TYPE_e)),
                   m_toolbar,
                   SLOT(slotStyleChnagedNotify(STYLE_TYPE_e)));
        disconnect(m_layout,
                   SIGNAL(sigShowHideStyleAndPageControl(bool, quint8)),
                   m_toolbar,
                   SLOT(slotShowHideStyleAndPageControl(bool, quint8)));
        disconnect(m_toolbar,
                   SIGNAL(sigExitVideoPopupState()),
                   m_layout,
                   SLOT(slotExitVideoPopupState()));
        disconnect(m_toolbar,
                   SIGNAL(sigOpenWizard()),
                   this,
                   SLOT(slotOpenWizard()));
        DELETE_OBJ(m_toolbar);
    }

    if (IS_VALID_OBJ(m_applicationMode))
    {
        disconnect(m_applicationMode,
                   SIGNAL(sigApplicationModeChanged()),
                   this,
                   SLOT(slotApplicationModeChanged()));
        DELETE_OBJ(m_applicationMode);
    }

    if (IS_VALID_OBJ(m_infoPage))
    {
        disconnect(m_infoPage,
                   SIGNAL(sigInfoPageCnfgBtnClick(int)),
                   this,
                   SLOT(slotInfoPageButtonClicked(int)));
        DELETE_OBJ (m_infoPage);
    }

    if (IS_VALID_OBJ(m_hddCleanInfoTimer))
    {
        if (m_hddCleanInfoTimer->isActive())
        {
            m_hddCleanInfoTimer->stop();
        }

        disconnect(m_hddCleanInfoTimer,
                   SIGNAL(timeout()),
                   this,
                   SLOT(slotHddCleanInfoTimeout()));
        DELETE_OBJ(m_hddCleanInfoTimer);
    }

    if (IS_VALID_OBJ(m_clientAudInclTimer))
    {
        if (m_clientAudInclTimer->isActive())
        {
            m_clientAudInclTimer->stop();
        }

        disconnect(m_clientAudInclTimer,
                   SIGNAL(timeout()),
                   this,
                   SLOT(slotClientAudInclude()));
        DELETE_OBJ(m_clientAudInclTimer);
    }

    if (IS_VALID_OBJ(m_autoTmznRebootInfoPage))
    {
        disconnect(m_autoTmznRebootInfoPage,
                   SIGNAL(sigInfoPageCnfgBtnClick(int)),
                   this,
                   SLOT(slotAutoTmznRbtInfoPgBtnClick(int)));
        DELETE_OBJ(m_autoTmznRebootInfoPage);
    }

    if (IS_VALID_OBJ(m_startLiveViewInfoPage))
    {
        disconnect(m_startLiveViewInfoPage,
                   SIGNAL(sigInfoPageCnfgBtnClick(int)),
                   this,
                   SLOT(slotStartLiveViewInfoPageClick(int)));
        DELETE_OBJ(m_startLiveViewInfoPage);
    }

    DELETE_OBJ(m_autoAddCamInfoPage);
    DELETE_OBJ(m_multiLanguageInfoPage);

    if (IS_VALID_OBJ(m_reportTable))
    {
        disconnect(m_reportTable,
                   SIGNAL(sigClosePage(quint8)),
                   this,
                   SLOT(slotReportTableClose(quint8)));
        DELETE_OBJ(m_reportTable);
    }

    if (IS_VALID_OBJ(m_osdUpdate))
    {
        m_osdUpdate->terminate();
        m_osdUpdate->wait();
        DELETE_OBJ(m_osdUpdate);
    }

    if (IS_VALID_OBJ(m_messageAlert))
    {
        disconnect(m_messageAlert ,
                   SIGNAL(sigCloseAlert()),
                   this,
                   SLOT(slotMessageAlertClose()));
        DELETE_OBJ(m_messageAlert);
    }

    DELETE_OBJ(m_messageBanner);
    DELETE_OBJ(m_validationMessage);
    DELETE_OBJ(m_payloadLib);

    if (IS_VALID_OBJ(m_applController))
    {
        disconnect(m_applController,
                   SIGNAL(sigChangeAudButtonState()),
                   this,
                   SLOT(slotChangeAudButtonState()));
        disconnect(m_applController,
                   SIGNAL(sigHdmiInfoPage(bool)),
                   this,
                   SLOT(slotHdmiInfoPage(bool)));
        disconnect(m_applController,
                   SIGNAL(sigStreamObjectDelete(DISPLAY_TYPE_e*, quint16*)),
                   this,
                   SLOT(slotStreamObjectDelete(DISPLAY_TYPE_e*, quint16*)));
        disconnect(m_applController,
                   SIGNAL(sigDeviceListChangeToGui()),
                   this,
                   SLOT(slotDeviceListChangeToGui()));
        disconnect(m_applController,
                   SIGNAL(sigStreamRequestResponse(STREAM_COMMAND_TYPE_e, StreamRequestParam*, DEVICE_REPLY_TYPE_e)),
                   this,
                   SLOT(slotStreamRequestResponse(STREAM_COMMAND_TYPE_e, StreamRequestParam*, DEVICE_REPLY_TYPE_e)));
        disconnect(m_applController,
                   SIGNAL(sigPopUpEventToGui(QString, quint8, QString, quint32, QString, QString, quint8)),
                   this,
                   SLOT(slotPopUpEvtToGui(QString, quint8, QString, quint32, QString, QString, quint8)));
        disconnect(m_applController,
                   SIGNAL(sigEventToGui(QString, quint8, quint8, quint8, quint8, QString, bool)),
                   this,
                   SLOT(slotEventToGui(QString, quint8, quint8, quint8, quint8, QString, bool)));
        disconnect(m_applController,
                   SIGNAL(sigDevCommGui(QString, DevCommParam*)),
                   this,
                   SLOT(slotDevCommGui(QString, DevCommParam*)));
        disconnect(m_applController,
                   SIGNAL(sigLanguageCfgModified(QString)),
                   this,
                   SLOT(slotLanguageCfgModified(QString)));
        m_applController->deleteAppCntrollrInstance();
        m_applController = NULL;
    }

    if (IS_VALID_OBJ(m_layout))
    {
        disconnect(m_layout,
                   SIGNAL(sigUnloadToolbar()),
                   this,
                   SLOT(slotUnloadToolbar()));
        disconnect(m_layout,
                   SIGNAL(sigHideToolbarPage()),
                   this,
                   SLOT(slotHideToolbarPage()));
        disconnect(m_layout,
                   SIGNAL(sigCloseCosecPopup()),
                   this,
                   SLOT(slotCloseCosecPopUp()));
        disconnect(m_layout,
                   SIGNAL(sigCloseAnalogCameraFeature()),
                   this,
                   SLOT(slotCloseAnalogCameraFeature()));
        disconnect(m_layout,
                   SIGNAL(sigRaiseWidgetOnTopOfAll()),
                   this,
                   SLOT(slotRaiseWidget()));
        disconnect(m_layout,
                   SIGNAL(sigStopClntAud(DEVICE_REPLY_TYPE_e)),
                   this,
                   SLOT(slotStopClntAud(DEVICE_REPLY_TYPE_e)));
        disconnect(m_layout,
                   SIGNAL(sigFindNextRcd (quint16, quint16)),
                   this,
                   SLOT(slotFindNextRecord(quint16, quint16 )));
        disconnect(m_layout,
                   SIGNAL(sigFindPrevRecord(quint16, quint16)),
                   this,
                   SLOT(slotFindPrevRecord(quint16, quint16)));
        disconnect(m_layout,
                   SIGNAL(sigGetNextPrevRecord(quint16, quint16, quint16)),
                   this,
                   SLOT(slotGetNextPrevRecord(quint16, quint16, quint16)));
        disconnect(m_layout,
                   SIGNAL(sigUpdateUIGeometry(bool)),
                   this,
                   SLOT(slotUpdateUIGeometry(bool)));
        DELETE_OBJ(m_layout);
    }
}

void MainWindow::createLayout()
{
    m_layout = new Layout(this);
    connect(m_layout,
            SIGNAL(sigCloseAnalogCameraFeature()),
            this,
            SLOT(slotCloseAnalogCameraFeature()));
    connect(m_layout,
            SIGNAL(sigCloseCosecPopup()),
            this,
            SLOT(slotCloseCosecPopUp()));
    connect(m_layout,
            SIGNAL(sigUnloadToolbar()),
            this,
            SLOT(slotUnloadToolbar()));
    connect(m_layout,
            SIGNAL(sigHideToolbarPage()),
            this,
            SLOT(slotHideToolbarPage()));
    connect(m_layout,
            SIGNAL(sigLoadProcessBar()),
            this,
            SLOT(slotLoadProcessBar()));
    connect(m_layout,
            SIGNAL(sigRaiseWidgetOnTopOfAll()),
            this,
            SLOT(slotRaiseWidget()));
    connect(m_layout,
            SIGNAL(sigStopClntAud(DEVICE_REPLY_TYPE_e)),
            this,
            SLOT(slotStopClntAud(DEVICE_REPLY_TYPE_e)));
    connect(m_layout,
            SIGNAL(sigFindNextRcd(quint16, quint16)),
            this,
            SLOT(slotFindNextRecord(quint16, quint16)));
    connect(m_layout,
            SIGNAL(sigFindPrevRecord(quint16, quint16)),
            this,
            SLOT(slotFindPrevRecord(quint16, quint16)));
    connect(m_layout,
            SIGNAL(sigGetNextPrevRecord(quint16, quint16, quint16)),
            this,
            SLOT(slotGetNextPrevRecord(quint16, quint16, quint16)));
    connect(m_layout,
            SIGNAL(sigUpdateUIGeometry(bool)),
            this,
            SLOT(slotUpdateUIGeometry(bool)),
            Qt::QueuedConnection);
}

void MainWindow::createToolbar()
{
    m_toolbar = new Toolbar(this);
    connect(m_toolbar,
            SIGNAL(sigOpenToolbarPage(TOOLBAR_BUTTON_TYPE_e)),
            this,
            SLOT(slotOpenToolbarPage(TOOLBAR_BUTTON_TYPE_e)));
    connect(m_toolbar,
            SIGNAL(sigCloseToolbarPage(TOOLBAR_BUTTON_TYPE_e)),
            this,
            SLOT(slotCloseToolbarPage(TOOLBAR_BUTTON_TYPE_e)));
    connect(this,
            SIGNAL(sigNotifyStatusIcon(TOOLBAR_BUTTON_TYPE_e, bool)),
            m_toolbar,
            SLOT(slotNotifyStatusIcon(TOOLBAR_BUTTON_TYPE_e, bool)));
    connect(this,
            SIGNAL(sigChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e, STATE_TYPE_e)),
            m_toolbar,
            SLOT(slotChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e, STATE_TYPE_e)));
    connect(m_layout,
            SIGNAL(sigChangePageFromLayout()),
            m_toolbar,
            SLOT(slotChangePageFromLayout()),
            Qt::QueuedConnection);
    connect(m_layout,
            SIGNAL(sigChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e, STATE_TYPE_e)),
            m_toolbar,
            SLOT(slotChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e, STATE_TYPE_e)));
    connect(m_toolbar,
            SIGNAL(sigChangePage(NAVIGATION_TYPE_e)),
            m_layout,
            SLOT(slotChangePage(NAVIGATION_TYPE_e)));
    connect(m_toolbar,
            SIGNAL(sigToolbarApplyStyle(DISPLAY_TYPE_e, DISPLAY_CONFIG_t, STYLE_TYPE_e)),
            m_layout,
            SLOT(slotDispSettingApplyChanges(DISPLAY_TYPE_e, DISPLAY_CONFIG_t, STYLE_TYPE_e)));
    connect(m_layout,
            SIGNAL(sigToolbarStyleChnageNotify(STYLE_TYPE_e)),
            m_toolbar,
            SLOT(slotStyleChnagedNotify(STYLE_TYPE_e)));
    connect(m_layout,
            SIGNAL(sigShowHideStyleAndPageControl(bool, quint8)),
            m_toolbar,
            SLOT(slotShowHideStyleAndPageControl(bool, quint8)));
    connect(m_toolbar,
            SIGNAL(sigExitVideoPopupState()),
            m_layout,
            SLOT(slotExitVideoPopupState()));
    connect(m_toolbar,
            SIGNAL(sigOpenWizard()),
            this,
            SLOT(slotOpenWizard()));
}

void MainWindow::updateUsbStatus()
{
    quint8 usbStatus[MAX_CAMERAS];

    if (false == m_applController->GetHlthStatusSingleParam(LOCAL_DEVICE_NAME, usbStatus, USB_STS))
    {
        return;
    }

    for (int index = 0; index < MAX_USB_TYPE; index++)
    {
        UsbControl::updateShowUsbFlag((index + 1), ((usbStatus[index] == 0) ? false : true));
    }

    if (UsbControl::currentNoOfUsb != 0)
    {
        emit sigNotifyStatusIcon(USB_CONTROL_BUTTON, true);
    }
}

void MainWindow::updateAudioStatus()
{
    AUDIO_CONFIG_t audioConfig;
    QList<QVariant> paramList;

    memset(&audioConfig, 0, sizeof(audioConfig));
    paramList.append(READ_AUDIO_ACTIVITY);

    if (m_applController->processActivity(AUDIO_SETTING, paramList, &audioConfig))
    {
        STATE_TYPE_e state = (audioConfig.muteStatus == AUDIO_MUTE ? STATE_2 : STATE_1);
        emit sigChangeToolbarButtonState(AUDIO_CONTROL_BUTTON, state);
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent * event)
{
    qint32 tempHeight = ApplController::getYPosOfScreen() + ApplController::getHeightOfScreen();

    if ((event->y() >= (tempHeight - TOOLBAR_BUTTON_HEIGHT)) && (event->y() <= tempHeight))
    {
        if ((ApplicationMode::getApplicationMode() == IDLE_MODE) && (Layout::cosecPopupEvtData.m_currentFeatureState == FEATURE_STATE_NONE)
                && (Layout::currentModeType[MAIN_DISPLAY] != STATE_SYNC_PLAYBACK_MODE)
                && (Layout::analogFeatureDataType.m_currentFeatureState == FEATURE_STATE_NONE))
        {
            if ((m_toolbar != NULL) && (!m_toolbar->isVisible()))
            {
                m_toolbar->setVisible(true);
                m_toolbar->getDateAndTime(LOCAL_DEVICE_NAME);
            }
        }
    }
    else
    {
        if (ApplicationMode::getApplicationMode() == TOOLBAR_MODE)
        {
            if ((m_toolbar != NULL) && (m_toolbar->isVisible()))
            {
                m_toolbar->setVisible(false);
            }
        }
    }
}

void MainWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(QColor(TRANSPARENTKEY_COLOR), Qt::NoBrush));
    painter.drawRect(ApplController::getXPosOfScreen(), ApplController::getYPosOfScreen(),
                     ApplController::getWidthOfScreen(), ApplController::getHeightOfScreen());
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (IS_VALID_OBJ(m_initDeInitMsgBox))
    {
        return;
    }

    if (((ApplicationMode::getApplicationMode() == IDLE_MODE) || (ApplicationMode::getApplicationMode() == TOOLBAR_MODE))
            && (Layout::currentModeType[MAIN_DISPLAY] == STATE_OTHER_MODE_NONE) && (Layout::isAnyWindowReplacing(MAIN_DISPLAY) == false))
    {
        switch (event->key())
        {
            case Qt::Key_Up :
            case Qt::Key_Down:
            case Qt::Key_Left :
            case Qt::Key_Right:
            {
                event->accept();
                if (ApplicationMode::getApplicationMode() == IDLE_MODE)
                {
                    if (m_toolbar->isVisible())
                    {
                        m_toolbar->unloadToolbarPage();
                    }
                    m_layout->navigationKeyPressed(event);
                }
            }
            break;

            case Qt::Key_F1:        // key is 'display' key
            {
                event->accept();
                m_toolbar->loadToolbarPage(DISPLAY_MODE_BUTTON);
            }
            break;

            case Qt::Key_F2:        // key is 'search' key
            {
                event->accept();
                m_toolbar->loadToolbarPage(SYNC_PLAYBACK_BUTTON);
            }
            break;

            case Qt::Key_F3:        // key is 'record' key
            {
                event->accept();
                m_toolbar->unloadToolbarPage();
                m_layout->startStopManualRecording(Layout::currentDisplayConfig[MAIN_DISPLAY].selectedWindow);
            }
            break;

            case  Qt::Key_F4:       // load/Unload toolbar
            {
                event->accept();
                if ((Layout::streamInfoArray[MAIN_DISPLAY][Layout::currentDisplayConfig[MAIN_DISPLAY].selectedWindow].m_videoType == VIDEO_TYPE_PLAYBACKSTREAM)
                        || (Layout::streamInfoArray[MAIN_DISPLAY][Layout::currentDisplayConfig[MAIN_DISPLAY].selectedWindow].m_videoType == VIDEO_TYPE_INSTANTPLAYBACKSTREAM))
                {
                    m_layout->slotWindowSelected(Layout::currentDisplayConfig[MAIN_DISPLAY].selectedWindow);
                }
                else
                {
                    if (m_toolbar->isVisible())
                    {
                        m_toolbar->unloadToolbarPage();
                    }
                    else
                    {
                        m_toolbar->setVisible(true);
                        m_toolbar->getDateAndTime(LOCAL_DEVICE_NAME);
                    }
                }
            }
            break;

            case Qt::Key_F6:        // Instant playback
            {
                event->accept();
                if (Layout::streamInfoArray[MAIN_DISPLAY][Layout::currentDisplayConfig[MAIN_DISPLAY].selectedWindow].m_videoType != VIDEO_TYPE_INSTANTPLAYBACKSTREAM)
                {
                    m_layout->slotLiveViewToolbarButtonClicked(LIVEVIEW_INSTANTPLAYBACK_BUTTON, STATE_1, Layout::currentDisplayConfig[MAIN_DISPLAY].selectedWindow);
                }
            }
            break;

            case Qt::Key_F7:        // snapshot
            {
                event->accept();
                m_layout->slotLiveViewToolbarButtonClicked(LIVEVIEW_SNAPSHOT_BUTTON, STATE_1, Layout::currentDisplayConfig[MAIN_DISPLAY].selectedWindow);
            }
            break;

            case Qt::Key_F8:        // Auto page navigation
            {
                event->accept();
                m_toolbar->loadToolbarPage(SEQUENCE_BUTTON);
            }
            break;

            case Qt::Key_F11:
            {
                event->accept();
                m_layout->mouseDoubleClicked(Layout::currentDisplayConfig[MAIN_DISPLAY].selectedWindow);
            }
            break;

            case Qt::Key_Escape:
            case Qt::Key_Delete:
            {
                event->accept();

                if (event->key() == Qt::Key_Escape)
                {
                    m_toolbar->setVisible(false);
                }
            }
            break;

            default:
            {
                QWidget::keyPressEvent(event);
            }
            break;
        }
    }
}

void MainWindow::getAutoCnfgFlag()
{
    QString payloadString = m_payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                               GENERAL_TABLE_INDEX,
                                                               CNFG_FRM_INDEX,
                                                               1,
                                                               FIELD_AUTO_CONFIG + 1,
                                                               FIELD_AUTO_CONFIG + 1,
                                                               1);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_GET_CFG;
    param->payload = payloadString;
    m_isAutoCnfgChecked = true;
    m_applController->processActivity(LOCAL_DEVICE_NAME, DEVICE_COMM, param);
}

void MainWindow::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    switch (param->msgType)
    {
        case MSG_GET_CFG:
        {
            if (param->deviceStatus != CMD_SUCCESS)
            {
                break;
            }

            m_payloadLib->parsePayload(param->msgType, param->payload);
            quint32 tableId = m_payloadLib->getcnfgTableIndex();
            DPRINT(DEVICE_CLIENT, "get config msg in main window: [device=%s], [tableId=%d]", deviceName.toUtf8().constData(), tableId);

            switch (tableId)
            {
                case GENERAL_TABLE_INDEX:
                {
                    if (m_isAutoTmZnUpdtRunning)
                    {
                        /* We are checking this param, it is in between video standard and date fmt so need to remember */
                        m_currAutoTmZnParam[VIDEO_STD] = m_payloadLib->getCnfgArrayAtIndex(0).toUInt();
                        m_currAutoTmZnParam[COSEC_INT] = m_payloadLib->getCnfgArrayAtIndex(1).toUInt();
                        m_currAutoTmZnParam[DATE_FMT] = m_payloadLib->getCnfgArrayAtIndex(2).toUInt();

                        m_detectedAutoTmZnParam[COSEC_INT] = m_currAutoTmZnParam[COSEC_INT];
                        if (m_currAutoTmZnParam[VIDEO_STD] != m_detectedAutoTmZnParam[VIDEO_STD])
                        {
                            DEV_TABLE_INFO_t devTableInfo;
                            m_applController->GetDeviceInfo(LOCAL_DEVICE_NAME, devTableInfo);
                            if (devTableInfo.analogCams == 0)
                            {
                                setAutoTimeZoneIndex(true);
                                break;
                            }

                            if (m_autoTmznRebootInfoPage == NULL)
                            {
                                m_autoTmznRebootInfoPage = new InfoPage(ApplController::getXPosOfScreen(), ApplController::getYPosOfScreen(),
                                                                        ApplController::getWidthOfScreen(), ApplController::getHeightOfScreen(),
                                                                        MAX_INFO_PAGE_TYPE, this, true, false);
                                connect(m_autoTmznRebootInfoPage,
                                        SIGNAL(sigInfoPageCnfgBtnClick(int)),
                                        this,
                                        SLOT(slotAutoTmznRbtInfoPgBtnClick(int)));
                            }

                            m_autoTmznRebootInfoPage->loadInfoPage(ValidationMessage::getValidationMessage(DATE_TIME_CNG_CFG_VIDEO_STD), true);
                        }
                        else if (m_currAutoTmZnParam[DATE_FMT] != m_detectedAutoTmZnParam[DATE_FMT])
                        {
                            setAutoTimeZoneIndex(true);
                        }
                    }
                    else
                    {
                        if (m_payloadLib->getCnfgArrayAtIndex(0).toUInt() == 1)
                        {
                            if (IS_VALID_OBJ(m_autoAddCam))
                            {
                                break;
                            }

                            /* PARASOFT: Memory Dealloacted in slot AutoAddClose */
                            m_autoAddCam = new MxAutoAddCam(this);
                            connect(m_autoAddCam ,
                                    SIGNAL(sigCloseAlert()),
                                    this,
                                    SLOT(slotAutoAddClose()));
                            connect(m_autoAddCam,
                                    SIGNAL(sigAutoAddCamPopUpAction(MX_AUTO_ADD_CAM_SEARCH_ACTION_e)),
                                    this,
                                    SLOT(slotAutoAddCamPopUpAction(MX_AUTO_ADD_CAM_SEARCH_ACTION_e)));
                            isOnbootAuoCamSearchRunning = true;
                            if (m_infoPage->isVisible())
                            {
                                m_autoAddCam->setIsInfoPageLoaded(true);
                            }
                        }
                        else
                        {
                            DEV_TABLE_INFO_t deviceInfo;
                            m_applController->GetDeviceInfo(LOCAL_DEVICE_NAME, deviceInfo);
                            if (true == deviceInfo.startLiveView)
                            {
                                break;
                            }

                            m_isStartLiveView = true;
                            if (m_startLiveViewInfoPage == NULL)
                            {
                                m_startLiveViewInfoPage = new InfoPage(ApplController::getXPosOfScreen(), ApplController::getYPosOfScreen(),
                                                                       ApplController::getWidthOfScreen(), ApplController::getHeightOfScreen(),
                                                                       MAX_INFO_PAGE_TYPE, this, true, false);
                                connect(m_startLiveViewInfoPage,
                                        SIGNAL(sigInfoPageCnfgBtnClick(int)),
                                        this,
                                        SLOT(slotStartLiveViewInfoPageClick(int)));
                            }

                            m_startLiveViewInfoPage->loadInfoPage(MAIN_WINDOW_STARTLIVEVIEWMESSAGE(), true, true, "", "Yes", "No");
                        }
                    }
                }
                break;

                case DATETIME_TABLE_INDEX:
                {
                    if (false == m_isAutoTmZnUpdtRunning)
                    {
                        break;
                    }

                    m_currAutoTmZnParam[TIMEZONE_INDEX] = m_payloadLib->getCnfgArrayAtIndex(FIELD_TIME_ZONE).toUInt();
                    if (m_currAutoTmZnParam[TIMEZONE_INDEX] == m_detectedAutoTmZnParam[TIMEZONE_INDEX])
                    {
                        break;
                    }

                    if (((OPTION_STATE_TYPE_e)(m_payloadLib->getCnfgArrayAtIndex(FIELD_AUTO_UPDATE_REGIONAL).toUInt()) == ON_STATE))
                    {
                        getVideoStdDtFrmt();
                    }
                    else
                    {
                        setAutoTimeZoneIndex(false);
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
            if (m_isAutoTmZnUpdtRunning)
            {
                m_isAutoTmZnUpdtRunning = false;
            }
        }
        break;

        case MSG_SET_CMD:
        {
            if ((param->cmdType != AUTO_CFG_STATUS_RPRT) && (param->cmdType != GET_USER_LANGUAGE))
            {
                break;
            }

            if (param->deviceStatus != CMD_SUCCESS)
            {
                MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                break;
            }

            m_payloadLib->parseDevCmdReply(true, param->payload);

            if (param->cmdType == GET_USER_LANGUAGE)
            {
                slotLanguageCfgModified(m_payloadLib->getCnfgArrayAtIndex(0).toString());
            }
            else if (param->cmdType == AUTO_CFG_STATUS_RPRT)
            {
                QStringList reportList;
                quint8 totalReports = (m_payloadLib->getTotalCmdFields() / MAX_MX_CMD_AUTO_CFG_STATUS_RPRT);

                for (quint8 index = 0; index < totalReports; index++)
                {
                    reportList.append(INT_TO_QSTRING(index+1));
                    reportList.append(m_payloadLib->getCnfgArrayAtIndex((index * MAX_MX_CMD_AUTO_CFG_STATUS_RPRT) + MX_CMD_FIELDS_DETECTED_IP_ADDR).toString());
                    quint8 status = m_payloadLib->getCnfgArrayAtIndex((index * MAX_MX_CMD_AUTO_CFG_STATUS_RPRT) + MX_CMD_FEILDS_AUTO_CNFG_STATUS).toUInt();
                    if (status == SUCCESS)
                    {
                        reportList.append("Success");
                        reportList.append(Multilang("Added with ") +
                                          (m_payloadLib->getCnfgArrayAtIndex((index * MAX_MX_CMD_AUTO_CFG_STATUS_RPRT) + MX_CMD_FEILDS_CHANGED_IP_ADDR).toString()) +
                                          " " + Multilang("at Index") + ": " +
                                          (m_payloadLib->getCnfgArrayAtIndex((index * MAX_MX_CMD_AUTO_CFG_STATUS_RPRT) + MX_CMD_FEILDS_CAM_INDEX).toString()));
                    }
                    else
                    {
                        reportList.append("Failed");
                        quint8 failReason = m_payloadLib->getCnfgArrayAtIndex((index * MAX_MX_CMD_AUTO_CFG_STATUS_RPRT) + MX_CMD_FEILDS_CNFG_FAIL_REASON).toUInt();
                        switch (failReason)
                        {
                            case 0:
                                reportList.append("Authentication failed");
                                break;

                            case 1:
                                reportList.append("No free index available");
                                break;

                            case 2:
                                reportList.append("Request failed.");
                                break;

                            case 3:
                                reportList.append("IP Address unavailable in defined range");
                                break;

                            case 4:
                                reportList.append("Camera and IP Address range are in different IP Address families");
                                break;

                            default:
                                break;
                        }
                    }
                }

                if (m_reportTable == NULL)
                {
                    QList<quint16> widthList;
                    QStringList strList;

                    widthList << SCALE_WIDTH(40) << SCALE_WIDTH(340) << SCALE_WIDTH(80) << SCALE_WIDTH(620);
                    strList << "No." << "Detected IP Address" << "Status" << "Reason";
                    m_reportTable = new MxReportTable(50, (widthList.count()), 10, "Auto Configure Camera - Status Report",
                                                      widthList, strList, this, m_reportCurPageNo, SCALE_HEIGHT(35));
                    connect(m_reportTable,
                            SIGNAL(sigClosePage(quint8)),
                            this,
                            SLOT(slotReportTableClose(quint8)));
                }

                m_reportTable->raise();
                m_reportTable->showReport(reportList);
                slotRaiseWidget();
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

void MainWindow::getAutoUpdtRegFlg()
{
    QString payloadString = m_payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                               DATETIME_TABLE_INDEX,
                                                               CNFG_FRM_INDEX,
                                                               1,
                                                               CNFG_FRM_FIELD,
                                                               MAX_DATE_TIME_FIELD_NO,
                                                               0);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_GET_CFG;
    param->payload = payloadString;
    m_applController->processActivity(LOCAL_DEVICE_NAME, DEVICE_COMM, param);
}

void MainWindow::getVideoStdDtFrmt()
{
    QString payloadString = m_payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                               GENERAL_TABLE_INDEX,
                                                               CNFG_FRM_INDEX,
                                                               1,
                                                               FIELD_VIDEO_STD + 1,
                                                               FIELD_DATE_FORMAT + 1,
                                                               3);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_GET_CFG;
    param->payload = payloadString;
    m_applController->processActivity(LOCAL_DEVICE_NAME, DEVICE_COMM, param);
}

void MainWindow::setAutoTimeZoneIndex(bool isAutoUpdtRegSetting)
{
    m_payloadLib->setCnfgArrayAtIndex(TIMEZONE_INDEX,m_detectedAutoTmZnParam[TIMEZONE_INDEX]);

    if ((isAutoUpdtRegSetting) && ((m_currAutoTmZnParam[VIDEO_STD] != m_detectedAutoTmZnParam[VIDEO_STD]) ||
                                  (m_currAutoTmZnParam[DATE_FMT] != m_detectedAutoTmZnParam[DATE_FMT])))
    {
        m_payloadLib->setCnfgArrayAtIndex(VIDEO_STD, m_detectedAutoTmZnParam[VIDEO_STD]);
        m_payloadLib->setCnfgArrayAtIndex(COSEC_INT, m_detectedAutoTmZnParam[COSEC_INT]);
        m_payloadLib->setCnfgArrayAtIndex(DATE_FMT, m_detectedAutoTmZnParam[DATE_FMT]);
    }

    QString payloadString = m_payloadLib->createDevCnfgPayload(MSG_SET_CFG,
                                                               DATETIME_TABLE_INDEX,
                                                               CNFG_FRM_INDEX,
                                                               1,
                                                               FIELD_TIME_ZONE + 1,
                                                               FIELD_TIME_ZONE + 1,
                                                               1);

    if ((isAutoUpdtRegSetting) && ((m_currAutoTmZnParam[VIDEO_STD] != m_detectedAutoTmZnParam[VIDEO_STD]) ||
                                  (m_currAutoTmZnParam[DATE_FMT] != m_detectedAutoTmZnParam[DATE_FMT])))
    {
        payloadString = m_payloadLib->createDevCnfgPayload(MSG_SET_CFG,
                                                           GENERAL_TABLE_INDEX,
                                                           CNFG_FRM_INDEX,
                                                           1,
                                                           FIELD_VIDEO_STD + 1,
                                                           FIELD_DATE_FORMAT + 1,
                                                           (FIELD_TIME_FORMAT - FIELD_VIDEO_STD),
                                                           payloadString,
                                                           FIELD_TIME_ZONE + 1);
    }

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CFG;
    param->payload = payloadString;
    m_applController->processActivity(LOCAL_DEVICE_NAME, DEVICE_COMM, param);
}

void MainWindow::sendClntAudCmd(SET_COMMAND_e cmdId, DEVICE_REPLY_TYPE_e deviceReply)
{
    StreamRequestParam *streamRequestParam = NULL;
    SERVER_SESSION_INFO_t serverSessionInfo;

    serverSessionInfo.serverInfo.ipAddress = "";
    serverSessionInfo.serverInfo.tcpPort = DFLT_TCP_PORT;
    serverSessionInfo.sessionInfo.sessionId = "";
    serverSessionInfo.sessionInfo.timeout = 0;

    if (false == m_applController->GetServerSessionInfo(LOCAL_DEVICE_NAME, serverSessionInfo))
    {
        EPRINT(DEVICE_CLIENT, "fail to get server info for client audio");
    }

    if (cmdId == STRT_CLNT_AUDIO)
    {
        DPRINT(DEVICE_CLIENT, "send start client audio request: [sessionId=%s]", serverSessionInfo.sessionInfo.sessionId.toUtf8().constData());
        streamRequestParam = new StreamRequestParam();
        streamRequestParam->deviceName = LOCAL_DEVICE_NAME;
        streamRequestParam->windowId = CLIENT_AUDIO_DECODER_ID;
        m_applController->processStreamActivity(STRT_CLNT_AUDIO_COMMAND, serverSessionInfo, streamRequestParam);
    }
    else if (cmdId == STOP_CLNT_AUDIO)
    {
        DPRINT(DEVICE_CLIENT, "send stop client audio request: [sessionId=%s]", serverSessionInfo.sessionInfo.sessionId.toUtf8().constData());
        m_payloadLib->setCnfgArrayAtIndex(0, deviceReply);

        /* Check whether stream request instance was created or not? If created then send STOP_CLNT_AUD command through that stream request instance,
         * which in return,delete stream request instance. If not created then send cmd independently. */
        if (m_clntAudStreamId != MAX_STREAM_SESSION)
        {
            streamRequestParam = new StreamRequestParam();
            streamRequestParam->deviceName = LOCAL_DEVICE_NAME;
            streamRequestParam->windowId = CLIENT_AUDIO_DECODER_ID;
            streamRequestParam->payload = m_payloadLib->createDevCmdPayload(1);
            streamRequestParam->streamId = m_clntAudStreamId;
            m_applController->processStreamActivity(STOP_CLNT_AUDIO_COMMAND, serverSessionInfo, streamRequestParam);
        }
        else
        {
            DevCommParam* param = new DevCommParam();
            param->msgType = MSG_SET_CMD;
            param->cmdType = STOP_CLNT_AUDIO;
            param->payload = m_payloadLib->createDevCmdPayload(1);
            m_applController->processActivity(LOCAL_DEVICE_NAME, DEVICE_COMM, param);
        }
    }
}

void MainWindow::processClntStrmReqRes(SET_COMMAND_e commandId, DEVICE_REPLY_TYPE_e statusId)
{
    switch (commandId)
    {
        case STRT_CLNT_AUDIO:
        {
            if (statusId == CMD_SUCCESS)
            {
                m_isClntAudStartedInInstPB = false;
                m_audioStatusBeforeClntAud = false;

                if (m_layout == NULL)
                {
                    break;
                }

                m_layout->m_isClientAudProcessRunning = true;
                if (IS_VALID_OBJ(m_syncPlayback))
                {
                    if (m_syncPlayback->getSyncPBAudPlayStatus() == true)
                    {
                        m_syncPlayback->stopCamAudAfterClntAud();
                    }
                    else
                    {
                        m_syncPlayback->setClientAudioRunningFlag(true);
                        m_clientAudInclretryCnt = 0;
                        if (IS_VALID_OBJ(m_clientAudInclTimer))
                        {
                            m_clientAudInclTimer->start();
                        }
                    }
                }
                else
                {
                    quint16 curAudioWindow = m_layout->getStartAudioInWindow();
                    if (curAudioWindow != MAX_CHANNEL_FOR_SEQ)
                    {
                        VIDEO_STREAM_TYPE_e videoType = Layout::streamInfoArray[MAIN_DISPLAY][curAudioWindow].m_videoType;
                        m_audioStatusBeforeClntAud = true;

                        if ((videoType == VIDEO_TYPE_LIVESTREAM) || (videoType == VIDEO_TYPE_LIVESTREAM_AWAITING))
                        {
                            MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(TWO_WAY_CAM_AUD_STP_PRO_CLNT_AUD));
                        }
                        else
                        {
                            MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(TWO_WAY_PLYBK_AUD_STP_PRO_CLNT_AUD));
                        }

                        if ((videoType == VIDEO_TYPE_INSTANTPLAYBACKSTREAM) && (m_layout->getlastEnableAudioWindow() == MAX_CHANNEL_FOR_SEQ))
                        {
                            m_isClntAudStartedInInstPB = true;
                        }

                        m_layout->setlastEnableAudioWindow(curAudioWindow);
                        m_layout->includeExcludeAudio(curAudioWindow, true);
                    }
                    else
                    {
                        /* If audio is on in live view, instantPB started with audio, if user has stopped audio,
                         * now clnt audio started and stopped, at that time if instPB going on then we don't have to start audio. */
                        m_audioStatusBeforeClntAud = false;
                        m_clientAudInclretryCnt = 0;
                        if (IS_VALID_OBJ(m_clientAudInclTimer))
                        {
                            m_clientAudInclTimer->start();
                        }
                    }
                }
            }
            else
            {
                if ((IS_VALID_OBJ(m_layout)) && (m_layout->m_isClientAudProcessRunning == true))
                {
                    if ((IS_VALID_OBJ(m_clientAudInclTimer)) && (m_clientAudInclTimer->isActive()))
                    {
                        m_clientAudInclTimer->stop();
                    }

                    ExcludeAudio(CLIENT_AUDIO_DECODER_ID);
                    if (IS_VALID_OBJ(m_syncPlayback))
                    {
                        m_syncPlayback->startCamAudAfterClntAud();
                    }
                    else
                    {
                        quint16 lastAudWindow = m_layout->getlastEnableAudioWindow();
                        if ((lastAudWindow != MAX_CHANNEL_FOR_SEQ))
                        {
                            quint16 windowLimit[2] = {0,MAX_CHANNEL_FOR_SEQ};
                            m_layout->getFirstAndLastWindow(Layout::currentDisplayConfig[MAIN_DISPLAY].currPage,
                                                            Layout::currentDisplayConfig[MAIN_DISPLAY].layoutId, windowLimit);

                            if (((m_layout->currentModeType[MAIN_DISPLAY] >= STATE_EXPAND_WINDOW_START_MODE) &&
                                 (m_layout->currentModeType[MAIN_DISPLAY] <= STATE_EXPAND_WINDOW)) ||
                                    (m_layout->currentModeType[MAIN_DISPLAY] == STATE_VIDEO_POPUP_FEATURE))
                            {
                                m_layout->setlastEnableAudioWindow(lastAudWindow);
                            }
                            else if ((lastAudWindow >= windowLimit[0]) && (lastAudWindow <= windowLimit[1]))
                            {
                                if (Layout::streamInfoArray[MAIN_DISPLAY][lastAudWindow].m_errorType == VIDEO_ERROR_NONE)
                                {
                                    /* If it is instant PB and audio is started in liveview, then we have to remain lastAudioInWindow variable as it is. */
                                    if ((Layout::streamInfoArray[MAIN_DISPLAY][lastAudWindow].m_videoType == VIDEO_TYPE_INSTANTPLAYBACKSTREAM))
                                    {
                                        if (m_isClntAudStartedInInstPB == true)
                                        {
                                            m_layout->includeExcludeAudio(lastAudWindow, false);
                                        }
                                        else
                                        {
                                            /* If audio is on in live view,instantPB started with audio, if user has stopped audio,
                                             * now clnt audio started and stopped, at that time if instPB going on then we don't have to start audio. */
                                            if (m_audioStatusBeforeClntAud == true)
                                            {
                                                m_layout->includeExcludeAudio(lastAudWindow, true);
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (m_isClntAudStartedInInstPB == true)
                                        {
                                            m_layout->setlastEnableAudioWindow(MAX_CHANNEL_FOR_SEQ);
                                        }
                                        else
                                        {
                                            m_layout->includeExcludeAudio(lastAudWindow, false);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                    statusId = CMD_AUD_SND_REQ_FAIL;
                }

                m_clntAudStreamId = MAX_STREAM_SESSION;
                sendClntAudCmd(STOP_CLNT_AUDIO, statusId);
                if (IS_VALID_OBJ(m_layout))
                {
                    m_layout->m_isClientAudProcessRunning = false;
                }
            }
        }
        break;

        case STOP_CLNT_AUDIO:
        {
            m_clntAudStreamId = MAX_STREAM_SESSION;
            if ((statusId != CMD_SUCCESS) || (m_layout == NULL) || (m_layout->m_isClientAudProcessRunning == false))
            {
                break;
            }

            m_layout->m_isClientAudProcessRunning = false;
            if (IS_VALID_OBJ(m_clientAudInclTimer))
            {
                if (m_clientAudInclTimer->isActive())
                {
                    m_clientAudInclTimer->stop();
                }
            }

            ExcludeAudio(CLIENT_AUDIO_DECODER_ID);

            if (IS_VALID_OBJ(m_syncPlayback))
            {
                m_syncPlayback->startCamAudAfterClntAud();
                break;
            }

            quint16 lastAudWindow = m_layout->getlastEnableAudioWindow();
            if (lastAudWindow == MAX_CHANNEL_FOR_SEQ)
            {
                break;
            }

            quint16 windowLimit[2] = {0, MAX_CHANNEL_FOR_SEQ};
            m_layout->getFirstAndLastWindow(Layout::currentDisplayConfig[MAIN_DISPLAY].currPage,
                                            Layout::currentDisplayConfig[MAIN_DISPLAY].layoutId, windowLimit);

            if (((m_layout->currentModeType[MAIN_DISPLAY] >= STATE_EXPAND_WINDOW_START_MODE) &&
                (m_layout->currentModeType[MAIN_DISPLAY] <= STATE_EXPAND_WINDOW)) ||
                    (m_layout->currentModeType[MAIN_DISPLAY] == STATE_VIDEO_POPUP_FEATURE))
            {
                m_layout->setlastEnableAudioWindow(lastAudWindow);
            }
            else if ((lastAudWindow >= windowLimit[0]) && (lastAudWindow <= windowLimit[1]))
            {
                if (Layout::streamInfoArray[MAIN_DISPLAY][lastAudWindow].m_errorType != VIDEO_ERROR_NONE)
                {
                    break;
                }

                /* If it is instant PB and audio is started in liveview, then we have to remain lastAudioInWindow variable as it is. */
                if ((Layout::streamInfoArray[MAIN_DISPLAY][lastAudWindow].m_videoType == VIDEO_TYPE_INSTANTPLAYBACKSTREAM))
                {
                    if (m_isClntAudStartedInInstPB == true)
                    {
                        m_layout->includeExcludeAudio(lastAudWindow, false);
                    }
                    else
                    {
                        /* If audio is on in live view, instantPB started with audio, if user has stopped audio,
                         * now clnt audio started and stopped, at that time if instPB going on then we don't have to start audio. */
                        if (m_audioStatusBeforeClntAud == true)
                        {
                            m_layout->includeExcludeAudio(lastAudWindow, true);
                        }
                    }
                }
                else
                {
                    if (m_isClntAudStartedInInstPB == true)
                    {
                        m_layout->setlastEnableAudioWindow(MAX_CHANNEL_FOR_SEQ);
                    }
                    else
                    {
                        m_layout->includeExcludeAudio(lastAudWindow, false);
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

void MainWindow::createInitDeinitBox(QString msgDisplay)
{
    if (m_initDeInitMsgBox == NULL)
    {
        /* PARASOFT: Memory Deallocated in delete InitDeInit Box */
        m_initDeInitMsgBox = new Rectangle(ApplController::getXPosOfScreen(), ApplController::getYPosOfScreen(),
                                           ApplController::getWidthOfScreen(), ApplController::getHeightOfScreen(),
                                           0, SHADOW_FONT_COLOR, SHADOW_FONT_COLOR, this);
    }

    if (m_initDeInitMsg == NULL)
    {
        /* PARASOFT: Memory Deallocated in delete InitDeInit Box */
        m_initDeInitMsg = new TextLabel(m_initDeInitMsgBox->width()/2, m_initDeInitMsgBox->height()/2, SCALE_FONT(28), msgDisplay,
                                        this, HIGHLITED_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_CENTRE_X_CENTER_Y);
    }
}

void MainWindow::deleteInitDeInitBox()
{
    DELETE_OBJ(m_initDeInitMsg);
    DELETE_OBJ(m_initDeInitMsgBox);
}

void MainWindow::slotOpenToolbarPage(TOOLBAR_BUTTON_TYPE_e index)
{
    ApplicationMode::setApplicationMode(PAGE_MODE);

    switch (index)
    {
        case ABOUT_US_BUTTON:
        {
            if (IS_VALID_OBJ(m_aboutUs))
            {
                break;
            }

            /* PARASOFT: Memory Deallocated in slot CloseToolbarPage */
            m_aboutUs = new AboutUs(this);
            m_toolbarPage = m_aboutUs;
            connect(m_aboutUs,
                    SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                    m_toolbar,
                    SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
        }
        break;

        case LIVE_VIEW_BUTTON:
        {
            if (IS_VALID_OBJ(m_displaySetting))
            {
                break;
            }

            /* PARASOFT: Memory Deallocated in slot CloseToolbarPage */
            m_displaySetting = new DisplaySetting(this);
            m_toolbarPage = m_displaySetting;
            connect(m_displaySetting,
                    SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                    m_toolbar,
                    SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
            connect(m_displaySetting,
                    SIGNAL(sigChangeLayout(LAYOUT_TYPE_e, DISPLAY_TYPE_e, quint16, bool, bool)),
                    m_layout,
                    SLOT(slotChangeLayout(LAYOUT_TYPE_e, DISPLAY_TYPE_e, quint16, bool, bool)));
            connect(m_layout,
                    SIGNAL(sigWindowResponseToDisplaySettingsPage(DISPLAY_TYPE_e, QString, quint8, quint16)),
                    m_displaySetting,
                    SLOT(slotWindowResponseToDisplaySettingsPage(DISPLAY_TYPE_e, QString, quint8, quint16)));
            connect(m_displaySetting,
                    SIGNAL(sigProcessApplyStyleToLayout(DISPLAY_TYPE_e, DISPLAY_CONFIG_t, STYLE_TYPE_e)),
                    m_layout,
                    SLOT(slotDispSettingApplyChanges(DISPLAY_TYPE_e, DISPLAY_CONFIG_t, STYLE_TYPE_e)));
            connect(m_layout,
                    SIGNAL(sigDisplaySettingApplyChangesNotify(DISPLAY_TYPE_e, bool)),
                    m_displaySetting,
                    SLOT(slotLayoutResponseOnApply(DISPLAY_TYPE_e, bool)));
            connect(m_displaySetting,
                    SIGNAL(sigToolbarStyleChnageNotify(STYLE_TYPE_e)),
                    m_toolbar,
                    SLOT(slotStyleChnagedNotify(STYLE_TYPE_e)));
            connect(m_displaySetting,
                    SIGNAL(sigliveViewAudiostop()),
                    m_layout,
                    SLOT(slotliveViewAudiostop()));
        }
        break;

        case DISPLAY_MODE_BUTTON:
        {
            if (IS_VALID_OBJ(m_displayMode))
            {
                break;
            }

            /* PARASOFT: Memory Deallocated in slot CloseToolbarPage */
            m_displayMode = new DisplayMode(this);
            m_toolbarPage = m_displayMode;
            connect(m_displayMode,
                    SIGNAL(sigApplyNewLayout(DISPLAY_TYPE_e, DISPLAY_CONFIG_t, STYLE_TYPE_e)),
                    m_layout,
                    SLOT(slotDispSettingApplyChanges(DISPLAY_TYPE_e, DISPLAY_CONFIG_t, STYLE_TYPE_e)));
            connect(m_displayMode,
                    SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                    m_toolbar,
                    SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
            connect(m_displayMode,
                    SIGNAL(sigToolbarStyleChnageNotify(STYLE_TYPE_e)),
                    m_toolbar,
                    SLOT(slotStyleChnagedNotify(STYLE_TYPE_e)));
        }
        break;

        case ASYN_PLAYBACK_BUTTON:
        {
            if (IS_VALID_OBJ(m_playbackSearch))
            {
                break;
            }

            /* PARASOFT: Memory Deallocated in slot CloseToolbarPage */
            m_playbackSearch = new PlaybackSearch(this);
            m_toolbarPage = m_playbackSearch;
            connect(m_playbackSearch,
                    SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                    m_toolbar,
                    SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
            connect(m_playbackSearch,
                    SIGNAL(sigPlaybackPlayBtnClick(PlaybackRecordData, QString, bool)),
                    m_layout,
                    SLOT(slotPlaybackPlayClick(PlaybackRecordData, QString, bool)));
            connect(m_playbackSearch,
                    SIGNAL(sigPreviousRecords(PlaybackRecordData*, quint8, TEMP_PLAYBACK_REC_DATA_t*)),
                    this,
                    SLOT(slotPrevRecords(PlaybackRecordData*, quint8, TEMP_PLAYBACK_REC_DATA_t*)));

            if (m_totalRecord)
            {
                quint8 devIdx, devCount;
                QStringList devList;

                m_applController->GetEnableDevList(devCount, devList);
                for (devIdx = 0; devIdx < devCount; devIdx++)
                {
                    if (m_prevRecordData[0].deviceName == devList.value(devIdx))
                    {
                        m_playbackSearch->showPrevTableData(m_prevRecordData, m_totalRecord, &m_pbSearchPrevData);
                        break;
                    }
                }

                if (devIdx >= devCount)
                {
                    m_totalRecord = 0;
                }
            }

            if (m_quickBackup != NULL)
            {
                m_playbackSearch->setQuickBackupFlag(m_quickBackup->getQuickBackupFlag());
            }
        }
        break;

        case SYNC_PLAYBACK_BUTTON:
        {
            if (IS_VALID_OBJ(m_syncPlayback))
            {
                break;
            }

            m_layout->processingBeforeSyncPlayback();
            /* PARASOFT: Memory Deallocated in slot CloseToolbarPage */
            m_syncPlayback = new SyncPlayback(m_layout->width(), m_layout->height(),
                                              (Layout::currentModeType[MAIN_DISPLAY] == STATE_SYNC_PLAYBACK_MODE), this);
            m_toolbarPage = m_syncPlayback;
            connect(m_syncPlayback,
                    SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                    m_toolbar,
                    SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
            connect(m_syncPlayback,
                    SIGNAL(sigChangeLayout(LAYOUT_TYPE_e, DISPLAY_TYPE_e, quint16, bool, bool)),
                    m_layout,
                    SLOT(slotChangeLayout(LAYOUT_TYPE_e, DISPLAY_TYPE_e, quint16, bool, bool)));
            connect(m_layout,
                    SIGNAL(sigPreProcessingDoneForSyncPlayback()),
                    m_syncPlayback,
                    SLOT(slotPreProcessingDoneForSyncPlayback()));
            connect(m_layout,
                    SIGNAL(sigChangePageFromLayout()),
                    m_syncPlayback,
                    SLOT(slotLayoutChanged()));

            if (m_quickBackup != NULL)
            {
                m_syncPlayback->setQuickBackupFlag(m_quickBackup->getQuickBackupFlag());
            }

            m_syncPlayback->setClientAudioRunningFlag(m_layout->m_isClientAudProcessRunning);
        }
        break;

        case EVENT_LOG_BUTTON:
        {
            if (IS_VALID_OBJ(m_eventLogSearch))
            {
                break;
            }

            /* PARASOFT: Memory Deallocated in slot CloseToolbarPage */
            m_eventLogSearch = new EventLogSearch(this);
            m_toolbarPage = m_eventLogSearch;
            connect(m_eventLogSearch,
                    SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                    m_toolbar,
                    SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
            connect(m_eventLogSearch,
                    SIGNAL(sigPlaybackPlayBtnClick(PlaybackRecordData, QString, bool)),
                    m_layout,
                    SLOT(slotPlaybackPlayClick(PlaybackRecordData, QString, bool)));
        }
        break;

        case SETTINGS_BUTTON:
        {
            USRS_GROUP_e currentUserType = MAX_USER_GROUP;
            m_applController->GetUserGroupType (LOCAL_DEVICE_NAME, currentUserType);
            if ((m_autoAddCamAction == MX_AUTO_ADD_CAM_SEARCH_PAGE_LOAD) && (currentUserType != ADMIN))
            {
                m_autoAddCamInfoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(CMD_NO_PRIVILEGE));
                m_toolbar->closeToolbarPage(SETTINGS_BUTTON);
                m_autoAddCamAction =  MAX_MX_AUTO_ADD_CAM_SEARCH_ACTION;
                break;
            }

            if (m_settings == NULL)
            {
                /* PARASOFT: Memory Deallocated in slot CloseToolbarPage */
                m_settings = new Settings(this);
                m_toolbarPage = m_settings;
                connect(m_settings,
                        SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                        m_toolbar,
                        SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
                connect(m_settings,
                        SIGNAL(sigOpenCameraFeature(void*, CAMERA_FEATURE_TYPE_e, quint8, void*, QString)),
                        this,
                        SLOT(slotOpenCameraFeature(void*, CAMERA_FEATURE_TYPE_e, quint8, void*, QString)));
                connect(m_settings,
                        SIGNAL(sigChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e, STATE_TYPE_e)),
                        m_toolbar,
                        SLOT(slotChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e, STATE_TYPE_e)));
            }

            if (m_autoAddCamAction == MX_AUTO_ADD_CAM_SEARCH_PAGE_LOAD)
            {
                if (isOnbootAuoCamSearchRunning)
                {
                    if (m_settings->getCurrentSettingMenu() != CAMERA_SETTING)
                    {
                        m_settings->loadPage(CAMERA_SETTING,CAMERA_SEARCH);
                    }
                    else if ((m_settings->getCurrentSettingMenu() == CAMERA_SETTING) && (m_settings->getCurrentSettingSubMenu() != CAMERA_SEARCH))
                    {
                        m_settings->slotSubSettingsOptions (CAMERA_SEARCH);
                    }

                    QString payload = m_autoAddCam->getPayloadList();
                    m_settings->loadCamsearchOnAutoConfig();
                    if (payload != "")
                    {
                        DevCommParam *param = new DevCommParam;
                        param->payload = payload;
                        param->deviceStatus = CMD_SUCCESS;
                        m_settings->updateCamSearchList(param);
                    }

                    m_settings->raise();
                }
                else
                {
                    if (IS_VALID_OBJ(m_toolbar))
                    {
                        m_toolbar->closeToolbarPage(SETTINGS_BUTTON);
                        slotAutoAddClose();
                    }
                }

                m_autoAddCamAction =  MAX_MX_AUTO_ADD_CAM_SEARCH_ACTION;
            }
            else if (m_quickBackupActions == QUICK_BACKUP_ADVANCE_BUTTON)
            {
                if (m_settings->getCurrentSettingMenu() != STORAGE_AND_BACKUP_SETTING)
                {
                    m_settings->loadPage(STORAGE_AND_BACKUP_SETTING,STORAGE_BACKUP);
                }
                else if ((m_settings->getCurrentSettingMenu() == STORAGE_AND_BACKUP_SETTING) && (m_settings->getCurrentSettingSubMenu() != STORAGE_BACKUP))
                {
                    m_settings->slotSubSettingsOptions (STORAGE_BACKUP);
                }

                m_quickBackupActions = MAX_QUICK_BACKUP_ELEMENTS;
            }
        }
        break;

        case MANAGE_BUTTON:
        {
            if (IS_VALID_OBJ(m_managePages))
            {
                break;
            }

            /* PARASOFT: Memory Deallocated in slot CloseToolbarPage */
            m_managePages = new ManagePages(m_toolbar->getCurrentToolBarButtonState (LOG_BUTTON), this);
            m_toolbarPage = m_managePages;
            connect(m_managePages,
                    SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                    m_toolbar,
                    SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
        }
        break;

        case SYSTEM_STATUS_BUTTON:
        {
            if (IS_VALID_OBJ(m_systemDetail))
            {
                break;
            }

            /* PARASOFT: Memory Deallocated in slot CloseToolbarPage */
            m_systemDetail = new SystemDetail(this);
            m_toolbarPage = m_systemDetail;
            connect(m_systemDetail,
                    SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                    m_toolbar,
                    SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
        }
        break;

        case AUDIO_CONTROL_BUTTON:
        {
            if (IS_VALID_OBJ(m_volumeControl))
            {
                break;
            }

            /* PARASOFT: Memory Deallocated in slot CloseToolbarPage */
            m_volumeControl = new VolumeControl((ApplController::getXPosOfScreen() + (TOOLBAR_BUTTON_WIDTH   * AUDIO_CONTROL_BUTTON) - SCALE_WIDTH(9)),
                                                (ApplController::getYPosOfScreen() + ApplController::getHeightOfScreen() - TOOLBAR_BUTTON_HEIGHT - SCALE_HEIGHT(200)),
                                                TOOLBAR_BUTTON_WIDTH + SCALE_WIDTH(20), SCALE_HEIGHT(200), this);
            m_toolbarPage = m_volumeControl;
            connect(m_volumeControl,
                    SIGNAL(sigChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e, STATE_TYPE_e)),
                    m_toolbar,
                    SLOT(slotChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e, STATE_TYPE_e)));
            connect(m_volumeControl,
                    SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                    m_toolbar,
                    SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
        }
        break;

        case SEQUENCE_BUTTON:
        {
            if (m_toolbar->getCurrentToolBarButtonState(SEQUENCE_BUTTON) == STATE_1)
            {
                m_layout->startSequencing(MAIN_DISPLAY);
            }
            else
            {
                m_layout->stopSequencing(MAIN_DISPLAY);
            }

            m_toolbar->closeToolbarPage(SEQUENCE_BUTTON);
        }
        break;

        case QUICK_BACKUP:
        {
            if (IS_VALID_OBJ(m_quickBackup))
            {
                m_quickBackup->show();
                m_quickBackup->setMinimizeFlag(false);
                m_quickBackup->setFocus();
                m_toolbarPage = m_quickBackup;
                break;
            }

            /* PARASOFT: Memory Deallocated in slot CloseToolbarPage */
            m_quickBackup = new MxQuickBackup(this);
            m_toolbarPage = m_quickBackup;
            connect(m_quickBackup,
                    SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                    m_toolbar,
                    SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
            connect(m_quickBackup,
                    SIGNAL(sigChangeToolbarBtnState(TOOLBAR_BUTTON_TYPE_e, bool)),
                    m_toolbar,
                    SLOT(slotChangeToolbarBtnState(TOOLBAR_BUTTON_TYPE_e, bool)));
            connect(this,
                    SIGNAL(sigChangeToolbarBtnState(TOOLBAR_BUTTON_TYPE_e, bool)),
                    m_toolbar,
                    SLOT(slotChangeToolbarBtnState(TOOLBAR_BUTTON_TYPE_e, bool)));
            connect(m_quickBackup,
                    SIGNAL(sigUpdateQuickbackupFlag(bool)),
                    this,
                    SLOT(slotUpdateQuickbackupFlag(bool)));
            connect(m_quickBackup,
                    SIGNAL(sigAdavnceOptions(QUICK_BACKUP_ELEMENTS_e)),
                    this,
                    SLOT(slotAdvanceOption(QUICK_BACKUP_ELEMENTS_e)));
        }
        break;

        case VIDEO_POPUP_BUTTON:
        {
            if (m_toolbar->getCurrentToolBarButtonState(VIDEO_POPUP_BUTTON) == STATE_1)
            {
                m_showVideoPopupF = true;
                m_toolbar->changeToolbarButtonState(VIDEO_POPUP_BUTTON, STATE_2);
            }
            else
            {
                m_showVideoPopupF = false;
                m_toolbar->changeToolbarButtonState(VIDEO_POPUP_BUTTON, STATE_1);
            }
            m_toolbar->closeToolbarPage(VIDEO_POPUP_BUTTON);
        }
        break;

        case WIZARD_BUTTON:
        {
            if (IS_VALID_OBJ(m_setupWizard))
            {
                break;
            }

            /* PARASOFT: Memory Deallocated in slot CloseToolbarPage */
            m_setupWizard = new SetupWizard(LOCAL_DEVICE_NAME, this);
            m_toolbarPage = m_setupWizard;
            connect(this,
                    SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                    m_toolbar,
                    SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
            connect(m_setupWizard,
                    SIGNAL(sigQuitSetupWiz()),
                    this,
                    SLOT(slotQuitSetupWiz()));
        }
        break;

        case LOG_BUTTON:
        {
            if (m_toolbar == NULL)
            {
                break;
            }

            if (IS_VALID_OBJ(m_layout))
            {
                m_layout->updateNextToolbarPageIndex(m_toolbar->getNextToolBarButton());
            }

            if (IS_VALID_OBJ(m_logIn))
            {
                break;
            }

            /* PARASOFT: Memory Deallocated in slot CloseToolbarPage */
            m_logIn = new LogIn(m_toolbar->getCurrentToolBarButtonState (LOG_BUTTON), this);
            m_toolbarPage = m_logIn;
            connect(m_logIn,
                    SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                    m_toolbar,
                    SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
            connect(m_logIn,
                    SIGNAL(sigChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e, STATE_TYPE_e)),
                    m_toolbar,
                    SLOT(slotChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e, STATE_TYPE_e)));

            if (m_toolbar->getCurrentToolBarButtonState(LOG_BUTTON) == STATE_1)
            {
                m_logIn->getOtherUserParam();
                break;
            }

            if (m_quickBackup == NULL)
            {
                m_logIn->getlocalUserParam();
            }
            else
            {
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(QUICK_BKUP_ON_LOGOUT_MSG));
                m_toolbar->closeToolbarPage(LOG_BUTTON);
            }
        }
        break;

        case BUZZER_CONTROL_BUTTON:
        {
            if (IS_VALID_OBJ(m_buzzerControl))
            {
                m_buzzerControl->sendStopBuzzerCommand();
                break;
            }

            /* PARASOFT: Memory Deallocated in slot CloseToolbarPage */
            m_buzzerControl = new BuzzerControl(this);
            connect(m_buzzerControl,
                    SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                    m_toolbar,
                    SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
        }
        break;

        case LIVE_EVENT_BUTTON:
        {
            if (IS_VALID_OBJ(m_liveEvent))
            {
                break;
            }

            /* PARASOFT: Memory Deallocated in slot CloseToolbarPage */
            m_liveEvent = new LiveEvent(this);
            m_toolbarPage = m_liveEvent;
            connect(m_liveEvent,
                    SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                    m_toolbar,
                    SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
            connect(m_liveEvent,
                    SIGNAL(sigNotifyStatusIcon(TOOLBAR_BUTTON_TYPE_e, bool)),
                    m_toolbar,
                    SLOT(slotNotifyStatusIcon(TOOLBAR_BUTTON_TYPE_e, bool)));
            m_liveEvent->forceActiveFocus();
        }
        break;

        case USB_CONTROL_BUTTON:
        {
            if (IS_VALID_OBJ(m_usbControl))
            {
                break;
            }

            /* PARASOFT: Memory Deallocated in slot CloseToolbarPage */
            m_usbControl = new UsbControl(this);
            m_toolbarPage = m_usbControl;
            connect(m_usbControl,
                    SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                    m_toolbar,
                    SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
        }
        break;

        case COLLAPSE_BUTTON:
        {
            m_layout->collapseWindow();
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }

    slotRaiseWidget();
}

void MainWindow::slotCloseToolbarPage(TOOLBAR_BUTTON_TYPE_e index)
{
    switch (index)
    {
        case ABOUT_US_BUTTON:
        {
            if (m_aboutUs == NULL)
            {
                break;
            }

            disconnect (m_aboutUs,
                        SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                        m_toolbar,
                        SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
            DELETE_OBJ(m_aboutUs);
        }
        break;

        case LIVE_VIEW_BUTTON:
        {
            if (m_displaySetting == NULL)
            {
                break;
            }

            disconnect(m_layout,
                       SIGNAL(sigDisplaySettingApplyChangesNotify(DISPLAY_TYPE_e, bool)),
                       m_displaySetting,
                       SLOT(slotLayoutResponseOnApply(DISPLAY_TYPE_e, bool)));
            disconnect(m_displaySetting,
                       SIGNAL(sigProcessApplyStyleToLayout(DISPLAY_TYPE_e, DISPLAY_CONFIG_t, STYLE_TYPE_e)),
                       m_layout,
                       SLOT(slotDispSettingApplyChanges(DISPLAY_TYPE_e, DISPLAY_CONFIG_t, STYLE_TYPE_e)));
            disconnect(m_layout,
                       SIGNAL(sigWindowResponseToDisplaySettingsPage(DISPLAY_TYPE_e, QString, quint8, quint16)),
                       m_displaySetting,
                       SLOT(slotWindowResponseToDisplaySettingsPage(DISPLAY_TYPE_e, QString, quint8, quint16)));
            disconnect(m_displaySetting,
                       SIGNAL(sigChangeLayout(LAYOUT_TYPE_e, DISPLAY_TYPE_e, quint16, bool, bool)),
                       m_layout,
                       SLOT(slotChangeLayout(LAYOUT_TYPE_e, DISPLAY_TYPE_e, quint16, bool, bool)));
            disconnect(m_displaySetting,
                       SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                       m_toolbar,
                       SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
            disconnect(m_displaySetting,
                       SIGNAL(sigToolbarStyleChnageNotify(STYLE_TYPE_e)),
                       m_toolbar,
                       SLOT(slotStyleChnagedNotify(STYLE_TYPE_e)));
            disconnect(m_displaySetting,
                       SIGNAL(sigliveViewAudiostop()),
                       m_layout,
                       SLOT(slotliveViewAudiostop()));
            DELETE_OBJ(m_displaySetting);
        }
        break;

        case DISPLAY_MODE_BUTTON:
        {
            if (m_displayMode == NULL)
            {
                break;
            }

            disconnect(m_displayMode,
                       SIGNAL(sigToolbarStyleChnageNotify(STYLE_TYPE_e)),
                       m_toolbar,
                       SLOT(slotStyleChnagedNotify(STYLE_TYPE_e)));
            disconnect(m_displayMode,
                       SIGNAL(sigApplyNewLayout(DISPLAY_TYPE_e, DISPLAY_CONFIG_t, STYLE_TYPE_e)),
                       m_layout,
                       SLOT(slotDispSettingApplyChanges(DISPLAY_TYPE_e, DISPLAY_CONFIG_t, STYLE_TYPE_e)));
            disconnect(m_displayMode,
                       SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                       m_toolbar,
                       SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
            DELETE_OBJ(m_displayMode);
            m_toolbar->unloadToolbarPage();
        }
        break;

        case ASYN_PLAYBACK_BUTTON:
        {
            if (m_playbackSearch == NULL)
            {
                break;
            }

            m_pbSearchPrevData.currentPage = m_playbackSearch->getCurrPage();
            disconnect(m_playbackSearch,
                       SIGNAL(sigPlaybackPlayBtnClick(PlaybackRecordData, QString, bool)),
                       m_layout,
                       SLOT(slotPlaybackPlayClick(PlaybackRecordData, QString, bool)));
            disconnect(m_playbackSearch,
                       SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                       m_toolbar,
                       SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
            disconnect(m_playbackSearch,
                       SIGNAL(sigPreviousRecords(PlaybackRecordData*, quint8, TEMP_PLAYBACK_REC_DATA_t*)),
                       this,
                       SLOT(slotPrevRecords(PlaybackRecordData*, quint8, TEMP_PLAYBACK_REC_DATA_t*)));
            DELETE_OBJ(m_playbackSearch);
        }
        break;

        case SYNC_PLAYBACK_BUTTON:
        {
            if (m_syncPlayback == NULL)
            {
                break;
            }

            disconnect(m_syncPlayback,
                       SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                       m_toolbar,
                       SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
            disconnect(m_syncPlayback,
                       SIGNAL(sigChangeLayout(LAYOUT_TYPE_e, DISPLAY_TYPE_e, quint16, bool, bool)),
                       m_layout,
                       SLOT(slotChangeLayout(LAYOUT_TYPE_e, DISPLAY_TYPE_e, quint16, bool, bool)));
            disconnect(m_layout,
                       SIGNAL(sigPreProcessingDoneForSyncPlayback()),
                       m_syncPlayback,
                       SLOT(slotPreProcessingDoneForSyncPlayback()));
            disconnect(m_layout,
                       SIGNAL(sigChangePageFromLayout()),
                       m_syncPlayback,
                       SLOT(slotLayoutChanged()));
            DELETE_OBJ(m_syncPlayback);
            m_layout->processingAfterSyncPlayback();
        }
        break;

        case EVENT_LOG_BUTTON:
        {
            if (m_eventLogSearch == NULL)
            {
                break;
            }

            disconnect(m_eventLogSearch,
                       SIGNAL(sigPlaybackPlayBtnClick(PlaybackRecordData, QString, bool)),
                       m_layout,
                       SLOT(slotPlaybackPlayClick(PlaybackRecordData, QString, bool)));
            disconnect(m_eventLogSearch,
                       SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                       m_toolbar,
                       SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
            DELETE_OBJ(m_eventLogSearch);
        }
        break;

        case SETTINGS_BUTTON:
        {
            if (m_settings == NULL)
            {
                break;
            }

            disconnect(m_settings,
                       SIGNAL(sigChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e, STATE_TYPE_e)),
                       m_toolbar,
                       SLOT(slotChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e, STATE_TYPE_e)));
            disconnect(m_settings,
                       SIGNAL(sigOpenCameraFeature(void*, CAMERA_FEATURE_TYPE_e, quint8, void*, QString)),
                       this,
                       SLOT(slotOpenCameraFeature(void*, CAMERA_FEATURE_TYPE_e, quint8, void*, QString)));
            disconnect(m_settings,
                       SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                       m_toolbar,
                       SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
            DELETE_OBJ(m_settings);
        }
        break;

        case MANAGE_BUTTON:
        {
            if (m_managePages == NULL)
            {
                break;
            }

            disconnect(m_managePages,
                       SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                       m_toolbar,
                       SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
            DELETE_OBJ(m_managePages);
        }
        break;

        case SYSTEM_STATUS_BUTTON:
        {
            if (m_systemDetail == NULL)
            {
                break;
            }

            disconnect(m_systemDetail,
                       SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                       m_toolbar,
                       SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
            DELETE_OBJ(m_systemDetail);
        }
        break;

        case AUDIO_CONTROL_BUTTON:
        {
            if (m_volumeControl == NULL)
            {
                break;
            }

            disconnect(m_volumeControl,
                       SIGNAL(sigChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e, STATE_TYPE_e)),
                       m_toolbar,
                       SLOT(slotChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e, STATE_TYPE_e)));
            disconnect(m_volumeControl,
                       SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                       m_toolbar,
                       SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
            DELETE_OBJ(m_volumeControl);
            m_toolbar->unloadToolbarPage();
        }
        break;

        case QUICK_BACKUP:
        {
            if (m_quickBackup == NULL)
            {
                break;
            }

            if (true == m_quickBackup->getMinimizeFlag())
            {
                break;
            }

            m_quickBackup->sendStopBackupCmd();
            emit sigChangeToolbarBtnState(QUICK_BACKUP, false);
            disconnect(m_quickBackup,
                       SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                       m_toolbar,
                       SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
            disconnect(m_quickBackup,
                       SIGNAL(sigChangeToolbarBtnState(TOOLBAR_BUTTON_TYPE_e, bool)),
                       m_toolbar,
                       SLOT(slotChangeToolbarBtnState(TOOLBAR_BUTTON_TYPE_e, bool)));
            disconnect(this,
                       SIGNAL(sigChangeToolbarBtnState(TOOLBAR_BUTTON_TYPE_e, bool)),
                       m_toolbar,
                       SLOT(slotChangeToolbarBtnState(TOOLBAR_BUTTON_TYPE_e, bool)));
            disconnect(m_quickBackup,
                       SIGNAL(sigUpdateQuickbackupFlag(bool)),
                       this,
                       SLOT(slotUpdateQuickbackupFlag(bool)));
            disconnect(m_quickBackup,
                       SIGNAL(sigAdavnceOptions(QUICK_BACKUP_ELEMENTS_e)),
                       this,
                       SLOT(slotAdvanceOption(QUICK_BACKUP_ELEMENTS_e)));
            DELETE_OBJ(m_quickBackup);
        }
        break;

        case WIZARD_BUTTON:
        {
            if (m_setupWizard == NULL)
            {
                break;
            }

            disconnect(m_setupWizard ,
                       SIGNAL(sigQuitSetupWiz()),
                       this,
                       SLOT(slotQuitSetupWiz()));
            DELETE_OBJ(m_setupWizard);
        }
        break;

        case LOG_BUTTON:
        {
            if (m_logIn == NULL)
            {
                break;
            }

            USRS_GROUP_e currentUserType = VIEWER;
            m_applController->GetUserGroupType(LOCAL_DEVICE_NAME, currentUserType);

            if (m_isAutoTmZnUpdtRunning)
            {
                if (m_toolbar->getCurrentToolBarButtonState(LOG_BUTTON) == STATE_1)
                {
                    m_isAutoTmZnUpdtRunning = false;
                }
                else
                {
                    if (currentUserType == ADMIN)
                    {
                        getAutoUpdtRegFlg();
                    }
                    else
                    {
                        m_isAutoTmZnUpdtRunning = false;
                        MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(CMD_NO_PRIVILEGE));
                    }
                }
            }
            else
            {
                if (m_isStartLiveView)
                {
                    if (m_toolbar->getCurrentToolBarButtonState(LOG_BUTTON) == STATE_1)
                    {
                        m_isStartLiveView = false;
                    }
                    else
                    {
                        if (currentUserType == ADMIN)
                        {
                            m_payloadLib->setCnfgArrayAtIndex(0, 1);
                            QString payloadString = m_payloadLib->createDevCnfgPayload(MSG_SET_CFG,
                                                                                       GENERAL_TABLE_INDEX,
                                                                                       CNFG_FRM_INDEX,
                                                                                       1,
                                                                                       FIELD_START_LIVE_VIEW_FLAG + 1,
                                                                                       FIELD_START_LIVE_VIEW_FLAG + 1,
                                                                                       1);
                            DevCommParam* param = new DevCommParam();
                            param->msgType = MSG_SET_CFG;
                            param->payload = payloadString;
                            m_applController->processActivity(LOCAL_DEVICE_NAME, DEVICE_COMM, param);
                        }
                        else
                        {
                            m_isStartLiveView = false;
                            MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(CMD_NO_PRIVILEGE));
                        }
                    }
                }
            }

            if ((m_toolbar->getCurrentToolBarButtonState(LOG_BUTTON) == STATE_2) && (m_autoAddCam != NULL))
            {
                if (m_isUnloadAutoAddCam == true)
                {
                    if (currentUserType == ADMIN)
                    {
                        m_isUnloadAutoAddCam = false;
                        m_autoAddCam->saveConfiguration();
                        m_autoAddCam->loadAutoAddCamAfterLogin();
                    }
                    else
                    {
                        m_isUnloadAutoAddCam = false;
                        m_autoAddCamInfoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(CMD_NO_PRIVILEGE));
                    }
                }
            }

            if ((IS_VALID_OBJ(m_autoAddCam)) && (IS_VALID_OBJ(m_toolbar)) && (m_toolbar->getCurrentToolBarButtonState(LOG_BUTTON) == STATE_1))
            {
                m_isUnloadAutoAddCam = false;
            }

            // For Auto Add Cam
            if ((IS_VALID_OBJ(m_autoAddCam)) && (IS_VALID_OBJ(m_toolbar)) && (m_autoAddCamAction == MX_AUTO_ADD_CAM_SEARCH_PAGE_LOAD)
                    &&  (m_toolbar->getCurrentToolBarButtonState(LOG_BUTTON) == STATE_1))
            {
                m_autoAddCam->setIsCamSearchPageLoaded(false);
                m_autoAddCamAction = MAX_MX_AUTO_ADD_CAM_SEARCH_ACTION;

                if (!isOnbootAuoCamSearchRunning)
                {
                    slotAutoAddClose();
                }
            }

            if (m_logIn->getCurrentLoginState() != LOGIN_NORMAL)
            {
                m_logIn->hide();
                m_logIn->deleteElements();
                m_logIn->getlocalUserParam ();
                m_logIn->sendChangeUserCmd();
            }
            else
            {
                disconnect(m_logIn,
                           SIGNAL(sigChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e, STATE_TYPE_e)),
                           m_toolbar,
                           SLOT(slotChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e, STATE_TYPE_e)));
                disconnect(m_logIn,
                           SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                           m_toolbar,
                           SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
                DELETE_OBJ(m_logIn);
            }
        }
        break;

        case BUZZER_CONTROL_BUTTON:
        {
            if (m_buzzerControl == NULL)
            {
                break;
            }

            disconnect(m_buzzerControl,
                       SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                       m_toolbar,
                       SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
            DELETE_OBJ(m_buzzerControl);
        }
        break;

        case LIVE_EVENT_BUTTON:
        {
            if (m_liveEvent == NULL)
            {
                break;
            }

            disconnect(m_liveEvent,
                       SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                       m_toolbar,
                       SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
            disconnect(m_liveEvent,
                       SIGNAL(sigNotifyStatusIcon(TOOLBAR_BUTTON_TYPE_e,bool)),
                       m_toolbar,
                       SLOT(slotNotifyStatusIcon(TOOLBAR_BUTTON_TYPE_e,bool)));
            DELETE_OBJ(m_liveEvent);
        }
        break;

        case USB_CONTROL_BUTTON:
        {
            if (m_usbControl == NULL)
            {
                break;
            }

            disconnect(m_usbControl,
                       SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                       m_toolbar,
                       SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
            DELETE_OBJ(m_usbControl);
        }
        break;

        case VIDEO_POPUP_BUTTON:
        case SEQUENCE_BUTTON:
        default:
        {
            /* Nothing to do */
        }
        break;
    }

    m_toolbarPage = NULL;
    slotRaiseWidget();
}

void MainWindow::sendCommand(QString deviceName, SET_COMMAND_e cmdType, int totalfeilds)
{
    QString payloadString = m_payloadLib->createDevCmdPayload(totalfeilds);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = cmdType;
    param->payload = payloadString;
    DPRINT(STREAM_REQ, "send cmd in main window: [device=%s], [cmd=%s]", deviceName.toUtf8().constData(), cmdString[cmdType].toUtf8().constData());
    m_applController->processActivity(deviceName, DEVICE_COMM, param);
}

void MainWindow::slotDevCommGui(QString deviceName, DevCommParam* param)
{
    DEVICE_STATE_TYPE_e devState = MAX_DEVICE_STATE;
    DEV_TABLE_INFO_t devTable;
    QString username, password;

    switch (param->msgType)
    {
        case MSG_REQ_LOG:
        {
            switch (param->deviceStatus)
            {
                /* First time on online device gets connected */
                case CMD_DEV_CONNECTED:
                case CMD_DISK_CLEANUP_REQUIRED:
                {
                    /* If it is local device or toolbar is open */
                    if ((m_toolbar != NULL) || (deviceName == LOCAL_DEVICE_NAME))
                    {
                        m_layout->setDefaultDisplay(deviceName);
                    }

                    /* Get device response */
                    m_applController->GetDeviceInfo(deviceName, devTable);

                    /* If local device login and no toolbar open */
                    if (deviceName == LOCAL_DEVICE_NAME)
                    {
                        if (m_toolbar == NULL)
                        {
                            this->activateWindow();
                            createToolbar();
                            m_layout->setDefaultResolution();
                            sleep(3);   /* Added delay to display "Loading..." text for some time */
                            m_layout->setDefaultLayout();
                            deleteInitDeInitBox();
                            updateUsbStatus();
                            updateAudioStatus();

                            if (param->deviceStatus == CMD_DISK_CLEANUP_REQUIRED)
                            {
                                m_infoPage->loadInfoPage(MAIN_WINDOW_DISKCLEANUP_REQUIRED_MESSAGE(m_hddTimerCnt), true, true, "", "Yes", "No");
                                m_hddCleanInfoTimer->start ();
                            }
                        }

                        if (deviceRespInfo.configuredAnalogCameras != 0)
                        {
                            if (m_osdUpdate == NULL)
                            {
                                m_applController->GetDeviceInfo (deviceName, devTable);
                                m_osdUpdate = new OsdUpdate(devTable.analogCams);
                                m_osdUpdate->start();
                            }
                            else
                            {
                                m_osdUpdate->updateServerInfo();
                                m_osdUpdate->updateTimeCountForOsdUpdate(0);
                            }
                        }

                        getAutoCnfgFlag();
                        slotUpdateUIGeometry(true);

                        m_applController->setUserRightsResponseNotify(false);
                    }

                    sendCommand(deviceName, GET_USR_RIGHTS);
                    m_applController->writeMaxWindowsForDisplay(m_applController->GetTotalCameraOfEnableDevices());
                    m_applController->GetDeviceInfo(LOCAL_DEVICE_NAME, devTable);
                    m_preStartLiveViewF = devTable.startLiveView;

                    if (devTable.startLiveView == false)
                    {
                        m_layout->stopLocalDecoding(devTable.startLiveView);
                    }

                    devState = CONNECTED;
                    DPRINT(DEVICE_CLIENT, "login response: [device=%s], [status=CONNECTED]", deviceName.toUtf8().constData());
                }
                break;

                case CMD_DEV_CONFLICT:
                {
                    devState = CONFLICT;
                    DPRINT(DEVICE_CLIENT, "login response: [device=%s], [status=CONFLICT]", deviceName.toUtf8().constData());
                }
                break;

                case CMD_DEV_LOGGEDOUT:
                case CMD_DEV_DISCONNECTED:
                case CMD_DEV_DELETED:
                {
                    switch (param->deviceStatus)
                    {
                        case CMD_DEV_LOGGEDOUT:
                            devState = LOGGED_OUT;
                            DPRINT(DEVICE_CLIENT, "login response: [device=%s], [status=LOGOUT]", deviceName.toUtf8().constData());
                            break;

                        case CMD_DEV_DISCONNECTED:
                            devState = DISCONNECTED;
                            DPRINT(DEVICE_CLIENT, "login response: [device=%s], [status=DISCONNECTED]", deviceName.toUtf8().constData());
                            break;

                        case CMD_DEV_DELETED:
                            devState = DELETED;
                            DPRINT(DEVICE_CLIENT, "login response: [device=%s], [status=DELETED]", deviceName.toUtf8().constData());
                            break;

                        default:
                            break;
                    }

                    if (IS_VALID_OBJ(m_layout))
                    {
                        m_layout->disconnectDeviceNotify(deviceName);
                    }

                    if (m_settings != NULL)
                    {
                        m_settings->disconnectDeviceNotify(deviceName, false);
                    }
                    else if (m_managePages != NULL)
                    {
                        m_managePages->disconnectDeviceNotify(deviceName, false);
                    }
                    else if (m_syncPlayback != NULL)
                    {
                        m_syncPlayback->deviceDisconnectNotify(deviceName);
                    }

                    if ((deviceName == LOCAL_DEVICE_NAME) && ((devState == LOGGED_OUT) || (devState == DISCONNECTED)))
                    {
                        emit sigChangeToolbarButtonState(LOG_BUTTON, STATE_1);

                        if (IS_VALID_OBJ(m_toolbar))
                        {
                            if (m_toolbar->getCurrentToolBarButton () == SETTINGS_BUTTON)
                            {
                                m_toolbar->closeToolbarPage (SETTINGS_BUTTON);
                            }
                        }
                    }
                }
                break;

                case CMD_SUCCESS:
                {
                    /* Nothing */
                }
                break;

                default:
                {
                    devState = DISCONNECTED;
                }
                break;
            }

            if ((param->deviceStatus == CMD_SUCCESS) || (param->deviceStatus == CMD_DISK_CLEANUP_REQUIRED))
            {
                break;
            }

            if (IS_VALID_OBJ(m_layout))
            {
                m_layout->actionWindowforDeviceStatechange(deviceName, devState);
                m_layout->updateDeviceState(deviceName, devState);
            }

            if (m_displaySetting != NULL)
            {
                m_displaySetting->updateDeviceState(deviceName, devState);
            }

            if (Layout::currentModeType[MAIN_DISPLAY] == STATE_LOCAL_DECODING)
            {
               if (IS_VALID_OBJ(m_layout))
               {
                    m_layout->updateWindowDataForLocalDecording();
               }
            }

            QString msgStr = ValidationMessage::getDeviceResponceMessage(param->deviceStatus);
            if (deviceName == LOCAL_DEVICE_NAME)
            {
                if (msgStr != "")
                {
                    MessageBanner::addMessageInBanner(m_applController->GetDispDeviceName(deviceName) + QString(" : ") + msgStr);
                }
                break;
            }

            if (param->deviceStatus == CMD_RESET_PASSWORD)
            {
                if (m_applController->GetDevicePreferNativeDeviceState(deviceName))
                {
                    m_applController->getUsernameFrmDev(LOCAL_DEVICE_NAME, username);
                    if (username != DEFAULT_LOGIN_USER)
                    {
                        m_applController->getPasswordFrmDev(LOCAL_DEVICE_NAME, password);
                        m_payloadLib->setCnfgArrayAtIndex(0, password);
                        sendCommand(deviceName, CNG_PWD, 1);
                    }
                    else
                    {
                        //if local user login then set password to the login credetial
                        m_applController->getUsernameFrmDev(deviceName, username);
                        m_applController->getPasswordFrmDev(deviceName, password);
                        m_payloadLib->setCnfgArrayAtIndex(0, password);
                        sendCommand(deviceName, CNG_PWD, 1);
                    }
                }
                else
                {
                    //if local user login then set password to the login credetial
                    m_applController->getUsernameFrmDev(deviceName, username);
                    m_applController->getPasswordFrmDev(deviceName, password);
                    m_payloadLib->setCnfgArrayAtIndex(0, password);
                    sendCommand(deviceName, CNG_PWD, 1);
                }

                /* Don't display banner message */
                break;
            }

            /* Update message string if required */
            if (param->deviceStatus == CMD_USER_ACCOUNT_LOCK)
            {
                quint32 retryMin = (quint32)param->payload.mid(62, 3).toUInt();
                msgStr = (retryMin != 0) ? USER_ACC_LOCKED_DUE_TO_FAIL_ATTEMPT_MSG(retryMin) : "";
            }

            /* Nothing to do if message string is blanked */
            if (msgStr == "")
            {
                break;
            }

            /* Delete the device for these reponses */
            if ((param->deviceStatus == CMD_PASSWORD_EXPIRE) || (param->deviceStatus == CMD_USER_ACCOUNT_LOCK) || (param->deviceStatus == CMD_DEV_CONFLICT))
            {
                QList<QVariant> paramList;
                paramList.append(param->deviceStatus);
                m_applController->processActivity(deviceName, DISCONNECT_DEVICE, paramList, NULL);
            }

            /* Display message banner */
            MessageBanner::addMessageInBanner(m_applController->GetDispDeviceName(deviceName) + QString(" : ") + msgStr);
        }
        break;

        case MSG_SET_CMD:
        {
            if (CPU_LOAD != param->cmdType)
            {
                DPRINT(DEVICE_CLIENT, "set cmd resp in main window: [device=%s], [cmd=%s], [status=%d]",
                       deviceName.toUtf8().constData(), cmdString[param->cmdType].toUtf8().constData(), param->deviceStatus);
            }

            switch (param->cmdType)
            {
                case CNG_LV_STRM:
                case SRT_LV_STRM:
                case STP_LV_STRM:
                case INC_LV_AUD:
                case EXC_LV_AUD:
                case GET_PLY_STRM_ID:
                case PLY_RCD_STRM:
                case STP_RCD_STRM:
                case STEP_RCD_STRM:
                case CLR_PLY_STRM_ID:
                case WIN_AUDIO:
                case STP_MAN_PTZ_TOUR:
                case SRT_MAN_PTZ_TOUR:
                case SETPANTILT:
                case SETZOOM:
                case SETFOCUS:
                case SETIRIS:
                case CALLPRESET:
                case SET_PRESET:
                case SRT_MAN_REC:
                case STP_MAN_REC:
                case COSEC_VIDEO_POPUP:
                case STRT_INSTANT_PLY:
                case SEEK_INSTANT_PLY:
                case STOP_INSTANT_PLY:
                case RESUME_PTZ_TOUR:
                case PTZ_TOUR_STATUS:
                {
                    m_layout->processDeviceResponse(param,deviceName);
                }
                break;

                case GET_DATE_TIME:
                case SET_DATE_TIME:
                {
                    m_layout->processDeviceResponse(param, deviceName);

                    if (m_setupWizard != NULL)
                    {
                        m_setupWizard->processDeviceResponse(param, deviceName);
                    }
                    else if (m_toolbar->isVisible())
                    {
                        m_toolbar->processDeviceResponse(param, deviceName);
                    }
                    else if (m_settings != NULL)
                    {
                        m_settings->processDeviceResponse(param, deviceName);
                    }
                    else if (m_playbackSearch != NULL)
                    {
                        m_playbackSearch->processDeviceResponse(param, deviceName);
                    }
                    else if (m_syncPlayback != NULL)
                    {
                        m_syncPlayback->processDeviceResponse(param, deviceName);
                    }
                    else if (m_eventLogSearch != NULL)
                    {
                        m_eventLogSearch->processDeviceResponse(param, deviceName);
                    }
                    else if (m_quickBackup != NULL)
                    {
                        m_quickBackup->processDeviceResponseforSliderTime(param, deviceName);
                    }
                }
                break;

                case OTHR_SUP:
                {
                    if (m_setupWizard != NULL)
                    {
                        m_setupWizard->processDeviceResponse(param, deviceName);
                    }
                    else if (m_settings != NULL)
                    {
                        m_settings->processDeviceResponse(param, deviceName);
                    }
                    else
                    {
                        m_layout->processDeviceResponse(param, deviceName);
                    }
                }
                break;

                case GET_MAC:
                case MODEM_STS:
                case UPDT_HOST_NM:
                case UPDT_DDNS:
                case TST_MAIL:
                case TST_FTP_CON:
                case TST_TCP_CON:
                case TST_SMS:
                case CHK_BAL:
                case USB_DISK_STS:
                case USBFORMAT:
                case ENCDR_SUP:
                case RES_SUP:
                case FR_SUP:
                case QLT_SUP:
                case GET_MAX_SUPP_PROF:
                case GET_PROF_PARA:
                case GET_BITRATE:
                case GET_MOTION_WINDOW:
                case GET_PRIVACY_MASK_WINDOW:
                case ADV_CAM_SEARCH:
                case CHANGE_IP_CAM_ADDRS:
                case GENERATE_FAILURE_REPORT:
                case SET_MOTION_WINDOW:
                case TST_ND_CON:
                case SET_PRIVACY_MASK_WINDOW:
                case GET_REG_CFG:
                case ADD_CAM_INITIATED:
                case GET_CAM_INITIATED_LIST:
                case RJCT_CAM_INITIATED:
                case MAN_BACKUP:
                case GET_STATUS:
                case SEARCH_FIRMWARE:
                case START_UPGRADE:
                case GET_CAPABILITY:
                case GET_PUSH_DEV_LIST:
                case DEL_PUSH_DEV:
                {
                    if (m_settings != NULL)
                    {
                        m_settings->processDeviceResponse(param, deviceName);
                    }
                }
                break;

                case GET_LANGUAGE:
                case GET_USER_LANGUAGE:
                case SET_USER_LANGUAGE:
                {
                    if (m_settings != NULL)
                    {
                        m_settings->processDeviceResponse(param, deviceName);
                    }

                    if (m_managePages != NULL)
                    {
                        m_managePages->processDeviceResponse(param, deviceName);
                    }

                    if (m_setupWizard != NULL)
                    {
                        m_setupWizard->processDeviceResponse(param, deviceName);
                    }

                    if ((param->cmdType == GET_USER_LANGUAGE) && (m_userLanguagePendingF == true))
                    {
                        m_userLanguagePendingF = false;
                        processDeviceResponse(param, deviceName);

                        /* Added to apply multi-language after login when get language command's response received. If we don't do this
                         * then dynamically generated multi-language text will be displayed in english only after applying multi-language */
                        if ((m_toolbar != NULL) && (m_toolbar->getNextToolBarButton() < MAX_TOOLBAR_BUTTON))
                        {
                            m_toolbar->openNextToolbarPage();
                        }
                        else if ((true == m_setupWizardPendingF) && (false == IS_VALID_OBJ(m_setupWizard)))
                        {
                            /* PARASOFT: Memory Deallocated in slot QuitSetupWiz */
                            m_inVisibleWidget = new InvisibleWidgetCntrl(this);
                            m_setupWizard = new SetupWizard(LOCAL_DEVICE_NAME, this);
                            connect(m_setupWizard ,
                                    SIGNAL(sigQuitSetupWiz()),
                                    this,
                                    SLOT(slotQuitSetupWiz()));
                        }

                        /* Now setup wizard is not required */
                        m_setupWizardPendingF = false;
                    }
                }
                break;

                case GET_USER_DETAIL:
                case GET_CAMERA_INFO:
                case BRND_NAME:
                case MDL_NAME:
                case MAX_ADD_CAM:
                case ADD_CAMERAS:
                case TST_CAM:
                case PHYSICAL_DISK_STS:
                case HDDFORMAT:
                case LOGICAL_VOLUME_STS:
                case STOP_RAID:
                case GET_DHCP_LEASE:
                {
                    if (m_setupWizard != NULL)
                    {
                        m_setupWizard->processDeviceResponse(param, deviceName);
                    }
                    else if (m_settings != NULL)
                    {
                        m_settings->processDeviceResponse(param, deviceName);
                    }
                }
                break;

                case AUTO_CONFIGURE_CAMERA:
                case CNCL_AUTO_SEARCH:
                case AUTO_SEARCH:
                case GET_ACQ_LIST:
                {
                    bool isCameraSearchLoad = false;

                    if ((param->cmdType == GET_ACQ_LIST) || (param->cmdType == AUTO_CONFIGURE_CAMERA))
                    {
                        if (IS_VALID_OBJ(m_settings))
                        {
                            isCameraSearchLoad = m_settings->getDisableLeftPanelState();
                        }

                        if (IS_VALID_OBJ(m_autoAddCam))
                        {
                            m_autoAddCam->setIsCamSearchPageLoaded(isCameraSearchLoad);
                        }
                    }

                    if (IS_VALID_OBJ(m_setupWizard) && (param->windowId != WIN_ID_MXAUTO_ADD_CAM))
                    {
                        m_setupWizard->processDeviceResponse(param, deviceName);
                    }

                    if ((IS_VALID_OBJ(m_autoAddCam)) && (!isCameraSearchLoad) && (isOnbootAuoCamSearchRunning == true) && (m_setupWizard == NULL))
                    {
                        m_autoAddCam->processDeviceResponse(param, deviceName);
                    }
                    else if (IS_VALID_OBJ(m_settings))
                    {
                        m_settings->processDeviceResponse(param, deviceName);
                    }

                    if ((param->cmdType == CNCL_AUTO_SEARCH) && (m_isAutoAddCamTimeout == true) && (m_setupWizard == NULL))
                    {
                        m_isAutoAddCamTimeout = false;

                        DEV_TABLE_INFO_t deviceInfo;
                        memset(&deviceInfo, 0, sizeof(deviceInfo));
                        if (false == m_applController->GetDeviceInfo(LOCAL_DEVICE_NAME, deviceInfo))
                        {
                            EPRINT(DEVICE_CLIENT, "fail to get device info");
                        }

                        if (deviceInfo.startLiveView == false)
                        {
                            m_isStartLiveView = true;
                            if (m_startLiveViewInfoPage == NULL)
                            {
                                m_startLiveViewInfoPage = new InfoPage(ApplController::getXPosOfScreen(), ApplController::getYPosOfScreen(),
                                                                       ApplController::getWidthOfScreen(), ApplController::getHeightOfScreen(),
                                                                       MAX_INFO_PAGE_TYPE, this, true, false);
                                connect(m_startLiveViewInfoPage,
                                        SIGNAL(sigInfoPageCnfgBtnClick(int)),
                                        this,
                                        SLOT(slotStartLiveViewInfoPageClick(int)));
                            }
                            m_startLiveViewInfoPage->loadInfoPage(MAIN_WINDOW_STARTLIVEVIEWMESSAGE(), true, true, "", "Yes", "No");
                        }
                    }
                }
                break;

                case CHK_PRIV:
                {
                    if (IS_VALID_OBJ(m_layout))
                    {
                        m_layout->processDeviceResponse(param, deviceName);
                    }
                }
                break;

                case ADVANCE_STS:
                case ADVANCE_STS_CAMERA:
                case HEALTH_STS:
                {
                    if (m_systemDetail != NULL)
                    {
                        m_systemDetail->processDeviceResponse(param, deviceName);
                    }

                    if (m_setupWizard != NULL)
                    {
                        m_setupWizard->processDeviceResponse(param, deviceName);
                    }
                }
                break;

                case SEARCH_RCD_DATA:
                {
                    if (m_playbackSearch != NULL)
                    {
                        m_playbackSearch->processDeviceResponse(param, deviceName);
                    }
                    else if (m_eventLogSearch != NULL)
                    {
                        m_eventLogSearch->processDeviceResponse(param, deviceName);
                    }
                }
                break;

                case GET_MAN_BKP_LOC:
                case SEARCH_RCD_ALL_DATA:
                {
                    if (m_playbackSearch != NULL)
                    {
                        m_playbackSearch->processDeviceResponse(param, deviceName);
                    }
                    else if (m_quickBackup != NULL)
                    {
                        m_quickBackup->processDeviceResponse(param, deviceName);
                    }
                }
                break;

                case BKUP_RCD:
                {
                    if (m_playbackSearch != NULL)
                    {
                        if (m_quickBackup == NULL)
                        {
                            m_playbackSearch->processDeviceResponse(param, deviceName);
                        }
                        else if (m_quickBackup->getQuickBackupFlag() == false)
                        {
                            m_playbackSearch->processDeviceResponse(param, deviceName);
                        }
                    }
                    else if (m_eventLogSearch != NULL)
                    {
                        m_eventLogSearch->processDeviceResponse(param, deviceName);
                    }

                    if (m_quickBackup != NULL)
                    {
                        m_quickBackup->processDeviceResponse(param, deviceName);
                    }
                }
                break;

                case BUZ_CTRL:
                {
                    if (m_buzzerControl != NULL)
                    {
                        m_buzzerControl->processDeviceResponse(param, deviceName);
                    }
                }
                break;

                case USBUNPLUG:
                {
                    if (m_settings != NULL)
                    {
                        m_settings->processDeviceResponse(param, deviceName);
                    }

                    if (m_usbControl != NULL)
                    {
                        m_usbControl->processDeviceResponse(param, deviceName);
                    }

                    if (UsbControl::currentNoOfUsb == 0)
                    {
                        emit sigNotifyStatusIcon(USB_CONTROL_BUTTON, false);
                    }
                }
                break;

                case RESTART:
                {
                    if (m_managePages != NULL)
                    {
                        m_managePages->processDeviceResponse(param, deviceName);
                    }
                }
                break;

                case VALIDATE_USER_CRED:
                {
                    if (m_displaySetting != NULL)
                    {
                        m_displaySetting->processDeviceResponse(param, deviceName);
                    }
                }
                break;

                case VIEW_BLK_USR:
                case BLK_USR:
                case UNBLK_USR:
                case ONLINE_USR:
                case ALRM_OUT:
                case SHUTDOWN:
                case SRT_MAN_TRG:
                case STP_MAN_TRG:
                case FAC_DEF_CFG:
                case TEST_EMAIL_ID:
                case GET_PWD_RST_INFO:
                case SET_PWD_RST_INFO:
                {
                    if (m_managePages != NULL)
                    {
                        m_managePages->processDeviceResponse(param, deviceName);
                    }
                    else if (m_logIn != NULL)
                    {
                        m_logIn->processDeviceResponse(param, deviceName);
                    }
                }
                break;

                case PLYBCK_SRCH_MNTH:
                case PLYBCK_SRCH_DAY:
                case PLYBCK_RCD:
                case SYNC_PLYBCK_RCD:
                case PAUSE_RCD:
                case STP_RCD:
                case SYNC_STEP_FORWARD:
                case SYNC_STEP_REVERSE:
                case SYNC_CLP_RCD:
                case SYNC_PB_AUDIO_INCLD:
                case CLR_RCD:
                {
                    if (m_syncPlayback != NULL)
                    {
                        m_syncPlayback->processDeviceResponse(param, deviceName);
                    }
                }
                break;

                case STP_BCKUP:
                {
                    if (m_playbackSearch != NULL)
                    {
                        m_playbackSearch->processDeviceResponse(param, deviceName);
                    }
                    else if (m_syncPlayback != NULL)
                    {
                        m_syncPlayback->processDeviceResponse(param, deviceName);
                    }
                    else if (m_quickBackup != NULL)
                    {
                        m_quickBackup->processDeviceResponse(param, deviceName);
                    }
                }
                break;

                case CNG_PWD:
                case CNG_USER:
                {
                    m_totalRecord = 0;
                    if (m_setupWizard != NULL)
                    {
                        m_setupWizard->processDeviceResponse(param, deviceName);
                    }

                    if (m_settings != NULL)
                    {
                        m_settings->processDeviceResponse(param, deviceName);
                    }
                    else if (m_logIn !=  NULL)
                    {
                        m_logIn->processDeviceResponse(param, deviceName);
                    }
                    else if (m_managePages != NULL)
                    {
                        m_managePages->processDeviceResponse(param, deviceName);
                    }

                    if ((param->cmdType == CNG_USER) && (param->deviceStatus == CMD_SUCCESS))
                    {
                        for (quint8 displayIndex = 0; displayIndex < MAX_DISPLAY_TYPE; displayIndex++)
                        {
                            m_layout->refreshCurrentPage((DISPLAY_TYPE_e)displayIndex);
                        }
                    }

                    /* If audio ON then OFF on logout */
                    if (m_layout->lastEnableAudioWindow != MAX_CHANNEL_FOR_SEQ)
                    {
                        m_layout->includeExcludeAudio(m_layout->lastEnableAudioWindow);
                    }

                    /* Microphone On then stop Audio-In on User LogOut event */
                    if (MAX_CHANNEL_FOR_SEQ != m_layout->m_activeMicroPhoneWindow)
                    {
                        m_layout->sendIncludeExcludeDevAudioInCmd(m_layout->m_activeMicroPhoneWindow, false);
                    }

                    m_layout->stopAutoPlayFeature();
                    DEV_TABLE_INFO_t devTable;
                    m_applController->GetDeviceInfo (deviceName, devTable);

                    if (deviceName == LOCAL_DEVICE_NAME)
                    {
                        m_applController->setUserRightsResponseNotify(false);
                    }

                    sendCommand(deviceName, GET_USR_RIGHTS);
                    if ((param->cmdType == CNG_USER) && (param->deviceStatus == CMD_SUCCESS))
                    {
                        m_userLanguagePendingF = true;
                    }
                }
                break;

                case SEARCH_STR_EVT:
                {
                    if (m_eventLogSearch != NULL)
                    {
                        m_eventLogSearch->processDeviceResponse(param, deviceName);
                    }
                }
                break;

                case LOGOUT:
                {
                    m_layout->actionWindowforDeviceStatechange(deviceName, LOGGED_OUT);
                }
                break;

                case SNP_SHT:
                {
                    if (param->deviceStatus == CMD_SUCCESS)
                    {
                        MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(MAIN_WINDOW_SNAPSHOT_SUCCESS));
                    }
                    else
                    {
                        MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                    }
                }
                break;

                case CPU_LOAD:
                {
                    m_toolbar->processDeviceResponse(param,deviceName);
                }
                break;

                case GET_USR_RIGHTS:
                {
                    if (param->deviceStatus != CMD_SUCCESS)
                    {
                        if (param->deviceStatus != CMD_INTERNAL_RESOURCE_LIMIT)
                        {
                            EPRINT(DEVICE_CLIENT, "get usr rights resp: [status=%d]", param->deviceStatus);
                            break;
                        }

                        sendCommand(deviceName, GET_USR_RIGHTS);
                        break;
                    }

                    quint8 cameraCount = 0;
                    m_payloadLib->parseDevCmdReply(true, param->payload);
                    if (m_applController->GetTotalCamera(deviceName, cameraCount))
                    {
                        for (quint8 index = 0;index < cameraCount; index++)
                        {
                            m_applController->SetCameraRights(deviceName, index, m_payloadLib->getCnfgArrayAtIndex(index).toUInt());
                        }
                    }

                    if (deviceName != LOCAL_DEVICE_NAME)
                    {
                        break;
                    }

                    m_applController->setUserRightsResponseNotify(true);
                    if (m_playbackSearch != NULL)
                    {
                        QStringList devList;
                        quint8 devListCount;
                        m_applController->GetEnableDevList(devListCount, devList);
                        m_playbackSearch->slotSpinboxValueChanged(devList.at(0), 1);
                    }
                    else if (m_syncPlayback != NULL)
                    {
                        m_syncPlayback->updateOnNewDeviceConnected();
                    }

                    if (m_quickBackup != NULL)
                    {
                        m_quickBackup->updateChangeCameraElement();
                    }

                    if (m_userLanguagePendingF == true)
                    {
                        getUserPreferredLanguage();
                    }
                }
                break;

                case AUTO_CFG_STATUS_RPRT:
                {
                    processDeviceResponse(param, deviceName);
                }
                break;

                case STRT_CLNT_AUDIO:
                case STOP_CLNT_AUDIO:
                {
                    processClntStrmReqRes(param->cmdType, param->deviceStatus);
                }
                break;

                default:
                {
                    /* Nothing */
                }
                break;
            }
        }
        break;

        case MSG_GET_CFG:
        case MSG_SET_CFG:
        {
            if (m_managePages != NULL)
            {
                m_managePages->processDeviceResponse(param, deviceName);
            }

            if ((m_autoAddCam != NULL) && (param->windowId == WIN_ID_MXAUTO_ADD_CAM))
            {
                m_autoAddCam->processDeviceResponse(param, deviceName);
            }
            else if (m_setupWizard != NULL)
            {
                m_setupWizard->processDeviceResponse(param, deviceName);
            }
            else if (m_logIn !=  NULL)
            {
                m_logIn->processDeviceResponse(param, deviceName);
            }
            else if (m_isAutoTmZnUpdtRunning == true)
            {
                 processDeviceResponse(param,deviceName);
            }
            else if ((m_settings != NULL) && (param->windowId == MAX_WIN_ID))
            {
                m_settings->processDeviceResponse(param, deviceName);
            }
            else if (m_syncPlayback != NULL)
            {
                m_syncPlayback->processDeviceResponse(param, deviceName);
            }
            else if (m_playbackSearch !=  NULL)
            {
                m_playbackSearch->processDeviceResponse(param, deviceName);
            }
            else if (m_toolbar->getCurrentToolBarButton() == MAX_TOOLBAR_BUTTON)
            {
                m_layout->processDeviceResponse(param, deviceName);
            }

            if (m_isAutoCnfgChecked == true)
            {
                m_isAutoCnfgChecked = false;
                processDeviceResponse(param, deviceName);
            }
        }
        break;

        case MSG_DEF_CFG:
        {
            if (m_settings != NULL)
            {
                m_settings->processDeviceResponse(param, deviceName);
            }
        }
        break;

        case MSG_PWD_RST:
        {
            if (m_logIn !=  NULL)
            {
                m_logIn->processDeviceResponse(param, deviceName);
            }
        }
        break;

        default:
        {
            /* Nothing */
        }
        break;
    }
    DELETE_OBJ(param);
}

void MainWindow::slotEventToGui(QString deviceName, quint8 tEventType, quint8 eventSubType, quint8 eventIndex,
                                quint8 eventState, QString eventAdvanceDetail, bool isLiveEvent)
{
    QString evtUsrName;

    switch (tEventType)
    {
        case LOG_OTHER_EVENT:
        {
            switch (eventSubType)
            {
                case LOG_BUZZER_STATUS:
                {
                    if (deviceName != LOCAL_DEVICE_NAME)
                    {
                        break;
                    }

                    emit sigNotifyStatusIcon(BUZZER_CONTROL_BUTTON, (eventState == EVENT_ACTIVE ? true : false));
                }
                break;

                case LOG_USB_STATUS:
                {
                    if (deviceName != LOCAL_DEVICE_NAME)
                    {
                        break;
                    }

                    bool state = (eventState == EVENT_ACTIVE) ? true : false;
                    UsbControl::updateShowUsbFlag(eventIndex, state);

                    if ((state == true) || ((state == false) && (UsbControl::currentNoOfUsb == 0)))
                    {
                        emit sigNotifyStatusIcon(USB_CONTROL_BUTTON, state);
                    }

                    if (m_usbControl != NULL)
                    {
                        m_usbControl->setCurrentUsbStatus(eventIndex, state);
                    }
                }
                break;

                case LOG_MANUAL_BACKUP_STATUS:
                {
                    if (m_playbackSearch != NULL)
                    {
                        if (m_quickBackup == NULL)
                        {
                            m_playbackSearch->updateManualBkpStatusEvtAction(deviceName, eventIndex);
                        }
                        else if ((m_quickBackup != NULL) && (m_quickBackup->getQuickBackupFlag() == false))
                        {
                            m_playbackSearch->updateManualBkpStatusEvtAction(deviceName, eventIndex);
                        }

                    }
                    else if (m_syncPlayback != NULL)
                    {
                        if (m_quickBackup == NULL)
                        {
                            m_syncPlayback->updateManualBackupStatusEventAction(deviceName, eventIndex);
                        }
                        else if ((m_quickBackup != NULL) && (m_quickBackup->getQuickBackupFlag() == false))
                        {
                            m_syncPlayback->updateManualBackupStatusEventAction(deviceName, eventIndex);
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
        break;

        case LOG_ALARM_EVENT:
        {
            switch (eventSubType)
            {
                case LOG_ALARM_OUTPUT:
                {
                    if (m_managePages != NULL)
                    {
                        m_managePages->updatePage(deviceName, eventState, (eventIndex - 1), tEventType, eventSubType);
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

        case LOG_USER_EVENT:
        {
            switch (eventSubType)
            {
                case LOG_MANUAL_TRIGGER:
                {
                    if (m_managePages != NULL)
                    {
                        m_managePages->updatePage(deviceName, eventState, (eventIndex - 1), tEventType, eventSubType);
                    }
                }
                break;

                case LOG_CONFIG_CHANGE:
                {
                    if (eventIndex == CAMERA_TABLE_INDEX)
                    {
                        for (quint8 displayIndex = 0; displayIndex < deviceRespInfo.maxDisplayOutput; displayIndex++)
                        {
                            if ((Layout::currentModeType[displayIndex] == STATE_OTHER_MODE_NONE)
                                    && (Layout::isAnyWindowReplacing((DISPLAY_TYPE_e)displayIndex) == false))
                            {
                                m_layout->setupUIData((DISPLAY_TYPE_e)displayIndex, deviceName);

                                DEV_TABLE_INFO_t devTableInfo;
                                memset(&devTableInfo, 0, sizeof(devTableInfo));
                                if (false == m_applController->GetDeviceInfo(deviceName, devTableInfo))
                                {
                                    EPRINT(DEVICE_CLIENT, "fail to get device info");
                                }

                                for (quint8 cameraIdx = 0; cameraIdx < devTableInfo.totalCams; cameraIdx++)
                                {
                                    quint16 windowIndex = Layout::findWindowOfLiveStream((DISPLAY_TYPE_e)displayIndex, deviceName, cameraIdx);
                                    if (windowIndex != MAX_CHANNEL_FOR_SEQ)
                                    {
                                        if ((Layout::streamInfoArray[displayIndex][windowIndex].m_streamId == MAX_STREAM_SESSION)
                                                && (Layout::streamInfoArray[displayIndex][windowIndex].m_startRequestPending == false)
                                                && (Layout::streamInfoArray[displayIndex][windowIndex].m_stopRequestPending == false)
                                                && (Layout::streamInfoArray[displayIndex][windowIndex].m_startInstantPlayback == false)
                                                && (Layout::streamInfoArray[displayIndex][windowIndex].m_stopInstantPlayback == false))
                                        {
                                            m_layout->startLiveStream((DISPLAY_TYPE_e)displayIndex,
                                                                      Layout::streamInfoArray[displayIndex][windowIndex].m_deviceName,
                                                                      Layout::streamInfoArray[displayIndex][windowIndex].m_cameraId, windowIndex);
                                        }
                                    }
                                }
                            }
                        }

                        if (m_osdUpdate != NULL)
                        {
                            m_osdUpdate->updateAllOSD(true);
                        }
                    }
                    else if (eventIndex == GENERAL_TABLE_INDEX)
                    {
                        if (m_displaySetting != NULL)
                        {
                            m_displaySetting->updateDeviceState(deviceName, CONNECTED);
                        }

                        m_layout->updateDeviceState(deviceName, CONNECTED);
                        m_layout->setupUIData(MAIN_DISPLAY, deviceName);
                        slotDeviceListChangeToGui();

                        DEV_TABLE_INFO_t deviceInfo;
                        memset(&deviceInfo, 0, sizeof(deviceInfo));
                        if (false == m_applController->GetDeviceInfo(LOCAL_DEVICE_NAME, deviceInfo))
                        {
                            EPRINT(DEVICE_CLIENT, "fail to get device info");
                        }

                        if ((deviceInfo.startLiveView == false) && (deviceInfo.startLiveView != m_preStartLiveViewF))
                        {
                            m_preStartLiveViewF = deviceInfo.startLiveView;

                            if (m_toolbarPage != NULL)
                            {
                                if (IS_VALID_OBJ(m_displayMode))
                                {
                                  m_toolbar->closeToolbarPage(m_toolbar->getCurrentToolBarButton());
                                }

                                if (IS_VALID_OBJ(m_displaySetting))
                                {
                                    m_toolbar->closeToolbarPage(m_toolbar->getCurrentToolBarButton());
                                }
                            }

                            m_toolbar->deletePicklist();
                            m_layout->stopLocalDecoding (deviceInfo.startLiveView, eventAdvanceDetail);
                        }
                        else if ((deviceInfo.startLiveView != m_preStartLiveViewF))
                        {
                            m_preStartLiveViewF = deviceInfo.startLiveView;
                            m_layout->startLocalDecoding(deviceInfo.startLiveView);
                        }
                    }
                }
                break;

                case LOG_ALLOWED_ACCESS:
                {
                    if (m_applController->getUsernameFrmDev(deviceName, evtUsrName))
                    {
                        if (evtUsrName == eventAdvanceDetail)
                        {
                            MessageBanner::addMessageInBanner(deviceName + QString(" : ") + Multilang("Login Session will expire in 1 minute"));
                        }
                    }
                }
                break;

                case LOG_SESSION_EXPIRE:
                {
                    if (false == m_applController->getUsernameFrmDev(LOCAL_DEVICE_NAME, evtUsrName))
                    {
                        break;
                    }

                    if (evtUsrName != eventAdvanceDetail)
                    {
                        break;
                    }

                    if (m_toolbar != NULL)
                    {
                        slotUnloadToolbar();
                        MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(MAIN_WINDOW_LOGIN_EXPIRED));
                        m_toolbar->openToolbarPage(LOG_BUTTON);
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

        case LOG_CAMERA_EVENT:
        {
            switch (eventSubType)
            {
                case LOG_MOTION_DETECTION:
                case LOG_NO_MOTION_DETECTION:
                case LOG_VIEW_TEMPERING:
                case LOG_MANUAL_RECORDING:
                case LOG_ALARM_RECORDING:
                case LOG_SCHEDULE_RECORDING:
                case LOG_LINE_CROSSING:
                case LOG_INTRUSION_DETECTION:
                case LOG_AUDIO_EXCEPTION_DETECTION:
                case LOG_MISSING_OBJECT:
                case LOG_SUSPICIOUS_OBJECT:
                case LOG_LOITERING:
                {
                    for (quint8 displayIndex = 0; displayIndex < deviceRespInfo.maxDisplayOutput; displayIndex++)
                    {
                        if ((Layout::currentModeType[displayIndex] == STATE_OTHER_MODE_NONE)
                                && (Layout::isAnyWindowReplacing((DISPLAY_TYPE_e)displayIndex) == false))
                        {
                            m_layout->setupUIData((DISPLAY_TYPE_e)displayIndex, deviceName, eventIndex);
                        }
                    }
                }
                break;

                case LOG_CONNECTIVITY:
                {
                    if (eventState != EVENT_CONNECT)
                    {
                        break;
                    }

                    for (quint8 displayIndex = 0; displayIndex < deviceRespInfo.maxDisplayOutput; displayIndex++)
                    {
                        if ((Layout::currentModeType[displayIndex] == STATE_OTHER_MODE_NONE)
                                && (Layout::isAnyWindowReplacing((DISPLAY_TYPE_e)displayIndex) == false))
                        {
                            quint16 windowIndex = Layout::findWindowOfLiveStream((DISPLAY_TYPE_e)displayIndex, deviceName, eventIndex);
                            if (windowIndex != MAX_CHANNEL_FOR_SEQ)
                            {
                                m_layout->startLiveStream((DISPLAY_TYPE_e)displayIndex,
                                                          Layout::streamInfoArray[displayIndex][windowIndex].m_deviceName,
                                                          Layout::streamInfoArray[displayIndex][windowIndex].m_cameraId, windowIndex);
                            }
                        }
                    }
                }
                break;

                case LOG_PRESET_TOUR:
                {
                    m_layout->sendEventToPtz(deviceName, eventIndex, (LOG_EVENT_STATE_e)eventState);
                }
                break;

                case LOG_VIDEO_POP_UP:
                {
                    if (m_showVideoPopupF == false)
                    {
                        /* If video popup is off from toolbar btn then discard events. */
                        break;
                    }

                    if (IS_VALID_OBJ(m_syncPlayback))
                    {
                        #if 0
                        if (m_syncPlayback->stopSyncBackupManual() == true)
                        {
                            m_syncPlayback->stopSyncPlaybackRecord();
                            m_syncPlayback->processStopSyncPlaybackAction();
                            m_syncPlayback->clearSyncPlaybackRecord();
                        }
                        #endif

                        /* Note: Handling of sync playback is pending, so for now, video popup event is ignored when sync playback is going on */
                        break;
                    }

                    DEV_TABLE_INFO_t deviceInfo;
                    memset(&deviceInfo, 0, sizeof(deviceInfo));
                    if (false == m_applController->GetDeviceInfo(LOCAL_DEVICE_NAME, deviceInfo))
                    {
                        EPRINT(DEVICE_CLIENT, "fail to get device info");
                    }

                    if (deviceInfo.startLiveView == false)
                    {
                        break;
                    }

                    if (m_toolbarPage != NULL)
                    {
                        if (IS_VALID_OBJ(m_quickBackup))
                        {
                            m_quickBackup->hide();
                            m_quickBackup->setMinimizeFlag(true);
                        }

                        m_toolbar->closeToolbarPage(m_toolbar->getCurrentToolBarButton());
                    }

                    m_applController->GetDeviceInfo(deviceName, deviceInfo);
                    m_layout->showVideoPopupFeature(eventIndex, deviceName, eventAdvanceDetail.toInt(), deviceInfo.videoPopUpDuration);
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

        case LOG_SYSTEM_EVENT:
        {
            switch (eventSubType)
            {
                case LOG_MANUAL_BACKUP:
                {
                    if (m_playbackSearch != NULL)
                    {
                        if (m_quickBackup == NULL)
                        {
                            m_playbackSearch->manualBkpSysEvtAction(deviceName, (LOG_EVENT_STATE_e)eventState);
                        }
                        else if ((m_quickBackup != NULL) && (m_quickBackup->getQuickBackupFlag() == false))
                        {
                            m_playbackSearch->manualBkpSysEvtAction(deviceName, (LOG_EVENT_STATE_e)eventState);
                        }
                    }
                    else if (m_syncPlayback != NULL)
                    {
                        if (m_quickBackup == NULL)
                        {
                            m_syncPlayback->updateManualBackupSystemEventAction(deviceName, (LOG_EVENT_STATE_e)eventState);
                        }
                        else if ((m_quickBackup != NULL) && (m_quickBackup->getQuickBackupFlag() == false))
                        {
                            m_syncPlayback->updateManualBackupSystemEventAction(deviceName, (LOG_EVENT_STATE_e)eventState);
                        }
                    }

                    if (m_quickBackup != NULL)
                    {
                        m_quickBackup->BkpSysEvtAction(deviceName, (LOG_EVENT_STATE_e)eventState);
                    }
                }
                break;

                case LOG_SHUTDOWN:
                case LOG_RESTART:
                {
                    if (deviceName != LOCAL_DEVICE_NAME)
                    {
                        break;
                    }

                    m_toolbar->unloadToolbarPage();
                    for (quint8 dispIndex = 0; dispIndex < deviceRespInfo.maxDisplayOutput; dispIndex++)
                    {
                        m_layout->stopStreamsExceptCurrentPage((DISPLAY_TYPE_e)dispIndex);
                        m_layout->stopLiveStreamInAllWindows((DISPLAY_TYPE_e)dispIndex);
                    }

                    if (IS_VALID_OBJ(m_infoPage))
                    {
                        m_infoPage->unloadInfoPage();
                    }

                    if (IS_VALID_OBJ(m_autoAddCam))
                    {
                        m_autoAddCam->deleteLater();
                    }

                    if (IS_VALID_OBJ(m_autoTmznRebootInfoPage))
                    {
                        m_autoTmznRebootInfoPage->unloadInfoPage();
                    }

                    if (IS_VALID_OBJ(m_popUpAlert))
                    {
                        m_popUpAlert->deleteComponent();
                    }

                    if (IS_VALID_OBJ(m_startLiveViewInfoPage))
                    {
                        m_startLiveViewInfoPage->unloadInfoPage();
                    }

                    createInitDeinitBox(ValidationMessage::getValidationMessage(MAIN_WINDOW_DEINIT_MSG));
                }
                break;

                case LOG_RTC_UPDATE:
                case LOG_DST_EVENT:
                {
                    if ((m_toolbar != NULL) && (m_toolbar->isVisible()))
                    {
                        m_toolbar->getDateAndTime(LOCAL_DEVICE_NAME);
                    }

                    if (m_osdUpdate !=  NULL)
                    {
                        m_osdUpdate->updateTimeCountForOsdUpdate(0);
                    }
                }
                break;

                case LOG_RECORDING_FAIL:
                {
                    if (m_initDeInitMsgBox == NULL)
                    {
                        if (m_messageAlert == NULL)
                        {
                            isMessageAlertLoaded = true;
                            m_messageAlert = new MessageAlert(this);
                            connect(m_messageAlert ,
                                    SIGNAL(sigCloseAlert()),
                                    this,
                                    SLOT(slotMessageAlertClose()));
                        }

                        m_messageAlert->addMessageAlert(deviceName,eventIndex);
                    }
                    else if (m_messageAlert != NULL)
                    {
                        disconnect(m_messageAlert ,
                                   SIGNAL(sigCloseAlert()),
                                   this,
                                   SLOT(slotMessageAlertClose()));
                        m_messageAlert->deleteLater();
                        m_messageAlert = NULL;
                    }
                }
                break;

                case LOG_AUTO_CFG_STS_REPORT:
                {
                    if (deviceName != LOCAL_DEVICE_NAME)
                    {
                        break;
                    }

                    if (IS_VALID_OBJ(m_popUpAlert))
                    {
                        m_popUpAlert->addPopUp(AUTO_CNFG_CAM, MSG_WITH_HYPERLINK);
                        break;
                    }

                    /* PARASOFT: Memory Deallocated in slot PopUpAlertClose */
                    m_popUpAlert = new MxPopUpAlert((ApplController::getXPosOfScreen() + ApplController::getWidthOfScreen()),
                                                    (ApplController::getYPosOfScreen() + ApplController::getHeightOfScreen() - m_toolbar->height()),
                                                    AUTO_CNFG_CAM, MSG_WITH_HYPERLINK, this);
                    connect(m_popUpAlert,
                            SIGNAL(sigCloseAlert(int, bool)),
                            this,
                            SLOT(slotPopUpAlertClose(int, bool)));
                }
                break;

                case LOG_TIME_ZONE_UPDATE:
                {
                    if (deviceName != LOCAL_DEVICE_NAME)
                    {
                        break;
                    }

                    m_detectedAutoTmZnParam[TIMEZONE_INDEX] = eventIndex;
                    m_detectedAutoTmZnParam[VIDEO_STD] = eventAdvanceDetail.left(1).toUInt();
                    m_detectedAutoTmZnParam[DATE_FMT] = eventAdvanceDetail.right(1).toUInt();

                    if (IS_VALID_OBJ(m_popUpAlert))
                    {
                        m_popUpAlert->addPopUp(AUTO_TIMEZONE_UPDATE, MSG_WITH_TWO_CTRL);
                        break;
                    }

                    /* PARASOFT: Memory Deallocated in slot PopUpAlertClose */
                    m_popUpAlert = new MxPopUpAlert((ApplController::getXPosOfScreen() + ApplController::getWidthOfScreen()),
                                                    (ApplController::getYPosOfScreen() + ApplController::getHeightOfScreen() - m_toolbar->height()),
                                                    AUTO_TIMEZONE_UPDATE, MSG_WITH_TWO_CTRL, this);
                    connect(m_popUpAlert,
                            SIGNAL(sigCloseAlert(int,bool)),
                            this,
                            SLOT(slotPopUpAlertClose(int,bool)));
                }
                break;

                case LOG_TWO_WAY_AUDIO:
                {
                    if (eventState == EVENT_STOP)
                    {
                        if (IS_VALID_OBJ(m_layout) && (m_layout->m_isClientAudProcessRunning == true))
                        {
                            sendClntAudCmd(STOP_CLNT_AUDIO, CMD_SUCCESS);
                        }
                    }
                    else if (eventState == EVENT_START)
                    {
                        if (CAMERA_AUDIO_PRIORITY != (AUDIO_OUT_PRIORITY_e)eventAdvanceDetail.toUInt())
                        {
                            sendClntAudCmd(STRT_CLNT_AUDIO);
                            break;
                        }

                        if (m_layout == NULL)
                        {
                            break;
                        }

                        quint16 audioWindow = m_layout->getStartAudioInWindow();
                        if (audioWindow == MAX_CHANNEL_FOR_SEQ)
                        {
                            sendClntAudCmd(STRT_CLNT_AUDIO);
                            break;
                        }

                        if ((Layout::streamInfoArray[MAIN_DISPLAY][audioWindow].m_videoType == VIDEO_TYPE_LIVESTREAM) ||
                                (Layout::streamInfoArray[MAIN_DISPLAY][audioWindow].m_videoType == VIDEO_TYPE_LIVESTREAM_AWAITING))
                        {
                            sendClntAudCmd(STOP_CLNT_AUDIO, CMD_REQ_FAIL_CHNG_AUD_OUT_PRI);
                        }
                        else
                        {
                            sendClntAudCmd(STRT_CLNT_AUDIO);
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
        break;

        case LOG_COSEC_EVENT:
        {
            switch (eventSubType)
            {
                case LOG_COSEC_RECORDING:
                {
                    for (quint8 displayIndex = 0; displayIndex < deviceRespInfo.maxDisplayOutput; displayIndex++)
                    {
                        if ((Layout::currentModeType[displayIndex] == STATE_OTHER_MODE_NONE)
                                && (Layout::isAnyWindowReplacing((DISPLAY_TYPE_e)displayIndex) == false))
                        {
                            m_layout->setupUIData((DISPLAY_TYPE_e)displayIndex, deviceName, eventIndex);
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
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }

    if (isLiveEvent)
    {
        emit sigNotifyStatusIcon(LIVE_EVENT_BUTTON, true);
    }
}

void MainWindow::slotPopUpEvtToGui(QString deviceName, quint8 cameraIndex, QString userName, quint32 popUpTime,
                                   QString userId, QString doorName, quint8 eventCodeIndex)
{
    quint8 maxLocalCam;
    m_applController->GetTotalCamera(LOCAL_DEVICE_NAME, maxLocalCam);

    if ((deviceName == LOCAL_DEVICE_NAME) && (cameraIndex <= maxLocalCam))
    {
        if ((m_syncPlayback == NULL) && (m_layout->isPlaybackRunning() == false)
                && (Layout::currentModeType[MAIN_DISPLAY] != STATE_EXPAND_WINDOW)
                && (Layout::currentModeType[MAIN_DISPLAY] != STATE_LOCAL_DECODING))
        {
            if ((QApplication::overrideCursor() != NULL) && (QApplication::overrideCursor()->shape() == Qt::OpenHandCursor))
            {
                QApplication::setOverrideCursor(Qt::ArrowCursor);
            }

            m_layout->showCosecPopupFeature(cameraIndex, userName, popUpTime, userId, doorName, eventCodeIndex);
        }
    }

    if (m_messageAlert != NULL)
    {
        m_messageAlert->raise();
    }
}

void MainWindow::slotStreamRequestResponse(STREAM_COMMAND_TYPE_e streamCommandType, StreamRequestParam *streamRequestParam, DEVICE_REPLY_TYPE_e deviceReply)
{
    if (((streamCommandType == EXCLUDE_AUDIO_COMMAND) || (streamCommandType == AUDIO_PLABACK_STREAM_COMMAND) ||
        (streamCommandType == AUDIO_SYNCPLABACK_STREAM_COMMAND) || (streamCommandType == AUDIO_INSTANTPLAYBACK_COMMAND)) && (deviceReply == CMD_SUCCESS))
    {
        if ((m_layout->m_isClientAudProcessRunning == true))
        {
            m_clientAudInclretryCnt = 0;
            if (IS_VALID_OBJ(m_clientAudInclTimer))
            {
                m_clientAudInclTimer->start();
            }
        }
    }

    if (((streamCommandType >= START_STREAM_COMMAND) && (streamCommandType <= CLEAR_PLAYBACK_ID_COMMAND))
            || ((streamCommandType >= START_INSTANTPLAYBACK_COMMAND) && (streamCommandType <= AUDIO_INSTANTPLAYBACK_COMMAND)))
    {
        m_layout->processStreamRequestResponse(streamCommandType, streamRequestParam, deviceReply);
    }
    else if ((streamCommandType >= PLAY_SYNCPLABACK_STREAM_COMMAND) && (streamCommandType <= CLEAR_SYNCPLABACK_STREAM_COMMAND))
    {
        if (m_syncPlayback != NULL)
        {
            m_syncPlayback->processStreamRequestResponse(streamCommandType, streamRequestParam, deviceReply);
        }
    }
    else if (streamCommandType == STRT_CLNT_AUDIO_COMMAND)
    {
        if (m_clntAudStreamId != MAX_STREAM_SESSION)
        {
            m_clntAudStreamId = streamRequestParam->streamId;
        }

        processClntStrmReqRes(STRT_CLNT_AUDIO, deviceReply);
    }
    else if (streamCommandType == STOP_CLNT_AUDIO_COMMAND)
    {
        processClntStrmReqRes(STOP_CLNT_AUDIO, deviceReply);
    }

    DELETE_OBJ(streamRequestParam);
}

void MainWindow::slotOpenCameraFeature(void *param, CAMERA_FEATURE_TYPE_e featureType, quint8 cameraIndex, void *configParam, QString devName)
{
    DEV_TABLE_INFO_t devTableInfo;
    memset(&devTableInfo, 0, sizeof(devTableInfo));

    if (false == m_applController->GetDeviceInfo(LOCAL_DEVICE_NAME, devTableInfo))
    {
        EPRINT(DEVICE_CLIENT, "fail to get device info");
    }

    if ((devTableInfo.startLiveView == true))
    {
        if (m_settings != NULL)
        {
            m_settings->setVisible(false);
        }

        m_layout->showAnalogCameraFeature(cameraIndex, featureType, param, devName, configParam);
    }
    else
    {
        MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(LOCAL_DECODING_DISABLED_CLICK_ACTION));
    }
}

void MainWindow::slotCloseAnalogCameraFeature()
{
    if (m_settings != NULL)
    {
        m_settings->setVisible(true);
    }
}

void MainWindow::slotApplicationModeChanged()
{
    if (ApplicationMode::getApplicationMode() == IDLE_MODE)
    {
        m_layout->giveFocusToWindow(Layout::currentDisplayConfig[MAIN_DISPLAY].selectedWindow);
    }
}

void MainWindow::slotCloseCosecPopUp()
{
    switch (ApplicationMode::getApplicationMode())
    {
        case PAGE_MODE:
            if (m_toolbarPage != NULL)
            {
                m_toolbarPage->show();
            }
            break;

        case IDLE_MODE:
            m_layout->loadAndShowOptionsInCosecPopup();
            break;

        default:
            break;
    }

    if (m_messageAlert != NULL)
    {
        m_messageAlert->raise();
    }
}

void MainWindow::slotUnloadToolbar()
{
    m_toolbar->deletePicklist();
    m_toolbar->unloadToolbarPage();

    if (m_toolbar->isVisible())
    {
        m_toolbar->setVisible(false);
    }
}

void MainWindow::slotHideToolbarPage()
{
    if (m_toolbarPage != NULL)
    {
        m_toolbarPage->hide();
    }
}

void MainWindow::slotInfoPageButtonClicked(int index)
{
    if (IS_VALID_OBJ(m_hddCleanInfoTimer))
    {
        m_hddCleanInfoTimer->stop();
    }

    if (IS_VALID_OBJ(m_autoAddCam))
    {
        m_autoAddCam->setIsInfoPageLoaded(false);
    }

    if (index == INFO_OK_BTN)
    {
        DevCommParam* param = new DevCommParam();
        param->msgType = MSG_SET_CMD;
        param->cmdType = DISK_CLEANUP;
        m_applController->processActivity(LOCAL_DEVICE_NAME, DEVICE_COMM, param);
    }
}

void MainWindow::slotDeviceListChangeToGui()
{
    if (m_settings != NULL)
    {
        m_settings->updateDeviceList();
    }

    if (m_playbackSearch != NULL)
    {
        m_playbackSearch->updateDeviceList();
    }

    if (m_syncPlayback != NULL)
    {
        m_syncPlayback->updateDeviceList();
    }

    if (m_eventLogSearch != NULL)
    {
        m_eventLogSearch->updateDeviceList();
    }

    if (m_managePages != NULL)
    {
        m_managePages->updateDeviceList();
    }

    if (m_systemDetail != NULL)
    {
        m_systemDetail->updateDeviceList();
    }

    if (m_quickBackup != NULL)
    {
        m_quickBackup->updateDeviceList();
    }

    if (m_liveEvent != NULL)
    {
        m_liveEvent->updateDeviceList();
    }
}

void MainWindow::slotLoadProcessBar()
{
    if (m_settings != NULL)
    {
        m_settings->loadProcessBar();
    }
}

void MainWindow::slotStreamObjectDelete(DISPLAY_TYPE_e *displayTypeForDelete, quint16 *actualWindowIdForDelete)
{
    m_layout->processStreamObjectDelete(displayTypeForDelete, actualWindowIdForDelete);
}

void MainWindow::slotMessageAlertClose()
{
    if (m_messageAlert != NULL)
    {
        disconnect(m_messageAlert ,
                   SIGNAL(sigCloseAlert()),
                   this,
                   SLOT(slotMessageAlertClose()));
        m_messageAlert->deleteLater();
        m_messageAlert = NULL;
        isMessageAlertLoaded = false;
    }
}

void MainWindow::slotRaiseWidget()
{
    if (IS_VALID_OBJ(m_autoAddCam))
    {
        m_autoAddCam->raise();
        m_autoAddCam->raiseVirtualKeypad();
    }

    if (IS_VALID_OBJ(m_popUpAlert))
    {
        m_popUpAlert->raise();
    }

    if (IS_VALID_OBJ(m_messageAlert))
    {
        m_messageAlert->raise();
    }
}

void MainWindow::slotAutoAddClose()
{
    if (IS_VALID_OBJ(m_autoAddCam))
    {
        disconnect(m_autoAddCam ,
                   SIGNAL(sigCloseAlert()),
                   this,
                   SLOT(slotAutoAddClose()));
        disconnect(m_autoAddCam,
                SIGNAL(sigAutoAddCamPopUpAction(MX_AUTO_ADD_CAM_SEARCH_ACTION_e)),
                this,
                SLOT(slotAutoAddCamPopUpAction(MX_AUTO_ADD_CAM_SEARCH_ACTION_e)));

        m_autoAddCam->deleteLater();
        m_autoAddCam = NULL;
    }
    isOnbootAuoCamSearchRunning = false;
}

void MainWindow::slotHddCleanInfoTimeout()
{
    m_hddTimerCnt--;
    if (m_hddTimerCnt == 0)
    {
        if (IS_VALID_OBJ(m_hddCleanInfoTimer))
        {
            m_hddCleanInfoTimer->stop();
        }

        if (IS_VALID_OBJ(m_infoPage))
        {
            m_infoPage->unloadInfoPage();
        }

        if (IS_VALID_OBJ(m_autoAddCam))
        {
            m_autoAddCam->setIsInfoPageLoaded(false);
        }
    }
    else
    {
        if (IS_VALID_OBJ(m_infoPage))
        {
            m_infoPage->changeText(MAIN_WINDOW_DISKCLEANUP_REQUIRED_MESSAGE(m_hddTimerCnt));
        }
    }
}

void MainWindow::slotAutoAddCamPopUpAction(MX_AUTO_ADD_CAM_SEARCH_ACTION_e action)
{
    switch (action)
    {
        case MX_AUTO_ADD_CAM_PAUSE_PLAYBACK:        // Pause Playbacks
        {
            if (IS_VALID_OBJ(m_syncPlayback))
            {
                m_syncPlayback->pauseSyncPlaybackRecord();
            }

            if (IS_VALID_OBJ(m_layout))
            {
                m_layout->pausePlaybackStreamOfCurrentPage();
            }
        }
        break;

        case MX_AUTO_ADD_CAM_SEARCH_PAGE_LOAD:
        {
            if (m_toolbar == NULL)
            {
                break;
            }

            m_autoAddCam->setIsCamSearchPageLoaded(true);
            m_autoAddCamAction = MX_AUTO_ADD_CAM_SEARCH_PAGE_LOAD;
            m_isUnloadAutoAddCam = false;

            if ((m_toolbar->getCurrentToolBarButton() != MAX_TOOLBAR_BUTTON) && (m_toolbar->getCurrentToolBarButton() != SETTINGS_BUTTON))
            {
                if (m_toolbar->getCurrentToolBarButton() == LOG_BUTTON)
                {
                   m_toolbar->changeNextToolBarBtnLoad(SETTINGS_BUTTON);
                }
                else
                {
                    m_toolbar->closeToolbarPage(m_toolbar->getCurrentToolBarButton());
                    m_toolbar->openToolbarPage(SETTINGS_BUTTON);
                }
            }
            else
            {
                m_toolbar->openToolbarPage(SETTINGS_BUTTON);
            }
        }
        break;

        case MX_AUTO_ADD_CAM_LOGIN:
        {
            if (m_toolbar == NULL)
            {
                break;
            }

            if (m_toolbar->getCurrentToolBarButtonState(LOG_BUTTON) == STATE_1)
            {
                m_isUnloadAutoAddCam = true;
                if (m_toolbar->getCurrentToolBarButton() != LOG_BUTTON)
                {
                    if (m_toolbar->getCurrentToolBarButton() != MAX_TOOLBAR_BUTTON)
                    {
                        m_toolbar->closeToolbarPage(m_toolbar->getCurrentToolBarButton());
                    }

                    m_toolbar->openToolbarPage(LOG_BUTTON);
                }
            }
            else
            {
                USRS_GROUP_e currentUserType = MAX_USER_GROUP;
                if (false == m_applController->GetUserGroupType(LOCAL_DEVICE_NAME,currentUserType))
                {
                    EPRINT(DEVICE_CLIENT, "fail to get user group type");
                }

                if (currentUserType != ADMIN)
                {
                    m_autoAddCamInfoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(CMD_NO_PRIVILEGE));
                    m_autoAddCamInfoPage->raise();
                }
                else
                {
                    m_autoAddCam->saveConfiguration();
                    m_autoAddCam->loadAutoAddCamAfterLogin();
                }
            }
        }
        break;

        case MX_AUTO_ADD_CAM_TIME_OUT:
        {
            m_isAutoAddCamTimeout = true;
        }
        break;

        default:
        {
            /* Nothing to */
        }
        break;
    }

    slotRaiseWidget();
}

void MainWindow::slotPopUpAlertClose(int index, bool isQueueEmpty)
{
    if (m_popUpAlert == NULL)
    {
        return;
    }

    POP_UP_ALERT_NAME_e popUpName = m_popUpAlert->getPopUpName();
    switch (popUpName)
    {
        case AUTO_CNFG_CAM:
        {
            if (index == POP_UP_ALRT_CTRL_NO_HYPLINK)
            {
                sendCommand(LOCAL_DEVICE_NAME, AUTO_CFG_STATUS_RPRT);
            }
        }
        break;

        case AUTO_TIMEZONE_UPDATE:
        {
            if (index == POP_UP_ALRT_CTRL_YES)
            {
                m_isAutoTmZnUpdtRunning = true;
                if (IS_VALID_OBJ(m_toolbar))
                {
                    if (m_toolbar->getCurrentToolBarButtonState(LOG_BUTTON) == STATE_1)
                    {
                        m_toolbar->openToolbarPage(LOG_BUTTON);
                    }
                    else
                    {
                        getAutoUpdtRegFlg();
                    }
                }
            }
        }
        break;

        default:
        {
            /* Nothing to */
        }
        break;
    }

    if (isQueueEmpty == true)
    {
        disconnect(m_popUpAlert,
                   SIGNAL(sigCloseAlert(int, bool)),
                   this,
                   SLOT(slotPopUpAlertClose(int, bool)));
        m_popUpAlert->deleteLater();
        m_popUpAlert = NULL;
    }
}

void MainWindow::slotReportTableClose(quint8 pageNo)
{
    m_reportCurPageNo = pageNo;

    if (IS_VALID_OBJ(m_reportTable))
    {
        disconnect(m_reportTable,
                   SIGNAL(sigClosePage(quint8)),
                   this,
                   SLOT(slotReportTableClose(quint8)));
        DELETE_OBJ(m_reportTable);
    }
}

void MainWindow::slotAutoTmznRbtInfoPgBtnClick(int index)
{
    if (index == INFO_OK_BTN)
    {
        setAutoTimeZoneIndex(true);
    }
    else
    {
        m_isAutoTmZnUpdtRunning = false;
    }

    if (IS_VALID_OBJ(m_autoTmznRebootInfoPage))
    {
        disconnect(m_autoTmznRebootInfoPage,
                   SIGNAL(sigInfoPageCnfgBtnClick(int)),
                   this,
                   SLOT(slotAutoTmznRbtInfoPgBtnClick(int)));
        DELETE_OBJ(m_autoTmznRebootInfoPage);
    }
}

void MainWindow::slotLanguageCfgModified(QString langStr)
{
    qApp->processEvents();
    qApp->removeTranslator(&m_languageTranslator);

    if (langStr == "English")
    {
        return;
    }

    QString tsFilePath = LANGUAGES_DIR_PATH "/" + langStr + ".ts";
    QString qmFilePath = LANGUAGES_DIR_PATH "/" + langStr + ".qm";
    QString cmd ="lrelease "+ tsFilePath + " -qm " + qmFilePath + " > /tmp/result.txt";
    DPRINT(GUI_SYS, "convert language file: [language=%s], [cmd=%s]", langStr.toUtf8().constData(), cmd.toUtf8().constData());

    if (Utils_ExeCmd(cmd.toUtf8().data()) == FAIL)
    {
        EPRINT(GUI_SYS, "fail to convert language file");
    }

    if (m_languageTranslator.load(qmFilePath))
    {
        qApp->installTranslator(&m_languageTranslator);
    }
    else
    {
        m_multiLanguageInfoPage->raise();
        m_multiLanguageInfoPage->loadInfoPage("Error occurred while translating language");
        qApp->removeTranslator(&m_languageTranslator);
    }
}

void MainWindow::slotHdmiInfoPage(bool isHdmiInfoShow)
{
    m_layout->slotChangeLayout(Layout::currentDisplayConfig[MAIN_DISPLAY].layoutId, MAIN_DISPLAY,
                               Layout::currentDisplayConfig[MAIN_DISPLAY].selectedWindow, false, true);
    this->window()->repaint();
    Q_UNUSED(isHdmiInfoShow);
}

void MainWindow::slotUpdateQuickbackupFlag(bool flag)
{
    if (IS_VALID_OBJ(m_playbackSearch))
    {
        m_playbackSearch->setQuickBackupFlag(flag);
    }
    else if (IS_VALID_OBJ(m_syncPlayback))
    {
        m_syncPlayback->setQuickBackupFlag(flag);
    }
}

void MainWindow::slotClientAudInclude()
{
    BOOL status = IncludeAudio(CLIENT_AUDIO_DECODER_ID);
    if (status == SUCCESS)
    {
        m_clientAudInclTimer->stop();
        m_clientAudInclretryCnt = 0;
        m_applController->slotChangeAudState();
        return;
    }

    m_clientAudInclretryCnt++;
    if (m_clientAudInclretryCnt < MAX_CLIENT_INC_AUD_RETRY_CNT)
    {
        return;
    }

    m_clientAudInclTimer->stop();
    m_clientAudInclretryCnt = 0;
    sendClntAudCmd(STOP_CLNT_AUDIO, CMD_AUD_SND_REQ_FAIL);
}

void MainWindow::slotStopClntAud(DEVICE_REPLY_TYPE_e statusId)
{
    sendClntAudCmd(STOP_CLNT_AUDIO, statusId);
}

void MainWindow::slotStartLiveViewInfoPageClick(int index)
{
    if (index == INFO_OK_BTN)
    {
        if (IS_VALID_OBJ(m_toolbar))
        {
            if (m_toolbar->getCurrentToolBarButtonState (LOG_BUTTON) == STATE_1)
            {
                if (m_toolbar->getCurrentToolBarButton() != LOG_BUTTON)
                {
                    if (m_toolbar->getCurrentToolBarButton() != MAX_TOOLBAR_BUTTON)
                    {
                        m_toolbar->closeToolbarPage(m_toolbar->getCurrentToolBarButton());
                    }

                    m_toolbar->openToolbarPage(LOG_BUTTON);
                }
            }
            else
            {
                USRS_GROUP_e currentUserType = MAX_USER_GROUP;

                if (false == m_applController->GetUserGroupType(LOCAL_DEVICE_NAME, currentUserType))
                {
                    EPRINT(DEVICE_CLIENT, "fail to get user group type");
                }

                if (currentUserType != ADMIN)
                {
                    MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(CMD_NO_PRIVILEGE));
                }
                else
                {
                    m_payloadLib->setCnfgArrayAtIndex(0, 1);
                    QString payloadString = m_payloadLib->createDevCnfgPayload(MSG_SET_CFG,
                                                                               GENERAL_TABLE_INDEX,
                                                                               CNFG_FRM_INDEX,
                                                                               1,
                                                                               FIELD_START_LIVE_VIEW_FLAG + 1,
                                                                               FIELD_START_LIVE_VIEW_FLAG + 1,
                                                                               1);
                    DevCommParam* param = new DevCommParam();
                    param->msgType = MSG_SET_CFG;
                    param->payload = payloadString;
                    param->windowId = WIN_ID_LIVE_VIEW_POPUP;
                    m_applController->processActivity(LOCAL_DEVICE_NAME, DEVICE_COMM, param);
                }
            }
        }
    }
    else if (index == INFO_CANCEL_BTN)
    {
        m_isStartLiveView =false;
    }

    if (IS_VALID_OBJ(m_startLiveViewInfoPage))
    {
        disconnect(m_startLiveViewInfoPage,
                   SIGNAL(sigInfoPageCnfgBtnClick(int)),
                   this,
                   SLOT(slotStartLiveViewInfoPageClick(int)));
        DELETE_OBJ(m_startLiveViewInfoPage);
    }
}

void MainWindow::slotPrevRecords(PlaybackRecordData* tempRecord, quint8 totalTempRecord, TEMP_PLAYBACK_REC_DATA_t* tempData)
{
    m_layout->clearsearchBtnClickResult();

    for (quint16 index = 0; index < MAX_REC_ALLOWED; index++)
    {
        m_prevRecordData[index].clearPlaybackInfo();
    }

    if (totalTempRecord == 0)
    {
        m_totalRecord = 0;
        return;
    }

    for (quint8 totalRecord = 0; totalRecord <totalTempRecord; totalRecord++)
    {
        m_prevRecordData[totalRecord].startTime = tempRecord[totalRecord].startTime;
        m_prevRecordData[totalRecord].endTime =  tempRecord[totalRecord].endTime;
        m_prevRecordData[totalRecord].camNo = tempRecord[totalRecord].camNo;
        m_prevRecordData[totalRecord].evtType = tempRecord[totalRecord].evtType;
        m_prevRecordData[totalRecord].overlap =  tempRecord[totalRecord].overlap;
        m_prevRecordData[totalRecord].hddIndicator =  tempRecord[totalRecord].hddIndicator;
        m_prevRecordData[totalRecord].partionIndicator = tempRecord[totalRecord].partionIndicator;
        m_prevRecordData[totalRecord].recDriveIndex = tempRecord[totalRecord].recDriveIndex;
        m_prevRecordData[totalRecord].deviceName = tempRecord[totalRecord].deviceName;
    }

    m_pbSearchPrevData.tempChannel = tempData->tempChannel;
    m_pbSearchPrevData.tempType = tempData->tempType;
    m_pbSearchPrevData.tempSource = tempData->tempSource;
    m_pbSearchPrevData.tempStartTime = tempData->tempStartTime;
    m_pbSearchPrevData.tempEndTime = tempData->tempEndTime;
    m_totalRecord = totalTempRecord;
}

void MainWindow::slotFindNextRecord(quint16 curRec, quint16 windIndx)
{
    quint16 nextRec = 0, nextToNext = 0;

    if (m_totalRecord == 0)
    {
        return;
    }

    for (nextRec = curRec + 1; nextRec < m_totalRecord; nextRec++)
    {
        if ((nextRec < MAX_REC_ALLOWED) && (curRec < MAX_REC_ALLOWED))
        {
            if (m_prevRecordData[curRec].camNo == m_prevRecordData[nextRec].camNo)
            {
                break;
            }
        }
    }

    if (nextRec >= m_totalRecord)
    {
        return;
    }

    for (nextToNext = nextRec + 1; nextToNext < m_totalRecord; nextToNext++)
    {
        if ((nextRec < MAX_REC_ALLOWED) && (nextToNext < MAX_REC_ALLOWED))
        {
            if (m_prevRecordData[nextRec].camNo == m_prevRecordData[nextToNext].camNo)
            {
                break;
            }
        }
    }

    if (nextToNext <= m_totalRecord)
    {
        if (nextRec < MAX_REC_ALLOWED)
        {
            m_prevRecordData[nextRec].asyncPbIndex = nextRec;
            m_layout->playNextRecord(m_prevRecordData[nextRec],windIndx,nextRec);
        }
    }
}

void MainWindow::slotFindPrevRecord(quint16 curRec, quint16 windIndx)
{
    int prevRec = 0;

    if ((curRec > 0) && (curRec < MAX_REC_ALLOWED))
    {
        for (prevRec = (curRec - 1); prevRec >= 0; prevRec--)
        {
            if (m_prevRecordData[curRec].camNo == m_prevRecordData[prevRec].camNo)
            {
                break;
            }
        }

        if ((prevRec >= 0) && (prevRec < MAX_REC_ALLOWED))
        {
            m_prevRecordData[prevRec].asyncPbIndex = prevRec;
            m_layout->playNextRecord(m_prevRecordData[prevRec], windIndx, prevRec);
        }
    }
}

void MainWindow::slotGetNextPrevRecord(quint16 curRec, quint16 windowIndex, quint16 camId)
{
    int nextRec = 0, prevRec = 0;
    bool prevFlag = false, nextFlag = false;

    if ((m_layout->m_searchBtnClickRecNo[windowIndex] >= MAX_REC_ALLOWED)  || (camId != m_prevRecordData[curRec].camNo))
    {
        m_layout->UpdateAsyncCenterBtn(prevFlag, nextFlag, windowIndex);
        return;
    }

    if (m_totalRecord != 0)
    {
        for (nextRec = curRec + 1; nextRec < m_totalRecord; nextRec++)
        {
            if (m_prevRecordData[curRec].camNo == m_prevRecordData[nextRec].camNo)
            {
                nextFlag = true;
                break;
            }
        }
    }

    if ((curRec > 0) && (m_totalRecord != 0))
    {
        for (prevRec = (curRec - 1); prevRec >= 0; prevRec--)
        {
            if (m_prevRecordData[curRec].camNo == m_prevRecordData[prevRec].camNo)
            {
                prevFlag = true;
                break;
            }
        }
    }

    m_layout->UpdateAsyncCenterBtn(prevFlag, nextFlag, windowIndex);
}

void MainWindow::slotUpdateUIGeometry(bool isRedraw)
{
    if (IS_VALID_OBJ(m_toolbar))
    {
        m_toolbar->updateGeometry();
    }

    if (IS_VALID_OBJ(m_messageAlert))
    {
        QMap<QString, quint8> msgMap;
        QStringList deviceList;

        MSG_ALERT_MODE_e alertMode = m_messageAlert->getMessageAlertMode();
        m_messageAlert->getMessageList(msgMap, deviceList);

        disconnect(m_messageAlert ,
                   SIGNAL(sigCloseAlert()),
                   this,
                   SLOT(slotMessageAlertClose()));
        DELETE_OBJ(m_messageAlert);

        m_messageAlert = new MessageAlert(this);
        connect(m_messageAlert ,
                SIGNAL(sigCloseAlert()),
                this,
                SLOT(slotMessageAlertClose()));
        m_messageAlert->setMessageList(msgMap, deviceList);
        m_messageAlert->changeDisplayMode(alertMode);
    }

    if ((isRedraw))
    {
        if (IS_VALID_OBJ(m_infoPage))
        {
            m_infoPage->updateGeometry();
        }

        if (IS_VALID_OBJ(m_messageBanner))
        {
            m_messageBanner->updateGeometry();
        }

        if (IS_VALID_OBJ(m_popUpAlert) && IS_VALID_OBJ(m_toolbar))
        {
            m_popUpAlert->updateGeometry((ApplController::getXPosOfScreen() + ApplController::getWidthOfScreen()),
                                         (ApplController::getYPosOfScreen() + ApplController::getHeightOfScreen() - m_toolbar->height()));
        }

        if (IS_VALID_OBJ(m_displaySetting))
        {
            slotCloseToolbarPage(LIVE_VIEW_BUTTON);
            slotOpenToolbarPage(LIVE_VIEW_BUTTON);
        }
    }

    update();
}

void MainWindow::slotChangeAudButtonState()
{
    emit sigChangeToolbarButtonState(AUDIO_CONTROL_BUTTON, STATE_1); //Audio Off
}

void MainWindow::slotAdvanceOption(QUICK_BACKUP_ELEMENTS_e index)
{
    if (m_toolbar == NULL)
    {
        return;
    }

    m_quickBackupActions = index;
    if ((m_toolbar->getCurrentToolBarButton() != MAX_TOOLBAR_BUTTON) && (m_toolbar->getCurrentToolBarButton() != SETTINGS_BUTTON))
    {
        m_toolbar->closeToolbarPage (m_toolbar->getCurrentToolBarButton());
        m_toolbar->openToolbarPage(SETTINGS_BUTTON);
    }
    else
    {
        m_toolbar->openToolbarPage (SETTINGS_BUTTON);
    }
}

void MainWindow::slotQuitSetupWiz()
{
    if (IS_VALID_OBJ(m_setupWizard))
    {
        disconnect(m_setupWizard ,
                   SIGNAL(sigQuitSetupWiz()),
                   this,
                   SLOT(slotQuitSetupWiz()));
        DELETE_OBJ(m_setupWizard);
    }

    DELETE_OBJ(m_inVisibleWidget);

    emit sigClosePage(WIZARD_BUTTON);
}

void MainWindow::slotOpenWizard()
{
    if (IS_VALID_OBJ(m_setupWizard))
    {
        return;
    }

    QList<QVariant> paramList;
    paramList.insert(SUB_ACTIVITY_TYPE, READ_WIZARD_PARAM);

    WIZ_OPEN_CONFIG_t wizardOpenStatus;
    memset(&wizardOpenStatus, 0, sizeof(wizardOpenStatus));

    if (m_applController->processActivity(OTHER_LOGIN_ACTIVITY, paramList, &wizardOpenStatus) == true)
    {
        /* We have to open setup wizard after getting user language response if open wizard flag is true */
        m_setupWizardPendingF = wizardOpenStatus.wizardOpen;
    }
}

void MainWindow::getUserPreferredLanguage(void)
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = GET_USER_LANGUAGE;
    m_applController->processActivity(LOCAL_DEVICE_NAME, DEVICE_COMM, param);
}
