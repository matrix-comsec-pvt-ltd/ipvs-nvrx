#ifndef CAMERAALARMOUTPUT_H
#define CAMERAALARMOUTPUT_H

#include "DataStructure.h"
#include "Controls/SpinBox.h"
#include "Controls/TextBox.h"
#include "Controls/ConfigPageControl.h"
#include "Controls/DropDown.h"

class CameraAlarmOutput : public ConfigPageControl
{
    Q_OBJECT
public:
    explicit CameraAlarmOutput(QString devName,
                               QWidget *parent = 0,
                               DEV_TABLE_INFO_t* devTab = NULL);

    ~CameraAlarmOutput();

    // virtual Function of configPage Control
    void getConfig ();
    void saveConfig ();
    void defaultConfig ();
    void processDeviceResponse (DevCommParam *param, QString deviceName);
    bool isUserChangeConfig();
    void handleInfoPageMessage(int index);

signals:
    
public slots:
    void slotLoadInfopage(int,INFO_MSG_TYPE_e);
    void slotSpinBoxValueChange(QString,quint32);

private:

    // private Variable
    DropDown*       cameraNameDropDownBox;
    DropDown*        cameraAlarmDropDownBox;
    DropDown*        activeModeSpinbox;

    TextboxParam*   pulsePeriodTextBoxParam;
    TextBox*        pulsePeriodTextBox;

    QMap<quint8, QString>     cameraNameList;
    quint8          currentCameraIndex;
    quint8          currentAlarmIndex;


    // private Functions
    void getCameraList();
    void createDefaultComponents();
    void createPayload(REQ_MSG_ID_e msgType);
    
};

#endif // CAMERAALARMOUTPUT_H
