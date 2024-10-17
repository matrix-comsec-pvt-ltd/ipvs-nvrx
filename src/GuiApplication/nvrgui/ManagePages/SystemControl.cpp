#include "SystemControl.h"
#include "Controls/MessageBanner.h"
#include "ValidationMessage.h"

#define MAX_CHECK_BOX                   3
#define BGTILE_WIDTH                    SCALE_WIDTH(480)
#define MAX_CELL_ON_PAGE                7
#define MAX_USERS                       9
#define INNER_CONTROL_MARGIN            SCALE_WIDTH(5)

typedef enum {
    MANUAL_TRIGGER_STRING,
    ACTIVATE_STRING,
    SYSTEM_STRING,
    SHUTDOWN_STRING,
    RESTART_STRING,
    DEFAULT_STRING,
    NETWORK_PARAMETER_STRING,
    USER_ACCOUNT_STRING,
    CONFIGURATION_STRING,
    OK_STRING,
    DEACTIVATE_STRING,
    UNKNOWN_STRING,
    MAX_SYSTEM_CONTROL_STRINGS
} SYSTEM_CONTROL_STRINGS_e;

static const QString systemControlStrings[MAX_SYSTEM_CONTROL_STRINGS]={
    "Manual Trigger",
    "Activate",
    "System",
    "Shutdown",
    "Restart",
    "Default",
    "Network Parameters",
    "User Account",
    "Configuration",
    "OK",
    "DeActivate",
    "Unknown"

};

typedef enum{
    SYS_CTRL_DEVICE_RESTART,
    SYS_CTRL_DEVICE_SHUTDOWN,
    MAX_SYSTEM_CONTROL_INFO_STRINGS
}SYSTEM_CONTROL_INFO_STRINGS_e;

static const QString systemControlInfoStrings[MAX_SYSTEM_CONTROL_INFO_STRINGS]={
    "Device is going to Restart",
    "Device is going to Shutdown",
};

