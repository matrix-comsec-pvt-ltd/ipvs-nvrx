#include "OsdUpdate.h"
#include <sys/prctl.h>


#define REF_COUNT_FOR_OSD_UPDATE    3600
#define SLEEP_CAL_TIME              999800

OsdUpdate::OsdUpdate(quint8 cameraCount) : m_cameraCount(cameraCount), m_timerCountForOsdUpdate(0)
{
    m_isAllOSDUpdate = true;
    m_isFetchDateTimeForOsdUpdate = false;
    m_isTimeInitDone = false;
    m_commandRequest = NULL;

    m_applController = ApplController::getInstance ();

    updateServerInfo();

    m_osdUpdateDateTime = QDateTime::currentDateTime();

    updateOSDParam();
}

OsdUpdate::~OsdUpdate ()
{

}

void OsdUpdate::updateServerInfo()
{
    m_OsdParamUpdateLock.lock ();
    m_applController->GetServerSessionInfo (LOCAL_DEVICE_NAME, m_sesionInfo);
    m_OsdParamUpdateLock.unlock ();
}

void OsdUpdate::updateTimeCountForOsdUpdate(quint8 count)
{
    m_OsdParamUpdateLock.lock ();
    m_timerCountForOsdUpdate = count;
    m_OsdParamUpdateLock.unlock ();
}

void OsdUpdate::updateAllOSD(bool flag)
{
    m_OsdParamUpdateLock.lock ();
    m_isAllOSDUpdate = flag;
    m_OsdParamUpdateLock.unlock ();
}

void OsdUpdate::updateOSDParam()
{
    bool isFetchDateTimeForOsdUpdate = false;
    bool isTimeInitDone = false;
    quint32 timerForOSDCount = 0;
    QDateTime dateTime;

    m_OsdParamUpdateLock.lock ();

    isTimeInitDone = m_isTimeInitDone;
    isFetchDateTimeForOsdUpdate = m_isFetchDateTimeForOsdUpdate;

    if(isTimeInitDone == true)
    {
        timerForOSDCount = m_timerCountForOsdUpdate;

        if(m_isFetchDateTimeForOsdUpdate == false)
        {
            if(m_timerCountForOsdUpdate >= REF_COUNT_FOR_OSD_UPDATE)
            {
                m_timerCountForOsdUpdate = 0;
                timerForOSDCount = 0;
                m_isFetchDateTimeForOsdUpdate = true;
            }
            else
            {
                m_timerCountForOsdUpdate++;
            }
        }

        m_osdUpdateDateTime = m_osdUpdateDateTime.addSecs(1);

        dateTime = m_osdUpdateDateTime;
        m_isAllOSDUpdate = false;
    }
    else
    {
        m_isFetchDateTimeForOsdUpdate = true;
    }

    m_OsdParamUpdateLock.unlock ();

    if( (timerForOSDCount == 0) && (isFetchDateTimeForOsdUpdate == false) )
    {
        getDateTimeForOsdUpdate();
    }
}

void OsdUpdate::getDateTimeForOsdUpdate()
{
    bool                isTimeInitDone = false;
    DEVICE_REPLY_TYPE_e devResponse = CMD_SUCCESS;
    quint32             seconds = CMD_NORMAL_TIMEOUT;
    QString             payload = "";
    REQ_INFO_t          requestInfo;
    SERVER_INFO_t       serverInfo;
    QDateTime           dateTime = m_osdUpdateDateTime;

    m_OsdParamUpdateLock.lock ();
    requestInfo.sessionId = m_sesionInfo.sessionInfo.sessionId;
    requestInfo.timeout = m_sesionInfo.sessionInfo.timeout;
    requestInfo.requestId = MSG_SET_CMD;
    requestInfo.payload = payload;
    requestInfo.bytePayload = NULL;
    requestInfo.windowId = MAX_WIN_ID;
    serverInfo.ipAddress = m_sesionInfo.serverInfo.ipAddress;
    serverInfo.tcpPort = m_sesionInfo.serverInfo.tcpPort;
    m_OsdParamUpdateLock.unlock ();

    if(m_commandRequest == NULL)
    {
        /* PARASOFT: Memory Deallocated below in the same function */
        m_commandRequest = new CommandRequest (serverInfo,
                                               requestInfo,
                                               GET_DATE_TIME,
                                               255);

        m_commandRequest->getBlockingRes(payload, devResponse);
        delete m_commandRequest;
        m_commandRequest = NULL;

        if(devResponse == CMD_SUCCESS)
        {
            QStringList responseList = payload.split(FSP);
            QString     dateTimeString = responseList.at (1);

            qint32 currentDate = dateTimeString.mid(0, 2).toInt();
            qint32 currentMonth = dateTimeString.mid(2, 2).toInt();
            qint32 currentYear = dateTimeString.mid(4, 4).toInt();
            qint32 currentHour = dateTimeString.mid(8, 2).toInt() ;
            qint32 currentMinute = dateTimeString.mid(10, 2).toInt();
            qint32 currentSecond = dateTimeString.mid(12, 2).toInt();

            dateTime = QDateTime(QDate(currentYear, currentMonth, currentDate),
                                 QTime(currentHour, currentMinute, currentSecond),
                                 Qt::UTC);

            seconds = ((60 * currentMinute) + currentSecond);
            isTimeInitDone = true;
        }

        m_OsdParamUpdateLock.lock ();

        m_timerCountForOsdUpdate = (seconds + 1);

        if(isTimeInitDone == true)
        {
            m_isTimeInitDone = isTimeInitDone;
            m_osdUpdateDateTime = dateTime;
        }

        m_isFetchDateTimeForOsdUpdate = false;

        m_OsdParamUpdateLock.unlock ();
    }
}

void OsdUpdate::run()
{
    QDateTime   dateTime;
    quint64     timeBeforeUpdateInMSec = 0;
    quint64     timeAfterUpdateInMSec = 0;
    quint64     diffTimeInMSec = 0;
    quint64     sleepMSec = 0;

    prctl(PR_SET_NAME, "OSD_UPDATE", 0, 0, 0);

    while(1)
    {
        // diff for second to sleep
        timeBeforeUpdateInMSec = dateTime.currentMSecsSinceEpoch ();

        updateOSDParam ();

        timeAfterUpdateInMSec = dateTime.currentMSecsSinceEpoch ();

        diffTimeInMSec = ((timeAfterUpdateInMSec - timeBeforeUpdateInMSec) * 1000);

        if(diffTimeInMSec >= SLEEP_CAL_TIME)
        {
            sleepMSec = 1000;
        }
        else
        {
            sleepMSec = (SLEEP_CAL_TIME - diffTimeInMSec);
        }
        usleep(sleepMSec);
    }
}
