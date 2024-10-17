#include "LogIn.h"
#include <QKeyEvent>
#include "Controls/MessageBanner.h"
#include "ApplicationMode.h"
#include "ValidationMessage.h"

#define LOG_IN_BOX_HEIGHT        SCALE_HEIGHT(277)
#define LOG_IN_BOX_WIDTH         SCALE_WIDTH(526)
#define LOG_SETTING_FROM_INDEX   1
#define LOG_SETTING_FROM_FIELD   1
#define LOG_SETTING_TO_FIELD     11

typedef enum
{
    LOG_IN_USERNAME_STRING,
    LOG_IN_PASSWORD_STRING,
    LOG_IN_CURRENT_PASSWORD_STRING,
    LOG_IN_CONFIRM_PASSWORD_STRING,
    LOG_IN_REMEMBER_PASSWORD_STRING,
    LOG_IN_CREATE_PASSWORD_STRING,
    LOG_IN_CHANGE_PASSWORD_STRING,
    LOG_IN_NEW_PASSWORD_STRING,
    LOG_IN_FORGOT_PASSWORD_STRING,
    LOG_IN_LOG_BTN_STRING,
    MAX_LOG_IN_STRING
}LOG_IN_STRING_e;

static const QString userLoginStrings[MAX_LOG_IN_STRING] =
{
    "Username ",
    "Password ",
    "Current Password ",
    "Confirm Password ",
    "Remember Password ",
    "Create Password ",
    "Change Password ",
    "New Password ",
    "Forgot Password?",
    "Login ",
};

LogIn :: LogIn(STATE_TYPE_e imgType, QWidget *parent) :
    BackGround((ApplController::getXPosOfScreen() + ((ApplController::getWidthOfScreen() - LOG_IN_BOX_WIDTH) / 2)) ,
               (ApplController::getYPosOfScreen() + ((ApplController::getHeightOfScreen() - LOG_IN_BOX_HEIGHT) / 2)),
               LOG_IN_BOX_WIDTH,
               LOG_IN_BOX_HEIGHT,
               BACKGROUND_TYPE_4,
               LOG_BUTTON,
               parent,true)
{
    INIT_OBJ(m_passwordRecovery);
    INIT_OBJ(m_forgotPassword);

    this->hide ();

    m_imgType = imgType;
    m_isLogInRequestSend = false;

    m_elementList[LOG_IN_CLS_BTN] = m_mainCloseButton;
    m_mainCloseButton->setIndexInPage(LOG_IN_CLS_BTN);

    connect(m_mainCloseButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_applController= ApplController::getInstance ();
    m_payloadLib = new PayloadLib();

    quint8 devcount;
    QStringList devlist;
    m_applController->GetEnableDevList (devcount,devlist);
    m_currentDevName = devlist.at (0);

    setCurrentLoginState(LOGIN_NORMAL);
    if(m_imgType == STATE_1)
    {
        m_logInHeadings = new TextLabel((LOG_IN_BOX_WIDTH - BGTILE_SMALL_SIZE_WIDTH )/2, 15, NORMAL_FONT_SIZE, "", this);

        m_userParam = new TextboxParam();
        m_userParam->labelStr = userLoginStrings[LOG_IN_USERNAME_STRING];
        m_userParam->isCentre = true;
        m_userParam->isTotalBlankStrAllow = true;
        m_userParam->maxChar = 24;
        m_userParam->validation = QRegExp(asciiset1ValidationStringWithoutSpace);

        m_userTextBox = new TextBox((LOG_IN_BOX_WIDTH - BGTILE_SMALL_SIZE_WIDTH )/2 ,
                                    SCALE_HEIGHT(58),
                                    BGTILE_SMALL_SIZE_WIDTH,
                                    BGTILE_HEIGHT,
                                    LOG_IN_USR_TXTBOX,
                                    TEXTBOX_LARGE,
                                    this,
                                    m_userParam);
        m_elementList[LOG_IN_USR_TXTBOX] = m_userTextBox;
        connect (m_userTextBox,
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));

        m_currentPasswordParam = new TextboxParam();
        m_currentPasswordParam->labelStr = userLoginStrings[LOG_IN_CURRENT_PASSWORD_STRING];
        m_currentPasswordParam->isCentre = true;
        m_currentPasswordParam->isTotalBlankStrAllow = true;
        m_currentPasswordParam->maxChar = 16;
        m_currentPasswordParam->minChar = 4;
        m_currentPasswordParam->validation = QRegExp(asciiset1ValidationStringWithoutSpace);

        m_currentPasswordTextBox = new PasswordTextbox(m_userTextBox->x () ,
                                                       m_userTextBox->y (),
                                                       SCALE_WIDTH(410),
                                                       BGTILE_HEIGHT,
                                                       LOG_IN_PASS_TXTBOX,
                                                       TEXTBOX_LARGE,
                                                       this,
                                                       m_currentPasswordParam);
        connect (m_currentPasswordTextBox,
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
        m_currentPasswordTextBox->hide();
        m_currentPasswordTextBox->setIsEnabled(false);

        m_passwordParam = new TextboxParam();
        m_passwordParam->labelStr = userLoginStrings[LOG_IN_PASSWORD_STRING];
        m_passwordParam->isCentre = true;
        m_passwordParam->isTotalBlankStrAllow = true;
        m_passwordParam->maxChar = 16;
        m_passwordParam->minChar = 4;
        m_passwordParam->validation = QRegExp(asciiset1ValidationStringWithoutSpace);

        m_passwordTextBox = new PasswordTextbox(m_userTextBox->x () ,
                                                m_userTextBox->y () + BGTILE_HEIGHT,
                                                SCALE_WIDTH(410),
                                                BGTILE_HEIGHT,
                                                LOG_IN_PASS_TXTBOX,
                                                TEXTBOX_LARGE,
                                                this,
                                                m_passwordParam);
        m_elementList[LOG_IN_PASS_TXTBOX] = m_passwordTextBox;
        connect (m_passwordTextBox,
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));

        m_confmpasswordParam = new TextboxParam();
        m_confmpasswordParam->labelStr = userLoginStrings[LOG_IN_CONFIRM_PASSWORD_STRING];
        m_confmpasswordParam->isCentre = true;
        m_confmpasswordParam->isTotalBlankStrAllow = true;
        m_confmpasswordParam->maxChar = 16;
        m_confmpasswordParam->minChar = 4;
        m_confmpasswordParam->validation = QRegExp(asciiset1ValidationStringWithoutSpace);

        m_confmpasswordTextBox = new PasswordTextbox(m_userTextBox->x () ,
                                                     m_passwordTextBox->y () + BGTILE_HEIGHT,
                                                     SCALE_WIDTH(410),
                                                     BGTILE_HEIGHT,
                                                     LOG_IN_PASS_TXTBOX,
                                                     TEXTBOX_LARGE,
                                                     this,
                                                     m_confmpasswordParam);
        connect (m_confmpasswordTextBox,
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
        m_confmpasswordTextBox->hide();
        m_confmpasswordTextBox->setIsEnabled(false);

        m_rmbPwdChkBox = new OptionSelectButton(m_userTextBox->x () ,
                                                m_passwordTextBox->y () + BGTILE_HEIGHT,
                                                BGTILE_SMALL_SIZE_WIDTH,
                                                BGTILE_HEIGHT,
                                                CHECK_BUTTON_INDEX,
                                                this,
                                                COMMON_LAYER,
                                                userLoginStrings[LOG_IN_REMEMBER_PASSWORD_STRING],
                                                "",
                                                -1,
                                                LOG_IN_REM_PASS_CHEKBOX);
        m_elementList[LOG_IN_REM_PASS_CHEKBOX] = m_rmbPwdChkBox;
        connect (m_rmbPwdChkBox,
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));

        m_logInButton = new CnfgButton(CNFGBUTTON_EXTRALARGE,
                                       LOG_IN_BOX_WIDTH/2 ,
                                       (m_rmbPwdChkBox->y () + SCALE_HEIGHT(20)) +
                                       ( LOG_IN_BOX_HEIGHT - (m_rmbPwdChkBox->y () + BGTILE_HEIGHT))/2,
                                       userLoginStrings[LOG_IN_LOG_BTN_STRING],
                                       this,
                                       LOG_IN_LOG_BTN);
        m_elementList[LOG_IN_LOG_BTN] = m_logInButton;
        connect (m_logInButton,
                 SIGNAL(sigButtonClick(int)),
                 this,
                 SLOT(slotButtonClick(int)));

        connect (m_logInButton,
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));

        m_forgotPasswordLabel = new TextLabel(LOG_IN_BOX_WIDTH - SCALE_WIDTH(30),
                                              m_logInButton->y() + SCALE_HEIGHT(75),
                                              NORMAL_FONT_SIZE,
                                              userLoginStrings[LOG_IN_FORGOT_PASSWORD_STRING],
                                              this,
                                              HIGHLITED_FONT_COLOR,
                                              NORMAL_FONT_FAMILY,
                                              ALIGN_END_X_END_Y,
                                              0,
                                              true,
                                              0,
                                              0);
        m_elementList[LOG_IN_FORGOT_PASS_LINK] = m_forgotPasswordLabel;
        connect(m_forgotPasswordLabel,
                SIGNAL(sigTextClick(int)),
                this,
                SLOT(slotTextClicked(int)));
        connect(m_forgotPasswordLabel,
                SIGNAL(sigMouseHover(int,bool)),
                this,
                SLOT(slotTextLableHover(int,bool)));

        m_infoPage = new InfoPage (0, 0, LOG_IN_BOX_WIDTH, LOG_IN_BOX_HEIGHT, INFO_LOGIN, this);
        connect (m_infoPage,
                 SIGNAL(sigInfoPageCnfgBtnClick(int)),
                 this,
                 SLOT(slotInfoPageBtnclick(int)));

        m_currElement = LOG_IN_USR_TXTBOX;
        m_elementList[m_currElement]->forceActiveFocus();
        this->show ();
    }
}

LogIn::~LogIn()
{
    disconnect(m_mainCloseButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_payloadLib;

    deleteElements();
}

void LogIn::getOtherUserParam()
{
    QList<QVariant> paramList;
    paramList.insert(SUB_ACTIVITY_TYPE, READ_OTHER_LOGIN_PARAM);

    if(m_applController->processActivity(OTHER_LOGIN_ACTIVITY, paramList, &m_otherLoginParam) == true)
    {
        m_rmbPwdChkBox->changeState (m_otherLoginParam.rememberMe == true ? ON_STATE : OFF_STATE);
    }

    if(m_otherLoginParam.rememberMe == true)
    {
        m_userTextBox->setInputText(m_otherLoginParam.username);
        m_passwordTextBox->setInputText(m_otherLoginParam.password);
    }
    else
    {
        m_userTextBox->setInputText("admin");
    }
}

void LogIn::getlocalUserParam()
{
    QList<QVariant> paramList;
    paramList.insert(0,"");

    if(m_applController->processActivity(LOCAL_LOGIN_ACTIVITY, paramList, NULL) == true)
    {
        m_username = paramList.at(LOCAL_LOGIN_USERNAME).toString();
        m_password = paramList.at(LOCAL_LOGIN_PASSWORD).toString();
    }
    else
    {
        m_username = "";
        m_password = "";
    }
    sendChangeUserCmd ();
}

void LogIn::rememberStatusToFile()
{
    QList<QVariant> paramList;
    paramList.insert(SUB_ACTIVITY_TYPE, WRITE_OTHER_LOGIN_PARAM);

    snprintf (m_otherLoginParam.username, MAX_USERNAME_SIZE, "%s", m_username.toLatin1 ().constData ());
    snprintf (m_otherLoginParam.password, MAX_PASSWORD_SIZE, "%s", m_password.toLatin1 ().constData ());

    m_otherLoginParam.rememberMe = m_rmbPwdChkBox->getCurrentState () == ON_STATE ? true : false;

    if(m_applController->processActivity(OTHER_LOGIN_ACTIVITY, paramList, &m_otherLoginParam) == false)
    {
        emit sigClosePage(m_toolbarPageIndex);
    }
}

bool LogIn::sendChangeUserCmd()
{
    if(true == m_isLogInRequestSend)
    {
        return true;
    }

    m_isLogInRequestSend = true;
    if(m_imgType == STATE_1)
    {
        if(m_userParam->textStr == "" )
        {
            m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LOGIN_USER_NAME_ERROR));
            return false;
        }
        else if(m_passwordParam->textStr == "" )
        {
            m_infoPage->loadInfoPage (ValidationMessage::getValidationMessage(LOGIN_PASSWORD_ERROR));
            return false;
        }

        m_username = m_userParam->textStr;
        m_password = m_passwordParam->textStr;
    }

    m_payloadLib->setCnfgArrayAtIndex (0,m_username);
    m_payloadLib->setCnfgArrayAtIndex (1,m_password);

    QString payloadString = m_payloadLib->createDevCmdPayload(2);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = CNG_USER;
    param->payload = payloadString;

    m_applController->processActivity(m_currentDevName, DEVICE_COMM, param);
    return true;
}

void LogIn::sendChangePsdCmd()
{
    if((m_currentPasswordTextBox->getIsEnabled()) && (m_currentPasswordTextBox->getInputText() == ""))
    {
        m_infoPage->loadInfoPage (ValidationMessage::getValidationMessage(LOGIN_CURRNT_PASS_ERROR));
    }
    else if(m_passwordTextBox->getInputText() == "" )
    {
        if(m_currentPasswordTextBox->getIsEnabled())
        {
            m_infoPage->loadInfoPage (ValidationMessage::getValidationMessage(LOGIN_NEW_PASS_BLANK_ERROR));
        }
        else
        {
            m_infoPage->loadInfoPage (ValidationMessage::getValidationMessage(LOGIN_PASSWORD_ERROR));
        }
    }
    else if(m_confmpasswordTextBox->getInputText() == "")
    {
        m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LOGIN_CONFM_PASS_ERROR));
    }
    else if(m_passwordTextBox->getInputText() != m_confmpasswordTextBox->getInputText())
    {
        m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LOGIN_PASS_MISMATCH_ERROR));
    }
    else if((m_currentPasswordTextBox->getIsEnabled()) && (m_password != m_currentPasswordTextBox->getInputText()))
    {
        m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LOGIN_INCORRECT_PASS_ERROR));
    }
    else
    {
        m_payloadLib->setCnfgArrayAtIndex (0,m_passwordTextBox->getInputText());

        QString payloadString = m_payloadLib->createDevCmdPayload(1);

        DevCommParam* param = new DevCommParam();
        param->msgType = MSG_SET_CMD;
        param->cmdType = CNG_PWD;
        param->payload = payloadString;

        m_applController->processActivity(m_currentDevName, DEVICE_COMM, param);
    }
}

void LogIn::getDeviceConfig()
{
    QString payloadString = m_payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                               NETWORK_DEVICE_SETTING_TABLE_INDEX,
                                                               LOG_SETTING_FROM_INDEX,
                                                               MAX_REMOTE_DEVICES,
                                                               LOG_SETTING_FROM_FIELD,
                                                               LOG_SETTING_TO_FIELD,
                                                               LOG_SETTING_TO_FIELD);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_GET_CFG;
    param->payload = payloadString;

    m_applController->processActivity(m_currentDevName, DEVICE_COMM, param);
}

