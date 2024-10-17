#ifndef DDNSCLIENT_H_
#define DDNSCLIENT_H_
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		DdnsClient.h
@brief      This module provides functionalities to initialize; check and update configurations of
            the Dynamic DNS Client which in turn registers the current DynDNS configuration (Router's
            IP address or NVR IP address, host name and other information) to DynDNS server. It uses
            open source 'inadyn' utility to perform registration to configured DynDNS server. It
            automatically retrieves its public IP if behind NAT.Once registered successfully, it keeps
            updating DynDNS server periodically at interval configured by user. In case of failure
            to register, it retries at pre-defined period.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "Config.h"

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitDdnsClient(void);
//-------------------------------------------------------------------------------------------------
void DeInitDdnsClient(void);
//-------------------------------------------------------------------------------------------------
void UpdateDdnsConfig(DDNS_CONFIG_t newDdnsConfig, DDNS_CONFIG_t *oldDdnsConfig);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e UpdateDdnsRegIp(NW_CMD_REPLY_CB callbackFun, UINT32 clientFd);
//-------------------------------------------------------------------------------------------------
void DdnsClientEventNotify(CHAR *event, CHAR *ipAddr, CHAR *errNum, CHAR *errMsg);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif /* DDNSCLIENT_H_ */
