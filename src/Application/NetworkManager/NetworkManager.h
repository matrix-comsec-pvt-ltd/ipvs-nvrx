#if !defined NETWORKMANAGER_H
#define NETWORKMANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		NetworkManager.h
@brief      This module provides Network communication API functionality.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <pthread.h>

/* Application Includes */
#include "SysTimer.h"
#include "EventLogger.h"
#include "ConfigApi.h"
#include "UserSession.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define MAX_REPLY_SZ 				65536
#define MAX_RCV_SZ                  16384
#define LOCAL_USER_IP               0x100007F	//127.0.0.1
#define	MESSAGE_REPLY_TIMEOUT		2
#define INTERNAL_SOCKET_FILE 		"/tmp/IntSocket"

#define MAX_COMMAND_ARG_LEN			64
#define GET_CAMERA_INDEX(x)         (x - 1)
#if defined(RK3588_NVRH) || defined(HI3536_NVRH)
#define MAX_EVENTS_IN_BUFFER 		400
#define	MAX_EVENTS_IN_ONE_SEG		210	//(MAX_EVENTS_IN_BUFFER/2)
#else
#define MAX_EVENTS_IN_BUFFER 		200
#define	MAX_EVENTS_IN_ONE_SEG		110	//(MAX_EVENTS_IN_BUFFER/2)
#endif

#define LOCAL_CLIENT_EVT_STR        "Local Client"
#define DEVICE_CLIENT_EVT_STR       "Device Client"
#define MOBILE_CLIENT_EVT_STR       "Mobile Client"
#define P2P_MOBILE_CLIENT_EVT_STR   "P2P Mobile Client"
#define P2P_DESKTOP_CLIENT_EVT_STR  "P2P Desktop Client"
#define SAMAS_INTEGRATION_EVT_STR   "SAMAS Integration"

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
BOOL InitNetworkManager(void);
//-------------------------------------------------------------------------------------------------
void NwCallbackFuncForBkupRcd(NET_CMD_STATUS_e status, INT32 connFd, float freeSize, float recordSize, BOOL closeConn, CLIENT_CB_TYPE_e callbackType);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e FindHeaderIndex(CHARPTR *pMsgBuff, UINT8 *pMsgId);
//-------------------------------------------------------------------------------------------------
void DeinitNwManager(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e FindSessionIndex(CHARPTR *pMsgBuff, UINT8 *pSessionIdx);
//-------------------------------------------------------------------------------------------------
void GetLiveEvents(UINT8 sessionIdx, INT32 connFd, CLIENT_CB_TYPE_e clientCbType);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#ifdef __cplusplus
}
#endif
#endif  // NETWORKMANAGER_H
