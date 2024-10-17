#include "SmsSetting.h"
#include "ValidationMessage.h"

#define CNFG_TO_INDEX               1

#define FIRST_ELE_XOFFSET           SCALE_WIDTH(259)
#define FIRST_ELE_YOFFSET           SCALE_HEIGHT(66) //(558 - 426)/2
#define LEFT_MARGIN_FROM_CENTER     SCALE_WIDTH(60)

// List of control
typedef enum
{
    SMS_SERVICE_PROVIDER,
    SMS_USERNAME,
    SMS_PASSWORD,
    SMS_SENDER_ID,
    SMS_FLASH_MSG,
    SMS_CHECK_BAL,
    SMS_MOB_NUM,
    SMS_TEST_BTN,

    MAX_SMS_STG_ELEMETS
}SMS_STG_ELELIST_e;

// cnfg field no According to CMS comm. module
typedef enum
{
    FIELD_MODE,
    FIELD_SERVICE_PROVIDER,
    FIELD_USERNAME,
    FIELD_PASSWORD,
    FIELD_SENDER_ID,
    FIELD_FLASH_MSG,

    MAX_FIELD_NO
}CNFG_FIELD_NO_e;

static const QString labelStr[MAX_SMS_STG_ELEMETS] =
{
    "Service Provider",
    "Username",
    "Password",
    "Sender ID",
    "Flash Message",
    "Check Balance",
    "Mobile Number",
    "Test"
};

static const QStringList serProviderList = QStringList()
        << "SMS Gateway Centre"
        << "SMS Lane"
        << "Business SMS"
        << "Bulk SMS";

SmsSetting::SmsSetting(QString devName, QWidget *parent)
    :ConfigPageControl(devName, parent, MAX_SMS_STG_ELEMETS),
      currProvider(MAX_SMS_SERV_PROVIDER)
{
    createDefaultComponent();
    SmsSetting::getConfig();
}

