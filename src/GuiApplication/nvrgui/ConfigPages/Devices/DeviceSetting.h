#ifndef DEVICESETTING_H
#define DEVICESETTING_H

#include <QWidget>
#include "Controls/ConfigPageControl.h"
#include "Controls/TextBox.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/CnfgButton.h"
#include "DataStructure.h"
#include "NavigationControl.h"
#include "Controls/DropDown.h"
#include "ApplController.h"
#include "Controls/ControlButton.h"
#include "Controls/ElementHeading.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/IpTextBox.h"
#include "Controls/MacTextBox.h"
#include "Controls/PasswordTextbox.h"

#define MAX_CONTROL_BUTTONS 3

typedef enum
{
    DEV_SET_DEVICE_NAME,
    DEV_SET_REGISTER_MODE,
    DEV_SET_REGISTER_MODE_ADDRESS,
    DEV_SET_PORT,
    DEV_SET_USERNAME,
    DEV_SET_PASSWORD,
    DEV_SET_ENABLE,
    DEV_SET_AUTOLOGIN,
    DEV_SET_LIVE_STREAM,
    DEV_SET_PREFER_NATIVE_DEV_CREDENTIAL,
    DEV_SET_FORWARDED_PORT,
    MAX_DEVICE_SETTINGS_FIELDS
}DEVICE_SETTINGS_FIELDS_e;

class DeviceSetting : public ConfigPageControl
{
    Q_OBJECT

public:
    explicit DeviceSetting(QString devName,
                  QWidget *parent = 0);
    ~DeviceSetting();

    void getConfig ();
    void saveConfig();
    void defaultConfig ();

    void handleInfoPageMessage (int index);

    void processDeviceResponse (DevCommParam *param, QString deviceName);

public slots:
    void slotDropDownBoxValueChanged(QString string, quint32 indexInPage);
    void slotTextBoxLoadInfopage(int index, INFO_MSG_TYPE_e msgType);
    void slotControlButtonClicked(int index);
    void slotRadioButtonClicked(OPTION_STATE_TYPE_e state, int indexInPage);

private:

    quint8              m_deviceCount;
    quint8              m_controlBtnIndex;
    quint8              m_currentDeviceIndex;
    bool                enabled;

    QString             m_LocalDeviceName;
    QString             m_DeviceName;

    QStringList         m_deviceList;

    ApplController*     m_applController;
    DropDown*           m_deviceNameDropDownBox;
    ControlButton*      m_controlButton[MAX_CONTROL_BUTTONS];
    ElementHeading*     m_addEditHeading;
    TextboxParam*       m_deviceNameParam;
    TextboxParam*       m_deviceModel;
    TextBox*            m_deviceName;
    TextBox*            m_deviceModelName;
    OptionSelectButton* m_deviceEnableBox;
    DropDown*           m_registerMode;
    IpTextBox*          m_ipAddressTextbox;
    TextboxParam*       m_matrixDDNSParam;
    TextBox*            m_matrixDDNSTextBox;
    MacTextBox*         m_macAddressBox;
    TextboxParam*       m_DDNSHostNameParam;
    TextBox*            m_DDNSHostName;
    TextboxParam*       m_forwardedTcpPortTextBoxParam;
    TextBox*            m_forwardedTcpPortTextBox;

    // Port Number, username, password
    TextboxParam*       m_textboxParam[MAX_CONTROL_BUTTONS];
    TextBox*            m_textbox[MAX_CONTROL_BUTTONS -1];
    PasswordTextbox*    m_passwordBox;
    OptionSelectButton* m_preferDevCredential;   
    OptionSelectButton* m_autoLogin;
    OptionSelectButton *m_mainRadioButton, *m_subRadioButton, *m_optimizedRadioButton;

    bool isDeviceListEnable;
    bool isInitDone;

    bool getconfigCall;

    void createPayload(REQ_MSG_ID_e requestType);
    void createDefaultElements();
    void setUserDataForConfigForDefault ();
    void enableDisableAllControl(bool state);
    void defaultFields();
    void getDeviceConfig();
    void getDeviceList();

    bool dataVerification();
    void setUserDataForConfig ();
};


#endif // DEVICESETTING_H
