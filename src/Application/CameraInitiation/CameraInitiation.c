// #################################################################################################
//  FILE BRIEF
// #################################################################################################
/**
@file		CameraInitiation.c
@brief      File containing the definition of different functions for camera login request and give
            response. It serves all camera initiation related request and response.
*/
// #################################################################################################
//  @INCLUDES
// #################################################################################################
/* Application Includes */
#include "CameraInitiation.h"
#include "Config.h"
#include "DateTime.h"
#include "DebugLog.h"
#include "EventHandler.h"
#include "Queue.h"
#include "TcpClient.h"
#include "Utils.h"

// #################################################################################################
//  @DEFINES
// #################################################################################################
#define DATA_RECV_WAIT_TIME_SEC        1
#define RECV_MSG_HEADER_LENGTH         20
#define MAX_FIELD_SIZE                 25
#define MAX_KEEP_ALIVE_TIME            5
#define CI_RECV_MSG_HEADER_LEN         1024
#define MAX_ADVANCE_DETAIL_LEN         31
#define MAX_EVT_QUEUE_SIZE             50
#define INIT_POLL_SLEEP_MAX_US         20000
#define INIT_POLL_RETRY_MAX            3
#define NO_CAM_POLL_SLEEP_MAX_SEC      1

#define CAM_INIT_DEFAULT_NAME          "CamInit %.8s"
#define GET_CAM_INIT_NAME_OFFSET(x)    (x + 9)

#define POLL_CAM_INIT_THREAD_STACK_SZ  (1 * MEGA_BYTE)
#define CAM_INIT_EVENT_THREAD_STACK_SZ (1 * MEGA_BYTE)
#define CAM_INIT_MAIN_THREAD_STACK_SZ  (1 * MEGA_BYTE)

// #################################################################################################
//  @DATA TYPES
// #################################################################################################
typedef enum
{
    CI_MOTION_DETECTION = 0,
    CI_VIEW_TAMPER = 1,
    CI_TRIP_WIRE = 2,
    CI_ZONE_INTRUSION = 4,
    CI_MISSING_OBJ = 6,
    CI_SUSPICIOUS_OBJ = 7,
    CI_OBJECT_COUNTING_IN = 8,
    CI_ALARM_IN_1 = 9,
    CI_AUDIO_EXCEPTION = 10,
    CI_LOITERING_DETECTION = 11,
    CI_ALARM_OUT = 12,
    CI_OBJECT_COUNTING_OUT = 13,
    CI_NO_MOTION_DETECTION = 14,
    CI_ALARM_IN_2 = 15,
    MAX_CI_CAM_EVENT
} CI_CAMERA_EVENT_e;

typedef enum
{
    CMD_FD_NONE,
    CMD_GET_FD,
    CMD_WAIT_FOR_FD,
    CMD_FD_PENDING,
    CMD_FD_BUSY
} CI_CMD_STATE_TYPE_e;

typedef enum
{
    CI_REG_LOG = 0,
    CI_ACK_LOG = 1,
    CI_REQ_POLL = 2,
    CI_ACK_POL = 3,
    CI_RCV_EVT = 4,
    CI_ACK_DID = 5,
    MAX_CI_HEADER_REQ
} CAM_HEADER_REQ_e;

typedef struct
{
    UINT32 loginCode;
    UINT16 softwareVersion;
    UINT16 softwareRevision;
    UINT8  audioIn;
    UINT8  audioOut;
    UINT8  sensorIn;
    UINT8  alarmOut;
    UINT8  ptzSupport;
    CHAR   cameraMacAddr[MAX_MAC_ADDRESS_WIDTH];
    CHAR   productName[MAX_PRODUCT_NAME_SIZE];
    CHAR   cameraModelname[MAX_MODEL_NAME_SIZE];
    UINT8  maxProfileSupport;
    UINT8  sdSupport;
    UINT8  osdSupport;
    UINT8  zoomSupport;
    UINT8  focusSupport;
    UINT8  irisSupport;
    UINT8  wifiSupport;
    INT32  cameraWiseFd;
    BOOL   isSessionIsActive;
    CHAR   ipAddr[IPV6_ADDR_LEN_MAX];
} REQ_LOG_PARAM_t;

typedef struct
{
    BOOL  status;
    UINT8 deviceId;
    UINT8 eventStatus;
    UINT8 eventType;
    UINT8 event;
    BOOL  eventState;
    CHAR  advanceDetail[MAX_ADVANCE_DETAIL_LEN];
} CAM_INIT_EVENT_t;

typedef struct
{
    CAM_INIT_EVENT_t camEventInfo[MAX_EVT_QUEUE_SIZE];
    UINT8            rdWrIndex;
    pthread_rwlock_t ciEvtQueueLock;
} CAM_EVENT_INFO_t;

typedef struct
{
    AUTO_INIT_CAM_LIST_INFO listInfo[MAX_AUTO_INIT_CAM_LIST];
    pthread_mutex_t         camListLock;
    INT8                    writeListIndex;
    INT8                    readListIndex;
} AUTO_INIT_CAM_LIST;

typedef struct
{
    INT32           commandCamFd;
    UINT8           stateReq;
    pthread_mutex_t cmdDataLock;
} CMD_DATA_t;

typedef struct
{
    UINT8         camIndex[MAX_CAMERA];
    struct pollfd pollFds[MAX_CAMERA];
} CI_POLL_FD_INFO_t;

// #################################################################################################
//  @STATIC VARIABLES
// #################################################################################################
static REQ_LOG_PARAM_t    loginDataForCamInit[MAX_CAMERA];
static AUTO_INIT_CAM_LIST autoInitCamList;
static CMD_DATA_t         commandToCamInfo[MAX_CAMERA][MAX_CMD_QUEUE];
static CAM_EVENT_INFO_t   eventCamInitInfo[MAX_CAMERA];
static INT32              ciServerSockFd = INVALID_CONNECTION;
static BOOL               terminateFlag = FALSE;

/* Polling related variables */
static UINT8 ciPollDuration;
static UINT8 ciPollInterval;

/* Thread Ids for different threads */
static pthread_t pollThreadId;
static pthread_t eventThreadId;
static pthread_t mainThread;

static const CHARPTR camHeaderReq[MAX_CI_HEADER_REQ] = {
    "REG_LOG",   // index 0
    "ACK_LOG",   // index 1
    "REQ_POLL",  // index 2
    "ACK_POL",   // index 3
    "RCV_EVT",   // index 4
    "ACK_DID",   // index 5
};

