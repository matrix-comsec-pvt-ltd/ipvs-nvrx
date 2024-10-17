#if !defined P2P_CLIENT_COMM_H
#define P2P_CLIENT_COMM_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		P2pClientComm.h
@brief      File containing the declaration of P2P client communication functions.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "P2pProcessUtils.h"

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
    P2P_MSG_TYPE_SESSION = 0,
    P2P_MSG_TYPE_CONTROL,
    P2P_MSG_TYPE_DATA,
    P2P_MSG_TYPE_MAX

}P2P_MSG_TYPE_e;

typedef enum
{
    P2P_SESSION_MSG_TYPE_HEARTBEAT_REQ  = 0,    /* Client ---> Device */
    P2P_SESSION_MSG_TYPE_HEARTBEAT_RESP,        /* Device ---> Client */
    P2P_SESSION_MSG_TYPE_HANDSHAKE_REQ,         /* Device <--- Client */
    P2P_SESSION_MSG_TYPE_HANDSHAKE_RESP,        /* Device ---> Client */
    P2P_SESSION_MSG_TYPE_HANDSHAKE_ACK,         /* Device <--- Client */
    P2P_SESSION_MSG_TYPE_HOLD_REQ,              /* Client ---> Device */
    P2P_SESSION_MSG_TYPE_HOLD_RESP,             /* Device ---> Client */
    P2P_SESSION_MSG_TYPE_RESUME_REQ,            /* Client ---> Device */
    P2P_SESSION_MSG_TYPE_RESUME_RESP,           /* Device ---> Client */
    P2P_SESSION_MSG_TYPE_CLOSE,                 /* Device <--> Client */
    P2P_SESSION_MSG_TYPE_MAX,

}P2P_SESSION_MSG_TYPE_e;

typedef struct
{
    BOOL                fallbackToRelayServer;
    P2P_FAIL_REASON_e   errorCode;
    UINT8               clientIdx;
    UINT8               userLoginSessionIdx;
    INT32               userLoginSessionUid;
    INT32               sockFd;
    IP_ADDR_INFO_t      localAddr;
    IP_ADDR_INFO_t      publicAddr;

    BOOL                isPeerRelayAddrAllocated;
    IP_ADDR_INFO_t      peerRelayAddr;
    DOMAIN_ADDR_INFO_t  *pRelayServerAddr;

    P2P_DEVICE_TYPE_e   clientType;
    CHAR                clientUid[P2P_CLIENT_UID_LEN_MAX];
    IP_ADDR_INFO_t      clientLocalAddr;
    IP_ADDR_INFO_t      clientPublicAddr;

    CHAR                model[P2P_CLIENT_MODEL_STR_LEN_MAX];
    CHAR                deviceUid[P2P_UNIQUE_RAND_NUM_LEN_MAX];
    CHAR                deviceSessionId[P2P_RAW_SESSION_ID_LEN_MAX];
    CHAR                clientSessionId[P2P_RAW_SESSION_ID_LEN_MAX];

}P2P_CLIENT_INFO_t;

typedef struct __attribute__((__packed__))
{
    UINT32  magicCode;
    UINT8   version;
    UINT8   msgType;
    UINT32  msgUid;
    UINT32  payloadLen;

}P2P_CLIENT_MSG_HEADER_t;

typedef struct __attribute__((__packed__))
{
    UINT8   msgType;
    CHAR    deviceId[P2P_DEVICE_ID_LEN_MAX];
    CHAR    sessionId[P2P_ENC_SESSION_ID_LEN_MAX];
    CHAR    timestamp[P2P_TIMESTAMP_LEN_MAX];

}P2P_CLIENT_SESSION_MSG_t;

typedef struct __attribute__((__packed__))
{
    UINT32  totalMsgLen;
    UINT16  currChunkIdx;

}P2P_CLIENT_CONTROL_MSG_t;

typedef struct __attribute__((__packed__))
{
    UINT32  totalFrameLen;
    UINT16  keyFrameIdx;
    UINT16  subFrameIdx;
    UINT8   cameraId;
    UINT16  currChunkIdx;

}P2P_CLIENT_STREAM_MSG_t;

typedef union
{
    P2P_CLIENT_SESSION_MSG_t    session;
    P2P_CLIENT_CONTROL_MSG_t    control;
    P2P_CLIENT_STREAM_MSG_t     stream;

}P2P_CLIENT_MSG_PAYLOAD_u;

typedef struct __attribute__((__packed__))
{
    P2P_CLIENT_MSG_HEADER_t     header;
    P2P_CLIENT_MSG_PAYLOAD_u    payload;

}P2P_CLIENT_MSG_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitP2pClientCommInfo(void);
//-------------------------------------------------------------------------------------------------
BOOL StartP2pClientCommunication(P2P_CLIENT_INFO_t *pClientInfo);
//-------------------------------------------------------------------------------------------------
BOOL SendP2pClientControlData(INT32 localMsgUid, UINT8 *pSendMsg, UINT32 sendMsgLen, UINT32 timeout);
//-------------------------------------------------------------------------------------------------
BOOL SendP2pClientFrameData(INT32 localMsgUid, UINT8 *pSendMsg, UINT32 sendMsgLen, UINT32 timeout);
//-------------------------------------------------------------------------------------------------
void CloseP2pClientConn(INT32 *pLocalMsgUid);
//-------------------------------------------------------------------------------------------------
BOOL GetP2pClientDataXferFd(P2P_CLIENT_FD_TYPE_e fdType, INT32 localMsgUid, INT32 *pDataRecvFd);
//-------------------------------------------------------------------------------------------------
void CloseP2pClientDataXferFd(P2P_CLIENT_FD_TYPE_e fdType);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif
