#include "ManagePasswordRecovery.h"
#include "Controls/MessageBanner.h"
#include "ValidationMessage.h"

#define LEFT_MARGIN_FROM_CENTER     SCALE_WIDTH(110)

ManagePasswordRecovery::ManagePasswordRecovery(QString devName, QWidget *parent) : ManageMenuOptions(devName, parent, PWD_RECOVERY_STRINGS_MAX)
{
    for (quint8 index = 0; index < PASSWORD_RECOVERY_QA_MAX; index++)
    {
        INIT_OBJ(m_secQuestionDropDown[index]);
        INIT_OBJ(m_answerTextboxParam[index]);
        INIT_OBJ(m_answerTextBox[index]);
    }

    INIT_OBJ(m_emailIdTextboxParam);
    INIT_OBJ(m_emailIdTextBox);
    INIT_OBJ(m_testButton);
    INIT_OBJ(m_okButton);
    INIT_OBJ(m_cancelButton);

    m_emailIdTextboxParam = new TextboxParam();
    m_emailIdTextboxParam->labelStr = passwordRecoveryStrings[EMAIL_ID];
    m_emailIdTextboxParam->maxChar = 50;
    m_emailIdTextboxParam->isEmailAddrType = true;
    m_emailIdTextboxParam->isTotalBlankStrAllow = true;

    m_emailIdTextBox = new TextBox(SCALE_WIDTH(35),
                                   (BGTILE_HEIGHT * EMAIL_ID),
                                   BGTILE_MEDIUM_SIZE_WIDTH + SCALE_WIDTH(150),
                                   BGTILE_HEIGHT,
                                   EMAIL_ID,
                                   TEXTBOX_ULTRAMEDIAM,
                                   this,
                                   m_emailIdTextboxParam,
                                   COMMON_LAYER, true, false,
                                   false, BGTILE_MEDIUM_SIZE_WIDTH/2 - SCALE_WIDTH(20));
    m_elementList[EMAIL_ID] = m_emailIdTextBox;
    connect(m_emailIdTextBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (m_emailIdTextBox,
                SIGNAL(sigLoadInfopage(int, INFO_MSG_TYPE_e)),
                this,
                SLOT(slotTextBoxInfoPage(int, INFO_MSG_TYPE_e)));

    m_testButton = new ControlButton(EMAIL_BUTTON_INDEX,
                                     BGTILE_MEDIUM_SIZE_WIDTH + SCALE_WIDTH(60),
                                     (BGTILE_HEIGHT * EMAIL_ID),
                                     SCALE_WIDTH(50),
                                     BGTILE_HEIGHT,
                                     this, NO_LAYER, SCALE_WIDTH(5),
                                     passwordRecoveryStrings[TEST_BUTTON], true,
                                     TEST_BUTTON);
    m_elementList[TEST_BUTTON] = m_testButton;
    connect(m_testButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (m_testButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotTestBtnClick(int)));

     QMap<quint8, QString> securityQuestionsList;
     quint8 listIndex = 0;
     quint16 xPos, yPos, stringIndex;
     for (quint8 index = 0; index < PASSWORD_RECOVERY_QA_MAX; index++)
     {
        stringIndex = ((SECURITY_QUESTION_2 - SECURITY_QUESTION_1) * index) + SECURITY_QUESTION_1;
        xPos = m_emailIdTextBox->x();
        yPos = (index == PASSWORD_RECOVERY_QA_1) ? (m_emailIdTextBox->y() + BGTILE_HEIGHT + SCALE_HEIGHT(5)) : m_answerTextBox[index - 1]->y() + BGTILE_HEIGHT;

        listIndex = 0;
        for (quint8 secIndex = 0; secIndex < MAX_SEC_QUESTIONS; secIndex++)
        {
            if ((secIndex < PASSWORD_RECOVERY_QA_MAX) && (secIndex != index))
            {
                continue;
            }
            securityQuestionsList.insert(listIndex++, securityQuestionStrings[secIndex]);
        }

        m_secQuestionDropDown[index] = new DropDown(xPos,
                                                    yPos,
                                                    BGTILE_MEDIUM_SIZE_WIDTH + SCALE_WIDTH(150),
                                                    BGTILE_HEIGHT,
                                                    stringIndex,
                                                    DROPDOWNBOX_SIZE_405,
                                                    Multilang(passwordRecoveryStrings[stringIndex].toUtf8().constData()) + QString(" %1").arg(index+1),
                                                    securityQuestionsList,
                                                    this,
                                                    "",
                                                    true,
                                                    0,
                                                    COMMON_LAYER,
                                                    true,
                                                    5,
                                                    false,
                                                    (index == 2) ? true : false,
                                                    15, LEFT_MARGIN_FROM_CENTER);

         m_elementList[stringIndex] = m_secQuestionDropDown[index];
         connect (m_secQuestionDropDown[index],
                  SIGNAL(sigValueChanged(QString,quint32)),
                  this,
                  SLOT(slotSpinBoxValueChange(QString,quint32)));
         connect(m_secQuestionDropDown[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));

         m_answerTextboxParam[index] = new TextboxParam();
         m_answerTextboxParam[index]->maxChar = 25;
         m_answerTextboxParam[index]->minChar = 1;
         m_answerTextboxParam[index]->isTotalBlankStrAllow = true;
         m_answerTextboxParam[index]->labelStr = Multilang(passwordRecoveryStrings[stringIndex + 1].toUtf8().constData()) + QString(" %1").arg(index+1);

         m_answerTextBox[index] = new TextBox(xPos,
                                              yPos + BGTILE_HEIGHT,
                                              BGTILE_MEDIUM_SIZE_WIDTH + SCALE_WIDTH(150),
                                              BGTILE_HEIGHT,
                                              stringIndex + 1,
                                              TEXTBOX_ULTRAMEDIAM,
                                              this,
                                              m_answerTextboxParam[index],
                                              COMMON_LAYER, true, false, false,
                                              LEFT_MARGIN_FROM_CENTER);

         m_elementList[stringIndex + 1] = m_answerTextBox[index];
         connect(m_answerTextBox[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));

     }
     m_okButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                 m_answerTextBox[PASSWORD_RECOVERY_QA_3]->x() + SCALE_WIDTH(250),
                                 (m_answerTextBox[PASSWORD_RECOVERY_QA_3]->y() + SCALE_HEIGHT(61)),
                                 passwordRecoveryStrings[OK_BUTTON],
                                 this,
                                 OK_BUTTON);

     m_elementList[OK_BUTTON] = m_okButton;
     connect (m_okButton,
              SIGNAL(sigButtonClick(int)),
              this,
              SLOT(slotConfigButtonClick(int)));
     connect (m_okButton,
              SIGNAL(sigUpdateCurrentElement(int)),
              this,
              SLOT(slotUpdateCurrentElement(int)));

     m_cancelButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                     m_answerTextBox[PASSWORD_RECOVERY_QA_3]->x() + SCALE_WIDTH(400),
                                     (m_answerTextBox[PASSWORD_RECOVERY_QA_3]->y() + SCALE_HEIGHT(61)),
                                     passwordRecoveryStrings[CANCEL_BUTTON],
                                     this,
                                     CANCEL_BUTTON);

     m_elementList[CANCEL_BUTTON] = m_cancelButton;
     connect (m_cancelButton,
              SIGNAL(sigButtonClick(int)),
              this,
              SLOT(slotConfigButtonClick(int)));
     connect (m_cancelButton,
              SIGNAL(sigUpdateCurrentElement(int)),
              this,
              SLOT(slotUpdateCurrentElement(int)));

     getConfig();
}

