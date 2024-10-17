#ifndef SNTPCLIENT_H_
#define SNTPCLIENT_H_
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		SntpClient.h
@brief      SntpClient is sub-module of Date Time Module. This sub-module introduces SNTP i.e. Simple
            Network Time Protocol which is used to synchronize the system clock with respect to a
            reference clock (that provides the current time) since the computer's clock is prone to
            drift over time.  Here we are using open source utility sntp to achieve our goal. This
            sub-module provides the APIs to initialize; start and stop the SNTP utility so to synchronize
            the NVR RTC clock. StartSntp API is used to synchronize the NVR system time with respect
            to the configured NTP server. When successfully synchronized, StartNtp again synchronizes
            NVR system time periodically in the configured update interval otherwise it retries
            periodically in the predefined interval.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "MxTypedef.h"

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitSntp(void);
//-------------------------------------------------------------------------------------------------
BOOL ExecuteSntp(BOOL restartSntpF, DATE_TIME_CONFIG_t * dateTime, struct tm * currTmPtr, UINT8PTR nextUpdateHr);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif
