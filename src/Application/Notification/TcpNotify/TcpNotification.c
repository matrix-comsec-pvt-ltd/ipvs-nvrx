//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		TcpNotification.h
@brief		When application request a new message to be sent, this module will queue it in a table.
            Actual process of sending mail is done in a separate thread which is blocked for new
            entry to be added in the table. Whenever a new message is added to the table, TCP thread
            is signaled to process it. TCP thread will wake up on the signal and process all pending
            messages in the table. While sending message from the queue, connectivity to the server
            is checked. If it cannot connect to the server, message is left in the queue and retried
            after pre-defined time. Test messages are sent on its own separate thread and runs only
            when there is test message to send.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "TcpNotification.h"
#include "SysTimer.h"
#include "ConfigApi.h"
#include "DateTime.h"
#include "DebugLog.h"
#include "Utils.h"
#include "EventLogger.h"
#include "NetworkManager.h"
#include "Queue.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
// Maximum number of message stores in queue.
#define TCP_MAX_QUEUE_SIZE			20

// Time define for re connect with tcp server
#define TCP_CONN_REFRESH_RATE		30		//Sec

// Maximum time to received data from tcp servers
#define MAX_TIME_TO_SEND_RECV		10
#define	TCP_RECV_BUF_LENGTH			64
#define VALID_RESPONSE				'0'
#define MESSAGE_FORMAT_SIZE			100
#define TOTAL_TCP_MESSAGE_SIZE 		(MAX_TCP_MESSAGE_WIDTH + MESSAGE_FORMAT_SIZE)

#define TCP_SEND_CMD 				"SET_MSG"
#define TCP_RPLY_CMD 				"RPL_MSG"

#define TCP_NOTIFY_TEST_THREAD_STACK_SZ (1*MEGA_BYTE)
#define TCPNOTIFY_THREAD_STACK_SZ       (1*MEGA_BYTE)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef struct
{
	BOOL					testMsgStatus;
	NW_CMD_REPLY_CB			callback;
	INT32					connFd;
	pthread_mutex_t 		testTcpMutex;
	CHAR 					testMessage[MAX_TCP_MESSAGE_WIDTH + MESSAGE_FORMAT_SIZE];
	TCP_NOTIFY_CONFIG_t 	tcpNotifyCfg;
}TEST_TCP_t;

//#################################################################################################
// @PROTOTYPE
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static VOIDPTR tcpNotifyThread(VOIDPTR pthreadParam);
//-------------------------------------------------------------------------------------------------
static VOIDPTR tcpTestNotifyThread(VOIDPTR pData);
//-------------------------------------------------------------------------------------------------
static void tcpTimerCallback(UINT32 data);
//-------------------------------------------------------------------------------------------------
static BOOL sendTcpMessage(TCP_NOTIFY_CONFIG_t * tcpConfig, CHARPTR message);
//-------------------------------------------------------------------------------------------------
static BOOL tcpValidResponse(CHARPTR message);
//-------------------------------------------------------------------------------------------------
static void getTcpMessageSendString(CHARPTR inString, CHARPTR outString);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
// This mutex used by tcp thread for blocking critical section of shared memory.
static pthread_mutex_t  tcpMutex;

// This signal used to wake up tcp thread when new request for an message was added.
static pthread_cond_t   tcpCondSignal;

// Thread attribute for tcp notification thread
static pthread_t        tcpThread;

// System timer handle used for retry timer.
static TIMER_HANDLE     tcpTimerHandle;

// CloseSocketd tcp notification
static BOOL             closeSocketTcpNotify;
static BOOL             sendNewMessageF;
static TEST_TCP_t       testTcp;
static const CHARPTR    testTcpString = "This is a test message sent by the device";

//For queue to carry tcp message
static QUEUE_HANDLE     tcpQHandle;

