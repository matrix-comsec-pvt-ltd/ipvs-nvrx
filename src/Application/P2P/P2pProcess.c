//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		P2pProcess.c
@brief      File containing the defination of internal modules functions to provide P2P service
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <ctype.h>

/* Application Includes */
#include "P2pProcess.h"
#include "CommonApi.h"
#include "Utils.h"
#include "DebugLog.h"
#include "CameraInterface.h"
#include "MobileBroadBand.h"
#include "P2pClientComm.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define P2P_PROC_TMR_TICK_RESOL_IN_MS   100
#define P2P_PROC_TMR_TICK_RESOL_IN_NS   100000000LL /* P2P_PROC_TMR_TICK_RESOL_IN_MS * NANO_SEC_PER_MILLI_SEC */
#define P2P_MAX_SLEEP_DELAY_CNT_IN_MS   3000

#define GET_MAC_OCTET(x)                ((x-1) * 3)
#define P2P_SERVER_RAND_NUM_START       7
#define CLIENT_MODEL_SPECIAL_CHAR_LIST  " \t'\"\n\r\\"

#define P2P_SERVER_MSG_MAGIC_CODE       0x0001
#define P2P_SERVER_MSG_HEADER_LEN_MAX   sizeof(P2P_SERVER_MSG_HEADER_t)
#define P2P_SERVER_PAYLOAD_LEN_MAX      (P2P_PACKET_SIZE_MAX - P2P_SERVER_MSG_HEADER_LEN_MAX)

/* P2P server communication port */
#define P2P_SERVER_COMM_PORT            5555

#define RETURN_IF_INVLD_CLIENT_ID(clientIdx) \
    if (clientIdx >= P2P_CLIENT_SUPPORT_MAX) {EPRINT(P2P_MODULE, "invld client session: [client=%d]", clientIdx); return;}

#define RETURN_FALSE_IF_INVLD_CLIENT_ID(clientIdx) \
    if (clientIdx >= P2P_CLIENT_SUPPORT_MAX) {EPRINT(P2P_MODULE, "invld client session: [client=%d]", clientIdx); return CLIENT_PROC_STAT_RUNNING;}

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
    P2P_PROC_STATE_REGISTER = 0,
    P2P_PROC_STATE_HEARTBEAT,
    P2P_PROC_STATE_MAX

}P2P_PROC_STATE_e;

typedef enum
{
    PROC_FSM_EVENT_MSG_RECV = 0,
    PROC_FSM_EVENT_REG_INIT_WAIT_TMR_EXP,
    PROC_FSM_EVENT_CONNECT_WAIT_TMR_EXP,
    PROC_FSM_EVENT_REG_REQ_START_TMR_EXP,
    PROC_FSM_EVENT_REG_AUTH_WAIT_TMR_EXP,
    PROC_FSM_EVENT_REG_RESP_WAIT_TMR_EXP,
    PROC_FSM_EVENT_HEARTBEAT_RESP_WAIT_TMR_EXP,
    PROC_FSM_EVENT_HEARTBEAT_REQ_TMR_EXP,
    PROC_FSM_EVENT_STUN_RESP_WAIT_TMR_EXP,
    PROC_FSM_EVENT_CONN_DETAIL_RESP_WAIT_TMR_EXP,
    PROC_FSM_EVENT_CLIENT_STATUS_CHECK_TMR_EXP,
    PROC_FSM_EVENT_RELAY_ADDR_STATUS_CHECK_TMR_EXP,
    PROC_FSM_EVENT_RELAY_DETAIL_RESP_WAIT_TMR_EXP,
    PROC_FSM_EVENT_MAX

}PROC_FSM_EVENT_e;

typedef struct
{
    UINT16 magicCode;
    UINT16 payloadLen;

}P2P_SERVER_MSG_HEADER_t;

typedef struct
{
    BOOL                isAllocated;
    CLIENT_PROC_STAT_e  isThreadRun;
    pthread_mutex_t     threadLock;
    SYS_TIMER_t         respTimer;
    P2P_CLIENT_INFO_t   clientInfo;

}P2P_CLIENT_SESSION_t;

typedef struct
{
    BOOL                isThreadRun;
    pthread_mutex_t     threadLock;
    P2P_PROC_STATE_e    procState;
    P2P_STATUS_e        p2pStatus;
    SYS_TIMER_t         *pSysTimerListHead;
    SYS_TIMER_t         fsmTimer;
    INT32               procSockFd;
    IP_ADDR_INFO_t      localAddr;
    IP_ADDR_INFO_t      publicAddr;
    IP_ADDR_INFO_t      p2pServerAddr;
    CHAR                p2pServerDomain[MAX_MAC_SERVER_NAME_LEN];
    CHAR                p2pDeviceIdL[P2P_DEVICE_ID_LEN_MAX];    /* In lower case */
    CHAR                p2pDeviceIdU[P2P_DEVICE_ID_LEN_MAX];    /* In upper case */
    NETWORK_PORT_e      p2pNetworkPort;
    BOOL                isRelayServerAvailable;
    DOMAIN_ADDR_INFO_t  relayServerAddr[RELAY_SERVER_SUPPORT_MAX];
    CHAR                p2pServerSessionId[P2P_RAW_SESSION_ID_LEN_MAX];
    CHAR                p2pCryptoSaltKey[CRYPTO_SALT_KEY_LEN_MAX];

}P2P_PROCESS_INFO_t;

/* P2P process fsm event callback function prototype */
typedef void (*P2P_PROC_FSM_EVT_CB)(UINT8 clientIdx, void *pFsmEvtData);

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void startP2pProcess(void);
//-------------------------------------------------------------------------------------------------
static void stopP2pProcess(void);
//-------------------------------------------------------------------------------------------------
static void pollP2pProcSocket(void);
//-------------------------------------------------------------------------------------------------
static void p2pProcessFsm(PROC_FSM_EVENT_e fsmEvent, UINT8 clientIdx, void *pEvtData);
//-------------------------------------------------------------------------------------------------
static void p2pProcRegInitTmrExpEvt(UINT8 clientIdx, void *pIsTimerExpired);
//-------------------------------------------------------------------------------------------------
static void p2pProcRegConnectWaitTmrExpEvt(UINT8 clientIdx, void *pIsTimerExpired);
//-------------------------------------------------------------------------------------------------
static void p2pProcRegReqStartTmrExpEvt(UINT8 clientIdx, void *pDummyData);
//-------------------------------------------------------------------------------------------------
static void p2pProcRegAuthWaitTmrExpEvt(UINT8 clientIdx, void *pDummyData);
//-------------------------------------------------------------------------------------------------
static void p2pProcRegisterData(UINT8 clientIdx, void *pDataBuff);
//-------------------------------------------------------------------------------------------------
static void p2pProcRegRespWaitTmrExpEvt(UINT8 clientIdx, void *pDummyData);
//-------------------------------------------------------------------------------------------------
static void p2pProcHeartbeatRespWaitTmrExpEvt(UINT8 clientIdx, void *pDummyData);
//-------------------------------------------------------------------------------------------------
static void p2pProcHeartbeatReqTmrExpEvt(UINT8 clientIdx, void *pDummyData);
//-------------------------------------------------------------------------------------------------
static void p2pProcStunConnectWaitTmrExpEvt(UINT8 clientIdx, void *pIsTimerExpired);
//-------------------------------------------------------------------------------------------------
static void p2pProcStunRespWaitTmrExpEvt(UINT8 clientIdx, void *pIsTimerExpired);
//-------------------------------------------------------------------------------------------------
static void p2pProcConnDetailRespWaitTmrExpEvt(UINT8 clientIdx, void *pDummyData);
//-------------------------------------------------------------------------------------------------
static void p2pProcClientStatusCheckTmrExpEvt(UINT8 clientIdx, void *pDummyData);
//-------------------------------------------------------------------------------------------------
static void p2pProcRelayAddrStatusCheckTmrExpEvt(UINT8 clientIdx, void *pDummyData);
//-------------------------------------------------------------------------------------------------
static void p2pProcRelayDetailRespWaitTmrExpEvt(UINT8 clientIdx, void *pIsTimerExpired);
//-------------------------------------------------------------------------------------------------
static void p2pProcHeartbeatData(UINT8 clientIdx, void *pDataBuff);
//-------------------------------------------------------------------------------------------------
static UINT8 getValidP2pClientSession(P2P_PAYLOAD_t *pPayload, P2P_FAIL_REASON_e *pFailReason);
//-------------------------------------------------------------------------------------------------
static void sendRegReqMsg(BOOL withAuthF);
//-------------------------------------------------------------------------------------------------
static BOOL parseRegRespData(CHAR *pDataBuff);
//-------------------------------------------------------------------------------------------------
static void sendHeartbeatMsg(void);
//-------------------------------------------------------------------------------------------------
static BOOL parseHeartbeatData(CHAR *pDataBuff);
//-------------------------------------------------------------------------------------------------
static void sendUnRegReqMsg(void);
//-------------------------------------------------------------------------------------------------
static void sendConnDetailMsg(P2P_CLIENT_INFO_t *pClientInfo);
//-------------------------------------------------------------------------------------------------
static void sendFallbackToRelayDetailMsg(P2P_CLIENT_INFO_t *pClientInfo);
//-------------------------------------------------------------------------------------------------
static void sendConnFailMsg(CHAR *pClientUid, P2P_FAIL_REASON_e reason);
//-------------------------------------------------------------------------------------------------
static void sendStunReqMsg(P2P_CLIENT_INFO_t *pClientInfo);
//-------------------------------------------------------------------------------------------------
static BOOL parseStunRespMsg(CHAR *pDataBuff);
//-------------------------------------------------------------------------------------------------
static void sendRelayFallbackFailMsg(CHAR *pClientUid, P2P_FAIL_REASON_e reason);
//-------------------------------------------------------------------------------------------------
static BOOL sendP2pServerMsg(INT32 sockFd);
//-------------------------------------------------------------------------------------------------
static BOOL prepareP2pMsgHeader(P2P_MSG_ID_e msgId, BOOL withAuthF, P2P_HEADER_t *p2pHeader);
//-------------------------------------------------------------------------------------------------
static BOOL isValidP2pMsgHeader(P2P_HEADER_t *p2pHeader, UINT16 publicPort, BOOL withAuthF);
//-------------------------------------------------------------------------------------------------
static void handleSocketFatalErr(void);
//-------------------------------------------------------------------------------------------------
static void startP2pProcFsmTimer(PROC_FSM_EVENT_e fsmEvent);
//-------------------------------------------------------------------------------------------------
static void startP2pClientSessionRespTimer(PROC_FSM_EVENT_e fsmEvent, UINT8 clientIdx);
//-------------------------------------------------------------------------------------------------
static void stopP2pClientSessionRespTimer(UINT8 clientIdx);
//-------------------------------------------------------------------------------------------------
static void changeP2pProcFsmState(P2P_PROC_STATE_e fsmState);
//-------------------------------------------------------------------------------------------------
static void changeP2pProcessStatus(P2P_STATUS_e p2pStatus);
//-------------------------------------------------------------------------------------------------
static BOOL getLocalIpAddr(NETWORK_PORT_e p2pNetworkPort, CHAR *ipAddrBuff, UINT16 buffLen);
//-------------------------------------------------------------------------------------------------
static void p2pProcFsmTmrExpHandler(UINT32 appData1, UINT32 appData2, BOOL isTimerExpired);
//-------------------------------------------------------------------------------------------------
static void generateCryptoPassword(UINT16 publicPort1, UINT16 publicPort2, CHAR *pTimestamp, CHAR *pPassword, UINT16 passLen);
//-------------------------------------------------------------------------------------------------
static void generateSaltKey(CHAR *pDeviceIdL, CHAR *pDeviceIdU, CHAR *pSaltKey, UINT8 keyLen);
//-------------------------------------------------------------------------------------------------
static void generateP2pClientSessionId(P2P_DEVICE_TYPE_e type, CHAR *pRandNum, CHAR *pSessionId, UINT8 sessionIdLen);
//-------------------------------------------------------------------------------------------------
static void getP2pClientSessionId(P2P_CLIENT_INFO_t *pClientInfo);
//-------------------------------------------------------------------------------------------------
static BOOL encodeP2pAuthPass(P2P_HEADER_t *p2pHeader, CHAR *pEncPassKey, UINT16 passKeyLen);
//-------------------------------------------------------------------------------------------------
static BOOL encodeP2pToken(UINT16 publicPort, UINT16 serverPort, CHAR *pTimestamp, CHAR *pSaltKey,
                           CHAR *pClientUid, CHAR *pDeviceUid, CHAR *pEncData, UINT16 encDataLen);
//-------------------------------------------------------------------------------------------------
static BOOL encodeP2pData(UINT16 publicPort1, UINT16 publicPort2, CHAR *pTimestamp, CHAR *pSaltKey,
                          CHAR *pPlainData, UINT16 plainDataLen, CHAR *pEncData, UINT16 encDataLen);
//-------------------------------------------------------------------------------------------------
static BOOL decodeP2pData(UINT16 publicPort1, UINT16 publicPort2, CHAR *pTimestamp, CHAR *pSaltKey,
                          CHAR *pEncData, UINT16 encDataLen, CHAR *pPlainData, UINT16 plainDataLen);
//-------------------------------------------------------------------------------------------------
static UINT8 getP2pClientIdxFromClientUid(CHAR *pClientUid);
//-------------------------------------------------------------------------------------------------
static UINT8 allocP2pClientSession(CHAR *pClientUid, UINT8 *pClientIdx, BOOL *pIsSessionAllocated);
//-------------------------------------------------------------------------------------------------
static void deallocP2pClientSession(UINT8 clientIdx);
//-------------------------------------------------------------------------------------------------
static void stopP2pClientSession(UINT8 clientIdx);
//-------------------------------------------------------------------------------------------------
static void waitToExitP2pClient(UINT8 clientIdx);
//-------------------------------------------------------------------------------------------------
static void stopP2pClientProcessThread(UINT8 clientIdx);
//-------------------------------------------------------------------------------------------------
static void stopAllP2pClientProcess(void);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
/* To store reference of ticker */
static UINT64 p2pProcFsmRefTime = 0;

static P2P_PROCESS_INFO_t       p2pProcInfo;
static P2P_MSG_INFO_t           p2pMsgInfo;
static P2P_CLIENT_SESSION_t     p2pClientSession[P2P_CLIENT_SUPPORT_MAX];
static P2P_SERVER_MSG_HEADER_t  p2pServerMsgHeader;

/* Different state string for debugging */
static const CHAR *p2pProcStateStr[P2P_PROC_STATE_MAX] = {"REGISTER", "HEARTBEAT"};
static const CHAR *p2pProcStatusStr[P2P_STATUS_MAX] = {"DISABLED", "TRYING", "CONNECTED", "NOT_CONNECTED"};

/* Different timers and counters for P2P process FSM */
static const P2P_FSM_TIMER_t p2pProcFsmTimer[PROC_FSM_EVENT_MAX] =
{
    {     0,     0 }, /* PROC_FSM_EVENT_MSG_RECV */
    {  5000,    10 }, /* PROC_FSM_EVENT_REG_INIT_WAIT_TMR_EXP */
    {   100,    50 }, /* PROC_FSM_EVENT_CONNECT_WAIT_TMR_EXP */
    {  1000,     1 }, /* PROC_FSM_EVENT_REG_REQ_START_TMR_EXP */
    { 20000,     1 }, /* PROC_FSM_EVENT_REG_AUTH_WAIT_TMR_EXP */
    { 20000,     1 }, /* PROC_FSM_EVENT_REG_RESP_WAIT_TMR_EXP */
    { 50000,     1 }, /* PROC_FSM_EVENT_HEARTBEAT_RESP_WAIT_TMR_EXP */
    { 20000,     1 }, /* PROC_FSM_EVENT_HEARTBEAT_REQ_TMR_EXP */
    {   100,    80 }, /* PROC_FSM_EVENT_STUN_RESP_WAIT_TMR_EXP */
    { 20000,     1 }, /* PROC_FSM_EVENT_CONN_DETAIL_RESP_WAIT_TMR_EXP */
    {  2000,     0 }, /* PROC_FSM_EVENT_CLIENT_STATUS_CHECK_TMR_EXP */
    {   100,     0 }, /* PROC_FSM_EVENT_RELAY_ADDR_STATUS_CHECK_TMR_EXP */
    {   100,   200 }, /* PROC_FSM_EVENT_RELAY_DETAIL_RESP_WAIT_TMR_EXP */
};

static const P2P_PROC_FSM_EVT_CB p2pProcFsmEvtCb[PROC_FSM_EVENT_MAX][P2P_PROC_STATE_MAX] =
{
    {p2pProcRegisterData,                   p2pProcHeartbeatData                    }, /* PROC_FSM_EVENT_MSG_RECV */
    {p2pProcRegInitTmrExpEvt,               NULL                                    }, /* PROC_FSM_EVENT_REG_INIT_WAIT_TMR_EXP */
    {p2pProcRegConnectWaitTmrExpEvt,        p2pProcStunConnectWaitTmrExpEvt         }, /* PROC_FSM_EVENT_CONNECT_WAIT_TMR_EXP */
    {p2pProcRegReqStartTmrExpEvt,           NULL                                    }, /* PROC_FSM_EVENT_REG_REQ_START_TMR_EXP */
    {p2pProcRegAuthWaitTmrExpEvt,           NULL                                    }, /* PROC_FSM_EVENT_REG_AUTH_WAIT_TMR_EXP */
    {p2pProcRegRespWaitTmrExpEvt,           NULL                                    }, /* PROC_FSM_EVENT_REG_RESP_WAIT_TMR_EXP */
    {NULL,                                  p2pProcHeartbeatRespWaitTmrExpEvt       }, /* PROC_FSM_EVENT_HEARTBEAT_RESP_WAIT_TMR_EXP */
    {NULL,                                  p2pProcHeartbeatReqTmrExpEvt            }, /* PROC_FSM_EVENT_HEARTBEAT_REQ_TMR_EXP */
    {NULL,                                  p2pProcStunRespWaitTmrExpEvt            }, /* PROC_FSM_EVENT_STUN_RESP_WAIT_TMR_EXP */
    {NULL,                                  p2pProcConnDetailRespWaitTmrExpEvt      }, /* PROC_FSM_EVENT_CONN_DETAIL_RESP_WAIT_TMR_EXP */
    {p2pProcClientStatusCheckTmrExpEvt,     p2pProcClientStatusCheckTmrExpEvt       }, /* PROC_FSM_EVENT_CLIENT_STATUS_CHECK_TMR_EXP */
    {p2pProcRelayAddrStatusCheckTmrExpEvt,  p2pProcRelayAddrStatusCheckTmrExpEvt    }, /* PROC_FSM_EVENT_RELAY_ADDR_STATUS_CHECK_TMR_EXP */
    {NULL,                                  p2pProcRelayDetailRespWaitTmrExpEvt     }, /* PROC_FSM_EVENT_RELAY_DETAIL_RESP_WAIT_TMR_EXP */
};

//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   Initialization of p2p process info
 */
