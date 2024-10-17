#include "SensorEventAndAction.h"
#include "ValidationMessage.h"

#define MAX_SENSOR_EVENT_AND_ACTION_END_FEILDS  SENS_EVT_SCHD_ENTIRE_DAY
#define MAX_SENSOR_EVNT_ACT_FLDS_SCH_MNG        (MAX_SENSOR_EVENT_AND_ACTION_FEILDS - SENS_EVT_SCHD_ENTIRE_DAY)
#define CNFG_FEILD_FOR_SENSOR_LIST              2
#define MAX_SES_EVNT_SCHEDULE_BAR               MAX_SENS_EVNT_EVENTS

typedef enum
{
    SENS_EVT_SENSORLIST_SPINBOX_CTRL,
    SENS_EVT_EVENTLIST_SPINBOX_CTRL,
    SENS_EVT_ACTION_CHECKBOX_CTRL,
    SENS_EVT_WEEKDAYS_SPINBOX_CTRL,
    SENS_EVT_WEEKDAYS_SET_CTRL,
    SENS_EVT_EVENT_ALARM_SET_CTRL,
    SENS_EVT_EVENT_IMG_UPLD_SET_CTRL,
    SENS_EVT_EVENT_EMAIL_SET_CTRL,
    SENS_EVT_EVENT_TCP_SET_CTRL,
    SENS_EVT_EVENT_SMS_SET_CTRL,
    SENS_EVT_EVENT_PRESET_SET_CTRL,
    SENS_EVT_EVENT_DEVICE_ALARM_SET_CTRL,
    SENS_EVT_EVENT_CAMERA_ALARM_SET_CTRL,
    SENS_EVT_EVENT_BUZZER_SET_CTRL,
    SENS_EVT_EVENT_PUSH_NOTIFICATION_SET_CTRL,
    MAX_SENSOR_EVENT_AND_ACTION_CTRL
}SENSOR_EVENT_AND_ACTION_CTRL_e;

typedef enum
{
    SENS_EVT_SENSORLIST_SPINBOX_LABEL,
    SENS_EVT_EVENTLIST_SPINBOX_LABEL,
    SENS_EVT_ACTION_CHECKBOX_LABEL,
    SENS_EVT_ACTION_SCHD_HEADING,
    SENS_EVT_WEEKDAYS_SPINBOX_LABEL,
    SENS_EVT_EVENT_ACTION_LABEL,

    MAX_SENSOR_EVENT_AND_ACTION_STRINGS = (SENS_EVT_EVENT_ACTION_LABEL + MAX_SENS_EVNT_EVENTS)
}SENSOR_EVENT_AND_ACTION_STRINGS_e;

static const QString sensorEventAndEventActionStrings[MAX_SENSOR_EVENT_AND_ACTION_STRINGS] =
{
    "Sensor",
    "Event Type",
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
    "Push Notification",
};

typedef enum
{
    SENS_EVE_ENABLE_ACTION = 0,
    SENS_EVE_CAM_NUM_ALARM_REC_START,
    SENS_EVE_CAM_NUM_ALARM_REC_END = SENS_EVE_CAM_NUM_ALARM_REC_START + CAMERA_MASK_MAX - 1,
    SENS_EVE_CAM_NUM_UPLOAD_REC_START,
    SENS_EVE_CAM_NUM_UPLOAD_REC_END = SENS_EVE_CAM_NUM_UPLOAD_REC_START + CAMERA_MASK_MAX - 1,
    SENS_EVE_EMAIL_ADDRESS,
    SENS_EVE_EMAIL_SUBJECT,
    SENS_EVE_EMAIL_MESSAGE,
    SENS_EVE_TCP_MESSAGE,
    SENS_EVE_SMS_MOB1,
    SENS_EVE_SMS_MOB2,
    SENS_EVE_SMS_MESSAGE,
    SENS_EVE_PTZ_CAM_NUM,
    SENS_EVE_PTZ_PRE_POS,
    SENS_EVE_SYS_ALM_OP_PORT1,
    SENS_EVE_SYS_ALM_OP_PORT2,
    SENS_EVE_SYS_ALM_OP_PORT3,
    SENS_EVE_SYS_ALM_OP_PORT4,
    SENS_EVE_CAM_ALM_CAM_NUM,
    SENS_EVE_CAM_ALM_ENB1,
    SENS_EVE_CAM_ALM_ENB2,
    SENS_EVE_CAM_ALM_ENB3,

    SENS_EVT_SCHD_ENTIRE_DAY,
    SENS_EVT_SCHD_ACTION_STATUS_ENTIRE_DAY,
    SENS_EVT_SCHD_START_TIME1,
    SENS_EVT_SCHD_STOP_TIME1,
    SENS_EVT_SCHD_ACTION_STATUS_TIME1,
    SENS_EVT_SCHD_START_TIME2,
    SENS_EVT_SCHD_STOP_TIME2,
    SENS_EVT_SCHD_ACTION_STATUS_TIME2,
    SENS_EVT_SCHD_START_TIME3,
    SENS_EVT_SCHD_STOP_TIME3,
    SENS_EVT_SCHD_ACTION_STATUS_TIME3,
    SENS_EVT_SCHD_START_TIME4,
    SENS_EVT_SCHD_STOP_TIME4,
    SENS_EVT_SCHD_ACTION_STATUS_TIME4,
    SENS_EVT_SCHD_START_TIME5,
    SENS_EVT_SCHD_STOP_TIME5,
    SENS_EVT_SCHD_ACTION_STATUS_TIME5,
    SENS_EVT_SCHD_START_TIME6,
    SENS_EVT_SCHD_STOP_TIME6,
    SENS_EVT_SCHD_ACTION_STATUS_TIME6,

    MAX_SENSOR_EVENT_AND_ACTION_FEILDS
}SENSOR_EVENT_AND_ACTION_FEILDS_e;

static const QStringList weekdayList = QStringList() << "Sun" << "Mon" << "Tue" << "Wed" << "Thu" << "Fri" << "Sat";

SensorEventAndAction::SensorEventAndAction(QString deviceName, QWidget* parent, DEV_TABLE_INFO_t *devTabInfo)
    : ConfigPageControl(deviceName, parent,MAX_SENSOR_EVENT_AND_ACTION_CTRL,devTabInfo)
{
    initlizeVariables();
    currentSensorIndex = 0;
    createDefaultComponents ();
    getSensorList();
}

SensorEventAndAction::~SensorEventAndAction ()
{
    disconnect (sensorListDropDownBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (sensorListDropDownBox,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotSpinBoxValueChanged(QString,quint32)));
    delete sensorListDropDownBox;

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

    for(quint8 index = 0; index < MAX_SENS_EVNT_SCHEDULE_BAR; index++)
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

    deleteSubObejects();
}

