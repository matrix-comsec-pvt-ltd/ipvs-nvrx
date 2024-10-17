#include "CameraEventAndEventAction.h"
#include "ValidationMessage.h"

#define MAX_CAMERA_EVENT_AND_ACTION_END_FEILDS  (CAM_EVE_COPY_TO_FEILD_END + 1)
#define MAX_CAM_EVNT_ACT_FLDS_SCH_MNG           20
#define DEV_ALARM_INDX                          7

typedef enum
{
    CAM_EVT_CAMERALIST_SPINBOX_CTRL,
    CAM_EVT_EVENTLIST_SPINBOX_CTRL,
    CAM_EVT_ACTION_CHECKBOX_CTRL,
    CAM_EVT_WEEKDAYS_SPINBOX_CTRL,
    CAM_EVT_WEEKDAYS_SET_CTRL,
    CAM_EVT_EVENT_ALARM_SET_CTRL,
    CAM_EVT_EVENT_IMG_UPLD_SET_CTRL,
    CAM_EVT_EVENT_EMAIL_SET_CTRL,
    CAM_EVT_EVENT_TCP_SET_CTRL,
    CAM_EVT_EVENT_SMS_SET_CTRL,
    CAM_EVT_EVENT_PRESET_SET_CTRL,
    CAM_EVT_EVENT_DEVICE_ALARM_SET_CTRL,
    CAM_EVT_EVENT_CAMERA_ALARM_SET_CTRL,
    CAM_EVT_EVENT_BUZZER_SET_CTRL,
    CAM_EVT_EVENT_VIDEO_POPUP_SET_CTRL,
    CAM_EVT_COPY_TO_CAM_CTRL,
    MAX_CAMERA_EVENT_AND_ACTION_CTRL
}CAMERA_EVENT_AND_ACTION_CTRL_e;

typedef enum
{
    CAM_EVT_CAMERALIST_SPINBOX_LABEL,
    CAM_EVT_EVENTLIST_SPINBOX_LABEL,
    CAM_EVT_COPYTOCAMERA_LABEL,
    CAM_EVT_ACTION_CHECKBOX_LABEL,
    CAM_EVT_ACTION_SCHD_HEADING,
    CAM_EVT_WEEKDAYS_SPINBOX_LABEL,
    CAM_EVT_EVENT_ACTION_LABEL,
    CAM_EVT_SET_CNTRL_LABEL =  (CAM_EVT_EVENT_ACTION_LABEL + MAX_CAM_EVNT_EVENTS),
    MAX_CAMERA_EVENT_AND_ACTION_STRINGS
}CAMERA_EVENT_AND_ACTION_STRINGS_e;

typedef enum
{
    OTHR_SUP_MOTION,
    OTHR_SUP_TEMPER,
    OTHR_SUP_PTZ,
    OTHR_SUP_AUDIO,
    OTHR_SUP_SENSOR,
    OTHR_SUP_ALARM,
    OTHR_SUP_MOTION_WINDOW,
    OTHR_SUP_PRIVACY_MASK,
    OTHR_SUP_LINE_DETECT,
    OTHR_SUP_INTRUCTION,
    OTHR_SUP_AUDIO_EXCEPTION,
    OTHR_SUP_MISSING_OBJECT,
    OTHR_SUP_SUSPIOUS_OBJECT,
    OTHR_SUP_LOITERING,
    OTHR_SUP_OBJECT_COUNTING,
    OTHR_SUP_NO_MOTION,
    MAX_CAM_EVENT_OTHER_SUP
}CAM_EVENT_OTHER_SUP_e;

static const QString cameraEventAndEventActionStrings[MAX_CAMERA_EVENT_AND_ACTION_STRINGS] =
{
    "Camera",
    "Event Type",
    "Copy to Camera",
    "Action",
    "Action Schedule",
    "Day",
    "Alarm Recording",
    "Image Upload",
    "Email Notification",
    "TCP Notification",
    "SMS Notification",
    "Preset Position",
    "Device Alarm",
    "Camera Alarm",
    "Buzzer",
    "Video Pop-up",
    "Push Notification",
    "Set"
};

typedef enum
{
    CAM_EVE_ENABLE_ACTION,
    CAM_EVE_CAM_NUM_ALARM_REC_START,
    CAM_EVE_CAM_NUM_ALARM_REC_END = CAM_EVE_CAM_NUM_ALARM_REC_START + CAMERA_MASK_MAX - 1,
    CAM_EVE_CAM_NUM_UPLOAD_REC_START,
    CAM_EVE_CAM_NUM_UPLOAD_REC_END = CAM_EVE_CAM_NUM_UPLOAD_REC_START + CAMERA_MASK_MAX - 1,
    CAM_EVE_EMAIL_ADDRESS,
    CAM_EVE_EMAIL_SUBJECT,
    CAM_EVE_EMAIL_MESSAGE,
    CAM_EVE_TCP_MESSAGE,
    CAM_EVE_SMS_MOB1,
    CAM_EVE_SMS_MOB2,
    CAM_EVE_SMS_MESSAGE,
    CAM_EVE_PTZ_CAM_NUM,
    CAM_EVE_PTZ_PRE_POS,
    CAM_EVE_SYS_ALM_OP_PORT1,
    CAM_EVE_SYS_ALM_OP_PORT2,
    CAM_EVE_SYS_ALM_OP_PORT3,
    CAM_EVE_SYS_ALM_OP_PORT4,
    CAM_EVE_CAM_ALM_CAM_NUM,
    CAM_EVE_CAM_ALM_ENB1,
    CAM_EVE_CAM_ALM_ENB2,
    CAM_EVE_CAM_ALM_ENB3,
    CAM_EVE_COPY_TO_FEILD_START,
    CAM_EVE_COPY_TO_FEILD_END = CAM_EVE_COPY_TO_FEILD_START + CAMERA_MASK_MAX - 1,
    CAM_EVT_SCHD_ENTIRE_DAY,
    CAM_EVT_SCHD_ACTION_STATUS_ENTIRE_DAY,
    CAM_EVT_SCHD_START_TIME1,
    CAM_EVT_SCHD_STOP_TIME1,
    CAM_EVT_SCHD_ACTION_STATUS_TIME1,
    CAM_EVT_SCHD_START_TIME2,
    CAM_EVT_SCHD_STOP_TIME2,
    CAM_EVT_SCHD_ACTION_STATUS_TIME2,
    CAM_EVT_SCHD_START_TIME3,
    CAM_EVT_SCHD_STOP_TIME3,
    CAM_EVT_SCHD_ACTION_STATUS_TIME3,
    CAM_EVT_SCHD_START_TIME4,
    CAM_EVT_SCHD_STOP_TIME4,
    CAM_EVT_SCHD_ACTION_STATUS_TIME4,
    CAM_EVT_SCHD_START_TIME5,
    CAM_EVT_SCHD_STOP_TIME5,
    CAM_EVT_SCHD_ACTION_STATUS_TIME5,
    CAM_EVT_SCHD_START_TIME6,
    CAM_EVT_SCHD_STOP_TIME6,
    CAM_EVT_SCHD_ACTION_STATUS_TIME6,
    MAX_CAMERA_EVENT_AND_ACTION_FEILDS
}CAMERA_EVENT_AND_ACTION_FEILDS_e;

typedef enum
{
    CAM_EVNT_MOTION_DETECTION,
    CAM_EVNT_VIEW_TEMPER,
    CAM_EVNT_SENSOR_INPUT1,
    CAM_EVNT_SENSOR_INPUT2,
    CAM_EVNT_SENSOR_INPUT3,
    CAM_EVNT_CONNECTION_FAILURE,
    CAM_EVNT_RECORDING_FAIL,
    CAM_EVNT_LINE_CROSSING,
    CAM_EVNT_INTRUSION_DETECTION,
    CAM_EVNT_AUDIO_EXCEPTION_DETECTION,
    CAM_EVT_MISSING_OBJECT,
    CAM_EVT_SUSPIOUS_OBJECT,
    CAM_EVT_LOITREING,
    CAM_EVT_CAMERA_ONLINE,
    CAM_EVNT_RECORDING_START,
    CAM_EVT_OBJECT_COUNTING,
    CAM_EVNT_NO_MOTION_DETECTION,
    MAX_CAM_EVNT_LIST
}CAMERA_EVENT_LIST_e;

static const QStringList cameraEventList = QStringList()
        << "Motion Detection"
        << "View Tampering"
        << "Sensor Input 1"
        << "Sensor Input 2"
        << "Sensor Input 3"
        << "Camera Offline"
        << "Recording Fail"
        << "Trip Wire"
        << "Object Intrusion"
        << "Audio Exception"
        << "Missing Object"
        << "Suspicious Object"
        << "Loitering"
        << "Camera Online"
        << "Recording Start"
        << "Object Counting"
        << "No Motion Detection";

static const QStringList cameraEventTypeList = QStringList()
        << "Motion Detection"
        << "View Tampering"
        << "Camera Offline"
        << "Recording Fail"
        << "Camera Online"
        << "Recording Start";

/* Adding Camera Events in the following list will enable all configuration related to video popup in Event & Action UI Page. */
static const QStringList videoPopupSupportedEventList = QStringList()
        << "Motion Detection"
        << "View Tampering"
        << "Sensor Input 1"
        << "Sensor Input 2"
        << "Sensor Input 3"
        << ""
        << "Recording Fail"
        << "Trip Wire"
        << "Object Intrusion"
        << "Audio Exception"
        << "Missing Object"
        << "Suspicious Object"
        << "Loitering"
        << "Camera Online"
        << "Recording Start"
        << "Object Counting"
        << "No Motion Detection";

