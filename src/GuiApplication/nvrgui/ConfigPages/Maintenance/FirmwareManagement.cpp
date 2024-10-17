#include "FirmwareManagement.h"
#include "ValidationMessage.h"

#define FIRST_ELE_TOP_MARGIN            SCALE_HEIGHT(15)
#define CNFG_TO_INDEX                   1
#define FW_UPDATE_AVAILABLE_STR(fwVer)  (Multilang("Firmware Upgrade Available:") + " " + QString(fwVer) + QString("\n\n") + Multilang("During upgrade, device will get restarted. Do you want to continue?"))
#define LEFT_MARGIN_FROM_CENTER         SCALE_WIDTH(50)

// List of control
typedef enum
{
    AUTO_FIRMAWARE_UPGRADE_OPTION,
    AUTO_FIRMAWARE_UPGRADE_SYNC_TIME,
    FTP_SERVER_OPTION,
    FTP_SERVER_URL,
    FTP_SERVER_PORT,
    FTP_USER_NAME,
    FTP_PASSWORD,
    FTP_PATH,
    CHECK_FOR_UPDATE_BUTTON,

    MAX_FIRMWARE_MANAGEMENT_ELEMETS
}STATIC_ROUTE_ELELIST_e;

typedef enum
{
    FIRMWARE_MANAGEMENT_CFG_MODE = 0,
    FIRMWARE_MANAGEMENT_CFG_SCHEDULE,
    FIRMWARE_MANAGEMENT_CFG_FTP_SERVER_TYPE,
    FIRMWARE_MANAGEMENT_CFG_FTP_SERVER_URL,
    FIRMWARE_MANAGEMENT_CFG_FTP_SERVER_PORT,
    FIRMWARE_MANAGEMENT_CFG_FTP_SERVER_USERNAME,
    FIRMWARE_MANAGEMENT_CFG_FTP_SERVER_PASSWORD,
    FIRMWARE_MANAGEMENT_CFG_FTP_PATH,
    MAX_FIRMWARE_MANAGEMENT_CFG_FIELD

}FIRMWARE_MANAGEMENT_CFG_FIELD_e;

typedef enum
{
    FIRMWARE_FTP_SERVER_MATRIX = 0,
    FIRMWARE_FTP_SERVER_CUSTOM,
    FIRMWARE_FTP_SERVER_MAX,

}FIRMWARE_FTP_SERVER_TYPE_e;

typedef enum
{
    AUTO_FIRMWARE_UPGRADE_NEVER = 0,
    AUTO_FIRMWARE_UPGRADE_NOTIFY_ONLY,
    AUTO_FIRMWARE_UPGRADE_DOWNLOAD_AND_UPGRADE,
    AUTO_FIRMWARE_UPGRADE_MODE_MAX,

}AUTO_FIRMWARE_UPGRADE_MODE_e;

const static QString labelOfElements[MAX_FIRMWARE_MANAGEMENT_ELEMETS]=
{
    "Auto Firmware Upgrade",
    "Daily At (HH:MM)",
    "FTP Server",
    "Address",
    "Port",
    "Username",
    "Password",
    "Firmware Path",
    "Check for Update",
};

const static QMap<quint8, QString> autoFirmwareUpgradeOptMapList =
{
    {AUTO_FIRMWARE_UPGRADE_NEVER, "Never"},
    {AUTO_FIRMWARE_UPGRADE_NOTIFY_ONLY, "Notify Only"},
    {AUTO_FIRMWARE_UPGRADE_DOWNLOAD_AND_UPGRADE, "Download and Upgrade"}
};

static const QMap<quint8, QString> ftpServerOptMapList =
{
    {FIRMWARE_FTP_SERVER_MATRIX, "MATRIX COMSEC"},
    {FIRMWARE_FTP_SERVER_CUSTOM, "Custom"}
};

FirmwareManagement::FirmwareManagement(QString devName, QWidget *parent, DEV_TABLE_INFO_t* devTabInfo)
    :ConfigPageControl(devName, parent, MAX_FIRMWARE_MANAGEMENT_ELEMETS, devTabInfo)
{
    createDefaultComponent();
    FirmwareManagement::getConfig();
}

FirmwareManagement::~FirmwareManagement()
{
    if(IS_VALID_OBJ(m_infoPage))
    {
        disconnect (m_infoPage,
                    SIGNAL(sigInfoPageCnfgBtnClick(int)),
                    this,
                    SLOT(slotInfoPageBtnclick(int)));
        DELETE_OBJ(m_infoPage);
    }

    if(IS_VALID_OBJ(m_checkForUpdateButton))
    {
        disconnect(m_checkForUpdateButton,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_checkForUpdateButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClicked(int)));
        DELETE_OBJ(m_checkForUpdateButton);
    }

    if(IS_VALID_OBJ(m_ftpPathTextbox))
    {
        disconnect(m_ftpPathTextbox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_ftpPathTextbox);
    }
    DELETE_OBJ(m_ftpPathParam);

    if(IS_VALID_OBJ(m_ftpPasswordTextbox))
    {
        disconnect(m_ftpPasswordTextbox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_ftpPasswordTextbox);
    }
    DELETE_OBJ(m_ftpPasswordParam);

    if(IS_VALID_OBJ(m_ftpUserNameTextbox))
    {
        disconnect(m_ftpUserNameTextbox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_ftpUserNameTextbox);
    }
    DELETE_OBJ(m_ftpUserNameParam);

    if(IS_VALID_OBJ(m_ftpPortTextbox))
    {
        disconnect(m_ftpPortTextbox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_ftpPortTextbox);
    }
    DELETE_OBJ(m_ftpPortParam);

    if(IS_VALID_OBJ(m_ftpAddrTextbox))
    {
        disconnect(m_ftpAddrTextbox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_ftpAddrTextbox);
    }
    DELETE_OBJ(m_ftpAddrParam);

    if(IS_VALID_OBJ(m_ftpServerOptionDropDownBox))
    {
        disconnect (m_ftpServerOptionDropDownBox,
                    SIGNAL(sigValueChanged(QString,quint32)),
                    this,
                    SLOT(slotSpinboxValueChanged(QString,quint32)));
        disconnect(m_ftpServerOptionDropDownBox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_ftpServerOptionDropDownBox);
    }

    if(IS_VALID_OBJ(m_autoFirmwareUpgradeSyncTime))
    {
        disconnect (m_autoFirmwareUpgradeSyncTime,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_autoFirmwareUpgradeSyncTime);
    }

    if(IS_VALID_OBJ(m_autoFirmwareUpgradeOptionDropDownBox))
    {
        disconnect (m_autoFirmwareUpgradeOptionDropDownBox,
                    SIGNAL(sigValueChanged(QString,quint32)),
                    this,
                    SLOT(slotSpinboxValueChanged(QString,quint32)));
        disconnect(m_autoFirmwareUpgradeOptionDropDownBox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_autoFirmwareUpgradeOptionDropDownBox);
    }
}

void FirmwareManagement::createDefaultComponent()
{
    m_autoFirmwareUpgradeOptionDropDownBox = new DropDown((this->width() - BGTILE_ULTRAMEDIUM_SIZE_WIDTH)/2,
                                                          SCALE_HEIGHT(80),
                                                          BGTILE_ULTRAMEDIUM_SIZE_WIDTH,
                                                          BGTILE_HEIGHT,
                                                          AUTO_FIRMAWARE_UPGRADE_OPTION,
                                                          DROPDOWNBOX_SIZE_320,
                                                          labelOfElements[AUTO_FIRMAWARE_UPGRADE_OPTION],
                                                          autoFirmwareUpgradeOptMapList,
                                                          this,
                                                          "", true, 0,
                                                          UP_LAYER, true, 8,
                                                          false, false, 5,
                                                          LEFT_MARGIN_FROM_CENTER);
    m_elementList[AUTO_FIRMAWARE_UPGRADE_OPTION] = m_autoFirmwareUpgradeOptionDropDownBox;

    connect(m_autoFirmwareUpgradeOptionDropDownBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (m_autoFirmwareUpgradeOptionDropDownBox,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotSpinboxValueChanged(QString,quint32)));

    m_autoFirmwareUpgradeSyncTime = new ClockSpinbox(m_autoFirmwareUpgradeOptionDropDownBox->x(),
                                                     (m_autoFirmwareUpgradeOptionDropDownBox->y() + m_autoFirmwareUpgradeOptionDropDownBox->height()),
                                                     BGTILE_ULTRAMEDIUM_SIZE_WIDTH,
                                                     BGTILE_HEIGHT,
                                                     AUTO_FIRMAWARE_UPGRADE_SYNC_TIME,
                                                     CLK_SPINBOX_With_NO_SEC,
                                                     labelOfElements[AUTO_FIRMAWARE_UPGRADE_SYNC_TIME], 6,
                                                     this, "", true, 0,
                                                     UP_LAYER, false, 5,
                                                     false, false,
                                                     LEFT_MARGIN_FROM_CENTER);
    m_elementList[AUTO_FIRMAWARE_UPGRADE_SYNC_TIME] = m_autoFirmwareUpgradeSyncTime;

    connect (m_autoFirmwareUpgradeSyncTime,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    m_ftpServerOptionDropDownBox = new DropDown(m_autoFirmwareUpgradeSyncTime->x(),
                                                (m_autoFirmwareUpgradeSyncTime->y() + m_autoFirmwareUpgradeSyncTime->height()),
                                                BGTILE_ULTRAMEDIUM_SIZE_WIDTH,
                                                BGTILE_HEIGHT,
                                                FTP_SERVER_OPTION,
                                                DROPDOWNBOX_SIZE_320,
                                                labelOfElements[FTP_SERVER_OPTION],
                                                ftpServerOptMapList,
                                                this,
                                                "", true, 0,
                                                UP_LAYER, true, 8,
                                                false, false, 5,
                                                LEFT_MARGIN_FROM_CENTER);
    m_elementList[FTP_SERVER_OPTION] = m_ftpServerOptionDropDownBox;

    connect(m_ftpServerOptionDropDownBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (m_ftpServerOptionDropDownBox,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotSpinboxValueChanged(QString,quint32)));

    m_ftpAddrParam = new TextboxParam();
    m_ftpAddrParam->labelStr = labelOfElements[FTP_SERVER_URL];
    m_ftpAddrParam->maxChar = 40;
    m_ftpAddrParam->validation = QRegExp(QString("[^\\ ]"));
    m_ftpAddrTextbox = new TextBox(m_ftpServerOptionDropDownBox->x(),
                                   m_ftpServerOptionDropDownBox->y()+ m_ftpServerOptionDropDownBox->height(),
                                   BGTILE_ULTRAMEDIUM_SIZE_WIDTH,
                                   BGTILE_HEIGHT,
                                   FTP_SERVER_URL,
                                   TEXTBOX_ULTRALARGE,
                                   this, m_ftpAddrParam,
                                   UP_LAYER, true, false,
                                   false, LEFT_MARGIN_FROM_CENTER);
    m_elementList[FTP_SERVER_URL] = m_ftpAddrTextbox;
    connect (m_ftpAddrTextbox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    m_ftpPortParam = new TextboxParam();
    m_ftpPortParam->labelStr = labelOfElements[FTP_SERVER_PORT];
    m_ftpPortParam->suffixStr = "(1 - 65535)";
    m_ftpPortParam->isNumEntry = true;
    m_ftpPortParam->minNumValue = 1;
    m_ftpPortParam->maxNumValue = 65535;
    m_ftpPortParam->maxChar = 5;
    m_ftpPortParam->validation = QRegExp(QString("[0-9]"));

    m_ftpPortTextbox = new TextBox(m_ftpAddrTextbox->x(),
                                   m_ftpAddrTextbox->y()+ m_ftpAddrTextbox->height(),
                                   BGTILE_ULTRAMEDIUM_SIZE_WIDTH,
                                   BGTILE_HEIGHT,
                                   FTP_SERVER_PORT,
                                   TEXTBOX_SMALL,
                                   this, m_ftpPortParam,
                                   UP_LAYER, true, false,
                                   false, LEFT_MARGIN_FROM_CENTER);
    m_elementList[FTP_SERVER_PORT] = m_ftpPortTextbox;
    connect (m_ftpPortTextbox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    m_ftpUserNameParam = new TextboxParam();
    m_ftpUserNameParam->labelStr = labelOfElements[FTP_USER_NAME];
    m_ftpUserNameParam->maxChar = 40;
    m_ftpUserNameTextbox = new TextBox(m_ftpPortTextbox->x(),
                                       m_ftpPortTextbox->y()+ m_ftpPortTextbox->height(),
                                       BGTILE_ULTRAMEDIUM_SIZE_WIDTH,
                                       BGTILE_HEIGHT,
                                       FTP_USER_NAME,
                                       TEXTBOX_ULTRALARGE,
                                       this, m_ftpUserNameParam,
                                       UP_LAYER, true, false, false,
                                       LEFT_MARGIN_FROM_CENTER);
    m_elementList[FTP_USER_NAME] = m_ftpUserNameTextbox;
    connect (m_ftpUserNameTextbox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    m_ftpPasswordParam = new TextboxParam();
    m_ftpPasswordParam->labelStr = labelOfElements[FTP_PASSWORD];
    m_ftpPasswordParam->maxChar = 24;
    m_ftpPasswordTextbox = new PasswordTextbox(m_ftpUserNameTextbox->x(),
                                               m_ftpUserNameTextbox->y() + m_ftpUserNameTextbox->height(),
                                               BGTILE_ULTRAMEDIUM_SIZE_WIDTH,
                                               BGTILE_HEIGHT,
                                               FTP_PASSWORD,
                                               TEXTBOX_ULTRALARGE, this,
                                               m_ftpPasswordParam, UP_LAYER,
                                               true, LEFT_MARGIN_FROM_CENTER);
    m_elementList[FTP_PASSWORD] = m_ftpPasswordTextbox;
    connect(m_ftpPasswordTextbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_ftpPathParam = new TextboxParam();
    m_ftpPathParam->labelStr = labelOfElements[FTP_PATH];
    m_ftpPathParam->maxChar = 255;
    m_ftpPathTextbox = new TextBox(m_ftpPasswordTextbox->x(),
                                   m_ftpPasswordTextbox->y()+ m_ftpPasswordTextbox->height(),
                                   BGTILE_ULTRAMEDIUM_SIZE_WIDTH,
                                   BGTILE_HEIGHT,
                                   FTP_PATH,
                                   TEXTBOX_ULTRALARGE,
                                   this, m_ftpPathParam,
                                   UP_LAYER, true, false,
                                   false, LEFT_MARGIN_FROM_CENTER);
    m_elementList[FTP_PATH] = m_ftpPathTextbox;
    connect (m_ftpPathTextbox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    m_checkForUpdateButton = new ControlButton(UPDATE_BUTTON_INDEX,
                                               m_ftpPathTextbox->x(),
                                               m_ftpPathTextbox->y() + m_ftpPathTextbox->height(),
                                               BGTILE_ULTRAMEDIUM_SIZE_WIDTH, BGTILE_HEIGHT,
                                               this, UP_LAYER, -1,
                                               labelOfElements[CHECK_FOR_UPDATE_BUTTON],
                                               true, CHECK_FOR_UPDATE_BUTTON, true,
                                               LEFT_MARGIN_FROM_CENTER);
    m_elementList[CHECK_FOR_UPDATE_BUTTON] = m_checkForUpdateButton;
    connect(m_checkForUpdateButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_checkForUpdateButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClicked(int)));
    m_infoPage = new InfoPage(0, 0,
                              SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) + SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH),
                              SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT),
                              INFO_CONFIG_PAGE,
                              parentWidget());
    connect (m_infoPage,
             SIGNAL(sigInfoPageCnfgBtnClick(int)),
             this,
             SLOT(slotInfoPageCnfgBtnClick(int)));
}

void FirmwareManagement::getConfig()
{
    createPayload(MSG_GET_CFG);
}

void FirmwareManagement::defaultConfig()
{
    createPayload(MSG_DEF_CFG);
}

void FirmwareManagement::saveConfig()
{
    quint32     tHour, tMin;
    QString     tStr = "";

    if(m_ftpServerOptionDropDownBox->getIndexofCurrElement() != 0)
    {
        if(m_ftpAddrTextbox->getInputText () == "")
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(FTP_CLIENT_ENT_FTP_SERVER_ADDR));
            return;
        }
        else if(m_ftpUserNameTextbox->getInputText () == "")
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_USER_NM));
            return;
        }
        else if(m_ftpPasswordTextbox->getInputText () == "")
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_PASSWORD));
            return;
        }
    }

    m_autoFirmwareUpgradeSyncTime->currentValue(tHour, tMin);
    if(tHour < 10)
    {
        tStr.append (QString("%1%2").arg (0).arg(tHour));
    }
    else
    {
        tStr.append (QString("%1").arg(tHour));
    }
    if(tMin < 10)
    {
        tStr.append (QString("%1%2").arg (0).arg(tMin));
    }
    else
    {
        tStr.append (QString("%1").arg(tMin));
    }
    payloadLib->setCnfgArrayAtIndex(FIRMWARE_MANAGEMENT_CFG_MODE, m_autoFirmwareUpgradeOptionDropDownBox->getIndexofCurrElement());
    payloadLib->setCnfgArrayAtIndex(FIRMWARE_MANAGEMENT_CFG_SCHEDULE, tStr);
    payloadLib->setCnfgArrayAtIndex(FIRMWARE_MANAGEMENT_CFG_FTP_SERVER_TYPE, m_ftpServerOptionDropDownBox->getIndexofCurrElement());
    payloadLib->setCnfgArrayAtIndex(FIRMWARE_MANAGEMENT_CFG_FTP_SERVER_URL, m_ftpAddrTextbox->getInputText());
    payloadLib->setCnfgArrayAtIndex(FIRMWARE_MANAGEMENT_CFG_FTP_SERVER_PORT, m_ftpPortTextbox->getInputText());
    payloadLib->setCnfgArrayAtIndex(FIRMWARE_MANAGEMENT_CFG_FTP_SERVER_USERNAME, m_ftpUserNameTextbox->getInputText());
    payloadLib->setCnfgArrayAtIndex(FIRMWARE_MANAGEMENT_CFG_FTP_SERVER_PASSWORD, m_ftpPasswordTextbox->getInputText());
    payloadLib->setCnfgArrayAtIndex(FIRMWARE_MANAGEMENT_CFG_FTP_PATH, m_ftpPathTextbox->getInputText());
    createPayload(MSG_SET_CFG);
}

