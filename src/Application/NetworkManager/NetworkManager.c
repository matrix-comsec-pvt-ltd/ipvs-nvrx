//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		NetworkManager.c
@brief      This module provides network related functionality. Client can set/get NVR configuration
            data. Client can get runtime event from NVR and send command against event. Network
            manager control Media stream which is from IP camera. This module is entry point command
            and stream control.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Library Includes */
#include <jansson.h>

/* Application Includes */
#include "NetworkManager.h"
#include "Utils.h"
#include "DebugLog.h"
#include "NetworkConfig.h"
#include "NetworkCommand.h"
#include "NetworkFileTransfer.h"
#include "NetworkController.h"
#include "NetworkInterface.h"
#include "MobileBroadBand.h"
#include "DiskController.h"
#include "MxPcap.h"
#include "DoorCommand.h"
#include "DeviceInitiation.h"
#include "RtspClientInterface.h"
#include "DhcpServer.h"
#include "PasswordResetCmd.h"
#include "DdnsClient.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#if defined(OEM_JCI)
/* sha256 of log page password: "nimda123" */
#define LOG_WEB_PAGE_PASSWORD           "c97b08d52fda3acfc8836ad416bb4cc29ca319cfd248fcb9f1a68505af4bcac3"
#else
/* sha256 of log page password: "xirtam123" */
#define LOG_WEB_PAGE_PASSWORD           "c0d813a6137408554966cc851511c3e2c01dead754dc02bbbbeb25b9f53a4046"
#endif

#define WS_JSON_RESP_MSG                "{\"status\": %d}"
#define UI_UNIX_SOCKET_FILE_PATH        "/tmp/IntUiSocket"

#define MAX_CONCURRANT_REQ              (MAX_NW_CLIENT * 2)
#define MAX_SESSION_ID_LEN              8
#define	RECV_MSG_HEADER_LENGTH          30
#define MAX_INTERAL_SND_TIMEOUT         10
#define MAX_FRAME_WAIT_TIME             1		// In second
#define MAX_TX_SZ                       1024

#define DHCP_SERVER_INFO_FIELD_MAX      6
#define DDNS_CLIENT_INFO_FIELD_MAX      4
#define DEBUG_INFO_FIELD_MAX            (6 + MAX_SERVER_LEVELS + (MAX_GUI_LEVELS - GROUP_1))
#define	MAX_INFO_PARSE                  DEBUG_INFO_FIELD_MAX
#define	MAX_PARSE_LENGTH                256

/* Stack Size for threads */
#define INT_NW_MNGR_THREAD_STACK_SZ     (1*MEGA_BYTE)
#define EXT_NW_MNGR_THREAD_STACK_SZ     (0*MEGA_BYTE)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
    DEVICE_SUBSYSTEM_BLOCK,
    DEVICE_SUBSYSTEM_USB,
    DEVICE_SUBSYSTEM_TTY,
    DEVICE_SUBSYSTEM_NET,
    DEVICE_SUBSYSTEM_INPUT,
    DEVICE_SUBSYSTEM_MAX
}DEVICE_SUBSYSTEM_e;

typedef struct
{
    BOOL    terminateFlg;
    INT32   nvrServerSockFd;

}NETWORK_MANAGER_SERVER_PARA_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static INT32                            internalServerSockFd = INVALID_CONNECTION;
static NETWORK_MANAGER_SERVER_PARA_t    networkManagerInfo;

static const CHARPTR    deviceSubsystemStr[DEVICE_SUBSYSTEM_MAX] ={"block", "usb", "tty", "net", "input"};
static const UINT8      deviceSubsystemArgs[DEVICE_SUBSYSTEM_MAX] = {4, 14, 3, 2, 1};

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static VOIDPTR internalMsgServerLoop(VOIDPTR arg);
//-------------------------------------------------------------------------------------------------
static VOIDPTR runNetworkManagerMain(VOIDPTR arg);
//-------------------------------------------------------------------------------------------------
static BOOL initExternalNw(void);
//-------------------------------------------------------------------------------------------------
static BOOL initInternalNw(void);
//-------------------------------------------------------------------------------------------------
static void sendWebStatusRespMsg(INT32 sockFd, UINT32 status);
//-------------------------------------------------------------------------------------------------
static void sendGetSystemInfoResponse(INT32 clientSockFd);
//-------------------------------------------------------------------------------------------------
static void sendGetSystemLogsResponse(INT32 clientSockFd);
//-------------------------------------------------------------------------------------------------
static void sendSetSystemLogsResponse(INT32 clientSockFd, CHARPTR jsonMsg);
//-------------------------------------------------------------------------------------------------
static void sendGetPcapTraceStatsResponse(INT32 clientSockFd);
//-------------------------------------------------------------------------------------------------
static void sendSetPcapTraceActionResponse(INT32 clientSockFd, CHARPTR jsonMsg);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   These functions initialize network manger and start network related service.
 *          This function called once in main loop.
 * @return  SUCCESS or FAIL
 */
