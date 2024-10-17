#ifndef WIZARDADDCAMERA_H
#define WIZARDADDCAMERA_H

#include "PayloadLib.h"
#include "KeyBoard.h"
#include "Controls/Heading.h"
#include "Controls/Closebuttton.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/TextWithList.h"
#include "Controls/ReadOnlyElement.h"
#include "DataStructure.h"
#include "Controls/TextBox.h"
#include "Controls/PasswordTextbox.h"
#include "Controls/CnfgButton.h"
#include "Controls/InfoPage.h"
#include "ApplController.h"
#include "Controls/ProcessBar.h"
#include "WizardCameraInformation.h"
#include "Controls/ControlButton.h"
#include "Controls/DropDown.h"
#include "EnumFile.h"

typedef enum
{
    WIZ_ADD_CAMERAS_CLOSE_BUTTON,
    WIZ_ADD_CAMERAS_CAMERA_INFO_BUTTON,
    WIZ_ADD_CAMERAS_ONVIF_CHECKBOX,
    WIZ_ADD_CAMERAS_IP_ADDRESS_DROPDOWN,
    WIZ_ADD_CAMERAS_BRAND_NAME_PICKLIST,
    WIZ_ADD_CAMERAS_MODEL_NAME_PICKLIST,
    WIZ_ADD_CAMERAS_CAMERA_NAME_TEXTBOX,
    WIZ_ADD_CAMERAS_USERNAME_TEXTBOX,
    WIZ_ADD_CAMERAS_PASSWORD_PASSWORDBOX,
    WIZ_ADD_CAMERAS_SAVE_BUTTON,
    WIZ_MAX_ADD_CAMERA_ELEMENTS
}WIZ_ADD_CAMERA_ELEMENTS_e;

class WizardAddCamera : public KeyBoard
{
    Q_OBJECT

public:
    WizardAddCamera(QString currDevName,
                    QMap<quint8, QString> ipAddrMapList,
                    QString httpPortStr,
                    QString onvifPortStr,
                    bool onvifSupport,
                    QString brandlistStr,
                    QString modellistStr,
                    QString camName,
                    QString usrName,
                    QString paswrd,
                    QWidget *parent,
                    quint8 idxInPage = 0,
                    MX_CAM_STATUS_e camStatus = MAX_MX_CAM_STATUS,
                    quint8 camIndex = 0,
                    DEV_TABLE_INFO_t *devTabInfo = NULL);

    ~WizardAddCamera();

    void processDeviceResponse(DevCommParam *param, QString deviceName);
    void createCommandRequest(SET_COMMAND_e cmdType, quint8 totalfeilds);

signals:
    void sigDeleteObject(quint8 cameraIndex,
                         bool saveCameraFlag,
                         QString ipAddressStr,
                         QString onvifPortStr,
                         QString httpPortStr,
                         bool onvifSupport,
                         QString brandlistStr,
                         QString modellistStr,
                         QString camName,
                         QString userName,
                         QString password,
                         quint8 selectedIndex,
                         quint8 currIndex);

public slots:
    void slotButtonClick(int);
    void slotValueChanged(QString, quint32);
    void slotOptionButtonClick(OPTION_STATE_TYPE_e,int);
    void slotInfoPageBtnclick(int);
    void slotObjectDelete(quint8);
    void slotValueListEmpty(quint8);

private:

    Rectangle*              backGround;
    CloseButtton*           closeButton;
    Heading*                heading;

    ReadOnlyElement*        cameraIndexReadOnly;
    DropDown*               ipAddrDropdown;
    ReadOnlyElement*        httpPortReadOnly;
    OptionSelectButton*     onvifSupportCheckBox;
    TextWithList*           brandNameDropdown;
    TextboxParam*           brandNameListParam;
    ReadOnlyElement*        onvifPortReadOnly;
    TextWithList*           modelNameDropdown;
    TextboxParam*           modelNameListParam;

    TextboxParam*           ipCameraNameTextboxParam;
    TextBox*                ipCameraNameTextBox;

    TextboxParam*           usernameTextboxParam;
    TextBox*                usernameTextBox;

    TextboxParam*           passwordTextboxParam;
    PasswordTextbox*        passwordTextBox;
    CnfgButton*             saveButton;

    PayloadLib*             payloadLib;
    ApplController*         applController;

    QString                 currentDeviceName;
    QMap<quint8, QString>   ipAddrMap;
    QString                 httpPortAddress;
    QString                 onvifPort;
    QString                 brandNameValue;
    QString                 modelNameValue;
    QString                 userName;
    QString                 password;
    QString                 cameraName;
    bool                    isOnvifSupport;
    bool                    isCameraAdded;

    QMap<quint8, QString>   brandNameList;
    QMap<quint8, QString>   modelNameList;

    InfoPage*               infoPage;
    ProcessBar*             processbar;
    quint8                  indexInPage;
    MX_CAM_STATUS_e         cameraStatus;

    quint8                  selIndex;
    quint8                  currentIndex;

    WizardCameraInformation *m_cameraInformation;
    ControlButton*          cameraListButton;
    DEV_TABLE_INFO_t*       m_devTabInfo;

    void createDefaultElements();
    void getBrandNameList();
    void saveValues();
    void getConfig();
};

#endif // WIZARDADDCAMERA_H
