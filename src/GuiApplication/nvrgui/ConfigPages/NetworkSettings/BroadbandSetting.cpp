#include "BroadbandSetting.h"
#include "ValidationMessage.h"

#define ACTIVE_BB_TABLE_INDEX       1
#define ACTIVE_BB_TABLE_FIELD       1
#define BB_TABLE_MAX_INDEX          5
#define LEFT_MARGIN_FROM_CENTER     SCALE_WIDTH(80)

// List of control
typedef enum
{
    BB_STG_ACTIVE_PROFILE,
    BB_STG_ACTIVE_PROF_STATUS,
    BB_STG_PROFILE_NO,
    BB_STG_PROFILE_NAME,
    BB_STG_DIAL_NO,
    BB_STG_USERNAME,
    BB_STG_PASSWORD,
    BB_STG_APN,
    MAX_BB_STG_ELEMENTS
}BB_SETTING_ELELIST_e;

// cnfg field no According to CMS comm. module
typedef enum
{
    FIELD_ACTIVE_PROFILE = 0,
    FIELD_PROFILE_NAME,
    FIELD_DIAL_NO,
    FIELD_USERNAME,
    FIELD_PASSWORD,
    FIELD_APN,
    MAX_FIELD_NO
}CNFG_FIELD_NO_e;

const static QString labelOfElements[MAX_BB_STG_ELEMENTS]=
{
    "Active Profile",
    "Status",
    "Profile Number",
    "Profile Name",
    "Dial Number",
    "Username",
    "Password",
    "APN"
};

BroadbandSetting::BroadbandSetting(QString devName, QWidget *parent)
    : ConfigPageControl(devName, parent, MAX_BB_STG_ELEMENTS)
{
    createDefaultComponent();
    broadbandStatus = NULL;
    m_initDone = false;
    m_currActiveProfIndex = 0;
    m_frmIndex = CNFG_FRM_INDEX;
    m_toIndex = BB_TABLE_MAX_INDEX;
    broadbandStatus = NULL;
    m_prevProfIndex = 0;
    m_currProfIndex = 0;
    BroadbandSetting::getConfig();
}

BroadbandSetting::~BroadbandSetting()
{
    disconnect(activeProfileDropDownBox,
               SIGNAL(sigValueChanged(QString,quint32)),
               this,
               SLOT(slotProfNumChanged(QString,quint32)));
    disconnect(activeProfileDropDownBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete activeProfileDropDownBox;

    disconnect(statusPageopenBtn,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotStatusPageButtonClick(int)));
    disconnect(statusPageopenBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete statusPageopenBtn;

    delete eleHeading;
    disconnect(profileNoDropDownBox,
               SIGNAL(sigValueChanged(QString,quint32)),
               this,
               SLOT(slotProfNumChanged(QString,quint32)));
    disconnect(profileNoDropDownBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete profileNoDropDownBox;

    disconnect(profileNameTextbox,
               SIGNAL(sigTextValueAppended(QString,int)),
               this,
               SLOT(slotProfileNameTextValueAppended(QString,int)));
    disconnect(profileNameTextbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete profileNameTextbox;
    delete profileNameParam;

    disconnect(dialNumberTextbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete dialNumberTextbox;
    delete dialNumberParam;

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

    disconnect(apnTextbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete apnTextbox;
    delete apnParam;

    if (IS_VALID_OBJ(broadbandStatus))
    {
        disconnect(broadbandStatus,
                   SIGNAL(sigStatusPageClosed()),
                   this,
                   SLOT(slotStatusPageClosed()));
        DELETE_OBJ(broadbandStatus);
    }
}

void BroadbandSetting::createDefaultComponent()
{
    activeProfileDropDownBox = new DropDown((this->width() - BGTILE_MEDIUM_SIZE_WIDTH)/2,
                                            SCALE_HEIGHT(100),
                                            BGTILE_MEDIUM_SIZE_WIDTH,
                                            BGTILE_HEIGHT,
                                            BB_STG_ACTIVE_PROFILE,
                                            DROPDOWNBOX_SIZE_200,
                                            labelOfElements[BB_STG_ACTIVE_PROFILE],
                                            defaultProfileList(),
                                            this, "", true, 0, COMMON_LAYER,
                                            true, 8, false, false, 5, LEFT_MARGIN_FROM_CENTER);
    m_elementList[BB_STG_ACTIVE_PROFILE] = activeProfileDropDownBox;
    connect(activeProfileDropDownBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(activeProfileDropDownBox,
            SIGNAL(sigValueChanged(QString,quint32)),
            this,
            SLOT(slotProfNumChanged(QString,quint32)));

    statusPageopenBtn = new PageOpenButton(activeProfileDropDownBox->x() + activeProfileDropDownBox->width() - SCALE_WIDTH(90),
                                           activeProfileDropDownBox->y(),
                                           BGTILE_MEDIUM_SIZE_WIDTH,
                                           BGTILE_HEIGHT,
                                           BB_STG_ACTIVE_PROF_STATUS,
                                           PAGEOPENBUTTON_MEDIAM_NEXT,
                                           labelOfElements[BB_STG_ACTIVE_PROF_STATUS],
                                           this, "", "", false, 0, NO_LAYER);
    m_elementList[BB_STG_ACTIVE_PROF_STATUS] = statusPageopenBtn;
    connect(statusPageopenBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(statusPageopenBtn,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotStatusPageButtonClick(int)));

    eleHeading = new ElementHeading(activeProfileDropDownBox->x(),
                                    activeProfileDropDownBox->y() + activeProfileDropDownBox->height() + SCALE_HEIGHT(6),
                                    BGTILE_MEDIUM_SIZE_WIDTH,
                                    BGTILE_HEIGHT,
                                    "Set Profile",
                                    TOP_LAYER, this, false, SCALE_WIDTH(25), NORMAL_FONT_SIZE, true);

    QMap<quint8, QString> profileNumList;
    for(quint8 index = 0; index < BB_TABLE_MAX_INDEX; index++)
    {
        profileNumList.insert(index, INT_TO_QSTRING(index+1));
    }

    profileNoDropDownBox = new DropDown(eleHeading->x(),
                                        eleHeading->y() + eleHeading->height(),
                                        BGTILE_MEDIUM_SIZE_WIDTH,
                                        BGTILE_HEIGHT,
                                        BB_STG_PROFILE_NO,
                                        DROPDOWNBOX_SIZE_90,
                                        labelOfElements[BB_STG_PROFILE_NO],
                                        profileNumList,
                                        this, "", true, 0, MIDDLE_TABLE_LAYER);
    m_elementList[BB_STG_PROFILE_NO] = profileNoDropDownBox;
    connect(profileNoDropDownBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(profileNoDropDownBox,
            SIGNAL(sigValueChanged(QString,quint32)),
            this,
            SLOT(slotProfNumChanged(QString,quint32)));

    profileNameParam = new TextboxParam();
    profileNameParam->labelStr = labelOfElements[BB_STG_PROFILE_NAME];
    profileNameParam->maxChar = 16;

    profileNameTextbox = new TextBox(profileNoDropDownBox->x(),
                                     profileNoDropDownBox->y() + profileNoDropDownBox->height(),
                                     BGTILE_MEDIUM_SIZE_WIDTH,
                                     BGTILE_HEIGHT,
                                     BB_STG_PROFILE_NAME,
                                     TEXTBOX_LARGE,
                                     this, profileNameParam, MIDDLE_TABLE_LAYER);
    m_elementList[BB_STG_PROFILE_NAME] = profileNameTextbox;
    connect(profileNameTextbox,
            SIGNAL(sigTextValueAppended(QString,int)),
            this,
            SLOT(slotProfileNameTextValueAppended(QString,int)));
    connect(profileNameTextbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    dialNumberParam = new TextboxParam();
    dialNumberParam->labelStr = labelOfElements[BB_STG_DIAL_NO];
    dialNumberParam->maxChar = 16;

    dialNumberTextbox = new TextBox(profileNameTextbox->x(),
                                    profileNameTextbox->y() + profileNameTextbox->height(),
                                    BGTILE_MEDIUM_SIZE_WIDTH,
                                    BGTILE_HEIGHT,
                                    BB_STG_DIAL_NO,
                                    TEXTBOX_LARGE,
                                    this, dialNumberParam, MIDDLE_TABLE_LAYER);
    m_elementList[BB_STG_DIAL_NO] = dialNumberTextbox;
    connect(dialNumberTextbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    usernameParam = new TextboxParam();
    usernameParam->labelStr = labelOfElements[BB_STG_USERNAME];
    usernameParam->maxChar = 40;
    usernameParam->isTotalBlankStrAllow = true;

    usernameTextbox = new TextBox(dialNumberTextbox->x(),
                                  dialNumberTextbox->y() + dialNumberTextbox->height(),
                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                  BGTILE_HEIGHT,
                                  BB_STG_USERNAME,
                                  TEXTBOX_LARGE,
                                  this, usernameParam, MIDDLE_TABLE_LAYER);
    m_elementList[BB_STG_USERNAME] = usernameTextbox;
    connect(usernameTextbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    passwordParam = new TextboxParam();
    passwordParam->labelStr = labelOfElements[BB_STG_PASSWORD];
    passwordParam->maxChar = 40;

    passwordTextbox = new PasswordTextbox(usernameTextbox->x(),
                                          usernameTextbox->y() + usernameTextbox->height(),
                                          BGTILE_MEDIUM_SIZE_WIDTH,
                                          BGTILE_HEIGHT,
                                          BB_STG_PASSWORD,
                                          TEXTBOX_LARGE,
                                          this, passwordParam, MIDDLE_TABLE_LAYER);
    m_elementList[BB_STG_PASSWORD] = passwordTextbox;
    connect(passwordTextbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    apnParam = new TextboxParam();
    apnParam->labelStr = labelOfElements[BB_STG_APN];
    apnParam->maxChar = 40;

    apnTextbox = new TextBox(passwordTextbox->x(),
                             passwordTextbox->y() + passwordTextbox->height(),
                             BGTILE_MEDIUM_SIZE_WIDTH,
                             BGTILE_HEIGHT,
                             BB_STG_APN,
                             TEXTBOX_LARGE,
                             this, apnParam, BOTTOM_TABLE_LAYER);
    m_elementList[BB_STG_APN] = apnTextbox;
    connect(apnTextbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
}

QMap<quint8, QString> BroadbandSetting:: defaultProfileList()
{
    QMap<quint8, QString> tempList;

    activeProfileList.clear();
    activeProfileList.insert(0, "Airtel");
    activeProfileList.insert(1, "BSNL");
    activeProfileList.insert(2, "VI");
    activeProfileList.insert(3, "Jio");
    activeProfileList.insert(4, "Custom");

    tempList.insert(0, "None");
    for(quint8 index = 0; index < activeProfileList.size(); index++)
    {
        tempList.insert((index+1), QString("%1").arg(index+1) + QString(": ") + activeProfileList.value(index));
    }

    return tempList;
}

void BroadbandSetting::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if (deviceName != currDevName)
    {
        return;
    }

    if (param->deviceStatus != CMD_SUCCESS)
    {
        processBar->unloadProcessBar();
        infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
        return;
    }

    switch(param->msgType)
    {
        case MSG_GET_CFG:
        {
            payloadLib->parsePayload(param->msgType, param->payload);
            if ((payloadLib->getcnfgTableIndex(0) != ACTIVE_BROADBAND_TABLE_INDEX)
                    || (payloadLib->getcnfgTableIndex(1) != BROADBAND_TABLE_INDEX))
            {
                break;
            }

            m_currActiveProfIndex = payloadLib->getCnfgArrayAtIndex(FIELD_ACTIVE_PROFILE).toUInt();
            profileNameTextbox->setInputText(payloadLib->getCnfgArrayAtIndex(FIELD_PROFILE_NAME).toString());
            dialNumberTextbox->setInputText(payloadLib->getCnfgArrayAtIndex(FIELD_DIAL_NO).toString());
            usernameTextbox->setInputText(payloadLib->getCnfgArrayAtIndex(FIELD_USERNAME).toString());
            passwordTextbox->setInputText(payloadLib->getCnfgArrayAtIndex(FIELD_PASSWORD).toString());
            apnTextbox->setInputText(payloadLib->getCnfgArrayAtIndex(FIELD_APN).toString());
            if(m_initDone == false)
            {
                quint8                  fieldIdx;
                QMap<quint8, QString>   newList;

                newList.insert(0, "None");
                activeProfileList.clear();
                for(quint8 index = 0; index < BB_TABLE_MAX_INDEX; index++)
                {
                    fieldIdx = (index * (MAX_FIELD_NO - 1)) + 1;
                    newList.insert((index+1), (QString("%1").arg(index+1) + ": " + payloadLib->getCnfgArrayAtIndex(fieldIdx).toString()));
                    activeProfileList.insert(index, payloadLib->getCnfgArrayAtIndex(fieldIdx).toString());
                }

                activeProfileDropDownBox->setNewList(newList);
                m_initDone = true;
            }

            activeProfileDropDownBox->setIndexofCurrElement(m_currActiveProfIndex);
            processBar->unloadProcessBar();

            if(m_currActiveProfIndex == 0)
            {
                if(statusPageopenBtn->hasFocus())
                {
                    m_currentElement = BB_STG_ACTIVE_PROFILE;
                    m_elementList[m_currentElement]->forceActiveFocus();
                }
                statusPageopenBtn->setIsEnabled(false);
            }
            else
            {
                if(!statusPageopenBtn->isEnabled())
                {
                    statusPageopenBtn->setIsEnabled(true);
                }
                m_elementList[m_currentElement]->forceActiveFocus();
            }
        }
        break;

        case MSG_SET_CFG:
        {
            processBar->unloadProcessBar();
            m_frmIndex = m_toIndex = (profileNoDropDownBox->getIndexofCurrElement() + 1);
            MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
            getConfig();
        }
        break;

        case MSG_DEF_CFG:
        {
            activeProfileDropDownBox->setNewList(defaultProfileList(), 0);
            getConfig();
        }
        break;

        case MSG_SET_CMD:
        {
            if(param->cmdType == MODEM_STS)
            {
                if (IS_VALID_OBJ(broadbandStatus))
                {
                    broadbandStatus->processDeviceResponse(param);
                }
            }
            processBar->unloadProcessBar();
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void BroadbandSetting::getModemStatus()
{
    DevCommParam *param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = MODEM_STS;
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void BroadbandSetting::getConfig()
{
    processBar->loadProcessBar();
    createPayload(MSG_GET_CFG);
}

void BroadbandSetting::defaultConfig()
{
    processBar->loadProcessBar();
    createPayload(MSG_DEF_CFG);
}

void BroadbandSetting::saveConfig()
{
    if(apnTextbox->getInputText() == "")
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(BROAD_SETT_ENT_APN));
        profileNoDropDownBox->setIndexofCurrElement(m_prevProfIndex);
    }
    else
    {
        QString str = profileNameTextbox->getInputText();
        quint8  index;

        for(index = 0; index < activeProfileList.count(); index++)
        {
            if(activeProfileList.value(index) == str)
            {
                break;
            }
        }

        if((index < activeProfileList.count()) && (index != profileNoDropDownBox->getIndexofCurrElement()))
        {
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(BROAD_SETT_PROF_ALREADY_EXISTS));
            profileNoDropDownBox->setIndexofCurrElement(m_prevProfIndex);
        }
        else
        {
            activeProfileList.insert(profileNoDropDownBox->getIndexofCurrElement(), profileNameTextbox->getInputText());

            // fill record
            payloadLib->setCnfgArrayAtIndex(FIELD_ACTIVE_PROFILE, activeProfileDropDownBox->getIndexofCurrElement());
            payloadLib->setCnfgArrayAtIndex(FIELD_PROFILE_NAME, profileNameTextbox->getInputText());
            payloadLib->setCnfgArrayAtIndex(FIELD_DIAL_NO, dialNumberTextbox->getInputText());
            payloadLib->setCnfgArrayAtIndex(FIELD_USERNAME, usernameTextbox->getInputText());
            payloadLib->setCnfgArrayAtIndex(FIELD_PASSWORD, passwordTextbox->getInputText());
            payloadLib->setCnfgArrayAtIndex(FIELD_APN, apnTextbox->getInputText());
            processBar->loadProcessBar();
            createPayload(MSG_SET_CFG);
        }      
    }
}

void BroadbandSetting::createPayload(REQ_MSG_ID_e requestType)
{
    QString payloadString = payloadLib->createDevCnfgPayload(requestType,
                                                             ACTIVE_BROADBAND_TABLE_INDEX,
                                                             ACTIVE_BB_TABLE_INDEX,
                                                             ACTIVE_BB_TABLE_INDEX,
                                                             ACTIVE_BB_TABLE_FIELD,
                                                             ACTIVE_BB_TABLE_FIELD,
                                                             ACTIVE_BB_TABLE_FIELD);

    payloadString = payloadLib->createDevCnfgPayload(requestType,
                                                     BROADBAND_TABLE_INDEX,
                                                     m_frmIndex,
                                                     m_toIndex,
                                                     CNFG_FRM_FIELD,
                                                     MAX_FIELD_NO-1,
                                                     MAX_FIELD_NO-1,
                                                     payloadString,
                                                     ACTIVE_BB_TABLE_FIELD);
    DevCommParam *param = new DevCommParam();
    param->msgType = requestType;
    param->payload = payloadString;
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void BroadbandSetting::slotProfNumChanged(QString str, quint32 index)
{
    switch(index)
    {
        case BB_STG_ACTIVE_PROFILE:
            statusPageopenBtn->setIsEnabled((str == "None") ? false : true);
            break;

        case BB_STG_PROFILE_NO:
            m_currProfIndex = profileNoDropDownBox->getIndexofCurrElement();
            checkDataChange();
            break;

        default:
            break;
    }
}

bool BroadbandSetting::checkDataChange()
{
    if ((m_currActiveProfIndex != activeProfileDropDownBox->getIndexofCurrElement())
            || (payloadLib->getCnfgArrayAtIndex(FIELD_PROFILE_NAME).toString() != profileNameTextbox->getInputText())
            || (payloadLib->getCnfgArrayAtIndex(FIELD_DIAL_NO).toString() != dialNumberTextbox->getInputText())
            || (payloadLib->getCnfgArrayAtIndex(FIELD_USERNAME).toString() != usernameTextbox->getInputText())
            || (payloadLib->getCnfgArrayAtIndex(FIELD_PASSWORD).toString() != passwordTextbox->getInputText())
            || (payloadLib->getCnfgArrayAtIndex(FIELD_APN).toString() != apnTextbox->getInputText()))
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(BROAD_SETT_SAVE_CURRENT_CHANGES), true);
        return false;
    }

    if (m_initDone == true)
    {
        m_prevProfIndex = m_currProfIndex;
        m_frmIndex = m_toIndex = (profileNoDropDownBox->getIndexofCurrElement() + 1);
        getConfig();
    }

    return true;
}

void BroadbandSetting::slotProfileNameTextValueAppended(QString str,int index)
{
    if (index == BB_STG_PROFILE_NAME)
    {
        quint8 tempIndex = profileNoDropDownBox->getCurrValue().toUInt();
        QString tempStr = QString("%1").arg(tempIndex) + ": " + str;
        activeProfileDropDownBox->changeTextAtIndex(tempIndex, tempStr);
    }
}
void BroadbandSetting::handleInfoPageMessage(int index)
{
    if(index == INFO_OK_BTN)
    {
        if(infoPage->getText() == ValidationMessage::getValidationMessage(BROAD_SETT_SAVE_CURRENT_CHANGES))
        {
            saveConfig();
        }
        else if (infoPage->getText() == ValidationMessage::getValidationMessage(BROAD_SETT_PROF_ALREADY_EXISTS))
        {
            QMap<quint8, QString> newList;

            newList.insert(0, "None");
            for(quint8 index = 0; index < BB_TABLE_MAX_INDEX; index++)
            {
                newList.insert((index + 1), (QString("%1").arg(index+1) + ": " + activeProfileList.value(index)));
            }

            activeProfileDropDownBox->setNewList(newList);
        }
    }
    else
    {
        if(infoPage->getText() == ValidationMessage::getValidationMessage(BROAD_SETT_SAVE_CURRENT_CHANGES))
        {
            m_prevProfIndex = m_currProfIndex;
            m_frmIndex = m_toIndex = (profileNoDropDownBox->getIndexofCurrElement() + 1);
            getConfig();
        }
    }
}

void BroadbandSetting::slotStatusPageButtonClick(int)
{
    if(checkDataChange() == true)
    {
        if(broadbandStatus == NULL)
        {
            broadbandStatus = new BroadbandStatus(parentWidget());
            connect(broadbandStatus,
                     SIGNAL(sigStatusPageClosed()),
                     this,
                     SLOT(slotStatusPageClosed()));
        }
        getModemStatus();
    }
}

void BroadbandSetting::slotStatusPageClosed()
{
    if (IS_VALID_OBJ(broadbandStatus))
    {
        disconnect(broadbandStatus,
                    SIGNAL(sigStatusPageClosed()),
                    this,
                    SLOT(slotStatusPageClosed()));
        DELETE_OBJ(broadbandStatus);
    }

    m_currentElement = BB_STG_ACTIVE_PROF_STATUS;
    m_elementList[m_currentElement]->forceActiveFocus();
}

