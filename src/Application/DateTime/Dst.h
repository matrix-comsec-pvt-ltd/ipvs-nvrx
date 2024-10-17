#ifndef DST_H_
#define DST_H_
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		Dst.h
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
/* OS Includes */
#include <time.h>

/* Application Includes */
#include "Config.h"

//#################################################################################################
// @DATA TYPES
//#################################################################################################
//DST scale to identify the given time stamp is in which duration w.r.t DST
typedef enum
{
	INVALID_INSTANCE,			//Instance cannot occur when DST applied
	INSTANCE_IN_DST,			//Instance within DST
	INSTANCE_OUTOF_DST,			//Instance out of duration when DST is applied
	DST_DISABLED				//DST is disable
}DST_SCALE_e;

//#################################################################################################
// @EXTERN VARIABLES
//#################################################################################################
extern INT32 DstPeriodInSec;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitDstParam(struct tm * currTimePtr, DST_CONFIG_t * dstConfigPtr);
//-------------------------------------------------------------------------------------------------
void UpdateDstFlag(BOOL initDst);
//-------------------------------------------------------------------------------------------------
DST_SCALE_e CheckTimeInDstScale(time_t newTime,time_t *dstTimePeriod);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif /* DST_H_ */
