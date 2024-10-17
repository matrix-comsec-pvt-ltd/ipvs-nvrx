// #################################################################################################
//  FILE BRIEF
// #################################################################################################
/**
@file		DhcpServer.h
@brief      File containing the function defination of DHCP server functionality in NVR
*/
// #################################################################################################
//  @INCLUDES
// #################################################################################################
/* Application Includes */
#include "DhcpServer.h"
#include "DebugLog.h"
#include "NetworkController.h"
#include "SysTimer.h"
#include "Utils.h"

// #################################################################################################
//  @DEFINES
// #################################################################################################
/* DHCP server configuration file path */
#define DHCP_SERVER_CONFIG_FILE_PATH         "/etc/dnsmasq-dhcp-server.conf"

/* DHCP server lease file path */
#define DHCP_SERVER_LEASE_FILE_PATH          "/etc/dhcp-server.lease"

/* DHCP server notify script path */
#define DHCP_SERVER_NOTIFY_SCRIPT_PATH       SCRIPTS_DIR_PATH "/dhcpServerNotify.sh"

/* DHCP server start command: "dnsmasq -C /etc/dnsmasq-dhcp-server.conf" */
#define DHCP_SERVER_START_CMD                "dnsmasq -C " DHCP_SERVER_CONFIG_FILE_PATH

/* DHCP server stop command: "pkill -15 -f 'dnsmasq -C /etc/dnsmasq-dhcp-server.conf'" */
#define DHCP_SERVER_STOP_CMD                 "pkill -TERM -f '" DHCP_SERVER_START_CMD "'"

/* Delay time to start DHCP server process on bootup. Provide some time to system */
#define DHCP_SERVER_START_ON_BOOT_TMR        15 /* Seconds */

/* Timer to start DHCP server on config change */
#define DHCP_SERVER_START_ON_CNFG_CHANGE_TMR 2 /* Seconds */

// #################################################################################################
//  @DATA TYPES
// #################################################################################################
typedef enum
{
    LEASE_ENTRY_NOT_FOUND = 0,
    LEASE_ENTRY_FOUND,
    LEASE_ENTRY_CONFLICT,
    LEASE_ENTRY_STATUS_MAX
} LEASE_ENTRY_STATUS_e;

typedef union
{
    UINT8  octet[4];
    UINT32 littelAddr;
} IP_ADDR_OCTET_u;

typedef struct
{
    TIMER_HANDLE    timerHandle;
    pthread_mutex_t dataLock;
    LAN_CONFIG_ID_e lanPort;
    CHAR            startIpAddrRange[IPV4_ADDR_LEN_MAX];
    CHAR            endIpAddrRange[IPV4_ADDR_LEN_MAX];
    CHAR            subnet[IPV4_ADDR_LEN_MAX];
    CHAR            gateway[IPV4_ADDR_LEN_MAX];
    CHAR            dns[IPV4_ADDR_LEN_MAX];
    UINT16          noOfHost;
    UINT16          leaseHour;
} DHCP_SERVER_INFO_t;

// #################################################################################################
//  @STATIC VARIABLES
// #################################################################################################
static DHCP_SERVER_INFO_t         dhcpServerInfo;
static pthread_mutex_t            leaseStatusLock = PTHREAD_MUTEX_INITIALIZER;
static DHCP_SERVER_LEASE_STATUS_t dhcpServerLeaseStatus[DHCP_SERVER_LEASE_CLIENT_MAX];

// #################################################################################################
//  @PROTOTYPES
// #################################################################################################
//-------------------------------------------------------------------------------------------------
static void startDhcpServerProcessTimer(UINT8 secCnt);
//-------------------------------------------------------------------------------------------------
static void stopDhcpServerProcessTimer(void);
//-------------------------------------------------------------------------------------------------
static void startDhcpServerProcess(UINT32 data);
//-------------------------------------------------------------------------------------------------
static void stopDhcpServerProcess(BOOL isRetainLeaseData);
//-------------------------------------------------------------------------------------------------
static void restartDhcpServerProcess(BOOL isRetainLeaseData);
//-------------------------------------------------------------------------------------------------
static BOOL prepareDhcpServerConfigFile(void);
//-------------------------------------------------------------------------------------------------
static void parseIpAddressInOctet(CHARPTR ipAddrstr, IP_ADDR_OCTET_u *pIpAddrOctet);
//-------------------------------------------------------------------------------------------------
static void resetAllClientLeaseInfo(void);
//-------------------------------------------------------------------------------------------------
static void resetClientLeaseInfo(UINT16 leaseNo);
//-------------------------------------------------------------------------------------------------
static LEASE_ENTRY_STATUS_e getClientLeaseInfoStatus(DHCP_SERVER_NOTIFY_t *pDhcpServerNotify, UINT16 *pLeaseNo);
//-------------------------------------------------------------------------------------------------
static BOOL updateClientLeaseInfo(DHCP_SERVER_NOTIFY_t *pDhcpServerNotify, UINT16 leaseNo);
//-------------------------------------------------------------------------------------------------
// #################################################################################################
// @FUNCTIONS
// #################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   Initialization of DHCP server
 */
