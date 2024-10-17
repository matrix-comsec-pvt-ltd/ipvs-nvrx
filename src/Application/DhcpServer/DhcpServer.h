#if !defined DHCP_SERVER_H
#define DHCP_SERVER_H

//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		DhcpServer.h
@brief      File containing the function prototype of DHCP server functionality in NVR
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "ConfigApi.h"
#include "DateTime.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define DHCP_SERVER_ACTION_STR_MAX      4
#define DHCP_SERVER_HOSTNAME_STR_MAX    16

/* Max DHCP server lease status to be maintain */
#define DHCP_SERVER_LEASE_CLIENT_MAX    100

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef struct
{
    CHAR        action[DHCP_SERVER_ACTION_STR_MAX];
    CHAR        clientMacAddr[MAX_MAC_ADDRESS_WIDTH];
    CHAR        assignIpAddr[IPV4_ADDR_LEN_MAX];
    CHAR        clientHostname[DHCP_SERVER_HOSTNAME_STR_MAX];
    time_t      leaseExpireTime;
    UINT32      remainingTime;

}DHCP_SERVER_NOTIFY_t;

typedef struct
{
    CHAR        assignIpAddr[IPV4_ADDR_LEN_MAX];
    CHAR        clientMacAddr[MAX_MAC_ADDRESS_WIDTH];
    CHAR        clientHostname[DHCP_SERVER_HOSTNAME_STR_MAX];
    time_t      leaseExpireTime;

}DHCP_SERVER_LEASE_STATUS_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitDhcpServer(void);
//-------------------------------------------------------------------------------------------------
void DeInitDhcpServer(void);
//-------------------------------------------------------------------------------------------------
void DhcpServerConfigNotify(DHCP_SERVER_CONFIG_t newCopy, DHCP_SERVER_CONFIG_t *oldCopy);
//-------------------------------------------------------------------------------------------------
void DhcpServerLanConfigUpdate(LAN_CONFIG_ID_e lanNo, LAN_CONFIG_t newLanConfig, LAN_CONFIG_t *oldLanConfig);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e ValidateDhcpServerCfg(DHCP_SERVER_CONFIG_t newDhcpServerConfig);
//-------------------------------------------------------------------------------------------------
void DhcpServerConfigRestoreNotify(void);
//-------------------------------------------------------------------------------------------------
void DhcpServerLeaseUpdateNotify(DHCP_SERVER_NOTIFY_t *pDhcpServerNotify);
//-------------------------------------------------------------------------------------------------
UINT32 GetDhcpServerLeaseStatus(DHCP_SERVER_LEASE_STATUS_t *pLeaseStatus);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif
