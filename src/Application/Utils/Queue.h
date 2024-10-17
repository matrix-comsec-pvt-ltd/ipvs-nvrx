#ifndef QUEUE_H_
#define QUEUE_H_
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file   Queue.h
@brief  This file provides API to maintain queue.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <pthread.h>

/* Application Includes */
#include "MxTypedef.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define QUEUE_HANDLE    VOIDPTR

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
	Q_REMOVE_CURR,
	Q_REMOVE_ALL_BUT_CURR,
	Q_REMOVE_ALL,
	MAX_QUEUE_REMOVE

}Q_REMOVE_e;

typedef void (*QUEUE_FULL_CB)(VOIDPTR entry);

typedef struct
{
	UINT16			sizoOfMember;		// Shows sizeof member of a Queue
	UINT16			maxNoOfMembers;		// Maximum number of members in queue
    BOOL			syncRW;				// This will add a feature like waiting on adding of queue entry
	BOOL			overwriteOldest;
    QUEUE_FULL_CB	callback;			// NOTE: this call back is called holding internal mutex lock

}QUEUE_INIT_t;

#define QUEUE_PARAM_INIT(qInfo) qInfo.sizoOfMember = 0;	\
								qInfo.maxNoOfMembers = 0;	\
								qInfo.syncRW = FALSE;	\
								qInfo.overwriteOldest = FALSE; \
								qInfo.callback = NULL;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
QUEUE_HANDLE QueueCreate(QUEUE_INIT_t *qProperty);
//-------------------------------------------------------------------------------------------------
BOOL QueueAddEntry(QUEUE_HANDLE qHandle, VOIDPTR data);
//-------------------------------------------------------------------------------------------------
UINT8 QueueAddMultipleEntries(QUEUE_HANDLE qHandle, VOIDPTR *data, UINT8 noEntry);
//-------------------------------------------------------------------------------------------------
VOIDPTR QueueGetEntry(QUEUE_HANDLE qHandle);
//-------------------------------------------------------------------------------------------------
VOIDPTR QueueGetAndFreeEntry(QUEUE_HANDLE qHandle, BOOL waitForEver);
//-------------------------------------------------------------------------------------------------
VOIDPTR QueueGetNewEntryIfOverWritten(QUEUE_HANDLE qHandle);
//-------------------------------------------------------------------------------------------------
BOOL QueueRemoveEntry(QUEUE_HANDLE qHandle, Q_REMOVE_e removeType);
//-------------------------------------------------------------------------------------------------
BOOL QueueDestroy(QUEUE_HANDLE qHandle);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif
