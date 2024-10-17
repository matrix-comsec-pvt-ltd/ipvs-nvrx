#if !defined MXONVIFCLIENT_H
#define MXONVIFCLIENT_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		MxOnvifClient.h
@brief      This file contains all the defines and data types used to communicate with an ONVIF supported cameras.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "Utils.h"
#include "NetworkManager.h"
#include "CameraDatabase.h"
#include "CameraInterface.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define ONVIF_MULTICAST_PORT        3702
#define MAX_AVAILABLE_ADDRESS_LEN   256
#define DEVICE_DISCOVERY_RESP_END   "Envelope>"
#define ONVIF_ENTRY_POINT_TAG       "XAddrs>"
#define DEVICE_DISCOVERY_MSG        "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" "\
                                    "xmlns:a=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\">"\
                                    "<s:Header><a:Action s:mustUnderstand=\"1\">"\
                                    "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe</a:Action>"\
                                    "<a:MessageID>uuid:%s</a:MessageID>"\
                                    "<a:ReplyTo><a:Address>http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous</a:Address></a:ReplyTo>"\
                                    "<a:To s:mustUnderstand=\"1\">urn:schemas-xmlsoap-org:ws:2005:04:discovery</a:To>"\
                                    "</s:Header><s:Body><Probe xmlns=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\">"\
                                    "<d:Types xmlns:d=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" "\
                                    "xmlns:dp0=\"http://www.onvif.org/ver10/network/wsdl\">dp0:NetworkVideoTransmitter"\
                                    "</d:Types></Probe></s:Body></s:Envelope>"

//#################################################################################################
// @DATA_TYPES
//#################################################################################################
typedef UINT16  ONVIF_HANDLE;

typedef enum
{
	ONVIF_GET_DATE_TIME,
	ONVIF_GET_ENTRY_POINT,
	ONVIF_GET_BRAND_MODEL,
	ONVIF_GET_CAPABILITY,
	ONVIF_SET_DATE_TIME,
	ONVIF_GET_MEDIA_URL,
	ONVIF_GET_SNAPSHOT_URL,
	ONVIF_SET_PTZ,
	ONVIF_EDIT_PRESET,
	ONVIF_GOTO_PRESET,
	ONVIF_SET_ALARM,
	ONVIF_SET_FOCUS,
	ONVIF_SET_IRIS,
	ONVIF_RETRY_REQUEST,
	ONVIF_START_EVENT,
	ONVIF_UNSUBSCRIBE_EVENT_REQ,
	ONVIF_GET_EVENT_NOTIFICATION,
	ONVIF_RENEW_EVENT_REQ,
	ONVIF_SEARCH_DEVICES,
	ONVIF_OSD_SET_REQ,
	ONVIF_PROFILE_CONFIG,	// for profile releted max supported param as well as current profile config
	ONVIF_GET_MOTION_WINDOW,
	ONVIF_SET_MOTION_WINDOW,
	ONVIF_CHANGE_IP_ADDR,
	ONVIF_MEDIA_PROFILES_UPDATE,
	ONVIF_SET_PASSWORD,    
    ONVIF_GET_IMAGING_CAPABILITY,
    ONVIF_GET_IMAGING_SETTING,
    ONVIF_SET_IMAGING_SETTING,
    ONVIF_REQUEST_MAX

}ONVIF_REQUEST_e;

typedef enum
{
	ONVIF_CMD_SUCCESS,
	ONVIF_CMD_FAIL,
	ONVIF_CMD_TIMEOUT,
	ONVIF_CMD_RETRY,
	ONVIF_CMD_FEATURE_NOT_SUPPORTED,
    ONVIF_CMD_IP_CHANGE_FAIL,
	ONVIF_CMD_AUTHENTICATION_FAIL,
    ONVIF_CMD_CAMERA_REBOOT_FAIL,
    ONVIF_CMD_RESP_STATUS_MAX

}ONVIF_RESP_STATUS_e;

typedef enum
{
    ONVIF_CAM_BRAND_NAME,
    ONVIF_CAM_MODEL_NAME,
    MAX_ONVIF_CAM_DETAIL
}ONVIF_CAM_DETAIL_e;

