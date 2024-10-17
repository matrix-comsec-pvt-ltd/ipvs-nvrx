#include <unistd.h>
#include "RTCTest.h"

RTCTest::RTCTest(quint16 xPos, quint16 yPos, quint8 totalRow, quint8 totalCol, CONDUNCT_TEST_e hwIndex, QWidget *parent)
    : HardwareTestControl(hwIndex, parent)
{
    QStringList strlst;

    memset(&m_currentTime, 0, sizeof(m_currentTime));
    memset(&m_AfterTime, 0, sizeof(m_AfterTime));

    strlst << "RTC" ;
    createDefaultComponent(xPos, yPos, totalRow, totalCol, false, false, TEST_HEADING_FONT_SIZE, strlst);
}

void RTCTest::hardwareTestStart()
{
    struct timespec req = {5, 0};
    getLocalTimeInSec(&m_currentTime);
    nanosleep(&req, &req);
    this->timeOut();
}

void RTCTest::saveHwTestStatus(CONDUNCT_TEST_e hwIndex)
{
    HardwareTestControl::testResult[hwIndex] = getStatusOfHwTest();
}

bool RTCTest::getLocalTimeInBrokenTm(struct tm * localTimeInTmStruct)
{
    time_t localTimeInSec; 			//local time in time_t
    struct tm tempStruct; 			//temporary structure
    bool retValue = false;

    if (getLocalTimeInSec(&localTimeInSec) == true)
    {
        //convert above time in broken down time structure
        if (gmtime_r(&localTimeInSec, &tempStruct) != NULL)
        {
            //copy temporary structure to output structure
            memcpy(localTimeInTmStruct, &tempStruct, sizeof(struct tm));

            //add century to the years
            localTimeInTmStruct->tm_year = (tempStruct.tm_year + 1900);

            retValue = true;
        }
    }
    return retValue;
}

bool RTCTest::getLocalTimeInSec(time_t * currLocalTime)
{
    time_t  tempTime;
    bool    retValue = false;
    int     timeZoneinSec = 19800;	/* "GMT-5:30",		0. Culcutta,Chennai,Mumbai,New Delhi */

    //find the System Time
    if (time(&tempTime) != -1)
    {
        //add TimeZone to UTC
        *currLocalTime = tempTime + timeZoneinSec;
        retValue = true;
    }
    return retValue;
}

QString RTCTest::getLocalTime(struct tm  *tempInputTmStruct)
{
    getLocalTimeInBrokenTm (tempInputTmStruct);

    QString time = QString("%1").arg (tempInputTmStruct->tm_hour) + QString(" : ") +
            QString("%1").arg (tempInputTmStruct->tm_min) + QString(" : ") +
            QString("%1").arg (tempInputTmStruct->tm_sec) ;

    return time;
}

void RTCTest::timeOut()
{
    getLocalTimeInSec(&m_AfterTime);

    if (m_AfterTime > m_currentTime)
    {
        if ((m_AfterTime - m_currentTime) == 5)
        {
            testCondunctResult[0] = HW_TEST_PASS;
        }
        else
        {
            DPRINT(GUI_SYS, "[RTC] Fail: Time-Ref1-%ld Ref2-%ld", m_currentTime, m_AfterTime);
            testCondunctResult[0] = HW_TEST_FAIL;
        }
    }
    else
    {
        testCondunctResult[0] = HW_TEST_FAIL;
    }
}
