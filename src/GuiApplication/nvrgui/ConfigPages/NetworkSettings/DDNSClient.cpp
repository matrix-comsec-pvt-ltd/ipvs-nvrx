#include "DDNSClient.h"
#include "ValidationMessage.h"

#define FIRST_ELE_XOFFSET           SCALE_WIDTH(259)
#define FIRST_ELE_YOFFSET           SCALE_HEIGHT(134)
#define LEFT_MARGIN_FROM_CENTER     SCALE_WIDTH(50)

#define CNFG_TO_INDEX               1
#define DDNS_SER_NAME               "DynDns"

// List of control
typedef enum
{
    DDNS_ENABLE,
    DDNS_USERNAME,
    DDNS_PASSWORD,
    DDNS_HOST_NAME,
    DDNS_UPDATE_INTERVAL,
    DDNS_UPDATE_BUTTON,

    MAX_DDNS_ELEMETS
}DDNS_ELELIST_e;

// cnfg field no According to CMS comm. module
typedef enum
{
    FIELD_DDNS_ENABLE = 0,
    FIELD_USERNAME = 2,
    FIELD_PASSWORD,
    FIELD_HOST_NAME,
    FIELD_INTERVAL,

    MAX_FIELD_NO
}CNFG_FIELD_NO_e;

static const QString labelStr[MAX_DDNS_ELEMETS] =
{
    "Enable DDNS Service",
    "Username",
    "Password",
    "Host Name",
    "Update Interval",
    "Update"
};

DDNSClient::DDNSClient(QString devName, QWidget *parent)
    :ConfigPageControl(devName, parent, MAX_DDNS_ELEMETS)
{
    saveEnable = (OPTION_STATE_TYPE_e) 0;
    createDefaultComponent();
    DDNSClient::getConfig();
}