void InitDhcpServer(void)
{
    DHCP_SERVER_CONFIG_t dhcpServerConfig;

    /* Reset dhcp server info */
    memset(&dhcpServerInfo, 0, sizeof(dhcpServerInfo));
    MUTEX_INIT(dhcpServerInfo.dataLock, NULL);
    dhcpServerInfo.timerHandle = INVALID_TIMER_HANDLE;

    /* Reset lease information of all the clients */
    resetAllClientLeaseInfo();

    /* Get DHCP server configuration */
    ReadDhcpServerConfig(&dhcpServerConfig);

    /* Check DHCP server is enabled or not */
    if (DISABLE == dhcpServerConfig.enable)
    {
        /* DHCP server is disabled */
        return;
    }

    /* DHCP server is enabled. Hence start process */
    startDhcpServerProcessTimer(DHCP_SERVER_START_ON_BOOT_TMR);

    /* DHCP server process started */
    DPRINT(DHCP_SERVER, "dhcp server process started on bootup..!!");
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Exit DHCP server
 */
void DeInitDhcpServer(void)
{
    /* Stop DHCP server and retain lease file */
    stopDhcpServerProcess(TRUE);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   DHCP server config changed
 * @param   newCopy
 * @param   oldCopy
 */
void DhcpServerConfigNotify(DHCP_SERVER_CONFIG_t newCopy, DHCP_SERVER_CONFIG_t *oldCopy)
{
    BOOL isRetainLeaseData;

    /* Is enable/disable DHCP server action performed? */
    if (newCopy.enable != oldCopy->enable)
    {
        /* Stop DHCP server and remove lease file */
        stopDhcpServerProcess(FALSE);

        /* Start DHCP server if enabled */
        if (newCopy.enable == ENABLE)
        {
            /* Start DHCP server */
            startDhcpServerProcessTimer(DHCP_SERVER_START_ON_CNFG_CHANGE_TMR);
        }

        /* Required action done */
        return;
    }

    /* Is DHCP server disabled */
    if (newCopy.enable == DISABLE)
    {
        return;
    }

    /* Is DHCP server interface changed? */
    if (newCopy.lanPort != oldCopy->lanPort)
    {
        /* Remove lease file on interface change */
        isRetainLeaseData = FALSE;
    }
    /* Is start IP address changed? */
    else if (strcmp(newCopy.startIpAddr, oldCopy->startIpAddr))
    {
        /* Remove lease file on start ip address change */
        isRetainLeaseData = FALSE;
    }
    /* Are number of hosts changed? */
    else if (newCopy.noOfHost != oldCopy->noOfHost)
    {
        /* Remove lease file on number of hosts change */
        isRetainLeaseData = FALSE;
    }
    else
    {
        /* Retain lease data for all other cases (e.g. change in lease hours, dns address etc.) */
        isRetainLeaseData = TRUE;
    }

    /* Restart DHCP server */
    restartDhcpServerProcess(isRetainLeaseData);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   LAN config update notify for DHCP server
 * @param   lanNo
 * @param   newLanConfig
 * @param   oldLanConfig
 */
void DhcpServerLanConfigUpdate(LAN_CONFIG_ID_e lanNo, LAN_CONFIG_t newLanConfig, LAN_CONFIG_t *oldLanConfig)
{
    DHCP_SERVER_CONFIG_t dhcpServerConfig;

    /* Get DHCP server configuration */
    ReadDhcpServerConfig(&dhcpServerConfig);

    /* Nothing to do if DHCP server is disabled */
    if (DISABLE == dhcpServerConfig.enable)
    {
        return;
    }

    /* Check LAN config changed port and DHCP server port */
    if (lanNo != dhcpServerConfig.lanPort)
    {
        return;
    }

    /* Check Ip assinment mode changed or not */
    if ((lanNo == LAN1_PORT) && (newLanConfig.ipv4.ipAssignMode != IPV4_ASSIGN_STATIC))
    {
        /* IP assignment mode changed from static to dynamic. Disable DHCP server */
        DPRINT(DHCP_SERVER, "dhcp server cnfg default due to ip mode changed to dynamic: [lan=%d], [mode=%d]", lanNo, newLanConfig.ipv4.ipAssignMode);
        DfltDhcpServerConfig();
        return;
    }

    /* Is LAN ip and DHCP server start IP in same subnet? */
    if (FALSE == IsIpAddrInSameSubnet(newLanConfig.ipv4.lan.ipAddress, dhcpServerConfig.startIpAddr, newLanConfig.ipv4.lan.subnetMask))
    {
        /* Both are in different subnet */
        DPRINT(DHCP_SERVER, "dhcp server cnfg default due to LAN ip/subnet changed: [lan=%d], [lanIp=%s], [subnet=%s], [dhcpStartIp=%s]", lanNo,
               newLanConfig.ipv4.lan.ipAddress, newLanConfig.ipv4.lan.subnetMask, dhcpServerConfig.startIpAddr);
        DfltDhcpServerConfig();
        return;
    }

    /* Is Subnet mask changed? */
    if (strcmp(newLanConfig.ipv4.lan.subnetMask, oldLanConfig->ipv4.lan.subnetMask))
    {
        IP_ADDR_OCTET_u ipOctet;
        CHAR            endIpAddrRange[IPV4_ADDR_LEN_MAX];

        /* Get start IP address octet */
        parseIpAddressInOctet(dhcpServerConfig.startIpAddr, &ipOctet);

        /* Get the end eddress octet by adding host in start address */
        ipOctet.littelAddr += (dhcpServerConfig.noOfHost - 1);
        snprintf(endIpAddrRange, sizeof(endIpAddrRange), "%hhu.%hhu.%hhu.%hhu", ipOctet.octet[3], ipOctet.octet[2], ipOctet.octet[1],
                 ipOctet.octet[0]);

        /* Is DHCP server start ip and end IP in same subnet? */
        if (FALSE == IsIpAddrInSameSubnet(endIpAddrRange, dhcpServerConfig.startIpAddr, newLanConfig.ipv4.lan.subnetMask))
        {
            /* Both are in different subnet */
            DPRINT(DHCP_SERVER,
                   "dhcp server cnfg default due to LAN subnet changed: [lan=%d], [lanIp=%s], [subnet=%s], [dhcpStartIp=%s], [dhcpEndIp=%s]", lanNo,
                   newLanConfig.ipv4.lan.ipAddress, newLanConfig.ipv4.lan.subnetMask, dhcpServerConfig.startIpAddr, endIpAddrRange);
            DfltDhcpServerConfig();
            return;
        }

        /* Restart DHCP server by removing lease data */
        restartDhcpServerProcess(FALSE);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Validate new DHCP server config before save into file
 * @param   newDhcpServerConfig
 * @return
 */
NET_CMD_STATUS_e ValidateDhcpServerCfg(DHCP_SERVER_CONFIG_t newDhcpServerConfig)
{
    IP_ADDR_OCTET_u ipOctet;
    IP_ADDR_OCTET_u subnetOctet;
    LAN_CONFIG_t    lanConfig;

    /* Skip all validations DHCP server is disabled */
    if (newDhcpServerConfig.enable == DISABLE)
    {
        /* DHCP server is disabled */
        return CMD_SUCCESS;
    }

    /* Check lan port for dhcp server */
    if (newDhcpServerConfig.lanPort == LAN2_PORT)
    {
        /* Check number of lan port in system */
        if (GetNoOfLanPort() == SINGLE_ETHERNET_PORT)
        {
            /* System have only one lan port */
            EPRINT(DHCP_SERVER, "invld lan port for dhcp server: [lan=%d]", newDhcpServerConfig.lanPort);
            return CMD_PROCESS_ERROR;
        }

        /* Read LAN2 config for DHCP server */
        ReadLan2Config(&lanConfig);
    }
    else
    {
        /* Read LAN1 config for DHCP server */
        ReadLan1Config(&lanConfig);

        /* Check LAN1 IP mode */
        if (lanConfig.ipv4.ipAssignMode != IPV4_ASSIGN_STATIC)
        {
            /* LAN1 config is not static */
            return CMD_PROCESS_ERROR;
        }
    }

    /* Check LAN and DHCP server start IP address subnet. It must be same */
    if (FALSE == IsIpAddrInSameSubnet(lanConfig.ipv4.lan.ipAddress, newDhcpServerConfig.startIpAddr, lanConfig.ipv4.lan.subnetMask))
    {
        /* Both are in different subnet */
        return CMD_IP_SUBNET_MISMATCH;
    }

    /* Validate gateway address only if it is not null */
    if (newDhcpServerConfig.gatewayAddr[0] != '\0')
    {
        /* Are start IP address and Gateway in same subnet? */
        if (FALSE == IsIpAddrInSameSubnet(newDhcpServerConfig.startIpAddr, newDhcpServerConfig.gatewayAddr, lanConfig.ipv4.lan.subnetMask))
        {
            /* Both are in different subnet */
            return CMD_IP_GATEWAY_SAME_SUBNET;
        }
    }

    /* Get subnet octet for number of host validation */
    parseIpAddressInOctet(lanConfig.ipv4.lan.subnetMask, &subnetOctet);

    /* Invert mask to get number of hosts in network */
    subnetOctet.littelAddr = ~subnetOctet.littelAddr;

    /* Validate number of hosts */
    if (subnetOctet.littelAddr <= 2)
    {
        /* Invalid network prefix length */
        EPRINT(DHCP_SERVER, "network is too small for dhcp server: [networkHost=%d]", subnetOctet.littelAddr);
        return CMD_PROCESS_ERROR;
    }

    /* Get start ip address in octet */
    parseIpAddressInOctet(newDhcpServerConfig.startIpAddr, &ipOctet);

    /* Get start ip address number */
    ipOctet.littelAddr &= subnetOctet.littelAddr;

    /* DHCP server open source will skip Network address and broadcast address bt itself
     * and number of hosts having staring address itself (hence -1) */
    if ((newDhcpServerConfig.noOfHost + ipOctet.littelAddr - 1) > subnetOctet.littelAddr)
    {
        EPRINT(DHCP_SERVER, "network is too small for hosts: [networkHost=%d]", subnetOctet.littelAddr);
        return CMD_INVALID_HOST_SIZE;
    }

    /* DHCP server config is valid */
    return CMD_SUCCESS;
}

//------------------------------------------------------------------------------------------------
/**
 * @brief   DHCP server notify before config restore
 */
void DhcpServerConfigRestoreNotify(void)
{
    /* Stop DHCP server and remove lease file */
    DPRINT(DHCP_SERVER, "stop dhcp server due to config restore");
    stopDhcpServerProcess(FALSE);
}

//------------------------------------------------------------------------------------------------
/**
 * @brief   Notification of change in DHCP server lease file
 * @param   pDhcpServerNotify
 */
void DhcpServerLeaseUpdateNotify(DHCP_SERVER_NOTIFY_t *pDhcpServerNotify)
{
    LEASE_ENTRY_STATUS_e leaseEntryStatus;
    UINT16               leaseEntryNo;
    CHAR                 detail[MAX_EVENT_DETAIL_SIZE];
    CHAR                 advncDetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];

    /* Print notified info */
    DPRINT(DHCP_SERVER, "[action=%s], [ip=%s], [mac=%s], [expire=%ld], [remaining=%d], [hostname=%s]", pDhcpServerNotify->action,
           pDhcpServerNotify->assignIpAddr, pDhcpServerNotify->clientMacAddr, pDhcpServerNotify->leaseExpireTime, pDhcpServerNotify->remainingTime,
           pDhcpServerNotify->clientHostname);

    /* Lock DHCP server status info */
    MUTEX_LOCK(leaseStatusLock);

    /* Get the status of client lease info status */
    leaseEntryStatus = getClientLeaseInfoStatus(pDhcpServerNotify, &leaseEntryNo);
    if (leaseEntryStatus == LEASE_ENTRY_CONFLICT)
    {
        /* Conflict found in lease info */
        WPRINT(DHCP_SERVER, "lease conflict: [ip=%s], [localMac=%s], [notifyMac=%s]", dhcpServerLeaseStatus[leaseEntryNo].assignIpAddr,
               dhcpServerLeaseStatus[leaseEntryNo].clientMacAddr, pDhcpServerNotify->assignIpAddr);
    }

    /* Check notified entry status (add, del or old) */
    if (strcmp(pDhcpServerNotify->action, "add") == 0)
    {
        /* Add entry in lease status info */
        if (FAIL == updateClientLeaseInfo(pDhcpServerNotify, leaseEntryNo))
        {
            /* No free entry found in status info */
            WPRINT(DHCP_SERVER, "fail to add entry: [ip=%s], [mac=%s]", pDhcpServerNotify->assignIpAddr, pDhcpServerNotify->clientMacAddr);
            MUTEX_UNLOCK(leaseStatusLock);
            return;
        }

        /* Add event log only if entry not available in status info */
        if (leaseEntryStatus == LEASE_ENTRY_FOUND)
        {
            /* Entry already available in status info */
            WPRINT(DHCP_SERVER, "entry already available in lease add: [ip=%s], [mac=%s]", pDhcpServerNotify->assignIpAddr,
                   pDhcpServerNotify->clientMacAddr);
        }
        else
        {
            /* Log ip assign entry in event */
            snprintf(detail, MAX_EVENT_DETAIL_SIZE, "%d", (dhcpServerInfo.lanPort + 1));
            snprintf(advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s | %s", pDhcpServerNotify->assignIpAddr, pDhcpServerNotify->clientMacAddr);
            WriteEvent(LOG_NETWORK_EVENT, LOG_DHCP_SERVER_IP_ASSIGN, detail, advncDetail, EVENT_ALERT);
        }
    }
    else if (strcmp(pDhcpServerNotify->action, "del") == 0)
    {
        /* Remove entry from lease status info */
        resetClientLeaseInfo(leaseEntryNo);
        if (leaseEntryStatus != LEASE_ENTRY_FOUND)
        {
            /* Entry not available in status info */
            WPRINT(DHCP_SERVER, "entry not found in lease remove: [ip=%s], [mac=%s]", pDhcpServerNotify->assignIpAddr,
                   pDhcpServerNotify->clientMacAddr);
        }

        /* Log ip remove entry in event */
        snprintf(detail, MAX_EVENT_DETAIL_SIZE, "%d", (dhcpServerInfo.lanPort + 1));
        snprintf(advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s | %s", pDhcpServerNotify->assignIpAddr, pDhcpServerNotify->clientMacAddr);
        WriteEvent(LOG_NETWORK_EVENT, LOG_DHCP_SERVER_IP_EXPIRE, detail, advncDetail, EVENT_ALERT);
    }
    else
    {
        /* Update entry in lease status info */
        if (FAIL == updateClientLeaseInfo(pDhcpServerNotify, leaseEntryNo))
        {
            /* Failed to update lease entry */
            WPRINT(DHCP_SERVER, "fail to update entry: [ip=%s], [mac=%s]", pDhcpServerNotify->assignIpAddr, pDhcpServerNotify->clientMacAddr);
            MUTEX_UNLOCK(leaseStatusLock);
            return;
        }
    }

    /* Unlock DHCP server status info */
    MUTEX_UNLOCK(leaseStatusLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get the DHCP server lease status
 * @param   pLeaseStatus
 * @return	Number of lease entry
 */
UINT32 GetDhcpServerLeaseStatus(DHCP_SERVER_LEASE_STATUS_t *pLeaseStatus)
{
    UINT16 leaseCnt;
    UINT16 totalLeaseEntry = 0;
    time_t epochTime;
    time_t expireTime;

    /* Get time since epoch */
    if (time(&epochTime) == -1)
    {
        /* Fail to get time */
        EPRINT(DHCP_SERVER, "fail to get time: [err=%s]", STR_ERR);
        return 0;
    }

    /* Lock DHCP server status info */
    MUTEX_LOCK(leaseStatusLock);
    for (leaseCnt = 0; leaseCnt < DHCP_SERVER_LEASE_CLIENT_MAX; leaseCnt++)
    {
        /* Is invalid ip address? */
        if (dhcpServerLeaseStatus[leaseCnt].assignIpAddr[0] == '\0')
        {
            /* Lease is not assigned */
            continue;
        }

        /* Copy lease status with remaining time in seconds */
        memcpy(&pLeaseStatus[totalLeaseEntry], &dhcpServerLeaseStatus[leaseCnt], sizeof(DHCP_SERVER_LEASE_STATUS_t));
        if (dhcpServerLeaseStatus[leaseCnt].leaseExpireTime > epochTime)
        {
            /* Get remaining lease time */
            expireTime = dhcpServerLeaseStatus[leaseCnt].leaseExpireTime - epochTime;

            /* It is not in minute resolution then add one minute to avoid last expire time zero for last minute */
            if (expireTime % SEC_IN_ONE_MIN)
            {
                /* Make expire time in multiples of minute */
                expireTime = ((expireTime / SEC_IN_ONE_MIN) * SEC_IN_ONE_MIN) + SEC_IN_ONE_MIN;
            }
        }
        else
        {
            /* System time may jump forward */
            expireTime = 0;
        }

        /* Update lease expire time */
        pLeaseStatus[totalLeaseEntry].leaseExpireTime = expireTime;

        /* Valid lease entry added */
        totalLeaseEntry++;
    }

    /* Unlock DHCP server status info */
    MUTEX_UNLOCK(leaseStatusLock);
    return totalLeaseEntry;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Start DHCP server process timer and on timeout start DHCP server
 * @param   secCnt - Delay in seconds
 */
static void startDhcpServerProcessTimer(UINT8 secCnt)
{
    TIMER_INFO_t timerInfo = {.count = CONVERT_SEC_TO_TIMER_COUNT(secCnt), .funcPtr = startDhcpServerProcess, .data = 0};

    /* Lock DHCP Server process timer */
    MUTEX_LOCK(dhcpServerInfo.dataLock);

    /* Delete timer if already runnning */
    DeleteTimer(&dhcpServerInfo.timerHandle);

    /* Start a new timer */
    StartTimer(timerInfo, &dhcpServerInfo.timerHandle);

    /* Unlock DHCP Server process timer */
    MUTEX_UNLOCK(dhcpServerInfo.dataLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Stop DHCP server process timer
 */
static void stopDhcpServerProcessTimer(void)
{
    /* Lock DHCP Server process timer */
    MUTEX_LOCK(dhcpServerInfo.dataLock);

    /* Delete the dhcp server process timer */
    DeleteTimer(&dhcpServerInfo.timerHandle);

    /* Unlock DHCP Server process timer */
    MUTEX_UNLOCK(dhcpServerInfo.dataLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Start DHCP server related process
 * @param   data - User Data
 */
static void startDhcpServerProcess(UINT32 data)
{
    IP_ADDR_OCTET_u      ipOctet;
    LAN_CONFIG_t         lanConfig;
    DHCP_SERVER_CONFIG_t dhcpServerConfig;

    /* Get DHCP server configuration */
    ReadDhcpServerConfig(&dhcpServerConfig);

    /* Check lan port for dhcp server */
    if (dhcpServerConfig.lanPort == LAN2_PORT)
    {
        /* Read LAN2 config for DHCP server */
        ReadLan2Config(&lanConfig);
    }
    else
    {
        /* Read LAN1 config for DHCP server */
        ReadLan1Config(&lanConfig);
    }

    /* Stop DHCP server process */
    stopDhcpServerProcess(TRUE);

    /* Get start IP address octet */
    parseIpAddressInOctet(dhcpServerConfig.startIpAddr, &ipOctet);

    /* Get the end eddress octet by adding host in start address */
    ipOctet.littelAddr += (dhcpServerConfig.noOfHost - 1);

    /* Lock DHCP Server process data */
    MUTEX_LOCK(dhcpServerInfo.dataLock);

    /* Store config in local copy and start DHCP server */
    dhcpServerInfo.lanPort = dhcpServerConfig.lanPort;
    snprintf(dhcpServerInfo.startIpAddrRange, sizeof(dhcpServerInfo.startIpAddrRange), "%s", dhcpServerConfig.startIpAddr);
    snprintf(dhcpServerInfo.endIpAddrRange, sizeof(dhcpServerInfo.endIpAddrRange), "%hhu.%hhu.%hhu.%hhu", ipOctet.octet[3], ipOctet.octet[2],
             ipOctet.octet[1], ipOctet.octet[0]);
    snprintf(dhcpServerInfo.subnet, sizeof(dhcpServerInfo.subnet), "%s", lanConfig.ipv4.lan.subnetMask);
    snprintf(dhcpServerInfo.gateway, sizeof(dhcpServerInfo.gateway), "%s", dhcpServerConfig.gatewayAddr);
    snprintf(dhcpServerInfo.dns, sizeof(dhcpServerInfo.dns), "%s", dhcpServerConfig.dnsServerAddr);
    dhcpServerInfo.noOfHost = dhcpServerConfig.noOfHost;
    dhcpServerInfo.leaseHour = dhcpServerConfig.leaseHour;

    /* Prepare DHCP server config file */
    if (FAIL == prepareDhcpServerConfigFile())
    {
        /* Unlock DHCP Server process data */
        MUTEX_UNLOCK(dhcpServerInfo.dataLock);
        EPRINT(DHCP_SERVER, "fail to create dhcp server config file");
        return;
    }

    /* dnsmasq doesn't start the dhcp server if interface is down */
    if (FALSE == IsNetworkPortLinkUp(dhcpServerInfo.lanPort ? NETWORK_PORT_LAN2 : NETWORK_PORT_LAN1))
    {
        /* Unlock DHCP Server process data */
        MUTEX_UNLOCK(dhcpServerInfo.dataLock);
        WPRINT(DHCP_SERVER, "dhcp server not started due to interface is down");
        startDhcpServerProcessTimer(DHCP_SERVER_START_ON_BOOT_TMR);
        return;
    }

    /* Start DHCP server */
    if (FALSE == ExeSysCmd(TRUE, DHCP_SERVER_START_CMD))
    {
        /* Unlock DHCP Server process data */
        MUTEX_UNLOCK(dhcpServerInfo.dataLock);
        EPRINT(DHCP_SERVER, "fail to start dhcp server");
        return;
    }

    /* Unlock DHCP Server process data */
    MUTEX_UNLOCK(dhcpServerInfo.dataLock);
    DPRINT(DHCP_SERVER, "dhcp server started successfully");
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Stop DHCP server related process
 * @param   isRetainLeaseData
 */
static void stopDhcpServerProcess(BOOL isRetainLeaseData)
{
    /* Delete DHCP server process timer */
    stopDhcpServerProcessTimer();

    /* Lock DHCP server process data */
    MUTEX_LOCK(dhcpServerInfo.dataLock);

    /* Stop DHCP server */
    ExeSysCmd(TRUE, DHCP_SERVER_STOP_CMD);

    /* Check and remove lease file if required */
    if (isRetainLeaseData == FALSE)
    {
        /* Remove lease file */
        DPRINT(DHCP_SERVER, "dhcp server lease file removed");
        unlink(DHCP_SERVER_LEASE_FILE_PATH);
    }

    /* Unlock DHCP server process data */
    MUTEX_UNLOCK(dhcpServerInfo.dataLock);

    /* Reset lease information of all the clients */
    resetAllClientLeaseInfo();
    DPRINT(DHCP_SERVER, "dhcp server process stopped");
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Restart DHCP server process
 */
static void restartDhcpServerProcess(BOOL isRetainLeaseData)
{
    /* Terminate DHCP server process */
    stopDhcpServerProcess(isRetainLeaseData);

    /* Start DHCP server process */
    startDhcpServerProcessTimer(DHCP_SERVER_START_ON_CNFG_CHANGE_TMR);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Prepare DHCP server config file
 * @return  SUCCESS / FAIL
 */
static BOOL prepareDhcpServerConfigFile(void)
{
    FILE *pDhcpConfFile;
    CHAR  lan1MacAddr[MAX_MAC_ADDRESS_WIDTH] = {0};

    /* Open file in write mode */
    pDhcpConfFile = fopen(DHCP_SERVER_CONFIG_FILE_PATH, "w");
    if (NULL == pDhcpConfFile)
    {
        /* Failed to create config file */
        EPRINT(DHCP_SERVER, "fail to create dhcp server config file: [err=%s]", STR_ERR);
        return FAIL;
    }

    /* Add DHCP server config message */
    fprintf(pDhcpConfFile, "## Configuration file for DHCP server ##\n\n");

    /* It is usefull when multiple instances of dnsmasq is required */
    fprintf(pDhcpConfFile, "bind-interfaces\n\n");

    /* Disable DNS server feature in dnsmasq */
    fprintf(pDhcpConfFile, "port=0\n\n");

    /* Set interface name on which dnsmasq will start DHCP server */
    fprintf(pDhcpConfFile, "interface=%s\n\n", GetLanPortName(dhcpServerInfo.lanPort));

    /* Add start ip, subnet, end ip and lease hours for allocation */
    fprintf(pDhcpConfFile, "dhcp-range=%s,%s,%s,%hdh\n\n", dhcpServerInfo.startIpAddrRange, dhcpServerInfo.endIpAddrRange, dhcpServerInfo.subnet,
            dhcpServerInfo.leaseHour);

    /* Add max allowed lease IPs */
    fprintf(pDhcpConfFile, "dhcp-lease-max=%hu\n\n", dhcpServerInfo.noOfHost);

    /* Add gateway ip address only if configured */
    if (dhcpServerInfo.gateway[0] != '\0')
    {
        /* Add gateway address */
        fprintf(pDhcpConfFile, "dhcp-option=option:router,%s\n\n", dhcpServerInfo.gateway);
    }
    else
    {
        /* Do not send default router in offer if not configured */
        fprintf(pDhcpConfFile, "dhcp-option=option:router\n\n");
    }

    /* Add dns address option only if configured */
    if (dhcpServerInfo.dns[0] != '\0')
    {
        /* Add gateway address */
        fprintf(pDhcpConfFile, "dhcp-option=option:dns-server,%s\n\n", dhcpServerInfo.dns);
    }

    /* Add lease file path */
    fprintf(pDhcpConfFile, "dhcp-leasefile=%s\n\n", DHCP_SERVER_LEASE_FILE_PATH);

    /* Everytime dnsmasq writes a leases file, the below exe will be called */
    fprintf(pDhcpConfFile, "dhcp-script=%s\n\n", DHCP_SERVER_NOTIFY_SCRIPT_PATH);

    /* Ignore ip assignment to our LAN1 port in DHCP mode */
    GetMacAddr(LAN1_PORT, lan1MacAddr);
    fprintf(pDhcpConfFile, "dhcp-host=%s,ignore\n\n", lan1MacAddr);

    /* Assign ip address sequential */
    fprintf(pDhcpConfFile, "dhcp-sequential-ip\n\n");

    /* Execute "dhcp-script" on renewal */
    fprintf(pDhcpConfFile, "script-on-renewal\n\n");

    /* close file */
    fclose(pDhcpConfFile);

    /* Successfully created config file */
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   parseIpAddressInOctet
 * @param   ipAddrstr
 * @param   pIpAddrOctet
 */
static void parseIpAddressInOctet(CHARPTR ipAddrstr, IP_ADDR_OCTET_u *pIpAddrOctet)
{
    /* Reset info before parsing */
    pIpAddrOctet->littelAddr = 0;

    /* Get ip string octet info decimal */
    sscanf(ipAddrstr, "%hhu.%hhu.%hhu.%hhu", &pIpAddrOctet->octet[3], &pIpAddrOctet->octet[2], &pIpAddrOctet->octet[1], &pIpAddrOctet->octet[0]);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Reset lease info of all the clients
 */
static void resetAllClientLeaseInfo(void)
{
    UINT16 leaseCnt;

    /* Lock DHCP server status info */
    MUTEX_LOCK(leaseStatusLock);
    for (leaseCnt = 0; leaseCnt < DHCP_SERVER_LEASE_CLIENT_MAX; leaseCnt++)
    {
        /* Reset DHCP server lease info */
        resetClientLeaseInfo(leaseCnt);
    }
    MUTEX_UNLOCK(leaseStatusLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Reset DHCP server client lease information
 * @param   leaseNo
 */
static void resetClientLeaseInfo(UINT16 leaseNo)
{
    /* Is lease number valid? */
    if (leaseNo >= DHCP_SERVER_LEASE_CLIENT_MAX)
    {
        /* Invalid lease number */
        return;
    }

    /* Reset lease status info */
    memset(&dhcpServerLeaseStatus[leaseNo], 0, sizeof(DHCP_SERVER_LEASE_STATUS_t));
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Check client entry available in status info
 * @param   pDhcpServerNotify
 * @param   pLeaseNo
 * @return  TRUE if available else FALSE
 */
static LEASE_ENTRY_STATUS_e getClientLeaseInfoStatus(DHCP_SERVER_NOTIFY_t *pDhcpServerNotify, UINT16 *pLeaseNo)
{
    /* Check entry in whole list */
    for (*pLeaseNo = 0; *pLeaseNo < DHCP_SERVER_LEASE_CLIENT_MAX; (*pLeaseNo)++)
    {
        /* Is valid ip address? */
        if (dhcpServerLeaseStatus[*pLeaseNo].assignIpAddr[0] == '\0')
        {
            /* Invalid ip address. Hence check next */
            continue;
        }

        /* Are notified and stored ip addresses same? */
        if (strcmp(dhcpServerLeaseStatus[*pLeaseNo].assignIpAddr, pDhcpServerNotify->assignIpAddr))
        {
            /* Both IPs are different */
            continue;
        }

        /* Are MAC addresses also same? */
        if (strcmp(dhcpServerLeaseStatus[*pLeaseNo].clientMacAddr, pDhcpServerNotify->clientMacAddr))
        {
            /* IPs are same but MACs are different */
            return LEASE_ENTRY_CONFLICT;
        }

        /* Valid lease entry found */
        return LEASE_ENTRY_FOUND;
    }

    /* Lease entry not found */
    return LEASE_ENTRY_NOT_FOUND;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Add or Update client lease information
 * @param   pDhcpServerNotify
 * @param   leaseNo
 * @return
 */
static BOOL updateClientLeaseInfo(DHCP_SERVER_NOTIFY_t *pDhcpServerNotify, UINT16 leaseNo)
{
    UINT16 leaseCnt;

    /* Do we have to assign fresh lease number ? Yes --> new lease : No --> update lease */
    if (leaseNo >= DHCP_SERVER_LEASE_CLIENT_MAX)
    {
        /* We have to assign fresh number as this is new lease entry */
        for (leaseCnt = 0; leaseCnt < DHCP_SERVER_LEASE_CLIENT_MAX; leaseCnt++)
        {
            /* Is invalid ip address? */
            if (dhcpServerLeaseStatus[leaseCnt].assignIpAddr[0] == '\0')
            {
                /* Free lease entry found */
                break;
            }
        }

        /* Is status info full? */
        if (leaseCnt >= DHCP_SERVER_LEASE_CLIENT_MAX)
        {
            /* No free entry found in status info */
            return FAIL;
        }

        /* Store assigned entry number */
        leaseNo = leaseCnt;
    }

    /* Store lease info on given lease number */
    snprintf(dhcpServerLeaseStatus[leaseNo].assignIpAddr, sizeof(dhcpServerLeaseStatus[leaseNo].assignIpAddr), "%s", pDhcpServerNotify->assignIpAddr);
    snprintf(dhcpServerLeaseStatus[leaseNo].clientMacAddr, sizeof(dhcpServerLeaseStatus[leaseNo].clientMacAddr), "%s",
             pDhcpServerNotify->clientMacAddr);
    if (pDhcpServerNotify->clientHostname[0] != '\0')
    {
        snprintf(dhcpServerLeaseStatus[leaseNo].clientHostname, sizeof(dhcpServerLeaseStatus[leaseNo].clientHostname), "%s",
                 pDhcpServerNotify->clientHostname);
    }
    dhcpServerLeaseStatus[leaseNo].leaseExpireTime = pDhcpServerNotify->leaseExpireTime;
    DPRINT(DHCP_SERVER, "lease entry updated: [leaseNo=%d], [ip=%s], [mac=%s]", leaseNo, pDhcpServerNotify->assignIpAddr,
           pDhcpServerNotify->clientMacAddr);
    return SUCCESS;
}

// #################################################################################################
//  @END OF FILE
// #################################################################################################
