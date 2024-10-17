#include "Login/PasswordReset.h"
#include "Controls/MessageBanner.h"
#include "ValidationMessage.h"

#define PWD_RST_BOX_WIDTH       SCALE_WIDTH(526)
#define PWD_RST_BOX_HEIGHT      SCALE_HEIGHT(277)

static const QString pwdRstLabelStr[PWD_RST_LABEL_MAX] = {"", "Username", "Password", "Confirm Password", "Create Password"};

PasswordReset::PasswordReset(QString deviceName, QString username, QWidget *parent) :
    BackGround((ApplController::getXPosOfScreen() + ((ApplController::getWidthOfScreen() - PWD_RST_BOX_WIDTH) / 2)) ,
               (ApplController::getYPosOfScreen() + ((ApplController::getHeightOfScreen() - PWD_RST_BOX_HEIGHT) / 2)),
               PWD_RST_BOX_WIDTH,
               PWD_RST_BOX_HEIGHT,
               BACKGROUND_TYPE_4,
               LOG_BUTTON,
               parent,
               false,
               ValidationMessage::getDeviceResponceMessage(CMD_RESET_PASSWORD))
{
    m_applController = ApplController::getInstance();
    m_currentDeviceName = deviceName;

    m_closeButton = new CloseButtton(PWD_RST_BOX_WIDTH - SCALE_WIDTH(30),
                                     SCALE_HEIGHT(30),
                                     this,
                                     CLOSE_BTN_TYPE_1,
                                     PWD_RST_LABEL_CLOSE_BUTTON);
    connect(m_closeButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_closeButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotcloseButtonClick(int)));
    m_elementList[PWD_RST_LABEL_CLOSE_BUTTON] = m_closeButton;

    m_usernameParam = new TextboxParam();
    m_usernameParam->isCentre = true;
    m_usernameParam->maxChar = 24;
    m_usernameParam->labelStr = pwdRstLabelStr[PWD_RST_LABEL_USERNAME];
    m_usernameParam->textStr = username;

    m_usernameTextBox = new TextBox((PWD_RST_BOX_WIDTH - BGTILE_SMALL_SIZE_WIDTH)/2 ,
                                    SCALE_HEIGHT(58),
                                    BGTILE_SMALL_SIZE_WIDTH,
                                    BGTILE_HEIGHT,
                                    PWD_RST_LABEL_USERNAME,
                                    TEXTBOX_LARGE,
                                    this,
                                    m_usernameParam,
                                    COMMON_LAYER,
                                    false);
    m_elementList[PWD_RST_LABEL_USERNAME] = m_usernameTextBox;
    connect (m_usernameTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    m_passwordParam = new TextboxParam();
    m_passwordParam->labelStr = pwdRstLabelStr[PWD_RST_LABEL_PASSWORD];
    m_passwordParam->isCentre = true;
    m_passwordParam->isTotalBlankStrAllow = true;
    m_passwordParam->maxChar = 16;
    m_passwordParam->minChar = 4;
    m_passwordParam->validation = QRegExp(asciiset1ValidationStringWithoutSpace);

    m_passwordTextBox = new PasswordTextbox(m_usernameTextBox->x() ,
                                            m_usernameTextBox->y() + BGTILE_HEIGHT,
                                            SCALE_WIDTH(410),
                                            BGTILE_HEIGHT,
                                            PWD_RST_LABEL_PASSWORD,
                                            TEXTBOX_LARGE,
                                            this,
                                            m_passwordParam);
    m_elementList[PWD_RST_LABEL_PASSWORD] = m_passwordTextBox;
    connect (m_passwordTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    m_confmpasswordParam = new TextboxParam();
    m_confmpasswordParam->labelStr = pwdRstLabelStr[PWD_RST_LABEL_CONFIRM_PASSWORD];
    m_confmpasswordParam->isCentre = true;
    m_confmpasswordParam->isTotalBlankStrAllow = true;
    m_confmpasswordParam->maxChar = 16;
    m_confmpasswordParam->minChar = 4;
    m_confmpasswordParam->validation = QRegExp(asciiset1ValidationStringWithoutSpace);

    m_confmpasswordTextBox = new PasswordTextbox(m_passwordTextBox->x() ,
                                                 m_passwordTextBox->y() + BGTILE_HEIGHT,
                                                 SCALE_WIDTH(410),
                                                 BGTILE_HEIGHT,
                                                 PWD_RST_LABEL_CONFIRM_PASSWORD,
                                                 TEXTBOX_LARGE,
                                                 this,
                                                 m_confmpasswordParam);
    m_elementList[PWD_RST_LABEL_CONFIRM_PASSWORD] = m_confmpasswordTextBox;
    connect (m_confmpasswordTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    m_logInButton = new CnfgButton(CNFGBUTTON_EXTRALARGE,
                                   PWD_RST_BOX_WIDTH/2,
                                   (m_confmpasswordTextBox->y () + SCALE_HEIGHT(20)) + (PWD_RST_BOX_HEIGHT - (m_confmpasswordTextBox->y() + BGTILE_HEIGHT))/2,
                                   pwdRstLabelStr[PWD_RST_LABEL_PASSWORD_BUTTON],
                                   this,
                                   PWD_RST_LABEL_PASSWORD_BUTTON);
    m_elementList[PWD_RST_LABEL_PASSWORD_BUTTON] = m_logInButton;
    connect (m_logInButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    connect (m_logInButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    m_processBar = new ProcessBar(this->x(), this->y(), this->width(), this->height(), SCALE_WIDTH(0), parent);

    m_payloadLib = new PayloadLib();

    m_infoPage = new InfoPage(this->x(), this->y(), this->width(), this->height(), INFO_LOGIN, parent, false, false);
    connect(m_infoPage,
            SIGNAL(sigInfoPageCnfgBtnClick(int)),
            this,
            SLOT(slotInfoPageCnfgBtnClick(int)));

    m_currentElement = PWD_RST_LABEL_PASSWORD;
    m_elementList[m_currentElement]->forceActiveFocus();
    this->show();
}

PasswordReset::~PasswordReset()
{
    disconnect(m_closeButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_closeButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotcloseButtonClick(int)));
    DELETE_OBJ(m_closeButton);

    disconnect (m_usernameTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    DELETE_OBJ(m_usernameTextBox);
    DELETE_OBJ(m_usernameParam);

    disconnect (m_passwordTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    DELETE_OBJ(m_passwordTextBox);
    DELETE_OBJ(m_passwordParam);

    disconnect (m_confmpasswordTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    DELETE_OBJ(m_confmpasswordTextBox);
    DELETE_OBJ(m_confmpasswordParam);

    disconnect (m_logInButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (m_logInButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    DELETE_OBJ(m_logInButton);

    DELETE_OBJ(m_processBar);
    DELETE_OBJ(m_payloadLib);

    disconnect(m_infoPage,
               SIGNAL(sigInfoPageCnfgBtnClick(int)),
               this,
               SLOT(slotInfoPageCnfgBtnClick(int)));
    DELETE_OBJ(m_infoPage);
}

void PasswordReset::processDeviceResponse(DevCommParam* param, QString deviceName)
{
    m_processBar->unloadProcessBar();
    if (deviceName != m_currentDeviceName)
    {
        return;
    }

    if (param->msgType != MSG_PWD_RST)
    {
        return;
    }

    if (param->pwdRstCmdType == CLR_PWD_RST_SESSION)
    {
        /* emit signal to close window */
        emit sigExitPwdRstPage();
        return;
    }

    if (param->pwdRstCmdType != SET_NEW_PWD)
    {
        return;
    }

    switch(param->deviceStatus)
    {
        case CMD_SUCCESS:
        {
            /* Display password changed successfully message */
            MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(MODIFY_PASS_CHANGE_SUCCESS));

            /* Free allocated password reset session */
            sendCommand(CLR_PWD_RST_SESSION, 0);
        }
        break;

        case CMD_MIN_PASSWORD_CHAR_REQUIRED:
        {
            m_payloadLib->parseDevCmdReply(false, param->payload);
            m_infoPage->loadInfoPage(USER_PASSWROD_MIN_LEN_MSG(m_payloadLib->getCnfgArrayAtIndex(0).toUInt()));
        }
        break;

        default:
        {
            m_infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
        }
        break;
    }
}

void PasswordReset::sendCommand(PWD_RST_CMD_e cmdType, int totalfeilds)
{
    /* Create payload to send cmd */
    QString payloadString = m_payloadLib->createDevCmdPayload(totalfeilds);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_PWD_RST;
    param->pwdRstCmdType = cmdType;
    param->payload = payloadString;
    m_processBar->loadProcessBar();
    m_applController->processActivity(m_currentDeviceName, DEVICE_COMM, param);
}

void PasswordReset::slotUpdateCurrentElement(int indexInPage)
{
    m_currentElement = indexInPage;
}

void PasswordReset::slotcloseButtonClick(int)
{
    /* Free allocated password reset session */
    sendCommand(CLR_PWD_RST_SESSION, 0);
}

void PasswordReset::slotButtonClick(int)
{
    if (m_passwordTextBox->getInputText() == "")
    {
        m_infoPage->loadInfoPage (ValidationMessage::getValidationMessage(LOGIN_PASSWORD_ERROR));
    }
    else if (m_confmpasswordTextBox->getInputText() == "")
    {
        m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LOGIN_CONFM_PASS_ERROR));
    }
    else if (m_passwordTextBox->getInputText() != m_confmpasswordTextBox->getInputText())
    {
        m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LOGIN_PASS_MISMATCH_ERROR));
    }
    else
    {
        m_payloadLib->setCnfgArrayAtIndex(0, m_passwordTextBox->getInputText());
        sendCommand(SET_NEW_PWD, 1);
    }
}

void PasswordReset::slotInfoPageCnfgBtnClick(int)
{
    if (m_infoPage->getText() == ValidationMessage::getDeviceResponceMessage(CMD_SESSION_EXPIRED))
    {
        /* emit signal to close window */
        emit sigExitPwdRstPage();
    }
}