void SensorEventAndAction::initlizeVariables ()
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
    cnfg1FrmIndx = 0;
    cnfg1ToIndx = 0;
    cnfg2FrmIndx = 0;
    cnfg2ToIndx = 0;
    m_currentDayIndex = 0;
    presetCameraNum = 0;
    presetGotoPosition = 0;
    totalCamera = 0;
    memset(&alarmRecordingSelectedCamera, 0, sizeof(alarmRecordingSelectedCamera));
    memset(&uploadImageSelectedCamera, 0, sizeof(uploadImageSelectedCamera));

    memset(&scheduleTimeing, 0, sizeof(scheduleTimeing));
    memset(&actionStatus, 0, sizeof(actionStatus));

    for(quint8 index = 0; index < MAX_SENS_EVNT_WEEKDAYS;index++)
    {
        isScheduleSetForEntireDay[index] = false;
        isDaySelectForSchedule[index] = false;
    }

    for(quint8 index = 0; index < MAX_CAM_ALARM;index++)
    {
        cameraAlarm[index] = false;
        deviceAlaram[index] = false;
    }
}

void SensorEventAndAction::createDefaultComponents ()
{
    fillCameraList();
    currentCamForAlarm = 1;

    memset(&scheduleTimeing, 0, sizeof(scheduleTimeing));

    sensorList.clear ();
    for(quint8 index = 0; index < devTableInfo->sensors; index++)
    {
        /* e.g: "1 : Sensor - 1", "2 : Sensor - 2" */
        sensorList.insert(index, QString("%1%2%3").arg(index+1).arg(" : Sensor - ").arg(index+1));
    }

    m_elementList[SENS_EVT_SENSORLIST_SPINBOX_CTRL] = NULL;
    sensorListDropDownBox = new DropDown(SCALE_WIDTH(13),
                                         SCALE_HEIGHT(30),
                                         BGTILE_LARGE_SIZE_WIDTH,
                                         BGTILE_HEIGHT,
                                         SENS_EVT_EVENTLIST_SPINBOX_CTRL,
                                         DROPDOWNBOX_SIZE_320,
                                         sensorEventAndEventActionStrings[SENS_EVT_SENSORLIST_SPINBOX_LABEL],
                                         sensorList,
                                         this,
                                         "",
                                         false,
                                         SCALE_WIDTH(15));
    m_elementList[SENS_EVT_EVENTLIST_SPINBOX_CTRL] = sensorListDropDownBox;
    connect (sensorListDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (sensorListDropDownBox,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotSpinBoxValueChanged(QString,quint32)));

    actionEventCheckbox = new OptionSelectButton(sensorListDropDownBox->x (),
                                                 sensorListDropDownBox->y () + BGTILE_HEIGHT + SCALE_HEIGHT(5),
                                                 BGTILE_LARGE_SIZE_WIDTH,
                                                 BGTILE_HEIGHT,
                                                 CHECK_BUTTON_INDEX,
                                                 this,
                                                 UP_LAYER,
                                                 "",
                                                 "",
                                                 SCALE_WIDTH(10),
                                                 SENS_EVT_ACTION_CHECKBOX_CTRL);
    m_elementList[SENS_EVT_ACTION_CHECKBOX_CTRL] = actionEventCheckbox;
    connect (actionEventCheckbox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (actionEventCheckbox,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotCheckBoxValueChanged(OPTION_STATE_TYPE_e,int)));

    actionSchdHeading = new ElementHeading(sensorListDropDownBox->x () + SCALE_WIDTH(40),
                                           actionEventCheckbox->y (),
                                           BGTILE_LARGE_SIZE_WIDTH,
                                           BGTILE_HEIGHT,
                                           sensorEventAndEventActionStrings[SENS_EVT_ACTION_SCHD_HEADING],
                                           NO_LAYER,
                                           this,
                                           false,
                                           0, NORMAL_FONT_SIZE, true);

    QMap<quint8, QString>  weekdayMapList;
    for(quint8 index = 0; index < weekdayList.length (); index++)
    {
        weekdayMapList.insert (index,weekdayList.at (index));
    }

    weekdayDropDownBox = new DropDown(sensorListDropDownBox->x (),
                                      actionSchdHeading->y () + BGTILE_HEIGHT,
                                      BGTILE_LARGE_SIZE_WIDTH,
                                      BGTILE_HEIGHT,
                                      SENS_EVT_WEEKDAYS_SPINBOX_CTRL,
                                      DROPDOWNBOX_SIZE_90,
                                      sensorEventAndEventActionStrings[SENS_EVT_WEEKDAYS_SPINBOX_LABEL],
                                      weekdayMapList,
                                      this,
                                      "",
                                      false,
                                      SCALE_WIDTH(19),
                                      MIDDLE_TABLE_LAYER);
    m_elementList[SENS_EVT_WEEKDAYS_SPINBOX_CTRL] = weekdayDropDownBox;
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
                                       SENS_EVT_WEEKDAYS_SET_CTRL,
                                       PAGEOPENBUTTON_SMALL,
                                       "Set",
                                       this,
                                       "","",false,0,NO_LAYER,false);
    m_elementList[SENS_EVT_WEEKDAYS_SET_CTRL] = setButtons[0];
    connect (setButtons[0],
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (setButtons[0],
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClick(int)));

    for(quint8 index = 0; index < MAX_SENS_EVNT_SCHEDULE_BAR; index++)
    {
        scheduleBar[index] = new ScheduleBar(sensorListDropDownBox->x (),
                                             weekdayDropDownBox->y () +
                                             (index + 1)*BGTILE_HEIGHT,
                                             BGTILE_LARGE_SIZE_WIDTH,
                                             BGTILE_HEIGHT,
                                             index < 8 ? MIDDLE_TABLE_LAYER : BOTTOM_TABLE_LAYER,
                                             index == 0 ? "" : sensorEventAndEventActionStrings[SENS_EVT_EVENT_ACTION_LABEL + (index - 1)],
                                             SCALE_WIDTH(10),
                                             this,
                                             (index == 0) ? true : false,
                                             (index == 0) ? true : false);
        if(index > 0 )
        {
            setButtons[index] = new PageOpenButton(BGTILE_LARGE_SIZE_WIDTH - SCALE_WIDTH(65),
                                                   scheduleBar[index]->y (),
                                                   BGTILE_LARGE_SIZE_WIDTH,
                                                   BGTILE_HEIGHT,
                                                   SENS_EVT_EVENT_ALARM_SET_CTRL + (index-1),
                                                   PAGEOPENBUTTON_SMALL,
                                                   "Set",
                                                   this,
                                                   "","",false,
                                                   0,
                                                   NO_LAYER,
                                                   false);
            m_elementList[ SENS_EVT_EVENT_ALARM_SET_CTRL + (index-1)] = setButtons[index];
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
	
    for(quint8 index = EVNT_VIDEO_POP_UP; index < MAX_SENS_EVNT_SCHEDULE_BAR; index++)
    {
        setButtons[index]->setVisible(false);
    }

    #if defined(OEM_JCI)
    scheduleBar[EVNT_SMS]->setVisible(false);
    setButtons[EVNT_SMS]->setVisible(false);
    scheduleBar[EVNT_TCP]->setVisible(false);
    setButtons[EVNT_TCP]->setVisible(false);

    for(quint8 index = EVNT_SMS + 1 ; index < MAX_SENS_EVNT_SCHEDULE_BAR; index++)
    {
        scheduleBar[index]->resetGeometry(scheduleBar[index]->x(),
                                          scheduleBar[index]->y() - (2 * BGTILE_HEIGHT),
                                          BGTILE_LARGE_SIZE_WIDTH,
                                          BGTILE_HEIGHT);
        setButtons[index]->resetGeometry(setButtons[index]->x(),
                                         setButtons[index]->y() - (2 * BGTILE_HEIGHT));
    }
    #endif
	
    resetGeometryOfCnfgbuttonRow (SCALE_HEIGHT(20));
}

void SensorEventAndAction::fillCameraList ()
{
    cameraList.clear();
    for(quint8 index = 0; index < devTableInfo->totalCams; index++)
    {
        cameraList.insert(index, QString("%1%2%3").arg(index + 1).arg(" : ").arg(applController->GetCameraNameOfDevice(currDevName, index)));
    }
}

void SensorEventAndAction::updateScheduleBar()
{
    SCHEDULE_TIMING_t tempSchd;

    memset(&tempSchd, 0, sizeof(SCHEDULE_TIMING_t));
    for(quint8 index = 1; index < MAX_SES_EVNT_SCHEDULE_BAR; index++)
    {
        scheduleBar[index]->setSchedule (tempSchd);
    }

    m_currentDayIndex =  weekdayDropDownBox->getIndexofCurrElement ();

    if(isScheduleSetForEntireDay[m_currentDayIndex] == true)
    {
        for(quint8 index = 0; index < EVNT_VIDEO_POP_UP; index++ )
        {
            if(actionStatus[MAX_SENS_EVNT_WEEKDAYS*m_currentDayIndex ] & (1 << index))
            {
                scheduleBar [EVNT_VIDEO_POP_UP - index ]->setScheduleForEntireDay (true);
            }
            else
            {
                scheduleBar [EVNT_VIDEO_POP_UP - index]->setScheduleForEntireDay (false);
            }
            scheduleBar [EVNT_VIDEO_POP_UP - index]->update ();
        }

        for(quint8 index = EVNT_PUSH_NOTIFICATION; index < MAX_EVNT_SCHD_EVNT; index++ )
        {
            if(actionStatus[MAX_SENS_EVNT_WEEKDAYS*m_currentDayIndex ] & (1 << index))
            {
                scheduleBar [index]->setScheduleForEntireDay (true);
            }
            else
            {
                scheduleBar [index]->setScheduleForEntireDay (false);
            }
            scheduleBar [index]->update ();
        }
    }
    else
    {
        for(quint8 index = 0; index < EVNT_VIDEO_POP_UP;index++ )
        {
            memset(&tempSchd, 0, sizeof(SCHEDULE_TIMING_t));
            for(quint8 index1 = 0; index1 < MAX_TIME_SLOT; index1++)
            {
                if(actionStatus[index1 + MAX_SENS_EVNT_WEEKDAYS*m_currentDayIndex + 1]  &  (1 << index))
                {
                    tempSchd.start_hour[index1] = scheduleTimeing[m_currentDayIndex].start_hour[index1];
                    tempSchd.stop_hour[index1] = scheduleTimeing[m_currentDayIndex].stop_hour[index1];
                    tempSchd.start_min[index1] = scheduleTimeing[m_currentDayIndex].start_min[index1];
                    tempSchd.stop_min[index1] = scheduleTimeing[m_currentDayIndex].stop_min[index1];
                }
            }

            scheduleBar [EVNT_VIDEO_POP_UP - index]->setSchedule (tempSchd);
            scheduleBar [EVNT_VIDEO_POP_UP - index]->update ();
        }

        for(quint8 index = EVNT_PUSH_NOTIFICATION; index < MAX_EVNT_SCHD_EVNT; index++ )
        {
            memset(&tempSchd, 0, sizeof(SCHEDULE_TIMING_t));
            for(quint8 index1 = 0; index1 < MAX_TIME_SLOT; index1++)
            {
                if(actionStatus[index1 + MAX_SENS_EVNT_WEEKDAYS*m_currentDayIndex + 1]  &  (1 << index))
                {
                    tempSchd.start_hour[index1] = scheduleTimeing[m_currentDayIndex].start_hour[index1];
                    tempSchd.stop_hour[index1] = scheduleTimeing[m_currentDayIndex].stop_hour[index1];
                    tempSchd.start_min[index1] = scheduleTimeing[m_currentDayIndex].start_min[index1];
                    tempSchd.stop_min[index1] = scheduleTimeing[m_currentDayIndex].stop_min[index1];
                }
            }

            scheduleBar [index]->setSchedule (tempSchd);
            scheduleBar [index]->update ();
        }
    }
}

void SensorEventAndAction::getSensorList ()
{
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                             SYSTEM_SENSOR_INPUT_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             MAX_DEV_SENSOR,
                                                             CNFG_FEILD_FOR_SENSOR_LIST,
                                                             CNFG_FEILD_FOR_SENSOR_LIST,
                                                             MAX_DEV_SENSOR);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_GET_CFG;
    param->payload = payloadString;

    processBar->loadProcessBar ();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void SensorEventAndAction::createPayload(REQ_MSG_ID_e msgType)
{
    cnfg1FrmIndx = currentSensorIndex + 1;
    cnfg2ToIndx = ((weekdayList.length ()*currentSensorIndex) + 1);
    cnfg2FrmIndx = cnfg2ToIndx + MAX_SENS_EVNT_WEEKDAYS - 1;

    QString payloadString = payloadLib->createDevCnfgPayload(msgType,
                                                             SENSOR_EVENT_ACTION_MANAGMENT_TABLE_INDEX,
                                                             cnfg1FrmIndx,
                                                             cnfg1FrmIndx,
                                                             CNFG_FRM_INDEX,
                                                             MAX_SENSOR_EVENT_AND_ACTION_END_FEILDS,
                                                             MAX_SENSOR_EVENT_AND_ACTION_END_FEILDS);

    payloadString = payloadLib->createDevCnfgPayload(msgType,
                                                     SENSOR_EVENT_ACTION_SCHDULE_TABLE_INDEX,
                                                     cnfg2ToIndx,
                                                     cnfg2FrmIndx,
                                                     CNFG_FRM_INDEX,
                                                     MAX_SENSOR_EVNT_ACT_FLDS_SCH_MNG,
                                                     weekdayList.length ()*
                                                     MAX_SENSOR_EVNT_ACT_FLDS_SCH_MNG,
                                                     payloadString,
                                                     MAX_SENSOR_EVENT_AND_ACTION_END_FEILDS);

    DevCommParam* param = new DevCommParam();
    param->msgType = msgType;
    param->payload = payloadString;

    processBar->loadProcessBar ();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void SensorEventAndAction::defaultConfig ()
{
    createPayload(MSG_DEF_CFG);
}

bool SensorEventAndAction::isUserChangeConfig()
{
    quint8 index;

    if(m_configResponse.isEmpty())
    {
        return false;
    }

    if((IS_VALID_OBJ(actionEventCheckbox)) && (m_configResponse[SENS_EVE_ENABLE_ACTION] != actionEventCheckbox->getCurrentState()))
    {
        return true;
    }

    for (quint8 maskIdx = 0; maskIdx < CAMERA_MASK_MAX; maskIdx++)
    {
        if(m_configResponse[SENS_EVE_CAM_NUM_ALARM_REC_START+maskIdx] != alarmRecordingSelectedCamera.bitMask[maskIdx])
        {
            return true;
        }

        if(m_configResponse[SENS_EVE_CAM_NUM_UPLOAD_REC_START+maskIdx] != uploadImageSelectedCamera.bitMask[maskIdx])
        {
            return true;
        }
    }

    if(m_configResponse[SENS_EVE_EMAIL_ADDRESS] != emailAddress)
    {
        return true;
    }

    if(m_configResponse[SENS_EVE_EMAIL_SUBJECT] != emailSubject)
    {
        return true;
    }

    if(m_configResponse[SENS_EVE_EMAIL_MESSAGE] != emailMessage)
    {
        return true;
    }

    if(m_configResponse[SENS_EVE_TCP_MESSAGE] != tcpMessage)
    {
        return true;
    }

    if(m_configResponse[SENS_EVE_SMS_MOB1] != smsMobileNum1)
    {
        return true;
    }

    if(m_configResponse[SENS_EVE_SMS_MOB2] != smsMobileNum2)
    {
        return true;
    }

    if(m_configResponse[SENS_EVE_SMS_MESSAGE] != smsMessage)
    {
        return true;
    }

    if(m_configResponse[SENS_EVE_PTZ_CAM_NUM] != presetCameraNum)
    {
        return true;
    }

    if(m_configResponse[SENS_EVE_PTZ_PRE_POS] != presetGotoPosition)
    {
        return true;
    }

    if(m_configResponse[SENS_EVE_CAM_ALM_CAM_NUM] != currentCamForAlarm)
    {
        return true;
    }

    for(index = 0; index < MAX_DEV_ALARM; index++)
    {
        if(m_configResponse[SENS_EVE_SYS_ALM_OP_PORT1+ index] != deviceAlaram[index])
        {
            return true;
        }
    }

    for(index = 0; index < MAX_CAM_ALARM; index++)
    {
        if(m_configResponse[SENS_EVE_CAM_ALM_ENB1+ index] != cameraAlarm[index])
        {
            return true;
        }
    }

    for(index = 0; index< MAX_SENS_EVNT_WEEKDAYS;index++)
    {
        if(m_configResponse[SENS_EVT_SCHD_ENTIRE_DAY + (index * MAX_SENSOR_EVNT_ACT_FLDS_SCH_MNG)] != isScheduleSetForEntireDay[index])
        {
            return true;
        }

        for(quint8 index1 = 0; index1 < MAX_SENS_SCHD_TIME_SLOTS;index1++)
        {
            if((m_configResponse[SENS_EVT_SCHD_START_TIME1+(MAX_SENSOR_EVNT_ACT_FLDS_SCH_MNG * index) +  3*index1].toString ().mid (0,2).toUInt ()) != scheduleTimeing[index].start_hour[index1])
            {
                return true;
            }

            if((m_configResponse[SENS_EVT_SCHD_START_TIME1+(MAX_SENSOR_EVNT_ACT_FLDS_SCH_MNG * index) +  3*index1].toString ().mid (2,2).toUInt ()) != scheduleTimeing[index].start_min[index1])
            {
                return true;
            }

            if((m_configResponse[SENS_EVT_SCHD_STOP_TIME1+(MAX_SENSOR_EVNT_ACT_FLDS_SCH_MNG * index) +  3*index1].toString ().mid (0,2).toUInt ()) != scheduleTimeing[index].stop_hour[index1])
            {
                return true;
            }

            if((m_configResponse[SENS_EVT_SCHD_STOP_TIME1+(MAX_SENSOR_EVNT_ACT_FLDS_SCH_MNG * index) +  3*index1].toString ().mid (2,2).toUInt ()) != scheduleTimeing[index].stop_min[index1])
            {
                return true;
            }
        }
    }

    for(quint8 dayindex = 0; dayindex < MAX_SENS_EVNT_WEEKDAYS;dayindex++)
    {
        if((m_configResponse[SENS_EVT_SCHD_ACTION_STATUS_ENTIRE_DAY + dayindex * MAX_SENSOR_EVNT_ACT_FLDS_SCH_MNG]) != actionStatus[dayindex*MAX_SENS_EVNT_WEEKDAYS])
        {
            return true;
        }

        for(index = 1; index < MAX_SENS_EVNT_WEEKDAYS; index++)
        {
            if((m_configResponse[SENS_EVT_SCHD_ACTION_STATUS_TIME1 + ((MAX_SENSOR_EVNT_ACT_FLDS_SCH_MNG * (dayindex)) + (index-1)*3)]) != actionStatus[index + (dayindex * MAX_SENS_EVNT_WEEKDAYS)])
            {
                return true;
            }
        }
    }

    return false;
}

void SensorEventAndAction::handleInfoPageMessage(int index)
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
            currentSensorIndex = sensorListDropDownBox->getIndexofCurrElement ();
            getConfig();
        }
    }
}