DDNSClient::~DDNSClient()
{
    disconnect (enableCheckbox,
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotEnableButtonClicked(OPTION_STATE_TYPE_e,int)));
    disconnect(enableCheckbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete enableCheckbox;

    delete serNameReadOnly;

    disconnect (passwordTextbox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete passwordTextbox;
    delete passwordParam;

    disconnect (textBoxes[DDNS_TEXTBOX_INTERVAL],
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotTextboxLoadInfopage(int,INFO_MSG_TYPE_e)));
    for(quint8 index = 0; index < MAX_DDNS_TEXTBOX; index++)
    {
        disconnect (textBoxes[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete textBoxes[index];
        delete textboxParams[index];
    }

    disconnect (updateButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotUpdateButtonClick(int)));
    disconnect(updateButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete updateButton;
}

void DDNSClient::createDefaultComponent()
{
    enableCheckbox = new OptionSelectButton(FIRST_ELE_XOFFSET,
                                            FIRST_ELE_YOFFSET,
                                            BGTILE_MEDIUM_SIZE_WIDTH,
                                            SCALE_HEIGHT(50),
                                            CHECK_BUTTON_INDEX,
                                            labelStr[DDNS_ENABLE],
                                            this,
                                            UP_LAYER,
                                            SCALE_WIDTH(15) ,
                                            MX_OPTION_TEXT_TYPE_SUFFIX,
                                            NORMAL_FONT_SIZE,
                                            DDNS_ENABLE, true, NORMAL_FONT_COLOR, true);
    m_elementList[DDNS_ENABLE] = enableCheckbox;
    connect(enableCheckbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (enableCheckbox,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotEnableButtonClicked(OPTION_STATE_TYPE_e,int)));

    serNameReadOnly = new ReadOnlyElement(enableCheckbox->x (),
                                          enableCheckbox->y () + enableCheckbox->height (),
                                          BGTILE_MEDIUM_SIZE_WIDTH,
                                          BGTILE_HEIGHT,
                                          SCALE_WIDTH(READONLY_MEDIAM_WIDTH),
                                          READONLY_HEIGHT,
                                          DDNS_SER_NAME,
                                          this,
                                          MIDDLE_TABLE_LAYER, -1,
                                          SCALE_WIDTH(10), "DDNS Server", "", "",
                                          SCALE_FONT(10), true, NORMAL_FONT_COLOR,
                                          LEFT_MARGIN_FROM_CENTER);

    textboxParams[DDNS_TEXTBOX_USERNAME] = new TextboxParam();
    textboxParams[DDNS_TEXTBOX_USERNAME]->labelStr = labelStr[DDNS_USERNAME];
    textboxParams[DDNS_TEXTBOX_USERNAME]->maxChar = 40;

    textboxParams[DDNS_TEXTBOX_HOSTNAME] = new TextboxParam();
    textboxParams[DDNS_TEXTBOX_HOSTNAME]->labelStr = labelStr[DDNS_HOST_NAME];
    textboxParams[DDNS_TEXTBOX_HOSTNAME]->maxChar = 40;
    textboxParams[DDNS_TEXTBOX_HOSTNAME]->validation = QRegExp(asciiset1ValidationStringWithoutSpace);

    textboxParams[DDNS_TEXTBOX_INTERVAL] = new TextboxParam();
    textboxParams[DDNS_TEXTBOX_INTERVAL]->labelStr = labelStr[DDNS_UPDATE_INTERVAL];
    textboxParams[DDNS_TEXTBOX_INTERVAL]->suffixStr = "(5 - 99) min";
    textboxParams[DDNS_TEXTBOX_INTERVAL]->isNumEntry = true;
    textboxParams[DDNS_TEXTBOX_INTERVAL]->maxChar = 2;
    textboxParams[DDNS_TEXTBOX_INTERVAL]->minNumValue = 5;
    textboxParams[DDNS_TEXTBOX_INTERVAL]->maxNumValue = 99;
    textboxParams[DDNS_TEXTBOX_INTERVAL]->validation = QRegExp(QString("[0-9]"));

    passwordParam = new TextboxParam();
    passwordParam->labelStr = labelStr[DDNS_PASSWORD];
    passwordParam->maxChar = 24;

    textBoxes[DDNS_TEXTBOX_USERNAME] = new TextBox(serNameReadOnly->x (),
                                                   serNameReadOnly->y () + serNameReadOnly->height (),
                                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                                   BGTILE_HEIGHT,
                                                   DDNS_USERNAME,
                                                   TEXTBOX_ULTRALARGE,
                                                   this, textboxParams[DDNS_TEXTBOX_USERNAME],
                                                   MIDDLE_TABLE_LAYER, true, false, false,
                                                   LEFT_MARGIN_FROM_CENTER);
    m_elementList[DDNS_USERNAME] = textBoxes[DDNS_TEXTBOX_USERNAME];

    passwordTextbox = new PasswordTextbox(textBoxes[DDNS_TEXTBOX_USERNAME]->x (),
                                          textBoxes[DDNS_TEXTBOX_USERNAME]->y () + textBoxes[DDNS_TEXTBOX_USERNAME]->height (),
                                          BGTILE_MEDIUM_SIZE_WIDTH,
                                          BGTILE_HEIGHT,
                                          DDNS_PASSWORD,
                                          TEXTBOX_ULTRALARGE, this, passwordParam,
                                          MIDDLE_TABLE_LAYER, true, LEFT_MARGIN_FROM_CENTER);
    m_elementList[DDNS_PASSWORD] = passwordTextbox;
    connect (passwordTextbox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    textBoxes[DDNS_TEXTBOX_HOSTNAME] = new TextBox(passwordTextbox->x (),
                                                   passwordTextbox->y () + passwordTextbox->height (),
                                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                                   BGTILE_HEIGHT,
                                                   DDNS_HOST_NAME,
                                                   TEXTBOX_ULTRALARGE,
                                                   this, textboxParams[DDNS_TEXTBOX_HOSTNAME],
                                                   MIDDLE_TABLE_LAYER, true, false, false,
                                                   LEFT_MARGIN_FROM_CENTER);
    m_elementList[DDNS_HOST_NAME] = textBoxes[DDNS_TEXTBOX_HOSTNAME];

    textBoxes[DDNS_TEXTBOX_INTERVAL] = new TextBox(textBoxes[DDNS_TEXTBOX_HOSTNAME]->x (),
                                                   textBoxes[DDNS_TEXTBOX_HOSTNAME]->y () + textBoxes[DDNS_TEXTBOX_HOSTNAME]->height (),
                                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                                   BGTILE_HEIGHT,
                                                   DDNS_UPDATE_INTERVAL,
                                                   TEXTBOX_EXTRASMALL,
                                                   this, textboxParams[DDNS_TEXTBOX_INTERVAL],
                                                   MIDDLE_TABLE_LAYER, true, false, false,
                                                   LEFT_MARGIN_FROM_CENTER);
    m_elementList[DDNS_UPDATE_INTERVAL] = textBoxes[DDNS_TEXTBOX_INTERVAL];

    for(quint8 index = 0; index < MAX_DDNS_TEXTBOX; index++)
    {
        connect (textBoxes[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
    }
    connect (textBoxes[DDNS_TEXTBOX_INTERVAL],
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotTextboxLoadInfopage(int,INFO_MSG_TYPE_e)));

    updateButton = new ControlButton(UPDATE_BUTTON_INDEX,
                                     textBoxes[DDNS_TEXTBOX_INTERVAL]->x (),
                                     textBoxes[DDNS_TEXTBOX_INTERVAL]->y () + textBoxes[DDNS_TEXTBOX_INTERVAL]->height (),
                                     BGTILE_MEDIUM_SIZE_WIDTH, BGTILE_HEIGHT,
                                     this, DOWN_LAYER, SCALE_WIDTH(365),
                                     labelStr[DDNS_UPDATE_BUTTON],
                                     true, DDNS_UPDATE_BUTTON);
    m_elementList[DDNS_UPDATE_BUTTON] = updateButton;
    connect(updateButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (updateButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotUpdateButtonClick(int)));
}

void DDNSClient::getConfig()
{
    //create the payload for Get Cnfg
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                             DDNS_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_TO_INDEX,
                                                             CNFG_FRM_FIELD,
                                                             MAX_FIELD_NO,
                                                             0);

    DevCommParam* param = new DevCommParam();

    param->msgType = MSG_GET_CFG;
    param->payload = payloadString;

    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void DDNSClient::defaultConfig()
{
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_DEF_CFG,
                                                             DDNS_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_TO_INDEX,
                                                             CNFG_FRM_FIELD,
                                                             MAX_FIELD_NO,
                                                             0);

    DevCommParam* param = new DevCommParam();

    param->msgType = MSG_DEF_CFG;
    param->payload = payloadString;

    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void DDNSClient::saveConfig()
{
    if ( IS_VALID_OBJ(textBoxes[DDNS_TEXTBOX_INTERVAL]))
    {
        if(!textBoxes[DDNS_TEXTBOX_INTERVAL]->doneKeyValidation())
        {
           return;
        }
    }

    QString tempUserName = textBoxes[DDNS_TEXTBOX_USERNAME]->getInputText ();
    QString tempPassword = passwordTextbox->getInputText ();
    QString tempHostname = textBoxes[DDNS_TEXTBOX_HOSTNAME]->getInputText ();
    OPTION_STATE_TYPE_e tempstate = enableCheckbox->getCurrentState ();

    if(tempstate == ON_STATE)
    {
        if(tempUserName == "")
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_USER_NM));
            return;
        }
        else if(tempPassword == "")
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_PASSWORD));
            return;
        }
        else if(tempHostname == "")
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_HOSTNAME));
            return;
        }
    }

    // This is fix, becuse we have only one ddns server as of now
    payloadLib->setCnfgArrayAtIndex (1, 0);

    payloadLib->setCnfgArrayAtIndex (FIELD_DDNS_ENABLE, tempstate);
    payloadLib->setCnfgArrayAtIndex (FIELD_USERNAME, tempUserName);
    payloadLib->setCnfgArrayAtIndex (FIELD_PASSWORD, tempPassword);
    payloadLib->setCnfgArrayAtIndex (FIELD_HOST_NAME, tempHostname);
    payloadLib->setCnfgArrayAtIndex (FIELD_INTERVAL,
                                     textBoxes[DDNS_TEXTBOX_INTERVAL]->getInputText ());

    //create the payload for Get Cnfg
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_SET_CFG,
                                                             DDNS_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_TO_INDEX,
                                                             CNFG_FRM_FIELD,
                                                             MAX_FIELD_NO,
                                                             MAX_FIELD_NO);

    DevCommParam* param = new DevCommParam();

    param->msgType = MSG_SET_CFG;
    param->payload = payloadString;

    processBar->loadProcessBar ();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void DDNSClient::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    //    OPTION_STATE_TYPE_e state;
    if(deviceName == currDevName)
    {
        switch(param->deviceStatus)
        {
        case CMD_SUCCESS:

            switch(param->msgType)
            {
            case MSG_GET_CFG:

                payloadLib->parsePayload(param->msgType, param->payload);
                if(payloadLib->getcnfgTableIndex () == DDNS_TABLE_INDEX)
                {
                    saveEnable = (OPTION_STATE_TYPE_e)
                            (payloadLib->getCnfgArrayAtIndex (FIELD_DDNS_ENABLE).toUInt ());

                    saveUsrname = payloadLib->getCnfgArrayAtIndex (FIELD_USERNAME).toString ();
                    savePassword = payloadLib->getCnfgArrayAtIndex (FIELD_PASSWORD).toString ();

                    saveHostname = payloadLib->getCnfgArrayAtIndex (FIELD_HOST_NAME).toString ();
                    saveInterval = payloadLib->getCnfgArrayAtIndex (FIELD_INTERVAL).toString ();

                    enableCheckbox->changeState (saveEnable);
                    textBoxes[DDNS_TEXTBOX_USERNAME]->setInputText (saveUsrname);
                    textBoxes[DDNS_TEXTBOX_HOSTNAME]->setInputText (saveHostname);
                    textBoxes[DDNS_TEXTBOX_INTERVAL]->setInputText (saveInterval);
                    passwordTextbox->setInputText (savePassword);

                    slotEnableButtonClicked(saveEnable, 0);
                }
                processBar->unloadProcessBar ();

                break;

            case MSG_SET_CFG:
                // unload processing icon
                processBar->unloadProcessBar ();
                saveEnable = enableCheckbox->getCurrentState ();
                saveUsrname = textBoxes[DDNS_TEXTBOX_USERNAME]->getInputText ();
                savePassword = passwordTextbox->getInputText ();
                saveHostname = textBoxes[DDNS_TEXTBOX_HOSTNAME]->getInputText ();
                saveInterval = textBoxes[DDNS_TEXTBOX_INTERVAL]->getInputText ();;
                //load info page with msg
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
                break;

            case MSG_DEF_CFG:
                getConfig();
                break;

            case MSG_SET_CMD:
                if(param->cmdType == UPDT_DDNS)
                {
                    processBar->unloadProcessBar ();
                    infoPage->loadInfoPage (ValidationMessage::getValidationMessage(DDNS_CLIENT_UPDATED_SUCCESS));
                }
                break;

            default:
                break;
            }

            break;

        default:
            processBar->unloadProcessBar ();
            infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
            break;
        }
        update ();
    }
}