SystemControl::SystemControl(QString devName,
                             QWidget *parent)
    :ManageMenuOptions(devName, parent, MAX_SYSTEM_CONTROL_ELEMENT), m_userName(""),
      m_password("")
{
    quint16 topMargin = (MANAGE_PAGE_RIGHT_PANEL_HEIGHT -
                         (BGTILE_HEIGHT + INNER_CONTROL_MARGIN + BGTILE_HEIGHT +
                          INNER_CONTROL_MARGIN + ((BGTILE_HEIGHT + 10) * 2) +
                          (3 * BGTILE_HEIGHT))) / 2;
    quint16 leftMargin = (MANAGE_PAGE_RIGHT_PANEL_WIDTH - BGTILE_WIDTH) / 2;
    isManualTriggerActive = false;
    for(quint8 index = 0; index < 3; index++)
    {
        m_bgTile_M[index] = NULL;
    }
    m_currentConfig = 0;
    userValidation = NULL;

    for(quint8 index = 0; index < MAX_SYSTEM_CONTROL_ELEMENT; index++)
    {
        m_elementList[index] = NULL;
    }
    m_heading_1 = new ElementHeading(leftMargin,
                                     topMargin,
                                     BGTILE_WIDTH,
                                     BGTILE_HEIGHT,
                                     systemControlStrings[MANUAL_TRIGGER_STRING],
                                     COMMON_LAYER,
                                     this,
                                     false,
                                     SCALE_WIDTH(20));

    m_activate = new CnfgButton(CNFGBUTTON_MEDIAM,
                                (((m_heading_1->x() + m_heading_1->width() / 2))),
                                (m_heading_1->y() + (m_heading_1->height() / 2)),
                                systemControlStrings[ACTIVATE_STRING],
                                this,
                                ACTIVATE_CONFIG);

    m_elementList[ACTIVATE_CONFIG] = m_activate;

    connect(m_activate,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotConfigButtonClick(int)));
    connect(m_activate,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_heading_2 = new ElementHeading(leftMargin,
                                     (m_heading_1->y() + m_heading_1->height() + INNER_CONTROL_MARGIN),
                                     BGTILE_WIDTH,
                                     BGTILE_HEIGHT,
                                     systemControlStrings[SYSTEM_STRING],
                                     COMMON_LAYER,
                                     this,
                                     false,
                                     SCALE_WIDTH(20));

    m_shutdown = new CnfgButton(CNFGBUTTON_MEDIAM,
                                (((m_heading_2->x() + m_heading_2->width()/ 2))),
                                (m_heading_2->y() + (m_heading_2->height() / 2)),
                                systemControlStrings[SHUTDOWN_STRING],
                                this,
                                SHUTDOWN_CONFIG);

    m_elementList[SHUTDOWN_CONFIG] = m_shutdown;

    connect(m_shutdown,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotConfigButtonClick(int)));
    connect(m_shutdown,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));


    m_restart = new CnfgButton(CNFGBUTTON_MEDIAM,
                               (m_shutdown->x() + m_shutdown->width() + SCALE_WIDTH(80) ),
                               (m_heading_2->y() + (m_heading_2->height() / 2)),
                               systemControlStrings[RESTART_STRING],
                               this,
                               RESTART_CONFIG);

    m_elementList[RESTART_CONFIG] = m_restart;

    connect(m_restart,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotConfigButtonClick(int)));
    connect(m_restart,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_heading_3 = new ElementHeading(leftMargin,
                                     (m_heading_2->y() + m_heading_2->height() + INNER_CONTROL_MARGIN),
                                     BGTILE_WIDTH,
                                     BGTILE_HEIGHT,
                                     systemControlStrings[DEFAULT_STRING],
                                     COMMON_LAYER,
                                     this,
                                     false,
                                     SCALE_WIDTH(20), NORMAL_FONT_SIZE, true);

    for(quint8 index = 0; index < MAX_CHECK_BOX; index ++)
    {
        m_checkBox[index] = new OptionSelectButton(leftMargin,
                                                   (m_heading_3->y() +
                                                    ((index+1) * BGTILE_HEIGHT)) -1,
                                                   BGTILE_WIDTH,
                                                   BGTILE_HEIGHT,
                                                   CHECK_BUTTON_INDEX,
                                                   systemControlStrings[NETWORK_PARAMETER_STRING + index],
                                                   this,
                                                   MIDDLE_TABLE_LAYER,
                                                   20,
                                                   MX_OPTION_TEXT_TYPE_SUFFIX,
                                                   NORMAL_FONT_SIZE,
                                                   (index + CHECKBOX), true, NORMAL_FONT_COLOR, true);
        m_elementList[index + CHECKBOX] =  m_checkBox[index] ;
        connect(m_checkBox[index],
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e ,int)),
                this,
                SLOT(slotButtonClicked(OPTION_STATE_TYPE_e ,int)));
        connect(m_checkBox[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    }

    m_bgTile_Bottom = new BgTile(leftMargin,
                                 (m_checkBox[2]->y() + m_checkBox[2]->height()),
                                 BGTILE_WIDTH,
                                 BGTILE_HEIGHT,
                                 BOTTOM_TABLE_LAYER,
                                 this);

    m_ok = new CnfgButton(CNFGBUTTON_MEDIAM,
                          (m_bgTile_Bottom->x() + (m_bgTile_Bottom->width() / 2)),
                          (m_bgTile_Bottom->y() + ((m_bgTile_Bottom->height() - SCALE_HEIGHT(10)) / 2)),
                          systemControlStrings[OK_STRING],
                          this,
                          OK_CONFIG,
                          false);
    m_elementList[OK_CONFIG] =  m_ok;
    connect(m_ok,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotConfigButtonClick(int)));
    connect(m_ok,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    getManualTriggerStatus(m_currentDeviceName);
}

SystemControl :: ~SystemControl()
{
    disconnect(m_ok,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_ok,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotConfigButtonClick(int)));
    delete m_ok;
    delete m_bgTile_Bottom;

    for(quint8 index = 0;index < MAX_CHECK_BOX ;index ++)
    {
        disconnect(m_checkBox[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_checkBox[index],
                   SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e ,int)),
                   this,
                   SLOT(slotButtonClicked(OPTION_STATE_TYPE_e ,int)));
        delete m_checkBox[index] ;
    }

    delete m_heading_3;

    disconnect(m_restart,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_restart,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotConfigButtonClick(int)));
    delete m_restart;

    disconnect(m_shutdown,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_shutdown,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotConfigButtonClick(int)));
    delete m_shutdown;
    delete m_heading_2;

    disconnect(m_activate,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_activate,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotConfigButtonClick(int)));
    delete m_activate;
    delete m_heading_1;

    if(userValidation != NULL)
    {
        disconnect(userValidation,
                   SIGNAL(sigOkButtonClicked(QString,QString)),
                   this,
                   SLOT(slotOkButtonClicked(QString,QString)));
        delete userValidation;
        userValidation = NULL;
    }
}


void SystemControl:: getManualTriggerStatus(QString devName)
{
    if(m_applController->GetHlthStatusSingleParam(devName, manualTriggerStatus,
                                                  MANUAL_TRIGGER_STS))
    {
        changeText();
    }
}

void SystemControl::changeText()
{
    switch (manualTriggerStatus[0])
    {
    case MANUAL_TRIGGER_NORMAL:
        m_activate->changeText (systemControlStrings[ACTIVATE_STRING]);
        break;
    case MANUAL_TRIGGER_ACTIVE:
        m_activate->changeText (systemControlStrings[DEACTIVATE_STRING]);
        break;
    default:
        m_activate->changeText (systemControlStrings[UNKNOWN_STRING]);
        break;
    }
    m_activate->update();
}

void SystemControl:: updateStatus (QString devName, qint8 status, qint8, quint8 eventType,
                                   quint8 eventSubType)
{
    if((devName == m_currentDeviceName) &&
            (eventType == LOG_USER_EVENT) &&
            (eventSubType == LOG_MANUAL_TRIGGER) )
    {
        manualTriggerStatus[0] = status;
        processBar->unloadProcessBar();
        changeText();
    }
}

void SystemControl::handleInfoPageMessage(int buttonIndex)
{
    if(buttonIndex == INFO_OK_BTN)
    {
        if(((m_currentConfig == SHUTDOWN_CONFIG) ||
            (m_currentConfig == RESTART_CONFIG) ||
            (m_currentConfig == OK_CONFIG)) &&
                (userValidation == NULL))
        {
            userValidation = new UsersValidation(parentWidget(), PAGE_ID_MANAGE_PAGE);
            connect(userValidation,
                    SIGNAL(sigOkButtonClicked(QString,QString)),
                    this,
                    SLOT(slotOkButtonClicked(QString,QString)),
                    Qt::QueuedConnection);

        }
    }
}

void SystemControl::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if ((deviceName == m_currentDeviceName)
            && (param->msgType == MSG_SET_CMD))
    {
        if(param->deviceStatus == CMD_SUCCESS)
        {
            m_payloadLib->parseDevCmdReply(true, param->payload);
        }
        else
        {
            m_currentConfig = MAX_SYSTEM_CONTROL_ELEMENT;
            infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
        }
    }
    processBar->unloadProcessBar();
}

void SystemControl::manualTrigger(QString devName)
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;

    if (isManualTriggerActive == true)
    {
        param->cmdType = SRT_MAN_TRG;
    }
    else
    {
        param->cmdType = STP_MAN_TRG;
    }

    processBar->loadProcessBar();

    if(m_applController->processActivity(devName, DEVICE_COMM, param) == true)
    {
        m_currentDeviceName = devName;
    }
}

void SystemControl:: shutdownDevice(QString devName)
{
    m_payloadLib->setCnfgArrayAtIndex (0,m_userName);
    m_payloadLib->setCnfgArrayAtIndex (1,m_password);

    QString payloadString = m_payloadLib->createDevCmdPayload(2);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = SHUTDOWN;
    param->payload = payloadString;

    processBar->loadProcessBar();

    if(m_applController->processActivity(devName, DEVICE_COMM, param) == true)
    {
        m_currentDeviceName = devName;
    }
}

