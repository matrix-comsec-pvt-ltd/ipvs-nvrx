//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		P2pProcessUtils.c
@brief      File containing the defination of helper functions for P2P process
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <sys/ioctl.h>

/* Library Includes */
#include <jansson.h>

/* Application Includes */
#include "P2pProcessUtils.h"
#include "DebugLog.h"
#include "DateTime.h"
#include "UtilCommon.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define P2P_PROTOCOL_VERSION_TAG        "version"

#define P2P_MSG_HEADER_TAG              "header"
#define P2P_MSG_PAYLOAD_TAG             "payload"

#define P2P_MSG_ID_TAG                  "msgid"
#define P2P_DEVICE_ID_TAG               "device"
#define P2P_SESSION_ID_TAG              "session"
#define P2P_TIME_STAMP_TAG              "timestamp"

#define P2P_DEVICE_VERSION_TAG          "d_firmware"
#define P2P_DEVICE_TYPE_TAG             "d_type"
#define P2P_DEVICE_MODEL_TAG            "d_model"
#define P2P_DEVICE_PASSKEY_TAG          "passkey"
#define P2P_DEVICE_LOCAL_IP_TAG         "d_localip"
#define P2P_DEVICE_LOCAL_PORT_TAG       "d_localport"
#define P2P_DEVICE_PUBLIC_IP_TAG        "d_publicip"
#define P2P_DEVICE_PUBLIC_PORT_TAG      "d_publicport"
#define P2P_DEVICE_TOKEN_TAG            "d_token"
#define P2P_DEVICE_REGION_TAG           "d_region"
#define P2P_DEVICE_FALLBACK_TAG         "d_fallback"

#define P2P_CLIENT_VERSION_TAG          "c_firmware"
#define P2P_CLIENT_TYPE_TAG             "c_type"
#define P2P_CLIENT_UID_TAG              "c_uid"
#define P2P_CLIENT_LOCAL_IP_TAG         "c_localip"
#define P2P_CLIENT_LOCAL_PORT_TAG       "c_localport"
#define P2P_CLIENT_PUBLIC_IP_TAG        "c_publicip"
#define P2P_CLIENT_PUBLIC_PORT_TAG      "c_publicport"
#define P2P_CLIENT_MODEL_TAG            "c_model"
#define P2P_CLIENT_CONN_MODE_TAG        "c_connmode"

#define P2P_RELAY_SERVER_TAG            "relayserver"
#define P2P_RELAY_SERVER_ADDR_TAG       "r_ipaddr"
#define P2P_RELAY_SERVER_PORT_TAG       "r_port"

#define P2P_FAIL_REASON_TAG             "reason"

#define JSON_SET_OBJECT(obj, tag, val)  json_object_set_new(obj, tag, val);
#define JSON_SET_INTEGER(obj, tag, val) json_object_set_new(obj, tag, json_integer(val));
#define JSON_SET_STRING(obj, tag, val)  json_object_set_new(obj, tag, json_string(val));

#define JSON_GET_OBJECT(root, obj, tag)                                     \
    {                                                                       \
        obj = json_object_get(root, tag);                                   \
        if (!json_is_object(obj))                                           \
        {                                                                   \
            EPRINT(P2P_MODULE, "fail to get json tag: [tag=%s]", tag);      \
            json_decref(root);                                              \
            return FALSE;                                                   \
        }                                                                   \
    }

#define JSON_GET_INTEGER(root, obj, tag, val)                               \
    {                                                                       \
        json_t *json;                                                       \
        json = json_object_get(obj, tag);                                   \
        if (!json_is_integer(json))                                         \
        {                                                                   \
            EPRINT(P2P_MODULE, "fail to get json tag: [tag=%s]", tag);      \
            json_decref(root);                                              \
            return FALSE;                                                   \
        }                                                                   \
        val = json_integer_value(json);                                     \
    }

/* Tag may be missing */
#define JSON_GET_STRING3(root, obj, tag, val)                               \
    {                                                                       \
        json_t *json;                                                       \
        json = json_object_get(obj, tag);                                   \
        if (json_is_string(json))                                           \
        {                                                                   \
            snprintf(val, sizeof(val), "%s", json_string_value(json));      \
        }                                                                   \
    }


/* Output string may empty but tag must be there */
#define JSON_GET_STRING2(root, obj, tag, val)                               \
    {                                                                       \
        json_t *json;                                                       \
        json = json_object_get(obj, tag);                                   \
        if (!json_is_string(json))                                          \
        {                                                                   \
            EPRINT(P2P_MODULE, "fail to get json tag: [tag=%s]", tag);      \
            json_decref(root);                                              \
            return FALSE;                                                   \
        }                                                                   \
        snprintf(val, sizeof(val), "%s", json_string_value(json));          \
    }

/* Output string must not be null */
#define JSON_GET_STRING(root, obj, tag, val)                                \
    {                                                                       \
        JSON_GET_STRING2(root, obj, tag, val)                               \
        if (val[0] == '\0')                                                 \
        {                                                                   \
            EPRINT(P2P_MODULE, "fail to json data: [tag=%s]", tag);         \
            json_decref(root);                                              \
            return FALSE;                                                   \
        }                                                                   \
    }

#define JSON_IS_ARRAY(obj, tag)     json_is_array(json_object_get(obj, tag))

#define JSON_GET_ARRAY(root, obj, tag, val1, val2)                          \
    {                                                                       \
        val1 = json_object_get(obj, tag);                                   \
        if (!json_is_array(val1))                                           \
        {                                                                   \
            EPRINT(P2P_MODULE, "fail to get json array: [tag=%s]", tag);    \
            json_decref(root);                                              \
            return FALSE;                                                   \
        }                                                                   \
        val2 = json_array_size(val1);                                       \
    }

#define JSON_GET_ARRAY_OBJECT(root, obj, idx, val)                          \
    {                                                                       \
        val = json_array_get(obj, idx);                                     \
        if (!json_is_object(val))                                           \
        {                                                                   \
            EPRINT(P2P_MODULE, "fail to get json array: [index=%d]", idx);  \
            json_decref(root);                                              \
            return FALSE;                                                   \
        }                                                                   \
    }

/* Define id not available in header */
#ifndef SO_REUSEPORT
#define SO_REUSEPORT	15
#endif

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static BOOL isTmrInList(SYS_TIMER_t *pHead, SYS_TIMER_t *pTmr);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   initialize system timer library
 * @param   pHead - Timer list head
 */
void SysTimerInit(SYS_TIMER_t **pHead)
{
    *pHead = NULL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set Callback funtion to be called when timer expires
 * @param   pTmr - Timer handle
 * @param   timerTickInMs - time interval at which tick will be provided to module
 * @param   callBack - Callback routine
 * @return  TRUE if init successfully, FALSE otherwise
 */
BOOL SysTimerConfigure(SYS_TIMER_t *pTmr, UINT32 timerTickInMs, SYS_TIMER_CB callBack)
{
    if ((0 == timerTickInMs) || (NULL == callBack))
    {
        return FALSE;
    }

    pTmr->timerTickInMs = timerTickInMs;
    pTmr->callBack = callBack;
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Insert timer in the timer list
 * @param   pHead - Timer List Head
 * @param   pTmr - Timer Handle
 * @param   appData1 - Application Data
 * @param   appData2 - Application Data
 * @param   intervalInMs - Time in millisec after which timer expires
 * @param   loopCnt - Timer will be executed number of times (0 = continues, others = no. of times)
 * @return  TRUE if starts successfully, FALSE otherwise
 * @note    It timer is already inserted in the list, it will not be inserted again but number of ticks will get overwrite
 */
BOOL SysTimerStart(SYS_TIMER_t **pHead, SYS_TIMER_t *pTmr, UINT32 appData1, UINT32 appData2, UINT32 intervalInMs, UINT16 loopCnt)
{
    /* Invalid timer tick is configured */
    if (0 == pTmr->timerTickInMs)
    {
        return (FALSE);
    }

    /* Calculate tick based on base tick */
    pTmr->ticks = (intervalInMs/pTmr->timerTickInMs);
    if (0 == pTmr->ticks)
    {
        return (FALSE);
    }

    /* Set reload timer. It will be reloaded after timer expired */
    pTmr->reloadTimerTicks = pTmr->ticks;

    /* If loop count is 0 means it is cyclic timer which runs continuesly */
    if (0 == loopCnt)
    {
        /* Se timer as continues */
        pTmr->loopCnt = -1;
    }
    else
    {
        /* Se timer loop counts */
        pTmr->loopCnt = loopCnt;
    }

    /* Store application data */
    pTmr->appData1 = appData1;
    pTmr->appData2 = appData2;

    if (*pHead == NULL)
    {
        *pHead = pTmr;
        pTmr->pNext = NULL;
        return (TRUE);
    }

    /* Is timer already present in list? */
    if (isTmrInList(*pHead, pTmr) == TRUE)
    {
        /* Yes it is */
        return (TRUE);
    }

    /* Add timer in the list */
    pTmr->pNext = *pHead;
    *pHead = pTmr;
    return (TRUE);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Remove timer from the lists
 * @param   pHead - Timer List Head
 * @param   pTmr - Timer handle
 */
void SysTimerStop(SYS_TIMER_t **pHead, SYS_TIMER_t *pTmr)
{
    if (*pHead == NULL)
    {
        return;
    }

    if (*pHead == pTmr)
    {
        *pHead = (*pHead)->pNext;
        pTmr->pNext = NULL;
        return;
    }

    SYS_TIMER_t *pPrevNode = *pHead;
    SYS_TIMER_t *pCurNode = (*pHead)->pNext;
    while (pCurNode != NULL)
    {
        if (pCurNode == pTmr)
        {
            pPrevNode->pNext = pCurNode->pNext;
            return;
        }

        pPrevNode = pCurNode;
        pCurNode = pCurNode->pNext;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Decrement number of ticks of each timer in the list
 * @param   pHead - Timer List Head
 */
void SysTimerTick(SYS_TIMER_t **pHead)
{
    if (*pHead == NULL)
    {
        return;
    }

    SYS_TIMER_t *pCurNode = *pHead;
    while (pCurNode != NULL)
    {
        pCurNode->ticks--;
        if (pCurNode->ticks)
        {
            pCurNode = pCurNode->pNext;
            continue;
        }

        /* Update timer counter for loop timers */
        if (pCurNode->loopCnt > 0)
        {
            /* Update loop counts */
            pCurNode->loopCnt--;
        }

        /* If loop count is zero means timer is expired */
        if (pCurNode->loopCnt == 0)
        {
            SYS_TIMER_t *pExpireNode = pCurNode;
            pCurNode = pCurNode->pNext;

            /* Stop single shot timer */
            SysTimerStop(pHead, pExpireNode);

            /* Execute timer expiry callback */
            (*pExpireNode->callBack)(pExpireNode->appData1, pExpireNode->appData2, TRUE);
        }
        else
        {
            /* Reload timer for loop */
            pCurNode->ticks = pCurNode->reloadTimerTicks;

            /* Execute timer expiry callback */
            (*pCurNode->callBack)(pCurNode->appData1, pCurNode->appData2, FALSE);

            /* Check next node */
            pCurNode = pCurNode->pNext;
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Wait for next tick to execute tasks
 * @param   pRefTime - Reference time to derive next tick
 * @param   tickInNs - Tick in nano seconds
 * @param   overflowTime - Allowed max delay
 * @note    pRefTime must be init with 0 for first time function calling
 */
void WaitNextTick(UINT8 clientIdx, UINT64 *pRefTime, UINT64 tickInNs, UINT32 overflowTime)
{
    INT64   tTimeDiff;
    UINT64  tEndTime;

    /* For first tick, reference time will be 0 */
    if (*pRefTime == 0)
    {
        /* Get current time in nano seconds for reference */
        *pRefTime = GetMonotonicTimeInNanoSec();
        return;
    }

    /* Add the nanoseconds of timer tick resolution to derive the Sleep */
    *pRefTime += tickInNs;

    /* Get current time in nano seconds */
    tEndTime = GetMonotonicTimeInNanoSec();

    /* Get the time difference with previous */
    tTimeDiff = *pRefTime - tEndTime;
    if (tTimeDiff > 0)
    {
        /* We have time to wait for next tick */
        SleepNanoSec(tTimeDiff);
        return;
    }

    /* Convert difference into positive */
    tTimeDiff = ((-tTimeDiff)/NANO_SEC_PER_MILLI_SEC);
    if (tTimeDiff <= 0)
    {
        return;
    }

    /* Add warning only if we're delayed by minimum interval. */
    if (tTimeDiff > overflowTime)
    {
        WPRINT(P2P_MODULE, "p2p thread is running late by [%lld]ms: [client=%d]", tTimeDiff, clientIdx);
    }

    /* Provide half of base tick and get latest time */
    SleepNanoSec(tickInNs/2);
    *pRefTime = GetMonotonicTimeInNanoSec();
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Check if timer is in the timer list or not
 * @param   pHead - Timer list head
 * @param   pTmr - Timer handle
 * @return  TRUE if timer is in the list, FALSE otherwise
 */
static BOOL isTmrInList(SYS_TIMER_t *pHead, SYS_TIMER_t *pTmr)
{
    if (pHead == NULL)
    {
        return (FALSE);
    }

    if (pHead == pTmr)
    {
        return (TRUE);
    }

    SYS_TIMER_t *pNextNode = pHead->pNext;
    while (pNextNode != NULL)
    {
        if (pNextNode == pTmr)
        {
            return (TRUE);
        }
        pNextNode = pNextNode->pNext;
    }

    return (FALSE);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   P2P message information to convert in json format
 * @param   pJsonInfo - Input information for json msg generation
 * @param   pJsonMsg - Output msg in json format
 * @param   jsonBufLen - Output json buffer length max
 * @return  Returns gnerated json message length
 */
UINT16 PrepareJsonMsg(P2P_MSG_INFO_t *pJsonInfo, CHAR *pJsonMsg, UINT16 jsonBufLen)
{
    json_t  *root;
    json_t  *header;
    json_t  *payload;
    UINT16  jsonMsgLen;

    /* Create root json object */
    root = json_object();
    if (root == NULL)
    {
        EPRINT(P2P_MODULE, "fail to create json root tag");
        return 0;
    }

    /* Create header json object */
    header = json_object();
    if (header == NULL)
    {
        EPRINT(P2P_MODULE, "fail to create json header tag");
        json_decref(root);
        return 0;
    }

    /* Create payload json object */
    payload = json_object();
    if (payload == NULL)
    {
        EPRINT(P2P_MODULE, "fail to create json payload tag");
        json_decref(header);
        json_decref(root);
        return 0;
    }

    /* Add protocol version in root object */
    JSON_SET_INTEGER(root, P2P_PROTOCOL_VERSION_TAG, pJsonInfo->protocolVersion);

    /* Add header info in header object */
    JSON_SET_INTEGER(header, P2P_MSG_ID_TAG, pJsonInfo->header.msgId);
    JSON_SET_STRING(header, P2P_DEVICE_ID_TAG, pJsonInfo->header.deviceId);
    JSON_SET_STRING(header, P2P_SESSION_ID_TAG, pJsonInfo->header.sessionId);
    JSON_SET_STRING(header, P2P_TIME_STAMP_TAG, pJsonInfo->header.timestamp);

    /* Prepare payload based on id */
    switch(pJsonInfo->header.msgId)
    {
        case P2P_MSG_ID_REQ_REGISTER:
        {
            /* Add payload info in payload object */
            JSON_SET_STRING(payload, P2P_DEVICE_VERSION_TAG, pJsonInfo->payload.version);
            JSON_SET_INTEGER(payload, P2P_DEVICE_TYPE_TAG, pJsonInfo->payload.type);
            JSON_SET_STRING(payload, P2P_DEVICE_MODEL_TAG, pJsonInfo->payload.model);
            JSON_SET_STRING(payload, P2P_DEVICE_PASSKEY_TAG, pJsonInfo->payload.passkey);
            JSON_SET_STRING(payload, P2P_DEVICE_LOCAL_IP_TAG, pJsonInfo->payload.localAddr.ip);
            JSON_SET_INTEGER(payload, P2P_DEVICE_LOCAL_PORT_TAG, pJsonInfo->payload.localAddr.port);
            JSON_SET_INTEGER(payload, P2P_DEVICE_REGION_TAG, pJsonInfo->payload.region);
        }
        break;

        case P2P_MSG_ID_REQ_CONNECTION_DETAILS:
        {
            /* Add payload info in payload object */
            JSON_SET_STRING(payload, P2P_CLIENT_UID_TAG, pJsonInfo->payload.uId);
            JSON_SET_STRING(payload, P2P_DEVICE_TOKEN_TAG, pJsonInfo->payload.p2pToken);
            JSON_SET_STRING(payload, P2P_DEVICE_LOCAL_IP_TAG, pJsonInfo->payload.localAddr.ip);
            JSON_SET_INTEGER(payload, P2P_DEVICE_LOCAL_PORT_TAG, pJsonInfo->payload.localAddr.port);
            JSON_SET_STRING(payload, P2P_DEVICE_PUBLIC_IP_TAG, pJsonInfo->payload.publicAddr.ip);
            JSON_SET_INTEGER(payload, P2P_DEVICE_PUBLIC_PORT_TAG, pJsonInfo->payload.publicAddr.port);
            JSON_SET_INTEGER(payload, P2P_DEVICE_FALLBACK_TAG, pJsonInfo->payload.relayServerFallback);
        }
        break;

        case P2P_MSG_ID_REQ_CONNECTION_FAIL:
        case P2P_MSG_ID_REQ_FALLBACK_TO_RELAY_FAIL:
        {
            /* Add payload info in payload object */
            JSON_SET_STRING(payload, P2P_CLIENT_UID_TAG, pJsonInfo->payload.uId);
            JSON_SET_INTEGER(payload, P2P_FAIL_REASON_TAG, pJsonInfo->payload.reason);
        }
        break;

        case P2P_MSG_ID_REQ_FALLBACK_TO_RELAY_DETAILS:
        {
            /* Add payload info in payload object */
            JSON_SET_STRING(payload, P2P_CLIENT_UID_TAG, pJsonInfo->payload.uId);
            JSON_SET_STRING(payload, P2P_DEVICE_TOKEN_TAG, pJsonInfo->payload.p2pToken);
            JSON_SET_STRING(payload, P2P_RELAY_SERVER_ADDR_TAG, pJsonInfo->payload.peerRelayAddr.ip);
            JSON_SET_INTEGER(payload, P2P_RELAY_SERVER_PORT_TAG, pJsonInfo->payload.peerRelayAddr.port);
        }
        break;

        case P2P_MSG_ID_REQ_HEARTBEAT:
        case P2P_MSG_ID_REQ_UNREGISTER:
        case P2P_MSG_ID_REQ_STUN_QUERY:
        {
            /* No payload */
        }
        break;

        default:
        {
            /* Free all objects */
            json_decref(header);
            json_decref(payload);
            json_decref(root);
            EPRINT(P2P_MODULE, "invld msg id in p2p msg send: [msgId=%d]", pJsonInfo->header.msgId);
        }
        return 0;
    }

    /* Add header and payload object in root object */
    JSON_SET_OBJECT(root, P2P_MSG_HEADER_TAG, header);
    JSON_SET_OBJECT(root, P2P_MSG_PAYLOAD_TAG, payload);

    /* Get json string */
    jsonMsgLen = json_dumpb(root, pJsonMsg, jsonBufLen, 0);

    /* Free root object */
    json_decref(root);

    /* Return Json to string */
    return jsonMsgLen;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Decode JSON message to proprietary structure
 * @param   pJsonMsg
 * @param   pJsonInfo
 * @return  TRUE on success; FALSE otherwise
 */
BOOL DecodeJsonMsg(CHAR *pJsonMsg, P2P_MSG_INFO_t *pJsonInfo)
{
    json_t          *root;
    json_t          *header;
    json_t          *payload;
    json_error_t    jsonErr;

    /* Load json msg */
    root = json_loads(pJsonMsg, 0, &jsonErr);
    if (root == NULL)
    {
        EPRINT(P2P_MODULE, "fail to load json msg: [line=%d], [err=%s]", jsonErr.line, jsonErr.text);
        return FALSE;
    }

    /* Get protocol version */
    JSON_GET_INTEGER(root, root, P2P_PROTOCOL_VERSION_TAG, pJsonInfo->protocolVersion);

    /* Validate protocol version support */
    if (pJsonInfo->protocolVersion != GET_SERVER_PROTOCOL_VERSION)
    {
        EPRINT(P2P_MODULE, "unsupported protocol version: [support=%d], [recv=%d]", GET_SERVER_PROTOCOL_VERSION, pJsonInfo->protocolVersion);
        json_decref(root);
        return FALSE;
    }

    /* Get P2P header tag object */
    JSON_GET_OBJECT(root, header, P2P_MSG_HEADER_TAG);

    /* Get P2P payload tag object */
    JSON_GET_OBJECT(root, payload, P2P_MSG_PAYLOAD_TAG);

    /* Get header information */
    JSON_GET_INTEGER(root, header, P2P_MSG_ID_TAG, pJsonInfo->header.msgId);
    JSON_GET_STRING(root, header, P2P_DEVICE_ID_TAG, pJsonInfo->header.deviceId);

    /* Session ID is not mandatory in STUN response */
    if (pJsonInfo->header.msgId == P2P_MSG_ID_RESP_STUN_RESPONSE)
    {
        /* Session id is not mandatory */
        JSON_GET_STRING2(root, header, P2P_SESSION_ID_TAG, pJsonInfo->header.sessionId);
    }
    else
    {
        /* Session id is mandatory */
        JSON_GET_STRING(root, header, P2P_SESSION_ID_TAG, pJsonInfo->header.sessionId);
    }

    JSON_GET_STRING(root, header, P2P_TIME_STAMP_TAG, pJsonInfo->header.timestamp);

    /* Prepare payload based on id */
    switch(pJsonInfo->header.msgId)
    {
        case P2P_MSG_ID_RESP_HEARTBEAT_ACK:
        {
            /* No payload */
        }
        break;

        case P2P_MSG_ID_RESP_AUTHENTICATION:
        case P2P_MSG_ID_RESP_STUN_RESPONSE:
        {
            /* Get payload information */
            JSON_GET_STRING(root, payload, P2P_DEVICE_PUBLIC_IP_TAG, pJsonInfo->payload.publicAddr.ip);
            JSON_GET_INTEGER(root, payload, P2P_DEVICE_PUBLIC_PORT_TAG, pJsonInfo->payload.publicAddr.port);
        }
        break;

        case P2P_MSG_ID_RESP_REGISTER_SUCCESS:
        {
            json_t  *pRelayServer;
            json_t  *pAddrIdx;
            UINT32  index, addrIndexMax;

            /* Get payload information */
            JSON_GET_STRING(root, payload, P2P_DEVICE_PUBLIC_IP_TAG, pJsonInfo->payload.publicAddr.ip);
            JSON_GET_INTEGER(root, payload, P2P_DEVICE_PUBLIC_PORT_TAG, pJsonInfo->payload.publicAddr.port);

            /* Is relay server tag available? */
            if (FALSE == JSON_IS_ARRAY(payload, P2P_RELAY_SERVER_TAG))
            {
                WPRINT(P2P_MODULE, "relay server address tag not available in register");
                break;
            }

            /* Get the array of relay server aadress and max available entries in it */
            JSON_GET_ARRAY(root, payload, P2P_RELAY_SERVER_TAG, pRelayServer, addrIndexMax);

            /* Truncate the max address to max supported by us if more than that provided */
            addrIndexMax = (addrIndexMax > RELAY_SERVER_SUPPORT_MAX) ? RELAY_SERVER_SUPPORT_MAX : addrIndexMax;

            /* Extract all the relay server addresses */
            for (index = 0; index < addrIndexMax; index++)
            {
                /* Get the array index to get the information */
                JSON_GET_ARRAY_OBJECT(root, pRelayServer, index, pAddrIdx);

                /* Get the relay server domain address or URL */
                JSON_GET_STRING2(root, pAddrIdx, P2P_RELAY_SERVER_ADDR_TAG, pJsonInfo->payload.relayAddr[index].domain);

                /* Get the relay server port */
                JSON_GET_INTEGER(root, pAddrIdx, P2P_RELAY_SERVER_PORT_TAG, pJsonInfo->payload.relayAddr[index].port);
            }
        }
        break;

        case P2P_MSG_ID_RESP_REGISTER_FAIL:
        case P2P_MSG_ID_RESP_HEARTBEAT_FAIL:
        case P2P_MSG_ID_RESP_FALLBACK_TO_RELAY_FAIL:
        {
            /* Get payload information */
            JSON_GET_INTEGER(root, payload, P2P_FAIL_REASON_TAG, pJsonInfo->payload.reason);
        }
        break;

        case P2P_MSG_ID_RESP_CONNECTION_REQUEST:
        case P2P_MSG_ID_RESP_FALLBACK_TO_RELAY:
        {
            /* Get payload information */
            JSON_GET_STRING(root, payload, P2P_CLIENT_VERSION_TAG, pJsonInfo->payload.version);
            JSON_GET_INTEGER(root, payload, P2P_CLIENT_TYPE_TAG, pJsonInfo->payload.type);
            JSON_GET_STRING(root, payload, P2P_CLIENT_UID_TAG, pJsonInfo->payload.uId);
            JSON_GET_STRING(root, payload, P2P_CLIENT_LOCAL_IP_TAG, pJsonInfo->payload.localAddr.ip);
            JSON_GET_INTEGER(root, payload, P2P_CLIENT_LOCAL_PORT_TAG, pJsonInfo->payload.localAddr.port);
            JSON_GET_STRING(root, payload, P2P_CLIENT_PUBLIC_IP_TAG, pJsonInfo->payload.publicAddr.ip);
            JSON_GET_INTEGER(root, payload, P2P_CLIENT_PUBLIC_PORT_TAG, pJsonInfo->payload.publicAddr.port);

            /* Newly added parameters, which are not mandatory and tags may not available */
            JSON_GET_STRING3(root, payload, P2P_CLIENT_MODEL_TAG, pJsonInfo->payload.model);
            JSON_GET_STRING3(root, payload, P2P_CLIENT_CONN_MODE_TAG, pJsonInfo->payload.connMode);
        }
        break;

        case P2P_MSG_ID_RESP_CONNECTION_FAIL:
        {
            /* Get payload information */
            JSON_GET_STRING(root, payload, P2P_CLIENT_UID_TAG, pJsonInfo->payload.uId);
            JSON_GET_INTEGER(root, payload, P2P_FAIL_REASON_TAG, pJsonInfo->payload.reason);
        }
        break;

        case P2P_MSG_ID_RESP_CONNECTION_DETAILS_ACK:
        case P2P_MSG_ID_RESP_FALLBACK_TO_RELAY_ACK:
        {
            /* Get payload information */
            JSON_GET_STRING(root, payload, P2P_CLIENT_UID_TAG, pJsonInfo->payload.uId);
        }
        break;

        default:
        {
            /* Free all objects */
            json_decref(root);
            EPRINT(P2P_MODULE, "invld msg id in p2p msg recv: [msgId=%d]", pJsonInfo->header.msgId);
        }
        return FALSE;
    }

    /* Successfully parsed json message */
    json_decref(root);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get local time in buffer (Format: YYYYMMDDHHMMSSsss)
 * @param   pTsBuff
 * @param   buffLen
 * @note    Timezone and DST are not applied. Hence time will be in UTC format
 */
void GetP2pTimeStamp(CHAR *pTsBuff, UINT8 buffLen)
{
    struct timespec ts;
    struct tm       brokenTime;

    /* Get realtime clock */
    clock_gettime(CLOCK_REALTIME, &ts);

    /* Convert time since epoch to current date time time */
    ConvertLocalTimeInBrokenTm(&ts.tv_sec, &brokenTime);

    /* Store time in buffer with milliseconds */
    snprintf(pTsBuff, buffLen, "%04d%02d%02d%02d%02d%02d%03d",
             brokenTime.tm_year, brokenTime.tm_mon + 1, brokenTime.tm_mday,
             brokenTime.tm_hour, brokenTime.tm_min, brokenTime.tm_sec,
             (UINT16)(ts.tv_nsec/NANO_SEC_PER_MILLI_SEC));
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Open P2P socket
 * @param   isUdpSock
 * @param   pLocalAddr
 * @param   pSockFd
 * @return  TRUE on success; FALSE otherwise
 */
BOOL OpenP2pSocket(BOOL isUdpSock, IP_ADDR_INFO_t *pLocalAddr, INT32 *pSockFd)
{
    struct sockaddr_in  localSockAddr;
    socklen_t           sockLen = sizeof(struct sockaddr_in);
    INT32 				sockOpt = 1;

    /* Open P2P process socket for registration */
    *pSockFd = socket(AF_INET, ((isUdpSock == TRUE) ? UDP_NB_SOCK_OPTIONS : TCP_NB_SOCK_OPTIONS), 0);
    if (*pSockFd == INVALID_CONNECTION)
    {
        EPRINT(P2P_MODULE, "failed to create socket: [err=%s], [isUdpSock=%d]", STR_ERR, isUdpSock);
        return FALSE;
    }

    do
    {
        /* Set reuse address for TCP protocol */
        if (isUdpSock == FALSE)
        {
            /* Set socket as reuse addr */
            if (setsockopt(*pSockFd, SOL_SOCKET, SO_REUSEADDR, &sockOpt, sizeof(sockOpt)) < 0)
            {
                EPRINT(P2P_MODULE, "failed to set reuse addr: [err=%s]", STR_ERR);
                break;
            }

            /* Set socket as reuse port */
            if (setsockopt(*pSockFd, SOL_SOCKET, SO_REUSEPORT, &sockOpt, sizeof(sockOpt)) < 0)
            {
                EPRINT(P2P_MODULE, "failed to set reuse port: [err=%s]", STR_ERR);
                break;
            }
        }

        /* Local address information */
        memset(&localSockAddr, 0, sizeof(localSockAddr));
        localSockAddr.sin_family = AF_INET;
        localSockAddr.sin_port = htons(pLocalAddr->port);

        /*  Convert local ip from string to network format */
        if (inet_pton(AF_INET, pLocalAddr->ip, &localSockAddr.sin_addr.s_addr) != 1)
        {
            EPRINT(P2P_MODULE, "failed to convert ip in nw format: [ip=%s]", pLocalAddr->ip);
            break;
        }

        /* Bind socket with local address */
        if (bind(*pSockFd, (struct sockaddr*)&localSockAddr, sockLen) < STATUS_OK)
        {
            EPRINT(P2P_MODULE, "failed to bind socket: [ip=%s], [err=%s]", pLocalAddr->ip, STR_ERR);
            break;
        }

        /* Get bound ip and port if not provided */
        if (pLocalAddr->port == 0)
        {
            memset(&localSockAddr, 0, sizeof(localSockAddr));
            if (getsockname(*pSockFd, (struct sockaddr*)&localSockAddr, &sockLen))
            {
                EPRINT(P2P_MODULE, "failed to get connection info: [err=%s]", STR_ERR);
                break;
            }

            /* Save local port */
            pLocalAddr->port = htons(localSockAddr.sin_port);
        }

        /* Required socket created */
        return TRUE;

    }while(0);

    /* Close socket on failure */
    CloseSocket(pSockFd);
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Connect P2P Socket for one-to-one connection
 * @param   sockFd
 * @param   pRemoteAddr
 * @return  TRUE on success; IN_PROGRESS on wait more, FALSE otherwise
 */
BOOL ConnectP2pSocket(INT32 sockFd, IP_ADDR_INFO_t *pRemoteAddr)
{
    struct sockaddr_in p2pSockAddr;

    /* Remote address information */
    memset(&p2pSockAddr, 0, sizeof(p2pSockAddr));
    p2pSockAddr.sin_family = AF_INET;
    p2pSockAddr.sin_port = htons(pRemoteAddr->port);

    /*  Convert remote ip from string to network format */
    if (inet_pton(AF_INET, pRemoteAddr->ip, &p2pSockAddr.sin_addr.s_addr) != 1)
    {
        EPRINT(P2P_MODULE, "failed to convert ip in nw format: [ip=%s]", pRemoteAddr->ip);
        return FALSE;
    }

    /* Connect socket */
    if (connect(sockFd, (struct sockaddr *)&p2pSockAddr, sizeof(p2pSockAddr)))
    {
        /* Wait if connection in progress */
        if (errno == EINPROGRESS)
        {
            /* Wait for connection */
            return IN_PROGRESS;
        }
        else
        {
            EPRINT(P2P_MODULE, "failed to connect socket: [err=%s]", STR_ERR);
            return FALSE;
        }
    }

    /* Successfully bound socket */
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Check for connection establishment
 * @param   sockFd
 * @return  TRUE on success; IN_PROGRESS on wait more, FALSE otherwise
 */
BOOL IsP2pConnectionEstablished(INT32 sockFd)
{
    INT32               sockOpt = STATUS_OK;
    socklen_t           optLen = sizeof(sockOpt);
    UINT8               pollSts;
    INT16               pollRevent;

    /* Check FD validity */
    if (sockFd == INVALID_CONNECTION)
    {
        /* Invalid socket FD */
        EPRINT(P2P_MODULE, "invld socket fd");
        return FALSE;
    }

    /* Check for write ready socket */
    pollSts = GetSocketPollEvent(sockFd, POLLWRNORM, 0, &pollRevent);

    /* No write event till timeout */
    if (TIMEOUT == pollSts)
    {
        return IN_PROGRESS;
    }

    /* poll failed */
    if (FAIL == pollSts)
    {
        EPRINT(P2P_MODULE, "poll failed: [fd=%d], [err=%s]", sockFd, STR_ERR);
        return FALSE;
    }

    /* Check for event on socket */
    if (getsockopt(sockFd, SOL_SOCKET, SO_ERROR, &sockOpt, &optLen) != STATUS_OK)
    {
        EPRINT(P2P_MODULE, "fail to get socket options for connect: [fd=%d], [err=%s]", sockFd, STR_ERR);
        return FALSE;
    }

    /* Check error found or not socket */
    if (sockOpt != STATUS_OK)
    {
        /* Wait if connection in progress */
        if (sockOpt == EINPROGRESS)
        {
            /* Wait for connection */
            return IN_PROGRESS;
        }

        EPRINT(P2P_MODULE, "error found in connect: [fd=%d], [err=%s]", sockFd, strerror(sockOpt));
        if ((sockOpt == ECONNREFUSED) || (sockOpt == ECONNRESET) || (sockOpt == ECONNABORTED))
        {
            return REFUSE;
        }
        return FALSE;
    }

    /* Successfully bound socket */
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send P2P message on connected socket
 * @param   sockFd
 * @param   pMsgBuff
 * @param   msgLen
 * @param   timeout
 * @return  Returns: SUCCESS-->Data recv, IN_PROGRESS-->No data recv, REFUSE --> Conn closed or Bad FD else FAIL
 */
BOOL SendP2pMsg(INT32 sockFd, CHAR *pMsgBuff, UINT32 msgLen, UINT32 timeout)
{
    INT32 sentLen;

    /* Use regular data send function id timeout specified */
    if (timeout)
    {
        /* Send data on socket */
        return SendToSocket(sockFd, (UINT8 *)pMsgBuff, msgLen, timeout);
    }

    /* Check FD is valid or not */
    if (sockFd == INVALID_CONNECTION)
    {
        EPRINT(P2P_MODULE, "invld fd found");
        return REFUSE;
    }

    /* Send message on socket */
    sentLen = send(sockFd, pMsgBuff, msgLen, MSG_DONTWAIT | MSG_NOSIGNAL);
    if (sentLen == -1)
    {
        /* Recv API failed */
        switch(errno)
        {
            case EINTR:
            {
                EPRINT(P2P_MODULE, "send interrupted by siganl: [fd=%d]", sockFd);
            }
            return FAIL;

            case EBADF:
            {
                EPRINT(P2P_MODULE, "bad fd found: [fd=%d]", sockFd);
            }
            return REFUSE;

            case EAGAIN:
            #if (EWOULDBLOCK != EAGAIN)
            case EWOULDBLOCK:
            #endif
            {
                /* No space available on socket */
            }
            return IN_PROGRESS;

            case EPIPE:
            case ECONNRESET:
            case ECONNREFUSED:
            {
                EPRINT(P2P_MODULE, "connection closed: [fd=%d], [err=%s]", sockFd, STR_ERR);
            }
            return REFUSE;

            default:
            {
                EPRINT(P2P_MODULE, "unhandled socket error: [err=%s]", STR_ERR);
            }
            return FAIL;
        }
    }

    /* Sanity check */
    if (sentLen != (INT32)msgLen)
    {
        EPRINT(P2P_MODULE, "truncated msg sent: [msgLen=%d], [sentLen=%d]", msgLen, sentLen);
        return FAIL;
    }

    /* Message sent successfully */
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Receive P2P message from connected socket
 * @param   sockFd
 * @param   pMsgBuff
 * @param   buffSize
 * @param   pMsgLen
 * @return  Returns: SUCCESS-->Data recv, IN_PROGRESS-->No data recv, REFUSE --> Conn closed or Bad FD else FAIL
 */
BOOL RecvP2pMsg(INT32 sockFd, CHAR *pMsgBuff, UINT32 buffSize, UINT32 *pMsgLen)
{
    INT32 recvLen;

    /* Check FD is valid or not */
    if (sockFd == INVALID_CONNECTION)
    {
        EPRINT(P2P_MODULE, "invld fd found");
        return REFUSE;
    }

    /* Receive data from socket */
    recvLen = recv(sockFd, pMsgBuff, buffSize, MSG_DONTWAIT | MSG_NOSIGNAL);
    if (recvLen > 0)
    {
        *pMsgLen = recvLen;
        return SUCCESS;
    }

    /* Remote connection closed */
    if (recvLen == 0)
    {
        /* Error occurred on socket */
        //EPRINT(P2P_MODULE, "remote connection closed: [err=%s]", STR_ERR);
        return REFUSE;
    }

    /* Recv API failed */
    switch(errno)
    {
        case EINTR:
        {
            EPRINT(P2P_MODULE, "recv interrupted by siganl: [fd=%d]", sockFd);
        }
        return FAIL;

        case EBADF:
        {
            EPRINT(P2P_MODULE, "bad fd found: [fd=%d]", sockFd);
        }
        return REFUSE;

        case EAGAIN:
        #if (EWOULDBLOCK != EAGAIN)
        case EWOULDBLOCK:
        #endif
        {
            /* No data available on socket */
        }
        return IN_PROGRESS;

        case ECONNREFUSED:
        {
            //EPRINT(P2P_MODULE, "connection closed: [fd=%d], [err=%s]", sockFd, STR_ERR);
        }
        return REFUSE;

        default:
        {
            EPRINT(P2P_MODULE, "unhandled socket error: [err=%s]", STR_ERR);
        }
        return FAIL;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get available bytes on socket to be read from socket
 * @param   sockFd
 * @return  Pending bytes to be read from socket
 */
INT32 GetAvailableBytesOnSocket(INT32 sockFd)
{
    INT32 byteCnt;

    /* Get available bytes on socket */
    if (ioctl(sockFd, FIONREAD, &byteCnt) < 0)
    {
        /* No data availables */
        return 0;
    }

    /* Pending byte count */
    return byteCnt;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
