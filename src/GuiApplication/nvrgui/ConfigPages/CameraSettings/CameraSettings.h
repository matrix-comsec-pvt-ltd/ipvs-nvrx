#ifndef CAMERASETTIGS_H
#define CAMERASETTIGS_H

#include "Controls/ConfigPageControl.h"
#include "Controls/SpinBox.h"
#include "Controls/TextBox.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/PageOpenButton.h"
#include "ConfigPages/CameraSettings/TestCamera.h"
#include "Controls/ControlButton.h"
#include "ConfigPages/CameraSettings/CameraOSDSettings.h"
#include "Controls/DropDown.h"
#include "ConfigPages/CameraSettings/IpChangeProcessRequest.h"
#include "CopyToCamera.h"
#include "DataStructure.h"
#include "Controls/PasswordTextbox.h"
#include "ConfigPages/CameraSettings/IpAddressChange.h"
#include "Controls/ToolTip.h"
#include "Controls/TextWithList.h"

typedef enum
{
    CAMERA_SETTINGS_IS_ENABLE_STATUS,
    CAMERA_SETTINGS_CAMERA_NAME,
    CAMERA_SETTINGS_CAMERA_TYPE,
    CAMERA_SETTINGS_LOG_DETECTION_STATUS,
    CAMERA_SETTINGS_MOTION_REDETECTION_DELAY,
    CAMERA_SETTINGS_CAMERA_NAME_OSD,
    CAMERA_SETTINGS_CAMERA_STATUS_OSD,
    CAMERA_SETTINGS_CAMERA_DATE_TIME_OVERLAY,
    CAMERA_SETTINGS_CAMERA_DATE_TIME_POSITION,
    CAMERA_SETTINGS_CAMERA_TEXT_OVERLAY,
    CAMERA_SETTINGS_CAMERA_TEXT_0,
    CAMERA_SETTINGS_CAMERA_TEXT_POSITION_0,
    CAMERA_SETTINGS_CAMERA_MOBILE_NUMBER,
    CAMERA_SETTINGS_PRIVACY_MASKING_STATUS,
    CAMERA_SETTINGS_MOTION_DETECTION_STATUS,
    CAMERA_SETTINGS_RECORDING_STREAM,
    CAMERA_SETTINGS_MOTION_DETECTION_COPY_TO_CAMERA_START,
    CAMERA_SETTINGS_MOTION_DETECTION_COPY_TO_CAMERA_END = CAMERA_SETTINGS_MOTION_DETECTION_COPY_TO_CAMERA_START + CAMERA_MASK_MAX - 1,
    CAMERA_SETTINGS_CAMERA_TEXT_1,
    CAMERA_SETTINGS_CAMERA_TEXT_POSITION_1,
    CAMERA_SETTINGS_CAMERA_TEXT_2,
    CAMERA_SETTINGS_CAMERA_TEXT_POSITION_2,
    CAMERA_SETTINGS_CAMERA_TEXT_3,
    CAMERA_SETTINGS_CAMERA_TEXT_POSITION_3,
    CAMERA_SETTINGS_CAMERA_TEXT_4,
    CAMERA_SETTINGS_CAMERA_TEXT_POSITION_4,
    CAMERA_SETTINGS_CAMERA_TEXT_5,
    CAMERA_SETTINGS_CAMERA_TEXT_POSITION_5,
    CAMERA_SETTINGS_FIELD_MAX
}CAMERA_SETTINGS_RECORD_e;

typedef enum
{
    IP_CAMERA_BRAND,
    IP_CAMERA_MODEL,
    IP_CAMERA_URL,
    IP_CAMERA_ADDRESS,
    IP_CAMERA_HTTP_PORT,
    IP_CAMERA_RTSP_PORT,
    IP_CAMERA_USERNAME,
    IP_CAMERA_PASSWORD,
    IP_CAMERA_ONVIF_SUPPORT,
    IP_CAMERA_ONVIF_PORT,
    IP_CAMERA_PROTOCOL,
    IP_CAMERA_MAC_ADDRESS,
    MAX_IP_CAMERA_SETTINGS_FIELDS
}IP_CAMERA_SETTINGS_FIELDS_e;

class CameraSettings : public ConfigPageControl
{
    Q_OBJECT
public:
    explicit CameraSettings(QString deviceName, QWidget* parent = 0, DEV_TABLE_INFO_t *devTabInfo = NULL);
    ~CameraSettings();