void SystemControl::restartDevice(QString devName)
{
    m_payloadLib->setCnfgArrayAtIndex (0,m_userName);
    m_payloadLib->setCnfgArrayAtIndex (1,m_password);

    QString payloadString = m_payloadLib->createDevCmdPayload(2);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = RESTART;
    param->payload = payloadString;

    processBar->loadProcessBar();

    if(m_applController->processActivity(devName, DEVICE_COMM, param) == true)
    {
        m_currentDeviceName = devName;
    }
}


void SystemControl::factoryDefault(QString devName)
{
    m_payloadLib->setCnfgArrayAtIndex (0,m_userName);
    m_payloadLib->setCnfgArrayAtIndex (1,m_password);

    qint8 feildValue = 2;
    for(qint8 index = 0;index < 3;index++)
    {
        m_payloadLib->setCnfgArrayAtIndex(feildValue++, m_checkBox[index]->getCurrentState());
    }

    QString payloadString = m_payloadLib->createDevCmdPayload(5);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = FAC_DEF_CFG;
    param->payload = payloadString;

    processBar->loadProcessBar();

    if(m_applController->processActivity(devName, DEVICE_COMM, param) == true)
    {
        m_currentDeviceName = devName;
    }
}


void SystemControl::slotButtonClicked(OPTION_STATE_TYPE_e  state ,int)
{
    if(state)
    {
        m_ok->setIsEnabled(true);
    }
    else
    {
        bool isClick = false;
        for (quint8 index = 0; index < 3; index++)
        {
            if(m_checkBox[index]->getCurrentState() == ON_STATE)
            {
                isClick = true;
                break;
            }
        }
        if(!isClick)
        {
            m_ok->setIsEnabled(false);
        }
    }
}

void SystemControl::slotConfigButtonClick(int index)
{
    if (index ==  ACTIVATE_CONFIG)
    {
        if(manualTriggerStatus[0] == MANUAL_TRIGGER_NORMAL)
        {
            manualTriggerStatus[0] = MANUAL_TRIGGER_ACTIVE;
            isManualTriggerActive = true;
        }
        else
        {
            manualTriggerStatus[0] = MANUAL_TRIGGER_NORMAL;
            isManualTriggerActive = false;
        }
        manualTrigger(m_currentDeviceName);
    }
    else
    {
        m_currentConfig = index;
        switch (index)
        {
        case SHUTDOWN_CONFIG:
        {
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(SYSTEM_CONTROL_SHUTDOWN_MESSAGE), true);
        }
            break;

        case RESTART_CONFIG:
        {
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(SYSTEM_CONTROL_RESTART_MESSAGE), true);
        }
            break;

        case OK_CONFIG:
        {
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(SYSTEM_CONTROL_FACTORY_RESTORE_MESSAGE), true);
        }
            break;
        }
    }
}

void SystemControl::slotOkButtonClicked (QString userName, QString password)
{
    m_userName = userName;
    m_password = password;
    disconnect(userValidation,
               SIGNAL(sigOkButtonClicked(QString,QString)),
               this,
               SLOT(slotOkButtonClicked(QString,QString)));
    delete userValidation;
    userValidation = NULL;

    if((m_userName != "")
            && (m_password != ""))
    {
        if(infoPage->getText() == ValidationMessage::getValidationMessage(SYSTEM_CONTROL_SHUTDOWN_MESSAGE))
        {
            shutdownDevice(m_currentDeviceName);
        }
        else if(infoPage->getText() == (ValidationMessage::getValidationMessage(SYSTEM_CONTROL_RESTART_MESSAGE)))
        {
            restartDevice(m_currentDeviceName);
        }
        else if(infoPage->getText() == (ValidationMessage::getValidationMessage(SYSTEM_CONTROL_FACTORY_RESTORE_MESSAGE)))
        {
            factoryDefault(m_currentDeviceName);
        }

    }
    m_currentConfig = MAX_SYSTEM_CONTROL_ELEMENT;
    m_elementList[m_currentElement]->forceActiveFocus();
}
