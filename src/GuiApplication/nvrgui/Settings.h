#ifndef SETTINGS_H
#define SETTINGS_H

#include <QWidget>

#include "ApplController.h"
#include "DataStructure.h"
#include "PayloadLib.h"
#include "Controls/BackGround.h"
#include "Controls/MenuButton.h"
#include "Controls/DropDown.h"
#include "Controls/ConfigPageControl.h"
#include "Controls/InfoPage.h"

#include "ConfigPages/BasicSettings/GeneralSetting.h"
#include "ConfigPages/BasicSettings/DateTimeSetting.h"
#include "ConfigPages/BasicSettings/DayLightSaving.h"

#include "ConfigPages/CameraSettings/CameraSearch.h"
#include "ConfigPages/CameraSettings/CameraSettings.h"
#include "ConfigPages/CameraSettings/Recording.h"
#include "ConfigPages/CameraSettings/UploadImage.h"
#include "ConfigPages/CameraSettings/ImageSettings.h"
#include "ConfigPages/CameraSettings/AudioSettings.h"
#include "ConfigPages/CameraSettings/IpStreamSettings.h"
#include "ConfigPages/CameraSettings/PresetTour.h"
#include "ConfigPages/CameraSettings/CameraAlarmOutput.h"
#include "ConfigPages/CameraSettings/SnapshotSchedule.h"

#include "ConfigPages/NetworkSettings/Lan1Setting.h"
#include "ConfigPages/NetworkSettings/Lan2Setting.h"
#include "ConfigPages/NetworkSettings/BroadbandSetting.h"
#include "ConfigPages/NetworkSettings/StaticRouting.h"
#include "ConfigPages/NetworkSettings/IpFiltering.h"
#include "ConfigPages/NetworkSettings/MatrixDnsClient.h"
#include "ConfigPages/NetworkSettings/DDNSClient.h"
#include "ConfigPages/NetworkSettings/EmailClient.h"
#include "ConfigPages/NetworkSettings/FtpClient.h"
#include "ConfigPages/NetworkSettings/TcpNotification.h"
#include "ConfigPages/NetworkSettings/MediaFileAccess.h"
#include "ConfigPages/NetworkSettings/SmsSetting.h"
#include "ConfigPages/NetworkSettings/P2pSetting.h"
#include "ConfigPages/NetworkSettings/DhcpServerSetting.h"

#include "ConfigPages/StorageAndBackupSettings/HDDMangment.h"
#include "ConfigPages/StorageAndBackupSettings/HDDGroupManagement.h"
#include "ConfigPages/StorageAndBackupSettings/StorageManagment.h"
#include "ConfigPages/StorageAndBackupSettings/USBManagment.h"
#include "ConfigPages/StorageAndBackupSettings/BackupManagment.h"
#include "ConfigPages/StorageAndBackupSettings/NetworkDrive.h"

#include "ConfigPages/DeviceIOSettings/DeviceSensorInput.h"
#include "ConfigPages/DeviceIOSettings/DeviceAlarmOutput.h"

#include "ConfigPages/UserAccountManagmentSettings/UserAccountManagment.h"
#include "ConfigPages/UserAccountManagmentSettings/PasswordPolicy.h"
#include "ConfigPages/UserAccountManagmentSettings/PushNotificationStatus.h"

#include "ConfigPages/EventAndActionSettings/CameraEventAndEventAction.h"
#include "ConfigPages/EventAndActionSettings/SensorEventAndAction.h"
#include "ConfigPages/EventAndActionSettings/SystemEventAndAction.h"

#include "ConfigPages/Maintenance/FirmwareManagement.h"

#include "ConfigPages/Devices/DeviceSetting.h"

#define MAX_SUB_SETTING_CONTOL_ELEMENT  2
#define MAX_SUB_SETTING_OPTION          15

typedef enum
{
    BASIC_SETTING,
    CAMERA_SETTING,
    NETWORK_SETTTING,
    STORAGE_AND_BACKUP_SETTING,
    DEVICE_IO_SETTING,
    USER_ACCOUNT_MANAGMENT_SETTING,
    EVENT_AND_ACTION_SETTING,
    MAINTENANCE,
    DEVICES,
    MAX_SETTING_OPTION
}SETTING_OPTION_e;

typedef enum
{
    STG_CLOSE_BUTTON,
    STG_DROPDOWNBOX,
    STG_SETTING_OPTION,
    MAX_SETTING_CONTROL_ELEMENT = (STG_SETTING_OPTION + MAX_SETTING_OPTION)
}SETTING_CONTROL_ELEMENT_e;

