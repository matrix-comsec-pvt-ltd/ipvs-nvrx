#include "Settings.h"
#include "ValidationMessage.h"

#include <QKeyEvent>

#define STNG_TOP_MARGIN                 SCALE_HEIGHT(125)
#define CELL_HEIGHT                     SCALE_HEIGHT(30)
#define CELL_WIDTH                      SCALE_WIDTH(260)
#define SUBCELL_WIDTH                   SCALE_WIDTH(240)
#define SUBCELL_HORIZONTAL_OFFSET       SCALE_WIDTH(35)
#define SUBCELL_X_OFFSET                SCALE_WIDTH(46)

#define DEVICE_HEADING_STRING           "Device"
#define SETTING_HEADING_STRING          "Settings"

#define SETTING_LEFT_PANEL_DISABLE_IMG_PATH ":/Images_Nvrx/SettingMenu/LeftPanelDisable.png"

static const QString settingsOption[MAX_SETTING_OPTION] =
{
    "Basic Settings",
    "Camera Settings",
    "Network Settings",
    "Storage and Backup Settings",
    "Device I/O Settings",
    "User Account Management",
    "Event and Action Settings",
    "Maintenance",
    "Devices",
};

static const QString settingsOptionHeadings[MAX_SETTING_OPTION] =
{
    "General Settings",
    "Camera Search",
    "LAN 1 Settings",
    "HDD Management",
    "Device Sensor Input Settings",
    "User Account Management",
    "Camera Event and Action Settings",
    "Firmware Management",
    "Network Devices",
};

static const QString settingsBasicSubOption[MAX_BASIC_SUBSETTING_OPTION] =
{
    "General",
    "Date and Time",
    "Daylight Saving Time",
};

static const QString settingsBasicSubOptionHeading[MAX_BASIC_SUBSETTING_OPTION] =
{
    "General Settings",
    "Date and Time Settings",
    "Daylight Saving Time (DST) Settings",
};

static const QString settingsCameraSubOption[MAX_CAMERA_SUBSETTING_OPTION] =
{
    "Camera Search",
    "Camera",
    "Stream",
    "Image Settings",
    "Recording",
    "Schedule Snapshot",
    "Image Upload",
    "PTZ Tour",
    "Audio",
    "Alarm Output",
};

static const QString settingsCameraSubOptionHeading[MAX_CAMERA_SUBSETTING_OPTION] =
{
    "Camera Search",
    "Camera Settings",
    "Stream Settings",
    "Image Settings",
    "Recording",
    "Schedule Snapshot",
    "Image Upload Settings",
    "PTZ Tour Settings",
    "Audio Settings",
    "Alarm Output Settings",
};

/* For displaying the sub-category labels of network setting */
static const QString settingsNetworkSubOption[MAX_NETWORK_SUBSETTING_OPTION] =
{
    "LAN 1",
    "LAN 2",
    "DHCP Server",
    "Broadband",
    "Static Routing",
    "IP Address Filtering",
    "Matrix DNS Client",
    "DDNS Client",
    "Email Client",
    "FTP Client",
    "TCP Notification",
    "Media File Access",
    "SMS",
    "P2P",
};

/* To display the heading of individual sub-category page of network settings */
static const QString settingsNetworkSubOptionHeading[MAX_NETWORK_SUBSETTING_OPTION] =
{
    "LAN 1 Settings",
    "LAN 2 Settings",
    "DHCP Server Settings",
    "Broadband Settings",
    "Static Routing",
    "IP Address Filtering",
    "Matrix DNS Client Settings",
    "DDNS Client Settings",
    "Email Client Settings",
    "FTP Client Settings",
    "TCP Client Settings",
    "Media File Access Settings",
    "SMS Settings",
    "P2P Settings",
};

static const QString settingsStorageSubOption[MAX_STORAGE_SUBSETTING_OPTION] =
{
    "HDD",
    "HDD Group",
    "Network Drive",
    "Storage",
    "Backup",
    "USB",
};

static const QString settingsStorageSubOptionHeading[MAX_STORAGE_SUBSETTING_OPTION] =
{
    "HDD Management",
    "HDD Group Management",
    "Network Drive Management",
    "Storage Management",
    "Backup Management",
    "USB Management",
};

static const QString settingsDeviceIOSubOption[MAX_DEVICE_IO_SUBSETTING_OPTION] =
{
    "Sensor Input",
    "Alarm Output",
};

static const QString settingsDeviceIOSubOptionHeading[MAX_DEVICE_IO_SUBSETTING_OPTION] =
{
    "Device Sensor Input Settings",
    "Device Alarm Output Settings",
};

static const QString settingsEventSubOption[MAX_EVENT_SUBSETTING_OPTION] =
{
    "Camera",
    "Device Sensor",
    "System",
};

static const QString settingsEventSubOptionHeading[MAX_EVENT_SUBSETTING_OPTION] =
{
    "Camera Event and Action Settings",
    "Device Sensor Event and Action Settings",
    "System Event and Action Settings",
};

static const QString settingsUserAccountManagmentSubOption[MAX_USER_ACCOUNT_SUBSETTING_OPTION] =
{
    "User Account",
    "Password Policy",
    "Push Notification Status",
};

static const QString settingsUserAccountManagmentSubOptionHeading[MAX_USER_ACCOUNT_SUBSETTING_OPTION] =
{
    "User Account Management",
    "Password Policy",
    "Push Notification Status",
};

static const QString settingsMaintenanceSubOption[MAX_MAINTENANCE_SUBSETTING_OPTION] =
{
    "Firmware Management",
};

static const QString settingsMaintenanceSubOptionHeading[MAX_MAINTENANCE_SUBSETTING_OPTION] =
{
    "Firmware Management",
};

static const QString settingsDevicesSubOption[] =
{
    "Devices",
};

static const QString settingsDevicesSubOptionHeading[] =
{
    "Network Devices",
};

