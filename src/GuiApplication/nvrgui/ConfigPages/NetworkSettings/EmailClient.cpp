#include "EmailClient.h"
#include "ValidationMessage.h"

#define CNFG_TO_INDEX               1
#define FIRST_ELE_XOFFSET           SCALE_WIDTH(261)
#define FIRST_ELE_YOFFSET           SCALE_HEIGHT(61) //(558 - 436)/2
#define LEFT_MARGIN_FROM_CENTER     SCALE_WIDTH(60)

// List of control
typedef enum
{
    EMAIL_ENABLE,
    EMAIL_MAIL_SERVER,
    EMAIL_PORT,
    EMAIL_USERNAME,
    EMAIL_PASSWORD,
    EMAIL_SENDER_ID,
    EMAIL_ENCRYPTION_TYPE,
    EMAIL_RECEIVER_ID,
    EMAIL_TEST_BTN,
    MAX_EMAIL_STG_ELEMETS
}EMAIL_STG_ELELIST_e;

// cnfg field no According to CMS comm. module
typedef enum
{
    FIELD_ENABLE,
    FIELD_MAIL_SERVER,
    FIELD_PORT,
    FIELD_USERNAME,
    FIELD_PASSWORD,
    FIELD_SENDER_ID,
    FIELD_ENCRYPTION_TYPE,
    MAX_FIELD_NO
}CNFG_FIELD_NO_e;

static const QString labelStr[MAX_EMAIL_STG_ELEMETS] =
{
    "Enable Email Service",
    "Mail Server",
    "Port",
    "Username",
    "Password",
    "Sender's Email ID",
    "Encryption",
    "Receiver's Email ID",
    "Test"
};

static const QStringList encTypeOptionStrList = QStringList() << "None" << "SSL" << "TLS";

EmailClient::EmailClient(QString devName,QWidget *parent) : ConfigPageControl(devName, parent, MAX_EMAIL_STG_ELEMETS)
{
    createDefaultComponent();
    EmailClient::getConfig();
}

