//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		NetworkInterface.c
@brief      This file provides the interface with network manager library module
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Library Includes */
#include "nm_iputility.h"
#include "network_manager.h"

/* Application Includes */
#include "NetworkInterface.h"
#include "NetworkController.h"
#include "DebugLog.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define NETWORK_MANAGER_DAEMON_PATH                     BIN_DIR_PATH "/networkmanagerd"
#define RETURN_IF_NETWORK_PORT_NOT_REGISTERED(portType) if (portType >= NETWORK_PORT_MAX) \
                                                            {EPRINT(ETHERNET, "invld network port type: [portType=%d]", portType); return FAIL;} \
                                                        if (nwIfaceInfo[portType].ifaceId == NM_INVALID_INTERFACE_ID) \
                                                            {EPRINT(ETHERNET, "network port not registered: [portType=%d]", portType); return FAIL;}

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef struct
{
    CHAR            ifaceName[INTERFACE_NAME_LEN_MAX];
    NM_InterfaceId  ifaceId;

}NETWORK_INTERFACE_INFO_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static NETWORK_INTERFACE_INFO_t nwIfaceInfo[NETWORK_PORT_MAX];

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static NETWORK_PORT_e getNetworkPortType(NM_InterfaceId ifaceId);
//-------------------------------------------------------------------------------------------------
static void nmLogCb(NM_LogLevel_e level, const mxChar* msg, mxI32_t len);
//-------------------------------------------------------------------------------------------------
static void updateIpv4DnsServerAddr(IPV4_LAN_CONFIG_t *ipv4AddrInfo);
//-------------------------------------------------------------------------------------------------
static void updateIpv6DnsServerAddr(IPV6_LAN_CONFIG_t *ipv6AddrInfo);
//-------------------------------------------------------------------------------------------------
static void networkManagerEventCb(NM_EventId_e event, NM_InterfaceId ifaceId, const void *eventData);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   Init network manager module
 */
void InitNetworkInterface(void)
{
    NMSts_e         nmSts;
    NETWORK_PORT_e  portType;
    UINT8           retryCnt = 0;

    /* Reset network interface info */
    for (portType = 0; portType < NETWORK_PORT_MAX; portType++)
    {
        nwIfaceInfo[portType].ifaceId = NM_INVALID_INTERFACE_ID;
        memset(nwIfaceInfo[portType].ifaceName, 0, sizeof(nwIfaceInfo[portType].ifaceName));
    }

    /* Start network manager daemon */
    if (FAIL == ExeSysCmd(TRUE, NETWORK_MANAGER_DAEMON_PATH))
    {
        EPRINT(ETHERNET, "fail to start network manager daemon");
        return;
    }

    while(TRUE)
    {
        /* Wait and check */
        sleep(1);

        /* Init network manager module */
        nmSts = NMLib_Init();
        if (nmSts == NMSTS_SUCCESS)
        {
            DPRINT(ETHERNET, "network manager module started successfully");
            break;
        }

        retryCnt++;
        if (retryCnt > 5)
        {
            EPRINT(ETHERNET, "fail to init network manager module: [nmSts=%d]", nmSts);
            return;
        }
    }

    /* Set network manager generated event callback */
    NMLib_SetEventCallback(networkManagerEventCb);

    /* Set network manager debug config */
    SetNetworkManagerDebugConfig();

    DPRINT(ETHERNET, "network manager init successfully");
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Deinit network manager module
 */
void DeInitNetworkInterface(void)
{
    NMSts_e         nmSts;
    NETWORK_PORT_e  portType;

    /* Deregister network port */
    for (portType = 0; portType < NETWORK_PORT_MAX; portType++)
    {
        if (nwIfaceInfo[portType].ifaceId != NM_INVALID_INTERFACE_ID)
        {
            DeregisterNetworkPort(portType);
        }
    }

    /* Exit networm manager module */
    nmSts = NMLib_Exit();
    if (nmSts != NMSTS_SUCCESS)
    {
        EPRINT(ETHERNET, "fail to deinit network manager module: [nmSts=%d]", nmSts);
    }

    if (FAIL == ExeSysCmd(TRUE, "pkill -TERM -f \"" NETWORK_MANAGER_DAEMON_PATH "\""))
    {
        EPRINT(ETHERNET, "fail to stop network manager daemon");
    }

    DPRINT(ETHERNET, "network manager deinit successfully");
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set network manager debug config
 */
void SetNetworkManagerDebugConfig(void)
{
    NMSts_e         nmSts;
    NM_LogConfig_t  logConfig;

    /* Set network manager module debug config */
    memset(&logConfig, 0, sizeof(logConfig));
    logConfig.level = NM_LOG_TRACE;
    logConfig.sink = GetDebugFlag(ETHERNET) ? NM_LOG_SINK_CALLBACK : NM_LOG_SINK_NONE;
    logConfig.logCallback = nmLogCb;

    /* Set network manager module logs as callback */
    nmSts = NMLib_ConfigureLogs(&logConfig);
    if (nmSts != NMSTS_SUCCESS)
    {
        EPRINT(ETHERNET, "fail to set network manager log: [nmSts=%d]", nmSts);
        return;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get network port name string
 * @param   portType
 * @param   ifaceName
 * @return  Returns pointer to interface name on success otherwise null on failure
 */
const CHAR *GetNetworkPortName(NETWORK_PORT_e portType, CHAR *ifaceName)
{
    /* If interface name needed then init with null */
    if (NULL != ifaceName)
    {
        /* Init with null interface name */
        ifaceName[0] = '\0';
    }

    /* Validate input param */
    if (portType >= NETWORK_PORT_MAX)
    {
        /* Invalid input parameters */
        return NULL;
    }

    /* Is interface registered? */
    if (nwIfaceInfo[portType].ifaceId == NM_INVALID_INTERFACE_ID)
    {
        /* Interface is not registered */
        return NULL;
    }

    /* Set registered interface name */
    if (NULL == ifaceName)
    {
        /* Provide only interface name */
        return nwIfaceInfo[portType].ifaceName;
    }

    /* Set interface name in input buffer if provided */
    snprintf(ifaceName, INTERFACE_NAME_LEN_MAX, "%s", nwIfaceInfo[portType].ifaceName);
    return ifaceName;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Register network port in network manager module for management
 * @param   portType
 * @param   ifaceName
 * @return  Returns success or fail
 */
BOOL RegisterNetworkPort(NETWORK_PORT_e portType, const CHAR *ifaceName)
{
    NMSts_e                     nmSts;
    NMIface_InterfaceConfig_t   ifaceInfo;

    /* Is valid network port type? */
    if (portType >= NETWORK_PORT_MAX)
    {
        EPRINT(ETHERNET, "invld network port type: [portType=%d]", portType);
        return FAIL;
    }

    /* Is network port already registered? */
    if (nwIfaceInfo[portType].ifaceId != NM_INVALID_INTERFACE_ID)
    {
        WPRINT(ETHERNET, "network port already registered: [port=%d], [ifaceId=%d], [ifname=%s]",
               portType, nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName);
        return FAIL;
    }

    /* Set manage interface information */
    memset(&ifaceInfo, 0, sizeof(ifaceInfo));
    snprintf(ifaceInfo.ifaceName, sizeof(ifaceInfo.ifaceName), "%s", ifaceName);
    ifaceInfo.deviceType = (portType == NETWORK_PORT_USB_MODEM) ? NM_DEVICE_TYPE_USB_MODEM : NM_DEVICE_TYPE_ETHERNET;

    /* Manage interface in network manager module */
    nmSts = NMIface_ManageInterface(&ifaceInfo, &nwIfaceInfo[portType].ifaceId);
    if (nmSts != NMSTS_SUCCESS)
    {
        nwIfaceInfo[portType].ifaceId = NM_INVALID_INTERFACE_ID;
        EPRINT(ETHERNET, "fail to add interface: [ifname=%s], [nmSts=%d]", ifaceName, nmSts);
        return FAIL;
    }

    /* Store interface name for future communication */
    snprintf(nwIfaceInfo[portType].ifaceName, sizeof(nwIfaceInfo[portType].ifaceName), "%s", ifaceName);
    DPRINT(ETHERNET, "interface added successfully: [ifaceId=%d], [ifname=%s]",
           nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Deregister network port from network manager module for management
 * @param   portType
 * @return  Returns success or fail
 */
BOOL DeregisterNetworkPort(NETWORK_PORT_e portType)
{
    NMSts_e nmSts;

    /* Return if network port is not registered with network manager */
    RETURN_IF_NETWORK_PORT_NOT_REGISTERED(portType);

    /* Release interface from network manager module */
    nmSts = NMIface_ReleaseInterface(nwIfaceInfo[portType].ifaceId);
    if (nmSts != NMSTS_SUCCESS)
    {
        EPRINT(ETHERNET, "fail to remove interface: [ifaceId=%d], [ifname=%s], [nmSts=%d]",
               nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName, nmSts);
        return FAIL;
    }

    /* Reset network port information */
    DPRINT(ETHERNET, "interface removed successfully: [ifaceId=%d], [ifname=%s]",
           nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName);
    nwIfaceInfo[portType].ifaceId = NM_INVALID_INTERFACE_ID;
    memset(nwIfaceInfo[portType].ifaceName, 0, sizeof(nwIfaceInfo[portType].ifaceName));
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Make network port link up
 * @param   portType
 * @return  Returns success or fail
 */
BOOL NetworkPortLinkUp(NETWORK_PORT_e portType)
{
    NMSts_e nmSts;

    /* Return if network port is not registered with network manager */
    RETURN_IF_NETWORK_PORT_NOT_REGISTERED(portType);

    /* Make interface link up */
    nmSts = NMIface_InterfaceUp(nwIfaceInfo[portType].ifaceId);
    if (nmSts != NMSTS_SUCCESS)
    {
        EPRINT(ETHERNET, "fail to up interface link: [ifaceId=%d], [ifname=%s], [nmSts=%d]",
               nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName, nmSts);
        return FAIL;
    }

    DPRINT(ETHERNET, "interface link up successfully: [ifaceId=%d], [ifname=%s]",
           nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Make network port link down
 * @param   portType
 * @return  Returns success or fail
 */
BOOL NetworkPortLinkDown(NETWORK_PORT_e portType)
{
    NMSts_e nmSts;

    /* Return if network port is not registered with network manager */
    RETURN_IF_NETWORK_PORT_NOT_REGISTERED(portType);

    /* Make interface link down */
    nmSts = NMIface_InterfaceDown(nwIfaceInfo[portType].ifaceId);
    if (nmSts != NMSTS_SUCCESS)
    {
        EPRINT(ETHERNET, "fail to down interface link: [ifaceId=%d], [ifname=%s], [nmSts=%d]",
               nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName, nmSts);
        return FAIL;
    }

    DPRINT(ETHERNET, "interface link down successfully: [ifaceId=%d], [ifname=%s]",
           nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get network port mac address
 * @param   portType
 * @param   macAddr
 * @return  Returns success or fail
 */
BOOL GetNetworkPortMacAddr(NETWORK_PORT_e portType, CHAR *macAddr)
{
    NMSts_e                 nmSts;
    NMIface_MacAddrInfo_t   macAddrInfo;

    /* Return if network port is not registered with network manager */
    RETURN_IF_NETWORK_PORT_NOT_REGISTERED(portType);

    /* Get interface mac address */
    nmSts = NMIface_GetMacAddr(nwIfaceInfo[portType].ifaceId, &macAddrInfo);
    if (nmSts != NMSTS_SUCCESS)
    {
        EPRINT(ETHERNET, "fail to get mac addr: [ifaceId=%d], [ifname=%s], [nmSts=%d]",
               nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName, nmSts);
        return FAIL;
    }

    snprintf(macAddr, MAX_MAC_ADDRESS_WIDTH, "%s", macAddrInfo.macAddr);
    DPRINT(ETHERNET, "mac addr get successfully: [ifaceId=%d], [ifname=%s], [mac=%s]",
           nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName, macAddr);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Remove IPv4 address from interface
 * @param   portType
 * @return  Returns success or fail
 */
BOOL RemoveIpv4Addr(NETWORK_PORT_e portType)
{
    NMSts_e nmSts;

    /* Return if network port is not registered with network manager */
    RETURN_IF_NETWORK_PORT_NOT_REGISTERED(portType);

    /* Remove all ipv4 address */
    nmSts = NMIpv4_ClearAllAddr(nwIfaceInfo[portType].ifaceId);
    if (nmSts != NMSTS_SUCCESS)
    {
        EPRINT(ETHERNET, "fail to remove ipv4 addr: [ifaceId=%d], [ifname=%s], [nmSts=%d]",
               nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName, nmSts);
        return FAIL;
    }

    DPRINT(ETHERNET, "ipv4 addr removed successfully: [ifaceId=%d], [ifname=%s]",
           nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Remove IPv4 address assignment mode
 * @param   portType
 * @return  Returns success or fail
 */
BOOL RemoveIpv4Mode(NETWORK_PORT_e portType)
{
    NMSts_e nmSts;

    /* Return if network port is not registered with network manager */
    RETURN_IF_NETWORK_PORT_NOT_REGISTERED(portType);

    /* Remove ipv4 address assignment mode */
    nmSts = NMIpv4_ClearMethod(nwIfaceInfo[portType].ifaceId);
    if (nmSts != NMSTS_SUCCESS)
    {
        EPRINT(ETHERNET, "fail to clear ipv4 method: [ifaceId=%d], [ifname=%s], [nmSts=%d]",
               nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName, nmSts);
        return FAIL;
    }

    DPRINT(ETHERNET, "ipv4 methed cleared successfully: [ifaceId=%d], [ifname=%s]",
           nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set IPv4 static address on network port
 * @param   portType
 * @param   staticAddrInfo
 * @return  Returns success or fail
 */
BOOL SetIpv4StaticMode(NETWORK_PORT_e portType, const IPV4_NETWORK_PARAMETER_t *staticAddrInfo)
{
    NMSts_e                 nmSts;
    NMIpv4_StaticConfig_t   ipv4Static;

    /* Return if network port is not registered with network manager */
    RETURN_IF_NETWORK_PORT_NOT_REGISTERED(portType);

    /* Set IPv4 static address info */
    memset(&ipv4Static, 0, sizeof(ipv4Static));
    snprintf(ipv4Static.addrInfo.ip, sizeof(ipv4Static.addrInfo.ip), "%s", staticAddrInfo->ipAddress);
    snprintf(ipv4Static.addrInfo.subnet, sizeof(ipv4Static.addrInfo.subnet), "%s", staticAddrInfo->subnetMask);
    ipv4Static.conflictCheck = NM_FALSE;

    /* Set ipv4 static address */
    nmSts = NMIpv4_SetStaticMethod(nwIfaceInfo[portType].ifaceId, &ipv4Static);
    if (nmSts != NMSTS_SUCCESS)
    {
        EPRINT(ETHERNET, "fail to set ipv4 static addr: [ifaceId=%d], [ifname=%s], [nmSts=%d]",
               nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName, nmSts);
        return FAIL;
    }

    DPRINT(ETHERNET, "ipv4 static addr set successfully: [ifaceId=%d], [ifname=%s]",
           nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set IPv4 DHCP mode on network port
 * @param   portType
 * @param   hostname
 * @return  Returns success or fail
 */
BOOL SetIpv4DhcpMode(NETWORK_PORT_e portType)
{
    NMSts_e nmSts;

    /* Return if network port is not registered with network manager */
    RETURN_IF_NETWORK_PORT_NOT_REGISTERED(portType);

    /* Set ipv4 DHCP mode */
    nmSts = NMIpv4_SetDhcpMethod(nwIfaceInfo[portType].ifaceId);
    if (nmSts != NMSTS_IN_PROGRESS)
    {
        EPRINT(ETHERNET, "fail to set ipv4 dhcp mode: [ifaceId=%d], [ifname=%s], [nmSts=%d]",
               nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName, nmSts);
        return FAIL;
    }

    DPRINT(ETHERNET, "ipv4 dhcp mode set successfully: [ifaceId=%d], [ifname=%s]",
           nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set IPv4 DHCP assigned address on network port
 * @param   portType
 * @param   dhcpAddrInfo
 * @return  Returns success or fail
 */
BOOL SetIpv4DhcpAddr(NETWORK_PORT_e portType, const IPV4_NETWORK_PARAMETER_t *dhcpAddrInfo)
{
    NMSts_e             nmSts;
    NMIpv4_AddrInfo_t   dhcpAssignedAddr;

    /* Return if network port is not registered with network manager */
    RETURN_IF_NETWORK_PORT_NOT_REGISTERED(portType);

    /* Set IPv4 DHCP assigned address info */
    memset(&dhcpAssignedAddr, 0, sizeof(dhcpAssignedAddr));
    snprintf(dhcpAssignedAddr.ip, sizeof(dhcpAssignedAddr.ip), "%s", dhcpAddrInfo->ipAddress);
    snprintf(dhcpAssignedAddr.subnet, sizeof(dhcpAssignedAddr.subnet), "%s", dhcpAddrInfo->subnetMask);

    /* Set IPv4 DHCP assigned address */
    nmSts = NMIpv4_SetDhcpAssignedAddr(nwIfaceInfo[portType].ifaceId, &dhcpAssignedAddr);
    if (nmSts != NMSTS_SUCCESS)
    {
        EPRINT(ETHERNET, "fail to set ipv4 dhcp addr: [ifaceId=%d], [ifname=%s], [nmSts=%d]",
               nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName, nmSts);
        return FAIL;
    }

    DPRINT(ETHERNET, "ipv4 dhcp addr set successfully: [ifaceId=%d], [ifname=%s]",
           nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set IPv4 PPPoE mode on network port
 * @param   portType
 * @param   pppoeInfo
 * @return  Returns success or fail
 */
BOOL SetIpv4PppoeMode(NETWORK_PORT_e portType, const PPPOE_PARAMETER_t *pppoeInfo)
{
    NMSts_e                 nmSts;
    NMIpv4_PppoeConfig_t    ipv4Pppoe;

    /* Return if network port is not registered with network manager */
    RETURN_IF_NETWORK_PORT_NOT_REGISTERED(portType);

    /* Set IPv4 PPPoE mode info */
    memset(&ipv4Pppoe, 0, sizeof(ipv4Pppoe));
    snprintf(ipv4Pppoe.username, sizeof(ipv4Pppoe.username), "%s", pppoeInfo->username);
    snprintf(ipv4Pppoe.password, sizeof(ipv4Pppoe.password), "%s", pppoeInfo->password);

    /* Set ipv4 PPPoE mode */
    nmSts = NMIpv4_SetPppoeMethod(nwIfaceInfo[portType].ifaceId, &ipv4Pppoe);
    if (nmSts != NMSTS_IN_PROGRESS)
    {
        EPRINT(ETHERNET, "fail to set ipv4 pppoe mode: [ifaceId=%d], [ifname=%s], [nmSts=%d]",
               nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName, nmSts);
        return FAIL;
    }

    DPRINT(ETHERNET, "ipv4 pppoe mode set successfully: [ifaceId=%d], [ifname=%s]",
           nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Add IPv4 default gateway
 * @param   portType
 * @param   gatewayAddrInfo
 * @return  Returns success or fail
 */
BOOL AddIpv4DefaultGateway(NETWORK_PORT_e portType, const IPV4_NETWORK_PARAMETER_t *gatewayAddrInfo)
{
    NMSts_e                 nmSts;
    NMIpv4_GatewayInfo_t    defaultGateway;

    /* Return if network port is not registered with network manager */
    RETURN_IF_NETWORK_PORT_NOT_REGISTERED(portType);

    /* Add IPv4 default gateway info */
    memset(&defaultGateway, 0, sizeof(defaultGateway));
    snprintf(defaultGateway.gateway, sizeof(defaultGateway.gateway), "%s", gatewayAddrInfo->gateway);

    /* Add ipv4 default gateway */
    nmSts = NMIpv4_SetDefaultGateway(nwIfaceInfo[portType].ifaceId, &defaultGateway);
    if (nmSts != NMSTS_SUCCESS)
    {
        EPRINT(ETHERNET, "fail to add ipv4 default gateway: [ifaceId=%d], [ifname=%s], [nmSts=%d]",
               nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName, nmSts);
        return FAIL;
    }

    DPRINT(ETHERNET, "ipv4 default gateway added successfully: [ifaceId=%d], [ifname=%s]",
           nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Remove IPv4 default gateway
 * @param   portType
 * @return  Returns success or fail
 */
BOOL RemoveIpv4DefaultGateway(NETWORK_PORT_e portType)
{
    NMSts_e nmSts;

    /* Return if network port is not registered with network manager */
    RETURN_IF_NETWORK_PORT_NOT_REGISTERED(portType);

    /* Remove ipv4 default gateway */
    nmSts = NMIpv4_ClearDefaultGateway(nwIfaceInfo[portType].ifaceId);
    if (nmSts != NMSTS_SUCCESS)
    {
        EPRINT(ETHERNET, "fail to remove ipv4 default gateway: [ifaceId=%d], [ifname=%s], [nmSts=%d]",
               nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName, nmSts);
        return FAIL;
    }

    DPRINT(ETHERNET, "ipv4 default gateway removed successfully: [ifaceId=%d], [ifname=%s]",
           nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Add IPv4 static route
 * @param   portType
 * @param   networkAddr
 * @param   subnet
 * @param   gatewayAddr
 * @return  Returns success or fail
 */
BOOL AddIpv4StaticRoute(NETWORK_PORT_e portType, const CHAR *networkAddr, const CHAR *subnet, const CHAR *gatewayAddr)
{
    NMSts_e nmSts;
    NMIpv4_StaticRouteConfig_t ipv4StaticRoute;

    /* Return if network port is not registered with network manager */
    RETURN_IF_NETWORK_PORT_NOT_REGISTERED(portType);

    /* Set IPv4 static route info */
    memset(&ipv4StaticRoute, 0, sizeof(ipv4StaticRoute));
    snprintf(ipv4StaticRoute.networkAddr, sizeof(ipv4StaticRoute.networkAddr), "%s", networkAddr);
    snprintf(ipv4StaticRoute.subnet, sizeof(ipv4StaticRoute.subnet), "%s", subnet);
    snprintf(ipv4StaticRoute.gateway, sizeof(ipv4StaticRoute.gateway), "%s", gatewayAddr);

    /* Add IPv4 static route */
    nmSts = NMIpv4_AddStaticRoute(nwIfaceInfo[portType].ifaceId, &ipv4StaticRoute);
    if (nmSts != NMSTS_SUCCESS)
    {
        EPRINT(ETHERNET, "fail to add ipv4 static route: [ifaceId=%d], [ifname=%s], [nmSts=%d]",
               nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName, nmSts);
        return FAIL;
    }

    DPRINT(ETHERNET, "ipv4 static route added successfully: [ifaceId=%d], [ifname=%s]",
           nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Remove IPv4 static route
 * @param   portType
 * @param   networkAddr
 * @param   subnet
 * @param   gatewayAddr
 * @return  Returns success or fail
 */
BOOL RemoveIpv4StaticRoute(NETWORK_PORT_e portType, const CHAR *networkAddr, const CHAR *subnet, const CHAR *gatewayAddr)
{
    NMSts_e nmSts;
    NMIpv4_StaticRouteConfig_t ipv4StaticRoute;

    /* Return if network port is not registered with network manager */
    RETURN_IF_NETWORK_PORT_NOT_REGISTERED(portType);

    /* Set IPv4 static route info */
    memset(&ipv4StaticRoute, 0, sizeof(ipv4StaticRoute));
    snprintf(ipv4StaticRoute.networkAddr, sizeof(ipv4StaticRoute.networkAddr), "%s", networkAddr);
    snprintf(ipv4StaticRoute.subnet, sizeof(ipv4StaticRoute.subnet), "%s", subnet);
    snprintf(ipv4StaticRoute.gateway, sizeof(ipv4StaticRoute.gateway), "%s", gatewayAddr);

    /* Delete IPv4 static route */
    nmSts = NMIpv4_DeleteStaticRoute(nwIfaceInfo[portType].ifaceId, &ipv4StaticRoute);
    if (nmSts != NMSTS_SUCCESS)
    {
        EPRINT(ETHERNET, "fail to remove ipv4 static route: [ifaceId=%d], [ifname=%s], [nmSts=%d]",
               nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName, nmSts);
        return FAIL;
    }

    DPRINT(ETHERNET, "ipv4 static route removed successfully: [ifaceId=%d], [ifname=%s]",
           nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Enable IPv6 stack on interface
 * @param   portType
 * @return  Returns success or fail
 */
BOOL EnableIpv6OnInterface(NETWORK_PORT_e portType)
{
    NMSts_e nmSts;

    /* Return if network port is not registered with network manager */
    RETURN_IF_NETWORK_PORT_NOT_REGISTERED(portType);

    nmSts = NMIpv6_EnableOnInterface(nwIfaceInfo[portType].ifaceId);
    if (nmSts != NMSTS_SUCCESS)
    {
        EPRINT(ETHERNET, "fail to enable ipv6 stack: [ifaceId=%d], [ifname=%s], [nmSts=%d]",
               nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName, nmSts);
        return FAIL;
    }

    DPRINT(ETHERNET, "ipv6 stack enabled successfully: [ifaceId=%d], [ifname=%s]",
           nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Disable IPv6 stack on interface
 * @param   portType
 * @return  Returns success or fail
 */
BOOL DisableIpv6OnInterface(NETWORK_PORT_e portType)
{
    NMSts_e nmSts;

    /* Return if network port is not registered with network manager */
    RETURN_IF_NETWORK_PORT_NOT_REGISTERED(portType);

    nmSts = NMIpv6_DisableOnInterface(nwIfaceInfo[portType].ifaceId);
    if (nmSts != NMSTS_SUCCESS)
    {
        EPRINT(ETHERNET, "fail to disable ipv6 stack: [ifaceId=%d], [ifname=%s], [nmSts=%d]",
               nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName, nmSts);
        return FAIL;
    }

    DPRINT(ETHERNET, "ipv6 stack disabled successfully: [ifaceId=%d], [ifname=%s]",
           nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Remove IPv6 address from interface
 * @param   portType
 * @return  Returns success or fail
 */
BOOL RemoveIpv6Addr(NETWORK_PORT_e portType)
{
    NMSts_e nmSts;

    /* Return if network port is not registered with network manager */
    RETURN_IF_NETWORK_PORT_NOT_REGISTERED(portType);

    /* Remove all ipv6 address */
    nmSts = NMIpv6_ClearAllAddr(nwIfaceInfo[portType].ifaceId);
    if (nmSts != NMSTS_SUCCESS)
    {
        EPRINT(ETHERNET, "fail to remove ipv6 addr: [ifaceId=%d], [ifname=%s], [nmSts=%d]",
               nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName, nmSts);
        return FAIL;
    }

    DPRINT(ETHERNET, "ipv6 addr removed successfully: [ifaceId=%d], [ifname=%s]",
           nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Update IPv6 address attributes (Preferred lifetime and valid lifetime)
 * @param   portType
 * @param   addrAttrInfo
 * @return  Returns success or fail
 */
BOOL UpdateIpv6AddrAttr(NETWORK_PORT_e portType, const IPV6_NETWORK_PARAMETER_t *addrAttrInfo)
{
    NMSts_e             nmSts;
    NMIpv6_AddrInfo_t   ipv6AddrAttr;

    /* Return if network port is not registered with network manager */
    RETURN_IF_NETWORK_PORT_NOT_REGISTERED(portType);

    /* Set IPv6 address attributes info */
    memset(&ipv6AddrAttr, 0, sizeof(ipv6AddrAttr));
    snprintf(ipv6AddrAttr.ip, sizeof(ipv6AddrAttr.ip), "%s", addrAttrInfo->ipAddress);
    ipv6AddrAttr.preferredTime = addrAttrInfo->ipPreferredTime;
    ipv6AddrAttr.validTime = addrAttrInfo->ipValidTime;
    ipv6AddrAttr.prefixLen = addrAttrInfo->prefixLen;

    /* Update ipv6 address attributes */
    nmSts = NMIpv6_UpdateAddrAttributes(nwIfaceInfo[portType].ifaceId, &ipv6AddrAttr);
    if (nmSts != NMSTS_SUCCESS)
    {
        EPRINT(ETHERNET, "fail to update ipv6 addr attr: [ifaceId=%d], [ifname=%s], [nmSts=%d]",
               nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName, nmSts);
        return FAIL;
    }

    DPRINT(ETHERNET, "ipv6 addr attr updated successfully: [ifaceId=%d], [ifname=%s]",
           nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Remove IPv6 address assignment mode
 * @param   portType
 * @return  Returns success or fail
 */
BOOL RemoveIpv6Mode(NETWORK_PORT_e portType)
{
    NMSts_e nmSts;

    /* Return if network port is not registered with network manager */
    RETURN_IF_NETWORK_PORT_NOT_REGISTERED(portType);

    /* Remove ipv6 address assignment mode */
    nmSts = NMIpv6_ClearMethod(nwIfaceInfo[portType].ifaceId);
    if (nmSts != NMSTS_SUCCESS)
    {
        EPRINT(ETHERNET, "fail to clear ipv6 method: [ifaceId=%d], [ifname=%s], [nmSts=%d]",
               nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName, nmSts);
        return FAIL;
    }

    DPRINT(ETHERNET, "ipv6 methed cleared successfully: [ifaceId=%d], [ifname=%s]",
           nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set IPv6 static address on network port
 * @param   portType
 * @param   staticInfo
 * @return  Returns success or fail
 */
BOOL SetIpv6StaticMode(NETWORK_PORT_e portType, const IPV6_NETWORK_PARAMETER_t *staticAddrInfo)
{
    NMSts_e             nmSts;
    NMIpv6_AddrInfo_t   ipv6Static;

    /* Return if network port is not registered with network manager */
    RETURN_IF_NETWORK_PORT_NOT_REGISTERED(portType);

    /* Set IPv6 static address info */
    memset(&ipv6Static, 0, sizeof(ipv6Static));
    snprintf(ipv6Static.ip, sizeof(ipv6Static.ip), "%s", staticAddrInfo->ipAddress);
    ipv6Static.prefixLen = staticAddrInfo->prefixLen;

    /* Set IPv6 static address */
    nmSts = NMIpv6_SetStaticMethod(nwIfaceInfo[portType].ifaceId, &ipv6Static);
    if (nmSts != NMSTS_IN_PROGRESS)
    {
        EPRINT(ETHERNET, "fail to set ipv6 static addr: [ifaceId=%d], [ifname=%s], [nmSts=%d]",
               nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName, nmSts);
        return FAIL;
    }

    DPRINT(ETHERNET, "ipv6 static addr set successfully: [ifaceId=%d], [ifname=%s]",
           nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set IPv6 DHCP mode
 * @param   portType
 * @return  Returns success or fail
 */
BOOL SetIpv6DhcpMode(NETWORK_PORT_e portType)
{
    NMSts_e nmSts;

    /* Return if network port is not registered with network manager */
    RETURN_IF_NETWORK_PORT_NOT_REGISTERED(portType);

    /* Set IPv6 DHCP mode */
    nmSts = NMIpv6_SetDhcpMethod(nwIfaceInfo[portType].ifaceId);
    if (nmSts != NMSTS_IN_PROGRESS)
    {
        EPRINT(ETHERNET, "fail to set ipv6 dhcp mode: [ifaceId=%d], [ifname=%s], [nmSts=%d]",
               nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName, nmSts);
        return FAIL;
    }

    DPRINT(ETHERNET, "ipv6 dhcp mode set successfully: [ifaceId=%d], [ifname=%s]",
           nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set IPv6 DHCP assigned address on network port
 * @param   portType
 * @param   dhcpAddrInfo
 * @return  Returns success or fail
 */
BOOL SetIpv6DhcpAddr(NETWORK_PORT_e portType, const IPV6_NETWORK_PARAMETER_t *dhcpAddrInfo)
{
    NMSts_e             nmSts;
    NMIpv6_AddrInfo_t   dhcpAssignedAddr;

    /* Return if network port is not registered with network manager */
    RETURN_IF_NETWORK_PORT_NOT_REGISTERED(portType);

    /* Set IPv6 DHCP assigned address info */
    memset(&dhcpAssignedAddr, 0, sizeof(dhcpAssignedAddr));
    snprintf(dhcpAssignedAddr.ip, sizeof(dhcpAssignedAddr.ip), "%s", dhcpAddrInfo->ipAddress);
    dhcpAssignedAddr.preferredTime = dhcpAddrInfo->ipPreferredTime;
    dhcpAssignedAddr.validTime = dhcpAddrInfo->ipValidTime;
    dhcpAssignedAddr.prefixLen = dhcpAddrInfo->prefixLen;

    /* Set IPv6 DHCP assigned address */
    nmSts = NMIpv6_SetDhcpAssignedAddr(nwIfaceInfo[portType].ifaceId, &dhcpAssignedAddr);
    if (nmSts != NMSTS_IN_PROGRESS)
    {
        EPRINT(ETHERNET, "fail to set ipv6 dhcp addr: [ifaceId=%d], [ifname=%s], [nmSts=%d]",
               nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName, nmSts);
        return FAIL;
    }

    DPRINT(ETHERNET, "ipv6 dhcp addr set successfully: [ifaceId=%d], [ifname=%s]",
           nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set IPv6 SLAAC mode
 * @param   portType
 * @return  Returns success or fail
 */
BOOL SetIpv6SlaacMode(NETWORK_PORT_e portType)
{
    NMSts_e nmSts;

    /* Return if network port is not registered with network manager */
    RETURN_IF_NETWORK_PORT_NOT_REGISTERED(portType);

    /* Set IPv6 SLAAC mode */
    nmSts = NMIpv6_SetSlaacMethod(nwIfaceInfo[portType].ifaceId);
    if (nmSts != NMSTS_IN_PROGRESS)
    {
        EPRINT(ETHERNET, "fail to set ipv6 slaac mode: [ifaceId=%d], [ifname=%s], [nmSts=%d]",
               nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName, nmSts);
        return FAIL;
    }

    DPRINT(ETHERNET, "ipv6 slaac mode set successfully: [ifaceId=%d], [ifname=%s]",
           nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set IPv6 SLAAC assigned address on network port
 * @param   portType
 * @param   slaacAddrInfo
 * @return  Returns success or fail
 */
BOOL SetIpv6SlaacAddr(NETWORK_PORT_e portType, const IPV6_NETWORK_PARAMETER_t *slaacAddrInfo)
{
    NMSts_e             nmSts;
    NMIpv6_AddrInfo_t   slaacAssignedAddr;

    /* Return if network port is not registered with network manager */
    RETURN_IF_NETWORK_PORT_NOT_REGISTERED(portType);

    /* Set IPv6 SLAAC assigned address info */
    memset(&slaacAssignedAddr, 0, sizeof(slaacAssignedAddr));
    snprintf(slaacAssignedAddr.ip, sizeof(slaacAssignedAddr.ip), "%s", slaacAddrInfo->ipAddress);
    slaacAssignedAddr.preferredTime = slaacAddrInfo->ipPreferredTime;
    slaacAssignedAddr.validTime = slaacAddrInfo->ipValidTime;
    slaacAssignedAddr.prefixLen = slaacAddrInfo->prefixLen;

    /* Set IPv6 SLAAC assigned address */
    nmSts = NMIpv6_SetSlaacAssignedAddr(nwIfaceInfo[portType].ifaceId, &slaacAssignedAddr);
    if (nmSts != NMSTS_IN_PROGRESS)
    {
        EPRINT(ETHERNET, "fail to set ipv6 slaac addr: [ifaceId=%d], [ifname=%s], [nmSts=%d]",
               nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName, nmSts);
        return FAIL;
    }

    DPRINT(ETHERNET, "ipv6 slaac addr set successfully: [ifaceId=%d], [ifname=%s]",
           nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Add IPv6 default gateway
 * @param   portType
 * @param   gatewayAddrInfo
 * @return  Returns success or fail
 */
BOOL AddIpv6DefaultGateway(NETWORK_PORT_e portType, const IPV6_NETWORK_PARAMETER_t *gatewayAddrInfo)
{
    NMSts_e                 nmSts;
    NMIpv6_GatewayInfo_t    defaultGateway;

    /* Return if network port is not registered with network manager */
    RETURN_IF_NETWORK_PORT_NOT_REGISTERED(portType);

    /* Add IPv6 default gateway info */
    memset(&defaultGateway, 0, sizeof(defaultGateway));
    snprintf(defaultGateway.gateway, sizeof(defaultGateway.gateway), "%s", gatewayAddrInfo->gateway);
    defaultGateway.metric = gatewayAddrInfo->gatewayMetric;

    /* Add ipv6 default gateway */
    nmSts = NMIpv6_SetDefaultGateway(nwIfaceInfo[portType].ifaceId, &defaultGateway);
    if (nmSts != NMSTS_SUCCESS)
    {
        EPRINT(ETHERNET, "fail to add ipv6 default gateway: [ifaceId=%d], [ifname=%s], [nmSts=%d]",
               nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName, nmSts);
        return FAIL;
    }

    DPRINT(ETHERNET, "ipv6 default gateway added successfully: [ifaceId=%d], [ifname=%s]",
           nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Remove IPv6 default gateway
 * @param   portType
 * @return  Returns success or fail
 */
BOOL RemoveIpv6DefaultGateway(NETWORK_PORT_e portType)
{
    NMSts_e nmSts;

    /* Return if network port is not registered with network manager */
    RETURN_IF_NETWORK_PORT_NOT_REGISTERED(portType);

    /* Remove ipv6 default gateway */
    nmSts = NMIpv6_ClearDefaultGateway(nwIfaceInfo[portType].ifaceId);
    if (nmSts != NMSTS_SUCCESS)
    {
        EPRINT(ETHERNET, "fail to remove ipv6 default gateway: [ifaceId=%d], [ifname=%s], [nmSts=%d]",
               nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName, nmSts);
        return FAIL;
    }

    DPRINT(ETHERNET, "ipv6 default gateway removed successfully: [ifaceId=%d], [ifname=%s]",
           nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Add IPv6 static route
 * @param   portType
 * @param   networkAddr
 * @param   prefixLen
 * @param   gateway
 * @return  Returns success or fail
 */
BOOL AddIpv6StaticRoute(NETWORK_PORT_e portType, const CHAR *networkAddr, UINT8 prefixLen, const CHAR *gateway)
{
    NMSts_e nmSts;
    NMIpv6_StaticRouteConfig_t ipv6StaticRoute;

    /* Return if network port is not registered with network manager */
    RETURN_IF_NETWORK_PORT_NOT_REGISTERED(portType);

    /* Set IPv6 static route info */
    memset(&ipv6StaticRoute, 0, sizeof(ipv6StaticRoute));
    snprintf(ipv6StaticRoute.networkAddr, sizeof(ipv6StaticRoute.networkAddr), "%s", networkAddr);
    ipv6StaticRoute.prefixLen = prefixLen;
    snprintf(ipv6StaticRoute.gateway, sizeof(ipv6StaticRoute.gateway), "%s", gateway);

    /* Add IPv6 static route */
    nmSts = NMIpv6_AddStaticRoute(nwIfaceInfo[portType].ifaceId, &ipv6StaticRoute);
    if (nmSts != NMSTS_SUCCESS)
    {
        EPRINT(ETHERNET, "fail to add ipv6 static route: [ifaceId=%d], [ifname=%s], [nmSts=%d]",
               nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName, nmSts);
        return FAIL;
    }

    DPRINT(ETHERNET, "ipv6 static route added successfully: [ifaceId=%d], [ifname=%s]",
           nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Remove IPv6 static route
 * @param   portType
 * @param   networkAddr
 * @param   prefixLen
 * @param   gateway
 * @return  Returns success or fail
 */
BOOL RemoveIpv6StaticRoute(NETWORK_PORT_e portType, const CHAR *networkAddr, UINT8 prefixLen, const CHAR *gateway)
{
    NMSts_e nmSts;
    NMIpv6_StaticRouteConfig_t ipv6StaticRoute;

    /* Return if network port is not registered with network manager */
    RETURN_IF_NETWORK_PORT_NOT_REGISTERED(portType);

    /* Set IPv6 static route info */
    memset(&ipv6StaticRoute, 0, sizeof(ipv6StaticRoute));
    snprintf(ipv6StaticRoute.networkAddr, sizeof(ipv6StaticRoute.networkAddr), "%s", networkAddr);
    ipv6StaticRoute.prefixLen = prefixLen;
    snprintf(ipv6StaticRoute.gateway, sizeof(ipv6StaticRoute.gateway), "%s", gateway);

    /* Delete IPv6 static route */
    nmSts = NMIpv6_DeleteStaticRoute(nwIfaceInfo[portType].ifaceId, &ipv6StaticRoute);
    if (nmSts != NMSTS_SUCCESS)
    {
        EPRINT(ETHERNET, "fail to remove ipv6 static route: [ifaceId=%d], [ifname=%s], [nmSts=%d]",
               nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName, nmSts);
        return FAIL;
    }

    DPRINT(ETHERNET, "ipv6 static route removed successfully: [ifaceId=%d], [ifname=%s]",
           nwIfaceInfo[portType].ifaceId, nwIfaceInfo[portType].ifaceName);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Start usb to serial modem communication with ISP
 * @param   portType
 * @param   usbModemConfig
 * @return  Returns success or fail
 */
BOOL StartUsbToSerialModem(NETWORK_PORT_e portType, const BROAD_BAND_PROFILE_t *usbModemConfig)
{
    NMSts_e nmSts;
    NMIpv4_UsbToSerialConfig_t usbToSerialInfo;

    /* Return if network port is not registered with network manager */
    RETURN_IF_NETWORK_PORT_NOT_REGISTERED(portType);

    /* Set usb to serial modem info */
    memset(&usbToSerialInfo, 0, sizeof(usbToSerialInfo));
    snprintf(usbToSerialInfo.dialNumber, sizeof(usbToSerialInfo.dialNumber), "%s", usbModemConfig->dialNumber);
    snprintf(usbToSerialInfo.apn, sizeof(usbToSerialInfo.apn), "%s", usbModemConfig->apn);
    snprintf(usbToSerialInfo.username, sizeof(usbToSerialInfo.username), "%s", usbModemConfig->userName);
    snprintf(usbToSerialInfo.password, sizeof(usbToSerialInfo.password), "%s", usbModemConfig->password);

    /* Start usb to serial modem communication */
    nmSts = NMIpv4_StartUsbModemPppConnection(nwIfaceInfo[portType].ifaceId, &usbToSerialInfo);
    if (nmSts != NMSTS_IN_PROGRESS)
    {
        EPRINT(ETHERNET, "fail to start usb to serial modem communication: [ifaceId=%d], [nmSts=%d]", nwIfaceInfo[portType].ifaceId, nmSts);
        return FAIL;
    }

    DPRINT(ETHERNET, "usb to serial modem communication started successfully: [ifaceId=%d]", nwIfaceInfo[portType].ifaceId);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Stop usb to serial modem communication with ISP
 * @param   portType
 * @return  Returns success or fail
 */
BOOL StopUsbToSerialModem(NETWORK_PORT_e portType)
{
    /* Return if network port is not registered with network manager */
    RETURN_IF_NETWORK_PORT_NOT_REGISTERED(portType);

    /* Stop usb to serial modem communication */
    return RemoveIpv4Mode(portType);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set dns server address
 * @param   ipv4DnsAddr
 * @param   ipv6DnsAddr
 * @return  Returns success or fail
 */
BOOL SetDnsServerAddr(const DNS_PARAMETER_t *ipv4DnsAddr, const DNS_PARAMETER_t *ipv6DnsAddr)
{
    NMSts_e             nmSts;
    UINT8               dnsCnt = 0;
    NM_DnsServerInfo_t  dnsServerInfo;

    /* Set dns server info */
    memset(&dnsServerInfo, 0, sizeof(dnsServerInfo));

    /* Set primary ipv4 dns address if not null */
    if (ipv4DnsAddr->primaryAddress[0] != '\0')
    {
        snprintf(dnsServerInfo.nameServers[dnsCnt], sizeof(dnsServerInfo.nameServers[dnsCnt]), "%s", ipv4DnsAddr->primaryAddress);
        dnsCnt++;
    }

    /* Set secondary ipv4 dns address if not null */
    if (ipv4DnsAddr->secondaryAddress[0] != '\0')
    {
        snprintf(dnsServerInfo.nameServers[dnsCnt], sizeof(dnsServerInfo.nameServers[dnsCnt]), "%s", ipv4DnsAddr->secondaryAddress);
        dnsCnt++;
    }

    /* Add IPv6 dns address if available */
    if (ipv6DnsAddr != NULL)
    {
        /* Set primary ipv6 dns address if not null */
        if (ipv6DnsAddr->primaryAddress[0] != '\0')
        {
            snprintf(dnsServerInfo.nameServers[dnsCnt], sizeof(dnsServerInfo.nameServers[dnsCnt]), "%s", ipv6DnsAddr->primaryAddress);
            dnsCnt++;
        }

        /* Set secondary ipv6 dns address if not null */
        if (ipv6DnsAddr->secondaryAddress[0] != '\0')
        {
            snprintf(dnsServerInfo.nameServers[dnsCnt], sizeof(dnsServerInfo.nameServers[dnsCnt]), "%s", ipv6DnsAddr->secondaryAddress);
            dnsCnt++;
        }
    }

    /* Set remote dns server address */
    nmSts = NMDns_SetRemoteDnsServer(&dnsServerInfo);
    if (nmSts != NMSTS_SUCCESS)
    {
        EPRINT(ETHERNET, "fail to set dns server addr: [nmSts=%d]", nmSts);
        return FAIL;
    }

    DPRINT(ETHERNET, "dns server addr set successfully");
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set system hostname
 * @param   hostname
 * @return  Returns success or fail
 */
BOOL SetSystemHostName(const CHAR *hostname)
{
    NMSts_e             nmSts;
    NM_HostnameInfo_t   hostnameInfo;
    NM_HostsEntryInfo_t hostsEntryInfo;

    /* Set hostname info */
    memset(&hostnameInfo, 0, sizeof(hostnameInfo));
    snprintf(hostnameInfo.hostname, sizeof(hostnameInfo.hostname), "%s", hostname);

    nmSts = NMHost_SetHostname(&hostnameInfo);
    if (nmSts != NMSTS_SUCCESS)
    {
        EPRINT(ETHERNET, "fail to set system hostname: [nmSts=%d]", nmSts);
        return FAIL;
    }

    DPRINT(ETHERNET, "system hostname set successfully");

    /* Set host entry info */
    memset(&hostsEntryInfo, 0, sizeof(hostsEntryInfo));
    snprintf(hostsEntryInfo.hosts[0].ipAddr, sizeof(hostsEntryInfo.hosts[0].ipAddr), NM_IPV4_LOOPBACK_ADDR_STR);
    snprintf(hostsEntryInfo.hosts[0].domain, sizeof(hostsEntryInfo.hosts[0].domain), NM_LOOPBACK_HOSTNAME);
    snprintf(hostsEntryInfo.hosts[1].ipAddr, sizeof(hostsEntryInfo.hosts[1].ipAddr), NM_IPV4_LOOPBACK_ADDR_STR);
    snprintf(hostsEntryInfo.hosts[1].domain, sizeof(hostsEntryInfo.hosts[1].domain), "%s", hostname);
    snprintf(hostsEntryInfo.hosts[2].ipAddr, sizeof(hostsEntryInfo.hosts[2].ipAddr), NM_IPV6_LOOPBACK_ADDR_STR);
    snprintf(hostsEntryInfo.hosts[2].domain, sizeof(hostsEntryInfo.hosts[2].domain), NM_LOOPBACK_HOSTNAME);
    snprintf(hostsEntryInfo.hosts[3].ipAddr, sizeof(hostsEntryInfo.hosts[3].ipAddr), NM_IPV6_LOOPBACK_ADDR_STR);
    snprintf(hostsEntryInfo.hosts[3].domain, sizeof(hostsEntryInfo.hosts[3].domain), "%s", hostname);

    /* Set hosts entry */
    nmSts = NMDns_UpdateHostsFile(&hostsEntryInfo);
    if (nmSts != NMSTS_SUCCESS)
    {
        EPRINT(ETHERNET, "fail to update hosts entry: [nmSts=%d]", nmSts);
        return FAIL;
    }

    DPRINT(ETHERNET, "hosts entry updated successfully");
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get network port type from interface id
 * @param   ifaceId
 * @return  Network port type
 */
static NETWORK_PORT_e getNetworkPortType(NM_InterfaceId ifaceId)
{
    NETWORK_PORT_e portType;

    /* Check all registered network ports */
    for (portType = 0; portType < NETWORK_PORT_MAX; portType++)
    {
        /* Skip the non allocated port */
        if (nwIfaceInfo[portType].ifaceId == NM_INVALID_INTERFACE_ID)
        {
            continue;
        }

        /* Is it required network port? */
        if (nwIfaceInfo[portType].ifaceId == ifaceId)
        {
            return portType;
        }
    }

    /* Network port not found */
    return NETWORK_PORT_MAX;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Network manager log callback
 * @param   level
 * @param   msg
 * @param   len
 */
static void nmLogCb(NM_LogLevel_e level, const mxChar* msg, mxI32_t len)
{
    /* Print network manager debugs as per level */
    if ((level == NM_LOG_FATAL) || (level == NM_LOG_ERROR))
    {
        /* It is error or fatal debug */
        EPRINT(NETWORK_LIBRARY, "%s", msg);
    }
    else if (level == NM_LOG_WARN)
    {
        /* It is warning debug */
        WPRINT(NETWORK_LIBRARY, "%s", msg);
    }
    else
    {
        /* It is normal or info debug */
        DPRINT(NETWORK_LIBRARY, "%s", msg);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Update IPv4 dns server address. Use gateway address as dns if dns address is not available
 * @param   ipv4AddrInfo
 */
static void updateIpv4DnsServerAddr(IPV4_LAN_CONFIG_t *ipv4AddrInfo)
{
    /* Is primary dns address available? */
    if (ipv4AddrInfo->dns.primaryAddress[0] == '\0')
    {
        /* Use gateway as primary dns address */
        snprintf(ipv4AddrInfo->dns.primaryAddress, sizeof(ipv4AddrInfo->dns.primaryAddress), "%s", ipv4AddrInfo->lan.gateway);
    }
    else if (ipv4AddrInfo->dns.secondaryAddress[0] == '\0')
    {
        /* If secondary dns address not available and primary dns address is not same as default gateway */
        if (strcmp(ipv4AddrInfo->dns.primaryAddress, ipv4AddrInfo->lan.gateway) != 0)
        {
            /* Use gateway as secondary dns address */
            snprintf(ipv4AddrInfo->dns.secondaryAddress, sizeof(ipv4AddrInfo->dns.secondaryAddress), "%s", ipv4AddrInfo->lan.gateway);
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Update IPv6 dns server address. Use gateway address as dns if dns address is not available
 * @param   ipv6AddrInfo
 */
static void updateIpv6DnsServerAddr(IPV6_LAN_CONFIG_t *ipv6AddrInfo)
{
    /* Is primary dns address available? */
    if (ipv6AddrInfo->dns.primaryAddress[0] == '\0')
    {
        /* Use gateway as primary dns address */
        snprintf(ipv6AddrInfo->dns.primaryAddress, sizeof(ipv6AddrInfo->dns.primaryAddress), "%s", ipv6AddrInfo->lan.gateway);
    }
    else if (ipv6AddrInfo->dns.secondaryAddress[0] == '\0')
    {
        /* If secondary dns address not available and primary dns address is not same as default gateway */
        if (strcmp(ipv6AddrInfo->dns.primaryAddress, ipv6AddrInfo->lan.gateway) != 0)
        {
            /* Use gateway as secondary dns address */
            snprintf(ipv6AddrInfo->dns.secondaryAddress, sizeof(ipv6AddrInfo->dns.secondaryAddress), "%s", ipv6AddrInfo->lan.gateway);
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Network manager event callback
 * @param   event
 * @param   ifaceId
 * @param   eventData
 */
static void networkManagerEventCb(NM_EventId_e event, NM_InterfaceId ifaceId, const void *eventData)
{
    /* Handle daemon fatal error if generated */
    if (event == NM_EVENT_ID_DAEMON_FATAL_ERR)
    {
        EPRINT(ETHERNET, "network manager is no more alive..!!");
        return;
    }

    /* Get network port type from interface id */
    NETWORK_PORT_e portType = getNetworkPortType(ifaceId);
    if (portType >= NETWORK_PORT_MAX)
    {
        EPRINT(ETHERNET, "invld interface id: [ifaceId=%d], [event=%d]", ifaceId, event);
        return;
    }

    /* Handle network manager events */
    switch(event)
    {
        /* Handle ethernet link up event */
        case NM_EVENT_ID_PHY_LINK_UP:
        {
            OnPhyLinkUp(portType);
        }
        break;

        /* Handle ethernet link down event */
        case NM_EVENT_ID_PHY_LINK_DOWN:
        {
            OnPhyLinkDown(portType);
        }
        break;

        /* Handle IPv4 DHCP deconfig event */
        case NM_EVENT_ID_IPV4_DHCP_DECONFIG:
        {
            OnIpv4DhcpDeconfig(portType);
        }
        break;

        /* Handle IPv4 lease fail event */
        case NM_EVENT_ID_IPV4_DHCP_LEASE_FAIL:
        {
            OnIpv4DhcpLeaseFail(portType);
        }
        break;

        /* Handle IPv4 DHCP bound and renew event */
        case NM_EVENT_ID_IPV4_DHCP_BOUND:
        {
            IPV4_LAN_CONFIG_t           ipv4AddrInfo = {0};
            NMEvent_Ipv4DhcpAddrInfo_t  *dhcpAddrInfo = (NMEvent_Ipv4DhcpAddrInfo_t*)eventData;

            snprintf(ipv4AddrInfo.lan.ipAddress, sizeof(ipv4AddrInfo.lan.ipAddress), "%s", dhcpAddrInfo->ipAddrInfo.ip);
            snprintf(ipv4AddrInfo.lan.subnetMask, sizeof(ipv4AddrInfo.lan.subnetMask), "%s", dhcpAddrInfo->ipAddrInfo.subnet);
            snprintf(ipv4AddrInfo.lan.gateway, sizeof(ipv4AddrInfo.lan.gateway), "%s", dhcpAddrInfo->gatewayAddrInfo.gateway);
            ipv4AddrInfo.dns.mode = DNS_AUTO;
            snprintf(ipv4AddrInfo.dns.primaryAddress, sizeof(ipv4AddrInfo.dns.primaryAddress), "%s", dhcpAddrInfo->dns1);
            snprintf(ipv4AddrInfo.dns.secondaryAddress, sizeof(ipv4AddrInfo.dns.secondaryAddress), "%s", dhcpAddrInfo->dns2);
            updateIpv4DnsServerAddr(&ipv4AddrInfo);
            OnIpv4DhcpBound(portType, &ipv4AddrInfo);
        }
        break;

        case NM_EVENT_ID_IPV4_DHCP_RENEW:
        {
            IPV4_LAN_CONFIG_t           ipv4AddrInfo = {0};
            NMEvent_Ipv4DhcpAddrInfo_t  *dhcpAddrInfo = (NMEvent_Ipv4DhcpAddrInfo_t*)eventData;

            snprintf(ipv4AddrInfo.lan.ipAddress, sizeof(ipv4AddrInfo.lan.ipAddress), "%s", dhcpAddrInfo->ipAddrInfo.ip);
            snprintf(ipv4AddrInfo.lan.subnetMask, sizeof(ipv4AddrInfo.lan.subnetMask), "%s", dhcpAddrInfo->ipAddrInfo.subnet);
            snprintf(ipv4AddrInfo.lan.gateway, sizeof(ipv4AddrInfo.lan.gateway), "%s", dhcpAddrInfo->gatewayAddrInfo.gateway);
            ipv4AddrInfo.dns.mode = DNS_AUTO;
            snprintf(ipv4AddrInfo.dns.primaryAddress, sizeof(ipv4AddrInfo.dns.primaryAddress), "%s", dhcpAddrInfo->dns1);
            snprintf(ipv4AddrInfo.dns.secondaryAddress, sizeof(ipv4AddrInfo.dns.secondaryAddress), "%s", dhcpAddrInfo->dns2);
            updateIpv4DnsServerAddr(&ipv4AddrInfo);
            OnIpv4DhcpRenew(portType, &ipv4AddrInfo);
        }
        break;

        /* Handle IPv4 PPP ip up event */
        case NM_EVENT_ID_IPV4_PPP_IP_UP:
        {
            IPV4_LAN_CONFIG_t           ipv4AddrInfo = {0};
            NMEvent_Ipv4PppAddrInfo_t   *pppAddrInfo = (NMEvent_Ipv4PppAddrInfo_t*)eventData;

            /* We will get the interface name at runtime */
            if (portType == NETWORK_PORT_USB_MODEM)
            {
                /* Update interface name */
                snprintf(nwIfaceInfo[portType].ifaceName, sizeof(nwIfaceInfo[portType].ifaceName), "%s", pppAddrInfo->pppIfaceName);
            }

            snprintf(ipv4AddrInfo.lan.ipAddress, sizeof(ipv4AddrInfo.lan.ipAddress), "%s", pppAddrInfo->ipAddrInfo.ip);
            snprintf(ipv4AddrInfo.lan.subnetMask, sizeof(ipv4AddrInfo.lan.subnetMask), "%s", pppAddrInfo->ipAddrInfo.subnet);
            snprintf(ipv4AddrInfo.lan.gateway, sizeof(ipv4AddrInfo.lan.gateway), "%s", pppAddrInfo->gatewayAddrInfo.gateway);
            ipv4AddrInfo.dns.mode = DNS_AUTO;
            snprintf(ipv4AddrInfo.dns.primaryAddress, sizeof(ipv4AddrInfo.dns.primaryAddress), "%s", pppAddrInfo->dns1);
            snprintf(ipv4AddrInfo.dns.secondaryAddress, sizeof(ipv4AddrInfo.dns.secondaryAddress), "%s", pppAddrInfo->dns2);
            updateIpv4DnsServerAddr(&ipv4AddrInfo);
            OnIpv4PppoeIpUp(portType, &ipv4AddrInfo);
        }
        break;

        /* Handle IPv4 PPP ip down event */
        case NM_EVENT_ID_IPV4_PPP_IP_DOWN:
        {
            OnIpv4PppoeIpDown(portType);
        }
        break;

        /* Handle IPv6 DAD success event */
        case NM_EVENT_ID_IPV6_STATIC_SUCCESS:
        case NM_EVENT_ID_IPV6_DHCP_SUCCESS:
        case NM_EVENT_ID_IPV6_SLAAC_SUCCESS:
        {
            OnIpv6AddrSuccess(portType);
        }
        break;

        /* Handle IPv6 DAD fail event */
        case NM_EVENT_ID_IPV6_STATIC_DAD:
        case NM_EVENT_ID_IPV6_DHCP_DAD:
        case NM_EVENT_ID_IPV6_SLAAC_DAD:
        {
            OnIpv6AddrDadFail(portType);
        }
        break;

        /* Handle IPv6 DHCP started event */
        case NM_EVENT_ID_IPV6_DHCP_STARTED:
        {
            OnIpv6DhcpStarted(portType);
        }
        break;

        /* Handle IPv6 DHCP unbound event */
        case NM_EVENT_ID_IPV6_DHCP_UNBOUND:
        {
            OnIpv6DhcpUnbound(portType);
        }
        break;

        /* Handle IPv6 DHCP rebound event */
        case NM_EVENT_ID_IPV6_DHCP_REBOUND:
        {
            OnIpv6DhcpUnbound(portType);
        }
        /* FALL THROUGH */
        /* Handle IPv6 DHCP bound event */
        case NM_EVENT_ID_IPV6_DHCP_BOUND:
        {
            IPV6_LAN_CONFIG_t           ipv6AddrInfo = {0};
            NMEvent_Ipv6DhcpAddrInfo_t  *dhcpAddrInfo = (NMEvent_Ipv6DhcpAddrInfo_t*)eventData;

            snprintf(ipv6AddrInfo.lan.ipAddress, sizeof(ipv6AddrInfo.lan.ipAddress), "%s", dhcpAddrInfo->ipAddrInfo.ip);
            ipv6AddrInfo.lan.ipPreferredTime = dhcpAddrInfo->ipAddrInfo.preferredTime;
            ipv6AddrInfo.lan.ipValidTime = dhcpAddrInfo->ipAddrInfo.validTime;
            /* Override RFC's 128-bit prefix length and use a 64-bit prefix for DHCP to handle multiple Ethernet scenarios.
             * When the default gateway is set for LAN2 and the LAN1 IPv6 address is assigned from a DHCPv6 server with a 128-bit prefix,
             * the system will not be able to communicate with other nodes. */
            ipv6AddrInfo.lan.prefixLen = DFLT_IPV6_PREFIX_LEN;
            snprintf(ipv6AddrInfo.lan.gateway, sizeof(ipv6AddrInfo.lan.gateway), "%s", dhcpAddrInfo->gatewayAddrInfo.gateway);
            ipv6AddrInfo.lan.gatewayMetric = dhcpAddrInfo->gatewayAddrInfo.metric;
            ipv6AddrInfo.dns.mode = DNS_AUTO;
            snprintf(ipv6AddrInfo.dns.primaryAddress, sizeof(ipv6AddrInfo.dns.primaryAddress), "%s", dhcpAddrInfo->dns1);
            snprintf(ipv6AddrInfo.dns.secondaryAddress, sizeof(ipv6AddrInfo.dns.secondaryAddress), "%s", dhcpAddrInfo->dns2);
            updateIpv6DnsServerAddr(&ipv6AddrInfo);
            OnIpv6DhcpBoundOrRebound(portType, &ipv6AddrInfo);
        }
        break;

        /* Handle IPv6 DHCP updated event */
        case NM_EVENT_ID_IPV6_DHCP_UPDATED:
        {
            IPV6_LAN_CONFIG_t           ipv6AddrInfo = {0};
            NMEvent_Ipv6DhcpAddrInfo_t  *dhcpAddrInfo = (NMEvent_Ipv6DhcpAddrInfo_t*)eventData;

            snprintf(ipv6AddrInfo.lan.ipAddress, sizeof(ipv6AddrInfo.lan.ipAddress), "%s", dhcpAddrInfo->ipAddrInfo.ip);
            ipv6AddrInfo.lan.ipPreferredTime = dhcpAddrInfo->ipAddrInfo.preferredTime;
            ipv6AddrInfo.lan.ipValidTime = dhcpAddrInfo->ipAddrInfo.validTime;
            /* Override RFC's 128-bit prefix length and use a 64-bit prefix for DHCP to handle multiple Ethernet scenarios.
             * When the default gateway is set for LAN2 and the LAN1 IPv6 address is assigned from a DHCPv6 server with a 128-bit prefix,
             * the system will not be able to communicate with other nodes. */
            ipv6AddrInfo.lan.prefixLen = DFLT_IPV6_PREFIX_LEN;
            snprintf(ipv6AddrInfo.lan.gateway, sizeof(ipv6AddrInfo.lan.gateway), "%s", dhcpAddrInfo->gatewayAddrInfo.gateway);
            ipv6AddrInfo.lan.gatewayMetric = dhcpAddrInfo->gatewayAddrInfo.metric;
            ipv6AddrInfo.dns.mode = DNS_AUTO;
            snprintf(ipv6AddrInfo.dns.primaryAddress, sizeof(ipv6AddrInfo.dns.primaryAddress), "%s", dhcpAddrInfo->dns1);
            snprintf(ipv6AddrInfo.dns.secondaryAddress, sizeof(ipv6AddrInfo.dns.secondaryAddress), "%s", dhcpAddrInfo->dns2);
            updateIpv6DnsServerAddr(&ipv6AddrInfo);
            OnIpv6DhcpUpdated(portType, &ipv6AddrInfo);
        }
        break;

        /* Handle IPv6 SLAAC started event */
        case NM_EVENT_ID_IPV6_SLAAC_STARTED:
        {
            OnIpv6SlaacStarted(portType);
        }
        break;

        /* Handle IPv6 SLAAC RA received event */
        case NM_EVENT_ID_IPV6_SLAAC_RA_RECEIVED:
        {
            IPV6_LAN_CONFIG_t           ipv6AddrInfo = {0};
            NMEvent_Ipv6SlaacAddrInfo_t *slaacAddrInfo = (NMEvent_Ipv6SlaacAddrInfo_t*)eventData;

            snprintf(ipv6AddrInfo.lan.ipAddress, sizeof(ipv6AddrInfo.lan.ipAddress), "%s", slaacAddrInfo->ipAddrInfo.ip);
            ipv6AddrInfo.lan.ipPreferredTime = slaacAddrInfo->ipAddrInfo.preferredTime;
            ipv6AddrInfo.lan.ipValidTime = slaacAddrInfo->ipAddrInfo.validTime;
            ipv6AddrInfo.lan.prefixLen = slaacAddrInfo->ipAddrInfo.prefixLen;
            snprintf(ipv6AddrInfo.lan.gateway, sizeof(ipv6AddrInfo.lan.gateway), "%s", slaacAddrInfo->gatewayAddrInfo.gateway);
            ipv6AddrInfo.lan.gatewayMetric = slaacAddrInfo->gatewayAddrInfo.metric;
            ipv6AddrInfo.dns.mode = DNS_AUTO;
            snprintf(ipv6AddrInfo.dns.primaryAddress, sizeof(ipv6AddrInfo.dns.primaryAddress), "%s", slaacAddrInfo->dns1);
            snprintf(ipv6AddrInfo.dns.secondaryAddress, sizeof(ipv6AddrInfo.dns.secondaryAddress), "%s", slaacAddrInfo->dns2);
            updateIpv6DnsServerAddr(&ipv6AddrInfo);
            OnIpv6SlaacRaReceived(portType, &ipv6AddrInfo);
        }
        break;

        /* Handle IPv6 SLAAC RA updated event */
        case NM_EVENT_ID_IPV6_SLAAC_RA_UPDATED:
        {
            IPV6_LAN_CONFIG_t           ipv6AddrInfo = {0};
            NMEvent_Ipv6SlaacAddrInfo_t *slaacAddrInfo = (NMEvent_Ipv6SlaacAddrInfo_t*)eventData;

            snprintf(ipv6AddrInfo.lan.ipAddress, sizeof(ipv6AddrInfo.lan.ipAddress), "%s", slaacAddrInfo->ipAddrInfo.ip);
            ipv6AddrInfo.lan.ipPreferredTime = slaacAddrInfo->ipAddrInfo.preferredTime;
            ipv6AddrInfo.lan.ipValidTime = slaacAddrInfo->ipAddrInfo.validTime;
            ipv6AddrInfo.lan.prefixLen = slaacAddrInfo->ipAddrInfo.prefixLen;
            snprintf(ipv6AddrInfo.lan.gateway, sizeof(ipv6AddrInfo.lan.gateway), "%s", slaacAddrInfo->gatewayAddrInfo.gateway);
            ipv6AddrInfo.lan.gatewayMetric = slaacAddrInfo->gatewayAddrInfo.metric;
            ipv6AddrInfo.dns.mode = DNS_AUTO;
            snprintf(ipv6AddrInfo.dns.primaryAddress, sizeof(ipv6AddrInfo.dns.primaryAddress), "%s", slaacAddrInfo->dns1);
            snprintf(ipv6AddrInfo.dns.secondaryAddress, sizeof(ipv6AddrInfo.dns.secondaryAddress), "%s", slaacAddrInfo->dns2);
            updateIpv6DnsServerAddr(&ipv6AddrInfo);
            OnIpv6SlaacRaUpdated(portType, &ipv6AddrInfo);
        }
        break;

        /* Print error on unhandled event */
        default:
        {
            EPRINT(ETHERNET, "unhandled event: [ifaceId=%d], [event=%d]", ifaceId, event);
        }
        break;
    }
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
