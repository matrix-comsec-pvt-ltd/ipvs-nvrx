//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		NetworkController.c
@brief      This module provides functionalities with which the Ethernet ports of NVR: Lan1 and Lan2
            are configured as given by the user in the configurations. Then it activates the link of
            the Ethernet Port.  If the configuration mode is Static Mode then this module assigns
            configured IP address and other related configured parameters. If the configuration mode
            is DHCP or PPPoE then DHCP or PPPoE client is executed respectively to retrieve and assign
            IP address and other related parameters from remote server. For this purpose the signal
            handler is registered for signal SIGUSR1 and this signal is raised when DHCP or PPPoE ip-up
            script executes our script lan2Up. Our script signals NVR that all IP related parameters
            are received from the server. Once the parameters are received we assign them to Lan2 in
            our application. Ethernet Module also provides API with which the link status of Ethernet
            port can be found, whether it is UP or DOWN. And using the same function it polls for link
            status to find if the LINK1 or LINK2 is down and if so it retries to configure IP of
            respective link at predefined interval. This Module also updates DNS configuration file
            (resolv.conf) used by Linux DNS resolver, in both cases when the configured DNS settings
            is AUTO or STATIC. In AUTO DNS state the DNS server IP received by DHCP or PPPoE received
            are configured but in STATIC DNS state the user configured DNS server IPs are set.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <net/if.h>
#include <sys/ioctl.h>
#include <net/route.h>
#include <linux/if_ether.h>

/* Library Includes */
#include "nm_iputility.h"

/* Application Includes */
#include "NetworkController.h"
#include "NetworkInterface.h"
#include "SysTimer.h"
#include "DebugLog.h"
#include "Utils.h"
#include "MobileBroadBand.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define GET_PORT_TYPE_FROM_LAN_ID(lanId)    (lanId == LAN1_PORT) ? NETWORK_PORT_LAN1 : NETWORK_PORT_LAN2

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
// LAN interface name
static const CHARPTR    lanInterfaceName[MAX_LAN_PORT] = {"eth0", "eth1"};

/* Network ports config information's working copy */
static LAN_CONFIG_t	 	gNetworkConfig[NETWORK_PORT_MAX];
static pthread_mutex_t 	gNetworkCfgMutex[NETWORK_PORT_MAX];

/* Current network port link status */
static pthread_mutex_t 	gPhyStatusMutex[NETWORK_PORT_MAX];
static BOOL 			gPhyLinkStatus[NETWORK_PORT_MAX] = {LAN_LINK_STATUS_DOWN, LAN_LINK_STATUS_DOWN, MODEM_NOT_PRESENT};

static CHAR 		 	lanMacAddrWithColon[MAX_LAN_PORT][MAX_MAC_ADDRESS_WIDTH];       // Lan Mac Address xx:xx:xx:xx:xx:xx
static CHAR				lanMacAddrWithDash[MAX_LAN_PORT][MAX_MAC_ADDRESS_WIDTH];        // Lan Mac Address xx_xx_xx_xx_xx_xx
static UINT8			lanMacAddrWithoutColon[MAX_LAN_PORT][MAX_MAC_ADDRESS_WIDTH];    // Lan Mac Address xxxxxxxxxxxx

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void getAllLanMacAddr(void);
//-------------------------------------------------------------------------------------------------
static void setIpv4NetworkInfo(NETWORK_PORT_e portType, IPV4_LAN_CONFIG_t *ipv4NetworkInfo);
//-------------------------------------------------------------------------------------------------
static void clearIpv4NetworkInfo(NETWORK_PORT_e portType);
//-------------------------------------------------------------------------------------------------
static void setIpv4LanMode(NETWORK_PORT_e portType, IPV4_LAN_CONFIG_t *ipv4NetworkInfo);
//-------------------------------------------------------------------------------------------------
static void clearIpv4LanMode(NETWORK_PORT_e portType, IPV4_LAN_CONFIG_t *ipv4NetworkInfo, IPV4_LAN_CONFIG_t *prevIpv4NetworkInfo);
//-------------------------------------------------------------------------------------------------
static void clearIpv4LanAddr(NETWORK_PORT_e portType, IPV4_LAN_CONFIG_t *ipv4NetworkInfo, IPV4_LAN_CONFIG_t *prevIpv4NetworkInfo);
//-------------------------------------------------------------------------------------------------
static void setIpv4DefaultGateway(NETWORK_PORT_e portType, IPV4_LAN_CONFIG_t *ipv4NetworkInfo);
//-------------------------------------------------------------------------------------------------
static void clearIpv4DefaultGateway(NETWORK_PORT_e portType);
//-------------------------------------------------------------------------------------------------
static void addRemoveIpv4StaticRoute(NETWORK_PORT_e portType, BOOL action, IPV4_LAN_CONFIG_t *prevIpv4NetworkInfo);
//-------------------------------------------------------------------------------------------------
static BOOL updateIpv4StaticRouteTable(NETWORK_PORT_e portType, const CHAR *networkAddr, UINT8 subnetMask, const CHAR *gateway, BOOL action);
//-------------------------------------------------------------------------------------------------
static void setIpv6NetworkInfo(NETWORK_PORT_e portType, IPV6_LAN_CONFIG_t *ipv6NetworkInfo);
//-------------------------------------------------------------------------------------------------
static void clearIpv6NetworkInfo(NETWORK_PORT_e portType);
//-------------------------------------------------------------------------------------------------
static void setIpv6LanMode(NETWORK_PORT_e portType, IPV6_LAN_CONFIG_t *ipv6NetworkInfo);
//-------------------------------------------------------------------------------------------------
static void clearIpv6LanMode(NETWORK_PORT_e portType, IPV6_LAN_CONFIG_t *ipv6NetworkInfo, IPV6_LAN_CONFIG_t *prevIpv6NetworkInfo);
//-------------------------------------------------------------------------------------------------
static void clearIpv6LanAddr(NETWORK_PORT_e portType, IPV6_LAN_CONFIG_t *ipv6NetworkInfo, IPV6_LAN_CONFIG_t *prevIpv6NetworkInfo);
//-------------------------------------------------------------------------------------------------
static void setIpv6DefaultGateway(NETWORK_PORT_e portType, IPV6_LAN_CONFIG_t *ipv6NetworkInfo);
//-------------------------------------------------------------------------------------------------
static void clearIpv6DefaultGateway(NETWORK_PORT_e portType);
//-------------------------------------------------------------------------------------------------
static void addRemoveIpv6StaticRoute(NETWORK_PORT_e portType, BOOL action, IPV6_LAN_CONFIG_t *prevIpv6NetworkInfo);
//-------------------------------------------------------------------------------------------------
static BOOL updateIpv6StaticRouteTable(NETWORK_PORT_e portType, const CHAR *networkAddr, UINT8 prefixLength, const CHAR *gateway, BOOL action);
//-------------------------------------------------------------------------------------------------
static void setDnsServerAddr(NETWORK_PORT_e portType, LAN_CONFIG_t *networkInfo);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is the function which initializes the Ethernet module, finds the Mac address of
 *          each of the present lan, registers a signal handler which is invoked when lan goes UP
 *          or Down by dynamic Ip assignment by some client like udhcpc and PPPoE. Then this function
 *          creates a timer which checks the link status every one second and goes to sleep.
 */
