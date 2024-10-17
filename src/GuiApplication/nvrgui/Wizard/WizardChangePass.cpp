#include "WizardChangePass.h"
#include "Controls/MessageBanner.h"
#include "ValidationMessage.h"
#include <QPainter>

typedef enum
{
    MDFY_PSD_CONFIRM_PASSWORD_STRING,
    MDFY_PSD_NEW_PASSWORD_STRING,
    MDFY_PSD_OLD_PASSWORD_STRING,
    MDFY_PSD_OK_BUTTON_STRING,
    MAX_MDFY_PSD_VALIDATION_STRING
}MDFY_PSD_STRING_e;

static const QString modifyPasswordStrings[MAX_MDFY_PSD_VALIDATION_STRING] = {
    "Old Password",
    "New Password",
    "Confirm Password",
    "OK"
};

WizardChangePass::WizardChangePass(QString devName, QWidget *parent, STATE_TYPE_e state)
    : QWidget(parent)

{
    textLabelElide = NULL;
    m_applController = ApplController::getInstance();
    m_payloadLib = NULL;
    m_payloadLib = new PayloadLib();
    m_currentDeviceName  = devName;
    m_logButtonState = STATE_2;

    backGround = new Rectangle(SCALE_WIDTH(300),
                               SCALE_HEIGHT(200),
                               SCALE_WIDTH(500),
                               SCALE_HEIGHT(350),
                               SCALE_WIDTH(RECT_RADIUS),
                               NORMAL_BKG_COLOR,
                               NORMAL_BKG_COLOR,this);

    pageHeading = new Heading(backGround->x() + SCALE_WIDTH(265),
                              backGround->y() +  SCALE_HEIGHT(30), "Modify Password", this,
                              HEADING_TYPE_1,
                              SCALE_FONT(20));

    closeButtton = new CloseButtton((backGround->x() + SCALE_WIDTH(475)),
                                    backGround->y() +  SCALE_HEIGHT(25),
                                    this);
    connect (closeButtton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotcloseButtonClick(int)));

    for(quint8 index = 0; index < MAX_PASSWORD_BOX; index++)
    {
        passwordParam[index] = new TextboxParam();
        passwordParam[index]->labelStr =  modifyPasswordStrings[index] ;
        passwordParam[index]->isCentre =  true;
        passwordParam[index]->maxChar  = 16;
        passwordParam[index]->minChar = 4;
        passwordParam[index]->validation = QRegExp(asciiset1ValidationStringWithoutSpace);
        passwordParam[index]->isTotalBlankStrAllow = true;

        passwordTextBox[index] = new PasswordTextbox(((backGround->x() + SCALE_WIDTH(55))),
                                                     (backGround->y() + SCALE_HEIGHT(65) + (BGTILE_HEIGHT*index)),
                                                     BGTILE_SMALL_SIZE_WIDTH,
                                                     BGTILE_HEIGHT,
                                                     index,
                                                     TEXTBOX_LARGE,
                                                     this,
                                                     passwordParam[index]);
    }

    okButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                              (passwordTextBox[2]->x() + SCALE_WIDTH(215)),
                              (passwordTextBox[2]->y()+ BGTILE_HEIGHT + SCALE_WIDTH(25)),
                              modifyPasswordStrings[MDFY_PSD_OK_BUTTON_STRING],
                              this,
                              MAX_PASSWORD_BOX);
    connect (okButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotOkButtonClick(int)));

    MessageTimer = new QTimer(this);
    connect (MessageTimer,
             SIGNAL(timeout()),
             this,
             SLOT(slotStatusRepTimerTimeout()));
    MessageTimer->setInterval (10000);
    MessageTimer->setSingleShot(true);
    elementHeadingElide = NULL;
    elementHeadingElide = new ElementHeading(passwordTextBox[2]->x() - SCALE_WIDTH(10),
                                            (okButton->y() + SCALE_HEIGHT(50)),
                                            SCALE_WIDTH(430),
                                            SCALE_HEIGHT(50),
                                            "",
                                            NO_LAYER,
                                            this);
    QString fontColor = "#c8c8c8";
    QString fontWidth = "" + QString::number(SCALE_WIDTH(16)) +"px";
    QString styl = "ElidedLabel \
    { \
        color: %1; \
        font-size: %2; \
        font-family: %3; \
    }";

    elementHeadingElide->setStyleSheet(styl.arg(fontColor).arg(fontWidth).arg(NORMAL_FONT_FAMILY));
    getOldPassword(m_currentDeviceName);

    this->show();
    Q_UNUSED(state);
}

WizardChangePass::~WizardChangePass ()
{
    DELETE_OBJ(backGround);
    DELETE_OBJ(pageHeading);

    if(IS_VALID_OBJ(closeButtton))
    {
        disconnect (closeButtton,
                 SIGNAL(sigButtonClick(int)),
                 this,
                 SLOT(slotcloseButtonClick(int)));
        delete closeButtton;
    }

    disconnect (okButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotOkButtonClick(int)));
    delete okButton;

    for(quint8 index = 0; index < MAX_PASSWORD_BOX; index++)
    {
        delete passwordTextBox[index];
        delete passwordParam[index];
    }

    if(IS_VALID_OBJ(MessageTimer))
    {
        if(MessageTimer->isActive())
        {
            MessageTimer->stop();
        }
        disconnect (MessageTimer,
              SIGNAL(timeout()),
              this,
              SLOT(slotStatusRepTimerTimeout()));

        DELETE_OBJ(MessageTimer);
    }

    DELETE_OBJ(m_payloadLib);
    DELETE_OBJ(textLabelElide);
    DELETE_OBJ(elementHeadingElide);
}

