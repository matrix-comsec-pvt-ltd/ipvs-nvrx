//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		SntpClient.c
@brief      This sub-module uses SNTP i.e. Simple Network Time Protocol to synchronize the system
            clock with respect to a reference clock (NTP server).  This open source utility SNTP
            is of version 4 and part of ntp-4.2.6p3 package. The utility will be run in the thread
            "UpdateTimeWithSntp" using system command. On successful synchronization of NVR system
            time, the StartSntp is called again in the configured time interval to synchronize the
            time again. But, on failure, the thread waits for predefined interval and then retries again.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "DateTime.h"
#include "SntpClient.h"
#include "Manual.h"
#include "DebugLog.h"
#include "EventLogger.h"
#include "Utils.h"
#include "CameraInterface.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define	MAX_HOUR_NO         4
#define TWENTYFOUR_HOUR     0
#define SIX_HOUR            6
#define TWELVE_HOUR         12
#define EIGHTEEN_HOUR       18
#define SNTP_CMD            "sntp -t 10 -S -K /dev/null %s > /dev/null"
#define DATE_TIME_NTP       "01"

//#################################################################################################
// @DATA TYPES
//#################################################################################################
//Possible status of time synchronization of NVR using SNTP Utility
typedef enum
{
	SNTP_IDLE,			//Sntp utility not running
	SNTP_PROCESS, 		//Sntp utility still executing
	SNTP_UPDATED,		//Sntp successfully updated the time
}SNTP_STATE_e;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static SNTP_STATE_e 	currSntpState;			//current state of SNTP

//Possible NTP Server to be used for synchronization
static const CHARPTR sntpServerTab[MAX_TIME_SERVER] =
{
    "time.google.com",
    "time.windows.com",
    "time.nist.gov"
};

//NTP Interval LOOKUP Table
static const UINT8 autoUpdateHrTab[MAX_NTP_UPDATE][MAX_HOUR_NO] =
{
	{
		SIX_HOUR,
		TWELVE_HOUR,
		EIGHTEEN_HOUR,
		TWENTYFOUR_HOUR
	},
	{
		TWENTYFOUR_HOUR,
		TWELVE_HOUR,
		TWENTYFOUR_HOUR,
		TWELVE_HOUR
	},
	{
		TWENTYFOUR_HOUR,
		TWENTYFOUR_HOUR,
		TWENTYFOUR_HOUR,
		TWENTYFOUR_HOUR
	}
};

//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   Initialization of sntp
 */
void InitSntp(void)
{
	currSntpState = SNTP_IDLE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function executes sntp utility using system command
 * @param   restartSntpF
 * @param   dateTime
 * @param   currTmPtr
 * @param   nextUpdateHr
 * @return  It returns SUCCESS on successful synchronization otherwise it returns FAIL.
 */
BOOL ExecuteSntp(BOOL restartSntpF, DATE_TIME_CONFIG_t *dateTime, struct tm *currTmPtr, UINT8PTR nextUpdateHr)
{
    CHAR        cmd[250];
    UINT8       count;
    struct 	tm  currLocalTm;		//current time broken down struct.
    UINT8       serverNumber = USER_DEFINED_NTP;

	if(restartSntpF == TRUE)
	{
		currSntpState = SNTP_IDLE;
	}

	switch(currSntpState)
	{
        default:
        case SNTP_IDLE:
        {
            currSntpState = SNTP_PROCESS;
        }
        // Fall through
        case SNTP_PROCESS:
        {
            if (dateTime->ntp.userDefinedServer[0] == '\0')
            {
                serverNumber = WISCONSIN_TIME_SERVER;
            }

            do
            {
                /* Is user defined NTP server? */
                if (serverNumber == USER_DEFINED_NTP)
                {
                    snprintf(cmd, sizeof(cmd), SNTP_CMD, dateTime->ntp.userDefinedServer);
                }
                else
                {
                    /* If internet is not connected then will not do query on default NTP server for sync */
                    if (getInternetConnStatus() != ACTIVE)
                    {
                        break;
                    }

                    /* Do NTP time sync query on default pre-defined NTP server */
                    snprintf(cmd, sizeof(cmd), SNTP_CMD, sntpServerTab[serverNumber - 1]);
                }

                if (ExeSysCmd(TRUE, cmd) == SUCCESS)
                {
                    //if hardware clock successfully set to system time
                    if(SetHwClockTime() == SUCCESS)
                    {
                        if(GetLocalTimeInBrokenTm(&currLocalTm) == SUCCESS)
                        {
                            if (dateTime->ntp.updateInterval !=  NTP_UPDATE_24HOUR)
                            {
                                *nextUpdateHr = TWENTYFOUR_HOUR;

                                //find next update hour
                                for (count = 0; count < MAX_HOUR_NO; count++)
                                {
                                    if (currLocalTm.tm_hour < autoUpdateHrTab[dateTime->ntp.updateInterval][count])
                                    {
                                        //store next update hour
                                        *nextUpdateHr = autoUpdateHrTab[dateTime->ntp.updateInterval][count];
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                *nextUpdateHr = 24;
                            }
                        }

                        CiActionChangeDateTime();
                        WriteEvent(LOG_SYSTEM_EVENT, LOG_RTC_UPDATE, DATE_TIME_NTP, NULL, EVENT_ALERT);
                        DPRINT(SYS_LOG, "sync and set ntp time in rtc successfully");
                        currSntpState = SNTP_UPDATED;
                        return SUCCESS;
                    }
                }
                else
                {
                    EPRINT(DATE_TIME, "fail to sync ntp time: [cmd=%s]", cmd);
                }

            }while(serverNumber++ != NIST_TIME_SERVER);
        }
        break;

        case SNTP_UPDATED:
        {
            if((*nextUpdateHr == 24) && (currTmPtr->tm_hour > 0))
            {
                *nextUpdateHr = TWENTYFOUR_HOUR;
            }

            //if current time matches with next update hour
            if(currTmPtr->tm_hour == *nextUpdateHr)
            {
                currSntpState = SNTP_PROCESS;
            }
        }
        break;
	}

    return FAIL;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
