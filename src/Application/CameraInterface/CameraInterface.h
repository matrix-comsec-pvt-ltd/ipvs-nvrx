#if !defined CAMERAINTERFACE_H
#define CAMERAINTERFACE_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
  @file : CameraInterface.h
  @brief: This module provide interface with ip camera.Other module can request for media stream ,
          send command or request for any parameter to ip camera. It also provide event poll
          functionality. Other module can request for any media stream of the three ie.H264,MPEG4 or
          MJPEG.  Module provide generic interface for all ip camera to get stream and send a command.
          Command includes general command and PTZ for change a viewing position.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
#include "DeviceDefine.h"
#include "ConfigApi.h"
#include "RtspClientInterface.h"
#include "DateTime.h"
#include "CameraEvent.h"
#include "NetworkManager.h"
#include "CameraDatabase.h"
#include "CameraSearch.h"
#include "UrlRequest.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define MAX_MAIN_STRM_BUFFER_SIZE       (5 * MEGA_BYTE)
#define MAX_SUB_STRM_BUFFER_SIZE        (1 * MEGA_BYTE)
#define MAX_LIVE_STRM_BUFFER_SIZE       (2 * MEGA_BYTE)
#define ONVIF_CAMERA					255
#define INVALID_RTSP_HANDLE             0xFF

//#################################################################################################
// @DATA TYPES
//#################################################################################################
// provides camera connectivity state
typedef enum
{
	CAM_NOT_REACHABLE = 0,
	CAM_REACHABLE,
	MAX_CAM_CONNECT_STATE

}CAM_CONN_STATE_e;

typedef enum
{
    CLIENT_AUDIO_DESTINATION_DEVICE = 0,
    CLIENT_AUDIO_DESTINATION_CAMERA,
    CLIENT_AUDIO_DESTINATION_MAX

}CLIENT_AUDIO_DESTINATION_e;

typedef enum
{
    CI_STREAM_CLIENT_LIVE_START = 0,
    CI_STREAM_CLIENT_RECORD = MAX_NW_CLIENT,
    CI_STREAM_CLIENT_PRE_ALARM_RECORD,
    CI_STREAM_CLIENT_COSEC_RECORD,
    MAX_CI_STREAM_CLIENT,

}CI_STREAM_CLIENT_e;

typedef enum
{
    CI_STREAM_RESP_START = 0,
    CI_STREAM_RESP_MEDIA,
    CI_STREAM_RESP_RETRY,
    CI_STREAM_RESP_CLOSE,
    MAX_CI_STREAM_RESP

}CI_STREAM_RESP_e;

typedef enum
{
    CI_READ_LATEST_FRAME = 0,
    CI_READ_OLDEST_FRAME

}CI_BUFFER_READ_POS_e;

typedef enum
{
    PAN_TILT_FUNCTION = 0,
    ZOOM_FUNCTION,
    FOCUS_FUNCTION,
    IRIS_FUNCTION,
    MAX_PTZ_FUNCTION

}PTZ_FUNCTION_e;

//It is parameter for any type of stream
//Used as an output parameter to GetNextLiveFrame function.

typedef struct
{
	UINT32 					sampleRate;
	STREAM_CODEC_TYPE_e		streamCodecType;
	RESOLUTION_e 			resolution;
	UINT8					noOfRefFrame;
	FRAME_TYPE_e 			videoStreamType;
}STREAM_PARA_t;

//It contains all information for particular (Audio/Video) frame.
//Used as an output parameter to GetNextLiveFrame function. Also send timestamp
typedef struct
{
	STREAM_TYPE_e 			streamType;
	STREAM_PARA_t 			streamPara;
	LocalTime_t 			localTime;
}STREAM_STATUS_INFO_t;

typedef struct
{
	CI_STREAM_RESP_e	respCode;
    /* cmdStatus is only set in case of CI_STREAM_RESP_START, in other case it will be garbage value */
	NET_CMD_STATUS_e	cmdStatus;
	UINT8 				camIndex;
	UINT8				clientIndex;

}CI_STREAM_RESP_PARAM_t;

typedef struct
{
    PTZ_OPTION_e			pan;
    PTZ_OPTION_e			tilt;
    PTZ_OPTION_e			zoom;
    CAMERA_FOCUS_e			focus;
    CAMERA_IRIS_e			iris;
    PTZ_FUNCTION_e			ptzFunction;
    UINT8					speed;
    INT32					clientSocket;
    NW_CMD_REPLY_CB 		cmdRespCallback;
    BOOL                    action;

}PTZ_MSG_t;

