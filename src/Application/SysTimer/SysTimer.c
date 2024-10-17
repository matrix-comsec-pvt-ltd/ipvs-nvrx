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
/* OS Includes */
#include <pthread.h>

/* Application Includes */
#include "SysTimer.h"
#include "DateTime.h"
#include "DebugLog.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define MAX_ONE_MIN_FUNC	(7)

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static UINT32 				sysTick;			// keeps count of system ticks @ 100 mSec
static pthread_rwlock_t 	sysTickMutex;

static SYS_TIMER_LIST_t     *sysTimerListHead;	// start node address of timer list
static SYS_TIMER_LIST_t     *sysTimerListTail;	// end node address of timer list

static pthread_mutex_t 		resourceMutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

static UINT8				oneMinFunCnt;
static ONE_MIN_NOTIFY_t		oneMinNotify[MAX_ONE_MIN_FUNC];

static SYS_TIMER_LIST_t     *nextNodePtr;			// next node in list after current node
static BOOL				 	sysTmrCallBackActiveF;	// indicates if call back process is active

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initializes all system timer related parameters. It needs to be called
 *          during start-up of the program.
 */
void InitSysTimer(void)
{
    UINT8 cnt;

	// reset system tick counter
	sysTick = 0;
	pthread_rwlock_init(&sysTickMutex, NULL);

	// reset all link list node pointers
	sysTimerListHead = NULL;
	sysTimerListTail = NULL;
	nextNodePtr = NULL;
	sysTmrCallBackActiveF = FALSE;
	oneMinFunCnt = 0;

	for(cnt = 0; cnt < MAX_ONE_MIN_FUNC; cnt++)
	{
		oneMinNotify[cnt].funcPtr = NULL;
		oneMinNotify[cnt].userData = 0;
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function traverses through timer lists and updates counts of the active timers.
 *          If any of the timers from the list expires, it calls the callback function registered
 *          with that timer. This function is called upon the receipt of SIG_ALRM at interval of 50mSec.
 * @note    This function needs to run in super-loop. so accuracy varies with the execution time of the loop.
 */
void RunSysTimer(void)
{
    static UINT8        prevMin = SEC_IN_ONE_MIN;
    SYS_TIMER_LIST_t    *currNodePtr;
	
	// set node pointer to beginning of list and set active call-back flag
	currNodePtr = sysTimerListHead;

	// lock the node, in order to prevent multiple simultaneous access to timer list.
    MUTEX_LOCK(resourceMutex);

	sysTmrCallBackActiveF = TRUE;

	// traverse through the list until end of the list is reached
	while(currNodePtr != NULL)
	{
        // Store next node pointer first since it can be deleted by callback function of current node.
		nextNodePtr = currNodePtr->next;

		// increment current count of timer, if it is greater than or equal to user count,
		// reset current count, invoke the call-back function and pass the data to it
        currNodePtr->sysTimer.currCount++;
        if(currNodePtr->sysTimer.currCount >= currNodePtr->sysTimer.timerInfo.count)
		{
			currNodePtr->sysTimer.currCount = 0;
            (*(currNodePtr->sysTimer.timerInfo.funcPtr)) (currNodePtr->sysTimer.timerInfo.data);
		}

		// point address to next node
		currNodePtr = nextNodePtr;
	}

	sysTmrCallBackActiveF = FALSE;

	// unlock the resource
    MUTEX_UNLOCK(resourceMutex);

	pthread_rwlock_wrlock(&sysTickMutex);
	sysTick++;
    if (sysTick % 10)
	{
        pthread_rwlock_unlock(&sysTickMutex);
        return;
    }
    pthread_rwlock_unlock(&sysTickMutex);

    struct tm localTime;
    if(SUCCESS != GetLocalTimeInBrokenTm(&localTime))
    {
        EPRINT(SYS_LOG, "failed to get local time in broken");
        return;
    }

    if (prevMin == localTime.tm_min)
    {
        return;
    }

    UINT8 cnt;
    prevMin = localTime.tm_min;
    for(cnt = 0; cnt < MAX_ONE_MIN_FUNC; cnt++)
    {
        if(oneMinNotify[cnt].funcPtr != NULL)
        {
            oneMinNotify[cnt].funcPtr(oneMinNotify[cnt].userData);
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function starts a new timer with specified timer count and register callback function
 *          by adding a node in timer list. On expiry of the timer, user supplied callback function
 *          will be invoked along with data supplied. Timer handle returned to caller is nothing but
 *          address of timer node in the list.
 * @param   timerInfo
 * @param   handle
 * @return  Returns SUCCESS on creation else return FAIL
 */
BOOL StartTimer(TIMER_INFO_t timerInfo, TIMER_HANDLE *handle)
{
    SYS_TIMER_LIST_t	*newNode;
	SYS_TIMER_LIST_t	**head;
	SYS_TIMER_LIST_t	**tail;

	// Make sure pointer where we return handle is not NULL
    if(handle == INVALID_TIMER_HANDLE)
	{
        return FAIL;
    }

    // get memory from the pool for new timer to be created, store its address in new node pointer
    newNode = (SYS_TIMER_LIST_t *)malloc(sizeof(SYS_TIMER_LIST_t));

    // if no memory is available, NULL the timer handle and return fail status
    if(newNode == NULL)
    {
        *handle = INVALID_TIMER_HANDLE;
        EPRINT(SYS_LOG, "fail to alloc memory");
        return FAIL;
    }

    // if memory is available, load user data to timer buffer
    newNode->sysTimer.currCount = 0;
    newNode->sysTimer.timerInfo.count = timerInfo.count;
    newNode->sysTimer.timerInfo.data = timerInfo.data;
    newNode->sysTimer.timerInfo.funcPtr = timerInfo.funcPtr;

    // initialize timer list pointers
    head = &sysTimerListHead;
    tail = &sysTimerListTail;

    // lock the resource to avoid simultaneous access
    MUTEX_LOCK(resourceMutex);

    // if this is a first entry in list, insert it at beginning otherwise insert it at the end of the list
    if(*head == INVALID_TIMER_HANDLE)
    {
        (*head) = newNode;
    }
    else
    {
        (*tail)->next = newNode;
    }

    newNode->next = NULL;
    newNode->prev = (*tail);
    (*tail) = newNode;

    // unlock the resource
    MUTEX_UNLOCK(resourceMutex);

    // store new node as timer handle
    *handle = newNode;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function marks the timer to be deleted from the list.
 * @param   handlePtr
 */
void DeleteTimer(TIMER_HANDLE *	handlePtr)
{
	SYS_TIMER_LIST_t * tempNodePtr;

	// check if the handle pointer is invalid or not
    if(handlePtr == INVALID_TIMER_HANDLE)
	{
        return;
    }

    // check if the timer handle is invalid or not
    if(*handlePtr == INVALID_TIMER_HANDLE)
    {
        return;
    }

    // lock the timer list prior to search operation
    MUTEX_LOCK(resourceMutex);

    // Start searching specified handle from head
    tempNodePtr = sysTimerListHead;

    // Traverse through the entire list
    while(tempNodePtr != NULL)
    {
        // if specified timer found then mark the delete flag, else point to next timer
        if(tempNodePtr != *handlePtr)
        {
            // fetch next timer
            tempNodePtr = tempNodePtr->next;
            continue;
        }

        // if head is deleted then update head
        if(tempNodePtr == sysTimerListHead)
        {
            sysTimerListHead = tempNodePtr->next;
            if(sysTimerListHead != NULL)
            {
                sysTimerListHead->prev = NULL;
            }
        }
        // if tail is deleted then update tail
        else if(tempNodePtr == sysTimerListTail)
        {
            sysTimerListTail = tempNodePtr->prev;
            sysTimerListTail->next = NULL;
        }
        else // deleting node other than head & tail
        {
            if(tempNodePtr->prev != NULL)
            {
                tempNodePtr->prev->next = tempNodePtr->next;
            }

            if(tempNodePtr->next != NULL)
            {
                tempNodePtr->next->prev = tempNodePtr->prev;
            }
        }

        // While timer task processes current node, it backs up for next node that is to be processed after the current node has been processed.
        // So, if we needs to delete next node of current node then we need to update next node.
        if((tempNodePtr == nextNodePtr) && (sysTmrCallBackActiveF == TRUE))
        {
            nextNodePtr = tempNodePtr->next;
        }

        free(tempNodePtr);
        tempNodePtr = NULL;
        *handlePtr = INVALID_TIMER_HANDLE;
        break;
    }

    // unlock timer list at end of search
    MUTEX_UNLOCK(resourceMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function reloads timer count, specified by handle.
 * @param   handle
 * @param   count
 * @return  Returns SUCCESS on reload else return FAIL
 */
BOOL ReloadTimer(TIMER_HANDLE handle, UINT32 count)
{
    SYS_TIMER_LIST_t *tempNodePtr;

	// check handle is not invalid, if it is then return with fail status
    if(handle == INVALID_TIMER_HANDLE)
	{
        return FAIL;
    }

    MUTEX_LOCK(resourceMutex);
    // traverse through the timer list from the beginning,
    tempNodePtr = sysTimerListHead;
    while(tempNodePtr != NULL)
    {
        // find out the node specified by handle
        if(tempNodePtr != handle)
        {
            // fetch next node
            tempNodePtr = tempNodePtr->next;
            continue;
        }

        // load reload count to timer; reset current count to zero and return success
        handle->sysTimer.timerInfo.count = count;
        handle->sysTimer.currCount = 0;
        MUTEX_UNLOCK(resourceMutex);
        return SUCCESS;
    }
    MUTEX_UNLOCK(resourceMutex);

	// if handle is invalid return with fail status
    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get Elapsed timer since started timer of that particluar handle.
 * @param   handle
 * @param   timeCnt
 * @return  SUCCESS/FAIL
 */
BOOL GetElapsedTime(TIMER_HANDLE handle, UINT32PTR timeCnt)
{
    SYS_TIMER_LIST_t *tempNodePtr;

	// check handle is not invalid, if it is then return with fail status
    if(handle == INVALID_TIMER_HANDLE)
	{
        return FAIL;
    }

    MUTEX_LOCK(resourceMutex);
    // traverse through the timer list from the beginning,
    tempNodePtr = sysTimerListHead;
    while(tempNodePtr != NULL)
    {
        // find out the node specified by handle
        if(tempNodePtr != handle)
        {
            // fetch next node
            tempNodePtr = tempNodePtr->next;
            continue;
        }

        // load reload count to timer; reset current count to zero and return success
        *timeCnt = handle->sysTimer.currCount;
        MUTEX_UNLOCK(resourceMutex);
        return SUCCESS;
    }
    MUTEX_UNLOCK(resourceMutex);

	// if handle is invalid return with fail status
    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get Remaining timer since started timer of that particluar handle.
 * @param   handle
 * @param   timeCnt
 * @return  SUCCESS/FAIL
 */
BOOL GetRemainingTime(TIMER_HANDLE handle, UINT32PTR timeCnt)
{
    SYS_TIMER_LIST_t *tempNodePtr;

	// check handle is not invalid, if it is then return with fail status
    if(handle == INVALID_TIMER_HANDLE)
	{
        return FAIL;
    }

    MUTEX_LOCK(resourceMutex);
    // traverse through the timer list from the beginning,
    tempNodePtr = sysTimerListHead;
    while(tempNodePtr != NULL)
    {
        // find out the node specified by handle
        if(tempNodePtr != handle)
        {
            // fetch next node
            tempNodePtr = tempNodePtr->next;
            continue;
        }

        if(handle->sysTimer.timerInfo.count >= handle->sysTimer.currCount)
        {
            // load reload count to timer; reset current count to zero and return success
            *timeCnt = (handle->sysTimer.timerInfo.count - handle->sysTimer.currCount);
            MUTEX_UNLOCK(resourceMutex);
            return SUCCESS;
        }
        else
        {
            *timeCnt = 0;
            MUTEX_UNLOCK(resourceMutex);
            return FAIL;
        }
    }
    MUTEX_UNLOCK(resourceMutex);

	// if handle is invalid return with fail status
    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function returns current value of system ticks.
 * @return  Return the count of system timer ticks
 */
UINT32 GetSysTick(void)
{
	pthread_rwlock_rdlock(&sysTickMutex);
    UINT32 tick = sysTick;
	pthread_rwlock_unlock(&sysTickMutex);
    return tick;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function returns the count of system ticks since the mark tick.
 * @param   markTick
 * @return  Return the count of system timer elapsed ticks
 */
UINT32 ElapsedTick(UINT32 markTick)
{
    UINT32 tick;

    // return the difference between current system tick and marked tick
	pthread_rwlock_rdlock(&sysTickMutex);
	if (sysTick < markTick)
	{
        tick = (sysTick + (0xFFFFFFFF - markTick));
	}
	else
	{
        tick = (sysTick - markTick);
	}
	pthread_rwlock_unlock(&sysTickMutex);
    return tick;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function registers one min function which is call @ every actual minute modified.
 * @param   oneMinFun
 * @return  SUCCESS/FAIL
 */
BOOL RegisterOnMinFun(ONE_MIN_NOTIFY_t *oneMinFun)
{
    if (oneMinFunCnt >= MAX_ONE_MIN_FUNC)
	{
        EPRINT(SYS_LOG, "space not available to register one min timer");
        return FAIL;
    }

    oneMinNotify[oneMinFunCnt].funcPtr = oneMinFun->funcPtr;
    oneMinNotify[oneMinFunCnt].userData = oneMinFun->userData;
    oneMinFunCnt++;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Takes amount of time for sleep in nano second
 * @param	ns count for sleep
 */
void SleepNanoSec(UINT64 ns)
{
    struct timespec req = {0, 0};

    if (ns >= NANO_SEC_PER_SEC)
    {
        req.tv_sec = (ns / NANO_SEC_PER_SEC);
        req.tv_nsec = (ns % NANO_SEC_PER_SEC);
    }
    else
    {
        req.tv_nsec = ns;
    }

    /* sleep until next tick */
    while ((-1 == nanosleep(&req, &req)) && (EINTR == errno))
    {
        continue;
    }
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
