#include "ManagePages.h"
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include "ValidationMessage.h"

//#define DEVIC_NOT_CONNECT_ERROR     "Device not connected."

typedef enum{
    MANAGE_HEADING_STRING,
    SPINBOX_HEADING_STRING,
    ONLINE_USER_STRING,
    BLOCK_USER_STRING,
    MAX_MANAGE_PAGE_STRING
}MANAGE_PAGE_STRING_e;

static const QString ManagePagesOptions[MAX_MANAGE_OPTION] = {"Users",
                                                              "System Control",
                                                              "Language",
                                                              "Modify Password",
                                                              "Alarm Output",
                                                              "Password Recovery"};

static const QString ManagePagesStrings[MAX_MANAGE_PAGE_STRING]= {"Manage",
                                                                  "Device",
                                                                  "Online Users",
                                                                  "Blocked Users"};

ManagePages::ManagePages(STATE_TYPE_e state, QWidget *parent) :
    BackGround((ApplController::getXPosOfScreen() + ((ApplController::getWidthOfScreen() - MANAGE_LEFT_PANEL_WIDTH) / 2)),
               (ApplController::getYPosOfScreen() + ((ApplController::getHeightOfScreen() - MANAGE_LEFT_PANEL_HEIGHT) / 2)),
               MANAGE_LEFT_PANEL_WIDTH,
               MANAGE_LEFT_PANEL_HEIGHT,
               BACKGROUND_TYPE_2,
               MANAGE_BUTTON,
               parent,
               MANAGE_RIGHT_PANEL_WIDTH,
               MANAGE_RIGHT_PANEL_HEIGHT,
               ManagePagesStrings[MANAGE_HEADING_STRING]), m_logState(state)
{
    m_manageMenuOptions = NULL;
    isUserPageCreated = false;
    isLanguagePageCreated = false;
    subElementFocusListEnable = false;
    isRightPanelOpen = false;
    isPasswordRecoveryPageCreated = false;
    m_subcurrentElement = 0;

    for(quint8 index = 0; index < MAX_MANAGE_CONTROL_ELEMENT; index++ )
    {
        m_elementList[index] = NULL;
    }

    for(quint8 index = 0; index < MAX_SUB_SETTING_CONTOL_ELEMENT; index++ )
    {
        m_subelementList[index] = NULL;
    }
    m_elementList[MNG_CLOSE_BTN] = m_mainCloseButton;
    m_mainCloseButton->setIndexInPage(MNG_CLOSE_BTN);
    connect(m_mainCloseButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    QMap<quint8, QString> deviceMapList;
    applController = ApplController::getInstance();
    applController->GetDevNameDropdownMapList(deviceMapList);
    m_currentDevName = applController->GetRealDeviceName(deviceMapList.value(0));
    applController->GetDeviceInfo(m_currentDevName, devTableInfo);

    deviceNameDropDown = new DropDown(SCALE_WIDTH(25),
                                      SCALE_HEIGHT(65),
                                      MANAGE_LEFT_PANEL_WIDTH,
                                      BGTILE_HEIGHT,
                                      MNG_DROPBOX_CTRL,
                                      DROPDOWNBOX_SIZE_200,
                                      ManagePagesStrings[SPINBOX_HEADING_STRING],
                                      deviceMapList,
                                      this,
                                      "",
                                      false,
                                      SCALE_WIDTH(5),
                                      NO_LAYER);

    m_elementList[MNG_DROPBOX_CTRL] = deviceNameDropDown;

    connect (deviceNameDropDown,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotSpinBoxValueChange(QString,quint32)));
    connect(deviceNameDropDown,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    for(quint8 index = 0; index < MAX_MANAGE_OPTION; index++)
    {
        m_ManagePageOptions[index] = new MenuButton(index,
                                                    SCALE_WIDTH(260),
                                                    SCALE_HEIGHT(30),
                                                    ManagePagesOptions[index],
                                                    this,
                                                    SCALE_WIDTH(20),
                                                    SCALE_WIDTH(26),
                                                    SCALE_HEIGHT(125),
                                                    (index + MNG_MANAGE_OPTION));
        connect(m_ManagePageOptions[index],
                SIGNAL(sigButtonClicked(int)),
                this,
                SLOT (slotManageOptions(int)));
        connect(m_ManagePageOptions[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        m_elementList[MNG_MANAGE_OPTION + index] =  m_ManagePageOptions[index];
    }

    /* get current logged in user */
    applController->getUsernameFrmDev(m_currentDevName, m_currLoginUser);

    /* Current logged in user is local then hide the modify password tab */
    if (m_currLoginUser == DEFAULT_LOGIN_USER)
    {
        m_ManagePageOptions[MANAGE_OPTION_MODIFY_PASSWORD]->setIsEnabled(false);
        for(quint8 index = MANAGE_OPTION_ALARAM_OUTPUT; index < MAX_MANAGE_OPTION; index++)
        {
            m_ManagePageOptions[index]->resetGeometry(0,-1);
        }
    }

    if((devTableInfo.alarms == 0) && (devTableInfo.sensors == 0))
    {
         m_ManagePageOptions[MANAGE_OPTION_ALARAM_OUTPUT]->setIsEnabled(false);
         for(quint8 index = MANAGE_OPTION_PASSWORD_RECOVERY; index < MAX_MANAGE_OPTION; index++)
         {
             m_ManagePageOptions[index]->resetGeometry(0,-1);
         }
    }

    /* Current logged in user is local then hide the password recovery tab. if not local user then show password recovery tab */
    m_ManagePageOptions[MANAGE_OPTION_PASSWORD_RECOVERY]->setVisible((m_currLoginUser == DEFAULT_LOGIN_USER) ? false : true);
    m_ManagePageOptions[MANAGE_OPTION_PASSWORD_RECOVERY]->setIsEnabled((m_currLoginUser == DEFAULT_LOGIN_USER) ? false : true);

    infoPage = NULL;
    infoPage = new InfoPage (0 ,
                             0,
                             this->window ()->width(),
                             this->window ()->height(),
                             MAX_INFO_PAGE_TYPE,
                             this->window (),
                             false,
                             false);
    connect (infoPage,
             SIGNAL(sigInfoPageCnfgBtnClick(int)),
             this,
             SLOT(slotInfoPageBtnclick(int)));

    m_currentElement = MNG_DROPBOX_CTRL;
    m_subelementList[0] = m_subCloseButton;
    m_elementList[m_currentElement]->forceActiveFocus();
    m_optionIndex = 0;

    this->show();
}

ManagePages:: ~ManagePages()
{
    disconnect(m_mainCloseButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    if(m_manageMenuOptions != NULL)
    {
        if(isUserPageCreated == true)
        {
            disconnect(m_manageMenuOptions,
                       SIGNAL (sigSubHeadingChange(bool)),
                       this,
                       SLOT (slotSubHeadingChange(bool)));
        }
        if(isLanguagePageCreated == true)
        {
            disconnect(m_manageMenuOptions,
                    SIGNAL (sigLanguageCfgChanged(QString)),
                    applController,
                    SLOT (slotLanguageCfgChanged(QString)));
            isLanguagePageCreated = false;
        }
        if(isPasswordRecoveryPageCreated == true)
        {
            disconnect(m_manageMenuOptions,
                       SIGNAL(sigCancelbuttonClick(void)),
                       this,
                       SLOT(slotCancelbuttonClick(void)));
            isPasswordRecoveryPageCreated = false;
        }
        delete m_manageMenuOptions;
        m_manageMenuOptions = NULL;
    }

    for(quint8 index = 0; index < MAX_MANAGE_OPTION; index++)
    {
        disconnect(m_ManagePageOptions[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));

        disconnect(m_ManagePageOptions[index],
                   SIGNAL(sigButtonClicked(int)),
                   this,
                   SLOT (slotManageOptions(int)));
        delete m_ManagePageOptions[index];
    }

    disconnect(deviceNameDropDown,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));

    disconnect (deviceNameDropDown,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotSpinBoxValueChange(QString,quint32)));
    delete deviceNameDropDown;
    if(infoPage != NULL)
    {
        disconnect (infoPage,
                 SIGNAL(sigInfoPageCnfgBtnClick(int)),
                 this,
                 SLOT(slotInfoPageBtnclick(int)));
        DELETE_OBJ(infoPage);
    }
}

void ManagePages :: processDeviceResponse(DevCommParam *param,
                                          QString deviceName)
{

    if(m_manageMenuOptions != NULL)
    {
        m_manageMenuOptions->processDeviceResponse(param,deviceName);
    }
}

void ManagePages:: rightPanelClose()
{
    deleteSubPageObject();

    this->resetGeometry(((this->window()->width() - MANAGE_LEFT_PANEL_WIDTH) / 2));

    for (quint8 index = 0; index < MAX_MANAGE_OPTION; index ++)
    {
        m_ManagePageOptions[index]->setShowClickedImage(false);
    }

    isRightPanelOpen = false;
    m_elementList[m_currentElement]->forceActiveFocus();
}

void ManagePages::showEvent (QShowEvent *event)
{
    QWidget::showEvent (event);

    if(!isRightPanelOpen)
    {
        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

void ManagePages::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void ManagePages::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void ManagePages::navigationKeyPressed (QKeyEvent *event)
{
    event->accept();
}

void ManagePages::insertKeyPressed (QKeyEvent *event)
{
//    case Qt::Key_Insert:
//    case Qt::Key_F4:
//    case Qt::Key_Menu:
    event->accept();
    takeMenuKeyAction();
}

void ManagePages::escKeyPressed (QKeyEvent *event)
{
    event->accept();
    if(subElementFocusListEnable)
    {
        m_subcurrentElement = 0;
        m_subelementList[m_subcurrentElement]->forceActiveFocus();
    }
    else
    {
        m_currentElement = MNG_CLOSE_BTN;
        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

void ManagePages::hideEvent(QHideEvent * event)
{
    QWidget::hideEvent(event);
    if(infoPage->isVisible())
    {
        infoPage->unloadInfoPage();
    }
}

void ManagePages::takeLeftKeyAction()
{
    bool status = true;
    if((isRightPanelOpen == true) && (subElementFocusListEnable == true))
    {
        m_subcurrentElement = 1;
        m_subelementList[m_subcurrentElement]->forceFocusToPage(false);
    }
    else
    {
        do
        {
            if(m_currentElement == 0)
            {
                m_currentElement = (MAX_MANAGE_CONTROL_ELEMENT);
            }
            if(m_currentElement)
            {
                m_currentElement = (m_currentElement - 1);
            }
            else
            {
                status = false;
                break;
            }
        }while((m_elementList[m_currentElement] == NULL)
               ||(!m_elementList[m_currentElement]->getIsEnabled()));

        if(status == true)
        {
            m_elementList[m_currentElement]->forceActiveFocus();
        }
    }
}

void ManagePages::takeRightKeyAction()
{
    bool status = true;
    if((isRightPanelOpen == true) && (subElementFocusListEnable == true))
    {
            m_subcurrentElement = 1;
            m_subelementList[m_subcurrentElement]->forceFocusToPage(true);
    }
    else
    {
        do
        {
            if(m_currentElement == (MAX_MANAGE_CONTROL_ELEMENT - 1))
            {
                m_currentElement = -1;
            }
            if(m_currentElement != (MAX_MANAGE_CONTROL_ELEMENT - 1))
            {
                m_currentElement = (m_currentElement + 1);
            }
            else
            {
                status = false;
                break;
            }
        }while((m_elementList[m_currentElement] == NULL)
               ||(!m_elementList[m_currentElement]->getIsEnabled()));

        if(status == true)
        {
            m_elementList[m_currentElement]->forceActiveFocus();
        }
    }
}

void ManagePages::takeMenuKeyAction()
{
    if(isRightPanelOpen)
    {
        if(subElementFocusListEnable != true)
        {
            subElementFocusListEnable = true;
            m_subcurrentElement = 1;
            m_subelementList[m_subcurrentElement]->forceFocusToPage(true);
        }
        else
        {
            subElementFocusListEnable = false;
            m_elementList[m_currentElement]->forceActiveFocus();
        }
    }
}

void ManagePages::updatePage(QString deviceName, qint8 status, qint8 index,quint8 eventType,quint8 eventSubType)
{
    if(m_manageMenuOptions != NULL)
    {
        m_manageMenuOptions->updateStatus(deviceName, status, index,eventType,eventSubType);
    }
}

void ManagePages::loadSubPage(quint8 optionIndex)
{
    m_ManagePageOptions[m_optionIndex]->setShowClickedImage(false);
    m_ManagePageOptions[optionIndex]->setShowClickedImage(true);
    m_optionIndex =  optionIndex;

    deleteSubPageObject();
    createSubPageObject(optionIndex);

    m_subcurrentElement = 1;
    m_subelementList[m_subcurrentElement]  = m_manageMenuOptions;

    isRightPanelOpen = true;
    m_elementList[m_currentElement]->forceActiveFocus();

    if(optionIndex == MANAGE_OPTION_USERS)
    {
        changeSubPageHeading(ManagePagesStrings[ONLINE_USER_STRING]);
    }
    else
    {
        changeSubPageHeading(ManagePagesOptions[optionIndex]);
    }
}

void ManagePages::createSubPageObject(quint8 optionIndex)
{
    switch(optionIndex)
    {
        case MANAGE_OPTION_USERS:
        {
            m_manageMenuOptions = new Users(m_currentDevName, this);
            isUserPageCreated = true;
            connect(m_manageMenuOptions,
                    SIGNAL (sigSubHeadingChange(bool)),
                    this,
                    SLOT (slotSubHeadingChange(bool)));
        }
        break;

        case MANAGE_OPTION_ALARAM_OUTPUT:
        {
            m_manageMenuOptions = new AlaramOutput(m_currentDevName, this);
        }
        break;

        case MANAGE_OPTION_STSTEM_LANGUAGE:
        {
            m_manageMenuOptions = new Language(m_currentDevName, this);
            isLanguagePageCreated = true;
            connect(m_manageMenuOptions,
                    SIGNAL (sigLanguageCfgChanged(QString)),
                    applController,
                    SLOT (slotLanguageCfgChanged(QString)));
        }
        break;

        case MANAGE_OPTION_STSTEM_CONTROL:
        {
            m_manageMenuOptions = new SystemControl(m_currentDevName, this);
        }
        break;

        case MANAGE_OPTION_MODIFY_PASSWORD:
        {
            m_manageMenuOptions = new ModifyPassword(m_currentDevName, this, m_logState);
        }
        break;

        case MANAGE_OPTION_PASSWORD_RECOVERY:
        {
            /* create ManagePasswordRecovery sub page */
            m_manageMenuOptions = new ManagePasswordRecovery(m_currentDevName, this);
            isPasswordRecoveryPageCreated = true;
            connect(m_manageMenuOptions,
                    SIGNAL(sigCancelbuttonClick(void)),
                    this,
                    SLOT(slotCancelbuttonClick(void)));
        }
        break;
    }

    connect(m_manageMenuOptions,
            SIGNAL(sigFocusToOtherElement(bool)),
            this,
            SLOT(slotFocusToOtherElement(bool)));
}

void ManagePages::deleteSubPageObject()
{
    if(m_manageMenuOptions != NULL)
    {
        if(isUserPageCreated == true)
        {
            disconnect(m_manageMenuOptions,
                       SIGNAL (sigSubHeadingChange(bool)),
                       this,
                       SLOT (slotSubHeadingChange(bool)));
            isUserPageCreated = false;
        }
        if(isLanguagePageCreated == true)
        {
            disconnect(m_manageMenuOptions,
                    SIGNAL (sigLanguageCfgChanged(QString)),
                    applController,
                    SLOT (slotLanguageCfgChanged(QString)));
            isLanguagePageCreated = false;
        }
        if(isPasswordRecoveryPageCreated == true)
        {
            disconnect(m_manageMenuOptions,
                       SIGNAL(sigCancelbuttonClick(void)),
                       this,
                       SLOT(slotCancelbuttonClick(void)));
            isPasswordRecoveryPageCreated = false;
        }
        disconnect(m_manageMenuOptions,
                   SIGNAL(sigFocusToOtherElement(bool)),
                   this,
                   SLOT(slotFocusToOtherElement(bool)));
        delete m_manageMenuOptions;
        m_manageMenuOptions = NULL;
    }
}

void ManagePages::disconnectDeviceNotify (QString deviceName, bool forcePopup)
{
    QString dispDevName = deviceNameDropDown->getCurrValue();
    if(applController->GetRealDeviceName(deviceNameDropDown->getCurrValue()) == deviceName)
    {
        if(isRightPanelOpen == true)
        {
            hideRightpanel ();
            rightPanelClose ();
            infoPage->loadInfoPage (dispDevName + " " + ValidationMessage::getDeviceResponceMessage(CMD_DEV_DISCONNECTED));
        }
        else if(forcePopup == true)
        {
            infoPage->loadInfoPage (dispDevName + " " + ValidationMessage::getDeviceResponceMessage(CMD_DEV_DISCONNECTED));
        }
    }
}


void ManagePages:: slotSpinBoxValueChange(QString newName ,quint32)
{
    newName = applController->GetRealDeviceName(newName);
    applController->GetDeviceInfo(newName,devTableInfo);
    if(m_currentDevName == newName)
    {
        return;
    }

    m_currentDevName = newName;
    hideRightpanel();
    rightPanelClose();

    /* Current logged in user is local then hide the modify password tab */
    if (m_currLoginUser == DEFAULT_LOGIN_USER)
    {
        m_ManagePageOptions[MANAGE_OPTION_MODIFY_PASSWORD]->setIsEnabled(false);
        for(quint8 index = MANAGE_OPTION_ALARAM_OUTPUT; index < MAX_MANAGE_OPTION; index++)
        {
            m_ManagePageOptions[index]->resetGeometry(0,-1);
        }
    }
    else
    {
        m_ManagePageOptions[MANAGE_OPTION_MODIFY_PASSWORD]->setIsEnabled(true);
        for(quint8 index = MANAGE_OPTION_ALARAM_OUTPUT; index < MAX_MANAGE_OPTION; index++)
        {
            m_ManagePageOptions[index]->resetGeometry(0,0);
        }
    }

    if((devTableInfo.alarms == 0) && (devTableInfo.sensors == 0))
    {
        m_ManagePageOptions[MANAGE_OPTION_ALARAM_OUTPUT]->setIsEnabled(false);
        for(quint8 index = MANAGE_OPTION_PASSWORD_RECOVERY; index < MAX_MANAGE_OPTION; index++)
        {
            m_ManagePageOptions[index]->resetGeometry(0,-1);
        }
    }
    else
    {
        m_ManagePageOptions[MANAGE_OPTION_ALARAM_OUTPUT]->setIsEnabled(true);
        for(quint8 index = MANAGE_OPTION_PASSWORD_RECOVERY; index < MAX_MANAGE_OPTION; index++)
        {
            m_ManagePageOptions[index]->resetGeometry(0,0);
        }
    }

    // if communication mismatch or Product other than NVR-X or User type other than ADMIN, language option should be disable for that device.
    if((devTableInfo.deviceConflictType != MAX_MX_DEVICE_CONFLICT_TYPE) || (devTableInfo.productVariant < NVRX_SUPPORT_START_VARIANT)
            || ((m_currLoginUser != DEFAULT_LOGIN_USER) && (devTableInfo.userGroupType != ADMIN)))
    {
        m_ManagePageOptions[MANAGE_OPTION_STSTEM_LANGUAGE]->disableButton(true);
    }
    else
    {
        m_ManagePageOptions[MANAGE_OPTION_STSTEM_LANGUAGE]->disableButton(false);
    }

    /* if selected device is remote device then hide the password recovery tab.
     * if selected device is local device and current logged user is local then hide the password recovery tab.
     * if selected device is local device and current logged user is not local user then show the password recovery tab. */
    if((m_currentDevName != LOCAL_DEVICE_NAME) || ((m_currentDevName == LOCAL_DEVICE_NAME) && (m_currLoginUser == DEFAULT_LOGIN_USER)))
    {
        m_ManagePageOptions[MANAGE_OPTION_PASSWORD_RECOVERY]->setVisible(false);
        m_ManagePageOptions[MANAGE_OPTION_PASSWORD_RECOVERY]->setIsEnabled(false);
    }
    else
    {
        m_ManagePageOptions[MANAGE_OPTION_PASSWORD_RECOVERY]->setVisible(true);
        m_ManagePageOptions[MANAGE_OPTION_PASSWORD_RECOVERY]->setIsEnabled(true);
    }

    if(IS_VALID_OBJ(m_elementList[m_currentElement]))
    m_elementList[m_currentElement]->forceActiveFocus();
}

void ManagePages::slotManageOptions(int index)
{
    if(applController->GetDeviceInfo(m_currentDevName,devTableInfo))
    {
        this->resetGeometry(((this->window()->width() - MANAGE_LEFT_PANEL_WIDTH - MANAGE_RIGHT_PANEL_WIDTH) / 2 ));

        showRightpanel();
        loadSubPage((quint8)index);
    }
    else
    {
        m_optionIndex = index;
        disconnectDeviceNotify(m_currentDevName, true);
    }
}

void ManagePages::slotSubHeadingChange(bool isOnline)
{
    if(isOnline == true)
    {
        changeSubPageHeading(ManagePagesStrings[ONLINE_USER_STRING]);
    }
    else
    {
        changeSubPageHeading(ManagePagesStrings[BLOCK_USER_STRING]);
    }
}

void ManagePages::slotUpdateCurrentElement(int index)
{
    m_currentElement = index;
}

void ManagePages::slotInfoPageBtnclick(int)
{
    m_ManagePageOptions[m_optionIndex]->setShowClickedImage(false);
    m_elementList[m_currentElement]->forceActiveFocus();
}

void ManagePages::slotFocusToOtherElement(bool isPreviousElement)
{
    if(isPreviousElement)
    {
        takeLeftKeyAction();
    }
    else
    {
        takeRightKeyAction();
    }
}

void ManagePages::updateDeviceList(void)
{
    QMap<quint8, QString> deviceMapList;
    applController->GetDevNameDropdownMapList(deviceMapList);

    /* Check if selected device found in new updated list or not. It is possible that index of that device name may changed.
     * Hence update device list with current device index */
    for (quint8 deviceIndex = 0; deviceIndex < deviceMapList.count(); deviceIndex++)
    {
        if (deviceNameDropDown->getCurrValue() == deviceMapList.value(deviceIndex))
        {
            deviceNameDropDown->setNewList(deviceMapList, deviceIndex);
            return;
        }
    }

    /* If selected device is local device then it will update the device name only otherwise it will clear the data and will select the local device */
    deviceNameDropDown->setNewList(deviceMapList, 0);
    slotSpinBoxValueChange(deviceMapList.value(0), MNG_DROPBOX_CTRL);
}

void ManagePages::slotCancelbuttonClick(void)
{
    /* hide the right panel and close the rightpanel when cancel button is clicked from ManagePasswordRecovery page */
    hideRightpanel();
    rightPanelClose();
}
