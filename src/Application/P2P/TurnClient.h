#if !defined TURN_CLIENT_H
#define TURN_CLIENT_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		TurnClient.h
@brief      File containing the declaration of turn client interface functions for TCP transport.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "MxTypedef.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
/* Session allocation timeout in TURN server */
#define P2P_TURN_LIFETIME_MAX_IN_SEC    600
#define TURN_ERR_MSG_STR_LEN_MAX        129

/* Username string length: "MAC(16)-ClientUid(32)-Model(25)" */
#define TURN_USERNAME_LEN_MAX           80

/* Password string length: base64 of password */
#define TURN_PASSWORD_LEN_MAX           140 /* (4 * (password len + 2) / 3) + 1 */

/* Realm and nonce string length max */
#define TURN_REALM_LEN_MAX              128
#define TURN_NONCE_LEN_MAX              128

/* TURN server error codes */
#define TURN_EC_ALLOC_QUOTA_REACHED     486
#define TURN_EC_INSUFFICIENT_CAPACITY   508

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
    TURN_STATUS_OK = 0,
    TURN_STATUS_FAIL,
    TURN_STATUS_ERROR,
    TURN_STATUS_CHALLENGE,
    TURN_STATUS_MAX,

}TURN_STATUS_e;

typedef enum
{
    TURN_MSG_TYPE_ALLOCATE = 0,
    TURN_MSG_TYPE_CREATE_PERMISSION,
    TURN_MSG_TYPE_CONNECTION_BIND,
    TURN_MSG_TYPE_REFRESH,
    TURN_MSG_TYPE_CONNECTION_ATTEMPT,
    TURN_MSG_TYPE_MAX,

}TURN_MSG_TYPE_e;

typedef struct
{
    /* Turn send/receive message buffer */
    UINT8           msgBuf[NETWORK_MTU_SIZE_MAX];
    size_t          msgLen;

    /* Message error information */
    INT32           errCode;
    UINT8           errMsg[TURN_ERR_MSG_STR_LEN_MAX];

    /* STUN session information */
    UINT32          lifetime;
    UINT32          connectionId;

    /* Session authentication information */
    UINT8           username[TURN_USERNAME_LEN_MAX];
    UINT8           password[TURN_PASSWORD_LEN_MAX];
    UINT8           nonce[TURN_REALM_LEN_MAX];
    UINT8           realm[TURN_NONCE_LEN_MAX];

    /* Session IP address information */
    IP_ADDR_INFO_t  relayAddr;
    IP_ADDR_INFO_t  mappedAddr;
    IP_ADDR_INFO_t  peerAddr;

}TURN_SESSION_INFO_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
INT32 Turn_GetMsgLen(TURN_SESSION_INFO_t *pTurnSession);
//-------------------------------------------------------------------------------------------------
TURN_STATUS_e Turn_PrepareAllocateReqMsg(TURN_SESSION_INFO_t *pTurnSession);
//-------------------------------------------------------------------------------------------------
TURN_STATUS_e Turn_PrepareCreatePermissionReqMsg(TURN_SESSION_INFO_t *pTurnSession);
//-------------------------------------------------------------------------------------------------
TURN_STATUS_e Turn_PrepareConnectionBindReqMsg(TURN_SESSION_INFO_t *pTurnSession);
//-------------------------------------------------------------------------------------------------
TURN_STATUS_e Turn_PrepareRefreshReqMsg(TURN_SESSION_INFO_t *pTurnSession);
//-------------------------------------------------------------------------------------------------
TURN_STATUS_e Turn_ParseAllocateRespMsg(TURN_SESSION_INFO_t *pTurnSession);
//-------------------------------------------------------------------------------------------------
TURN_STATUS_e Turn_ParseCreatePermissionRespMsg(TURN_SESSION_INFO_t *pTurnSession);
//-------------------------------------------------------------------------------------------------
TURN_STATUS_e Turn_ParseConnectionBindRespMsg(TURN_SESSION_INFO_t *pTurnSession);
//-------------------------------------------------------------------------------------------------
TURN_STATUS_e Turn_ParseConnectionRefreshRespMsg(TURN_SESSION_INFO_t *pTurnSession);
//-------------------------------------------------------------------------------------------------
TURN_STATUS_e Turn_ParseConnectionAttemptIndMsg(TURN_SESSION_INFO_t *pTurnSession);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif
