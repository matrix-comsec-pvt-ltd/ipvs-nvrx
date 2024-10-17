#ifndef ETHER_H
#define ETHER_H

#include "HardwareTestControl.h"
#include "FileIO.h"

class Ether : public HardwareTestControl
{
    Q_OBJECT
public:
    explicit Ether(quint16 xPos, quint16 yPos, quint8 totalRow, quint8 totalCol, CONDUNCT_TEST_e hwIndex, QWidget *parent = 0, quint8 noOfSubDev = 1);
    ~Ether();

    void hardwareTestStart();
    void hardwareTestStop() { };
    void saveHwTestStatus(CONDUNCT_TEST_e index);

private:
    FileIO*     m_fileIO[2];
    quint8      totalLanPort;
};

#endif // ETHER_H
