#include "DeviceSensorInput.h"
#include "ValidationMessage.h"

typedef enum{

    DEV_SEN_ENBL_DETECTION,
    DEV_SEN_NAME,
    DEV_SEN_MODE,
    DEV_SEN_DELAY,

    MAX_DEV_SENS_IN_FILDS
}DEV_SENS_IN_FILDS_e;

typedef enum{

    DEV_SENS_IN_SPINBOX,
    DEV_SENS_ENABLE,
    DEV_SENS_NAME,
    DEV_SENS_MODE_SPINBOX,
    DEV_SENS_DELAY_SPINBOX,

    MAX_DEV_SENS_IN_CTRL
}DEV_SENS_IN_CTRL_e;

static const QString deviceSensorInputStrings [] = {
    "Sensor Input",
    "Enable",
    "Name",
    "Mode",
    "Delay Time",
    "(0-10) sec"
};

static const QStringList sensorModeList = QStringList()
        << "Normally Open" << "Normally Close";

DeviceSensorInput::DeviceSensorInput(QString devName, QWidget *parent, DEV_TABLE_INFO_t *devTabInfo)
    : ConfigPageControl(devName, parent,MAX_DEV_SENS_IN_CTRL,devTabInfo)
{
    createDefaultComponents ();
    m_sensorIndex = 1;
    DeviceSensorInput::getConfig ();
}

void DeviceSensorInput::createDefaultComponents ()
{
    sensorList.clear ();
    for(quint8 index = 0; index < devTableInfo->sensors; index++)
    {
        sensorList.insert(index, QString("%1").arg(index + 1));
    }

    sensorInputDropDownBox = new DropDown((SCALE_WIDTH(PAGE_RIGHT_PANEL_WIDTH) - BGTILE_MEDIUM_SIZE_WIDTH)/2 + SCALE_WIDTH(10),
                                          (SCALE_HEIGHT(PAGE_RIGHT_PANEL_HEIGHT_WITHOUT_CNFGBUTTON) - 5*BGTILE_HEIGHT)/2,
                                          BGTILE_MEDIUM_SIZE_WIDTH,
                                          BGTILE_HEIGHT,
                                          DEV_SENS_IN_SPINBOX,
                                          DROPDOWNBOX_SIZE_90,
                                          deviceSensorInputStrings[DEV_SENS_IN_SPINBOX],
                                          sensorList,
                                          this);

    m_elementList[DEV_SENS_IN_SPINBOX] = sensorInputDropDownBox;

    connect (sensorInputDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (sensorInputDropDownBox,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotSpinBoxValueChanged(QString,quint32)));

    enableCheckBox = new OptionSelectButton(sensorInputDropDownBox->x (),
                                            sensorInputDropDownBox->y () + BGTILE_HEIGHT,
                                            BGTILE_MEDIUM_SIZE_WIDTH,
                                            BGTILE_HEIGHT,
                                            CHECK_BUTTON_INDEX,
                                            this,
                                            COMMON_LAYER,
                                            deviceSensorInputStrings[DEV_SENS_ENABLE],
                                            "",-1,
                                            DEV_SENS_ENABLE);

    m_elementList[DEV_SENS_ENABLE] = enableCheckBox;

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
    nameTextBoxParam->labelStr = deviceSensorInputStrings[DEV_SENS_NAME];
    //    nameTextBoxParam->validation = QRegExp(QString("[^{}\\[\\]()+&=]"));
    nameTextBoxParam->maxChar = 16;

    nameTextBox = new TextBox(enableCheckBox->x (),
                              enableCheckBox->y () + enableCheckBox->height (),
                              BGTILE_MEDIUM_SIZE_WIDTH,
                              BGTILE_HEIGHT,
                              DEV_SENS_NAME,
                              TEXTBOX_MEDIAM,
                              this,
                              nameTextBoxParam);

    m_elementList[DEV_SENS_NAME] = nameTextBox;

    connect (nameTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    QMap<quint8, QString>  sensorModeMapList;

    for(quint8 index = 0; index < sensorModeList.length (); index++)
    {
        sensorModeMapList.insert (index,sensorModeList.at (index));
    }

    sensorModeDropDownBox = new DropDown(nameTextBox->x (),
                                         nameTextBox->y () + nameTextBox->height (),
                                         BGTILE_MEDIUM_SIZE_WIDTH,
                                         BGTILE_HEIGHT,
                                         DEV_SENS_MODE_SPINBOX,
                                         DROPDOWNBOX_SIZE_200,
                                         deviceSensorInputStrings[DEV_SENS_MODE_SPINBOX],
                                         sensorModeMapList,
                                         this);

    m_elementList[DEV_SENS_MODE_SPINBOX] = sensorModeDropDownBox;

    connect (sensorModeDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    delayList.clear ();
    for(quint8 index = 0; index <= 10; index++)
    {
        delayList.insert(index, QString("%1").arg(index));
    }

    delayDropDownBox = new DropDown(sensorModeDropDownBox->x (),
                                    sensorModeDropDownBox->y () + sensorModeDropDownBox->height (),
                                    BGTILE_MEDIUM_SIZE_WIDTH,
                                    BGTILE_HEIGHT,
                                    DEV_SENS_DELAY_SPINBOX,
                                    DROPDOWNBOX_SIZE_90,
                                    deviceSensorInputStrings[DEV_SENS_DELAY_SPINBOX],
                                    delayList,
                                    this,
                                    deviceSensorInputStrings[DEV_SENS_DELAY_SPINBOX+1],
                                    true,
                                    0,
                                    COMMON_LAYER,
                                    true,
                                    5);

    m_elementList[DEV_SENS_DELAY_SPINBOX] = delayDropDownBox;

    connect (delayDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
}

DeviceSensorInput::~DeviceSensorInput()
{
    sensorList.clear ();

    disconnect (sensorInputDropDownBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

    disconnect (sensorInputDropDownBox,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotSpinBoxValueChanged(QString,quint32)));
    delete sensorInputDropDownBox;


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

    disconnect (sensorModeDropDownBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete sensorModeDropDownBox;

    delayList.clear ();


    disconnect (delayDropDownBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete delayDropDownBox;
}

void DeviceSensorInput::enableControls (bool state)
{
    nameTextBox->setIsEnabled (state);
    sensorModeDropDownBox->setIsEnabled (state);
    delayDropDownBox->setIsEnabled (state);
}

void DeviceSensorInput::createPayload(REQ_MSG_ID_e msgType)
{
    //    m_sensorIndex = sensorInputDropDownBox->getIndexofCurrElement () + 1;

    QString payloadString =
            payloadLib->createDevCnfgPayload(msgType,
                                             SYSTEM_SENSOR_INPUT_TABLE_INDEX,
                                             m_sensorIndex,
                                             m_sensorIndex,
                                             CNFG_FRM_INDEX,
                                             MAX_DEV_SENS_IN_FILDS,
                                             MAX_DEV_SENS_IN_FILDS);

    DevCommParam* param = new DevCommParam();
    param->msgType = msgType;
    param->payload = payloadString;

    processBar->loadProcessBar ();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void DeviceSensorInput::defaultConfig ()
{
    createPayload(MSG_DEF_CFG);
}

bool DeviceSensorInput::isUserChangeConfig()
{
    bool isChange = false;

    do
    {
        if(m_configResponse.isEmpty())
        {
            break;
        }

        if((IS_VALID_OBJ(enableCheckBox)) && (m_configResponse[DEV_SEN_ENBL_DETECTION]!= enableCheckBox->getCurrentState()))
        {
            isChange = true;
            break;
        }

        if((IS_VALID_OBJ(nameTextBox)) && (m_configResponse[DEV_SEN_NAME] != nameTextBox->getInputText()))
        {
            isChange = true;
            break;
        }

        if((IS_VALID_OBJ(sensorModeDropDownBox)) && (m_configResponse[DEV_SEN_MODE] != sensorModeDropDownBox->getIndexofCurrElement()))
        {
            isChange = true;
            break;
        }

        if((IS_VALID_OBJ(delayDropDownBox)) && (m_configResponse[DEV_SEN_DELAY] != delayDropDownBox->getIndexofCurrElement()))
        {
            isChange = true;
            break;
        }
    }while(0);

    return isChange;
}

void DeviceSensorInput::handleInfoPageMessage(int index)
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
            m_sensorIndex = sensorInputDropDownBox->getIndexofCurrElement () + 1;
            getConfig();
        }
    }
}

void DeviceSensorInput::getConfig ()
{
    createPayload(MSG_GET_CFG);
}

void DeviceSensorInput::saveConfig ()
{
    quint8 temp;

    temp = enableCheckBox->getCurrentState ();
    payloadLib->setCnfgArrayAtIndex (DEV_SEN_ENBL_DETECTION,temp);

    payloadLib->setCnfgArrayAtIndex (DEV_SEN_NAME,nameTextBox->getInputText ());

    temp = sensorModeDropDownBox->getIndexofCurrElement ();
    payloadLib->setCnfgArrayAtIndex (DEV_SEN_MODE,temp);

    temp = delayDropDownBox->getIndexofCurrElement ();
    payloadLib->setCnfgArrayAtIndex (DEV_SEN_DELAY,temp);

    createPayload (MSG_SET_CFG);
}

void DeviceSensorInput::processDeviceResponse (DevCommParam *param, QString deviceName)
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

                if(payloadLib->getcnfgTableIndex () == SYSTEM_SENSOR_INPUT_TABLE_INDEX)
                {
                    quint8 temp;
                    temp = payloadLib->getCnfgArrayAtIndex (DEV_SEN_ENBL_DETECTION).toUInt ();
                    m_configResponse[DEV_SEN_ENBL_DETECTION]=payloadLib->getCnfgArrayAtIndex (DEV_SEN_ENBL_DETECTION);

                    enableCheckBox->changeState (temp == 1? ON_STATE:OFF_STATE);
                    enableControls (temp == 1 ? true : false);

                    nameTextBox->setInputText (
                                payloadLib->getCnfgArrayAtIndex (DEV_SEN_NAME).toString ());
                    m_configResponse[DEV_SEN_NAME]=payloadLib->getCnfgArrayAtIndex (DEV_SEN_NAME);

                    sensorModeDropDownBox->setIndexofCurrElement (
                                payloadLib->getCnfgArrayAtIndex (DEV_SEN_MODE).toUInt ());
                    m_configResponse[DEV_SEN_MODE]=payloadLib->getCnfgArrayAtIndex (DEV_SEN_MODE);

                    delayDropDownBox->setIndexofCurrElement (
                                payloadLib->getCnfgArrayAtIndex (DEV_SEN_DELAY).toUInt ());
                    m_configResponse[DEV_SEN_DELAY]=payloadLib->getCnfgArrayAtIndex (DEV_SEN_DELAY);

                    processBar->unloadProcessBar ();
                }
            }
                break;

            case MSG_SET_CFG:
            {
                isUnloadProcessbar = false;
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
                m_sensorIndex = sensorInputDropDownBox->getIndexofCurrElement () + 1;
                getConfig ();
            }
                break;

            case MSG_DEF_CFG:
            {
                isUnloadProcessbar = false;
                m_sensorIndex = sensorInputDropDownBox->getIndexofCurrElement () + 1;
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
            sensorInputDropDownBox->setIndexofCurrElement (m_sensorIndex);
            break;
        }
    }

    if(isUnloadProcessbar)
    {
        processBar->unloadProcessBar ();
    }
}

void DeviceSensorInput::slotCheckBoxClicked (OPTION_STATE_TYPE_e state, int)
{
    enableControls (state);
}

void DeviceSensorInput::slotSpinBoxValueChanged (QString, quint32)
{
    if(isUserChangeConfig())
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(SAVE_CHANGES),true,false,"",CONFORMATION_BTN_YES,CONFORMATION_BTN_NO);
    }
    else
    {
        m_sensorIndex = sensorInputDropDownBox->getIndexofCurrElement () + 1;
        getConfig ();
    }
}
