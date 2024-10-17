#ifndef RTCTEST_H
#define RTCTEST_H

#include <QObject>
#include "HardwareTestControl.h"

class RTCTest : public HardwareTestControl
{
    Q_OBJECT
private:
    time_t  m_currentTime;
    time_t  m_AfterTime;

    void timeOut();

public:
    explicit RTCTest(quint16 xPos, quint16 yPos, quint8 totalRow, quint8 totalCol, CONDUNCT_TEST_e hwIndex, QWidget *parent = 0);
    ~RTCTest() { };

    void hardwareTestStart();
    void hardwareTestStop() { };
    void saveHwTestStatus(CONDUNCT_TEST_e hwIndex);

    bool getLocalTimeInBrokenTm(struct tm * localTimeInTmStruct);
    bool getLocalTimeInSec(time_t * currLocalTime);
    Q_INVOKABLE QString getLocalTime(struct tm  *tempInputTmStruct);
};
#endif // RTCTEST_H
