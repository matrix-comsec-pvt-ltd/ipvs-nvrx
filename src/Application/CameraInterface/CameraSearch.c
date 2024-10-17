//#################################################################################################
// @FILE BRIEF
//#################################################################################################
/**
@file       CameraSearch.c
@brief      This File Provides to Search Camera using UPnP Protocol & ONVIF Device Discovery Service.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "DebugLog.h"
#include "MxOnvifClient.h"
#include "NetworkController.h"
#include "CameraSearch.h"
#include "CameraInterface.h"
#include "NetworkCommand.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define	CAM_SEARCH_TIMEOUT			5		//second
#define CAM_SRCH_THREAD_STACK_SZ    (4 * MEGA_BYTE)

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static VOIDPTR camSearchThread(VOIDPTR data);
//-------------------------------------------------------------------------------------------------
static void endCameraSearchProcess(void);
//-------------------------------------------------------------------------------------------------
static BOOL	sendHttpReq(void);
//-------------------------------------------------------------------------------------------------
static BOOL onvifCamSearchCb(VOIDPTR data);
//-------------------------------------------------------------------------------------------------
static void httpCamInfoCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t * dataInfo);
//-------------------------------------------------------------------------------------------------
static void deleteNotUpdatedEntryFromList(void);
//-------------------------------------------------------------------------------------------------
static void triggerCameraSearchThread(void);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static IP_CAM_SEARCH_SESSION_PARAM_t camSearchSession =
{
    .requestStatus = INACTIVE,
    .totalDevices = 0,
    .devicesFoundInUpnP = 0,
    .requestCount = 0,
    .httpReqStatus = FAIL,
    .pData = NULL,
    .reqStatusLock = PTHREAD_MUTEX_INITIALIZER
};

static pthread_mutex_t  resultLock = PTHREAD_MUTEX_INITIALIZER;
static BOOL             sessionActiveF[MAX_CAMERA_SEARCH_SESSION];
static pthread_mutex_t  sessionActiveFlagLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t  threadWaitMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t   threadWaitVar = PTHREAD_COND_INITIALIZER;

static const CHAR *validUPnPDeviceType[] =
{
    "Basic",
    "tvdevice",
    "DigitalSecurityCamera",
    "EmbeddedNetDevice",
    "ology",
    NULL,
};

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   Init camera search info
 */
void InitCameraSearch(void)
{
    UINT8 index;
    MUTEX_LOCK(sessionActiveFlagLock);
	for(index = 0; index < MAX_CAMERA_SEARCH_SESSION; index++)
	{
		sessionActiveF[index] = INACTIVE;
	}
    MUTEX_UNLOCK(sessionActiveFlagLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API Start Camera Search If Session is INACTIVE, otherwise it will return CMD_REQUEST_IN_PROGRESS.
 * @param   sessionIndex
 * @return
 */
NET_CMD_STATUS_e StartCameraSearch(UINT8 sessionIndex)
{
    MUTEX_LOCK(camSearchSession.reqStatusLock);
	if (camSearchSession.requestStatus == INACTIVE)
	{
        MUTEX_UNLOCK(camSearchSession.reqStatusLock);

        /* Create the detached thread to start camera search process */
        if (FALSE == Utils_CreateThread(NULL, camSearchThread, NULL, DETACHED_THREAD, CAM_SRCH_THREAD_STACK_SZ))
        {
            return CMD_RESOURCE_LIMIT;
		}
	}
    else if (camSearchSession.requestStatus == INTERRUPTED)
    {
        MUTEX_UNLOCK(camSearchSession.reqStatusLock);
        return CMD_REQUEST_IN_PROGRESS;
    }
	else
	{
        MUTEX_UNLOCK(camSearchSession.reqStatusLock);
	}

    MUTEX_LOCK(sessionActiveFlagLock);
    sessionActiveF[sessionIndex] = ACTIVE;
    MUTEX_UNLOCK(sessionActiveFlagLock);
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API Stops Camera Search if it is running.
 * @param   sessionIndex
 */
void StopCameraSearch(UINT8 sessionIndex)
{
    UINT8 index;

    if (sessionIndex >= MAX_CAMERA_SEARCH_SESSION)
    {
        return;
    }

    MUTEX_LOCK(sessionActiveFlagLock);
	sessionActiveF[sessionIndex] = INACTIVE;
	for(index = 0; index < MAX_CAMERA_SEARCH_SESSION; index++)
	{
		if(sessionActiveF[index] == ACTIVE)
		{
			break;
		}
	}
    MUTEX_UNLOCK(sessionActiveFlagLock);

    MUTEX_LOCK(camSearchSession.reqStatusLock);
    if((index >= MAX_CAMERA_SEARCH_SESSION) && (camSearchSession.requestStatus == ACTIVE))
	{
		camSearchSession.requestStatus = INTERRUPTED;
		triggerCameraSearchThread();
	}
    MUTEX_UNLOCK(camSearchSession.reqStatusLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   GetAcqListOfCameras
 * @param   result
 * @param   numOfResult
 * @return
 */
NET_CMD_STATUS_e GetAcqListOfCameras(IP_CAM_SEARCH_RESULT_t *result, UINT8PTR numOfResult)
{
    UINT8 index;

    MUTEX_LOCK(resultLock);
    if(camSearchSession.totalDevices == 0)
	{
        MUTEX_UNLOCK(resultLock);
        return CMD_PROCESS_ERROR;
    }

    for(index = 0; index < camSearchSession.totalDevices; index++)
    {
        *(result + index) = camSearchSession.pData->result[index];
    }
    *numOfResult = camSearchSession.totalDevices;
    MUTEX_UNLOCK(resultLock);
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   endCameraSearchProcess
 */
static void endCameraSearchProcess(void)
{
    UINT8 index;

    MUTEX_LOCK(sessionActiveFlagLock);
	for(index = 0; index < MAX_CAMERA_SEARCH_SESSION; index++)
	{
		if(sessionActiveF[index] == ACTIVE)
		{
            MUTEX_LOCK(camSearchSession.reqStatusLock);
			if(camSearchSession.requestStatus == INTERRUPTED)
			{
				camSearchSession.requestStatus = ACTIVE;
			}
            MUTEX_UNLOCK(camSearchSession.reqStatusLock);
			break;
		}
	}
    MUTEX_UNLOCK(sessionActiveFlagLock);

    if (index < MAX_CAMERA_SEARCH_SESSION)
    {
		deleteNotUpdatedEntryFromList();

        // create the detached thread to start camera search process
        if (FALSE == Utils_CreateThread(NULL, camSearchThread, NULL, DETACHED_THREAD, CAM_SRCH_THREAD_STACK_SZ))
        {
            MUTEX_LOCK(camSearchSession.reqStatusLock);
            camSearchSession.requestStatus = INACTIVE;
            MUTEX_UNLOCK(camSearchSession.reqStatusLock);
        }
	}
	else
	{
        MUTEX_LOCK(camSearchSession.reqStatusLock);
        MUTEX_LOCK(resultLock);
		camSearchSession.totalDevices = 0;
        FREE_MEMORY(camSearchSession.pData);
        MUTEX_UNLOCK(resultLock);
        camSearchSession.requestStatus = INACTIVE;
        MUTEX_UNLOCK(camSearchSession.reqStatusLock);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   deleteNotUpdatedEntryFromList
 */
static void deleteNotUpdatedEntryFromList(void)
{
    UINT8 index, remainIndex;

    MUTEX_LOCK(resultLock);
    if (camSearchSession.pData == NULL)
    {
        MUTEX_UNLOCK(resultLock);
        return;
    }

    for (index = 0; index < camSearchSession.totalDevices; index++)
    {
        if (camSearchSession.pData->result[index].updationOnNewSearch == TRUE)
        {
            continue;
        }

        for (remainIndex = (index + 1); remainIndex < camSearchSession.totalDevices; remainIndex++)
        {
            camSearchSession.pData->result[remainIndex-1] = camSearchSession.pData->result[remainIndex];
        }

        if (camSearchSession.totalDevices)
        {
            camSearchSession.totalDevices--;
        }
    }
    MUTEX_UNLOCK(resultLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   triggerCameraSearchThread
 */
static void triggerCameraSearchThread(void)
{
    MUTEX_LOCK(threadWaitMutex);
	pthread_cond_signal(&threadWaitVar);
    MUTEX_UNLOCK(threadWaitMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   camSearchThread
 * @param   data
 * @return
 */
static VOIDPTR camSearchThread(VOIDPTR data)
{
    INT32 	 				connFd[IP_ADDR_TYPE_MAX];
	INT32 					reuse = 1;
	BOOL					freeSession = FALSE;
    NETWORK_PORT_e			routePort;
	LAN_CONFIG_t			lanConfig;
    ONVIF_REQ_PARA_t		reqParam;
	ONVIF_HANDLE			onvifHandle;
	UINT8					seachCamNoLoop = 0;
    BOOL					isLanStatusUp = TRUE;
	UINT8					searchHoldCnt = 0;
	struct timespec			ts;
    CHAR                    sysCmd[SYS_CMD_MSG_LEN_MAX];
    UINT8                   status = INACTIVE;
    UINT8                   index;
    CHAR                    ipAddressForUrl[DOMAIN_NAME_SIZE_MAX] = { 0 };

    THREAD_START("CAM_SEARCH");

    memset(&lanConfig, 0, sizeof(lanConfig));
    MUTEX_LOCK(camSearchSession.reqStatusLock);
	if(camSearchSession.requestStatus == INACTIVE)
	{
        MUTEX_LOCK(resultLock);
		camSearchSession.totalDevices = 0;
        camSearchSession.pData = malloc(sizeof(DYNAMIC_DATA_t));
        if (camSearchSession.pData != NULL)
        {
            memset(camSearchSession.pData, 0, sizeof(DYNAMIC_DATA_t));
        }
        MUTEX_UNLOCK(resultLock);
		camSearchSession.requestStatus = ACTIVE;
	}
    MUTEX_UNLOCK(camSearchSession.reqStatusLock);

	camSearchSession.requestCount = 0;
    camSearchSession.devicesFoundInUpnP = 0;

    MUTEX_LOCK(resultLock);
    if (camSearchSession.pData != NULL)
    {
        /* Make onvif port default to all search result. If any camera is detected as onvif, then its port will be replaced */
        for(seachCamNoLoop = 0; seachCamNoLoop < MAX_CAM_SEARCH_IN_ONE_SHOT; seachCamNoLoop++)
		{
			camSearchSession.pData->result[seachCamNoLoop].updationOnNewSearch = FALSE;
		}
    }
    else
    {
        freeSession = TRUE;
    }
    MUTEX_UNLOCK(resultLock);

    if (freeSession == FALSE)
    {
        for (routePort = NETWORK_PORT_LAN1; routePort <= NETWORK_PORT_LAN2; routePort++)
        {
            /* Init both ipv4 & ipv6 socket fd to invalid value */
            for (index = 0; index < IP_ADDR_TYPE_MAX; index++)
            {
                connFd[index] = INVALID_FILE_FD;
            }

            /* Check if Lan Port is up */
            if (TRUE == IsNetworkPortLinkUp(routePort))
			{
                if(SUCCESS != GetNetworkParamInfo(routePort, &lanConfig))
				{
                    EPRINT(CAMERA_INTERFACE, "fail to get lan info: [lan=%d]", routePort);
				}
			}
			else
			{
				searchHoldCnt++;
                if (searchHoldCnt != MAX_LAN_PORT)
				{
                    continue;
                }

                searchHoldCnt = 0;
                isLanStatusUp = FALSE;

                clock_gettime(CLOCK_REALTIME, &ts);
                ts.tv_sec += CAM_SEARCH_TIMEOUT;

                /* Wait here to avoid thread start- stop frequently when lan is not up */
                MUTEX_LOCK(threadWaitMutex);
                pthread_cond_timedwait(&threadWaitVar, &threadWaitMutex, &ts);
                MUTEX_UNLOCK(threadWaitMutex);
                break;
			}

            do
            {
                /* Create socket to send UPnP multicast message using ipv4 address */
                connFd[IP_ADDR_TYPE_IPV4] = socket(AF_INET, UDP_SOCK_OPTIONS, 0);

                if (connFd[IP_ADDR_TYPE_IPV4] == INVALID_CONNECTION)
                {
                    EPRINT(CAMERA_INTERFACE, "fail to create multicast socket: [lan=%d], [err=%s]", routePort, STR_ERR);
                    break;
                }

                if (setsockopt(connFd[IP_ADDR_TYPE_IPV4], SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) != STATUS_OK)
                {
                    EPRINT(CAMERA_INTERFACE, "fail to set reuse addr: [lan=%d], [err=%s]", routePort, STR_ERR);
                    CloseSocket(&connFd[IP_ADDR_TYPE_IPV4]);
                    connFd[IP_ADDR_TYPE_IPV4] = INVALID_CONNECTION;
                    break;
                }

                /* Update routing table to receive multicast messages */
                snprintf(sysCmd, sizeof(sysCmd), ADD_MULTICAST_ROUTE_CMD, IPV4_MULTICAST_RECV_ADDRESS, GetNetworkPortName(routePort, NULL));
                if (FALSE == ExeSysCmd(TRUE, sysCmd))
                {
                    EPRINT(CAMERA_INTERFACE, "fail to update routing table: [lan=%d], [err=%s]", routePort, STR_ERR);
                    break;
                }

                /* Prepare ipv4 multicast message to send */
                snprintf(camSearchSession.pData->msgBuffer, MAX_BUFFER_SIZE + 1, SEARCH_MESSAGE_FORMAT, IPV4_MULTICAST_ADDRESS, MULTICAST_PORT);

                /* Send multicast message using ipv4 address */
                if (FAIL == SendMulticastMessage(connFd[IP_ADDR_TYPE_IPV4], lanConfig.ipv4.lan.ipAddress, IPV4_MULTICAST_ADDRESS, MULTICAST_PORT,
                                                 GetNetworkPortName(routePort, NULL), camSearchSession.pData->msgBuffer))
                {
                    EPRINT(CAMERA_INTERFACE, "fail to send multicast message: [lan=%d]", routePort);
                    break;
                }

            } while (0);

            do
            {
                /* Multicast upnp using ipv6 if enabled on interface */
                if (lanConfig.ipAddrMode != IP_ADDR_MODE_DUAL_STACK)
                {
                    break;
                }

                /* Create socket to send UPnP multicast message using ipv6 address */
                connFd[IP_ADDR_TYPE_IPV6] = socket(AF_INET6, UDP_SOCK_OPTIONS, 0);

                if (connFd[IP_ADDR_TYPE_IPV6] == INVALID_CONNECTION)
                {
                    EPRINT(CAMERA_INTERFACE, "fail to create multicast socket: [lan=%d], [err=%s]", routePort, STR_ERR);
                    break;
                }

                if (setsockopt(connFd[IP_ADDR_TYPE_IPV6], SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) != STATUS_OK)
                {
                    EPRINT(CAMERA_INTERFACE, "fail to set reuse addr: [lan=%d], [err=%s]", routePort, STR_ERR);
                    CloseSocket(&connFd[IP_ADDR_TYPE_IPV6]);
                    connFd[IP_ADDR_TYPE_IPV6] = INVALID_FILE_FD;
                    break;
                }

                PrepareIpAddressForUrl(IPV6_MULTICAST_ADDRESS, ipAddressForUrl);

                /* Prepare ipv6 multicast message to send */
                snprintf(camSearchSession.pData->msgBuffer, MAX_BUFFER_SIZE + 1, SEARCH_MESSAGE_FORMAT, ipAddressForUrl, MULTICAST_PORT);

                /* Send multicast message using ipv6 address */
                if (FAIL == SendMulticastMessage(connFd[IP_ADDR_TYPE_IPV6], lanConfig.ipv6.lan.ipAddress, IPV6_MULTICAST_ADDRESS, MULTICAST_PORT,
                                                 GetNetworkPortName(routePort, NULL), camSearchSession.pData->msgBuffer))
                {
                    EPRINT(CAMERA_INTERFACE, "fail to send multicast message: [lan=%d]", routePort);
                    break;
                }

            } while(0);

            status = ACTIVE;
            if ((connFd[IP_ADDR_TYPE_IPV4] != INVALID_FILE_FD) || (connFd[IP_ADDR_TYPE_IPV6] != INVALID_FILE_FD))
            {
                status = RecvUpnpResponse(connFd);
            }

            /* Remove multicast entry from routing table */
            snprintf(sysCmd, sizeof(sysCmd), DEL_MULTICAST_ROUTE_CMD, IPV4_MULTICAST_RECV_ADDRESS, GetNetworkPortName(routePort, NULL));
            if (FALSE == ExeSysCmd(TRUE, sysCmd))
            {
                EPRINT(CAMERA_INTERFACE, "fail to update routing table: [lan=%d], [err=%s]", routePort, STR_ERR);
            }

            /* Close sockets */
            for (index = 0; index < IP_ADDR_TYPE_MAX; index++)
            {
                CloseSocket(&connFd[index]);
            }

            /* If max camera found OR Session interrupted */
            if (ACTIVE != status)
            {
                break;
            }
        }

        if ((status != INTERRUPTED) && (isLanStatusUp == TRUE))
		{
			reqParam.camIndex = 0; //dummy index
            reqParam.camPara.ipAddr[0] = '\0';
			reqParam.camPara.name[0] = '\0';
			reqParam.camPara.port = 0;
			reqParam.camPara.pwd[0] = '\0';
			reqParam.onvifReq = ONVIF_SEARCH_DEVICES;
			reqParam.onvifCallback = (ONVIF_CAMERA_CB)onvifCamSearchCb;

			if (StartOnvifClient(&reqParam, &onvifHandle) == FAIL)
			{
				freeSession = TRUE;
			}
		}
		else
		{
			freeSession = TRUE;
		}
	}

	if (freeSession == TRUE)
	{
		endCameraSearchProcess();
	}

	pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   checkIfCamAdded
 * @param   cameraIndex
 */
static void checkIfCamAdded(UINT8 cameraIndex)
{
	INT16				loop;
	CAMERA_CONFIG_t		camCfg[MAX_CAMERA];
	IP_CAMERA_CONFIG_t  ipCamCfg[MAX_CAMERA];
	CAMERA_BRAND_e		brandNum;
	CAMERA_MODEL_e 		modelNum;
    CHAR				cameraIpAddr[IPV6_ADDR_LEN_MAX];

	ReadCameraConfig(camCfg);
	ReadIpCameraConfig(ipCamCfg);

    if (cameraIndex >= MAX_CAM_SEARCH_IN_ONE_SHOT)
    {
        return;
    }

	camSearchSession.pData->result[cameraIndex].camStatus = CAM_UNIDENTIFIED;
	for (loop = 0; loop < getMaxCameraForCurrentVariant(); loop++)
    {
        if(GetCamIpAddress(loop, cameraIpAddr) == FALSE)
		{
            snprintf(cameraIpAddr, sizeof(cameraIpAddr), "%s", ipCamCfg[loop].cameraAddress);
		}
		// if ip address matches with configuration we send cam as added
		// following change has been done to search virtual cameras on single IP with different ONVIF port. 
        if ((IsCameraAlreadyFound(camSearchSession.pData->result[cameraIndex].ipv4Addr, cameraIpAddr,
                                  camSearchSession.pData->result[cameraIndex].ipv6Addr, cameraIpAddr, cameraIpAddr))
                && (ipCamCfg[loop].onvifPort == camSearchSession.pData->result[cameraIndex].onvifPort))
        {
			camSearchSession.pData->result[cameraIndex].camStatus = CAM_ADDED;
			break;
		}
	}

	if(camSearchSession.pData->result[cameraIndex].camStatus != CAM_ADDED)
	{
		// not added then check brand model if onvif found
        if((GetBrandNum(camSearchSession.pData->result[cameraIndex].brand, &brandNum) == SUCCESS)
                && (GetModelNum(camSearchSession.pData->result[cameraIndex].brand, camSearchSession.pData->result[cameraIndex].model, &modelNum) == SUCCESS))
		{
			camSearchSession.pData->result[cameraIndex].camStatus = CAM_IDENTIFIED;
		}
	}

	if (camSearchSession.pData->result[cameraIndex].camStatus == CAM_ADDED)
	{
        snprintf(camSearchSession.pData->result[cameraIndex].camName,MAX_CAMERA_NAME_WIDTH, "%s", camCfg[loop].name);
		camSearchSession.pData->result[cameraIndex].camIndex = (loop + 1);
	}
	else
	{
		// When added one it will give 0 (Invalid Camera index for client)
		camSearchSession.pData->result[cameraIndex].camName[0] = '\0';
		camSearchSession.pData->result[cameraIndex].camIndex = 0;
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   sendHttpReq
 * @return
 */
static BOOL	sendHttpReq(void)
{
	BOOL			retVal = FAIL;
	UINT8			loop;
    CHAR            ipAddrStr[IPV6_ADDR_LEN_MAX];
    CHARPTR         relativeUrl;
	HTTP_HANDLE		httpHandle;
    HTTP_INFO_t 	*httpInfo;
    CHAR            httpUrlStr[DOMAIN_NAME_SIZE_MAX];

    MUTEX_LOCK(resultLock);
    httpInfo = &camSearchSession.pData->httpInfo;
	httpInfo->authMethod = AUTH_TYPE_ANY;
	httpInfo->maxConnTime = MAX_CONN_TIME;
	httpInfo->maxFrameTime = MAX_CONN_TIME;
	httpInfo->httpUsrPwd.password[0] = '\0';
	httpInfo->httpUsrPwd.username[0] = '\0';
	httpInfo->userAgent = CURL_USER_AGENT;
	httpInfo->interface = MAX_HTTP_INTERFACE;

    /* Send Next Request */
    for (loop = camSearchSession.requestCount; loop < camSearchSession.devicesFoundInUpnP; loop++)
    {
        httpInfo->port = DFLT_HTTP_PORT;        

        if (FAIL == GetIpAddrAndPortFromUrl(camSearchSession.pData->location[loop], "http://", ipAddrStr, &httpInfo->port))
        {
            continue;
        }

        snprintf(httpInfo->ipAddress, sizeof(httpInfo->ipAddress), "%s", ipAddrStr);

        /* Make a copy of URL to avoid change in original URL */
        StringNCopy(httpUrlStr, camSearchSession.pData->location[loop], sizeof(httpUrlStr));

        /* Skip "http://" part */
        relativeUrl = &httpUrlStr[strlen("http://")];

        /* Extract relative URL */
        if (FAIL == ParseDelimiterValue(NULL, "/", ipAddrStr, IPV6_ADDR_LEN_MAX, &relativeUrl))
        {
            continue;
        }

        snprintf(httpInfo->relativeUrl, MAX_RELATIVE_URL_WIDTH, "%s%s", "/", relativeUrl);
        camSearchSession.requestCount = loop;
        camSearchSession.httpReqStatus = FAIL;

        if (StartHttp(GET_REQUEST, &camSearchSession.pData->httpInfo, httpCamInfoCb, 0, &httpHandle) == SUCCESS)
        {
            retVal = SUCCESS;
            break;
        }
	}
    MUTEX_UNLOCK(resultLock);

	return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   onvifCamSearchCb
 * @param   data
 * @return
 */
static BOOL onvifCamSearchCb(VOIDPTR data)
{
	BOOL						retVal = FALSE;
	BOOL						deviceAlreadyFound = FALSE;
	UINT8 						loop;
    ONVIF_CAM_SEARCH_CB_PARAM_t	*param = (ONVIF_CAM_SEARCH_CB_PARAM_t *)data;

    MUTEX_LOCK(camSearchSession.reqStatusLock);
	if (camSearchSession.requestStatus == INTERRUPTED)
	{
		retVal = TRUE;
	}
    MUTEX_UNLOCK(camSearchSession.reqStatusLock);

	if (param->respCode == ONVIF_CAM_SEARCH_RESP_SUCCESS)
	{
        if ((param->ipv4Addr[0] == '\0') && (param->ipv6Addr[IPV6_ADDR_GLOBAL][0] == '\0') && (param->ipv6Addr[IPV6_ADDR_LINKLOCAL][0] == '\0'))
		{
            return retVal;
        }

        /* Check if maximum device search reached */
        MUTEX_LOCK(resultLock);
        if (camSearchSession.totalDevices >= MAX_CAM_SEARCH_IN_ONE_SHOT)
        {
            MUTEX_UNLOCK(resultLock);
            EPRINT(CAMERA_INTERFACE, "max camera search limit reached!!");
            return TRUE;
        }

        /* Check if device has already been found */
        for (loop = 0; loop < camSearchSession.totalDevices; loop++)
        {
            /* following change has been done to search virtual cameras on single IP with different ONVIF port */
            if (IsCameraAlreadyFound(param->ipv4Addr, camSearchSession.pData->result[loop].ipv4Addr,
                                     param->ipv6Addr, camSearchSession.pData->result[loop].ipv6Addr[IPV6_ADDR_GLOBAL],
                                     camSearchSession.pData->result[loop].ipv6Addr[IPV6_ADDR_LINKLOCAL])
                    && (param->onvifPort == camSearchSession.pData->result[loop].onvifPort))
            {
                deviceAlreadyFound = TRUE;

                /* Update ipv4 & ipv6 address */
                snprintf(camSearchSession.pData->result[loop].ipv4Addr,
                         sizeof(camSearchSession.pData->result[loop].ipv4Addr) ,"%s", param->ipv4Addr);
                snprintf(camSearchSession.pData->result[loop].ipv6Addr[IPV6_ADDR_GLOBAL],
                         IPV6_ADDR_LEN_MAX, "%s", param->ipv6Addr[IPV6_ADDR_GLOBAL]);
                snprintf(camSearchSession.pData->result[loop].ipv6Addr[IPV6_ADDR_LINKLOCAL],
                         IPV6_ADDR_LEN_MAX, "%s", param->ipv6Addr[IPV6_ADDR_LINKLOCAL]);
                camSearchSession.pData->result[loop].onvifSupport = TRUE;
                camSearchSession.pData->result[loop].updationOnNewSearch = TRUE;
                break;
            }
        }

        if (deviceAlreadyFound == FALSE)
        {
            snprintf(camSearchSession.pData->result[camSearchSession.totalDevices].ipv4Addr,
                    sizeof(camSearchSession.pData->result[camSearchSession.totalDevices].ipv4Addr) ,"%s", param->ipv4Addr);
            snprintf(camSearchSession.pData->result[camSearchSession.totalDevices].ipv6Addr[IPV6_ADDR_GLOBAL],
                    IPV6_ADDR_LEN_MAX ,"%s", param->ipv6Addr[IPV6_ADDR_GLOBAL]);
            snprintf(camSearchSession.pData->result[camSearchSession.totalDevices].ipv6Addr[IPV6_ADDR_LINKLOCAL],
                    IPV6_ADDR_LEN_MAX ,"%s", param->ipv6Addr[IPV6_ADDR_LINKLOCAL]);
            camSearchSession.pData->result[camSearchSession.totalDevices].httpPort = param->httpPort;
            camSearchSession.pData->result[camSearchSession.totalDevices].onvifPort = param->onvifPort;
            camSearchSession.pData->result[camSearchSession.totalDevices].onvifSupport = TRUE;
            camSearchSession.pData->result[camSearchSession.totalDevices].brand[0] = '\0';
            camSearchSession.pData->result[camSearchSession.totalDevices].model[0] = '\0';
            camSearchSession.pData->result[camSearchSession.totalDevices].updationOnNewSearch = TRUE;
            camSearchSession.totalDevices++;
        }

        /* In search process repeat, we are adding new camera to search result, but if it is old one,
         * we need to check whether it is added or not every time, because configuration may change in between. */
        checkIfCamAdded(loop);
        MUTEX_UNLOCK(resultLock);
	}
	else
	{
		if (retVal == FALSE)
		{
            if (sendHttpReq() == FAIL)
			{
				retVal = TRUE;
			}
		}

		if (retVal == TRUE)
		{
			endCameraSearchProcess();
		}
	}

	return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is callback to HTTP Module. It parses XML response, extract brand, model, http port.
 *          Sends Next HTTP Request at closing of other one request.
 * @param   httpHandle
 * @param   dataInfo
 */
static void httpCamInfoCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t * dataInfo)
{
    CHARPTR             startPtr;
    BOOL                userHasAbortedSearch = FALSE;
    BOOL                deviceAlreadyFound = FALSE;
    BOOL                freeSession = FALSE;
    UINT8               devIndex;
    UINT16              httpPort = DFLT_HTTP_PORT;
    struct in6_addr     ipv6Addr;

	switch (dataInfo->httpResponse)
	{
        case HTTP_SUCCESS:
        {
            /*
             * Example Response of Panasonic Camera
             *
                <?xml version="1.0"?>
                <root xmlns="urn:schemas-upnp-org:device-1-0">
                        <specVersion>
                                <major>1</major>
                                <minor>0</minor>
                        </specVersion>
                        <URLBase>http://192.168.102.171:1900</URLBase>
                        <device>
                                <deviceType>urn:schemas-upnp-org:device:Basic:1</deviceType>
                                <friendlyName>Network Camera WV-SC385</friendlyName>
                                <manufacturer>Panasonic</manufacturer>
                                <manufacturerURL>http://www.panasonic.com/</manufacturerURL>
                                <modelDescription>Panasonic Network Camera</modelDescription>
                                <modelName>Network Camera</modelName>
                                <modelNumber>WV-SC385</modelNumber>
                                <modelURL>http://panasonic.com/business/security/products/index.asp</modelURL>
                                <UDN>uuid:4d454930-0000-1000-8000-080023947e66</UDN>
                                <UPC></UPC>
                                <serviceList>
                                        <service>
                                                <serviceType>urn:panasonic-com:service:p01NetCamService:1</serviceType>
                                                <serviceId>urn:panasonic-com:serviceId:p01NetCamService1</serviceId>
                                                <controlURL></controlURL>
                                                <eventSubURL></eventSubURL>
                                                <SCPDURL></SCPDURL>
                                        </service>
                                </serviceList>
                                <presentationURL>http://192.168.102.171:80</presentationURL>
                        </device>
                </root>
             */

            if ((dataInfo->frameSize == 0) || (dataInfo->storagePtr == NULL))
            {
                break;
            }

            /* Check if device has already been found */
            MUTEX_LOCK(resultLock);
            for (devIndex = 0; devIndex < camSearchSession.totalDevices; devIndex++)
            {
                if (IsCameraAlreadyFound(camSearchSession.pData->result[devIndex].ipv4Addr, camSearchSession.pData->httpInfo.ipAddress,
                                         camSearchSession.pData->result[devIndex].ipv6Addr, camSearchSession.pData->httpInfo.ipAddress,
                                         camSearchSession.pData->httpInfo.ipAddress))
                {
                    deviceAlreadyFound = TRUE;
                    break;
                }
            }

            do
            {
                if (devIndex >= MAX_CAM_SEARCH_IN_ONE_SHOT)
                {
                    break;
                }

                /* Validate Device */
                startPtr = dataInfo->storagePtr;
                if (ValidateDevice(&startPtr) == FAIL)
                {
                    WPRINT(CAMERA_INTERFACE, "upnp device validation fail: [ip=%s]", camSearchSession.pData->httpInfo.ipAddress);
                    break;
                }

                /*  Task to Do */
                //	Parameter		XML Tag
                // 1) Brand Name -	manufacturer
                // 2) Model Name -	modelNumber
                // 3) Http Port	 -  extract From presentationURL

                /* Extract Brand Name */
                if (GetXMLTag(&startPtr, XML_TAG_BRAND, camSearchSession.pData->result[devIndex].brand, MAX_BRAND_NAME_LEN) == FAIL)
                {
                    WPRINT(CAMERA_INTERFACE, "fail to get brand over upnp: [ip=%s]", camSearchSession.pData->httpInfo.ipAddress);
                    break;
                }

                /* Extract Model Name */
                if (GetXMLTag(&startPtr, XML_TAG_MODEL, camSearchSession.pData->result[devIndex].model, MAX_MODEL_NAME_LEN) == FAIL)
                {
                    WPRINT(CAMERA_INTERFACE, "fail to get model over upnp: [ip=%s]", camSearchSession.pData->httpInfo.ipAddress);
                    break;
                }
				
                /* Get updated model name if camera brand is MATRIX */
                if (strcmp(camSearchSession.pData->result[devIndex].brand, MATRIX_BRAND_NAME) == STATUS_OK)
                {
                    GetUpdatedMatrixCameraModelName(camSearchSession.pData->result[devIndex].model, MAX_MODEL_NAME_LEN);
                }

                /* Extract http Port from presentationURL */
                if ((startPtr = strstr(startPtr, camSearchSession.pData->httpInfo.ipAddress)) != NULL)
                {
                    startPtr += strlen(camSearchSession.pData->httpInfo.ipAddress);

                    /* Adjust pointer to account for the offset of the closing bracket (']') if the address type is IPv6 */
                    if (NM_IPADDR_FAMILY_V6 == NMIpUtil_GetIpAddrFamily(camSearchSession.pData->httpInfo.ipAddress))
                    {
                        startPtr++;
                    }

                    /* Check if Port Field Exists */
                    if (startPtr[0] == ':')
                    {
                        startPtr++;
                        if ((sscanf(startPtr, "%hd", &httpPort) != 1) || (httpPort == 0))
                        {
                            httpPort = DFLT_HTTP_PORT;
                        }
                    }
                }

                camSearchSession.pData->result[devIndex].httpPort = httpPort;

                if (deviceAlreadyFound == FALSE)
                {
                    /* Fill Ip Address */
                    if (NM_IPADDR_FAMILY_V4 == NMIpUtil_GetIpAddrFamily(camSearchSession.pData->httpInfo.ipAddress))
                    {
                        snprintf(camSearchSession.pData->result[devIndex].ipv4Addr, sizeof(camSearchSession.pData->result[devIndex].ipv4Addr),
                                 "%s", camSearchSession.pData->httpInfo.ipAddress);
                        camSearchSession.pData->result[devIndex].ipv6Addr[IPV6_ADDR_GLOBAL][0] = '\0';
                        camSearchSession.pData->result[devIndex].ipv6Addr[IPV6_ADDR_LINKLOCAL][0] = '\0';
                    }
                    else
                    {
                        inet_pton(AF_INET6, camSearchSession.pData->httpInfo.ipAddress, &ipv6Addr);

                        if (IN6_IS_ADDR_LINKLOCAL(&ipv6Addr))
                        {
                            snprintf(camSearchSession.pData->result[devIndex].ipv6Addr[IPV6_ADDR_LINKLOCAL], IPV6_ADDR_LEN_MAX,
                                     "%s", camSearchSession.pData->httpInfo.ipAddress);
                            camSearchSession.pData->result[devIndex].ipv6Addr[IPV6_ADDR_GLOBAL][0] = '\0';
                        }
                        else
                        {
                            snprintf(camSearchSession.pData->result[devIndex].ipv6Addr[IPV6_ADDR_GLOBAL], IPV6_ADDR_LEN_MAX,
                                     "%s", camSearchSession.pData->httpInfo.ipAddress);
                            camSearchSession.pData->result[devIndex].ipv6Addr[IPV6_ADDR_LINKLOCAL][0] = '\0';
                        }

                        camSearchSession.pData->result[devIndex].ipv4Addr[0] = '\0';
                    }

                    /* Increment total Devices Found */
                    camSearchSession.totalDevices++;
                    camSearchSession.pData->result[devIndex].onvifPort = DFLT_ONVIF_PORT;
                    camSearchSession.pData->result[devIndex].onvifSupport = FALSE;
                }

                camSearchSession.pData->result[devIndex].updationOnNewSearch = TRUE;
                checkIfCamAdded(devIndex);

            }while(0);

            camSearchSession.httpReqStatus = SUCCESS;
            MUTEX_UNLOCK(resultLock);
        }
        break;

        case HTTP_CLOSE_ON_SUCCESS:
        case HTTP_CLOSE_ON_ERROR:
        {
            if (camSearchSession.httpReqStatus == FAIL)
            {
                MUTEX_LOCK(resultLock);
                /* Check if device has already been found */
                for (devIndex = 0; devIndex < camSearchSession.totalDevices; devIndex++)
                {
                    if (IsCameraAlreadyFound(camSearchSession.pData->result[devIndex].ipv4Addr, camSearchSession.pData->httpInfo.ipAddress,
                                             camSearchSession.pData->result[devIndex].ipv6Addr, camSearchSession.pData->httpInfo.ipAddress,
                                             camSearchSession.pData->httpInfo.ipAddress))
                    {
                        deviceAlreadyFound = TRUE;
                        camSearchSession.pData->result[devIndex].updationOnNewSearch = TRUE;
                        break;
                    }
                }

                if ((devIndex < MAX_CAM_SEARCH_IN_ONE_SHOT) && (deviceAlreadyFound == FALSE))
                {
                    /* Fill Ip Address */
                    if (NM_IPADDR_FAMILY_V4 == NMIpUtil_GetIpAddrFamily(camSearchSession.pData->httpInfo.ipAddress))
                    {
                        snprintf(camSearchSession.pData->result[devIndex].ipv4Addr, sizeof(camSearchSession.pData->result[devIndex].ipv4Addr),
                                 "%s", camSearchSession.pData->httpInfo.ipAddress);
                        camSearchSession.pData->result[devIndex].ipv6Addr[IPV6_ADDR_GLOBAL][0] = '\0';
                        camSearchSession.pData->result[devIndex].ipv6Addr[IPV6_ADDR_LINKLOCAL][0] = '\0';
                    }
                    else
                    {
                        inet_pton(AF_INET6, camSearchSession.pData->httpInfo.ipAddress, &ipv6Addr);

                        if (IN6_IS_ADDR_LINKLOCAL(&ipv6Addr))
                        {
                            snprintf(camSearchSession.pData->result[devIndex].ipv6Addr[IPV6_ADDR_LINKLOCAL],
                                     IPV6_ADDR_LEN_MAX, "%s", camSearchSession.pData->httpInfo.ipAddress);
                            camSearchSession.pData->result[devIndex].ipv6Addr[IPV6_ADDR_GLOBAL][0] = '\0';
                        }
                        else
                        {
                            snprintf(camSearchSession.pData->result[devIndex].ipv6Addr[IPV6_ADDR_GLOBAL],
                                     IPV6_ADDR_LEN_MAX, "%s", camSearchSession.pData->httpInfo.ipAddress);
                            camSearchSession.pData->result[devIndex].ipv6Addr[IPV6_ADDR_LINKLOCAL][0] = '\0';
                        }

                        camSearchSession.pData->result[devIndex].ipv4Addr[0] = '\0';
                    }

                    camSearchSession.pData->result[devIndex].brand[0] = '\0';
                    camSearchSession.pData->result[devIndex].model[0] = '\0';
                    camSearchSession.pData->result[devIndex].httpPort = DFLT_HTTP_PORT;
                    camSearchSession.pData->result[devIndex].onvifPort = DFLT_ONVIF_PORT;
                    camSearchSession.pData->result[devIndex].onvifSupport = FALSE;
                    camSearchSession.pData->result[devIndex].updationOnNewSearch = TRUE;
                    /* Increment total Devices Found */
                    camSearchSession.totalDevices++;
                }

                checkIfCamAdded(devIndex);
                MUTEX_UNLOCK(resultLock);
            }

            // Increment Request Count
            camSearchSession.requestCount++;

            // Check if Any Request is pending to be sent
            if (camSearchSession.requestCount < camSearchSession.devicesFoundInUpnP)
            {
                MUTEX_LOCK(camSearchSession.reqStatusLock);
                if (camSearchSession.requestStatus == INTERRUPTED)
                {
                    userHasAbortedSearch = TRUE;
                }
                MUTEX_UNLOCK(camSearchSession.reqStatusLock);

                if (userHasAbortedSearch == FALSE)
                {
                    // Send Next Request
                    if (sendHttpReq() == FAIL)
                    {
                        freeSession = TRUE;
                    }
                }
                else
                {
                    freeSession = TRUE;
                }
            }
            else
            {
                freeSession = TRUE;
            }

            if (freeSession == TRUE)
            {
                endCameraSearchProcess();
            }
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
 * @brief   This function receives upnp responses sent by various devices & stores url
 * @param   connFd
 * @return  status INACTIVE if max search limit, INTERRUPTED if user has aborted search, SUCCESS otherwise
 */
UINT8 RecvUpnpResponse(INT32 *connFd)
{
    UINT64              prevTimeMs = 0;
    INT32               pollSts;
    UINT8               loop;
    UINT8               ipAddrType;
    INT32               recvBytes = 0;
    CHARPTR             startStr;
    CHARPTR             endStr;
    CHAR                tmpLocation[MAX_LOCATION_NAME_LEN];
    BOOL                deviceAlreadyFound = FALSE;
    BOOL                breakLoop = FALSE;
    BOOL                userHasAbortedSearch = FALSE;
    struct pollfd       pollFds[IP_ADDR_TYPE_MAX];

    for (ipAddrType = IP_ADDR_TYPE_IPV4; ipAddrType < IP_ADDR_TYPE_MAX; ipAddrType++)
    {
        pollFds[ipAddrType].fd = connFd[ipAddrType];
        pollFds[ipAddrType].events = POLLRDNORM;
    }

    do
    {
        /* Poll available socket's fd */
        pollSts = poll(pollFds, MAX_LAN_PORT, GetRemainingPollTime(&prevTimeMs, MAX_RECEIVE_MESSAGE_TIME_OUT_MS));

        if (pollSts == -1)
        {
            /* Poll failed */
            EPRINT(CAMERA_INTERFACE, "poll failed: [err=%s]", STR_ERR);
            break;
        }
        else if (pollSts == 0)
        {
            /* Poll timeout */
            WPRINT(CAMERA_INTERFACE, "multicast msg resp receive timeout");
            break;
        }

        for (ipAddrType = IP_ADDR_TYPE_IPV4; ipAddrType < IP_ADDR_TYPE_MAX; ipAddrType++)
        {
            /* Nothing to do if socket is invalid */
            if (pollFds[ipAddrType].fd == INVALID_CONNECTION)
            {
                continue;
            }

            /* If no events */
            if (pollFds[ipAddrType].revents == 0)
            {
                continue;
            }

            /* If other than read event */
            if ((pollFds[ipAddrType].revents & POLLRDNORM) != POLLRDNORM)
            {
                EPRINT(CAMERA_INTERFACE, "invalid poll event:[0x%x]", pollFds[ipAddrType].revents);
                continue;
            }

            /* Receive multicast message sent by different ip camera */
            /* PARASOFT rule BD-TRS-MLOCK marked false positive */
            recvBytes = recv(pollFds[ipAddrType].fd, camSearchSession.pData->msgBuffer, MAX_BUFFER_SIZE, MSG_NOSIGNAL);
            if (recvBytes <= 0)
            {
                EPRINT(CAMERA_INTERFACE, "fail to receive msg resp: [err=%s]", STR_ERR);
                continue;
            }

            /* Terminate receive message with NULL */
            camSearchSession.pData->msgBuffer[recvBytes] = '\0';

            /* Parse Reponse to Get Location Field */
            startStr = strcasestr(camSearchSession.pData->msgBuffer, LOCATION_FIELD);
            if (startStr == NULL)
            {
                continue;
            }

            startStr = startStr + strlen(LOCATION_FIELD);

            /* Skip White Spaces */
            while (startStr[0] == ' ')
            {
                startStr++;
            }

            if ((endStr = strchr(startStr, '\r')) != NULL)
            {
                if (((endStr - startStr) > 0) && ((endStr - startStr) < MAX_LOCATION_NAME_LEN))
                {
                    strncpy(tmpLocation, startStr, (endStr - startStr));
                    tmpLocation[endStr - startStr] = '\0';
                    deviceAlreadyFound = FALSE;

                    /* Check if device has already been found */
                    for (loop = 0; loop < camSearchSession.devicesFoundInUpnP; loop++)
                    {
                        if (strcmp(tmpLocation, camSearchSession.pData->location[loop]) == STATUS_OK)
                        {
                            deviceAlreadyFound = TRUE;
                            break;
                        }
                    }

                    if (deviceAlreadyFound == FALSE)
                    {
                        strncpy(camSearchSession.pData->location[camSearchSession.devicesFoundInUpnP], startStr, (endStr - startStr));
                        camSearchSession.pData->location[camSearchSession.devicesFoundInUpnP][endStr - startStr] = '\0';
                        camSearchSession.devicesFoundInUpnP++;
                    }
                }
            }

            if (camSearchSession.devicesFoundInUpnP >= MAX_CAM_SEARCH_IN_ONE_SHOT)
            {
                breakLoop = TRUE;
            }
            else
            {
                MUTEX_LOCK(camSearchSession.reqStatusLock);
                if (camSearchSession.requestStatus == INTERRUPTED)
                {
                    breakLoop = TRUE;
                    userHasAbortedSearch = TRUE;
                }
                MUTEX_UNLOCK(camSearchSession.reqStatusLock);
            }
            /* This loop would be breaked after 3 second */
        }
    }
    while (breakLoop == FALSE);

    if (userHasAbortedSearch == TRUE)
    {
        return INTERRUPTED;
    }
    else
    {
        return ((breakLoop == FALSE) ? ACTIVE : INACTIVE);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function validates UPnP device to filter IP Camera from Other UPnP device.
 * @param   source
 * @return
 */
BOOL ValidateDevice(CHARPTR *source)
{
	UINT8		loop = 0;
	CHARPTR		tmpPtr;
	CHAR		deviceType[MAX_DEVICE_TYPE_LEN];

    if (GetXMLTag(source, XML_TAG_DEVICE_TYPE, deviceType, MAX_DEVICE_TYPE_LEN) == FAIL)
	{
        return FAIL;
    }

    tmpPtr = strstr(deviceType, UPNP_DEFLT_STR);
    if(tmpPtr == NULL)
    {
        return FAIL;
    }

    tmpPtr += strlen(UPNP_DEFLT_STR);
    while(validUPnPDeviceType[loop] != NULL)
    {
        if (strncasecmp(tmpPtr, validUPnPDeviceType[loop], strlen(validUPnPDeviceType[loop])) == STATUS_OK)
        {
            return SUCCESS;
        }

        loop++;
    }

    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function gives value of requested tag from given source XML data.
 * @param   source
 * @param   tag
 * @param   dest
 * @param   maxSize
 * @return
 */
BOOL GetXMLTag(CHARPTR *source, CHARPTR tag, CHARPTR dest, UINT16 maxSize)
{
	CHARPTR	startPtr;
	CHARPTR	endPtr;

	// Find Tag in Source
    startPtr = strstr(*source, tag);
    if (startPtr == NULL)
	{
        return FAIL;
    }

    // Check if It Valid Tag Value Start
    startPtr += strlen(tag);
    if (startPtr[0] != '>')
    {
        return FAIL;
    }

    // Find End of Tag
    startPtr++;
    endPtr = strchr(startPtr, '<');
    if (endPtr == NULL)
    {
        return FAIL;
    }

    // Copy Tag
    if ((endPtr - startPtr) < maxSize)
    {
        maxSize = (endPtr - startPtr);
    }
    else
    {
        maxSize--;
    }

    strncpy(dest, startPtr, maxSize);
    dest[maxSize] = '\0';
    *source = (endPtr + strlen(tag) + 1);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   IsCamSearchActiveForClient
 * @param   sessionIndex
 * @return
 */
BOOL IsCamSearchActiveForClient(UINT8 sessionIndex)
{
    MUTEX_LOCK(sessionActiveFlagLock);
    BOOL status = sessionActiveF[sessionIndex];
    MUTEX_UNLOCK(sessionActiveFlagLock);
	return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Verify if same camera has already been discovered
 * @param   ipv4AddrCamList         : ipv4 address from discovered camera list
 * @param   ipv4Addr                : ipv4 address of new camera found
 * @param   ipv6AddrCamList         : ipv6 addresses from discovered camera list
 * @param   globalIpv6Addr          : global ipv6 address of new camera found
 * @param   linkLocalIpv6Addr       : link-local ipv6 address of new camera found
 * @return  status
 */
BOOL IsCameraAlreadyFound(const CHAR *ipv4AddrCamList, const CHAR *ipv4Addr,
                          CHAR ipv6AddrCamList[MAX_IPV6_ADDR_TYPE][IPV6_ADDR_LEN_MAX], const CHAR *globalIpv6Addr, const CHAR *linkLocalIpv6Addr)
{
    if ((ipv4AddrCamList[0] != '\0') && (ipv4Addr[0] != '\0') && (strcmp(ipv4AddrCamList, ipv4Addr) == STATUS_OK))
    {
        return TRUE;
    }

    /* PARASOFT : Rule MISRAC2012-RULE_21_17-a marked false positive */
    if ((ipv6AddrCamList[IPV6_ADDR_GLOBAL][0] != '\0') && (globalIpv6Addr[0] != '\0')
        && (strcmp(ipv6AddrCamList[IPV6_ADDR_GLOBAL], globalIpv6Addr) == STATUS_OK))
    {
        return TRUE;
    }

    if ((ipv6AddrCamList[IPV6_ADDR_LINKLOCAL][0] != '\0') && (linkLocalIpv6Addr[0] != '\0')
        && (strcmp(ipv6AddrCamList[IPV6_ADDR_LINKLOCAL], linkLocalIpv6Addr) == STATUS_OK))
    {
        return TRUE;
    }

    return FALSE;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
