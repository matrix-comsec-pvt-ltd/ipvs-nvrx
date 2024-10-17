//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		DeviceInitiation.c
@brief      This module provides functionalities to communicate with SAMAS
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "DebugLog.h"
#include "Utils.h"
#include "NetworkManager.h"
#include "ConfigApi.h"
#include "DeviceInitiation.h"
#include "DiCommandHandle.h"
#include "ViewerUserSession.h"
#include "NetworkConfig.h"
#include "NetworkCommand.h"
#include "NetworkFileTransfer.h"
#include "DoorCommand.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define EVENT_THREAD_SLEEP_TIME			(2)
#define DI_DEVICE_POLL_DURATION			(3)  					// in sec

#define MAX_DI_RCV_MSG_SZ 				(16384)
#define MAX_DI_POLL_MSG_QUEUE			(50)
#define	RECV_MSG_HEADER_LENGTH			(25)

#define GEN_CONFIG_THREAD_STACK_SZ      (500*KILO_BYTE)
#define DI_EVENT_THREAD_STACK_SZ        (500*KILO_BYTE)
#define DI_INIT_THREAD_STACK_SZ         (1*MEGA_BYTE)
#define DI_POLL_SERVICE_THREAD_STACK_SZ (2*MEGA_BYTE)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef struct
{
	UINT8 					msgId;
	UINT8 					sessionId;
	INT32  					clientFd;
	BOOL					threadRunningF;
	BOOL					eventThreadRunningF;
	BOOL					pollServiceThreadRunningF;
	struct 	sockaddr_in 	serverAddr;
	pthread_mutex_t			diManMutex;
	CHAR					rcvBuf[MAX_DI_RCV_MSG_SZ];
	UINT32					rcvSize;
    DI_CONNECTION_STATUS_e  connectionStatus;

}DI_MAN_PARAM_t;

typedef struct
{
	UINT8					readIdx;
	UINT8					writeIdx;
    UINT8					numOfPorts[MAX_DI_POLL_MSG_QUEUE];
	pthread_mutex_t 		diPollMsgCondMutex;
	pthread_cond_t 			diPollMsgCondSignal;

}DI_POLL_MSG_QUE_t;

typedef struct
{
    BOOL					threadActive;
    pthread_mutex_t			configMutex;
    GENERAL_CONFIG_t		newGenConfig;
    GENERAL_CONFIG_t		oldGenConfig;

}CONFIG_THREAD_INFO_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static BOOL getDeviceInitiationStatus(void);
//-------------------------------------------------------------------------------------------------
static void setDeviceInitiationStatus(BOOL status);
//-------------------------------------------------------------------------------------------------
static void diInitUserSession(void);
//-------------------------------------------------------------------------------------------------
static void diInitModule(void);
//-------------------------------------------------------------------------------------------------
static BOOL diClientLogin(void);
//-------------------------------------------------------------------------------------------------
static void diSessinLogout(void);
//-------------------------------------------------------------------------------------------------
static void diUserLogout(void);
//-------------------------------------------------------------------------------------------------
static void diConnectToServer(void);
//-------------------------------------------------------------------------------------------------
static VOIDPTR deviceInitiationThread(VOIDPTR arg);
//-------------------------------------------------------------------------------------------------
static VOIDPTR diEventThread(VOIDPTR arg);
//-------------------------------------------------------------------------------------------------
static VOIDPTR diPollServiceThread(VOIDPTR arg);
//-------------------------------------------------------------------------------------------------
static BOOL diLongPoll(UINT8PTR pReqPortCnt);
//-------------------------------------------------------------------------------------------------
static BOOL diSendCommand(DI_CLIENT_CMD_e cmdId, INT32 connFd, VOIDPTR reqData);
//-------------------------------------------------------------------------------------------------
static BOOL diParseCmdResp(CHARPTR msgPtr, UINT8 msgId, VOIDPTR respData);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e diFindHeaderIndex(CHARPTR *pBufPtr, UINT8PTR pMsgId);
//-------------------------------------------------------------------------------------------------
static void diStartSystemEventThread(void);
//-------------------------------------------------------------------------------------------------
static void diStartpollServeThread(void);
//-------------------------------------------------------------------------------------------------
static void diInitPollMsgQueue(void);
//-------------------------------------------------------------------------------------------------
static void diDeIninitPollMsgQueue(void);
//-------------------------------------------------------------------------------------------------
static BOOL diWriteMsgToQueue(UINT8 portsToOpen);
//-------------------------------------------------------------------------------------------------
static VOIDPTR generalConfigThread(VOIDPTR arg);
//-------------------------------------------------------------------------------------------------
static BOOL isLoginSessionActive(void);
//-------------------------------------------------------------------------------------------------
static void waitForDiPollInterval(void);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
/* pointer to device initiation user session */
static USER_SESSION_INFO_t  *diUserSessionInfo;

static BOOL                 diRunStatusF = START;
static pthread_mutex_t      diRunStatusMutex = PTHREAD_MUTEX_INITIALIZER;

static DI_MAN_PARAM_t       DiMan;
static LOGIN_RESP_t         loginResp;
static DI_POLL_MSG_QUE_t    pollMsgQueue;
static CONFIG_THREAD_INFO_t configThreadInfo;
static const CHARPTR        diHeaderReq[MAX_DI_HEADER_REQ] = {"ACK_LOG", "ACK_POL"};

//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   Init device initiation
 */
