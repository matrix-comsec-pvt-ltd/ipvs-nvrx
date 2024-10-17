//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		MatrixMacClient.C
@brief      This file act as client of matrix mac server. It updates the public IP of device to
            matrix mac server.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "MatrixMacClient.h"
#include "HttpClient.h"
#include "SysTimer.h"
#include "NetworkController.h"
#include "EventLogger.h"
#include "NetworkManager.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define MAC_SERVER_RESPONSE         "<Request Successful>"
#define MAC_SERVER_ERROR_RESP		"<Request Failed: Host Name already assigned>"
#define SMART_CODE_MAC				"28574"
#define HTTP_FORCE_RETRY_PERIOD		10	// Change in Enable FLag
#define HTTP_FAIL_RETRY_PERIOD		(1 * SEC_IN_ONE_MIN) // URL Get FAIL
#define HTTP_SUCCESS_RETRY_PERIOD	(10 * SEC_IN_ONE_MIN) // URL Gets SUCCESS
#define ACTION_SET					"set"
#define ACTION_UPDATE				"update"
#define SCRAMBLE_ARRAY_NO			8
#define SCRAMBLE_ARRAY_SZ			8
#define MAC_ADDRESS_SIZE			6
#define HTTP_PORT					80
#define REG_MAC_FSP					';'
#define FVS_MAC						'='
#define FSP_MAC						'?'
#define HTTP_TIMEOUT				20

//#################################################################################################
// @DATA_TYPES
//#################################################################################################
typedef enum
{
    UPDATE_MAC_SERVER = 0,
	UPDATE_HOST_NAME,
	MAX_MAC_CLIENT_STATE
}MAC_CLIENT_STATE_e;

typedef enum
{
	MACS_IP_REQ_IP_REG,
	MACS_IP_REQ_HOST_REG
}MAC_SERVER_IP_REQ_e;

typedef	enum
{
	MACS_REQ_GETDATA = 0,
	MACS_REQ_SETDATA,
	MACS_REQ_UPDATEDATA,
	MAX_MAC_SERVER_REQ
}MACS_SERVER_REQ_e;

typedef	enum
{
	MACS_ARG_CODE = 0,
	MACS_ARG_ACT,
	MACS_ARG_ID,
	MACS_ARG_FEATURE,
	MACS_ARG_PSWD,
	MACS_ARG_IP,
	MACS_ARG_HOST_NAME,
	MACS_ARG_PORT,
	MAX_MAC_SERVER_ARG
}MAC_SERVER_ARG_e;

typedef struct
{
	BOOL						status;
	NET_CMD_STATUS_e			updateStatus;
	INT32						connFd;
	NW_CMD_REPLY_CB				callback;
	pthread_mutex_t				mutex;
	MATRIX_DNS_SERVER_CONFIG_t	matrixDnsServerCfg;
	CHAR						ipAddress[IPV4_ADDR_LEN_MAX];
}UPDT_HOST_NAME_PARAM_t;

typedef struct
{
	BOOL						status;
	BOOL						actionStatus;
	BOOL						lastUpdate;
	TIMER_HANDLE				timerHandle;
	CHAR						publicIP[IPV4_ADDR_LEN_MAX];
	pthread_mutex_t				mutex;
}UPDT_IP_PARAM_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static UPDT_IP_PARAM_t	updtIp =
{
	.status = FREE,
	.actionStatus = FAIL,
	.lastUpdate = DISABLE,
	.timerHandle = INVALID_TIMER_HANDLE,
	.publicIP = "",
	.mutex = PTHREAD_MUTEX_INITIALIZER,
};

static UPDT_HOST_NAME_PARAM_t updtHostNameParam =
{
	.status = FREE,
	.updateStatus = CMD_PROCESS_ERROR,
	.connFd = INVALID_CONNECTION,
	.callback = NULL,
	.mutex = PTHREAD_MUTEX_INITIALIZER,
	.matrixDnsServerCfg = {0 , "" , 0},
	.ipAddress = ""
};

static const CHARPTR serverReqStr[MAX_MAC_SERVER_REQ] =
{
	"get_data",
	"set_datav2",
	"update_data"
};

