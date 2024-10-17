#include "ForgotPassword.h"

#define FORGOT_PWD_BOX_WIDTH        SCALE_WIDTH(640)
#define FORGOT_PWD_BOX_HEIGHT       SCALE_HEIGHT(400)
#define EMAIL_MASK_CHAR_LEN         3
#define LEFT_MARGIN_FROM_CENTER     SCALE_WIDTH(110)

static const QString forgotPwdLabelStr[MAX_FRGT_PWD_CTRL] =
{
    "",
    "Verification Mode",
    "Receive OTP at",
    "Enter OTP",
    "",
    "",
    "",
    "Resend OTP after",
    "Verify",
    "Get OTP",
    "Resend OTP",
    "Cancel",
};

static const QString verificationModeStrings[PWD_RST_VERIFY_MODE_MAX] = {"Receive OTP", "Security Question"};

ForgotPassword::ForgotPassword(QString deviceName, QString username, PASSWORD_RESET_INFO_t &pwdRstInfo, QWidget *parent) :
    BackGround((ApplController::getXPosOfScreen() + FORGOT_PWD_BOX_WIDTH) ,
               (ApplController::getYPosOfScreen() + FORGOT_PWD_BOX_HEIGHT - SCALE_HEIGHT(60)),
               FORGOT_PWD_BOX_WIDTH,
               FORGOT_PWD_BOX_HEIGHT,
               BACKGROUND_TYPE_4,
               LOG_BUTTON,
               parent,
               false,
               "Reset Password")
{
    INIT_OBJ(m_passwordReset);
    m_applController = ApplController::getInstance();
    m_username = username;
    m_currentDeviceName = deviceName;

    /* Display only first 3 characters and hide remaining characters from email username and domain.
     * e.g: Convert from "matrix.comsec@matrixcomsec.com" to "mat**********@mat*********.com" */
    /* Get length of email name and including domain */
    quint8 emailNameLen = pwdRstInfo.emailId.indexOf('@');
    quint8 domainNameLen = pwdRstInfo.emailId.lastIndexOf('.');
    if (emailNameLen > EMAIL_MASK_CHAR_LEN)
    {
        quint8 maskLen = emailNameLen - EMAIL_MASK_CHAR_LEN;
        pwdRstInfo.emailId.replace(EMAIL_MASK_CHAR_LEN, maskLen, QString(maskLen, '*'));
    }

    /* Add '@' in email length and remove email name and @ length from domain length */
    emailNameLen += 1;
    domainNameLen -= emailNameLen;
    if (domainNameLen > EMAIL_MASK_CHAR_LEN)
    {
        quint8 maskLen = domainNameLen - EMAIL_MASK_CHAR_LEN;
        pwdRstInfo.emailId.replace(emailNameLen + EMAIL_MASK_CHAR_LEN, maskLen, QString(maskLen, '*'));
    }

    m_closeButton = new CloseButtton(FORGOT_PWD_BOX_WIDTH - SCALE_WIDTH(30),
                                     SCALE_HEIGHT(30),
                                     this,
                                     CLOSE_BTN_TYPE_1,
                                     FRGT_PWD_CLOSE_BTN);
    connect(m_closeButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_closeButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotcloseButtonClick(int)));
    m_elementList[FRGT_PWD_CLOSE_BTN] = m_closeButton;

    QMap<quint8, QString> verifyModeList;
    for (quint8 index = 0; index < PWD_RST_VERIFY_MODE_MAX; index++)
    {
        verifyModeList.insert(index, verificationModeStrings[index]);
    }

    m_verificationModeDropDown = new DropDown(SCALE_WIDTH(20),
                                              SCALE_HEIGHT(50),
                                              FORGOT_PWD_BOX_WIDTH - SCALE_WIDTH(40),
                                              BGTILE_HEIGHT,
                                              FRGT_PWD_VERIFY_MODE,
                                              DROPDOWNBOX_SIZE_200,
                                              forgotPwdLabelStr[FRGT_PWD_VERIFY_MODE],
                                              verifyModeList,
                                              this,
                                              "",
                                              true,
                                              SCALE_WIDTH(5),
                                              COMMON_LAYER);
    m_elementList[FRGT_PWD_VERIFY_MODE] = m_verificationModeDropDown;
    connect (m_verificationModeDropDown,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotSpinBoxValueChange(QString, quint32)));
    connect(m_verificationModeDropDown,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_receiveOtpLabel = new TextLabel((FORGOT_PWD_BOX_WIDTH/2) - SCALE_WIDTH(10),
                                      m_verificationModeDropDown->y() + BGTILE_HEIGHT + SCALE_HEIGHT(20),
                                      NORMAL_FONT_SIZE,
                                      forgotPwdLabelStr[FRGT_PWD_RECEIVE_OTP],
                                      this,
                                      NORMAL_FONT_COLOR,
                                      NORMAL_FONT_FAMILY,
                                      ALIGN_END_X_START_Y);

    m_emailLabel = new TextLabel(FORGOT_PWD_BOX_WIDTH/2,
                                 m_receiveOtpLabel->y(),
                                 NORMAL_FONT_SIZE,
                                 pwdRstInfo.emailId,
                                 this,
                                 HIGHLITED_FONT_COLOR,
                                 NORMAL_FONT_FAMILY,
                                 ALIGN_START_X_START_Y, 0, false, SCALE_WIDTH((FORGOT_PWD_BOX_WIDTH/2)-20), 0, true);
    m_elementList[FRGT_PWD_RECEIVE_OTP] = m_emailLabel;

    m_otpTime = new QTime(0, 2, 0); /* 00:02:00 --> 2min */
    m_verifyOtpTimer = new QTimer();
    connect(m_verifyOtpTimer,
            SIGNAL(timeout()),
            this,
            SLOT(slotUpdateVerifyOtpTime()));
    m_verifyOtpTimer->setInterval(1000);

    m_enterOtpParam = new TextboxParam();
    m_enterOtpParam->maxChar = 6;
    m_enterOtpParam->isNumEntry = true;
    m_enterOtpParam->maxNumValue = 999999;
    m_enterOtpParam->isForceCenter = true;
    m_enterOtpParam->isTotalBlankStrAllow = true;
    m_enterOtpParam->labelStr = forgotPwdLabelStr[FRGT_PWD_OTP_TXTBOX];

    m_enterOtpTextBox = new TextBox(SCALE_WIDTH(20),
                                    m_receiveOtpLabel->y() + SCALE_HEIGHT(30),
                                    FORGOT_PWD_BOX_WIDTH - SCALE_WIDTH(40),
                                    BGTILE_HEIGHT,
                                    FRGT_PWD_OTP_TXTBOX,
                                    TEXTBOX_MEDIAM,
                                    this,
                                    m_enterOtpParam,
                                    NO_LAYER);
    m_elementList[FRGT_PWD_OTP_TXTBOX] = m_enterOtpTextBox;
    connect(m_enterOtpTextBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_otpTimerLabel = new TextLabel(FORGOT_PWD_BOX_WIDTH/2 - SCALE_WIDTH(10),
                                    m_enterOtpTextBox->y() + BGTILE_HEIGHT + SCALE_HEIGHT(10),
                                    NORMAL_FONT_SIZE,
                                    "",
                                    this,
                                    NORMAL_FONT_COLOR,
                                    NORMAL_FONT_FAMILY,
                                    ALIGN_END_X_START_Y);

    m_resendOtpLabel = new TextLabel(FORGOT_PWD_BOX_WIDTH - SCALE_WIDTH(30),
                                     m_enterOtpTextBox->y() + BGTILE_HEIGHT + SCALE_HEIGHT(10),
                                     NORMAL_FONT_SIZE,
                                     forgotPwdLabelStr[FRGT_PWD_RESEND_OTP_BTN],
                                     this,
                                     HIGHLITED_FONT_COLOR,
                                     NORMAL_FONT_FAMILY,
                                     ALIGN_END_X_START_Y,
                                     0,
                                     true,
                                     0,
                                     FRGT_PWD_RESEND_OTP_BTN);

    m_elementList[FRGT_PWD_RESEND_OTP_BTN] = m_resendOtpLabel;
    connect(m_resendOtpLabel,
            SIGNAL(sigTextClick(int)),
            this,
            SLOT(slotConfigButtonClick(int)));
    connect(m_resendOtpLabel,
            SIGNAL(sigMouseHover(int,bool)),
            this,
            SLOT(slotTextLableHover(int,bool)));

    quint16 xPos, yPos, stringIndex;
    for (quint8 index = 0; index < PASSWORD_RECOVERY_QA_MAX; index++)
    {
        stringIndex = ((SECURITY_QUESTION_2 - SECURITY_QUESTION_1) * index) + SECURITY_QUESTION_1;
        xPos = m_verificationModeDropDown->x();
        yPos = (index == PASSWORD_RECOVERY_QA_1) ? (m_verificationModeDropDown->y() + BGTILE_HEIGHT + SCALE_HEIGHT(10)) : m_answerTextBox[index - PASSWORD_RECOVERY_QA_2]->y() + BGTILE_HEIGHT;

        m_secQuestionTextboxParam[index] = new TextboxParam();
        m_secQuestionTextboxParam[index]->maxChar = 50;
        m_secQuestionTextboxParam[index]->labelStr = passwordRecoveryStrings[stringIndex] + QString(" %1").arg(index+1);
        m_secQuestionTextboxParam[index]->textStr = securityQuestionStrings[pwdRstInfo.questionId[index]];

        m_secQuestionTextBox[index] = new TextBox(xPos,
                                                  yPos,
                                                  FORGOT_PWD_BOX_WIDTH - SCALE_WIDTH(40),
                                                  BGTILE_HEIGHT,
                                                  index,
                                                  TEXTBOX_ULTRAMEDIAM,
                                                  this,
                                                  m_secQuestionTextboxParam[index],
                                                  COMMON_LAYER,
                                                  false, false, false,
                                                  LEFT_MARGIN_FROM_CENTER);

        m_answerTextboxParam[index] = new TextboxParam();
        m_answerTextboxParam[index]->maxChar = 25;
        m_answerTextboxParam[index]->minChar = 1;
        m_answerTextboxParam[index]->isTotalBlankStrAllow = true;
        m_answerTextboxParam[index]->labelStr = passwordRecoveryStrings[stringIndex + 1] + QString(" %1").arg(index+1);

        m_answerTextBox[index] = new TextBox(xPos,
                                             yPos + BGTILE_HEIGHT,
                                             FORGOT_PWD_BOX_WIDTH - SCALE_WIDTH(40),
                                             BGTILE_HEIGHT,
                                             index + FRGT_PWD_ANS_1_TXTBOX,
                                             TEXTBOX_ULTRAMEDIAM,
                                             this,
                                             m_answerTextboxParam[index],
                                             COMMON_LAYER, true, false, false,
                                             LEFT_MARGIN_FROM_CENTER);
        m_elementList[index + FRGT_PWD_ANS_1_TXTBOX] = m_answerTextBox[index];
        connect(m_answerTextBox[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

        m_secQuestionTextBox[index]->setVisible(false);
        m_answerTextBox[index]->setVisible(false);
    }

    m_okButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                m_answerTextBox[PASSWORD_RECOVERY_QA_3]->x() + SCALE_WIDTH(240),
                                (m_answerTextBox[PASSWORD_RECOVERY_QA_3]->y() + SCALE_HEIGHT(65)),
                                forgotPwdLabelStr[FRGT_PWD_VERIFY_BTN],
                                this,
                                FRGT_PWD_VERIFY_BTN);
    m_elementList[FRGT_PWD_VERIFY_BTN] = m_okButton;
    connect (m_okButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotConfigButtonClick(int)));
    connect (m_okButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    m_cancelButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                    m_okButton->x() + SCALE_WIDTH(200),
                                    (m_answerTextBox[PASSWORD_RECOVERY_QA_3]->y() + SCALE_HEIGHT(65)),
                                    passwordRecoveryStrings[CANCEL_BUTTON],
                                    this,
                                    FRGT_PWD_CANCEL_BTN);
    m_elementList[FRGT_PWD_CANCEL_BTN] = m_cancelButton;
    connect(m_cancelButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotConfigButtonClick(int)));
    connect(m_cancelButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_processBar = new ProcessBar(this->x(), this->y(), this->width(), this->height(), SCALE_WIDTH(0), parent);

    m_payloadLib = new PayloadLib();

    m_infoPage = new InfoPage(this->x(), this->y(), this->width(), this->height(), INFO_CONFIG_PAGE, parent, false, false);
    connect(m_infoPage,
            SIGNAL(sigInfoPageCnfgBtnClick(int)),
            this,
            SLOT(slotInfoPageCnfgBtnClick(int)));

    m_currentElement = FRGT_PWD_CLOSE_BTN;
    m_elementList[m_currentElement]->forceActiveFocus();

    if ((pwdRstInfo.isModeCnfg[PWD_RST_VERIFY_MODE_OTP] == false) || (pwdRstInfo.isModeCnfg[PWD_RST_VERIFY_MODE_SEC_QA] == false))
    {
        m_verificationModeDropDown->setIsEnabled(false);
    }

    m_modeChangeAllow = m_verificationModeDropDown->getIsEnabled();
    for (quint8 index = 0; index < PWD_RST_VERIFY_MODE_MAX; index++)
    {
        if (pwdRstInfo.isModeCnfg[index] == true)
        {
            m_verificationModeDropDown->setIndexofCurrElement(index);
            slotSpinBoxValueChange("", FRGT_PWD_VERIFY_MODE);
            break;
        }
    }

    this->show();
}

ForgotPassword::~ForgotPassword()
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

    disconnect(m_verificationModeDropDown,
               SIGNAL(sigValueChanged(QString,quint32)),
               this,
               SLOT(slotSpinBoxValueChange(QString, quint32)));
    disconnect(m_verificationModeDropDown,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    DELETE_OBJ(m_verificationModeDropDown);

    if (m_verifyOtpTimer->isActive())
    {
        m_verifyOtpTimer->stop();
    }

    disconnect(m_verifyOtpTimer,
               SIGNAL(timeout()),
               this,
               SLOT(slotUpdateVerifyOtpTime()));
    DELETE_OBJ(m_verifyOtpTimer);
    DELETE_OBJ(m_otpTime);

    DELETE_OBJ(m_emailLabel);
    DELETE_OBJ(m_receiveOtpLabel);

    disconnect(m_resendOtpLabel,
               SIGNAL(sigTextClick(int)),
               this,
               SLOT(slotConfigButtonClick(int)));
    disconnect(m_resendOtpLabel,
               SIGNAL(sigMouseHover(int,bool)),
               this,
               SLOT(slotTextLableHover(int,bool)));
    disconnect(m_enterOtpTextBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    DELETE_OBJ(m_enterOtpTextBox);
    DELETE_OBJ(m_enterOtpParam);

    DELETE_OBJ(m_resendOtpLabel);
    DELETE_OBJ(m_otpTimerLabel);

    for (quint8 index = 0; index < PASSWORD_RECOVERY_QA_MAX; index++)
    {
        DELETE_OBJ(m_secQuestionTextBox[index]);
        DELETE_OBJ(m_secQuestionTextboxParam[index]);

        disconnect(m_answerTextBox[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_answerTextBox[index]);
        DELETE_OBJ(m_answerTextboxParam[index]);
    }

    disconnect(m_okButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotConfigButtonClick(int)));
    disconnect(m_okButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    DELETE_OBJ(m_okButton);

    disconnect(m_cancelButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotConfigButtonClick(int)));
    disconnect(m_cancelButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    DELETE_OBJ(m_cancelButton);
    DELETE_OBJ(m_processBar);
    DELETE_OBJ(m_payloadLib);

    disconnect(m_infoPage,
               SIGNAL(sigInfoPageCnfgBtnClick(int)),
               this,
               SLOT(slotInfoPageCnfgBtnClick(int)));
    DELETE_OBJ(m_infoPage);
}

void ForgotPassword::processDeviceResponse(DevCommParam* param, QString deviceName)
{
    if (IS_VALID_OBJ(m_passwordReset))
    {
        /* pass deviceResponse to PasswordReset class */
        m_passwordReset->processDeviceResponse(param, deviceName);
        return;
    }

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
        emit sigExitPage();
        return;
    }

    switch(param->deviceStatus)
    {
        case CMD_SUCCESS:
        {
            switch(param->pwdRstCmdType)
            {
                case GET_PWD_RST_OTP:
                {
                    m_verificationModeDropDown->setIsEnabled(false);
                    m_okButton->changeText(forgotPwdLabelStr[FRGT_PWD_VERIFY_BTN]);
                    m_okButton->setIndexInPage(FRGT_PWD_VERIFY_BTN);
                    m_okButton->setIsEnabled(true);
                    m_enterOtpTextBox->setIsEnabled(true);
                    m_enterOtpTextBox->setVisible(true);
                    m_resendOtpLabel->setVisible(true);
                    m_resendOtpLabel->setIsEnabled(false);
                    slotTextLableHover();
                    m_otpTimerLabel->setVisible(true);

                    /* Reload count down timer */
                    *m_otpTime = QTime(0, 2, 0); /* 00:02:00 --> 2min */
                    if (!m_verifyOtpTimer->isActive())
                    {
                        m_verifyOtpTimer->start(1000);
                    }
                }
                break;

                case VERIFY_PWD_RST_OTP:
                case VERIFY_PWD_RST_QA:
                {
                    this->hide();

                    /* PARASOFT: Memory Deallocated in slot ExitPwdRstPage */
                    m_passwordReset = new PasswordReset(deviceName, m_username, parentWidget());
                    connect(m_passwordReset,
                            SIGNAL(sigExitPwdRstPage(void)),
                            this,
                            SLOT(slotExitPwdRstPage(void)));
                }
                break;

                default:
                {
                    /* Nothing to do */
                }
                break;
            }
        }
        break;

        case CMD_MISMATCH_SECURITY_ANSWERS:
        case CMD_MISMATCH_OTP:
        {
            QString str = "";
            m_payloadLib->parseDevCmdReply(false, param->payload);
            quint32 remainingAttempts = m_payloadLib->getCnfgArrayAtIndex(0).toUInt();
            if (remainingAttempts)
            {
                str = REMAINING_FAILED_ATTEMPT_MSG(remainingAttempts);
            }
            m_infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus)+str);
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

void ForgotPassword::sendCommand(PWD_RST_CMD_e cmdType, int totalfeilds)
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

void ForgotPassword::slotExitPwdRstPage(void)
{
    /* close the PasswordRecovery window */
    if (IS_VALID_OBJ(m_passwordReset))
    {
        disconnect(m_passwordReset,
                   SIGNAL(sigExitPwdRstPage(void)),
                   this,
                   SLOT(slotExitPwdRstPage(void)));
        DELETE_OBJ(m_passwordReset);
    }

    /* emit signal to close window */
    emit sigExitPage();
}

void ForgotPassword::slotUpdateCurrentElement(int indexInPage)
{
    m_currentElement = indexInPage;
}

void ForgotPassword::slotSpinBoxValueChange(QString, quint32)
{
    m_enterOtpTextBox->setVisible(false);
    m_otpTimerLabel->setVisible(false);
    m_resendOtpLabel->setVisible(false);
    m_okButton->setIsEnabled(true);

    if (m_verificationModeDropDown->getIndexofCurrElement() == PWD_RST_VERIFY_MODE_OTP)
    {
        m_receiveOtpLabel->setVisible(true);
        m_emailLabel->setVisible(true);
        m_okButton->changeText(forgotPwdLabelStr[FRGT_PWD_GET_OTP_BTN]);
        m_okButton->setIndexInPage(FRGT_PWD_GET_OTP_BTN);

        for (quint8 index = 0; index < PASSWORD_RECOVERY_QA_MAX; index++)
        {
            m_secQuestionTextBox[index]->setVisible(false);
            m_answerTextBox[index]->setVisible(false);
        }
    }
    else if(m_verificationModeDropDown->getIndexofCurrElement() == PWD_RST_VERIFY_MODE_SEC_QA)
    {
        m_receiveOtpLabel->setVisible(false);
        m_emailLabel->setVisible(false);
        m_okButton->changeText(forgotPwdLabelStr[FRGT_PWD_VERIFY_BTN]);
        m_okButton->setIndexInPage(FRGT_PWD_VERIFY_BTN);

        for (quint8 index = 0; index < PASSWORD_RECOVERY_QA_MAX; index++)
        {
            m_secQuestionTextBox[index]->setVisible(true);
            m_answerTextBox[index]->setVisible(true);
        }
    }
}

void ForgotPassword::slotUpdateVerifyOtpTime(void)
{
    *m_otpTime = m_otpTime->addSecs(-1);
    QString countdown = m_otpTime->toString("mm:ss");
    if (countdown == "00:00")
    {
        m_verifyOtpTimer->stop();
        m_enterOtpTextBox->setIsEnabled(false);
        m_otpTimerLabel->changeText("");
        m_otpTimerLabel->setVisible(false);
        m_resendOtpLabel->setIsEnabled(true);
        slotTextLableHover();
        m_okButton->setIsEnabled(false);
        m_verificationModeDropDown->setIsEnabled(m_modeChangeAllow);
    }
    else
    {
        m_otpTimerLabel->changeText(forgotPwdLabelStr[FRGT_PWD_RESEND_OTP_TIMER] + " " + countdown);
        m_otpTimerLabel->update();
    }
}

void ForgotPassword::slotTextLableHover(int, bool isHoverIn)
{
    if (m_resendOtpLabel->getIsEnabled() == false)
    {
        m_resendOtpLabel->changeColor(DISABLE_FONT_COLOR);
    }
    else if (isHoverIn == true)
    {
        m_resendOtpLabel->changeColor(MOUSE_HOWER_COLOR);
    }
    else
    {
        m_resendOtpLabel->changeColor(HIGHLITED_FONT_COLOR);
    }
    m_resendOtpLabel->repaint();
}

void ForgotPassword::slotConfigButtonClick(int indexInPage)
{
    if (indexInPage == FRGT_PWD_GET_OTP_BTN)
    {
        /* Send get OTP command */
        sendCommand(GET_PWD_RST_OTP, 0);
    }
    else if (indexInPage == FRGT_PWD_RESEND_OTP_BTN)
    {
        if (m_resendOtpLabel->getIsEnabled())
        {
            /* Clear OTP Text box */
            m_enterOtpTextBox->setInputText("");

            /* Send get OTP command */
            sendCommand(GET_PWD_RST_OTP, 0);
        }
    }
    else if (indexInPage == FRGT_PWD_VERIFY_BTN)
    {
        if (m_verificationModeDropDown->getIndexofCurrElement() == PWD_RST_VERIFY_MODE_OTP)
        {
            if ((m_enterOtpTextBox->getInputText() == "") || (m_enterOtpTextBox->getInputText().length() != m_enterOtpParam->maxChar))
            {
                m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_PWD_RST_OTP));
                return;
            }

            /* Send verify OTP command */
            m_payloadLib->setCnfgArrayAtIndex(0, m_enterOtpTextBox->getInputText());
            sendCommand(VERIFY_PWD_RST_OTP, 1);
        }
        else if (m_verificationModeDropDown->getIndexofCurrElement() == PWD_RST_VERIFY_MODE_SEC_QA)
        {
            quint8 index;
            for (index = 0; index < PASSWORD_RECOVERY_QA_MAX; index++)
            {
                if (m_answerTextBox[index]->getInputText() == "")
                {
                    m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_ALL_QA));
                    return;
                }
            }

            for (index = 0; index < PASSWORD_RECOVERY_QA_MAX; index++)
            {
                m_payloadLib->setCnfgArrayAtIndex(index, m_answerTextBox[index]->getInputText());
            }

            /* Send QA verify command */
            sendCommand(VERIFY_PWD_RST_QA, index);
        }
    }
    else if (indexInPage == FRGT_PWD_CANCEL_BTN)
    {
        /* if cancel button is clicked perform same action as close button */
        slotcloseButtonClick(0);
    }
}

void ForgotPassword::slotcloseButtonClick(int)
{
    /* Free allocated password reset session */
    sendCommand(CLR_PWD_RST_SESSION, 0);
}

void ForgotPassword::slotInfoPageCnfgBtnClick(int)
{
    if (m_infoPage->getText() == ValidationMessage::getDeviceResponceMessage(CMD_SESSION_EXPIRED))
    {
        /* emit signal to close window */
        emit sigExitPage();
    }
    else if (m_infoPage->getText().left(QString(USER_ACCOUNT_LOCK_PREFIX_STR).length()) == USER_ACCOUNT_LOCK_PREFIX_STR)
    {
        /* On account lock, go back to login page */
        slotcloseButtonClick(0);
    }
}
