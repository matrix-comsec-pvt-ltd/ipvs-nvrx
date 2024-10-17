//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		Manual.h
@brief      This is the sub module which is used to set the date and time of NVR as given by the user
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <sys/ioctl.h>
#if defined(HI3536_NVRL) || defined(HI3536_NVRH)
#include "hi_rtc.h"
#endif

/* Application Includes */
#include "TimeZone.h"
#include "DateTime.h"
#include "Dst.h"
#include "Manual.h"
#include "DebugLog.h"
#include "EventLogger.h"
#include "Utils.h"
#include "InputOutput.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define DATE_TIME_MANUAL    "00"

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static BOOL setSystemTime(struct timeval * newTimeVal, CHAR *username);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function checks the given time instance falls within which DST Window, if DST is
 *          enabled. If new time instance falls within invalid period; then it is considered as
 *          Standard Time NVR is set to modified time period and local time returned by other api
 *          will be new time instance and dst period added to it. For example, If DST is to be applied
 *          at 2 am of 2nd April and forwarded to 3 am and user has given new time 2.15 am of 2nd April.
 *          It is invalid instance and new system time will be same 2.15 am but local time returned by
 *          other api will be 3.15am. If given time instance is within DST period than it will be
 *          considered as local time and standard time will be calculated and System time is set to
 *          this standard time. If given time instance is out of DST or DST is disabled than the local
 *          and standard time will be same and System time is set to this given time.
 * @param   newUserInstance
 * @param   timezone
 * @param   username
 * @return  SUCCESS / FAIL
 */
BOOL SetTimeManually(time_t newUserInstance, UINT8 timezone, CHAR *username)
{
	time_t			dstPeriod;
	time_t 			newSysInstance;
	struct timeval 	newTimeVal;
	DST_SCALE_e 	currScale;					//DST SCALE for input time

	//get the current value of DST scale for new time instance
	currScale = CheckTimeInDstScale(newUserInstance, &dstPeriod);

	//system time is equal to the new time instance in terms of UTC
	newSysInstance = newUserInstance - TimeZoneinSec[timezone - 1];

	switch(currScale)
	{
        case INSTANCE_OUTOF_DST:			//if new instance is out DST window
            if(DstPeriodInSec != 0)
            {
                DstPeriodInSec = 0;
            }
            break;

        case INSTANCE_IN_DST:				//if new instance is in DST window
            newSysInstance = newUserInstance - dstPeriod;
            break;

        case INVALID_INSTANCE: 				//If New Time instance is invalid
        case DST_DISABLED:
        default:
            break;
	}

	newTimeVal.tv_sec = newSysInstance;
	newTimeVal.tv_usec = 0;

	//set the above calculate time to system
    return setSystemTime(&newTimeVal, username);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function sets hwclock rtc time in to system
 * @return  SUCCESS / FAIL
 */
BOOL SetHwClockTime(void)
{
    #if defined(HI3536_NVRL) || defined(HI3536_NVRH)
    if (FALSE == IsExternalRtcAvailable()) // for board with internal RTC
	{
        INT32       fd;
        time_t      rawTime = {0};
        rtc_time_t  rtcTime = {0};
        struct tm   *timeInfo;

        time(&rawTime);
        timeInfo = localtime(&rawTime);
        if(timeInfo == NULL)
        {
            EPRINT(DATE_TIME, "fail to get localtime");
            return FAIL;
        }

        fd = open("/dev/hi_rtc", READ_WRITE_MODE);
        if (fd < STATUS_OK)
        {
            EPRINT(DATE_TIME, "fail to open hi rtc: [err=%s]", STR_ERR);
            return FAIL;
        }

        rtcTime.date = timeInfo->tm_mday;
        rtcTime.hour = timeInfo->tm_hour;
        rtcTime.year = timeInfo->tm_year + START_YEAR;
        rtcTime.month = timeInfo->tm_mon + 1;
        rtcTime.minute = timeInfo->tm_min;
        rtcTime.second = timeInfo->tm_sec;
        rtcTime.weekday = timeInfo->tm_wday;

        if (ioctl(fd, HI_RTC_SET_TIME, &rtcTime) < STATUS_OK)
        {
            EPRINT(DATE_TIME, "ioctl HI_RTC_SET_TIME failed: [err=%s]", STR_ERR);
            close(fd);
            return FAIL;
        }

        close(fd);
	}
	else
    #endif
	{
        if (FALSE == ExeSysCmd(TRUE, "hwclock --systohc --utc"))
        {
            EPRINT(DATE_TIME, "fail to set new time in rtc");
            return FAIL;
        }
	}

    DPRINT(DATE_TIME, "date-time is updated in rtc suceessfully");
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gets hwclock rtc time in to system
 * @return  SUCCESS / FAIL
 */
BOOL GetHwClockTime(void)
{
    #if defined(HI3536_NVRL) || defined(HI3536_NVRH)
    if (FALSE == IsExternalRtcAvailable()) // for board with internal RTC
    {
        INT32			fd;
        time_t			rawTime = {0};
        rtc_time_t		rtcTime = {0};
        struct tm		timeInfo = {0};
        struct timeval	time = {0};

        fd = open("/dev/hi_rtc", READ_WRITE_MODE);
        if (fd < STATUS_OK)
        {
            EPRINT(DATE_TIME, "fail to open hi rtc: [err=%s]", STR_ERR);
            return FAIL;
        }

        if (ioctl(fd, HI_RTC_RD_TIME, &rtcTime) < STATUS_OK)
        {
            EPRINT(DATE_TIME, "ioctl HI_RTC_RD_TIME failed: [err=%s]", STR_ERR);
            close(fd);
            return FAIL;
        }

        timeInfo.tm_min = rtcTime.minute;
        timeInfo.tm_sec = rtcTime.second;
        timeInfo.tm_mon = rtcTime.month - 1;
        timeInfo.tm_year = rtcTime.year - START_YEAR;
        timeInfo.tm_mday = rtcTime.date;
        timeInfo.tm_hour = rtcTime.hour;
        timeInfo.tm_wday = rtcTime.weekday;

        //set the newTimeVal as system time
        rawTime = mktime(&timeInfo);
        time.tv_sec = rawTime;
        if(settimeofday(&time, NULL) == NILL)
        {
            EPRINT(DATE_TIME, "fail to set system time: [err=%s]", STR_ERR);
            close(fd);
            return FAIL;
        }

        DPRINT(DATE_TIME, "time set successfully from rtc to system");
        close(fd);
	}
	else
    #endif
	{
        if (FALSE == ExeSysCmd(TRUE, "hwclock --hctosys --utc"))
        {
            EPRINT(DATE_TIME, "fail to set new time from rtc to system");
            return FAIL;
        }
	}

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function sets the time passed as input parameter to the System and RTC considering
 *          it is the time since Epoch.
 * @param   newTimeVal
 * @param   username
 * @return  SUCCESS / FAIL
 */
static BOOL setSystemTime(struct timeval * newTimeVal, CHAR *username)
{
	//set the newTimeVal as system time
	if(settimeofday(newTimeVal, 0) == NILL)
	{
        EPRINT(DATE_TIME, "settimeofday failed: [err=%s]", STR_ERR);
        return FAIL;
	}

    if(FAIL == SetHwClockTime())
    {
        return FAIL;
    }

    WriteEvent(LOG_SYSTEM_EVENT, LOG_RTC_UPDATE, DATE_TIME_MANUAL, username, EVENT_ALERT);
    DPRINT(SYS_LOG, "new time set in rtc successfully: [user=%s]", username);
    return SUCCESS;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