static const QStringList weekdayList = QStringList() << "Sun" << "Mon" << "Tue" << "Wed" << "Thu" << "Fri" << "Sat";

CameraEventAndEventAction::CameraEventAndEventAction(QString deviceName, QWidget* parent, DEV_TABLE_INFO_t *devTabInfo)
    : ConfigPageControl(deviceName, parent,MAX_CAMERA_EVENT_AND_ACTION_CTRL,devTabInfo),
      currentDayIndex(0), currentCamForAlarm(0)
{
    setObjectName("CAM_EVNT");
    cameraAlarm[MAX_CAM_ALARM - 1] = {0};
    deviceAlaram[MAX_DEV_ALARM - 1] = {0};
    initlizeVariables();
    currentCameraIndex = 0;
    currentEventType = 0;
    isAudioExceptionSupport = false;
    isAudioSupport = false;
    isIntrusionSupport = false;
    isLineDetectionSupport = false;
    isLoteringSupport = false;
    isMissingObjectSupport = false;
    isMotionSupport = false;
    isNoMotionSupport = false;
    isObjectCountingSupport = false;
    isPTZSupport = false;
    isRequestSend = false;
    isSuspiousObjectSupport = false;
    isTemperSupport = false;
    maxAlarm = 0;
    maxSensor = 0;
    presetCameraNum = 0;
    presetGotoPosition = 0;
    totalCamera = 0;

    currentEventStr = cameraEventList.at(0);
    suppAlarms = devTabInfo->alarms;
    suppSensors = devTabInfo->sensors;
    createDefaultComponents ();
    CameraEventAndEventAction::getConfig ();
}