void InitNetworkController(void)
{
    NETWORK_PORT_e      portType;
    GENERAL_CONFIG_t    generalConfig;

    DPRINT(ETHERNET, "initializing of ethernet module");

    /* Init network manager module */
    InitNetworkInterface();

    for(portType = 0; portType < NETWORK_PORT_MAX; portType++)
	{
        MUTEX_INIT(gPhyStatusMutex[portType], NULL);
        MUTEX_INIT(gNetworkCfgMutex[portType], NULL);

        if (portType == NETWORK_PORT_LAN1)
        {
            /* Get LAN1 config in working copy */
            ReadLan1Config(&gNetworkConfig[portType]);
            if (gNetworkConfig[portType].ipv4.ipAssignMode != IPV4_ASSIGN_STATIC)
            {
                clearIpv4NetworkInfo(portType);
            }

            if (gNetworkConfig[portType].ipv6.ipAssignMode != IPV6_ASSIGN_STATIC)
            {
                clearIpv6NetworkInfo(portType);
            }
        }
        else if (portType == NETWORK_PORT_LAN2)
        {
            /* Get LAN2 config in working copy */
            ReadLan2Config(&gNetworkConfig[portType]);
        }
        else
        {
            /* Reset working copy config for other ports */
            clearIpv4NetworkInfo(portType);
            clearIpv6NetworkInfo(portType);
            gNetworkConfig[portType].ipAddrMode = IP_ADDR_MODE_DUAL_STACK;
        }

        /* Register port only which are physically available */
        if (portType >= GetNoOfLanPort())
        {
            continue;
        }

        /* Register network port in network manager module */
        RegisterNetworkPort(portType, lanInterfaceName[portType]);

        /* Enable IPv6 Stack if ip addressing mode is IPv4 & IPv6 */
        if (gNetworkConfig[portType].ipAddrMode == IP_ADDR_MODE_DUAL_STACK)
        {
            EnableIpv6OnInterface(portType);
        }
	}

    /* Get MAC address of all LAN ports */
    getAllLanMacAddr();

    /* Read general config and set hostname */
    ReadGeneralConfig(&generalConfig);
    SetSystemHostName(generalConfig.deviceName);

    /* Init usb modem info */
    InitMobileBroadband();
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function deinit the network manager
 */
void DeInitNetworkController(void)
{
    /* Deinit network manager module */
    DeInitNetworkInterface();
    DPRINT(ETHERNET, "deinit of network controller module");
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get LAN port interface name
 * @param   lanId
 * @return  Interface name
 */
const CHAR *GetLanPortName(LAN_CONFIG_ID_e lanId)
{
    /* Returns LAN port name */
    return lanInterfaceName[lanId];
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function stores the MAC address of lan Id to the output parameter.
 * @param   lanId
 * @param   macAddrStr
 * @return  It returns SUCCESS if the LAN index does not exceed the maximum LAN port otherwise it returns FAIL.
 */
BOOL GetMacAddr(LAN_CONFIG_ID_e lanId, CHARPTR macAddrStr)
{
    if (lanId >= GetNoOfLanPort())
    {
        return FAIL;
    }

    snprintf(macAddrStr, MAX_MAC_ADDRESS_WIDTH, "%s", lanMacAddrWithColon[lanId]);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function stores the MAC address of lan Id to the output parameter like xx-xx-xx-xx-xx-xx.
 * @param   lanId
 * @param   macAddrStr
 * @return  It returns SUCCESS if the LAN index not exceed the maximum LAN port otherwise it returns FAIL.
 */
BOOL GetMacAddrPrepare(LAN_CONFIG_ID_e lanId, CHARPTR macAddrStr)
{
    if (lanId >= GetNoOfLanPort())
    {
        return FAIL;
    }

    snprintf(macAddrStr, MAX_MAC_ADDRESS_WIDTH, "%s", lanMacAddrWithDash[lanId]);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function stores the MAC address of lan Id without colon in string
 * @param   lanId
 * @param   rawMacAddrStr
 * @return  It returns SUCCESS if the LAN index not exceed the maximum LAN port otherwise it returns FAIL.
 */
BOOL GetRawMacAddrStr(LAN_CONFIG_ID_e lanId, CHARPTR rawMacAddrStr)
{
    if (lanId >= GetNoOfLanPort())
    {
        return FAIL;
    }

    snprintf(rawMacAddrStr, MAX_MAC_ADDRESS_WIDTH, "%s", lanMacAddrWithoutColon[lanId]);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function finds out the Mac address of all lan interfaces
 */
void getAllLanMacAddr(void)
{
    UINT8 lanId, byteCnt, charCnt;

    /* Reset all mac addresses */
    memset(lanMacAddrWithColon, 0, sizeof(lanMacAddrWithColon));
    memset(lanMacAddrWithDash, 0, sizeof(lanMacAddrWithDash));
    memset(lanMacAddrWithoutColon, 0, sizeof(lanMacAddrWithoutColon));

    for (lanId = LAN1_PORT; lanId < MAX_LAN_PORT; lanId++)
    {
        /* Is lan port supported? */
        if (lanId >= GetNoOfLanPort())
        {
            continue;
        }

        /* Get mac address of network port */
        if (FAIL == GetNetworkPortMacAddr(GET_PORT_TYPE_FROM_LAN_ID(lanId), lanMacAddrWithColon[lanId]))
        {
            EPRINT(ETHERNET, "fail to get mac addr: [lan=%s]", lanInterfaceName[lanId]);
            continue;
        }

        /* Convert mac address in other format as per need */
        charCnt = 0;
        for (byteCnt = 0; byteCnt < MAX_MAC_ADDRESS_WIDTH; byteCnt++)
        {
            /* Replace ':' with '_' in one format and remove ':' in other format */
            if (lanMacAddrWithColon[lanId][byteCnt] == ':')
            {
                lanMacAddrWithDash[lanId][byteCnt] = '_';
            }
            else
            {
                lanMacAddrWithDash[lanId][byteCnt] = lanMacAddrWithColon[lanId][byteCnt];
                lanMacAddrWithoutColon[lanId][charCnt++] = lanMacAddrWithColon[lanId][byteCnt];
            }
        }

        DPRINT(SYS_LOG, "mac address found: [lan=%s], [colonMac=%s]", lanInterfaceName[lanId], lanMacAddrWithColon[lanId]);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is called when user has manually made changes in the network parameters of
 *          any of the Lan Port. The new configuration parameters are read from configuration module
 *          for the lan of Lan Index No
 * @param   lanNo
 * @param   newLanConfig
 * @param   oldLanConfig
 * @return
 */
BOOL UpdateLanConfig(LAN_CONFIG_ID_e lanId, LAN_CONFIG_t newLanConfig, LAN_CONFIG_t *oldLanConfig)
{
    BOOL            ipv4ReinitDone = FALSE;
    BOOL            ipv6ReinitDone = FALSE;
    BOOL            setDnsAddr = FALSE;
    NETWORK_PORT_e  portType = GET_PORT_TYPE_FROM_LAN_ID(lanId);
    LAN_CONFIG_t    prevLanConfig;

    DPRINT(ETHERNET, "updating the new configurations: [portType=%d]", portType);
    MUTEX_LOCK(gNetworkCfgMutex[portType]);
    prevLanConfig = gNetworkConfig[portType];
    gNetworkConfig[portType] = newLanConfig;
    MUTEX_UNLOCK(gNetworkCfgMutex[portType]);

    do
    {
        /* Lan configurations for IPv4 method */
        if ((newLanConfig.ipv4.ipAssignMode != oldLanConfig->ipv4.ipAssignMode)
                || ((newLanConfig.ipv4.ipAssignMode == IPV4_ASSIGN_PPPOE)
                    && ((strcmp(newLanConfig.ipv4.pppoe.username, oldLanConfig->ipv4.pppoe.username) != STATUS_OK)
                        || (strcmp(newLanConfig.ipv4.pppoe.password, oldLanConfig->ipv4.pppoe.password) != STATUS_OK))))
        {
            clearIpv4LanMode(portType, &newLanConfig.ipv4, &prevLanConfig.ipv4);
            setIpv4LanMode(portType, &newLanConfig.ipv4);
            ipv4ReinitDone = TRUE;
        }
        else if ((newLanConfig.ipv4.ipAssignMode == IPV4_ASSIGN_STATIC)
                 && ((strcmp(newLanConfig.ipv4.lan.ipAddress, oldLanConfig->ipv4.lan.ipAddress) != STATUS_OK)
                     || (strcmp(newLanConfig.ipv4.lan.subnetMask, oldLanConfig->ipv4.lan.subnetMask) != STATUS_OK)
                     || (strcmp(newLanConfig.ipv4.lan.gateway, oldLanConfig->ipv4.lan.gateway) != STATUS_OK)))
        {
            clearIpv4LanMode(portType, &newLanConfig.ipv4, &prevLanConfig.ipv4);
            setIpv4LanMode(portType, &newLanConfig.ipv4);
            ipv4ReinitDone = TRUE;
        }

        /* DNS configurations for IPv4 method */
        if (newLanConfig.ipv4.dns.mode != oldLanConfig->ipv4.dns.mode)
        {
            if (newLanConfig.ipv4.dns.mode == DNS_STATIC)
            {
                setDnsAddr = TRUE;
            }
            else
            {
                if (FALSE == ipv4ReinitDone)
                {
                    clearIpv4LanMode(portType, &newLanConfig.ipv4, &prevLanConfig.ipv4);
                    setIpv4LanMode(portType, &newLanConfig.ipv4);
                }
            }
        }
        else if (newLanConfig.ipv4.dns.mode == DNS_STATIC)
        {
            if ((strcmp(newLanConfig.ipv4.dns.primaryAddress, oldLanConfig->ipv4.dns.primaryAddress) != STATUS_OK)
                    || (strcmp(newLanConfig.ipv4.dns.secondaryAddress, oldLanConfig->ipv4.dns.secondaryAddress) != STATUS_OK))
            {
                setDnsAddr = TRUE;
            }
        }

        /* Lan configurations for IPv6 method */
        if (newLanConfig.ipAddrMode != oldLanConfig->ipAddrMode)
        {
            if (newLanConfig.ipAddrMode == IP_ADDR_MODE_DUAL_STACK)
            {
                EnableIpv6OnInterface(portType);
                clearIpv6LanMode(portType, &newLanConfig.ipv6, &prevLanConfig.ipv6);
                setIpv6LanMode(portType, &newLanConfig.ipv6);
            }
            else
            {
                clearIpv6LanMode(portType, &newLanConfig.ipv6, &prevLanConfig.ipv6);
                DisableIpv6OnInterface(portType);
            }

            break;
        }

        if (newLanConfig.ipv6.ipAssignMode != oldLanConfig->ipv6.ipAssignMode)
        {
            clearIpv6LanMode(portType, &newLanConfig.ipv6, &prevLanConfig.ipv6);
            setIpv6LanMode(portType, &newLanConfig.ipv6);
            ipv6ReinitDone = TRUE;
        }
        else if ((newLanConfig.ipv6.ipAssignMode == IPV6_ASSIGN_STATIC)
                     && ((strcmp(newLanConfig.ipv6.lan.ipAddress, oldLanConfig->ipv6.lan.ipAddress) != STATUS_OK)
                     || (newLanConfig.ipv6.lan.prefixLen != oldLanConfig->ipv6.lan.prefixLen)
                     || (strcmp(newLanConfig.ipv6.lan.gateway, oldLanConfig->ipv6.lan.gateway) != STATUS_OK)))
        {
            clearIpv6LanMode(portType, &newLanConfig.ipv6, &prevLanConfig.ipv6);
            setIpv6LanMode(portType, &newLanConfig.ipv6);
            ipv6ReinitDone = TRUE;
        }

        /* DNS configurations for IPv6 method */
        if (newLanConfig.ipv6.dns.mode != oldLanConfig->ipv6.dns.mode)
        {
            if (newLanConfig.ipv6.dns.mode == DNS_STATIC)
            {
                setDnsAddr = TRUE;
            }
            else
            {
                if (FALSE == ipv6ReinitDone)
                {
                    clearIpv6LanMode(portType, &newLanConfig.ipv6, &prevLanConfig.ipv6);
                    setIpv6LanMode(portType, &newLanConfig.ipv6);
                }
            }
        }
        else if (newLanConfig.ipv6.dns.mode == DNS_STATIC)
        {
            if ((strcmp(newLanConfig.ipv6.dns.primaryAddress, oldLanConfig->ipv6.dns.primaryAddress) != STATUS_OK)
                    || (strcmp(newLanConfig.ipv6.dns.secondaryAddress, oldLanConfig->ipv6.dns.secondaryAddress) != STATUS_OK))
            {
                setDnsAddr = TRUE;
            }
        }

    } while(0);

    if (TRUE == setDnsAddr)
    {
        setDnsServerAddr(portType, &newLanConfig);
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will Validate the change in Static routing table or Default routing Port.
 *          After validation  reflects the Change in Kernel's Routing Table.
 * @param   newCopy
 * @param   oldCopy
 * @return
 */
BOOL UpdateStaticRouting(STATIC_ROUTING_CONFIG_t newCopy, STATIC_ROUTING_CONFIG_t *oldCopy)
{
    BOOL                retVal = SUCCESS;
    UINT8               entry;
    NM_IpAddrFamily_e   ipAddrFamily;
    LAN_CONFIG_t        networkConfig = {0};

    // Check if the default Routing port Has been Changed
    if (newCopy.defaultPort != oldCopy->defaultPort)
    {
        /* Clear old default gateway */
        RemoveIpv4DefaultGateway(oldCopy->defaultPort - 1);
        RemoveIpv6DefaultGateway(oldCopy->defaultPort - 1);

        /* Get network port info */
        newCopy.defaultPort--;
        GetNetworkParamInfo(newCopy.defaultPort, &networkConfig);
        setIpv4DefaultGateway(newCopy.defaultPort, &networkConfig.ipv4);
        setIpv6DefaultGateway(newCopy.defaultPort, &networkConfig.ipv6);
        setDnsServerAddr(newCopy.defaultPort, &networkConfig);
    }

    //Check Which Entry has changed
    for(entry = 0; entry < MAX_STATIC_ROUTING_ENTRY; entry++)
    {
        if (memcmp(&oldCopy->entry[entry], &newCopy.entry[entry], sizeof(ROUTING_ENTRY_t)) == STATUS_OK)
        {
            continue;
        }

        /* Get old entry ip address family */
        ipAddrFamily = NMIpUtil_GetIpAddrFamily(oldCopy->entry[entry].networkAddr);
        oldCopy->entry[entry].routePort--;

        /* Get network config of previous route port */
        GetNetworkParamInfo(oldCopy->entry[entry].routePort, &networkConfig);

        /* Check address family and process as per type */
        if (NM_IPADDR_FAMILY_V4 == ipAddrFamily)
        {
            updateIpv4StaticRouteTable(oldCopy->entry[entry].routePort, oldCopy->entry[entry].networkAddr,
                                       oldCopy->entry[entry].subnetMask, networkConfig.ipv4.lan.gateway, CLEAR);
        }
        else if (NM_IPADDR_FAMILY_V6 == ipAddrFamily)
        {
            updateIpv6StaticRouteTable(oldCopy->entry[entry].routePort, oldCopy->entry[entry].networkAddr,
                                       oldCopy->entry[entry].subnetMask, networkConfig.ipv6.lan.gateway, CLEAR);
        }

        /* Get new entry ip address family */
        ipAddrFamily = NMIpUtil_GetIpAddrFamily(newCopy.entry[entry].networkAddr);
        newCopy.entry[entry].routePort--;

        /* Get network config of new route port */
        GetNetworkParamInfo(newCopy.entry[entry].routePort, &networkConfig);

        /* Check address family and process as per type */
        if (NM_IPADDR_FAMILY_V4 == ipAddrFamily)
        {
            retVal = updateIpv4StaticRouteTable(newCopy.entry[entry].routePort, newCopy.entry[entry].networkAddr,
                                                newCopy.entry[entry].subnetMask, networkConfig.ipv4.lan.gateway, SET);
        }
        else if (NM_IPADDR_FAMILY_V6 == ipAddrFamily)
        {
            retVal = updateIpv6StaticRouteTable(newCopy.entry[entry].routePort, newCopy.entry[entry].networkAddr,
                                                newCopy.entry[entry].subnetMask, networkConfig.ipv6.lan.gateway, SET);
        }
    }

    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function Gives notification of configuration was changed and it gives a local copy
 *          of updated config.
 * @param   updateConfig
 * @param   oldGenCfg
 */
void NcGeneralCfgUpdate(GENERAL_CONFIG_t updateConfig, GENERAL_CONFIG_t *oldGenCfg)
{
    if(strcmp(oldGenCfg->deviceName, updateConfig.deviceName) != 0)
    {
        SetSystemHostName(updateConfig.deviceName);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is validate lan configuration.
 * @param   lanId
 * @param   newLanConfig
 * @return
 */
NET_CMD_STATUS_e ValidateLanConfig(LAN_CONFIG_ID_e lanId, LAN_CONFIG_t newLanConfig)
{
	NET_CMD_STATUS_e	retVal = CMD_SUCCESS;
	UINT32				lanAddrN[MAX_LAN_PORT]  = { 0 };
	UINT32				subNetAddrN[MAX_LAN_PORT] = { 0 };
	UINT32				gateWayAddrN[MAX_LAN_PORT] = { 0 };
    NETWORK_PORT_e      portType = GET_PORT_TYPE_FROM_LAN_ID(lanId);
    UINT8               otherLanPort = (portType == NETWORK_PORT_LAN1) ? NETWORK_PORT_LAN2 : NETWORK_PORT_LAN1;

    if (newLanConfig.ipv4.ipAssignMode != IPV4_ASSIGN_STATIC)
	{
        return CMD_SUCCESS;
    }

    do
    {
        if (GetNoOfLanPort() > LAN2_PORT)
        {
            MUTEX_LOCK(gNetworkCfgMutex[otherLanPort]);

            // Verify Duplicate IPv4 Address
            if (strcmp(gNetworkConfig[otherLanPort].ipv4.lan.ipAddress, newLanConfig.ipv4.lan.ipAddress) == STATUS_OK)
            {
                EPRINT(ETHERNET, "duplicate ip addr detected: [ip=%s]", newLanConfig.ipv4.lan.ipAddress);
                retVal = CMD_DUP_IP_ADDR;
                break;
            }
        }

        // Check If Whether GateWay Lies in same Network
        if(inet_pton(AF_INET, newLanConfig.ipv4.lan.ipAddress, &lanAddrN[portType]) != 1)
        {
            EPRINT(ETHERNET, "failed to convert new ip in nw format: [ip=%s]", newLanConfig.ipv4.lan.ipAddress);
            retVal = CMD_PROCESS_ERROR;
            break;
        }

        if(inet_pton(AF_INET, newLanConfig.ipv4.lan.subnetMask, &subNetAddrN[portType]) != 1)
        {
            EPRINT(ETHERNET, "failed to convert new subnet in nw format: [subnet=%s]", newLanConfig.ipv4.lan.subnetMask);
            retVal = CMD_PROCESS_ERROR;
            break;
        }

        if(inet_pton(AF_INET, newLanConfig.ipv4.lan.gateway, &gateWayAddrN[portType]) != 1)
        {
            EPRINT(ETHERNET, "failed to convert new gateway in nw format: [gateway=%s]", newLanConfig.ipv4.lan.gateway);
            retVal = CMD_PROCESS_ERROR;
            break;
        }

        if ((lanAddrN[portType] & subNetAddrN[portType]) != (gateWayAddrN[portType] & subNetAddrN[portType]))
        {
            EPRINT(ETHERNET, "gateway is not in same subnet");
            retVal = CMD_IP_GATEWAY_SAME_SUBNET;
        }

        if (GetNoOfLanPort() <= LAN2_PORT)
        {
            break;
        }

        // If the lan1 port has static ip check for same subnet
        if (gNetworkConfig[otherLanPort].ipv4.ipAssignMode != IPV4_ASSIGN_STATIC)
        {
            break;
        }

        // Check If Whether lan1 & lan2 has same subnet
        if (inet_pton(AF_INET, gNetworkConfig[otherLanPort].ipv4.lan.subnetMask, &subNetAddrN[otherLanPort]) != 1)
        {
            EPRINT(ETHERNET, "failed to convert cur subnet in nw format: [subnet=%s]", gNetworkConfig[otherLanPort].ipv4.lan.subnetMask);
            retVal = CMD_PROCESS_ERROR;
            break;
        }

        if (inet_pton(AF_INET, gNetworkConfig[otherLanPort].ipv4.lan.gateway, &gateWayAddrN[otherLanPort]) != 1)
        {
            EPRINT(ETHERNET, "failed to convert cur gateway in nw format: [gateway=%s]", gNetworkConfig[otherLanPort].ipv4.lan.gateway);
            retVal = CMD_PROCESS_ERROR;
            break;
        }

        /* Check if the other Lan's SubNet is bigger than this. NOTE: comparison is '>' because in Network Byte Order 255.255.255.0 if 0x00FFFFFF */
        if (subNetAddrN[portType] > subNetAddrN[otherLanPort])
        {
            // Mask the gateway with subnet of bigger network
            gateWayAddrN[portType] &= subNetAddrN[otherLanPort];
            gateWayAddrN[otherLanPort] &= subNetAddrN[otherLanPort];
        }
        else
        {
            gateWayAddrN[portType] &= subNetAddrN[portType];
            gateWayAddrN[otherLanPort] &= subNetAddrN[portType];
        }

        // Check if GateWay Address is same
        if (gateWayAddrN[portType] == gateWayAddrN[otherLanPort])
        {
            EPRINT(ETHERNET, "lan1 and lan2 should not be in same network");
            retVal = CMD_LAN1_LAN2_SAME_SUBNET;
            break;
        }

        // Verify Duplicate IPv6 Address if IP Addressing Mode is IPv4 & IPv6
        if ((newLanConfig.ipAddrMode == IP_ADDR_MODE_DUAL_STACK) && (newLanConfig.ipv6.ipAssignMode == IPV6_ASSIGN_STATIC))
        {
            if (strcmp(gNetworkConfig[otherLanPort].ipv6.lan.ipAddress, newLanConfig.ipv6.lan.ipAddress) == STATUS_OK)
            {
                EPRINT(ETHERNET, "duplicate ip addr detected: [ip=%s]", newLanConfig.ipv6.lan.ipAddress);
                retVal = CMD_DUP_IP_ADDR;
                break;
            }
        }

    }while(0);

    if(GetNoOfLanPort() > LAN2_PORT)
    {
        MUTEX_UNLOCK(gNetworkCfgMutex[otherLanPort]);
    }

	return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will Validate the change in Static routing table.
 * @param   newCfg
 * @return
 */
NET_CMD_STATUS_e ValidateStaticRoutingCfg(STATIC_ROUTING_CONFIG_t newCfg)
{
    NET_CMD_STATUS_e	retVal = CMD_SUCCESS;
    UINT8 				loop = 0;
    UINT32              subnetMask;
    CHAR                subnetMaskStr[IPV4_ADDR_LEN_MAX];
    IP_NW_ADDR_u        staticRoutEntry, lan1Addr, lan2Addr, hostAddr;

    /* Check Which Entry has changed */
    for(loop = 0; loop < MAX_STATIC_ROUTING_ENTRY; loop++)
    {
        if (newCfg.entry[loop].networkAddr[0] == '\0')
        {
            continue;
        }

        newCfg.entry[loop].routePort--;
        if (newCfg.entry[loop].routePort >= NETWORK_PORT_MAX)
        {
            continue;
        }

        if (inet_pton(AF_INET, newCfg.entry[loop].networkAddr, &staticRoutEntry.ip4) == 1)
        {
            /* Construct subnet mask in string format */
            subnetMask = htonl((__UINT32_MAX__ << (newCfg.entry[loop].subnetMask - 1)));
            inet_ntop(AF_INET, &subnetMask, subnetMaskStr, sizeof(subnetMaskStr));

            /* Get host address using subnet mask */
            GetHostAddress(newCfg.entry[loop].networkAddr, subnetMaskStr, AF_INET, &hostAddr);

            /* Check if any host bit is present */
            if (hostAddr.ip4 != 0)
            {
                EPRINT(ETHERNET, "invld network address: [entry=%d]", loop);
                retVal = CMD_DEST_ADDR_IS_NOT_NW_ADDR;
                break;
            }

            /* Get Network Address based on subnet mask */
            GetNetworkAddress(newCfg.entry[loop].networkAddr, subnetMaskStr, AF_INET, &staticRoutEntry);

            /* Compare network mask of LAN 1 only if ip is statically assigned (Applicable only for IPv4)*/
            if (lanCfg[LAN1_PORT].ipv4.ipAssignMode == IPV4_ASSIGN_STATIC)
            {
                GetNetworkAddress(lanCfg[LAN1_PORT].ipv4.lan.ipAddress, &lanCfg[LAN1_PORT].ipv4.lan.subnetMask, AF_INET, &lan1Addr);

                /* Check if static rout entry matches with LAN 1 network */
                if (staticRoutEntry.ip4 == lan1Addr.ip4)
                {
                    EPRINT(ETHERNET, "addr should not be in the subnet of lan1");
                    retVal = CMD_ERROR_SUBMIT_ROUTE_TABLE;
                    break;
                }
            }

            if (GetNoOfLanPort() > LAN2_PORT)
            {
                /* Get Network Address based on subnet mask */
                GetNetworkAddress(lanCfg[LAN2_PORT].ipv4.lan.ipAddress, &lanCfg[LAN2_PORT].ipv4.lan.subnetMask, AF_INET, &lan2Addr);

                /* Check if static rout entry matches with LAN 2 network */
                if (staticRoutEntry.ip4 == lan2Addr.ip4)
                {
                    EPRINT(ETHERNET, "addr should not be in the subnet of lan2");
                    retVal = CMD_ERROR_SUBMIT_ROUTE_TABLE;
                    break;
                }
            }
        }
        else if (inet_pton(AF_INET6, newCfg.entry[loop].networkAddr, &staticRoutEntry.ip6) == 1)
        {
            /* Get host address using prefix length */
            GetHostAddress(newCfg.entry[loop].networkAddr, &newCfg.entry[loop].subnetMask, AF_INET6, &hostAddr);

            /* Check if any host bit is present */
            if (FALSE == IN6_IS_ADDR_UNSPECIFIED(&hostAddr.ip6))
            {
                EPRINT(ETHERNET, "invld network address: [entry=%d]", loop);
                retVal = CMD_DEST_ADDR_IS_NOT_NW_ADDR;
                break;
            }

            if (strcmp(lanCfg[LAN1_PORT].ipv6.lan.ipAddress, "") != STATUS_OK)
            {
                /* Get Network Address based on prefix length */
                GetNetworkAddress(newCfg.entry[loop].networkAddr, &newCfg.entry[loop].subnetMask, AF_INET6, &staticRoutEntry);
                GetNetworkAddress(lanCfg[LAN1_PORT].ipv6.lan.ipAddress, &lanCfg[LAN1_PORT].ipv6.lan.prefixLen, AF_INET6, &lan1Addr);

                /* Check if static rout entry matches with LAN 1 network */
                if (TRUE == IN6_ARE_ADDR_EQUAL(&staticRoutEntry.ip6, &lan1Addr.ip6))
                {
                    EPRINT(ETHERNET, "addr should not be in the subnet of lan1");
                    retVal = CMD_ERROR_SUBMIT_ROUTE_TABLE;
                    break;
                }
            }

            if ((GetNoOfLanPort() > LAN2_PORT) && (strcmp(lanCfg[LAN2_PORT].ipv6.lan.ipAddress, "") != STATUS_OK))
            {
                /* Get Network Address based on prefix length */
                GetNetworkAddress(lanCfg[LAN2_PORT].ipv6.lan.ipAddress, &lanCfg[LAN2_PORT].ipv6.lan.prefixLen, AF_INET6, &lan2Addr);

                /* Check if static rout entry matches with LAN 2 network */
                if (TRUE == IN6_ARE_ADDR_EQUAL(&staticRoutEntry.ip6, &lan2Addr.ip6))
                {
                    EPRINT(ETHERNET, "addr should not be in the subnet of lan2");
                    retVal = CMD_ERROR_SUBMIT_ROUTE_TABLE;
                    break;
                }
            }
        }
        else
        {
            EPRINT(ETHERNET, "entered ip address is invalid: [ip:%s]", newCfg.entry[loop].networkAddr);
            retVal = CMD_PROCESS_ERROR;
            break;
        }
    }

    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function checks the status of the link which is passed in the first argument of the function.
 * @param   portType
 * @return  If the interface is up it returns UP otherwise it returns DOWN.
 */
BOOL IsNetworkPortLinkUp(NETWORK_PORT_e portType)
{
    BOOL retVal;

    if (portType == NETWORK_PORT_USB_MODEM)
    {
        MUTEX_LOCK(gPhyStatusMutex[portType]);
        retVal = (gPhyLinkStatus[portType] == MODEM_CONNECTED) ? TRUE : FALSE;
        MUTEX_UNLOCK(gPhyStatusMutex[portType]);
    }
    else
    {
        /* PARASOFT : Rule BD-TRS-LOCK marked as false positive */
        MUTEX_LOCK(gPhyStatusMutex[portType]);
        retVal = (gPhyLinkStatus[portType] == LAN_LINK_STATUS_UP) ? TRUE : FALSE;
        MUTEX_UNLOCK(gPhyStatusMutex[portType]);
    }

    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function checks the status of the link which is passed in the first argument of the function.
 * @param   portType
 * @return  If the interface is up it returns UP otherwise it returns DOWN.
 */
BOOL GetNetworkPortLinkStatus(NETWORK_PORT_e portType)
{
    MUTEX_LOCK(gPhyStatusMutex[portType]);
    BOOL retVal = gPhyLinkStatus[portType];
    MUTEX_UNLOCK(gPhyStatusMutex[portType]);
    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set link status of physical interface
 * @param   portType
 * @param   status
 */
void SetNetworkPortLinkStatus(NETWORK_PORT_e portType, BOOL status)
{
    MUTEX_LOCK(gPhyStatusMutex[portType]);
    gPhyLinkStatus[portType] = status;
    MUTEX_UNLOCK(gPhyStatusMutex[portType]);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function stores the lan's parameters of lan Id into the output parameter.
 * @param   portType
 * @param   networkInfo
 * @return  It returns SUCCESS if the LAN index does not exceed the maximum LAN port otherwise it returns FAIL.
 */
BOOL GetNetworkParamInfo(NETWORK_PORT_e portType, LAN_CONFIG_t *networkInfo)
{
    if ((portType != NETWORK_PORT_USB_MODEM) && (portType >= GetNoOfLanPort()))
	{
        return FAIL;
    }

    /* PARASOFT : Rule BD-TRS-LOCK marked as false positive */
    MUTEX_LOCK(gNetworkCfgMutex[portType]);
    *networkInfo = gNetworkConfig[portType];
    MUTEX_UNLOCK(gNetworkCfgMutex[portType]);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Check If IPv6 is enebled on any interface
 * @return  status TRUE/FALSE
 */
BOOL IsIpv6Enabled()
{
    NETWORK_PORT_e  routePort;
    LAN_CONFIG_t    lanConfig = {0};

    for (routePort = NETWORK_PORT_LAN1; routePort < GetNoOfLanPort(); routePort++)
    {
        GetNetworkParamInfo(routePort, &lanConfig);
        if (lanConfig.ipAddrMode == IP_ADDR_MODE_DUAL_STACK)
        {
            return TRUE;
        }
    }

    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set IPv4 network parameters information
 * @param   portType
 * @param   ipv4NetworkInfo
 */
static void setIpv4NetworkInfo(NETWORK_PORT_e portType, IPV4_LAN_CONFIG_t *ipv4NetworkInfo)
{
    MUTEX_LOCK(gNetworkCfgMutex[portType]);
    gNetworkConfig[portType].ipv4 = *ipv4NetworkInfo;
    MUTEX_UNLOCK(gNetworkCfgMutex[portType]);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function reset the ipv4 parameters of network port
 * @param   portType
 */
static void clearIpv4NetworkInfo(NETWORK_PORT_e portType)
{
    MUTEX_LOCK(gNetworkCfgMutex[portType]);
    memset(&gNetworkConfig[portType].ipv4.lan, 0, sizeof(gNetworkConfig[portType].ipv4.lan));

    /* Set DNS auto for usb modem */
    if (portType == NETWORK_PORT_USB_MODEM)
    {
        gNetworkConfig[portType].ipv4.ipAssignMode = IPV4_ASSIGN_DHCP;
        gNetworkConfig[portType].ipv4.dns.mode = DNS_AUTO;
    }

    /* Reset DNS address if mode is auto */
    if(gNetworkConfig[portType].ipv4.dns.mode == DNS_AUTO)
    {
        memset(gNetworkConfig[portType].ipv4.dns.primaryAddress, 0, sizeof(gNetworkConfig[portType].ipv4.dns.primaryAddress));
        memset(gNetworkConfig[portType].ipv4.dns.secondaryAddress, 0, sizeof(gNetworkConfig[portType].ipv4.dns.secondaryAddress));
    }
    MUTEX_UNLOCK(gNetworkCfgMutex[portType]);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set IPv4 LAN address assignment mode
 * @param   portType
 * @param   ipv4NetworkInfo
 */
static void setIpv4LanMode(NETWORK_PORT_e portType, IPV4_LAN_CONFIG_t *ipv4NetworkInfo)
{
    /* Set IPv4 address mode as per configuration */
    if (ipv4NetworkInfo->ipAssignMode == IPV4_ASSIGN_DHCP)
    {
        SetIpv4DhcpMode(portType);
    }
    else if (ipv4NetworkInfo->ipAssignMode == IPV4_ASSIGN_PPPOE)
    {
        SetIpv4PppoeMode(portType, &ipv4NetworkInfo->pppoe);
    }
    else /* Static */
    {
        SetIpv4StaticMode(portType, &ipv4NetworkInfo->lan);
        setIpv4DefaultGateway(portType, ipv4NetworkInfo);
        addRemoveIpv4StaticRoute(portType, SET, ipv4NetworkInfo);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Clear IPv4 LAN address assignment mode
 * @param   portType
 * @param   ipv4NetworkInfo
 * @param   prevIpv4NetworkInfo
 */
static void clearIpv4LanMode(NETWORK_PORT_e portType, IPV4_LAN_CONFIG_t *ipv4NetworkInfo, IPV4_LAN_CONFIG_t *prevIpv4NetworkInfo)
{
    /* Clear the all IP address from interface */
    clearIpv4LanAddr(portType, ipv4NetworkInfo, prevIpv4NetworkInfo);

    /* Remove the IPv4 assignment mode */
    RemoveIpv4Mode(portType);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Clear IPv4 LAN address
 * @param   portType
 * @param   ipv4NetworkInfo
 * @param   prevIpv4NetworkInfo
 */
static void clearIpv4LanAddr(NETWORK_PORT_e portType, IPV4_LAN_CONFIG_t *ipv4NetworkInfo, IPV4_LAN_CONFIG_t *prevIpv4NetworkInfo)
{
    /* Clear the all IP address from interface */
    addRemoveIpv4StaticRoute(portType, CLEAR, prevIpv4NetworkInfo);
    clearIpv4DefaultGateway(portType);
    RemoveIpv4Addr(portType);

    /* Clear local ip address if assigned dynamically */
    if (ipv4NetworkInfo->ipAssignMode != IPV4_ASSIGN_STATIC)
    {
        clearIpv4NetworkInfo(portType);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set IPv4 default gateway on interface
 * @param   portType
 * @param   ipv4NetworkInfo
 */
static void setIpv4DefaultGateway(NETWORK_PORT_e portType, IPV4_LAN_CONFIG_t *ipv4NetworkInfo)
{
    STATIC_ROUTING_CONFIG_t staticRouteConfig;

    // Read static route config
    ReadStaticRoutingConfig(&staticRouteConfig);

    /* Is given port is default route port? */
    if ((staticRouteConfig.defaultPort - 1) != portType)
    {
        /* It is not a default route port */
        return;
    }

    /* Set IPv4 default gateway */
    AddIpv4DefaultGateway(portType, &ipv4NetworkInfo->lan);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Clear IPv4 default gateway from interface
 * @param   portType
 */
static void clearIpv4DefaultGateway(NETWORK_PORT_e portType)
{
    STATIC_ROUTING_CONFIG_t staticRouteConfig;

    // Read static route config
    ReadStaticRoutingConfig(&staticRouteConfig);

    /* Is given port is default route port? */
    if ((staticRouteConfig.defaultPort - 1) != portType)
    {
        /* It is not a default route port */
        return;
    }

    /* Set IPv4 default gateway */
    RemoveIpv4DefaultGateway(portType);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set IPv4 static routing
 * @param   portType
 * @param   action
 * @param   prevIpv4NetworkInfo
 */
static void addRemoveIpv4StaticRoute(NETWORK_PORT_e portType, BOOL action, IPV4_LAN_CONFIG_t *prevIpv4NetworkInfo)
{
    UINT8                   entry;
    STATIC_ROUTING_CONFIG_t staticRouteConfig;

    // Read static route config
    ReadStaticRoutingConfig(&staticRouteConfig);

    /* Is given port is default route port? */
    if ((staticRouteConfig.defaultPort - 1) == portType)
    {
        /* It is default route port. No need to apply static route */
        return;
    }

    /* Add routing entry for network port */
    for (entry = 0; entry < MAX_STATIC_ROUTING_ENTRY; entry++)
    {
        /* Is it required network port? */
        staticRouteConfig.entry[entry].routePort--;
        if(staticRouteConfig.entry[entry].routePort != portType)
        {
            continue;
        }

        /* Check address family. Only process for IPv4 type */
        if (NM_IPADDR_FAMILY_V4 == NMIpUtil_GetIpAddrFamily(staticRouteConfig.entry[entry].networkAddr))
        {
            /* Update static routing table */
            updateIpv4StaticRouteTable(portType, staticRouteConfig.entry[entry].networkAddr, staticRouteConfig.entry[entry].subnetMask,
                                       prevIpv4NetworkInfo->lan.gateway, action);
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Update static routing table of the system
 * @param   portType
 * @param   networkAddr
 * @param   subnetMask
 * @param   gateway
 * @param   action
 * @return  Returns status
 */
static BOOL updateIpv4StaticRouteTable(NETWORK_PORT_e portType, const CHAR *networkAddr, UINT8 subnetMask, const CHAR *gateway, BOOL action)
{
    CHAR            netmask[IPV4_ADDR_LEN_MAX];
    IP_NW_ADDR_u    ipAddrNw;

    /* Convert subnet mask to standard format */
    ipAddrNw.ip4 = htonl(__UINT32_MAX__ << (subnetMask - 1));
    NMIpUtil_ConvertIpv4NetworkAddrToString(&ipAddrNw.ip4, netmask, sizeof(netmask));

    /* Add or remove static routing based on action */
    if (action == SET)
    {
        /* Add IPv4 static routing */
        return (AddIpv4StaticRoute(portType, networkAddr, netmask, gateway));
    }
    else
    {
        /* Remove IPv4 static routing */
        return (RemoveIpv4StaticRoute(portType, networkAddr, netmask, gateway));
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set IPv6 network parameters information
 * @param   portType
 * @param   ipv6NetworkInfo
 */
static void setIpv6NetworkInfo(NETWORK_PORT_e portType, IPV6_LAN_CONFIG_t *ipv6NetworkInfo)
{
    MUTEX_LOCK(gNetworkCfgMutex[portType]);
    gNetworkConfig[portType].ipv6 = *ipv6NetworkInfo;
    MUTEX_UNLOCK(gNetworkCfgMutex[portType]);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function reset the ipv6 parameters of network port
 * @param   portType
 */
static void clearIpv6NetworkInfo(NETWORK_PORT_e portType)
{
    MUTEX_LOCK(gNetworkCfgMutex[portType]);
    memset(&gNetworkConfig[portType].ipv6.lan, 0, sizeof(gNetworkConfig[portType].ipv6.lan));
    gNetworkConfig[portType].ipv6.lan.prefixLen = DFLT_IPV6_PREFIX_LEN;

    /* Set DNS auto for usb modem */
    if (portType == NETWORK_PORT_USB_MODEM)
    {
        gNetworkConfig[portType].ipv6.ipAssignMode = IPV6_ASSIGN_SLAAC;
        gNetworkConfig[portType].ipv6.dns.mode = DNS_AUTO;
    }

    /* Reset DNS address if mode is auto */
    if(gNetworkConfig[portType].ipv6.dns.mode == DNS_AUTO)
    {
        memset(gNetworkConfig[portType].ipv6.dns.primaryAddress, 0, sizeof(gNetworkConfig[portType].ipv6.dns.primaryAddress));
        memset(gNetworkConfig[portType].ipv6.dns.secondaryAddress, 0, sizeof(gNetworkConfig[portType].ipv6.dns.secondaryAddress));
    }
    MUTEX_UNLOCK(gNetworkCfgMutex[portType]);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set IPv6 LAN address assignment mode
 * @param   portType
 * @param   ipv6NetworkInfo
 */
static void setIpv6LanMode(NETWORK_PORT_e portType, IPV6_LAN_CONFIG_t *ipv6NetworkInfo)
{
    /* Set IPv6 address mode as per configuration */
    if (ipv6NetworkInfo->ipAssignMode == IPV6_ASSIGN_DHCP)
    {
        SetIpv6DhcpMode(portType);
    }
    else if (ipv6NetworkInfo->ipAssignMode == IPV6_ASSIGN_SLAAC)
    {
        SetIpv6SlaacMode(portType);
    }
    else /* Static */
    {
        SetIpv6StaticMode(portType, &ipv6NetworkInfo->lan);
        setIpv6DefaultGateway(portType, ipv6NetworkInfo);
        addRemoveIpv6StaticRoute(portType, SET, ipv6NetworkInfo);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Clear IPv6 LAN address assignment mode
 * @param   portType
 * @param   ipv6NetworkInfo
 * @param   prevIpv6NetworkInfo
 */
static void clearIpv6LanMode(NETWORK_PORT_e portType, IPV6_LAN_CONFIG_t *ipv6NetworkInfo, IPV6_LAN_CONFIG_t *prevIpv6NetworkInfo)
{
    /* Clear the all IP address from interface */
    clearIpv6LanAddr(portType, ipv6NetworkInfo, prevIpv6NetworkInfo);

    /* Remove the IPv6 assignment mode */
    RemoveIpv6Mode(portType);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Clear IPv6 LAN address
 * @param   portType
 * @param   ipv6NetworkInfo
 * @param   prevIpv6NetworkInfo
 */
static void clearIpv6LanAddr(NETWORK_PORT_e portType, IPV6_LAN_CONFIG_t *ipv6NetworkInfo, IPV6_LAN_CONFIG_t *prevIpv6NetworkInfo)
{
    /* Clear the all IP address from interface */
    addRemoveIpv6StaticRoute(portType, CLEAR, prevIpv6NetworkInfo);
    clearIpv6DefaultGateway(portType);
    RemoveIpv6Addr(portType);

    /* Clear local ip address if assigned dynamically */
    if (ipv6NetworkInfo->ipAssignMode != IPV6_ASSIGN_STATIC)
    {
        clearIpv6NetworkInfo(portType);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set IPv6 default gateway on interface
 * @param   portType
 * @param   ipv6NetworkInfo
 */
static void setIpv6DefaultGateway(NETWORK_PORT_e portType, IPV6_LAN_CONFIG_t *ipv6NetworkInfo)
{
    STATIC_ROUTING_CONFIG_t staticRouteConfig;

    /* Read static route config */
    ReadStaticRoutingConfig(&staticRouteConfig);

    /* Is given port is default route port? */
    if ((staticRouteConfig.defaultPort - 1) != portType)
    {
        /* It is not a default route port */
        return;
    }

    /* Set IPv6 default gateway */
    AddIpv6DefaultGateway(portType, &ipv6NetworkInfo->lan);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Clear IPv6 default gateway from interface
 * @param   portType
 */
static void clearIpv6DefaultGateway(NETWORK_PORT_e portType)
{
    STATIC_ROUTING_CONFIG_t staticRouteConfig;

    /* Read static route config */
    ReadStaticRoutingConfig(&staticRouteConfig);

    /* Is given port is default route port? */
    if ((staticRouteConfig.defaultPort - 1) != portType)
    {
        /* It is not a default route port */
        return;
    }

    /* Set IPv6 default gateway */
    RemoveIpv6DefaultGateway(portType);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set IPv6 static routing
 * @param   portType
 * @param   action
 * @param   prevIpv6NetworkInfo
 */
static void addRemoveIpv6StaticRoute(NETWORK_PORT_e portType, BOOL action, IPV6_LAN_CONFIG_t *prevIpv6NetworkInfo)
{
    UINT8                   entry;
    STATIC_ROUTING_CONFIG_t staticRouteConfig;

    /* Read static route config */
    ReadStaticRoutingConfig(&staticRouteConfig);

    /* Is given port is default route port? */
    if ((staticRouteConfig.defaultPort - 1) == portType)
    {
        /* It is default route port. No need to apply static route */
        return;
    }

    /* Add routing entry for network port */
    for (entry = 0; entry < MAX_STATIC_ROUTING_ENTRY; entry++)
    {
        /* Is it required network port? */
        staticRouteConfig.entry[entry].routePort--;
        if(staticRouteConfig.entry[entry].routePort != portType)
        {
            continue;
        }

        /* Check address family. Only process for IPv6 type */
        if (NM_IPADDR_FAMILY_V6 == NMIpUtil_GetIpAddrFamily(staticRouteConfig.entry[entry].networkAddr))
        {
            /* Update static routing table */
            updateIpv6StaticRouteTable(portType, staticRouteConfig.entry[entry].networkAddr, staticRouteConfig.entry[entry].subnetMask,
                                       prevIpv6NetworkInfo->lan.gateway, action);
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Update static routing table of the system
 * @param   portType
 * @param   networkAddr
 * @param   prefixLength
 * @param   gateway
 * @param   action
 * @return  Returns status
 */
static BOOL updateIpv6StaticRouteTable(NETWORK_PORT_e portType, const CHAR *networkAddr, UINT8 prefixLength, const CHAR *gateway, BOOL action)
{
    /* Add or remove static routing based on action */
    if (action == SET)
    {
        /* Add IPv6 static routing */
        return (AddIpv6StaticRoute(portType, networkAddr, prefixLength, gateway));
    }
    else
    {
        /* Remove IPv6 static routing */
        return (RemoveIpv6StaticRoute(portType, networkAddr, prefixLength, gateway));
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set DNS address in the system
 * @param   portType
 * @param   networkInfo
 */
static void setDnsServerAddr(NETWORK_PORT_e portType, LAN_CONFIG_t *networkInfo)
{
    STATIC_ROUTING_CONFIG_t staticRouteConfig;

    /* Read static route config */
    ReadStaticRoutingConfig(&staticRouteConfig);

    /* Set DNS config of LAN 1 if port type is LAN 2 */
    if (portType == NETWORK_PORT_LAN2)
    {
        MUTEX_LOCK(gNetworkCfgMutex[NETWORK_PORT_LAN1]);
        networkInfo->ipv4.dns = gNetworkConfig[NETWORK_PORT_LAN1].ipv4.dns;
        MUTEX_UNLOCK(gNetworkCfgMutex[NETWORK_PORT_LAN1]);
    }

    /* Is given port is default route port? */
    if ((staticRouteConfig.defaultPort - 1) != portType)
    {
        if((portType == NETWORK_PORT_USB_MODEM) || ((staticRouteConfig.defaultPort - 1) == NETWORK_PORT_USB_MODEM))
        {
            return;
        }
    }

    /* Set only IPv4 DNS for V4 only mode */
    if (networkInfo->ipAddrMode == IP_ADDR_MODE_IPV4)
    {
        /* Set IPv4 DNS server address only */
        SetDnsServerAddr(&networkInfo->ipv4.dns, NULL);
    }
    else
    {
        /* Set IPv4 and IPv6 DNS server address */
        SetDnsServerAddr(&networkInfo->ipv4.dns, &networkInfo->ipv6.dns);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Physical link up event received
 * @param   portType
 */
void OnPhyLinkUp(NETWORK_PORT_e portType)
{
    LAN_CONFIG_t    networkConfig = {0};
    CHAR            detail[MAX_EVENT_DETAIL_SIZE];

    /* Get network port info */
    GetNetworkParamInfo(portType, &networkConfig);

    /* Set IPv4 Lan mode */
    setIpv4LanMode(portType, &networkConfig.ipv4);

    /* Is IPv6 enabled? */
    if (networkConfig.ipAddrMode == IP_ADDR_MODE_DUAL_STACK)
    {
        /* Set IPv6 Lan mode */
        setIpv6LanMode(portType, &networkConfig.ipv6);
    }

    setDnsServerAddr(portType, &networkConfig);

    /* Update link status to up */
    SetNetworkPortLinkStatus(portType, LAN_LINK_STATUS_UP);

    snprintf(detail, MAX_EVENT_DETAIL_SIZE, "%02d", (portType + 1));
    WriteEvent(LOG_NETWORK_EVENT, LOG_ETHERNET_LINK, detail, NULL, EVENT_UP);
    WPRINT(SYS_LOG, "interface link up: [portType=%d], [ifname=%s]", portType, GetNetworkPortName(portType, NULL));
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Physical link down event received
 * @param   portType
 */
void OnPhyLinkDown(NETWORK_PORT_e portType)
{
    LAN_CONFIG_t    networkConfig = {0};
    CHAR            detail[MAX_EVENT_DETAIL_SIZE];

    /* Update link status to down */
    SetNetworkPortLinkStatus(portType, LAN_LINK_STATUS_DOWN);

    /* Get network port info */
    GetNetworkParamInfo(portType, &networkConfig);

    /* Clear IPv4 Lan mode */
    clearIpv4LanMode(portType, &networkConfig.ipv4, &networkConfig.ipv4);

    /* Is IPv6 enabled? */
    if (networkConfig.ipAddrMode == IP_ADDR_MODE_DUAL_STACK)
    {
        /* Clear IPv6 Lan mode */
        clearIpv6LanMode(portType, &networkConfig.ipv6, &networkConfig.ipv6);
    }

    snprintf(detail, MAX_EVENT_DETAIL_SIZE, "%02d", (portType + 1));
    WriteEvent(LOG_NETWORK_EVENT, LOG_ETHERNET_LINK, detail, NULL, EVENT_DOWN);
    WPRINT(SYS_LOG, "interface link down: [portType=%d], [ifname=%s]", portType, GetNetworkPortName(portType, NULL));
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   IPv4 DHCP Deconfig event received
 * @param   portType
 */
void OnIpv4DhcpDeconfig(NETWORK_PORT_e portType)
{
    DPRINT(ETHERNET, "ipv4 dhcp deconfig: [ifname=%s]", GetNetworkPortName(portType, NULL));
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   IPv4 DHCP lease fail event received
 * @param   portType
 */
void OnIpv4DhcpLeaseFail(NETWORK_PORT_e portType)
{
    LAN_CONFIG_t networkConfig = {0};

    /* Get network port info */
    GetNetworkParamInfo(portType, &networkConfig);

    /* Nothing to do for usb modem */
    if (portType == NETWORK_PORT_USB_MODEM)
    {
        /* Clear the IPv4 LAN address */
        clearIpv4NetworkInfo(portType);

        /* Update modem status */
        if (GetNetworkPortLinkStatus(portType) == MODEM_CONNECTED)
        {
            SetNetworkPortLinkStatus(portType, MODEM_DISCONNECTED);
            setDnsServerAddr(portType, &networkConfig);
        }

        /* USB modem status updated */
        UsbModemStatusChanged(FALSE);
    }
    else
    {
        /* Clear the IPv4 LAN address */
        clearIpv4LanAddr(portType, &networkConfig.ipv4, &networkConfig.ipv4);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   IPv4 DHCP bound event received
 * @param   portType
 * @param   ipv4DhcpAddr
 */
void OnIpv4DhcpBound(NETWORK_PORT_e portType, IPV4_LAN_CONFIG_t *ipv4DhcpAddr)
{
    LAN_CONFIG_t networkConfig = {0};

    /* Get network port info */
    GetNetworkParamInfo(portType, &networkConfig);

    networkConfig.ipv4.lan = ipv4DhcpAddr->lan;
    if (networkConfig.ipv4.dns.mode == DNS_AUTO)
    {
        networkConfig.ipv4.dns = ipv4DhcpAddr->dns;
    }

    /* Update IPv4 network config in copy */
    setIpv4NetworkInfo(portType, &networkConfig.ipv4);

    /* Set IPv4 DHCP assigned address */
    SetIpv4DhcpAddr(portType, &networkConfig.ipv4.lan);
    setIpv4DefaultGateway(portType, &networkConfig.ipv4);
    addRemoveIpv4StaticRoute(portType, SET, &networkConfig.ipv4);
    setDnsServerAddr(portType, &networkConfig);

    if (portType == NETWORK_PORT_USB_MODEM)
    {
        SetNetworkPortLinkStatus(portType, MODEM_CONNECTED);
        WriteEvent(LOG_NETWORK_EVENT, LOG_MODEM_STATUS, NULL, NULL, EVENT_UP);
        UsbModemStatusChanged(TRUE);
    }
    else
    {
        WriteEvent(LOG_NETWORK_EVENT, LOG_IP_ASSIGN, networkConfig.ipv4.lan.ipAddress, NULL, EVENT_DHCP);
    }

    DPRINT(ETHERNET, "dhcp ipv4 info: [ifname=%s], [ip=%s], [subnet=%s], [gateway=%s], [dns1=%s], [dns2=%s]",
           GetNetworkPortName(portType, NULL), networkConfig.ipv4.lan.ipAddress, networkConfig.ipv4.lan.subnetMask,
           networkConfig.ipv4.lan.gateway, networkConfig.ipv4.dns.primaryAddress, networkConfig.ipv4.dns.secondaryAddress);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   IPv4 DHCP renew event received
 * @param   portType
 * @param   ipv4DhcpAddr
 */
void OnIpv4DhcpRenew(NETWORK_PORT_e portType, IPV4_LAN_CONFIG_t *ipv4DhcpAddr)
{
    LAN_CONFIG_t networkConfig = {0};

    /* Get network port info */
    GetNetworkParamInfo(portType, &networkConfig);

    if (memcmp(&networkConfig.ipv4.lan, &ipv4DhcpAddr->lan, sizeof(networkConfig.ipv4.lan)) != 0)
    {
        /* Update ip and gateway information */
        networkConfig.ipv4.lan = ipv4DhcpAddr->lan;

        /* Update IPv4 network config in copy */
        setIpv4NetworkInfo(portType, &networkConfig.ipv4);

        /* Set IPv4 DHCP assigned address */
        SetIpv4DhcpAddr(portType, &networkConfig.ipv4.lan);
        setIpv4DefaultGateway(portType, &networkConfig.ipv4);
        addRemoveIpv4StaticRoute(portType, SET, &networkConfig.ipv4);
        setDnsServerAddr(portType, &networkConfig);
    }

    if (networkConfig.ipv4.dns.mode == DNS_AUTO)
    {
        if (memcmp(&networkConfig.ipv4.dns, &ipv4DhcpAddr->dns, sizeof(networkConfig.ipv4.dns)) != 0)
        {
            /* Update dynamic DNS information */
            networkConfig.ipv4.dns = ipv4DhcpAddr->dns;

            /* Update IPv4 network config in copy */
            setIpv4NetworkInfo(portType, &networkConfig.ipv4);
            setDnsServerAddr(portType, &networkConfig);
        }
    }

    if (portType == NETWORK_PORT_USB_MODEM)
    {
        WriteEvent(LOG_NETWORK_EVENT, LOG_MODEM_STATUS, NULL, NULL, EVENT_UP);
    }
    else
    {
        WriteEvent(LOG_NETWORK_EVENT, LOG_IP_ASSIGN, networkConfig.ipv4.lan.ipAddress, NULL, EVENT_DHCP);
    }

    DPRINT(ETHERNET, "dhcp ipv4 info: [ifname=%s], [ip=%s], [subnet=%s], [gateway=%s], [dns1=%s], [dns2=%s]",
           GetNetworkPortName(portType, NULL), networkConfig.ipv4.lan.ipAddress, networkConfig.ipv4.lan.subnetMask,
           networkConfig.ipv4.lan.gateway, networkConfig.ipv4.dns.primaryAddress, networkConfig.ipv4.dns.secondaryAddress);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   IPv4 PPPoE ip up event received
 * @param   portType
 * @param   ipv4PppoeAddr
 */
void OnIpv4PppoeIpUp(NETWORK_PORT_e portType, IPV4_LAN_CONFIG_t *ipv4PppoeAddr)
{
    LAN_CONFIG_t networkConfig = {0};

    /* Get network port info */
    GetNetworkParamInfo(portType, &networkConfig);

    networkConfig.ipv4.lan = ipv4PppoeAddr->lan;
    if (networkConfig.ipv4.dns.mode == DNS_AUTO)
    {
        networkConfig.ipv4.dns = ipv4PppoeAddr->dns;
    }

    /* Update IPv4 network config in copy */
    setIpv4NetworkInfo(portType, &networkConfig.ipv4);

    /* Update gateway, static routing and DNS server */
    setIpv4DefaultGateway(portType, &networkConfig.ipv4);
    addRemoveIpv4StaticRoute(portType, SET, &networkConfig.ipv4);
    setDnsServerAddr(portType, &networkConfig);

    if (portType == NETWORK_PORT_USB_MODEM)
    {
        SetNetworkPortLinkStatus(portType, MODEM_CONNECTED);
        WriteEvent(LOG_NETWORK_EVENT, LOG_MODEM_STATUS, NULL, NULL, EVENT_UP);
        UsbModemStatusChanged(TRUE);
    }
    else
    {
        WriteEvent(LOG_NETWORK_EVENT, LOG_IP_ASSIGN, networkConfig.ipv4.lan.ipAddress, NULL, EVENT_PPPOE);
    }

    DPRINT(ETHERNET, "ppp ipv4 up info: [ifname=%s], [ip=%s], [subnet=%s], [gateway=%s], [dns1=%s], [dns2=%s]",
           GetNetworkPortName(portType, NULL), networkConfig.ipv4.lan.ipAddress, networkConfig.ipv4.lan.subnetMask,
           networkConfig.ipv4.lan.gateway, networkConfig.ipv4.dns.primaryAddress, networkConfig.ipv4.dns.secondaryAddress);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   IPv4 PPPoE ip down event received
 * @param   portType
 */
void OnIpv4PppoeIpDown(NETWORK_PORT_e portType)
{
    LAN_CONFIG_t networkConfig = {0};

    /* Get network port info */
    GetNetworkParamInfo(portType, &networkConfig);

    if (portType == NETWORK_PORT_USB_MODEM)
    {
        /* Clear the IPv4 LAN address */
        clearIpv4NetworkInfo(portType);

        /* Update modem status */
        if (GetNetworkPortLinkStatus(portType) == MODEM_CONNECTED)
        {
            SetNetworkPortLinkStatus(portType, MODEM_DISCONNECTED);
            setDnsServerAddr(portType, &networkConfig);
        }

        /* USB modem status updated */
        UsbModemStatusChanged(FALSE);
    }
    else
    {
        /* Clear the IPv4 LAN address */
        clearIpv4LanAddr(portType, &networkConfig.ipv4, &networkConfig.ipv4);
    }

    DPRINT(ETHERNET, "ppp ipv4 down: [ifname=%s]", GetNetworkPortName(portType, NULL));
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   IPv6 DAD Success event received
 * @param   portType
 */
void OnIpv6AddrSuccess(NETWORK_PORT_e portType)
{
    LAN_CONFIG_t networkConfig = {0};

    /* Get network port info */
    GetNetworkParamInfo(portType, &networkConfig);

    IPRINT(ETHERNET, "ipv6 dad success: [ifname=%s], [ip=%s/%d]", GetNetworkPortName(portType, NULL),
           networkConfig.ipv6.lan.ipAddress, networkConfig.ipv6.lan.prefixLen);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   IPv6 DAD Fail event received
 * @param   portType
 */
void OnIpv6AddrDadFail(NETWORK_PORT_e portType)
{
    LAN_CONFIG_t networkConfig = {0};

    /* Get network port info */
    GetNetworkParamInfo(portType, &networkConfig);

    EPRINT(ETHERNET, "ipv6 dad fail: [ifname=%s], [ip=%s/%d]", GetNetworkPortName(portType, NULL),
           networkConfig.ipv6.lan.ipAddress, networkConfig.ipv6.lan.prefixLen);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   IPv6 DHCP start event received
 * @param   portType
 */
void OnIpv6DhcpStarted(NETWORK_PORT_e portType)
{
    DPRINT(ETHERNET, "ipv6 dhcp started: [ifname=%s]", GetNetworkPortName(portType, NULL));
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   IPv6 DHCP unbound or rebound event received
 * @param   portType
 */
void OnIpv6DhcpUnbound(NETWORK_PORT_e portType)
{
    LAN_CONFIG_t networkConfig = {0};

    /* Get network port info */
    GetNetworkParamInfo(portType, &networkConfig);

    /* Clear the IPv6 LAN address */
    clearIpv6LanAddr(portType, &networkConfig.ipv6, &networkConfig.ipv6);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   IPv6 DHCP bound or rebound event received
 * @param   portType
 * @param   ipv6AddrInfo
 */
void OnIpv6DhcpBoundOrRebound(NETWORK_PORT_e portType, IPV6_LAN_CONFIG_t *ipv6AddrInfo)
{
    LAN_CONFIG_t networkConfig = {0};

    /* Get network port info */
    GetNetworkParamInfo(portType, &networkConfig);

    networkConfig.ipv6.lan = ipv6AddrInfo->lan;
    if (networkConfig.ipv6.dns.mode == DNS_AUTO)
    {
        networkConfig.ipv6.dns = ipv6AddrInfo->dns;
    }

    /* Update IPv6 network config in copy */
    setIpv6NetworkInfo(portType, &networkConfig.ipv6);

    /* Set IPv6 DHCP assigned address */
    SetIpv6DhcpAddr(portType, &networkConfig.ipv6.lan);
    setIpv6DefaultGateway(portType, &networkConfig.ipv6);
    addRemoveIpv6StaticRoute(portType, SET, &networkConfig.ipv6);
    setDnsServerAddr(portType, &networkConfig);

    WriteEvent(LOG_NETWORK_EVENT, LOG_IP_ASSIGN, networkConfig.ipv6.lan.ipAddress, NULL, EVENT_DHCP);
    DPRINT(ETHERNET, "dhcp ipv6 info: [ifname=%s], [ip=%s/%d], [preferTime=%u], [validTime=%u], [gateway=%s], [metric=%d], [dns1=%s], [dns2=%s]",
           GetNetworkPortName(portType, NULL), networkConfig.ipv6.lan.ipAddress, networkConfig.ipv6.lan.prefixLen,
           networkConfig.ipv6.lan.ipPreferredTime, networkConfig.ipv6.lan.ipValidTime,
           networkConfig.ipv6.lan.gateway, networkConfig.ipv6.lan.gatewayMetric,
           networkConfig.ipv6.dns.primaryAddress, networkConfig.ipv6.dns.secondaryAddress);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   IPv6 DHCP updated (renew) event received
 * @param   portType
 * @param   ipv6DhcpAddr
 */
void OnIpv6DhcpUpdated(NETWORK_PORT_e portType, IPV6_LAN_CONFIG_t *ipv6AddrInfo)
{
    LAN_CONFIG_t networkConfig = {0};
    BOOL         ipv6AddrChng = FALSE;
    BOOL         ipv6GatewayChng = FALSE;
    BOOL         updateIpv6Attr = FALSE;
    BOOL         dnsAddrChng = FALSE;

    /* Get network port info */
    GetNetworkParamInfo(portType, &networkConfig);

    /* If IPv6 address parameters updated by DHCP server */
    if ((strcmp(ipv6AddrInfo->lan.ipAddress, networkConfig.ipv6.lan.ipAddress) != STATUS_OK)
            || (ipv6AddrInfo->lan.prefixLen != networkConfig.ipv6.lan.prefixLen))
    {
        /* Clear Ipv6 address parameters */
        clearIpv6LanAddr(portType, &networkConfig.ipv6, &networkConfig.ipv6);
        ipv6AddrChng = TRUE;
    }
    else if (strcmp(ipv6AddrInfo->lan.gateway, networkConfig.ipv6.lan.gateway) != STATUS_OK)
    {
        /* Clear static routing and default gateway for interface */
        addRemoveIpv6StaticRoute(portType, CLEAR, &networkConfig.ipv6);
        clearIpv6DefaultGateway(portType);
        ipv6GatewayChng = TRUE;
    }

    /* If preffered time updated by DHCP server */
    if (ipv6AddrInfo->lan.ipPreferredTime >= networkConfig.ipv6.lan.ipPreferredTime)
    {
        updateIpv6Attr = TRUE;
    }

    /* If DNS parameters updated by DHCP server */
    if ((networkConfig.ipv6.dns.mode == DNS_AUTO)
            && ((strcmp(ipv6AddrInfo->dns.primaryAddress, networkConfig.ipv6.dns.primaryAddress) != STATUS_OK)
            || (strcmp(ipv6AddrInfo->dns.secondaryAddress, networkConfig.ipv6.dns.secondaryAddress) != STATUS_OK)))
    {
        dnsAddrChng = TRUE;
    }

    /* Update ipv6 address parameters */
    networkConfig.ipv6.lan = ipv6AddrInfo->lan;

    /* Update ipv6 dns parameters */
    networkConfig.ipv6.dns = ipv6AddrInfo->dns;

    /* Update IPv6 network config in copy */
    setIpv6NetworkInfo(portType, &networkConfig.ipv6);

    if (TRUE == ipv6AddrChng)
    {
        /* Set IPv6 DHCP assigned address */
        SetIpv6DhcpAddr(portType, &networkConfig.ipv6.lan);
        setIpv6DefaultGateway(portType, &networkConfig.ipv6);
        addRemoveIpv6StaticRoute(portType, SET, &networkConfig.ipv6);
    }
    else if (TRUE == updateIpv6Attr)
    {
        /* Set preferred and valid lifetime */
        UpdateIpv6AddrAttr(portType, &ipv6AddrInfo->lan);
    }

    if (TRUE == ipv6GatewayChng)
    {
        /* Set default gateway and static routing for interface */
        setIpv6DefaultGateway(portType, &networkConfig.ipv6);
        addRemoveIpv6StaticRoute(portType, SET, &networkConfig.ipv6);
    }

    if (TRUE == dnsAddrChng)
    {
        /* Set DNS server address */
        setDnsServerAddr(portType, &networkConfig);
    }

    DPRINT(ETHERNET, "dhcp ipv6 updated: [ifname=%s], [ip=%s/%d], [preferTime=%u], [validTime=%u], [gateway=%s], [metric=%d], [dns1=%s], [dns2=%s]",
           GetNetworkPortName(portType, NULL), networkConfig.ipv6.lan.ipAddress, networkConfig.ipv6.lan.prefixLen,
           networkConfig.ipv6.lan.ipPreferredTime, networkConfig.ipv6.lan.ipValidTime,
           networkConfig.ipv6.lan.gateway, networkConfig.ipv6.lan.gatewayMetric,
           networkConfig.ipv6.dns.primaryAddress, networkConfig.ipv6.dns.secondaryAddress);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   IPv6 SLAAC start event received
 * @param   portType
 */
void OnIpv6SlaacStarted(NETWORK_PORT_e portType)
{
    DPRINT(ETHERNET, "slaac ipv6 started: [ifname=%s]", GetNetworkPortName(portType, NULL));
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   IPv6 SLAAC Router Advertisement event received
 * @param   portType
 * @param   ipv6AddrInfo
 */
void OnIpv6SlaacRaReceived(NETWORK_PORT_e portType, IPV6_LAN_CONFIG_t *ipv6AddrInfo)
{
    LAN_CONFIG_t networkConfig = {0};

    /* Get network port info */
    GetNetworkParamInfo(portType, &networkConfig);

    networkConfig.ipv6.lan = ipv6AddrInfo->lan;
    if (networkConfig.ipv6.dns.mode == DNS_AUTO)
    {
        networkConfig.ipv6.dns = ipv6AddrInfo->dns;
    }

    /* Update IPv6 network config in copy */
    setIpv6NetworkInfo(portType, &networkConfig.ipv6);

    /* Set IPv6 SLAAC assigned address */
    SetIpv6SlaacAddr(portType, &networkConfig.ipv6.lan);
    setIpv6DefaultGateway(portType, &networkConfig.ipv6);
    addRemoveIpv6StaticRoute(portType, SET, &networkConfig.ipv6);
    setDnsServerAddr(portType, &networkConfig);

    WriteEvent(LOG_NETWORK_EVENT, LOG_IP_ASSIGN, networkConfig.ipv6.lan.ipAddress, NULL, EVENT_SLAAC);
    DPRINT(ETHERNET, "slaac ipv6 info: [ifname=%s], [ip=%s/%d], [preferTime=%u], [validTime=%u], [gateway=%s], [metric=%d], [dns1=%s], [dns2=%s]",
           GetNetworkPortName(portType, NULL), networkConfig.ipv6.lan.ipAddress, networkConfig.ipv6.lan.prefixLen,
           networkConfig.ipv6.lan.ipPreferredTime, networkConfig.ipv6.lan.ipValidTime,
           networkConfig.ipv6.lan.gateway, networkConfig.ipv6.lan.gatewayMetric,
           networkConfig.ipv6.dns.primaryAddress, networkConfig.ipv6.dns.secondaryAddress);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   IPv6 SLAAC Router Advertisement update event received
 * @param   portType
 * @param   ipv6AddrInfo
 */
void OnIpv6SlaacRaUpdated(NETWORK_PORT_e portType, IPV6_LAN_CONFIG_t *ipv6AddrInfo)
{
    LAN_CONFIG_t networkConfig = {0};
    BOOL         ipv6AddrChng = FALSE;
    BOOL         ipv6GatewayChng = FALSE;
    BOOL         updateIpv6Attr = FALSE;
    BOOL         dnsAddrChng = FALSE;

    /* Get network port info */
    GetNetworkParamInfo(portType, &networkConfig);

    /* If IPv6 address parameters updated by router */
    if ((strcmp(ipv6AddrInfo->lan.ipAddress, networkConfig.ipv6.lan.ipAddress) != STATUS_OK)
            || (ipv6AddrInfo->lan.prefixLen != networkConfig.ipv6.lan.prefixLen))
    {
        /* Clear Ipv6 address parameters */
        clearIpv6LanAddr(portType, &networkConfig.ipv6, &networkConfig.ipv6);
        ipv6AddrChng = TRUE;
    }
    else if (strcmp(ipv6AddrInfo->lan.gateway, networkConfig.ipv6.lan.gateway) != STATUS_OK)
    {
        /* Clear static routing and default gateway for interface */
        addRemoveIpv6StaticRoute(portType, CLEAR, &networkConfig.ipv6);
        clearIpv6DefaultGateway(portType);
        ipv6GatewayChng = TRUE;
    }

    /* If preffered time updated by router */
    if (ipv6AddrInfo->lan.ipPreferredTime >= networkConfig.ipv6.lan.ipPreferredTime)
    {
        updateIpv6Attr = TRUE;
    }

    /* If DNS parameters updated by router */
    if ((networkConfig.ipv6.dns.mode == DNS_AUTO)
            && ((strcmp(ipv6AddrInfo->dns.primaryAddress, networkConfig.ipv6.dns.primaryAddress) != STATUS_OK)
            || (strcmp(ipv6AddrInfo->dns.secondaryAddress, networkConfig.ipv6.dns.secondaryAddress) != STATUS_OK)))
    {
        dnsAddrChng = TRUE;
    }

    /* Update ipv6 address parameters */
    networkConfig.ipv6.lan = ipv6AddrInfo->lan;

    /* Update ipv6 dns parameters */
    networkConfig.ipv6.dns = ipv6AddrInfo->dns;

    /* Update IPv6 network config in copy */
    setIpv6NetworkInfo(portType, &networkConfig.ipv6);

    if (TRUE == ipv6AddrChng)
    {
        /* Set IPv6 SLAAC address */
        SetIpv6SlaacAddr(portType, &networkConfig.ipv6.lan);
        setIpv6DefaultGateway(portType, &networkConfig.ipv6);
        addRemoveIpv6StaticRoute(portType, SET, &networkConfig.ipv6);
    }
    else if (TRUE == updateIpv6Attr)
    {
        /* Set preferred and valid lifetime */
        UpdateIpv6AddrAttr(portType, &ipv6AddrInfo->lan);
    }

    if (TRUE == ipv6GatewayChng)
    {
        /* Set default gateway and static routing for interface */
        setIpv6DefaultGateway(portType, &networkConfig.ipv6);
        addRemoveIpv6StaticRoute(portType, SET, &networkConfig.ipv6);
    }

    if (TRUE == dnsAddrChng)
    {
        /* Set DNS server address */
        setDnsServerAddr(portType, &networkConfig);
    }

    DPRINT(ETHERNET, "slaac ipv6 ra-update: [ifname=%s], [ip=%s/%d], [preferTime=%u], [validTime=%u], [gateway=%s], [metric=%d], [dns1=%s], [dns2=%s]",
           GetNetworkPortName(portType, NULL), networkConfig.ipv6.lan.ipAddress, networkConfig.ipv6.lan.prefixLen,
           networkConfig.ipv6.lan.ipPreferredTime, networkConfig.ipv6.lan.ipValidTime,
           networkConfig.ipv6.lan.gateway, networkConfig.ipv6.lan.gatewayMetric,
           networkConfig.ipv6.dns.primaryAddress, networkConfig.ipv6.dns.secondaryAddress);
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
