#include "SyncPlayback.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPaintEvent>
#include "Layout/Layout.h"
#include "Controls/MessageBanner.h"
#include "ValidationMessage.h"
#include "../DecoderLib/include/DecDispLib.h"

#define RIGHT_PANEL_LEFT_MARGIN         SCALE_WIDTH(1610)
#define CONTROL_WIDTH                   SCALE_WIDTH(300)
#define CALENDER_HEIGHT                 SCALE_HEIGHT(190)
#define OTHER_ELEMENT_HEIGHT            SCALE_HEIGHT(30)
#define SEARCH_BUTTON_Y_OFFSET          SCALE_HEIGHT(26)
#define SEARCH_BUTTON_X_OFFSET          SCALE_HEIGHT(155)
#define TOOLBAR_NORMAL_MODE_HEIGHT      SCALE_HEIGHT(38)
#define TOOLBAR_FULL_MODE_HEIGHT        SCALE_HEIGHT(60)
#define BOTTOM_RECT_WIDTH               SCALE_WIDTH(1600)
#define SPACE_BET_TWO_TYPE_CHECKBOX     SCALE_WIDTH(135)
#define HOURS_HEADING_START_X           SCALE_WIDTH(1320)
#define CAMERA_CHECKBOX_HEIGHT          SCALE_HEIGHT(32)
#define TYPE_INDICATION_SPACE           SCALE_WIDTH(30)
#define HOURS_HEADING_WIDTH             SCALE_WIDTH(60)
#define DOTS_SEPARATION                 SCALE_WIDTH(15)
#define MAX_CLIP                        64
#define SCROLLBAR_WIDTH                 SCALE_WIDTH(13)
#define MAX_SEARCH_ATTEMPTS             2
#define RECORDS_IN_MIN                  (1440)  /* 1 Day = 24 Hour * 60 Min */
#define MONTH_SEARCH_EVENT_MASK         0x0F    /* Manual, Alarm, Schedule and Cosec */

const QString hourTypeString[] = {"24", "12", "6", "1"};

const QString recordingTypeString[MAX_RECORDING_TYPE] = {"Alarm", "Manual", "Schedule", "COSEC"};

const QString recordingColor[MAX_RECORDING_TYPE] = {"#990303", "#0c822d", "#c39405",  "#528dc9"};

static const QStringList recDriveListSync = QStringList()
            << "All" << "Local Drive" <<  "Network Drive 1" << "Network Drive 2";

/* Back-end uses bits 0-3 for recording types:
 * MSB->LSB: bit3: Cosec, bit2: Schedule, bit1: Alarm, bit0: Manual
 */
const quint8 recordingValue[MAX_RECORDING_TYPE] = {2, 1, 4, 8};

/* DeviceClient thread uses bits 1-4 for recording types:
 * MSB->LSB: bit4: Alarm, bit3: Cosec, bit2: Manual, bit1: Schedule, Bit0: xx
 */
const quint8 recTypeMaplist[MAX_RECORDING_TYPE] = {16, 4, 2, 8};

LAYOUT_TYPE_e SyncPlayback::m_currentLayout = MAX_LAYOUT;

SyncPlayback::SyncPlayback(quint16 layoutWidth, quint16 layoutHeight, bool isPreProcessingDone, QWidget* parent)
    : KeyBoard(parent),
	 m_currentMode(NORMAL_MODE),
     m_modeBeforeZoomAction(MAX_SYNCPB_TOOLBAR_MODE_TYPE),
	 m_currentHourFormat(HOUR_24),
     m_currentPlaybackState(SYNC_PLAYBACK_NONE_STATE),
     m_previousPlaybackState(SYNC_PLAYBACK_NONE_STATE),
     m_playBackSpeed(PB_SPEED_NORMAL), m_playBackDirection(MAX_PLAY_DIRECTION),
     m_moveCount(0), m_windowIndexForAudio(MAX_CHANNEL_FOR_SEQ),
     m_windowIndexForAudioOld(MAX_CHANNEL_FOR_SEQ), m_currentWindowIndex(0),
     m_syncPlaybackStreamId(MAX_STREAM_SESSION), m_firstCameraIndex(0), m_lastCameraIndex(0),
     m_searchAttemptsForDate(0), m_searchAttemptsForHours(0),
     m_audCamIndexAfterClntAud(MAX_CHANNEL_FOR_SEQ)
{
    memset(&m_cameraValueForSearch, 0, sizeof(m_cameraValueForSearch));
    memset(&m_audioValueForPlayRecord, 0, sizeof(m_audioValueForPlayRecord));
    memset(&m_cameraValueForPlayRecord, 0, sizeof(m_cameraValueForPlayRecord));
    m_inVisibleWidget = NULL;
    m_lastFrameSecond = 0;
    m_recordValueForSearch = 0;
    m_isGetDateTimeRequestPending = m_isSearchDateRequestPending = m_isSearchHoursRequestPending = false;
    m_syncScrollbar = NULL;
    m_isPreProcessingIsDone = isPreProcessingDone;
    m_isFirstTimePlay = true;
	m_currentLayout = ONE_X_ONE_PLAYBACK;
    m_isSliderPositionChangedbyUser = false;
    m_isRequestPendingForSliderPositionChanged = false;
    m_isAudioCommandSend = false;
    m_syncPlaybackCropAndBackup = NULL;
    m_currentZoomState = SYNC_PLAYBACK_ZOOM_OUT;
    m_isSyncRequestPending = false;
    m_isSearchDateRequired = true;
    m_zoomFeatureControl = NULL;
    m_isQuickBackupon = false;
    m_isClientAudioProcessing = false;
    m_cameraListCount = 0;
    m_selectedCamCnt = 0;
    m_prevSelectedWindowIndex = MAX_CHANNEL_FOR_SEQ;
    m_isCloseButtonClicked = false;

    for(quint8 index = 0; index < MAX_SYNC_PLAYBACK_ELEMENT; index++)
    {
        m_elementList[index] = NULL;
    }

    this->setGeometry(ApplController::getXPosOfScreen(),
                      ApplController::getYPosOfScreen(),
                      ApplController::getWidthOfScreen(),
                      ApplController::getHeightOfScreen());

    m_payloadLib = new PayloadLib();
    m_applController = ApplController::getInstance();

    QMap<quint8, QString> deviceMapList;
    m_applController->GetDevNameDropdownMapList(deviceMapList);
    m_currentDeviceName = m_applController->GetRealDeviceName(deviceMapList.value(0));
    m_isDeviceConnected = m_applController->GetAllCameraNamesOfDevice(m_currentDeviceName, m_cameraNameList, PLAYBACK_CAM_LIST_TYPE, &m_cameraIndex[0]);

    m_mainRightRectangle = new Rectangle(layoutWidth,
                                         0,
                                         (ApplController::getWidthOfScreen() - (layoutWidth)),
                                         ApplController::getHeightOfScreen(),
                                         NORMAL_BKG_COLOR,
                                         this);

    m_mainBottomRectangle = new Rectangle(0,
                                          layoutHeight,
                                          (BOTTOM_RECT_WIDTH + 1),
                                          (ApplController::getHeightOfScreen() - layoutHeight),
                                          NORMAL_BKG_COLOR,
                                          this);

    m_closeButton = new CloseButtton((this->width() - SCALE_WIDTH(18)),
                                     SCALE_HEIGHT(18),
                                     this,
                                     CLOSE_BTN_TYPE_1,
                                     SYNC_PLAYBACK_CLOSEBUTTON);
    m_elementList[SYNC_PLAYBACK_CLOSEBUTTON] = m_closeButton;
    connect(m_closeButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotCloseButtonClicked(int)));
    connect(m_closeButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_toolbar = new SyncPlaybackToolbar(m_mainBottomRectangle->x(),
                                        m_mainBottomRectangle->y(),
                                        BOTTOM_RECT_WIDTH,
                                        TOOLBAR_NORMAL_MODE_HEIGHT,
                                        SYNC_PLAYBACK_TOOLBAR,
                                        this);
    m_elementList[SYNC_PLAYBACK_TOOLBAR] = m_toolbar;
    connect(m_toolbar,
            SIGNAL(sigFocusToOtherElement(bool)),
            this,
            SLOT(slotFocusChangedFromCurrentElement(bool)));
    connect(m_toolbar,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_toolbar,
            SIGNAL(sigToolbarButtonClicked(SYNCPB_TOOLBAR_BUTTON_TYPE_e,STATE_TYPE_e)),
            this,
            SLOT(slotToolbarButtonClicked(SYNCPB_TOOLBAR_BUTTON_TYPE_e,STATE_TYPE_e)));
    connect(m_toolbar,
            SIGNAL(sigChangeLayout(LAYOUT_TYPE_e)),
            this,
            SLOT(slotchangeLayout(LAYOUT_TYPE_e)));

    //time line width is inremented to 'SCALE_WIDHT(10)' for sync playback timeline-tooltip underlap issue
    m_timeLine = new SyncPlaybackTimeLine(m_mainBottomRectangle->x(),
                                          (m_toolbar->y() + m_toolbar->height()),
                                          BOTTOM_RECT_WIDTH + SCALE_WIDTH(10),
                                          (m_mainBottomRectangle->height() - m_toolbar->height()),
                                          SYNC_PLAYBACK_TIMELINE,
                                          this);
    m_elementList[SYNC_PLAYBACK_TIMELINE] = m_timeLine;
    connect(m_timeLine,
            SIGNAL(sigFocusToOtherElement(bool)),
            this,
            SLOT(slotFocusChangedFromCurrentElement(bool)));
    connect(m_timeLine,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_timeLine,
            SIGNAL(sigSliderPositionChanged()),
            this,
            SLOT(slotSliderPositionChanged()));
    connect(m_timeLine,
            SIGNAL(sigSliderPositionChangedStart()),
            this,
            SLOT(slotSliderPositionChangedStart()));

    m_deviceNameDropDownBox = new DropDown(RIGHT_PANEL_LEFT_MARGIN,
                                           (m_closeButton->y() + m_closeButton->height() + SCALE_HEIGHT(5)),
                                           CONTROL_WIDTH, BGTILE_HEIGHT,
                                           SYNC_PLAYBACK_DEVICENAME_SPINBOX,
                                           DROPDOWNBOX_SIZE_200,
                                           "Device",
                                           deviceMapList,
                                           this,
                                           "",
                                           false,
                                           SCALE_WIDTH(10),
                                           NO_LAYER,true,8,true);
    m_elementList[SYNC_PLAYBACK_DEVICENAME_SPINBOX] = m_deviceNameDropDownBox;
    connect(m_deviceNameDropDownBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_deviceNameDropDownBox,
            SIGNAL(sigValueChanged(QString,quint32)),
            this,
            SLOT(slotDeviceNameChanged(QString,quint32)));

    QMap<quint8, QString>  recDriveListMap;
    for(quint8 index = 0; index < recDriveListSync.length() ; index++)
    {
        recDriveListMap.insert (index,recDriveListSync.at (index));
    }

    m_recDriveListDropBox = new DropDown(RIGHT_PANEL_LEFT_MARGIN,
                                         (m_deviceNameDropDownBox->y() + m_deviceNameDropDownBox->height()),
                                         CONTROL_WIDTH, BGTILE_HEIGHT,
                                         SYNC_PLAYBACK_REC_DRIVE_DROPBOX,
                                         DROPDOWNBOX_SIZE_200,
                                         "Source",
                                         recDriveListMap,
                                         this,
                                         "",
                                         false,
                                         SCALE_WIDTH(10),
                                         NO_LAYER,true,8,true);
    m_elementList[SYNC_PLAYBACK_REC_DRIVE_DROPBOX] = m_recDriveListDropBox;
    connect(m_recDriveListDropBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_recDriveListDropBox,
            SIGNAL(sigValueChanged(QString,quint32)),
            this,
            SLOT(slotRecDriveDropBoxChanged(QString,quint32)));

    m_calender = new SyncPlaybackCalender(RIGHT_PANEL_LEFT_MARGIN,
                                          m_recDriveListDropBox->y() + m_recDriveListDropBox->height() + SCALE_HEIGHT(10),
                                          CONTROL_WIDTH,
                                          CALENDER_HEIGHT,
                                          SYNC_PLAYBACK_CALENDER,
                                          this);
    m_elementList[SYNC_PLAYBACK_CALENDER] = m_calender;
    connect(m_calender,
            SIGNAL(sigFocusToOtherElement(bool)),
            this,
            SLOT(slotFocusChangedFromCurrentElement(bool)));
    connect(m_calender,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_calender,
            SIGNAL(sigFetchNewSelectedDate()),
            this,
            SLOT(slotFetchNewSelectedDate()));
    connect(m_calender,
            SIGNAL(sigFetchRecordForNewDate()),
            this,
            SLOT(slotFetchRecordForNewDate()));

    m_typesHeading = new ElementHeading(RIGHT_PANEL_LEFT_MARGIN ,
                                        m_calender->y() + m_calender->height() + SCALE_HEIGHT(5),
                                        CONTROL_WIDTH,
                                        BGTILE_HEIGHT,
                                        "Type",
                                        NO_LAYER,
                                        this,
                                        false,
                                        SCALE_WIDTH(10), NORMAL_FONT_SIZE, true,NORMAL_FONT_COLOR,true);

    for(quint8 index = 0; index < MAX_RECORDING_TYPE; index++)
    {
        m_recordingTypeCheckbox[index] = new OptionSelectButton((m_typesHeading->x() + SCALE_WIDTH(5) + ((index/2) * SPACE_BET_TWO_TYPE_CHECKBOX)),
                                                                (m_typesHeading->y() + m_typesHeading->height() + ((index % 2) * OTHER_ELEMENT_HEIGHT)),
                                                                (m_typesHeading->width() / 2),
                                                                OTHER_ELEMENT_HEIGHT,
                                                                CHECK_BUTTON_INDEX,
                                                                this,
                                                                NO_LAYER,
                                                                "",
                                                                "",
                                                                SCALE_WIDTH(10),
                                                                (SYNC_PLAYBACK_RECORDINGTYPE_CHECKBOX + index));
         m_recordingTypeCheckbox[index]->changeState(ON_STATE);
         m_elementList[(SYNC_PLAYBACK_RECORDINGTYPE_CHECKBOX + index)] = m_recordingTypeCheckbox[index];
         connect(m_recordingTypeCheckbox[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
         connect(m_recordingTypeCheckbox[index],
                 SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                 this,
				 SLOT(slotRecTypeCheckboxClicked(OPTION_STATE_TYPE_e,int)));

        m_recordingInitialLabel[index] = new TextLabel((m_recordingTypeCheckbox[index]->x() + SCALE_WIDTH(35)),
                                                       (m_recordingTypeCheckbox[index]->y() + (m_recordingTypeCheckbox[index]->height() / 2)),
                                                       NORMAL_FONT_SIZE,
                                                       recordingTypeString[index].at(0),
                                                       this,
                                                       recordingColor[index],
                                                       NORMAL_FONT_FAMILY,
                                                       ALIGN_START_X_CENTRE_Y);

        m_recordingLabel[index] = new TextLabel((m_recordingInitialLabel[index]->x() + m_recordingInitialLabel[index]->width()),
                                                (m_recordingTypeCheckbox[index]->y() + (m_recordingTypeCheckbox[index]->height() / 2)),
                                                NORMAL_FONT_SIZE,
                                                recordingTypeString[index].right(recordingTypeString[index].length() - 1),
                                                this,
                                                NORMAL_FONT_COLOR,
                                                NORMAL_FONT_FAMILY,
                                                ALIGN_START_X_CENTRE_Y);
    }

    #if defined(OEM_JCI)
    m_recordingTypeCheckbox[COSEC_RECORDING]->changeState(OFF_STATE);
    m_recordingTypeCheckbox[COSEC_RECORDING]->setVisible(false);
    m_recordingTypeCheckbox[COSEC_RECORDING]->setIsEnabled(false);
    m_recordingInitialLabel[COSEC_RECORDING]->setVisible(false);
    m_recordingLabel[COSEC_RECORDING]->setVisible(false);
    #endif

    m_searchButton = new CnfgButton(CNFGBUTTON_EXTRALARGE,
                                    RIGHT_PANEL_LEFT_MARGIN + SEARCH_BUTTON_X_OFFSET,
                                    (m_typesHeading->y() + m_typesHeading->height() + (OTHER_ELEMENT_HEIGHT * 2) + SEARCH_BUTTON_Y_OFFSET),
                                    "Search",
                                    this,
                                    SYNC_PLAYBACK_SEARCH_CONTROLBUTTON);
    m_elementList[SYNC_PLAYBACK_SEARCH_CONTROLBUTTON] = m_searchButton;
    connect(m_searchButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_searchButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotSearchButtonClicked(int)));

    m_camerasHeading = new ElementHeading(RIGHT_PANEL_LEFT_MARGIN,
                                          (m_searchButton->y() + m_searchButton->height() - SCALE_HEIGHT(5)),
                                          CONTROL_WIDTH,
                                          BGTILE_HEIGHT,
                                          "Cameras",
                                          NO_LAYER,
                                          this,
                                          false,
                                          SCALE_WIDTH(10), NORMAL_FONT_SIZE, true,NORMAL_FONT_COLOR,true);
    m_camerasHeading->setVisible(false);

    for(quint8 index=0; index < MAX_CAM_LIST_CNT; index++)
    {
        quint8 tileWidth = (index == 0 ? CONTROL_WIDTH: CONTROL_WIDTH - SCROLLBAR_WIDTH);
        QString labelString = (index == 0 ? "Select Maximum" : " ");
        m_cameraCheckbox[index] = new OptionSelectButton(RIGHT_PANEL_LEFT_MARGIN + DOTS_SEPARATION + SCALE_WIDTH(5),
                                                         ((m_camerasHeading->y() + m_camerasHeading->height()) + (index * CAMERA_CHECKBOX_HEIGHT)),
                                                         (tileWidth  - TYPE_INDICATION_SPACE),
                                                         CAMERA_CHECKBOX_HEIGHT,
                                                         CHECK_BUTTON_INDEX,
                                                         labelString,
                                                         this,
                                                         NO_LAYER,
                                                         SCALE_WIDTH(10),
                                                         MX_OPTION_TEXT_TYPE_SUFFIX,
                                                         NORMAL_FONT_SIZE,
                                                         (SYNC_PLAYBACK_CAMERASELECT_CHECKBOX + index),
                                                         true,
                                                         NORMAL_FONT_COLOR, true);
        m_elementList[(SYNC_PLAYBACK_CAMERASELECT_CHECKBOX + index)] = m_cameraCheckbox[index];
        connect(m_cameraCheckbox[index],
                        SIGNAL(sigUpdateCurrentElement(int)),
                        this,
                        SLOT(slotUpdateCurrentElement(int)));
        connect(m_cameraCheckbox[index],
                        SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                        this,
                        SLOT(slotCameraCheckboxClicked(OPTION_STATE_TYPE_e,int)));
        m_cameraCheckbox[index]->setVisible(false);

        if(index < MAX_CAM_LIST_SEPARATOR_CNT)
        {
           m_cameraListSeparator[index] = new Rectangle(RIGHT_PANEL_LEFT_MARGIN,
                                                        (m_cameraCheckbox[index]->y() + m_cameraCheckbox[index]->height()),
                                                        CONTROL_WIDTH,
                                                        1,
                                                        BORDER_1_COLOR,
                                                        this,0,0,
                                                        BORDER_1_COLOR,0.5);
           m_cameraListSeparator[index]->setVisible(false);
        }

        /* For Select Maximum, Recording incation is not required */
        if (0 == index)
        {
            continue;
        }

        for(quint8 loop = 0; loop < MAX_RECORDING_TYPE; loop++)
        {
            m_cameraRecType[index-1][loop] = new TextLabel(RIGHT_PANEL_LEFT_MARGIN + ((loop / 2) * DOTS_SEPARATION) + SCALE_WIDTH(5),
                                                           m_cameraCheckbox[index]->y() + ((loop % 2) * DOTS_SEPARATION) + SCALE_HEIGHT(18),
                                                           SCALE_FONT(LARGE_HEADING_FONT_SIZE - 5),
                                                           "",
                                                           this,
                                                           recordingColor[loop],
                                                           NORMAL_FONT_FAMILY,
                                                           ALIGN_END_X_END_Y);
            m_cameraRecType[index-1][loop]->setVisible(false);
        }
    }

	m_errorMsgTxt = new TextLabel(m_camerasHeading->x(),
                                  m_camerasHeading->y() + m_camerasHeading->height() + SCALE_HEIGHT(5),
                                  NORMAL_FONT_SIZE,
                                  "",
                                  this,
                                  NORMAL_FONT_COLOR,
                                  NORMAL_FONT_FAMILY,
                                  ALIGN_START_X_CENTRE_Y);
	m_errorMsgTxt->setVisible(false);

    m_hoursHeading = new ElementHeading(m_toolbar->x() + HOURS_HEADING_START_X,
                                        m_toolbar->y(),
                                        HOURS_HEADING_WIDTH,
                                        m_toolbar->height(),
                                        "Hours",
                                        NO_LAYER,
                                        this,
                                        false,
                                        SCALE_WIDTH(10), NORMAL_FONT_SIZE, true,NORMAL_FONT_COLOR);

    for(quint8 index = 0; index < MAX_HOUR_FORMAT_TYPE; index++)
    {
        m_hoursRadioButtion[index] = new OptionSelectButton(m_hoursHeading->x() + ((index+1) * m_hoursHeading->width()),
                                                            m_hoursHeading->y(),
                                                            m_hoursHeading->width(),
                                                            m_hoursHeading->height(),
                                                            RADIO_BUTTON_INDEX,
                                                            hourTypeString[index],
                                                            this,
                                                            NO_LAYER,
                                                            SCALE_WIDTH(10),
                                                            MX_OPTION_TEXT_TYPE_SUFFIX,
                                                            NORMAL_FONT_SIZE,
                                                            (SYNC_PLAYBACK_HOURTYPE_RADIOBUTTON + index));
        m_elementList[(SYNC_PLAYBACK_HOURTYPE_RADIOBUTTON + index)] = m_hoursRadioButtion[index];
        connect(m_hoursRadioButtion[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        connect(m_hoursRadioButtion[index],
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotHourFormatChanged(OPTION_STATE_TYPE_e,int)));

        /*Disable hours button untill recording not found*/
        m_hoursRadioButtion[index]->setIsEnabled(false);
    }

    m_hoursRadioButtion[HOUR_24]->changeState(ON_STATE);

    m_syncPlaybackLoadingText = new SyncPlayBackLoadingText(layoutWidth, layoutHeight, this);
    m_retryTimer = new QTimer(this);
    connect (m_retryTimer,
             SIGNAL(timeout()),
             this,
             SLOT(slotRetryTimeOut()));
    m_retryTimer->setInterval(200);

    if((!m_isPreProcessingIsDone) && (!m_cameraNameList.isEmpty()))
    {
        m_syncPlaybackLoadingText->setVisible(true);
    }

    m_currentElement = SYNC_PLAYBACK_DEVICENAME_SPINBOX;
    m_elementList[m_currentElement]->forceActiveFocus();
    setLayoutWindowArea();

    /* Get date and time from server */
    getDateAndTime();

    this->setEnabled(true);
    this->setMouseTracking(true);
    this->show();

    //defining previous layout for double click logic considering no layout change to take place if swiching from 1x1_pb to 1x1_pb again
    m_prevLayout = ONE_X_ONE_PLAYBACK;
    Q_UNUSED(layoutHeight);
}

SyncPlayback::~SyncPlayback()
{
    if(m_retryTimer->isActive())
    {
        m_retryTimer->stop();
    }
    disconnect (m_retryTimer,
                SIGNAL(timeout()),
                this,
                SLOT(slotRetryTimeOut()));
    DELETE_OBJ (m_retryTimer);

    DELETE_OBJ (m_syncPlaybackLoadingText);
    for(quint8 index = 0; index < MAX_HOUR_FORMAT_TYPE; index++)
    {
        disconnect(m_hoursRadioButtion[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_hoursRadioButtion[index],
                   SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                   this,
                   SLOT(slotHourFormatChanged(OPTION_STATE_TYPE_e,int)));
        DELETE_OBJ (m_hoursRadioButtion[index]);
    }

    DELETE_OBJ (m_hoursHeading);
    disconnect(m_searchButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_searchButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotSearchButtonClicked(int)));
    DELETE_OBJ (m_searchButton);

    DeleteCameraList();
    disconnect(m_deviceNameDropDownBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_deviceNameDropDownBox,
               SIGNAL(sigValueChanged(QString,quint32)),
               this,
               SLOT(slotDeviceNameChanged(QString,quint32)));
    DELETE_OBJ (m_deviceNameDropDownBox);

    disconnect(m_calender,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_calender,
               SIGNAL(sigFocusToOtherElement(bool)),
               this,
               SLOT(slotFocusChangedFromCurrentElement(bool)));
    disconnect(m_calender,
               SIGNAL(sigFetchNewSelectedDate()),
               this,
               SLOT(slotFetchNewSelectedDate()));
    disconnect(m_calender,
               SIGNAL(sigFetchRecordForNewDate()),
               this,
               SLOT(slotFetchRecordForNewDate()));

    DELETE_OBJ (m_calender);
    for(quint8 index = 0; index < MAX_RECORDING_TYPE; index++)
    {
        DELETE_OBJ (m_recordingInitialLabel[index]);
        DELETE_OBJ (m_recordingLabel[index]);

        disconnect(m_recordingTypeCheckbox[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_recordingTypeCheckbox[index],
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotRecTypeCheckboxClicked(OPTION_STATE_TYPE_e,int)));
        DELETE_OBJ (m_recordingTypeCheckbox[index]);
    }

    DELETE_OBJ (m_typesHeading);
    disconnect(m_timeLine,
               SIGNAL(sigFocusToOtherElement(bool)),
               this,
               SLOT(slotFocusChangedFromCurrentElement(bool)));
    disconnect(m_timeLine,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_timeLine,
               SIGNAL(sigSliderPositionChanged()),
               this,
               SLOT(slotSliderPositionChanged()));
    disconnect(m_timeLine,
               SIGNAL(sigSliderPositionChangedStart()),
               this,
               SLOT(slotSliderPositionChangedStart()));
    DELETE_OBJ (m_timeLine);

    disconnect(m_toolbar,
               SIGNAL(sigFocusToOtherElement(bool)),
               this,
               SLOT(slotFocusChangedFromCurrentElement(bool)));
    disconnect(m_toolbar,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_toolbar,
               SIGNAL(sigToolbarButtonClicked(SYNCPB_TOOLBAR_BUTTON_TYPE_e,STATE_TYPE_e)),
               this,
               SLOT(slotToolbarButtonClicked(SYNCPB_TOOLBAR_BUTTON_TYPE_e,STATE_TYPE_e)));
    disconnect(m_toolbar,
                SIGNAL(sigChangeLayout(LAYOUT_TYPE_e)),
                this,
                SLOT(slotchangeLayout(LAYOUT_TYPE_e)));
    DELETE_OBJ (m_toolbar);

    disconnect(m_closeButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotCloseButtonClicked(int)));
    disconnect(m_closeButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    DELETE_OBJ (m_closeButton);

    DELETE_OBJ (m_mainRightRectangle);
    DELETE_OBJ (m_mainBottomRectangle);
    DELETE_OBJ (m_payloadLib);

    disconnect(m_recDriveListDropBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));

    disconnect(m_recDriveListDropBox,
               SIGNAL(sigValueChanged(QString,quint32)),
               this,
               SLOT(slotRecDriveDropBoxChanged(QString,quint32)));
    DELETE_OBJ (m_recDriveListDropBox);

    m_cameraNameList.clear();
}

bool SyncPlayback::stopSyncBackupManual()
{
    bool status;

    if((m_currentPlaybackState == SYNC_PLAYBACK_PLAY_STATE) || (m_currentPlaybackState == SYNC_PLAYBACK_PAUSE_STATE) ||
       (m_currentPlaybackState == SYNC_PLAYBACK_REVERSEPLAY_STATE) || (m_currentPlaybackState == SYNC_PLAYBACK_STEP_STATE))
    {
        status = true;
    }
    else
    {
        status = false;
    }

    if(m_syncPlaybackCropAndBackup != NULL)
    {
        m_syncPlaybackCropAndBackup->stopBackupProcess();
        m_syncPlaybackCropAndBackup->closeSyncBackupManual();
    }
    return status;
}

void SyncPlayback::ResponceOnDeviceDisconnected()
{
    // for network device which disconnected
    bool isControlActive = (!m_cameraNameList.empty());

	/* Enable/Disable Search filters on seach parameters change */
	m_recDriveListDropBox->setIsEnabled(isControlActive);
	m_calender->setIsEnabled(isControlActive);
	m_searchButton->setIsEnabled(isControlActive);
	for(quint8 index = 0; index < MAX_RECORDING_TYPE; index++)
	{
		m_recordingTypeCheckbox[index]->setIsEnabled (isControlActive);
	}

	if(m_isDeviceConnected == false)
	{
		MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(CMD_DEV_DISCONNECTED));
	}
    else if((m_cameraNameList.isEmpty()) && ((m_applController->getUserRightsResponseNotify())))
	{
        MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(CMD_NO_PRIVILEGE));
	}
}

void SyncPlayback::updateOnNewDeviceConnected()
{
    if (m_currentDeviceName == LOCAL_DEVICE_NAME)
    {
        m_isDeviceConnected = m_applController->GetAllCameraNamesOfDevice(LOCAL_DEVICE_NAME,
                                                                          m_cameraNameList,
                                                                          PLAYBACK_CAM_LIST_TYPE,
                                                                          &m_cameraIndex[0]);
        ResponceOnDeviceDisconnected();
    }
}

void SyncPlayback::resetGeometryForCameraOption()
{
    for(quint8 index = 1; index < MAX_CAM_LIST_CNT; index++)
    {
        if((index > m_firstCameraIndex) && (index <= m_lastCameraIndex) && (m_cameraListCount != 0))
        {
            m_cameraCheckbox[(index)]->resetGeometry(RIGHT_PANEL_LEFT_MARGIN + DOTS_SEPARATION + SCALE_WIDTH(5),
                                                     (m_camerasHeading->y() + m_camerasHeading->height()) + ((index - m_firstCameraIndex) * CAMERA_CHECKBOX_HEIGHT),
                                                     (CONTROL_WIDTH - SCROLLBAR_WIDTH), CAMERA_CHECKBOX_HEIGHT);
            m_cameraCheckbox[(index)]->setVisible(true);
			for(quint8 loop=0; loop < MAX_RECORDING_TYPE; loop++)
			{
                m_cameraRecType[index-1][loop]->resetGeometry(RIGHT_PANEL_LEFT_MARGIN + ((loop / 2) * DOTS_SEPARATION) + SCALE_WIDTH(5),
                                                              m_cameraCheckbox[index]->y() + ((loop % 2) * DOTS_SEPARATION) + SCALE_HEIGHT(18));
                m_cameraRecType[index-1][loop]->setVisible(true);
			}
		}
        else
        {
            m_cameraCheckbox[(index)]->setVisible(false);
            for(quint8 loop=0; loop < MAX_RECORDING_TYPE; loop++)
            {
                m_cameraRecType[index-1][loop]->setVisible(false);
            }
		}
	}
    update();
}

void SyncPlayback::getDateAndTime()
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = GET_DATE_TIME;
    if(m_applController->processActivity(LOCAL_DEVICE_NAME, DEVICE_COMM, param))
    {
        if(!m_syncPlaybackLoadingText->isVisible())
        {
            m_syncPlaybackLoadingText->setVisible(true);
        }
        m_isGetDateTimeRequestPending = true;
    }
}

bool SyncPlayback::searchNewRecord()
{
    if (true == m_cameraNameList.isEmpty())
    {
        return false;
    }

    if(m_isDeviceConnected == false)
    {
        MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(CMD_DEV_DISCONNECTED));
        return false;
    }

    setCameraValueForSearch();
    setRecordValueForSearch();
    if(m_recordValueForSearch == 0)
    {
        //if no record type is selected
        MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SYNC_PB_SEL_MIN_EVENT));
        return false;
    }

    return true;
}

void SyncPlayback::searchDatesHavingRecord()
{
    if((m_isDeviceConnected == true) && (m_cameraNameList.isEmpty()))
    {
        MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(CMD_NO_PRIVILEGE));
        if (m_syncPlaybackLoadingText->isVisible())
        {
            m_syncPlaybackLoadingText->setVisible(false);
        }
        return;
    }

    if (false == searchNewRecord())
    {
        return;
    }

    quint8 fieldIdx = 0;
    quint8 recordIndex = m_recDriveListDropBox->getIndexofCurrElement();

    m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, m_calender->getMonthYearString());
    m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, MONTH_SEARCH_EVENT_MASK);
    m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, m_cameraValueForSearch.bitMask[0]);
    m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, (recordIndex == 0) ? 4 : (recordIndex - 1));
    m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, m_cameraValueForSearch.bitMask[1]);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = PLYBCK_SRCH_MNTH;
    param->payload = m_payloadLib->createDevCmdPayload(fieldIdx);

    DPRINT(GUI_SYNC_PB_MEDIA, "playback search by month: [cameraMask=0x%llx & 0x%llx], [recordType=0x%x]",
           m_cameraValueForSearch.bitMask[0], m_cameraValueForSearch.bitMask[1], MONTH_SEARCH_EVENT_MASK);
    if (false == m_applController->processActivity(m_currentDeviceName, DEVICE_COMM, param))
    {
        MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(CMD_DEV_DISCONNECTED));
        if (m_syncPlaybackLoadingText->isVisible())
        {
            m_syncPlaybackLoadingText->setVisible(false);
        }
        return;
    }

    if(!m_syncPlaybackLoadingText->isVisible())
    {
        m_syncPlaybackLoadingText->setVisible(true);
    }
    m_isSearchDateRequestPending = true;
    m_searchAttemptsForDate++;
}

void SyncPlayback::searchHoursHavingRecord()
{
    if ((true == m_cameraNameList.isEmpty()) || (false == searchNewRecord()))
    {
        if (m_syncPlaybackLoadingText->isVisible())
        {
            m_syncPlaybackLoadingText->setVisible(false);
        }
        return;
    }

    quint8 fieldIdx = 0;
    quint8 recordIndex = m_recDriveListDropBox->getIndexofCurrElement();

    m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, m_calender->getDateMonthYearString());
    m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, m_recordValueForSearch);
    m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, m_cameraValueForSearch.bitMask[0]);
    m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, (recordIndex == 0) ? 4 : (recordIndex - 1));
    m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, m_cameraValueForSearch.bitMask[1]);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = PLYBCK_SRCH_DAY;
    param->payload = m_payloadLib->createDevCmdPayload(fieldIdx);

    DPRINT(GUI_SYNC_PB_MEDIA, "playback search by day: [cameraMask=0x%llx & 0x%llx], [recordType=0x%x]",
           m_cameraValueForSearch.bitMask[0], m_cameraValueForSearch.bitMask[1], m_recordValueForSearch);
    if(false == m_applController->processActivity(m_currentDeviceName, DEVICE_COMM, param))
    {
        MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(CMD_DEV_DISCONNECTED));
        return;
    }

    if(!m_syncPlaybackLoadingText->isVisible())
    {
        m_syncPlaybackLoadingText->setVisible(true);
    }
    m_isSearchHoursRequestPending = true;
    m_searchAttemptsForHours++;
}

void SyncPlayback::setCameraValueForSearch()
{
    quint16 cameraIndex;
    memset(&m_cameraValueForSearch, 0, sizeof(m_cameraValueForSearch));
    for(quint8 index = 0; index < m_cameraNameList.length(); index++)
	{
        cameraIndex = m_cameraIndex[index] - 1;
        SET_CAMERA_MASK_BIT(m_cameraValueForSearch, cameraIndex);
    }
}

void SyncPlayback::setRecordValueForSearch()
{
	m_recordValueForSearch = 0;
	for(quint8 index = 0; index< MAX_RECORDING_TYPE; index++)
	{
		if(m_recordingTypeCheckbox[index]->getCurrentState() == ON_STATE)
		{
			m_recordValueForSearch += recordingValue[index];
		}
	}
}

void SyncPlayback::selectDeselectAllRecords(OPTION_STATE_TYPE_e state)
{
    /* When Select-Max pressed if previously any cameras for search request present it must be remove and when Deselect-Max pressed it must remove all cameras from search request */
    memset(&m_cameraValueForPlayRecord, 0, sizeof(m_cameraValueForPlayRecord));
    m_selectedCamCnt = 0;
	m_cameraCheckbox[0]->changeState(state);
    for(quint8 index = 0; index < m_cameraListCount; index++)
	{
		if(index >= MAX_SYNC_PB_SESSION)
		{
			/* Disable CamCheckBox */
			m_cameraCheckbox[index + 1]->changeState(OFF_STATE);
			continue;
		}

		/* Update CamCheckBox */
		m_cameraCheckbox[index + 1]->changeState(state);
		if(ON_STATE == state)
		{
			/* Assign Layout Window */
			Layout::changeStreamStateOfWindow(MAIN_DISPLAY,
                                              index,
                                              m_currentDeviceName,
                                              ((m_cameraCheckbox[index + 1]->getButtonIndex()) - SYNC_PLAYBACK_CAMERASELECT_CHECKBOX),
                                              Layout::streamInfoArray[MAIN_DISPLAY][index].m_streamId,
                                              VIDEO_TYPE_SYNCPLAYBAKSTREAM,
                                              VIDEO_STATUS_RUNNING,
                                              VIDEO_ERROR_NONE);
			Layout::streamInfoArray[MAIN_DISPLAY][index].m_windowId = index;
            quint16 cameraIndex = Layout::getCameraId(index) - 1;
            SET_CAMERA_MASK_BIT(m_cameraValueForPlayRecord, cameraIndex);
			m_selectedCamCnt++;
		}
		else
		{
			/* Un-assign Layout Window */
			Layout::changeStreamStateOfWindow(MAIN_DISPLAY,
                                              index,
                                              "",
                                              INVALID_CAMERA_INDEX,
                                              MAX_STREAM_SESSION,
                                              VIDEO_TYPE_NONE,
                                              VIDEO_STATUS_NONE,
                                              VIDEO_ERROR_NONE);
		}
	}

	/* Show CamRec on Timeline for current selected window */
	if(ON_STATE == state)
	{
        m_timeLine->showRecordsOnTimeline(Layout::getCameraId(m_currentWindowIndex));
	}
	else
	{
        m_timeLine->resetRecordsOnTimeline();
	}
}

bool SyncPlayback::isAllButtonChecked()
{
	bool status = true;
	for(quint8 index = 1; index <= m_cameraListCount; index++)
	{
		if(index <= MAX_SYNC_PB_SESSION)
		{
			if(m_cameraCheckbox[index]->getCurrentState() == OFF_STATE)
			{
				status = false;
				break;
			}
		}
	}
	return status;
}

void SyncPlayback::selectDeselectCameraCheckBox(OPTION_STATE_TYPE_e iButtonState, int indexInPage)
{
	m_cameraCheckbox[0]->changeState((OPTION_STATE_TYPE_e)isAllButtonChecked());
	if(iButtonState == ON_STATE)
	{
		/* Assign Layout Window */
		Layout::changeStreamStateOfWindow(MAIN_DISPLAY,
                                          m_selectedCamCnt,
                                          m_currentDeviceName,
                                          (indexInPage - SYNC_PLAYBACK_CAMERASELECT_CHECKBOX),
                                          Layout::streamInfoArray[MAIN_DISPLAY][m_selectedCamCnt].m_streamId,
                                          VIDEO_TYPE_SYNCPLAYBAKSTREAM,
                                          VIDEO_STATUS_RUNNING,
                                          VIDEO_ERROR_NONE);
                                          Layout::streamInfoArray[MAIN_DISPLAY][m_selectedCamCnt].m_windowId = m_selectedCamCnt;
        quint16 cameraIndex = Layout::getCameraId(m_selectedCamCnt) - 1;
        SET_CAMERA_MASK_BIT(m_cameraValueForPlayRecord, cameraIndex);

        /* Show CamRec on Timeline for current selected window */
        if(m_currentWindowIndex == m_selectedCamCnt)
		{
            m_timeLine->showRecordsOnTimeline(Layout::getCameraId(m_currentWindowIndex));
		}
		m_selectedCamCnt++;
	}
	else
	{
        quint8 index = Layout::findWindowOfSyncPlaybackStream(MAIN_DISPLAY,m_currentDeviceName,indexInPage - SYNC_PLAYBACK_CAMERASELECT_CHECKBOX);

        /* Un-assign camera for search*/
        quint16 cameraIndex = Layout::getCameraId(index) - 1;
        CLR_CAMERA_MASK_BIT(m_cameraValueForPlayRecord, cameraIndex);
        m_selectedCamCnt--;

		/* Shift Layout Windows */
        for(;index < m_selectedCamCnt; index++)
		{
			Layout::changeStreamStateOfWindow(MAIN_DISPLAY,
											  index,
											  m_currentDeviceName,
											  Layout::getCameraId(index + 1),
											  Layout::streamInfoArray[MAIN_DISPLAY][index + 1].m_streamId,
											  VIDEO_TYPE_SYNCPLAYBAKSTREAM,
											  VIDEO_STATUS_RUNNING,
											  VIDEO_ERROR_NONE);
			Layout::streamInfoArray[MAIN_DISPLAY][index].m_windowId = index;
		}
        /* Update CamRec on Timeline for current selected window */
        if(m_selectedCamCnt == 0)
        {
            m_timeLine->resetRecordsOnTimeline();
        }
        else
        {
            m_timeLine->showRecordsOnTimeline(Layout::getCameraId(m_currentWindowIndex));
        }

		/* Un-assign Last Layout Window */
		Layout::changeStreamStateOfWindow(MAIN_DISPLAY,
										  m_selectedCamCnt,
										  "",
										  INVALID_CAMERA_INDEX,
										  MAX_STREAM_SESSION,
										  VIDEO_TYPE_NONE,
										  VIDEO_STATUS_NONE,
                                          VIDEO_ERROR_NONE);
	}
}

void SyncPlayback::changeMode(SYNCPB_TOOLBAR_MODE_TYPE_e mode)
{
	LAYOUT_TYPE_e tLayout = m_currentLayout;

	m_currentMode = mode;

	switch(mode)
	{
		case NORMAL_MODE:
		{
			m_toolbar->resetGeometry(m_mainBottomRectangle->x(),
									 m_mainBottomRectangle->y(),
									 BOTTOM_RECT_WIDTH,
									 TOOLBAR_NORMAL_MODE_HEIGHT);
			makeVisibleInvisibleAllElements(true);
			//layout change in alternate mode with reference to layout of current mode
			switch (m_currentLayout)
			{
                case ONE_X_ONE :
                    tLayout = ONE_X_ONE_PLAYBACK;
                    break;
                case TWO_X_TWO :
                    tLayout = TWO_X_TWO_PLAYBACK;
                    break;
                case THREE_X_THREE :
                    tLayout = THREE_X_THREE_PLAYBACK;
                    break;
                case FOUR_X_FOUR :
                    tLayout = FOUR_X_FOUR_PLAYBACK;
                    break;
                default:
                    tLayout = ONE_X_ONE_PLAYBACK;
                    break;
			}
			changeLayout(tLayout);
			if(m_elementList[m_currentElement] != NULL)
            {
				m_elementList[m_currentElement]->forceFocusToPage(true);
            }

            //layout previous change in alternate mode with reference to layout of current mode
            switch (m_prevLayout)
            {
                case ONE_X_ONE :
                    m_prevLayout = ONE_X_ONE_PLAYBACK;
                    break;
                case TWO_X_TWO :
                    m_prevLayout = TWO_X_TWO_PLAYBACK;
                    break;
                case THREE_X_THREE :
                    m_prevLayout = THREE_X_THREE_PLAYBACK;
                    break;
                case FOUR_X_FOUR :
                    m_prevLayout = FOUR_X_FOUR_PLAYBACK;
                    break;
                default:
                    break;
            }
            /* Enable CROP_AND_BACKUP_BUTTON in Normal Mode for states  other than this */
            if((m_currentPlaybackState != SYNC_PLAYBACK_NONE_STATE) && (m_currentPlaybackState != SYNC_PLAYBACK_STOP_STATE))
            {
                m_toolbar->changeButtonEnableState(CROP_AND_BACKUP_BUTTON, true);
            }
		}
		break;

		case FULL_MODE:
		{
            m_toolbar->resetGeometry(0,
                                     (this->height() - TOOLBAR_FULL_MODE_HEIGHT),
                                     this->width(),
                                     TOOLBAR_FULL_MODE_HEIGHT);
			makeVisibleInvisibleAllElements(false);
			switch (m_currentLayout)
			{
                case ONE_X_ONE_PLAYBACK :
                    tLayout = ONE_X_ONE;
                    break;
                case TWO_X_TWO_PLAYBACK :
                    tLayout = TWO_X_TWO;
                    break;
                case THREE_X_THREE_PLAYBACK :
                    tLayout = THREE_X_THREE;
                    break;
                case FOUR_X_FOUR_PLAYBACK :
                    tLayout = FOUR_X_FOUR;
                    break;
                default:
                    tLayout = ONE_X_ONE;
                    break;
			}
			changeLayout(tLayout);

            //layout previous change in alternate mode with reference to layout of current mode
            switch (m_prevLayout)
            {
                case ONE_X_ONE_PLAYBACK :
                    m_prevLayout = ONE_X_ONE;
                    break;
                case TWO_X_TWO_PLAYBACK :
                    m_prevLayout = TWO_X_TWO;
                    break;
                case THREE_X_THREE_PLAYBACK :
                    m_prevLayout = THREE_X_THREE;
                    break;
                case FOUR_X_FOUR_PLAYBACK :
                    m_prevLayout = FOUR_X_FOUR;
                    break;
                default:
                    break;
            }

            /* Disable CROP_AND_BACKUP_BUTTON in Full Mode */
            m_toolbar->changeButtonEnableState(CROP_AND_BACKUP_BUTTON, false);
		}
		break;

		default:
			break;
	}

	m_toolbar->changeMode(mode);
	if(mode == FULL_MODE)
	{
		this->setFocus();
	}
}

void SyncPlayback::changeLayout(LAYOUT_TYPE_e layout)
{
	/* Do not proceed if current layout is already set */
	if(layout == m_currentLayout)
	{
		return;
	}
	m_prevLayout = m_currentLayout;
	m_currentLayout = layout;

	emit sigChangeLayout(m_currentLayout, MAIN_DISPLAY, m_currentWindowIndex, true, true);
}

void SyncPlayback::changeCurrentPlaybackState(SYNC_PLAYBACK_STATE_e state)
{
	if(state != m_currentPlaybackState)
	{
        m_previousPlaybackState = m_currentPlaybackState;
		m_currentPlaybackState = state;
		m_toolbar->changeState(state);
		m_timeLine->changeState(state);

        bool isOtherControlActive = (state == SYNC_PLAYBACK_NONE_STATE) || (state == SYNC_PLAYBACK_STOP_STATE);

		m_calender->setIsEnabled(isOtherControlActive);
		m_deviceNameDropDownBox->setIsEnabled(isOtherControlActive);
		m_recDriveListDropBox->setIsEnabled(isOtherControlActive);
		m_searchButton->setIsEnabled(isOtherControlActive);
		for(quint8 index = 0; index < MAX_RECORDING_TYPE; index++)
		{
			m_recordingTypeCheckbox[index]->setIsEnabled(isOtherControlActive);
		}

        for(quint8 index = 0; index <= m_cameraListCount; index++)
		{
			m_cameraCheckbox[index]->setIsEnabled(isOtherControlActive);
		}
		if(m_syncScrollbar != NULL)
		{
			m_syncScrollbar->setIsEnabled(isOtherControlActive);
		}

        /* Disable Clip List button when clip is not present */
        if(m_cropAndBackupData.length() == 0)
		{
            m_toolbar->changeButtonEnableState(LIST_BUTTON, false);
		}

		if(m_currentZoomState == SYNC_PLAYBACK_ZOOM_IN)
		{
			m_toolbar->changeButtonEnableState(CHANGE_MODE_BUTTON, false);
			m_toolbar->changeButtonEnableState(LAYOUT_BUTTON, false);
            m_toolbar->changeButtonEnableState(CROP_AND_BACKUP_BUTTON, false);
		}

		bool flag = ((m_currentPlaybackState == SYNC_PLAYBACK_CROPANDBACKUP_STATE) ? false : true);
        for(quint8 index = 0; index < MAX_HOUR_FORMAT_TYPE; index++)
        {
            m_hoursRadioButtion[index]->setIsEnabled(flag);
        }

        /* Do not allow CROP_AND_BACKUP_BUTTON to get enabled in full mode even if state changes */
        if(m_currentMode == FULL_MODE)
        {
            m_toolbar->changeButtonEnableState(CROP_AND_BACKUP_BUTTON, false);
        }
	}
}

void SyncPlayback::changeAudioStateOfAudioButton()
{
    if(Layout::currentDisplayConfig[MAIN_DISPLAY].selectedWindow == m_windowIndexForAudio)
    {
        m_toolbar->changeButtonState(AUDIO_BUTTON, STATE_2);
    }
    else
    {
        m_toolbar->changeButtonState(AUDIO_BUTTON, STATE_1);
    }
}

void SyncPlayback::makeVisibleInvisibleAllElements(bool visibleFlag)
{
	m_mainRightRectangle->setVisible(visibleFlag);
	m_mainBottomRectangle->setVisible(visibleFlag);
	m_closeButton->setVisible(visibleFlag);
	m_toolbar->setVisible(visibleFlag);
	m_timeLine->setVisible(visibleFlag);
	m_calender->setVisible(visibleFlag);
	m_deviceNameDropDownBox->setVisible(visibleFlag);
	m_recDriveListDropBox->setVisible(visibleFlag);

	if(m_cameraListCount)
	{
		m_camerasHeading->setVisible(visibleFlag);
		m_cameraCheckbox[0]->setVisible(visibleFlag);
        m_cameraListSeparator[0]->setVisible(visibleFlag);

		for(quint8 index = m_firstCameraIndex; index < m_lastCameraIndex; index++)
		{
			/* Visible Camera CheckBox */
			m_cameraCheckbox[(index + 1)]->setVisible(visibleFlag);
			/* Visible Camera RecordType Labels */
			for(quint8 loop = 0; loop< MAX_RECORDING_TYPE; loop++)
			{
				m_cameraRecType[index][loop]->setVisible(visibleFlag);
			}
		}

        /* Visible Camera List Separator */
        quint8 limit = (m_cameraListCount > MAX_CAM_LIST_SEPARATOR_CNT ? MAX_CAM_LIST_SEPARATOR_CNT : m_cameraListCount);
        for(quint8 index = 0; index < limit; index++)
        {
            m_cameraListSeparator[(index + 1)]->setVisible(visibleFlag);
        }

		if(m_syncScrollbar != NULL)
		{
			/*Scrollbar's only visibility is modified when on searching cameras with recording, now if record count is less than Camera seen
			 then on switching from Full to Normal Mode we will see scrollbar on screen which is not required hence this condition*/
            if(m_cameraListCount > MAX_SCROLL_CAM_LIST_CNT)
            {
				m_syncScrollbar->setVisible(visibleFlag);
            }
		}
	}
    else
    {
        m_errorMsgTxt->setVisible(visibleFlag);
    }

	m_searchButton->setVisible(visibleFlag);
	m_hoursHeading->setVisible(visibleFlag);
	for(quint8 index = 0; index < MAX_HOUR_FORMAT_TYPE; index++)
	{
		m_hoursRadioButtion[index]->setVisible(visibleFlag);
	}

	m_typesHeading->setVisible(visibleFlag);

	for(quint8 index = 0; index < MAX_RECORDING_TYPE; index++)
	{
		m_recordingTypeCheckbox[index]->setVisible(visibleFlag);
		m_recordingInitialLabel[index]->setVisible(visibleFlag);
		m_recordingLabel[index]->setVisible(visibleFlag);
	}
}

void SyncPlayback::processDeviceResponse(DevCommParam* param, QString deviceName)
{
	quint64 tDateCollection;

    if (deviceName != m_currentDeviceName)
    {
        return;
    }

    if (param->msgType != MSG_SET_CMD)
    {
        return;
    }

    switch(param->cmdType)
    {
        case GET_DATE_TIME:
        {
            if(false == m_isGetDateTimeRequestPending)
            {
                break;
            }

            m_isGetDateTimeRequestPending = false;
            if (param->deviceStatus != CMD_SUCCESS)
            {
                if(m_isPreProcessingIsDone)
                {
                    m_syncPlaybackLoadingText->setVisible(false);
                }
                MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                break;
            }

            m_payloadLib->parseDevCmdReply(true, param->payload);
            m_calender->initializeCalender((m_payloadLib->getCnfgArrayAtIndex(0).toString().mid(0, 8)));

            /* After getting successful response get Month wise records */
            searchDatesHavingRecord();
        }
        break;

        case PLYBCK_SRCH_MNTH:
        {
            if(false == m_isSearchDateRequestPending)
            {
                break;
            }

            m_isSearchDateRequestPending = false;
            switch(param->deviceStatus)
            {
                case CMD_SUCCESS:
                {
                    tDateCollection = m_applController->GetSyncPlaybackMonthRecord();
                    m_searchAttemptsForDate = 0;
                    m_calender->changeDateCollection(tDateCollection);
                    m_syncPlaybackLoadingText->setVisible(false);

                    /* Disable search button on no dates having record */
                    if(tDateCollection == 0)
                    {
                        m_searchButton->setIsEnabled(false);
                    }
                }
                break;

                case CMD_REQUEST_IN_PROGRESS:
                {
                    if(m_searchAttemptsForDate != MAX_SEARCH_ATTEMPTS)
                    {
                        searchDatesHavingRecord();
                    }
                    else
                    {
                        m_searchAttemptsForDate = 0;
                        if(m_isPreProcessingIsDone)
                        {
                            m_syncPlaybackLoadingText->setVisible(false);
                        }
                        MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                    }
                }
                break;

                default:
                {
                    m_searchAttemptsForDate = 0;
                    if(m_isPreProcessingIsDone)
                    {
                        m_syncPlaybackLoadingText->setVisible(false);
                    }
                    MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                }
                break;
            }
        }
        break;

        case PLYBCK_SRCH_DAY:
        {
            if(false == m_isSearchHoursRequestPending)
            {
                break;
            }

            m_isSearchHoursRequestPending = false;
            if(m_isPreProcessingIsDone)
            {
                m_syncPlaybackLoadingText->setVisible(false);
            }

            switch(param->deviceStatus)
            {
                case CMD_SUCCESS:
                {
                    /* Create Camera List as per avialable Recordings */
                    CreateCameraList();
                    m_searchAttemptsForHours = 0;
                    if(m_retryTimer->isActive())
                    {
                        m_retryTimer->stop();
                    }
                }
                break;

                case CMD_REQUEST_IN_PROGRESS:
                {
                    if(m_searchAttemptsForHours != MAX_SEARCH_ATTEMPTS)
                    {
                        m_retryTimer->start();
                    }
                    else
                    {
                        m_searchAttemptsForHours = 0;
                        if(m_retryTimer->isActive())
                        {
                            m_retryTimer->stop();
                        }
                        MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                    }
                }
                break;

                default:
                {
                    m_searchAttemptsForHours = 0;
                    if(m_retryTimer->isActive())
                    {
                        m_retryTimer->stop();
                    }
                    MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                }
                break;
            }
        }
        break;

        case SYNC_CLP_RCD:
        {
            switch(param->deviceStatus)
            {
                case CMD_SUCCESS:
                {
                    if(!m_isSyncRequestPending)
                    {
                        /* Searching month records required on date change in different month */
                        m_isSearchDateRequired = true;
                        clearSyncPlaybackRecord();
                    }
                }
                break;

                case CMD_INVALID_FILE_SIZE:
                {
                    m_payloadLib->parseDevCmdReply(true, param->payload);
                    MessageBanner::addMessageInBanner(NOT_ENOUGH_SPACE_ON_USB(m_payloadLib->getCnfgArrayAtIndex(0).toString(),
                                                                              m_payloadLib->getCnfgArrayAtIndex(1).toString()));
                    if(m_syncPlaybackCropAndBackup != NULL)
                    {
                        m_syncPlaybackCropAndBackup->changeBackupExpotStateToStop();
                    }
                }
                break;

                default:
                {
                    MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                    if(m_syncPlaybackCropAndBackup != NULL)
                    {
                        m_syncPlaybackCropAndBackup->changeBackupExpotStateToStop();
                    }
                }
                break;
            }
        }
        break;

        case STP_BCKUP:
        {
            if(param->deviceStatus != CMD_SUCCESS)
            {
                MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
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

void SyncPlayback::processStreamRequestResponse(STREAM_COMMAND_TYPE_e streamCommandType,
                                                StreamRequestParam *streamRequestParam, DEVICE_REPLY_TYPE_e deviceReply)
{
    if(streamRequestParam->deviceName != m_currentDeviceName)
    {
        return;
    }

    if((deviceReply == CMD_SERVER_NOT_RESPONDING) || (deviceReply == CMD_REC_DRIVE_CONFIG_CHANGES)
            || (deviceReply == CMD_REC_MEDIA_ERR) || (deviceReply == CMD_STREAM_HDD_FORMAT)
            || (deviceReply == CMD_PLAYBACK_PROCESS_ERROR))
    {
        /* If server responce error occurs when Clip-Export has started */
        stopClipMaking();

        /* If server responce error occurs when GUI in SYNC_PLAYBACK_ZOOM_IN state */
        if(m_currentZoomState == SYNC_PLAYBACK_ZOOM_IN)
        {
            exitFromZoomAction();
        }

        /* If server responce error occurs when GUI in FULL_MODE state */
        if(m_currentMode == FULL_MODE)
        {
            changeMode(NORMAL_MODE);
        }

        /* When Error condition occurs it should not go for searching records */
         m_isSearchDateRequired = false;

        /* clear window and initialize all parameters to default type */
        processClearSyncPlaybackAction();

        m_isGetDateTimeRequestPending = m_isSearchDateRequestPending = m_isSearchHoursRequestPending = false;
        m_isSyncRequestPending = false;

        changeAudioStateOfAudioButton();
        MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(deviceReply));

        if (m_isCloseButtonClicked == true)
        {
            emit sigClosePage(SYNC_PLAYBACK_BUTTON);
        }

        return;
    }


    switch(streamCommandType)
    {
        case PLAY_SYNCPLABACK_STREAM_COMMAND:
        case SYNCPLAY_SYNCPLABACK_STREAM_COMMAND:
        {
            m_isSyncRequestPending = false;
            quint8 cameraIndex = streamRequestParam->payload.toInt();
            quint16 windowIndex = Layout::findWindowOfSyncPlaybackStream(MAIN_DISPLAY, m_currentDeviceName, cameraIndex);

            //Added for two way audio. If reply was not success than reset audio variable bcoz when we start client audio we check for this variable
            if(((deviceReply != CMD_SUCCESS) && (deviceReply != CMD_STREAM_PB_SPEED)) && (m_isAudioCommandSend == true) && (deviceReply != CMD_PLAYBACK_TIME))
            {
                m_windowIndexForAudio = MAX_CHANNEL_FOR_SEQ;
            }

            switch(deviceReply)
            {
                case CMD_SUCCESS:
                {
                    if(streamCommandType == PLAY_SYNCPLABACK_STREAM_COMMAND)
                    {
                        for(quint8 index = 0; index < m_selectedCamCnt; index++)
                        {
                            Layout::changeStreamStateOfWindow(MAIN_DISPLAY,
                                                              index,
                                                              m_currentDeviceName,
                                                              Layout::getCameraId(index),
                                                              streamRequestParam->streamId,
                                                              VIDEO_TYPE_SYNCPLAYBAKSTREAM,
                                                              VIDEO_STATUS_RUNNING,
                                                              VIDEO_ERROR_NONE);
                            Layout::streamInfoArray[MAIN_DISPLAY][index].m_windowId = index;
                        }
                    }

                    if(m_currentPlaybackState != SYNC_PLAYBACK_CROPANDBACKUP_STATE)
                    {
                        if(m_playBackDirection == FORWARD_PLAY)
                        {
                            changeCurrentPlaybackState(SYNC_PLAYBACK_PLAY_STATE);
                            m_toolbar->changeButtonState(PLAY_BUTTON, STATE_2);

                            //if previously it was playing in backward direction then reverseplaybutton was showing pause, so make it showing play
                            m_toolbar->changeButtonState(REVERSE_PLAY_BUTTON, STATE_1);
                        }
                        else
                        {
                            changeCurrentPlaybackState(SYNC_PLAYBACK_REVERSEPLAY_STATE);
                            m_toolbar->changeButtonState(REVERSE_PLAY_BUTTON, STATE_2);

                            //if previously it was playing in backward direction then reverseplaybutton was showing pause, so make it showing play
                            m_toolbar->changeButtonState(PLAY_BUTTON, STATE_1);
                        }
                    }

                    if(m_isAudioCommandSend)
                    {
                        m_isAudioCommandSend = false;
                        sendSyncPlaybackAudioCommand();
                    }
                    if(m_isRequestPendingForSliderPositionChanged)
                    {
                        m_isRequestPendingForSliderPositionChanged = false;
                    }
                    if(m_isSliderPositionChangedbyUser)
                    {
                        m_isSliderPositionChangedbyUser  = false;
                    }
                    m_isFirstTimePlay = false;
                    m_syncPlaybackStreamId = streamRequestParam->streamId;
                }
                break;

                case CMD_STREAM_PB_SPEED:
                {
                    m_playBackSpeed = (PB_SPEED_e) streamRequestParam->payload.toInt();
                    m_toolbar->changePlaybackSpeedDirection(m_playBackSpeed, m_playBackDirection);
                }
                break;

                case CMD_STREAM_VIDEO_LOSS:
                {
                    if ((streamRequestParam->payload == "") || (windowIndex >= MAX_CHANNEL_FOR_SEQ))
                    {
                        break;
                    }

                    Layout::changeStreamStateOfWindow(MAIN_DISPLAY,
                                                      windowIndex,
                                                      m_currentDeviceName,
                                                      cameraIndex,
                                                      streamRequestParam->streamId,
                                                      VIDEO_TYPE_SYNCPLAYBAKSTREAM,
                                                      VIDEO_STATUS_VIDEOLOSS,
                                                      VIDEO_ERROR_VIDEOLOSS);
                }
                break;

                case CMD_STREAM_NO_VIDEO_LOSS:
                {
                    if ((streamRequestParam->payload == "") || (windowIndex >= MAX_CHANNEL_FOR_SEQ))
                    {
                        break;
                    }

                    Layout::changeStreamStateOfWindow(MAIN_DISPLAY,
                                                      windowIndex,
                                                      m_currentDeviceName,
                                                      cameraIndex,
                                                      streamRequestParam->streamId,
                                                      VIDEO_TYPE_SYNCPLAYBAKSTREAM,
                                                      VIDEO_STATUS_RUNNING,
                                                      VIDEO_ERROR_NONE);
                }
                break;

                case CMD_PLAYBACK_TIME:
                {
                    //Nothing to be play and stop play back then no need to update time
                    if((m_currentPlaybackState == SYNC_PLAYBACK_NONE_STATE) || (m_currentPlaybackState == SYNC_PLAYBACK_STOP_STATE))
                    {
                        break;
                    }

                    m_lastFrameSecond = streamRequestParam->payload.toULongLong();
                    QDateTime realDate = QDateTime::fromMSecsSinceEpoch(m_lastFrameSecond * 1000);
                    m_toolbar->changeDateTime(realDate);

                    if((!m_isRequestPendingForSliderPositionChanged) && (!m_isSliderPositionChangedbyUser))
                    {
                        m_timeLine->changeTime(realDate.time());
                    }
                }
                break;

                case CMD_MAX_STREAM_LIMIT:
                {
                    if(streamRequestParam->payload != "")
                    {
                        MessageBanner::addMessageInBanner(UNABLE_TO_PLAY(m_applController->GetCameraNameOfDevice(m_currentDeviceName,
                                                                         (streamRequestParam->payload.toInt() - 1)),
                                                                         ValidationMessage::getDeviceResponceMessage(CMD_MAX_STREAM_LIMIT)));
                    }
                    else
                    {
                        MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(CMD_MAX_STREAM_LIMIT));
                    }
                }
                break;

                case CMD_STREAM_PLAYBACK_OVER:
                {
                    /* Searching month records required on date change in different month */
                    m_isSearchDateRequired = true;
                    clearSyncPlaybackRecord();
                }
                break;

                case CMD_DECODER_ERROR:
                {
                    if ((streamRequestParam->payload == "") || (windowIndex >= MAX_CHANNEL_FOR_SEQ))
                    {
                        break;
                    }

                    Layout::changeStreamStateOfWindow(MAIN_DISPLAY,
                                                      windowIndex,
                                                      m_currentDeviceName,
                                                      cameraIndex,
                                                      streamRequestParam->streamId,
                                                      VIDEO_TYPE_SYNCPLAYBAKSTREAM,
                                                      VIDEO_STATUS_ERROR,
                                                      VIDEO_ERROR_OTHERERROR);
                }
                break;

                case CMD_DECODER_CAPACITY_ERROR:
                {
                    if ((streamRequestParam->payload == "") || (windowIndex >= MAX_CHANNEL_FOR_SEQ))
                    {
                        break;
                    }

                    Layout::changeStreamStateOfWindow(MAIN_DISPLAY,
                                                      windowIndex,
                                                      m_currentDeviceName,
                                                      cameraIndex,
                                                      streamRequestParam->streamId,
                                                      VIDEO_TYPE_SYNCPLAYBAKSTREAM,
                                                      VIDEO_STATUS_ERROR,
                                                      VIDEO_ERROR_NO_DECODING_CAP);
                }
                break;

                case CMD_STREAM_FILE_ERROR:
                {
                    if ((streamRequestParam->payload == "") || (windowIndex >= MAX_CHANNEL_FOR_SEQ))
                    {
                        break;
                    }

                    Layout::changeStreamStateOfWindow(MAIN_DISPLAY,
                                                      windowIndex,
                                                      m_currentDeviceName,
                                                      cameraIndex,
                                                      streamRequestParam->streamId,
                                                      VIDEO_TYPE_SYNCPLAYBAKSTREAM,
                                                      VIDEO_STATUS_ERROR,
                                                      VIDEO_ERROR_OTHERERROR);

                    MessageBanner::addMessageInBanner(UNABLE_TO_PLAY(m_applController->GetCameraNameOfDevice(m_currentDeviceName, (cameraIndex - 1)), "File I/O Error"));
                    EPRINT(GUI_SYNC_PB_MEDIA, "file I/O error: [cmd=STREAM_FILE_ERROR], [camera=%d]", (cameraIndex - 1));
                }
                break;

                case CMD_STREAM_CONFIG_CHANGE:
                {
                    if ((streamRequestParam->payload == "") || (windowIndex >= MAX_CHANNEL_FOR_SEQ))
                    {
                        break;
                    }

                    Layout::changeStreamStateOfWindow(MAIN_DISPLAY,
                                                      windowIndex,
                                                      m_currentDeviceName,
                                                      cameraIndex,
                                                      streamRequestParam->streamId,
                                                      VIDEO_TYPE_SYNCPLAYBAKSTREAM,
                                                      VIDEO_STATUS_ERROR,
                                                      VIDEO_ERROR_OTHERERROR);

                    MessageBanner::addMessageInBanner(CAMERA_CONFIG_CHANGED(m_applController->GetCameraNameOfDevice(m_currentDeviceName, (cameraIndex - 1))));
                }
                break;

                case CMD_NO_PRIVILEGE:
                {
                    if (streamRequestParam->payload == "")
                    {
                        MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(deviceReply));
                        break;
                    }

                    if (windowIndex >= MAX_CHANNEL_FOR_SEQ)
                    {
                        break;
                    }

                    Layout::changeStreamStateOfWindow(MAIN_DISPLAY,
                                                      windowIndex,
                                                      m_currentDeviceName,
                                                      cameraIndex,
                                                      streamRequestParam->streamId,
                                                      VIDEO_TYPE_SYNCPLAYBAKSTREAM,
                                                      VIDEO_STATUS_ERROR,
                                                      VIDEO_ERROR_NOUSERRIGHTS);
                }
                break;

                case CMD_PROCESS_ERROR:
                {
                    if(m_isRequestPendingForSliderPositionChanged)
                    {
                        m_isRequestPendingForSliderPositionChanged = false;
                    }
                    if(m_isSliderPositionChangedbyUser)
                    {
                        m_isSliderPositionChangedbyUser  = false;
                    }
                    MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(deviceReply));
                }
                break;

                default:
                {
                    MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(deviceReply));
                    if ((streamRequestParam->payload == "") || (windowIndex >= MAX_CHANNEL_FOR_SEQ))
                    {
                        break;
                    }

                    Layout::changeStreamStateOfWindow(MAIN_DISPLAY,
                                                      windowIndex,
                                                      m_currentDeviceName,
                                                      cameraIndex,
                                                      streamRequestParam->streamId,
                                                      VIDEO_TYPE_SYNCPLAYBAKSTREAM,
                                                      VIDEO_STATUS_ERROR,
                                                      VIDEO_ERROR_OTHERERROR);
                }
                break;
            }
        }
        break;

        case SET_SPEED_SYNCPLABACK_STREAM_COMMAND:
        {
            m_isSyncRequestPending = false;
            if (deviceReply != CMD_SUCCESS)
            {
                DPRINT(GUI_SYNC_PB_MEDIA, "set playback speed: [deviceReply=%d]", deviceReply);
                break;
            }

            if(m_playBackSpeed != streamRequestParam->payload.toInt())
            {
                MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(CMD_DECODER_ERROR));
            }
            m_playBackSpeed = (PB_SPEED_e) streamRequestParam->payload.toInt();
            DPRINT(GUI_SYNC_PB_MEDIA, "set playback speed: [speed=%d]", m_playBackSpeed);
            m_toolbar->changePlaybackSpeedDirection(m_playBackSpeed, m_playBackDirection);
            if ((m_playBackSpeed == PB_SPEED_NORMAL) && (m_playBackDirection == FORWARD_PLAY))
            {
                /* Enable AUDIO_BUTTON */
                m_toolbar->changeButtonEnableState(AUDIO_BUTTON, true);
            }
            else if((m_playBackSpeed == PB_SPEED_2S) || (m_playBackSpeed == PB_SPEED_2F))
            {
                /* Disable AUDIO_BUTTON */
                m_toolbar->changeButtonEnableState(AUDIO_BUTTON, false);
            }
        }
        break;

        case STEPFORWARD_SYNCPLABACK_STREAM_COMMAND:
        case STEPBACKWARD_SYNCPLABACK_STREAM_COMMAND:
        {
            m_isSyncRequestPending = false;
            switch(deviceReply)
            {
                case CMD_SUCCESS:
                {
                    m_toolbar->changeButtonState(PLAY_BUTTON, STATE_1);
                    m_toolbar->changeButtonState(REVERSE_PLAY_BUTTON, STATE_1);
                    //it is seen that first CMD_STREAM_PLAYBACK_OVER for SYNC_PLYBCK_RCD
                    //is recivied and then CMD_SUCCESS for SYNC_STEP_REVERSE is recivied
                    if((m_currentPlaybackState != SYNC_PLAYBACK_NONE_STATE) && (m_currentPlaybackState != SYNC_PLAYBACK_STOP_STATE))
                    {
                        changeCurrentPlaybackState(SYNC_PLAYBACK_STEP_STATE);
                    }
                }
                break;

                case CMD_STREAM_PB_SPEED:
                {
                    m_playBackSpeed = (PB_SPEED_e) streamRequestParam->payload.toInt();
                    m_toolbar->changePlaybackSpeedDirection(m_playBackSpeed, m_playBackDirection);
                }
                break;

                default:
                {
                    MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(deviceReply));
                }
                break;
            }

            if(m_isRequestPendingForSliderPositionChanged)
            {
                m_isRequestPendingForSliderPositionChanged = false;
            }

            if(m_isSliderPositionChangedbyUser)
            {
                m_isSliderPositionChangedbyUser = false;
            }
        }
        break;

        case PAUSE_SYNCPLABACK_STREAM_COMMAND:
        {
            m_isSyncRequestPending = false;
            if (deviceReply != CMD_SUCCESS)
            {
                break;
            }

            if(m_currentPlaybackState == SYNC_PLAYBACK_PLAY_STATE)
            {
                m_toolbar->changeButtonState(PLAY_BUTTON, STATE_1);
            }
            else
            {
                m_toolbar->changeButtonState(REVERSE_PLAY_BUTTON, STATE_1);
            }
            changeCurrentPlaybackState(SYNC_PLAYBACK_PAUSE_STATE);
        }
        break;

        case STOP_SYNCPLABACK_STREAM_COMMAND:
        {
            m_isSyncRequestPending = false;
            if(deviceReply != CMD_SUCCESS)
            {
                MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(deviceReply));
            }
            processStopSyncPlaybackAction();

            if(m_windowIndexForAudio != MAX_CHANNEL_FOR_SEQ)
            {
                Layout::streamInfoArray[MAIN_DISPLAY][m_windowIndexForAudio].m_audioStatus = false;
                m_windowIndexForAudio = MAX_CHANNEL_FOR_SEQ;
                m_windowIndexForAudioOld = MAX_CHANNEL_FOR_SEQ;
            }
        }
        break;

        case CLEAR_SYNCPLABACK_STREAM_COMMAND:
        {
            processClearSyncPlaybackAction();
            if (m_isCloseButtonClicked == true)
            {
                emit sigClosePage(SYNC_PLAYBACK_BUTTON);
            }
        }
        break;

        case AUDIO_SYNCPLABACK_STREAM_COMMAND:
        {
            m_isSyncRequestPending = false;
            if(m_windowIndexForAudio != m_windowIndexForAudioOld)
            {
                if(m_windowIndexForAudioOld != MAX_CHANNEL_FOR_SEQ)
                {
                    Layout::streamInfoArray[MAIN_DISPLAY][m_windowIndexForAudioOld].m_audioStatus = false;
                    Layout::updateWindowDataForSyncPB(m_windowIndexForAudioOld);
                }
                m_windowIndexForAudioOld = MAX_CHANNEL_FOR_SEQ;
            }

            if (deviceReply != CMD_SUCCESS)
            {
                m_windowIndexForAudio = MAX_CHANNEL_FOR_SEQ;
                MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(deviceReply));
                changeAudioStateOfAudioButton();
                break;
            }

            if(streamRequestParam->audioStatus == false)
            {
                if(m_windowIndexForAudio != MAX_CHANNEL_FOR_SEQ)
                {
                    Layout::streamInfoArray[MAIN_DISPLAY][m_windowIndexForAudio].m_audioStatus = false;
                }

                Layout::updateWindowDataForSyncPB(m_windowIndexForAudio);
                m_windowIndexForAudio = MAX_CHANNEL_FOR_SEQ;
            }
            else
            {
                if(m_windowIndexForAudio != MAX_CHANNEL_FOR_SEQ)
                {
                    m_windowIndexForAudioOld = m_windowIndexForAudio;
                    Layout::streamInfoArray[MAIN_DISPLAY][m_windowIndexForAudio].m_audioStatus = true;
                    Layout::updateWindowDataForSyncPB(m_windowIndexForAudio);
                }
            }

            changeAudioStateOfAudioButton();
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void SyncPlayback::updateManualBackupStatusEventAction(QString deviceName, quint8 copiedPercentage)
{
    if(m_syncPlaybackCropAndBackup != NULL)
    {
        m_syncPlaybackCropAndBackup->updateManualBackupStatusEventAction(deviceName, copiedPercentage);
    }
}