void SensorEventAndAction::getConfig ()
{
    initlizeVariables();
    createPayload(MSG_GET_CFG);
}

void SensorEventAndAction::setConfigFields()
{
    payloadLib->setCnfgArrayAtIndex (SENS_EVE_ENABLE_ACTION, actionEventCheckbox->getCurrentState ());

    for (quint8 maskIdx = 0; maskIdx < CAMERA_MASK_MAX; maskIdx++)
    {
        payloadLib->setCnfgArrayAtIndex (SENS_EVE_CAM_NUM_ALARM_REC_START+maskIdx, alarmRecordingSelectedCamera.bitMask[maskIdx]);
        payloadLib->setCnfgArrayAtIndex (SENS_EVE_CAM_NUM_UPLOAD_REC_START+maskIdx, uploadImageSelectedCamera.bitMask[maskIdx]);
    }

    payloadLib->setCnfgArrayAtIndex (SENS_EVE_EMAIL_ADDRESS,emailAddress);
    payloadLib->setCnfgArrayAtIndex (SENS_EVE_EMAIL_SUBJECT,emailSubject);
    payloadLib->setCnfgArrayAtIndex (SENS_EVE_EMAIL_MESSAGE,emailMessage);

    payloadLib->setCnfgArrayAtIndex (SENS_EVE_TCP_MESSAGE,tcpMessage);

    payloadLib->setCnfgArrayAtIndex (SENS_EVE_SMS_MOB1,smsMobileNum1);
    payloadLib->setCnfgArrayAtIndex (SENS_EVE_SMS_MOB2,smsMobileNum2);
    payloadLib->setCnfgArrayAtIndex (SENS_EVE_SMS_MESSAGE,smsMessage);

    payloadLib->setCnfgArrayAtIndex (SENS_EVE_PTZ_CAM_NUM,presetCameraNum);
    payloadLib->setCnfgArrayAtIndex (SENS_EVE_PTZ_PRE_POS,presetGotoPosition);

    payloadLib->setCnfgArrayAtIndex (SENS_EVE_CAM_ALM_CAM_NUM,currentCamForAlarm);

    for(quint8 index = 0; index < MAX_DEV_ALARM; index++)
    {
        payloadLib->setCnfgArrayAtIndex((SENS_EVE_SYS_ALM_OP_PORT1 + index),((deviceAlaram[index] == false) ? 0 : 1));
    }

    for(quint8 index = 0; index < MAX_CAM_ALARM; index++)
    {
        payloadLib->setCnfgArrayAtIndex((SENS_EVE_CAM_ALM_ENB1 + index),cameraAlarm[index] == false ? 0 : 1);
    }
}

void SensorEventAndAction::fillRecords()
{
    QString stopTime = "";
    QString startTime = "";

    for(quint8 index = 0;index < MAX_SENS_EVNT_WEEKDAYS;index++)
    {
        if(isScheduleSetForEntireDay[index])
        {
            payloadLib->setCnfgArrayAtIndex (SENS_EVT_SCHD_ENTIRE_DAY + ((index)*MAX_SENSOR_EVNT_ACT_FLDS_SCH_MNG),1);

        }
        else
        {
            payloadLib->setCnfgArrayAtIndex (SENS_EVT_SCHD_ENTIRE_DAY + ((index)*MAX_SENSOR_EVNT_ACT_FLDS_SCH_MNG),0);

            for(quint8 index1 = 0; index1 < MAX_SENS_SCHD_TIME_SLOTS;index1++)
            {
                if(scheduleTimeing[index].start_hour[index1] > 24)
                {
                    scheduleTimeing[index].start_hour[index1] = 0;
                }

                startTime = scheduleTimeing[index].start_hour[index1] < 10 ?
                            (QString("0") + QString("%1").arg (scheduleTimeing[index].start_hour[index1])) : (QString("%1").arg (scheduleTimeing[index].start_hour[index1]));

                if(scheduleTimeing[index].start_min[index1] > 60)
                {
                    scheduleTimeing[index].start_min[index1] = 0;
                }

                scheduleTimeing[index].start_min[index1] < 10 ? startTime.append(QString("0") + QString("%1").arg(scheduleTimeing[index].start_min[index1])) :
                                                                startTime.append(QString("%1").arg(scheduleTimeing[index].start_min[index1]));

                payloadLib->setCnfgArrayAtIndex((SENS_EVT_SCHD_START_TIME1 + (MAX_SENSOR_EVNT_ACT_FLDS_SCH_MNG * (index)) + 3*(index1)),startTime);

                if(scheduleTimeing[index].stop_hour[index1] > 24)
                {
                    scheduleTimeing[index].stop_hour[index1] = 0;
                }

                stopTime = scheduleTimeing[index].stop_hour[index1] < 10 ? (QString("0") + QString("%1").arg (scheduleTimeing[index].stop_hour[index1])): (QString("%1").arg (scheduleTimeing[index].stop_hour[index1]));

                if(scheduleTimeing[index].stop_min[index1] > 60)
                {
                    scheduleTimeing[index].stop_min[index1] = 0;
                }

                scheduleTimeing[index].stop_min[index1] < 10 ? stopTime.append (QString("0") + QString("%1").arg(scheduleTimeing[index].stop_min[index1])) :
                                                               stopTime.append (QString("%1").arg(scheduleTimeing[index].stop_min[index1]));

                payloadLib->setCnfgArrayAtIndex((SENS_EVT_SCHD_STOP_TIME1 + (MAX_SENSOR_EVNT_ACT_FLDS_SCH_MNG * (index)) + 3*(index1)),stopTime);
            }
        }
    }

    for(quint8 dayindex = 0; dayindex < MAX_SENS_EVNT_WEEKDAYS; dayindex++)
    {
        payloadLib->setCnfgArrayAtIndex(SENS_EVT_SCHD_ACTION_STATUS_ENTIRE_DAY + dayindex*MAX_SENSOR_EVNT_ACT_FLDS_SCH_MNG, actionStatus[dayindex*MAX_SENS_EVNT_WEEKDAYS]);

        for(quint8 index = 1; index < MAX_SENS_EVNT_WEEKDAYS; index++)
        {
            payloadLib->setCnfgArrayAtIndex(SENS_EVT_SCHD_ACTION_STATUS_TIME1 + MAX_SENSOR_EVNT_ACT_FLDS_SCH_MNG *dayindex + (index-1)*3, actionStatus[index + (dayindex * MAX_SENS_EVNT_WEEKDAYS)]);
        }
    }
}

