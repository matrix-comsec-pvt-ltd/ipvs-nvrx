#ifndef NETWORKDRIVE_H
#define NETWORKDRIVE_H

#include "Controls/ConfigPageControl.h"
#include "Controls/DropDown.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/TextBox.h"
#include "DataStructure.h"
#include "Controls/IpTextBox.h"
#include "Controls/PasswordTextbox.h"
#include "Controls/ControlButton.h"

class NetworkDrive : public ConfigPageControl
{
    Q_OBJECT
public:
    explicit NetworkDrive (QString devName,QWidget *parent = 0);
    ~NetworkDrive();
    
    void createDefaultComponents ();
    void enableControls(bool);

    void createPayload(REQ_MSG_ID_e msgType);
    void getConfig ();
    void saveConfig ();
    void defaultConfig ();

    void processDeviceResponse (DevCommParam *param, QString deviceName);
    void handleInfoPageMessage(int index);
    bool isUserChangeConfig ();

signals:
    
public slots:
    void slotCheckBoxClicked(OPTION_STATE_TYPE_e,int);
    void slotSpinBoxValueChanged(QString,quint32);
    void slotTestBtnClick(int);

private:

    DropDown*           networkDriveDropDownBox;
    OptionSelectButton* enableDrive;
    TextboxParam*       nameTextBoxParam;
    TextBox*            nameTextBox;
    IpTextBox*          ipTextBox;
    TextboxParam*       usernameTextBoxParam;
    TextBox*            usernameTextBox;
    TextboxParam*       passwordTextBoxParam;
    PasswordTextbox*    passwordTextBox;
    DropDown*           netwrkFileSysDriveDropDownBox;
    TextboxParam*       defaultFolderTextBoxParam;
    TextBox*            defaultFolderTextBox;
    ControlButton*      testConnButton;

    quint8 cnfgIndex;

};

#endif // NETWORKDRIVE_H