//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   TCP notification parameters and creates thread for sending a message. Also initialize
 *          local variables to known state.
 * @return  SUCCESS/FAIL
 */
 void InitTcpNotification(void)
{
    QUEUE_INIT_t        tcpQInfo;
    TCP_NOTIFY_CONFIG_t tcpNotifyConfig;

    ReadTcpNotifyConfig(&tcpNotifyConfig);

    /* start tcp notification thread only if enabled in configuration */
    if (tcpNotifyConfig.tcpNotify == DISABLE)
    {
        return;
    }

	tcpTimerHandle = INVALID_TIMER_HANDLE;
    closeSocketTcpNotify = FALSE;
    sendNewMessageF = FALSE;
    testTcp.testMsgStatus = FREE;
    testTcp.connFd = NILL;
    testTcp.callback = NULL;
    MUTEX_INIT(testTcp.testTcpMutex, NULL);
    MUTEX_INIT(tcpMutex, NULL);
    pthread_cond_init(&tcpCondSignal, NULL);

#if !defined(OEM_JCI)
	QUEUE_PARAM_INIT(tcpQInfo);
	tcpQInfo.sizoOfMember 	= TOTAL_TCP_MESSAGE_SIZE;
	tcpQInfo.maxNoOfMembers = TCP_MAX_QUEUE_SIZE + MESSAGE_FORMAT_SIZE;
	tcpQHandle = QueueCreate(&tcpQInfo);

	// Create Tcp notification thread and check thread was created properly
    Utils_CreateThread(&tcpThread, tcpNotifyThread, NULL, JOINABLE_THREAD, TCPNOTIFY_THREAD_STACK_SZ);
#endif
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This functions frees up resources used by this module and if thread is sleep state the
 *          wake up the thread and sends terminate signals to thread.
 */
void DeInitTcpNotification(void)
{
	// Terminate tcp notification thread
    MUTEX_LOCK(tcpMutex);
	closeSocketTcpNotify = TRUE;
	pthread_cond_signal(&tcpCondSignal);
    MUTEX_UNLOCK(tcpMutex);
#if !defined(OEM_JCI)
	pthread_join(tcpThread,NULL);
#endif
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   InitTcpNotiication must call before this function.
 * @param   newCopy
 * @param   oldCopy
 */
void TcpConfigUpdate(TCP_NOTIFY_CONFIG_t newCopy, TCP_NOTIFY_CONFIG_t *oldCopy)
{
    if (oldCopy->tcpNotify == newCopy.tcpNotify)
    {
        return;
    }

    if(newCopy.tcpNotify == ENABLE)
    {
        InitTcpNotification();
    }
    else
    {
        DeInitTcpNotification();
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function accepts a message request from user. This function adds message request
 *          into queue. If queue was full then oldest message was discarded from queue and requested
 *          message was inserted. The queue was build up based on FIFO configuration.
 * @param   pMessage
 * @return
 */
BOOL SendTcpNotification(CHARPTR pMessage)
{
#if !defined(OEM_JCI)
	CHAR 				tcpMessage[TOTAL_TCP_MESSAGE_SIZE];
	TCP_NOTIFY_CONFIG_t tcpCfgNotify;

	// Check for bad or NULL pointer
    if((pMessage == NULL) || (pMessage[0] == '\0'))
	{
        return FAIL;
    }

    //Read Tcp/General configuration
    ReadTcpNotifyConfig(&tcpCfgNotify);
    if(tcpCfgNotify.tcpNotify == DISABLE)
    {
        return FAIL;
    }

    getTcpMessageSendString(pMessage, tcpMessage);
    QueueAddEntry(tcpQHandle, (VOIDPTR)tcpMessage);

    // Inform Tcp notification, new message was inserted so it wakes up and processes it.
    // Signal new message in list condition to client thread
    MUTEX_LOCK(tcpMutex);
    sendNewMessageF = TRUE;
    pthread_cond_signal(&tcpCondSignal);
    MUTEX_UNLOCK(tcpMutex);
#endif
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function accepts a test message request from user and creates a thread to send it immediately.
 * @param   tcpDataPtr
 * @param   commandCallback
 * @param   clientFd
 * @return
 */
NET_CMD_STATUS_e SendTestTcpNotify(TCP_NOTIFY_CONFIG_t * tcpDataPtr, NW_CMD_REPLY_CB commandCallback, INT32 clientFd)
{
#if !defined(OEM_JCI)
    // If we have already accepted request previously and still pending to execute, don’t accept new request.
    MUTEX_LOCK(testTcp.testTcpMutex);
    if(testTcp.testMsgStatus == BUSY)
    {
        MUTEX_UNLOCK(testTcp.testTcpMutex);
        return CMD_TESTING_ON;
    }
    testTcp.testMsgStatus = BUSY;
    MUTEX_UNLOCK(testTcp.testTcpMutex);

    getTcpMessageSendString(testTcpString, testTcp.testMessage);
    testTcp.callback = commandCallback;
    testTcp.connFd = clientFd;
    memcpy(&testTcp.tcpNotifyCfg, tcpDataPtr, sizeof(TCP_NOTIFY_CONFIG_t));

    // Create a thread for sending test message
    if (FALSE == Utils_CreateThread(NULL, tcpTestNotifyThread, NULL, DETACHED_THREAD, TCP_NOTIFY_TEST_THREAD_STACK_SZ))
    {
        EPRINT(TCP_NOTIFY, "fail to create tcp test thread");
        MUTEX_LOCK(testTcp.testTcpMutex);
        testTcp.testMsgStatus = FREE;
        MUTEX_UNLOCK(testTcp.testTcpMutex);
        return CMD_RESOURCE_LIMIT;
    }

    return CMD_SUCCESS;
#else
    return CMD_PROCESS_ERROR;
#endif
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is entry function of TCP notification thread. It sends message that was queued. After
 *          successful sending a message, that message is removed from list. If a message fails to send
 *          because of connection error to server, then sending further message was stop till connection
 *          was not establish. For any other failures, this message is discarded from queue. The thread
 *          waits for reply from server within certain time periods. If response not received from server
 *          then declared this message sending fail. This thread is blocked waiting for either new mail
 *          placed in its queue or connection retry time expired. The first inserted or oldest message
 *          was sent first(based on FIFO)
 * @param   pthreadParam
 * @return
 */
static VOIDPTR tcpNotifyThread(VOIDPTR pthreadParam)
{
    BOOL                    waitNewMsgF = TRUE;
	CHARPTR             	tcpQMessage;
	TIMER_INFO_t			tcpTimerInfo;
	TCP_NOTIFY_CONFIG_t 	tcpNotifyCfg;

    THREAD_START("TCP_NOTIFY");

    while(TRUE)
	{
        /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
        MUTEX_LOCK(tcpMutex);
        if (TRUE == closeSocketTcpNotify)
        {
            MUTEX_UNLOCK(tcpMutex);
            break;
        }

        /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
        if ((waitNewMsgF == TRUE) && (sendNewMessageF == FALSE))
		{
			pthread_cond_wait(&tcpCondSignal, &tcpMutex);
		}
        waitNewMsgF = sendNewMessageF = FALSE;
        MUTEX_UNLOCK(tcpMutex);

        /* If No Message Available */
        DeleteTimer(&tcpTimerHandle);
        tcpQMessage = QueueGetEntry(tcpQHandle);
		if(tcpQMessage == NULL)
		{
            waitNewMsgF = TRUE;
			continue;
		}

		ReadTcpNotifyConfig(&tcpNotifyCfg);
		if(sendTcpMessage(&tcpNotifyCfg, tcpQMessage) == FAIL)
		{
            WriteEvent(LOG_NETWORK_EVENT, LOG_TCP_NOTIFICATION, NULL, NULL, EVENT_FAIL);
			tcpTimerInfo.count = CONVERT_SEC_TO_TIMER_COUNT(TCP_CONN_REFRESH_RATE);
			tcpTimerInfo.funcPtr = &tcpTimerCallback;
			tcpTimerInfo.data = 0;
			StartTimer(tcpTimerInfo, &tcpTimerHandle);
            waitNewMsgF = TRUE;
			continue;
		}
		QueueRemoveEntry(tcpQHandle, Q_REMOVE_CURR);
    }

	//if timer is running then delete it
	DeleteTimer(&tcpTimerHandle);

	//Destroy the Queue
	QueueDestroy(tcpQHandle);
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is Test message entry function. It executes whenever test message is requested to
 *          be sent. It sends test message and stores status of sent message.
 * @param   pData
 * @return
 */
static VOIDPTR tcpTestNotifyThread(VOIDPTR pData)
{
    NET_CMD_STATUS_e tcpCmdStatus = CMD_TESTING_FAIL;

	if(sendTcpMessage(&testTcp.tcpNotifyCfg, testTcp.testMessage) == SUCCESS)
	{
		tcpCmdStatus = CMD_SUCCESS;
	}

	testTcp.callback(tcpCmdStatus, testTcp.connFd, TRUE);
    MUTEX_LOCK(testTcp.testTcpMutex);
	testTcp.testMsgStatus = FREE;
    MUTEX_UNLOCK(testTcp.testTcpMutex);
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It sends signal to tcpNotifyThread informing that its time to retry to establish connection.
 * @param   data
 */
static void tcpTimerCallback(UINT32 data)
{
    // Send tcpCondSignal to client thread so that it wakes up if waiting for condition.
    MUTEX_LOCK(tcpMutex);
	sendNewMessageF = TRUE;
	pthread_cond_signal(&tcpCondSignal);
    MUTEX_UNLOCK(tcpMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function read tcp config and send tcp message to server.
 * @param   tcpConfig
 * @param   message
 * @return
 */
static BOOL sendTcpMessage(TCP_NOTIFY_CONFIG_t * tcpConfig, CHARPTR message)
{
    BOOL                retVal = FAIL;
    CHAR                tcpServerAddr[IPV6_ADDR_LEN_MAX] = { '\0' };
    INT32               sockFd = INVALID_CONNECTION;
    UINT32              recvLen;
    CHAR                pResRecv[TCP_RECV_BUF_LENGTH];
    SOCK_ADDR_INFO_u    sockParam;

    if (GetIpAddrFromDomainName(tcpConfig->server, IP_ADDR_TYPE_MAX, tcpServerAddr) == FAIL)
    {
        EPRINT(TCP_NOTIFY, "fail to get ip from url: [url=%s]", tcpConfig->server);
        return FAIL;
    }

    DPRINT(TCP_NOTIFY, "tcp server address: [ip=%s]", tcpServerAddr);

    /* Get socket address parameters */
    if (FAIL == GetSockAddr(tcpServerAddr, tcpConfig->serverPort, &sockParam))
    {
        EPRINT(TCP_NOTIFY, "invalid ip address: [ip=%s]", tcpServerAddr);
        return FAIL;
    }

    /* Create socket based on Ip Address family */
    sockFd = socket(sockParam.sockAddr.sa_family, TCP_SOCK_OPTIONS, 0);

    if (sockFd == INVALID_CONNECTION)
    {
        EPRINT(TCP_NOTIFY, "fail to create socket: [err=%s]", STR_ERR);
        return FAIL;
    }

    do
    {
        if (connect(sockFd, &sockParam.sockAddr, SOCK_ADDR_SIZE(sockParam)) != STATUS_OK)
        {
            EPRINT(TCP_NOTIFY, "fail to connect with tcp msg server: [err=%s]", STR_ERR);
			break;
		}

        /* Make this connection as nonblocking */
        if (FALSE == SetSockFdOption(sockFd))
        {
            EPRINT(TCP_NOTIFY, "fail to make socket nonblock");
            break;
        }

        if (SendToSocket(sockFd, (UINT8PTR)message, strlen(message), MAX_TIME_TO_SEND_RECV) == FAIL)
		{
            EPRINT(TCP_NOTIFY, "fail to send msg");
			break;
		}

        if (RecvMessage(sockFd, pResRecv, &recvLen, SOM, EOM, TCP_RECV_BUF_LENGTH, MAX_TIME_TO_SEND_RECV) != SUCCESS)
		{
            EPRINT(TCP_NOTIFY, "tcp msg resp recv timeout or connection closed");
			break;
		}

        pResRecv[recvLen] = '\0';
        if (tcpValidResponse(pResRecv) == FAIL)
		{
            EPRINT(TCP_NOTIFY, "invld tcp msg resp recv: [pResRecv=%s]", pResRecv);
			break;
		}

        DPRINT(TCP_NOTIFY, "tcp msg sent successfully with valid resp");
		retVal = SUCCESS;
	}
	while(0);

	CloseSocket(&sockFd);
	return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function parse the tcp reply response and see whether response is correct or not.
 *          If correct then it returns SUCCESS.
 * @param   message
 * @return
 */
static BOOL tcpValidResponse(CHARPTR message)
{
    if(*message != SOM)
	{
        return FAIL;
    }

    message++;
    if(strstr(message, TCP_RPLY_CMD) == NULL)
    {
        return FAIL;
    }

    message += strlen(TCP_RPLY_CMD);
    if(*message != FSP)
    {
        return FAIL;
    }

    message++;
    if(*message != VALID_RESPONSE)
    {
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function fills out string with actual tcp message to send.
 * @param   inString
 * @param   outString
 */
static void getTcpMessageSendString(CHARPTR inString, CHARPTR outString)
{
    GENERAL_CONFIG_t    genCfg;
	struct tm 			currTime = { 0 };

	ReadGeneralConfig(&genCfg);

	// Get current time and date
	GetLocalTimeInBrokenTm(&currTime);

	// Make message as per define in configuration module in pTmpMsg
    snprintf(outString, TOTAL_TCP_MESSAGE_SIZE, "%c%s%c%s%c%s%c%02d%02d%04d%02d%02d%02d%c%s%c%c",
             SOM, TCP_SEND_CMD, FSP, SMART_CODE, FSP, genCfg.deviceName, FSP, currTime.tm_mday,
             (currTime.tm_mon + 1), currTime.tm_year, currTime.tm_hour, currTime.tm_min, currTime.tm_sec,
             FSP, inString, FSP, EOM);
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
