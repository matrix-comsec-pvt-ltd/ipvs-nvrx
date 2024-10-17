//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file   Queue.c
@brief  This file provides API to maintain queue.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "Queue.h"
#include "Utils.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
// Check for Queue roll-over, If occurs, Because it's a circular Queue assign 0th index
#define UPDATE_RW_INDEX(qHandle, rwIndex)				\
qHandle->rwIndex++;										\
if (qHandle->rwIndex >= qHandle->maxNoOfMember)			\
{														\
	qHandle->rwIndex = 0;								\
}														\

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef struct
{
	UINT16			sizoOfMember;	// Shows sizeof member of a Queue
	UINT16			maxNoOfMember;	// Maximum number of members in queue
	UINT16			readIndx;		// Pointing to the Entry to be read
	UINT16			writeIndx;		// Pointing to the Entry to be read
	VOIDPTR			*entriesPtr;	// Array of Pointer to Entries
	QUEUE_FULL_CB	callback;
	BOOL			syncRW;
	BOOL			overwriteOldest;
	BOOL			overWriteOccured;
	pthread_mutex_t	queueLock;		// Mutex to Protect Queue
	pthread_cond_t	queueCond;		// Cond to Wait on Queue

}QUEUE_INFO_t;

//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   These functions create a queue for requested user. If Fails to create queue it will return NULL.
 * @param   qProperty
 * @return  Queue handle
 */
QUEUE_HANDLE QueueCreate(QUEUE_INIT_t *qProperty)
{
	QUEUE_INFO_t	*qHandle = NULL;
	UINT16			loop;

	// Allocate memory for storing Queue information
    if ((qProperty->maxNoOfMembers == 0) || (qProperty->sizoOfMember == 0))
	{
        return NULL;
    }

    qHandle = (QUEUE_INFO_t *) malloc(sizeof(QUEUE_INFO_t));
    if (qHandle == NULL)
    {
        return NULL;
    }

    // Fill Parameter as requested
    qHandle->sizoOfMember = qProperty->sizoOfMember;
    qHandle->maxNoOfMember = qProperty->maxNoOfMembers;
    qHandle->syncRW = qProperty->syncRW;
    qHandle->overwriteOldest = qProperty->overwriteOldest;
    qHandle->callback = qProperty->callback;
    qHandle->overWriteOccured = FALSE;

    // Allocate memory to store pointer of Entries
    qHandle->entriesPtr = (VOIDPTR *) malloc(qProperty->maxNoOfMembers * sizeof(VOIDPTR));

    // If Failed to allocate memory
    if (qHandle->entriesPtr == NULL)
    {
        // release memory allocated for storing Queue information
        free(qHandle);
        return NULL;
    }

    // Initialise Necessary Parameters
    qHandle->readIndx = 0;
    qHandle->writeIndx = 0;
    MUTEX_INIT(qHandle->queueLock, NULL);
    pthread_cond_init(&qHandle->queueCond, NULL);

    for(loop = 0; loop < qProperty->maxNoOfMembers; loop++)
    {
        // Make All Entries Invalid
        qHandle->entriesPtr[loop] = NULL;
    }

	return qHandle;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function add an entry into the requested queue. If the maximum number of queue
 *          members is exceeding than it will not add an entry and return FAIL.
 * @param   qHandle
 * @param   data
 * @return  SUCCESS/ FAIL
 */
BOOL QueueAddEntry(QUEUE_HANDLE qHandle, VOIDPTR data)
{
	QUEUE_INFO_t	*priv = (QUEUE_INFO_t *)qHandle;
	BOOL			overWritten = FALSE;

    if (priv == NULL)
	{
        return FAIL;
    }

    MUTEX_LOCK(priv->queueLock);

    // If A valid Entry exists free that entry
    if (priv->entriesPtr[priv->writeIndx] != NULL)
    {
        if (priv->overwriteOldest == FALSE)
        {
            MUTEX_UNLOCK(priv->queueLock);
            return FAIL;
        }

        if (priv->callback != NULL)
        {
            priv->callback(priv->entriesPtr[priv->writeIndx]);
        }

        free(priv->entriesPtr[priv->writeIndx]);
        priv->entriesPtr[priv->writeIndx] = malloc(priv->sizoOfMember);
        overWritten = TRUE;
    }
    else
    {
        priv->entriesPtr[priv->writeIndx] = malloc(priv->sizoOfMember);
    }

    if (priv->entriesPtr[priv->writeIndx] == NULL)
    {
        MUTEX_UNLOCK(priv->queueLock);
        return FAIL;
    }

    memcpy(priv->entriesPtr[priv->writeIndx], data, priv->sizoOfMember);
    UPDATE_RW_INDEX(priv, writeIndx);
    if (overWritten == TRUE)
    {
        UPDATE_RW_INDEX(priv, readIndx);
        priv->overWriteOccured = TRUE;
    }

    if (priv->syncRW == TRUE)
    {
        pthread_cond_signal(&priv->queueCond);
    }

    MUTEX_UNLOCK(priv->queueLock);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function add entries into the requested queue. returns no of entries added into queue.
 * @param   qHandle
 * @param   data - Pointer to entries
 * @param   noEntry - no of Entries to be added
 * @return  no of entries successfully added
 */
UINT8 QueueAddMultipleEntries(QUEUE_HANDLE qHandle, VOIDPTR *data, UINT8 noEntry)
{
	QUEUE_INFO_t	*priv = (QUEUE_INFO_t *)qHandle;
    UINT8			entryCnt = 0;
	UINT8 			loop;
	BOOL			overWritten = FALSE;

    if (priv == NULL)
	{
        return 0;
    }

    MUTEX_LOCK(priv->queueLock);

    // Loop through all given entry
    for (loop = 0; loop < noEntry; loop++)
    {
        // If A valid Entry exists free that entry
        if (priv->entriesPtr[priv->writeIndx] != NULL)
        {
            if (priv->overwriteOldest == FALSE)
            {
                break;
            }

            if (priv->callback != NULL)
            {
                priv->callback(priv->entriesPtr[priv->writeIndx]);
            }

            free(priv->entriesPtr[priv->writeIndx]);
            overWritten = TRUE;
        }

        priv->entriesPtr[priv->writeIndx] = malloc(priv->sizoOfMember);
        if (priv->entriesPtr[priv->writeIndx] == NULL)
        {
            // break on memory allocation failure
            break;
        }

        memcpy(priv->entriesPtr[priv->writeIndx], data[loop], priv->sizoOfMember);
        UPDATE_RW_INDEX(priv, writeIndx);
        if (overWritten == TRUE)
        {
            UPDATE_RW_INDEX(priv, readIndx);
            priv->overWriteOccured = TRUE;
        }

        // Increment counter to get no of entries added into queue
        entryCnt++;
    }

    if ((entryCnt > 0) && (priv->syncRW == TRUE))
    {
        pthread_cond_signal(&priv->queueCond);
    }

    MUTEX_UNLOCK(priv->queueLock);
    return entryCnt;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives a entry from queue and remove its entry from queue. this memory
 *          pointer must be freed after usage. If there doesn't exist any it will return NULL.
 * @param   qHandle
 * @return
 */
VOIDPTR QueueGetEntry(QUEUE_HANDLE qHandle)
{
	QUEUE_INFO_t	*priv = (QUEUE_INFO_t *)qHandle;
	VOIDPTR			retVal = NULL;

    if (priv == NULL)
	{
        return NULL;
    }

    MUTEX_LOCK(priv->queueLock);
    // Give Pointer to An Entry at read Index in Output
    retVal = priv->entriesPtr[priv->readIndx];
    MUTEX_UNLOCK(priv->queueLock);
	return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives a entry from queue and remove its entry from queue. this memory
 *          pointer must be freed after usage. If there doesn't exist any it will return NULL.
 * @param   qHandle
 * @param   waitForEver
 * @return
 */
VOIDPTR QueueGetAndFreeEntry(QUEUE_HANDLE qHandle, BOOL waitForEver)
{
	QUEUE_INFO_t	*priv = (QUEUE_INFO_t *)qHandle;
	VOIDPTR			retVal = NULL;

    if (priv == NULL)
	{
        return NULL;
    }

    MUTEX_LOCK(priv->queueLock);

    // Give Pointer to An Entry at read Index in Output
    retVal = priv->entriesPtr[priv->readIndx];
    if ((retVal == NULL) && (waitForEver == TRUE) && (priv->syncRW == TRUE))
    {
        pthread_cond_wait(&priv->queueCond, &priv->queueLock);
        retVal = priv->entriesPtr[priv->readIndx];
    }

    // Check if Queue is not empty
    if (retVal != NULL)
    {
        priv->entriesPtr[priv->readIndx] = NULL;
        UPDATE_RW_INDEX(priv, readIndx);
    }

    MUTEX_UNLOCK(priv->queueLock);
	return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives a entry from queue and remove its entry from queue. this memory
 *          pointer must be freed after usage. If there doesn't exist any it will return NULL.
 * @param   qHandle
 * @return
 */
VOIDPTR QueueGetNewEntryIfOverWritten(QUEUE_HANDLE qHandle)
{
	QUEUE_INFO_t	*priv = (QUEUE_INFO_t *)qHandle;
	VOIDPTR			retVal = NULL;

    if (priv == NULL)
	{
        return NULL;
    }

    MUTEX_LOCK(priv->queueLock);
    if (priv->overWriteOccured == TRUE)
    {
        priv->overWriteOccured = FALSE;

        // Give Pointer to An Entry at read Index in Output
        retVal = priv->entriesPtr[priv->readIndx];

        // Check if Queue is not empty
        if (retVal != NULL)
        {
            priv->entriesPtr[priv->readIndx] = NULL;
            UPDATE_RW_INDEX(priv, readIndx);
        }
    }
    MUTEX_UNLOCK(priv->queueLock);
	return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function removes entry/ entries from queue as requested.
 * @param   qHandle
 * @param   removeType
 * @return  SUCCESS/ FAIL
 */
BOOL QueueRemoveEntry(QUEUE_HANDLE qHandle, Q_REMOVE_e removeType)
{
	QUEUE_INFO_t	*priv = (QUEUE_INFO_t *)qHandle;
	UINT8			retVal = SUCCESS;
	UINT8 			loop;

    if ((priv == NULL) || (removeType >= MAX_QUEUE_REMOVE))
	{
        return FAIL;
    }

    MUTEX_LOCK(priv->queueLock);
    if (removeType == Q_REMOVE_CURR)
    {
        if (priv->entriesPtr[priv->readIndx] != NULL)
        {
            free(priv->entriesPtr[priv->readIndx]);
            priv->entriesPtr[priv->readIndx] = NULL;
            UPDATE_RW_INDEX(priv, readIndx);
        }
    }
    else if (removeType == Q_REMOVE_ALL_BUT_CURR)
    {
        for(loop = 0; loop < priv->maxNoOfMember; loop++)
        {
            // Don't remove entry at read index
            if (loop != priv->readIndx)
            {
                // Check If Valid Entry exists at this Index
                FREE_MEMORY(priv->entriesPtr[loop]);
            }
        }
    }
    else if (removeType == Q_REMOVE_ALL)
    {
        for(loop = 0; loop < priv->maxNoOfMember; loop++)
        {
            // Check If Valid Entry exists at this Index
            FREE_MEMORY(priv->entriesPtr[loop]);
        }
    }

    MUTEX_UNLOCK(priv->queueLock);
	return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function destroy the queue. After this, Any Queue Operation with given Handle will
 *          lead to System- Crash.
 * @param   qHandle
 * @return
 */
BOOL QueueDestroy(QUEUE_HANDLE qHandle)
{
    QUEUE_INFO_t *priv = (QUEUE_INFO_t *)qHandle;

    if (priv == NULL)
	{
        return FAIL;
    }

    // Free Allocated bytes to store Pointers
    QueueRemoveEntry(priv, Q_REMOVE_ALL);
    pthread_mutex_destroy(&priv->queueLock);
    free(priv->entriesPtr);
    free(qHandle);
    return SUCCESS;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