void WizardChangePass::changePassword(QString devName)
{
    m_payloadLib->setCnfgArrayAtIndex (0, newPassword);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = CNG_PWD;
    param->payload = m_payloadLib->createDevCmdPayload(1);
    if(m_applController->processActivity(devName, DEVICE_COMM, param) == true)
    {
        m_currentDeviceName = devName;
    }
}

void WizardChangePass::getOldPassword(QString devName)
{
    m_applController->getPasswordFrmDev(devName, oldPasswordValue);
}

void WizardChangePass :: processDeviceResponse(DevCommParam *param, QString deviceName)
{
    bool closepage = false;
    QString tStr = "";

    if ((deviceName == m_currentDeviceName) && (param->msgType == MSG_SET_CMD) && (param->cmdType == CNG_PWD))
    {
        switch(param->deviceStatus)
        {
            case CMD_SUCCESS:
            {
                m_payloadLib->parseDevCmdReply(true, param->payload);
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(MODIFY_PASS_CHANGE_SUCCESS));
                getOldPassword (deviceName);
                closepage = true;
            }
            break;

            case CMD_USER_ACCOUNT_LOCK:
            {
                m_payloadLib->parseDevCmdReply (false,param->payload);
                tStr = USER_ACC_LOCKED_DUE_TO_FAIL_ATTEMPT_MSG(m_payloadLib->getCnfgArrayAtIndex(0).toUInt());
                closepage = true;
            }
            break;

            case CMD_MIN_PASSWORD_CHAR_REQUIRED:
            {
                m_payloadLib->parseDevCmdReply (false,param->payload);
                tStr = USER_PASSWROD_MIN_LEN_MSG(m_payloadLib->getCnfgArrayAtIndex(1).toUInt());
            }
            break;

            default:
            {
                tStr = ValidationMessage::getDeviceResponceMessage(param->deviceStatus);
            }
            break;
        }
    }

    if(tStr != "")
    {
        InfoMessage(tStr);
    }

    if(closepage == true)
    {
        emit sigExitPage(TOOLBAR_BUTTON_TYPE_e(0));
    }
}

void WizardChangePass::slotOkButtonClick(int)
{
    passwordTextBox[0]->getInputText (oldPassword);
    passwordTextBox[1]->getInputText (newPassword);
    passwordTextBox[2]->getInputText (confirmPassword);

    if(m_logButtonState == STATE_2)
    {
        if(oldPassword == "")
        {
            InfoMessage(ValidationMessage::getValidationMessage(MODIFY_PASS_ENT_OLD_PASS));
            return;
        }

        if(oldPassword != oldPasswordValue)
        {
            InfoMessage(ValidationMessage::getValidationMessage(MODIFY_PASS_INCORRECT_PASS));
            return;
        }

        if((newPassword != confirmPassword))
        {
            InfoMessage(ValidationMessage::getValidationMessage(PASS_MISMATCH));
            return;
        }

        changePassword(m_currentDeviceName);
    }
    else
    {
        InfoMessage(ValidationMessage::getValidationMessage(CMD_NO_PRIVILEGE));
    }
}

void WizardChangePass::slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e msgType)
{
    if( msgType == INFO_MSG_ERROR)
    {
        InfoMessage(ValidationMessage::getValidationMessage(PASSWORD_RANGE_ERROR));
    }
}

void WizardChangePass:: InfoMessage(QString StrInfo)
{
    if(textLabelElide != NULL)
    {
        MessageTimer->stop();
        textLabelElide->hide();
    }

    if(!MessageTimer->isActive())
    {
        MessageTimer->start();
    }

    textLabelElide = new ElidedLabel(Multilang(StrInfo.toUtf8().constData()), elementHeadingElide);
    textLabelElide->resize(SCALE_WIDTH(430), SCALE_HEIGHT(50));
    textLabelElide->raise();
    textLabelElide->show();
}

void WizardChangePass::slotStatusRepTimerTimeout()
{
    MessageTimer->stop();
}

void WizardChangePass::paintEvent(QPaintEvent *)
{
    this->setGeometry(0,0,  WIZARD_MAIN_RECT_WIDTH, WIZARD_MAIN_RECT_HEIGHT);

    QPainter painter(this);
    QColor color;

    color.setAlpha (150);
    painter.setBrush (QBrush(color));
    painter.setPen (Qt::NoPen);

    painter.drawRoundedRect (QRect(0,
                                   0,
                                   WIZARD_MAIN_RECT_WIDTH,
                                   WIZARD_MAIN_RECT_HEIGHT),
                                   SCALE_WIDTH(RECT_RADIUS), SCALE_HEIGHT(RECT_RADIUS));
}

void WizardChangePass::slotcloseButtonClick(int)
{
    emit sigExitPage(TOOLBAR_BUTTON_TYPE_e(21));
}
