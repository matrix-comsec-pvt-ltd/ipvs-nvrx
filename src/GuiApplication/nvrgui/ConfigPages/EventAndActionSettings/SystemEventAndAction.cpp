#include "SystemEventAndAction.h"
#include "ValidationMessage.h"

#define FOOTNOTE_STRING    "Note: Tick mark before each action will notify the actions to be performed"
#define DEV_ALARM_INDX      5

typedef enum
{
    SYS_EVE_ENABLE_ACTION,
    SYS_EVE_CAM_NUM_ALARM_REC_START,
    SYS_EVE_CAM_NUM_ALARM_REC_END = SYS_EVE_CAM_NUM_ALARM_REC_START + CAMERA_MASK_MAX - 1,
    SYS_EVE_CAM_NUM_UPLOAD_REC_START,
    SYS_EVE_CAM_NUM_UPLOAD_REC_END = SYS_EVE_CAM_NUM_UPLOAD_REC_START + CAMERA_MASK_MAX - 1,
    SYS_EVE_EMAIL_ADDRESS,
    SYS_EVE_EMAIL_SUBJECT,
    SYS_EVE_EMAIL_MESSAGE,
    SYS_EVE_TCP_MESSAGE,
    SYS_EVE_SMS_MOB1,
    SYS_EVE_SMS_MOB2,
    SYS_EVE_SMS_MESSAGE,
    SYS_EVE_PTZ_CAM_NUM,
    SYS_EVE_PTZ_PRE_POS,
    SYS_EVE_SYS_ALM_OP_PORT1,
    SYS_EVE_SYS_ALM_OP_PORT2,
    SYS_EVE_SYS_ALM_OP_PORT3,
    SYS_EVE_SYS_ALM_OP_PORT4,
    SYS_EVE_CAM_ALM_CAM_NUM,
    SYS_EVE_CAM_ALM_ENB1,
    SYS_EVE_CAM_ALM_ENB2,
    SYS_EVE_CAM_ALM_ENB3,
    SYS_EVE_ACTION_STATUS,

    MAX_SYSTEM_EVENT_AND_ACTION_FEILDS
}SYSTEM_EVENT_AND_ACTION_FEILDS;

typedef enum
{
    SYS_EVNT_SPINBOX_CTRL,
    SYS_EVNT_ACTION_CTRL,

    SYS_EVNT_IMG_UPLD,
    SYS_EVNT_EMAIL,
    SYS_EVNT_TCP,
    SYS_EVNT_SMS,
    SYS_EVNT_PRE_PTZ,
    SYS_EVNT_DVC_ALM,
    SYS_EVNT_DVC_CAM_ALM,
    SYS_EVNT_BUZZER,
    SYS_EVNT_PUSH_NOTIFICATION,

    SYS_EVNT_IMG_UPLD_SET,
    SYS_EVNT_EMAIL_SET,
    SYS_EVNT_TCP_SET,
    SYS_EVNT_SMS_SET,
    SYS_EVNT_PRE_PTZ_SET,
    SYS_EVNT_DVC_ALM_SET,
    SYS_EVNT_CAM_ALM_SET,

    MAX_SYS_EVNT_CTRL
}SYS_EVNT_CTRL_e;

typedef enum
{
    SYS_EVNT_SPINBOX_LABEL,
    SYS_EVNT_ACTION_LABEL,
    SYS_EVNT_TYPE_LABEL = SYS_EVNT_ACTION_LABEL + SYS_EVENT_TYPE_MAX,
    MAX_SYS_EVNT_STRINGS
}SYS_EVNT_STRINGS_e;

