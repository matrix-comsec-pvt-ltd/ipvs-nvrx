//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		SmsNotify.c
@brief      This module Provides APIs to Send / Test SMS Service. It uses "Queue Module" to manage
            SMS Queue. It reads SMS Configuration  using API of Configuration Module and based on
            "MODE" ( HTTP / BROADBAND) selected by User, It gives particular Task to SMSonHttp Module.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "SmsNotify.h"
#include "ConfigApi.h"
#include "DebugLog.h"
#include "EventLogger.h"
#include "Queue.h"
#include "Utils.h"
#include "SMSonHttp.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define SMS_QUEUE_SIZE				10
#define SMS_CONN_REFRESH_RATE		3
#define MAX_SMS_RETRY_CNT			3	// Number of times to retry sending message
#define TEST_MESSAGE				"This is a test message sent by the device"
#define SMS_NOTIFY_THREAD_STACK_SZ  (2*MEGA_BYTE)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef struct
{
	BOOL				status;
	NW_CMD_REPLY_CB		callback;
	INT32				connFd;
	pthread_mutex_t		mutex;
	SMS_PARAMTER_t 		smsParam;
	SMS_CONFIG_t		smsConfig;
}TEST_SMS_t;

typedef struct
{
	NET_CMD_STATUS_e	status;
	pthread_cond_t		cond;
	pthread_mutex_t		mutex;
}HTTP_SMS_PARAM_t;

typedef struct
{
	BOOL				sendSmsF;
	BOOL				terminateThread;
	pthread_t			smsThreadId;
	pthread_cond_t		taskCond;
	pthread_mutex_t		taskMutex;
	QUEUE_HANDLE		qHandle;
	TEST_SMS_t			testSMSParam;
	HTTP_SMS_PARAM_t	httpSMSParam;
}SMS_NOTIFY_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static SMS_NOTIFY_t		notifyParam;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void	smsOnHttpCb(NET_CMD_STATUS_e smsStatus);
//-------------------------------------------------------------------------------------------------
static VOIDPTR smsNotifyThread(VOIDPTR data);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   Initialise SMS notification and creates a thread for sending SMS. Also initialise its
 *          local variables to known states.
 */
