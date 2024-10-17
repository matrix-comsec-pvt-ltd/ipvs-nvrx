#include "Login/PasswordRecovery.h"
#include "Controls/MessageBanner.h"
#include "ValidationMessage.h"

PasswordRecovery::PasswordRecovery(QWidget *parent) :
    BackGround((ApplController::getXPosOfScreen() + SCALE_WIDTH(620)) ,
               (ApplController::getYPosOfScreen() + SCALE_HEIGHT(325)),
               SCALE_WIDTH(680),
               SCALE_HEIGHT(445),
               BACKGROUND_TYPE_4,
               LOG_BUTTON,
               parent,
               false,
               "Password Recovery")
{
    for (quint8 index = 0; index < PASSWORD_RECOVERY_QA_MAX; index++)
    {
        INIT_OBJ(m_secQuestionDropDown[index]);
        INIT_OBJ(m_answerTextboxParam[index]);
        INIT_OBJ(m_answerTextBox[index]);
    }

    INIT_OBJ(m_emailIdTextboxParam);
    INIT_OBJ(m_emailIdTextBox);
    INIT_OBJ(m_heading);
    INIT_OBJ(m_okButton);
    INIT_OBJ(m_cancelButton);
    INIT_OBJ(m_payloadLib);
    INIT_OBJ(m_processBar);
    INIT_OBJ(m_infoPage);
    INIT_OBJ(m_closeButton);

    m_applController = ApplController::getInstance();
    m_currentDeviceName = LOCAL_DEVICE_NAME;

    m_emailIdTextboxParam = new TextboxParam();
    m_emailIdTextboxParam->labelStr = passwordRecoveryStrings[EMAIL_ID];
    m_emailIdTextboxParam->maxChar = 50;
    m_emailIdTextboxParam->isCentre = false;
    m_emailIdTextboxParam->isEmailAddrType = true;
    m_emailIdTextboxParam->isTotalBlankStrAllow = true;
    m_emailIdTextboxParam->leftMargin = SCALE_WIDTH(24);

    m_emailIdTextBox = new TextBox(SCALE_WIDTH(22),
                                   SCALE_HEIGHT(50),
                                   BGTILE_MEDIUM_SIZE_WIDTH + SCALE_WIDTH(150),
                                   BGTILE_HEIGHT,
                                   EMAIL_ID,
                                   TEXTBOX_ULTRAMEDIAM,
                                   this,
                                   m_emailIdTextboxParam);
    m_elementList[EMAIL_ID] = m_emailIdTextBox;
    connect(m_emailIdTextBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (m_emailIdTextBox,
                SIGNAL(sigLoadInfopage(int, INFO_MSG_TYPE_e)),
                this,
                SLOT(slotTextBoxInfoPage(int, INFO_MSG_TYPE_e)));

    m_closeButton = new CloseButtton(SCALE_WIDTH(645),
                                     SCALE_HEIGHT(30),
                                     this,
                                     CLOSE_BTN_TYPE_1,
                                     CLOSE_BUTTON);
    m_elementList[CLOSE_BUTTON] = m_closeButton;
    connect(m_closeButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_closeButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotcloseButtonClick(int)));

    m_heading = new ElementHeading(m_emailIdTextBox->x(),
                                   (m_emailIdTextBox->y() + BGTILE_HEIGHT + SCALE_WIDTH(5)),
                                   BGTILE_MEDIUM_SIZE_WIDTH + SCALE_WIDTH(150),
                                   BGTILE_HEIGHT,
                                   "Security Questions",
                                   TOP_LAYER,
                                   this,
                                   false,
                                   SCALE_WIDTH(20), NORMAL_FONT_SIZE, true);

     QMap<quint8, QString> securityQuestionsList;
     quint8 listIndex = 0;
     quint16 xPos, yPos, stringIndex;
     for (quint8 index = 0; index < PASSWORD_RECOVERY_QA_MAX; index++)
     {
        stringIndex = ((SECURITY_QUESTION_2 - SECURITY_QUESTION_1) * index) + SECURITY_QUESTION_1;
        xPos = m_heading->x();
        yPos = (index == PASSWORD_RECOVERY_QA_1) ? (m_heading->y() + BGTILE_HEIGHT) : m_answerTextBox[index - PASSWORD_RECOVERY_QA_2]->y() + BGTILE_HEIGHT;

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
                                                    passwordRecoveryStrings[stringIndex] + QString(" %1").arg(index+1),
                                                    securityQuestionsList,
                                                    this,
                                                    "",
                                                    false,
                                                    SCALE_WIDTH(30),
                                                    BOTTOM_TABLE_LAYER,
                                                    true,
                                                    5,
                                                    false,
                                                    (index == 2) ? true : false,
                                                    15);

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
         m_answerTextboxParam[index]->isCentre = false;
         m_answerTextboxParam[index]->isTotalBlankStrAllow = true;
         m_answerTextboxParam[index]->leftMargin = SCALE_WIDTH(113);
         m_answerTextboxParam[index]->labelStr = passwordRecoveryStrings[stringIndex + 1] + QString(" %1").arg(index+1);

         m_answerTextBox[index] = new TextBox(xPos,
                                              yPos + BGTILE_HEIGHT,
                                              BGTILE_MEDIUM_SIZE_WIDTH + SCALE_WIDTH(150),
                                              BGTILE_HEIGHT,
                                              stringIndex + 1,
                                              TEXTBOX_ULTRAMEDIAM,
                                              this,
                                              m_answerTextboxParam[index],
                                              BOTTOM_TABLE_LAYER);

         m_elementList[stringIndex + 1] = m_answerTextBox[index];
         connect(m_answerTextBox[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
     }

     m_okButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                 m_answerTextBox[PASSWORD_RECOVERY_QA_3]->x() + SCALE_WIDTH(250),
                                 (m_answerTextBox[PASSWORD_RECOVERY_QA_3]->y() + SCALE_HEIGHT(75)),
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
                                     (m_answerTextBox[PASSWORD_RECOVERY_QA_3]->y() + SCALE_HEIGHT(75)),
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

     m_processBar = new ProcessBar(this->x(), this->y(), this->width(), this->height(), SCALE_WIDTH(0), parent);

     m_payloadLib = new PayloadLib();

     m_infoPage = new InfoPage(this->x(), this->y(), this->width(), this->height(), INFO_CONFIG_PAGE, parent, false, false);

     m_currentElement = EMAIL_ID;
     m_elementList[m_currentElement]->forceActiveFocus();
     this->show();
}

PasswordRecovery::~PasswordRecovery()
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
    DELETE_OBJ(m_heading);

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

    DELETE_OBJ(m_payloadLib);
    DELETE_OBJ(m_processBar);
    DELETE_OBJ(m_infoPage);
}

void PasswordRecovery::processDeviceResponse(DevCommParam* param, QString deviceName)
{
    m_processBar->unloadProcessBar();
    if(deviceName != m_currentDeviceName)
    {
        return;
    }

    if (param->deviceStatus != CMD_SUCCESS)
    {
        m_infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
        return;
    }

    if ((param->msgType == MSG_SET_CMD) && (param->cmdType == SET_PWD_RST_INFO))
    {
        /* Display the save config message */
        MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));

        /* emit signal to close window if status is success */
        emit sigExitPage();
    }
}

void PasswordRecovery::sendCommand(SET_COMMAND_e cmdType, int totalfeilds)
{
    /* Create payload to send cmd */
    QString payloadString = m_payloadLib->createDevCmdPayload(totalfeilds);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = cmdType;
    param->payload = payloadString;
    m_processBar->loadProcessBar();
    m_applController->processActivity(m_currentDeviceName, DEVICE_COMM, param);
}

void PasswordRecovery::slotSpinBoxValueChange(QString, quint32 indexInPage)
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

void PasswordRecovery::updateList(PASSWORD_RECOVERY_QA_e dropDownIndex_1, PASSWORD_RECOVERY_QA_e dropDownIndex_2, QMap<quint8, QString> &securityQuestionsList)
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

void PasswordRecovery::updateSecurityQuestionDropdownList(PASSWORD_RECOVERY_QA_e dropDownIndex, QMap<quint8, QString> securityQuestionsList)
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

void PasswordRecovery::slotTextBoxInfoPage(int indexInPage, INFO_MSG_TYPE_e msgType)
{
    if(indexInPage != EMAIL_ID)
    {
        return;
    }

    if(msgType == INFO_MSG_ERROR)
    {
        m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_VAILD_EMAIL_ADD));
    }
    else if (msgType == INFO_MSG_STRAT_CHAR)
    {
        m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_FIRST_ALPH));
    }
}

void PasswordRecovery::takeLeftKeyAction()
{
    do
    {
        m_currentElement = (m_currentElement - 1 + PWD_RECOVERY_STRINGS_MAX) % PWD_RECOVERY_STRINGS_MAX;
    }while(!m_elementList[m_currentElement]->getIsEnabled());

    m_elementList[m_currentElement]->forceActiveFocus();
}

void PasswordRecovery::takeRightKeyAction()
{
    do
    {
        m_currentElement = (m_currentElement + 1) % PWD_RECOVERY_STRINGS_MAX;
    }while(!m_elementList[m_currentElement]->getIsEnabled());

    m_elementList[m_currentElement]->forceActiveFocus();
}

void PasswordRecovery::showEvent (QShowEvent *event)
{
    QWidget::showEvent (event);
    m_elementList[m_currentElement]->forceActiveFocus();
}

void PasswordRecovery::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void PasswordRecovery::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void PasswordRecovery::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void PasswordRecovery::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    m_currentElement = CLOSE_BUTTON;
    m_elementList[m_currentElement]->forceActiveFocus();
}

void PasswordRecovery::slotUpdateCurrentElement(int index)
{
    m_currentElement = index;
}

void PasswordRecovery::slotConfigButtonClick(int indexInPage)
{
    if (indexInPage == OK_BUTTON)
    {
        /* ok button clicked save the info into config */
        saveConfig();
    }
    else if (indexInPage == CANCEL_BUTTON)
    {
        /* if cancel button is clicked emit signal to close window */
        emit sigExitPage();
    }
}

void PasswordRecovery::saveConfig(void)
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

    /* when qaCount is greater than 0 and less than PWD_RECOVERY_QA_MAX than validate message */
    if ((qaCount) && (qaCount < PASSWORD_RECOVERY_QA_MAX))
    {
        m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_ALL_QA));
        return;
    }

    /* when no email is entered and qaCount is 0 than validate message */
    if ((m_emailIdTextboxParam->textStr == "") && (qaCount == 0))
    {
        m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_EMAIL_ID_OR_ALL_QA));
        return;
    }

    /* update the payload if all validation is success */
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

void PasswordRecovery::slotcloseButtonClick(int indexInPage)
{
    if (indexInPage == CLOSE_BUTTON)
    {
        /* when close button is clicked emit the signal to close window */
        emit sigExitPage();
    }
}
