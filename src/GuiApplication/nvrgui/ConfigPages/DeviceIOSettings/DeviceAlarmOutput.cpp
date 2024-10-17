#include "DeviceAlarmOutput.h"
#include "ValidationMessage.h"

#define DEV_ALM_MAX_END_FIELD        4

typedef enum{

    DEV_ALM_NUM,
    DEV_ALM_ENABLE,
    DEV_ALM_NAME,
    DEV_ALM_MODE,
    DEV_ALM_PULSE_PERIOD,

    MAX_DEV_ALM_IN_CTRL
}DEV_ALM_IN_CTRL_e;


typedef enum{

    FIELD_ENABLE,
    FIELD_NAME,
    FIELD_MODE,
    FIELD_PULSE_PERIOD,

    MAX_ALM_FIELDS
}DEV_ALM_FIELD_e;

static const QString deviceAlarmInputStrings [] = {
    "Alarm Output",
    "Enable",
    "Name",
    "Active Mode",
    "Pulse Period",
    "(1-255) sec"

};


static const QStringList alarmModeList = QStringList()
        << "Interlock" << "Pulse";

DeviceAlarmOutput::DeviceAlarmOutput(QString devName, QWidget *parent,
                                     DEV_TABLE_INFO_t *devTabInfo)
    : ConfigPageControl(devName, parent,MAX_DEV_ALM_IN_CTRL,devTabInfo)
{
    createDefaultComponents ();
    m_alarmIndex = 1;
    DeviceAlarmOutput::getConfig ();
}

void DeviceAlarmOutput::createDefaultComponents ()
{
    QMap<quint8, QString>  alarmNumberList;

    alarmNumberList.clear ();

    for (quint8 index = 0; index < devTableInfo->alarms; index++)
    {
        alarmNumberList.insert (index,QString("%1").arg (index + 1 ));
    }

    alarmNumberDropDownBox = new DropDown((SCALE_WIDTH(PAGE_RIGHT_PANEL_WIDTH) - BGTILE_MEDIUM_SIZE_WIDTH)/2 + SCALE_WIDTH(10),
                                          (SCALE_HEIGHT(PAGE_RIGHT_PANEL_HEIGHT_WITHOUT_CNFGBUTTON) - 5*BGTILE_HEIGHT)/2,
                                          BGTILE_MEDIUM_SIZE_WIDTH,
                                          BGTILE_HEIGHT,
                                          DEV_ALM_NUM,
                                          DROPDOWNBOX_SIZE_78,
                                          deviceAlarmInputStrings[DEV_ALM_NUM],
                                          alarmNumberList,
                                          this);

    m_elementList[DEV_ALM_NUM] = alarmNumberDropDownBox;

    connect (alarmNumberDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect(alarmNumberDropDownBox,
            SIGNAL(sigValueChanged(QString,quint32)),
            this,
            SLOT(slotSpinBoxValueChange(QString,quint32)));

    enableCheckBox = new OptionSelectButton(alarmNumberDropDownBox->x (),
                                            alarmNumberDropDownBox->y () + BGTILE_HEIGHT,
                                            BGTILE_MEDIUM_SIZE_WIDTH,
                                            BGTILE_HEIGHT,
                                            CHECK_BUTTON_INDEX,
                                            this,
                                            COMMON_LAYER,
                                            deviceAlarmInputStrings[DEV_ALM_ENABLE],
                                            "",-1,
                                            DEV_ALM_ENABLE);

    m_elementList[DEV_ALM_ENABLE] = enableCheckBox;

    connect (enableCheckBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    connect (enableCheckBox,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));

    nameTextBoxParam = new TextboxParam();
    nameTextBoxParam->isCentre = true;
    nameTextBoxParam->labelStr = deviceAlarmInputStrings[DEV_ALM_NAME];
    //    nameTextBoxParam->validation = QRegExp(QString("[^{}\\[\\]()+&=]"));
    nameTextBoxParam->maxChar = 16;

    nameTextBox = new TextBox(enableCheckBox->x (),
                              enableCheckBox->y () + enableCheckBox->height (),
                              BGTILE_MEDIUM_SIZE_WIDTH,
                              BGTILE_HEIGHT,
                              DEV_ALM_NAME,
                              TEXTBOX_MEDIAM,
                              this,
                              nameTextBoxParam);

    m_elementList[DEV_ALM_NAME] = nameTextBox;

    connect (nameTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    QMap<quint8, QString>  alarmModeMapList;

    for(quint8 index = 0; index < alarmModeList.length (); index++)
    {
        alarmModeMapList.insert (index,alarmModeList.at (index));
    }

    alarmModeDropDownBox = new DropDown(nameTextBox->x (),
                                        nameTextBox->y () + nameTextBox->height (),
                                        BGTILE_MEDIUM_SIZE_WIDTH,
                                        BGTILE_HEIGHT,
                                        DEV_ALM_MODE,
                                        DROPDOWNBOX_SIZE_200,
                                        deviceAlarmInputStrings[DEV_ALM_MODE],
                                        alarmModeMapList,
                                        this);

    m_elementList[DEV_ALM_MODE] = alarmModeDropDownBox;

    connect(alarmModeDropDownBox,
            SIGNAL(sigValueChanged(QString,quint32)),
            this,
            SLOT(slotSpinBoxValueChange(QString,quint32)));
    connect (alarmModeDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    pulsePeriodTextBoxParam = new TextboxParam();
    pulsePeriodTextBoxParam->isCentre = true;
    pulsePeriodTextBoxParam->labelStr = deviceAlarmInputStrings[DEV_ALM_PULSE_PERIOD];
    pulsePeriodTextBoxParam->suffixStr = deviceAlarmInputStrings[DEV_ALM_PULSE_PERIOD + 1];
    pulsePeriodTextBoxParam->validation = QRegExp(QString("[0-9]"));
    pulsePeriodTextBoxParam->maxChar = 3;
    pulsePeriodTextBoxParam->minNumValue = 1;
    pulsePeriodTextBoxParam->maxNumValue = 255;
    pulsePeriodTextBoxParam->isNumEntry = true;

    pulsePeriodTextBox = new TextBox(alarmModeDropDownBox->x (),
                                     alarmModeDropDownBox->y () + alarmModeDropDownBox->height (),
                                     BGTILE_MEDIUM_SIZE_WIDTH,
                                     BGTILE_HEIGHT,
                                     DEV_ALM_PULSE_PERIOD,
                                     TEXTBOX_SMALL,
                                     this,
                                     pulsePeriodTextBoxParam);

    m_elementList[DEV_ALM_PULSE_PERIOD] = pulsePeriodTextBox;

    connect (pulsePeriodTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (pulsePeriodTextBox,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotLoadInfopage(int,INFO_MSG_TYPE_e)));
}

DeviceAlarmOutput::~DeviceAlarmOutput ()
{
    disconnect (alarmNumberDropDownBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (alarmNumberDropDownBox,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotSpinBoxValueChange(QString,quint32)));
    delete alarmNumberDropDownBox;

    disconnect (enableCheckBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

    disconnect (enableCheckBox,
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));
    delete enableCheckBox;

    disconnect (nameTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete nameTextBox;
    delete nameTextBoxParam;

    disconnect (alarmModeDropDownBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete alarmModeDropDownBox;

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

void DeviceAlarmOutput::enableControls (bool state)
{
    nameTextBox->setIsEnabled (state);
    alarmModeDropDownBox->setIsEnabled (state);

    if(alarmModeDropDownBox->getCurrValue () == alarmModeList.at (0))
    {
        pulsePeriodTextBox->setIsEnabled (false);
    }
    else
    {
        pulsePeriodTextBox->setIsEnabled (state);
    }
}

void DeviceAlarmOutput::createPayload(REQ_MSG_ID_e msgType)
{
    //    m_alarmIndex  = alarmNumberDropDownBox->getIndexofCurrElement () + 1;

    QString payloadString =
            payloadLib->createDevCnfgPayload(msgType,
                                             SYSTEM_ALARM_OUTPUT_INDEX,
                                             m_alarmIndex,
                                             m_alarmIndex,
                                             CNFG_FRM_INDEX,
                                             MAX_ALM_FIELDS,
                                             MAX_ALM_FIELDS);

    DevCommParam* param = new DevCommParam();
    param->msgType = msgType;
    param->payload = payloadString;

    processBar->loadProcessBar ();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void DeviceAlarmOutput::defaultConfig ()
{
    createPayload(MSG_DEF_CFG);
}

bool DeviceAlarmOutput::isUserChangeConfig()
{
    bool isChange = false;

    do
    {
        if(m_configResponse.isEmpty())
        {
            break;
        }

        if((IS_VALID_OBJ(enableCheckBox)) && (m_configResponse[FIELD_ENABLE] != enableCheckBox->getCurrentState()))
        {
            isChange = true;
            break;
        }

        if((IS_VALID_OBJ(nameTextBox)) && (m_configResponse[FIELD_NAME] != nameTextBox->getInputText()))
        {
            isChange = true;
            break;
        }

        if((IS_VALID_OBJ(alarmModeDropDownBox)) && (m_configResponse[FIELD_MODE] != alarmModeDropDownBox->getIndexofCurrElement()))
        {
            isChange = true;
            break;
        }

        if((IS_VALID_OBJ(pulsePeriodTextBox)) && (m_configResponse[FIELD_PULSE_PERIOD] != pulsePeriodTextBox->getInputText()))
        {
            isChange = true;
            break;
        }
    }while(0);

    return isChange;
}

void DeviceAlarmOutput::handleInfoPageMessage(int index)
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
            m_alarmIndex  = alarmNumberDropDownBox->getIndexofCurrElement () + 1;
            getConfig();
        }
    }
}

void DeviceAlarmOutput::getConfig ()
{
    createPayload(MSG_GET_CFG);
}

void DeviceAlarmOutput::saveConfig ()
{
    bool retVal = false;

    if ( IS_VALID_OBJ(pulsePeriodTextBox))
    {
        retVal = pulsePeriodTextBox->doneKeyValidation();
    }

    if ( retVal == true)
    {
        quint8 temp;

        temp = enableCheckBox->getCurrentState ();
        payloadLib->setCnfgArrayAtIndex (FIELD_ENABLE,temp);

        payloadLib->setCnfgArrayAtIndex (FIELD_NAME,nameTextBox->getInputText ());

        temp = alarmModeDropDownBox->getIndexofCurrElement ();
        payloadLib->setCnfgArrayAtIndex (FIELD_MODE,temp);

        payloadLib->setCnfgArrayAtIndex (FIELD_PULSE_PERIOD,pulsePeriodTextBox->getInputText ());

        createPayload (MSG_SET_CFG);
    }
}

void DeviceAlarmOutput::processDeviceResponse (DevCommParam *param, QString deviceName)
{
    bool isUnloadProcessBar = true;

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

                if(payloadLib->getcnfgTableIndex () == SYSTEM_ALARM_OUTPUT_INDEX)
                {
                    quint8 temp;

                    temp = payloadLib->getCnfgArrayAtIndex (FIELD_ENABLE).toUInt ();
                    m_configResponse[FIELD_ENABLE]=payloadLib->getCnfgArrayAtIndex (FIELD_ENABLE);
                    enableCheckBox->changeState (temp == 1? ON_STATE:OFF_STATE);

                    nameTextBox->setInputText (
                                payloadLib->getCnfgArrayAtIndex (FIELD_NAME).toString ());
                    m_configResponse[FIELD_NAME]=payloadLib->getCnfgArrayAtIndex (FIELD_NAME);

                    alarmModeDropDownBox->setIndexofCurrElement (
                                payloadLib->getCnfgArrayAtIndex (FIELD_MODE).toUInt ());
                    m_configResponse[FIELD_MODE]=payloadLib->getCnfgArrayAtIndex (FIELD_MODE);

                    pulsePeriodTextBox->setInputText (
                                payloadLib->getCnfgArrayAtIndex (FIELD_PULSE_PERIOD).toString ());
                    m_configResponse[FIELD_PULSE_PERIOD]=payloadLib->getCnfgArrayAtIndex (FIELD_PULSE_PERIOD);

                    enableControls ((temp == 1 )? true : false);

                    processBar->unloadProcessBar ();
                }
            }
                break;

            case MSG_SET_CFG:
            {
                isUnloadProcessBar = false;
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
                m_alarmIndex  = alarmNumberDropDownBox->getIndexofCurrElement () + 1;
                getConfig ();
            }
                break;

            case MSG_DEF_CFG:
            {
                isUnloadProcessBar = false;
                m_alarmIndex  = alarmNumberDropDownBox->getIndexofCurrElement () + 1;
                getConfig ();
            }
                break;

            default:
                break;
            }//inner switch

        } break;

        default:
            isUnloadProcessBar = false;
            processBar->unloadProcessBar ();
            infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
            alarmNumberDropDownBox->setIndexofCurrElement (m_alarmIndex);
            break;
        }
    }

    if(isUnloadProcessBar)
    {
        processBar->unloadProcessBar ();
    }
}

void DeviceAlarmOutput::slotCheckBoxClicked (OPTION_STATE_TYPE_e state, int)
{
    enableControls (state);
}

void DeviceAlarmOutput::slotLoadInfopage (int, INFO_MSG_TYPE_e msgtype)
{
    if(msgtype == INFO_MSG_ERROR)
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_PULSE_PERIOD_DEF_RANGE));
    }
}

void DeviceAlarmOutput::slotSpinBoxValueChange(QString str,quint32 indexInPage)
{
    switch(indexInPage)
    {
    case DEV_ALM_NUM:

        if(isUserChangeConfig())
        {
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(SAVE_CHANGES),true,false,"",CONFORMATION_BTN_YES,CONFORMATION_BTN_NO);
        }
        else
        {
            m_alarmIndex  = alarmNumberDropDownBox->getIndexofCurrElement () + 1;
            getConfig ();
        }
        break;

    case DEV_ALM_MODE:
    {
        if(str == alarmModeList.at (0))
        {
            pulsePeriodTextBox->setIsEnabled (false);
        }
        else
        {
            pulsePeriodTextBox->setIsEnabled (true);
        }
    }
        break;

    default:
        break;
    }
}
