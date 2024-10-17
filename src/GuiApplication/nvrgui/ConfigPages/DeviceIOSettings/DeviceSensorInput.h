#ifndef DEVICESENSORINPUT_H
#define DEVICESENSORINPUT_H

#include "Controls/ConfigPageControl.h"
#include "Controls/DropDown.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/TextBox.h"
#include "Controls/ConfigPageControl.h"
#include "DataStructure.h"

class DeviceSensorInput : public ConfigPageControl
{
    Q_OBJECT
public:
    explicit DeviceSensorInput(QString devName,QWidget *parent = 0,
                               DEV_TABLE_INFO_t *devTabInfo = NULL);
    ~DeviceSensorInput();
    
    void createDefaultComponents ();
    void enableControls(bool);

    void createPayload(REQ_MSG_ID_e msgType);
    void getConfig ();
    void saveConfig ();
    void defaultConfig ();
    bool isUserChangeConfig();
    void processDeviceResponse (DevCommParam *param, QString deviceName);
    void handleInfoPageMessage(int index);

signals:
    
public slots:
    void slotCheckBoxClicked(OPTION_STATE_TYPE_e,int);
    void slotSpinBoxValueChanged (QString, quint32);
    
private:

    DropDown*           sensorInputDropDownBox;

    OptionSelectButton* enableCheckBox;
    TextboxParam*       nameTextBoxParam;
    TextBox*            nameTextBox;

    DropDown*           sensorModeDropDownBox;
    DropDown*           delayDropDownBox;

    QMap<quint8, QString>         sensorList;
    QMap<quint8, QString>         delayList;

    quint8 m_sensorIndex;


};

#endif // DEVICESENSORINPUT_H