ManagePasswordRecovery::~ManagePasswordRecovery()
{
    disconnect(m_emailIdTextBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect (m_emailIdTextBox,
                SIGNAL(sigLoadInfopage(int, INFO_MSG_TYPE_e)),
                this,
                SLOT(slotTextBoxInfoPage(int, INFO_MSG_TYPE_e)));
    DELETE_OBJ(m_emailIdTextBox);
    DELETE_OBJ(m_emailIdTextboxParam);

    disconnect(m_testButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    disconnect (m_testButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotTestBtnClick(int)));
    DELETE_OBJ(m_testButton);

    for (quint8 index = 0; index < PASSWORD_RECOVERY_QA_MAX; index++)
    {
        disconnect(m_secQuestionDropDown[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_secQuestionDropDown[index],
                   SIGNAL(sigValueChanged(QString, quint32)),
                   this,
                   SLOT(slotSpinBoxValueChange(QString, quint32)));
        DELETE_OBJ(m_secQuestionDropDown[index]);

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
}

void ManagePasswordRecovery::processDeviceResponse(DevCommParam* param, QString deviceName)
{
    processBar->unloadProcessBar();
    if(deviceName != m_currentDeviceName)
    {
        return;
    }
    if (param->deviceStatus != CMD_SUCCESS)
    {
        infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
        return;
    }

    switch(param->msgType)
    {
        case MSG_SET_CMD:
        {
            switch(param->cmdType)
            {
                case TEST_EMAIL_ID:
                    infoPage->loadInfoPage(ValidationMessage::getValidationMessage(EMAIL_CLIENT_MAIL_SENT_SUCCESS));
                    break;

                case GET_PWD_RST_INFO:
                     /* on success display the config info */
                    displayPwdRecoveryConfig(param);
                    break;

                case SET_PWD_RST_INFO:
                    MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));

                    /* on sucess send the command to get the PWd reset info from config */
                    getConfig();
                    break;

                default:
                    break;
            }
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void ManagePasswordRecovery::sendCommand(SET_COMMAND_e cmdType, int totalfeilds)
{
    /* Create payload to send cmd */
    QString payloadString = m_payloadLib->createDevCmdPayload(totalfeilds);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = cmdType;
    param->payload = payloadString;
    processBar->loadProcessBar();
    m_applController->processActivity(m_currentDeviceName, DEVICE_COMM, param);
}

void ManagePasswordRecovery::slotSpinBoxValueChange(QString, quint32 indexInPage)
{
    switch(indexInPage)
    {
    case SECURITY_QUESTION_1:
        m_answerTextBox[PASSWORD_RECOVERY_QA_1]->setInputText("");
        break;

    case SECURITY_QUESTION_2:
        m_answerTextBox[PASSWORD_RECOVERY_QA_2]->setInputText("");
        break;

    case SECURITY_QUESTION_3:
        m_answerTextBox[PASSWORD_RECOVERY_QA_3]->setInputText("");
        break;

    default:
        break;
    }

    QMap<quint8, QString> securityQuestionsList;
    for (quint8 index = 0; index < PASSWORD_RECOVERY_QA_MAX; index++)
    {
        /* selected question should be omitted from the other questions list */
        updateList((PASSWORD_RECOVERY_QA_e)((index + 1) % PASSWORD_RECOVERY_QA_MAX),
                   (PASSWORD_RECOVERY_QA_e)((index + 2) % PASSWORD_RECOVERY_QA_MAX), securityQuestionsList);
        updateSecurityQuestionDropdownList((PASSWORD_RECOVERY_QA_e)index, securityQuestionsList);
    }
}

void ManagePasswordRecovery::updateList(PASSWORD_RECOVERY_QA_e dropDownIndex_1, PASSWORD_RECOVERY_QA_e dropDownIndex_2, QMap<quint8, QString> &securityQuestionsList)
{
    quint8 index, listIndex = 0;
    securityQuestionsList.clear();

    for (index = 0; index < MAX_SEC_QUESTIONS; index++)
    {
        if ((securityQuestionStrings[index] == m_secQuestionDropDown[dropDownIndex_1]->getCurrValue()) ||
                (securityQuestionStrings[index] == m_secQuestionDropDown[dropDownIndex_2]->getCurrValue()))
        {
            continue;
        }
        securityQuestionsList.insert(listIndex++, securityQuestionStrings[index]);
    }
}

void ManagePasswordRecovery::updateSecurityQuestionDropdownList(PASSWORD_RECOVERY_QA_e dropDownIndex, QMap<quint8, QString> securityQuestionsList)
{
    for (quint8 index = 0; index < securityQuestionsList.count(); index++)
    {
        if (m_secQuestionDropDown[dropDownIndex]->getCurrValue() == securityQuestionsList.value(index))
        {
            m_secQuestionDropDown[dropDownIndex]->setNewList(securityQuestionsList, index);
            break;
        }
    }
}

void ManagePasswordRecovery::slotTextBoxInfoPage(int index, INFO_MSG_TYPE_e msgType)
{
    if(index != EMAIL_ID_FIELD)
    {
        return;
    }

    if(msgType == INFO_MSG_ERROR)
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_VAILD_EMAIL_ADD));
    }
    else if (msgType == INFO_MSG_STRAT_CHAR)
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_FIRST_ALPH));
    }
}

