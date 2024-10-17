#if !defined URLREQUEST_H
#define URLREQUEST_H
//#################################################################################################
// @FILE BRIEF
//#################################################################################################
/**
@file       UrlRequest.h
@brief      Provide enums and structure definations required for camera http urls for supported
            brands.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "Config.h"
#include "HttpClient.h"
#include "CameraDatabase.h"
#include "NetworkManager.h"
#include "TcpClient.h"
#include "RtspClientInterface.h"

//#################################################################################################
// @DEFINE
//#################################################################################################
#define MAX_URL_REQUEST                 (20)
#define MAX_CLIENT_CB                   (2)
#define MAX_SEND_AUDIO_URL              (5)

#define SCALING_WINDOW_WIDTH            (704)
#define SCALING_WINDOW_HEIGHT           (576)


#define ARG_DELIM                       '&'
#define URL_DELIM                       '?'
#define ASSIGN_VAL                      '='
#define COMMA_SAP                       ','

#define CTRL_MSG_DURATION               (3)
#define MAX_FILE_NAME_LENGTH            (150)

#define MOTION_LEFT_STR                 "root.Motion.M%d.Left"
#define MOTION_RIGHT_STR                "root.Motion.M%d.Right"
#define MOTION_TOP_STR                  "root.Motion.M%d.Top"
#define MOTION_BOTTOM_STR               "root.Motion.M%d.Bottom"
#define MOTION_SENSITIVITY_STR          "root.Motion.M%d.Sensitivity"

#define MOTION_STR_LEN_MAX              (50)
#define	MOTION_TAG_STR_LEN_MAX          (60)

#define MOTION_ENABLED_STR              "motion_c0_enable"
#define MOTION_WINDOW_ENABLED_STR       "motion_c0_win_i%d_enable"
#define MOTION_WINDOW_LEFT_STR          "motion_c0_win_i%d_left"
#define MOTION_WINDOW_TOP_STR           "motion_c0_win_i%d_top"
#define MOTION_WINDOW_WIDTH_STR         "motion_c0_win_i%d_width"
#define MOTION_WINDOW_HEIGHT_STR        "motion_c0_win_i%d_height"
#define MOTION_WINDOW_SENSITIVITY_STR   "motion_c0_win_i%d_sensitivity"
#define MOTION_WINDOW_NAME_STR          "motion_c0_win_i%d_name"

#define PRIVACY_MASK_STR_LEN_MAX        (50)
#define	PRIVACY_MASK_TAG_STR_LEN_MAX    (60)

#define PRIVACY_MASK_ENABLED_STR        "privacymask_c0_enable"
#define PRIVACY_MASK_WIN_ENABLED_STR    "privacymask_c0_win_i%d_enable"
#define PRIVACY_MASK_WIN_NAME_STR       "privacymask_c0_win_i%d_name"
#define PRIVACY_MASK_WIN_LEFT_STR       "privacymask_c0_win_i%d_left"
#define PRIVACY_MASK_WIN_TOP_STR        "privacymask_c0_win_i%d_top"
#define PRIVACY_MASK_WIN_WIDTH_STR      "privacymask_c0_win_i%d_width"
#define PRIVACY_MASK_WIN_HEIGHT_STR     "privacymask_c0_win_i%d_height"

#define MOTION_M_LEFT_STR				"Motion.M.Left"
#define MOTION_M_TOP_STR				"Motion.M.Top"
#define MOTION_M_RIGHT_STR				"Motion.M.Right"
#define MOTION_M_BOTTOM_STR				"Motion.M.Bottom"
#define MOTION_M_SENSITIVITY_STR		"Motion.M.Sensitivity"
#define MOTION_M_NAME_STR				"Motion.M.Name"

#define AUDIO_MIME_TYPE_STR             "audio/basic"

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum CAM_PROTOCOL_TYPE_e
{
	CAM_PROTOCOL_NONE = 0,
	CAM_RTSP_PROTOCOL,
	CAM_HTTP_PROTOCOL,
	CAM_TCP_PROTOCOL,
	MAX_CAM_PROTOCOL_TYPE

}CAM_PROTOCOL_TYPE_e;

typedef enum CAM_REQUEST_TYPE_e
{
    CAM_REQ_CONTROL         = 0,
    CAM_REQ_MEDIA           = 1,
    CAM_REQ_CONFIG          = 2,
    CAM_REQ_REBOOT          = 3,
    MAX_CAM_REQ_TYPE        = 4,
    //------------------
    CAM_REQ_GET_WINDOW      = 0,
    CAM_REQ_SET_WINDOW      = 1,
    CAM_REQ_GET_MAXWIN      = 2,
    //------------------
    CAM_ALARM_INACTIVE      = 0,
    CAM_ALARM_ACTIVE        = 1,
    //------------------
    CAM_REQ_IMG_APPEARANCE      = 0,    /* Get Image Appearance Capability and Get/Set Image Settings */
    CAM_REQ_IMG_ADV_APPEARANCE  = 1,    /* Get Image Advance Appearance Capability and Get/Set Image Settings */
    CAM_REQ_IMG_DAY_NIGHT       = 2,    /* Get Image Day-Night Capability and Get/Set Image Settings */
    CAM_REQ_IMG_MAX             = 3,

}CAM_REQUEST_TYPE_e;

