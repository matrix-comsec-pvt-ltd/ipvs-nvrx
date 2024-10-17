#if !defined P2P_PROCESS_H
#define P2P_PROCESS_H

//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		P2pProcess.h
@brief      File containing the prototype of internal modules functions to provide P2P service
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "ConfigApi.h"
#include "DeviceInfo.h"
#include "P2pInterface.h"
#include "NetworkManager.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
/* Maximum clients to be served by P2P */
#define P2P_CLIENT_SUPPORT_MAX              (MAX_NW_CLIENT - 2) /* Max 9 clients support */

/* Ethernet packet size max */
#define P2P_PACKET_SIZE_MAX                 NETWORK_MTU_SIZE_MAX

/* P2P protocol tcp data retry max */
#define P2P_TCP_HEADER_DATA_RETRY_MAX       5

/* Password and salt key length for encryption and decryption */
#define CRYPTO_SALT_KEY_LEN_MAX             9   /* 8 + null */
#define CRYPTO_PASSWORD_LEN_MAX             28  /* 27 + null */

/* Random unique number length max */
#define P2P_UNIQUE_RAND_NUM_LEN_MAX         11  /* 10 + null */

/* P2P server protocol version supported by NVR */
#define P2P_SERVER_PROTOCOL_VERSION         1
#define SET_SERVER_PROTOCOL_VERSION(x)      (x = P2P_SERVER_PROTOCOL_VERSION)
#define GET_SERVER_PROTOCOL_VERSION         P2P_SERVER_PROTOCOL_VERSION

/* P2P client protocol version supported by NVR */
#define P2P_CLIENT_PROTOCOL_VERSION         1
#define SET_CLIENT_PROTOCOL_VERSION(x)      (x = P2P_CLIENT_PROTOCOL_VERSION)
#define GET_CLIENT_PROTOCOL_VERSION         P2P_CLIENT_PROTOCOL_VERSION

#define P2P_DEVICE_ID_LEN_MAX               MAX_MAC_ADDRESS_WIDTH
#define P2P_RAW_SESSION_ID_LEN_MAX          18  /* 17 + null */
#define P2P_ENC_SESSION_ID_LEN_MAX          45  /* 44 + null */
#define P2P_TIMESTAMP_LEN_MAX               18  /* YYYY MM YY HH MM SS MMM + null */

#define P2P_CLIENT_VERSION_LEN_MAX          12  /* xxx.yyy.zzz + null */
#define P2P_CLIENT_UID_LEN_MAX              33  /* 32 + null */
#define P2P_CLIENT_MODEL_STR_LEN_MAX        26  /* 25 + null */
#define P2P_CLIENT_CONN_MODE_STR_LEN_MAX    26  /* 25 + null */

#define P2P_DEVICE_RAW_PASS_KEY_LEN_MAX     34  /* 33 + null */
#define P2P_DEVICE_ENC_PASS_KEY_LEN_MAX     65  /* 64 + null */
#define P2P_DEVICE_RAW_TOKEN_LEN_MAX        43  /* 42 + null: (client unique id 32 + unique random num 10 + null) */
#define P2P_DEVICE_ENC_TOKEN_LEN_MAX        65  /* 64 + null */

/* Max relay server supported by NVR */
#define RELAY_SERVER_SUPPORT_MAX             5

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
    /* NVR to P2P Server Msg ID */
    P2P_MSG_ID_REQ_REGISTER                     = 1001,
    P2P_MSG_ID_REQ_HEARTBEAT                    = 1002,
    P2P_MSG_ID_REQ_UNREGISTER                   = 1003,
    P2P_MSG_ID_REQ_STUN_QUERY                   = 1004,
    P2P_MSG_ID_REQ_CONNECTION_DETAILS           = 1005,
    P2P_MSG_ID_REQ_CONNECTION_FAIL              = 1006,
    P2P_MSG_ID_REQ_FALLBACK_TO_RELAY_DETAILS    = 1007,
    P2P_MSG_ID_REQ_FALLBACK_TO_RELAY_FAIL       = 1008,
    P2P_MSG_ID_REQ_MAX,

    /* P2P Server to NVR Msg ID */
    P2P_MSG_ID_RESP_AUTHENTICATION              = 1501,
    P2P_MSG_ID_RESP_REGISTER_SUCCESS            = 1502,
    P2P_MSG_ID_RESP_REGISTER_FAIL               = 1503,
    P2P_MSG_ID_RESP_HEARTBEAT_ACK               = 1504,
    P2P_MSG_ID_RESP_HEARTBEAT_FAIL              = 1505,
    P2P_MSG_ID_RESP_STUN_RESPONSE               = 1506,
    P2P_MSG_ID_RESP_CONNECTION_REQUEST          = 1507,
    P2P_MSG_ID_RESP_CONNECTION_FAIL             = 1508,
    P2P_MSG_ID_RESP_CONNECTION_DETAILS_ACK      = 1509,
    P2P_MSG_ID_RESP_FALLBACK_TO_RELAY           = 1510,
    P2P_MSG_ID_RESP_FALLBACK_TO_RELAY_ACK       = 1511,
    P2P_MSG_ID_RESP_FALLBACK_TO_RELAY_FAIL      = 1512,
    P2P_MSG_ID_RESP_MAX

}P2P_MSG_ID_e;

typedef enum
{
    P2P_DEVICE_TYPE_NVR = 1,
    P2P_DEVICE_TYPE_CAMERA,
    P2P_DEVICE_TYPE_MOBILE_CLIENT,
    P2P_DEVICE_TYPE_DEVICE_CLIENT,

}P2P_DEVICE_TYPE_e;

typedef enum
{
    REG_FAIL_INCOMPLETE_DATA        = 1,
    REG_FAIL_INVLD_DATA             = 2,
    REG_FAIL_DEVICE_NOT_FOUND       = 3,
    REG_FAIL_AUTH_FAIL              = 4,

    CONN_FAIL_INVLD_DEVICE_ID       = 101,
    CONN_FAIL_DEVICE_OFFLINE        = 102,
    CONN_FAIL_INVLD_DATA            = 103,
    CONN_FAIL_CLIENT_NOT_ACTIVE     = 104,
    CONN_FAIL_SESSION_NOT_AVAIL     = 105,
    CONN_FAIL_RESOURCE_NOT_AVAIL    = 106,
    CONN_FAIL_STUN_NOT_RESOLVE      = 107,

    CONN_FAIL_RELAY_FALLBACK_FAIL   = 300,
    CONN_FAIL_ALLOC_QUOTA_REACHED   = 301,
    CONN_FAIL_INSUFFICIENT_CAPACITY = 302,

}P2P_FAIL_REASON_e;

typedef enum
{
    CLIENT_PROC_STAT_RUNNING = 0,
    CLIENT_PROC_STAT_STOP,
    CLIENT_PROC_STAT_EXIT,

}CLIENT_PROC_STAT_e;

typedef struct
{
    INT32   timeInMs;
    UINT32  loopCnt;

}P2P_FSM_TIMER_t;

typedef struct
{
    P2P_MSG_ID_e        msgId;
    CHAR                deviceId[P2P_DEVICE_ID_LEN_MAX];
    CHAR                sessionId[P2P_ENC_SESSION_ID_LEN_MAX];
    CHAR                timestamp[P2P_TIMESTAMP_LEN_MAX];

}P2P_HEADER_t;

typedef struct
{
    CHAR    domain[100];
    UINT16  port;

}DOMAIN_ADDR_INFO_t;

typedef struct
{
    CHAR                version[DEVICE_SW_VERSION_LEN_MAX];
    P2P_DEVICE_TYPE_e   type;
    CHAR                model[P2P_CLIENT_MODEL_STR_LEN_MAX];
    CHAR                uId[P2P_CLIENT_UID_LEN_MAX];
    CHAR                connMode[P2P_CLIENT_CONN_MODE_STR_LEN_MAX];
    UINT8               region;
    BOOL                relayServerFallback;
    IP_ADDR_INFO_t      localAddr;
    IP_ADDR_INFO_t      publicAddr;
    IP_ADDR_INFO_t      peerRelayAddr;
    DOMAIN_ADDR_INFO_t  relayAddr[RELAY_SERVER_SUPPORT_MAX];
    CHAR                p2pToken[P2P_DEVICE_ENC_TOKEN_LEN_MAX];
    CHAR                passkey[P2P_DEVICE_ENC_PASS_KEY_LEN_MAX];
    P2P_FAIL_REASON_e   reason;

}P2P_PAYLOAD_t;

typedef struct
{
    UINT8           protocolVersion;
    P2P_HEADER_t    header;
    P2P_PAYLOAD_t   payload;

}P2P_MSG_INFO_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitP2pProcInfo(void);
//-------------------------------------------------------------------------------------------------
P2P_STATUS_e GetP2pProcessStatus(void);
//-------------------------------------------------------------------------------------------------
NETWORK_PORT_e GetP2pNetworkPort(void);
//-------------------------------------------------------------------------------------------------
const CHAR *GetP2pDeviceId(void);
//-------------------------------------------------------------------------------------------------
void *P2pProcessThreadRoutine(void *threadInfo);
//-------------------------------------------------------------------------------------------------
void SetP2pProcessThreadRunStatus(BOOL runStatus);
//-------------------------------------------------------------------------------------------------
BOOL GetP2pProcessThreadRunStatus(void);
//-------------------------------------------------------------------------------------------------
CLIENT_PROC_STAT_e GetP2pClientProcessRunStatus(UINT8 sessionIdx);
//-------------------------------------------------------------------------------------------------
void SetP2pClientProcessRunStatus(UINT8 sessionIdx, CLIENT_PROC_STAT_e status);
//-------------------------------------------------------------------------------------------------
BOOL EncodeP2pClientSessionId(CHAR *pPlainSessionId, UINT16 clientPort, UINT16 devicePort, CHAR *pTimestamp, CHAR *pEncSessoinId, UINT8 sessionIdLen);
//-------------------------------------------------------------------------------------------------
BOOL DecodeP2pClientSessionId(CHAR *pEncSessoinId, UINT16 clientPort, UINT16 devicePort, CHAR *pTimestamp, CHAR *pPlainSessionId, UINT8 sessionIdLen);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif
