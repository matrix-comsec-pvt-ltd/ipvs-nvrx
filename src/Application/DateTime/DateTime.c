//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		DateTime.c
@brief      This module maintains NVR's date and time and provides APIs for other modules to get local
            date-time in different formats. It also provides API to set time and date as specified by
            user. It reads RTC during initialization which is kept in UTC and sets it as kernel's system
            time. When user request for a local time, this module adds configured timezone offset and
            DST period if current local time is DST to UTC time.
            This module is implemented by subdividing it to three different sub-modules and their
            implementation is as follows:-
            a) DST
            This sub-module provides mechanism to adjust local time for DST when DST is enabled. It
            checks whether the current time instance falls between DST Period (the forward time instance
            and reverse time instance) and maintains a DST flag. When this flag is set, configured DST
            advance time is added to standard time to derive current local time.
            Local time = UTC + time zone + DST (if applicable)
            b) SNTP
            This sub-module uses SNTP (Simple Network Time Protocol) client to synchronize the system
            clock with respect to a reference clock (NTP server).  It uses open source utility SNTP
            version 4 and part of ntp-4.2.6p3 package. The utility will be run in its own thread using
            system command. On successful synchronization of NVR system time, the time is synchronized
            again in the next update interval, configured in the system. But, on failure, the function
            is called again after predefined interval (one minute).
            c) Manual
            This sub-module provides the feature to change the date and time which is specified by user
            in the run-time.
*/

//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "TimeZone.h"
#include "DateTime.h"
#include "Dst.h"
#include "SntpClient.h"
#include "Manual.h"
#include "SysTimer.h"
#include "DebugLog.h"
#include "Utils.h"
#include "CameraInterface.h"
#include "InputOutput.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define MAX_NO_HOURS                4
#define TWENTYFOUR_HOUR             0
#define SIX_HOUR                    6
#define TWELVE_HOUR                 12
#define EIGHTEEN_HOUR               18
#define DATE_TIME_AUTO_TIME_ZONE    "02"
#define UPDATE_TIME_THREAD_STACK_SZ (500*KILO_BYTE)

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static UINT8				configTimezone = DFLT_TIMEZONE;
static BOOL                 syncTimeZone = ENABLE;
static BOOL					updateTimeF, updateDstPara, updateTzF;
static pthread_mutex_t		updateTimeMutex;
static pthread_cond_t 		updateTimeCond;
static const UINT8 			manualUpdateHrTab[MAX_NO_HOURS] = {SIX_HOUR, TWELVE_HOUR, EIGHTEEN_HOUR, TWENTYFOUR_HOUR};

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static VOIDPTR updateTime(VOIDPTR);
//-------------------------------------------------------------------------------------------------
static BOOL sigDateTime(UINT32 data);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function sets the system time with respect to RTC. Then it initializes its sub module
 */
void InitDateTime(void)
{
    ONE_MIN_NOTIFY_t    oneMinFun;
    DATE_TIME_CONFIG_t  dateTimeConfig;	//DateTime Cfg local working copy

    MUTEX_INIT(updateTimeMutex, NULL);
    MUTEX_LOCK(updateTimeMutex);
	pthread_cond_init(&updateTimeCond, NULL);
    MUTEX_UNLOCK(updateTimeMutex);

	ReadDateTimeConfig(&dateTimeConfig);
	configTimezone = dateTimeConfig.timezone;
    syncTimeZone = dateTimeConfig.syncTimeZoneToOnvifCam;
	updateTimeF = FALSE;
	updateDstPara = FALSE;
	updateTzF = FALSE;

    GetHwClockTime();
	InitSntp();

    /* Create detached thread to update time in 1 minute periodically */
    if (FALSE == Utils_CreateThread(NULL, updateTime, NULL, DETACHED_THREAD, UPDATE_TIME_THREAD_STACK_SZ))
	{
		EPRINT(DATE_TIME, "update date time thread create fail");
	}

	oneMinFun.funcPtr = sigDateTime;
	oneMinFun.userData = 0;
	RegisterOnMinFun(&oneMinFun);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It finds out current local time in total number of seconds and milliseconds and returns
 *          it as output parameter.
 * @param   pLocalTime
 * @return  If it is unable to get the time, it returns FAIL otherwise SUCCESS
 */
BOOL GetLocalTime(LocalTime_t *pLocalTime)
{
    struct timeval currTimeVal; /* time in secs & milisecs */

    /* Get the current time */
    if (gettimeofday(&currTimeVal, NULL) == NILL)
	{
        EPRINT(DATE_TIME, "gettimeofday failed: [err=%s]", STR_ERR);
        return FAIL;
	}

    /* Store Total Seconds(UTC + TZ + DST) and milliseconds */
    pLocalTime->totalSec = currTimeVal.tv_sec + TimeZoneinSec[configTimezone - 1] + DstPeriodInSec;
    pLocalTime->mSec = currTimeVal.tv_usec / 1000;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function converts the local time passed in as broken time to equivalent time_t	and
 *          returns it as output parameter.
 * @param   pBrokenTime
 * @param   pLocalTimeInSec
 * @return  If conversion fails, it returns FAIL otherwise SUCCESS
 */
BOOL ConvertLocalTimeInSec(const struct tm *pBrokenTime, time_t *pLocalTimeInSec)
{
    struct tm brokenTime = *pBrokenTime;

    /* The number of years since 1900 */
    brokenTime.tm_year -= START_YEAR;

    /* Covert the broken down time into total number of secs */
    if ((*pLocalTimeInSec = mktime(&brokenTime)) == NILL)
	{
        EPRINT(DATE_TIME, "mktime failed: [err=%s]", STR_ERR);
        return FAIL;
	}

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function converts the local time given in time_t* to the equivalent broken down time
 *          structure and returns it as output parameter.
 * @param   pLocalTimeInSec
 * @param   pBrokenTime
 * @return  If conversion fails, it returns FAIL otherwise SUCCESS
 */
BOOL ConvertLocalTimeInBrokenTm(const time_t *pLocalTimeInSec, struct tm *pBrokenTime)
{
    /* Get the broken time structure equivalent of above input time */
    if (NULL == gmtime_r(pLocalTimeInSec, pBrokenTime))
	{
        EPRINT(DATE_TIME, "gmtime_r failed: [err=%s]", STR_ERR);
        return FAIL;
    }

    /* Add century to the year */
    pBrokenTime->tm_year += START_YEAR;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function finds the current local time in total number of seconds. Then convert it to the
 *          broken time structure and values of broken down structure is returned as output parameter.
 * @param   pLocalTimeInSec
 * @return  If it is unable to get the time, it returns FAIL otherwise SUCCESS
 */
BOOL GetLocalTimeInSec(time_t *pLocalTimeInSec)
{
    /* Get the System Time */
    if (time(pLocalTimeInSec) == NILL)
	{
        EPRINT(DATE_TIME, "time failed: [err=%s]", STR_ERR);
        return FAIL;
    }

    /* Add TimeZone and DST to UTC */
    *pLocalTimeInSec += (TimeZoneinSec[configTimezone - 1] + DstPeriodInSec);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function calls time function to get system time in UTC. Then applies local time and
 *          convert it to the broken time structure. This broken time structure pointer is passed in
 *          output parameter.
 * @param   pBrokenTime
 * @return  If it is unable to get the time, it returns FAIL otherwise SUCCESS
 */
BOOL GetLocalTimeInBrokenTm(struct tm *pBrokenTime)
{
    time_t localTimeInSec;

    /* Get local time in seconds */
    if (FAIL == GetLocalTimeInSec(&localTimeInSec))
    {
        EPRINT(DATE_TIME, "fail to get local time in seconds");
        return FAIL;
    }

    /* Convert above time in broken down time structure */
    if (FAIL == ConvertLocalTimeInBrokenTm(&localTimeInSec, pBrokenTime))
    {
        EPRINT(DATE_TIME, "fail to convert local time in broken time");
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is thread entry function which initiates SNTP sync if date-time mode is auto and
 *          updates DST status periodically if DST is enabled. SNTP sync is also updated periodically
 *          as per configured duration.
 * @param   arg
 * @return
 */
static VOIDPTR updateTime(VOIDPTR arg)
{
	BOOL					ntpUpdateF = TRUE, dstUpdate = TRUE, tzUpdate = FALSE;
	UINT8					nextNtpUpdateHr;	//next time to sync with NTP
	UINT8					nextRtcReadHr;		//next time to read from RTC
	UINT8					count;
	struct tm				currLocalTm = { 0 };	//current time in broken down struct
	DATE_TIME_CONFIG_t		dateTimeConfig;	//DateTime Cfg local working copy

    THREAD_START("DATE_TIME");

	nextRtcReadHr = TWENTYFOUR_HOUR;
	if(SUCCESS != GetLocalTimeInBrokenTm(&currLocalTm))
	{
        EPRINT(DATE_TIME, "fail to get local time in broken time");
	}

	for (count = 0; count < MAX_NO_HOURS; count++)
	{
		//if current hour matches with configured autoUpdate Hr
		if(currLocalTm.tm_hour < manualUpdateHrTab[count])
		{
			nextRtcReadHr = manualUpdateHrTab[count];
			break;
		}
	}

	while(1)
	{
		if(SUCCESS != GetLocalTimeInBrokenTm(&currLocalTm))
		{
            EPRINT(DATE_TIME, "fail to get local time in broken time");
			continue;
		}

		// SNTP Update
		ReadDateTimeConfig(&dateTimeConfig);		//get DateTime working copy

		if(tzUpdate == TRUE)
		{
			configTimezone = dateTimeConfig.timezone;
			WriteEvent(LOG_SYSTEM_EVENT, LOG_RTC_UPDATE, DATE_TIME_AUTO_TIME_ZONE, NULL, EVENT_ALERT);
			DPRINT(SYS_LOG, "System Timezone Updated");
		}

		if(dateTimeConfig.updateMode == DATE_TIME_UPDATE_AUTO)
		{
			//sync with configured NTP server at configured rate
			ExecuteSntp(ntpUpdateF, &dateTimeConfig, &currLocalTm, &nextNtpUpdateHr);
		}

		// Kernal time update from RTC at every 6 hours
		//if current hour is hour to syn System with RTC
		if(currLocalTm.tm_hour == nextRtcReadHr)
		{
			//sync with hardware clock to remove drift
			if(GetHwClockTime() == SUCCESS)
			{
				nextRtcReadHr = TWENTYFOUR_HOUR;
				//find next update hour
				for (count = 0; count < MAX_NO_HOURS; count++)
				{
					//if current hour matches with configured autoUpdate Hr
					if(currLocalTm.tm_hour < manualUpdateHrTab[count])
					{
						nextRtcReadHr = manualUpdateHrTab[count];
						break;
					}
				}
			}
		}

		UpdateDstFlag(dstUpdate);

		//Block the thread for the conditional signal
		//The dateTimeTimer wakes it up at One Minute
        MUTEX_LOCK(updateTimeMutex);

		if( (updateTimeF == FALSE) && (updateDstPara == FALSE) && (updateTzF == FALSE) )
		{
			pthread_cond_wait(&updateTimeCond, &updateTimeMutex);
		}
		dstUpdate = updateDstPara;
		updateDstPara = FALSE;
		ntpUpdateF = updateTimeF;
		updateTimeF = FALSE;
		tzUpdate = updateTzF;
		updateTzF = FALSE;
        MUTEX_UNLOCK(updateTimeMutex);
	}

	pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is the function which is called when the Date Time Configuration changes are notified.
 * @param   newDateTimeConfig
 * @param   oldDateTimeConfig
 */
void UpdateDateTimeConfig(DATE_TIME_CONFIG_t newDateTimeConfig, DATE_TIME_CONFIG_t *oldDateTimeConfig)
{
    BOOL ntpUpdateF = FALSE, tzUpdateF = FALSE;

	if(configTimezone != newDateTimeConfig.timezone)
	{
		tzUpdateF = TRUE;
	}

    if (((newDateTimeConfig.updateMode != oldDateTimeConfig->updateMode) && (newDateTimeConfig.updateMode == DATE_TIME_UPDATE_AUTO))
            || (memcmp(&newDateTimeConfig.ntp, &oldDateTimeConfig->ntp, sizeof(NTP_PARAMETER_t)) != STATUS_OK))
	{
		ntpUpdateF = TRUE;
	}

    syncTimeZone = newDateTimeConfig.syncTimeZoneToOnvifCam;
	DPRINT(DATE_TIME, "Updated new Date Time parameters");

	//signal the thread to wake up
    MUTEX_LOCK(updateTimeMutex);
	updateTimeF = ntpUpdateF;
	updateTzF = tzUpdateF;
	pthread_cond_signal(&updateTimeCond);
    MUTEX_UNLOCK(updateTimeMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is invoked when the user changes the DST configurations. It updates the Dst
 *          working copy. It calls initDstParam to find new forward, reverse time instance and the
 *          DST period to be applied.
 * @param   newDstConfig
 */
void UpdateDstConfig(DST_CONFIG_t newDstConfig)
{
	//signal the thread to wake up
    MUTEX_LOCK(updateTimeMutex);
	updateDstPara = TRUE;
	pthread_cond_signal(&updateTimeCond);
    MUTEX_UNLOCK(updateTimeMutex);
    DPRINT(DATE_TIME, "Updated new DST parameters");
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   sigDateTime
 * @param   data
 * @return
 */
static BOOL sigDateTime(UINT32 data)
{
	//signal the thread to wake up
    MUTEX_LOCK(updateTimeMutex);
	pthread_cond_signal(&updateTimeCond);
    MUTEX_UNLOCK(updateTimeMutex);
	return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function parse the date time string passed in 1st argument to the Broken time
 *          structure sets it as Date and Time in NVR.
 * @param   newDateTime
 * @param   username
 * @return  If it successfully sets the the date and time it return SUCCESS otherwise it returns FAIL.
 */
BOOL SetNewDateTime(struct tm *newDateTime, CHAR *username)
{
	BOOL 	retValue = SUCCESS;
	time_t 	newDateTimeInSec;
	UINT8 	maxDateOfMonth[MAX_MONTH] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	DPRINT(DATE_TIME, "%d:%d:%d %d/%d/%d is date to be set from [%s] user",
			newDateTime->tm_hour, newDateTime->tm_min, newDateTime->tm_sec,
            newDateTime->tm_mday, newDateTime->tm_mon, newDateTime->tm_year, username);

	if(newDateTime->tm_year >= START_YEAR)
	{
		if( (newDateTime->tm_mon >= JANUARY) && (newDateTime->tm_mon <= DECEMBER) )
		{
			if(newDateTime->tm_mon == FEBRUARY)
			{
				//leap year
                if(newDateTime->tm_year % 400 == 0 || (newDateTime->tm_year % 100 != 0 && newDateTime->tm_year % 4 == 0))
				{
					//invalid date
                    if( (newDateTime->tm_mday > (maxDateOfMonth[FEBRUARY] + 1)) || (newDateTime->tm_mday < 0) )
					{
						retValue = FAIL;
					}
				}
				else			//other than leap year
				{
                    if( (newDateTime->tm_mday > (maxDateOfMonth[FEBRUARY])) || (newDateTime->tm_mday < 0) )
					{
						retValue = FAIL;
					}
				}
			}
			else					//other than February
			{
				//invalid date
                if( (newDateTime->tm_mday > (maxDateOfMonth[newDateTime->tm_mon])) || (newDateTime->tm_mday < 0) )
				{
					retValue = FAIL;
				}
			}
		}
		else						//invalid month
		{
			retValue = FAIL;
		}
	}
	else							//invalid year
	{
		retValue = FAIL;
	}

	if(retValue == SUCCESS)
	{
		if( (newDateTime->tm_hour >= 0) && (newDateTime->tm_hour <= 23) )
		{
			if( (newDateTime->tm_min >= 0) && (newDateTime->tm_min <= 59) )
			{
				if( (newDateTime->tm_sec < 0) || (newDateTime->tm_sec > 59) )
				{
					retValue = FAIL;
				}
			}
			else		//invalid minute
			{
				retValue = FAIL;
			}
		}
		else			//invalid hour
		{
			retValue = FAIL;
		}

		if(retValue == SUCCESS)
		{
			//convert the input local time to total seconds
			if(ConvertLocalTimeInSec(newDateTime, &newDateTimeInSec) == SUCCESS)
			{
                //notify the change in the date time
                retValue = SetTimeManually(newDateTimeInSec, configTimezone, username);
				if(retValue == SUCCESS)
				{
					DPRINT(DATE_TIME, "Date Time Set Successfully");
				}
			}
		}
	}

	if(retValue == SUCCESS)
	{
		CiActionChangeDateTime();
	}

	return retValue;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Function that gets the time elapsed since system power on in seconds
 * @return  Elapsed time in seconds
 */
UINT64 GetMonotonicTimeInSec(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (UINT64) (ts.tv_sec);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Function that gets the time elapsed since system power on in milli seconds
 * @return  Elapsed time in milli seconds
 */
UINT64 GetMonotonicTimeInMilliSec(void)
{
	struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (UINT64) (((UINT64)ts.tv_sec * 1000) + (ts.tv_nsec / NANO_SEC_PER_MILLI_SEC));
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Function that gets the time elapsed since system power on in nanosec
 * @return  Elapsed time in nanosec
 */
UINT64 GetMonotonicTimeInNanoSec(void)
{
    struct timespec ts;

    /* Get time in form of sec & nano sec */
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (UINT64) ((((UINT64)ts.tv_sec) * NANO_SEC_PER_SEC) + ts.tv_nsec);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   GetUtcTimeForOnvif
 * @param   tempTime
 * @param   TimezoneIndex
 * @param   updateTimeZone
 * @return  Success or Fail
 */
BOOL GetUtcTimeForOnvif(struct tm *pBrokenTime, UINT8PTR pTimeZoneIdx, UINT8PTR pTimeZoneSyncNeeded)
{
    time_t localTimeInSec;

    if (time(&localTimeInSec) == NILL)
	{
        EPRINT(DATE_TIME, "time failed: [err=%s]", STR_ERR);
        return FAIL;
    }

    if (gmtime_r(&localTimeInSec, pBrokenTime) == NULL)
    {
        EPRINT(DATE_TIME, "gmtime_r failed: [err=%s]", STR_ERR);
        return FAIL;
    }

    *pTimeZoneIdx = configTimezone;
    *pTimeZoneSyncNeeded = syncTimeZone;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get system configured timezone
 * @return  Timezone index (Starts from 1)
 */
UINT8 GetSystemTimezone(void)
{
    /* Timezone index */
    return configTimezone;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