typedef enum
{
    BASIC_GERENAL = 0,
    BASIC_DATE_AND_TIME,
    BASIC_DAYLIGHT_SAVING_TIME,
    MAX_BASIC_SUBSETTING_OPTION,

    CAMERA_SEARCH = 0,
    CAMERA_CAMERA,
    CAMERA_STREAM,
    CAMERA_IMAGE_SETTINGS,
    CAMERA_RECORDING,
    CAMERA_SNAP_SCHD,
    CAMERA_IMAGE_UPLOAD,
    CAMERA_PTZ_TOUR_SETTINGS,
    CAMERA_AUDIO,
    CAMERA_ALARM,
    MAX_CAMERA_SUBSETTING_OPTION,

    NETWORK_LAN  = 0,
    NETWORK_LAN2,
    NETWORK_DHCP_SERVER,
    NETWORK_BROADBAND,
    NETWORK_STATIC_ROUTING,
    NETWORK_IPADDRESS_FILTERING,
    NETWORK_MATRIX_DNS_CLIENT,
    NETWORK_DDNS_CLIENT,
    NETWORK_EMAIL_CLIENT,
    NETWORK_FTP_CLIENT,
    NETWORK_TCP_CLIENT,
    NETWORK_MEDIA_FILE_ACCESS,
    NETWORK_SMS,
    NETWORK_P2P,
    MAX_NETWORK_SUBSETTING_OPTION,

    STORAGE_HDD = 0,
    STORAGE_HDD_GROUP,
    STORAGE_NETWORK_DRIVE,
    STORAGE_STORAGE,
    STORAGE_BACKUP,
    STORAGE_USB,
    MAX_STORAGE_SUBSETTING_OPTION,

    SENSOR_INPUT = 0,
    ALARM_OUTPUT,
    MAX_DEVICE_IO_SUBSETTING_OPTION,

    USER_ACCOUNT = 0,
    PASSWORD_POLICY,
    PUSH_NOTIFICATION_STATUS,
    MAX_USER_ACCOUNT_SUBSETTING_OPTION,

    EVENT_CAMERA = 0,
    EVENT_DEVICE_SENSOR,
    EVENT_SYSTEM,
    MAX_EVENT_SUBSETTING_OPTION,

    FIRMWARE_MANAGEMENT = 0,
    MAX_MAINTENANCE_SUBSETTING_OPTION,

    MAX_DEVICE_SUBSETTING_OPTION = 0,
}SUB_SETTING_OPTION_e;

class Settings : public BackGround
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = 0);
    ~Settings();

    void updateDeviceList();

    void disconnectDeviceNotify(QString deviceName, bool forcePopup);
    void loadPage(quint8 optionIndex, quint8 subIndex = 0);

    void loadProcessBar();
    void processDeviceResponse(DevCommParam *param, QString deviceName);

    void rightPanelClose();

    void hideEvent(QHideEvent *event);
    void showEvent (QShowEvent* event);

    void loadCamsearchOnAutoConfig();
    bool getDisableLeftPanelState();
    bool getIsRightPanelEnable(void);
    SETTING_OPTION_e getCurrentSettingMenu();
    SUB_SETTING_OPTION_e getCurrentSettingSubMenu();
    void updateCamSearchList(DevCommParam *param);

    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);
    virtual void insertKeyPressed(QKeyEvent *event);
    virtual void backTab_KeyPressed(QKeyEvent *event);
    virtual void tabKeyPressed(QKeyEvent *event);

signals:
    void sigOpenCameraFeature(void *param, CAMERA_FEATURE_TYPE_e featureType, quint8 cameraIndex, void* configParam,QString devName);
    void sigChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e buttonIndex, STATE_TYPE_e state);

public slots:
    void slotSettingsOptions(int index);
    void slotSubSettingsOptions(int subOptionIndex);
    void slotSpinBoxValueChange(QString name, quint32 indexInPage);
    void slotUpdateCurrentElement(int index);
    void slotOpenCameraFeature(void * param, CAMERA_FEATURE_TYPE_e featureType, quint8 cameraIndex, void* configParam,QString devName);
    void slotChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e buttonIndex, STATE_TYPE_e state);
    void slotInfoPageBtnclick(int);

private:

    bool                isSubMenuOptionsCreated;
    bool                subElementFocusListEnable;
    bool                isRightPanelOpen;
    bool                isMediaFileAccessDisp; /* There is no FTP server support in RK3588 */
    bool                isHddGroupDisp;
    bool                isSmsDisp;
    bool                isP2pDisp;
    bool                isTcpClientDisp;
    bool                isMatrixDnsClientDisp;

    quint8              subMenuCount;
    quint8              prevOptionIndex;
    quint8              prevSubOptionIndex;
    quint8              currentClickedIndex;
    quint8              currentSubIndex;
    quint8              currentIndex;
    quint8              m_currentElement;
    quint8              m_subcurrentElement;
    quint16             maxSettingControlElement;

    QString             currDevName;
    DEV_TABLE_INFO_t    devTableInfo;

    DropDown*           deviceListDropDown;
    ApplController*     applController;
    InfoPage*           infoPage;

    MenuButton*         m_settingsOptions[MAX_SETTING_OPTION];
    MenuButton*         m_settingsSubOptions[MAX_SUB_SETTING_OPTION];

    ConfigPageControl*  configPageControl;
    NavigationControl*  m_elementList[MAX_SETTING_CONTROL_ELEMENT + MAX_SUB_SETTING_OPTION];
    NavigationControl*  m_subelementList[MAX_SUB_SETTING_CONTOL_ELEMENT];

    Image*              m_leftPanelDisableImg;
    DEVICE_STATE_TYPE_e currState;

    void deleteSubMenuOption();
    void createSubMenuOptions(quint8 optionIndex,quint8 tempSubIndex);
    bool loadSubPageMenu(quint8 optionIndex,quint8 subIndex = 0);
    void loadSubPage(quint8 optionIndex, quint8 subIndex = 0);
    void createSubPageObject(quint8 optionIndex, quint8 subOptionIndex);
    void deleteSubPageObject();
    bool isSameSubObject(quint8 subOptionIndex);

    void takeLeftKeyAction();
    void takeRightKeyAction();
    void takeMenuKeyAction();
};

#endif // SETTINGS_H