void DDNSClient::slotEnableButtonClicked(OPTION_STATE_TYPE_e state, int)
{
    for(quint8 index = 0; index < MAX_DDNS_TEXTBOX; index++)
    {
        textBoxes[index]->setIsEnabled (state);
    }
    passwordTextbox->setIsEnabled (state);
    updateButton->setIsEnabled (state);
}

void DDNSClient::slotUpdateButtonClick(int)
{
    QString tempUserName = textBoxes[DDNS_TEXTBOX_USERNAME]->getInputText ();
    QString tempPassword = passwordTextbox->getInputText ();
    QString tempHostname = textBoxes[DDNS_TEXTBOX_HOSTNAME]->getInputText ();
    OPTION_STATE_TYPE_e tempstate = enableCheckbox->getCurrentState ();

    if( (saveEnable != tempstate) ||
            (saveUsrname != tempUserName) ||
            (savePassword != tempPassword) ||
            (saveHostname != tempHostname) ||
            (saveInterval != textBoxes[DDNS_TEXTBOX_INTERVAL]->getInputText ()) )
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(CONFI_CONTROL_TEST_CONN_WARNG_MSG));
    }
    else
    {
        if(tempstate == ON_STATE)
        {
            if(tempUserName == "" )
            {
                infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_USER_NM));
                return;
            }
            else if(tempPassword == "" )
            {
                infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_PASSWORD));
                return;
            }
            else if(tempHostname == "" )
            {
                infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_HOSTNAME));
                return;
            }
        }

        DevCommParam* param = new DevCommParam();
        param->msgType = MSG_SET_CMD;
        param->cmdType = UPDT_DDNS;

        processBar->loadProcessBar ();
        applController->processActivity(currDevName, DEVICE_COMM, param);
    }
}

void DDNSClient::slotTextboxLoadInfopage(int, INFO_MSG_TYPE_e msgType)
{
    if(msgType == INFO_MSG_ERROR)
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(DDNS_CLIENT_UPDATE_INT_IN_RANGE));
    }
}
