//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		TurnClient.c
@brief      File containing the definations of turn client interface functions for TCP transport.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "TurnClient.h"
#include "DebugLog.h"
#include "DeviceInfo.h"

/* Library Includes */
#include "ns_turn_msg.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define TURN_TCP_PORT_ATTRIBUTE     STUN_ATTRIBUTE_TRANSPORT_TCP_VALUE
#define TURN_AUTH_SHA_TYPE          SHATYPE_DEFAULT

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static TURN_STATUS_e GetRespMsgStatus(TURN_SESSION_INFO_t *pTurnSession, BOOL checkChallengeResp);
//-------------------------------------------------------------------------------------------------
static BOOL AddMsgIntegrity(TURN_SESSION_INFO_t *pTurnSession);
//-------------------------------------------------------------------------------------------------
static BOOL AddAttributeValue(TURN_SESSION_INFO_t *pTurnSession, UINT16 attrType);
//-------------------------------------------------------------------------------------------------
static BOOL CheckMsgIntegrity(TURN_SESSION_INFO_t *pTurnSession);
//-------------------------------------------------------------------------------------------------
static BOOL ParseXorAddr(TURN_SESSION_INFO_t *pTurnSession, UINT16 attrType, IP_ADDR_INFO_t *pAddrInfo);
//-------------------------------------------------------------------------------------------------
static BOOL GetAttributeValue(TURN_SESSION_INFO_t *pTurnSession, UINT16 attrType, UINT32 *pAttrVal);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get the TURN message length
 * @param   pTurnSession
 * @return  Returns message length on success otherwise -1
 */