typedef struct
{
    UINT8                   cameraIndex;
    ONVIF_REQUEST_e         requestType;
    ONVIF_RESP_STATUS_e     response;
    VOIDPTR                 data;

}ONVIF_RESPONSE_PARA_t;

typedef BOOL (*ONVIF_CAMERA_CB) (ONVIF_RESPONSE_PARA_t *responsePara);

typedef struct
{
    UINT8                   index;
    BOOL                    actionStatus;
}ONVIF_ACTION_INFO_t;

typedef struct
{
    PTZ_OPTION_e            pan;
    PTZ_OPTION_e            tilt;
    PTZ_OPTION_e            zoom;
    float                   speed;
    BOOL                    action;

}ONVIF_PTZ_ACTION_INFO_t;

typedef struct
{
    CHAR                    name[MAX_CAMERA_USERNAME_WIDTH];
    CHAR                    pwd[MAX_CAMERA_PASSWORD_WIDTH];
    CHAR                    ipAddr[IPV6_ADDR_LEN_MAX];
    UINT16                  port;

}CAMERA_PARA_t;

typedef struct
{
    UINT8                   camIndex;
    BOOL                    forceConfigOnCamera;
    ONVIF_HANDLE            sessionIndex;
    ONVIF_REQUEST_e         onvifReq;
    ONVIF_CAMERA_CB         onvifCallback;
    CAMERA_PARA_t           camPara;
    STREAM_CONFIG_t         strmConfig;
    PTZ_PRESET_CONFIG_t     ptzPresetConfig;
    ONVIF_PTZ_ACTION_INFO_t ptzInfo;
    ONVIF_ACTION_INFO_t     actionInfo;
    CAMERA_FOCUS_e          focusInfo;
    CAMERA_IRIS_e           irisInfo;
    UINT8                   profileNum;
    VOIDPTR                 pOnvifData;

}ONVIF_REQ_PARA_t;

typedef enum
{
	ONVIF_CAM_SEARCH_RESP_SUCCESS,
	ONVIF_CAM_SEARCH_RESP_CLOSE,
	MAX_ONVIF_CAM_SEARCH_RESP

}ONVIF_CAM_SEARCH_RESP_e;

typedef struct
{
	ONVIF_CAM_SEARCH_RESP_e		respCode;
    CHAR						ipv4Addr[IPV4_ADDR_LEN_MAX];
    CHAR						ipv6Addr[MAX_IPV6_ADDR_TYPE][IPV6_ADDR_LEN_MAX];
	UINT16						httpPort;
	UINT16						onvifPort;

}ONVIF_CAM_SEARCH_CB_PARAM_t;

typedef struct
{
	CHAR 			videoEncoding[MAX_ENCODER_NAME_LEN];	// video encoding method
	CHAR 			resolution[MAX_RESOLUTION_NAME_LEN];	// image resolution
	UINT8 			framerate;								// frame rate
	UINT8 			quality;								// quality of stream
	BOOL 			enableAudio;							// control to enable/disable audio
	BITRATE_MODE_e	bitrateMode;
	BITRATE_VALUE_e	bitrateValue;
	UINT8			gop;
	UINT8			profileNum;

}ONVIF_PROFILE_STREAM_INFO_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitOnvifClient(void);
//-------------------------------------------------------------------------------------------------
BOOL StartOnvifClient(ONVIF_REQ_PARA_t *onvifReqPara, ONVIF_HANDLE *handlePtr);
//-------------------------------------------------------------------------------------------------
UINT8 GetOnvifConfiguredProfile(UINT8 camIndex);
//-------------------------------------------------------------------------------------------------
ONVIF_RESP_STATUS_e OnvifUnconfigureCamReq(CHARPTR ipAddress, UINT16 port, CHARPTR usrName, CHARPTR password, ONVIF_REQUEST_e reqType, VOIDPTR data);
//-------------------------------------------------------------------------------------------------
const CHARPTR GetOnvifReqName(ONVIF_REQUEST_e onvifReq);
//-------------------------------------------------------------------------------------------------
BOOL GetOnvifMedia2Support(UINT8 camIndex);
//-------------------------------------------------------------------------------------------------
BOOL ValidatePtzFuncForOnvif(UINT8 camIndex, PTZ_FUNCTION_e ptzFunction);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif  // MXONVIFCLIENT_H
