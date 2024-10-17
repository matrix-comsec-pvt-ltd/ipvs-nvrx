//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		MxOnvifClient.c
@brief      This Module handles all ONVIF communications with IP camera.Also Provides services to
            camera Interface Module. For every new request from camera interface module, a new
            thread is created to serve request which will send multiple ONVIF commands to camera to
            serve request and response is return to camera interface module via Callback.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "DebugLog.h"
#include "Utils.h"
#include "OnvifUtils.h"
#include "MxOnvifClient.h"
#include "OnvifCommand.h"
#include "CameraDatabase.h"
#include "NetworkController.h"
#include "TimeZone.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define MAX_ONVIF_SESSION                   (MAX_CAMERA * 5)
#define MAX_SERVICE_ADDR_LEN                128
#define MAX_RELATIVE_ADDR_LEN               64
#define MAX_TOKEN_SIZE                      64
#define OSD_TOKEN_MAX                       5

#define	HTTP_REQUEST_URL                    "http://%s:%d%s"
#define MAX_MULTICAST_MESSAGE_LEN           PATH_MAX
#define ONVIF_BITRATE_RANGE_STR             "BitrateRange"
#define ONVIF_BITRATE_RANGE_MIN_STR         "Min>"
#define ONVIF_BITRATE_RANGE_MAX_STR         "Max>"

#define ONVIF_ANALTICS_CELL_MOTION_STR      "CellMotionEngine"
#define ONVIF_ANALTICS_CELL_MOTION_DET_STR  "CellMotionDetector"
#define ONVIF_ANALTICS_ACTIVE_CELL_STR      "ActiveCells"
#define ONVIF_ANALTICS_SIMPLE_ITEM_MAX      4

//for codec index mapping with other module database
#define GET_VALID_CODEC_NUM(codecNum)       (UINT8)(codecNum - 1)
#define DEFAULT_PTZ_STEP                    40
#define FOCUS_SPEED_STEP                    5
#define FOCUS_STEP                          20
#define DEFAULT_IRIS_STEP                   2
#define MAX_EV_ADDITION_INFO_LEN            200 //addition info len for onvif camera info

#define GET_EVENT_PROPERTIES_ACTION         "http://www.onvif.org/ver10/events/wsdl/EventPortType/GetEventPropertiesRequest"
#define CREATE_PULL_PT_ACTION               "http://www.onvif.org/ver10/events/wsdl/EventPortType/CreatePullPointSubscriptionRequest"
#define SET_SYNC_PT_ACTION                  "http://www.onvif.org/ver10/events/wsdl/EventPortType/SetSynchronizationPointRequest"
#define PULL_MESSAGES_ACTION                "http://www.onvif.org/ver10/events/wsdl/PullPointSubscription/PullMessagesRequest"
#define RENEW_ACTION                        "http://www.onvif.org/ver10/events/wsdl/EventPortType/Renew"
#define UNSUBSCRIBE_ACTION                  "http://www.onvif.org/ver10/events/wsdl/EventPortType/Unsubscribe"
#define MEDIA2_WSDL_LOCATION                "http://www.onvif.org/ver20/media/wsdl"
#define ANALYTICS2_WSDL_LOCATION            "http://www.onvif.org/ver20/analytics/wsdl"

#define PULL_PT_SUB_TERMINATION_TIME        "PT20M"
#define PULL_MESSAGE_LIMIT                  1024
#define PULL_MESSAGE_TIMEOUT                "PT5S"
#define MAX_TAG_CNT                         3

#define MEDIA_MAIN_PROFILE                  "Main"
#define MEDIA_MAIN10_PROFILE                "Main10"
#define MEDIA_BASELINE_PROFILE              "Baseline"
#define MEDIA_EXTENDED_PROFILE              "Extended"
#define MEDIA_HIGH_PROFILE                  "High"
#define MAX_EVENT_STRING_LENGTH             300
#define MAX_TOPIC_NAME_SPACE_LEN            100
#define MAX_VALUE_STR                       15
#define MAX_TAG_LEN                         100

/* Set approx stack size. It is tunable based on requirement */
#define ONVIF_THRD_STACK_SIZE               (1*MEGA_BYTE)

//#################################################################################################
// @ENUMERATOR
//#################################################################################################
typedef enum
{
    NO_FUNCTION_SUPPORT = 0,
    RELATIVE_MOVE_SUPPORT,
    ABSOLUTE_MOVE_SUPPORT,
    CONTINUOUS_MOVE_SUPPORT,
    MAX_PTZ_SUPPORT_OPTIONS
}FUNCTION_SUPPORT_e;

typedef enum
{
    PAN_TILT_OPERATION = 0,
    ZOOM_OPERATION,
    MAX_PTZ_OPERATION
}PTZ_OPERATION_e;

//Event related enum
typedef enum
{
    ONVIF_MOTION_EVENT          = 0,
    ONVIF_SENSOR_EVENT          = 1,
    ONVIF_TAMPER_EVENT          = 2,
    ONVIF_LINE_CROSS_EVENT      = 3,
    ONVIF_OBJECT_INSIDE_EVENT   = 4,
    ONVIF_AUDIO_EXCEPTION_EVENT = 5,
    ONVIF_LOTERING_EVENT        = 6,
    ONVIF_OBJECT_CNT_EVENT      = 7,
    ONVIF_MAX_EVENT
}ONVIF_EVENT_e;

//#################################################################################################
// @STRUCT
//#################################################################################################
typedef struct
{
    CHAR 			 			profileToken[MAX_TOKEN_SIZE];
    CHAR 			 			videoEncoderConfigurationToken[MAX_TOKEN_SIZE];
    CHAR 			 			videoSourceToken[MAX_TOKEN_SIZE];
    UINT8 			 			defaultEncodingInterval[MAX_VIDEO_CODEC -1];
}ONVIF_MEDIA_PROFILE_INFO_t;

typedef struct
{
    ONVIF_MEDIA_PROFILE_INFO_t	mediaProfile[MAX_PROFILE_SUPPORT]; /* store media profile tags */
    MPEG4_PROFILE_e	 			mpeg4SupportedProfile;
    H264_PROFILE_e	 			h264SupportedProfile;
    CHAR 			 			audioEncoderConfigurationToken[MAX_TOKEN_SIZE];
    CHAR 			 			audioSourceToken[MAX_TOKEN_SIZE];
    CHAR 			 			mediaServiceAddr[MAX_SERVICE_ADDR_LEN];
    CHAR 			 			analyticsServiceAddr[MAX_SERVICE_ADDR_LEN];
    UINT8			 			initialQuality; /* Min quality can be 0 or 1 for Cameras */
    UINT8			 			rtspTranProtocol;
    CHAR 			 			relativeMediaServiceAddr[MAX_RELATIVE_ADDR_LEN];
    CHAR 			 			relativeAnalyticsServiceAddr[MAX_RELATIVE_ADDR_LEN];
    BOOL                        media2Support;      /* To identify if camera support media2 or not */
    BOOL                        analytics2Support;  /* To identify if camera support analytics2 or not */
}ONVIF_MEDIA_SERVICE_INFO_t;

typedef struct
{
    CHAR 			   			ptzServiceAddr[MAX_SERVICE_ADDR_LEN];
    CHAR 			   			profileToken[MAX_TOKEN_SIZE];
    CHAR 			   			ptzConfigurationToken[MAX_TOKEN_SIZE];
    float 			   			panStep;
    float 			   			tiltStep;
    float 			   			zoomStep;
    float 			   			pantiltSpeed;
    float 			   			zoomSpeed;
    FUNCTION_SUPPORT_e 			panTiltSupport;
    FUNCTION_SUPPORT_e 			zoomSupport;
    CHAR 			   			relativePtzServiceAddr[MAX_RELATIVE_ADDR_LEN];
}ONVIF_PTZ_SERVICE_INFO_t;

typedef struct
{
    FUNCTION_SUPPORT_e 			focusSupport;
    float 			   			focusSpeed;
    float 			   			focusStep;
}FOCUS_INFO_t;

typedef struct
{
    FUNCTION_SUPPORT_e 			irisSupport;
    float 			  			irisSpeed;
    float 			   			irisStep;
}IRIS_INFO_t;

typedef struct
{
    CHAR 		 				videoSourceToken[MAX_TOKEN_SIZE];
    CHAR		 				imagingServiceAddr[MAX_SERVICE_ADDR_LEN];
    FOCUS_INFO_t 				focusInfo;
    IRIS_INFO_t	 				irisInfo;
    CHAR		 				relativeImagingServiceAddr[MAX_RELATIVE_ADDR_LEN];
    BOOL                        isImgSettingSupport;

}ONVIF_IMAGING_SERVICE_t;

typedef struct
{
    UINT8 		  				tagAvailable;
    CHAR 		  				eventTag[MAX_TAG_CNT][MAX_TAG_LEN];
}ONVIF_EVENT_RESP_DET_t;

typedef struct
{
    CHAR						eventServiceUri[MAX_SERVICE_ADDR_LEN];
    CHAR 						evSubscribeRef[MAX_SERVICE_ADDR_LEN];
    CHAR				  		additionalInfo[MAX_EV_ADDITION_INFO_LEN];
    ONVIF_EVENT_RESP_DET_t 		onvifEvRespDet[ONVIF_MAX_EVENT];
}ONVIF_EVENT_SERVICE_INFO_t;

typedef struct
{
    CHAR 		 				videoAnalticsToken[MAX_TOKEN_SIZE];
}ONVIF_VIDEO_ANALYTIC_SERVICE_t;

typedef struct
{
    BOOL                            audioAvailable;
    //Time difference is required for password encryption and authentication
    TIME_DIFF_t                     timeDifference;
    CHAR                            deviceEntryPoint[MAX_SERVICE_ADDR_LEN];
    ONVIF_MEDIA_SERVICE_INFO_t      mediaServiceInfo;
    ONVIF_PTZ_SERVICE_INFO_t        ptzServiceInfo;
    ONVIF_IMAGING_SERVICE_t         imagingServiceInfo;
    CHAR                            relayToken[MAX_CAMERA_ALARM][MAX_TOKEN_SIZE];
    CHAR                            realtiveEntryPt[MAX_RELATIVE_ADDR_LEN];
    ONVIF_EVENT_SERVICE_INFO_t      eventServiceInfo;
    ONVIF_VIDEO_ANALYTIC_SERVICE_t	analyticsServiceInfo;
    UINT8                           maxSupportedProfile;
}ONVIF_CAMERA_INFO_t;

typedef struct
{
    BOOL			 			onvifSessionStatus;
    ONVIF_REQ_PARA_t 			onvifReqPara;
}ONVIF_CLIENT_INFO_t;

typedef struct
{
    STREAM_CODEC_TYPE_e 		codec;
    UINT8						fps;
    UINT8						quality;
    UINT8						gop;
    BITRATE_MODE_e				bitrateMode;
    BITRATE_VALUE_e				bitrateValue;
    UINT16 						resolHeight;
    UINT16 						resolWidth;
}ONVIF_ENC_CFG_t;

typedef struct
{
    ONVIF_CAM_SEARCH_CB_PARAM_t	cbParam;
    ONVIF_CAMERA_CB				callBack;
}ONVIF_DEVICE_DISCOVERY_INFO_t;

//#################################################################################################
// @STATIC_VARIABLES
//#################################################################################################
static ONVIF_CLIENT_INFO_t	onvifClientInfo[MAX_ONVIF_SESSION];
static pthread_mutex_t		onvifStatusLock;
static ONVIF_CAMERA_INFO_t	onvifCamInfo[MAX_CAMERA];

/* NONE, LEFT, RIGHT, STOP */
static const float panMultiplier[MAX_PTZ_PAN_OPTION] = {-1, 1};

/* NONE, UP, DOWN, STOP */
static const float tiltMultiplier[MAX_PTZ_TILT_OPTION] = {1, -1};

/* NONE, OUT, IN, STOP */
static const float zoomMultiplier[MAX_PTZ_ZOOM_OPTION] = {-1, 1};

/* NONE, FAR, NEAR, AUTO, STOP */
static const float focusMultiplier[MAX_FOCUS_OPTION] = {1, -1, 0};

/* NONE, CLOSE, OPEN, AUTO, STOP */
static const INT8 irisStepOption[MAX_IRIS_OPTION] = {1, -1, 0};

//Events Related Static Data
static const CHARPTR sensorTags[] =
{
    "Device/Trigger/AlarmIn",
    "Device/Trigger/DigitalInput",
    "Device/Trigger/DigitalInputs",
    "Device/Trigger/Relay",
    "Device/DigitalInput",
    "Device/IO/VirtualPort",
    "Device/IO/VirtualInput",
    "Device/IO/Port",
    "UserAlarm/AlarmDetector",
    "UserAlarm/GeneralAlarm",
    NULL
};

static const CHARPTR motionTags[] =
{
    "VideoAnalytics/MotionDetection",
    "VideoAnalytics/MotionDetector",
    "VideoAnalytics/Motion",
    "VideoSource/MotionAlarm",
    "RuleEngine/CellMotionDetector/Motion",
    NULL
};

static const CHARPTR tamperTags[] =
{
    "VideoSource/Tampering",
    "RuleEngine/TamperDetector/Tamper",
    NULL
};

static const CHARPTR lineCrossTags[] =
{
    "RuleEngine/LineDetector/Crossed",
    NULL
};

static const CHARPTR objectInsideTags[] =
{
    "RuleEngine/FieldDetector/ObjectsInside",
    NULL
};

static const CHARPTR audioExceptionTags[] =
{
    "AudioAnalytics/Audio/DetectedSound",
    NULL
};

static const CHARPTR loiteringTags[] =
{
    "RuleEngine/LoiteringDetector/ObjectIsLoitering",
    NULL
};

static const CHARPTR objectCntTags[] =
{
    "RuleEngine/CountAggregation/Counter",
    NULL
};

static const CHARPTR *availableTags[ONVIF_MAX_EVENT] =
{
    motionTags,
    sensorTags,
    tamperTags,
    lineCrossTags,
    objectInsideTags,
    audioExceptionTags,
    loiteringTags,
    objectCntTags,
};

static const CHARPTR onvifEvtStr[ONVIF_MAX_EVENT] =
{
    "MOTION_EVENT",
    "SENSOR_EVENT",
    "TAMPER_EVENT",
    "LINE_CROSS_EVENT",
    "OBJECT_INSIDE_EVENT",
    "AUDIO_EXCEPTION_EVENT",
    "LOITERING_EVENT",
    "OBJECT_CNT_EVENT"
};

static const CHARPTR dateFormatStrForOsd[MAX_DATE_FORMAT] =
{
    "dd/MM/yyyy",
    "MM/dd/yyyy",
    "yyyy/MM/dd",
    "dddd, MMMM dd, yyyy"
};

static const CHARPTR timeFormatStrForOsd[MAX_TIME_FORMAT] =
{
    "hh:mm:ss tt",
    "HH:mm:ss"
};

static const CHARPTR osdPosFormatStr[MAX_OSD_POS] =
{
    NULL,
    "UpperLeft",
    "UpperRight",
    "LowerLeft",
    "LowerRight"
};

static CHARPTR VEncoderName[MAX_VIDEO_CODEC] =
{
    "",
    "JPEG",
    "H264",
    "MPV4-ES",
    "",
    "H265"
};

static UINT8 OnvifIrLedModesMap[LED_MODE_MAX][2] =
{
    /* ONVIF --> MATRIX         MATRIX --> ONVIF */
    {LED_MODE_ON,           tt__IrCutFilterMode__AUTO},
    {LED_MODE_OFF,          tt__IrCutFilterMode__OFF},
    {LED_MODE_AUTO,         tt__IrCutFilterMode__ON},
};

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void *onvifClientThread(void *data);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e getEntryPointFromUnicastReq(CHARPTR deviceIpAddr, UINT8 camIndex);
//-------------------------------------------------------------------------------------------------
static BOOL multicastCameraSearch(CHARPTR entryPoint, ONVIF_DEVICE_DISCOVERY_INFO_t *searchParam);
//-------------------------------------------------------------------------------------------------
static void searchOnvifDevices(UINT16 sessionIndex);
//-------------------------------------------------------------------------------------------------
static void multicastDiscoveryMsg(NETWORK_PORT_e routePort, ONVIF_DEVICE_DISCOVERY_INFO_t *searchParam);
//-------------------------------------------------------------------------------------------------
static void recvOnvifDiscoveryResponse(INT32 *connFd, ONVIF_DEVICE_DISCOVERY_INFO_t *searchParam);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e getCameraDateTime(SOAP_t *soap, UINT8 camIndex, SOAP_USER_DETAIL_t *user);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e setCameraDateTime(SOAP_t *soap, UINT8 camIndex, SOAP_USER_DETAIL_t *user);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e getBrandModel(SOAP_t *soap, SOAP_USER_DETAIL_t *user, CHARPTR brandName, CHARPTR modelName, UINT8 camIndex);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e setUserPassword(SOAP_t *soap, SOAP_USER_DETAIL_t *user);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e getOnvifCameraCapability(UINT8 camIndex, SOAP_USER_DETAIL_t *user, SOAP_t *soap);
//-------------------------------------------------------------------------------------------------
static BOOL addResolutionToCamCapability(VIDEO_RESOLUTION_t *resolutionAvailable, UINT8 maxResolutionAvailable,
                                         UINT8PTR resolutionVal, UINT8 alreadyStored);
//-------------------------------------------------------------------------------------------------
static BOOL addResolutionToCamCapability2(VIDEO_RESOLUTION2_t *resolutionAvailable, UINT8 maxResolutionAvailable,
                                          UINT8PTR resolutionVal, UINT8 alreadyStored);
//-------------------------------------------------------------------------------------------------
static BOOL prepareMediaCapabilityFromEncoderConfigOptions(UINT8 camIndex, UINT8 profileNum,
                                                           GET_VIDEO_ENCODER_CONFIGURATION_OPTIONS_RESPONSE_t *videoEncoderConfigOptions);
//-------------------------------------------------------------------------------------------------
static BOOL prepareMedia2CapabilityFromEncoderConfigOptions(UINT8 camIndex, UINT8 profileNum,
                                                            GET_VIDEO_ENCODER_CONFIGURATION_OPTIONS_RESPONSE2_t *videoEncoderConfigOptions);
//-------------------------------------------------------------------------------------------------
static void fillOtherSupportedCapabiltyToCamera(CAPABILITY_TYPE *camCapability, GET_CAPABILITIES_RESPONSE_t *getCapabilitiesResponse);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e selectProfileForMediaService(SOAP_t *soap, SOAP_USER_DETAIL_t *user, BOOL ptzSupported, UINT8 camIndex);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e selectProfileForMedia2Service(SOAP_t *soap, SOAP_USER_DETAIL_t *user, BOOL ptzSupported, UINT8 camIndex);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e getOnvifProfileParam(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 profileNum,
                                                UINT8 camIndex, ONVIF_PROFILE_STREAM_INFO_t *profStreamInfo);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e getOnvifMedia2ProfileParam(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 profileNum, UINT8 camIndex,
                                                      ONVIF_PROFILE_STREAM_INFO_t *profStreamInfo);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e addAudioConfiguration(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 camIndex);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e addPtzConfiguration(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 camIndex);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e addImagingConfiguration(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 camIndex);
//-------------------------------------------------------------------------------------------------
static void setVideoProfile(SOAP_t *soap, ONVIF_ENC_CFG_t *onvifEncCfg, SOAP_USER_DETAIL_t *user, UINT8 camIndex, UINT8 profileIndex);
//-------------------------------------------------------------------------------------------------
static void setMedia2VideoProfile(SOAP_t *soap, ONVIF_ENC_CFG_t *onvifEncCfg, SOAP_USER_DETAIL_t *user, UINT8 camIndex, UINT8 profileIndex);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e getMediaUrl(SOAP_t *soap, SOAP_USER_DETAIL_t *user, VIDEO_TYPE_e streamType, STREAM_CONFIG_t *strmConfig,
                                       URL_REQUEST_t *urlReq, UINT8 camIndex, BOOL forceConfigOnCamera, UINT8 profileNum);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e getMedia2Url(SOAP_t *soap, SOAP_USER_DETAIL_t *user, VIDEO_TYPE_e streamType, STREAM_CONFIG_t *strmConfig,
                                        URL_REQUEST_t *urlReq, UINT8 camIndex, BOOL forceConfigOnCamera, UINT8 profileNum);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e getImageUrl(SOAP_t *soap, URL_REQUEST_t *urlReq,SOAP_USER_DETAIL_t *user, UINT8 camIndex);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e getImageUrl2(SOAP_t *soap, URL_REQUEST_t *urlReq,SOAP_USER_DETAIL_t *user, UINT8 camIndex);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e setRelayOutPut(SOAP_t *soap, UINT8 alarmIndex, BOOL action, SOAP_USER_DETAIL_t *user, UINT8 camIndex);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e setIris(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT16 sessionIndex, CAMERA_IRIS_e iris, UINT8 camIndex);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e ptzRelativeMove(SOAP_t *soap, SOAP_USER_DETAIL_t *userDeatil, UINT16 sessionIndex, PTZ_OPERATION_e moveType);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e ptzAbsoluteMove(SOAP_t *soap, SOAP_USER_DETAIL_t *userDeatil, UINT16 sessionIndex, PTZ_OPERATION_e moveType);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e ptzContinuousMove(SOAP_t *soap, SOAP_USER_DETAIL_t *userDeatil, UINT16 sessionIndex, PTZ_OPERATION_e moveType);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e gotoPresetCmd(SOAP_t *soap, SOAP_USER_DETAIL_t *userDeatil, UINT16 sessionIndex, PTZ_PRESET_CONFIG_t *pPtzPresetCfg);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e removePresetCmd(SOAP_t *soap, SOAP_USER_DETAIL_t *userDeatil, UINT16 sessionIndex, PTZ_PRESET_CONFIG_t *pPtzPresetCfg);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e setPresetCmd(SOAP_t *soap, SOAP_USER_DETAIL_t *userDeatil, UINT16 sessionIndex, PTZ_PRESET_CONFIG_t *pPtzPresetCfg);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e focusRelativeMove(SOAP_t *soap, SOAP_USER_DETAIL_t *userDeatil, UINT16 sessionIndex);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e focusAbsoluteMove(SOAP_t *soap,SOAP_USER_DETAIL_t *userDeatil, UINT16 sessionIndex);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e focusContinuousMove(SOAP_t *soap, SOAP_USER_DETAIL_t *userDeatil, UINT16 sessionIndex);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e focusAutoOperation(SOAP_t *soap, SOAP_USER_DETAIL_t *userDeatil, UINT16 sessionIndex);
//-------------------------------------------------------------------------------------------------
static BOOL getEventCapabilities(SOAP_t *soap, SOAP_USER_DETAIL_t *userDeatil, UINT8 camIndex);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e startOnvifEvent(SOAP_t *soap, SOAP_USER_DETAIL_t *userDeatil, UINT8 camIndex, UINT32 *pEvtSubTermTime);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e unSubscribeReq(SOAP_t *soap, SOAP_USER_DETAIL_t *userDeatil, UINT8 camIndex);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e getOnvifEventStatus(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 camIndex, BOOL *evtStatus);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e renewEventSubscription(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 camIndex, UINT32 *pEvtSubTermTime);
//-------------------------------------------------------------------------------------------------
static BOOL notifyOnvifEvent(UINT8 camIndex, PULL_MESSAGES_RESPONSE_t *pullMessagesResp, BOOL *evtStatus);
//-------------------------------------------------------------------------------------------------
static void fillWsa5HeaderInfo(UINT8 camIndex, CHARPTR action, SOAP_WSA5_t *wsa5Info);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e onvifSetOsdReq(SOAP_t *soap, SOAP_USER_DETAIL_t *userDeatil, UINT8 camIndex, OSD_PARAM_t *pOsdParam);
//-------------------------------------------------------------------------------------------------
#if 0
static ONVIF_RESP_STATUS_e onvifSetOsdReq2(SOAP_t *soap, SOAP_USER_DETAIL_t *userDeatil, UINT8 camIndex);
#endif
//-------------------------------------------------------------------------------------------------
static BITRATE_VALUE_e getBitrateIndex(INT32 bitRateValue);
//-------------------------------------------------------------------------------------------------
static BOOL parseBitrateMinMaxValue(CHARPTR sourceStr, UINT16PTR minValue, UINT16PTR maxValue);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e changeOnvifIpCameraAddress(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 camIndex, IP_ADDR_PARAM_t *networkParam);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e getOnvifMotionWindowConfig(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 camIndex, MOTION_BLOCK_METHOD_PARAM_t *motionGetWindowBuf);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e getOnvifMotionWindowConfig2(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 camIndex, MOTION_BLOCK_METHOD_PARAM_t *motionGetWindowBuf);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e setOnvifMotionWindowConfig(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 camIndex, MOTION_BLOCK_METHOD_PARAM_t *motionSetWindowBuf);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e setOnvifMotionWindowConfig2(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 camIndex, MOTION_BLOCK_METHOD_PARAM_t *motionSetWindowBuf);
//-------------------------------------------------------------------------------------------------
static BOOL convertMotionwinBufToActiveCells(CHARPTR motionSetWindowBuf, CHARPTR activeCellStr, UINT8 row, UINT8 column, const UINT16 activeCellLen);
//-------------------------------------------------------------------------------------------------
static BOOL parseBase64MotionWindow(CHARPTR source, UINT8 column, UINT8 row, CHARPTR outWindowResponse);
//-------------------------------------------------------------------------------------------------
static BOOL parseMedia2FrameRate(CHARPTR framrate,UINT8PTR min, UINT8PTR max);
//-------------------------------------------------------------------------------------------------
static BOOL parseEventTag(CHARPTR eventTag, struct soap_dom_element *dom, UINT8 len);
//-------------------------------------------------------------------------------------------------
static BOOL removeNameSpace(const CHAR *string, CHARPTR topic);
//-------------------------------------------------------------------------------------------------
static BOOL getValue(CHARPTR message, CHARPTR data, CHARPTR tag);
//-------------------------------------------------------------------------------------------------
static BOOL tagSequenceCompare(CHARPTR tag, CHARPTR evtTag);
//-------------------------------------------------------------------------------------------------
static BOOL getProperty(CHARPTR message, CHARPTR data);
//-------------------------------------------------------------------------------------------------
static void freeSoapObject(SOAP_t *soap);
//-------------------------------------------------------------------------------------------------
static const CHARPTR GetOnvifRespName(ONVIF_RESP_STATUS_e onvifReqStatus);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e getImagingCapability(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 camIndex, IMAGE_CAPABILITY_INFO_t *pImgCapsInfo);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e getImagingSetting(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 camIndex, IMAGE_CAPABILITY_INFO_t *pImgCapsInfo);
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e setImagingSetting(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 camIndex, IMAGE_CAPABILITY_INFO_t *pImgCapsInfo);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTION POINTER
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e (*ptzFuncPtr[MAX_PTZ_SUPPORT_OPTIONS])(SOAP_t *soapPtr, SOAP_USER_DETAIL_t *userDeatil, UINT16 sessionIndex, PTZ_OPERATION_e moveType) =
{
    NULL,
    ptzRelativeMove,
    ptzAbsoluteMove,
    ptzContinuousMove
};
//-------------------------------------------------------------------------------------------------
static ONVIF_RESP_STATUS_e (*focusFuncPtr[MAX_PTZ_SUPPORT_OPTIONS])(SOAP_t *soapPtr, SOAP_USER_DETAIL_t *userDeatil, UINT16 sessionIndex) =
{
    NULL,
    focusRelativeMove,
    focusAbsoluteMove,
    focusContinuousMove
};
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
/**
 * @brief InitOnvifClient - called by Main.c,will init all data varibles to their default values
 */
void InitOnvifClient(void)
{
    ONVIF_HANDLE    hOnvif;
    UINT8           camIndex;
    UINT8           profileIndex;

    MUTEX_INIT(onvifStatusLock, NULL);

    // Initialize all the ONVIF session as FREE
    for (hOnvif = 0; hOnvif < MAX_ONVIF_SESSION; hOnvif++)
    {
        onvifClientInfo[hOnvif].onvifSessionStatus = FREE;
    }

    // Initialize ONVIF model info
    for (camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
    {
        memset(&onvifCamInfo[camIndex].timeDifference, 0, sizeof(onvifCamInfo[camIndex].timeDifference));
        onvifCamInfo[camIndex].maxSupportedProfile = 0;

        snprintf(onvifCamInfo[camIndex].realtiveEntryPt, MAX_RELATIVE_ADDR_LEN, "/onvif/device_service");
        snprintf(onvifCamInfo[camIndex].mediaServiceInfo.relativeMediaServiceAddr, MAX_RELATIVE_ADDR_LEN, "/onvif/media_service");
        snprintf(onvifCamInfo[camIndex].mediaServiceInfo.relativeAnalyticsServiceAddr, MAX_RELATIVE_ADDR_LEN, "/onvif/analytics");
        snprintf(onvifCamInfo[camIndex].ptzServiceInfo.relativePtzServiceAddr, MAX_RELATIVE_ADDR_LEN, "/onvif/ptz_service");
        snprintf(onvifCamInfo[camIndex].imagingServiceInfo.relativeImagingServiceAddr, MAX_RELATIVE_ADDR_LEN, "/onvif/imaging_service");

        for (profileIndex = 0; profileIndex < MAX_PROFILE_SUPPORT; profileIndex++)
        {
            RESET_STR_BUFF(onvifCamInfo[camIndex].mediaServiceInfo.mediaProfile[profileIndex].profileToken);
            RESET_STR_BUFF(onvifCamInfo[camIndex].mediaServiceInfo.mediaProfile[profileIndex].videoEncoderConfigurationToken);
            RESET_STR_BUFF(onvifCamInfo[camIndex].mediaServiceInfo.mediaProfile[profileIndex].videoSourceToken);
        }

        onvifCamInfo[camIndex].mediaServiceInfo.media2Support = NO;
        onvifCamInfo[camIndex].mediaServiceInfo.analytics2Support = NO;

        onvifCamInfo[camIndex].imagingServiceInfo.isImgSettingSupport = NO;
        onvifCamInfo[camIndex].ptzServiceInfo.panTiltSupport = NO_FUNCTION_SUPPORT;
        onvifCamInfo[camIndex].imagingServiceInfo.focusInfo.focusSupport = NO_FUNCTION_SUPPORT;
        onvifCamInfo[camIndex].imagingServiceInfo.irisInfo.irisSupport = NO_FUNCTION_SUPPORT;
        RESET_STR_BUFF(onvifCamInfo[camIndex].ptzServiceInfo.profileToken);
        RESET_STR_BUFF(onvifCamInfo[camIndex].analyticsServiceInfo.videoAnalticsToken);
        onvifCamInfo[camIndex].audioAvailable = NO;

        //Events related initialization
        RESET_STR_BUFF(onvifCamInfo[camIndex].eventServiceInfo.eventServiceUri);
        RESET_STR_BUFF(onvifCamInfo[camIndex].eventServiceInfo.evSubscribeRef);
        RESET_STR_BUFF(onvifCamInfo[camIndex].eventServiceInfo.additionalInfo);
    }

    /* Migrate preset token configuration from local copy to system configuration */
    MigratePtzPresetOnvifTokenConfig();
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief StartOnvifClient  - will create onvif thread that will handle req/resp with camera.
 * @param onvifReqPara      - contains camera request information
 * @param handlePtr         - free session will be assigned
 * @return Fail             - if no free session present or failed to create thread
 */
BOOL StartOnvifClient(ONVIF_REQ_PARA_t *onvifReqPara, ONVIF_HANDLE *handlePtr)
{
    ONVIF_HANDLE sessionIndex;

    /* find first free session */
    MUTEX_LOCK(onvifStatusLock);
    for(sessionIndex = 0 ; sessionIndex < MAX_ONVIF_SESSION ; sessionIndex++)
    {
        if(onvifClientInfo[sessionIndex].onvifSessionStatus == FREE)
        {
            onvifClientInfo[sessionIndex].onvifSessionStatus = BUSY;
            break;
        }
    }
    MUTEX_UNLOCK(onvifStatusLock);

    if(sessionIndex >= MAX_ONVIF_SESSION)
    {
        EPRINT(ONVIF_CLIENT, "onvif free session not available: [camera=%d], [onvifReq=%s]", onvifReqPara->camIndex, GetOnvifReqName(onvifReqPara->onvifReq));
        return FAIL;
    }

    onvifClientInfo[sessionIndex].onvifReqPara = *onvifReqPara;
    onvifClientInfo[sessionIndex].onvifReqPara.sessionIndex = sessionIndex;

    if (FAIL == Utils_CreateThread(NULL, onvifClientThread, &onvifClientInfo[sessionIndex].onvifReqPara.sessionIndex,
                                   DETACHED_THREAD, ONVIF_THRD_STACK_SIZE))
    {
        EPRINT(ONVIF_CLIENT, "onvif free session not available: [camera=%d], [session=%d], [onvifReq=%s]",
               onvifReqPara->camIndex, sessionIndex, GetOnvifReqName(onvifReqPara->onvifReq));
        MUTEX_LOCK(onvifStatusLock);
        onvifClientInfo[sessionIndex].onvifSessionStatus = FREE;
        MUTEX_UNLOCK(onvifStatusLock);
        return FAIL;
    }

    /* Provide onvif handle */
    *handlePtr = sessionIndex;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief onvifClientThread : will handle onvif req and send response via callback
 * @param data              : Thread argument(Onvif request info)
 * @return
 */
static void *onvifClientThread(void *data)
{
    UINT16                      sessionIndex = *(UINT16PTR)data;
    SOAP_USER_DETAIL_t          user;
    ONVIF_RESPONSE_PARA_t       responseData;
    CHAR                        ipAddrForUrl[DOMAIN_NAME_SIZE_MAX];
    CHAR                        brandName[MAX_BRAND_NAME_LEN];
    CHAR                        modelName[MAX_MODEL_NAME_LEN];
    CHARPTR                     brandModelName[MAX_ONVIF_CAM_DETAIL] = {brandName, modelName};
    URL_REQUEST_t               urlReq;
    SOAP_t                      *soap = NULL;
    BOOL                        eventStatus[MAX_CAMERA_EVENT];
    UINT32                      outData = 0;
    UINT16                      complexCamIndex;
    UINT8                       realCamIndex;
    ONVIF_PROFILE_STREAM_INFO_t profileParam;

    THREAD_START_INDEX2("ONVIF_CLT", sessionIndex);

    complexCamIndex = onvifClientInfo[sessionIndex].onvifReqPara.camIndex;
    realCamIndex = GET_STREAM_INDEX(complexCamIndex);
    responseData.response = ONVIF_CMD_FAIL;

    //For onvif camera search command service address or authentication is not required
    if (onvifClientInfo[sessionIndex].onvifReqPara.onvifReq != ONVIF_SEARCH_DEVICES)
    {
        snprintf(user.name, sizeof(user.name), "%s", onvifClientInfo[sessionIndex].onvifReqPara.camPara.name);
        snprintf(user.pwd, sizeof(user.pwd), "%s", onvifClientInfo[sessionIndex].onvifReqPara.camPara.pwd);
        snprintf(user.ipAddr, sizeof(user.ipAddr), "%s", onvifClientInfo[sessionIndex].onvifReqPara.camPara.ipAddr);
        user.port = onvifClientInfo[sessionIndex].onvifReqPara.camPara.port;
        user.timeDiff = &onvifCamInfo[realCamIndex].timeDifference;

        PrepareIpAddressForUrl(onvifClientInfo[sessionIndex].onvifReqPara.camPara.ipAddr, ipAddrForUrl);
        snprintf(onvifCamInfo[realCamIndex].deviceEntryPoint, MAX_SERVICE_ADDR_LEN, HTTP_REQUEST_URL,
                 ipAddrForUrl, user.port,onvifCamInfo[realCamIndex].realtiveEntryPt);
        snprintf(onvifCamInfo[realCamIndex].mediaServiceInfo.mediaServiceAddr, MAX_SERVICE_ADDR_LEN, HTTP_REQUEST_URL,
                 ipAddrForUrl, user.port, onvifCamInfo[realCamIndex].mediaServiceInfo.relativeMediaServiceAddr);
        snprintf(onvifCamInfo[realCamIndex].mediaServiceInfo.analyticsServiceAddr, MAX_SERVICE_ADDR_LEN, HTTP_REQUEST_URL,
                 ipAddrForUrl, user.port, onvifCamInfo[realCamIndex].mediaServiceInfo.relativeAnalyticsServiceAddr);
        snprintf(onvifCamInfo[realCamIndex].ptzServiceInfo.ptzServiceAddr, MAX_SERVICE_ADDR_LEN, HTTP_REQUEST_URL,
                 ipAddrForUrl,user.port, onvifCamInfo[realCamIndex].ptzServiceInfo.relativePtzServiceAddr);
        snprintf(onvifCamInfo[realCamIndex].imagingServiceInfo.imagingServiceAddr, MAX_SERVICE_ADDR_LEN, HTTP_REQUEST_URL,
                 ipAddrForUrl, user.port, onvifCamInfo[realCamIndex].imagingServiceInfo.relativeImagingServiceAddr);

        /* All internal dynamic memory allocation will be handled by SOAP library itself. Use of soap_init() prone to memory leaks */
        soap = soap_new();
    }

    switch(onvifClientInfo[sessionIndex].onvifReqPara.onvifReq)
    {
        case ONVIF_GET_DATE_TIME:
            responseData.response = getCameraDateTime(soap, realCamIndex, &user);
            break;

        case ONVIF_GET_ENTRY_POINT:
            responseData.response = getEntryPointFromUnicastReq(user.ipAddr, realCamIndex);
            break;

        case ONVIF_GET_BRAND_MODEL:
            user.addr = onvifCamInfo[realCamIndex].deviceEntryPoint;
            responseData.response = getBrandModel(soap, &user, brandModelName[ONVIF_CAM_BRAND_NAME], brandModelName[ONVIF_CAM_MODEL_NAME], realCamIndex);
            responseData.data = brandModelName;
            break;

        case ONVIF_GET_CAPABILITY:
            responseData.response =	getOnvifCameraCapability(realCamIndex, &user, soap);
            break;

        case ONVIF_SET_DATE_TIME:
            responseData.response = setCameraDateTime(soap, realCamIndex, &user);
            break;

        case ONVIF_GET_MEDIA_URL:
            if (onvifCamInfo[realCamIndex].mediaServiceInfo.media2Support == NO)
            {
                responseData.response =	getMediaUrl(soap, &user, GET_STREAM_TYPE(complexCamIndex), &onvifClientInfo[sessionIndex].onvifReqPara.strmConfig,
                                                    &urlReq, realCamIndex, onvifClientInfo[sessionIndex].onvifReqPara.forceConfigOnCamera,
                                                    onvifClientInfo[sessionIndex].onvifReqPara.profileNum);
            }
            else
            {
                responseData.response =	getMedia2Url(soap, &user, GET_STREAM_TYPE(complexCamIndex), &onvifClientInfo[sessionIndex].onvifReqPara.strmConfig,
                                                     &urlReq, realCamIndex, onvifClientInfo[sessionIndex].onvifReqPara.forceConfigOnCamera,
                                                     onvifClientInfo[sessionIndex].onvifReqPara.profileNum);
            }
            responseData.data = (VOIDPTR)(&urlReq);
            break;

        case ONVIF_GET_SNAPSHOT_URL:
            if(onvifCamInfo[realCamIndex].mediaServiceInfo.media2Support == NO)
            {
                responseData.response =	getImageUrl(soap, &urlReq, &user, realCamIndex);
            }
            else
            {
                responseData.response =	getImageUrl2(soap, &urlReq, &user, realCamIndex);
            }
            responseData.data = (VOIDPTR)(&urlReq);
            break;

        case ONVIF_SET_PTZ:
            if (onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.zoom == MAX_PTZ_ZOOM_OPTION)
            {
                if (onvifCamInfo[realCamIndex].ptzServiceInfo.panTiltSupport == NO_FUNCTION_SUPPORT)
                {
                    responseData.response = ONVIF_CMD_FEATURE_NOT_SUPPORTED;
                }
                else
                {
                    responseData.response = ptzFuncPtr[onvifCamInfo[realCamIndex].ptzServiceInfo.panTiltSupport](soap, &user, sessionIndex, PAN_TILT_OPERATION);
                }
            }
            else
            {
                if (onvifCamInfo[realCamIndex].ptzServiceInfo.zoomSupport == NO_FUNCTION_SUPPORT)
                {
                    responseData.response = ONVIF_CMD_FEATURE_NOT_SUPPORTED;
                }
                else
                {
                    responseData.response = ptzFuncPtr[onvifCamInfo[realCamIndex].ptzServiceInfo.zoomSupport](soap, &user, sessionIndex, ZOOM_OPERATION);
                }
            }
            break;

        case ONVIF_EDIT_PRESET:
            if (onvifCamInfo[realCamIndex].ptzServiceInfo.panTiltSupport == NO_FUNCTION_SUPPORT)
            {
                responseData.response = ONVIF_CMD_FEATURE_NOT_SUPPORTED;
                break;
            }

            if (onvifClientInfo[sessionIndex].onvifReqPara.actionInfo.actionStatus == ADDED)
            {
                responseData.response = setPresetCmd(soap, &user, sessionIndex, &onvifClientInfo[sessionIndex].onvifReqPara.ptzPresetConfig);
            }
            else
            {
                responseData.response = removePresetCmd(soap, &user, sessionIndex, &onvifClientInfo[sessionIndex].onvifReqPara.ptzPresetConfig);
            }
            break;

        case ONVIF_GOTO_PRESET:
            if (onvifCamInfo[realCamIndex].ptzServiceInfo.panTiltSupport == NO_FUNCTION_SUPPORT)
            {
                responseData.response = ONVIF_CMD_FEATURE_NOT_SUPPORTED;
                break;
            }

            responseData.response = gotoPresetCmd(soap, &user, sessionIndex, &onvifClientInfo[sessionIndex].onvifReqPara.ptzPresetConfig);
            break;

        case ONVIF_SET_ALARM:
            responseData.response =	setRelayOutPut(soap, onvifClientInfo[sessionIndex].onvifReqPara.actionInfo.index,
                                                   onvifClientInfo[sessionIndex].onvifReqPara.actionInfo.actionStatus, &user, realCamIndex);
            responseData.data = (VOIDPTR)(&onvifClientInfo[sessionIndex].onvifReqPara.actionInfo.index);
            break;

        case ONVIF_SET_FOCUS:
            if (onvifClientInfo[sessionIndex].onvifReqPara.focusInfo == FOCUS_AUTO)
            {
                responseData.response = focusAutoOperation(soap, &user, sessionIndex);
                break;
            }

            if (onvifCamInfo[realCamIndex].imagingServiceInfo.focusInfo.focusSupport == NO_FUNCTION_SUPPORT)
            {
                responseData.response = ONVIF_CMD_FEATURE_NOT_SUPPORTED;
                break;
            }

            responseData.response = focusFuncPtr[onvifCamInfo[realCamIndex].imagingServiceInfo.focusInfo.focusSupport](soap, &user, sessionIndex);
            break;

        case ONVIF_SET_IRIS:
            responseData.response =	setIris(soap, &user, sessionIndex, onvifClientInfo[sessionIndex].onvifReqPara.irisInfo, realCamIndex);
            break;

        case ONVIF_START_EVENT:
            responseData.response =	startOnvifEvent(soap, &user, realCamIndex, &outData);
            responseData.data = (VOIDPTR)&outData;
            break;

        case ONVIF_GET_EVENT_NOTIFICATION:
            memset(eventStatus, UNKNOWN, MAX_CAMERA_EVENT);
            responseData.response =	getOnvifEventStatus(soap, &user, realCamIndex, eventStatus);
            responseData.data = (VOIDPTR)eventStatus;
            break;

        case ONVIF_RENEW_EVENT_REQ:
            responseData.response =	renewEventSubscription(soap, &user, realCamIndex, &outData);
            responseData.data = (VOIDPTR)&outData;
            break;

        case ONVIF_UNSUBSCRIBE_EVENT_REQ :
            responseData.response =	unSubscribeReq(soap, &user, realCamIndex);
            break;

        case ONVIF_SEARCH_DEVICES:
            searchOnvifDevices(sessionIndex);
            responseData.response = ONVIF_CMD_SUCCESS;
            break;

        case ONVIF_OSD_SET_REQ:
            responseData.response = onvifSetOsdReq(soap, &user, realCamIndex, onvifClientInfo[sessionIndex].onvifReqPara.pOnvifData);
            break;

        case ONVIF_PROFILE_CONFIG:
            if(onvifCamInfo[realCamIndex].mediaServiceInfo.media2Support == NO)
            {
                responseData.response = getOnvifProfileParam(soap, &user, onvifClientInfo[sessionIndex].onvifReqPara.profileNum, realCamIndex, &profileParam);
            }
            else
            {
                responseData.response = getOnvifMedia2ProfileParam(soap, &user, onvifClientInfo[sessionIndex].onvifReqPara.profileNum, realCamIndex, &profileParam);
            }
            responseData.data = (VOIDPTR)&profileParam;
            break;

        case ONVIF_GET_MOTION_WINDOW:
            if (NO == onvifCamInfo[realCamIndex].mediaServiceInfo.analytics2Support)
            {
                responseData.response = getOnvifMotionWindowConfig(soap, &user, realCamIndex, onvifClientInfo[sessionIndex].onvifReqPara.pOnvifData);
            }
            else
            {
                responseData.response = getOnvifMotionWindowConfig2(soap, &user, realCamIndex, onvifClientInfo[sessionIndex].onvifReqPara.pOnvifData);
            }
            responseData.data = onvifClientInfo[sessionIndex].onvifReqPara.pOnvifData;
            break;

        case ONVIF_SET_MOTION_WINDOW:
            if (NO == onvifCamInfo[realCamIndex].mediaServiceInfo.analytics2Support)
            {
                responseData.response = setOnvifMotionWindowConfig(soap, &user, realCamIndex, onvifClientInfo[sessionIndex].onvifReqPara.pOnvifData);
            }
            else
            {
                responseData.response = setOnvifMotionWindowConfig2(soap, &user, realCamIndex, onvifClientInfo[sessionIndex].onvifReqPara.pOnvifData);
            }
            break;

        case ONVIF_CHANGE_IP_ADDR:
            responseData.response = changeOnvifIpCameraAddress(soap, &user, realCamIndex, onvifClientInfo[sessionIndex].onvifReqPara.pOnvifData);
            break;

        case ONVIF_MEDIA_PROFILES_UPDATE:
            if(onvifCamInfo[realCamIndex].mediaServiceInfo.media2Support == NO)
            {
                responseData.response = selectProfileForMediaService(soap, &user, NO, realCamIndex);
            }
            else
            {
                responseData.response = selectProfileForMedia2Service(soap, &user, NO, realCamIndex);
            }
            break;

        case ONVIF_GET_IMAGING_CAPABILITY:
            responseData.response = getImagingCapability(soap, &user, realCamIndex, onvifClientInfo[sessionIndex].onvifReqPara.pOnvifData);
            break;

        case ONVIF_GET_IMAGING_SETTING:
            responseData.response = getImagingSetting(soap, &user, realCamIndex, onvifClientInfo[sessionIndex].onvifReqPara.pOnvifData);
            break;

        case ONVIF_SET_IMAGING_SETTING:
            responseData.response = setImagingSetting(soap, &user, realCamIndex, onvifClientInfo[sessionIndex].onvifReqPara.pOnvifData);
            break;

        default:
            break;
    }

    if (onvifClientInfo[sessionIndex].onvifReqPara.onvifReq != ONVIF_SEARCH_DEVICES)
    {
        /* Free soap object memory */
        freeSoapObject(soap);
    }

    if (onvifClientInfo[sessionIndex].onvifReqPara.onvifReq < ONVIF_REQUEST_MAX)
    {
        // exclude ONVIF_GET_EVENT_NOTIFICATION due to frequent debugs
        if((responseData.response < ONVIF_CMD_RESP_STATUS_MAX) &&
                ((onvifClientInfo[sessionIndex].onvifReqPara.onvifReq != ONVIF_GET_EVENT_NOTIFICATION) || (responseData.response != ONVIF_CMD_SUCCESS)))
        {
            DPRINT(ONVIF_CLIENT, "onvif cmd status: [camera=%d], [request=%s], [response=%s]", complexCamIndex,
                   GetOnvifReqName(onvifClientInfo[sessionIndex].onvifReqPara.onvifReq), GetOnvifRespName(responseData.response));
        }

        //send response back to calling module via registered callback
        if (onvifClientInfo[sessionIndex].onvifReqPara.onvifCallback != NULL)
        {
            responseData.cameraIndex = complexCamIndex;
            responseData.requestType = onvifClientInfo[sessionIndex].onvifReqPara.onvifReq;
            onvifClientInfo[sessionIndex].onvifReqPara.onvifCallback(&responseData);
        }
    }

    MUTEX_LOCK(onvifStatusLock);
    onvifClientInfo[sessionIndex].onvifSessionStatus = FREE;
    MUTEX_UNLOCK(onvifStatusLock);
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief multicastCameraSearch : camera search callback for multicast request, it will parse response to
 *                                get HTTP port & IP
 * @param entryPoint            : Entry point URL Sent by Camera onvif server
 * @param searchParam           : camera search parameters
 * @return
 */
static BOOL multicastCameraSearch(CHARPTR entryPoint, ONVIF_DEVICE_DISCOVERY_INFO_t *searchParam)
{
    UINT16              port = DFLT_ONVIF_PORT;
    CHARPTR             restUrl = entryPoint;
    CHARPTR             urlStr;
    CHAR                ipAddrStr[IPV6_ADDR_LEN_MAX];
    BOOL                ipv6Enabled;
    struct in6_addr     ipv6Addr;
    NM_IpAddrFamily_e   ipAddrFamily;

    searchParam->cbParam.ipv4Addr[0] = '\0';
    searchParam->cbParam.ipv6Addr[IPV6_ADDR_GLOBAL][0] = '\0';
    searchParam->cbParam.ipv6Addr[IPV6_ADDR_LINKLOCAL][0] = '\0';

    ipv6Enabled = IsIpv6Enabled();

    /* entryPoint example : http://192.168.101.251/onvif/device_service http://[6fff::55]/onvif/device_service */
    while ((urlStr = strtok_r(NULL, " ", &restUrl)))
    {
        /* Break loop if all addresses are found */
        if ((searchParam->cbParam.ipv4Addr[0] != '\0') &&
                (searchParam->cbParam.ipv6Addr[IPV6_ADDR_GLOBAL][0] != '\0') && (searchParam->cbParam.ipv6Addr[IPV6_ADDR_LINKLOCAL][0] != '\0'))
        {
            break;
        }

        if (FAIL == GetIpAddrAndPortFromUrl(urlStr, "http://", ipAddrStr, &port))
        {
            continue;
        }

        ipAddrFamily = NMIpUtil_GetIpAddrFamily(ipAddrStr);
        if (NM_IPADDR_FAMILY_INVALID == ipAddrFamily)
        {
            WPRINT(ONVIF_CLIENT, "Invalid ip addr family found in onvif device discovery response [ip=%s]", ipAddrStr);
            continue;
        }

        if (NM_IPADDR_FAMILY_V6 == ipAddrFamily)
        {
            /* If ipv6 is disabled skip ipv6 address */
            if (ipv6Enabled == FALSE)
            {
                continue;
            }

            inet_pton(AF_INET6, ipAddrStr, &ipv6Addr);

            if (IN6_IS_ADDR_LINKLOCAL(&ipv6Addr))
            {
                /* Store link local ipv6 address if entry is empty */
                if (searchParam->cbParam.ipv6Addr[IPV6_ADDR_LINKLOCAL][0] == '\0')
                {
                    snprintf(searchParam->cbParam.ipv6Addr[IPV6_ADDR_LINKLOCAL], IPV6_ADDR_LEN_MAX, "%s", ipAddrStr);
                }
            }
            else
            {
                /* Store global ipv6 address if entry is empty */
                if (searchParam->cbParam.ipv6Addr[IPV6_ADDR_GLOBAL][0] == '\0')
                {
                    snprintf(searchParam->cbParam.ipv6Addr[IPV6_ADDR_GLOBAL], IPV6_ADDR_LEN_MAX, "%s", ipAddrStr);
                }
            }
        }
        else
        {
            /* Store ipv4 address if not found previously */
            if (searchParam->cbParam.ipv4Addr[0] == '\0')
            {
                snprintf(searchParam->cbParam.ipv4Addr, IPV4_ADDR_LEN_MAX, "%s", ipAddrStr);
            }
        }
    }

    searchParam->cbParam.httpPort = DFLT_HTTP_PORT;
    searchParam->cbParam.onvifPort = port;

    /* Check if We Need to Exit from this multicast search */
    return searchParam->callBack((ONVIF_RESPONSE_PARA_t *)&searchParam->cbParam);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief searchOnvifDevices : Will send multicast search request to current Subnet. All onvif devices
 *                           : will response with entry point that will contain Camera IP and HTTP Port
 * @param sessionIndex       : session index
 */
static void searchOnvifDevices(UINT16 sessionIndex)
{
    NETWORK_PORT_e                  routePort;
    ONVIF_DEVICE_DISCOVERY_INFO_t   searchParam;

    if (onvifClientInfo[sessionIndex].onvifReqPara.onvifCallback == NULL)
    {
        return;
    }

    searchParam.cbParam.respCode = ONVIF_CAM_SEARCH_RESP_SUCCESS;
    searchParam.callBack = onvifClientInfo[sessionIndex].onvifReqPara.onvifCallback;

    for (routePort = NETWORK_PORT_LAN1; routePort <= NETWORK_PORT_LAN2; routePort++)
    {
        // Check if Lan Port is up
        if (TRUE == IsNetworkPortLinkUp(routePort))
        {
            multicastDiscoveryMsg(routePort, &searchParam);
        }

        // Invalidate Ip Address, As We Are Just Taking Input
        searchParam.cbParam.ipv4Addr[0] = '\0';
        searchParam.cbParam.ipv6Addr[IPV6_ADDR_GLOBAL][0] = '\0';
        searchParam.cbParam.ipv6Addr[IPV6_ADDR_LINKLOCAL][0] = '\0';

        if (searchParam.callBack((ONVIF_RESPONSE_PARA_t *)&searchParam.cbParam) == TRUE)
        {
            break;
        }
    }

    searchParam.cbParam.respCode = ONVIF_CAM_SEARCH_RESP_CLOSE;
    searchParam.callBack((ONVIF_RESPONSE_PARA_t *)&searchParam.cbParam);

    //to avoid multiple callback
    onvifClientInfo[sessionIndex].onvifReqPara.onvifCallback = NULL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief multicastDiscoveryMsg : will send multicast discovery message and receive response.
 * @param routePort             : interface used
 * @param searchParam           : camera search parameters
 */
static void multicastDiscoveryMsg(NETWORK_PORT_e routePort, ONVIF_DEVICE_DISCOVERY_INFO_t *searchParam)
{
    CHAR            uuidNum[MAX_UUID_LEN];
    INT32           connFd[IP_ADDR_TYPE_MAX];
    INT32           reuse = 1;
    CHAR            dataBuf[MAX_MULTICAST_MESSAGE_LEN];
    CHAR            sysCmd[SYS_CMD_MSG_LEN_MAX];
    LAN_CONFIG_t    lanConfig = {0};
    IP_ADDR_TYPE_e  ipAddrType;

    GetNetworkParamInfo(routePort, &lanConfig);

    /* Init both ipv4 & ipv6 socket fd to invalid value */
    for (ipAddrType = 0; ipAddrType < IP_ADDR_TYPE_MAX; ipAddrType++)
    {
        connFd[ipAddrType] = INVALID_FILE_FD;
    }

    do
    {
        /* Create socket to send multicast message using ipv4 address */
        connFd[IP_ADDR_TYPE_IPV4] = socket(AF_INET, UDP_NB_SOCK_OPTIONS, 0);
        if (connFd[IP_ADDR_TYPE_IPV4] == INVALID_CONNECTION)
        {
            EPRINT(ONVIF_CLIENT, "fail to create socket: [LAN=%d], [err=%s]", routePort, STR_ERR);
            break;
        }

        if (setsockopt(connFd[IP_ADDR_TYPE_IPV4], SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) != STATUS_OK)
        {
            EPRINT(ONVIF_CLIENT, "fail to set reuse addr: [LAN=%d], [err=%s]", routePort, STR_ERR);
            CloseSocket(&connFd[IP_ADDR_TYPE_IPV4]);
            break;
        }

        /* Update routing table to receive multicast messages */
        snprintf(sysCmd, sizeof(sysCmd), ADD_MULTICAST_ROUTE_CMD, IPV4_MULTICAST_RECV_ADDRESS, GetNetworkPortName(routePort, NULL));
        if (FALSE == ExeSysCmd(TRUE, sysCmd))
        {
            EPRINT(ONVIF_CLIENT, "fail to update routing table: [lan=%d], [err=%s]", routePort, STR_ERR);
            break;
        }

        /* Prepare onvif device discovery message */
        GenerateUuidStr(uuidNum);
        snprintf(dataBuf, MAX_MULTICAST_MESSAGE_LEN, DEVICE_DISCOVERY_MSG, uuidNum);

        /* Send multicast message using ipv4 address */
        if (FAIL == SendMulticastMessage(connFd[IP_ADDR_TYPE_IPV4], lanConfig.ipv4.lan.ipAddress, IPV4_MULTICAST_ADDRESS, ONVIF_MULTICAST_PORT,
                                         GetNetworkPortName(routePort, NULL), dataBuf))
        {
            EPRINT(ONVIF_CLIENT, "fail to send ipv4 onvif multicast message: [LAN=%d], [err=%s]", routePort, STR_ERR);
            break;
        }

    } while(0);

    do
    {
        /* Multicast onvif device discovery using ipv6 if enabled on interface */
        if (lanConfig.ipAddrMode != IP_ADDR_MODE_DUAL_STACK)
        {
            break;
        }

        /* Create socket to send UPnP multicast message using ipv6 address */
        connFd[IP_ADDR_TYPE_IPV6] = socket(AF_INET6, UDP_NB_SOCK_OPTIONS, 0);
        if (connFd[IP_ADDR_TYPE_IPV6] == INVALID_CONNECTION)
        {
            EPRINT(ONVIF_CLIENT, "fail to create socket: [LAN=%d], [err=%s]", routePort, STR_ERR);
            break;
        }

        if (setsockopt(connFd[IP_ADDR_TYPE_IPV6], SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) != STATUS_OK)
        {
            EPRINT(ONVIF_CLIENT, "fail to set reuse addr: [LAN=%d], [err=%s]", routePort, STR_ERR);
            CloseSocket(&connFd[IP_ADDR_TYPE_IPV6]);
            break;
        }

        /* Prepare ipv6 multicast message to send */
        GenerateUuidStr(uuidNum);
        snprintf(dataBuf, MAX_MULTICAST_MESSAGE_LEN, DEVICE_DISCOVERY_MSG, uuidNum);

        /* Send multicast message using ipv6 address */
        if (FAIL == SendMulticastMessage(connFd[IP_ADDR_TYPE_IPV6], lanConfig.ipv6.lan.ipAddress, IPV6_MULTICAST_ADDRESS, ONVIF_MULTICAST_PORT,
                                         GetNetworkPortName(routePort, NULL), dataBuf))
        {
            EPRINT(ONVIF_CLIENT, "fail to send ipv6 onvif multicast message: [LAN=%d], [err=%s]", routePort, STR_ERR);
            break;
        }

    } while(0);

    if ((connFd[IP_ADDR_TYPE_IPV4] != INVALID_FILE_FD) || (connFd[IP_ADDR_TYPE_IPV6] != INVALID_FILE_FD))
    {
        recvOnvifDiscoveryResponse(connFd, searchParam);
    }

    /* Remove multicast entry from routing table */
    snprintf(sysCmd, sizeof(sysCmd), DEL_MULTICAST_ROUTE_CMD, IPV4_MULTICAST_RECV_ADDRESS, GetNetworkPortName(routePort, NULL));
    if (FALSE == ExeSysCmd(TRUE, sysCmd))
    {
        EPRINT(ONVIF_CLIENT, "fail to update routing table: [lan=%d], [err=%s]", routePort, STR_ERR);
    }

    /* Close sockets */
    for (ipAddrType = 0; ipAddrType < IP_ADDR_TYPE_MAX; ipAddrType++)
    {
        CloseSocket(&connFd[ipAddrType]);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief recvOnvifDiscoveryResponse    : receive onvif device discovery multicast responses
 * @param connFd                        : list of fds
 * @param searchParam                   : camera search parameters
 * @return TRUE/FALSE
 */
static void recvOnvifDiscoveryResponse(INT32 *connFd, ONVIF_DEVICE_DISCOVERY_INFO_t *searchParam)
{
    UINT64              prevTimeMs = 0;
    INT32               pollSts;
    UINT8               ipAddrType;
    INT32               recvBytes;
    CHARPTR             startStr;
    CHARPTR             endStr;
    UINT8               ignoreBytes;
    INT16               messageLen;
    BOOL                breakLoop = FALSE;
    struct pollfd       pollFds[IP_ADDR_TYPE_MAX];
    CHAR                recvBuf[MAX_MULTICAST_MESSAGE_LEN];
    CHAR                parseUrlAddr[MAX_AVAILABLE_ADDRESS_LEN];

    while(breakLoop == FALSE)
    {
        for (ipAddrType = IP_ADDR_TYPE_IPV4; ipAddrType < IP_ADDR_TYPE_MAX; ipAddrType++)
        {
            pollFds[ipAddrType].fd = connFd[ipAddrType];
            pollFds[ipAddrType].events = POLLRDNORM;
            pollFds[ipAddrType].revents = 0;
        }

        /* Poll available socket's fd */
        pollSts = poll(pollFds, IP_ADDR_TYPE_MAX, GetRemainingPollTime(&prevTimeMs, MAX_RECEIVE_MESSAGE_TIME_OUT_MS));
        if (pollSts == -1)
        {
            /* Poll failed */
            EPRINT(ONVIF_CLIENT, "poll failed: [err=%s]", STR_ERR);
            break;
        }
        else if (pollSts == 0)
        {
            /* Poll timeout */
            WPRINT(ONVIF_CLIENT, "multicast msg resp receive timeout");
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
                EPRINT(ONVIF_CLIENT, "invalid poll event:[0x%x]", pollFds[ipAddrType].revents);
                continue;
            }

            /* Receive multicast message sent by different ip camera */
            recvBytes = recv(pollFds[ipAddrType].fd, recvBuf, (MAX_MULTICAST_MESSAGE_LEN - 1), MSG_NOSIGNAL);
            if (recvBytes <= 0)
            {
                EPRINT(ONVIF_CLIENT, "fail to receive multicast message: [err=%s]", strerror(errno));
                continue;
            }

            /* Terminate receive message with NULL */
            recvBuf[recvBytes] = '\0';
            ignoreBytes = 0;

            /* Ignore trailing '\r' '\n' */
            while ((recvBuf[recvBytes - 1 - ignoreBytes] == '\r') || (recvBuf[recvBytes - 1 - ignoreBytes] == '\n'))
            {
                ignoreBytes++;
            }

            /* Sanity check */
            if ((INT32)(recvBytes - ignoreBytes - strlen(DEVICE_DISCOVERY_RESP_END)) < 0)
            {
                WPRINT(ONVIF_CLIENT, "improper message response length receive: [length=%d]", recvBytes);
                continue;
            }

            /* Now parse the message to retrieve its device entry address. Check if String ends with </SOAP-ENV:Envelope> */
            if (strncasecmp((recvBuf + recvBytes - ignoreBytes - strlen(DEVICE_DISCOVERY_RESP_END)),
                            DEVICE_DISCOVERY_RESP_END, strlen(DEVICE_DISCOVERY_RESP_END)) != STATUS_OK)
            {
                continue;
            }

            /* Get Position of XAddrs in which entry point is given */
            if ((startStr = strcasestr(recvBuf, ONVIF_ENTRY_POINT_TAG)) == NULL)
            {
                continue;
            }

            startStr += strlen(ONVIF_ENTRY_POINT_TAG);
            if ((endStr = strchr(startStr, '<')) == NULL)
            {
                continue;
            }

            messageLen = (endStr - startStr);
            if ((messageLen <= 0) || (messageLen >= MAX_AVAILABLE_ADDRESS_LEN))
            {
                continue;
            }

            strncpy(parseUrlAddr, startStr, messageLen);
            parseUrlAddr[messageLen] = '\0';
            if (multicastCameraSearch(parseUrlAddr, searchParam) == TRUE)
            {
                breakLoop = TRUE;
                break;
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief getEntryPointFromUnicastReq   : Get entry point URL for camera whose IP address is known
 * @param deviceIpAddr                  : IP
 * @param camIndex                      : camera Index
 * @return
 */
static ONVIF_RESP_STATUS_e getEntryPointFromUnicastReq(CHARPTR deviceIpAddr, UINT8 camIndex)
{
    CHAR                uuidNum[MAX_UUID_LEN];
    INT16               messageLen;
    ONVIF_RESP_STATUS_e onvifResp = ONVIF_CMD_FAIL;
    SOCK_ADDR_INFO_u 	groupSock;
    INT32               connFd = INVALID_CONNECTION;
    CHARPTR             startStr;
    CHARPTR             endStr;
    UINT8               ignoreBytes;
    INT32               reuse = 1;
    CHAR                recvBuf[MAX_MULTICAST_MESSAGE_LEN];
    INT32               recvBytes;
    CHAR                address[MAX_AVAILABLE_ADDRESS_LEN];
    UINT8               pollSts;
    UINT64              prevTimeMs = 0;
    INT16               pollRevent;

    /* Get socket address from ip and port */
    if (FAIL == GetSockAddr(deviceIpAddr, MULTICAST_PORT, &groupSock))
    {
        EPRINT(ONVIF_CLIENT, "fail to get socket addr: [camera=%d], [ip=%s]", camIndex, deviceIpAddr);
        return ONVIF_CMD_FAIL;
    }

    GenerateUuidStr(uuidNum);
    messageLen = snprintf(recvBuf, MAX_MULTICAST_MESSAGE_LEN, DEVICE_DISCOVERY_MSG, uuidNum);

    /* Create socket to send unicast message */
    connFd = socket(groupSock.sockAddr.sa_family, UDP_SOCK_OPTIONS, 0);
    if (connFd == INVALID_CONNECTION)
    {
        EPRINT(ONVIF_CLIENT, "fail to create socket: [camera=%d], [ip=%s], [err=%s]", camIndex, deviceIpAddr, strerror(errno));
        return ONVIF_CMD_FAIL;
    }

    if (setsockopt(connFd, SOL_SOCKET, SO_REUSEADDR, (CHARPTR)&reuse, sizeof(reuse)) != STATUS_OK)
    {
        EPRINT(ONVIF_CLIENT, "fail to set reuse addr: [camera=%d], [ip=%s], [err=%s]", camIndex, deviceIpAddr, strerror(errno));
        CloseSocket(&connFd);
        return ONVIF_CMD_FAIL;
    }

    if (sendto(connFd, recvBuf, messageLen, MSG_NOSIGNAL, (struct sockaddr*)&groupSock, sizeof(groupSock)) != messageLen)
    {
        EPRINT(ONVIF_CLIENT, "fail to send onvif message: [camera=%d], [ip=%s], [err=%s]", camIndex, deviceIpAddr, strerror(errno));
        CloseSocket(&connFd);
        return ONVIF_CMD_FAIL;
    }

    while(TRUE)
    {
        // if time out then return
        pollSts = GetSocketPollEvent(connFd, (POLLRDNORM | POLLRDHUP), GetRemainingPollTime(&prevTimeMs, MAX_RECEIVE_MESSAGE_TIME_OUT_MS), &pollRevent);
        if (FAIL == pollSts)
        {
            EPRINT(ONVIF_CLIENT, "onvif message response receive poll failed: [camera=%d], [ip=%s]", camIndex, deviceIpAddr);
            break;
        }

        if (TIMEOUT == pollSts)
        {
            WPRINT(ONVIF_CLIENT, "onvif message response receive timeout: [camera=%d], [ip=%s]", camIndex, deviceIpAddr);
            onvifResp = ONVIF_CMD_TIMEOUT;
            break;
        }

        // if other than read event
        if ((pollRevent & POLLRDNORM) != POLLRDNORM)
        {
            if ((pollRevent & POLLRDHUP) == POLLRDHUP)
            {
                EPRINT(ONVIF_CLIENT, "remote connection closed");
                break;
            }

            EPRINT(ONVIF_CLIENT, "invalid poll event:[0x%x], [camera=%d], [ip=%s]", pollRevent, camIndex, deviceIpAddr);
            break;
        }

        // receive multicast message sent by different ip camera
        recvBytes = recv(connFd, recvBuf, (MAX_MULTICAST_MESSAGE_LEN - 1), MSG_NOSIGNAL);
        if (recvBytes <= 0)
        {
            EPRINT(ONVIF_CLIENT, "fail to receive onvif message: [camera=%d], [ip=%s], [err=%s]", camIndex, deviceIpAddr, strerror(errno));
            break;
        }

        // terminate receive message with NULL.
        recvBuf[recvBytes] = '\0';
        ignoreBytes = 0;

        // Ignore trailing '\r' '\n'
        while ((recvBuf[recvBytes - 1 - ignoreBytes] == '\r') || (recvBuf[recvBytes - 1 - ignoreBytes] == '\n'))
        {
            ignoreBytes++;
        }

        /* Sanity check */
        if ((INT32)(recvBytes - ignoreBytes - strlen(DEVICE_DISCOVERY_RESP_END)) < 0)
        {
            WPRINT(ONVIF_CLIENT, "improper message response length receive: [camera=%d], [ip=%s], [length=%d]", camIndex, deviceIpAddr, recvBytes);
            continue;
        }

        // Now parse the message to retrieve its device entry address and check if String ends with </SOAP-ENV:Envelope>
        if (strncasecmp((recvBuf + recvBytes - ignoreBytes - strlen(DEVICE_DISCOVERY_RESP_END)),
                        DEVICE_DISCOVERY_RESP_END, strlen(DEVICE_DISCOVERY_RESP_END)) != STATUS_OK)
        {
            continue;
        }

        // Get Position of XAddrs in which entry point is given
        if ((startStr = strcasestr(recvBuf, ONVIF_ENTRY_POINT_TAG)) == NULL)
        {
            continue;
        }

        startStr += strlen(ONVIF_ENTRY_POINT_TAG);
        if ((endStr = strchr(startStr, '<')) == NULL)
        {
            continue;
        }

        messageLen = (endStr - startStr);
        if ((messageLen <= 0) || (messageLen >= MAX_AVAILABLE_ADDRESS_LEN))
        {
            continue;
        }

        strncpy(address, startStr, messageLen);
        address[messageLen] = '\0';
        if (strstr(address, deviceIpAddr) == NULL)
        {
            break;
        }

        if (FAIL == ParseRequestUri(deviceIpAddr, address, onvifCamInfo[camIndex].realtiveEntryPt, sizeof(onvifCamInfo[camIndex].realtiveEntryPt)))
        {
            EPRINT(ONVIF_CLIENT, "fail to parse relative entry point: [camera=%d], [url=%s]", camIndex, address);
            break;
        }

        onvifResp = ONVIF_CMD_SUCCESS;
        DPRINT(ONVIF_CLIENT, "onvif device found: [camera=%d], [relativeEntryPoint=%s]", camIndex, onvifCamInfo[camIndex].realtiveEntryPt);
        break;
    }

    CloseSocket(&connFd);
    return onvifResp;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief getCameraDateTime : get date and time,store it to struct "timeDifference"
 * @param soap              : SOAP Instance
 * @param camIndex          : camera index
 * @param user              : user information
 * @return
 */
static ONVIF_RESP_STATUS_e getCameraDateTime(SOAP_t *soap, UINT8 camIndex, SOAP_USER_DETAIL_t *user)
{
    INT16  								soapResp;
    GET_SYSTEM_DATE_AND_TIME_t 			getSystemDateAndTime;
    GET_SYSTEM_DATE_AND_TIME_RESPONSE_t getSystemDateAndTimeResponse;
    time_t 								t = time(NULL);
    struct tm 							*tm = gmtime(&t);

    if (tm == NULL)
    {
        EPRINT(ONVIF_CLIENT, "fail to get system time: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    user->addr = onvifCamInfo[camIndex].deviceEntryPoint;
    soapResp = GetSystemDateAndTime(soap, &getSystemDateAndTime, &getSystemDateAndTimeResponse, user);
    if((soapResp != SOAP_OK) || (getSystemDateAndTimeResponse.SystemDateAndTime == NULL))
    {
        if((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
        {
            return ONVIF_CMD_TIMEOUT;
        }
        return ONVIF_CMD_FAIL;
    }

    /* NOTE: Following Modifications have been done as user->addr points to some address which is part of onvifCamInfo[camIndex].deviceEntryPoint,
     * Hence, on snprintf if source and destination buffer are same then it will give undefined beahviour */
    CHAR tmpDeviceEntryPoint[MAX_SERVICE_ADDR_LEN];
    snprintf(tmpDeviceEntryPoint, sizeof(tmpDeviceEntryPoint), "%s", user->addr);
    snprintf(onvifCamInfo[camIndex].deviceEntryPoint, sizeof(onvifCamInfo[camIndex].deviceEntryPoint), "%s", tmpDeviceEntryPoint);

    onvifCamInfo[camIndex].timeDifference.year = (getSystemDateAndTimeResponse.SystemDateAndTime->UTCDateTime->Date->Year - (START_YEAR + tm->tm_year));
    onvifCamInfo[camIndex].timeDifference.month = (getSystemDateAndTimeResponse.SystemDateAndTime->UTCDateTime->Date->Month - (tm->tm_mon + 1));
    onvifCamInfo[camIndex].timeDifference.day = (getSystemDateAndTimeResponse.SystemDateAndTime->UTCDateTime->Date->Day - tm->tm_mday);
    onvifCamInfo[camIndex].timeDifference.hour = (getSystemDateAndTimeResponse.SystemDateAndTime->UTCDateTime->Time->Hour - tm->tm_hour);
    onvifCamInfo[camIndex].timeDifference.minute = (getSystemDateAndTimeResponse.SystemDateAndTime->UTCDateTime->Time->Minute - tm->tm_min);
    onvifCamInfo[camIndex].timeDifference.seconds = (getSystemDateAndTimeResponse.SystemDateAndTime->UTCDateTime->Time->Second - tm->tm_sec);

    return ONVIF_CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief setCameraDateTime : will get UTC time from device and set it to camera
 * @param soap              : SOAP instance
 * @param camIndex          : camera Index
 * @param user              : user
 * @return
 */
static ONVIF_RESP_STATUS_e setCameraDateTime(SOAP_t *soap, UINT8 camIndex, SOAP_USER_DETAIL_t *user)
{
    struct tt__Time                     timeStruct;
    struct tt__Date                     date;
    struct tt__DateTime                 dateTime;
    INT16                               soapResp;
    SET_SYSTEM_DATE_AND_TIME_t          setSystemDateAndTime;
    SET_SYSTEM_DATE_AND_TIME_RESPONSE_t setSystemDateAndTimeResponse;
    struct tm                           localDateTime = {0};
    char                                timeZone[50];
    UINT8                               attempToSet = 0;
    struct tt__TimeZone                 timeZoneStruct;
    UINT8                               timeZoneIndex = 0;
    UINT8                               sendTimeZone = FALSE;
    DATE_TIME_CONFIG_t                  dateTimeConfig;

    dateTimeConfig.setUtcTime = FALSE;
    ReadDateTimeConfig(&dateTimeConfig);

    do
    {
        if (ONVIF_CMD_SUCCESS != getCameraDateTime(soap, camIndex, user))
        {
            EPRINT(ONVIF_CLIENT, "fail to get camera date-time: [camera=%d]", camIndex);
            return ONVIF_CMD_FAIL;
        }

        user->addr = onvifCamInfo[camIndex].deviceEntryPoint;
        user->timeDiff = &onvifCamInfo[camIndex].timeDifference;
        setSystemDateAndTime.DateTimeType = tt__SetDateTimeType__Manual;
        setSystemDateAndTime.DaylightSavings = XSD_BOOL_FALSE;

        if (dateTimeConfig.setUtcTime == FALSE) //Local time case
        {
            GetLocalTimeInBrokenTm(&localDateTime); //Use Local Time
            if (attempToSet == 0)
            {
                snprintf(timeZone, sizeof(timeZone), "UTC+0:00");
            }
            else
            {
                snprintf(timeZone, sizeof(timeZone), "GMT+0:00");
            }

            timeZoneStruct.TZ = &timeZone[0];
            setSystemDateAndTime.TimeZone = &timeZoneStruct;
        }
        else if (GetUtcTimeForOnvif(&localDateTime, &timeZoneIndex, &sendTimeZone) == SUCCESS) //UTC case
        {
            localDateTime.tm_year = (localDateTime.tm_year + START_YEAR);
            if (sendTimeZone == TRUE) //Need to send configured Timezone
            {
                if ((timeZoneIndex < MIN_TIMEZONE ) || (timeZoneIndex > MAX_TIMEZONE))
                {
                    EPRINT(ONVIF_CLIENT, "invld time-zone index: : [camera=%d], [timeZoneIndex=%d]", camIndex, timeZoneIndex);
                    return ONVIF_CMD_FAIL;
                }

                snprintf(timeZone, sizeof(timeZone), "%s", OnviftimeZoneNames[timeZoneIndex-1][attempToSet]);
                timeZoneStruct.TZ = &timeZone[0];
                setSystemDateAndTime.TimeZone = &timeZoneStruct;
            }
            else // No need to send Timezone only send UTC Time
            {
                 setSystemDateAndTime.TimeZone  = NULL;
            }
        }
        else
        {
            EPRINT(ONVIF_CLIENT, "fail to get system UTC time: [camera=%d]", camIndex);
            return ONVIF_CMD_FAIL;
        }

        date.Year = localDateTime.tm_year;
        date.Month = localDateTime.tm_mon + 1;
        date.Day = localDateTime.tm_mday;
        timeStruct.Hour = localDateTime.tm_hour;
        timeStruct.Minute = localDateTime.tm_min;
        timeStruct.Second = localDateTime.tm_sec;
        dateTime.Time = &timeStruct;
        dateTime.Date = &date;
        setSystemDateAndTime.UTCDateTime = &dateTime;
        soapResp = SetSystemDateAndTime(soap, &setSystemDateAndTime, &setSystemDateAndTimeResponse, user);
        if (soapResp != SOAP_OK)
        {
            if ((TRUE == sendTimeZone) && (++attempToSet < 2))
            {
                /* First we will try with contry time zone string, if it gets failed then we will try with
                 * CST (central strandard time) time zone string */
                continue;
            }

            if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
            {
                return ONVIF_CMD_TIMEOUT;
            }

            return ONVIF_CMD_FAIL;
        }

        /* NOTE: Following Modifications have been done as user->addr points to some address which is part of onvifCamInfo[camIndex].deviceEntryPoint,
         * Hence, on snprintf if source and destinationbuffer are same then it will give undefined beahviour */
        CHAR tmpDeviceEntryPoint[MAX_SERVICE_ADDR_LEN];
        snprintf(tmpDeviceEntryPoint, sizeof(tmpDeviceEntryPoint), "%s", user->addr);
        snprintf(onvifCamInfo[camIndex].deviceEntryPoint, sizeof(onvifCamInfo[camIndex].deviceEntryPoint), "%s", tmpDeviceEntryPoint);
        break;

    }while(TRUE);

    return ONVIF_CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief getOnvifCameraCapability  : Will send multiple command to get all services from camera and
 *                                    store it for other operations.
 * @param camIndex                  : Camera Index
 * @param user                      : User
 * @param soap                      : SOAP
 * @return
 */
static ONVIF_RESP_STATUS_e getOnvifCameraCapability(UINT8 camIndex, SOAP_USER_DETAIL_t *user, SOAP_t *soap)
{
    ONVIF_RESP_STATUS_e                                 status = ONVIF_CMD_FAIL;
    INT16                                               soapResp;
    UINT8                                               alarmOutput;
    BOOL                                                ptzSupport = NO;
    BOOL                                                imagingSupport = NO;
    GET_CAPABILITIES_t 								   	getCapabilities;
    GET_CAPABILITIES_RESPONSE_t 					   	getCapabilitiesResponse;
    GET_VIDEO_ENCODER_CONFIGURATION_OPTIONS_t 		   	getVideoEncoderConfigurationOptions;
    GET_VIDEO_ENCODER_CONFIGURATION_OPTIONS_RESPONSE_t 	getVideoEncoderConfigurationOptionsResponse;
    GET_VIDEOANALYTICSCONFIGS_t							videoAnalyticsConfigs;
    GET_VIDEOANALYTICSCONFIGSRESPONSE_t					videoAnalyticsConfigsResponse;
    CAPABILITY_CATEGORY_e         						category = tt__CapabilityCategory__All;
    GET_RELAY_OUTPUTS_t 		  						getRelayOutputs;
    GET_RELAY_OUTPUTS_RESPONSE_t  						getRelayOutputsResponse;
    GET_SERVICES_t                                      getServices;
    GET_SERVICES_RESPONSE_t                             getServicesResponse;
    CAPABILITY_TYPE                                     capabilitySupported = 0;
    UINT8                                               loop = 0;
    BOOL                                                media2Support = NO;
    BOOL                                                analytics2Support = NO;
    CHAR                                                ipAddrForUrl[DOMAIN_NAME_SIZE_MAX];
    CHAR                                                realtiveEntryPoint[MAX_RELATIVE_ADDR_LEN];

    //--------------------------------------------------------------------------------
    // Get Services
    //--------------------------------------------------------------------------------
    user->addr = onvifCamInfo[camIndex].deviceEntryPoint;
    user->timeDiff = &onvifCamInfo[camIndex].timeDifference;
    getServices.IncludeCapability = XSD_BOOL_FALSE;
    soapResp = GetServices(soap, &getServices, &getServicesResponse, user);
    if ((soapResp == SOAP_OK) && (getServicesResponse.__sizeService > 0) && (getServicesResponse.Service != NULL))
    {
        for (loop = 0; loop < getServicesResponse.__sizeService; loop++)
        {
            if (getServicesResponse.Service[loop].Namespace != NULL)
            {
                if ((NO == media2Support) && (strcmp(getServicesResponse.Service[loop].Namespace, MEDIA2_WSDL_LOCATION) == STATUS_OK))
                {
                    media2Support = YES;
                }
                else if ((NO == analytics2Support) && (strcmp(getServicesResponse.Service[loop].Namespace, ANALYTICS2_WSDL_LOCATION) == STATUS_OK))
                {
                    analytics2Support = YES;
                }

                if ((YES == media2Support) && (YES == analytics2Support))
                {
                    break;
                }
            }
        }
    }

    /* To check various capabitities supported */
    getCapabilities.Category = &category;
    getCapabilities.__sizeCategory = 1;

    do
    {
        //--------------------------------------------------------------------------------
        // Get Capabilities
        //--------------------------------------------------------------------------------
        soapResp = GetCapabilities(soap, &getCapabilities, &getCapabilitiesResponse, user);
        if (soapResp != SOAP_OK)
        {
            EPRINT(ONVIF_CLIENT, "fail to get capabilites: [camera=%d], [soapResp=%d]", camIndex, soapResp);
            break;
        }

        if ((soapResp != SOAP_OK) || (getCapabilitiesResponse.Capabilities == NULL))
        {
            EPRINT(ONVIF_CLIENT, "capabilites not available: [camera=%d]", camIndex);
            break;
        }

        //--------------------------------------------------------------------------------
        // Parse Media serivce URI
        //--------------------------------------------------------------------------------
        if (getCapabilitiesResponse.Capabilities->Media == NULL)
        {
            EPRINT(ONVIF_CLIENT, "no media service available: [camera=%d]", camIndex);
            break;
        }

        if (ParseRequestUri(user->ipAddr, getCapabilitiesResponse.Capabilities->Media->XAddr, onvifCamInfo[camIndex].mediaServiceInfo.relativeMediaServiceAddr,
                            sizeof(onvifCamInfo[camIndex].mediaServiceInfo.relativeMediaServiceAddr)) == FAIL)
        {
            EPRINT(ONVIF_CLIENT, "fail to parse media xaddr url: [camera=%d]", camIndex);
            break;
        }

        PrepareIpAddressForUrl(user->ipAddr, ipAddrForUrl);
        snprintf(onvifCamInfo[camIndex].mediaServiceInfo.mediaServiceAddr, MAX_SERVICE_ADDR_LEN, HTTP_REQUEST_URL,
                 ipAddrForUrl, user->port, onvifCamInfo[camIndex].mediaServiceInfo.relativeMediaServiceAddr);

        //--------------------------------------------------------------------------------
        // Parse PTZ serivce URI
        //--------------------------------------------------------------------------------
        if (getCapabilitiesResponse.Capabilities->PTZ != NULL)
        {
            if (ParseRequestUri(user->ipAddr, getCapabilitiesResponse.Capabilities->PTZ->XAddr, onvifCamInfo[camIndex].ptzServiceInfo.relativePtzServiceAddr,
                                sizeof(onvifCamInfo[camIndex].ptzServiceInfo.relativePtzServiceAddr)) == FAIL)
            {
                EPRINT(ONVIF_CLIENT, "fail to parse ptz xaddr url: [camera=%d]", camIndex);
            }
            else
            {
                snprintf(onvifCamInfo[camIndex].ptzServiceInfo.ptzServiceAddr, MAX_SERVICE_ADDR_LEN, HTTP_REQUEST_URL,
                         ipAddrForUrl, user->port, onvifCamInfo[camIndex].ptzServiceInfo.relativePtzServiceAddr);
                ptzSupport = YES;
                capabilitySupported |= MX_ADD(PTZ_SUPPORT);
            }
        }

        //--------------------------------------------------------------------------------
        // Parse Imaging serivce URI
        //--------------------------------------------------------------------------------
        if (getCapabilitiesResponse.Capabilities->Imaging != NULL)
        {
            if (ParseRequestUri(user->ipAddr, getCapabilitiesResponse.Capabilities->Imaging->XAddr, onvifCamInfo[camIndex].imagingServiceInfo.relativeImagingServiceAddr,
                                sizeof(onvifCamInfo[camIndex].imagingServiceInfo.relativeImagingServiceAddr)) == FAIL)
            {
                EPRINT(ONVIF_CLIENT, "fail to parse imaging xaddr url: [camera=%d]", camIndex);
            }
            else
            {
                snprintf(onvifCamInfo[camIndex].imagingServiceInfo.imagingServiceAddr, MAX_SERVICE_ADDR_LEN, HTTP_REQUEST_URL,
                         ipAddrForUrl, user->port, onvifCamInfo[camIndex].imagingServiceInfo.relativeImagingServiceAddr);
                imagingSupport = YES;
            }
        }

        //--------------------------------------------------------------------------------
        // Set Media Releated Capabilities
        //--------------------------------------------------------------------------------
        user->addr = onvifCamInfo[camIndex].mediaServiceInfo.mediaServiceAddr;
        if (media2Support == YES)
        {
            if ((status = selectProfileForMedia2Service(soap, user, ptzSupport, camIndex)) != ONVIF_CMD_SUCCESS)
            {
                /* If camera doesn't support media2 then ideally should not use analytics2. Hence withdraw support of it also */
                media2Support = NO;
                analytics2Support = NO;
                WPRINT(ONVIF_CLIENT, "fail to get media2 profile, trying for media1: [camera=%d], [status=%d]", camIndex, status);
            }
        }

        if (media2Support == NO)
        {
            if ((status = selectProfileForMediaService(soap, user,ptzSupport, camIndex)) != ONVIF_CMD_SUCCESS)
            {
                EPRINT(ONVIF_CLIENT, "fail to get media1 profile: [camera=%d], [status=%d]", camIndex, status);
                break;
            }
        }

        if (media2Support == NO)
        {
            onvifCamInfo[camIndex].mediaServiceInfo.initialQuality = 0;
            getVideoEncoderConfigurationOptions.ConfigurationToken = NULL;
            getVideoEncoderConfigurationOptions.ProfileToken = NULL;
            soapResp = GetVideoEncoderConfigurationOptions(soap, &getVideoEncoderConfigurationOptions, &getVideoEncoderConfigurationOptionsResponse, user);
            if(soapResp != SOAP_OK)
            {
                EPRINT(ONVIF_CLIENT, "fail to get video encoder config option: [camera=%d], [soapResp=%d]", camIndex, soapResp);
            }
            else if((getVideoEncoderConfigurationOptionsResponse.Options != NULL)
                     && (getVideoEncoderConfigurationOptionsResponse.Options->QualityRange != NULL))
            {
                onvifCamInfo[camIndex].mediaServiceInfo.initialQuality = getVideoEncoderConfigurationOptionsResponse.Options->QualityRange->Min;
            }
        }

        /* Set media2 support flag */
        onvifCamInfo[camIndex].imagingServiceInfo.isImgSettingSupport = NO;
        onvifCamInfo[camIndex].mediaServiceInfo.media2Support = media2Support;
        onvifCamInfo[camIndex].mediaServiceInfo.analytics2Support = analytics2Support;
        DPRINT(ONVIF_CLIENT, "media services status: [camera=%d], [media2Support=%s], [analytics2Support=%s]",
               camIndex, media2Support ? "YES" : "NO", analytics2Support ? "YES" : "NO");

        // fill stream capabilities by default UDP enable
        onvifCamInfo[camIndex].mediaServiceInfo.rtspTranProtocol = (1 << OVER_UDP);

        // fill stream capabilities- checking for TCP
        if(getCapabilitiesResponse.Capabilities->Media->StreamingCapabilities != NULL)
        {
            if(*(getCapabilitiesResponse.Capabilities->Media->StreamingCapabilities->RTP_USCORERTSP_USCORETCP))
            {
                onvifCamInfo[camIndex].mediaServiceInfo.rtspTranProtocol = (1 << TCP_INTERLEAVED);
            }
        }

        //--------------------------------------------------------------------------------
        // Add Audio Capabilities
        //--------------------------------------------------------------------------------
        status = addAudioConfiguration(soap, user, camIndex);
        if (status == ONVIF_CMD_SUCCESS)
        {
            capabilitySupported |= MX_ADD(AUDIO_MIC_SUPPORT);
            onvifCamInfo[camIndex].audioAvailable = YES;
        }
        else
        {
            onvifCamInfo[camIndex].audioAvailable = NO;
        }

        //--------------------------------------------------------------------------------
        // Add PTZ Capabilities
        //--------------------------------------------------------------------------------
        if (ptzSupport == YES)
        {
            user->addr = onvifCamInfo[camIndex].ptzServiceInfo.ptzServiceAddr;
            status = addPtzConfiguration(soap, user, camIndex);
            if(status  != ONVIF_CMD_SUCCESS)
            {
                EPRINT(ONVIF_CLIENT, "fail to get ptz config: [camera=%d], [status=%d]", camIndex, status);
            }
        }

        //--------------------------------------------------------------------------------
        // Add Alarm Capabilities
        //--------------------------------------------------------------------------------
        user->addr = onvifCamInfo[camIndex].deviceEntryPoint;
        soapResp = GetRelayOutputs(soap, &getRelayOutputs, &getRelayOutputsResponse, user);
        if(soapResp == SOAP_OK)
        {
            if(getRelayOutputsResponse.RelayOutputs != NULL)
            {
                for(alarmOutput = 0; ((alarmOutput < getRelayOutputsResponse.__sizeRelayOutputs) && (alarmOutput < MAX_CAMERA_ALARM)); alarmOutput++)
                {
                    if(StringNCopy(onvifCamInfo[camIndex].relayToken[alarmOutput],
                                   (getRelayOutputsResponse.RelayOutputs + alarmOutput)->token, MAX_TOKEN_SIZE) == SUCCESS)
                    {
                        capabilitySupported |= MX_ADD((alarmOutput + CAMERA_ALARM_OUTPUT1_SUPPORT));
                    }
                }
            }
            else
            {
                DPRINT(ONVIF_CLIENT, "no relay output support: [camera=%d]", camIndex);
            }
        }

        //--------------------------------------------------------------------------------
        // Add Imaging Capabilities
        //--------------------------------------------------------------------------------
        if (imagingSupport == YES)
        {
            status = addImagingConfiguration(soap, user, camIndex);
            if(status != ONVIF_CMD_SUCCESS)
            {
                if(onvifCamInfo[camIndex].imagingServiceInfo.focusInfo.focusSupport != NO_FUNCTION_SUPPORT)
                {
                    capabilitySupported |= MX_ADD(FOCUS_SUPPORT);
                }

                if(onvifCamInfo[camIndex].imagingServiceInfo.irisInfo.irisSupport != NO_FUNCTION_SUPPORT)
                {
                    capabilitySupported |= MX_ADD(IRIS_SUPPORT);
                }
                EPRINT(ONVIF_CLIENT, "fail to add imaging config: [camera=%d], [status=%d]", camIndex, status);
            }
        }

        //--------------------------------------------------------------------------------
        // Add Event Capabilities
        //--------------------------------------------------------------------------------
        if(getCapabilitiesResponse.Capabilities->Events != NULL)
        {
            if (ParseRequestUri(user->ipAddr, getCapabilitiesResponse.Capabilities->Events->XAddr, realtiveEntryPoint, sizeof(realtiveEntryPoint)) == FAIL)
            {
                EPRINT(ONVIF_CLIENT, "fail to parse events xaddr url: [camera=%d]", camIndex);
            }
            else
            {
                snprintf(onvifCamInfo[camIndex].eventServiceInfo.eventServiceUri, MAX_SERVICE_ADDR_LEN, HTTP_REQUEST_URL,
                         ipAddrForUrl, user->port, realtiveEntryPoint);
            }

            if(getEventCapabilities(soap, user, camIndex) == SUCCESS)
            {
                //If event tag for motion detection available then add motion detection capability
                if(onvifCamInfo[camIndex].eventServiceInfo.onvifEvRespDet[ONVIF_MOTION_EVENT].tagAvailable > 0)
                {
                    capabilitySupported |= MX_ADD(MOTION_DETECTION_SUPPORT);
                    DPRINT(ONVIF_CLIENT, "motion detection event tag found: [camera=%d], [tagCnt=%d]", camIndex,
                           onvifCamInfo[camIndex].eventServiceInfo.onvifEvRespDet[ONVIF_MOTION_EVENT].tagAvailable);
                }

                if(onvifCamInfo[camIndex].eventServiceInfo.onvifEvRespDet[ONVIF_SENSOR_EVENT].tagAvailable > 0)
                {
                    fillOtherSupportedCapabiltyToCamera(&capabilitySupported, &getCapabilitiesResponse);
                    DPRINT(ONVIF_CLIENT, "sensor event tag found: [camera=%d], [tagCnt=%d]", camIndex,
                           onvifCamInfo[camIndex].eventServiceInfo.onvifEvRespDet[ONVIF_SENSOR_EVENT].tagAvailable);
                }
                if(onvifCamInfo[camIndex].eventServiceInfo.onvifEvRespDet[ONVIF_TAMPER_EVENT].tagAvailable > 0)
                {
                    capabilitySupported |= MX_ADD(TAMPER_DETECTION_SUPPORT);
                    DPRINT(ONVIF_CLIENT, "temper detection event tag found: [camera=%d], [tagCnt=%d]", camIndex,
                           onvifCamInfo[camIndex].eventServiceInfo.onvifEvRespDet[ONVIF_TAMPER_EVENT].tagAvailable);
                }

                if(onvifCamInfo[camIndex].eventServiceInfo.onvifEvRespDet[ONVIF_LINE_CROSS_EVENT].tagAvailable > 0)
                {
                    capabilitySupported |= MX_ADD(LINE_CROSSING_SUPPORT);
                    DPRINT(ONVIF_CLIENT, "line cross event tag found: [camera=%d], [tagCnt=%d]", camIndex,
                           onvifCamInfo[camIndex].eventServiceInfo.onvifEvRespDet[ONVIF_LINE_CROSS_EVENT].tagAvailable);
                }

                if(onvifCamInfo[camIndex].eventServiceInfo.onvifEvRespDet[ONVIF_OBJECT_INSIDE_EVENT].tagAvailable > 0)
                {
                    capabilitySupported |= MX_ADD(OBJECT_INTRUSION_SUPPORT);
                    DPRINT(ONVIF_CLIENT, "object intrusion event tag found: [camera=%d], [tagCnt=%d]", camIndex,
                           onvifCamInfo[camIndex].eventServiceInfo.onvifEvRespDet[ONVIF_OBJECT_INSIDE_EVENT].tagAvailable);
                }

                if(onvifCamInfo[camIndex].eventServiceInfo.onvifEvRespDet[ONVIF_AUDIO_EXCEPTION_EVENT].tagAvailable > 0)
                {
                    capabilitySupported |= MX_ADD(AUDIO_EXCEPTION_SUPPORT);
                    DPRINT(ONVIF_CLIENT, "audio exception event tag found: [camera=%d], [tagCnt=%d]", camIndex,
                           onvifCamInfo[camIndex].eventServiceInfo.onvifEvRespDet[ONVIF_AUDIO_EXCEPTION_EVENT].tagAvailable);
                }

                if(onvifCamInfo[camIndex].eventServiceInfo.onvifEvRespDet[ONVIF_LOTERING_EVENT].tagAvailable > 0)
                {
                    capabilitySupported |= MX_ADD(LOITERING_SUPPORT);
                    DPRINT(ONVIF_CLIENT, "lotering event tag found: [camera=%d], [tagCnt=%d]", camIndex,
                           onvifCamInfo[camIndex].eventServiceInfo.onvifEvRespDet[ONVIF_LOTERING_EVENT].tagAvailable);
                }

                if(onvifCamInfo[camIndex].eventServiceInfo.onvifEvRespDet[ONVIF_OBJECT_CNT_EVENT].tagAvailable > 0)
                {
                    capabilitySupported |= MX_ADD(OBJECT_COUNTING_SUPPORT);
                    DPRINT(ONVIF_CLIENT, "object count event tag found: [camera=%d], [tagCnt=%d]", camIndex,
                           onvifCamInfo[camIndex].eventServiceInfo.onvifEvRespDet[ONVIF_OBJECT_CNT_EVENT].tagAvailable);
                }
            }
            else
            {
                EPRINT(ONVIF_CLIENT, "fail to get events config: [camera=%d]", camIndex);
            }
        }
        else
        {
            EPRINT(ONVIF_CLIENT, "events not supported by camera: [camera=%d]", camIndex);
        }

        //--------------------------------------------------------------------------------
        // Add Video Analytics Capabilities
        //--------------------------------------------------------------------------------
        if((getCapabilitiesResponse.Capabilities->Analytics != NULL)
                && (getCapabilitiesResponse.Capabilities->Analytics->AnalyticsModuleSupport == XSD_BOOL_TRUE)
                && (getCapabilitiesResponse.Capabilities->Analytics->RuleSupport == XSD_BOOL_TRUE))
        {
            if (ParseRequestUri(user->ipAddr, getCapabilitiesResponse.Capabilities->Analytics->XAddr, onvifCamInfo[camIndex].mediaServiceInfo.relativeAnalyticsServiceAddr,
                                sizeof(onvifCamInfo[camIndex].mediaServiceInfo.relativeAnalyticsServiceAddr)) == FAIL)
            {
                WPRINT(ONVIF_CLIENT, "fail to parse analytics xaddr url, withdraw analytics2 support: [camera=%d]", camIndex);
                onvifCamInfo[camIndex].mediaServiceInfo.analytics2Support = NO;
            }
            else
            {
                snprintf(onvifCamInfo[camIndex].mediaServiceInfo.analyticsServiceAddr, MAX_SERVICE_ADDR_LEN, HTTP_REQUEST_URL,
                         ipAddrForUrl, user->port, onvifCamInfo[camIndex].mediaServiceInfo.relativeAnalyticsServiceAddr);
            }

            user->addr = onvifCamInfo[camIndex].mediaServiceInfo.mediaServiceAddr;
            soapResp = GetVideoAnaltics(soap, &videoAnalyticsConfigs, &videoAnalyticsConfigsResponse, user);
            if(soapResp == SOAP_OK)
            {
                if((videoAnalyticsConfigsResponse.Configurations != NULL) && (videoAnalyticsConfigsResponse.Configurations->token != NULL))
                {
                    if(StringNCopy(onvifCamInfo[camIndex].analyticsServiceInfo.videoAnalticsToken,
                                   videoAnalyticsConfigsResponse.Configurations->token, MAX_TOKEN_SIZE) == SUCCESS)
                    {
                        capabilitySupported |= MX_ADD(MOTION_WIN_CONFIG_SUPPORT);
                    }
                }
                else
                {
                    EPRINT(ONVIF_CLIENT, "video analytics config not available: [camera=%d]", camIndex);
                }
            }
            else
            {
                EPRINT(ONVIF_CLIENT, "fail to get video analytics config: [camera=%d], [soapResp=%d]", camIndex, soapResp);
            }
        }

        /* save camera capabilities */
        SaveOnvifCameraCapabilty(camIndex, capabilitySupported);
        status = ONVIF_CMD_SUCCESS;
    }
    while(0);

    if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
    {
        status = ONVIF_CMD_TIMEOUT;
    }

    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief addResolutionToCamCapability  : for media 1 services
 * @param resolutionAvailable           : Resolution
 * @param maxResolutionAvailable        : Total fields
 * @param resolutionVal                 : Resolution string
 * @param alreadyStored                 : Flag to check already present or not
 * @return
 */
static BOOL addResolutionToCamCapability(VIDEO_RESOLUTION_t *resolutionAvailable, UINT8 maxResolutionAvailable,
                                         UINT8PTR resolutionVal, UINT8 alreadyStored)
{
    UINT8 			addIndex = alreadyStored;
    RESOLUTION_e 	resolutionNum;
    UINT8			resIdx, tmpIdx;

    for (resIdx = 0; resIdx < maxResolutionAvailable; resIdx++)
    {
        //  String ==> Resolution Enum
        if (FAIL == GetResolutionId((resolutionAvailable + resIdx)->Width, (resolutionAvailable + resIdx)->Height, &resolutionNum))
        {
            continue;
        }

        // Check if it already added
        for (tmpIdx = 0; tmpIdx < addIndex; tmpIdx++)
        {
            if (resolutionVal[tmpIdx] == (UINT8)resolutionNum)
            {
                break;
            }
        }

        // Add this resolution
        if (tmpIdx == addIndex)
        {
            resolutionVal[addIndex++] = (UINT8)resolutionNum;
        }
    }

    resolutionVal[addIndex] = MAX_RESOLUTION;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief addResolutionToCamCapability2     : For media2 services
 * @param resolutionAvailable               : Resolution
 * @param maxResolutionAvailable            : Total field
 * @param resolutionVal                     : Resolution value
 * @param alreadyStored                     : flag
 * @return
 */
static BOOL addResolutionToCamCapability2(VIDEO_RESOLUTION2_t *resolutionAvailable, UINT8 maxResolutionAvailable,
                                          UINT8PTR resolutionVal, UINT8 alreadyStored)
{
    UINT8 			addIndex = alreadyStored;
    RESOLUTION_e 	resolutionNum;
    UINT8			resIdx, tmpIdx;

    for (resIdx = 0; resIdx < maxResolutionAvailable; resIdx++)
    {
        //  String ==> Resolution Enum
        if (FAIL == GetResolutionId((resolutionAvailable + resIdx)->Width, (resolutionAvailable + resIdx)->Height, &resolutionNum))
        {
            continue;
        }

        // Check if it already added
        for (tmpIdx = 0; tmpIdx < addIndex; tmpIdx++)
        {
            if (resolutionVal[tmpIdx] == (UINT8)resolutionNum)
            {
                break;
            }
        }

        // Add this resolution
        if (tmpIdx == addIndex)
        {
            resolutionVal[addIndex++] = (UINT8)resolutionNum;
        }
    }

    resolutionVal[addIndex] = MAX_RESOLUTION;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief prepareMediaCapabilityFromEncoderConfigOptions    : Parse steam configuration and save to NVR
 * @param camIndex                                          : Camera index
 * @param profileNum                                        : Profile Index
 * @param videoEncoderConfigOptions                         : received from camera
 * @return
 */
static BOOL prepareMediaCapabilityFromEncoderConfigOptions(UINT8 camIndex, UINT8 profileNum,
                                                           GET_VIDEO_ENCODER_CONFIGURATION_OPTIONS_RESPONSE_t *videoEncoderConfigOptions)
{
    UINT8                       maxResolutionAvailable;
    VIDEO_RESOLUTION_t          *resoultionAvailable = NULL;
    FRAME_RATE_RANGE_t          *framerate = NULL;
    UINT8PTR                    *pRes = NULL;
    UINT8                       codec = VIDEO_MJPG;
    UINT8                       index = 0;
    BOOL                        codecParameterAvailable;
    CHARPTR                     tempExtensionStr = NULL;
    PROFILE_WISE_OPTION_PARAM_t	*tempProfileParam = NULL;

    if ((tempProfileParam = malloc(sizeof(PROFILE_WISE_OPTION_PARAM_t))) == NULL)
    {
        EPRINT(ONVIF_CLIENT, "fail to allocate profile memory: [camera=%d], [profile=%d]", camIndex, profileNum);
        return FAIL;
    }

    memset(tempProfileParam, '\0', sizeof(PROFILE_WISE_OPTION_PARAM_t));

    /* Allocate memory for resolution */
    tempProfileParam->resolutionSupported.isResolutionCodecDependent = YES;
    if ((tempProfileParam->resolutionSupported.resolutionAvailable = malloc(sizeof(UINT8PTR) * GET_VALID_CODEC_NUM(MAX_VIDEO_CODEC))) == NULL)
    {
        EPRINT(ONVIF_CLIENT, "fail to allocate resolution memory: [camera=%d], [profile=%d]", camIndex, profileNum);
        free(tempProfileParam);
        return FAIL;
    }

    /* initially, no nesolutions available for all codec */
    for (codec = VIDEO_MJPG; codec < MAX_VIDEO_CODEC; codec++)
    {
        ((UINT8PTR *)(tempProfileParam->resolutionSupported.resolutionAvailable))[GET_VALID_CODEC_NUM(codec)] = NULL;
        tempProfileParam->qualitySupported[GET_VALID_CODEC_NUM(codec)] = 0;
    }

    /* Allocate memory for framerate */
    tempProfileParam->framerateSupported.isFramerateCodecDependent = YES;
    tempProfileParam->framerateSupported.isFramerateResolutionDependent = NO;
    if ((tempProfileParam->framerateSupported.framerateAvailable = malloc(sizeof(UINT64) * GET_VALID_CODEC_NUM(MAX_VIDEO_CODEC))) == NULL)
    {
        EPRINT(ONVIF_CLIENT, "fail to allocate framerate memory: [camera=%d], [profile=%d]", camIndex, profileNum);
        free(tempProfileParam->resolutionSupported.resolutionAvailable);
        free(tempProfileParam);
        return FAIL;
    }

    for (codec = VIDEO_MJPG; codec < MAX_VIDEO_CODEC; codec++)
    {
        /* initially, no framerate & bitrate available for codec */
        tempProfileParam->framerateSupported.framerateAvailable[GET_VALID_CODEC_NUM(codec)] = 0;
        tempProfileParam->minBitRateSupport[GET_VALID_CODEC_NUM(codec)] = 0;
        tempProfileParam->maxBitRateSupport[GET_VALID_CODEC_NUM(codec)] = 0;
        codecParameterAvailable = NO;

        switch(codec)
        {
            case VIDEO_MJPG:
                if(videoEncoderConfigOptions->Options->JPEG != NULL)
                {
                    codecParameterAvailable = YES;
                    resoultionAvailable 	= videoEncoderConfigOptions->Options->JPEG->ResolutionsAvailable;
                    maxResolutionAvailable 	= videoEncoderConfigOptions->Options->JPEG->__sizeResolutionsAvailable;
                    framerate				= videoEncoderConfigOptions->Options->JPEG->FrameRateRange;
                }
                break;

            case VIDEO_H264:
                if(videoEncoderConfigOptions->Options->H264 != NULL)
                {
                    codecParameterAvailable = YES;
                    resoultionAvailable 	= videoEncoderConfigOptions->Options->H264->ResolutionsAvailable;
                    maxResolutionAvailable 	= videoEncoderConfigOptions->Options->H264->__sizeResolutionsAvailable;
                    framerate				= videoEncoderConfigOptions->Options->H264->FrameRateRange;
                }
                break;

            case VIDEO_MPEG4:
                if(videoEncoderConfigOptions->Options->MPEG4 != NULL)
                {
                    codecParameterAvailable = YES;
                    resoultionAvailable 	= videoEncoderConfigOptions->Options->MPEG4->ResolutionsAvailable;
                    maxResolutionAvailable 	= videoEncoderConfigOptions->Options->MPEG4->__sizeResolutionsAvailable;
                    framerate				= videoEncoderConfigOptions->Options->MPEG4->FrameRateRange;
                }
                break;

            default:
                break;
        }

        if (codecParameterAvailable == YES)
        {
            tempProfileParam->codecSupported |= MX_ADD(codec);
            if (resoultionAvailable != NULL)
            {
                pRes = &(((UINT8PTR *)(tempProfileParam->resolutionSupported.resolutionAvailable))[GET_VALID_CODEC_NUM(codec)]);
                if (*pRes == NULL)
                {
                    /* Memory deallocated at other place (FreeCameraCapability) */
                    if((*pRes = malloc((sizeof(UINT8) * maxResolutionAvailable) + 1)) != NULL)
                    {
                        addResolutionToCamCapability(resoultionAvailable, maxResolutionAvailable, *pRes, 0);
                    }
                }
            }

            if (framerate != NULL)
            {
                tempProfileParam->framerateSupported.framerateAvailable[GET_VALID_CODEC_NUM(codec)] = AddRange(framerate->Min, framerate->Max);
            }

            if (videoEncoderConfigOptions->Options->QualityRange != NULL)
            {
                tempProfileParam->qualitySupported[GET_VALID_CODEC_NUM(codec)] =
                        ((videoEncoderConfigOptions->Options->QualityRange->Max - videoEncoderConfigOptions->Options->QualityRange->Min) + 1);
            }
        }
    }

    if(videoEncoderConfigOptions->Options->Extension != NULL)
    {
        for(index = 0; index < videoEncoderConfigOptions->Options->Extension->__size; index++)
        {
            if(videoEncoderConfigOptions->Options->Extension->__any[index].name != NULL)
            {
                if(videoEncoderConfigOptions->Options->JPEG != NULL)
                {
                    tempExtensionStr = strstr(videoEncoderConfigOptions->Options->Extension->__any[index].name, "JPEG");
                    if(tempExtensionStr != NULL)
                    {
                        if(parseBitrateMinMaxValue(tempExtensionStr, &tempProfileParam->minBitRateSupport[GET_VALID_CODEC_NUM(VIDEO_MJPG)],
                                                   &tempProfileParam->maxBitRateSupport[GET_VALID_CODEC_NUM(VIDEO_MJPG)]) == TRUE)
                        {
                            continue;
                        }
                    }
                }

                if(videoEncoderConfigOptions->Options->H264 != NULL)
                {
                    tempExtensionStr = strstr(videoEncoderConfigOptions->Options->Extension->__any[index].name, "H264");
                    if(tempExtensionStr != NULL)
                    {
                        if(parseBitrateMinMaxValue(tempExtensionStr, &tempProfileParam->minBitRateSupport[GET_VALID_CODEC_NUM(VIDEO_H264)],
                                                   &tempProfileParam->maxBitRateSupport[GET_VALID_CODEC_NUM(VIDEO_H264)]) == TRUE)
                        {
                            continue;
                        }
                    }
                }

                if(videoEncoderConfigOptions->Options->MPEG4 != NULL)
                {
                    tempExtensionStr = strstr(videoEncoderConfigOptions->Options->Extension->__any[index].name, "MPEG4");
                    if(tempExtensionStr != NULL)
                    {
                        if(parseBitrateMinMaxValue(tempExtensionStr, &tempProfileParam->minBitRateSupport[GET_VALID_CODEC_NUM(VIDEO_MPEG4)],
                                                   &tempProfileParam->maxBitRateSupport[GET_VALID_CODEC_NUM(VIDEO_MPEG4)]) == TRUE)
                        {
                            continue;
                        }
                    }
                }
            }
        }
    }

    /* Memory deallocated at other place (FreeCameraCapability) */
    SaveOnvifCameraCapabiltyProfileWise(camIndex, profileNum, tempProfileParam);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief prepareMedia2CapabilityFromEncoderConfigOptions   : Parse steam configuration and save to NVR
 * @param camIndex                                          : Camera Index
 * @param profileNum                                        : Profile Number
 * @param videoEncoderConfigOptions                         : Received from camera
 * @return
 */
static BOOL prepareMedia2CapabilityFromEncoderConfigOptions(UINT8 camIndex, UINT8 profileNum,
                                                            GET_VIDEO_ENCODER_CONFIGURATION_OPTIONS_RESPONSE2_t *videoEncoderConfigOptions)
{
    UINT8                       maxResolutionAvailable;
    VIDEO_RESOLUTION2_t         *resoultionAvailable2;
    UINT8PTR                    *pRes;
    UINT8                       codec = VIDEO_MJPG;
    UINT8                       index = 0;
    PROFILE_WISE_OPTION_PARAM_t *tempProfileParam;
    UINT8                       minFPS;
    UINT8                       maxFPS;

    tempProfileParam = malloc(sizeof(PROFILE_WISE_OPTION_PARAM_t));
    if(tempProfileParam == NULL)
    {
        EPRINT(ONVIF_CLIENT, "fail to allocate profile memory: [camera=%d], [profile=%d]", camIndex, profileNum);
        return FAIL;
    }

    memset(tempProfileParam, '\0', sizeof(PROFILE_WISE_OPTION_PARAM_t));

    /* Allocate memory for resolution */
    tempProfileParam->resolutionSupported.isResolutionCodecDependent = YES;
    tempProfileParam->resolutionSupported.resolutionAvailable = malloc(sizeof(UINT8PTR) * GET_VALID_CODEC_NUM(MAX_VIDEO_CODEC));
    if(tempProfileParam->resolutionSupported.resolutionAvailable == NULL)
    {
        EPRINT(ONVIF_CLIENT, "fail to allocate resolution memory: [camera=%d], [profile=%d]", camIndex, profileNum);
        free(tempProfileParam);
        tempProfileParam = NULL;
        return FAIL;
    }

    /* Allocate memory for framerate */
    tempProfileParam->framerateSupported.isFramerateCodecDependent      = YES;
    tempProfileParam->framerateSupported.isFramerateResolutionDependent = NO;
    tempProfileParam->framerateSupported.framerateAvailable = malloc(sizeof(UINT64) * GET_VALID_CODEC_NUM(MAX_VIDEO_CODEC));
    if (tempProfileParam->framerateSupported.framerateAvailable == NULL)
    {
        EPRINT(ONVIF_CLIENT, "fail to allocate framerate memory: [camera=%d], [profile=%d]", camIndex, profileNum);
        free(tempProfileParam->resolutionSupported.resolutionAvailable);
        free(tempProfileParam);
        return FAIL;
    }

    /* initially, no nesolution, framerate, quality and bitrate available for all codec */
    for (codec = VIDEO_MJPG; codec < MAX_VIDEO_CODEC; codec++)
    {
        ((UINT8PTR *)(tempProfileParam->resolutionSupported.resolutionAvailable))[GET_VALID_CODEC_NUM(codec)] = NULL;
        tempProfileParam->framerateSupported.framerateAvailable[GET_VALID_CODEC_NUM(codec)] = 0;
        tempProfileParam->qualitySupported[GET_VALID_CODEC_NUM(codec)] = 0;
        tempProfileParam->minBitRateSupport[GET_VALID_CODEC_NUM(codec)] = 0;
        tempProfileParam->maxBitRateSupport[GET_VALID_CODEC_NUM(codec)] = 0;
    }

    for (index = 0; index < videoEncoderConfigOptions->__sizeOptions; index++)
    {
        //Required Field
        if(videoEncoderConfigOptions->Options[index].Encoding == NULL)
        {
            EPRINT(ONVIF_CLIENT, "no enconding config found: [camera=%d], [profile=%d], [subProfile=%d]", camIndex, profileNum, index);
            continue;
        }

        if(strcmp(videoEncoderConfigOptions->Options[index].Encoding, "H264") == 0)
        {
            codec = VIDEO_H264;
            if(videoEncoderConfigOptions->Options[index].ProfilesSupported != NULL)
            {
                if(strstr(videoEncoderConfigOptions->Options[index].ProfilesSupported, MEDIA_BASELINE_PROFILE) != NULL)
                {
                    tempProfileParam->profileSupported[GET_VALID_CODEC_NUM(codec)] = MEDIA_BASELINE_PROFILE;
                }
                else if(strstr(videoEncoderConfigOptions->Options[index].ProfilesSupported, MEDIA_MAIN_PROFILE) != NULL)
                {
                    tempProfileParam->profileSupported[GET_VALID_CODEC_NUM(codec)] = MEDIA_MAIN_PROFILE;
                }
                else if(strstr(videoEncoderConfigOptions->Options[index].ProfilesSupported, MEDIA_EXTENDED_PROFILE) != NULL)
                {
                    tempProfileParam->profileSupported[GET_VALID_CODEC_NUM(codec)] = MEDIA_EXTENDED_PROFILE;
                }
                else if(strstr(videoEncoderConfigOptions->Options[index].ProfilesSupported, MEDIA_HIGH_PROFILE) != NULL)
                {
                    tempProfileParam->profileSupported[GET_VALID_CODEC_NUM(codec)] = MEDIA_HIGH_PROFILE;
                }
                else
                {
                    tempProfileParam->profileSupported[GET_VALID_CODEC_NUM(codec)] = MEDIA_BASELINE_PROFILE; //Default case
                }
            }
        }
        else if(strcmp(videoEncoderConfigOptions->Options[index].Encoding, "H265") == 0)
        {
            codec = VIDEO_H265;
            if(videoEncoderConfigOptions->Options[index].ProfilesSupported != NULL)
            {
                if(strstr(videoEncoderConfigOptions->Options[index].ProfilesSupported, MEDIA_MAIN10_PROFILE) != NULL)
                {
                    tempProfileParam->profileSupported[GET_VALID_CODEC_NUM(codec)] = MEDIA_MAIN10_PROFILE;
                }
                else if(strstr(videoEncoderConfigOptions->Options[index].ProfilesSupported, MEDIA_MAIN_PROFILE) != NULL)
                {
                    tempProfileParam->profileSupported[GET_VALID_CODEC_NUM(codec)] = MEDIA_MAIN_PROFILE;
                }
                else
                {
                    tempProfileParam->profileSupported[GET_VALID_CODEC_NUM(codec)] = MEDIA_MAIN_PROFILE; //Default case
                }
            }
        }
        else if(strcmp(videoEncoderConfigOptions->Options[index].Encoding, "JPEG") == 0)
        {
            codec = VIDEO_MJPG;
            tempProfileParam->profileSupported[GET_VALID_CODEC_NUM(codec)] = NULL;
        }
        else if(strcmp(videoEncoderConfigOptions->Options[index].Encoding, "MPV4-ES") == 0)
        {
            codec = VIDEO_MPEG4;
            tempProfileParam->profileSupported[GET_VALID_CODEC_NUM(codec)] = NULL;
        }
        else
        {
            EPRINT(ONVIF_CLIENT, "invld encoding found: [camera=%d] [index=%d], [ecoding=%s]",
                   camIndex, index, videoEncoderConfigOptions->Options[index].Encoding);
            continue;
        }

        tempProfileParam->codecSupported |= MX_ADD(codec);

        //parse FPS supported
        if (videoEncoderConfigOptions->Options[index].FrameRatesSupported != NULL)
        {
            if(parseMedia2FrameRate(videoEncoderConfigOptions->Options[index].FrameRatesSupported, &minFPS, &maxFPS) == SUCCESS)
            {
                tempProfileParam->framerateSupported.framerateAvailable[GET_VALID_CODEC_NUM(codec)] = AddRange(minFPS, maxFPS);
            }
            else
            {
                EPRINT(ONVIF_CLIENT, "fail to parse min & max fps: [camera=%d], [fps=%s]",
                       camIndex, videoEncoderConfigOptions->Options[index].FrameRatesSupported);
            }
        }
        else
        {
            EPRINT(ONVIF_CLIENT, "no framrate list for profile: [camera=%d], [profile=%d], [encoding=%s]",
                   camIndex, profileNum, videoEncoderConfigOptions->Options[index].Encoding);
        }

        //parse resolution available
        resoultionAvailable2 	= videoEncoderConfigOptions->Options[index].ResolutionsAvailable;
        maxResolutionAvailable 	= videoEncoderConfigOptions->Options[index].__sizeResolutionsAvailable;
        if ((maxResolutionAvailable != 0) && (resoultionAvailable2 != NULL))
        {
            pRes = &(((UINT8PTR *)(tempProfileParam->resolutionSupported.resolutionAvailable))[GET_VALID_CODEC_NUM(codec)]);
            if (*pRes == NULL)
            {
                /* Memory deallocated at other place (FreeCameraCapability) */
                if((*pRes = malloc((sizeof(UINT8) * maxResolutionAvailable) + 1)) != NULL)
                {
                    addResolutionToCamCapability2(resoultionAvailable2, maxResolutionAvailable, *pRes, 0);
                }
            }
            else
            {
                EPRINT(ONVIF_CLIENT, "resolution already avaialble for codec: [camera=%d], [profile=%d], [encoding=%s]",
                       camIndex, profileNum, videoEncoderConfigOptions->Options[index].Encoding);
            }
        }
        else
        {
            EPRINT(ONVIF_CLIENT, "no resolution available: [camera=%d], [profile=%d], [encoding=%s]",
                   camIndex, profileNum, videoEncoderConfigOptions->Options[index].Encoding);
        }

        //parse quality range
        if(videoEncoderConfigOptions->Options[index].QualityRange != NULL)
        {
            tempProfileParam->qualitySupported[GET_VALID_CODEC_NUM(codec)] =
                    (UINT8)((videoEncoderConfigOptions->Options[index].QualityRange->Max - videoEncoderConfigOptions->Options[index].QualityRange->Min) + 1);

            //Required for set command
            onvifCamInfo[camIndex].mediaServiceInfo.initialQuality = (UINT8)videoEncoderConfigOptions->Options[index].QualityRange->Min;
        }
        else
        {
            EPRINT(ONVIF_CLIENT, "no quality range available: [camera=%d], [profile=%d], [encoding=%s]",
                   camIndex, profileNum, videoEncoderConfigOptions->Options[index].Encoding);
        }

        //parse bitrate range
        if(videoEncoderConfigOptions->Options[index].BitrateRange != NULL)
        {
            tempProfileParam->minBitRateSupport[GET_VALID_CODEC_NUM(codec)] = videoEncoderConfigOptions->Options[index].BitrateRange->Min;
            tempProfileParam->maxBitRateSupport[GET_VALID_CODEC_NUM(codec)] = videoEncoderConfigOptions->Options[index].BitrateRange->Max;
        }
        else
        {
            EPRINT(ONVIF_CLIENT, "no bitrate range available: [camera=%d], [profile=%d], [encoding=%s]",
                   camIndex, profileNum, videoEncoderConfigOptions->Options[index].Encoding);
        }
    }

    /* Memory deallocated at other place (FreeCameraCapability) */
    SaveOnvifCameraCapabiltyProfileWise(camIndex, profileNum, tempProfileParam);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief parseBitrateMinMaxValue   : Parse minimum and maximum bitrate value
 * @param sourceStr                 : Input string
 * @param minValue                  : Minimum bitrate supported
 * @param maxValue                  : Max bitrate supported
 * @return
 */
static BOOL parseBitrateMinMaxValue(CHARPTR sourceStr, UINT16PTR minValue, UINT16PTR maxValue)
{
    CHARPTR endCharPtr = NULL;
    CHAR bitRateStr[10] = "";

    sourceStr = strstr(sourceStr, ONVIF_BITRATE_RANGE_STR);
    if (sourceStr == NULL)
    {
        EPRINT(ONVIF_CLIENT, "bitrate string not found");
        return FALSE;
    }

    sourceStr = strstr(sourceStr, ONVIF_BITRATE_RANGE_MIN_STR);
    if(sourceStr == NULL)
    {
        EPRINT(ONVIF_CLIENT, "min bitrate string not found");
        return FAIL;
    }

    sourceStr += (sizeof(ONVIF_BITRATE_RANGE_MIN_STR) - 1);
    endCharPtr = strchr(sourceStr, '<');
    if (endCharPtr != NULL)
    {
        strncpy(bitRateStr, sourceStr, (endCharPtr - sourceStr));
        bitRateStr[(endCharPtr - sourceStr) + 1] = '\0';
        *minValue = atoi(bitRateStr);
    }

    bitRateStr[0] = '\0';
    sourceStr = strstr(sourceStr, ONVIF_BITRATE_RANGE_MAX_STR);
    if(sourceStr == NULL)
    {
        EPRINT(ONVIF_CLIENT, "max bitrate string not found");
        return FALSE;
    }

    sourceStr += (sizeof(ONVIF_BITRATE_RANGE_MAX_STR) - 1);
    endCharPtr = strchr(sourceStr, '<');
    if (endCharPtr != NULL)
    {
        strncpy(bitRateStr, sourceStr, (endCharPtr - sourceStr));
        bitRateStr[(endCharPtr - sourceStr) + 1] = '\0';
        *maxValue = atoi(bitRateStr);
    }

    DPRINT(ONVIF_CLIENT, "bitrate rang: [min=%d], [max=%d]", *minValue, *maxValue);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief fillOtherSupportedCapabiltyToCamera   : Check input ouput support
 * @param camCapability                         : Supported List
 * @param getCapabilitiesResponse               : Capability received from camera
 */
static void fillOtherSupportedCapabiltyToCamera(CAPABILITY_TYPE *camCapability, GET_CAPABILITIES_RESPONSE_t *getCapabilitiesResponse)
{
    UINT8 alarmInput;

    if (getCapabilitiesResponse->Capabilities->Device->IO == NULL)
    {
        return;
    }

    for (alarmInput = 0; (alarmInput < (*(getCapabilitiesResponse->Capabilities->Device->IO->InputConnectors)) && (alarmInput < MAX_CAMERA_ALARM)); alarmInput++)
    {
        (*camCapability) |= MX_ADD((alarmInput + CAMERA_ALARM_INPUT1_SUPPORT));
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief selectProfileForMediaService  : will store configuration for all supported profiles
 * @param soap                          : SOAP Instance
 * @param user                          : User information
 * @param ptzSupported                  : if PTZ support or not
 * @param camIndex                      : Camera index
 * @return
 */
static ONVIF_RESP_STATUS_e selectProfileForMediaService(SOAP_t *soap, SOAP_USER_DETAIL_t *user, BOOL ptzSupported, UINT8 camIndex)
{
    GET_PROFILES_t                                          getProfiles;
    GET_PROFILES_RESPONSE_t                                 getProfilesResponse;
    GET_COMPATIBLE_VIDEO_SOURCE_CONFIGURATIONS_t            getCompatibleVideoSourceConfigurations;
    GET_COMPATIBLE_VIDEO_SOURCE_CONFIGURATIONS_RESPONSE_t   getCompatibleVideoSourceConfigurationsResponse;
    GET_COMPATIBLE_VIDEO_ENCODER_CONFIGURATIONS_t           getCompatibleVideoEncoderConfigurations;
    GET_COMPATIBLE_VIDEO_ENCODER_CONFIGURATIONS_RESPONSE_t  getCompatibleVideoEncoderConfigurationsResponse;
    GET_VIDEO_ENCODER_CONFIGURATION_OPTIONS_t               getVideoEncoderConfigurationOptions;
    GET_VIDEO_ENCODER_CONFIGURATION_OPTIONS_RESPONSE_t      getVideoEncoderConfigurationOptionsResponse;
    INT16                                                   soapResp;
    UINT8                                                   profileIndex;
    UINT8                                                   profileCnt = 0;
    BOOL                                                    ptzProfileAvailable = NO;
    ONVIF_RESP_STATUS_e                                     status = ONVIF_CMD_FAIL;
    UINT8                                                   supportedProfileCount = 0;
    CHARPTR                                                 profileToken;
    CHARPTR                                                 vdoSrcCfgToken;
    CHARPTR                                                 vdoEncCfgToken;
    ONVIF_MEDIA_SERVICE_INFO_t                              *pMediaService = &(onvifCamInfo[camIndex].mediaServiceInfo);

    user->addr = onvifCamInfo[camIndex].mediaServiceInfo.mediaServiceAddr;
    soapResp = GetProfiles(soap, &getProfiles, &getProfilesResponse, user);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "fail to get profiles: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
        {
            return ONVIF_CMD_TIMEOUT;
        }
        return ONVIF_CMD_FAIL;
    }

    if (getProfilesResponse.__sizeProfiles <= 0)
    {
        EPRINT(ONVIF_CLIENT, "no profiles found: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    if (getProfilesResponse.Profiles == NULL)
    {
        EPRINT(ONVIF_CLIENT, "profiles not available: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    for (profileIndex = 0; profileIndex < getProfilesResponse.__sizeProfiles; profileIndex++)
    {
        profileToken = getProfilesResponse.Profiles[profileIndex].token;
        if(profileToken == NULL)
        {
            EPRINT(ONVIF_CLIENT, "profile token not available: [camera=%d], [profile=%d]", camIndex, profileIndex);
            continue;
        }

        //Set the PTZ Profile for PTZ operation
        if ((ptzProfileAvailable == NO) && (ptzSupported == YES))
        {
            if((getProfilesResponse.Profiles[profileIndex].PTZConfiguration != NULL)
                    && (getProfilesResponse.Profiles[profileIndex].PTZConfiguration->token != NULL))
            {
                StringNCopy(onvifCamInfo[camIndex].ptzServiceInfo.profileToken, profileToken, MAX_TOKEN_SIZE);
                StringNCopy(onvifCamInfo[camIndex].ptzServiceInfo.ptzConfigurationToken,
                            getProfilesResponse.Profiles[profileIndex].PTZConfiguration->token, MAX_TOKEN_SIZE);
                ptzProfileAvailable = YES;
            }
        }

        //Now check whether Video Source configuration already available in profile or not
        if (getProfilesResponse.Profiles[profileIndex].VideoSourceConfiguration != NULL)
        {
            vdoSrcCfgToken = getProfilesResponse.Profiles[profileIndex].VideoSourceConfiguration->token;
        }
        else
        {
            //video source configurations not available then get its compatible source configuration
            getCompatibleVideoSourceConfigurations.ProfileToken = profileToken;
            soapResp = GetCompatibleVideoSourceConfigurations(soap, &getCompatibleVideoSourceConfigurations, &getCompatibleVideoSourceConfigurationsResponse, user);
            if ((soapResp == SOAP_OK) && (getCompatibleVideoSourceConfigurationsResponse.Configurations != NULL))
            {
                vdoSrcCfgToken = getCompatibleVideoSourceConfigurationsResponse.Configurations->token;
            }
            else
            {
                EPRINT(ONVIF_CLIENT, "fail to get compatible video source config: [camera=%d], [profile=%d], [soapResp=%d]", camIndex, profileIndex, soapResp);
                continue;
            }
        }

        //Now check whether Video Encoder configuration already available in profile or not
        if (getProfilesResponse.Profiles[profileIndex].VideoEncoderConfiguration != NULL)
        {
            vdoEncCfgToken = getProfilesResponse.Profiles[profileIndex].VideoEncoderConfiguration->token;
        }
        else
        {
            //video encoder configurations not available then get its compatible encoder configuration
            getCompatibleVideoEncoderConfigurations.ProfileToken = profileToken;
            soapResp = GetCompatibleVideoEncoderConfigurations(soap, &getCompatibleVideoEncoderConfigurations, &getCompatibleVideoEncoderConfigurationsResponse, user);
            if ((soapResp == SOAP_OK) && (getCompatibleVideoEncoderConfigurationsResponse.Configurations != NULL))
            {
                vdoEncCfgToken = getCompatibleVideoEncoderConfigurationsResponse.Configurations->token;
            }
            else
            {
                EPRINT(ONVIF_CLIENT, "fail to get compatible video encoder config: [camera=%d], [profile=%d], [soapResp=%d]", camIndex, profileIndex, soapResp);
                continue;
            }
        }

        getVideoEncoderConfigurationOptions.ProfileToken = profileToken;
        getVideoEncoderConfigurationOptions.ConfigurationToken = NULL;
        soapResp = GetVideoEncoderConfigurationOptions(soap, &getVideoEncoderConfigurationOptions, &getVideoEncoderConfigurationOptionsResponse, user);
        if ((soapResp != SOAP_OK) || (getVideoEncoderConfigurationOptionsResponse.Options == NULL))
        {
            EPRINT(ONVIF_CLIENT, "fail to get video encoder config options: [camera=%d], [profile=%d], [soapResp=%d]", camIndex, profileIndex, soapResp);
            continue;
        }

        // if profile support one or more codec then select that profile for that codec streaming
        if(getVideoEncoderConfigurationOptionsResponse.Options->JPEG != NULL)
        {
            pMediaService->mediaProfile[profileCnt].defaultEncodingInterval[GET_VALID_CODEC_NUM(VIDEO_MJPG)] = 0;
            if(getVideoEncoderConfigurationOptionsResponse.Options->JPEG->EncodingIntervalRange != NULL)
            {
                pMediaService->mediaProfile[profileCnt].defaultEncodingInterval[GET_VALID_CODEC_NUM(VIDEO_MJPG)] =
                        getVideoEncoderConfigurationOptionsResponse.Options->JPEG->EncodingIntervalRange->Min;
            }
        }

        if(getVideoEncoderConfigurationOptionsResponse.Options->H264 != NULL)
        {
            pMediaService->h264SupportedProfile = H264_HIGH_PROFILE;

            if (getVideoEncoderConfigurationOptionsResponse.Options->H264->__sizeH264ProfilesSupported > 0)
            {
                supportedProfileCount = 0;

                while (supportedProfileCount != getVideoEncoderConfigurationOptionsResponse.Options->H264->__sizeH264ProfilesSupported)
                {
                    if(*(getVideoEncoderConfigurationOptionsResponse.Options->H264->H264ProfilesSupported + supportedProfileCount)
                            < pMediaService->h264SupportedProfile)
                    {
                        pMediaService->h264SupportedProfile =
                                *(getVideoEncoderConfigurationOptionsResponse.Options->H264->H264ProfilesSupported+supportedProfileCount);
                    }
                    supportedProfileCount++;
                }
            }

            pMediaService->mediaProfile[profileCnt].defaultEncodingInterval[GET_VALID_CODEC_NUM(VIDEO_H264)] = 0;
            if(getVideoEncoderConfigurationOptionsResponse.Options->H264->EncodingIntervalRange != NULL)
            {
                pMediaService->mediaProfile[profileCnt].defaultEncodingInterval[GET_VALID_CODEC_NUM(VIDEO_H264)] =
                        getVideoEncoderConfigurationOptionsResponse.Options->H264->EncodingIntervalRange->Min;
            }
        }

        if (getVideoEncoderConfigurationOptionsResponse.Options->MPEG4 != NULL)
        {
            supportedProfileCount = 0;
            pMediaService->mpeg4SupportedProfile = MPEG4_ASP_PROFILE;

            if(getVideoEncoderConfigurationOptionsResponse.Options->MPEG4->__sizeMpeg4ProfilesSupported > 0)
            {
                while(supportedProfileCount != getVideoEncoderConfigurationOptionsResponse.Options->MPEG4->__sizeMpeg4ProfilesSupported)
                {
                    if(*(getVideoEncoderConfigurationOptionsResponse.Options->MPEG4->Mpeg4ProfilesSupported+supportedProfileCount)
                            < pMediaService->mpeg4SupportedProfile)
                    {
                        pMediaService->mpeg4SupportedProfile =
                                *(getVideoEncoderConfigurationOptionsResponse.Options->MPEG4->Mpeg4ProfilesSupported+supportedProfileCount);
                    }
                    supportedProfileCount++;
                }
            }

            pMediaService->mediaProfile[profileCnt].defaultEncodingInterval[GET_VALID_CODEC_NUM(VIDEO_MPEG4)] = 0;
            if(getVideoEncoderConfigurationOptionsResponse.Options->MPEG4->EncodingIntervalRange != NULL)
            {
                pMediaService->mediaProfile[profileCnt].defaultEncodingInterval[GET_VALID_CODEC_NUM(VIDEO_MPEG4)] =
                        getVideoEncoderConfigurationOptionsResponse.Options->MPEG4->EncodingIntervalRange->Min;
            }
        }

        /* Reset all data of current profile */
        pMediaService->mediaProfile[profileCnt].profileToken[0] = '\0';
        pMediaService->mediaProfile[profileCnt].videoSourceToken[0] = '\0';
        pMediaService->mediaProfile[profileCnt].videoEncoderConfigurationToken[0] = '\0';
        FreeCameraCapabilityProfileWise(camIndex, profileCnt);

        if (FAIL == prepareMediaCapabilityFromEncoderConfigOptions(camIndex, profileCnt, &getVideoEncoderConfigurationOptionsResponse))
        {
            EPRINT(ONVIF_CLIENT, "fail to store video encoder config: [camera=%d], [profile=%d], [profileToken=%s], [vdoSrcCfgToken=%s], [vdoEncCfgToken=%s]",
                   camIndex, profileIndex, profileToken, vdoSrcCfgToken, vdoEncCfgToken);
            continue;
        }

        StringNCopy(pMediaService->mediaProfile[profileCnt].profileToken, profileToken, MAX_TOKEN_SIZE);
        StringNCopy(pMediaService->mediaProfile[profileCnt].videoSourceToken, vdoSrcCfgToken, MAX_TOKEN_SIZE);
        StringNCopy(pMediaService->mediaProfile[profileCnt].videoEncoderConfigurationToken, vdoEncCfgToken, MAX_TOKEN_SIZE);

        DPRINT(ONVIF_CLIENT, "video encoder config stored: [camera=%d], profile: [nvr=%d], [cam=%d], [token=%s]",
               camIndex, profileCnt, profileIndex, profileToken);

        profileCnt++;
        status = ONVIF_CMD_SUCCESS;

        /* Profiles must not increase to supported by us */
        if (profileCnt >= MAX_PROFILE_SUPPORT)
        {
            break;
        }
    }
    onvifCamInfo[camIndex].maxSupportedProfile = profileCnt;

    if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
    {
        status = ONVIF_CMD_TIMEOUT;
    }

    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief selectProfileForMedia2Service : will store configuration for all supported profiles
 * @param soap                          : SOAP instance
 * @param user                          : User Information
 * @param ptzSupported                  : if PTZ support or not
 * @param camIndex                      : Camera Index
 * @return
 */
static ONVIF_RESP_STATUS_e selectProfileForMedia2Service(SOAP_t *soap, SOAP_USER_DETAIL_t *user, BOOL ptzSupported, UINT8 camIndex)
{
    GET_PROFILES2_t                                         getProfiles;
    GET_PROFILES_RESPONSE2_t                                getProfilesResponse;
    GET_VIDEO_ENCODER_CONFIGURATION_OPTIONS2_t              getVideoEncoderConfigurationOptions2;
    GET_VIDEO_ENCODER_CONFIGURATION_OPTIONS_RESPONSE2_t     getVideoEncoderConfigurationOptionsResponse2;
    INT16                                                   soapResp;
    BOOL                                                    ptzProfileAvailable = NO;
    ONVIF_RESP_STATUS_e                                     status = ONVIF_CMD_FAIL;
    CHARPTR                                                 profileToken;
    CHARPTR                                                 vdoSrcCfgToken;
    CHARPTR                                                 vdoEncCfgToken;
    ONVIF_MEDIA_SERVICE_INFO_t                              *pMediaService = &(onvifCamInfo[camIndex].mediaServiceInfo);
    UINT8                                                   profileIndex;
    UINT8                                                   profileCnt = 0;
    CHAR                                                    *pProfileTypes= "All";

    user->addr = onvifCamInfo[camIndex].mediaServiceInfo.mediaServiceAddr;
    getProfiles.Token      = NULL;
    getProfiles.__sizeType = 1;
    getProfiles.Type       = &pProfileTypes;
    soapResp = GetProfiles2(soap, &getProfiles, &getProfilesResponse, user);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "fail to get profiles: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
        {
            return ONVIF_CMD_TIMEOUT;
        }
        return ONVIF_CMD_FAIL;
    }

    if (getProfilesResponse.__sizeProfiles <= 0)
    {
        EPRINT(ONVIF_CLIENT, "no profiles found: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    if (getProfilesResponse.Profiles == NULL)
    {
        EPRINT(ONVIF_CLIENT, "profiles not available: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    for (profileIndex = 0; profileIndex < getProfilesResponse.__sizeProfiles; profileIndex++)
    {
        profileToken = getProfilesResponse.Profiles[profileIndex].token;
        if (profileToken == NULL)
        {
            EPRINT(ONVIF_CLIENT, "profile token not available: [camera=%d], [profile=%d]", camIndex, profileIndex);
            continue;
        }

        if (getProfilesResponse.Profiles[profileIndex].Configurations == NULL)
        {
            EPRINT(ONVIF_CLIENT, "profile config not available: [camera=%d], [profile=%d]", camIndex, profileIndex);
            continue;
        }

        //Set the PTZ Profile for PTZ operation
        if ((ptzProfileAvailable == NO) && (ptzSupported == YES))
        {
            if((getProfilesResponse.Profiles[profileIndex].Configurations->PTZ != NULL) &&
                    getProfilesResponse.Profiles[profileIndex].Configurations->PTZ->token != NULL)
            {
                StringNCopy(onvifCamInfo[camIndex].ptzServiceInfo.profileToken,profileToken,MAX_TOKEN_SIZE);
                StringNCopy(onvifCamInfo[camIndex].ptzServiceInfo.ptzConfigurationToken,
                            getProfilesResponse.Profiles[profileIndex].Configurations->PTZ->token,MAX_TOKEN_SIZE);
                ptzProfileAvailable = YES;
            }
        }

        //Now check whether Video Source configuration already available in profile or not
        if (getProfilesResponse.Profiles[profileIndex].Configurations->VideoSource != NULL &&
             getProfilesResponse.Profiles[profileIndex].Configurations->VideoSource->token !=NULL)
        {
            vdoSrcCfgToken = getProfilesResponse.Profiles[profileIndex].Configurations->VideoSource->token;
        }
        else
        {
            EPRINT(ONVIF_CLIENT, "video source config not available: [camera=%d], [profile=%d]", camIndex, profileIndex);
            continue;
        }

        if (getProfilesResponse.Profiles[profileIndex].Configurations->VideoEncoder != NULL &&
             getProfilesResponse.Profiles[profileIndex].Configurations->VideoEncoder->token != NULL)
        {
            vdoEncCfgToken = getProfilesResponse.Profiles[profileIndex].Configurations->VideoEncoder->token;
        }
        else
        {
            EPRINT(ONVIF_CLIENT, "video encoder config not available: [camera=%d], [profile=%d]", camIndex, profileIndex);
            continue;
        }

        getVideoEncoderConfigurationOptions2.ProfileToken = profileToken;
        getVideoEncoderConfigurationOptions2.ConfigurationToken = NULL;
        soapResp = GetVideoEncoderConfigurationOptions2(soap, &getVideoEncoderConfigurationOptions2, &getVideoEncoderConfigurationOptionsResponse2, user);
        if ((soapResp != SOAP_OK) || (getVideoEncoderConfigurationOptionsResponse2.__sizeOptions <= 0) ||
                (getVideoEncoderConfigurationOptionsResponse2.Options == NULL))
        {
            EPRINT(ONVIF_CLIENT, "fail to get video encoder config: [camera=%d], [profile=%d], [soapResp=%d]",
                   camIndex, profileIndex, soapResp);
            continue;
        }

        /* Reset all data of current profile */
        pMediaService->mediaProfile[profileCnt].profileToken[0] = '\0';
        pMediaService->mediaProfile[profileCnt].videoSourceToken[0] = '\0';
        pMediaService->mediaProfile[profileCnt].videoEncoderConfigurationToken[0] = '\0';
        FreeCameraCapabilityProfileWise(camIndex, profileCnt);

        if (FAIL == prepareMedia2CapabilityFromEncoderConfigOptions(camIndex, profileCnt, &getVideoEncoderConfigurationOptionsResponse2))
        {
            EPRINT(ONVIF_CLIENT, "fail to store video encoder config: [camera=%d], [profile=%d], [profileToken=%s], [vdoSrcCfgToken=%s], [vdoEncCfgToken=%s]",
                   camIndex, profileIndex, profileToken, vdoSrcCfgToken, vdoEncCfgToken);
            continue;
        }

        StringNCopy(pMediaService->mediaProfile[profileCnt].profileToken, profileToken, MAX_TOKEN_SIZE);
        StringNCopy(pMediaService->mediaProfile[profileCnt].videoSourceToken, vdoSrcCfgToken, MAX_TOKEN_SIZE);
        StringNCopy(pMediaService->mediaProfile[profileCnt].videoEncoderConfigurationToken, vdoEncCfgToken, MAX_TOKEN_SIZE);

        DPRINT(ONVIF_CLIENT, "video encoder config stored: [camera=%d], profile: [nvr=%d], [cam=%d], [token=%s]",
               camIndex, profileCnt, profileIndex, profileToken);

        profileCnt++;
        status = ONVIF_CMD_SUCCESS;

        /* Profiles must not increase to supported by us */
        if (profileCnt >= MAX_PROFILE_SUPPORT)
        {
            break;
        }
    }
    onvifCamInfo[camIndex].maxSupportedProfile = profileCnt;

    if((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
    {
        status = ONVIF_CMD_TIMEOUT;
    }

    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief addAudioConfiguration : will send onvif commands to required audio information
 * @param soap                  : SOAP instance
 * @param user                  : User information
 * @param camIndex              : Camera Index
 * @return
 */
static ONVIF_RESP_STATUS_e addAudioConfiguration(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 camIndex)
{
    GET_AUDIO_SOURCES_t                         getAudioSources;
    GET_AUDIO_SOURCES_RESPONSE_t                getAudioSourcesResponse;
    GET_AUDIO_SOURCE_CONFIGURATIONS_t           getAudioSourceConfigurations;
    GET_AUDIO_SOURCE_CONFIGURATIONS_RESPONSE_t  getAudioSourceConfigurationsResponse;
    GET_AUDIO_ENCODER_CONFIGURATIONS_t          getAudioEncoderConfigurations;
    GET_AUDIO_ENCODER_CONFIGURATIONS_RESPONSE_t getAudioEncoderConfigurationsResponse;
    INT16                                       soapResp;
    ONVIF_RESP_STATUS_e                         status = ONVIF_CMD_FAIL;

    do
    {
        soapResp = GetAudioSources(soap, &getAudioSources, &getAudioSourcesResponse, user);
        if (soapResp != SOAP_OK)
        {
            EPRINT(ONVIF_CLIENT, "fail to get audio source: [camera=%d], [soapResp=%d]", camIndex, soapResp);
            break;
        }

        if(getAudioSourcesResponse.AudioSources == NULL)
        {
            EPRINT(ONVIF_CLIENT, "audio source not available: [camera=%d]", camIndex);
            break;
        }

        soapResp = GetAudioSourceConfigurations(soap, &getAudioSourceConfigurations, &getAudioSourceConfigurationsResponse, user);
        if (soapResp != SOAP_OK)
        {
            EPRINT(ONVIF_CLIENT, "fail to get audio source config: [camera=%d], [soapResp=%d]", camIndex, soapResp);
            break;
        }

        if ((getAudioSourceConfigurationsResponse.Configurations == NULL) || (getAudioSourceConfigurationsResponse.Configurations->token == NULL))
        {
            EPRINT(ONVIF_CLIENT, "audio source config token not available: [camera=%d]", camIndex);
            break;
        }
        else
        {
            StringNCopy(onvifCamInfo[camIndex].mediaServiceInfo.audioSourceToken,
                        getAudioSourceConfigurationsResponse.Configurations->token, MAX_TOKEN_SIZE);
        }

        soapResp = GetAudioEncoderConfigurations(soap, &getAudioEncoderConfigurations, &getAudioEncoderConfigurationsResponse, user);
        if (soapResp != SOAP_OK)
        {
            EPRINT(ONVIF_CLIENT, "fail to get audio encoder config: [camera=%d], [soapResp=%d]", camIndex, soapResp);
            break;
        }

        if ((getAudioEncoderConfigurationsResponse.Configurations == NULL) || (getAudioEncoderConfigurationsResponse.Configurations->token == NULL))
        {
            EPRINT(ONVIF_CLIENT, "audio encoder config not available: [camera=%d]", camIndex);
            break;
        }
        else
        {
            StringNCopy(onvifCamInfo[camIndex].mediaServiceInfo.audioEncoderConfigurationToken,
                        getAudioEncoderConfigurationsResponse.Configurations->token, MAX_TOKEN_SIZE);
        }

        status = ONVIF_CMD_SUCCESS;
    }
    while(0);

    if((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
    {
        status = ONVIF_CMD_TIMEOUT;
    }

    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief addPtzConfiguration   : Parse PTZ capabilities and add them to NVR
 * @param soap                  : SOAP instance
 * @param user                  : User information
 * @param camIndex              : camera index
 * @return
 */
static ONVIF_RESP_STATUS_e addPtzConfiguration(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 camIndex)
{
    GET_CONFIGURATIONS_t 		  			getConfigurations;
    GET_CONFIGURATIONS_RESPONSE_t			getConfigurationsResponse;
    GET_CONFIGURATION_OPTIONS_t 	     	getConfigurationOptions;
    GET_CONFIGURATION_OPTIONS_RESPONSE_t 	getConfigurationOptionsResponse;
    GET_NODES_t                             getNodes;
    GET_NODES_RESPONSE_t                    getNodesResponse;
    ADD_PTZ_CONFIG_t                        addPtzConfig;
    ADD_PTZ_CONFIG_RESP_t                   addPtzConfigResp;
    INT16                                   soapResp;
    ONVIF_RESP_STATUS_e                     status = ONVIF_CMD_FAIL;

    /* Validate camera index */
    if (camIndex >= getMaxCameraForCurrentVariant())
    {
        EPRINT(ONVIF_CLIENT, "invld camera index found: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    do
    {
        /* Added to pass ONVIF Conformance test */
        soapResp = GetNodes(soap, &getNodes, &getNodesResponse, user);
        if((soapResp != SOAP_OK) || (getNodesResponse.PTZNode == NULL) || (getNodesResponse.__sizePTZNode == 0))
        {
             EPRINT(ONVIF_CLIENT, "fail to get ptz nodes: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        }

        soapResp = GetConfigurations(soap, &getConfigurations, &getConfigurationsResponse, user);
        if ((soapResp != SOAP_OK) || (getConfigurationsResponse.PTZConfiguration == NULL)
                || (getConfigurationsResponse.PTZConfiguration->token == NULL))
        {
            EPRINT(ONVIF_CLIENT, "fail to get ptz config: [camera=%d], [soapResp=%d]", camIndex, soapResp);
            break;
        }

        StringNCopy(onvifCamInfo[camIndex].ptzServiceInfo.ptzConfigurationToken,
                    getConfigurationsResponse.PTZConfiguration->token, MAX_TOKEN_SIZE);

        /* Added to pass ONVIF Conformance test */
        addPtzConfig.ConfigurationToken = onvifCamInfo[camIndex].ptzServiceInfo.ptzConfigurationToken;
        addPtzConfig.ProfileToken       = onvifCamInfo[camIndex].ptzServiceInfo.profileToken;
        soapResp = AddPtzConfiguration(soap, &addPtzConfig, &addPtzConfigResp, user);
        if(soapResp != SOAP_OK)
        {
             EPRINT(ONVIF_CLIENT, "fail to add ptz config: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        }

        getConfigurationOptions.ConfigurationToken = onvifCamInfo[camIndex].ptzServiceInfo.ptzConfigurationToken;
        soapResp = GetConfigurationOptions(soap, &getConfigurationOptions, &getConfigurationOptionsResponse, user);
        if ((soapResp != SOAP_OK) || (getConfigurationOptionsResponse.PTZConfigurationOptions == NULL))
        {
            EPRINT(ONVIF_CLIENT, "fail to get ptz config options: [camera=%d], [soapResp=%d]", camIndex, soapResp);
            break;
        }

        if (getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces == NULL)
        {
            EPRINT(ONVIF_CLIENT, "ptz config not available: [camera=%d]", camIndex);
            break;
        }

        if (getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->PanTiltSpeedSpace != NULL)
        {
            onvifCamInfo[camIndex].ptzServiceInfo.pantiltSpeed =
                    ((getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->PanTiltSpeedSpace->XRange->Max -
                      getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->PanTiltSpeedSpace->XRange->Min) / MAX_PTZ_SPEED_STEPS);
        }
        else
        {
            onvifCamInfo[camIndex].ptzServiceInfo.pantiltSpeed = 0;
        }

        if (getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->ZoomSpeedSpace != NULL)
        {
            onvifCamInfo[camIndex].ptzServiceInfo.zoomSpeed =
                    ((getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->ZoomSpeedSpace->XRange->Max -
                       getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->ZoomSpeedSpace->XRange->Min) / MAX_PTZ_SPEED_STEPS);
        }
        else
        {
            onvifCamInfo[camIndex].ptzServiceInfo.zoomSpeed = 0;
        }

        onvifCamInfo[camIndex].ptzServiceInfo.panTiltSupport = NO_FUNCTION_SUPPORT;
        onvifCamInfo[camIndex].ptzServiceInfo.zoomSupport = NO_FUNCTION_SUPPORT;

        //pan-tilt
        do
        {
            if (getConfigurationsResponse.PTZConfiguration->DefaultContinuousPanTiltVelocitySpace != NULL)
            {
                if (getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->ContinuousPanTiltVelocitySpace != NULL)
                {
                    if ((getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->ContinuousPanTiltVelocitySpace->XRange != NULL)
                            && (getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->ContinuousPanTiltVelocitySpace->YRange != NULL))
                    {
                        onvifCamInfo[camIndex].ptzServiceInfo.panTiltSupport = CONTINUOUS_MOVE_SUPPORT;

                        /* As per generic onvif Pan/Tilt velocity space PTZ camera will provide Min value -1 & Max value 1 considoring movement direction,
                         * since the range includes both direction of movement(denoted by sign), we need to divide the range into (MAX_PTZ_SPEED_STEPS * 2) parts */
                        onvifCamInfo[camIndex].ptzServiceInfo.panStep =
                                ((getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->ContinuousPanTiltVelocitySpace->XRange->Max -
                                   getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->ContinuousPanTiltVelocitySpace->XRange->Min)
                                  / (MAX_PTZ_SPEED_STEPS * 2));
                        onvifCamInfo[camIndex].ptzServiceInfo.tiltStep =
                                ((getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->ContinuousPanTiltVelocitySpace->YRange->Max -
                                   getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->ContinuousPanTiltVelocitySpace->YRange->Min)
                                  / (MAX_PTZ_SPEED_STEPS * 2));
                        break;
                    }
                }
            }

            if (getConfigurationsResponse.PTZConfiguration->DefaultRelativePanTiltTranslationSpace != NULL)
            {
                if (getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->RelativePanTiltTranslationSpace != NULL)
                {
                    if ((getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->RelativePanTiltTranslationSpace->XRange != NULL)
                            && (getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->RelativePanTiltTranslationSpace->YRange != NULL))
                    {
                        onvifCamInfo[camIndex].ptzServiceInfo.panTiltSupport = RELATIVE_MOVE_SUPPORT;
                        onvifCamInfo[camIndex].ptzServiceInfo.panStep =
                                ((getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->RelativePanTiltTranslationSpace->XRange->Max -
                                   getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->RelativePanTiltTranslationSpace->XRange->Min)
                                  / DEFAULT_PTZ_STEP);
                        onvifCamInfo[camIndex].ptzServiceInfo.tiltStep =
                                ((getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->RelativePanTiltTranslationSpace->YRange->Max -
                                   getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->RelativePanTiltTranslationSpace->YRange->Min)
                                  / DEFAULT_PTZ_STEP);
                        break;
                    }
                }
            }

            if (getConfigurationsResponse.PTZConfiguration->DefaultAbsolutePantTiltPositionSpace != NULL)
            {
                if (getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->AbsolutePanTiltPositionSpace != NULL)
                {
                    if ((getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->AbsolutePanTiltPositionSpace->XRange != NULL)
                            && (getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->AbsolutePanTiltPositionSpace->YRange != NULL))
                    {
                        onvifCamInfo[camIndex].ptzServiceInfo.panTiltSupport = ABSOLUTE_MOVE_SUPPORT;
                        onvifCamInfo[camIndex].ptzServiceInfo.panStep =
                                ((getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->AbsolutePanTiltPositionSpace->XRange->Max -
                                   getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->AbsolutePanTiltPositionSpace->XRange->Min)
                                  / DEFAULT_PTZ_STEP);
                        onvifCamInfo[camIndex].ptzServiceInfo.tiltStep =
                                ((getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->AbsolutePanTiltPositionSpace->YRange->Max -
                                   getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->AbsolutePanTiltPositionSpace->YRange->Min)
                                  / DEFAULT_PTZ_STEP);
                        break;
                    }
                }
            }
        }
        while(0);

        //zoom
        do
        {
            if (getConfigurationsResponse.PTZConfiguration->DefaultContinuousZoomVelocitySpace != NULL)
            {
                if (getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->ContinuousZoomVelocitySpace != NULL)
                {
                    if (getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->ContinuousZoomVelocitySpace->XRange != NULL)
                    {
                        onvifCamInfo[camIndex].ptzServiceInfo.zoomSupport = CONTINUOUS_MOVE_SUPPORT;

                        /* As per generic onvif zoom velocity space PTZ camera will provide Min value -1 & Max value 1 considoring movement direction,
                         * since the range includes both direction of movement(denoted by sign), we need to divide the range into (MAX_PTZ_SPEED_STEPS * 2) parts */
                        onvifCamInfo[camIndex].ptzServiceInfo.zoomStep =
                                ((getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->ContinuousZoomVelocitySpace->XRange->Max -
                                   getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->ContinuousZoomVelocitySpace->XRange->Min)
                                  / (MAX_PTZ_SPEED_STEPS * 2));
                        break;
                    }
                }
            }

            if (getConfigurationsResponse.PTZConfiguration->DefaultRelativeZoomTranslationSpace != NULL)
            {
                if (getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->RelativeZoomTranslationSpace != NULL)
                {
                    if (getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->RelativeZoomTranslationSpace->XRange != NULL)
                    {
                        onvifCamInfo[camIndex].ptzServiceInfo.zoomSupport = RELATIVE_MOVE_SUPPORT;
                        onvifCamInfo[camIndex].ptzServiceInfo.zoomStep =
                                ((getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->RelativeZoomTranslationSpace->XRange->Max -
                                   getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->RelativeZoomTranslationSpace->XRange->Min)
                                  / DEFAULT_PTZ_STEP);
                        break;
                    }
                }
            }

            if (getConfigurationsResponse.PTZConfiguration->DefaultAbsoluteZoomPositionSpace != NULL)
            {
                if (getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->AbsoluteZoomPositionSpace != NULL)
                {
                    if (getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->AbsoluteZoomPositionSpace->XRange != NULL)
                    {
                        onvifCamInfo[camIndex].ptzServiceInfo.zoomSupport = ABSOLUTE_MOVE_SUPPORT;
                        onvifCamInfo[camIndex].ptzServiceInfo.zoomStep =
                                ((getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->AbsoluteZoomPositionSpace->XRange->Max -
                                   getConfigurationOptionsResponse.PTZConfigurationOptions->Spaces->AbsoluteZoomPositionSpace->XRange->Min)
                                  / DEFAULT_PTZ_STEP);
                        break;
                    }
                }
            }
        }
        while(0);

        status = ONVIF_CMD_SUCCESS;
    }
    while(0);

    if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
    {
        status = ONVIF_CMD_TIMEOUT;
    }

    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief addImagingConfiguration   : add camera imaging configuration
 * @param soap                      : SOAP instance
 * @param user                      : user information
 * @param camIndex                  : Camera Index
 * @return
 */
static ONVIF_RESP_STATUS_e addImagingConfiguration(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 camIndex)
{
    GET_MOVE_OPTIONS_t 							getMoveOptions;
    GET_MOVE_OPTIONS_RESPONSE_t 				getMoveOptionsResponse;
    GET_VIDEO_SOURCE_CONFIGURATIONS_t 		   	getVideoSourceConfigurations;
    GET_VIDEO_SOURCE_CONFIGURATIONS_RESPONSE_t 	getVideoSourceConfigurationsResponse;
    GET_OPTIONS_t 		   						getOptions;
    GET_OPTIONS_RESPONSE_t 						getOptionsResponse;
    INT16 										soapResp;
    ONVIF_RESP_STATUS_e 						status = ONVIF_CMD_FAIL;
    BOOL 										focusSupport = NO;
    BOOL 										irisSupport = NO;

    onvifCamInfo[camIndex].imagingServiceInfo.focusInfo.focusSupport = NO_FUNCTION_SUPPORT;
    onvifCamInfo[camIndex].imagingServiceInfo.irisInfo.irisSupport = NO_FUNCTION_SUPPORT;
    user->addr = onvifCamInfo[camIndex].mediaServiceInfo.mediaServiceAddr;

    //--------------------------------------------------------------------------------
    // Get Video source configuration
    //--------------------------------------------------------------------------------
    soapResp = GetVideoSourceConfigurations(soap, &getVideoSourceConfigurations, &getVideoSourceConfigurationsResponse, user);
    if ((soapResp != SOAP_OK) || (getVideoSourceConfigurationsResponse.Configurations == NULL)
            || (getVideoSourceConfigurationsResponse.Configurations->SourceToken == NULL))
    {
        /* onvif command fail */
        if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
        {
            status = ONVIF_CMD_TIMEOUT;
        }
        return status;
    }

    //--------------------------------------------------------------------------------
    // Get Move Option (FOCUS)
    //--------------------------------------------------------------------------------
    do
    {
        user->addr = onvifCamInfo[camIndex].imagingServiceInfo.imagingServiceAddr;
        StringNCopy(onvifCamInfo[camIndex].imagingServiceInfo.videoSourceToken, getVideoSourceConfigurationsResponse.Configurations->SourceToken, MAX_TOKEN_SIZE);
        getMoveOptions.VideoSourceToken = onvifCamInfo[camIndex].imagingServiceInfo.videoSourceToken;
        soapResp = GetMoveOptions(soap, &getMoveOptions, &getMoveOptionsResponse, user);
        if ((soapResp != SOAP_OK) || (getMoveOptionsResponse.MoveOptions == NULL))
        {
            EPRINT(ONVIF_CLIENT, "fail to get ptz move options: [camera=%d], [soapResp=%d]", camIndex, soapResp);
            break;
        }

        /* Move priority will be: 1. Continuous, 2. Relative, 3. Absolute: Make it same as zoom, pan and tilt */
        onvifCamInfo[camIndex].imagingServiceInfo.focusInfo.focusSpeed = 0;
        if(getMoveOptionsResponse.MoveOptions->Continuous != NULL)
        {
            onvifCamInfo[camIndex].imagingServiceInfo.focusInfo.focusSupport = CONTINUOUS_MOVE_SUPPORT;
            if(getMoveOptionsResponse.MoveOptions->Continuous->Speed != NULL)
            {
                onvifCamInfo[camIndex].imagingServiceInfo.focusInfo.focusStep =
                        ((getMoveOptionsResponse.MoveOptions->Continuous->Speed->Max - getMoveOptionsResponse.MoveOptions->Continuous->Speed->Min) / (MAX_PTZ_SPEED_STEPS * 2));
                focusSupport = YES;
                break;
            }
        }

        if(getMoveOptionsResponse.MoveOptions->Relative != NULL)
        {
            onvifCamInfo[camIndex].imagingServiceInfo.focusInfo.focusSupport = RELATIVE_MOVE_SUPPORT;
            if(getMoveOptionsResponse.MoveOptions->Relative->Distance != NULL)
            {
                onvifCamInfo[camIndex].imagingServiceInfo.focusInfo.focusStep =
                        ((getMoveOptionsResponse.MoveOptions->Relative->Distance->Max - getMoveOptionsResponse.MoveOptions->Relative->Distance->Min) / FOCUS_STEP);
                if(getMoveOptionsResponse.MoveOptions->Relative->Speed != NULL)
                {
                    onvifCamInfo[camIndex].imagingServiceInfo.focusInfo.focusSpeed =
                            ((getMoveOptionsResponse.MoveOptions->Relative->Speed->Max - getMoveOptionsResponse.MoveOptions->Relative->Speed->Min) / FOCUS_SPEED_STEP);
                }
                focusSupport = YES;
                break;
            }
        }

        if(getMoveOptionsResponse.MoveOptions->Absolute != NULL)
        {
            onvifCamInfo[camIndex].imagingServiceInfo.focusInfo.focusSupport = ABSOLUTE_MOVE_SUPPORT;
            if(getMoveOptionsResponse.MoveOptions->Absolute->Position != NULL)
            {
                onvifCamInfo[camIndex].imagingServiceInfo.focusInfo.focusStep =
                        ((getMoveOptionsResponse.MoveOptions->Absolute->Position->Max - getMoveOptionsResponse.MoveOptions->Absolute->Position->Min) / FOCUS_STEP);
                if(getMoveOptionsResponse.MoveOptions->Absolute->Speed != NULL)
                {
                    onvifCamInfo[camIndex].imagingServiceInfo.focusInfo.focusSpeed =
                            ((getMoveOptionsResponse.MoveOptions->Absolute->Speed->Max - getMoveOptionsResponse.MoveOptions->Absolute->Speed->Min) / FOCUS_SPEED_STEP);
                }
                focusSupport = YES;
                break;
            }
        }
    }
    while(0);

    //--------------------------------------------------------------------------------
    // Get Options (Imaging Options)
    //--------------------------------------------------------------------------------
    user->addr = onvifCamInfo[camIndex].imagingServiceInfo.imagingServiceAddr;
    getOptions.VideoSourceToken = onvifCamInfo[camIndex].imagingServiceInfo.videoSourceToken;
    soapResp = GetOptions(soap, &getOptions, &getOptionsResponse, user);
    if((soapResp == SOAP_OK) && (getOptionsResponse.ImagingOptions != NULL))
    {
        //--------------------------------------------------------------------------------
        // Get Iris Property
        //--------------------------------------------------------------------------------
        if((getOptionsResponse.ImagingOptions->Exposure != NULL) && (getOptionsResponse.ImagingOptions->Exposure->Iris != NULL))
        {
            onvifCamInfo[camIndex].imagingServiceInfo.irisInfo.irisStep =
                    ((getOptionsResponse.ImagingOptions->Exposure->Iris->Max - getOptionsResponse.ImagingOptions->Exposure->Iris->Min) / DEFAULT_IRIS_STEP);
            if(onvifCamInfo[camIndex].imagingServiceInfo.irisInfo.irisStep != 0)
            {
                onvifCamInfo[camIndex].imagingServiceInfo.irisInfo.irisSupport = ABSOLUTE_MOVE_SUPPORT;
                irisSupport = YES;
            }
        }

        /* Camera supports imaging params */
        onvifCamInfo[camIndex].imagingServiceInfo.isImgSettingSupport = YES;
    }

    /* Generate response code */
    if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
    {
        status = ONVIF_CMD_TIMEOUT;
    }
    else if ((focusSupport == YES) || (irisSupport == YES))
    {
        status = ONVIF_CMD_SUCCESS;
    }

    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief setVideoProfile   : Will set media Profile configuration to camera using Media 1 API
 * @param soap              : SOAP Instance
 * @param onvifEncCfg       : Stream profile configuration to be set
 * @param user              : User information
 * @param camIndex          : Camera Index
 * @param profileNum        : profile Number
 */
static void setVideoProfile(SOAP_t *soap, ONVIF_ENC_CFG_t *onvifEncCfg, SOAP_USER_DETAIL_t *user, UINT8 camIndex, UINT8 profileIndex)
{
    ADD_VIDEO_SOURCE_CONFIGURATION_t 		   	addVideoSourceConfiguration;
    ADD_VIDEO_SOURCE_CONFIGURATION_RESPONSE_t 	addVideoSourceConfigurationResponse;
    VIDEO_ENCODER_CONFIGURATION_t				*videoEncoderCfg = NULL;
    ADD_VIDEO_ENCODER_CONFIGURATION_t		   	addVideoEncoderConfiguration;
    ADD_VIDEO_ENCODER_CONFIGURATION_RESPONSE_t 	addVideoEncoderConfigurationResponse;
    GET_VIDEO_ENCODER_CONFIGURATION_t 		   	getVideoEncoderConfiguration;
    GET_VIDEO_ENCODER_CONFIGURATION_RESPONSE_t 	getVideoEncoderConfigurationResponse;
    SET_VIDEO_ENCODER_CONFIGURATION_t		   	setVideoEncoderConfiguration;
    SET_VIDEO_ENCODER_CONFIGURATION_RESPONSE_t 	setVideoEncoderConfigurationResponse;
    GET_PROFILE_t								getProfile;
    GET_PROFILE_RESPONSE_t						getProfileResponse;
    VIDEO_RATE_CONTROL_t  						rateControl;
    VIDEO_RESOLUTION_t	  						resolution;
    MPEG4_CONFIGURATION_t 						mpeg4;
    H264_CONFIGURATION_t  						h264;
    BOOL 										setVdoEncCfg = YES;
    BOOL 										addVdoSrcCfg = YES;
    BOOL 										addVdoEncCfg = YES;
    INT16   									soapResp;
    ONVIF_MEDIA_PROFILE_INFO_t					*pMediaProfile;

    pMediaProfile = &(onvifCamInfo[camIndex].mediaServiceInfo.mediaProfile[profileIndex]);
    getProfile.ProfileToken = pMediaProfile->profileToken;
    soapResp = GetProfile(soap, &getProfile, &getProfileResponse, user);
    if (soapResp == SOAP_OK)
    {
        if (getProfileResponse.Profile == NULL)
        {
            EPRINT(ONVIF_CLIENT, "null profile found: [camera=%d]", camIndex);
        }
        else
        {
            //If no video source configuration available then add video source configuration
            if(getProfileResponse.Profile->VideoSourceConfiguration != NULL)
            {
                addVdoSrcCfg = NO;
            }

            //If no video source configuration available then add video encoder configuration
            if(getProfileResponse.Profile->VideoEncoderConfiguration != NULL)
            {
                videoEncoderCfg = getProfileResponse.Profile->VideoEncoderConfiguration;
                addVdoEncCfg = NO;
            }
        }
    }
    else
    {
        EPRINT(ONVIF_CLIENT, "fail to get profile: [camera=%d], [soapResp=%d]", camIndex, soapResp);
    }

    if(addVdoSrcCfg == YES)
    {
        addVideoSourceConfiguration.ProfileToken = pMediaProfile->profileToken;
        addVideoSourceConfiguration.ConfigurationToken = pMediaProfile->videoSourceToken;
        soapResp = AddVideoSourceConfiguration(soap, &addVideoSourceConfiguration, &addVideoSourceConfigurationResponse, user);
        if (soapResp != SOAP_OK)
        {
            EPRINT(ONVIF_CLIENT, "fail to add video source config: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        }
    }

    if(addVdoEncCfg == YES)
    {
        getVideoEncoderConfiguration.ConfigurationToken = pMediaProfile->videoEncoderConfigurationToken;
        soapResp = GetVideoEncoderConfiguration(soap, &getVideoEncoderConfiguration, &getVideoEncoderConfigurationResponse, user);
        if ((soapResp != SOAP_OK) || (getVideoEncoderConfigurationResponse.Configuration == NULL))
        {
            EPRINT(ONVIF_CLIENT, "fail to get video encoder config: [camera=%d], [soapResp=%d]", camIndex, soapResp);
            setVdoEncCfg = NO;
        }
        else
        {
            videoEncoderCfg = getVideoEncoderConfigurationResponse.Configuration;
        }
    }

    //If configuration is available then set video encoder configuration
    if(setVdoEncCfg == YES)
    {
        setVideoEncoderConfiguration.Configuration = videoEncoderCfg;
        setVideoEncoderConfiguration.Configuration->token = pMediaProfile->videoEncoderConfigurationToken;

        if(onvifEncCfg->fps != 0) //Not an invalid fps
        {
            if(setVideoEncoderConfiguration.Configuration->RateControl == NULL)
            {
                setVideoEncoderConfiguration.Configuration->RateControl = &rateControl;
                setVideoEncoderConfiguration.Configuration->RateControl->BitrateLimit = GetDefaultBitRateMax(camIndex, profileIndex, onvifEncCfg->codec);
                setVideoEncoderConfiguration.Configuration->RateControl->EncodingInterval = pMediaProfile->defaultEncodingInterval[GET_VALID_CODEC_NUM(onvifEncCfg->codec)];
            }

            setVideoEncoderConfiguration.Configuration->RateControl->BitrateLimit = onvifBitrateValue[onvifEncCfg->bitrateValue];
            setVideoEncoderConfiguration.Configuration->RateControl->FrameRateLimit = onvifEncCfg->fps;
        }

        if (onvifEncCfg->quality != 0)
        {
            if(onvifCamInfo[camIndex].mediaServiceInfo.initialQuality == 1)
            {
                setVideoEncoderConfiguration.Configuration->Quality = onvifEncCfg->quality;
            }
            else
            {
                setVideoEncoderConfiguration.Configuration->Quality = (onvifEncCfg->quality + onvifCamInfo[camIndex].mediaServiceInfo.initialQuality - 1);
            }
        }

        if((onvifEncCfg->resolHeight != 0) && (onvifEncCfg->resolWidth != 0))
        {
            if(setVideoEncoderConfiguration.Configuration->Resolution == NULL)
            {
                setVideoEncoderConfiguration.Configuration->Resolution = &resolution;
            }

            setVideoEncoderConfiguration.Configuration->Resolution->Height = onvifEncCfg->resolHeight;
            setVideoEncoderConfiguration.Configuration->Resolution->Width = onvifEncCfg->resolWidth;
        }

        switch (onvifEncCfg->codec)
        {
            default:
                break;

            case VIDEO_MJPG:
                setVideoEncoderConfiguration.Configuration->Encoding = VIDEO_ENCODING_JPEG;
                setVideoEncoderConfiguration.Configuration->MPEG4	  = NULL;
                setVideoEncoderConfiguration.Configuration->H264 	  = NULL;
                break;

            case VIDEO_MPEG4:
                setVideoEncoderConfiguration.Configuration->Encoding = VIDEO_ENCODING_MPEG4;
                if(setVideoEncoderConfiguration.Configuration->MPEG4 == NULL)
                {
                    setVideoEncoderConfiguration.Configuration->MPEG4 = &mpeg4;
                }
                setVideoEncoderConfiguration.Configuration->MPEG4->GovLength = onvifEncCfg->gop;
                setVideoEncoderConfiguration.Configuration->MPEG4->Mpeg4Profile = onvifCamInfo[camIndex].mediaServiceInfo.mpeg4SupportedProfile;
                setVideoEncoderConfiguration.Configuration->H264 = NULL;
                break;

            case VIDEO_H264:
                setVideoEncoderConfiguration.Configuration->Encoding = VIDEO_ENCODING_H264;
                if(setVideoEncoderConfiguration.Configuration->H264 == NULL)
                {
                    setVideoEncoderConfiguration.Configuration->H264 = &h264;
                }
                setVideoEncoderConfiguration.Configuration->H264->GovLength = onvifEncCfg->gop;
                setVideoEncoderConfiguration.Configuration->H264->H264Profile = onvifCamInfo[camIndex].mediaServiceInfo.h264SupportedProfile;
                setVideoEncoderConfiguration.Configuration->MPEG4 = NULL;
                break;
        }

        setVideoEncoderConfiguration.ForcePersistence = XSD_BOOL_FALSE;
        soapResp = SetVideoEncoderConfiguration(soap, &setVideoEncoderConfiguration, &setVideoEncoderConfigurationResponse, user);
        if (soapResp != SOAP_OK)
        {
            EPRINT(ONVIF_CLIENT, "fail to set video encoder config: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        }
    }

    if (addVdoEncCfg == YES)
    {
        addVideoEncoderConfiguration.ProfileToken = pMediaProfile->profileToken;
        addVideoEncoderConfiguration.ConfigurationToken = pMediaProfile->videoEncoderConfigurationToken;
        soapResp = AddVideoEncoderConfiguration(soap, &addVideoEncoderConfiguration, &addVideoEncoderConfigurationResponse, user);
        if (soapResp != SOAP_OK)
        {
            EPRINT(ONVIF_CLIENT, "fail to add video encoder config: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief setMedia2VideoProfile : will set all profile param to Camera for given profile
 * @param soap                  : soap instance
 * @param onvifEncCfg           : contains new param to be set
 * @param user                  : Required to communicate with camera
 * @param camIndex              : Camera Index
 * @param profileNum            : Camera Profile Number
 */
static void setMedia2VideoProfile(SOAP_t *soap, ONVIF_ENC_CFG_t *onvifEncCfg, SOAP_USER_DETAIL_t *user, UINT8 camIndex, UINT8 profileIndex)
{
    VIDEO_ENCODER2_CONFIGURATION_t              videoEncoder;
    SET_VIDEO_ENCODER_CONFIGURATION2_t          setVideoEncoderConfiguration;
    SET_VIDEO_ENCODER_CONFIGURATION_RESPONSE2_t setVideoEncoderConfigurationResponse;
    GET_PROFILES2_t                             getProfiles;
    GET_PROFILES_RESPONSE2_t                    getProfilesResponse;
    VIDEO_RATE_CONTROL2_t                       rateControl;
    VIDEO_RESOLUTION2_t                         resolution;
    INT16                                       soapResp;
    ONVIF_MEDIA_PROFILE_INFO_t                  *pMediaProfile;
    CHAR                                        *pProfileType = "VideoEncoder";
    CHAR                                        profileSupported[CODEC_PROFILE_LEN] = "";
    INT32                                       gop = onvifEncCfg->gop;
    CHAR                                        encoding[CODEC_PROFILE_LEN] ="";
    enum xsd__boolean                           constantBitRate;

    //pointer to profile data structure
    pMediaProfile = &(onvifCamInfo[camIndex].mediaServiceInfo.mediaProfile[profileIndex]);
    getProfiles.Token = pMediaProfile->profileToken;
    getProfiles.__sizeType = 1;
    getProfiles.Type= &pProfileType; //We only require Video encoder configuration
    soapResp = GetProfiles2(soap, &getProfiles, &getProfilesResponse, user);
    if ((soapResp != SOAP_OK) || (getProfilesResponse.__sizeProfiles < 1) || (getProfilesResponse.Profiles == NULL))
    {
        EPRINT(ONVIF_CLIENT, "fail to get profiles: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        return;
    }

    //No configuration received
    if(getProfilesResponse.Profiles[0].Configurations == NULL)
    {
        EPRINT(ONVIF_CLIENT, "profile config not available: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        return;
    }

    /* Set video encoder configuration */
    if (getProfilesResponse.Profiles[0].Configurations->VideoEncoder != NULL)
    {
        setVideoEncoderConfiguration.Configuration = getProfilesResponse.Profiles[0].Configurations->VideoEncoder;
    }
    else
    {
        memset(&videoEncoder, '\0', sizeof(VIDEO_ENCODER2_CONFIGURATION_t));
        setVideoEncoderConfiguration.Configuration = &videoEncoder;
    }

    setVideoEncoderConfiguration.Configuration->token = pMediaProfile->videoEncoderConfigurationToken;
    if (onvifEncCfg->fps != 0) //Not an invalid fps
    {
        if(setVideoEncoderConfiguration.Configuration->RateControl == NULL)
        {
            setVideoEncoderConfiguration.Configuration->RateControl = &rateControl;
            setVideoEncoderConfiguration.Configuration->RateControl->BitrateLimit = GetDefaultBitRateMax(camIndex, profileIndex, onvifEncCfg->codec);
        }

        setVideoEncoderConfiguration.Configuration->RateControl->BitrateLimit = onvifBitrateValue[onvifEncCfg->bitrateValue];
        setVideoEncoderConfiguration.Configuration->RateControl->FrameRateLimit = (float)onvifEncCfg->fps;

        constantBitRate = (onvifEncCfg->bitrateMode == CBR) ? XSD_BOOL_TRUE : XSD_BOOL_FALSE;
        setVideoEncoderConfiguration.Configuration->RateControl->ConstantBitRate = &constantBitRate;
    }

    if (onvifEncCfg->quality != 0)
    {
        //Min quality should be zero not 1
        if(onvifCamInfo[camIndex].mediaServiceInfo.initialQuality == 1)
        {
            setVideoEncoderConfiguration.Configuration->Quality = onvifEncCfg->quality;
        }
        else
        {
            setVideoEncoderConfiguration.Configuration->Quality = (onvifEncCfg->quality + onvifCamInfo[camIndex].mediaServiceInfo.initialQuality - 1);
        }
    }

    if((onvifEncCfg->resolHeight != 0) && (onvifEncCfg->resolWidth != 0))
    {
        if(setVideoEncoderConfiguration.Configuration->Resolution == NULL)
        {
            setVideoEncoderConfiguration.Configuration->Resolution = &resolution;
        }

        setVideoEncoderConfiguration.Configuration->Resolution->Height = onvifEncCfg->resolHeight;
        setVideoEncoderConfiguration.Configuration->Resolution->Width = onvifEncCfg->resolWidth;
        setVideoEncoderConfiguration.Configuration->Resolution->__size = 1;
        setVideoEncoderConfiguration.Configuration->Resolution->__any = NULL;
    }

    if (onvifEncCfg->codec < MAX_VIDEO_CODEC)
    {
        if(setVideoEncoderConfiguration.Configuration->Encoding == NULL)
        {
            setVideoEncoderConfiguration.Configuration->Encoding = encoding;
        }

        snprintf(setVideoEncoderConfiguration.Configuration->Encoding, CODEC_PROFILE_LEN, "%s", VEncoderName[onvifEncCfg->codec]);
        GetSupportedProfileForCodec(camIndex,profileIndex,onvifEncCfg->codec,profileSupported);
        setVideoEncoderConfiguration.Configuration->Profile = profileSupported;
    }
    else
    {
        EPRINT(ONVIF_CLIENT, "valid codec not found: [camera=%d], [codec=%d]", camIndex, onvifEncCfg->codec);
    }

    setVideoEncoderConfiguration.Configuration->GovLength = &gop;
    soapResp = SetVideoEncoderConfiguration2(soap, &setVideoEncoderConfiguration, &setVideoEncoderConfigurationResponse, user);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "fail to set video encoder config: [camera=%d], [soapResp=%d]", camIndex, soapResp);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief getMediaUrl       : Will get RTSP stream URL from Camera
 * @param soap              : soap instance
 * @param user              : Required authentication info
 * @param streamType        : Main or sub
 * @param strmConfig        : stream configuration
 * @param urlReq            : url result
 * @param camIndex          : camera index
 * @param forceConfigOnCamera : Will set video stream config to camera if flag is true
 * @param profileNum        : stream profile index
 * @return                  : onvif response
 */
static ONVIF_RESP_STATUS_e getMediaUrl(SOAP_t *soap, SOAP_USER_DETAIL_t *user, VIDEO_TYPE_e streamType, STREAM_CONFIG_t *strmConfig,
                                       URL_REQUEST_t *urlReq, UINT8 camIndex, BOOL forceConfigOnCamera, UINT8 profileNum)
{
    ADD_AUDIO_ENCODER_CONFIGURATION_t		 		addAudioEncoderConfiguration;
    ADD_AUDIO_ENCODER_CONFIGURATION_RESPONSE_t		addAudioEncoderConfigurationResponse;
    ADD_AUDIO_SOURCE_CONFIGURATION_t		 		addAudioSourceConfiguration;
    ADD_AUDIO_SOURCE_CONFIGURATION_RESPONSE_t		addAudioSourceConfigurationResponse;
    REMOVE_AUDIO_ENCODER_CONFIGURATION_t		  	removeAudioEncoderConfiguration;
    REMOVE_AUDIO_ENCODER_CONFIGURATION_RESPONSE_t 	removeAudioEncoderConfigurationResponse;
    REMOVE_AUDIO_SOURCE_CONFIGURATION_t 		 	removeAudioSourceConfiguration;
    REMOVE_AUDIO_SOURCE_CONFIGURATION_RESPONSE_t 	removeAudioSourceConfigurationResponse;
    GET_STREAM_URI_t 		  						getStreamUri;
    GET_STREAM_URI_RESPONSE_t 						getStreamUriResponse;
    ONVIF_RESP_STATUS_e   							status = ONVIF_CMD_FAIL;
    INT16   										soapResp = SOAP_CLI_FAULT;
    STREAM_SETUP_t	  								streamSetup;
    MEDIA_TRANSPORT_t 								transport;
    ONVIF_ENC_CFG_t									onvifEncCfg;
    ONVIF_MEDIA_PROFILE_INFO_t						*pMediaProfile;
    IP_CAMERA_CONFIG_t								ipCamCfg;
    TRANSPORT_TYPE_e								rtspTransportType;
    UINT8											profileIndex;

    /* for other modules profile range is [1:MAX], here it is [0:MAX] */
    if ((profileNum == 0) || (profileNum > MAX_PROFILE_SUPPORT))
    {
        EPRINT(ONVIF_CLIENT, "invld profile number: [camera=%d], [profile=%d]", camIndex, profileNum);
        return ONVIF_CMD_FAIL;
    }

    /* index starting from 0 */
    profileIndex = profileNum - 1;

    /* Profile token must be present */
    pMediaProfile = &(onvifCamInfo[camIndex].mediaServiceInfo.mediaProfile[profileIndex]);
    if (pMediaProfile->profileToken[0] == '\0')
    {
        EPRINT(ONVIF_CLIENT, "profile token not available: [camera=%d], [profile=%d]", camIndex, profileIndex);
        return ONVIF_CMD_FAIL;
    }

    /* Get the time difference between NVR and Camera */
    getCameraDateTime(soap, camIndex, user);
    user->addr = onvifCamInfo[camIndex].mediaServiceInfo.mediaServiceAddr;
    user->timeDiff = &onvifCamInfo[camIndex].timeDifference;

    do
    {
        /* If TRUE then first given profile param will be set to camera stream profile otherwise no need to set profile
         * in camera, only get Media RTSP url from Camera */
        if (forceConfigOnCamera == TRUE)
        {
            DPRINT(ONVIF_CLIENT, "set nvr profile config in camera: [camera=%d], [profile=%d]", camIndex, profileIndex);
            if (streamType == MAIN_STREAM)
            {
                if (GetVideoCodecNum(strmConfig->videoEncoding, &onvifEncCfg.codec) == FAIL)
                {
                    EPRINT(ONVIF_CLIENT, "invld video codec for main stream: [camera=%d], [profile=%d]", camIndex, profileIndex);
                    break;
                }

                onvifEncCfg.fps = strmConfig->framerate;
                onvifEncCfg.quality = strmConfig->quality;
                onvifEncCfg.bitrateMode = strmConfig->bitrateMode;
                onvifEncCfg.gop = strmConfig->gop;
                onvifEncCfg.bitrateValue = strmConfig->bitrateValue;
                GetResolutionHeightWidth(strmConfig->resolution, &onvifEncCfg.resolHeight, &onvifEncCfg.resolWidth);
            }
            else
            {
                if (GetVideoCodecNum(strmConfig->videoEncodingSub, &onvifEncCfg.codec) == FAIL)
                {
                    EPRINT(ONVIF_CLIENT, "invld video codec for sub stream: [camera=%d], [profile=%d]", camIndex, profileIndex);
                    break;
                }

                onvifEncCfg.fps = strmConfig->framerateSub;
                onvifEncCfg.quality = strmConfig->qualitySub;
                onvifEncCfg.bitrateMode = strmConfig->bitrateModeSub;
                onvifEncCfg.gop = strmConfig->gopSub;
                onvifEncCfg.bitrateValue = strmConfig->bitrateValueSub;
                GetResolutionHeightWidth(strmConfig->resolutionSub, &onvifEncCfg.resolHeight, &onvifEncCfg.resolWidth);
            }

            // Set Video Profile to camera
            setVideoProfile(soap, &onvifEncCfg, user, camIndex, profileIndex);

            //If audio available then add it
            if (onvifCamInfo[camIndex].audioAvailable == YES)
            {
                if (((streamType == MAIN_STREAM) && (strmConfig->enableAudio == ENABLE)) ||
                        ((streamType == SUB_STREAM) && (strmConfig->enableAudioSub == ENABLE)))
                {
                    addAudioSourceConfiguration.ProfileToken = pMediaProfile->profileToken;
                    addAudioSourceConfiguration.ConfigurationToken = onvifCamInfo[camIndex].mediaServiceInfo.audioSourceToken;
                    soapResp = AddAudioSourceConfiguration(soap, &addAudioSourceConfiguration, &addAudioSourceConfigurationResponse, user);
                    if (soapResp != SOAP_OK)
                    {
                        EPRINT(ONVIF_CLIENT, "fail to add audio source config: [camera=%d], [profile=%d], [stream=%d], [soapResp=%d]",
                               camIndex, profileIndex, streamType, soapResp);
                    }

                    addAudioEncoderConfiguration.ProfileToken = pMediaProfile->profileToken;
                    addAudioEncoderConfiguration.ConfigurationToken = onvifCamInfo[camIndex].mediaServiceInfo.audioEncoderConfigurationToken;
                    soapResp = AddAudioEncoderConfiguration(soap, &addAudioEncoderConfiguration, &addAudioEncoderConfigurationResponse, user);
                    if (soapResp != SOAP_OK)
                    {
                        EPRINT(ONVIF_CLIENT, "fail to add audio encoder config: [camera=%d], [profile=%d], [stream=%d], [soapResp=%d]",
                               camIndex, profileIndex, streamType, soapResp);
                    }
                }
                else
                {
                    removeAudioEncoderConfiguration.ProfileToken = pMediaProfile->profileToken;
                    soapResp = RemoveAudioEncoderConfiguration(soap, &removeAudioEncoderConfiguration, &removeAudioEncoderConfigurationResponse, user);
                    if (soapResp != SOAP_OK)
                    {
                        EPRINT(ONVIF_CLIENT, "fail to remove audio encoder config: [camera=%d], [profile=%d], [stream=%d], [soapResp=%d]",
                               camIndex, profileIndex, streamType, soapResp);
                    }

                    removeAudioSourceConfiguration.ProfileToken = pMediaProfile->profileToken;
                    soapResp = RemoveAudioSourceConfiguration(soap, &removeAudioSourceConfiguration, &removeAudioSourceConfigurationResponse, user);
                    if(soapResp != SOAP_OK)
                    {
                        EPRINT(ONVIF_CLIENT, "fail to remove audio source config: [camera=%d], [profile=%d], [stream=%d], [soapResp=%d]",
                               camIndex, profileIndex, streamType, soapResp);
                    }
                }
            }

            /* We dont need to send stream command, As camera interface will send one more request with forceConfigOnCamera = FALSE */
            status = ONVIF_CMD_SUCCESS;
            break;
        }

        getStreamUri.ProfileToken = pMediaProfile->profileToken;
        getStreamUri.StreamSetup = &streamSetup;
        getStreamUri.StreamSetup->Transport = &transport;
        getStreamUri.StreamSetup->Stream = STREAM_TYPE_RTP_UNICAST;
        getStreamUri.StreamSetup->__any = NULL;
        soap_default_xsd__anyAttribute(NULL, &getStreamUri.StreamSetup->__anyAttribute);
        getStreamUri.StreamSetup->__size = 0;

        ReadSingleIpCameraConfig(camIndex, &ipCamCfg);
        if ((ipCamCfg.rtspProtocol == RTSP_OVER_TCP) && (onvifCamInfo[camIndex].mediaServiceInfo.rtspTranProtocol & (1 << TCP_INTERLEAVED)))
        {
            getStreamUri.StreamSetup->Transport->Protocol = TRANSPORT_PROTOCOL_INTERLEAVED;
            rtspTransportType = TCP_INTERLEAVED;
        }
        else if (ipCamCfg.rtspProtocol == RTSP_OVER_HTTP)
        {
            getStreamUri.StreamSetup->Transport->Protocol = TRANSPORT_PROTOCOL_HTTP;
            rtspTransportType = HTTP_TUNNELING;
        }
        else
        {
            getStreamUri.StreamSetup->Transport->Protocol = TRANSPORT_PROTOCOL_UDP;
            rtspTransportType = OVER_UDP;
        }

        getStreamUri.StreamSetup->Transport->Tunnel = NULL;

        soapResp = GetStreamUri(soap, &getStreamUri, &getStreamUriResponse, user);
        if (soapResp != SOAP_OK)
        {
            EPRINT(ONVIF_CLIENT, "fail to get stream uri: [camera=%d], [profile=%d], [stream=%d], [soapResp=%d]",
                   camIndex, profileIndex, streamType, soapResp);
            break;
        }

        if (getStreamUriResponse.MediaUri == NULL)
        {
            EPRINT(ONVIF_CLIENT, "media uri response not available: [camera=%d], [profile=%d], [stream=%d], [soapResp=%d]",
                   camIndex, profileIndex, streamType, soapResp);
            break;
        }

        /* Parse RTSP port from the onvif response */
        if (SUCCESS == ParseRtspPort(getStreamUriResponse.MediaUri->Uri, &ipCamCfg.rtspPort))
        {
            WriteSingleIpCameraConfig(camIndex, &ipCamCfg);
        }

        if (FAIL == ParseRequestUri(user->ipAddr, getStreamUriResponse.MediaUri->Uri, urlReq->relativeUrl, sizeof(urlReq->relativeUrl)))
        {
            EPRINT(ONVIF_CLIENT, "fail to parse media uri: [camera=%d], [profile=%d], [stream=%d]", camIndex, profileIndex, streamType);
            break;
        }

        DPRINT(ONVIF_CLIENT, "media uri found: [camera=%d], [profile=%d], [stream=%d], [rtspPort=%d], [url=%s]",
               camIndex, profileIndex, streamType, ipCamCfg.rtspPort, urlReq->relativeUrl);
        urlReq->protocolType      = CAM_RTSP_PROTOCOL;
        urlReq->requestType	 	  = CAM_REQ_MEDIA;
        urlReq->rtspTransportType = rtspTransportType;
        status = ONVIF_CMD_SUCCESS;
    }
    while(0);

    if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
    {
        status = ONVIF_CMD_TIMEOUT;
    }

    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief getMedia2Url  : will get Media RTSP URL from Camera
 * @param soap          : Soap instance
 * @param user          : required authentication information
 * @param streamType    : main or sub
 * @param strmConfig    : stream config
 * @param urlReq        : responded URL
 * @param camIndex      : camera index
 * @param forceConfigOnCamera  : set stream param if its value is true
 * @param profileNum    : profile index
 * @return
 */
static ONVIF_RESP_STATUS_e getMedia2Url(SOAP_t *soap, SOAP_USER_DETAIL_t *user, VIDEO_TYPE_e streamType, STREAM_CONFIG_t *strmConfig,
                                        URL_REQUEST_t *urlReq, UINT8 camIndex, BOOL forceConfigOnCamera, UINT8 profileNum)
{
    ADD_AUDIO_ENCODER_CONFIGURATION_t		 		addAudioEncoderConfiguration;
    ADD_AUDIO_ENCODER_CONFIGURATION_RESPONSE_t		addAudioEncoderConfigurationResponse;
    ADD_AUDIO_SOURCE_CONFIGURATION_t		 		addAudioSourceConfiguration;
    ADD_AUDIO_SOURCE_CONFIGURATION_RESPONSE_t		addAudioSourceConfigurationResponse;
    REMOVE_AUDIO_ENCODER_CONFIGURATION_t		  	removeAudioEncoderConfiguration;
    REMOVE_AUDIO_ENCODER_CONFIGURATION_RESPONSE_t 	removeAudioEncoderConfigurationResponse;
    REMOVE_AUDIO_SOURCE_CONFIGURATION_t 		 	removeAudioSourceConfiguration;
    REMOVE_AUDIO_SOURCE_CONFIGURATION_RESPONSE_t 	removeAudioSourceConfigurationResponse;
    GET_MEDIA2_STREAM_URI_t                         getStreamUri;
    GET_MEDIA2_STREAM_URI_RESPONSE_t                getStreamUriResponse;
    ONVIF_RESP_STATUS_e   							status = ONVIF_CMD_FAIL;
    INT16   										soapResp = SOAP_CLI_FAULT;
    ONVIF_ENC_CFG_t									onvifEncCfg;
    ONVIF_MEDIA_PROFILE_INFO_t						*pMediaProfile;
    IP_CAMERA_CONFIG_t								ipCamCfg;
    TRANSPORT_TYPE_e								rtspTransportType;
    UINT8                                           profileIndex;

    /* for other modules profile range is [1:MAX], here it is [0:MAX] */
    if ((profileNum == 0) || (profileNum > MAX_PROFILE_SUPPORT))
    {
        EPRINT(ONVIF_CLIENT, "invld profile number: [camera=%d], [profile=%d]", camIndex, profileNum);
        return ONVIF_CMD_FAIL;
    }

    /* index starting from 0 */
    profileIndex = profileNum - 1;

    /* Profile token must be present */
    pMediaProfile = &(onvifCamInfo[camIndex].mediaServiceInfo.mediaProfile[profileIndex]);
    if (pMediaProfile->profileToken[0] == '\0')
    {
        EPRINT(ONVIF_CLIENT, "profile token not available: [camera=%d], [profile=%d]", camIndex, profileIndex);
        return ONVIF_CMD_FAIL;
    }

    /* Get the time difference between NVR and Camera */
    getCameraDateTime(soap, camIndex, user);
    user->addr = onvifCamInfo[camIndex].mediaServiceInfo.mediaServiceAddr;
    user->timeDiff = &onvifCamInfo[camIndex].timeDifference;

    do
    {
        /* If TRUE then first given profile param will be set to camera stream profile otherwise no need to set profile
         * in camera, only get Media RTSP url from Camera */
        if (forceConfigOnCamera == TRUE)
        {
            DPRINT(ONVIF_CLIENT, "set nvr profile config in camera: [camera=%d], [profile=%d]", camIndex, profileIndex);
            if(streamType == MAIN_STREAM)
            {
                if (GetVideoCodecNum(strmConfig->videoEncoding, &onvifEncCfg.codec) == FAIL)
                {
                    EPRINT(ONVIF_CLIENT, "invld video codec for main stream: [camera=%d], [profile=%d]", camIndex, profileIndex);
                    break;
                }

                onvifEncCfg.fps = strmConfig->framerate;
                onvifEncCfg.quality = strmConfig->quality;
                onvifEncCfg.bitrateMode = strmConfig->bitrateMode;
                onvifEncCfg.gop = strmConfig->gop;
                onvifEncCfg.bitrateValue = strmConfig->bitrateValue;
                GetResolutionHeightWidth(strmConfig->resolution, &onvifEncCfg.resolHeight, &onvifEncCfg.resolWidth);
            }
            else
            {
                if (GetVideoCodecNum(strmConfig->videoEncodingSub, &onvifEncCfg.codec) == FAIL)
                {
                    EPRINT(ONVIF_CLIENT, "invld video codec for sub stream: [camera=%d], [profile=%d]", camIndex, profileIndex);
                    break;
                }

                onvifEncCfg.fps = strmConfig->framerateSub;
                onvifEncCfg.quality = strmConfig->qualitySub;
                onvifEncCfg.bitrateMode = strmConfig->bitrateModeSub;
                onvifEncCfg.gop = strmConfig->gopSub;
                onvifEncCfg.bitrateValue = strmConfig->bitrateValueSub;
                GetResolutionHeightWidth(strmConfig->resolutionSub, &onvifEncCfg.resolHeight, &onvifEncCfg.resolWidth);
            }

            // Set Video Profile
            setMedia2VideoProfile(soap, &onvifEncCfg, user, camIndex, profileIndex);

            //If audio available then add it
            if(onvifCamInfo[camIndex].audioAvailable == YES)
            {
                if(((streamType == MAIN_STREAM) && (strmConfig->enableAudio == ENABLE)) ||
                        ((streamType == SUB_STREAM) && (strmConfig->enableAudioSub == ENABLE)))
                {
                    addAudioSourceConfiguration.ProfileToken = pMediaProfile->profileToken;
                    addAudioSourceConfiguration.ConfigurationToken = onvifCamInfo[camIndex].mediaServiceInfo.audioSourceToken;
                    soapResp = AddAudioSourceConfiguration(soap, &addAudioSourceConfiguration, &addAudioSourceConfigurationResponse, user);
                    if (soapResp != SOAP_OK)
                    {
                        EPRINT(ONVIF_CLIENT, "fail to add audio source config: [camera=%d], [profile=%d], [stream=%d], [soapResp=%d]",
                               camIndex, profileIndex, streamType, soapResp);
                    }

                    addAudioEncoderConfiguration.ProfileToken = pMediaProfile->profileToken;
                    addAudioEncoderConfiguration.ConfigurationToken = onvifCamInfo[camIndex].mediaServiceInfo.audioEncoderConfigurationToken;
                    soapResp = AddAudioEncoderConfiguration(soap, &addAudioEncoderConfiguration, &addAudioEncoderConfigurationResponse, user);
                    if (soapResp != SOAP_OK)
                    {
                        EPRINT(ONVIF_CLIENT, "fail to add audio encoder config: [camera=%d], [profile=%d], [stream=%d], [soapResp=%d]",
                               camIndex, profileIndex, streamType, soapResp);
                    }
                }
                else
                {
                    removeAudioEncoderConfiguration.ProfileToken = pMediaProfile->profileToken;
                    soapResp = RemoveAudioEncoderConfiguration(soap, &removeAudioEncoderConfiguration, &removeAudioEncoderConfigurationResponse, user);
                    if (soapResp != SOAP_OK)
                    {
                        EPRINT(ONVIF_CLIENT, "fail to remove audio encoder config: [camera=%d], [profile=%d], [stream=%d], [soapResp=%d]",
                               camIndex, profileIndex, streamType, soapResp);
                    }

                    removeAudioSourceConfiguration.ProfileToken = pMediaProfile->profileToken;
                    soapResp = RemoveAudioSourceConfiguration(soap, &removeAudioSourceConfiguration, &removeAudioSourceConfigurationResponse, user);
                    if (soapResp != SOAP_OK)
                    {
                        EPRINT(ONVIF_CLIENT, "fail to remove audio source config: [camera=%d], [profile=%d], [stream=%d], [soapResp=%d]",
                               camIndex, profileIndex, streamType, soapResp);
                    }
                }
            }

            /* We dont need to send stream command, As camera interface will send one more request with forceConfigOnCamera = FALSE */
            status = ONVIF_CMD_SUCCESS;
            break;
        }

        ReadSingleIpCameraConfig(camIndex, &ipCamCfg);
        if (ipCamCfg.rtspProtocol == RTSP_OVER_TCP)
        {
            getStreamUri.Protocol = "RTSP";
            rtspTransportType = TCP_INTERLEAVED;
        }
        else if (ipCamCfg.rtspProtocol == RTSP_OVER_HTTP)
        {
            getStreamUri.Protocol = "RtspOverHttp";
            rtspTransportType = HTTP_TUNNELING;
        }
        else
        {
            getStreamUri.Protocol = "RtspUnicast";
            rtspTransportType = OVER_UDP;
        }

        getStreamUri.ProfileToken = pMediaProfile->profileToken;
        soapResp = GetMedia2StreamUri(soap, &getStreamUri, &getStreamUriResponse, user);
        if (soapResp != SOAP_OK)
        {
            EPRINT(ONVIF_CLIENT, "fail to get stream uri: [camera=%d], [profile=%d], [stream=%d], [soapResp=%d]",
                   camIndex, profileIndex, streamType, soapResp);
            break;
        }

        if (getStreamUriResponse.Uri == NULL)
        {
            EPRINT(ONVIF_CLIENT, "media uri response not available: [camera=%d], [profile=%d], [stream=%d], [soapResp=%d]",
                   camIndex, profileIndex, streamType, soapResp);
            break;
        }

        /* Parse RTSP port from the onvif response */
        if (SUCCESS == ParseRtspPort(getStreamUriResponse.Uri, &ipCamCfg.rtspPort))
        {
            WriteSingleIpCameraConfig(camIndex, &ipCamCfg);
        }

        if (FAIL == ParseRequestUri(user->ipAddr, getStreamUriResponse.Uri, urlReq->relativeUrl,sizeof(urlReq->relativeUrl)))
        {
            EPRINT(ONVIF_CLIENT, "fail to parse media uri: [camera=%d], [profile=%d], [stream=%d]", camIndex, profileIndex, streamType);
            break;
        }

        DPRINT(ONVIF_CLIENT, "media uri found: [camera=%d], [profile=%d], [stream=%d], [rtspPort=%d], [url=%s]",
               camIndex, profileIndex, streamType, ipCamCfg.rtspPort, urlReq->relativeUrl);
        urlReq->protocolType      = CAM_RTSP_PROTOCOL;
        urlReq->requestType	 	  = CAM_REQ_MEDIA;
        urlReq->rtspTransportType = rtspTransportType;
        status = ONVIF_CMD_SUCCESS;
    }
    while(0);

    if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
    {
        status = ONVIF_CMD_TIMEOUT;
    }

    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief getImageUrl   : will get snapshot URL from camera
 * @param soap          : soap instance
 * @param urlReq        : URL
 * @param user          : required authentication information
 * @param camIndex      : camrea index
 * @return
 */
static ONVIF_RESP_STATUS_e getImageUrl(SOAP_t *soap, URL_REQUEST_t *urlReq, SOAP_USER_DETAIL_t *user, UINT8 camIndex)
{
    GET_SNAPSHOT_URI_t 			getSnapshotUri;
    GET_SNAPSHOT_URI_RESPONSE_t getSnapshotUriResponse;
    INT16 						soapResp = SOAP_ERR;
    CHARPTR						profileToken = NULL;
    UINT8						profileIndex = 0;

    /* Get the time difference between NVR and Camera */
    getCameraDateTime(soap, camIndex, user);
    user->addr = onvifCamInfo[camIndex].mediaServiceInfo.mediaServiceAddr;
    user->timeDiff = &onvifCamInfo[camIndex].timeDifference;

    /* Try to get SNAPSHOT url for higher profiles, Usually snapshot is provided for JPEG profile */
    for (profileIndex = 0; profileIndex < MAX_PROFILE_SUPPORT; profileIndex++)
    {
        profileToken = onvifCamInfo[camIndex].mediaServiceInfo.mediaProfile[profileIndex].profileToken;
        if (profileToken[0] == '\0')
        {
            EPRINT(ONVIF_CLIENT, "media profile token not available: [camera=%d], [profile=%d]", camIndex, profileIndex);
            continue;
        }

        getSnapshotUri.ProfileToken = profileToken;
        soapResp = GetSnapshotUri(soap, &getSnapshotUri, &getSnapshotUriResponse, user);
        if ((soapResp != SOAP_OK) || (getSnapshotUriResponse.MediaUri == NULL))
        {
            EPRINT(ONVIF_CLIENT, "fail to get snapshot uri: [camera=%d], [profile=%d], [soapResp=%d]", camIndex, profileIndex, soapResp);
            continue;
        }

        /* No need to try for other profiles */
        if (FAIL == ParseRequestUri(user->ipAddr, getSnapshotUriResponse.MediaUri->Uri, urlReq->relativeUrl, sizeof(urlReq->relativeUrl)))
        {
            EPRINT(ONVIF_CLIENT, "fail to parse snapshot uri: [camera=%d], [profile=%d]", camIndex, profileIndex);
            return ONVIF_CMD_FAIL;
        }

        urlReq->protocolType	= CAM_HTTP_PROTOCOL;
        urlReq->requestType	 	= CAM_REQ_MEDIA;
        urlReq->authMethod		= AUTH_TYPE_ANY;
        urlReq->httpRequestType = GET_REQUEST;
        return ONVIF_CMD_SUCCESS;
    }

    if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
    {
        return ONVIF_CMD_TIMEOUT;
    }

    return ONVIF_CMD_FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief getImageUrl2  : will get snapshot URL from camera using Media2
 * @param soap          : soap instance
 * @param urlReq        : URL
 * @param user          : required authentication information
 * @param camIndex      : camrea index
 * @return
 */
static ONVIF_RESP_STATUS_e getImageUrl2(SOAP_t *soap, URL_REQUEST_t *urlReq, SOAP_USER_DETAIL_t *user, UINT8 camIndex)
{
    GET_SNAPSHOT_URI2_t 			getSnapshotUri;
    GET_SNAPSHOT_URI_RESPONSE2_t    getSnapshotUriResponse;
    INT16                           soapResp = SOAP_ERR;
    CHARPTR                         profileToken = NULL;
    UINT8                           profileIndex = 0;

    /* Get the time difference between NVR and Camera */
    getCameraDateTime(soap, camIndex, user);
    user->addr = onvifCamInfo[camIndex].mediaServiceInfo.mediaServiceAddr;
    user->timeDiff = &onvifCamInfo[camIndex].timeDifference;

    /* Try to get SNAPSHOT url for higher profiles, Usually snapshot is provided for JPEG profile */
    for(profileIndex = 0; profileIndex < MAX_PROFILE_SUPPORT; profileIndex++)
    {
        profileToken = onvifCamInfo[camIndex].mediaServiceInfo.mediaProfile[profileIndex].profileToken;
        if (profileToken[0] == '\0')
        {
            EPRINT(ONVIF_CLIENT, "media profile token not available: [camera=%d], [profile=%d]", camIndex, profileIndex);
            continue;
        }

        getSnapshotUri.ProfileToken = profileToken;
        soapResp = GetSnapshotUri2(soap, &getSnapshotUri, &getSnapshotUriResponse, user);
        if ((soapResp != SOAP_OK) || (getSnapshotUriResponse.Uri == NULL))
        {
            EPRINT(ONVIF_CLIENT, "fail to get snapshot uri: [camera=%d], [profile=%d], [soapResp=%d]", camIndex, profileIndex, soapResp);
            continue;
        }

        /* No need to try for other profiles */
        if (FAIL == ParseRequestUri(user->ipAddr, getSnapshotUriResponse.Uri, urlReq->relativeUrl, sizeof(urlReq->relativeUrl)))
        {
            EPRINT(ONVIF_CLIENT, "fail to parse snapshot uri: [camera=%d], [profile=%d]", camIndex, profileIndex);
            return ONVIF_CMD_FAIL;
        }

        urlReq->protocolType	= CAM_HTTP_PROTOCOL;
        urlReq->requestType	 	= CAM_REQ_MEDIA;
        urlReq->authMethod		= AUTH_TYPE_ANY;
        urlReq->httpRequestType = GET_REQUEST;
        return ONVIF_CMD_SUCCESS;
    }

    if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
    {
        return ONVIF_CMD_TIMEOUT;
    }

    return ONVIF_CMD_FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief setRelayOutPut    : set alaram state for camera
 * @param soap              : soap instance
 * @param alarmIndex        : alaram sensor index
 * @param action            : state active or inactive
 * @param user              : required authentication information
 * @param camIndex          : camera index
 * @return
 */
static ONVIF_RESP_STATUS_e setRelayOutPut(SOAP_t *soap, UINT8 alarmIndex, BOOL action, SOAP_USER_DETAIL_t *user, UINT8 camIndex)
{
    SET_RELAY_OUTPUT_STATE_t 		  	setRelayOutputState;
    SET_RELAY_OUTPUT_STATE_RESPONSE_t 	setRelayOutputStateResponse;
    INT16   							soapResp;

    /* Get the time difference between NVR and Camera */
    getCameraDateTime(soap, camIndex, user);
    user->addr = onvifCamInfo[camIndex].deviceEntryPoint;
    user->timeDiff = &onvifCamInfo[camIndex].timeDifference;
    setRelayOutputState.RelayOutputToken = onvifCamInfo[camIndex].relayToken[alarmIndex];
    setRelayOutputState.LogicalState = (action == ACTIVE) ? ALARM_ACTIVE : ALARM_INACTIVE;

    soapResp = SetRelayOutputState(soap, &setRelayOutputState, &setRelayOutputStateResponse, user);
    if(soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "fail to set relay output: [camera=%d], [serviceAddr=%s], [soapResp=%d]", camIndex, user->addr, soapResp);
        if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
        {
            return ONVIF_CMD_TIMEOUT;
        }

        return ONVIF_CMD_FAIL;
    }

    return ONVIF_CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief getBrandModel : Get Brand Model information from camera
 * @param soap          : soap instance
 * @param user          : required information
 * @param brandName     : brand name received
 * @param modelName     : model name received
 * @param camIndex      : camera index
 * @return
 */
static ONVIF_RESP_STATUS_e getBrandModel(SOAP_t *soap, SOAP_USER_DETAIL_t *user, CHARPTR brandName, CHARPTR modelName, UINT8 camIndex)
{
    GET_DEVICE_INFORMATION_t  		  	getDeviceInformation;
    GET_DEVICE_INFORMATION_RESPONSE_t 	getDeviceInformationResponse;
    INT16   							soapResp;
    BOOL                                isAuthFail = FALSE;

    soapResp = GetDeviceInformation(soap, &getDeviceInformation, &getDeviceInformationResponse, user, &isAuthFail);
    if (soapResp != SOAP_OK)
    {
        /* Camera interface will keep on retrying in case of timeout */
        if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
        {
            EPRINT(ONVIF_CLIENT, "get device information timeout: [camera=%d], [soapResp=%d]", camIndex, soapResp);
            return ONVIF_CMD_TIMEOUT;
        }

        /* Check authentication status of request. Added to display credential issue in UI */
        EPRINT(ONVIF_CLIENT, "fail to get device information: [camera=%d], [soapResp=%d], [isAuthFail=%d]", camIndex, soapResp, isAuthFail);
        return ((TRUE == isAuthFail) ? ONVIF_CMD_AUTHENTICATION_FAIL : ONVIF_CMD_FAIL);
    }

    if ((getDeviceInformationResponse.Manufacturer == NULL) || (getDeviceInformationResponse.Model == NULL))
    {
        EPRINT(ONVIF_CLIENT, "brand-model info not available: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        return ONVIF_CMD_FAIL;
    }

    /* Get updated model name if camera brand is MATRIX */
    if (strcmp(getDeviceInformationResponse.Manufacturer, MATRIX_BRAND_NAME) == STATUS_OK)
    {
        GetUpdatedMatrixCameraModelName(getDeviceInformationResponse.Model, MAX_MODEL_NAME_LEN);
    }

    /* Store brand name */
    snprintf(brandName, MAX_BRAND_NAME_LEN, "%s", getDeviceInformationResponse.Manufacturer);

    /* Store model name */
    snprintf(modelName, MAX_MODEL_NAME_LEN, "%s", getDeviceInformationResponse.Model);
    return ONVIF_CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief setUserPassword : set user & password to camera , required in case of first time login
 * @param soap            : soap instance
 * @param user            : user information
 * @return
 */
static ONVIF_RESP_STATUS_e setUserPassword(SOAP_t *soap, SOAP_USER_DETAIL_t *user)
{
    struct tt__User                 userConfiguration;
    SOAP_USER_DETAIL_t              userLocal;
    SET_USER_INFORMATION_t          setUserPassword;
    SET_USER_INFORMATION_RESPONSE_t setUserPasswordResponse;
    INT16                           soapResp;

    memcpy(&userLocal, user, sizeof(SOAP_USER_DETAIL_t));
    userConfiguration.Username = userLocal.name;
    userConfiguration.Password = userLocal.pwd;
    userConfiguration.Extension = NULL;
    userConfiguration.UserLevel = 0;
    soap_default_xsd__anyAttribute(NULL, &userConfiguration.__anyAttribute);
    setUserPassword.__sizeUser = 1;
    setUserPassword.User = &userConfiguration;
    soapResp = setUser(soap, &setUserPassword, &setUserPasswordResponse, &userLocal);
    if (soapResp != SOAP_OK)
    {
        if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
        {
            EPRINT(ONVIF_CLIENT, "set user password timeout: [soapResp=%d]", soapResp);
            return ONVIF_CMD_TIMEOUT;
        }

        EPRINT(ONVIF_CLIENT, "fail to set user password: [soapResp=%d]", soapResp);
        return ONVIF_CMD_FAIL;
    }

    return ONVIF_CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief setIris       : set iris configuration to camera
 * @param soap          : soap instance
 * @param user          : user information
 * @param sessionIndex  : Onvif session index
 * @param iris          : iris state
 * @param camIndex      : camera Index
 * @return
 */
static ONVIF_RESP_STATUS_e setIris(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT16 sessionIndex, CAMERA_IRIS_e iris, UINT8 camIndex)
{
    GET_IMAGING_SETTINGS_t 			getImagingSettings;
    GET_IMAGING_SETTINGS_RESPONSE_t getImagingSettingsResponse;
    SET_IMAGING_SETTINGS_t 			setImagingSettings;
    SET_IMAGING_SETTINGS_RESPONSE_t setImagingSettingsResponse;
    INT16   						soapResp;

    if (onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.action == STOP)
    {
        return ONVIF_CMD_SUCCESS;
    }

    /* If imaging service not available then return */
    if (onvifCamInfo[camIndex].imagingServiceInfo.irisInfo.irisSupport == NO_FUNCTION_SUPPORT)
    {
        EPRINT(ONVIF_CLIENT, "iris feature not supported: [camera=%d]", camIndex);
        return ONVIF_CMD_FEATURE_NOT_SUPPORTED;
    }

    do
    {
        /* Get the time difference between NVR and Camera */
        getCameraDateTime(soap, camIndex, user);
        user->addr = onvifCamInfo[camIndex].imagingServiceInfo.imagingServiceAddr;
        getImagingSettings.VideoSourceToken = onvifCamInfo[camIndex].imagingServiceInfo.videoSourceToken;
        soapResp = GetImagingSettings(soap, &getImagingSettings, &getImagingSettingsResponse, user);
        if(soapResp != SOAP_OK)
        {
            EPRINT(ONVIF_CLIENT, "fail to get imaging settings: [camera=%d], [serviceAddr=%s], [soapResp=%d]", camIndex, user->addr, soapResp);
            break;
        }

        setImagingSettings.VideoSourceToken = onvifCamInfo[camIndex].imagingServiceInfo.videoSourceToken;
        setImagingSettings.ForcePersistence = FALSE;
        if (getImagingSettingsResponse.ImagingSettings == NULL)
        {
            EPRINT(ONVIF_CLIENT, "iris imaging settings not available: [camera=%d]", camIndex);
            break;
        }

        setImagingSettings.ImagingSettings = getImagingSettingsResponse.ImagingSettings;
        if (getImagingSettingsResponse.ImagingSettings->Exposure == NULL)
        {
            EPRINT(ONVIF_CLIENT, "iris exposure settings not available: [camera=%d]", camIndex);
            break;
        }

        /* Set value of get image settings and update required values */
        setImagingSettings.ImagingSettings->Exposure = getImagingSettingsResponse.ImagingSettings->Exposure;
        if (iris == IRIS_AUTO)
        {
            setImagingSettings.ImagingSettings->Exposure->Mode = tt__ExposureMode__AUTO;
        }
        else
        {
            if (getImagingSettingsResponse.ImagingSettings->Exposure->Mode == tt__ExposureMode__AUTO)
            {
                setImagingSettings.ImagingSettings->Exposure->Mode = tt__ExposureMode__MANUAL;
            }
            else
            {
                if (setImagingSettings.ImagingSettings->Exposure->Iris)
                {
                    *setImagingSettings.ImagingSettings->Exposure->Iris = (*getImagingSettingsResponse.ImagingSettings->Exposure->Iris +
                                                                           ((iris == MAX_IRIS_OPTION) ? 0 : ((float)1.0 * irisStepOption[iris])));

                    if ((getImagingSettingsResponse.ImagingSettings->Exposure->MinIris != NULL) 
                        && (*setImagingSettings.ImagingSettings->Exposure->Iris < *getImagingSettingsResponse.ImagingSettings->Exposure->MinIris))
                    {
                        *setImagingSettings.ImagingSettings->Exposure->Iris = *getImagingSettingsResponse.ImagingSettings->Exposure->MinIris;
                    }
                    else if ((getImagingSettingsResponse.ImagingSettings->Exposure->MaxIris != NULL) 
                        && (*setImagingSettings.ImagingSettings->Exposure->Iris > *getImagingSettingsResponse.ImagingSettings->Exposure->MaxIris))
                    {
                        *setImagingSettings.ImagingSettings->Exposure->Iris = *getImagingSettingsResponse.ImagingSettings->Exposure->MaxIris;
                    }
                }
            }
        }

        soapResp = SetImagingSettings(soap, &setImagingSettings, &setImagingSettingsResponse, user);
        if (soapResp != SOAP_OK)
        {
            EPRINT(ONVIF_CLIENT, "fail to set imaging settings: [camera=%d], [serviceAddr=%s], [soapResp=%d]", camIndex, user->addr, soapResp);
            break;
        }

        return ONVIF_CMD_SUCCESS;
    }
    while(0);

    if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
    {
        return ONVIF_CMD_TIMEOUT;
    }

    return ONVIF_CMD_FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief ptzRelativeMove   : Handle PTZ Relative move command
 * @param soapPtr           : soap instance
 * @param userDeatil        : user information
 * @param sessionIndex      : Onvif session index
 * @param moveType          : PAN or TILT type
 * @return
 */
static ONVIF_RESP_STATUS_e ptzRelativeMove(SOAP_t *soap, SOAP_USER_DETAIL_t *userDeatil, UINT16 sessionIndex, PTZ_OPERATION_e moveType)
{
    INT16						soapResp;
    RELATIVE_MOVE_t 			relativeMoveParam;
    RELATIVE_MOVE_RESPONSE_t	relativeMoveRespParam;
    PTZ_VECTOR_t				ptzVectorParam;
    VECTOR2D_t					panTiltValue;
    VECTOR2D_t					panTiltSpeed;
    VECTOR1D_t					zoomValue;
    VECTOR1D_t					zoomSpeed;
    PTZ_SPEED_t					ptzSpeedParam;
    UINT8 						camIndex;

    camIndex = onvifClientInfo[sessionIndex].onvifReqPara.camIndex;

    /* Get the time difference between NVR and Camera */
    getCameraDateTime(soap, camIndex, userDeatil);
    userDeatil->addr = onvifCamInfo[camIndex].ptzServiceInfo.ptzServiceAddr;

    if (moveType == PAN_TILT_OPERATION)
    {
        // specify PTZ Value
        panTiltValue.x = ((onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.pan == MAX_PTZ_PAN_OPTION) ?
                    0 : (panMultiplier[onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.pan] * onvifCamInfo[camIndex].ptzServiceInfo.panStep));
        panTiltValue.y = ((onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.tilt == MAX_PTZ_TILT_OPTION) ?
                    0 : (tiltMultiplier[onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.tilt] * onvifCamInfo[camIndex].ptzServiceInfo.tiltStep));
        panTiltValue.space	= NULL;
        ptzVectorParam.PanTilt = &panTiltValue;
        ptzVectorParam.Zoom = NULL;

        // Fill Speed Parameter
        panTiltSpeed.x = (onvifCamInfo[camIndex].ptzServiceInfo.panStep * onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.speed);
        panTiltSpeed.y = (onvifCamInfo[camIndex].ptzServiceInfo.tiltStep * onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.speed);
        panTiltSpeed.space = NULL;
        ptzSpeedParam.PanTilt = &panTiltSpeed;
        ptzSpeedParam.Zoom = NULL;
    }
    else
    {
        // specify PTZ Value
        zoomValue.x = ((onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.zoom == MAX_PTZ_ZOOM_OPTION) ?
                    0 : (zoomMultiplier[onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.zoom] * onvifCamInfo[camIndex].ptzServiceInfo.zoomStep));
        zoomValue.space = NULL;
        ptzVectorParam.Zoom = &zoomValue;
        ptzVectorParam.PanTilt = NULL;

        // Fill Speed Parameter
        zoomSpeed.x = onvifCamInfo[camIndex].ptzServiceInfo.zoomSpeed;
        zoomSpeed.space = NULL;
        ptzSpeedParam.PanTilt = NULL;
        ptzSpeedParam.Zoom = &zoomSpeed;
    }

    // Fill relative move Param
    relativeMoveParam.ProfileToken 	= onvifCamInfo[camIndex].ptzServiceInfo.profileToken;
    relativeMoveParam.Translation 	= &ptzVectorParam;
    relativeMoveParam.Speed 		= &ptzSpeedParam;

    // PTZ Relative Move Cmd
    soapResp = RelativeMove(soap, &relativeMoveParam, &relativeMoveRespParam, userDeatil);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "ptz relative move failed: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
        {
            return ONVIF_CMD_TIMEOUT;
        }

        return ONVIF_CMD_FAIL;
    }

    return ONVIF_CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief ptzAbsoluteMove   : Handle PTZ absolute move feature
 * @param soapPtr           : Soap instance
 * @param userDeatil        : User information
 * @param sessionIndex      : session index
 * @param moveType          : PAN or TILT operation
 * @return
 */
static ONVIF_RESP_STATUS_e ptzAbsoluteMove(SOAP_t *soap, SOAP_USER_DETAIL_t *userDeatil, UINT16 sessionIndex, PTZ_OPERATION_e moveType)
{
    INT16						soapResp;
    ABSOLUTE_MOVE_t				absoluteMoveParam;
    ABSOLUTE_MOVE_RESPONSE_t	absoluteMoveRespParam;
    PTZ_VECTOR_t				ptzVectorParam;
    VECTOR2D_t					panTiltValue;
    VECTOR2D_t					panTiltSpeed;
    VECTOR1D_t					zoomValue;
    VECTOR1D_t					zoomSpeed;
    PTZ_SPEED_t					ptzSpeedParam;
    GET_PTZ_STATUS_t			getStatusParam;
    GET_PTZ_STATUS_RESPONSE_t	currStatus;
    UINT8 						camIndex;

    camIndex = onvifClientInfo[sessionIndex].onvifReqPara.camIndex;

    /* Get the time difference between NVR and Camera */
    getCameraDateTime(soap, camIndex, userDeatil);
    userDeatil->addr = onvifCamInfo[camIndex].ptzServiceInfo.ptzServiceAddr;
    getStatusParam.ProfileToken = onvifCamInfo[camIndex].ptzServiceInfo.profileToken;

    // Get Current PTZ Position
    soapResp = GetPTZStatus(soap, &getStatusParam, &currStatus, userDeatil);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "fail to get ptz status: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
        {
            return ONVIF_CMD_TIMEOUT;
        }

        return ONVIF_CMD_FAIL;
    }

    if (moveType == PAN_TILT_OPERATION)
    {
        // Fill PTZ Value as per request
        panTiltValue.x = (currStatus.PTZStatus->Position->PanTilt->x +
                          (((onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.pan == MAX_PTZ_PAN_OPTION) ?
                                0 : panMultiplier[onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.pan]) * onvifCamInfo[camIndex].ptzServiceInfo.panStep));
        panTiltValue.y = (currStatus.PTZStatus->Position->PanTilt->y +
                          (((onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.tilt == MAX_PTZ_TILT_OPTION) ?
                                0 : tiltMultiplier[onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.tilt]) * onvifCamInfo[camIndex].ptzServiceInfo.tiltStep));
        panTiltValue.space 		= NULL;
        ptzVectorParam.PanTilt 	= &panTiltValue;
        ptzVectorParam.Zoom 	= NULL;

        // Fill Speed Parameter
        panTiltSpeed.x 			= (onvifCamInfo[camIndex].ptzServiceInfo.panStep * onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.speed);
        panTiltSpeed.y 			= (onvifCamInfo[camIndex].ptzServiceInfo.tiltStep * onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.speed);
        panTiltSpeed.space 		= NULL;
        ptzSpeedParam.PanTilt 	= &panTiltSpeed;
        ptzSpeedParam.Zoom 		= NULL;
    }
    else
    {
        // Fill PTZ Value as per request
        zoomValue.x = (currStatus.PTZStatus->Position->Zoom->x +
                       ((onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.zoom == MAX_PTZ_ZOOM_OPTION) ?
                            0 : (zoomMultiplier[onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.zoom] * onvifCamInfo[camIndex].ptzServiceInfo.zoomStep)));
        zoomValue.space 		= NULL;
        ptzVectorParam.PanTilt 	= NULL;
        ptzVectorParam.Zoom 	= &zoomValue;

        // Fill Speed Parameter
        zoomSpeed.x 			= onvifCamInfo[camIndex].ptzServiceInfo.zoomSpeed;
        zoomSpeed.space 		= NULL;
        ptzSpeedParam.PanTilt 	= NULL;
        ptzSpeedParam.Zoom 		= &zoomSpeed;
    }

    absoluteMoveParam.Speed			= &ptzSpeedParam;
    absoluteMoveParam.ProfileToken	= onvifCamInfo[camIndex].ptzServiceInfo.profileToken;
    absoluteMoveParam.Position 		= &ptzVectorParam;

    // Absolute PTZ Move cmd
    soapResp = AbsoluteMove(soap, &absoluteMoveParam, &absoluteMoveRespParam, userDeatil);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "ptz absolute move failed: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
        {
            return ONVIF_CMD_TIMEOUT;
        }

        return ONVIF_CMD_FAIL;
    }

    return ONVIF_CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief ptzContinuousMove : PTZ continous move handling
 * @param soapPtr           : SOAP instance
 * @param userDeatil        : user Information
 * @param sessionIndex      : session index
 * @param moveType          : PAN or TILT
 * @return
 */
static ONVIF_RESP_STATUS_e ptzContinuousMove(SOAP_t *soap, SOAP_USER_DETAIL_t *userDeatil, UINT16 sessionIndex, PTZ_OPERATION_e moveType)
{
    INT16						soapResp;
    PTZ_SPEED_t					ptzSpeedParam;
    VECTOR2D_t					panTiltValue;
    VECTOR1D_t					zoomValue;
    CONTINUOUS_MOVE_t			continuousMoveParam;
    CONTINUOUS_MOVE_RESPONSE_t	continuousMoveRespParam;
    STOP_PTZ_t					stopParam;
    STOP_PTZ_RESPONSE_t			stopRespParam;
    UINT8 						camIndex;
    enum xsd__boolean           value = XSD_BOOL_TRUE;

    camIndex = onvifClientInfo[sessionIndex].onvifReqPara.camIndex;

    /* Get the time difference between NVR and Camera */
    getCameraDateTime(soap, camIndex, userDeatil);
    userDeatil->addr = onvifCamInfo[camIndex].ptzServiceInfo.ptzServiceAddr;

    DPRINT(ONVIF_CLIENT, "ptz operation: [camera=%d], [moveType=%d], [action=%d], [pan=%d], [tilt=%d], [zoom=%d], [speed=%f]",
           camIndex, moveType, onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.action, onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.pan,
           onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.tilt, onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.zoom,
           onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.speed);

    if (onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.action == STOP)
    {
        /* Stop PTZ Move Cmd */
        DPRINT(ONVIF_CLIENT, "ptz continuous move stop: [camera=%d]", camIndex);
        stopParam.ProfileToken 	= onvifCamInfo[camIndex].ptzServiceInfo.profileToken;
        stopParam.PanTilt 		= &value;
        stopParam.Zoom 			= &value;
        soapResp = StopPTZ(soap, &stopParam, &stopRespParam, userDeatil);
        if (soapResp != SOAP_OK)
        {
            EPRINT(ONVIF_CLIENT, "stop ptz continuous move failed: [camera=%d], [soapResp=%d]", camIndex, soapResp);
            if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
            {
                return ONVIF_CMD_TIMEOUT;
            }

            return ONVIF_CMD_FAIL;
        }

        return ONVIF_CMD_SUCCESS;
    }

    if (moveType == PAN_TILT_OPERATION)
    {
        panTiltValue.x = ((onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.pan == MAX_PTZ_PAN_OPTION) ?
                              0 : (panMultiplier[onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.pan] *
                              (onvifCamInfo[camIndex].ptzServiceInfo.panStep * onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.speed)));
        panTiltValue.y = ((onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.tilt == MAX_PTZ_TILT_OPTION) ?
                              0 : (tiltMultiplier[onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.tilt] *
                              (onvifCamInfo[camIndex].ptzServiceInfo.tiltStep * onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.speed)));
        panTiltValue.space = NULL;
        ptzSpeedParam.PanTilt = &panTiltValue;
        ptzSpeedParam.Zoom = NULL;

        DPRINT(ONVIF_CLIENT, "pan-tilt operation: [camera=%d], [moveType=%d]", camIndex, moveType);
    }
    else
    {
        zoomValue.x = ((onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.zoom == MAX_PTZ_ZOOM_OPTION)
                       ? 0 : (zoomMultiplier[onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.zoom] *
                         onvifCamInfo[camIndex].ptzServiceInfo.zoomStep * onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.speed));
        zoomValue.space = NULL;
        ptzSpeedParam.PanTilt = NULL;
        ptzSpeedParam.Zoom = &zoomValue;
        DPRINT(ONVIF_CLIENT, "zoom operation: [camera=%d], [moveType=%d]", camIndex, moveType);
    }

    /* Continuous PTZ Move cmd */
    continuousMoveParam.ProfileToken 	= onvifCamInfo[camIndex].ptzServiceInfo.profileToken;
    continuousMoveParam.Timeout 		= NULL;
    continuousMoveParam.Velocity		= &ptzSpeedParam;
    soapResp = ContinuousMove(soap, &continuousMoveParam, &continuousMoveRespParam, userDeatil);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "ptz continuous move operation failed: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
        {
            return ONVIF_CMD_TIMEOUT;
        }

        return ONVIF_CMD_FAIL;
    }

    return ONVIF_CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set camera position to preconfigured preset location
 * @param   soap: SOAP instance
 * @param   userDeatil: User Information
 * @param   sessionIndex: Session Index
 * @param   pPtzPresetCfg: ptz preset configuration
 * @return
 */
static ONVIF_RESP_STATUS_e gotoPresetCmd(SOAP_t *soap, SOAP_USER_DETAIL_t *userDeatil, UINT16 sessionIndex, PTZ_PRESET_CONFIG_t *pPtzPresetCfg)
{
    INT16					soapResp;
    GOTO_PRESET_t			gotoPresetParam;
    GOTO_PRESET_RESPONSE_t	gotoPresetRespParam;
    VECTOR2D_t				panTiltSpeed;
    VECTOR1D_t				zoomSpeed;
    PTZ_SPEED_t				ptzSpeedParam;
    UINT8 					camIndex;

    /* Name and token must be present to set preset position */
    if ((pPtzPresetCfg->name[0] == '\0') || (pPtzPresetCfg->token[0] == '\0'))
    {
        return ONVIF_CMD_FAIL;
    }

    /* Get the time difference between NVR and Camera */
    camIndex = onvifClientInfo[sessionIndex].onvifReqPara.camIndex;
    getCameraDateTime(soap, camIndex, userDeatil);
    userDeatil->addr = onvifCamInfo[camIndex].ptzServiceInfo.ptzServiceAddr;

    /* Set goto ptz preset parameters */
    panTiltSpeed.space = NULL;
    panTiltSpeed.x = onvifCamInfo[camIndex].ptzServiceInfo.pantiltSpeed;
    panTiltSpeed.y = onvifCamInfo[camIndex].ptzServiceInfo.pantiltSpeed;
    zoomSpeed.x = onvifCamInfo[camIndex].ptzServiceInfo.zoomSpeed;
    zoomSpeed.space = NULL;
    ptzSpeedParam.PanTilt = &panTiltSpeed;
    ptzSpeedParam.Zoom = &zoomSpeed;
    gotoPresetParam.Speed = &ptzSpeedParam;
    gotoPresetParam.ProfileToken = onvifCamInfo[camIndex].ptzServiceInfo.profileToken;
    gotoPresetParam.PresetToken = pPtzPresetCfg->token;

    /* Send goto ptz preset command */
    soapResp = GoToPreset(soap, &gotoPresetParam, &gotoPresetRespParam, userDeatil);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "goto ptz preset position failed: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
        {
            return ONVIF_CMD_TIMEOUT;
        }

        return ONVIF_CMD_FAIL;
    }

    return ONVIF_CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Remove ptz preset position from camera
 * @param   soap: SOAP instance
 * @param   userDeatil: User Information
 * @param   sessionIndex: session index
 * @param   pPtzPresetCfg: ptz preset configuration
 * @return
 */
static ONVIF_RESP_STATUS_e removePresetCmd(SOAP_t *soap, SOAP_USER_DETAIL_t *userDeatil, UINT16 sessionIndex, PTZ_PRESET_CONFIG_t *pPtzPresetCfg)
{
    INT16						soapResp;
    REMOVE_PRESET_t				removePresetParam;
    REMOVE_PRESET_RESPONSE_t	removePresetRespParam;
    UINT8 						camIndex;

    /* Nothing to do if PTZ preset token is not present */
    if (pPtzPresetCfg->token[0] == '\0')
    {
        return ONVIF_CMD_SUCCESS;
    }

    /* Get the time difference between NVR and Camera */
    camIndex = onvifClientInfo[sessionIndex].onvifReqPara.camIndex;
    getCameraDateTime(soap, camIndex, userDeatil);
    userDeatil->addr = onvifCamInfo[camIndex].ptzServiceInfo.ptzServiceAddr;

    /* Set remove ptz preset parameters */
    removePresetParam.PresetToken = pPtzPresetCfg->token;
    removePresetParam.ProfileToken = onvifCamInfo[camIndex].ptzServiceInfo.profileToken;

    /* Send remove ptz preset command */
    soapResp = RemovePreset(soap, &removePresetParam, &removePresetRespParam, userDeatil);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "remove ptz preset position failed: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
        {
            return ONVIF_CMD_TIMEOUT;
        }

        return ONVIF_CMD_FAIL;
    }

    return ONVIF_CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Add ptz preset postion to camera
 * @param   soap: SOAP instance
 * @param   userDeatil: User information
 * @param   sessionIndex: session index
 * @param   pPtzPresetCfg: ptz preset configuration
 * @return
 */
static ONVIF_RESP_STATUS_e setPresetCmd(SOAP_t *soap, SOAP_USER_DETAIL_t *userDeatil, UINT16 sessionIndex, PTZ_PRESET_CONFIG_t *pPtzPresetCfg)
{
    INT16					soapResp;
    SET_PRESET_t			setPresetParam;
    SET_PRESET_RESPONSE_t	setPresetRespParam;
    UINT8 					camIndex;
    UINT8					presetIndex;

    /* Name must be present to create the preset */
    if (pPtzPresetCfg->name[0] == '\0')
    {
        return ONVIF_CMD_FAIL;
    }

    /* Get the time difference between NVR and Camera */
    camIndex = onvifClientInfo[sessionIndex].onvifReqPara.camIndex;
    presetIndex = onvifClientInfo[sessionIndex].onvifReqPara.actionInfo.index;
    getCameraDateTime(soap, camIndex, userDeatil);
    userDeatil->addr = onvifCamInfo[camIndex].ptzServiceInfo.ptzServiceAddr;

    /* Set Token to NULL to create a new preset */
    setPresetParam.ProfileToken = onvifCamInfo[camIndex].ptzServiceInfo.profileToken;
    setPresetParam.PresetToken = NULL;
    setPresetParam.PresetName = pPtzPresetCfg->name;

    /* Send set ptz preset command */
    soapResp = SetPreset(soap, &setPresetParam, &setPresetRespParam, userDeatil);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "set ptz preset position failed: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
        {
            return ONVIF_CMD_TIMEOUT;
        }

        return ONVIF_CMD_FAIL;
    }

    if (setPresetRespParam.PresetToken == NULL)
    {
        return ONVIF_CMD_FAIL;
    }

    /* Store created ptz token in the configuration */
    StringNCopy(pPtzPresetCfg->token, setPresetRespParam.PresetToken, MAX_PRESET_TOKEN_LEN);
    WriteSinglePtzPresetConfig(camIndex, presetIndex, pPtzPresetCfg);
    return ONVIF_CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief focusRelativeMove : sets camera focus with input param
 * @param soapPtr           : SOAP instance
 * @param userDeatil        : user infomation
 * @param sessionIndex      : session index
 * @return
 */
static ONVIF_RESP_STATUS_e focusRelativeMove(SOAP_t *soap, SOAP_USER_DETAIL_t *userDeatil, UINT16 sessionIndex)
{
    INT16				soapResp;
    MOVE_t				moveParam;
    MOVE_RESPONSE_t		moveRespParam;
    FOCUS_MOVE_t		focusMove;
    RELATIVE_FOCUS_t	relativeFocus;
    UINT8 				camIndex;

    camIndex = onvifClientInfo[sessionIndex].onvifReqPara.camIndex;

    /* Get the time difference between NVR and Camera */
    getCameraDateTime(soap, camIndex, userDeatil);
    userDeatil->addr = onvifCamInfo[camIndex].imagingServiceInfo.imagingServiceAddr;

    relativeFocus.Distance = ((onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.action == START) ?
                                  (focusMultiplier[onvifClientInfo[sessionIndex].onvifReqPara.focusInfo] * onvifCamInfo[camIndex].imagingServiceInfo.focusInfo.focusStep) : 0);
    relativeFocus.Speed	= (onvifCamInfo[camIndex].imagingServiceInfo.focusInfo.focusSpeed == 0) ? NULL : &onvifCamInfo[camIndex].imagingServiceInfo.focusInfo.focusSpeed;
    focusMove.Relative = &relativeFocus;
    focusMove.Absolute = NULL;
    focusMove.Continuous = NULL;
    moveParam.VideoSourceToken = onvifCamInfo[camIndex].imagingServiceInfo.videoSourceToken;
    moveParam.Focus = &focusMove;

    // Focus Relative Move Cmd
    soapResp = FocusMove(soap, &moveParam, &moveRespParam, userDeatil);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "focus relative move failed: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
        {
            return ONVIF_CMD_TIMEOUT;
        }

        return ONVIF_CMD_FAIL;
    }

    return ONVIF_CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief focusAbsoluteMove : set focus value to camera
 * @param soapPtr           : SOAP instance
 * @param userDeatil        : USER inforamtion
 * @param sessionIndex      : Session index
 * @return
 */
static ONVIF_RESP_STATUS_e focusAbsoluteMove(SOAP_t *soap, SOAP_USER_DETAIL_t *userDeatil, UINT16 sessionIndex)
{
    INT16						soapResp;
    MOVE_t						moveParam;
    MOVE_RESPONSE_t				moveRespParam;
    FOCUS_MOVE_t				focusMove;
    GET_FOCUS_STATUS_t			getFocusStatusParam;
    GET_FOCUS_STATUS_RESPONSE_t	currFocusStatus;
    ABSOLUTE_FOCUS_t			absoluteFocus;
    UINT8 						camIndex;

    camIndex = onvifClientInfo[sessionIndex].onvifReqPara.camIndex;

    /* Get the time difference between NVR and Camera */
    getCameraDateTime(soap, camIndex, userDeatil);
    userDeatil->addr = onvifCamInfo[camIndex].imagingServiceInfo.imagingServiceAddr;
    getFocusStatusParam.VideoSourceToken = onvifCamInfo[camIndex].imagingServiceInfo.videoSourceToken;

    // Get Current Focus Status
    soapResp = GetFocusStatus(soap, &getFocusStatusParam, &currFocusStatus, userDeatil);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "get focus status failed: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
        {
            return ONVIF_CMD_TIMEOUT;
        }

        return ONVIF_CMD_FAIL;
    }

    // Fill appropriate Focus value as per request
    absoluteFocus.Position =(currFocusStatus.Status->FocusStatus20->Position
                             + (((onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.action == START) ?
                                     focusMultiplier[onvifClientInfo[sessionIndex].onvifReqPara.focusInfo] : 0) * onvifCamInfo[camIndex].imagingServiceInfo.focusInfo.focusStep));
    absoluteFocus.Speed = &onvifCamInfo[camIndex].imagingServiceInfo.focusInfo.focusSpeed;
    focusMove.Relative = NULL;
    focusMove.Absolute = &absoluteFocus;
    focusMove.Continuous = NULL;
    moveParam.VideoSourceToken = onvifCamInfo[camIndex].imagingServiceInfo.videoSourceToken;
    moveParam.Focus = &focusMove;
    soapResp = FocusMove(soap, &moveParam, &moveRespParam, userDeatil);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "focus absolute move failed: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
        {
            return ONVIF_CMD_TIMEOUT;
        }

        return ONVIF_CMD_FAIL;
    }

    return ONVIF_CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief focusContinuousMove   : sets focus to camera
 * @param soapPtr               : SOAP Instance
 * @param userDeatil            : user information
 * @param sessionIndex          : session index
 * @return
 */
static ONVIF_RESP_STATUS_e focusContinuousMove(SOAP_t *soap, SOAP_USER_DETAIL_t *userDeatil, UINT16 sessionIndex)
{
    INT16					soapResp;
    MOVE_t					moveParam;
    MOVE_RESPONSE_t			moveRespParam;
    FOCUS_MOVE_t			focusMove;
    CONTINUOUS_FOCUS_t		continuousFocusParam;
    STOP_FOCUS_t			stopFocusParam;
    STOP_FOCUS_RESPONSE_t	stopFocusRespParam;
    UINT8 					camIndex;

    camIndex = onvifClientInfo[sessionIndex].onvifReqPara.camIndex;

    /* Get the time difference between NVR and Camera */
    getCameraDateTime(soap, camIndex, userDeatil);
    userDeatil->addr = onvifCamInfo[camIndex].imagingServiceInfo.imagingServiceAddr;

    if((onvifClientInfo[sessionIndex].onvifReqPara.focusInfo != MAX_FOCUS_OPTION) && (onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.action != STOP))
    {
        continuousFocusParam.Speed = (((onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.action == START) ?
                                           focusMultiplier[onvifClientInfo[sessionIndex].onvifReqPara.focusInfo] : 0) *
                                      (onvifCamInfo[camIndex].imagingServiceInfo.focusInfo.focusStep * onvifClientInfo[sessionIndex].onvifReqPara.ptzInfo.speed));
        focusMove.Relative = NULL;
        focusMove.Absolute = NULL;
        focusMove.Continuous = &continuousFocusParam;
        moveParam.VideoSourceToken = onvifCamInfo[camIndex].imagingServiceInfo.videoSourceToken;
        moveParam.Focus = &focusMove;

        // Continuous Focus Command
        soapResp = FocusMove(soap, &moveParam, &moveRespParam, userDeatil);
        if (soapResp != SOAP_OK)
        {
            EPRINT(ONVIF_CLIENT, "focus continuous move failed: [camera=%d], [soapResp=%d]", camIndex, soapResp);
            if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
            {
                return ONVIF_CMD_TIMEOUT;
            }

            return ONVIF_CMD_FAIL;
        }
    }
    else
    {
        // Stop Focus Command
        stopFocusParam.VideoSourceToken = onvifCamInfo[camIndex].imagingServiceInfo.videoSourceToken;
        soapResp = StopFocus(soap, &stopFocusParam, &stopFocusRespParam, userDeatil);
        if (soapResp != SOAP_OK)
        {
            EPRINT(ONVIF_CLIENT, "stop focus failed: [camera=%d], [soapResp=%d]", camIndex, soapResp);
            if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
            {
                return ONVIF_CMD_TIMEOUT;
            }

            return ONVIF_CMD_FAIL;
        }
    }

    return ONVIF_CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief focusAutoOperation : sets focus to camera
 * @param soapPtr            : SOAP instance
 * @param userDeatil         : User information
 * @param sessionIndex       : session index
 * @return
 */
static ONVIF_RESP_STATUS_e focusAutoOperation(SOAP_t *soap, SOAP_USER_DETAIL_t *userDeatil, UINT16 sessionIndex)
{
    INT16							soapResp;
    SET_IMAGING_SETTINGS_t			setImagingSettParam;
    SET_IMAGING_SETTINGS_RESPONSE_t	setImagingSettRespParam;
    FOCUS_CONFIGURATION20_t			focusConfig;
    IMAGING_SETTINGS20_t			imaggingSett;
    UINT8 							camIndex;

    /* Get the time difference between NVR and Camera */
    camIndex = onvifClientInfo[sessionIndex].onvifReqPara.camIndex;
    getCameraDateTime(soap, camIndex, userDeatil);
    userDeatil->addr = onvifCamInfo[camIndex].imagingServiceInfo.imagingServiceAddr;

    // set all pointers to NULL
    memset(&focusConfig, 0, sizeof(focusConfig));
    memset(&imaggingSett, 0, sizeof(imaggingSett));

    focusConfig.AutoFocusMode = ONVIF_AUTO_FOCUS_MODE;
    imaggingSett.Focus = &focusConfig;
    setImagingSettParam.VideoSourceToken = onvifCamInfo[camIndex].imagingServiceInfo.videoSourceToken;
    setImagingSettParam.ImagingSettings = &imaggingSett;
    setImagingSettParam.ForcePersistence = NULL;

    // Set Imaging Settings for Auto Focus
    soapResp = SetImagingSettings(soap, &setImagingSettParam, &setImagingSettRespParam, userDeatil);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "set auto focus failed: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
        {
            return ONVIF_CMD_TIMEOUT;
        }

        return ONVIF_CMD_FAIL;
    }

    return ONVIF_CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief getEventCapabilities : It will gets all supported camera events tag and compare with predefine
 *                               Tags, if matched then that event will be supported in NVR
 * @param soap                 : SOAP instance
 * @param user                 : User information
 * @param camIndex             : Camera index
 * @return
 */
static BOOL getEventCapabilities(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 camIndex)
{
    BOOL                            status = FAIL;
    INT16                           soapResp;
    GET_EVENT_PROPERTIES_t          getEventProperties;
    GET_EVENT_PROPERTIES_RESPONSE_t getEventPropertiesResponse;
    UINT8                           cnt;
    UINT8                           evtTagCnt;
    UINT8                           tagCnt;
    SOAP_WSA5_t                     wsa5Info;
    CHAR                            currentTag[MAX_EVENT_STRING_LENGTH];
    UINT8                           tagLen;
    const CHAR                      *tagName = NULL;
    struct soap_dom_element         *subDom = NULL;
    struct soap_dom_element         *currentDom = NULL;

    user->addr = onvifCamInfo[camIndex].eventServiceInfo.eventServiceUri;
    wsa5Info.addInfo 		= NULL;
    wsa5Info.wsa5__Action 	= GET_EVENT_PROPERTIES_ACTION;
    wsa5Info.wsa5_to 		= user->addr;
    soapResp = GetEventProperties(soap, &getEventProperties, &getEventPropertiesResponse, user,&wsa5Info);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "fail to get event properties: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        return FAIL;
    }

    //Sets all tag values to zero
    for(cnt = ONVIF_MOTION_EVENT; cnt < ONVIF_MAX_EVENT; cnt++)
    {
        onvifCamInfo[camIndex].eventServiceInfo.onvifEvRespDet[cnt].tagAvailable = 0;
    }

    if(getEventPropertiesResponse.wstop__TopicSet == NULL)
    {
        EPRINT(ONVIF_CLIENT, "event topic set not available: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        return FAIL;
    }

    //For each parent node (refer SOAP Dom structure)
    for(cnt = 0; cnt < getEventPropertiesResponse.wstop__TopicSet->__size; cnt++)
    {
        currentDom = (getEventPropertiesResponse.wstop__TopicSet->__any + cnt);
        if(currentDom == NULL)
        {
            continue;
        }

        subDom = soap_elt_first(currentDom);
        while(subDom)
        {
            memset(currentTag, '\0', MAX_EVENT_STRING_LENGTH);
            tagName = soap_elt_get_tag(currentDom);
            if(tagName == NULL)
            {
                break;
            }

            removeNameSpace(tagName, currentTag);
            tagLen = strlen(currentTag);
            if (parseEventTag(currentTag, subDom, tagLen) == FAIL)
            {
                EPRINT(ONVIF_CLIENT, "fail to parse event tag: [camera=%d]", camIndex);
                subDom = soap_elt_next(subDom);
                continue;
            }

            //so war we have all tags supported by camera , now compare with NVR supported Tags
            for(evtTagCnt = ONVIF_MOTION_EVENT; evtTagCnt < ONVIF_MAX_EVENT; evtTagCnt++)
            {
                tagCnt = 0;
                while(availableTags[evtTagCnt][tagCnt] != NULL)
                {
                    if (tagSequenceCompare(currentTag, availableTags[evtTagCnt][tagCnt]) == SUCCESS)
                    {
                        //This Event is supported by NVR , store all available Tags
                        if(onvifCamInfo[camIndex].eventServiceInfo.onvifEvRespDet[evtTagCnt].tagAvailable < MAX_TAG_CNT)
                        {
                            snprintf(onvifCamInfo[camIndex].eventServiceInfo.onvifEvRespDet[evtTagCnt].eventTag
                                     [onvifCamInfo[camIndex].eventServiceInfo.onvifEvRespDet[evtTagCnt].tagAvailable],
                                    MAX_TAG_LEN, "%s",availableTags[evtTagCnt][tagCnt]);
                            onvifCamInfo[camIndex].eventServiceInfo.onvifEvRespDet[evtTagCnt].tagAvailable++;
                        }
                        else
                        {
                            WPRINT(ONVIF_CLIENT, "max event tag limit reached: [camera=%d], [evtTagCnt=%d]", camIndex, evtTagCnt);
                            break;
                        }
                        status = SUCCESS;
                    }
                    tagCnt++;
                }
            }
            subDom = soap_elt_next(subDom);
        }
    }

    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief startOnvifEvent   : Will subscribe to camera for given EVENT
 * @param soap              : SOAP Instance
 * @param user              : User Information
 * @param camIndex          : Camera Index
 * @param pEvtSubTermTime   : Event subscription termination time
 * @return
 */
static ONVIF_RESP_STATUS_e startOnvifEvent(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 camIndex, UINT32 *pEvtSubTermTime)
{
    ONVIF_RESP_STATUS_e 					onvifResp = ONVIF_CMD_FAIL;
    INT16 									soapResp;
    CREATE_PULL_PT_SUBSCRIPTION_t 			createPullPtSub;
    CREATE_PULL_PT_SUBSCRIPTION_RESPONSE_t 	createPullPtSubResp;
    SOAP_WSA5_t  							wsa5Info;
    UINT32									tmpLength;

    /* If GetEventService Capabilities is not received yet */
    if(strlen(onvifCamInfo[camIndex].eventServiceInfo.eventServiceUri) == 0)
    {
        EPRINT(ONVIF_CLIENT, "event service capabilities not received yet: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    /* Get the time difference between NVR and Camera */
    getCameraDateTime(soap, camIndex, user);
    onvifCamInfo[camIndex].eventServiceInfo.evSubscribeRef[0] = '\0';

    //Call CreatePullPointSubscription command to get event subscription reference address
    user->addr 				= onvifCamInfo[camIndex].eventServiceInfo.eventServiceUri;
    wsa5Info.addInfo 		= NULL;
    wsa5Info.wsa5__Action 	= CREATE_PULL_PT_ACTION;
    wsa5Info.wsa5_to 		= user->addr;

    //fill create pull point request
    createPullPtSub.Filter 					= NULL;
    createPullPtSub.InitialTerminationTime 	= PULL_PT_SUB_TERMINATION_TIME;
    createPullPtSub.SubscriptionPolicy 		= NULL;
    createPullPtSub.__any 					= NULL;
    createPullPtSub.__size 					= 0;

    do
    {
        //Create Pull Pt Subscription
        soapResp = CreatePullPtSubscription(soap, &createPullPtSub, &createPullPtSubResp, user, &wsa5Info);
        if(soapResp != SOAP_OK)
        {
            EPRINT(ONVIF_CLIENT, "fail to create pull point subscription: [camera=%d], [serviceUri=%s]", camIndex, user->addr);
            break;
        }

        if (createPullPtSubResp.SubscriptionReference.Address == NULL)
        {
            EPRINT(ONVIF_CLIENT, "pull point subscription reference not available: [camera=%d]", camIndex);
            break;
        }

        tmpLength = strlen(createPullPtSubResp.SubscriptionReference.Address);
        if ((tmpLength == 0) || (tmpLength >= MAX_SERVICE_ADDR_LEN))
        {
            EPRINT(ONVIF_CLIENT, "invld pull point subscription reference address length: [camera=%d], [length=%d]", camIndex, tmpLength);
            break;
        }

        DPRINT(ONVIF_CLIENT, "pull point subscription created: [camera=%d], [refAddr=%s]", camIndex, createPullPtSubResp.SubscriptionReference.Address);
        onvifResp = ONVIF_CMD_SUCCESS;

        //Save Event Subscription reference
        StringNCopy(onvifCamInfo[camIndex].eventServiceInfo.evSubscribeRef, createPullPtSubResp.SubscriptionReference.Address, MAX_SERVICE_ADDR_LEN);
        onvifCamInfo[camIndex].eventServiceInfo.additionalInfo[0] = '\0';

        /* Get event subscription expire time */
        *pEvtSubTermTime = createPullPtSubResp.wsnt__TerminationTime - createPullPtSubResp.wsnt__CurrentTime;

        if (createPullPtSubResp.SubscriptionReference.ReferenceParameters == NULL)
        {
            break;
        }

        if (createPullPtSubResp.SubscriptionReference.ReferenceParameters->__any == NULL)
        {
            break;
        }

        if (createPullPtSubResp.SubscriptionReference.ReferenceParameters->__any[0] == NULL)
        {
            break;
        }

        tmpLength = strlen(createPullPtSubResp.SubscriptionReference.ReferenceParameters->__any[0]);
        if (tmpLength == 0)
        {
            break;
        }

        //save additional information like, reference id in response
        if (tmpLength < MAX_EV_ADDITION_INFO_LEN)
        {
            StringNCopy(onvifCamInfo[camIndex].eventServiceInfo.additionalInfo,
                        createPullPtSubResp.SubscriptionReference.ReferenceParameters->__any[0], MAX_EV_ADDITION_INFO_LEN);
        }
        else
        {
            EPRINT(ONVIF_CLIENT, "insufficient buffer to store event additional info: [camera=%d], [length=%d]", camIndex, tmpLength);
        }
    }
    while(0);

    if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
    {
        onvifResp = ONVIF_CMD_TIMEOUT;
    }

    return onvifResp;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief renewEventSubscription    : renew subscription for event
 * @param soap                      : SOAP instance
 * @param user                      : User Index
 * @param camIndex                  : Camera Index
 * @param pEvtSubTermTime           : Event subscription termination time
 * @return
 */
static ONVIF_RESP_STATUS_e renewEventSubscription(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 camIndex, UINT32 *pEvtSubTermTime)
{
    SOAP_WSA5_t         wsa5Info;
    INT16               soapResp;
    RENEW_t             renew;
    RENEW_RESPONSE_t    renewResp;

    if (onvifCamInfo[camIndex].eventServiceInfo.evSubscribeRef[0] == '\0')
    {
        EPRINT(ONVIF_CLIENT, "event subscription address not received yet: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    fillWsa5HeaderInfo(camIndex, RENEW_ACTION, &wsa5Info);
    renew.TerminationTime = PULL_PT_SUB_TERMINATION_TIME;
    renew.__any 		  = NULL;
    renew.__size 		  = 0;

    /* Get the time difference between NVR and Camera */
    getCameraDateTime(soap, camIndex, user);
    user->addr = onvifCamInfo[camIndex].eventServiceInfo.evSubscribeRef;
    soapResp = Renew(soap, &renew, &renewResp, user, &wsa5Info);
    if (soapResp != SOAP_OK)
    {
        if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
        {
            return ONVIF_CMD_TIMEOUT;
        }

        return ONVIF_CMD_FAIL;
    }

    /* Get event subscription expire time */
    if (NULL == renewResp.CurrentTime)
    {
        /* Assuming NVR and camera time are in sync */
        *pEvtSubTermTime = renewResp.TerminationTime - time(NULL);
        WPRINT(ONVIF_CLIENT, "current time not rcvd in event subscription renew response: [camera=%d]", camIndex);
    }
    else
    {
        *pEvtSubTermTime = renewResp.TerminationTime - *renewResp.CurrentTime;
    }
    return ONVIF_CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief unSubscribeReq : unsubscribe Event
 * @param soap           : SOAP instance
 * @param user           : User Information
 * @param camIndex       : camera index
 * @return
 */
static ONVIF_RESP_STATUS_e unSubscribeReq(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 camIndex)
{
    SOAP_WSA5_t 			 wsa5Info;
    INT16					 soapResp;
    UNSUBSCRIBE_t 			 unSubscribe;
    UNSUBSCRIBE_RESPONSE_t   unSubscribeResp;

    if (onvifCamInfo[camIndex].eventServiceInfo.evSubscribeRef[0] == '\0')
    {
        EPRINT(ONVIF_CLIENT, "event service subscription not received yet: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    fillWsa5HeaderInfo(camIndex, UNSUBSCRIBE_ACTION, &wsa5Info);
    unSubscribe.__any 	= NULL;
    unSubscribe.__size 	= 0;

    /* Get the time difference between NVR and Camera */
    getCameraDateTime(soap, camIndex, user);

    user->addr = onvifCamInfo[camIndex].eventServiceInfo.evSubscribeRef;
    soapResp = UnSubscribe(soap, &unSubscribe, &unSubscribeResp, user, &wsa5Info);
    if (soapResp != SOAP_OK)
    {
        if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
        {
            return ONVIF_CMD_TIMEOUT;
        }

        return ONVIF_CMD_FAIL;
    }

    return ONVIF_CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief getOnvifEventStatus   : Gets current status of event also parse if current state
 * @param soap                  : SOAP instance
 * @param user                  : User information
 * @param camIndex              : camera index
 * @param evtStatus             : event status active or inactive
 * @return
 */
static ONVIF_RESP_STATUS_e getOnvifEventStatus(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 camIndex, BOOL *evtStatus)
{
    SOAP_WSA5_t 			 wsa5Info;
    INT16					 soapResp = SOAP_CLI_FAULT;
    PULL_MESSAGES_t 		 pullMessages;
    PULL_MESSAGES_RESPONSE_t pullMessagesResp;

    if (onvifCamInfo[camIndex].eventServiceInfo.evSubscribeRef[0] == '\0')
    {
        EPRINT(ONVIF_CLIENT, "event service subscription not received yet: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    fillWsa5HeaderInfo(camIndex, PULL_MESSAGES_ACTION, &wsa5Info);
    pullMessages.MessageLimit 	= PULL_MESSAGE_LIMIT;
    pullMessages.Timeout  		= PULL_MESSAGE_TIMEOUT;
    pullMessages.__any 			= NULL;
    pullMessages.__size			= 0;

    /* sync date and time with camera & Notify events available in message response. */
    getCameraDateTime(soap, camIndex, user);
    user->addr = onvifCamInfo[camIndex].eventServiceInfo.evSubscribeRef;
    soapResp = PullMessages(soap, &pullMessages, &pullMessagesResp, user, &wsa5Info);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "fail to pull event message: [camera=%d]", camIndex);
        if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
        {
            return ONVIF_CMD_TIMEOUT;
        }

        return ONVIF_CMD_FAIL;
    }

    notifyOnvifEvent(camIndex, &pullMessagesResp, evtStatus);
    return ONVIF_CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief notifyOnvifEvent  : It will parse events status to be active or inactive
 * @param camIndex          : Camera index
 * @param pullMessagesResp  : response received from camera
 * @param eventStatus       : active or inactive
 * @return
 */
static BOOL notifyOnvifEvent(UINT8 camIndex, PULL_MESSAGES_RESPONSE_t *pullMessagesResp, BOOL *eventStatus)
{
    UINT8 			msgCnt;
    INT32			tempVal;
    CHAR 			value[MAX_VALUE_STR];
    CAMERA_EVENT_e 	camEvent;
    BOOL		   	evtStatus;
    ONVIF_EVENT_e   onvifEvent;
    BOOL 			tagCompareStatus = FAIL;
    UINT8			tagCnt;
    SOAP_t          *soap;
    CHAR            *out;

    if ((pullMessagesResp->__sizeNotificationMessage <= 0) || (pullMessagesResp->wsnt__NotificationMessage == NULL))
    {
        return FAIL;
    }

    for (msgCnt = 0; msgCnt < pullMessagesResp->__sizeNotificationMessage; msgCnt++)
    {
        if (pullMessagesResp->wsnt__NotificationMessage[msgCnt].Topic == NULL)
        {
            EPRINT(ONVIF_CLIENT, "pull event message topic not available: [camera=%d], [msgCnt=%d]", camIndex, msgCnt);
            continue;
        }

        if (&pullMessagesResp->wsnt__NotificationMessage[msgCnt].Topic->__any == NULL)
        {
            EPRINT(ONVIF_CLIENT, "pull event message topic any not available: [camera=%d], [msgCnt=%d]", camIndex, msgCnt);
            continue;
        }

        /* Decide available tag belongs to which event's tag */
        soap = soap_new1(SOAP_C_UTFSTRING | SOAP_XML_INDENT | SOAP_DOM_TREE);
        out = NULL;

        soap->os = (const char**)&out;
        if((soap_free_temp(soap), soap_begin_send(soap)
            || (soap_serialize_xsd__anyType(soap,&pullMessagesResp->wsnt__NotificationMessage[msgCnt].Topic->__any), 0)
            || soap_put_xsd__anyType(soap, &pullMessagesResp->wsnt__NotificationMessage[msgCnt].Topic->__any, "xsd:anyType", "")
            || soap_end_send(soap) || (soap)->error))
        {
            /* Nothing to do */
        }

        /* make standard out null */
        soap->os = NULL;

        /* Free soap object memory */
        freeSoapObject(soap);

        /* Check if current camera event tag is supported or not */
        tagCompareStatus = FAIL;
        for(onvifEvent = ONVIF_MOTION_EVENT; onvifEvent < ONVIF_MAX_EVENT; onvifEvent++)
        {
            for (tagCnt = 0; tagCnt < onvifCamInfo[camIndex].eventServiceInfo.onvifEvRespDet[onvifEvent].tagAvailable; tagCnt++)
            {
                char *temp = (char*)pullMessagesResp->wsnt__NotificationMessage[msgCnt].Topic->__any.text;
                tagCompareStatus = tagSequenceCompare(temp, onvifCamInfo[camIndex].eventServiceInfo.onvifEvRespDet[onvifEvent].eventTag[tagCnt]);
                if (tagCompareStatus == SUCCESS)
                {
                    break;
                }
            }

            if (tagCompareStatus == SUCCESS)
            {
                break;
            }
        }

        /* If received tag is not same as already registerd tag */
        if (FAIL == tagCompareStatus)
        {
            EPRINT(ONVIF_CLIENT, "event tag not matched with supported tags: [camera=%d], [msgCnt=%d]", camIndex, msgCnt);
            continue;
        }

        //Retrieve Value
        soap = soap_new1(SOAP_C_UTFSTRING | SOAP_XML_INDENT | SOAP_DOM_TREE);
        out = NULL;

        soap->os = (const char**)&out;
        if ((soap_free_temp(soap), soap_begin_send(soap)
             || (soap_serialize_xsd__anyType(soap,&pullMessagesResp->wsnt__NotificationMessage[msgCnt].Message.__any), 0)
             || soap_put_xsd__anyType(soap, &pullMessagesResp->wsnt__NotificationMessage[msgCnt].Message.__any, "xsd:anyType", "")
             || soap_end_send(soap) || (soap)->error))
        {
            /* Nothing to do */
        }

        /* make standard out null */
        soap->os = NULL;

        /* Parse PropertyOperation field, if it is changed then only consider the notification. It value can be
         * 1.Intilized(at subscription)
         * 2.Changed(intermediate)
         * 3.Deleted(Unsubscribe)
         * reference : ONVIF-Core-Spefication-ver.19.12.pdf */
        getProperty(out, value);
        if (strcmp(value, "Changed") != 0)
        {
            /* If PropertyOpertion is other than 'Changed', avoid further parsing. */
            DPRINT(ONVIF_CLIENT, "event initialized: [camera=%d], [onvifEvent=%d], [msgCnt=%d]", camIndex, onvifEvent, msgCnt);

            /* Free soap object memory */
            freeSoapObject(soap);
            continue;
        }

        camEvent = MAX_CAMERA_EVENT;
        if (onvifEvent == ONVIF_SENSOR_EVENT)
        {
            /* If value contains '1', '2', '3' then it will detect it as SENSOR1,2,3 else it will be always considered as SENSOR1.
             * Decide Source Value */
            if(getValue(out, value, "Source") == SUCCESS)
            {
                tempVal = (value[0] - '1');

                if((tempVal >= 0) && (tempVal < 3))
                {
                    camEvent = (CAMERA_SENSOR_1 + tempVal);
                }
                else
                {
                    camEvent = CAMERA_SENSOR_1;
                }
            }
        }
        else if(onvifEvent == ONVIF_MOTION_EVENT)
        {
            camEvent = MOTION_DETECT;
        }
        else if(onvifEvent == ONVIF_TAMPER_EVENT)
        {
            camEvent = VIEW_TEMPERING;
        }
        else if(onvifEvent == ONVIF_LINE_CROSS_EVENT)
        {
            camEvent = LINE_CROSS;
        }
        else if(onvifEvent == ONVIF_OBJECT_INSIDE_EVENT)
        {
            camEvent = OBJECT_INTRUSION;
        }
        else if(onvifEvent == ONVIF_AUDIO_EXCEPTION_EVENT)
        {
            camEvent = AUDIO_EXCEPTION;
        }
        else if(onvifEvent == ONVIF_LOTERING_EVENT)
        {
            camEvent = LOITERING;
        }
        else if(onvifEvent == ONVIF_OBJECT_CNT_EVENT)
        {
            camEvent = OBJECT_COUNTING;
        }

        /* Nothing to do if we don't support event */
        if (camEvent >= MAX_CAMERA_EVENT)
        {
            /* Free soap object memory */
            freeSoapObject(soap);
            continue;
        }

        /* By default state will be inactive. So if any event is not parsed properly it will be in INACTIVE state. */
        evtStatus = INACTIVE;
        switch(camEvent)
        {
            case MOTION_DETECT:
            case VIEW_TEMPERING:
            case CAMERA_SENSOR_1:
            case CAMERA_SENSOR_2:
            case CAMERA_SENSOR_3:
            {
                /* Following events has data field in the notification message. That can have two possivle values true/false.
                 * reference: ONVIF-Analytics-Service-Spec.pdf(Ver.19.12)
                 * Eg.
                 * <tt:Data>
                 *      <tt:SimpleItem Name="IsXYZ" Value="false"></tt:SimpleItem>
                 * </tt:Data> */
                if(getValue(out, value, "Data") == SUCCESS)
                {
                    if(strcmp(value,"true") == STATUS_OK)
                    {
                        evtStatus = ACTIVE;
                    }
                    else
                    {
                        evtStatus = INACTIVE;
                    }
                }
            }
            break;

            case OBJECT_INTRUSION:
            {
                /* reference: ONVIF-Analytics-Service-Spec.pdf(Ver.19.12)
                 * Eg.
                 * <tt:Key>
                 *      <tt:SimpleItem Name="ObjectId" Type="xs:integer"></tt:SimpleItem>
                 * </tt:Key>
                 * <tt:Data>
                 *      <tt:SimpleItem Name="IsInside" Type="xs:boolean"></tt:SimpleItem>
                 * </tt:Data>
                 *
                 * NOTE:
                 * Currently Matrix Camera doesn't have KEY and DATA field in message. If we consider 'InInside' property for state of event
                 * then our action will misbehave. As discussion with Camera Team,  'IsInside=True' when any object is intruded in the zone.
                 * It does not have either case such as false. So we will have only TRUE in the value. We cannot use event as ACTIVE/NORMAL
                 * logic. To overcome this Debounce Timer is configured against the event so it will get NORMAL on the basis of timer.*/
                #if 0
                if(getValue(out,value,"Key") == SUCCESS)
                {
                    // parse Object ID
                }

                if(getValue(out,value,"Data") == SUCCESS)
                {
                    // parse 'InInside'
                }
                #endif
                evtStatus = ACTIVE;
            }
            break;

            case LINE_CROSS:
            {
                /* NOTE: Currently, Matrix camera don't have DATA field in message. Data field contains OBJECT-ID.
                 * Eg.
                 * <tt:Data>
                 *      <tt:SimpleItem Name="ObjectId" Type="xs:integer"></tt:SimpleItem>
                 * </tt:Data>
                 * Event will be active when we receive notification, as no state is mentioned in the message.
                 * It will get normal after debounce timer. */
                #if 0
                if(getValue(out, value, "Data") == SUCCESS){}
                #endif
                evtStatus = ACTIVE;
            }
            break;

            case AUDIO_EXCEPTION:
            {
                /* As per ONVIF format, DATA should have two fields
                 * 1.IsSoundDetected
                 * 2.UTCTime
                 * We have not parsed UTC time here.
                 * reference: ONVIF-Analytics-Service-Spec.pdf(Ver.19.12)
                 * Eg.
                 * <tt:Data>
                 *      <tt:SimpleItem Name="IsSoundDetected" Type="xs:boolean"></tt:SimpleItem>
                 *      <tt:SimpleItem Name="UTCTime" Type="xs:dateTime"></tt:SimpleItem>
                 * </tt:Data> */
                if(getValue(out, value, "Data") == SUCCESS)
                {
                    if(strcmp(value,"true") == STATUS_OK)
                    {
                        evtStatus = ACTIVE;
                    }
                    else if(strcmp(value,"false") == STATUS_OK)
                    {
                        evtStatus = INACTIVE;
                    }
                }
            }
            break;

            case LOITERING:
            {
                /* NOTE: We need to decide for parsing of following parameters.
                 * Eg.
                 * <tt:Key>
                 *      <tt:SimpleItem Name="ObjectId" Type="xs:integer"></tt:SimpleItem>
                 * </tt:Key>
                 * <tt:Data>
                 *      <tt:SimpleItem Name="Since" Type="xs:dateTime"></tt:SimpleItem>
                 *      <tt:SimpleItem Name="TimeStamp" Type="xs:dateTime"></tt:SimpleItem>
                 * </tt:Data> */
                #if 0
                if(getValue(out,value,"Key") == SUCCESS)
                {
                    // parse Object ID
                }

                if(getValue(out,value,"Data") == SUCCESS)
                {
                    // parse 'Since'
                }

                if(getValue(out,value,"Data") == SUCCESS)
                {
                    // parse 'TimeStamp'
                }
                #endif

                /* For now, event will be active when we receive notification. */
                evtStatus = ACTIVE;
            }
            break;

            case OBJECT_COUNTING:
            {
                /* NOTE: ObjectId and Count are not used further. So, parsing is commented!
                 * reference: ONVIF-Analytics-Service-Spec.pdf(Ver.19.12)
                 * Eg.
                 * <tt:Key>
                 *      <tt:SimpleItem Name="ObjectId" Type="xs:integer"></tt:SimpleItem>
                 * </tt:Key>
                 * <tt:Data>
                 *      <tt:SimpleItem Name="Count" Type="xs:nonNegativeInteger"></tt:SimpleItem>
                 * </tt:Data> */
                #if 0
                if(getValue(out,value,"Key") == SUCCESS)
                {
                    // parse Object ID
                }

                if(getValue(out,value,"Data") == SUCCESS)
                {
                    // parse 'Count'
                }
                #endif

                /* For now, event will be active when we receive notification. */
                evtStatus = ACTIVE;
            }
            break;

            default:
            {
                /* Nothing to do */
            }
            break;
        }

        /* Free soap object memory */
        freeSoapObject(soap);

        eventStatus[camEvent] = evtStatus;
        DPRINT(ONVIF_CLIENT, "notify onvif event: [camera=%d], [onvifEvent=%s], [cameraEvent=%d], [onvifEvtStatus=%s], [cameraEventStatus=%s]",
               camIndex, onvifEvtStr[onvifEvent], camEvent, value, evtStatus ? "ACTIVE" : "INACTIVE");
    }

    return tagCompareStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief fillWsa5HeaderInfo    : Sets Wsa5 header information
 * @param camIndex              : camera Index
 * @param action                : action type
 * @param wsa5Info              : info
 */
static void fillWsa5HeaderInfo(UINT8 camIndex, CHARPTR action, SOAP_WSA5_t *wsa5Info)
{
    if(onvifCamInfo[camIndex].eventServiceInfo.additionalInfo[0] != '\0')
    {
        wsa5Info->addInfo = onvifCamInfo[camIndex].eventServiceInfo.additionalInfo;
    }
    else
    {
        wsa5Info->addInfo = NULL;
    }
    wsa5Info->wsa5__Action = action;
    wsa5Info->wsa5_to = onvifCamInfo[camIndex].eventServiceInfo.evSubscribeRef;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief onvifSetOsdReq    : Sets OSD configuration to camera
 * @param soapPtr           : SOAP instance
 * @param userDeatil        : User information
 * @param camIndex          : camera Index
 * @param pOsdParam         : OSD parameter configuration
 * @return
 */
static ONVIF_RESP_STATUS_e onvifSetOsdReq(SOAP_t *soap, SOAP_USER_DETAIL_t *userDeatil, UINT8 camIndex, OSD_PARAM_t *pOsdParam)
{
    struct tt__OSDConfiguration     osdConfiguration;
    struct tt__OSDPosConfiguration  position;	/* required element of type tt:OSDPosConfiguration */
    struct tt__OSDTextConfiguration textString;
    struct tt__OSDReference         osdReference;
    GET_OSDS_t                      getOsds;
    GET_OSDSRESPONSE_t              getOsdsRes;
    SET_OSD_t                       setOsd;
    SET_OSDRESPONSE_t               setOsdResponse;
    CREATE_OSD_t                    createOsd;
    CREATE_OSDRESPONSE_t            createOsdRes;
    DELETE_OSD_t                    deleteOsd;
    DELETE_OSDRESPONSE_t            deleteOsdResponse;
    CHAR                            plainText[] = "Plain";
    CHAR                            dateAndTimeStr[] = "DateAndTime";
    CHAR                            dateTimeOsdToken[OSD_TOKEN_MAX][MAX_TOKEN_SIZE];
    CHAR                            textOsdToken[OSD_TOKEN_MAX][MAX_TOKEN_SIZE];
    UINT8                           dateTimeTokenCnt = 0;
    UINT8                           textTokenCnt = 0;
    GENERAL_CONFIG_t                generalConfig;
    INT16                           soapResp;
    UINT8                           index;
    CHAR                            osdVideoSrcToken[MAX_TOKEN_SIZE];

    /* Get the time difference between NVR and Camera */
    getCameraDateTime(soap, camIndex, userDeatil);
    userDeatil->addr = onvifCamInfo[camIndex].mediaServiceInfo.mediaServiceAddr;
    getOsds.ConfigurationToken = NULL; /* Fill null profile video source config token to get all configured OSDs */
    soapResp = Getosds(soap, &getOsds, &getOsdsRes, userDeatil);
    if(soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "fail to get osd: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        return ONVIF_CMD_FAIL;
    }

    /* Reset tokens string */
    memset(dateTimeOsdToken, '\0', sizeof(dateTimeOsdToken));
    memset(textOsdToken, '\0', sizeof(textOsdToken));
    memset(osdVideoSrcToken, '\0', sizeof(osdVideoSrcToken));

    if (getOsdsRes.OSDs != NULL)
    {
        // fill datetime and text osd token if present
        for (index = 0; index < getOsdsRes.__sizeOSDs; index++)
        {
            if (((getOsdsRes.OSDs + index)->token == NULL) || ((getOsdsRes.OSDs + index)->Type != tt__OSDType__Text)
                    || ((getOsdsRes.OSDs + index)->TextString == NULL) || ((getOsdsRes.OSDs + index)->TextString->Type == NULL))
            {
                continue;
            }

            if (strcmp((getOsdsRes.OSDs + index)->TextString->Type, dateAndTimeStr) == 0)
            {
                if (dateTimeTokenCnt >= OSD_TOKEN_MAX)
                {
                    EPRINT(ONVIF_CLIENT, "more osd tokens available: [camera=%d]", camIndex);
                    continue;
                }

                StringNCopy(dateTimeOsdToken[dateTimeTokenCnt], (getOsdsRes.OSDs + index)->token, MAX_TOKEN_SIZE);
                dateTimeTokenCnt++;
            }
            else if (strcmp((getOsdsRes.OSDs + index)->TextString->Type, plainText) == 0)
            {
                if (textTokenCnt >= OSD_TOKEN_MAX)
                {
                    EPRINT(ONVIF_CLIENT, "more text tokens available: [camera=%d]", camIndex);
                    continue;
                }

                StringNCopy(textOsdToken[textTokenCnt], (getOsdsRes.OSDs + index)->token, MAX_TOKEN_SIZE);
                textTokenCnt++;
            }
        }

        if ((getOsdsRes.OSDs->VideoSourceConfigurationToken != NULL) && (getOsdsRes.OSDs->VideoSourceConfigurationToken->__item != NULL))
        {
            StringNCopy(osdVideoSrcToken, getOsdsRes.OSDs->VideoSourceConfigurationToken->__item, MAX_TOKEN_SIZE);
        }
        else
        {
            for (index = 0; index < MAX_PROFILE_SUPPORT; index++)
            {
                if (onvifCamInfo[camIndex].mediaServiceInfo.mediaProfile[index].videoSourceToken[0] == '\0')
                {
                    continue;
                }

                StringNCopy(osdVideoSrcToken, onvifCamInfo[camIndex].mediaServiceInfo.mediaProfile[index].videoSourceToken, MAX_TOKEN_SIZE);
                break;
            }
        }
    }
    else
    {
        EPRINT(ONVIF_CLIENT, "osds not available: [camera=%d]", camIndex);
    }

    /* Read current configuration */
    ReadGeneralConfig(&generalConfig);
    soap_default_tt__OSDTextConfiguration(NULL, &textString);
    soap_default_tt__OSDPosConfiguration(NULL, &position);

    soap_default_tt__OSDReference(NULL, &osdReference);
    osdReference.__item	= osdVideoSrcToken;

    osdConfiguration.token = NULL;
    osdConfiguration.Image = NULL;
    osdConfiguration.Extension = NULL;
    osdConfiguration.Position = &position;
    osdConfiguration.TextString = &textString;
    osdConfiguration.Type = tt__OSDType__Text;
    osdConfiguration.VideoSourceConfigurationToken = &osdReference;
    soap_default_xsd__anyAttribute(NULL, &osdConfiguration.__anyAttribute);

    createOsd.OSD = &osdConfiguration;
    createOsd.__any = NULL;
    createOsd.__size = 0;

    setOsd.OSD = &osdConfiguration;
    setOsd.__any = NULL;
    setOsd.__size = 0;

    deleteOsd.__any = NULL;
    deleteOsd.__size = 0;

    /* Set date & time related parameters on camera */
    textString.Type = dateAndTimeStr;
    textString.PlainText = NULL;
    textString.DateFormat = dateFormatStrForOsd[generalConfig.dateFormat];
    textString.TimeFormat = timeFormatStrForOsd[generalConfig.timeFormat];
    position.Type = osdPosFormatStr[pOsdParam->dateTimePos];
    if (pOsdParam->dateTimeOverlay == TRUE)
    {
        /* if datetime token is not preset then create it */
        if(dateTimeTokenCnt == 0)
        {
            /* Create datetime osd */
            osdConfiguration.token = NULL;
            soapResp = CreateOsd(soap, &createOsd, &createOsdRes, userDeatil);
        }
        else
        {
            /* Set datetime osd with current configuration */
            osdConfiguration.token = dateTimeOsdToken[0];
            soapResp = SetOsd(soap, &setOsd, &setOsdResponse, userDeatil);
        }
    }
    else
    {
        /* delete all datetime related osds */
        for(index = 0; index < dateTimeTokenCnt; index++)
        {
            /* if datetime token is preset then delete it */
            if (dateTimeOsdToken[index][0] != '\0')
            {
                /* Delete datetime osd */
                deleteOsd.OSDToken = dateTimeOsdToken[index];
                soapResp = DeleteOsd(soap, &deleteOsd, &deleteOsdResponse, userDeatil);
            }
        }
    }

    /* Apply text overlay settings only if text overlay param changed */
    if (pOsdParam->textoverlayChanged == TRUE)
    {
        /* Set plain text related parameters on camera */
        textString.Type = plainText;
        textString.PlainText = pOsdParam->channelName[0];
        textString.DateFormat = NULL;
        textString.TimeFormat = NULL;
        position.Type = osdPosFormatStr[pOsdParam->channelNamePos[0]];
        if(pOsdParam->channelNameOverlay == TRUE)
        {
            /* if text token is not preset then create it */
            if(textTokenCnt == 0)
            {
                /* Create text osd */
                osdConfiguration.token = NULL;
                soapResp = CreateOsd(soap, &createOsd, &createOsdRes, userDeatil);
            }
            else
            {
                /* Set text osd with current configuration */
                osdConfiguration.token = textOsdToken[0];
                soapResp = SetOsd(soap, &setOsd, &setOsdResponse, userDeatil);
            }
        }
        else
        {
            /* delete all text related osds */
            for(index = 0; index < textTokenCnt; index++)
            {
                /* if text token is preset then delete it */
                if(textOsdToken[index][0] != '\0')
                {
                    /* Delete text osd */
                    deleteOsd.OSDToken = textOsdToken[index];
                    soapResp = DeleteOsd(soap, &deleteOsd, &deleteOsdResponse, userDeatil);
                }
            }
        }
    }

    return ONVIF_CMD_SUCCESS;
}

#if 0
//-------------------------------------------------------------------------------------------------
/**
 * @brief onvifSetOsdReq2   : Sets OSD configuration to camera using Media2
 * @param soapPtr           : SOAP instance
 * @param userDeatil        : User information
 * @param camIndex          : camera Index
 * @return
 */
static ONVIF_RESP_STATUS_e onvifSetOsdReq2(SOAP_t *soap, SOAP_USER_DETAIL_t *userDeatil, UINT8 camIndex)
{
    struct tt__OSDConfiguration     osdConfiguration;
    struct tt__OSDPosConfiguration  position;	/* required element of type tt:OSDPosConfiguration */
    struct tt__OSDTextConfiguration textString;
    struct tt__OSDReference         osdReference;
    GET_OSDS2_t                     getOsds;
    GET_OSDSRESPONSE2_t             getOsdsRes;
    SET_OSD2_t                      setOsd;
    SET_OSDRESPONSE2_t              setOsdResponse;
    CREATE_OSD2_t                   createOsd;
    CREATE_OSDRESPONSE2_t           createOsdRes;
    DELETE_OSD2_t                   deleteOsd;
    DELETE_OSDRESPONSE2_t           deleteOsdResponse;
    CHAR                            plainText[] = "Plain";
    CHAR                            dateAndTimeStr[] = "DateAndTime";
    CHAR                            dateTimeOsdToken[OSD_TOKEN_MAX][MAX_TOKEN_SIZE];
    CHAR                            textOsdToken[OSD_TOKEN_MAX][MAX_TOKEN_SIZE];
    UINT8                           dateTimeTokenCnt = 0;
    UINT8                           textTokenCnt = 0;
    CAMERA_CONFIG_t                 cameraConfig;
    GENERAL_CONFIG_t                generalConfig;
    INT16                           soapResp;
    UINT8                           index;
    CHAR                            osdVideoSrcToken[MAX_TOKEN_SIZE];

    /* Get the time difference between NVR and Camera */
    getCameraDateTime(soap, camIndex, userDeatil);
    userDeatil->addr = onvifCamInfo[camIndex].mediaServiceInfo.mediaServiceAddr;
    getOsds.ConfigurationToken = NULL; /* Fill null profile video source config token to get all configured OSDs */
    getOsds.OSDToken           = NULL;
    soapResp = Getosds2(soap, &getOsds, &getOsdsRes, userDeatil);
    if(soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "fail to get osd: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        return ONVIF_CMD_FAIL;
    }

    /* Reset tokens string */
    memset(dateTimeOsdToken, '\0', sizeof(dateTimeOsdToken));
    memset(textOsdToken, '\0', sizeof(textOsdToken));
    memset(osdVideoSrcToken, '\0', sizeof(osdVideoSrcToken));

    if (getOsdsRes.OSDs != NULL)
    {
        // fill datetime and text osd token if present
        for (index = 0; index < getOsdsRes.__sizeOSDs; index++)
        {
            if (((getOsdsRes.OSDs + index)->token == NULL) || ((getOsdsRes.OSDs + index)->Type != tt__OSDType__Text)
                    || ((getOsdsRes.OSDs + index)->TextString == NULL) || ((getOsdsRes.OSDs + index)->TextString->Type == NULL))
            {
                continue;
            }

            if (strcmp((getOsdsRes.OSDs + index)->TextString->Type, dateAndTimeStr) == 0)
            {
                if (dateTimeTokenCnt >= OSD_TOKEN_MAX)
                {
                    EPRINT(ONVIF_CLIENT, "more osd tokens available: [camera=%d]", camIndex);
                    continue;
                }

                StringNCopy(dateTimeOsdToken[dateTimeTokenCnt], (getOsdsRes.OSDs + index)->token, MAX_TOKEN_SIZE);
                dateTimeTokenCnt++;
            }
            else if (strcmp((getOsdsRes.OSDs + index)->TextString->Type, plainText) == 0)
            {
                if (textTokenCnt >= OSD_TOKEN_MAX)
                {
                    EPRINT(ONVIF_CLIENT, "more text tokens available: [camera=%d]", camIndex);
                    continue;
                }

                StringNCopy(textOsdToken[textTokenCnt], (getOsdsRes.OSDs + index)->token, MAX_TOKEN_SIZE);
                textTokenCnt++;
            }
        }

        if ((getOsdsRes.OSDs->VideoSourceConfigurationToken != NULL) && (getOsdsRes.OSDs->VideoSourceConfigurationToken->__item != NULL))
        {
            StringNCopy(osdVideoSrcToken, getOsdsRes.OSDs->VideoSourceConfigurationToken->__item, MAX_TOKEN_SIZE);
        }
        else
        {
            for (index = 0; index < MAX_PROFILE_SUPPORT; index++)
            {
                if (onvifCamInfo[camIndex].mediaServiceInfo.mediaProfile[index].videoSourceToken[0] == '\0')
                {
                    continue;
                }

                StringNCopy(osdVideoSrcToken, onvifCamInfo[camIndex].mediaServiceInfo.mediaProfile[index].videoSourceToken, MAX_TOKEN_SIZE);
                break;
            }
        }
    }
    else
    {
        EPRINT(ONVIF_CLIENT, "osds not available: [camera=%d]", camIndex);
    }

    /* Read current configuration */
    ReadGeneralConfig(&generalConfig);
    ReadSingleCameraConfig(camIndex, &cameraConfig);

    soap_default_tt__OSDTextConfiguration(NULL, &textString);
    soap_default_tt__OSDPosConfiguration(NULL, &position);

    soap_default_tt__OSDReference(NULL, &osdReference);
    osdReference.__item	= osdVideoSrcToken;

    osdConfiguration.token = NULL;
    osdConfiguration.Image = NULL;
    osdConfiguration.Extension = NULL;
    osdConfiguration.Position = &position;
    osdConfiguration.TextString = &textString;
    osdConfiguration.Type = tt__OSDType__Text;
    osdConfiguration.VideoSourceConfigurationToken = &osdReference;
    soap_default_xsd__anyAttribute(NULL, &osdConfiguration.__anyAttribute);

    createOsd.OSD = &osdConfiguration;
    setOsd.OSD = &osdConfiguration;

    /* Set date & time related parameters on camera */
    textString.Type = dateAndTimeStr;
    textString.PlainText = NULL;
    textString.DateFormat = dateFormatStrForOsd[generalConfig.dateFormat];
    textString.TimeFormat = timeFormatStrForOsd[generalConfig.timeFormat];
    position.Type = osdPosFormatStr[cameraConfig.dateTimePos];
    if (cameraConfig.dateTimeOverlay == TRUE)
    {
        /* if datetime token is not preset then create it */
        if(dateTimeTokenCnt == 0)
        {
            /* Create datetime osd */
            osdConfiguration.token = NULL;
            soapResp = CreateOsd2(soap, &createOsd, &createOsdRes, userDeatil);
        }
        else
        {
            /* Set datetime osd with current configuration */
            osdConfiguration.token = dateTimeOsdToken[0];
            soapResp = SetOsd2(soap, &setOsd, &setOsdResponse, userDeatil);
        }
    }
    else
    {
        /* delete all datetime related osds */
        for(index = 0; index < dateTimeTokenCnt; index++)
        {
            /* if datetime token is preset then delete it */
            if (dateTimeOsdToken[index][0] != '\0')
            {
                /* Delete datetime osd */
                deleteOsd.OSDToken = dateTimeOsdToken[index];
                soapResp = DeleteOsd2(soap, &deleteOsd, &deleteOsdResponse, userDeatil);
            }
        }
    }

    /* Set plain text related parameters on camera */
    textString.Type = plainText;
    textString.PlainText = cameraConfig.channelName;
    textString.DateFormat = NULL;
    textString.TimeFormat = NULL;
    position.Type = osdPosFormatStr[cameraConfig.channelNamePos];
    if(cameraConfig.channelNameOverlay == TRUE)
    {
        /* if text token is not preset then create it */
        if(textTokenCnt == 0)
        {
            /* Create text osd */
            osdConfiguration.token = NULL;
            soapResp = CreateOsd2(soap, &createOsd, &createOsdRes, userDeatil);
        }
        else
        {
            /* Set text osd with current configuration */
            osdConfiguration.token = textOsdToken[0];
            soapResp = SetOsd2(soap, &setOsd, &setOsdResponse, userDeatil);
        }
    }
    else
    {
        /* delete all text related osds */
        for(index = 0; index < textTokenCnt; index++)
        {
            /* if text token is preset then delete it */
            if(textOsdToken[index][0] != '\0')
            {
                /* Delete text osd */
                deleteOsd.OSDToken = textOsdToken[index];
                soapResp = DeleteOsd2(soap, &deleteOsd, &deleteOsdResponse, userDeatil);
            }
        }
    }

    return ONVIF_CMD_SUCCESS;
}
#endif

//-------------------------------------------------------------------------------------------------
/**
 * @brief getOnvifProfileParam  : gets profile stream paramters
 * @param soap                  : SOAP instance
 * @param user                  : User information
 * @param profileNum            : profile index
 * @param camIndex              : camera index
 * @param profStreamInfo        : stream configuration
 * @return
 */
static ONVIF_RESP_STATUS_e getOnvifProfileParam(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 profileNum,
                                                UINT8 camIndex, ONVIF_PROFILE_STREAM_INFO_t *profStreamInfo)
{
    GET_VIDEO_ENCODER_CONFIGURATION_t			getVideoEncoderConfiguration;
    GET_VIDEO_ENCODER_CONFIGURATION_RESPONSE_t 	getVideoEncoderConfigurationResponse;
    INT16  										soapResp;
    RESOLUTION_e 								resolutionNum;
    CHARPTR                                     codecStr[] = {"Motion JPEG", "MPEG4", "H264"};
    UINT8                                       profileIndex;

    /* Validate profile number */
    if ((profileNum == 0) || (profileNum > MAX_PROFILE_SUPPORT))
    {
        EPRINT(ONVIF_CLIENT, "invld profile number: [camera=%d], [profile=%d]", camIndex, profileNum);
        return ONVIF_CMD_FAIL;
    }

    profileIndex = profileNum - 1;
    profStreamInfo->profileNum = profileNum;
    profStreamInfo->enableAudio = FALSE;

    /* Get the time difference between NVR and Camera */
    getCameraDateTime(soap, camIndex, user);
    user->addr = onvifCamInfo[camIndex].mediaServiceInfo.mediaServiceAddr;
    getVideoEncoderConfiguration.ConfigurationToken = onvifCamInfo[camIndex].mediaServiceInfo.mediaProfile[profileIndex].videoEncoderConfigurationToken;
    soapResp = GetVideoEncoderConfiguration(soap, &getVideoEncoderConfiguration, &getVideoEncoderConfigurationResponse, user);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "fail to get video encoder config: [camera=%d], [profile=%d], [soapResp=%d]", camIndex, profileIndex, soapResp);
        return ONVIF_CMD_FAIL;
    }

    if (getVideoEncoderConfigurationResponse.Configuration == NULL)
    {
        EPRINT(ONVIF_CLIENT, "video encoder config not available: [camera=%d], [profile=%d]", camIndex, profileIndex);
        return ONVIF_CMD_FAIL;
    }

    if (getVideoEncoderConfigurationResponse.Configuration->Encoding < 3)
    {
        StringNCopy(profStreamInfo->videoEncoding, codecStr[getVideoEncoderConfigurationResponse.Configuration->Encoding], MAX_TOKEN_SIZE);
    }
    else
    {
        /* Set video encoding null */
        profStreamInfo->videoEncoding[0] = '\0';
        EPRINT(ONVIF_CLIENT, "supported video encoding not available: [camera=%d], [profile=%d], [encoding=%d]",
               camIndex, profileIndex, getVideoEncoderConfigurationResponse.Configuration->Encoding);
    }

    if (getVideoEncoderConfigurationResponse.Configuration->Resolution != NULL)
    {
        if (GetResolutionId(getVideoEncoderConfigurationResponse.Configuration->Resolution->Width,
                            getVideoEncoderConfigurationResponse.Configuration->Resolution->Height, &resolutionNum) == SUCCESS)
        {
            GetResolutionString(profStreamInfo->resolution, resolutionNum);
        }
        else
        {
            ConvertCameraResolutionToString(profStreamInfo->resolution, getVideoEncoderConfigurationResponse.Configuration->Resolution->Width,
                                            getVideoEncoderConfigurationResponse.Configuration->Resolution->Height);
        }
    }
    else
    {
        /* Set video resolution null */
        profStreamInfo->resolution[0] = '\0';
        EPRINT(ONVIF_CLIENT, "video resolution not available: [camera=%d], [profile=%d]", camIndex, profileIndex);
    }

    if (onvifCamInfo[camIndex].mediaServiceInfo.initialQuality == 1)
    {
        profStreamInfo->quality = ((UINT8)getVideoEncoderConfigurationResponse.Configuration->Quality);
    }
    else
    {
        profStreamInfo->quality = ((UINT8)getVideoEncoderConfigurationResponse.Configuration->Quality - onvifCamInfo[camIndex].mediaServiceInfo.initialQuality + 1);
    }

    /* We can't decide bitratemode by onvif. so fill it default value */
    profStreamInfo->bitrateMode = VBR;

    if (getVideoEncoderConfigurationResponse.Configuration->RateControl != NULL)
    {
        profStreamInfo->framerate = (UINT8)getVideoEncoderConfigurationResponse.Configuration->RateControl->FrameRateLimit;
        profStreamInfo->bitrateValue = getBitrateIndex(getVideoEncoderConfigurationResponse.Configuration->RateControl->BitrateLimit);
    }
    else
    {
        profStreamInfo->framerate = DFLT_PAL_FRAME_RATE;
        profStreamInfo->bitrateValue = BITRATE_1024;
        EPRINT(ONVIF_CLIENT, "fps and bitrate not available: [camera=%d], [profile=%d]", camIndex, profileIndex);
    }

    if (getVideoEncoderConfigurationResponse.Configuration->Encoding == tt__VideoEncoding__MPEG4)
    {
        if (getVideoEncoderConfigurationResponse.Configuration->MPEG4 != NULL)
        {
            profStreamInfo->gop = (UINT8)getVideoEncoderConfigurationResponse.Configuration->MPEG4->GovLength;
        }
        else
        {
            profStreamInfo->gop = DFLT_CAM_GOP;
            EPRINT(ONVIF_CLIENT, "gop not available for mpeg4: [camera=%d], [profile=%d]", camIndex, profileIndex);
        }
    }
    else if (getVideoEncoderConfigurationResponse.Configuration->Encoding == tt__VideoEncoding__H264)
    {
        if (getVideoEncoderConfigurationResponse.Configuration->H264 != NULL)
        {
            profStreamInfo->gop = (UINT8)getVideoEncoderConfigurationResponse.Configuration->H264->GovLength;
        }
        else
        {
            profStreamInfo->gop = DFLT_CAM_GOP;
            EPRINT(ONVIF_CLIENT, "gop not available for h264: [camera=%d], [profile=%d]", camIndex, profileIndex);
        }
    }
    else
    {
        profStreamInfo->gop = DFLT_CAM_GOP;
    }

//    DPRINT(ONVIF_CLIENT, "profile stream info: [camera=%d], [profile=%d], [encoding=%s], [bitrate=%d], [fps=%d], [bitrateMode=%d], [gop=%d], [resolution=%s], [quality=%d]",
//           camIndex, profileIndex, profStreamInfo->videoEncoding, profStreamInfo->bitrateValue, profStreamInfo->framerate,
//           profStreamInfo->bitrateMode, profStreamInfo->gop, profStreamInfo->resolution, profStreamInfo->quality);
    return ONVIF_CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief getOnvifMedia2ProfileParam    : Get profile param for given profile number
 * @param soap                          : SOAP instance
 * @param user                          : User inforamtion
 * @param profileNum                    : profile index
 * @param camIndex                      : camera index
 * @param profStreamInfo                : Stream information
 * @return
 */
static ONVIF_RESP_STATUS_e getOnvifMedia2ProfileParam(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 profileNum,
                                                      UINT8 camIndex, ONVIF_PROFILE_STREAM_INFO_t *profStreamInfo)
{
    GET_VIDEO_ENCODER_CONFIGURATION2_t              getVideoEncoderConfiguration;
    GET_VIDEO_ENCODER_CONFIGURATION_RESPONSE2_t 	getVideoEncoderConfigurationResponse;
    INT16                                           soapResp;
    RESOLUTION_e                                    resolutionNum;
    UINT8                                           profileIndex;

    /* Validate profile number */
    if ((profileNum == 0) || (profileNum > MAX_PROFILE_SUPPORT))
    {
        EPRINT(ONVIF_CLIENT, "invld profile number: [camera=%d], [profile=%d]", camIndex, profileNum);
        return ONVIF_CMD_FAIL;
    }

    profileIndex = profileNum - 1;
    profStreamInfo->profileNum = profileNum;
    profStreamInfo->enableAudio = FALSE;

    /* Get the time difference between NVR and Camera */
    getCameraDateTime(soap, camIndex, user);
    user->addr                                      = onvifCamInfo[camIndex].mediaServiceInfo.mediaServiceAddr;
    getVideoEncoderConfiguration.ProfileToken       = onvifCamInfo[camIndex].mediaServiceInfo.mediaProfile[profileIndex].profileToken;
    getVideoEncoderConfiguration.ConfigurationToken = onvifCamInfo[camIndex].mediaServiceInfo.mediaProfile[profileIndex].videoEncoderConfigurationToken;
    soapResp = GetVideoEncoderConfiguration2(soap, &getVideoEncoderConfiguration, &getVideoEncoderConfigurationResponse, user);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "fail to get video encoder config: [camera=%d], [profile=%d], [soapResp=%d]", camIndex, profileIndex, soapResp);
        return ONVIF_CMD_FAIL;
    }

    if ((getVideoEncoderConfigurationResponse.__sizeConfigurations < 1) || (getVideoEncoderConfigurationResponse.Configurations == NULL))
    {
        EPRINT(ONVIF_CLIENT, "video encoder config not available: [camera=%d], [profile=%d]", camIndex, profileIndex);
        return ONVIF_CMD_FAIL;
    }

    if (getVideoEncoderConfigurationResponse.Configurations[0].Encoding != NULL)
    {
        /* For JPEG, store name as Motion JPEG */
        if(strcmp(getVideoEncoderConfigurationResponse.Configurations[0].Encoding, "JPEG")== 0)
        {
            StringNCopy(profStreamInfo->videoEncoding, "Motion JPEG", MAX_TOKEN_SIZE);
        }
        else if(strcmp(getVideoEncoderConfigurationResponse.Configurations[0].Encoding, "MPV4-ES")== 0)
        {
            StringNCopy(profStreamInfo->videoEncoding, "MPEG4", MAX_TOKEN_SIZE);
        }
        else
        {
            StringNCopy(profStreamInfo->videoEncoding, getVideoEncoderConfigurationResponse.Configurations[0].Encoding, MAX_TOKEN_SIZE);
        }
    }
    else
    {
        /* Set video encoding null */
        profStreamInfo->videoEncoding[0] = '\0';
        EPRINT(ONVIF_CLIENT, "video encoding not available: [camera=%d], [profile=%d]", camIndex, profileIndex);
    }

    if (getVideoEncoderConfigurationResponse.Configurations[0].Resolution != NULL)
    {
        if (GetResolutionId(getVideoEncoderConfigurationResponse.Configurations[0].Resolution->Width,
                            getVideoEncoderConfigurationResponse.Configurations[0].Resolution->Height, &resolutionNum) == SUCCESS)
        {
            GetResolutionString(profStreamInfo->resolution, resolutionNum);
        }
        else
        {
            ConvertCameraResolutionToString(profStreamInfo->resolution, getVideoEncoderConfigurationResponse.Configurations[0].Resolution->Width,
                                            getVideoEncoderConfigurationResponse.Configurations[0].Resolution->Height);
        }
    }
    else
    {
        /* Set video resolution null */
        profStreamInfo->resolution[0] = '\0';
        EPRINT(ONVIF_CLIENT, "video resolution not available: [camera=%d], [profile=%d]", camIndex, profileIndex);
    }

    if (onvifCamInfo[camIndex].mediaServiceInfo.initialQuality == 1)
    {
        profStreamInfo->quality = ((UINT8)getVideoEncoderConfigurationResponse.Configurations[0].Quality);
    }
    else
    {
        profStreamInfo->quality = ((UINT8)getVideoEncoderConfigurationResponse.Configurations[0].Quality - onvifCamInfo[camIndex].mediaServiceInfo.initialQuality + 1);
    }

    profStreamInfo->bitrateMode = VBR;
    if (getVideoEncoderConfigurationResponse.Configurations[0].RateControl != NULL)
    {
        profStreamInfo->bitrateValue = getBitrateIndex(getVideoEncoderConfigurationResponse.Configurations[0].RateControl->BitrateLimit);
        profStreamInfo->framerate = (UINT8)getVideoEncoderConfigurationResponse.Configurations[0].RateControl->FrameRateLimit;
        if(getVideoEncoderConfigurationResponse.Configurations[0].RateControl->ConstantBitRate != NULL)
        {
            if(*getVideoEncoderConfigurationResponse.Configurations[0].RateControl->ConstantBitRate == XSD_BOOL_TRUE)
            {
                profStreamInfo->bitrateMode = CBR;
            }
        }
    }
    else
    {
        profStreamInfo->framerate = DFLT_PAL_FRAME_RATE;
        profStreamInfo->bitrateValue = BITRATE_1024;
        EPRINT(ONVIF_CLIENT, "fps and bitrate not available: [camera=%d], [profile=%d]", camIndex, profileIndex);
    }

    if (getVideoEncoderConfigurationResponse.Configurations[0].GovLength != NULL)
    {
        profStreamInfo->gop = (UINT8)*(getVideoEncoderConfigurationResponse.Configurations[0].GovLength);
    }
    else
    {
        profStreamInfo->gop = DFLT_CAM_GOP;
        EPRINT(ONVIF_CLIENT, "gop not available: [camera=%d], [profile=%d]", camIndex, profileIndex);
    }

//    DPRINT(ONVIF_CLIENT, "profile stream info: [camera=%d], [profile=%d], [encoding=%s], [bitrate=%d], [fps=%d], [bitrateMode=%d], [gop=%d], [resolution=%s], [quality=%d]",
//           camIndex, profileIndex, profStreamInfo->videoEncoding, profStreamInfo->bitrateValue, profStreamInfo->framerate,
//           profStreamInfo->bitrateMode, profStreamInfo->gop, profStreamInfo->resolution, profStreamInfo->quality);
    return ONVIF_CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetOnvifConfiguredProfile : to get max supported profile by camera
 * @param camIndex                  : camera index
 * @return
 */
UINT8 GetOnvifConfiguredProfile(UINT8 camIndex)
{
    return ((onvifCamInfo[camIndex].maxSupportedProfile) ? onvifCamInfo[camIndex].maxSupportedProfile : 1);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief getBitrateIndex   : Get enum value for bitrate
 * @param bitRateValue      : bit rate value
 * @return
 */
static BITRATE_VALUE_e getBitrateIndex(INT32 bitRateValue)
{
    UINT8 index;

    for (index = 0; index < MAX_BITRATE_VALUE; index++)
    {
        if (bitRateValue <= onvifBitrateValue[index])
        {
            return index;
        }
    }

    return BITRATE_1024;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief changeOnvifIpCameraAddress: change IP address of camera
 * @param soap                      : SOAP instance
 * @param user                      : User information
 * @param realCamIndex              : camera index
 * @param networkParam              : camera network parameters
 * @return
 */
static ONVIF_RESP_STATUS_e changeOnvifIpCameraAddress(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 camIndex, IP_ADDR_PARAM_t *networkParam)
{
    GET_NETWORK_INTERFACES_t 				getNetworkInterfaces;
    GET_NETWORK_INTERFACESRESPONSE_t 		getNetworkInterfacesResponse;
    SET_NETWORK_INTERFACES_t 				setNetworkInterfaces;
    SET_NETWORK_INTERFACESRESPONSE_t 		setNetworkInterfacesResponse;
    SET_NETWORK_DFLT_GATEWAY_t				setNetworkDefaultGateway;
    SET_NETWORK_DFLT_GATEWAYRESPONSE_t 		setNetworkDefaultGatewayResponse;
    SYSTEM_REBOOT_t							systemReboot;
    SYSTEM_REBOOTRESPONSE_t					systemRebootResponse;
    IP_ADDR_PARAM_t                         cameraNetworkParam = *networkParam;
    NETWORK_INTERFACE_SET_CONFIG_t 			networkInterfaceSetConfiguration;
    IPv4_NETWORK_INTERFACE_SET_CONFIG_t		ipv4NetworkInterfaceSetConfiguration;
    IPv6_NETWORK_INTERFACE_SET_CONFIG_t     ipv6NetworkInterfaceSetConfiguration;
    PREFIXED_IPv4_ADDR_t					prefixedIpv4Address;
    PREFIXED_IPv6_ADDR_t					prefixedIpv6Address;
    INT16  									soapResp;
    CHAR									networkIntrefaceToken[MAX_TOKEN_SIZE];
    INT32									networkMtu = NETWORK_MTU_SIZE_MAX;
    enum xsd__boolean 						xsdFalse = XSD_BOOL_FALSE;
    enum xsd__boolean 						xsdTrue = XSD_BOOL_TRUE;
    enum tt__IPv6DHCPConfiguration          ipv6DhcpConfig = IPV6_DHCP_OFF;
    CHARPTR									gatewayAddr = cameraNetworkParam.gateway;

    if (cameraNetworkParam.prefixLen == 0)
    {
        EPRINT(ONVIF_CLIENT, "invld ip addr prefix length: [camera=%d], [ip=%s], [prefixLen=%d]", camIndex, user->ipAddr, cameraNetworkParam.prefixLen);
        return ONVIF_CMD_IP_CHANGE_FAIL;
    }

    if (camIndex < getMaxCameraForCurrentVariant())
    {
        /* Get the time difference between NVR and Camera */
        getCameraDateTime(soap, camIndex, user);
    }

    soapResp = GetNetworkInterfaces(soap, &getNetworkInterfaces, &getNetworkInterfacesResponse, user);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "fail to get network interfaces: [camera=%d], [ip=%s], [soapResp=%d]", camIndex, user->ipAddr, soapResp);
        if (soapResp == 401)
        {
            /* Authentication error */
            return ONVIF_CMD_AUTHENTICATION_FAIL;
        }
        return ONVIF_CMD_IP_CHANGE_FAIL;
    }

    if (getNetworkInterfacesResponse.NetworkInterfaces->token == NULL)
    {
        EPRINT(ONVIF_CLIENT, "network interfaces not available: [camera=%d], [ip=%s]", camIndex, user->ipAddr);
        return ONVIF_CMD_IP_CHANGE_FAIL;
    }

    /* Fill network interface token, by which we can set */
    StringNCopy(networkIntrefaceToken, getNetworkInterfacesResponse.NetworkInterfaces->token, MAX_TOKEN_SIZE);
    if (((getNetworkInterfacesResponse.NetworkInterfaces->Info) != NULL) && ((getNetworkInterfacesResponse.NetworkInterfaces->Info->MTU) != NULL))
    {
        networkMtu = *(getNetworkInterfacesResponse.NetworkInterfaces->Info->MTU);
    }

    /* Now, set network default gateway ...first (like ODM) */
    if (cameraNetworkParam.ipAddrType == IP_ADDR_TYPE_IPV4)
    {
        setNetworkDefaultGateway.__sizeIPv4Address = 1;
        setNetworkDefaultGateway.IPv4Address = &gatewayAddr;
        setNetworkDefaultGateway.__sizeIPv6Address = 0;
        setNetworkDefaultGateway.IPv6Address = NULL;
    }
    else
    {
        setNetworkDefaultGateway.__sizeIPv4Address = 0;
        setNetworkDefaultGateway.IPv4Address = NULL;
        setNetworkDefaultGateway.__sizeIPv6Address = 1;
        setNetworkDefaultGateway.IPv6Address = &gatewayAddr;
    }

    soapResp = SetNetworkDfltGateway(soap, &setNetworkDefaultGateway, &setNetworkDefaultGatewayResponse, user);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "fail to set network default gateway: [camera=%d], [ip=%s], [soapResp=%d]", camIndex, user->ipAddr, soapResp);
        return ONVIF_CMD_IP_CHANGE_FAIL;
    }

    /* Now set Network parameter, as forcefully static ip */

    if (cameraNetworkParam.ipAddrType == IP_ADDR_TYPE_IPV4)
    {
        prefixedIpv4Address.Address = cameraNetworkParam.ipAddr;
        prefixedIpv4Address.PrefixLength = cameraNetworkParam.prefixLen;

        ipv4NetworkInterfaceSetConfiguration.Manual = &prefixedIpv4Address;
        ipv4NetworkInterfaceSetConfiguration.Enabled = &xsdTrue;
        ipv4NetworkInterfaceSetConfiguration.__sizeManual = 1;
        ipv4NetworkInterfaceSetConfiguration.DHCP = &xsdFalse;

        networkInterfaceSetConfiguration.IPv4 = &ipv4NetworkInterfaceSetConfiguration;
        networkInterfaceSetConfiguration.IPv6 = NULL;
    }
    else
    {
        prefixedIpv6Address.Address = cameraNetworkParam.ipAddr;
        prefixedIpv6Address.PrefixLength = cameraNetworkParam.prefixLen;

        ipv6NetworkInterfaceSetConfiguration.Manual = &prefixedIpv6Address;
        ipv6NetworkInterfaceSetConfiguration.Enabled = &xsdTrue;
        ipv6NetworkInterfaceSetConfiguration.AcceptRouterAdvert = &xsdTrue;
        ipv6NetworkInterfaceSetConfiguration.__sizeManual = 1;
        ipv6NetworkInterfaceSetConfiguration.DHCP = &ipv6DhcpConfig;

        networkInterfaceSetConfiguration.IPv6 = &ipv6NetworkInterfaceSetConfiguration;
        networkInterfaceSetConfiguration.IPv4 = NULL;
    }

    networkInterfaceSetConfiguration.Enabled = &xsdTrue;
    networkInterfaceSetConfiguration.Extension = NULL;
    networkInterfaceSetConfiguration.Link = NULL;
    networkInterfaceSetConfiguration.MTU = &networkMtu;
    soap_default_xsd__anyAttribute(NULL, &networkInterfaceSetConfiguration.__anyAttribute);
    setNetworkInterfaces.InterfaceToken = networkIntrefaceToken;
    setNetworkInterfaces.NetworkInterface = &networkInterfaceSetConfiguration;
    soapResp = SetNetworkInterfaces(soap, &setNetworkInterfaces, &setNetworkInterfacesResponse, user);
    if(soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "fail to set camera ip addr: [camera=%d], [ip=%s], [soapResp=%d]", camIndex, user->ipAddr, soapResp);
        return ONVIF_CMD_IP_CHANGE_FAIL;
    }

    /* Now, check reboot is required or not */
    if (setNetworkInterfacesResponse.RebootNeeded != XSD_BOOL_TRUE)
    {
        /* IP set and reboot not required */
        return ONVIF_CMD_SUCCESS;
    }

    /* Send system reboot command to camera */
    soapResp = SystemReboot(soap, &systemReboot, &systemRebootResponse, user);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "fail to reboot camera after setting ip addr: [camera=%d], [ip=%s], [soapResp=%d]", camIndex, user->ipAddr, soapResp);
        return ONVIF_CMD_CAMERA_REBOOT_FAIL;
    }

    /* IP set and reboot command sent to camera */
    return ONVIF_CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief getOnvifMotionWindowConfig    : will get motion window config and decode it
 * @param soap                          : SOAP Instance
 * @param user                          : User information
 * @param camIndex                      : camera index
 * @param motionGetWindowBuf            : motion cells configuration
 * @return
 */
static ONVIF_RESP_STATUS_e getOnvifMotionWindowConfig(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 camIndex, MOTION_BLOCK_METHOD_PARAM_t *motionGetWindowBuf)
{
    GET_VIDEOANALYTICSCONFIG_t 				getVideoAnalticConfig;
    GET_VIDEOANALYTICSCONFIGRESPONSE_t 		getVideoAnalticConfigResponse;
    INT16  									soapResp;
    UINT16									sensitivity = 0;
    UINT16 									index = 0, simpleItemIndex = 0;
    INT32									totalRow = 0, totalCol = 0;
    CHAR									activeCellBase64Str[512];
    BOOL									tagFound;

    /* Get the time difference between NVR and Camera */
    getCameraDateTime(soap, camIndex, user);
    user->addr = onvifCamInfo[camIndex].mediaServiceInfo.mediaServiceAddr;
    getVideoAnalticConfig.ConfigurationToken = onvifCamInfo[camIndex].analyticsServiceInfo.videoAnalticsToken;
    soapResp = GetVideoAnaltic(soap, &getVideoAnalticConfig, &getVideoAnalticConfigResponse, user);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "fail to get video analytics: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        return ONVIF_CMD_FAIL;
    }

    if (getVideoAnalticConfigResponse.Configuration == NULL)
    {
        EPRINT(ONVIF_CLIENT, "video analytics config not available: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    if (getVideoAnalticConfigResponse.Configuration->AnalyticsEngineConfiguration == NULL)
    {
        EPRINT(ONVIF_CLIENT, "video analytics engine config not available: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    if (getVideoAnalticConfigResponse.Configuration->RuleEngineConfiguration == NULL)
    {
        EPRINT(ONVIF_CLIENT, "video analytics rule engine config not available: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    tagFound = FALSE;
    for (index = 0; index < getVideoAnalticConfigResponse.Configuration->AnalyticsEngineConfiguration->__sizeAnalyticsModule; index++)
    {
        if (strstr((getVideoAnalticConfigResponse.Configuration->AnalyticsEngineConfiguration->AnalyticsModule + index)->Type, ONVIF_ANALTICS_CELL_MOTION_STR) != NULL)
        {
            tagFound = TRUE;
            break;
        }
    }

    if (FALSE == tagFound)
    {
        EPRINT(ONVIF_CLIENT, "video analytics cell motion engine tag not available: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    if ((getVideoAnalticConfigResponse.Configuration->AnalyticsEngineConfiguration->AnalyticsModule + index)->Parameters == NULL)
    {
        EPRINT(ONVIF_CLIENT, "video analytics cell motion parameters not available: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    if ((getVideoAnalticConfigResponse.Configuration->AnalyticsEngineConfiguration->AnalyticsModule + index)->Parameters->__sizeSimpleItem > 0)
    {
        sensitivity = (UINT16)atoi((getVideoAnalticConfigResponse.Configuration->AnalyticsEngineConfiguration->AnalyticsModule + index)->Parameters->SimpleItem->Value);
    }
    else
    {
        sensitivity = 0;
    }

    /* [
     * <tt:CellLayout Rows="18" Columns="22"><tt:Transformation><tt:Translate x="-1.000000" y="-1.000000"/>
     * <tt:Scale x="0.090909" y="0.111111"/>
     * </tt:Transformation>
     * </tt:CellLayout>
     * ] */
    if (((getVideoAnalticConfigResponse.Configuration->AnalyticsEngineConfiguration->AnalyticsModule + index)->Parameters->__sizeElementItem > 0) &&
            ((getVideoAnalticConfigResponse.Configuration->AnalyticsEngineConfiguration->AnalyticsModule + index)->Parameters->ElementItem->__any.atts))
    {
        xsd__anyAttribute *temp;
        temp = (getVideoAnalticConfigResponse.Configuration->AnalyticsEngineConfiguration->AnalyticsModule + index)->Parameters->ElementItem->__any.atts;
        while (temp)
        {
            if (temp->name)
            {
                if (strcmp(temp->name, "Columns") == STATUS_OK)
                {
                    if(temp->text)
                    {
                        sscanf(temp->text, "%d", &totalCol);
                    }
                }
                else if (strcmp(temp->name, "Rows") == STATUS_OK)
                {
                    if (temp->text)
                    {
                        sscanf(temp->text,"%d", &totalRow);
                    }
                }
            }
            temp = temp->next;
        }
    }

    DPRINT(ONVIF_CLIENT, "video analytics cell montion engine tag found: [camera=%d], [row=%d], [column=%d], sensitivity[%d]",
           camIndex, totalRow, totalCol, sensitivity);

    tagFound = FALSE;
    for (index = 0; index < getVideoAnalticConfigResponse.Configuration->RuleEngineConfiguration->__sizeRule; index++)
    {
        if (strstr((getVideoAnalticConfigResponse.Configuration->RuleEngineConfiguration->Rule + index)->Type, ONVIF_ANALTICS_CELL_MOTION_DET_STR) != NULL)
        {
            tagFound = TRUE;
            break;
        }
    }

    if (tagFound == FALSE)
    {
        EPRINT(ONVIF_CLIENT, "video analytics rule motion engine tag not available: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    if ((getVideoAnalticConfigResponse.Configuration->RuleEngineConfiguration->Rule + index)->Parameters == NULL)
    {
        EPRINT(ONVIF_CLIENT, "video analytics rule parameters not available: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    tagFound = FALSE;
    for (simpleItemIndex = 0; simpleItemIndex < (getVideoAnalticConfigResponse.Configuration->RuleEngineConfiguration->Rule + index)->Parameters->__sizeSimpleItem; simpleItemIndex++)
    {
        if (strcasecmp(((getVideoAnalticConfigResponse.Configuration->RuleEngineConfiguration->Rule + index)->Parameters->SimpleItem + simpleItemIndex)->Name,
                      ONVIF_ANALTICS_ACTIVE_CELL_STR) == STATUS_OK)
        {
            StringNCopy(activeCellBase64Str, ((getVideoAnalticConfigResponse.Configuration->RuleEngineConfiguration->Rule + index)->Parameters->SimpleItem + simpleItemIndex)->Value, 500);
            tagFound = TRUE;
            break;
        }
    }

    if (tagFound == FALSE)
    {
        EPRINT(ONVIF_CLIENT, "video analytics rule motion active cell tag not available: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    if (FAIL == parseBase64MotionWindow(activeCellBase64Str, (UINT8)totalCol, (UINT8)totalRow, motionGetWindowBuf->blockBitString))
    {
        EPRINT(ONVIF_CLIENT, "fail to parse active cell to motion window: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    motionGetWindowBuf->sensitivity = (UINT8) (sensitivity / 10);
    motionGetWindowBuf->noMotionSupportF = FALSE;
    motionGetWindowBuf->isNoMotionEvent = FALSE;
    motionGetWindowBuf->noMotionDuration = 5;

    return ONVIF_CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief getOnvifMotionWindowConfig2   : will get motion window config and decode it
 * @param soap                          : SOAP Instance
 * @param user                          : User information
 * @param camIndex                      : camera index
 * @param motionGetWindowBuf            : motion cells configuration
 * @return
 */
static ONVIF_RESP_STATUS_e getOnvifMotionWindowConfig2(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 camIndex, MOTION_BLOCK_METHOD_PARAM_t *motionGetWindowBuf)
{
    GET_ANALYTICS_MODULES_t                 getAnalyticsModule;
    GET_ANALYTICS_MODULES_RESPONSE_t        getAnalyticsModuleResponse;
    GET_RULES_t                             getRules;
    GET_RULES_RESPONSE_t                    getRulesResponse;
    INT16  									soapResp;
    UINT16									sensitivity = 0;
    UINT16 									index = 0, simpleItemIndex = 0;
    INT32									totalRow = 0, totalCol = 0;
    CHAR									activeCellBase64Str[512];
    BOOL									tagFound;

    /* Get the time difference between NVR and Camera */
    getCameraDateTime(soap, camIndex, user);
    user->addr =  onvifCamInfo[camIndex].mediaServiceInfo.analyticsServiceAddr;
    getAnalyticsModule.ConfigurationToken = onvifCamInfo[camIndex].analyticsServiceInfo.videoAnalticsToken;
    soapResp = GetAnalyticsModules(soap, &getAnalyticsModule, &getAnalyticsModuleResponse, user);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "fail to get analytics modules: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        return ONVIF_CMD_FAIL;
    }

    if ((getAnalyticsModuleResponse.__sizeAnalyticsModule == 0) || (getAnalyticsModuleResponse.AnalyticsModule == NULL))
    {
        EPRINT(ONVIF_CLIENT, "analytics modules config not available: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        return ONVIF_CMD_FAIL;
    }

    getCameraDateTime(soap, camIndex, user);
    user->addr = onvifCamInfo[camIndex].mediaServiceInfo.analyticsServiceAddr;
    getRules.ConfigurationToken = onvifCamInfo[camIndex].analyticsServiceInfo.videoAnalticsToken;
    soapResp = GetRules(soap, &getRules, &getRulesResponse, user);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "fail to get analytics rules: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        return ONVIF_CMD_FAIL;
    }

    if ((getRulesResponse.__sizeRule == 0) || (getRulesResponse.Rule == NULL))
    {
        EPRINT(ONVIF_CLIENT, "analytics rules not available: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        return ONVIF_CMD_FAIL;
    }

    tagFound = FALSE;
    for (index = 0; index < getAnalyticsModuleResponse.__sizeAnalyticsModule; index++)
    {
        if (strstr((getAnalyticsModuleResponse.AnalyticsModule + index)->Type, ONVIF_ANALTICS_CELL_MOTION_STR) != NULL)
        {
            tagFound = TRUE;
            break;
        }
    }

    if (FALSE == tagFound)
    {
        EPRINT(ONVIF_CLIENT, "video analytics cell motion engine tag not available: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    if ((getAnalyticsModuleResponse.AnalyticsModule + index)->Parameters == NULL)
    {
        EPRINT(ONVIF_CLIENT, "video analytics cell motion parameters not available: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    if ((getAnalyticsModuleResponse.AnalyticsModule + index)->Parameters->__sizeSimpleItem > 0)
    {
        sensitivity = (UINT16)atoi((getAnalyticsModuleResponse.AnalyticsModule + index)->Parameters->SimpleItem->Value);
    }
    else
    {
        sensitivity = 0;
    }

    /* [
     * <tt:CellLayout Rows="18" Columns="22"><tt:Transformation><tt:Translate x="-1.000000" y="-1.000000"/>
     * <tt:Scale x="0.090909" y="0.111111"/>
     * </tt:Transformation>
     * </tt:CellLayout>
     * ] */
    if (((getAnalyticsModuleResponse.AnalyticsModule + index)->Parameters->__sizeElementItem > 0) &&
            ((getAnalyticsModuleResponse.AnalyticsModule + index)->Parameters->ElementItem->__any.atts))
    {
        xsd__anyAttribute *temp;
        temp = (getAnalyticsModuleResponse.AnalyticsModule + index)->Parameters->ElementItem->__any.atts;
        while (temp)
        {
            if (temp->name)
            {
                if (strcmp(temp->name, "Columns") == STATUS_OK)
                {
                    if(temp->text)
                    {
                        sscanf(temp->text, "%d", &totalCol);
                    }
                }
                else if (strcmp(temp->name, "Rows") == STATUS_OK)
                {
                    if (temp->text)
                    {
                        sscanf(temp->text,"%d", &totalRow);
                    }
                }
            }
            temp = temp->next;
        }
    }

    DPRINT(ONVIF_CLIENT, "video analytics cell montion engine tag found: [camera=%d], [row=%d], [column=%d], sensitivity[%d]",
           camIndex, totalRow, totalCol, sensitivity);

    tagFound = FALSE;
    for (index = 0; index < getRulesResponse.__sizeRule; index++)
    {
        if (strstr((getRulesResponse.Rule + index)->Type, ONVIF_ANALTICS_CELL_MOTION_DET_STR) != NULL)
        {
            tagFound = TRUE;
            break;
        }
    }

    if (tagFound == FALSE)
    {
        EPRINT(ONVIF_CLIENT, "video analytics rule motion engine tag not available: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    if ((getRulesResponse.Rule + index)->Parameters == NULL)
    {
        EPRINT(ONVIF_CLIENT, "video analytics rule parameters not available: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    tagFound = FALSE;
    for (simpleItemIndex = 0; simpleItemIndex < (getRulesResponse.Rule + index)->Parameters->__sizeSimpleItem; simpleItemIndex++)
    {
        if (strcasecmp(((getRulesResponse.Rule + index)->Parameters->SimpleItem + simpleItemIndex)->Name, ONVIF_ANALTICS_ACTIVE_CELL_STR) == STATUS_OK)
        {
            StringNCopy(activeCellBase64Str, ((getRulesResponse.Rule + index)->Parameters->SimpleItem + simpleItemIndex)->Value, 500);
            tagFound = TRUE;
            break;
        }
    }

    if (tagFound == FALSE)
    {
        EPRINT(ONVIF_CLIENT, "video analytics rule motion active cell tag not available: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    if (FAIL == parseBase64MotionWindow(activeCellBase64Str, (UINT8)totalCol, (UINT8)totalRow, motionGetWindowBuf->blockBitString))
    {
        EPRINT(ONVIF_CLIENT, "fail to parse active cell to motion window: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    motionGetWindowBuf->sensitivity = (UINT8) (sensitivity / 10);
    motionGetWindowBuf->noMotionSupportF = FALSE;
    motionGetWindowBuf->isNoMotionEvent = FALSE;
    motionGetWindowBuf->noMotionDuration = 5;

    return ONVIF_CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief parseBase64MotionWindow   : Decode motion window info to required format
 * @param source                    : inpur string
 * @param column                    : coulmn number
 * @param row                       : row number
 * @param outWindowResponse         : outpur string
 * @return
 */
static BOOL parseBase64MotionWindow(CHARPTR source, UINT8 column, UINT8 row, CHARPTR outWindowResponse)
{
    UINT16		unpackLength = 0;
    UINT16		outLen = 0;
    UINT32		packLenth = 0;
    CHAR		unPackByte[MOTION_AREA_BLOCK_BYTES+1];
    CHARPTR		decodedStr;
    CHAR        **gridRendomPtr;
    CHAR        **grid44_36Ptr;

    decodedStr = DecodeBase64(source, strlen(source), &packLenth);
    if (decodedStr == NULL)
    {
        return FAIL;
    }

    gridRendomPtr = (CHAR **)Allocate2DArray(row, column, sizeof(CHAR));
    if (NULL == gridRendomPtr)
    {
        FREE_MEMORY(decodedStr);
        return FAIL;
    }

    grid44_36Ptr = (CHAR **)Allocate2DArray(36, 44, sizeof(CHAR));
    if (NULL == grid44_36Ptr)
    {
        Free2DArray((void**)gridRendomPtr, row);
        FREE_MEMORY(decodedStr);
        return FAIL;
    }

    unpackLength = (UINT16)Tiff6_unPackBits(decodedStr, packLenth, (UINT8PTR)unPackByte);
    unPackByte[unpackLength] = '\0';

    //function will convert the packed hex bytes to unpack hex bytes and put it in to the grid of colxrow
    UnPackGridGeneral(unPackByte, column, row, gridRendomPtr, unpackLength);

    //function will conver grid of given colxrow in to 44x36 grid
    ConvertUnpackedGridTo44_36(gridRendomPtr, grid44_36Ptr, column, row);

    //function will convert 44x36 grid in to thew packed bits of buffer 198 bytes
    PackGridGeneral(grid44_36Ptr, 44, 36, outWindowResponse, &outLen);

    /* Free allocated memory */
    Free2DArray((void**)gridRendomPtr, row);
    Free2DArray((void**)grid44_36Ptr, 36);
    FREE_MEMORY(decodedStr);

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief setOnvifMotionWindowConfig    : will set motion window to camera
 * @param soap                          : SOAP instance
 * @param user                          : User information
 * @param camIndex                      : camera index
 * @param motionSetWindowBuf            : selected motion Grid information
 * @return
 */
static ONVIF_RESP_STATUS_e setOnvifMotionWindowConfig(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 camIndex, MOTION_BLOCK_METHOD_PARAM_t *motionSetWindowBuf)
{
    GET_VIDEOANALYTICSCONFIG_t 				getVideoAnalticConfig;
    GET_VIDEOANALYTICSCONFIGRESPONSE_t 		getVideoAnalticConfigResponse;
    SET_VIDEOANALYTICSCONFIG_t				setVideoAnalyticsConfig;
    SET_VIDEOANALYTICSCONFIGRESPONSE_t		setVideoAnalyticsConfigResponse;
    VIDEO_ANALYTICS_CONFIGURATION_t			videoAnalyticsConfig;
    ITEMLIST_SIMPLEITEM_t					simpleItem[ONVIF_ANALTICS_SIMPLE_ITEM_MAX];
    ITEMLIST_SIMPLEITEM_t					analticSimpleItem;
    ITEMLIST_t 								analyticItemList;
    ITEMLIST_t 								rulesItemList;
    CONFIG_t 								analyticConfig;
    CONFIG_t 								rulesConfig;
    ANALYTICSENGINECONFIG_t 				analyticsEngineConfiguration;
    RULEENGINECONFIG_t 						ruleEngineConfiguration;
    CHAR									sensitivityValue[5];
    INT16  									soapResp;
    UINT8 									analyticsIndex, ruleIndex, simpleItemIndex = 0;
    CHAR									activeCellStr[512];
    INT32									totalRow = 0, totalCol = 0;
    BOOL									tagFound;

    /* Get the time difference between NVR and Camera */
    getCameraDateTime(soap, camIndex, user);
    user->addr = onvifCamInfo[camIndex].mediaServiceInfo.mediaServiceAddr;
    getVideoAnalticConfig.ConfigurationToken = onvifCamInfo[camIndex].analyticsServiceInfo.videoAnalticsToken;
    soapResp = GetVideoAnaltic(soap, &getVideoAnalticConfig, &getVideoAnalticConfigResponse, user);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "fail to get video analytics: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        return ONVIF_CMD_FAIL;
    }

    if (getVideoAnalticConfigResponse.Configuration == NULL)
    {
        EPRINT(ONVIF_CLIENT, "video analytics config not available: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    if (getVideoAnalticConfigResponse.Configuration->AnalyticsEngineConfiguration == NULL)
    {
        EPRINT(ONVIF_CLIENT, "video analytics engine config not available: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    if (getVideoAnalticConfigResponse.Configuration->RuleEngineConfiguration == NULL)
    {
        EPRINT(ONVIF_CLIENT, "video analytics rule engine config not available: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    tagFound = FALSE;
    for (analyticsIndex = 0; analyticsIndex < getVideoAnalticConfigResponse.Configuration->AnalyticsEngineConfiguration->__sizeAnalyticsModule; analyticsIndex++)
    {
        //No need for NULL check here
        if (strstr((getVideoAnalticConfigResponse.Configuration->AnalyticsEngineConfiguration->AnalyticsModule + analyticsIndex)->Type, ONVIF_ANALTICS_CELL_MOTION_STR) != NULL)
        {
            tagFound = TRUE;
            break;
        }
    }

    if (FALSE == tagFound)
    {
        EPRINT(ONVIF_CLIENT, "video analytics cell motion engine tag not available: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    if ((getVideoAnalticConfigResponse.Configuration->AnalyticsEngineConfiguration->AnalyticsModule + analyticsIndex)->Parameters == NULL)
    {
        EPRINT(ONVIF_CLIENT, "video analytics cell motion parameters not available: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    /* [
     * <tt:CellLayout Rows="18" Columns="22"><tt:Transformation><tt:Translate x="-1.000000" y="-1.000000"/>
     * <tt:Scale x="0.090909" y="0.111111"/>
     * </tt:Transformation>
     * </tt:CellLayout>
     * ] */
    if (((getVideoAnalticConfigResponse.Configuration->AnalyticsEngineConfiguration->AnalyticsModule + analyticsIndex)->Parameters->__sizeElementItem > 0) &&
            ((getVideoAnalticConfigResponse.Configuration->AnalyticsEngineConfiguration->AnalyticsModule + analyticsIndex)->Parameters->ElementItem->__any.atts))
    {
        xsd__anyAttribute *temp;
        temp = (getVideoAnalticConfigResponse.Configuration->AnalyticsEngineConfiguration->AnalyticsModule + analyticsIndex)->Parameters->ElementItem->__any.atts;
        while(temp)
        {
            if (temp->name)
            {
                if (strcmp(temp->name,"Columns") == STATUS_OK)
                {
                    if (temp->text)
                    {
                        sscanf(temp->text,"%d", &totalCol);
                    }
                }
                else if (strcmp(temp->name,"Rows") == STATUS_OK)
                {
                    if (temp->text)
                    {
                        sscanf(temp->text,"%d", &totalRow);
                    }
                }
            }
            temp = temp->next;
        }
    }

    snprintf(sensitivityValue, sizeof(sensitivityValue), "%d", (motionSetWindowBuf->sensitivity * 10));
    if (FAIL == convertMotionwinBufToActiveCells(motionSetWindowBuf->blockBitString, activeCellStr, (UINT8)totalRow, (UINT8)totalCol, sizeof(activeCellStr)))
    {
        EPRINT(ONVIF_CLIENT, "fail to convert motion window to active cell: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    tagFound = FALSE;
    for (ruleIndex = 0; ruleIndex < getVideoAnalticConfigResponse.Configuration->RuleEngineConfiguration->__sizeRule; ruleIndex++)
    {
        if (strstr((getVideoAnalticConfigResponse.Configuration->RuleEngineConfiguration->Rule + ruleIndex)->Type, ONVIF_ANALTICS_CELL_MOTION_DET_STR) != NULL)
        {
            tagFound = TRUE;
            break;
        }
    }

    if (tagFound == FALSE)
    {
        EPRINT(ONVIF_CLIENT, "video analytics rule motion engine tag not available: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    if ((getVideoAnalticConfigResponse.Configuration->RuleEngineConfiguration->Rule + ruleIndex)->Parameters == NULL)
    {
        EPRINT(ONVIF_CLIENT, "video analytics rule parameters not available: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    for (simpleItemIndex = 0; simpleItemIndex < (getVideoAnalticConfigResponse.Configuration->RuleEngineConfiguration->Rule + ruleIndex)->Parameters->__sizeSimpleItem; simpleItemIndex++)
    {
        if (simpleItemIndex >= ONVIF_ANALTICS_SIMPLE_ITEM_MAX)
        {
            EPRINT(ONVIF_CLIENT, "more simple items available in video analytics rule parameters: [camera=%d]", camIndex);
            break;
        }

        simpleItem[simpleItemIndex].Name = ((getVideoAnalticConfigResponse.Configuration->RuleEngineConfiguration->Rule + ruleIndex)->Parameters->SimpleItem + simpleItemIndex)->Name;
        if (strcasecmp(simpleItem[simpleItemIndex].Name, ONVIF_ANALTICS_ACTIVE_CELL_STR) == STATUS_OK)
        {
            simpleItem[simpleItemIndex].Value = activeCellStr;
        }
        else
        {
            simpleItem[simpleItemIndex].Value = ((getVideoAnalticConfigResponse.Configuration->RuleEngineConfiguration->Rule + ruleIndex)->Parameters->SimpleItem + simpleItemIndex)->Value;
        }
    }

    // fill anaytics engine configuration param, which will set sensitivity
    analticSimpleItem.Name = (getVideoAnalticConfigResponse.Configuration->AnalyticsEngineConfiguration->AnalyticsModule + analyticsIndex)->Parameters->SimpleItem->Name;
    analticSimpleItem.Value = sensitivityValue;
    analyticItemList.ElementItem = (getVideoAnalticConfigResponse.Configuration->AnalyticsEngineConfiguration->AnalyticsModule + analyticsIndex)->Parameters->ElementItem;
    analyticItemList.Extension = NULL;
    analyticItemList.SimpleItem = &analticSimpleItem;
    soap_default_xsd__anyAttribute(NULL, &analyticItemList.__anyAttribute);
    analyticItemList.__sizeElementItem = (getVideoAnalticConfigResponse.Configuration->AnalyticsEngineConfiguration->AnalyticsModule + analyticsIndex)->Parameters->__sizeElementItem;
    analyticItemList.__sizeSimpleItem = 1;
    analyticConfig.Name = (getVideoAnalticConfigResponse.Configuration->AnalyticsEngineConfiguration->AnalyticsModule + analyticsIndex)->Name;
    analyticConfig.Type = (getVideoAnalticConfigResponse.Configuration->AnalyticsEngineConfiguration->AnalyticsModule + analyticsIndex)->Type;
    analyticConfig.Parameters = &analyticItemList;
    analyticsEngineConfiguration.AnalyticsModule = &analyticConfig;
    analyticsEngineConfiguration.Extension = NULL;
    soap_default_xsd__anyAttribute(NULL, &analyticsEngineConfiguration.__anyAttribute);
    analyticsEngineConfiguration.__sizeAnalyticsModule = 1;

    // fill rule engine configuration param, which will set activecells
    rulesItemList.ElementItem = NULL;
    rulesItemList.Extension = NULL;
    rulesItemList.SimpleItem = simpleItem;
    soap_default_xsd__anyAttribute(NULL, &rulesItemList.__anyAttribute);
    rulesItemList.__sizeElementItem = 0;
    rulesItemList.__sizeSimpleItem = simpleItemIndex;
    rulesConfig.Name = (getVideoAnalticConfigResponse.Configuration->RuleEngineConfiguration->Rule + ruleIndex)->Name;
    rulesConfig.Type = (getVideoAnalticConfigResponse.Configuration->RuleEngineConfiguration->Rule + ruleIndex)->Type;
    rulesConfig.Parameters = &rulesItemList;
    ruleEngineConfiguration.Rule = &rulesConfig;
    ruleEngineConfiguration.Extension = NULL;
    soap_default_xsd__anyAttribute(NULL, &ruleEngineConfiguration.__anyAttribute);
    ruleEngineConfiguration.__sizeRule = 1;

    // fill both in videoAnalyticsConfig
    videoAnalyticsConfig.Name = getVideoAnalticConfigResponse.Configuration->Name;
    videoAnalyticsConfig.UseCount = getVideoAnalticConfigResponse.Configuration->UseCount;
    videoAnalyticsConfig.__any = NULL;
    soap_default_xsd__anyAttribute(NULL, &videoAnalyticsConfig.__anyAttribute);

    videoAnalyticsConfig.__size = getVideoAnalticConfigResponse.Configuration->__size;
    videoAnalyticsConfig.token = getVideoAnalticConfigResponse.Configuration->token;
    videoAnalyticsConfig.AnalyticsEngineConfiguration = &analyticsEngineConfiguration;
    videoAnalyticsConfig.RuleEngineConfiguration = &ruleEngineConfiguration;

    setVideoAnalyticsConfig.Configuration = &videoAnalyticsConfig;
    setVideoAnalyticsConfig.ForcePersistence = XSD_BOOL_TRUE;

    soapResp = SetVideoAnaltic(soap, &setVideoAnalyticsConfig, &setVideoAnalyticsConfigResponse, user);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "fail to set video analytics: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        return ONVIF_CMD_FAIL;
    }

    return ONVIF_CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief setOnvifMotionWindowConfig2   : will set motion window to camera
 * @param soap                          : SOAP instance
 * @param user                          : User information
 * @param camIndex                      : camera index
 * @param motionSetWindowBuf            : selected motion Grid information
 * @return
 */
static ONVIF_RESP_STATUS_e setOnvifMotionWindowConfig2(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 camIndex, MOTION_BLOCK_METHOD_PARAM_t *motionSetWindowBuf)
{
    GET_ANALYTICS_MODULES_t                 getAnalyticsModule;
    GET_ANALYTICS_MODULES_RESPONSE_t        getAnalyticsModuleResponse;
    GET_RULES_t                             getRules;
    GET_RULES_RESPONSE_t                    getRulesResponse;
    MODIFY_ANALYTICS_MODULES_t              modifyAnalyticsModule;
    MODIFY_ANALYTICS_MODULES_RESPONSE_t     modifyAnalyticsModuleResponse;
    MODIFY_RULES_t                          modifyRules;
    MODIFY_RULES_RESPONSE_t                 modifyRulesResponse;
    ITEMLIST_SIMPLEITEM_t					simpleItem[ONVIF_ANALTICS_SIMPLE_ITEM_MAX];
    ITEMLIST_SIMPLEITEM_t					analticSimpleItem;
    ITEMLIST_t 								analyticItemList;
    ITEMLIST_t 								rulesItemList;
    CONFIG_t 								analyticConfig;
    CONFIG_t 								rulesConfig;
    CHAR									sensitivityValue[5];
    INT16  									soapResp;
    UINT8 									analyticsIndex, ruleIndex, simpleItemIndex = 0;
    CHAR									activeCellStr[512];
    INT32									totalRow = 0, totalCol = 0;
    BOOL									tagFound;

    /* Added this logic for tiandy OEM camera */
    getCameraDateTime(soap, camIndex, user);
    user->addr =  onvifCamInfo[camIndex].mediaServiceInfo.analyticsServiceAddr;
    getAnalyticsModule.ConfigurationToken = onvifCamInfo[camIndex].analyticsServiceInfo.videoAnalticsToken;
    soapResp = GetAnalyticsModules(soap, &getAnalyticsModule, &getAnalyticsModuleResponse, user);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "fail to get analytics modules: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        return ONVIF_CMD_FAIL;
    }

    if ((getAnalyticsModuleResponse.__sizeAnalyticsModule == 0) || (getAnalyticsModuleResponse.AnalyticsModule == NULL))
    {
        EPRINT(ONVIF_CLIENT, "analytics modules config not available: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        return ONVIF_CMD_FAIL;
    }

    getCameraDateTime(soap, camIndex, user);
    user->addr =  onvifCamInfo[camIndex].mediaServiceInfo.analyticsServiceAddr;
    getRules.ConfigurationToken = onvifCamInfo[camIndex].analyticsServiceInfo.videoAnalticsToken;
    soapResp = GetRules(soap, &getRules, &getRulesResponse, user);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "fail to get analytics rules: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        return ONVIF_CMD_FAIL;
    }

    if ((getRulesResponse.__sizeRule == 0) || (getRulesResponse.Rule == NULL))
    {
        EPRINT(ONVIF_CLIENT, "analytics rules config not available: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        return ONVIF_CMD_FAIL;
    }

    tagFound = FALSE;
    for (analyticsIndex = 0; analyticsIndex < getAnalyticsModuleResponse.__sizeAnalyticsModule; analyticsIndex++)
    {
        if (strstr((getAnalyticsModuleResponse.AnalyticsModule + analyticsIndex)->Type, ONVIF_ANALTICS_CELL_MOTION_STR) != NULL)
        {
            tagFound = TRUE;
            break;
        }
    }

    if (FALSE == tagFound)
    {
        EPRINT(ONVIF_CLIENT, "video analytics cell motion engine tag not available: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    if ((getAnalyticsModuleResponse.AnalyticsModule + analyticsIndex)->Parameters == NULL)
    {
        EPRINT(ONVIF_CLIENT, "video analytics cell motion parameters not available: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    /* [
     * <tt:CellLayout Rows="18" Columns="22"><tt:Transformation><tt:Translate x="-1.000000" y="-1.000000"/>
     * <tt:Scale x="0.090909" y="0.111111"/>
     * </tt:Transformation>
     * </tt:CellLayout>
     * ] */
    if (((getAnalyticsModuleResponse.AnalyticsModule + analyticsIndex)->Parameters->__sizeElementItem > 0) &&
            ((getAnalyticsModuleResponse.AnalyticsModule + analyticsIndex)->Parameters->ElementItem->__any.atts))
    {
        xsd__anyAttribute *temp;
        temp = (getAnalyticsModuleResponse.AnalyticsModule + analyticsIndex)->Parameters->ElementItem->__any.atts;
        while (temp)
        {
            if (temp->name)
            {
                if (strcmp(temp->name, "Columns") == STATUS_OK)
                {
                    if(temp->text)
                    {
                        sscanf(temp->text, "%d", &totalCol);
                    }
                }
                else if (strcmp(temp->name, "Rows") == STATUS_OK)
                {
                    if (temp->text)
                    {
                        sscanf(temp->text,"%d", &totalRow);
                    }
                }
            }
            temp = temp->next;
        }
    }

    snprintf(sensitivityValue, sizeof(sensitivityValue), "%d", (motionSetWindowBuf->sensitivity * 10));
    if (FAIL == convertMotionwinBufToActiveCells(motionSetWindowBuf->blockBitString, activeCellStr, (UINT8)totalRow, (UINT8)totalCol, sizeof(activeCellStr)))
    {
        EPRINT(ONVIF_CLIENT, "fail to convert motion window to active cell: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    tagFound = FALSE;
    for (ruleIndex = 0; ruleIndex < getRulesResponse.__sizeRule; ruleIndex++)
    {
        if (strstr((getRulesResponse.Rule + ruleIndex)->Type, ONVIF_ANALTICS_CELL_MOTION_DET_STR) != NULL)
        {
            tagFound = TRUE;
            break;
        }
    }

    if (tagFound == FALSE)
    {
        EPRINT(ONVIF_CLIENT, "video analytics rule motion engine tag not available: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    if ((getRulesResponse.Rule + ruleIndex)->Parameters == NULL)
    {
        EPRINT(ONVIF_CLIENT, "video analytics rule parameters not available: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    for (simpleItemIndex = 0; simpleItemIndex < (getRulesResponse.Rule + ruleIndex)->Parameters->__sizeSimpleItem; simpleItemIndex++)
    {
        if (simpleItemIndex >= ONVIF_ANALTICS_SIMPLE_ITEM_MAX)
        {
            EPRINT(ONVIF_CLIENT, "more simple items available in video analytics rule parameters: [camera=%d]", camIndex);
            break;
        }

        simpleItem[simpleItemIndex].Name = ((getRulesResponse.Rule + ruleIndex)->Parameters->SimpleItem + simpleItemIndex)->Name;
        if (strcasecmp(simpleItem[simpleItemIndex].Name, ONVIF_ANALTICS_ACTIVE_CELL_STR) == STATUS_OK)
        {
            simpleItem[simpleItemIndex].Value = activeCellStr;
        }
        else
        {
            simpleItem[simpleItemIndex].Value = ((getRulesResponse.Rule + ruleIndex)->Parameters->SimpleItem + simpleItemIndex)->Value;
        }
    }

    // fill anaytics engine configuration param, which will set sensitivity
    analticSimpleItem.Name = (getAnalyticsModuleResponse.AnalyticsModule + analyticsIndex)->Parameters->SimpleItem->Name;
    analticSimpleItem.Value = sensitivityValue;
    analyticItemList.ElementItem = (getAnalyticsModuleResponse.AnalyticsModule + analyticsIndex)->Parameters->ElementItem;
    analyticItemList.Extension = NULL;
    analyticItemList.SimpleItem = &analticSimpleItem;
    soap_default_xsd__anyAttribute(NULL, &analyticItemList.__anyAttribute);
    analyticItemList.__sizeElementItem = (getAnalyticsModuleResponse.AnalyticsModule + analyticsIndex)->Parameters->__sizeElementItem;
    analyticItemList.__sizeSimpleItem = 1;
    analyticConfig.Name = (getAnalyticsModuleResponse.AnalyticsModule + analyticsIndex)->Name;
    analyticConfig.Type = (getAnalyticsModuleResponse.AnalyticsModule + analyticsIndex)->Type;
    analyticConfig.Parameters = &analyticItemList;
    modifyAnalyticsModule.__sizeAnalyticsModule = 1;
    modifyAnalyticsModule.AnalyticsModule = &analyticConfig;

    // fill rule engine configuration param, which will set activecells
    rulesItemList.ElementItem = NULL;
    rulesItemList.Extension = NULL;
    rulesItemList.SimpleItem = simpleItem;
    soap_default_xsd__anyAttribute(NULL, &rulesItemList.__anyAttribute);
    rulesItemList.__sizeElementItem = 0;
    rulesItemList.__sizeSimpleItem = simpleItemIndex;
    rulesConfig.Name = (getRulesResponse.Rule + ruleIndex)->Name;
    rulesConfig.Type = (getRulesResponse.Rule + ruleIndex)->Type;
    rulesConfig.Parameters = &rulesItemList;
    modifyRules.__sizeRule = 1;
    modifyRules.Rule = &rulesConfig;

    getCameraDateTime(soap, camIndex, user);
    user->addr =  onvifCamInfo[camIndex].mediaServiceInfo.analyticsServiceAddr;
    modifyAnalyticsModule.ConfigurationToken = onvifCamInfo[camIndex].analyticsServiceInfo.videoAnalticsToken;
    soapResp = ModifyAnalyticsModules(soap, &modifyAnalyticsModule, &modifyAnalyticsModuleResponse, user);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "fail to modify analytics modules: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        return ONVIF_CMD_FAIL;
    }

    getCameraDateTime(soap, camIndex, user);
    user->addr =  onvifCamInfo[camIndex].mediaServiceInfo.analyticsServiceAddr;
    modifyRules.ConfigurationToken = onvifCamInfo[camIndex].analyticsServiceInfo.videoAnalticsToken;
    soapResp = ModifyRules(soap, &modifyRules, &modifyRulesResponse, user);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "fail to modify analytics rules: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        return ONVIF_CMD_FAIL;
    }

    return ONVIF_CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief convertMotionwinBufToActiveCells : convert motion window to onvif required strings
 * @param motionSetWindowBuf               : motion window configurtion
 * @param activeCellStr                    : cell format string
 * @param row                              : row number
 * @param column                           : column number
 * @return
 */
static BOOL convertMotionwinBufToActiveCells(CHARPTR motionSetWindowBuf, CHARPTR activeCellStr, UINT8 row, UINT8 column, const UINT16 activeCellLen)
{
    CHAR        **grid44_36Ptr;
    CHAR        **gridRendomPtr;
    UINT8		tempCnt;
    CHAR		packGridFinal[MOTION_AREA_BLOCK_BYTES+1];
    UINT8		packGridFinalInt[MOTION_AREA_BLOCK_BYTES+1] = {0};
    UINT16		packGridLen = 0;
    CHAR		packBytes[512];
    CHARPTR		encodedStrPtr;
    UINT32		packBitLen;

    grid44_36Ptr = (CHAR **)Allocate2DArray(36, 44, sizeof(CHAR));
    if (NULL == grid44_36Ptr)
    {
        return FAIL;
    }

    gridRendomPtr = (CHAR **)Allocate2DArray(row, column, sizeof(CHAR));
    if (NULL == gridRendomPtr)
    {
        Free2DArray((void**)grid44_36Ptr, 36);
        return FAIL;
    }

    UnPackGridGeneral(motionSetWindowBuf, 44, 36, grid44_36Ptr, MOTION_AREA_BLOCK_BYTES);
    Convert44_36ToUnpackedGrid(grid44_36Ptr, gridRendomPtr, column, row);
    PackGridGeneral(gridRendomPtr, column, row, packGridFinal, &packGridLen);
    Free2DArray((void**)grid44_36Ptr, 36);
    Free2DArray((void**)gridRendomPtr, row);

    for (tempCnt=0; tempCnt< packGridLen; tempCnt++)
    {
        packGridFinalInt[tempCnt] = (UINT8)packGridFinal[tempCnt];
    }

    packBitLen = Tiff6_PackBits(packGridFinalInt, packGridLen, (UINT8PTR)packBytes);
    packBytes[packBitLen] = '\0';

    /* encode into base64 */
    encodedStrPtr = EncodeBase64(packBytes, packBitLen);
    if (encodedStrPtr == NULL)
    {
        EPRINT(ONVIF_CLIENT, "error while encoding base64");
        return FAIL;
    }

    snprintf(activeCellStr, activeCellLen, "%s", encodedStrPtr);
    FREE_MEMORY(encodedStrPtr);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief OnvifUnconfigureCamReq    : used by autoconfig feature
 * @param ipAddress                 : IP
 * @param port                      : ONVIF port
 * @param usrName                   : User name
 * @param password                  : Password
 * @param reqType                   : can be  change Ip, get brand model & set password
 * @param data                      : data
 * @return
 */
ONVIF_RESP_STATUS_e OnvifUnconfigureCamReq(CHARPTR ipAddress, UINT16 port, CHARPTR usrName, CHARPTR password, ONVIF_REQUEST_e reqType, VOIDPTR data)
{
    SOAP_USER_DETAIL_t    				user;
    SOAP_t                              *soap;
    CHAR								deviceEntryPoint[MAX_SERVICE_ADDR_LEN];
    ONVIF_RESP_STATUS_e 				status = ONVIF_CMD_SUCCESS;
    INT16  								soapResp = SOAP_OK;
    GET_SYSTEM_DATE_AND_TIME_t 			getSystemDateAndTime;
    GET_SYSTEM_DATE_AND_TIME_RESPONSE_t getSystemDateAndTimeResponse;
    time_t 								t = time(NULL);
    TIME_DIFF_t				   			timeDifference;
    struct tm 							*tm = gmtime(&t);
    CHAR                                ipAddrForUrl[DOMAIN_NAME_SIZE_MAX];
    CHAR 		  		  				brandName[MAX_BRAND_NAME_LEN];
    CHAR 		  		  				modelName[MAX_MODEL_NAME_LEN];
    UINT8								cameraIndex = INVALID_CAMERA_INDEX;

    if (tm == NULL)
    {
        EPRINT(ONVIF_CLIENT, "fail to get system time: [ip=%s], [port=%d], [reqType=%d]", ipAddress, port, reqType);
        return ONVIF_CMD_FAIL;
    }

    snprintf(user.name, sizeof(user.name), "%s", usrName);
    snprintf(user.pwd, sizeof(user.pwd), "%s", password);
    snprintf(user.ipAddr, sizeof(user.ipAddr), "%s", ipAddress);
    user.port = port;
    PrepareIpAddressForUrl(ipAddress, ipAddrForUrl);
    snprintf(deviceEntryPoint, MAX_SERVICE_ADDR_LEN, HTTP_REQUEST_URL, ipAddrForUrl, user.port, "/onvif/device_service");
    user.addr = deviceEntryPoint;
    user.timeDiff = NULL;

    soap = soap_new();
    soapResp = GetSystemDateAndTime(soap, &getSystemDateAndTime, &getSystemDateAndTimeResponse, &user);
    if ((soapResp != SOAP_OK) || (getSystemDateAndTimeResponse.SystemDateAndTime == NULL))
    {
        /* Free soap object memory */
        freeSoapObject(soap);
        EPRINT(ONVIF_CLIENT, "fail to get date-time: [ip=%s], [port=%d], [reqType=%d], [soapResp=%d]", ipAddress, port, reqType, soapResp);
        if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
        {
            return ONVIF_CMD_TIMEOUT;
        }

        return ONVIF_CMD_FAIL;
    }

    /* Set camera time for new request */
    timeDifference.year = (getSystemDateAndTimeResponse.SystemDateAndTime->UTCDateTime->Date->Year - (START_YEAR + tm->tm_year));
    timeDifference.month = (getSystemDateAndTimeResponse.SystemDateAndTime->UTCDateTime->Date->Month - (tm->tm_mon + 1));
    timeDifference.day = (getSystemDateAndTimeResponse.SystemDateAndTime->UTCDateTime->Date->Day - tm->tm_mday);
    timeDifference.hour	= (getSystemDateAndTimeResponse.SystemDateAndTime->UTCDateTime->Time->Hour - tm->tm_hour);
    timeDifference.minute = (getSystemDateAndTimeResponse.SystemDateAndTime->UTCDateTime->Time->Minute - tm->tm_min);
    timeDifference.seconds = (getSystemDateAndTimeResponse.SystemDateAndTime->UTCDateTime->Time->Second - tm->tm_sec);
    user.timeDiff = &timeDifference;

    // serve the request
    switch(reqType)
    {
        case ONVIF_CHANGE_IP_ADDR:
        {
            status = changeOnvifIpCameraAddress(soap, &user, INVALID_CAMERA_INDEX, (IP_ADDR_PARAM_t *)data);
        }
        break;

        case ONVIF_GET_BRAND_MODEL:
        {
            status = getBrandModel(soap, &user, brandName, modelName, cameraIndex);
        }
        break;

        case ONVIF_SET_PASSWORD:
        {
            status = setUserPassword(soap, &user);
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }

    /* Free soap object memory */
    freeSoapObject(soap);
    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief parseMedia2FrameRate  : find min and max interger from string
 * @param framrate              : string of intergers
 * @param min                   : min value present in string
 * @param max                   : max value present in string
 * @return
 */
static BOOL parseMedia2FrameRate(CHARPTR framrate, UINT8PTR min, UINT8PTR max)
{
    CHARPTR ptr = framrate;

    *min =  200;
    *max =  0;

    while(*ptr)
    {
        if(isdigit(*ptr))
        {
            errno = 0;
            double val = strtod(ptr, &ptr);
            if(0 != errno)
            {
                return FAIL;
            }

            if(val > *max)
            {
                *max = val;
            }
            else if(val < *min)
            {
                *min = val;
            }
        }
        else
        {
            ptr++;
        }
    }

    /* Verify failure case */
    if ((*max == 0) || (*min == 200))
    {
        *max = 0;
        *min = 0;
        return FAIL;
    }

    if (*min == 0)
    {
        *min = 1;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief parseEventTag : Parse all event tag present in dom tree
 * @param eventTag      : required tag
 * @param root          : dom tree genrated from xml
 * @param len           : required to append string
 * @return
 */
static BOOL parseEventTag(CHARPTR eventTag, struct soap_dom_element *dom, UINT8 len)
{
    CHAR                        topic[MAX_TOPIC_NAME_SPACE_LEN];
    struct soap_dom_element     *temp = dom;
    const CHAR                  *tagName = NULL;

    if ((eventTag == NULL) || (dom == NULL))
    {
        return FAIL;
    }

    while(temp)
    {
        tagName = soap_elt_get_tag(temp);
        if (tagName)
        {
            if (strstr(tagName, "MessageDescription"))
            {
                break;
            }

            memset(topic, '\0', MAX_TOPIC_NAME_SPACE_LEN);
            removeNameSpace(tagName, topic);
            len += snprintf(eventTag + len, MAX_TOPIC_NAME_SPACE_LEN, "/%s", topic);
        }
        temp = soap_elt_first(temp);
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief removeNameSpace : remove namespace tag  from string
 * @param string eg       : tns1:RuleEngine
 * @param topic  output   : RuleEngine
 * @return
 */
static BOOL removeNameSpace(const CHAR *string, CHARPTR topic)
{
    CHARPTR     endStr = NULL;
    const CHAR  *startStr= NULL;

    if ((string == NULL) || (topic == NULL))
    {
        return FAIL;
    }

    startStr = string;
    if ((endStr = strchr(startStr, ':')) != NULL)
    {
        startStr = endStr + 1;
    }

    snprintf(topic, MAX_TOPIC_NAME_SPACE_LEN, "%s", startStr);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief getValue  : Get attribute value corresponding to given Tag
 * @param message   : Input XML data
 * @param data      : Result
 * @param tag       : tag value
 * @return
 */
static BOOL getValue(CHARPTR message, CHARPTR data, CHARPTR tag)
{
    CHARPTR startStr;

    if ((message == NULL) || (tag == NULL))
    {
        return FAIL;
    }

    //Find tag specified in argument/ it will be whether Source or Data
    startStr = strstr(message, tag);
    if (startStr == NULL)
    {
        return FAIL;
    }

    startStr = startStr + strlen(tag);
    startStr = strstr(startStr, "Value=");
    if (startStr == NULL)
    {
        return FAIL;
    }

    startStr = startStr + strlen("Value=");

    //Leave the blank
    while ((*startStr == '"') || (*startStr == ' '))
    {
        startStr++ ;
    }

    //Now Get the Value
    do
    {
        *data++ = *startStr++;
    }
    while((*startStr != '"') && (*startStr != ' '));

    *data = '\0';

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief tagSequenceCompare    : Compare if two tag match or not
 * @param evtRecvTag            : Tag1
 * @param storeTag              : Tag2
 * @return
 */
static BOOL tagSequenceCompare(CHARPTR evtRecvTag, CHARPTR storeTag)
{
    BOOL    status = FAIL, continueLoop;
    UINT32  cnt;

    while(1)
    {
        cnt = 0;

        do
        {
            if ((*(evtRecvTag + cnt) == '/') || (*(evtRecvTag + cnt) == '\0'))
            {
                break;
            }
            else if(*(evtRecvTag + cnt) == ':')
            {
                evtRecvTag += (cnt + 1);
                break;
            }
            else
            {
                cnt++;
            }
        }
        while(1);

        continueLoop = NO;

        while(*evtRecvTag == *storeTag)
        {
            //If any of tag terminates with NULL then return
            if ((*evtRecvTag == '\0') || (*storeTag == '\0'))
            {
                status = SUCCESS; //Tag Found
                break;
            }
            else if((*evtRecvTag == '/') && (*storeTag == '/'))
            {
                continueLoop = YES;
                evtRecvTag++;
                storeTag++;
                break;
            }
            evtRecvTag++;
            storeTag++;
        }

        if (continueLoop == NO)
        {
            break;
        }
    }
    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief getProperty - This function will search the values of propertyOperation.
                     eg. <Message propertyOperation="Initilized"> or <Message PropertyOperation="Changed">
 * @param message
 * @param data
  * @return
 */
BOOL getProperty(CHARPTR message,CHARPTR data)
{
    CHARPTR startStr;

    if ((message == NULL))
    {
        return FAIL;
    }

    //Find tag PropertyOperation
    startStr = strstr(message, "PropertyOperation=");
    if (startStr == NULL)
    {
        return FAIL;
    }

    startStr = startStr + strlen("PropertyOperation=");
    if (startStr == NULL)
    {
        return FAIL;
    }

    //Leave the blank
    while((*startStr == '"') || (*startStr == ' '))
    {
        startStr++ ;
    }

    //Now Get the Value
    do
    {
        *data++ = *startStr++;
    }
    while((*startStr != '"') && (*startStr != ' '));

    *data = '\0';

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief freeSoapObject
 * @param soap
 */
static void freeSoapObject(SOAP_t *soap)
{
    soap_destroy(soap); // delete managed C++ instances
    soap_end(soap);     // delete managed memory
    soap_free(soap);    // free the context
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   GetOnvifReqName
 * @param   onvifReq
 * @return
 */
const CHARPTR GetOnvifReqName(ONVIF_REQUEST_e onvifReq)
{
#define SKIP_PREFIX_SIZE (sizeof("ONVIF_")-1)
    switch(onvifReq)
    {
        CASE_TO_STR(ONVIF_GET_DATE_TIME, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_GET_ENTRY_POINT, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_GET_BRAND_MODEL, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_GET_CAPABILITY, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_SET_DATE_TIME, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_GET_MEDIA_URL, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_GET_SNAPSHOT_URL, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_SET_PTZ, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_EDIT_PRESET, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_GOTO_PRESET, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_SET_ALARM, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_SET_FOCUS, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_SET_IRIS, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_RETRY_REQUEST, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_START_EVENT, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_UNSUBSCRIBE_EVENT_REQ, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_GET_EVENT_NOTIFICATION, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_RENEW_EVENT_REQ, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_SEARCH_DEVICES, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_OSD_SET_REQ, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_PROFILE_CONFIG, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_GET_MOTION_WINDOW, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_SET_MOTION_WINDOW, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_CHANGE_IP_ADDR, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_MEDIA_PROFILES_UPDATE, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_SET_PASSWORD, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_GET_IMAGING_CAPABILITY, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_GET_IMAGING_SETTING, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_SET_IMAGING_SETTING, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_REQUEST_MAX, SKIP_PREFIX_SIZE);
    }
#undef SKIP_PREFIX_SIZE
    return ("UNKNOWN");
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   GetOnvifReqStatusName
 * @param   onvifReqStatus
 * @return
 */
static const CHARPTR GetOnvifRespName(ONVIF_RESP_STATUS_e onvifReqStatus)
{
#define SKIP_PREFIX_SIZE (sizeof("ONVIF_CMD_")-1)
    switch(onvifReqStatus)
    {
        CASE_TO_STR(ONVIF_CMD_SUCCESS, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_CMD_FAIL, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_CMD_TIMEOUT, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_CMD_RETRY, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_CMD_FEATURE_NOT_SUPPORTED, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_CMD_IP_CHANGE_FAIL, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_CMD_AUTHENTICATION_FAIL, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_CMD_CAMERA_REBOOT_FAIL, SKIP_PREFIX_SIZE);
        CASE_TO_STR(ONVIF_CMD_RESP_STATUS_MAX, SKIP_PREFIX_SIZE);
    }
#undef SKIP_PREFIX_SIZE
    return ("UNKNOWN");
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief getImagingCapability      : Get camera imaging capability
 * @param soap                      : SOAP instance
 * @param user                      : user information
 * @param camIndex                  : Camera Index
 * @param pImageCapsInfo            : Image setting capability copy of camera interface
 * @return
 */
static ONVIF_RESP_STATUS_e getImagingCapability(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 camIndex, IMAGE_CAPABILITY_INFO_t *pImgCapsInfo)
{
    GET_OPTIONS_t           getOptions;
    GET_OPTIONS_RESPONSE_t  getOptionsResponse;
    IMAGING_OPTIONS20_t     *pImagingOptions;
    INT16                   soapResp;
    UINT8                   loop;

    /* Nothing to do if camera doesn't support imaging settings */
    if (NO == onvifCamInfo[camIndex].imagingServiceInfo.isImgSettingSupport)
    {
        EPRINT(ONVIF_CLIENT, "camera doesn't support imaging setting: [camera=%d]", camIndex);
        return ONVIF_CMD_FEATURE_NOT_SUPPORTED;
    }

    //--------------------------------------------------------------------------------
    // Get Options (Imaging Options)
    //--------------------------------------------------------------------------------
    getCameraDateTime(soap, camIndex, user);
    user->addr = onvifCamInfo[camIndex].imagingServiceInfo.imagingServiceAddr;
    getOptions.VideoSourceToken = onvifCamInfo[camIndex].imagingServiceInfo.videoSourceToken;

    /* Send get image setting capability command to camera */
    soapResp = GetOptions(soap, &getOptions, &getOptionsResponse, user);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "fail to get options: [camera=%d], [soapResp=%d]", camIndex, soapResp);
        if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
        {
            return ONVIF_CMD_TIMEOUT;
        }

        return ONVIF_CMD_FAIL;
    }

    /* Validate whether imaging options */
    if (getOptionsResponse.ImagingOptions == NULL)
    {
        EPRINT(ONVIF_CLIENT, "get options is not available: [camera=%d]", camIndex);
        return ONVIF_CMD_FAIL;
    }

    /* Get imaging options locally */
    pImagingOptions = getOptionsResponse.ImagingOptions;

    //--------------------------------------------------------------------------------
    // Get Brightness Property
    //--------------------------------------------------------------------------------
    if (pImagingOptions->Brightness != NULL)
    {
        if (IS_VALID_IMAGE_SETTING_RANGE(pImagingOptions->Brightness->Min, pImagingOptions->Brightness->Max))
        {
            pImgCapsInfo->imagingCapability |= MX_ADD(IMG_SETTING_BRIGHTNESS);
            pImgCapsInfo->brightness.step = GET_IMAGE_SETTING_STEP_VALUE(pImagingOptions->Brightness->Min, pImagingOptions->Brightness->Max, BRIGHTNESS_MAX);
            pImgCapsInfo->brightness.min = pImagingOptions->Brightness->Min;
            pImgCapsInfo->brightness.max = pImagingOptions->Brightness->Max;
        }
    }

    //--------------------------------------------------------------------------------
    // Get Contrast Property
    //--------------------------------------------------------------------------------
    if (pImagingOptions->Contrast != NULL)
    {
        if (TRUE == IS_VALID_IMAGE_SETTING_RANGE(pImagingOptions->Contrast->Min, pImagingOptions->Contrast->Max))
        {
            pImgCapsInfo->imagingCapability |= MX_ADD(IMG_SETTING_CONTRAST);
            pImgCapsInfo->contrast.step = GET_IMAGE_SETTING_STEP_VALUE(pImagingOptions->Contrast->Min, pImagingOptions->Contrast->Max, CONTRAST_MAX);
            pImgCapsInfo->contrast.min = pImagingOptions->Contrast->Min;
            pImgCapsInfo->contrast.max = pImagingOptions->Contrast->Max;
        }
    }

    //--------------------------------------------------------------------------------
    // Get Saturation Property
    //--------------------------------------------------------------------------------
    if (pImagingOptions->ColorSaturation != NULL)
    {
        if (TRUE == IS_VALID_IMAGE_SETTING_RANGE(pImagingOptions->ColorSaturation->Min, pImagingOptions->ColorSaturation->Max))
        {
            pImgCapsInfo->imagingCapability |= MX_ADD(IMG_SETTING_SATURATION);
            pImgCapsInfo->saturation.step = GET_IMAGE_SETTING_STEP_VALUE(pImagingOptions->ColorSaturation->Min, pImagingOptions->ColorSaturation->Max, SATURATION_MAX);
            pImgCapsInfo->saturation.min = pImagingOptions->ColorSaturation->Min;
            pImgCapsInfo->saturation.max = pImagingOptions->ColorSaturation->Max;
        }
    }

    //--------------------------------------------------------------------------------
    // Get Sharpness Property
    //--------------------------------------------------------------------------------
    if (pImagingOptions->Sharpness != NULL)
    {
        if (TRUE == IS_VALID_IMAGE_SETTING_RANGE(pImagingOptions->Sharpness->Min, pImagingOptions->Sharpness->Max))
        {
            pImgCapsInfo->imagingCapability |= MX_ADD(IMG_SETTING_SHARPNESS);
            pImgCapsInfo->sharpness.step = GET_IMAGE_SETTING_STEP_VALUE(pImagingOptions->Sharpness->Min, pImagingOptions->Sharpness->Max, SHARPNESS_MAX);
            pImgCapsInfo->sharpness.min = pImagingOptions->Sharpness->Min;
            pImgCapsInfo->sharpness.max = pImagingOptions->Sharpness->Max;
        }
    }

    //--------------------------------------------------------------------------------
    // Get White Balance Property
    //--------------------------------------------------------------------------------
    if (pImagingOptions->WhiteBalance != NULL)
    {
        if (pImagingOptions->WhiteBalance->__sizeMode > 0)
        {
            for (loop = 0; loop < pImagingOptions->WhiteBalance->__sizeMode; loop++)
            {
                if ((pImagingOptions->WhiteBalance->Mode[loop] == tt__WhiteBalanceMode__AUTO) ||
                        (pImagingOptions->WhiteBalance->Mode[loop] == tt__WhiteBalanceMode__MANUAL))
                {
                    pImgCapsInfo->whiteBalance.modeSupported |= MX_ADD(pImagingOptions->WhiteBalance->Mode[loop]);
                }
            }

            if (pImgCapsInfo->whiteBalance.modeSupported)
            {
                pImgCapsInfo->imagingCapability |= MX_ADD(IMG_SETTING_WHITE_BALANCE);
            }
        }
    }

    //--------------------------------------------------------------------------------
    // Get WDR Property
    //--------------------------------------------------------------------------------
    if (pImagingOptions->WideDynamicRange != NULL)
    {
        if (pImagingOptions->WideDynamicRange->__sizeMode > 0)
        {
            for (loop = 0; loop < pImagingOptions->WideDynamicRange->__sizeMode; loop++)
            {
                if ((pImagingOptions->WideDynamicRange->Mode[loop] == tt__WideDynamicMode__OFF) ||
                        (pImagingOptions->WideDynamicRange->Mode[loop] == tt__WideDynamicMode__ON))
                {
                    pImgCapsInfo->wdr.modeSupported |= MX_ADD(pImagingOptions->WideDynamicRange->Mode[loop]);
                }
            }

            if (pImgCapsInfo->wdr.modeSupported)
            {
                pImgCapsInfo->imagingCapability |= MX_ADD(IMG_SETTING_WDR_MODE);
            }
        }

        if (pImagingOptions->WideDynamicRange->Level != NULL)
        {
            if (TRUE == IS_VALID_IMAGE_SETTING_RANGE(pImagingOptions->WideDynamicRange->Level->Min, pImagingOptions->WideDynamicRange->Level->Max))
            {
                pImgCapsInfo->imagingCapability |= MX_ADD(IMG_SETTING_WDR_STRENGTH);
                pImgCapsInfo->wdrStrength.step = GET_IMAGE_SETTING_STEP_VALUE(pImagingOptions->WideDynamicRange->Level->Min, pImagingOptions->WideDynamicRange->Level->Max, (WDR_STRENGTH_MAX - WDR_STRENGTH_MIN));
                pImgCapsInfo->wdrStrength.min = pImagingOptions->WideDynamicRange->Level->Min;
                pImgCapsInfo->wdrStrength.max = pImagingOptions->WideDynamicRange->Level->Max;
            }
        }
    }

    //--------------------------------------------------------------------------------
    // Get Backlight Control Property
    //--------------------------------------------------------------------------------
    if (pImagingOptions->BacklightCompensation != NULL)
    {
        if (pImagingOptions->BacklightCompensation->__sizeMode > 0)
        {
            for (loop = 0; loop < pImagingOptions->BacklightCompensation->__sizeMode; loop++)
            {
                if ((pImagingOptions->BacklightCompensation->Mode[loop] == tt__BacklightCompensationMode__OFF) ||
                        (pImagingOptions->BacklightCompensation->Mode[loop] == tt__BacklightCompensationMode__ON))
                {
                    pImgCapsInfo->backlightControl.modeSupported |= MX_ADD(pImagingOptions->BacklightCompensation->Mode[loop]);
                }
            }

            if (pImgCapsInfo->backlightControl.modeSupported)
            {
                pImgCapsInfo->imagingCapability |= MX_ADD(IMG_SETTING_BACKLIGHT);
            }
        }
    }

    //--------------------------------------------------------------------------------
    // Get LED Mode Property
    //--------------------------------------------------------------------------------
    if (pImagingOptions->IrCutFilterModes != NULL)
    {
        if (pImagingOptions->__sizeIrCutFilterModes > 0)
        {
            for (loop = 0; loop < pImagingOptions->__sizeIrCutFilterModes; loop++)
            {
                if ((pImagingOptions->IrCutFilterModes[loop] >= tt__IrCutFilterMode__ON) &&
                        (pImagingOptions->IrCutFilterModes[loop] <= tt__IrCutFilterMode__AUTO))
                {
                    pImgCapsInfo->irLed.modeSupported |= MX_ADD(OnvifIrLedModesMap[pImagingOptions->IrCutFilterModes[loop]][0]);
                }
            }

            if (pImgCapsInfo->irLed.modeSupported)
            {
                pImgCapsInfo->imagingCapability |= MX_ADD(IMG_SETTING_LED_MODE);
            }
        }
    }

    DPRINT(ONVIF_CLIENT, "imaging setting capability: [camera=%d], [mask=0x%X]", camIndex, pImgCapsInfo->imagingCapability);
    return ONVIF_CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief getImagingSetting         : Get camera imaging config
 * @param soap                      : SOAP instance
 * @param user                      : user information
 * @param camIndex                  : Camera Index
 * @param pImageCapsInfo            : Image setting capability copy of camera interface
 * @return
 */
static ONVIF_RESP_STATUS_e getImagingSetting(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 camIndex, IMAGE_CAPABILITY_INFO_t *pImgCapsInfo)
{
    GET_IMAGING_SETTINGS_t 			getImagingSettings;
    GET_IMAGING_SETTINGS_RESPONSE_t getImagingSettingsResponse;
    IMAGING_SETTINGS20_t            *pImagingSettings;
    INT16   						soapResp;
    BOOL                            wdrStrengthFound = FALSE, dwdrStrengthFound = FALSE;
    float                           wdrStrength = 0.0f, dwdrStrength = 0.0f;

    /* Get the time difference between NVR and camera */
    getCameraDateTime(soap, camIndex, user);
    user->addr = onvifCamInfo[camIndex].imagingServiceInfo.imagingServiceAddr;
    getImagingSettings.VideoSourceToken = onvifCamInfo[camIndex].imagingServiceInfo.videoSourceToken;

    /* Send get image setting parameters command to camera */
    soapResp = GetImagingSettings(soap, &getImagingSettings, &getImagingSettingsResponse, user);
    if((soapResp != SOAP_OK) || (getImagingSettingsResponse.ImagingSettings == NULL))
    {
        EPRINT(ONVIF_CLIENT, "fail to get imaging settings: [camera=%d], [serviceAddr=%s], [soapResp=%d]", camIndex, user->addr, soapResp);
        if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
        {
            return ONVIF_CMD_TIMEOUT;
        }

        return ONVIF_CMD_FAIL;
    }

    /* Get imaging settings locally */
    pImagingSettings = getImagingSettingsResponse.ImagingSettings;

    //--------------------------------------------------------------------------------
    // Get Brightness Value
    //--------------------------------------------------------------------------------
    if (GET_BIT(pImgCapsInfo->imagingCapability, IMG_SETTING_BRIGHTNESS))
    {
        if (pImagingSettings->Brightness == NULL)
        {
            /* We got capability but didn't receive value */
            RESET_BIT(pImgCapsInfo->imagingCapability, IMG_SETTING_BRIGHTNESS);
        }
        else
        {
            /* Get bightness value */
            pImgCapsInfo->brightness.value = GET_IMAGE_SETTING_VALUE(*pImagingSettings->Brightness, pImgCapsInfo->brightness.step,
                                                                     pImgCapsInfo->brightness.min, pImgCapsInfo->brightness.max, BRIGHTNESS_MIN);
        }
    }

    //--------------------------------------------------------------------------------
    // Get Contrast Value
    //--------------------------------------------------------------------------------
    if (GET_BIT(pImgCapsInfo->imagingCapability, IMG_SETTING_CONTRAST))
    {
        if (pImagingSettings->Contrast == NULL)
        {
            /* We got capability but didn't receive value */
            RESET_BIT(pImgCapsInfo->imagingCapability, IMG_SETTING_CONTRAST);
        }
        else
        {
            /* Get contrast value */
            pImgCapsInfo->contrast.value = GET_IMAGE_SETTING_VALUE(*pImagingSettings->Contrast, pImgCapsInfo->contrast.step,
                                                                   pImgCapsInfo->contrast.min, pImgCapsInfo->contrast.max, CONTRAST_MIN);
        }
    }

    //--------------------------------------------------------------------------------
    // Get Saturation Value
    //--------------------------------------------------------------------------------
    if (GET_BIT(pImgCapsInfo->imagingCapability, IMG_SETTING_SATURATION))
    {
        if (pImagingSettings->ColorSaturation == NULL)
        {
            /* We got capability but didn't receive value */
            RESET_BIT(pImgCapsInfo->imagingCapability, IMG_SETTING_SATURATION);
        }
        else
        {
            /* Get saturation value */
            pImgCapsInfo->saturation.value = GET_IMAGE_SETTING_VALUE(*pImagingSettings->ColorSaturation, pImgCapsInfo->saturation.step,
                                                                     pImgCapsInfo->saturation.min, pImgCapsInfo->saturation.max, SATURATION_MIN);
        }
    }

    //--------------------------------------------------------------------------------
    // Get Sharpness Value
    //--------------------------------------------------------------------------------
    if (GET_BIT(pImgCapsInfo->imagingCapability, IMG_SETTING_SHARPNESS))
    {
        if (pImagingSettings->Sharpness == NULL)
        {
            /* We got capability but didn't receive value */
            RESET_BIT(pImgCapsInfo->imagingCapability, IMG_SETTING_SHARPNESS);
        }
        else
        {
            /* Get saturation value */
            pImgCapsInfo->sharpness.value = GET_IMAGE_SETTING_VALUE(*pImagingSettings->Sharpness, pImgCapsInfo->sharpness.step,
                                                                    pImgCapsInfo->sharpness.min, pImgCapsInfo->sharpness.max, SHARPNESS_MIN);
        }
    }

    //--------------------------------------------------------------------------------
    // Get White Balance Value
    //--------------------------------------------------------------------------------
    if (GET_BIT(pImgCapsInfo->imagingCapability, IMG_SETTING_WHITE_BALANCE))
    {
        if (pImagingSettings->WhiteBalance == NULL)
        {
            /* We got capability but didn't receive value */
            RESET_BIT(pImgCapsInfo->imagingCapability, IMG_SETTING_WHITE_BALANCE);
        }
        else
        {
            /* Get white balance value */
            pImgCapsInfo->whiteBalance.mode = WHITE_BALANCE_MODE_AUTO;
            if (GET_BIT(pImgCapsInfo->whiteBalance.modeSupported, pImagingSettings->WhiteBalance->Mode))
            {
                pImgCapsInfo->whiteBalance.mode = pImagingSettings->WhiteBalance->Mode;
            }
        }
    }

    //--------------------------------------------------------------------------------
    // Get WDR Value
    //--------------------------------------------------------------------------------
    if (GET_BIT(pImgCapsInfo->imagingCapability, IMG_SETTING_WDR_MODE))
    {
        if (pImagingSettings->WideDynamicRange == NULL)
        {
            /* We got capability but didn't receive value */
            RESET_BIT(pImgCapsInfo->imagingCapability, IMG_SETTING_WDR_MODE);
        }
        else
        {
            /* Get wdr value */
            pImgCapsInfo->wdr.mode = WDR_MODE_OFF;
            if (GET_BIT(pImgCapsInfo->wdr.modeSupported, pImagingSettings->WideDynamicRange->Mode))
            {
                pImgCapsInfo->wdr.mode = pImagingSettings->WideDynamicRange->Mode;
            }

            /* Get wdr strength value */
            if (GET_BIT(pImgCapsInfo->imagingCapability, IMG_SETTING_WDR_STRENGTH))
            {
                if (pImagingSettings->WideDynamicRange->Level != NULL)
                {
                    wdrStrength = *pImagingSettings->WideDynamicRange->Level;
                    wdrStrengthFound = TRUE;
                }
            }
        }
    }

    //--------------------------------------------------------------------------------
    // Get Backlight Control Value
    //--------------------------------------------------------------------------------
    if (GET_BIT(pImgCapsInfo->imagingCapability, IMG_SETTING_BACKLIGHT))
    {
        if (pImagingSettings->BacklightCompensation == NULL)
        {
            /* We got capability but didn't receive value */
            RESET_BIT(pImgCapsInfo->imagingCapability, IMG_SETTING_BACKLIGHT);
        }
        else
        {
            /* Get backlight value */
            pImgCapsInfo->backlightControl.mode = BACKLIGHT_MODE_OFF;
            if (GET_BIT(pImgCapsInfo->backlightControl.modeSupported, pImagingSettings->BacklightCompensation->Mode))
            {
                pImgCapsInfo->backlightControl.mode = pImagingSettings->BacklightCompensation->Mode;
            }

            /* Get dwdr strength value */
            if (GET_BIT(pImgCapsInfo->imagingCapability, IMG_SETTING_WDR_STRENGTH))
            {
                if (pImagingSettings->BacklightCompensation->Level != NULL)
                {
                    dwdrStrength = *pImagingSettings->BacklightCompensation->Level;
                    dwdrStrengthFound = TRUE;
                }
            }
        }
    }

    //--------------------------------------------------------------------------------
    // Get WDR Strength Value
    //--------------------------------------------------------------------------------
    if (((wdrStrengthFound == TRUE) || (dwdrStrengthFound == TRUE)) && (GET_BIT(pImgCapsInfo->imagingCapability, IMG_SETTING_WDR_STRENGTH)))
    {
        if ((GET_BIT(pImgCapsInfo->imagingCapability, IMG_SETTING_BACKLIGHT)) && (dwdrStrengthFound == TRUE) &&
                (pImgCapsInfo->wdr.mode == WDR_MODE_OFF) && (pImgCapsInfo->backlightControl.mode != BACKLIGHT_MODE_OFF))
        {
            pImgCapsInfo->wdrStrength.value = GET_IMAGE_SETTING_VALUE(dwdrStrength, pImgCapsInfo->wdrStrength.step,
                                                                      pImgCapsInfo->wdrStrength.min, pImgCapsInfo->wdrStrength.max, WDR_STRENGTH_MIN);
        }
        else if ((GET_BIT(pImgCapsInfo->imagingCapability, IMG_SETTING_WDR_MODE)) && (wdrStrengthFound == TRUE))
        {
            pImgCapsInfo->wdrStrength.value = GET_IMAGE_SETTING_VALUE(wdrStrength, pImgCapsInfo->wdrStrength.step,
                                                                      pImgCapsInfo->wdrStrength.min, pImgCapsInfo->wdrStrength.max, WDR_STRENGTH_MIN);
        }
        else
        {
            /* We got capability but didn't receive value */
            RESET_BIT(pImgCapsInfo->imagingCapability, IMG_SETTING_WDR_STRENGTH);
        }
    }
    else
    {
        /* We got capability but didn't receive value */
        RESET_BIT(pImgCapsInfo->imagingCapability, IMG_SETTING_WDR_STRENGTH);
    }

    //--------------------------------------------------------------------------------
    // Get LED Mode Value
    //--------------------------------------------------------------------------------
    if (GET_BIT(pImgCapsInfo->imagingCapability, IMG_SETTING_LED_MODE))
    {
        if (pImagingSettings->IrCutFilter == NULL)
        {
            /* We got capability but didn't receive value */
            RESET_BIT(pImgCapsInfo->imagingCapability, IMG_SETTING_LED_MODE);
        }
        else
        {
            /* Get ir-led value */
            pImgCapsInfo->irLed.mode = LED_MODE_AUTO;
            if (GET_BIT(pImgCapsInfo->irLed.modeSupported, *pImagingSettings->IrCutFilter))
            {
                pImgCapsInfo->irLed.mode = OnvifIrLedModesMap[*pImagingSettings->IrCutFilter][0];
            }
        }
    }

    return ONVIF_CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief setImagingSetting         : Set imaging config to camera
 * @param soap                      : SOAP instance
 * @param user                      : user information
 * @param camIndex                  : Camera Index
 * @param pImageCapsInfo            : Image setting capability copy of camera interface
 * @return
 */
static ONVIF_RESP_STATUS_e setImagingSetting(SOAP_t *soap, SOAP_USER_DETAIL_t *user, UINT8 camIndex, IMAGE_CAPABILITY_INFO_t *pImgCapsInfo)
{
    GET_IMAGING_SETTINGS_t 			getImagingSettings;
    GET_IMAGING_SETTINGS_RESPONSE_t getImagingSettingsResponse;
    SET_IMAGING_SETTINGS_t 			setImagingSettings;
    SET_IMAGING_SETTINGS_RESPONSE_t setImagingSettingsResponse;
    IMAGING_SETTINGS20_t            *pImagingSettings;
    INT16   						soapResp;

    /* Get the time difference between NVR and Camera */
    getCameraDateTime(soap, camIndex, user);
    user->addr = onvifCamInfo[camIndex].imagingServiceInfo.imagingServiceAddr;
    getImagingSettings.VideoSourceToken = onvifCamInfo[camIndex].imagingServiceInfo.videoSourceToken;

    //--------------------------------------------------------------------------------
    // Get Imaging settings
    //--------------------------------------------------------------------------------
    soapResp = GetImagingSettings(soap, &getImagingSettings, &getImagingSettingsResponse, user);
    if((soapResp != SOAP_OK) || (getImagingSettingsResponse.ImagingSettings == NULL))
    {
        EPRINT(ONVIF_CLIENT, "fail to get imaging settings: [camera=%d], [serviceAddr=%s], [soapResp=%d]", camIndex, user->addr, soapResp);
        if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
        {
            return ONVIF_CMD_TIMEOUT;
        }

        return ONVIF_CMD_FAIL;
    }

    /* set video source token for imaging service request */
    setImagingSettings.VideoSourceToken = onvifCamInfo[camIndex].imagingServiceInfo.videoSourceToken;
    setImagingSettings.ForcePersistence = FALSE;

    /* copy recevied image settings to keep other parameter intact */
    pImagingSettings = getImagingSettingsResponse.ImagingSettings;
    setImagingSettings.ImagingSettings = pImagingSettings;

    //--------------------------------------------------------------------------------
    // Set Brightness Value
    //--------------------------------------------------------------------------------
    if (GET_BIT(pImgCapsInfo->imagingCapability, IMG_SETTING_BRIGHTNESS) && (pImagingSettings->Brightness != NULL))
    {
        *pImagingSettings->Brightness = SET_IMAGE_SETTING_VALUE(pImgCapsInfo->brightness.value, pImgCapsInfo->brightness.step, pImgCapsInfo->brightness.min, BRIGHTNESS_MIN);
    }

    //--------------------------------------------------------------------------------
    // Set Contrast Value
    //--------------------------------------------------------------------------------
    if (GET_BIT(pImgCapsInfo->imagingCapability, IMG_SETTING_CONTRAST) && (pImagingSettings->Contrast != NULL))
    {
        *pImagingSettings->Contrast = SET_IMAGE_SETTING_VALUE(pImgCapsInfo->contrast.value, pImgCapsInfo->contrast.step, pImgCapsInfo->contrast.min, CONTRAST_MIN);
    }

    //--------------------------------------------------------------------------------
    // Set Saturation Value
    //--------------------------------------------------------------------------------
    if (GET_BIT(pImgCapsInfo->imagingCapability, IMG_SETTING_SATURATION) && (pImagingSettings->ColorSaturation != NULL))
    {
        *pImagingSettings->ColorSaturation = SET_IMAGE_SETTING_VALUE(pImgCapsInfo->saturation.value, pImgCapsInfo->saturation.step, pImgCapsInfo->saturation.min, SATURATION_MIN);
    }

    //--------------------------------------------------------------------------------
    // Set Sharpness Value
    //--------------------------------------------------------------------------------
    if (GET_BIT(pImgCapsInfo->imagingCapability, IMG_SETTING_SHARPNESS) && (pImagingSettings->Sharpness != NULL))
    {
        *pImagingSettings->Sharpness = SET_IMAGE_SETTING_VALUE(pImgCapsInfo->sharpness.value, pImgCapsInfo->sharpness.step, pImgCapsInfo->sharpness.min, SHARPNESS_MIN);
    }

    //--------------------------------------------------------------------------------
    // Get White Balance Value
    //--------------------------------------------------------------------------------
    if (GET_BIT(pImgCapsInfo->imagingCapability, IMG_SETTING_WHITE_BALANCE) && (pImagingSettings->WhiteBalance != NULL))
    {
        pImagingSettings->WhiteBalance->Mode = pImgCapsInfo->whiteBalance.mode;
    }

    //--------------------------------------------------------------------------------
    // Set WDR Value
    //--------------------------------------------------------------------------------
    if (GET_BIT(pImgCapsInfo->imagingCapability, IMG_SETTING_WDR_MODE) && (pImagingSettings->WideDynamicRange != NULL))
    {
        pImagingSettings->WideDynamicRange->Mode = pImgCapsInfo->wdr.mode;
        if ((pImgCapsInfo->wdr.mode != WDR_MODE_OFF) && (pImagingSettings->WideDynamicRange->Level != NULL))
        {
            *pImagingSettings->WideDynamicRange->Level = SET_IMAGE_SETTING_VALUE(pImgCapsInfo->wdrStrength.value, pImgCapsInfo->wdrStrength.step, pImgCapsInfo->wdrStrength.min, WDR_STRENGTH_MIN);
        }
    }

    //--------------------------------------------------------------------------------
    // Set Backlight Control Value
    //--------------------------------------------------------------------------------
    if (GET_BIT(pImgCapsInfo->imagingCapability, IMG_SETTING_BACKLIGHT) && (pImagingSettings->BacklightCompensation != NULL))
    {
        pImagingSettings->BacklightCompensation->Mode = pImgCapsInfo->backlightControl.mode;
        if ((pImgCapsInfo->backlightControl.mode != BACKLIGHT_MODE_OFF) && (pImagingSettings->BacklightCompensation->Level != NULL))
        {
            *pImagingSettings->BacklightCompensation->Level = SET_IMAGE_SETTING_VALUE(pImgCapsInfo->wdrStrength.value, pImgCapsInfo->wdrStrength.step, pImgCapsInfo->wdrStrength.min, WDR_STRENGTH_MIN);
        }
    }

    //--------------------------------------------------------------------------------
    // Set LED Mode Value
    //--------------------------------------------------------------------------------
    if (GET_BIT(pImgCapsInfo->imagingCapability, IMG_SETTING_LED_MODE) && (pImagingSettings->IrCutFilter != NULL))
    {
        *pImagingSettings->IrCutFilter = OnvifIrLedModesMap[pImgCapsInfo->irLed.mode][1];
    }

    /* Send set image setting parameters command to camera */
    soapResp = SetImagingSettings(soap, &setImagingSettings, &setImagingSettingsResponse, user);
    if (soapResp != SOAP_OK)
    {
        EPRINT(ONVIF_CLIENT, "fail to set imaging settings: [camera=%d], [serviceAddr=%s], [soapResp=%d]", camIndex, user->addr, soapResp);
        if ((soapResp == SOAP_ERR) || (soapResp == SOAP_TCP_ERROR))
        {
            return ONVIF_CMD_TIMEOUT;
        }

        return ONVIF_CMD_FAIL;
    }

    return ONVIF_CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief	GetOnvifMedia2Support
 * @param	camIndex
 * @return	BOOL Media 2 Profile [Supported/NotSupported] by Camera
 */
BOOL GetOnvifMedia2Support(UINT8 camIndex)
{
    return onvifCamInfo[camIndex].mediaServiceInfo.media2Support;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief	ValidatePtzFuncForOnvif
 * @param	camIndex
 * @param   ptzFunction
 * @return	SUCCESS/FAIL
 */
BOOL ValidatePtzFuncForOnvif(UINT8 camIndex, PTZ_FUNCTION_e ptzFunction)
{
    switch (ptzFunction)
    {
        case PAN_TILT_FUNCTION:
            return ((onvifCamInfo[camIndex].ptzServiceInfo.panTiltSupport == NO_FUNCTION_SUPPORT) ||
                    (onvifCamInfo[camIndex].ptzServiceInfo.panTiltSupport >= MAX_PTZ_SUPPORT_OPTIONS)) ? FAIL : SUCCESS;

        case ZOOM_FUNCTION:
            return ((onvifCamInfo[camIndex].ptzServiceInfo.zoomSupport == NO_FUNCTION_SUPPORT) ||
                    (onvifCamInfo[camIndex].ptzServiceInfo.zoomSupport >= MAX_PTZ_SUPPORT_OPTIONS)) ? FAIL : SUCCESS;

        case FOCUS_FUNCTION:
            return ((onvifCamInfo[camIndex].imagingServiceInfo.focusInfo.focusSupport == NO_FUNCTION_SUPPORT) ||
                    (onvifCamInfo[camIndex].imagingServiceInfo.focusInfo.focusSupport >= MAX_PTZ_SUPPORT_OPTIONS)) ? FAIL : SUCCESS;

        case IRIS_FUNCTION:
            return ((onvifCamInfo[camIndex].imagingServiceInfo.irisInfo.irisSupport == NO_FUNCTION_SUPPORT) ||
                    (onvifCamInfo[camIndex].imagingServiceInfo.irisInfo.irisSupport >= MAX_PTZ_SUPPORT_OPTIONS)) ? FAIL : SUCCESS;

        default:
            return FAIL;
    }
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