void InitP2pProcInfo(void)
{
    UINT8               byteCnt;
    MAC_SERVER_CNFG_t   macServerCnfg;

    /* Init with default values */
    p2pProcInfo.isThreadRun = FALSE;
    MUTEX_INIT(p2pProcInfo.threadLock, NULL);
    p2pProcInfo.procState = P2P_PROC_STATE_REGISTER;
    p2pProcInfo.procSockFd = INVALID_CONNECTION;
    p2pProcInfo.p2pStatus = P2P_STATUS_DISABLED;
    p2pServerMsgHeader.magicCode = P2P_SERVER_MSG_MAGIC_CODE;

    /* Get MAC server config for P2P server domain name */
    ReadMacServerConfig(&macServerCnfg);

    /* Set P2P server domain name and other network information */
    snprintf(p2pProcInfo.p2pServerDomain, sizeof(p2pProcInfo.p2pServerDomain), "%s", macServerCnfg.name);
    GetMacAddr(LAN1_PORT, p2pProcInfo.p2pDeviceIdL);
    p2pProcInfo.p2pNetworkPort = NETWORK_PORT_MAX;

    /* Convert P2P device ID in upper case */
    for (byteCnt = 0; byteCnt < strlen(p2pProcInfo.p2pDeviceIdL); byteCnt++)
    {
        p2pProcInfo.p2pDeviceIdU[byteCnt] = toupper(p2pProcInfo.p2pDeviceIdL[byteCnt]);
    }

    /* Append the null in upper device id */
    p2pProcInfo.p2pDeviceIdU[byteCnt] = '\0';

    /* Generate salt for encryption and decryption */
    generateSaltKey(p2pProcInfo.p2pDeviceIdL, p2pProcInfo.p2pDeviceIdU, p2pProcInfo.p2pCryptoSaltKey, sizeof(p2pProcInfo.p2pCryptoSaltKey));

    /* Init P2P client info */
    InitP2pClientCommInfo();
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get P2P process current status
 * @return  P2P status
 */
P2P_STATUS_e GetP2pProcessStatus(void)
{
    /* Get P2P process status */
    return p2pProcInfo.p2pStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get p2p network interface port on which P2P is running
 * @return  P2P interface port
 */
NETWORK_PORT_e GetP2pNetworkPort(void)
{
    /* Get P2P network interface port */
    return p2pProcInfo.p2pNetworkPort;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get P2P device ID
 * @return  Device ID
 */
const CHAR *GetP2pDeviceId(void)
{
    /* Return P2P device ID */
    return p2pProcInfo.p2pDeviceIdU;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   P2P process thread
 * @param   threadInfo
 * @return  NULL
 */
void *P2pProcessThreadRoutine(void *threadInfo)
{
    /* Set thread name */
    THREAD_START("P2P_PROCESS");

    /* Start P2P process */
    startP2pProcess();

    /* It will exit when external trigger receives */
    while(GetP2pProcessThreadRunStatus())
    {
        /* Wait for next tick */
        WaitNextTick(P2P_CLIENT_SUPPORT_MAX, &p2pProcFsmRefTime, P2P_PROC_TMR_TICK_RESOL_IN_NS, P2P_MAX_SLEEP_DELAY_CNT_IN_MS);

        /* Give tick to timer */
        SysTimerTick(&p2pProcInfo.pSysTimerListHead);

        /* Poll for socket data */
        pollP2pProcSocket();
    }

    /* Stop P2P process */
    stopP2pProcess();

    /* Exit from thread */
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set P2P process thread run status
 */
void SetP2pProcessThreadRunStatus(BOOL runStatus)
{
    /* Lock P2P process thread status variable */
    MUTEX_LOCK(p2pProcInfo.threadLock);

    /* Exit from thread */
    p2pProcInfo.isThreadRun = runStatus;

    /* Unlock P2P process thread status variable */
    MUTEX_UNLOCK(p2pProcInfo.threadLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get current thread run status
 * @return  Returns status
 */
BOOL GetP2pProcessThreadRunStatus(void)
{
    BOOL runStatus;

    /* Lock P2P process thread status variable */
    MUTEX_LOCK(p2pProcInfo.threadLock);

    /* Exit from thread */
    runStatus = p2pProcInfo.isThreadRun;

    /* Unlock P2P process thread status variable */
    MUTEX_UNLOCK(p2pProcInfo.threadLock);

    return runStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get P2P client process running status
 * @param   clientIdx
 * @return
 */
CLIENT_PROC_STAT_e GetP2pClientProcessRunStatus(UINT8 clientIdx)
{
    /* Return if client session index is invalid */
    RETURN_FALSE_IF_INVLD_CLIENT_ID(clientIdx);

    /* Protect the thread variable and get the status */
    MUTEX_LOCK(p2pClientSession[clientIdx].threadLock);
    CLIENT_PROC_STAT_e status = p2pClientSession[clientIdx].isThreadRun;
    MUTEX_UNLOCK(p2pClientSession[clientIdx].threadLock);
    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set P2P client process running status
 * @param   clientIdx
 * @param   status
 */
void SetP2pClientProcessRunStatus(UINT8 clientIdx, CLIENT_PROC_STAT_e status)
{
    /* Return if client session index is invalid */
    RETURN_IF_INVLD_CLIENT_ID(clientIdx);

    /* Protect the thread variable and update the status */
    MUTEX_LOCK(p2pClientSession[clientIdx].threadLock);
    p2pClientSession[clientIdx].isThreadRun = status;
    MUTEX_UNLOCK(p2pClientSession[clientIdx].threadLock);
    DPRINT(P2P_MODULE, "client session status: [client=%d], [model=%s], [status=%d]", clientIdx, p2pClientSession[clientIdx].clientInfo.model, status);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Start P2P process
 */
static void startP2pProcess(void)
{
    UINT8                   clientIdx;
    STATIC_ROUTING_CONFIG_t staticRoutingConfig = {0};

    /* Read configuration of static routing */
    ReadStaticRoutingConfig(&staticRoutingConfig);

    /* Set Default P2P process FSM state */
    changeP2pProcFsmState(P2P_PROC_STATE_REGISTER);

    /* Set default values */
    p2pProcFsmRefTime = 0;
    RESET_STR_BUFF(p2pProcInfo.localAddr.ip);
    RESET_STR_BUFF(p2pProcInfo.p2pServerAddr.ip);
    p2pProcInfo.p2pServerAddr.port = P2P_SERVER_COMM_PORT;
    p2pProcInfo.procSockFd = INVALID_CONNECTION;
    p2pProcInfo.p2pNetworkPort = (staticRoutingConfig.defaultPort - 1);
    p2pProcInfo.isRelayServerAvailable = FALSE;
    memset(p2pProcInfo.relayServerAddr, 0, sizeof(p2pProcInfo.relayServerAddr));

    /* Init timer object */
    SysTimerInit(&p2pProcInfo.pSysTimerListHead);

    /* Configure P2P process FSM timer */
    if (FALSE == SysTimerConfigure(&p2pProcInfo.fsmTimer, P2P_PROC_TMR_TICK_RESOL_IN_MS, p2pProcFsmTmrExpHandler))
    {
        /* Failed to configure P2P process FSM timer */
        EPRINT(P2P_MODULE, "fail to config p2p process fsm timer");
        return;
    }

    /* Reset client connection information */
    memset(p2pClientSession, 0, sizeof(p2pClientSession));

    for (clientIdx = 0; clientIdx < P2P_CLIENT_SUPPORT_MAX; clientIdx++)
    {
        /* Set default client connection info */
        p2pClientSession[clientIdx].isAllocated = FALSE;
        p2pClientSession[clientIdx].isThreadRun = CLIENT_PROC_STAT_EXIT;
        p2pClientSession[clientIdx].clientInfo.clientIdx = clientIdx;
        p2pClientSession[clientIdx].clientInfo.sockFd = INVALID_CONNECTION;
        p2pClientSession[clientIdx].clientInfo.fallbackToRelayServer = FALSE;
        p2pClientSession[clientIdx].clientInfo.errorCode = 0;
        p2pClientSession[clientIdx].clientInfo.isPeerRelayAddrAllocated = FALSE;
        p2pClientSession[clientIdx].clientInfo.pRelayServerAddr = p2pProcInfo.relayServerAddr;
        MUTEX_INIT(p2pClientSession[clientIdx].threadLock, NULL);

        /* Configure P2P client connection response timer */
        if (FALSE == SysTimerConfigure(&p2pClientSession[clientIdx].respTimer, P2P_PROC_TMR_TICK_RESOL_IN_MS, p2pProcFsmTmrExpHandler))
        {
            /* Failed to configure P2P process FSM timer */
            EPRINT(P2P_MODULE, "fail to config client conn resp timer: [client=%d]", clientIdx);
            return;
        }
    }

    /* Start P2P FSM Timer */
    startP2pProcFsmTimer(PROC_FSM_EVENT_REG_INIT_WAIT_TMR_EXP);

    /* Change P2P status */
    changeP2pProcessStatus(P2P_STATUS_TRYING);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Stop P2P process
 * @note    No need to stop timers because all timers are managed locally.
 */
static void stopP2pProcess(void)
{
    P2P_CONFIG_t p2PConfig;

    /* Read P2P config */
    ReadP2PConfig(&p2PConfig);

    /* Log event based on configuration. It may called on system reboot */
    if (p2PConfig.p2pEnable == DISABLE)
    {
        /* Set P2P status not connected */
        changeP2pProcessStatus(P2P_STATUS_DISABLED);

        /* Send Un-register request to P2P server as P2P is disabled */
        sendUnRegReqMsg();

        /* Set Default P2P process FSM state */
        changeP2pProcFsmState(P2P_PROC_STATE_REGISTER);
    }

    /* Close P2P process socket */
    CloseSocket(&p2pProcInfo.procSockFd);

    /* Stop all running clients */
    stopAllP2pClientProcess();
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Restart P2P process from beginning
 */
static void restartP2pPocess(void)
{
    UINT8 clientIdx;

    /* Close P2P process socket */
    CloseSocket(&p2pProcInfo.procSockFd);

    /* Change P2P process FSM state */
    changeP2pProcFsmState(P2P_PROC_STATE_REGISTER);

    /* Start P2P FSM Timer */
    startP2pProcFsmTimer(PROC_FSM_EVENT_REG_INIT_WAIT_TMR_EXP);

    /* Stop clients who are in intermediate stage */
    for (clientIdx = 0; clientIdx < P2P_CLIENT_SUPPORT_MAX; clientIdx++)
    {
        /* Nothing to do if client is running */
        if (CLIENT_PROC_STAT_RUNNING == GetP2pClientProcessRunStatus(clientIdx))
        {
            continue;
        }

        /* Stop intermediate client */
        stopP2pClientSession(clientIdx);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Poll socket for data. If data available on socket then receive it and send it to FSM directly.
 */
static void pollP2pProcSocket(void)
{
    BOOL                            retSts;
    UINT8                           pktCnt;
    UINT32                          recvLen;
    UINT16                          payloadLen;
    static CHAR                     recvBuff[P2P_PACKET_SIZE_MAX+1];
    static UINT8                    dataWaitRetryCnt = 0;
    static P2P_SERVER_MSG_HEADER_t  p2pServerMsgHeader = {.magicCode = 0, .payloadLen = 0};

    /* Validate socket fd */
    if (p2pProcInfo.procSockFd == INVALID_CONNECTION)
    {
        /* FD is not valid. Hence no need to poll FDs */
        return;
    }

    /* Receive multiple packets; It is required when multiple clients request simultaneously */
    for (pktCnt = 0; pktCnt < P2P_CLIENT_SUPPORT_MAX; pktCnt++)
    {
        /* Get TCP header before reading any data */
        if (p2pServerMsgHeader.payloadLen == 0)
        {
            /* Get available bytes on socket */
            if (GetAvailableBytesOnSocket(p2pProcInfo.procSockFd) < (INT32)P2P_SERVER_MSG_HEADER_LEN_MAX)
            {
                /* Required data not available */
                return;
            }

            /* Receive P2P TCP header */
            retSts = RecvP2pMsg(p2pProcInfo.procSockFd, (CHAR *)&p2pServerMsgHeader, P2P_SERVER_MSG_HEADER_LEN_MAX, &recvLen);

            /* No data available on socket */
            if (retSts == IN_PROGRESS)
            {
                return;
            }

            /* Receive Msg Failed */
            if (retSts == FAIL)
            {
                EPRINT(P2P_MODULE, "fail to recv header for P2P process");
                return;
            }

            /* Remote connection closed */
            if (retSts == REFUSE)
            {
                EPRINT(P2P_MODULE, "remote connection closed");
                handleSocketFatalErr();
                return;
            }

            /* Validate magic code */
            if (p2pServerMsgHeader.magicCode != P2P_SERVER_MSG_MAGIC_CODE)
            {
                EPRINT(P2P_MODULE, "invld p2p server magic code: [magicCode=0x%x]", p2pServerMsgHeader.magicCode);
                return;
            }

            /* Validate payload length */
            if ((p2pServerMsgHeader.payloadLen == 0) || (p2pServerMsgHeader.payloadLen > P2P_SERVER_PAYLOAD_LEN_MAX))
            {
                p2pServerMsgHeader.payloadLen = 0;
                dataWaitRetryCnt = 0;
                EPRINT(P2P_MODULE, "invld p2p server msg payload length: [payloadLen=%d]", p2pServerMsgHeader.payloadLen);
                return;
            }
        }

        /* Get available bytes on socket */
        if (GetAvailableBytesOnSocket(p2pProcInfo.procSockFd) < p2pServerMsgHeader.payloadLen)
        {
            /* Required data not available. Retry for data */
            dataWaitRetryCnt++;
            if (dataWaitRetryCnt > P2P_TCP_HEADER_DATA_RETRY_MAX)
            {
                /* No data available till timeout */
                p2pServerMsgHeader.payloadLen = 0;
                dataWaitRetryCnt = 0;
            }
            return;
        }

        /* Required data found on socket */
        payloadLen = p2pServerMsgHeader.payloadLen;
        p2pServerMsgHeader.payloadLen = 0;
        dataWaitRetryCnt = 0;

        /* Receive data from socket */
        retSts = RecvP2pMsg(p2pProcInfo.procSockFd, recvBuff, payloadLen, &recvLen);

        /* No data available on socket */
        if (retSts == IN_PROGRESS)
        {
            return;
        }

        /* Receive Msg Failed */
        if (retSts == FAIL)
        {
            EPRINT(P2P_MODULE, "fail to recv msg for P2P process");
            return;
        }

        /* Remote connection closed */
        if (retSts == REFUSE)
        {
            EPRINT(P2P_MODULE, "remote connection closed");
            handleSocketFatalErr();
            return;
        }

        if (payloadLen != recvLen)
        {
            EPRINT(P2P_MODULE, "truncate p2p msg recv: [length=%d-->%d]", payloadLen, recvLen);
            return;
        }

        /* Add null to terminate string buffer */
        recvBuff[recvLen] = '\0';
        p2pProcessFsm(PROC_FSM_EVENT_MSG_RECV, P2P_CLIENT_SUPPORT_MAX, recvBuff);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   P2P process FSM
 * @param   fsmEvent - Responsible for FSM processing
 * @param   clientIdx - Client session index
 * @param   pEvtData - FSM event data
 */
static void p2pProcessFsm(PROC_FSM_EVENT_e fsmEvent, UINT8 clientIdx, void *pEvtData)
{
    /* Validate FSM state */
    if (p2pProcInfo.procState >= P2P_PROC_STATE_MAX)
    {
        EPRINT(P2P_MODULE, "invld fsm state: [fsmState=%d], [fsmEvent=%d], [client=%d]", p2pProcInfo.procState, fsmEvent, clientIdx);
        return;
    }

    /* Validate FSM event */
    if (fsmEvent >= PROC_FSM_EVENT_MAX)
    {
        EPRINT(P2P_MODULE, "invld fsm event: [fsmState=%d], [fsmEvent=%d], [client=%d]", p2pProcInfo.procState, fsmEvent, clientIdx);
        return;
    }

    /* Do we have to handle this event? */
    if (NULL == p2pProcFsmEvtCb[fsmEvent][p2pProcInfo.procState])
    {
        EPRINT(P2P_MODULE, "unhandled fsm event: [fsmState=%d], [fsmEvent=%d], [client=%d]", p2pProcInfo.procState, fsmEvent, clientIdx);
        return;
    }

    /* Invoke event callback function to process event data */
    p2pProcFsmEvtCb[fsmEvent][p2pProcInfo.procState](clientIdx, pEvtData);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Start registration process if all requirement met
 * @param   clientIdx
 * @param   pIsTimerExpired
 */
static void p2pProcRegInitTmrExpEvt(UINT8 clientIdx, void *pIsTimerExpired)
{
    /* Check timer expired or not */
    if (TRUE == *(BOOL*)pIsTimerExpired)
    {
        /* Start this P2P FSM Timer again */
        startP2pProcFsmTimer(PROC_FSM_EVENT_REG_INIT_WAIT_TMR_EXP);

        /* Reset resolved IP address. It may change if dynamic. */
        RESET_STR_BUFF(p2pProcInfo.p2pServerAddr.ip);

        /* Check internet connectivity is available or not */
        if (ACTIVE !=  getInternetConnStatus())
        {
            /* Internet is not available */
            WPRINT(P2P_MODULE, "internet connectivity is not available");
        }
    }

    /* Check internet connectivity is available or not */
    if (ACTIVE !=  getInternetConnStatus())
    {
        return;
    }

    /* Resolve domain if we have not resolved yet and Get IP Address from domain */
    DPRINT(P2P_MODULE, "p2p registration process started: [server=%s:%d]", p2pProcInfo.p2pServerAddr.ip, p2pProcInfo.p2pServerAddr.port);
    if ((p2pProcInfo.p2pServerAddr.ip[0] == '\0') &&
            (FAIL == GetIpAddrFromDomainName(p2pProcInfo.p2pServerDomain, IP_ADDR_TYPE_IPV4, p2pProcInfo.p2pServerAddr.ip)))
    {
        /* Failed to resolve domain */
        EPRINT(P2P_MODULE, "fail to resolve domain: [addr=%s]", p2pProcInfo.p2pServerDomain);
        return;
    }

    /* Get Local Ip Address for P2P */
    if ((FALSE == getLocalIpAddr(p2pProcInfo.p2pNetworkPort, p2pProcInfo.localAddr.ip, sizeof(p2pProcInfo.localAddr.ip)))
            || (p2pProcInfo.localAddr.ip[0] == '\0'))
    {
        EPRINT(P2P_MODULE, "fail to get local ip address: [p2pNetworkPort=%d]", p2pProcInfo.p2pNetworkPort);
        return;
    }

    /* Open P2P socket and bind with local address and random port */
    p2pProcInfo.localAddr.port = 0;
    if (FALSE == OpenP2pSocket(FALSE, &p2pProcInfo.localAddr, &p2pProcInfo.procSockFd))
    {
        EPRINT(P2P_MODULE, "fail to create p2p socket");
        return;
    }

    /* Connect P2P socket for one-to-one connection */
    BOOL retSts = ConnectP2pSocket(p2pProcInfo.procSockFd, &p2pProcInfo.p2pServerAddr);
    if (FALSE == retSts)
    {
        EPRINT(P2P_MODULE, "fail to connect p2p socket");
        CloseSocket(&p2pProcInfo.procSockFd);
    }
    else if (IN_PROGRESS == retSts)
    {
        DPRINT(P2P_MODULE, "registration connection in progress: [local=%s:%d], [remote=%s:%d]",
               p2pProcInfo.localAddr.ip, p2pProcInfo.localAddr.port, p2pProcInfo.p2pServerAddr.ip, p2pProcInfo.p2pServerAddr.port);

        /* Start P2P FSM Timer */
        startP2pProcFsmTimer(PROC_FSM_EVENT_CONNECT_WAIT_TMR_EXP);
    }
    else
    {
        DPRINT(P2P_MODULE, "registration connection created: [local=%s:%d], [remote=%s:%d]",
               p2pProcInfo.localAddr.ip, p2pProcInfo.localAddr.port, p2pProcInfo.p2pServerAddr.ip, p2pProcInfo.p2pServerAddr.port);

        /* Reset previous stored info */
        RESET_STR_BUFF(p2pProcInfo.p2pServerSessionId);

        /* Start P2P FSM Timer */
        startP2pProcFsmTimer(PROC_FSM_EVENT_REG_REQ_START_TMR_EXP);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Wait for connection establish with P2P server
 * @param   clientIdx
 * @param   pIsTimerExpired
 */
static void p2pProcRegConnectWaitTmrExpEvt(UINT8 clientIdx, void *pIsTimerExpired)
{
    /* Check status of connection */
    BOOL retSts = IsP2pConnectionEstablished(p2pProcInfo.procSockFd);
    if (TRUE == retSts)
    {
        /* Start P2P FSM Timer. As connection established */
        startP2pProcFsmTimer(PROC_FSM_EVENT_REG_REQ_START_TMR_EXP);
        return;
    }

    /* Check timer expired or not */
    if (FALSE == *(BOOL*)pIsTimerExpired)
    {
        /* Wait for connection establishment */
        if (IN_PROGRESS == retSts)
        {
            /* Wait till time expired */
            return;
        }

        /* Connection failed with P2P server */
        EPRINT(P2P_MODULE, "connect fail with p2p server");
    }
    else
    {
        /* Connection not received till timeout */
        EPRINT(P2P_MODULE, "connect timeout with p2p server");
    }

    /* Reinit P2P process */
    restartP2pPocess();
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send registration message and wait for response
 * @param   clientIdx
 * @param   pDummyData
 */
static void p2pProcRegReqStartTmrExpEvt(UINT8 clientIdx, void *pDummyData)
{
    /* Send Registration request without authentication */
    sendRegReqMsg(FALSE);

    /* Start P2P FSM Timer */
    startP2pProcFsmTimer(PROC_FSM_EVENT_REG_AUTH_WAIT_TMR_EXP);

    /* Change P2P status */
    changeP2pProcessStatus(P2P_STATUS_TRYING);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Register authentication message wait timer expired
 * @param   clientIdx
 * @param   pDummyData
 */
static void p2pProcRegAuthWaitTmrExpEvt(UINT8 clientIdx, void *pDummyData)
{
    /* Response not received till timeout */
    EPRINT(P2P_MODULE, "resp recv timeout: register without auth");

    /* Reinit P2P process */
    restartP2pPocess();

    /* Change P2P status */
    changeP2pProcessStatus(P2P_STATUS_NOT_CONNECTED);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Registration with auth response wait timer expired
 * @param   clientIdx
 * @param   pDummyData
 */
static void p2pProcRegRespWaitTmrExpEvt(UINT8 clientIdx, void *pDummyData)
{
    /* Response not received till timeout */
    EPRINT(P2P_MODULE, "resp recv timeout: register with auth");

    /* Reinit P2P process */
    restartP2pPocess();

    /* Change P2P status */
    changeP2pProcessStatus(P2P_STATUS_NOT_CONNECTED);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Process response against register request
 * @param   clientIdx
 * @param   pDataBuff
 */
static void p2pProcRegisterData(UINT8 clientIdx, void *pDataBuff)
{
    /* Receive register reponse message */
    if (FALSE == parseRegRespData((CHAR*)pDataBuff))
    {
        /* Invld message received */
        EPRINT(P2P_MODULE, "valid resp not recv: register");
        return;
    }

    /* Check registration response message id */
    if (p2pMsgInfo.header.msgId == P2P_MSG_ID_RESP_AUTHENTICATION)
    {
        /* Store public endpoint information */
        p2pProcInfo.publicAddr = p2pMsgInfo.payload.publicAddr;

        /* Send Registration request with authentication */
        sendRegReqMsg(TRUE);

        /* Start P2P FSM Timer */
        startP2pProcFsmTimer(PROC_FSM_EVENT_REG_RESP_WAIT_TMR_EXP);

        /* Change P2P status */
        changeP2pProcessStatus(P2P_STATUS_TRYING);
    }
    else if (p2pMsgInfo.header.msgId == P2P_MSG_ID_RESP_REGISTER_FAIL)
    {
        /* Registration failed */
        EPRINT(P2P_MODULE, "registration failed: [reason=%d]", p2pMsgInfo.payload.reason);

        /* Reinit P2P process */
        restartP2pPocess();

        /* Change P2P status */
        changeP2pProcessStatus(P2P_STATUS_TRYING);
    }
    else
    {
        UINT8 relayServerCnt;

        /* Stored received relay server address */
        p2pProcInfo.isRelayServerAvailable = FALSE;
        memcpy(p2pProcInfo.relayServerAddr, p2pMsgInfo.payload.relayAddr, sizeof(p2pProcInfo.relayServerAddr));
        for (relayServerCnt = 0; relayServerCnt < RELAY_SERVER_SUPPORT_MAX; relayServerCnt++)
        {
            if ((p2pProcInfo.relayServerAddr[relayServerCnt].domain[0] != '\0') && (p2pProcInfo.relayServerAddr[relayServerCnt].port))
            {
                p2pProcInfo.isRelayServerAvailable = TRUE;
                DPRINT(P2P_MODULE, "relay server-%d info: [addr=%s:%d]", relayServerCnt+1,
                       p2pProcInfo.relayServerAddr[relayServerCnt].domain, p2pProcInfo.relayServerAddr[relayServerCnt].port);
            }
        }

        /* Send heartbeat message */
        sendHeartbeatMsg();

        /* Change P2P process FSM state */
        changeP2pProcFsmState(P2P_PROC_STATE_HEARTBEAT);

        /* Start P2P FSM Timer */
        startP2pProcFsmTimer(PROC_FSM_EVENT_HEARTBEAT_RESP_WAIT_TMR_EXP);

        /* Change P2P status */
        changeP2pProcessStatus(P2P_STATUS_CONNECTED);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   P2P heartbeat response not received till timer expired
 * @param   clientIdx
 * @param   pDummyData
 */
static void p2pProcHeartbeatRespWaitTmrExpEvt(UINT8 clientIdx, void *pDummyData)
{
    /* Response not received till timeout */
    EPRINT(P2P_MODULE, "resp recv timeout: heartbeat");

    /* Reinit P2P process */
    restartP2pPocess();

    /* Change P2P status */
    changeP2pProcessStatus(P2P_STATUS_NOT_CONNECTED);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   P2P request send timer expired. Hence send it.
 * @param   clientIdx
 * @param   pDummyData
 */
static void p2pProcHeartbeatReqTmrExpEvt(UINT8 clientIdx, void *pDummyData)
{
    /* Send heartbeat message */
    sendHeartbeatMsg();

    /* Start P2P FSM Timer */
    startP2pProcFsmTimer(PROC_FSM_EVENT_HEARTBEAT_RESP_WAIT_TMR_EXP);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Wait for connection with STUN server
 * @param   clientIdx
 * @param   pIsTimerExpired
 */
static void p2pProcStunConnectWaitTmrExpEvt(UINT8 clientIdx, void *pIsTimerExpired)
{
    /* Return if client session index is invalid */
    RETURN_IF_INVLD_CLIENT_ID(clientIdx);

    /* Check status of connection */
    BOOL retSts = IsP2pConnectionEstablished(p2pClientSession[clientIdx].clientInfo.sockFd);
    if (TRUE == retSts)
    {
        /* Send STUN query */
        sendStunReqMsg(&p2pClientSession[clientIdx].clientInfo);

        /* Start P2P STUN response wait timer */
        startP2pClientSessionRespTimer(PROC_FSM_EVENT_STUN_RESP_WAIT_TMR_EXP, clientIdx);
        return;
    }

    /* Check timer expired or not */
    if (FALSE == *(BOOL*)pIsTimerExpired)
    {
        /* Wait for connection establishment */
        if (IN_PROGRESS == retSts)
        {
            /* Wait till time expired */
            return;
        }

        /* Connection failed with stun server */
        EPRINT(P2P_MODULE, "stun connection fail: [client=%d], [clientUid=%s]", clientIdx, p2pClientSession[clientIdx].clientInfo.clientUid);
    }
    else
    {
        /* Connection not received till timeout */
        EPRINT(P2P_MODULE, "stun connection timeout: [client=%d], [clientUid=%s]", clientIdx, p2pClientSession[clientIdx].clientInfo.clientUid);
    }

    /* Send STUN query fail response */
    sendConnFailMsg(p2pClientSession[clientIdx].clientInfo.clientUid, CONN_FAIL_STUN_NOT_RESOLVE);

    /* Stop P2P client session as failed to resolve STUN */
    stopP2pClientSession(clientIdx);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   P2P stun response check timer expired.
 * @param   clientIdx
 * @param   pIsTimerExpired
 */
void p2pProcStunRespWaitTmrExpEvt(UINT8 clientIdx, void *pIsTimerExpired)
{
    BOOL                    retSts;
    UINT32                  recvLen;
    CHAR                    recvBuff[P2P_PACKET_SIZE_MAX+1];
    P2P_SERVER_MSG_HEADER_t p2pServerMsgHeader = {.magicCode = 0, .payloadLen = 0};

    /* Return if client session index is invalid */
    RETURN_IF_INVLD_CLIENT_ID(clientIdx);

    /* Check timer expired or not */
    if (TRUE == *(BOOL*)pIsTimerExpired)
    {
        /* Stun query response not received */
        EPRINT(P2P_MODULE, "resp recv timeout: stun query: [client=%d], [clientUid=%s]", clientIdx, p2pClientSession[clientIdx].clientInfo.clientUid);

        /* Send STUN query fail response */
        sendConnFailMsg(p2pClientSession[clientIdx].clientInfo.clientUid, CONN_FAIL_STUN_NOT_RESOLVE);

        /* Stop P2P client session as failed to resolve STUN */
        stopP2pClientSession(clientIdx);

        /* Wait till time expired */
        return;
    }

    /* Validate socket fd */
    P2P_CLIENT_INFO_t *pClientInfo = &p2pClientSession[clientIdx].clientInfo;
    if (pClientInfo->sockFd == INVALID_CONNECTION)
    {
        /* FD is not valid. Hence no need to poll FDs */
        EPRINT(P2P_MODULE, "invld fd found: [client=%d]", clientIdx);
        return;
    }

    /* Get available bytes on socket */
    if (GetAvailableBytesOnSocket(pClientInfo->sockFd) <= (INT32)P2P_SERVER_MSG_HEADER_LEN_MAX)
    {
        /* Required data not available */
        return;
    }

    retSts = RecvP2pMsg(pClientInfo->sockFd, (CHAR *)&p2pServerMsgHeader, P2P_SERVER_MSG_HEADER_LEN_MAX, &recvLen);

    /* Not data received yet */
    if (retSts == IN_PROGRESS)
    {
        /* No data available on socket */
        return;
    }

    /* Socket closed */
    if (retSts == REFUSE)
    {
        /* Remote connection closed */
        EPRINT(P2P_MODULE, "remote connection closed: [client=%d]", clientIdx);

        /* Send STUN query fail response */
        sendConnFailMsg(pClientInfo->clientUid, CONN_FAIL_STUN_NOT_RESOLVE);

        /* Stop P2P client session as STUN socket closed */
        stopP2pClientSession(clientIdx);
        return;
    }

    /* Receive failed */
    if (retSts == FAIL)
    {
        /* Receive Msg Failed */
        EPRINT(P2P_MODULE, "fail to recv resp: stun query: [client=%d]", clientIdx);
        return;
    }

    /* Validate magic code */
    if (p2pServerMsgHeader.magicCode != P2P_SERVER_MSG_MAGIC_CODE)
    {
        EPRINT(P2P_MODULE, "invld p2p server magic code: [client=%d], [magicCode=0x%x]", clientIdx, p2pServerMsgHeader.magicCode);
        return;
    }

    /* Receive STUN query response */
    retSts = RecvP2pMsg(pClientInfo->sockFd, recvBuff, p2pServerMsgHeader.payloadLen, &recvLen);

    /* Not data received yet */
    if (retSts == IN_PROGRESS)
    {
        /* No data available on socket */
        return;
    }

    /* Socket closed */
    if (retSts == REFUSE)
    {
        /* Remote connection closed */
        EPRINT(P2P_MODULE, "remote connection closed: [client=%d]", clientIdx);

        /* Send STUN query fail response */
        sendConnFailMsg(pClientInfo->clientUid, CONN_FAIL_STUN_NOT_RESOLVE);

        /* Stop P2P client session as STUN socket closed */
        stopP2pClientSession(clientIdx);
        return;
    }

    /* Receive failed */
    if (retSts == FAIL)
    {
        /* Receive Msg Failed */
        EPRINT(P2P_MODULE, "fail to recv resp: stun query: [client=%d]", clientIdx);
        return;
    }

    /* Check required length is received or not */
    if (p2pServerMsgHeader.payloadLen != recvLen)
    {
        EPRINT(P2P_MODULE, "truncate stun msg recv: [client=%d], [length=%d-->%d]", clientIdx, p2pServerMsgHeader.payloadLen, recvLen);
        return;
    }

    /* Add null to terminate string buffer */
    recvBuff[recvLen] = '\0';
    if (FALSE == parseStunRespMsg(recvBuff))
    {
        /* Receive Msg Failed */
        EPRINT(P2P_MODULE, "valid resp not recv: stun query: [client=%d]", clientIdx);
        return;
    }

    /* Store device public end-points */
    pClientInfo->publicAddr = p2pMsgInfo.payload.publicAddr;

    /* Send P2P Connection Details Message */
    sendConnDetailMsg(pClientInfo);

    /* Start connection details response wait timer */
    startP2pClientSessionRespTimer(PROC_FSM_EVENT_CONN_DETAIL_RESP_WAIT_TMR_EXP, clientIdx);

    /* Close connection with P2P server */
    CloseSocket(&pClientInfo->sockFd);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   P2P client connection details resp wait timer expired
 * @param   clientIdx
 * @param   pDummyData
 */
static void p2pProcConnDetailRespWaitTmrExpEvt(UINT8 clientIdx, void *pDummyData)
{
    /* Return if client session index is invalid */
    RETURN_IF_INVLD_CLIENT_ID(clientIdx);

    /* Connection details response not recv */
    EPRINT(P2P_MODULE, "resp recv timeout: connection details: [client=%d]", clientIdx);

    /* Stop P2P client session as failed to receive response againt connnection details */
    stopP2pClientSession(clientIdx);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   P2P process client status check timer expired event to check client status
 * @param   clientIdx
 * @param   pDummyData
 */
static void p2pProcClientStatusCheckTmrExpEvt(UINT8 clientIdx, void *pDummyData)
{
    /* Return if client session index is invalid */
    RETURN_IF_INVLD_CLIENT_ID(clientIdx);

    /* Check client is in stop wait state */
    if (CLIENT_PROC_STAT_RUNNING == GetP2pClientProcessRunStatus(clientIdx))
    {
        /* Client is running, hence check its login session status */
        if (p2pClientSession[clientIdx].clientInfo.userLoginSessionIdx >= MAX_NW_CLIENT)
        {
            /* Client is not logged in yet */
            return;
        }

        /* Validate user login session */
        if (TRUE == IsUserSessionValid(p2pClientSession[clientIdx].clientInfo.userLoginSessionIdx,
                                       p2pClientSession[clientIdx].clientInfo.userLoginSessionUid))
        {
            /* Check client is active or not. It will be inactive after user login when forcefully logged-out by other triggers */
            if (TRUE == IsUserSessionActive(p2pClientSession[clientIdx].clientInfo.userLoginSessionIdx))
            {
                /* User session is active */
                return;
            }
        }

        /* Free user session not available */
        EPRINT(P2P_MODULE, "user login session found as inactive: [client=%d], [userLoginSessionIdx=%d], [userLoginSessionUid=%d]",
               clientIdx, p2pClientSession[clientIdx].clientInfo.userLoginSessionIdx, p2pClientSession[clientIdx].clientInfo.userLoginSessionUid);

        /* Exit from running thread */
        SetP2pClientProcessRunStatus(clientIdx, CLIENT_PROC_STAT_STOP);
    }

    /* Peer relay addr allocation failure */
    DPRINT(P2P_MODULE, "p2p client thread exited, stop client session: [client=%d]", clientIdx);

    /* Wait for client process thread to exit */
    stopP2pClientProcessThread(clientIdx);

    /* Stop P2P client session */
    stopP2pClientSession(clientIdx);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   P2P relay address allocation for peer wait timer expired
 * @param   clientIdx
 * @param   pDummyData
 */
static void p2pProcRelayAddrStatusCheckTmrExpEvt(UINT8 clientIdx, void *pDummyData)
{
    /* Return if client session index is invalid */
    RETURN_IF_INVLD_CLIENT_ID(clientIdx);

    /* Relay server related process is going on? FSM state may change due to connectivity between p2p server.
     * If connectivity lost then we don't know where to send the fallback to relay response. Hence, terminate the process. */
    if ((p2pProcInfo.procState == P2P_PROC_STATE_HEARTBEAT) && (CLIENT_PROC_STAT_RUNNING == GetP2pClientProcessRunStatus(clientIdx)))
    {
        /* Is relay addresss allocated for peer? */
        if (TRUE == p2pClientSession[clientIdx].clientInfo.isPeerRelayAddrAllocated)
        {
            /* Peer relay address allocated */
            sendFallbackToRelayDetailMsg(&p2pClientSession[clientIdx].clientInfo);

            /* Start relay details response wait timer */
            startP2pClientSessionRespTimer(PROC_FSM_EVENT_RELAY_DETAIL_RESP_WAIT_TMR_EXP, clientIdx);
        }

        /* Client process is running */
        return;
    }

    /* Peer relay addr allocation failure */
    EPRINT(P2P_MODULE, "peer relay addr alloc failure, stop client session: [client=%d]", clientIdx);

    /* Fallback to relay server considered as failed because client is exited */
    sendRelayFallbackFailMsg(p2pClientSession[clientIdx].clientInfo.clientUid, p2pClientSession[clientIdx].clientInfo.errorCode);

    /* Wait for client process thread to exit */
    stopP2pClientProcessThread(clientIdx);

    /* Stop P2P client session as failed in fallback to relay server */
    stopP2pClientSession(clientIdx);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   P2P client relay details resp wait timer expired
 * @param   clientIdx
 * @param   pIsTimerExpired
 */
static void p2pProcRelayDetailRespWaitTmrExpEvt(UINT8 clientIdx, void *pIsTimerExpired)
{
    /* Return if client session index is invalid */
    RETURN_IF_INVLD_CLIENT_ID(clientIdx);

    /* Relay server related process is going on? */
    if (CLIENT_PROC_STAT_RUNNING == GetP2pClientProcessRunStatus(clientIdx))
    {
        /* Check timer expired or not */
        if (FALSE == *(BOOL*)pIsTimerExpired)
        {
            /* Client process is running and timer is not expired */
            return;
        }
    }

    /* Relay connection details ack not received till timeout */
    EPRINT(P2P_MODULE, "fallback to relay ack not rcvd: [client=%d]", clientIdx);

    /* Wait for client process thread to exit */
    stopP2pClientProcessThread(clientIdx);

    /* Stop P2P client session as failed in fallback to relay server */
    stopP2pClientSession(clientIdx);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Process received data
 * @param   clientIdx
 * @param   pDataBuff
 */
static void p2pProcHeartbeatData(UINT8 clientIdx, void *pDataBuff)
{
    UINT8               p2pClientIdx;
    P2P_PAYLOAD_t       *pPayload = &p2pMsgInfo.payload;
    P2P_CLIENT_INFO_t   *pClientInfo = NULL;
    P2P_FAIL_REASON_e   failReason = CONN_FAIL_INVLD_DATA;

    /* Receive heartbeat or other message data */
    if (FALSE == parseHeartbeatData((CHAR*)pDataBuff))
    {
        /* Invld message received */
        EPRINT(P2P_MODULE, "valid resp not recv: heartbeat");
        return;
    }

    /* Process as per message id */
    switch(p2pMsgInfo.header.msgId)
    {
        case P2P_MSG_ID_RESP_HEARTBEAT_ACK:
        {
            /* Start P2P FSM Timer */
            startP2pProcFsmTimer(PROC_FSM_EVENT_HEARTBEAT_REQ_TMR_EXP);
        }
        break;

        case P2P_MSG_ID_RESP_HEARTBEAT_FAIL:
        {
            /* Heartbeat failed with P2P server */
            EPRINT(P2P_MODULE, "heartbeat fail: [reason=%d]", pPayload->reason);

            /* Reinit P2P process */
            restartP2pPocess();

            /* Change P2P status */
            changeP2pProcessStatus(P2P_STATUS_TRYING);
        }
        break;

        case P2P_MSG_ID_RESP_CONNECTION_REQUEST:
        {
            /* Validate p2p data and provide valid client session if communication is possible */
            p2pClientIdx = getValidP2pClientSession(pPayload, &failReason);
            if (p2pClientIdx >= P2P_CLIENT_SUPPORT_MAX)
            {
                /* Client communication is not possible */
                EPRINT(P2P_MODULE, "p2p client session not available, connection request failed: [clientUid=%s], [type=%d], [model=%s]",
                       pPayload->uId, pPayload->type, pPayload->model);
                sendConnFailMsg(pPayload->uId, failReason);
                return;
            }

            /* Store client data related data */
            pClientInfo = &p2pClientSession[p2pClientIdx].clientInfo;
            pClientInfo->clientType = pPayload->type;
            snprintf(pClientInfo->clientUid, sizeof(pClientInfo->clientUid), "%s", pPayload->uId);
            RemoveSpecialCharacters(pPayload->model, CLIENT_MODEL_SPECIAL_CHAR_LIST, pClientInfo->model);
            pClientInfo->clientLocalAddr = pPayload->localAddr;
            pClientInfo->clientPublicAddr = pPayload->publicAddr;

            DPRINT(P2P_MODULE, "new p2p connection request: [client=%d], [clientUid=%s], [model=%s], [connMode=%s]",
                   p2pClientIdx, pClientInfo->clientUid, pClientInfo->model, pPayload->connMode);

            /* Generate and get p2p client session id */
            getP2pClientSessionId(pClientInfo);

            /* Store local IP address */
            pClientInfo->localAddr = p2pProcInfo.localAddr;
            pClientInfo->localAddr.port = 0;

            /* Reset login related information */
            pClientInfo->userLoginSessionIdx = MAX_NW_CLIENT;
            pClientInfo->userLoginSessionUid = INVALID_CONNECTION;

            /* Open new socket for client */
            if (FALSE == OpenP2pSocket(FALSE, &pClientInfo->localAddr, &pClientInfo->sockFd))
            {
                /* Free user session not available */
                EPRINT(P2P_MODULE, "fail to open stun port: [client=%d]", p2pClientIdx);
                sendConnFailMsg(pClientInfo->clientUid, CONN_FAIL_RESOURCE_NOT_AVAIL);
                stopP2pClientSession(p2pClientIdx);
                return;
            }

            /* Connect P2P socket for one-to-one connection */
            BOOL retSts = ConnectP2pSocket(pClientInfo->sockFd, &p2pProcInfo.p2pServerAddr);
            if (FALSE == retSts)
            {
                /* Failed to connect with stun server */
                EPRINT(P2P_MODULE, "fail to connect p2p port for client: [client=%d]", p2pClientIdx);
                sendConnFailMsg(pClientInfo->clientUid, CONN_FAIL_STUN_NOT_RESOLVE);
                stopP2pClientSession(p2pClientIdx);
                return;
            }

            /* Wait for connection establishment */
            DPRINT(P2P_MODULE, "stun connection in progress: [client=%d], [clientUid=%s], [version=%s], [localAddr=%s:%d]",
                   p2pClientIdx, pClientInfo->clientUid, pPayload->version, pClientInfo->localAddr.ip, pClientInfo->localAddr.port);
            startP2pClientSessionRespTimer(PROC_FSM_EVENT_CONNECT_WAIT_TMR_EXP, p2pClientIdx);
        }
        break;

        case P2P_MSG_ID_RESP_FALLBACK_TO_RELAY:
        {
            P2P_CONFIG_t p2PConfig;

            /* Read P2P config */
            ReadP2PConfig(&p2PConfig);
            if ((FALSE == p2PConfig.fallbackToRelayServer) || (FALSE == p2pProcInfo.isRelayServerAvailable))
            {
                /* Client communication is not possible */
                EPRINT(P2P_MODULE, "relay server fallback is disabled: [clientUid=%s], [type=%d], [model=%s]", pPayload->uId, pPayload->type, pPayload->model);
                sendRelayFallbackFailMsg(pPayload->uId, CONN_FAIL_RELAY_FALLBACK_FAIL);
                return;
            }

            /* Validate p2p data and provide valid client session if communication is possible */
            p2pClientIdx = getValidP2pClientSession(pPayload, &failReason);
            if (p2pClientIdx >= P2P_CLIENT_SUPPORT_MAX)
            {
                /* Client communication is not possible */
                EPRINT(P2P_MODULE, "p2p client session not available, relay request failed: [clientUid=%s], [type=%d], [model=%s]",
                       pPayload->uId, pPayload->type, pPayload->model);
                sendRelayFallbackFailMsg(pPayload->uId, failReason);
                return;
            }

            /* Store client data related data */
            pClientInfo = &p2pClientSession[p2pClientIdx].clientInfo;
            pClientInfo->clientType = pPayload->type;
            snprintf(pClientInfo->clientUid, sizeof(pClientInfo->clientUid), "%s", pPayload->uId);
            RemoveSpecialCharacters(pPayload->model, CLIENT_MODEL_SPECIAL_CHAR_LIST, pClientInfo->model);
            pClientInfo->clientLocalAddr = pPayload->localAddr;
            pClientInfo->clientPublicAddr = pPayload->publicAddr;

            DPRINT(P2P_MODULE, "new p2p relay request: [client=%d], [clientUid=%s], [model=%s], [connMode=%s]",
                   p2pClientIdx, pClientInfo->clientUid, pClientInfo->model, pPayload->connMode);

            /* Generate and get p2p client session id */
            getP2pClientSessionId(pClientInfo);

            /* Store local IP and public IP address */
            pClientInfo->publicAddr = p2pProcInfo.publicAddr;
            pClientInfo->localAddr = p2pProcInfo.localAddr;
            pClientInfo->localAddr.port = 0;

            /* Reset login related information */
            pClientInfo->userLoginSessionIdx = MAX_NW_CLIENT;
            pClientInfo->userLoginSessionUid = INVALID_CONNECTION;

            /* Start P2P client thread */
            SetP2pClientProcessRunStatus(p2pClientIdx, CLIENT_PROC_STAT_RUNNING);
            pClientInfo->fallbackToRelayServer = TRUE;
            pClientInfo->isPeerRelayAddrAllocated = FALSE;
            pClientInfo->errorCode = CONN_FAIL_RELAY_FALLBACK_FAIL;
            if (FALSE == StartP2pClientCommunication(pClientInfo))
            {
                /* Failed to create thread */
                EPRINT(P2P_MODULE, "fail to create relay client thread: [client=%d]", p2pClientIdx);
                SetP2pClientProcessRunStatus(p2pClientIdx, CLIENT_PROC_STAT_EXIT);
                stopP2pClientSession(p2pClientIdx);
            }
            else
            {
                /* P2P process thread started */
                DPRINT(P2P_MODULE, "relay client thread started: [client=%d]", p2pClientIdx);

                /* Now we have to send relay address back to p2p server */
                startP2pClientSessionRespTimer(PROC_FSM_EVENT_RELAY_ADDR_STATUS_CHECK_TMR_EXP, p2pClientIdx);
            }
        }
        break;

        case P2P_MSG_ID_RESP_FALLBACK_TO_RELAY_ACK:
        {
            /* Check free P2P session available or not for client */
            p2pClientIdx = getP2pClientIdxFromClientUid(pPayload->uId);
            if (p2pClientIdx >= P2P_CLIENT_SUPPORT_MAX)
            {
                /* Free user session not available */
                EPRINT(P2P_MODULE, "client session not match: [clientUid=%s]", pPayload->uId);
                return;
            }

            /* Check client status on timer expiry */
            DPRINT(P2P_MODULE, "fallback to relay ack rcvd: [client=%d]", p2pClientIdx);
            startP2pClientSessionRespTimer(PROC_FSM_EVENT_CLIENT_STATUS_CHECK_TMR_EXP, p2pClientIdx);
        }
        break;

        case P2P_MSG_ID_RESP_CONNECTION_FAIL:
        case P2P_MSG_ID_RESP_FALLBACK_TO_RELAY_FAIL:
        {
            /* Check free P2P session available or not for client */
            p2pClientIdx = getP2pClientIdxFromClientUid(pPayload->uId);
            if (p2pClientIdx >= P2P_CLIENT_SUPPORT_MAX)
            {
                /* Free user session not available */
                EPRINT(P2P_MODULE, "client session not match for hole punching: [clientUid=%s]", pPayload->uId);
                return;
            }

            /* Stop P2P client session as failed response against connnection details */
            EPRINT(P2P_MODULE, "p2p conn fail recv for client: [client=%d], [reason=%d]", p2pClientIdx, pPayload->reason);

            /* Wait for client process thread to exit */
            stopP2pClientProcessThread(p2pClientIdx);

            /* Stop P2P client session as failed in fallback to relay server */
            stopP2pClientSession(p2pClientIdx);
        }
        break;

        case P2P_MSG_ID_RESP_CONNECTION_DETAILS_ACK:
        {
            /* Check free P2P session available or not for client */
            p2pClientIdx = getP2pClientIdxFromClientUid(pPayload->uId);
            if (p2pClientIdx >= P2P_CLIENT_SUPPORT_MAX)
            {
                /* Free user session not available */
                EPRINT(P2P_MODULE, "client session not match for hole punching: [clientUid=%s]", pPayload->uId);
                return;
            }

            /* Stop P2P client connection response timer */
            stopP2pClientSessionRespTimer(p2pClientIdx);

            /* Start hole punching process */
            DPRINT(P2P_MODULE, "start p2p hole punching: [client=%d]", p2pClientIdx);

            /* Start P2P client thread */
            pClientInfo = &p2pClientSession[p2pClientIdx].clientInfo;
            SetP2pClientProcessRunStatus(p2pClientIdx, CLIENT_PROC_STAT_RUNNING);
            pClientInfo->fallbackToRelayServer = FALSE;
            if (FALSE == StartP2pClientCommunication(pClientInfo))
            {
                /* Failed to create thread */
                EPRINT(P2P_MODULE, "fail to create p2p client thread: [client=%d]", p2pClientIdx);
                SetP2pClientProcessRunStatus(p2pClientIdx, CLIENT_PROC_STAT_EXIT);
                stopP2pClientSession(p2pClientIdx);
            }
            else
            {
                /* P2P process thread started */
                DPRINT(P2P_MODULE, "p2p client thread started: [client=%d]", p2pClientIdx);

                /* Check client status on timer expiry */
                startP2pClientSessionRespTimer(PROC_FSM_EVENT_CLIENT_STATUS_CHECK_TMR_EXP, p2pClientIdx);
            }
        }
        break;

        default:
        {
            /* Unhandled message received from server */
            DPRINT(P2P_MODULE, "unhandled msg received: [msgId=%d]", p2pMsgInfo.header.msgId);
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Validate p2p data and provide valid client session if communication is possible
 * @param   pPayload
 * @param   pFailReason
 * @return  Client session
 */
static UINT8 getValidP2pClientSession(P2P_PAYLOAD_t *pPayload, P2P_FAIL_REASON_e *pFailReason)
{
    BOOL    sessionAllocated;
    UINT8   clientIdx;

    /* Validate input payload data */
    if ((pPayload->type != P2P_DEVICE_TYPE_MOBILE_CLIENT) && (pPayload->type != P2P_DEVICE_TYPE_DEVICE_CLIENT))
    {
        /* Free user session not available */
        EPRINT(P2P_MODULE, "invld client data: [clientUid=%s], [type=%d]", pPayload->uId, pPayload->type);
        *pFailReason = CONN_FAIL_INVLD_DATA;
        return P2P_CLIENT_SUPPORT_MAX;
    }

    /* Check free client session available or not */
    if (FALSE == allocP2pClientSession(pPayload->uId, &clientIdx, &sessionAllocated))
    {
        /* Free user session not available */
        EPRINT(P2P_MODULE, "client session not available: [clientUid=%s]", pPayload->uId);
        *pFailReason = CONN_FAIL_SESSION_NOT_AVAIL;
        return P2P_CLIENT_SUPPORT_MAX;
    }

    /* Is client session already allocated and running? */
    if (FALSE == sessionAllocated)
    {
        /* Check login user session available or not for client */
        if (FALSE == IsFreeUserSessionAvailable())
        {
            /* Free user session not available */
            EPRINT(P2P_MODULE, "login session not available: [client=%d], [clientUid=%s], [model=%s]",
                   clientIdx, pPayload->uId, pPayload->model);
            deallocP2pClientSession(clientIdx);
            *pFailReason = CONN_FAIL_SESSION_NOT_AVAIL;
            return P2P_CLIENT_SUPPORT_MAX;
        }
    }
    else
    {
        /* Client session already allocated */
        DPRINT(P2P_MODULE, "client session already allocated: [client=%d], [clientUid=%s], [model=%s]",
               clientIdx, pPayload->uId, pPayload->model);

        /* Wait for client process thread to exit */
        stopP2pClientProcessThread(clientIdx);
    }

    /* Return allocated client session */
    return clientIdx;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send registration request without or without authentication
 * @param   withAuthF - TRUE = With Authentication and FALSE = Without Authentication
 */
static void sendRegReqMsg(BOOL withAuthF)
{
    /* Reset all data */
    memset(&p2pMsgInfo, 0, sizeof(p2pMsgInfo));

    /* Set protocol version */
    SET_SERVER_PROTOCOL_VERSION(p2pMsgInfo.protocolVersion);

    /* Prepare msg header */
    if (FALSE == prepareP2pMsgHeader(P2P_MSG_ID_REQ_REGISTER, withAuthF, &p2pMsgInfo.header))
    {
        EPRINT(P2P_MODULE, "fail to generate msg header: register");
        return;
    }

    /* Set P2P payload parameters */
    snprintf(p2pMsgInfo.payload.version, sizeof(p2pMsgInfo.payload.version), "%s", GetSoftwareVersionStr());
    p2pMsgInfo.payload.type = P2P_DEVICE_TYPE_NVR;
    snprintf(p2pMsgInfo.payload.model, sizeof(p2pMsgInfo.payload.model), "%s", GetNvrModelStr());
    p2pMsgInfo.payload.localAddr = p2pProcInfo.localAddr;
    p2pMsgInfo.payload.region = GetSystemTimezone();

    /* Add authemtication related info */
    if (FALSE == withAuthF)
    {
        /* Send without auth paras */
        RESET_STR_BUFF(p2pMsgInfo.payload.passkey);
    }
    else
    {
        /* Add password in payload */
        if (FALSE == encodeP2pAuthPass(&p2pMsgInfo.header, p2pMsgInfo.payload.passkey, sizeof(p2pMsgInfo.payload.passkey)))
        {
            EPRINT(P2P_MODULE, "fail to get auth passkey");
            return;
        }
    }

    /* Send P2P message on socket */
    if (SUCCESS != sendP2pServerMsg(p2pProcInfo.procSockFd))
    {
        EPRINT(P2P_MODULE, "fail to send msg: register");
    }
    else
    {
        DPRINT(P2P_MODULE, "msg sent: register %s auth", withAuthF ? "with" : "without");
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Receive registration response message
 * @param   pDataBuff
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL parseRegRespData(CHAR *pDataBuff)
{
    /* Reset all data */
    memset(&p2pMsgInfo, 0, sizeof(p2pMsgInfo));

    /* Decode json message */
    if (FALSE == DecodeJsonMsg(pDataBuff, &p2pMsgInfo))
    {
        EPRINT(P2P_MODULE, "fail to decode json msg");
        return FALSE;
    }

    /* Is it registration authentication message? */
    if (p2pMsgInfo.header.msgId == P2P_MSG_ID_RESP_AUTHENTICATION)
    {
        /* Validate P2P device ID */
        if (strcmp(p2pProcInfo.p2pDeviceIdU, p2pMsgInfo.header.deviceId))
        {
            EPRINT(P2P_MODULE, "invld device id recv: [deviceId=%s]", p2pMsgInfo.header.deviceId);
            return FALSE;
        }

        /* Get P2P server session ID */
        if (FALSE == decodeP2pData(p2pMsgInfo.payload.publicAddr.port, p2pProcInfo.p2pServerAddr.port, p2pMsgInfo.header.timestamp,
                                   p2pProcInfo.p2pCryptoSaltKey, p2pMsgInfo.header.sessionId, strlen(p2pMsgInfo.header.sessionId),
                                   p2pProcInfo.p2pServerSessionId, sizeof(p2pProcInfo.p2pServerSessionId)))
        {
            EPRINT(P2P_MODULE, "fail to get server session id: [sessionId=%s]", p2pMsgInfo.header.sessionId);
            return FALSE;
        }

        /* Validate P2P server session id */
        CHAR sessIdBuff[P2P_SERVER_RAND_NUM_START+1];
        snprintf(sessIdBuff, sizeof(sessIdBuff), "%.2s%d%.2s%.2s", &p2pProcInfo.p2pDeviceIdL[GET_MAC_OCTET(4)],
                P2P_DEVICE_TYPE_NVR, &p2pProcInfo.p2pDeviceIdL[GET_MAC_OCTET(6)], &p2pProcInfo.p2pDeviceIdU[GET_MAC_OCTET(5)]);
        if (strncmp(p2pProcInfo.p2pServerSessionId, sessIdBuff, strlen(sessIdBuff)))
        {
            EPRINT(P2P_MODULE, "invld session id recv: [sessionId=%s]", p2pMsgInfo.header.sessionId);
            RESET_STR_BUFF(p2pProcInfo.p2pServerSessionId);
            return FALSE;
        }

        /* Decode P2P server session id */
        return TRUE;
    }

    /* Message must be registration response with success or fail */
    if ((p2pMsgInfo.header.msgId != P2P_MSG_ID_RESP_REGISTER_SUCCESS) && (p2pMsgInfo.header.msgId != P2P_MSG_ID_RESP_REGISTER_FAIL))
    {
        EPRINT(P2P_MODULE, "invld msg id recv: [msgId=%d]", p2pMsgInfo.header.msgId);
        return FALSE;
    }

    /* Register success received without challenge */
    if (p2pProcInfo.p2pServerSessionId[0] == '\0')
    {
        EPRINT(P2P_MODULE, "reg resp rcvd without challenge: [msgId=%d]", p2pMsgInfo.header.msgId);
        return FALSE;
    }

    /* Validate P2P header and return status */
    return isValidP2pMsgHeader(&p2pMsgInfo.header, p2pMsgInfo.payload.publicAddr.port, TRUE);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send heartbeat message
 */
static void sendHeartbeatMsg(void)
{
    /* Reset all data */
    memset(&p2pMsgInfo, 0, sizeof(p2pMsgInfo));

    /* Set protocol version */
    SET_SERVER_PROTOCOL_VERSION(p2pMsgInfo.protocolVersion);

    /* Prepare msg header */
    if (FALSE == prepareP2pMsgHeader(P2P_MSG_ID_REQ_HEARTBEAT, TRUE, &p2pMsgInfo.header))
    {
        EPRINT(P2P_MODULE, "fail to generate msg header: heartbeat");
        return;
    }

    /* Send P2P message on socket */
    if (SUCCESS != sendP2pServerMsg(p2pProcInfo.procSockFd))
    {
        EPRINT(P2P_MODULE, "fail to send msg: heartbeat");
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Receive heartbeat ot other messages
 * @param   pDataBuff
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL parseHeartbeatData(CHAR *pDataBuff)
{
    /* Reset all data */
    memset(&p2pMsgInfo, 0, sizeof(p2pMsgInfo));

    /* Decode json message */
    if (FALSE == DecodeJsonMsg(pDataBuff, &p2pMsgInfo))
    {
        EPRINT(P2P_MODULE, "fail to decode json msg");
        return FALSE;
    }

    /* Message must be registration success if it not gets failed */
    if ((p2pMsgInfo.header.msgId != P2P_MSG_ID_RESP_HEARTBEAT_ACK) && (p2pMsgInfo.header.msgId != P2P_MSG_ID_RESP_HEARTBEAT_FAIL) &&
            ((p2pMsgInfo.header.msgId < P2P_MSG_ID_RESP_CONNECTION_REQUEST) || (p2pMsgInfo.header.msgId >= P2P_MSG_ID_RESP_MAX)))
    {
        EPRINT(P2P_MODULE, "invld msg id recv: [msgId=%d]", p2pMsgInfo.header.msgId);
        return FALSE;
    }

    /* Validate P2P header and return status */
    return isValidP2pMsgHeader(&p2pMsgInfo.header, p2pProcInfo.publicAddr.port, TRUE);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send Un-Register message to P2P server
 */
static void sendUnRegReqMsg(void)
{
    /* We should have valid socket FD */
    if (p2pProcInfo.procSockFd == INVALID_CONNECTION)
    {
        return;
    }

    /* Device should be registered with P2P server */
    if (p2pProcInfo.procState != P2P_PROC_STATE_HEARTBEAT)
    {
        return;
    }

    /* Session ID should be valid */
    if (p2pProcInfo.p2pServerSessionId[0] == '\0')
    {
        return;
    }

    /* Reset all data */
    memset(&p2pMsgInfo, 0, sizeof(p2pMsgInfo));

    /* Set protocol version */
    SET_SERVER_PROTOCOL_VERSION(p2pMsgInfo.protocolVersion);

    /* Prepare msg header */
    if (FALSE == prepareP2pMsgHeader(P2P_MSG_ID_REQ_UNREGISTER, TRUE, &p2pMsgInfo.header))
    {
        EPRINT(P2P_MODULE, "fail to generate msg header: unregister");
        return;
    }

    /* Send P2P message on socket */
    if (SUCCESS != sendP2pServerMsg(p2pProcInfo.procSockFd))
    {
        EPRINT(P2P_MODULE, "fail to send msg: unregister");
    }
    else
    {
        DPRINT(P2P_MODULE, "msg sent: unregister");
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send connection details message to P2P server
 * @param   pClientInfo
 */
static void sendConnDetailMsg(P2P_CLIENT_INFO_t *pClientInfo)
{
    P2P_CONFIG_t p2PConfig;

    /* Read P2P config */
    ReadP2PConfig(&p2PConfig);

    /* Reset all data */
    memset(&p2pMsgInfo, 0, sizeof(p2pMsgInfo));

    /* Set protocol version */
    SET_SERVER_PROTOCOL_VERSION(p2pMsgInfo.protocolVersion);

    /* Prepare msg header */
    if (FALSE == prepareP2pMsgHeader(P2P_MSG_ID_REQ_CONNECTION_DETAILS, TRUE, &p2pMsgInfo.header))
    {
        EPRINT(P2P_MODULE, "fail to generate msg header: connection details: [client=%d]", pClientInfo->clientIdx);
        return;
    }

    /* Generate and encrypt P2P token */
    if (FALSE == encodeP2pToken(p2pProcInfo.publicAddr.port, p2pProcInfo.p2pServerAddr.port, p2pMsgInfo.header.timestamp,
                                p2pProcInfo.p2pCryptoSaltKey, pClientInfo->clientUid, pClientInfo->deviceUid,
                                p2pMsgInfo.payload.p2pToken, sizeof(p2pMsgInfo.payload.p2pToken)))
    {
        EPRINT(P2P_MODULE, "fail to encode token: connection details: [client=%d]", pClientInfo->clientIdx);
        return;
    }

    /* Set P2P payload parameters */
    snprintf(p2pMsgInfo.payload.uId, sizeof(p2pMsgInfo.payload.uId), "%s", pClientInfo->clientUid);
    p2pMsgInfo.payload.localAddr = pClientInfo->localAddr;
    p2pMsgInfo.payload.publicAddr = pClientInfo->publicAddr;
    p2pMsgInfo.payload.relayServerFallback = ((TRUE == p2PConfig.fallbackToRelayServer) && (TRUE == p2pProcInfo.isRelayServerAvailable));

    /* Send P2P message on socket */
    if (SUCCESS != sendP2pServerMsg(p2pProcInfo.procSockFd))
    {
        EPRINT(P2P_MODULE, "fail to send msg: connection details: [client=%d]", pClientInfo->clientIdx);
    }
    else
    {
        DPRINT(P2P_MODULE, "msg sent: connection details: [client=%d]", pClientInfo->clientIdx);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send fallback to relay details message to P2P server
 * @param   pClientInfo
 */
static void sendFallbackToRelayDetailMsg(P2P_CLIENT_INFO_t *pClientInfo)
{
    /* Reset all data */
    memset(&p2pMsgInfo, 0, sizeof(p2pMsgInfo));

    /* Set protocol version */
    SET_SERVER_PROTOCOL_VERSION(p2pMsgInfo.protocolVersion);

    /* Prepare msg header */
    if (FALSE == prepareP2pMsgHeader(P2P_MSG_ID_REQ_FALLBACK_TO_RELAY_DETAILS, TRUE, &p2pMsgInfo.header))
    {
        EPRINT(P2P_MODULE, "fail to generate msg header: fallback to relay details: [client=%d]", pClientInfo->clientIdx);
        return;
    }

    /* Generate and encrypt P2P token */
    if (FALSE == encodeP2pToken(p2pProcInfo.publicAddr.port, p2pProcInfo.p2pServerAddr.port, p2pMsgInfo.header.timestamp,
                                p2pProcInfo.p2pCryptoSaltKey, pClientInfo->clientUid, pClientInfo->deviceUid,
                                p2pMsgInfo.payload.p2pToken, sizeof(p2pMsgInfo.payload.p2pToken)))
    {
        EPRINT(P2P_MODULE, "fail to encode token: fallback to relay details: [client=%d]", pClientInfo->clientIdx);
        return;
    }

    /* Set P2P payload parameters */
    snprintf(p2pMsgInfo.payload.uId, sizeof(p2pMsgInfo.payload.uId), "%s", pClientInfo->clientUid);
    p2pMsgInfo.payload.peerRelayAddr = pClientInfo->peerRelayAddr;

    /* Send P2P message on socket */
    if (SUCCESS != sendP2pServerMsg(p2pProcInfo.procSockFd))
    {
        EPRINT(P2P_MODULE, "fail to send msg: fallback to relay details: [client=%d]", pClientInfo->clientIdx);
    }
    else
    {
        DPRINT(P2P_MODULE, "msg sent: fallback to relay details: [client=%d]", pClientInfo->clientIdx);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send failure message to P2P server
 * @param   msgId
 * @param   pClientUid
 * @param   reason
 */
static void sendP2pFailMsg(P2P_MSG_ID_e msgId, CHAR *pClientUid, P2P_FAIL_REASON_e reason)
{
    CHAR clientUid[P2P_CLIENT_UID_LEN_MAX];

    /* In few cases, clientUid points to p2pMsgInfo and it will be reset later on. Hence taken local copy */
    snprintf(clientUid, sizeof(clientUid), "%s", pClientUid);

    /* Reset all data */
    memset(&p2pMsgInfo, 0, sizeof(p2pMsgInfo));

    /* Set protocol version */
    SET_SERVER_PROTOCOL_VERSION(p2pMsgInfo.protocolVersion);

    /* Prepare msg header */
    if (FALSE == prepareP2pMsgHeader(msgId, TRUE, &p2pMsgInfo.header))
    {
        EPRINT(P2P_MODULE, "fail to generate msg header: connection fail: [msgId=%d], [reason=%d]", msgId, reason);
        return;
    }

    /* Set connection fail reason and client Uid */
    snprintf(p2pMsgInfo.payload.uId, sizeof(p2pMsgInfo.payload.uId), "%s", clientUid);
    p2pMsgInfo.payload.reason = reason;

    /* Send P2P message on socket */
    if (SUCCESS != sendP2pServerMsg(p2pProcInfo.procSockFd))
    {
        EPRINT(P2P_MODULE, "fail to send msg: connection fail: [msgId=%d], [reason=%d]", msgId, reason);
    }
    else
    {
        DPRINT(P2P_MODULE, "msg sent: connection fail: [msgId=%d], [reason=%d]", msgId, reason);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send connection fail message to P2P server
 * @param   pClientUid
 * @param   reason
 */
static void sendConnFailMsg(CHAR *pClientUid, P2P_FAIL_REASON_e reason)
{
    /* Send p2p connection failure message */
    sendP2pFailMsg(P2P_MSG_ID_REQ_CONNECTION_FAIL, pClientUid, reason);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send STUN query request message to P2P server
 * @param   pClientInfo
 */
static void sendStunReqMsg(P2P_CLIENT_INFO_t *pClientInfo)
{
    /* Reset all data */
    memset(&p2pMsgInfo, 0, sizeof(p2pMsgInfo));

    /* Set protocol version */
    SET_SERVER_PROTOCOL_VERSION(p2pMsgInfo.protocolVersion);

    /* Prepare msg header */
    if (FALSE == prepareP2pMsgHeader(P2P_MSG_ID_REQ_STUN_QUERY, FALSE, &p2pMsgInfo.header))
    {
        EPRINT(P2P_MODULE, "fail to generate msg header: stun query");
        return;
    }

    /* Send P2P message on socket */
    if (SUCCESS != sendP2pServerMsg(pClientInfo->sockFd))
    {
        EPRINT(P2P_MODULE, "fail to send msg: stun query: [client=%d]", pClientInfo->clientIdx);
    }
    else
    {
        DPRINT(P2P_MODULE, "msg sent: stun query: [client=%d]", pClientInfo->clientIdx);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Receive STUN query response message from P2P server
 * @param   pDataBuff
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL parseStunRespMsg(CHAR *pDataBuff)
{
    /* Reset all data */
    memset(&p2pMsgInfo, 0, sizeof(p2pMsgInfo));

    /* Decode json message */
    if (FALSE == DecodeJsonMsg(pDataBuff, &p2pMsgInfo))
    {
        EPRINT(P2P_MODULE, "fail to decode json msg");
        return FALSE;
    }

    /* Message must be registration success if it not gets failed */
    if (p2pMsgInfo.header.msgId != P2P_MSG_ID_RESP_STUN_RESPONSE)
    {
        EPRINT(P2P_MODULE, "invld msg id recv: [msgId=%d]", p2pMsgInfo.header.msgId);
        return FALSE;
    }

    /* Validate P2P header and return status */
    return isValidP2pMsgHeader(&p2pMsgInfo.header, p2pMsgInfo.payload.publicAddr.port, FALSE);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send fallback to relay fail message to P2P server
 * @param   pClientUid
 * @param   reason
 */
static void sendRelayFallbackFailMsg(CHAR *pClientUid, P2P_FAIL_REASON_e reason)
{
    /* Send p2p relay fallback failure message */
    sendP2pFailMsg(P2P_MSG_ID_REQ_FALLBACK_TO_RELAY_FAIL, pClientUid, reason);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send P2P server message on socket
 * @param   sockFd
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL sendP2pServerMsg(INT32 sockFd)
{
    CHAR    jsonMsg[P2P_SERVER_PAYLOAD_LEN_MAX];
    UINT8   headerLen = P2P_SERVER_MSG_HEADER_LEN_MAX;

    /* Prepare message in JSON format */
    p2pServerMsgHeader.payloadLen = PrepareJsonMsg(&p2pMsgInfo, jsonMsg + headerLen, sizeof(jsonMsg));
    if (0 == p2pServerMsgHeader.payloadLen)
    {
        EPRINT(P2P_MODULE, "fail to create msg: [msgId=%d]", p2pMsgInfo.header.msgId);
        return FALSE;
    }

    /* Update header for TCP message */
    memcpy(jsonMsg, &p2pServerMsgHeader, headerLen);
    p2pServerMsgHeader.payloadLen += headerLen;

    /* Send json message and return status */
    return SendP2pMsg(sockFd, jsonMsg, p2pServerMsgHeader.payloadLen, 0);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Prepare P2P message header
 * @param   msdId
 * @param   withAuthF
 * @param   p2pHeader
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL prepareP2pMsgHeader(P2P_MSG_ID_e msgId, BOOL withAuthF, P2P_HEADER_t *p2pHeader)
{
    /* Set P2P header parameters */
    p2pHeader->msgId = msgId;
    snprintf(p2pHeader->deviceId, sizeof(p2pHeader->deviceId), "%s", p2pProcInfo.p2pDeviceIdU);
    GetP2pTimeStamp(p2pHeader->timestamp, sizeof(p2pHeader->timestamp));

    /* Add session id in header */
    if (withAuthF)
    {
        /* Prepare auth and return */
        return encodeP2pData(p2pProcInfo.publicAddr.port, p2pProcInfo.p2pServerAddr.port, p2pHeader->timestamp,
                             p2pProcInfo.p2pCryptoSaltKey, p2pProcInfo.p2pServerSessionId,
                             strlen(p2pProcInfo.p2pServerSessionId), p2pHeader->sessionId, sizeof(p2pHeader->sessionId));
    }

    /* Set session id as null */
    RESET_STR_BUFF(p2pHeader->sessionId);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Validate header of received message
 * @param   p2pHeader
 * @param   publicPort
 * @param   withAuthF
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL isValidP2pMsgHeader(P2P_HEADER_t *p2pHeader, UINT16 publicPort, BOOL withAuthF)
{
    CHAR sessionId[P2P_RAW_SESSION_ID_LEN_MAX];

    /* Validate P2P device ID */
    if (strcmp(p2pProcInfo.p2pDeviceIdU, p2pHeader->deviceId))
    {
        EPRINT(P2P_MODULE, "invld device id recv: [deviceId=%s]", p2pHeader->deviceId);
        return FALSE;
    }

    /* Do we need to auth session ID? */
    if (withAuthF)
    {
        /* Get P2P server session ID */
        if (FALSE == decodeP2pData(publicPort, p2pProcInfo.p2pServerAddr.port, p2pHeader->timestamp,
                                   p2pProcInfo.p2pCryptoSaltKey, p2pHeader->sessionId, strlen(p2pHeader->sessionId),
                                   sessionId, sizeof(sessionId)))
        {
            EPRINT(P2P_MODULE, "fail to get server session id: [sessionId=%s]", p2pHeader->sessionId);
            return FALSE;
        }

        /* Validate P2P server session id */
        if (strcmp(p2pProcInfo.p2pServerSessionId, sessionId))
        {
            EPRINT(P2P_MODULE, "invld session id recv: [sessionId=%s]", p2pHeader->sessionId);
            return FALSE;
        }
    }

    /* Valid header found */
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Fatal error found on socket
 */
static void handleSocketFatalErr(void)
{
    /* Re-init P2P process as connection closed */
    restartP2pPocess();
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Start P2P process FSM timer for given FSM event
 * @param   fsmEvent
 */
static void startP2pProcFsmTimer(PROC_FSM_EVENT_e fsmEvent)
{
    /* Start Timer to start P2P process */
    if (FALSE == SysTimerStart(&p2pProcInfo.pSysTimerListHead, &p2pProcInfo.fsmTimer, fsmEvent, P2P_CLIENT_SUPPORT_MAX,
                               p2pProcFsmTimer[fsmEvent].timeInMs, p2pProcFsmTimer[fsmEvent].loopCnt))
    {
        EPRINT(P2P_MODULE, "fail to start p2p process fsm timer: [fsmEvent=%d]", fsmEvent);
        return;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Start P2P client session response timer for FSM
 * @param   fsmEvent
 * @param   clientIdx
 */
static void startP2pClientSessionRespTimer(PROC_FSM_EVENT_e fsmEvent, UINT8 clientIdx)
{
    /* Start P2P client connection response timer */
    if (FALSE == SysTimerStart(&p2pProcInfo.pSysTimerListHead, &p2pClientSession[clientIdx].respTimer, fsmEvent, clientIdx,
                               p2pProcFsmTimer[fsmEvent].timeInMs, p2pProcFsmTimer[fsmEvent].loopCnt))
    {
        EPRINT(P2P_MODULE, "fail to start client conn resp timer: [fsmEvent=%d]", fsmEvent);
        return;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Stop P2P client session response timer
 * @param   clientIdx
 */
static void stopP2pClientSessionRespTimer(UINT8 clientIdx)
{
    /* Stop all client connection related timer */
    SysTimerStop(&p2pProcInfo.pSysTimerListHead, &p2pClientSession[clientIdx].respTimer);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Change P2P process FSM state
 * @param   fsmState
 */
static void changeP2pProcFsmState(P2P_PROC_STATE_e fsmState)
{
    /* Check FSM state changed or not */
    if (p2pProcInfo.procState == fsmState)
    {
        /* No state changed */
        return;
    }

    DPRINT(P2P_MODULE, "fsm state changed: [status=%s], [%s --> %s]",
           p2pProcStatusStr[p2pProcInfo.p2pStatus], p2pProcStateStr[p2pProcInfo.procState], p2pProcStateStr[fsmState]);
    p2pProcInfo.procState = fsmState;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Change P2P process status
 * @param   p2pStatus
 */
static void changeP2pProcessStatus(P2P_STATUS_e p2pStatus)
{
    /* Check status changed or not */
    if (p2pProcInfo.p2pStatus == p2pStatus)
    {
        /* No status changed */
        return;
    }

    /* Log P2P connection status change event if applicable */
    if (p2pStatus == P2P_STATUS_CONNECTED)
    {
        /* If new status changed to connected then write P2P up event */
        WriteEvent(LOG_NETWORK_EVENT, LOG_P2P_STATUS, NULL, NULL, EVENT_UP);
    }
    else if (p2pProcInfo.p2pStatus == P2P_STATUS_CONNECTED)
    {
        /* If new status changed from connected to other then write P2P down event */
        WriteEvent(LOG_NETWORK_EVENT, LOG_P2P_STATUS, NULL, NULL, EVENT_DOWN);
    }

    DPRINT(P2P_MODULE, "p2p status changed: [state=%s], [%s --> %s]",
           p2pProcStateStr[p2pProcInfo.procState], p2pProcStatusStr[p2pProcInfo.p2pStatus], p2pProcStatusStr[p2pStatus]);
    p2pProcInfo.p2pStatus = p2pStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get local IP address
 * @param   p2pNetworkPort
 * @param   ipAddrBuff
 * @param   buffLen
 * @return  Returns TRUE on success else returns FALSE
 */
static BOOL getLocalIpAddr(NETWORK_PORT_e p2pNetworkPort, CHAR *ipAddrBuff, UINT16 buffLen)
{
    LAN_CONFIG_t lanCfg;

    /* Get LAN1 or LAN2 parameters for ip address */
    if (SUCCESS != GetNetworkParamInfo(p2pNetworkPort, &lanCfg))
    {
        EPRINT(P2P_MODULE, "fail to get lan nw param: [lan=%d]", p2pNetworkPort);
        return FALSE;
    }

    /* Copy local IP address */
    snprintf(ipAddrBuff, buffLen, "%s", lanCfg.ipv4.lan.ipAddress);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   P2P process fsm timer expire callback handler
 * @param   appData1 - FSM event
 * @param   appData2 - FSM event
 * @param   isTmrOvr - TRUE if timer expired else FALSE
 */
static void p2pProcFsmTmrExpHandler(UINT32 appData1, UINT32 appData2, BOOL isTimerExpired)
{
    p2pProcessFsm(appData1, appData2, &isTimerExpired);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   prepare P2P server auth password using encryption and encoding for sending in regestration
 * @param   p2pHeader
 * @param   pEncPassKey
 * @param   passKeyLen
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL encodeP2pAuthPass(P2P_HEADER_t *p2pHeader, CHAR *pEncPassKey, UINT16 passKeyLen)
{
    CHAR    ipAddrWoDot[IPV4_ADDR_LEN_MAX];
    CHAR    authPassKey[P2P_DEVICE_RAW_PASS_KEY_LEN_MAX];
    UINT8   loopCnt;
    UINT8   byteCnt = 0;

    /* Remove '.' from ip addr string */
    for (loopCnt = 0; loopCnt < strlen(p2pProcInfo.publicAddr.ip); loopCnt++)
    {
        if (p2pProcInfo.publicAddr.ip[loopCnt] != '.')
        {
            ipAddrWoDot[byteCnt++] = p2pProcInfo.publicAddr.ip[loopCnt];
        }
    }

    /* Terminate with null */
    ipAddrWoDot[byteCnt] = '\0';

    /* MAC 1-MAC 3- MAC 2-Public IP Address-Random Number  */
    snprintf(authPassKey, sizeof(authPassKey), "%.2s%.2s%.2s%s%d%.10s",
             &p2pProcInfo.p2pDeviceIdL[GET_MAC_OCTET(4)], &p2pProcInfo.p2pDeviceIdL[GET_MAC_OCTET(6)],
            &p2pProcInfo.p2pDeviceIdU[GET_MAC_OCTET(5)], ipAddrWoDot, p2pProcInfo.publicAddr.port,
            &p2pProcInfo.p2pServerSessionId[P2P_SERVER_RAND_NUM_START]);

    /* Encrypt authentication password */
    return encodeP2pData(p2pProcInfo.publicAddr.port, p2pProcInfo.p2pServerAddr.port, p2pHeader->timestamp,
                         p2pProcInfo.p2pCryptoSaltKey, authPassKey, strlen(authPassKey), pEncPassKey, passKeyLen);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Generate P2P token from device and client unique id, encrypt it and convert in base 64
 * @param   publicPort
 * @param   serverPort
 * @param   pTimestamp
 * @param   pSaltKey
 * @param   pClientUid
 * @param   pDeviceUid
 * @param   pEncData
 * @param   encDataLen
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL encodeP2pToken(UINT16 publicPort, UINT16 serverPort, CHAR *pTimestamp, CHAR *pSaltKey,
                           CHAR *pClientUid, CHAR *pDeviceUid, CHAR *pEncData, UINT16 encDataLen)
{
    CHAR p2pToken[P2P_DEVICE_RAW_TOKEN_LEN_MAX];

    /* Generate P2P token from client and device unique ID */
    snprintf(p2pToken, sizeof(p2pToken), "%s%s", pClientUid, pDeviceUid);

    /* Encrypt P2P token */
    return encodeP2pData(publicPort, serverPort, pTimestamp, pSaltKey, p2pToken, strlen(p2pToken), pEncData, encDataLen);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Generate password, encrypt data and encode in base64
 * @param   publicPort1 - Device public port (Device <--> Cloud) OR Client Public Port (Device <--> Client)
 * @param   publicPort2 - Server public port (Device <--> Cloud) OR Device public port (Device <--> Client)
 * @param   pTimestamp - Current time stamp
 * @param   pSaltKey - Salt key for encryption
 * @param   pPlainData - Data to be encrypt
 * @param   plainDataLen - Data length
 * @param   pEncData - Output encrypted data
 * @param   encDataLen - Encrypted data length
 * @return  TRUE on success; FALSE otherwise
 */
BOOL encodeP2pData(UINT16 publicPort1, UINT16 publicPort2, CHAR *pTimestamp, CHAR *pSaltKey,
                   CHAR *pPlainData, UINT16 plainDataLen, CHAR *pEncData, UINT16 encDataLen)
{
    BOOL    retStatus = FALSE;
    CHAR    *cipherData = NULL;
    CHAR    *encodedData = NULL;
    UINT32  outLen = 0;
    CHAR    password[CRYPTO_PASSWORD_LEN_MAX];

    /* generate password for encryption */
    generateCryptoPassword(publicPort1, publicPort2, pTimestamp, password, sizeof(password));

    do
    {
        /* encrypt session id */
        cipherData = (CHAR *)EncryptAes256((UINT8 *)pPlainData, plainDataLen, password, pSaltKey, &outLen);
        if (NULL == cipherData)
        {
            EPRINT(P2P_MODULE, "fail to encrypt data");
            break;
        }

        /* Convert P2P server session id from raw data to base64 */
        encodedData = EncodeBase64(cipherData, outLen);
        if (NULL == encodedData)
        {
            EPRINT(P2P_MODULE, "fail to encode data");
            break;
        }

        /* Store P2P session ID */
        snprintf(pEncData, encDataLen, "%s", encodedData);
        retStatus = TRUE;

    }while(0);

    /* free allocated memory */
    FREE_MEMORY(cipherData)
    FREE_MEMORY(encodedData)
    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Generate password, decode from base64 and decrypt data
 * @param   publicPort1 - Device public port (Device <--> Cloud) OR Client Public Port (Device <--> Client)
 * @param   publicPort2 - Server public port (Device <--> Cloud) OR Device public port (Device <--> Client)
 * @param   pTimestamp - Current time stamp
 * @param   pSaltKey - Salt key for encryption
 * @param   pEncData - Output encrypted data
 * @param   encDataLen - Encrypted data length
 * @param   pPlainData - Data to be encrypt
 * @param   plainDataLen - Data length
 * @return  TRUE on success; FALSE otherwise
 */
static BOOL decodeP2pData(UINT16 publicPort1, UINT16 publicPort2, CHAR *pTimestamp, CHAR *pSaltKey,
                          CHAR *pEncData, UINT16 encDataLen, CHAR *pPlainData, UINT16 plainDataLen)
{
    BOOL    retStatus = FALSE;
    CHAR    *encodedData = NULL;
    CHAR    *cipherData = NULL;
    UINT32  cipherDataLen = 0;
    UINT32  outLen = 0;
    CHAR    password[CRYPTO_PASSWORD_LEN_MAX];

    /* generate password for decryption */
    generateCryptoPassword(publicPort1, publicPort2, pTimestamp, password, sizeof(password));

    do
    {
        /* Convert P2P server session id from base64 to raw data */
        cipherData = DecodeBase64(pEncData, encDataLen, &cipherDataLen);
        if (NULL == cipherData)
        {
            EPRINT(P2P_MODULE, "fail to get decode data");
            break;
        }

        /* Decrypt p2p server session id */
        encodedData = (CHAR *)DecryptAes256((UINT8 *)cipherData, cipherDataLen, password, pSaltKey, &outLen);
        if (NULL == encodedData)
        {
            EPRINT(P2P_MODULE, "fail to decrypt data");
            break;
        }

        /* Store P2P session ID */
        snprintf(pPlainData, plainDataLen, "%s", encodedData);

        /* Successfully acquired required data */
        retStatus = TRUE;

    }while(0);

    /* Free allocated memory */
    FREE_MEMORY(encodedData);
    FREE_MEMORY(cipherData);
    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Generate password for encryption/decryption
 * @param   publicPort1 - Device public port (Device <--> Cloud) OR Client Public Port (Device <--> Client)
 * @param   publicPort2 - Server public port (Device <--> Cloud) OR Device public port (Device <--> Client)
 * @param   timeStamp
 * @param   password - output
 */
static void generateCryptoPassword(UINT16 publicPort1, UINT16 publicPort2, CHAR *pTimestamp, CHAR *pPassword, UINT16 passLen)
{
    /* Prepare P2P server decrypt password: YYYYMMDD-publicPort1-publicPort2-sss-HHMMSS */
    snprintf(pPassword, passLen, "%.8s%d%d%.3s%.6s", &pTimestamp[0], publicPort1, publicPort2, &pTimestamp[14], &pTimestamp[8]);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Generate salt key for encryption/decryption
 * @param   pDeviceIdL
 * @param   pDeviceIdU
 * @param   pSaltKey - output
 * @param   keyLen
 */
static void generateSaltKey(CHAR *pDeviceIdL, CHAR *pDeviceIdU, CHAR *pSaltKey, UINT8 keyLen)
{
    /* Prepare P2P server salt key: MAC3-MAC6-MAC5-MAC2 */
    snprintf(pSaltKey, keyLen, "%.2s%.2s%.2s%.2s",
             &pDeviceIdU[GET_MAC_OCTET(3)], &pDeviceIdL[GET_MAC_OCTET(6)], &pDeviceIdU[GET_MAC_OCTET(5)], &pDeviceIdL[GET_MAC_OCTET(2)]);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Generate session id for client or device based on type
 * @param   type
 * @param   pRandNum
 * @param   pSessionId
 * @param   sessionIdLen
 */
static void generateP2pClientSessionId(P2P_DEVICE_TYPE_e type, CHAR *pRandNum, CHAR *pSessionId, UINT8 sessionIdLen)
{
    /* Generate device/client session ID: Type-MAC5-mac4-Random Number-mac6 */
    snprintf(pSessionId, sessionIdLen, "%d%.2s%.2s%s%.2s", type, &p2pProcInfo.p2pDeviceIdU[GET_MAC_OCTET(5)],
            &p2pProcInfo.p2pDeviceIdL[GET_MAC_OCTET(4)], pRandNum, &p2pProcInfo.p2pDeviceIdL[GET_MAC_OCTET(6)]);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get unique p2p client session id
 * @param   pClientInfo
 */
static void getP2pClientSessionId(P2P_CLIENT_INFO_t *pClientInfo)
{
    /* Get unique alpha numeric string */
    GetRandomAlphaNumStr(pClientInfo->deviceUid, sizeof(pClientInfo->deviceUid), FALSE);

    /* Store device session ID to be sent to client */
    generateP2pClientSessionId(P2P_DEVICE_TYPE_NVR, pClientInfo->deviceUid, pClientInfo->deviceSessionId, sizeof(pClientInfo->deviceSessionId));

    /* Store client session ID to be expected from client */
    generateP2pClientSessionId(pClientInfo->clientType, pClientInfo->deviceUid, pClientInfo->clientSessionId, sizeof(pClientInfo->clientSessionId));
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Encode P2P client session ID; Encrypt and convert into base64
 * @param   pPlainSessionId - Plain session ID
 * @param   clientPort - Client public port
 * @param   devicePort - Device public port
 * @param   pTimestamp - Current timestamp
 * @param   pEncSessoinId - Encrypted session ID
 * @param   sessionIdLen - output buffer length
 * @return  TRUE on success; FALSE otherwise
 */
BOOL EncodeP2pClientSessionId(CHAR *pPlainSessionId, UINT16 clientPort, UINT16 devicePort, CHAR *pTimestamp, CHAR *pEncSessoinId, UINT8 sessionIdLen)
{
    /* Encrypt session id */
    return encodeP2pData(clientPort, devicePort, pTimestamp, p2pProcInfo.p2pCryptoSaltKey,
                         pPlainSessionId, strlen(pPlainSessionId), pEncSessoinId, sessionIdLen);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Decode P2P client session ID; Decode from base64 and decrypt it
 * @param   pEncSessoinId
 * @param   clientPort
 * @param   devicePort
 * @param   pTimestamp
 * @param   pPlainSessionId
 * @param   sessionIdLen
 * @return  TRUE on success; FALSE otherwise
 */
BOOL DecodeP2pClientSessionId(CHAR *pEncSessoinId, UINT16 clientPort, UINT16 devicePort, CHAR *pTimestamp, CHAR *pPlainSessionId, UINT8 sessionIdLen)
{
    return decodeP2pData(clientPort, devicePort, pTimestamp, p2pProcInfo.p2pCryptoSaltKey,
                         pEncSessoinId, strlen(pEncSessoinId), pPlainSessionId, sessionIdLen);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get P2P client session index from client unique ID
 * @param   pClientUid
 * @return  Valid if session matched else invalid
 */
static UINT8 getP2pClientIdxFromClientUid(CHAR *pClientUid)
{
    UINT8 clientIdx;

    for (clientIdx = 0; clientIdx < P2P_CLIENT_SUPPORT_MAX; clientIdx++)
    {
        if (strcmp(pClientUid, p2pClientSession[clientIdx].clientInfo.clientUid) == 0)
        {
            break;
        }
    }

    return clientIdx;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Allocate existing or new client session index with existing client session running status
 * @param   pClientUid
 * @param   pClientIdx
 * @param   pIsSessionAllocated
 * @return  Returns TRUE if session is available else returns FALSE
 */
static UINT8 allocP2pClientSession(CHAR *pClientUid, UINT8 *pClientIdx, BOOL *pIsSessionAllocated)
{
    UINT8 clientIdx;

    /* Init with default invalid values */
    *pIsSessionAllocated = FALSE;
    *pClientIdx = P2P_CLIENT_SUPPORT_MAX;

    /* Assign same index to client if we have already allocated in past and available in list */
    for (clientIdx = 0; clientIdx < P2P_CLIENT_SUPPORT_MAX; clientIdx++)
    {
        /* Is client already available in list? */
        if (strcmp(pClientUid, p2pClientSession[clientIdx].clientInfo.clientUid) == 0)
        {
            /* Client already available in list. Provide that client index */
            *pClientIdx = clientIdx;
            *pIsSessionAllocated = p2pClientSession[clientIdx].isAllocated;
            p2pClientSession[clientIdx].isAllocated = TRUE;
            return TRUE;
        }
    }

    /* Try to allocate fresh index to client */
    for (clientIdx = 0; clientIdx < P2P_CLIENT_SUPPORT_MAX; clientIdx++)
    {
        if ((FALSE == p2pClientSession[clientIdx].isAllocated) && (p2pClientSession[clientIdx].clientInfo.clientUid[0] == '\0'))
        {
            /* Assign free fresh client index */
            *pClientIdx = clientIdx;
            p2pClientSession[clientIdx].isAllocated = TRUE;
            return TRUE;
        }
    }

    /* We have no fresh index then find free index from beginning */
    for (clientIdx = 0; clientIdx < P2P_CLIENT_SUPPORT_MAX; clientIdx++)
    {
        if (FALSE == p2pClientSession[clientIdx].isAllocated)
        {
            /* Assign free client index */
            *pClientIdx = clientIdx;
            p2pClientSession[clientIdx].isAllocated = TRUE;
            return TRUE;
        }
    }

    /* We have no free index */
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Deallocate client session
 * @param   clientIdx
 */
static void deallocP2pClientSession(UINT8 clientIdx)
{
    /* Free client session */
    p2pClientSession[clientIdx].isAllocated = FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Stop P2P client session as STUN failed
 * @param   clientIdx
 */
void stopP2pClientSession(UINT8 clientIdx)
{
    /* Stop all timers of client and free all info of client */
    stopP2pClientSessionRespTimer(clientIdx);

    /* Deallocate client session */
    deallocP2pClientSession(clientIdx);

    /* Close client socket */
    CloseSocket(&p2pClientSession[clientIdx].clientInfo.sockFd);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   wait to exit P2P client process thread
 * @param   clientIdx
 */
static void waitToExitP2pClient(UINT8 clientIdx)
{
    UINT8 waitCnt = 10;

    /* Wait for client exit */
    while ((CLIENT_PROC_STAT_EXIT != GetP2pClientProcessRunStatus(clientIdx)) || (--waitCnt == 0))
    {
        /* Wait to change status */
        SleepNanoSec(P2P_PROC_TMR_TICK_RESOL_IN_NS/5);
    }

    /* Client thread stuck some where */
    if (waitCnt == 0)
    {
        /* Force stop client thread */
        EPRINT(P2P_MODULE, "client thread stuck: [client=%d], [model=%s]", clientIdx, p2pClientSession[clientIdx].clientInfo.model);
        SetP2pClientProcessRunStatus(clientIdx, CLIENT_PROC_STAT_EXIT);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Stop P2P client process thread
 * @param   clientIdx
 */
static void stopP2pClientProcessThread(UINT8 clientIdx)
{
    /* Return if client session index is invalid */
    RETURN_IF_INVLD_CLIENT_ID(clientIdx);

    /* Check client thread running or not */
    if (CLIENT_PROC_STAT_EXIT == GetP2pClientProcessRunStatus(clientIdx))
    {
        WPRINT(P2P_MODULE, "client session allocated but thread not running: [client=%d], [model=%s]", clientIdx, p2pClientSession[clientIdx].clientInfo.model);
        return;
    }

    /* Exit from running thread */
    SetP2pClientProcessRunStatus(clientIdx, CLIENT_PROC_STAT_STOP);

    /* Wait to thread exit */
    DPRINT(P2P_MODULE, "waiting to client process exit: [client=%d], [model=%s]", clientIdx, p2pClientSession[clientIdx].clientInfo.model);
    waitToExitP2pClient(clientIdx);
    DPRINT(P2P_MODULE, "client process exited: [client=%d], [model=%s]", clientIdx, p2pClientSession[clientIdx].clientInfo.model);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Stop all P2P client processes
 */
static void stopAllP2pClientProcess(void)
{
    UINT8 clientIdx;

    for (clientIdx = 0; clientIdx < P2P_CLIENT_SUPPORT_MAX; clientIdx++)
    {
        /* Nothing to do if client is already exited */
        if (CLIENT_PROC_STAT_EXIT == GetP2pClientProcessRunStatus(clientIdx))
        {
            continue;
        }

        /* Set client status to stop */
        SetP2pClientProcessRunStatus(clientIdx, CLIENT_PROC_STAT_STOP);
    }

    for (clientIdx = 0; clientIdx < P2P_CLIENT_SUPPORT_MAX; clientIdx++)
    {
        /* Wait for client exit */
        waitToExitP2pClient(clientIdx);

        /* Stop P2P client session */
        stopP2pClientSession(clientIdx);
    }
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