void ManagePasswordRecovery::slotTestBtnClick(int)
{    
    if(m_emailIdTextboxParam->textStr == "")
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_EMAIL_ADD));
        return;
    }

    /* send the command to Test the Email before saving into config */
    m_payloadLib->setCnfgArrayAtIndex(EMAIL_ID_FIELD, m_emailIdTextBox->getInputText());
    sendCommand(TEST_EMAIL_ID, TEST_BUTTON);
}

void ManagePasswordRecovery::slotConfigButtonClick(int index)
{
    if (index == OK_BUTTON)
    {
        /* ok button clicked save the info into config */
        saveConfig();
    }
    else if (index == CANCEL_BUTTON)
    {
        /* if cancel button is clicked emit signal to close right panel */
        emit sigCancelbuttonClick();
    }
}

void ManagePasswordRecovery::displayPwdRecoveryConfig(DevCommParam* param)
{
    QMap<quint8, QString> securityQuestionsList;
    quint8 fieldValue[PASSWORD_RECOVERY_QA_MAX];
    quint8 listIndex, index, stringIndex, selectIndex;

    /* parse the payload to display the config info */
    m_payloadLib->parseDevCmdReply(true, param->payload);
    m_emailIdTextBox->setInputText(m_payloadLib->getCnfgArrayAtIndex(EMAIL_ID_FIELD).toString());
    fieldValue[PASSWORD_RECOVERY_QA_1] = m_payloadLib->getCnfgArrayAtIndex(SECURITY_QUESTION_1_FIELD).toInt();
    m_answerTextBox[PASSWORD_RECOVERY_QA_1]->setInputText(m_payloadLib->getCnfgArrayAtIndex(SECURITY_ANSWER_1_FIELD).toString());
    fieldValue[PASSWORD_RECOVERY_QA_2] = m_payloadLib->getCnfgArrayAtIndex(SECURITY_QUESTION_2_FIELD).toInt();
    m_answerTextBox[PASSWORD_RECOVERY_QA_2]->setInputText(m_payloadLib->getCnfgArrayAtIndex(SECURITY_ANSWER_2_FIELD).toString());
    fieldValue[PASSWORD_RECOVERY_QA_3] = m_payloadLib->getCnfgArrayAtIndex(SECURITY_QUESTION_3_FIELD).toInt();
    m_answerTextBox[PASSWORD_RECOVERY_QA_3]->setInputText(m_payloadLib->getCnfgArrayAtIndex(SECURITY_ANSWER_3_FIELD).toString());

    selectIndex = fieldValue[PASSWORD_RECOVERY_QA_1];
    for (index = 0; index < PASSWORD_RECOVERY_QA_MAX; index++)
    {
        listIndex = 0;
        securityQuestionsList.clear();
        for (stringIndex = 0; stringIndex < MAX_SEC_QUESTIONS; stringIndex++)
        {
            /* selected question should be omitted from the other questions list */
            if ((stringIndex == fieldValue[(index + 1) % PASSWORD_RECOVERY_QA_MAX]) ||
                    (stringIndex == fieldValue[(index + 2) % PASSWORD_RECOVERY_QA_MAX]))
            {
                continue;
            }

            if (fieldValue[index] == stringIndex)
            {
                /* To update the dropdown list at selected index */
                selectIndex = listIndex;
            }

            securityQuestionsList.insert(listIndex++, securityQuestionStrings[stringIndex]);
        }
        m_secQuestionDropDown[index]->setNewList(securityQuestionsList, selectIndex);
    }
}

void ManagePasswordRecovery::getConfig(void)
{
    /* send the command to get the PWd reset info from config */
    sendCommand(GET_PWD_RST_INFO, PASSWORD_RECOVERY_QA_1);
}

void ManagePasswordRecovery::saveConfig(void)
{
    if ((m_emailIdTextboxParam->textStr != "") && (!m_emailIdTextBox->doneKeyValidation()))
    {
        return;
    }

    quint8 qaCount = 0;
    for (quint8 index = 0; index < PASSWORD_RECOVERY_QA_MAX; index++)
    {
        if (m_answerTextboxParam[index]->textStr != "")
        {
            qaCount++;
        }
    }

    /* when qaCount is greater than 0 and less than PASSWORD_RECOVERY_QA_MAX than validate message */
    if ((qaCount) && (qaCount < PASSWORD_RECOVERY_QA_MAX))
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_ALL_QA));
        return;
    }

    /* when no email is entered and qaCount is 0 than validate message */
    if ((m_emailIdTextboxParam->textStr == "") && (qaCount == 0))
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_EMAIL_ID_OR_ALL_QA));
        return;
    }

    /* update the payload on success */
    m_payloadLib->setCnfgArrayAtIndex(EMAIL_ID_FIELD, m_emailIdTextBox->getInputText());
    for (quint8 index = 0; index < PASSWORD_RECOVERY_QA_MAX; index++)
    {
        for (quint8 stringIndex = 0; stringIndex < MAX_SEC_QUESTIONS; stringIndex++)
        {
            if (securityQuestionStrings[stringIndex] == m_secQuestionDropDown[index]->getCurrValue())
            {
                 m_payloadLib->setCnfgArrayAtIndex(((2 * index) + SECURITY_QUESTION_1_FIELD), stringIndex);
                break;
            }
        }
    }
    m_payloadLib->setCnfgArrayAtIndex(SECURITY_ANSWER_1_FIELD, m_answerTextBox[PASSWORD_RECOVERY_QA_1]->getInputText());
    m_payloadLib->setCnfgArrayAtIndex(SECURITY_ANSWER_2_FIELD, m_answerTextBox[PASSWORD_RECOVERY_QA_2]->getInputText());
    m_payloadLib->setCnfgArrayAtIndex(SECURITY_ANSWER_3_FIELD, m_answerTextBox[PASSWORD_RECOVERY_QA_3]->getInputText());

    /* send the command to save the PWD reset info to config  */
    sendCommand(SET_PWD_RST_INFO, PWD_RECOVERY_FIELD_MAX);
}
