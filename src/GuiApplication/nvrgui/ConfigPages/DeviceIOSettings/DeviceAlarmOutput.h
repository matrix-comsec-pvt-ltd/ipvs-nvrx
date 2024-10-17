#ifndef DEVICEALARMOUTPUT_H
#define DEVICEALARMOUTPUT_H

#include "Controls/ConfigPageControl.h"
#include "Controls/DropDown.h"
#include "Controls/ReadOnlyElement.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/TextBox.h"
#include "DataStructure.h"

class DeviceAlarmOutput : public ConfigPageControl
{
    Q_OBJECT
public:
    explicit DeviceAlarmOutput(QString devName,QWidget *parent = 0,
                               DEV_TABLE_INFO_t *devTabInfo = NULL);
    ~DeviceAlarmOutput();

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
    void slotLoadInfopage(int,INFO_MSG_TYPE_e);
    void slotSpinBoxValueChange(QString,quint32);

private:

    DropDown*           alarmNumberDropDownBox;

    OptionSelectButton* enableCheckBox;
    TextboxParam*       nameTextBoxParam;
    TextBox*            nameTextBox;

    DropDown*           alarmModeDropDownBox;
    TextboxParam*       pulsePeriodTextBoxParam;
    TextBox*            pulsePeriodTextBox;
    quint8              m_alarmIndex;

};

#endif // DEVICEALARAMOUTPUT_H
