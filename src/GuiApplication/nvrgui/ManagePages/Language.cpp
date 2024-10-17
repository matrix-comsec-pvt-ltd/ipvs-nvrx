#include "Language.h"

Language::Language(QString devName,
                           QWidget *parent)
    :ManageMenuOptions(devName, parent, MAX_DEV_ALARM)
{
    QMap<quint8, QString> lanStringList;
    lanStringList.clear ();

    m_languageDropDownBox = new DropDown(((this->width() - BGTILE_SMALL_SIZE_WIDTH) / 2) ,
                                         ((this->height()) -(140))/2,
                                         SCALE_HEIGHT(410),
                                         SCALE_HEIGHT(40),
                                         0,
                                         DROPDOWNBOX_SIZE_200,
                                         "Language",
                                         lanStringList,
                                         this, "", false, SCALE_WIDTH(60));
    m_elementList[0] = m_languageDropDownBox;

    connect(m_languageDropDownBox,
            SIGNAL(sigValueChanged(QString,quint32)),
            this,
            SLOT(slotDropDownBoxValueChanged(QString,quint32)));
    connect (m_languageDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    this->show();
    getLanguage();
}

Language :: ~Language()
{
    if(m_languageDropDownBox != NULL)
    {
        disconnect(m_languageDropDownBox,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotDropDownBoxValueChanged(QString,quint32)));
        disconnect(m_languageDropDownBox,
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_languageDropDownBox);
    }

}

void Language::slotDropDownBoxValueChanged(QString string, quint32)
{
    USRS_GROUP_e currentUserType = VIEWER;
    selectedLangStr = string;
    m_applController->GetUserGroupType(m_currentDeviceName, currentUserType);
    if(currentUserType == ADMIN)
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(CHANGE_PREFEREED_LANGUAGE),true,false,"",CONFORMATION_BTN_YES,CONFORMATION_BTN_NO);
        return;
    }

    if(m_currentDeviceName == LOCAL_DEVICE_NAME)
    {
        emit sigLanguageCfgChanged(selectedLangStr);
    }
}

void Language::handleInfoPageMessage(int index)
{

    if(m_currentDeviceName == LOCAL_DEVICE_NAME)
    {
        emit sigLanguageCfgChanged(selectedLangStr);
    }

    if(index == INFO_OK_BTN)
    {
        m_payloadLib->setCnfgArrayAtIndex(0, selectedLangStr);
        setUserPreferredLanguage();
    }
}

void Language::getLanguage(void)
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = GET_LANGUAGE;
    m_applController->processActivity(m_currentDeviceName, DEVICE_COMM, param);
}

void Language::getUserPreferredLanguage(void)
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = GET_USER_LANGUAGE;
    m_applController->processActivity(m_currentDeviceName, DEVICE_COMM, param);
}

void Language::setUserPreferredLanguage(void)
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = SET_USER_LANGUAGE;
    param->payload = m_payloadLib->createDevCmdPayload(1);
    processBar->loadProcessBar();
    m_applController->processActivity(m_currentDeviceName, DEVICE_COMM, param);
}

void Language::processDeviceResponse(DevCommParam *param, QString devName)
{
    processBar->unloadProcessBar ();
    if ((devName != m_currentDeviceName) || (param->deviceStatus != CMD_SUCCESS) || (param->msgType != MSG_SET_CMD))
    {
        return;
    }

    quint8 tDeviceIndex = 0;
    m_applController->getDeviceIndex(m_currentDeviceName, tDeviceIndex);

    if(param->cmdType == GET_LANGUAGE)
    {
        QMap<quint8, QString> lanStringList;
        lanStringList.clear();

        m_payloadLib->parseDevCmdReply(true, param->payload);

        for(quint8 tIndex = 0; tIndex < MAX_SYSTEM_LANGUAGE; tIndex++)
        {
            lanStringList.insert(tIndex, m_payloadLib->getCnfgArrayAtIndex(tIndex).toString());
        }

        m_languageDropDownBox->setNewList(lanStringList);
        getUserPreferredLanguage();
    }
    else if(param->cmdType == GET_USER_LANGUAGE)
    {
        m_payloadLib->parseDevCmdReply(true, param->payload);
        m_languageDropDownBox->setCurrValue(m_payloadLib->getCnfgArrayAtIndex(0).toString());
    }
}
