#ifndef USBSTATUS_H
#define USBSTATUS_H

#include "HardwareTestControl.h"

class USBStatus : public HardwareTestControl
{
    Q_OBJECT
public:
    explicit USBStatus(quint16 xPos, quint16 yPos, quint8 totalRow, quint8 totalCol, CONDUNCT_TEST_e hwIndex, QWidget *parent = 0, quint8 noOfSubDev = 1);
    ~USBStatus() { };

    void hardwareTestStart();
    void hardwareTestStop() { };
    void saveHwTestStatus(CONDUNCT_TEST_e hwIndex);

private:
    quint8  noOfUSB;
};

#endif // USBSTATUS_H
