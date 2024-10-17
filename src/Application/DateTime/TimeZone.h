#ifndef TIMEZONE_H_
#define TIMEZONE_H_
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		TimeZone.h
@brief      This module provides cyclic periodic timer functionality. Application can start, delete
            and reload timers. When starting a timer, application provides time period and registers
            function to be invoked on expiry of the timer. On expiry of timer, it is automatically
            reloaded with same timer count and hence it keeps on running until it is deleted. Timer
            can be reloaded to new count without deleting or restarting the timer. Also there are
            functions which can provide system ticks and the ticks elapsed from the specific point.
            Concept of this module is based on the ACS system timer.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "MxTypedef.h"
#include "ConfigComnDef.h"

//#################################################################################################
// @EXTERN VARIABLES
//#################################################################################################
extern const INT32      TimeZoneinSec[];
extern const UINT32     RegionalInfo[MAX_TIMEZONE][MAX_REG_INFO];
extern const CHARPTR    OnviftimeZoneNames[MAX_TIMEZONE][2];

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitAutoTimezoneSrch(void);
//-------------------------------------------------------------------------------------------------
void DeinitAutoTimezoneSrch(void);
//-------------------------------------------------------------------------------------------------
void GetTimezoneStr(CHAR *timezoneStr);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif /* TIMEZONE_H_ */