    void processDeviceResponse (DevCommParam *param, QString deviceName);
    void getConfig();
    void defaultConfig();
    void saveConfig();
    void loadProcessBar();

signals:
    void sigToolTipShowHide(bool);

public slots:
    void slotSpinBoxValueChanged(QString,quint32);
    void slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e);
    void slotOptionButtonClicked(OPTION_STATE_TYPE_e, int);
    void slotCreateCMDRequest(SET_COMMAND_e, quint8);
    void slotButtonClick(int);
    void slotPopupPageDeleted(quint8 indexInPage);
    void slotcameraListUpdateTimerTimeout();
    void slotIpAddressChangeDelete();
    void slotIpAddressChangeData(QString, QString, QString);
    void slotToolTipShowHide(bool);
    void slotValueListEmpty(quint8);

private:
    DropDown*                       cameraNumberDropDownBox;
    ControlButton*                  testCameraButton;
    ReadOnlyElement*                cameraTypeReadOnly;

    TextboxParam*                   cameraNameTextboxParam;
    TextBox*                        cameraNameTextbox;
    OptionSelectButton*             enableCameraCheckBox;

    OptionSelectButton*             logMotionDetectionCheckBox;
    PageOpenButton*                 osdSettingsPageOpenButton;

    TextboxParam*                   motionDelayTextboxParam;
    TextBox*                        motionDelayTextbox;
    TextboxParam*                   mobileNumberParam;
    TextBox*                        mobileNumberTextBox;

    TestCamera*                     testCamera;
    CameraOSDSettings*              cameraOsdSettings;

    OSD_SETTINGS_t                  cameraOsdParams;
    CAMERA_TYPE_e                   currentCameraType;

    quint8                          m_currentCameraIndex;
    QString                         currentCameraName;

    QMap<quint8, QString>           cameraList;
    QTimer*                         m_cameraListUpdateTimer;

    OptionSelectButton*             motionDetectionCheckBox;
    PageOpenButton*                 motionDetectionSetButton;
    PageOpenButton*                 m_motionDetectionCopyToCamButton;
    CAMERA_BIT_MASK_t               m_motionDetectionCopyToCameraField;
    CopyToCamera*                   m_copyToCamera;

    OptionSelectButton*             privacyMaskCheckBox;
    PageOpenButton*                 privacyMaskSetButton;

    PRIVACY_MASK_DATA_t             privacyMask[MAX_PRIVACYMASK_AREA];
    MOTION_DETECTION_CONFIG_t       motionDetectionConfig;

    bool                            isMotionAreaSetAllow;
    bool                            isPrivacyAreaSetAllow;
    bool                            isCameraEnable;
    bool                            isCameraConfigured;

    OptionSelectButton*             recordingStreamMainCheckBox;
    OptionSelectButton*             recordingStreamSubCheckBox;

    IpChangeProcessRequest*         m_ipChangeProcessRequest;
    ReadOnlyElement*                cameraMacAddressReadOnly;

    /* Ip Camera Setting fields */
    TextboxParam*                   cameraAddressTextboxParam;
    TextBox*                        cameraAddressTextbox;
    OptionSelectButton*             onvifSupportCheckBox;

    TextboxParam*                   httpPortTextboxParam;
    TextBox*                        httpPortTextBox;
    TextWithList*                   brandNameDropdown;
    TextboxParam*                   brandNameListParam;

    TextboxParam*                   rtspPortTextboxParam;
    TextBox*                        rtspPortTextBox;
    TextWithList*                   modelNameDropdown;
    TextboxParam*                   modelNameListParam;

    TextboxParam*                   onvifPortTextboxParam;
    TextBox*                        onvifPortTextBox;

    TextboxParam*                   usernameTextboxParam;
    TextBox*                        usernameTextBox;

    TextboxParam*                   urlTextboxParam;
    TextBox*                        urlTextBox;
    TextboxParam*                   passwordTextboxParam;
    PasswordTextbox*                passwordTextBox;

    QMap<quint8, QString>           brandNameList;
    QMap<quint8, QString>           modelNameList;

    ControlButton*                  ipAddressChangeButton;
    IpAddressChange*                ipAddressChange;

    QString                         changedIpAddress;
    DropDown*                       protocolSection;
    Image*                          autoAddCameraImage;
    ToolTip*                        autoAddToolTip;

    bool                            isIpChngEnable;
    QString                         m_camMacAddr;

    void createDefaultElements();
    void createPayload(REQ_MSG_ID_e requestType);
    void createCmdRequest(SET_COMMAND_e cmd, quint8 totalField);

    void fillCameraList();
    void enableChangeIpButtonState(bool isEnable);
    void enableIpCameraCntrls();
    void getModelList();
    void createOsdSetting(CAMERA_TYPE_e camType);
    void mouseMoveEvent(QMouseEvent *event);
};

#endif // CAMERASETTINGS_H
