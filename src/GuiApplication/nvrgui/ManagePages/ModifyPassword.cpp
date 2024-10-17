#include "ModifyPassword.h"
#include "Controls/MessageBanner.h"
#include "ValidationMessage.h"

typedef enum{
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

ModifyPassword::ModifyPassword(QString devName,
                               QWidget *parent, STATE_TYPE_e state)
    :ManageMenuOptions(devName, parent, (MAX_PASSWORD_BOX + 1))
{
    quint16 topMargin = (MANAGE_PAGE_RIGHT_PANEL_HEIGHT_WITHOUT_CNFGBUTTON - (BGTILE_HEIGHT * MAX_PASSWORD_BOX)) / 2;

    m_logButtonState = state;
    for(quint8 index = 0; index < MAX_PASSWORD_BOX; index++)
    {
        passwordParam[index] = new TextboxParam();

        passwordParam[index]->labelStr =  modifyPasswordStrings[index] ;
        passwordParam[index]->isCentre =  true;
        passwordParam[index]->maxChar  = 16;
        passwordParam[index]->minChar = 4;
        passwordParam[index]->validation = QRegExp(asciiset1ValidationStringWithoutSpace);
        passwordParam[index]->isTotalBlankStrAllow = true;

        passwordTextBox[index] = new PasswordTextbox(((this->width() - BGTILE_SMALL_SIZE_WIDTH) / 2),
                                                     (topMargin + (BGTILE_HEIGHT * index)),
                                                     BGTILE_SMALL_SIZE_WIDTH,
                                                     BGTILE_HEIGHT,
                                                     index,
                                                     TEXTBOX_LARGE,
                                                     this,
                                                     passwordParam[index]);

        m_elementList[index] = passwordTextBox[index];
//        connect(passwordTextBox[index],
//                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
//                this,
//                SLOT(slotTextBoxLoadInfopage (int,INFO_MSG_TYPE_e)));

        connect (passwordTextBox[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
    }

    okButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                              (this->width() / 2),
                              (MANAGE_PAGE_RIGHT_PANEL_HEIGHT - SCALE_HEIGHT(55)),
                              modifyPasswordStrings[MDFY_PSD_OK_BUTTON_STRING],
                              this,
                              MAX_PASSWORD_BOX);
    m_elementList[MAX_PASSWORD_BOX] = okButton;
    connect (okButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotOkButtonClick(int)));
    connect (okButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    getOldPassword(m_currentDeviceName);
}

ModifyPassword::~ModifyPassword ()
{
    disconnect (okButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotOkButtonClick(int)));
    disconnect (okButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete okButton;

    for(quint8 index = 0; index < MAX_PASSWORD_BOX; index++)
    {
        disconnect (passwordTextBox[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        disconnect(passwordTextBox[index],
                   SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                   this,
                   SLOT(slotTextBoxLoadInfopage (int,INFO_MSG_TYPE_e)));

        delete passwordTextBox[index];
        delete passwordParam[index];
    }
}


void ModifyPassword::changePassword(QString devName)
{
    m_payloadLib->setCnfgArrayAtIndex (0, newPassword);

    QString payloadString = m_payloadLib->createDevCmdPayload(1);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = CNG_PWD;
    param->payload = payloadString;

    processBar->loadProcessBar();

    if(m_applController->processActivity(devName, DEVICE_COMM, param) == true)
    {
        m_currentDeviceName = devName;
    }
}

void ModifyPassword::getOldPassword(QString devName)
{
    m_applController->getPasswordFrmDev(devName, oldPasswordValue);
}

void ModifyPassword :: processDeviceResponse(DevCommParam *param,
                                             QString deviceName)
{
    QString str = "";

    if ((deviceName == m_currentDeviceName)
            && (param->msgType == MSG_SET_CMD)
            && (param->cmdType == CNG_PWD))
    {
        switch(param->deviceStatus)
        {
            case CMD_SUCCESS:
            {
                m_payloadLib->parseDevCmdReply(true, param->payload);
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(MODIFY_PASS_CHANGE_SUCCESS));
                getOldPassword (deviceName);
            }
            break;

            case CMD_USER_ACCOUNT_LOCK:
            {
                m_payloadLib->parseDevCmdReply (false,param->payload);
                str = USER_ACC_LOCKED_DUE_TO_FAIL_ATTEMPT_MSG(m_payloadLib->getCnfgArrayAtIndex(0).toUInt());
            }
            break;

            case CMD_MIN_PASSWORD_CHAR_REQUIRED:
            {
                m_payloadLib->parseDevCmdReply (false,param->payload);
                str = USER_PASSWROD_MIN_LEN_MSG(m_payloadLib->getCnfgArrayAtIndex(1).toUInt());
            }
            break;

            default:
            {
                str = ValidationMessage::getDeviceResponceMessage(param->deviceStatus);
            }
            break;
        }
    }
    processBar->unloadProcessBar();

    if(str != "")
    {
        infoPage->loadInfoPage(str);
    }
}

void ModifyPassword::slotOkButtonClick(int)
{
    passwordTextBox[0]->getInputText (oldPassword);
    passwordTextBox[1]->getInputText (newPassword);
    passwordTextBox[2]->getInputText (confirmPassword);

    if(m_logButtonState == STATE_2)
    {
        if(oldPassword == "")
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(MODIFY_PASS_ENT_OLD_PASS));
            return;
        }
        if( oldPassword != oldPasswordValue )
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(MODIFY_PASS_INCORRECT_PASS));
            return;
        }
        else if(( newPassword != confirmPassword ) )
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(PASS_MISMATCH));
            return;
        }
        else
        {
            changePassword(m_currentDeviceName);
        }
    }
    else
    {
        infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(CMD_NO_PRIVILEGE));
    }
}

void ModifyPassword::slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e msgType)
{
    if( msgType == INFO_MSG_ERROR)
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(PASSWORD_RANGE_ERROR));
    }
}