BOOL InitNetworkManager(void)
{
    GENERAL_CONFIG_t generalConfig;

    /* Read general configuration */
    ReadGeneralConfig(&generalConfig);

    /* Start web server */
    WebServerServiceStartStop(START, generalConfig.httpPort);

    /* Initialization of external network communication socket for local, device and mobile client */
    if (initExternalNw() == FAIL)
    {
        return FAIL;
    }

    /* Initialization of internal network communication socket for Logs, Pcap udev etc. */
    if (initInternalNw() == FAIL)
    {
        return FAIL;
    }

    /* Init device initiation */
    InitDeviceInitiation();
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is callback function which send data to client with response.
 * @param   status
 * @param   connFd
 * @param   freeSize
 * @param   recordSize
 * @param   closeConn
 * @param   callbackType
 */
void  NwCallbackFuncForBkupRcd(NET_CMD_STATUS_e status, INT32 connFd, float freeSize, float recordSize, BOOL closeConn, CLIENT_CB_TYPE_e callbackType)
{
    UINT16  outLen;
    CHAR    replyMsg[256];

    outLen = snprintf(replyMsg, sizeof(replyMsg), "%c%s%c%02d%c" "%c%d%c%07.2f%c%07.2f%c%c%c",
                      SOM, headerReq[RPL_CMD], FSP, status, FSP, SOI, 1, FSP, recordSize, FSP, freeSize, FSP, EOI, EOM);
    if(outLen > sizeof(replyMsg))
    {
        EPRINT(NETWORK_MANAGER, "more buffer size required for msg: [length=%d], [fd=%d]", outLen, connFd);
        outLen = sizeof(replyMsg);
    }

    sendCmdCb[callbackType](connFd, (UINT8PTR)replyMsg, outLen, MESSAGE_REPLY_TIMEOUT);

    if (closeConn == TRUE)
    {
        closeConnCb[callbackType](&connFd);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function out index of header if success otherwise error
 * @param   pMsgBuff - buf pointer
 * @param   pMsgId - index of header
 * @return  Message parse success or error reason
 */
NET_CMD_STATUS_e FindHeaderIndex(CHARPTR *pMsgBuff, UINT8 *pMsgId)
{
    CHAR buf[RECV_MSG_HEADER_LENGTH];

    if (FAIL == ParseStr(pMsgBuff, FSP, buf, RECV_MSG_HEADER_LENGTH))
    {
        return CMD_INVALID_SYNTAX;
    }

    *pMsgId = ConvertStringToIndex(buf, headerReq, MAX_HEADER_REQ);
    if(*pMsgId < MAX_HEADER_REQ)
    {
        return CMD_SUCCESS;
    }
    else
    {
        return CMD_INVALID_MESSAGE;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function deintiliaze all networkmanager
 */
void DeinitNwManager(void)
{
    DeInitDeviceInitiation();
    WebServerServiceStartStop(STOP, 0);
    networkManagerInfo.terminateFlg = STOP;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set log level to local client
 * @param   pDebugConfig
 */
static void setLocalClientDebugConfig(DBG_CONFIG_PARAM_t *pDebugConfig)
{
    INT32               clientFd;
    struct sockaddr_un  clientParam;

    /* Create the socket */
    clientFd = socket(AF_UNIX, TCP_SOCK_OPTIONS, 0);
    if (clientFd == INVALID_CONNECTION)
    {
        return;
    }

    /* Load with socket parameters */
    memset(&clientParam, 0, sizeof(clientParam));
    clientParam.sun_family = AF_UNIX;
    snprintf(clientParam.sun_path, sizeof(clientParam.sun_path), UI_UNIX_SOCKET_FILE_PATH);

    /* Connecting to the server */
    if (connect(clientFd, (struct sockaddr*)&clientParam, sizeof(clientParam)) < STATUS_OK)
    {
        close(clientFd);
        return;
    }

    /* Send request to network manager */
    if (send(clientFd, pDebugConfig, sizeof(DBG_CONFIG_PARAM_t), 0) < (INT32)sizeof(DBG_CONFIG_PARAM_t))
    {
        close(clientFd);
        return;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is server unix socket handler function which receive message from  internal loopback.
 *          Send response to internal loopback.
 * @param   arg
 */
static VOIDPTR internalMsgServerLoop(VOIDPTR arg)
{
    INT32   clientSockFd = INVALID_CONNECTION;
    UINT8   devDetIdx;
    UINT16  loop;
    UINT8   msgId;
    CHARPTR msgPtr;
    CHAR    subSys[16];
    CHAR    parseStr[MAX_INFO_PARSE][MAX_PARSE_LENGTH];
    CHAR    recvMsg[MAX_RCV_SZ];
    UINT32  msgLen;
    CHAR    replyMsg[MAX_TX_SZ];

    THREAD_START("INT_NW");

    memset(parseStr, 0, sizeof(parseStr));
    while(TRUE)
    {
        // Wait for client to connected
        clientSockFd = accept(internalServerSockFd, NULL, NULL);
        if (clientSockFd < STATUS_OK)
        {
            //Message failed to try again
            EPRINT(NETWORK_MANAGER, "invld fd in connection accept: [err-%s]", STR_ERR);
            continue;
        }

        //Make this connection as nonblocking
        if (FALSE == SetSockFdOption(clientSockFd))
        {
            EPRINT(NETWORK_MANAGER, "fail to set sock options: [fd=%d]", clientSockFd);
            CloseSocket(&clientSockFd);
            continue;
        }

        // read data from TCP socket
        if(RecvMessage(clientSockFd, recvMsg, &msgLen, SOM, EOM, MAX_RCV_SZ-1, MAX_FRAME_WAIT_TIME) != SUCCESS)
        {
            EPRINT(NETWORK_MANAGER, "internal msg recv timeout or connection closed");
            CloseSocket(&clientSockFd);
            continue;
        }

        recvMsg[msgLen] = '\0';
        msgPtr = (recvMsg + 1); // for remove SOM from received data
        if(FindHeaderIndex(&msgPtr, &msgId) != CMD_SUCCESS)
        {
            EPRINT(NETWORK_MANAGER, "fail to parse header index: [msg=%s]", msgPtr);
            CloseSocket(&clientSockFd);
            continue;
        }

        switch(msgId)
        {
            case DEV_DETECT:
            {
                if(ParseStr(&msgPtr, FSP, subSys, sizeof(subSys)) == FALSE)
                {
                    EPRINT(NETWORK_MANAGER, "fail to parse device detect index");
                    break;
                }

                devDetIdx = ConvertStringToIndex(subSys, deviceSubsystemStr, DEVICE_SUBSYSTEM_MAX);
                if (devDetIdx >= DEVICE_SUBSYSTEM_MAX)
                {
                    EPRINT(NETWORK_MANAGER, "invld device detect index found: [devDetIdx=%d]", devDetIdx);
                    break;
                }

                for(loop = 0; loop < deviceSubsystemArgs[devDetIdx]; loop++)
                {
                    if (ParseStr(&msgPtr, FSP, parseStr[loop], sizeof(parseStr[loop])) == FALSE)
                    {
                        EPRINT(NETWORK_MANAGER, "fail to parse device detect number: [loop=%d]", loop);
                        devDetIdx = DEVICE_SUBSYSTEM_MAX;
                        break;
                    }
                }

                switch(devDetIdx)
                {
                    case DEVICE_SUBSYSTEM_BLOCK:
                    {
                        UDEV_DEVICE_INFO_t udevInfo;

                        snprintf(udevInfo.action, sizeof(udevInfo.action), "%s", parseStr[0]);
                        snprintf(udevInfo.path, sizeof(udevInfo.path), "%s", parseStr[1]);
                        snprintf(udevInfo.serial, sizeof(udevInfo.serial), "%s", parseStr[2]);
                        snprintf(udevInfo.baseNode, sizeof(udevInfo.baseNode), "%s", parseStr[3]);
                        UpdateStorageMedia(&udevInfo);
                    }
                    break;

                    case DEVICE_SUBSYSTEM_USB:
                    {
                        BROADBAND_DEVICE_INFO_t broadBandDevice;

                        memset(&broadBandDevice, 0, sizeof(BROADBAND_DEVICE_INFO_t));
                        /* The sequence of attributes in the udev rules is as below,
                         * 1.subsystem 2.action 3.path 4.vendorid 5.productid 6.interfaceclass 7.interfacesubclass
                         * 8.interfaceprotocol 9.interfacenumber 10.vendor 11.model 12.rev 13.manufacturer 14.product 15.serial
                         * Whenever this sequence needs to be changed, the change should be reflected in the udev rules also */
                        snprintf(broadBandDevice.action, MAX_DEVICE_INFO_LENGTH, "%s", parseStr[0]);
                        snprintf(broadBandDevice.path, MAX_DEVICE_PATH_LENGTH, "%s", parseStr[1]);
                        snprintf(broadBandDevice.vendorId, MAX_DEVICE_INFO_LENGTH, "%s", parseStr[2]);
                        snprintf(broadBandDevice.productId, MAX_DEVICE_INFO_LENGTH, "%s", parseStr[3]);
                        snprintf(broadBandDevice.interfaceClass, MAX_DEVICE_INFO_LENGTH, "%s", parseStr[4]);
                        snprintf(broadBandDevice.interfaceSubClass, MAX_DEVICE_INFO_LENGTH, "%s", parseStr[5]);
                        snprintf(broadBandDevice.interfaceProtocol, MAX_DEVICE_INFO_LENGTH, "%s", parseStr[6]);
                        snprintf(broadBandDevice.interfaceNumber, MAX_DEVICE_INFO_LENGTH, "%s", parseStr[7]);
                        snprintf(broadBandDevice.vendor, MAX_DEVICE_INFO_LENGTH, "%s", parseStr[8]);
                        snprintf(broadBandDevice.model, MAX_DEVICE_INFO_LENGTH, "%s", parseStr[9]);
                        snprintf(broadBandDevice.rev, MAX_DEVICE_INFO_LENGTH, "%s", parseStr[10]);
                        snprintf(broadBandDevice.manufacturer, MAX_MANUFACTURER_NAME_LENGTH, "%s", parseStr[11]);
                        snprintf(broadBandDevice.product, MAX_MANUFACTURER_NAME_LENGTH, "%s", parseStr[12]);
                        snprintf(broadBandDevice.serial, MAX_DEVICE_INFO_LENGTH, "%s", parseStr[13]);

                        /* USB broadband detected */
                        DetectBroadBandDevice(&broadBandDevice);
                    }
                    break;

                    case DEVICE_SUBSYSTEM_TTY:
                    {
                        /* USB modem redetected as tty serial device: 0=action, 1=devPath, 2=ttyNode */
                        DetectTtyDevice(parseStr[0], parseStr[1], parseStr[2]);
                    }
                    break;

                    case DEVICE_SUBSYSTEM_NET:
                    {
                        /* USB modem redetected as cdc ethernet device: 0=action, 1=devNode */
                        DetectNetDevice(parseStr[0], parseStr[1]);
                    }
                    break;

                    case DEVICE_SUBSYSTEM_INPUT:
                    {
                        /* Input device status updated */
                        DPRINT(NETWORK_MANAGER, "input device info: [action=%s]", parseStr[0]);
                    }
                    break;

                    default:
                    {
                        /* Nothing to do */
                    }
                    break;
                }
            }
            break;

            case CGI_REQ_GETPORT:
            {
                GENERAL_CONFIG_t generalConfig;

                ReadGeneralConfig(&generalConfig);
                msgLen = snprintf(replyMsg, MAX_TX_SZ, "%c%s%c%d%c%d%c%c",
                                  SOM, headerReq[CGI_REQ_GETPORT], FSP, generalConfig.tcpPort, FSP, generalConfig.forwardedTcpPort, FSP,EOM);

                if(SendToSocket(clientSockFd, (UINT8PTR)replyMsg, msgLen, MAX_INTERAL_SND_TIMEOUT) == FAIL)
                {
                    EPRINT(NETWORK_MANAGER, "internal msg send failed for get port request");
                }
            }
            break;

            case WS_SYSTEM_INFO:
            {
                /* Send response of get system information request */
                sendGetSystemInfoResponse(clientSockFd);
            }
            break;

            case WS_SYSTEM_LOGS:
            {
                /* Parse request type */
                if (ParseStr(&msgPtr, FSP, parseStr[0], sizeof(parseStr[0])) == FALSE)
                {
                    sendWebStatusRespMsg(clientSockFd, SYS_LOG_STS_PARSE_ERR);
                    break;
                }

                if (strcmp(parseStr[0], "GET") == 0)
                {
                    /* Send response of get system logs request */
                    sendGetSystemLogsResponse(clientSockFd);
                }
                else
                {
                    /* Parse json message */
                    if (ParseNStr(&msgPtr, FSP, replyMsg, sizeof(replyMsg)) == FALSE)
                    {
                        EPRINT(NETWORK_MANAGER, "fail to parse json message data");
                        sendWebStatusRespMsg(clientSockFd, SYS_LOG_STS_PARSE_ERR);
                        break;
                    }

                    /* Send response of set system logs request */
                    sendSetSystemLogsResponse(clientSockFd, replyMsg);
                }
            }
            break;

            case WS_PCAP_TRACE:
            {
                /* Parse request type */
                if (ParseStr(&msgPtr, FSP, parseStr[0], sizeof(parseStr[0])) == FALSE)
                {
                    sendWebStatusRespMsg(clientSockFd, PCAP_STATUS_PARSE_ERROR);
                    break;
                }

                if (strcmp(parseStr[0], "GET") == 0)
                {
                    /* Send response of get pcap trace statistics request */
                    sendGetPcapTraceStatsResponse(clientSockFd);
                }
                else
                {
                    /* Parse json message */
                    if (ParseNStr(&msgPtr, FSP, replyMsg, sizeof(replyMsg)) == FALSE)
                    {
                        EPRINT(NETWORK_MANAGER, "fail to parse json message data");
                        sendWebStatusRespMsg(clientSockFd, PCAP_STATUS_PARSE_ERROR);
                        break;
                    }

                    /* Send response of set pcap trace action request */
                    sendSetPcapTraceActionResponse(clientSockFd, replyMsg);
                }
            }
            break;

            case GET_MAC_SERVER_CFG:
            {
                MAC_SERVER_CNFG_t macServerCfg;

                ReadMacServerConfig(&macServerCfg);
                snprintf(replyMsg, MAX_TX_SZ, "%c%s%c%s%c%d%c%s%c%c", SOM, headerReq[GET_MAC_SERVER_CFG], FSP,
                        (macServerCfg.connectMode == CONNECT_THROUGH_IP ? macServerCfg.ip : macServerCfg.name), FSP,
                        macServerCfg.port, FSP, macServerCfg.service, FSP, EOM);

                // Send response message
                if (SendToSocket(clientSockFd, (UINT8PTR)replyMsg, strlen(replyMsg), MAX_INTERAL_SND_TIMEOUT) == FAIL)
                {
                    EPRINT(NETWORK_MANAGER, "fail to send mac server config");
                }
            }
            break;

            case DHCP_SERVER_NOTIFY:
            {
                DHCP_SERVER_NOTIFY_t dhcpServerNotify;

                /* Parse all fields of DHCP server notify information */
                for(loop = 0; loop < DHCP_SERVER_INFO_FIELD_MAX; loop++)
                {
                    if (ParseStr(&msgPtr, FSP, parseStr[loop], sizeof(parseStr[loop])) == FALSE)
                    {
                        EPRINT(NETWORK_MANAGER, "fail to parse dhcp server notify info: [loop=%d]", loop);
                        break;
                    }
                }

                /* Check successfully all fields parsed or not */
                if (loop < DHCP_SERVER_INFO_FIELD_MAX)
                {
                    /* Error in field parsing */
                    break;
                }

                /* Store provide information */
                snprintf(dhcpServerNotify.action, sizeof(dhcpServerNotify.action), "%s", parseStr[0]);
                snprintf(dhcpServerNotify.clientMacAddr, sizeof(dhcpServerNotify.clientMacAddr), "%s", parseStr[1]);
                snprintf(dhcpServerNotify.assignIpAddr, sizeof(dhcpServerNotify.assignIpAddr), "%s", parseStr[2]);
                dhcpServerNotify.leaseExpireTime = atol(parseStr[3]);
                dhcpServerNotify.remainingTime = atol(parseStr[4]);
                snprintf(dhcpServerNotify.clientHostname, sizeof(dhcpServerNotify.clientHostname), "%s", parseStr[5]);

                /* Provide information to DHCP server handler */
                DhcpServerLeaseUpdateNotify(&dhcpServerNotify);
            }
            break;

            case DDNS_CLIENT_NOTIFY:
            {
                /* Parse all fields of DHCP server notify information */
                for(loop = 0; loop < DDNS_CLIENT_INFO_FIELD_MAX; loop++)
                {
                    if (ParseStr(&msgPtr, FSP, parseStr[loop], sizeof(parseStr[loop])) == FALSE)
                    {
                        EPRINT(NETWORK_MANAGER, "fail to parse ddns client notify info: [loop=%d]", loop);
                        break;
                    }
                }

                /* Check successfully all fields parsed or not */
                if (loop < DDNS_CLIENT_INFO_FIELD_MAX)
                {
                    /* Error in field parsing */
                    break;
                }

                /* DDNS client event notify: 0=event, 1=ip, 2=errNum, 3=errMsg */
                DdnsClientEventNotify(parseStr[0], parseStr[1], parseStr[2], parseStr[3]);
            }
            break;

            default:
            {
                /* Nothing to do */
                EPRINT(NETWORK_MANAGER, "invld msg id rcvd: [msgId=%d]", msgId);
            }
            break;
        }

        /* Close client socket connection */
        CloseSocket(&clientSockFd);
    }

    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is thread function.This function manages network connection. It sends and receives
 *          command from CMS/Web. It process message and send acknowledge to client. According Client
 *          it manages the session.
 * @param   arg
 */
VOIDPTR runNetworkManagerMain(VOIDPTR arg)
{
    INT32                   clientSockFd = INVALID_CONNECTION;
    SOCK_ADDR_INFO_u        clientSockAddr;
    socklen_t               clientSockSize = sizeof(clientSockAddr);
    UINT8                   msgId;
    CHARPTR 				msgPtr;
    CHAR                    clientAddrStr[IPV6_ADDR_LEN_MAX];
    UINT16                  clientPort;
    USER_ACCOUNT_CONFIG_t	userAccountConfig;
    INT16 					curEventWriteIndex = 0;
    UINT64                  prevTimeInSec;
    UINT64                  currTimeInSec;
    UINT8                   sessionIdx;
    NET_CMD_STATUS_e        respStatus;
    CHAR                    recvMsg[MAX_RCV_SZ];
    UINT32                  recvMsgLen;
    CHAR                    replyMsg[MAX_REPLY_SZ];
    CHAR                    advncdetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];
    UINT8                   pollSts;
    INT16                   pollRevent;

    THREAD_START("EXT_NW");

    /* Initializa previous time variable for future reference */
    prevTimeInSec = GetMonotonicTimeInSec();

    /* Initialisation of thread status */
    networkManagerInfo.terminateFlg = START;
    while(networkManagerInfo.terminateFlg == START)
    {
        /* Separate 1 second timer managed for keepalive and polling timer counter because if any remote device is
         * doing TCP flooding on server socket without any data then we will accept connection and will wait for
         * 1 second for data from that connection and data is not available then we again wait for new connection.
         * So that, select() API time out occures after ~1000 second (becases 1 sec wait in data receive). */

        /* Get current monotonic time */
        currTimeInSec = GetMonotonicTimeInSec();

        /* Monotonic time increases linearly. Hence any changed found in second means atleast 1 second time elapsed */
        if (currTimeInSec != prevTimeInSec)
        {
            /* Update previous monotonic time for future reference */
            prevTimeInSec = currTimeInSec;

            /* Take a local copy of current event write index */
            curEventWriteIndex = GetCurrentEventWriteIndex();

            /* Check for all the active network sessions */
            for (sessionIdx = 0; sessionIdx < (MAX_NW_CLIENT - 1); sessionIdx++)
            {
                /* check if user session is assigned or not */
                if (GetUserClientType(sessionIdx) >= NW_CLIENT_TYPE_P2P_MOBILE)
                {
                    /* user session is not assigned to n/w manager */
                    continue;
                }

                /* check if session is active */
                if (TRUE != IsUserSessionActive(sessionIdx))
                {
                    continue;
                }

                /* tick keep alive timer */
                if (TRUE != TickUserKeepAlive(sessionIdx))
                {
                    /* user has been logged out due to timer expire */
                    continue;
                }

                /* check if polling active */
                if (TRUE != IsUserPollingActive(sessionIdx))
                {
                    /* Polling is not active for this session */
                    continue;
                }

                /* check if event available */
                if (curEventWriteIndex != GetUserEventReadIndex(sessionIdx))
                {
                    respStatus = CMD_EVENT_AVAILABLE;
                }
                else
                {
                    /* decrement poll timer */
                    if (TRUE != TickUserPollTimer(sessionIdx))
                    {
                        /* Polling timer expired but we don't have any event */
                        respStatus = CMD_NO_EVENTS;
                    }
                    else
                    {
                        /* Polling timer is not expired */
                        respStatus = CMD_SUCCESS;
                    }
                }

                /* Polling timer is not expired and we don't have any event to send back then nothing to do */
                if (respStatus != CMD_SUCCESS)
                {
                    INT32 pollFd = GetUserPollFd(sessionIdx);

                    /* Check socket status, Send data only if it is active */
                    if (TRUE == CheckSocketFdState(pollFd))
                    {
                        /* Prepare reply message with either event(s) available or no events available */
                        snprintf(replyMsg, MAX_REPLY_SZ, "%c%s%c%d%c%c", SOM, headerReq[RSP_POL], FSP, respStatus, FSP, EOM);

                        /* Send replay message */
                        SendToSocket(pollFd, (UINT8PTR)replyMsg, strlen(replyMsg), MESSAGE_REPLY_TIMEOUT);
                    }

                    /* Now close socket */
                    close(pollFd);
                    SetUserPollFd(sessionIdx, INVALID_CONNECTION);
                }
            }

            /* Check all password reset sessions timer */
            CheckPasswordResetSession();
        }

        /* Is any event available on server socket? */
        pollSts = GetSocketPollEvent(networkManagerInfo.nvrServerSockFd, (POLLRDNORM | POLLRDHUP), 1000, &pollRevent);

        /* Check for event on FD with timeout */
        if (TIMEOUT == pollSts)
        {
            /* It is timed-out. Hence again wait for new event */
            continue;
        }

        if (FAIL == pollSts)
        {
            /* Error in poll */
            EPRINT(NETWORK_MANAGER, "poll failed");
            continue;
        }

        /* There is a connection request. So, Wait for client to connect */
        clientSockFd = accept(networkManagerInfo.nvrServerSockFd, &clientSockAddr.sockAddr, &clientSockSize);
        if(clientSockFd < 0)
        {
            /* Accept failed to try again */
            EPRINT(NETWORK_MANAGER, "fail to accept connection: [err=%s]", STR_ERR);
            continue;
        }

        /* We need to convert socket address from ipv6 to ipv4 if client is ipv4 */
        ConvertIpv4MappedIpv6SockAddr(&clientSockAddr);
        GetIpAddrStrFromSockAddr(&clientSockAddr, clientAddrStr, sizeof(clientAddrStr));
        clientPort = GetHostPortFromSockAddr(&clientSockAddr);

        /* Make this connection as nonblocking */
        if (FALSE == SetSockFdOption(clientSockFd))
        {
            EPRINT(NETWORK_MANAGER, "fail to set sock options: [fd=%d], [addr=%s:%d]", clientSockFd, clientAddrStr, clientPort);
            CloseSocket(&clientSockFd);
            continue;
        }

        /* Receive message from socket */
        memset(recvMsg, 0, sizeof(recvMsg));
        if (RecvMessage(clientSockFd, recvMsg, &recvMsgLen, SOM, EOM, MAX_RCV_SZ-1, MAX_FRAME_WAIT_TIME) != SUCCESS)
        {
            EPRINT(NETWORK_MANAGER, "external msg recv timeout or connection closed: [addr=%s:%d]", clientAddrStr, clientPort);
            CloseSocket(&clientSockFd);
            continue;
        }

        /* Add null in received message */
        recvMsg[recvMsgLen] = '\0';

        /* Remove SOM from received data */
        msgPtr = (recvMsg + 1);

        //Get header
        if(FindHeaderIndex(&msgPtr, &msgId) != CMD_SUCCESS)
        {
            EPRINT(NETWORK_MANAGER, "fail to parse header index: [msg=%s], [addr=%s:%d]", msgPtr, clientAddrStr, clientPort);
            CloseSocket(&clientSockFd);
            continue;
        }

        /* Check for request type */
        switch(msgId)
        {
            case REQ_LOG:
            {
                /* process client's login request, validations and appropriate response */
                ProcessClientLoginRequest(msgPtr, &clientSockAddr, clientSockFd, CLIENT_CB_TYPE_NATIVE, NW_CLIENT_TYPE_MAX, &sessionIdx);
            }
            break;

            case REQ_POL:
            {
                /* find session index from session id */
                if (FindSessionIndex(&msgPtr, &sessionIdx) != CMD_SUCCESS)
                {
                    EPRINT(NETWORK_MANAGER, "invld session id found: [msg=%s], [addr=%s:%d]", headerReq[msgId], clientAddrStr, clientPort);
                    break;
                }

                /* if session is not active or polling is already on */
                if ((FALSE == IsUserSessionActive(sessionIdx)) || (INVALID_CONNECTION != GetUserPollFd(sessionIdx)))
                {
                    break;
                }

                /* reload keep alive */
                ReloadUserKeepAlive(sessionIdx);

                /* take local copy of event write index */
                curEventWriteIndex = GetCurrentEventWriteIndex();
                if (curEventWriteIndex != GetUserEventReadIndex(sessionIdx))
                {
                    /* event available */
                    snprintf(replyMsg, MAX_REPLY_SZ, "%c%s%c%02d%c%c", SOM, headerReq[RSP_POL], FSP, CMD_EVENT_AVAILABLE, FSP, EOM);
                    SendToSocket(clientSockFd, (UINT8PTR)replyMsg, strlen(replyMsg),MESSAGE_REPLY_TIMEOUT);
                }
                else
                {
                    /* event not available, store client polling fd and wait for event to be available */
                    SetUserPollFd(sessionIdx, clientSockFd);

                    /* re-load poll timer */
                    ReloadUserPollTimer(sessionIdx);

                    /* Don't close this connection as we have stored fd */
                    clientSockFd = INVALID_CONNECTION;
                }
            }
            break;

            case GET_CFG:
            {
                /* find session index from session id */
                if (FindSessionIndex(&msgPtr, &sessionIdx) != CMD_SUCCESS)
                {
                    EPRINT(NETWORK_MANAGER, "invld session id found: [msg=%s], [addr=%s:%d]", headerReq[msgId], clientAddrStr, clientPort);
                    break;
                }

                /* check if user is active */
                if (FALSE == IsUserSessionActive(sessionIdx))
                {
                    EPRINT(NETWORK_MANAGER, "user session is not active: [msg=%s], [sessionIdx=%d], [addr=%s:%d]", headerReq[msgId], sessionIdx,
                           clientAddrStr, clientPort);
                    break;
                }

                /* reload keep alive timer */
                ReloadUserKeepAlive(sessionIdx);

                /* process config request */
                ProcessGetConfig(msgPtr, clientSockFd, CLIENT_CB_TYPE_NATIVE);
            }
            break;

            case SET_CFG:
            {
                /* find session index from session id */
                if (FindSessionIndex(&msgPtr, &sessionIdx) != CMD_SUCCESS)
                {
                    EPRINT(NETWORK_MANAGER, "invld session id found: [msg=%s], [addr=%s:%d]", headerReq[msgId], clientAddrStr, clientPort);
                    break;
                }

                /* check if user is active */
                if (FALSE == IsUserSessionActive(sessionIdx))
                {
                    EPRINT(NETWORK_MANAGER, "user session is not active: [msg=%s], [sessionIdx=%d], [addr=%s:%d]", headerReq[msgId], sessionIdx,
                           clientAddrStr, clientPort);
                    break;
                }

                /* reload keep alive timer */
                ReloadUserKeepAlive(sessionIdx);
                ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIdx), &userAccountConfig);

                /* check user rights */
                if (userAccountConfig.userGroup != ADMIN)
                {
                    /* Config change not allowed to other than admin group */
                    snprintf(replyMsg, MAX_REPLY_SZ, "%c%s%c%d%c%c", SOM, headerReq[RPL_CFG], FSP, CMD_NO_PRIVILEGE, FSP, EOM);
                    SendToSocket(clientSockFd, (UINT8PTR)replyMsg, strlen(replyMsg), MESSAGE_REPLY_TIMEOUT);
                    break;
                }

                /* Check from where request has been received. If IP is local host than it is local client */
                if ((clientSockAddr.sockAddr.sa_family == AF_INET) && (clientSockAddr.sockAddr4.sin_addr.s_addr == LOCAL_USER_IP))
                {
                    /* Add user name and local client string in advance details of event log */
                    snprintf(advncdetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s | "LOCAL_CLIENT_EVT_STR, userAccountConfig.username);
                }
                else
                {
                    /* Now either it is device client or it is mobile client. Add user name, device client or mobile client stirng,
                     * and add its IP address in event log for better identification */
                    snprintf(advncdetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s | %s | %s", userAccountConfig.username,
                             (NW_CLIENT_TYPE_WEB == GetUserClientType(sessionIdx)) ? DEVICE_CLIENT_EVT_STR : MOBILE_CLIENT_EVT_STR, clientAddrStr);
                }

                /* Start processing the SET_CFG request */
                ProcessSetConfig(msgPtr, clientSockFd, advncdetail, CLIENT_CB_TYPE_NATIVE);
            }
            break;

            case DEF_CFG:
            {
                /* find session index from session id */
                if (FindSessionIndex(&msgPtr, &sessionIdx) != CMD_SUCCESS)
                {
                    EPRINT(NETWORK_MANAGER, "invld session id found: [msg=%s], [addr=%s:%d]", headerReq[msgId], clientAddrStr, clientPort);
                    break;
                }

                /* check if user is active */
                if (FALSE == IsUserSessionActive(sessionIdx))
                {
                    EPRINT(NETWORK_MANAGER, "user session is not active: [msg=%s], [sessionIdx=%d], [addr=%s:%d]", headerReq[msgId], sessionIdx,
                           clientAddrStr, clientPort);
                    break;
                }

                /* reload keep alive timer */
                ReloadUserKeepAlive(sessionIdx);
                ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIdx), &userAccountConfig);

                /* check user rights */
                if (userAccountConfig.userGroup != ADMIN)
                {
                    /* Config change not allowed to other than admin group */
                    snprintf(replyMsg, MAX_REPLY_SZ, "%c%s%c%d%c%c", SOM, headerReq[RPL_CFG], FSP, CMD_NO_PRIVILEGE, FSP, EOM);
                    SendToSocket(clientSockFd, (UINT8PTR)replyMsg, strlen(replyMsg), MESSAGE_REPLY_TIMEOUT);
                    break;
                }

                /* Check from where request has been received. If IP is local host than it is local client */
                if ((clientSockAddr.sockAddr.sa_family == AF_INET) && (clientSockAddr.sockAddr4.sin_addr.s_addr == LOCAL_USER_IP))
                {
                    /* Add user name and local client string in advance details of event log */
                    snprintf(advncdetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s | "LOCAL_CLIENT_EVT_STR, userAccountConfig.username);
                }
                else
                {
                    /* Now either it is device client or it is mobile client. Add user name, device client or mobile client stirng,
                     * and add its IP address in event log for better identification */
                    snprintf(advncdetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s | %s | %s", userAccountConfig.username,
                             (NW_CLIENT_TYPE_WEB == GetUserClientType(sessionIdx)) ? DEVICE_CLIENT_EVT_STR : MOBILE_CLIENT_EVT_STR, clientAddrStr);
                }

                /* process default config request */
                ProcessDefConfig(msgPtr, clientSockFd, advncdetail, CLIENT_CB_TYPE_NATIVE);
            }
            break;

            case SET_CMD:
            {
                /* find session index from session id */
                if (FindSessionIndex(&msgPtr, &sessionIdx) != CMD_SUCCESS)
                {
                    EPRINT(NETWORK_MANAGER, "invld session id found: [msg=%s], [addr=%s:%d]", headerReq[msgId], clientAddrStr, clientPort);
                    break;
                }

                /* check if user is active */
                if (FALSE == IsUserSessionActive(sessionIdx))
                {
                    EPRINT(NETWORK_MANAGER, "user session is not active: [msg=%s], [sessionIdx=%d], [addr=%s:%d]", headerReq[msgId], sessionIdx,
                           clientAddrStr, clientPort);
                    break;
                }

                /* reload keep alive timer */
                ReloadUserKeepAlive(sessionIdx);

                /* Process set command request and provide the response to the client */
                ProcessSetCommand(msgPtr, sessionIdx, clientSockFd, CLIENT_CB_TYPE_NATIVE);

                /* Don't close this connection as we have passed fd for further action */
                clientSockFd = INVALID_CONNECTION;
            }
            break;

            case REQ_EVT:
            {
                /* find session index from session id */
                if (FindSessionIndex(&msgPtr, &sessionIdx) != CMD_SUCCESS)
                {
                    EPRINT(NETWORK_MANAGER, "invld session id found: [msg=%s], [addr=%s:%d]", headerReq[msgId], clientAddrStr, clientPort);
                    break;
                }

                /* check if user is active */
                if (FALSE == IsUserSessionActive(sessionIdx))
                {
                    EPRINT(NETWORK_MANAGER, "user session is not active: [msg=%s], [sessionIdx=%d], [addr=%s:%d]", headerReq[msgId], sessionIdx,
                           clientAddrStr, clientPort);
                    break;
                }

                /* reload keep alive timer */
                ReloadUserKeepAlive(sessionIdx);

                /* Get and send live events to client */
                GetLiveEvents(sessionIdx, clientSockFd, CLIENT_CB_TYPE_NATIVE);

                /* Don't close this connection as we have already closed */
                clientSockFd = INVALID_CONNECTION;
            }
            break;

            case REQ_FTS:
            {
                /* find session index from session id */
                if (FindSessionIndex(&msgPtr, &sessionIdx) != CMD_SUCCESS)
                {
                    snprintf(replyMsg, MAX_REPLY_SZ, "%c%s%c%d%c%c", SOM, headerReq[RPL_FTS], FSP, CMD_INVALID_SESSION, FSP, EOM);
                    SendToSocket(clientSockFd, (UINT8PTR)replyMsg, strlen(replyMsg), MESSAGE_REPLY_TIMEOUT);
                    break;
                }

                /* check if user is active */
                if (FALSE == IsUserSessionActive(sessionIdx))
                {
                    EPRINT(NETWORK_MANAGER, "user session is not active: [msg=%s], [sessionIdx=%d], [addr=%s:%d]", headerReq[msgId], sessionIdx,
                           clientAddrStr, clientPort);
                    break;
                }

                /* Process file transfer request */
                ProcessFileTransferReq(&msgPtr, clientSockFd, sessionIdx);

                /* Don't close this connection because we passed it for further action */
                clientSockFd = INVALID_CONNECTION;
            }
            break;

            case DOR_CMD:
            {
                /* Process door command request */
                ProcessDoorCommand(msgPtr);
            }
            break;

            case PWD_RST:
            {
                /* Process Password Reset Command */
                ProcessPasswordResetCmd(&msgPtr, &clientSockAddr, clientSockFd, CLIENT_CB_TYPE_NATIVE);

                /* Don't close this connection as we have passed fd for further action */
                clientSockFd = INVALID_CONNECTION;
            }
            break;

            default:
            {
                /* Nothing to do */
                EPRINT(NETWORK_MANAGER, "invld msg id recvd: [msgId=%d]", msgId);
            }
            break;
        }

        /* Close the connection if fd is valid */
        CloseSocket(&clientSockFd);
    }

    /* Now logout all users */
    for(sessionIdx = 0; sessionIdx < (MAX_NW_CLIENT - 1); sessionIdx++)
    {
        /* check if user session is assigned or not */
        if (GetUserClientType(sessionIdx) >= NW_CLIENT_TYPE_P2P_MOBILE)
        {
            /* session is not assigned to network manager */
            continue;
        }

        UserLogout(sessionIdx);
    }

    /* Clear all password reset sessions */
    for(sessionIdx = 0; sessionIdx < PWD_RST_SESSION_MAX; sessionIdx++)
    {
        DeallocPasswordResetSession(sessionIdx);
    }

    /* close server socket */
    CloseSocket(&networkManagerInfo.nvrServerSockFd);

    // Reinit network manager if restart
    if(networkManagerInfo.terminateFlg == RESTART)
    {
        DPRINT(NETWORK_MANAGER, "network manager restarting...");

        do
        {
            sleep(2);

        } while(initExternalNw() == FAIL);
    }

    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get live events for current client and send them to client
 * @param   sessionIdx
 * @param   connFd
 * @param   clientCbType
 */
void GetLiveEvents(UINT8 sessionIdx, INT32 connFd, CLIENT_CB_TYPE_e clientCbType)
{
    UINT16              evtCnt = 0;
    NET_CMD_STATUS_e    cmdResp = CMD_SUCCESS;
    EVENT_LOG_t         evtLog;
    CHAR                replyMsg[MAX_REPLY_SZ];

    if (sessionIdx >= MAX_NW_CLIENT)
    {
        EPRINT(NETWORK_MANAGER, "invalid session Index: [sessionId=%d]", sessionIdx);
        closeConnCb[clientCbType](&connFd);
        return;
    }

    /* Check if any event available for client, if available get current write index.
     * Take A local copy of current event write index */
    INT16 tmpEventWriteIndex = GetCurrentEventWriteIndex();
    if (GetUserEventReadIndex(sessionIdx) == tmpEventWriteIndex)
    {
        /* Set no events response code */
        cmdResp = CMD_NO_EVENTS;
    }

    /* Prepare message header with response code */
    UINT32 outLen = snprintf(replyMsg, MAX_REPLY_SZ, "%c%s%c%d%c", SOM, headerReq[RCV_EVT], FSP, cmdResp, FSP);
    if (outLen > MAX_REPLY_SZ)
    {
        EPRINT(NETWORK_MANAGER, "more buffer size required for msg: [length=%d]", outLen);
        outLen = MAX_REPLY_SZ;
    }

    do
    {
        /* If no events available then send only response code */
        if (cmdResp == CMD_NO_EVENTS)
        {
            /* Events are not available */
            break;
        }

        while (GetUserEventReadIndex(sessionIdx) != tmpEventWriteIndex)
        {
            /* Take Temporary Pointer for event at read index */
            GetEventForCurrentReadIndex(sessionIdx, &evtLog);

            // Append Event Data in buffer
            outLen += snprintf(replyMsg + outLen, MAX_REPLY_SZ - outLen, "%c%02d%02d%04d%02d%02d%02d%c%d%c%d%c%s%c%d%c%s%c%c",
                               SOI, evtLog.eventTime.tm_mday, (evtLog.eventTime.tm_mon + 1), evtLog.eventTime.tm_year,
                               evtLog.eventTime.tm_hour, evtLog.eventTime.tm_min, evtLog.eventTime.tm_sec,
                               FSP, evtLog.eventType, FSP, evtLog.eventSubtype, FSP, evtLog.detail,
                               FSP, evtLog.eventState, FSP, evtLog.advncDetail, FSP, EOI);

            /* Validate buffer size for add last two character including NULL */
            if (outLen > MAX_REPLY_SZ - 2)
            {
                EPRINT(NETWORK_MANAGER, "more buffer size required for msg: [length=%d]", outLen);
                outLen = MAX_REPLY_SZ - 2;
            }

            /* Count event to restrict to single shot event count */
            evtCnt++;
            if (evtCnt >= MAX_EVENTS_IN_ONE_SEG)
            {
                break;
            }
        }

    }while(0);

    /* Append EOM to indicate end of message */
    outLen += snprintf(replyMsg + outLen, MAX_REPLY_SZ - outLen, "%c", EOM);
    if(outLen > MAX_REPLY_SZ)
    {
        EPRINT(NETWORK_MANAGER, "more buffer size required for msg: [length=%d]", outLen);
        outLen = MAX_REPLY_SZ;
    }

    /* Send receive event response */
    if (FAIL == sendCmdCb[clientCbType](connFd, (UINT8PTR)replyMsg, outLen, MESSAGE_REPLY_TIMEOUT))
    {
        EPRINT(NETWORK_MANAGER, "fail to send live event resp: [sessionIdx=%d], [cmdResp=%d]", sessionIdx, cmdResp);
    }

    /* Close the socket */
    closeConnCb[clientCbType](&connFd);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function output is index of session if success otherwise error
 * @param   pMsgBuff - buf pointer
 * @param   pSessionIdx - index of session
 * @return  Message parse success or error reason
 */
NET_CMD_STATUS_e FindSessionIndex(CHARPTR *pMsgBuff, UINT8 *pSessionIdx)
{
    CHAR    idBuff[MAX_SESSION_ID_LEN];
    INT32   sessionId;

    if (ParseStr(pMsgBuff, FSP, idBuff, MAX_SESSION_ID_LEN) != SUCCESS)
    {
        return CMD_INVALID_SYNTAX;
    }

    /* Find session index from given session id */
    sessionId = atoi(idBuff);
    for(*pSessionIdx = 0; *pSessionIdx < MAX_NW_CLIENT; (*pSessionIdx)++)
    {
        if (TRUE == IsUserSessionValid(*pSessionIdx, sessionId))
        {
            /* Session id match */
            return CMD_SUCCESS;
        }
    }

    /* Session not found */
    return CMD_INVALID_SYNTAX;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is general config notify function
 * @param   newGenCfg
 * @param   oldGenCfg
 */
void NwGeneralCfgUpdate(GENERAL_CONFIG_t newGenCfg, GENERAL_CONFIG_t *oldGenCfg)
{
    if(oldGenCfg->httpPort != newGenCfg.httpPort)
    {
        WebServerServiceStartStop(STOP, oldGenCfg->httpPort);
        WebServerServiceStartStop(START, newGenCfg.httpPort);
    }

    if(oldGenCfg->tcpPort != newGenCfg.tcpPort)
    {
        networkManagerInfo.terminateFlg = RESTART;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initialize tcp server for external device (Web client or CMS)
 * @return  SUCESS/FAIL
 */
static BOOL initExternalNw(void)
{
    INT32               yes = TRUE;
    GENERAL_CONFIG_t    generalConfig;
    SOCK_ADDR_INFO_u    serverSockAddr;

    /* Read general configuration */
    ReadGeneralConfig(&generalConfig);

    /* Create NVR server for network communication and internal messages */
    networkManagerInfo.nvrServerSockFd = socket(AF_INET6, TCP_SOCK_OPTIONS, 0);
    if(networkManagerInfo.nvrServerSockFd == INVALID_CONNECTION)
    {
        EPRINT(NETWORK_MANAGER, "fail to open external server socket: [err=%s]", STR_ERR);
        return FAIL;
    }

    memset(&serverSockAddr, 0, sizeof(serverSockAddr));
    serverSockAddr.sockAddr6.sin6_family = AF_INET6;
    serverSockAddr.sockAddr6.sin6_addr = in6addr_any;
    serverSockAddr.sockAddr6.sin6_port = htons(generalConfig.tcpPort);

    setsockopt(networkManagerInfo.nvrServerSockFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    if(bind(networkManagerInfo.nvrServerSockFd, &serverSockAddr.sockAddr, sizeof(serverSockAddr.sockAddr6)) < STATUS_OK)
    {
        CloseSocket(&networkManagerInfo.nvrServerSockFd);
        EPRINT(NETWORK_MANAGER, "fail to bind external server socket: [err=%s]", STR_ERR);
        return FAIL;
    }

    if(listen(networkManagerInfo.nvrServerSockFd, MAX_CONCURRANT_REQ) < STATUS_OK)
    {
        EPRINT(NETWORK_MANAGER, "fail to listen external server socket: [err=%s]", STR_ERR);
        CloseSocket(&networkManagerInfo.nvrServerSockFd);
        return FAIL;
    }

    DPRINT(NETWORK_MANAGER, "external server started: [tcpPort=%d]", generalConfig.tcpPort);
    if (FALSE == Utils_CreateThread(NULL, runNetworkManagerMain, NULL, DETACHED_THREAD, EXT_NW_MNGR_THREAD_STACK_SZ))
    {
        EPRINT(NETWORK_MANAGER, "fail to create external server thread");
        CloseSocket(&networkManagerInfo.nvrServerSockFd);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initialize tcp server for internal usage.
 * @return  SUCESS/FAIL
 */
static BOOL initInternalNw(void)
{
    INT32               yes = TRUE;
    struct sockaddr_un  internalServerAddr;

    /* Create local server for handle requst.(e.g cgi, etc) */
    internalServerSockFd = socket(AF_UNIX, TCP_SOCK_OPTIONS, 0);
    if (internalServerSockFd == INVALID_CONNECTION)
    {
        EPRINT(NETWORK_MANAGER, "fail to open internal server socket: [err=%s]", STR_ERR);
        return FAIL;
    }

    unlink(INTERNAL_SOCKET_FILE);
    setsockopt(internalServerSockFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    memset(&internalServerAddr, 0, sizeof(internalServerAddr));
    internalServerAddr.sun_family = AF_UNIX;
    snprintf(internalServerAddr.sun_path, sizeof(internalServerAddr.sun_path), INTERNAL_SOCKET_FILE);

    if (bind(internalServerSockFd, (struct sockaddr*)&internalServerAddr, sizeof(internalServerAddr)) < STATUS_OK)
    {
        EPRINT(NETWORK_MANAGER, "fail to bind internal server socket: [err=%s]", STR_ERR);
        CloseSocket(&internalServerSockFd);
        return FAIL;
    }

    if (listen(internalServerSockFd, MAX_CONCURRANT_REQ) < STATUS_OK)
    {
        EPRINT(NETWORK_MANAGER, "fail to listen internal server socket: [err=%s]", STR_ERR);
        CloseSocket(&internalServerSockFd);
        return FAIL;
    }

    //Changing permision of socket file
    chmod(INTERNAL_SOCKET_FILE, 0777);

    /* Create thread receive message from internal AF_UNIX socket */
    if (FALSE == Utils_CreateThread(NULL, internalMsgServerLoop, &internalServerSockFd, DETACHED_THREAD, INT_NW_MNGR_THREAD_STACK_SZ))
    {
        EPRINT(NETWORK_MANAGER, "fail to create internal server thread");
        CloseSocket(&internalServerSockFd);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will construct the status response message.
 * @param   sockFd
 * @param   status
 */
static void sendWebStatusRespMsg(INT32 sockFd, UINT32 status)
{
    CHAR    respMsg[MAX_TX_SZ];
    UINT16  respMsgLen;

    /* Prepare response message */
    respMsgLen = snprintf(respMsg, sizeof(respMsg), WS_JSON_RESP_MSG, status);

    /* Send response message */
    SendToSocket(sockFd, (UINT8PTR)respMsg, respMsgLen, MAX_INTERAL_SND_TIMEOUT);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send response of get system information request
 * @param   clientSockFd
 */
static void sendGetSystemInfoResponse(INT32 clientSockFd)
{
    json_t  *root;
    CHARPTR jsonMsg;

    /* Create json object */
    root = json_object();

    /* Add required tag and value in json message */
    json_object_set_new(root, "status", json_integer(SYS_LOG_STS_SUCCESS));
    json_object_set_new(root, "model", json_string(GetNvrModelStr()));
    json_object_set_new(root, "version", json_string(GetSoftwareVersionStr()));
    json_object_set_new(root, "buildtime", json_string(GetBuildDateTimeStr()));

    /* Get json string */
    jsonMsg = json_dumps(root, JSON_INDENT(4));

    /* Free json object */
    json_decref(root);

    /* Is json message generated? */
    if (jsonMsg == NULL)
    {
        EPRINT(NETWORK_MANAGER, "json msg generation failed for build date-time request");
        return;
    }

    /* Send response message */
    if (SendToSocket(clientSockFd, (UINT8PTR)jsonMsg, strlen(jsonMsg), MAX_INTERAL_SND_TIMEOUT) == FAIL)
    {
        EPRINT(NETWORK_MANAGER, "internal msg send failed for build date-time request");
    }

    /* Free json message */
    FREE_MEMORY(jsonMsg);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send response of get system logs request
 * @param   clientSockFd
 */
static void sendGetSystemLogsResponse(INT32 clientSockFd)
{
    CHARPTR             jsonMsg;
    json_t              *root;
    json_t              *systemlogs;
    json_t              *serverlevels;
    json_t              *guilevels;
    LOG_LEVEL_e         level;
    DBG_CONFIG_PARAM_t  debugConfig;

    /* Create json object */
    root = json_object();
    systemlogs = json_object();
    serverlevels = json_array();
    guilevels = json_array();

    /* Get current debug config */
    GetDebugConfig(&debugConfig);

    /* Set server log levels in json format */
    for(level = 0; level < MAX_SERVER_LEVELS; level++)
    {
        json_array_append_new(serverlevels, json_boolean(GET_BIT(debugConfig.debugLevels, level)));
    }

    /* Set gui log levels in json format */
    for(level = GROUP_1; level < MAX_GUI_LEVELS; level++)
    {
        json_array_append_new(guilevels, json_boolean(GET_BIT(debugConfig.debugLevels, level)));
    }

    /* Add required tag and value in json message */
    json_object_set_new(root, "status", json_integer(SYS_LOG_STS_SUCCESS));
    json_object_set_new(root, "remotelogin", json_boolean(debugConfig.remoteLoginEnable));
    json_object_set_new(systemlogs, "enable", json_boolean(debugConfig.debugEnable));
    json_object_set_new(systemlogs, "sink", json_integer(debugConfig.debugDestination));
    json_object_set_new(systemlogs, "serverip", json_string(debugConfig.syslogServerAddr));
    json_object_set_new(systemlogs, "serverport", json_integer(debugConfig.syslogServerPort));
    json_object_set_new(systemlogs, "serverlevels", serverlevels);
    json_object_set_new(systemlogs, "guilevels", guilevels);

    /* Add header and payload object in root object */
    json_object_set_new(root, "systemlogs", systemlogs);

    /* Get json string */
    jsonMsg = json_dumps(root, JSON_INDENT(4));

    /* Free json object */
    json_decref(root);

    /* Is json message generated? */
    if (jsonMsg == NULL)
    {
        EPRINT(NETWORK_MANAGER, "json msg generation failed for get debug info request");
        sendWebStatusRespMsg(clientSockFd, SYS_LOG_STS_RESOURCE_ERR);
        return;
    }

    /* Send response message */
    if (SendToSocket(clientSockFd, (UINT8PTR)jsonMsg, strlen(jsonMsg), MAX_INTERAL_SND_TIMEOUT) == FAIL)
    {
        EPRINT(NETWORK_MANAGER, "internal msg send failed for get debug info request");
    }

    /* Free json message */
    FREE_MEMORY(jsonMsg);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send response of set system logs request
 * @param   clientSockFd
 * @param   jsonMsg
 */
static void sendSetSystemLogsResponse(INT32 clientSockFd, CHARPTR jsonMsg)
{
    json_t              *root;
    json_t              *json;
    json_t              *systemlogs;
    json_t              *serverlevels;
    json_t              *guilevels;
    json_error_t        jsonErr;
    LOG_LEVEL_e         level;
    DBG_CONFIG_PARAM_t  debugConfig;

    /* Load json msg */
    root = json_loads(jsonMsg, 0, &jsonErr);
    if (root == NULL)
    {
        EPRINT(NETWORK_MANAGER, "fail to load json msg for set debug info request: [line=%d], [err=%s]", jsonErr.line, jsonErr.text);
        sendWebStatusRespMsg(clientSockFd, SYS_LOG_STS_PARSE_ERR);
        return;
    }

    /* Get authentication paramter and validate it */
    json = json_object_get(root, "authentication");
    if ((!json_is_string(json)) || (strcmp(json_string_value(json), LOG_WEB_PAGE_PASSWORD)))
    {
        /* Invalid authentication found */
        json_decref(root);
        EPRINT(NETWORK_MANAGER, "fail to get auth param or auth failed");
        sendWebStatusRespMsg(clientSockFd, SYS_LOG_STS_AUTH_ERR);
        return;
    }

    /* Get current debug config */
    GetDebugConfig(&debugConfig);
    debugConfig.debugLevels = 0;

    /* Get system logs objects */
    systemlogs = json_object_get(root, "systemlogs");
    serverlevels = json_object_get(systemlogs, "serverlevels");
    guilevels = json_object_get(systemlogs, "guilevels");

    /* Validate system logs objects */
    if ((!json_is_object(systemlogs)) || (!json_is_array(serverlevels)) || (!json_is_array(guilevels)))
    {
        /* Invalid object found */
        json_decref(root);
        EPRINT(NETWORK_MANAGER, "fail to get log settings from json");
        sendWebStatusRespMsg(clientSockFd, SYS_LOG_STS_PARSE_ERR);
        return;
    }

    json = json_object_get(root, "remotelogin");
    if (json_is_boolean(json)) debugConfig.remoteLoginEnable = json_boolean_value(json);

    json = json_object_get(systemlogs, "enable");
    if (json_is_boolean(json)) debugConfig.debugEnable = json_boolean_value(json);

    json = json_object_get(systemlogs, "sink");
    if (json_is_integer(json)) debugConfig.debugDestination = json_integer_value(json);

    json = json_object_get(systemlogs, "serverip");
    if (json_is_string(json)) snprintf(debugConfig.syslogServerAddr, sizeof(debugConfig.syslogServerAddr), "%s", json_string_value(json));

    json = json_object_get(systemlogs, "serverport");
    if (json_is_integer(json)) debugConfig.syslogServerPort = json_integer_value(json);

    /* Get server log levels from json format */
    for (level = 0; level < MAX_SERVER_LEVELS; level++)
    {
        json = json_array_get(serverlevels, level);
        if (json_boolean(json)) json_is_true(json) ? SET_BIT(debugConfig.debugLevels, level) : RESET_BIT(debugConfig.debugLevels, level);
    }

    /* Get gui log levels from json format */
    for (level = GROUP_1; level < MAX_GUI_LEVELS; level++)
    {
        json = json_array_get(guilevels, (level - GROUP_1));
        if (json_boolean(json)) json_boolean_value(json) ? SET_BIT(debugConfig.debugLevels, level) : RESET_BIT(debugConfig.debugLevels, level);
    }

    /* Free json object */
    json_decref(root);

    /* Set debug levels */
    SetDebugConfig(&debugConfig);

    /* Notify other applications for debug config sync */
    setLocalClientDebugConfig(&debugConfig);
    SetRtspClientDebugConfig();
    SetCmdExeDebugConfig();
    SetNetworkManagerDebugConfig();

    /* Send success response to web client */
    sendWebStatusRespMsg(clientSockFd, SYS_LOG_STS_SUCCESS);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send common payload success response against pcap trace request
 * @param   clientSockFd
 * @param   pcapStatus
 * @param   numOfPktCaptured
 * @param   numOfBytesCaptured
 */
static void sendPcapTraceResponse(INT32 clientSockFd, PCAP_STATUS_e pcapStatus, UINT16 numOfPktCaptured, UINT32 numOfBytesCaptured)
{
    json_t  *root;
    CHARPTR jsonMsg;

    /* Create json object */
    root = json_object();

    /* Add required tag and value in json message */
    json_object_set_new(root, "status", json_integer(pcapStatus));
    json_object_set_new(root, "packets", json_integer(numOfPktCaptured));
    json_object_set_new(root, "bytes", json_integer(numOfBytesCaptured));

    /* Get json string */
    jsonMsg = json_dumps(root, JSON_INDENT(4));

    /* Free json object */
    json_decref(root);

    /* Is json message generated? */
    if (jsonMsg == NULL)
    {
        EPRINT(NETWORK_MANAGER, "json msg generation failed for get pcap status");
        sendWebStatusRespMsg(clientSockFd, PCAP_STATUS_PROCESS_ERROR);
        return;
    }

    /* Send response message */
    if (SendToSocket(clientSockFd, (UINT8PTR)jsonMsg, strlen(jsonMsg), MAX_INTERAL_SND_TIMEOUT) == FAIL)
    {
        EPRINT(NETWORK_MANAGER, "internal msg send failed for get pcap status");
    }

    /* Free json message */
    FREE_MEMORY(jsonMsg);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send response of get pcap trace statistics request
 * @param   clientSockFd
 */
static void sendGetPcapTraceStatsResponse(INT32 clientSockFd)
{
    UINT16 numOfPktCaptured;
    UINT32 numOfBytesCaptured;

    /* Get current stats of pcap trace */
    PCAP_STATUS_e pcapStatus = PcapGetStatus(&numOfPktCaptured, &numOfBytesCaptured);

    /* Send current stats of pcap trace */
    sendPcapTraceResponse(clientSockFd, pcapStatus, numOfPktCaptured, numOfBytesCaptured);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send response of set pcap trace action request
 * @param   clientSockFd
 * @param   jsonMsg
 */
static void sendSetPcapTraceActionResponse(INT32 clientSockFd, CHARPTR jsonMsg)
{
    PCAP_STATUS_e   pcapStatus;
    UINT16          numOfPktCaptured = 0;
    UINT32			numOfBytesCaptured = 0;
    json_t          *root;
    json_t          *json;
    json_error_t    jsonErr;

    /* Load json msg */
    root = json_loads(jsonMsg, 0, &jsonErr);
    if (root == NULL)
    {
        EPRINT(NETWORK_MANAGER, "fail to load json msg for pcap post request: [line=%d], [err=%s]", jsonErr.line, jsonErr.text);
        sendWebStatusRespMsg(clientSockFd, PCAP_STATUS_PARSE_ERROR);
        return;
    }

    /* Get pcap trace action object */
    json = json_object_get(root, "action");
    if (!json_is_string(json))
    {
        /* Action not found */
        json_decref(root);
        EPRINT(NETWORK_MANAGER, "fail to get action string from json");
        sendWebStatusRespMsg(clientSockFd, PCAP_STATUS_PARSE_ERROR);
        return;
    }

    /* Check pcap trace action type */
    if (strcmp(json_string_value(json), "start") == 0)
    {
        PCAP_START_t pcapStartParam = {NETWORK_PORT_MAX, ""};

        /* Get pcap capture interface and filter string */
        json = json_object_get(root, "interface");
        if (json_is_integer(json)) pcapStartParam.interface = json_integer_value(json);

        json = json_object_get(root, "filter");
        if (json_is_string(json)) snprintf(pcapStartParam.filterStr, sizeof(pcapStartParam.filterStr), "%s", json_string_value(json));

        /* Start Packet Capture with given Parameter */
        pcapStatus = PcapStart(&pcapStartParam);
    }
    else
    {
        /* Get current pcap status and stop the pcap capture */
        pcapStatus = PCAP_STATUS_SAFELY_CLOSED;
        PcapGetStatus(&numOfPktCaptured, &numOfBytesCaptured);
        PcapStop();
    }

    /* Free json object */
    json_decref(root);

    /* Send last stats of pcap trace */
    sendPcapTraceResponse(clientSockFd, pcapStatus, numOfPktCaptured, numOfBytesCaptured);
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
