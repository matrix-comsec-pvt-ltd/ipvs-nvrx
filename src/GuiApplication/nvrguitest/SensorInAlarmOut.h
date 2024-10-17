#ifndef SENSORINALARMOUT_H
#define SENSORINALARMOUT_H

#include <QObject>
#include "HardwareTestControl.h"
#include "FileIO.h"

#define MAX_SENSOR_IN           2

class SensorInAlarmOut : public HardwareTestControl
{
    Q_OBJECT
public:
    explicit SensorInAlarmOut(quint16 xPos, quint16 yPos, quint8 totalRow, quint8 totalCol, CONDUNCT_TEST_e hwIndex, QWidget *parent = 0, quint8 noOfSubDev = 1);
    ~SensorInAlarmOut();

    void hardwareTestStart();
    void hardwareTestStop() { };
    void saveHwTestStatus(CONDUNCT_TEST_e index);
    void timerOut();

private:
    FileIO*         m_fileIO[3];
    QString         m_sensorStatus[MAX_SENSOR_IN];
};

#endif // SENSORINALARMOUT_H