void LogIn::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if (IS_VALID_OBJ(m_passwordRecovery))
    {   
        /* pass deviceResponse to PasswordRecovery class */
        m_passwordRecovery->processDeviceResponse(param, deviceName);
        return;
    }

    if (IS_VALID_OBJ(m_forgotPassword))
    {
        /* pass deviceResponse to ForgotPassword class */
        m_forgotPassword->processDeviceResponse(param, deviceName);
        return;
    }

    if (deviceName != m_currentDevName)
    {
        return;
    }

    if (param->msgType == MSG_SET_CMD)
    {
        if (param->cmdType == CNG_PWD)
        {
            switch(param->deviceStatus)
            {
                case CMD_SUCCESS:
                {
                    if (m_imgType != STATE_1)
                    {
                        break;
                    }

                    /* Current password textbox will be enabled only if password expired */
                    bool isPasswordExpireResp = m_currentPasswordTextBox->getIsEnabled();
                    m_currentPasswordTextBox->hide();
                    m_currentPasswordTextBox->setIsEnabled(false);

                    m_userTextBox->show();
                    m_userTextBox->setIsEnabled(true);
                    m_elementList[LOG_IN_USR_TXTBOX] = m_userTextBox;

                    m_confmpasswordTextBox->hide();
                    m_confmpasswordTextBox->setIsEnabled(false);

                    m_rmbPwdChkBox->show();
                    m_rmbPwdChkBox->setIsEnabled(true);
                    m_elementList[LOG_IN_REM_PASS_CHEKBOX] = m_rmbPwdChkBox;

                    m_logInHeadings->changeText("");
                    m_logInButton->changeText(userLoginStrings[LOG_IN_LOG_BTN_STRING]);
                    m_passwordTextBox->changeLabel(userLoginStrings[LOG_IN_PASSWORD_STRING]);
                    m_forgotPasswordLabel->show();

                    /* Display password changed successfully message */
                    MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(MODIFY_PASS_CHANGE_SUCCESS));

                    /* Do not prompt password recovery info in case of password expire response */
                    if (isPasswordExpireResp)
                    {
                        break;
                    }

                    /* hide the login window */
                    this->hide();

                    /* create the PasswordRecovery window when user is first time login */
                    /* PARASOFT: Memory Deallocated in slot ExitPage */
                    m_passwordRecovery = new PasswordRecovery(parentWidget());
                    connect(m_passwordRecovery,
                            SIGNAL(sigExitPage(void)),
                            this,
                            SLOT(slotExitPage(void)));
                }
                break;

                case CMD_USER_ACCOUNT_LOCK:
                {
                    m_payloadLib->parseDevCmdReply (false, param->payload);
                    m_infoPage->loadInfoPage(USER_ACC_LOCKED_DUE_TO_FAIL_ATTEMPT_MSG(m_payloadLib->getCnfgArrayAtIndex(2).toUInt()));
                }
                break;

                case CMD_MIN_PASSWORD_CHAR_REQUIRED:
                {
                    m_payloadLib->parseDevCmdReply (false,param->payload);
                    m_infoPage->loadInfoPage(USER_PASSWROD_MIN_LEN_MSG(m_payloadLib->getCnfgArrayAtIndex(1).toUInt()));
                }
                break;

                default:
                {
                    m_infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                }
                break;
            }
        }
        else if (param->cmdType == CNG_USER)
        {
            switch(param->deviceStatus)
            {
                case CMD_SUCCESS:
                {
                    bool isClosePageReqNeedToSend = true;
                    m_payloadLib->parseDevCmdReply(false, param->payload);
                    m_applController->SetUserGroupType (deviceName,(USRS_GROUP_e)m_payloadLib->getCnfgArrayAtIndex (1).toUInt ());

                    if(m_imgType == STATE_1)
                    {
                        m_logInHeadings->changeText("");
                        if(m_payloadLib->getCnfgArrayAtIndex(3).toUInt() == 1)
                        {
                            isClosePageReqNeedToSend = false;
                            m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LOGIN_PASS_EXPIRE_ONE_DAY));
                        }
                        rememberStatusToFile();
                        MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(LOGIN_SUCCESS_MSG));
                        emit sigChangeToolbarButtonState(LOG_BUTTON, STATE_2);
                    }
                    else
                    {
                        MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(LOGIN_LOGOUT_SUCCESS_MSG));
                        emit sigChangeToolbarButtonState(LOG_BUTTON, STATE_1);
                    }

                    setCurrentLoginState(LOGIN_NORMAL);

                    if(isClosePageReqNeedToSend)
                    {
                        getDeviceConfig();
                    }
                }
                break;

                case CMD_RESET_PASSWORD:
                {
                    if(m_imgType == STATE_1)
                    {
                        m_rmbPwdChkBox->setIsEnabled(false);
                        m_rmbPwdChkBox->hide();

                        m_confmpasswordTextBox->show();
                        m_confmpasswordTextBox->setIsEnabled(true);

                        m_confmpasswordTextBox->setInputText("");
                        m_passwordTextBox->setInputText("");
                        m_userTextBox->setIsEnabled(false);

                        m_logInButton->changeText(userLoginStrings[LOG_IN_CREATE_PASSWORD_STRING]);
                        m_elementList[LOG_IN_REM_PASS_CHEKBOX] = m_confmpasswordTextBox;
                        m_logInHeadings->changeText(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                        m_forgotPasswordLabel->hide();
                    }
                    setCurrentLoginState(LOGIN_RESET_PASS);
                    m_isLogInRequestSend = false;
                }
                break;

                case CMD_PASSWORD_EXPIRE:
                {
                    m_logInHeadings->changeText(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));

                    if(m_imgType == STATE_1)
                    {
                        m_rmbPwdChkBox->hide();
                        m_rmbPwdChkBox->setIsEnabled(false);

                        m_userTextBox->hide();
                        m_userTextBox->setIsEnabled(false);

                        m_currentPasswordTextBox->show();
                        m_currentPasswordTextBox->setIsEnabled(true);
                        m_elementList[LOG_IN_USR_TXTBOX] = m_currentPasswordTextBox;

                        m_confmpasswordTextBox->show();
                        m_confmpasswordTextBox->setIsEnabled(true);
                        m_elementList[LOG_IN_REM_PASS_CHEKBOX] = m_confmpasswordTextBox;

                        m_logInButton->changeText(userLoginStrings[LOG_IN_CHANGE_PASSWORD_STRING]);
                        m_passwordTextBox->changeLabel(userLoginStrings[LOG_IN_NEW_PASSWORD_STRING]);
                        m_password = m_passwordTextBox->getInputText();
                        m_passwordTextBox->setInputText("");
                        m_forgotPasswordLabel->hide();
                    }
                    setCurrentLoginState(LOGIN_EXPIRE_PASS);
                    m_isLogInRequestSend = false;
                }
                break;

                case CMD_USER_ACCOUNT_LOCK:
                {
                    m_payloadLib->parseDevCmdReply (false,param->payload);
                    m_infoPage->loadInfoPage(USER_ACC_LOCKED_DUE_TO_FAIL_ATTEMPT_MSG(m_payloadLib->getCnfgArrayAtIndex(2).toUInt()));
                    m_isLogInRequestSend = false;
                }
                break;

                default:
                {
                    m_isLogInRequestSend = false;
                    if(m_imgType == STATE_1)
                    {
                        m_payloadLib->parseDevCmdReply (false,param->payload);
                        if(param->deviceStatus == CMD_INVALID_CREDENTIAL)
                        {
                            m_passwordTextBox->setInputText("");
                        }
                        m_logInHeadings->changeText("");

                        QString str = "";
                        quint32 remainingLoginAttempts = m_payloadLib->getCnfgArrayAtIndex (4).toUInt();
                        if (remainingLoginAttempts)
                        {
                            str = REMAINING_FAILED_ATTEMPT_MSG(remainingLoginAttempts);
                        }
                        m_infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus)+str);
                    }
                    else
                    {
                        MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                        emit sigClosePage(m_toolbarPageIndex);
                    }
                }
                break;
            }
        }
    }
    else if (param->msgType == MSG_PWD_RST)
    {
        if (param->pwdRstCmdType != REQ_PWD_RST_SESSION)
        {
            return;
        }

        m_isLogInRequestSend = false;
        switch(param->deviceStatus)
        {
            case CMD_SUCCESS:
            {
                PASSWORD_RESET_INFO_t pwdRstInfo;

                /* Skip 0th index. It is sessionId */
                m_payloadLib->parseDevCmdReply(false, param->payload);
                quint8 pwdRstModeMask = m_payloadLib->getCnfgArrayAtIndex(1).toUInt();
                pwdRstInfo.emailId = m_payloadLib->getCnfgArrayAtIndex(2).toString();
                pwdRstInfo.questionId[0] = m_payloadLib->getCnfgArrayAtIndex(3).toUInt();
                pwdRstInfo.questionId[1] = m_payloadLib->getCnfgArrayAtIndex(4).toUInt();
                pwdRstInfo.questionId[2] = m_payloadLib->getCnfgArrayAtIndex(5).toUInt();

                pwdRstInfo.isModeCnfg[PWD_RST_VERIFY_MODE_OTP] = ((pwdRstModeMask >> PWD_RST_VERIFY_MODE_OTP) & 1);
                pwdRstInfo.isModeCnfg[PWD_RST_VERIFY_MODE_SEC_QA] = ((pwdRstModeMask >> PWD_RST_VERIFY_MODE_SEC_QA) & 1);

                this->hide();

                /* PARASOFT: Memory Deallocated in slot ExitPage */
                m_forgotPassword = new ForgotPassword(deviceName, m_userTextBox->getInputText(), pwdRstInfo, parentWidget());
                connect(m_forgotPassword,
                        SIGNAL(sigExitPage(void)),
                        this,
                        SLOT(slotExitPage(void)));
            }
            break;

            case CMD_USER_ACCOUNT_LOCK:
            {
                m_payloadLib->parseDevCmdReply(false, param->payload);
                m_infoPage->loadInfoPage(USER_ACC_LOCKED_DUE_TO_FAIL_ATTEMPT_MSG(m_payloadLib->getCnfgArrayAtIndex(0).toUInt()));
            }
            break;

            default:
            {
                m_infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
            }
            break;
        }
    }
    else if (param->msgType == MSG_GET_CFG)
    {
        if(param->deviceStatus != CMD_SUCCESS)
        {
            return;
        }

        m_payloadLib->parsePayload(param->msgType, param->payload);
        if(m_payloadLib->getcnfgTableIndex(0) != NETWORK_DEVICE_SETTING_TABLE_INDEX)
        {
            return;
        }

        QString regString;
        QStringList cfgList, cfgFields;
        quint8 remoteDeviceIndex;
        QStringList fieldAndValue;
        quint8 fieldNo;
        QString fieldValue;

        regString.append('[');
        regString.append(QChar(SOT));
        regString.append(QChar(SOI));
        regString.append(QChar(EOI));
        regString.append(QChar(EOT));
        regString.append(']');

        QRegExp regExp (regString);

        cfgList = param->payload.split(regExp, QString::SkipEmptyParts);
        cfgList.removeFirst ();

        for(quint8 index = 0; index < MAX_REMOTE_DEVICES; index++)
        {
            cfgFields = cfgList.at(index).split(FSP, QString::KeepEmptyParts);
            remoteDeviceIndex = cfgFields.at(0).toUInt ();

            for(quint8 field = 1; field < (cfgFields.length() - 1); field++)
            {
                fieldAndValue = cfgFields.at(field).split(FVS);
                fieldNo = fieldAndValue.at(0).toUInt();
                fieldValue = fieldAndValue.at(1);

                switch(fieldNo)
                {
                    case 1:
                        snprintf(rdevTempConfig[(remoteDeviceIndex - 1)].deviceName, MAX_DEVICE_NAME_SIZE, "%s", fieldValue.toUtf8().constData());
                        break;

                    case 2:
                        rdevTempConfig[(remoteDeviceIndex - 1)].connType = (CONNECTION_TYPE_e)fieldValue.toUInt();
                        break;

                    case 3 :
                        snprintf(rdevTempConfig[(remoteDeviceIndex - 1)].ipAddress, MAX_IP_ADDRESS_SIZE, "%s", fieldValue.toLatin1().constData());
                        break;

                    case 4:
                        rdevTempConfig[(remoteDeviceIndex - 1)].port = fieldValue.toUInt();
                        break;

                    case 5:
                        snprintf(rdevTempConfig[(remoteDeviceIndex - 1)].username, MAX_USERNAME_SIZE, "%s",fieldValue.toLatin1().constData());
                        break;

                    case 6:
                        snprintf(rdevTempConfig[(remoteDeviceIndex - 1)].password, MAX_PASSWORD_SIZE, "%s",fieldValue.toLatin1().constData());
                        break;

                    case 7:
                        rdevTempConfig[(remoteDeviceIndex - 1)].enable = ((fieldValue.toUInt() == 0) ? false : true);
                        break;

                    case 8:
                        rdevTempConfig[(remoteDeviceIndex - 1)].autoLogin = ((fieldValue.toUInt() == 0) ? false : true);
                        break;

                    case 9:
                        rdevTempConfig[(remoteDeviceIndex - 1)].liveStreamType = fieldValue.toUInt();
                        break;

                    case 10:
                        rdevTempConfig[(remoteDeviceIndex - 1)].nativeDeviceCredential = ((fieldValue.toUInt() == 0) ? false : true);
                        break;

                    case 11:
                        rdevTempConfig[(remoteDeviceIndex - 1)].forwardedTcpPort = fieldValue.toUInt();
                        break;

                    default:
                        break;
                }
            }

            if((m_username != DEFAULT_LOGIN_USER) && (rdevTempConfig[(remoteDeviceIndex - 1)].nativeDeviceCredential))
            {
                snprintf(rdevTempConfig[(remoteDeviceIndex - 1)].username, MAX_USERNAME_SIZE,"%s", m_username.toUtf8().constData());
                snprintf(rdevTempConfig[(remoteDeviceIndex - 1)].password, MAX_PASSWORD_SIZE,"%s", m_password.toUtf8().constData());
            }

            // signal to notify change in device update
            m_applController->slotDeviceCfgUpdate(remoteDeviceIndex,&rdevTempConfig[(remoteDeviceIndex-1)],true);
        }
        emit sigClosePage(m_toolbarPageIndex);
    }
}

