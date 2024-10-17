#include <unistd.h>

#include "SensorInAlarmOut.h"

#if defined(RK3568_NVRL) || defined(RK3588_NVRH)
#define GET_SENSOR1_STATUS  "/sys/class/gpios/MxGpioDrv/almin1"
#define GET_SENSOR2_STATUS  "/sys/class/gpios/MxGpioDrv/almin2"
#define ALARM_OUT           "/sys/class/gpios/MxGpioDrv/almout"
#else
#define GET_SENSOR1_STATUS  "/sys/class/gpio/MxGpioDrv/almin1"
#define GET_SENSOR2_STATUS  "/sys/class/gpio/MxGpioDrv/almin2"
#define ALARM_OUT           "/sys/class/gpio/MxGpioDrv/almout"
#endif

#define STATUS_ACTIVE       "1"
#define STATUS_INACTIVE     "0"

static const QString sensorAlarm[] =
{
    GET_SENSOR1_STATUS,
    GET_SENSOR2_STATUS,
    ALARM_OUT
};

SensorInAlarmOut::SensorInAlarmOut(quint16 xPos, quint16 yPos, quint8 totalRow, quint8 totalCol, CONDUNCT_TEST_e hwIndex, QWidget *parent, quint8 noOfSubDev)
    : HardwareTestControl(hwIndex, parent, noOfSubDev)
{
     QStringList strlst;

     for(quint8 index = 0; index < noOfSubDev; index++)
     {
         m_fileIO[index] = new FileIO(this, sensorAlarm[index]);
     }

     strlst << "Sensor 1" << "Sensor 2" << "Alarm";
     createDefaultComponent(xPos, yPos, totalRow, totalCol, false, false, SUFFIX_FONT_SIZE, strlst, QString("Sensor IN and Alarm OUT"));
}

SensorInAlarmOut::~SensorInAlarmOut()
{
    for(quint8 index = 0; index < 3; index++)
    {
        DELETE_OBJ(m_fileIO[index]);
    }
}

void SensorInAlarmOut::hardwareTestStart()
{
    for(quint8 index = 0; index < MAX_SENSOR_IN; index++)
    {
        m_sensorStatus[index] = m_fileIO[index]->read();
    }

    m_fileIO[2]->write(STATUS_ACTIVE);
    sleep(1);
    for(quint8 index = 0; index < MAX_SENSOR_IN; index++)
    {
        QString status;
        status = m_fileIO[index]->read();
        if (m_sensorStatus[index] == status)
        {
            testCondunctResult[index] = HW_TEST_FAIL;
        }
        else
        {
           m_sensorStatus[index] = status;
        }
    }
    this->timerOut();
}

void SensorInAlarmOut::timerOut()
{
    m_fileIO[2]->write(STATUS_INACTIVE);
    sleep(1);
    for(quint8 index = 0; index < MAX_SENSOR_IN; index++)
    {
        QString status;
        status = m_fileIO[index]->read();
        if(m_sensorStatus[index] == status)
        {
            testCondunctResult[index] = HW_TEST_FAIL;
        }
        else
        {
           m_sensorStatus[index] = status;
           testCondunctResult[index] = HW_TEST_PASS;
        }
    }
    if((testCondunctResult[0] == HW_TEST_PASS) ||
            (testCondunctResult[1] == HW_TEST_PASS))
    {
        testCondunctResult[2] = HW_TEST_PASS;
    }
    else
    {
        testCondunctResult[2] = HW_TEST_FAIL;
    }
}

void SensorInAlarmOut::saveHwTestStatus(CONDUNCT_TEST_e hwIndex)
{
     HardwareTestControl::testResult[hwIndex] = getStatusOfHwTest();
}

