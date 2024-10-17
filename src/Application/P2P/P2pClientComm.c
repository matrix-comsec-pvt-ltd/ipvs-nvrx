//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		P2pClientComm.c
@brief      File containing the definations of P2P client communication functions.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "P2pClientComm.h"
#include "DebugLog.h"
#include "Utils.h"
#include "DateTime.h"
#include "NetworkCommand.h"
#include "NetworkConfig.h"
#include "MediaStreamer.h"
#include "CameraInterface.h"
#include "TurnClient.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define P2P_CLIENT_TMR_TICK_RESOL_IN_MS     100
#define P2P_CLIENT_TMR_TICK_RESOL_IN_NS     100000000LL /* P2P_CLIENT_TMR_TICK_RESOL_IN_MS * NANO_SEC_PER_MILLI_SEC */
#define P2P_CLIENT_MAX_SLEEP_DELAY_IN_MS    3000

#define P2P_HEADER_SEND_WAIT_SEC_MAX        1
#define P2P_PACKET_POLL_CNT_MAX             16
#define P2P_CLIENT_STORE_MSG_MAX            50
#define P2P_CLIENT_MSG_ID_MASK              0x0000A000
#define P2P_MSG_MAGIC_CODE                  0x55AA55AA
#define P2P_CLIENT_MSG_HEADER_LEN           sizeof(P2P_CLIENT_MSG_HEADER_t)
#define P2P_SESSION_PAYLOAD_HEADER_LEN      sizeof(P2P_CLIENT_SESSION_MSG_t)
#define P2P_SESSION_MSG_MIN_LEN             (P2P_CLIENT_MSG_HEADER_LEN + P2P_SESSION_PAYLOAD_HEADER_LEN)
#define P2P_CLIENT_THREAD_STACK_SIZE        (1 * MEGA_BYTE)
#define RELAY_CHALLENGE_RETRY_MAX           3

//#################################################################################################
// @DATA TYPES
//#################################################################################################
/* p2p client fsm state */
typedef enum
{
    P2P_CLIENT_STATE_HANDSHAKE = 0,
    P2P_CLIENT_STATE_HANDSHAKE_ACK,
    P2P_CLIENT_STATE_LOGIN,
    P2P_CLIENT_STATE_HEARTBEAT,
    P2P_CLIENT_STATE_HOLD,
    P2P_CLIENT_STATE_MAX

}P2P_CLIENT_FSM_STATE_e;

/* p2p client fsm events */
typedef enum
{
    P2P_CLIENT_FSM_EVENT_MSG_RECV = 0,
    P2P_CLIENT_FSM_EVENT_HANDSHAKE_INIT_WAIT_TMR_EXP,
    P2P_CLIENT_FSM_EVENT_HANDSHAKE_WAIT_TMR_EXP,
    P2P_CLIENT_FSM_EVENT_HANDSHAKE_ACK_WAIT_TMR_EXP,
    P2P_CLIENT_FSM_EVENT_RELAY_CONTROL_CONNECT_WAIT_TMR_EXP,
    P2P_CLIENT_FSM_EVENT_RELAY_ALLOCATION_RESP_WAIT_TMR_EXP,
    P2P_CLIENT_FSM_EVENT_RELAY_CREATE_PERMISSION_RESP_WAIT_TMR_EXP,
    P2P_CLIENT_FSM_EVENT_RELAY_CONNECTION_ATTEMPT_WAIT_TMR_EXP,
    P2P_CLIENT_FSM_EVENT_RELAY_DATA_CONNECT_WAIT_TMR_EXP,
    P2P_CLIENT_FSM_EVENT_RELAY_CONNECTION_BIND_RESP_WAIT_TMR_EXP,
    P2P_CLIENT_FSM_EVENT_RELAY_REFRESH_REQ_WAIT_TMR_EXP,
    P2P_CLIENT_FSM_EVENT_RELAY_REFRESH_RESP_WAIT_TMR_EXP,
    P2P_CLIENT_FSM_EVENT_LOGIN_WAIT_TMR_EXP,
    P2P_CLIENT_FSM_EVENT_HEARTBEAT_WAIT_TMR_EXP,
    P2P_CLIENT_FSM_EVENT_HEARTBEAT_RESP_RETRY_TMR_EXP,
    P2P_CLIENT_FSM_EVENT_HOLD_ACK_TMR_EXP,
    P2P_CLIENT_FSM_EVENT_MAX

}P2P_CLIENT_FSM_EVENT_e;

typedef struct
{
    UINT32  clientMsgUid;
    UINT32  localMsgUid;

}P2P_MSG_UID_t;

typedef struct
{
    INT32   sockFd;
    UINT32  clientMsgUid;

}P2P_DATA_SEND_FD_INFO_t;

/* p2p client information */
typedef struct
{
    P2P_CLIENT_FSM_STATE_e      clientFsmState;
    P2P_CLIENT_INFO_t           *pConnInfo;

    pthread_mutex_t             socketLock;
    INT32                       serverSockFd;
    INT32                       acceptedClientFd;
    INT32                       clientLocalSockFd;
    INT32                       clientPublicSockFd;
    INT32                       relayControlChannelFd;
    BOOL                        localClientConnStatus;
    BOOL                        publicClientConnStatus;
    UINT8                       relayServerIdx;
    UINT8                       challengeMsgRetryCnt;
    BOOL                        isRelayChnlAllocDone;
    BOOL                        isP2pClientHold;
    SYS_TIMER_t                 *pSysTimerListHead;
    SYS_TIMER_t                 fsmTimer;
    SYS_TIMER_t                 utilTimer;
    UINT64                      refTime;

    P2P_CLIENT_MSG_HEADER_t     clientMsgHeader;
    UINT8                       dataRecvRetryCnt;

    IP_ADDR_INFO_t              relayServerAddr;
    IP_ADDR_INFO_t              recvAddr;
    UINT32                      sessionMsgReqUid;
    UINT32                      sessionMsgRespUid;
    P2P_DATA_SEND_FD_INFO_t     dataSendFdInfo[P2P_CLIENT_FD_TYPE_MAX];

    P2P_MSG_UID_t               p2pMsgUid[P2P_CLIENT_STORE_MSG_MAX];

    TURN_SESSION_INFO_t         turnSessionInfo;

}P2P_CLIENT_PROCESS_t;

typedef struct
{
    UINT8           clientIdx;
    pthread_mutex_t clientLock;

}P2P_CLIENT_FD_INFO_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static const CHAR *pClientFsmStateStr[P2P_CLIENT_STATE_MAX] = {"HANDSHAKE", "HANDSHAKE_ACK", "LOGIN", "HEARTBEAT", "HOLD"};
static const CHAR *pTurnMsgTypeStr[TURN_MSG_TYPE_MAX] = {"ALLOCATE", "CREATE_PERMISSION", "CONNECTION_BIND", "REFRESH", "CONNECTION_ATTEMPT"};
static const CHAR *pTurnMsgStatusStr[TURN_STATUS_MAX] = {"OK", "FAIL", "ERROR", "CHALLENGE"};

static P2P_CLIENT_PROCESS_t p2pClientProc[P2P_CLIENT_SUPPORT_MAX];
static P2P_CLIENT_FD_INFO_t p2pClientFdInfo[P2P_CLIENT_FD_TYPE_MAX];

static const P2P_FSM_TIMER_t p2pClientFsmTimer[P2P_CLIENT_FSM_EVENT_MAX] =
{
    {     0,     0 }, /* P2P_CLIENT_FSM_EVENT_MSG_RECV */
    {  1000,     1 }, /* P2P_CLIENT_FSM_EVENT_HANDSHAKE_INIT_WAIT_TMR_EXP */
    {  1000,    20 }, /* P2P_CLIENT_FSM_EVENT_HANDSHAKE_WAIT_TMR_EXP */
    { 20000,     1 }, /* P2P_CLIENT_FSM_EVENT_HANDSHAKE_ACK_WAIT_TMR_EXP */
    {   100,    50 }, /* P2P_CLIENT_FSM_EVENT_RELAY_CONTROL_CONNECT_WAIT_TMR_EXP */
    {   100,    50 }, /* P2P_CLIENT_FSM_EVENT_RELAY_ALLOCATION_RESP_WAIT_TMR_EXP */
    {   100,    50 }, /* P2P_CLIENT_FSM_EVENT_RELAY_CREATE_PERMISSION_RESP_WAIT_TMR_EXP */
    {   100,   200 }, /* P2P_CLIENT_FSM_EVENT_RELAY_CONNECTION_ATTEMPT_WAIT_TMR_EXP */
    {   100,    50 }, /* P2P_CLIENT_FSM_EVENT_RELAY_DATA_CONNECT_WAIT_TMR_EXP */
    {   100,    50 }, /* P2P_CLIENT_FSM_EVENT_RELAY_CONNECTION_BIND_RESP_WAIT_TMR_EXP */
    { 30000,     1 }, /* P2P_CLIENT_FSM_EVENT_RELAY_REFRESH_REQ_WAIT_TMR_EXP */
    {   100,    10 }, /* P2P_CLIENT_FSM_EVENT_RELAY_REFRESH_RESP_WAIT_TMR_EXP */
    { 20000,     1 }, /* P2P_CLIENT_FSM_EVENT_LOGIN_WAIT_TMR_EXP */
    { 60000,     1 }, /* P2P_CLIENT_FSM_EVENT_HEARTBEAT_WAIT_TMR_EXP */
    {  1000,     5 }, /* P2P_CLIENT_FSM_EVENT_HEARTBEAT_RESP_RETRY_TMR_EXP */
    {  5000,    36 }, /* P2P_CLIENT_FSM_EVENT_HOLD_ACK_TMR_EXP */
};

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void *p2pClientCommThreadRoutine(void *threadInfo);
//-------------------------------------------------------------------------------------------------
static void startP2pClient(P2P_CLIENT_PROCESS_t *pP2pClientProc);
//-------------------------------------------------------------------------------------------------
static void stopP2pClient(P2P_CLIENT_PROCESS_t *pP2pClientProc);
//-------------------------------------------------------------------------------------------------
static BOOL isClientLocalCommPossible(P2P_CLIENT_PROCESS_t *pP2pClientProc);
//-------------------------------------------------------------------------------------------------
static BOOL isClientPublicCommPossible(P2P_CLIENT_PROCESS_t *pP2pClientProc);
//-------------------------------------------------------------------------------------------------
static void pollP2pClientSocket(P2P_CLIENT_PROCESS_t *pP2pClientProc);
//-------------------------------------------------------------------------------------------------
static void stopP2pClientProcess(P2P_CLIENT_PROCESS_t *pP2pClientProc);
//-------------------------------------------------------------------------------------------------
static void procP2pClientFsm(P2P_CLIENT_FSM_EVENT_e fsmEvent, P2P_CLIENT_PROCESS_t *pP2pClientProc, void *pFsmData, UINT32 dataLen);
//-------------------------------------------------------------------------------------------------
static void procP2pClientHandshakeInitTmrExpEvt(P2P_CLIENT_PROCESS_t *pP2pClientProc);
//-------------------------------------------------------------------------------------------------
static void procP2pClientHandshakeWaitTmrExpEvt(P2P_CLIENT_PROCESS_t *pP2pClientProc, BOOL *pIsTimerExpired);
//-------------------------------------------------------------------------------------------------
static void procP2pClientRelayControlConnectWaitTmrExpEvt(P2P_CLIENT_PROCESS_t *pP2pClientProc, BOOL *pIsTimerExpired);
//-------------------------------------------------------------------------------------------------
static void procP2pClientRelayAllocRespWaitTmrExpEvt(P2P_CLIENT_PROCESS_t *pP2pClientProc, BOOL *pIsTimerExpired);
//-------------------------------------------------------------------------------------------------
static void procP2pClientRelayPermissionRespWaitTmrExpEvt(P2P_CLIENT_PROCESS_t *pP2pClientProc, BOOL *pIsTimerExpired);
//-------------------------------------------------------------------------------------------------
static void procP2pClientRelayConnAttemptWaitTmrExpEvt(P2P_CLIENT_PROCESS_t *pP2pClientProc, BOOL *pIsTimerExpired);
//-------------------------------------------------------------------------------------------------
static void procP2pClientRelayDataConnectWaitTmrExpEvt(P2P_CLIENT_PROCESS_t *pP2pClientProc, BOOL *pIsTimerExpired);
//-------------------------------------------------------------------------------------------------
static void procP2pClientRelayConnBindRespWaitTmrExpEvt(P2P_CLIENT_PROCESS_t *pP2pClientProc, BOOL *pIsTimerExpired);
//-------------------------------------------------------------------------------------------------
static void procP2pClientHandshakeData(P2P_CLIENT_PROCESS_t *pP2pClientProc, CHAR *pDataBuff, UINT32 dataLen);
//-------------------------------------------------------------------------------------------------
static void procP2pClientHandshakeAckWaitTmrExpEvt(P2P_CLIENT_PROCESS_t *pP2pClientProc);
//-------------------------------------------------------------------------------------------------
static void procP2pClientHandshakeAckData(P2P_CLIENT_PROCESS_t *pP2pClientProc, CHAR *pDataBuff, UINT32 dataLen);
//-------------------------------------------------------------------------------------------------
static void procP2pClientLoginWaitTmrExpEvt(P2P_CLIENT_PROCESS_t *pP2pClientProc);
//-------------------------------------------------------------------------------------------------
static void procP2pClientLoginData(P2P_CLIENT_PROCESS_t *pP2pClientProc, CHAR *pDataBuff, UINT32 dataLen);
//-------------------------------------------------------------------------------------------------
static void procP2pClientHeartbeatWaitTmrExpEvt(P2P_CLIENT_PROCESS_t *pP2pClientProc);
//-------------------------------------------------------------------------------------------------
static void procP2pClientHeartbeatRespRetryTmrExpEvt(P2P_CLIENT_PROCESS_t *pP2pClientProc, BOOL *pIsTimerExpired);
//-------------------------------------------------------------------------------------------------
static void procP2pClientRelayRefreshReqWaitTmrExpEvt(P2P_CLIENT_PROCESS_t *pP2pClientProc);
//-------------------------------------------------------------------------------------------------
static void procP2pClientRelayRefreshRespWaitTmrExpEvt(P2P_CLIENT_PROCESS_t *pP2pClientProc, BOOL *pIsTimerExpired);
//-------------------------------------------------------------------------------------------------
static void procP2pClientHeartbeatData(P2P_CLIENT_PROCESS_t *pP2pClientProc, CHAR *pDataBuff, UINT32 dataLen);
//-------------------------------------------------------------------------------------------------
static void procP2pClientHoldAckTmrExpEvt(P2P_CLIENT_PROCESS_t *pP2pClientProc, BOOL *pIsTimerExpired);
//-------------------------------------------------------------------------------------------------
static void procP2pClientHoldData(P2P_CLIENT_PROCESS_t *pP2pClientProc, CHAR *pDataBuff, UINT32 dataLen);
//-------------------------------------------------------------------------------------------------
static BOOL startRelayControlChannel(P2P_CLIENT_PROCESS_t *pP2pClientProc);
//-------------------------------------------------------------------------------------------------
static void sendHandshakeReqMsg(P2P_CLIENT_PROCESS_t *pP2pClientProc);
//-------------------------------------------------------------------------------------------------
static void sendRelayControlMsg(INT32 sockFd, TURN_MSG_TYPE_e turnMsgType, P2P_CLIENT_PROCESS_t *pP2pClientProc);
//-------------------------------------------------------------------------------------------------
static UINT32 recvRelayControlMsg(INT32 sockFd, TURN_MSG_TYPE_e turnMsgType, P2P_CLIENT_PROCESS_t *pP2pClientProc, TURN_STATUS_e *pTurnStatus);
//-------------------------------------------------------------------------------------------------
static BOOL sendP2pClientSessionMsg(P2P_CLIENT_PROCESS_t *pP2pClientProc, P2P_SESSION_MSG_TYPE_e sessionMsgType);
//-------------------------------------------------------------------------------------------------
static void prepareMsgHeader(P2P_CLIENT_MSG_HEADER_t *pHeader, P2P_MSG_TYPE_e msgType, UINT32 msgUid, UINT32 payloadLen);
//-------------------------------------------------------------------------------------------------
static BOOL isValidMsgHeader(P2P_CLIENT_PROCESS_t *pP2pClientProc, P2P_CLIENT_MSG_HEADER_t *pHeader, UINT32 msgLen);
//-------------------------------------------------------------------------------------------------
static BOOL isValidSessionMsgHeader(P2P_CLIENT_PROCESS_t *pP2pClientProc, P2P_CLIENT_SESSION_MSG_t *pSessionPayload);
//-------------------------------------------------------------------------------------------------
static BOOL prepareSessionMsgPaylaod(P2P_SESSION_MSG_TYPE_e sessionMsgType, P2P_CLIENT_PROCESS_t *pP2pClientProc,
                                     P2P_CLIENT_SESSION_MSG_t *pSessionPayload, UINT32 *pPayloadLen);
//-------------------------------------------------------------------------------------------------
static void setRelayFailStatusCode(P2P_CLIENT_PROCESS_t *pP2pClientProc);
//-------------------------------------------------------------------------------------------------
static void changeP2pClientFsmState(P2P_CLIENT_PROCESS_t *pP2pClientProc, P2P_CLIENT_FSM_STATE_e clientFsmState);
//-------------------------------------------------------------------------------------------------
static void p2pClientFsmTmrHandler(UINT32 appData1, UINT32 appData2, BOOL isTimerExpired);
//-------------------------------------------------------------------------------------------------
static void startP2pClientFsmTmr(P2P_CLIENT_PROCESS_t *pP2pClientProc, P2P_CLIENT_FSM_EVENT_e fsmEvent);
//-------------------------------------------------------------------------------------------------
static void startP2pClientUtilTmr(P2P_CLIENT_PROCESS_t *pP2pClientProc, P2P_CLIENT_FSM_EVENT_e fsmEvent);
//-------------------------------------------------------------------------------------------------
static void procP2pClientSessionMsg(P2P_CLIENT_PROCESS_t *pP2pClientProc, CHAR *pDataBuff, UINT32 dataLen);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e procP2pClientControlMsg(P2P_CLIENT_PROCESS_t *pP2pClientProc, CHAR *pDataBuff, UINT32 dataLen);
//-------------------------------------------------------------------------------------------------
static void procP2pClientStreamMsg(P2P_CLIENT_PROCESS_t *pP2pClientProc, CHAR *pDataBuff, UINT32 dataLen);
//-------------------------------------------------------------------------------------------------
static UINT16 allocateP2pClientMsgIdx(P2P_CLIENT_PROCESS_t *pP2pClientProc, UINT32 msgUid);
//-------------------------------------------------------------------------------------------------
static void freeP2pClientMsgIdx(P2P_CLIENT_PROCESS_t *pP2pClientProc, UINT16 msgIdx);
//-------------------------------------------------------------------------------------------------
static void logoutP2pClientSession(P2P_CLIENT_PROCESS_t *pP2pClientProc);
//-------------------------------------------------------------------------------------------------
static void clearP2pClientDataXferFdInfo(UINT8 clientIdx, P2P_CLIENT_FD_TYPE_e fdType);
//-------------------------------------------------------------------------------------------------
static NW_CLIENT_TYPE_e getNwClientType(P2P_DEVICE_TYPE_e p2pClientType);
//-------------------------------------------------------------------------------------------------
static const CHAR *getSessionMsgTypeStr(P2P_SESSION_MSG_TYPE_e sessionMsgType);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   Init P2P client communication information
 */