void LogIn::takeLeftKeyAction()
{
    do
    {
        m_currElement = (m_currElement - 1 + MAX_LOG_IN_CTRL) % MAX_LOG_IN_CTRL;
    }while(!m_elementList[m_currElement]->getIsEnabled());

    m_elementList[m_currElement]->forceActiveFocus();
}

void LogIn::takeRightKeyAction()
{
    do
    {
        m_currElement = (m_currElement + 1) % MAX_LOG_IN_CTRL;
    }while(!m_elementList[m_currElement]->getIsEnabled());

    m_elementList[m_currElement]->forceActiveFocus();
}

void LogIn::showEvent (QShowEvent *event)
{
    QWidget::showEvent (event);
    m_elementList[m_currElement]->forceActiveFocus();
}

void LogIn::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction ();
}

void LogIn::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void LogIn::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void LogIn::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    m_currElement = LOG_IN_CLS_BTN;
    m_elementList[m_currElement]->forceActiveFocus();
}

void LogIn::slotUpdateCurrentElement(int index)
{
    m_currElement = index;
}

void LogIn::deleteElements()
{
    if(m_imgType != STATE_1)
    {
        m_imgType = STATE_2;
        return;
    }

    /* Delete password recovery related objects if created */
    slotExitPage();

    DELETE_OBJ(m_logInHeadings);

    if(IS_VALID_OBJ(m_userTextBox))
    {
        disconnect (m_userTextBox,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_userTextBox);
    }
    DELETE_OBJ(m_userParam);

    if(IS_VALID_OBJ(m_passwordTextBox))
    {
        disconnect (m_passwordTextBox,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_passwordTextBox);
    }
    DELETE_OBJ(m_passwordParam);

    if(IS_VALID_OBJ(m_confmpasswordTextBox))
    {
        disconnect (m_confmpasswordTextBox,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_confmpasswordTextBox);
    }
    DELETE_OBJ(m_confmpasswordParam);

    if(IS_VALID_OBJ(m_rmbPwdChkBox))
    {
        disconnect (m_rmbPwdChkBox,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_rmbPwdChkBox);
    }

    if(IS_VALID_OBJ(m_logInButton))
    {
        disconnect (m_logInButton,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        disconnect (m_logInButton,
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotButtonClick(int)));
        DELETE_OBJ(m_logInButton);
    }

    if(IS_VALID_OBJ(m_forgotPasswordLabel))
    {
        disconnect(m_forgotPasswordLabel,
                   SIGNAL(sigTextClick(int)),
                   this,
                   SLOT(slotTextClicked(int)));
        disconnect(m_forgotPasswordLabel,
                   SIGNAL(sigMouseHover(int,bool)),
                   this,
                   SLOT(slotTextLableHover(int,bool)));
        DELETE_OBJ(m_forgotPasswordLabel);
    }

    if(IS_VALID_OBJ(m_infoPage))
    {
        disconnect (m_infoPage,
                    SIGNAL(sigInfoPageCnfgBtnClick(int)),
                    this,
                    SLOT(slotInfoPageBtnclick(int)));
        DELETE_OBJ(m_infoPage);
    }

    if(IS_VALID_OBJ(m_currentPasswordTextBox))
    {
        disconnect (m_currentPasswordTextBox,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_currentPasswordTextBox);
    }
    DELETE_OBJ(m_currentPasswordParam);
    m_imgType = STATE_2;
}
LOGIN_STATE_e LogIn::getCurrentLoginState() const
{
    return m_currentLoginState;
}