void FirmwareManagement::createPayload(REQ_MSG_ID_e requestType)
{
    QString payloadString = payloadLib->createDevCnfgPayload(requestType,
                                                             FIRMWARE_MANAGEMNET_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_TO_INDEX,
                                                             CNFG_FRM_FIELD,
                                                             MAX_FIRMWARE_MANAGEMENT_CFG_FIELD,
                                                             (MSG_SET_CFG == requestType) ? MAX_FIRMWARE_MANAGEMENT_CFG_FIELD : 0);
    DevCommParam* param = new DevCommParam();

    param->msgType = requestType;
    param->payload = payloadString;

    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void FirmwareManagement::processDeviceResponse (DevCommParam *param, QString deviceName)
{
    QString timeStr;
    quint8  tFtpServerOpt = 0;
    quint8  tIsEnable = false;

    processBar->unloadProcessBar();

    if(deviceName != currDevName)
    {
        return;
    }

    switch(param->deviceStatus)
    {
        case CMD_SUCCESS:
        {
            switch(param->msgType)
            {
                case MSG_SET_CMD:
                {
                    if(param->cmdType == SEARCH_FIRMWARE)
                    {
                        payloadLib->parseDevCmdReply(true, param->payload);
                        m_infoPage->loadInfoPage(FW_UPDATE_AVAILABLE_STR(payloadLib->getCnfgArrayAtIndex(0).toString()),
                                                 true, false, "", CONFORMATION_BTN_YES, CONFORMATION_BTN_NO);
                    }
                    else if(param->cmdType == START_UPGRADE)
                    {
                        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(FIMRWARE_UPGRADE_NOTE));
                    }
                }
                break;

                case MSG_GET_CFG:
                {
                    payloadLib->parsePayload(param->msgType, param->payload);
                    if(payloadLib->getcnfgTableIndex (0) == FIRMWARE_MANAGEMNET_TABLE_INDEX)
                    {
                        m_autoFirmwareUpgradeOptionDropDownBox->setIndexofCurrElement(payloadLib->getCnfgArrayAtIndex(FIRMWARE_MANAGEMENT_CFG_MODE).toUInt());
                        timeStr = payloadLib->getCnfgArrayAtIndex(FIRMWARE_MANAGEMENT_CFG_SCHEDULE).toString();
                        m_autoFirmwareUpgradeSyncTime->assignValue (timeStr.mid(0, 2), timeStr.mid (2, 2));
                        tFtpServerOpt = payloadLib->getCnfgArrayAtIndex(FIRMWARE_MANAGEMENT_CFG_FTP_SERVER_TYPE).toUInt();

                        #if defined(OEM_JCI)
                        tFtpServerOpt = FIRMWARE_FTP_SERVER_CUSTOM;
                        m_ftpServerOptionDropDownBox->setIsEnabled(false);
                        #endif

                        m_ftpServerOptionDropDownBox->setIndexofCurrElement(tFtpServerOpt);

                        if(0 != payloadLib->getCnfgArrayAtIndex(FIRMWARE_MANAGEMENT_CFG_MODE).toUInt())
                        {
                            m_autoFirmwareUpgradeSyncTime->setIsEnabled(true);
                        }
                        else
                        {
                            m_autoFirmwareUpgradeSyncTime->setIsEnabled(false);
                        }
                        m_ftpAddr = payloadLib->getCnfgArrayAtIndex(FIRMWARE_MANAGEMENT_CFG_FTP_SERVER_URL).toString();
                        m_ftpPort = payloadLib->getCnfgArrayAtIndex(FIRMWARE_MANAGEMENT_CFG_FTP_SERVER_PORT).toString();
                        m_ftpUserName = payloadLib->getCnfgArrayAtIndex(FIRMWARE_MANAGEMENT_CFG_FTP_SERVER_USERNAME).toString();
                        m_ftpPassword = payloadLib->getCnfgArrayAtIndex(FIRMWARE_MANAGEMENT_CFG_FTP_SERVER_PASSWORD).toString();
                        m_ftpPath = payloadLib->getCnfgArrayAtIndex(FIRMWARE_MANAGEMENT_CFG_FTP_PATH).toString();

                        tIsEnable = (0 != tFtpServerOpt)? true : false;
                        m_ftpAddrTextbox->setIsEnabled(tIsEnable);
                        m_ftpPortTextbox->setIsEnabled(tIsEnable);
                        m_ftpUserNameTextbox->setIsEnabled(tIsEnable);
                        m_ftpPasswordTextbox->setIsEnabled(tIsEnable);
                        m_ftpPathTextbox->setIsEnabled(tIsEnable);
                        m_ftpAddrTextbox->setInputText((tIsEnable) ? m_ftpAddr : "");
                        m_ftpPortTextbox->setInputText((tIsEnable) ? m_ftpPort : "21");
                        m_ftpUserNameTextbox->setInputText((tIsEnable) ? m_ftpUserName : "");
                        m_ftpPasswordTextbox->setInputText((tIsEnable) ? m_ftpPassword : "");
                        m_ftpPathTextbox->setInputText((tIsEnable) ? m_ftpPath : "");
                    }
                }
                break;

                case MSG_SET_CFG:
                {
                    MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
                    getConfig();
                }
                break;

                case MSG_DEF_CFG:
                {
                    getConfig();
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

        default:
        {
            infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
            if(param->deviceStatus != CMD_SERVER_NOT_RESPONDING)
            {
                getConfig();
            }
        }
    }
}

void FirmwareManagement::slotSpinboxValueChanged(QString str, quint32 index)
{
    quint8 tIsEnable = false;

    switch(index)
    {
        case AUTO_FIRMAWARE_UPGRADE_OPTION:
            if (AUTO_FIRMWARE_UPGRADE_NEVER != autoFirmwareUpgradeOptMapList.key(str))
            {
                m_autoFirmwareUpgradeSyncTime->setIsEnabled(true);
            }
            else
            {
                m_autoFirmwareUpgradeSyncTime->setIsEnabled(false);
            }
            break;

        case FTP_SERVER_OPTION:
            tIsEnable = (FIRMWARE_FTP_SERVER_MATRIX != ftpServerOptMapList.key(str))? true : false;
            m_ftpAddrTextbox->setIsEnabled(tIsEnable);
            m_ftpPortTextbox->setIsEnabled(tIsEnable);
            m_ftpUserNameTextbox->setIsEnabled(tIsEnable);
            m_ftpPasswordTextbox->setIsEnabled(tIsEnable);
            m_ftpPathTextbox->setIsEnabled(tIsEnable);
            m_ftpAddrTextbox->setInputText((tIsEnable) ? m_ftpAddr : "");
            m_ftpPortTextbox->setInputText((tIsEnable) ? m_ftpPort : "21");
            m_ftpUserNameTextbox->setInputText((tIsEnable) ? m_ftpUserName : "");
            m_ftpPasswordTextbox->setInputText((tIsEnable) ? m_ftpPassword : "");
            m_ftpPathTextbox->setInputText((tIsEnable) ? m_ftpPath : "");
            break;

        default:
            break;
    }
}

void FirmwareManagement::slotButtonClicked(int index)
{
    if(CHECK_FOR_UPDATE_BUTTON != index)
    {
        return;
    }

    QString payloadString = payloadLib->createDevCmdPayload(1);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = SEARCH_FIRMWARE;
    param->payload = payloadString;
    if(processBar->isLoadedProcessBar() == false)
    {
        processBar->loadProcessBar();
    }
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void FirmwareManagement::slotInfoPageCnfgBtnClick(int index)
{
    if(INFO_OK_BTN != index)
    {
        return;
    }
    QString payloadString = payloadLib->createDevCmdPayload(1);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = START_UPGRADE;
    param->payload = payloadString;
    if(processBar->isLoadedProcessBar() == false)
    {
        processBar->loadProcessBar();
    }
    applController->processActivity(currDevName, DEVICE_COMM, param);
}