void SyncPlayback::updateManualBackupSystemEventAction(QString deviceName, LOG_EVENT_STATE_e eventState)
{
    if(m_syncPlaybackCropAndBackup != NULL)
    {
        m_syncPlaybackCropAndBackup->updateManualBackupSystemEventAction(deviceName, eventState);
    }
}

bool SyncPlayback::createStreamCommand(STREAM_COMMAND_TYPE_e streamCommandType, QString payload)
{
    bool status = false;
    bool isRequestSend = false;
    SERVER_SESSION_INFO_t serverSessionInfo;
    quint8 pendingStreamRequest = 0;

    do
    {
        status = m_applController->GetServerSessionInfo(m_currentDeviceName, serverSessionInfo);
        if (status == false)
        {
            break;
        }

        if((m_syncPlaybackStreamId == MAX_STREAM_SESSION) && (streamCommandType == CLEAR_SYNCPLABACK_STREAM_COMMAND))
        {
            break;
        }

        StreamRequestParam *streamRequestParam = new StreamRequestParam();
        streamRequestParam->streamId = m_syncPlaybackStreamId;
        streamRequestParam->deviceName = m_currentDeviceName;
        streamRequestParam->displayType = MAIN_DISPLAY;
        streamRequestParam->streamRequestType = SYNC_PLAYBACK_REQUEST;
        streamRequestParam->payload = payload;

        if(streamCommandType == AUDIO_SYNCPLABACK_STREAM_COMMAND)
        {
            streamRequestParam->channelId = Layout::streamInfoArray[MAIN_DISPLAY][m_windowIndexForAudio].m_cameraId;
            streamRequestParam->audioStatus = (m_toolbar->getButtonState(AUDIO_BUTTON) == STATE_1);
        }

        pendingStreamRequest = m_applController->processStreamActivity(streamCommandType, serverSessionInfo, streamRequestParam);
        if (pendingStreamRequest == 0)
        {
            status = false;
            DELETE_OBJ(streamRequestParam);
            break;
        }

        isRequestSend = true;
        if (streamCommandType != CLEAR_SYNCPLABACK_STREAM_COMMAND)
        {
            m_isSyncRequestPending = true;
        }

    }while(0);

    if(isRequestSend == false)
    {
        processClearSyncPlaybackAction();
        if (m_isCloseButtonClicked == true)
        {
            emit sigClosePage(SYNC_PLAYBACK_BUTTON);
        }
    }

    return status;
}

void SyncPlayback::startSyncPlaybackRecord()
{
	QString					payloadString;
    quint8					fieldIdx = 0;
	STREAM_COMMAND_TYPE_e	streamCommandType;

    DPRINT(GUI_SYNC_PB_MEDIA, "start sync playback: [cameraMask=0x%llx & 0x%llx], [cnt=%d], [recordType=0x%x], [direction=%d], [speed=%d], [audio=0x%llx & 0x%llx]",
           m_cameraValueForPlayRecord.bitMask[0], m_cameraValueForPlayRecord.bitMask[1], m_selectedCamCnt, m_recordValueForSearch, m_playBackDirection,
            m_playBackSpeed, m_audioValueForPlayRecord.bitMask[0], m_audioValueForPlayRecord.bitMask[1]);

    m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, m_cameraValueForPlayRecord.bitMask[0]);
    m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, m_playBackDirection);

    if((m_isFirstTimePlay) || (m_isSliderPositionChangedbyUser))
    {
        m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, (m_calender->getDateMonthYearString() + m_timeLine->getTimeString()));
    }
    else
    {
        m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, QDateTime::fromMSecsSinceEpoch(m_lastFrameSecond * 1000).toString("ddMMyyyyhhmmss"));
    }

    m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, m_playBackSpeed);
    m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, m_recordValueForSearch);

    if(m_isFirstTimePlay)
    {
        quint8 recordIndex = m_recDriveListDropBox->getIndexofCurrElement();
        m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, (recordIndex == 0) ? 4 : (recordIndex - 1));
        m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, m_cameraValueForPlayRecord.bitMask[1]);
		streamCommandType = PLAY_SYNCPLABACK_STREAM_COMMAND;
    }
    else
    {
        m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, m_audioValueForPlayRecord.bitMask[0]);
        m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, m_moveCount);
        m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, m_cameraValueForPlayRecord.bitMask[1]);
        m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, m_audioValueForPlayRecord.bitMask[1]);
        streamCommandType = SYNCPLAY_SYNCPLABACK_STREAM_COMMAND;
    }

	if(m_isSliderPositionChangedbyUser)
    {
        m_isRequestPendingForSliderPositionChanged = true;
    }

    payloadString = m_payloadLib->createDevCmdPayload(fieldIdx);
    createStreamCommand(streamCommandType, payloadString);
}

void SyncPlayback::SetSpeed(void)
{
    QString payloadString;
    quint8  fieldIdx = 0;

    DPRINT(GUI_SYNC_PB_MEDIA, "set sync playback speed: [speed=%d], [direction=%d]", m_playBackSpeed, m_playBackDirection);

    m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, m_playBackSpeed);
    m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, m_playBackDirection);

    payloadString = m_payloadLib->createDevCmdPayload(fieldIdx);
	createStreamCommand(SET_SPEED_SYNCPLABACK_STREAM_COMMAND, payloadString);
}

void SyncPlayback::setQuickBackupFlag(bool flag)
{
    m_isQuickBackupon = flag;
}
void SyncPlayback::pauseSyncPlaybackRecord()
{
    DPRINT(GUI_SYNC_PB_MEDIA, "pause sync playback");
    createStreamCommand(PAUSE_SYNCPLABACK_STREAM_COMMAND, "");
}

void SyncPlayback::stopSyncPlaybackRecord()
{
    DPRINT(GUI_SYNC_PB_MEDIA, "stop sync playback");
    createStreamCommand(STOP_SYNCPLABACK_STREAM_COMMAND, "");
}

void SyncPlayback::stepSyncPlaybackRecord(SYNC_PLAYBACK_STEP_DIRECTION_e stepDirection)
{
    quint8 fieldIdx = 0;
    QString payloadString;
    STREAM_COMMAND_TYPE_e streamCommandType = (stepDirection == STEP_FORWARD ? STEPFORWARD_SYNCPLABACK_STREAM_COMMAND : STEPBACKWARD_SYNCPLABACK_STREAM_COMMAND);

    DPRINT(GUI_SYNC_PB_MEDIA, "step sync playback: [cameraMask=0x%llx & 0x%llx], [cameraCnt=%d], [recordType=0x%x], [direction=%d]",
           m_cameraValueForPlayRecord.bitMask[0], m_cameraValueForPlayRecord.bitMask[1], m_selectedCamCnt, m_recordValueForSearch, stepDirection);

    m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, m_cameraValueForPlayRecord.bitMask[0]);
    m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, stepDirection);
    m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, (m_calender->getDateMonthYearString() + m_timeLine->getTimeString()));
    m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, PB_SPEED_NORMAL);
    m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, m_recordValueForSearch);
    m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, m_audioValueForPlayRecord.bitMask[0]);
    m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, m_moveCount);
    m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, m_cameraValueForPlayRecord.bitMask[1]);
    m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, m_audioValueForPlayRecord.bitMask[1]);
    m_payloadLib->setCnfgArrayAtIndex(fieldIdx++, (m_isSliderPositionChangedbyUser ? 1 : 0)); /* It is used internally */

    payloadString = m_payloadLib->createDevCmdPayload(fieldIdx);
    if(createStreamCommand(streamCommandType, payloadString) == true)
    {
        if(m_isSliderPositionChangedbyUser)
        {
            m_isRequestPendingForSliderPositionChanged = true;
        }
    }
}

void SyncPlayback::sendSyncPlaybackAudioCommand()
{
    createStreamCommand(AUDIO_SYNCPLABACK_STREAM_COMMAND, "");
}

void SyncPlayback::clearSyncPlaybackRecord()
{
    createStreamCommand(CLEAR_SYNCPLABACK_STREAM_COMMAND, "");
}

void SyncPlayback::deviceDisconnectNotify (QString deviceName)
{
	if(m_currentDeviceName == deviceName)
	{
		/* Searching month records required on date change in different month */
		m_isSearchDateRequired = true;
		clearSyncPlaybackRecord();
	}
}

void SyncPlayback::processStopSyncPlaybackAction()
{
    if(m_currentZoomState == SYNC_PLAYBACK_ZOOM_IN)
    {
        exitFromZoomAction();
    }
    if(m_currentMode == FULL_MODE)
    {
        changeMode(NORMAL_MODE);
    }

    stopClipMaking();

    for(quint16 index = 0; (index < m_selectedCamCnt) && (index < MAX_CHANNEL_FOR_SEQ); index++)
    {
        if(Layout::streamInfoArray[MAIN_DISPLAY][index].m_videoType == VIDEO_TYPE_SYNCPLAYBAKSTREAM)
        {
            Layout::changeStreamStateOfWindow(MAIN_DISPLAY,
                                              index,
                                              m_currentDeviceName,
											  Layout::getCameraId(index),
                                              Layout::streamInfoArray[MAIN_DISPLAY][index].m_streamId,
                                              VIDEO_TYPE_SYNCPLAYBAKSTREAM,
                                              VIDEO_STATUS_NONE,
                                              VIDEO_ERROR_NONE);
            Layout::streamInfoArray[MAIN_DISPLAY][index].m_windowId = index;
        }
    }

    m_hoursRadioButtion[HOUR_24]->takeEnterKeyAction();
    m_toolbar->resetToolbar();
    m_timeLine->changeTime(QTime(0,0,0));
    m_playBackSpeed = PB_SPEED_NORMAL;
    m_lastFrameSecond = 0;
    m_audCamIndexAfterClntAud = MAX_CHANNEL_FOR_SEQ;
    memset(&m_audioValueForPlayRecord, 0, sizeof(m_audioValueForPlayRecord));
    changeCurrentPlaybackState(SYNC_PLAYBACK_STOP_STATE);
}

void SyncPlayback::ClearSyncPlaybackWindow()
{
    for(quint16 index = 0; index < MAX_SYNC_PB_SESSION; index++)
    {
        if(Layout::streamInfoArray[MAIN_DISPLAY][index].m_videoType == VIDEO_TYPE_SYNCPLAYBAKSTREAM)
        {
            Layout::changeStreamStateOfWindow(MAIN_DISPLAY,
                                              index,
                                              "",
                                              INVALID_CAMERA_INDEX,
                                              MAX_STREAM_SESSION,
                                              VIDEO_TYPE_NONE,
                                              VIDEO_STATUS_NONE,
                                              VIDEO_ERROR_NONE);
            Layout::streamInfoArray[MAIN_DISPLAY][index].m_windowId = index;
        }
    }
}

void SyncPlayback::processClearSyncPlaybackAction()
{
	if(m_currentZoomState == SYNC_PLAYBACK_ZOOM_IN)
	{
		exitFromZoomAction();
	}

	if(m_currentMode == FULL_MODE)
	{
		changeMode(NORMAL_MODE);
	}

	stopClipMaking();
    clearRecords();
    m_currentDeviceName = m_applController->GetRealDeviceName(m_deviceNameDropDownBox->getCurrValue());
    m_cameraNameList.clear();
    m_isDeviceConnected = m_applController->GetAllCameraNamesOfDevice(m_currentDeviceName, m_cameraNameList,PLAYBACK_CAM_LIST_TYPE,&m_cameraIndex[0]);

    if((m_cameraNameList.length() > 0) && (m_cameraNameList.contains ("")))
	{
        m_cameraNameList.clear();
	}
	else
	{
        /* Search for current month recording when change in camera search parametrs when required */
        if(m_isSearchDateRequired == true)
		{
			searchDatesHavingRecord();
		}
	}
	ResponceOnDeviceDisconnected();
}

void SyncPlayback::clearRecords()
{
    /* Clear all sync playback windows */
    ClearSyncPlaybackWindow();

	/* Disable CamCheckBox */
	for(quint8 index = 0; index <= m_cameraListCount; index++)
	{
		/* Disable CamCheckBox */
		m_cameraCheckbox[index]->changeState(OFF_STATE);
	}

	/* Reset SelectedCamCnt */
	m_selectedCamCnt = 0;

	/* Reset Selected Camera list */
    memset(&m_cameraValueForPlayRecord, 0, sizeof(m_cameraValueForPlayRecord));

    /* Reset toolbar so that all the icons get back to its default state */
    m_toolbar->resetToolbar();

	/* Disable play button explicitly when camera search parameter changed and
	 * camera selected but instead of play we again change camera seach parameter */
	m_toolbar->changeButtonEnableState(PLAY_BUTTON,false);

    /* playback speed to be made default state as it is not initialized to default state on play button pressed */
    m_playBackSpeed = PB_SPEED_NORMAL;
    m_audCamIndexAfterClntAud = MAX_CHANNEL_FOR_SEQ;
    memset(&m_audioValueForPlayRecord, 0, sizeof(m_audioValueForPlayRecord));
    m_syncPlaybackStreamId = MAX_STREAM_SESSION;
    m_isFirstTimePlay = true;
    m_isSliderPositionChangedbyUser = false;
    m_isRequestPendingForSliderPositionChanged = false;
    m_windowIndexForAudio = MAX_CHANNEL_FOR_SEQ;
    m_windowIndexForAudioOld = MAX_CHANNEL_FOR_SEQ;

    /* We do not need to clear the highlighted calendar dates having recording when month not changed*/
    if(m_isSearchDateRequired == true)
    {
        m_calender->changeDateCollection(0);
    }

	/* Reset Timeline span to 24Hrs */
	m_hoursRadioButtion[HOUR_24]->takeEnterKeyAction();

	/* Reset Timeline */
	m_timeLine->changeTime(QTime(0,0,0));
    m_timeLine->resetRecordsOnTimeline();

	/* Reset the SelectedWindow */
	m_currentWindowIndex = 0;
    m_prevSelectedWindowIndex = m_currentWindowIndex;
	Layout::selectWindow(MAIN_DISPLAY, m_currentWindowIndex);

	/* Reset Layout back to 1x1_PB */
	if(m_currentLayout != ONE_X_ONE_PLAYBACK)
	{
		changeLayout(ONE_X_ONE_PLAYBACK);
	}

	/*Reset Previous Layout to 1x1_PB*/
	m_prevLayout = ONE_X_ONE_PLAYBACK;
    changeCurrentPlaybackState(SYNC_PLAYBACK_STOP_STATE);
}

void SyncPlayback::startClipMaking()
{
    /* m_cropAndBackupData is a list which stores upto 64 clips untill sync pb page is not closed or the recording is not remove from clip list page*/
	if(m_cropAndBackupData.length() < MAX_CLIP)
	{
		changeCurrentPlaybackState(SYNC_PLAYBACK_CROPANDBACKUP_STATE);
		m_timeLine->showCropAndBackupDataLines();
		m_toolbar->changeButtonState(CROP_AND_BACKUP_BUTTON, STATE_2);
        for(quint8 index = 0; index < m_selectedCamCnt; index++)
		{
            quint8 cameraIndex = Layout::getCameraId(index);
			CROP_AND_BACKUP_DATA_t data;
			data.cameraIndex = cameraIndex;
			data.startTime = QDateTime::fromString((m_calender->getDateMonthYearString() + m_timeLine->getTimeString()), "ddMMyyyyhhmmss");
			data.endTime = QDateTime::fromString((m_calender->getDateMonthYearString() + m_timeLine->getTimeString()), "ddMMyyyyhhmmss");
			data.exportStatus = EXPORT_STATUS_NOT_EXPORTED;
			m_cropAndBackupData.append(data);
		}

        MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SYNC_PB_CLIP_MAKE_STARTED));
	}
	else
	{
		MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SYNC_PB_DEL_CLIP));
	}
}

void SyncPlayback::stopClipMaking()
{
    if(m_currentPlaybackState != SYNC_PLAYBACK_CROPANDBACKUP_STATE)
	{
        return;
    }

    quint16 startTime = 0;
    quint16 endTime = 0;
    qint8 index = 0;
    quint8 recordAvailable = 0;

    /* prevClipCnt indicates previous available clip records in the list before pressing clip export */
    quint8 prevClipCnt = (m_cropAndBackupData.length() - m_selectedCamCnt);

    for(index = prevClipCnt; index < m_cropAndBackupData.length(); index++)
    {
        CROP_AND_BACKUP_DATA_t data = m_cropAndBackupData.value(index);

        data.endTime = QDateTime::fromString((m_calender->getDateMonthYearString() + m_timeLine->getTimeString()), "ddMMyyyyhhmmss");

        startTime = ((data.startTime.time().hour() * 60) +  data.startTime.time().minute());
        endTime = ((data.endTime.time().hour() * 60) +  data.endTime.time().minute());
        if (data.endTime == data.startTime)
        {
            /* index is decremented so that the next camera is not skipped */
            m_cropAndBackupData.removeAt(index--);
            continue;
        }

        if(data.endTime > data.startTime)
        {
            m_cropAndBackupData.replace(index, data);
        }
        else
        {
            quint16 t_temp = endTime;
            QDateTime temp = data.endTime;
            data.endTime = data.startTime;
            data.startTime = temp;
            m_cropAndBackupData.replace(index, data);

            /* EndTime and startTime are passed as parameter to check current time recording and hence they also need to be swapped */
            endTime = startTime;
            startTime = t_temp;
        }

         //Check in timeframe records not available then remove from the List
        if((startTime <= RECORDS_IN_MIN) && (endTime <= RECORDS_IN_MIN))
        {
            if (false == m_applController->IsCamRecAvailable(((data.cameraIndex) - 1), startTime, endTime))
            {
                m_cropAndBackupData.removeAt(index--);
                continue;
            }

            recordAvailable++;
        }
    }

    changeCurrentPlaybackState(m_previousPlaybackState);
    m_timeLine->resetCropAndBackupDataLines();
    m_toolbar->changeButtonState(CROP_AND_BACKUP_BUTTON, STATE_1);
    MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SYNC_PB_CLIP_MAKE_STOPPED));

    /* If we have already stored cameras and its addition with new camers to be stored  exceeds the limit of 64 clips then we remove all those cameras which are to be newly added*/
    if((prevClipCnt + recordAvailable) > MAX_CLIP)
    {
        for(quint8 index = 0; index < recordAvailable; index++)
        {
            m_cropAndBackupData.removeLast();
        }

        DPRINT(GUI_SYNC_PB_MEDIA, "exceeding limit of clip making with list: [length=%d], [newRecords=%d]", m_cropAndBackupData.length(), recordAvailable);
        MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SYNC_PB_DEL_CLIP));
    }
}

void SyncPlayback::setLayoutWindowArea()
{
	quint32 width, height;
	quint8 layout_window_parts_root=MAX_LAYOUT;
	quint8 window_area_index=0;

	switch(m_currentLayout)
	{
		case ONE_X_ONE:
		case ONE_X_ONE_PLAYBACK:
			layout_window_parts_root=1;
			break;

		case TWO_X_TWO:
		case TWO_X_TWO_PLAYBACK:
			layout_window_parts_root=2;
			break;

		case THREE_X_THREE:
		case THREE_X_THREE_PLAYBACK:
			layout_window_parts_root=3;
			break;

		case FOUR_X_FOUR:
		case FOUR_X_FOUR_PLAYBACK:
			layout_window_parts_root=4;
			break;

		default :
			break;
	}

	/* Calculate Width & Height avialable for Each Layout Window */
	if(m_currentMode == FULL_MODE)
	{
		width = ( this->width() / layout_window_parts_root);
		height = (this->height() / layout_window_parts_root);
	}
	else
	{
		width = (( this->width() - m_mainRightRectangle->width()) / layout_window_parts_root);
		height = ((this->height()  - m_mainBottomRectangle->height()) / layout_window_parts_root);
	}

	/* Create Layout Window Area and set Geomertry */
	for(quint8 Var_Row = 0; Var_Row < layout_window_parts_root; Var_Row++)
	{
		for(quint8 Var_Col = 0; Var_Col < layout_window_parts_root; Var_Col++)
		{
			window_area_index= Var_Col + (Var_Row*layout_window_parts_root) ;
			m_windowArea[window_area_index].setRect(width*Var_Col, height*Var_Row, width, height);
		}
	}
	/* Reset Geometry for other windows which are not visible in current Layout */
	for(++window_area_index; window_area_index < MAX_SYNC_PB_SESSION; window_area_index++)
	{
		m_windowArea[window_area_index].setRect(0,0,0,0);
	}
}

void SyncPlayback::enterInZoomAction()
{
    if(m_zoomFeatureControl == NULL)
    {
        m_modeBeforeZoomAction = m_currentMode;

        m_toolbar->changeButtonState(ZOOM_BUTTON, STATE_2);
        m_toolbar->changeButtonEnableState(CHANGE_MODE_BUTTON, false);
        m_toolbar->changeButtonEnableState(LAYOUT_BUTTON, false);

        if(m_currentMode != FULL_MODE)
        {
            changeMode(FULL_MODE);
        }
        if(m_currentLayout != ONE_X_ONE)
        {
            changeLayout(ONE_X_ONE);
        }
        m_currentZoomState = SYNC_PLAYBACK_ZOOM_IN;
        /* PARASOFT: Memory Deallocated in slot ExitFromZoomFeature */
        m_zoomFeatureControl = new ZoomFeatureControl(m_currentDeviceName,
                                                      m_currentWindowIndex,
                                                      this,
                                                      false,
                                                      true,
                                                      (this->height() - TOOLBAR_FULL_MODE_HEIGHT));
        connect(m_zoomFeatureControl,
                SIGNAL(destroyed()),
                this,
                SLOT(slotExitFromZoomFeature()));
    }
}

void SyncPlayback::exitFromZoomAction()
{
    if(m_zoomFeatureControl != NULL)
    {
        m_zoomFeatureControl->exitAction();
    }
}

void SyncPlayback::takeLeftKeyAction()
{
    bool status = true;
    do
    {
        if(m_currentElement == SYNC_PLAYBACK_CLOSEBUTTON)
        {
            m_currentElement = (SYNC_PLAYBACK_TIMELINE);
        }

        if(m_currentElement == SYNC_PLAYBACK_SEARCH_CONTROLBUTTON)
        {
            m_currentElement = SYNC_PLAYBACK_RECORDINGTYPE_CHECKBOX;
        }
        else if((m_currentElement >= SYNC_PLAYBACK_CAMERASELECT_CHECKBOX) && (m_currentElement < SYNCPB_SCROLLBAR))
        {
             m_currentElement = SYNC_PLAYBACK_SEARCH_CONTROLBUTTON;
        }
        else if(m_currentElement)
        {
            m_currentElement = (m_currentElement - 1);
        }
        else
        {
            // pass key to parent
            status = false;
            break;
        }
    }while((m_elementList[m_currentElement] == NULL) || (!m_elementList[m_currentElement]->getIsEnabled()));

    if(status == true)
    {
        if(((m_currentElement == SYNC_PLAYBACK_CALENDER) && (m_calender->getIsEnabled()))
                || (m_currentElement == SYNC_PLAYBACK_TOOLBAR) || (m_currentElement == SYNC_PLAYBACK_TIMELINE))
        {
            m_elementList[m_currentElement]->forceFocusToPage(false);
        }
        else
        {
            m_elementList[m_currentElement]->forceActiveFocus();
        }
    }
}

void SyncPlayback::takeRightKeyAction()
{
    bool status = true;
    do
    {
        if(m_currentElement == (MAX_SYNC_PLAYBACK_ELEMENT - 1))
        {
            m_currentElement = SYNC_PLAYBACK_CLOSEBUTTON;
        }

        if((m_currentElement >= SYNC_PLAYBACK_CAMERASELECT_CHECKBOX) && (m_currentElement < SYNCPB_SCROLLBAR))
        {
            m_currentElement = SYNC_PLAYBACK_TOOLBAR;
        }
        else if(m_currentElement != (MAX_SYNC_PLAYBACK_ELEMENT - 1))
        {
            m_currentElement = (m_currentElement + 1);
        }
        else
        {
            status = false;
            break;
        }

    }while((m_elementList[m_currentElement] == NULL) || (!m_elementList[m_currentElement]->getIsEnabled()));

    if(status == true)
    {
        if((m_currentElement == SYNC_PLAYBACK_CALENDER) || (m_currentElement == SYNC_PLAYBACK_TOOLBAR) || (m_currentElement == SYNC_PLAYBACK_TIMELINE))
        {
            m_elementList[m_currentElement]->forceFocusToPage(true);
        }
        else
        {
            m_elementList[m_currentElement]->forceActiveFocus();
        }
    }
}

void SyncPlayback::takeUpKeyAction()
{
    quint8 availableCameras = m_cameraListCount;

    if((m_currentElement > SYNC_PLAYBACK_CAMERASELECT_CHECKBOX) && (m_currentElement <= (SYNC_PLAYBACK_CAMERASELECT_CHECKBOX + availableCameras)))
    {
        bool status = true;
        do
        {
            if(m_currentElement > 0)
            {
                m_currentElement = (m_currentElement - 1);
            }
            else
            {
                status = false;
                break;
            }

            if((m_currentElement > SYNC_PLAYBACK_CAMERASELECT_CHECKBOX) && (m_currentElement < (SYNC_PLAYBACK_CAMERASELECT_CHECKBOX + m_firstCameraIndex + 1)))
            {
                status = true;
                break;
            }

        }while((m_elementList[m_currentElement] == NULL) || (!m_elementList[m_currentElement]->getIsEnabled()));

        if(status == true)
        {
            if(m_currentElement == (SYNC_PLAYBACK_CAMERASELECT_CHECKBOX + m_firstCameraIndex))
            {
                if((IS_VALID_OBJ(m_syncScrollbar)) && (m_syncScrollbar->getIsEnabled()))
                {
                     m_syncScrollbar->updateBarGeometry(-1);
                     if(IS_VALID_OBJ(m_elementList[m_currentElement]))
                     {
                         m_elementList[m_currentElement]->forceActiveFocus();
                     }
                }
            }
            else
            {
                m_elementList[m_currentElement]->forceActiveFocus();
            }
        }
    }
}

void SyncPlayback::takeDownKeyAction()
{
    quint8 availableCameras = m_cameraListCount;

    if((m_currentElement >= SYNC_PLAYBACK_CAMERASELECT_CHECKBOX) && (m_currentElement < (SYNC_PLAYBACK_CAMERASELECT_CHECKBOX + availableCameras)))
    {
        bool status = true;
        do
        {
            if(m_currentElement != (MAX_SYNC_PLAYBACK_ELEMENT - 1))
            {
                m_currentElement = (m_currentElement + 1);
            }
            else
            {
                status = false;
                break;
            }

            if(m_currentElement > (SYNC_PLAYBACK_CAMERASELECT_CHECKBOX + m_lastCameraIndex + 1))
            {
                status = true;
                break;
            }
        }while((m_elementList[m_currentElement] == NULL) || (!m_elementList[m_currentElement]->getIsEnabled()));

        if(status == true)
        {
            if(m_currentElement == (SYNC_PLAYBACK_CAMERASELECT_CHECKBOX + m_lastCameraIndex + 2))
            {
                if((IS_VALID_OBJ(m_syncScrollbar)) && (m_syncScrollbar->getIsEnabled()))
                {
                     m_syncScrollbar->updateBarGeometry(1);
                     if(IS_VALID_OBJ(m_elementList[m_currentElement]))
                     {
                         m_elementList[m_currentElement]->forceActiveFocus();
                     }
                }
            }
            else
            {
                m_elementList[m_currentElement]->forceActiveFocus();
            }
        }
    }
}

void SyncPlayback::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();

    if ((m_currentMode != FULL_MODE) && (m_toolbar->isVisible() == true))
    {
        takeLeftKeyAction();
    }
}

void SyncPlayback::tabKeyPressed(QKeyEvent *event)
{
    event->accept();

    if ((m_currentMode != FULL_MODE) && (m_toolbar->isVisible() == true))
    {
        takeRightKeyAction();
    }
}

void SyncPlayback::navigationKeyPressed(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_Left:
            event->accept();
            if ((m_currentMode == FULL_MODE) && (m_toolbar->isVisible() == false))
            {
                m_toolbar->takeButtonEnterKeyAction(SLOW_PLAY_BUTTON);
            }
            break;

        case Qt::Key_Right:
            event->accept();
            if ((m_currentMode == FULL_MODE) && (m_toolbar->isVisible() == false))
            {
                m_toolbar->takeButtonEnterKeyAction(FAST_PLAY_BUTTON);
            }
            break;

        case Qt::Key_Up:
            event->accept();
            if ((m_currentMode == FULL_MODE) && (m_toolbar->isVisible() == false))
            {
                m_toolbar->takeButtonEnterKeyAction(STOP_BUTTON);
            }
            else
            {
                takeUpKeyAction();
            }
            break;

        case Qt::Key_Down:
            event->accept();
            if ((m_currentMode == FULL_MODE) && (m_toolbar->isVisible() == false))
            {
                m_toolbar->takeButtonEnterKeyAction(NEXT_FRAME_BUTTON);
            }
            else
            {
                takeDownKeyAction();
            }
            break;

        default:
            event->accept();
            break;
    }
}

void SyncPlayback::insertKeyPressed(QKeyEvent *event)
{
    event->accept();
    if(m_currentMode == FULL_MODE)
    {
        if(m_toolbar->isVisible())
        {
            m_toolbar->hide();
            this->setFocus();
        }
        else
        {
            m_toolbar->show();
        }
    }
}

void SyncPlayback::enterKeyPressed(QKeyEvent *event)
{
    event->accept();
    if ((m_currentMode == FULL_MODE) && (m_toolbar->isVisible() == false))
    {
        m_toolbar->takeButtonEnterKeyAction(PLAY_BUTTON);
    }
}

void SyncPlayback::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    m_currentElement = SYNC_PLAYBACK_CLOSEBUTTON;
    m_elementList[m_currentElement]->forceActiveFocus();
}

void SyncPlayback::functionKeyPressed(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_F4:
            event->accept();
            if(m_toolbar->isVisible() == false)
            {
                m_toolbar->setVisible(true);
            }
            break;

          default:
            event->accept();
            break;
    }
}

void SyncPlayback::mouseMoveEvent(QMouseEvent * event)
{
	if(m_currentMode == FULL_MODE)
	{
		if((event->y() >= (this->height() - TOOLBAR_FULL_MODE_HEIGHT)) && (event->y() <= this->height()))
		{
			if(m_toolbar->isVisible() == false)
			{
				m_toolbar->setVisible(true);
			}
		}
		else
		{
			/* Do not Hide Toolbar if Layout List is Visible */
			if((m_toolbar->isVisible() == true) && (false == m_toolbar->isLayoutListVisible()))
			{
				m_toolbar->setVisible(false);
				this->setFocus();
			}
		}
	}
	event->accept();
}

void SyncPlayback::mousePressEvent(QMouseEvent * event)
{
	quint16 windowIndex = MAX_CHANNEL_FOR_SEQ;

	if(!m_syncPlaybackLoadingText->isVisible())
	{
        if((m_toolbar->isVisible()) && (m_toolbar->geometry().contains (event->pos())))
		{
            event->accept();
			return;
		}
		else
		{
			/* Hide LayoutList on mouse clicked in any other position in bottom rectangle */
			m_toolbar->setLayoutListVisiblity(false);
		}

		for(quint8 index = 0; index < MAX_SYNC_PB_SESSION; index++)
		{
			if(m_windowArea[index].contains(event->pos()))
			{
				windowIndex = index;
				break;
			}
		}

		/* Do not allow the user to select windows that are not allocated with any cameras */
		if(windowIndex >= m_selectedCamCnt)
		{
            event->accept();
			return;
		}

		if (m_currentLayout == ONE_X_ONE || m_currentLayout == ONE_X_ONE_PLAYBACK)
		{
			if(m_prevSelectedWindowIndex != MAX_CHANNEL_FOR_SEQ)
			{
				/* Re-Store current WindowIndex */
				m_currentWindowIndex = m_prevSelectedWindowIndex;
				Layout::selectWindow(MAIN_DISPLAY, m_prevSelectedWindowIndex);

				/* Show CamRec on Timeline for current selected window */
				m_timeLine->showRecordsOnTimeline(Layout::getCameraId(m_prevSelectedWindowIndex));
			}
		}
		else
		{
			/* Store current WindowIndex */
			m_currentWindowIndex = windowIndex;
			m_prevSelectedWindowIndex = m_currentWindowIndex;
			Layout::selectWindow(MAIN_DISPLAY, windowIndex);

			/* Show CamRec on Timeline for current selected window */
			m_timeLine->showRecordsOnTimeline(Layout::getCameraId(windowIndex));
		}

		changeAudioStateOfAudioButton();
	}
	event->accept();
}

