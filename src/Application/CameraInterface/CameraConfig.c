//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		CameraConfig.c
@brief      Camera interface send user request to camera and returns response to the user. It requests
            for stream, image, PTZ control, etc. It monitors camera event through streaming or by
            sending commands periodically to camera.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "CameraConfig.h"
#include "CameraInterface.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define INVALID_CONFIG_HANDLE           (255)
#define INVALID_PROFILE_INDEX           (11)
#define PROFILE_CONFIG_THREAD_STACK_SZ  (3* MEGA_BYTE)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef UINT8   CONFIG_HANDLE;

typedef struct
{
	CAMERA_REQUEST_t 		camRequest;
	STREAM_CONFIG_t			streamConfig;
	CONFIG_HANDLE			configHandle;
	VIDEO_TYPE_e			streamType;
	UINT8					camIndex;
	UINT8					profileIndex;
	BOOL					camConfigRequestF;
	pthread_cond_t 			threadFlagCv;
	pthread_mutex_t 		condWaitFlagMutex;
	pthread_mutex_t			threadFlagMutex;
	CAMERA_CONFIG_CB		callBack;
	CHARPTR					configDataBufPtr;
	UINT32					configDataBufSize;
	BOOL					configDataBufStatus;

}CAMERA_CONFIG_REQUEST_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static CAMERA_CONFIG_REQUEST_t  configRequest[MAX_CAMERA * 2];

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void *getProfileConfigForCamera(void *configHandle);
//-------------------------------------------------------------------------------------------------
static void tcpParseBitrate(INT32 bitrate, BITRATE_VALUE_e *value);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   InitCameraConfig
 */