// #################################################################################################
//  @PROTOTYPES
// #################################################################################################
//-------------------------------------------------------------------------------------------------
static BOOL initCameraInitiationNw(void);
//-------------------------------------------------------------------------------------------------
static VOIDPTR runCameraInitiationMain(VOIDPTR arg);
//-------------------------------------------------------------------------------------------------
static VOIDPTR pollForCameraInit(VOIDPTR arg);
//-------------------------------------------------------------------------------------------------
static VOIDPTR receiveEventFromCamInit(VOIDPTR arg);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e findHeaderIndexCI(CHARPTR *pStrBufPtr, UINT8PTR pMsgId);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e assignCamera(CHARPTR source, INT32 connFd, UINT8PTR camAssign, const CHAR *camIpAddr);
//-------------------------------------------------------------------------------------------------
static void deInitCameraInitiation(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
static UINT8 checkIfConfiguredLessMax(UINT8PTR firstFreeIndex, CHARPTR source);
//-------------------------------------------------------------------------------------------------
static CAMERA_EVENT_e ConvertCamEvtToDeviceEvt(CI_CAMERA_EVENT_e ciCamEvent);
//-------------------------------------------------------------------------------------------------
static void arrangeCamInitList(CHARPTR macAddr);
//-------------------------------------------------------------------------------------------------
static void clearCamInitList(void);
//-------------------------------------------------------------------------------------------------
static void generateCamLoginResponse(CHARPTR replyMsg, NET_CMD_STATUS_e cmdResponse, UINT32 deviceId);
//-------------------------------------------------------------------------------------------------
// #################################################################################################
// @FUNCTIONS
// #################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   InitCameraInitiationTcpNw
 * @return  SUCCESS/FAIL
 */
void InitCameraInitiationTcpNw(void)
{
    UINT8            index;
    GENERAL_CONFIG_t generalConfig;
    UINT8            cmdIndex;

    terminateFlag = FALSE;
    ReadGeneralConfig(&generalConfig);
    DPRINT(CAMERA_INITIATION, "camera initiation: [pollDuration=%d], [pollInterval=%d]", generalConfig.pollDuration, generalConfig.pollInterval);

    for (index = 0; index < getMaxCameraForCurrentVariant(); index++)
    {
        loginDataForCamInit[index].isSessionIsActive = INACTIVE;
        loginDataForCamInit[index].cameraWiseFd = INVALID_CONNECTION;
    }

    autoInitCamList.readListIndex = 0;
    autoInitCamList.writeListIndex = 0;

    ciPollInterval = generalConfig.pollInterval;
    ciPollDuration = generalConfig.pollDuration;

    for (index = 0; index < MAX_AUTO_INIT_CAM_LIST; index++)
    {
        RESET_STR_BUFF(autoInitCamList.listInfo[index].cameraMacAddr);
        RESET_STR_BUFF(autoInitCamList.listInfo[index].productName);
        RESET_STR_BUFF(autoInitCamList.listInfo[index].cameraModelname);
    }

    for (index = 0; index < MAX_CAMERA; index++)
    {
        eventCamInitInfo[index].rdWrIndex = 0;
        pthread_rwlock_init(&eventCamInitInfo[index].ciEvtQueueLock, NULL);

        for (cmdIndex = 0; cmdIndex < MAX_EVT_QUEUE_SIZE; cmdIndex++)
        {
            eventCamInitInfo[index].camEventInfo[cmdIndex].status = FREE;
        }

        for (cmdIndex = 0; cmdIndex < MAX_CMD_QUEUE; cmdIndex++)
        {
            commandToCamInfo[index][cmdIndex].commandCamFd = INVALID_CONNECTION;
            commandToCamInfo[index][cmdIndex].stateReq = CMD_FD_NONE;
            MUTEX_INIT(commandToCamInfo[index][cmdIndex].cmdDataLock, NULL);
        }
    }

#if !defined(OEM_JCI)
    if (FAIL == Utils_CreateThread(&pollThreadId, pollForCameraInit, NULL, JOINABLE_THREAD, POLL_CAM_INIT_THREAD_STACK_SZ))
    {
        EPRINT(CAMERA_INITIATION, "cam init poll thread not started");
    }
    else if (FAIL == Utils_CreateThread(&eventThreadId, receiveEventFromCamInit, NULL, JOINABLE_THREAD, CAM_INIT_EVENT_THREAD_STACK_SZ))
    {
        EPRINT(CAMERA_INITIATION, "cam init recv event thread not started");
    }

    initCameraInitiationNw();
#endif
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief DeInitCameraInitiationTcpNw
 */
void DeInitCameraInitiationTcpNw(void)
{
    UINT8 camIndex, cmdIndex;

    DPRINT(CAMERA_INITIATION, "cam initiation de-init started");
    terminateFlag = TRUE;

#if !defined(OEM_JCI)
    pthread_join(pollThreadId, NULL);
    DPRINT(CAMERA_INITIATION, "cam init poll thread exit");

    pthread_join(eventThreadId, NULL);
    DPRINT(CAMERA_INITIATION, "cam init event thread exit");

    pthread_join(mainThread, NULL);
    DPRINT(CAMERA_INITIATION, "cam init main thread exit");
    CloseSocket(&ciServerSockFd);
#endif

    for (camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
    {
        for (cmdIndex = 0; cmdIndex < MAX_CMD_QUEUE; cmdIndex++)
        {
            MUTEX_LOCK(commandToCamInfo[camIndex][cmdIndex].cmdDataLock);
            CloseSocket(&commandToCamInfo[camIndex][cmdIndex].commandCamFd);
            commandToCamInfo[camIndex][cmdIndex].stateReq = CMD_FD_NONE;
            MUTEX_UNLOCK(commandToCamInfo[camIndex][cmdIndex].cmdDataLock);
        }

        deInitCameraInitiation(camIndex);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   CiServerConfigChangeNotify
 * @param   newGeneralConfig
 * @param   oldGeneralConfig
 */
void CiServerConfigChangeNotify(GENERAL_CONFIG_t newGeneralConfig, GENERAL_CONFIG_t *oldGeneralConfig)
{
    if (newGeneralConfig.autoAddCamTcpPort != oldGeneralConfig->autoAddCamTcpPort)
    {
        DPRINT(CAMERA_INITIATION, "cam init port change: [oldPort=%d], [newPort=%d]", oldGeneralConfig->autoAddCamTcpPort,
               newGeneralConfig.autoAddCamTcpPort);
        DeInitCameraInitiationTcpNw();
        InitCameraInitiationTcpNw();
    }
    else if ((newGeneralConfig.pollDuration != oldGeneralConfig->pollDuration) || (newGeneralConfig.pollInterval != oldGeneralConfig->pollInterval))
    {
        DPRINT(CAMERA_INITIATION, "cam init duration/interval change: [pollDuration=%d], [pollInterval=%d]", newGeneralConfig.pollDuration,
               newGeneralConfig.pollInterval);
        ciPollDuration = newGeneralConfig.pollDuration;
        ciPollInterval = newGeneralConfig.pollInterval;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   CiStreamCamConfigChange
 * @param   newCameraConfig
 * @param   oldCameraConfig
 * @param   cameraIndex
 */
void CiStreamCamConfigChange(CAMERA_CONFIG_t newCameraConfig, CAMERA_CONFIG_t *oldCameraConfig, UINT8 cameraIndex)
{
    UINT8 cmdIndex;

    /* Nothing to do if old or new camera is not CI camera */
    if ((newCameraConfig.type != AUTO_ADDED_CAMERA) && (oldCameraConfig->type != AUTO_ADDED_CAMERA))
    {
        /* It is not CI camera */
        return;
    }

    /* Nothing to do if old or new camera status is same and camera type is same */
    if ((newCameraConfig.camera == oldCameraConfig->camera) && (newCameraConfig.type == oldCameraConfig->type))
    {
        /* No change in camera config */
        return;
    }

    /* Nothing to do when new CI camera configured */
    if ((TRUE == newCameraConfig.camera) && (newCameraConfig.type == AUTO_ADDED_CAMERA))
    {
        /* Camera is enabled */
        return;
    }

    /* Deinit the CI camera on config disable/default */
    for (cmdIndex = 0; cmdIndex < MAX_CMD_QUEUE; cmdIndex++)
    {
        MUTEX_LOCK(commandToCamInfo[cameraIndex][cmdIndex].cmdDataLock);
        if (commandToCamInfo[cameraIndex][cmdIndex].stateReq != CMD_FD_NONE)
        {
            CloseSocket(&commandToCamInfo[cameraIndex][cmdIndex].commandCamFd);
            commandToCamInfo[cameraIndex][cmdIndex].stateReq = CMD_FD_NONE;
        }
        MUTEX_UNLOCK(commandToCamInfo[cameraIndex][cmdIndex].cmdDataLock);
    }

    deInitCameraInitiation(cameraIndex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initialize tcp server for camera initiation
 * @return  SUCCESS/FAIL
 */
static BOOL initCameraInitiationNw(void)
{
    INT32            sockOpt = TRUE;
    SOCK_ADDR_INFO_u nvrServerAddr;
    GENERAL_CONFIG_t generalCfg;

    ciServerSockFd = socket(AF_INET6, TCP_SOCK_OPTIONS, 0);
    if (ciServerSockFd == INVALID_CONNECTION)
    {
        EPRINT(CAMERA_INITIATION, "fail to create cam init socket: [err=%s]", STR_ERR);
        return FAIL;
    }

    /* Read general config for camera init port */
    ReadGeneralConfig(&generalCfg);
    DPRINT(CAMERA_INITIATION, "cam initiation: [tcpPort=%d]", generalCfg.autoAddCamTcpPort);

    /* Init socket structure before using it */
    memset(&nvrServerAddr, 0, sizeof(nvrServerAddr));
    nvrServerAddr.sockAddr6.sin6_family = AF_INET6;
    nvrServerAddr.sockAddr6.sin6_addr = in6addr_any;
    nvrServerAddr.sockAddr6.sin6_port = htons(generalCfg.autoAddCamTcpPort);

    do
    {
        if (setsockopt(ciServerSockFd, SOL_SOCKET, SO_REUSEADDR, &sockOpt, sizeof(sockOpt)) < STATUS_OK)
        {
            EPRINT(CAMERA_INITIATION, "fail to set socket option: [err=%s]", STR_ERR);
            break;
        }

        if (bind(ciServerSockFd, (struct sockaddr *)&nvrServerAddr, sizeof(nvrServerAddr)) < STATUS_OK)
        {
            EPRINT(CAMERA_INITIATION, "fail to bind to socket: [err=%s]", STR_ERR);
            break;
        }

        if (listen(ciServerSockFd, MAX_AUTO_INIT_CAM_LIST) < STATUS_OK)
        {
            EPRINT(CAMERA_INITIATION, "fail to listen: [err=%s]", STR_ERR);
            break;
        }

        if (FAIL == Utils_CreateThread(&mainThread, runCameraInitiationMain, NULL, JOINABLE_THREAD, CAM_INIT_MAIN_THREAD_STACK_SZ))
        {
            EPRINT(CAMERA_INITIATION, "fail to create cam init main thread");
            break;
        }

        DPRINT(CAMERA_INITIATION, "cam init tcp server started: [tcpPort=%d]", generalCfg.autoAddCamTcpPort);
        return SUCCESS;

    } while (0);

    /* Close socket on failure */
    CloseSocket(&ciServerSockFd);
    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is thread function.This function manages network connection. It sends and receives
 *          command from CMS/Web. It process message and send acknowledge to client. According Client
 *          it manages the session.
 * @param   arg
 * @return  SUCCESS/FAIL
 */
VOIDPTR runCameraInitiationMain(VOIDPTR arg)
{
    INT32            clientSockFd = INVALID_CONNECTION;
    SOCK_ADDR_INFO_u clientSockAddr;
    socklen_t        clientSockSize = sizeof(clientSockAddr);
    UINT8            msgId;
    CHARPTR          msgPtr;
    CHAR             cameraAddrStr[IPV6_ADDR_LEN_MAX];
    NET_CMD_STATUS_e respStatus;
    CHAR             recvMsg[MAX_RCV_SZ];
    UINT32           recvMsgLen;
    CHAR             replyMsg[MAX_REPLY_SZ];
    UINT8            camIndex;
    UINT8            cmdIndex;
    BOOL             eventStatus;
    UINT8            rdWrIndex;
    UINT64           parseValue;

    memset(&clientSockAddr, 0, sizeof(clientSockAddr));

    /* Set thread name for camera init thread */
    THREAD_START("CAM_INIT");

    /* Do not break this loop until external trigger receives */
    while (terminateFlag == FALSE)
    {
        /* Poll for read event on socket */
        if (SUCCESS != GetSocketPollEvent(ciServerSockFd, POLLRDNORM, 1000, NULL))
        {
            /* No read event available */
            continue;
        }

        /* Read event available on socket */
        clientSockFd = accept(ciServerSockFd, &clientSockAddr.sockAddr, &clientSockSize);
        if (clientSockFd == INVALID_CONNECTION)
        {
            EPRINT(CAMERA_INITIATION, "fail to accept connection: [err=%s]", STR_ERR);
            continue;
        }

        /* We need to convert socket address from ipv6 to ipv4 if camera is ipv4 */
        ConvertIpv4MappedIpv6SockAddr(&clientSockAddr);
        GetIpAddrStrFromSockAddr(&clientSockAddr, cameraAddrStr, sizeof(cameraAddrStr));

        /* Make this connection as nonblocking */
        if (FALSE == SetSockFdOption(clientSockFd))
        {
            EPRINT(CAMERA_INITIATION, "cam init fail to set socket option: [ip=%s]", cameraAddrStr);
            CloseSocket(&clientSockFd);
            continue;
        }

        if (SUCCESS != RecvMessage(clientSockFd, recvMsg, &recvMsgLen, SOM, EOM, MAX_RCV_SZ - 1, DATA_RECV_WAIT_TIME_SEC))
        {
            EPRINT(CAMERA_INITIATION, "fail to recv cam init msg: [ip=%s]", cameraAddrStr);
            CloseSocket(&clientSockFd);
            continue;
        }

        /* Terminate received string buffer with null */
        recvMsg[recvMsgLen] = '\0';

        /* Remove SOM from received data */
        msgPtr = (recvMsg + 1);

        /* Get message ID */
        if (findHeaderIndexCI(&msgPtr, &msgId) != CMD_SUCCESS)
        {
            EPRINT(CAMERA_INITIATION, "cam init header not found");
            CloseSocket(&clientSockFd);
            continue;
        }

        switch (msgId)
        {
            /* Camera login request */
            case CI_REG_LOG:
            {
                /* Assign camera */
                respStatus = assignCamera(msgPtr, clientSockFd, &camIndex, cameraAddrStr);

                /* Generate response for login request based on status */
                generateCamLoginResponse(replyMsg, respStatus, camIndex);
                DPRINT(CAMERA_INITIATION, "cam init login request: [camera=%d], [respStatus=%d], [ip=%s]", camIndex, respStatus, cameraAddrStr);

                if (FAIL == SendToSocket(clientSockFd, (UINT8PTR)replyMsg, strlen(replyMsg), MESSAGE_REPLY_TIMEOUT))
                {
                    EPRINT(CAMERA_INITIATION, "fail to send login resp: [camera=%d]", camIndex);
                    if (camIndex < getMaxCameraForCurrentVariant())
                    {
                        /* Reset all camera information as camera index is valid */
                        deInitCameraInitiation(camIndex);
                    }
                    else
                    {
                        /* Close connection as camera index is not valid */
                        CloseSocket(&clientSockFd);
                    }
                    break;
                }

                /* On success retain camera FD else close it */
                if (CMD_SUCCESS == respStatus)
                {
                    /* Now this session is active */
                    loginDataForCamInit[camIndex].isSessionIsActive = ACTIVE;
                }
                else
                {
                    /* Failure response generated */
                    CloseSocket(&clientSockFd);
                }
            }
            break;

            /* Camera event */
            case CI_RCV_EVT:
            {
                if (ParseStringGetVal(&msgPtr, &parseValue, 1, FSP) != SUCCESS)
                {
                    CloseSocket(&clientSockFd);
                    EPRINT(CAMERA_INITIATION, "fail to parse cam init event");
                    continue;
                }

                /* Get event status */
                eventStatus = (BOOL)parseValue;

                if (ParseStringGetVal(&msgPtr, &parseValue, 1, FSP) != SUCCESS)
                {
                    CloseSocket(&clientSockFd);
                    EPRINT(CAMERA_INITIATION, "fail to parse camId from cam init event");
                    continue;
                }

                /* Get camera ID */
                camIndex = (UINT8)parseValue;

                /* Validate received camera ID */
                if (camIndex >= getMaxCameraForCurrentVariant())
                {
                    CloseSocket(&clientSockFd);
                    EPRINT(CAMERA_INITIATION, "invld camId in cam init event: [camera=%d], [ip=%s]", camIndex, cameraAddrStr);
                    continue;
                }

                pthread_rwlock_wrlock(&eventCamInitInfo[camIndex].ciEvtQueueLock);
                rdWrIndex = eventCamInitInfo[camIndex].rdWrIndex;
                eventCamInitInfo[camIndex].camEventInfo[rdWrIndex].eventStatus = eventStatus;
                eventCamInitInfo[camIndex].camEventInfo[rdWrIndex].deviceId = camIndex;

                /* Remove SOI */
                msgPtr = msgPtr + 1;

                /* No need of date and time */
                CHARPTR cmdStrPtr = memchr(msgPtr, FSP, 255);
                if (NULL != cmdStrPtr)
                {
                    cmdStrPtr = cmdStrPtr + 1;
                    msgPtr = cmdStrPtr;
                }
                else
                {
                    pthread_rwlock_unlock(&eventCamInitInfo[camIndex].ciEvtQueueLock);
                    EPRINT(CAMERA_INITIATION, "fail to parse event timestamp: [camera=%d]", camIndex);
                    CloseSocket(&clientSockFd);
                    continue;
                }

                if (ParseStringGetVal(&msgPtr, &parseValue, 1, FSP) != SUCCESS)
                {
                    pthread_rwlock_unlock(&eventCamInitInfo[camIndex].ciEvtQueueLock);
                    EPRINT(CAMERA_INITIATION, "fail to parse event type: [camera=%d]", camIndex);
                    CloseSocket(&clientSockFd);
                    continue;
                }

                /* Get event type (Dummy) */
                eventCamInitInfo[camIndex].camEventInfo[rdWrIndex].eventType = (UINT8)parseValue;

                if (ParseStringGetVal(&msgPtr, &parseValue, 1, FSP) != SUCCESS)
                {
                    pthread_rwlock_unlock(&eventCamInitInfo[camIndex].ciEvtQueueLock);
                    EPRINT(CAMERA_INITIATION, "fail to parse camera event: [camera=%d]", camIndex);
                    CloseSocket(&clientSockFd);
                    continue;
                }

                /* Get Camera Event */
                eventCamInitInfo[camIndex].camEventInfo[rdWrIndex].event = (UINT8)parseValue;

                /* No need of input number */
                cmdStrPtr = memchr(msgPtr, FSP, 255);
                if (NULL != cmdStrPtr)
                {
                    cmdStrPtr = cmdStrPtr + 1;
                    msgPtr = cmdStrPtr;
                }
                else
                {
                    pthread_rwlock_unlock(&eventCamInitInfo[camIndex].ciEvtQueueLock);
                    EPRINT(CAMERA_INITIATION, "fail to parse event number: [camera=%d]", camIndex);
                    CloseSocket(&clientSockFd);
                    continue;
                }

                if (ParseStringGetVal(&msgPtr, &parseValue, 1, FSP) != SUCCESS)
                {
                    pthread_rwlock_unlock(&eventCamInitInfo[camIndex].ciEvtQueueLock);
                    EPRINT(CAMERA_INITIATION, "fail to parse event state: [camera=%d]", camIndex);
                    CloseSocket(&clientSockFd);
                    continue;
                }

                eventCamInitInfo[camIndex].camEventInfo[rdWrIndex].eventState = (BOOL)parseValue;
                DPRINT(CAMERA_INITIATION, "camera event info: [camera=%d], [cam_event=%d], [event_state=%d], [event_type=%d]", camIndex,
                       eventCamInitInfo[camIndex].camEventInfo[rdWrIndex].event, eventCamInitInfo[camIndex].camEventInfo[rdWrIndex].eventState,
                       eventCamInitInfo[camIndex].camEventInfo[rdWrIndex].eventType);

                if (ParseStr(&msgPtr, FSP, eventCamInitInfo[camIndex].camEventInfo[rdWrIndex].advanceDetail, MAX_ADVANCE_DETAIL_LEN) != SUCCESS)
                {
                    pthread_rwlock_unlock(&eventCamInitInfo[camIndex].ciEvtQueueLock);
                    EPRINT(CAMERA_INITIATION, "fail to parse event advance details: [camera=%d]", camIndex);
                    CloseSocket(&clientSockFd);
                    continue;
                }

                eventCamInitInfo[camIndex].camEventInfo[rdWrIndex].status = BUSY;
                eventCamInitInfo[camIndex].rdWrIndex++;
                eventCamInitInfo[camIndex].rdWrIndex = eventCamInitInfo[camIndex].rdWrIndex % MAX_EVT_QUEUE_SIZE;
                pthread_rwlock_unlock(&eventCamInitInfo[camIndex].ciEvtQueueLock);

                /* Close the socket after processing event */
                CloseSocket(&clientSockFd);
            }
            break;

            /* Camera ACK */
            case CI_ACK_DID:
            {
                BOOL isFdObtain = FALSE;

                if (ParseStringGetVal(&msgPtr, &parseValue, 1, FSP) != SUCCESS)
                {
                    CloseSocket(&clientSockFd);
                    EPRINT(CAMERA_INITIATION, "fail to parse camId in ack did");
                    continue;
                }

                /* Get camera ID */
                camIndex = parseValue;

                /* Validate received camera ID */
                if (camIndex >= getMaxCameraForCurrentVariant())
                {
                    CloseSocket(&clientSockFd);
                    EPRINT(CAMERA_INITIATION, "invld camera in ack did: [camera=%d], [ip=%s]", camIndex, cameraAddrStr);
                    continue;
                }

                for (cmdIndex = 0; cmdIndex < MAX_CMD_QUEUE; cmdIndex++)
                {
                    MUTEX_LOCK(commandToCamInfo[camIndex][cmdIndex].cmdDataLock);
                    if (CMD_WAIT_FOR_FD == commandToCamInfo[camIndex][cmdIndex].stateReq)
                    {
                        CloseSocket(&commandToCamInfo[camIndex][cmdIndex].commandCamFd);
                        commandToCamInfo[camIndex][cmdIndex].commandCamFd = clientSockFd;
                        commandToCamInfo[camIndex][cmdIndex].stateReq = CMD_FD_PENDING;
                        isFdObtain = TRUE;
                    }
                    MUTEX_UNLOCK(commandToCamInfo[camIndex][cmdIndex].cmdDataLock);

                    if (TRUE == isFdObtain)
                    {
                        break;
                    }
                }

                /* Close the socket */
                if (FALSE == isFdObtain)
                {
                    EPRINT(CAMERA_INITIATION, "no pending req in ack did, close connection: [camera=%d]", camIndex);
                    CloseSocket(&clientSockFd);
                }
            }
            break;

            default:
            {
                EPRINT(CAMERA_INITIATION, "cam init invld msg: [msgId=%d]", msgId);
            }
            break;
        }
    }

    CloseSocket(&clientSockFd);
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function out index of header if success otherwise error
 * @param   pStrBufPtr
 * @param   pMsgId
 * @return  Message parse success or error reason
 */
static NET_CMD_STATUS_e findHeaderIndexCI(CHARPTR *pStrBufPtr, UINT8PTR pMsgId)
{
    CHAR buf[RECV_MSG_HEADER_LENGTH];

    if (FAIL == ParseStr(pStrBufPtr, FSP, buf, RECV_MSG_HEADER_LENGTH))
    {
        return CMD_INVALID_SYNTAX;
    }

    *pMsgId = ConvertStringToIndex(buf, camHeaderReq, MAX_CI_HEADER_REQ);
    if (*pMsgId >= MAX_CI_HEADER_REQ)
    {
        return CMD_INVALID_MESSAGE;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Assign CI camera in configuration and start communication
 * @param   source
 * @param   connFd
 * @param   camAssign
 * @param   camIpAddr
 * @return  status
 * @note    This function should be only called from one thread
 */
static NET_CMD_STATUS_e assignCamera(CHARPTR source, INT32 connFd, UINT8PTR camAssign, const CHAR *camIpAddr)
{
    UINT8              freeIndex = 0;
    NET_CMD_STATUS_e   msgStatus = CMD_SUCCESS;
    CAMERA_CONFIG_t    tempCameraCfg;
    IP_CAMERA_CONFIG_t tempIpCameraCfg;
    UINT8              camListCnt = 0, status;
    GENERAL_CONFIG_t   generalCfg;
    CHAR               macAddr[MAX_MAC_ADDRESS_WIDTH];
    UINT8              index = 0;
    CHAR               eventDetail[MAX_EVENT_DETAIL_SIZE];

    *camAssign = INVALID_CAMERA_INDEX;
    snprintf(macAddr, MAX_MAC_ADDRESS_WIDTH, "%s", &source[12]);

    status = checkIfConfiguredLessMax(&freeIndex, macAddr);
    if (REFUSE == status)
    {
        msgStatus = CMD_MAX_IP_CAMERA_CONFIGURED;
    }
    else if (SUCCESS == status)
    {
        ReadSingleCameraConfig(freeIndex, &tempCameraCfg);
        if (DISABLE == tempCameraCfg.camera)
        {
            DPRINT(CAMERA_INITIATION, "assign camera previously configured but disable: [camera=%d], [mac=%s]", freeIndex, macAddr);
            msgStatus = CMD_PROCESS_ERROR;
            return (msgStatus);
        }

        DPRINT(CAMERA_INITIATION, "assign camera previously configured: [camera=%d], [mac=%s]", freeIndex, macAddr);
    }

    ReadGeneralConfig(&generalCfg);
    if (((TRUE == generalCfg.autoAddCamFlag) && (status != REFUSE)) || (SUCCESS == status))
    {
        if (INVALID_CONNECTION != loginDataForCamInit[freeIndex].cameraWiseFd)
        {
            DPRINT(CAMERA_INITIATION, "login request without previous session logout: [camera=%d],[mac=%s]", freeIndex, macAddr);
            deInitCameraInitiation(freeIndex);
        }

        memcpy(&tempIpCameraCfg, &DfltIpCameraCfg, sizeof(IP_CAMERA_CONFIG_t));
        memcpy(&loginDataForCamInit[freeIndex].loginCode, &source[0], 3);
        memcpy(&loginDataForCamInit[freeIndex].softwareVersion, &source[3], 2);
        memcpy(&loginDataForCamInit[freeIndex].softwareRevision, &source[5], 2);
        memcpy(&loginDataForCamInit[freeIndex].audioIn, &source[7], 1);
        memcpy(&loginDataForCamInit[freeIndex].audioOut, &source[8], 1);
        memcpy(&loginDataForCamInit[freeIndex].sensorIn, &source[9], 1);
        memcpy(&loginDataForCamInit[freeIndex].alarmOut, &source[10], 1);
        memcpy(&loginDataForCamInit[freeIndex].ptzSupport, &source[11], 1);
        memcpy(&loginDataForCamInit[freeIndex].cameraMacAddr[0], &source[12], 17);
        memcpy(&loginDataForCamInit[freeIndex].productName[0], &source[29], 16);
        memcpy(&loginDataForCamInit[freeIndex].cameraModelname[0], &source[45], 14);
        memcpy(&loginDataForCamInit[freeIndex].maxProfileSupport, &source[59], 1);
        memcpy(&loginDataForCamInit[freeIndex].sdSupport, &source[60], 1);
        memcpy(&loginDataForCamInit[freeIndex].osdSupport, &source[61], 1);
        memcpy(&loginDataForCamInit[freeIndex].zoomSupport, &source[62], 1);
        memcpy(&loginDataForCamInit[freeIndex].focusSupport, &source[63], 1);
        memcpy(&loginDataForCamInit[freeIndex].irisSupport, &source[64], 1);
        memcpy(&loginDataForCamInit[freeIndex].wifiSupport, &source[65], 1);

        snprintf(loginDataForCamInit[freeIndex].ipAddr, sizeof(loginDataForCamInit[freeIndex].ipAddr), "%s", camIpAddr);
        loginDataForCamInit[freeIndex].cameraWiseFd = connFd;

        ReadSingleCameraConfig(freeIndex, &tempCameraCfg);
        tempCameraCfg.camera = ENABLE;
        tempCameraCfg.type = AUTO_ADDED_CAMERA;

        if (strcasecmp(loginDataForCamInit[freeIndex].productName, MATRIX_PRODUCT_NAME) == 0)
        {
            snprintf(tempIpCameraCfg.brand, MATRIX_BRAND_NAME_LEN, MATRIX_BRAND_NAME);
        }
        else
        {
            snprintf(tempIpCameraCfg.brand, MATRIX_PRODUCT_NAME_LEN, "%s", loginDataForCamInit[freeIndex].productName);
        }

        /* Update matrix camera model name if found in old camera list */
        GetUpdatedMatrixCameraModelName(loginDataForCamInit[freeIndex].cameraModelname, MAX_MODEL_NAME_SIZE);

        snprintf(tempIpCameraCfg.model, sizeof(tempIpCameraCfg.model), "%s", loginDataForCamInit[freeIndex].cameraModelname);
        snprintf(tempIpCameraCfg.macAddr, sizeof(tempIpCameraCfg.macAddr), "%s", loginDataForCamInit[freeIndex].cameraMacAddr);
        snprintf(tempIpCameraCfg.cameraAddress, sizeof(tempIpCameraCfg.cameraAddress), "%s", loginDataForCamInit[freeIndex].ipAddr);
        if (SUCCESS != status)
        {
            snprintf(tempCameraCfg.name, MAX_CAMERA_NAME_WIDTH, CAM_INIT_DEFAULT_NAME,
                     GET_CAM_INIT_NAME_OFFSET(loginDataForCamInit[freeIndex].cameraMacAddr));
        }

        if ((WriteSingleCameraConfig(freeIndex, &tempCameraCfg) == CMD_SUCCESS) &&
            (WriteSingleIpCameraConfig(freeIndex, &tempIpCameraCfg) == CMD_SUCCESS))
        {
            DPRINT(NETWORK_MANAGER, "cam init login successful: [camera=%d], [ip=%s]", freeIndex, loginDataForCamInit[freeIndex].ipAddr);
            msgStatus = CMD_SUCCESS;
            *camAssign = freeIndex;
            GET_EVENT_CONFIG_DETAIL(eventDetail, MAX_EVENT_DETAIL_SIZE, TBL_CAMERA_CFG);
            WriteEvent(LOG_USER_EVENT, LOG_CONFIG_CHANGE, eventDetail,
                       (TRUE == generalCfg.autoAddCamFlag) ? "Auto Camera Initiation" : "Manual Camera Initiation", EVENT_CHANGE);
        }

        MUTEX_LOCK(autoInitCamList.camListLock);
        if ((autoInitCamList.writeListIndex >= 0) && (TRUE == generalCfg.autoAddCamFlag))
        {
            clearCamInitList();
        }
        MUTEX_UNLOCK(autoInitCamList.camListLock);
    }
    else if (FALSE == generalCfg.autoAddCamFlag)
    {
        if (status != REFUSE)
        {
            msgStatus = CMD_REQUEST_IN_PROGRESS;
        }

        /* Check camera already available in list or not */
        MUTEX_LOCK(autoInitCamList.camListLock);
        for (index = 0; index < MAX_AUTO_INIT_CAM_LIST; index++)
        {
            if (STATUS_OK == strcmp(macAddr, autoInitCamList.listInfo[index].cameraMacAddr))
            {
                /* Camera available in database */
                MUTEX_UNLOCK(autoInitCamList.camListLock);
                return msgStatus;
            }

            if (STATUS_OK == strcmp(autoInitCamList.listInfo[index].cameraMacAddr, ""))
            {
                break;
            }
        }

        do
        {
            if (strcmp(autoInitCamList.listInfo[autoInitCamList.writeListIndex].cameraMacAddr, "") == STATUS_OK)
            {
                break;
            }

            camListCnt++;
            autoInitCamList.writeListIndex++;
            if (autoInitCamList.writeListIndex >= MAX_AUTO_INIT_CAM_LIST)
            {
                autoInitCamList.writeListIndex = 0;
            }

        } while (MAX_AUTO_INIT_CAM_LIST != camListCnt);

        memcpy(&autoInitCamList.listInfo[autoInitCamList.writeListIndex].cameraMacAddr, &source[12], 17);
        memcpy(&autoInitCamList.listInfo[autoInitCamList.writeListIndex].productName, &source[29], 16);
        memcpy(&autoInitCamList.listInfo[autoInitCamList.writeListIndex].cameraModelname, &source[45], 14);

        /* Update matrix camera model name if found in old camera list */
        GetUpdatedMatrixCameraModelName(autoInitCamList.listInfo[autoInitCamList.writeListIndex].cameraModelname, MAX_MODEL_NAME_SIZE);
        snprintf(autoInitCamList.listInfo[autoInitCamList.writeListIndex].camIpAddr,
                 sizeof(autoInitCamList.listInfo[autoInitCamList.writeListIndex].camIpAddr), "%s", camIpAddr);
        MUTEX_UNLOCK(autoInitCamList.camListLock);
    }

    return msgStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   receiveEventFromCamInit
 * @param   arg
 * @return
 */
static VOIDPTR receiveEventFromCamInit(VOIDPTR arg)
{
    UINT8                 camIndex, eventQueueIndex;
    CAMERA_EVENT_CONFIG_t cameraEventCfg;
    CAMERA_EVENT_e        eventNo;
    struct tm             brokenTime;
    ACTION_BIT_u          actionBitField;

    THREAD_START("CAM_INIT_EVENT");

    while (terminateFlag == FALSE)
    {
        for (camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
        {
            pthread_rwlock_rdlock(&eventCamInitInfo[camIndex].ciEvtQueueLock);
            for (eventQueueIndex = 0; eventQueueIndex < MAX_EVT_QUEUE_SIZE; eventQueueIndex++)
            {
                if (FREE == eventCamInitInfo[camIndex].camEventInfo[eventQueueIndex].status)
                {
                    continue;
                }

                eventNo = ConvertCamEvtToDeviceEvt(eventCamInitInfo[camIndex].camEventInfo[eventQueueIndex].event);
                if (eventNo >= MAX_CAMERA_EVENT)
                {
                    eventCamInitInfo[camIndex].camEventInfo[eventQueueIndex].status = FREE;
                    continue;
                }

                memset(&cameraEventCfg, 0, (sizeof(CAMERA_EVENT_CONFIG_t)));
                ReadSingleCameraEventConfig(camIndex, eventNo, &cameraEventCfg);
                if (cameraEventCfg.action == DISABLE)
                {
                    eventCamInitInfo[camIndex].camEventInfo[eventQueueIndex].status = FREE;
                    continue;
                }

                if ((GetLocalTimeInBrokenTm(&brokenTime) == SUCCESS) &&
                    (CheckEventTimeWindow(&cameraEventCfg.weeklySchedule[brokenTime.tm_wday], &actionBitField, &brokenTime) == SUCCESS))
                {
                    EventDetectFunc(camIndex, eventNo, eventCamInitInfo[camIndex].camEventInfo[eventQueueIndex].eventState);
                }
                else if (eventCamInitInfo[camIndex].camEventInfo[eventQueueIndex].eventState == INACTIVE)
                {
                    EventDetectFunc(camIndex, eventNo, eventCamInitInfo[camIndex].camEventInfo[eventQueueIndex].eventState);
                }

                eventCamInitInfo[camIndex].camEventInfo[eventQueueIndex].status = FREE;
            }

            eventCamInitInfo[camIndex].rdWrIndex = 0;
            pthread_rwlock_unlock(&eventCamInitInfo[camIndex].ciEvtQueueLock);
        }

        usleep(40000);
    }

    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   ConvertCamEvtToDeviceEvt
 * @param   ciCamEvent
 * @return
 */
static CAMERA_EVENT_e ConvertCamEvtToDeviceEvt(CI_CAMERA_EVENT_e ciCamEvent)
{
    switch (ciCamEvent)
    {
        case CI_MOTION_DETECTION:
            return MOTION_DETECT;

        case CI_NO_MOTION_DETECTION:
            return NO_MOTION_DETECTION;

        case CI_VIEW_TAMPER:
            return VIEW_TEMPERING;

        case CI_TRIP_WIRE:
            return LINE_CROSS;

        case CI_ZONE_INTRUSION:
            return OBJECT_INTRUSION;

        case CI_MISSING_OBJ:
            return MISSING_OBJECT;

        case CI_SUSPICIOUS_OBJ:
            return SUSPICIOUS_OBJECT;

        case CI_ALARM_IN_1:
            return CAMERA_SENSOR_1;

        case CI_ALARM_IN_2:
            return CAMERA_SENSOR_2;

        case CI_AUDIO_EXCEPTION:
            return AUDIO_EXCEPTION;

        case CI_LOITERING_DETECTION:
            return LOITERING;

        case CI_OBJECT_COUNTING_IN:
        case CI_OBJECT_COUNTING_OUT:
            return OBJECT_COUNTING;

        default:
            return MAX_CAMERA_EVENT;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   pollForCameraInit
 * @param   arg
 * @return
 */
static VOIDPTR pollForCameraInit(VOIDPTR arg)
{
    UINT8             camIndex, msgId;
    CHAR              rcvMsg[CI_RECV_MSG_HEADER_LEN];
    UINT32            rcvMsgLen;
    CHARPTR           msgPtr;
    CHAR              replyMsg[40];
    UINT8             totalCamera = getMaxCameraForCurrentVariant();
    UINT8             cmdIndex;
    BOOL              retVal;
    UINT8             recvFailCount[MAX_CAMERA];
    BOOL              startCameraPollF[MAX_CAMERA];
    UINT64            failTimeStamp[MAX_CAMERA];
    UINT64            currentTimeStamp;
    UINT64            pollIntervalInMs;
    UINT64            pollDurationInMs;
    UINT64            lastSendTimeStamp[MAX_CAMERA];
    INT32             pollSts;
    UINT8             fdIndex, pollFdCnt;
    CI_POLL_FD_INFO_t pollFdInfo;

    THREAD_START("CAM_INIT_POLL");

    /* Initialize all the index */
    for (camIndex = 0; camIndex < MAX_CAMERA; camIndex++)
    {
        startCameraPollF[camIndex] = TRUE;
        recvFailCount[camIndex] = 0;
        failTimeStamp[camIndex] = 0;
        lastSendTimeStamp[camIndex] = 0;
        pollFdInfo.camIndex[camIndex] = INVALID_CAMERA_INDEX;
        INIT_POLL_FD(pollFdInfo.pollFds[camIndex]);
    }

    while (terminateFlag == FALSE)
    {
        /* Get current timestamp before polling */
        currentTimeStamp = GetMonotonicTimeInMilliSec();
        pollIntervalInMs = ((UINT64)ciPollInterval * MILLI_SEC_PER_SEC);
        pollDurationInMs = ((UINT64)ciPollDuration * MILLI_SEC_PER_SEC);

        /* Prepare polling fd list */
        pollFdCnt = 0;
        for (camIndex = 0; camIndex < totalCamera; camIndex++)
        {
            /* Skip invalid camera fd */
            if (INVALID_CONNECTION == loginDataForCamInit[camIndex].cameraWiseFd)
            {
                continue;
            }

            /* If start camera poll is false then check next poll time */
            if (startCameraPollF[camIndex] == FALSE)
            {
                /* Check next polling time */
                if ((currentTimeStamp - lastSendTimeStamp[camIndex]) < pollIntervalInMs)
                {
                    continue;
                }

                /* Now we can start next poll */
                startCameraPollF[camIndex] = TRUE;
            }

            /* Add camera fd in polling list */
            pollFdInfo.camIndex[pollFdCnt] = camIndex;
            pollFdInfo.pollFds[pollFdCnt].fd = loginDataForCamInit[camIndex].cameraWiseFd;
            pollFdInfo.pollFds[pollFdCnt].events = POLLRDNORM | POLLRDHUP;
            pollFdInfo.pollFds[pollFdCnt].revents = 0;
            pollFdCnt++;
        }

        /* No camera added in polling */
        if (pollFdCnt == 0)
        {
            sleep(NO_CAM_POLL_SLEEP_MAX_SEC);
            continue;
        }

        /* Start camera fd polling with 1 second timeout */
        pollSts = poll(pollFdInfo.pollFds, pollFdCnt, 1000);
        if (pollSts < STATUS_OK)
        {
            /* Error found on fd */
            usleep(INIT_POLL_SLEEP_MAX_US);
            continue;
        }

        /* Get current timestamp after polling */
        currentTimeStamp = GetMonotonicTimeInMilliSec();
        for (fdIndex = 0; fdIndex < pollFdCnt; fdIndex++)
        {
            /* Check camera fd is valid or not. It will be invalid when changed from outside */
            camIndex = pollFdInfo.camIndex[fdIndex];
            if (INVALID_CONNECTION == loginDataForCamInit[camIndex].cameraWiseFd)
            {
                continue;
            }

            /* Assume that error found on receive or poll timeout occurred */
            retVal = FAIL;

            /* Is event found on any fds? */
            if (pollSts > STATUS_OK)
            {
                /* Is it poll read event? */
                if (pollFdInfo.pollFds[fdIndex].revents & POLLRDNORM)
                {
                    retVal = RecvMessage(loginDataForCamInit[camIndex].cameraWiseFd, rcvMsg, &rcvMsgLen, SOM, EOM, sizeof(rcvMsg), 0);
                }
                /* Is connection closed remotely? */
                else if (pollFdInfo.pollFds[fdIndex].revents & POLLRDHUP)
                {
                    retVal = REFUSE;
                }
            }

            /* Error found on receive or poll timeout occurred */
            if (retVal == FAIL)
            {
                /* Update fail timestamp */
                startCameraPollF[camIndex] = TRUE;
                if (failTimeStamp[camIndex] == 0)
                {
                    failTimeStamp[camIndex] = GetMonotonicTimeInMilliSec();
                }

                /* Is retry time exited with poll duration? */
                if ((currentTimeStamp - failTimeStamp[camIndex]) < pollDurationInMs)
                {
                    continue;
                }

                /* Mark this retry as failed */
                startCameraPollF[camIndex] = FALSE;
                failTimeStamp[camIndex] = 0;
                lastSendTimeStamp[camIndex] = currentTimeStamp;

                /* Increase the fail count */
                recvFailCount[camIndex]++;
                if (recvFailCount[camIndex] >= INIT_POLL_RETRY_MAX)
                {
                    EPRINT(CAMERA_INITIATION, "cam init maximum recv fail count reached: [camera=%d]", camIndex);
                    deInitCameraInitiation(camIndex);
                    recvFailCount[camIndex] = 0;
                    lastSendTimeStamp[camIndex] = 0;
                }

                /* Start poll process again */
                continue;
            }

            /* Connection refused */
            if (retVal == REFUSE)
            {
                EPRINT(CAMERA_INITIATION, "cam init connection closed: [camera=%d]", camIndex);
                deInitCameraInitiation(camIndex);
                startCameraPollF[camIndex] = TRUE;
                recvFailCount[camIndex] = 0;
                failTimeStamp[camIndex] = 0;
                lastSendTimeStamp[camIndex] = 0;
                continue;
            }

            /* Polling data received from camera */
            recvFailCount[camIndex] = 0;
            failTimeStamp[camIndex] = 0;
            lastSendTimeStamp[camIndex] = currentTimeStamp;
            startCameraPollF[camIndex] = FALSE;

            /* Find the message id */
            msgPtr = rcvMsg + 1;
            if (CMD_SUCCESS != findHeaderIndexCI(&msgPtr, &msgId))
            {
                EPRINT(CAMERA_INITIATION, "cam init header not found: [camera=%d]", camIndex);
                continue;
            }

            /* Is it camera poll request? */
            if (msgId != CI_REQ_POLL)
            {
                EPRINT(CAMERA_INITIATION, "cam init invld msg: [camera=%d], [msgId=%d]", camIndex, msgId);
                continue;
            }

            /* Provide the response with required connection count */
            UINT8 reqPort = 0;
            for (cmdIndex = 0; cmdIndex < MAX_CMD_QUEUE; cmdIndex++)
            {
                MUTEX_LOCK(commandToCamInfo[camIndex][cmdIndex].cmdDataLock);
                if (CMD_GET_FD == commandToCamInfo[camIndex][cmdIndex].stateReq)
                {
                    commandToCamInfo[camIndex][cmdIndex].stateReq = CMD_WAIT_FOR_FD;
                    reqPort++;
                }
                MUTEX_UNLOCK(commandToCamInfo[camIndex][cmdIndex].cmdDataLock);
            }

            /* Send the poll response to camera */
            snprintf(replyMsg, sizeof(replyMsg), "%c%s%c%02d%c%c", SOM, camHeaderReq[CI_ACK_POL], FSP, reqPort, FSP, EOM);
            if (FAIL == SendToSocket(loginDataForCamInit[camIndex].cameraWiseFd, (UINT8PTR)replyMsg, strlen(replyMsg), MESSAGE_REPLY_TIMEOUT))
            {
                EPRINT(CAMERA_INITIATION, "cam init fail to send poll resp: [camera=%d]", camIndex);
            }
        }

        /* Provide sleep after iteration */
        usleep(INIT_POLL_SLEEP_MAX_US);
    }

    /* Exit from thread */
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   generateCamLoginResponse
 * @param   replyMsg
 * @param   cmdResponse
 * @param   deviceId
 */
static void generateCamLoginResponse(CHARPTR replyMsg, NET_CMD_STATUS_e cmdResponse, UINT32 deviceId)
{
    if (CMD_SUCCESS == cmdResponse)
    {
        // Send login message to client
        snprintf(replyMsg, MAX_REPLY_SZ,
                 "%c"   // SOM
                 "%s"   // ACK_LOG
                 "%c"   // FSP
                 "%d"   // login resp
                 "%c"   // FSP
                 "%d"   // deviceId
                 "%c"   // FSP
                 "%d"   // pollDuration
                 "%c"   // FSP
                 "%d"   // pollInterval
                 "%c"   // FSP
                 "%c",  // EOM
                 SOM, camHeaderReq[CI_ACK_LOG], FSP, cmdResponse, FSP, deviceId, FSP, ciPollDuration, FSP, ciPollInterval, FSP, EOM);
    }
    else
    {
        // Send login message to client
        snprintf(replyMsg, MAX_REPLY_SZ,
                 "%c"    // SOM
                 "%s"    // ACK_LOG
                 "%c"    // FSP
                 "%03d"  // login resp
                 "%c"    // FSP
                 "%c",   // EOM
                 SOM, camHeaderReq[CI_ACK_LOG], FSP, cmdResponse, FSP, EOM);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief deInitCameraInitiation
 * @param cameraIndex
 */
static void deInitCameraInitiation(UINT8 cameraIndex)
{
    DPRINT(CAMERA_INITIATION, "cam init deInitialize: [camera=%d]", cameraIndex);
    CloseSocket(&loginDataForCamInit[cameraIndex].cameraWiseFd);
    loginDataForCamInit[cameraIndex].isSessionIsActive = INACTIVE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   checkIfConfiguredLessMax
 * @param   firstFreeIndex
 * @param   source
 * @return
 */
static UINT8 checkIfConfiguredLessMax(UINT8PTR firstFreeIndex, CHARPTR source)
{
    UINT8              camIndex = 0;
    BOOL               isConfigLessMax = FAIL;
    IP_CAMERA_CONFIG_t ipCameraCfg[MAX_CAMERA];
    UINT8              alreadyConfiguredCnt = 0;

    *firstFreeIndex = MAX_CAMERA;
    ReadIpCameraConfig(ipCameraCfg);

    for (camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
    {
        if (STATUS_OK == strncmp(ipCameraCfg[camIndex].macAddr, source, MAX_MAC_ADDRESS_WIDTH - 1))
        {
            DPRINT(CAMERA_INITIATION, "cam init camera configured: [camera=%d], [mac=%s]", camIndex, source);
            *firstFreeIndex = camIndex;
            isConfigLessMax = SUCCESS;
            break;
        }

        if (ipCameraCfg[camIndex].cameraAddress[0] != '\0')
        {
            alreadyConfiguredCnt++;
            continue;
        }

        if (*firstFreeIndex == MAX_CAMERA)
        {
            *firstFreeIndex = camIndex;
        }
    }

    if (alreadyConfiguredCnt >= getMaxCameraForCurrentVariant())
    {
        isConfigLessMax = REFUSE;
    }

    return isConfigLessMax;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   GetCamInitList
 * @param   searchResult
 * @param   resultCnt
 * @return
 */
NET_CMD_STATUS_e GetCamInitList(AUTO_INIT_CAM_LIST_INFO *searchResult, UINT8 *resultCnt)
{
    MUTEX_LOCK(autoInitCamList.camListLock);
    autoInitCamList.readListIndex = 0;

    do
    {
        if (STATUS_OK == strcmp(autoInitCamList.listInfo[autoInitCamList.readListIndex].cameraMacAddr, ""))
        {
            break;
        }

        *(searchResult + autoInitCamList.readListIndex) = autoInitCamList.listInfo[autoInitCamList.readListIndex];
        ++autoInitCamList.readListIndex;

    } while (MAX_AUTO_INIT_CAM_LIST != autoInitCamList.readListIndex);

    *resultCnt = autoInitCamList.readListIndex;
    MUTEX_UNLOCK(autoInitCamList.camListLock);
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief AddCmdCamCount
 * @param camIndex
 * @param handle
 * @return
 */
BOOL AddCmdCamCount(UINT8 camIndex, UINT8PTR handle)
{
    UINT8 cmdIndex;

    if (camIndex >= getMaxCameraForCurrentVariant())
    {
        EPRINT(CAMERA_INITIATION, "invld camId: [camera=%d]", camIndex);
        *handle = MAX_CMD_QUEUE;
        return FAIL;
    }

    for (cmdIndex = 0; cmdIndex < MAX_CMD_QUEUE; cmdIndex++)
    {
        MUTEX_LOCK(commandToCamInfo[camIndex][cmdIndex].cmdDataLock);
        if (CMD_FD_NONE == commandToCamInfo[camIndex][cmdIndex].stateReq)
        {
            commandToCamInfo[camIndex][cmdIndex].stateReq = CMD_GET_FD;
            *handle = cmdIndex;
            MUTEX_UNLOCK(commandToCamInfo[camIndex][cmdIndex].cmdDataLock);
            DPRINT(CAMERA_INITIATION, "ci camera fd handle: [camera=%d], [handle=%d]", camIndex, *handle);
            return SUCCESS;
        }
        MUTEX_UNLOCK(commandToCamInfo[camIndex][cmdIndex].cmdDataLock);
    }

    *handle = MAX_CMD_QUEUE;
    EPRINT(CAMERA_INITIATION, "fail to get ci camera fd handle: [camera=%d]", camIndex);
    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   GetCmdFd
 * @param   camIndex
 * @param   pCmdFd
 * @param   pHandle
 * @param   isTimeOut
 * @return
 */
BOOL GetCmdFd(UINT8 camIndex, INT32 *pCmdFd, UINT8 *pHandle, BOOL isTimeOut)
{
    if (*pHandle >= MAX_CMD_QUEUE)
    {
        EPRINT(CAMERA_INITIATION, "cam init invld cmd: [camera=%d], [handle=%d]", camIndex, *pHandle);
        return FAIL;
    }

    if (camIndex >= getMaxCameraForCurrentVariant())
    {
        EPRINT(CAMERA_INITIATION, "cam init invld camera: [camera=%d], [handle=%d]", camIndex, *pHandle);
        return FAIL;
    }

    if (TRUE == isTimeOut)
    {
        MUTEX_LOCK(commandToCamInfo[camIndex][*pHandle].cmdDataLock);
        commandToCamInfo[camIndex][*pHandle].stateReq = CMD_FD_NONE;
        MUTEX_UNLOCK(commandToCamInfo[camIndex][*pHandle].cmdDataLock);
        *pCmdFd = INVALID_CONNECTION;
        return FAIL;
    }

    MUTEX_LOCK(commandToCamInfo[camIndex][*pHandle].cmdDataLock);
    if (CMD_FD_PENDING == commandToCamInfo[camIndex][*pHandle].stateReq)
    {
        *pCmdFd = commandToCamInfo[camIndex][*pHandle].commandCamFd;
        commandToCamInfo[camIndex][*pHandle].stateReq = CMD_FD_BUSY;
        MUTEX_UNLOCK(commandToCamInfo[camIndex][*pHandle].cmdDataLock);
        return SUCCESS;
    }
    MUTEX_UNLOCK(commandToCamInfo[camIndex][*pHandle].cmdDataLock);

    *pCmdFd = INVALID_CONNECTION;
    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Close the camera initiation request FD
 * @param   camIndex
 * @param   handle
 * @param   isFdNeeded (when fd needed by application but request index don't want to occupy)
 *          Now it is application's responsibility to close this FD
 */
void CloseCamCmdFd(UINT8 camIndex, UINT8 handle, BOOL isFdNeeded)
{
    if (handle >= MAX_CMD_QUEUE)
    {
        EPRINT(CAMERA_INITIATION, "cam init invld cmd: [camera=%d], [handle=%d]", camIndex, handle);
        return;
    }

    if (camIndex >= getMaxCameraForCurrentVariant())
    {
        EPRINT(CAMERA_INITIATION, "cam init invld camera: [camera=%d], [handle=%d]", camIndex, handle);
        return;
    }

    MUTEX_LOCK(commandToCamInfo[camIndex][handle].cmdDataLock);
    if (CMD_FD_BUSY == commandToCamInfo[camIndex][handle].stateReq)
    {
        if (isFdNeeded == FALSE)
        {
            CloseSocket(&commandToCamInfo[camIndex][handle].commandCamFd);
        }
        else
        {
            commandToCamInfo[camIndex][handle].commandCamFd = INVALID_CONNECTION;
        }
        commandToCamInfo[camIndex][handle].stateReq = CMD_FD_NONE;
    }
    MUTEX_UNLOCK(commandToCamInfo[camIndex][handle].cmdDataLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API Rearrange list on Add and Reject Camera.
 * @param   macAddr
 */
static void arrangeCamInitList(CHARPTR macAddr)
{
    UINT8 index = 0, rmvIndex = 0;
    BOOL  status = FALSE;

    for (index = 0; index < MAX_AUTO_INIT_CAM_LIST; index++)
    {
        if (STATUS_OK == strcmp(macAddr, autoInitCamList.listInfo[index].cameraMacAddr))
        {
            status = TRUE;
            rmvIndex = index;
            break;
        }
        else if (STATUS_OK == strcmp(autoInitCamList.listInfo[index].cameraMacAddr, ""))
        {
            break;
        }
    }

    if (TRUE == status)
    {
        for (index = rmvIndex; index < (MAX_AUTO_INIT_CAM_LIST - 1); index++)
        {
            if (STATUS_OK == strcmp(autoInitCamList.listInfo[index + 1].cameraMacAddr, ""))
            {
                break;
            }

            memcpy(&autoInitCamList.listInfo[index], &autoInitCamList.listInfo[index + 1], sizeof(AUTO_INIT_CAM_LIST_INFO));
        }

        RESET_STR_BUFF(autoInitCamList.listInfo[index].cameraMacAddr);
        RESET_STR_BUFF(autoInitCamList.listInfo[index].productName);
        RESET_STR_BUFF(autoInitCamList.listInfo[index].cameraModelname);
        RESET_STR_BUFF(autoInitCamList.listInfo[index].camIpAddr);

        /* Decrement write index if it is non zero */
        if (autoInitCamList.writeListIndex)
        {
            autoInitCamList.writeListIndex--;
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   clearCamInitList
 */
static void clearCamInitList(void)
{
    UINT8 index;

    autoInitCamList.readListIndex = 0;
    autoInitCamList.writeListIndex = 0;

    for (index = 0; index < MAX_AUTO_INIT_CAM_LIST; index++)
    {
        RESET_STR_BUFF(autoInitCamList.listInfo[index].cameraMacAddr);
        RESET_STR_BUFF(autoInitCamList.listInfo[index].productName);
        RESET_STR_BUFF(autoInitCamList.listInfo[index].cameraModelname);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   AddInitiatedCam
 * @param   cameraList
 * @param   cameraCnt
 * @return
 */
NET_CMD_STATUS_e AddInitiatedCam(AUTO_INIT_CAM_LIST_INFO *cameraList, UINT8 cameraCnt)
{
    CAMERA_CONFIG_t    tempCameraCfg;
    IP_CAMERA_CONFIG_t tempIpCameraCfg;
    UINT8              index, freeIndex = 0, addCamIndex;
    BOOL               status = FALSE;

    MUTEX_LOCK(autoInitCamList.camListLock);
    for (addCamIndex = 0; addCamIndex < cameraCnt; addCamIndex++)
    {
        for (index = 0; index < MAX_AUTO_INIT_CAM_LIST; index++)
        {
            if (autoInitCamList.listInfo[index].cameraMacAddr[0] == '\0')
            {
                break;
            }

            if (STATUS_OK == strcmp(cameraList[addCamIndex].cameraMacAddr, autoInitCamList.listInfo[index].cameraMacAddr))
            {
                break;
            }
        }

        if (index >= MAX_AUTO_INIT_CAM_LIST)
        {
            DPRINT(NETWORK_MANAGER, "camera not found: [camera=%d], [mac=%s]", addCamIndex, cameraList[addCamIndex].cameraMacAddr);
            continue;
        }

        status = checkIfConfiguredLessMax(&freeIndex, autoInitCamList.listInfo[index].cameraMacAddr);
        if (REFUSE == status)
        {
            MUTEX_UNLOCK(autoInitCamList.camListLock);
            return CMD_MAX_IP_CAMERA_CONFIGURED;
        }

        ReadSingleIpCameraConfig(freeIndex, &tempIpCameraCfg);
        ReadSingleCameraConfig(freeIndex, &tempCameraCfg);

        tempCameraCfg.camera = ENABLE;
        tempCameraCfg.type = AUTO_ADDED_CAMERA;

        tempIpCameraCfg.onvifSupportF = FALSE;
        snprintf(tempIpCameraCfg.macAddr, sizeof(tempIpCameraCfg.macAddr), "%s", autoInitCamList.listInfo[index].cameraMacAddr);
        snprintf(tempIpCameraCfg.cameraAddress, sizeof(autoInitCamList.listInfo[index].camIpAddr), "%s", autoInitCamList.listInfo[index].camIpAddr);

        if (SUCCESS != status)
        {
            snprintf(tempCameraCfg.name, MAX_CAMERA_NAME_WIDTH, CAM_INIT_DEFAULT_NAME,
                     GET_CAM_INIT_NAME_OFFSET(autoInitCamList.listInfo[index].cameraMacAddr));
        }

        if ((WriteSingleCameraConfig(freeIndex, &tempCameraCfg) == CMD_SUCCESS) &&
            (WriteSingleIpCameraConfig(freeIndex, &tempIpCameraCfg) == CMD_SUCCESS))
        {
            arrangeCamInitList(autoInitCamList.listInfo[index].cameraMacAddr);
        }
    }

    MUTEX_UNLOCK(autoInitCamList.camListLock);
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   RejectInitiatedCam
 * @param   cameraList
 * @param   cameraCnt
 * @return
 */
NET_CMD_STATUS_e RejectInitiatedCam(AUTO_INIT_CAM_LIST_INFO *cameraList, UINT8 cameraCnt)
{
    UINT8 camIndex;

    MUTEX_LOCK(autoInitCamList.camListLock);
    for (camIndex = 0; camIndex < cameraCnt; camIndex++)
    {
        arrangeCamInitList(cameraList[camIndex].cameraMacAddr);
    }
    MUTEX_UNLOCK(autoInitCamList.camListLock);
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   GetCameraConnectionStatus
 * @param   camIndex
 * @return
 */
UINT8 GetCameraConnectionStatus(UINT8 camIndex)
{
    return loginDataForCamInit[camIndex].isSessionIsActive;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   GetCameraBrandModelString
 * @param   camIndex
 * @param   brand
 * @param   model
 */
void GetCameraBrandModelString(UINT8 camIndex, CHARPTR *brand, CHARPTR *model)
{
    if (strcasecmp(loginDataForCamInit[camIndex].productName, MATRIX_PRODUCT_NAME) == 0)
    {
        *brand = MATRIX_BRAND_NAME;
    }
    else
    {
        *brand = loginDataForCamInit[camIndex].productName;
    }

    *model = loginDataForCamInit[camIndex].cameraModelname;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get camera initiation polling duration
 * @return  Polling duration
 */
UINT8 GetCiPollDuration(void)
{
    return ciPollDuration;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get camera initiation polling time (Duration + Interval) in seconds
 * @return  Polling time in seconds
 */
UINT8 GetCiPollTimeInSec(void)
{
    return (ciPollDuration + ciPollInterval);
}

// #################################################################################################
//  @END OF FILE
// #################################################################################################