void SyncPlayback::mouseDoubleClickEvent(QMouseEvent *event)
{
	quint16 windowIndex = MAX_CHANNEL_FOR_SEQ;

	if(!m_syncPlaybackLoadingText->isVisible())
	{
        if((m_toolbar->isVisible()) && (m_toolbar->geometry().contains (event->pos())))
		{
            event->accept();
			return;
		}

		for(quint8 index = 0; index < MAX_SYNC_PB_SESSION; index++)
		{
			if(m_windowArea[index].contains(event->pos()))
			{
				windowIndex = index;
				break;
			}
		}

		/* Do not allow the user to select windows that are not allocated with any cameras */
		if(windowIndex >= m_selectedCamCnt)
		{
            event->accept();
			return;
		}

		m_currentWindowIndex = windowIndex;

		if(m_currentMode == FULL_MODE)
		{
			if (m_currentLayout == ONE_X_ONE)
			{
				/* Re-Store current Layout in case of Layout is other than ONE_X_ONE */
				if (m_prevLayout != ONE_X_ONE)
				{
					changeLayout(m_prevLayout);
				}
			}
			else
			{
				/* Store current Layout and Change Layout to ONE_X_ONE */
				m_prevLayout = m_currentLayout;
				changeLayout(ONE_X_ONE);
			}
		}
		else
		{
			if (m_currentLayout == ONE_X_ONE_PLAYBACK)
			{
				/* Re-Store current Layout in case of Layout is other than ONE_X_ONE_PLAYBACK */
				if (m_prevLayout != ONE_X_ONE_PLAYBACK)
				{
					changeLayout(m_prevLayout);
				}
			}
			else
			{
				/* Store current Layout and Change Layout to ONE_X_ONE_PLAYBACK */
				m_prevLayout = m_currentLayout;
				changeLayout(ONE_X_ONE_PLAYBACK);
			}
		}
	}
	event->accept();
}

void SyncPlayback::wheelEvent(QWheelEvent * event)
{
	if((m_syncScrollbar != NULL)
			&& (event->x() >= RIGHT_PANEL_LEFT_MARGIN)
			&& (event->y() >= m_syncScrollbar->y())
			&& (event->y() <= (m_syncScrollbar->y() + m_syncScrollbar->height()))
			&& (m_syncScrollbar->getIsEnabled() == true)
			&& (m_syncScrollbar->isVisible() == true)
			&& (m_syncPlaybackLoadingText->isVisible() == false))
	{
		m_syncScrollbar->wheelEvent(event);
	}
	else
	{
		QWidget::wheelEvent(event);
	}
}

void SyncPlayback::setClientAudioRunningFlag(bool state)
{
    m_isClientAudioProcessing = state;
}

bool SyncPlayback::getSyncPBAudPlayStatus()
{
    return ((m_windowIndexForAudio == MAX_CHANNEL_FOR_SEQ)? (false) : (true));
}

void SyncPlayback::stopCamAudAfterClntAud()
{
    m_isClientAudioProcessing = true;
    m_audCamIndexAfterClntAud = m_windowIndexForAudio;
    if((m_audCamIndexAfterClntAud != MAX_CHANNEL_FOR_SEQ) && (!m_isSyncRequestPending))
    {
        memset(&m_audioValueForPlayRecord, 0, sizeof(m_audioValueForPlayRecord));
        MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(TWO_WAY_PLYBK_AUD_STP_PRO_CLNT_AUD));
        m_isAudioCommandSend = true;
        startSyncPlaybackRecord();
    }
}

void SyncPlayback::startCamAudAfterClntAud()
{
    m_isClientAudioProcessing = false;
    if((m_audCamIndexAfterClntAud != MAX_CHANNEL_FOR_SEQ) && (!m_isSyncRequestPending))
    {
        quint16 cameraIndex = Layout::streamInfoArray[MAIN_DISPLAY][m_audCamIndexAfterClntAud].m_cameraId - 1;
        memset(&m_audioValueForPlayRecord, 0, sizeof(m_audioValueForPlayRecord));
        SET_CAMERA_MASK_BIT(m_audioValueForPlayRecord, cameraIndex);
        m_windowIndexForAudio = m_audCamIndexAfterClntAud;
        m_isAudioCommandSend = true;
        m_audCamIndexAfterClntAud = MAX_CHANNEL_FOR_SEQ;
        startSyncPlaybackRecord();
    }
}

void SyncPlayback::slotCloseButtonClicked(int)
{
    DPRINT(GUI_SYNC_PB_MEDIA, "close button clicked");
    /* When close button pressed GUI parameters handling not required as constructor initialize them again whenever syncplayback page opened again */
    /* As we are closing the page we do not require to search the recording for the month again */
    m_isSearchDateRequired = false;

	/* Clear sync playback windows*/
    ClearSyncPlaybackWindow();
    m_isCloseButtonClicked = true;
    m_syncPlaybackLoadingText->setVisible(true);
    clearSyncPlaybackRecord();
}

void SyncPlayback::slotUpdateCurrentElement(int index)
{
	/* Hide LayoutList on Device or Source Option clicked */
    if(SYNC_PLAYBACK_TOOLBAR != index)
	{
		m_toolbar->setLayoutListVisiblity(false);
	}
	m_currentElement = index;
}

void SyncPlayback::slotDeviceNameChanged(QString string, quint32)
{
    string = m_applController->GetRealDeviceName(string);
    if(m_currentDeviceName == string)
	{
        return;
    }

    DEV_TABLE_INFO_t devTableInfo;
    if(m_applController->GetDeviceConnectionState (string) == CONFLICT)
    {
        QString msgStr = "Firmware mismatch detected. Please upgrade firmware";
        if(m_applController->GetDeviceInfo (string, devTableInfo))
        {
            if(devTableInfo.deviceConflictType == MX_DEVICE_CONFLICT_TYPE_SERVER_NEW)
            {
                msgStr = "Firmware mismatch detected. Please upgrade network device firmware";
            }
        }
        MessageBanner::addMessageInBanner(msgStr);
    }

    /* Hide Camera List */
    HideCameraList();

    /* Searching month records as the recording in month indication for same month can be different */
    m_isSearchDateRequired = true;
    clearSyncPlaybackRecord();
}

void SyncPlayback::slotRecDriveDropBoxChanged(QString, quint32)
{
    /* Hide Camera List */
    HideCameraList();

    /* Searching month records required as the recording in month indication for same month can be different */
    m_isSearchDateRequired = true;
	clearSyncPlaybackRecord();
}

void SyncPlayback::slotRecTypeCheckboxClicked(OPTION_STATE_TYPE_e , int )
{
    /* Hide Camera List */
    HideCameraList();

    /* Searching month records not required as it does not affect the date parameter*/
    m_isSearchDateRequired = true;
	clearSyncPlaybackRecord();
}

void SyncPlayback::slotFocusChangedFromCurrentElement(bool isPreviousElement)
{
    if(isPreviousElement)
    {
        takeLeftKeyAction();
    }
    else
    {
        takeRightKeyAction();
    }
}

void SyncPlayback::slotFetchNewSelectedDate()
{
    /* Searching month records required on date change in different month */
    m_isSearchDateRequired = true;
	clearSyncPlaybackRecord();
}

void SyncPlayback::slotFetchRecordForNewDate()
{
    /* Hide Camera List */
    HideCameraList();

    /* Searching month records not required on date change in same month */
    m_isSearchDateRequired = true;
	clearSyncPlaybackRecord();
}

void SyncPlayback::slotSearchButtonClicked(int)
{
	/* Hide Camera List */
	HideCameraList();

    /* Disable search button */
    m_searchButton->setIsEnabled(false);

	/* Search CameraList for which CamRec is available for selected date */
    searchHoursHavingRecord();
}

void SyncPlayback::slotCameraCheckboxClicked(OPTION_STATE_TYPE_e iButtonState, int indexInPage)
{
	LAYOUT_TYPE_e tLayout;

	if(indexInPage == SYNC_PLAYBACK_CAMERASELECT_CHECKBOX)
	{
		/* Handle CamCheckBox(Selecte All) Click Event */
		selectDeselectAllRecords(iButtonState);
	}
	else
	{
		/* Handle CamCheckBox(Other than Select All) Click Event */
        selectDeselectCameraCheckBox(iButtonState, indexInPage);
	}
    DPRINT(GUI_SYNC_PB_MEDIA, "camera checkbox clicked: [camera=%d], [cameraMask=0x%llx & 0x%llx], [cameraCnt=%d] [buttonState=%d]",
           (indexInPage - SYNC_PLAYBACK_CAMERASELECT_CHECKBOX), m_cameraValueForPlayRecord.bitMask[0],
            m_cameraValueForPlayRecord.bitMask[1], m_selectedCamCnt, iButtonState);

	if(m_selectedCamCnt >= MAX_SYNC_PB_SESSION)
	{
		/* Disable other camera checknox which is in OFF state */
		for(quint8 index = 1; index <= m_cameraListCount; index++)
		{
			if(m_cameraCheckbox[index]->getCurrentState() == OFF_STATE)
			{
				m_cameraCheckbox[index]->setIsEnabled(false);
			}
            else
            {
                m_cameraCheckbox[index]->setIsEnabled(true);
            }
		}
	}
	else if((m_selectedCamCnt == 0) || (m_selectedCamCnt == (MAX_SYNC_PB_SESSION - 1)))
	{
		/* Enable other camera checknox which is in OFF state */
		for(quint8 index = 1; index <= m_cameraListCount; index++)
		{
			if(m_cameraCheckbox[index]->getCurrentState() == OFF_STATE)
			{
				m_cameraCheckbox[index]->setIsEnabled(true);
			}
		}
	}

	/* Make current window index to default, if window reallocation affects current selected window */
	if(m_selectedCamCnt <= m_currentWindowIndex)
	{
		/* Clear the window index to default */
		m_currentWindowIndex = 0;
        m_prevSelectedWindowIndex = m_currentWindowIndex;

		/* Making selected window to default */
		Layout::selectWindow(MAIN_DISPLAY, m_currentWindowIndex);
        m_timeLine->showRecordsOnTimeline(Layout::getCameraId(m_currentWindowIndex));
	}

	/* Change Layout based on SelectedCamCnt */
    if (m_selectedCamCnt > windowPerPage[THREE_X_THREE_PLAYBACK])
    {
        tLayout = FOUR_X_FOUR_PLAYBACK;
    }
    else if (m_selectedCamCnt > windowPerPage[TWO_X_TWO_PLAYBACK])
    {
        tLayout = THREE_X_THREE_PLAYBACK;
    }
    else if (m_selectedCamCnt > windowPerPage[ONE_X_ONE_PLAYBACK])
    {
        tLayout = TWO_X_TWO_PLAYBACK;
    }
    else
    {
        tLayout = ONE_X_ONE_PLAYBACK;
    }

	if(tLayout != m_currentLayout)
	{
		changeLayout(tLayout);
	}

	/* Enable PLAY Button based on SelectedCamCnt */
	bool isSetEnable = (m_selectedCamCnt == 0 ? false : true);
	m_toolbar->changeButtonEnableState(PLAY_BUTTON,isSetEnable);

	/* Disable search button */
	bool isSearchEnable = (m_selectedCamCnt != 0 ? false : true);
	m_searchButton->setIsEnabled(isSearchEnable);
}

void SyncPlayback::slotHourFormatChanged(OPTION_STATE_TYPE_e, int indexInPage)
{
    HOURS_FORMAT_TYPE_e newHourFormat = (HOURS_FORMAT_TYPE_e)(indexInPage - SYNC_PLAYBACK_HOURTYPE_RADIOBUTTON);
    if(m_currentPlaybackState == SYNC_PLAYBACK_CROPANDBACKUP_STATE)
    {
        stopClipMaking();
    }

    if(m_currentHourFormat != newHourFormat)
    {
        for(quint8 index = 0; index < MAX_HOUR_FORMAT_TYPE; index++)
        {
            if(index != (indexInPage - SYNC_PLAYBACK_HOURTYPE_RADIOBUTTON))
            {
                m_hoursRadioButtion[index]->changeState(OFF_STATE);
            }
        }
        m_currentHourFormat = newHourFormat;
        m_timeLine->changeHourFormat(m_currentHourFormat);
    }
}

void SyncPlayback::slotSliderPositionChanged()
{
    if((m_currentPlaybackState == SYNC_PLAYBACK_PLAY_STATE) || (m_currentPlaybackState == SYNC_PLAYBACK_REVERSEPLAY_STATE)
            || ((m_currentPlaybackState == SYNC_PLAYBACK_CROPANDBACKUP_STATE)
                /* Do not play the video if previously video is in pause state */
                && ((m_previousPlaybackState == SYNC_PLAYBACK_PLAY_STATE) || (m_previousPlaybackState == SYNC_PLAYBACK_REVERSEPLAY_STATE))))
    {
        m_moveCount++;
        startSyncPlaybackRecord();
    }
}

void SyncPlayback::slotSliderPositionChangedStart()
{
    m_isSliderPositionChangedbyUser = true;
}

