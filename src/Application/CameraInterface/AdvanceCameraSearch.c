//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		AdvanceCameraSearch.c
@brief      This File Provides to Search Camera using UPnP Protocol 1.2 unicast request and http
            get device information request.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "DebugLog.h"
#include "MxOnvifClient.h"
#include "NetworkCommand.h"
#include "CameraSearch.h"
#include "AdvanceCameraSearch.h"
#include "CameraInterface.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define MAX_DEVINFO_REQ_IN_ONE_SHOT		30
#define MAX_ADV_SERCH_UPNP_TIMEOUT_MS	(300)       // in milli sec
#define MAX_ADV_SERCH_ONVIF_TIMEOUT_MS  (500)       // in milli sec
#define MAX_DEVICE_INFO_URL				5
#define MAX_UPNP_UNICAST_REQ_SIZE		500
#define MAX_ONVIF_UNICAST_REQ_SIZE		1000

#define MAX_ADV_SEARCH_SESSION			MAX_NW_CLIENT
#define INVALID_ADV_SEARCH_SESSION		(255)
#define ADVANCE_SRCH_THREAD_STACK_SZ    (3* MEGA_BYTE)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
	ADV_CAM_UNAUTHRIZE = 0,
	ADV_CAM_HOST_UNAVAILABLE,
	ADV_CAM_COMM_ERROR,
	ADV_CAM_SERACH_SUCESS,
	MAX_ADV_CAM_HOST_RESPONSE
}ADV_CAM_HOST_RESPONSE;

typedef struct
{
    UINT8                       sessionId;
    BOOL                        requestStatus;
    UINT8                       totalDevices;
    UINT8                       devicesFoundInUpnP;
    UINT8                       requestCount;
    BOOL                        httpReqStatus;
    DYNAMIC_DATA_t              *pData;
    pthread_mutex_t             requestStatusMutex;
	ADV_IP_CAM_SEARCH_INPARAM_t inputParam;
    UINT8                       startIpAddressAllOctect[4];
    UINT8                       endIpAddressLastOctect;
    URL_REQUEST_t               url[MAX_DEVICE_INFO_URL];
    UINT8                       numOfRequest;
    UINT8                       reqNoToServe;	// current request no which is server in given range
    CAMERA_BRAND_e              brandNum;		// brand num
    UINT8                       devInfoReqSent; // This is like counter of device info req sent
    HTTP_HANDLE                 httpHande;

    UINT8                       failDevCount;
    UINT8                       failIpList[255];
    UINT8                       failIpSatus[255];
    pthread_mutex_t             failDevicesLock;
    pthread_mutex_t             searchResultLock;

}ADV_IP_CAM_SEARCH_SESSION_PARAM_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static ADV_IP_CAM_SEARCH_SESSION_PARAM_t advCamSearchSession[MAX_ADV_SEARCH_SESSION];

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void *advanceCamSearchThread(void *pThreadData);
//-------------------------------------------------------------------------------------------------
static BOOL checkCameraOnvifSupport(CHARPTR camIpAddress, UINT8 sessionIndex, UINT16PTR onvifPort);
//-------------------------------------------------------------------------------------------------
static void replyEndAdvanceCamSearchResult(UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static void checkIfCamAdded(UINT8 cameraIndex, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL	sendAdvSearchUpnpLocHTTPReq(UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL	sendDevInfoHTTPReq(UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static void httpAdvCamSearchUpnpLocationCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void httpAdvCamSearchDevInfoCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t * dataInfo);
//-------------------------------------------------------------------------------------------------
static ADV_CAM_HOST_RESPONSE processOnvifUnicastReq(INT32 connFd, CHARPTR ipAddress, CHARPTR msgPtr,
                                                    UINT16 messageLength, UINT8 sessionIndex, UINT16PTR onvifPort);
//-------------------------------------------------------------------------------------------------
static ADV_CAM_HOST_RESPONSE processUpnpUnicastReq(INT32 connFd, CHARPTR ipAddress, CHARPTR msgPtr, UINT16 messageLength, UINT8	sessionIndex);
//-------------------------------------------------------------------------------------------------
static void getOnvifIpAddrPortFromEntryPoint(CHARPTR entryPoint, UINT8 sessionIndex, UINT16PTR onvifPort);
//-------------------------------------------------------------------------------------------------
static void insertFailDevEntry(UINT8 lastOctect, ADV_CAM_HOST_RESPONSE hostResponse, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static void removeFailDevEntry(UINT8 lastOctect, ADV_CAM_HOST_RESPONSE hostResponse, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static void deleteNotUpdatedEntryFromList(UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This api init the advance camera search parameter of all session.
 */
void InitAdvCameraSearch(void)
{
    UINT8 index = 0;

	for(index = 0; index < MAX_ADV_SEARCH_SESSION; index++)
	{
        MUTEX_INIT(advCamSearchSession[index].requestStatusMutex, NULL);
        MUTEX_INIT(advCamSearchSession[index].failDevicesLock, NULL);
        MUTEX_INIT(advCamSearchSession[index].searchResultLock, NULL);
		advCamSearchSession[index].requestStatus = INACTIVE;
		advCamSearchSession[index].sessionId = INVALID_ADV_SEARCH_SESSION;
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API Start Camera Search If Session is INACTIVE, otherwise it will return CMD_REQUEST_IN_PROGRESS.
 * @param   nwSessionIndex
 * @param   inParam
 * @return  Network command status
 */
NET_CMD_STATUS_e StartAdvanceCameraSearch(UINT8 nwSessionIndex, ADV_IP_CAM_SEARCH_INPARAM_t *inParam)
{
    if (nwSessionIndex >= MAX_ADV_SEARCH_SESSION)
	{
        return CMD_PROCESS_ERROR;
    }

    MUTEX_LOCK(advCamSearchSession[nwSessionIndex].requestStatusMutex);
    if (advCamSearchSession[nwSessionIndex].requestStatus != INACTIVE)
    {
        MUTEX_UNLOCK(advCamSearchSession[nwSessionIndex].requestStatusMutex);
        return CMD_REQUEST_IN_PROGRESS;
    }

    advCamSearchSession[nwSessionIndex].sessionId = nwSessionIndex;
    MUTEX_UNLOCK(advCamSearchSession[nwSessionIndex].requestStatusMutex);

    memset(advCamSearchSession[nwSessionIndex].url, '\0', (MAX_DEVICE_INFO_URL * sizeof(URL_REQUEST_t)));
    advCamSearchSession[nwSessionIndex].inputParam =  *inParam;

    ParseIpAddressGetIntValue(advCamSearchSession[nwSessionIndex].inputParam.endRangeIpAddr, advCamSearchSession[nwSessionIndex].startIpAddressAllOctect);
    advCamSearchSession[nwSessionIndex].endIpAddressLastOctect = advCamSearchSession[nwSessionIndex].startIpAddressAllOctect[3];
    ParseIpAddressGetIntValue(advCamSearchSession[nwSessionIndex].inputParam.startRangeIpAddr, advCamSearchSession[nwSessionIndex].startIpAddressAllOctect);

    /* Create the detached thread to start advance camera search process */
    if (FALSE == Utils_CreateThread(NULL, advanceCamSearchThread, &advCamSearchSession[nwSessionIndex].sessionId, DETACHED_THREAD, ADVANCE_SRCH_THREAD_STACK_SZ))
    {
        MUTEX_LOCK(advCamSearchSession[nwSessionIndex].requestStatusMutex);
        advCamSearchSession[nwSessionIndex].requestStatus = INACTIVE;
        advCamSearchSession[nwSessionIndex].sessionId = INVALID_ADV_SEARCH_SESSION;
        MUTEX_UNLOCK(advCamSearchSession[nwSessionIndex].requestStatusMutex);
        return CMD_RESOURCE_LIMIT;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief StopAdvanceCameraSearch
 * @param sessionIndex
 */
void StopAdvanceCameraSearch(UINT8 sessionIndex)
{
    MUTEX_LOCK(advCamSearchSession[sessionIndex].requestStatusMutex);
	if (advCamSearchSession[sessionIndex].requestStatus == ACTIVE)
	{
		advCamSearchSession[sessionIndex].requestStatus = INTERRUPTED;
	}
    MUTEX_UNLOCK(advCamSearchSession[sessionIndex].requestStatusMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   GetAcqListOfAdvSearchCameras
 * @param   sessionIndex
 * @param   result
 * @param   numOfResult
 * @return  Network command status
 */
NET_CMD_STATUS_e GetAcqListOfAdvSearchCameras(UINT8 sessionIndex, IP_CAM_SEARCH_RESULT_t *result, UINT8PTR numOfResult)
{
    UINT8 index = 0;

    MUTEX_LOCK(advCamSearchSession[sessionIndex].searchResultLock);
    if(advCamSearchSession[sessionIndex].totalDevices == 0)
	{
        MUTEX_UNLOCK(advCamSearchSession[sessionIndex].searchResultLock);
        return CMD_PROCESS_ERROR;
    }

    for (index = 0; index < advCamSearchSession[sessionIndex].totalDevices; index++)
    {
        *(result + index) = advCamSearchSession[sessionIndex].pData->result[index];
    }

    *numOfResult = advCamSearchSession[sessionIndex].totalDevices;
    MUTEX_UNLOCK(advCamSearchSession[sessionIndex].searchResultLock);
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   checkCameraOnvifSupport
 * @param   camIpAddress
 * @param   sessionIndex
 * @param   onvifPort
 * @return  TRUE/FALSE
 */
static BOOL checkCameraOnvifSupport(CHARPTR camIpAddress, UINT8 sessionIndex, UINT16PTR onvifPort)
{
    BOOL    onvifSupport = TRUE;
    INT32   reuse = 1;
    INT32   connFd;
    CHAR    uuidNum[MAX_UUID_LEN];
    CHAR    onvifUnicastMsg[MAX_ONVIF_UNICAST_REQ_SIZE];
    INT16   messageLenOnvif;

    GenerateUuidStr(uuidNum);
    messageLenOnvif = snprintf(onvifUnicastMsg, MAX_ONVIF_UNICAST_REQ_SIZE, DEVICE_DISCOVERY_MSG, uuidNum);

	 // create socket to multicast message
    connFd = socket(AF_INET, UDP_SOCK_OPTIONS, 0);
	if (connFd == INVALID_CONNECTION)
	{
        EPRINT(CAMERA_INTERFACE, "failed to create socket: [err=%s]", STR_ERR);
        return FALSE;
	}

    if (setsockopt(connFd, SOL_SOCKET, SO_REUSEADDR, (CHARPTR)&reuse, sizeof(reuse)) != STATUS_OK)
    {
        EPRINT(CAMERA_INTERFACE, "failed to set reuse addr: [err=%s]", STR_ERR);
        onvifSupport = FALSE;
    }
    else if (processOnvifUnicastReq(connFd, camIpAddress, onvifUnicastMsg, messageLenOnvif, sessionIndex, onvifPort) != ADV_CAM_SERACH_SUCESS)
    {
        onvifSupport = FALSE;
    }

    CloseSocket(&connFd);
	return onvifSupport;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   advanceCamSearchThread
 * @param   pThreadData
 * @return  None
 */
static void *advanceCamSearchThread(void *pThreadData)
{
    UINT8					sessionIndex = *(UINT8PTR)pThreadData;
	INT32 	 				connFd = INVALID_CONNECTION;
    CHAR  					uuidNum[MAX_UUID_LEN];
	INT32 					reuse = 1;
	BOOL					freeSession = FALSE;
	CHAR					upnpUnicastMsg[MAX_UPNP_UNICAST_REQ_SIZE];
	CHAR					onvifUnicastMsg[MAX_ONVIF_UNICAST_REQ_SIZE];
	INT16 					messageLenOnvif = 0, messageLenUpnp = 0;
	UINT8					seachCamNoLoop = 0, ipRangeLoop = 0;
    CHAR					cameraIpAddr[IPV6_ADDR_LEN_MAX];
	BOOL					hasUserAborted = FALSE;
	UINT16					onvifPort = DFLT_ONVIF_PORT;
	ADV_CAM_HOST_RESPONSE	upnpHostResponse, onvifHostResponse;
	UINT8  					devIndex = 0;

    THREAD_START("ADV_CAM_SEARCH");

    MUTEX_LOCK(advCamSearchSession[sessionIndex].requestStatusMutex);
	if(advCamSearchSession[sessionIndex].requestStatus == INACTIVE)
	{
        MUTEX_UNLOCK(advCamSearchSession[sessionIndex].requestStatusMutex);
		advCamSearchSession[sessionIndex].totalDevices = 0;
		advCamSearchSession[sessionIndex].pData = malloc(sizeof(DYNAMIC_DATA_t));
        if (advCamSearchSession[sessionIndex].pData != NULL)
        {
            memset(advCamSearchSession[sessionIndex].pData, 0, sizeof(DYNAMIC_DATA_t));
        }
		advCamSearchSession[sessionIndex].requestStatus = ACTIVE;
        MUTEX_LOCK(advCamSearchSession[sessionIndex].failDevicesLock);
		advCamSearchSession[sessionIndex].failDevCount = 0;
        MUTEX_UNLOCK(advCamSearchSession[sessionIndex].failDevicesLock);
	}
	else
	{
        MUTEX_UNLOCK(advCamSearchSession[sessionIndex].requestStatusMutex);
	}

    advCamSearchSession[sessionIndex].devicesFoundInUpnP = 0;
	advCamSearchSession[sessionIndex].requestCount = 0;
	advCamSearchSession[sessionIndex].devInfoReqSent = 0;
	advCamSearchSession[sessionIndex].reqNoToServe = 0;
	advCamSearchSession[sessionIndex].httpHande = INVALID_HTTP_HANDLE;

	if (advCamSearchSession[sessionIndex].pData != NULL)
	{
        MUTEX_LOCK(advCamSearchSession[sessionIndex].searchResultLock);
		// If any camera is detected as onvif , then its port will be replaced
		for(seachCamNoLoop = 0; seachCamNoLoop < MAX_CAM_SEARCH_IN_ONE_SHOT; seachCamNoLoop++)
		{
			advCamSearchSession[sessionIndex].pData->result[seachCamNoLoop].updationOnNewSearch = FALSE;
		}
        MUTEX_UNLOCK(advCamSearchSession[sessionIndex].searchResultLock);

		// check if brand is any..
        if (GetBrandNum(advCamSearchSession[sessionIndex].inputParam.brand, &advCamSearchSession[sessionIndex].brandNum) == FAIL)
		{
            EPRINT(CAMERA_INTERFACE, "failed to get brand number: [sessionIndex=%d]", sessionIndex);
			advCamSearchSession[sessionIndex].brandNum = MAX_CAMERA_BRAND;
		}

		if(advCamSearchSession[sessionIndex].brandNum == CAMERA_BRAND_GENERIC)
		{
            GenerateUuidStr(uuidNum);
            messageLenOnvif = snprintf(onvifUnicastMsg, MAX_ONVIF_UNICAST_REQ_SIZE, DEVICE_DISCOVERY_MSG, uuidNum);
            messageLenUpnp = snprintf(upnpUnicastMsg, MAX_UPNP_UNICAST_REQ_SIZE, SEARCH_MESSAGE_FORMAT, IPV4_MULTICAST_ADDRESS, MULTICAST_PORT);

			// create socket to multicast message
            connFd = socket(AF_INET, UDP_SOCK_OPTIONS, 0);
			if (connFd == INVALID_CONNECTION)
			{
                EPRINT(CAMERA_INTERFACE, "failed to create socket: [err=%s]", STR_ERR);
			}
            else if (setsockopt(connFd, SOL_SOCKET, SO_REUSEADDR, (CHARPTR)&reuse, sizeof(reuse)) != STATUS_OK)
			{
                EPRINT(CAMERA_INTERFACE, "failed to set reuse addr: [err=%s]", STR_ERR);
			}
			else
			{
				do
				{
					// Initialize add route response with fail status
					for(ipRangeLoop = advCamSearchSession[sessionIndex].startIpAddressAllOctect[3];
								ipRangeLoop <= advCamSearchSession[sessionIndex].endIpAddressLastOctect; ipRangeLoop++)
					{
                        if (ipRangeLoop == 0)
						{
							break;
						}

                        snprintf(cameraIpAddr, sizeof(cameraIpAddr), "%d.%d.%d.%d", advCamSearchSession[sessionIndex].startIpAddressAllOctect[0],
                                advCamSearchSession[sessionIndex].startIpAddressAllOctect[1],
                                advCamSearchSession[sessionIndex].startIpAddressAllOctect[2], ipRangeLoop);

                        onvifHostResponse = processOnvifUnicastReq(connFd, cameraIpAddr, onvifUnicastMsg, messageLenOnvif, sessionIndex, &onvifPort);
                        upnpHostResponse = processUpnpUnicastReq(connFd, cameraIpAddr, upnpUnicastMsg, messageLenUpnp, sessionIndex);

						if(onvifHostResponse == ADV_CAM_SERACH_SUCESS)
						{
                            MUTEX_LOCK(advCamSearchSession[sessionIndex].searchResultLock);
							for (devIndex = 0; devIndex < advCamSearchSession[sessionIndex].totalDevices; devIndex++)
							{
                                if (strcmp(advCamSearchSession[sessionIndex].pData->result[devIndex].ipv4Addr, cameraIpAddr) == STATUS_OK)
								{
                                    advCamSearchSession[sessionIndex].pData->result[devIndex].onvifPort = onvifPort;
								    advCamSearchSession[sessionIndex].pData->result[devIndex].onvifSupport = TRUE;
									break;
								}
							}

							advCamSearchSession[sessionIndex].pData->result[devIndex].updationOnNewSearch = TRUE;

							if(devIndex == advCamSearchSession[sessionIndex].totalDevices)
							{
                                snprintf(advCamSearchSession[sessionIndex].pData->result[devIndex].ipv4Addr,
                                         sizeof(advCamSearchSession[sessionIndex].pData->result[devIndex].ipv4Addr), "%s", cameraIpAddr);
                                advCamSearchSession[sessionIndex].pData->result[devIndex].ipv6Addr[IPV6_ADDR_GLOBAL][0] = '\0';
                                advCamSearchSession[sessionIndex].pData->result[devIndex].ipv6Addr[IPV6_ADDR_LINKLOCAL][0] = '\0';
								advCamSearchSession[sessionIndex].pData->result[devIndex].httpPort = DFLT_HTTP_PORT;
								advCamSearchSession[sessionIndex].pData->result[devIndex].onvifPort = onvifPort;
								advCamSearchSession[sessionIndex].pData->result[devIndex].onvifSupport = TRUE;
								advCamSearchSession[sessionIndex].pData->result[devIndex].brand[0] = '\0';
								advCamSearchSession[sessionIndex].pData->result[devIndex].model[0] = '\0';
								advCamSearchSession[sessionIndex].totalDevices++;
							}

                            /* In search process repeat ... we are adding new camera to search result, but if it is old one,
                             * we need to check whether it is added or not every time, because configuration may change in between. */
							checkIfCamAdded(devIndex, sessionIndex);
                            MUTEX_UNLOCK(advCamSearchSession[sessionIndex].searchResultLock);
						}

						if((onvifHostResponse != ADV_CAM_SERACH_SUCESS) && (upnpHostResponse != ADV_CAM_SERACH_SUCESS))
						{
							insertFailDevEntry(ipRangeLoop, ADV_CAM_HOST_UNAVAILABLE, sessionIndex);
						}
						else
						{
							removeFailDevEntry(ipRangeLoop, ADV_CAM_SERACH_SUCESS, sessionIndex);
						}

                        MUTEX_LOCK(advCamSearchSession[sessionIndex].requestStatusMutex);
						if (advCamSearchSession[sessionIndex].requestStatus == INTERRUPTED)
						{
							freeSession = TRUE;
                            MUTEX_UNLOCK(advCamSearchSession[sessionIndex].requestStatusMutex);
							break;
						}
						MUTEX_UNLOCK(advCamSearchSession[sessionIndex].requestStatusMutex);
					}
				}
				while(0);
			}

            CloseSocket(&connFd);
			if((freeSession == FALSE) && (sendAdvSearchUpnpLocHTTPReq(sessionIndex) == FALSE))
			{
				freeSession = TRUE;
			}
		}
		else
		{
            if (FAIL == GetBrandDeviceInfo(advCamSearchSession[sessionIndex].brandNum, CAMERA_MODEL_NONE,
                                           advCamSearchSession[sessionIndex].url, &advCamSearchSession[sessionIndex].numOfRequest))
            {
                EPRINT(CAMERA_INTERFACE, "get device info not supported: [brandNum=%d]", advCamSearchSession[sessionIndex].brandNum);
                freeSession = TRUE;
            }
			else
            {
                DPRINT(CAMERA_INTERFACE, "send request: [brand=%s], [NumOfRequest=%d]",
                       advCamSearchSession[sessionIndex].inputParam.brand, advCamSearchSession[sessionIndex].numOfRequest);
                sendDevInfoHTTPReq(sessionIndex);
                MUTEX_LOCK(advCamSearchSession[sessionIndex].requestStatusMutex);
                if (advCamSearchSession[sessionIndex].requestStatus == INTERRUPTED)
                {
                    hasUserAborted = TRUE;
                }
                MUTEX_UNLOCK(advCamSearchSession[sessionIndex].requestStatusMutex);

                if((hasUserAborted == TRUE) || (advCamSearchSession[sessionIndex].devInfoReqSent == 0))
                {
                    freeSession = TRUE;
                }
			}
		}
	}
	else
	{
        EPRINT(CAMERA_INTERFACE, "invld pointer for advance camera search session: [sessionIndex=%d]", sessionIndex);
		freeSession = TRUE;
	}

	if(freeSession == TRUE)
	{
		replyEndAdvanceCamSearchResult(sessionIndex);
	}

	pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   replyEndAdvanceCamSearchResult
 * @param   sessionIndex
 */
static void replyEndAdvanceCamSearchResult(UINT8 sessionIndex)
{
    if (sessionIndex >= MAX_ADV_SEARCH_SESSION)
    {
        EPRINT(CAMERA_INTERFACE, "invld session found: [sessionIndex=%d]", sessionIndex);
        return;
    }

    MUTEX_LOCK(advCamSearchSession[sessionIndex].requestStatusMutex);
	if(advCamSearchSession[sessionIndex].requestStatus == INTERRUPTED)
	{
		advCamSearchSession[sessionIndex].requestStatus = INACTIVE;
        MUTEX_UNLOCK(advCamSearchSession[sessionIndex].requestStatusMutex);
		advCamSearchSession[sessionIndex].sessionId = INVALID_ADV_SEARCH_SESSION;
        FREE_MEMORY(advCamSearchSession[sessionIndex].pData);
        return;
	}

    MUTEX_UNLOCK(advCamSearchSession[sessionIndex].requestStatusMutex);
    deleteNotUpdatedEntryFromList(sessionIndex);

    /* Create the detached thread to start again advance camera search process */
    if (FALSE == Utils_CreateThread(NULL, advanceCamSearchThread, &advCamSearchSession[sessionIndex].sessionId, DETACHED_THREAD, ADVANCE_SRCH_THREAD_STACK_SZ))
    {
        MUTEX_LOCK(advCamSearchSession[sessionIndex].requestStatusMutex);
        advCamSearchSession[sessionIndex].requestStatus = INACTIVE;
        MUTEX_UNLOCK(advCamSearchSession[sessionIndex].requestStatusMutex);
        advCamSearchSession[sessionIndex].sessionId = INVALID_ADV_SEARCH_SESSION;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   checkIfCamAdded
 * @param   cameraIndex
 * @param   sessionIndex
 */
static void checkIfCamAdded(UINT8 cameraIndex, UINT8 sessionIndex)
{
	INT16				loop;
	CAMERA_CONFIG_t		camCfg[MAX_CAMERA];
	IP_CAMERA_CONFIG_t  ipCamCfg[MAX_CAMERA];
	CAMERA_BRAND_e		brandNum;
	CAMERA_MODEL_e 		modelNum;
    CHAR				camIpAddr[IPV6_ADDR_LEN_MAX];

	ReadCameraConfig(camCfg);
	ReadIpCameraConfig(ipCamCfg);

	advCamSearchSession[sessionIndex].pData->result[cameraIndex].camStatus = CAM_UNIDENTIFIED;

	for (loop = 0; loop < getMaxCameraForCurrentVariant(); loop++)
	{
        if(GetCamIpAddress(loop, camIpAddr) == FALSE)
		{
            snprintf(camIpAddr, sizeof(camIpAddr), "%s", ipCamCfg[loop].cameraAddress);
		}

		// if ip address matches with configuratation we send cam as added
        if (strcmp(camIpAddr, advCamSearchSession[sessionIndex].pData->result[cameraIndex].ipv4Addr) == STATUS_OK)
		{
			advCamSearchSession[sessionIndex].pData->result[cameraIndex].camStatus = CAM_ADDED;
			break;
		}
	}

	if(advCamSearchSession[sessionIndex].pData->result[cameraIndex].camStatus != CAM_ADDED)
	{
		// not added then check brand model if onvif found
        if((GetBrandNum(advCamSearchSession[sessionIndex].pData->result[cameraIndex].brand, &brandNum) == SUCCESS)
			&& (GetModelNum(advCamSearchSession[sessionIndex].pData->result[cameraIndex].brand,
                    advCamSearchSession[sessionIndex].pData->result[cameraIndex].model, &modelNum) == SUCCESS))
		{
			advCamSearchSession[sessionIndex].pData->result[cameraIndex].camStatus = CAM_IDENTIFIED;
		}
	}

	if (advCamSearchSession[sessionIndex].pData->result[cameraIndex].camStatus == CAM_ADDED)
	{
        snprintf(advCamSearchSession[sessionIndex].pData->result[cameraIndex].camName, MAX_CAMERA_NAME_WIDTH, "%s", camCfg[loop].name);
		advCamSearchSession[sessionIndex].pData->result[cameraIndex].camIndex = (loop + 1);
	}
	else
	{
		// When added one it will give 0 (Invalid Camera index for client)
		advCamSearchSession[sessionIndex].pData->result[cameraIndex].camName[0] = '\0';
		advCamSearchSession[sessionIndex].pData->result[cameraIndex].camIndex = 0;
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   deleteNotUpdatedEntryFromList
 * @param   sessionIndex
 */
static void deleteNotUpdatedEntryFromList(UINT8 sessionIndex)
{
    UINT8					index = 0, remainIndex = 0, remainCnt, fillIndex = 0;
	IP_CAM_SEARCH_RESULT_t	tempResult[MAX_CAM_SEARCH_IN_ONE_SHOT];

    if (sessionIndex >= MAX_ADV_SEARCH_SESSION)
    {
        EPRINT(CAMERA_INTERFACE, "invld session found: [sessionIndex=%d]", sessionIndex);
        return;
    }

    MUTEX_LOCK(advCamSearchSession[sessionIndex].searchResultLock);
	do
	{
		if (advCamSearchSession[sessionIndex].pData == NULL)
        {
			break;
        }

		remainCnt = 0;
		for(index = 0; index < advCamSearchSession[sessionIndex].totalDevices; index++)
		{
			if(advCamSearchSession[sessionIndex].pData->result[index].updationOnNewSearch == FALSE)
			{
				break;
			}
		}

        if (index >= advCamSearchSession[sessionIndex].totalDevices)
		{
			break;
        }

        for(remainIndex = (index + 1); remainIndex < advCamSearchSession[sessionIndex].totalDevices; remainIndex++)
        {
            tempResult[remainCnt++] = advCamSearchSession[sessionIndex].pData->result[remainIndex];
        }

        if (advCamSearchSession[sessionIndex].totalDevices)
        {
            advCamSearchSession[sessionIndex].totalDevices--;
        }

        for (fillIndex = index; fillIndex < (index + remainCnt); fillIndex++)
        {
            advCamSearchSession[sessionIndex].pData->result[fillIndex] = tempResult[fillIndex - index];
        }

    } while(1);

    MUTEX_UNLOCK(advCamSearchSession[sessionIndex].searchResultLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   sendAdvSearchUpnpLocHTTPReq
 * @param   sessionIndex
 * @return
 */
static BOOL	sendAdvSearchUpnpLocHTTPReq(UINT8 sessionIndex)
{
	BOOL			retVal = FAIL;
	CHARPTR			startPtr;
	CHARPTR			endPtr;
	UINT8			loop;
	HTTP_HANDLE		httpHandle;
    HTTP_INFO_t 	*httpInfo = &advCamSearchSession[sessionIndex].pData->httpInfo;

	httpInfo->authMethod = AUTH_TYPE_ANY;
	httpInfo->maxConnTime = MAX_CONN_TIME;
	httpInfo->maxFrameTime = MAX_CONN_TIME;
	httpInfo->httpUsrPwd.password[0] = '\0';
	httpInfo->httpUsrPwd.username[0] = '\0';
	httpInfo->userAgent = CURL_USER_AGENT;
	httpInfo->interface = MAX_HTTP_INTERFACE;

	// Send Next Request
    for (loop = advCamSearchSession[sessionIndex].requestCount; loop < advCamSearchSession[sessionIndex].devicesFoundInUpnP; loop++)
	{
		startPtr = advCamSearchSession[sessionIndex].pData->location[loop];
		startPtr += strlen("http://");

        if ((endPtr = strchr(startPtr, ':')) != NULL)
		{
            if ((endPtr - startPtr) < IPV4_ADDR_LEN_MAX)
			{
				strncpy(httpInfo->ipAddress, startPtr, (endPtr - startPtr));
				httpInfo->ipAddress[(endPtr - startPtr)] = '\0';
				startPtr = (endPtr + 1);

				if (sscanf(startPtr, "%hd", &httpInfo->port) == 1)
				{
                    if ((startPtr = strchr(startPtr, '/')) != NULL)
					{
                        snprintf(httpInfo->relativeUrl, MAX_RELATIVE_URL_WIDTH, "%s", startPtr);
                        DPRINT(CAMERA_INTERFACE, "[url=http://%s:%d%s]", httpInfo->ipAddress, httpInfo->port,  httpInfo->relativeUrl);
						advCamSearchSession[sessionIndex].requestCount = loop;
						advCamSearchSession[sessionIndex].httpReqStatus = FAIL;

                        if (StartHttp(GET_REQUEST, &advCamSearchSession[sessionIndex].pData->httpInfo,
                                      httpAdvCamSearchUpnpLocationCb, sessionIndex, &httpHandle) == SUCCESS)
						{
							retVal = SUCCESS;
							break;
						}
					}
				}
			}
		}
        else if ((endPtr = strchr(startPtr, '/')) != NULL)
		{
			if ((endPtr - startPtr) < IPV4_ADDR_LEN_MAX)
			{
				strncpy(httpInfo->ipAddress, startPtr, (endPtr - startPtr));
				httpInfo->ipAddress[(endPtr - startPtr)] = '\0';
				startPtr = endPtr;
                httpInfo->port = DFLT_HTTP_PORT;
                snprintf(httpInfo->relativeUrl, MAX_RELATIVE_URL_WIDTH, "%s", startPtr);
                DPRINT(CAMERA_INTERFACE, "[url=http://%s:%d%s]", httpInfo->ipAddress, httpInfo->port,  httpInfo->relativeUrl);
				advCamSearchSession[sessionIndex].requestCount = loop;
				advCamSearchSession[sessionIndex].httpReqStatus = FAIL;

                if (StartHttp(GET_REQUEST, &advCamSearchSession[sessionIndex].pData->httpInfo,
                              httpAdvCamSearchUpnpLocationCb, sessionIndex, &httpHandle) == SUCCESS)
				{
					retVal = SUCCESS;
					break;
				}
			}
		}
	}

	return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   httpAdvCamSearchUpnpLocationCb
 * @param   httpHandle
 * @param   dataInfo
 */
static void httpAdvCamSearchUpnpLocationCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t * dataInfo)
{
	UINT8	sessionIndex = (UINT8)(dataInfo->userData);
	CHARPTR	startPtr;
	BOOL	userHasAbortedSearch = FALSE;
	BOOL	deviceAlreadyFound = FALSE;
	BOOL	freeSession = FALSE;
	UINT8	devIndex;
	UINT16	httpPort = DFLT_HTTP_PORT;

	switch (dataInfo->httpResponse)
	{
        case HTTP_SUCCESS:
        {
            MUTEX_LOCK(advCamSearchSession[sessionIndex].searchResultLock);
            do
            {
                if ((dataInfo->frameSize == 0) || (dataInfo->storagePtr == NULL))
                {
                    break;
                }

                // Check if device has already been found
                for (devIndex = 0; devIndex < advCamSearchSession[sessionIndex].totalDevices; devIndex++)
                {
                    if (strcmp(advCamSearchSession[sessionIndex].pData->httpInfo.ipAddress,
                            advCamSearchSession[sessionIndex].pData->result[devIndex].ipv4Addr) == STATUS_OK)
                    {
                        deviceAlreadyFound = TRUE;
                        break;
                    }
                }

                if (devIndex >= MAX_CAM_SEARCH_IN_ONE_SHOT)
                {
                    break;
                }

                // Validate Device
                startPtr = dataInfo->storagePtr;
                if (ValidateDevice(&startPtr) == FAIL)
                {
                    WPRINT(CAMERA_INTERFACE, "upnp device validation failed: [ip=%s]", advCamSearchSession[sessionIndex].pData->httpInfo.ipAddress);
                    break;
                }

                // Task to Do
                //	Parameter		XML Tag
                // 1) Brand Name -	manufacturer
                // 2) Model Name -	modelNumber
                // 3) Http Port	 -  extract From presentationURL

                // Extract Brand Name
                if (GetXMLTag(&startPtr, XML_TAG_BRAND, advCamSearchSession[sessionIndex].pData->result[devIndex].brand, MAX_BRAND_NAME_LEN) == FAIL)
                {
                    WPRINT(CAMERA_INTERFACE, "upnp brand fail: [ip=%s]", advCamSearchSession[sessionIndex].pData->httpInfo.ipAddress);
                    break;
                }

                // Extract Model Name
                if (GetXMLTag(&startPtr, XML_TAG_MODEL, advCamSearchSession[sessionIndex].pData->result[devIndex].model, MAX_MODEL_NAME_LEN) == FAIL)
                {
                    WPRINT(CAMERA_INTERFACE, "upnp model fail: [ip=%s]", advCamSearchSession[sessionIndex].pData->httpInfo.ipAddress);
                    break;
                }
				
				// Get updated model name if camera brand is MATRIX	
                if (strcmp(advCamSearchSession[sessionIndex].pData->result[devIndex].brand, MATRIX_BRAND_NAME) == STATUS_OK)
                {
                    GetUpdatedMatrixCameraModelName(advCamSearchSession[sessionIndex].pData->result[devIndex].model, MAX_MODEL_NAME_LEN);
                }

                // Extract http Port from presentationURL
                if ((startPtr = strstr(startPtr, advCamSearchSession[sessionIndex].pData->httpInfo.ipAddress)) != NULL)
                {
                    startPtr += strlen(advCamSearchSession[sessionIndex].pData->httpInfo.ipAddress);

                    // Check if Port Field Exists
                    if (startPtr[0] == ':')
                    {
                        startPtr++;
                        if ((sscanf(startPtr, "%hd", &httpPort) != 1) || (httpPort == 0))
                        {
                            httpPort = DFLT_HTTP_PORT;
                        }
                    }
                }

                advCamSearchSession[sessionIndex].pData->result[devIndex].httpPort = httpPort;

                // Fill Ip Address
                snprintf(advCamSearchSession[sessionIndex].pData->result[devIndex].ipv4Addr, sizeof(advCamSearchSession[sessionIndex].pData->result[devIndex].ipv4Addr),
                         "%s", advCamSearchSession[sessionIndex].pData->httpInfo.ipAddress);
                advCamSearchSession[sessionIndex].pData->result[devIndex].ipv6Addr[IPV6_ADDR_GLOBAL][0] = '\0';
                advCamSearchSession[sessionIndex].pData->result[devIndex].ipv6Addr[IPV6_ADDR_LINKLOCAL][0] = '\0';

                if (deviceAlreadyFound == FALSE)
                {
                    // Increment total Devices Found
                    advCamSearchSession[sessionIndex].totalDevices++;
                    advCamSearchSession[sessionIndex].pData->result[devIndex].onvifPort = DFLT_ONVIF_PORT;
                    advCamSearchSession[sessionIndex].pData->result[devIndex].onvifSupport = FALSE;
                }

                // send To Client......
                advCamSearchSession[sessionIndex].pData->result[devIndex].updationOnNewSearch = TRUE;
                checkIfCamAdded(devIndex, sessionIndex);

            }while(0);

            MUTEX_UNLOCK(advCamSearchSession[sessionIndex].searchResultLock);
            advCamSearchSession[sessionIndex].httpReqStatus = SUCCESS;
        }
        break;

        case HTTP_CLOSE_ON_SUCCESS:
        case HTTP_CLOSE_ON_ERROR:
        {
            if (advCamSearchSession[sessionIndex].httpReqStatus == FAIL)
            {
                MUTEX_LOCK(advCamSearchSession[sessionIndex].searchResultLock);
                // Check if device has already been found
                for (devIndex = 0; devIndex < advCamSearchSession[sessionIndex].totalDevices; devIndex++)
                {
                    if (strcmp(advCamSearchSession[sessionIndex].pData->httpInfo.ipAddress,
                            advCamSearchSession[sessionIndex].pData->result[devIndex].ipv4Addr) == STATUS_OK)
                    {
                        deviceAlreadyFound = TRUE;
                        advCamSearchSession[sessionIndex].pData->result[devIndex].updationOnNewSearch = TRUE;
                        break;
                    }
                }

                if ((devIndex < MAX_CAM_SEARCH_IN_ONE_SHOT) && (deviceAlreadyFound == FALSE))
                {
                    // Fill Ip Address
                    snprintf(advCamSearchSession[sessionIndex].pData->result[devIndex].ipv4Addr, sizeof(advCamSearchSession[sessionIndex].pData->result[devIndex].ipv4Addr),
                             "%s", advCamSearchSession[sessionIndex].pData->httpInfo.ipAddress);
                    advCamSearchSession[sessionIndex].pData->result[devIndex].ipv6Addr[IPV6_ADDR_GLOBAL][0] = '\0';
                    advCamSearchSession[sessionIndex].pData->result[devIndex].ipv6Addr[IPV6_ADDR_LINKLOCAL][0] = '\0';
                    advCamSearchSession[sessionIndex].pData->result[devIndex].brand[0] = '\0';
                    advCamSearchSession[sessionIndex].pData->result[devIndex].model[0] = '\0';
                    advCamSearchSession[sessionIndex].pData->result[devIndex].httpPort = DFLT_HTTP_PORT;
                    advCamSearchSession[sessionIndex].pData->result[devIndex].onvifSupport = FALSE;
                    advCamSearchSession[sessionIndex].pData->result[devIndex].updationOnNewSearch = TRUE;
                    advCamSearchSession[sessionIndex].totalDevices++;
                }

                /* In search process reapeat ... we are adding new camera to search result, but if it is old one,
                 * we need to check whether it is added or not every time, because configuration may change in between. */
                checkIfCamAdded(devIndex, sessionIndex);
                MUTEX_UNLOCK(advCamSearchSession[sessionIndex].searchResultLock);
            }

            // Increment Request Count
            advCamSearchSession[sessionIndex].requestCount++;

            MUTEX_LOCK(advCamSearchSession[sessionIndex].requestStatusMutex);
            if (advCamSearchSession[sessionIndex].requestStatus == INTERRUPTED)
            {
                userHasAbortedSearch = TRUE;
            }
            MUTEX_UNLOCK(advCamSearchSession[sessionIndex].requestStatusMutex);

            // Check if Any Request is pending to be sent
            if (advCamSearchSession[sessionIndex].requestCount < advCamSearchSession[sessionIndex].devicesFoundInUpnP)
            {
                if (userHasAbortedSearch == FALSE)
                {
                    // Send Next Request
                    if (sendAdvSearchUpnpLocHTTPReq(sessionIndex) == FAIL)
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
                replyEndAdvanceCamSearchResult(sessionIndex);
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
 * @brief   sendDevInfoHTTPReq
 * @param   sessionIndex
 * @return  SUCCESS/FAIL
 */
static BOOL	sendDevInfoHTTPReq(UINT8 sessionIndex)
{
	BOOL			retVal = SUCCESS, deviceAlreadyFound = FALSE, breakLoop = FALSE;
	HTTP_HANDLE		httpHandle;
	UINT8			devIndex = 0;
	UINT8			ipLastOctect = 0;
	UINT32			httpUserData = 0;
	HTTP_INFO_t 	httpInfo;

    if (sessionIndex >= MAX_ADV_SEARCH_SESSION)
    {
        EPRINT(CAMERA_INTERFACE, "invld session found: [sessionIndex=%d]", sessionIndex);
        return FAIL;
    }

	httpInfo.authMethod = AUTH_TYPE_ANY;
	httpInfo.maxConnTime = MAX_CONN_TIME;
	httpInfo.maxFrameTime = MAX_CONN_TIME;
    snprintf(httpInfo.httpUsrPwd.username,MAX_CAMERA_USERNAME_WIDTH,"%s", advCamSearchSession[sessionIndex].inputParam.camUsername);
    snprintf(httpInfo.httpUsrPwd.password,MAX_CAMERA_PASSWORD_WIDTH,"%s", advCamSearchSession[sessionIndex].inputParam.camPassword);
	httpInfo.port = advCamSearchSession[sessionIndex].inputParam.httpPort;
	httpInfo.userAgent = CURL_USER_AGENT;
	httpInfo.interface = MAX_HTTP_INTERFACE;

	do
	{
		do
		{
			// First of all we need to check whether this same ip address is found in upnp search,
			// if same ip found , send skip this ip, go for next ip
			deviceAlreadyFound = FALSE;
			ipLastOctect = (advCamSearchSession[sessionIndex].requestCount + advCamSearchSession[sessionIndex].startIpAddressAllOctect[3]);
			if((ipLastOctect > advCamSearchSession[sessionIndex].endIpAddressLastOctect) || (ipLastOctect == 0))
			{
				retVal = FAIL;
				breakLoop = TRUE;
				break;
			}
			else
			{
                snprintf(httpInfo.ipAddress,MAX_CAMERA_ADDRESS_WIDTH, "%d.%d.%d.%d", advCamSearchSession[sessionIndex].startIpAddressAllOctect[0],
                        advCamSearchSession[sessionIndex].startIpAddressAllOctect[1], advCamSearchSession[sessionIndex].startIpAddressAllOctect[2], ipLastOctect);

				// Check if device has already been found
                MUTEX_LOCK(advCamSearchSession[sessionIndex].searchResultLock);
				for (devIndex = 0; devIndex < advCamSearchSession[sessionIndex].totalDevices; devIndex++)
				{
                    if (strcmp(httpInfo.ipAddress, advCamSearchSession[sessionIndex].pData->result[devIndex].ipv4Addr) == STATUS_OK)
					{
						deviceAlreadyFound = TRUE;
						advCamSearchSession[sessionIndex].requestCount++;
						break;
					}
				}
                MUTEX_UNLOCK(advCamSearchSession[sessionIndex].searchResultLock);
			}
		}while(deviceAlreadyFound == TRUE);

		if(breakLoop == TRUE)
		{
			break;
		}

		if(retVal == SUCCESS)
		{
            snprintf(httpInfo.relativeUrl, MAX_RELATIVE_URL_WIDTH, "%s", advCamSearchSession[sessionIndex].url[advCamSearchSession[sessionIndex].reqNoToServe].relativeUrl);
			advCamSearchSession[sessionIndex].httpReqStatus = FAIL;

			httpUserData = (UINT8)sessionIndex;
            httpUserData |= ((advCamSearchSession[sessionIndex].reqNoToServe << 8) & 0xff00);
			httpUserData |= (UINT32)(ipLastOctect << 16);

            if((retVal = StartHttp(GET_REQUEST, &httpInfo, httpAdvCamSearchDevInfoCb, httpUserData , &httpHandle)) == SUCCESS)
			{
				advCamSearchSession[sessionIndex].requestCount++;
				advCamSearchSession[sessionIndex].devInfoReqSent++;
                DPRINT(CAMERA_INTERFACE, "start http: [ReqNo=%d], [sent=%d], [requestCount=%d]", advCamSearchSession[sessionIndex].reqNoToServe,
                       advCamSearchSession[sessionIndex].devInfoReqSent, advCamSearchSession[sessionIndex].requestCount);
				if(advCamSearchSession[sessionIndex].devInfoReqSent >= MAX_DEVINFO_REQ_IN_ONE_SHOT)
				{
					breakLoop = TRUE;
				}
			}
			else
			{
				breakLoop = TRUE;
			}
		}

        MUTEX_LOCK(advCamSearchSession[sessionIndex].requestStatusMutex);
		if (advCamSearchSession[sessionIndex].requestStatus == INTERRUPTED)
		{
            MUTEX_UNLOCK(advCamSearchSession[sessionIndex].requestStatusMutex);
			break;
		}
		else
		{
            MUTEX_UNLOCK(advCamSearchSession[sessionIndex].requestStatusMutex);
		}

    } while(breakLoop == FALSE);

	return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   httpAdvCamSearchDevInfoCb
 * @param   httpHandle
 * @param   dataInfo
 */
static void httpAdvCamSearchDevInfoCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t * dataInfo)
{
	BOOL					userHasAbortedSearch = FALSE;
	BOOL					freeSession = FALSE;
    PARSER_DEVICE_INFO_FUNC *parseDevInfoFunc;
	CHAR					tempModelName[MAX_MODEL_NAME_LEN];
    CHAR					cameraIpAddr[IPV6_ADDR_LEN_MAX];
    UINT8 					devIndex = 0;
	UINT16					onvifPort;
	ADV_CAM_HOST_RESPONSE	hostResponse = ADV_CAM_HOST_UNAVAILABLE;
	UINT8					sessionIndex = (UINT8)(dataInfo->userData & 0x00ff);
    UINT8					reqNoToServe = (UINT8)((dataInfo->userData & 0xff00) >> 8);
	UINT8					lastOctect = (UINT8)((dataInfo->userData >> 16) & 0xffff);

    /* Validate session index. It should be in defined range */
    if (sessionIndex >= MAX_ADV_SEARCH_SESSION)
    {
        EPRINT(CAMERA_INTERFACE, "invld session found: [sessionIndex=%d]", sessionIndex);
        return;
    }

	switch (dataInfo->httpResponse)
	{
        case HTTP_SUCCESS:
        {
            if ((dataInfo->frameSize > 0) && (dataInfo->storagePtr != NULL))
            {
                if (((parseDevInfoFunc = ParseGetDevInfoFunc(advCamSearchSession[sessionIndex].brandNum))) == NULL)
                {
                    EPRINT(CAMERA_INTERFACE, "parser function not registered: [brand=%d]", advCamSearchSession[sessionIndex].brandNum);
                }
                else if ((*parseDevInfoFunc)(reqNoToServe, dataInfo->frameSize, tempModelName, dataInfo->storagePtr) != CMD_SUCCESS)
                {
                    EPRINT(CAMERA_INTERFACE, "failed to parse get device info: [reqNo=%d], [url=%s]",
                           reqNoToServe, advCamSearchSession[sessionIndex].url[reqNoToServe].relativeUrl);
                    hostResponse = ADV_CAM_COMM_ERROR;
                    insertFailDevEntry(lastOctect, hostResponse, sessionIndex);
                }
                else
                {
                    // parsing get device information success, fill data in result
                    snprintf(cameraIpAddr, sizeof(cameraIpAddr), "%d.%d.%d.%d" , advCamSearchSession[sessionIndex].startIpAddressAllOctect[0],
                    advCamSearchSession[sessionIndex].startIpAddressAllOctect[1], advCamSearchSession[sessionIndex].startIpAddressAllOctect[2], lastOctect);

                    MUTEX_LOCK(advCamSearchSession[sessionIndex].searchResultLock);
                    if(advCamSearchSession[sessionIndex].sessionId != INVALID_ADV_SEARCH_SESSION)
                    {
                        for (devIndex = 0; devIndex < advCamSearchSession[sessionIndex].totalDevices; devIndex++)
                        {
                            if (strcmp(advCamSearchSession[sessionIndex].pData->result[devIndex].ipv4Addr, cameraIpAddr) == STATUS_OK)
                            {
                                break;
                            }
                        }

                        advCamSearchSession[sessionIndex].pData->result[devIndex].updationOnNewSearch = TRUE;
                        if(devIndex == advCamSearchSession[sessionIndex].totalDevices)
                        {
                            advCamSearchSession[sessionIndex].pData->result[devIndex].onvifSupport = checkCameraOnvifSupport(cameraIpAddr, sessionIndex, &onvifPort);
                            advCamSearchSession[sessionIndex].pData->result[devIndex].onvifPort = 
                                    advCamSearchSession[sessionIndex].pData->result[devIndex].onvifSupport ? onvifPort : DFLT_ONVIF_PORT;

                            snprintf(advCamSearchSession[sessionIndex].pData->result[devIndex].ipv4Addr,
                                     sizeof(advCamSearchSession[sessionIndex].pData->result[devIndex].ipv4Addr), "%s", cameraIpAddr);
                            advCamSearchSession[sessionIndex].pData->result[devIndex].ipv6Addr[IPV6_ADDR_GLOBAL][0] = '\0';
                            advCamSearchSession[sessionIndex].pData->result[devIndex].ipv6Addr[IPV6_ADDR_LINKLOCAL][0] = '\0';
                            advCamSearchSession[sessionIndex].pData->result[devIndex].httpPort = advCamSearchSession[sessionIndex].inputParam.httpPort;
                            snprintf(advCamSearchSession[sessionIndex].pData->result[devIndex].brand, MAX_BRAND_NAME_LEN, "%s",
                                     advCamSearchSession[sessionIndex].inputParam.brand);
                            snprintf(advCamSearchSession[sessionIndex].pData->result[devIndex].model, MAX_MODEL_NAME_LEN, "%s", tempModelName);

                            // send Response To client
                            advCamSearchSession[sessionIndex].pData->result[devIndex].updationOnNewSearch = TRUE;
                            advCamSearchSession[sessionIndex].totalDevices++;
                        }

                        checkIfCamAdded(devIndex, sessionIndex);
                        removeFailDevEntry(lastOctect, ADV_CAM_SERACH_SUCESS, sessionIndex);
                    }
                    MUTEX_UNLOCK(advCamSearchSession[sessionIndex].searchResultLock);
                }
            }
            advCamSearchSession[sessionIndex].httpReqStatus = SUCCESS;
        }
        break;

        case HTTP_ERROR:
        {
            if(dataInfo->cmdResponse == CMD_CRED_INVALID)
            {
                hostResponse = ADV_CAM_UNAUTHRIZE;
            }
            else
            {
                hostResponse = ADV_CAM_COMM_ERROR;
            }
            insertFailDevEntry(lastOctect, hostResponse, sessionIndex);
        }
        break;


        case HTTP_CLOSE_ON_ERROR:
        {
            // Check if device has already been found in fail list
            insertFailDevEntry(lastOctect, ADV_CAM_HOST_UNAVAILABLE, sessionIndex);
        }
        // fall through
        case HTTP_CLOSE_ON_SUCCESS:
        {
            // Increment Request Count ... this reqCount will be added in next ip address generate.
            MUTEX_LOCK(advCamSearchSession[sessionIndex].requestStatusMutex);
            if (advCamSearchSession[sessionIndex].requestStatus == INTERRUPTED)
            {
                userHasAbortedSearch = TRUE;
            }
            MUTEX_UNLOCK(advCamSearchSession[sessionIndex].requestStatusMutex);

            if (userHasAbortedSearch == FALSE)
            {
                if(advCamSearchSession[sessionIndex].devInfoReqSent)
                {
                    advCamSearchSession[sessionIndex].devInfoReqSent--;
                    if(advCamSearchSession[sessionIndex].devInfoReqSent == 0)
                    {
                        //send next bunch of requests,
                        if(sendDevInfoHTTPReq(sessionIndex) == FALSE)
                        {
                            advCamSearchSession[sessionIndex].reqNoToServe++;
                            if(advCamSearchSession[sessionIndex].reqNoToServe < advCamSearchSession[sessionIndex].numOfRequest)
                            {
                                advCamSearchSession[sessionIndex].requestCount = 0;
                                sendDevInfoHTTPReq(sessionIndex);
                                MUTEX_LOCK(advCamSearchSession[sessionIndex].requestStatusMutex);
                                if (advCamSearchSession[sessionIndex].requestStatus == INTERRUPTED)
                                {
                                    userHasAbortedSearch = TRUE;
                                }
                                MUTEX_UNLOCK(advCamSearchSession[sessionIndex].requestStatusMutex);
                                if((userHasAbortedSearch == TRUE) || (advCamSearchSession[sessionIndex].devInfoReqSent == 0))
                                {
                                    freeSession = TRUE;
                                }
                            }
                            else if(advCamSearchSession[sessionIndex].devInfoReqSent == 0)
                            {
                                freeSession = TRUE;
                            }
                        }
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
                if (userHasAbortedSearch == FALSE)
                {
                    sleep(5);
                }
                replyEndAdvanceCamSearchResult(sessionIndex);
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
 * @brief   processOnvifUnicastReq
 * @param   connFd
 * @param   ipAddress
 * @param   msgPtr
 * @param   messageLength
 * @param   sessionIndex
 * @param   onvifPort
 * @return
 */
static ADV_CAM_HOST_RESPONSE processOnvifUnicastReq(INT32 connFd, CHARPTR ipAddress, CHARPTR msgPtr,
                                                    UINT16 messageLength, UINT8 sessionIndex, UINT16PTR onvifPort)
{
	struct sockaddr_in 		groupSock;
	INT32 					recvBytes = 0;
	UINT8 					ignoreBytes = 0;
	CHARPTR 				startStr;
	CHARPTR 				endStr;
	INT16 					messageLen;
	CHAR 					address[MAX_AVAILABLE_ADDRESS_LEN];
	ADV_CAM_HOST_RESPONSE	hostResponse = ADV_CAM_HOST_UNAVAILABLE;
    UINT8                   pollSts;
    INT16                   pollRevent;

    // now multicast device discovery message
    memset(&groupSock, 0, sizeof(groupSock));
    groupSock.sin_family = AF_INET;
    groupSock.sin_addr.s_addr = inet_addr(ipAddress);
    groupSock.sin_port = htons(ONVIF_MULTICAST_PORT);

	do
    {
        if (sendto(connFd, msgPtr, messageLength, MSG_NOSIGNAL, (struct sockaddr*)&groupSock, sizeof(groupSock)) != messageLength)
        {
            EPRINT(CAMERA_INTERFACE, "failed to send onvif unicast request: [ip=%s]", ipAddress);
            break;
        }

        pollSts = GetSocketPollEvent(connFd, (POLLRDNORM | POLLRDHUP), MAX_ADV_SERCH_ONVIF_TIMEOUT_MS, &pollRevent);

        if (FAIL == pollSts)
        {
            EPRINT(CAMERA_INTERFACE, "poll Failed: [ip=%s]", ipAddress);
            break;
        }

        // if time out then return
        if (TIMEOUT == pollSts)
		{
            EPRINT(CAMERA_INTERFACE, "onvif response not recv, timeout occurred: [ip=%s]", ipAddress);
			break;
		}

        // if other than read event
        if ((pollRevent & POLLRDNORM) != POLLRDNORM)
        {
            if ((pollRevent & POLLRDHUP) == POLLRDHUP)
            {
                EPRINT(CAMERA_INTERFACE, "remote connection closed: [ip=%s]", ipAddress);
            }
            else
            {
                EPRINT(CAMERA_INTERFACE, "invalid poll event:[0x%x], [ip=%s]", pollRevent, ipAddress);
            }

            break;
        }

		// receive multicast message sent by different ip camera
        recvBytes = recv(connFd, advCamSearchSession[sessionIndex].pData->msgBuffer, MAX_BUFFER_SIZE, MSG_NOSIGNAL);
        if (recvBytes <= 0)
        {
            EPRINT(CAMERA_INTERFACE, "msg recv fail: [ip=%s]", ipAddress);
			break;
		}

		// terminate receive message with NULL.
		advCamSearchSession[sessionIndex].pData->msgBuffer[recvBytes] = '\0';
		ignoreBytes = 0;

		// Ignore trailing '\r' '\n'
        while ((advCamSearchSession[sessionIndex].pData->msgBuffer[recvBytes - 1 - ignoreBytes] == '\r')
                || (advCamSearchSession[sessionIndex].pData->msgBuffer[recvBytes - 1 - ignoreBytes] == '\n'))
		{
			ignoreBytes++;
		}

		// Now parse the message to retrieve its device entry address
		// Check if String ends with </SOAP-ENV:Envelope>
		if (strncasecmp((advCamSearchSession[sessionIndex].pData->msgBuffer + recvBytes - ignoreBytes - strlen(DEVICE_DISCOVERY_RESP_END)),
                DEVICE_DISCOVERY_RESP_END, strlen(DEVICE_DISCOVERY_RESP_END)) != STATUS_OK)
		{
            break;
        }

        // Get Position of XAddrs in which entry point is given
        if ((startStr = strcasestr(advCamSearchSession[sessionIndex].pData->msgBuffer, ONVIF_ENTRY_POINT_TAG)) == NULL)
        {
            break;
        }

        startStr += strlen(ONVIF_ENTRY_POINT_TAG);
        if ((endStr = strchr(startStr, '<')) != NULL)
        {
            messageLen = (endStr - startStr);
            if((messageLen > 0) && (messageLen < MAX_AVAILABLE_ADDRESS_LEN))
            {
                strncpy(address, startStr, messageLen);
                address[messageLen] = '\0';
                getOnvifIpAddrPortFromEntryPoint(address, sessionIndex, onvifPort);
                hostResponse = ADV_CAM_SERACH_SUCESS;
            }
        }

	}while(0);

	return hostResponse;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   getOnvifIpAddrPortFromEntryPoint
 * @param   entryPoint
 * @param   sessionIndex
 * @param   onvifPort
 */
static void getOnvifIpAddrPortFromEntryPoint(CHARPTR entryPoint, UINT8 sessionIndex, UINT16PTR onvifPort)
{
	UINT8		stLen = 0;
	CHARPTR 	searchPtr;
	BOOL		colFound = FALSE;
	CHAR		tempChr;
	CHARPTR 	endPtr = NULL;
    INT32		tmpHttpPort = DFLT_HTTP_PORT;

	stLen = strlen("http://");
	// Check if it is in format http://ipAdderss/RelativePath
    if (strncmp(entryPoint, "http://", stLen) != STATUS_OK)
	{
        return;
    }

    searchPtr = (entryPoint + stLen);
    stLen = 0;

    while(1)
    {
        tempChr = *(searchPtr + stLen);
        if(tempChr == ':')
        {
            endPtr = (searchPtr + stLen);
            colFound = TRUE;
            break;
        }
        else if(tempChr == '/')
        {
            endPtr = (searchPtr + stLen);
            break;
        }
        else if(tempChr == '\0')
        {
            break;
        }
        else
        {
            stLen++;
        }
    }

    // Get End Address Of Ip Address
    if (endPtr != NULL)
    {
        if(colFound == TRUE)
        {
            // Here, xadderss [http://:-1/onvif/device_service] found sometime
            // so, we have to take care of signed value in port
            if((sscanf((endPtr + 1), "%d", &tmpHttpPort) != 1) || (tmpHttpPort <= 0))
            {
                tmpHttpPort = DFLT_HTTP_PORT;
            }
        }
    }

    (*onvifPort) = (UINT16)tmpHttpPort;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   processUpnpUnicastReq
 * @param   connFd
 * @param   ipAddress
 * @param   msgPtr
 * @param   messageLength
 * @param   sessionIndex
 * @return
 */
static ADV_CAM_HOST_RESPONSE processUpnpUnicastReq(INT32 connFd, CHARPTR ipAddress, CHARPTR msgPtr, UINT16 messageLength, UINT8	sessionIndex)
{
	struct sockaddr_in 		groupSock;
    UINT8                   pollSts;
    INT16                   pollRevent;
	INT32 					recvBytes = 0;
	CHARPTR 				startStr;
	CHARPTR 				endStr;
	ADV_CAM_HOST_RESPONSE	hostResponse = ADV_CAM_HOST_UNAVAILABLE;

	memset(&groupSock, 0, sizeof(groupSock));
	groupSock.sin_family = AF_INET;
	groupSock.sin_addr.s_addr = inet_addr(ipAddress);
	groupSock.sin_port = htons(MULTICAST_PORT);

    if (sendto(connFd, msgPtr, messageLength, MSG_NOSIGNAL, (struct sockaddr*)&groupSock, sizeof(groupSock)) != messageLength)
	{
        EPRINT(CAMERA_INTERFACE, "failed to send upnp unicast request: [ip=%s]", ipAddress);
	}

	do
    {
        // poll available socket's fd
        pollSts = GetSocketPollEvent(connFd, (POLLRDNORM | POLLRDHUP), MAX_ADV_SERCH_UPNP_TIMEOUT_MS, &pollRevent);
        if (TIMEOUT == pollSts)
		{
            EPRINT(CAMERA_INTERFACE, "upnp response not recv, timeout occurred: [ip=%s]", ipAddress);
			break;
		}

        if (FAIL == pollSts)
        {
            EPRINT(CAMERA_INTERFACE, "upnp response not recv, poll failed: [ip=%s]", ipAddress);
            break;
        }
		
        // if other than read event
        if ((pollRevent & POLLRDNORM) != POLLRDNORM)
        {
            if ((pollRevent & POLLRDHUP) == POLLRDHUP)
            {
                EPRINT(CAMERA_INTERFACE, "remote connection closed: [ip=%s]", ipAddress);
            }
            else
            {
                EPRINT(CAMERA_INTERFACE, "invalid poll event:[0x%x], [ip=%s]", pollRevent, ipAddress);
            }

            break;
        }

		// receive multicast message sent by different ip camera
        recvBytes = recv(connFd, advCamSearchSession[sessionIndex].pData->msgBuffer, MAX_BUFFER_SIZE, MSG_NOSIGNAL);
        if (recvBytes <= 0)
        {
            EPRINT(CAMERA_INTERFACE, "msg recv fail: [ip=%s]", ipAddress);
			break;
		}

		// terminate receive message with NULL.
		advCamSearchSession[sessionIndex].pData->msgBuffer[recvBytes] = '\0';

		// Parse Reponse to Get Location Field
        if ((startStr = strcasestr(advCamSearchSession[sessionIndex].pData->msgBuffer, LOCATION_FIELD)) == NULL)
		{
            break;
        }

        startStr = startStr + strlen(LOCATION_FIELD);
        // Skip White Spaces
        while (startStr[0] == ' ')
        {
            startStr++;
        }

        if ((endStr = strchr(startStr, '\r')) != NULL)
        {
            if (((endStr - startStr) > 0) && ((endStr - startStr) < MAX_LOCATION_NAME_LEN))
            {
                strncpy(advCamSearchSession[sessionIndex].pData->location[advCamSearchSession[sessionIndex].devicesFoundInUpnP], startStr, (endStr - startStr));
                advCamSearchSession[sessionIndex].pData->location[advCamSearchSession[sessionIndex].devicesFoundInUpnP][endStr - startStr] = '\0';
                //DPRINT(CAMERA_INTERFACE, "upnp resp: [location=%s]",
                //       advCamSearchSession[sessionIndex].pData->location[advCamSearchSession[sessionIndex].devicesFoundinUpnP]);
                advCamSearchSession[sessionIndex].devicesFoundInUpnP++;
                hostResponse = ADV_CAM_SERACH_SUCESS;
            }
        }

	}while(0);

	return hostResponse;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   GenerateFailureReport
 * @param   connFd
 * @param   callbackType
 * @param   sessionIndex
 * @return
 */
NET_CMD_STATUS_e GenerateFailureReport(INT32 connFd, CLIENT_CB_TYPE_e callbackType, UINT8 sessionIndex)
{
    UINT8   indexId = 0;
    UINT32  outLen;
    CHAR    replyMsg[10000];

    outLen = snprintf(replyMsg, sizeof(replyMsg), "%c%s%c%d%c", SOM, headerReq[RPL_CMD], FSP, CMD_SUCCESS, FSP);
    if(outLen > sizeof(replyMsg))
    {
        EPRINT(CAMERA_INTERFACE, "more buffer required: [outLen=%d]", outLen);
        outLen = sizeof(replyMsg);
    }

    MUTEX_LOCK(advCamSearchSession[sessionIndex].failDevicesLock);
	for(indexId = 0; indexId < advCamSearchSession[sessionIndex].failDevCount; indexId++)
	{
        outLen += snprintf(replyMsg + outLen,sizeof(replyMsg) - outLen,
                "%c%d%c%d.%d.%d.%d%c%d%c%c", SOI, (indexId + 1), FSP,
				advCamSearchSession[sessionIndex].startIpAddressAllOctect[0], advCamSearchSession[sessionIndex].startIpAddressAllOctect[1],
				advCamSearchSession[sessionIndex].startIpAddressAllOctect[2], (advCamSearchSession[sessionIndex].failIpList[indexId]),
                FSP, (advCamSearchSession[sessionIndex].failIpSatus[indexId]), FSP, EOI);

        //Validate Buffer for add last two character including NULL
        if(outLen > sizeof(replyMsg) - 2)
        {
            EPRINT(CAMERA_INTERFACE, "more buffer required: [outLen=%d]", outLen);
            outLen = sizeof(replyMsg) - 2;
            break;
        }
	}
    MUTEX_UNLOCK(advCamSearchSession[sessionIndex].failDevicesLock);

    outLen += snprintf(replyMsg + outLen,sizeof(replyMsg) - outLen, "%c", EOM);
    if(outLen > sizeof(replyMsg))
    {
        EPRINT(CAMERA_INTERFACE, "more buffer required: [outLen=%d]", outLen);
        outLen = sizeof(replyMsg);
    }

    if(FAIL == sendCmdCb[callbackType](connFd, (UINT8PTR)replyMsg, outLen, MESSAGE_REPLY_TIMEOUT))
    {
        EPRINT(CAMERA_INTERFACE, "failed to send advance camera search report");
        return CMD_CONNECTIVITY_ERROR;
    }

    closeConnCb[callbackType](&connFd);
	return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   insertFailDevEntry
 * @param   lastOctect
 * @param   hostResponse
 * @param   sessionIndex
 */
static void insertFailDevEntry(UINT8 lastOctect, ADV_CAM_HOST_RESPONSE hostResponse, UINT8 sessionIndex)
{
	UINT8	devIndex = 0, laterIndex = 0;
	BOOL	devFoud = FALSE;
	UINT8	tempOctectArray[255];
	ADV_CAM_HOST_RESPONSE	tempHostResponse[255];
	UINT8	index = 0;

    if (sessionIndex >= MAX_ADV_SEARCH_SESSION)
    {
        EPRINT(CAMERA_INTERFACE, "invld session found: [sessionIndex=%d]", sessionIndex);
        return;
    }

    MUTEX_LOCK(advCamSearchSession[sessionIndex].failDevicesLock);
	for (devIndex = 0; devIndex < advCamSearchSession[sessionIndex].failDevCount; devIndex++)
	{
		if(advCamSearchSession[sessionIndex].failIpList[devIndex] == lastOctect)
		{
			devFoud = TRUE;
		}
	}

	if(devFoud == FALSE)
	{
		for (devIndex = 0; devIndex < advCamSearchSession[sessionIndex].failDevCount; devIndex++)
		{
			if(advCamSearchSession[sessionIndex].failIpList[devIndex] > lastOctect)
			{
				break;;
			}
		}

		for(laterIndex = devIndex; laterIndex < advCamSearchSession[sessionIndex].failDevCount; laterIndex++)
		{
			tempOctectArray[index] = advCamSearchSession[sessionIndex].failIpList[laterIndex];
			tempHostResponse[index] = advCamSearchSession[sessionIndex].failIpSatus[laterIndex];
			index++;
		}

		advCamSearchSession[sessionIndex].failIpList[devIndex] = lastOctect;
		advCamSearchSession[sessionIndex].failIpSatus[devIndex] = hostResponse;
		advCamSearchSession[sessionIndex].failDevCount++;

		index = 0;
		for(laterIndex = (devIndex + 1); laterIndex < advCamSearchSession[sessionIndex].failDevCount; laterIndex++)
		{
			advCamSearchSession[sessionIndex].failIpList[laterIndex] = tempOctectArray[index];
			advCamSearchSession[sessionIndex].failIpSatus[laterIndex] = tempHostResponse[index];
			index++;
		}
	}
	else
	{
		advCamSearchSession[sessionIndex].failIpSatus[devIndex] = hostResponse;
	}
    MUTEX_UNLOCK(advCamSearchSession[sessionIndex].failDevicesLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   removeFailDevEntry
 * @param   lastOctect
 * @param   hostResponse
 * @param   sessionIndex
 */
static void removeFailDevEntry(UINT8 lastOctect, ADV_CAM_HOST_RESPONSE hostResponse, UINT8 sessionIndex)
{
    UINT8                   devIndex = 0, laterIndex = 0;
    BOOL                    devFoud = FALSE;
    UINT8                   tempOctectArray[255];
	ADV_CAM_HOST_RESPONSE	tempHostResponse[255];
    UINT8                   index = 0;

    MUTEX_LOCK(advCamSearchSession[sessionIndex].failDevicesLock);
	for (devIndex = 0; devIndex < advCamSearchSession[sessionIndex].failDevCount; devIndex++)
	{
		if(advCamSearchSession[sessionIndex].failIpList[devIndex] == lastOctect)
		{
			devFoud = TRUE;
			break;
		}
	}

	if(devFoud == TRUE)
	{
		for(laterIndex = (devIndex + 1); laterIndex < advCamSearchSession[sessionIndex].failDevCount; laterIndex++)
		{
			tempOctectArray[index] = advCamSearchSession[sessionIndex].failIpList[laterIndex];
			tempHostResponse[index] = advCamSearchSession[sessionIndex].failIpSatus[laterIndex];
			index++;
		}

		advCamSearchSession[sessionIndex].failDevCount --;

		index = 0;
		for(laterIndex = devIndex; laterIndex < advCamSearchSession[sessionIndex].failDevCount; laterIndex++)
		{
			advCamSearchSession[sessionIndex].failIpList[laterIndex] = tempOctectArray[index];
			advCamSearchSession[sessionIndex].failIpSatus[laterIndex] = tempHostResponse[index];
			index++;
		}
	}
    MUTEX_UNLOCK(advCamSearchSession[sessionIndex].failDevicesLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function gives value of requested tag from given source XML data.
 * @param   sessionIndex
 * @return
 */
BOOL IsAdvCamSearchActiveForClient(UINT8 sessionIndex)
{
	BOOL status = INACTIVE;

    MUTEX_LOCK(advCamSearchSession[sessionIndex].requestStatusMutex);
    if ((advCamSearchSession[sessionIndex].requestStatus == ACTIVE) || (advCamSearchSession[sessionIndex].requestStatus == INTERRUPTED))
	{
		status = ACTIVE;
	}
    MUTEX_UNLOCK(advCamSearchSession[sessionIndex].requestStatusMutex);

	return status;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