EmailClient::~EmailClient()
{
    disconnect(enableEmailOpt,
               SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
               this,
               SLOT(slotEnableButtonClicked(OPTION_STATE_TYPE_e,int)));
    disconnect(enableEmailOpt,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete enableEmailOpt;

    disconnect(passwordTextbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete passwordTextbox;
    delete passwordParam;

    disconnect(textboxes[EMAIL_TEXTBOX_RECEIVER_ID],
               SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
               this,
               SLOT(slotTextboxLoadInfopage(int,INFO_MSG_TYPE_e)));
    disconnect(textboxes[EMAIL_TEXTBOX_SENDER_ID],
               SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
               this,
               SLOT(slotTextboxLoadInfopage(int,INFO_MSG_TYPE_e)));

    for(quint8 index = EMAIL_TEXTBOX_SERVER; index <= EMAIL_TEXTBOX_RECEIVER_ID; index++)
    {
        disconnect(textboxes[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        delete textboxes[index];
        delete textboxParams[index];
    }

    disconnect(encryptionType,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete encryptionType;

    delete testMailHeading;
    disconnect(testButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotTestBtnClick(int)));
    disconnect(testButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete testButton;
}

void EmailClient::createDefaultComponent()
{
    textboxParams[EMAIL_TEXTBOX_SERVER] = new TextboxParam();
    textboxParams[EMAIL_TEXTBOX_SERVER]->labelStr = labelStr[EMAIL_MAIL_SERVER];
    textboxParams[EMAIL_TEXTBOX_SERVER]->maxChar = 40;
    textboxParams[EMAIL_TEXTBOX_SERVER]->validation = QRegExp(QString("[^\\ ]"));

    textboxParams[EMAIL_TEXTBOX_PORT] = new TextboxParam();
    textboxParams[EMAIL_TEXTBOX_PORT]->labelStr = labelStr[EMAIL_PORT];
    textboxParams[EMAIL_TEXTBOX_PORT]->suffixStr = "(1 - 65535)";
    textboxParams[EMAIL_TEXTBOX_PORT]->isNumEntry = true;
    textboxParams[EMAIL_TEXTBOX_PORT]->minNumValue = 1;
    textboxParams[EMAIL_TEXTBOX_PORT]->maxNumValue= 65535;
    textboxParams[EMAIL_TEXTBOX_PORT]->maxChar = 5;
    textboxParams[EMAIL_TEXTBOX_PORT]->validation = QRegExp(QString("[0-9]"));

    textboxParams[EMAIL_TEXTBOX_USERNAME] = new TextboxParam();
    textboxParams[EMAIL_TEXTBOX_USERNAME]->labelStr = labelStr[EMAIL_USERNAME];
    textboxParams[EMAIL_TEXTBOX_USERNAME]->maxChar = 40;

    textboxParams[EMAIL_TEXTBOX_SENDER_ID] = new TextboxParam();
    textboxParams[EMAIL_TEXTBOX_SENDER_ID]->labelStr = labelStr[EMAIL_SENDER_ID];
    textboxParams[EMAIL_TEXTBOX_SENDER_ID]->maxChar = 40;
    textboxParams[EMAIL_TEXTBOX_SENDER_ID]->isEmailAddrType = true;

    textboxParams[EMAIL_TEXTBOX_RECEIVER_ID] = new TextboxParam();
    textboxParams[EMAIL_TEXTBOX_RECEIVER_ID]->labelStr = labelStr[EMAIL_RECEIVER_ID];
    textboxParams[EMAIL_TEXTBOX_RECEIVER_ID]->maxChar = 40;
    textboxParams[EMAIL_TEXTBOX_RECEIVER_ID]->isEmailAddrType = true;

    passwordParam = new TextboxParam();
    passwordParam->labelStr = labelStr[EMAIL_PASSWORD];
    passwordParam->maxChar = 24;

    enableEmailOpt = new OptionSelectButton(FIRST_ELE_XOFFSET,
                                            FIRST_ELE_YOFFSET,
                                            BGTILE_MEDIUM_SIZE_WIDTH,
                                            BGTILE_HEIGHT + SCALE_HEIGHT(10),
                                            CHECK_BUTTON_INDEX,
                                            labelStr[EMAIL_ENABLE],
                                            this,
                                            UP_LAYER,
                                            SCALE_WIDTH(15),
                                            MX_OPTION_TEXT_TYPE_SUFFIX,
                                            NORMAL_FONT_SIZE,
                                            EMAIL_ENABLE, true, NORMAL_FONT_COLOR, true);
    m_elementList[EMAIL_ENABLE] = enableEmailOpt;
    connect(enableEmailOpt,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(enableEmailOpt,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotEnableButtonClicked(OPTION_STATE_TYPE_e,int)));

    for(quint8 index = EMAIL_TEXTBOX_SERVER; index <= EMAIL_TEXTBOX_USERNAME; index++)
    {
        textboxes[index] = new TextBox(enableEmailOpt->x(),
                                       enableEmailOpt->y()+ enableEmailOpt->height() +
                                       (index * BGTILE_HEIGHT),
                                       BGTILE_MEDIUM_SIZE_WIDTH,
                                       BGTILE_HEIGHT,
                                       (EMAIL_MAIL_SERVER + index),
                                       (index == EMAIL_TEXTBOX_PORT)?TEXTBOX_SMALL:(TEXTBOX_ULTRALARGE),
                                       this, textboxParams[index], MIDDLE_TABLE_LAYER, true, false, false,
                                       LEFT_MARGIN_FROM_CENTER);
        m_elementList[EMAIL_MAIL_SERVER + index] = textboxes[index];
    }

    passwordTextbox = new PasswordTextbox(textboxes[EMAIL_TEXTBOX_USERNAME]->x(),
                                          textboxes[EMAIL_TEXTBOX_USERNAME]->y() + textboxes[EMAIL_TEXTBOX_USERNAME]->height(),
                                          BGTILE_MEDIUM_SIZE_WIDTH,
                                          BGTILE_HEIGHT,
                                          EMAIL_PASSWORD, TEXTBOX_ULTRALARGE,
                                          this, passwordParam, MIDDLE_TABLE_LAYER, 
										  true, LEFT_MARGIN_FROM_CENTER);
    m_elementList[EMAIL_PASSWORD] = passwordTextbox;
    connect(passwordTextbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    textboxes[EMAIL_TEXTBOX_SENDER_ID] = new TextBox(passwordTextbox->x(),
                                                     passwordTextbox->y() + passwordTextbox->height(),
                                                     BGTILE_MEDIUM_SIZE_WIDTH,
                                                     BGTILE_HEIGHT,
                                                     EMAIL_SENDER_ID, TEXTBOX_ULTRALARGE,
                                                     this, textboxParams[EMAIL_TEXTBOX_SENDER_ID],
                                                     MIDDLE_TABLE_LAYER, true, false, false,
                                                     LEFT_MARGIN_FROM_CENTER);
    m_elementList[EMAIL_SENDER_ID] = textboxes[EMAIL_TEXTBOX_SENDER_ID];

    QMap<quint8, QString> encTypeOptionStrMapList;
    for(quint8 index = 0; index < encTypeOptionStrList.length(); index++)
    {
        encTypeOptionStrMapList.insert (index, encTypeOptionStrList.at(index));
    }

    encryptionType = new DropDown(textboxes[EMAIL_TEXTBOX_SENDER_ID]->x(),
                                  textboxes[EMAIL_TEXTBOX_SENDER_ID]->y() + textboxes[EMAIL_TEXTBOX_SENDER_ID]->height(),
                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                  BGTILE_HEIGHT,
                                  EMAIL_ENCRYPTION_TYPE,
                                  DROPDOWNBOX_SIZE_114,
                                  labelStr[EMAIL_ENCRYPTION_TYPE],
                                  encTypeOptionStrMapList,
                                  this, "", true, 0, BOTTOM_TABLE_LAYER, true, 8, false, false, 5, LEFT_MARGIN_FROM_CENTER);
    m_elementList[EMAIL_ENCRYPTION_TYPE] = encryptionType;
    connect(encryptionType,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    testMailHeading = new ElementHeading(encryptionType->x(),
                                         encryptionType->y() + encryptionType->height() + SCALE_HEIGHT(6),
                                         BGTILE_MEDIUM_SIZE_WIDTH,
                                         BGTILE_HEIGHT, "Test Mail",
                                         TOP_LAYER, this, true, SCALE_WIDTH(25), NORMAL_FONT_SIZE, true);

    textboxes[EMAIL_TEXTBOX_RECEIVER_ID] = new TextBox(testMailHeading->x(),
                                                       testMailHeading->y() + testMailHeading->height(),
                                                       BGTILE_MEDIUM_SIZE_WIDTH, BGTILE_HEIGHT,
                                                       EMAIL_RECEIVER_ID,
                                                       TEXTBOX_ULTRALARGE, this,
                                                       textboxParams[EMAIL_TEXTBOX_RECEIVER_ID],
                                                       MIDDLE_TABLE_LAYER, true, false, false,
                                                       LEFT_MARGIN_FROM_CENTER);
    m_elementList[EMAIL_RECEIVER_ID] = textboxes[EMAIL_TEXTBOX_RECEIVER_ID];

    for(quint8 index = EMAIL_TEXTBOX_SERVER; index <= EMAIL_TEXTBOX_RECEIVER_ID; index++)
    {
        connect(textboxes[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    }

    connect(textboxes[EMAIL_TEXTBOX_SENDER_ID],
            SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
            this,
            SLOT(slotTextboxLoadInfopage(int,INFO_MSG_TYPE_e)));
    connect(textboxes[EMAIL_TEXTBOX_RECEIVER_ID],
            SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
            this,
            SLOT(slotTextboxLoadInfopage(int,INFO_MSG_TYPE_e)));

    testButton = new ControlButton(EMAIL_BUTTON_INDEX,
                                   textboxes[EMAIL_TEXTBOX_RECEIVER_ID]->x(),
                                   textboxes[EMAIL_TEXTBOX_RECEIVER_ID]->y() + textboxes[EMAIL_TEXTBOX_RECEIVER_ID]->height(),
                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                   BGTILE_HEIGHT,
                                   this, DOWN_LAYER, BGTILE_MEDIUM_SIZE_WIDTH - SCALE_WIDTH(90),
                                   labelStr[EMAIL_TEST_BTN], true,
                                   EMAIL_TEST_BTN);
    m_elementList[EMAIL_TEST_BTN] = testButton;
    connect(testButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(testButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotTestBtnClick(int)));
}

void EmailClient::getConfig()
{
    //create the payload for Get Cnfg
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                             EMAIL_TABLE_INDEX,
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

void EmailClient::defaultConfig()
{
    //create the payload for Get Cnfg
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_DEF_CFG,
                                                             EMAIL_TABLE_INDEX,
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

void EmailClient::saveConfig()
{
    if (enableEmailOpt->getCurrentState() == ON_STATE)
    {
        if((!textboxes[EMAIL_TEXTBOX_SENDER_ID]->doneKeyValidation()))
        {
            return;
        }
    }

    if (checkDataIsSufficient() == false)
    {
        return;
    }

    payloadLib->setCnfgArrayAtIndex(FIELD_ENABLE, enableEmailOpt->getCurrentState());
    payloadLib->setCnfgArrayAtIndex(FIELD_MAIL_SERVER, textboxes[EMAIL_TEXTBOX_SERVER]->getInputText());
    payloadLib->setCnfgArrayAtIndex(FIELD_PORT, textboxes[EMAIL_TEXTBOX_PORT]->getInputText());
    payloadLib->setCnfgArrayAtIndex(FIELD_USERNAME, textboxes[EMAIL_TEXTBOX_USERNAME]->getInputText());
    payloadLib->setCnfgArrayAtIndex(FIELD_PASSWORD, passwordTextbox->getInputText());
    payloadLib->setCnfgArrayAtIndex(FIELD_SENDER_ID, textboxes[EMAIL_TEXTBOX_SENDER_ID]->getInputText());
    payloadLib->setCnfgArrayAtIndex(FIELD_ENCRYPTION_TYPE, encryptionType->getIndexofCurrElement());

    /* Create the payload for Get Cnfg */
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_SET_CFG,
                                                             EMAIL_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_TO_INDEX,
                                                             CNFG_FRM_FIELD,
                                                             MAX_FIELD_NO,
                                                             MAX_FIELD_NO);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CFG;
    param->payload = payloadString;
    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void EmailClient::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if (deviceName != currDevName)
    {
        return;
    }

    if (param->deviceStatus != CMD_SUCCESS)
    {
        processBar->unloadProcessBar();
        infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
        update();
        return;
    }

    switch(param->msgType)
    {
        case MSG_GET_CFG:
        {
            processBar->unloadProcessBar();
            payloadLib->parsePayload(param->msgType, param->payload);
            if (payloadLib->getcnfgTableIndex() != EMAIL_TABLE_INDEX)
            {
                break;
            }

            OPTION_STATE_TYPE_e state = (OPTION_STATE_TYPE_e)(payloadLib->getCnfgArrayAtIndex(FIELD_ENABLE).toUInt());
            enableEmailOpt->changeState(state);
            for(quint8 index = EMAIL_TEXTBOX_SERVER; index <= EMAIL_TEXTBOX_USERNAME; index++)
            {
                textboxes[index]->setInputText (payloadLib->getCnfgArrayAtIndex (FIELD_MAIL_SERVER + index).toString());
            }

            passwordTextbox->setInputText (payloadLib->getCnfgArrayAtIndex(FIELD_PASSWORD).toString());
            textboxes[EMAIL_TEXTBOX_SENDER_ID]->setInputText(payloadLib->getCnfgArrayAtIndex(FIELD_SENDER_ID).toString());
            encryptionType->setIndexofCurrElement(payloadLib->getCnfgArrayAtIndex(FIELD_ENCRYPTION_TYPE).toUInt());
            slotEnableButtonClicked(state, 0);
        }
        break;

        case MSG_SET_CFG:
        {
            // unload processing icon
            processBar->unloadProcessBar();

            // load info page with msg
            MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
        }
        break;

        case MSG_DEF_CFG:
        {
            textboxes[EMAIL_TEXTBOX_RECEIVER_ID]->setInputText("");
            getConfig();
        }
        break;

        case MSG_SET_CMD:
        {
            if (param->cmdType != TST_MAIL)
            {
                break;
            }

            // unload processing icon
            processBar->unloadProcessBar();

            // load info page with msg
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(EMAIL_CLIENT_MAIL_SENT_SUCCESS));
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

bool EmailClient::checkDataIsSufficient()
{
    if(enableEmailOpt->getCurrentState() == OFF_STATE)
    {
        return true;
    }

    if(textboxes[EMAIL_TEXTBOX_SERVER]->getInputText() == "")
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(EMAIL_CLIENT_ENT_MAIL_SERVER));
        return false;
    }

    if(textboxes[EMAIL_TEXTBOX_PORT]->getInputText() == "")
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(EMAIL_CLIENT_ENT_PORT));
        return false;
    }

    if(textboxes[EMAIL_TEXTBOX_USERNAME]->getInputText() == "")
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_USER_NM));
        return false;
    }

    if(passwordTextbox->getInputText() == "")
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_PASSWORD));
        return false;
    }

    if(textboxes[EMAIL_TEXTBOX_SENDER_ID]->getInputText() == "")
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(EMAIL_CLIENT_SENDER_EMAIL_ID));
        return false;
    }

    return true;
}