typedef enum
{
    EV_RESP_MULTI_PART = 0,
    EV_RESP_ONVIF,
    MAX_EV_RESP_TYPE

}EVENT_RESP_TYPE_e;

typedef enum
{
    GET_MULTIPLE_EV_INFO = 0,
    MAX_GET_EV_INFO_TYPE

}EVENT_DET_TYPE_e;

typedef enum
{
    IMAGE_SETTING_ACTION_GET_CAPABILITY = 0,
    IMAGE_SETTING_ACTION_GET_PARAM,
    IMAGE_SETTING_ACTION_SET_PARAM,
    IMAGE_SETTING_ACTION_MAX,

}IMAGE_SETTING_ACTION_e;

typedef struct
{
    CHAR 				relativeUrl[MAX_CAMERA_URI_WIDTH];
    CHAR				fileForPutReq[MAX_FILE_NAME_LENGTH];
    UINT32 				sizeOfPutFile;
    CAM_REQUEST_TYPE_e	requestType;
    CAM_PROTOCOL_TYPE_e protocolType;
    REQUEST_AUTH_TYPE_e authMethod;
    HTTP_REQUEST_e 		httpRequestType;
    TRANSPORT_TYPE_e 	rtspTransportType;
    TCP_REQUEST_e       tcpRequestType;
    UINT8			    maxConnTime;
    HTTP_CONTENT_TYPE_e httpContentType;

}URL_REQUEST_t;

typedef struct
{
    CHAR                userName[MAX_USERNAME_WIDTH];
    CHAR                password[MAX_PASSWORD_WIDTH];
    CHAR                ipAddr[IPV6_ADDR_LEN_MAX];
    UINT16              httpPort;
    UINT8               noOfUrlReq;
    UINT8               noOfStopReq;
    URL_REQUEST_t       url[MAX_SEND_AUDIO_URL];
    HTTP_HANDLE         httpHandle[MAX_SEND_AUDIO_URL];
    HTTP_USER_AGENT_e	httpUserAgent;
}AUD_TO_CAM_INFO_t;

typedef struct
{
    EVENT_RESP_TYPE_e   evRespType;
    EVENT_DET_TYPE_e    evDetType;

}EVENT_RESP_INFO_t;

typedef struct
{
    CHAR                userName[MAX_USERNAME_WIDTH];
    CHAR                password[MAX_PASSWORD_WIDTH];
    HTTP_USER_AGENT_e   httpUserAgent;

}SEND_AUDIO_INFO_t;

typedef struct
{
    BOOL        currentOsdStatus;
    BOOL        textoverlayChanged;
    BOOL        timeOverlayChanged;
    BOOL        dateTimeOverlay;
    OSD_POS_e   dateTimePos;
    BOOL        channelNameOverlay;
    CHAR        channelName[TEXT_OVERLAY_MAX][MAX_CHANNEL_NAME_WIDTH];
    OSD_POS_e   channelNamePos[TEXT_OVERLAY_MAX];
    UINT8       channelNameOverlayMax;

}OSD_PARAM_t;

typedef struct
{
    UINT8 					cameraIndex;
    UINT8 					numOfRequest;
    UINT8 					requestCount;
    NET_CMD_STATUS_e		requestStatus;
    BOOL					camReqBusyF;
    VOIDPTR	 				clientCb[MAX_CLIENT_CB];
    INT32 					clientSocket[MAX_CLIENT_CB];
    HTTP_HANDLE 			http[MAX_URL_REQUEST];
    URL_REQUEST_t 			url[MAX_URL_REQUEST];
    pthread_mutex_t			camReqFlagLock;
    HTTP_CALLBACK 			httpCallback[MAX_CAM_REQ_TYPE];
    VOIDPTR			     	tcpCallback[MAX_CAM_REQ_TYPE];
    RTSP_HANDLE 			rtspHandle;
    VOIDPTR	 				mediaCallback;
    TIMER_HANDLE			timerHandle;
    CLIENT_CB_TYPE_e        clientCbType;

}CAMERA_REQUEST_t;

