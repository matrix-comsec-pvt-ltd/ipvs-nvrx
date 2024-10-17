#ifndef USBMANAGMENT_H
#define USBMANAGMENT_H

#include <QWidget>

#include "Controls/ConfigPageControl.h"
#include "Controls/ReadOnlyElement.h"
#include "Controls/ElementHeading.h"
#include "Controls/SpinBox.h"
#include "Controls/ControlButton.h"

#define MAX_READONLY_ELEMENT 6
#define MAX_USB_CNTRL        3

class USBManagment : public ConfigPageControl
{
    Q_OBJECT
public:
    explicit USBManagment(QString devName,QWidget *parent = 0);
    ~USBManagment();

    void createDefaultComponents ();
    void handleInfoPageMessage(int index);

    void sendCommand(SET_COMMAND_e cmdType, int totalfeilds = 0);
    void getUsbState();
    void formatUsb(int index);
    void unplugUsb(int index);
    void stopBackup(int index);

    void processDeviceResponse (DevCommParam *param, QString deviceName);
signals:
    
public slots:
    void slotStatusRepTimerTimeout();
    void slotButtonClick(int);

private:
    QTimer *statusRepTimer;
    quint8 currentButtonClick;
    USB_TYPE_e currentClickUsb;

    ElementHeading*  manualBackUpHeading;
    ElementHeading*  schdBackUpHeading;

    ReadOnlyElement* manualBackUpElements[MAX_READONLY_ELEMENT];
    ReadOnlyElement* schdBackUpElements[MAX_READONLY_ELEMENT];

    ControlButton*  manualBackUpControls[MAX_USB_CNTRL];
    ControlButton*  schdBackUpControls[MAX_USB_CNTRL];
    
};

#endif // USBMANAGMENT_H