void EmailClient::slotEnableButtonClicked(OPTION_STATE_TYPE_e state, int)
{
    for(quint8 index = EMAIL_TEXTBOX_SERVER; index <= EMAIL_TEXTBOX_RECEIVER_ID; index++)
    {
        textboxes[index]->setIsEnabled(state);
    }

    passwordTextbox->setIsEnabled(state);
    encryptionType->setIsEnabled(state);
    testButton->setIsEnabled(state);
}

void EmailClient::slotTestBtnClick(int)
{
    if (checkDataIsSufficient() == false)
    {
        return;
    }

    if (textboxes[EMAIL_TEXTBOX_RECEIVER_ID]->getInputText() == "")
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(EMAIL_CLIENT_RECEIVER_EMAIL_ID));
        return;
    }

    if ((!textboxes[EMAIL_TEXTBOX_SENDER_ID]->doneKeyValidation()) || (!textboxes[EMAIL_TEXTBOX_RECEIVER_ID]->doneKeyValidation()))
    {
        return;
    }

    payloadLib->setCnfgArrayAtIndex(0, textboxes[EMAIL_TEXTBOX_RECEIVER_ID]->getInputText());
    payloadLib->setCnfgArrayAtIndex(1, textboxes[EMAIL_TEXTBOX_SERVER]->getInputText());
    payloadLib->setCnfgArrayAtIndex(2, textboxes[EMAIL_TEXTBOX_PORT]->getInputText());
    payloadLib->setCnfgArrayAtIndex(3, textboxes[EMAIL_TEXTBOX_USERNAME]->getInputText());
    payloadLib->setCnfgArrayAtIndex(4, passwordTextbox->getInputText());
    payloadLib->setCnfgArrayAtIndex(5, textboxes[EMAIL_TEXTBOX_SENDER_ID]->getInputText());
    payloadLib->setCnfgArrayAtIndex(6, encryptionType->getIndexofCurrElement());

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = TST_MAIL;
    param->payload = payloadLib->createDevCmdPayload(7);
    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void EmailClient::slotTextboxLoadInfopage(int, INFO_MSG_TYPE_e msgType)
{
    if(msgType == INFO_MSG_STRAT_CHAR)
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_FIRST_ALPH));
    }
    else if(msgType == INFO_MSG_ERROR)
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_VAILD_EMAIL_ADD));
    }
}
