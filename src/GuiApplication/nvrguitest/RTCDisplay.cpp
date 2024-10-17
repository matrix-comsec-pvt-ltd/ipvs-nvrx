#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#if defined(HI3536_NVRH) || defined(HI3536_NVRL)
#include "hi_rtc.h"
#endif

#include "RTCDisplay.h"
#include "BoardTypeWiseInfo.h"
#include "HardwareTestControl.h"
#include "CommonApi.h"

#define	 SNTP_CMD "sntp -4 -t 1 -S -K /dev/null "

RTCDisplay::RTCDisplay(QWidget *parent)
{
    Q_UNUSED(parent);
}

#if defined(HI3536_NVRL) || defined(HI3536_NVRH)
bool RTCDisplay::SetHwClockTimeHI3536()
{
    INT32 		   fd = INVALID_FILE_FD, retval = -1;
    time_t		   rawTime;
    rtc_time_t	   rtcTime;
    struct tm	   *timeInfo;
    BOOL           ret = SUCCESS;

    memset(&rtcTime, 0, sizeof(rtcTime));
    memset(&rawTime, 0, sizeof(rawTime));

    do
    {
        fd = open("/dev/hi_rtc", O_RDWR | O_CLOEXEC);
        if (fd < STATUS_OK)
        {
            EPRINT(GUI_SYS, "open failed RTC Device");
            ret = FAIL;
            break;
        }

        time( &rawTime );
        if((timeInfo = localtime( &rawTime )) == NULL)
        {
            EPRINT(GUI_SYS, "Failed to get localtime");
            ret = FAIL;
            break;
        }

        rtcTime.date	= timeInfo->tm_mday;
        rtcTime.hour	= timeInfo->tm_hour;
        rtcTime.year	= timeInfo->tm_year + 1900;
        rtcTime.month	= timeInfo->tm_mon + 1;
        rtcTime.minute	= timeInfo->tm_min;
        rtcTime.second	= timeInfo->tm_sec;
        rtcTime.weekday	= timeInfo->tm_wday;

        retval = ioctl(fd, HI_RTC_SET_TIME, &rtcTime);
        if (retval < STATUS_OK)
        {
            EPRINT(GUI_SYS, "ioctl: HI_RTC_SET_TIME failed");
            ret = FAIL;
            break;
        }

    }while(0);

    if(INVALID_FILE_FD != fd)
    {
        close(fd);
    }
    return ret;
}
#endif

bool RTCDisplay::setSystemTime()
{
    if(false == BoardTypeWiseInfo::isExtRTC)           //for board with internal RTC
    {
#if defined(HI3536_NVRL) || defined(HI3536_NVRH)
        if(false == SetHwClockTimeHI3536())
        {
            EPRINT(GUI_SYS, "Set new time in RTC FAIL");
            return FAIL;
        }
#endif
    }
    else
    {
        if (false == Utils_ExeCmd((char*)"hwclock --systohc --utc"))
        {
            EPRINT(GUI_SYS, "Set new time in RTC fail");
            return FAIL;
        }
    }
    DPRINT(GUI_SYS, "Date and Time is updated in RTC suceessfully");
    return SUCCESS;
}

quint8 RTCDisplay::getSntpTime()
{
    quint8 retryCnt = 0;

    QString command = SNTP_CMD + HardwareTestControl::ntpServerIpAddr;
    while(retryCnt < 5)
    {
        if (true == Utils_ExeCmd(command.toUtf8().data()))
        {
            break;
        }
        retryCnt++;
    }

    if (retryCnt >= 5)
    {
        EPRINT(GUI_SYS, "fail to get time from NTP server");
        return (1);
    }

	DPRINT(GUI_SYS, "time get from NTP server successfully: [retryCnt=%d]", retryCnt);
    if (false == setSystemTime())
    {
        EPRINT(GUI_SYS, "fail to Update RTC");
        return (2);
    }
    return (0);
}

bool RTCDisplay::getLocalTimeInBrokenTm(struct tm * localTimeInTmStruct)
{
    time_t localTimeInSec; 			//local time in time_t
    struct tm tempStruct; 			//temporary structure

    if (getLocalTimeInSec(&localTimeInSec) == true)
    {
        //convert above time in broken down time structure
        if (gmtime_r(&localTimeInSec, &tempStruct) != NULL)
        {
            //copy temporary structure to output structure
            memcpy(localTimeInTmStruct, &tempStruct, sizeof(struct tm));

            //add century to the years
            localTimeInTmStruct->tm_year = (tempStruct.tm_year + 1900);
            return SUCCESS;
        }
    }
    return FAIL;
}

bool RTCDisplay::getLocalTimeInSec(time_t * currLocalTime)
{
    time_t  tempTime;
    int     timeZoneinSec = 19800;	/* "GMT-5:30",		0. Culcutta,Chennai,Mumbai,New Delhi */

    //find the System Time
    if (time(&tempTime) != -1)
    {
        //add TimeZone to UTC
        *currLocalTime = tempTime + timeZoneinSec;
        return SUCCESS;
    }
    return FAIL;
}

QString RTCDisplay::getLocalTime(struct tm  *tempInputTmStruct)
{
    getLocalTimeInBrokenTm (tempInputTmStruct);

    QString time = QString("%1").arg (tempInputTmStruct->tm_hour) + QString(" : ") +
            QString("%1").arg (tempInputTmStruct->tm_min) + QString(" : ") +
            QString("%1").arg (tempInputTmStruct->tm_sec) ;

    return (time);
}

bool RTCDisplay::ConvertLocalTimeInSec(struct tm *inputTimeStruct, time_t *outputTimeInSec)
{
    struct tm tempInputTmStruct;		//temporary structure

    //copy input parameter to temporary structure
    memcpy(&tempInputTmStruct, inputTimeStruct, sizeof(struct tm));

    //the number of years since 1900
    tempInputTmStruct.tm_year = (tempInputTmStruct.tm_year - 1900);

    //covert the broken down time into total number of secs
    if((*outputTimeInSec = mktime(&tempInputTmStruct)) != -1)
    {
        return SUCCESS;
    }
    return FAIL;
}