void LogIn::setCurrentLoginState(const LOGIN_STATE_e &currentLoginState)
{
    m_currentLoginState = currentLoginState;
}

void LogIn::slotButtonClick (int)
{
    if(m_rmbPwdChkBox->getIsEnabled())
    {
        sendChangeUserCmd();
    }
    else
    {
        sendChangePsdCmd();
    }
}

void LogIn::slotInfoPageBtnclick(int)
{
    if(m_infoPage->getText() == ValidationMessage::getValidationMessage(LOGIN_PASS_EXPIRE_ONE_DAY))
    {
        emit sigClosePage(m_toolbarPageIndex);
    }
    else
    {
        m_elementList[m_currElement]->forceActiveFocus();
    }
    m_isLogInRequestSend = false;
}

void LogIn::slotExitPage(void)
{
    /* close the PasswordRecovery window */
    if (IS_VALID_OBJ(m_passwordRecovery))
    {
        disconnect(m_passwordRecovery,
                   SIGNAL(sigExitPage(void)),
                   this,
                   SLOT(slotExitPage(void)));
        DELETE_OBJ(m_passwordRecovery);
    }

    if (IS_VALID_OBJ(m_forgotPassword))
    {
        disconnect(m_forgotPassword,
                   SIGNAL(sigExitPage(void)),
                   this,
                   SLOT(slotExitPage(void)));
        DELETE_OBJ(m_forgotPassword);
    }

    /* show the login window */
    this->show();
}

void LogIn::slotTextLableHover(int, bool isHoverIn)
{
    if(isHoverIn == true)
    {
        m_forgotPasswordLabel->changeColor(MOUSE_HOWER_COLOR);
        m_forgotPasswordLabel->repaint();
    }
    else
    {
        m_forgotPasswordLabel->changeColor(HIGHLITED_FONT_COLOR);
        m_forgotPasswordLabel->repaint();
    }
}

void LogIn::slotTextClicked(int)
{
    if(true == m_isLogInRequestSend)
    {
        return;
    }

    m_isLogInRequestSend = true;
    if (m_userParam->textStr == "")
    {
        m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LOGIN_USER_NAME_ERROR));
        return;
    }

    m_payloadLib->setCnfgArrayAtIndex(0, NVR_SMART_CODE);
    m_payloadLib->setCnfgArrayAtIndex(1, m_userParam->textStr);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_PWD_RST;
    param->pwdRstCmdType = REQ_PWD_RST_SESSION;
    param->payload = m_payloadLib->createDevCmdPayload(2);

    m_applController->processActivity(m_currentDevName, DEVICE_COMM, param);
}