void InitDeviceInitiation(void)
{
	diInitModule();
    MUTEX_INIT(DiMan.diManMutex, NULL);
    MUTEX_INIT(pollMsgQueue.diPollMsgCondMutex, NULL);
    MUTEX_LOCK(pollMsgQueue.diPollMsgCondMutex);
    pthread_cond_init(&pollMsgQueue.diPollMsgCondSignal, NULL);
    MUTEX_UNLOCK(pollMsgQueue.diPollMsgCondMutex);
    diConnectToServer();
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   DeInitDeviceInitiation
 */
void DeInitDeviceInitiation(void)
{
    DPRINT(DEVICE_INITIATION, "deinit device initiation module");
    setDeviceInitiationStatus(STOP);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get device initiation process running status
 * @return  STOP if terminate the device initiaition process otherwise START
 */
static BOOL getDeviceInitiationStatus(void)
{
    MUTEX_LOCK(diRunStatusMutex);
    BOOL status = diRunStatusF;
    MUTEX_UNLOCK(diRunStatusMutex);
    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set device initiation running status
 * @param   status
 */
static void setDeviceInitiationStatus(BOOL status)
{
    MUTEX_LOCK(diRunStatusMutex);
    diRunStatusF = status;
    MUTEX_UNLOCK(diRunStatusMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Init device initiation user session info
 * @return
 */
static void diInitUserSession(void)
{
    /* Get local copy of device init user session */
    diUserSessionInfo = GetDiUserSession(DI_CLIENT_SESSION);

    /* Init device init user session info */
    MUTEX_LOCK(diUserSessionInfo->userSessionLock);
    diUserSessionInfo->isSessionActive = FALSE;
    MUTEX_UNLOCK(diUserSessionInfo->userSessionLock);
    diUserSessionInfo->sessionId = -1;
    diUserSessionInfo->pollFd = INVALID_CONNECTION;
    diUserSessionInfo->viewerUserCfgIndex = MAX_USER_ACCOUNT;
    diUserSessionInfo->viewerSessMultiLogin = FALSE;
    diUserSessionInfo->viewerUserSysTk = 0;
    diUserSessionInfo->userIndex = USER_ADMIN;
    diUserSessionInfo->clientType = NW_CLIENT_TYPE_MAX;
    diUserSessionInfo->pollTimerCnt = 0;
    diUserSessionInfo->keepAliveTimerCnt = 0;
    diUserSessionInfo->ipAddr[0] = '\0';
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Init device initiation module
 */
static void diInitModule(void)
{
    DPRINT(DEVICE_INITIATION, "init device initiation module");

    /* Init device init user session info */
    diInitUserSession();

    /* Init DI manager info */
    DiMan.rcvSize = 0;
    DiMan.sessionId = MAX_NW_CLIENT;
    DiMan.clientFd = INVALID_CONNECTION;
    DiMan.threadRunningF = FALSE;
    DiMan.eventThreadRunningF = FALSE;
	DiMan.pollServiceThreadRunningF = FALSE;
    memset(&DiMan.rcvBuf, 0, MAX_DI_RCV_MSG_SZ);
	memset(&DiMan.serverAddr, 0, sizeof(DiMan.serverAddr));
	loginResp.deviceId = INVALID_CONNECTION;
	loginResp.loginState =  FAIL;
    setDeviceInitiationStatus(START);
    SetDiConnectionStatus(DI_STATUS_CONNECTION_IN_PROGRESS);
	configThreadInfo.threadActive = FALSE;
    MUTEX_INIT(configThreadInfo.configMutex, NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Init device initiation message queue
 */
static void diInitPollMsgQueue(void)
{
    DPRINT(DEVICE_INITIATION, "device initiation poll msg queue init");
	pollMsgQueue.readIdx = 0;
	pollMsgQueue.writeIdx = 0;
    memset(pollMsgQueue.numOfPorts, 0, sizeof(pollMsgQueue.numOfPorts));
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Deinit device initiation message queue
 */
static void diDeIninitPollMsgQueue(void)
{
	diInitPollMsgQueue();
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Device initiation connection with server
 */
void diConnectToServer(void)
{
#if !defined(OEM_JCI)
    GENERAL_CONFIG_t generalCnfg;

	ReadGeneralConfig(&generalCnfg);
    if(generalCnfg.devInitWithServerF == FALSE)
    {
        EPRINT(DEVICE_INITIATION, "device initiation with server is disabled");
        return;
    }

    if (DiMan.threadRunningF == TRUE)
    {
        EPRINT(DEVICE_INITIATION, "device initiation thread already running");
        return;
    }

    setDeviceInitiationStatus(START);
    DPRINT(DEVICE_INITIATION, "device initiation started with samas server");
    if (FAIL == Utils_CreateThread(NULL, deviceInitiationThread, NULL, DETACHED_THREAD, DI_INIT_THREAD_STACK_SZ))
    {
        EPRINT(DEVICE_INITIATION, "fail to create device initiation thread");
        return;
    }

    DiMan.threadRunningF = TRUE;
#endif
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Device initiation config change notify
 * @param   newGeneralConfig
 * @param   oldGeneralConfig
 */
void DiGeneralConfigNotify(GENERAL_CONFIG_t newGeneralConfig, GENERAL_CONFIG_t *oldGeneralConfig)
{
#if !defined(OEM_JCI)
    do
    {
        /* Is SAMAS integration flag status changed? */
        if (newGeneralConfig.devInitWithServerF != oldGeneralConfig->devInitWithServerF)
        {
            /* SAMAS integration flag status changed */
            break;
        }

        /* Is flag status disabled? */
        if (newGeneralConfig.devInitWithServerF == FALSE)
        {
            /* Nothing to do if flag status is not changed and flag is disabled */
            return;
        }

        /* Is SAMAS ip address changed? */
        if (strcmp(newGeneralConfig.devInitServerIpAddr, oldGeneralConfig->devInitServerIpAddr))
        {
            break;
        }

        /* Is SAMAS port changed? */
        if (newGeneralConfig.devInitServerPort != oldGeneralConfig->devInitServerPort)
        {
            break;
        }

        /* No change in SAMAS integration config */
        return;

    } while(0);

    /* Change the connection status with SAMAS as it needs to connect again */
    SetDiConnectionStatus(DI_STATUS_CONNECTION_IN_PROGRESS);
    DPRINT(DEVICE_INITIATION, "change in samas integration config");
    MUTEX_LOCK(configThreadInfo.configMutex);
    if(configThreadInfo.threadActive == TRUE)
    {
        MUTEX_UNLOCK(configThreadInfo.configMutex);
        EPRINT(DEVICE_INITIATION, "config change thread already running");
        return;
    }
    configThreadInfo.threadActive = TRUE;
    memcpy(&configThreadInfo.newGenConfig, &newGeneralConfig, sizeof(GENERAL_CONFIG_t));
    memcpy(&configThreadInfo.oldGenConfig, oldGeneralConfig, sizeof(GENERAL_CONFIG_t));
    MUTEX_UNLOCK(configThreadInfo.configMutex);

    if (FAIL == Utils_CreateThread(NULL, generalConfigThread, NULL, DETACHED_THREAD, GEN_CONFIG_THREAD_STACK_SZ))
    {
        EPRINT(DEVICE_INITIATION, "fail to create config change thread");
        MUTEX_LOCK(configThreadInfo.configMutex);
        configThreadInfo.threadActive = FALSE;
        MUTEX_UNLOCK(configThreadInfo.configMutex);
    }
#endif
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Config change thread
 * @param   arg
 * @return
 */
static VOIDPTR generalConfigThread(VOIDPTR arg)
{
    BOOL startDevInitF = FALSE;

    if(configThreadInfo.newGenConfig.devInitWithServerF != configThreadInfo.oldGenConfig.devInitWithServerF)
	{
		if(configThreadInfo.newGenConfig.devInitWithServerF == TRUE)
		{
            DPRINT(DEVICE_INITIATION, "device initiation config changed to enable");
            startDevInitF = TRUE;
		}
		else
		{
            DPRINT(DEVICE_INITIATION, "device initiation config changed to disable");
            setDeviceInitiationStatus(STOP);
		}
	}
    else
	{
        DPRINT(DEVICE_INITIATION, "device initiation server ip/port changed: [ip=%s], [port=%d]",
               configThreadInfo.newGenConfig.devInitServerIpAddr, configThreadInfo.newGenConfig.devInitServerPort);
        setDeviceInitiationStatus(STOP);
        startDevInitF = TRUE;
	}

    /* Start device initiation */
    while(startDevInitF == TRUE)
    {
        MUTEX_LOCK(DiMan.diManMutex);
        if(DiMan.threadRunningF == FALSE)
        {
            MUTEX_UNLOCK(DiMan.diManMutex);
            diConnectToServer();
            break;
        }
        MUTEX_UNLOCK(DiMan.diManMutex);
        usleep(100000);
    }

    MUTEX_LOCK(configThreadInfo.configMutex);
	configThreadInfo.threadActive = FALSE;
    MUTEX_UNLOCK(configThreadInfo.configMutex);
	pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   device initiation thread
 * @param   arg
 * @return
 */
static VOIDPTR deviceInitiationThread(VOIDPTR arg)
{
	BOOL				retVal = SUCCESS;
    BOOL                terminateThreadF = FALSE;
	UINT8				longPollRetryCnt = 0;
    UINT8				reqPortCnt;
	INT32				sockFd = INVALID_CONNECTION;
	GENERAL_CONFIG_t 	generalCnfg;
    CHAR                advncdetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];

	THREAD_START("DEV_INIT")
    SetDiConnectionStatus(DI_STATUS_CONNECTION_IN_PROGRESS);
	sleep(10);

    while(TRUE)
    {
        /* Make connection with server and send login request */
        while(TRUE)
        {
            if (STOP == getDeviceInitiationStatus())
            {
                terminateThreadF = TRUE;
                break;
            }

            ReadGeneralConfig(&generalCnfg);
            DiMan.serverAddr.sin_family = AF_INET;
            DiMan.serverAddr.sin_port = htons(generalCnfg.devInitServerPort);
            DiMan.serverAddr.sin_addr.s_addr = inet_addr(generalCnfg.devInitServerIpAddr);

            sockFd = socket(AF_INET, TCP_SOCK_OPTIONS, 0);
            if (sockFd == INVALID_CONNECTION)
            {
                EPRINT(DEVICE_INITIATION, "fail to open socket: [err=%s]", STR_ERR);
                sleep(DI_DEVICE_POLL_DURATION);
                continue;
            }

            DPRINT(DEVICE_INITIATION, "trying to connect to samas server!!");
            if ((connect(sockFd, (struct sockaddr_in *)&DiMan.serverAddr, sizeof(DiMan.serverAddr)) != STATUS_OK) || (FALSE == SetSockFdOption(sockFd)))
            {
                EPRINT(DEVICE_INITIATION, "fail to connect device to server: [err=%s]", STR_ERR);
                CloseSocket(&sockFd);

                /* Update connection status to failure */
                SetDiConnectionStatus(DI_STATUS_CONNECTION_FAILURE);
                sleep(DI_DEVICE_POLL_DURATION);
                continue;
            }

            DPRINT(DEVICE_INITIATION, "connected with samas server, sending login request: [ip=%s], [port=%d]", generalCnfg.devInitServerIpAddr, generalCnfg.devInitServerPort);
            DiMan.clientFd = sockFd;

            /* Prepare and send login request */
            retVal = diClientLogin();
            if (retVal != SUCCESS)
            {
                if (retVal == REFUSE)
                {
                    EPRINT(DEVICE_INITIATION, "login connection refused by samas: [ip=%s], [port=%d]", generalCnfg.devInitServerIpAddr, generalCnfg.devInitServerPort);
                    CloseSocket(&DiMan.clientFd);
                    setDeviceInitiationStatus(STOP);
                    terminateThreadF = TRUE;
                    SetDiConnectionStatus(DI_STATUS_CONNECTION_REFUSED);
                    break;
                }

                SetDiConnectionStatus((retVal == IN_PROGRESS) ? DI_STATUS_CONNECTION_REQUEST_PENDING : DI_STATUS_CONNECTION_FAILURE);
                EPRINT(DEVICE_INITIATION, "login failed: [ip=%s], [port=%d]", generalCnfg.devInitServerIpAddr, generalCnfg.devInitServerPort);
                diSessinLogout();
                sleep(DI_DEVICE_POLL_DURATION);
                continue;
            }

            DPRINT(DEVICE_INITIATION, "device initiation login request success: [sessionId=%d]", DI_CLIENT_SESSION);
            MUTEX_LOCK(diUserSessionInfo->userSessionLock);
            diUserSessionInfo->isSessionActive = TRUE;
            MUTEX_UNLOCK(diUserSessionInfo->userSessionLock);
            diUserSessionInfo->eventReadIndex = GetCurrentEventWriteIndex();
            diUserSessionInfo->sessionId = loginResp.deviceId;
            snprintf(diUserSessionInfo->ipAddr, sizeof(diUserSessionInfo->ipAddr), "%s", generalCnfg.devInitServerIpAddr);
            DiMan.sessionId = DI_CLIENT_SESSION;
            snprintf(advncdetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, SAMAS_INTEGRATION_EVT_STR " | %s", diUserSessionInfo->ipAddr);
            WriteEvent(LOG_USER_EVENT, LOG_USER_SESSION, ADMIN_USER_NAME, advncdetail, EVENT_LOGIN);
            if(loginResp.loginState != SUCCESS)
            {
                EPRINT(DEVICE_INITIATION, "trying to start poll service thread and event thread without login");
                break;
            }

            DPRINT(DEVICE_INITIATION, "starting poll service thread and event thread on successfull login");
            if(DiMan.eventThreadRunningF == FALSE)
            {
                diStartSystemEventThread();
            }

            if(DiMan.pollServiceThreadRunningF == FALSE)
            {
                diStartpollServeThread();
            }

            /* Update the connection status with SAMAS as connected */
            SetDiConnectionStatus(DI_STATUS_CONNECTED);
            break;
        }

        /* Make connection with server and send polling request */
        while(terminateThreadF == FALSE)
        {
            if (STOP == getDeviceInitiationStatus())
            {
                terminateThreadF = TRUE;
                break;
            }

            DPRINT(DEVICE_INITIATION, "send long poll request to server");
            reqPortCnt = 0;
            if(diLongPoll(&reqPortCnt) != SUCCESS)
            {
                longPollRetryCnt++;
                if (longPollRetryCnt > 2)
                {
                    longPollRetryCnt = 0;
                    setDeviceInitiationStatus(STOP);
                    SetDiConnectionStatus(DI_STATUS_CONNECTION_FAILURE);
                    break;
                }

                /* Wait between two polling if interval configured */
                waitForDiPollInterval();
                continue;
            }

            longPollRetryCnt = 0;
            DPRINT(DEVICE_INITIATION, "long poll response: [reqPortCnt=%d]", reqPortCnt);

            /* Write msg in queue if server needed port */
            if (reqPortCnt > 0)
            {
                if(diWriteMsgToQueue(reqPortCnt) == FAIL)
                {
                    EPRINT(DEVICE_INITIATION, "fail to write msg in queue");
                }
            }

            /* Wait between two polling if interval configured */
            waitForDiPollInterval();
        }

        diSessinLogout();
        DPRINT(DEVICE_INITIATION, "di exited from polling: [terminateThreadF=%d]", terminateThreadF);

        do
        {
            if((DiMan.pollServiceThreadRunningF == FALSE) && (DiMan.eventThreadRunningF == FALSE))
            {
                DPRINT(DEVICE_INITIATION, "stop poll service thread and event thread");
                break;
            }

            DPRINT(DEVICE_INITIATION, "waiting to stop polling and event thread: [polling=%d], [event=%d]",
                   DiMan.pollServiceThreadRunningF, DiMan.eventThreadRunningF);
            usleep(100000);

        }while(TRUE);

        /* Do we have to exit from thread? */
        if (terminateThreadF == TRUE)
        {
            /* Yes, Exit from thread */
            break;
        }

        /* Logout current user session and reinit user session params */
        UserLogout(DI_CLIENT_SESSION);
        diInitUserSession();

        /* Restart the device initiation session */
        setDeviceInitiationStatus(START);
        SetDiConnectionStatus(DI_STATUS_CONNECTION_IN_PROGRESS);
        EPRINT(DEVICE_INITIATION, "device initiation session restarted, sending login request");
    }

    /* Logout DI session and exit from thread */
    diUserLogout();
	pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Device initiation user logout
 */
void diUserLogout(void)
{
    /* User session logout */
	UserLogout(DI_CLIENT_SESSION);
	diInitModule();
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Device initiation session logout
 */
static void diSessinLogout(void)
{
    DPRINT(DEVICE_INITIATION, "device initiation session logout");
	CloseSocket(&DiMan.clientFd);
	loginResp.deviceId = INVALID_CONNECTION;
    loginResp.loginState = FAIL;
    loginResp.pollDuration = 0;
    loginResp.pollInterval = 0;
    diUserSessionInfo->sessionId = -1;
	DiMan.sessionId = MAX_NW_CLIENT;
	memset(&DiMan.rcvBuf, '\0', MAX_DI_RCV_MSG_SZ);
	DiMan.rcvSize = 0;

	//if this thread is in conditional send signal and make it free
	if(DiMan.pollServiceThreadRunningF == TRUE)
	{
        DPRINT(DEVICE_INITIATION, "sending signal to poll service thread");
        MUTEX_LOCK(pollMsgQueue.diPollMsgCondMutex);
		pthread_cond_signal(&pollMsgQueue.diPollMsgCondSignal);
        MUTEX_UNLOCK(pollMsgQueue.diPollMsgCondMutex);
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Device initiation client login
 * @return
 */
static BOOL diClientLogin(void)
{
    BOOL            retVal = SUCCESS;
    CHARPTR         msgPtr;
    LOGIN_RESP_t    tempLoginResp;

    if (diSendCommand(LOGIN_CMD, DiMan.clientFd, NULL) != SUCCESS)
	{
        EPRINT(DEVICE_INITIATION, "fail to send login command");
        return FAIL;
    }

    retVal = RecvMessage(DiMan.clientFd, DiMan.rcvBuf, &DiMan.rcvSize, SOM, EOM, MAX_DI_RCV_MSG_SZ-1, DI_DEVICE_POLL_DURATION);
    if (retVal != SUCCESS)
    {
        if (retVal == REFUSE)
        {
            EPRINT(DEVICE_INITIATION, "connection closed by remote");
            return REFUSE;
        }

        EPRINT(DEVICE_INITIATION, "fail to recv msg");
        return FAIL;
    }

    DiMan.rcvBuf[DiMan.rcvSize] = '\0';
    DPRINT(DEVICE_INITIATION, "msg recv: [size=%d], [data=%s]", DiMan.rcvSize, DiMan.rcvBuf);
    msgPtr = (DiMan.rcvBuf + 1);
    if(diFindHeaderIndex(&msgPtr, &DiMan.msgId) != CMD_SUCCESS)
    {
        EPRINT(DEVICE_INITIATION, "fail to parse header index: [msg=%s]", msgPtr);
        return FAIL;
    }

    if (DiMan.msgId != DI_ACK_LOG)
    {
        EPRINT(DEVICE_INITIATION, "invld msg id recv in resp: [rcvBuf=%s]", DiMan.rcvBuf);
        return FAIL;
    }

    retVal = diParseCmdResp(msgPtr, DiMan.msgId, (VOIDPTR)&tempLoginResp);
    if(retVal == FAIL)
    {
        EPRINT(DEVICE_INITIATION, "login req refuse");
        return REFUSE;
    }

    if (retVal == IN_PROGRESS)
    {
        return IN_PROGRESS;
    }

    memcpy(&loginResp, &tempLoginResp, sizeof(LOGIN_RESP_t));
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Start system event handling thread
 */
static void diStartSystemEventThread(void)
{
    /* Is login session active? */
    if (FALSE == isLoginSessionActive())
    {
        EPRINT(DEVICE_INITIATION, "network session is not active, fail to start event thread");
        return;
    }

    if (FAIL == Utils_CreateThread(NULL, diEventThread, NULL, DETACHED_THREAD, DI_EVENT_THREAD_STACK_SZ))
    {
        EPRINT(DEVICE_INITIATION, "system event thread not started");
        return;
    }

    DiMan.eventThreadRunningF = TRUE;
    DPRINT(DEVICE_INITIATION, "event thread statrted");
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   System event handling thread
 * @param   arg
 * @return
 */
static VOIDPTR diEventThread(VOIDPTR arg)
{
    INT32 sockFd = INVALID_CONNECTION;

	THREAD_START("DI_EVENT");
    while(TRUE)
    {
        if (STOP == getDeviceInitiationStatus())
        {
            DPRINT(DEVICE_INITIATION, "event poll thread exit");
            break;
        }

        sleep(EVENT_THREAD_SLEEP_TIME);
        if (FALSE == isLoginSessionActive())
        {
            continue;
        }

        sockFd = socket(AF_INET, TCP_SOCK_OPTIONS, 0);
        if (sockFd == INVALID_CONNECTION)
        {
            EPRINT(DEVICE_INITIATION, "fail to create socket for event");
            continue;
        }

        if ((connect(sockFd, (struct sockaddr_in *)&DiMan.serverAddr, sizeof(DiMan.serverAddr)) != STATUS_OK) || (FALSE == SetSockFdOption(sockFd)))
        {
            CloseSocket(&sockFd);
            EPRINT(DEVICE_INITIATION, "fail to connect to server for event");
            continue;
        }

        /* Get and send live events to client */
        GetLiveEvents(DI_CLIENT_SESSION, sockFd, CLIENT_CB_TYPE_NATIVE);
        sockFd = INVALID_CONNECTION;
    }

    CloseSocket(&sockFd);
	DiMan.eventThreadRunningF = FALSE;
	pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Device initiation start poll service thread
 */
static void diStartpollServeThread(void)
{
	diInitPollMsgQueue();

    /* Is login session active? */
    if (FALSE == isLoginSessionActive())
    {
        EPRINT(DEVICE_INITIATION, "network session is not active, fail to start poll service thread");
        return;
    }

    if (FAIL == Utils_CreateThread(NULL, diPollServiceThread, NULL, DETACHED_THREAD, DI_POLL_SERVICE_THREAD_STACK_SZ))
    {
        EPRINT(DEVICE_INITIATION, "poll service thread not started");
        return;
    }

    DiMan.pollServiceThreadRunningF = TRUE;
    DPRINT(DEVICE_INITIATION, "poll service thread started");
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Device initiation write msg to queue
 * @param   portsToOpen
 * @return
 */
static BOOL diWriteMsgToQueue(UINT8 portsToOpen)
{
    if(DiMan.pollServiceThreadRunningF == FAIL)
    {
        DPRINT(DEVICE_INITIATION, "trying to start polling service thread");
        diStartpollServeThread();
        return FAIL;
    }

    MUTEX_LOCK(pollMsgQueue.diPollMsgCondMutex);
    UINT8 readIdx = pollMsgQueue.readIdx;
    UINT8 writeIdx = pollMsgQueue.writeIdx + 1;
    if(writeIdx >= MAX_DI_POLL_MSG_QUEUE)
    {
        writeIdx = 0;
    }

    if(writeIdx == readIdx)
    {
        MUTEX_UNLOCK(pollMsgQueue.diPollMsgCondMutex);
        EPRINT(DEVICE_INITIATION, "device initiation queue full: [readIdx=%d], [writeIdx=%d]", readIdx, writeIdx);
        return FAIL;
    }

    pollMsgQueue.numOfPorts[writeIdx] = portsToOpen;
    pollMsgQueue.writeIdx = writeIdx;
    pthread_cond_signal(&pollMsgQueue.diPollMsgCondSignal);
    MUTEX_UNLOCK(pollMsgQueue.diPollMsgCondMutex);
    DPRINT(DEVICE_INITIATION, "device initiation msg write successful: [readIdx=%d], [writeIdx=%d]", readIdx, writeIdx);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Device initiation poll service thread
 * @param   arg
 * @return
 */
static VOIDPTR diPollServiceThread(VOIDPTR arg)
{
    UINT8                   portCnt, totalPort;
    INT32                   clientCmdFd = INVALID_FILE_FD;
    UINT32                  rcvSize;
    CHARPTR                 msgPtr;
    UINT8                   msgId;
    UINT8                   sessionIdx;
    CHAR                    rcvBuf[MAX_DI_RCV_MSG_SZ];
    UINT32                  deviceId;
    CHAR                    replyMsg[MAX_REPLY_SZ];
    USER_ACCOUNT_CONFIG_t 	userAccountConfig;

	THREAD_START("DI_POLL");
    while(TRUE)
	{
        if (STOP == getDeviceInitiationStatus())
        {
            DPRINT(DEVICE_INITIATION, "poll service thread exit");
            break;
        }

        MUTEX_LOCK(pollMsgQueue.diPollMsgCondMutex);
        if(pollMsgQueue.readIdx == pollMsgQueue.writeIdx)
        {
            pthread_cond_wait(&pollMsgQueue.diPollMsgCondSignal, &pollMsgQueue.diPollMsgCondMutex);
            MUTEX_UNLOCK(pollMsgQueue.diPollMsgCondMutex);
            continue;
        }

        pollMsgQueue.readIdx = pollMsgQueue.readIdx + 1;
        if(pollMsgQueue.readIdx >= MAX_DI_POLL_MSG_QUEUE)
        {
            pollMsgQueue.readIdx = 0;
        }

        totalPort = pollMsgQueue.numOfPorts[pollMsgQueue.readIdx];
        DPRINT(DEVICE_INITIATION, "msg read: [totalPort=%d], [readIdx=%d], [writeIdx=%d]", totalPort, pollMsgQueue.readIdx, pollMsgQueue.writeIdx);
        MUTEX_UNLOCK(pollMsgQueue.diPollMsgCondMutex);

        /* Port count must be non zero to procced */
        if (totalPort == 0)
        {
            continue;
        }

        portCnt = 0;
        while(totalPort > portCnt)
        {
            if (STOP == getDeviceInitiationStatus())
            {
                EPRINT(DEVICE_INITIATION, "poll service thread exit");
                break;
            }

            portCnt++;
            DPRINT(DEVICE_INITIATION, "create connection: [portCnt=%d], [totalPort=%d]", portCnt, totalPort);
            clientCmdFd = socket(AF_INET, TCP_SOCK_OPTIONS, 0);
            if (clientCmdFd == INVALID_CONNECTION)
            {
                EPRINT(DEVICE_INITIATION, "command port connection fail: [err=%s]", STR_ERR);
                continue;
            }

            if ((connect(clientCmdFd, (struct sockaddr_in *)&DiMan.serverAddr, sizeof(DiMan.serverAddr)) != STATUS_OK) || (FALSE == SetSockFdOption(clientCmdFd)))
            {
                EPRINT(DEVICE_INITIATION, "command port connection fail");
                CloseSocket(&clientCmdFd);
                continue;
            }

            DPRINT(DEVICE_INITIATION, "port opened and connected successfully");
            deviceId = diUserSessionInfo->sessionId;
            if(diSendCommand(SEND_DID, clientCmdFd, (VOIDPTR)&deviceId) != SUCCESS)
            {
                CloseSocket(&clientCmdFd);
                EPRINT(DEVICE_INITIATION, "fail to send did cmd: [portCnt=%d], [totalPort=%d]", portCnt, totalPort);
                continue;
            }

            if(RecvMessage(clientCmdFd, rcvBuf, &rcvSize, SOM, EOM, MAX_DI_RCV_MSG_SZ-1, loginResp.pollDuration) != SUCCESS)
            {
                CloseSocket(&clientCmdFd);
                EPRINT(DEVICE_INITIATION, "msg recv timeout or connection closed: [portCnt=%d], [totalPort=%d]", portCnt, totalPort);
                continue;
            }

            /* Append null and remove SOM from received data */
            rcvBuf[rcvSize] = '\0';
            msgPtr = (rcvBuf + 1);

            //Get header
            if(FindHeaderIndex(&msgPtr, &msgId) != CMD_SUCCESS)
            {
                CloseSocket(&clientCmdFd);
                EPRINT(DEVICE_INITIATION, "invld msg id recv: [portCnt=%d], [totalPort=%d]", portCnt, totalPort);
                continue;
            }

            DPRINT(DEVICE_INITIATION, "di msg recvd: [msgId=%s]", headerReq[msgId]);
            switch(msgId)
            {
                case GET_CFG:
                {
                    // parse request string for session index & check if the session is active or not
                    if(FindSessionIndex(&msgPtr, &sessionIdx) != CMD_SUCCESS)
                    {
                        break;
                    }

                    /* Is login session active? */
                    if (FALSE == isLoginSessionActive())
                    {
                        break;
                    }

                    ProcessGetConfig(msgPtr, clientCmdFd, CLIENT_CB_TYPE_NATIVE);
                }
                break;

                case SET_CFG:
                {
                    // parse request string for session index & check if the session is active or not
                    if(FindSessionIndex(&msgPtr, &sessionIdx) != CMD_SUCCESS)
                    {
                        break;
                    }

                    /* Is login session active? */
                    if (FALSE == isLoginSessionActive())
                    {
                        break;
                    }

                    ReadSingleUserAccountConfig(diUserSessionInfo->userIndex, &userAccountConfig);
                    if (userAccountConfig.userGroup != ADMIN)
                    {
                        snprintf(replyMsg, MAX_REPLY_SZ, "%c%s%c%d%c%c", SOM, headerReq[RPL_CFG], FSP, CMD_NO_PRIVILEGE, FSP, EOM);
                        SendToSocket(clientCmdFd, (UINT8PTR)replyMsg, strlen(replyMsg), MESSAGE_REPLY_TIMEOUT);
                        break;
                    }

                    // start processing the SET_CFG request
                    ProcessSetConfig(msgPtr, clientCmdFd, userAccountConfig.username, CLIENT_CB_TYPE_NATIVE);
                }
                break;

                case DEF_CFG:
                {
                    // parse request string for session index & check if the session is active or not
                    if(FindSessionIndex(&msgPtr, &sessionIdx) != CMD_SUCCESS)
                    {
                        break;
                    }

                    /* Is login session active? */
                    if (FALSE == isLoginSessionActive())
                    {
                        break;
                    }

                    ReadSingleUserAccountConfig(diUserSessionInfo->userIndex, &userAccountConfig);
                    if (userAccountConfig.userGroup != ADMIN)
                    {
                        snprintf(replyMsg, MAX_REPLY_SZ, "%c%s%c%d%c%c", SOM, headerReq[RPL_CFG], FSP, CMD_NO_PRIVILEGE, FSP, EOM);
                        SendToSocket(clientCmdFd, (UINT8PTR)replyMsg, strlen(replyMsg), MESSAGE_REPLY_TIMEOUT);
                        break;
                    }

                    // start processing the SET_CFG request
                    ProcessDefConfig(msgPtr, clientCmdFd, userAccountConfig.username, CLIENT_CB_TYPE_NATIVE);
                }
                break;

                case SET_CMD:
                {
                    //Find session index from given session id
                    if(FindSessionIndex(&msgPtr, &sessionIdx) != CMD_SUCCESS)
                    {
                        break;
                    }

                    /* Is login session active? */
                    if (FALSE == isLoginSessionActive())
                    {
                        break;
                    }

                    /* Don't close this connection as we have passed fd for further action */
                    ProcessSetCommand(msgPtr, DI_CLIENT_SESSION, clientCmdFd, CLIENT_CB_TYPE_NATIVE);
                    clientCmdFd = INVALID_CONNECTION;
                }
                break;

                case REQ_FTS:
                {
                    // find session index
                    if(FindSessionIndex(&msgPtr, &sessionIdx) != CMD_SUCCESS)
                    {
                        snprintf(replyMsg, MAX_REPLY_SZ, "%c%s%c%d%c%c", SOM, headerReq[RPL_FTS], FSP, CMD_INVALID_SESSION, FSP, EOM);
                        SendToSocket(clientCmdFd, (UINT8PTR)replyMsg, strlen(replyMsg), MESSAGE_REPLY_TIMEOUT);
                        break;
                    }

                    /* Is login session active? */
                    if (FALSE == isLoginSessionActive())
                    {
                        break;
                    }

                    /* Don't close this connection as we have passed fd for further action */
                    ProcessFileTransferReq(&msgPtr, clientCmdFd, DI_CLIENT_SESSION);
                    clientCmdFd = INVALID_CONNECTION;
                }
                break;

                case DOR_CMD:
                {
                    /* Process door command request */
                    ProcessDoorCommand(msgPtr);
                }
                break;

                default:
                {
                    /* Nothing to do */
                }
                break;
            }

            /* Close the connection if fd is valid */
            CloseSocket(&clientCmdFd);
        }
	}

	diDeIninitPollMsgQueue();
	DiMan.pollServiceThreadRunningF = FALSE;
    CloseSocket(&clientCmdFd);
	pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Device initiation send long poll message and get the required port count (new connections)
 * @param   pReqPortCnt
 * @return
 */
static BOOL diLongPoll(UINT8PTR pReqPortCnt)
{
    BOOL    retVal;
    UINT32  deviceId;
    CHARPTR msgPtr;

    deviceId = diUserSessionInfo->sessionId;
    if (SUCCESS != diSendCommand(LONG_POLL, DiMan.clientFd, (VOIDPTR)&deviceId))
    {
        EPRINT(DEVICE_INITIATION, "poll msg sending fail");
        waitForDiPollInterval();
        return FAIL;
    }

    /* Here we cannot differentiate failure due to timeout or anything like connection close etc,
     * so select above will indicate that given fd is readable or timeout in given time. if fd is
     * readable then we can wait to recv msg if still msg recv fail then we can declair failure */
    retVal = RecvMessage(DiMan.clientFd, DiMan.rcvBuf, &DiMan.rcvSize, SOM, EOM, MAX_DI_RCV_MSG_SZ-1, loginResp.pollDuration);
    if(retVal != SUCCESS)
    {
        if (retVal == REFUSE)
        {
            EPRINT(DEVICE_INITIATION, "connection closed by remote");
            return REFUSE;
        }

        EPRINT(DEVICE_INITIATION, "fail to recv msg");
        return FAIL;
    }

    DiMan.rcvBuf[DiMan.rcvSize] = '\0';
    msgPtr = (DiMan.rcvBuf + 1);

    if(diFindHeaderIndex(&msgPtr, &DiMan.msgId) != CMD_SUCCESS)
    {
        EPRINT(NETWORK_MANAGER, "Header index find error :%s", msgPtr);
        return FAIL;
    }

    if(DiMan.msgId != DI_ACK_POL)
    {
        EPRINT(DEVICE_INITIATION, "invld msg id: [msgId=%d]", DiMan.msgId);
        return FAIL;
    }

    //dont accept any packet other than DI_ACK_POLL if packet parsing is fail dont break loop
    if(diParseCmdResp(msgPtr, DiMan.msgId, (VOIDPTR)pReqPortCnt) == FAIL)
    {
        EPRINT(DEVICE_INITIATION, "fail to parse long poll response");
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Device initiation send command
 * @param   cmdId
 * @param   connFd
 * @param   data
 * @return
 */
static BOOL diSendCommand(DI_CLIENT_CMD_e cmdId, INT32 connFd, VOIDPTR data)
{
	CHAR 	msgBuf[MAX_CMD_SZ];
	UINT16 	cmdOutLen;

    if(DiMakeXXXCmd(cmdId, msgBuf, &cmdOutLen, data) == FAIL)
	{
        return FAIL;
    }

    DPRINT(DEVICE_INITIATION, "req sending: [msg=%s], [fd=%d]", msgBuf, connFd);
    return SendToSocket(connFd, (UINT8PTR)msgBuf, cmdOutLen, DI_DEVICE_POLL_DURATION);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   diParseCmdResp
 * @param   msgPtr
 * @param   msgId
 * @param   respData
 * @return
 */
static BOOL diParseCmdResp(CHARPTR msgPtr, UINT8 msgId, VOIDPTR respData)
{
    return DiParseXXXCmdResp(msgId, &msgPtr, respData);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function out index of header if success otherwise error
 * @param   pBufPtr
 * @param   pMsgId
 * @return
 */
NET_CMD_STATUS_e diFindHeaderIndex(CHARPTR *pBufPtr, UINT8PTR pMsgId)
{
    CHAR buf[RECV_MSG_HEADER_LENGTH];

    if(ParseStr(pBufPtr, FSP, buf, RECV_MSG_HEADER_LENGTH) == FAIL)
	{
        return CMD_INVALID_SYNTAX;
    }

    *pMsgId = ConvertStringToIndex(buf, diHeaderReq, MAX_DI_HEADER_REQ);
    if(*pMsgId >= MAX_HEADER_REQ)
    {
        return CMD_INVALID_MESSAGE;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Check login session active or not
 * @return  TRUE/FALSE
 */
static BOOL isLoginSessionActive(void)
{
    MUTEX_LOCK(diUserSessionInfo->userSessionLock);
    if(diUserSessionInfo->isSessionActive == FALSE)
    {
        MUTEX_UNLOCK(diUserSessionInfo->userSessionLock);
        return FALSE;
    }
    MUTEX_UNLOCK(diUserSessionInfo->userSessionLock);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Sleep the process for next poll with poll interval time (If it is non zero)
 */
static void waitForDiPollInterval(void)
{
    if (loginResp.pollInterval == 0)
    {
        /* Sleep is not required as interval is zero */
        return;
    }

    /* Sleep between two polling */
    sleep(loginResp.pollInterval);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Modify connection status of NVR with SAMAS
 * @param   status
 */
void SetDiConnectionStatus(DI_CONNECTION_STATUS_e status)
{
    DiMan.connectionStatus = status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get connection status of NVR with SAMAS
 * @param   status
 */
DI_CONNECTION_STATUS_e GetDiConnectionStatus(void)
{
    return DiMan.connectionStatus;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
