#if !defined NETWORKCOMMAND_H
#define NETWORKCOMMAND_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		NetworkCommand.h
@brief      This file contains functions to process the commands and send response back to client.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "MxTypedef.h"

//#################################################################################################
// @ENUM
//#################################################################################################
typedef enum
{
    REQ_LOG,
    ACK_LOG,
    REQ_POL,
    RSP_POL,
    GET_CFG,
    SET_CFG,
    DEF_CFG,
    RPL_CFG,
    SET_CMD,
    RPL_CMD,
    REQ_EVT,
    RCV_EVT,
    REQ_FTS,
    RPL_FTS,
    ACK_FTS,
    SET_FTS,
    STP_FTS,
    RPL_STP,
    DOR_CMD,
    PWD_RST,
    RPL_PWD,

    /* These onwords our message structure*/
    DEV_DETECT,
    CGI_REQ_GETPORT,
    WS_SYSTEM_LOGS,
    WS_PCAP_TRACE,
    GET_MAC_SERVER_CFG,
    WS_SYSTEM_INFO,
    DHCP_SERVER_NOTIFY,
    DDNS_CLIENT_NOTIFY,
    MAX_HEADER_REQ
}HEADER_REQ_e;

//#################################################################################################
// @EXTERN VARIABLES
//#################################################################################################
/* Send command response to client */
extern const NW_CMD_REPLY_CB clientCmdRespCb[CLIENT_CB_TYPE_MAX];

/* Send command response data on socket as per client type */
extern const SEND_TO_SOCKET_CB sendCmdCb[CLIENT_CB_TYPE_MAX];

/* Send frame data on socket */
extern const SEND_TO_SOCKET_CB sendDataCb[CLIENT_CB_TYPE_MAX];
extern const SEND_TO_CLIENT_CB sendClientDataCb[CLIENT_CB_TYPE_MAX];

/* Close client socket */
extern const CLOSE_SOCKET_CB closeConnCb[CLIENT_CB_TYPE_MAX];

/* Protocol message header */
extern const CHARPTR headerReq[MAX_HEADER_REQ];

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitNetworkCommand(void);
//-------------------------------------------------------------------------------------------------
BOOL ProcessSetCommand(CHARPTR commandStrPtr, UINT8 sessionIndex, INT32 clientSocket, CLIENT_CB_TYPE_e callbackType);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif  // NETWORKCOMMAND_H
