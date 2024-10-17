#include "CameraAlarmOutput.h"
#include "ValidationMessage.h"

typedef enum{

    CAMERA_ALM_CAMERA_NAME_SPINBOX,
    CAMERA_ALM_CAMERA_ALARM_SPINBOX,
    CAMERA_ALM_ACTIVE_MODE_SPINBOX,
    CAMERA_ALM_PULSE_PERIOD,

    MAX_CAMERA_ALARM_CTRL
}CAMERA_ALARM_CTRL_e;

typedef enum{

    CAM_ALM_FIELD_ACTIVE_MODE,
    CAM_ALM_FIELD_PULSE_PERIOD,

    MAX_CAMERA_ALARM_FEILD
}CAMERA_ALARM_FEILD_e;

static const QString cameraAlarmStrings[] = {
    "Camera",
    "Camera Alarm",
    "Active Mode",
    "Pulse Period"
};

static const QStringList alarmModeList = QStringList()
        << "Interlock" << "Pulse";

CameraAlarmOutput::CameraAlarmOutput(QString deviceName,
                                     QWidget* parent,
                                     DEV_TABLE_INFO_t* devTab) :
    ConfigPageControl(deviceName,parent,MAX_CAMERA_ALARM_CTRL,devTab)
{
    createDefaultComponents ();
    currentCameraIndex = 0;
    currentAlarmIndex = 0;
    CameraAlarmOutput::getConfig ();
}

CameraAlarmOutput::~CameraAlarmOutput ()
{
    cameraNameList.clear();

    disconnect (cameraNameDropDownBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (cameraNameDropDownBox,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotSpinBoxValueChange(QString,quint32)));
    delete cameraNameDropDownBox;

    disconnect (cameraAlarmDropDownBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (cameraAlarmDropDownBox,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotSpinBoxValueChange(QString,quint32)));
    delete cameraAlarmDropDownBox;

    disconnect (activeModeSpinbox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (activeModeSpinbox,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotSpinBoxValueChange(QString,quint32)));
    delete activeModeSpinbox;

    disconnect (pulsePeriodTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (pulsePeriodTextBox,
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotLoadInfopage(int,INFO_MSG_TYPE_e)));
    delete pulsePeriodTextBox;
    delete pulsePeriodTextBoxParam;
}

void CameraAlarmOutput::getCameraList ()
{
    QString tempStr;
    cameraNameList.clear();

    //    for(quint8 index =  devTableInfo->analogCams; index < devTableInfo->totalCams; index++)
    for(quint8 index = 0; index < devTableInfo->ipCams; index++)
    {
        tempStr = applController->GetCameraNameOfDevice(currDevName,(index + devTableInfo->analogCams));
        if(((index + 1 + devTableInfo->analogCams) < 10) && (devTableInfo->totalCams > 10))
        {
            cameraNameList.insert(index, QString(" %1%2%3").arg(index + 1 + devTableInfo->analogCams)
                                  .arg(" : ").arg (tempStr));
        }
        else
        {
            cameraNameList.insert(index, QString("%1%2%3").arg(index + 1 + devTableInfo->analogCams)
                                  .arg(" : ").arg (tempStr));
        }
    }
    cameraNameDropDownBox->setNewList (cameraNameList,currentCameraIndex);
}

void CameraAlarmOutput::createDefaultComponents ()
{
    QMap<quint8, QString> alarmList;
    alarmList.clear ();
    cameraNameList.clear ();

    // fill camera alarm List
    for(quint8 index = 0; index < MAX_CAM_ALARM; index++)
    {
        alarmList.insert (index,QString("%1").arg (index + 1));
    }

    //    currentCameraIndex = 1;

    cameraNameDropDownBox = new DropDown((SCALE_WIDTH(PAGE_RIGHT_PANEL_WIDTH) - BGTILE_ULTRAMEDIUM_SIZE_WIDTH)/2 + SCALE_WIDTH(10),
                                         (SCALE_HEIGHT(PAGE_RIGHT_PANEL_HEIGHT_WITHOUT_CNFGBUTTON) - (MAX_CAMERA_ALARM_CTRL*BGTILE_HEIGHT))/2,
                                         BGTILE_ULTRAMEDIUM_SIZE_WIDTH,
                                         BGTILE_HEIGHT,
                                         CAMERA_ALM_CAMERA_NAME_SPINBOX,
                                         DROPDOWNBOX_SIZE_320,
                                         cameraAlarmStrings[CAMERA_ALM_CAMERA_NAME_SPINBOX],
                                         cameraNameList,
                                         this,
                                         "");

    m_elementList[CAMERA_ALM_CAMERA_NAME_SPINBOX] = cameraNameDropDownBox;

    connect (cameraNameDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (cameraNameDropDownBox,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotSpinBoxValueChange(QString,quint32)));

    cameraAlarmDropDownBox = new DropDown(cameraNameDropDownBox->x (),
                                          cameraNameDropDownBox->y () + cameraNameDropDownBox->height (),
                                          BGTILE_ULTRAMEDIUM_SIZE_WIDTH,
                                          BGTILE_HEIGHT,
                                          CAMERA_ALM_CAMERA_ALARM_SPINBOX,
                                          DROPDOWNBOX_SIZE_320,
                                          cameraAlarmStrings[CAMERA_ALM_CAMERA_ALARM_SPINBOX],
                                          alarmList,
                                          this,
                                          "");

    m_elementList[CAMERA_ALM_CAMERA_ALARM_SPINBOX] = cameraAlarmDropDownBox;

    connect (cameraAlarmDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (cameraAlarmDropDownBox,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotSpinBoxValueChange(QString,quint32)));

    QMap<quint8, QString> alarmModeMapList;
    for(quint8 index = 0; index < alarmModeList.length (); index++)
    {
        alarmModeMapList.insert (index,alarmModeList.at (index));
    }

    activeModeSpinbox = new DropDown(cameraAlarmDropDownBox->x (),
                                     cameraAlarmDropDownBox->y () + cameraAlarmDropDownBox->height (),
                                     BGTILE_ULTRAMEDIUM_SIZE_WIDTH,
                                     BGTILE_HEIGHT,
                                     CAMERA_ALM_ACTIVE_MODE_SPINBOX,
                                     DROPDOWNBOX_SIZE_320,
                                     cameraAlarmStrings[CAMERA_ALM_ACTIVE_MODE_SPINBOX],
                                     alarmModeMapList,
                                     this,
                                     "");

    m_elementList[CAMERA_ALM_ACTIVE_MODE_SPINBOX] = activeModeSpinbox;

    connect (activeModeSpinbox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (activeModeSpinbox,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotSpinBoxValueChange(QString,quint32)));

    pulsePeriodTextBoxParam = new TextboxParam();
    pulsePeriodTextBoxParam->isCentre = true;
    pulsePeriodTextBoxParam->labelStr = cameraAlarmStrings[CAMERA_ALM_PULSE_PERIOD];
    pulsePeriodTextBoxParam->suffixStr = QString("(1-255 ") + Multilang("sec")+QString(")");
    pulsePeriodTextBoxParam->validation = QRegExp(QString("[0-9]"));
    pulsePeriodTextBoxParam->maxChar = 3;
    pulsePeriodTextBoxParam->minNumValue = 1;
    pulsePeriodTextBoxParam->maxNumValue = 255;
    pulsePeriodTextBoxParam->isNumEntry = true;

    pulsePeriodTextBox = new TextBox(activeModeSpinbox->x (),
                                     activeModeSpinbox->y () + activeModeSpinbox->height (),
                                     BGTILE_ULTRAMEDIUM_SIZE_WIDTH,
                                     BGTILE_HEIGHT,
                                     CAMERA_ALM_PULSE_PERIOD,
                                     TEXTBOX_SMALL,
                                     this,
                                     pulsePeriodTextBoxParam);

    m_elementList[CAMERA_ALM_PULSE_PERIOD] = pulsePeriodTextBox;

    connect (pulsePeriodTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (pulsePeriodTextBox,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotLoadInfopage(int,INFO_MSG_TYPE_e)));
}

void CameraAlarmOutput::createPayload(REQ_MSG_ID_e msgType)
{
    quint8 cnfgFrmIndex  = (((currentCameraIndex
                              + devTableInfo->analogCams)* MAX_CAM_ALARM)
                            + currentAlarmIndex
                            + 1 );

    QString payloadString =
            payloadLib->createDevCnfgPayload(msgType,
                                             CAMERA_ALARM_OUTPUT_TABLE_INDEX,
                                             cnfgFrmIndex,
                                             cnfgFrmIndex,
                                             CNFG_FRM_INDEX,
                                             MAX_CAMERA_ALARM_FEILD,
                                             MAX_CAMERA_ALARM_FEILD);

    DevCommParam* param = new DevCommParam();
    param->msgType = msgType;
    param->payload = payloadString;

    processBar->loadProcessBar ();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void CameraAlarmOutput::defaultConfig ()
{
    createPayload(MSG_DEF_CFG);
}

void CameraAlarmOutput::getConfig ()
{
    //    currentCameraIndex = cameraNameDropDownBox->getIndexofCurrElement ();
    getCameraList();
    createPayload(MSG_GET_CFG);
}

void CameraAlarmOutput::saveConfig ()
{
    if (IS_VALID_OBJ(pulsePeriodTextBox))
    {
        if (!pulsePeriodTextBox->doneKeyValidation())
        {
            return;
        }
    }

    payloadLib->setCnfgArrayAtIndex (CAM_ALM_FIELD_ACTIVE_MODE,
                                     activeModeSpinbox->getIndexofCurrElement ());
    payloadLib->setCnfgArrayAtIndex (CAM_ALM_FIELD_PULSE_PERIOD,
                                     pulsePeriodTextBox->getInputText ());

    createPayload (MSG_SET_CFG);
}

void CameraAlarmOutput::processDeviceResponse (DevCommParam *param, QString deviceName)
{
    bool isUnloadProcessbar = true;

    if(deviceName == currDevName)
    {
        switch(param->deviceStatus)
        {
        case CMD_SUCCESS:
        {
            switch(param->msgType)
            {
            case MSG_GET_CFG:
            {
                m_configResponse.clear();

                payloadLib->parsePayload (param->msgType, param->payload);
                quint8 tempIndex = payloadLib->getCnfgArrayAtIndex (CAM_ALM_FIELD_ACTIVE_MODE).toUInt ();
                tempIndex = (tempIndex < MAX_CAMERA_ALARM_FEILD) ? tempIndex : 0;

                m_configResponse[CAM_ALM_FIELD_ACTIVE_MODE]=payloadLib->getCnfgArrayAtIndex (CAM_ALM_FIELD_ACTIVE_MODE);

                activeModeSpinbox->setIndexofCurrElement (tempIndex);
                pulsePeriodTextBox->setInputText
                        (payloadLib->getCnfgArrayAtIndex (CAM_ALM_FIELD_PULSE_PERIOD).toString ());
                pulsePeriodTextBox->setIsEnabled ((tempIndex == 1) ? true : false);

                m_configResponse[CAM_ALM_FIELD_PULSE_PERIOD]=payloadLib->getCnfgArrayAtIndex (CAM_ALM_FIELD_PULSE_PERIOD);
           }
                break;

            case MSG_SET_CFG:
            {
                isUnloadProcessbar = false;
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
                currentCameraIndex = cameraNameDropDownBox->getIndexofCurrElement () ;
                getConfig ();
            }
                break;

            case MSG_DEF_CFG:
            {
                isUnloadProcessbar = false;
                currentCameraIndex = cameraNameDropDownBox->getIndexofCurrElement () ;
                getConfig ();
            }
            	break;

            default:
                break;
            }//inner switch

        } break;

        default:
            isUnloadProcessbar = false;
            processBar->unloadProcessBar ();
            infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
            cameraNameDropDownBox->setIndexofCurrElement (currentCameraIndex) ;
            break;
        }
    }

    if(isUnloadProcessbar)
    {
        processBar->unloadProcessBar ();
    }
}

bool CameraAlarmOutput::isUserChangeConfig()
{
    bool isChange = false;

    do
    {
        if((IS_VALID_OBJ(activeModeSpinbox)) && (m_configResponse[CAM_ALM_FIELD_ACTIVE_MODE] != activeModeSpinbox->getIndexofCurrElement () ))
        {
            isChange = true;
            break;
        }

        if((IS_VALID_OBJ(activeModeSpinbox)) && (m_configResponse[CAM_ALM_FIELD_PULSE_PERIOD] != pulsePeriodTextBox->getInputText() ))
        {
            isChange = true;
            break;
        }

    }while(0);

    return isChange;
}

void CameraAlarmOutput::handleInfoPageMessage(int index)
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
            currentCameraIndex = cameraNameDropDownBox->getIndexofCurrElement ();
            getConfig();
        }
    }
}

void CameraAlarmOutput::slotSpinBoxValueChange (QString str, quint32 index)
{
    switch(index)
    {
    case CAMERA_ALM_CAMERA_NAME_SPINBOX:
    {
        if(isUserChangeConfig())
        {
            infoPage->loadInfoPage((ValidationMessage::getValidationMessage(SAVE_CHANGES)),true,false,"",CONFORMATION_BTN_YES,CONFORMATION_BTN_NO);
        }
        else
        {
            currentCameraIndex = cameraNameDropDownBox->getIndexofCurrElement ();
            getConfig ();
        }
    }
        break;


    case CAMERA_ALM_CAMERA_ALARM_SPINBOX:
    {
        if(isUserChangeConfig())
        {
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(SAVE_CHANGES),true,false,"",CONFORMATION_BTN_YES,CONFORMATION_BTN_NO);
        }
        else
        {
            currentCameraIndex = cameraNameDropDownBox->getIndexofCurrElement ();
            currentAlarmIndex = cameraAlarmDropDownBox->getIndexofCurrElement();
            getConfig ();
        }
    }
        break;

    case CAMERA_ALM_ACTIVE_MODE_SPINBOX:
        if(str == alarmModeList.at (0))
        {
            pulsePeriodTextBox->setIsEnabled (false);
        }
        else
        {
            pulsePeriodTextBox->setIsEnabled (true);
        }
        break;

    default:
        break;

    }
}

void CameraAlarmOutput::slotLoadInfopage(int,INFO_MSG_TYPE_e msgtype)
{
    if(msgtype == INFO_MSG_ERROR)
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_PULSE_PERIOD_DEF_RANGE));
    }
}
