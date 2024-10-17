//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		DdnsClient.c
@brief      This module implements DynDns Client using open source 'inadyn' utility to register
            configure NVR's hostname to DynDNS server. This utility will be run by a thread as
            separate process using system command whenever required. The outcome of registration by
            inadyn is stored in inadyn logfile "/tmp/dyndnsLogFile" in a particular format. This
            module provide API to parse this file to determine  whether the registration is successful
            or not. Following is the directory structure of inadyn utility in NVR required
            1. inadyn; 2. /tmp/ddnsConfFile  (inadyn configuration file); 3. /tmp/dyndnsLogFile
            (file where outcome of IP detection and registration are stored).
            This module will retry to update after pre-defined time if in case the DDNS client is
            terminated. In case of successful update, the client itself try to refresh registration
            after the configured time.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "Utils.h"
#include "DdnsClient.h"
#include "ConfigApi.h"
#include "SysTimer.h"
#include "DebugLog.h"
#include "EventLogger.h"
#include "CameraInterface.h"

/* Library Includes */
#include "nm_iputility.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define DDNS_REG_RETRY_TMR          (SEC_IN_ONE_MIN)    // 60 Second
#define DDNS_REG_MANUAL_CHK_TMR     25                  // Second

#define DDNS_RUNTIME_DIR            "/tmp/inadyn"
#define DDNS_BIN_PATH               BIN_DIR_PATH "/inadyn"
#define DDNS_CONF_FILE_PATH         DDNS_RUNTIME_DIR "/ddns.conf"
#define DDNS_NOTIFY_SCRIPT_PATH     SCRIPTS_DIR_PATH "/ddnsClientNotify.sh"
#define	DDNS_BIN_RUN_CMD            DDNS_BIN_PATH " --no-pidfile --exec-mode=event -f " DDNS_CONF_FILE_PATH " --cache-dir " DDNS_RUNTIME_DIR " -e " DDNS_NOTIFY_SCRIPT_PATH
#define	DDNS_BIN_KILL_CMD           "pkill -TERM -f '" DDNS_BIN_RUN_CMD "'"
#define DDNS_USER_AGENT_STR         "nvr-dyndns"

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef struct
{
    CHAR				lastRegIpAddr[NM_IPADDR_FAMILY_INVALID][IPV6_ADDR_LEN_MAX];
    INT32 				clientFd;
    NW_CMD_REPLY_CB		callback;
    pthread_mutex_t		mutexLock;
}DDNS_REG_INFO_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
// This structure was exported from system timer module and used for delete or reload start timer.
static TIMER_HANDLE     ddnsClientTmrHandle;

//current DDNS Registration Info structure
static DDNS_REG_INFO_t  ddnsRegInfo;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void ddnsClientTimerHandler(UINT32 data);
//-------------------------------------------------------------------------------------------------
static void ddnsReponseTimerHandler(UINT32 data);
//-------------------------------------------------------------------------------------------------
static BOOL startDdnsClient(BOOL isManualRequest);
//-------------------------------------------------------------------------------------------------
static void stopDdnsClient(void);
//-------------------------------------------------------------------------------------------------
static void sendDdnsRegResp(NET_CMD_STATUS_e cmdResp);
//-------------------------------------------------------------------------------------------------
static void createDdnsConfigFile(DDNS_CONFIG_t *ddnsConfig);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initializes the DdnsClient module.
 */
void InitDdnsClient(void)
{
    DDNS_CONFIG_t   ddnsConfig;
    TIMER_INFO_t    ddnsTimer;

    memset(ddnsRegInfo.lastRegIpAddr, '\0', sizeof(ddnsRegInfo.lastRegIpAddr));
    ddnsRegInfo.callback = NULL;
    ddnsRegInfo.clientFd = INVALID_CONNECTION;
    MUTEX_INIT(ddnsRegInfo.mutexLock, NULL);
    ddnsClientTmrHandle = INVALID_TIMER_HANDLE;

    ReadDdnsConfig(&ddnsConfig);
    if(ddnsConfig.ddns == DISABLE)
    {
        return;
    }

    /* Create DDNS Configuration File using working copy of DDNS CFG */
    createDdnsConfigFile(&ddnsConfig);

    ddnsTimer.count = CONVERT_SEC_TO_TIMER_COUNT(DDNS_REG_RETRY_TMR);
    ddnsTimer.data = 0;
    ddnsTimer.funcPtr = ddnsClientTimerHandler;
    StartTimer(ddnsTimer, &ddnsClientTmrHandle);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function stops the DdnsClient.And removes the DDNS client log file.
 */
void DeInitDdnsClient(void)
{
    stopDdnsClient();
    DPRINT(DDNS_CLIENT, "ddns client deinit is successfull");
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is the function which runs the DDNS client "inadyn" using system command. It deletes
 *          the earlier DDNS client log file. So new log file is created by the client. It also starts
 *          the timer which checks whether the DDNS client has successfully registered or not and
 *          generate log accordingly.
 * @param   isManualRequest
 * @return  SUCCESS/FAIL
 */
static BOOL startDdnsClient(BOOL isManualRequest)
{
    DDNS_CONFIG_t 	ddnsConfig;
    TIMER_INFO_t	ddnsTimer;

    //If DDNS is Enabled
    ReadDdnsConfig(&ddnsConfig);
    if(ddnsConfig.ddns == DISABLE)
    {
        return FAIL;
    }

    MUTEX_LOCK(ddnsRegInfo.mutexLock);
    if (ddnsClientTmrHandle != INVALID_TIMER_HANDLE)
    {
        MUTEX_UNLOCK(ddnsRegInfo.mutexLock);
        return FAIL;
    }

    /* If internet is not available then wait for internet connectivity */
    if (ACTIVE != getInternetConnStatus())
    {
        MUTEX_UNLOCK(ddnsRegInfo.mutexLock);
        WPRINT(DDNS_CLIENT, "internet connectivity is not available");
        ddnsTimer.count = CONVERT_SEC_TO_TIMER_COUNT(DDNS_REG_RETRY_TMR);
        ddnsTimer.data = 0;
        ddnsTimer.funcPtr = ddnsClientTimerHandler;
        StartTimer(ddnsTimer, &ddnsClientTmrHandle);
        return FAIL;
    }

    /* Start DynDNS client */
    ExeSysCmd(TRUE, DDNS_BIN_RUN_CMD);

    /* If it is manual test request then we must have to provide the response */
    if (TRUE == isManualRequest)
    {
        ddnsTimer.count = CONVERT_SEC_TO_TIMER_COUNT(DDNS_REG_MANUAL_CHK_TMR);
        ddnsTimer.data = 0;
        ddnsTimer.funcPtr = ddnsReponseTimerHandler;
        StartTimer(ddnsTimer, &ddnsClientTmrHandle);
    }

    MUTEX_UNLOCK(ddnsRegInfo.mutexLock);
    DPRINT(DDNS_CLIENT, "ddns client is started");
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function updates the new configuration given by input parameter and take necessary
 *          action of start or stopping DDNS client as per to the changes made in configurations.
 * @param   newDdnsConfig
 * @param   oldDdnsConfig
 */
void UpdateDdnsConfig(DDNS_CONFIG_t newDdnsConfig, DDNS_CONFIG_t *oldDdnsConfig)
{
    /* Is enable/disable ddns config? */
    if (newDdnsConfig.ddns != oldDdnsConfig->ddns)
    {
        /* Is ddns config disabled? */
        if (newDdnsConfig.ddns == DISABLE)
        {
            /* Stop ddns client */
            stopDdnsClient();
            return;
        }
    }
    else
    {
        /* Is ddns config disabled? */
        if (newDdnsConfig.ddns == DISABLE)
        {
            /* Nothing to do */
            return;
        }

        /* Stop ddns client to start with new config */
        stopDdnsClient();
    }

    /* Start ddns client with latest config */
    createDdnsConfigFile(&newDdnsConfig);
    startDdnsClient(FALSE);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function stops the running DDNS client and start the DDNS client again, so the IP
 *          detection and registration is done at the instant this function is called.
 * @param   callbackFun
 * @param   clientFd
 * @return  Status
 */
NET_CMD_STATUS_e UpdateDdnsRegIp(NW_CMD_REPLY_CB callbackFun, UINT32 clientFd)
{
    MUTEX_LOCK(ddnsRegInfo.mutexLock);
    if(ddnsRegInfo.callback != NULL)
    {
        MUTEX_UNLOCK(ddnsRegInfo.mutexLock);
        return CMD_TESTING_ON;
    }

    ddnsRegInfo.callback = callbackFun;
    ddnsRegInfo.clientFd = clientFd;
    MUTEX_UNLOCK(ddnsRegInfo.mutexLock);

    /* Stop the running DDNS client */
    stopDdnsClient();
    if(startDdnsClient(TRUE) == FAIL)
    {
        MUTEX_LOCK(ddnsRegInfo.mutexLock);
        ddnsRegInfo.callback = NULL;
        ddnsRegInfo.clientFd = INVALID_CONNECTION;
        MUTEX_UNLOCK(ddnsRegInfo.mutexLock);
        return CMD_PROCESS_ERROR;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   DDNS event received from ddns client
 * @param   event
 * @param   ipAddr
 * @param   errNum
 * @param   errMsg
 */
void DdnsClientEventNotify(CHAR *event, CHAR *ipAddr, CHAR *errNum, CHAR *errMsg)
{
    /* Is error occurred? */
    if (strcmp(event, "error") == STATUS_OK)
    {
        /* Send failure response */
        EPRINT(NETWORK_MANAGER, "ddns ip registration error: [errNum=%s], [errMsg=%s], [ipAddr=%s]", errNum, errMsg, ipAddr);
        return;
    }

    /* Get ip address family from address */
    NM_IpAddrFamily_e ipFamily = NMIpUtil_GetIpAddrFamily(ipAddr);
    if (ipFamily >= NM_IPADDR_FAMILY_INVALID)
    {
        EPRINT(NETWORK_MANAGER, "invld ddns ip addr: [event=%s], [ipAddr=%s]", event, ipAddr);
        return;
    }

    /* Send success response to client */
    sendDdnsRegResp(CMD_SUCCESS);

    /* Is last registered address changed? */
    if (strcmp(ddnsRegInfo.lastRegIpAddr[ipFamily], ipAddr) == STATUS_OK)
    {
        IPRINT(NETWORK_MANAGER, "no change in ddns ip addr: [event=%s], [ipAddr=%s]", event, ipAddr);
        return;
    }

    DPRINT(DDNS_CLIENT, "ddns ip addr updated: [event=%s], [lastIpAddr=%s], [currentIpAddr=%s]", event, ddnsRegInfo.lastRegIpAddr[ipFamily], ipAddr);
    snprintf(ddnsRegInfo.lastRegIpAddr[ipFamily], sizeof(ddnsRegInfo.lastRegIpAddr[ipFamily]), "%s", ipAddr);
    WriteEvent(LOG_NETWORK_EVENT, LOG_DDNS_IP_UPDATE, NULL, ipAddr, EVENT_ALERT);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function starts ddns client
 * @param   data
 */
static void ddnsClientTimerHandler(UINT32 data)
{
    DeleteTimer(&ddnsClientTmrHandle);
    startDdnsClient(FALSE);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function sends response to local/device client
 * @param   data
 */
static void ddnsReponseTimerHandler(UINT32 data)
{
    /* We have not received the response till timeout */
    DeleteTimer(&ddnsClientTmrHandle);
    sendDdnsRegResp(CMD_DDNS_UPDATE_FAILED);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function terminates DynDNS updates to server.
 */
static void stopDdnsClient(void)
{
    MUTEX_LOCK(ddnsRegInfo.mutexLock);
    DeleteTimer(&ddnsClientTmrHandle);
    ExeSysCmd(TRUE, DDNS_BIN_KILL_CMD);
    MUTEX_UNLOCK(ddnsRegInfo.mutexLock);
    DPRINT(DDNS_CLIENT, "ddns client is stopped");
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send ddns registration test response if executed manually
 * @param   cmdResp
 */
static void sendDdnsRegResp(NET_CMD_STATUS_e cmdResp)
{
    MUTEX_LOCK(ddnsRegInfo.mutexLock);
    if (ddnsRegInfo.callback != NULL)
    {
        ddnsRegInfo.callback(cmdResp, ddnsRegInfo.clientFd, TRUE);
        ddnsRegInfo.callback = NULL;
        ddnsRegInfo.clientFd = INVALID_CONNECTION;
    }
    MUTEX_UNLOCK(ddnsRegInfo.mutexLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function creates the configuration file of inadyn utility using the current DDNS
 *          configured parameters of NVR given in the input parameter.
 * @param   ddnsConfig
 */
static void createDdnsConfigFile(DDNS_CONFIG_t *ddnsConfig)
{
    FILE *file;

    /* Flush all data from directory and create directory if not present */
    ExeSysCmd(TRUE, "rm -rf " DDNS_RUNTIME_DIR "/* && mkdir -p " DDNS_RUNTIME_DIR);

    /* Create config file if does not exist or truncate and open it in write mode */
    file = fopen(DDNS_CONF_FILE_PATH, "w+");
    if (file == NULL)
    {
        EPRINT(DDNS_CLIENT, "fail to open ddns config file: [err=%s]", STR_ERR);
        return;
    }

    /* Write add auto generated text message to ddns conf file */
    fprintf(file, "# Auto generated by Matrix DDNS Client. Do not edit!\n");

    /* Write allow ipv6 address to ddns conf file */
    fprintf(file, "allow-ipv6 = true\n");

    /* Write the user update interval to ddns conf file */
    fprintf(file, "period = %d\n", (ddnsConfig->updateDuration * SEC_IN_ONE_MIN));

    /* Write force update ip address (@everyday interval) to ddns conf file */
    fprintf(file, "forced-update = 86400\n");

    /* Write to ignore provider certificate validation errors to ddns conf file */
    fprintf(file, "secure-ssl = false\n");

    /* Write to set nvr's dyndns user agent string to ddns conf file */
    fprintf(file, "user-agent = %s\n", DDNS_USER_AGENT_STR);

    /* Write the extra new line to ddns conf file */
    fprintf(file, "\n");

    /* Write the config file based on the provider */
    if (ddnsConfig->server == DDNS_SERVER_DYN_DNS_ORG)
    {
        /* Write dyndns.org ipv6 address to ddns conf file */
        fprintf(file, "provider ipv6@dyndns.org {\n");

        /* Write the password to ddns conf file */
        fprintf(file, "\tusername = %s\n", ddnsConfig->username);

        /* Write the password to ddns conf file */
        fprintf(file, "\tpassword = %s\n", ddnsConfig->password);

        /* Write the hostname to ddns conf file */
        fprintf(file, "\thostname = %s\n", ddnsConfig->hostname);

        /* Write to ignore provider certificate validation errors to ddns conf file */
        fprintf(file, "\tssl = false\n");

        /* Write the config closing bracket to ddns conf file */
        fprintf(file, "}\n\n");

        /* Write dyndns.org ipv4 address to ddns conf file */
        fprintf(file, "provider default@dyndns.org {\n");

        /* Write the password to ddns conf file */
        fprintf(file, "\tusername = %s\n", ddnsConfig->username);

        /* Write the password to ddns conf file */
        fprintf(file, "\tpassword = %s\n", ddnsConfig->password);

        /* Write the hostname to ddns conf file */
        fprintf(file, "\thostname = %s\n", ddnsConfig->hostname);

        /* Write to ignore provider certificate validation errors to ddns conf file */
        fprintf(file, "\tssl = false\n");

        /* Write the config closing bracket to ddns conf file */
        fprintf(file, "}\n\n");

        DPRINT(DDNS_CLIENT, "ddns config file created for 'dyndns.org'");
    }
    else
    {
        EPRINT(DDNS_CLIENT, "invld ddns config server: [server=%d]", ddnsConfig->server);
    }

    /* Close the ddns config file */
    fclose(file);
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
