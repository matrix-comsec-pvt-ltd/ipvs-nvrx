#if !defined SYS_TIMER_H
#define SYS_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file       SysTimer.h
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

//#################################################################################################
// @DEFINES
//#################################################################################################
#define INVALID_TIMER_HANDLE				NULL		// invalid timer handle
#define TIMER_RESOLUTION_MINIMUM_MSEC		100			//Minimum resolution in msec
#define CONVERT_SEC_TO_TIMER_COUNT(sec)		((sec * MILLI_SEC_PER_SEC) / TIMER_RESOLUTION_MINIMUM_MSEC)     // Calculation for 1 second
#define CONVERT_MSEC_TO_TIMER_COUNT(msec)	(((msec / TIMER_RESOLUTION_MINIMUM_MSEC) == 0) ? 1 : (msec / TIMER_RESOLUTION_MINIMUM_MSEC))

//#################################################################################################
// @DATA TYPES
//#################################################################################################
//	This structure capsulates parameters required to start a new timer.
typedef struct
{
	UINT32		count;						// timer count (100ms resolution)
    void		(*funcPtr)(UINT32 data);	// pointer to call-back function
	UINT32		data;						// input data to pass back with callback function

}TIMER_INFO_t;

typedef struct
{
    TIMER_INFO_t    timerInfo;              // timer information structure
    UINT32          currCount;              // current count of timer

}SYS_TIMER_INFO_t;

typedef struct TIMER_LIST_NODE
{
    struct TIMER_LIST_NODE	*next;			// next node pointer
    struct TIMER_LIST_NODE	*prev;			// previous node pointer
    SYS_TIMER_INFO_t		sysTimer;		// system timer structure

}SYS_TIMER_LIST_t;

typedef struct
{
    BOOL    (*funcPtr)(UINT32 userData);
    UINT32  userData;

}ONE_MIN_NOTIFY_t;

typedef SYS_TIMER_LIST_t*   TIMER_HANDLE;   // timer handle

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitSysTimer(void);
//-------------------------------------------------------------------------------------------------
void RunSysTimer(void);
//-------------------------------------------------------------------------------------------------
BOOL StartTimer(TIMER_INFO_t timerInfo, TIMER_HANDLE *handle);
//-------------------------------------------------------------------------------------------------
void DeleteTimer(TIMER_HANDLE *handle);
//-------------------------------------------------------------------------------------------------
BOOL ReloadTimer(TIMER_HANDLE handle, UINT32 count);
//-------------------------------------------------------------------------------------------------
BOOL GetElapsedTime(TIMER_HANDLE handle, UINT32PTR timeCnt);
//-------------------------------------------------------------------------------------------------
BOOL GetRemainingTime(TIMER_HANDLE handle, UINT32PTR timeCnt);
//-------------------------------------------------------------------------------------------------
UINT32 GetSysTick(void);
//-------------------------------------------------------------------------------------------------
UINT32 ElapsedTick(UINT32 markTick);
//-------------------------------------------------------------------------------------------------
BOOL RegisterOnMinFun(ONE_MIN_NOTIFY_t * oneMinFun);
//-------------------------------------------------------------------------------------------------
void SleepNanoSec(UINT64 ns);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#ifdef __cplusplus
}
#endif
#endif  // SYS_TIMER_H