SmsSetting::~SmsSetting()
{
    delete httpModeHeading;

    disconnect (serProviderDropDownBox,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotSpinboxValueChanged(QString,quint32)));
    disconnect(serProviderDropDownBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete serProviderDropDownBox;

    disconnect(usernameTextbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete usernameTextbox;
    delete usernameParam;

    disconnect(passwordTextbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete passwordTextbox;
    delete passwordParam;

    disconnect(senderIdTextbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete senderIdTextbox;
    delete senderIdParam;

    disconnect(flashMsgOpt,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete flashMsgOpt;

    disconnect (checkBalBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotControlBtnClick(int)));
    disconnect(checkBalBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete checkBalBtn;

    delete testSmsHeading;

    disconnect (mobNumTextbox,
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotTextboxLoadInfopage(int,INFO_MSG_TYPE_e)));
    disconnect(mobNumTextbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete mobNumTextbox;
    delete mobNumParam;

    disconnect (testSmsBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotControlBtnClick(int)));
    disconnect(testSmsBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete testSmsBtn;
}

void SmsSetting::createDefaultComponent ()
{
    httpModeHeading = new ElementHeading(FIRST_ELE_XOFFSET,
                                         FIRST_ELE_YOFFSET,
                                         BGTILE_MEDIUM_SIZE_WIDTH,
                                         BGTILE_HEIGHT,
                                         "HTTP Mode", TOP_LAYER,
                                         this, false, SCALE_WIDTH(15), NORMAL_FONT_SIZE, true);

    QMap<quint8, QString>  serProviderMapList;

    for(quint8 index = 0; index <  serProviderList.length (); index++)
    {
        serProviderMapList.insert (index,serProviderList.at (index));
    }

    serProviderDropDownBox = new DropDown(httpModeHeading->x (),
                                          httpModeHeading->y () + httpModeHeading->height (),
                                          BGTILE_MEDIUM_SIZE_WIDTH,
                                          BGTILE_HEIGHT,
                                          SMS_SERVICE_PROVIDER,
                                          DROPDOWNBOX_SIZE_225,
                                          labelStr[SMS_SERVICE_PROVIDER],
                                          serProviderMapList,
                                          this, "",
                                          true, 0, MIDDLE_TABLE_LAYER,
                                          true, 8, false, false, 5,
                                          LEFT_MARGIN_FROM_CENTER);
    m_elementList[SMS_SERVICE_PROVIDER] = serProviderDropDownBox;

    connect(serProviderDropDownBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (serProviderDropDownBox,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotSpinboxValueChanged(QString,quint32)));

    usernameParam = new TextboxParam();
    usernameParam->labelStr = labelStr[SMS_USERNAME];
    usernameParam->maxChar = 100;

    usernameTextbox = new TextBox(serProviderDropDownBox->x (),
                                  serProviderDropDownBox->y () + serProviderDropDownBox->height (),
                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                  BGTILE_HEIGHT,
                                  SMS_USERNAME, TEXTBOX_ULTRALARGE,
                                  this, usernameParam,
                                  MIDDLE_TABLE_LAYER, true, false,
                                  false, LEFT_MARGIN_FROM_CENTER);
    m_elementList[SMS_USERNAME] = usernameTextbox;
    connect(usernameTextbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    passwordParam = new TextboxParam();
    passwordParam->labelStr = labelStr[SMS_PASSWORD];
    passwordParam->maxChar = 100;

    passwordTextbox = new PasswordTextbox(usernameTextbox->x (),
                                          usernameTextbox->y () + usernameTextbox->height (),
                                          BGTILE_MEDIUM_SIZE_WIDTH,
                                          BGTILE_HEIGHT,
                                          SMS_PASSWORD, TEXTBOX_ULTRALARGE,
                                          this, passwordParam,
                                          MIDDLE_TABLE_LAYER, true,
                                          LEFT_MARGIN_FROM_CENTER);
    m_elementList[SMS_PASSWORD] = passwordTextbox;
    connect(passwordTextbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    senderIdParam = new TextboxParam();
    senderIdParam->labelStr = labelStr[SMS_SENDER_ID];
    senderIdParam->maxChar = 25;
    senderIdParam->isTotalBlankStrAllow = true;
    senderIdParam->validation = QRegExp(QString("[a-zA-Z0-9]"));

    senderIdTextbox = new TextBox(passwordTextbox->x (),
                                  passwordTextbox->y () + passwordTextbox->height (),
                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                  BGTILE_HEIGHT,
                                  SMS_SENDER_ID, TEXTBOX_ULTRALARGE,
                                  this, senderIdParam,
                                  MIDDLE_TABLE_LAYER,
                                  true, false, false,
                                  LEFT_MARGIN_FROM_CENTER);
    m_elementList[SMS_SENDER_ID] = senderIdTextbox;
    connect(senderIdTextbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    flashMsgOpt = new OptionSelectButton(senderIdTextbox->x (),
                                         senderIdTextbox->y ()+ senderIdTextbox->height (),
                                         BGTILE_MEDIUM_SIZE_WIDTH,
                                         BGTILE_HEIGHT,
                                         CHECK_BUTTON_INDEX,
                                         this, MIDDLE_TABLE_LAYER,
                                         labelStr[SMS_FLASH_MSG], "",
                                         -1, SMS_FLASH_MSG, true, -1,
                                         SUFFIX_FONT_COLOR, false,
                                         LEFT_MARGIN_FROM_CENTER);
    m_elementList[SMS_FLASH_MSG] = flashMsgOpt;
    connect(flashMsgOpt,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    checkBalBtn = new ControlButton(CHECK_BAL_BUTTON_INDEX,
                                    flashMsgOpt->x (),
                                    flashMsgOpt->y () + flashMsgOpt->height (),
                                    BGTILE_MEDIUM_SIZE_WIDTH,
                                    BGTILE_HEIGHT,
                                    this, DOWN_LAYER,
                                    SCALE_HEIGHT(310), labelStr[SMS_CHECK_BAL],
                                    true, SMS_CHECK_BAL);
    m_elementList[SMS_CHECK_BAL] = checkBalBtn;
    connect(checkBalBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (checkBalBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotControlBtnClick(int)));

    testSmsHeading = new ElementHeading(checkBalBtn->x (),
                                        checkBalBtn->y () + checkBalBtn->height () + SCALE_WIDTH(6),
                                        BGTILE_MEDIUM_SIZE_WIDTH,
                                        BGTILE_HEIGHT,
                                        "Test SMS", TOP_LAYER,
                                        this, false, SCALE_WIDTH(15), NORMAL_FONT_SIZE, true);

    mobNumParam = new TextboxParam();
    mobNumParam->labelStr = labelStr[SMS_MOB_NUM];
    mobNumParam->minChar = 12;
    mobNumParam->maxChar = 14;
    mobNumParam->validation = QRegExp(QString("[0-9]"));
    mobNumParam->textStr = "91";

    mobNumTextbox = new TextBox(testSmsHeading->x (),
                                testSmsHeading->y ()+ testSmsHeading->height (),
                                BGTILE_MEDIUM_SIZE_WIDTH,
                                BGTILE_HEIGHT,
                                SMS_MOB_NUM, TEXTBOX_LARGE,
                                this, mobNumParam,
                                MIDDLE_TABLE_LAYER,
                                true, false, false,
                                LEFT_MARGIN_FROM_CENTER);
    m_elementList[SMS_MOB_NUM] = mobNumTextbox;
    connect(mobNumTextbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (mobNumTextbox,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotTextboxLoadInfopage(int,INFO_MSG_TYPE_e)));

    testSmsBtn = new ControlButton(TEST_SMS_BUTTON_INDEX,
                                   mobNumTextbox->x (),
                                   mobNumTextbox->y () + mobNumTextbox->height (),
                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                   BGTILE_HEIGHT,
                                   this, DOWN_LAYER,
                                   SCALE_WIDTH(400), labelStr[SMS_TEST_BTN],
                                   true, SMS_TEST_BTN);
    m_elementList[SMS_TEST_BTN] = testSmsBtn;
    connect(testSmsBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (testSmsBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotControlBtnClick(int)));
}

void SmsSetting::getConfig()
{
    //create the payload for Get Cnfg
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                             SMS_TABLE_INDEX,
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

void SmsSetting::defaultConfig()
{
    //create the payload for Get Cnfg
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_DEF_CFG,
                                                             SMS_TABLE_INDEX,
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

void SmsSetting::saveConfig()
{
    if ( IS_VALID_OBJ(mobNumTextbox)
            && (currProvider != SMS_LANE)
            && (!mobNumTextbox->doneKeyValidation()))
    {
        return;
    }

    if(checkDataIsSufficient () == false)
    {
        return;
    }
    else
    {
        // THIS ELE is SMS MODE, which is only HTTP
        payloadLib->setCnfgArrayAtIndex (FIELD_MODE, 0);

        payloadLib->setCnfgArrayAtIndex (FIELD_SERVICE_PROVIDER,
                                         serProviderDropDownBox->getIndexofCurrElement ());
        payloadLib->setCnfgArrayAtIndex (FIELD_USERNAME,
                                         usernameTextbox->getInputText ());
        payloadLib->setCnfgArrayAtIndex (FIELD_PASSWORD,
                                         passwordTextbox->getInputText ());
        payloadLib->setCnfgArrayAtIndex (FIELD_SENDER_ID,
                                         senderIdTextbox->getInputText ());
        payloadLib->setCnfgArrayAtIndex (FIELD_FLASH_MSG,
                                         flashMsgOpt->getCurrentState ());
        //create the payload for Get Cnfg
        QString payloadString = payloadLib->createDevCnfgPayload(MSG_SET_CFG,
                                                                 SMS_TABLE_INDEX,
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
}

void SmsSetting::processDeviceResponse (DevCommParam *param, QString deviceName)
{
    processBar->unloadProcessBar();
    if(deviceName == currDevName)
    {
        switch(param->deviceStatus)
        {
        case CMD_SUCCESS:
            switch(param->msgType)
            {
            case MSG_GET_CFG:
                payloadLib->parsePayload(param->msgType, param->payload);
                if(payloadLib->getcnfgTableIndex () == SMS_TABLE_INDEX)
                {
                    currProvider = (SMS_SER_PROVIDER_e)
                            (payloadLib->getCnfgArrayAtIndex (FIELD_SERVICE_PROVIDER).toUInt ());

                    serProviderDropDownBox->setIndexofCurrElement (currProvider);
                    usernameTextbox->setInputText (payloadLib->getCnfgArrayAtIndex (FIELD_USERNAME).toString ());
                    passwordTextbox->setInputText (payloadLib->getCnfgArrayAtIndex (FIELD_PASSWORD).toString ());
                    senderIdTextbox->setInputText (payloadLib->getCnfgArrayAtIndex (FIELD_SENDER_ID).toString ());
                    flashMsgOpt->changeState ((OPTION_STATE_TYPE_e)
                                              (payloadLib->getCnfgArrayAtIndex (FIELD_FLASH_MSG).toUInt ()));
                    slotSpinboxValueChanged(serProviderList.at (currProvider), 0);
                }
                break;

            case MSG_SET_CFG:
                //load info page with msg
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
                break;

            case MSG_DEF_CFG:
                getConfig();
                break;

            case MSG_SET_CMD:
                switch(param->cmdType)
                {
                case TST_SMS:
                    //load info page with msg
                    infoPage->loadInfoPage (ValidationMessage::getValidationMessage(SMS_SETT_MESSAGE_SENT_SUCCESS));
                    break;

                case CHK_BAL:
                    payloadLib->parseDevCmdReply (true, param->payload);
                    //load info page with msg
                    infoPage->loadInfoPage(Multilang("Available Balance is") + QString(" ") + payloadLib->getCnfgArrayAtIndex (0).toString ());
                    break;

                default:
                    break;
                }
                break;

            default:
                break;
            }
            break;

        default:
            infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
            break;
        }
        update();
    }
}

bool SmsSetting::checkDataIsSufficient ()
{
    // Below process is under HTTP sms mode only,
    // we have only HTTP mode as of now
    if(usernameTextbox->getInputText () == "")
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_USER_NM));
        return false;
    }
    else if(passwordTextbox->getInputText () == "")
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_PASSWORD));
        return false;
    }
    else if((serProviderDropDownBox->getIndexofCurrElement () == SMS_GATEWAY_CENTRE)
             && (senderIdTextbox->getInputText () == ""))
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(SMS_SETT_ENT_SENDER_ID));
        return false;
    }
    return true;
}

void SmsSetting::slotSpinboxValueChanged(QString str,quint32)
{
    currProvider = (SMS_SER_PROVIDER_e)serProviderList.indexOf (str);
    switch(currProvider)
    {
    case SMS_GATEWAY_CENTRE:
        senderIdTextbox->setIsEnabled (true);
        flashMsgOpt->setIsEnabled (false);
        checkBalBtn->setIsEnabled (true);
        testSmsBtn->setIsEnabled (true);
        mobNumTextbox->setIsEnabled (true);
        senderIdParam->isTotalBlankStrAllow = false;
        break;

    case SMS_LANE:
        senderIdTextbox->setIsEnabled (true);
        flashMsgOpt->setIsEnabled (true);
        checkBalBtn->setIsEnabled (true);
        testSmsBtn->setIsEnabled (false);
        mobNumTextbox->setIsEnabled (false);
        senderIdParam->isTotalBlankStrAllow = true;
        break;

    case BUSINESS_SMS:
        senderIdTextbox->setIsEnabled (false);
        flashMsgOpt->setIsEnabled (true);
        checkBalBtn->setIsEnabled (true);
        testSmsBtn->setIsEnabled (true);
        mobNumTextbox->setIsEnabled (true);
        senderIdParam->isTotalBlankStrAllow = true;
        break;

    case BULK_SMS:
        senderIdTextbox->setIsEnabled (true);
        flashMsgOpt->setIsEnabled (true);
        checkBalBtn->setIsEnabled (true);
        testSmsBtn->setIsEnabled (true);
        mobNumTextbox->setIsEnabled (true);
        senderIdParam->isTotalBlankStrAllow = true;
        break;

    default:
        break;
    }
}

void SmsSetting::slotControlBtnClick(int index)
{
    if(index == SMS_CHECK_BAL)
    {
        if(checkDataIsSufficient () == false)
        {
            return;
        }
        else
        {
            // FIRST ELE is SMS MODE, which is only HTTP
            payloadLib->setCnfgArrayAtIndex (0,0);

            payloadLib->setCnfgArrayAtIndex (1,
                                             serProviderDropDownBox->getIndexofCurrElement ());
            payloadLib->setCnfgArrayAtIndex (2,
                                             usernameTextbox->getInputText ());
            payloadLib->setCnfgArrayAtIndex (3,
                                             passwordTextbox->getInputText ());
            payloadLib->setCnfgArrayAtIndex (4,
                                             senderIdTextbox->getInputText ());
            payloadLib->setCnfgArrayAtIndex (5,
                                             flashMsgOpt->getCurrentState ());

            QString payloadString = payloadLib->createDevCmdPayload(6);

            DevCommParam* param = new DevCommParam();
            param->msgType = MSG_SET_CMD;
            param->cmdType = CHK_BAL;
            param->payload = payloadString;

            processBar->loadProcessBar();
            applController->processActivity(currDevName, DEVICE_COMM, param);
        }
    }
    else if(index == SMS_TEST_BTN)
    {
        if(checkDataIsSufficient () == false)
        {
            return;
        }
        else if(mobNumTextbox->getInputText () == "91")
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(SMS_SETT_ENT_MOBILE_NO));
            return;
        }
        else
        {
            payloadLib->setCnfgArrayAtIndex (0,
                                             mobNumTextbox->getInputText ());
            // THIS ELE is SMS MODE, which is only HTTP
            payloadLib->setCnfgArrayAtIndex (1, 0);

            payloadLib->setCnfgArrayAtIndex (2,
                                             serProviderDropDownBox->getIndexofCurrElement ());
            payloadLib->setCnfgArrayAtIndex (3,
                                             usernameTextbox->getInputText ());
            payloadLib->setCnfgArrayAtIndex (4,
                                             passwordTextbox->getInputText ());
            payloadLib->setCnfgArrayAtIndex (5,
                                             senderIdTextbox->getInputText ());
            payloadLib->setCnfgArrayAtIndex (6,
                                             flashMsgOpt->getCurrentState ());

            QString payloadString = payloadLib->createDevCmdPayload(7);

            DevCommParam* param = new DevCommParam();
            param->msgType = MSG_SET_CMD;
            param->cmdType = TST_SMS;
            param->payload = payloadString;

            processBar->loadProcessBar ();
            applController->processActivity(currDevName, DEVICE_COMM, param);
        }
    }
}

void SmsSetting::slotTextboxLoadInfopage(int, INFO_MSG_TYPE_e msgType)
{
    if(msgType == INFO_MSG_ERROR)
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(SMS_SETT_ENT_VALID_MOBILE_NO));
    }
}