void InitSmsNotification(void)
{
    QUEUE_INIT_t qProperty;

	InitSMSonHttp();

	// Notify Module Parameters
	QUEUE_PARAM_INIT(qProperty);
	qProperty.maxNoOfMembers = SMS_QUEUE_SIZE;
	qProperty.sizoOfMember = sizeof(SMS_PARAMTER_t);
	notifyParam.qHandle = QueueCreate(&qProperty);

    MUTEX_INIT(notifyParam.taskMutex, NULL);
    MUTEX_LOCK(notifyParam.taskMutex);
    notifyParam.terminateThread = TRUE;
	notifyParam.sendSmsF = FREE;
    pthread_cond_init(&notifyParam.taskCond, NULL);
    MUTEX_UNLOCK(notifyParam.taskMutex);

    // Test SMS Parameter
    MUTEX_INIT(notifyParam.testSMSParam.mutex, NULL);
    MUTEX_LOCK(notifyParam.testSMSParam.mutex);
	notifyParam.testSMSParam.status = FREE;
    MUTEX_UNLOCK(notifyParam.testSMSParam.mutex);
	notifyParam.testSMSParam.callback = NULL;
	notifyParam.testSMSParam.connFd = INVALID_CONNECTION;

	// HTTP SMS Parameters
	pthread_cond_init(&notifyParam.httpSMSParam.cond, NULL);
    MUTEX_INIT(notifyParam.httpSMSParam.mutex, NULL);

#if !defined(OEM_JCI)
    if (FALSE == Utils_CreateThread(&notifyParam.smsThreadId, smsNotifyThread, NULL, JOINABLE_THREAD, SMS_NOTIFY_THREAD_STACK_SZ))
    {
        QueueDestroy(notifyParam.qHandle);
        notifyParam.qHandle = NULL;
        MUTEX_LOCK(notifyParam.testSMSParam.mutex);
        notifyParam.testSMSParam.status = BUSY;
        MUTEX_UNLOCK(notifyParam.testSMSParam.mutex);
        EPRINT(SMS_NOTIFY, "fail to create sms notify thread");
    }
#endif
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function accepts SMS and add it in Queue of this Module. It Also signals Thread to
 *          Notify for New Task.
 * @param   smsParam
 * @return  SUCCESS/FAIL
 */
BOOL SendSmsNotification(SMS_PARAMTER_t * smsParam)
{
	SMS_PARAMTER_t	tmpSmsParam[2]; // to store/ manipulate message temporarily
	VOIDPTR			entryPtr[2];
	UINT8 			noEntryAdded;

    // Check if thread running
    if (notifyParam.terminateThread == TRUE)
    {
        return FAIL;
    }

	// Check for bad or NULL pointer
    if (smsParam == NULL)
	{
        return FAIL;
    }

    // It must have at least one mobile number & a non-null Message
    if (((smsParam->mobileNumber1[0] == '\0') && (smsParam->mobileNumber2[0] == '\0')) || (smsParam->message[0] == '\0'))
    {
        return FAIL;
    }

    tmpSmsParam[0] = *smsParam;

    // Check if there are two recipients in Request
    if ((tmpSmsParam[0].mobileNumber1[0] != '\0') && (tmpSmsParam[0].mobileNumber2[0] != '\0'))
    {
        // Make 2nd recipient invalid from 1st request
        tmpSmsParam[0].mobileNumber2[0] = '\0';

        // duplicate request
        tmpSmsParam[1] = *smsParam;

        // copy 2nd recipient to 1st recipient
        snprintf(tmpSmsParam[1].mobileNumber1, MAX_MOBILE_NUMBER_WIDTH, "%s", tmpSmsParam[1].mobileNumber2);

        // Make 2nd recipient invalid
        tmpSmsParam[1].mobileNumber2[0] = '\0';

        // Assign Pointer to Entries
        entryPtr[0] = &tmpSmsParam[0];
        entryPtr[1] = &tmpSmsParam[1];

        // Add this entries into queue
        noEntryAdded = QueueAddMultipleEntries(notifyParam.qHandle, (VOIDPTR *)entryPtr, 2);

        // None of entry added
        if (noEntryAdded == 0)
        {
            // Write two respective failure event
            WriteEvent(LOG_NETWORK_EVENT, LOG_SMS_NOTIFICATION, tmpSmsParam[0].mobileNumber1, NULL, EVENT_FAIL);
            WriteEvent(LOG_NETWORK_EVENT, LOG_SMS_NOTIFICATION, tmpSmsParam[1].mobileNumber1, NULL, EVENT_FAIL);
            return FAIL;
        }

        // One Entry Added
        if (noEntryAdded == 1)
        {
            // As Failed to Add 2nd, write respective failure event, but one added successfully. so reply with success
            WriteEvent(LOG_NETWORK_EVENT, LOG_SMS_NOTIFICATION, tmpSmsParam[1].mobileNumber1, NULL, EVENT_FAIL);
        }
    }
    else
    {
        // If mobile number is added in as 2nd recipient shift it to 1st recipient and make 2nd invalid
        if ((tmpSmsParam[0].mobileNumber1[0] == '\0') && (tmpSmsParam[0].mobileNumber2[0] != '\0'))
        {
            snprintf(tmpSmsParam[0].mobileNumber1, MAX_MOBILE_NUMBER_WIDTH, "%s", tmpSmsParam[0].mobileNumber2);
            tmpSmsParam[0].mobileNumber2[0] = '\0';
        }

        // Add entry into queue
        if (QueueAddEntry(notifyParam.qHandle, (VOIDPTR)(&tmpSmsParam[0])) == FAIL)
        {
            // Write Failure Event
            WriteEvent(LOG_NETWORK_EVENT, LOG_SMS_NOTIFICATION, tmpSmsParam[0].mobileNumber1, NULL, EVENT_FAIL);
            return FAIL;
        }
    }

    // Signal Thread For a new Task
    MUTEX_LOCK(notifyParam.taskMutex);
    notifyParam.sendSmsF = BUSY;
    pthread_cond_signal(&notifyParam.taskCond);
    MUTEX_UNLOCK(notifyParam.taskMutex);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function adds a test SMS, taking care that only one Test Message is being send at
 *          a time. It signals Thread for a new Task.
 * @param   smsDataPtr
 * @param   deviceNumber
 * @param   callBack
 * @param   connFd
 * @return  Status
 */
NET_CMD_STATUS_e TestSmsNotification(SMS_CONFIG_t *smsDataPtr, CHARPTR deviceNumber, NW_CMD_REPLY_CB callBack, INT32 connFd)
{
    // Check if thread running
    if (notifyParam.terminateThread == TRUE)
    {
        return CMD_PROCESS_ERROR;
    }

	// Check for bad or NULL pointer
    if ((deviceNumber == NULL) || (callBack == NULL) || (connFd == INVALID_CONNECTION))
	{
        return CMD_PROCESS_ERROR;
    }

    if(deviceNumber[0] == '\0')
    {
        return CMD_INVALID_FIELD_VALUE;
    }

    MUTEX_LOCK(notifyParam.testSMSParam.mutex);
    if (notifyParam.testSMSParam.status == BUSY)
    {
        MUTEX_UNLOCK(notifyParam.testSMSParam.mutex);
        return CMD_TESTING_ON;
    }
    notifyParam.testSMSParam.status = BUSY;
    MUTEX_UNLOCK(notifyParam.testSMSParam.mutex);

    memcpy(&notifyParam.testSMSParam.smsConfig, smsDataPtr, sizeof(SMS_CONFIG_t));
    snprintf(notifyParam.testSMSParam.smsParam.mobileNumber1, MAX_MOBILE_NUMBER_WIDTH, "%s", deviceNumber);
    notifyParam.testSMSParam.smsParam.mobileNumber2[0] = '\0';
    snprintf(notifyParam.testSMSParam.smsParam.message, MAX_SMS_WIDTH, TEST_MESSAGE);
    notifyParam.testSMSParam.callback = callBack;
    notifyParam.testSMSParam.connFd = connFd;

    // Signal Thread For a new Task
    MUTEX_LOCK(notifyParam.taskMutex);
    notifyParam.sendSmsF = BUSY;
    pthread_cond_signal(&notifyParam.taskCond);
    MUTEX_UNLOCK(notifyParam.taskMutex);
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function frees up all resources used by this module and terminates thread.
 */
void DeInitSmsNotification(void)
{
	// Flush All SMS from Queue except current Task
	QueueRemoveEntry(notifyParam.qHandle, Q_REMOVE_ALL_BUT_CURR);

	// Set termination flag and signal thread to EXIT
    MUTEX_LOCK(notifyParam.taskMutex);
	notifyParam.terminateThread = TRUE;
	pthread_cond_signal(&notifyParam.taskCond);
    MUTEX_UNLOCK(notifyParam.taskMutex);

#if !defined(OEM_JCI)
	pthread_join(notifyParam.smsThreadId, NULL);
#endif

    // Set Test SMS Status BUSY so That No One Can Add New Test SMS
    MUTEX_LOCK(notifyParam.testSMSParam.mutex);
	notifyParam.testSMSParam.status = BUSY;
    MUTEX_UNLOCK(notifyParam.testSMSParam.mutex);
    DPRINT(SYS_LOG, "sms notify de-initialize");
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is a callback to SMSonHTTP Module. It will store SMS status and Signal Thread.
 * @param   smsStatus
 */
static void	smsOnHttpCb(NET_CMD_STATUS_e smsStatus)
{
    MUTEX_LOCK(notifyParam.httpSMSParam.mutex);
	notifyParam.httpSMSParam.status = smsStatus;
	// Signal Thread For Status of Message
	pthread_cond_signal(&notifyParam.httpSMSParam.cond);
    MUTEX_UNLOCK(notifyParam.httpSMSParam.mutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will be waiting for A signal when there's no SMS to be sent. Whenever thread
 *          wakes-up it check for Test SMS or an SMS from Queue. Based on Configuration of SMS it will
 *          give this SMS to either SMSonHttp Module. It will reflect back status of SMS if Necessary
 *          and executes next Task.
 * @param   data
 * @return
 */
static VOIDPTR smsNotifyThread(VOIDPTR data)
{
	BOOL				sendingTestSms;
	NET_CMD_STATUS_e	smsStatus;
    SMS_PARAMTER_t		*pSms;
	SMS_CONFIG_t		smsConfig;
	UINT8				retryCnt = 0;

    THREAD_START("SMS_NOTIFY");
    MUTEX_LOCK(notifyParam.taskMutex);
    notifyParam.terminateThread = FALSE;
	while(notifyParam.terminateThread == FALSE)
	{
		if(notifyParam.sendSmsF == FREE)
		{
			pthread_cond_wait(&notifyParam.taskCond, &notifyParam.taskMutex);
		}
		notifyParam.sendSmsF = PENDING;
        MUTEX_UNLOCK(notifyParam.taskMutex);

        MUTEX_LOCK(notifyParam.testSMSParam.mutex);
		if(notifyParam.testSMSParam.status == BUSY)
		{
            MUTEX_UNLOCK(notifyParam.testSMSParam.mutex);
			sendingTestSms = TRUE;
            pSms = &notifyParam.testSMSParam.smsParam;
            memcpy(&smsConfig, &notifyParam.testSMSParam.smsConfig, sizeof(SMS_CONFIG_t));
		}
		else
		{
            MUTEX_UNLOCK(notifyParam.testSMSParam.mutex);
			sendingTestSms = FALSE;

			// Get Entry From Queue
			pSms = (SMS_PARAMTER_t *)QueueGetEntry(notifyParam.qHandle);
			if(pSms == NULL)
			{
                MUTEX_LOCK(notifyParam.taskMutex);
				if(notifyParam.sendSmsF == PENDING)
				{
					notifyParam.sendSmsF = FREE;
				}
				continue;
			}

			ReadSmsConfig(&smsConfig);
		}

        smsStatus = SendSMSonHttp(pSms, &smsConfig, smsOnHttpCb);
        if(smsStatus == CMD_SUCCESS)
        {
            MUTEX_LOCK(notifyParam.httpSMSParam.mutex);
            // Wait For Status of Message
            pthread_cond_wait(&notifyParam.httpSMSParam.cond, &notifyParam.httpSMSParam.mutex);
            smsStatus = notifyParam.httpSMSParam.status;
            MUTEX_UNLOCK(notifyParam.httpSMSParam.mutex);
        }

		if(sendingTestSms == TRUE)
		{
			if(notifyParam.testSMSParam.callback != NULL)
			{
                notifyParam.testSMSParam.callback(smsStatus, notifyParam.testSMSParam.connFd, TRUE);
				notifyParam.testSMSParam.callback = NULL;
				notifyParam.testSMSParam.connFd = INVALID_CONNECTION;
			}

            MUTEX_LOCK(notifyParam.testSMSParam.mutex);
			notifyParam.testSMSParam.status = FREE;
            MUTEX_UNLOCK(notifyParam.testSMSParam.mutex);
		}
		else
		{
			if (smsStatus != CMD_SUCCESS)
			{
				// Check for any known errors, don't retry in this case
                if ((smsStatus == CMD_INVALID_MOBILE_NO) || (smsStatus == CMD_CRED_INVALID)
                        || (smsStatus == CMD_SMS_ACC_EXPIRED) || (smsStatus == CMD_SMS_ACC_INSUFF_CREDITS))
				{
                    // Set retryCnt to Default and Discard Message
					retryCnt = 0;
                    WriteEvent(LOG_NETWORK_EVENT, LOG_SMS_NOTIFICATION, pSms->mobileNumber1, NULL, EVENT_FAIL);
					QueueRemoveEntry(notifyParam.qHandle, Q_REMOVE_CURR);
                    EPRINT(SMS_NOTIFY, "discarding message without retry: [smsStatus=%d]", smsStatus);
				}
				else
				{
					// Retry After SMS_CONN_REFRESH_RATE Time
					sleep(SMS_CONN_REFRESH_RATE);
					retryCnt++;
					if (retryCnt > MAX_SMS_RETRY_CNT)
					{
                        EPRINT(SMS_NOTIFY, "discarding message after multiple retry: [retry=%d]", MAX_SMS_RETRY_CNT);
                        WriteEvent(LOG_NETWORK_EVENT, LOG_SMS_NOTIFICATION, pSms->mobileNumber1, NULL, EVENT_FAIL);
						retryCnt = 0;
						QueueRemoveEntry(notifyParam.qHandle, Q_REMOVE_CURR);
					}
				}
			}
			else
			{
				retryCnt = 0;
				QueueRemoveEntry(notifyParam.qHandle, Q_REMOVE_CURR);
			}
		}
        MUTEX_LOCK(notifyParam.taskMutex);
	}
    MUTEX_UNLOCK(notifyParam.taskMutex);

	// Destroy Queue So that No One can add New Entry in Queue
	QueueDestroy(notifyParam.qHandle);
	notifyParam.qHandle = NULL;
    pthread_exit(NULL);
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