/* Only single handle and url is used in event and action. We can remove other if not required any more */
typedef struct
{
    UINT8 					numOfRequest;
    UINT8 					requestCount;
    HTTP_HANDLE		 		handle[MAX_URL_REQUEST];
    URL_REQUEST_t 			url[MAX_URL_REQUEST];
    HTTP_CALLBACK 			httpCallback[MAX_CAM_REQ_TYPE];

}CAMERA_EVENT_REQUEST_t;

typedef struct
{
    IP_ADDR_TYPE_e  ipAddrType;
    CHAR            ipAddr[IPV6_ADDR_LEN_MAX];
    UINT8           prefixLen;
    CHAR            gateway[IPV6_ADDR_LEN_MAX];

}IP_ADDR_PARAM_t;

//-------------------------------------------------------------------------------------------------
typedef NET_CMD_STATUS_e (*GET_STREAM_URL)(CAMERA_MODEL_e modelNo, STREAM_CONFIG_t * streamConfig, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq, BOOL considerConfig,VIDEO_TYPE_e streamType);
//-------------------------------------------------------------------------------------------------
typedef NET_CMD_STATUS_e (*GET_IMAGE_URL)(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
typedef NET_CMD_STATUS_e (*SET_PTZ_URL)(CAMERA_MODEL_e modelNo, PTZ_OPTION_e pan, PTZ_OPTION_e tilt, PTZ_OPTION_e zoom, BOOL action, UINT8 speed, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
typedef NET_CMD_STATUS_e (*STORE_PTZ_URL)(CAMERA_MODEL_e modelNo, UINT8 ptzIndex, CHARPTR presetName, BOOL presetAction, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
typedef NET_CMD_STATUS_e (*GOTO_PTZ_URL)(CAMERA_MODEL_e modelNo, UINT8 ptzIndex, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
typedef NET_CMD_STATUS_e (*SET_IRIS_URL)(CAMERA_MODEL_e modelNo, CAMERA_IRIS_e iris, BOOL action, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
typedef NET_CMD_STATUS_e (*SET_FOCUS_URL)(CAMERA_MODEL_e modelNo, CAMERA_FOCUS_e focus, BOOL action, UINT8 speed, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
typedef NET_CMD_STATUS_e (*SET_ALARM_URL)(CAMERA_MODEL_e modelNo, UINT8 alarmIndex, BOOL defaultState, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
typedef NET_CMD_STATUS_e (*GET_CURR_STRM_CFG_URL)(CAMERA_MODEL_e modelNo, UINT8 profileIndex, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq,VIDEO_TYPE_e streamType);
//-------------------------------------------------------------------------------------------------
typedef NET_CMD_STATUS_e (*CHANGE_CAM_IP_URL)(CAMERA_MODEL_e modelNo, IP_ADDR_PARAM_t *networkParam, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
typedef NET_CMD_STATUS_e (*SET_OSD_URL)(CAMERA_MODEL_e modelNo, OSD_PARAM_t *osdParam, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
typedef NET_CMD_STATUS_e (*GET_DEVICE_INFO_URL)(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
typedef NET_CMD_STATUS_e (*GET_MOTION_WINDOW_URL)(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
typedef NET_CMD_STATUS_e (*SET_MOTION_WINDOW_URL)(CAMERA_MODEL_e modelNo, MOTION_BLOCK_METHOD_PARAM_t *motionBlockParam, URL_REQUEST_t *urlReqPtr,UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
typedef NET_CMD_STATUS_e (*GET_PRIVACYMASK_URL)(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
typedef NET_CMD_STATUS_e (*SET_PRIVACYMASK_URL)(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq, PRIVACY_MASK_CONFIG_t *privacyArea, BOOL reqSource,	CHARPTR *privacyMaskName);
//-------------------------------------------------------------------------------------------------
typedef NET_CMD_STATUS_e (*GET_MAX_PRIVACY_MASK_WIN_URL)(CAMERA_MODEL_e brandNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
typedef NET_CMD_STATUS_e (*SEND_AUDIO_URL)(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq, UINT8PTR noOfStopReq, SEND_AUDIO_INFO_t* sendAudInfo);
//-------------------------------------------------------------------------------------------------
typedef NET_CMD_STATUS_e (*EVENT_REQUEST_URL)(CAMERA_MODEL_e modelNo, CAMERA_EVENT_e camEvent, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq, EVENT_RESP_INFO_t *evRespInfo);
//-------------------------------------------------------------------------------------------------
typedef NET_CMD_STATUS_e (*PARSER_FUNC)(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR data, VOIDPTR evRes);
//-------------------------------------------------------------------------------------------------
typedef NET_CMD_STATUS_e (*PARSER_CONFIG_FUNC)(CAMERA_MODEL_e modelNo, UINT32 dataSize, STREAM_CONFIG_t *streamConfig, CHARPTR data, UINT8 camIndex,VIDEO_TYPE_e streamType,UINT8 profileIndex);
//-------------------------------------------------------------------------------------------------
typedef NET_CMD_STATUS_e (*PARSER_DEVICE_INFO_FUNC)(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR modelName, CHARPTR data);
//-------------------------------------------------------------------------------------------------
typedef NET_CMD_STATUS_e (*PARSER_GET_MOTION_WINDOW_RESPONSE)(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR data, MOTION_BLOCK_METHOD_PARAM_t *motionBlockParam);
//-------------------------------------------------------------------------------------------------
typedef NET_CMD_STATUS_e (*PARSER_PRIVACYMSAK_RESPONSE)(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR data,PRIVACY_MASK_CONFIG_t *privacyArea, CHARPTR *privacyMaskName);
//-------------------------------------------------------------------------------------------------
typedef NET_CMD_STATUS_e (*SET_PASSWD_URL)(CAMERA_MODEL_e brandNo,CHARPTR userName,CHARPTR passwd, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
typedef NET_CMD_STATUS_e (*SET_DATE_TIME_URL)(CAMERA_MODEL_e modelNo,URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
typedef NET_CMD_STATUS_e (*IMAGE_SETTING_URL)(CAMERA_MODEL_e modelNo,URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq, IMAGE_SETTING_ACTION_e action, IMAGE_CAPABILITY_INFO_t *pImageCapsInfo);
//-------------------------------------------------------------------------------------------------
typedef NET_CMD_STATUS_e (*PARSER_IMAGE_SETTING_RESPONSE)(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR data, IMAGE_SETTING_ACTION_e action, IMAGE_CAPABILITY_INFO_t *pImageCapsInfo);
//-------------------------------------------------------------------------------------------------

typedef enum
{
    REQ_URL_GET_STREAM_CFG = 0,
    REQ_URL_SET_STREAM_CFG,
    REQ_URL_GET_STREAM,
    REQ_URL_START_STREAM,
    REQ_URL_STOP_STREAM,
    REQ_URL_SET_DATE_TIME,
    REQ_URL_SET_OSD,
    REQ_URL_GET_EVENT,
    REQ_URL_GET_IMAGE,
    REQ_URL_START_AUDIO,
    REQ_URL_STOP_AUDIO,
    REQ_URL_SET_ALARM,
    REQ_URL_SET_PASSWORD,
    REQ_URL_CHANGE_CAMERA_ADDR,
    REQ_URL_GET_MAX_PRIVACY_MASK,
    REQ_URL_GET_PRIVACY_MASK,
    REQ_URL_SET_PRIVACY_MASK,
    REQ_URL_GET_MOTION_WINDOW,
    REQ_URL_SET_MOTION_WINDOW,
    REQ_URL_STORE_PTZ,
    REQ_URL_GO_TO_PTZ,
    REQ_URL_SET_PTZ,
    REQ_URL_SET_FOCUS,
    REQ_URL_SET_IRIS,
    REQ_URL_GET_IMAGE_CAPABILITY,
    REQ_URL_GET_IMAGE_SETTING,
    REQ_URL_SET_IMAGE_SETTING,
    REQ_URL_MAX,

}REQ_URL_e;

typedef struct
{
    GET_STREAM_URL                  toGetStream;
    GET_IMAGE_URL                   toGetImage;
    SET_PTZ_URL                     toSetPtz;
    STORE_PTZ_URL                   toStorePtz;
    GOTO_PTZ_URL                    toGotoPtz;
    SET_IRIS_URL                    toSetIris;
    SET_FOCUS_URL                   toSetFocus;
    SET_ALARM_URL                   toSetAlarm;
    EVENT_REQUEST_URL               toReqEvent;
    GET_CURR_STRM_CFG_URL           toGetCurrStrmCfg;
    CHANGE_CAM_IP_URL               toChangeCamIpAddr;
    GET_DEVICE_INFO_URL             toGetDeviceInfo;
    SET_OSD_URL                     toSetOsd;
    GET_MOTION_WINDOW_URL           toGetMotionWindow;
    SET_MOTION_WINDOW_URL           toSetMotionWindow;
    GET_PRIVACYMASK_URL             toGetPrivacyMask;
    SET_PRIVACYMASK_URL             toSetPrivacyMask;
    SEND_AUDIO_URL                  toSendAudio;
    SET_PASSWD_URL                  toSetPasswd;
    GET_MAX_PRIVACY_MASK_WIN_URL    toGetMaxPrivacyMaskWindow;
    SET_DATE_TIME_URL               toSetDateTime;
    IMAGE_SETTING_URL               toImageSetting;

}URL_FUNCTION_t;

//#################################################################################################
// @EXTERN VARIABLES
//#################################################################################################
/* Function Pointers defined in MatrixUrl.c */
extern const PARSER_FUNC                        MatrixParser;
extern const PARSER_CONFIG_FUNC                 MatrixCnfgParser;
extern const PARSER_DEVICE_INFO_FUNC            MatrixDeviceInfoParser;
extern const PARSER_PRIVACYMSAK_RESPONSE        MatrixPrivacyMaskParser;
extern const PARSER_GET_MOTION_WINDOW_RESPONSE  MatrixGetMotionWindowParser;
extern const PARSER_IMAGE_SETTING_RESPONSE      MatrixImageSettingParser;
extern const URL_FUNCTION_t                     MatrixUrl;

//#################################################################################################
// @FUNCTION PROTOTYPE
//#################################################################################################
//-------------------------------------------------------------------------------------------------
URL_FUNCTION_t *GetUrl(CAMERA_BRAND_e brand);
//-------------------------------------------------------------------------------------------------
PARSER_FUNC *ParseFunc(CAMERA_BRAND_e brand);
//-------------------------------------------------------------------------------------------------
PARSER_CONFIG_FUNC *ParseCnfgFunc(CAMERA_BRAND_e brand);
//-------------------------------------------------------------------------------------------------
PARSER_DEVICE_INFO_FUNC *ParseGetDevInfoFunc(CAMERA_BRAND_e brand);
//-------------------------------------------------------------------------------------------------
PARSER_GET_MOTION_WINDOW_RESPONSE *ParseGetMotionWindowResponseFunc(CAMERA_BRAND_e brand);
//-------------------------------------------------------------------------------------------------
PARSER_PRIVACYMSAK_RESPONSE *ParsePrivacyMaskRersponseFunc(CAMERA_BRAND_e brand);
//-------------------------------------------------------------------------------------------------
PARSER_IMAGE_SETTING_RESPONSE *ParseImageSettingResponseFunc(CAMERA_BRAND_e brand);
//-------------------------------------------------------------------------------------------------
void GetCiRequestUrl(UINT8 cameraIndex, REQ_URL_e urlType, UINT32 cmdArg, CAM_REQUEST_TYPE_e reqType,
                     VOIDPTR callback, CAMERA_REQUEST_t *pCamReqInfo, VOIDPTR pInArg);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e GetCameraBrandModelUrl(UINT8 cameraIndex, REQ_URL_e reqType, CAMERA_REQUEST_t *pCamReqInfo,
                                        VOIDPTR pInArg1, VOIDPTR pInArg2, VOIDPTR pInArg3, VOIDPTR pInArg4);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e GetBrandModelReqestUrl(UINT8 cameraIndex, CAMERA_BRAND_e brand, CAMERA_MODEL_e model, REQ_URL_e reqType,
                                        CAMERA_REQUEST_t *pCamReqInfo, VOIDPTR pInArg1, VOIDPTR pInArg2, VOIDPTR pInArg3);
//-------------------------------------------------------------------------------------------------
BOOL GetBrandDeviceInfo(CAMERA_BRAND_e brand, CAMERA_MODEL_e model, URL_REQUEST_t *pReqestUrl, UINT8 *pNumOfRequest);
//-------------------------------------------------------------------------------------------------
BOOL MapPrivacyMaskNvrToCamera(CAMERA_BRAND_e brand, CAMERA_MODEL_e model, PRIVACY_MASK_CONFIG_t *pPrivacyMask);
//-------------------------------------------------------------------------------------------------
BOOL MapPrivacyMaskCameraToNvr(CAMERA_BRAND_e brand, CAMERA_MODEL_e model, PRIVACY_MASK_CONFIG_t *pPrivacyMask);
//-------------------------------------------------------------------------------------------------
BOOL GetResolutionNoforMatrixIpCamera(CHARPTR resolutionStr, MATRIX_IP_CAM_RESOLUTION_e *resolutionNo);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif	// URLREQUEST_H