//-------------------------------------------------------------------------------------------------
typedef void (*STREAM_REQUEST_CB)(const CI_STREAM_RESP_PARAM_t *respParam);
//-------------------------------------------------------------------------------------------------
typedef void (*IMAGE_REQUEST_CB)(UINT8 cameraIndex, NET_CMD_STATUS_e status, CHARPTR bufferPtr, UINT32 sizeToRead, CLIENT_CB_TYPE_e clientCbType);
//-------------------------------------------------------------------------------------------------
typedef void (*SET_ALARM_REQUEST_CB)(UINT8 cameraIndex, UINT8 alarmIndex, NET_CMD_STATUS_e status);
//-------------------------------------------------------------------------------------------------
typedef void (*ALARM_ACTION_REQUEST_CB)(UINT8 cameraIndex, UINT8 alarmIndex, NET_CMD_STATUS_e status);
//-------------------------------------------------------------------------------------------------
typedef void (*EVENT_RESP_CB)(UINT8 camIndex, CAMERA_EVENT_e camEvent, NET_CMD_STATUS_e camEventStatus);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitCameraInterface(void);
//-------------------------------------------------------------------------------------------------
BOOL GetCamIpAddress(UINT8 cameraIndex, CHARPTR ipAddress);
//-------------------------------------------------------------------------------------------------
BOOL GetCamAlarmState(UINT8 cameraIndex, UINT8 alarmIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e StartStream(UINT8 cameraIndex, STREAM_REQUEST_CB callback, CI_STREAM_CLIENT_e clientType);
//-------------------------------------------------------------------------------------------------
BOOL InitStreamSession(UINT8 cameraIndex, UINT8 sessionIndex,CI_BUFFER_READ_POS_e readPos);
//-------------------------------------------------------------------------------------------------
void StopStream(UINT8 cameraIndex, CI_STREAM_CLIENT_e clientType);
//-------------------------------------------------------------------------------------------------
void GetCameraFpsGop(UINT8 cameraIndex, UINT8 streamType, UINT8 *pFps, UINT8 *pGop);
//-------------------------------------------------------------------------------------------------
BOOL GetCameraStreamStatus(UINT8 cameraIndex, UINT8 camState);
//-------------------------------------------------------------------------------------------------
UINT32 GetNextFrame(UINT16 cameraIndex, UINT8 clientIndex, STREAM_STATUS_INFO_t **streamStatusInfo, UINT8PTR *streamBuffPtr, UINT32PTR streamDataLen);
//-------------------------------------------------------------------------------------------------
UINT32 GetNextFrameForLive(UINT16 cameraIndex, UINT8 clientIndex, STREAM_STATUS_INFO_t **streamStatusInfo,
                           UINT8PTR *streamBuffPtr, UINT32PTR streamDataLen,BOOL firstIframeSent);
//-------------------------------------------------------------------------------------------------
void CiCameraConfigNotify(CAMERA_CONFIG_t newCameraConfig, CAMERA_CONFIG_t *oldCameraConfig, UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
void CiIpCameraConfigNotify(IP_CAMERA_CONFIG_t newIpCfg, IP_CAMERA_CONFIG_t *oldIpCfg, UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
void CiGeneralCfgUpdate(GENERAL_CONFIG_t generalConfig, GENERAL_CONFIG_t *oldgeneralConfig);
//-------------------------------------------------------------------------------------------------
void CiLanCfgUpdate(LAN_CONFIG_ID_e lanNo, LAN_CONFIG_t newLanConfig, LAN_CONFIG_t *oldLanConfig);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e GenerateAudioToCameraRequest(UINT8 cameraIndex, CAMERA_TYPE_e *cameraType, AUD_TO_CAM_INFO_t *audToCamInfo);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e StartSendAudioToCameraRequest(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e StopSendAudioToCameraRequest(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e GetImage(UINT8 cameraIndex, CHARPTR resolutionStr, IMAGE_REQUEST_CB callback, CLIENT_CB_TYPE_e clientCbType);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e SetFocus(UINT8 cameraIndex, CAMERA_FOCUS_e focus, UINT8 speed, UINT8 sessionIndex, NW_CMD_REPLY_CB callback, INT32 clientSocket);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e SetIris(UINT8 cameraIndex, CAMERA_IRIS_e iris, UINT8 sessionIndex, NW_CMD_REPLY_CB callback, INT32 clientSocket);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e GetProfileParam(UINT8 cameraIndex, UINT8 profileIndex, VIDEO_TYPE_e streamType, NW_CMD_REPLY_CB callback,
                                 INT32 clientSocket, CLIENT_CB_TYPE_e clientCbType);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e GetMotionWindowConfig(UINT8 cameraIndex,NW_CMD_REPLY_CB callBack, INT32 clientSocket, CLIENT_CB_TYPE_e clientCbType);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e SetMotionWindowConfig(UINT8 cameraIndex, MOTION_BLOCK_METHOD_PARAM_t *pMotionDataBuf, NW_CMD_REPLY_CB callback,
                                       INT32 clientSocket, CLIENT_CB_TYPE_e clientCbType, BOOL newConfigStatusF);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e SetPrivacyMaskConfig(UINT8 cameraIndex, void *privacyAreaBuf, NW_CMD_REPLY_CB callback,
                                      INT32 clientSocket, BOOL privacyMaskStatus, CLIENT_CB_TYPE_e clientCbType);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e GetPrivacyMaskWindowConfig(UINT8 cameraIndex,NW_CMD_REPLY_CB callBack, INT32 clientSocket, CLIENT_CB_TYPE_e clientCbType);
//-------------------------------------------------------------------------------------------------
void CameraAlarmAction(UINT8 cameraIndex, UINT8 alarmIndex,	BOOL alarmAction, ALARM_ACTION_REQUEST_CB callback);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e SetPtzPosition(UINT8 cameraIndex, PTZ_OPTION_e pan, PTZ_OPTION_e tilt, PTZ_OPTION_e zoom,
                                UINT8 speed, UINT8 sessionIndex, NW_CMD_REPLY_CB callback, INT32 clientSocket);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e GotoPtzPosition(UINT8 cameraIndex,	UINT8 ptzIndex, NW_CMD_REPLY_CB callback, INT32 clientSocket, UINT8 isPauseConsider);
//-------------------------------------------------------------------------------------------------
void CiPtzPositionConfigNotify(PTZ_PRESET_CONFIG_t newPtzPresetCfg, PTZ_PRESET_CONFIG_t *oldPtzPresetCfg, UINT8 cameraIndex, UINT8 ptzIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e ValidatePtzFuncSupport(UINT8 cameraIndex, PTZ_FUNCTION_e ptzFunction);
//-------------------------------------------------------------------------------------------------
void NotifyAlarmConfigChange(CAMERA_ALARM_CONFIG_t newAlarmConfig, CAMERA_ALARM_CONFIG_t *oldAlarmConfig, UINT8 cameraIndex, UINT8 alarmIndex);
//-------------------------------------------------------------------------------------------------
void GetCameraBrandModelNumber(UINT8 cameraIndex, CAMERA_BRAND_e *brand, CAMERA_MODEL_e *model);
//-------------------------------------------------------------------------------------------------
BOOL OnvifSupportF(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
CAMERA_TYPE_e CameraType(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e ChangeCamIpAddr(UINT8 cameraIndex, IP_ADDR_PARAM_t *pNetworkParam, NW_CMD_REPLY_CB callback,
                                 INT32 clientSocket, CLIENT_CB_TYPE_e clientCbType);
//-------------------------------------------------------------------------------------------------
void SetCurrentSubStreamProfile(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
void CiActionChangeDateTime(void);
//-------------------------------------------------------------------------------------------------
BOOL getInternetConnStatus(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e GetImagingCapability(UINT8 camIndex, CAPABILITY_TYPE *pCapability);
//-------------------------------------------------------------------------------------------------
void DefaultCameraImageCapability(UINT8 camIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e WriteImageSettingAfterAdjust(UINT8 sourceCamera, IMG_SETTING_CONFIG_t *pSrcImgSettingCfg, CAMERA_BIT_MASK_t copyToCamera);
//-------------------------------------------------------------------------------------------------
void ImageSettingConfigNotify(IMG_SETTING_CONFIG_t newCopy, IMG_SETTING_CONFIG_t *oldCopy, UINT8 camIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e StreamParamCopyToCamera(UINT8 srcCamIndex, STREAM_CONFIG_t *pSrcStreamCfg, CAMERA_BIT_MASK_t *pCopyToCamMask);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e MotionDetectionCopyToCamera(UINT8 srcCamIndex, CAMERA_CONFIG_t *pSrcCamConfig, CAMERA_BIT_MASK_t copyToCamMask);
//-------------------------------------------------------------------------------------------------
BOOL GetOnvifMedia2SupportCapability(UINT8 camIndex);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif  //CAMERAINTERFACE_H