static const QString systemEventAndActionStrings[MAX_SYS_EVNT_STRINGS] =
{
    "Event Type",
    "Action",
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

static const QStringList systemActionList = QStringList()
        << "Manual Trigger"
        << "On boot"
        << "Storage Alert"
        << "Disk volume full"
        << "Disk Fault"
        << "Scheduled Backup Fail"
        << "Firmware Upgrade";

SystemEventAndAction::SystemEventAndAction(QString deviceName, QWidget* parent, DEV_TABLE_INFO_t *devTabInfo)
    : ConfigPageControl(deviceName, parent,MAX_SYS_EVNT_CTRL,devTabInfo),
      currentCamforAlarm(1), suppAlarm(devTabInfo->alarms), suppSensors(devTabInfo->sensors)
{
    cameraSelectForAlarm = NULL;
    cameraSelectForImageUpload = NULL;
    eventActionEmailNotify = NULL;
    eventActionTCPNotify = NULL;
    eventActionSmsNotify = NULL;
    eventActionPtzPos = NULL;
    eventActionDeviceAlarm = NULL;
    eventCameraAlarmOutput = NULL;
    actionStatus = 0;
    currentclickedIndex = 0;
    presetCameraNum = 0;
    presetGotoPosition = 0;
    memset(&alarmRecordingSelectedCamera, 0, sizeof(alarmRecordingSelectedCamera));
    memset(&uploadImageSelectedCamera, 0, sizeof(uploadImageSelectedCamera));

    for(quint8 index = 0; index < MAX_CAM_ALARM; index++)
    {
        cameraAlarm[index] = false;
        deviceAlaram[index] = false;
    }

    createDefaultComponents();
    currentEventIndex = 0;
    SystemEventAndAction::getConfig();
}

void SystemEventAndAction::createDefaultComponents()
{
    QMap<quint8, QString>  systemActionMapList;

    for(quint8 index = 0; index < systemActionList.length (); index++)
    {
        systemActionMapList.insert (index,systemActionList.at (index));
    }

    eventTypeDropDownBox = new DropDown(SCALE_WIDTH(INNER_RIGHT_MARGIN)/2 +
                                        (SCALE_WIDTH(PAGE_RIGHT_PANEL_WIDTH) - BGTILE_MEDIUM_SIZE_WIDTH)/2,
                                        SCALE_HEIGHT(60),
                                        BGTILE_MEDIUM_SIZE_WIDTH,
                                        BGTILE_HEIGHT,
                                        SYS_EVNT_SPINBOX_CTRL,
                                        DROPDOWNBOX_SIZE_320,
                                        systemEventAndActionStrings[SYS_EVNT_SPINBOX_LABEL],
                                        systemActionMapList,
                                        this,
                                        "",
                                        false,
                                        SCALE_WIDTH(30));
    m_elementList[SYS_EVNT_SPINBOX_CTRL] = eventTypeDropDownBox;
    connect (eventTypeDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (eventTypeDropDownBox,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotSpinboxValueChanged(QString,quint32)));

    actionSelection = new OptionSelectButton(eventTypeDropDownBox->x (),
                                             eventTypeDropDownBox->y () + SCALE_HEIGHT(5) + BGTILE_HEIGHT,
                                             BGTILE_MEDIUM_SIZE_WIDTH,
                                             BGTILE_HEIGHT,
                                             CHECK_BUTTON_INDEX,
                                             this,
                                             UP_LAYER,
                                             "",
                                             systemEventAndActionStrings[SYS_EVNT_ACTION_LABEL],
                                             SCALE_WIDTH(30),
                                             SYS_EVNT_ACTION_CTRL,
                                             true,
                                             NORMAL_FONT_SIZE,
                                             NORMAL_FONT_COLOR);
    m_elementList[SYS_EVNT_ACTION_CTRL] = actionSelection;
    connect (actionSelection,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (actionSelection,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));

    for(quint8 index = 0; index < SYS_EVENT_TYPE_MAX; index++)
    {
        events[index] = new OptionSelectButton(eventTypeDropDownBox->x (),
                                               actionSelection->y () + (index + 1 )*BGTILE_HEIGHT,
                                               BGTILE_MEDIUM_SIZE_WIDTH,
                                               BGTILE_HEIGHT,
                                               CHECK_BUTTON_INDEX,
                                               this,
                                               index < 6 ? MIDDLE_TABLE_LAYER : BOTTOM_TABLE_LAYER,
                                               "",
                                               systemEventAndActionStrings[SYS_EVNT_ACTION_LABEL + index + 1],
                                               SCALE_WIDTH(20),
                                               (SYS_EVNT_IMG_UPLD + index*2),
                                               true,
                                               NORMAL_FONT_SIZE,
                                               ((index == DEV_ALARM_INDX) && (suppAlarm == 0) && (suppSensors == 0)) ? DISABLE_FONT_COLOR : NORMAL_FONT_COLOR);
        m_elementList[(SYS_EVNT_IMG_UPLD + index*2)] = events[index];
        connect (events[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
        connect (events[index],
                 SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                 this,
                 SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));

        if(index < SYS_EVENT_TYPE_MAX - SYS_EVENT_WO_SET_BTN)
        {
            eventsSetBtn[index] = new PageOpenButton(eventTypeDropDownBox->x () + BGTILE_MEDIUM_SIZE_WIDTH - SCALE_WIDTH(100),
                                                     events[index]->y (),
                                                     BGTILE_MEDIUM_SIZE_WIDTH,
                                                     BGTILE_HEIGHT,
                                                     (SYS_EVNT_IMG_UPLD + index*2 + 1),
                                                     PAGEOPENBUTTON_SMALL,
                                                     "Set",
                                                     this,
                                                     "",
                                                     "",
                                                     false,
                                                     0,
                                                     NO_LAYER,
                                                     true);
            m_elementList[ (SYS_EVNT_IMG_UPLD + index*2 + 1)] = eventsSetBtn[index];
            connect (eventsSetBtn[index],
                     SIGNAL(sigUpdateCurrentElement(int)),
                     this,
                     SLOT(slotUpdateCurrentElement(int)));
            connect (eventsSetBtn[index],
                     SIGNAL(sigButtonClick(int)),
                     this,
                     SLOT(slotButtonClick(int)));
        }
    }

    footNoteLabel = new TextLabel(INNER_RIGHT_MARGIN/2 + this->width ()/2,
                                  events[SYS_EVENT_TYPE_MAX - 1]->y () + events[SYS_EVENT_TYPE_MAX - 1]->height () + SCALE_HEIGHT(20),
                                  NORMAL_FONT_SIZE,
                                  FOOTNOTE_STRING,
                                  this,
                                  NORMAL_FONT_COLOR,
                                  NORMAL_FONT_FAMILY,
                                  ALIGN_CENTRE_X_START_Y);

    #if defined(OEM_JCI)
    events[SYS_EVENT_TYPE_SMS - 1]->setVisible(false);
    eventsSetBtn[SYS_EVENT_TYPE_SMS - 1]->setVisible(false);
    events[SYS_EVENT_TYPE_TCP - 3]->setVisible(false);
    eventsSetBtn[SYS_EVENT_TYPE_TCP - 3]->setVisible(false);

    for(quint8 index = SYS_EVENT_TYPE_SMS; index < SYS_EVENT_TYPE_MAX; index++)
    {
        events[index]->resetGeometry(events[index]->x (),
                                     events[index]->y () - (2 * BGTILE_HEIGHT),
                                     BGTILE_MEDIUM_SIZE_WIDTH,
                                     BGTILE_HEIGHT);
        if(index < SYS_EVENT_TYPE_MAX - SYS_EVENT_WO_SET_BTN)
        {
            eventsSetBtn[index]->resetGeometry(eventsSetBtn[index]->x (), eventsSetBtn[index]->y () - (2 * BGTILE_HEIGHT));
        }
    }

    footNoteLabel->setOffset(INNER_RIGHT_MARGIN/2 + this->width ()/2, footNoteLabel->y() - (2 * BGTILE_HEIGHT));
    #endif

}

SystemEventAndAction::~SystemEventAndAction()
{
    disconnect (eventTypeDropDownBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete eventTypeDropDownBox;

    disconnect (actionSelection,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete actionSelection;

    for(quint8 index = 0; index < SYS_EVENT_TYPE_MAX; index++)
    {
        disconnect (events[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        disconnect (events[index],
                    SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                    this,
                    SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));
        delete events[index];

        if(index < SYS_EVENT_TYPE_MAX - SYS_EVENT_WO_SET_BTN)
        {
            disconnect (eventsSetBtn[index],
                        SIGNAL(sigUpdateCurrentElement(int)),
                        this,
                        SLOT(slotUpdateCurrentElement(int)));
            disconnect (eventsSetBtn[index],
                        SIGNAL(sigButtonClick(int)),
                        this,
                        SLOT(slotButtonClick(int)));
            delete eventsSetBtn[index];
        }
    }

    deleteSubObejects();
    DELETE_OBJ(footNoteLabel);
}

void SystemEventAndAction::fillCameraList ()
{
    cameraList.clear();
    for(quint8 index = 0; index < devTableInfo->totalCams; index++)
    {
        cameraList.insert(index, QString("%1%2%3").arg(index + 1).arg(" : ").arg (applController->GetCameraNameOfDevice(currDevName, index)));
    }
}

void SystemEventAndAction::createPayload(REQ_MSG_ID_e msgType)
{
    QString payloadString = payloadLib->createDevCnfgPayload(msgType,
                                                             SYSTEM_EVENT_ACTION_MANAGMENT_TABLE_INDEX,
                                                             currentEventIndex + 1,
                                                             currentEventIndex + 1,
                                                             CNFG_FRM_INDEX,
                                                             MAX_SYSTEM_EVENT_AND_ACTION_FEILDS,
                                                             MAX_SYSTEM_EVENT_AND_ACTION_FEILDS);
    DevCommParam* param = new DevCommParam();
    param->msgType = msgType;
    param->payload = payloadString;

    processBar->loadProcessBar ();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void SystemEventAndAction::defaultConfig ()
{
    createPayload(MSG_DEF_CFG);
}

bool SystemEventAndAction::isUserChangeConfig()
{
    if(m_configResponse.isEmpty())
    {
        return false;
    }

    quint32 tempActionStatus = 0;
    for(quint8 index = 0; index < SYS_EVENT_TYPE_MAX ; index++)
    {
        quint8 actionIndex = (index >= SYS_EVENT_TYPE_PUSH_NOTIFICATION) ? (index + SYS_EVENT_OFFSET) : (SYS_EVENT_TYPE_IMAGE_UPLOAD - index);
        if(events[index]->getCurrentState () == ON_STATE)
        {
            tempActionStatus |= (1 << actionIndex);
        }
        else
        {
            tempActionStatus &= ~(1 << actionIndex);
        }
    }

    if(m_configResponse[SYS_EVE_ACTION_STATUS] != tempActionStatus)
    {
        return true;
    }

    if((IS_VALID_OBJ(actionSelection)) && (m_configResponse[SYS_EVE_ENABLE_ACTION] != actionSelection->getCurrentState()))
    {
        return true;
    }

    for (quint8 maskIdx = 0; maskIdx < CAMERA_MASK_MAX; maskIdx++)
    {
        if(m_configResponse[SYS_EVE_CAM_NUM_ALARM_REC_START+maskIdx] != alarmRecordingSelectedCamera.bitMask[maskIdx])
        {
            return true;
        }

        if(m_configResponse[SYS_EVE_CAM_NUM_UPLOAD_REC_START+maskIdx] != uploadImageSelectedCamera.bitMask[maskIdx])
        {
            return true;
        }
    }

    if(m_configResponse[SYS_EVE_EMAIL_ADDRESS] != emailAddress)
    {
        return true;
    }

    if(m_configResponse[SYS_EVE_EMAIL_SUBJECT] != emailSubject)
    {
        return true;
    }

    if(m_configResponse[SYS_EVE_EMAIL_MESSAGE] != emailMessage)
    {
        return true;
    }

    if(m_configResponse[SYS_EVE_TCP_MESSAGE] != tcpMessage)
    {
        return true;
    }

    if(m_configResponse[SYS_EVE_SMS_MOB1] != smsMobileNum1)
    {
        return true;
    }

    if(m_configResponse[SYS_EVE_SMS_MOB2] != smsMobileNum2)
    {
        return true;
    }

    if(m_configResponse[SYS_EVE_SMS_MESSAGE] != smsMessage)
    {
        return true;
    }

    if(m_configResponse[SYS_EVE_PTZ_CAM_NUM] != presetCameraNum)
    {
        return true;
    }

    if(m_configResponse[SYS_EVE_PTZ_PRE_POS] != presetGotoPosition)
    {
        return true;
    }

    for(quint8 index = 0 ; index < MAX_DEV_ALARM ; index++)
    {
        if(m_configResponse[SYS_EVE_SYS_ALM_OP_PORT1+index] != deviceAlaram[index])
        {
            return true;
        }
    }

    for(quint8 index = 0 ; index < MAX_CAM_ALARM ; index++)
    {
        if(m_configResponse[SYS_EVE_CAM_ALM_ENB1+index] != cameraAlarm[index])
        {
            return true;
        }
    }

    if(m_configResponse[SYS_EVE_CAM_ALM_CAM_NUM] != currentCamforAlarm)
    {
        return true;
    }

    return false;
}

void SystemEventAndAction::handleInfoPageMessage(int index)
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
            updateEventNumber();
            getConfig();
        }
    }
}

void SystemEventAndAction::getConfig ()
{
    fillCameraList();
    createPayload(MSG_GET_CFG);
}

void SystemEventAndAction::setConfigFields()
{
    payloadLib->setCnfgArrayAtIndex (SYS_EVE_ENABLE_ACTION, actionSelection->getCurrentState ());

    for (quint8 maskIdx = 0; maskIdx < CAMERA_MASK_MAX; maskIdx++)
    {
        payloadLib->setCnfgArrayAtIndex (SYS_EVE_CAM_NUM_ALARM_REC_START+maskIdx, alarmRecordingSelectedCamera.bitMask[maskIdx]);
        payloadLib->setCnfgArrayAtIndex (SYS_EVE_CAM_NUM_UPLOAD_REC_START+maskIdx, uploadImageSelectedCamera.bitMask[maskIdx]);
    }

    payloadLib->setCnfgArrayAtIndex (SYS_EVE_EMAIL_ADDRESS,emailAddress);
    payloadLib->setCnfgArrayAtIndex (SYS_EVE_EMAIL_SUBJECT,emailSubject);
    payloadLib->setCnfgArrayAtIndex (SYS_EVE_EMAIL_MESSAGE,emailMessage);

    payloadLib->setCnfgArrayAtIndex (SYS_EVE_TCP_MESSAGE,tcpMessage);

    payloadLib->setCnfgArrayAtIndex (SYS_EVE_SMS_MOB1,smsMobileNum1);
    payloadLib->setCnfgArrayAtIndex (SYS_EVE_SMS_MOB2,smsMobileNum2);
    payloadLib->setCnfgArrayAtIndex (SYS_EVE_SMS_MESSAGE,smsMessage);

    payloadLib->setCnfgArrayAtIndex(SYS_EVE_PTZ_CAM_NUM,presetCameraNum);
    payloadLib->setCnfgArrayAtIndex(SYS_EVE_PTZ_PRE_POS,presetGotoPosition);

    payloadLib->setCnfgArrayAtIndex (SYS_EVE_CAM_ALM_CAM_NUM,currentCamforAlarm);

    for(quint8 index = 0 ; index < MAX_DEV_ALARM ; index++)
    {
        payloadLib->setCnfgArrayAtIndex((SYS_EVE_SYS_ALM_OP_PORT1 + index),deviceAlaram[index] == false ? 0 : 1);
    }

    for(quint8 index = 0 ; index < MAX_CAM_ALARM ; index++)
    {
        payloadLib->setCnfgArrayAtIndex((SYS_EVE_CAM_ALM_ENB1 + index),cameraAlarm[index] == false ? 0 : 1);
    }

    for(quint8 index = 0; index < SYS_EVENT_TYPE_MAX ; index++)
    {
        quint8 actionIndex = (index >= SYS_EVENT_TYPE_PUSH_NOTIFICATION) ? (index + SYS_EVENT_OFFSET) : (SYS_EVENT_TYPE_IMAGE_UPLOAD - index);
        if(events[index]->getCurrentState () == ON_STATE)
        {
            actionStatus |= (1 << actionIndex);
        }
        else
        {
            actionStatus &= ~(1 << actionIndex);
        }
    }

    payloadLib->setCnfgArrayAtIndex(SYS_EVE_ACTION_STATUS, actionStatus);
}

void SystemEventAndAction::saveConfig ()
{
    setConfigFields();
    createPayload(MSG_SET_CFG);
}

void SystemEventAndAction::processDeviceResponse (DevCommParam *param, QString deviceName)
{
    bool isUnloadProcessbar = true;

    if(deviceName != currDevName)
    {
        processBar->unloadProcessBar();
        return;
    }

    if (param->deviceStatus != CMD_SUCCESS)
    {
        infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
        eventTypeDropDownBox->setIndexofCurrElement (currentEventIndex);
        processBar->unloadProcessBar();
        return;
    }

    payloadLib->parsePayload (param->msgType, param->payload);
    switch(param->msgType)
    {
        case MSG_GET_CFG:
        {
            presetList.clear ();
            m_configResponse.clear();

            if(payloadLib->getcnfgTableIndex () == PRESET_POSITION_TABLE_INDEX)
            {
                presetList.insert (0,"None");
                for(quint8 index = 0; index < MAX_PRESET_POS; index++)
                {
                    QString tempStr  = (payloadLib->getCnfgArrayAtIndex(index)).toString () ;
                    if(tempStr != "")
                    {
                        presetList.insert((index +1), QString("%1%2%3").arg(index + 1).arg(" : ").arg (tempStr));
                    }
                }

                if(eventActionPtzPos != NULL)
                {
                    eventActionPtzPos->processDeviceResponse (presetList);
                }
            }

            if(payloadLib->getcnfgTableIndex () == SYSTEM_EVENT_ACTION_MANAGMENT_TABLE_INDEX)
            {
                actionSelection->changeState((payloadLib->getCnfgArrayAtIndex (SYS_EVE_ENABLE_ACTION).toUInt ()) == 0 ? OFF_STATE : ON_STATE);

                m_configResponse[SYS_EVE_ENABLE_ACTION] = payloadLib->getCnfgArrayAtIndex(SYS_EVE_ENABLE_ACTION);

                bool isEnable = (payloadLib->getCnfgArrayAtIndex (SYS_EVE_ENABLE_ACTION).toUInt () == 1) ? true : false;

                slotCheckBoxClicked(isEnable ? ON_STATE : OFF_STATE, SYS_EVNT_ACTION_CTRL);

                for (quint8 maskIdx = 0; maskIdx < CAMERA_MASK_MAX; maskIdx++)
                {
                    alarmRecordingSelectedCamera.bitMask[maskIdx] = (payloadLib->getCnfgArrayAtIndex(SYS_EVE_CAM_NUM_ALARM_REC_START+maskIdx).toULongLong());
                    m_configResponse[SYS_EVE_CAM_NUM_ALARM_REC_START+maskIdx] = alarmRecordingSelectedCamera.bitMask[maskIdx];

                    uploadImageSelectedCamera.bitMask[maskIdx] = (payloadLib->getCnfgArrayAtIndex(SYS_EVE_CAM_NUM_UPLOAD_REC_START+maskIdx).toULongLong());
                    m_configResponse[SYS_EVE_CAM_NUM_UPLOAD_REC_START+maskIdx] = uploadImageSelectedCamera.bitMask[maskIdx];
                }

                emailAddress = (payloadLib->getCnfgArrayAtIndex(SYS_EVE_EMAIL_ADDRESS).toString ());
                m_configResponse[SYS_EVE_EMAIL_ADDRESS] = emailAddress;

                emailSubject = (payloadLib->getCnfgArrayAtIndex(SYS_EVE_EMAIL_SUBJECT).toString ());
                m_configResponse[SYS_EVE_EMAIL_SUBJECT] = emailSubject;

                emailMessage = (payloadLib->getCnfgArrayAtIndex(SYS_EVE_EMAIL_MESSAGE).toString ());
                m_configResponse[SYS_EVE_EMAIL_MESSAGE] = emailMessage;

                tcpMessage = (payloadLib->getCnfgArrayAtIndex(SYS_EVE_TCP_MESSAGE).toString ());
                m_configResponse[SYS_EVE_TCP_MESSAGE] = tcpMessage;

                smsMobileNum1  = (payloadLib->getCnfgArrayAtIndex(SYS_EVE_SMS_MOB1).toString ());
                m_configResponse[SYS_EVE_SMS_MOB1] = smsMobileNum1;

                smsMobileNum2 = (payloadLib->getCnfgArrayAtIndex (SYS_EVE_SMS_MOB2).toString ());
                m_configResponse[SYS_EVE_SMS_MOB2] = smsMobileNum2;

                smsMessage = (payloadLib->getCnfgArrayAtIndex(SYS_EVE_SMS_MESSAGE).toString ());
                m_configResponse[SYS_EVE_SMS_MESSAGE] = smsMessage;

                presetCameraNum = (payloadLib->getCnfgArrayAtIndex(SYS_EVE_PTZ_CAM_NUM).toUInt ());
                m_configResponse[SYS_EVE_PTZ_CAM_NUM] = presetCameraNum;

                presetGotoPosition = (payloadLib->getCnfgArrayAtIndex(SYS_EVE_PTZ_PRE_POS).toUInt ());
                m_configResponse[SYS_EVE_PTZ_PRE_POS] = presetGotoPosition;

                for(quint8 index = 0 ; index < MAX_DEV_ALARM ; index++)
                {
                    deviceAlaram[index] = (payloadLib->getCnfgArrayAtIndex(SYS_EVE_SYS_ALM_OP_PORT1 + index)).toUInt () == 0 ? false : true;
                    m_configResponse[SYS_EVE_SYS_ALM_OP_PORT1+index] = deviceAlaram[index];
                }

                for(quint8 index = 0 ; index < MAX_CAM_ALARM ; index++)
                {
                    cameraAlarm[index] = (payloadLib->getCnfgArrayAtIndex(SYS_EVE_CAM_ALM_ENB1 + index)).toUInt () == 0 ? false : true;
                    m_configResponse[SYS_EVE_CAM_ALM_ENB1+index] = cameraAlarm[index];
                }

                actionStatus = (payloadLib->getCnfgArrayAtIndex(SYS_EVE_ACTION_STATUS).toUInt ());
                m_configResponse[SYS_EVE_ACTION_STATUS] = actionStatus;

                currentCamforAlarm = (payloadLib->getCnfgArrayAtIndex(SYS_EVE_CAM_ALM_CAM_NUM).toUInt ());

                currentCamforAlarm = (currentCamforAlarm == 0) ? 1 : currentCamforAlarm;
                m_configResponse[SYS_EVE_CAM_ALM_CAM_NUM] =  currentCamforAlarm;

                for(quint8 index = 0 ; index < SYS_EVENT_TYPE_MAX; index++ )
                {
                    events[index]->changeState(OFF_STATE);
                    if(index < (SYS_EVENT_TYPE_MAX - SYS_EVENT_WO_SET_BTN))
                    {
                        eventsSetBtn[index]->setIsEnabled (false);
                    }
                }

                for(quint8 index = 0 ; index < SYS_EVENT_TYPE_MAX; index++ )
                {
                    if (index == (SYS_EVENT_TYPE_MAX - SYS_EVENT_WO_SET_BTN))
                    {
                        if(actionStatus & (0x01 << SYS_EVENT_TYPE_BUZZER))
                        {
                            events[index]->changeState (ON_STATE);
                        }
                    }
                    else if (index >= SYS_EVENT_TYPE_PUSH_NOTIFICATION)
                    {
                        if(actionStatus & (0x01 << (index + SYS_EVENT_OFFSET)))
                        {
                            events[index]->changeState (ON_STATE);
                        }
                    }
                    else if(actionStatus & (0x01 << (SYS_EVENT_TYPE_IMAGE_UPLOAD - index)))
                    {
                        events[index]->changeState (ON_STATE);
                        eventsSetBtn[index]->setIsEnabled (true);
                    }
                }
            }
        }
        break;

        case MSG_SET_CFG:
        {
            isUnloadProcessbar = false;
            MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
            updateEventNumber();
            getConfig ();
        }
        break;

        case MSG_DEF_CFG:
        {
            isUnloadProcessbar = false;
            updateEventNumber();
            getConfig ();
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }

    if(isUnloadProcessbar)
    {
        processBar->unloadProcessBar ();
    }

    update ();
}

void SystemEventAndAction::slotSpinboxValueChanged (QString, quint32 )
{
    if(isUserChangeConfig())
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(SAVE_CHANGES),true,false,"",CONFORMATION_BTN_YES,CONFORMATION_BTN_NO);
    }
    else
    {
        updateEventNumber();
        getConfig ();
    }
}

void SystemEventAndAction::updateEventNumber()
{
    currentEventIndex = eventTypeDropDownBox->getIndexofCurrElement ();
    /* Done this because in SystemEventAction array after storage alert there is actuallly system on UPS field is there but
     * currently it is not in use so i have removed it and to maintain field index here i have added 1 for the list index greater than 2 */
    if(currentEventIndex >= 3)
    {
        currentEventIndex += 1;
    }
}

void SystemEventAndAction::deleteSubObejects ()
{
    switch(currentclickedIndex)
    {
        case SYS_EVNT_IMG_UPLD_SET:
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

        case SYS_EVNT_EMAIL_SET:
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

        case SYS_EVNT_TCP_SET:
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

        case SYS_EVNT_SMS_SET:
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

        case SYS_EVNT_PRE_PTZ_SET:
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

        case SYS_EVNT_DVC_ALM_SET:
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

        case SYS_EVNT_CAM_ALM_SET:
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

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void SystemEventAndAction::slotButtonClick(int index)
{
    quint8 tempIndex = ( index/2  - SYS_EVNT_IMG_UPLD + 1) + SYS_EVNT_IMG_UPLD_SET;
    switch(tempIndex)
    {
        case SYS_EVNT_IMG_UPLD_SET:
        {
            if (IS_VALID_OBJ(cameraSelectForImageUpload))
            {
                break;
            }

            cameraSelectForImageUpload = new CopyToCamera(cameraList,
                                                          uploadImageSelectedCamera,
                                                          parentWidget (),
                                                          systemEventAndActionStrings
                                                          [SYS_EVNT_ACTION_LABEL + 1],
                                                          SYS_EVNT_IMG_UPLD_SET);
            connect (cameraSelectForImageUpload,
                     SIGNAL(sigDeleteObject(quint8)),
                     this,
                     SLOT(slotSubObjectDelete(quint8)));
        }
        break;

        case SYS_EVNT_EMAIL_SET:
        {
            if (IS_VALID_OBJ(eventActionEmailNotify))
            {
                break;
            }

            eventActionEmailNotify = new EventActionEmailNotify(SYS_EVNT_EMAIL_SET,
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

        case SYS_EVNT_TCP_SET:
        {
            if (IS_VALID_OBJ(eventActionTCPNotify))
            {
                break;
            }

            eventActionTCPNotify = new EventActionTCPNotify(SYS_EVNT_TCP_SET,
                                                            tcpMessage,
                                                            parentWidget ());
            connect (eventActionTCPNotify,
                     SIGNAL(sigDeleteObject(quint8)),
                     this,
                     SLOT(slotSubObjectDelete(quint8)));
        }
        break;

        case SYS_EVNT_SMS_SET:
        {
            if (IS_VALID_OBJ(eventActionSmsNotify))
            {
                break;
            }

            eventActionSmsNotify = new EventActionSmsNotify(SYS_EVNT_SMS_SET,
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

        case SYS_EVNT_PRE_PTZ_SET:
        {
            if (IS_VALID_OBJ(eventActionPtzPos))
            {
                break;
            }

            eventActionPtzPos = new EventActionPtzPos(SYS_EVNT_PRE_PTZ_SET,
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

        case SYS_EVNT_DVC_ALM_SET:
        {
            if (IS_VALID_OBJ(eventActionDeviceAlarm))
            {
                break;
            }

            eventActionDeviceAlarm = new EventActionDeviceAlarm(SYS_EVNT_DVC_ALM_SET,
                                                                deviceAlaram,
                                                                devTableInfo,
                                                                parentWidget ());
            connect (eventActionDeviceAlarm,
                     SIGNAL(sigDeleteObject(quint8)),
                     this,
                     SLOT(slotSubObjectDelete(quint8)));
        }
        break;

        case SYS_EVNT_CAM_ALM_SET:
        {
            if (IS_VALID_OBJ(eventCameraAlarmOutput))
            {
                break;
            }

            eventCameraAlarmOutput = new EventCameraAlarmOutput(SYS_EVNT_CAM_ALM_SET,
                                                                cameraList,
                                                                cameraAlarm,
                                                                currentCamforAlarm,
                                                                parentWidget ());
            connect (eventCameraAlarmOutput,
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

    currentclickedIndex =  tempIndex;
}

void SystemEventAndAction::slotSubObjectDelete (quint8 index)
{
    currentclickedIndex = index;
    deleteSubObejects();
    eventsSetBtn[index - SYS_EVNT_IMG_UPLD_SET]->forceActiveFocus ();
}

void SystemEventAndAction:: slotCheckBoxClicked(OPTION_STATE_TYPE_e state,int index)
{
    if(index == SYS_EVNT_ACTION_CTRL)
    {
        bool isEnable = (state == ON_STATE) ? true : false;

        for(quint8 index = 0; index < SYS_EVENT_TYPE_MAX; index++)
        {
            events[index]->setIsEnabled (isEnable);

            if(index < SYS_EVENT_TYPE_MAX - SYS_EVENT_WO_SET_BTN)
            {
                eventsSetBtn[index]->setIsEnabled (isEnable);
            }

            if((index == DEV_ALARM_INDX) && (suppAlarm == 0) && (suppSensors == 0))
            {
                eventsSetBtn[index]->setIsEnabled (false);
                events[index]->setIsEnabled (false);
            }
        }

        #if defined(OEM_JCI)
        events[SYS_EVENT_TYPE_SMS - 1]->setIsEnabled(false);
        eventsSetBtn[SYS_EVENT_TYPE_SMS - 1]->setIsEnabled(false);
        events[SYS_EVENT_TYPE_TCP - 3]->setIsEnabled(false);
        eventsSetBtn[SYS_EVENT_TYPE_TCP - 3]->setIsEnabled(false);
        #endif
    }
    else
    {
        quint8 tempIndex = (SYS_EVNT_IMG_UPLD + index/2 - 3);

        if(tempIndex < (SYS_EVENT_TYPE_MAX - SYS_EVENT_WO_SET_BTN))
        {
            eventsSetBtn[tempIndex]->setIsEnabled (state == ON_STATE ? true : false );
        }

        if((tempIndex == DEV_ALARM_INDX) && (suppAlarm == 0) && (suppSensors == 0))
        {
            eventsSetBtn[tempIndex]->setIsEnabled (false);
        }
    }
}
