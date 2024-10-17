//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		AutoConfigCamera.c
@brief      This file provides to configure camera automatically
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "AutoConfigCamera.h"
#include "CameraConfig.h"
#include "MxOnvifClient.h"
#include "DeviceDefine.h"
#include "CameraInterface.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define AUTO_CONFIG_THREAD_STACK_SZ     (4* MEGA_BYTE)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef struct
{
    BOOL                        camIpChangeStatus;
    BOOL                        isChangeIpRequest;
    UINT8                       ipChangeSuccessCnt;
    UINT16                      httpPort;
    UINT16                      onvifSupportF;
    UINT16                      onvifPort;
    CHAR                        userName[MAX_CAMERA_USERNAME_WIDTH];
    CHAR                        passWord[MAX_CAMERA_PASSWORD_WIDTH];
    CHAR                        ipAddress[IPV6_ADDR_LEN_MAX];
    CHAR                        brand[MAX_BRAND_NAME_LEN];
    CHAR                        model[MAX_MODEL_NAME_LEN];
    IP_ADDR_PARAM_t             newNetworkParam;
    CAMERA_REQUEST_t            changeCamIpAddr;
    pthread_cond_t              camIpChangeCondWait;
    pthread_mutex_t             camIpChangeCondWaitMutex;

}AUTO_CONFIG_CHANGE_IP_ADDR_t;

typedef struct
{
    BOOL                        getDeviceInfoStatus;
    BOOL                        isSetPassword;
    CHAR                        userName[MAX_CAMERA_USERNAME_WIDTH];
    CHAR                        passWord[MAX_CAMERA_PASSWORD_WIDTH];
    CHAR                        ipAddress[IPV6_ADDR_LEN_MAX];
    CHAR                        brand[MAX_BRAND_NAME_LEN];
    CHAR                        model[MAX_MODEL_NAME_LEN];
    UINT16                      httpPort;
    CAMERA_REQUEST_t            checkAuthentication;
    ONVIF_PROFILE_STREAM_INFO_t	profileParam;
    UINT8                       cameraIndex;

}AUTO_CONFIG_CHECK_AUTH_t;

typedef struct
{
    AUTO_CONFIG_REQ_PARAM_t cameraList[MAX_CONFIGURE_CAMERA_LIST];
    UINT8					reqListEntry;
    pthread_mutex_t			reqListEntryLock;
    BOOL					requestStatus;
    pthread_mutex_t			requestStatusLock;

}AUTO_CONFIG_CAMERA_SESSION_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static AUTO_CONFIG_CHANGE_IP_ADDR_t autoConfigChangeIpRequest;
static AUTO_CONFIG_CHECK_AUTH_t     autoConfigCheckAuthentication;
static AUTO_CONFIG_CAMERA_SESSION_t autoConfigSession;
static AUTO_CONFIG_STATUS_LIST_t    autoConfigStatusList;
static AUTO_CONFIG_REPORT_STATE_e   autoConfigResultState[MAX_CAMERA_SEARCH_SESSION];
static BOOL							autoConfigThreadExit = TRUE;
static AUTO_CONFIG_REQUEST_e		autoConfigRequest;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static VOIDPTR camAutoConfigThread(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
static void restoreDfltAutoConfigCamIpChangeReqParam(BOOL isFullDefault);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e autoConfigChangeCamIpAddr(CHARPTR brand, CHARPTR model, BOOL isOnvifSupp, UINT16 onvifPort, BOOL *isWaitNeeded, IP_ADDR_PARAM_t *networkParam);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e sendReqToUnconfiguredCamera(CAMERA_REQUEST_t * request, CHARPTR ipAddr, UINT16 httpPort, CHARPTR userName, CHARPTR password);
//-------------------------------------------------------------------------------------------------
static void httpAutoConfigChangeIpAddrCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t * dataInfo);
//-------------------------------------------------------------------------------------------------
static void httpAutoConfigRebootIpCamCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t * dataInfo);
//-------------------------------------------------------------------------------------------------
static BOOL checkCameraAlreadyAdded(AUTO_CONFIG_REQ_PARAM_t *pCameraAutoConfigList);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e autoConfigCheckAuth(CHARPTR brand , CHARPTR model,BOOL isOnvifSupp,UINT16 onvifPort,BOOL *isWaitNeeded);
//-------------------------------------------------------------------------------------------------
static void httpAutoCnfgDeviceInfoCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t * dataInfo);
//-------------------------------------------------------------------------------------------------
static 	void httpSetPasswdCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t * dataInfo);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   Init auto camera config info
 */
void InitAutoCameraConfig(void)
{
    UINT8 requestCount;

    autoConfigChangeIpRequest.changeCamIpAddr.cameraIndex = INVALID_CAMERA_INDEX;
    for(requestCount = 0; requestCount < MAX_URL_REQUEST; requestCount++)
    {
        autoConfigChangeIpRequest.changeCamIpAddr.http[requestCount] = INVALID_HTTP_HANDLE;
    }

    autoConfigChangeIpRequest.changeCamIpAddr.rtspHandle = INVALID_RTSP_HANDLE;
    memset(autoConfigChangeIpRequest.changeCamIpAddr.httpCallback, '\0', sizeof(autoConfigChangeIpRequest.changeCamIpAddr.httpCallback));
    autoConfigChangeIpRequest.changeCamIpAddr.httpCallback[CAM_REQ_CONTROL] = httpAutoConfigChangeIpAddrCb;
    autoConfigChangeIpRequest.changeCamIpAddr.httpCallback[CAM_REQ_REBOOT] = httpAutoConfigRebootIpCamCb;

    memset(autoConfigCheckAuthentication.checkAuthentication.httpCallback, '\0', sizeof(autoConfigCheckAuthentication.checkAuthentication.httpCallback));
    autoConfigCheckAuthentication.checkAuthentication.httpCallback[CAM_REQ_CONTROL] = httpAutoCnfgDeviceInfoCb;
    autoConfigCheckAuthentication.checkAuthentication.httpCallback[CAM_REQ_MEDIA] = httpSetPasswdCb;
    autoConfigChangeIpRequest.changeCamIpAddr.mediaCallback = NULL;
    autoConfigChangeIpRequest.isChangeIpRequest = FAIL;

    MUTEX_INIT(autoConfigChangeIpRequest.changeCamIpAddr.camReqFlagLock, NULL);
    autoConfigChangeIpRequest.changeCamIpAddr.camReqBusyF = FREE;
    autoConfigChangeIpRequest.changeCamIpAddr.numOfRequest = 0;
    memset(autoConfigChangeIpRequest.changeCamIpAddr.clientCb, '\0', sizeof(autoConfigChangeIpRequest.changeCamIpAddr.clientCb));
    memset(autoConfigCheckAuthentication.ipAddress, '\0', sizeof(autoConfigCheckAuthentication.ipAddress));
    memset(autoConfigCheckAuthentication.userName, '\0', MAX_CAMERA_USERNAME_WIDTH);
    memset(autoConfigCheckAuthentication.passWord, '\0', MAX_CAMERA_PASSWORD_WIDTH);
    MUTEX_INIT(autoConfigCheckAuthentication.checkAuthentication.camReqFlagLock,NULL);
    autoConfigCheckAuthentication.checkAuthentication.camReqBusyF = FREE;
    autoConfigCheckAuthentication.getDeviceInfoStatus = FAIL;

    autoConfigChangeIpRequest.newNetworkParam.ipAddrType = IP_ADDR_TYPE_MAX;
    memset(autoConfigChangeIpRequest.newNetworkParam.ipAddr, '\0', sizeof(autoConfigChangeIpRequest.newNetworkParam.ipAddr));
    autoConfigChangeIpRequest.newNetworkParam.prefixLen = 0;
    memset(autoConfigChangeIpRequest.newNetworkParam.gateway, '\0', sizeof(autoConfigChangeIpRequest.newNetworkParam.gateway));

    MUTEX_INIT(autoConfigChangeIpRequest.camIpChangeCondWaitMutex, NULL);
    MUTEX_LOCK(autoConfigChangeIpRequest.camIpChangeCondWaitMutex);
    pthread_cond_init(&autoConfigChangeIpRequest.camIpChangeCondWait, NULL);
    MUTEX_UNLOCK(autoConfigChangeIpRequest.camIpChangeCondWaitMutex);
    autoConfigChangeIpRequest.camIpChangeStatus = FAIL;

    autoConfigChangeIpRequest.httpPort = DFLT_HTTP_PORT;
    memset(autoConfigChangeIpRequest.ipAddress, '\0', sizeof(autoConfigChangeIpRequest.ipAddress));
    memset(autoConfigChangeIpRequest.userName, '\0', sizeof(autoConfigChangeIpRequest.userName));
    memset(autoConfigChangeIpRequest.passWord, '\0', sizeof(autoConfigChangeIpRequest.passWord));

    // session related parameter
    MUTEX_INIT(autoConfigSession.reqListEntryLock, NULL);
    MUTEX_INIT(autoConfigSession.requestStatusLock, NULL);
    autoConfigSession.reqListEntry = 0;
    autoConfigSession.requestStatus = INACTIVE;

    for(requestCount = 0; requestCount < MAX_CONFIGURE_CAMERA_LIST; requestCount++)
    {
        memset(autoConfigSession.cameraList, 0, sizeof(autoConfigSession.cameraList));
    }

    autoConfigStatusList.currentListIndex = 0;
    for(requestCount = 0; requestCount < MAX_CONFIGURE_STATUS_LIST; requestCount++)
    {
        autoConfigStatusList.autoConfigStatusReport[requestCount].autoConfigStatus = FAIL;
        autoConfigStatusList.autoConfigStatusReport[requestCount].cameraIndex = getMaxCameraForCurrentVariant();
        autoConfigStatusList.autoConfigStatusReport[requestCount].configFailReason = MAX_AUTO_CONFIG_FAIL_REASON;
        memset(autoConfigStatusList.autoConfigStatusReport[requestCount].detectedIpAddress, '\0',
               sizeof(autoConfigStatusList.autoConfigStatusReport[requestCount].detectedIpAddress));
        memset(autoConfigStatusList.autoConfigStatusReport[requestCount].changedIpAddress, '\0',
               sizeof(autoConfigStatusList.autoConfigStatusReport[requestCount].changedIpAddress));
    }

    for(requestCount = 0; requestCount < MAX_CAMERA_SEARCH_SESSION; requestCount++)
    {
        autoConfigResultState[requestCount] = AUTO_CONFIG_REPORT_CLEAR;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Start auto config process
 * @param   autoConfigList
 * @param   numOfResult
 * @param   sessionId
 * @return  Status
 */
NET_CMD_STATUS_e StartAutoConfigProcess(AUTO_CONFIG_REQ_PARAM_t *autoConfigList, UINT8 numOfResult, UINT8 sessionId)
{
    UINT8 index = 0, listIndex = 0;

    // find if requested ip is already in table
    MUTEX_LOCK(autoConfigSession.reqListEntryLock);
    for(index = 0; index < numOfResult; index++)
    {
        for(listIndex = 0; listIndex < autoConfigSession.reqListEntry; listIndex++)
        {
            if ((strcmp(autoConfigList[index].ipAddr, autoConfigSession.cameraList[listIndex].ipAddr) != STATUS_OK)
                    || (autoConfigList[index].onvifSupport != autoConfigSession.cameraList[listIndex].onvifSupport))
            {
                continue;
            }

            if (TRUE == autoConfigList[index].onvifSupport)
            {
                if (autoConfigList[index].onvifPort == autoConfigSession.cameraList[listIndex].onvifPort)
                {
                    break;
                }
            }
            else
            {
                if (autoConfigList[index].httpPort == autoConfigSession.cameraList[listIndex].httpPort)
                {
                    break;
                }
            }
        }

        if(listIndex >= autoConfigSession.reqListEntry)
        {
            // Add entry to list
            autoConfigSession.cameraList[autoConfigSession.reqListEntry] = autoConfigList[index];
            DPRINT(CAMERA_INTERFACE, "[index=%d], [brand=%s], [model=%s]", autoConfigSession.reqListEntry,
                    autoConfigSession.cameraList[autoConfigSession.reqListEntry].brand,
                    autoConfigSession.cameraList[autoConfigSession.reqListEntry].model);
            autoConfigSession.reqListEntry++;
        }
    }
    MUTEX_UNLOCK(autoConfigSession.reqListEntryLock);

    //if session not started, start thread else do nothing
    MUTEX_LOCK(autoConfigSession.requestStatusLock);
    if(autoConfigSession.requestStatus == ACTIVE)
    {
        MUTEX_UNLOCK(autoConfigSession.requestStatusLock);
        EPRINT(CAMERA_INTERFACE, "auto camera configuration already running");
        return CMD_SUCCESS;
    }

    // start thread
    autoConfigSession.requestStatus = ACTIVE;
    MUTEX_UNLOCK(autoConfigSession.requestStatusLock);

    // create the detached thread to start camera search process
    if (FAIL == Utils_CreateThread(NULL, camAutoConfigThread, &autoConfigResultState[sessionId], DETACHED_THREAD, AUTO_CONFIG_THREAD_STACK_SZ))
    {
        MUTEX_LOCK(autoConfigSession.requestStatusLock);
        autoConfigSession.requestStatus = INACTIVE;
        MUTEX_UNLOCK(autoConfigSession.requestStatusLock);
        return CMD_RESOURCE_LIMIT;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function sets auto config result state
 * @param   state
 * @param   sessionId
 */
void SetAutoConfigReportFlag(AUTO_CONFIG_REPORT_STATE_e state, UINT8 sessionId)
{
    if(sessionId < MAX_CAMERA_SEARCH_SESSION)
    {
        autoConfigResultState[sessionId] = state;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gets auto config result state
 * @param   sessionId
 * @return
 */
AUTO_CONFIG_REPORT_STATE_e GetAutoConfigReportFlag(UINT8 sessionId)
{
    if (sessionId >= MAX_CAMERA_SEARCH_SESSION)
    {
        return AUTO_CONFIG_REPORT_CLEAR;
    }

    return autoConfigResultState[sessionId];
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Auto camera configuration thread
 * @param   threadArg
 * @return
 */
static VOIDPTR camAutoConfigThread(VOIDPTR threadArg)
{
    UINT8                       currListServedNo = 0;
    UINT8                       loop = 0;
    IP_CAMERA_CONFIG_t          ipCamConfig[MAX_CAMERA];
    NET_CMD_STATUS_e            cmdResp = CMD_SUCCESS;
    UINT8                       freeIndex = 0;
    GENERAL_CONFIG_t            generalConfig;
    CAMERA_CONFIG_t             tempCameraCfg;
    IP_CAMERA_CONFIG_t          tempIpCameraCfg;
    LAN_CONFIG_t                lanConfig;
    CHAR                        eventDetail[MAX_EVENT_DETAIL_SIZE];
    UINT8                       endOctect = 0, ipOctect[4];
    STREAM_CONFIG_t             streamConfig;
    BOOL                        waitNeededAfterIpChange = FALSE;
    BOOL                        ipAlreadyAssignedLoop = FALSE;
    CHAR                        camIpAddr[IPV6_ADDR_LEN_MAX];
    UINT16                      onvifPort = DFLT_ONVIF_PORT;
    BOOL                        waitNeededAfterAuthCheck = FALSE;
    BOOL                        isRangeOver = FALSE;
    AUTO_CONFIG_REPORT_STATE_e  *pAutoConfigReportState = (AUTO_CONFIG_REPORT_STATE_e*)threadArg;
    UINT32                      subnetMask;
    IP_NW_ADDR_u                startIpAddr;
    IP_NW_ADDR_u                tIpAddr;
    IP_NW_ADDR_u                endIpAddr;

    ReadLan1Config(&lanConfig);
    MUTEX_LOCK(autoConfigSession.reqListEntryLock);

    while(TRUE)
    {
        cmdResp = CMD_SUCCESS;
        autoConfigThreadExit =  FALSE;
        DPRINT(CAMERA_INTERFACE, "entry: [total=%d], [current=%s]", autoConfigSession.reqListEntry, autoConfigSession.cameraList[currListServedNo].ipAddr);
        MUTEX_UNLOCK(autoConfigSession.reqListEntryLock);

        if(*pAutoConfigReportState < AUTO_CONFIG_REPORT_CLEAR)
        {
            autoConfigStatusList.autoConfigStatusReport[autoConfigStatusList.currentListIndex].autoConfigStatus = FAIL;
            autoConfigStatusList.autoConfigStatusReport[autoConfigStatusList.currentListIndex].configFailReason = AUTO_CONFIG_REQUEST_FAILED;
            snprintf(autoConfigStatusList.autoConfigStatusReport[autoConfigStatusList.currentListIndex].detectedIpAddress,
                    sizeof(autoConfigStatusList.autoConfigStatusReport[autoConfigStatusList.currentListIndex].detectedIpAddress),
                    "%s", autoConfigSession.cameraList[currListServedNo].ipAddr);
        }

        ReadGeneralConfig(&generalConfig);
        onvifPort = DFLT_ONVIF_PORT;
        if(CheckIfConfiguredLessMax(&freeIndex) == FALSE)
        {
            cmdResp = CMD_MAX_IP_CAMERA_CONFIGURED;
            if(*pAutoConfigReportState < AUTO_CONFIG_REPORT_CLEAR)
            {
                autoConfigStatusList.autoConfigStatusReport[autoConfigStatusList.currentListIndex].configFailReason = AUTO_CONFIG_NO_FREE_INDEX;
                MUTEX_LOCK(autoConfigSession.reqListEntryLock);
                for(loop = (currListServedNo + 1); loop < autoConfigSession.reqListEntry; loop++)
                {
                    autoConfigStatusList.currentListIndex = (autoConfigStatusList.currentListIndex + 1) % MAX_CONFIGURE_STATUS_LIST;
                    snprintf(autoConfigStatusList.autoConfigStatusReport[autoConfigStatusList.currentListIndex].detectedIpAddress,
                            sizeof(autoConfigStatusList.autoConfigStatusReport[autoConfigStatusList.currentListIndex].detectedIpAddress), "%s", autoConfigSession.cameraList[loop].ipAddr);
                    autoConfigStatusList.autoConfigStatusReport[autoConfigStatusList.currentListIndex].configFailReason = AUTO_CONFIG_NO_FREE_INDEX;
                }
                MUTEX_UNLOCK(autoConfigSession.reqListEntryLock);

                // to point to next Index of list
                autoConfigStatusList.currentListIndex = (autoConfigStatusList.currentListIndex + 1) % MAX_CONFIGURE_STATUS_LIST;
            }
            break;
        }

        if(autoConfigSession.cameraList[currListServedNo].camStatus == CAM_ADDED)
        {
            currListServedNo++;
            MUTEX_LOCK(autoConfigSession.reqListEntryLock);
            if(currListServedNo < autoConfigSession.reqListEntry)
            {
                continue;
            }
            MUTEX_UNLOCK(autoConfigSession.reqListEntryLock);
            break;
        }

        if (checkCameraAlreadyAdded(&autoConfigSession.cameraList[currListServedNo]) == FALSE)
        {
            // fill camera name
            memcpy(&tempCameraCfg, &DfltCameraCfg, sizeof(CAMERA_CONFIG_t));
            memcpy(&tempIpCameraCfg, &DfltIpCameraCfg, sizeof(IP_CAMERA_CONFIG_t));
            tempCameraCfg.camera = ENABLE;
            snprintf(autoConfigCheckAuthentication.ipAddress, sizeof(autoConfigCheckAuthentication.ipAddress), "%s", autoConfigSession.cameraList[currListServedNo].ipAddr);
            autoConfigCheckAuthentication.httpPort = autoConfigSession.cameraList[currListServedNo].httpPort;
            snprintf(autoConfigCheckAuthentication.brand, MAX_BRAND_NAME_LEN, "%s", autoConfigSession.cameraList[currListServedNo].brand);
            snprintf(autoConfigCheckAuthentication.model, MAX_MODEL_NAME_LEN, "%s", autoConfigSession.cameraList[currListServedNo].model);
            snprintf(autoConfigCheckAuthentication.userName, MAX_CAMERA_USERNAME_WIDTH, "%s", generalConfig.userName);
            snprintf(autoConfigCheckAuthentication.passWord, MAX_CAMERA_PASSWORD_WIDTH, "%s", generalConfig.password);
            onvifPort = autoConfigSession.cameraList[currListServedNo].onvifPort;
            autoConfigRequest = AUTO_CONFIG_DEVICE_INFO;
            autoConfigCheckAuthentication.getDeviceInfoStatus = FAIL;

            //Authentication purpose send device info url.
            cmdResp = autoConfigCheckAuth(autoConfigSession.cameraList[currListServedNo].brand, autoConfigSession.cameraList[currListServedNo].model,
                                          autoConfigSession.cameraList[currListServedNo].onvifSupport, onvifPort, &waitNeededAfterAuthCheck);

            if(generalConfig.retainIpAddresses == TRUE)
            {
                cmdResp = CMD_SUCCESS;
                if(autoConfigSession.cameraList[currListServedNo].camStatus == CAM_IDENTIFIED)
                {
                    snprintf(tempIpCameraCfg.brand, MAX_BRAND_NAME_LEN, "%s", autoConfigSession.cameraList[currListServedNo].brand);
                    snprintf(tempIpCameraCfg.model, MAX_MODEL_NAME_LEN, "%s", autoConfigSession.cameraList[currListServedNo].model);
                }
                else if(autoConfigSession.cameraList[currListServedNo].onvifSupport == TRUE)
                {
                    // else add as a onvif if onvif supported...
                    tempIpCameraCfg.onvifSupportF = autoConfigSession.cameraList[currListServedNo].onvifSupport;
                }
                else
                {
                    cmdResp = CMD_PROCESS_ERROR;
                    if(*pAutoConfigReportState >= AUTO_CONFIG_REPORT_CLEAR)
                    {
                        autoConfigStatusList.autoConfigStatusReport[autoConfigStatusList.currentListIndex].configFailReason = AUTO_CONFIG_REQUEST_FAILED;
                    }
                }
            }
            else
            {
                if (NMIpUtil_GetIpAddrFamily(generalConfig.autoConfigStartIp) != NMIpUtil_GetIpAddrFamily(autoConfigSession.cameraList[currListServedNo].ipAddr))
                {
                    cmdResp = CMD_PROCESS_ERROR;
                    autoConfigRequest = MAX_AUTO_CONFIG_REQUEST;

                    if(*pAutoConfigReportState < AUTO_CONFIG_REPORT_CLEAR)
                    {
                        autoConfigStatusList.autoConfigStatusReport[autoConfigStatusList.currentListIndex].configFailReason = AUTO_CONFIG_IP_FAMILY_MISMATCH;
                    }

                    EPRINT(CAMERA_INTERFACE, "Camera and IP Address range belong to different IP families");
                }

                if(cmdResp == CMD_SUCCESS)
                {
                    DPRINT(CAMERA_INTERFACE, "process of ip change: [ip=%s]", autoConfigSession.cameraList[currListServedNo].ipAddr);
                    ReadIpCameraConfig(ipCamConfig);

                    if (NM_IPADDR_FAMILY_V4 == NMIpUtil_GetIpAddrFamily(generalConfig.autoConfigStartIp))
                    {
                        autoConfigChangeIpRequest.newNetworkParam.ipAddrType = IP_ADDR_TYPE_IPV4;

                        // Prepare Ip change request
                        ParseIpAddressGetIntValue(generalConfig.autoConfigEndIp, ipOctect);
                        endOctect = ipOctect[3];
                        ParseIpAddressGetIntValue(generalConfig.autoConfigStartIp, ipOctect);

                        // fill gateway & subnetmask
                        snprintf(autoConfigChangeIpRequest.newNetworkParam.gateway, sizeof(autoConfigChangeIpRequest.newNetworkParam.gateway),
                                 "%s", lanConfig.ipv4.lan.gateway);
                        inet_pton(AF_INET, lanConfig.ipv4.lan.subnetMask, &subnetMask);
                        autoConfigChangeIpRequest.newNetworkParam.prefixLen = __builtin_popcount(subnetMask);
                    }
                    else
                    {
                        autoConfigChangeIpRequest.newNetworkParam.ipAddrType = IP_ADDR_TYPE_IPV6;
                        inet_pton(AF_INET6, generalConfig.autoConfigStartIp, &tIpAddr.ip6);
                        inet_pton(AF_INET6, generalConfig.autoConfigStartIp, &startIpAddr.ip6);
                        inet_pton(AF_INET6, generalConfig.autoConfigEndIp, &endIpAddr.ip6);
                        startIpAddr.ip6.s6_addr16[7] = ntohs(startIpAddr.ip6.s6_addr16[7]);
                        endIpAddr.ip6.s6_addr16[7] = ntohs(endIpAddr.ip6.s6_addr16[7]);

                        // fill gateway & subnetmask
                        snprintf(autoConfigChangeIpRequest.newNetworkParam.gateway, sizeof(autoConfigChangeIpRequest.newNetworkParam.gateway),
                                 "%s", lanConfig.ipv6.lan.gateway);
                        autoConfigChangeIpRequest.newNetworkParam.prefixLen = lanConfig.ipv6.lan.prefixLen;
                    }

                    snprintf(autoConfigChangeIpRequest.brand, sizeof(autoConfigChangeIpRequest.brand), "%s", autoConfigSession.cameraList[currListServedNo].brand);
                    snprintf(autoConfigChangeIpRequest.model, sizeof(autoConfigChangeIpRequest.model), "%s", autoConfigSession.cameraList[currListServedNo].model);
                    snprintf(autoConfigChangeIpRequest.ipAddress, sizeof(autoConfigChangeIpRequest.ipAddress), "%s", autoConfigSession.cameraList[currListServedNo].ipAddr);
                    snprintf(autoConfigChangeIpRequest.userName, sizeof(autoConfigChangeIpRequest.userName), "%s", generalConfig.userName);
                    snprintf(autoConfigChangeIpRequest.passWord, sizeof(autoConfigChangeIpRequest.passWord), "%s", generalConfig.password);
                    autoConfigChangeIpRequest.onvifPort = autoConfigSession.cameraList[currListServedNo].onvifPort;
                    autoConfigChangeIpRequest.onvifSupportF = autoConfigSession.cameraList[currListServedNo].onvifSupport;

                    do
                    {
                        // check if this prepaid ip is already in our camera configuartion, if yes, then skip this ip
                        ipAlreadyAssignedLoop = FALSE;
                        if (autoConfigChangeIpRequest.newNetworkParam.ipAddrType == IP_ADDR_TYPE_IPV4)
                        {
                            if((ipOctect[3] + autoConfigChangeIpRequest.ipChangeSuccessCnt) > endOctect)
                            {
                                // IP range is over !!!!!
                                cmdResp = CMD_PROCESS_ERROR;
                                isRangeOver = TRUE;
                                break;
                            }

                            snprintf(autoConfigChangeIpRequest.newNetworkParam.ipAddr, sizeof(autoConfigChangeIpRequest.newNetworkParam.ipAddr),
                                     "%d.%d.%d.%d", ipOctect[0], ipOctect[1], ipOctect[2], (ipOctect[3] + autoConfigChangeIpRequest.ipChangeSuccessCnt));
                        }
                        else
                        {
                            if((startIpAddr.ip6.s6_addr16[7] + autoConfigChangeIpRequest.ipChangeSuccessCnt) > endIpAddr.ip6.s6_addr16[7])
                            {
                                // IP range is over !!!!!
                                cmdResp = CMD_PROCESS_ERROR;
                                isRangeOver = TRUE;
                                break;
                            }

                            /* Get next ipv6 address in given range by incrimenting last group */
                            tIpAddr.ip6.s6_addr16[7] = ntohs(tIpAddr.ip6.s6_addr16[7]);
                            tIpAddr.ip6.s6_addr16[7] = startIpAddr.ip6.s6_addr16[7] + autoConfigChangeIpRequest.ipChangeSuccessCnt;
                            tIpAddr.ip6.s6_addr16[7] = htons(tIpAddr.ip6.s6_addr16[7]);

                            inet_ntop(AF_INET6, &tIpAddr.ip6, autoConfigChangeIpRequest.newNetworkParam.ipAddr, sizeof(autoConfigChangeIpRequest.newNetworkParam.ipAddr));
                        }

                        for(loop = 0; loop < getMaxCameraForCurrentVariant(); loop++)
                        {
                            if(GetCamIpAddress(loop, camIpAddr) == FALSE)
                            {
                                snprintf(camIpAddr, sizeof(camIpAddr), "%s", ipCamConfig[loop].cameraAddress);
                            }

                            if (strcmp(camIpAddr, autoConfigChangeIpRequest.newNetworkParam.ipAddr) == STATUS_OK)
                            {
                                autoConfigChangeIpRequest.ipChangeSuccessCnt++;
                                ipAlreadyAssignedLoop = TRUE;
                                break;
                            }
                        }
                    }
                    while(ipAlreadyAssignedLoop == TRUE);

                    //fail when range is finished
                    if((cmdResp != CMD_SUCCESS) && (isRangeOver))
                    {
                        EPRINT(CAMERA_INTERFACE, "ip range for auto configure exceeded");
                        isRangeOver = FALSE;

                        if(*pAutoConfigReportState < AUTO_CONFIG_REPORT_CLEAR)
                        {
                            autoConfigStatusList.autoConfigStatusReport[autoConfigStatusList.currentListIndex].configFailReason = AUTO_CONFIG_MAX_IP_ADDRESS_RANGE;
                            MUTEX_LOCK(autoConfigSession.reqListEntryLock);
                            for(loop = (currListServedNo + 1); loop < autoConfigSession.reqListEntry; loop++)
                            {
                                autoConfigStatusList.currentListIndex = (autoConfigStatusList.currentListIndex + 1) % MAX_CONFIGURE_STATUS_LIST;
                                snprintf(autoConfigStatusList.autoConfigStatusReport[autoConfigStatusList.currentListIndex].detectedIpAddress,
                                        sizeof(autoConfigStatusList.autoConfigStatusReport[autoConfigStatusList.currentListIndex].detectedIpAddress), "%s",
                                        autoConfigSession.cameraList[loop].ipAddr);
                                autoConfigStatusList.autoConfigStatusReport[autoConfigStatusList.currentListIndex].configFailReason = AUTO_CONFIG_MAX_IP_ADDRESS_RANGE;
                            }
                            MUTEX_UNLOCK(autoConfigSession.reqListEntryLock);
                            // to point to next Index of list
                            autoConfigStatusList.currentListIndex = (autoConfigStatusList.currentListIndex + 1) % MAX_CONFIGURE_STATUS_LIST;
                        }
                        break;
                    }

                    // All is right....go for Ip change
                    autoConfigChangeIpRequest.isChangeIpRequest = SUCCESS;
                    autoConfigChangeIpRequest.camIpChangeStatus = FAIL;
                    autoConfigRequest = AUTO_CONFIG_IP_CHANGE;

                    /* Wait for the complition of camera authentication */
                    if(waitNeededAfterAuthCheck == TRUE)
                    {
                        MUTEX_LOCK(autoConfigChangeIpRequest.camIpChangeCondWaitMutex);
                        pthread_cond_wait(&autoConfigChangeIpRequest.camIpChangeCondWait, &autoConfigChangeIpRequest.camIpChangeCondWaitMutex);
                        MUTEX_UNLOCK(autoConfigChangeIpRequest.camIpChangeCondWaitMutex);
                    }

                    MUTEX_LOCK(autoConfigCheckAuthentication.checkAuthentication.camReqFlagLock);
                    autoConfigCheckAuthentication.checkAuthentication.camReqBusyF = FREE;
                    MUTEX_UNLOCK(autoConfigCheckAuthentication.checkAuthentication.camReqFlagLock);

                    cmdResp = autoConfigChangeCamIpAddr(autoConfigSession.cameraList[currListServedNo].brand,
                            autoConfigSession.cameraList[currListServedNo].model, autoConfigSession.cameraList[currListServedNo].onvifSupport,
                            onvifPort, &waitNeededAfterIpChange, &autoConfigChangeIpRequest.newNetworkParam);
                }
            }

            switch(autoConfigRequest)
            {
                case AUTO_CONFIG_IP_CHANGE:
                {
                    if (cmdResp == CMD_NO_PRIVILEGE)
                    {
                        if(*pAutoConfigReportState < AUTO_CONFIG_REPORT_CLEAR)
                        {
                            autoConfigStatusList.autoConfigStatusReport[autoConfigStatusList.currentListIndex].configFailReason = AUTO_CONFIG_AUTH_FAIL;
                        }
                        break;
                    }

                    if(cmdResp != CMD_SUCCESS)
                    {
                        break;
                    }

                    if(waitNeededAfterIpChange == TRUE)
                    {
                        // wait till ip address change action performed
                        MUTEX_LOCK(autoConfigChangeIpRequest.camIpChangeCondWaitMutex);
                        pthread_cond_wait(&autoConfigChangeIpRequest.camIpChangeCondWait, &autoConfigChangeIpRequest.camIpChangeCondWaitMutex);
                        MUTEX_UNLOCK(autoConfigChangeIpRequest.camIpChangeCondWaitMutex);
                    }

                    MUTEX_LOCK(autoConfigChangeIpRequest.changeCamIpAddr.camReqFlagLock);
                    autoConfigChangeIpRequest.changeCamIpAddr.camReqBusyF = FREE;
                    MUTEX_UNLOCK(autoConfigChangeIpRequest.changeCamIpAddr.camReqFlagLock);

                    if((autoConfigChangeIpRequest.camIpChangeStatus == SUCCESS) && (autoConfigCheckAuthentication.getDeviceInfoStatus == SUCCESS))
                    {
                        restoreDfltAutoConfigCamIpChangeReqParam(FALSE);
                        DPRINT(CAMERA_INTERFACE, "ip changed success: [old=%s], [new=%s]",
                               autoConfigSession.cameraList[currListServedNo].ipAddr, autoConfigChangeIpRequest.newNetworkParam.ipAddr);

                        tempIpCameraCfg.onvifSupportF = autoConfigSession.cameraList[currListServedNo].onvifSupport;
                        if((autoConfigSession.cameraList[currListServedNo].brand[0] != '\0') &&
                                (autoConfigSession.cameraList[currListServedNo].camStatus == CAM_IDENTIFIED))
                        {
                            tempIpCameraCfg.onvifSupportF = FALSE;
                            snprintf(tempIpCameraCfg.brand, MAX_BRAND_NAME_LEN, "%s", autoConfigSession.cameraList[currListServedNo].brand);
                            snprintf(tempIpCameraCfg.model, MAX_MODEL_NAME_LEN, "%s", autoConfigSession.cameraList[currListServedNo].model);
                        }

                        snprintf(autoConfigSession.cameraList[currListServedNo].ipAddr,
                                 sizeof(autoConfigSession.cameraList[currListServedNo].ipAddr), "%s", autoConfigChangeIpRequest.newNetworkParam.ipAddr);
                        snprintf(tempIpCameraCfg.cameraAddress, sizeof(tempIpCameraCfg.cameraAddress), "%s", autoConfigChangeIpRequest.newNetworkParam.ipAddr);
                        (autoConfigChangeIpRequest.ipChangeSuccessCnt)++;
                    }
                    else
                    {
                        // iP CHANGE REQ FAIL
                        cmdResp = CMD_PROCESS_ERROR;
                        restoreDfltAutoConfigCamIpChangeReqParam(FALSE);
                    }
                }
                break;

                case AUTO_CONFIG_DEVICE_INFO:
                {
                    if(cmdResp == CMD_NO_PRIVILEGE)
                    {
                        if(*pAutoConfigReportState < AUTO_CONFIG_REPORT_CLEAR)
                        {
                            autoConfigStatusList.autoConfigStatusReport[autoConfigStatusList.currentListIndex].configFailReason = AUTO_CONFIG_AUTH_FAIL;
                        }
                        break;
                    }

                    if (cmdResp != CMD_SUCCESS)
                    {
                        break;
                    }

                    if(waitNeededAfterAuthCheck == TRUE)
                    {
                        MUTEX_LOCK(autoConfigChangeIpRequest.camIpChangeCondWaitMutex);
                        pthread_cond_wait(&autoConfigChangeIpRequest.camIpChangeCondWait, &autoConfigChangeIpRequest.camIpChangeCondWaitMutex);
                        MUTEX_UNLOCK(autoConfigChangeIpRequest.camIpChangeCondWaitMutex);
                    }

                    MUTEX_LOCK(autoConfigCheckAuthentication.checkAuthentication.camReqFlagLock);
                    autoConfigCheckAuthentication.checkAuthentication.camReqBusyF = FREE;
                    MUTEX_UNLOCK(autoConfigCheckAuthentication.checkAuthentication.camReqFlagLock);

                    if(autoConfigCheckAuthentication.getDeviceInfoStatus == SUCCESS)
                    {
                        restoreDfltAutoConfigCamIpChangeReqParam(FALSE);
                        DPRINT(CAMERA_INTERFACE, "get device info success: [ip=%s]", autoConfigSession.cameraList[currListServedNo].ipAddr);

                        tempIpCameraCfg.onvifSupportF = autoConfigSession.cameraList[currListServedNo].onvifSupport;
                        if((autoConfigSession.cameraList[currListServedNo].brand[0] != '\0') &&
                                (autoConfigSession.cameraList[currListServedNo].camStatus == CAM_IDENTIFIED))
                        {
                            tempIpCameraCfg.onvifSupportF = FALSE;
                        }

                        snprintf(tempIpCameraCfg.cameraAddress, MAX_CAMERA_ADDRESS_WIDTH, "%s", autoConfigSession.cameraList[currListServedNo].ipAddr);
                    }
                    else
                    {
                        cmdResp = CMD_PROCESS_ERROR;
                        restoreDfltAutoConfigCamIpChangeReqParam(FALSE);
                    }
                }
                break;

                default:
                {
                    /* Nothing to do */
                }
                break;
            }

            if (cmdResp == CMD_SUCCESS)
            {
                snprintf(tempCameraCfg.name, sizeof(tempCameraCfg.name), "%s", autoConfigSession.cameraList[currListServedNo].ipAddr);
                tempIpCameraCfg.httpPort = autoConfigSession.cameraList[currListServedNo].httpPort;
                tempIpCameraCfg.onvifPort = autoConfigSession.cameraList[currListServedNo].onvifPort;

                snprintf(tempIpCameraCfg.username, MAX_CAMERA_USERNAME_WIDTH ,"%s", generalConfig.userName );
                snprintf(tempIpCameraCfg.password, MAX_CAMERA_PASSWORD_WIDTH, "%s", generalConfig.password );

                DPRINT(CAMERA_INTERFACE, "add camera: [index=%d], [ip=%s]", freeIndex, tempIpCameraCfg.cameraAddress);
                if(*pAutoConfigReportState < AUTO_CONFIG_REPORT_CLEAR)
                {
                    autoConfigStatusList.autoConfigStatusReport[autoConfigStatusList.currentListIndex].autoConfigStatus = SUCCESS;
                    autoConfigStatusList.autoConfigStatusReport[autoConfigStatusList.currentListIndex].cameraIndex = (freeIndex + 1);
                    snprintf(autoConfigStatusList.autoConfigStatusReport[autoConfigStatusList.currentListIndex].changedIpAddress,
                            sizeof(autoConfigStatusList.autoConfigStatusReport[autoConfigStatusList.currentListIndex].changedIpAddress), "%s", tempIpCameraCfg.cameraAddress);
                }

                WriteSingleCameraConfig(freeIndex, &tempCameraCfg);
                WriteSingleIpCameraConfig(freeIndex, &tempIpCameraCfg);

                //check if stream profile parameter left retain
                if(generalConfig.retainDfltProfile == FALSE)
                {
                    snprintf(streamConfig.videoEncoding, MAX_ENCODER_NAME_LEN, "%s", generalConfig.videoEncoding);
                    snprintf(streamConfig.resolution, MAX_RESOLUTION_NAME_LEN, "%s", generalConfig.resolution);
                    streamConfig.framerate = generalConfig.framerate;
                    streamConfig.bitrateMode = generalConfig.bitrateMode;
                    streamConfig.bitrateValue = generalConfig.bitrateValue;
                    streamConfig.quality = generalConfig.quality;
                    streamConfig.gop = generalConfig.gop;
                    streamConfig.enableAudio = generalConfig.enableAudio;
                    streamConfig.mainStreamProfile = DFLT_MAIN_STREAM_PROFILE;

                    snprintf(streamConfig.videoEncodingSub, MAX_ENCODER_NAME_LEN, "%s", generalConfig.videoEncodingSub);
                    snprintf(streamConfig.resolutionSub, MAX_RESOLUTION_NAME_LEN, "%s", generalConfig.resolutionSub);
                    streamConfig.framerateSub = generalConfig.framerateSub;
                    streamConfig.bitrateModeSub = generalConfig.bitrateModeSub;
                    streamConfig.bitrateValueSub = generalConfig.bitrateValueSub;
                    streamConfig.qualitySub = generalConfig.qualitySub;
                    streamConfig.gopSub = generalConfig.gopSub;
                    streamConfig.enableAudioSub = generalConfig.enableAudioSub;
                    streamConfig.subStreamProfile = DFLT_SUB_STREAM_PROFILE;

                    WriteSingleStreamConfig(freeIndex, &streamConfig);
                }

                GET_EVENT_CONFIG_DETAIL(eventDetail, MAX_EVENT_DETAIL_SIZE, TBL_CAMERA_CFG);
                WriteEvent(LOG_USER_EVENT, LOG_CONFIG_CHANGE, eventDetail, "Auto Added", EVENT_CHANGE);
            }
        }
        else
        {
            EPRINT(CAMERA_INTERFACE, "camera already added in auto config, ignored it");
        }
        currListServedNo++;

        if(*pAutoConfigReportState < AUTO_CONFIG_REPORT_CLEAR)
        {
            autoConfigStatusList.currentListIndex = (autoConfigStatusList.currentListIndex + 1) % MAX_CONFIGURE_STATUS_LIST;
        }

        if (currListServedNo >= autoConfigSession.reqListEntry)
        {
            break;
        }

        MUTEX_LOCK(autoConfigSession.reqListEntryLock);
    }

    restoreDfltAutoConfigCamIpChangeReqParam(TRUE);
    autoConfigThreadExit = TRUE;

    if(*pAutoConfigReportState < AUTO_CONFIG_REPORT_CLEAR)
    {
        WriteEvent(LOG_SYSTEM_EVENT, LOG_AUTO_CFG_STS_REPORT, NULL, NULL, EVENT_ALERT);
    }

    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Restore default values in auto config camera info
 * @param   isFullDefault
 */
static void restoreDfltAutoConfigCamIpChangeReqParam(BOOL isFullDefault)
{
    UINT8 requestCount;

    autoConfigChangeIpRequest.changeCamIpAddr.cameraIndex = INVALID_CAMERA_INDEX;
    for(requestCount = 0; requestCount < MAX_URL_REQUEST; requestCount++)
    {
        autoConfigChangeIpRequest.changeCamIpAddr.http[requestCount] = INVALID_HTTP_HANDLE;
    }

    autoConfigChangeIpRequest.changeCamIpAddr.rtspHandle = INVALID_RTSP_HANDLE;
    autoConfigChangeIpRequest.changeCamIpAddr.numOfRequest = 0;
    autoConfigChangeIpRequest.camIpChangeStatus = FAIL;
    autoConfigCheckAuthentication.checkAuthentication.numOfRequest = 0;
    autoConfigCheckAuthentication.getDeviceInfoStatus = FAIL;
    autoConfigCheckAuthentication.isSetPassword = FAIL;
    autoConfigCheckAuthentication.checkAuthentication.cameraIndex = INVALID_CAMERA_INDEX;

    if(isFullDefault)
    {
        autoConfigChangeIpRequest.changeCamIpAddr.camReqBusyF = FREE;
        autoConfigChangeIpRequest.newNetworkParam.ipAddrType = IP_ADDR_TYPE_MAX;
        memset(autoConfigChangeIpRequest.newNetworkParam.ipAddr, '\0', sizeof(autoConfigChangeIpRequest.newNetworkParam.ipAddr));
        autoConfigChangeIpRequest.newNetworkParam.prefixLen = 0;
        memset(autoConfigChangeIpRequest.newNetworkParam.gateway, '\0', sizeof(autoConfigChangeIpRequest.newNetworkParam.gateway));

        autoConfigChangeIpRequest.httpPort = DFLT_HTTP_PORT;
        memset(autoConfigChangeIpRequest.ipAddress, '\0', sizeof(autoConfigChangeIpRequest.ipAddress));
        memset(autoConfigChangeIpRequest.userName, '\0', MAX_CAMERA_USERNAME_WIDTH);
        memset(autoConfigChangeIpRequest.passWord, '\0', MAX_CAMERA_PASSWORD_WIDTH);

        autoConfigChangeIpRequest.ipChangeSuccessCnt = 0;

        MUTEX_LOCK(autoConfigSession.requestStatusLock);
        autoConfigSession.requestStatus = INACTIVE;
        MUTEX_UNLOCK(autoConfigSession.requestStatusLock);
        autoConfigSession.reqListEntry = 0;

        for(requestCount = 0; requestCount < MAX_CONFIGURE_CAMERA_LIST; requestCount++)
        {
            memset(autoConfigSession.cameraList[requestCount].ipAddr, '\0', sizeof(autoConfigSession.cameraList[requestCount].ipAddr));
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   autoConfigCheckAuth
 * @param   brand
 * @param   model
 * @param   isOnvifSupp
 * @param   onvifPort
 * @param   isWaitNeeded
 * @return  Status
 */
static NET_CMD_STATUS_e autoConfigCheckAuth(CHARPTR brand, CHARPTR model, BOOL isOnvifSupp, UINT16 onvifPort, BOOL *isWaitNeeded)
{
    NET_CMD_STATUS_e	requestStatus = CMD_PROCESS_ERROR;
    ONVIF_RESP_STATUS_e	onvifRes;
    CAMERA_BRAND_e		brandNum;
    CAMERA_MODEL_e 		modelNum;

    do
    {
        MUTEX_LOCK(autoConfigCheckAuthentication.checkAuthentication.camReqFlagLock);
        if(autoConfigCheckAuthentication.checkAuthentication.camReqBusyF == BUSY)
        {
            MUTEX_UNLOCK(autoConfigCheckAuthentication.checkAuthentication.camReqFlagLock);
            requestStatus = CMD_CAM_REQUEST_IN_PROCESS;
            break;
        }
        autoConfigCheckAuthentication.checkAuthentication.camReqBusyF = BUSY;
        MUTEX_UNLOCK(autoConfigCheckAuthentication.checkAuthentication.camReqFlagLock);

        // for Authentication purpose check device Info url for common
        if ((GetBrandNum(brand, &brandNum) == FAIL) || (GetModelNum(brand, model, &modelNum) == FAIL))
        {
            break;
        }

        if (FAIL == GetBrandDeviceInfo(brandNum, modelNum, autoConfigCheckAuthentication.checkAuthentication.url,
                                       &autoConfigCheckAuthentication.checkAuthentication.numOfRequest))
        {
            EPRINT(CAMERA_INTERFACE, "fail to get device info: [brand=%d], [model=%d]", brandNum, modelNum);
            requestStatus = CMD_PROCESS_ERROR;
            break;
        }

        autoConfigCheckAuthentication.checkAuthentication.requestCount = 0;
        autoConfigCheckAuthentication.checkAuthentication.requestStatus = CMD_PROCESS_ERROR;

        requestStatus = sendReqToUnconfiguredCamera(&autoConfigCheckAuthentication.checkAuthentication,
                                                    autoConfigCheckAuthentication.ipAddress,autoConfigCheckAuthentication.httpPort,
                                                    autoConfigCheckAuthentication.userName,autoConfigCheckAuthentication.passWord);

        if(requestStatus != CMD_SUCCESS)
        {
            break;
        }

        *isWaitNeeded = TRUE;
        return CMD_SUCCESS;

    }while(0);

    *isWaitNeeded = FALSE;
    if (isOnvifSupp == TRUE)
    {
        onvifRes = OnvifUnconfigureCamReq(autoConfigCheckAuthentication.ipAddress, onvifPort,
                                          autoConfigCheckAuthentication.userName, autoConfigCheckAuthentication.passWord, ONVIF_GET_BRAND_MODEL, NULL);
        if (onvifRes == ONVIF_CMD_SUCCESS)
        {
            autoConfigCheckAuthentication.getDeviceInfoStatus = SUCCESS;
            requestStatus = CMD_SUCCESS;
        }
        else if ((onvifRes == ONVIF_CMD_TIMEOUT)|| (onvifRes == ONVIF_CMD_FAIL))
        {
            requestStatus = CMD_NO_PRIVILEGE;
        }
    }

    MUTEX_LOCK(autoConfigCheckAuthentication.checkAuthentication.camReqFlagLock);
    autoConfigCheckAuthentication.checkAuthentication.camReqBusyF = FREE;
    MUTEX_UNLOCK(autoConfigCheckAuthentication.checkAuthentication.camReqFlagLock);
    return requestStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   autoConfigChangeCamIpAddr
 * @param   brand
 * @param   model
 * @param   isOnvifSupp
 * @param   onvifPort
 * @param   isWaitNeeded
 * @param   netWorkParam
 * @return
 */
static NET_CMD_STATUS_e autoConfigChangeCamIpAddr(CHARPTR brand, CHARPTR model, BOOL isOnvifSupp, UINT16 onvifPort,
                                                  BOOL *isWaitNeeded, IP_ADDR_PARAM_t *networkParam)
{
    NET_CMD_STATUS_e	requestStatus = CMD_PROCESS_ERROR;
    ONVIF_RESP_STATUS_e	onvifRes;
    CAMERA_BRAND_e		brandNum;
    CAMERA_MODEL_e 		modelNum;

    *isWaitNeeded = FALSE;
    MUTEX_LOCK(autoConfigChangeIpRequest.changeCamIpAddr.camReqFlagLock);
    if(autoConfigChangeIpRequest.changeCamIpAddr.camReqBusyF == BUSY)
    {
        MUTEX_UNLOCK(autoConfigChangeIpRequest.changeCamIpAddr.camReqFlagLock);
        return CMD_CAM_REQUEST_IN_PROCESS;
    }
    autoConfigChangeIpRequest.changeCamIpAddr.camReqBusyF = BUSY;
    MUTEX_UNLOCK(autoConfigChangeIpRequest.changeCamIpAddr.camReqFlagLock);

    do
    {
        /* Give priority to brand model, if its not identified or ip change url not found then check if onvif support & change ip using onvif req */
        if ((GetBrandNum(brand, &brandNum) == FAIL) || (GetModelNum(brand, model, &modelNum) == FAIL))
        {
            break;
        }

        requestStatus = GetBrandModelReqestUrl(INVALID_CAMERA_INDEX, brandNum, modelNum, REQ_URL_CHANGE_CAMERA_ADDR,
                                               &autoConfigChangeIpRequest.changeCamIpAddr, networkParam, NULL, NULL);
        if(requestStatus != CMD_SUCCESS)
        {
            EPRINT(CAMERA_INTERFACE, "fail to change ip addr: [brand=%d], [model=%d], [status=%d]", brandNum, modelNum, requestStatus);
            requestStatus = CMD_PROCESS_ERROR;
            break;
        }

        autoConfigChangeIpRequest.changeCamIpAddr.requestCount = 0;
        autoConfigChangeIpRequest.changeCamIpAddr.requestStatus = CMD_PROCESS_ERROR;
        DPRINT(CAMERA_INTERFACE, "sending ip change request: [brand=%d], [model=%d]", brandNum, modelNum);
        requestStatus = sendReqToUnconfiguredCamera(&autoConfigChangeIpRequest.changeCamIpAddr, autoConfigChangeIpRequest.ipAddress,
                                                    autoConfigChangeIpRequest.httpPort, autoConfigChangeIpRequest.userName,
                                                    autoConfigChangeIpRequest.passWord);
        if (requestStatus != CMD_SUCCESS)
        {
            break;
        }

        *isWaitNeeded = TRUE;
        return CMD_SUCCESS;

    }while(0);

    if(isOnvifSupp == TRUE)
    {
        onvifRes = OnvifUnconfigureCamReq(autoConfigChangeIpRequest.ipAddress, onvifPort, autoConfigChangeIpRequest.userName,
                                          autoConfigChangeIpRequest.passWord, ONVIF_CHANGE_IP_ADDR, (VOIDPTR)&autoConfigChangeIpRequest.newNetworkParam);
        if((onvifRes == ONVIF_CMD_CAMERA_REBOOT_FAIL) || (onvifRes == ONVIF_CMD_SUCCESS))
        {
            autoConfigChangeIpRequest.camIpChangeStatus = SUCCESS;
            requestStatus = CMD_SUCCESS;
        }
        else
        {
            EPRINT(CAMERA_INTERFACE, "fail to change ip addr over onvif: [resp=%d]", onvifRes);
            if(onvifRes == ONVIF_CMD_AUTHENTICATION_FAIL)
            {
                requestStatus = CMD_NO_PRIVILEGE;
            }
            autoConfigChangeIpRequest.camIpChangeStatus = FAIL;
        }
    }

    MUTEX_LOCK(autoConfigChangeIpRequest.changeCamIpAddr.camReqFlagLock);
    autoConfigChangeIpRequest.changeCamIpAddr.camReqBusyF = FREE;
    MUTEX_UNLOCK(autoConfigChangeIpRequest.changeCamIpAddr.camReqFlagLock);
    return requestStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function sends commands to the camera over RTSP or HTTP whichever is selected.
 * @param   request
 * @param   ipAddr
 * @param   httpPort
 * @param   userName
 * @param   password
 * @return
 */
static NET_CMD_STATUS_e sendReqToUnconfiguredCamera(CAMERA_REQUEST_t *request, CHARPTR ipAddr, UINT16 httpPort, CHARPTR userName, CHARPTR password)
{
    HTTP_INFO_t 		httpInfo;
    CAM_REQUEST_TYPE_e 	requestType = request->url[request->requestCount].requestType;

    RESET_STR_BUFF(httpInfo.ipAddress);
    httpInfo.port = DFLT_HTTP_PORT;
    RESET_STR_BUFF(httpInfo.relativeUrl);
    RESET_STR_BUFF(httpInfo.fileForPutReq);
    httpInfo.sizeOfPutFile = 0;
    RESET_STR_BUFF(httpInfo.httpUsrPwd.username);
    RESET_STR_BUFF(httpInfo.httpUsrPwd.password);
    httpInfo.authMethod = MAX_AUTH_TYPE;
    httpInfo.maxConnTime = MAX_CONN_TIME;
    httpInfo.maxFrameTime = MAX_CONN_TIME;
    httpInfo.interface = MAX_HTTP_INTERFACE;
    httpInfo.userAgent = MAX_HTTP_USER_AGENT;
    httpInfo.contentType = HTTP_CONTENT_TYPE_MAX;

    /* Nothing to do for RTSP protocol */
    if (request->url[request->requestCount].protocolType == CAM_RTSP_PROTOCOL)
    {
        return CMD_SUCCESS;
    }

    /* We will process only http protocol */
    if (request->url[request->requestCount].protocolType != CAM_HTTP_PROTOCOL)
    {
        return CMD_PROCESS_ERROR;
    }

    // If IP is zero, that means it is not resolved, return disconnected
    if ((ipAddr[0] == '\0') || (userName[0] == '\0') || (password[0] == '\0'))
    {
        EPRINT(CAMERA_INTERFACE, "invld ip/username/password: [ip=%s], [username=%s], [password=%s]", ipAddr, userName, password);
        return  CMD_PROCESS_ERROR;
    }

    snprintf(httpInfo.ipAddress, MAX_CAMERA_ADDRESS_WIDTH ,"%s", ipAddr);
    snprintf(httpInfo.httpUsrPwd.username, MAX_CAMERA_USERNAME_WIDTH, "%s", userName);
    snprintf(httpInfo.httpUsrPwd.password, MAX_CAMERA_PASSWORD_WIDTH, "%s", password);
    httpInfo.authMethod = request->url[request->requestCount].authMethod;
    if(request->url[request->requestCount].httpRequestType == PUT_REQUEST)
    {
        snprintf(httpInfo.fileForPutReq, MAX_FILE_NAME_LENGTH, "%s", request->url[request->requestCount].fileForPutReq);
        httpInfo.sizeOfPutFile = request->url[request->requestCount].sizeOfPutFile;
        httpInfo.contentType = request->url[request->requestCount].httpContentType;
    }
    httpInfo.maxConnTime = HTTP_CONNECTION_TIMEOUT;
    httpInfo.maxFrameTime = HTTP_FRAME_TIMEOUT;
    httpInfo.port = httpPort;
    httpInfo.userAgent = CURL_USER_AGENT;
    httpInfo.interface = MAX_HTTP_INTERFACE;

    //retrieve relative url
    snprintf(httpInfo.relativeUrl, MAX_RELATIVE_URL_WIDTH, "%s", request->url[request->requestCount].relativeUrl);

    // Send request
    if(StartHttp(request->url[request->requestCount].httpRequestType, &httpInfo, request->httpCallback[requestType],
                 INVALID_CAMERA_INDEX, &(request->http[request->requestCount])) == FAIL)
    {
        EPRINT(CAMERA_INTERFACE, "http resource limit reach max: [cnt=%d], [type=%d], [url=http://%s:%d%s]", request->requestCount,
               request->url[request->requestCount].requestType, httpInfo.ipAddress, httpInfo.port, request->url[request->requestCount].relativeUrl);
        return CMD_RESOURCE_LIMIT;
    }

    DPRINT(CAMERA_INTERFACE, "request info: [cnt=%d], [handle=%d], [type=%d], [url=http://%s:%d%s]",
           request->requestCount, request->http[request->requestCount], request->url[request->requestCount].requestType,
            httpInfo.ipAddress, httpInfo.port, request->url[request->requestCount].relativeUrl);
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is call back of the control request sent to the camera. It takes action
 *          based on the response received
 * @param   httpHandle
 * @param   dataInfo
 */
static void httpAutoConfigChangeIpAddrCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t * dataInfo)
{
    UINT8               requestCount;
    NET_CMD_STATUS_e    requestStatus = CMD_PROCESS_ERROR;
    BOOL                emitSignal = FALSE;
    CAMERA_BRAND_e      brandNum;
    CAMERA_MODEL_e      modelNum;
    CHARPTR             mainData = NULL;
    CHAR                buf[20] = {'\0'};

    mainData = (CHARPTR)dataInfo->storagePtr;
    switch(dataInfo->httpResponse)
    {
        case HTTP_ERROR:
        {
            autoConfigChangeIpRequest.camIpChangeStatus = FAIL;
        }
        break;

        case HTTP_SUCCESS:
        {
            ParseStr(&mainData, 0x0D, buf, sizeof(buf));
            if(0 == strcmp(buf, MATRIX_CAM_HTTP_RESP_CODE_7_STR))
            {
                autoConfigCheckAuthentication.isSetPassword = SUCCESS;
            }
            else if(autoConfigCheckAuthentication.isSetPassword == FAIL)
            {
                autoConfigChangeIpRequest.camIpChangeStatus = SUCCESS;
                emitSignal = TRUE;
            }
        }
        break;

        case HTTP_CLOSE_ON_SUCCESS:
        {
            if(autoConfigCheckAuthentication.isSetPassword == SUCCESS)
            {
                if ((GetBrandNum(autoConfigCheckAuthentication.brand, &brandNum) == SUCCESS)
                        && (GetModelNum(autoConfigCheckAuthentication.brand, autoConfigCheckAuthentication.model, &modelNum) == SUCCESS))
                {
                    requestStatus = GetBrandModelReqestUrl(INVALID_CAMERA_INDEX, brandNum, modelNum, REQ_URL_SET_PASSWORD,
                                                           &autoConfigCheckAuthentication.checkAuthentication,
                                                           autoConfigCheckAuthentication.userName, autoConfigCheckAuthentication.passWord, NULL);
                    if (requestStatus == CMD_SUCCESS)
                    {
                        autoConfigCheckAuthentication.checkAuthentication.requestCount = 0;
                        autoConfigCheckAuthentication.checkAuthentication.requestStatus = CMD_PROCESS_ERROR;
                        requestStatus = sendReqToUnconfiguredCamera(&autoConfigCheckAuthentication.checkAuthentication,
                                                                    autoConfigCheckAuthentication.ipAddress, autoConfigCheckAuthentication.httpPort,
                                                                    autoConfigCheckAuthentication.userName, autoConfigCheckAuthentication.passWord);
                    }
                }
            }

            if(autoConfigChangeIpRequest.camIpChangeStatus == SUCCESS)
            {
                for (requestCount = 0; requestCount < autoConfigChangeIpRequest.changeCamIpAddr.numOfRequest; requestCount++)
                {
                    if (autoConfigChangeIpRequest.changeCamIpAddr.http[requestCount] == httpHandle)
                    {
                        requestCount++;
                        break;
                    }
                }

                if (requestCount < autoConfigChangeIpRequest.changeCamIpAddr.numOfRequest)
                {
                    autoConfigChangeIpRequest.changeCamIpAddr.http[(requestCount-1)] = INVALID_HTTP_HANDLE;
                    autoConfigChangeIpRequest.changeCamIpAddr.requestCount++;
                    if ((autoConfigChangeIpRequest.changeCamIpAddr.requestCount < autoConfigChangeIpRequest.changeCamIpAddr.numOfRequest)
                            && ((autoConfigChangeIpRequest.changeCamIpAddr.url[autoConfigChangeIpRequest.changeCamIpAddr.requestCount].requestType == CAM_REQ_CONTROL)
                                    || (autoConfigChangeIpRequest.changeCamIpAddr.url[autoConfigChangeIpRequest.changeCamIpAddr.requestCount].requestType == CAM_REQ_REBOOT)))
                    {
                        requestStatus = sendReqToUnconfiguredCamera(&autoConfigChangeIpRequest.changeCamIpAddr,
                                                                    autoConfigChangeIpRequest.ipAddress, autoConfigChangeIpRequest.httpPort,
                                                                    autoConfigChangeIpRequest.userName, autoConfigChangeIpRequest.passWord);
                    }
                }

                if (requestStatus != CMD_SUCCESS)
                {
                    emitSignal = TRUE;
                }
            }
            else
            {
                emitSignal = TRUE;
            }
        }
        break;

        case HTTP_CLOSE_ON_ERROR:
        {
            autoConfigChangeIpRequest.camIpChangeStatus = FAIL;
            emitSignal = TRUE;
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }

    if(emitSignal == TRUE)
    {
        MUTEX_LOCK(autoConfigChangeIpRequest.camIpChangeCondWaitMutex);
        pthread_cond_signal(&autoConfigChangeIpRequest.camIpChangeCondWait);
        MUTEX_UNLOCK(autoConfigChangeIpRequest.camIpChangeCondWaitMutex);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is call back of the control request sent to the camera. It takes action
 *          based on the response received
 * @param   httpHandle
 * @param   dataInfo
 */
static void httpAutoConfigRebootIpCamCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t * dataInfo)
{
    switch(dataInfo->httpResponse)
    {
        case HTTP_ERROR:
            autoConfigChangeIpRequest.camIpChangeStatus = FAIL;
            break;

        case HTTP_SUCCESS:
            autoConfigChangeIpRequest.camIpChangeStatus = SUCCESS;
            break;

        case HTTP_CLOSE_ON_SUCCESS:
        case HTTP_CLOSE_ON_ERROR:
            DPRINT(CAMERA_INTERFACE, "ip address changed successfully");
            MUTEX_LOCK(autoConfigChangeIpRequest.camIpChangeCondWaitMutex);
            pthread_cond_signal(&autoConfigChangeIpRequest.camIpChangeCondWait);
            MUTEX_UNLOCK(autoConfigChangeIpRequest.camIpChangeCondWaitMutex);
            break;

        default:
            break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is call back of the control request sent to the camera. It takes action
 *          based on the response received
 * @param   httpHandle
 * @param   dataInfo
 */
static void httpAutoCnfgDeviceInfoCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t * dataInfo)
{
    BOOL                emitSignal = FALSE;
    CAMERA_BRAND_e      brandNum;
    CAMERA_MODEL_e      modelNum;
    NET_CMD_STATUS_e    requestStatus = CMD_PROCESS_ERROR;
    CHARPTR             mainData = NULL;
    CHAR                buf[20] = {'\0'};

    mainData = (CHARPTR)dataInfo->storagePtr;
    switch((dataInfo->httpResponse))
    {
        case HTTP_SUCCESS:
        {
            ParseStr(&mainData, 0x0D, buf, sizeof(buf));
            if(0 == strcmp(buf, MATRIX_CAM_HTTP_RESP_CODE_7_STR))
            {
                autoConfigCheckAuthentication.isSetPassword = SUCCESS;
            }
            else
            {
                autoConfigCheckAuthentication.getDeviceInfoStatus = SUCCESS;
                emitSignal = TRUE;
            }
        }
        break;

        case HTTP_ERROR:
        {
            autoConfigCheckAuthentication.getDeviceInfoStatus = FAIL;
            EPRINT(CAMERA_INTERFACE, "fail to get device info");
            emitSignal = TRUE;
        }
        break;

        case HTTP_CLOSE_ON_SUCCESS:
        {
            if(autoConfigCheckAuthentication.isSetPassword == FAIL)
            {
                break;
            }

            if ((GetBrandNum(autoConfigCheckAuthentication.brand, &brandNum) == FAIL)
                    || (GetModelNum(autoConfigCheckAuthentication.brand, autoConfigCheckAuthentication.model, &modelNum) == FAIL))
            {
                break;
            }

            requestStatus = GetBrandModelReqestUrl(INVALID_CAMERA_INDEX, brandNum, modelNum, REQ_URL_SET_PASSWORD,
                                                   &autoConfigCheckAuthentication.checkAuthentication,
                                                   autoConfigCheckAuthentication.userName, autoConfigCheckAuthentication.passWord, NULL);
            if (requestStatus != CMD_SUCCESS)
            {
                break;
            }

            autoConfigCheckAuthentication.checkAuthentication.requestCount = 0;
            autoConfigCheckAuthentication.checkAuthentication.requestStatus = CMD_PROCESS_ERROR;
            requestStatus = sendReqToUnconfiguredCamera(&autoConfigCheckAuthentication.checkAuthentication,
                                                        autoConfigCheckAuthentication.ipAddress, autoConfigCheckAuthentication.httpPort,
                                                        autoConfigCheckAuthentication.userName, autoConfigCheckAuthentication.passWord);
        }
        break;

        case HTTP_CLOSE_ON_ERROR:
        {
            autoConfigCheckAuthentication.getDeviceInfoStatus = FAIL;
            emitSignal = TRUE;
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }

    if(emitSignal == TRUE)
    {
        MUTEX_LOCK(autoConfigChangeIpRequest.camIpChangeCondWaitMutex);
        pthread_cond_signal(&autoConfigChangeIpRequest.camIpChangeCondWait);
        MUTEX_UNLOCK(autoConfigChangeIpRequest.camIpChangeCondWaitMutex);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is call back of the control request sent to the camera. It takes action
 *          based on the response received
 * @param   httpHandle
 * @param   dataInfo
 */
static void httpSetPasswdCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t * dataInfo)
{
    BOOL    emitSignal = FALSE;
    BOOL    isWaitNeeded = FALSE;
    CHARPTR mainData = NULL;
    CHAR    buf[20] = {'\0'};

    mainData = (CHARPTR)dataInfo->storagePtr;
    switch(dataInfo->httpResponse)
    {
        case HTTP_ERROR:
        {
            if(autoConfigChangeIpRequest.isChangeIpRequest == SUCCESS)
            {
                autoConfigChangeIpRequest.camIpChangeStatus = FAIL;
                emitSignal = TRUE;
            }
            else
            {
                autoConfigCheckAuthentication.getDeviceInfoStatus = FAIL;
                emitSignal = TRUE;
            }
        }
        break;

        case HTTP_SUCCESS:
        {
            ParseStr(&mainData, 0x0D, buf, sizeof(buf));
            if(0 == strcmp(buf, MATRIX_CAM_HTTP_RESP_CODE_0_STR))
            {
                if(autoConfigChangeIpRequest.isChangeIpRequest == SUCCESS)
                {
                    autoConfigCheckAuthentication.isSetPassword = FAIL;
                    autoConfigCheckAuthentication.getDeviceInfoStatus = SUCCESS;

                    MUTEX_LOCK(autoConfigChangeIpRequest.changeCamIpAddr.camReqFlagLock);
                    autoConfigChangeIpRequest.changeCamIpAddr.camReqBusyF = FREE;
                    MUTEX_UNLOCK(autoConfigChangeIpRequest.changeCamIpAddr.camReqFlagLock);

                    autoConfigRequest = AUTO_CONFIG_IP_CHANGE;
                    autoConfigChangeCamIpAddr(autoConfigChangeIpRequest.brand, autoConfigChangeIpRequest.model,
                                              autoConfigChangeIpRequest.onvifSupportF, autoConfigChangeIpRequest.onvifPort,
                                              &isWaitNeeded, &autoConfigChangeIpRequest.newNetworkParam);
                }
                else
                {
                    autoConfigCheckAuthentication.getDeviceInfoStatus = SUCCESS;
                    emitSignal = TRUE;
                }
            }
            else
            {
                autoConfigCheckAuthentication.getDeviceInfoStatus = FAIL;
                emitSignal = TRUE;
            }
        }
        break;

        case HTTP_CLOSE_ON_ERROR:
        {
            if(autoConfigChangeIpRequest.isChangeIpRequest == SUCCESS)
            {
                autoConfigChangeIpRequest.camIpChangeStatus = FAIL;
                emitSignal = TRUE;
            }
            else
            {
                autoConfigCheckAuthentication.getDeviceInfoStatus = FAIL;
                emitSignal = TRUE;
            }
        }
        break;

        case HTTP_CLOSE_ON_SUCCESS:
        default:
        {
            /* Nothing to do */
        }
        break;
    }

    if(emitSignal == TRUE)
    {
        MUTEX_LOCK(autoConfigChangeIpRequest.camIpChangeCondWaitMutex);
        pthread_cond_signal(&autoConfigChangeIpRequest.camIpChangeCondWait);
        MUTEX_UNLOCK(autoConfigChangeIpRequest.camIpChangeCondWaitMutex);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Check camera is already configured or not in the system through same method
 * @param   pCameraAutoConfigList
 * @return  Returns TRUE if camera configured else returns FALSE
 */
static BOOL checkCameraAlreadyAdded(AUTO_CONFIG_REQ_PARAM_t *pCameraAutoConfigList)
{
    UINT8               loop;
    IP_CAMERA_CONFIG_t  ipCameraCfg[MAX_CAMERA];

    ReadIpCameraConfig(ipCameraCfg);
    for(loop = 0; loop < getMaxCameraForCurrentVariant(); loop++)
    {
        if ((strcmp(ipCameraCfg[loop].cameraAddress, pCameraAutoConfigList->ipAddr) != STATUS_OK)
                || (ipCameraCfg[loop].onvifSupportF != pCameraAutoConfigList->onvifSupport))
        {
            continue;
        }

        if (TRUE == ipCameraCfg[loop].onvifSupportF)
        {
            if (ipCameraCfg[loop].onvifPort == pCameraAutoConfigList->onvifPort)
            {
                return TRUE;
            }
        }
        else
        {
            if (ipCameraCfg[loop].httpPort == pCameraAutoConfigList->httpPort)
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   CheckIfConfiguredLessMax
 * @param   firstFreeIndex
 * @return
 */
BOOL CheckIfConfiguredLessMax(UINT8PTR firstFreeIndex)
{
    UINT8 					loop;
    IP_CAMERA_CONFIG_t		ipCameraCfg[MAX_CAMERA];
    UINT8					alreadyConfiguredCnt = 0;

    ReadIpCameraConfig(ipCameraCfg);
    for(loop = 0; loop < getMaxCameraForCurrentVariant(); loop++)
    {
        if(ipCameraCfg[loop].cameraAddress[0] == '\0')
        {
            *firstFreeIndex = loop;
            break;
        }

        alreadyConfiguredCnt++;
    }

    if (alreadyConfiguredCnt >= getMaxCameraForCurrentVariant())
    {
        return FALSE;
    }

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function get auto Config Result List
 * @param   sessionId
 * @return
 */
AUTO_CONFIG_STATUS_LIST_t *GetAutoConfigStatusReportData(UINT8 sessionId)
{
    AUTO_CONFIG_STATUS_LIST_t *autoConfigStatusListPtr = NULL;

    if (sessionId >= MAX_CAMERA_SEARCH_SESSION)
    {
        return NULL;
    }

    if (autoConfigResultState[sessionId] < AUTO_CONFIG_REPORT_CLEAR)
    {
        autoConfigStatusListPtr = &autoConfigStatusList;
    }

    if((autoConfigResultState[sessionId] == AUTO_CONFIG_REPORT_ABOUT_TO_CLEAR) && (autoConfigThreadExit == TRUE))
    {
        autoConfigResultState[sessionId] = AUTO_CONFIG_REPORT_CLEAR;
    }

    return autoConfigStatusListPtr;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
