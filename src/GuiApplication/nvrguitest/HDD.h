#ifndef HDD_H
#define HDD_H

#include <QObject>
#include "HardwareTestControl.h"

class HDDStatus : public HardwareTestControl
{
    Q_OBJECT
public:
    explicit HDDStatus(quint16 xPos, quint16 yPos, quint8 totalRow, quint8 totalCol, CONDUNCT_TEST_e hwIndex, QWidget *parent = 0, quint8 noOfSubDev = 1);
    ~HDDStatus() { };

    void hardwareTestStart();
    void hardwareTestStop() { };
    void saveHwTestStatus(CONDUNCT_TEST_e hwIndex);

private:
    quint8          noOfHDD;
};

#endif // HDD_H