void InitP2pClientCommInfo(void)
{
    UINT8                   clientIdx;
    P2P_CLIENT_FD_TYPE_e    fdType;

    for (clientIdx = 0; clientIdx < P2P_CLIENT_SUPPORT_MAX; clientIdx++)
    {
        MUTEX_INIT(p2pClientProc[clientIdx].socketLock, NULL);
    }

    for (fdType = 0; fdType < P2P_CLIENT_FD_TYPE_MAX; fdType++)
    {
        p2pClientFdInfo[fdType].clientIdx = P2P_CLIENT_SUPPORT_MAX;
        MUTEX_INIT(p2pClientFdInfo[fdType].clientLock, NULL);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Start communication with P2P client
 * @param   pClientInfo
 * @return
 */
BOOL StartP2pClientCommunication(P2P_CLIENT_INFO_t *pClientInfo)
{
    /* Create the client thread and start communication */
    return Utils_CreateThread(NULL, p2pClientCommThreadRoutine, pClientInfo, DETACHED_THREAD, P2P_CLIENT_THREAD_STACK_SIZE);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   P2P client process thread for individual client connections.
 * @param   threadInfo
 * @return  NULL
 */
static void *p2pClientCommThreadRoutine(void *threadInfo)
{
    P2P_CLIENT_PROCESS_t *pP2pClientProc;

    /* Validate client index */
    if (((P2P_CLIENT_INFO_t *)threadInfo)->clientIdx >= P2P_CLIENT_SUPPORT_MAX)
    {
        /* Invalid client index */
        EPRINT(P2P_MODULE, "invld client found: [client=%d]", ((P2P_CLIENT_INFO_t *)threadInfo)->clientIdx);
        pthread_exit(NULL);
    }

    /* Assign client index */
    pP2pClientProc = &p2pClientProc[((P2P_CLIENT_INFO_t *)threadInfo)->clientIdx];

    /* Set thread info */
    pP2pClientProc->pConnInfo = (P2P_CLIENT_INFO_t *)threadInfo;

    /* Set thread name */
    THREAD_START_INDEX("P2P_CLIENT", pP2pClientProc->pConnInfo->clientIdx);
    DPRINT(P2P_MODULE, "p2p client started: [client=%d]", pP2pClientProc->pConnInfo->clientIdx);

    /* Start p2p client information */
    startP2pClient(pP2pClientProc);

    /* loop until external trigger */
    while(CLIENT_PROC_STAT_RUNNING == GetP2pClientProcessRunStatus(pP2pClientProc->pConnInfo->clientIdx))
    {
        /* Wait for next tick */
        WaitNextTick(pP2pClientProc->pConnInfo->clientIdx, &pP2pClientProc->refTime,
                     P2P_CLIENT_TMR_TICK_RESOL_IN_NS, P2P_CLIENT_MAX_SLEEP_DELAY_IN_MS);

        /* Give tick to timer */
        SysTimerTick(&pP2pClientProc->pSysTimerListHead);

        /* Poll P2P client socket */
        pollP2pClientSocket(pP2pClientProc);
    }

    /* de-init p2p client communication */
    stopP2pClient(pP2pClientProc);

    /* exit thread */
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Initilize P2P client information, fsm state, confiugre timer and start fsm timer
 * @param   p2pClientProc
 */
static void startP2pClient(P2P_CLIENT_PROCESS_t *pP2pClientProc)
{
    P2P_CLIENT_FD_TYPE_e fdType;

    /* Initilize client info */
    pP2pClientProc->refTime = 0;
    pP2pClientProc->clientFsmState = P2P_CLIENT_STATE_HANDSHAKE;
    pP2pClientProc->isP2pClientHold = FALSE;
    pP2pClientProc->sessionMsgReqUid = GetRandomNum();
    pP2pClientProc->sessionMsgRespUid = 0;
    pP2pClientProc->serverSockFd = INVALID_CONNECTION;
    pP2pClientProc->acceptedClientFd = INVALID_CONNECTION;
    pP2pClientProc->clientLocalSockFd = INVALID_CONNECTION;
    pP2pClientProc->clientPublicSockFd = INVALID_CONNECTION;
    pP2pClientProc->relayControlChannelFd = INVALID_CONNECTION;
    pP2pClientProc->localClientConnStatus = IN_PROGRESS;
    pP2pClientProc->publicClientConnStatus = IN_PROGRESS;
    pP2pClientProc->dataRecvRetryCnt = 0;
    pP2pClientProc->relayServerIdx = 0;
    pP2pClientProc->challengeMsgRetryCnt = 0;
    pP2pClientProc->isRelayChnlAllocDone = FALSE;
    memset(&pP2pClientProc->clientMsgHeader, 0, sizeof(pP2pClientProc->clientMsgHeader));
    memset(pP2pClientProc->p2pMsgUid, 0, sizeof(pP2pClientProc->p2pMsgUid));
    memset(&pP2pClientProc->turnSessionInfo, 0, sizeof(pP2pClientProc->turnSessionInfo));

    for (fdType = 0; fdType < P2P_CLIENT_FD_TYPE_MAX; fdType++)
    {
        pP2pClientProc->dataSendFdInfo[fdType].sockFd = INVALID_CONNECTION;
        pP2pClientProc->dataSendFdInfo[fdType].clientMsgUid = 0;
    }

    /* Init timer object */
    SysTimerInit(&pP2pClientProc->pSysTimerListHead);

    /* Configure p2p client fsm timer */
    if (FALSE == SysTimerConfigure(&pP2pClientProc->fsmTimer, P2P_CLIENT_TMR_TICK_RESOL_IN_MS, p2pClientFsmTmrHandler))
    {
        /* failed to configure p2p client fsm timer */
        EPRINT(P2P_MODULE, "fail to configure p2p client fsm timer: [client=%d]", pP2pClientProc->pConnInfo->clientIdx);
        return;
    }

    /* Configure p2p client utility timer */
    if (FALSE == SysTimerConfigure(&pP2pClientProc->utilTimer, P2P_CLIENT_TMR_TICK_RESOL_IN_MS, p2pClientFsmTmrHandler))
    {
        /* failed to configure p2p client utility timer */
        EPRINT(P2P_MODULE, "fail to configure p2p client utility timer: [client=%d]", pP2pClientProc->pConnInfo->clientIdx);
        return;
    }

    /* Is this process for fallback to relay server? */
    if (TRUE == pP2pClientProc->pConnInfo->fallbackToRelayServer)
    {
        /* Start Relay Control channel */
        startRelayControlChannel(pP2pClientProc);

        /* Set p2p device id as username of turn server */
        snprintf((CHAR*)pP2pClientProc->turnSessionInfo.username, TURN_USERNAME_LEN_MAX, "%s-%s-%s",
                 GetP2pDeviceId(), pP2pClientProc->pConnInfo->clientUid, pP2pClientProc->pConnInfo->model);

        /* Prepare turn server raw password */
        snprintf((CHAR*)pP2pClientProc->turnSessionInfo.password, TURN_PASSWORD_LEN_MAX, "%c%c%c%c%c%c%c%s%c%c%c%c%c%c%c%c%c%c%c",
                 '#','p','*','2', '*','p', '@', (CHAR*)pP2pClientProc->turnSessionInfo.username, '@', 'r', '?', 'e', '?', 'l', '?', 'a', '?', 'y', '#');

        DPRINT(P2P_MODULE, "start communication with relay server: [client=%d], [username=%s]",
               pP2pClientProc->pConnInfo->clientIdx, (CHAR*)pP2pClientProc->turnSessionInfo.username);

        /* Convert raw password into base64 for better security */
        CHAR *pTurnPwdB64 = EncodeBase64((CHAR*)pP2pClientProc->turnSessionInfo.password, strlen((CHAR*)pP2pClientProc->turnSessionInfo.password));
        if (pTurnPwdB64 == NULL)
        {
            /* Fail to alloc memory */
            EPRINT(P2P_MODULE, "fail to alloc memory: [client=%d]", pP2pClientProc->pConnInfo->clientIdx);
            return;
        }

        /* Set base64 formatted password */
        snprintf((CHAR*)pP2pClientProc->turnSessionInfo.password, TURN_PASSWORD_LEN_MAX, "%s", pTurnPwdB64);
        free(pTurnPwdB64);
    }
    else
    {
        /* start p2p client fsm timer */
        startP2pClientFsmTmr(pP2pClientProc, P2P_CLIENT_FSM_EVENT_HANDSHAKE_INIT_WAIT_TMR_EXP);
        DPRINT(P2P_MODULE, "start p2p hole punch: [client=%d], [model=%s], [nvrLocalAddr=%s:%d], [nvrPublicAddr=%s:%d]",
               pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->model, pP2pClientProc->pConnInfo->localAddr.ip,
               pP2pClientProc->pConnInfo->localAddr.port, pP2pClientProc->pConnInfo->publicAddr.ip, pP2pClientProc->pConnInfo->publicAddr.port);

        do
        {
            /* Open server socket for client connection */
            if (FALSE == OpenP2pSocket(FALSE, &pP2pClientProc->pConnInfo->localAddr, &pP2pClientProc->serverSockFd))
            {
                EPRINT(P2P_MODULE, "fail to open handshake server socket: [client=%d], [err=%s]", pP2pClientProc->pConnInfo->clientIdx, STR_ERR);
                break;
            }
            else
            {
                DPRINT(P2P_MODULE, "start hole punch with client: [client=%d], [nvr=%s:%d]",
                       pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->localAddr.ip, pP2pClientProc->pConnInfo->localAddr.port);
            }

            /* Start listening on server socket */
            if (listen(pP2pClientProc->serverSockFd, 5) < STATUS_OK)
            {
                EPRINT(P2P_MODULE, "fail to listen handshake server socket: [client=%d], [err=%s]", pP2pClientProc->pConnInfo->clientIdx, STR_ERR);
                break;
            }

            if (TRUE == isClientLocalCommPossible(pP2pClientProc))
            {
                /* Open client socket for handshake on local end-point */
                if (FALSE == OpenP2pSocket(FALSE, &pP2pClientProc->pConnInfo->localAddr, &pP2pClientProc->clientLocalSockFd))
                {
                    EPRINT(P2P_MODULE, "fail to open handshake client local socket: [client=%d], [err=%s]", pP2pClientProc->pConnInfo->clientIdx, STR_ERR);
                    break;
                }

                /* Start creating connection with client local end-point */
                if (FALSE == ConnectP2pSocket(pP2pClientProc->clientLocalSockFd, &pP2pClientProc->pConnInfo->clientLocalAddr))
                {
                    pP2pClientProc->localClientConnStatus = FALSE;
                    EPRINT(P2P_MODULE, "fail to connect handshake client local endpoint: [client=%d], [clientLocalAddr=%s:%d]",
                           pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->clientLocalAddr.ip, pP2pClientProc->pConnInfo->clientLocalAddr.port);
                }
                else
                {
                    pP2pClientProc->localClientConnStatus = IN_PROGRESS;
                    DPRINT(P2P_MODULE, "connect in progress for local endpoint: [client=%d], [clientLocalAddr=%s:%d]",
                           pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->clientLocalAddr.ip, pP2pClientProc->pConnInfo->clientLocalAddr.port);
                }
            }

            if (TRUE == isClientPublicCommPossible(pP2pClientProc))
            {
                /* Open client socket for handshake on public end-point */
                if (FALSE == OpenP2pSocket(FALSE, &pP2pClientProc->pConnInfo->localAddr, &pP2pClientProc->clientPublicSockFd))
                {
                    EPRINT(P2P_MODULE, "fail to open handshake client public socket: [client=%d], [err=%s]", pP2pClientProc->pConnInfo->clientIdx, STR_ERR);
                    break;
                }

                /* Start creating connection with client public end-point */
                if (FALSE == ConnectP2pSocket(pP2pClientProc->clientPublicSockFd, &pP2pClientProc->pConnInfo->clientPublicAddr))
                {
                    pP2pClientProc->publicClientConnStatus = FALSE;
                    EPRINT(P2P_MODULE, "fail to connect handshake client public endpoint: [client=%d], [clientPublicAddr=%s:%d]",
                           pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->clientPublicAddr.ip, pP2pClientProc->pConnInfo->clientPublicAddr.port);
                }
                else
                {
                    pP2pClientProc->publicClientConnStatus = IN_PROGRESS;
                    DPRINT(P2P_MODULE, "connect in progress for public endpoint: [client=%d], [clientPublicAddr=%s:%d]",
                           pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->clientPublicAddr.ip, pP2pClientProc->pConnInfo->clientPublicAddr.port);
                }
            }

            /* Wait for connection establishment */
            return;

        }while(0);

        /* Close all sockets on failure */
        CloseSocket(&pP2pClientProc->serverSockFd);
        CloseSocket(&pP2pClientProc->clientLocalSockFd);
        CloseSocket(&pP2pClientProc->clientPublicSockFd);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   de-initilize p2p client information. Stop p2p client communication.
 * @param   p2pClientProc
 */
static void stopP2pClient(P2P_CLIENT_PROCESS_t *pP2pClientProc)
{
    P2P_CLIENT_FD_TYPE_e fdType;

    /* Logout P2P client */
    logoutP2pClientSession(pP2pClientProc);

    /* Check P2P client socket status:
     * It will be valid if client stop requested by main thread
     * It will be invalid when client stop triggerred internally */
    if ((pP2pClientProc->pConnInfo->sockFd != INVALID_CONNECTION) && (pP2pClientProc->clientFsmState != P2P_CLIENT_STATE_HANDSHAKE))
    {
        sendP2pClientSessionMsg(pP2pClientProc, P2P_SESSION_MSG_TYPE_CLOSE);
    }

    /* Check relay control channel status. If it is active then de-allocate channel in relay server */
    if ((pP2pClientProc->relayControlChannelFd != INVALID_CONNECTION) && (TRUE == pP2pClientProc->isRelayChnlAllocDone))
    {
        /* Send de-allocate request by setting 0 lifetime in refresh request */
        pP2pClientProc->turnSessionInfo.lifetime = 0;
        sendRelayControlMsg(pP2pClientProc->relayControlChannelFd, TURN_MSG_TYPE_REFRESH, pP2pClientProc);
    }

    /* Check receive fd type allocated to current client or not */
    for (fdType = 0; fdType < P2P_CLIENT_FD_TYPE_MAX; fdType++)
    {
        if (pP2pClientProc->dataSendFdInfo[fdType].sockFd != INVALID_CONNECTION)
        {
            /* Socket fd allocated to client, hence free it */
            clearP2pClientDataXferFdInfo(pP2pClientProc->pConnInfo->clientIdx, fdType);
        }
    }

    /* Close all sockets on termination */
    CloseSocket(&pP2pClientProc->serverSockFd);
    CloseSocket(&pP2pClientProc->acceptedClientFd);
    CloseSocket(&pP2pClientProc->clientLocalSockFd);
    CloseSocket(&pP2pClientProc->clientPublicSockFd);
    CloseSocket(&pP2pClientProc->relayControlChannelFd);

    /* Close client socket */
    CloseSocket(&pP2pClientProc->pConnInfo->sockFd);

    /* Reset other varaibles */
    pP2pClientProc->isRelayChnlAllocDone = FALSE;

    /* Exit client thread */
    SetP2pClientProcessRunStatus(pP2pClientProc->pConnInfo->clientIdx, CLIENT_PROC_STAT_EXIT);
    DPRINT(P2P_MODULE, "client thread exited: [client=%d], [model=%s]", pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->model);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Check client communication is possible or not using local endpoint
 * @param   pP2pClientProc
 * @return  TRUE is possible; FALSE otherwise
 */
static BOOL isClientLocalCommPossible(P2P_CLIENT_PROCESS_t *pP2pClientProc)
{
    /* If device local and client local IPs are same then do not send request, kernel will not send packet to destination */
    if (strcmp(pP2pClientProc->pConnInfo->localAddr.ip, pP2pClientProc->pConnInfo->clientLocalAddr.ip) == 0)
    {
        WPRINT(P2P_MODULE, "device local and client local ip are same: [client=%d], [clientLocalAddr=%s]",
               pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->clientLocalAddr.ip);
        return FALSE;
    }

    /* Client is connected on public IP */
    if ((strcmp(pP2pClientProc->pConnInfo->clientPublicAddr.ip, pP2pClientProc->pConnInfo->clientLocalAddr.ip) == 0)
             && (pP2pClientProc->pConnInfo->clientLocalAddr.port == pP2pClientProc->pConnInfo->clientPublicAddr.port))
    {
        DPRINT(P2P_MODULE, "client local and public end points are same: [client=%d], [clientPublicAddr=%s:%d]",
               pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->clientPublicAddr.ip, pP2pClientProc->pConnInfo->clientLocalAddr.port);
        return FALSE;
    }

    /* Local communication is possible */
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Check client communication is possible or not using public endpoint
 * @param   pP2pClientProc
 * @return  TRUE is possible; FALSE otherwise
 */
static BOOL isClientPublicCommPossible(P2P_CLIENT_PROCESS_t *pP2pClientProc)
{
    /* If device local and client public IPs are same then do not send request, kernel will not send packet to destination */
    if (strcmp(pP2pClientProc->pConnInfo->localAddr.ip, pP2pClientProc->pConnInfo->clientPublicAddr.ip) == 0)
    {
        WPRINT(P2P_MODULE, "device local and client public ip are same: [client=%d], [clientPublicAddr=%s]",
               pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->clientPublicAddr.ip);
        return FALSE;
    }

    /* Public communication is possible */
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Poll P2P client socket
 * @param   pP2pClientProc
 */
static void pollP2pClientSocket(P2P_CLIENT_PROCESS_t *pP2pClientProc)
{
    BOOL                retSts;
    UINT8               pktCnt;
    UINT32              recvLen;
    UINT32              payloadLen;
    CHAR                recvBuff[P2P_PACKET_SIZE_MAX+1];
    P2P_CLIENT_INFO_t   *pConnInfo = pP2pClientProc->pConnInfo;

    /* Validate socket fd */
    if (pConnInfo->sockFd == INVALID_CONNECTION)
    {
        /* FD is not valid. Hence no need to poll FDs */
        return;
    }

    /* Receive multiple packets; It is required when multiple packets queued on socket */
    for (pktCnt = 0; pktCnt < P2P_PACKET_POLL_CNT_MAX; pktCnt++)
    {
        /* Get TCP header before reading any data */
        if (pP2pClientProc->clientMsgHeader.payloadLen == 0)
        {
            /* Get available bytes on socket */
            if (GetAvailableBytesOnSocket(pConnInfo->sockFd) < (INT32)P2P_CLIENT_MSG_HEADER_LEN)
            {
                /* Required data not available */
                return;
            }

            /* Receive P2P TCP header */
            retSts = RecvP2pMsg(pConnInfo->sockFd, (CHAR *)&pP2pClientProc->clientMsgHeader, P2P_CLIENT_MSG_HEADER_LEN, &recvLen);

            /* No data available on socket */
            if (retSts == IN_PROGRESS)
            {
                return;
            }

            /* Receive Msg Failed */
            if (retSts == FAIL)
            {
                EPRINT(P2P_MODULE, "fail to recv P2P client header: [client=%d]", pConnInfo->clientIdx);
                return;
            }

            /* Remote connection closed */
            if (retSts == REFUSE)
            {
                /* Nothing to do */
                return;
            }

            /* Validate magic code */
            if (pP2pClientProc->clientMsgHeader.magicCode != P2P_MSG_MAGIC_CODE)
            {
                pP2pClientProc->dataRecvRetryCnt = 0;
                pP2pClientProc->clientMsgHeader.payloadLen = 0;
                EPRINT(P2P_MODULE, "invld p2p server magic code: [magicCode=0x%x]", pP2pClientProc->clientMsgHeader.magicCode);
                return;
            }

            /* Validate payload length */
            if ((pP2pClientProc->clientMsgHeader.payloadLen == 0)
                    || (pP2pClientProc->clientMsgHeader.payloadLen > (P2P_PACKET_SIZE_MAX - P2P_CLIENT_MSG_HEADER_LEN)))
            {
                pP2pClientProc->dataRecvRetryCnt = 0;
                pP2pClientProc->clientMsgHeader.payloadLen = 0;
                EPRINT(P2P_MODULE, "invld p2p client payload length: [client=%d], [payloadLen=%d]",
                       pConnInfo->clientIdx, pP2pClientProc->clientMsgHeader.payloadLen);
                return;
            }
        }

        /* Get available bytes on socket */
        if (GetAvailableBytesOnSocket(pConnInfo->sockFd) < (INT32)pP2pClientProc->clientMsgHeader.payloadLen)
        {
            /* Required data not available. Retry for data */
            pP2pClientProc->dataRecvRetryCnt++;
            if (pP2pClientProc->dataRecvRetryCnt > P2P_TCP_HEADER_DATA_RETRY_MAX)
            {
                /* No data available till timeout */
                EPRINT(P2P_MODULE, "fail to recv whole data: [client=%d], [payloadLen=%d]", pConnInfo->clientIdx, pP2pClientProc->clientMsgHeader.payloadLen);
                pP2pClientProc->clientMsgHeader.payloadLen = 0;
                pP2pClientProc->dataRecvRetryCnt = 0;
            }
            return;
        }

        /* Required data found on socket */
        memcpy(recvBuff, &pP2pClientProc->clientMsgHeader, P2P_CLIENT_MSG_HEADER_LEN);
        payloadLen = pP2pClientProc->clientMsgHeader.payloadLen;
        pP2pClientProc->clientMsgHeader.payloadLen = 0;
        pP2pClientProc->dataRecvRetryCnt = 0;

        /* Receive data from connected socket */
        retSts = RecvP2pMsg(pConnInfo->sockFd, recvBuff+P2P_CLIENT_MSG_HEADER_LEN, payloadLen, &recvLen);

        /* No data available on socket */
        if (retSts == IN_PROGRESS)
        {
            return;
        }

        /* Receive Msg Failed */
        if (retSts == FAIL)
        {
            EPRINT(P2P_MODULE, "fail to recv msg for P2P client process: [client=%d]", pConnInfo->clientIdx);
            return;
        }

        /* Remote connection closed */
        if (retSts == REFUSE)
        {
            /* Nothing to do */
            return;
        }

        /* Process received message in FSM */
        procP2pClientFsm(P2P_CLIENT_FSM_EVENT_MSG_RECV, pP2pClientProc, recvBuff, recvLen+P2P_CLIENT_MSG_HEADER_LEN);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Stop P2P client process as error found
 * @param   pP2pClientProc
 */
static void stopP2pClientProcess(P2P_CLIENT_PROCESS_t *pP2pClientProc)
{
    /* Logout P2P client */
    logoutP2pClientSession(pP2pClientProc);

    /* Stop all timers */
    SysTimerStop(&pP2pClientProc->pSysTimerListHead, &pP2pClientProc->fsmTimer);
    SysTimerStop(&pP2pClientProc->pSysTimerListHead, &pP2pClientProc->utilTimer);

    /* Close client socket */
    CloseSocket(&pP2pClientProc->pConnInfo->sockFd);

    /* Stop client and wait to exit */
    SetP2pClientProcessRunStatus(pP2pClientProc->pConnInfo->clientIdx, CLIENT_PROC_STAT_STOP);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Process p2p client fsm
 * @param   fsmEvent
 * @param   pP2pClientProc
 * @param   pFsmData
 * @param   dataLen - Received data length (Applicable to P2P_CLIENT_FSM_EVENT_MSG_RECV event)
 */
static void procP2pClientFsm(P2P_CLIENT_FSM_EVENT_e fsmEvent, P2P_CLIENT_PROCESS_t *pP2pClientProc, void *pFsmData, UINT32 dataLen)
{
    switch(pP2pClientProc->clientFsmState)
    {
        case P2P_CLIENT_STATE_HANDSHAKE:
        {
            if (fsmEvent == P2P_CLIENT_FSM_EVENT_HANDSHAKE_INIT_WAIT_TMR_EXP)
            {
                procP2pClientHandshakeInitTmrExpEvt(pP2pClientProc);
            }
            else if (fsmEvent == P2P_CLIENT_FSM_EVENT_HANDSHAKE_WAIT_TMR_EXP)
            {
                procP2pClientHandshakeWaitTmrExpEvt(pP2pClientProc, pFsmData);
            }
            else if (fsmEvent == P2P_CLIENT_FSM_EVENT_RELAY_CONTROL_CONNECT_WAIT_TMR_EXP)
            {
                procP2pClientRelayControlConnectWaitTmrExpEvt(pP2pClientProc, pFsmData);
            }
            else if (fsmEvent == P2P_CLIENT_FSM_EVENT_RELAY_ALLOCATION_RESP_WAIT_TMR_EXP)
            {
                procP2pClientRelayAllocRespWaitTmrExpEvt(pP2pClientProc, pFsmData);
            }
            else if (fsmEvent == P2P_CLIENT_FSM_EVENT_RELAY_CREATE_PERMISSION_RESP_WAIT_TMR_EXP)
            {
                procP2pClientRelayPermissionRespWaitTmrExpEvt(pP2pClientProc, pFsmData);
            }
            else if (fsmEvent == P2P_CLIENT_FSM_EVENT_RELAY_CONNECTION_ATTEMPT_WAIT_TMR_EXP)
            {
                procP2pClientRelayConnAttemptWaitTmrExpEvt(pP2pClientProc, pFsmData);
            }
            else if (fsmEvent == P2P_CLIENT_FSM_EVENT_RELAY_DATA_CONNECT_WAIT_TMR_EXP)
            {
                procP2pClientRelayDataConnectWaitTmrExpEvt(pP2pClientProc, pFsmData);
            }
            else if (fsmEvent == P2P_CLIENT_FSM_EVENT_RELAY_CONNECTION_BIND_RESP_WAIT_TMR_EXP)
            {
                procP2pClientRelayConnBindRespWaitTmrExpEvt(pP2pClientProc, pFsmData);
            }
            else if (fsmEvent == P2P_CLIENT_FSM_EVENT_MSG_RECV)
            {
                procP2pClientHandshakeData(pP2pClientProc, pFsmData, dataLen);
            }
            else
            {
                EPRINT(P2P_MODULE, "unhandled event: [client=%d], [fsm=%d], [event=%d]",
                       pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->clientFsmState, fsmEvent);
            }
        }
        break;

        case P2P_CLIENT_STATE_HANDSHAKE_ACK:
        {
            if (fsmEvent == P2P_CLIENT_FSM_EVENT_HANDSHAKE_ACK_WAIT_TMR_EXP)
            {
                procP2pClientHandshakeAckWaitTmrExpEvt(pP2pClientProc);
            }
            else if (fsmEvent == P2P_CLIENT_FSM_EVENT_MSG_RECV)
            {
                procP2pClientHandshakeAckData(pP2pClientProc, pFsmData, dataLen);
            }
            else
            {
                EPRINT(P2P_MODULE, "unhandled event: [client=%d], [fsm=%d], [event=%d]",
                       pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->clientFsmState, fsmEvent);
            }
        }
        break;

        case P2P_CLIENT_STATE_LOGIN:
        {
            if (fsmEvent == P2P_CLIENT_FSM_EVENT_LOGIN_WAIT_TMR_EXP)
            {
                procP2pClientLoginWaitTmrExpEvt(pP2pClientProc);
            }
            else if (fsmEvent == P2P_CLIENT_FSM_EVENT_RELAY_REFRESH_REQ_WAIT_TMR_EXP)
            {
                procP2pClientRelayRefreshReqWaitTmrExpEvt(pP2pClientProc);
            }
            else if (fsmEvent == P2P_CLIENT_FSM_EVENT_RELAY_REFRESH_RESP_WAIT_TMR_EXP)
            {
                procP2pClientRelayRefreshRespWaitTmrExpEvt(pP2pClientProc, pFsmData);
            }
            else if (fsmEvent == P2P_CLIENT_FSM_EVENT_MSG_RECV)
            {
                procP2pClientLoginData(pP2pClientProc, pFsmData, dataLen);
            }
            else
            {
                EPRINT(P2P_MODULE, "unhandled event: [client=%d], [fsm=%d], [event=%d]",
                       pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->clientFsmState, fsmEvent);
            }
        }
        break;

        case P2P_CLIENT_STATE_HEARTBEAT:
        {
            if (fsmEvent == P2P_CLIENT_FSM_EVENT_HEARTBEAT_WAIT_TMR_EXP)
            {
                procP2pClientHeartbeatWaitTmrExpEvt(pP2pClientProc);
            }
            else if (fsmEvent == P2P_CLIENT_FSM_EVENT_HEARTBEAT_RESP_RETRY_TMR_EXP)
            {
                procP2pClientHeartbeatRespRetryTmrExpEvt(pP2pClientProc, pFsmData);
            }
            else if (fsmEvent == P2P_CLIENT_FSM_EVENT_RELAY_REFRESH_REQ_WAIT_TMR_EXP)
            {
                procP2pClientRelayRefreshReqWaitTmrExpEvt(pP2pClientProc);
            }
            else if (fsmEvent == P2P_CLIENT_FSM_EVENT_RELAY_REFRESH_RESP_WAIT_TMR_EXP)
            {
                procP2pClientRelayRefreshRespWaitTmrExpEvt(pP2pClientProc, pFsmData);
            }
            else if (fsmEvent == P2P_CLIENT_FSM_EVENT_MSG_RECV)
            {
                procP2pClientHeartbeatData(pP2pClientProc, pFsmData, dataLen);
            }
            else
            {
                EPRINT(P2P_MODULE, "unhandled event: [client=%d], [fsm=%d], [event=%d]",
                       pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->clientFsmState, fsmEvent);
            }
        }
        break;

        case P2P_CLIENT_STATE_HOLD:
        {
            if (fsmEvent == P2P_CLIENT_FSM_EVENT_HOLD_ACK_TMR_EXP)
            {
                procP2pClientHoldAckTmrExpEvt(pP2pClientProc, pFsmData);
            }
            else if (fsmEvent == P2P_CLIENT_FSM_EVENT_RELAY_REFRESH_REQ_WAIT_TMR_EXP)
            {
                procP2pClientRelayRefreshReqWaitTmrExpEvt(pP2pClientProc);
            }
            else if (fsmEvent == P2P_CLIENT_FSM_EVENT_RELAY_REFRESH_RESP_WAIT_TMR_EXP)
            {
                procP2pClientRelayRefreshRespWaitTmrExpEvt(pP2pClientProc, pFsmData);
            }
            else if (fsmEvent == P2P_CLIENT_FSM_EVENT_MSG_RECV)
            {
                procP2pClientHoldData(pP2pClientProc, pFsmData, dataLen);
            }
            else
            {
                EPRINT(P2P_MODULE, "unhandled event: [client=%d], [fsm=%d], [event=%d]",
                       pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->clientFsmState, fsmEvent);
            }
        }
        break;

        default:
        {
            EPRINT(P2P_MODULE, "unhandled fsm state: [client=%d], [fsm=%d], [event=%d]",
                   pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->clientFsmState, fsmEvent);
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Process handshake init wait timer expired
 * @param   pP2pClientProc
 */
static void procP2pClientHandshakeInitTmrExpEvt(P2P_CLIENT_PROCESS_t *pP2pClientProc)
{
    /* Send handshake message */
    sendHandshakeReqMsg(pP2pClientProc);

    /* Start Handshake response wait timer */
    startP2pClientFsmTmr(pP2pClientProc, P2P_CLIENT_FSM_EVENT_HANDSHAKE_WAIT_TMR_EXP);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Handshake response wait timer expired
 * @param   pP2pClientProc
 * @param   pIsTimerExpired
 */
static void procP2pClientHandshakeWaitTmrExpEvt(P2P_CLIENT_PROCESS_t *pP2pClientProc, BOOL *pIsTimerExpired)
{
    /* Check timer expired or not */
    if (FALSE == *pIsTimerExpired)
    {
        /* Send handshake request */
        sendHandshakeReqMsg(pP2pClientProc);

        /* Wait till time expired */
        return;
    }

    /* Failed to do hole punching */
    EPRINT(P2P_MODULE, "handshake wait timer expired: hole punching failed: [client=%d]", pP2pClientProc->pConnInfo->clientIdx);

    /* Stop P2P client process */
    stopP2pClientProcess(pP2pClientProc);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Process relay server connect wait timer expire event for control channel
 * @param   pP2pClientProc
 * @param   pIsTimerExpired
 */
static void procP2pClientRelayControlConnectWaitTmrExpEvt(P2P_CLIENT_PROCESS_t *pP2pClientProc, BOOL *pIsTimerExpired)
{
    do
    {
        /* Is timer expired? or socket is not valid? */
        if ((TRUE == *pIsTimerExpired) || (pP2pClientProc->relayControlChannelFd == INVALID_CONNECTION))
        {
            /* Try communication with next relay server */
            WPRINT(P2P_MODULE, "relay control channel connect wait timer expired or connection init failed, try next relay server: [client=%d]",
                   pP2pClientProc->pConnInfo->clientIdx);
            if (FAIL == startRelayControlChannel(pP2pClientProc))
            {
                break;
            }

            /* We will try all relay servers before giving up */
            return;
        }

        /* Get connect status with relay server */
        BOOL relayControlConnStatus = IsP2pConnectionEstablished(pP2pClientProc->relayControlChannelFd);

        /* Is error found in connection? */
        if ((REFUSE == relayControlConnStatus) || (FAIL == relayControlConnStatus))
        {
            /* Try communication with next relay server */
            WPRINT(P2P_MODULE, "relay control channel connect failed, try next relay server: [client=%d], [status=%d]",
                   pP2pClientProc->pConnInfo->clientIdx, relayControlConnStatus);
            if (FAIL == startRelayControlChannel(pP2pClientProc))
            {
                break;
            }

            /* We will try all relay servers before giving up */
            return;
        }

        if (IN_PROGRESS == relayControlConnStatus)
        {
            /* Connection in progress */
            return;
        }

        /* Send relay allocate request without auth */
        sendRelayControlMsg(pP2pClientProc->relayControlChannelFd, TURN_MSG_TYPE_ALLOCATE, pP2pClientProc);

        /* Start relay allocate response wait timer */
        startP2pClientFsmTmr(pP2pClientProc, P2P_CLIENT_FSM_EVENT_RELAY_ALLOCATION_RESP_WAIT_TMR_EXP);
        return;

    }while(0);

    /* Failed to connect with relay server */
    EPRINT(P2P_MODULE, "relay control channel connect wait timer expired or fallback to relay failed: [client=%d], [model=%s]",
           pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->model);

    /* Stop P2P client process */
    stopP2pClientProcess(pP2pClientProc);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Process relay server allocation response wait timer expire event
 * @param   pP2pClientProc
 * @param   pIsTimerExpired
 */
static void procP2pClientRelayAllocRespWaitTmrExpEvt(P2P_CLIENT_PROCESS_t *pP2pClientProc, BOOL *pIsTimerExpired)
{
    do
    {
        /* Check timer expired or not */
        if (TRUE == *pIsTimerExpired)
        {
            /* Allocation failed with relay server */
            EPRINT(P2P_MODULE, "relay alloc resp wait timer expired, try next relay server: [client=%d]", pP2pClientProc->pConnInfo->clientIdx);

            /* Try communication with next relay server */
            if (FAIL == startRelayControlChannel(pP2pClientProc))
            {
                break;
            }

            /* We will try all relay servers before giving up */
            return;
        }

        /* Receive the allocate message response */
        TURN_STATUS_e turnStatus = TURN_STATUS_FAIL;
        if (0 == recvRelayControlMsg(pP2pClientProc->relayControlChannelFd, TURN_MSG_TYPE_ALLOCATE, pP2pClientProc, &turnStatus))
        {
            /* Wait till time expired */
            return;
        }

        /* Is it auth challenge? */
        if (turnStatus == TURN_STATUS_CHALLENGE)
        {
            pP2pClientProc->challengeMsgRetryCnt++;
            if (pP2pClientProc->challengeMsgRetryCnt >= RELAY_CHALLENGE_RETRY_MAX)
            {
                /* We tried max to provide valid credential */
                EPRINT(P2P_MODULE, "max relay challenge req reached for alloc: [client=%d]", pP2pClientProc->pConnInfo->clientIdx);
                break;
            }

            /* Send relay allocate request with auth */
            sendRelayControlMsg(pP2pClientProc->relayControlChannelFd, TURN_MSG_TYPE_ALLOCATE, pP2pClientProc);

            /* Start relay allocate response wait timer */
            startP2pClientFsmTmr(pP2pClientProc, P2P_CLIENT_FSM_EVENT_RELAY_ALLOCATION_RESP_WAIT_TMR_EXP);
            return;
        }

        /* Reset the challenge message retry counter as other response received */
        pP2pClientProc->challengeMsgRetryCnt = 0;

        /* Is it success to proceed? */
        if (turnStatus == TURN_STATUS_OK)
        {
            pP2pClientProc->isRelayChnlAllocDone = TRUE;
            DPRINT(P2P_MODULE, "relay allocation success: [client=%d], [lifetime=%d]",
                   pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->turnSessionInfo.lifetime);

            /* Send relay create permission request with auth */
            pP2pClientProc->turnSessionInfo.peerAddr = pP2pClientProc->pConnInfo->clientPublicAddr;
            sendRelayControlMsg(pP2pClientProc->relayControlChannelFd, TURN_MSG_TYPE_CREATE_PERMISSION, pP2pClientProc);

            /* Start relay create permission response wait timer */
            startP2pClientFsmTmr(pP2pClientProc, P2P_CLIENT_FSM_EVENT_RELAY_CREATE_PERMISSION_RESP_WAIT_TMR_EXP);
            return;
        }

        /* Error found in allocation */
        EPRINT(P2P_MODULE, "relay alloc failed by server: [client=%d], [server=%s:%d], [errCode=%d], [errMsg=%s]",
               pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->relayServerAddr.ip, pP2pClientProc->relayServerAddr.port,
               pP2pClientProc->turnSessionInfo.errCode, pP2pClientProc->turnSessionInfo.errMsg);

        /* Set relay server connection error code for response */
        setRelayFailStatusCode(pP2pClientProc);

        /* Try communication with next relay server */
        if (FAIL == startRelayControlChannel(pP2pClientProc))
        {
            break;
        }

        /* Communication started with new relay server */
        return;

    }while(0);

    /* Relay allocation failed */
    EPRINT(P2P_MODULE, "relay alloc resp wait timer expired or failed by server: [client=%d], [model=%s]",
           pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->model);

    /* Stop P2P client process */
    stopP2pClientProcess(pP2pClientProc);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Process relay server create permission response wait timer expire event
 * @param   pP2pClientProc
 * @param   pIsTimerExpired
 */
static void procP2pClientRelayPermissionRespWaitTmrExpEvt(P2P_CLIENT_PROCESS_t *pP2pClientProc, BOOL *pIsTimerExpired)
{
    /* Check timer expired or not */
    if (FALSE == *pIsTimerExpired)
    {
        /* Receive the create permission message response */
        TURN_STATUS_e turnStatus = TURN_STATUS_FAIL;
        if (0 == recvRelayControlMsg(pP2pClientProc->relayControlChannelFd, TURN_MSG_TYPE_CREATE_PERMISSION, pP2pClientProc, &turnStatus))
        {
            /* Wait till time expired */
            return;
        }

        /* Is it success to proceed? */
        if (turnStatus == TURN_STATUS_OK)
        {
            /* Set allocated relay server address for peer (mobile client) */
            pP2pClientProc->pConnInfo->peerRelayAddr = pP2pClientProc->turnSessionInfo.relayAddr;
            pP2pClientProc->pConnInfo->isPeerRelayAddrAllocated = TRUE;
            DPRINT(P2P_MODULE, "relay peer addr allocated: [client=%d], [peerAddr=%s:%d], [relayAddr=%s:%d]", pP2pClientProc->pConnInfo->clientIdx,
                   pP2pClientProc->pConnInfo->clientPublicAddr.ip, pP2pClientProc->pConnInfo->clientPublicAddr.port,
                   pP2pClientProc->turnSessionInfo.relayAddr.ip, pP2pClientProc->turnSessionInfo.relayAddr.port);

            /* Start relay connection attempt indication wait timer */
            startP2pClientFsmTmr(pP2pClientProc, P2P_CLIENT_FSM_EVENT_RELAY_CONNECTION_ATTEMPT_WAIT_TMR_EXP);
            return;
        }

        /* Create permission failed */
        EPRINT(P2P_MODULE, "relay create permission failed by server: [client=%d], [errCode=%d], [errMsg=%s]",
               pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->turnSessionInfo.errCode, pP2pClientProc->turnSessionInfo.errMsg);

        /* Set relay server connection error code for response */
        setRelayFailStatusCode(pP2pClientProc);
    }

    /* Create permission failed with relay server */
    EPRINT(P2P_MODULE, "relay create permission resp wait timer expired or failed by server: [client=%d], [model=%s]",
           pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->model);

    /* Stop P2P client process */
    stopP2pClientProcess(pP2pClientProc);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Process relay server connection attempt indication wait timer expire event
 * @param   pP2pClientProc
 * @param   pIsTimerExpired
 */
static void procP2pClientRelayConnAttemptWaitTmrExpEvt(P2P_CLIENT_PROCESS_t *pP2pClientProc, BOOL *pIsTimerExpired)
{
    do
    {
        /* Check timer expired or not */
        if (TRUE == *pIsTimerExpired)
        {
            break;
        }

        /* Receive the connection attempt message indication */
        TURN_STATUS_e turnStatus = TURN_STATUS_FAIL;
        if (0 == recvRelayControlMsg(pP2pClientProc->relayControlChannelFd, TURN_MSG_TYPE_CONNECTION_ATTEMPT, pP2pClientProc, &turnStatus))
        {
            /* Wait till time expired */
            return;
        }

        /* Is it not a valid connection attempt? */
        if (turnStatus != TURN_STATUS_OK)
        {
            /* Connection bind rejected with error */
            WPRINT(P2P_MODULE, "invld connection attempt resp rcvd from server, waiting...: [client=%d], [turnStatus=%s], [errCode=%d], [errMsg=%s]",
                   pP2pClientProc->pConnInfo->clientIdx, pTurnMsgStatusStr[turnStatus],
                   pP2pClientProc->turnSessionInfo.errCode, pP2pClientProc->turnSessionInfo.errMsg);
            return;
        }

        /* Set peer address as receive address */
        pP2pClientProc->recvAddr = pP2pClientProc->turnSessionInfo.peerAddr;

        /* Start relay connection attempt indication wait timer */
        DPRINT(P2P_MODULE, "relay connection attempted: [client=%d], [requsetAddr=%s:%d], [peerAddr=%s:%d]",
               pP2pClientProc->pConnInfo->clientIdx,
               pP2pClientProc->pConnInfo->clientPublicAddr.ip, pP2pClientProc->pConnInfo->clientPublicAddr.port,
               pP2pClientProc->turnSessionInfo.peerAddr.ip, pP2pClientProc->turnSessionInfo.peerAddr.port);

        /* Open client socket to communicate with relay server for data communication */
        pP2pClientProc->pConnInfo->localAddr.port = 0;
        if (FALSE == OpenP2pSocket(FALSE, &pP2pClientProc->pConnInfo->localAddr, &pP2pClientProc->clientLocalSockFd))
        {
            EPRINT(P2P_MODULE, "fail to open relay data channel socket: [client=%d], [err=%s]", pP2pClientProc->pConnInfo->clientIdx, STR_ERR);
            break;
        }

        /* Start creating connection with relay server for data channel */
        if (FALSE == ConnectP2pSocket(pP2pClientProc->clientLocalSockFd, &pP2pClientProc->relayServerAddr))
        {
            /* Close the socket */
            CloseSocket(&pP2pClientProc->clientLocalSockFd);
            EPRINT(P2P_MODULE, "fail to connect relay data channel: [client=%d], [server=%s:%d]",
                   pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->relayServerAddr.ip, pP2pClientProc->relayServerAddr.port);
            break;
        }

        /* Connection in progress for relay data channel */
        DPRINT(P2P_MODULE, "connect in progress for relay data channel: [client=%d], [server=%s:%d]",
               pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->relayServerAddr.ip, pP2pClientProc->relayServerAddr.port);

        /* Start the data connection wait timer */
        startP2pClientFsmTmr(pP2pClientProc, P2P_CLIENT_FSM_EVENT_RELAY_DATA_CONNECT_WAIT_TMR_EXP);
        return;

    } while(0);

    /* Connection attempt failed with relay server */
    EPRINT(P2P_MODULE, "relay connection attempt resp wait timer expired or fallback to relay failed: [client=%d], [model=%s]",
           pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->model);

    /* Stop P2P client process */
    stopP2pClientProcess(pP2pClientProc);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Process relay server connect wait timer expire event for data channel
 * @param   pP2pClientProc
 * @param   pIsTimerExpired
 */
static void procP2pClientRelayDataConnectWaitTmrExpEvt(P2P_CLIENT_PROCESS_t *pP2pClientProc, BOOL *pIsTimerExpired)
{
    do
    {
        /* Check timer expired or not */
        if (TRUE == *pIsTimerExpired)
        {
            break;
        }

        /* Is valid socket? */
        if (pP2pClientProc->clientLocalSockFd == INVALID_CONNECTION)
        {
            break;
        }

        /* Get connect status with relay server */
        BOOL relayDataConnStatus = IsP2pConnectionEstablished(pP2pClientProc->clientLocalSockFd);
        if ((REFUSE == relayDataConnStatus) || (FAIL == relayDataConnStatus))
        {
            /* Connection closed by relay server or Other error occurred */
            EPRINT(P2P_MODULE, "relay data channel connect failed: [client=%d], [status=%d]", pP2pClientProc->pConnInfo->clientIdx, relayDataConnStatus);
            break;
        }

        if (IN_PROGRESS == relayDataConnStatus)
        {
            /* Connection in progress */
            return;
        }

        /* Send relay connection bind request */
        sendRelayControlMsg(pP2pClientProc->clientLocalSockFd, TURN_MSG_TYPE_CONNECTION_BIND, pP2pClientProc);

        /* Start relay connection bind response wait timer */
        startP2pClientFsmTmr(pP2pClientProc, P2P_CLIENT_FSM_EVENT_RELAY_CONNECTION_BIND_RESP_WAIT_TMR_EXP);
        return;

    }while(0);

    /* Failed to connect with relay server */
    EPRINT(P2P_MODULE, "relay data channel connect wait timer expired or fallback to relay failed: [client=%d], [model=%s]",
           pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->model);

    /* Stop P2P client process */
    stopP2pClientProcess(pP2pClientProc);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Process relay server connection bind response wait timer expire event
 * @param   pP2pClientProc
 * @param   pIsTimerExpired
 */
static void procP2pClientRelayConnBindRespWaitTmrExpEvt(P2P_CLIENT_PROCESS_t *pP2pClientProc, BOOL *pIsTimerExpired)
{
    do
    {
        /* Check timer expired or not */
        if (TRUE == *pIsTimerExpired)
        {
            /* Connection bind failed with relay server */
            EPRINT(P2P_MODULE, "relay connection bind resp wait timer expired: [client=%d], [model=%s]",
                   pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->model);
            break;
        }

        /* Receive the connection bind message response */
        TURN_STATUS_e turnStatus = TURN_STATUS_FAIL;
        UINT32 msgLen = recvRelayControlMsg(pP2pClientProc->clientLocalSockFd, TURN_MSG_TYPE_CONNECTION_BIND, pP2pClientProc, &turnStatus);
        if (0 == msgLen)
        {
            /* Wait till time expired */
            return;
        }

        /* Is it rejected with error? */
        if (turnStatus != TURN_STATUS_OK)
        {
            /* Connection bind rejected with error */
            EPRINT(P2P_MODULE, "relay connection bind failed by server: [client=%d], [turnStatus=%s], [errCode=%d], [errMsg=%s]",
                   pP2pClientProc->pConnInfo->clientIdx, pTurnMsgStatusStr[turnStatus],
                   pP2pClientProc->turnSessionInfo.errCode, pP2pClientProc->turnSessionInfo.errMsg);
            break;
        }

        /* Set messgae length with actual data received length. It is possible that this message have connection bind response
         * as well as client login request. Turn session message length will be set as TURN message length. To check whether
         * complete login message is received or not, init with actual message data length. */
        pP2pClientProc->turnSessionInfo.msgLen = msgLen;

        /* Get turn message length. */
        INT32 stunMsgLen = Turn_GetMsgLen(&pP2pClientProc->turnSessionInfo);
        if ((stunMsgLen == -1) || (stunMsgLen > (INT32)msgLen))
        {
            /* Login message is not received yet. Wait for it */
            WPRINT(P2P_MODULE, "connection bind success but login msg not rcvd yet, waiting..: [client=%d], [stunMsgLen=%d], [msgLen=%d]",
                   pP2pClientProc->pConnInfo->clientIdx, stunMsgLen, msgLen);
            return;
        }

        /* Check message length and received data length */
        if ((stunMsgLen + P2P_CLIENT_MSG_HEADER_LEN) > msgLen)
        {
            /* Partial login data received */
            WPRINT(P2P_MODULE, "connection bind success but login msg header not rcvd yet, waiting..: [client=%d], [msgLen=%d]",
                   pP2pClientProc->pConnInfo->clientIdx, msgLen);
            return;
        }

        /* Check message header */
        P2P_CLIENT_MSG_HEADER_t *pMsgHeader = (P2P_CLIENT_MSG_HEADER_t*)&pP2pClientProc->turnSessionInfo.msgBuf[stunMsgLen];

        /* Validate magic code */
        if (pMsgHeader->magicCode != P2P_MSG_MAGIC_CODE)
        {
            EPRINT(P2P_MODULE, "connection bind success but invld p2p server magic code: [client=%d], [magicCode=0x%x]",
                   pP2pClientProc->pConnInfo->clientIdx, pMsgHeader->magicCode);
            break;
        }

        /* Validate payload length */
        if ((pMsgHeader->payloadLen == 0) || (pMsgHeader->payloadLen > (P2P_PACKET_SIZE_MAX - P2P_CLIENT_MSG_HEADER_LEN)))
        {
            EPRINT(P2P_MODULE, "connection bind success but invld payload length: [client=%d], [payloadLen=%d], [msgLen=%d]",
                   pP2pClientProc->pConnInfo->clientIdx, pMsgHeader->payloadLen, msgLen);
            break;
        }

        /* Is required data received? */
        if ((msgLen - (stunMsgLen + P2P_CLIENT_MSG_HEADER_LEN)) < pMsgHeader->payloadLen)
        {
            /* Required data not received yet */
            WPRINT(P2P_MODULE, "connection bind success but partial login msg: [client=%d], [msgLen=%d]", pP2pClientProc->pConnInfo->clientIdx, msgLen);
            return;
        }

        /* Reset message length as we have received complete login message */
        pP2pClientProc->turnSessionInfo.msgLen = 0;

        /* Change FSM state */
        changeP2pClientFsmState(pP2pClientProc, P2P_CLIENT_STATE_LOGIN);

        /* Start login request wait timer */
        startP2pClientFsmTmr(pP2pClientProc, P2P_CLIENT_FSM_EVENT_LOGIN_WAIT_TMR_EXP);

        /* Now start receiving data as normal socket */
        pP2pClientProc->pConnInfo->sockFd = pP2pClientProc->clientLocalSockFd;
        pP2pClientProc->clientLocalSockFd = INVALID_CONNECTION;

        /* Process received message in FSM */
        procP2pClientFsm(P2P_CLIENT_FSM_EVENT_MSG_RECV, pP2pClientProc, &pP2pClientProc->turnSessionInfo.msgBuf[stunMsgLen], (msgLen - stunMsgLen));

        /* Send relay connection refresh and wait for response */
        procP2pClientRelayRefreshReqWaitTmrExpEvt(pP2pClientProc);
        return;

    }while(0);

    /* Connection bind failed with relay server */
    EPRINT(P2P_MODULE, "connection bind resp wait timer expired or failed by server: [client=%d], [model=%s]",
           pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->model);

    /* Stop P2P client process */
    stopP2pClientProcess(pP2pClientProc);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Process received handshake data
 * @param   pP2pClientProc
 * @param   pDataBuff
 * @param   dataLen
 */
static void procP2pClientHandshakeData(P2P_CLIENT_PROCESS_t *pP2pClientProc, CHAR *pDataBuff, UINT32 dataLen)
{
    /* Validate message length */
    if (dataLen < P2P_SESSION_MSG_MIN_LEN)
    {
        /* Invalid message length received */
        EPRINT(P2P_MODULE, "invld handshake msg length recv: [client=%d], [dataLen=%d]", pP2pClientProc->pConnInfo->clientIdx, dataLen);
        return;
    }

    /* Typecast with client message, as it is expected over here */
    P2P_CLIENT_MSG_t *pClientMsg = (P2P_CLIENT_MSG_t *)pDataBuff;

    /* Validate received header */
    if (FALSE == isValidMsgHeader(pP2pClientProc, &pClientMsg->header, dataLen))
    {
        /* Invalid message header received */
        EPRINT(P2P_MODULE, "invld msg header recv: [client=%d]", pP2pClientProc->pConnInfo->clientIdx);
        return;
    }

    /* Received message must be session message */
    if (pClientMsg->header.msgType != P2P_MSG_TYPE_SESSION)
    {
        /* Invalid message type received */
        EPRINT(P2P_MODULE, "invld msg type recv: [client=%d], [msgType=%d]", pP2pClientProc->pConnInfo->clientIdx, pClientMsg->header.msgType);
        return;
    }

    /* Get session message header */
    P2P_CLIENT_SESSION_MSG_t *pSessionPayload = &pClientMsg->payload.session;

    /* Validate message type. It must be session message */
    if (pSessionPayload->msgType != P2P_SESSION_MSG_TYPE_HANDSHAKE_REQ)
    {
        /* Invalid message type received */
        EPRINT(P2P_MODULE, "invld session msg type recv: [client=%d], [msgType=%d]", pP2pClientProc->pConnInfo->clientIdx, pSessionPayload->msgType);
        return;
    }

    /* Validate session message header */
    if (FALSE == isValidSessionMsgHeader(pP2pClientProc, pSessionPayload))
    {
        /* Invalid session message header received */
        EPRINT(P2P_MODULE, "invld session msg header recv: [client=%d], [msgType=%d]", pP2pClientProc->pConnInfo->clientIdx, pSessionPayload->msgType);
        return;
    }

    DPRINT(P2P_MODULE, "P2P connection established: [client=%d], [clientType=%d], [clientUid=%s], [ip=%s], [port=%d]",
           pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->clientType,
           pP2pClientProc->pConnInfo->clientUid, pP2pClientProc->recvAddr.ip, pP2pClientProc->recvAddr.port);

    /* Store message unique ID for response */
    pP2pClientProc->sessionMsgRespUid = pClientMsg->header.msgUid;

    /* We have received handshake request */
    DPRINT(P2P_MODULE, "handshake req recv: [client=%d]", pP2pClientProc->pConnInfo->clientIdx);

    /* Send Handshake response */
    sendP2pClientSessionMsg(pP2pClientProc, P2P_SESSION_MSG_TYPE_HANDSHAKE_RESP);

    /* Start Handshake response wait timer */
    startP2pClientFsmTmr(pP2pClientProc, P2P_CLIENT_FSM_EVENT_HANDSHAKE_ACK_WAIT_TMR_EXP);

    /* Change FSM state */
    changeP2pClientFsmState(pP2pClientProc, P2P_CLIENT_STATE_HANDSHAKE_ACK);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   P2P client handshake wait timer expired
 * @param   pP2pClientProc
 */
static void procP2pClientHandshakeAckWaitTmrExpEvt(P2P_CLIENT_PROCESS_t *pP2pClientProc)
{
    /* Failed to do hole punching */
    EPRINT(P2P_MODULE, "handshake ack wait timer expired: hole punching failed: [client=%d]", pP2pClientProc->pConnInfo->clientIdx);

    /* Stop P2P client process */
    stopP2pClientProcess(pP2pClientProc);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   P2P client handshake ack data receive
 * @param   pP2pClientProc
 * @param   pDataBuff
 * @param   dataLen
 */
static void procP2pClientHandshakeAckData(P2P_CLIENT_PROCESS_t *pP2pClientProc, CHAR *pDataBuff, UINT32 dataLen)
{
    /* Validate message length */
    if (dataLen < P2P_SESSION_MSG_MIN_LEN)
    {
        /* Invalid message length received */
        EPRINT(P2P_MODULE, "invld handshake ack msg length recv: [client=%d]", pP2pClientProc->pConnInfo->clientIdx);
        return;
    }

    /* Typecast with client message, as it is expected over here */
    P2P_CLIENT_MSG_t *pClientMsg = (P2P_CLIENT_MSG_t *)pDataBuff;

    /* Validate received header */
    if (FALSE == isValidMsgHeader(pP2pClientProc, &pClientMsg->header, dataLen))
    {
        /* Invalid message header received */
        EPRINT(P2P_MODULE, "invld msg header recv: [client=%d]", pP2pClientProc->pConnInfo->clientIdx);
        return;
    }

    /* Received message must be session message */
    if (pClientMsg->header.msgType != P2P_MSG_TYPE_SESSION)
    {
        /* Invalid message type received */
        EPRINT(P2P_MODULE, "invld msg type recv: [client=%d], [msgType=%d]", pP2pClientProc->pConnInfo->clientIdx, pClientMsg->header.msgType);
        return;
    }

    /* Get session message header */
    P2P_CLIENT_SESSION_MSG_t *pSessionPayload = &pClientMsg->payload.session;

    /* Validate message type. It must be session message */
    if ((pSessionPayload->msgType != P2P_SESSION_MSG_TYPE_HANDSHAKE_REQ) && (pSessionPayload->msgType != P2P_SESSION_MSG_TYPE_HANDSHAKE_ACK))
    {
        /* Invalid message type received */
        EPRINT(P2P_MODULE, "invld session msg type recv: [client=%d], [msgType=%d]", pP2pClientProc->pConnInfo->clientIdx, pSessionPayload->msgType);
        return;
    }

    /* Validate session message header */
    if (FALSE == isValidSessionMsgHeader(pP2pClientProc, pSessionPayload))
    {
        /* Invalid session message header received */
        EPRINT(P2P_MODULE, "invld session msg header recv: [client=%d], [msgType=%d]", pP2pClientProc->pConnInfo->clientIdx, pSessionPayload->msgType);
        return;
    }

    /* Store message unique ID for response */
    pP2pClientProc->sessionMsgRespUid = pClientMsg->header.msgUid;

    if (pSessionPayload->msgType == P2P_SESSION_MSG_TYPE_HANDSHAKE_REQ)
    {
        /* We have received handshake again request */
        DPRINT(P2P_MODULE, "handshake req recv again: [client=%d]", pP2pClientProc->pConnInfo->clientIdx);

        /* Send Handshake response */
        sendP2pClientSessionMsg(pP2pClientProc, P2P_SESSION_MSG_TYPE_HANDSHAKE_RESP);

        /* Start Handshake response wait timer */
        startP2pClientFsmTmr(pP2pClientProc, P2P_CLIENT_FSM_EVENT_HANDSHAKE_ACK_WAIT_TMR_EXP);

        /* Change FSM state */
        changeP2pClientFsmState(pP2pClientProc, P2P_CLIENT_STATE_HANDSHAKE_ACK);
    }
    else
    {
        /* We have received handshake ack */
        DPRINT(P2P_MODULE, "handshake ack recv: [client=%d]", pP2pClientProc->pConnInfo->clientIdx);

        /* Start login request wait timer */
        startP2pClientFsmTmr(pP2pClientProc, P2P_CLIENT_FSM_EVENT_LOGIN_WAIT_TMR_EXP);

        /* Change FSM state */
        changeP2pClientFsmState(pP2pClientProc, P2P_CLIENT_STATE_LOGIN);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   P2P client login wait timer expired
 * @param   pP2pClientProc
 */
static void procP2pClientLoginWaitTmrExpEvt(P2P_CLIENT_PROCESS_t *pP2pClientProc)
{
    /* Login request not received */
    EPRINT(P2P_MODULE, "login wait timer expired: [client=%d]", pP2pClientProc->pConnInfo->clientIdx);

    /* Stop P2P client process */
    stopP2pClientProcess(pP2pClientProc);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   P2P client login data receive
 * @param   pP2pClientProc
 * @param   pDataBuff
 * @param   dataLen
 */
static void procP2pClientLoginData(P2P_CLIENT_PROCESS_t *pP2pClientProc, CHAR *pDataBuff, UINT32 dataLen)
{
    UINT8 p2pHeaderLen = P2P_CLIENT_MSG_HEADER_LEN;

    /* Validate message length */
    if (dataLen < p2pHeaderLen)
    {
        EPRINT(P2P_MODULE, "invld login msg length: [client=%d], [dataLen=%d]", pP2pClientProc->pConnInfo->clientIdx, dataLen);
        return;
    }

    /* Typecast with client message, as it is expected over here */
    P2P_CLIENT_MSG_t *pClientMsg = (P2P_CLIENT_MSG_t *)pDataBuff;

    /* Validate received header */
    if (FALSE == isValidMsgHeader(pP2pClientProc, &pClientMsg->header, dataLen))
    {
        /* Invalid message header received */
        EPRINT(P2P_MODULE, "invld msg header recv: [client=%d]", pP2pClientProc->pConnInfo->clientIdx);
        return;
    }

    /* It should be control message type */
    if (pClientMsg->header.msgType != P2P_MSG_TYPE_CONTROL)
    {
        /* Invalid message type received */
        EPRINT(P2P_MODULE, "invld msg type recv: [client=%d], [msgType=%d]", pP2pClientProc->pConnInfo->clientIdx, pClientMsg->header.msgType);
        return;
    }

    /* Terminate message with null */
    pDataBuff[dataLen] = '\0';

    CHAR    *pMsgData = pDataBuff + p2pHeaderLen;
    UINT8   msgId = MAX_HEADER_REQ;

    /* Validate SOM in message */
    if (pMsgData[0] != SOM)
    {
        EPRINT(P2P_MODULE, "SOM not found in msg: [client=%d], [msg=%s]", pP2pClientProc->pConnInfo->clientIdx, pMsgData);
        return;
    }

    /* Remove SOM from message */
    pMsgData++;

    /* Parse message id */
    if (FindHeaderIndex(&pMsgData, &msgId) != CMD_SUCCESS)
    {
        EPRINT(P2P_MODULE, "fail to parse header index: [client=%d], [msg=%s]", pP2pClientProc->pConnInfo->clientIdx, pMsgData);
        return;
    }

    /* It must be login request */
    if (msgId != REQ_LOG)
    {
        EPRINT(P2P_MODULE, "fail to get header index: [client=%d], [msg=%s]", pP2pClientProc->pConnInfo->clientIdx, pMsgData);
        return;
    }

    UINT16 msgIdx = allocateP2pClientMsgIdx(pP2pClientProc, pClientMsg->header.msgUid);
    if (msgIdx >= P2P_CLIENT_STORE_MSG_MAX)
    {
        EPRINT(P2P_MODULE, "fail to alloc new msgIdx: [client=%d], [model=%s], [msgUid=0x%x]",
               pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->model, pClientMsg->header.msgUid);
        return;
    }

    do
    {
        UINT32              remoteIpNw;
        UINT8               userLoginSessionIdx = MAX_NW_CLIENT;
        SOCK_ADDR_INFO_u    remoteClientInfo;

        /*  Convert remote ip from string to network format */
        if (inet_pton(AF_INET, pP2pClientProc->recvAddr.ip, &remoteIpNw) != 1)
        {
            EPRINT(P2P_MODULE, "fail to convert ip in nw format: [ip=%s]", pP2pClientProc->recvAddr.ip);
            break;
        }

        memset(&remoteClientInfo, 0, sizeof(remoteClientInfo));
        // TODO : Implement IPv6 support by dynamically filling appropriate structure based on client type
        remoteClientInfo.sockAddr4.sin_family = AF_INET;
        remoteClientInfo.sockAddr4.sin_addr.s_addr = remoteIpNw;

        /* Check client is already logged in or not */
        if (pP2pClientProc->pConnInfo->userLoginSessionIdx < MAX_NW_CLIENT)
        {
            /* Logout client and login again */
            EPRINT(P2P_MODULE, "client is already login: [client=%d], [userLoginSessionIdx=%d], [userLoginSessionUid=%d]",
                   pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->userLoginSessionIdx, pP2pClientProc->pConnInfo->userLoginSessionUid);
            logoutP2pClientSession(pP2pClientProc);
        }

        /* Process login request */
        if (CMD_SUCCESS != ProcessClientLoginRequest(pMsgData, &remoteClientInfo, pP2pClientProc->p2pMsgUid[msgIdx].localMsgUid, CLIENT_CB_TYPE_P2P,
                                                     getNwClientType(pP2pClientProc->pConnInfo->clientType), &userLoginSessionIdx))
        {
            break;
        }

        /* Free client message index */
        freeP2pClientMsgIdx(pP2pClientProc, msgIdx);

        /* To avoid intermediate status for P2P process thread */
        pP2pClientProc->pConnInfo->userLoginSessionUid = GetUserSessionId(userLoginSessionIdx);
        pP2pClientProc->pConnInfo->userLoginSessionIdx = userLoginSessionIdx;

        /* Start heartbeat wait timer */
        DPRINT(P2P_MODULE, "login success: [client=%d], [userLoginSessionIdx=%d], [userLoginSessionUid=%d]",
               pP2pClientProc->pConnInfo->clientIdx, userLoginSessionIdx, pP2pClientProc->pConnInfo->userLoginSessionUid);
        startP2pClientFsmTmr(pP2pClientProc, P2P_CLIENT_FSM_EVENT_HEARTBEAT_WAIT_TMR_EXP);

        /* Change FSM state */
        changeP2pClientFsmState(pP2pClientProc, P2P_CLIENT_STATE_HEARTBEAT);

        /* Client logged in successfully */
        return;

    }while(0);

    /* Free client message index */
    freeP2pClientMsgIdx(pP2pClientProc, msgIdx);

    /* Start login request wait timer */
    EPRINT(P2P_MODULE, "login failed: [client=%d], [model=%s]", pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->model);
    startP2pClientFsmTmr(pP2pClientProc, P2P_CLIENT_FSM_EVENT_LOGIN_WAIT_TMR_EXP);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   P2P client heartbeat wait timer expired
 * @param   pP2pClientProc
 */
static void procP2pClientHeartbeatWaitTmrExpEvt(P2P_CLIENT_PROCESS_t *pP2pClientProc)
{
    /* Login request not received */
    EPRINT(P2P_MODULE, "heartbeat wait timer expired: [client=%d], [model=%s]", pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->model);

    /* Stop P2P client process */
    stopP2pClientProcess(pP2pClientProc);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   P2P client heartbeat response retry timer expired
 * @param   pP2pClientProc
 * @param   pIsTimerExpired
 */
static void procP2pClientHeartbeatRespRetryTmrExpEvt(P2P_CLIENT_PROCESS_t *pP2pClientProc, BOOL *pIsTimerExpired)
{
    /* Send heartbeat response */
    if (SUCCESS == sendP2pClientSessionMsg(pP2pClientProc, P2P_SESSION_MSG_TYPE_HEARTBEAT_RESP))
    {
        /* Heartbeat response sent */
        startP2pClientFsmTmr(pP2pClientProc, P2P_CLIENT_FSM_EVENT_HEARTBEAT_WAIT_TMR_EXP);
        return;
    }

    /* Check timer expired or not */
    if (FALSE == *pIsTimerExpired)
    {
        /* Wait for timer expired */
        return;
    }

    /* Heartbeat response retry timer expired */
    EPRINT(P2P_MODULE, "heartbeat resp retry timer expired: [client=%d]", pP2pClientProc->pConnInfo->clientIdx);

    /* Fail to send heartbeat response. Now wait for new request */
    startP2pClientFsmTmr(pP2pClientProc, P2P_CLIENT_FSM_EVENT_HEARTBEAT_WAIT_TMR_EXP);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   P2P client relay refresh request send wait timer expired
 * @param   pP2pClientProc
 */
static void procP2pClientRelayRefreshReqWaitTmrExpEvt(P2P_CLIENT_PROCESS_t *pP2pClientProc)
{
    /* Send relay refresh request */
    pP2pClientProc->turnSessionInfo.lifetime = P2P_TURN_LIFETIME_MAX_IN_SEC;
    sendRelayControlMsg(pP2pClientProc->relayControlChannelFd, TURN_MSG_TYPE_REFRESH, pP2pClientProc);

    /* Start relay refresh response wait timer */
    startP2pClientUtilTmr(pP2pClientProc, P2P_CLIENT_FSM_EVENT_RELAY_REFRESH_RESP_WAIT_TMR_EXP);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Process relay server refresh response wait timer expire event
 * @param   pP2pClientProc
 * @param   pIsTimerExpired
 */
static void procP2pClientRelayRefreshRespWaitTmrExpEvt(P2P_CLIENT_PROCESS_t *pP2pClientProc, BOOL *pIsTimerExpired)
{
    do
    {
        /* Check timer expired or not */
        if (TRUE == *pIsTimerExpired)
        {
            /* Refresh failed with relay server */
            EPRINT(P2P_MODULE, "relay refresh resp wait timer expired: [client=%d]", pP2pClientProc->pConnInfo->clientIdx);
            break;
        }

        /* Receive the refresh message response */
        TURN_STATUS_e turnStatus = TURN_STATUS_FAIL;
        if (0 == recvRelayControlMsg(pP2pClientProc->relayControlChannelFd, TURN_MSG_TYPE_REFRESH, pP2pClientProc, &turnStatus))
        {
            /* Wait till time expired */
            return;
        }

        /* Is it auth challenge? */
        if (turnStatus == TURN_STATUS_CHALLENGE)
        {
            pP2pClientProc->challengeMsgRetryCnt++;
            if (pP2pClientProc->challengeMsgRetryCnt >= RELAY_CHALLENGE_RETRY_MAX)
            {
                /* We tried max to provide valid credential */
                EPRINT(P2P_MODULE, "max relay challenge req reached for refresh: [client=%d]", pP2pClientProc->pConnInfo->clientIdx);
                break;
            }

            /* Send relay refresh request with updated auth */
            sendRelayControlMsg(pP2pClientProc->relayControlChannelFd, TURN_MSG_TYPE_REFRESH, pP2pClientProc);

            /* Start relay refresh response wait timer */
            startP2pClientUtilTmr(pP2pClientProc, P2P_CLIENT_FSM_EVENT_RELAY_REFRESH_RESP_WAIT_TMR_EXP);
            return;
        }

        /* Reset the challenge message retry counter as other response received */
        pP2pClientProc->challengeMsgRetryCnt = 0;

        /* Is it success to proceed? */
        if (turnStatus == TURN_STATUS_OK)
        {
            /* Lifetime must not be 0 */
            if (pP2pClientProc->turnSessionInfo.lifetime == 0)
            {
                EPRINT(P2P_MODULE, "relay connection refresh expired: [client=%d], [lifetime=%d]",
                       pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->turnSessionInfo.lifetime);
                break;
            }

            /* Refresh success with relay server */
            DPRINT(P2P_MODULE, "relay connection refreshed: [client=%d], [lifetime=%d]",
                   pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->turnSessionInfo.lifetime);

            /* Restart relay connection refresh timer */
            startP2pClientUtilTmr(pP2pClientProc, P2P_CLIENT_FSM_EVENT_RELAY_REFRESH_REQ_WAIT_TMR_EXP);
            return;
        }

        /* Error received in refresh response */
        EPRINT(P2P_MODULE, "relay refresh failed by server: [client=%d], [errCode=%d], [errMsg=%s]",
               pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->turnSessionInfo.errCode, pP2pClientProc->turnSessionInfo.errMsg);

    }while(0);

    /* Relay refresh failed */
    EPRINT(P2P_MODULE, "relay refresh resp wait timer expired or failed by server: [client=%d], [model=%s]",
           pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->model);

    /* Stop P2P client process */
    stopP2pClientProcess(pP2pClientProc);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   P2P client heartbeat data receive
 * @param   pP2pClientProc
 * @param   pDataBuff
 * @param   dataLen
 */
static void procP2pClientHeartbeatData(P2P_CLIENT_PROCESS_t *pP2pClientProc, CHAR *pDataBuff, UINT32 dataLen)
{
    /* Validate message length */
    if (dataLen < P2P_CLIENT_MSG_HEADER_LEN)
    {
        /* Invalid message length received */
        EPRINT(P2P_MODULE, "invld msg length recv: [client=%d], [dataLen=%d]", pP2pClientProc->pConnInfo->clientIdx, dataLen);
        return;
    }

    /* Typecast with client message, as it is expected over here */
    P2P_CLIENT_MSG_t *pClientMsg = (P2P_CLIENT_MSG_t *)pDataBuff;

    /* Validate received header */
    if (FALSE == isValidMsgHeader(pP2pClientProc, &pClientMsg->header, dataLen))
    {
        /* Invalid message header received */
        EPRINT(P2P_MODULE, "invld msg header recv: [client=%d]", pP2pClientProc->pConnInfo->clientIdx);
        return;
    }

    if (pClientMsg->header.msgType == P2P_MSG_TYPE_SESSION)
    {
        /* Session message received */
        procP2pClientSessionMsg(pP2pClientProc, pDataBuff, dataLen);
    }
    else if (pClientMsg->header.msgType == P2P_MSG_TYPE_CONTROL)
    {
        /* Control message received */
        procP2pClientControlMsg(pP2pClientProc, pDataBuff, dataLen);
    }
    else
    {
        /* Data message received */
        procP2pClientStreamMsg(pP2pClientProc, pDataBuff, dataLen);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   P2P client hold ack timer expired
 * @param   pP2pClientProc
 */
static void procP2pClientHoldAckTmrExpEvt(P2P_CLIENT_PROCESS_t *pP2pClientProc, BOOL *pIsTimerExpired)
{
    /* Check timer expired or not */
    if (FALSE == *pIsTimerExpired)
    {
        /* Send hold ack to client to keep open hole */
        if (REFUSE != sendP2pClientSessionMsg(pP2pClientProc, P2P_SESSION_MSG_TYPE_HOLD_RESP))
        {
            /* Wait till time expired */
            return;
        }
    }

    /* Login request not received */
    EPRINT(P2P_MODULE, "hold ack timer expired or connection refused: [client=%d], [model=%s]",
           pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->model);

    /* Stop P2P client process */
    stopP2pClientProcess(pP2pClientProc);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Process received data; Only hold ack is expected
 * @param   pP2pClientProc
 * @param   pDataBuff
 * @param   dataLen
 */
static void procP2pClientHoldData(P2P_CLIENT_PROCESS_t *pP2pClientProc, CHAR *pDataBuff, UINT32 dataLen)
{
    /* Validate message length */
    if (dataLen < P2P_SESSION_MSG_MIN_LEN)
    {
        /* Invalid message length received */
        EPRINT(P2P_MODULE, "invld msg length recv: [client=%d], [dataLen=%d]", pP2pClientProc->pConnInfo->clientIdx, dataLen);
        return;
    }

    /* Typecast with client message, as it is expected over here */
    P2P_CLIENT_MSG_t *pClientMsg = (P2P_CLIENT_MSG_t *)pDataBuff;

    /* Validate received header */
    if (FALSE == isValidMsgHeader(pP2pClientProc, &pClientMsg->header, dataLen))
    {
        /* Invalid message header received */
        EPRINT(P2P_MODULE, "invld msg header recv: [client=%d]", pP2pClientProc->pConnInfo->clientIdx);
        return;
    }

    /* Received message must be session message */
    if (pClientMsg->header.msgType != P2P_MSG_TYPE_SESSION)
    {
        /* Invalid message type received */
        EPRINT(P2P_MODULE, "invld msg type recv: [client=%d], [msgType=%d]", pP2pClientProc->pConnInfo->clientIdx, pClientMsg->header.msgType);
        return;
    }

    /* Get session message header */
    P2P_CLIENT_SESSION_MSG_t *pSessionPayload = &pClientMsg->payload.session;

    /* Validate message type. It must be resume request message */
    if ((pSessionPayload->msgType != P2P_SESSION_MSG_TYPE_RESUME_REQ) && (pSessionPayload->msgType != P2P_SESSION_MSG_TYPE_CLOSE))
    {
        /* Invalid message type received */
        EPRINT(P2P_MODULE, "invld session msg type recv: [client=%d], [msgType=%d]", pP2pClientProc->pConnInfo->clientIdx, pSessionPayload->msgType);
        return;
    }

    /* Session message received */
    procP2pClientSessionMsg(pP2pClientProc, pDataBuff, dataLen);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Create the TCP connection to start communication with relay server.
 * @param   pP2pClientProc
 * @return  Returns FAIL if communication is not possible else returns SUCCESS
 */
static BOOL startRelayControlChannel(P2P_CLIENT_PROCESS_t *pP2pClientProc)
{
    DOMAIN_ADDR_INFO_t relayDomainAddr;

    /* start p2p client fsm timer for relay */
    startP2pClientFsmTmr(pP2pClientProc, P2P_CLIENT_FSM_EVENT_RELAY_CONTROL_CONNECT_WAIT_TMR_EXP);

    /* Check server details to start with it */
    while(pP2pClientProc->relayServerIdx < RELAY_SERVER_SUPPORT_MAX)
    {
        if ((pP2pClientProc->pConnInfo->pRelayServerAddr[pP2pClientProc->relayServerIdx].domain[0] != '\0')
                && (pP2pClientProc->pConnInfo->pRelayServerAddr[pP2pClientProc->relayServerIdx].port != 0))
        {
            /* Relay server IP address and port are valid */
            break;
        }

        /* Check for next relay server */
        pP2pClientProc->relayServerIdx++;
    }

    /* No next valid IP server and port found */
    if (pP2pClientProc->relayServerIdx >= RELAY_SERVER_SUPPORT_MAX)
    {
        EPRINT(P2P_MODULE, "next relay server not available: [client=%d]", pP2pClientProc->pConnInfo->clientIdx);
        pP2pClientProc->relayServerIdx = 0;
        return FAIL;
    }

    /* Close the socket with old relay server */
    CloseSocket(&pP2pClientProc->relayControlChannelFd);

    /* Get relay server address local copy for reference */
    memcpy(&relayDomainAddr, &pP2pClientProc->pConnInfo->pRelayServerAddr[pP2pClientProc->relayServerIdx], sizeof(DOMAIN_ADDR_INFO_t));

    /* Use next relay server on failure */
    pP2pClientProc->relayServerIdx++;

    /* Resolve relay server domain address */
    if (FAIL == GetIpAddrFromDomainName(relayDomainAddr.domain, IP_ADDR_TYPE_IPV4, pP2pClientProc->relayServerAddr.ip))
    {
        /* We will check next relay server after sometime (connect wait timer) */
        EPRINT(P2P_MODULE, "fail to resolve relay server domain: [client=%d], [domain=%s]", pP2pClientProc->pConnInfo->clientIdx, relayDomainAddr.domain);
        return SUCCESS;
    }

    /* Store relay server port */
    pP2pClientProc->relayServerAddr.port = relayDomainAddr.port;

    /* Open client socket to communicate with relay new server */
    pP2pClientProc->pConnInfo->localAddr.port = 0;
    if (FALSE == OpenP2pSocket(FALSE, &pP2pClientProc->pConnInfo->localAddr, &pP2pClientProc->relayControlChannelFd))
    {
        EPRINT(P2P_MODULE, "fail to open relay control channel socket: [client=%d], [server=%d], [relayServerAddr=%s:%d], [err=%s]",
               pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->relayServerIdx-1, pP2pClientProc->relayServerAddr.ip, pP2pClientProc->relayServerAddr.port, STR_ERR);
        return FAIL;
    }

    /* Start creating connection with relay server for control channel */
    if (FALSE == ConnectP2pSocket(pP2pClientProc->relayControlChannelFd, &pP2pClientProc->relayServerAddr))
    {
        /* We Will handle this in relay control connect wait timer */
        CloseSocket(&pP2pClientProc->relayControlChannelFd);
        EPRINT(P2P_MODULE, "fail to connect relay control channel: [client=%d], [server=%d], [relayServerAddr=%s:%d]",
               pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->relayServerIdx-1, pP2pClientProc->relayServerAddr.ip, pP2pClientProc->relayServerAddr.port);
    }
    else
    {
        /* Connection in progress, wait for sometime */
        DPRINT(P2P_MODULE, "connect in progress for relay control channel: [client=%d], [server=%d], [nvrAddr=%s:%d], [relayServerAddr=%s:%d]",
               pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->relayServerIdx-1,
               pP2pClientProc->pConnInfo->localAddr.ip, pP2pClientProc->pConnInfo->localAddr.port,
               pP2pClientProc->relayServerAddr.ip, pP2pClientProc->relayServerAddr.port);
    }

    /* Relay communication started */
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send handshake request message on all local and public end points of client
 * @param   pP2pClientProc
 */
static void sendHandshakeReqMsg(P2P_CLIENT_PROCESS_t *pP2pClientProc)
{
    /* The handshake request will be sent by mobile client. We will not initiate it. We will wait for handshake message.
     * We will continue with socket from where handshake message data available and remaining unnecessary connections will get closed */
    /* Do not proceed further if connection already established */
    if (pP2pClientProc->pConnInfo->sockFd != INVALID_CONNECTION)
    {
        /* Connection establised */
        return;
    }

    /* Local connection is possible with client */
    if ((pP2pClientProc->clientLocalSockFd != INVALID_CONNECTION) && (TRUE == isClientLocalCommPossible(pP2pClientProc)))
    {
        if (IN_PROGRESS == pP2pClientProc->localClientConnStatus)
        {
            pP2pClientProc->localClientConnStatus = IsP2pConnectionEstablished(pP2pClientProc->clientLocalSockFd);
        }

        if (TRUE == pP2pClientProc->localClientConnStatus)
        {
            if (GetAvailableBytesOnSocket(pP2pClientProc->clientLocalSockFd) == P2P_SESSION_MSG_MIN_LEN)
            {
                pP2pClientProc->pConnInfo->sockFd = pP2pClientProc->clientLocalSockFd;
                pP2pClientProc->clientLocalSockFd = INVALID_CONNECTION;
                CloseSocket(&pP2pClientProc->clientPublicSockFd);
                CloseSocket(&pP2pClientProc->acceptedClientFd);
                CloseSocket(&pP2pClientProc->serverSockFd);
                pP2pClientProc->recvAddr = pP2pClientProc->pConnInfo->clientLocalAddr;
                DPRINT(P2P_MODULE, "tcp connection go-ahead with local endpoint: [client=%d], [remoteAddr=%s:%d]",
                       pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->recvAddr.ip, pP2pClientProc->recvAddr.port);
                return;
            }
            else
            {
                WPRINT(P2P_MODULE, "tcp connection done with local but data not available: [client=%d], [clientLocalAddr=%s:%d]",
                       pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->clientLocalAddr.ip,
                       pP2pClientProc->pConnInfo->clientLocalAddr.port);
            }
        }
        else if (IN_PROGRESS != pP2pClientProc->localClientConnStatus)
        {
            if (REFUSE == pP2pClientProc->localClientConnStatus)
            {
                /* Open client socket for handshake on local end-point */
                CloseSocket(&pP2pClientProc->clientLocalSockFd);
                if (FALSE == OpenP2pSocket(FALSE, &pP2pClientProc->pConnInfo->localAddr, &pP2pClientProc->clientLocalSockFd))
                {
                    EPRINT(P2P_MODULE, "fail to open handshake client local socket: [client=%d], [err=%s]", pP2pClientProc->pConnInfo->clientIdx, STR_ERR);
                }
            }

            if (pP2pClientProc->clientLocalSockFd != INVALID_CONNECTION)
            {
                /* Start creating connection with client local end-point */
                if (FALSE == ConnectP2pSocket(pP2pClientProc->clientLocalSockFd, &pP2pClientProc->pConnInfo->clientLocalAddr))
                {
                    pP2pClientProc->localClientConnStatus = FALSE;
                    EPRINT(P2P_MODULE, "fail to connect handshake client local endpoint: [client=%d], [clientLocalAddr=%s:%d]",
                           pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->clientLocalAddr.ip, pP2pClientProc->pConnInfo->clientLocalAddr.port);
                }
                else
                {
                    pP2pClientProc->localClientConnStatus = IN_PROGRESS;
                    DPRINT(P2P_MODULE, "connect in progress for local endpoint: [client=%d], [clientLocalAddr=%s:%d]",
                           pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->clientLocalAddr.ip, pP2pClientProc->pConnInfo->clientLocalAddr.port);
                }
            }
        }
    }

    /* Public connection is possible with client */
    if ((pP2pClientProc->clientPublicSockFd != INVALID_CONNECTION) && (TRUE == isClientPublicCommPossible(pP2pClientProc)))
    {
        if (IN_PROGRESS == pP2pClientProc->publicClientConnStatus)
        {
            pP2pClientProc->publicClientConnStatus = IsP2pConnectionEstablished(pP2pClientProc->clientPublicSockFd);
        }

        if (TRUE == pP2pClientProc->publicClientConnStatus)
        {
            if (GetAvailableBytesOnSocket(pP2pClientProc->clientPublicSockFd) == P2P_SESSION_MSG_MIN_LEN)
            {
                pP2pClientProc->pConnInfo->sockFd = pP2pClientProc->clientPublicSockFd;
                pP2pClientProc->clientPublicSockFd = INVALID_CONNECTION;
                CloseSocket(&pP2pClientProc->clientLocalSockFd);
                CloseSocket(&pP2pClientProc->acceptedClientFd);
                CloseSocket(&pP2pClientProc->serverSockFd);
                pP2pClientProc->recvAddr = pP2pClientProc->pConnInfo->clientPublicAddr;
                DPRINT(P2P_MODULE, "tcp connection go ahead with public endpoint: [client=%d], [remoteAddr=%s:%d]",
                       pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->recvAddr.ip, pP2pClientProc->recvAddr.port);
                return;
            }
            else
            {
                WPRINT(P2P_MODULE, "tcp connection done with public but data not available: [client=%d], [clientPublicAddr=%s:%d]",
                       pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->clientPublicAddr.ip,
                       pP2pClientProc->pConnInfo->clientPublicAddr.port);
            }
        }
        else if (IN_PROGRESS != pP2pClientProc->publicClientConnStatus)
        {
            if (REFUSE == pP2pClientProc->publicClientConnStatus)
            {
                /* Open client socket for handshake on public end-point */
                CloseSocket(&pP2pClientProc->clientPublicSockFd);
                if (FALSE == OpenP2pSocket(FALSE, &pP2pClientProc->pConnInfo->localAddr, &pP2pClientProc->clientPublicSockFd))
                {
                    EPRINT(P2P_MODULE, "fail to open handshake client public socket: [client=%d], [err=%s]", pP2pClientProc->pConnInfo->clientIdx, STR_ERR);
                }
            }

            if (pP2pClientProc->clientPublicSockFd != INVALID_CONNECTION)
            {
                /* Start creating connection with client public end-point */
                if (FALSE == ConnectP2pSocket(pP2pClientProc->clientPublicSockFd, &pP2pClientProc->pConnInfo->clientPublicAddr))
                {
                    pP2pClientProc->publicClientConnStatus = FALSE;
                    EPRINT(P2P_MODULE, "fail to connect handshake client public endpoint: [client=%d], [clientPublicAddr=%s:%d]",
                           pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->clientPublicAddr.ip, pP2pClientProc->pConnInfo->clientPublicAddr.port);
                }
                else
                {
                    pP2pClientProc->publicClientConnStatus = IN_PROGRESS;
                    DPRINT(P2P_MODULE, "connect in progress for public endpoint: [client=%d], [clientPublicAddr=%s:%d]",
                           pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->clientPublicAddr.ip, pP2pClientProc->pConnInfo->clientPublicAddr.port);
                }
            }
        }
    }

    /* Ideally server socket should not be invalid */
    if (pP2pClientProc->serverSockFd != INVALID_CONNECTION)
    {
        /* Check new connection on server socket and if connection is already accepted then check data on socket */
        if (pP2pClientProc->acceptedClientFd == INVALID_CONNECTION)
        {
            struct sockaddr_in  clientAddr;
            socklen_t           sockLen = sizeof(clientAddr);

            /* Check for new connection */
            pP2pClientProc->acceptedClientFd = accept(pP2pClientProc->serverSockFd, (struct sockaddr*)&clientAddr, &sockLen);
            if (pP2pClientProc->acceptedClientFd == INVALID_CONNECTION)
            {
                /* Error in connection accept */
                if ((errno != EAGAIN) && (errno != EWOULDBLOCK))
                {
                    EPRINT(P2P_MODULE, "fail to accept connection: [client=%d], [err=%s]", pP2pClientProc->pConnInfo->clientIdx, STR_ERR);
                }
            }
            else
            {
                /* Convert IP from network to string format */
                if (NULL == inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr.s_addr, pP2pClientProc->recvAddr.ip, sizeof(pP2pClientProc->recvAddr.ip)))
                {
                    EPRINT(P2P_MODULE, "fail to convert remote ip in str: [client=%d], [ipNw=%ul]",
                           pP2pClientProc->pConnInfo->clientIdx, clientAddr.sin_addr.s_addr);
                }

                /* Convert port from network to host format */
                pP2pClientProc->recvAddr.port = ntohs(clientAddr.sin_port);

                /* Make this connection non-blocking */
                if (FALSE == SetSockFdOption(pP2pClientProc->acceptedClientFd))
                {
                    EPRINT(P2P_MODULE, "fail to set socket option: [client=%d], [recvAddr=%s:%d]",
                           pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->recvAddr.ip, pP2pClientProc->recvAddr.port);
                }
            }
        }

        if (pP2pClientProc->acceptedClientFd != INVALID_CONNECTION)
        {
            if (GetAvailableBytesOnSocket(pP2pClientProc->acceptedClientFd) == P2P_SESSION_MSG_MIN_LEN)
            {
                pP2pClientProc->pConnInfo->sockFd = pP2pClientProc->acceptedClientFd;
                pP2pClientProc->acceptedClientFd = INVALID_CONNECTION;
                CloseSocket(&pP2pClientProc->clientLocalSockFd);
                CloseSocket(&pP2pClientProc->clientPublicSockFd);
                DPRINT(P2P_MODULE, "tcp connection go ahead with server socket connection: [client=%d], [remoteAddr=%s:%d]",
                       pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->recvAddr.ip, pP2pClientProc->recvAddr.port);
                return;
            }
            else
            {
                WPRINT(P2P_MODULE, "connecton done on server socket but data not available: [client=%d], [remoteAddr=%s:%d]",
                       pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->recvAddr.ip, pP2pClientProc->recvAddr.port);
            }
        }
    }

    /* Hole punching may not happen in this case. Enable hairpin feature in router to support this scenario */
    if (strcmp(pP2pClientProc->pConnInfo->publicAddr.ip, pP2pClientProc->pConnInfo->clientPublicAddr.ip) == 0)
    {
        WPRINT(P2P_MODULE, "device and client public ip are same: [client=%d], [nvrPublicAddr=%s]",
               pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->publicAddr.ip);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send relay control request message to relay server
 * @param   pP2pClientProc
 * @param   turnMsgType
 * @param   pP2pClientProc
 */
static void sendRelayControlMsg(INT32 sockFd, TURN_MSG_TYPE_e turnMsgType, P2P_CLIENT_PROCESS_t *pP2pClientProc)
{
    TURN_STATUS_e turnStatus;

    /* Reset message length before preparing new message */
    pP2pClientProc->turnSessionInfo.msgLen = 0;

    switch(turnMsgType)
    {
        case TURN_MSG_TYPE_ALLOCATE:
        {
            /* Prepare turn allocation request message */
            turnStatus = Turn_PrepareAllocateReqMsg(&pP2pClientProc->turnSessionInfo);
        }
        break;

        case TURN_MSG_TYPE_CREATE_PERMISSION:
        {
            /* Prepare turn create permission request message */
            turnStatus = Turn_PrepareCreatePermissionReqMsg(&pP2pClientProc->turnSessionInfo);
        }
        break;

        case TURN_MSG_TYPE_CONNECTION_BIND:
        {
            /* Prepare turn connection bind request message */
            turnStatus = Turn_PrepareConnectionBindReqMsg(&pP2pClientProc->turnSessionInfo);
        }
        break;

        case TURN_MSG_TYPE_REFRESH:
        {
            /* Prepare turn refresh request message */
            turnStatus = Turn_PrepareRefreshReqMsg(&pP2pClientProc->turnSessionInfo);
        }
        break;

        default:
        {
            EPRINT(P2P_MODULE, "invld msg type: [client=%d], [turnMsgType=%d]", pP2pClientProc->pConnInfo->clientIdx, turnMsgType);
        }
        return;
    }

    /* Check message prepare status */
    if (TURN_STATUS_FAIL == turnStatus)
    {
        pP2pClientProc->turnSessionInfo.msgLen = 0;
        EPRINT(P2P_MODULE, "fail to prepare turn req: [client=%d], [msg=%s]", pP2pClientProc->pConnInfo->clientIdx, pTurnMsgTypeStr[turnMsgType]);
        return;
    }

    /* Send relay control message to relay server */
    if (SUCCESS != SendP2pMsg(sockFd, (CHAR*)pP2pClientProc->turnSessionInfo.msgBuf, pP2pClientProc->turnSessionInfo.msgLen, 0))
    {
        pP2pClientProc->turnSessionInfo.msgLen = 0;
        EPRINT(P2P_MODULE, "fail to send turn req: [client=%d], [msg=%s]", pP2pClientProc->pConnInfo->clientIdx, pTurnMsgTypeStr[turnMsgType]);
        return;
    }

    /* Reset message length here because same is used in receive for buffer offset */
    pP2pClientProc->turnSessionInfo.msgLen = 0;
    DPRINT(P2P_MODULE, "turn req msg sent: [client=%d], [msg=%s]", pP2pClientProc->pConnInfo->clientIdx, pTurnMsgTypeStr[turnMsgType]);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Receive relay server control message
 * @param   sockFd
 * @param   turnMsgType
 * @param   pP2pClientProc
 * @param   pTurnStatus
 * @return  Returns 0 if whole message not received else received bytes
 */
static UINT32 recvRelayControlMsg(INT32 sockFd, TURN_MSG_TYPE_e turnMsgType, P2P_CLIENT_PROCESS_t *pP2pClientProc, TURN_STATUS_e *pTurnStatus)
{
    UINT32 msgLen = 0;

    /* Is any data available on socket? */
    if (GetAvailableBytesOnSocket(sockFd) == 0)
    {
        /* No data available to read */
        return 0;
    }

    /* Receive control message response */
    if (SUCCESS != RecvP2pMsg(sockFd, (CHAR*)&pP2pClientProc->turnSessionInfo.msgBuf[pP2pClientProc->turnSessionInfo.msgLen],
                              sizeof(pP2pClientProc->turnSessionInfo.msgBuf) - pP2pClientProc->turnSessionInfo.msgLen, &msgLen))
    {
        /* Ideally, it should not happen */
        EPRINT(P2P_MODULE, "fail to recv turn control resp: [client=%d]", pP2pClientProc->pConnInfo->clientIdx);
        return 0;
    }

    /* Update message length */
    pP2pClientProc->turnSessionInfo.msgLen += msgLen;
    if (-1 == Turn_GetMsgLen(&pP2pClientProc->turnSessionInfo))
    {
        /* Complete message not received yet */
        return 0;
    }

    /* Store message length for future use */
    msgLen = pP2pClientProc->turnSessionInfo.msgLen;
    switch(turnMsgType)
    {
        case TURN_MSG_TYPE_ALLOCATE:
        {
            /* Parse turn allocation response message */
            *pTurnStatus = Turn_ParseAllocateRespMsg(&pP2pClientProc->turnSessionInfo);
        }
        break;

        case TURN_MSG_TYPE_CREATE_PERMISSION:
        {
            /* Parse turn create permission response message */
            *pTurnStatus = Turn_ParseCreatePermissionRespMsg(&pP2pClientProc->turnSessionInfo);
        }
        break;

        case TURN_MSG_TYPE_CONNECTION_BIND:
        {
            /* Parse turn connection bind response message */
            *pTurnStatus = Turn_ParseConnectionBindRespMsg(&pP2pClientProc->turnSessionInfo);
        }
        break;

        case TURN_MSG_TYPE_REFRESH:
        {
            /* Parse turn refresh response message */
            *pTurnStatus = Turn_ParseConnectionRefreshRespMsg(&pP2pClientProc->turnSessionInfo);
        }
        break;

        case TURN_MSG_TYPE_CONNECTION_ATTEMPT:
        {
            /* Parse turn connection attempt indication message */
            *pTurnStatus = Turn_ParseConnectionAttemptIndMsg(&pP2pClientProc->turnSessionInfo);
        }
        break;

        default:
        {
            pP2pClientProc->turnSessionInfo.msgLen = 0;
            EPRINT(P2P_MODULE, "invld msg type: [client=%d], [turnMsgType=%d]", pP2pClientProc->pConnInfo->clientIdx, turnMsgType);
        }
        return 0;
    }

    /* Reset message length here because same is used in receive message for buffer offset */
    pP2pClientProc->turnSessionInfo.msgLen = 0;
    DPRINT(P2P_MODULE, "turn resp msg rcvd: [client=%d], [msg=%s], [status=%s]",
           pP2pClientProc->pConnInfo->clientIdx, pTurnMsgTypeStr[turnMsgType], pTurnMsgStatusStr[*pTurnStatus]);

    /* Complete message received and parsed successfully */
    return msgLen;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send client session message
 * @param   pP2pClientProc
 * @param   sessionMsgType
 * @return  Data sent status
 */
static BOOL sendP2pClientSessionMsg(P2P_CLIENT_PROCESS_t *pP2pClientProc, P2P_SESSION_MSG_TYPE_e sessionMsgType)
{
    BOOL                retSts;
    P2P_CLIENT_MSG_t    clientMsg;
    UINT32              msgLen = 0;

    /* Reset information */
    memset(&clientMsg, 0, sizeof(clientMsg));

    /* Generate session message payload for session message type */
    if (FALSE == prepareSessionMsgPaylaod(sessionMsgType, pP2pClientProc, &clientMsg.payload.session, &msgLen))
    {
        EPRINT(P2P_MODULE, "fail to generate session message resp payload: [client=%d]", pP2pClientProc->pConnInfo->clientIdx);
        return FAIL;
    }

    /* Prepare header */
    prepareMsgHeader(&clientMsg.header, P2P_MSG_TYPE_SESSION, pP2pClientProc->sessionMsgRespUid, msgLen);
    msgLen += P2P_CLIENT_MSG_HEADER_LEN;

    /* Send session message response */
    MUTEX_LOCK(pP2pClientProc->socketLock);
    retSts = SendP2pMsg(pP2pClientProc->pConnInfo->sockFd, (CHAR *)&clientMsg, msgLen, 0);
    MUTEX_UNLOCK(pP2pClientProc->socketLock);

    if (SUCCESS != retSts)
    {
        EPRINT(P2P_MODULE, "fail to send session msg resp: [client=%d], [model=%s], [msgType=%s]",
               pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->model, getSessionMsgTypeStr(sessionMsgType));
    }

    return retSts;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Prepare client message header
 * @param   pHeader
 * @param   msgType
 * @param   payloadLen
 */
static void prepareMsgHeader(P2P_CLIENT_MSG_HEADER_t *pHeader, P2P_MSG_TYPE_e msgType, UINT32 msgUid, UINT32 payloadLen)
{
    pHeader->magicCode = P2P_MSG_MAGIC_CODE;
    SET_CLIENT_PROTOCOL_VERSION(pHeader->version);
    pHeader->msgType = msgType;
    pHeader->msgUid = msgUid;
    pHeader->payloadLen = payloadLen;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Validate header of message
 * @param   pP2pClientProc
 * @param   pHeader
 * @param   msgLen
 * @return
 */
static BOOL isValidMsgHeader(P2P_CLIENT_PROCESS_t *pP2pClientProc, P2P_CLIENT_MSG_HEADER_t *pHeader, UINT32 msgLen)
{
    /* Validate magic code */
    if (pHeader->magicCode != P2P_MSG_MAGIC_CODE)
    {
        /* Invalid magic code received */
        EPRINT(P2P_MODULE, "invld magic code: [client=%d], [magicCode=0x%x]",
               pP2pClientProc->pConnInfo->clientIdx, pHeader->magicCode);
        return FALSE;
    }

    /* Validate magic code */
    if (pHeader->version != GET_CLIENT_PROTOCOL_VERSION)
    {
        /* Unsupported message length received */
        EPRINT(P2P_MODULE, "unsupported protocol version: [client=%d], [version=%d]",
               pP2pClientProc->pConnInfo->clientIdx, pHeader->version);
        return FALSE;
    }

    /* Validate message type */
    if (pHeader->msgType >= P2P_MSG_TYPE_MAX)
    {
        EPRINT(P2P_MODULE, "invld message type: [client=%d], [msgType=%d]", pP2pClientProc->pConnInfo->clientIdx, pHeader->msgType);
        return FALSE;
    }

    /* Validate unique msg id */
    if ((pHeader->msgUid == 0) || ((INT32)pHeader->msgUid == INVALID_CONNECTION))
    {
        /* Invalid payload length */
        EPRINT(P2P_MODULE, "invld msgUid recv: [client=%d], [msgUid=0x%x]", pP2pClientProc->pConnInfo->clientIdx, pHeader->msgUid);
        return FALSE;
    }

    /* Validate payload length */
    if ((pHeader->payloadLen == 0) || (msgLen != (pHeader->payloadLen + P2P_CLIENT_MSG_HEADER_LEN)))
    {
        /* Invalid payload length */
        EPRINT(P2P_MODULE, "invld payload length recv: [client=%d], [payloadLen=%d], [msgLen=%d]",
               pP2pClientProc->pConnInfo->clientIdx, pHeader->payloadLen, msgLen);
        return FALSE;
    }

    /* Valid header found */
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Validate session message header
 * @param   pP2pClientProc
 * @param   pSessionPayload
 * @return  TRUE if valid header; FALSE otherwise
 */
static BOOL isValidSessionMsgHeader(P2P_CLIENT_PROCESS_t *pP2pClientProc, P2P_CLIENT_SESSION_MSG_t *pSessionPayload)
{
    /* Validate session message type */
    if (pSessionPayload->msgType >= P2P_SESSION_MSG_TYPE_MAX)
    {
        EPRINT(P2P_MODULE, "invld message type: [client=%d], [msgType=%d]", pP2pClientProc->pConnInfo->clientIdx, pSessionPayload->msgType);
        return FALSE;
    }

    /* Validate device ID */
    if (strcmp(pSessionPayload->deviceId, GetP2pDeviceId()))
    {
        EPRINT(P2P_MODULE, "invld device id: [client=%d], [deviceId=%s]", pP2pClientProc->pConnInfo->clientIdx, pSessionPayload->deviceId);
        return FALSE;
    }

    /* Get session id for validation */
    CHAR clientSessionId[P2P_RAW_SESSION_ID_LEN_MAX];
    UINT16 publicPort1, publicPort2;
    if (TRUE == pP2pClientProc->pConnInfo->fallbackToRelayServer)
    {
        /* Use peer relay public port as device and client public port for session id */
        publicPort1 = publicPort2 = pP2pClientProc->pConnInfo->peerRelayAddr.port;
    }
    else
    {
        publicPort1 = pP2pClientProc->pConnInfo->clientPublicAddr.port;
        publicPort2 = pP2pClientProc->pConnInfo->publicAddr.port;
    }

    if (FALSE == DecodeP2pClientSessionId(pSessionPayload->sessionId, publicPort1, publicPort2,
                                          pSessionPayload->timestamp, clientSessionId, sizeof(clientSessionId)))
    {
        EPRINT(P2P_MODULE, "fail to decode client session id: [client=%d], [deviceId=%s]", pP2pClientProc->pConnInfo->clientIdx, pSessionPayload->sessionId);
        return FALSE;
    }

    /* Validate session id */
    if (strcmp(clientSessionId, pP2pClientProc->pConnInfo->clientSessionId))
    {
        EPRINT(P2P_MODULE, "invld session id: [client=%d], [deviceId=%s]", pP2pClientProc->pConnInfo->clientIdx, pSessionPayload->sessionId);
        return FALSE;
    }

    /* Valid Session Message Header */
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Prepare session message payload
 * @param   sessionMsgType
 * @param   pP2pClientProc
 * @param   pSessionPayload
 * @return
 */
static BOOL prepareSessionMsgPaylaod(P2P_SESSION_MSG_TYPE_e sessionMsgType, P2P_CLIENT_PROCESS_t *pP2pClientProc,
                                     P2P_CLIENT_SESSION_MSG_t *pSessionPayload, UINT32 *pPayloadLen)
{
    UINT16 publicPort1, publicPort2;

    /* Store session message payload information */
    pSessionPayload->msgType = sessionMsgType;
    snprintf(pSessionPayload->deviceId, sizeof(pSessionPayload->deviceId), "%s", GetP2pDeviceId());
    GetP2pTimeStamp(pSessionPayload->timestamp, sizeof(pSessionPayload->timestamp));
    if (TRUE == pP2pClientProc->pConnInfo->fallbackToRelayServer)
    {
        /* Use peer relay public port as device and client public port for session id */
        publicPort1 = publicPort2 = pP2pClientProc->pConnInfo->peerRelayAddr.port;
    }
    else
    {
        publicPort1 = pP2pClientProc->pConnInfo->clientPublicAddr.port;
        publicPort2 = pP2pClientProc->pConnInfo->publicAddr.port;
    }

    if (FALSE == EncodeP2pClientSessionId(pP2pClientProc->pConnInfo->deviceSessionId, publicPort1, publicPort2, pSessionPayload->timestamp,
                                          pSessionPayload->sessionId, sizeof(pSessionPayload->sessionId)))
    {
        return FALSE;
    }

    *pPayloadLen = P2P_SESSION_PAYLOAD_HEADER_LEN;
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set relay server fail status code
 * @param   pP2pClientProc
 */
static void setRelayFailStatusCode(P2P_CLIENT_PROCESS_t *pP2pClientProc)
{
    switch(pP2pClientProc->pConnInfo->errorCode)
    {
        case CONN_FAIL_ALLOC_QUOTA_REACHED:
        {
            /* Nothing to do as high priority status code is already set */
        }
        break;

        case CONN_FAIL_INSUFFICIENT_CAPACITY:
        {
            /* Set high prioriry status code only */
            if (pP2pClientProc->turnSessionInfo.errCode == TURN_EC_ALLOC_QUOTA_REACHED)
            {
                pP2pClientProc->pConnInfo->errorCode = CONN_FAIL_ALLOC_QUOTA_REACHED;
            }
        }
        break;

        default:
        {
            /* Set appropriate status code */
            switch(pP2pClientProc->turnSessionInfo.errCode)
            {
                case TURN_EC_ALLOC_QUOTA_REACHED:
                {
                    pP2pClientProc->pConnInfo->errorCode = CONN_FAIL_ALLOC_QUOTA_REACHED;
                }
                break;

                case TURN_EC_INSUFFICIENT_CAPACITY:
                {
                    pP2pClientProc->pConnInfo->errorCode = CONN_FAIL_INSUFFICIENT_CAPACITY;
                }
                break;

                default:
                {
                    pP2pClientProc->pConnInfo->errorCode = CONN_FAIL_RELAY_FALLBACK_FAIL;
                }
                break;
            }
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Change P2P client FSM state
 * @param   fsmState
 */
static void changeP2pClientFsmState(P2P_CLIENT_PROCESS_t *pP2pClientProc, P2P_CLIENT_FSM_STATE_e clientFsmState)
{
    /* Check FSM state changed or not */
    if (pP2pClientProc->clientFsmState == clientFsmState)
    {
        /* No state changed */
        return;
    }

    DPRINT(P2P_MODULE, "client fsm state changed: [fallbackToRelay=%s], [%s --> %s], [model=%s]",
           pP2pClientProc->pConnInfo->fallbackToRelayServer ? "YES" : "NO",
           pClientFsmStateStr[pP2pClientProc->clientFsmState], pClientFsmStateStr[clientFsmState],
           pP2pClientProc->pConnInfo->model);
    pP2pClientProc->clientFsmState = clientFsmState;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Handle client FSM timer hanlder expiry event
 * @param   appData1
 * @param   appData2
 * @param   isTimerExpired
 */
static void p2pClientFsmTmrHandler(UINT32 appData1, UINT32 appData2, BOOL isTimerExpired)
{
    /* Validate client index */
    if (appData2 >= P2P_CLIENT_SUPPORT_MAX)
    {
        /* Invalid client index */
        EPRINT(P2P_MODULE, "invld client found: [client=%d]", appData2);
        return;
    }

    /* Process event in FSM */
    procP2pClientFsm(appData1, &p2pClientProc[appData2], &isTimerExpired, 0);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Start P2P client FSM timer
 * @param   pP2pClientProc
 * @param   fsmEvent
 */
static void startP2pClientFsmTmr(P2P_CLIENT_PROCESS_t *pP2pClientProc, P2P_CLIENT_FSM_EVENT_e fsmEvent)
{
    /* Start P2P client fsm timer */
    if (FALSE == SysTimerStart(&pP2pClientProc->pSysTimerListHead, &pP2pClientProc->fsmTimer, fsmEvent, pP2pClientProc->pConnInfo->clientIdx,
                               p2pClientFsmTimer[fsmEvent].timeInMs, p2pClientFsmTimer[fsmEvent].loopCnt))
    {
        EPRINT(P2P_MODULE, "fail to start client fsm timer: [client=%d], [fsmEvent=%d]", pP2pClientProc->pConnInfo->clientIdx, fsmEvent);
        return;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Start P2P client utility timer
 * @param   pP2pClientProc
 * @param   fsmEvent
 */
static void startP2pClientUtilTmr(P2P_CLIENT_PROCESS_t *pP2pClientProc, P2P_CLIENT_FSM_EVENT_e fsmEvent)
{
    /* Start P2P client utility timer */
    if (FALSE == SysTimerStart(&pP2pClientProc->pSysTimerListHead, &pP2pClientProc->utilTimer, fsmEvent, pP2pClientProc->pConnInfo->clientIdx,
                               p2pClientFsmTimer[fsmEvent].timeInMs, p2pClientFsmTimer[fsmEvent].loopCnt))
    {
        EPRINT(P2P_MODULE, "fail to start client utility timer: [client=%d], [fsmEvent=%d]", pP2pClientProc->pConnInfo->clientIdx, fsmEvent);
        return;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Process received client session message
 * @param   pP2pClientProc
 * @param   pDataBuff
 * @param   dataLen
 */
static void procP2pClientSessionMsg(P2P_CLIENT_PROCESS_t *pP2pClientProc, CHAR *pDataBuff, UINT32 dataLen)
{
    P2P_CLIENT_MSG_HEADER_t     *pMsgHeader = &((P2P_CLIENT_MSG_t *)pDataBuff)->header;
    P2P_CLIENT_SESSION_MSG_t    *pSessionPayload = &((P2P_CLIENT_MSG_t *)pDataBuff)->payload.session;

    /* Validate message length */
    if (dataLen < P2P_SESSION_MSG_MIN_LEN)
    {
        EPRINT(P2P_MODULE, "invld session msg length: [client=%d], [dataLen=%d]", pP2pClientProc->pConnInfo->clientIdx, dataLen);
        return;
    }

    /* Validate session message header */
    if (FALSE == isValidSessionMsgHeader(pP2pClientProc, pSessionPayload))
    {
        /* Invalid session message header received */
        EPRINT(P2P_MODULE, "invld session msg header recv: [client=%d], [msgType=%d]", pP2pClientProc->pConnInfo->clientIdx, pSessionPayload->msgType);
        return;
    }

    /* Store message unique ID for response */
    pP2pClientProc->sessionMsgRespUid = pMsgHeader->msgUid;

    switch(pSessionPayload->msgType)
    {
        case P2P_SESSION_MSG_TYPE_HEARTBEAT_REQ:
        {
            /* Reload heartbeat timer */
            DPRINT(P2P_MODULE, "p2p heartbeat recv: [client=%d], [model=%s], [clientUid=%s]",
                   pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->model, pP2pClientProc->pConnInfo->clientUid);


            /* Send heartbeat response */
            if (SUCCESS == sendP2pClientSessionMsg(pP2pClientProc, P2P_SESSION_MSG_TYPE_HEARTBEAT_RESP))
            {
                /* Heartbeat response sent successfully */
                startP2pClientFsmTmr(pP2pClientProc, P2P_CLIENT_FSM_EVENT_HEARTBEAT_WAIT_TMR_EXP);
            }
            else
            {
                /* Failed to send heartbeat response */
                startP2pClientFsmTmr(pP2pClientProc, P2P_CLIENT_FSM_EVENT_HEARTBEAT_RESP_RETRY_TMR_EXP);
            }
        }
        break;

        case P2P_SESSION_MSG_TYPE_HOLD_REQ:
        {
            /* Start P2P hold ack timer */
            DPRINT(P2P_MODULE, "p2p hold recv: [client=%d], [model=%s], [status=%d]",
                   pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->model, pP2pClientProc->isP2pClientHold);
            startP2pClientFsmTmr(pP2pClientProc, P2P_CLIENT_FSM_EVENT_HOLD_ACK_TMR_EXP);

            /* Send hold response */
            sendP2pClientSessionMsg(pP2pClientProc, P2P_SESSION_MSG_TYPE_HOLD_RESP);

            /* Change FSM to hold */
            changeP2pClientFsmState(pP2pClientProc, P2P_CLIENT_STATE_HOLD);

            /* Hold all streams */
            pP2pClientProc->isP2pClientHold = TRUE;

            /* Validate user login session */
            if (TRUE == IsUserSessionValid(pP2pClientProc->pConnInfo->userLoginSessionIdx, pP2pClientProc->pConnInfo->userLoginSessionUid))
            {
                /* Hold all activities for this session */
                UserHold(pP2pClientProc->pConnInfo->userLoginSessionIdx);
            }
        }
        break;

        case P2P_SESSION_MSG_TYPE_RESUME_REQ:
        {
            /* Start P2P hold ack timer */
            DPRINT(P2P_MODULE, "p2p resume recv: [client=%d], [model=%s], [status=%d]",
                   pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->model, pP2pClientProc->isP2pClientHold);
            startP2pClientFsmTmr(pP2pClientProc, P2P_CLIENT_FSM_EVENT_HEARTBEAT_WAIT_TMR_EXP);

            /* Send hold response */
            sendP2pClientSessionMsg(pP2pClientProc, P2P_SESSION_MSG_TYPE_RESUME_RESP);

            /* Change FSM to heartbeat */
            changeP2pClientFsmState(pP2pClientProc, P2P_CLIENT_STATE_HEARTBEAT);

            /* Resume all streams */
            pP2pClientProc->isP2pClientHold = FALSE;

            /* Validate user login session */
            if (TRUE == IsUserSessionValid(pP2pClientProc->pConnInfo->userLoginSessionIdx, pP2pClientProc->pConnInfo->userLoginSessionUid))
            {
                /* Resume all activities for this session */
                UserResume(pP2pClientProc->pConnInfo->userLoginSessionIdx);
            }
        }
        break;

        case P2P_SESSION_MSG_TYPE_CLOSE:
        {
            /* P2P close received from client */
            DPRINT(P2P_MODULE, "p2p close recv: [client=%d], [model=%s]", pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->model);

            /* Stop P2P client process */
            stopP2pClientProcess(pP2pClientProc);
        }
        break;

        default:
        {
            EPRINT(P2P_MODULE, "invld session msg type recv: [client=%d], [msgType=%d], [model=%s]",
                   pP2pClientProc->pConnInfo->clientIdx, pSessionPayload->msgType, pP2pClientProc->pConnInfo->model);
            return;
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Process received client control message
 * @param   pP2pClientProc
 * @param   pDataBuff
 * @param   dataLen
 * @return  Network Command Status
 */
static NET_CMD_STATUS_e procP2pClientControlMsg(P2P_CLIENT_PROCESS_t *pP2pClientProc, CHAR *pDataBuff, UINT32 dataLen)
{
    UINT8                   p2pHeaderLen = P2P_CLIENT_MSG_HEADER_LEN;
    P2P_CLIENT_MSG_HEADER_t *pMsgHeader = &((P2P_CLIENT_MSG_t *)pDataBuff)->header;

    /* Validate message length */
    if (dataLen < p2pHeaderLen)
    {
        EPRINT(P2P_MODULE, "invld control msg length: [client=%d], [dataLen=%d]", pP2pClientProc->pConnInfo->clientIdx, dataLen);
        return CMD_PROCESS_ERROR;
    }

    /* Terminate message with null */
    pDataBuff[dataLen] = '\0';

    CHAR    *pMsgData = pDataBuff + p2pHeaderLen;
    UINT8   msgId = MAX_HEADER_REQ;
    UINT8   sessionIdx;

    /* Validate SOM in message */
    if (pMsgData[0] != SOM)
    {
        EPRINT(P2P_MODULE, "SOM not found in msg: [client=%d], [msg=%s]", pP2pClientProc->pConnInfo->clientIdx, pMsgData);
        return CMD_PROCESS_ERROR;
    }

    /* Remove SOM from message */
    pMsgData++;

    /* Parse message id */
    if (FindHeaderIndex(&pMsgData, &msgId) != CMD_SUCCESS)
    {
        EPRINT(P2P_MODULE, "fail to parse header index: [client=%d], [msg=%s]", pP2pClientProc->pConnInfo->clientIdx, pMsgData);
        return CMD_PROCESS_ERROR;
    }

    /* find session index from session id */
    if (FindSessionIndex(&pMsgData, &sessionIdx) != CMD_SUCCESS)
    {
        EPRINT(P2P_MODULE, "invld session id found: [client=%d], [msgId=%s]", pP2pClientProc->pConnInfo->clientIdx, headerReq[msgId]);
        return CMD_PROCESS_ERROR;
    }

    /* if session index don't match with assigned session */
    if (pP2pClientProc->pConnInfo->userLoginSessionIdx != sessionIdx)
    {
        EPRINT(P2P_MODULE, "invld session index found: [client=%d], [msgId=%s], [sessionIdx=%d], [userLoginSessionIdx=%d]",
               pP2pClientProc->pConnInfo->clientIdx, headerReq[msgId], sessionIdx, pP2pClientProc->pConnInfo->userLoginSessionIdx);
        return CMD_PROCESS_ERROR;
    }

    /* check if user is active */
    if (FALSE == IsUserSessionActive(sessionIdx))
    {
        EPRINT(P2P_MODULE, "user session is inactive: [client=%d], [msgId=%s], [sessionIdx=%d]",
               pP2pClientProc->pConnInfo->clientIdx, headerReq[msgId], sessionIdx);
        return CMD_PROCESS_ERROR;
    }

    UINT16 msgIdx = allocateP2pClientMsgIdx(pP2pClientProc, pMsgHeader->msgUid);
    if (msgIdx >= P2P_CLIENT_STORE_MSG_MAX)
    {
        EPRINT(P2P_MODULE, "fail to alloc new msgIdx: [client=%d], [model=%s], [msgUid=0x%x]",
               pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->model, pMsgHeader->msgUid);
        return CMD_PROCESS_ERROR;
    }

    switch(msgId)
    {
        case GET_CFG:
        {
            /* Process get config request */
            ProcessGetConfig(pMsgData, pP2pClientProc->p2pMsgUid[msgIdx].localMsgUid, CLIENT_CB_TYPE_P2P);

            /* Free client message index */
            freeP2pClientMsgIdx(pP2pClientProc, msgIdx);
        }
        break;

        case SET_CMD:
        {
            /* Process set command request */
            ProcessSetCommand(pMsgData, sessionIdx, pP2pClientProc->p2pMsgUid[msgIdx].localMsgUid, CLIENT_CB_TYPE_P2P);
        }
        break;

        case REQ_EVT:
        {
            /* Get and send live events to client */
            GetLiveEvents(sessionIdx, pP2pClientProc->p2pMsgUid[msgIdx].localMsgUid, CLIENT_CB_TYPE_P2P);
        }
        break;

        default:
        {
            /* Free client message index due to invalid control message */
            EPRINT(P2P_MODULE, "invld control msg: [client=%d], [msgId=%s]", pP2pClientProc->pConnInfo->clientIdx, headerReq[msgId]);
            freeP2pClientMsgIdx(pP2pClientProc, msgIdx);
        }
        break;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Process received client data message
 * @param   pP2pClientProc
 * @param   pDataBuff
 * @param   dataLen
 */
static void procP2pClientStreamMsg(P2P_CLIENT_PROCESS_t *pP2pClientProc, CHAR *pDataBuff, UINT32 dataLen)
{
    BOOL                    retSts;
    P2P_CLIENT_FD_TYPE_e    fdType = P2P_CLIENT_FD_TYPE_TWO_WAY_AUDIO;
    P2P_CLIENT_MSG_HEADER_t *pMsgHeader = &((P2P_CLIENT_MSG_t *)pDataBuff)->header;

    /* Check FD found or not */
    if (pMsgHeader->msgUid != pP2pClientProc->dataSendFdInfo[fdType].clientMsgUid)
    {
        EPRINT(P2P_MODULE, "fd not found to send stream: [client=%d], [clientMsgUid=0x%x]", pP2pClientProc->pConnInfo->clientIdx, pMsgHeader->msgUid);
        return;
    }

    /* Send data to respective module */
    retSts = SendP2pMsg(pP2pClientProc->dataSendFdInfo[fdType].sockFd, pDataBuff + P2P_CLIENT_MSG_HEADER_LEN, dataLen - P2P_CLIENT_MSG_HEADER_LEN, 0);
    if (retSts == REFUSE)
    {
        EPRINT(P2P_MODULE, "p2p stream recv fd closed: [client=%d], [msgUid=0x%x]", pP2pClientProc->pConnInfo->clientIdx, pMsgHeader->msgUid);
        clearP2pClientDataXferFdInfo(pP2pClientProc->pConnInfo->clientIdx, fdType);
    }
    else if (retSts != SUCCESS)
    {
        EPRINT(P2P_MODULE, "fail to send stream on p2p recv fd: [client=%d], [msgUid=0x%x]", pP2pClientProc->pConnInfo->clientIdx, pMsgHeader->msgUid);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Allocate free index and generate unique ID for every control message from client and message index
 * @param   pP2pClientProc
 * @param   msgUid - Msg ID received from client for control message
 * @param   pLocalMsgUid - Local unique ID for client and message
 * @return  Valid if client message index allocated else invalid
 */
static UINT16 allocateP2pClientMsgIdx(P2P_CLIENT_PROCESS_t *pP2pClientProc, UINT32 msgUid)
{
    UINT16 msgIdx;

    for (msgIdx = 0; msgIdx < P2P_CLIENT_STORE_MSG_MAX; msgIdx++)
    {
        if ((pP2pClientProc->p2pMsgUid[msgIdx].clientMsgUid == msgUid) && (pP2pClientProc->p2pMsgUid[msgIdx].localMsgUid))
        {
            WPRINT(P2P_MODULE, "client msgUid already allocated: [client=%d], [localMsgUid=0x%x], [msgUid=0x%x]",
                   pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->p2pMsgUid[msgIdx].localMsgUid, msgUid);
            return P2P_CLIENT_STORE_MSG_MAX;
        }
    }

    for (msgIdx = 0; msgIdx < P2P_CLIENT_STORE_MSG_MAX; msgIdx++)
    {
        if (pP2pClientProc->p2pMsgUid[msgIdx].localMsgUid == 0)
        {
            pP2pClientProc->p2pMsgUid[msgIdx].clientMsgUid = msgUid;
            pP2pClientProc->p2pMsgUid[msgIdx].localMsgUid =
                    ((UINT16)GetRandomNum() << 16) | (P2P_CLIENT_MSG_ID_MASK | (msgIdx << 4) |  pP2pClientProc->pConnInfo->clientIdx);
            break;
        }
    }

    return msgIdx;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Free allocated message index
 * @param   pP2pClientProc
 * @param   msgIdx
 */
static void freeP2pClientMsgIdx(P2P_CLIENT_PROCESS_t *pP2pClientProc, UINT16 msgIdx)
{
    /* Validate message index */
    if (msgIdx >= P2P_CLIENT_STORE_MSG_MAX)
    {
        EPRINT(P2P_MODULE, "invld msgIdx: [client=%d], [msgIdx=%d]", pP2pClientProc->pConnInfo->clientIdx, msgIdx);
        return;
    }

    /* Free message index */
    pP2pClientProc->p2pMsgUid[msgIdx].clientMsgUid = 0;
    pP2pClientProc->p2pMsgUid[msgIdx].localMsgUid = 0;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get client message index from local unique identity
 * @param   localMsgUid
 * @param   pClientIdx
 * @param   pMsgIdx
 * @return
 */
static BOOL getClientMsgIdxFromLocalMsgUid(UINT32 localMsgUid, UINT8 *pClientIdx, UINT16 *pMsgIdx)
{
    /* If connection already closed then return */
    if ((INT32)localMsgUid == INVALID_CONNECTION)
    {
        /* Nothing to do if connection closed */
        return FALSE;
    }

    /* Validate local message uID */
    if ((localMsgUid & P2P_CLIENT_MSG_ID_MASK) != P2P_CLIENT_MSG_ID_MASK)
    {
        EPRINT(P2P_MODULE, "invld local msgUid: [localMsgUid=0x%x]", localMsgUid);
        return FALSE;
    }

    /* Get client index */
    *pClientIdx = localMsgUid & 0x0000000F;
    if (*pClientIdx >= P2P_CLIENT_SUPPORT_MAX)
    {
        EPRINT(P2P_MODULE, "invld client index: [localMsgUid=0x%x]", localMsgUid);
        return FALSE;
    }

    /* Get message index */
    *pMsgIdx = (localMsgUid >> 4) & 0x000000FF;
    if (*pMsgIdx >= P2P_CLIENT_STORE_MSG_MAX)
    {
        EPRINT(P2P_MODULE, "invld message index: [localMsgUid=0x%x]", localMsgUid);
        return FALSE;
    }

    /* Check index still allocated or not for given local msg id */
    if (p2pClientProc[*pClientIdx].p2pMsgUid[*pMsgIdx].localMsgUid != localMsgUid)
    {
        EPRINT(P2P_MODULE, "invld localMsgUid: [require=0x%x], [avail=0x%x]", localMsgUid, p2pClientProc[*pClientIdx].p2pMsgUid[*pMsgIdx].localMsgUid);
        return FALSE;
    }

    /* We got required info */
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send client control message data (Equivalent to SendToSocket)
 * @param   localMsgUid
 * @param   pSendMsg
 * @param   sendMsgLen
 * @param   timeout
 * @return  TRUE on success; FALSE otherwise
 */
BOOL SendP2pClientControlData(INT32 localMsgUid, UINT8 *pSendMsg, UINT32 sendMsgLen, UINT32 timeout)
{
    UINT8                   clientIdx;
    UINT16                  msgIdx;
    P2P_CLIENT_MSG_HEADER_t msgHeader;

    /* Get client index and message index */
    if (FALSE == getClientMsgIdxFromLocalMsgUid(localMsgUid, &clientIdx, &msgIdx))
    {
        /* Print error debug only for valid socket */
        if (localMsgUid != INVALID_CONNECTION)
        {
            EPRINT(P2P_MODULE, "fail to get client: [localMsgUid=0x%x]", localMsgUid);
        }
        return FALSE;
    }

    /* Get client information */
    P2P_CLIENT_PROCESS_t *pP2pClientProc = &p2pClientProc[clientIdx];

    /* Is client ready to receive data? */
    if (TRUE == pP2pClientProc->isP2pClientHold)
    {
        /* Client is not ready to receive data. As hold send by client */
        EPRINT(P2P_MODULE, "client in hold state: [client=%d], [msgUid=0x%x]",
               pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->p2pMsgUid[msgIdx].clientMsgUid);
        return FALSE;
    }

    /* Prepare header */
    MUTEX_LOCK(pP2pClientProc->socketLock);
    prepareMsgHeader(&msgHeader, P2P_MSG_TYPE_CONTROL, pP2pClientProc->p2pMsgUid[msgIdx].clientMsgUid, sendMsgLen);

    /* Send control message */
    if (((SUCCESS != SendP2pMsg(pP2pClientProc->pConnInfo->sockFd, (CHAR *)&msgHeader, P2P_CLIENT_MSG_HEADER_LEN, P2P_HEADER_SEND_WAIT_SEC_MAX)))
            || (SUCCESS != SendP2pMsg(pP2pClientProc->pConnInfo->sockFd, (CHAR *)pSendMsg, sendMsgLen, timeout)))
    {
        MUTEX_UNLOCK(pP2pClientProc->socketLock);
        EPRINT(P2P_MODULE, "fail to send control msg resp: [client=%d], [model=%s]", pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->model);
        return FALSE;
    }
    MUTEX_UNLOCK(pP2pClientProc->socketLock);

    /* Message sent successfully */
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send client audio/video frame related data (Equivalent to SendToSocket)
 * @param   localMsgUid
 * @param   pSendMsg
 * @param   sendMsgLen
 * @return  TRUE on success; FALSE otherwise
 */
BOOL SendP2pClientFrameData(INT32 localMsgUid, UINT8 *pSendMsg, UINT32 sendMsgLen, UINT32 timeout)
{
    UINT8                   clientIdx;
    UINT16                  msgIdx;
    P2P_CLIENT_MSG_HEADER_t msgHeader;

    /* Validate length of stream. It should atleast frame header size */
    if (sendMsgLen < FRAME_HEADER_LEN_MAX)
    {
        EPRINT(P2P_MODULE, "invld frame length: [localMsgUid=0x%x], [length=%d]", localMsgUid, sendMsgLen);
        return FALSE;
    }

    /* Get client index and message index */
    if (FALSE == getClientMsgIdxFromLocalMsgUid(localMsgUid, &clientIdx, &msgIdx))
    {
        EPRINT(P2P_MODULE, "fail to get client: [localMsgUid=0x%x]", localMsgUid);
        return FALSE;
    }

    /* Get client information */
    P2P_CLIENT_PROCESS_t *pP2pClientProc = &p2pClientProc[clientIdx];

    /* Is client ready to receive stream? */
    if (TRUE == pP2pClientProc->isP2pClientHold)
    {
        /* Client is not ready to receive stream but do not close socket. As hold sent by client */
        return TRUE;
    }

    /* Prepare header */
    MUTEX_LOCK(pP2pClientProc->socketLock);
    prepareMsgHeader(&msgHeader, P2P_MSG_TYPE_DATA, pP2pClientProc->p2pMsgUid[msgIdx].clientMsgUid, sendMsgLen);

    /* Send control message */
    if (((SUCCESS != SendP2pMsg(pP2pClientProc->pConnInfo->sockFd, (CHAR *)&msgHeader, P2P_CLIENT_MSG_HEADER_LEN, timeout)))
            || (SUCCESS != SendP2pMsg(pP2pClientProc->pConnInfo->sockFd, (CHAR *)pSendMsg, sendMsgLen, timeout)))
    {
        MUTEX_UNLOCK(pP2pClientProc->socketLock);
        EPRINT(P2P_MODULE, "fail to send frame msg: [client=%d], [model=%s]", pP2pClientProc->pConnInfo->clientIdx, pP2pClientProc->pConnInfo->model);
        return FALSE;
    }
    MUTEX_UNLOCK(pP2pClientProc->socketLock);

    /* Message sent successfully */
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Client Message Close Callback (Equivalent to CloseSocket)
 * @param   pLocalMsgUid
 */
void CloseP2pClientConn(INT32 *pLocalMsgUid)
{
    UINT8   clientIdx;
    UINT16  msgIdx;

    /* Return if invalid msgUid found */
    if (*pLocalMsgUid == INVALID_CONNECTION)
    {
        /* Local message id already destroy */
        return;
    }

    /* Get client index and message index */
    if (FALSE == getClientMsgIdxFromLocalMsgUid(*pLocalMsgUid, &clientIdx, &msgIdx))
    {
        EPRINT(P2P_MODULE, "fail to get client: [localMsgUid=0x%x]", *pLocalMsgUid);
    }
    else
    {
        /* free client msg index */
        freeP2pClientMsgIdx(&p2pClientProc[clientIdx], msgIdx);
    }

    /* Free index */
    *pLocalMsgUid = INVALID_CONNECTION;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Logout P2P client session
 * @param   pP2pClientProc
 */
static void logoutP2pClientSession(P2P_CLIENT_PROCESS_t *pP2pClientProc)
{
    /* Logout user if successfully logged-in */
    if (pP2pClientProc->pConnInfo->userLoginSessionIdx >= MAX_NW_CLIENT)
    {
        /* User not logged-in */
        return;
    }

    UINT8 userLoginSessionIdx = pP2pClientProc->pConnInfo->userLoginSessionIdx;
    INT32 userLoginSessionUid = pP2pClientProc->pConnInfo->userLoginSessionUid;
    pP2pClientProc->pConnInfo->userLoginSessionUid = INVALID_CONNECTION;
    pP2pClientProc->pConnInfo->userLoginSessionIdx = MAX_NW_CLIENT;

    /* Validate login session */
    if (FALSE == IsUserSessionValid(userLoginSessionIdx, userLoginSessionUid))
    {
        /* User session is not valid */
        EPRINT(P2P_MODULE, "invld user session: [client=%d], [userLoginSessionIdx=%d], [userLoginSessionUid=%d]",
               pP2pClientProc->pConnInfo->clientIdx, userLoginSessionIdx, userLoginSessionUid);
        return;
    }
    else
    {
        /* User logout */
        DPRINT(P2P_MODULE, "user logout: [client=%d], [userLoginSessionIdx=%d], [userLoginSessionUid=%d]",
               pP2pClientProc->pConnInfo->clientIdx, userLoginSessionIdx, userLoginSessionUid);
    }

    /* Valid user session found. So logout user */
    UserLogout(userLoginSessionIdx);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get P2P client data transfer fd. It will create two socket fds: one for sending data
 *          (received from P2P client) and other for receiving (data sent by sending socket).
 *          We will connect these sockets with each other. Data receive FD closed by respective
 *          module; eg. Two way audio module, File transfer module etc.
 * @param   fdType
 * @param   localMsgUid
 * @param   pDataRecvFd
 * @return  TRUE on success; FALSE otherwise
 */
BOOL GetP2pClientDataXferFd(P2P_CLIENT_FD_TYPE_e fdType, INT32 localMsgUid, INT32 *pDataRecvFd)
{
    UINT8           clientIdx;
    UINT16          msgIdx;
    UINT16          recvPort = 0;
    UINT16          sendPort = 0;
    IP_ADDR_INFO_t  addrInfo;

    /* Validate required fd type */
    if (fdType >= P2P_CLIENT_FD_TYPE_MAX)
    {
        EPRINT(P2P_MODULE, "invld required fd type: [localMsgUid=0x%x], [fdType=%d]", localMsgUid, fdType);
        return FALSE;
    }

    /* Get client index and message index */
    if (FALSE == getClientMsgIdxFromLocalMsgUid(localMsgUid, &clientIdx, &msgIdx))
    {
        EPRINT(P2P_MODULE, "fail to get client: [localMsgUid=0x%x]", localMsgUid);
        return FALSE;
    }

    /* Lock client to access */
    MUTEX_LOCK(p2pClientFdInfo[fdType].clientLock);

    /* Is fd already allocated */
    if (p2pClientFdInfo[fdType].clientIdx != P2P_CLIENT_SUPPORT_MAX)
    {
        /* Now free client lock */
        MUTEX_UNLOCK(p2pClientFdInfo[fdType].clientLock);
        EPRINT(P2P_MODULE, "fd type already allocated: [client=%d], [localMsgUid=0x%x], [allocClient=%d], [fdType=%d]",
               clientIdx, localMsgUid, p2pClientFdInfo[fdType].clientIdx, fdType);
        return FALSE;
    }

    /* Get client information */
    P2P_CLIENT_PROCESS_t *pP2pClientProc = &p2pClientProc[clientIdx];

    /* Set invalid connection for receive FD */
    pP2pClientProc->dataSendFdInfo[fdType].sockFd = INVALID_CONNECTION;
    *pDataRecvFd = INVALID_CONNECTION;

    do
    {
        /* Create send socket */
        snprintf(addrInfo.ip, sizeof(addrInfo.ip), LOCAL_CLIENT_IP_ADDR);
        addrInfo.port = 0;
        if (FALSE == OpenP2pSocket(TRUE, &addrInfo, &pP2pClientProc->dataSendFdInfo[fdType].sockFd))
        {
            EPRINT(P2P_MODULE, "fail to create send socket: [client=%d], [fdType=%d]", clientIdx, fdType);
            break;
        }

        /* Store allocated send port */
        sendPort = addrInfo.port;

        /* Create receive socket */
        snprintf(addrInfo.ip, sizeof(addrInfo.ip), LOCAL_CLIENT_IP_ADDR);
        addrInfo.port = 0;
        if (FALSE == OpenP2pSocket(TRUE, &addrInfo, pDataRecvFd))
        {
            EPRINT(P2P_MODULE, "fail to create recv socket: [client=%d], [fdType=%d]", clientIdx, fdType);
            break;
        }

        /* Store allocated receive port */
        recvPort = addrInfo.port;

        /* Connect send socket with receive addr */
        snprintf(addrInfo.ip, sizeof(addrInfo.ip), LOCAL_CLIENT_IP_ADDR);
        addrInfo.port = recvPort;
        if (FALSE == ConnectP2pSocket(pP2pClientProc->dataSendFdInfo[fdType].sockFd, &addrInfo))
        {
            EPRINT(P2P_MODULE, "fail to connect send socket: [client=%d], [fdType=%d]", clientIdx, fdType);
            break;
        }

        /* Connect receive socket with send addr */
        snprintf(addrInfo.ip, sizeof(addrInfo.ip), LOCAL_CLIENT_IP_ADDR);
        addrInfo.port = sendPort;
        if (FALSE == ConnectP2pSocket(*pDataRecvFd, &addrInfo))
        {
            EPRINT(P2P_MODULE, "fail to connect recv socket: [client=%d], [fdType=%d]", clientIdx, fdType);
            break;
        }

        /* Assign client index to fd info */
        p2pClientFdInfo[fdType].clientIdx = clientIdx;

        /* Assign message unique id to fd to identify data */
        pP2pClientProc->dataSendFdInfo[fdType].clientMsgUid = pP2pClientProc->p2pMsgUid[msgIdx].clientMsgUid;

        /* Now free client lock */
        MUTEX_UNLOCK(p2pClientFdInfo[fdType].clientLock);
        DPRINT(P2P_MODULE, "p2p data xfer fd created: [client=%d], [fdType=%d], [clientMsgUid=0x%x], [sendFd=%d], [recvFd=%d]",
               clientIdx, fdType, pP2pClientProc->dataSendFdInfo[fdType].clientMsgUid, pP2pClientProc->dataSendFdInfo[fdType].sockFd, *pDataRecvFd);
        return TRUE;

    }while(0);

    /* Close all sockets on failure */
    CloseSocket(&pP2pClientProc->dataSendFdInfo[fdType].sockFd);
    CloseSocket(pDataRecvFd);

    /* Now free client lock */
    MUTEX_UNLOCK(p2pClientFdInfo[fdType].clientLock);
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Close P2P data transfer fd: It will close send FD and receive FD will be closed by
 *          respective module.
 * @param   fdType
 */
void CloseP2pClientDataXferFd(P2P_CLIENT_FD_TYPE_e fdType)
{
    UINT8 clientIdx;

    /* Validate required fd type */
    if (fdType >= P2P_CLIENT_FD_TYPE_MAX)
    {
        EPRINT(P2P_MODULE, "invld required fd type: [fdType=%d]", fdType);
        return;
    }

    /* Get client index for data transfer fd and check whether it is valid or not */
    MUTEX_LOCK(p2pClientFdInfo[fdType].clientLock);
    clientIdx = p2pClientFdInfo[fdType].clientIdx;
    if (clientIdx >= P2P_CLIENT_SUPPORT_MAX)
    {
        MUTEX_UNLOCK(p2pClientFdInfo[fdType].clientLock);
        return;
    }
    MUTEX_UNLOCK(p2pClientFdInfo[fdType].clientLock);

    /* Clear client data transfer fd info */
    clearP2pClientDataXferFdInfo(clientIdx, fdType);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Clear P2P client data transfer FD information
 * @param   clientIdx
 * @param   fdType
 */
static void clearP2pClientDataXferFdInfo(UINT8 clientIdx, P2P_CLIENT_FD_TYPE_e fdType)
{
    /* Lock client to access */
    MUTEX_LOCK(p2pClientFdInfo[fdType].clientLock);

    /* Remove client index from fd info */
    if (clientIdx == p2pClientFdInfo[fdType].clientIdx)
    {
        /* Remove binding between client and data recv info */
        DPRINT(P2P_MODULE, "free alloc data xfer fd: [client=%d], [fdType=%d]", clientIdx, fdType);
        p2pClientFdInfo[fdType].clientIdx = P2P_CLIENT_SUPPORT_MAX;
        p2pClientProc[clientIdx].dataSendFdInfo[fdType].clientMsgUid = 0;
        CloseSocket(&p2pClientProc[clientIdx].dataSendFdInfo[fdType].sockFd);
    }
    else
    {
        EPRINT(P2P_MODULE, "invld client for fd close: [client=%d], [fdType=%d]", clientIdx, fdType);
    }

    /* Now free client lock */
    MUTEX_UNLOCK(p2pClientFdInfo[fdType].clientLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get network client type from p2p client type
 * @param   p2pClientType
 * @return  Network client type
 */
static NW_CLIENT_TYPE_e getNwClientType(P2P_DEVICE_TYPE_e p2pClientType)
{
    switch(p2pClientType)
    {
        case P2P_DEVICE_TYPE_MOBILE_CLIENT:
            return NW_CLIENT_TYPE_P2P_MOBILE;

        case P2P_DEVICE_TYPE_DEVICE_CLIENT:
            return NW_CLIENT_TYPE_P2P_DESKTOP;

        default:
            return NW_CLIENT_TYPE_MAX;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get P2P session message type string from enum name
 * @return  P2P session message type string
 */
static const CHAR *getSessionMsgTypeStr(P2P_SESSION_MSG_TYPE_e sessionMsgType)
{
#define SKIP_PREFIX_SIZE (sizeof("P2P_SESSION_MSG_TYPE_")-1)
    switch (sessionMsgType)
    {
        CASE_TO_STR(P2P_SESSION_MSG_TYPE_HEARTBEAT_REQ, SKIP_PREFIX_SIZE);
        CASE_TO_STR(P2P_SESSION_MSG_TYPE_HEARTBEAT_RESP, SKIP_PREFIX_SIZE);
        CASE_TO_STR(P2P_SESSION_MSG_TYPE_HANDSHAKE_REQ, SKIP_PREFIX_SIZE);
        CASE_TO_STR(P2P_SESSION_MSG_TYPE_HANDSHAKE_RESP, SKIP_PREFIX_SIZE);
        CASE_TO_STR(P2P_SESSION_MSG_TYPE_HANDSHAKE_ACK, SKIP_PREFIX_SIZE);
        CASE_TO_STR(P2P_SESSION_MSG_TYPE_HOLD_REQ, SKIP_PREFIX_SIZE);
        CASE_TO_STR(P2P_SESSION_MSG_TYPE_HOLD_RESP, SKIP_PREFIX_SIZE);
        CASE_TO_STR(P2P_SESSION_MSG_TYPE_RESUME_REQ, SKIP_PREFIX_SIZE);
        CASE_TO_STR(P2P_SESSION_MSG_TYPE_RESUME_RESP, SKIP_PREFIX_SIZE);
        CASE_TO_STR(P2P_SESSION_MSG_TYPE_CLOSE, SKIP_PREFIX_SIZE);
        CASE_TO_STR(P2P_SESSION_MSG_TYPE_MAX, SKIP_PREFIX_SIZE);
    }
#undef SKIP_PREFIX_SIZE
    return ("UNKNOWN");
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