void InitCameraConfig(void)
{
    UINT8 loop;

    for(loop = 0; loop < (MAX_CAMERA * 2); loop++)
	{
		configRequest[loop].camConfigRequestF = FREE;
        configRequest[loop].configHandle = INVALID_CONFIG_HANDLE;
        memcpy(&configRequest[loop].streamConfig, &DfltCameraStreamCfg, sizeof(STREAM_CONFIG_t));
        MUTEX_INIT(configRequest[loop].threadFlagMutex, NULL);
        MUTEX_INIT(configRequest[loop].condWaitFlagMutex, NULL);
        pthread_cond_init(&configRequest[loop].threadFlagCv, NULL);
        configRequest[loop].camIndex = INVALID_CAMERA_INDEX;
		configRequest[loop].streamType = MAX_STREAM;
        configRequest[loop].profileIndex = INVALID_PROFILE_INDEX;
		configRequest[loop].callBack = NULL;
		configRequest[loop].configDataBufPtr = NULL;
		configRequest[loop].configDataBufSize = 0;
		configRequest[loop].configDataBufStatus = SUCCESS;
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API starts GetProfileConfigForCamera Thread
 * @param   request
 * @param   complexCamIndex
 * @param   profileIndex
 * @param   callback
 * @return  SUCCESS/FAIL
 */
BOOL GetCurrentCameraProfileConfig(CAMERA_REQUEST_t *request, UINT8 complexCamIndex,UINT8 profileIndex, CAMERA_CONFIG_CB callback)
{
    CAMERA_CONFIG_REQUEST_t *configReqInfo;
    UINT8                   camIndex = GET_STREAM_INDEX(complexCamIndex);
    VIDEO_TYPE_e            streamType = GET_STREAM_TYPE(complexCamIndex);

    if ((camIndex >= getMaxCameraForCurrentVariant()) || (request == NULL) || (streamType >= MAX_STREAM) || (callback == NULL))
	{
        EPRINT(CAMERA_INTERFACE, "invld params: [camera=%d], [streamType=%d]", camIndex, streamType);
        return FAIL;
	}

    if (GetCamEventStatus(camIndex, CONNECTION_FAILURE) == INACTIVE)
	{
        EPRINT(NETWORK_MANAGER, "camera not reachable: [camera=%d]", camIndex);
        return FAIL;
	}

    configReqInfo = &configRequest[complexCamIndex];
    MUTEX_LOCK(configReqInfo->threadFlagMutex);

    if(configReqInfo->camConfigRequestF == BUSY)
    {
        MUTEX_UNLOCK(configReqInfo->threadFlagMutex);
        EPRINT(CAMERA_INTERFACE, "session already running: [camera=%d], [streamType=%d]", camIndex, streamType);
        return FAIL;
    }

    if(configReqInfo->camConfigRequestF == FREE)
    {
        configReqInfo->camConfigRequestF = BUSY;
        configReqInfo->configHandle = complexCamIndex;
        MUTEX_UNLOCK(configReqInfo->threadFlagMutex);
    }
    else
    {
        MUTEX_UNLOCK(configReqInfo->threadFlagMutex);
        return FAIL;
    }

    configReqInfo->camIndex = camIndex;
    configReqInfo->streamType = streamType;
    configReqInfo->profileIndex = profileIndex;
    configReqInfo->callBack = callback;

    memcpy(&configReqInfo->camRequest,request,sizeof(CAMERA_REQUEST_t));
    ReadSingleStreamConfig(camIndex,&configReqInfo->streamConfig);

    if (FAIL == Utils_CreateThread(NULL, getProfileConfigForCamera, &configReqInfo->configHandle, DETACHED_THREAD, PROFILE_CONFIG_THREAD_STACK_SZ))
    {
        MUTEX_LOCK(configReqInfo->threadFlagMutex);
        configReqInfo->camConfigRequestF = FREE;
        configReqInfo->configHandle = INVALID_CONFIG_HANDLE;
        configReqInfo->profileIndex = INVALID_PROFILE_INDEX;
        configReqInfo->callBack = NULL;
        memcpy(&configReqInfo->streamConfig, &DfltCameraStreamCfg, sizeof(STREAM_CONFIG_t));
        ReadSingleStreamConfig(configReqInfo->camIndex,&configReqInfo->streamConfig);
        MUTEX_UNLOCK(configReqInfo->threadFlagMutex);
        return FAIL;
    }

    DPRINT(CAMERA_INTERFACE, "get profile config thread started: [camera=%d], [handle=%d]", camIndex, configRequest[complexCamIndex].configHandle);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API starts GetProfileConfigForCamera Thread
 * @param   sessionHandler
 * @return  None
 */
static void *getProfileConfigForCamera(void *sessionHandler)
{
	CAMERA_CONFIG_REQUEST_t 	*configCamProfileParam;
    UINT8 						loop;
	IP_CAMERA_CONFIG_t			ipCamConfig;
	CONFIG_HANDLE				configHandle = *((CONFIG_HANDLE *)sessionHandler);

	THREAD_START("CAM_PROF");

	configCamProfileParam = &configRequest[configHandle];
	configCamProfileParam->camRequest.requestCount = 0;

	for (loop = 0; loop < configCamProfileParam->camRequest.numOfRequest; loop++)
	{
		if (configCamProfileParam->camRequest.url[loop].requestType == CAM_REQ_CONFIG)
		{
			break;
		}
	}

    if (loop < configCamProfileParam->camRequest.numOfRequest)
	{
        configCamProfileParam->camRequest.requestCount = loop;
        ReadSingleIpCameraConfig(configCamProfileParam->camIndex, &ipCamConfig);
        ReadSingleStreamConfig(configCamProfileParam->camIndex, &configCamProfileParam->streamConfig);

        if((SendReqForConfig(&configCamProfileParam->camRequest, &ipCamConfig, configCamProfileParam->streamType) == SUCCESS))
		{
            MUTEX_LOCK(configCamProfileParam->condWaitFlagMutex);
			pthread_cond_wait (&configCamProfileParam->threadFlagCv, &configCamProfileParam->condWaitFlagMutex);
            MUTEX_UNLOCK(configCamProfileParam->condWaitFlagMutex);
            DPRINT(CAMERA_INTERFACE, "success response rcvd: [camera=%d], [streamType=%d]", configCamProfileParam->camIndex, configCamProfileParam->streamType);
		}

		if(configCamProfileParam->callBack != NULL)
		{
            DPRINT(CAMERA_INTERFACE, "main stream: [camera=%d], [streamType=%d], [videoEncoding=%s], [resolution=%s], [bitrateMode=%d], [bitrateValue=%d], [framerate=%d], [gop=%d], [quality=%d]",
                   configCamProfileParam->camIndex, configCamProfileParam->streamType,
                   configCamProfileParam->streamConfig.videoEncoding, configCamProfileParam->streamConfig.resolution,
                   configCamProfileParam->streamConfig.bitrateMode, configCamProfileParam->streamConfig.bitrateValue,
                   configCamProfileParam->streamConfig.framerate, configCamProfileParam->streamConfig.gop, configCamProfileParam->streamConfig.quality);

            DPRINT(CAMERA_INTERFACE, "sub stream: [camera=%d], [streamType=%d], [videoEncoding=%s], [resolution=%s], [bitrateMode=%d], [bitrateValue=%d], [framerate=%d], [gop=%d], [quality=%d]",
                   configCamProfileParam->camIndex, configCamProfileParam->streamType,
                   configCamProfileParam->streamConfig.videoEncodingSub, configCamProfileParam->streamConfig.resolutionSub,
                   configCamProfileParam->streamConfig.bitrateModeSub, configCamProfileParam->streamConfig.bitrateValueSub,
                   configCamProfileParam->streamConfig.framerateSub, configCamProfileParam->streamConfig.gopSub, configCamProfileParam->streamConfig.qualitySub);

            configCamProfileParam->callBack((configCamProfileParam->camIndex + (configCamProfileParam->streamType * getMaxCameraForCurrentVariant())),
                                            &configCamProfileParam->streamConfig, configCamProfileParam->profileIndex);
		}
	}

    MUTEX_LOCK(configCamProfileParam->threadFlagMutex);
    configCamProfileParam->camConfigRequestF = FREE;
    configCamProfileParam->configHandle = INVALID_CONFIG_HANDLE;
    configCamProfileParam->profileIndex = INVALID_PROFILE_INDEX;
    configCamProfileParam->callBack = NULL;
    configHandle = INVALID_CONFIG_HANDLE;
    memcpy(&configCamProfileParam->streamConfig, &DfltCameraStreamCfg, sizeof(STREAM_CONFIG_t));
    MUTEX_UNLOCK(configCamProfileParam->threadFlagMutex);
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is call back of the control request sent to the camera. It takes action
 *          based on the response received
 * @param   httpHandle
 * @param   dataInfo
 */
void HttpStreamConfigCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t * dataInfo)
{
	UINT8 						requestCount;
    UINT8						cameraIndex;
    CONFIG_HANDLE				configHandle = INVALID_CONFIG_HANDLE;
    CAMERA_CONFIG_REQUEST_t 	*configRequestHandler;
    PARSER_CONFIG_FUNC			*parserForCfg;
    VIDEO_TYPE_e 				streamType;
    IP_CAMERA_CONFIG_t			ipCameraCfg;

    if (NULL == dataInfo)
	{
		return;
	}

    cameraIndex = (UINT8)(GET_STREAM_INDEX(dataInfo->userData));
    streamType = (UINT8)(GET_STREAM_TYPE(dataInfo->userData));
    configHandle = (getMaxCameraForCurrentVariant() * streamType + cameraIndex);
	configRequestHandler = &configRequest[configHandle];

    switch(dataInfo->httpResponse)
	{
        // NOTE: here We Have Ignored HTTP_SUCCESS & HTTP_ERROR, because we want to continue stream though failure of parameter setting
        case HTTP_SUCCESS:
        {
            CAMERA_BRAND_e brand;
            CAMERA_MODEL_e model;

            requestCount = (UINT8)(dataInfo->userData & 0xFFFF);
            if ((dataInfo->frameSize == 0) || (dataInfo->storagePtr == NULL))
            {
                break;
            }

            GetCameraBrandModelNumber(cameraIndex, &brand, &model);
            parserForCfg = ParseCnfgFunc(brand);
            if (parserForCfg == NULL)
            {
                break;
            }

            if ((*parserForCfg)(model, dataInfo->frameSize, &configRequestHandler->streamConfig, dataInfo->storagePtr,
                                configRequestHandler->camIndex,configRequestHandler->streamType,configRequestHandler->profileIndex) == CMD_SUCCESS)
            {
                break;
            }

            EPRINT(CAMERA_INTERFACE, "failed to parse response: [camera=%d]", cameraIndex);
            if(configRequestHandler->streamType == MAIN_STREAM)
            {
                configRequestHandler->streamConfig.videoEncoding[0] = '\0';
                configRequestHandler->streamConfig.resolution[0] = '\0';
                configRequestHandler->streamConfig.framerate = 0;
                configRequestHandler->streamConfig.quality = 0;
                configRequestHandler->streamConfig.enableAudio = FALSE;
                configRequestHandler->streamConfig.bitrateMode = VBR;
                configRequestHandler->streamConfig.bitrateValue =BITRATE_2048;
                configRequestHandler->streamConfig.gop = 50;
                configRequestHandler->streamConfig.mainStreamProfile = configRequestHandler->profileIndex;
            }
            else
            {
                configRequestHandler->streamConfig.videoEncodingSub[0] = '\0';
                configRequestHandler->streamConfig.resolutionSub[0] = '\0';
                configRequestHandler->streamConfig.framerateSub = 0;
                configRequestHandler->streamConfig.qualitySub = 0;
                configRequestHandler->streamConfig.enableAudioSub = FALSE;
                configRequestHandler->streamConfig.bitrateModeSub = VBR;
                configRequestHandler->streamConfig.bitrateValueSub =BITRATE_2048;
                configRequestHandler->streamConfig.gopSub = 50;
                configRequestHandler->streamConfig.subStreamProfile = configRequestHandler->profileIndex;
            }
        }
        break;

        case HTTP_CLOSE_ON_ERROR:
        {
            EPRINT(CAMERA_INTERFACE, "config error: [camera=%d]", cameraIndex);
        }
        /* FALLS THROUGH */
        case HTTP_CLOSE_ON_SUCCESS:
        {
            for (requestCount = 0; requestCount < configRequestHandler->camRequest.numOfRequest; requestCount++)
            {
                if (configRequestHandler->camRequest.http[requestCount] == httpHandle)
                {
                    requestCount++;
                    break;
                }
            }

            if (requestCount < configRequestHandler->camRequest.numOfRequest)
            {
                configRequestHandler->camRequest.http[(requestCount-1)] = INVALID_HTTP_HANDLE;
                configRequestHandler->camRequest.requestCount++;

                // Check if Any Control URl is left to be sent
                if ((configRequestHandler->camRequest.requestCount < configRequestHandler->camRequest.numOfRequest)
                        && (configRequestHandler->camRequest.url[configRequestHandler->camRequest.requestCount].requestType == CAM_REQ_CONFIG))
                {
                    ReadSingleIpCameraConfig(cameraIndex, &ipCameraCfg);
                    if (SendReqForConfig(&configRequestHandler->camRequest, &ipCameraCfg, streamType) == FAIL)
                    {
                        MUTEX_LOCK(configRequestHandler->condWaitFlagMutex);
                        pthread_cond_signal (&configRequestHandler->threadFlagCv);
                        MUTEX_UNLOCK(configRequestHandler->condWaitFlagMutex);
                    }
                }
            }
            else
            {
                MUTEX_LOCK(configRequestHandler->condWaitFlagMutex);
                pthread_cond_signal (&configRequestHandler->threadFlagCv);
                MUTEX_UNLOCK(configRequestHandler->condWaitFlagMutex);
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
 * @brief   TcpGetStreamProfileCnfgCb
 * @param   tcpHandle
 * @param   dataInfo
 */
void TcpGetStreamProfileCnfgCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t * dataInfo)
{
    UINT8                       cameraIndex = GET_STREAM_INDEX(dataInfo->camIndex);
    VIDEO_TYPE_e                streamType = GET_STREAM_TYPE(dataInfo->camIndex);
    CONFIG_HANDLE               configHandle = (getMaxCameraForCurrentVariant() * streamType + cameraIndex);;
    CAMERA_CONFIG_REQUEST_t     *configRequestHandler = &configRequest[configHandle];
    CI_STREAM_PROFILE_CONFIG_t  *pStreamProfileConfig = dataInfo->ciCfgData;

    if (pStreamProfileConfig != NULL)
	{
		if(streamType == MAIN_STREAM)
		{
            snprintf(configRequestHandler->streamConfig.videoEncoding, MAX_ENCODER_NAME_LEN, "%s", pStreamProfileConfig->codec);
            snprintf(configRequestHandler->streamConfig.resolution, MAX_RESOLUTION_NAME_LEN, "%s", pStreamProfileConfig->resolution);
            configRequestHandler->streamConfig.framerate = (UINT8)pStreamProfileConfig->fps;
            configRequestHandler->streamConfig.bitrateMode = ConvertStringToIndex(pStreamProfileConfig->bitRateCtl, bitRateModeStrForMatrixIP, MAX_BITRATE_MODE);
            tcpParseBitrate(pStreamProfileConfig->bitRate, &(configRequestHandler->streamConfig.bitrateValue));
            configRequestHandler->streamConfig.quality = pStreamProfileConfig->imageQuality + 1;
            configRequestHandler->streamConfig.gop = (UINT8)pStreamProfileConfig->gop;
            configRequestHandler->streamConfig.enableAudio = pStreamProfileConfig->audio;
		}
		else
		{
            snprintf(configRequestHandler->streamConfig.videoEncodingSub, MAX_ENCODER_NAME_LEN, "%s", pStreamProfileConfig->codec);
            snprintf(configRequestHandler->streamConfig.resolutionSub, MAX_RESOLUTION_NAME_LEN, "%s", pStreamProfileConfig->resolution);
            configRequestHandler->streamConfig.framerateSub = pStreamProfileConfig->fps;
            configRequestHandler->streamConfig.bitrateModeSub = ConvertStringToIndex(pStreamProfileConfig->bitRateCtl, bitRateModeStrForMatrixIP, MAX_BITRATE_MODE);
            tcpParseBitrate(pStreamProfileConfig->bitRate,&(configRequestHandler->streamConfig.bitrateValueSub));
            configRequestHandler->streamConfig.qualitySub = pStreamProfileConfig->imageQuality + 1;
            configRequestHandler->streamConfig.gopSub = pStreamProfileConfig->gop;
            configRequestHandler->streamConfig.enableAudioSub = pStreamProfileConfig->audio;
		}

        FREE_MEMORY(dataInfo->ciCfgData);
	}

    MUTEX_LOCK(configRequestHandler->condWaitFlagMutex);
    pthread_cond_signal(&configRequestHandler->threadFlagCv);
    MUTEX_UNLOCK(configRequestHandler->condWaitFlagMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   tcpParseBitrate
 * @param   bitrate
 * @param   value
 */
static void tcpParseBitrate(INT32 bitrate, BITRATE_VALUE_e *value)
{
    UINT8   cnt;
    CHAR    data[15];
    CHAR    bitrateStr[15];

    snprintf(bitrateStr, sizeof(bitrateStr), "%d", bitrate);

	for(cnt = 0; cnt < MAX_BITRATE_VALUE; cnt ++)
	{
        snprintf(data,sizeof(data), "%d", bitRateValueForMatrixIP[cnt]);
		if(strcmp(bitrateStr, data) == STATUS_OK)
		{
			break;
		}
	}

	if(cnt < MAX_BITRATE_VALUE)
	{
		*value = cnt;
	}
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