static const CHARPTR serverArgStr[MAX_MAC_SERVER_ARG] =
{
	"smart_code",
	"action",
	"identifier",
	"feature",
	"password",
	"ip_address",
	"host_name",
	"forward_port"
};

static const UINT8 scrambledBitPos[SCRAMBLE_ARRAY_NO][SCRAMBLE_ARRAY_SZ] =
{
	{ 0x74 , 0x47 , 0x22 , 0x65 , 0x10 , 0x36 , 0x53 , 0x01 },
	{ 0x16 , 0x70 , 0x34 , 0x07 , 0x61 , 0x52 , 0x45 , 0x23 },
	{ 0x50 , 0x17 , 0x64 , 0x21 , 0x72 , 0x05 , 0x46 , 0x33 },
	{ 0x11 , 0x77 , 0x63 , 0x55 , 0x30 , 0x02 , 0x26 , 0x44 },
	{ 0x04 , 0x25 , 0x73 , 0x12 , 0x56 , 0x43 , 0x62 , 0x31 },
	{ 0x13 , 0x00 , 0x75 , 0x27 , 0x40 , 0x32 , 0x71 , 0x67 },
	{ 0x51 , 0x14 , 0x37 , 0x54 , 0x35 , 0x42 , 0x20 , 0x66 },
	{ 0x76 , 0x03 , 0x15 , 0x60 , 0x06 , 0x41 , 0x57 , 0x24 }
};

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void getScrambledData(UINT8PTR dest, CHARPTR macAddrStr, UINT8PTR key);
//-------------------------------------------------------------------------------------------------
static void macClientTimeout(UINT32 data);
//-------------------------------------------------------------------------------------------------
static void macServerResponseCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void setTimer(UINT16 data);
//-------------------------------------------------------------------------------------------------
static void constructHttpRequest(HTTP_INFO_t *httpPtr, MAC_CLIENT_STATE_e state, MATRIX_DNS_SERVER_CONFIG_t *cfgPtr, CHARPTR ipAddr);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function Starts a Timer to initialise MacClient.
 */
void InitMacClient(void)
{
	MATRIX_DNS_SERVER_CONFIG_t	matrixDnsServerCfg;

	ReadMatrixDnsServerConfig(&matrixDnsServerCfg);
    updtIp.publicIP[0] = '\0';

	if (matrixDnsServerCfg.enMacClient == ENABLE)
	{
		setTimer(HTTP_FAIL_RETRY_PERIOD);
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function Starts a Timer to initialise MacClient.
 * @param   newCopy
 * @param   oldCopy
 */
void UpdateMatrixDnsServerCfg(MATRIX_DNS_SERVER_CONFIG_t *newCopy, MATRIX_DNS_SERVER_CONFIG_t *oldCopy)
{
    /* Check if the feature is changed from disabled <--> enabled */
    if (oldCopy->enMacClient == newCopy->enMacClient)
	{
        return;
    }

    MUTEX_LOCK(updtIp.mutex);
    if (updtIp.status == FREE)
    {
        MUTEX_UNLOCK(updtIp.mutex);
        setTimer(HTTP_FORCE_RETRY_PERIOD);
    }
    else
    {
        MUTEX_UNLOCK(updtIp.mutex);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function Starts a Timer to initialise MacClient.
 * @param   dnsServerCfg
 * @param   callback
 * @param   connFd
 * @return
 */
NET_CMD_STATUS_e UpdateMacClientHostName(MATRIX_DNS_SERVER_CONFIG_t dnsServerCfg, NW_CMD_REPLY_CB callback, INT32 connFd)
{
#if !defined(OEM_JCI)
    HTTP_INFO_t     httpInfo;
    HTTP_HANDLE     httpHandle;
    LAN_CONFIG_t    lan1Cfg;

    MUTEX_LOCK(updtHostNameParam.mutex);
    if (updtHostNameParam.status == BUSY)
	{
        MUTEX_UNLOCK(updtHostNameParam.mutex);
        return CMD_REQUEST_IN_PROGRESS;
    }

    if (SUCCESS != ReadLan1ConfigCms(&lan1Cfg))
    {
        MUTEX_UNLOCK(updtHostNameParam.mutex);
        EPRINT(MAC_CLIENT, "fail to read lan1 config");
        return CMD_PROCESS_ERROR;
    }
    updtHostNameParam.status = BUSY;
    MUTEX_UNLOCK(updtHostNameParam.mutex);

    snprintf(updtHostNameParam.ipAddress, sizeof(updtHostNameParam.ipAddress), "%s", lan1Cfg.ipv4.lan.ipAddress);
    updtHostNameParam.callback = callback;
    updtHostNameParam.connFd = connFd;
    updtHostNameParam.matrixDnsServerCfg = dnsServerCfg;
    updtHostNameParam.updateStatus = CMD_HOST_NAME_REG_FAIL;
    constructHttpRequest(&httpInfo, UPDATE_HOST_NAME, &updtHostNameParam.matrixDnsServerCfg, updtHostNameParam.ipAddress);

    if (FAIL == StartHttp(GET_REQUEST, &httpInfo, macServerResponseCb, MACS_IP_REQ_HOST_REG, &httpHandle))
    {
        MUTEX_LOCK(updtHostNameParam.mutex);
        updtHostNameParam.status = FREE;
        MUTEX_UNLOCK(updtHostNameParam.mutex);
        return CMD_PROCESS_ERROR;
    }

    return CMD_SUCCESS;
#else
    return CMD_PROCESS_ERROR;
#endif
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function Gets the MAC address of the LAN2 Port and Scramble the Data Array with Predefined Method.
 * @param   dest
 * @param   macAddrStr
 * @param   key
 */
static void getScrambledData(UINT8PTR dest, CHARPTR macAddrStr, UINT8PTR key)
{
	UINT8	value;
	UINT8	sourceByteNo, destByteNo;
	UINT8	sourceBitPos, destBitPos;
	UINT8	dataToScramble[SCRAMBLE_ARRAY_SZ];
	UINT32	loop = 0, subLoop = 0;
	UINT32	macInt[MAC_ADDRESS_SIZE];

	// As this MAC Address is in CHAR form, Convert it into UINT8
    sscanf(macAddrStr, "%x:%x:%x:%x:%x:%x", &macInt[0], &macInt[1], &macInt[2], &macInt[3], &macInt[4], &macInt[5]);
	while(loop < 2)
	{
		dataToScramble[loop] = key[loop];
		loop++;
	}

	// Construct Array to be Scrambled
    while((loop < SCRAMBLE_ARRAY_SZ) && (subLoop < MAC_ADDRESS_SIZE))
	{
		dataToScramble[loop] = macInt[MAC_ADDRESS_SIZE - subLoop - 1];
		loop++;
		subLoop++;
	}

	// Scramble Array
	for(sourceByteNo = 0; sourceByteNo < SCRAMBLE_ARRAY_NO; sourceByteNo++)
	{
		for(sourceBitPos = 0; sourceBitPos < SCRAMBLE_ARRAY_SZ; sourceBitPos++)
		{
			// upper nibble gives byte no
			// lower nibble gives bit position
			destByteNo = (scrambledBitPos[sourceByteNo][sourceBitPos] >> 4);
			destBitPos = (scrambledBitPos[sourceByteNo][sourceBitPos] & 0x0f);
			value = ( (dataToScramble[sourceByteNo] >> sourceBitPos) & 0x01);
			dest[destByteNo] |= (value << destBitPos);
		}
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function is common to construct http request depends on state.
 * @param   httpPtr
 * @param   state
 * @param   cfgPtr
 * @param   ipAddr
 */
static void constructHttpRequest(HTTP_INFO_t *httpPtr, MAC_CLIENT_STATE_e state, MATRIX_DNS_SERVER_CONFIG_t *cfgPtr, CHARPTR ipAddr)
{
    UINT8               scrambledData[SCRAMBLE_ARRAY_SZ];
    CHAR                macAddrStr[MAX_MAC_ADDRESS_WIDTH];
    MAC_SERVER_CNFG_t   macServerCnfg;

	// Construct Http Reuest
	httpPtr->maxConnTime = HTTP_TIMEOUT;
	httpPtr->maxFrameTime = HTTP_TIMEOUT;
	httpPtr->authMethod = AUTH_TYPE_BASIC;
	httpPtr->httpUsrPwd.username[0] = '\0';
	httpPtr->httpUsrPwd.password[0] = '\0';
	httpPtr->userAgent = CURL_USER_AGENT;
	httpPtr->interface = MAX_HTTP_INTERFACE;

	switch (state)
	{
        case UPDATE_MAC_SERVER:
        {
            // Construct Http request to Update IP on MAC Server Read mac Config
            ReadMacServerConfig(&macServerCnfg);

            // Get MAC Address of LAN2 PORT
            GetMacAddr(LAN1_PORT, macAddrStr);

            memset(scrambledData, 0, SCRAMBLE_ARRAY_SZ);
            getScrambledData(scrambledData, macAddrStr, (UINT8PTR)&macServerCnfg.key);

            if (macServerCnfg.connectMode == CONNECT_THROUGH_IP)
            {
                snprintf(httpPtr->ipAddress, MAX_CAMERA_ADDRESS_WIDTH, "%s", macServerCnfg.ip);
            }
            else
            {
                snprintf(httpPtr->ipAddress, MAX_CAMERA_ADDRESS_WIDTH, "%s", macServerCnfg.name);
            }

            httpPtr->port = macServerCnfg.port;
            updtIp.lastUpdate = cfgPtr->enMacClient;
            snprintf(httpPtr->relativeUrl, MAX_RELATIVE_URL_WIDTH, "/%s/%s%c%s%c%s%c%s%c%s%c%s%c%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x%c%s%c%d%c%s%c%s",
                     macServerCnfg.service, serverReqStr[MACS_REQ_SETDATA], FSP_MAC, serverArgStr[MACS_ARG_CODE], FVS_MAC, SMART_CODE_MAC, REG_MAC_FSP,
                     serverArgStr[MACS_ARG_ACT], FVS_MAC, ACTION_SET, REG_MAC_FSP, serverArgStr[MACS_ARG_ID], FVS_MAC, scrambledData[0],
                    scrambledData[1], scrambledData[2], scrambledData[3], scrambledData[4], scrambledData[5], scrambledData[6], scrambledData[7], REG_MAC_FSP,
                    serverArgStr[MACS_ARG_FEATURE], FVS_MAC, updtIp.lastUpdate, REG_MAC_FSP, serverArgStr[MACS_ARG_IP], FVS_MAC, ipAddr);
            DPRINT(MAC_CLIENT, "update mac server: [ip=%s], [port=%d], [url=%s]", httpPtr->ipAddress, httpPtr->port, httpPtr->relativeUrl);
        }
        break;

        case UPDATE_HOST_NAME:
        {
            // Construct Http request to Update IP on MAC Server Read mac Config
            ReadMacServerConfig(&macServerCnfg);

            // Get MAC Address of LAN2 PORT
            GetMacAddr(LAN1_PORT, macAddrStr);

            memset(scrambledData, 0, SCRAMBLE_ARRAY_SZ);
            getScrambledData(scrambledData, macAddrStr, (UINT8PTR)&macServerCnfg.key);

            if(macServerCnfg.connectMode == CONNECT_THROUGH_IP)
            {
                snprintf(httpPtr->ipAddress, MAX_CAMERA_ADDRESS_WIDTH, "%s", macServerCnfg.ip);
            }
            else
            {
                snprintf(httpPtr->ipAddress, MAX_CAMERA_ADDRESS_WIDTH, "%s", macServerCnfg.name);
            }

            httpPtr->port = macServerCnfg.port;
            snprintf(httpPtr->relativeUrl, MAX_RELATIVE_URL_WIDTH, "/%s/%s%c%s%c%s%c%s%c%s%c%s%c%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x"
                                                                   "%c%s%c%s%c%s%c%d%c%s%c%s",
                     macServerCnfg.service, serverReqStr[MACS_REQ_UPDATEDATA], FSP_MAC, serverArgStr[MACS_ARG_CODE], FVS_MAC, SMART_CODE_MAC, REG_MAC_FSP,
                     serverArgStr[MACS_ARG_ACT], FVS_MAC, ACTION_UPDATE, REG_MAC_FSP, serverArgStr[MACS_ARG_ID], FVS_MAC, scrambledData[0],
                    scrambledData[1], scrambledData[2], scrambledData[3], scrambledData[4], scrambledData[5], scrambledData[6], scrambledData[7], REG_MAC_FSP,
                    serverArgStr[MACS_ARG_HOST_NAME], FVS_MAC, cfgPtr->hostName, REG_MAC_FSP, serverArgStr[MACS_ARG_PORT], FVS_MAC,
                    cfgPtr->forwardedPort, REG_MAC_FSP, serverArgStr[MACS_ARG_IP], FVS_MAC, ipAddr);
            DPRINT(MAC_CLIENT, "update host name: [ip=%s], [port=%d], [url=%s]", httpPtr->ipAddress, httpPtr->port, httpPtr->relativeUrl);
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function constructs URL of DynDns web server and handle it to Http client with
 *          callback of getIpAddressCb.
 * @param   data
 */
static void macClientTimeout(UINT32 data)
{
	HTTP_INFO_t 	httpInfo;
	HTTP_HANDLE		httpHandle;
	LAN_CONFIG_t	lan1Cfg = { 0 };
	MATRIX_DNS_SERVER_CONFIG_t	matrixDnsServerCfg;

	ReadMatrixDnsServerConfig(&matrixDnsServerCfg);

    MUTEX_LOCK(updtIp.mutex);
	updtIp.status = BUSY;
    MUTEX_UNLOCK(updtIp.mutex);

	// Initialise with FAIL to trace status of this action
	updtIp.actionStatus = FAIL;

    if (updtIp.publicIP[0] == '\0')
	{
		ReadLan1ConfigCms(&lan1Cfg);
        snprintf(updtIp.publicIP, sizeof(updtIp.publicIP), "%s", lan1Cfg.ipv4.lan.ipAddress);
	}

    constructHttpRequest(&httpInfo, UPDATE_MAC_SERVER, &matrixDnsServerCfg, updtIp.publicIP);

    if (StartHttp(GET_REQUEST, &httpInfo, macServerResponseCb, MACS_IP_REQ_IP_REG, &httpHandle) == FAIL)
	{
        EPRINT(MAC_CLIENT, "failed to start http client");
	}

	DeleteTimer(&updtIp.timerHandle);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function Starts timer for given time and Set function Pointer to macClientTimeOut Function.
 */
static void setTimer(UINT16 data)
{
#if !defined(OEM_JCI)
    TIMER_INFO_t timer;

	timer.data = 0;
	timer.count = CONVERT_SEC_TO_TIMER_COUNT(data);
	timer.funcPtr = macClientTimeout;

    if (updtIp.timerHandle == INVALID_TIMER_HANDLE)
	{
		// Start a new timer
        StartTimer(timer, &updtIp.timerHandle);
	}
	else
	{
        ReloadTimer(updtIp.timerHandle, timer.count);
	}
#endif
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function Parse the Response of the MAC Server. If the correct response is received,
 *          it again starts timer if the MAC feature is enabled If the response is invalid it starts
 *          timer to retry.
 * @param   httpHandle
 * @param   dataInfo
 */
static void macServerResponseCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo)
{
    DPRINT(MAC_CLIENT, "http client response: [reqType=%d], [response=%d], [handle=%d]", dataInfo->userData, dataInfo->httpResponse, httpHandle);
	if (dataInfo->userData == MACS_IP_REQ_HOST_REG)
	{
		switch(dataInfo->httpResponse)
		{
            case HTTP_SUCCESS:
            {
                // Check response of MAC Server
                if ((dataInfo->storagePtr != NULL) && (dataInfo->frameSize > 0))
                {
                    // Check if Response is successful
                    if(strncmp( (CHARPTR)dataInfo->storagePtr, MAC_SERVER_RESPONSE, strlen(MAC_SERVER_RESPONSE) ) == STATUS_OK)
                    {
                        updtHostNameParam.updateStatus = CMD_SUCCESS;
                    }
                    // Check if error is "host name already assigned"
                    else if(strncmp( (CHARPTR)dataInfo->storagePtr, MAC_SERVER_ERROR_RESP, strlen(MAC_SERVER_ERROR_RESP) ) == STATUS_OK)
                    {
                        updtHostNameParam.updateStatus = CMD_HOST_NAME_DUPLICATION;
                    }
                    else
                    {
                        updtHostNameParam.updateStatus = CMD_HOST_NAME_REG_FAIL;
                    }
                }
            }
            break;

            case HTTP_CLOSE_ON_SUCCESS:
            case HTTP_CLOSE_ON_ERROR:
            {
                if(updtHostNameParam.callback != NULL)
                {
                    updtHostNameParam.callback(updtHostNameParam.updateStatus, updtHostNameParam.connFd, TRUE);
                    updtHostNameParam.callback = NULL;
                    updtHostNameParam.connFd = INVALID_CONNECTION;
                }

                MUTEX_LOCK(updtHostNameParam.mutex);
                updtHostNameParam.status = FREE;
                MUTEX_UNLOCK(updtHostNameParam.mutex);
                DPRINT(MAC_CLIENT, "update host name: [status=%d]", updtHostNameParam.updateStatus);
            }
            break;

            default:
            {
                /* Nothing to do */
            }
            break;
		}
	}
	else
	{
		switch(dataInfo->httpResponse)
		{
            case HTTP_SUCCESS:
            {
                // Check response of MAC Server
                if ((dataInfo->storagePtr != NULL) && (dataInfo->frameSize > 0)
                    && (strncmp((CHARPTR)dataInfo->storagePtr, MAC_SERVER_RESPONSE, strlen(MAC_SERVER_RESPONSE)) == STATUS_OK))
                {
                    updtIp.actionStatus = SUCCESS;
                    WriteEvent(LOG_NETWORK_EVENT, LOG_MAC_SERVER_UPDATE, updtIp.publicIP, NULL, EVENT_ALERT);
                    DPRINT(MAC_CLIENT, "ip addr updated on mac server");
                }
            }
            break;

            case HTTP_CLOSE_ON_ERROR:
            case HTTP_CLOSE_ON_SUCCESS:
            {
                UINT16						retryTime = 0;
                MATRIX_DNS_SERVER_CONFIG_t	matrixDnsServerCfg;

                /* Read dns server config */
                ReadMatrixDnsServerConfig(&matrixDnsServerCfg);

                /* Check if Public IP Updated on Server */
                if (updtIp.actionStatus == SUCCESS)
                {
                    if (updtIp.lastUpdate != matrixDnsServerCfg.enMacClient)
                    {
                        retryTime = HTTP_FORCE_RETRY_PERIOD;
                    }
                    else
                    {
                        if (matrixDnsServerCfg.enMacClient == ENABLE)
                        {
                            retryTime = HTTP_SUCCESS_RETRY_PERIOD;
                        }
                        else
                        {
                            DPRINT(MAC_CLIENT, "mac client disabled. hence stopped dns server ip update process on success");
                        }
                    }
                }
                else
                {
                    if (matrixDnsServerCfg.enMacClient == ENABLE)
                    {
                        retryTime = HTTP_FAIL_RETRY_PERIOD;
                        EPRINT(MAC_CLIENT, "failed to update ip on mac server");
                    }
                    else
                    {
                        EPRINT(MAC_CLIENT, "mac client disabled. hence stopped dns server ip update process on failure");
                    }
                }

                if (retryTime > 0)
                {
                    setTimer(retryTime);
                }

                MUTEX_LOCK(updtIp.mutex);
                updtIp.status = FREE;
                MUTEX_UNLOCK(updtIp.mutex);
            }
            break;

            default:
            {
                /* Nothing to do */
            }
            break;
		}
	}
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
