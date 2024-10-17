#ifndef DATETIME_H_
#define DATETIME_H_
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		DateTime.h
@brief      The purpose of Date-Time Module is to provide the API's which
            1. Initialize the system time from RTC and apply time zone to it.
            2. Provides the local time of user (after checking whether DST is enabled or not) and
            3. Start SNTP client if System time update is set to auto.
            4. Set or update the NVR system and RTC time.

            This module is further divided into three sub-modules to fulfill three different purpose
            of this Module. The three sub-modules and their functionality are as follows:-
            1.DST: - If DST is enabled, this sub-module provides the API which checks whether the
            current time instance falls within the forward and reverse time instance. And if so, DST
            period in second is added to the RTC and the system time. And when DST should be removed
            the DST period is subtracted from the RTC and the system time.
            2. SNTP:-This sub-module uses SNTP utility i.e. Simple Network Time Protocol which is used
            to synchronize the system clock with respect to a reference clock (from sntp server).
            This sub-module provides the APIs to initialize; start and stop the SNTP utility so to
            synchronize the NVR RTC clock
            3. Manual:- This is the sub module which is used to set the date and time of NVR as given by the user.
*/

//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <sys/time.h>
#include <time.h>

/* Application Includes */
#include "ConfigApi.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define START_YEAR				1900

#define RESET_EXTRA_BORKEN_TIME_VAR(brokenTime) \
    brokenTime.tm_wday = 0;                     \
    brokenTime.tm_yday = 0;                     \
    brokenTime.tm_isdst = 0;                    \
    brokenTime.tm_gmtoff = 0;                   \
    brokenTime.tm_zone = NULL;

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef struct __attribute__((packed))
{
	UINT32 totalSec;	//for total number of seconds
	UINT16 mSec;		//milliseconds of the instance
}LocalTime_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitDateTime(void);
//-------------------------------------------------------------------------------------------------
BOOL GetLocalTimeInSec(time_t *pLocalTimeInSec);
//-------------------------------------------------------------------------------------------------
BOOL GetLocalTimeInBrokenTm(struct tm *pBrokenTime);
//-------------------------------------------------------------------------------------------------
BOOL GetLocalTime(LocalTime_t *pLocalTime);
//-------------------------------------------------------------------------------------------------
BOOL ConvertLocalTimeInSec(const struct tm *pBrokenTime, time_t *pLocalTimeInSec);
//-------------------------------------------------------------------------------------------------
BOOL ConvertLocalTimeInBrokenTm(const time_t *pLocalTimeInSec, struct tm *pBrokenTime);
//-------------------------------------------------------------------------------------------------
void UpdateDateTimeConfig(DATE_TIME_CONFIG_t newDateTimeConfig, DATE_TIME_CONFIG_t *oldDateTimeConfig);
//-------------------------------------------------------------------------------------------------
void UpdateDstConfig(DST_CONFIG_t newDstConfig);
//-------------------------------------------------------------------------------------------------
BOOL SetNewDateTime(struct tm *newDateTime, CHAR *username);
//-------------------------------------------------------------------------------------------------
BOOL GetUtcTimeForOnvif(struct tm *pBrokenTime, UINT8PTR pTimeZoneIdx, UINT8PTR pTimeZoneSyncNeeded);
//-------------------------------------------------------------------------------------------------
UINT8 GetSystemTimezone(void);
//-------------------------------------------------------------------------------------------------
UINT64 GetMonotonicTimeInSec(void);
//-------------------------------------------------------------------------------------------------
UINT64 GetMonotonicTimeInMilliSec(void);
//-------------------------------------------------------------------------------------------------
UINT64 GetMonotonicTimeInNanoSec(void);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif /* DATETIME_H_ */