void SyncPlayback::slotToolbarButtonClicked(SYNCPB_TOOLBAR_BUTTON_TYPE_e index, STATE_TYPE_e state)
{
    DPRINT(GUI_SYNC_PB_MEDIA, "toolbar button clicked: [buttonId=%d], [buttonState=%d]", index, state);
    switch(index)
    {
        case PLAY_BUTTON:
        {
            if (true == m_isSyncRequestPending)
            {
                break;
            }

            /* Stop clip making if state chnages from SYNC_PLAYBACK_CROPANDBACKUP_STATE to anty other state*/
            stopClipMaking();
            if(state == STATE_1)
            {
                m_playBackDirection = FORWARD_PLAY;
                m_playBackSpeed = PB_SPEED_NORMAL;
                m_toolbar->changeButtonState(REVERSE_PLAY_BUTTON, STATE_1);
                startSyncPlaybackRecord();
            }
            else
            {
                pauseSyncPlaybackRecord();
            }
        }
        break;

        case REVERSE_PLAY_BUTTON:
        {
            if (true == m_isSyncRequestPending)
            {
                break;
            }

            /* Stop clip making if state chnages from SYNC_PLAYBACK_CROPANDBACKUP_STATE to anty other state*/
            stopClipMaking();
            if(state == STATE_1)
            {
                m_playBackDirection = BACKWARD_PLAY;
                m_playBackSpeed = PB_SPEED_NORMAL;
                m_toolbar->changeButtonState(PLAY_BUTTON, STATE_1);
                startSyncPlaybackRecord();
            }
            else
            {
                pauseSyncPlaybackRecord();
            }
        }
        break;

        case STOP_BUTTON:
        {
            if (true == m_isSyncRequestPending)
            {
                break;
            }

            /* Searching month records required on date change in different month */
            m_isSearchDateRequired = true;
            clearSyncPlaybackRecord();
        }
        break;

        case SLOW_PLAY_BUTTON:
        {
            if (true == m_isSyncRequestPending)
            {
                break;
            }

            if(m_playBackSpeed == PB_SPEED_16S)
            {
                /* Disable SLOW_PLAY_BUTTON */
                m_toolbar->changeButtonEnableState(SLOW_PLAY_BUTTON, false);
                break;
            }

            m_playBackSpeed = PB_SPEED_e(m_playBackSpeed - 1);
            if ((m_playBackDirection == FORWARD_PLAY) && (m_playBackSpeed == PB_SPEED_4F))
            {
                /* Send SYNC_PLYBCK_RCD to receive Normal stream again */
                startSyncPlaybackRecord();
            }
            else if(m_playBackSpeed == PB_SPEED_8F)
            {
                /* Enable FAST_PLAY_BUTTON */
                m_toolbar->changeButtonEnableState(FAST_PLAY_BUTTON, true);
                /* Send SYNC_PLYBCK_RCD to receive I-Frames */
                startSyncPlaybackRecord();
            }
            else
            {
                if(m_playBackSpeed == PB_SPEED_16S)
                {
                    /* Disable SLOW_PLAY_BUTTON */
                    m_toolbar->changeButtonEnableState(SLOW_PLAY_BUTTON, false);
                }

                /* Don't Send SYNC_PLYBCK_RCD only set speed to SyncPb Feeder threads */
                SetSpeed();
            }
        }
        break;

        case FAST_PLAY_BUTTON:
        {
            if (true == m_isSyncRequestPending)
            {
                break;
            }

            if(m_playBackSpeed == PB_SPEED_16F)
            {
                /* Disable FAST_PLAY_BUTTON */
                m_toolbar->changeButtonEnableState(FAST_PLAY_BUTTON, false);
            }
            else
            {
                m_playBackSpeed = PB_SPEED_e(m_playBackSpeed + 1);
                if ((m_playBackDirection == FORWARD_PLAY) && (m_playBackSpeed == PB_SPEED_8F))
                {
                    /* Send SYNC_PLYBCK_RCD to receive I-Frames */
                    startSyncPlaybackRecord();
                }
                else if(m_playBackSpeed == PB_SPEED_16F)
                {
                    /* Disable FAST_PLAY_BUTTON */
                    m_toolbar->changeButtonEnableState(FAST_PLAY_BUTTON, false);
                    /* Send SYNC_PLYBCK_RCD to receive Alternate I-Frames */
                    startSyncPlaybackRecord();
                }
                else
                {
                    if(m_playBackSpeed == PB_SPEED_8S)
                    {
                        /* Enable FAST_PLAY_BUTTON */
                        m_toolbar->changeButtonEnableState(SLOW_PLAY_BUTTON, true);
                    }
                    /* Don't Send SYNC_PLYBCK_RCD only set speed to SyncPb Feeder threads */
                    SetSpeed();
                }
            }
        }
        break;

        case PREVIOUS_FRAME_BUTTON:
        {
            if (true == m_isSyncRequestPending)
            {
                break;
            }

            /* Stop clip making if state chnages from SYNC_PLAYBACK_CROPANDBACKUP_STATE to anty other state*/
            stopClipMaking();
            m_playBackSpeed = PB_SPEED_NORMAL;
            m_toolbar->changeButtonState(PLAY_BUTTON, STATE_1);
            m_toolbar->changeButtonState(REVERSE_PLAY_BUTTON, STATE_1);
            stepSyncPlaybackRecord(STEP_BACKWARD);
        }
        break;

        case NEXT_FRAME_BUTTON:
        {
            if (true == m_isSyncRequestPending)
            {
                break;
            }

            /* Stop clip making if state chnages from SYNC_PLAYBACK_CROPANDBACKUP_STATE to anty other state*/
            stopClipMaking();
            m_playBackSpeed = PB_SPEED_NORMAL;
            m_toolbar->changeButtonState(PLAY_BUTTON, STATE_1);
            m_toolbar->changeButtonState(REVERSE_PLAY_BUTTON, STATE_1);
            stepSyncPlaybackRecord(STEP_FORWARD);
        }
        break;

        case AUDIO_BUTTON:
        {
            if(m_isClientAudioProcessing == true)
            {
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(TWO_WAY_PLYBK_AUD_REQ_FAIL_PRO_CLNT_AUD));
                break;
            }

            if (true == m_isSyncRequestPending)
            {
                break;
            }

            if (Layout::streamInfoArray[MAIN_DISPLAY][m_currentWindowIndex].m_cameraId >= INVALID_CAMERA_INDEX)
            {
                break;
            }

            m_isAudioCommandSend = true;
            if(state == STATE_1)
            {
                //include audio for selected window camera
                quint16 cameraIndex = Layout::streamInfoArray[MAIN_DISPLAY][m_currentWindowIndex].m_cameraId - 1;
                memset(&m_audioValueForPlayRecord, 0, sizeof(m_audioValueForPlayRecord));
                SET_CAMERA_MASK_BIT(m_audioValueForPlayRecord, cameraIndex);
                m_windowIndexForAudio = m_currentWindowIndex;
                startSyncPlaybackRecord();
            }
            else
            {
                //exclude auido for selected window camera
                memset(&m_audioValueForPlayRecord, 0, sizeof(m_audioValueForPlayRecord));
                startSyncPlaybackRecord();
            }
        }
        break;

        case ZOOM_BUTTON:
        {
            if(state == STATE_1)
            {
                if(Layout::streamInfoArray[MAIN_DISPLAY][m_currentWindowIndex].m_cameraId != INVALID_CAMERA_INDEX)
                {
                    enterInZoomAction();
                    this->setFocus();
                }
            }
            else
            {
                exitFromZoomAction();
            }
        }
        break;

        case CROP_AND_BACKUP_BUTTON:
        {
            if(state == STATE_1)
            {
                startClipMaking();
            }
            else
            {
                stopClipMaking();
            }
        }
        break;

        case LIST_BUTTON:
        {
            if (true == m_isQuickBackupon)
            {
                MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(CMD_BACKUP_IN_PROCESS));
                break;
            }

            /* PARASOFT: Memory Deallocated in slot CropAndBackupPageClosed */
            m_inVisibleWidget = new QWidget(this->window());
            m_inVisibleWidget->setGeometry(0,
                                           0,
                                           this->window()->width(),
                                           this->window()->height());
            m_inVisibleWidget->show();

            if(m_syncPlaybackCropAndBackup == NULL)
            {
                quint8 recordIndex = m_recDriveListDropBox->getIndexofCurrElement();
                /* PARASOFT: Memory Deallocated in slot CropAndBackupPageClosed */
                m_syncPlaybackCropAndBackup = new SyncPlaybackCropAndBackup(this->window(),
                                                                            m_cropAndBackupData,
                                                                            m_recordValueForSearch,
                                                                            m_currentDeviceName,
                                                                            (recordIndex == 0) ? 4 : (recordIndex - 1));
                connect(m_syncPlaybackCropAndBackup,
                        SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                        this,
                        SLOT(slotCropAndBackupPageClosed(TOOLBAR_BUTTON_TYPE_e)));
            }
        }
        break;

        case CHANGE_MODE_BUTTON:
        {
            changeMode(m_currentMode == NORMAL_MODE ? FULL_MODE : NORMAL_MODE);
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void SyncPlayback::slotCropAndBackupPageClosed(TOOLBAR_BUTTON_TYPE_e)
{
    m_cropAndBackupData = m_syncPlaybackCropAndBackup->getCropAndBackupList();

    DELETE_OBJ(m_inVisibleWidget);
    disconnect(m_syncPlaybackCropAndBackup,
               SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
               this,
               SLOT(slotCropAndBackupPageClosed(TOOLBAR_BUTTON_TYPE_e)));
    DELETE_OBJ(m_syncPlaybackCropAndBackup);

    if(m_cropAndBackupData.length() == 0)
    {
        m_toolbar->changeButtonEnableState(LIST_BUTTON, false);
    }

    m_currentElement = SYNC_PLAYBACK_TOOLBAR;
    m_elementList[m_currentElement]->forceFocusToPage(true);
}

void SyncPlayback::slotPreProcessingDoneForSyncPlayback()
{
    m_isPreProcessingIsDone = true;
    if((m_isGetDateTimeRequestPending == false) && (m_isSearchDateRequestPending == false) && (m_isSearchHoursRequestPending == false))
    {
        m_syncPlaybackLoadingText->setVisible(false);
    }
}

void SyncPlayback::slotLayoutChanged()
{
    setLayoutWindowArea();
}

void SyncPlayback::slotExitFromZoomFeature()
{
    m_currentZoomState = SYNC_PLAYBACK_ZOOM_OUT;
    disconnect(m_zoomFeatureControl,
               SIGNAL(destroyed()),
               this,
               SLOT(slotExitFromZoomFeature()));

	changeLayout(m_prevLayout);
	if(m_currentMode != m_modeBeforeZoomAction)
    {
        changeMode(m_modeBeforeZoomAction);
    }

    m_toolbar->changeButtonState(ZOOM_BUTTON, STATE_1);
    m_toolbar->changeButtonEnableState(CHANGE_MODE_BUTTON, true);
    m_toolbar->changeButtonEnableState(LAYOUT_BUTTON, true);

    m_modeBeforeZoomAction = MAX_SYNCPB_TOOLBAR_MODE_TYPE;
    m_zoomFeatureControl = NULL;
    m_elementList[m_currentElement]->forceFocusToPage(true);
}

void SyncPlayback::slotScrollbarClick(int numberOfSteps)
{
	if(m_syncPlaybackLoadingText->isVisible() == false)
	{
		if(((m_firstCameraIndex+ numberOfSteps)>= 0)&&((m_lastCameraIndex+numberOfSteps)>= 0))
		{
			m_firstCameraIndex += numberOfSteps;
			m_lastCameraIndex += numberOfSteps;
			resetGeometryForCameraOption();
		}
	}
}

void SyncPlayback::slotRetryTimeOut()
{
	m_retryTimer->stop();
    m_cameraListCount = 0;
	searchHoursHavingRecord();
}

void SyncPlayback::slotchangeLayout(LAYOUT_TYPE_e iLayoutType)
{
	Layout::selectWindow (MAIN_DISPLAY,m_currentWindowIndex);
	changeLayout(iLayoutType);
    m_elementList[m_currentElement]->forceFocusToPage(true);
}

void SyncPlayback::CreateCameraList(void)
{
    /* Reset m_cameraListCount & m_selectedCamCnt */
    m_cameraListCount = 0;
    m_selectedCamCnt = 0;
    memset(&m_cameraValueForPlayRecord, 0, sizeof(m_cameraValueForPlayRecord));

	for(quint8 index = 0; index < m_cameraNameList.length(); index++)
	{
		/*MSB->LSB: bit4: Alarm, bit3: Cosec, bit2: Manual, bit1: Schedule, Bit0: xx */
		quint8 cameraRecType = 0;
		cameraRecType = m_applController->GetCamRecType((m_cameraIndex[index]-1));
		if(0 == cameraRecType)
        {
			continue;
        }

		m_cameraListCount++;

		/* Udpate Camera Checkbox Geometry and Text */
		m_cameraCheckbox[m_cameraListCount]->resetGeometry(RIGHT_PANEL_LEFT_MARGIN + DOTS_SEPARATION + SCALE_WIDTH(5),
                                                           ((m_camerasHeading->y() + m_camerasHeading->height()) + (m_cameraListCount * CAMERA_CHECKBOX_HEIGHT)),
                                                           CONTROL_WIDTH - SCROLLBAR_WIDTH - TYPE_INDICATION_SPACE, CAMERA_CHECKBOX_HEIGHT);
		m_cameraCheckbox[m_cameraListCount]->changeLabel(MX_OPTION_TEXT_TYPE_SUFFIX,QString("%1").arg(m_cameraIndex[index]) + ": " + m_cameraNameList.at(index));
		m_cameraCheckbox[m_cameraListCount]->setIndexInPage(SYNC_PLAYBACK_CAMERASELECT_CHECKBOX + m_cameraIndex[index]);

		/* Disable Camera Checkbox by Default */
		m_cameraCheckbox[m_cameraListCount]->changeState(OFF_STATE);

		/* Udpate Camera RecordType Labels Geometry and Text */
        for(quint8 loop = 0; loop < MAX_RECORDING_TYPE; loop++)
		{
            if(cameraRecType & recTypeMaplist[loop])
			{
                m_cameraRecType[m_cameraListCount-1][loop]->resetGeometry(RIGHT_PANEL_LEFT_MARGIN + ((loop / 2) * DOTS_SEPARATION) + SCALE_WIDTH(5),
                                                            m_cameraCheckbox[m_cameraListCount]->y() + ((loop % 2) * DOTS_SEPARATION) + SCALE_HEIGHT(18));
                m_cameraRecType[m_cameraListCount-1][loop]->changeText(".");
            }
            else
            {
                m_cameraRecType[m_cameraListCount-1][loop]->changeText("");
            }
		}

		/* Udpate Camera List Separator Geometry */
        if(m_cameraListCount < MAX_CAM_LIST_SEPARATOR_CNT)
		{
			m_cameraListSeparator[m_cameraListCount]->resetGeometry(RIGHT_PANEL_LEFT_MARGIN,
                                                                    (m_cameraCheckbox[m_cameraListCount]->y() + m_cameraCheckbox[m_cameraListCount]->height()),
                                                                    CONTROL_WIDTH,1);
			m_cameraListSeparator[m_cameraListCount]->setVisible(true);
		}
	}

	if(m_cameraListCount == 0)
	{
        DPRINT(GUI_SYNC_PB_MEDIA, "search sync playback: no cameras found");
        m_errorMsgTxt->changeText(ValidationMessage::getDeviceResponceMessage(CMD_NO_RECORD_FOUND));
		m_errorMsgTxt->setVisible(true);
        return;
	}

    /* Update First and Last Camera Index for Scrollbar */
    m_firstCameraIndex = 0;
    m_lastCameraIndex = ((m_cameraListCount > MAX_SCROLL_CAM_LIST_CNT) ? MAX_SCROLL_CAM_LIST_CNT : m_cameraListCount);
    DPRINT(GUI_SYNC_PB_MEDIA, "search sync playback: [cameraCnt=%d]", m_cameraListCount);

    /* Visible Camera Heading */
    m_camerasHeading->setVisible(true);

    /* Disable Camera Checkbox-Select All by Default */
    m_cameraCheckbox[0]->changeState(OFF_STATE);
    for(quint8 index = m_firstCameraIndex; index <= m_lastCameraIndex; index++)
    {
        /* Visible Camera Checkbox */
        m_cameraCheckbox[index]->setVisible(true);

        /* Visible Camera List Separator */
        m_cameraListSeparator[index]->setVisible(true);

        /* Visible Camera RecordType Labels */
        if(0 != index)
        {
            for(quint8 loop = 0; loop <MAX_RECORDING_TYPE; loop++)
            {
                m_cameraRecType[index-1][loop]->setVisible(true);
            }
        }
    }

    /*Enable hours button after recording is obtained*/
    for(quint8 index = 0; index < MAX_HOUR_FORMAT_TYPE; index++)
    {
        m_hoursRadioButtion[index]->setIsEnabled(true);
    }

    /* Delete Existing Scrollbar and Create New Scrollbar */
    if(IS_VALID_OBJ(m_syncScrollbar))
    {
        disconnect(m_syncScrollbar,
                    SIGNAL(sigScroll(int)),
                    this,
                    SLOT(slotScrollbarClick(int)));
        disconnect(m_syncScrollbar,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ (m_syncScrollbar);
    }

    m_syncScrollbar = new ScrollBar((m_cameraCheckbox[1]->x()+m_cameraCheckbox[1]->width()) + SCALE_WIDTH(12),
                                    m_cameraListSeparator[0]->y(),
                                    SCROLLBAR_WIDTH,
                                    MAX_SCROLL_CAM_LIST_CNT,
                                    CAMERA_CHECKBOX_HEIGHT ,
                                    m_cameraListCount,
                                    m_firstCameraIndex,
                                    this,
                                    VERTICAL_SCROLLBAR,
                                    SYNCPB_SCROLLBAR,
                                    ((m_cameraListCount > MAX_SCROLL_CAM_LIST_CNT)?true:false),
                                    false);
    connect(m_syncScrollbar,
            SIGNAL(sigScroll(int)),
            this,
            SLOT(slotScrollbarClick(int)));
    /* Adding a slot so that when mouse moves over scrollbar the layout lists visibility to be turned off */
    connect(m_syncScrollbar,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    //making visibility off for scrollbar if list is less than or equal to max_seen cameras
    m_syncScrollbar->setVisible((m_cameraListCount <= MAX_SCROLL_CAM_LIST_CNT) ? false : true);
}

void SyncPlayback::DeleteCameraList(void)
{
	for(quint8 index = 0; index < MAX_CAM_LIST_CNT; index++)
	{
		/* Delete Camera RecordType Labels */
		if(0 != index)
		{
			for(quint8 loop = 0; loop < MAX_RECORDING_TYPE; loop++)
			{
				DELETE_OBJ (m_cameraRecType[index-1][loop]);
			}
		}

		/* Delete Camera List Separator */
		if(index < MAX_CAM_LIST_SEPARATOR_CNT)
		{
			DELETE_OBJ (m_cameraListSeparator[index]);
		}

		/* Delete Camera List Checkbox */
		disconnect(m_cameraCheckbox[index],
					SIGNAL(sigUpdateCurrentElement(int)),
					this,
					SLOT(slotUpdateCurrentElement(int)));

		disconnect(m_cameraCheckbox[index],
					SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
					this,
					SLOT(slotCameraCheckboxClicked(OPTION_STATE_TYPE_e,int)));
		DELETE_OBJ (m_cameraCheckbox[index]);
		m_elementList[(SYNC_PLAYBACK_CAMERASELECT_CHECKBOX + index)] = NULL;
	}

	/* Delete Camera Heading */
	DELETE_OBJ (m_camerasHeading);

	/* Delete Camera List Scrollbar */
	if(IS_VALID_OBJ(m_syncScrollbar))
	{
		disconnect(m_syncScrollbar,
					SIGNAL(sigScroll(int)),
					this,
					SLOT(slotScrollbarClick(int)));
        disconnect(m_syncScrollbar,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
		DELETE_OBJ (m_syncScrollbar);
	}

	/* Delete Error Msg Text Label */
	DELETE_OBJ (m_errorMsgTxt);
}

void SyncPlayback::HideCameraList(void)
{
	/* Hide Camera Heading */
	m_camerasHeading->setVisible(false);
	for(quint8 index = 0; index < MAX_CAM_LIST_CNT; index++)
	{
		/* Hide Camera List Checkbox */
		m_cameraCheckbox[index]->setVisible(false);

		/* Hide Camera RecordType Labels */
		if(0 != index)
		{
			for(quint8 loop=0; loop < MAX_RECORDING_TYPE; loop++)
			{
				m_cameraRecType[index-1][loop]->setVisible(false);
			}
		}

		/* Hide Camera List Separator */
		if(index < MAX_CAM_LIST_SEPARATOR_CNT)
		{
			m_cameraListSeparator[index]->setVisible(false);
		}
	}

	/* Hide Camera List Scrollbar */
	if(IS_VALID_OBJ(m_syncScrollbar))
	{
		m_syncScrollbar->setVisible(false);
	}

	/* Hide Error Msg Text Label */
	m_errorMsgTxt->setVisible(false);
}

void SyncPlayback::updateDeviceList(void)
{
    QMap<quint8, QString> deviceMapList;
    m_applController->GetDevNameDropdownMapList(deviceMapList);

    /* Check if selected device found in new updated list or not. It is possible that index of that device name may changed.
     * Hence update device list with current device index */
    for (quint8 deviceIndex = 0; deviceIndex < deviceMapList.count(); deviceIndex++)
    {
        if (m_deviceNameDropDownBox->getCurrValue() == deviceMapList.value(deviceIndex))
        {
            m_deviceNameDropDownBox->setNewList(deviceMapList, deviceIndex);
            return;
        }
    }

    /* Device name not found in the list. If selected device is local device then we have to update the window name also if playback is going on */
    if (m_deviceNameDropDownBox->getIndexofCurrElement() == 0)
    {
        quint8  totalCamera = 0;
        quint16 windowIndex;
        QString realDevName = m_applController->GetRealDeviceName(deviceMapList.value(0));

        m_applController->GetTotalCamera(realDevName, totalCamera);
        for(quint8 cameraIndex = 0; cameraIndex < totalCamera; cameraIndex++)
        {
            windowIndex = Layout::findWindowOfSyncPlaybackStream(MAIN_DISPLAY, realDevName, (cameraIndex + 1));
            if (windowIndex < MAX_CHANNEL_FOR_SEQ)
            {
                Layout::updateWindowDataForSyncPB(windowIndex);
            }
        }
    }

    /* If selected device is local device then it will update the device name only otherwise it will clear all the data and will select the local device */
    m_deviceNameDropDownBox->setNewList(deviceMapList, 0);
    slotDeviceNameChanged(deviceMapList.value(0), SYNC_PLAYBACK_DEVICENAME_SPINBOX);
    m_currentDeviceName = m_applController->GetRealDeviceName(deviceMapList.value(0));
}
