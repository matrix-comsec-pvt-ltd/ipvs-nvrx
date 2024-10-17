//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		Dst.c
@brief      DST is sub-module of Date Time Module. And it provides the API which checks whether the
            current time instance falls within the forward and reverse time instance. And if so, Date
            Time Module returns the DST applied time in seconds and in broken time structure. The
            system time is set to the standard time which is the UTC plus the time considering the
            time zone. But, this sub-module gives us the local time i.e. the UTC plus time zone plus
            DST time.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "Dst.h"
#include "DateTime.h"
#include "EventLogger.h"
#include "DebugLog.h"
#include "CameraInterface.h"

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
	//when forward instance > reverse instance, both appear in same year
	SAME_YEAR = 0,
	//when reverse instance > forward instance, reverse day is applied next year
	ROLL_OVER,
	//max value
	DST_TYPE_MAX
}DST_TYPE_e;

typedef enum
{
	DST_NORMAL = 0,
	DST_APPL,
	DST_OVER,
	DST_ACT_MAX
}DST_ACTION_e;

//#################################################################################################
// @GLOBAL VARIABLES
//#################################################################################################
INT32 DstPeriodInSec = 0;   //DST interval

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static time_t 			stdFwdInstance, stdRvsInstance;
static INT32 			currDstYear;		//year in which DST param initialized
static DST_TYPE_e 		currDstType;		//SAME YEAR or ROLL OVER
static DST_ACTION_e 	dstFlag = DST_NORMAL;

static const CHARPTR dstAppStr[DST_ACT_MAX] =
{
	"DST Normal",
	"DST Appl",
	"DST Over",
};

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static time_t getDstDate(DST_CLOCK_t * dstClockPtr);
//-------------------------------------------------------------------------------------------------
static UINT8 getDateFromDST( DST_CLOCK_t DstClock, INT16 currDstYear);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function finds out the forward time instance, reverse time instance for current year
 *          in terms of time_t.It also calculates DST period from the configurations of DST forward
 *          Clock and DST reverse Clock. finds the type of DST i.e. if forward and reverse instance
 *          fall in the same year or the reverse instance fall in the next year. This function also
 *          finds DST flag (whether dst should be applied or not).
 * @param   currTimePtr
 * @param   dstConfigPtr
 */
void InitDstParam(struct tm * currTimePtr, DST_CONFIG_t * dstConfigPtr)
{
	currDstYear = currTimePtr->tm_year;

	//store the date from where Forward Clock begins
	stdFwdInstance = getDstDate(&dstConfigPtr->forwardClock);

	//store the date from where Reverse Clock begins
	stdRvsInstance = getDstDate(&dstConfigPtr->reverseClock);

	//find the DST type
	if(stdFwdInstance < stdRvsInstance)
	{
		currDstType = SAME_YEAR;
        DPRINT(DATE_TIME, "dst type same year");
	}
	else
	{
		currDstType = ROLL_OVER;
        DPRINT(DATE_TIME, "dst type roll over year");
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function checks whether the current time falls between the DST window.If so, it
 *          sets DST flag and stores the DST period in a variable which is used to calculate local time.
 * @param   initDst
 */
void UpdateDstFlag(BOOL initDst)
{
	time_t 			currTime = { 0 };		//current time in seconds
	struct 	tm 		currLocalTm = { 0 };	//current time in broken down struct
    DST_CONFIG_t	dstConfig;              //Dst Cfg local working copy
	INT32			tPrevDstPeriodInSec = DstPeriodInSec;

	if(dstFlag != DST_NORMAL)
	{
        DPRINT(DATE_TIME, "dst updated: [action=%s]", dstAppStr[dstFlag]);
	}

	ReadDstConfig(&dstConfig);
	if(dstConfig.dst == ENABLE)
	{
		if(SUCCESS != GetLocalTimeInSec(&currTime))
		{
            EPRINT(DATE_TIME, "fail to get local time");
		}

		if(SUCCESS != ConvertLocalTimeInBrokenTm(&currTime, &currLocalTm))
		{
            EPRINT(DATE_TIME, "fail to get local time in broken time");
		}

		//if the current year doesn't match with DST Year
        if((currLocalTm.tm_year != currDstYear) || (initDst == TRUE))
		{
			//we need to initialize the DST parameters again
			InitDstParam(&currLocalTm, &dstConfig);
		}

        //Find the current time falls within which DST window and according set or reset the DST FLAG and calculate dstPeriodInSec
		if(currDstType == SAME_YEAR)
		{
			if(dstFlag == DST_NORMAL)
			{
				//if current time is between DST window
                if ((currTime >= stdFwdInstance) && (currTime < stdRvsInstance))
				{
					dstFlag = DST_APPL;
                    DstPeriodInSec = (dstConfig.period.hour * SEC_IN_ONE_HOUR) + (dstConfig.period.minute * SEC_IN_ONE_MIN);
					WriteEvent(LOG_SYSTEM_EVENT, LOG_DST_EVENT, NULL, NULL, EVENT_ACTIVE);
                    DPRINT(DATE_TIME, "dst same year applied");
				}
			}
			else if(dstFlag == DST_APPL)
			{
				//if current time is not within DST window
				if(currTime >= stdRvsInstance)
				{
					dstFlag = DST_OVER;
					DstPeriodInSec = 0;			//Set DST period to 0
					WriteEvent(LOG_SYSTEM_EVENT, LOG_DST_EVENT, NULL, NULL, EVENT_NORMAL);
                    DPRINT(DATE_TIME, "dst same year over");
				}
			}
			else
			{
				if(currTime > stdRvsInstance)
				{
					dstFlag = DST_NORMAL;
                    DPRINT(DATE_TIME, "dst same year normal");
				}
			}
		}
		else
		{
			if(dstFlag == DST_NORMAL)
			{
                if((currTime <= stdRvsInstance) || (currTime > stdFwdInstance))
				{
					dstFlag = DST_APPL;
					//calculate DST period in Secs
                    DstPeriodInSec = (dstConfig.period.hour * SEC_IN_ONE_HOUR) + (dstConfig.period.minute * SEC_IN_ONE_MIN);
					WriteEvent(LOG_SYSTEM_EVENT, LOG_DST_EVENT, NULL, NULL, EVENT_ACTIVE);
                    DPRINT(DATE_TIME, "dst roll over year applied");
				}
			}
			else if(dstFlag == DST_APPL)
			{
                if((currTime >= stdRvsInstance) && (currTime < stdFwdInstance))
				{
					dstFlag = DST_OVER;
					DstPeriodInSec = 0;			//Set DST period to 0
					WriteEvent(LOG_SYSTEM_EVENT, LOG_DST_EVENT, NULL, NULL, EVENT_NORMAL);
                    DPRINT(DATE_TIME, "dst roll over year over");
				}
			}
			else
			{
				if(currTime > stdRvsInstance)
				{
					dstFlag = DST_NORMAL;
                    DPRINT(DATE_TIME, "dst roll over year normal");
				}
			}
		}
	}
	else if(dstFlag != DST_NORMAL)
	{
        dstFlag = DST_NORMAL;       //clear DST flag
		DstPeriodInSec = 0;			//Set DST period to 0
		WriteEvent(LOG_SYSTEM_EVENT, LOG_DST_EVENT, NULL, NULL, EVENT_NORMAL);
        DPRINT(DATE_TIME, "dst disable, so consider normal");
	}

	/* Update Time with DST to Cameras */
	if(tPrevDstPeriodInSec != DstPeriodInSec)
	{
		CiActionChangeDateTime();
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function finds out the exact date of specified Dst Clock for the specified year.
 * @param   dstClockPtr
 * @return  time_t
 */
static time_t getDstDate(DST_CLOCK_t * dstClockPtr)
{
	struct 	tm 	brokenTime;
	time_t 		brkInstance;
	UINT8 		dstDate;			//Date of the input clock
	UINT8 		maxDate = 30;		//maximum date of input clock

	//Find the weekday on 1st of month of input clock
	brokenTime.tm_sec = 0;
	brokenTime.tm_min = dstClockPtr->instance.minute;
	brokenTime.tm_hour = dstClockPtr->instance.hour;
	brokenTime.tm_mday = 1;
	brokenTime.tm_mon = dstClockPtr->day.month;
	brokenTime.tm_year = (currDstYear - START_YEAR);

	brkInstance = mktime(&brokenTime);
	gmtime_r(&brkInstance, &brokenTime);

	if(dstClockPtr->day.month < AUGUST)
	{
		if(dstClockPtr->day.month == FEBRUARY)
		{
			//leap year
            if((brokenTime.tm_year % 400 == 0) || ((brokenTime.tm_year % 100 != 0) && (brokenTime.tm_year % 4 == 0)))
			{
				maxDate = 29;
			}
			else
			{
				maxDate = 28;
			}
		}
		else
		{
            if((dstClockPtr->day.month % 2) == 0)
			{
				maxDate = 31;
			}
			else
			{
				maxDate = 30;
			}
		}
	}
	else
	{
        if ((dstClockPtr->day.month % 2) == 1)
		{
			maxDate = 31;
		}
		else
		{
			maxDate = 30;
		}
	}

    //Find the date of input clock. if DST week day specified is not present in 1st week
    if((INT32)dstClockPtr->day.weekDay < brokenTime.tm_wday)
	{
		//we shift backwards to DST week day then add DST clock week number
        dstDate = ((MAX_WEEK_DAYS - (brokenTime.tm_wday - dstClockPtr->day.weekDay)) + (MAX_WEEK_DAYS * (dstClockPtr->day.week)) + 1);
	}
	//if DST week day is greater than weekday of 1st of Dst month
    else if((INT32)dstClockPtr->day.weekDay > brokenTime.tm_wday)
	{
		//we will shift forward to DST weekday then add DST clock week number
        dstDate = ((dstClockPtr->day.weekDay - brokenTime.tm_wday) + (MAX_WEEK_DAYS * dstClockPtr->day.week) + 1);
	}
	//if DST week day and weekday of 1st of the DST month are same
	else
	{
		//we will add DST week numbers to find exact date
		dstDate = ((MAX_WEEK_DAYS * dstClockPtr->day.week) + 1);
	}

	if(dstDate > maxDate)
	{
        EPRINT(DATE_TIME, "configured dst date does not exist for current year/month");
	}

    DPRINT(DATE_TIME, "dst info: [date=%d], [month=%d], [year=%d]", dstDate, dstClockPtr->day.month+1, currDstYear);
	brokenTime.tm_sec = 0;
	brokenTime.tm_min = dstClockPtr->instance.minute;
	brokenTime.tm_hour = dstClockPtr->instance.hour;
	brokenTime.tm_mday = dstDate;
	brokenTime.tm_mon = dstClockPtr->day.month;
	brokenTime.tm_year = (currDstYear - START_YEAR);
	brkInstance = mktime(&brokenTime);
	return brkInstance;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function finds out whether the given time instance is invalid or within period when
 *          DST has to be applied, out of period where DST is applied or if DST is disable.By invalid
 *          instance we mean that it cannot occur. for eg. If DST is to be applied at 2 am of 2nd April
 *          and forwarded to 3 am and user has given new time 2.15 am of 2nd April. So, it is invalid.
 * @param   newTime
 * @param   dstTimePeriod
 * @return
 */
DST_SCALE_e CheckTimeInDstScale(time_t newTime,time_t *dstTimePeriod)
{
    struct tm       newLocalTm;     //current time in broken down struct
    struct tm       fwdTime;        //Forward Instance broken down struct
    struct tm       rvsTime;        //Reverse Instance broken down struct
    DST_SCALE_e     currDstScale = INSTANCE_OUTOF_DST;
    DST_CONFIG_t    dstConfig;

	ReadDstConfig(&dstConfig);
    if(dstConfig.dst == DISABLE)
	{
        return DST_DISABLED;
    }

    *dstTimePeriod = DstPeriodInSec;		//dst Period
    if(currDstType == SAME_YEAR)
    {
        if(newTime >= stdFwdInstance)
        {
            //if input time is in invalid span of time
            if(newTime < (stdFwdInstance + DstPeriodInSec))
            {
                currDstScale = INVALID_INSTANCE;
            }

            //if input time is within DST window
            else if((newTime < stdRvsInstance) && ((newTime - stdFwdInstance) > DstPeriodInSec) && ((stdRvsInstance - stdFwdInstance) > 0))
            {
                currDstScale	=	INSTANCE_IN_DST;
            }

            //if input time occurs after Reverse Time
            if(newTime > stdRvsInstance)
            {
                currDstScale = INSTANCE_OUTOF_DST;
            }
        }
        else    //if input time occurs before forward time
        {
            currDstScale = INSTANCE_OUTOF_DST;
        }
    }
    else        //DST Type = ROLL_OVER
    {
        gmtime_r(&newTime, &newLocalTm);    //convert it to broken down time

        //if the current year doesn't match with DST Year
        if(newLocalTm.tm_year != currDstYear)
        {
            currDstYear = newLocalTm.tm_year;			//store current Dst Year

            //store the date from where Forward Clock begins
            fwdTime.tm_mday = getDateFromDST(dstConfig.forwardClock, currDstYear);

            //store the minute, hour and month parameters of stdRvsClock
            fwdTime.tm_mon = dstConfig.forwardClock.day.month;
            fwdTime.tm_year = currDstYear;
            fwdTime.tm_min = dstConfig.forwardClock.instance.minute;
            fwdTime.tm_hour = dstConfig.forwardClock.instance.hour;
            fwdTime.tm_sec = 0;

            //get standard forward instance using mktime
            stdFwdInstance = mktime(&fwdTime);

            //store the date from where Reverse Clock begins
            rvsTime.tm_mday = getDateFromDST(dstConfig.reverseClock, currDstYear);

            //store the minute, hour and month parameters of stdRvsClock
            rvsTime.tm_mon = dstConfig.reverseClock.day.month;
            rvsTime.tm_year = currDstYear;
            rvsTime.tm_min = dstConfig.reverseClock.instance.minute;
            rvsTime.tm_hour = dstConfig.reverseClock.instance.hour;
            rvsTime.tm_sec = 0;
            stdRvsInstance = mktime(&rvsTime);		//get reverse instance
        }

        //if input time occurs before Reverse Time
        if(newTime < stdRvsInstance)
        {
            currDstScale = INSTANCE_IN_DST;
        }

        //if input time occurs after forward Instance
        else if(newTime >= stdFwdInstance)
        {
            //if input time Instance is in invalid span of time
            if(newTime < (stdFwdInstance + DstPeriodInSec))
            {
                currDstScale = INVALID_INSTANCE;
            }
        }
        else		//if input time fall out of DST window
        {
            currDstScale = INSTANCE_OUTOF_DST;
        }
    }

	return currDstScale;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function finds out the exact date of specified Dst Clock for the specified year.
 * @param   DstClock
 * @param   currDstYear
 * @return  dstDate
 */
static UINT8 getDateFromDST( DST_CLOCK_t DstClock, INT16 currDstYear)
{
    struct tm   brokenTime;
    INT16       dstDate;			//Date of the input clock
    INT16       maxDate = 30;		//maximum date of input clock

	//Find the weekday on 1st of month of input clock
	brokenTime.tm_sec = 0;
	brokenTime.tm_min = DstClock.instance.minute;
	brokenTime.tm_hour = DstClock.instance.hour;
	brokenTime.tm_mday = 1;
	brokenTime.tm_mon = DstClock.day.month;
	brokenTime.tm_year = currDstYear;

	mktime(&brokenTime);
	if(DstClock.day.month < AUGUST)
	{
		if(DstClock.day.month == FEBRUARY)
		{
            //leap year
            if(brokenTime.tm_year % 400 == 0 || (brokenTime.tm_year % 100 != 0 && brokenTime.tm_year % 4 == 0))
            {
                maxDate = 29;
            }
            else
            {
                maxDate = 28;
            }
		}
		else
		{
			if(DstClock.day.month % 2 == 0)
			{
				maxDate = 31;
			}
			else
			{
				maxDate = 30;
			}
		}
	}
	else
	{
		if (DstClock.day.month % 2 == 1)
		{
			maxDate = 31;
		}
		else
		{
			maxDate = 30;
		}
	}

	//Find the date of input clock
	do
	{
		//if DST week day specified is not present in 1st week
		if(((INT32)DstClock.day.weekDay) < brokenTime.tm_wday)
		{
			//we shift backwards to DST week day then add DST clock week number
            dstDate = 1 + (MAX_WEEK_DAYS - (brokenTime.tm_wday - DstClock.day.weekDay)) + (MAX_WEEK_DAYS*(DstClock.day.week));
		}
		//if DST week day is greater than weekday of 1st of Dst month
		else if(((INT32)DstClock.day.weekDay) > brokenTime.tm_wday)
		{
			//we will shift forward to DST weekday then add DST clock week number
            dstDate = 1 + (DstClock.day.weekDay - brokenTime.tm_wday) + (MAX_WEEK_DAYS*(DstClock.day.week));
		}
		//if DST week day and weekday of 1st of the DST month are same
		else
		{
			//we will add DST week numbers to find exact date
			dstDate = 1 + (MAX_WEEK_DAYS*(DstClock.day.week));
		}

		//actual weeks of the month may be configured weekno
		DstClock.day.week--;

	}while(dstDate > maxDate); 		//Until dstDate falls within maxDate

	return dstDate;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