CameraEventAndEventAction::~CameraEventAndEventAction()
{
    deleteSubObejects ();

    disconnect (cameraListDropDownBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (cameraListDropDownBox,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotSpinBoxValueChanged(QString,quint32)));
    delete cameraListDropDownBox;

    disconnect (eventListDropDownBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

    disconnect (eventListDropDownBox,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotSpinBoxValueChanged(QString,quint32)));
    delete eventListDropDownBox;

    disconnect (actionEventCheckbox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (actionEventCheckbox,
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotCheckBoxValueChanged(OPTION_STATE_TYPE_e,int)));
    delete actionEventCheckbox;

    delete actionSchdHeading;

    disconnect (weekdayDropDownBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (weekdayDropDownBox,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotSpinBoxValueChanged(QString,quint32)));
    delete weekdayDropDownBox;

    for(quint8 index = 0; index < MAX_CAM_EVNT_SCHEDULE_BAR; index++)
    {
        delete scheduleBar[index];

        disconnect (setButtons[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        disconnect (setButtons[index],
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotButtonClick(int)));
        delete setButtons[index];

    }

    if(copyToCameraBtn != NULL)
    {
        disconnect (copyToCameraBtn,
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotButtonClick(int)));
        disconnect (copyToCameraBtn,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete copyToCameraBtn;
        copyToCameraBtn = NULL;
    }
}

void CameraEventAndEventAction::initlizeVariables()
{
    eventActionSchedule = NULL;
    cameraSelectForAlarm = NULL;
    cameraSelectForImageUpload = NULL;
    eventActionEmailNotify = NULL;
    eventActionTCPNotify = NULL;
    eventActionSmsNotify = NULL;
    eventActionPtzPos = NULL;
    eventActionDeviceAlarm = NULL;
    eventCameraAlarmOutput = NULL;
    eventCopyToCamera = NULL;
    copyToCameraBtn = NULL;

    memset(&alarmRecordingSelectedCamera, 0, sizeof(alarmRecordingSelectedCamera));
    memset(&uploadImageSelectedCamera, 0, sizeof(uploadImageSelectedCamera));
    memset(&eventSelectedCamera, 0, sizeof(eventSelectedCamera));
    memset(&scheduleTimeing, 0, sizeof(scheduleTimeing));
    memset(&actionStatus, 0, sizeof(actionStatus));

    for(quint8 index = 0; index < MAX_CAM_EVNT_WEEKDAYS;index++)
    {
        isScheduleSetForEntireDay[index] = false;
        isDaySelectForSchedule[index] = false;
    }
}

void CameraEventAndEventAction::createDefaultComponents ()
{
    isCameraEnable = false;
    isMotionConfigured = false;

    m_currentElement = CAM_EVT_CAMERALIST_SPINBOX_CTRL;
    for(quint8 index = 0; index < MAX_CAM_EVNT_SCHEDULE_BAR; index++)
    {
        setButtons[index] = NULL;
        scheduleBar[index] = NULL;
    }
    for(quint8 index = 0; index < MAX_CAMERA_EVENT_AND_ACTION_CTRL; index++)
    {
        m_elementList[index]  = NULL;
    }

    cameraListDropDownBox = new DropDown(SCALE_WIDTH(13),
                                         SCALE_HEIGHT(30),
                                         BGTILE_LARGE_SIZE_WIDTH,
                                         BGTILE_HEIGHT,
                                         CAM_EVT_CAMERALIST_SPINBOX_CTRL,
                                         DROPDOWNBOX_SIZE_320,
                                         cameraEventAndEventActionStrings
                                         [CAM_EVT_CAMERALIST_SPINBOX_LABEL],
                                         cameraList,
                                         this,
                                         "",
                                         false,
                                         SCALE_WIDTH(10));
    m_elementList[CAM_EVT_CAMERALIST_SPINBOX_CTRL] = cameraListDropDownBox;
    connect (cameraListDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (cameraListDropDownBox,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotSpinBoxValueChanged(QString,quint32)));

    for(quint8 index = 0; index < cameraEventTypeList.length (); index++)
    {
        cameraEventTypeMapList.insert (index,cameraEventTypeList.at (index));
    }

    eventListDropDownBox = new DropDown(BGTILE_LARGE_SIZE_WIDTH/2 - SCALE_WIDTH(50),
                                        cameraListDropDownBox->y (),
                                        BGTILE_LARGE_SIZE_WIDTH,
                                        BGTILE_HEIGHT,
                                        CAM_EVT_EVENTLIST_SPINBOX_CTRL,
                                        DROPDOWNBOX_SIZE_225,
                                        cameraEventAndEventActionStrings
                                        [CAM_EVT_EVENTLIST_SPINBOX_LABEL],
                                        cameraEventTypeMapList,
                                        this,
                                        "",
                                        false,
                                        0,
                                        NO_LAYER);
    m_elementList[CAM_EVT_EVENTLIST_SPINBOX_CTRL] = eventListDropDownBox;
    connect (eventListDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (eventListDropDownBox,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotSpinBoxValueChanged(QString,quint32)));

    copyToCameraBtn = new PageOpenButton((cameraListDropDownBox->x () +
                                          cameraListDropDownBox->width () ) - SCALE_WIDTH(210),
                                         cameraListDropDownBox->y () + SCALE_HEIGHT(5),
                                         SCALE_WIDTH(100),
                                         SCALE_HEIGHT(30),
                                         CAM_EVT_COPY_TO_CAM_CTRL,
                                         PAGEOPENBUTTON_EXTRALARGE,
                                         cameraEventAndEventActionStrings[CAM_EVT_COPYTOCAMERA_LABEL],
                                         this,
                                         "","",
                                         false,
                                         0,
                                         NO_LAYER);
    m_elementList[CAM_EVT_COPY_TO_CAM_CTRL] = copyToCameraBtn;
    connect (copyToCameraBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));
    connect (copyToCameraBtn,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    actionEventCheckbox = new OptionSelectButton(cameraListDropDownBox->x (),
                                                 cameraListDropDownBox->y () + BGTILE_HEIGHT + SCALE_HEIGHT(5),
                                                 BGTILE_LARGE_SIZE_WIDTH,
                                                 BGTILE_HEIGHT,
                                                 CHECK_BUTTON_INDEX,
                                                 this,
                                                 UP_LAYER,
                                                 "",
                                                 "",
                                                 SCALE_WIDTH(10),
                                                 CAM_EVT_ACTION_CHECKBOX_CTRL);
    m_elementList[CAM_EVT_ACTION_CHECKBOX_CTRL] = actionEventCheckbox;
    connect (actionEventCheckbox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (actionEventCheckbox,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotCheckBoxValueChanged(OPTION_STATE_TYPE_e,int)));

    actionSchdHeading = new ElementHeading(cameraListDropDownBox->x () + SCALE_WIDTH(50),
                                           actionEventCheckbox->y (),
                                           BGTILE_LARGE_SIZE_WIDTH,
                                           BGTILE_HEIGHT,
                                           cameraEventAndEventActionStrings
                                           [CAM_EVT_ACTION_SCHD_HEADING],
                                           NO_LAYER,
                                           this,
                                           false,
                                           0, NORMAL_FONT_SIZE, true);

    QMap<quint8, QString>  weekdayMapList;
    for(quint8 index = 0; index < weekdayList.length (); index++)
    {
        weekdayMapList.insert (index, weekdayList.at (index));
    }

    weekdayDropDownBox = new DropDown(cameraListDropDownBox->x (),
                                      actionSchdHeading->y () + BGTILE_HEIGHT,
                                      BGTILE_LARGE_SIZE_WIDTH,
                                      BGTILE_HEIGHT,
                                      CAM_EVT_WEEKDAYS_SPINBOX_CTRL,
                                      DROPDOWNBOX_SIZE_90,
                                      cameraEventAndEventActionStrings
                                      [CAM_EVT_WEEKDAYS_SPINBOX_LABEL],
                                      weekdayMapList,
                                      this,
                                      "",
                                      false,
                                      SCALE_WIDTH(19),
                                      MIDDLE_TABLE_LAYER);
    m_elementList[CAM_EVT_WEEKDAYS_SPINBOX_CTRL] = weekdayDropDownBox;
    connect (weekdayDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (weekdayDropDownBox,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotSpinBoxValueChanged(QString,quint32)));

    setButtons[0] = new PageOpenButton(weekdayDropDownBox->x () + SCALE_WIDTH(170),
                                       weekdayDropDownBox->y (),
                                       BGTILE_LARGE_SIZE_WIDTH,
                                       BGTILE_HEIGHT,
                                       CAM_EVT_WEEKDAYS_SET_CTRL,
                                       PAGEOPENBUTTON_SMALL,
                                       cameraEventAndEventActionStrings
                                       [CAM_EVT_SET_CNTRL_LABEL],
                                       this,
                                       "","",false,0,NO_LAYER,false);
    m_elementList[CAM_EVT_WEEKDAYS_SET_CTRL] = setButtons[0];
    connect (setButtons[0],
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (setButtons[0],
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClick(int)));

    for(quint8 index = 0; index < MAX_CAM_EVNT_SCHEDULE_BAR; index++)
    {
        scheduleBar[index] = new ScheduleBar(cameraListDropDownBox->x (),
                                             weekdayDropDownBox->y () + (index + 1)*BGTILE_HEIGHT,
                                             BGTILE_LARGE_SIZE_WIDTH,
                                             BGTILE_HEIGHT,
                                             index < 8 ? MIDDLE_TABLE_LAYER : BOTTOM_TABLE_LAYER,
                                             index == 0 ? "" : cameraEventAndEventActionStrings[CAM_EVT_EVENT_ACTION_LABEL + (index - 1)],
                                             SCALE_WIDTH(10),
                                             this,
                                             (index == 0) ? true : false,
                                             (index == 0) ? true : false);
        if (index > 0)
        {
            setButtons[index] = new PageOpenButton(BGTILE_LARGE_SIZE_WIDTH - SCALE_WIDTH(65),
                                                   scheduleBar[index]->y (),
                                                   BGTILE_LARGE_SIZE_WIDTH,
                                                   BGTILE_HEIGHT,
                                                   CAM_EVT_EVENT_ALARM_SET_CTRL + (index-1),
                                                   PAGEOPENBUTTON_SMALL,
                                                   cameraEventAndEventActionStrings
                                                   [CAM_EVT_SET_CNTRL_LABEL],
                                                   this,
                                                   "","",false,
                                                   0,
                                                   NO_LAYER,
                                                   false);
            m_elementList[ CAM_EVT_EVENT_ALARM_SET_CTRL + (index-1)] = setButtons[index];
            connect (setButtons[index] ,
                     SIGNAL(sigUpdateCurrentElement(int)),
                     this,
                     SLOT(slotUpdateCurrentElement(int)));
            connect (setButtons[index],
                     SIGNAL(sigButtonClick(int)),
                     this,
                     SLOT(slotButtonClick(int)));
        }
    }

    for(quint8 index = EVNT_VIDEO_POP_UP; index < MAX_CAM_EVNT_SCHEDULE_BAR; index++)
    {
        setButtons[index]->setVisible(false);
    }

    #if defined(OEM_JCI)
    scheduleBar[EVNT_SMS]->setVisible(false);
    setButtons[EVNT_SMS]->setVisible(false);
    scheduleBar[EVNT_TCP]->setVisible(false);
    setButtons[EVNT_TCP]->setVisible(false);

    for(quint8 index = EVNT_TCP + 1 ; index < MAX_CAM_EVNT_SCHEDULE_BAR; index++)
    {
        scheduleBar[index]->resetGeometry(scheduleBar[index]->x(),
                                          scheduleBar[index]->y() - (2 * BGTILE_HEIGHT),
                                          BGTILE_LARGE_SIZE_WIDTH,
                                          BGTILE_HEIGHT);
        setButtons[index]->resetGeometry(setButtons[index]->x(),
                                         setButtons[index]->y() - (2 * BGTILE_HEIGHT));
    }
    #endif

    resetGeometryOfCnfgbuttonRow (SCALE_HEIGHT(45));
}

void CameraEventAndEventAction::fillCameraList ()
{
    cameraList.clear();
    for(quint8 index = 0; index < devTableInfo->totalCams; index++)
    {
        cameraList.insert(index, QString("%1%2%3").arg(index + 1).arg(" : ").arg(applController->GetCameraNameOfDevice(currDevName, index)));
    }

    cameraListDropDownBox->setNewList(cameraList, currentCameraIndex);
}

void CameraEventAndEventAction::createPayload(REQ_MSG_ID_e msgType)
{
    quint8 tempVar = cameraEventList.indexOf (currentEventStr);

    cnfg1FrmIndx = ((currentCameraIndex*MAX_CAM_EVNT_LIST) + tempVar + 1);// 1 for camera index
    cnfg2ToIndx = (((currentCameraIndex*MAX_CAM_EVNT_LIST) + tempVar)*weekdayList.length ()) + 1;
    cnfg2FrmIndx = (cnfg2ToIndx + weekdayList.length() - 1);

    QString payloadString = payloadLib->createDevCnfgPayload(msgType,
                                                             CAMERA_EVENT_ACTION_MANAGMENT_TABLE_INDEX,
                                                             cnfg1FrmIndx,
                                                             cnfg1FrmIndx,
                                                             CNFG_FRM_INDEX,
                                                             MAX_CAMERA_EVENT_AND_ACTION_END_FEILDS,
                                                             MAX_CAMERA_EVENT_AND_ACTION_END_FEILDS);

    payloadString = payloadLib->createDevCnfgPayload(msgType,
                                                     CAMERA_EVENT_ACTION_SCHDULE_TABLE_INDEX,
                                                     cnfg2ToIndx,
                                                     cnfg2FrmIndx,
                                                     CNFG_FRM_INDEX,
                                                     MAX_CAM_EVNT_ACT_FLDS_SCH_MNG,
                                                     (weekdayList.length () * MAX_CAM_EVNT_ACT_FLDS_SCH_MNG),
                                                     payloadString,
                                                     MAX_CAMERA_EVENT_AND_ACTION_END_FEILDS);

    DevCommParam* param = new DevCommParam();
    param->msgType = msgType;
    param->payload = payloadString;

    processBar->loadProcessBar ();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void CameraEventAndEventAction::defaultConfig ()
{
    createPayload(MSG_DEF_CFG);
}

bool CameraEventAndEventAction::isUserChangeConfig()
{
    quint8 index;

    if(m_configResponse.isEmpty())
    {
        return false;
    }

    if((IS_VALID_OBJ(actionEventCheckbox)) && (m_configResponse[CAM_EVE_ENABLE_ACTION] != actionEventCheckbox->getCurrentState()))
    {
        return true;
    }

    for(quint8 maskIdx = 0; maskIdx < CAMERA_MASK_MAX; maskIdx++)
    {
        if(m_configResponse[CAM_EVE_CAM_NUM_ALARM_REC_START + maskIdx].toULongLong() != alarmRecordingSelectedCamera.bitMask[maskIdx])
        {
            return true;
        }

        if(m_configResponse[CAM_EVE_CAM_NUM_UPLOAD_REC_START + maskIdx].toULongLong() != uploadImageSelectedCamera.bitMask[maskIdx])
        {
            return true;
        }

        if(m_configResponse[CAM_EVE_COPY_TO_FEILD_START + maskIdx].toULongLong() != eventSelectedCamera.bitMask[maskIdx])
        {
            return true;
        }
    }

    if(m_configResponse[CAM_EVE_EMAIL_ADDRESS] != emailAddress)
    {
        return true;
    }

    if(m_configResponse[CAM_EVE_EMAIL_SUBJECT] != emailSubject)
    {
        return true;
    }

    if(m_configResponse[CAM_EVE_EMAIL_MESSAGE] != emailMessage)
    {
        return true;
    }

    if(m_configResponse[CAM_EVE_TCP_MESSAGE] != tcpMessage)
    {
        return true;
    }

    if(m_configResponse[CAM_EVE_SMS_MOB1] != smsMobileNum1)
    {
        return true;
    }

    if(m_configResponse[CAM_EVE_SMS_MOB2] != smsMobileNum2)
    {
        return true;
    }

    if(m_configResponse[CAM_EVE_SMS_MESSAGE] != smsMessage)
    {
        return true;
    }

    if(m_configResponse[CAM_EVE_PTZ_CAM_NUM] != presetCameraNum)
    {
        return true;
    }

    if(m_configResponse[CAM_EVE_PTZ_PRE_POS] != presetGotoPosition)
    {
        return true;
    }

    if(m_configResponse[CAM_EVE_CAM_ALM_CAM_NUM] != currentCamForAlarm)
    {
        return true;
    }

    for(index = 0; index < MAX_DEV_ALARM; index++)
    {
        if(m_configResponse[CAM_EVE_SYS_ALM_OP_PORT1 + index] != deviceAlaram[index])
        {
            return true;
        }
    }

    for(index = 0; index < MAX_CAM_ALARM; index++)
    {
        if(m_configResponse[CAM_EVE_CAM_ALM_ENB1 + index] != cameraAlarm[index])
        {
            return true;
        }
    }

    for(index = 0; index< MAX_CAM_EVNT_WEEKDAYS;index++)
    {
        if(m_configResponse[CAM_EVT_SCHD_ENTIRE_DAY + (index * MAX_CAM_EVNT_ACT_FLDS_SCH_MNG)] != isScheduleSetForEntireDay[index])
        {
            return true;
        }

        for(quint8 index1 = 0; index1 < MAX_TIME_SLOT;index1++)
        {
            if((m_configResponse[CAM_EVT_SCHD_START_TIME1+(MAX_CAM_EVNT_ACT_FLDS_SCH_MNG * index) +  3*index1].toString ().mid (0,2).toUInt ()) != scheduleTimeing[index].start_hour[index1])
            {
                return true;
            }

            if((m_configResponse[CAM_EVT_SCHD_START_TIME1+(MAX_CAM_EVNT_ACT_FLDS_SCH_MNG * index) +  3*index1].toString ().mid (2,2).toUInt ()) != scheduleTimeing[index].start_min[index1])
            {
                return true;
            }

            if((m_configResponse[CAM_EVT_SCHD_STOP_TIME1+(MAX_CAM_EVNT_ACT_FLDS_SCH_MNG * index) +  3*index1].toString ().mid (0,2).toUInt ()) != scheduleTimeing[index].stop_hour[index1])
            {
                return true;
            }

            if((m_configResponse[CAM_EVT_SCHD_STOP_TIME1+(MAX_CAM_EVNT_ACT_FLDS_SCH_MNG * index) +  3*index1].toString ().mid (2,2).toUInt ()) != scheduleTimeing[index].stop_min[index1])
            {
                return true;
            }
        }
    }

    for(quint8 dayindex = 0; dayindex < MAX_CAM_EVNT_WEEKDAYS;dayindex++)
    {
        if((m_configResponse[CAM_EVT_SCHD_ACTION_STATUS_ENTIRE_DAY + dayindex * MAX_CAM_EVNT_ACT_FLDS_SCH_MNG]) != actionStatus[dayindex*MAX_CAM_EVNT_WEEKDAYS])
        {
            return true;
        }

        for(index = 1;index < MAX_CAM_EVNT_WEEKDAYS; index++)
        {
            if((m_configResponse[CAM_EVT_SCHD_ACTION_STATUS_TIME1 + ((MAX_CAM_EVNT_ACT_FLDS_SCH_MNG * (dayindex)) + (index-1)*3)]) != actionStatus[index + (dayindex * MAX_CAM_EVNT_WEEKDAYS)])
            {
                return true;
            }
        }
    }

    return false;
}

void CameraEventAndEventAction::handleInfoPageMessage(int index)
{
    if(index == INFO_OK_BTN)
    {
        if(infoPage->getText() == (ValidationMessage::getValidationMessage(SAVE_CHANGES)))
        {
            saveConfig();
        }
    }
    else
    {
        if(infoPage->getText() == (ValidationMessage::getValidationMessage(SAVE_CHANGES)))
        {
            currentCameraIndex = cameraListDropDownBox->getIndexofCurrElement ();
            currentEventType = eventListDropDownBox->getIndexofCurrElement ();
            getConfig();
        }
    }
}

void CameraEventAndEventAction::sendCommand(SET_COMMAND_e cmdType, quint8 totalfeilds)
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = cmdType;
    param->payload = payloadLib->createDevCmdPayload(totalfeilds);
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void CameraEventAndEventAction::getConfig ()
{
    initlizeVariables();

    currentcamtype = applController->GetCameraType (currDevName,(currentCameraIndex));

    fillCameraList();

    if((currentcamtype == IP_CAMERA) || (currentcamtype == AUTO_ADD_IP_CAMERA))
    {
        payloadLib->setCnfgArrayAtIndex (0,currentCameraIndex + 1);
        sendCommand (OTHR_SUP,1);
    }
    else if(currentcamtype == ANALOG_CAMERA)
    {
        eventListDropDownBox->setNewList (cameraEventTypeMapList,currentEventType);
        currentEventStr = eventListDropDownBox->getCurrValue ();
        getConfig1 ();
    }
}

void CameraEventAndEventAction::getConfig1 ()
{
    createPayload(MSG_GET_CFG);
}

void CameraEventAndEventAction::setConfigFields()
{
    payloadLib->setCnfgArrayAtIndex(CAM_EVE_ENABLE_ACTION, actionEventCheckbox->getCurrentState ());

    /* Check if any mask bit is set or not */
    if (TRUE == IS_ALL_CAMERA_MASK_BIT_CLR(alarmRecordingSelectedCamera))
    {
        SET_CAMERA_MASK_BIT(alarmRecordingSelectedCamera, currentCameraIndex);
    }

    /* Check if any mask bit is set or not */
    if (TRUE == IS_ALL_CAMERA_MASK_BIT_CLR(uploadImageSelectedCamera))
    {
        SET_CAMERA_MASK_BIT(uploadImageSelectedCamera, currentCameraIndex);
    }

    SET_CAMERA_MASK_BIT(eventSelectedCamera, currentCameraIndex);
    for (quint8 maskIdx = 0; maskIdx < CAMERA_MASK_MAX; maskIdx++)
    {
        payloadLib->setCnfgArrayAtIndex((CAM_EVE_CAM_NUM_ALARM_REC_START + maskIdx), alarmRecordingSelectedCamera.bitMask[maskIdx]);
        payloadLib->setCnfgArrayAtIndex((CAM_EVE_CAM_NUM_UPLOAD_REC_START + maskIdx), uploadImageSelectedCamera.bitMask[maskIdx]);
        payloadLib->setCnfgArrayAtIndex((CAM_EVE_COPY_TO_FEILD_START + maskIdx), eventSelectedCamera.bitMask[maskIdx]);
    }

    payloadLib->setCnfgArrayAtIndex(CAM_EVE_EMAIL_ADDRESS,emailAddress);
    payloadLib->setCnfgArrayAtIndex(CAM_EVE_EMAIL_SUBJECT,emailSubject);
    payloadLib->setCnfgArrayAtIndex(CAM_EVE_EMAIL_MESSAGE,emailMessage);
    payloadLib->setCnfgArrayAtIndex(CAM_EVE_TCP_MESSAGE,tcpMessage);
    payloadLib->setCnfgArrayAtIndex(CAM_EVE_SMS_MOB1,smsMobileNum1);
    payloadLib->setCnfgArrayAtIndex(CAM_EVE_SMS_MOB2,smsMobileNum2);
    payloadLib->setCnfgArrayAtIndex(CAM_EVE_SMS_MESSAGE,smsMessage);
    payloadLib->setCnfgArrayAtIndex(CAM_EVE_PTZ_CAM_NUM,presetCameraNum);
    payloadLib->setCnfgArrayAtIndex(CAM_EVE_PTZ_PRE_POS,presetGotoPosition);
    payloadLib->setCnfgArrayAtIndex(CAM_EVE_CAM_ALM_CAM_NUM,currentCamForAlarm);

    for(quint8 index = 0; index < MAX_DEV_ALARM; index++)
    {
        payloadLib->setCnfgArrayAtIndex((CAM_EVE_SYS_ALM_OP_PORT1 + index), (deviceAlaram[index] == false) ? 0 : 1);
    }

    for(quint8 index = 0; index < MAX_CAM_ALARM; index++)
    {
        payloadLib->setCnfgArrayAtIndex((CAM_EVE_CAM_ALM_ENB1 + index), (cameraAlarm[index] == false) ? 0 : 1);
    }

    DPRINT(CONFIG_PAGES,"[CAMERA_EVENT_ACTION][Action state: %d][Camera: %d]",actionEventCheckbox->getCurrentState(), currentCameraIndex);
}

void CameraEventAndEventAction::fillRecords()
{
    QString stopTime = "";
    QString startTime = "";

    for(quint8 index = 0;index < MAX_CAM_EVNT_WEEKDAYS;index++)
    {
        if(isScheduleSetForEntireDay[index])
        {
            payloadLib->setCnfgArrayAtIndex (CAM_EVT_SCHD_ENTIRE_DAY + ((index)*MAX_CAM_EVNT_ACT_FLDS_SCH_MNG),1);
        }
        else
        {
            payloadLib->setCnfgArrayAtIndex (CAM_EVT_SCHD_ENTIRE_DAY + ((index)*MAX_CAM_EVNT_ACT_FLDS_SCH_MNG),0);

            for(quint8 index1 = 0; index1 < MAX_TIME_SLOT;index1++)
            {
                if(scheduleTimeing[index].start_hour[index1] > 24)
                {
                    scheduleTimeing[index].start_hour[index1] = 0;
                }

                startTime = scheduleTimeing[index].start_hour[index] < 10 ? (QString("0") + QString("%1").arg (scheduleTimeing[index].start_hour[index1]))
                                                                          : ( QString("%1").arg (scheduleTimeing[index].start_hour[index1]) );

                if(scheduleTimeing[index].start_min[index1] > 60)
                {
                    scheduleTimeing[index].start_min[index1] = 0;
                }

                scheduleTimeing[index].start_min[index1] < 10 ? startTime.append( QString("0") + QString("%1").arg(scheduleTimeing[index].start_min[index1]) )
                                                              : startTime.append (QString("%1").arg(scheduleTimeing[index].start_min[index1]));


                payloadLib->setCnfgArrayAtIndex((CAM_EVT_SCHD_START_TIME1 + (MAX_CAM_EVNT_ACT_FLDS_SCH_MNG * (index) ) + 3*(index1)),startTime);

                if(scheduleTimeing[index].stop_hour[index1] > 24)
                {
                    scheduleTimeing[index].stop_hour[index1] = 0;
                }

                stopTime = scheduleTimeing[index].stop_hour[index1] < 10 ? (QString("0") + QString("%1").arg (scheduleTimeing[index].stop_hour[index1]) )
                                                                         : ( QString("%1").arg (scheduleTimeing[index].stop_hour[index1]) );

                if(scheduleTimeing[index].stop_min[index1] > 60)
                {
                    scheduleTimeing[index].stop_min[index1] = 0;
                }

                scheduleTimeing[index].stop_min[index1] < 10 ? stopTime.append (QString("0") + QString("%1").arg(scheduleTimeing[index].stop_min[index1]) )
                                                             : stopTime.append (QString("%1").arg(scheduleTimeing[index].stop_min[index1]));

                payloadLib->setCnfgArrayAtIndex((CAM_EVT_SCHD_STOP_TIME1 + (MAX_CAM_EVNT_ACT_FLDS_SCH_MNG * (index)) + 3*(index1)),stopTime);
            }
        }
    }

    for(quint8 dayindex = 0; dayindex < MAX_CAM_EVNT_WEEKDAYS; dayindex++)
    {
        payloadLib->setCnfgArrayAtIndex(CAM_EVT_SCHD_ACTION_STATUS_ENTIRE_DAY + dayindex*MAX_CAM_EVNT_ACT_FLDS_SCH_MNG, actionStatus[dayindex * MAX_CAM_EVNT_WEEKDAYS]);

        for(quint8 index = 1; index < MAX_CAM_EVNT_WEEKDAYS; index++)
        {
            payloadLib->setCnfgArrayAtIndex( CAM_EVT_SCHD_ACTION_STATUS_TIME1 + MAX_CAM_EVNT_ACT_FLDS_SCH_MNG *dayindex + (index-1)*3 , actionStatus[index + (dayindex * MAX_CAM_EVNT_WEEKDAYS)]);
        }
    }
}

void CameraEventAndEventAction::saveConfig ()
{
    setConfigFields();
    fillRecords();
    createPayload(MSG_SET_CFG);
}

void CameraEventAndEventAction::updateScheduleBar()
{
    SCHEDULE_TIMING_t tempSchd;

    memset(&tempSchd, 0, sizeof(SCHEDULE_TIMING_t));
    for(quint8 index = 1; index < MAX_CAM_EVNT_SCHEDULE_BAR; index++)
    {
        scheduleBar[index]->setSchedule (tempSchd);
    }

    currentDayIndex =  weekdayDropDownBox->getIndexofCurrElement ();
    if(isScheduleSetForEntireDay[currentDayIndex] == true)
    {
        for(quint8 index = 0; index < EVNT_VIDEO_POP_UP; index++ )
        {
            if(actionStatus[MAX_CAM_EVNT_WEEKDAYS*currentDayIndex ] & (0x01 << index))
            {
                scheduleBar [EVNT_VIDEO_POP_UP - index]->setScheduleForEntireDay (true);
            }
            else
            {
                scheduleBar [EVNT_VIDEO_POP_UP - index]->setScheduleForEntireDay (false);
            }
            scheduleBar [EVNT_VIDEO_POP_UP - index]->update ();
        }

        for(quint8 index = EVNT_VIDEO_POP_UP; index < MAX_EVNT_SCHD_EVNT; index++ )
        {
            if(actionStatus[MAX_CAM_EVNT_WEEKDAYS*currentDayIndex ] & (0x01 << index))
            {
                scheduleBar [index+1]->setScheduleForEntireDay (true);
            }
            else
            {
                scheduleBar [index+1]->setScheduleForEntireDay (false);
            }
            scheduleBar [index+1]->update ();
        }
    }
    else
    {
        for(quint8 index = 0; index < EVNT_VIDEO_POP_UP;index++ )
        {
            memset(&tempSchd, 0, sizeof(SCHEDULE_TIMING_t));
            for(quint8 index1 = 0; index1 < MAX_TIME_SLOT; index1++)
            {
                if(actionStatus[index1 + MAX_CAM_EVNT_WEEKDAYS*currentDayIndex + 1]  &  ( 1 << index))
                {
                    tempSchd.start_hour[index1] = scheduleTimeing[currentDayIndex].start_hour[index1];
                    tempSchd.stop_hour[index1] = scheduleTimeing[currentDayIndex].stop_hour[index1];
                    tempSchd.start_min[index1] = scheduleTimeing[currentDayIndex].start_min[index1];
                    tempSchd.stop_min[index1] = scheduleTimeing[currentDayIndex].stop_min[index1];
                }
            }

            scheduleBar [EVNT_VIDEO_POP_UP - index]->setSchedule (tempSchd);
            scheduleBar [EVNT_VIDEO_POP_UP - index]->update ();
        }

        for(quint8 index = EVNT_VIDEO_POP_UP; index < MAX_EVNT_SCHD_EVNT; index++ )
        {
            memset(&tempSchd, 0, sizeof(SCHEDULE_TIMING_t));
            for(quint8 index1 = 0; index1 < MAX_TIME_SLOT; index1++)
            {
                if(actionStatus[index1 + MAX_CAM_EVNT_WEEKDAYS*currentDayIndex + 1]  &  ( 1 << index))
                {
                    tempSchd.start_hour[index1] = scheduleTimeing[currentDayIndex].start_hour[index1];
                    tempSchd.stop_hour[index1] = scheduleTimeing[currentDayIndex].stop_hour[index1];
                    tempSchd.start_min[index1] = scheduleTimeing[currentDayIndex].start_min[index1];
                    tempSchd.stop_min[index1] = scheduleTimeing[currentDayIndex].stop_min[index1];
                }
            }
            scheduleBar [index+1]->setSchedule (tempSchd);
            scheduleBar [index+1]->update ();
        }
    }
}

void CameraEventAndEventAction::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if (deviceName != currDevName)
    {
        return;
    }

    if (param->deviceStatus != CMD_SUCCESS)
    {
        processBar->unloadProcessBar();
        if (param->msgType != MSG_SET_CMD)
        {
            infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
            return;
        }

        if (param->cmdType == OTHR_SUP)
        {
            isMotionSupport = false;
            isNoMotionSupport = false;
            isTemperSupport = false;
            isPTZSupport = false;
            isAudioSupport = false;
            isLineDetectionSupport = false;
            isIntrusionSupport = false;
            isAudioExceptionSupport = false;
            maxSensor = 0;
            maxAlarm = 0;
            isMissingObjectSupport = false;
            isSuspiousObjectSupport = false;
            isLoteringSupport = false;
            isObjectCountingSupport = false;
            fillOtherSupportData ();
        }
        return;
    }

    switch(param->msgType)
    {
        case MSG_GET_CFG:
        {
            m_configResponse.clear();
            payloadLib->parsePayload (param->msgType, param->payload);
            presetList.clear ();
            if(payloadLib->getcnfgTableIndex () == PRESET_POSITION_TABLE_INDEX)
            {
                presetList.insert (0,"None");
                for(quint8 index = 0; index< MAX_PRESET_POS; index++)
                {
                    QString tempStr  = (payloadLib->getCnfgArrayAtIndex(index)).toString ();
                    if(tempStr != "")
                    {
                        presetList.insert((index + 1) , QString("%1%2%3").arg(index + 1).arg(" : ").arg (tempStr));
                    }
                }

                if(eventActionPtzPos != NULL)
                {
                    eventActionPtzPos->processDeviceResponse (presetList);
                }
            }

            if(payloadLib->getcnfgTableIndex (0) == CAMERA_EVENT_ACTION_MANAGMENT_TABLE_INDEX)
            {
                actionEventCheckbox->changeState ((payloadLib->getCnfgArrayAtIndex (CAM_EVE_ENABLE_ACTION).toUInt ()) == 0 ? OFF_STATE : ON_STATE);

                m_configResponse[CAM_EVE_ENABLE_ACTION] =  payloadLib->getCnfgArrayAtIndex(CAM_EVE_ENABLE_ACTION);

                enableControlsOnAction (actionEventCheckbox->getCurrentState () == ON_STATE ? true : false );

                memset(&alarmRecordingSelectedCamera, 0, sizeof(alarmRecordingSelectedCamera));
                memset(&uploadImageSelectedCamera, 0, sizeof(uploadImageSelectedCamera));
                memset(&eventSelectedCamera, 0, sizeof(eventSelectedCamera));

                for(quint8 maskIdx = 0; maskIdx < CAMERA_MASK_MAX; maskIdx++)
                {
                    alarmRecordingSelectedCamera.bitMask[maskIdx] = (payloadLib->getCnfgArrayAtIndex(CAM_EVE_CAM_NUM_ALARM_REC_START + maskIdx).toULongLong());
                    m_configResponse[CAM_EVE_CAM_NUM_ALARM_REC_START + maskIdx] = alarmRecordingSelectedCamera.bitMask[maskIdx];

                    uploadImageSelectedCamera.bitMask[maskIdx] = (payloadLib->getCnfgArrayAtIndex(CAM_EVE_CAM_NUM_UPLOAD_REC_START + maskIdx).toULongLong());
                    m_configResponse[CAM_EVE_CAM_NUM_UPLOAD_REC_START + maskIdx] = uploadImageSelectedCamera.bitMask[maskIdx];

                    eventSelectedCamera.bitMask[maskIdx] = (payloadLib->getCnfgArrayAtIndex(CAM_EVE_COPY_TO_FEILD_START + maskIdx).toULongLong());
                    m_configResponse[CAM_EVE_COPY_TO_FEILD_START + maskIdx] = eventSelectedCamera.bitMask[maskIdx];
                }

                emailAddress = (payloadLib->getCnfgArrayAtIndex (CAM_EVE_EMAIL_ADDRESS).toString ());
                m_configResponse[CAM_EVE_EMAIL_ADDRESS] = emailAddress;

                emailSubject = (payloadLib->getCnfgArrayAtIndex (CAM_EVE_EMAIL_SUBJECT).toString ());
                m_configResponse[CAM_EVE_EMAIL_SUBJECT] = emailSubject;

                emailMessage = (payloadLib->getCnfgArrayAtIndex (CAM_EVE_EMAIL_MESSAGE).toString ());
                m_configResponse[CAM_EVE_EMAIL_MESSAGE] = emailMessage;

                tcpMessage = (payloadLib->getCnfgArrayAtIndex (CAM_EVE_TCP_MESSAGE).toString ());
                m_configResponse[CAM_EVE_TCP_MESSAGE] = tcpMessage;

                smsMobileNum1 = (payloadLib->getCnfgArrayAtIndex (CAM_EVE_SMS_MOB1).toString ());
                m_configResponse[CAM_EVE_SMS_MOB1] = smsMobileNum1;

                smsMobileNum2 = (payloadLib->getCnfgArrayAtIndex (CAM_EVE_SMS_MOB2).toString ());
                m_configResponse[CAM_EVE_SMS_MOB2] = smsMobileNum2;

                smsMessage = (payloadLib->getCnfgArrayAtIndex (CAM_EVE_SMS_MESSAGE).toString ());
                m_configResponse[CAM_EVE_SMS_MESSAGE] = smsMessage;

                presetCameraNum = (payloadLib->getCnfgArrayAtIndex (CAM_EVE_PTZ_CAM_NUM).toUInt ());
                m_configResponse[CAM_EVE_PTZ_CAM_NUM] = presetCameraNum;

                presetGotoPosition = (payloadLib->getCnfgArrayAtIndex (CAM_EVE_PTZ_PRE_POS).toUInt ());
                m_configResponse[CAM_EVE_PTZ_PRE_POS] = presetGotoPosition;

                currentCamForAlarm = (payloadLib->getCnfgArrayAtIndex (CAM_EVE_CAM_ALM_CAM_NUM).toUInt ());
                m_configResponse[CAM_EVE_CAM_ALM_CAM_NUM] = currentCamForAlarm;

                currentCamForAlarm = (currentCamForAlarm == 0) ? 1 : currentCamForAlarm;

                for(quint8 index = 0; index < MAX_DEV_ALARM; index++)
                {
                    deviceAlaram[index] = (payloadLib->getCnfgArrayAtIndex (CAM_EVE_SYS_ALM_OP_PORT1 + index)).toUInt () == 0 ? false : true;
                    m_configResponse[CAM_EVE_SYS_ALM_OP_PORT1 + index] = deviceAlaram[index];
                }

                for(quint8 index = 0; index < MAX_CAM_ALARM; index++)
                {
                    cameraAlarm[index] = (payloadLib->getCnfgArrayAtIndex (CAM_EVE_CAM_ALM_ENB1 + index)).toUInt () == 0 ? false : true;
                    m_configResponse[CAM_EVE_CAM_ALM_ENB1 + index] = cameraAlarm[index];
                }
            }

            if(payloadLib->getcnfgTableIndex (1) == CAMERA_EVENT_ACTION_SCHDULE_TABLE_INDEX)
            {
                for(quint8 index = 0; index< MAX_CAM_EVNT_WEEKDAYS; index++)
                {
                    if( payloadLib->getCnfgArrayAtIndex(CAM_EVT_SCHD_ENTIRE_DAY + (index * MAX_CAM_EVNT_ACT_FLDS_SCH_MNG)).toUInt () == 1)
                    {
                        isScheduleSetForEntireDay[index] = true;
                        m_configResponse[CAM_EVT_SCHD_ENTIRE_DAY+ (index * MAX_CAM_EVNT_ACT_FLDS_SCH_MNG)] = isScheduleSetForEntireDay[index];
                    }
                    else
                    {
                        isScheduleSetForEntireDay[index] = false;
                        m_configResponse[CAM_EVT_SCHD_ENTIRE_DAY + (index * MAX_CAM_EVNT_ACT_FLDS_SCH_MNG)] = isScheduleSetForEntireDay[index];

                        for(quint8 index1 = 0; index1 < MAX_TIME_SLOT; index1++)
                        {
                            QString time = (payloadLib->getCnfgArrayAtIndex(CAM_EVT_SCHD_START_TIME1 + 3*index1 + (MAX_CAM_EVNT_ACT_FLDS_SCH_MNG * (index)))).toString ().mid (0,2);
                            m_configResponse[CAM_EVT_SCHD_START_TIME1+(MAX_CAM_EVNT_ACT_FLDS_SCH_MNG * index) + 3*index1] =
                                    payloadLib-> getCnfgArrayAtIndex( CAM_EVT_SCHD_START_TIME1 + 3*index1 + (MAX_CAM_EVNT_ACT_FLDS_SCH_MNG * (index)));
                            scheduleTimeing[index].start_hour[index1] = time.toUInt ();

                            time = (payloadLib->getCnfgArrayAtIndex(CAM_EVT_SCHD_START_TIME1 + 3*index1 + (MAX_CAM_EVNT_ACT_FLDS_SCH_MNG * (index)))).toString ().mid (2,2);
                            scheduleTimeing[index].start_min[index1] = time.toUInt ();

                            time = (payloadLib->getCnfgArrayAtIndex( CAM_EVT_SCHD_STOP_TIME1 + 3*index1 + (MAX_CAM_EVNT_ACT_FLDS_SCH_MNG * (index)))).toString ().mid (0,2);
                            m_configResponse[CAM_EVT_SCHD_STOP_TIME1+(MAX_CAM_EVNT_ACT_FLDS_SCH_MNG * index) + 3*index1] =
                                    payloadLib->getCnfgArrayAtIndex( CAM_EVT_SCHD_STOP_TIME1 + 3*index1 + (MAX_CAM_EVNT_ACT_FLDS_SCH_MNG * (index)));
                            scheduleTimeing[index].stop_hour[index1] = time.toUInt ();

                            time = (payloadLib->getCnfgArrayAtIndex( CAM_EVT_SCHD_STOP_TIME1 + 3*index1 + (MAX_CAM_EVNT_ACT_FLDS_SCH_MNG * (index)))).toString ().mid (2,2);
                            scheduleTimeing[index].stop_min[index1] = time.toUInt ();
                        }
                    }
                }

                for(quint8 dayindex = 0; dayindex < MAX_CAM_EVNT_WEEKDAYS; dayindex++)
                {
                    actionStatus[dayindex*MAX_CAM_EVNT_WEEKDAYS] = (payloadLib->getCnfgArrayAtIndex(CAM_EVT_SCHD_ACTION_STATUS_ENTIRE_DAY + dayindex*MAX_CAM_EVNT_ACT_FLDS_SCH_MNG)).toUInt();
                    m_configResponse[CAM_EVT_SCHD_ACTION_STATUS_ENTIRE_DAY + dayindex * MAX_CAM_EVNT_ACT_FLDS_SCH_MNG] = actionStatus[dayindex*MAX_CAM_EVNT_WEEKDAYS];

                    for(quint8 index = 1; index < MAX_CAM_EVNT_WEEKDAYS; index++)
                    {
                        actionStatus[index + (dayindex * MAX_CAM_EVNT_WEEKDAYS)] = (payloadLib->getCnfgArrayAtIndex( CAM_EVT_SCHD_ACTION_STATUS_TIME1 + (MAX_CAM_EVNT_ACT_FLDS_SCH_MNG * (dayindex)) + (index-1)*3 )).toUInt ();
                        m_configResponse[CAM_EVT_SCHD_ACTION_STATUS_TIME1 + ((MAX_CAM_EVNT_ACT_FLDS_SCH_MNG * (dayindex)) + (index-1)*3)] = actionStatus[index + (dayindex * MAX_CAM_EVNT_WEEKDAYS)];
                    }
                }

                updateScheduleBar();
            }

            processBar->unloadProcessBar ();
        }
        break;

        case MSG_SET_CFG:
        {
            processBar->unloadProcessBar ();
            MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
            currentCameraIndex = cameraListDropDownBox->getIndexofCurrElement ();
            currentEventType = eventListDropDownBox->getIndexofCurrElement();
            getConfig ();
        }
        break;

        case MSG_DEF_CFG:
        {
            processBar->unloadProcessBar ();
            currentCameraIndex = cameraListDropDownBox->getIndexofCurrElement ();
            currentEventType = eventListDropDownBox->getIndexofCurrElement();
            getConfig ();
        }
        break;

        case MSG_SET_CMD:
        {
            if (param->cmdType != OTHR_SUP)
            {
                break;
            }

            payloadLib->parseDevCmdReply(true, param->payload);
            isMotionSupport = payloadLib->getCnfgArrayAtIndex (OTHR_SUP_MOTION).toBool ();
            isTemperSupport = payloadLib->getCnfgArrayAtIndex (OTHR_SUP_TEMPER).toBool ();
            isPTZSupport = payloadLib->getCnfgArrayAtIndex (OTHR_SUP_PTZ).toBool ();
            isAudioSupport = payloadLib->getCnfgArrayAtIndex (OTHR_SUP_AUDIO).toBool ();
            maxSensor = payloadLib->getCnfgArrayAtIndex (OTHR_SUP_SENSOR).toUInt ();
            maxAlarm =  payloadLib->getCnfgArrayAtIndex (OTHR_SUP_ALARM).toUInt ();
            isLineDetectionSupport = payloadLib->getCnfgArrayAtIndex(OTHR_SUP_LINE_DETECT).toBool();
            isIntrusionSupport = payloadLib->getCnfgArrayAtIndex(OTHR_SUP_INTRUCTION).toBool();
            isAudioExceptionSupport = payloadLib->getCnfgArrayAtIndex(OTHR_SUP_AUDIO_EXCEPTION).toBool();
            isMissingObjectSupport = payloadLib->getCnfgArrayAtIndex(OTHR_SUP_MISSING_OBJECT).toBool();
            isSuspiousObjectSupport = payloadLib->getCnfgArrayAtIndex(OTHR_SUP_SUSPIOUS_OBJECT).toBool();
            isLoteringSupport = payloadLib->getCnfgArrayAtIndex(OTHR_SUP_LOITERING).toBool();
            isObjectCountingSupport = payloadLib->getCnfgArrayAtIndex(OTHR_SUP_OBJECT_COUNTING).toBool();
            isNoMotionSupport = payloadLib->getCnfgArrayAtIndex(OTHR_SUP_NO_MOTION).toBool();
            fillOtherSupportData ();
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void CameraEventAndEventAction::fillOtherSupportData ()
{
    QMap<quint8, QString>  eventlist;
    quint8 insertIndex = 0;
    eventlist.clear ();

    for(quint8 index = 0; index < cameraEventList.length (); index++)
    {
        switch(index)
        {
            case CAM_EVNT_MOTION_DETECTION:
                if(isMotionSupport)
                {
                    eventlist.insert (insertIndex++,cameraEventList.at (index));
                }
                break;

            case CAM_EVNT_VIEW_TEMPER:
                if(isTemperSupport)
                {
                    eventlist.insert (insertIndex++,cameraEventList.at (index));
                }
                break;

            case CAM_EVNT_SENSOR_INPUT1:
            case CAM_EVNT_SENSOR_INPUT2:
            case CAM_EVNT_SENSOR_INPUT3:
                if(maxSensor != 0)
                {
                    eventlist.insert (insertIndex++,cameraEventList.at (index));
                    maxSensor--;
                }
                break;

            case CAM_EVNT_CONNECTION_FAILURE:
            case CAM_EVNT_RECORDING_FAIL:
            case CAM_EVNT_RECORDING_START:
                eventlist.insert (insertIndex++,cameraEventList.at (index));
                break;

            case CAM_EVNT_LINE_CROSSING:
                if(isLineDetectionSupport)
                {
                    eventlist.insert (insertIndex++,cameraEventList.at (index));
                }
                break;

            case CAM_EVNT_INTRUSION_DETECTION:
                if(isIntrusionSupport)
                {
                    eventlist.insert (insertIndex++,cameraEventList.at (index));
                }
                break;

            case CAM_EVNT_AUDIO_EXCEPTION_DETECTION:
                if(isAudioExceptionSupport)
                {
                    eventlist.insert (insertIndex++,cameraEventList.at (index));
                }
                break;

            case CAM_EVT_MISSING_OBJECT:
                if(isMissingObjectSupport)
                {
                    eventlist.insert (insertIndex++,cameraEventList.at (index));
                }
                break;

            case CAM_EVT_SUSPIOUS_OBJECT:
                if(isSuspiousObjectSupport)
                {
                    eventlist.insert (insertIndex++,cameraEventList.at (index));
                }
                break;

            case CAM_EVT_LOITREING:
                if(isLoteringSupport)
                {
                    eventlist.insert (insertIndex++,cameraEventList.at (index));
                }
                break;

            case CAM_EVT_CAMERA_ONLINE:
                eventlist.insert (insertIndex++,cameraEventList.at (index));
                break;

            case CAM_EVT_OBJECT_COUNTING:
                if(isObjectCountingSupport)
                {
                    eventlist.insert (insertIndex++,cameraEventList.at (index));
                }
                break;

            case CAM_EVNT_NO_MOTION_DETECTION:
                if(isNoMotionSupport)
                {
                    eventlist.insert (insertIndex++,cameraEventList.at (index));
                }
                break;

            default:
                break;
        }
    }

    eventListDropDownBox->setNewList (eventlist,currentEventType);
    currentEventStr = eventListDropDownBox->getCurrValue ();
    getConfig1 ();
}

void CameraEventAndEventAction::deleteSubObejects ()
{
    switch(m_currentElement)
    {
        case CAM_EVT_WEEKDAYS_SET_CTRL:
        {
            if(IS_VALID_OBJ(eventActionSchedule))
            {
                disconnect (eventActionSchedule,
                            SIGNAL(sigDeleteObject(quint8)),
                            this,
                            SLOT(slotSubObjectDelete(quint8)));
                DELETE_OBJ(eventActionSchedule);
            }
        }
        break;

        case CAM_EVT_EVENT_ALARM_SET_CTRL:
        {
            if(IS_VALID_OBJ(cameraSelectForAlarm))
            {
                disconnect (cameraSelectForAlarm,
                            SIGNAL(sigDeleteObject(quint8)),
                            this,
                            SLOT(slotSubObjectDelete(quint8)));
                DELETE_OBJ(cameraSelectForAlarm);
            }
        }
        break;

        case CAM_EVT_EVENT_IMG_UPLD_SET_CTRL:
        {
            if(IS_VALID_OBJ(cameraSelectForImageUpload))
            {
                disconnect (cameraSelectForImageUpload,
                            SIGNAL(sigDeleteObject(quint8)),
                            this,
                            SLOT(slotSubObjectDelete(quint8)));
                DELETE_OBJ(cameraSelectForImageUpload);
            }
        }
        break;

        case CAM_EVT_EVENT_EMAIL_SET_CTRL:
        {
            if(IS_VALID_OBJ(eventActionEmailNotify))
            {
                disconnect (eventActionEmailNotify,
                            SIGNAL(sigDeleteObject(quint8)),
                            this,
                            SLOT(slotSubObjectDelete(quint8)));
                DELETE_OBJ(eventActionEmailNotify);
            }
        }
        break;

        case CAM_EVT_EVENT_TCP_SET_CTRL:
        {
            if(IS_VALID_OBJ(eventActionTCPNotify))
            {
                disconnect (eventActionTCPNotify,
                            SIGNAL(sigDeleteObject(quint8)),
                            this,
                            SLOT(slotSubObjectDelete(quint8)));
                DELETE_OBJ(eventActionTCPNotify);
            }
        }
        break;

        case CAM_EVT_EVENT_SMS_SET_CTRL:
        {
            if(IS_VALID_OBJ(eventActionSmsNotify))
            {
                disconnect (eventActionSmsNotify,
                            SIGNAL(sigDeleteObject(quint8)),
                            this,
                            SLOT(slotSubObjectDelete(quint8)));
                DELETE_OBJ(eventActionSmsNotify);
            }
        }
        break;

        case CAM_EVT_EVENT_PRESET_SET_CTRL:
        {
            if(IS_VALID_OBJ(eventActionPtzPos))
            {
                disconnect (eventActionPtzPos,
                            SIGNAL(sigDeleteObject(quint8)),
                            this,
                            SLOT(slotSubObjectDelete(quint8)));
                DELETE_OBJ(eventActionPtzPos);
            }
        }
        break;

        case CAM_EVT_EVENT_DEVICE_ALARM_SET_CTRL:
        {
            if(IS_VALID_OBJ(eventActionDeviceAlarm))
            {
                disconnect (eventActionDeviceAlarm,
                            SIGNAL(sigDeleteObject(quint8)),
                            this,
                            SLOT(slotSubObjectDelete(quint8)));
                DELETE_OBJ(eventActionDeviceAlarm);
            }
        }
        break;

        case CAM_EVT_EVENT_CAMERA_ALARM_SET_CTRL:
        {
            if(IS_VALID_OBJ(eventCameraAlarmOutput))
            {
                disconnect (eventCameraAlarmOutput,
                            SIGNAL(sigDeleteObject(quint8)),
                            this,
                            SLOT(slotSubObjectDelete(quint8)));
                DELETE_OBJ(eventCameraAlarmOutput);
            }
        }
        break;

        case CAM_EVT_COPY_TO_CAM_CTRL:
        {
            if(IS_VALID_OBJ(eventCopyToCamera))
            {
                disconnect (eventCopyToCamera,
                            SIGNAL(sigDeleteObject(quint8)),
                            this,
                            SLOT(slotSubObjectDelete(quint8)));
                DELETE_OBJ(eventCopyToCamera);
            }
        }
        break;

        case CAM_EVT_EVENT_BUZZER_SET_CTRL:
        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void CameraEventAndEventAction::enableControlsOnAction(bool status )
{
    for(quint8 index = 0; index < MAX_CAM_EVNT_SCHEDULE_BAR; index++)
    {
        if (index < (MAX_CAM_EVNT_EVENTS - 2))
        {
            setButtons[index]->setIsEnabled (status);
        }

        if(index > 0)
        {
            scheduleBar[index]->setIsEnable (status);
        }

        if((index == DEV_ALARM_INDX) && (suppAlarms == 0) && (suppSensors == 0))
        {
            scheduleBar[index]->setIsEnable(false);
            setButtons[index]->setIsEnabled(false);
        }
    }

    #if defined(OEM_JCI)
    scheduleBar[EVNT_SMS]->setIsEnable(false);
    setButtons[EVNT_SMS]->setIsEnabled(false);
    scheduleBar[EVNT_TCP]->setIsEnable(false);
    setButtons[EVNT_TCP]->setIsEnabled(false);
    #endif
}

void CameraEventAndEventAction::slotSpinBoxValueChanged (QString, quint32 index)
{
    switch(index)
    {
        case CAM_EVT_CAMERALIST_SPINBOX_CTRL:
        {
            if(isUserChangeConfig())
            {
                infoPage->loadInfoPage(ValidationMessage::getValidationMessage(SAVE_CHANGES),true,false,"",CONFORMATION_BTN_YES,CONFORMATION_BTN_NO);
            }
            else
            {
                currentCameraIndex = cameraListDropDownBox->getIndexofCurrElement ();
                currentEventType = eventListDropDownBox->getIndexofCurrElement();
                getConfig ();
            }
        }
        break;

        case CAM_EVT_EVENTLIST_SPINBOX_CTRL:
        {
            if(isUserChangeConfig())
            {
                infoPage->loadInfoPage(ValidationMessage::getValidationMessage(SAVE_CHANGES),true,false,"",CONFORMATION_BTN_YES,CONFORMATION_BTN_NO);
            }
            else
            {
                currentEventType = eventListDropDownBox->getIndexofCurrElement ();
                currentEventStr = eventListDropDownBox->getCurrValue ();
                getConfig ();
            }
        }
        break;

        case CAM_EVT_WEEKDAYS_SPINBOX_CTRL:
        {
            updateScheduleBar();
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void CameraEventAndEventAction::slotCheckBoxValueChanged (OPTION_STATE_TYPE_e status, int)
{
    enableControlsOnAction ((status == ON_STATE) ? true : false);
}

void CameraEventAndEventAction::slotButtonClick (int index)
{
    switch(index)
    {
        case CAM_EVT_WEEKDAYS_SET_CTRL:
        {
            currentDayIndex = weekdayDropDownBox->getIndexofCurrElement ();

            if (IS_VALID_OBJ(eventActionSchedule))
            {
                break;
            }

            eventActionSchedule = new EventActionSchedule (index,
                                                           currentDayIndex,
                                                           scheduleTimeing,
                                                           isScheduleSetForEntireDay,
                                                           actionStatus,
                                                           parentWidget(),
                                                           devTableInfo,
                                                           (videoPopupSupportedEventList.contains(eventListDropDownBox->getCurrValue()) ?
                                                           MX_VIDEOPOPUP_UPDATE_MODE : MX_VIDEOPOPUP_DISABLE_MODE));
            connect (eventActionSchedule,
                     SIGNAL(sigDeleteObject(quint8)),
                     this,
                     SLOT(slotSubObjectDelete(quint8)));
        }
        break;

        case CAM_EVT_EVENT_ALARM_SET_CTRL:
        {
            if (IS_VALID_OBJ(cameraSelectForAlarm))
            {
                break;
            }

            /* Check if any mask bit is set or not */
            if (TRUE == IS_ALL_CAMERA_MASK_BIT_CLR(alarmRecordingSelectedCamera))
            {
                SET_CAMERA_MASK_BIT(alarmRecordingSelectedCamera, (cameraListDropDownBox->getIndexofCurrElement()));
            }

            cameraSelectForAlarm = new CopyToCamera(cameraList,
                                                    alarmRecordingSelectedCamera,
                                                    parentWidget (),
                                                    cameraEventAndEventActionStrings
                                                    [CAM_EVT_EVENT_ACTION_LABEL],
                                                    CAM_EVT_EVENT_ALARM_SET_CTRL);
            connect (cameraSelectForAlarm,
                     SIGNAL(sigDeleteObject(quint8)),
                     this,
                     SLOT(slotSubObjectDelete(quint8)));
        }
        break;

        case CAM_EVT_EVENT_IMG_UPLD_SET_CTRL:
        {
            if (IS_VALID_OBJ(cameraSelectForImageUpload))
            {
                break;
            }

            /* Check if any mask bit is set or not */
            if (TRUE == IS_ALL_CAMERA_MASK_BIT_CLR(uploadImageSelectedCamera))
            {
                SET_CAMERA_MASK_BIT(uploadImageSelectedCamera, (cameraListDropDownBox->getIndexofCurrElement()));
            }

            cameraSelectForImageUpload = new CopyToCamera(cameraList,
                                                          uploadImageSelectedCamera,
                                                          parentWidget (),
                                                          cameraEventAndEventActionStrings
                                                          [CAM_EVT_EVENT_ACTION_LABEL + 1],
                                                          CAM_EVT_EVENT_IMG_UPLD_SET_CTRL);
            connect (cameraSelectForImageUpload,
                     SIGNAL(sigDeleteObject(quint8)),
                     this,
                     SLOT(slotSubObjectDelete(quint8)));
        }
        break;

        case CAM_EVT_EVENT_EMAIL_SET_CTRL:
        {
            if (IS_VALID_OBJ(eventActionEmailNotify))
            {
                break;
            }

            eventActionEmailNotify = new EventActionEmailNotify(CAM_EVT_EVENT_EMAIL_SET_CTRL,
                                                                emailAddress,
                                                                emailSubject,
                                                                emailMessage,
                                                                parentWidget ());
            connect (eventActionEmailNotify,
                     SIGNAL(sigDeleteObject(quint8)),
                     this,
                     SLOT(slotSubObjectDelete(quint8)));
        }
        break;

        case CAM_EVT_EVENT_TCP_SET_CTRL:
        {
            if (IS_VALID_OBJ(eventActionTCPNotify))
            {
                break;
            }

            eventActionTCPNotify = new EventActionTCPNotify(CAM_EVT_EVENT_TCP_SET_CTRL,
                                                            tcpMessage,
                                                            parentWidget ());
            connect (eventActionTCPNotify,
                     SIGNAL(sigDeleteObject(quint8)),
                     this,
                     SLOT(slotSubObjectDelete(quint8)));
        }
        break;

        case CAM_EVT_EVENT_SMS_SET_CTRL:
        {
            if (IS_VALID_OBJ(eventActionSmsNotify))
            {
                break;
            }

            eventActionSmsNotify = new EventActionSmsNotify(CAM_EVT_EVENT_SMS_SET_CTRL,
                                                            smsMobileNum1,
                                                            smsMobileNum2,
                                                            smsMessage,
                                                            parentWidget ());
            connect (eventActionSmsNotify,
                     SIGNAL(sigDeleteObject(quint8)),
                     this,
                     SLOT(slotSubObjectDelete(quint8)));
        }
        break;

        case CAM_EVT_EVENT_PRESET_SET_CTRL:
        {
            if (IS_VALID_OBJ(eventActionPtzPos))
            {
                break;
            }

            eventActionPtzPos = new EventActionPtzPos(CAM_EVT_EVENT_PRESET_SET_CTRL,
                                                      currDevName,
                                                      presetCameraNum,
                                                      presetGotoPosition,
                                                      devTableInfo->totalCams,
                                                      parentWidget ());
            connect (eventActionPtzPos,
                     SIGNAL(sigDeleteObject(quint8)),
                     this,
                     SLOT(slotSubObjectDelete(quint8)));
        }
        break;

        case CAM_EVT_EVENT_DEVICE_ALARM_SET_CTRL:
        {
            if (IS_VALID_OBJ(eventActionDeviceAlarm))
            {
                break;
            }

            eventActionDeviceAlarm = new EventActionDeviceAlarm(CAM_EVT_EVENT_DEVICE_ALARM_SET_CTRL,
                                                                deviceAlaram,
                                                                devTableInfo,
                                                                parentWidget ());
            connect (eventActionDeviceAlarm,
                     SIGNAL(sigDeleteObject(quint8)),
                     this,
                     SLOT(slotSubObjectDelete(quint8)));
        }
        break;

        case CAM_EVT_EVENT_CAMERA_ALARM_SET_CTRL:
        {
            if (IS_VALID_OBJ(eventCameraAlarmOutput))
            {
                break;
            }

            eventCameraAlarmOutput = new EventCameraAlarmOutput(CAM_EVT_EVENT_CAMERA_ALARM_SET_CTRL,
                                                                cameraList,
                                                                cameraAlarm,
                                                                currentCamForAlarm,
                                                                parentWidget ());
            connect (eventCameraAlarmOutput,
                     SIGNAL(sigDeleteObject(quint8)),
                     this,
                     SLOT(slotSubObjectDelete(quint8)));
        }
        break;

        case CAM_EVT_COPY_TO_CAM_CTRL:
        {
            if (IS_VALID_OBJ(eventCopyToCamera))
            {
                break;
            }

            memset(&eventSelectedCamera, 0, sizeof(eventSelectedCamera));
            SET_CAMERA_MASK_BIT(eventSelectedCamera, (cameraListDropDownBox->getIndexofCurrElement()));
            for(quint8 maskIdx = 0; maskIdx < CAMERA_MASK_MAX; maskIdx++)
            {
                m_configResponse[CAM_EVE_COPY_TO_FEILD_START + maskIdx] = eventSelectedCamera.bitMask[maskIdx];
            }

            eventCopyToCamera = new CopyToCamera(cameraList,
                                                 eventSelectedCamera,
                                                 parentWidget (),
                                                 "Camera Event and Action Settings",
                                                 CAM_EVT_COPY_TO_CAM_CTRL,
                                                 true);
            connect (eventCopyToCamera,
                     SIGNAL(sigDeleteObject(quint8)),
                     this,
                     SLOT(slotSubObjectDelete(quint8)));
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }

    m_currentElement = index;
}

void CameraEventAndEventAction::slotSubObjectDelete(quint8 index)
{
    m_currentElement = index;
    deleteSubObejects();
    m_elementList[m_currentElement]->forceActiveFocus();

    if(index == CAM_EVT_WEEKDAYS_SET_CTRL)
    {
        updateScheduleBar ();
    }
}
