#ifndef DIRECTPASSFAILTEST_H
#define DIRECTPASSFAILTEST_H

#include "HardwareTestControl.h"

class DirectPassFailTest : public HardwareTestControl
{
    Q_OBJECT
public:
    explicit DirectPassFailTest(quint16 xPos, quint16 yPos, quint8 totalRow, quint8 totalCol, CONDUNCT_TEST_e hwIndex, QWidget *parent = 0);
    ~DirectPassFailTest() { };

    void hardwareTestStart() { };
    void hardwareTestStop() { };
    void saveHwTestStatus(CONDUNCT_TEST_e hwIndex);
};
#endif // DIRECTPASSFAILTEST_H