INT32 Turn_GetMsgLen(TURN_SESSION_INFO_t *pTurnSession)
{
    /* Get the stun message length */
    return stun_get_command_message_len_str(pTurnSession->msgBuf, pTurnSession->msgLen);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Prepare TURN allocation request message
 * @param   pTurnSession
 * @return  Returns TURN_STATUS_OK on success otherwise TURN_STATUS_FAIL
 */
TURN_STATUS_e Turn_PrepareAllocateReqMsg(TURN_SESSION_INFO_t *pTurnSession)
{
    /* Prepare turn allocation request with lifetime and only ipv4 family. Don't set remaining attributes */
    if (-1 == stun_set_allocate_request_str(pTurnSession->msgBuf, &pTurnSession->msgLen, P2P_TURN_LIFETIME_MAX_IN_SEC, 1, 0, TURN_TCP_PORT_ATTRIBUTE, 0, NULL, -1))
    {
        /* Fail to prepare allocate message */
        EPRINT(P2P_MODULE, "fail to prepare msg");
        return TURN_STATUS_FAIL;
    }

    /* Add software attribute in the message */
    if (FALSE == AddAttributeValue(pTurnSession, STUN_ATTRIBUTE_SOFTWARE))
    {
        /* Fail to add software attribute in message */
        EPRINT(P2P_MODULE, "fail to add sw attr in msg");
    }

    /* Add authentication information if nonce available */
    if (pTurnSession->nonce[0] == '\0')
    {
        /* It is initial allocate request without authentication */
        return TURN_STATUS_OK;
    }

    /* Add message integrity as authentication */
    if (FALSE == AddMsgIntegrity(pTurnSession))
    {
        /* Fail to add integrity in allocate message */
        EPRINT(P2P_MODULE, "fail to add integrity in msg");
        return TURN_STATUS_FAIL;
    }

    /* Successfully prepared the allocate message */
    return TURN_STATUS_OK;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Prepare TURN create permission request message
 * @param   pTurnSession
 * @return  Returns TURN_STATUS_OK on success otherwise TURN_STATUS_FAIL
 */
TURN_STATUS_e Turn_PrepareCreatePermissionReqMsg(TURN_SESSION_INFO_t *pTurnSession)
{
    ioa_addr peerAddr;

    /* Prepare turn create permission request message */
    stun_init_request_str(STUN_METHOD_CREATE_PERMISSION, pTurnSession->msgBuf, &pTurnSession->msgLen);

    /* Convert string address to network address (We can't predict the port number. Hence used 0) */
    if (-1 == make_ioa_addr((const uint8_t*)pTurnSession->peerAddr.ip, 0, &peerAddr))
    {
        /* Fail to convert peer string addr to network address */
        EPRINT(P2P_MODULE, "fail to convert peer addr");
        return TURN_STATUS_FAIL;
    }

    /* Add XOR peer address attribute in message */
    if (-1 == stun_attr_add_addr_str(pTurnSession->msgBuf, &pTurnSession->msgLen, STUN_ATTRIBUTE_XOR_PEER_ADDRESS, &peerAddr))
    {
        /* Fail to add XOR peer address attribute in message */
        EPRINT(P2P_MODULE, "fail to add peer addr in msg");
        return TURN_STATUS_FAIL;
    }

    /* Add software attribute in the message */
    if (FALSE == AddAttributeValue(pTurnSession, STUN_ATTRIBUTE_SOFTWARE))
    {
        /* Fail to add software attribute in message */
        EPRINT(P2P_MODULE, "fail to add sw attr in msg");
    }

    /* Add message integrity as authentication */
    if (FALSE == AddMsgIntegrity(pTurnSession))
    {
        /* Fail to add integrity in create permission message */
        EPRINT(P2P_MODULE, "fail to add integrity in msg");
        return TURN_STATUS_FAIL;
    }

    /* Successfully prepared the create permission message */
    return TURN_STATUS_OK;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Prepare TURN connection bind request message
 * @param   pTurnSession
 * @return  Returns TURN_STATUS_OK on success otherwise TURN_STATUS_FAIL
 */
TURN_STATUS_e Turn_PrepareConnectionBindReqMsg(TURN_SESSION_INFO_t *pTurnSession)
{
    /* Prepare turn connection bind request message */
    stun_init_request_str(STUN_METHOD_CONNECTION_BIND, pTurnSession->msgBuf, &pTurnSession->msgLen);

    /* Add connection id attribute in message */
    if (FALSE == AddAttributeValue(pTurnSession, STUN_ATTRIBUTE_CONNECTION_ID))
    {
        /* Fail to add connection id in message */
        EPRINT(P2P_MODULE, "fail to add connection id in msg");
        return TURN_STATUS_FAIL;
    }

    /* Add software attribute in the message */
    if (FALSE == AddAttributeValue(pTurnSession, STUN_ATTRIBUTE_SOFTWARE))
    {
        /* Fail to add software attribute in message */
        EPRINT(P2P_MODULE, "fail to add sw attr in msg");
    }

    /* Add message integrity as authentication */
    if (FALSE == AddMsgIntegrity(pTurnSession))
    {
        /* Fail to add integrity in connection bind message */
        EPRINT(P2P_MODULE, "fail to add integrity in msg");
        return TURN_STATUS_FAIL;
    }

    /* Successfully prepared the connection bind message */
    return TURN_STATUS_OK;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Prepare TURN connection refresh request message
 * @param   pTurnSession
 * @return  Returns TURN_STATUS_OK on success otherwise TURN_STATUS_FAIL
 */
TURN_STATUS_e Turn_PrepareRefreshReqMsg(TURN_SESSION_INFO_t *pTurnSession)
{
    /* Prepare turn connection refresh request message */
    stun_init_request_str(STUN_METHOD_REFRESH, pTurnSession->msgBuf, &pTurnSession->msgLen);

    /* Add ip address family type attribute in message */
    if (FALSE == AddAttributeValue(pTurnSession, STUN_ATTRIBUTE_REQUESTED_ADDRESS_FAMILY))
    {
        /* Fail to add ip address family type in message */
        EPRINT(P2P_MODULE, "fail to add ip address family type in msg");
        return TURN_STATUS_FAIL;
    }

    /* Add lifetime attribute in the message */
    if (FALSE == AddAttributeValue(pTurnSession, STUN_ATTRIBUTE_LIFETIME))
    {
        /* Fail to add lifetime attribute in message */
        EPRINT(P2P_MODULE, "fail to add lifetime attr in msg");
        return TURN_STATUS_FAIL;
    }

    /* Add software attribute in the message */
    if (FALSE == AddAttributeValue(pTurnSession, STUN_ATTRIBUTE_SOFTWARE))
    {
        /* Fail to add software attribute in message */
        EPRINT(P2P_MODULE, "fail to add sw attr in msg");
    }

    /* Add message integrity as authentication */
    if (FALSE == AddMsgIntegrity(pTurnSession))
    {
        /* Fail to add integrity in refresh message */
        EPRINT(P2P_MODULE, "fail to add integrity in msg");
        return TURN_STATUS_FAIL;
    }

    /* Successfully prepared the refresh message */
    return TURN_STATUS_OK;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Check and parse TURN allocate response message
 * @param   pTurnSession
 * @return  TURN_STATUS_OK: if successfully parsed response
 *          TURN_STATUS_CHALLENGE: if challenged by the server
 *          TURN_STATUS_ERROR: if error response received
 *          TURN_STATUS_FAIL: other responses
 */
TURN_STATUS_e Turn_ParseAllocateRespMsg(TURN_SESSION_INFO_t *pTurnSession)
{
    /* Get the turn response status */
    TURN_STATUS_e turnStatus = GetRespMsgStatus(pTurnSession, TRUE);
    if (turnStatus != TURN_STATUS_OK)
    {
        /* Success response not received */
        return turnStatus;
    }

    /* Check allocate response message integrity */
    if (FALSE == CheckMsgIntegrity(pTurnSession))
    {
        /* Fail to check message integrity */
        EPRINT(P2P_MODULE, "fail to check msg integrity");
        return TURN_STATUS_FAIL;
    }

    /* Get the relay address from message */
    if (FALSE == ParseXorAddr(pTurnSession, STUN_ATTRIBUTE_XOR_RELAYED_ADDRESS, &pTurnSession->relayAddr))
    {
        /* Fail to parse relay address from message */
        EPRINT(P2P_MODULE, "fail to parse relay address");
        return TURN_STATUS_FAIL;
    }

    /* Get the mapped address from message */
    if (FALSE == ParseXorAddr(pTurnSession, STUN_ATTRIBUTE_XOR_MAPPED_ADDRESS, &pTurnSession->mappedAddr))
    {
        /* Fail to parse mapped address from message */
        EPRINT(P2P_MODULE, "fail to parse mapped address");
        return TURN_STATUS_FAIL;
    }

    /* Get the lifetime value from message attribute */
    if (FALSE == GetAttributeValue(pTurnSession, STUN_ATTRIBUTE_LIFETIME, &pTurnSession->lifetime))
    {
        /* Fail to parse lifetime value */
        EPRINT(P2P_MODULE, "fail to parse lifetime");
        return TURN_STATUS_FAIL;
    }

    /* Successfully parsed allocate message */
    return TURN_STATUS_OK;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Check and parse TURN create permission response message
 * @param   pTurnSession
 * @return  TURN_STATUS_OK: if successfully parsed response
 *          TURN_STATUS_ERROR: if error response received
 *          TURN_STATUS_FAIL: other responses
 */
TURN_STATUS_e Turn_ParseCreatePermissionRespMsg(TURN_SESSION_INFO_t *pTurnSession)
{
    /* Get the turn response status */
    TURN_STATUS_e turnStatus = GetRespMsgStatus(pTurnSession, FALSE);
    if (turnStatus != TURN_STATUS_OK)
    {
        /* Success response not received */
        return turnStatus;
    }

    /* Check create permission response message integrity */
    if (FALSE == CheckMsgIntegrity(pTurnSession))
    {
        /* Fail to check message integrity */
        EPRINT(P2P_MODULE, "fail to check msg integrity");
        return TURN_STATUS_FAIL;
    }

    /* Successfully parsed create permission message */
    return TURN_STATUS_OK;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Check and parse TURN connection bind response message
 * @param   pTurnSession
 * @return  TURN_STATUS_OK: if successfully parsed response
 *          TURN_STATUS_ERROR: if error response received
 *          TURN_STATUS_FAIL: other responses
 */
TURN_STATUS_e Turn_ParseConnectionBindRespMsg(TURN_SESSION_INFO_t *pTurnSession)
{
    /* Get the turn response status */
    TURN_STATUS_e turnStatus = GetRespMsgStatus(pTurnSession, FALSE);
    if (turnStatus != TURN_STATUS_OK)
    {
        /* Success response not received */
        return turnStatus;
    }

    /* Check connection bind response message integrity */
    if (FALSE == CheckMsgIntegrity(pTurnSession))
    {
        /* Fail to check message integrity */
        EPRINT(P2P_MODULE, "fail to check msg integrity");
        return TURN_STATUS_FAIL;
    }

    /* Successfully parsed connection bind message */
    return TURN_STATUS_OK;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Check and parse TURN connection refresh response message
 * @param   pTurnSession
 * @return  TURN_STATUS_OK: if successfully parsed response
 *          TURN_STATUS_CHALLENGE: if challenged by the server
 *          TURN_STATUS_ERROR: if error response received
 *          TURN_STATUS_FAIL: other responses
 */
TURN_STATUS_e Turn_ParseConnectionRefreshRespMsg(TURN_SESSION_INFO_t *pTurnSession)
{
    /* Get the turn response status */
    TURN_STATUS_e turnStatus = GetRespMsgStatus(pTurnSession, TRUE);
    if (turnStatus != TURN_STATUS_OK)
    {
        /* Success response not received */
        return turnStatus;
    }

    /* Check connection bind response message integrity */
    if (FALSE == CheckMsgIntegrity(pTurnSession))
    {
        /* Fail to check message integrity */
        EPRINT(P2P_MODULE, "fail to check msg integrity");
        return TURN_STATUS_FAIL;
    }

    /* Get the lifetime value from message attribute */
    if (FALSE == GetAttributeValue(pTurnSession, STUN_ATTRIBUTE_LIFETIME, &pTurnSession->lifetime))
    {
        /* Fail to parse lifetime value */
        EPRINT(P2P_MODULE, "fail to parse lifetime");
        return TURN_STATUS_FAIL;
    }

    /* Successfully parsed connection bind message */
    return TURN_STATUS_OK;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Check and parse TURN connection attempt indication message
 * @param   pTurnSession
 * @return  Returns TURN_STATUS_OK if indication message parsed otherwise TURN_STATUS_FAIL
 */
TURN_STATUS_e Turn_ParseConnectionAttemptIndMsg(TURN_SESSION_INFO_t *pTurnSession)
{
    uint16_t method;

    /* Is it indication message? */
    if (0 == stun_is_indication_str(pTurnSession->msgBuf, pTurnSession->msgLen))
    {
        /* It is not indication message */
        EPRINT(P2P_MODULE, "it is not indication msg");
        return TURN_STATUS_FAIL;
    }

    /* Is it connection attempt indication? */
    method = stun_get_method_str(pTurnSession->msgBuf, pTurnSession->msgLen);
    if (method != STUN_METHOD_CONNECTION_ATTEMPT)
    {
        /* Connection attempt method not found */
        EPRINT(P2P_MODULE, "connection attempt method not found: [method=0x%x]", method);
        return TURN_STATUS_FAIL;
    }

    /* Get the peer address from message */
    if (FALSE == ParseXorAddr(pTurnSession, STUN_ATTRIBUTE_XOR_PEER_ADDRESS, &pTurnSession->peerAddr))
    {
        /* Fail to parse peer address from message */
        EPRINT(P2P_MODULE, "fail to parse peer address");
        return TURN_STATUS_FAIL;
    }

    /* Get the connection id value from message attribute */
    if (FALSE == GetAttributeValue(pTurnSession, STUN_ATTRIBUTE_CONNECTION_ID, &pTurnSession->connectionId))
    {
        /* Fail to parse connection id value */
        EPRINT(P2P_MODULE, "fail to parse connection id");
        return TURN_STATUS_FAIL;
    }

    /* Successfully parsed connection attempt message */
    return TURN_STATUS_OK;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Check response is success or not
 * @param   pTurnSession
 * @param   checkChallengeResp
 * @return  TURN_STATUS_OK: if success response
 *          TURN_STATUS_CHALLENGE: if challenged by the server
 *          TURN_STATUS_ERROR: if error response received
 *          TURN_STATUS_FAIL: other responses
 */
static TURN_STATUS_e GetRespMsgStatus(TURN_SESSION_INFO_t *pTurnSession, BOOL checkChallengeResp)
{
    /* Is it success response message? */
    if (TRUE == stun_is_success_response_str(pTurnSession->msgBuf, pTurnSession->msgLen))
    {
        /* It is success response */
        return TURN_STATUS_OK;
    }

    /* Do we have to check challenge response whether it is challenge response or not? */
    if (TRUE == checkChallengeResp)
    {
        /* Is it challenge response message? */
        if (TRUE == stun_is_challenge_response_str(pTurnSession->msgBuf, pTurnSession->msgLen, &pTurnSession->errCode, pTurnSession->errMsg,
                                                   TURN_ERR_MSG_STR_LEN_MAX, pTurnSession->realm, pTurnSession->nonce, NULL, NULL))
        {
            /* It is challenge response */
            return TURN_STATUS_CHALLENGE;
        }
    }

    /* Is it error response message? */
    if (TRUE == stun_is_error_response_str(pTurnSession->msgBuf, pTurnSession->msgLen, &pTurnSession->errCode, pTurnSession->errMsg, TURN_ERR_MSG_STR_LEN_MAX))
    {
        /* It is error response */
        return TURN_STATUS_ERROR;
    }

    /* Unknown response received */
    pTurnSession->errCode = 0;
    snprintf((CHAR*)pTurnSession->errMsg, TURN_ERR_MSG_STR_LEN_MAX, "unknown");
    return TURN_STATUS_FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Add TURN message integrity in request for authentication
 * @param   pTurnSession
 * @return  Returns TRUE on success otherwise FALSE
 */
static BOOL AddMsgIntegrity(TURN_SESSION_INFO_t *pTurnSession)
{
    /* Generate message integrity from auth para and add into message */
    if (-1 == stun_attr_add_integrity_by_user_str(pTurnSession->msgBuf, &pTurnSession->msgLen, pTurnSession->username,
                                                  pTurnSession->realm, pTurnSession->password, pTurnSession->nonce, TURN_AUTH_SHA_TYPE))
    {
        /* Fail to generate message with integrity */
        return FALSE;
    }

    /* Successfully added auth info in the message */
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Add attribute value in the TURN message
 * @param   pTurnSession
 * @param   attrType
 * @return  Returns TRUE on success otherwise FALSE
 */
static BOOL AddAttributeValue(TURN_SESSION_INFO_t *pTurnSession, UINT16 attrType)
{
    UINT32          attrLen;
    uint32_t        attrVal;
    const uint8_t   *pAttrVal;

    /* Check attribute type */
    switch(attrType)
    {
        /* Prepare ip address family attribute data */
        case STUN_ATTRIBUTE_REQUESTED_ADDRESS_FAMILY:
        {
            attrVal = STUN_ATTRIBUTE_REQUESTED_ADDRESS_FAMILY_VALUE_IPV4;
            pAttrVal = (const uint8_t*)&attrVal;
            attrLen = 4;
        }
        break;

        /* Prepare allocation lifetime attribute data */
        case STUN_ATTRIBUTE_LIFETIME:
        {
            attrVal = nswap32(pTurnSession->lifetime);
            pAttrVal = (const uint8_t*)&attrVal;
            attrLen = 4;
        }
        break;

        /* Prepare connection id attribute data */
        case STUN_ATTRIBUTE_CONNECTION_ID:
        {
            attrVal = nswap32(pTurnSession->connectionId);
            pAttrVal = (const uint8_t*)&attrVal;
            attrLen = 4;
        }
        break;

        /* Prepare software attribute data */
        case STUN_ATTRIBUTE_SOFTWARE:
        {
            pAttrVal = (const uint8_t *)GetNvrDeviceInfoStr();
            attrLen = strlen((const CHAR *)pAttrVal);
        }
        break;

        /* Invalid attribute type */
        default:
        {
            /* Nothing to do */
        }
        return FALSE;
    }

    /* Add software attribute in the TURN message */
    if (-1 == stun_attr_add_str(pTurnSession->msgBuf, &pTurnSession->msgLen, attrType, pAttrVal, attrLen))
    {
        /* Fail to add attribute in the message */
        return FALSE;
    }

    /* Attribute added successfully in the message */
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Check TURN response message integrity
 * @param   pTurnSession
 * @return  Returns TRUE if integrity check ok otherwise FALSE
 */
static BOOL CheckMsgIntegrity(TURN_SESSION_INFO_t *pTurnSession)
{
    /* Check the message integrity */
    if (1 != stun_check_message_integrity_str(TURN_CREDENTIALS_LONG_TERM, pTurnSession->msgBuf, pTurnSession->msgLen,
                                              pTurnSession->username, pTurnSession->realm, pTurnSession->password, TURN_AUTH_SHA_TYPE))
    {
        EPRINT(P2P_MODULE, "message integrity check fail");
        return FALSE;
    }

    /* Message integrity is OK */
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Parse address from XOR format to string format
 * @param   pTurnSession
 * @param   attrType
 * @param   pAddrInfo
 * @return  Returns TRUE on success otherwise FALSE
 */
static BOOL ParseXorAddr(TURN_SESSION_INFO_t *pTurnSession, UINT16 attrType, IP_ADDR_INFO_t *pAddrInfo)
{
    ioa_addr        sockAddr;
    stun_attr_ref   pStunAttr;

    /* Get the XOR address attribute from message */
    pStunAttr = stun_attr_get_first_by_type_str(pTurnSession->msgBuf, pTurnSession->msgLen, attrType);
    if (pStunAttr == NULL)
    {
        /* XOR address attributes not available */
        EPRINT(P2P_MODULE, "xor addr not available");
        return FALSE;
    }

    /* Get XOR address to network format address */
    if (-1 == stun_attr_get_addr_str(pTurnSession->msgBuf, pTurnSession->msgLen, pStunAttr, &sockAddr, NULL))
    {
        /* Fail to get XOR address from attribute */
        EPRINT(P2P_MODULE, "fail to get xor addr from attr");
        return FALSE;
    }

    /* Address must be IPv4 only */
    if (sockAddr.ss.sa_family != AF_INET)
    {
        /* Invalid address received */
        EPRINT(P2P_MODULE, "invld ip addr type: [type=%d]", sockAddr.ss.sa_family);
        return FALSE;
    }

    /* Convert network address to string address */
    if (NULL == inet_ntop(AF_INET, &sockAddr.s4.sin_addr, pAddrInfo->ip, sizeof(pAddrInfo->ip)))
    {
        /* Fail to convert network address to string address */
        EPRINT(P2P_MODULE, "fail to convert nw addr to str: [type=%d], [addr=0x%x]", sockAddr.ss.sa_family, sockAddr.s4.sin_addr.s_addr);
        return FALSE;
    }

    /* Convert port and store it */
    pAddrInfo->port = nswap16(sockAddr.s4.sin_port);

    /* Successfully parsed XOR address */
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get attribute value from message
 * @param   pTurnSession
 * @param   attrType
 * @param   pAttrVal
 * @return  Returns TRUE on success otherwise FALSE
 */
static BOOL GetAttributeValue(TURN_SESSION_INFO_t *pTurnSession, UINT16 attrType, UINT32 *pAttrVal)
{
    const uint8_t   *pAttrValue;
    stun_attr_ref   pStunAttr;

    /* Get attribute from message */
    pStunAttr = stun_attr_get_first_by_type_str(pTurnSession->msgBuf, pTurnSession->msgLen, attrType);
    if (pStunAttr == NULL)
    {
        /* Attributes not available */
        EPRINT(P2P_MODULE, "attr not available");
        return FALSE;
    }

    /* Get the attribute length. It must be 4 */
    if (stun_attr_get_len(pStunAttr) != 4)
    {
        /* Invalid attribute field format */
        EPRINT(P2P_MODULE, "invld attr field format");
        return FALSE;
    }

    /* Get the attribute value */
    pAttrValue = stun_attr_get_value(pStunAttr);
    if (pAttrValue == NULL)
    {
        /* Invalid attribute field data */
        EPRINT(P2P_MODULE, "invld attr field data");
        return FALSE;
    }

    /* Convert attribute value and store it for future reference */
    *pAttrVal = nswap32(*((const uint32_t*)pAttrValue));

    /* Successfully parsed attribute value */
    return TRUE;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