void SensorEventAndAction::saveConfig ()
{
    setConfigFields();
    fillRecords();
    createPayload(MSG_SET_CFG);
}

void SensorEventAndAction::processDeviceResponse (DevCommParam *param, QString deviceName)
{
    bool isUnloadProcessBar = true;

    if (deviceName != currDevName)
    {
        processBar->unloadProcessBar();
        return;
    }

    if (param->deviceStatus != CMD_SUCCESS)
    {
        processBar->unloadProcessBar();
        infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
        sensorListDropDownBox->setIndexofCurrElement (currentSensorIndex);
        return;
    }

    switch(param->msgType)
    {
        case MSG_GET_CFG:
        {
            m_configResponse.clear();
            payloadLib->parsePayload (param->msgType, param->payload);

            if(payloadLib->getcnfgTableIndex () == SYSTEM_SENSOR_INPUT_TABLE_INDEX)
            {
                QString tempStr;
                sensorList.clear();
                for(quint8 index = 0; index < devTableInfo->sensors; index++)
                {
                    tempStr = payloadLib->getCnfgArrayAtIndex (index).toString ();
                    if(tempStr == " ")
                    {
                        break;
                    }

                    sensorList.insert(index, QString("%1%2%3").arg(index + 1).arg(" : ").arg (tempStr));
                }

                sensorListDropDownBox->setNewList (sensorList,(currentSensorIndex-1));
                processBar->unloadProcessBar ();
                getConfig ();
            }

            if(payloadLib->getcnfgTableIndex () == PRESET_POSITION_TABLE_INDEX)
            {
                presetList.clear ();
                presetList.insert (0,"None");
                for(quint8 index = 0; index < MAX_PRESET_POS; index++)
                {
                    QString tempStr  = (payloadLib->getCnfgArrayAtIndex(index)).toString ();
                    if(tempStr != "")
                    {
                        presetList.insert((index + 1), QString("%1%2%3").arg(index + 1).arg(" : ").arg (tempStr));
                    }
                }

                if(eventActionPtzPos != NULL)
                {
                    eventActionPtzPos->processDeviceResponse (presetList);
                }
            }

            if(payloadLib->getcnfgTableIndex (0) == SENSOR_EVENT_ACTION_MANAGMENT_TABLE_INDEX)
            {
                actionEventCheckbox->changeState ((payloadLib->getCnfgArrayAtIndex(SENS_EVE_ENABLE_ACTION).toUInt ()) == 0 ? OFF_STATE : ON_STATE);
                m_configResponse[SENS_EVE_ENABLE_ACTION] = payloadLib->getCnfgArrayAtIndex(SENS_EVE_ENABLE_ACTION);

                enableControlsOnAction (actionEventCheckbox->getCurrentState () == ON_STATE ? true :false );

                for (quint8 maskIdx = 0; maskIdx < CAMERA_MASK_MAX; maskIdx++)
                {
                    alarmRecordingSelectedCamera.bitMask[maskIdx] = (payloadLib->getCnfgArrayAtIndex(SENS_EVE_CAM_NUM_ALARM_REC_START+maskIdx).toULongLong());
                    m_configResponse[SENS_EVE_CAM_NUM_ALARM_REC_START+maskIdx] = alarmRecordingSelectedCamera.bitMask[maskIdx];

                    uploadImageSelectedCamera.bitMask[maskIdx] = (payloadLib->getCnfgArrayAtIndex(SENS_EVE_CAM_NUM_UPLOAD_REC_START+maskIdx).toULongLong());
                    m_configResponse[SENS_EVE_CAM_NUM_UPLOAD_REC_START+maskIdx] = uploadImageSelectedCamera.bitMask[maskIdx];
                }

                emailAddress = (payloadLib->getCnfgArrayAtIndex (SENS_EVE_EMAIL_ADDRESS).toString ());
                m_configResponse[SENS_EVE_EMAIL_ADDRESS] = emailAddress;

                emailSubject = (payloadLib->getCnfgArrayAtIndex (SENS_EVE_EMAIL_SUBJECT).toString ());
                m_configResponse[SENS_EVE_EMAIL_SUBJECT] = emailSubject;

                emailMessage = (payloadLib->getCnfgArrayAtIndex (SENS_EVE_EMAIL_MESSAGE).toString ());
                m_configResponse[SENS_EVE_EMAIL_MESSAGE] = emailMessage;

                tcpMessage = (payloadLib->getCnfgArrayAtIndex (SENS_EVE_TCP_MESSAGE).toString ());
                m_configResponse[SENS_EVE_TCP_MESSAGE] = tcpMessage;

                smsMobileNum1 = (payloadLib->getCnfgArrayAtIndex (SENS_EVE_SMS_MOB1).toString ());
                m_configResponse[SENS_EVE_SMS_MOB1] = smsMobileNum1;

                smsMobileNum2 = (payloadLib->getCnfgArrayAtIndex (SENS_EVE_SMS_MOB2).toString ());
                m_configResponse[SENS_EVE_SMS_MOB2] = smsMobileNum2;

                smsMessage = (payloadLib->getCnfgArrayAtIndex (SENS_EVE_SMS_MESSAGE).toString ());
                m_configResponse[SENS_EVE_SMS_MESSAGE] = smsMessage;

                presetCameraNum = (payloadLib->getCnfgArrayAtIndex (SENS_EVE_PTZ_CAM_NUM).toUInt ());
                m_configResponse[SENS_EVE_PTZ_CAM_NUM] = presetCameraNum;

                presetGotoPosition = (payloadLib->getCnfgArrayAtIndex (SENS_EVE_PTZ_PRE_POS).toUInt ());
                m_configResponse[SENS_EVE_PTZ_PRE_POS] = presetGotoPosition;

                currentCamForAlarm = (payloadLib->getCnfgArrayAtIndex (SENS_EVE_CAM_ALM_CAM_NUM).toUInt ());
                m_configResponse[SENS_EVE_CAM_ALM_CAM_NUM] = currentCamForAlarm;

                currentCamForAlarm = (currentCamForAlarm == 0) ? 1 : currentCamForAlarm;

                for(quint8 index = 0; index < MAX_DEV_ALARM; index++)
                {
                    deviceAlaram[index] = (payloadLib->getCnfgArrayAtIndex (SENS_EVE_SYS_ALM_OP_PORT1 + index)).toUInt () == 0 ? false : true;
                    m_configResponse[SENS_EVE_SYS_ALM_OP_PORT1+ index] = deviceAlaram[index];
                }

                for(quint8 index = 0; index < MAX_CAM_ALARM; index++)
                {
                    cameraAlarm[index] = (payloadLib->getCnfgArrayAtIndex (SENS_EVE_CAM_ALM_ENB1 + index)).toUInt () == 0 ? false : true;
                    m_configResponse[SENS_EVE_CAM_ALM_ENB1 + index] = cameraAlarm[index];
                }
            }

            if(payloadLib->getcnfgTableIndex (1) == SENSOR_EVENT_ACTION_SCHDULE_TABLE_INDEX)
            {
                for(quint8 index = 0; index< MAX_SENS_EVNT_WEEKDAYS;index++)
                {
                    if( payloadLib->getCnfgArrayAtIndex(SENS_EVT_SCHD_ENTIRE_DAY + (index * MAX_SENSOR_EVNT_ACT_FLDS_SCH_MNG)).toUInt () == 1)
                    {
                        isScheduleSetForEntireDay[index] = true;
                        m_configResponse[SENS_EVT_SCHD_ENTIRE_DAY + (index * MAX_SENSOR_EVNT_ACT_FLDS_SCH_MNG)] = isScheduleSetForEntireDay[index];
                    }
                    else
                    {
                        isScheduleSetForEntireDay[index] = false;
                        m_configResponse[SENS_EVT_SCHD_ENTIRE_DAY + (index * MAX_SENSOR_EVNT_ACT_FLDS_SCH_MNG)] = isScheduleSetForEntireDay[index];

                        for(quint8 index1 = 0; index1 < MAX_SENS_SCHD_TIME_SLOTS;index1++)
                        {
                            QString time = (payloadLib->getCnfgArrayAtIndex(SENS_EVT_SCHD_START_TIME1 +  (MAX_SENSOR_EVNT_ACT_FLDS_SCH_MNG * index) +  3*index1   )).toString ().mid (0,2);
                            m_configResponse[SENS_EVT_SCHD_START_TIME1+(MAX_SENSOR_EVNT_ACT_FLDS_SCH_MNG * index) + 3*index1] =
                                    payloadLib-> getCnfgArrayAtIndex( SENS_EVT_SCHD_START_TIME1 + 3*index1 + (MAX_SENSOR_EVNT_ACT_FLDS_SCH_MNG * (index)));
                            scheduleTimeing[index].start_hour[index1] = time.toUInt ();

                            time = (payloadLib->getCnfgArrayAtIndex(SENS_EVT_SCHD_START_TIME1 + 3*index1 + (MAX_SENSOR_EVNT_ACT_FLDS_SCH_MNG * (index)))).toString ().mid (2,2);
                            scheduleTimeing[index].start_min[index1] = time.toUInt ();

                            time = (payloadLib->getCnfgArrayAtIndex( SENS_EVT_SCHD_STOP_TIME1 + 3*index1 + (MAX_SENSOR_EVNT_ACT_FLDS_SCH_MNG * (index)))).toString ().mid (0,2);
                            m_configResponse[SENS_EVT_SCHD_STOP_TIME1+(MAX_SENSOR_EVNT_ACT_FLDS_SCH_MNG * index) + 3*index1] =
                                    payloadLib->getCnfgArrayAtIndex( SENS_EVT_SCHD_STOP_TIME1 + 3*index1 + (MAX_SENSOR_EVNT_ACT_FLDS_SCH_MNG * (index)));
                            scheduleTimeing[index].stop_hour[index1] = time.toUInt ();

                            time = (payloadLib->getCnfgArrayAtIndex( SENS_EVT_SCHD_STOP_TIME1 + 3*index1 + (MAX_SENSOR_EVNT_ACT_FLDS_SCH_MNG * (index)))).toString ().mid (2,2);
                            scheduleTimeing[index].stop_min[index1] = time.toUInt ();
                        }
                    }
                }

                for(quint8 dayindex = 0; dayindex < MAX_SENS_EVNT_WEEKDAYS;dayindex++)
                {
                    actionStatus[dayindex*MAX_SENS_EVNT_WEEKDAYS] = (payloadLib->getCnfgArrayAtIndex (SENS_EVT_SCHD_ACTION_STATUS_ENTIRE_DAY + dayindex*MAX_SENSOR_EVNT_ACT_FLDS_SCH_MNG)).toUInt();
                    m_configResponse[SENS_EVT_SCHD_ACTION_STATUS_ENTIRE_DAY + dayindex * MAX_SENSOR_EVNT_ACT_FLDS_SCH_MNG] = actionStatus[dayindex*MAX_SENS_EVNT_WEEKDAYS];

                    for(quint8 index = 1; index < MAX_SENS_EVNT_WEEKDAYS; index++)
                    {
                        actionStatus[index + (dayindex * MAX_SENS_EVNT_WEEKDAYS)] = (payloadLib->getCnfgArrayAtIndex (SENS_EVT_SCHD_ACTION_STATUS_TIME1 + (MAX_SENSOR_EVNT_ACT_FLDS_SCH_MNG * (dayindex)) + (index-1)*3)).toUInt ();
                        m_configResponse[SENS_EVT_SCHD_ACTION_STATUS_TIME1 + ((MAX_SENSOR_EVNT_ACT_FLDS_SCH_MNG * (dayindex)) + (index-1)*3)] = actionStatus[index + (dayindex * MAX_SENS_EVNT_WEEKDAYS)];
                    }
                }

                updateScheduleBar();
            }
        }
        break;

        case MSG_SET_CFG:
        {
            isUnloadProcessBar = false;
            MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
            currentSensorIndex = sensorListDropDownBox->getIndexofCurrElement ();
            getConfig();
        }
        break;

        case MSG_DEF_CFG:
        {
            isUnloadProcessBar = false;
            currentSensorIndex = sensorListDropDownBox->getIndexofCurrElement ();
            getConfig();
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }

    if(isUnloadProcessBar)
    {
        processBar->unloadProcessBar();
    }
}

void SensorEventAndAction::enableControlsOnAction(bool status)
{
    for(quint8 index = 0; index < MAX_SENS_EVNT_SCHEDULE_BAR; index++)
    {
        if (index < (MAX_SENS_EVNT_EVENTS - 1))
        {
            setButtons[index]->setIsEnabled (status);
        }

        if(index > 0)
        {
            scheduleBar[index]->setIsEnable (status);
        }
    }
    #if defined(OEM_JCI)
    scheduleBar[EVNT_SMS]->setIsEnable(false);
    setButtons[EVNT_SMS]->setIsEnabled(false);
    scheduleBar[EVNT_TCP]->setIsEnable(false);
    setButtons[EVNT_TCP]->setIsEnabled(false);
    #endif
}

void SensorEventAndAction::deleteSubObejects ()
{
    switch(m_currentElement)
    {
        case SENS_EVT_WEEKDAYS_SET_CTRL:
        {
            if (IS_VALID_OBJ(eventActionSchedule))
            {
                disconnect (eventActionSchedule,
                            SIGNAL(sigDeleteObject(quint8)),
                            this,
                            SLOT(slotSubObjectDelete(quint8)));
                DELETE_OBJ(eventActionSchedule);
            }
        }
        break;

        case SENS_EVT_EVENT_ALARM_SET_CTRL:
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

        case SENS_EVT_EVENT_IMG_UPLD_SET_CTRL:
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

        case SENS_EVT_EVENT_EMAIL_SET_CTRL:
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

        case SENS_EVT_EVENT_TCP_SET_CTRL:
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

        case SENS_EVT_EVENT_SMS_SET_CTRL:
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

        case SENS_EVT_EVENT_PRESET_SET_CTRL:
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

        case SENS_EVT_EVENT_DEVICE_ALARM_SET_CTRL:
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

        case SENS_EVT_EVENT_CAMERA_ALARM_SET_CTRL:
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

        case SENS_EVT_EVENT_BUZZER_SET_CTRL:
        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void SensorEventAndAction::slotCheckBoxValueChanged (OPTION_STATE_TYPE_e status, int)
{
    enableControlsOnAction(status == ON_STATE ? true : false);
}

void SensorEventAndAction::slotSpinBoxValueChanged (QString, quint32 index)
{
    switch(index)
    {
        case SENS_EVT_EVENTLIST_SPINBOX_CTRL:
        {
            if(isUserChangeConfig())
            {
                infoPage->loadInfoPage(ValidationMessage::getValidationMessage(SAVE_CHANGES),true,false,"",CONFORMATION_BTN_YES,CONFORMATION_BTN_NO);
            }
            else
            {
                currentSensorIndex = sensorListDropDownBox->getIndexofCurrElement ();
                getConfig ();
            }
        }
        break;

        case SENS_EVT_WEEKDAYS_SPINBOX_CTRL:
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

void SensorEventAndAction::slotButtonClick (int index)
{
    switch(index)
    {
        case SENS_EVT_WEEKDAYS_SET_CTRL:
        {
            if (IS_VALID_OBJ(eventActionSchedule))
            {
                break;
            }

            eventActionSchedule = new EventActionSchedule(SENS_EVT_WEEKDAYS_SET_CTRL,
                                                          m_currentDayIndex,
                                                          scheduleTimeing,
                                                          isScheduleSetForEntireDay,
                                                          actionStatus,
                                                          parentWidget(),
                                                          devTableInfo);
            connect(eventActionSchedule,
                    SIGNAL(sigDeleteObject(quint8)),
                    this,
                    SLOT(slotSubObjectDelete(quint8)));
        }
        break;

        case SENS_EVT_EVENT_ALARM_SET_CTRL:
        {
            if (IS_VALID_OBJ(cameraSelectForAlarm))
            {
                break;
            }

            cameraSelectForAlarm = new CopyToCamera(cameraList,
                                                    alarmRecordingSelectedCamera,
                                                    parentWidget (),
                                                    sensorEventAndEventActionStrings[SENS_EVT_EVENT_ACTION_LABEL],
                                                    SENS_EVT_EVENT_ALARM_SET_CTRL);
            connect(cameraSelectForAlarm,
                    SIGNAL(sigDeleteObject(quint8)),
                    this,
                    SLOT(slotSubObjectDelete(quint8)));
        }
        break;

        case SENS_EVT_EVENT_IMG_UPLD_SET_CTRL:
        {
            if (IS_VALID_OBJ(cameraSelectForImageUpload))
            {
                break;
            }

            cameraSelectForImageUpload = new CopyToCamera(cameraList,
                                                          uploadImageSelectedCamera,
                                                          parentWidget (),
                                                          sensorEventAndEventActionStrings[SENS_EVT_EVENT_ACTION_LABEL + 1],
                                                          SENS_EVT_EVENT_IMG_UPLD_SET_CTRL);
            connect(cameraSelectForImageUpload,
                    SIGNAL(sigDeleteObject(quint8)),
                    this,
                    SLOT(slotSubObjectDelete(quint8)));
        }
        break;

        case SENS_EVT_EVENT_EMAIL_SET_CTRL:
        {
            if (IS_VALID_OBJ(eventActionEmailNotify))
            {
                break;
            }

            eventActionEmailNotify = new EventActionEmailNotify(SENS_EVT_EVENT_EMAIL_SET_CTRL,
                                                                emailAddress,
                                                                emailSubject,
                                                                emailMessage,
                                                                parentWidget ());
            connect(eventActionEmailNotify,
                    SIGNAL(sigDeleteObject(quint8)),
                    this,
                    SLOT(slotSubObjectDelete(quint8)));
        }
        break;

        case SENS_EVT_EVENT_TCP_SET_CTRL:
        {
            if (IS_VALID_OBJ(eventActionTCPNotify))
            {
                break;
            }

            eventActionTCPNotify = new EventActionTCPNotify(SENS_EVT_EVENT_TCP_SET_CTRL,
                                                            tcpMessage,
                                                            parentWidget ());
            connect(eventActionTCPNotify,
                    SIGNAL(sigDeleteObject(quint8)),
                    this,
                    SLOT(slotSubObjectDelete(quint8)));
        }
        break;

        case SENS_EVT_EVENT_SMS_SET_CTRL:
        {
            if (IS_VALID_OBJ(eventActionSmsNotify))
            {
                break;
            }

            eventActionSmsNotify = new EventActionSmsNotify(SENS_EVT_EVENT_SMS_SET_CTRL,
                                                            smsMobileNum1,
                                                            smsMobileNum2,
                                                            smsMessage,
                                                            parentWidget ());
            connect(eventActionSmsNotify,
                    SIGNAL(sigDeleteObject(quint8)),
                    this,
                    SLOT(slotSubObjectDelete(quint8)));
        }
        break;

        case SENS_EVT_EVENT_PRESET_SET_CTRL:
        {
            if (IS_VALID_OBJ(eventActionPtzPos))
            {
                break;
            }

            eventActionPtzPos = new EventActionPtzPos(SENS_EVT_EVENT_PRESET_SET_CTRL,
                                                      currDevName,
                                                      presetCameraNum,
                                                      presetGotoPosition,
                                                      devTableInfo->totalCams,
                                                      parentWidget ());
            connect(eventActionPtzPos,
                    SIGNAL(sigDeleteObject(quint8)),
                    this,
                    SLOT(slotSubObjectDelete(quint8)));
        }
        break;

        case SENS_EVT_EVENT_DEVICE_ALARM_SET_CTRL:
        {
            if (IS_VALID_OBJ(eventActionDeviceAlarm))
            {
                break;
            }

            eventActionDeviceAlarm = new EventActionDeviceAlarm(SENS_EVT_EVENT_DEVICE_ALARM_SET_CTRL,
                                                                deviceAlaram,
                                                                devTableInfo,
                                                                parentWidget ());
            connect(eventActionDeviceAlarm,
                    SIGNAL(sigDeleteObject(quint8)),
                    this,
                    SLOT(slotSubObjectDelete(quint8)));
        }
        break;

        case SENS_EVT_EVENT_CAMERA_ALARM_SET_CTRL:
        {
            if (IS_VALID_OBJ(eventCameraAlarmOutput))
            {
                break;
            }

            eventCameraAlarmOutput = new EventCameraAlarmOutput(SENS_EVT_EVENT_CAMERA_ALARM_SET_CTRL,
                                                                cameraList,
                                                                cameraAlarm,
                                                                currentCamForAlarm,
                                                                parentWidget ());
            connect(eventCameraAlarmOutput,
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
    m_currentElement =  index;
}

void SensorEventAndAction::slotSubObjectDelete (quint8 index)
{
    m_currentElement =  index;

    if(index == SENS_EVT_WEEKDAYS_SET_CTRL)
    {
        updateScheduleBar ();
    }

    deleteSubObejects();
    m_elementList[m_currentElement]->forceActiveFocus ();
}