static const quint8 maxSubOptionCount[MAX_SETTING_OPTION] =
{
    MAX_BASIC_SUBSETTING_OPTION,
    MAX_CAMERA_SUBSETTING_OPTION,
    MAX_NETWORK_SUBSETTING_OPTION,
    MAX_STORAGE_SUBSETTING_OPTION,
    MAX_DEVICE_IO_SUBSETTING_OPTION,
    MAX_USER_ACCOUNT_SUBSETTING_OPTION,
    MAX_EVENT_SUBSETTING_OPTION,
    MAX_MAINTENANCE_SUBSETTING_OPTION,
    MAX_DEVICE_SUBSETTING_OPTION
};

static const QString *maxSubOptionMenuLabel[MAX_SETTING_OPTION] =
{
    settingsBasicSubOption,
    settingsCameraSubOption,
    settingsNetworkSubOption,
    settingsStorageSubOption,
    settingsDeviceIOSubOption,
    settingsUserAccountManagmentSubOption,
    settingsEventSubOption,
    settingsMaintenanceSubOption,
    settingsDevicesSubOption,
};

static const QString *maxSubOptionPageHeadings[MAX_SETTING_OPTION] =
{
    settingsBasicSubOptionHeading,
    settingsCameraSubOptionHeading,
    settingsNetworkSubOptionHeading,
    settingsStorageSubOptionHeading,
    settingsDeviceIOSubOptionHeading,
    settingsUserAccountManagmentSubOptionHeading,
    settingsEventSubOptionHeading,
    settingsMaintenanceSubOptionHeading,
    settingsDevicesSubOptionHeading,
};

static const bool isClickOnClickNeeded[] =
{
    true,   //    BASIC_SETTING,
    true,   //    CAMERA_SETTING,
    true,   //    NETWORK_SETTTING,
    true,   //    STORAGE_AND_BACKUP_SETTING,
    true,   //    DEVICE_IO_SETTING,
    true,   //    USER_ACCOUNT_MANAGMENT_SETTING,
    true,   //    EVENT_AND_ACTION_SETTING,
    true,   //    MAINTENANCE,
    false,  //    DEVICES,
};

Settings::Settings(QWidget *parent) :
    BackGround((ApplController::getXPosOfScreen() + ((ApplController::getWidthOfScreen() - SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH)) / 2)),
               (ApplController::getYPosOfScreen() + ((ApplController::getHeightOfScreen() - SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT)) / 2)),
               SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH),
               SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT),
               BACKGROUND_TYPE_2,
               SETTINGS_BUTTON,
               parent,
               SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH),
               SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT),
               SETTING_HEADING_STRING)
{
    setObjectName("Setting");

    for(quint8 index = 0; index < (MAX_SETTING_CONTROL_ELEMENT + MAX_SUB_SETTING_OPTION); index++)
    {
        m_elementList[index] = NULL;
    }

    isSubMenuOptionsCreated = false;
    for(quint8 index = 0; index < MAX_SUB_SETTING_OPTION; index++)
    {
        m_settingsSubOptions[index] = NULL;
    }

    for(quint8 index = 0; index < MAX_SUB_SETTING_CONTOL_ELEMENT; index++)
    {
        m_subelementList[index] = NULL;
    }

    m_subcurrentElement = 0;
    currentClickedIndex = MAX_SETTING_OPTION;
    currentIndex = MAX_SETTING_OPTION;
    currentSubIndex = MAX_CAMERA_SUBSETTING_OPTION;
    prevOptionIndex = MAX_SETTING_OPTION;
    currentSubIndex= 0;
    m_elementList[STG_CLOSE_BUTTON] = m_mainCloseButton;
    m_mainCloseButton->setIndexInPage(STG_CLOSE_BUTTON);
    currState = CONNECTED;
    connect(m_mainCloseButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    subElementFocusListEnable = false;
    isRightPanelOpen = false;
    configPageControl = NULL;

    INIT_OBJ(m_leftPanelDisableImg);
    subMenuCount = 0;
    applController = ApplController::getInstance();
    QMap<quint8, QString> deviceMapList;
    applController->GetDevNameDropdownMapList(deviceMapList);
    currDevName = applController->GetRealDeviceName(deviceMapList.value(0));
    applController->GetDeviceInfo(currDevName, devTableInfo);

    #if defined(OEM_JCI)
    isHddGroupDisp = true;
    isMediaFileAccessDisp = false;
    isSmsDisp = false;
    isP2pDisp = false;
    isTcpClientDisp = false;
    isMatrixDnsClientDisp = false;
    #else
    if (((devTableInfo.productVariant >= NVR3202XP2) && (devTableInfo.productVariant <= NVR6408XP2)) || (devTableInfo.productVariant == NVR9608XP2))
    {
        isHddGroupDisp = true;
        isMediaFileAccessDisp = false;
    }
    else
    {
        isHddGroupDisp = false;
        isMediaFileAccessDisp = true;
    }
    isSmsDisp = true;
    isP2pDisp = true;
    isTcpClientDisp = true;
    isMatrixDnsClientDisp = true;
    #endif

    deviceListDropDown = new DropDown (SCALE_WIDTH(25),
                                       SCALE_HEIGHT(65),
                                       SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH),
                                       SCALE_HEIGHT(50),
                                       STG_DROPDOWNBOX,
                                       DROPDOWNBOX_SIZE_200,
                                       DEVICE_HEADING_STRING,
                                       deviceMapList,
                                       this,
                                       "",
                                       false,
                                       SCALE_WIDTH(5),
                                       NO_LAYER);
    m_elementList[STG_DROPDOWNBOX] = deviceListDropDown;
    connect(deviceListDropDown,
            SIGNAL(sigValueChanged(QString,quint32)),
            this,
            SLOT(slotSpinBoxValueChange(QString,quint32)));
    connect(deviceListDropDown,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    for(quint8 index = 0; index < MAX_SETTING_OPTION; index++)
    {
        m_settingsOptions[index] = new MenuButton(index, CELL_WIDTH , CELL_HEIGHT,
                                                  settingsOption[index],
                                                  this,
                                                  SCALE_WIDTH(12) ,
                                                  SCALE_WIDTH(26),
                                                  STNG_TOP_MARGIN,
                                                  index + STG_SETTING_OPTION,
                                                  true,
                                                  true,
                                                  isClickOnClickNeeded[index]);
        m_elementList[(index + STG_SETTING_OPTION)] = m_settingsOptions[index];
        connect(m_settingsOptions[index],
                SIGNAL(sigButtonClicked(int)),
                this,
                SLOT (slotSettingsOptions(int)));
        connect(m_settingsOptions[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    }

    if ((devTableInfo.sensors == 0) && (devTableInfo.alarms == 0))
    {
        m_settingsOptions[DEVICE_IO_SETTING]->setIsEnabled(false);
        for(quint8 index = USER_ACCOUNT_MANAGMENT_SETTING; index < MAX_SETTING_OPTION; index++)
        {
            m_settingsOptions[index]->resetGeometry(0,-1);
        }
    }

    prevOptionIndex = MAX_SETTING_CONTROL_ELEMENT;
    prevSubOptionIndex = MAX_SETTING_CONTROL_ELEMENT;
    maxSettingControlElement = MAX_SETTING_CONTROL_ELEMENT;

    infoPage = new InfoPage (0,
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

    m_currentElement = STG_DROPDOWNBOX;
    m_elementList[m_currentElement]->forceActiveFocus();
    this->show();
}

Settings::~Settings()
{
    disconnect(m_mainCloseButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    deleteSubPageObject();

    disconnect (infoPage,
                SIGNAL(sigInfoPageCnfgBtnClick(int)),
                this,
                SLOT(slotInfoPageBtnclick(int)));
    delete infoPage;

    if(subMenuCount > 0)
    {
        for(quint8 index = 0; index < subMenuCount; index++)
        {
            if (IS_VALID_OBJ(m_settingsSubOptions[index]))
            {
                disconnect(m_settingsSubOptions[index],
                           SIGNAL(sigUpdateCurrentElement(int)),
                           this,
                           SLOT(slotUpdateCurrentElement(int)));
                disconnect(m_settingsSubOptions[index],
                           SIGNAL(sigButtonClicked(int)),
                           this,
                           SLOT (slotSubSettingsOptions(int)));
                DELETE_OBJ(m_settingsSubOptions[index]);
            }
        }
    }

    DELETE_OBJ(m_leftPanelDisableImg);

    for(quint8 index = 0; index < MAX_SETTING_OPTION; index++)
    {
        if(IS_VALID_OBJ(m_settingsOptions[index]))
        {
            disconnect(m_settingsOptions[index],
                       SIGNAL(sigUpdateCurrentElement(int)),
                       this,
                       SLOT(slotUpdateCurrentElement(int)));
            disconnect(m_settingsOptions[index],
                       SIGNAL(sigButtonClicked(int)),
                       this,
                       SLOT (slotSettingsOptions(int)));
            DELETE_OBJ(m_settingsOptions[index]);
        }
    }

    disconnect(deviceListDropDown,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(deviceListDropDown,
               SIGNAL(sigValueChanged(QString,quint32)),
               this,
               SLOT(slotSpinBoxValueChange(QString,quint32)));
    delete deviceListDropDown;
}

void Settings::createSubMenuOptions(quint8 optionIndex,quint8 tempSubIndex)
{
    subMenuCount = maxSubOptionCount[optionIndex];
    if (subMenuCount == 0)
    {
        return;
    }

    const QString* settings = maxSubOptionMenuLabel[optionIndex];
    for(quint8 index = (optionIndex+ 1); index < maxSettingControlElement; index++)
    {
        m_elementList[STG_SETTING_OPTION + index] = NULL;
    }

    maxSettingControlElement += subMenuCount;
    for(quint8 index = 0; index < subMenuCount; index++)
    {
        if (((optionIndex == NETWORK_SETTTING) && (index == NETWORK_LAN2) && (devTableInfo.numOfLan == 1))
            || ((optionIndex == NETWORK_SETTTING) && (index == NETWORK_MEDIA_FILE_ACCESS) && (isMediaFileAccessDisp == false))
            || ((optionIndex == NETWORK_SETTTING) && (index == NETWORK_SMS) && (isSmsDisp == false))
            || ((optionIndex == NETWORK_SETTTING) && (index == NETWORK_P2P) && (isP2pDisp == false))
            || ((optionIndex == NETWORK_SETTTING) && (index == NETWORK_MATRIX_DNS_CLIENT) && (isMatrixDnsClientDisp == false))
            || ((optionIndex == NETWORK_SETTTING) && (index == NETWORK_TCP_CLIENT) && (isTcpClientDisp == false))
            || ((optionIndex == EVENT_AND_ACTION_SETTING) && (index == EVENT_DEVICE_SENSOR) && (devTableInfo.sensors == 0) && ((currState == CONNECTED) || (currState == CONFLICT)))
            || ((optionIndex == STORAGE_AND_BACKUP_SETTING) && (index == STORAGE_HDD_GROUP) && (isHddGroupDisp == false)))
        {
            m_settingsSubOptions[index] = NULL;
            m_elementList[STG_SETTING_OPTION + optionIndex + 1 + index] = NULL;
            continue;
        }

        m_settingsSubOptions[index] = new MenuButton(index, SUBCELL_WIDTH, CELL_HEIGHT,
                                                     settings[index],
                                                     this,
                                                     SUBCELL_HORIZONTAL_OFFSET,
                                                     SUBCELL_X_OFFSET,
                                                     STNG_TOP_MARGIN + CELL_HEIGHT + ( CELL_HEIGHT * optionIndex) ,
                                                     (STG_SETTING_OPTION + optionIndex + 1 + index),
                                                     true,
                                                     true,
                                                     false);
        m_elementList[(STG_SETTING_OPTION + optionIndex + 1 + index)] =  m_settingsSubOptions[index];
        connect(m_settingsSubOptions[index],
                SIGNAL(sigButtonClicked(int)),
                this,
                SLOT (slotSubSettingsOptions(int)));
        connect(m_settingsSubOptions[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

        if ((optionIndex > DEVICE_IO_SETTING) && ((currState == CONNECTED) || (currState == CONFLICT))
                && ((devTableInfo.sensors == 0) && (devTableInfo.alarms == 0)))
        {
           m_settingsSubOptions[index]->resetGeometry(0,-1);
        }
    }

    for(quint8 index = (optionIndex + 1); index < MAX_SETTING_OPTION; index++)
    {
        m_elementList[STG_SETTING_OPTION + subMenuCount + (index)] =  m_settingsOptions[index];
    }

    if (optionIndex == NETWORK_SETTTING)
    {
        if (isP2pDisp == false)
        {
            subMenuCount--;
        }

        if (isSmsDisp == false)
        {
            for(quint8 index = NETWORK_P2P; index < subMenuCount; index++)
            {
                m_settingsSubOptions[index]->resetGeometry(0,-1);
            }
            subMenuCount--;
        }

        if (isMediaFileAccessDisp == false)
        {
            for(quint8 index = NETWORK_SMS; index < subMenuCount; index++)
            {
                m_settingsSubOptions[index]->resetGeometry(0,-1);
            }
            subMenuCount--;
        }

        if (isTcpClientDisp == false)
        {
            for(quint8 index = NETWORK_MEDIA_FILE_ACCESS; index < subMenuCount; index++)
            {
                m_settingsSubOptions[index]->resetGeometry(0,-1);
            }
            subMenuCount--;
        }

        if (isMatrixDnsClientDisp == false)
        {
            for(quint8 index = NETWORK_DDNS_CLIENT; index < subMenuCount; index++)
            {
                m_settingsSubOptions[index]->resetGeometry(0,-1);
            }
            subMenuCount--;
        }

        if (devTableInfo.numOfLan == 1)
        {
            for(quint8 index = NETWORK_DHCP_SERVER; index < subMenuCount; index++)
            {
                m_settingsSubOptions[index]->resetGeometry(0,-1);
            }
            subMenuCount--;
        }
    }

    if ((optionIndex == EVENT_AND_ACTION_SETTING) && ((currState == CONNECTED) || (currState == CONFLICT))
            && ((devTableInfo.sensors == 0) && (devTableInfo.alarms == 0)))
    {
        for(quint8 index = EVENT_SYSTEM; index < subMenuCount; index++)
        {
            m_settingsSubOptions[index]->resetGeometry(0,-2);
        }
        subMenuCount--;
    }

    if (optionIndex == STORAGE_AND_BACKUP_SETTING)
    {
        if (isHddGroupDisp == false)
        {
            for(quint8 index = STORAGE_NETWORK_DRIVE; index < subMenuCount; index++)
            {
                m_settingsSubOptions[index]->resetGeometry(0,-1);
            }
            subMenuCount--;
        }
    }

    m_settingsSubOptions[tempSubIndex]->setShowClickedImage (true);
    changeSubPageHeading (maxSubOptionPageHeadings[optionIndex][tempSubIndex]);

    for(quint8 index = (optionIndex + 1); index < MAX_SETTING_OPTION; index++)
    {
        if ((index > DEVICE_IO_SETTING) && ((currState == CONNECTED) || (currState == CONFLICT))
                && ((devTableInfo.sensors == 0) && (devTableInfo.alarms == 0)))
        {
            m_settingsOptions[index]->resetGeometry (0, subMenuCount - 1);
        }
        else
        {
            m_settingsOptions[index]->resetGeometry (0, subMenuCount);
        }
    }

    m_currentElement = (STG_SETTING_OPTION + optionIndex + 1 + tempSubIndex);
}

void Settings::deleteSubMenuOption()
{
    if (subMenuCount == 0)
    {
        return;
    }

    if(((currentIndex == NETWORK_SETTTING) && (devTableInfo.numOfLan == 1))
        || ((currentIndex == NETWORK_SETTTING) && (isMediaFileAccessDisp == false))
        || ((currentIndex == NETWORK_SETTTING) && (isSmsDisp == false))
        || ((currentIndex == NETWORK_SETTTING) && (isP2pDisp == false))
        || ((currentIndex == NETWORK_SETTTING)  && (isMatrixDnsClientDisp == false))
        || ((currentIndex == NETWORK_SETTTING) && (isTcpClientDisp == false))
        || ((currentIndex == EVENT_AND_ACTION_SETTING) && (devTableInfo.sensors == 0) && ((currState == CONNECTED) || (currState == CONFLICT)))
        || ((currentIndex == STORAGE_AND_BACKUP_SETTING) && (isHddGroupDisp == false)))
    {
        subMenuCount++;
    }

    for(quint8 index = 0; index < subMenuCount; index++)
    {
        if (IS_VALID_OBJ(m_settingsSubOptions[index]))
        {
            disconnect(m_settingsSubOptions[index],
                       SIGNAL(sigUpdateCurrentElement(int)),
                       this,
                       SLOT(slotUpdateCurrentElement(int)));
            disconnect(m_settingsSubOptions[index],
                       SIGNAL(sigButtonClicked(int)),
                       this,
                       SLOT (slotSubSettingsOptions(int)));
            DELETE_OBJ(m_settingsSubOptions[index]);
        }
    }

    for(quint8 index = 0 ;index < MAX_SETTING_OPTION;index++)
    {
        if ((index > DEVICE_IO_SETTING) && ((currState == CONNECTED) || (currState == CONFLICT))
                && ((devTableInfo.sensors == 0) && (devTableInfo.alarms == 0)))
        {
            m_settingsOptions[index]->resetGeometry (0, -1);
        }
        else
        {
            m_settingsOptions[index]->resetGeometry (0, 0);
        }
        m_elementList[(index + STG_SETTING_OPTION)] = m_settingsOptions[index];
    }

    maxSettingControlElement = MAX_SETTING_CONTROL_ELEMENT;
    subMenuCount = 0;
    m_currentElement = (currentIndex + STG_SETTING_OPTION);
    currentSubIndex = 0;
}

void Settings::createSubPageObject(quint8 optionIndex, quint8 subOptionIndex)
{
    subElementFocusListEnable = false;

    currentIndex = optionIndex;
    currentSubIndex = subOptionIndex;
    switch(currentIndex)
    {
        case BASIC_SETTING:
        {
            switch(currentSubIndex)
            {
                case BASIC_GERENAL:
                    configPageControl = new GeneralSetting(currDevName, this, &devTableInfo);
                    break;

                case BASIC_DATE_AND_TIME:
                    configPageControl = new DateTimeSetting(currDevName, this, &devTableInfo);
                    break;

                case BASIC_DAYLIGHT_SAVING_TIME:
                    configPageControl = new  DayLightSaving(currDevName, this);
                    break;

                default:
                    break;
            }
        }
        break;

        case CAMERA_SETTING:
        {
            switch(currentSubIndex)
            {
                case CAMERA_SEARCH:
                    configPageControl = new CameraSearch(currDevName, this, &devTableInfo);
                    break;

                case CAMERA_CAMERA:
                    configPageControl = new CameraSettings(currDevName, this, &devTableInfo);
                    connect(configPageControl,
                            SIGNAL(sigOpenCameraFeature(void*,CAMERA_FEATURE_TYPE_e,quint8,void*,QString)),
                            this,
                            SLOT(slotOpenCameraFeature(void*,CAMERA_FEATURE_TYPE_e,quint8,void*,QString)));
                    break;

                case CAMERA_STREAM:
                    configPageControl = new IpStreamSettings(currDevName, this, &devTableInfo);
                    break;

                case CAMERA_IMAGE_SETTINGS:
                    configPageControl = new ImageSettings(currDevName, this, &devTableInfo);
                    break;

                case CAMERA_RECORDING:
                    configPageControl = new Recording(currDevName, this, &devTableInfo);
                    break;

                case CAMERA_SNAP_SCHD:
                    configPageControl = new SnapshotSchedule(currDevName, this, &devTableInfo);
                    break;

                case CAMERA_IMAGE_UPLOAD:
                    configPageControl = new UploadImage(currDevName, this, &devTableInfo);
                    break;

                case CAMERA_AUDIO:
                    configPageControl = new AudioSettings(currDevName, this, &devTableInfo);
                    break;

                case CAMERA_PTZ_TOUR_SETTINGS:
                    configPageControl = new PresetTour(currDevName, this, &devTableInfo);
                    break;

                case CAMERA_ALARM:
                    configPageControl = new CameraAlarmOutput(currDevName, this, &devTableInfo);
                    break;

                default:
                    break;
            }
        }
        break;

        case NETWORK_SETTTING:
        {
            switch(currentSubIndex)
            {
                case NETWORK_LAN:
                    configPageControl = new Lan1Setting(currDevName, this);
                    break;

                case NETWORK_LAN2:
                    if(devTableInfo.numOfLan == 2)
                    {
                        configPageControl = new Lan2Setting(currDevName, this);
                    }
                    break;

                case NETWORK_DHCP_SERVER:
                    configPageControl = new DhcpServerSetting(currDevName, this, &devTableInfo);
                    break;

                case NETWORK_BROADBAND:
                    configPageControl = new BroadbandSetting(currDevName, this);
                    break;

                case NETWORK_STATIC_ROUTING:
                    configPageControl = new StaticRouting(currDevName, this, &devTableInfo);
                    break;

                case NETWORK_IPADDRESS_FILTERING:
                    configPageControl = new IpFiltering(currDevName, this);
                    break;

                case NETWORK_MATRIX_DNS_CLIENT:
                    if(isMatrixDnsClientDisp == true)
                    {
                        configPageControl = new MatrixDnsClient(currDevName, this);
                    }
                    break;

                case NETWORK_DDNS_CLIENT:
                    configPageControl = new DDNSClient(currDevName, this);
                    break;

                case NETWORK_EMAIL_CLIENT:
                    configPageControl = new EmailClient(currDevName, this);
                    break;

                case NETWORK_FTP_CLIENT:
                    configPageControl = new FtpClient(currDevName, this);
                    break;

                case NETWORK_TCP_CLIENT:
                    if(isTcpClientDisp == true)
                    {
                        configPageControl = new TcpNotification(currDevName, this);
                    }
                    break;

                case NETWORK_MEDIA_FILE_ACCESS:
                    if (isMediaFileAccessDisp == true)
                    {
                        configPageControl = new MediaFileAccess(currDevName, this);
                    }
                    break;

                case NETWORK_SMS:
                    if (isSmsDisp == true)
                    {
                        configPageControl = new SmsSetting(currDevName, this);
                    }
                    break;

                case NETWORK_P2P:
                    if (isP2pDisp == true)
                    {
                        configPageControl = new P2pSetting(currDevName, this);
                    }
                    break;

                default:
                    break;
            }
        }
        break;

        case STORAGE_AND_BACKUP_SETTING:
        {
            switch(currentSubIndex)
            {
                case STORAGE_HDD:
                    configPageControl = new HDDMangment(currDevName, this, &devTableInfo);
                    break;

                case STORAGE_HDD_GROUP:
                    if (isHddGroupDisp == true)
                    {
                        configPageControl = new HDDGroupManagement(currDevName, this, &devTableInfo);
                    }
                    break;

                case STORAGE_NETWORK_DRIVE:
                    configPageControl = new NetworkDrive(currDevName, this);
                    break;

                case STORAGE_STORAGE:
                    configPageControl = new StorageManagment(currDevName, this, &devTableInfo);
                    break;

                case STORAGE_BACKUP:
                    configPageControl = new BackupManagment(currDevName, this, &devTableInfo);
                    break;

                case STORAGE_USB:
                    configPageControl = new USBManagment(currDevName, this);
                    break;

                default:
                    break;
            }
        }
        break;

        case DEVICE_IO_SETTING:
        {
            switch(currentSubIndex)
            {
                case SENSOR_INPUT:
                    configPageControl = new DeviceSensorInput(currDevName, this, &devTableInfo);
                    break;

                case ALARM_OUTPUT:
                    configPageControl = new DeviceAlarmOutput(currDevName, this, &devTableInfo);
                    break;

                default:
                    break;
            }
        }
        break;

        case USER_ACCOUNT_MANAGMENT_SETTING:
        {
            switch (currentSubIndex)
            {
                case USER_ACCOUNT:
                    configPageControl = new UserAccountManagment(currDevName, this, &devTableInfo);
                    break;

                case PASSWORD_POLICY:
                    configPageControl = new PasswordPolicy(currDevName, this, &devTableInfo);
                    break;

                case PUSH_NOTIFICATION_STATUS:
                    configPageControl = new PushNotification(currDevName, this, &devTableInfo);
                    break;

                default:
                    break;
            }
        }
        break;

        case EVENT_AND_ACTION_SETTING:
        {
            switch(currentSubIndex)
            {
                case EVENT_CAMERA:
                    configPageControl = new CameraEventAndEventAction(currDevName, this, &devTableInfo);
                    break;

                case EVENT_DEVICE_SENSOR:
                    configPageControl = new SensorEventAndAction(currDevName, this, &devTableInfo);
                    break;

                case EVENT_SYSTEM:
                    configPageControl = new SystemEventAndAction(currDevName, this, &devTableInfo);
                    break;

                default:
                    break;
            }
        }
        break;

        case MAINTENANCE:
        {
            switch(currentSubIndex)
            {
                case FIRMWARE_MANAGEMENT:
                    configPageControl = new FirmwareManagement(currDevName, this, &devTableInfo);
                    break;

                default:
                    break;
            }
        }
        break;

        case DEVICES:
        {
            configPageControl = new DeviceSetting(currDevName,this);
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void Settings::deleteSubPageObject()
{
    if (IS_VALID_OBJ(configPageControl))
    {
        if ((currentIndex == CAMERA_SETTING) && (currentSubIndex == CAMERA_CAMERA))
        {
            disconnect(configPageControl,
                       SIGNAL(sigOpenCameraFeature(void*,CAMERA_FEATURE_TYPE_e,quint8,void*,QString)),
                       this,
                       SLOT(slotOpenCameraFeature(void*,CAMERA_FEATURE_TYPE_e,quint8,void*,QString)));
        }

        DELETE_OBJ(configPageControl);
    }
}

void Settings::loadSubPage (quint8 optionIndex, quint8 subIndex)
{
    changeSubPageHeading(settingsOptionHeadings[optionIndex]);

    bool isNewObjectToCreate = loadSubPageMenu(optionIndex,subIndex);
    if(currentIndex != MAX_SETTING_OPTION)
    {
        m_settingsOptions[currentIndex]->setShowClickedImage(false);
    }

    if(currentIndex != optionIndex)
    {
        m_settingsOptions[optionIndex]->setShowClickedImage(true);
        currentIndex = optionIndex;
    }
    else
    {
        prevOptionIndex = 0;
        currentIndex = MAX_SETTING_OPTION;
    }

    if(isNewObjectToCreate)
    {
        createSubPageObject(optionIndex, subIndex);
    }

    m_subelementList[0] = m_subCloseButton;
    m_subelementList[1] = configPageControl;

    m_subcurrentElement = 1;
    m_elementList[m_currentElement]->forceActiveFocus();
}

bool Settings::loadSubPageMenu(quint8 optionIndex,quint8 subIndex)
{
    bool status = false;

    if(currentIndex == optionIndex)
    {
        deleteSubMenuOption ();

        if(prevOptionIndex != currentIndex)
        {
            deleteSubPageObject ();
            createSubPageObject (optionIndex, subIndex);
        }
        m_settingsOptions[currentIndex]->setShowClickedImage(false);
    }
    else
    {
        if(prevOptionIndex != currentIndex)
        {
            prevOptionIndex = currentIndex;
            deleteSubPageObject();
            status = true;
        }
        deleteSubMenuOption ();
        createSubMenuOptions (optionIndex,subIndex);
    }

    return status;
}

bool Settings::isSameSubObject(quint8 subOptionIndex)
{
    bool status = false;

    changeSubPageHeading (maxSubOptionPageHeadings[currentIndex][subOptionIndex]);

    if(currentSubIndex != subOptionIndex)
    {
        deleteSubPageObject();
        status = true;
    }
    return status;
}

void Settings::takeLeftKeyAction()
{
    if(subElementFocusListEnable == false)
    {
        do
        {
            m_currentElement = (m_currentElement - 1 + maxSettingControlElement) % maxSettingControlElement;
        }while((m_elementList[m_currentElement] == NULL) || (!m_elementList[m_currentElement]->getIsEnabled()));

        m_elementList[m_currentElement]->forceActiveFocus();
    }
    else
    {
        if(m_subcurrentElement == 0)
        {
            m_subcurrentElement = 1;
            m_subelementList[m_subcurrentElement]->forceFocusToPage (false);
        }
        else
        {
            m_subcurrentElement = 0;
            m_subelementList[m_subcurrentElement]->forceActiveFocus ();
        }
    }
}

void Settings::takeRightKeyAction()
{
    if(subElementFocusListEnable == false)
    {
        do
        {
            m_currentElement = ((m_currentElement + 1) % maxSettingControlElement);
        }while((m_elementList[m_currentElement] == NULL) || (!m_elementList[m_currentElement]->getIsEnabled()));

        m_elementList[m_currentElement]->forceActiveFocus();
    }
    else
    {
        if(m_subcurrentElement == 0)
        {
            m_subcurrentElement = 1;
            m_subelementList[m_subcurrentElement]->forceFocusToPage (true);
        }
        else
        {
            m_subcurrentElement = 0;
            m_subelementList[m_subcurrentElement]->forceActiveFocus ();
        }
    }
}

void Settings::takeMenuKeyAction()
{
    if(isRightPanelOpen)
    {
        if(subElementFocusListEnable == false)
        {
            subElementFocusListEnable = true;
            m_subcurrentElement = 1;
            m_subelementList[m_subcurrentElement]->forceFocusToPage (true);
        }
        else
        {
            subElementFocusListEnable = false;
            m_elementList[m_currentElement]->forceActiveFocus();
        }
    }
}

void Settings::hideEvent(QHideEvent * event)
{
    QWidget::hideEvent (event);
    if(infoPage->isVisible())
    {
        infoPage->unloadInfoPage();
    }
}

void Settings::showEvent (QShowEvent* event)
{
    QWidget::showEvent (event);

    if(!isRightPanelOpen)
    {
        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

void Settings::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void Settings::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void Settings::navigationKeyPressed (QKeyEvent *event)
{
    event->accept();
}

void Settings::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    if(subElementFocusListEnable)
    {
        m_subcurrentElement = 0;
        m_subelementList[m_subcurrentElement]->forceActiveFocus ();
    }
    else
    {
        m_currentElement = STG_CLOSE_BUTTON;
        m_elementList[m_currentElement]->forceActiveFocus ();
    }
}

void Settings::insertKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeMenuKeyAction();
}

void Settings::rightPanelClose()
{
    deleteSubPageObject();

    this->resetGeometry(ApplController::getXPosOfScreen() + ((ApplController::getWidthOfScreen() - SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH)) / 2));

    deleteSubMenuOption();

    for(quint8 index = 0; index < MAX_SETTING_OPTION; index ++)
    {
        m_settingsOptions[index]->setShowClickedImage (false);
    }

    subElementFocusListEnable = false;
    isRightPanelOpen = false;

    prevOptionIndex = 0;
    currentIndex = MAX_SETTING_OPTION;
    prevSubOptionIndex = MAX_SETTING_OPTION;
    m_currentElement = STG_DROPDOWNBOX;
    m_elementList[m_currentElement]->forceActiveFocus ();

    //close left panel also if it is disabled
    if(IS_VALID_OBJ(m_leftPanelDisableImg) && (m_leftPanelDisableImg->isVisible()))
    {
        emit sigClosePage(SETTINGS_BUTTON);
    }
}

void Settings::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if(configPageControl != NULL)
    {
        configPageControl->processDeviceResponse (param, deviceName);
    }
}

void Settings::disconnectDeviceNotify (QString deviceName, bool forcePopup)
{
    QString dispDevName = deviceListDropDown->getCurrValue();
    if(applController->GetRealDeviceName(dispDevName) == deviceName)
    {
        if (isRightPanelOpen == true)
        {
            hideRightpanel();
            rightPanelClose();
            infoPage->loadInfoPage (dispDevName + " " + ValidationMessage::getDeviceResponceMessage(CMD_DEV_DISCONNECTED));
        }
        else if (forcePopup == true)
        {
            infoPage->loadInfoPage (dispDevName + " " + ValidationMessage::getDeviceResponceMessage(CMD_DEV_DISCONNECTED));
        }
    }
}

void Settings::loadPage(quint8 optionIndex, quint8 subIndex)
{
    this->resetGeometry (ApplController::getXPosOfScreen() + ((ApplController::getWidthOfScreen() - SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH) - SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH)) / 2));
    showRightpanel();
    isRightPanelOpen = true;
    loadSubPage(optionIndex,subIndex);
}

void Settings::loadCamsearchOnAutoConfig()
{
    if(IS_VALID_OBJ(configPageControl))
    {
        configPageControl->loadCamsearchOnAutoConfig();
    }
    if(!IS_VALID_OBJ(m_leftPanelDisableImg))
    {
        m_leftPanelDisableImg = new Image(0,
                                          0,
                                          SETTING_LEFT_PANEL_DISABLE_IMG_PATH,
                                          this,
                                          START_X_START_Y,
                                          0,
                                          false,
                                          true);
    }
}

bool Settings::getDisableLeftPanelState()
{
    return IS_VALID_OBJ(m_leftPanelDisableImg);
}

bool Settings::getIsRightPanelEnable()
{
    return isRightPanelOpen;
}

SETTING_OPTION_e Settings::getCurrentSettingMenu()
{
    return (SETTING_OPTION_e)currentIndex;
}

SUB_SETTING_OPTION_e Settings::getCurrentSettingSubMenu()
{
    return (SUB_SETTING_OPTION_e)currentSubIndex;
}

void Settings::updateCamSearchList(DevCommParam *param)
{
    if((IS_VALID_OBJ(configPageControl)) && (param->payload != ""))
    {
        configPageControl->updateList(param);
    }
}

void Settings::updateDeviceList()
{
    QMap<quint8, QString> deviceMapList;
    applController->GetDevNameDropdownMapList(deviceMapList);

    /* Check if selected device found in new updated list or not. It is possible that index of that device name may changed.
     * Hence update device list with current device index */
    for (quint8 deviceIndex = 0; deviceIndex < deviceMapList.count(); deviceIndex++)
    {
        if (deviceListDropDown->getCurrValue() == deviceMapList.value(deviceIndex))
        {
            deviceListDropDown->setNewList(deviceMapList, deviceIndex);
            return;
        }
    }

    deviceListDropDown->setNewList(deviceMapList, 0);
    slotSpinBoxValueChange(deviceMapList.value(0), STG_DROPDOWNBOX);
}

void Settings::loadProcessBar()
{
    if((currentIndex == CAMERA_SETTING) && (currentSubIndex == CAMERA_CAMERA))
    {
        configPageControl->loadProcessBar();
    }
}

void Settings::slotSpinBoxValueChange(QString newName, quint32 indexInPage)
{
    newName = applController->GetRealDeviceName(newName);
    if((indexInPage != STG_DROPDOWNBOX) || (currDevName == newName))
    {
        return;
    }

    currDevName = newName;
    hideRightpanel();
    rightPanelClose();

    if(newName == LOCAL_DEVICE_NAME)
    {
        m_settingsOptions[DEVICES]->setVisible(true);
        m_settingsOptions[DEVICES]->setIsEnabled(true);
    }
    else
    {
        m_settingsOptions[DEVICES]->setVisible(false);
        m_settingsOptions[DEVICES]->setIsEnabled(false);
    }

    applController->GetDeviceInfo (currDevName,devTableInfo);
    currState = applController->GetDeviceConnectionState(newName);

    #if defined(OEM_JCI)
    isHddGroupDisp = true;
    isMediaFileAccessDisp = false;
    isSmsDisp = false;
    isP2pDisp = false;
    isTcpClientDisp = false;
    isMatrixDnsClientDisp = false;
    #else
    if (((devTableInfo.productVariant >= NVR3202XP2) && (devTableInfo.productVariant <= NVR6408XP2)) || (devTableInfo.productVariant == NVR9608XP2))
    {
        isHddGroupDisp = true;
        isMediaFileAccessDisp = false;
    }
    else
    {
        isHddGroupDisp = false;
        isMediaFileAccessDisp = true;
    }
    isSmsDisp = true;
    isP2pDisp = true;
    isTcpClientDisp = true;
    isMatrixDnsClientDisp = true;
    #endif

    if (((currState == CONNECTED) || (currState == CONFLICT)) && ((devTableInfo.sensors == 0) && (devTableInfo.alarms == 0)))
    {
        m_settingsOptions[DEVICE_IO_SETTING]->setIsEnabled(false);
        for(quint8 index = USER_ACCOUNT_MANAGMENT_SETTING; index < MAX_SETTING_OPTION; index++)
        {
            m_settingsOptions[index]->resetGeometry(0,-1);
        }
    }
    else
    {
        m_settingsOptions[DEVICE_IO_SETTING]->setIsEnabled(true);
        for(quint8 index = DEVICE_IO_SETTING; index < MAX_SETTING_OPTION; index++)
        {
            m_settingsOptions[index]->resetGeometry(0,0);

        }
    }
}

void Settings::slotSettingsOptions(int optionIndex)
{
    if(applController->GetDeviceConnectionState (currDevName) == CONFLICT)
    {
        if(isRightPanelOpen == true)
        {
            hideRightpanel();
            rightPanelClose();
        }

        m_settingsOptions[optionIndex]->setShowClickedImage(false);

        QString str = ValidationMessage::getValidationMessage (DEVICE_FIRMWARE_MISMATCH_SERVER_OLD);
        if(applController->GetDeviceInfo (currDevName,devTableInfo))
        {
            if(devTableInfo.deviceConflictType == MX_DEVICE_CONFLICT_TYPE_SERVER_NEW)
            {
                str = ValidationMessage::getValidationMessage (DEVICE_FIRMWARE_MISMATCH_SERVER_NEW);
            }
        }
        infoPage->loadInfoPage (str);
    }
    else if(applController->GetDeviceInfo (currDevName,devTableInfo))
    {
        if(devTableInfo.userGroupType == VIEWER)
        {
            MessageBanner::addMessageInBanner (ValidationMessage::getDeviceResponceMessage(CMD_NO_PRIVILEGE));
            m_settingsOptions[optionIndex]->setShowClickedImage(false);
        }
        else
        {
            loadPage (optionIndex);
        }
    }
    else
    {
        currentClickedIndex = optionIndex;
        disconnectDeviceNotify (currDevName, true);
    }
}

void Settings::slotSubSettingsOptions(int subOptionIndex)
{
    bool isNewSubObjectToCreate = false;
    bool makeForceActive = true;

    isNewSubObjectToCreate = isSameSubObject(subOptionIndex);

    m_settingsSubOptions[currentSubIndex]->setShowClickedImage (false);
    m_settingsSubOptions[subOptionIndex]->setShowClickedImage (true);
    currentSubIndex = (subOptionIndex);

    if(isNewSubObjectToCreate)
    {
        createSubPageObject(currentIndex, currentSubIndex);
    }
    m_subelementList[0] = m_subCloseButton;
    m_subelementList[1] = configPageControl;

    m_subcurrentElement = 1;
    if((m_elementList[m_currentElement] != NULL) && (makeForceActive == true))
    {
        m_elementList[m_currentElement]->forceActiveFocus ();
    }
}

void Settings::slotUpdateCurrentElement(int index)
{
    m_currentElement = index;
}

void Settings::slotOpenCameraFeature(void *param, CAMERA_FEATURE_TYPE_e featureType, quint8 cameraIndex, void* configParam, QString devName)
{
    emit sigOpenCameraFeature(param, featureType, cameraIndex, configParam, devName);
}

void Settings::slotChangeToolbarButtonState (TOOLBAR_BUTTON_TYPE_e buttonIndex, STATE_TYPE_e state)
{
    emit sigChangeToolbarButtonState(buttonIndex,state);
}

void Settings::slotInfoPageBtnclick(int)
{
    if(currentClickedIndex != MAX_SETTING_OPTION)
    {
        m_settingsOptions[currentClickedIndex]->setShowClickedImage(false);
    }
    m_elementList[m_currentElement]->forceActiveFocus ();
}
