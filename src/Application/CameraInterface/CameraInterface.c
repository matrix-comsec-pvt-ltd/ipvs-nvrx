//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file       CameraInterface.c
@brief      Camera interface send user request to camera and returns response to the user. It
            requests for stream, image, PTZ control, etc. It monitors camera event through
            streaming or by sending commands periodically to camera.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "DebugLog.h"
#include "Utils.h"
#include "CameraInterface.h"
#include "NetworkManager.h"
#include "HttpClient.h"
#include "RecordManager.h"
#include "LiveMediaStreamer.h"
#include "EventLogger.h"
#include "MxOnvifClient.h"
#include "CameraConfig.h"
#include "AutoConfigCamera.h"
#include "CameraSearch.h"
#include "AdvanceCameraSearch.h"
#include "PtzTour.h"
#include "TcpClient.h"
#include "ClientMediaStreamer.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define MAX_FRAME_IN_BUFFER                 (1000)

#define MAX_CAMERA_DISCONN_COUNT            (3)
#define MIN_CAMERA_DISCONN_COUNT            (0)
#define DFLT_CAMERA_DISCONN_COUNT           (0)
#define DFLT_START_DISCONN_COUNT            (2)

#define RECORD_STRM_RETRY_CNT               (10)
#define STREAM_RETRY_TIME                   (2)
#define MAX_CONN_CHECK_PERIOD               (1)
#define MAX_CONN_CHECK_PERIOD_MS            (MAX_CONN_CHECK_PERIOD * 1000)

#define MAX_ONVIF_RETRY                     (5)
#define TIMER_COUNT_FOR_PTZ_REQ             (300)
#define TIMER_COUNT_FOR_PTZ_REQ_RETRY       (100)
#define MAX_PTZ_REQ_TIMEOUT                 (30)

/* To avoid execution of profile update and image setting timer at same time. Start Image setting timer with some delay */
#define IMAGE_SETTING_UPDATE_TIME           (15)    //in minutes
#define POWER_ON_IMAGE_SETTING_DELAY_TIME   (7)     //in minutes
#define PROFILE_UPDATE_TIME                 (15)    //in minutes

/* This parameter is been tuned and kept. Changing should first need of tunning. */
#define WINDOW_SIZE                         (250)
#define AVG_WINDOW_SIZE                     (100)

#define	PTZ_MSG_QUEUE_MAX                   25  /* Size increased for joystic bulk commands handling */

#define INTERNET_CONNECTIVITY_CHECK_IPV4    "8.8.8.8"               /* Google's IPv4 DNS server */
#define INTERNET_CONNECTIVITY_CHECK_IPV6    "2001:4860:4860::8888"  /* Google's IPv6 DNS server */
#define INTERNET_CONNECTIVITY_CHECK_PORT    (53)

/* Default fps if stream config is not updated from camera due to error */
#define DEFAULT_FPS                         (30)
#define CONN_MONITORT_THREAD_STACK_SZ       (2 * MEGA_BYTE)
#define PTZ_CTRL_THREAD_STACK_SZ            (2 * MEGA_BYTE)

//#################################################################################################
// @DATA_TYPES
//#################################################################################################
typedef enum
{
    CAM_REQ_STRUCT_PTZ,
    CAM_REQ_STRUCT_STORE_PRESET,
    CAM_REQ_STRUCT_GOTO_PRESET,
    CAM_REQ_STRUCT_FOCUS,
    CAM_REQ_STRUCT_IRIS,
    CAM_REQ_STRUCT_SET_OSDS,
    MAX_CAM_REQ_STRUCT_TYPE

}CAM_REQ_STRUCT_TYPE_e;

typedef enum
{
    CI_STREAM_EXT_TRG_START_STREAM,
    CI_STREAM_EXT_TRG_STOP_STREAM,
    CI_STREAM_EXT_TRG_CONTROL_CB,
    CI_STREAM_EXT_TRG_MEDIA_CB,
    CI_STREAM_EXT_TRG_CONFIG_CHANGE,
    CI_STREAM_EXT_TRG_CONNECTIVITY,
    CI_STREAM_EXT_TRG_TIMER_CB,
    MAX_CI_STREAM_EXT_TRG

}CI_STREAM_EXT_TRG_e;

typedef enum
{
    CI_STREAM_STATE_OFF,
    CI_STREAM_STATE_READY,
    CI_STREAM_STATE_ON_WAIT,
    CI_STREAM_STATE_ON,
    CI_STREAM_STATE_OFF_WAIT,
    CI_STREAM_STATE_RETRY,
    CI_STREAM_STATE_RESTART,
    MAX_CI_STREAM_STATE

}CI_STREAM_STATE_e;

typedef enum
{
    CI_STREAM_INT_RESP_FRAME,
    CI_STREAM_INT_RESP_CLOSE,
    MAX_CI_STREAM_INT_RESP

}CI_STREAM_INT_RESP_e;

typedef struct
{
    NET_CMD_STATUS_e		status;
    CI_STREAM_EXT_TRG_e		type;
    CI_STREAM_CLIENT_e		clientType;
    STREAM_REQUEST_CB		callback;
    CI_STREAM_INT_RESP_e	mediaResp;
    void                    *bufferPtr;
    STREAM_TYPE_e 			streamType;
    MEDIA_FRAME_INFO_t		*mediaFrame;
    UINT8					camIndex;
    BOOL					connState;

}CI_STREAM_EXT_TRG_PARAM_t;

// This structure stores parameter required for camera connectivity monitor function.
typedef struct
{
    INT32					connFd;			// file descriptor to check camera connectivity
    INT8					disconnCount;	// camera consecutive disconnection count
    CHAR					ipAddr[IPV6_ADDR_LEN_MAX];
    pthread_mutex_t			ipAddrLock;

}CAMERA_CONNECTIVITY_t;

//Frame information
typedef struct
{
    UINT32 					frameLen;
    UINT8PTR 				framePtr;
    STREAM_STATUS_INFO_t	streamStatusInfo;

}FRAME_INFO_t;

//This provide current Read / Write point inside buffer.
typedef struct
{
    INT16 					rdPos[MAX_CI_STREAM_CLIENT];
    INT16 					wrPos;
    INT16 					maxWriteIndex;
    INT16					lastIframe;
    pthread_rwlock_t 		writeIndexLock;
    pthread_mutex_t 		writeBuffLock;
    FRAME_INFO_t 			frameInfo[MAX_FRAME_IN_BUFFER];

}BUFFER_MARKER_t;

//This provide current camera stream status.It is for all supported camera.
typedef struct
{
    CI_STREAM_STATE_e		currState;
    STREAM_REQUEST_CB		clientCb[MAX_CI_STREAM_CLIENT];
    pthread_mutex_t			currStateLock;			// stream status access lock
    UINT8PTR 				bufferPtr;				// buffer pointer where stream of data to be stored
    BUFFER_MARKER_t 		frameMarker;			// manages read / write index and relative frame info
    CAMERA_REQUEST_t 		getStreamRequest;		// parameters required for camera request
    TIMER_HANDLE 			retryTimerHandle;		// timer to retry for stream, if failed previously
    UINT16					fHeight;				//
    UINT16					fWidth;					//
    RESOLUTION_e			fResolution;			//
    UINT32 					bufferSize;				// size of the frame buffer
    UINT32					lastFrameTimeSec;
    BOOL					forceConfigOnCamera;	// Force Stream Config On Camera
    BOOL 					streamRetryRunning;
    UINT8					fps;
    UINT8					fpsTotal;
    UINT8					gop;
    UINT8					gopTotal;
    UINT8					recStrmRetryCnt;

    UINT8					offWaitUnHandleCnt;

    BOOL					firstAudioFrame;
    BOOL					firstVideoFrame;

    INT16					endWinId;
    INT16					startWinId;
    INT16					maxWindow;

    UINT64					presentationTimeAvg[WINDOW_SIZE];
    UINT32					frameDiff;
    LocalTime_t 			localPrevTime;
    LocalTime_t 			localPrevTimeAudio;
    UINT8                   configFrameRate;            //store current config FPS (Req for HI3536 decoder)

}STREAM_INFO_t;

typedef struct
{
    UINT8 					requestCounter;
    BOOL					reqStatus[2];
    pthread_mutex_t 		reqStatusLock;
    pthread_mutex_t 		counterLock;
    TIMER_HANDLE 			timerHandle;
    CAMERA_REQUEST_t 		request;

}ALARM_ACTION_REQUEST_t;

typedef struct
{
    CAMERA_BRAND_e			brand;
    CAMERA_MODEL_e			model;
    STREAM_CODEC_TYPE_e		videoCodec[MAX_STREAM];
    RESOLUTION_e			resolution[MAX_STREAM];
    BOOL 					onvifSupportF;
    BOOL					configChangeNotify;
    pthread_rwlock_t		configInfoLock;
    CAMERA_TYPE_e			cameraType;

}CAM_CONFIG_INFO_t;

typedef struct
{
    BOOL					initialSetupF;                              // Status of initial setup
    BOOL					authFailF;                                  // Status of authentication of onvif get device information request
    BOOL					entryPointReceived;                         // Status of device entry point
    UINT8					retryCount;
    ONVIF_REQUEST_e			prevRequest;
    pthread_mutex_t			onvifAccessLock;
    TIMER_HANDLE			onvifRetryTimerHandle;                      // Retry timer handle for ONVIF
    TIMER_HANDLE			onvifStrmUrlRetryTimerHandle[MAX_STREAM];   // Retry timer handle for ONVIF
    CHAR					onvifCamBrand[MAX_BRAND_NAME_LEN];          // Brand of the ONVIF camera
    CHAR					onvifCamModel[MAX_MODEL_NAME_LEN];          // Model of the ONVIF camera

}ONVIF_CAM_DETAIL_t;

typedef struct
{
    IP_ADDR_PARAM_t     newNetworkParam;
    CAMERA_REQUEST_t    request;
    BOOL                status;

}CHANGE_CAM_IP_ADDR_t;

typedef struct
{
    CAMERA_REQUEST_t            request;
    BOOL                        status;
    MOTION_BLOCK_METHOD_PARAM_t motionParam;
    BOOL                        configStatus;   /* motion detection enable status */

}MOTION_WINDOW_REQ_t;

typedef struct
{
    CAMERA_REQUEST_t		request;
    BOOL					status;
    CHAR					privacyMaskName[MAX_PRIVACY_MASKS][25];
    PRIVACY_MASK_CONFIG_t	privacyArea[MAX_PRIVACY_MASKS];
    UINT8					maxSupportedMaskWindow;

}PRIVACY_WINDOW_REQ_t;

typedef struct
{
    BOOL					setCnfgToCameraF;
    pthread_mutex_t         setCnfgToCameraLock;
    CAMERA_REQUEST_t		request;
    IMAGE_CAPABILITY_INFO_t imageCapsInfo;

}IMAGE_SETTING_REQ_t;

typedef struct
{
    BOOL                    isOsdSetPending;
    OSD_PARAM_t             osdParam;
    CAMERA_REQUEST_t        request;

}OSD_PARAM_REQ_t;

typedef struct
{
    BOOL                    needToSetPwd;
    CAMERA_REQUEST_t        request;

}DATE_TIME_REQ_t;

typedef struct
{
    pthread_rwlock_t		configNotifyLock;
    BOOL					cameraConfigNotifyF;

}CAMERA_CONFIG_NOTIFY_CONTROL_t;

typedef struct
{
    pthread_mutex_t         profileSchedularProcRunLock;
    BOOL                    profileSchedularProcRunF;

}PROFILE_SHEDULAR_PROCESS_PARAM_t;

typedef struct
{
    UINT32					readIdx;
    UINT32					writeIdx;
    PTZ_MSG_t				ptzMsg[PTZ_MSG_QUEUE_MAX];

}PTZ_MSG_QUE_t;

typedef struct
{
    BOOL					runningStatus;
    UINT8					sessionIndex;
    UINT32					cameraIndex;
    pthread_mutex_t 		ptzMutex;
    PTZ_MSG_QUE_t			ptzMagQue;
    pthread_mutex_t 		ptzSignalLock;
    pthread_cond_t 			ptzSignal;
    UINT8                   stopTimeoutCnt;

}PTZ_COMMAND_INFO_t;

typedef struct
{
    BOOL                    ptzFunctionPause;
    pthread_mutex_t         ptzPauseLock;

}PTZ_FUNC_PAUSE_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void initCameraRequestParam(UINT8 cameraIndex, CAMERA_REQUEST_t *pCameraRequest);
//-------------------------------------------------------------------------------------------------
static void getCameraCodecNum(UINT8 cameraIndex, VIDEO_TYPE_e streamType, STREAM_CODEC_TYPE_e *codecNo);
//-------------------------------------------------------------------------------------------------
static void *connMonitorThread(void *pThreadArg);
//-------------------------------------------------------------------------------------------------
static void profileSchedulerCb(UINT32 data);
//-------------------------------------------------------------------------------------------------
static void resetCamIpAddress(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
static BOOL getStreamProfileNum(VIDEO_TYPE_e streamType, UINT16 camIndex, UINT8PTR streamProfileIndex);
//-------------------------------------------------------------------------------------------------
static BOOL isStreamConfigValid(STREAM_CONFIG_t *streamCfg, VIDEO_TYPE_e streamType);
//-------------------------------------------------------------------------------------------------
static BOOL isStreamUrlAvailable(CAMERA_REQUEST_t *pCameraRequest);
//-------------------------------------------------------------------------------------------------
static void storePtzPosition(UINT8 cameraIndex, UINT8 ptzIndex, BOOL presetAction, PTZ_PRESET_CONFIG_t *pPtzPresetCfg);
//-------------------------------------------------------------------------------------------------
static BOOL updatePtzPosInAllCfg(UINT8 cameraIndex, UINT8 ptzIndex);
//-------------------------------------------------------------------------------------------------
static void httpStreamControlCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void httpChangeCamIpAddrCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void httpRebootIpCamCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static BOOL sendChangeCamIpAddrResponseToClient(UINT8 cameraIndex, NET_CMD_STATUS_e response);
//-------------------------------------------------------------------------------------------------
static void restoreDfltCamIpChangeReqParam(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
static void httpGetMotionRequestCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void sendGetMotionWindowResponseToClient(UINT8 cameraIndex, NET_CMD_STATUS_e responseStatus);
//-------------------------------------------------------------------------------------------------
static void restoreDefaultMotionConfig(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
static void httpSetMotionRequestCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void httpGetPrivacyMaskWindowRequestCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void sendGetPrivacyMaskResponseToClient(UINT8 cameraIndex, NET_CMD_STATUS_e responseStatus);
//-------------------------------------------------------------------------------------------------
static void restoreDefaultPrivacyMaskConfig(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
static void httpSetPrivacyMaskWindowRequestCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void httpGetMaxPrivacyMaskWindowRequestCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void tcpMediaCb(TCP_FRAME_INFO_t *tcpFrame);
//-------------------------------------------------------------------------------------------------
static void httpStreamMediaCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void rtspCb(MediaFrameResp_e mediaResp, UINT8PTR rtspData, MEDIA_FRAME_INFO_t *frameInfo, UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
static BOOL startStreamRetry(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
static void stopAllStreamRequests(UINT8 cameraIndex, CI_STREAM_INT_RESP_e mediaResp);
//-------------------------------------------------------------------------------------------------
static void streamRetryTimeout(UINT32 index);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e sendReqToCamera(CAMERA_REQUEST_t *request, IP_CAMERA_CONFIG_t *ipCamCfg, VIDEO_TYPE_e streamType);
//-------------------------------------------------------------------------------------------------
static void resetPtsAvgParam(STREAM_INFO_t *pStreamInfo);
//-------------------------------------------------------------------------------------------------
static void actualTimeStampFrame(UINT8 cameraIndex,MEDIA_FRAME_INFO_t *frameInfoPtr, STREAM_TYPE_e streamType);
//-------------------------------------------------------------------------------------------------
static void writeToStreamBuff(UINT8 cameraIndex, STREAM_TYPE_e streamType, UINT8PTR streamData, MEDIA_FRAME_INFO_t *frameInfoPtr);
//-------------------------------------------------------------------------------------------------
static UINT32 readFromStreamBuff(UINT16 cameraIndex, UINT8 clientIndex, STREAM_STATUS_INFO_t **streamStatusInfo,
                                 UINT8PTR *streamData, UINT32PTR streamDataLen,BOOL firstIframeSent);
//-------------------------------------------------------------------------------------------------
static void httpImageControlCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void httpImageMediaCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void httpSetPtzControlCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void httpStorePtzControlCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void httpGotoPtzControlCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void httpSetFocusControlCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void httpSetIrisControlCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void httpSetOsdControlCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void httpAlarmActivateControlCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void httpAlarmDeactivateControlCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void httpCommonControlCb(CAM_REQ_STRUCT_TYPE_e reqType,HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void httpSetDateTimeCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void httpSetPasswordCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static BOOL cameraConfigGetProfileParamCb(UINT8 camIndex, STREAM_CONFIG_t *streamConfig, UINT8 profileIndex);
//-------------------------------------------------------------------------------------------------
static BOOL sendProfileResponseToClient(UINT8 camIndex, STREAM_CONFIG_t *streamConfig, UINT8 profileIndex, NET_CMD_STATUS_e response);
//-------------------------------------------------------------------------------------------------
static void alarmPulseTimeout(UINT32 data);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e requestCameraAlarm(UINT8 cameraIndex, UINT8 alarmIndex, IP_CAMERA_CONFIG_t *ipCamCfg, BOOL requestType);
//-------------------------------------------------------------------------------------------------
static void alarmRespHandler(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo, BOOL alarmAction);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e sendOnvifCommand(ONVIF_REQ_PARA_t *pOnvifReq, IP_CAMERA_CONFIG_t *pIpCamCfg);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e sendOnvifCommonCmd(UINT8 cameraIndex, ONVIF_REQUEST_e reqType, ONVIF_CAMERA_CB callback,
                                           IP_CAMERA_CONFIG_t *pIpCamCfg, VOIDPTR pOnvifData);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e sendOnvifStreamCmd(UINT8 cameraIndex, ONVIF_REQUEST_e reqType, ONVIF_CAMERA_CB callback,
                                           IP_CAMERA_CONFIG_t *pIpCamCfg, UINT8 profile, BOOL forceCfg, STREAM_CONFIG_t *pStreamCfg);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e sendOnvifActionCmd(UINT8 cameraIndex, ONVIF_REQUEST_e reqType, ONVIF_CAMERA_CB callback,
                                           IP_CAMERA_CONFIG_t *pIpCamCfg, UINT8 actionIndex, BOOL actionStatus, PTZ_PRESET_CONFIG_t *pPresetCfg);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e sendOnvifPtzCmd(UINT8 cameraIndex, ONVIF_REQUEST_e reqType, ONVIF_CAMERA_CB callback, IP_CAMERA_CONFIG_t *pIpCamCfg,
                                        PTZ_OPTION_e pan, PTZ_OPTION_e tilt, PTZ_OPTION_e zoom, BOOL action, float speed, CAMERA_FOCUS_e focus, CAMERA_IRIS_e iris);
//-------------------------------------------------------------------------------------------------
static BOOL startOnvifRetryTimer(UINT8 cameraIndex, ONVIF_REQUEST_e request);
//-------------------------------------------------------------------------------------------------
static void startOnvifStreamUrlRetryTimer(UINT8 cameraIndex, ONVIF_REQUEST_e request);
//-------------------------------------------------------------------------------------------------
static void onvifTimerTimeout(UINT32 data);
//-------------------------------------------------------------------------------------------------
static void onvifStreamUrlTimerCB(UINT32 data);
//-------------------------------------------------------------------------------------------------
static BOOL onvifDateTimeCb(ONVIF_RESPONSE_PARA_t *responseData);
//-------------------------------------------------------------------------------------------------
static BOOL onvifDeviceEntryPointCb(ONVIF_RESPONSE_PARA_t *responseData);
//-------------------------------------------------------------------------------------------------
static BOOL onvifBrandModelCb(ONVIF_RESPONSE_PARA_t *responseData);
//-------------------------------------------------------------------------------------------------
static BOOL onvifCamCapabilityCb(ONVIF_RESPONSE_PARA_t *responseData);
//-------------------------------------------------------------------------------------------------
static BOOL onvifStreamUrlCb(ONVIF_RESPONSE_PARA_t *responseData);
//-------------------------------------------------------------------------------------------------
static BOOL onvifImageCb(ONVIF_RESPONSE_PARA_t *responseData);
//-------------------------------------------------------------------------------------------------
static BOOL onvifSetDateTimeCb(ONVIF_RESPONSE_PARA_t *responseData);
//-------------------------------------------------------------------------------------------------
static BOOL onvifSetPtzCb(ONVIF_RESPONSE_PARA_t *responseData);
//-------------------------------------------------------------------------------------------------
static BOOL onvifStorePresetCb(ONVIF_RESPONSE_PARA_t *responseData);
//-------------------------------------------------------------------------------------------------
static BOOL onvifGotoPresetCb(ONVIF_RESPONSE_PARA_t *responseData);
//-------------------------------------------------------------------------------------------------
static BOOL onvifSetFocusCb(ONVIF_RESPONSE_PARA_t *responseData);
//-------------------------------------------------------------------------------------------------
static BOOL onvifSetIrisCb(ONVIF_RESPONSE_PARA_t *responseData);
//-------------------------------------------------------------------------------------------------
static BOOL onvifSetOsdCb(ONVIF_RESPONSE_PARA_t *responseData);
//-------------------------------------------------------------------------------------------------
static BOOL onvifGetProfileParamCb(ONVIF_RESPONSE_PARA_t *responseData);
//-------------------------------------------------------------------------------------------------
static BOOL onvifGetMotionWindowCb(ONVIF_RESPONSE_PARA_t *responseData);
//-------------------------------------------------------------------------------------------------
static BOOL onvifSetMotionWindowCb(ONVIF_RESPONSE_PARA_t *responseData);
//-------------------------------------------------------------------------------------------------
static BOOL onvifChangeIpAddressCb(ONVIF_RESPONSE_PARA_t *responseData);
//-------------------------------------------------------------------------------------------------
static void onvifCommonCtrlCb(CAM_REQ_STRUCT_TYPE_e reqType, ONVIF_RESPONSE_PARA_t *responseData);
//-------------------------------------------------------------------------------------------------
static BOOL onvifActivateAlarmCb(ONVIF_RESPONSE_PARA_t *responseData);
//-------------------------------------------------------------------------------------------------
static BOOL onvifInactivateAlarmCb(ONVIF_RESPONSE_PARA_t *responseData);
//-------------------------------------------------------------------------------------------------
static BOOL onvifAlarmRespHandler(ONVIF_RESPONSE_PARA_t *responseData, BOOL alarmAction);
//-------------------------------------------------------------------------------------------------
static BOOL onvifMediaProfileUpdateCb(ONVIF_RESPONSE_PARA_t *responseData);
//-------------------------------------------------------------------------------------------------
static void tcpGetMotionCnfgCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void tcpGetTestCamCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void tcpSetOverlayCnfgCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void tcpSetMotionCnfgCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void tcpCamAlarmOutCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void tcpSetPrivacyMaskCnfgCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void tcpGetMaxPrivacyMaskCnfgCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void tcpStreamControlCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void tcpSetPtzControlCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void tcpSetFocusControlCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void tcpStorePresetControlCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void tcpGotoPresetControlCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void tcpCommonControlCb(CAM_REQ_STRUCT_TYPE_e reqType, TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void tcpSetDateTimeCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void tcpStartSndAudReqCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void tcpStopSndAudReqCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e setOsds(UINT8 cameraIndex, OSD_PARAM_t *osdParam);
//-------------------------------------------------------------------------------------------------
static void restoreDefaultOsdParam(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e ciStreamExtTriggerHandler(const CI_STREAM_EXT_TRG_PARAM_t *triggerParam);
//-------------------------------------------------------------------------------------------------
static BOOL writePtzMsg(UINT8 camIndex, PTZ_MSG_t *pPtzMsg, BOOL emitPtzSignal);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e sendMsgToPtzCtrlThread(UINT8 cameraIndex, UINT8 sessionIndex, PTZ_MSG_t *pPtzMsg);
//-------------------------------------------------------------------------------------------------
static void ptzCtrlThreadCleanup(PTZ_COMMAND_INFO_t *pPtzCmdInfo);
//-------------------------------------------------------------------------------------------------
static void *ptzCtrlThread(void *threadArg);
//-------------------------------------------------------------------------------------------------
static void ptzControlTimerCb(UINT32 userData);
//-------------------------------------------------------------------------------------------------
static BOOL ValidatePtzFuncForBrandModel(CAMERA_BRAND_e brand, CAMERA_MODEL_e model, PTZ_FUNCTION_e ptzFunction);
//-------------------------------------------------------------------------------------------------
static void updateVideoLossStatus(UINT8 camIndex, BOOL status);
//-------------------------------------------------------------------------------------------------
static void setAllCameraDateTime(UINT32 data);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e setCameraDateTime(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
static void saveCameraImageCapability(UINT8 camIndex, IMAGE_CAPABILITY_INFO_t *pImageCapsInfo);
//-------------------------------------------------------------------------------------------------
static BOOL onvifImagingSettingsRespCb(ONVIF_RESPONSE_PARA_t *responseData);
//-------------------------------------------------------------------------------------------------
static void imagingSettingTimerSyncCb(UINT32 data);
//-------------------------------------------------------------------------------------------------
static BOOL getImageSettingCapability(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
static BOOL getImageSettingConfig(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
static void httpGetImageSettingCapabilityCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void httpGetImageSettingParamCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void tcpGetImageSettingCapabilityCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void tcpGetImageSettingParamCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static BOOL setImageSettingConfig(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
static void httpSetImageSettingParamCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void tcpSetImageSettingParamCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void resetMotionWindowConfig(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static TIMER_HANDLE                     setDateTimeToAllCamHandle = INVALID_TIMER_HANDLE;
static TIMER_HANDLE                     profileSchedulerTimerHandle = INVALID_TIMER_HANDLE;
static TIMER_HANDLE                     imagingSettingSyncTimerHandle = INVALID_TIMER_HANDLE;

static UINT8                            recordMainStreamBuffer[MAX_CAMERA][MAX_MAIN_STRM_BUFFER_SIZE];
static UINT8                            recordSubStreamBuffer[MAX_CAMERA][MAX_SUB_STRM_BUFFER_SIZE];

static CAMERA_CONNECTIVITY_t            camConnectivity[MAX_CAMERA];
static BOOL                             internetConnectivityStatus = INACTIVE;

static CAM_CONFIG_INFO_t				camConfigInfo[MAX_CAMERA];
static CAMERA_CONFIG_NOTIFY_CONTROL_t	camCnfgNotifyControl[MAX_CAMERA];

static STREAM_INFO_t                    streamInfo[MAX_CAMERA][MAX_STREAM_TYPE];
static CAMERA_REQUEST_t                 storePtzRequest[MAX_CAMERA];
static CAMERA_REQUEST_t                 getImageRequest[MAX_CAMERA];
static CAMERA_REQUEST_t                 setPtzRequest[MAX_CAMERA];
static CAMERA_REQUEST_t                 gotoPtzRequest[MAX_CAMERA];
static CAMERA_REQUEST_t                 setFocusRequest[MAX_CAMERA];
static CAMERA_REQUEST_t                 setIrisRequest[MAX_CAMERA];
static OSD_PARAM_REQ_t                  setOsdsRequest[MAX_CAMERA];
static DATE_TIME_REQ_t                  setDateTimeRequest[MAX_CAMERA];
static CAMERA_REQUEST_t                 twoWayAudioRequest[MAX_CAMERA];
static IMAGE_SETTING_REQ_t              imageSettingRequest[MAX_CAMERA];

static CAMERA_REQUEST_t                 cameraGetProfileParam[MAX_CAMERA][MAX_STREAM];
static PROFILE_SHEDULAR_PROCESS_PARAM_t profileShedularProcParam[MAX_CAMERA][MAX_STREAM];

static MOTION_WINDOW_REQ_t              motionWindowRequest[MAX_CAMERA];
static PRIVACY_WINDOW_REQ_t             privacyMaskRequest[MAX_CAMERA];

static CHANGE_CAM_IP_ADDR_t             changeCamIpRequest[MAX_CAMERA];

static ALARM_ACTION_REQUEST_t           alarmActionRequest[MAX_CAMERA][MAX_CAMERA_ALARM];
static ONVIF_CAM_DETAIL_t               onvifCamDetail[MAX_CAMERA];
static PTZ_COMMAND_INFO_t               ptzList[MAX_CAMERA];
static PTZ_FUNC_PAUSE_t                 ptzPauseState[MAX_CAMERA];

// Inverted state of video loss 0 = video Loss, 1 = stream
static BOOL                             videoLossStatus[MAX_CAMERA];
static pthread_mutex_t                  videoLossMutex = PTHREAD_MUTEX_INITIALIZER;

static const ONVIF_CAMERA_CB onvifSetupCallback[ONVIF_GET_MEDIA_URL] =
{
    onvifDateTimeCb,
    onvifDeviceEntryPointCb,
    onvifBrandModelCb,
    onvifCamCapabilityCb,
    onvifSetDateTimeCb,
};

static const CHARPTR alarmReqStr[2] =
{
    "DEACTIVATE",
    "ACTIVATE",
};

static const CHARPTR camReqStructStr[MAX_CAM_REQ_STRUCT_TYPE] =
{
    "PTZ",
    "STORE PRESET",
    "GO TO PRESET",
    "FOCUS",
    "IRIS",
    "OSD",
};

static const CHARPTR ciStreamExtTriggerStr[MAX_CI_STREAM_EXT_TRG] =
{
    "START_STREAM",
    "STOP_STREAM",
    "CONTROL_CB",
    "MEDIA_CB",
    "CONFIG_CHANGE",
    "CONNECTIVITY",
    "TIMER_CB",
};

static const CHARPTR ciStreamStateStr[MAX_CI_STREAM_STATE] =
{
    "OFF",
    "READY",
    "ON_WAIT",
    "ON",
    "OFF_WAIT",
    "RETRY",
    "RESTART",
};

static const CHARPTR httpRespStr[MAX_HTTP_RESPONSE] =
{
    "CLOSE_ON_SUCCESS",
    "CLOSE_ON_ERROR",
    "SUCCESS",
    "ERROR",
};

//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initializes camera interface module. This includes initialization of variable
 *          for different request types. It also starts monitoring connectivity of enabled cameras.
 */
void InitCameraInterface(void)
{
    UINT8 					cameraIndex, loop, requestCount;
    CAMERA_CONFIG_t			cameraConfig;
    IP_CAMERA_CONFIG_t		ipCamCfg;
    STREAM_CONFIG_t			streamConfig;
    TIMER_INFO_t            timerInfo;

    /* Initializing Camera interface related module */
    InitCameraConfig();
    InitCameraSearch();
    InitAdvCameraSearch();
    InitAutoCameraConfig();

    /* Init internet connectivity params */
    internetConnectivityStatus = INACTIVE;

    /* Init all required variables with default values */
    for(cameraIndex = 0; cameraIndex < getMaxCameraForCurrentVariant(); cameraIndex++)
    {
        ReadSingleCameraConfig(cameraIndex, &cameraConfig);
        camConnectivity[cameraIndex].connFd	= INVALID_CONNECTION;
        camConnectivity[cameraIndex].disconnCount = DFLT_CAMERA_DISCONN_COUNT;
        memset(camConnectivity[cameraIndex].ipAddr, '\0', sizeof(camConnectivity[cameraIndex].ipAddr));
        MUTEX_INIT(camConnectivity[cameraIndex].ipAddrLock, NULL);

        /* Initialization of stream related variables */
        for (loop = 0; loop < MAX_STREAM_TYPE; loop++)
        {
            MUTEX_INIT(streamInfo[cameraIndex][loop].currStateLock, NULL);
            streamInfo[cameraIndex][loop].currState = CI_STREAM_STATE_OFF;
            streamInfo[cameraIndex][loop].forceConfigOnCamera = FALSE;

            if (loop == MAIN_STREAM)
            {
                /* Set main stream record buffer */
                streamInfo[cameraIndex][loop].bufferPtr = recordMainStreamBuffer[cameraIndex];
                streamInfo[cameraIndex][loop].bufferSize = MAX_MAIN_STRM_BUFFER_SIZE;
            }
            else
            {
                /* Set sub stream record buffer */
                streamInfo[cameraIndex][loop].bufferPtr = recordSubStreamBuffer[cameraIndex];
                streamInfo[cameraIndex][loop].bufferSize = MAX_SUB_STRM_BUFFER_SIZE;
            }

            streamInfo[cameraIndex][loop].gopTotal = 0;
            streamInfo[cameraIndex][loop].fpsTotal = 0;
            streamInfo[cameraIndex][loop].fHeight = 0;
            streamInfo[cameraIndex][loop].fWidth = 0;
            streamInfo[cameraIndex][loop].lastFrameTimeSec = 0;
            streamInfo[cameraIndex][loop].recStrmRetryCnt = 0;
            streamInfo[cameraIndex][loop].offWaitUnHandleCnt = 0;
            streamInfo[cameraIndex][loop].configFrameRate = 0;

            pthread_rwlock_init(&streamInfo[cameraIndex][loop].frameMarker.writeIndexLock, NULL);
            MUTEX_INIT(streamInfo[cameraIndex][loop].frameMarker.writeBuffLock, NULL);

            /* Initialize write index of live stream frame marker to zero */
            streamInfo[cameraIndex][loop].frameMarker.wrPos = 0;

            /* Initialize max write index of live stream frame marker to one to avoid arithmatic exception in modulo */
            streamInfo[cameraIndex][loop].frameMarker.maxWriteIndex = 1;

            /* Initialize last I-frame write index of live stream */
            streamInfo[cameraIndex][loop].frameMarker.lastIframe = 0;

            /* Initialize address at 0th index to frame buffer head */
            streamInfo[cameraIndex][loop].frameMarker.frameInfo[0].framePtr = streamInfo[cameraIndex][loop].bufferPtr;
            for (requestCount = 0; requestCount < MAX_CI_STREAM_CLIENT; requestCount++)
            {
                streamInfo[cameraIndex][loop].clientCb[requestCount] = NULL;
            }

            /*******************************************************************************/
            initCameraRequestParam(cameraIndex, &streamInfo[cameraIndex][loop].getStreamRequest);
            streamInfo[cameraIndex][loop].getStreamRequest.httpCallback[CAM_REQ_CONTROL] = httpStreamControlCb;
            streamInfo[cameraIndex][loop].getStreamRequest.httpCallback[CAM_REQ_MEDIA] = httpStreamMediaCb;
            streamInfo[cameraIndex][loop].getStreamRequest.httpCallback[CAM_REQ_CONFIG] = HttpStreamConfigCb;
            streamInfo[cameraIndex][loop].retryTimerHandle = INVALID_TIMER_HANDLE;
            streamInfo[cameraIndex][loop].streamRetryRunning = FALSE;
            resetPtsAvgParam(&streamInfo[cameraIndex][loop]);
        }

        /*******************************************************************************/
        initCameraRequestParam(cameraIndex, &getImageRequest[cameraIndex]);
        getImageRequest[cameraIndex].httpCallback[CAM_REQ_CONTROL] = httpImageControlCb;
        getImageRequest[cameraIndex].httpCallback[CAM_REQ_MEDIA] = httpImageMediaCb;

        /*******************************************************************************/
        initCameraRequestParam(cameraIndex, &setPtzRequest[cameraIndex]);
        setPtzRequest[cameraIndex].httpCallback[CAM_REQ_CONTROL] = httpSetPtzControlCb;

        /*******************************************************************************/
        initCameraRequestParam(cameraIndex, &storePtzRequest[cameraIndex]);
        storePtzRequest[cameraIndex].httpCallback[CAM_REQ_CONTROL] = httpStorePtzControlCb;

        /*******************************************************************************/
        initCameraRequestParam(cameraIndex, &gotoPtzRequest[cameraIndex]);
        gotoPtzRequest[cameraIndex].httpCallback[CAM_REQ_CONTROL] = httpGotoPtzControlCb;

        /*******************************************************************************/
        initCameraRequestParam(cameraIndex, &setFocusRequest[cameraIndex]);
        setFocusRequest[cameraIndex].httpCallback[CAM_REQ_CONTROL] = httpSetFocusControlCb;

        /*******************************************************************************/
        initCameraRequestParam(cameraIndex, &setIrisRequest[cameraIndex]);
        setIrisRequest[cameraIndex].httpCallback[CAM_REQ_CONTROL] = httpSetIrisControlCb;

        /*******************************************************************************/
        initCameraRequestParam(cameraIndex, &setOsdsRequest[cameraIndex].request);
        setOsdsRequest[cameraIndex].request.httpCallback[CAM_REQ_CONTROL] = httpSetOsdControlCb;
        setOsdsRequest[cameraIndex].isOsdSetPending = FALSE;
        memset(&setOsdsRequest[cameraIndex].osdParam, 0, sizeof(setOsdsRequest[cameraIndex].osdParam));

        /*******************************************************************************/
        initCameraRequestParam(cameraIndex, &setDateTimeRequest[cameraIndex].request);
        setDateTimeRequest[cameraIndex].request.httpCallback[CAM_REQ_CONTROL] = httpSetDateTimeCb;
        setDateTimeRequest[cameraIndex].needToSetPwd = FALSE;

        /*******************************************************************************/
        initCameraRequestParam(cameraIndex, &twoWayAudioRequest[cameraIndex]);

        /*******************************************************************************/
        initCameraRequestParam(cameraIndex, &motionWindowRequest[cameraIndex].request);
        motionWindowRequest[cameraIndex].request.httpCallback[CAM_REQ_GET_WINDOW] = httpGetMotionRequestCb;
        motionWindowRequest[cameraIndex].request.httpCallback[CAM_REQ_SET_WINDOW] = httpSetMotionRequestCb;
        memset(&motionWindowRequest[cameraIndex].motionParam, 0, sizeof(motionWindowRequest[cameraIndex].motionParam));
        restoreDefaultMotionConfig(cameraIndex);

        /*******************************************************************************/
        initCameraRequestParam(cameraIndex, &privacyMaskRequest[cameraIndex].request);
        privacyMaskRequest[cameraIndex].request.httpCallback[CAM_REQ_GET_WINDOW] = httpGetPrivacyMaskWindowRequestCb;
        privacyMaskRequest[cameraIndex].request.httpCallback[CAM_REQ_SET_WINDOW] = httpSetPrivacyMaskWindowRequestCb;
        privacyMaskRequest[cameraIndex].request.httpCallback[CAM_REQ_GET_MAXWIN] = httpGetMaxPrivacyMaskWindowRequestCb;
        privacyMaskRequest[cameraIndex].maxSupportedMaskWindow = 0;
        memset(privacyMaskRequest[cameraIndex].privacyMaskName, '\0', sizeof(privacyMaskRequest[cameraIndex].privacyMaskName));
        restoreDefaultPrivacyMaskConfig(cameraIndex);

        /*******************************************************************************/
        initCameraRequestParam(cameraIndex, &imageSettingRequest[cameraIndex].request);
        imageSettingRequest[cameraIndex].request.httpCallback[CAM_REQ_IMG_APPEARANCE] = httpGetImageSettingCapabilityCb;
        imageSettingRequest[cameraIndex].request.httpCallback[CAM_REQ_IMG_ADV_APPEARANCE] = httpGetImageSettingCapabilityCb;
        imageSettingRequest[cameraIndex].request.httpCallback[CAM_REQ_IMG_DAY_NIGHT] = httpGetImageSettingCapabilityCb;
        memset(&imageSettingRequest[cameraIndex].imageCapsInfo, 0, sizeof(imageSettingRequest[cameraIndex].imageCapsInfo));
        MUTEX_INIT(imageSettingRequest[cameraIndex].setCnfgToCameraLock, NULL);
        imageSettingRequest[cameraIndex].setCnfgToCameraF = TRUE;

        /*******************************************************************************/
        for(loop = 0; loop < MAX_STREAM; loop++)
        {
            initCameraRequestParam(cameraIndex, &cameraGetProfileParam[cameraIndex][loop]);
            cameraGetProfileParam[cameraIndex][loop].httpCallback[CAM_REQ_CONFIG] = HttpStreamConfigCb;
        }

        /*******************************************************************************/
        initCameraRequestParam(cameraIndex, &changeCamIpRequest[cameraIndex].request);
        changeCamIpRequest[cameraIndex].request.httpCallback[CAM_REQ_CONTROL] = httpChangeCamIpAddrCb;
        changeCamIpRequest[cameraIndex].request.httpCallback[CAM_REQ_REBOOT] = httpRebootIpCamCb;
        restoreDfltCamIpChangeReqParam(cameraIndex);

        /******* Initialization of set alarm function *********************************/
        for(loop = 0; loop < MAX_CAMERA_ALARM; loop++)
        {
            initCameraRequestParam(cameraIndex, &alarmActionRequest[cameraIndex][loop].request);
            alarmActionRequest[cameraIndex][loop].request.httpCallback[INACTIVE] = httpAlarmDeactivateControlCb;
            alarmActionRequest[cameraIndex][loop].request.httpCallback[ACTIVE] = httpAlarmActivateControlCb;
            alarmActionRequest[cameraIndex][loop].timerHandle = INVALID_TIMER_HANDLE;
            MUTEX_INIT(alarmActionRequest[cameraIndex][loop].reqStatusLock, NULL);
            alarmActionRequest[cameraIndex][loop].reqStatus[ACTIVE] = FREE;
            alarmActionRequest[cameraIndex][loop].reqStatus[INACTIVE] = FREE;
        }

        /****************** Initialize ONVIF related parameters ****************/
        MUTEX_INIT(onvifCamDetail[cameraIndex].onvifAccessLock, NULL);
        onvifCamDetail[cameraIndex].initialSetupF = FAIL;
        onvifCamDetail[cameraIndex].authFailF = FALSE;
        onvifCamDetail[cameraIndex].entryPointReceived = NO;
        onvifCamDetail[cameraIndex].prevRequest = ONVIF_REQUEST_MAX;
        onvifCamDetail[cameraIndex].retryCount = 0;
        onvifCamDetail[cameraIndex].onvifRetryTimerHandle = INVALID_TIMER_HANDLE;
        onvifCamDetail[cameraIndex].onvifStrmUrlRetryTimerHandle[MAIN_STREAM] = INVALID_TIMER_HANDLE;
        onvifCamDetail[cameraIndex].onvifStrmUrlRetryTimerHandle[SUB_STREAM] = INVALID_TIMER_HANDLE;
        memset(onvifCamDetail[cameraIndex].onvifCamBrand, '\0', MAX_BRAND_NAME_LEN);
        memset(onvifCamDetail[cameraIndex].onvifCamModel, '\0', MAX_MODEL_NAME_LEN);

        /*********** Get camera brand-model from the config  ******************/
        pthread_rwlock_init(&camConfigInfo[cameraIndex].configInfoLock, NULL);
        camConfigInfo[cameraIndex].configChangeNotify = TRUE;
        camConfigInfo[cameraIndex].cameraType = cameraConfig.type;

        ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);
        camConfigInfo[cameraIndex].onvifSupportF = ipCamCfg.onvifSupportF;
        if(ipCamCfg.onvifSupportF == TRUE)
        {
            camConfigInfo[cameraIndex].brand = (CAMERA_BRAND_e)ONVIF_CAMERA;
            camConfigInfo[cameraIndex].model = (CAMERA_MODEL_e)ONVIF_CAMERA;
        }
        else
        {
            if(GetBrandNum(ipCamCfg.brand, &(camConfigInfo[cameraIndex].brand)) == SUCCESS)
            {
                GetModelNum(ipCamCfg.brand, ipCamCfg.model, &(camConfigInfo[cameraIndex].model));
            }
            else
            {
                camConfigInfo[cameraIndex].brand = CAMERA_BRAND_NONE;
                camConfigInfo[cameraIndex].model = CAMERA_MODEL_NONE;
            }
        }

        //if camera Analog no need to Retrieve brand model number
        ReadSingleStreamConfig(cameraIndex, &streamConfig);
        GetVideoCodecNum(streamConfig.videoEncoding, &(camConfigInfo[cameraIndex].videoCodec[MAIN_STREAM]));
        GetVideoCodecNum(streamConfig.videoEncodingSub, &(camConfigInfo[cameraIndex].videoCodec[SUB_STREAM]));
        GetResolutionNum(streamConfig.resolution, &(camConfigInfo[cameraIndex].resolution[MAIN_STREAM]));
        GetResolutionNum(streamConfig.resolutionSub, &(camConfigInfo[cameraIndex].resolution[SUB_STREAM]));

        /********** Initialize PTZ Control parameters **************************/
        ptzList[cameraIndex].cameraIndex = cameraIndex;
        memset(&ptzList[cameraIndex].ptzMagQue, 0, sizeof(PTZ_MSG_QUE_t));
        MUTEX_INIT(ptzList[cameraIndex].ptzMutex, NULL);
        MUTEX_INIT(ptzList[cameraIndex].ptzSignalLock, NULL);
        pthread_cond_init(&ptzList[cameraIndex].ptzSignal, NULL);
        ptzList[cameraIndex].runningStatus = FALSE;

        /*********** Initialize PTZ Pause Control parameters *******/
        ptzPauseState[cameraIndex].ptzFunctionPause = FALSE;
        MUTEX_INIT(ptzPauseState[cameraIndex].ptzPauseLock, NULL);

        pthread_rwlock_init(&camCnfgNotifyControl[cameraIndex].configNotifyLock,NULL);
        camCnfgNotifyControl[cameraIndex].cameraConfigNotifyF = TRUE;
        videoLossStatus[cameraIndex] = TRUE;

        for(loop = 0; loop < MAX_STREAM; loop++)
        {
            MUTEX_INIT(profileShedularProcParam[cameraIndex][loop].profileSchedularProcRunLock, NULL);
            profileShedularProcParam[cameraIndex][loop].profileSchedularProcRunF = FALSE;
        }
    }

    if (FAIL == Utils_CreateThread(NULL, connMonitorThread, NULL, DETACHED_THREAD, CONN_MONITORT_THREAD_STACK_SZ))
    {
        EPRINT(CAMERA_INTERFACE, "fail to create connectivity thread: [camera=%d]", cameraIndex);
    }

    /* start timer for schedule date-time sync */
    timerInfo.count = CONVERT_SEC_TO_TIMER_COUNT(2*60*60);
    timerInfo.data = 0;
    timerInfo.funcPtr = setAllCameraDateTime;
    StartTimer(timerInfo, &setDateTimeToAllCamHandle);

    /* start timer for schedule stream profile sync */
    timerInfo.count = CONVERT_SEC_TO_TIMER_COUNT(PROFILE_UPDATE_TIME*60);
    timerInfo.data = 0;
    timerInfo.funcPtr = profileSchedulerCb;
    StartTimer(timerInfo, &profileSchedulerTimerHandle);

    /* Start timer for schedule imaging setting sync */
    timerInfo.count = CONVERT_SEC_TO_TIMER_COUNT((IMAGE_SETTING_UPDATE_TIME + POWER_ON_IMAGE_SETTING_DELAY_TIME)*60);
    timerInfo.data = 0;
    timerInfo.funcPtr = imagingSettingTimerSyncCb;
    StartTimer(timerInfo, &imagingSettingSyncTimerHandle);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Init camera request param with default params
 * @param   cameraIndex
 * @param   pCameraRequest
 */
static void initCameraRequestParam(UINT8 cameraIndex, CAMERA_REQUEST_t *pCameraRequest)
{
    UINT8 loopCnt;

    /* Init camera request with default values */
    pCameraRequest->cameraIndex = cameraIndex;
    pCameraRequest->numOfRequest = 0;
    pCameraRequest->requestCount = 0;
    pCameraRequest->requestStatus = CMD_SUCCESS;
    pCameraRequest->camReqBusyF = FREE;

    MUTEX_INIT(pCameraRequest->camReqFlagLock, NULL);
    pCameraRequest->rtspHandle = INVALID_RTSP_HANDLE;
    pCameraRequest->mediaCallback = NULL;
    pCameraRequest->timerHandle = INVALID_TIMER_HANDLE;
    pCameraRequest->clientCbType = CLIENT_CB_TYPE_MAX;

    for (loopCnt = 0; loopCnt < MAX_CLIENT_CB; loopCnt++)
    {
        pCameraRequest->clientCb[loopCnt] = NULL;
        pCameraRequest->clientSocket[loopCnt] = INVALID_CONNECTION;
    }

    for (loopCnt = 0; loopCnt < MAX_URL_REQUEST; loopCnt++)
    {
        pCameraRequest->http[loopCnt] = INVALID_HTTP_HANDLE;
        memset(&pCameraRequest->url[loopCnt], 0, sizeof(URL_REQUEST_t));
    }

    for (loopCnt = 0; loopCnt < MAX_CAM_REQ_TYPE; loopCnt++)
    {
        pCameraRequest->httpCallback[loopCnt] = NULL;
        pCameraRequest->tcpCallback[loopCnt] = NULL;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetCameraBrandModelNumber : to get brand and model for camera
 * @param cameraIndex
 * @param brand
 * @param model
 */
void GetCameraBrandModelNumber(UINT8 cameraIndex, CAMERA_BRAND_e *brand, CAMERA_MODEL_e *model)
{
    pthread_rwlock_rdlock(&camConfigInfo[cameraIndex].configInfoLock);
    if(brand != NULL)
    {
        *brand = camConfigInfo[cameraIndex].brand;
    }

    if(model != NULL)
    {
        *model = camConfigInfo[cameraIndex].model;
    }
    pthread_rwlock_unlock(&camConfigInfo[cameraIndex].configInfoLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief OnvifSupportF : checks if camera have onvif supported
 * @param cameraIndex
 * @return TRUE/FALSE
 */
BOOL OnvifSupportF(UINT8 cameraIndex)
{
    pthread_rwlock_rdlock(&camConfigInfo[cameraIndex].configInfoLock);
    BOOL onvifSupportF = camConfigInfo[cameraIndex].onvifSupportF;
    pthread_rwlock_unlock(&camConfigInfo[cameraIndex].configInfoLock);
    return onvifSupportF;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief CameraType : checks camera type i.e onvif,brandmodel or cam init
 * @param cameraIndex
 * @return
 */
CAMERA_TYPE_e CameraType(UINT8 cameraIndex)
{
    pthread_rwlock_rdlock(&camConfigInfo[cameraIndex].configInfoLock);
    CAMERA_TYPE_e cameraType = camConfigInfo[cameraIndex].cameraType;
    pthread_rwlock_unlock(&camConfigInfo[cameraIndex].configInfoLock);
    return cameraType;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief getCameraCodecNum : for given stream type it provides configured codec
 * @param cameraIndex
 * @param streamType
 * @param codecNo
 */
static void getCameraCodecNum(UINT8 cameraIndex, VIDEO_TYPE_e streamType, STREAM_CODEC_TYPE_e *codecNo)
{
    pthread_rwlock_rdlock(&camConfigInfo[cameraIndex].configInfoLock);
    *codecNo = camConfigInfo[cameraIndex].videoCodec[streamType];
    pthread_rwlock_unlock(&camConfigInfo[cameraIndex].configInfoLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get camera request structure
 * @param   reqType
 * @param   camIndex
 * @return  Pointer to camera request structure
 */
static CAMERA_REQUEST_t *getCameraReqStruct(CAM_REQ_STRUCT_TYPE_e reqType, UINT8 camIndex)
{
    switch(reqType)
    {
        case CAM_REQ_STRUCT_PTZ:
            return &setPtzRequest[camIndex];

        case CAM_REQ_STRUCT_STORE_PRESET:
            return &storePtzRequest[camIndex];

        case CAM_REQ_STRUCT_GOTO_PRESET:
            return &gotoPtzRequest[camIndex];

        case CAM_REQ_STRUCT_FOCUS:
            return &setFocusRequest[camIndex];

        case CAM_REQ_STRUCT_IRIS:
            return &setIrisRequest[camIndex];

        case CAM_REQ_STRUCT_SET_OSDS:
            return &setOsdsRequest[camIndex].request;

        default:
            return NULL;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is a thread function which monitors camera connectivity. Upon receipt of periodic
 *          signal from timer call-back function, it sends HTTP request to respective camera. It
 *          expects server to respond within particular time. If it receives response within time,
 *          camera is considered reachable. If it doesn’t receive any response within time,
 *          consecutively for the maximum attempt count, it declares the camera is not reachable.
 * @param   pThreadArg
 * @return
 */
static void *connMonitorThread(void *pThreadArg)
{
    UINT8 						cameraIndex;
    INT8 						cameraCounter, connectedCamera;
    CHAR                        camIpAddr[IPV6_ADDR_LEN_MAX];
    CAMERA_CONFIG_t 			cameraConfig[MAX_CAMERA];
    IP_CAMERA_CONFIG_t 			ipCamCfg[MAX_CAMERA];
    INT32 						connectedFd;
    INT32						sockOptVal;
    socklen_t 					sockOptLen;
    BOOL						connStatus[MAX_CAMERA] = { INACTIVE };
    CI_STREAM_EXT_TRG_PARAM_t	triggerParam;
    GENERAL_CONFIG_t		    generalConfig;
    UINT8 						videoLossDuration;
    IP_ADDR_TYPE_e              internetIpAddrType = IP_ADDR_TYPE_IPV4;
    const CHAR                  *pInternetCheckIpAddr[IP_ADDR_TYPE_MAX] = {INTERNET_CONNECTIVITY_CHECK_IPV4, INTERNET_CONNECTIVITY_CHECK_IPV6};
    INT32                       internetConnectivitySockFd = INVALID_CONNECTION;
    INT8                        internetDisconnectCount = 0;
    BOOL                        internetConnStatus = INACTIVE;
    struct pollfd               pollFds[MAX_CAMERA + 1];
    UINT64                      prevTimeMs;

    THREAD_START("CONN_MONITOR");

    /* Super loop */
    while (TRUE)
    {
        /* Init all elements of pollFds with invalid value */
        for (cameraIndex = 0; cameraIndex < MAX_CAMERA + 1; cameraIndex++)
        {
            INIT_POLL_FD(pollFds[cameraIndex]);
        }

        /* Read camera and general configuration */
        ReadGeneralConfig(&generalConfig);
        ReadCameraConfig(cameraConfig);
        ReadIpCameraConfig(ipCamCfg);

        /* Convert counting seconds into count */
        videoLossDuration = generalConfig.preVideoLossDuration / MAX_CONN_CHECK_PERIOD;
        if ((generalConfig.preVideoLossDuration % MAX_CONN_CHECK_PERIOD) > 0)
        {
            videoLossDuration += 1;
        }

        /* Init with default value and check for all enabled/configured cameras */
        cameraCounter = 0;
        for(cameraIndex = 0; cameraIndex < getMaxCameraForCurrentVariant(); cameraIndex++)
        {
            /* Is camera disabled? */
            if (cameraConfig[cameraIndex].camera == DISABLE)
            {
                connStatus[cameraIndex] = INACTIVE;
                resetCamIpAddress(cameraIndex);
                continue;
            }

            /* Is it auto added camera? */
            if (AUTO_ADDED_CAMERA == cameraConfig[cameraIndex].type)
            {
                connStatus[cameraIndex] = (BOOL)GetCameraConnectionStatus(cameraIndex);
                continue;
            }

            /* Is it not an ip camera (onvif or BM)? */
            if (cameraConfig[cameraIndex].type != IP_CAMERA)
            {
                continue;
            }

            /* Get ip address and If it is blank then resolve it */
            MUTEX_LOCK(camConnectivity[cameraIndex].ipAddrLock);
            snprintf(camIpAddr, sizeof(camIpAddr), "%s", camConnectivity[cameraIndex].ipAddr);
            MUTEX_UNLOCK(camConnectivity[cameraIndex].ipAddrLock);
            if (camIpAddr[0] == '\0')
            {
                camConnectivity[cameraIndex].disconnCount = DFLT_START_DISCONN_COUNT;
                connStatus[cameraIndex] = INACTIVE;

                /* Get ip address from domain */
                if (FAIL == GetIpAddrFromDomainName(ipCamCfg[cameraIndex].cameraAddress, IP_ADDR_TYPE_MAX, camIpAddr))
                {
                    EPRINT(CAMERA_INTERFACE, "fail to get ip from url: [camera=%d], [ip=%s]", cameraIndex, ipCamCfg[cameraIndex].cameraAddress);
                    continue;
                }

                MUTEX_LOCK(camConnectivity[cameraIndex].ipAddrLock);
                snprintf(camConnectivity[cameraIndex].ipAddr, sizeof(camConnectivity[cameraIndex].ipAddr), "%s", camIpAddr);
                MUTEX_UNLOCK(camConnectivity[cameraIndex].ipAddrLock);
                DPRINT(CAMERA_INTERFACE, "camera addr resolved: [camera=%d], [ip=%s]", cameraIndex, camIpAddr);
            }

            /* Create non-blocking tcp socket with camera ip address and http/onvif port */
            if (FALSE == CreateNonBlockConnection(camIpAddr, (ipCamCfg[cameraIndex].onvifSupportF == TRUE) ?
                                                  ipCamCfg[cameraIndex].onvifPort : ipCamCfg[cameraIndex].httpPort, &camConnectivity[cameraIndex].connFd))
            {
                if(camConnectivity[cameraIndex].disconnCount < videoLossDuration)
                {
                    /* If camera disconnect count is greater than maximum disconnect count */
                    camConnectivity[cameraIndex].disconnCount++;
                    if(camConnectivity[cameraIndex].disconnCount >= videoLossDuration)
                    {
                        connStatus[cameraIndex] = INACTIVE;
                    }
                }
            }
            else
            {
                /* Increment fd counter, Add socket to poll fd set */
                cameraCounter++;
                pollFds[cameraIndex].fd = camConnectivity[cameraIndex].connFd;
                pollFds[cameraIndex].events = POLLWRNORM;
            }
        }

        /* Create non-blocking tcp socket with google's dns server for internet connectivity */
        if (FALSE == CreateNonBlockConnection(pInternetCheckIpAddr[internetIpAddrType], INTERNET_CONNECTIVITY_CHECK_PORT, &internetConnectivitySockFd))
        {
            internetConnStatus = INACTIVE;
            internetDisconnectCount = 0;
            internetIpAddrType = (internetIpAddrType == IP_ADDR_TYPE_IPV4) ? IP_ADDR_TYPE_IPV6 : IP_ADDR_TYPE_IPV4;
        }
        else
        {
            /* Increment fd counter, Add socket to poll fd set */
            cameraCounter++;
            pollFds[MAX_CAMERA].fd = internetConnectivitySockFd;
            pollFds[MAX_CAMERA].events = POLLWRNORM;
        }

        /* Wait for sometime to establish connections */
        sleep(MAX_CONN_CHECK_PERIOD);

        /* Loop until all enabled cameras are found connected OR maximum time-out occurs */
        connectedCamera = 0;
        prevTimeMs = 0;

        while (connectedCamera < cameraCounter)
        {
            /* If at least one socket is found accessible within maximum timeout divide by number of enabled camera */
            connectedFd = poll(pollFds, MAX_CAMERA + 1, GetRemainingPollTime(&prevTimeMs, MAX_CONN_CHECK_PERIOD_MS));
            if (connectedFd <= STATUS_OK)
            {
                break;
            }

            /* Update connected camera count */
            connectedCamera += connectedFd;

            /* Loop till maximum camera */
            for (cameraIndex = 0; cameraIndex < getMaxCameraForCurrentVariant(); cameraIndex++)
            {
                /* Nothing to do if socket is invalid */
                if (camConnectivity[cameraIndex].connFd == INVALID_CONNECTION)
                {
                    continue;
                }

                /* If no events on camera connection fd */
                if (pollFds[cameraIndex].revents == 0)
                {
                    continue;
                }

                INIT_POLL_FD(pollFds[cameraIndex]);

                /* If there is error in connection */
                sockOptLen = sizeof(sockOptVal);
                if(getsockopt(camConnectivity[cameraIndex].connFd, SOL_SOCKET, SO_ERROR, &sockOptVal, &sockOptLen) != STATUS_OK)
                {
                    CloseSocket(&camConnectivity[cameraIndex].connFd);
                    continue;
                }

                if (sockOptVal != STATUS_OK)
                {
                    if(camConnectivity[cameraIndex].disconnCount < videoLossDuration)
                    {
                        /* If camera disconnect count is greater than maximum disconnect count */
                        camConnectivity[cameraIndex].disconnCount++;
                        if(camConnectivity[cameraIndex].disconnCount >= videoLossDuration)
                        {
                            connStatus[cameraIndex] = INACTIVE;
                        }
                    }
                }
                else
                {
                    /* If camera disconnect count is greater than maximum disconnect count */
                    if(camConnectivity[cameraIndex].disconnCount >= MAX_CAMERA_DISCONN_COUNT)
                    {
                        camConnectivity[cameraIndex].disconnCount = DFLT_START_DISCONN_COUNT;
                    }

                    if(camConnectivity[cameraIndex].disconnCount >= MIN_CAMERA_DISCONN_COUNT)
                    {
                        /* If camera disconnect count is less than minimum disconnect count */
                        camConnectivity[cameraIndex].disconnCount--;
                        if(camConnectivity[cameraIndex].disconnCount <= MIN_CAMERA_DISCONN_COUNT)
                        {
                            connStatus[cameraIndex] = ACTIVE;
                            camConnectivity[cameraIndex].disconnCount = DFLT_CAMERA_DISCONN_COUNT;
                        }
                    }
                }

                /* CLose the socket */
                CloseSocket(&camConnectivity[cameraIndex].connFd);
            }

            /* Nothing to do if socket is invalid */
            if (internetConnectivitySockFd == INVALID_CONNECTION)
            {
                continue;
            }

            /* If no events on internet connection fd */
            if (pollFds[MAX_CAMERA].revents == 0)
            {
                continue;
            }

            INIT_POLL_FD(pollFds[MAX_CAMERA]);

            /* IF there is error in connection */
            sockOptLen = sizeof(sockOptVal);
            if (getsockopt(internetConnectivitySockFd, SOL_SOCKET, SO_ERROR, &sockOptVal, &sockOptLen) != STATUS_OK)
            {
                CloseSocket(&internetConnectivitySockFd);
                continue;
            }

            if (sockOptVal != STATUS_OK)
            {
                internetConnStatus = INACTIVE;
                internetDisconnectCount = 0;
            }
            else
            {
                internetDisconnectCount++;
                if(internetDisconnectCount >= 3)
                {
                    internetDisconnectCount = 0;
                    internetConnStatus = ACTIVE;
                }
            }

            /* Close the socket */
            CloseSocket(&internetConnectivitySockFd);
        }

        /* Loop till maximum camera */
        for (cameraIndex = 0; cameraIndex < getMaxCameraForCurrentVariant(); cameraIndex++)
        {
            /* If connection is timed out */
            if(camConnectivity[cameraIndex].connFd != INVALID_CONNECTION)
            {
                /* Close the socket */
                CloseSocket(&camConnectivity[cameraIndex].connFd);
                if (camConnectivity[cameraIndex].disconnCount < videoLossDuration)
                {
                    /* If camera disconnect count is greater than maximum disconnect count */
                    camConnectivity[cameraIndex].disconnCount++;
                    if(camConnectivity[cameraIndex].disconnCount >= videoLossDuration)
                    {
                        connStatus[cameraIndex] = INACTIVE;
                    }
                }
            }

            /* Is unknown camera type? */
            if ((cameraConfig[cameraIndex].type != IP_CAMERA) && (cameraConfig[cameraIndex].type != AUTO_ADDED_CAMERA))
            {
                continue;
            }

            /* No change in conection status */
            if (FALSE == EventDetectFunc(cameraIndex, CONNECTION_FAILURE, connStatus[cameraIndex]))
            {
                continue;
            }

            /* Set camera connectivity trigger parameters */
            triggerParam.camIndex = cameraIndex;
            triggerParam.connState = connStatus[cameraIndex];
            triggerParam.type = CI_STREAM_EXT_TRG_CONNECTIVITY;

            /* When Camera goes offline */
            if (connStatus[cameraIndex] == INACTIVE)
            {
                /* Reset camera address and other parameters */
                resetCamIpAddress(cameraIndex);
                MUTEX_LOCK(onvifCamDetail[cameraIndex].onvifAccessLock);
                onvifCamDetail[cameraIndex].initialSetupF = FAIL;
                onvifCamDetail[cameraIndex].authFailF = FALSE;
                MUTEX_UNLOCK(onvifCamDetail[cameraIndex].onvifAccessLock);

                /* Give connection trigger for main profile of camera */
                ciStreamExtTriggerHandler(&triggerParam);

                /* Give connection trigger for sub profile of camera */
                triggerParam.camIndex += getMaxCameraForCurrentVariant();
                ciStreamExtTriggerHandler(&triggerParam);
                continue;
            }

            /* When Camera comes online. Update Stream Video Loss status */
            updateVideoLossStatus(cameraIndex, TRUE);

            /* Give trigger to camera interface for BM and CI */
            if (ipCamCfg[cameraIndex].onvifSupportF == FALSE)
            {
                /* Give connection trigger for main profile of camera */
                ciStreamExtTriggerHandler(&triggerParam);

                /* Give connection trigger for sub profile of camera */
                triggerParam.camIndex += getMaxCameraForCurrentVariant();
                ciStreamExtTriggerHandler(&triggerParam);
                continue;
            }

            /* Start ONVIF camera online process */
            MUTEX_LOCK(onvifCamDetail[cameraIndex].onvifAccessLock);
            onvifCamDetail[cameraIndex].entryPointReceived = NO;
            onvifCamDetail[cameraIndex].prevRequest = ONVIF_REQUEST_MAX;
            onvifCamDetail[cameraIndex].retryCount = 0;
            MUTEX_UNLOCK(onvifCamDetail[cameraIndex].onvifAccessLock);

            /* If camera brand/model is not retrieved then call get date time */
            if ((onvifCamDetail[cameraIndex].onvifCamBrand[0] == '\0') || (onvifCamDetail[cameraIndex].onvifCamModel[0] == '\0'))
            {
                if (CMD_SUCCESS != sendOnvifCommonCmd(cameraIndex, ONVIF_GET_DATE_TIME, onvifDateTimeCb, &ipCamCfg[cameraIndex], NULL))
                {
                    startOnvifRetryTimer(cameraIndex, ONVIF_GET_DATE_TIME);
                }
            }
            else
            {
                if (CMD_SUCCESS != sendOnvifCommonCmd(cameraIndex, ONVIF_GET_BRAND_MODEL, onvifBrandModelCb, &ipCamCfg[cameraIndex], NULL))
                {
                    startOnvifRetryTimer(cameraIndex, ONVIF_GET_BRAND_MODEL);
                }
            }
        }

        /* If connection is timed out */
        if (internetConnectivitySockFd != INVALID_CONNECTION)
        {
            internetConnStatus = INACTIVE;
            internetDisconnectCount = 0;
            CloseSocket(&internetConnectivitySockFd);
        }

        /* Is internet connectivity status changed? */
        if (internetConnectivityStatus != internetConnStatus)
        {
            internetConnectivityStatus = internetConnStatus;
            WPRINT(CAMERA_INTERFACE, "internet status changed: [status=%s], [family=%s]",
                   internetConnStatus ? "connected" : "disconnected", (internetIpAddrType == IP_ADDR_TYPE_IPV4) ? "ipv4" : "ipv6");
        }
    }

    /* Exit from the thread */
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Calls ONVIF module API to send ONVIF req to camera
 * @param   pOnvifReq
 * @param   ipCamCfg
 * @return  Command status
 */
static NET_CMD_STATUS_e sendOnvifCommand(ONVIF_REQ_PARA_t *pOnvifReq, IP_CAMERA_CONFIG_t *pIpCamCfg)
{
    ONVIF_HANDLE        handle;
    CAMERA_CONFIG_t     cameraCfg;
    IP_CAMERA_CONFIG_t	ipCamCfg;
    UINT8               cameraIndex = GET_STREAM_INDEX(pOnvifReq->camIndex);

    ReadSingleCameraConfig(cameraIndex, &cameraCfg);
    if (cameraCfg.camera == DISABLE)
    {
        EPRINT(CAMERA_INTERFACE, "camera disabled for onvif request: [camera=%d], [onvifReq=%d]", pOnvifReq->camIndex, pOnvifReq->onvifReq);
        return CMD_CHANNEL_DISABLED;
    }

    if (GetCamEventStatus(cameraIndex, CONNECTION_FAILURE) == INACTIVE)
    {
        EPRINT(CAMERA_INTERFACE, "camera disconnected for onvif request: [camera=%d], [onvifReq=%d]", pOnvifReq->camIndex, pOnvifReq->onvifReq);
        return CMD_CAM_DISCONNECTED;
    }

    /* Check do we have ip camera config? */
    if (NULL == pIpCamCfg)
    {
        /* Read ip camera configuration */
        ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);
        pIpCamCfg = &ipCamCfg;
    }

    GetCamIpAddress(cameraIndex, pOnvifReq->camPara.ipAddr);
    pOnvifReq->camPara.port = pIpCamCfg->onvifPort;
    snprintf(pOnvifReq->camPara.name, MAX_CAMERA_USERNAME_WIDTH, "%s", pIpCamCfg->username);
    snprintf(pOnvifReq->camPara.pwd, MAX_CAMERA_PASSWORD_WIDTH, "%s", pIpCamCfg->password);

    if (pOnvifReq->onvifReq > ONVIF_GET_CAPABILITY)
    {
        MUTEX_LOCK(onvifCamDetail[cameraIndex].onvifAccessLock);
        if (onvifCamDetail[cameraIndex].initialSetupF == FAIL)
        {
            MUTEX_UNLOCK(onvifCamDetail[cameraIndex].onvifAccessLock);
            EPRINT(CAMERA_INTERFACE, "initial setup running for onvif request: [camera=%d], [onvifReq=%d]", pOnvifReq->camIndex, pOnvifReq->onvifReq);
            return CMD_ONVIF_CAM_CAPABILITY_ERROR;
        }
        MUTEX_UNLOCK(onvifCamDetail[cameraIndex].onvifAccessLock);
    }

    if (FAIL == StartOnvifClient(pOnvifReq, &handle))
    {
        EPRINT(CAMERA_INTERFACE, "fail to start onvif client for request: [camera=%d], [onvifReq=%d]", pOnvifReq->camIndex, pOnvifReq->onvifReq);
        return CMD_ONVIF_CAM_CAPABILITY_ERROR;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send Onvif common command which have only 3 args
 * @param   cameraIndex
 * @param   reqType
 * @param   callback
 * @param   pIpCamCfg
 * @param   pOnvifData
 * @return  Command status
 */
static NET_CMD_STATUS_e sendOnvifCommonCmd(UINT8 cameraIndex, ONVIF_REQUEST_e reqType, ONVIF_CAMERA_CB callback,
                                           IP_CAMERA_CONFIG_t *pIpCamCfg, VOIDPTR pOnvifData)
{
    ONVIF_REQ_PARA_t onvifReqPara;

    /* Set onvif parameters for request */
    onvifReqPara.camIndex = cameraIndex;
    onvifReqPara.onvifReq = reqType;
    onvifReqPara.onvifCallback = callback;
    onvifReqPara.pOnvifData = pOnvifData;

    /* Send onvif request */
    return sendOnvifCommand(&onvifReqPara, pIpCamCfg);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send stream related onvif command
 * @param   cameraIndex
 * @param   reqType
 * @param   callback
 * @param   pIpCamCfg
 * @param   profile
 * @param   forceCfg
 * @param   pStreamCfg
 * @return  Command status
 */
static NET_CMD_STATUS_e sendOnvifStreamCmd(UINT8 cameraIndex, ONVIF_REQUEST_e reqType, ONVIF_CAMERA_CB callback,
                                           IP_CAMERA_CONFIG_t *pIpCamCfg, UINT8 profile, BOOL forceCfg, STREAM_CONFIG_t *pStreamCfg)
{
    ONVIF_REQ_PARA_t onvifReqPara;

    /* Set onvif parameters for request */
    onvifReqPara.camIndex = cameraIndex;
    onvifReqPara.onvifReq = reqType;
    onvifReqPara.onvifCallback = callback;
    onvifReqPara.profileNum = profile;
    onvifReqPara.forceConfigOnCamera = forceCfg;
    if (pStreamCfg)
    {
        onvifReqPara.strmConfig = *pStreamCfg;
    }

    /* Send onvif request */
    return sendOnvifCommand(&onvifReqPara, pIpCamCfg);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send onvif action command
 * @param   cameraIndex
 * @param   reqType
 * @param   callback
 * @param   actionIndex
 * @param   actionStatus
 * @param   pPresetCfg
 * @return  Command status
 */
static NET_CMD_STATUS_e sendOnvifActionCmd(UINT8 cameraIndex, ONVIF_REQUEST_e reqType, ONVIF_CAMERA_CB callback,
                                           IP_CAMERA_CONFIG_t *pIpCamCfg, UINT8 actionIndex, BOOL actionStatus, PTZ_PRESET_CONFIG_t *pPresetCfg)
{
    ONVIF_REQ_PARA_t onvifReqPara;

    /* Set onvif parameters for request */
    onvifReqPara.camIndex = cameraIndex;
    onvifReqPara.onvifReq = reqType;
    onvifReqPara.onvifCallback = callback;
    onvifReqPara.actionInfo.index = actionIndex;
    onvifReqPara.actionInfo.actionStatus = actionStatus;
    if (pPresetCfg)
    {
        onvifReqPara.ptzPresetConfig = *pPresetCfg;
    }

    /* Send onvif request */
    return sendOnvifCommand(&onvifReqPara, pIpCamCfg);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send onvif ptz command
 * @param   cameraIndex
 * @param   reqType
 * @param   callback
 * @param   pan
 * @param   tilt
 * @param   zoom
 * @param 	action
 * @param   speed
 * @param   focus
 * @param   iris
 * @return  Command status
 */
static NET_CMD_STATUS_e sendOnvifPtzCmd(UINT8 cameraIndex, ONVIF_REQUEST_e reqType, ONVIF_CAMERA_CB callback, IP_CAMERA_CONFIG_t *pIpCamCfg,
                                        PTZ_OPTION_e pan, PTZ_OPTION_e tilt, PTZ_OPTION_e zoom, BOOL action, float speed, CAMERA_FOCUS_e focus, CAMERA_IRIS_e iris)
{
    ONVIF_REQ_PARA_t onvifReqPara;

    /* Set onvif parameters for request */
    onvifReqPara.camIndex = cameraIndex;
    onvifReqPara.onvifReq = reqType;
    onvifReqPara.onvifCallback = callback;
    onvifReqPara.ptzInfo.pan = pan;
    onvifReqPara.ptzInfo.tilt = tilt;
    onvifReqPara.ptzInfo.zoom = zoom;
    onvifReqPara.ptzInfo.action = action;
    onvifReqPara.ptzInfo.speed = speed;
    onvifReqPara.focusInfo = focus;
    onvifReqPara.irisInfo = iris;

    /* Send onvif request */
    return sendOnvifCommand(&onvifReqPara, pIpCamCfg);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Callback to Get Date time onvif command
 * @param   responseData
 * @return  SUCCESS/FAIL
 */
static BOOL onvifDateTimeCb(ONVIF_RESPONSE_PARA_t *responseData)
{
    if (responseData->response == ONVIF_CMD_TIMEOUT)
    {
        WPRINT(CAMERA_INTERFACE, "get date-time request timeout, retry timer started: [camera=%d]", responseData->cameraIndex);
        if (SUCCESS == startOnvifRetryTimer(responseData->cameraIndex, responseData->requestType))
        {
            return SUCCESS;
        }
    }
    else
    {
        if(responseData->response == ONVIF_CMD_SUCCESS)
        {
            DPRINT(CAMERA_INTERFACE, "get date-time request success, now get brand-model: [camera=%d]", responseData->cameraIndex);
        }
        else
        {
            WPRINT(CAMERA_INTERFACE, "get date-time request failed, now try brand-model: [camera=%d]", responseData->cameraIndex);
        }
    }

    /* Send Get Brand/Model request whether get date and time request get success/fail; even when get
     * date time got max time out response, send get Brand/Model request. */
    NET_CMD_STATUS_e requestStatus = sendOnvifCommonCmd(responseData->cameraIndex, ONVIF_GET_BRAND_MODEL, onvifBrandModelCb, NULL, NULL);
    if ((requestStatus != CMD_SUCCESS) && (requestStatus != CMD_CHANNEL_DISABLED))
    {
        /* If send onvif command gets failed then start timer */
        startOnvifRetryTimer(responseData->cameraIndex, ONVIF_GET_BRAND_MODEL);
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Callback to get device entry point onvif command
 * @param   responseData
 * @return  SUCCESS/FAIL
 */
static BOOL onvifDeviceEntryPointCb(ONVIF_RESPONSE_PARA_t *responseData)
{
    NET_CMD_STATUS_e requestStatus;

    /* Check camera index. It should be in valid range */
    if (responseData->cameraIndex >= getMaxCameraForCurrentVariant())
    {
        EPRINT(CAMERA_INTERFACE, "invld camera index found: [camera=%d]", responseData->cameraIndex);
        return FAIL;
    }

    if(responseData->response == ONVIF_CMD_SUCCESS)
    {
        DPRINT(CAMERA_INTERFACE, "get entry point request success, now get date-time: [camera=%d]", responseData->cameraIndex);
        MUTEX_LOCK(onvifCamDetail[responseData->cameraIndex].onvifAccessLock);
        onvifCamDetail[responseData->cameraIndex].entryPointReceived = YES;
        MUTEX_UNLOCK(onvifCamDetail[responseData->cameraIndex].onvifAccessLock);

        requestStatus = sendOnvifCommonCmd(responseData->cameraIndex, ONVIF_GET_DATE_TIME, onvifDateTimeCb, NULL, NULL);
        if ((requestStatus != CMD_SUCCESS) && (requestStatus != CMD_CHANNEL_DISABLED))
        {
            startOnvifRetryTimer(responseData->cameraIndex, ONVIF_GET_DATE_TIME);
        }

        /* Return with success */
        return SUCCESS;
    }

    if ((responseData->response == ONVIF_CMD_TIMEOUT) || (responseData->response == ONVIF_CMD_RETRY))
    {
        WPRINT(CAMERA_INTERFACE, "get entry point request timeout, retry timer started: [camera=%d]", responseData->cameraIndex);
        if (responseData->response == ONVIF_CMD_RETRY)
        {
            /* If we need to retry device discovery command then make its previous request invalid; so it doesn't update its retry count */
            MUTEX_LOCK(onvifCamDetail[responseData->cameraIndex].onvifAccessLock);
            onvifCamDetail[responseData->cameraIndex].prevRequest = ONVIF_RETRY_REQUEST;
            MUTEX_UNLOCK(onvifCamDetail[responseData->cameraIndex].onvifAccessLock);
        }

        /* Start device discovery timer again */
        if (SUCCESS == startOnvifRetryTimer(responseData->cameraIndex, responseData->requestType))
        {
            return SUCCESS;
        }
    }
    else
    {
        IP_CAMERA_CONFIG_t	ipCamCfg;
        EPRINT(CAMERA_INTERFACE, "get entry point request failed: [camera=%d]", responseData->cameraIndex);
        ReadSingleIpCameraConfig(responseData->cameraIndex, &ipCamCfg);
        memset(onvifCamDetail[responseData->cameraIndex].onvifCamBrand, '\0', MAX_BRAND_NAME_LEN);
        memset(onvifCamDetail[responseData->cameraIndex].onvifCamModel, '\0', MAX_MODEL_NAME_LEN);
        FreeCameraCapability(GET_STREAM_INDEX(responseData->cameraIndex));

        /* Clear image upload url so that in next image upload gets new url from camera */
        memset(getImageRequest[responseData->cameraIndex].url, 0, sizeof(getImageRequest[responseData->cameraIndex].url));
        if ((ipCamCfg.brand[0] != '\0') || (ipCamCfg.model[0] != '\0'))
        {
            EPRINT(CAMERA_INTERFACE, "entry point couldn't receive, config defaulted: [camera=%d]", responseData->cameraIndex);
            RESET_STR_BUFF(ipCamCfg.brand);
            RESET_STR_BUFF(ipCamCfg.model);
            WriteSingleIpCameraConfig(responseData->cameraIndex, &ipCamCfg);
        }

        /* Start date time timer again */
        if (SUCCESS == startOnvifRetryTimer(responseData->cameraIndex, ONVIF_GET_DATE_TIME))
        {
            return SUCCESS;
        }
    }

    /* Set param for get date-time request */
    requestStatus = sendOnvifCommonCmd(responseData->cameraIndex, ONVIF_GET_DATE_TIME, onvifDateTimeCb, NULL, NULL);
    if ((requestStatus != CMD_SUCCESS) && (requestStatus != CMD_CHANNEL_DISABLED))
    {
        /* If send onvif command gets failed then start timer */
        startOnvifRetryTimer(responseData->cameraIndex, ONVIF_GET_DATE_TIME);
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Callback to onvif get brand model command
 * @param   responseData
 * @return  SUCCESS/FAIL
 */
static BOOL onvifBrandModelCb(ONVIF_RESPONSE_PARA_t *responseData)
{
    IP_CAMERA_CONFIG_t			ipCamCfg;
    CHARPTR						brandNamePtr;
    CHARPTR						modelNamePtr;
    NET_CMD_STATUS_e			requestStatus;
    CI_STREAM_EXT_TRG_PARAM_t	triggerParam;
    BOOL 						startTmrStatus = SUCCESS;

    /* Check camera index. It should be in valid range */
    if (responseData->cameraIndex >= getMaxCameraForCurrentVariant())
    {
        EPRINT(CAMERA_INTERFACE, "invld camera index found: [camera=%d]", responseData->cameraIndex);
        return FAIL;
    }

    ReadSingleIpCameraConfig(responseData->cameraIndex, &ipCamCfg);
    if(responseData->response == ONVIF_CMD_TIMEOUT)
    {
        WPRINT(CAMERA_INTERFACE, "get brand-model request timeout, retry timer started: [camera=%d]", responseData->cameraIndex);
        startTmrStatus = startOnvifRetryTimer(responseData->cameraIndex, responseData->requestType);

        MUTEX_LOCK(onvifCamDetail[responseData->cameraIndex].onvifAccessLock);
        if((startTmrStatus == FAIL) && (onvifCamDetail[responseData->cameraIndex].entryPointReceived == NO))
        {
            MUTEX_UNLOCK(onvifCamDetail[responseData->cameraIndex].onvifAccessLock);
            WPRINT(CAMERA_INTERFACE, "entry point not received yet, requested: [camera=%d]", responseData->cameraIndex);
            startTmrStatus = SUCCESS;
            requestStatus = sendOnvifCommonCmd(responseData->cameraIndex, ONVIF_GET_ENTRY_POINT, onvifDeviceEntryPointCb, &ipCamCfg, NULL);
            if ((requestStatus != CMD_SUCCESS) && (requestStatus != CMD_CHANNEL_DISABLED))
            {
                startTmrStatus = startOnvifRetryTimer(responseData->cameraIndex, ONVIF_GET_ENTRY_POINT);
            }
        }
        else
        {
            MUTEX_UNLOCK(onvifCamDetail[responseData->cameraIndex].onvifAccessLock);
        }
    }
    else
    {
        if(responseData->response == ONVIF_CMD_SUCCESS)
        {
            DPRINT(CAMERA_INTERFACE, "get brand-model request success: [camera=%d]", responseData->cameraIndex);
            MUTEX_LOCK(onvifCamDetail[responseData->cameraIndex].onvifAccessLock);
            onvifCamDetail[responseData->cameraIndex].authFailF = FALSE;
            MUTEX_UNLOCK(onvifCamDetail[responseData->cameraIndex].onvifAccessLock);

            brandNamePtr = (((CHARPTR *)responseData->data)[ONVIF_CAM_BRAND_NAME][0] == '\0') ? "ONVIF" : ((CHARPTR *)responseData->data)[ONVIF_CAM_BRAND_NAME];
            modelNamePtr = (((CHARPTR *)responseData->data)[ONVIF_CAM_MODEL_NAME][0] == '\0') ? "ONVIF" : ((CHARPTR *)responseData->data)[ONVIF_CAM_MODEL_NAME];

            /* If camera config brand/model not available then do this */
            if((ipCamCfg.brand[0] == '\0') || (ipCamCfg.model[0] == '\0'))
            {
                DPRINT(CAMERA_INTERFACE, "get brand-model request success, now get capabilities: [camera=%d]", responseData->cameraIndex);
                snprintf(onvifCamDetail[responseData->cameraIndex].onvifCamBrand, MAX_BRAND_NAME_LEN, "%s", brandNamePtr);
                snprintf(onvifCamDetail[responseData->cameraIndex].onvifCamModel, MAX_MODEL_NAME_LEN, "%s", modelNamePtr);

                requestStatus = sendOnvifCommonCmd(responseData->cameraIndex, ONVIF_GET_CAPABILITY, onvifCamCapabilityCb, &ipCamCfg, NULL);
                if((requestStatus != CMD_SUCCESS) && (requestStatus != CMD_CHANNEL_DISABLED))
                {
                    startTmrStatus = startOnvifRetryTimer(responseData->cameraIndex, ONVIF_GET_CAPABILITY);
                }
            }
            else
            {
                //brand /models are changed for camera
                if((strcmp(ipCamCfg.brand, brandNamePtr) != STATUS_OK) || (strcmp(ipCamCfg.model, modelNamePtr) != STATUS_OK))
                {
                    DPRINT(CAMERA_INTERFACE, "camera brand-model changed: [camera=%d], [brand=%s-->%s], [model=%s-->%s]", responseData->cameraIndex,
                           ipCamCfg.brand, brandNamePtr, ipCamCfg.model, modelNamePtr);

                    snprintf(onvifCamDetail[responseData->cameraIndex].onvifCamBrand, MAX_BRAND_NAME_LEN, "%s", brandNamePtr);
                    snprintf(onvifCamDetail[responseData->cameraIndex].onvifCamModel, MAX_MODEL_NAME_LEN, "%s", modelNamePtr);
                    FreeCameraCapability(GET_STREAM_INDEX(responseData->cameraIndex));

                    // Clear image upload url so that in next image upload gets new url from camera
                    memset(getImageRequest[responseData->cameraIndex].url, 0, sizeof(getImageRequest[responseData->cameraIndex].url));

                    //make brand/model default Delete onvif camera capability
                    RESET_STR_BUFF(ipCamCfg.brand);
                    RESET_STR_BUFF(ipCamCfg.model);
                    WriteSingleIpCameraConfig(responseData->cameraIndex, &ipCamCfg);

                    MUTEX_LOCK(onvifCamDetail[responseData->cameraIndex].onvifAccessLock);
                    onvifCamDetail[responseData->cameraIndex].prevRequest = ONVIF_REQUEST_MAX;
                    onvifCamDetail[responseData->cameraIndex].retryCount = 0;
                    MUTEX_UNLOCK(onvifCamDetail[responseData->cameraIndex].onvifAccessLock);
                }
                // Camera brand & model matches with the previous
                else
                {
                    // If onvif detail doesn't have brand/model it is just after the power ON
                    // or camera is just enabled. Get capability for the camera(System is JUST ON)
                    if ((onvifCamDetail[responseData->cameraIndex].onvifCamBrand[0] == '\0') || (onvifCamDetail[responseData->cameraIndex].onvifCamModel[0] == '\0'))
                    {
                        DPRINT(CAMERA_INTERFACE, "system is just started, now get capabilities: [camera=%d]", responseData->cameraIndex);
                        snprintf(onvifCamDetail[responseData->cameraIndex].onvifCamBrand, MAX_BRAND_NAME_LEN, "%s", brandNamePtr);
                        snprintf(onvifCamDetail[responseData->cameraIndex].onvifCamModel, MAX_MODEL_NAME_LEN, "%s", modelNamePtr);

                        // If start onvif client fils with camera disabled, no need to retry
                        requestStatus = sendOnvifCommonCmd(responseData->cameraIndex, ONVIF_GET_CAPABILITY, onvifCamCapabilityCb, &ipCamCfg, NULL);
                        if((requestStatus != CMD_SUCCESS) && (requestStatus != CMD_CHANNEL_DISABLED))
                        {
                            startTmrStatus = startOnvifRetryTimer(responseData->cameraIndex, ONVIF_GET_CAPABILITY);
                        }

                        // If camera is disabled, clear onvif brand-model when camera will be enabled system will get its connectivity
                        // at that time it will request capabilities only if onvif bran-model are not available
                        else if(requestStatus == CMD_CHANNEL_DISABLED)
                        {
                            memset(onvifCamDetail[responseData->cameraIndex].onvifCamBrand, '\0', MAX_BRAND_NAME_LEN);
                            memset(onvifCamDetail[responseData->cameraIndex].onvifCamModel, '\0', MAX_MODEL_NAME_LEN);
                        }
                    }
                    else
                    {
                        //Capability already retrievd , can make request above cam capability
                        MUTEX_LOCK(onvifCamDetail[responseData->cameraIndex].onvifAccessLock);
                        onvifCamDetail[responseData->cameraIndex].initialSetupF = SUCCESS;
                        MUTEX_UNLOCK(onvifCamDetail[responseData->cameraIndex].onvifAccessLock);

                        triggerParam.camIndex = responseData->cameraIndex;
                        triggerParam.connState = CAM_REACHABLE;
                        triggerParam.type = CI_STREAM_EXT_TRG_CONNECTIVITY;

                        // Give connection trigger for main profile of camera
                        ciStreamExtTriggerHandler(&triggerParam);

                        // Give connection trigger for sub profile of camera
                        triggerParam.camIndex += getMaxCameraForCurrentVariant();
                        ciStreamExtTriggerHandler(&triggerParam);
                    }
                }
            }
        }
        else
        {
            // If camera request fails, check for the entry point. Entry point received flag will be reset whenever camera connectivity comes
            // to handle the possibility that the camera previously configured is replaced by some other camera
            // If get brand-model fails and entry point is not received, start the process considering it is new camera
            MUTEX_LOCK(onvifCamDetail[responseData->cameraIndex].onvifAccessLock);

            /* Added to display credential issue in UI */
            if (responseData->response == ONVIF_CMD_AUTHENTICATION_FAIL)
            {
                onvifCamDetail[responseData->cameraIndex].authFailF = TRUE;
            }

            if(onvifCamDetail[responseData->cameraIndex].entryPointReceived == NO)
            {
                MUTEX_UNLOCK(onvifCamDetail[responseData->cameraIndex].onvifAccessLock);
                WPRINT(CAMERA_INTERFACE, "entry point not received yet, requested: [camera=%d]", responseData->cameraIndex);
                requestStatus = sendOnvifCommonCmd(responseData->cameraIndex, ONVIF_GET_ENTRY_POINT, onvifDeviceEntryPointCb, &ipCamCfg, NULL);
                if((requestStatus != CMD_SUCCESS) && (requestStatus != CMD_CHANNEL_DISABLED))
                {
                    startTmrStatus = startOnvifRetryTimer(responseData->cameraIndex, ONVIF_GET_ENTRY_POINT);
                }
                else if(requestStatus == CMD_CHANNEL_DISABLED)
                {
                    memset(onvifCamDetail[responseData->cameraIndex].onvifCamBrand, '\0', MAX_BRAND_NAME_LEN);
                    memset(onvifCamDetail[responseData->cameraIndex].onvifCamModel, '\0', MAX_MODEL_NAME_LEN);
                }
            }
            else
            {
                onvifCamDetail[responseData->cameraIndex].prevRequest = ONVIF_REQUEST_MAX;
                onvifCamDetail[responseData->cameraIndex].retryCount = 0;
                MUTEX_UNLOCK(onvifCamDetail[responseData->cameraIndex].onvifAccessLock);
                EPRINT(CAMERA_INTERFACE, "get brand-model request failed with valid entry point: [camera=%d]", responseData->cameraIndex);
                memset(onvifCamDetail[responseData->cameraIndex].onvifCamBrand, '\0', MAX_BRAND_NAME_LEN);
                memset(onvifCamDetail[responseData->cameraIndex].onvifCamModel, '\0', MAX_MODEL_NAME_LEN);
                FreeCameraCapability(GET_STREAM_INDEX(responseData->cameraIndex));

                // Clear image upload url so that in next image upload gets new url from camera
                memset(getImageRequest[responseData->cameraIndex].url, 0, sizeof(getImageRequest[responseData->cameraIndex].url));

                // If camera brand & model of the camera were received previously and configured Reset camera brand-model in config
                if ((ipCamCfg.brand[0] != '\0') || (ipCamCfg.model[0] != '\0'))
                {
                    RESET_STR_BUFF(ipCamCfg.brand);
                    RESET_STR_BUFF(ipCamCfg.model);
                    WriteSingleIpCameraConfig(responseData->cameraIndex, &ipCamCfg);
                }
            }	// ELSE end - Entry point received
        }	// ELSE end - Request failed
    }	// ELSE end - response is other then timeout

    if(startTmrStatus == FAIL)
    {
        requestStatus = sendOnvifCommonCmd(responseData->cameraIndex, ONVIF_GET_DATE_TIME, onvifDateTimeCb, &ipCamCfg, NULL);
        if((requestStatus != CMD_SUCCESS) && (requestStatus != CMD_CHANNEL_DISABLED))
        {
            // If send onvif command gets failed then start timer
            startOnvifRetryTimer(responseData->cameraIndex, ONVIF_GET_DATE_TIME);
        }
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief onvifCamCapabilityCb : Callback to onvif Get capability command
 * @param responseData
 * @return SUCCESS/FAIL
 */
static BOOL onvifCamCapabilityCb(ONVIF_RESPONSE_PARA_t *responseData)
{
    NET_CMD_STATUS_e			requestStatus;
    IP_CAMERA_CONFIG_t			ipCamCfg;
    CI_STREAM_EXT_TRG_PARAM_t	triggerParam;

    if(responseData->response == ONVIF_CMD_TIMEOUT)
    {
        WPRINT(CAMERA_INTERFACE, "get capability request timeout, retry timer started: [camera=%d]", responseData->cameraIndex);
        startOnvifRetryTimer(responseData->cameraIndex, responseData->requestType);
        return SUCCESS;
    }

    ReadSingleIpCameraConfig(responseData->cameraIndex, &ipCamCfg);
    if(responseData->response == ONVIF_CMD_SUCCESS)
    {
        DPRINT(CAMERA_INTERFACE, "get capability request success: [camera=%d], [brand=%s], [model=%s]",  responseData->cameraIndex,
               onvifCamDetail[responseData->cameraIndex].onvifCamBrand, onvifCamDetail[responseData->cameraIndex].onvifCamModel);
        if((strlen(ipCamCfg.brand) == 0) || (strlen(ipCamCfg.model) == 0))
        {
            snprintf(ipCamCfg.brand, MAX_BRAND_NAME_LEN, "%s", onvifCamDetail[responseData->cameraIndex].onvifCamBrand);
            snprintf(ipCamCfg.model, MAX_MODEL_NAME_LEN, "%s", onvifCamDetail[responseData->cameraIndex].onvifCamModel);
            WriteSingleIpCameraConfig(responseData->cameraIndex, &ipCamCfg);
        }

        //make onvif in running state
        MUTEX_LOCK(onvifCamDetail[responseData->cameraIndex].onvifAccessLock);
        onvifCamDetail[responseData->cameraIndex].initialSetupF = SUCCESS;
        MUTEX_UNLOCK(onvifCamDetail[responseData->cameraIndex].onvifAccessLock);

        triggerParam.camIndex = responseData->cameraIndex;
        triggerParam.connState = CAM_REACHABLE;
        triggerParam.type = CI_STREAM_EXT_TRG_CONNECTIVITY;
        ciStreamExtTriggerHandler(&triggerParam);

        triggerParam.camIndex += getMaxCameraForCurrentVariant();
        ciStreamExtTriggerHandler(&triggerParam);
    }
    else
    {
        EPRINT(CAMERA_INTERFACE, "get capability request failed: [camera=%d]",  responseData->cameraIndex);
        memset(onvifCamDetail[responseData->cameraIndex].onvifCamBrand, '\0', MAX_BRAND_NAME_LEN);
        memset(onvifCamDetail[responseData->cameraIndex].onvifCamModel, '\0', MAX_MODEL_NAME_LEN);
        FreeCameraCapability(GET_STREAM_INDEX(responseData->cameraIndex));

        // Clear image upload url so that in next image upload gets new url from camera
        memset(getImageRequest[responseData->cameraIndex].url, 0, sizeof(getImageRequest[responseData->cameraIndex].url));

        if ((ipCamCfg.brand[0] != '\0') || (ipCamCfg.model[0] != '\0'))
        {
            WPRINT(CAMERA_INTERFACE, "camera capability not recv, default brand-model: [camera=%d]",  responseData->cameraIndex);
            RESET_STR_BUFF(ipCamCfg.brand);
            RESET_STR_BUFF(ipCamCfg.model);
            WriteSingleIpCameraConfig(responseData->cameraIndex, &ipCamCfg);
        }
        else
        {
            /* Send Get Brand/Model request whether get date and time request get success/fail;
             * even when get date time got max time out response ,send get Brand/Model request. */
            if(FAIL == startOnvifRetryTimer(responseData->cameraIndex, ONVIF_GET_DATE_TIME))
            {
                requestStatus = sendOnvifCommonCmd(responseData->cameraIndex, ONVIF_GET_DATE_TIME, onvifDateTimeCb, &ipCamCfg, NULL);
                if((requestStatus != CMD_SUCCESS) && (requestStatus != CMD_CHANNEL_DISABLED))
                {
                    // If send onvif command gets failed then start timer
                    startOnvifRetryTimer(responseData->cameraIndex, ONVIF_GET_DATE_TIME);
                }
            }
        }
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief startOnvifRetryTimer : it will start timer and on timeout it will sent failed onvif cmd
 * @param cameraIndex
 * @param request
 * @return SUCCESS/FAIL
 */
static BOOL startOnvifRetryTimer(UINT8 cameraIndex, ONVIF_REQUEST_e request)
{
    if (cameraIndex >= getMaxCameraForCurrentVariant())
    {
        EPRINT(CAMERA_INTERFACE, "invld camera index found: [camera=%d]",  cameraIndex);
        return FAIL;
    }

    MUTEX_LOCK(onvifCamDetail[cameraIndex].onvifAccessLock);
    if((onvifCamDetail[cameraIndex].prevRequest == ONVIF_REQUEST_MAX) || (onvifCamDetail[cameraIndex].prevRequest != request))
    {
        if(onvifCamDetail[cameraIndex].prevRequest != ONVIF_RETRY_REQUEST)
        {
            onvifCamDetail[cameraIndex].retryCount = 1;
        }
        onvifCamDetail[cameraIndex].prevRequest = request;
    }
    else if(onvifCamDetail[cameraIndex].prevRequest == request)
    {
        onvifCamDetail[cameraIndex].retryCount++;
        if(onvifCamDetail[cameraIndex].retryCount >= MAX_ONVIF_RETRY)
        {
            onvifCamDetail[cameraIndex].retryCount = 0;
            MUTEX_UNLOCK(onvifCamDetail[cameraIndex].onvifAccessLock);
            DPRINT(CAMERA_INTERFACE, "max retry done for onvif request: [camera=%d], [request=%d]",  cameraIndex, request);
            return FAIL;
        }
    }
    MUTEX_UNLOCK(onvifCamDetail[cameraIndex].onvifAccessLock);

    TIMER_INFO_t timerInfo;
    timerInfo.count   = CONVERT_SEC_TO_TIMER_COUNT(STREAM_RETRY_TIME);
    timerInfo.data	  = ((cameraIndex << 16) | request );
    timerInfo.funcPtr = onvifTimerTimeout;

    if (onvifCamDetail[cameraIndex].onvifRetryTimerHandle == INVALID_TIMER_HANDLE)
    {
        if (StartTimer(timerInfo, &onvifCamDetail[cameraIndex].onvifRetryTimerHandle) == FAIL)
        {
            EPRINT(CAMERA_INTERFACE, "fail to start onvif request retry timer: [camera=%d], [request=%d]",  cameraIndex, request);
            MUTEX_LOCK(onvifCamDetail[cameraIndex].onvifAccessLock);
            onvifCamDetail[cameraIndex].prevRequest = ONVIF_REQUEST_MAX;
            onvifCamDetail[cameraIndex].retryCount = 0;
            MUTEX_UNLOCK(onvifCamDetail[cameraIndex].onvifAccessLock);
            return FAIL;
        }
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief startOnvifStreamUrlRetryTimer : on timeout it will retry for stream onvif cmd
 * @param cameraIndex
 * @param request
 */
static void startOnvifStreamUrlRetryTimer(UINT8 cameraIndex, ONVIF_REQUEST_e request)
{
    TIMER_INFO_t    timerInfo;
    UINT32          camIdx = GET_STREAM_INDEX(cameraIndex);
    VIDEO_TYPE_e    videoType = GET_STREAM_TYPE(cameraIndex);

    if (onvifCamDetail[camIdx].onvifStrmUrlRetryTimerHandle[videoType] != INVALID_TIMER_HANDLE)
    {
        return;
    }

    timerInfo.count   = CONVERT_SEC_TO_TIMER_COUNT(STREAM_RETRY_TIME);
    timerInfo.data	  = ((cameraIndex << 16) | request );
    timerInfo.funcPtr = onvifStreamUrlTimerCB;
    if (StartTimer(timerInfo, &onvifCamDetail[camIdx].onvifStrmUrlRetryTimerHandle[videoType]) == FAIL)
    {
        EPRINT(CAMERA_INTERFACE, "fail to start stream url retry timer: [camera=%d], [request=%d]",  cameraIndex, request);
    }
    else
    {
        DPRINT(CAMERA_INTERFACE, "stream url retry timer started: [camera=%d], [request=%d]",  cameraIndex, request);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief onvifTimerTimeout : timeout function for onvif timer
 * @param data
 */
static void onvifTimerTimeout(UINT32 data)
{
    NET_CMD_STATUS_e	onvifReqStatus;
    UINT8               cameraIndex = (UINT8)(data >> 16);
    ONVIF_REQUEST_e     onvifReq = (ONVIF_REQUEST_e)((data) & 0x0000FFFF);

    if (onvifReq >= ONVIF_GET_MEDIA_URL)
    {
        EPRINT(CAMERA_INTERFACE, "invld onvif request: [camera=%d], [request=%d]", cameraIndex, onvifReq);
        return;
    }

    WPRINT(CAMERA_INTERFACE, "onvif request timeout: [camera=%d], [request=%d]", cameraIndex, onvifReq);
    DeleteTimer(&onvifCamDetail[cameraIndex].onvifRetryTimerHandle);
    onvifReqStatus = sendOnvifCommonCmd(cameraIndex, onvifReq, onvifSetupCallback[onvifReq], NULL, NULL);

    //If channel is disabled or conenctivity lost then stop timer to make another request
    if ((onvifReqStatus == CMD_CAM_DISCONNECTED) || (onvifReqStatus == CMD_CHANNEL_DISABLED))
    {
        EPRINT(CAMERA_INTERFACE, "delete onvif request retry timer: [camera=%d], [request=%d]", cameraIndex, onvifReq);
        MUTEX_LOCK(onvifCamDetail[cameraIndex].onvifAccessLock);
        onvifCamDetail[cameraIndex].retryCount = 0;
        MUTEX_UNLOCK(onvifCamDetail[cameraIndex].onvifAccessLock);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief onvifStreamUrlTimerCB : callback function to onvif get stream URL cmd
 * @param data
 */
static void onvifStreamUrlTimerCB(UINT32 data)
{
    BOOL                forceLocalConfig;
    UINT16              complexCamIndex = (UINT16)(data >> 16);
    UINT16              realCamIndex = GET_STREAM_INDEX(complexCamIndex);
    VIDEO_TYPE_e        realVideoType = GET_STREAM_TYPE(complexCamIndex);
    ONVIF_REQUEST_e     onvifReq = (ONVIF_REQUEST_e)((data) & 0x0000FFFF);
    NET_CMD_STATUS_e    onvifReqStatus = CMD_SUCCESS;
    STREAM_CONFIG_t     streamConfig;
    UINT8               streamProfileIndex = 0;

    DPRINT(CAMERA_INTERFACE, "timer triggered onvif request retry: [camera=%d], [onvifReq=%d]",  complexCamIndex, onvifReq);
    DeleteTimer(&onvifCamDetail[realCamIndex].onvifStrmUrlRetryTimerHandle[realVideoType]);

    if (OnvifSupportF(realCamIndex) == TRUE)
    {
        ReadSingleStreamConfig(realCamIndex, &streamConfig);
        getStreamProfileNum(realVideoType, realCamIndex, &streamProfileIndex);
        forceLocalConfig = isStreamConfigValid(&streamConfig, realVideoType);
        if (forceLocalConfig == FALSE)
        {
            onvifReqStatus = sendOnvifStreamCmd(complexCamIndex, ONVIF_GET_MEDIA_URL, onvifStreamUrlCb, NULL, streamProfileIndex, FALSE, &streamConfig);
        }
        else
        {
            onvifReqStatus = sendOnvifStreamCmd(complexCamIndex, ONVIF_GET_MEDIA_URL, NULL, NULL, streamProfileIndex, TRUE, &streamConfig);
            if (onvifReqStatus == CMD_SUCCESS)
            {
                onvifReqStatus = sendOnvifStreamCmd(complexCamIndex, ONVIF_GET_MEDIA_URL, onvifStreamUrlCb, NULL, streamProfileIndex, FALSE, &streamConfig);
            }
        }
    }

    /* If channel is disabled or conenctivity lost then stop timer to make another request */
    if ((onvifReqStatus == CMD_CAM_DISCONNECTED) || (onvifReqStatus == CMD_CHANNEL_DISABLED))
    {
        CI_STREAM_EXT_TRG_PARAM_t triggerParam;

        EPRINT(CAMERA_INTERFACE, "delete onvif request retry timer: [camera=%d], [onvifReq=%d]",  complexCamIndex, onvifReq);
        triggerParam.camIndex = complexCamIndex;
        triggerParam.type = CI_STREAM_EXT_TRG_CONTROL_CB;
        triggerParam.status = onvifReqStatus;

        streamInfo[realCamIndex][realVideoType].getStreamRequest.numOfRequest = 1;
        // NOTE: Fixed requestCount ==> when we ++ ==> 0
        streamInfo[realCamIndex][realVideoType].getStreamRequest.requestCount = 255;
        ciStreamExtTriggerHandler(&triggerParam);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetCamAlarmState : Get current camera alram state
 * @param cameraIndex
 * @param alarmIndex
 * @return ON/OFF
 */
BOOL GetCamAlarmState(UINT8 cameraIndex, UINT8 alarmIndex)
{
    if ((cameraIndex >= getMaxCameraForCurrentVariant()) || (alarmIndex >= MAX_CAMERA_ALARM))
    {
        return OFF;
    }

    MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].counterLock);
    if (alarmActionRequest[cameraIndex][alarmIndex].requestCounter == 0)
    {
        MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].counterLock);
        return OFF;
    }
    MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].counterLock);

    return ON;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetCamIpAddress : gets camera IP address
 * @param cameraIndex
 * @param ipAddress
 * @return ON/OFF
 */
BOOL GetCamIpAddress(UINT8 cameraIndex, CHARPTR ipAddress)
{
    MUTEX_LOCK(camConnectivity[cameraIndex].ipAddrLock);
    snprintf(ipAddress, IPV6_ADDR_LEN_MAX, "%s", camConnectivity[cameraIndex].ipAddr);
    MUTEX_UNLOCK(camConnectivity[cameraIndex].ipAddrLock);
    return GetCamEventStatus(cameraIndex, CONNECTION_FAILURE);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief resetCamIpAddress : Reset stroed camera IP
 * @param cameraIndex
 * @return
 */
static void resetCamIpAddress(UINT8 cameraIndex)
{
    MUTEX_LOCK(camConnectivity[cameraIndex].ipAddrLock);
    camConnectivity[cameraIndex].ipAddr[0] = '\0';
    MUTEX_UNLOCK(camConnectivity[cameraIndex].ipAddrLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief ValidateCameraCfg
 * @param newCfg
 * @param oldCfg
 * @param camIndex
 * @return Network command status
 */
NET_CMD_STATUS_e ValidateCameraCfg(CAMERA_CONFIG_t *newCfg, CAMERA_CONFIG_t *oldCfg)
{
    UINT8			totalIpCamera = 0;
    UINT8           camIndex;
    CAMERA_CONFIG_t	camCfg;

    if ((oldCfg->type != newCfg->type) || ((oldCfg->camera == DISABLE) && (newCfg->camera == ENABLE)))
    {
        for (camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
        {
            ReadSingleCameraConfig(camIndex, &camCfg);
            if ((camCfg.type == IP_CAMERA) && (camCfg.camera == ENABLE))
            {
                totalIpCamera++;
            }
        }

        if (totalIpCamera >= getMaxCameraForCurrentVariant())
        {
            return CMD_MAX_CAMERA_CONFIGURED;
        }
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief CiIpCameraConfigNotify : will be called when thers is change in IP camera congif Table
 * @param newIpCfg
 * @param oldIpCfg
 * @param cameraIndex
 */
void CiIpCameraConfigNotify(IP_CAMERA_CONFIG_t newIpCfg, IP_CAMERA_CONFIG_t *oldIpCfg, UINT8 cameraIndex)
{
    UINT8               cnt;
    UINT8               stremAction = START;
    STREAM_CONFIG_t     streamCnfg;
    BOOL                passChangeF = FALSE;
    UINT8               streamProfileIndex ;

    if ((strcmp(oldIpCfg->cameraAddress, newIpCfg.cameraAddress) != STATUS_OK)
            || (oldIpCfg->httpPort != newIpCfg.httpPort) || (oldIpCfg->rtspPort != newIpCfg.rtspPort) || (oldIpCfg->onvifPort != newIpCfg.onvifPort))
    {
        resetCamIpAddress(cameraIndex);
        stremAction = RESTART;
    }
    else if ((strcmp(oldIpCfg->username, newIpCfg.username) != STATUS_OK) || (strcmp(oldIpCfg->password, newIpCfg.password) != STATUS_OK))
    {
        if ((newIpCfg.onvifSupportF == oldIpCfg->onvifSupportF) && (oldIpCfg->onvifSupportF == TRUE))
        {
            if ((oldIpCfg->brand[0] == '\0') || (oldIpCfg->model[0] == '\0')
                    || (onvifCamDetail[cameraIndex].onvifCamBrand[0] == '\0') || (onvifCamDetail[cameraIndex].onvifCamModel[0] == '\0'))
            {
                MUTEX_LOCK(onvifCamDetail[cameraIndex].onvifAccessLock);
                if(onvifCamDetail[cameraIndex].entryPointReceived == NO)
                {
                    MUTEX_UNLOCK(onvifCamDetail[cameraIndex].onvifAccessLock);
                    if (sendOnvifCommonCmd(cameraIndex, ONVIF_GET_DATE_TIME, onvifDateTimeCb, &newIpCfg, NULL) != CMD_SUCCESS)
                    {
                        startOnvifRetryTimer(cameraIndex, ONVIF_GET_DATE_TIME);
                    }
                }
                else
                {
                    MUTEX_UNLOCK(onvifCamDetail[cameraIndex].onvifAccessLock);
                    if (sendOnvifCommonCmd(cameraIndex, ONVIF_GET_BRAND_MODEL, onvifBrandModelCb, &newIpCfg, NULL) != CMD_SUCCESS)
                    {
                        startOnvifRetryTimer(cameraIndex, ONVIF_GET_BRAND_MODEL);
                    }
                }
            }
            else
            {
                stremAction = RESTART;
            }
        }
        else
        {
            if(newIpCfg.onvifSupportF == FALSE)
            {
                passChangeF = TRUE;
                stremAction = RESTART;
            }
        }
    }
    else if(oldIpCfg->rtspProtocol != newIpCfg.rtspProtocol)
    {
        if ((newIpCfg.onvifSupportF == oldIpCfg->onvifSupportF) && (oldIpCfg->onvifSupportF == TRUE))
        {
            stremAction = RESTART;
        }
    }

    /* Need to check only camera is not ONVIF */
    if (newIpCfg.onvifSupportF == FALSE)
    {
        CAMERA_BRAND_e brand;
        CAMERA_MODEL_e model;
        GetCameraBrandModelNumber(cameraIndex, &brand, &model);
        if ((strcmp(oldIpCfg->url, newIpCfg.url) != STATUS_OK) && (brand == CAMERA_BRAND_GENERIC))
        {
            resetCamIpAddress(cameraIndex);
            stremAction = RESTART;
        }
    }

    if ((strcmp(oldIpCfg->brand, newIpCfg.brand) != STATUS_OK) || (strcmp(oldIpCfg->model, newIpCfg.model) != STATUS_OK)
            || (oldIpCfg->onvifSupportF != newIpCfg.onvifSupportF))
    {
        pthread_rwlock_wrlock(&camConfigInfo[cameraIndex].configInfoLock);
        camConfigInfo[cameraIndex].configChangeNotify = FALSE;
        pthread_rwlock_unlock(&camConfigInfo[cameraIndex].configInfoLock);

        DPRINT(CAMERA_INTERFACE, "brand/model/onvif flag changed: [camera=%d]", cameraIndex);
        stremAction = STOP;
        passChangeF = FALSE;

        if(newIpCfg.onvifSupportF != oldIpCfg->onvifSupportF)
        {
            // Disconnect camera
            resetCamIpAddress(cameraIndex);

            MUTEX_LOCK(onvifCamDetail[cameraIndex].onvifAccessLock);
            onvifCamDetail[cameraIndex].entryPointReceived = NO;
            MUTEX_UNLOCK(onvifCamDetail[cameraIndex].onvifAccessLock);

            memset(onvifCamDetail[cameraIndex].onvifCamBrand, '\0', MAX_BRAND_NAME_LEN);
            memset(onvifCamDetail[cameraIndex].onvifCamModel, '\0', MAX_MODEL_NAME_LEN);

            // Clear image upload url so that in next image upload gets new url from camera
            memset(getImageRequest[cameraIndex].url, '\0', sizeof(getImageRequest[cameraIndex].url));

            if(newIpCfg.onvifSupportF == FALSE)
            {
                // Delete timer & database entries related to this camera
                FreeCameraCapability(cameraIndex);
                DeleteTimer(&onvifCamDetail[cameraIndex].onvifRetryTimerHandle);

                pthread_rwlock_wrlock(&camConfigInfo[cameraIndex].configInfoLock);
                camConfigInfo[cameraIndex].onvifSupportF = FALSE;
                pthread_rwlock_unlock(&camConfigInfo[cameraIndex].configInfoLock);
            }
            else
            {
                pthread_rwlock_wrlock(&camConfigInfo[cameraIndex].configInfoLock);
                camConfigInfo[cameraIndex].brand = (CAMERA_BRAND_e)ONVIF_CAMERA;
                camConfigInfo[cameraIndex].model = (CAMERA_MODEL_e)ONVIF_CAMERA;
                camConfigInfo[cameraIndex].onvifSupportF = TRUE;
                pthread_rwlock_unlock(&camConfigInfo[cameraIndex].configInfoLock);
            }

            // Default all the related parameters
            DfltSingleStreamConfig(cameraIndex);
            DfltSingleImageUploadConfig(cameraIndex);
            DfltSinglePresetTourConfig(cameraIndex);
            DfltSingleTourScheduleConfig(cameraIndex);

            /* Default image setting param and config */
            DefaultCameraImageCapability(cameraIndex);

            for(cnt = 0; cnt < MAX_PRESET_POSITION; cnt++)
            {
                DfltSinglePtzPresetConfig(cameraIndex, cnt);
            }

            for(cnt = 0; cnt < MAX_CAMERA_ALARM; cnt++)
            {
                DfltSingleCameraAlarmConfig(cameraIndex, cnt);
            }

            for(cnt = 0; cnt < MAX_CAMERA_EVENT; cnt++)
            {
                DfltSingleCameraEventConfig(cameraIndex, cnt);
            }
        }
        else
        {
            if ((newIpCfg.onvifSupportF == TRUE) && ((oldIpCfg->brand[0] == '\0') || (oldIpCfg->model[0] == '\0')))
            {
                DPRINT(CAMERA_INTERFACE, "brand-model recv for onvif: [camera=%d]", cameraIndex);
            }
            else
            {
                resetCamIpAddress(cameraIndex);
                // Default all the related parameters
                DfltSingleStreamConfig(cameraIndex);
                DfltSingleImageUploadConfig(cameraIndex);

                /* Default image setting param and config */
                DefaultCameraImageCapability(cameraIndex);

                for(cnt = 0; cnt < MAX_PRESET_POSITION; cnt++)
                {
                    DfltSinglePtzPresetConfig(cameraIndex, cnt);
                }

                for(cnt = 0; cnt < MAX_CAMERA_ALARM; cnt++)
                {
                    DfltSingleCameraAlarmConfig(cameraIndex, cnt);
                }

                for(cnt = 0; cnt < MAX_CAMERA_EVENT; cnt++)
                {
                    DfltSingleCameraEventConfig(cameraIndex, cnt);
                }
            }
        }	// ONVIF support flag is not changed

        pthread_rwlock_wrlock(&camConfigInfo[cameraIndex].configInfoLock);
        camConfigInfo[cameraIndex].configChangeNotify = TRUE;
        pthread_rwlock_unlock(&camConfigInfo[cameraIndex].configInfoLock);

        if (newIpCfg.onvifSupportF == FALSE)
        {
            pthread_rwlock_wrlock(&camConfigInfo[cameraIndex].configInfoLock);
            if(GetBrandNum(newIpCfg.brand, &camConfigInfo[cameraIndex].brand) == FAIL)
            {
                camConfigInfo[cameraIndex].brand = CAMERA_BRAND_NONE;
            }

            GetModelNum(newIpCfg.brand, newIpCfg.model, &camConfigInfo[cameraIndex].model);
            camConfigInfo[cameraIndex].onvifSupportF = FALSE;
            pthread_rwlock_unlock(&camConfigInfo[cameraIndex].configInfoLock);
        }
    }	// IF end - brand/model/ONVIF flag changed

    if(passChangeF == TRUE)
    {
        ReadSingleStreamConfig(cameraIndex, &streamCnfg);

        if (isStreamConfigValid(&streamCnfg, MAIN_STREAM) == FALSE)
        {
            getStreamProfileNum(MAIN_STREAM, cameraIndex, &streamProfileIndex);
            GetProfileParam(cameraIndex, streamProfileIndex, MAIN_STREAM, NULL, 0, CLIENT_CB_TYPE_NATIVE);
        }

        if (isStreamConfigValid(&streamCnfg, SUB_STREAM) == FALSE)
        {
            getStreamProfileNum(SUB_STREAM, cameraIndex, &streamProfileIndex);
            GetProfileParam(cameraIndex, streamProfileIndex, SUB_STREAM, NULL, 0, CLIENT_CB_TYPE_NATIVE);
        }
    }

    if ((stremAction != START) && (CameraType(cameraIndex) == IP_CAMERA))
    {
        CI_STREAM_EXT_TRG_PARAM_t triggerParam;

        triggerParam.clientType = MAX_CI_STREAM_CLIENT;
        if (stremAction == STOP)
        {
            triggerParam.type = CI_STREAM_EXT_TRG_STOP_STREAM;
        }
        else
        {
            triggerParam.type = CI_STREAM_EXT_TRG_CONFIG_CHANGE;
        }

        /* terminate two way audio if running for this camera */
        TerminateClientMediaStreamer(cameraIndex, INVALID_SESSION_INDEX);

        // Give Trigger to both FSM
        triggerParam.camIndex = cameraIndex;
        ciStreamExtTriggerHandler(&triggerParam);

        triggerParam.camIndex = (cameraIndex + getMaxCameraForCurrentVariant());
        ciStreamExtTriggerHandler(&triggerParam);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief CiCameraConfigNotify : will be called when there is change in any camera config table
 * @param newCameraConfig
 * @param oldCameraConfig
 * @param cameraIndex
 */
void CiCameraConfigNotify(CAMERA_CONFIG_t newCameraConfig, CAMERA_CONFIG_t * oldCameraConfig, UINT8 cameraIndex)
{
    UINT8					cnt;
    BOOL					stremAction = START;
    PRIVACY_MASK_CONFIG_t	privacyArea[MAX_PRIVACY_MASKS];
    OSD_PARAM_t             osdParam;

    /* Load default value in OSD params */
    memset(&osdParam, 0, sizeof(osdParam));
    osdParam.dateTimePos = MAX_OSD_POS;
    for (cnt = 0; cnt < TEXT_OVERLAY_MAX; cnt++)
    {
        osdParam.channelNamePos[cnt] = MAX_OSD_POS;
    }

    DPRINT(CAMERA_INTERFACE, "camera config notify: [camera=%d]", cameraIndex);
    pthread_rwlock_rdlock(&camCnfgNotifyControl[cameraIndex].configNotifyLock);
    if (FALSE == camCnfgNotifyControl[cameraIndex].cameraConfigNotifyF)
    {
        pthread_rwlock_unlock(&camCnfgNotifyControl[cameraIndex].configNotifyLock);
        return;
    }
    pthread_rwlock_unlock(&camCnfgNotifyControl[cameraIndex].configNotifyLock);

    if ((oldCameraConfig->camera != newCameraConfig.camera) && (newCameraConfig.camera == DISABLE))
    {
        setOsdsRequest[cameraIndex].isOsdSetPending = FALSE;
        stremAction = STOP;

        // Stop polling for events
        if(GetEventPollStatus(cameraIndex, MAX_CAMERA_EVENT) == ON)
        {
            for(cnt = MOTION_DETECT; cnt < MAX_CAMERA_EVENT; cnt++)
            {
                if(GetEventPollStatus(cameraIndex, cnt) == ON)
                {
                    StopCameraEventPoll(cameraIndex, cnt);
                }
            }
        }

        /* Default the stream config */
        DfltSingleStreamConfig(cameraIndex);

        /* Default image setting param and config */
        DefaultCameraImageCapability(cameraIndex);
    }

    if(newCameraConfig.camera == ENABLE)
    {
        if (newCameraConfig.type == IP_CAMERA || newCameraConfig.type ==  AUTO_ADDED_CAMERA)
        {
            DPRINT(CAMERA_INTERFACE, "ip camera enabled: [camera=%d]", cameraIndex);

            //checking for OSD config settings
            if(newCameraConfig.dateTimeOverlay != oldCameraConfig->dateTimeOverlay)
            {
                DPRINT(CAMERA_INTERFACE, "date-time overlay changed: [camera=%d], [overlay=%d]", cameraIndex, newCameraConfig.dateTimeOverlay);
                osdParam.timeOverlayChanged = TRUE;
                if(newCameraConfig.dateTimeOverlay == FALSE)
                {
                    osdParam.currentOsdStatus = TRUE;
                    osdParam.dateTimeOverlay = FALSE;
                }

                if(newCameraConfig.dateTimeOverlay == TRUE)
                {
                    osdParam.currentOsdStatus = TRUE;
                    osdParam.dateTimeOverlay = TRUE;
                    osdParam.dateTimePos = newCameraConfig.dateTimePos;
                }
            }
            else
            {
                if(newCameraConfig.dateTimeOverlay == TRUE)
                {
                    DPRINT(CAMERA_INTERFACE, "date-time overlay not changed: [camera=%d], [overlay=%d]", cameraIndex, newCameraConfig.dateTimeOverlay);
                    osdParam.timeOverlayChanged = TRUE;
                    osdParam.currentOsdStatus = TRUE;
                    osdParam.dateTimeOverlay = TRUE;
                    osdParam.dateTimePos = newCameraConfig.dateTimePos;
                }
            }

            if(newCameraConfig.channelNameOverlay != oldCameraConfig->channelNameOverlay)
            {
                DPRINT(CAMERA_INTERFACE, "channel name overlay changed: [camera=%d], [overlay=%d]", cameraIndex, newCameraConfig.channelNameOverlay);
                osdParam.textoverlayChanged = TRUE;
                if(newCameraConfig.channelNameOverlay == FALSE)
                {
                    osdParam.currentOsdStatus = TRUE;
                    osdParam.channelNameOverlay = FALSE;
                }

                if(newCameraConfig.channelNameOverlay == TRUE)
                {
                    osdParam.currentOsdStatus = TRUE;
                    osdParam.channelNameOverlay = TRUE;
                    for(cnt = 0; cnt < TEXT_OVERLAY_MAX; cnt++)
                    {
                        osdParam.channelNamePos[cnt] = newCameraConfig.channelNamePos[cnt];
                        snprintf(osdParam.channelName[cnt], MAX_CHANNEL_NAME_WIDTH, "%s", newCameraConfig.channelName[cnt]);
                    }
                }
            }
            else
            {
                if(newCameraConfig.channelNameOverlay == TRUE)
                {
                    DPRINT(CAMERA_INTERFACE, "channel name overlay not changed: [camera=%d], [overlay=%d]", cameraIndex, newCameraConfig.channelNameOverlay);
                    osdParam.textoverlayChanged = TRUE;
                    osdParam.currentOsdStatus = TRUE;
                    osdParam.channelNameOverlay = TRUE;
                    for(cnt = 0; cnt < TEXT_OVERLAY_MAX; cnt++)
                    {
                        osdParam.channelNamePos[cnt] = newCameraConfig.channelNamePos[cnt];
                        snprintf(osdParam.channelName[cnt], MAX_CHANNEL_NAME_WIDTH, "%s", newCameraConfig.channelName[cnt]);
                    }
                }
            }

            if(osdParam.currentOsdStatus == TRUE)
            {
                /* There are changes in OSD settings */
                setOsdsRequest[cameraIndex].osdParam = osdParam;
                if (DISABLE == oldCameraConfig->camera)
                {
                    /* Camera config changed from disabled to enabled. So that. We can't set the OSD right now
                     * becuase we don't have sufficient camera details to proceed further */
                    setOsdsRequest[cameraIndex].isOsdSetPending = TRUE;
                }
                else
                {
                    /* Runtime config changed */
                    if (setOsds(cameraIndex, &setOsdsRequest[cameraIndex].osdParam) == CMD_SUCCESS)
                    {
                        setOsdsRequest[cameraIndex].isOsdSetPending = FALSE;
                        DPRINT(CAMERA_INTERFACE, "set osd request sent successfully: [camera=%d]", cameraIndex);
                    }
                    else
                    {
                        EPRINT(CAMERA_INTERFACE, "fail to send set osd request: [camera=%d]", cameraIndex);
                    }
                }
            }

            if(oldCameraConfig->privacyMaskStaus != newCameraConfig.privacyMaskStaus)
            {
                if(newCameraConfig.privacyMaskStaus == FALSE)
                {
                    for(cnt = 0; cnt < MAX_PRIVACY_MASKS; cnt++)
                    {
                        privacyArea[cnt].startXPoint = 0;
                        privacyArea[cnt].startYPoint = 0;
                        privacyArea[cnt].width = 2;
                        privacyArea[cnt].height = 2;
                    }

                    SetPrivacyMaskConfig(cameraIndex, privacyArea, NULL, 0, DISABLE, CLIENT_CB_TYPE_NATIVE);
                }
            }

            /* If Motion Detection Status DISABLED, clearing the Motion detection window area */
            if(oldCameraConfig->motionDetectionStatus != newCameraConfig.motionDetectionStatus)
            {
                if(newCameraConfig.motionDetectionStatus == DISABLE)
                {
                    resetMotionWindowConfig(cameraIndex);
                }
            }
        }
    }

    // Save Camera Type
    if(newCameraConfig.type != oldCameraConfig->type)
    {
        /* Default event configuration before updating the camera type and defaulting other configuration.
         * It was creating problem in case of CI. Active event not becoming inactive after camera default */
        for(cnt = 0; cnt < MAX_CAMERA_EVENT; cnt++)
        {
            DfltSingleCameraEventConfig(cameraIndex, cnt);
        }

        pthread_rwlock_wrlock(&camConfigInfo[cameraIndex].configInfoLock);
        camConfigInfo[cameraIndex].cameraType = newCameraConfig.type;
        camConfigInfo[cameraIndex].configChangeNotify = FALSE;
        pthread_rwlock_unlock(&camConfigInfo[cameraIndex].configInfoLock);

        stremAction = STOP;

        // Default all the related parameters
        DfltSingleIpCameraConfig(cameraIndex);
        DfltSingleStreamConfig(cameraIndex);
        DfltSingleImageUploadConfig(cameraIndex);
        DfltSinglePresetTourConfig(cameraIndex);
        DfltSingleTourScheduleConfig(cameraIndex);

        /* Default image setting param and config */
        DefaultCameraImageCapability(cameraIndex);

        for(cnt = 0; cnt < MAX_PRESET_POSITION; cnt++)
        {
            DfltSinglePtzPresetConfig(cameraIndex, cnt);
        }

        for(cnt = 0; cnt < MAX_CAMERA_ALARM; cnt++)
        {
            DfltSingleCameraAlarmConfig(cameraIndex, cnt);
        }
    }

    if (stremAction == STOP)
    {
        CI_STREAM_EXT_TRG_PARAM_t triggerParam;

        triggerParam.clientType = MAX_CI_STREAM_CLIENT;
        triggerParam.type = CI_STREAM_EXT_TRG_STOP_STREAM;
        triggerParam.mediaResp = CI_STREAM_INT_RESP_CLOSE;

        // Give Trigger to both FSM
        triggerParam.camIndex = cameraIndex;
        ciStreamExtTriggerHandler(&triggerParam);

        triggerParam.camIndex = (cameraIndex + getMaxCameraForCurrentVariant());
        ciStreamExtTriggerHandler(&triggerParam);

        pthread_rwlock_wrlock(&camConfigInfo[cameraIndex].configInfoLock);
        camConfigInfo[cameraIndex].configChangeNotify = TRUE;
        pthread_rwlock_unlock(&camConfigInfo[cameraIndex].configInfoLock);

        /* Disable Two way audio if enabled when camera config changed to disabled */
        TerminateClientMediaStreamer(cameraIndex, INVALID_SESSION_INDEX);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief CiGeneralCfgUpdate : will be called when there is change in general camera config table
 * @param generalConfig
 * @param oldgeneralConfig
 */
void CiGeneralCfgUpdate(GENERAL_CONFIG_t generalConfig, GENERAL_CONFIG_t *oldgeneralConfig)
{
    UINT8           cameraIndex;
    CAMERA_CONFIG_t cameraCnfg;

    if (generalConfig.videoSystemType != oldgeneralConfig->videoSystemType)
    {
        DfltStreamConfig();
    }

    if ((generalConfig.timeFormat != oldgeneralConfig->timeFormat) || (generalConfig.dateFormat != oldgeneralConfig->dateFormat))
    {
        for(cameraIndex = 0; cameraIndex < getMaxCameraForCurrentVariant(); cameraIndex++)
        {
            ReadSingleCameraConfig(cameraIndex, &cameraCnfg);
            CiCameraConfigNotify(cameraCnfg, &cameraCnfg, cameraIndex);
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief CiStreamConfigNotify :will be called when there is change in camera stream config table
 * @param newStreamConfig
 * @param oldStreamConfig
 * @param cameraIndex
 */
void CiStreamConfigNotify(STREAM_CONFIG_t *newStreamConfig, STREAM_CONFIG_t *oldStreamConfig, UINT8 cameraIndex)
{
    CI_STREAM_EXT_TRG_PARAM_t	triggerParam;
    BOOL						giveTrigger[MAX_STREAM] = {FALSE, FALSE};
    CAMERA_CONFIG_t				cameraConfig;
    UINT8 						dfltFrameRate = 0;
    UINT8						dfltQuality = 0;

    ReadSingleCameraConfig(cameraIndex, &cameraConfig);

    pthread_rwlock_rdlock(&camConfigInfo[cameraIndex].configInfoLock);
    if (((cameraConfig.type == IP_CAMERA) || (cameraConfig.type == AUTO_ADDED_CAMERA)) && (camConfigInfo[cameraIndex].configChangeNotify == TRUE))
    {
        if((strcmp(oldStreamConfig->videoEncoding,"") == STATUS_OK) && (strcmp(oldStreamConfig->resolution,"") == STATUS_OK) &&
                (oldStreamConfig->framerate == dfltFrameRate) && (oldStreamConfig->bitrateValue == BITRATE_2048) &&
                (oldStreamConfig->bitrateMode == VBR) && (oldStreamConfig->gop == DFLT_CAM_GOP) &&
                (oldStreamConfig->enableAudio == DISABLE) && (oldStreamConfig->quality == dfltQuality) &&
                (oldStreamConfig->mainStreamProfile == DFLT_MAIN_STREAM_PROFILE))
        {
            giveTrigger[MAIN_STREAM] = TRUE;
        }

        if((strcmp(oldStreamConfig->videoEncodingSub,"") == STATUS_OK) && (strcmp(oldStreamConfig->resolutionSub,"") == STATUS_OK) &&
                (oldStreamConfig->framerateSub == dfltFrameRate) && (oldStreamConfig->bitrateValueSub == BITRATE_1024) &&
                (oldStreamConfig->bitrateModeSub == VBR) && (oldStreamConfig->gopSub == DFLT_CAM_GOP) &&
                (oldStreamConfig->enableAudioSub == DISABLE) && (oldStreamConfig->qualitySub == dfltQuality) &&
                (oldStreamConfig->subStreamProfile == DFLT_SUB_STREAM_PROFILE))
        {
            giveTrigger[SUB_STREAM] = TRUE;
        }
    }

    // NOTE: If Stream parameters are not valid stop stream
    // Change this when we support stream with invalid configuration
    if ((strcmp(oldStreamConfig->videoEncoding, newStreamConfig->videoEncoding) != STATUS_OK)
         || (strcmp(oldStreamConfig->resolution, newStreamConfig->resolution) != STATUS_OK) || (oldStreamConfig->framerate != newStreamConfig->framerate)
         || (oldStreamConfig->bitrateMode != newStreamConfig->bitrateMode) || (oldStreamConfig->bitrateValue != newStreamConfig->bitrateValue)
         || (oldStreamConfig->quality != newStreamConfig->quality) || (oldStreamConfig->enableAudio != newStreamConfig->enableAudio)
         || (oldStreamConfig->gop != newStreamConfig->gop) || (oldStreamConfig->mainStreamProfile != newStreamConfig->mainStreamProfile))
    {
        GetVideoCodecNum(newStreamConfig->videoEncoding, &camConfigInfo[cameraIndex].videoCodec[MAIN_STREAM]);
        GetResolutionNum(newStreamConfig->resolution, &camConfigInfo[cameraIndex].resolution[MAIN_STREAM]);
        if (camConfigInfo[cameraIndex].configChangeNotify == TRUE)
        {
            giveTrigger[MAIN_STREAM] = TRUE;
        }
    }

    if ((strcmp(oldStreamConfig->videoEncodingSub, newStreamConfig->videoEncodingSub) != STATUS_OK)
         || (strcmp(oldStreamConfig->resolutionSub, newStreamConfig->resolutionSub) != STATUS_OK) || (oldStreamConfig->framerateSub != newStreamConfig->framerateSub)
         || (oldStreamConfig->bitrateModeSub != newStreamConfig->bitrateModeSub) || (oldStreamConfig->bitrateValueSub != newStreamConfig->bitrateValueSub)
         || (oldStreamConfig->qualitySub != newStreamConfig->qualitySub) || (oldStreamConfig->enableAudioSub != newStreamConfig->enableAudioSub)
         || (oldStreamConfig->gopSub != newStreamConfig->gopSub) || (oldStreamConfig->subStreamProfile != newStreamConfig->subStreamProfile))
    {
        GetVideoCodecNum(newStreamConfig->videoEncodingSub, &camConfigInfo[cameraIndex].videoCodec[SUB_STREAM]);
        GetResolutionNum(newStreamConfig->resolutionSub, &camConfigInfo[cameraIndex].resolution[SUB_STREAM]);
        if (camConfigInfo[cameraIndex].configChangeNotify == TRUE)
        {
            giveTrigger[SUB_STREAM] = TRUE;
        }
    }

    pthread_rwlock_unlock(&camConfigInfo[cameraIndex].configInfoLock);

    triggerParam.type = CI_STREAM_EXT_TRG_CONFIG_CHANGE;
    triggerParam.clientType = MAX_CI_STREAM_CLIENT;

    if (giveTrigger[MAIN_STREAM] == TRUE)
    {
        triggerParam.camIndex = cameraIndex;
        ciStreamExtTriggerHandler(&triggerParam);
    }

    if (giveTrigger[SUB_STREAM] == TRUE)
    {
        triggerParam.camIndex = (cameraIndex + getMaxCameraForCurrentVariant());
        ciStreamExtTriggerHandler(&triggerParam);
    }

    //Update fps information for main and sub stream
    if(streamInfo[cameraIndex][MAIN_STREAM].configFrameRate !=  newStreamConfig->framerate)
    {
        streamInfo[cameraIndex][MAIN_STREAM].configFrameRate = newStreamConfig->framerate;
    }

    if(streamInfo[cameraIndex][SUB_STREAM].configFrameRate !=  newStreamConfig->framerateSub)
    {
        streamInfo[cameraIndex][SUB_STREAM].configFrameRate = newStreamConfig->framerateSub;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Process stream paramaters copy to camera
 * @param   srcCamIndex
 * @param   pSrcStreamCfg
 * @param   pCopyToCamMask
 * @return  Status
 */
NET_CMD_STATUS_e StreamParamCopyToCamera(UINT8 srcCamIndex, STREAM_CONFIG_t *pSrcStreamCfg, CAMERA_BIT_MASK_t *pCopyToCamMask)
{
    BOOL                isNewStreamParamApply;
    UINT8               streamType;
    UINT8               loopCnt;
    UINT8               camIndex;
    UINT8               dstCameraProfile;
    CHAR                details[MAX_EVENT_DETAIL_SIZE];
    CHAR                advncDetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];
    CAMERA_BRAND_e      cameraBrand;
    CAMERA_CONFIG_t     cameraConfig;
    IP_CAMERA_CONFIG_t  ipCameraConfig;
    STREAM_CONFIG_t     dstStreamConfig;
    UINT8               maxSupportedParam;
    CHARPTR             cameraCodec[MAX_VIDEO_CODEC];
    CHARPTR             cameraResolution[MAX_RESOLUTION];
    CHARPTR             cameraBitrate[MAX_BITRATE_VALUE];
    UINT64              cameraFps;
    BITRATE_VALUE_e     minBitrateIndex, maxBitrateIndex;
    CHAR                *pSrcCodec[MAX_STREAM];
    CHAR                *pSrcResolution[MAX_STREAM];
    UINT8               srcFramerate[MAX_STREAM];
    BITRATE_MODE_e      srcBitrateMode[MAX_STREAM];
    BITRATE_VALUE_e     srcBitrateValue[MAX_STREAM];
    UINT8               srcQuality[MAX_STREAM];
    UINT8               srcGop[MAX_STREAM];
    BOOL                srcAudioF[MAX_STREAM];
    const CHAR          *pStreamType[MAX_STREAM] = {"Main Stream", "Sub Stream"};

    /* Validate camera index */
    if (srcCamIndex >= getMaxCameraForCurrentVariant())
    {
        EPRINT(CAMERA_INTERFACE, "invld camera index found: [camera=%d]", srcCamIndex);
        return CMD_PROCESS_ERROR;
    }

    /* Read camera config and check camera is enabled or not */
    ReadSingleCameraConfig(srcCamIndex, &cameraConfig);
    if (cameraConfig.camera == DISABLE)
    {
        EPRINT(CAMERA_INTERFACE, "copy to camera stream not allowed due to camera is disabled: [camera=%d]", srcCamIndex);
        return CMD_CHANNEL_DISABLED;
    }

    /* Camera brand should not be generic */
    ReadSingleIpCameraConfig(srcCamIndex, &ipCameraConfig);
    if ((ipCameraConfig.onvifSupportF == DISABLE) && (cameraConfig.type == IP_CAMERA))
    {
        /* Get camera brand */
        if (FAIL == GetBrandNum(ipCameraConfig.brand, &cameraBrand))
        {
            EPRINT(CAMERA_INTERFACE, "copy to camera stream not allowed due to invld brand: [camera=%d]", srcCamIndex);
            return CMD_PROCESS_ERROR;
        }

        /* Camera brand is generic */
        if (cameraBrand == CAMERA_BRAND_GENERIC)
        {
            EPRINT(CAMERA_INTERFACE, "copy to camera stream not allowed due to generic camera: [camera=%d]", srcCamIndex);
            return CMD_GENERIC_CAM_STREAM_CONFIG;
        }
    }

    /* Take local copy of stream parameters */
    pSrcCodec[MAIN_STREAM] = pSrcStreamCfg->videoEncoding;
    pSrcResolution[MAIN_STREAM] = pSrcStreamCfg->resolution;
    srcFramerate[MAIN_STREAM] = pSrcStreamCfg->framerate;
    srcBitrateMode[MAIN_STREAM] = pSrcStreamCfg->bitrateMode;
    srcBitrateValue[MAIN_STREAM] = pSrcStreamCfg->bitrateValue;
    srcQuality[MAIN_STREAM] = pSrcStreamCfg->quality;
    srcGop[MAIN_STREAM] = pSrcStreamCfg->gop;
    srcAudioF[MAIN_STREAM] = pSrcStreamCfg->enableAudio;
    pSrcCodec[SUB_STREAM] = pSrcStreamCfg->videoEncodingSub;
    pSrcResolution[SUB_STREAM] = pSrcStreamCfg->resolutionSub;
    srcFramerate[SUB_STREAM] = pSrcStreamCfg->framerateSub;
    srcBitrateMode[SUB_STREAM] = pSrcStreamCfg->bitrateModeSub;
    srcBitrateValue[SUB_STREAM] = pSrcStreamCfg->bitrateValueSub;
    srcQuality[SUB_STREAM] = pSrcStreamCfg->qualitySub;
    srcGop[SUB_STREAM] = pSrcStreamCfg->gopSub;
    srcAudioF[SUB_STREAM] = pSrcStreamCfg->enableAudioSub;

    for (camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
    {
        /* Skip source camera from copy to camera :) */
        if (camIndex == srcCamIndex)
        {
            continue;
        }

        /* Check stream param copy needed for current camera */
        if ((GET_CAMERA_MASK_BIT(pCopyToCamMask[MAIN_STREAM], camIndex) == 0) && (GET_CAMERA_MASK_BIT(pCopyToCamMask[SUB_STREAM], camIndex) == 0))
        {
            continue;
        }

        /* Check camera config. It should be enabled to proceed further */
        ReadSingleCameraConfig(camIndex, &cameraConfig);
        if (DISABLE == cameraConfig.camera)
        {
            EPRINT(CAMERA_INTERFACE, "camera disabled: [camera=%d]", camIndex);
            continue;
        }

        ReadSingleIpCameraConfig(camIndex, &ipCameraConfig);
        snprintf(details, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(camIndex));
        if ((ipCameraConfig.onvifSupportF == DISABLE) && (cameraConfig.type == IP_CAMERA))
        {
            cameraBrand = MAX_CAMERA_BRAND;
            if ((FAIL == GetBrandNum(ipCameraConfig.brand, &cameraBrand)) || (cameraBrand == CAMERA_BRAND_GENERIC))
            {
                EPRINT(CAMERA_INTERFACE, "fail to get camera brand or camera brand is generic: [camera=%d], [brand=%d]", camIndex, cameraBrand);
                WriteEvent(LOG_CAMERA_EVENT, LOG_STREAM_PARAM_COPY_TO_CAM, details, "Generic Camera", EVENT_FAIL);
                continue;
            }
        }

        /* Check camera connectivity. Just for intimation */
        if (GetCamEventStatus(camIndex, CONNECTION_FAILURE) == INACTIVE)
        {
            WPRINT(CAMERA_INTERFACE, "camera offline: [camera=%d]", camIndex);
        }

        isNewStreamParamApply = FALSE;
        ReadSingleStreamConfig(camIndex, &dstStreamConfig);
        for (streamType = 0; streamType < MAX_STREAM; streamType++)
        {
            /* Check both streams of camera */
            if (GET_CAMERA_MASK_BIT(pCopyToCamMask[streamType], camIndex) == 0)
            {
                continue;
            }

            /* Compare source stream param with destination stream param */
            if (streamType == MAIN_STREAM)
            {
                if ((strcmp(pSrcCodec[streamType], dstStreamConfig.videoEncoding) == 0) && (strcmp(pSrcResolution[streamType], dstStreamConfig.resolution) == 0)
                        && (srcFramerate[streamType] == dstStreamConfig.framerate) && (srcBitrateMode[streamType] == dstStreamConfig.bitrateMode)
                        && (srcBitrateValue[streamType] == dstStreamConfig.bitrateValue) && (srcQuality[streamType] == dstStreamConfig.quality)
                        && (srcGop[streamType] == dstStreamConfig.gop) && (srcAudioF[streamType] == dstStreamConfig.enableAudio))
                {
                    /* All main stream params are same */
                    DPRINT(CAMERA_INTERFACE, "src and dst main stream params are same: [camera=%d], [dstCamera=%d]", srcCamIndex, camIndex);
                    continue;
                }
                dstCameraProfile = dstStreamConfig.mainStreamProfile;
            }
            else
            {
                if ((strcmp(pSrcCodec[streamType], dstStreamConfig.videoEncodingSub) == 0) && (strcmp(pSrcResolution[streamType], dstStreamConfig.resolutionSub) == 0)
                        && (srcFramerate[streamType] == dstStreamConfig.framerateSub) && (srcBitrateMode[streamType] == dstStreamConfig.bitrateModeSub)
                        && (srcBitrateValue[streamType] == dstStreamConfig.bitrateValueSub) && (srcQuality[streamType] == dstStreamConfig.qualitySub)
                        && (srcGop[streamType] == dstStreamConfig.gopSub) && (srcAudioF[streamType] == dstStreamConfig.enableAudioSub))
                {
                    /* All sub stream params are same */
                    DPRINT(CAMERA_INTERFACE, "src and dst sub stream params are same: [camera=%d], [dstCamera=%d]", srcCamIndex, camIndex);
                    continue;
                }
                dstCameraProfile = dstStreamConfig.subStreamProfile;
            }

            maxSupportedParam = 0;
            if (CMD_SUCCESS != GetSupportedCodec(camIndex, streamType, dstCameraProfile, cameraCodec, &maxSupportedParam))
            {
                EPRINT(CAMERA_INTERFACE, "%s: fail to get supported codec list: [camera=%d]", pStreamType[streamType], camIndex);
                snprintf(advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s: Codec mismatch", pStreamType[streamType]);
                WriteEvent(LOG_CAMERA_EVENT, LOG_STREAM_PARAM_COPY_TO_CAM, details, advncDetail, EVENT_FAIL);
                continue;
            }

            for (loopCnt = 0; loopCnt < maxSupportedParam; loopCnt++)
            {
                if (strcmp(pSrcCodec[streamType], cameraCodec[loopCnt]) == 0)
                {
                    /* Codec support found */
                    break;
                }
            }

            if (loopCnt >= maxSupportedParam)
            {
                EPRINT(CAMERA_INTERFACE, "%s: codec support not found: [camera=%d], [codec=%s]", pStreamType[streamType], camIndex, pSrcCodec[streamType]);
                snprintf(advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s: Codec mismatch", pStreamType[streamType]);
                WriteEvent(LOG_CAMERA_EVENT, LOG_STREAM_PARAM_COPY_TO_CAM, details, advncDetail, EVENT_FAIL);
                continue;
            }

            maxSupportedParam = 0;
            if (CMD_SUCCESS != GetSupportedResolution(camIndex, pSrcCodec[streamType], streamType, dstCameraProfile, cameraResolution, &maxSupportedParam))
            {
                EPRINT(CAMERA_INTERFACE, "%s: fail to get supported resolution list: [camera=%d]", pStreamType[streamType], camIndex);
                snprintf(advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s: Resolution mismatch", pStreamType[streamType]);
                WriteEvent(LOG_CAMERA_EVENT, LOG_STREAM_PARAM_COPY_TO_CAM, details, advncDetail, EVENT_FAIL);
                continue;
            }

            for (loopCnt = 0; loopCnt < maxSupportedParam; loopCnt++)
            {
                if (strcmp(pSrcResolution[streamType], cameraResolution[loopCnt]) == 0)
                {
                    /* Resolution support found */
                    break;
                }
            }

            if (loopCnt >= maxSupportedParam)
            {
                EPRINT(CAMERA_INTERFACE, "%s: resolution support not found: [camera=%d], [resolution=%s]", pStreamType[streamType], camIndex, pSrcResolution[streamType]);
                snprintf(advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s: Resolution mismatch", pStreamType[streamType]);
                WriteEvent(LOG_CAMERA_EVENT, LOG_STREAM_PARAM_COPY_TO_CAM, details, advncDetail, EVENT_FAIL);
                continue;
            }

            cameraFps = 0;
            if (CMD_SUCCESS != GetSupportedFramerate(camIndex, pSrcCodec[streamType], pSrcResolution[streamType], streamType, dstCameraProfile, &cameraFps))
            {
                EPRINT(CAMERA_INTERFACE, "%s: fail to get supported framerate list: [camera=%d]", pStreamType[streamType], camIndex);
                snprintf(advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s: Framerate mismatch", pStreamType[streamType]);
                WriteEvent(LOG_CAMERA_EVENT, LOG_STREAM_PARAM_COPY_TO_CAM, details, advncDetail, EVENT_FAIL);
                continue;
            }

            /* Check FPS bit in supported list */
            if (GET_BIT(cameraFps, (srcFramerate[streamType] - 1)) == 0)
            {
                EPRINT(CAMERA_INTERFACE, "%s: framerate support not found: [camera=%d], [srcFps=%d], [dstFps=0x%llx]",
                       pStreamType[streamType], camIndex, srcFramerate[streamType], cameraFps);
                snprintf(advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s: Framerate mismatch", pStreamType[streamType]);
                WriteEvent(LOG_CAMERA_EVENT, LOG_STREAM_PARAM_COPY_TO_CAM, details, advncDetail, EVENT_FAIL);
                continue;
            }

            maxSupportedParam = minBitrateIndex = maxBitrateIndex = 0;
            if (CMD_SUCCESS != GetSupportedBitRate(camIndex, pSrcCodec[streamType], streamType, dstCameraProfile, cameraBitrate,
                                                   &maxSupportedParam, &minBitrateIndex, &maxBitrateIndex))
            {
                EPRINT(CAMERA_INTERFACE, "%s: fail to get supported bitrate list: [camera=%d]", pStreamType[streamType], camIndex);
                snprintf(advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s: Bitrate mismatch", pStreamType[streamType]);
                WriteEvent(LOG_CAMERA_EVENT, LOG_STREAM_PARAM_COPY_TO_CAM, details, advncDetail, EVENT_FAIL);
                continue;
            }

            if ((srcBitrateValue[streamType] < minBitrateIndex) || (srcBitrateValue[streamType] > maxBitrateIndex))
            {
                EPRINT(CAMERA_INTERFACE, "%s: bitrate support not found: [camera=%d], [srcBitrate=%d], [dstBitrate=%d:%d]",
                       pStreamType[streamType], camIndex, srcBitrateValue[streamType], minBitrateIndex, maxBitrateIndex);
                snprintf(advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s: Bitrate mismatch", pStreamType[streamType]);
                WriteEvent(LOG_CAMERA_EVENT, LOG_STREAM_PARAM_COPY_TO_CAM, details, advncDetail, EVENT_FAIL);
                continue;
            }

            maxSupportedParam = 0;
            if (CMD_SUCCESS != GetSupportedQuality(camIndex, pSrcCodec[streamType], streamType, dstCameraProfile, &maxSupportedParam))
            {
                EPRINT(CAMERA_INTERFACE, "%s: fail to get supported quality list: [camera=%d]", pStreamType[streamType], camIndex);
                snprintf(advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s: Quality mismatch", pStreamType[streamType]);
                WriteEvent(LOG_CAMERA_EVENT, LOG_STREAM_PARAM_COPY_TO_CAM, details, advncDetail, EVENT_FAIL);
                continue;
            }

            /* Check quality support */
            if ((maxSupportedParam == 0) || (srcQuality[streamType] > maxSupportedParam))
            {
                EPRINT(CAMERA_INTERFACE, "%s: quality support not found: [camera=%d], [srcQuality=%d], [dstQuality=%d]",
                       pStreamType[streamType], camIndex, srcQuality[streamType], maxSupportedParam);
                snprintf(advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s: Quality mismatch", pStreamType[streamType]);
                WriteEvent(LOG_CAMERA_EVENT, LOG_STREAM_PARAM_COPY_TO_CAM, details, advncDetail, EVENT_FAIL);
                continue;
            }

            /* Prepare stream configuration to be written */
            isNewStreamParamApply = TRUE;
            if (streamType == MAIN_STREAM)
            {
                snprintf(dstStreamConfig.videoEncoding, sizeof(dstStreamConfig.videoEncoding), "%s", pSrcCodec[streamType]);
                snprintf(dstStreamConfig.resolution, sizeof(dstStreamConfig.resolution), "%s", pSrcResolution[streamType]);
                dstStreamConfig.framerate = srcFramerate[streamType];
                dstStreamConfig.bitrateMode = srcBitrateMode[streamType];
                dstStreamConfig.bitrateValue = srcBitrateValue[streamType];
                dstStreamConfig.quality = srcQuality[streamType];
                dstStreamConfig.gop = (srcGop[streamType] < MIN_CAM_GOP) ? MIN_CAM_GOP : ((srcGop[streamType] > MAX_CAM_GOP) ? MAX_CAM_GOP : srcGop[streamType]);
                dstStreamConfig.enableAudio = srcAudioF[streamType];
            }
            else
            {
                snprintf(dstStreamConfig.videoEncodingSub, sizeof(dstStreamConfig.videoEncodingSub), "%s", pSrcCodec[streamType]);
                snprintf(dstStreamConfig.resolutionSub, sizeof(dstStreamConfig.resolutionSub), "%s", pSrcResolution[streamType]);
                dstStreamConfig.framerateSub = srcFramerate[streamType];
                dstStreamConfig.bitrateModeSub = srcBitrateMode[streamType];
                dstStreamConfig.bitrateValueSub = srcBitrateValue[streamType];
                dstStreamConfig.qualitySub = srcQuality[streamType];
                dstStreamConfig.gopSub = (srcGop[streamType] < MIN_CAM_GOP) ? MIN_CAM_GOP : ((srcGop[streamType] > MAX_CAM_GOP) ? MAX_CAM_GOP : srcGop[streamType]);
                dstStreamConfig.enableAudioSub = srcAudioF[streamType];
            }
        }

        /* No need to write config in file */
        if (isNewStreamParamApply == FALSE)
        {
            continue;
        }

        /* Write destination camera configuration. After writting configuration, config module will notify to camera interface
         * for config update. config update will give trigger to camera interface state machine for streams (Main and/or Sub).
         * state machine will give internal trigger of connectivity based on camera status. connectivity will apply new parameters
         * in camera and start fetching stream with new params. No stream param related errors will be identified onwards flow */
        WriteSingleStreamConfig(camIndex, &dstStreamConfig);
        DPRINT(CAMERA_INTERFACE, "new stream param copy to camera written: [camera=%d], [dstCamera=%d]", srcCamIndex, camIndex);
    }

    DPRINT(CAMERA_INTERFACE, "stream param copy to camera done: [camera=%d], [mainMask1=0x%llx], [mainMask2=0x%llx], [subMask1=0x%llx], [subMask2=0x%llx]",
            srcCamIndex, pCopyToCamMask[MAIN_STREAM].bitMask[0], pCopyToCamMask[MAIN_STREAM].bitMask[1],
            pCopyToCamMask[SUB_STREAM].bitMask[0], pCopyToCamMask[SUB_STREAM].bitMask[1]);

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Process motion detection area copy to camera
 * @param   srcCamIndex
 * @param   pSrcCamConfig
 * @param   copyToCamMask
 * @return  Status
 */
NET_CMD_STATUS_e MotionDetectionCopyToCamera(UINT8 srcCamIndex, CAMERA_CONFIG_t *pSrcCamConfig, CAMERA_BIT_MASK_t copyToCamMask)
{
    UINT8           camIndex;
    CAMERA_CONFIG_t dstCameraConfig;
    CHAR            details[MAX_EVENT_DETAIL_SIZE];

    /* Validate camera index */
    if (srcCamIndex >= getMaxCameraForCurrentVariant())
    {
        EPRINT(CAMERA_INTERFACE, "invld camera index found: [camera=%d]", srcCamIndex);
        return CMD_PROCESS_ERROR;
    }

    /* Check camera is enabled or not */
    if (pSrcCamConfig->camera == DISABLE)
    {
        EPRINT(CAMERA_INTERFACE, "camera disabled: [camera=%d]", srcCamIndex);
        snprintf(details, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(srcCamIndex));
        WriteEvent(LOG_CAMERA_EVENT, LOG_MOTION_DETECTION_COPY_TO_CAM, details, "Source Camera Disabled", EVENT_FAIL);
        return CMD_CHANNEL_DISABLED;
    }

    /* Check for motion detection  feature support */
    if (CheckforVideoAnalyticsSupport(srcCamIndex) != MOTION_AREA_METHOD_BLOCK)
    {
        EPRINT(CAMERA_INTERFACE, "motion detection not supported: [camera=%d]", srcCamIndex);
        snprintf(details, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(srcCamIndex));
        WriteEvent(LOG_CAMERA_EVENT, LOG_MOTION_DETECTION_COPY_TO_CAM, details, "Feature not supported", EVENT_FAIL);
        return CMD_FEATURE_NOT_SUPPORTED;
    }

    for (camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
    {
        /* Skip source camera from copy to camera :) */
        if (camIndex == srcCamIndex)
        {
            continue;
        }

        /* Check motion detection area copy needed for current camera */
        if (GET_CAMERA_MASK_BIT(copyToCamMask, camIndex) == 0)
        {
            continue;
        }

        /* Read camera config */
        ReadSingleCameraConfig(camIndex, &dstCameraConfig);

        /* If config disable is requested, check in source camera config */
        if (pSrcCamConfig->motionDetectionStatus == DISABLE)
        {
            if (dstCameraConfig.motionDetectionStatus == ENABLE)
            {
                if (dstCameraConfig.camera == ENABLE)
                {
                    /* clear motion detection zone */
                    resetMotionWindowConfig(camIndex);
                }

                /* disabling motion detection status in config  */
                dstCameraConfig.motionDetectionStatus = DISABLE;
                pthread_rwlock_wrlock(&camCnfgNotifyControl[camIndex].configNotifyLock);
                camCnfgNotifyControl[camIndex].cameraConfigNotifyF = FALSE;
                pthread_rwlock_unlock(&camCnfgNotifyControl[camIndex].configNotifyLock);
                WriteSingleCameraConfig(camIndex, &dstCameraConfig);
                pthread_rwlock_wrlock(&camCnfgNotifyControl[camIndex].configNotifyLock);
                camCnfgNotifyControl[camIndex].cameraConfigNotifyF = TRUE;
                pthread_rwlock_unlock(&camCnfgNotifyControl[camIndex].configNotifyLock);
            }

            /* On config disable, above implementation will do needful */
            continue;
        }

        /* Check camera is enabled or not */
        ReadSingleCameraConfig(camIndex, &dstCameraConfig);
        if (dstCameraConfig.camera == DISABLE)
        {
            EPRINT(CAMERA_INTERFACE, "camera disabled: [camera=%d]", camIndex);
            continue;
        }

        /* Check camera connectivity */
        snprintf(details, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(camIndex));
        if (GetCamEventStatus(camIndex, CONNECTION_FAILURE)== INACTIVE)
        {
            EPRINT(CAMERA_INTERFACE, "camera offline: [camera=%d]", camIndex);
            WriteEvent(LOG_CAMERA_EVENT, LOG_MOTION_DETECTION_COPY_TO_CAM, details, "Camera Offline", EVENT_FAIL);
            continue;
        }

        /* Check for motion detection feature support */
        if (CheckforVideoAnalyticsSupport(camIndex) != MOTION_AREA_METHOD_BLOCK)
        {
            EPRINT(CAMERA_INTERFACE, "motion detection not supported: [camera=%d]", camIndex);
            WriteEvent(LOG_CAMERA_EVENT, LOG_MOTION_DETECTION_COPY_TO_CAM, details, "Feature not supported", EVENT_FAIL);
            continue;
        }

        /* if source camera is setting no-motion event but destination camera does not support it */
        if ((1 == motionWindowRequest[srcCamIndex].motionParam.isNoMotionEvent) && (FALSE == IsNoMotionEventSupported(camIndex)))
        {
            EPRINT(CAMERA_INTERFACE, "no motion detection not supported: [camera=%d]", camIndex);
            WriteEvent(LOG_CAMERA_EVENT, LOG_MOTION_DETECTION_COPY_TO_CAM, details, "No motion event not supported", EVENT_FAIL);
            continue;
        }

        /* Set motion window config in camera */
        if (CMD_SUCCESS != SetMotionWindowConfig(camIndex, &motionWindowRequest[srcCamIndex].motionParam, NULL, 0, CLIENT_CB_TYPE_NATIVE, ENABLE))
        {
            EPRINT(CAMERA_INTERFACE, "fail to set motion detection area: [camera=%d]", camIndex);
            WriteEvent(LOG_CAMERA_EVENT, LOG_MOTION_DETECTION_COPY_TO_CAM, details, NULL, EVENT_FAIL);
            continue;
        }

        DPRINT(CAMERA_INTERFACE, "motion detection area copied to camera: [camera=%d], [dstCamera=%d]", srcCamIndex, camIndex);
    }

    DPRINT(CAMERA_INTERFACE, "motion detection copy to camera done: [camera=%d], [mask1=0x%llx], [mask2=0x%llx]",
           srcCamIndex, copyToCamMask.bitMask[0], copyToCamMask.bitMask[1]);
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief ValidatePtzPresetCfg
 * @param cameraIndex
 * @return Network command status
 */
NET_CMD_STATUS_e ValidatePtzPresetCfg(PTZ_PRESET_CONFIG_t *newCfg, UINT8 cameraIndex)
{
    /* Skip validation if default config request */
    if ((STATUS_OK == strcmp(newCfg->name, DfltPtzPresetCfg.name)) &&
            (STATUS_OK == strcmp(newCfg->name, DfltPtzPresetCfg.name)))
    {
        return CMD_SUCCESS;
    }

    return ValidatePtzFuncSupport(cameraIndex, PAN_TILT_FUNCTION);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It will be called when there is change in camera PTZ preset configuration
 * @param   newPtzPosition
 * @param   oldPtzPresetCfg
 * @param   ptzIndex
 * @param   cameraIndex
 */
void CiPtzPositionConfigNotify(PTZ_PRESET_CONFIG_t newPtzPresetCfg, PTZ_PRESET_CONFIG_t *oldPtzPresetCfg, UINT8 cameraIndex, UINT8 ptzIndex)
{
    /* Is any change in preset name? */
    if (strcmp(newPtzPresetCfg.name, oldPtzPresetCfg->name) == 0)
    {
        return;
    }

    if (newPtzPresetCfg.name[0] != '\0')
    {
        /* Provide new config because name will be there */
        DPRINT(CAMERA_INTERFACE, "ptz added: [camera=%d], [ptzIndex=%d]", cameraIndex, ptzIndex);
        storePtzPosition(cameraIndex, ptzIndex, ADDED, &newPtzPresetCfg);
    }
    else
    {
        /* Provide old config because token will be there */
        DPRINT(CAMERA_INTERFACE, "ptz deleted: [camera=%d], [ptzIndex=%d]", cameraIndex, ptzIndex);
        storePtzPosition(cameraIndex, ptzIndex, REMOVED, oldPtzPresetCfg);
        updatePtzPosInAllCfg(cameraIndex, (ptzIndex + 1));
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief CiLanCfgUpdate : will be called when there is change in device network config table
 * @param lanNo
 * @param newLanConfig
 * @param oldLanConfig
 */
void CiLanCfgUpdate(LAN_CONFIG_ID_e lanNo, LAN_CONFIG_t newLanConfig, LAN_CONFIG_t * oldLanConfig)
{
    GENERAL_CONFIG_t generalConfig;

    ReadGeneralConfig(&generalConfig);

    if(ValidateIpSubnetWithLan1(generalConfig.autoConfigStartIp, generalConfig.autoConfigEndIp) == FALSE)
    {
        generalConfig.retainIpAddresses = TRUE;
        WriteGeneralConfig(&generalConfig);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function geneartes and set the para for sending audio to camera
 * @param   cameraIndex
 * @param   cameraType
 * @param   audToCamInfo
 * @return  Network command status
 */
NET_CMD_STATUS_e GenerateAudioToCameraRequest(UINT8 cameraIndex, CAMERA_TYPE_e *cameraType, AUD_TO_CAM_INFO_t *audToCamInfo)
{
    NET_CMD_STATUS_e    cmdResp;
    CAMERA_CONFIG_t     camConfig;
    IP_CAMERA_CONFIG_t  ipCamCfg;

    /* Check camera is enabled or disabled */
    ReadSingleCameraConfig(cameraIndex, &camConfig);
    if(DISABLE == camConfig.camera)
    {
        EPRINT(CAMERA_INTERFACE, "camera is disable: [camera=%d]", cameraIndex);
        return CMD_CHANNEL_DISABLED;
    }

    /* Onvif not supported; only Brand Model is allowed */
    ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);
    if (TRUE == ipCamCfg.onvifSupportF)
    {
        EPRINT(CAMERA_INTERFACE, "two way audio not supported over onvif: [camera=%d]", cameraIndex);
        return CMD_AUD_SND_REQ_FAIL;
    }

    /* Get camera type from config */
    *cameraType = camConfig.type;

    /* Check Camera Connection Start */
    if (INACTIVE == GetCamEventStatus(cameraIndex, CONNECTION_FAILURE))
    {
        EPRINT(CAMERA_INTERFACE, "camera disconnected: [camera=%d]", cameraIndex);
        return CMD_CAM_DISCONNECTED;
    }

    /* Check Camera capabilities Start */
    CAMERA_CAPABILTY_INFO_t camCapability;
    cmdResp = GetSupportedCapability(cameraIndex, &camCapability);
    if (CMD_SUCCESS != cmdResp)
    {
        EPRINT(CAMERA_INTERFACE, "fail to get supported feature capability: [camera=%d]", cameraIndex);
        return cmdResp;
    }

    // If Audio Feature is not suported by camera
    if (YES != camCapability.audioSupport)
    {
        EPRINT(CAMERA_INTERFACE, "two way audio feature not supported in onvif: [camera=%d]", cameraIndex);
        return CMD_FEATURE_NOT_SUPPORTED;
    }

    // for camera initiation
    if (AUTO_ADDED_CAMERA == camConfig.type)
    {
        /* cmdArg1 --> G711_64K = 0 */
        GetCiRequestUrl(cameraIndex, REQ_URL_START_AUDIO, 0, CAM_REQ_CONTROL, tcpStartSndAudReqCb, &twoWayAudioRequest[cameraIndex], NULL);
    }
    else // for brand model
    {
        SEND_AUDIO_INFO_t sendAudioInfo;

        snprintf(sendAudioInfo.userName, MAX_USERNAME_WIDTH, "%s", ipCamCfg.username);
        snprintf(sendAudioInfo.password, MAX_PASSWORD_WIDTH, "%s", ipCamCfg.password);

        cmdResp = GetCameraBrandModelUrl(cameraIndex, REQ_URL_START_AUDIO, NULL, audToCamInfo, &sendAudioInfo, NULL, NULL);
        if (cmdResp != CMD_SUCCESS)
        {
            EPRINT(CAMERA_INTERFACE, "audio to camera url not found: [camera=%d]", cameraIndex);
            return cmdResp;
        }

        audToCamInfo->httpUserAgent = sendAudioInfo.httpUserAgent;
        snprintf(audToCamInfo->userName, MAX_USERNAME_WIDTH, "%s", ipCamCfg.username);
        snprintf(audToCamInfo->password, MAX_PASSWORD_WIDTH, "%s", ipCamCfg.password);

        audToCamInfo->httpPort = ipCamCfg.httpPort;
        snprintf(audToCamInfo->ipAddr, sizeof(audToCamInfo->ipAddr), "%s", ipCamCfg.cameraAddress);
        DPRINT(CAMERA_INTERFACE, "generate audio to camera request success: [camera=%d]", cameraIndex);
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief StartSendAudioToCameraRequest
 * @param cameraIndex
 * @return Network Command Status
 */
NET_CMD_STATUS_e StartSendAudioToCameraRequest(UINT8 cameraIndex)
{
    NET_CMD_STATUS_e    cmdResp;
    IP_CAMERA_CONFIG_t  ipCamConfig;

    ReadSingleIpCameraConfig(cameraIndex, &ipCamConfig);
    cmdResp = sendReqToCamera(&twoWayAudioRequest[cameraIndex], &ipCamConfig, MAIN_STREAM);
    if (CMD_SUCCESS == cmdResp)
    {
        DPRINT(CAMERA_INTERFACE, "start send audio to camera success: [camera=%d]", cameraIndex);
    }
    else
    {
        EPRINT(CAMERA_INTERFACE, "start send audio to camera fail: [camera=%d]", cameraIndex);
    }

    return cmdResp;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief StopSendAudioToCameraRequest : This will send stop command for Aud out req
 * @param cameraIndex
 * @return Network Command Status
 */
NET_CMD_STATUS_e StopSendAudioToCameraRequest(UINT8 cameraIndex)
{
    NET_CMD_STATUS_e    cmdResp;
    IP_CAMERA_CONFIG_t  ipCamConfig;

    /* cmdArg1 --> G711_64K = 0 */
    ReadSingleIpCameraConfig(cameraIndex, &ipCamConfig);
    GetCiRequestUrl(cameraIndex, REQ_URL_STOP_AUDIO, 0, CAM_REQ_CONTROL, tcpStopSndAudReqCb, &twoWayAudioRequest[cameraIndex], NULL);
    cmdResp = sendReqToCamera(&twoWayAudioRequest[cameraIndex], &ipCamConfig, MAIN_STREAM);
    if (CMD_SUCCESS == cmdResp)
    {
        DPRINT(CAMERA_INTERFACE, "stop send audio to camera success: [camera=%d]", cameraIndex);
    }
    else
    {
        EPRINT(CAMERA_INTERFACE, "stop send audio to camera fail: [camera=%d]", cameraIndex);
    }

    return cmdResp;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief setOsds : sends setOSD request to camera
 * @param cameraIndex
 * @param osdParam
 * @return Network Command Status
 */
static NET_CMD_STATUS_e setOsds(UINT8 cameraIndex, OSD_PARAM_t *osdParam)
{
    NET_CMD_STATUS_e 	requestStatus = CMD_SUCCESS;
    IP_CAMERA_CONFIG_t 	ipCamCfg;

    if (cameraIndex >= getMaxCameraForCurrentVariant())
    {
        EPRINT(CAMERA_INTERFACE, "invld camera index found: [camera=%d]", cameraIndex);
        return CMD_PROCESS_ERROR;
    }

    DPRINT(CAMERA_INTERFACE, "set osd request recv: [camera=%d]", cameraIndex);

    MUTEX_LOCK(setOsdsRequest[cameraIndex].request.camReqFlagLock);
    if(setOsdsRequest[cameraIndex].request.camReqBusyF == BUSY)
    {
        MUTEX_UNLOCK(setOsdsRequest[cameraIndex].request.camReqFlagLock);
        WPRINT(CAMERA_INTERFACE, "set osd request already in progress: [camera=%d]", cameraIndex);
        return CMD_CAM_REQUEST_IN_PROCESS;
    }
    setOsdsRequest[cameraIndex].request.camReqBusyF = BUSY;
    MUTEX_UNLOCK(setOsdsRequest[cameraIndex].request.camReqFlagLock);

    /* Get total text overlay */
    osdParam->channelNameOverlayMax = GetSupportedTextOverlay(cameraIndex);
    setOsdsRequest[cameraIndex].request.clientCb[ON] = NULL;
    setOsdsRequest[cameraIndex].request.clientSocket[ON] = 0;
    ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);

    if (ipCamCfg.onvifSupportF == TRUE)
    {
        if ((ipCamCfg.brand[0] == '\0') || (ipCamCfg.model[0] == '\0'))
        {
            EPRINT(CAMERA_INTERFACE, "onvif capability not recv, brand-model unknown: [camera=%d]", cameraIndex);
            requestStatus = CMD_ONVIF_CAM_CAPABILITY_ERROR;
        }
        else
        {
            requestStatus = sendOnvifCommonCmd(cameraIndex, ONVIF_OSD_SET_REQ, onvifSetOsdCb, &ipCamCfg, osdParam);
        }
    }
    else if(CameraType(cameraIndex) == AUTO_ADDED_CAMERA)
    {
        GetCiRequestUrl(cameraIndex, REQ_URL_SET_OSD, 0, CAM_REQ_CONFIG, tcpSetOverlayCnfgCb, &setOsdsRequest[cameraIndex].request, osdParam);
        requestStatus =	sendReqToCamera(&setOsdsRequest[cameraIndex].request, &ipCamCfg, MAIN_STREAM);
    }
    else
    {
        requestStatus = GetCameraBrandModelUrl(cameraIndex, REQ_URL_SET_OSD, &setOsdsRequest[cameraIndex].request, osdParam, NULL, NULL, NULL);
        if (requestStatus != CMD_SUCCESS)
        {
            EPRINT(CAMERA_INTERFACE, "fail to set osd: [camera=%d], [numOfreq=%d]", cameraIndex, setOsdsRequest[cameraIndex].request.numOfRequest);
        }
        else
        {
            DPRINT(CAMERA_INTERFACE, "set osd successfully: [camera=%d], [numOfreq=%d]", cameraIndex,setOsdsRequest[cameraIndex].request.numOfRequest);
            setOsdsRequest[cameraIndex].request.requestCount = 0;
            setOsdsRequest[cameraIndex].request.requestStatus = CMD_PROCESS_ERROR;
            requestStatus =	sendReqToCamera(&setOsdsRequest[cameraIndex].request, &ipCamCfg, MAIN_STREAM);
        }
    }

    if (requestStatus != CMD_SUCCESS)
    {
        restoreDefaultOsdParam(cameraIndex);
        MUTEX_LOCK(setOsdsRequest[cameraIndex].request.camReqFlagLock);
        setOsdsRequest[cameraIndex].request.camReqBusyF = FREE;
        MUTEX_UNLOCK(setOsdsRequest[cameraIndex].request.camReqFlagLock);
    }

    return(requestStatus);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief restoreDefaultOsdParam
 * @param cameraIndex
 */
static void restoreDefaultOsdParam(UINT8 cameraIndex)
{
    UINT8 requestCount;

    for(requestCount = 0; requestCount < MAX_URL_REQUEST; requestCount++)
    {
        setOsdsRequest[cameraIndex].request.http[requestCount] = INVALID_HTTP_HANDLE;
    }

    setOsdsRequest[cameraIndex].request.numOfRequest = 0;
    memset(setOsdsRequest[cameraIndex].request.clientCb, '\0', sizeof(setOsdsRequest[cameraIndex].request.clientCb));
    setOsdsRequest[cameraIndex].request.rtspHandle = INVALID_RTSP_HANDLE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetPrivacyMaskWindowConfig
 * @param cameraIndex
 * @param callBack
 * @param clientSocket
 * @param clientCbType
 * @return Network Command Status
 */
NET_CMD_STATUS_e GetPrivacyMaskWindowConfig(UINT8 cameraIndex,NW_CMD_REPLY_CB callBack, INT32 clientSocket, CLIENT_CB_TYPE_e clientCbType)
{
    NET_CMD_STATUS_e    requestStatus = CMD_SUCCESS;
    IP_CAMERA_CONFIG_t  ipCamCfg;

    if (cameraIndex >= getMaxCameraForCurrentVariant())
    {
        EPRINT(CAMERA_INTERFACE, "invld camera index found: [camera=%d]", cameraIndex);
        return CMD_PROCESS_ERROR;
    }

    if (CheckforPrivacyMaskSupport(cameraIndex) != PRIVACY_MASK_METHOD_POINT)
    {
        WPRINT(CAMERA_INTERFACE, "privacy mask not supported: [camera=%d]", cameraIndex);
        return CMD_FEATURE_NOT_SUPPORTED;
    }

    MUTEX_LOCK(privacyMaskRequest[cameraIndex].request.camReqFlagLock);
    if(privacyMaskRequest[cameraIndex].request.camReqBusyF == BUSY)
    {
        MUTEX_UNLOCK(privacyMaskRequest[cameraIndex].request.camReqFlagLock);
        WPRINT(CAMERA_INTERFACE, "get privacy mask request busy: [camera=%d]", cameraIndex);
        return CMD_CAM_REQUEST_IN_PROCESS;
    }

    privacyMaskRequest[cameraIndex].request.camReqBusyF = BUSY;
    MUTEX_UNLOCK(privacyMaskRequest[cameraIndex].request.camReqFlagLock);

    DPRINT(CAMERA_INTERFACE, "get privacy mask request received: [camera=%d]", cameraIndex);
    privacyMaskRequest[cameraIndex].request.clientCb[ON] = callBack;
    privacyMaskRequest[cameraIndex].request.clientSocket[ON] = clientSocket;
    privacyMaskRequest[cameraIndex].request.clientCbType = clientCbType;
    memset(privacyMaskRequest[cameraIndex].privacyArea, 0, sizeof(privacyMaskRequest[cameraIndex].privacyArea));
    ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);

    if (ipCamCfg.onvifSupportF == TRUE)
    {
        if ((ipCamCfg.brand[0] == '\0') || (ipCamCfg.model[0] == '\0'))
        {
            EPRINT(CAMERA_INTERFACE, "onvif capability not recv, brand-model unknown: [camera=%d]", cameraIndex);
            requestStatus = CMD_ONVIF_CAM_CAPABILITY_ERROR;
        }
        else
        {
            EPRINT(CAMERA_INTERFACE, "privacy doesn't support through onvif: [camera=%d]", cameraIndex);
            requestStatus = CMD_FEATURE_NOT_SUPPORTED;
        }
    }
    else if(CameraType(cameraIndex) == AUTO_ADDED_CAMERA)
    {
        GetCiRequestUrl(cameraIndex, REQ_URL_GET_PRIVACY_MASK, 0, CAM_REQ_CONFIG, tcpGetMaxPrivacyMaskCnfgCb, &privacyMaskRequest[cameraIndex].request, NULL);
        privacyMaskRequest[cameraIndex].request.requestCount = 0;
        requestStatus =	sendReqToCamera(&privacyMaskRequest[cameraIndex].request, &ipCamCfg, MAIN_STREAM);
    }
    else
    {
        requestStatus = GetCameraBrandModelUrl(cameraIndex, REQ_URL_GET_MAX_PRIVACY_MASK, &privacyMaskRequest[cameraIndex].request,
                                               &privacyMaskRequest[cameraIndex].maxSupportedMaskWindow, NULL, NULL, NULL);
        if (requestStatus != CMD_SUCCESS)
        {
            EPRINT(CAMERA_INTERFACE, "fail to get privacy mask url: [camera=%d]", cameraIndex);
        }
        else
        {
            privacyMaskRequest[cameraIndex].request.requestCount = 0;
            privacyMaskRequest[cameraIndex].request.requestStatus = CMD_PROCESS_ERROR;
            requestStatus =	sendReqToCamera(&privacyMaskRequest[cameraIndex].request, &ipCamCfg, MAIN_STREAM);
        }
    }

    if (requestStatus != CMD_SUCCESS)
    {
        restoreDefaultPrivacyMaskConfig(cameraIndex);
        MUTEX_LOCK(privacyMaskRequest[cameraIndex].request.camReqFlagLock);
        privacyMaskRequest[cameraIndex].request.camReqBusyF = FREE;
        MUTEX_UNLOCK(privacyMaskRequest[cameraIndex].request.camReqFlagLock);
    }

    return requestStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetMotionWindowConfig
 * @param cameraIndex
 * @param callBack
 * @param clientSocket
 * @param clientCbType
 * @return Network Command Status
 */
NET_CMD_STATUS_e GetMotionWindowConfig(UINT8 cameraIndex,NW_CMD_REPLY_CB callBack, INT32 clientSocket, CLIENT_CB_TYPE_e clientCbType)
{
    NET_CMD_STATUS_e    requestStatus = CMD_SUCCESS;
    IP_CAMERA_CONFIG_t  ipCamCfg;

    if (cameraIndex >= getMaxCameraForCurrentVariant())
    {
        EPRINT(CAMERA_INTERFACE, "invld camera index found: [camera=%d]", cameraIndex);
        return CMD_PROCESS_ERROR;
    }

    if (CheckforVideoAnalyticsSupport(cameraIndex) != MOTION_AREA_METHOD_BLOCK)
    {
        WPRINT(CAMERA_INTERFACE, "motion window not supported: [camera=%d]", cameraIndex);
        return CMD_FEATURE_NOT_SUPPORTED;
    }

    MUTEX_LOCK(motionWindowRequest[cameraIndex].request.camReqFlagLock);
    if (motionWindowRequest[cameraIndex].request.camReqBusyF == BUSY)
    {
        MUTEX_UNLOCK(motionWindowRequest[cameraIndex].request.camReqFlagLock);
        WPRINT(CAMERA_INTERFACE, "get motion window request busy: [camera=%d]", cameraIndex);
        return CMD_CAM_REQUEST_IN_PROCESS;
    }
    motionWindowRequest[cameraIndex].request.camReqBusyF = BUSY;
    MUTEX_UNLOCK(motionWindowRequest[cameraIndex].request.camReqFlagLock);

    DPRINT(CAMERA_INTERFACE, "get motion window request received: [camera=%d]", cameraIndex);
    motionWindowRequest[cameraIndex].request.clientCb[ON] = callBack;
    motionWindowRequest[cameraIndex].request.clientSocket[ON] = clientSocket;
    motionWindowRequest[cameraIndex].request.clientCbType = clientCbType;
    memset(&motionWindowRequest[cameraIndex].motionParam, 0, sizeof(motionWindowRequest[cameraIndex].motionParam));
    ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);

    if (ipCamCfg.onvifSupportF == TRUE)
    {
        if ((ipCamCfg.brand[0] == '\0') || (ipCamCfg.model[0] == '\0'))
        {
            EPRINT(CAMERA_INTERFACE, "onvif capability not recv, brand-model unknown: [camera=%d]", cameraIndex);
            requestStatus = CMD_ONVIF_CAM_CAPABILITY_ERROR;
        }
        else
        {
            requestStatus = sendOnvifCommonCmd(cameraIndex, ONVIF_GET_MOTION_WINDOW, onvifGetMotionWindowCb,
                                               &ipCamCfg, &motionWindowRequest[cameraIndex].motionParam);
        }
    }
    else if(CameraType(cameraIndex) == AUTO_ADDED_CAMERA)
    {
        GetCiRequestUrl(cameraIndex, REQ_URL_GET_MOTION_WINDOW, 0, CAM_REQ_CONFIG, tcpGetMotionCnfgCb, &motionWindowRequest[cameraIndex].request, NULL);
        requestStatus =	sendReqToCamera(&motionWindowRequest[cameraIndex].request, &ipCamCfg, MAIN_STREAM);
    }
    else
    {
        requestStatus = GetCameraBrandModelUrl(cameraIndex, REQ_URL_GET_MOTION_WINDOW, &motionWindowRequest[cameraIndex].request, NULL, NULL, NULL, NULL);
        if (requestStatus != CMD_SUCCESS)
        {
            EPRINT(CAMERA_INTERFACE, "fail to get motion window url: [camera=%d]", cameraIndex);
        }
        else
        {
            motionWindowRequest[cameraIndex].request.requestCount = 0;
            motionWindowRequest[cameraIndex].request.requestStatus = CMD_PROCESS_ERROR;
            requestStatus =	sendReqToCamera(&motionWindowRequest[cameraIndex].request, &ipCamCfg, MAIN_STREAM);
        }
    }

    if (requestStatus != CMD_SUCCESS)
    {
        restoreDefaultMotionConfig(cameraIndex);
        MUTEX_LOCK(motionWindowRequest[cameraIndex].request.camReqFlagLock);
        motionWindowRequest[cameraIndex].request.camReqBusyF = FREE;
        MUTEX_UNLOCK(motionWindowRequest[cameraIndex].request.camReqFlagLock);
    }

    return requestStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief SetMotionWindowConfig
 * @param cameraIndex
 * @param motionDataBuf
 * @param callback
 * @param clientSocket
 * @param clientCbType
 * @param newConfigStatusF : motion detection status flag
 * @return Network Command Status
 */
NET_CMD_STATUS_e SetMotionWindowConfig(UINT8 cameraIndex, MOTION_BLOCK_METHOD_PARAM_t *pMotionDataBuf, NW_CMD_REPLY_CB callback,
                                       INT32 clientSocket, CLIENT_CB_TYPE_e clientCbType, BOOL newConfigStatusF)
{
    NET_CMD_STATUS_e    requestStatus = CMD_SUCCESS;
    IP_CAMERA_CONFIG_t  ipCamCfg;

    if (cameraIndex >= getMaxCameraForCurrentVariant())
    {
        EPRINT(CAMERA_INTERFACE, "invld camera index found: [camera=%d]", cameraIndex);
        return CMD_PROCESS_ERROR;
    }

    if (CheckforVideoAnalyticsSupport(cameraIndex) != MOTION_AREA_METHOD_BLOCK)
    {
        WPRINT(CAMERA_INTERFACE, "motion window not supported: [camera=%d]", cameraIndex);
        return CMD_FEATURE_NOT_SUPPORTED;
    }

    MUTEX_LOCK(motionWindowRequest[cameraIndex].request.camReqFlagLock);
    if (motionWindowRequest[cameraIndex].request.camReqBusyF == BUSY)
    {
        MUTEX_UNLOCK(motionWindowRequest[cameraIndex].request.camReqFlagLock);
        WPRINT(CAMERA_INTERFACE, "set motion window request busy: [camera=%d]", cameraIndex);
        return CMD_CAM_REQUEST_IN_PROCESS;
    }

    motionWindowRequest[cameraIndex].request.camReqBusyF = BUSY;
    MUTEX_UNLOCK(motionWindowRequest[cameraIndex].request.camReqFlagLock);

    DPRINT(CAMERA_INTERFACE, "set motion window request received: [camera=%d]", cameraIndex);
    motionWindowRequest[cameraIndex].request.clientCb[ON] = callback;
    motionWindowRequest[cameraIndex].request.clientSocket[ON] = clientSocket;
    motionWindowRequest[cameraIndex].request.clientCbType = clientCbType;
    memcpy(&motionWindowRequest[cameraIndex].motionParam, pMotionDataBuf, sizeof(MOTION_BLOCK_METHOD_PARAM_t));
    motionWindowRequest[cameraIndex].configStatus = newConfigStatusF;

    ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);
    if (ipCamCfg.onvifSupportF == TRUE)
    {
        if ((ipCamCfg.brand[0] == '\0') || (ipCamCfg.model[0] == '\0'))
        {
            EPRINT(CAMERA_INTERFACE, "onvif capability not recv, brand-model unknown: [camera=%d]", cameraIndex);
            requestStatus = CMD_ONVIF_CAM_CAPABILITY_ERROR;
        }
        else
        {
            requestStatus = sendOnvifCommonCmd(cameraIndex, ONVIF_SET_MOTION_WINDOW, onvifSetMotionWindowCb,
                                               &ipCamCfg, &motionWindowRequest[cameraIndex].motionParam);
        }
    }
    else if (CameraType(cameraIndex) == AUTO_ADDED_CAMERA)
    {
        motionWindowRequest[cameraIndex].status = FAIL;
        motionWindowRequest[cameraIndex].request.requestCount = 0;
        motionWindowRequest[cameraIndex].request.requestStatus = CMD_PROCESS_ERROR;
        GetCiRequestUrl(cameraIndex, REQ_URL_SET_MOTION_WINDOW, 0, CAM_REQ_CONFIG, tcpSetMotionCnfgCb,
                        &motionWindowRequest[cameraIndex].request, &motionWindowRequest[cameraIndex].motionParam);
        requestStatus =	sendReqToCamera(&motionWindowRequest[cameraIndex].request, &ipCamCfg, MAIN_STREAM);
        if (requestStatus != CMD_SUCCESS)
        {
            EPRINT(CAMERA_INTERFACE, "fail to send motion window url: [camera=%d]", cameraIndex);
        }
    }
    else
    {
        requestStatus = GetCameraBrandModelUrl(cameraIndex, REQ_URL_SET_MOTION_WINDOW, &motionWindowRequest[cameraIndex].request,
                                               &motionWindowRequest[cameraIndex].motionParam, NULL, NULL, NULL);
        if (requestStatus != CMD_SUCCESS)
        {
            EPRINT(CAMERA_INTERFACE, "fail to set motion window url: [camera=%d]", cameraIndex);
        }
        else
        {
            /* Making status fail so that after first request response in callback we can check whether
             * status is HTTP_SUCCESS or HTTP_ERROR if status is HTTP_SUCCESS than fill status and than
             * only you send second request and fill the status again fail so that respective request can be checked */
            motionWindowRequest[cameraIndex].status = FAIL;
            motionWindowRequest[cameraIndex].request.requestCount = 0;
            motionWindowRequest[cameraIndex].request.requestStatus = CMD_PROCESS_ERROR;
            requestStatus =	sendReqToCamera(&motionWindowRequest[cameraIndex].request, &ipCamCfg, MAIN_STREAM);
            if (requestStatus != CMD_SUCCESS)
            {
                EPRINT(CAMERA_INTERFACE, "fail to send motion window url: [camera=%d]", cameraIndex);
            }
        }
    }

    if (requestStatus != CMD_SUCCESS)
    {
        restoreDefaultMotionConfig(cameraIndex);
        MUTEX_LOCK(motionWindowRequest[cameraIndex].request.camReqFlagLock);
        motionWindowRequest[cameraIndex].request.camReqBusyF = FREE;
        MUTEX_UNLOCK(motionWindowRequest[cameraIndex].request.camReqFlagLock);
    }

    return requestStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief SetPrivacyMaskConfig
 * @param cameraIndex
 * @param privacyAreaBuf
 * @param callback
 * @param clientSocket
 * @param privacyMaskStatus
 * @param clientCbType
 * @return
 */
NET_CMD_STATUS_e SetPrivacyMaskConfig(UINT8 cameraIndex, void *privacyAreaBuf, NW_CMD_REPLY_CB callback,
                                      INT32 clientSocket, BOOL privacyMaskStatus, CLIENT_CB_TYPE_e clientCbType)
{
    NET_CMD_STATUS_e    requestStatus = CMD_SUCCESS;
    IP_CAMERA_CONFIG_t  ipCamCfg;

    if (cameraIndex >= getMaxCameraForCurrentVariant())
    {
        EPRINT(CAMERA_INTERFACE, "invld camera index found: [camera=%d]", cameraIndex);
        return CMD_PROCESS_ERROR;
    }

    if (CheckforPrivacyMaskSupport(cameraIndex) != PRIVACY_MASK_METHOD_POINT)
    {
        WPRINT(CAMERA_INTERFACE, "privacy mask not supported: [camera=%d]", cameraIndex);
        return CMD_FEATURE_NOT_SUPPORTED;
    }

    MUTEX_LOCK(privacyMaskRequest[cameraIndex].request.camReqFlagLock);
    if (privacyMaskRequest[cameraIndex].request.camReqBusyF == BUSY)
    {
        MUTEX_UNLOCK(privacyMaskRequest[cameraIndex].request.camReqFlagLock);
        WPRINT(CAMERA_INTERFACE, "set privacy mask request busy: [camera=%d]", cameraIndex);
        return CMD_CAM_REQUEST_IN_PROCESS;
    }

    privacyMaskRequest[cameraIndex].request.camReqBusyF = BUSY;
    MUTEX_UNLOCK(privacyMaskRequest[cameraIndex].request.camReqFlagLock);

    DPRINT(CAMERA_INTERFACE, "set privacy mask request received: [camera=%d]", cameraIndex);
    privacyMaskRequest[cameraIndex].request.clientCb[ON] = callback;
    privacyMaskRequest[cameraIndex].request.clientSocket[ON] = clientSocket;
    privacyMaskRequest[cameraIndex].request.clientCbType = clientCbType;
    ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);

    memcpy(privacyMaskRequest[cameraIndex].privacyArea, privacyAreaBuf, (privacyMaskRequest[cameraIndex].maxSupportedMaskWindow*sizeof(PRIVACY_MASK_CONFIG_t)));
    if (ipCamCfg.onvifSupportF == TRUE)
    {
        if ((ipCamCfg.brand[0] == '\0') || (ipCamCfg.model[0] == '\0'))
        {
            EPRINT(CAMERA_INTERFACE, "onvif capability not recv, brand-model unknown: [camera=%d]", cameraIndex);
            requestStatus = CMD_ONVIF_CAM_CAPABILITY_ERROR;
        }
        else
        {
            requestStatus = CMD_FEATURE_NOT_SUPPORTED;
        }
    }
    else if (CameraType(cameraIndex) == AUTO_ADDED_CAMERA)
    {
        CAMERA_BRAND_e  brand = 0;
        CAMERA_MODEL_e  model = 0;
        GetCiCameraBrandModelNumber(cameraIndex, &brand, &model);

        do
        {
            privacyMaskRequest[cameraIndex].status = IsPrivacyMaskMappingRequired(brand, model);
            if (FAIL == privacyMaskRequest[cameraIndex].status)
            {
                requestStatus = CMD_PROCESS_ERROR;
                break;
            }

            if (FAIL == MapPrivacyMaskNvrToCamera(brand, model, privacyMaskRequest[cameraIndex].privacyArea))
            {
                EPRINT(CAMERA_INTERFACE, "fail to map resolution for privacy mask: [camera=%d]", cameraIndex);
                requestStatus = CMD_PROCESS_ERROR;
                break;
            }

            GetCiRequestUrl(cameraIndex, REQ_URL_SET_PRIVACY_MASK, privacyMaskStatus, CAM_REQ_CONFIG, tcpSetPrivacyMaskCnfgCb,
                            &privacyMaskRequest[cameraIndex].request, privacyMaskRequest[cameraIndex].privacyArea);
            privacyMaskRequest[cameraIndex].status = FAIL;
            privacyMaskRequest[cameraIndex].request.requestCount = 0;
            privacyMaskRequest[cameraIndex].request.requestStatus = CMD_PROCESS_ERROR;
            requestStatus =	sendReqToCamera(&privacyMaskRequest[cameraIndex].request, &ipCamCfg, MAIN_STREAM);
            if(requestStatus != CMD_SUCCESS)
            {
                EPRINT(CAMERA_INTERFACE, "fail to set privacy mask: [camera=%d]", cameraIndex);
            }

        } while(0);
    }
    else
    {
        UINT8   regionCnt = 0;
        CHARPTR pPrivacyMaskName[MAX_PRIVACY_MASKS];

        for (regionCnt = 0; regionCnt < privacyMaskRequest[cameraIndex].maxSupportedMaskWindow; regionCnt++)
        {
            pPrivacyMaskName[regionCnt] = privacyMaskRequest[cameraIndex].privacyMaskName[regionCnt];
        }

        requestStatus = GetCameraBrandModelUrl(cameraIndex, REQ_URL_SET_PRIVACY_MASK, &privacyMaskRequest[cameraIndex].request,
                                               &privacyMaskStatus, privacyMaskRequest[cameraIndex].privacyArea, &pPrivacyMaskName, NULL);
        if (requestStatus != CMD_SUCCESS)
        {
            EPRINT(CAMERA_INTERFACE, "fail to set privacy mask url: [camera=%d]", cameraIndex);
        }
        else
        {
            /* Making status fail so that after first request response in callback we can check whether
             * status is HTTP_SUCCESS or HTTP_ERROR if status is HTTP_SUCCESS than fill status and than
             * only you send second request and fill the status again fail so that respective request can be checked */
            privacyMaskRequest[cameraIndex].status = FAIL;
            privacyMaskRequest[cameraIndex].request.requestCount = 0;
            privacyMaskRequest[cameraIndex].request.requestStatus = CMD_PROCESS_ERROR;
            requestStatus =	sendReqToCamera(&privacyMaskRequest[cameraIndex].request, &ipCamCfg, MAIN_STREAM);
            if (requestStatus != CMD_SUCCESS)
            {
                EPRINT(CAMERA_INTERFACE, "fail to send privacy mask url: [camera=%d]", cameraIndex);
            }
        }
    }

    if (requestStatus != CMD_SUCCESS)
    {
        restoreDefaultPrivacyMaskConfig(cameraIndex);
        MUTEX_LOCK(privacyMaskRequest[cameraIndex].request.camReqFlagLock);
        privacyMaskRequest[cameraIndex].request.camReqBusyF = FREE;
        MUTEX_UNLOCK(privacyMaskRequest[cameraIndex].request.camReqFlagLock);
    }

    return requestStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief SetCurrentSubStreamProfile
 * @param cameraIndex
 */
void SetCurrentSubStreamProfile(UINT8 cameraIndex)
{
    STREAM_CONFIG_t streamCnfg;

    ReadSingleStreamConfig(cameraIndex,&streamCnfg);
    streamCnfg.subStreamProfile = 1;
    streamCnfg.enableAudioSub = streamCnfg.enableAudio;
    streamCnfg.bitrateModeSub = streamCnfg.bitrateMode;
    streamCnfg.gopSub = streamCnfg.gop;
    streamCnfg.qualitySub = streamCnfg.quality;
    streamCnfg.framerateSub = streamCnfg.framerate;
    streamCnfg.bitrateValueSub = streamCnfg.bitrateValue;
    snprintf(streamCnfg.videoEncodingSub, MAX_ENCODER_NAME_LEN, "%s", streamCnfg.videoEncoding);
    snprintf(streamCnfg.resolutionSub, MAX_RESOLUTION_NAME_LEN, "%s", streamCnfg.resolution);

    pthread_rwlock_wrlock(&camConfigInfo[cameraIndex].configInfoLock);
    camConfigInfo[cameraIndex].configChangeNotify = FALSE;
    pthread_rwlock_unlock(&camConfigInfo[cameraIndex].configInfoLock);
    WriteSingleStreamConfig(cameraIndex,&streamCnfg);
    pthread_rwlock_wrlock(&camConfigInfo[cameraIndex].configInfoLock);
    camConfigInfo[cameraIndex].configChangeNotify = TRUE;
    pthread_rwlock_unlock(&camConfigInfo[cameraIndex].configInfoLock);
    DPRINT(CAMERA_INTERFACE, "main stream config copied in sub stream as camera supports only one profile: [camera=%d]", cameraIndex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief StartStream
 * @param cameraIndex
 * @param callback
 * @param clientType
 * @return Network command status
 */
NET_CMD_STATUS_e StartStream(UINT8 cameraIndex, STREAM_REQUEST_CB callback, CI_STREAM_CLIENT_e clientType)
{
    CI_STREAM_EXT_TRG_PARAM_t triggerParam;

    triggerParam.camIndex = cameraIndex;
    triggerParam.type = CI_STREAM_EXT_TRG_START_STREAM;
    triggerParam.clientType = clientType;
    triggerParam.callback = callback;

    return ciStreamExtTriggerHandler(&triggerParam);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief onvifStreamUrlCb
 * @param responseData
 * @return SUCCESS/FAIL
 */
static BOOL onvifStreamUrlCb(ONVIF_RESPONSE_PARA_t *responseData)
{
    UINT32						camIdx;
    VIDEO_TYPE_e				videoType;
    URL_REQUEST_t				*urlReqPtr;
    CI_STREAM_EXT_TRG_PARAM_t   triggerParam;
    IP_CAMERA_CONFIG_t          ipCamCfg;
    NET_CMD_STATUS_e            requestStatus = CMD_SUCCESS;

    triggerParam.camIndex = responseData->cameraIndex;
    triggerParam.type = CI_STREAM_EXT_TRG_CONTROL_CB;
    triggerParam.status = CMD_SUCCESS;

    camIdx = GET_STREAM_INDEX(responseData->cameraIndex);
    videoType = GET_STREAM_TYPE(responseData->cameraIndex);

    if (responseData->response == ONVIF_CMD_SUCCESS)
    {
        DPRINT(CAMERA_INTERFACE, "media url request success: [camera=%d]", responseData->cameraIndex);
        urlReqPtr = (URL_REQUEST_t *)responseData->data;
        memcpy(streamInfo[camIdx][videoType].getStreamRequest.url, urlReqPtr, sizeof(URL_REQUEST_t));

        streamInfo[camIdx][videoType].getStreamRequest.numOfRequest = 1;
        // NOTE: Fixed requestCount ==> when we ++ ==> 0
        streamInfo[camIdx][videoType].getStreamRequest.requestCount = 255;
        ciStreamExtTriggerHandler(&triggerParam);

        ReadSingleIpCameraConfig(camIdx, &ipCamCfg);
        if (ipCamCfg.onvifSupportF == TRUE)
        {
            if ((ipCamCfg.brand[0] == '\0') || (ipCamCfg.model[0] == '\0'))
            {
                EPRINT(CAMERA_INTERFACE, "onvif capability not recv, brand-model unknown: [camera=%d]", responseData->cameraIndex);
                requestStatus = CMD_ONVIF_CAM_CAPABILITY_ERROR;
            }
            else if (videoType == MAIN_STREAM)
            {
                requestStatus = setCameraDateTime(camIdx);
                if ((requestStatus != CMD_SUCCESS) && (requestStatus != CMD_CHANNEL_DISABLED) && (requestStatus != CMD_CAM_DISCONNECTED))
                {
                    startOnvifRetryTimer(camIdx, ONVIF_SET_DATE_TIME);
                }
            }
        }
    }
    else
    {
        memset(streamInfo[camIdx][videoType].getStreamRequest.url, 0, sizeof(URL_REQUEST_t));

        if (responseData->response == ONVIF_CMD_FEATURE_NOT_SUPPORTED)
        {
            EPRINT(CAMERA_INTERFACE, "media url feature not supported: [camera=%d]", responseData->cameraIndex);
            triggerParam.status = CMD_FEATURE_NOT_SUPPORTED;
        }
        else
        {
            EPRINT(CAMERA_INTERFACE, "media url request failed/timeout: [camera=%d]", responseData->cameraIndex);
            triggerParam.status = CMD_ONVIF_CAM_CAPABILITY_ERROR;
            startOnvifStreamUrlRetryTimer(responseData->cameraIndex, ONVIF_GET_MEDIA_URL);
        }
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief InitStreamSession
 * @param cameraIndex
 * @param sessionIndex
 * @param readPos
 * @return SUCCESS/FAIL
 */
BOOL InitStreamSession(UINT8 cameraIndex, UINT8 sessionIndex,CI_BUFFER_READ_POS_e readPos)
{
    if (sessionIndex >= MAX_CI_STREAM_CLIENT)
    {
        return FAIL;
    }

    STREAM_INFO_t *pStreamInfo = &streamInfo[GET_STREAM_INDEX(cameraIndex)][GET_STREAM_TYPE(cameraIndex)];
    pthread_rwlock_rdlock(&pStreamInfo->frameMarker.writeIndexLock);
    INT16 rdIndx = pStreamInfo->frameMarker.wrPos;

    if (sessionIndex == CI_STREAM_CLIENT_RECORD)
    {
        if(readPos == CI_READ_OLDEST_FRAME)
        {
            rdIndx +=5;
        }

        if(rdIndx >= pStreamInfo->frameMarker.maxWriteIndex)
        {
            rdIndx -= pStreamInfo->frameMarker.maxWriteIndex;
        }
    }

    pStreamInfo->frameMarker.rdPos[sessionIndex] = rdIndx;
    pthread_rwlock_unlock(&pStreamInfo->frameMarker.writeIndexLock);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief StopStream
 * @param cameraIndex
 * @param clientType
 */
void StopStream(UINT8 cameraIndex, CI_STREAM_CLIENT_e clientType)
{
    CI_STREAM_EXT_TRG_PARAM_t triggerParam = { 0 };

    triggerParam.camIndex = cameraIndex;
    triggerParam.type = CI_STREAM_EXT_TRG_STOP_STREAM;
    triggerParam.clientType = clientType;

    ciStreamExtTriggerHandler(&triggerParam);
}


//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get camera's current FPS and GOP
 * @param   cameraIndex
 * @param   streamType
 * @param   pFps
 * @param   pGop
 */
void GetCameraFpsGop(UINT8 cameraIndex, UINT8 streamType, UINT8 *pFps, UINT8 *pGop)
{
    if ((cameraIndex >= getMaxCameraForCurrentVariant()) || (streamType >= MAX_STREAM))
    {
        *pGop = *pFps = 0;
        return;
    }

    *pGop = streamInfo[cameraIndex][streamType].gopTotal;
    *pFps = streamInfo[cameraIndex][streamType].fpsTotal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief updateVideoLossStatus
 * @param camIndex
 * @param status
 */
static void updateVideoLossStatus(UINT8 camIndex, BOOL status)
{
    MUTEX_LOCK(videoLossMutex);
    videoLossStatus[camIndex] = status;
    MUTEX_UNLOCK(videoLossMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetCameraStreamStatus
 * @param cameraIndex
 * @param camState
 * @return TRUE/FALSE
 */
BOOL GetCameraStreamStatus(UINT8 cameraIndex, UINT8 camState)
{
    BOOL camStreamState = FALSE;

    if (cameraIndex < getMaxCameraForCurrentVariant())
    {
        MUTEX_LOCK(videoLossMutex);
        camStreamState = videoLossStatus[cameraIndex];
        MUTEX_UNLOCK(videoLossMutex);
    }

    return camStreamState;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief isStreamConfigValid
 * @param streamCfg
 * @param streamType
 * @return TRUE/FALSE
 */
static BOOL isStreamConfigValid(STREAM_CONFIG_t *streamCfg, VIDEO_TYPE_e streamType)
{
    STREAM_CODEC_TYPE_e	codecNo;

    if (streamType == MAIN_STREAM)
    {
        if ((GetVideoCodecNum(streamCfg->videoEncoding, &codecNo) == SUCCESS) && (streamCfg->resolution[0] != '\0') && (streamCfg->framerate > 0))
        {
            return TRUE;
        }
    }
    else
    {
        if ((GetVideoCodecNum(streamCfg->videoEncodingSub, &codecNo) == SUCCESS) && (streamCfg->resolutionSub[0] != '\0') && (streamCfg->framerateSub > 0))
        {
            return TRUE;
        }
    }

    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Verify stream URL is available or not
 * @param   pCameraRequest
 * @return  Returns TRUE if stream URL available else returns FALSE
 */
static BOOL isStreamUrlAvailable(CAMERA_REQUEST_t *pCameraRequest)
{
    UINT8 reqCnt;

    for (reqCnt = 0; reqCnt < pCameraRequest->numOfRequest; reqCnt++)
    {
        if (pCameraRequest->url[reqCnt].requestType == CAM_REQ_MEDIA)
        {
            return TRUE;
        }
    }

    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief sendReqToCamera
 * @param request
 * @param ipCamCfg
 * @param streamType
 * @return Network command status
 */
static NET_CMD_STATUS_e sendReqToCamera(CAMERA_REQUEST_t *request, IP_CAMERA_CONFIG_t *ipCamCfg, VIDEO_TYPE_e streamType)
{
    NET_CMD_STATUS_e 	requestStatus = CMD_SUCCESS;
    CAM_REQUEST_TYPE_e 	requestType = request->url[request->requestCount].requestType;
    UINT16				complexCamIndex = (request->cameraIndex + (streamType * getMaxCameraForCurrentVariant()));

    // Send request as per the protocol type
    switch(request->url[request->requestCount].protocolType)
    {
        case CAM_RTSP_PROTOCOL:
        {
            RtspStreamInfo_t rtspInfo;

            /* Check camera connectivity */
            if (GetCamIpAddress(request->cameraIndex, rtspInfo.ip) == INACTIVE)
            {
                WPRINT(CAMERA_INTERFACE, "camera not reachable for rtsp: [camera=%d]", complexCamIndex);
                return CMD_CAM_DISCONNECTED;
            }

            // If IP is zero, that means it is not resolved, return disconnected
            if ((rtspInfo.ip[0] == '\0') || (ipCamCfg == NULL))
            {
                WPRINT(CAMERA_INTERFACE, "ip not resolved to send rtsp request: [camera=%d]", complexCamIndex);
                return CMD_CAM_DISCONNECTED;
            }

            // If camera is connected, construct full URL
            snprintf(rtspInfo.usrname, MAX_CAMERA_USERNAME_WIDTH, "%s", ipCamCfg->username);
            snprintf(rtspInfo.pswd, MAX_CAMERA_PASSWORD_WIDTH, "%s", ipCamCfg->password);
            rtspInfo.transport = request->url[request->requestCount].rtspTransportType;
            rtspInfo.port = (rtspInfo.transport == HTTP_TUNNELING) ? ipCamCfg->httpPort : ipCamCfg->rtspPort;
            snprintf(rtspInfo.url, MAX_CAMERA_URL_WIDTH, "%s", request->url[request->requestCount].relativeUrl);
            rtspInfo.camIndex = complexCamIndex;
            request->mediaCallback = (RTSP_CALLBACK)rtspCb;

            DPRINT(CAMERA_INTERFACE, "send rtsp request: [camera=%d], [requestCount=%d], [requestType=%d], [url=rtsp://{%s}:%d%s]",
                   complexCamIndex, request->requestCount, request->url[request->requestCount].requestType,
                    rtspInfo.ip, rtspInfo.port, request->url[request->requestCount].relativeUrl);

            // Send stream request to camera over RTSP
            requestStatus = StartRtspStream(&rtspInfo, request->mediaCallback, &request->rtspHandle);
            if (requestStatus == CMD_SUCCESS)
            {
                DPRINT(CAMERA_INTERFACE, "rtsp stream success: [camera=%d], [handle=%d]", complexCamIndex, request->rtspHandle);
            }
            else if (requestStatus == CMD_RESOURCE_LIMIT)
            {
                EPRINT(CAMERA_INTERFACE, "rtsp resource limit reach max to start stream: [camera=%d]", complexCamIndex);
            }
            else
            {
                EPRINT(CAMERA_INTERFACE, "fail to start rtsp stream: [camera=%d]", complexCamIndex);
            }
        }
        break;

        case CAM_HTTP_PROTOCOL:
        {
            HTTP_INFO_t httpInfo;

            httpInfo.userAgent = CURL_USER_AGENT;
            httpInfo.interface = MAX_HTTP_INTERFACE;

            // Get IP, PORT, USERNAME, PASSWORD, and RELATIVE URL [video/audio]
            if (ipCamCfg == NULL)
            {
                GENERAL_CONFIG_t genConfig;

                ReadGeneralConfig(&genConfig);

                //fill http credentials for analog camera
                snprintf(httpInfo.ipAddress, MAX_CAMERA_ADDRESS_WIDTH, "127.0.0.1");
                RESET_STR_BUFF(httpInfo.httpUsrPwd.username);
                RESET_STR_BUFF(httpInfo.httpUsrPwd.password);
                httpInfo.port = genConfig.httpPort;
                httpInfo.authMethod = AUTH_TYPE_ANY;
                httpInfo.maxConnTime = MAX_CONN_TIME;
                httpInfo.maxFrameTime = MAX_CONN_TIME;
                httpInfo.fileForPutReq[0] = '\0';
                httpInfo.sizeOfPutFile = 0;
            }
            else
            {
                //check connectivity
                if (GetCamIpAddress(request->cameraIndex, httpInfo.ipAddress) == INACTIVE)
                {
                    WPRINT(CAMERA_INTERFACE, "camera not reachable for http: [camera=%d]", complexCamIndex);
                    return CMD_CAM_DISCONNECTED;
                }

                // If IP is zero, that means it is not resolved, return disconnected
                if (httpInfo.ipAddress[0] == '\0')
                {
                    WPRINT(CAMERA_INTERFACE, "ip not resolved to send http request: [camera=%d]", complexCamIndex);
                    return CMD_CAM_DISCONNECTED;
                }

                snprintf(httpInfo.httpUsrPwd.username, MAX_CAMERA_USERNAME_WIDTH, "%s", ipCamCfg->username);
                snprintf(httpInfo.httpUsrPwd.password, MAX_CAMERA_PASSWORD_WIDTH, "%s", ipCamCfg->password);
                httpInfo.port = (ipCamCfg->onvifSupportF == TRUE) ? ipCamCfg->onvifPort : ipCamCfg->httpPort;
                httpInfo.authMethod = request->url[request->requestCount].authMethod;
                if (request->url[request->requestCount].httpRequestType == PUT_REQUEST)
                {
                    snprintf(httpInfo.fileForPutReq, MAX_FILE_NAME_LENGTH, "%s", request->url[request->requestCount].fileForPutReq);
                    httpInfo.sizeOfPutFile = request->url[request->requestCount].sizeOfPutFile;
                    httpInfo.contentType = request->url[request->requestCount].httpContentType;
                }
                else
                {
                    httpInfo.fileForPutReq[0] = '\0';
                    httpInfo.sizeOfPutFile = 0;
                }

                httpInfo.maxConnTime = HTTP_CONNECTION_TIMEOUT;
                httpInfo.maxFrameTime = HTTP_FRAME_TIMEOUT;
            }

            //retrieve relative url
            snprintf(httpInfo.relativeUrl, MAX_RELATIVE_URL_WIDTH, "%s", request->url[request->requestCount].relativeUrl);

            DPRINT(CAMERA_INTERFACE, "send http request: [camera=%d], [requestCount=%d], [requestType=%d], [url=http://%s:%d%s]",
                   complexCamIndex, request->requestCount, request->url[request->requestCount].requestType,
                    httpInfo.ipAddress, httpInfo.port, request->url[request->requestCount].relativeUrl);

            // Send request
            if(StartHttp(request->url[request->requestCount].httpRequestType, &httpInfo,
                         request->httpCallback[requestType], complexCamIndex, &request->http[request->requestCount]) == FAIL)
            {
                requestStatus = CMD_RESOURCE_LIMIT;
                EPRINT(CAMERA_INTERFACE, "http resource limit reach max to start stream: [camera=%d]", complexCamIndex);
            }
            else
            {
                DPRINT(CAMERA_INTERFACE, "http request success: [camera=%d], [requestCount=%d], [handle=%d]",
                       complexCamIndex, request->requestCount, request->http[request->requestCount]);
            }
        }
        break;

        case CAM_TCP_PROTOCOL:
        {
            TCP_REQ_INFO_t  tcpInfo;
            UINT8           profileNum = 0;

            getStreamProfileNum(streamType, request->cameraIndex, &profileNum);
            profileNum = (profileNum > 0) ? (profileNum - 1) : MAIN_STREAM;

            tcpInfo.camIndex = complexCamIndex;
            tcpInfo.tcpRequest = request->url[request->requestCount].tcpRequestType;
            snprintf(tcpInfo.commandString, MAX_CAMERA_URI_WIDTH, "%s", request->url[request->requestCount].relativeUrl);
            request->rtspHandle = 0;

            DPRINT(CAMERA_INTERFACE, "send tcp request: [camera=%d], [requestCount=%d], [requestType=%d], [commandString=%s]",
                   complexCamIndex, request->requestCount, request->url[request->requestCount].requestType, tcpInfo.commandString);

            requestStatus = StartTcp(&tcpInfo, request->tcpCallback[request->url[request->requestCount].requestType],
                    &request->http[request->requestCount], &profileNum);
        }
        break;

        default:
        {
            EPRINT(CAMERA_INTERFACE, "invld request protocol type: [camera=%d], [protocolType=%d]",
                   complexCamIndex, request->url[request->requestCount].protocolType);
            requestStatus = CMD_PROCESS_ERROR;
        }
        break;
    }
    return requestStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief restoreDfltCamIpChangeReqParam
 * @param cameraIndex
 */
static void restoreDfltCamIpChangeReqParam(UINT8 cameraIndex)
{
    UINT8 requestCount;

    for(requestCount = 0; requestCount < MAX_URL_REQUEST; requestCount++)
    {
        changeCamIpRequest[cameraIndex].request.http[requestCount] = INVALID_HTTP_HANDLE;
    }

    changeCamIpRequest[cameraIndex].request.rtspHandle = INVALID_RTSP_HANDLE;
    changeCamIpRequest[cameraIndex].request.numOfRequest = 0;
    memset(changeCamIpRequest[cameraIndex].request.clientCb, '\0', sizeof(changeCamIpRequest[cameraIndex].request.clientCb));
    changeCamIpRequest[cameraIndex].newNetworkParam.ipAddrType = IP_ADDR_TYPE_MAX;
    memset(changeCamIpRequest[cameraIndex].newNetworkParam.ipAddr, '\0', sizeof(changeCamIpRequest[cameraIndex].newNetworkParam.ipAddr));
    changeCamIpRequest[cameraIndex].newNetworkParam.prefixLen = 0;
    memset(changeCamIpRequest[cameraIndex].newNetworkParam.gateway, '\0', sizeof(changeCamIpRequest[cameraIndex].newNetworkParam.gateway));
    changeCamIpRequest[cameraIndex].status = SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief httpChangeCamIpAddrCb
 * @param httpHandle
 * @param dataInfo
 */
static void httpChangeCamIpAddrCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo)
{
    UINT8 						requestCount;
    UINT8						cameraIndex = (UINT8)(GET_STREAM_INDEX(dataInfo->userData));
    VIDEO_TYPE_e 				streamType = (UINT8)(GET_STREAM_TYPE(dataInfo->userData));
    CHANGE_CAM_IP_ADDR_t		*camIpChangeRequest = &changeCamIpRequest[cameraIndex];
    IP_CAMERA_CONFIG_t			ipCameraCfg;
    NET_CMD_STATUS_e 			clientResponse = CMD_SUCCESS;
    BOOL						freeSession = FALSE;

    switch(dataInfo->httpResponse)
    {
        // NOTE: here We Have Ignored HTTP_SUCCESS & HTTP_ERROR, because we want to continue although failure of parameter setting
        case HTTP_ERROR:
        {
            camIpChangeRequest->status = FAIL;
        }
        break;

        case HTTP_SUCCESS:
        {
            camIpChangeRequest->status = SUCCESS;
        }
        break;

        case HTTP_CLOSE_ON_SUCCESS:
        {
            if(camIpChangeRequest->status == SUCCESS)
            {
                for (requestCount = 0; requestCount < camIpChangeRequest->request.numOfRequest; requestCount++)
                {
                    if (camIpChangeRequest->request.http[requestCount] == httpHandle)
                    {
                        requestCount++;
                        break;
                    }
                }

                if (requestCount < camIpChangeRequest->request.numOfRequest)
                {
                    camIpChangeRequest->request.http[(requestCount-1)] = INVALID_HTTP_HANDLE;
                    camIpChangeRequest->request.requestCount++;

                    if ((camIpChangeRequest->request.requestCount < camIpChangeRequest->request.numOfRequest)
                         && ((camIpChangeRequest->request.url[camIpChangeRequest->request.requestCount].requestType == CAM_REQ_CONTROL)
                             || (camIpChangeRequest->request.url[camIpChangeRequest->request.requestCount].requestType == CAM_REQ_REBOOT)))
                    {
                        ReadSingleIpCameraConfig(cameraIndex, &ipCameraCfg);
                        clientResponse = sendReqToCamera(&camIpChangeRequest->request, &ipCameraCfg, streamType);
                    }

                    if (clientResponse != CMD_SUCCESS)
                    {
                        clientResponse = CMD_IP_CHANGE_FAIL;
                        freeSession = TRUE;
                    }
                }
                else
                {
                    ReadSingleIpCameraConfig(cameraIndex, &ipCameraCfg);
                    snprintf(ipCameraCfg.cameraAddress, MAX_CAMERA_ADDRESS_WIDTH, "%s", camIpChangeRequest->newNetworkParam.ipAddr);

                    /* We dont have to notify for the changed written in the ipCamConfig because it's a same camera with same brand and model..
                     * just stream would be in video loss state for two to three minutes */
                    pthread_rwlock_wrlock(&camConfigInfo[cameraIndex].configInfoLock);
                    camConfigInfo[cameraIndex].configChangeNotify = FALSE;
                    pthread_rwlock_unlock(&camConfigInfo[cameraIndex].configInfoLock);
                    WriteSingleIpCameraConfig(cameraIndex,&ipCameraCfg);
                    pthread_rwlock_wrlock(&camConfigInfo[cameraIndex].configInfoLock);
                    camConfigInfo[cameraIndex].configChangeNotify = TRUE;
                    pthread_rwlock_unlock(&camConfigInfo[cameraIndex].configInfoLock);
                    DPRINT(CAMERA_INTERFACE, "ip addr changed successfully: [camera=%d]", cameraIndex);

                    clientResponse = CMD_SUCCESS;
                    freeSession = TRUE;
                }
            }
            else
            {
                clientResponse = CMD_IP_CHANGE_FAIL;
                freeSession = TRUE;
            }
        }
        break;

        case HTTP_CLOSE_ON_ERROR:
        {
            clientResponse = CMD_IP_CHANGE_FAIL;
            freeSession = TRUE;
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }

    if (freeSession == TRUE)
    {
        sendChangeCamIpAddrResponseToClient(cameraIndex, clientResponse);
        restoreDfltCamIpChangeReqParam(cameraIndex);

        MUTEX_LOCK(camIpChangeRequest->request.camReqFlagLock);
        changeCamIpRequest[cameraIndex].request.camReqBusyF = FREE;
        MUTEX_UNLOCK(camIpChangeRequest->request.camReqFlagLock);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief httpRebootIpCamCb
 * @param httpHandle
 * @param dataInfo
 */
static void httpRebootIpCamCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo)
{
    UINT8                   cameraIndex = (UINT8)(GET_STREAM_INDEX(dataInfo->userData));
    CHANGE_CAM_IP_ADDR_t    *camIpChangeRequest = &changeCamIpRequest[cameraIndex];
    IP_CAMERA_CONFIG_t      ipCameraCfg;

    switch(dataInfo->httpResponse)
    {
        /* NOTE: here We Have Ignored HTTP_SUCCESS & HTTP_ERROR, because we want to continue although failure of parameter setting */
        case HTTP_ERROR:
        {
            camIpChangeRequest->status = FAIL;
        }
        break;

        case HTTP_SUCCESS:
        {
            camIpChangeRequest->status = SUCCESS;
        }
        break;

        case HTTP_CLOSE_ON_SUCCESS:
        case HTTP_CLOSE_ON_ERROR:
        {
            ReadSingleIpCameraConfig(cameraIndex, &ipCameraCfg);
            snprintf(ipCameraCfg.cameraAddress, MAX_CAMERA_ADDRESS_WIDTH, "%s", camIpChangeRequest->newNetworkParam.ipAddr);

            pthread_rwlock_wrlock(&camConfigInfo[cameraIndex].configInfoLock);
            camConfigInfo[cameraIndex].configChangeNotify = FALSE;
            pthread_rwlock_unlock(&camConfigInfo[cameraIndex].configInfoLock);
            WriteSingleIpCameraConfig(cameraIndex,&ipCameraCfg);
            pthread_rwlock_wrlock(&camConfigInfo[cameraIndex].configInfoLock);
            camConfigInfo[cameraIndex].configChangeNotify = TRUE;
            pthread_rwlock_unlock(&camConfigInfo[cameraIndex].configInfoLock);
            DPRINT(CAMERA_INTERFACE, "reboot ip camera successfully: [camera=%d]", cameraIndex);

            if (camIpChangeRequest->status == SUCCESS)
            {
                sendChangeCamIpAddrResponseToClient(cameraIndex,CMD_SUCCESS);
                restoreDfltCamIpChangeReqParam(cameraIndex);
            }
            else
            {
                sendChangeCamIpAddrResponseToClient(cameraIndex,CMD_REBOOT_FAIL);
                restoreDfltCamIpChangeReqParam(cameraIndex);
            }

            MUTEX_LOCK(camIpChangeRequest->request.camReqFlagLock);
            camIpChangeRequest->request.camReqBusyF = FREE;
            MUTEX_UNLOCK(camIpChangeRequest->request.camReqFlagLock);
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
 * @brief httpStreamControlCb
 * @param httpHandle
 * @param dataInfo
 */
static void httpStreamControlCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo)
{
    UINT8 						requestCount;
    CI_STREAM_EXT_TRG_PARAM_t	triggerParam;
    STREAM_INFO_t				*pStreamInfo;

    switch(dataInfo->httpResponse)
    {
        /* NOTE: here We Have Ignored HTTP_SUCCESS & HTTP_ERROR, because we want to continue stream though failure of parameter setting */
        case HTTP_SUCCESS:
        {
            /* Nothing to do */
        }
        break;

        case HTTP_CLOSE_ON_SUCCESS:
        case HTTP_CLOSE_ON_ERROR:
        {
            triggerParam.camIndex = dataInfo->userData;
            pStreamInfo = &streamInfo[GET_STREAM_INDEX(triggerParam.camIndex)][GET_STREAM_TYPE(triggerParam.camIndex)];

            for (requestCount = 0; requestCount < pStreamInfo->getStreamRequest.numOfRequest; requestCount++)
            {
                if (pStreamInfo->getStreamRequest.http[requestCount] == httpHandle)
                {
                    break;
                }
            }

            if (requestCount < pStreamInfo->getStreamRequest.numOfRequest)
            {
                DPRINT(CAMERA_INTERFACE, "http stream control resp: [camera=%d], [requestCount=%d], [httpResponse=%s]", triggerParam.camIndex,
                       requestCount, httpRespStr[dataInfo->httpResponse]);

                pStreamInfo->getStreamRequest.http[requestCount] = INVALID_HTTP_HANDLE;
                triggerParam.status = CMD_SUCCESS;
                triggerParam.type = CI_STREAM_EXT_TRG_CONTROL_CB;
                ciStreamExtTriggerHandler(&triggerParam);
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
 * @brief httpGetMotionRequestCb
 * @param httpHandle
 * @param dataInfo
 */
static void httpGetMotionRequestCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo)
{
    NET_CMD_STATUS_e                    retVal = CMD_PROCESS_ERROR;
    UINT8                               cameraIndex = 0;
    CAMERA_BRAND_e                      brand;
    CAMERA_MODEL_e                      model;
    PARSER_GET_MOTION_WINDOW_RESPONSE   *parseGetMotionWindowResponse;

    if (NULL == dataInfo)
    {
        EPRINT(CAMERA_INTERFACE, "http get motion request response data null");
        return;
    }

    cameraIndex = (UINT8)(GET_STREAM_INDEX(dataInfo->userData));
    GetCameraBrandModelNumber(cameraIndex,&brand,&model);

    switch(dataInfo->httpResponse)
    {
        case HTTP_ERROR:
        {
            motionWindowRequest[cameraIndex].status = FAIL;
        }
        break;

        /* NOTE: here We Have Ignored HTTP_SUCCESS & HTTP_ERROR, because we want to continue stream though failure of parameter setting */
        case HTTP_SUCCESS:
        {
            if (CheckforVideoAnalyticsSupport(cameraIndex) != MOTION_AREA_METHOD_BLOCK)
            {
                EPRINT(CAMERA_INTERFACE, "motion analytics method not supported or unknown: [camera=%d]", cameraIndex);
                motionWindowRequest[cameraIndex].status = FAIL;
                break;
            }

            if ((dataInfo->frameSize == 0) || (dataInfo->storagePtr == NULL))
            {
                motionWindowRequest[cameraIndex].status = FAIL;
                break;
            }

            parseGetMotionWindowResponse = ParseGetMotionWindowResponseFunc(brand);
            if (parseGetMotionWindowResponse == NULL)
            {
                motionWindowRequest[cameraIndex].status = SUCCESS;
                break;
            }

            if ((*parseGetMotionWindowResponse)(model, dataInfo->frameSize, dataInfo->storagePtr, &motionWindowRequest[cameraIndex].motionParam) != CMD_SUCCESS)
            {
                EPRINT(CAMERA_INTERFACE, "fail to parse response: [camera=%d]", cameraIndex);
                motionWindowRequest[cameraIndex].status = FAIL;
            }
            else
            {
                motionWindowRequest[cameraIndex].motionParam.noMotionSupportF = IsNoMotionEventSupported(cameraIndex);
                motionWindowRequest[cameraIndex].status = SUCCESS;
            }
        }
        break;

        case HTTP_CLOSE_ON_SUCCESS:
        case HTTP_CLOSE_ON_ERROR:
        {
            motionWindowRequest[cameraIndex].request.http[0] = INVALID_HTTP_HANDLE;
            if (motionWindowRequest[cameraIndex].status == SUCCESS)
            {
                if (CheckforVideoAnalyticsSupport(cameraIndex) != MOTION_AREA_METHOD_BLOCK)
                {
                    motionWindowRequest[cameraIndex].status = FAIL;
                    retVal = CMD_PROCESS_ERROR;
                }
            }

            if(motionWindowRequest[cameraIndex].status == FAIL)
            {
                //NOTE: here this status code is used to display the Message UNABLE TO GET PARAMETERS
                retVal = CMD_ONVIF_CAM_CAPABILITY_ERROR;
                memset(&motionWindowRequest[cameraIndex].motionParam, 0, sizeof(motionWindowRequest[cameraIndex].motionParam));
            }
            else
            {
                retVal = CMD_SUCCESS;
            }


            sendGetMotionWindowResponseToClient(cameraIndex, retVal);
            restoreDefaultMotionConfig(cameraIndex);

            MUTEX_LOCK(motionWindowRequest[cameraIndex].request.camReqFlagLock);
            motionWindowRequest[cameraIndex].request.camReqBusyF = FREE;
            MUTEX_UNLOCK(motionWindowRequest[cameraIndex].request.camReqFlagLock);
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
 * @brief httpGetPrivacyMaskWindowRequestCb
 * @param httpHandle
 * @param dataInfo
 */
static void httpGetPrivacyMaskWindowRequestCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo)
{
    UINT8                       cameraIndex;
    CAMERA_BRAND_e              brand = MAX_CAMERA_BRAND;
    CAMERA_MODEL_e              model;
    PARSER_PRIVACYMSAK_RESPONSE *parsePrivacyMaskResponse;
    CHARPTR                     pPrivacyMaskName[MAX_PRIVACY_MASKS];
    UINT8                       regionCnt = 0;
    UINT8                       maxSupportedPrivacyMaskWin;

    if (NULL == dataInfo)
    {
        EPRINT(CAMERA_INTERFACE, "http get privacy mask window request response data null");
        return;
    }

    cameraIndex = (UINT8)(GET_STREAM_INDEX(dataInfo->userData));
    GetCameraBrandModelNumber(cameraIndex, &brand, &model);
    maxSupportedPrivacyMaskWin = privacyMaskRequest[cameraIndex].maxSupportedMaskWindow;

    switch(dataInfo->httpResponse)
    {
        case HTTP_ERROR:
        {
            privacyMaskRequest[cameraIndex].status = FAIL;
        }
        break;

        /* NOTE: here We Have Ignored HTTP_SUCCESS & HTTP_ERROR, because we want to continue stream though failure of parameter setting */
        case HTTP_SUCCESS:
        {
            if (CheckforPrivacyMaskSupport(cameraIndex) != PRIVACY_MASK_METHOD_POINT)
            {
                EPRINT(CAMERA_INTERFACE, "motion analytics method not supported or unknown: [camera=%d]", cameraIndex);
                privacyMaskRequest[cameraIndex].status = FAIL;
                break;
            }

            if ((dataInfo->frameSize == 0) || (dataInfo->storagePtr == NULL))
            {
                privacyMaskRequest[cameraIndex].status = FAIL;
                break;
            }

            parsePrivacyMaskResponse = ParsePrivacyMaskRersponseFunc(brand);
            if (parsePrivacyMaskResponse == NULL)
            {
                privacyMaskRequest[cameraIndex].status = FAIL;
                break;
            }

            for(regionCnt = 0; regionCnt < maxSupportedPrivacyMaskWin; regionCnt++)
            {
                pPrivacyMaskName[regionCnt] = privacyMaskRequest[cameraIndex].privacyMaskName[regionCnt];
            }

            if ((*parsePrivacyMaskResponse)(model, dataInfo->frameSize, dataInfo->storagePtr,
                                            privacyMaskRequest[cameraIndex].privacyArea, pPrivacyMaskName) != CMD_SUCCESS)
            {
                EPRINT(CAMERA_INTERFACE, "fail to parse response: [camera=%d]", cameraIndex);
                privacyMaskRequest[cameraIndex].status = FAIL;
            }
            else
            {
                privacyMaskRequest[cameraIndex].status = SUCCESS;
            }
        }
        break;


        case HTTP_CLOSE_ON_SUCCESS:
        case HTTP_CLOSE_ON_ERROR:
        {
            privacyMaskRequest[cameraIndex].request.http[0] = INVALID_HTTP_HANDLE;
            if (privacyMaskRequest[cameraIndex].status == SUCCESS)
            {
                if (CheckforPrivacyMaskSupport(cameraIndex) != PRIVACY_MASK_METHOD_POINT)
                {
                    privacyMaskRequest[cameraIndex].status = FAIL;
                }
                else
                {
                    if (IsPrivacyMaskMappingRequired(brand, model) == TRUE)
                    {
                        if (FAIL == MapPrivacyMaskCameraToNvr(brand, model, privacyMaskRequest[cameraIndex].privacyArea))
                        {
                            EPRINT(CAMERA_INTERFACE, "fail to map privacy mask: [camera=%d], [brand=%d], [model=%d]", cameraIndex, brand, model);
                            privacyMaskRequest[cameraIndex].status = FAIL;
                        }
                    }
                }
            }

            sendGetPrivacyMaskResponseToClient(cameraIndex, (privacyMaskRequest[cameraIndex].status == FAIL) ? CMD_ONVIF_CAM_CAPABILITY_ERROR : CMD_SUCCESS);
            restoreDefaultPrivacyMaskConfig(cameraIndex);

            MUTEX_LOCK(privacyMaskRequest[cameraIndex].request.camReqFlagLock);
            privacyMaskRequest[cameraIndex].request.camReqBusyF = FREE;
            MUTEX_UNLOCK(privacyMaskRequest[cameraIndex].request.camReqFlagLock);
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
 * @brief   sendGetMotionWindowResponseToClient
 * @param   cameraIndex
 * @param   requestStatus
 */
static void sendGetMotionWindowResponseToClient(UINT8 cameraIndex, NET_CMD_STATUS_e requestStatus)
{
    UINT32              outLen;
    CHAR                respStringPtr[MAX_REPLY_SZ];
    UINT8               blockByte = 0;
    CLIENT_CB_TYPE_e    clientCbType = motionWindowRequest[cameraIndex].request.clientCbType;

    outLen = snprintf(respStringPtr, MAX_REPLY_SZ, "%c%s%c%03d%c%c%d%c%d%c%d%c%d%c%d%c%d%c",
                      SOM, headerReq[RPL_CMD], FSP, requestStatus, FSP, SOI, 1, FSP, MOTION_AREA_METHOD_BLOCK, FSP,
                      motionWindowRequest[cameraIndex].motionParam.sensitivity, FSP,
                      motionWindowRequest[cameraIndex].motionParam.noMotionSupportF, FSP,
                      motionWindowRequest[cameraIndex].motionParam.isNoMotionEvent, FSP,
                      motionWindowRequest[cameraIndex].motionParam.noMotionDuration, FSP);
    for(blockByte = 0; blockByte < MOTION_AREA_BLOCK_BYTES; blockByte++)
    {
        snprintf(&respStringPtr[outLen + (blockByte*2)], MAX_REPLY_SZ - (outLen + (blockByte*2)), "%02x",
                (UINT8)motionWindowRequest[cameraIndex].motionParam.blockBitString[blockByte]);
    }

    outLen += (blockByte * 2);
    if(outLen > MAX_REPLY_SZ - 4)
    {
        EPRINT(CAMERA_INTERFACE, "more buffer required: [camera=%d], [outLen=%d]", cameraIndex, outLen);
        outLen = MAX_REPLY_SZ - 4;
    }

    outLen += snprintf((respStringPtr + outLen), sizeof(respStringPtr) - outLen, "%c%c%c", FSP, EOI, EOM);
    if(outLen > MAX_REPLY_SZ)
    {
        EPRINT(CAMERA_INTERFACE, "more buffer required: [camera=%d], [outLen=%d]", cameraIndex, outLen);
        outLen = MAX_REPLY_SZ;
    }

    sendCmdCb[clientCbType](motionWindowRequest[cameraIndex].request.clientSocket[ON], (UINT8PTR)respStringPtr, outLen, MESSAGE_REPLY_TIMEOUT);
    closeConnCb[clientCbType](&motionWindowRequest[cameraIndex].request.clientSocket[ON]);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   sendGetPrivacyMaskResponseToClient
 * @param   cameraIndex
 * @param   requestStatus
 */
static void sendGetPrivacyMaskResponseToClient(UINT8 cameraIndex, NET_CMD_STATUS_e requestStatus)
{
    UINT32  outLen;
    CHAR    respStringPtr[MAX_REPLY_SZ];
    UINT8   index;

    outLen = snprintf(respStringPtr, MAX_REPLY_SZ, "%c%s%c%d%c%c%d%c%d",
                      SOM, headerReq[RPL_CMD], FSP, requestStatus, FSP, SOI, 1, FSP, privacyMaskRequest[cameraIndex].maxSupportedMaskWindow);
    if (outLen > MAX_REPLY_SZ)
    {
        EPRINT(CAMERA_INTERFACE, "more buffer required: [camera=%d], [outLen=%d]", cameraIndex, outLen);
        outLen = MAX_REPLY_SZ;
    }

    for(index = 0; index < privacyMaskRequest[cameraIndex].maxSupportedMaskWindow; index++)
    {
        outLen += snprintf(respStringPtr + outLen, MAX_REPLY_SZ - outLen, "%c%d%c%d%c%d%c%d",
                           FSP,privacyMaskRequest[cameraIndex].privacyArea[index].startXPoint,
                           FSP,privacyMaskRequest[cameraIndex].privacyArea[index].startYPoint,
                           FSP,privacyMaskRequest[cameraIndex].privacyArea[index].width,
                           FSP,privacyMaskRequest[cameraIndex].privacyArea[index].height);
        if (outLen > MAX_REPLY_SZ - 4)
        {
            EPRINT(CAMERA_INTERFACE, "more buffer required: [camera=%d], [outLen=%d]", cameraIndex, outLen);
            outLen = MAX_REPLY_SZ - 4;
            break;
        }
    }

    outLen += snprintf(respStringPtr + outLen, sizeof(respStringPtr) - outLen, "%c%c%c", FSP, EOI, EOM);
    if(outLen > MAX_REPLY_SZ)
    {
        EPRINT(CAMERA_INTERFACE, "more buffer required: [camera=%d], [outLen=%d]", cameraIndex, outLen);
        outLen = MAX_REPLY_SZ;
    }

    CLIENT_CB_TYPE_e clientCbType = privacyMaskRequest[cameraIndex].request.clientCbType;
    sendCmdCb[clientCbType](privacyMaskRequest[cameraIndex].request.clientSocket[ON], (UINT8PTR)respStringPtr, outLen, MESSAGE_REPLY_TIMEOUT);
    closeConnCb[clientCbType](&privacyMaskRequest[cameraIndex].request.clientSocket[ON]);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief restoreDefaultMotionConfig
 * @param cameraIndex
 */
static void restoreDefaultMotionConfig(UINT8 cameraIndex)
{
    UINT8 requestCount;

    for(requestCount = 0; requestCount < MAX_URL_REQUEST; requestCount++)
    {
        motionWindowRequest[cameraIndex].request.http[requestCount] = INVALID_HTTP_HANDLE;
    }

    motionWindowRequest[cameraIndex].request.rtspHandle = INVALID_RTSP_HANDLE;
    motionWindowRequest[cameraIndex].request.numOfRequest = 0;
    memset(motionWindowRequest[cameraIndex].request.clientCb, '\0', sizeof(motionWindowRequest[cameraIndex].request.clientCb));
    motionWindowRequest[cameraIndex].status = FAIL;
    motionWindowRequest[cameraIndex].configStatus = DISABLE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief restoreDefaultPrivacyMaskConfig
 * @param cameraIndex
 */
static void restoreDefaultPrivacyMaskConfig(UINT8 cameraIndex)
{
    UINT8 requestCount;

    for (requestCount = 0; requestCount < MAX_URL_REQUEST; requestCount++)
    {
        privacyMaskRequest[cameraIndex].request.http[requestCount] = INVALID_HTTP_HANDLE;
    }

    privacyMaskRequest[cameraIndex].request.rtspHandle = INVALID_RTSP_HANDLE;
    privacyMaskRequest[cameraIndex].request.numOfRequest = 0;
    memset(privacyMaskRequest[cameraIndex].request.clientCb, '\0', sizeof(privacyMaskRequest[cameraIndex].request.clientCb));
    privacyMaskRequest[cameraIndex].status = FAIL;

    for(requestCount = 0; requestCount < MAX_PRIVACY_MASKS; requestCount++)
    {
        privacyMaskRequest[cameraIndex].privacyArea[requestCount].startXPoint = 0;
        privacyMaskRequest[cameraIndex].privacyArea[requestCount].startYPoint = 0;
        privacyMaskRequest[cameraIndex].privacyArea[requestCount].width = 0;
        privacyMaskRequest[cameraIndex].privacyArea[requestCount].height = 0;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief tcpMediaCb
 * @param tcpFrame
 */
static void tcpMediaCb(TCP_FRAME_INFO_t *tcpFrame)
{
    CI_STREAM_EXT_TRG_PARAM_t triggerParam;

    triggerParam.camIndex = tcpFrame->channel;
    triggerParam.type = CI_STREAM_EXT_TRG_MEDIA_CB;

    if (START == tcpFrame->state)
    {
        triggerParam.mediaResp = CI_STREAM_INT_RESP_FRAME;
        triggerParam.bufferPtr = tcpFrame->dataPtr;
        triggerParam.mediaFrame = &(tcpFrame->frameInfo);
        triggerParam.streamType = tcpFrame->streamType;
    }
    else
    {
        triggerParam.mediaResp = CI_STREAM_INT_RESP_CLOSE;
        EPRINT(CAMERA_INTERFACE, "stream request response: [camera=%d], [state=STOP]", triggerParam.camIndex);
    }

    ciStreamExtTriggerHandler(&triggerParam);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief httpStreamMediaCb
 * @param httpHandle
 * @param dataInfo
 */
static void httpStreamMediaCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo)
{
    UINT8 						requestCount;
    CI_STREAM_EXT_TRG_PARAM_t	triggerParam;
    STREAM_INFO_t				*pStreamInfo;

    triggerParam.camIndex = dataInfo->userData;
    triggerParam.type = CI_STREAM_EXT_TRG_MEDIA_CB;

    switch (dataInfo->httpResponse)
    {
        case HTTP_SUCCESS:
        {
            if (dataInfo->streamType == VIDEO_STREAM)
            {
                triggerParam.streamType = STREAM_TYPE_VIDEO;
            }
            else
            {
                triggerParam.streamType = STREAM_TYPE_AUDIO;
            }

            triggerParam.mediaResp = CI_STREAM_INT_RESP_FRAME;
            triggerParam.bufferPtr = dataInfo->storagePtr;
            triggerParam.mediaFrame = &dataInfo->mediaFrame;
        }
        break;

        case HTTP_CLOSE_ON_SUCCESS:
        case HTTP_CLOSE_ON_ERROR:
        {
            pStreamInfo = &streamInfo[GET_STREAM_INDEX(triggerParam.camIndex)][GET_STREAM_TYPE(triggerParam.camIndex)];
            for (requestCount = 0; requestCount < pStreamInfo->getStreamRequest.numOfRequest; requestCount++)
            {
                if (pStreamInfo->getStreamRequest.http[requestCount] == httpHandle)
                {
                    break;
                }
            }

            if (requestCount >= pStreamInfo->getStreamRequest.numOfRequest)
            {
                DPRINT(CAMERA_INTERFACE, "request not found to trigger ci: [camera=%d], [requestCount=%d], [numOfRequest=%d]",
                       triggerParam.camIndex, requestCount, pStreamInfo->getStreamRequest.numOfRequest);
                return;
            }

            DPRINT(CAMERA_INTERFACE, "http stream callback: [camera=%d], [requestCount=%d], [httpResponse=%s]",
                   triggerParam.camIndex, requestCount, httpRespStr[dataInfo->httpResponse]);

            // HTTP is clodsed successfully, reset the handle
            pStreamInfo->getStreamRequest.http[requestCount] = INVALID_HTTP_HANDLE;

            /* Check if Any Other Media Request is Alive, If exists don't send close trigger
             * NOTE: as the Handle variable is not synchronised, this logic may not work in all case,
             * but in worst case two close trigger will be given */
            for (requestCount = 0; requestCount < pStreamInfo->getStreamRequest.numOfRequest; requestCount++)
            {
                if (pStreamInfo->getStreamRequest.http[requestCount] != INVALID_HTTP_HANDLE)
                {
                    StopHttp(pStreamInfo->getStreamRequest.http[requestCount]);
                    DPRINT(CAMERA_INTERFACE, "wait for other media request to be closed: [camera=%d]", triggerParam.camIndex);
                    return;
                }
            }

            triggerParam.mediaResp = CI_STREAM_INT_RESP_CLOSE;
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        return;
    }

    ciStreamExtTriggerHandler(&triggerParam);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief rtspCb
 * @param mediaResp
 * @param rtspData
 * @param frameInfo
 * @param cameraIndex
 */
static void rtspCb(MediaFrameResp_e mediaResp, UINT8PTR rtspData, MEDIA_FRAME_INFO_t * frameInfo, UINT8 cameraIndex)
{
    CI_STREAM_EXT_TRG_PARAM_t	triggerParam;

    triggerParam.camIndex = cameraIndex;
    triggerParam.type = CI_STREAM_EXT_TRG_MEDIA_CB;

    switch(mediaResp)
    {
        default:
        {
            EPRINT(CAMERA_INTERFACE, "rtsp returned with invalid response: [camera=%d], [mediaResp=%d]", cameraIndex, mediaResp);
        }
        return;

        // IF response is video configuration data
        case RTSP_RESP_CODE_CONFIG_VIDEO_DATA:
        case RTSP_RESP_CODE_CONFIG_AUDIO_DATA:
        case RTSP_RESP_CODE_VIDEO_DATA:
        case RTSP_RESP_CODE_AUDIO_DATA:
        {
            if((mediaResp == RTSP_RESP_CODE_VIDEO_DATA) || (mediaResp == RTSP_RESP_CODE_CONFIG_VIDEO_DATA))
            {
                triggerParam.streamType = STREAM_TYPE_VIDEO;
            }
            else
            {
                triggerParam.streamType = STREAM_TYPE_AUDIO;
            }
            triggerParam.mediaResp = CI_STREAM_INT_RESP_FRAME;
            triggerParam.bufferPtr = rtspData;
            triggerParam.mediaFrame = frameInfo;
        }
        break;

        /* NOTE: In New Code RTSP_RESP_CODE_CONNECT_FAIL, RTSP_RESP_CODE_FRAME_TIMEOUT,
         * RTSP_RESP_CODE_CONN_CLOSE all response are considered same, which was handled differently */
        case RTSP_RESP_CODE_CONNECT_FAIL:
        case RTSP_RESP_CODE_FRAME_TIMEOUT:
        {
            DPRINT(CAMERA_INTERFACE, "rtsp returned with frame timeout/fail: [camera=%d], [mediaResp=%d]", cameraIndex, mediaResp);
        }

        /* fall through */
        case RTSP_RESP_CODE_CONN_CLOSE:
        {
            if (mediaResp == RTSP_RESP_CODE_CONN_CLOSE)
            {
                DPRINT(CAMERA_INTERFACE, "camera closed with success: [camera=%d]", cameraIndex);
            }

            streamInfo[GET_STREAM_INDEX(triggerParam.camIndex)][GET_STREAM_TYPE(triggerParam.camIndex)].getStreamRequest.rtspHandle = INVALID_RTSP_HANDLE;
            triggerParam.mediaResp = CI_STREAM_INT_RESP_CLOSE;
        }
        break;
    }

    ciStreamExtTriggerHandler(&triggerParam);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief startStreamRetry
 * @param cameraIndex
 * @return SUCCESS/FAIL
 */
static BOOL startStreamRetry(UINT8 cameraIndex)
{
    STREAM_INFO_t *pStreamInfo = &streamInfo[GET_STREAM_INDEX(cameraIndex)][GET_STREAM_TYPE(cameraIndex)];

    if (pStreamInfo->retryTimerHandle == INVALID_TIMER_HANDLE)
    {
        TIMER_INFO_t timerInfo;

        timerInfo.count	= CONVERT_SEC_TO_TIMER_COUNT(STREAM_RETRY_TIME);
        timerInfo.funcPtr = streamRetryTimeout;
        timerInfo.data = (UINT32)cameraIndex;

        if (StartTimer(timerInfo, &pStreamInfo->retryTimerHandle) == FAIL)
        {
            EPRINT(CAMERA_INTERFACE, "fail to start stream retry timer: [camera=%d]", cameraIndex);
            return FAIL;
        }
    }
    else
    {
        ReloadTimer(pStreamInfo->retryTimerHandle, CONVERT_SEC_TO_TIMER_COUNT(STREAM_RETRY_TIME));
        DPRINT(CAMERA_INTERFACE, "reloading stream retry timer: [camera=%d]", cameraIndex);
    }

    pStreamInfo->streamRetryRunning = TRUE;
    return SUCCESS;
}


//-------------------------------------------------------------------------------------------------
/**
 * @brief streamRetryTimeout
 * @param index
 */
static void streamRetryTimeout(UINT32 index)
{
    CI_STREAM_EXT_TRG_PARAM_t	triggerParam;

    triggerParam.type = CI_STREAM_EXT_TRG_TIMER_CB;
    triggerParam.camIndex = (UINT8)index;

    ciStreamExtTriggerHandler(&triggerParam);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief stopAllStreamRequests
 * @param cameraIndex
 * @param mediaResp
 */
static void stopAllStreamRequests(UINT8 cameraIndex, CI_STREAM_INT_RESP_e mediaResp)
{
    UINT8           requestCount;
    STREAM_INFO_t   *pStreamInfo = &streamInfo[GET_STREAM_INDEX(cameraIndex)][GET_STREAM_TYPE(cameraIndex)];

    // Delete retry timer and keep alive timer
    DeleteTimer(&pStreamInfo->retryTimerHandle);

    /* Close stream connections. Closing the connections will call respective media callback.
     * The media callbacks will start retry timer and then new request will be sent with latest parameters */
    for (requestCount = 0; requestCount < pStreamInfo->getStreamRequest.numOfRequest; requestCount++)
    {
        if (pStreamInfo->getStreamRequest.url[requestCount].requestType != CAM_REQ_MEDIA)
        {
            continue;
        }

        if (CAM_RTSP_PROTOCOL == pStreamInfo->getStreamRequest.url[requestCount].protocolType)
        {
            if (pStreamInfo->getStreamRequest.rtspHandle == INVALID_RTSP_HANDLE)
            {
                continue;
            }

            if (CMD_SUCCESS != StopRtspStream(cameraIndex, pStreamInfo->getStreamRequest.rtspHandle))
            {
                EPRINT(CAMERA_INTERFACE, "fail to stop rtsp media: [camera=%d]", cameraIndex);
            }
            else
            {
                pStreamInfo->getStreamRequest.rtspHandle = INVALID_RTSP_HANDLE;
            }
        }
        else if (CAM_TCP_PROTOCOL == pStreamInfo->getStreamRequest.url[requestCount].protocolType)
        {
            CAMERA_REQUEST_t    cameraRequest;
            UINT8				profileNum = 0;
            TCP_REQ_INFO_t      tcpInfo;
            TCP_HANDLE			tcpClientId;

            getStreamProfileNum(GET_VIDEO_STREAM_TYPE(cameraIndex), GET_STREAM_INDEX(cameraIndex), &profileNum);
            profileNum = (profileNum > 0) ? (profileNum - 1) : MAIN_STREAM;
            GetCiRequestUrl(GET_STREAM_INDEX(cameraIndex), REQ_URL_STOP_STREAM, profileNum, CAM_REQ_MEDIA, NULL, &cameraRequest, NULL);

            tcpInfo.camIndex = cameraIndex;
            tcpInfo.tcpRequest = cameraRequest.url[0].tcpRequestType;
            snprintf(tcpInfo.commandString, MAX_CAMERA_URI_WIDTH, "%s", cameraRequest.url[0].relativeUrl);

            if (StartTcp(&tcpInfo, NULL, &tcpClientId, &profileNum) == CMD_SUCCESS)
            {
                pStreamInfo->getStreamRequest.http[requestCount] = INVALID_HTTP_HANDLE;
            }
        }
        else
        {
            if (pStreamInfo->getStreamRequest.http[requestCount] != INVALID_HTTP_HANDLE)
            {
                StopHttp(pStreamInfo->getStreamRequest.http[requestCount]);
            }

            DPRINT(CAMERA_INTERFACE, "stop http stream: [camera=%d], [handle=%d]", cameraIndex, pStreamInfo->getStreamRequest.http[requestCount]);
            if (mediaResp == CI_STREAM_INT_RESP_CLOSE)
            {
                /* NOTE: this is useful in only special case. Like if there are two HTTP media streams are running on the same cameraindex
                 * one is of video and one is of audio and if stop stream comes and we stops the stream for the video then we should stop
                 * the audio stream. so first stop request will be handled in the httpStreammediaCb(we are assigning invalid handler there)
                 * second would be handled in this function by this "if"..(think of the two stream.. Use your imegination and check) */
                pStreamInfo->getStreamRequest.http[requestCount] = INVALID_HTTP_HANDLE;
            }
        }
    }

    pStreamInfo->gopTotal = 0;
    pStreamInfo->gop = 0;
    pStreamInfo->fpsTotal = 0;
    pStreamInfo->fHeight = 0;
    pStreamInfo->fWidth = 0;
    resetPtsAvgParam(pStreamInfo);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It will reset frame time smoothing logic params
 * @param   pStreamInfo
 */
static void resetPtsAvgParam(STREAM_INFO_t *pStreamInfo)
{
    pStreamInfo->startWinId = 0;
    pStreamInfo->endWinId = 0;
    pStreamInfo->maxWindow = 0;
    pStreamInfo->frameDiff = 0;
    pStreamInfo->firstAudioFrame = CLEAR;
    pStreamInfo->firstVideoFrame = CLEAR;
    memset(pStreamInfo->presentationTimeAvg, 0, sizeof(pStreamInfo->presentationTimeAvg));
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It will average the frame time difference by taking the reference of last 100 frames time
 * @param   cameraIndex
 * @param   frameInfoPtr
 * @param   streamType
 */
static void actualTimeStampFrame(UINT8 cameraIndex, MEDIA_FRAME_INFO_t *frameInfoPtr, STREAM_TYPE_e streamType)
{
    if (streamType != STREAM_TYPE_VIDEO)
    {
        /* Nothing to do for audio */
        return;
    }

    UINT16          refWindIdCnt;
    STREAM_INFO_t   *pStreamInfo = &streamInfo[GET_STREAM_INDEX(cameraIndex)][GET_STREAM_TYPE(cameraIndex)];
    UINT64          presentaionTimeUs = ((frameInfoPtr->avPresentationTime.tv_sec * MICRO_SEC_PER_SEC) + (frameInfoPtr->avPresentationTime.tv_usec));

    /* Get previous window id for current timestamp comparision */
    refWindIdCnt = (pStreamInfo->endWinId) ? (pStreamInfo->endWinId - 1) : (WINDOW_SIZE - 1);

    /* Is latest frame have older time than previous frame? */
    if (presentaionTimeUs < pStreamInfo->presentationTimeAvg[refWindIdCnt])
    {
        /* If camera gives previous frame less than 1 sec then logic will misbehave */
        if ((pStreamInfo->presentationTimeAvg[refWindIdCnt] - presentaionTimeUs) > MICRO_SEC_PER_SEC)
        {
            /* Time difference is more than 1 seconds between current and previous frame */
            EPRINT(CAMERA_INTERFACE, "latest frame time is older than previous frame: [camera=%d], [diff=%llums]",
                   cameraIndex, (pStreamInfo->presentationTimeAvg[refWindIdCnt] - presentaionTimeUs)/MILLI_SEC_PER_SEC);
            resetPtsAvgParam(pStreamInfo);
            return;
        }

        /* Otherwise store previous time as current time */
        pStreamInfo->presentationTimeAvg[pStreamInfo->endWinId] = pStreamInfo->presentationTimeAvg[refWindIdCnt];
    }
    else
    {
        /* Store current pts for future reference */
        pStreamInfo->presentationTimeAvg[pStreamInfo->endWinId] = presentaionTimeUs;
    }

    /* Initially start averaging from 0 */
    if (pStreamInfo->maxWindow < WINDOW_SIZE)
    {
        /* Wait for 250 frames and then do 100 frames averaging */
        pStreamInfo->maxWindow++;
        pStreamInfo->startWinId = 0;
    }

    /* Check start and end window index */
    if (pStreamInfo->endWinId == pStreamInfo->startWinId)
    {
        /* This is first frame after reset */
        pStreamInfo->frameDiff = 0;
    }
    else
    {
        /* Derive number of frame window count for averaging and take oldest and latest from frame time difference from that window and do average */
        refWindIdCnt = (pStreamInfo->endWinId > pStreamInfo->startWinId) ?
                    (pStreamInfo->endWinId - pStreamInfo->startWinId) : ((WINDOW_SIZE - pStreamInfo->startWinId) + pStreamInfo->endWinId);
        pStreamInfo->frameDiff = ((pStreamInfo->presentationTimeAvg[pStreamInfo->endWinId] - pStreamInfo->presentationTimeAvg[pStreamInfo->startWinId]) / refWindIdCnt)/MICRO_SEC_PER_MS;

        /* If frame difference is more than 1sec then reset logic */
        if (pStreamInfo->frameDiff > MILLI_SEC_PER_SEC)
        {
            EPRINT(CAMERA_INTERFACE, "frame difference too high in averaging: [camera=%d], [diff=%ums]", cameraIndex, pStreamInfo->frameDiff);
            resetPtsAvgParam(pStreamInfo);
            return;
        }
    }

    /* Increment window id for next frame */
    pStreamInfo->endWinId++;
    if (pStreamInfo->endWinId >= WINDOW_SIZE)
    {
        /* Window id is rollover */
        pStreamInfo->endWinId = 0;
    }

    /* Is frame difference found? */
    if (pStreamInfo->frameDiff > 0)
    {
        /* Derive oldest frame window id for averaging */
        pStreamInfo->startWinId = pStreamInfo->endWinId - AVG_WINDOW_SIZE;

        /* Is difference negative? */
        if(pStreamInfo->startWinId  < 0)
        {
            /* Add max window to make it positive and keep value to below or equal 100 */
            pStreamInfo->startWinId += WINDOW_SIZE;
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set date and time of all configured cameras
 * @param   data
 */
static void setAllCameraDateTime(UINT32 data)
{
    UINT8 cameraIndex;

    for (cameraIndex = 0; cameraIndex < getMaxCameraForCurrentVariant(); cameraIndex++)
    {
        /* Set camera date and time */
        setCameraDateTime(cameraIndex);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set data and time in camera
 * @param   cameraIndex
 * @return  Command status
 */
static NET_CMD_STATUS_e setCameraDateTime(UINT8 cameraIndex)
{
    CAMERA_CONFIG_t     camConfig;
    IP_CAMERA_CONFIG_t 	ipCamCfg;

    /* Validate input param */
    if (cameraIndex >= getMaxCameraForCurrentVariant())
    {
        EPRINT(CAMERA_INTERFACE, "invld camera index: [camera=%d]", cameraIndex);
        return CMD_PROCESS_ERROR;
    }

    ReadSingleCameraConfig(cameraIndex, &camConfig);
    if (camConfig.camera == DISABLE)
    {
        return CMD_CHANNEL_DISABLED;
    }

    ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);
    if (ipCamCfg.onvifSupportF == TRUE)
    {
        if ((ipCamCfg.brand[0] == '\0') || (ipCamCfg.model[0] == '\0'))
        {
            EPRINT(CAMERA_INTERFACE, "onvif capability not recv, brand-model unknown: [camera=%d]", cameraIndex);
            return CMD_ONVIF_CAM_CAPABILITY_ERROR;
        }

        return sendOnvifCommonCmd(cameraIndex, ONVIF_SET_DATE_TIME, onvifSetDateTimeCb, &ipCamCfg, NULL);
    }
    else if (camConfig.type == AUTO_ADDED_CAMERA)
    {
        GetCiRequestUrl(cameraIndex, REQ_URL_SET_DATE_TIME, 0, CAM_REQ_CONTROL, tcpSetDateTimeCb, &setDateTimeRequest[cameraIndex].request, NULL);
        return sendReqToCamera(&setDateTimeRequest[cameraIndex].request, &ipCamCfg, MAIN_STREAM);
    }
    else
    {
        if (CMD_SUCCESS != GetCameraBrandModelUrl(cameraIndex, REQ_URL_SET_DATE_TIME, &setDateTimeRequest[cameraIndex].request, NULL, NULL, NULL, NULL))
        {
            return CMD_PROCESS_ERROR;
        }

        setDateTimeRequest[cameraIndex].request.requestCount = 0;
        setDateTimeRequest[cameraIndex].request.url[0].requestType = CAM_REQ_CONTROL;
        setDateTimeRequest[cameraIndex].request.httpCallback[CAM_REQ_CONTROL] = httpSetDateTimeCb;
        return sendReqToCamera(&setDateTimeRequest[cameraIndex].request, &ipCamCfg, MAIN_STREAM);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief CiActionChangeDateTime : called from DateTime.c when SetNewDateTime() function is called.
 *        It resets the all algorithm for putting time in frame header.
 */
void CiActionChangeDateTime(void)
{
    UINT8 cameraIndex, loop;

    DPRINT(CAMERA_INTERFACE, "system date-time updated: reset the frame time calc algorithm");

    for(cameraIndex = 0; cameraIndex < getMaxCameraForCurrentVariant(); cameraIndex++)
    {
        for (loop = 0; loop < MAX_STREAM_TYPE; loop++)
        {
            MUTEX_LOCK(streamInfo[cameraIndex][loop].frameMarker.writeBuffLock);
            resetPtsAvgParam(&streamInfo[cameraIndex][loop]);
            MUTEX_UNLOCK(streamInfo[cameraIndex][loop].frameMarker.writeBuffLock);
        }
    }

    /* When date or time change need to update in all camera */
    setAllCameraDateTime(0);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API is used to store the received frame to the respective buffer. Actually, two
 *          buffers are maintained, one is frame buffer which stores the incoming frames and other
 *          stores location of those frames in frame buffer, length of frame and other frame information.
 *          Both buffers are circular; i.e. whenever overflow is detected, buffer is rolled back to
 *          beginning position.
 * @param cameraIndex
 * @param streamType
 * @param streamData
 * @param frameInfoPtr
 */
static void writeToStreamBuff(UINT8 cameraIndex, STREAM_TYPE_e streamType, UINT8PTR streamData, MEDIA_FRAME_INFO_t *frameInfoPtr)
{
    BOOL					frameIflag = FALSE;
    INT16 					writeIndex, maxWriteIndex;
    UINT8PTR 				nextFramePtr;
    STREAM_STATUS_INFO_t	*streamStatusPtr;
    STREAM_INFO_t			*pStreamInfo;
    LocalTime_t 			currentSystemTime = {0};

    /* Get stream info in local variable for ease of use */
    pStreamInfo = &streamInfo[GET_STREAM_INDEX(cameraIndex)][GET_STREAM_TYPE(cameraIndex)];

    /* Is this video frame? */
    if(streamType == STREAM_TYPE_VIDEO)
    {
        /* Update fps and gop counter */
        pStreamInfo->fps++;
        pStreamInfo->gop++;

        /* Is this i-frame? */
        if(frameInfoPtr->videoInfo.frameType == I_FRAME)
        {
            /* Store previous gop and reset gop counter */
            pStreamInfo->gopTotal = pStreamInfo->gop;
            pStreamInfo->gop = 0;
            frameIflag = TRUE;
        }

        /* Is video height-width changed? */
        if ((pStreamInfo->fWidth != frameInfoPtr->videoInfo.width) || (pStreamInfo->fHeight != frameInfoPtr->videoInfo.height))
        {
            /* Store newly received height-width and get matrix's resolution id */
            pStreamInfo->fWidth = frameInfoPtr->videoInfo.width;
            pStreamInfo->fHeight = frameInfoPtr->videoInfo.height;
            GetResolutionId(frameInfoPtr->videoInfo.width, frameInfoPtr->videoInfo.height, &pStreamInfo->fResolution);
        }
    }

    /* Get lock for frame write into buffer */
    MUTEX_LOCK(pStreamInfo->frameMarker.writeBuffLock);
    pthread_rwlock_rdlock(&pStreamInfo->frameMarker.writeIndexLock);
    writeIndex = pStreamInfo->frameMarker.wrPos;
    maxWriteIndex = pStreamInfo->frameMarker.maxWriteIndex;
    pthread_rwlock_unlock(&pStreamInfo->frameMarker.writeIndexLock);

    /* if we have written 1000 frames already in buffer, assign starting of buf to framePtr again :)) */
    if(writeIndex >= MAX_FRAME_IN_BUFFER)
    {
        maxWriteIndex = MAX_FRAME_IN_BUFFER;
        writeIndex = 0;
        pStreamInfo->frameMarker.frameInfo[writeIndex].framePtr = pStreamInfo->bufferPtr;
    }

    /* pStreamInfo->bufferPtr is a starting position of buffer of 5Mb or 2Mb.
     * so if current position - starting pos + current frame length > max buf size then our buf is overflowed */
    if ((pStreamInfo->frameMarker.frameInfo[writeIndex].framePtr - pStreamInfo->bufferPtr + frameInfoPtr->len) >= pStreamInfo->bufferSize)
    {
        maxWriteIndex = writeIndex;
        writeIndex = 0;
        pStreamInfo->frameMarker.frameInfo[writeIndex].framePtr = pStreamInfo->bufferPtr;
    }

    /* Store the received frame to frame buffer at current write pointer */
    memcpy(pStreamInfo->frameMarker.frameInfo[writeIndex].framePtr, streamData, frameInfoPtr->len);

    /* Update video loss status as no video loss */
    updateVideoLossStatus(GET_STREAM_INDEX(cameraIndex), TRUE);

    /* Store frame configuration data into frame marker at current write index */
    pStreamInfo->frameMarker.frameInfo[writeIndex].frameLen = frameInfoPtr->len;
    streamStatusPtr = &pStreamInfo->frameMarker.frameInfo[writeIndex].streamStatusInfo;

    /* Get system's local time */
    GetLocalTime(&currentSystemTime);

    /* Is this frame received from rtsp? */
    if(frameInfoPtr->isRTSP == TRUE)
    {
        /* Get average frame difference */
        actualTimeStampFrame(cameraIndex, frameInfoPtr, streamType);

        /* Get system time of video and audio frame for first frame */
        if((pStreamInfo->firstVideoFrame == CLEAR) && (streamType == STREAM_TYPE_VIDEO))
        {
            /* First video frame received. Start smoothing logic with current time */
            DPRINT(CAMERA_INTERFACE, "first video frame received: [camera=%d]", cameraIndex);
            pStreamInfo->localPrevTime = streamStatusPtr->localTime = currentSystemTime;
            pStreamInfo->firstVideoFrame = SET;
            pStreamInfo->fps = pStreamInfo->gop = pStreamInfo->gopTotal = 0;
        }
        else if((pStreamInfo->firstAudioFrame == CLEAR) && (streamType == STREAM_TYPE_AUDIO))
        {
            /* First audio frame received. Start smoothing logic with current time */
            DPRINT(CAMERA_INTERFACE, "first audio frame received: [camera=%d]", cameraIndex);
            streamStatusPtr->localTime = currentSystemTime;
            pStreamInfo->firstAudioFrame = SET;
        }

        if(streamType == STREAM_TYPE_VIDEO)
        {
            /* Reseting last audio frame time for new video frame */
            pStreamInfo->localPrevTimeAudio.totalSec = 0;
            pStreamInfo->localPrevTimeAudio.mSec = 0;

            /* Increase the video time by average difference got from camera PTS */
            if(pStreamInfo->frameDiff == 0)
            {
                /* Store previous time as current time because no time difference between current and previous frame */
                streamStatusPtr->localTime = pStreamInfo->localPrevTime;
            }
            else
            {
                /* Add current and previous frame time difference in previous frame time */
                streamStatusPtr->localTime.totalSec = pStreamInfo->localPrevTime.totalSec;
                streamStatusPtr->localTime.mSec = pStreamInfo->localPrevTime.mSec + (pStreamInfo->frameDiff);

                /* Rollover milli-seconds in second */
                if(streamStatusPtr->localTime.mSec >= MILLI_SEC_PER_SEC)
                {
                    /* Update the frame time */
                    streamStatusPtr->localTime.mSec -= MILLI_SEC_PER_SEC;
                    streamStatusPtr->localTime.totalSec = streamStatusPtr->localTime.totalSec + 1;
                }

                /* Store current frame time in previous for future reference */
                pStreamInfo->localPrevTime = streamStatusPtr->localTime;
            }

            /* Get system current time and frame time in milli-seconds */
            UINT32 actualTimeDiff;
            UINT64 systemTimeInMs = ((UINT64)currentSystemTime.totalSec * MILLI_SEC_PER_SEC) + currentSystemTime.mSec;
            UINT64 frameTimeInMs = ((UINT64)streamStatusPtr->localTime.totalSec * MILLI_SEC_PER_SEC) + streamStatusPtr->localTime.mSec;

            /* Is system time is latest? */
            if (systemTimeInMs >= frameTimeInMs)
            {
                /* Get system time and frame time diff */
                actualTimeDiff = systemTimeInMs - frameTimeInMs;

                /* Check actual time difference */
                if (actualTimeDiff > (2*MILLI_SEC_PER_SEC))
                {
                    /* Time diff is more than 2 sec. Hence update frame time with system time */
                    streamStatusPtr->localTime = currentSystemTime;

                    /* If it is more than 5 sec than print debug */
                    if (actualTimeDiff > (5*MILLI_SEC_PER_SEC))
                    {
                        WPRINT(CAMERA_INTERFACE, "more time diff in system and frame time: [camera=%d], [diff=%.3fsec]", cameraIndex, (float)actualTimeDiff/MILLI_SEC_PER_SEC);
                    }
                }
                else if (actualTimeDiff >= 300)
                {
                    /* Every 300ms time difference, add 1ms in frame time for smoothing */
                    streamStatusPtr->localTime.mSec += (actualTimeDiff / 300);
                }

                /* Rollover milli-seconds in second */
                if (streamStatusPtr->localTime.mSec >= MILLI_SEC_PER_SEC)
                {
                    /* Update the frame time */
                    streamStatusPtr->localTime.mSec -= MILLI_SEC_PER_SEC;
                    streamStatusPtr->localTime.totalSec += 1;
                }

                /* Store current frame time in previous for future reference */
                pStreamInfo->localPrevTime = streamStatusPtr->localTime;
            }
            else
            {
                /* Get system time and frame time diff */
                actualTimeDiff = frameTimeInMs - systemTimeInMs;

                /* Is received frame 30 sec older than current time? */
                if (actualTimeDiff > (30*MILLI_SEC_PER_SEC))
                {
                    /* Reset smoothing logic on more than 30sec difference */
                    EPRINT(CAMERA_INTERFACE, "system time is left back than frame header time. resetting algorithm: [camera=%d]", cameraIndex);
                    streamStatusPtr->localTime = currentSystemTime;
                    resetPtsAvgParam(pStreamInfo);
                }
            }
        }
        else if(streamType == STREAM_TYPE_AUDIO)
        {
            /* Is last audio frame time is valid? */
            if ((((UINT64)pStreamInfo->localPrevTimeAudio.totalSec * MILLI_SEC_PER_SEC) + pStreamInfo->localPrevTimeAudio.mSec) > 0)
            {
                /* Take reference from last audio frame time. It will happen when either only audio frames receive or
                 * two consecutive audio frames receive without video. On video frame, previous audio frame time will get reset */
                streamStatusPtr->localTime = pStreamInfo->localPrevTimeAudio;
            }
            else
            {
                /* Take reference from last video frame time */
                streamStatusPtr->localTime = pStreamInfo->localPrevTime;
            }

            /* Consider elapsed actual time is 3ms from last video frame time or audio frame time */
            streamStatusPtr->localTime.mSec += 3;

            /* Rollover milli-seconds in second */
            if(streamStatusPtr->localTime.mSec >= MILLI_SEC_PER_SEC)
            {
                /* Update the frame time */
                streamStatusPtr->localTime.mSec -= MILLI_SEC_PER_SEC;
                streamStatusPtr->localTime.totalSec	+= 1;
            }

            /* Store current frame time in previous for future reference */
            pStreamInfo->localPrevTimeAudio = streamStatusPtr->localTime;
        }
    }
    else
    {
        /* If we haven't got PTS then take system time */
        streamStatusPtr->localTime = currentSystemTime;
    }

    /* Calculate frame rate for video stream */
    if(streamType == STREAM_TYPE_VIDEO)
    {
        /* Is second changed? */
        if(pStreamInfo->lastFrameTimeSec != streamStatusPtr->localTime.totalSec)
        {
            /* Store previous fps and reset fps counter */
            pStreamInfo->fpsTotal = pStreamInfo->fps;
            pStreamInfo->fps = 0;
            pStreamInfo->lastFrameTimeSec = streamStatusPtr->localTime.totalSec;
        }

        /* Set resolution for video stream */
        streamStatusPtr->streamPara.resolution = pStreamInfo->fResolution;

        /* Set configured fps for video. fps might be zero if not updated from camera due to error then set default */
        streamStatusPtr->streamPara.sampleRate = (pStreamInfo->configFrameRate == 0) ? DEFAULT_FPS : pStreamInfo->configFrameRate;
    }
    else
    {
        /* Set invalid resolution for audio stream */
        streamStatusPtr->streamPara.resolution = MAX_RESOLUTION;

        /* Set sampling frequency for audio */
        streamStatusPtr->streamPara.sampleRate = frameInfoPtr->sampleRate;
    }

    /* Update other frame parameters */
    streamStatusPtr->streamType = streamType;
    streamStatusPtr->streamPara.videoStreamType = frameInfoPtr->videoInfo.frameType;
    streamStatusPtr->streamPara.noOfRefFrame = frameInfoPtr->videoInfo.noOfRefFrame;
    streamStatusPtr->streamPara.streamCodecType = frameInfoPtr->codecType;

    // Set next frame pointer to current write pointer plus current frame length
    nextFramePtr = (pStreamInfo->frameMarker.frameInfo[writeIndex].framePtr + frameInfoPtr->len);

    /* Increment write index by one */
    writeIndex++;
    if (writeIndex < MAX_FRAME_IN_BUFFER)
    {
        /* Update frame buffer for next frame */
        pStreamInfo->frameMarker.frameInfo[writeIndex].framePtr = nextFramePtr;
    }

    /* Update write index */
    if (maxWriteIndex < writeIndex)
    {
        maxWriteIndex = writeIndex;
    }

    /* Lock for frame buffer location update */
    pthread_rwlock_wrlock(&pStreamInfo->frameMarker.writeIndexLock);
    if (frameIflag == TRUE)
    {
        /* Set index for last received i-frame for reference */
        pStreamInfo->frameMarker.lastIframe = (writeIndex - 1);
    }

    /* Update write position and index */
    pStreamInfo->frameMarker.wrPos = writeIndex;
    pStreamInfo->frameMarker.maxWriteIndex = maxWriteIndex;
    pthread_rwlock_unlock(&pStreamInfo->frameMarker.writeIndexLock);
    MUTEX_UNLOCK(pStreamInfo->frameMarker.writeBuffLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function used to read a frame from the frame buffer. Other stream status information
 *          is also passed to user.
 * @param   cameraIndex
 * @param   clientIndex
 * @param   streamStatusInfo
 * @param   streamData
 * @param   streamDataLen
 * @param   firstIframeSent
 * @return  No. of pending frames
 */
static UINT32 readFromStreamBuff(UINT16 cameraIndex, UINT8 clientIndex, STREAM_STATUS_INFO_t **streamStatusInfo,
                                 UINT8PTR *streamData, UINT32PTR streamDataLen, BOOL firstIframeSent)
{
    BOOL				updateRdIdx = FALSE;
    INT16 				maxWriteIdx, writeIndex, readIndex, lastIframe;
    UINT32 				framesInBuff = 0;
    STREAM_INFO_t		*pStreamInfo = &streamInfo[GET_STREAM_INDEX(cameraIndex)][GET_STREAM_TYPE(cameraIndex)];
    BUFFER_MARKER_t		*frameMarkPtr = &pStreamInfo->frameMarker;

    pthread_rwlock_rdlock(&frameMarkPtr->writeIndexLock);
    writeIndex = frameMarkPtr->wrPos;
    maxWriteIdx = frameMarkPtr->maxWriteIndex;
    lastIframe = frameMarkPtr->lastIframe;
    pthread_rwlock_unlock(&frameMarkPtr->writeIndexLock);

    readIndex = (firstIframeSent == FALSE) ? lastIframe : frameMarkPtr->rdPos[clientIndex];
    if ((writeIndex == readIndex) || (writeIndex >= MAX_FRAME_IN_BUFFER))
    {
        return 0;
    }

    if (readIndex >= maxWriteIdx)
    {
        readIndex = 0;
    }

    framesInBuff = (UINT32)((maxWriteIdx - (readIndex - writeIndex)) % maxWriteIdx);
    if (clientIndex != CI_STREAM_CLIENT_RECORD)
    {
        if(framesInBuff > (UINT32)pStreamInfo->gopTotal)
        {
            if(writeIndex > readIndex)
            {
                if((lastIframe > readIndex) && (lastIframe < writeIndex))
                {
                    updateRdIdx = TRUE;
                }
            }
            else if(writeIndex < readIndex)
            {
                if(!((lastIframe < readIndex) && (lastIframe > writeIndex)))
                {
                    updateRdIdx = TRUE;
                }
            }

            if(updateRdIdx == TRUE)
            {
                readIndex = lastIframe;
                framesInBuff = (UINT32)((maxWriteIdx - (readIndex - writeIndex)) % maxWriteIdx);
            }
        }
    }

    *streamData = frameMarkPtr->frameInfo[readIndex].framePtr;
    *streamStatusInfo =	&frameMarkPtr->frameInfo[readIndex].streamStatusInfo;
    *streamDataLen = frameMarkPtr->frameInfo[readIndex].frameLen;

    frameMarkPtr->rdPos[clientIndex] = (readIndex + 1);
    return framesInBuff;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API is used to fetch the pending frames from the live stream buffer. It should be
 *          called upon receipt of the live frame received signal from live frame notification function.
 * @param   cameraIndex
 * @param   clientIndex
 * @param   streamStatusInfo
 * @param   streamBuffPtr
 * @param   streamDataLen
 * @return  No. of pending frames
 */
UINT32 GetNextFrame(UINT16 cameraIndex, UINT8 clientIndex, STREAM_STATUS_INFO_t **streamStatusInfo, UINT8PTR *streamBuffPtr, UINT32PTR streamDataLen)
{
    if ((GET_STREAM_INDEX(cameraIndex) < getMaxCameraForCurrentVariant()) && (clientIndex < MAX_CI_STREAM_CLIENT))
    {
        // Read pending frame from live buffer
        return readFromStreamBuff(cameraIndex, clientIndex, streamStatusInfo, streamBuffPtr, streamDataLen, TRUE);
    }

    return 0;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API is used to fetch the pending frames from the live stream buffer. It should be
 *          called upon receipt of the live frame received signal from live frame notification function.
 * @param   cameraIndex
 * @param   clientIndex
 * @param   streamStatusInfo
 * @param   streamBuffPtr
 * @param   streamDataLen
 * @param   firstIframeSent
 * @return  No. of pending frames
 */
UINT32 GetNextFrameForLive(UINT16 cameraIndex, UINT8 clientIndex, STREAM_STATUS_INFO_t **streamStatusInfo,
                           UINT8PTR * streamBuffPtr, UINT32PTR streamDataLen, BOOL firstIframeSent)
{
    if ((GET_STREAM_INDEX(cameraIndex) < getMaxCameraForCurrentVariant()) && (clientIndex < MAX_CI_STREAM_CLIENT))
    {
        // Read pending frame from live buffer
        return readFromStreamBuff(cameraIndex, clientIndex, streamStatusInfo, streamBuffPtr, streamDataLen, firstIframeSent);
    }

    return 0;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetImage :sends request to IP camera for JPEG image of the specified resolution.
 * @param cameraIndex
 * @param resolutionStr
 * @param callback
 * @param clientCbType
 * @return Network status command
 */
NET_CMD_STATUS_e GetImage(UINT8 cameraIndex, CHARPTR resolutionStr, IMAGE_REQUEST_CB callback, CLIENT_CB_TYPE_e clientCbType)
{
    NET_CMD_STATUS_e 	requestStatus = CMD_SUCCESS;
    CAMERA_CONFIG_t 	camCfg;
    IP_CAMERA_CONFIG_t 	ipCamCfg;

    if (cameraIndex >= getMaxCameraForCurrentVariant())
    {
        EPRINT(CAMERA_INTERFACE, "invld camera index found: [camera=%d]", cameraIndex);
        return CMD_PROCESS_ERROR;
    }

    /* Read camera configuration and check whether camera is enabled or not */
    ReadSingleCameraConfig(cameraIndex, &camCfg);
    if (camCfg.camera == DISABLE)
    {
        /* Camera is disabled */
        return CMD_CHANNEL_DISABLED;
    }

    // Only one request will be served at a time. hence if request is busy give failure
    MUTEX_LOCK(getImageRequest[cameraIndex].camReqFlagLock);
    if (getImageRequest[cameraIndex].camReqBusyF == BUSY)
    {
        MUTEX_UNLOCK(getImageRequest[cameraIndex].camReqFlagLock);
        EPRINT(CAMERA_INTERFACE, "get image request is busy: [camera=%d]", cameraIndex);
        return CMD_CAM_REQUEST_IN_PROCESS;
    }

    // If request is free, reset the flag
    getImageRequest[cameraIndex].camReqBusyF = BUSY;
    MUTEX_UNLOCK(getImageRequest[cameraIndex].camReqFlagLock);

    DPRINT(CAMERA_INTERFACE, "image request received: [camera=%d]", cameraIndex);
    getImageRequest[cameraIndex].clientCb[ON] = callback;
    getImageRequest[cameraIndex].clientCbType = clientCbType;

    /* Read ip camera config */
    ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);

    if(ipCamCfg.onvifSupportF == TRUE)
    {
        if ((ipCamCfg.brand[0] == '\0') || (ipCamCfg.model[0] == '\0'))
        {
            MUTEX_LOCK(onvifCamDetail[cameraIndex].onvifAccessLock);
            BOOL authFailF = onvifCamDetail[cameraIndex].authFailF;
            MUTEX_UNLOCK(onvifCamDetail[cameraIndex].onvifAccessLock);
            if (authFailF == TRUE)
            {
                EPRINT(CAMERA_INTERFACE, "onvif brand-model unknown b'coz auth fail: [camera=%d]", cameraIndex);
                requestStatus = CMD_CRED_INVALID;
            }
            else
            {
                EPRINT(CAMERA_INTERFACE, "onvif capability not recv, brand-model unknown: [camera=%d]", cameraIndex);
                requestStatus = CMD_ONVIF_CAM_CAPABILITY_ERROR;
            }
        }
        else
        {
            if (getImageRequest[cameraIndex].url[0].relativeUrl[0] == '\0')
            {
                requestStatus = sendOnvifCommonCmd(cameraIndex, ONVIF_GET_SNAPSHOT_URL, onvifImageCb, &ipCamCfg, NULL);
            }
            else
            {
                getImageRequest[cameraIndex].requestCount = 0;
                getImageRequest[cameraIndex].numOfRequest = 1;
                requestStatus = sendReqToCamera(&getImageRequest[cameraIndex], &ipCamCfg, MAIN_STREAM);
            }
        }
    }
    else if (CameraType(cameraIndex) == AUTO_ADDED_CAMERA)
    {
        GetCiRequestUrl(cameraIndex, REQ_URL_GET_IMAGE, 0, CAM_REQ_MEDIA, tcpGetTestCamCb, &getImageRequest[cameraIndex], NULL);
        getImageRequest[cameraIndex].requestCount = 0;
        DPRINT(CAMERA_INTERFACE, "image request: [camera=%d], [data=%s]", cameraIndex, getImageRequest[cameraIndex].url[0].relativeUrl);
        requestStatus =	sendReqToCamera(&getImageRequest[cameraIndex], &ipCamCfg, MAIN_STREAM);
    }
    else
    {
        requestStatus = GetCameraBrandModelUrl(cameraIndex, REQ_URL_GET_IMAGE, &getImageRequest[cameraIndex], NULL, NULL, NULL, NULL);
        if (CMD_SUCCESS == requestStatus)
        {
            getImageRequest[cameraIndex].requestCount = 0;
            getImageRequest[cameraIndex].requestStatus = CMD_PROCESS_ERROR;
            requestStatus =	sendReqToCamera(&getImageRequest[cameraIndex], &ipCamCfg, MAIN_STREAM);
        }
    }

    if (requestStatus != CMD_SUCCESS)
    {
        MUTEX_LOCK(getImageRequest[cameraIndex].camReqFlagLock);
        getImageRequest[cameraIndex].camReqBusyF = FREE;
        MUTEX_UNLOCK(getImageRequest[cameraIndex].camReqFlagLock);
    }

    return requestStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   call back to the snapshot request of an ONVIF camera. It will handle the response and
 *          take action as per the response received.
 * @param   responseData
 * @return  SUCCESS/FAIL
 */
static BOOL onvifImageCb(ONVIF_RESPONSE_PARA_t *responseData)
{
    NET_CMD_STATUS_e requestStatus = CMD_SUCCESS;

    if (responseData->cameraIndex >= getMaxCameraForCurrentVariant())
    {
        EPRINT(CAMERA_INTERFACE, "invld camera index found: [camera=%d]", responseData->cameraIndex);
        return FAIL;
    }

    if(responseData->response == ONVIF_CMD_SUCCESS)
    {
        IP_CAMERA_CONFIG_t ipCamCfg;

        memcpy(&getImageRequest[responseData->cameraIndex].url[0], (URL_REQUEST_t *)responseData->data, sizeof(URL_REQUEST_t));
        getImageRequest[responseData->cameraIndex].requestCount = 0;
        getImageRequest[responseData->cameraIndex].numOfRequest = 1;
        ReadSingleIpCameraConfig(responseData->cameraIndex, &ipCamCfg);
        requestStatus = sendReqToCamera(&getImageRequest[responseData->cameraIndex], &ipCamCfg, MAIN_STREAM);
    }
    else
    {
        if(responseData->response == ONVIF_CMD_FEATURE_NOT_SUPPORTED)
        {
            requestStatus = CMD_FEATURE_NOT_SUPPORTED;
            EPRINT(CAMERA_INTERFACE, "get image not supported: [camera=%d]", responseData->cameraIndex);
        }
        else
        {
            requestStatus = CMD_PROCESS_ERROR;
            EPRINT(CAMERA_INTERFACE, "get image request failed: [camera=%d]", responseData->cameraIndex);
        }
    }

    if (requestStatus != CMD_SUCCESS)
    {
        if (getImageRequest[responseData->cameraIndex].clientCb[ON] != NULL)
        {
            ((IMAGE_REQUEST_CB)getImageRequest[responseData->cameraIndex].clientCb[ON])
                    (responseData->cameraIndex, requestStatus, NULL, 0, getImageRequest[responseData->cameraIndex].clientCbType);
        }

        MUTEX_LOCK(getImageRequest[responseData->cameraIndex].camReqFlagLock);
        getImageRequest[responseData->cameraIndex].camReqBusyF = FREE;
        MUTEX_UNLOCK(getImageRequest[responseData->cameraIndex].camReqFlagLock);
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief onvifSetDateTimeCb
 * @param responseData
 * @return SUCCESS/FAIL
 */
static BOOL onvifSetDateTimeCb(ONVIF_RESPONSE_PARA_t *responseData)
{
    if(responseData->cameraIndex >= getMaxCameraForCurrentVariant())
    {
        EPRINT(CAMERA_INTERFACE, "invld camera index found: [camera=%d]", responseData->cameraIndex);
        return FAIL;
    }

    switch (responseData->response)
    {
        case ONVIF_CMD_SUCCESS:
            DPRINT(CAMERA_INTERFACE, "date-time set successfully: [camera=%d]", responseData->cameraIndex);
            break;

        case ONVIF_CMD_FEATURE_NOT_SUPPORTED:
            break;

        case ONVIF_CMD_AUTHENTICATION_FAIL:
            EPRINT(CAMERA_INTERFACE, "auth failed in set date-time: [camera=%d]", responseData->cameraIndex);
            break;

        case ONVIF_CMD_FAIL:
        case ONVIF_CMD_TIMEOUT:
            if (startOnvifRetryTimer(responseData->cameraIndex, ONVIF_SET_DATE_TIME) != SUCCESS)
            {
                DPRINT(CAMERA_INTERFACE, "retry timer started for set date-time: [camera=%d]", responseData->cameraIndex);
            }
            break;

        default:
            break;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief httpImageControlCb
 * @param httpHandle
 * @param dataInfo
 */
static void httpImageControlCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo)
{
    NET_CMD_STATUS_e 	requestStatus = CMD_SUCCESS;
    UINT8 				cameraIndex = dataInfo->userData;
    UINT8 				requestCount;
    IP_CAMERA_CONFIG_t 	ipCamCfg;

    if (cameraIndex >= getMaxCameraForCurrentVariant())
    {
        return;
    }

    for (requestCount = 0; requestCount < getImageRequest[cameraIndex].numOfRequest; requestCount++)
    {
        if (getImageRequest[cameraIndex].http[requestCount] == httpHandle)
        {
            break;
        }
    }

    if (requestCount >= getImageRequest[cameraIndex].numOfRequest)
    {
        return;
    }

    switch(dataInfo->httpResponse)
    {
        case HTTP_SUCCESS:
        {
            DPRINT(CAMERA_INTERFACE, "http success: [camera=%d], [requestCount=%d], [status=%s]", cameraIndex, requestCount, httpRespStr[HTTP_SUCCESS]);
            getImageRequest[cameraIndex].requestCount++;
            getImageRequest[cameraIndex].requestStatus = CMD_SUCCESS;
        }
        break;

        case HTTP_CLOSE_ON_SUCCESS:
        case HTTP_CLOSE_ON_ERROR:
        {
            DPRINT(CAMERA_INTERFACE, "http done: [camera=%d], [requestCount=%d], [status=%s]", cameraIndex, requestCount, httpRespStr[dataInfo->httpResponse]);
            getImageRequest[cameraIndex].http[requestCount] = INVALID_HTTP_HANDLE;
            if (getImageRequest[cameraIndex].requestStatus == CMD_SUCCESS)
            {
                ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);
                getImageRequest[cameraIndex].requestStatus = CMD_PROCESS_ERROR;
                requestStatus = sendReqToCamera(&getImageRequest[cameraIndex], &ipCamCfg, MAIN_STREAM);
            }
            else
            {
                requestStatus = getImageRequest[cameraIndex].requestStatus;
            }

            if (requestStatus != CMD_SUCCESS)
            {
                if (getImageRequest[cameraIndex].clientCb[ON] != NULL)
                {
                    ((IMAGE_REQUEST_CB)getImageRequest[cameraIndex].clientCb[ON])
                            (cameraIndex, requestStatus, NULL, 0, getImageRequest[cameraIndex].clientCbType);
                    getImageRequest[cameraIndex].clientCb[ON] = NULL;
                }

                MUTEX_LOCK(getImageRequest[cameraIndex].camReqFlagLock);
                getImageRequest[cameraIndex].camReqBusyF = FREE;
                MUTEX_UNLOCK(getImageRequest[cameraIndex].camReqFlagLock);
            }
        }
        break;

        default:
        {
            getImageRequest[cameraIndex].requestStatus = dataInfo->cmdResponse;
            EPRINT(CAMERA_INTERFACE, "http error: [camera=%d], [requestCount=%d], [status=%s]", cameraIndex, requestCount, httpRespStr[HTTP_ERROR]);
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief httpImageMediaCb
 * @param httpHandle
 * @param dataInfo
 */
static void httpImageMediaCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo)
{
    UINT8   cameraIndex = dataInfo->userData;
    UINT8   requestCount;

    if (cameraIndex >= getMaxCameraForCurrentVariant())
    {
        return;
    }

    for (requestCount = 0; requestCount < getImageRequest[cameraIndex].numOfRequest; requestCount++)
    {
        if(getImageRequest[cameraIndex].http[requestCount] == httpHandle)
        {
            break;
        }
    }

    if (requestCount >= getImageRequest[cameraIndex].numOfRequest)
    {
        return;
    }

    switch(dataInfo->httpResponse)
    {
        case HTTP_SUCCESS:
        {
            DPRINT(CAMERA_INTERFACE, "http success: [camera=%d], [requestCount=%d], [status=%s]", cameraIndex, requestCount, httpRespStr[HTTP_SUCCESS]);
            if (getImageRequest[cameraIndex].clientCb[ON] != NULL)
            {
                ((IMAGE_REQUEST_CB)getImageRequest[cameraIndex].clientCb[ON])
                        (cameraIndex, CMD_SUCCESS, dataInfo->storagePtr, dataInfo->frameSize, getImageRequest[cameraIndex].clientCbType);
                getImageRequest[cameraIndex].clientCb[ON] = NULL;
            }
        }
        break;

        case HTTP_CLOSE_ON_SUCCESS:
        case HTTP_CLOSE_ON_ERROR:
        {
            DPRINT(CAMERA_INTERFACE, "http done: [camera=%d], [requestCount=%d], [status=%s]", cameraIndex, requestCount, httpRespStr[dataInfo->httpResponse]);
            getImageRequest[cameraIndex].http[requestCount] = INVALID_HTTP_HANDLE;

            if (getImageRequest[cameraIndex].clientCb[ON] != NULL)
            {
                ((IMAGE_REQUEST_CB)getImageRequest[cameraIndex].clientCb[ON])
                        (cameraIndex, getImageRequest[cameraIndex].requestStatus, NULL, 0, getImageRequest[cameraIndex].clientCbType);
                getImageRequest[cameraIndex].clientCb[ON] = NULL;
            }

            MUTEX_LOCK(getImageRequest[cameraIndex].camReqFlagLock);
            getImageRequest[cameraIndex].camReqBusyF = FREE;
            MUTEX_UNLOCK(getImageRequest[cameraIndex].camReqFlagLock);
        }
        break;

        default:
        {
            getImageRequest[cameraIndex].requestStatus = dataInfo->cmdResponse;
            EPRINT(CAMERA_INTERFACE, "http error: [camera=%d], [requestCount=%d], [status=%s]", cameraIndex, requestCount, httpRespStr[HTTP_ERROR]);
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief httpSetDateTimeCb
 * @param httpHandle
 * @param dataInfo
 */
static void httpSetDateTimeCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo)
{
    UINT8 				cameraIndex = dataInfo->userData;
    CHARPTR				mainData = (CHARPTR)dataInfo->storagePtr;
    CHAR 				buf[20] = { '\0' };
    IP_CAMERA_CONFIG_t  ipCamCfg;

    switch(dataInfo->httpResponse)
    {
        case HTTP_SUCCESS:
        {
            ParseStr(&mainData, 0x0D, buf, sizeof(buf));
            if(0 == strcmp(buf, MATRIX_CAM_HTTP_RESP_CODE_7_STR))
            {
                setDateTimeRequest[cameraIndex].needToSetPwd = TRUE;
                EPRINT(CAMERA_INTERFACE, "http success but need to set password: [camera=%d]", cameraIndex);
            }
            else
            {
                setDateTimeRequest[cameraIndex].needToSetPwd = FALSE;
                DPRINT(CAMERA_INTERFACE, "http success: [camera=%d], [status=%s]", cameraIndex, httpRespStr[HTTP_SUCCESS]);
            }
        }
        break;

        case HTTP_CLOSE_ON_SUCCESS:
        {
            DPRINT(CAMERA_INTERFACE, "http success: [camera=%d], [status=%s]", cameraIndex, httpRespStr[HTTP_CLOSE_ON_SUCCESS]);
            if (cameraIndex >= getMaxCameraForCurrentVariant())
            {
                break;
            }

            if (FALSE == setDateTimeRequest[cameraIndex].needToSetPwd)
            {
                break;
            }

            setDateTimeRequest[cameraIndex].request.numOfRequest = 0;
            if (CMD_SUCCESS != GetCameraBrandModelUrl(cameraIndex, REQ_URL_SET_PASSWORD, &setDateTimeRequest[cameraIndex].request, "admin", "admin", NULL, NULL))
            {
                break;
            }

            ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);
            setDateTimeRequest[cameraIndex].request.requestCount = 0;
            setDateTimeRequest[cameraIndex].request.url[0].requestType = CAM_REQ_CONTROL;
            setDateTimeRequest[cameraIndex].request.httpCallback[CAM_REQ_CONTROL] = httpSetPasswordCb;
            sendReqToCamera(&setDateTimeRequest[cameraIndex].request, &ipCamCfg, MAIN_STREAM);
        }
        break;

        case HTTP_CLOSE_ON_ERROR:
        case HTTP_ERROR:
        {
            EPRINT(CAMERA_INTERFACE, "http error: [camera=%d], [status=%s]", cameraIndex, httpRespStr[dataInfo->httpResponse]);
        }
        break;

        default:
        {
            /* Nothing done */
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief httpSetPasswordCb
 * @param httpHandle
 * @param dataInfo
 */
static void httpSetPasswordCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo)
{

    UINT8                       cameraIndex = dataInfo->userData;
    CHARPTR                     mainData = (CHARPTR)dataInfo->storagePtr;
    CHAR                        buf[20] = { '\0' };
    CI_STREAM_EXT_TRG_PARAM_t	triggerParam;

    switch(dataInfo->httpResponse)
    {
        case HTTP_SUCCESS:
        {
            ParseStr(&mainData, 0x0D, buf, sizeof(buf));
            if(0 == strcmp(buf, MATRIX_CAM_HTTP_RESP_CODE_0_STR))
            {
                DPRINT(CAMERA_INTERFACE, "password created successfully: [camera=%d], [status=%s]", cameraIndex, httpRespStr[HTTP_SUCCESS]);
            }
            else
            {
                EPRINT(CAMERA_INTERFACE, "fail to create password: [camera=%d], [status=%s]", cameraIndex, httpRespStr[HTTP_SUCCESS]);
            }

            /* Send CAM_NOT_REACHABLE first to set camera in Ready state for further processing */
            triggerParam.camIndex = cameraIndex;
            triggerParam.connState = CAM_NOT_REACHABLE;
            triggerParam.type = CI_STREAM_EXT_TRG_CONNECTIVITY;
            ciStreamExtTriggerHandler(&triggerParam);
            triggerParam.camIndex += getMaxCameraForCurrentVariant();
            ciStreamExtTriggerHandler(&triggerParam);

            triggerParam.camIndex = cameraIndex;
            triggerParam.connState = CAM_REACHABLE;
            triggerParam.type = CI_STREAM_EXT_TRG_CONNECTIVITY;
            ciStreamExtTriggerHandler(&triggerParam);
            triggerParam.camIndex += getMaxCameraForCurrentVariant();
            ciStreamExtTriggerHandler(&triggerParam);
        }
        break;

        case HTTP_CLOSE_ON_SUCCESS:
        case HTTP_CLOSE_ON_ERROR:
        case HTTP_ERROR:
        {
            EPRINT(CAMERA_INTERFACE, "http error: [camera=%d], [status=%s]", cameraIndex, httpRespStr[dataInfo->httpResponse]);
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
 * @brief SetPtzPosition
 * @param cameraIndex
 * @param pan
 * @param tilt
 * @param zoom
 * @param speed
 * @param sessionIndex
 * @param callback
 * @param clientSocket
 * @return
 */
NET_CMD_STATUS_e SetPtzPosition(UINT8 cameraIndex, PTZ_OPTION_e pan, PTZ_OPTION_e tilt, PTZ_OPTION_e zoom, UINT8 speed,
                                UINT8 sessionIndex, NW_CMD_REPLY_CB callback, INT32 clientSocket)
{
    PTZ_MSG_t ptzMsg;

    if ((cameraIndex >= getMaxCameraForCurrentVariant()) ||
            ((pan >= MAX_PTZ_PAN_OPTION) && (tilt >= MAX_PTZ_TILT_OPTION) && (zoom >= MAX_PTZ_ZOOM_OPTION)))
    {
        EPRINT(CAMERA_INTERFACE, "invld params found: [camera=%d], [pan=%d], [tilt=%d], [zoom=%d]", cameraIndex, pan, tilt, zoom);
        return CMD_PROCESS_ERROR;
    }

    if (zoom == MAX_PTZ_ZOOM_OPTION)
    {
        ptzMsg.ptzFunction = PAN_TILT_FUNCTION;
    }
    else
    {
        ptzMsg.ptzFunction = ZOOM_FUNCTION;
    }

    DPRINT(CAMERA_INTERFACE, "ptz parameters: [camera=%d], [pan=%d], [tilt=%d], [zoom=%d], [speed=%d]", cameraIndex, pan, tilt, zoom, speed);
    ptzMsg.pan = pan;
    ptzMsg.tilt = tilt;
    ptzMsg.zoom = zoom;
    ptzMsg.focus = MAX_FOCUS_OPTION;
    ptzMsg.iris = MAX_IRIS_OPTION;
    ptzMsg.speed = speed;
    ptzMsg.cmdRespCallback = callback;
    ptzMsg.clientSocket = clientSocket;
    ptzMsg.action = START;

    /* Send ptz message to thread */
    return sendMsgToPtzCtrlThread(cameraIndex, sessionIndex, &ptzMsg);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief onvifSetPtzCb
 * @param responseData
 * @return SUCCESS
 */
static BOOL onvifSetPtzCb(ONVIF_RESPONSE_PARA_t *responseData)
{
    onvifCommonCtrlCb(CAM_REQ_STRUCT_PTZ, responseData);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief httpSetPtzControlCb
 * @param httpHandle
 * @param dataInfo
 */
static void httpSetPtzControlCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo)
{
    httpCommonControlCb(CAM_REQ_STRUCT_PTZ, httpHandle, dataInfo);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief httpSetOsdControlCb
 * @param httpHandle
 * @param dataInfo
 */
static void httpSetOsdControlCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo)
{
    httpCommonControlCb(CAM_REQ_STRUCT_SET_OSDS, httpHandle, dataInfo);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief httpSetMotionRequestCb
 * @param httpHandle
 * @param dataInfo
 */
static void httpSetMotionRequestCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo)
{
    UINT8               requestCount;
    UINT8               cameraIndex = (UINT8)(GET_STREAM_INDEX(dataInfo->userData));
    CAMERA_CONFIG_t     cameraCnfg;
    IP_CAMERA_CONFIG_t  ipCamCfg;
    BOOL                sessionClose = FALSE;

    switch(dataInfo->httpResponse)
    {
        case HTTP_CLOSE_ON_SUCCESS:
        case HTTP_CLOSE_ON_ERROR:
        {
            for (requestCount = 0; requestCount < motionWindowRequest[cameraIndex].request.numOfRequest; requestCount++)
            {
                if (motionWindowRequest[cameraIndex].request.http[requestCount] == httpHandle)
                {
                    requestCount++;
                    break;
                }
            }

            if (requestCount < motionWindowRequest[cameraIndex].request.numOfRequest)
            {
                motionWindowRequest[cameraIndex].request.requestCount++;
                motionWindowRequest[cameraIndex].request.http[requestCount-1] = INVALID_HTTP_HANDLE;

                ReadSingleIpCameraConfig(cameraIndex,&ipCamCfg);
                if (sendReqToCamera(&motionWindowRequest[cameraIndex].request, &ipCamCfg, MAIN_STREAM) != CMD_SUCCESS)
                {
                    sessionClose = TRUE;
                }
            }

            if ((sessionClose == FALSE) && (requestCount == motionWindowRequest[cameraIndex].request.numOfRequest))
            {
                motionWindowRequest[cameraIndex].request.requestStatus = CMD_SUCCESS;
                ReadSingleCameraConfig(cameraIndex, &cameraCnfg);

                /*updating motion detection status*/
                cameraCnfg.motionDetectionStatus = motionWindowRequest[cameraIndex].configStatus;

                pthread_rwlock_wrlock(&camCnfgNotifyControl[cameraIndex].configNotifyLock);
                camCnfgNotifyControl[cameraIndex].cameraConfigNotifyF = FALSE;
                pthread_rwlock_unlock(&camCnfgNotifyControl[cameraIndex].configNotifyLock);
                WriteSingleCameraConfig(cameraIndex,&cameraCnfg);
                pthread_rwlock_wrlock(&camCnfgNotifyControl[cameraIndex].configNotifyLock);
                camCnfgNotifyControl[cameraIndex].cameraConfigNotifyF = TRUE;
                pthread_rwlock_unlock(&camCnfgNotifyControl[cameraIndex].configNotifyLock);
                sessionClose = TRUE;
            }

            if (sessionClose == TRUE)
            {
                if (motionWindowRequest[cameraIndex].request.clientCb[ON] != NULL)
                {
                    ((NW_CMD_REPLY_CB)motionWindowRequest[cameraIndex].request.clientCb[ON])
                            (motionWindowRequest[cameraIndex].request.requestStatus, motionWindowRequest[cameraIndex].request.clientSocket[ON], TRUE);
                    motionWindowRequest[cameraIndex].request.clientCb[ON] = NULL;
                }

                restoreDefaultMotionConfig(cameraIndex);
                MUTEX_LOCK(motionWindowRequest[cameraIndex].request.camReqFlagLock);
                motionWindowRequest[cameraIndex].request.camReqBusyF = FREE;
                MUTEX_UNLOCK(motionWindowRequest[cameraIndex].request.camReqFlagLock);
            }
        }
        break;

        case HTTP_ERROR:
        case HTTP_SUCCESS:
        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief httpSetPrivacyMaskWindowRequestCb
 * @param httpHandle
 * @param dataInfo
 */
static void httpSetPrivacyMaskWindowRequestCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo)
{
    static UINT8        responseCode = 0;
    UINT8               requestCount;
    UINT8               cameraIndex = (UINT8)(GET_STREAM_INDEX(dataInfo->userData));
    CAMERA_CONFIG_t     cameraCnfg;
    IP_CAMERA_CONFIG_t  ipCamCfg;
    BOOL                sessionClose = FALSE;
    CHARPTR             token;
    CHARPTR             mainData = (CHARPTR)dataInfo->storagePtr;

    switch(dataInfo->httpResponse)
    {
        case HTTP_SUCCESS:
        {
            if (0 == strncmp(mainData, MATRIX_CAM_HTTP_RESP_CODE_STR, strlen(MATRIX_CAM_HTTP_RESP_CODE_STR)))
            {
                token = strchr(dataInfo->storagePtr, '=');
                if (NULL != token)
                {
                    token++;
                    responseCode = atoi(token);
                }
            }
        }
        break;

        case HTTP_CLOSE_ON_SUCCESS:
        case HTTP_CLOSE_ON_ERROR:
        {
            for (requestCount = 0; requestCount < privacyMaskRequest[cameraIndex].request.numOfRequest; requestCount++)
            {
                if (privacyMaskRequest[cameraIndex].request.http[requestCount] == httpHandle)
                {
                    requestCount++;
                    break;
                }
            }

            if (dataInfo->httpResponse == HTTP_CLOSE_ON_SUCCESS)
            {
                privacyMaskRequest[cameraIndex].request.requestStatus = CMD_SUCCESS;
            }
            else
            {
                privacyMaskRequest[cameraIndex].request.requestStatus = CMD_PROCESS_ERROR;
            }

            if (requestCount < privacyMaskRequest[cameraIndex].request.numOfRequest)
            {
                privacyMaskRequest[cameraIndex].request.requestCount++;
                privacyMaskRequest[cameraIndex].request.http[requestCount-1] = INVALID_HTTP_HANDLE;

                ReadSingleIpCameraConfig(cameraIndex,&ipCamCfg);
                if (sendReqToCamera(&privacyMaskRequest[cameraIndex].request, &ipCamCfg, MAIN_STREAM) != CMD_SUCCESS)
                {
                    sessionClose = TRUE;
                }
            }

            if ((sessionClose == FALSE ) && (requestCount == privacyMaskRequest[cameraIndex].request.numOfRequest))
            {
                if (responseCode != 0)
                {
                    privacyMaskRequest[cameraIndex].request.requestStatus = CMD_PROCESS_ERROR;
                }

                ReadSingleCameraConfig(cameraIndex, &cameraCnfg);
                if (privacyMaskRequest[cameraIndex].request.clientCb[ON] != NULL)
                {
                    cameraCnfg.privacyMaskStaus = TRUE;
                }

                pthread_rwlock_wrlock(&camCnfgNotifyControl[cameraIndex].configNotifyLock);
                camCnfgNotifyControl[cameraIndex].cameraConfigNotifyF = FALSE;
                pthread_rwlock_unlock(&camCnfgNotifyControl[cameraIndex].configNotifyLock);
                WriteSingleCameraConfig(cameraIndex, &cameraCnfg);
                pthread_rwlock_wrlock(&camCnfgNotifyControl[cameraIndex].configNotifyLock);
                camCnfgNotifyControl[cameraIndex].cameraConfigNotifyF = TRUE;
                pthread_rwlock_unlock(&camCnfgNotifyControl[cameraIndex].configNotifyLock);
                sessionClose = TRUE;
            }

            if (sessionClose == TRUE)
            {
                if (privacyMaskRequest[cameraIndex].request.clientCb[ON] != NULL)
                {
                    ((NW_CMD_REPLY_CB)privacyMaskRequest[cameraIndex].request.clientCb[ON])
                            (privacyMaskRequest[cameraIndex].request.requestStatus, privacyMaskRequest[cameraIndex].request.clientSocket[ON], TRUE);
                    privacyMaskRequest[cameraIndex].request.clientCb[ON] = NULL;
                }

                restoreDefaultPrivacyMaskConfig(cameraIndex);
                MUTEX_LOCK(privacyMaskRequest[cameraIndex].request.camReqFlagLock);
                privacyMaskRequest[cameraIndex].request.camReqBusyF = FREE;
                MUTEX_UNLOCK(privacyMaskRequest[cameraIndex].request.camReqFlagLock);
            }
        }
        break;

        default:
        {
            responseCode = 0;
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief httpGetMaxPrivacyMaskWindowRequestCb
 * @param httpHandle
 * @param dataInfo
 */
static void httpGetMaxPrivacyMaskWindowRequestCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo)
{
    NET_CMD_STATUS_e    requestStatus = CMD_PROCESS_ERROR;
    UINT8               cameraIndex = (UINT8)(GET_STREAM_INDEX(dataInfo->userData));
    CAMERA_BRAND_e      brand = 0;
    CAMERA_MODEL_e      model = 0;
    IP_CAMERA_CONFIG_t  ipCamCfg;
    const CHARPTR       maxPrivacyMaskStr = "maximum-privacy-mask=";
    CHARPTR             maxWinStrPtr = NULL;
    CHAR                countStr[2];

    switch(dataInfo->httpResponse)
    {
        case HTTP_SUCCESS:
        {
            do
            {
                /* Validate input param */
                if ((dataInfo->frameSize == 0) || (dataInfo->storagePtr == NULL))
                {
                    break;
                }

                /* Parse maximum supported privacy mask window count */
                if ((maxWinStrPtr = strstr(dataInfo->storagePtr, maxPrivacyMaskStr)) == NULL)
                {
                    break;
                }

                /* Get the camera brand/model and validate it */
                if (CMD_SUCCESS != GetAndValidateCameraBrandModel(cameraIndex, &brand, &model))
                {
                    EPRINT(CAMERA_INTERFACE, "invld brand and model: [camera=%d], [brand=%d], [model=%d]", cameraIndex, brand, model);
                    break;
                }

                /* Parse the number of privacy mask supported by the camera */
                strncpy(countStr, maxWinStrPtr + strlen(maxPrivacyMaskStr), 1);
                countStr[1] = '\0';
                privacyMaskRequest[cameraIndex].maxSupportedMaskWindow = atoi(countStr);

                /* Get max supported privacy mask from database */
                UINT8 maxSupportedMaskWindow = GetMaxSupportedPrivacyMaskWindow(brand, model);

                /* Get possible number of supported privacy masks by camera and NVR */
                if (privacyMaskRequest[cameraIndex].maxSupportedMaskWindow > maxSupportedMaskWindow)
                {
                    /* Restrict max mask to max buffer size */
                    privacyMaskRequest[cameraIndex].maxSupportedMaskWindow = maxSupportedMaskWindow;
                }

                requestStatus = GetBrandModelReqestUrl(cameraIndex, brand, model, REQ_URL_GET_PRIVACY_MASK,
                                                       &privacyMaskRequest[cameraIndex].request, NULL, NULL, NULL);
                if(requestStatus != CMD_SUCCESS)
                {
                    EPRINT(CAMERA_INTERFACE, "fail to get privacy mask url: [camera=%d], [brand=%d], [model=%d]", cameraIndex, brand, model);
                    break;
                }

                privacyMaskRequest[cameraIndex].request.requestCount = 0;
                privacyMaskRequest[cameraIndex].request.requestStatus = CMD_PROCESS_ERROR;
                ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);
                requestStatus =	sendReqToCamera(&privacyMaskRequest[cameraIndex].request, &ipCamCfg, MAIN_STREAM);

            }while(0);

            if (requestStatus != CMD_SUCCESS)
            {
                if (privacyMaskRequest[cameraIndex].request.clientCb[ON] != NULL)
                {
                    ((NW_CMD_REPLY_CB)privacyMaskRequest[cameraIndex].request.clientCb[ON])
                            (privacyMaskRequest[cameraIndex].request.requestStatus, privacyMaskRequest[cameraIndex].request.clientSocket[ON], TRUE);
                    privacyMaskRequest[cameraIndex].request.clientCb[ON] = NULL;
                }

                restoreDefaultPrivacyMaskConfig(cameraIndex);
                MUTEX_LOCK(privacyMaskRequest[cameraIndex].request.camReqFlagLock);
                privacyMaskRequest[cameraIndex].request.camReqBusyF = FREE;
                MUTEX_UNLOCK(privacyMaskRequest[cameraIndex].request.camReqFlagLock);
            }
        }
        break;

        case HTTP_CLOSE_ON_SUCCESS:
        case HTTP_CLOSE_ON_ERROR:
        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Add or remove ptz position
 * @param   cameraIndex
 * @param   ptzIndex
 * @param   presetAction
 * @param   pPtzPresetCfg
 */
static void storePtzPosition(UINT8 cameraIndex, UINT8 ptzIndex, BOOL presetAction, PTZ_PRESET_CONFIG_t *pPtzPresetCfg)
{
    NET_CMD_STATUS_e 	requestStatus = CMD_SUCCESS;
    IP_CAMERA_CONFIG_t 	ipCamCfg;
    PTZ_PRESET_INFO_t   presetInfo = {0};

    if((cameraIndex >= getMaxCameraForCurrentVariant()) || (ptzIndex >= MAX_PRESET_POSITION))
    {
        EPRINT(CAMERA_INTERFACE, "invld input parameter: [camera=%d], [ptzIndex=%d]", cameraIndex, ptzIndex);
        return;
    }

    DPRINT(CAMERA_INTERFACE, "store ptz received: [camera=%d], [ptzIndex=%d]", cameraIndex, ptzIndex);

    // Only one request will be served at a time. hence if busy send request status
    MUTEX_LOCK(storePtzRequest[cameraIndex].camReqFlagLock);
    if(storePtzRequest[cameraIndex].camReqBusyF == BUSY)
    {
        MUTEX_UNLOCK(storePtzRequest[cameraIndex].camReqFlagLock);
        WPRINT(CAMERA_INTERFACE, "store preset request is busy: [camera=%d], [ptzIndex=%d]", cameraIndex, ptzIndex);
        return;
    }
    storePtzRequest[cameraIndex].camReqBusyF = BUSY;
    MUTEX_UNLOCK(storePtzRequest[cameraIndex].camReqFlagLock);
    storePtzRequest[cameraIndex].clientCb[ON] = NULL;
    ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);

    if (ipCamCfg.onvifSupportF == TRUE)
    {
        if ((ipCamCfg.brand[0] == '\0') || (ipCamCfg.model[0] == '\0'))
        {
            EPRINT(CAMERA_INTERFACE, "onvif capability not recv, brand-model unknown: [camera=%d]", cameraIndex);
            requestStatus = CMD_ONVIF_CAM_CAPABILITY_ERROR;
        }
        else
        {
            storePtzRequest[cameraIndex].clientCb[ON] = NULL;
            requestStatus = sendOnvifActionCmd(cameraIndex, ONVIF_EDIT_PRESET, onvifStorePresetCb, &ipCamCfg, ptzIndex, presetAction, pPtzPresetCfg);
        }
    }
    else if (AUTO_ADDED_CAMERA == CameraType(cameraIndex))
    {
        presetInfo.action = presetAction;
        presetInfo.presetIndex = ptzIndex;
        snprintf(presetInfo.presetName, MAX_PRESET_POSITION_NAME_WIDTH, "%s", pPtzPresetCfg->name);

        GetCiRequestUrl(cameraIndex, REQ_URL_STORE_PTZ, 0, CAM_REQ_CONTROL, tcpStorePresetControlCb, &storePtzRequest[cameraIndex], &presetInfo);

        storePtzRequest[cameraIndex].requestCount = 0;
        storePtzRequest[cameraIndex].requestStatus = CMD_PROCESS_ERROR;
        requestStatus =	sendReqToCamera(&storePtzRequest[cameraIndex], &ipCamCfg, MAIN_STREAM);
    }
    else
    {
        ptzIndex += 1;
        requestStatus = GetCameraBrandModelUrl(cameraIndex, REQ_URL_STORE_PTZ, &storePtzRequest[cameraIndex], &ptzIndex, &presetAction, pPtzPresetCfg->name, NULL);

        if (requestStatus == CMD_SUCCESS)
        {
            storePtzRequest[cameraIndex].requestCount = 0;
            storePtzRequest[cameraIndex].requestStatus = CMD_PROCESS_ERROR;
            requestStatus =	sendReqToCamera(&storePtzRequest[cameraIndex], &ipCamCfg, MAIN_STREAM);
        }
    }

    // If request fails, reset request status
    if (requestStatus != CMD_SUCCESS)
    {
        MUTEX_LOCK(storePtzRequest[cameraIndex].camReqFlagLock);
        storePtzRequest[cameraIndex].camReqBusyF = FREE;
        MUTEX_UNLOCK(storePtzRequest[cameraIndex].camReqFlagLock);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief updatePtzPosInAllCfg
 * @param cameraIndex
 * @param ptzIndex
 * @return SUCCESS
 */
static BOOL updatePtzPosInAllCfg(UINT8 cameraIndex, UINT8 ptzIndex)
{
    BOOL					updateCfg = FALSE;
    UINT8					cnt;
    UINT8					tourPtzIdx;
    UINT8					evntIdx;
    PRESET_TOUR_CONFIG_t	tourCfg;
    CAMERA_EVENT_CONFIG_t	camEventCfg;
    SENSOR_EVENT_CONFIG_t	sensorEvntCfg;
    SYSTEM_EVENT_CONFIG_t	sysEvntCfg;

    ReadSinglePresetTourConfig(cameraIndex, &tourCfg);

    for(cnt = 0; cnt < MAX_TOUR_NUMBER; cnt++)
    {
        for(tourPtzIdx = 0; tourPtzIdx < MAX_ORDER_COUNT; tourPtzIdx++)
        {
            if(tourCfg.tour[cnt].ptz[tourPtzIdx].presetPosition == ptzIndex)
            {
                tourCfg.tour[cnt].ptz[tourPtzIdx].presetPosition = 0;
                updateCfg = TRUE;
            }
        }
    }

    if(updateCfg == TRUE)
    {
        updateCfg = FALSE;
        WriteSinglePresetTourConfig(cameraIndex, &tourCfg);
    }

    for(cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
    {
        for(evntIdx = 0; evntIdx < MAX_CAMERA_EVENT; evntIdx++)
        {
            ReadSingleCameraEventConfig(cnt,evntIdx, &camEventCfg);

            if ((camEventCfg.actionParam.gotoPosition.cameraNumber == GET_CAMERA_NO(cameraIndex)) && (camEventCfg.actionParam.gotoPosition.presetPosition == ptzIndex))
            {
                camEventCfg.actionParam.gotoPosition.presetPosition = 0;
                WriteSingleCameraEventConfig(cnt, evntIdx, &camEventCfg);
            }
        }
    }

    for(cnt = 0; cnt < MAX_SENSOR; cnt++)
    {
        ReadSingleSensorEventConfig(cnt, &sensorEvntCfg);

        if ((sensorEvntCfg.actionParam.gotoPosition.cameraNumber == GET_CAMERA_NO(cameraIndex)) && (sensorEvntCfg.actionParam.gotoPosition.presetPosition == ptzIndex))
        {
            sensorEvntCfg.actionParam.gotoPosition.presetPosition = 0;
            WriteSingleSensorEventConfig(cnt, &sensorEvntCfg);
        }
    }

    for(cnt = 0; cnt < MAX_SYSTEM_EVENT; cnt++)
    {
        ReadSingleSystemEventConfig(cnt, &sysEvntCfg);

        if ((sysEvntCfg.actionParam.gotoPosition.cameraNumber == GET_CAMERA_NO(cameraIndex)) && (sysEvntCfg.actionParam.gotoPosition.presetPosition == ptzIndex))
        {
            sysEvntCfg.actionParam.gotoPosition.presetPosition = 0;
            WriteSingleSystemEventConfig(cnt, &sysEvntCfg);
        }
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief onvifStorePresetCb
 * @param responseData
 * @return SUCCESS
 */
static BOOL onvifStorePresetCb(ONVIF_RESPONSE_PARA_t *responseData)
{
    onvifCommonCtrlCb(CAM_REQ_STRUCT_STORE_PRESET, responseData);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief httpStorePtzControlCb
 * @param httpHandle
 * @param dataInfo
 */
static void httpStorePtzControlCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo)
{
    httpCommonControlCb(CAM_REQ_STRUCT_STORE_PRESET, httpHandle, dataInfo);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GotoPtzPosition
 * @param cameraIndex
 * @param ptzIndex
 * @param callback
 * @param clientSocket
 * @param isPauseNeeded
 * @return Network command status
 */
NET_CMD_STATUS_e GotoPtzPosition(UINT8 cameraIndex,	UINT8 ptzIndex, NW_CMD_REPLY_CB callback, INT32 clientSocket, UINT8 isPauseNeeded)
{
    NET_CMD_STATUS_e        requestStatus = CMD_SUCCESS;
    IP_CAMERA_CONFIG_t      ipCamCfg;
    PRESET_TOUR_CONFIG_t    presetTourConfig;
    BOOL                    status = FALSE;

    requestStatus = ValidatePtzFuncSupport(cameraIndex, PAN_TILT_FUNCTION);

    if (requestStatus != CMD_SUCCESS)
    {
        return requestStatus;
    }

    if ((cameraIndex >= getMaxCameraForCurrentVariant()) || (ptzIndex == 0) || (ptzIndex > MAX_PRESET_POSITION))
    {
        EPRINT(CAMERA_INTERFACE, "invld input parameter: [camera=%d], [ptzIndex=%d]", cameraIndex, ptzIndex);
        return CMD_PROCESS_ERROR;
    }

    DPRINT(CAMERA_INTERFACE, "goto ptz received: [camera=%d], [ptzIndex=%d]", cameraIndex, ptzIndex);

    MUTEX_LOCK(gotoPtzRequest[cameraIndex].camReqFlagLock);
    if (gotoPtzRequest[cameraIndex].camReqBusyF == BUSY)
    {
        MUTEX_UNLOCK(gotoPtzRequest[cameraIndex].camReqFlagLock);
        WPRINT(CAMERA_INTERFACE, "goto preset request is busy: [camera=%d], [ptzIndex=%d]", cameraIndex, ptzIndex);
        return CMD_CAM_REQUEST_IN_PROCESS;
    }
    gotoPtzRequest[cameraIndex].camReqBusyF = BUSY;
    MUTEX_UNLOCK(gotoPtzRequest[cameraIndex].camReqFlagLock);

    ReadSinglePresetTourConfig(cameraIndex, &presetTourConfig);

    if ((presetTourConfig.activeTourOverride == ENABLE) && (isPauseNeeded == TRUE))
    {
        status = PausePtzTour(cameraIndex);
    }

    MUTEX_LOCK(ptzPauseState[cameraIndex].ptzPauseLock);
    ptzPauseState[cameraIndex].ptzFunctionPause = status;
    MUTEX_UNLOCK(ptzPauseState[cameraIndex].ptzPauseLock);

    gotoPtzRequest[cameraIndex].clientCb[ON] = callback;
    gotoPtzRequest[cameraIndex].clientSocket[ON] = clientSocket;
    ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);

    if (ipCamCfg.onvifSupportF == TRUE)
    {
        PTZ_PRESET_CONFIG_t ptzPresetCfg;
        ReadSinglePtzPresetConfig(cameraIndex, ptzIndex - 1, &ptzPresetCfg);
        requestStatus = sendOnvifActionCmd(cameraIndex, ONVIF_GOTO_PRESET, onvifGotoPresetCb, &ipCamCfg, ptzIndex - 1, FALSE, &ptzPresetCfg);
    }
    else if (CameraType(cameraIndex) == AUTO_ADDED_CAMERA)
    {
        GetCiRequestUrl(cameraIndex, REQ_URL_GO_TO_PTZ, (ptzIndex - 1), CAM_REQ_CONTROL, tcpGotoPresetControlCb, &gotoPtzRequest[cameraIndex], NULL);

        gotoPtzRequest[cameraIndex].requestCount = 0;
        gotoPtzRequest[cameraIndex].requestStatus = CMD_PROCESS_ERROR;
        requestStatus =	sendReqToCamera(&gotoPtzRequest[cameraIndex], &ipCamCfg, MAIN_STREAM);
    }
    else
    {
        requestStatus = GetCameraBrandModelUrl(cameraIndex, REQ_URL_GO_TO_PTZ, &gotoPtzRequest[cameraIndex], &ptzIndex, NULL, NULL, NULL);
        if (requestStatus == CMD_SUCCESS)
        {
            gotoPtzRequest[cameraIndex].requestCount = 0;
            gotoPtzRequest[cameraIndex].requestStatus = CMD_PROCESS_ERROR;
            requestStatus =	sendReqToCamera(&gotoPtzRequest[cameraIndex], &ipCamCfg, MAIN_STREAM);
        }
    }

    if (requestStatus != CMD_SUCCESS)
    {
        MUTEX_LOCK(gotoPtzRequest[cameraIndex].camReqFlagLock);
        gotoPtzRequest[cameraIndex].camReqBusyF = FREE;
        MUTEX_UNLOCK(gotoPtzRequest[cameraIndex].camReqFlagLock);
    }

    return(requestStatus);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief onvifGotoPresetCb
 * @param responseData
 * @return SUCCESS
 */
static BOOL onvifGotoPresetCb(ONVIF_RESPONSE_PARA_t *responseData)
{
    onvifCommonCtrlCb(CAM_REQ_STRUCT_GOTO_PRESET, responseData);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief httpGotoPtzControlCb
 * @param httpHandle
 * @param dataInfo
 */
static void httpGotoPtzControlCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo)
{
    httpCommonControlCb(CAM_REQ_STRUCT_GOTO_PRESET, httpHandle, dataInfo);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief SetFocus
 * @param cameraIndex
 * @param focus
 * @param speed
 * @param sessionIndex
 * @param callback
 * @param clientSocket
 * @return Network command status
 */
NET_CMD_STATUS_e SetFocus(UINT8 cameraIndex, CAMERA_FOCUS_e focus, UINT8 speed, UINT8 sessionIndex, NW_CMD_REPLY_CB callback, INT32 clientSocket)
{
    PTZ_MSG_t ptzMsg;

    if ((cameraIndex >= getMaxCameraForCurrentVariant()) || (focus >= MAX_FOCUS_OPTION))
    {
        EPRINT(CAMERA_INTERFACE, "invld input parameter: [camera=%d], [focus=%d]", cameraIndex, focus);
        return CMD_PROCESS_ERROR;
    }

    DPRINT(CAMERA_INTERFACE, "set focus received: [camera=%d], [focus=%d]", cameraIndex, focus);
    ptzMsg.ptzFunction = FOCUS_FUNCTION;
    ptzMsg.pan = MAX_PTZ_PAN_OPTION;
    ptzMsg.tilt = MAX_PTZ_TILT_OPTION;
    ptzMsg.zoom = MAX_PTZ_ZOOM_OPTION;
    ptzMsg.focus = focus;
    ptzMsg.action = START;
    ptzMsg.iris = MAX_IRIS_OPTION;
    ptzMsg.speed = speed;
    ptzMsg.cmdRespCallback = callback;
    ptzMsg.clientSocket = clientSocket;

    /* Send ptz message to thread */
    return sendMsgToPtzCtrlThread(cameraIndex, sessionIndex, &ptzMsg);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief onvifSetFocusCb
 * @param responseData
 * @return SUCCESS
 */
static BOOL onvifSetFocusCb(ONVIF_RESPONSE_PARA_t *responseData)
{
    onvifCommonCtrlCb(CAM_REQ_STRUCT_FOCUS, responseData);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief httpSetFocusControlCb
 * @param httpHandle
 * @param dataInfo
 */
static void httpSetFocusControlCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo)
{
    httpCommonControlCb(CAM_REQ_STRUCT_FOCUS, httpHandle, dataInfo);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief SetIris
 * @param cameraIndex
 * @param iris
 * @param sessionIndex
 * @param callback
 * @param clientSocket
 * @return Network command status
 */
NET_CMD_STATUS_e SetIris(UINT8 cameraIndex, CAMERA_IRIS_e iris, UINT8 sessionIndex, NW_CMD_REPLY_CB callback, INT32 clientSocket)
{
    PTZ_MSG_t ptzMsg;

    if ((cameraIndex >= getMaxCameraForCurrentVariant()) || (iris >= MAX_IRIS_OPTION))
    {
        EPRINT(CAMERA_INTERFACE, "invld input parameter: [camera=%d], [iris=%d]", cameraIndex, iris);
        return CMD_PROCESS_ERROR;
    }

    DPRINT(CAMERA_INTERFACE, "set iris received: [camera=%d], [iris=%d]", cameraIndex, iris);
    ptzMsg.ptzFunction = IRIS_FUNCTION;
    ptzMsg.pan = MAX_PTZ_PAN_OPTION;
    ptzMsg.tilt = MAX_PTZ_TILT_OPTION;
    ptzMsg.zoom = MAX_PTZ_ZOOM_OPTION;
    ptzMsg.focus = MAX_FOCUS_OPTION;
    ptzMsg.iris = iris;
    ptzMsg.action = START;
    ptzMsg.speed = 0;
    ptzMsg.cmdRespCallback = callback;
    ptzMsg.clientSocket = clientSocket;

    /* Send ptz message to thread */
    return sendMsgToPtzCtrlThread(cameraIndex, sessionIndex, &ptzMsg);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetProfileParam
 * @param cameraIndex
 * @param profileIndex
 * @param streamType
 * @param callback
 * @param clientSocket
 * @param paramReqType
 * @return Network command status
 */
NET_CMD_STATUS_e GetProfileParam(UINT8 cameraIndex, UINT8 profileIndex, VIDEO_TYPE_e streamType, NW_CMD_REPLY_CB callback,
                                 INT32 clientSocket, CLIENT_CB_TYPE_e clientCbType)
{
    NET_CMD_STATUS_e	requestStatus = CMD_SUCCESS;
    IP_CAMERA_CONFIG_t 	ipCamCfg;

    /* SET = Write in config in file & GET = Get config but dont write in config file */
    if ((cameraIndex >= getMaxCameraForCurrentVariant()) || (streamType >= MAX_STREAM))
    {
        EPRINT(CAMERA_INTERFACE, "invld input params: [camera=%d], [streamType=%d]", cameraIndex, streamType);
        return CMD_PROCESS_ERROR;
    }

    MUTEX_LOCK(cameraGetProfileParam[cameraIndex][streamType].camReqFlagLock);
    if (cameraGetProfileParam[cameraIndex][streamType].camReqBusyF == BUSY)
    {
        MUTEX_UNLOCK(cameraGetProfileParam[cameraIndex][streamType].camReqFlagLock);
        WPRINT(CAMERA_INTERFACE, "get profile param request already in-progress: [camera=%d], [streamType=%d], [profile=%d]", cameraIndex, streamType, profileIndex);
        return CMD_CAM_REQUEST_IN_PROCESS;
    }

    cameraGetProfileParam[cameraIndex][streamType].camReqBusyF = BUSY;
    MUTEX_UNLOCK(cameraGetProfileParam[cameraIndex][streamType].camReqFlagLock);
    cameraGetProfileParam[cameraIndex][streamType].clientCb[ON] = callback;
    cameraGetProfileParam[cameraIndex][streamType].clientSocket[ON] = clientSocket;
    cameraGetProfileParam[cameraIndex][streamType].clientCbType = clientCbType;
    ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);

    UINT8 complexCameraIndex = (cameraIndex + (streamType * getMaxCameraForCurrentVariant()));
    if (ipCamCfg.onvifSupportF == TRUE)
    {
        if ((ipCamCfg.brand[0] == '\0') || (ipCamCfg.model[0] == '\0'))
        {
            EPRINT(CAMERA_INTERFACE, "onvif capability not rcvd yet, brand and model unknown: [camera=%d], [streamType=%d]", cameraIndex, streamType);
            requestStatus = CMD_ONVIF_CAM_CAPABILITY_ERROR;
        }
        else
        {
            requestStatus = sendOnvifStreamCmd(complexCameraIndex, ONVIF_PROFILE_CONFIG, onvifGetProfileParamCb, &ipCamCfg, profileIndex, FALSE, NULL);
        }
    }
    else if (CameraType(cameraIndex) == AUTO_ADDED_CAMERA)
    {
        GetCiRequestUrl(cameraIndex, REQ_URL_GET_STREAM_CFG, (profileIndex - 1), CAM_REQ_CONFIG,
                        TcpGetStreamProfileCnfgCb, &cameraGetProfileParam[cameraIndex][streamType], NULL);
        if (FAIL == GetCurrentCameraProfileConfig(&cameraGetProfileParam[cameraIndex][streamType], complexCameraIndex, profileIndex, cameraConfigGetProfileParamCb))
        {
            EPRINT(CAMERA_INTERFACE, "fail to get auto added camera profile param: [camera=%d], [streamType=%d]", cameraIndex, streamType);
            requestStatus = CMD_PROCESS_ERROR;
        }
    }
    else
    {
        requestStatus = GetCameraBrandModelUrl(cameraIndex, REQ_URL_GET_STREAM_CFG, &cameraGetProfileParam[cameraIndex][streamType],
                                               &streamType, &profileIndex, NULL, NULL);
        if (requestStatus != CMD_SUCCESS)
        {
            EPRINT(CAMERA_INTERFACE, "fail to get brand-model camera profile param: [camera=%d], [streamType=%d], [status=%d]", cameraIndex, streamType, requestStatus);
        }
        else
        {
            cameraGetProfileParam[cameraIndex][streamType].requestCount = 0;
            cameraGetProfileParam[cameraIndex][streamType].requestStatus = CMD_PROCESS_ERROR;
            if (FAIL == GetCurrentCameraProfileConfig(&cameraGetProfileParam[cameraIndex][streamType], complexCameraIndex, profileIndex, cameraConfigGetProfileParamCb))
            {
                EPRINT(CAMERA_INTERFACE, "fail to get brand-model camera profile param: [camera=%d], [streamType=%d]", cameraIndex, streamType);
                requestStatus = CMD_PROCESS_ERROR;
            }
        }
    }

    if (requestStatus == CMD_SUCCESS)
    {
        DPRINT(CAMERA_INTERFACE, "get profile param request: [camera=%d], [streamType=%d], [profile=%d]", cameraIndex, streamType, profileIndex);
    }
    else
    {
        MUTEX_LOCK(cameraGetProfileParam[cameraIndex][streamType].camReqFlagLock);
        cameraGetProfileParam[cameraIndex][streamType].camReqBusyF = FREE;
        cameraGetProfileParam[cameraIndex][streamType].clientCb[ON] = NULL;
        MUTEX_UNLOCK(cameraGetProfileParam[cameraIndex][streamType].camReqFlagLock);
    }

    return requestStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief ChangeCamIpAddr
 * @param cameraIndex
 * @param pNetworkParam
 * @param callback
 * @param clientSocket
 * @param clientCbType
 * @return Network command status
 */
NET_CMD_STATUS_e ChangeCamIpAddr(UINT8 cameraIndex, IP_ADDR_PARAM_t *pNetworkParam,
                                 NW_CMD_REPLY_CB callback, INT32 clientSocket, CLIENT_CB_TYPE_e clientCbType)
{
    NET_CMD_STATUS_e	requestStatus = CMD_SUCCESS;
    IP_CAMERA_CONFIG_t 	ipCamCfg;

    if ((pNetworkParam == NULL) || (cameraIndex >= getMaxCameraForCurrentVariant()))
    {
        return CMD_PROCESS_ERROR;
    }

    if (pNetworkParam->ipAddrType == IP_ADDR_TYPE_IPV4)
    {
        if (pNetworkParam->prefixLen > 32)
        {
            return CMD_PROCESS_ERROR;
        }
    }
    else if (pNetworkParam->ipAddrType >= IP_ADDR_TYPE_IPV6)
    {
        if (pNetworkParam->prefixLen > 128)
        {
            return CMD_PROCESS_ERROR;
        }
    }
    else
    {
        return CMD_PROCESS_ERROR;
    }

    MUTEX_LOCK(changeCamIpRequest[cameraIndex].request.camReqFlagLock);
    if (changeCamIpRequest[cameraIndex].request.camReqBusyF == BUSY)
    {
        MUTEX_UNLOCK(changeCamIpRequest[cameraIndex].request.camReqFlagLock);
        return CMD_CAM_REQUEST_IN_PROCESS;
    }
    changeCamIpRequest[cameraIndex].request.camReqBusyF = BUSY;
    MUTEX_UNLOCK(changeCamIpRequest[cameraIndex].request.camReqFlagLock);

    memcpy(&changeCamIpRequest[cameraIndex].newNetworkParam, pNetworkParam, sizeof(IP_ADDR_PARAM_t));
    changeCamIpRequest[cameraIndex].request.clientCb[ON] = callback;
    changeCamIpRequest[cameraIndex].request.clientSocket[ON] = clientSocket;
    changeCamIpRequest[cameraIndex].request.clientCbType = clientCbType;
    ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);

    if (ipCamCfg.onvifSupportF == TRUE)
    {
        requestStatus = sendOnvifCommonCmd(cameraIndex, ONVIF_CHANGE_IP_ADDR, onvifChangeIpAddressCb, &ipCamCfg, &changeCamIpRequest[cameraIndex].newNetworkParam);
    }
    else
    {
        requestStatus = GetCameraBrandModelUrl(cameraIndex, REQ_URL_CHANGE_CAMERA_ADDR, &changeCamIpRequest[cameraIndex].request, pNetworkParam, NULL, NULL, NULL);
        if (requestStatus != CMD_SUCCESS)
        {
            EPRINT(CAMERA_INTERFACE, "fail to get url: [camera=%d]", cameraIndex);
        }
        else
        {
            changeCamIpRequest[cameraIndex].request.requestCount = 0;
            changeCamIpRequest[cameraIndex].request.requestStatus = CMD_PROCESS_ERROR;
            DPRINT(CAMERA_INTERFACE, "sending ip change request: [camera=%d]", cameraIndex);
            requestStatus = sendReqToCamera(&(changeCamIpRequest[cameraIndex].request),&ipCamCfg,MAIN_STREAM);
        }
    }

    if(requestStatus != CMD_SUCCESS)
    {
        MUTEX_LOCK(changeCamIpRequest[cameraIndex].request.camReqFlagLock);
        changeCamIpRequest[cameraIndex].request.camReqBusyF = FREE;
        MUTEX_UNLOCK(changeCamIpRequest[cameraIndex].request.camReqFlagLock);
    }

    return requestStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief onvifSetIrisCb
 * @param responseData
 * @return SUCCESS
 */
static BOOL onvifSetIrisCb(ONVIF_RESPONSE_PARA_t *responseData)
{
    onvifCommonCtrlCb(CAM_REQ_STRUCT_IRIS, responseData);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief onvifSetOsdCb
 * @param responseData
 * @return SUCCESS
 */
static BOOL onvifSetOsdCb(ONVIF_RESPONSE_PARA_t *responseData)
{
    onvifCommonCtrlCb(CAM_REQ_STRUCT_SET_OSDS, responseData);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief onvifGetProfileParamCb
 * @param responseData
 * @return SUCCESS
 */
static BOOL onvifGetProfileParamCb(ONVIF_RESPONSE_PARA_t *responseData)
{
    STREAM_CONFIG_t				streamConfig;
    ONVIF_PROFILE_STREAM_INFO_t	*profileStreamConfig = NULL;
    NET_CMD_STATUS_e			requestStatus = CMD_SUCCESS;
    UINT8 						profileIndexToSend = 1;

    if (responseData->response != ONVIF_CMD_SUCCESS)
    {
        requestStatus = (responseData->response == ONVIF_CMD_FEATURE_NOT_SUPPORTED) ? CMD_FEATURE_NOT_SUPPORTED : CMD_CAM_REQUEST_FAILED;
    }
    else
    {
        profileStreamConfig = (ONVIF_PROFILE_STREAM_INFO_t *)responseData->data;
        profileIndexToSend = profileStreamConfig->profileNum;
        DPRINT(CAMERA_INTERFACE, "got stream params: [camera=%d], [stream=%d], [profile=%d]",
               responseData->cameraIndex, GET_STREAM_TYPE(responseData->cameraIndex), profileIndexToSend);
    }

    ReadSingleStreamConfig(GET_STREAM_INDEX(responseData->cameraIndex), &streamConfig);
    if (requestStatus == CMD_SUCCESS)
    {
        if (GET_STREAM_TYPE(responseData->cameraIndex) == MAIN_STREAM)
        {
            snprintf(streamConfig.videoEncoding, MAX_ENCODER_NAME_LEN, "%s", profileStreamConfig->videoEncoding);
            snprintf(streamConfig.resolution, MAX_RESOLUTION_NAME_LEN, "%s", profileStreamConfig->resolution);
            streamConfig.framerate = profileStreamConfig->framerate;
            streamConfig.quality = profileStreamConfig->quality;
            streamConfig.bitrateMode = profileStreamConfig->bitrateMode;
            streamConfig.bitrateValue = profileStreamConfig->bitrateValue;
            streamConfig.gop = profileStreamConfig->gop;
            streamConfig.mainStreamProfile = profileIndexToSend;
        }
        else
        {
            snprintf(streamConfig.videoEncodingSub, MAX_ENCODER_NAME_LEN, "%s", profileStreamConfig->videoEncoding);
            snprintf(streamConfig.resolutionSub, MAX_RESOLUTION_NAME_LEN, "%s", profileStreamConfig->resolution);
            streamConfig.framerateSub = profileStreamConfig->framerate;
            streamConfig.qualitySub = profileStreamConfig->quality;
            streamConfig.bitrateModeSub = profileStreamConfig->bitrateMode;
            streamConfig.bitrateValueSub = profileStreamConfig->bitrateValue;
            streamConfig.gopSub = profileStreamConfig->gop;
            streamConfig.subStreamProfile = profileIndexToSend;
        }
    }

    sendProfileResponseToClient(responseData->cameraIndex, &streamConfig, profileIndexToSend, requestStatus);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief cameraConfigGetProfileParamCb
 * @param complexCamIndex
 * @param streamConfig
 * @param profileIndex
 * @return SUCCESS
 */
static BOOL cameraConfigGetProfileParamCb(UINT8 complexCamIndex, STREAM_CONFIG_t *streamConfig, UINT8 profileIndex)
{
    STREAM_CONFIG_t streamCfg;
    UINT8           cameraIndex = GET_STREAM_INDEX(complexCamIndex);
    VIDEO_TYPE_e    streamType = GET_STREAM_TYPE(complexCamIndex);

    ReadSingleStreamConfig(cameraIndex, &streamCfg);
    if(streamType == MAIN_STREAM)
    {
        snprintf(streamCfg.videoEncoding, MAX_ENCODER_NAME_LEN, "%s", streamConfig->videoEncoding);
        snprintf(streamCfg.resolution, MAX_RESOLUTION_NAME_LEN, "%s", streamConfig->resolution);
        streamCfg.framerate = streamConfig->framerate;
        streamCfg.quality = streamConfig->quality;
        streamCfg.enableAudio = streamConfig->enableAudio;
        streamCfg.bitrateMode = streamConfig->bitrateMode;
        streamCfg.bitrateValue = streamConfig->bitrateValue;
        streamCfg.gop = streamConfig->gop;
        streamCfg.mainStreamProfile = streamConfig->mainStreamProfile;
    }
    else
    {
        snprintf(streamCfg.videoEncodingSub, MAX_ENCODER_NAME_LEN, "%s", streamConfig->videoEncodingSub);
        snprintf(streamCfg.resolutionSub, MAX_RESOLUTION_NAME_LEN, "%s", streamConfig->resolutionSub);
        streamCfg.framerateSub = streamConfig->framerateSub;
        streamCfg.qualitySub = streamConfig->qualitySub;
        streamCfg.enableAudioSub = streamConfig->enableAudioSub;
        streamCfg.bitrateModeSub = streamConfig->bitrateModeSub;
        streamCfg.bitrateValueSub = streamConfig->bitrateValueSub;
        streamCfg.gopSub = streamConfig->gopSub;
        streamCfg.subStreamProfile = streamConfig->subStreamProfile;
    }

    sendProfileResponseToClient(complexCamIndex, &streamCfg, profileIndex, CMD_SUCCESS);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief sendChangeCamIpAddrResponseToClient
 * @param cameraIndex
 * @param response
 * @return SUCCESS/FAIL
 */
static BOOL sendChangeCamIpAddrResponseToClient(UINT8 cameraIndex, NET_CMD_STATUS_e response)
{
    if (cameraIndex >= getMaxCameraForCurrentVariant())
    {
        return FAIL;
    }

    if (changeCamIpRequest[cameraIndex].request.clientSocket == NULL)
    {
        return FAIL;
    }

    CHAR                respStringPtr[MAX_REPLY_SZ];
    UINT32              outLen = snprintf(respStringPtr, MAX_REPLY_SZ, "%c%s%c%d%c%c", SOM, headerReq[RPL_CMD], FSP, response, FSP, EOM);
    CLIENT_CB_TYPE_e    clientCbType = changeCamIpRequest[cameraIndex].request.clientCbType;

    sendCmdCb[clientCbType](changeCamIpRequest[cameraIndex].request.clientSocket[ON], (UINT8PTR)respStringPtr, outLen, MESSAGE_REPLY_TIMEOUT);
    closeConnCb[clientCbType](&changeCamIpRequest[cameraIndex].request.clientSocket[ON]);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief sendProfileResponseToClient
 * @param complexCamIndex
 * @param streamConfig
 * @param profileIndex
 * @param response
 * @return SUCCESS/FAIL
 */
static BOOL sendProfileResponseToClient(UINT8 complexCamIndex, STREAM_CONFIG_t *streamConfig, UINT8 profileIndex, NET_CMD_STATUS_e response)
{
    UINT8           cameraIndex = GET_STREAM_INDEX(complexCamIndex);
    VIDEO_TYPE_e    streamType = GET_STREAM_TYPE(complexCamIndex);
    UINT8           noOfProfileSupported;

    /* Check camera index. It should be in valid range */
    if ((cameraIndex >= getMaxCameraForCurrentVariant()) || (streamType >= MAX_STREAM))
    {
        EPRINT(CAMERA_INTERFACE, "invld input params: [camera=%d], [streamType=%d]", cameraIndex, streamType);
        return FAIL;
    }

    if (cameraGetProfileParam[cameraIndex][streamType].clientCb[ON] != NULL)
    {
        CHAR    respStringPtr[MAX_REPLY_SZ];
        UINT32  outLen = snprintf(respStringPtr, MAX_REPLY_SZ, "%c%s%c%d%c", SOM, headerReq[RPL_CMD], FSP, response, FSP);
        if(outLen > MAX_REPLY_SZ)
        {
            EPRINT(CAMERA_INTERFACE, "more buffer required: [camera=%d], [outLen=%d]", cameraIndex, outLen);
            outLen = MAX_REPLY_SZ;
        }

        if (response == CMD_SUCCESS)
        {
            outLen += snprintf(respStringPtr + outLen, sizeof(respStringPtr) - outLen, "%c%d%c%d%c%d%c%d",
                               SOI, 1/*IndexId*/, FSP, cameraIndex, FSP, profileIndex, FSP, streamType);
            if(outLen > MAX_REPLY_SZ)
            {
                EPRINT(CAMERA_INTERFACE, "more buffer required: [camera=%d], [outLen=%d]", cameraIndex, outLen);
                outLen = MAX_REPLY_SZ;
            }

            if(streamType == MAIN_STREAM)
            {
                outLen += snprintf(respStringPtr + outLen, sizeof(respStringPtr) - outLen, "%c%s%c%s%c%d%c%d%c%d%c%d%c%d%c%d%c%c",
                                   FSP, streamConfig->videoEncoding, FSP, streamConfig->resolution, FSP, streamConfig->framerate,
                                   FSP, streamConfig->bitrateMode, FSP, streamConfig->bitrateValue, FSP, streamConfig->quality,
                                   FSP, streamConfig->gop, FSP, streamConfig->enableAudio, FSP, EOI);
            }
            else if(streamType == SUB_STREAM)
            {
                outLen += snprintf(respStringPtr + outLen, sizeof(respStringPtr) - outLen, "%c%s%c%s%c%d%c%d%c%d%c%d%c%d%c%d%c%c",
                                   FSP, streamConfig->videoEncodingSub, FSP, streamConfig->resolutionSub, FSP, streamConfig->framerateSub,
                                   FSP, streamConfig->bitrateModeSub, FSP, streamConfig->bitrateValueSub, FSP, streamConfig->qualitySub,
                                   FSP, streamConfig->gopSub, FSP, streamConfig->enableAudioSub, FSP, EOI);
            }
        }

        //Validate buffer for add last two character including NULL
        if(outLen > MAX_REPLY_SZ - 2)
        {
            EPRINT(CAMERA_INTERFACE, "more buffer required: [camera=%d], [outLen=%d]", cameraIndex, outLen);
            outLen = MAX_REPLY_SZ - 2;
        }

        outLen += snprintf(respStringPtr + outLen, sizeof(respStringPtr) - outLen, "%c", EOM);
        if (outLen > MAX_REPLY_SZ)
        {
            EPRINT(CAMERA_INTERFACE, "more buffer required: [camera=%d], [outLen=%d]", cameraIndex, outLen);
            outLen = MAX_REPLY_SZ;
        }

        CLIENT_CB_TYPE_e clientCbType = cameraGetProfileParam[cameraIndex][streamType].clientCbType;
        sendCmdCb[clientCbType](cameraGetProfileParam[cameraIndex][streamType].clientSocket[ON], (UINT8PTR)respStringPtr, outLen, MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&cameraGetProfileParam[cameraIndex][streamType].clientSocket[ON]);

        // set network command callback null. so, that if intrernally this callback is called, then we have to store config in fiLE
        cameraGetProfileParam[cameraIndex][streamType].clientCb[ON] = NULL;
    }
    else
    {
        if(response == CMD_SUCCESS)
        {
            pthread_rwlock_wrlock(&camConfigInfo[cameraIndex].configInfoLock);
            camConfigInfo[cameraIndex].configChangeNotify = FALSE;
            pthread_rwlock_unlock(&camConfigInfo[cameraIndex].configInfoLock);

            WriteSingleStreamConfig(cameraIndex, streamConfig);

            /* Checking for max supported profiles. if camera supports only one profile then copy main stream parameters to the substream.
             * Sometimes camera misbehaves in ONVIF and provides only one profile in GetProfiles API during scheduled(15minutes) parameters fetch.
             * Due to that, main stream was copied into substream. To avoid that, We have validated substream config before copy from main to sub.
             * It will be copied only if substream config is not valid */
            if ((streamType == MAIN_STREAM) && ((OnvifSupportF(cameraIndex) == FALSE) || (FALSE == isStreamConfigValid(streamConfig, SUB_STREAM))))
            {
                GetMaxSupportedProfile(cameraIndex, &noOfProfileSupported);
            }

            MUTEX_LOCK(profileShedularProcParam[cameraIndex][streamType].profileSchedularProcRunLock);
            BOOL tempProfShedularFlag = profileShedularProcParam[cameraIndex][streamType].profileSchedularProcRunF;
            MUTEX_UNLOCK(profileShedularProcParam[cameraIndex][streamType].profileSchedularProcRunLock);
            if(tempProfShedularFlag)
            {
                if(OnvifSupportF(cameraIndex) == FALSE)
                {
                    MUTEX_LOCK(profileShedularProcParam[cameraIndex][streamType].profileSchedularProcRunLock);
                    profileShedularProcParam[cameraIndex][streamType].profileSchedularProcRunF = FALSE;
                    MUTEX_UNLOCK(profileShedularProcParam[cameraIndex][streamType].profileSchedularProcRunLock);
                }
                else
                {
                    if(sendOnvifCommonCmd(complexCamIndex, ONVIF_MEDIA_PROFILES_UPDATE, onvifMediaProfileUpdateCb, NULL, NULL) != CMD_SUCCESS)
                    {
                        MUTEX_LOCK(profileShedularProcParam[cameraIndex][streamType].profileSchedularProcRunLock);
                        profileShedularProcParam[cameraIndex][streamType].profileSchedularProcRunF = FALSE;
                        MUTEX_UNLOCK(profileShedularProcParam[cameraIndex][streamType].profileSchedularProcRunLock);
                    }
                }
            }
        }
    }

    pthread_rwlock_wrlock(&camConfigInfo[cameraIndex].configInfoLock);
    camConfigInfo[cameraIndex].configChangeNotify = TRUE;
    pthread_rwlock_unlock(&camConfigInfo[cameraIndex].configInfoLock);

    MUTEX_LOCK(cameraGetProfileParam[cameraIndex][streamType].camReqFlagLock);
    cameraGetProfileParam[cameraIndex][streamType].camReqBusyF = FREE;
    MUTEX_UNLOCK(cameraGetProfileParam[cameraIndex][streamType].camReqFlagLock);

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief onvifGetMotionWindowCb
 * @param responseData
 * @return SUCCESS
 */
static BOOL onvifGetMotionWindowCb(ONVIF_RESPONSE_PARA_t *responseData)
{
    UINT8 cameraIndex = GET_STREAM_INDEX(responseData->cameraIndex);

    if (responseData->response != ONVIF_CMD_SUCCESS)
    {
        memset(&motionWindowRequest[cameraIndex].motionParam, 0, sizeof(motionWindowRequest[cameraIndex].motionParam));
        motionWindowRequest[cameraIndex].motionParam.sensitivity = 5;
        motionWindowRequest[cameraIndex].motionParam.noMotionDuration = 5;
        if (motionWindowRequest[cameraIndex].request.clientCb[ON] != NULL)
        {
            ((NW_CMD_REPLY_CB)motionWindowRequest[cameraIndex].request.clientCb[ON])
                    (CMD_PROCESS_ERROR, motionWindowRequest[cameraIndex].request.clientSocket[ON], TRUE);
            motionWindowRequest[cameraIndex].request.clientCb[ON] = NULL;
        }
    }
    else
    {
        if (motionWindowRequest[cameraIndex].request.clientCb[ON] != NULL)
        {
            /* Send motion area config response to client */
            sendGetMotionWindowResponseToClient(cameraIndex, CMD_SUCCESS);
            motionWindowRequest[cameraIndex].request.clientCb[ON] = NULL;
        }
    }

    restoreDefaultMotionConfig(cameraIndex);
    MUTEX_LOCK(motionWindowRequest[cameraIndex].request.camReqFlagLock);
    motionWindowRequest[cameraIndex].request.camReqBusyF = FREE;
    MUTEX_UNLOCK(motionWindowRequest[cameraIndex].request.camReqFlagLock);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief onvifSetMotionWindowCb
 * @param responseData
 * @return SUCCESS
 */
static BOOL onvifSetMotionWindowCb(ONVIF_RESPONSE_PARA_t *responseData)
{
    CHAR                respStringPtr[MAX_REPLY_SZ];
    NET_CMD_STATUS_e    response = CMD_PROCESS_ERROR;
    UINT8               cameraIndex = responseData->cameraIndex;
    CAMERA_CONFIG_t     cameraCnfg;

    if (responseData->response != ONVIF_CMD_SUCCESS)
    {
        if (responseData->response == ONVIF_CMD_FEATURE_NOT_SUPPORTED)
        {
            response = CMD_FEATURE_NOT_SUPPORTED;
        }
        else
        {
            response = CMD_CAM_REQUEST_FAILED;
        }
    }
    else
    {
        response = CMD_SUCCESS;
        motionWindowRequest[cameraIndex].request.requestStatus = CMD_SUCCESS;
        ReadSingleCameraConfig(cameraIndex, &cameraCnfg);

        /*updating motion detection status*/
        cameraCnfg.motionDetectionStatus = motionWindowRequest[cameraIndex].configStatus;

        pthread_rwlock_wrlock(&camCnfgNotifyControl[cameraIndex].configNotifyLock);
        camCnfgNotifyControl[cameraIndex].cameraConfigNotifyF = FALSE;
        pthread_rwlock_unlock(&camCnfgNotifyControl[cameraIndex].configNotifyLock);
        WriteSingleCameraConfig(cameraIndex,&cameraCnfg);
        pthread_rwlock_wrlock(&camCnfgNotifyControl[cameraIndex].configNotifyLock);
        camCnfgNotifyControl[cameraIndex].cameraConfigNotifyF = TRUE;
        pthread_rwlock_unlock(&camCnfgNotifyControl[cameraIndex].configNotifyLock);
    }

    if (motionWindowRequest[responseData->cameraIndex].request.clientCb[ON] != NULL)
    {
        UINT32 outLen = snprintf(respStringPtr, MAX_REPLY_SZ, "%c%s%c%03d%c%c", SOM, headerReq[RPL_CMD], FSP, response, FSP, EOM);
        CLIENT_CB_TYPE_e clientCbType = motionWindowRequest[responseData->cameraIndex].request.clientCbType;
        sendCmdCb[clientCbType](motionWindowRequest[responseData->cameraIndex].request.clientSocket[ON],
                (UINT8PTR)respStringPtr, outLen, MESSAGE_REPLY_TIMEOUT);
        closeConnCb[clientCbType](&motionWindowRequest[responseData->cameraIndex].request.clientSocket[ON]);
        motionWindowRequest[responseData->cameraIndex].request.clientCb[ON] = NULL;
    }

    restoreDefaultMotionConfig(responseData->cameraIndex);
    MUTEX_LOCK(motionWindowRequest[responseData->cameraIndex].request.camReqFlagLock);
    motionWindowRequest[responseData->cameraIndex].request.camReqBusyF = FREE;
    MUTEX_UNLOCK(motionWindowRequest[responseData->cameraIndex].request.camReqFlagLock);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief onvifChangeIpAddressCb
 * @param responseData
 * @return SUCCESS
 */
static BOOL onvifChangeIpAddressCb(ONVIF_RESPONSE_PARA_t *responseData)
{
    NET_CMD_STATUS_e    response = CMD_SUCCESS;
    IP_CAMERA_CONFIG_t  ipCamConfig;
    BOOL                writeConfig = FALSE;

    /* Check camera index. It should be in valid range */
    if (responseData->cameraIndex >= getMaxCameraForCurrentVariant())
    {
        DPRINT(CAMERA_INTERFACE, "invld camera index found: [camera=%d]", responseData->cameraIndex);
        return FALSE;
    }

    switch(responseData->response)
    {
        case ONVIF_CMD_SUCCESS:
            writeConfig = TRUE;
            break;

        case ONVIF_CMD_FAIL:
        case ONVIF_CMD_TIMEOUT:
        case ONVIF_CMD_RETRY:
            response = CMD_CAM_REQUEST_FAILED;
            break;

        case ONVIF_CMD_FEATURE_NOT_SUPPORTED:
            response = CMD_FEATURE_NOT_SUPPORTED;
            break;

        case ONVIF_CMD_IP_CHANGE_FAIL:
        case ONVIF_CMD_AUTHENTICATION_FAIL:
            response = CMD_IP_CHANGE_FAIL;
            break;

        case ONVIF_CMD_CAMERA_REBOOT_FAIL:
            response = CMD_REBOOT_FAIL;
            writeConfig = TRUE;
            break;

        default:
            response = CMD_CAM_REQUEST_FAILED;
            break;
    }

    if(writeConfig == TRUE)
    {
        ReadSingleIpCameraConfig(responseData->cameraIndex, &ipCamConfig);
        snprintf(ipCamConfig.cameraAddress, MAX_CAMERA_ADDRESS_WIDTH, "%s", changeCamIpRequest[responseData->cameraIndex].newNetworkParam.ipAddr);
        WriteSingleIpCameraConfig(responseData->cameraIndex, &ipCamConfig);
    }

    sendChangeCamIpAddrResponseToClient(responseData->cameraIndex,response);
    restoreDfltCamIpChangeReqParam(responseData->cameraIndex);

    MUTEX_LOCK(changeCamIpRequest[responseData->cameraIndex].request.camReqFlagLock);
    changeCamIpRequest[responseData->cameraIndex].request.camReqBusyF = FREE;
    MUTEX_UNLOCK(changeCamIpRequest[responseData->cameraIndex].request.camReqFlagLock);

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief httpSetIrisControlCb
 * @param httpHandle
 * @param dataInfo
 */
static void httpSetIrisControlCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo)
{
    httpCommonControlCb(CAM_REQ_STRUCT_IRIS, httpHandle, dataInfo);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief NotifyAlarmConfigChange
 * @param newAlarmConfig
 * @param oldAlarmConfig
 * @param cameraIndex
 * @param alarmIndex
 */
void NotifyAlarmConfigChange(CAMERA_ALARM_CONFIG_t newAlarmConfig, CAMERA_ALARM_CONFIG_t *oldAlarmConfig, UINT8 cameraIndex, UINT8 alarmIndex)
{
    DPRINT(CAMERA_INTERFACE, "alarm config changed: [camera=%d], [alarmIndex=%d]", cameraIndex, alarmIndex);

    // If alarm is activated, we need to de-activate it if alarm mode is changed
    MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].counterLock);
    if (alarmActionRequest[cameraIndex][alarmIndex].requestCounter == 0)
    {
        MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].counterLock);
        return;
    }
    MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].counterLock);

    // If alarm active mode changes, In-activate respective alarm
    if (oldAlarmConfig->activeMode == newAlarmConfig.activeMode)
    {
        return;
    }

    // If alarm was pulse previously and timer is running, delete timer
    if ((oldAlarmConfig->activeMode == ALARM_PULSE) && (alarmActionRequest[cameraIndex][alarmIndex].timerHandle != INVALID_TIMER_HANDLE))
    {
        DeleteTimer(&alarmActionRequest[cameraIndex][alarmIndex].timerHandle);
        alarmActionRequest[cameraIndex][alarmIndex].timerHandle = INVALID_TIMER_HANDLE;
    }

    MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].counterLock);
    alarmActionRequest[cameraIndex][alarmIndex].requestCounter = 0;
    MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].counterLock);

    CameraAlarmAction(cameraIndex, alarmIndex, INACTIVE, NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief CameraAlarmAction
 * @param cameraIndex
 * @param alarmIndex
 * @param alarmAction
 * @param callback
 */
void CameraAlarmAction(UINT8 cameraIndex, UINT8 alarmIndex, BOOL alarmAction, ALARM_ACTION_REQUEST_CB callback)
{
    BOOL					otherAction;
    NET_CMD_STATUS_e 		requestStatus = CMD_SUCCESS;
    CAMERA_TYPE_e 			camType = MAX_CAMERA_TYPE;
    TIMER_INFO_t 			timerInfo;
    CAMERA_BRAND_e			brand = CAMERA_BRAND_NONE;
    CAMERA_MODEL_e			model = CAMERA_MODEL_NONE;
    IP_CAMERA_CONFIG_t 		ipCamCfg;
    CAMERA_ALARM_CONFIG_t 	cameraAlarmConfig;
    CHAR					detail[MAX_EVENT_DETAIL_SIZE];

    if ((cameraIndex >= getMaxCameraForCurrentVariant()) || (alarmIndex >= MAX_CAMERA_ALARM))
    {
        EPRINT(CAMERA_INTERFACE, "invld input params: [camera=%d], [alarm=%d]", cameraIndex, alarmIndex);
        return;
    }

    camType = CameraType(cameraIndex);
    do
    {
        if((camType != IP_CAMERA) && (camType != AUTO_ADDED_CAMERA))
        {
            requestStatus = CMD_FEATURE_NOT_SUPPORTED;
            break;
        }

        if(camType != AUTO_ADDED_CAMERA)
        {
            GetCameraBrandModelNumber(cameraIndex, &brand, &model);
            if (brand <= CAMERA_BRAND_GENERIC)
            {
                requestStatus = CMD_FEATURE_NOT_SUPPORTED;
                break;
            }
        }

        ReadSingleCameraAlarmConfig(cameraIndex, alarmIndex, &cameraAlarmConfig);
        switch(alarmAction)
        {
            case ACTIVE:
            {
                if ((cameraAlarmConfig.activeMode == ALARM_PULSE) && (alarmActionRequest[cameraIndex][alarmIndex].timerHandle != INVALID_TIMER_HANDLE))
                {
                    DPRINT(CAMERA_INTERFACE, "alarm is in pulse mode and timer is running: [camera=%d], [alarmIndex=%d]", cameraIndex, alarmIndex);
                    return;
                }

                // If alarm request is for ACTIVE and INACTIVE is busy, make ACTIVE pending register callback for it
                MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                if(alarmActionRequest[cameraIndex][alarmIndex].reqStatus[INACTIVE] == BUSY)
                {
                    alarmActionRequest[cameraIndex][alarmIndex].reqStatus[ACTIVE] = PENDING;
                    MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                    alarmActionRequest[cameraIndex][alarmIndex].request.clientCb[ACTIVE] = callback;
                    DPRINT(CAMERA_INTERFACE, "alarm deactivate is busy, make activate pending: [camera=%d], [alarmIndex=%d]", cameraIndex, alarmIndex);
                    return;
                }

                // If request to activate alarm is busy, send resource limit as only one request will be served at a time
                if(alarmActionRequest[cameraIndex][alarmIndex].reqStatus[ACTIVE] == BUSY)
                {
                    MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                    if (callback != NULL)
                    {
                        callback(cameraIndex, alarmIndex, CMD_RESOURCE_LIMIT);
                    }
                    DPRINT(CAMERA_INTERFACE, "alarm activate in progress: [camera=%d], [alarmIndex=%d]", cameraIndex, alarmIndex);
                    return;
                }

                // If both are free, set alarm request status and register callback
                alarmActionRequest[cameraIndex][alarmIndex].reqStatus[ACTIVE] = BUSY;
                MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                alarmActionRequest[cameraIndex][alarmIndex].request.clientCb[ACTIVE] = callback;
                DPRINT(CAMERA_INTERFACE, "alarm activate request received: [camera=%d], [alarmIndex=%d]", cameraIndex, alarmIndex);

                switch(cameraAlarmConfig.activeMode)
                {
                    case ALARM_INTERLOCK:
                    {
                        DPRINT(CAMERA_INTERFACE, "alarm interlock mode: [camera=%d], [alarmIndex=%d]", cameraIndex, alarmIndex);

                        // If request count for activating alarm out of the camera is zero it is first request, hence send command to camera
                        MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].counterLock);
                        if(alarmActionRequest[cameraIndex][alarmIndex].requestCounter == 0)
                        {
                            MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].counterLock);
                            ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);
                            if(ipCamCfg.onvifSupportF == TRUE)
                            {
                                requestStatus = sendOnvifActionCmd(cameraIndex, ONVIF_SET_ALARM, onvifActivateAlarmCb, &ipCamCfg, alarmIndex, alarmAction, NULL);
                            }
                            else if (camType == AUTO_ADDED_CAMERA)
                            {
                                GetCiRequestUrl(cameraIndex, REQ_URL_SET_ALARM, ACTIVE, CAM_ALARM_ACTIVE,
                                                tcpCamAlarmOutCb, &alarmActionRequest[cameraIndex][alarmIndex].request, &alarmIndex);
                                alarmActionRequest[cameraIndex][alarmIndex].request.requestStatus = CMD_PROCESS_ERROR;
                                alarmActionRequest[cameraIndex][alarmIndex].request.requestCount = 0;
                                requestStatus =	sendReqToCamera(&alarmActionRequest[cameraIndex][alarmIndex].request, &ipCamCfg, MAIN_STREAM);
                            }
                            else
                            {
                                requestStatus = requestCameraAlarm(cameraIndex, alarmIndex,	&ipCamCfg, alarmAction);
                            }

                            // If request fails, reset necessary variables
                            if(requestStatus != CMD_SUCCESS)
                            {
                                EPRINT(CAMERA_INTERFACE, "alarm activate fail: [camera=%d], [alarmIndex=%d]", cameraIndex, alarmIndex);

                                // Decrement request count
                                MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].counterLock);
                                alarmActionRequest[cameraIndex][alarmIndex].requestCounter = 0;
                                MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].counterLock);

                                // reset alarm activate request status of the camera
                                MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                                alarmActionRequest[cameraIndex][alarmIndex].reqStatus[ACTIVE] = FREE;
                                MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                            }
                            else
                            {
                                callback = NULL;
                                snprintf(detail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(cameraIndex));
                                WriteEvent(LOG_CAMERA_EVENT, (LOG_CAMERA_ALARM_1 + alarmIndex), detail, NULL, alarmAction);
                            }
                        }
                        // If request count is not zero, it is already activated hence no need to send request
                        else
                        {
                            // Increment request count
                            alarmActionRequest[cameraIndex][alarmIndex].requestCounter++;
                            MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].counterLock);

                            // Reset alarm activate status flag
                            MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                            alarmActionRequest[cameraIndex][alarmIndex].reqStatus[ACTIVE] = FREE;
                            MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                            DPRINT(CAMERA_INTERFACE, "alarm already activated: [camera=%d], [alarmIndex=%d]", cameraIndex, alarmIndex);
                        }
                    }
                    break;

                    case ALARM_PULSE:
                    {
                        DPRINT(CAMERA_INTERFACE, "alarm pulse mode: [camera=%d], [alarmIndex=%d]", cameraIndex, alarmIndex);

                        // If pulse timer of the camera is invalid, then it is first request
                        if(alarmActionRequest[cameraIndex][alarmIndex].timerHandle == INVALID_TIMER_HANDLE)
                        {
                            // Start pulse timer
                            timerInfo.count = CONVERT_SEC_TO_TIMER_COUNT(cameraAlarmConfig.activeTime);
                            timerInfo.funcPtr = alarmPulseTimeout;
                            timerInfo.data = ((UINT32)(cameraIndex << 16) | alarmIndex);

                            // If timer started, request alarm status
                            if(StartTimer(timerInfo, &alarmActionRequest[cameraIndex][alarmIndex].timerHandle) == SUCCESS)
                            {
                                ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);
                                if(ipCamCfg.onvifSupportF == TRUE)
                                {
                                    requestStatus = sendOnvifActionCmd(cameraIndex, ONVIF_SET_ALARM, onvifActivateAlarmCb, &ipCamCfg, alarmIndex, alarmAction, NULL);
                                }
                                else if (camType == AUTO_ADDED_CAMERA)
                                {
                                    GetCiRequestUrl(cameraIndex, REQ_URL_SET_ALARM, ACTIVE, CAM_ALARM_ACTIVE,
                                                    tcpCamAlarmOutCb, &alarmActionRequest[cameraIndex][alarmIndex].request, &alarmIndex);
                                    alarmActionRequest[cameraIndex][alarmIndex].request.requestStatus = CMD_PROCESS_ERROR;
                                    alarmActionRequest[cameraIndex][alarmIndex].request.requestCount = 0;
                                    requestStatus =	sendReqToCamera(&alarmActionRequest[cameraIndex][alarmIndex].request, &ipCamCfg, MAIN_STREAM);
                                }
                                else
                                {
                                    requestStatus = requestCameraAlarm(cameraIndex, alarmIndex,	&ipCamCfg, alarmAction);
                                }

                                // Request status failed
                                if(requestStatus != CMD_SUCCESS)
                                {
                                    EPRINT(CAMERA_INTERFACE, "alarm activate fail: [camera=%d], [alarmIndex=%d]", cameraIndex, alarmIndex);

                                    // Delete pulse timer
                                    DeleteTimer(&alarmActionRequest[cameraIndex][alarmIndex].timerHandle);

                                    // Reset alarm activate request status flag
                                    MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                                    alarmActionRequest[cameraIndex][alarmIndex].reqStatus[ACTIVE] = FREE;
                                    MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                                }
                                else
                                {
                                    callback = NULL;
                                    snprintf(detail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(cameraIndex));
                                    WriteEvent(LOG_CAMERA_EVENT, (LOG_CAMERA_ALARM_1 + alarmIndex), detail, NULL, alarmAction);
                                }
                            }
                            else
                            {
                                EPRINT(CAMERA_INTERFACE, "fail to start alarm pulse timer: [camera=%d], [alarmIndex=%d]", cameraIndex, alarmIndex);
                                requestStatus = CMD_RESOURCE_LIMIT;

                                // Reset activate request status
                                MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                                alarmActionRequest[cameraIndex][alarmIndex].reqStatus[ACTIVE] = FREE;
                                MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                            }
                        }
                        // If timer is already on, reload timer and increment count
                        else
                        {
                            ReloadTimer(alarmActionRequest[cameraIndex][alarmIndex].timerHandle, CONVERT_SEC_TO_TIMER_COUNT(cameraAlarmConfig.activeTime));

                            MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].counterLock);
                            alarmActionRequest[cameraIndex][alarmIndex].requestCounter++;
                            MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].counterLock);

                            // Reset activate request status flag
                            MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                            alarmActionRequest[cameraIndex][alarmIndex].reqStatus[ACTIVE] = FREE;
                            MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                            EPRINT(CAMERA_INTERFACE, "alarm pulse timer reloaded: [camera=%d], [alarmIndex=%d]", cameraIndex, alarmIndex);
                        }
                    }
                    break;

                    default:
                    {
                        EPRINT(CAMERA_INTERFACE, "invld alarm mode: [camera=%d], [alarmIndex=%d], [activeMode=%d]",
                               cameraIndex, alarmIndex, cameraAlarmConfig.activeMode);
                        requestStatus = CMD_PROCESS_ERROR;

                        MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].counterLock);
                        alarmActionRequest[cameraIndex][alarmIndex].requestCounter = 0;
                        MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].counterLock);

                        MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                        alarmActionRequest[cameraIndex][alarmIndex].reqStatus[ACTIVE] = FREE;
                        MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                    }
                    break;
                }
            }
            break;

            case INACTIVE:
            {
                // If pulse alarm is requested to inctivate and timer is still running, don't do anything
                if ((cameraAlarmConfig.activeMode == ALARM_PULSE) && (alarmActionRequest[cameraIndex][alarmIndex].timerHandle != INVALID_TIMER_HANDLE))
                {
                    DPRINT(CAMERA_INTERFACE, "alarm is in pulse mode and timer is running: [camera=%d], [alarmIndex=%d]", cameraIndex, alarmIndex);
                    return;
                }

                MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                // If Activate request is busy, make it inactivate and register callback
                if(alarmActionRequest[cameraIndex][alarmIndex].reqStatus[ACTIVE] == BUSY)
                {
                    alarmActionRequest[cameraIndex][alarmIndex].reqStatus[INACTIVE] = PENDING;
                    MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                    alarmActionRequest[cameraIndex][alarmIndex].request.clientCb[INACTIVE] = callback;
                    DPRINT(CAMERA_INTERFACE, "alarm activate is busy, make deactivate pending: [camera=%d], [alarmIndex=%d]", cameraIndex, alarmIndex);
                    return;
                }

                // If activate is busy only one request can be served at a time hence return failure
                if(alarmActionRequest[cameraIndex][alarmIndex].reqStatus[INACTIVE] == BUSY)
                {
                    MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                    if(callback != NULL)
                    {
                        callback(cameraIndex, alarmIndex, CMD_RESOURCE_LIMIT);
                    }
                    DPRINT(CAMERA_INTERFACE, "alarm deactivate in progress: [camera=%d], [alarmIndex=%d]", cameraIndex, alarmIndex);
                    return;
                }

                // If both are free, make it busy
                alarmActionRequest[cameraIndex][alarmIndex].reqStatus[INACTIVE] = BUSY;
                MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                alarmActionRequest[cameraIndex][alarmIndex].request.clientCb[INACTIVE] = callback;
                DPRINT(CAMERA_INTERFACE, "alarm deactivate request received: [camera=%d], [alarmIndex=%d]", cameraIndex, alarmIndex);

                // Decrement request count
                MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].counterLock);
                if(alarmActionRequest[cameraIndex][alarmIndex].requestCounter)
                {
                    alarmActionRequest[cameraIndex][alarmIndex].requestCounter--;
                }
                MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].counterLock);

                switch(cameraAlarmConfig.activeMode)
                {
                    case ALARM_INTERLOCK:
                    {
                        DPRINT(CAMERA_INTERFACE, "alarm interlock mode: [camera=%d], [alarmIndex=%d]", cameraIndex, alarmIndex);

                        // If count reached to zero, it is time to in-activate the alarm out
                        MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].counterLock);
                        if(alarmActionRequest[cameraIndex][alarmIndex].requestCounter == 0)
                        {
                            MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].counterLock);
                            ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);
                            if(ipCamCfg.onvifSupportF == TRUE)
                            {
                                requestStatus = sendOnvifActionCmd(cameraIndex, ONVIF_SET_ALARM, onvifInactivateAlarmCb, &ipCamCfg, alarmIndex, alarmAction, NULL);
                            }
                            else if (camType == AUTO_ADDED_CAMERA)
                            {
                                GetCiRequestUrl(cameraIndex, REQ_URL_SET_ALARM, INACTIVE, CAM_ALARM_INACTIVE,
                                                tcpCamAlarmOutCb, &alarmActionRequest[cameraIndex][alarmIndex].request, &alarmIndex);
                                alarmActionRequest[cameraIndex][alarmIndex].request.requestStatus = CMD_PROCESS_ERROR;
                                alarmActionRequest[cameraIndex][alarmIndex].request.requestCount = 0;
                                requestStatus =	sendReqToCamera(&alarmActionRequest[cameraIndex][alarmIndex].request, &ipCamCfg, MAIN_STREAM);
                            }
                            else
                            {
                                requestStatus = requestCameraAlarm(cameraIndex, alarmIndex,	&ipCamCfg, alarmAction);
                            }

                            // If request fails, reset request status
                            if(requestStatus != CMD_SUCCESS)
                            {
                                MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                                alarmActionRequest[cameraIndex][alarmIndex].reqStatus[INACTIVE] = FREE;
                                MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                            }
                            else
                            {
                                callback = NULL;
                                snprintf(detail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(cameraIndex));
                                WriteEvent(LOG_CAMERA_EVENT, (LOG_CAMERA_ALARM_1 + alarmIndex), detail, NULL, alarmAction);
                            }
                        }
                        else
                        {
                            MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].counterLock);

                            MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                            alarmActionRequest[cameraIndex][alarmIndex].reqStatus[INACTIVE] = FREE;
                            MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                            DPRINT(CAMERA_INTERFACE, "alarm is activated by other client: [camera=%d], [alarmIndex=%d]", cameraIndex, alarmIndex);
                        }
                    }
                    break;

                    case ALARM_PULSE:
                    {
                        // De-activate alarm only if it is requested from the pulse time out function. In that case call back will be NULL
                        if (callback != NULL)
                        {
                            MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                            alarmActionRequest[cameraIndex][alarmIndex].reqStatus[INACTIVE] = FREE;
                            MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                            break;
                        }

                        // If timer handle is invalid send in-activtae request
                        if (alarmActionRequest[cameraIndex][alarmIndex].timerHandle != INVALID_TIMER_HANDLE)
                        {
                            break;
                        }

                        DPRINT(CAMERA_INTERFACE, "alarm pulse mode: [camera=%d], [alarmIndex=%d]", cameraIndex, alarmIndex);
                        ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);
                        if(ipCamCfg.onvifSupportF == TRUE)
                        {
                            requestStatus = sendOnvifActionCmd(cameraIndex, ONVIF_SET_ALARM, onvifInactivateAlarmCb, &ipCamCfg, alarmIndex, alarmAction, NULL);
                        }
                        else if (camType == AUTO_ADDED_CAMERA)
                        {
                            GetCiRequestUrl(cameraIndex, REQ_URL_SET_ALARM, INACTIVE, CAM_ALARM_INACTIVE,
                                            tcpCamAlarmOutCb, &alarmActionRequest[cameraIndex][alarmIndex].request, &alarmIndex);
                            alarmActionRequest[cameraIndex][alarmIndex].request.requestStatus = CMD_PROCESS_ERROR;
                            alarmActionRequest[cameraIndex][alarmIndex].request.requestCount = 0;
                            requestStatus =	sendReqToCamera(&alarmActionRequest[cameraIndex][alarmIndex].request, &ipCamCfg, MAIN_STREAM);
                        }
                        else
                        {
                            requestStatus = requestCameraAlarm(cameraIndex, alarmIndex,	&ipCamCfg, alarmAction);
                        }

                        // If request failed, start pulse timer once again
                        if (requestStatus != CMD_SUCCESS)
                        {
                            DPRINT(CAMERA_INTERFACE, "alarm deactivate request failed, start pulse timer: [camera=%d], [alarmIndex=%d]", cameraIndex, alarmIndex);

                            timerInfo.count = CONVERT_SEC_TO_TIMER_COUNT(cameraAlarmConfig.activeTime);
                            timerInfo.funcPtr = alarmPulseTimeout;
                            timerInfo.data = ((UINT32)(cameraIndex << 16) | alarmIndex);
                            StartTimer(timerInfo, &alarmActionRequest[cameraIndex][alarmIndex].timerHandle);

                            MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                            alarmActionRequest[cameraIndex][alarmIndex].reqStatus[INACTIVE] = FREE;
                            MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                        }
                        else
                        {
                            callback = NULL;
                            snprintf(detail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(cameraIndex));
                            WriteEvent(LOG_CAMERA_EVENT, (LOG_CAMERA_ALARM_1 + alarmIndex), detail, NULL, alarmAction);
                        }
                    }
                    break;

                    default:
                    {
                        EPRINT(CAMERA_INTERFACE, "invld alarm mode: [camera=%d], [alarmIndex=%d], [activeMode=%d]",
                               cameraIndex, alarmIndex, cameraAlarmConfig.activeMode);
                        requestStatus = CMD_PROCESS_ERROR;
                        MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                        alarmActionRequest[cameraIndex][alarmIndex].reqStatus[INACTIVE] = FREE;
                        MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                    }
                    break;
                }
            }
            break;

            default:
            {
                EPRINT(CAMERA_INTERFACE, "invld alarm action: [camera=%d], [alarmIndex=%d], [alarmAction=%d]", cameraIndex, alarmIndex, alarmAction);
                requestStatus = CMD_PROCESS_ERROR;
            }
            break;
        }
    }while(0);

    if (callback != NULL)
    {
        callback(cameraIndex, alarmIndex, requestStatus);
    }

    MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
    if(alarmActionRequest[cameraIndex][alarmIndex].reqStatus[alarmAction] == FREE)
    {
        otherAction = (alarmAction == ACTIVE) ? INACTIVE : ACTIVE;
        if(alarmActionRequest[cameraIndex][alarmIndex].reqStatus[otherAction] == PENDING)
        {
            MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
            CameraAlarmAction(cameraIndex, alarmIndex, otherAction, alarmActionRequest[cameraIndex][alarmIndex].request.clientCb[otherAction]);
        }
        else
        {
            MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
        }
    }
    else
    {
        MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief onvifActivateAlarmCb
 * @param responseData
 * @return SUCCESS
 */

static BOOL onvifActivateAlarmCb(ONVIF_RESPONSE_PARA_t *responseData)
{
    onvifAlarmRespHandler(responseData, ACTIVE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief call back of ONVIF client for camera alarm inactivate request. It gives status to the requested module
 * @param responseData
 * @return SUCCESS
 */
static BOOL onvifInactivateAlarmCb(ONVIF_RESPONSE_PARA_t *responseData)
{
    onvifAlarmRespHandler(responseData, INACTIVE);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief handle camera alarm ACTIVATE/DE-ACTIVATE response. It takes appropriate action as per the response received
 * @param responseData
 * @param alarmAction
 * @return SUCCESS/FAIL
 */
static BOOL onvifAlarmRespHandler(ONVIF_RESPONSE_PARA_t *responseData, BOOL alarmAction)
{
    UINT8               alarmIndex;
    NET_CMD_STATUS_e    requestStatus;
    BOOL                otherAlarmAction;

    if (responseData->cameraIndex >= getMaxCameraForCurrentVariant())
    {
        return FAIL;
    }

    alarmIndex = *((UINT8PTR)responseData->data);

    switch(responseData->response)
    {
        case ONVIF_CMD_SUCCESS:
        {
            if(alarmAction == ACTIVE)
            {
                MUTEX_LOCK(alarmActionRequest[responseData->cameraIndex][alarmIndex].counterLock);
                alarmActionRequest[responseData->cameraIndex][alarmIndex].requestCounter++;
                MUTEX_UNLOCK(alarmActionRequest[responseData->cameraIndex][alarmIndex].counterLock);
            }
            DPRINT(CAMERA_INTERFACE, "alarm request success: [camera=%d], [alarmIndex=%d], [alarmAction=%s]",
                   responseData->cameraIndex, alarmIndex, alarmReqStr[alarmAction]);
            requestStatus = CMD_SUCCESS;
        }
        break;

        case ONVIF_CMD_FEATURE_NOT_SUPPORTED:
        {
            DPRINT(CAMERA_INTERFACE, "alarm feature not supported: [camera=%d], [alarmIndex=%d], [alarmAction=%s]",
                   responseData->cameraIndex, alarmIndex, alarmReqStr[alarmAction]);
            requestStatus = CMD_FEATURE_NOT_SUPPORTED;
        }
        break;

        case ONVIF_CMD_TIMEOUT:
        case ONVIF_CMD_FAIL:
        default:
        {
            DPRINT(CAMERA_INTERFACE, "alarm request failed: [camera=%d], [alarmIndex=%d], [alarmAction=%s]",
                   responseData->cameraIndex, alarmIndex, alarmReqStr[alarmAction]);
            requestStatus = CMD_CAM_REQUEST_FAILED;
        }
        break;
    }

    MUTEX_LOCK(alarmActionRequest[responseData->cameraIndex][alarmIndex].reqStatusLock);
    alarmActionRequest[responseData->cameraIndex][alarmIndex].reqStatus[alarmAction] = FREE;
    MUTEX_UNLOCK(alarmActionRequest[responseData->cameraIndex][alarmIndex].reqStatusLock);

    // If callback is not null, give callback with request status
    if (alarmActionRequest[responseData->cameraIndex][alarmIndex].request.clientCb[alarmAction] != NULL)
    {
        ((ALARM_ACTION_REQUEST_CB)alarmActionRequest[responseData->cameraIndex][alarmIndex].request.clientCb[alarmAction])
                (responseData->cameraIndex, alarmIndex, requestStatus);
    }

    // If request is free and other request is pending, serve it

    /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
    MUTEX_LOCK(alarmActionRequest[responseData->cameraIndex][alarmIndex].reqStatusLock);
    if(alarmActionRequest[responseData->cameraIndex][alarmIndex].reqStatus[alarmAction] == FREE)
    {
        otherAlarmAction = (alarmAction == ACTIVE) ? INACTIVE : ACTIVE;
        if(alarmActionRequest[responseData->cameraIndex][alarmIndex].reqStatus[otherAlarmAction] == PENDING)
        {
            MUTEX_UNLOCK(alarmActionRequest[responseData->cameraIndex][alarmIndex].reqStatusLock);
            CameraAlarmAction(responseData->cameraIndex, alarmIndex, otherAlarmAction,
                              alarmActionRequest[responseData->cameraIndex][alarmIndex].request.clientCb[otherAlarmAction]);
        }
        else
        {
            MUTEX_UNLOCK(alarmActionRequest[responseData->cameraIndex][alarmIndex].reqStatusLock);
        }
    }
    else
    {
        MUTEX_UNLOCK(alarmActionRequest[responseData->cameraIndex][alarmIndex].reqStatusLock);
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief httpAlarmActivateControlCb
 * @param httpHandle
 * @param dataInfo
 */
static void httpAlarmActivateControlCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo)
{
    alarmRespHandler(httpHandle, dataInfo, ACTIVE);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief httpAlarmDeactivateControlCb
 * @param httpHandle
 * @param dataInfo
 */
static void httpAlarmDeactivateControlCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo)
{
    alarmRespHandler(httpHandle, dataInfo, INACTIVE);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief alarmRespHandler
 * @param httpHandle
 * @param dataInfo
 * @param alarmAction
 */
static void alarmRespHandler(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo, BOOL alarmAction)
{
    NET_CMD_STATUS_e 		requestStatus = CMD_SUCCESS;
    UINT8 					cameraIndex = dataInfo->userData;
    UINT8 					alarmIndex;
    UINT8 					requestCount;
    BOOL 					breakLoop = FALSE;
    IP_CAMERA_CONFIG_t 		ipCamCfg;
    BOOL 					otherAlarmAction;

    if (cameraIndex >= getMaxCameraForCurrentVariant())
    {
        return;
    }

    for (alarmIndex = 0; alarmIndex < MAX_CAMERA_ALARM; alarmIndex++)
    {
        for(requestCount = 0; requestCount < alarmActionRequest[cameraIndex][alarmIndex].request.numOfRequest; requestCount++)
        {
            if(alarmActionRequest[cameraIndex][alarmIndex].request.http[requestCount] == httpHandle)
            {
                breakLoop = TRUE;
                break;
            }
        }

        if (breakLoop == TRUE)
        {
            break;
        }
    }

    if ((alarmIndex >= MAX_CAMERA_ALARM) || (requestCount >= alarmActionRequest[cameraIndex][alarmIndex].request.numOfRequest))
    {
        return;
    }

    switch(dataInfo->httpResponse)
    {
        case HTTP_SUCCESS:
        {
            DPRINT(CAMERA_INTERFACE, "alarm request success: [camera=%d], [alarmIndex=%d], [alarmAction=%s], [requestCount=%d]",
                   cameraIndex, alarmIndex, alarmReqStr[alarmAction], requestCount);
            if (alarmAction == ACTIVE)
            {
                MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].counterLock);
                alarmActionRequest[cameraIndex][alarmIndex].requestCounter++;
                MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].counterLock);
            }

            alarmActionRequest[cameraIndex][alarmIndex].request.http[requestCount] = INVALID_HTTP_HANDLE;

            if ((alarmActionRequest[cameraIndex][alarmIndex].request.requestCount + 1)
                    < alarmActionRequest[cameraIndex][alarmIndex].request.numOfRequest)
            {
                alarmActionRequest[cameraIndex][alarmIndex].request.requestCount++;

                //alarm out put only for IP camera
                if (CameraType(cameraIndex) == IP_CAMERA)
                {
                    ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);
                    requestStatus =	sendReqToCamera(&alarmActionRequest[cameraIndex][alarmIndex].request, &ipCamCfg, MAIN_STREAM);
                    if (requestStatus != CMD_SUCCESS)
                    {
                        EPRINT(CAMERA_INTERFACE, "alarm request failed: [camera=%d], [alarmIndex=%d], [requestCount=%d]",
                               cameraIndex, alarmIndex, alarmActionRequest[cameraIndex][alarmIndex].request.requestCount);

                        // Request failed, make request status free
                        MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                        alarmActionRequest[cameraIndex][alarmIndex].reqStatus[alarmAction] = FREE;
                        MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                    }
                }
                else
                {
                    //Invalid camera type
                    requestStatus = CMD_CAM_REQUEST_FAILED;
                }
            }
            else
            {
                // It is done, now reset request status
                MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                alarmActionRequest[cameraIndex][alarmIndex].reqStatus[alarmAction] = FREE;
                MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
            }
        }
        break;

        case HTTP_CLOSE_ON_SUCCESS:
        {
            DPRINT(CAMERA_INTERFACE, "alarm request closed with success: [camera=%d], [alarmIndex=%d], [alarmAction=%s], [requestCount=%d]",
                   cameraIndex, alarmIndex, alarmReqStr[alarmAction], requestCount);
            MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
            alarmActionRequest[cameraIndex][alarmIndex].reqStatus[alarmAction] = FREE;
            MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
            alarmActionRequest[cameraIndex][alarmIndex].request.http[requestCount] = INVALID_HTTP_HANDLE;
        }
        break;

        default:
        case HTTP_CLOSE_ON_ERROR:
        case HTTP_ERROR:
        {
            EPRINT(CAMERA_INTERFACE, "alarm request closed with error: [camera=%d], [alarmIndex=%d], [alarmAction=%s], [requestCount=%d], [httpResponse=%d]",
                   cameraIndex, alarmIndex, alarmReqStr[alarmAction], requestCount, dataInfo->httpResponse);

            alarmActionRequest[cameraIndex][alarmIndex].request.http[requestCount] = INVALID_HTTP_HANDLE;
            requestStatus = CMD_CAM_REQUEST_FAILED;

            // REquest failed, reset request status
            MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
            alarmActionRequest[cameraIndex][alarmIndex].reqStatus[alarmAction] = FREE;
            MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
        }
        break;
    }

    // If callback is not null, give callback with request status
    if (alarmActionRequest[cameraIndex][alarmIndex].request.clientCb[alarmAction] != NULL)
    {
        ((ALARM_ACTION_REQUEST_CB)alarmActionRequest[cameraIndex][alarmIndex].request.clientCb[alarmAction])(cameraIndex, alarmIndex, requestStatus);
    }

    // If request is free and other request is pending, serve it
    MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);

    if(alarmActionRequest[cameraIndex][alarmIndex].reqStatus[alarmAction] == FREE)
    {
        otherAlarmAction = (alarmAction == ACTIVE) ? INACTIVE : ACTIVE;
        if(alarmActionRequest[cameraIndex][alarmIndex].reqStatus[otherAlarmAction] == PENDING)
        {
            MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
            CameraAlarmAction(cameraIndex, alarmIndex, otherAlarmAction, alarmActionRequest[cameraIndex][alarmIndex].request.clientCb[otherAlarmAction]);
        }
        else
        {
            MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
        }
    }
    else
    {
        MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief requestCameraAlarm
 * @param cameraIndex
 * @param alarmIndex
 * @param ipCamCfg
 * @param requestType
 * @return Network command status
 */
static NET_CMD_STATUS_e requestCameraAlarm(UINT8 cameraIndex, UINT8 alarmIndex, IP_CAMERA_CONFIG_t *ipCamCfg, BOOL requestType)
{
    NET_CMD_STATUS_e requestStatus = GetCameraBrandModelUrl(cameraIndex, REQ_URL_SET_ALARM,
                                                            &alarmActionRequest[cameraIndex][alarmIndex].request, &alarmIndex, &requestType, NULL, NULL);
    if (requestStatus == CMD_SUCCESS)
    {
        alarmActionRequest[cameraIndex][alarmIndex].request.requestCount = 0;
        requestStatus =	sendReqToCamera(&alarmActionRequest[cameraIndex][alarmIndex].request, ipCamCfg, MAIN_STREAM);
    }

    return requestStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief alarmPulseTimeout
 * @param data
 */
static void alarmPulseTimeout(UINT32 data)
{
    UINT8 cameraIndex = (UINT8)(data >> 16);
    UINT8 alarmIndex = (UINT8)(data & 0x0000FFFF);

    DeleteTimer(&alarmActionRequest[cameraIndex][alarmIndex].timerHandle);
    CameraAlarmAction(cameraIndex, alarmIndex, INACTIVE, NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief httpCommonControlCb
 * @param reqType
 * @param httpHandle
 * @param dataInfo
 */
static void httpCommonControlCb(CAM_REQ_STRUCT_TYPE_e reqType, HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo)
{
    BOOL				freeReq = TRUE;
    BOOL				ptzPauseF= FALSE;
    NET_CMD_STATUS_e 	requestStatus = CMD_SUCCESS;
    UINT8 				cameraIndex = dataInfo->userData;
    UINT8               requestCount;
    NW_CMD_REPLY_CB 	callback = NULL;
    IP_CAMERA_CONFIG_t 	ipCamCfg;

    if (cameraIndex >= getMaxCameraForCurrentVariant())
    {
        return;
    }

    CAMERA_REQUEST_t *reqPtr = getCameraReqStruct(reqType, cameraIndex);
    if (reqPtr == NULL)
    {
        return;
    }

    // Find the request number
    for (requestCount = 0; requestCount < reqPtr->numOfRequest; requestCount++)
    {
        if (reqPtr->http[requestCount] == httpHandle)
        {
            break;
        }
    }

    if (requestCount >= reqPtr->numOfRequest)
    {
        return;
    }

    switch (dataInfo->httpResponse)
    {
        default:
        {
            EPRINT(CAMERA_INTERFACE, "http error: [camera=%d], [reqType=%s], [requestCount=%d]", cameraIndex, camReqStructStr[reqType], requestCount);
            // don't free request in case of HTTP_ERROR or other unknown resopnse wait for the closing response
            freeReq = FALSE;
        }
        break;

        case HTTP_SUCCESS:
        {
            DPRINT(CAMERA_INTERFACE, "http success: [camera=%d], [reqType=%s], [requestCount=%d]", cameraIndex, camReqStructStr[reqType], requestCount);
            // Set the requestStatus flag to SUCCESS, so that we can trace SUCCESS
            // at time of closing HTTP connection ( HTTP_CLOSE_ON_SUCCESS | HTTP_CLOSE_ON_ERROR)
            reqPtr->requestStatus = CMD_SUCCESS;
            freeReq = FALSE;
        }
        break;

        case HTTP_CLOSE_ON_SUCCESS:
        {
            DPRINT(CAMERA_INTERFACE, "http close on success: [camera=%d], [reqType=%s], [requestCount=%d]", cameraIndex, camReqStructStr[reqType], requestCount);
            reqPtr->http[requestCount] = INVALID_HTTP_HANDLE;

            /* Check if HTTP request was successful. we can trace this by checking requestStatus flag,
             * which we have set FAIL before starting request, if it set to SUCCESS ==> all is well.... */
            if (reqPtr->requestStatus == CMD_SUCCESS)
            {
                // Send another URL if pending
                if ((reqPtr->requestCount + 1) < reqPtr->numOfRequest)
                {
                    reqPtr->requestCount++;
                    reqPtr->requestStatus = CMD_PROCESS_ERROR;

                    ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);
                    if ((requestStatus = sendReqToCamera(reqPtr, &ipCamCfg, MAIN_STREAM)) == CMD_SUCCESS)
                    {
                        freeReq = FALSE;
                    }
                }
            }
            else
            {
                requestStatus = CMD_CAM_REQUEST_FAILED;
            }
        }
        break;

        case HTTP_CLOSE_ON_ERROR:
        {
            EPRINT(CAMERA_INTERFACE, "http close on error: [camera=%d], [reqType=%s], [requestCount=%d]", cameraIndex, camReqStructStr[reqType], requestCount);
            requestStatus = CMD_CAM_REQUEST_FAILED;
            reqPtr->http[requestCount] = INVALID_HTTP_HANDLE;
        }
        break;
    }

    if (freeReq == FALSE)
    {
        return;
    }

    if (reqPtr->clientCb[ON] != NULL)
    {
        MUTEX_LOCK(ptzPauseState[cameraIndex].ptzPauseLock);
        ptzPauseF = ptzPauseState[cameraIndex].ptzFunctionPause;
        MUTEX_UNLOCK(ptzPauseState[cameraIndex].ptzPauseLock);

        if ((requestStatus == CMD_SUCCESS) && (ptzPauseF == TRUE) &&
                ((reqType == CAM_REQ_STRUCT_PTZ) || (reqType == CAM_REQ_STRUCT_FOCUS) ||
                 (reqType == CAM_REQ_STRUCT_IRIS) || (reqType == CAM_REQ_STRUCT_GOTO_PRESET)))
        {
            requestStatus = CMD_ACTIVE_TOUR_PAUSE;
        }

        callback = reqPtr->clientCb[ON];
        callback(requestStatus, reqPtr->clientSocket[ON], TRUE);
        reqPtr->clientCb[ON] = NULL;
    }

    MUTEX_LOCK(reqPtr->camReqFlagLock);
    reqPtr->camReqBusyF = FREE;
    MUTEX_UNLOCK(reqPtr->camReqFlagLock);

    if ((reqType == CAM_REQ_STRUCT_PTZ) || (reqType == CAM_REQ_STRUCT_FOCUS) || (reqType == CAM_REQ_STRUCT_IRIS))
    {
        MUTEX_LOCK(ptzList[cameraIndex].ptzSignalLock);
        DPRINT(CAMERA_INTERFACE,"http commonCtrl signal ptzCtrlThread: [camera=%d]", cameraIndex);
        pthread_cond_signal(&ptzList[cameraIndex].ptzSignal);
        MUTEX_UNLOCK(ptzList[cameraIndex].ptzSignalLock);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief onvifCommonCtrlCb
 * @param reqType
 * @param responseData
 */
static void onvifCommonCtrlCb(CAM_REQ_STRUCT_TYPE_e reqType, ONVIF_RESPONSE_PARA_t *responseData)
{
    BOOL				ptzPauseF = FALSE;
    NET_CMD_STATUS_e	requestStatus = CMD_SUCCESS;
    NW_CMD_REPLY_CB 	callback = NULL;

    if (responseData->cameraIndex >= getMaxCameraForCurrentVariant())
    {
        EPRINT(CAMERA_INTERFACE, "invld camera index found: [camera=%d]", responseData->cameraIndex);
        return;
    }

    CAMERA_REQUEST_t *reqPtr = getCameraReqStruct(reqType, responseData->cameraIndex);

    if (reqPtr == NULL)
    {
        EPRINT(CAMERA_INTERFACE, "invld request type found: [camera=%d]", responseData->cameraIndex);
        return;
    }

    if (responseData->response != ONVIF_CMD_SUCCESS)
    {
        requestStatus = (responseData->response == ONVIF_CMD_FEATURE_NOT_SUPPORTED) ? CMD_FEATURE_NOT_SUPPORTED : CMD_CAM_REQUEST_FAILED;
    }

    if (reqPtr->clientCb[ON] != NULL)
    {
        MUTEX_LOCK(ptzPauseState[responseData->cameraIndex].ptzPauseLock);
        ptzPauseF = ptzPauseState[responseData->cameraIndex].ptzFunctionPause;
        MUTEX_UNLOCK(ptzPauseState[responseData->cameraIndex].ptzPauseLock);

        if ((requestStatus == CMD_SUCCESS) && (ptzPauseF == TRUE) &&
                ((reqType == CAM_REQ_STRUCT_PTZ) || (reqType == CAM_REQ_STRUCT_FOCUS) ||
                 (reqType == CAM_REQ_STRUCT_IRIS) || (reqType == CAM_REQ_STRUCT_GOTO_PRESET)))
        {
            requestStatus = CMD_ACTIVE_TOUR_PAUSE;
        }

        callback = reqPtr->clientCb[ON];
        callback(requestStatus, reqPtr->clientSocket[ON], TRUE);
        reqPtr->clientCb[ON] = NULL;
    }

    DPRINT(CAMERA_INTERFACE, "onvif common control response callback: [camera=%d], [reqType=%d]", responseData->cameraIndex, reqType);

    MUTEX_LOCK(reqPtr->camReqFlagLock);
    reqPtr->camReqBusyF = FREE;
    MUTEX_UNLOCK(reqPtr->camReqFlagLock);

    if ((reqType == CAM_REQ_STRUCT_PTZ) || (reqType == CAM_REQ_STRUCT_FOCUS) || (reqType == CAM_REQ_STRUCT_IRIS))
    {
        MUTEX_LOCK(ptzList[responseData->cameraIndex].ptzSignalLock);
        DPRINT(CAMERA_INTERFACE,"onvif commonCtrl signal ptzCtrlThread");
        pthread_cond_signal(&ptzList[responseData->cameraIndex].ptzSignal);
        MUTEX_UNLOCK(ptzList[responseData->cameraIndex].ptzSignalLock);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief getStreamProfileNum
 * @param streamType
 * @param camIndex
 * @param streamProfileIndex
 * @return SUCCESS/FAIL
 */
BOOL getStreamProfileNum(VIDEO_TYPE_e streamType, UINT16 camIndex, UINT8PTR streamProfileIndex)
{
    STREAM_CONFIG_t streamConfig;

    ReadSingleStreamConfig(camIndex, &streamConfig);
    if(streamType == MAIN_STREAM)
    {
        *(streamProfileIndex) = streamConfig.mainStreamProfile;
    }
    else if(streamType == SUB_STREAM)
    {
        *(streamProfileIndex) = streamConfig.subStreamProfile;
    }
    else
    {
        *(streamProfileIndex) = 0;
        EPRINT(CAMERA_INTERFACE, "invld stream type: [camera=%d], [streamType=%d]", camIndex, streamType);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief sendReqForConfig
 * @param request
 * @param ipCamCfg
 * @param streamType
 * @return SUCCESS/FAIL
 */
BOOL SendReqForConfig(CAMERA_REQUEST_t *request, IP_CAMERA_CONFIG_t *ipCamCfg, VIDEO_TYPE_e streamType)
{
    return (CMD_SUCCESS == sendReqToCamera(request, ipCamCfg, streamType)) ? SUCCESS : FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   ciStreamExtTriggerHandler
 * @param   triggerParam
 * @return  Network command status
 */
static NET_CMD_STATUS_e ciStreamExtTriggerHandler(const CI_STREAM_EXT_TRG_PARAM_t *triggerParam)
{
    CAMERA_CONFIG_t  			cameraConfig;
    STREAM_CONFIG_t  			streamConfig;
    CAMERA_BRAND_e				brand = MAX_CAMERA_BRAND;
    CAMERA_MODEL_e				model = 0;
    STREAM_CODEC_TYPE_e			codecNo;
    BOOL						localForceConfigOnCamera = FALSE;
    UINT8 						loop = 0;
    UINT8 						requestCount;
    UINT8						streamReqCnt;
    BOOL 						stopStream = TRUE;
    CI_STREAM_RESP_PARAM_t		respParam;
    CI_STREAM_STATE_e			localStreamState;
    STREAM_REQUEST_CB			clientCb[MAX_CI_STREAM_CLIENT];
    CI_STREAM_EXT_TRG_PARAM_t	internalTrigger;
    NET_CMD_STATUS_e 			retVal = CMD_SUCCESS;
    UINT16						complexCamIndex = triggerParam->camIndex;
    UINT16						realCamIndex;
    VIDEO_TYPE_e				realVideoType = MAX_STREAM;
    STREAM_INFO_t				*pStreamInfo;
    IP_CAMERA_CONFIG_t			ipCameraCfg;
    UINT8						streamProfileIndex;
    BOOL						templocalStreamConfigF = FALSE;
    GENERAL_CONFIG_t		    generalConfig;
    UINT8 						totalVideoLossDuration;

#define LOCK_STREAM_STATE               MUTEX_LOCK(pStreamInfo->currStateLock);
#define UNLOCK_STREAM_STATE             MUTEX_UNLOCK(pStreamInfo->currStateLock);

#define LOCAL_COPY_CB                   for (loop = 0; loop < MAX_CI_STREAM_CLIENT; loop++)     \
                                        {                                                       \
                                            clientCb[loop] = pStreamInfo->clientCb[loop];       \
                                        }

#define LOCAL_COPY_CB_AND_REMOVE        for (loop = 0; loop < MAX_CI_STREAM_CLIENT; loop++)     \
                                        {                                                       \
                                            clientCb[loop] = pStreamInfo->clientCb[loop];       \
                                            pStreamInfo->clientCb[loop] = NULL;                 \
                                        }

#define NOTIFY_CI_STREAM_CLIENTS        for (loop = 0; loop < MAX_CI_STREAM_CLIENT; loop++)     \
                                        {                                                       \
                                            if (clientCb[loop] != NULL)                         \
                                            {                                                   \
                                                respParam.clientIndex = loop;                   \
                                                clientCb[loop](&respParam);                     \
                                            }                                                   \
                                        }

#define NOTIFY_LIVE_STREAM_CLIENTS      for (loop = 0; loop < CI_STREAM_CLIENT_RECORD; loop++)  \
                                        {                                                       \
                                            if (clientCb[loop] != NULL)                         \
                                            {                                                   \
                                                respParam.clientIndex = loop;                   \
                                                clientCb[loop](&respParam);                     \
                                            }                                                   \
                                        }

#define NOTIFY_RECORD_STREAM_CLIENTS    if (clientCb[CI_STREAM_CLIENT_RECORD] != NULL)          \
                                        {                                                       \
                                            respParam.clientIndex = CI_STREAM_CLIENT_RECORD;    \
                                            clientCb[CI_STREAM_CLIENT_RECORD](&respParam);      \
                                        }

#define CALC_STREAM_CLIENTS             streamReqCnt = 0;                                       \
                                        for (loop = 0; loop < MAX_CI_STREAM_CLIENT; loop++)     \
                                        {                                                       \
                                            if (pStreamInfo->clientCb[loop] != NULL)            \
                                            {                                                   \
                                                streamReqCnt++;                                 \
                                            }                                                   \
                                        }

#define PRINT_TRG_INFO(state)   DPRINT(CAMERA_INTERFACE, "camera trigger: [camera=%d], [type=%s], [state=%s]",              \
                                complexCamIndex, ciStreamExtTriggerStr[triggerParam->type], ciStreamStateStr[state]);

#define PRINT_UNHANDLED_INFO    EPRINT(CAMERA_INTERFACE, "unhandled camera trigger: [camera=%d], [type=%s], [state=%s]",    \
                                complexCamIndex, ciStreamExtTriggerStr[triggerParam->type], ciStreamStateStr[pStreamInfo->currState]);

#define PRINT_IGNORE_INFO       DPRINT(CAMERA_INTERFACE, "ignoring camera trigger: [camera=%d], [type=%s], [state=%s]",     \
                                complexCamIndex, ciStreamExtTriggerStr[triggerParam->type], ciStreamStateStr[pStreamInfo->currState]);

#define PRINT_CLOSE_INFO        DPRINT(CAMERA_INTERFACE, "close camera trigger: [camera=%d], [type=%s], [state=%s]",        \
                                complexCamIndex, ciStreamExtTriggerStr[triggerParam->type], ciStreamStateStr[pStreamInfo->currState]);

#define PRINT_STATE_CHANGE(st)  DPRINT(CAMERA_INTERFACE, "change trigger state: [camera=%d], [state=%s]", complexCamIndex, ciStreamStateStr[st]);

#define CI_SET_LOCAL_STATE      localStreamState = (GetCamEventStatus(realCamIndex, CONNECTION_FAILURE) == INACTIVE) ? CI_STREAM_STATE_OFF : CI_STREAM_STATE_READY;

#define CI_STREAM_END_STATE     CI_SET_LOCAL_STATE;                         \
                                LOCK_STREAM_STATE;                          \
                                pStreamInfo->currState = localStreamState;  \
                                UNLOCK_STREAM_STATE;

    // Validate Camera Number
    if (complexCamIndex >= (2*getMaxCameraForCurrentVariant()))
    {
        EPRINT(CAMERA_INTERFACE, "invld camera found in trigger: [camera=%d], [type=%s]", complexCamIndex,  ciStreamExtTriggerStr[triggerParam->type]);
        return CMD_MAX_CAMERA_CONFIGURED;
    }

    realCamIndex = GET_STREAM_INDEX(complexCamIndex);
    realVideoType = GET_STREAM_TYPE(complexCamIndex);
    respParam.camIndex = complexCamIndex;
    pStreamInfo = &streamInfo[realCamIndex][realVideoType];

    switch (triggerParam->type)
    {
        default:
        {
            EPRINT(CAMERA_INTERFACE, "unknown camera trigger: [camera=%d], [type=%d]", complexCamIndex, triggerParam->type);
        }
        break;

        /* TRIGGER START: CI_STREAM_EXT_TRG_CONNECTIVITY this trigger is given while staring stream for any camera.
         * It has two states 1)Camera Reachable 2) Camera not reachable */
        case CI_STREAM_EXT_TRG_CONNECTIVITY:
        {
            LOCK_STREAM_STATE

            /* If camera connection state is CAM_REACHABLE */
            if (triggerParam->connState == CAM_REACHABLE)
            {
                switch (pStreamInfo->currState)
                {
                    default:
                    {
                        UNLOCK_STREAM_STATE;
                    }
                    break;

                    /* STATE START: CI_STREAM_STATE_OFF, CI_STREAM_STATE_RETRY */
                    case CI_STREAM_STATE_OFF:
                    case CI_STREAM_STATE_RETRY:
                    {
                        PRINT_TRG_INFO(pStreamInfo->currState);
                        UNLOCK_STREAM_STATE;

                        // Get Urls to be Sent
                        ReadSingleCameraConfig(realCamIndex, &cameraConfig);
                        ReadSingleStreamConfig(realCamIndex, &streamConfig);
                        GetCameraBrandModelNumber(realCamIndex, &brand, &model);
                        localForceConfigOnCamera = isStreamConfigValid(&streamConfig, realVideoType);

                        if (OnvifSupportF(realCamIndex) == TRUE)
                        {
                            // get current profile num from stream config acoording to stream type
                            getStreamProfileNum(realVideoType, realCamIndex, &streamProfileIndex);
                            if(localForceConfigOnCamera == FALSE)
                            {
                                /* Get stream configuration from camera */
                                retVal = sendOnvifStreamCmd(complexCamIndex, ONVIF_PROFILE_CONFIG, onvifGetProfileParamCb, NULL, streamProfileIndex, FALSE, NULL);
                            }
                            else
                            {
                                /* Set local configuration in camera */
                                retVal = sendOnvifStreamCmd(complexCamIndex, ONVIF_GET_MEDIA_URL, NULL, NULL, streamProfileIndex, TRUE, &streamConfig);
                            }

                            if (retVal == CMD_SUCCESS)
                            {
                                /* Get stream URL from camera */
                                retVal = sendOnvifStreamCmd(complexCamIndex, ONVIF_GET_MEDIA_URL, onvifStreamUrlCb, NULL, streamProfileIndex, FALSE, &streamConfig);
                            }
                        }
                        else if (AUTO_ADDED_CAMERA == cameraConfig.type)
                        {
                            localStreamState = CI_STREAM_STATE_OFF;
                            if (localForceConfigOnCamera == FALSE)
                            {
                                getStreamProfileNum(realVideoType, realCamIndex, &streamProfileIndex);
                                GetCiRequestUrl(realCamIndex, REQ_URL_GET_STREAM_CFG, (streamProfileIndex - 1),
                                                CAM_REQ_CONFIG, TcpGetStreamProfileCnfgCb, &pStreamInfo->getStreamRequest, NULL);
                                GetCurrentCameraProfileConfig(&pStreamInfo->getStreamRequest, complexCamIndex, streamProfileIndex, cameraConfigGetProfileParamCb);
                                GetCiRequestUrl(realCamIndex, REQ_URL_GET_STREAM, 0, CAM_REQ_MEDIA, tcpMediaCb, &pStreamInfo->getStreamRequest, NULL);
                            }
                            else
                            {
                                pStreamInfo->getStreamRequest.tcpCallback[CAM_REQ_CONTROL] = tcpStreamControlCb;
                                GetCiRequestUrl(realCamIndex, REQ_URL_SET_STREAM_CFG, realVideoType, CAM_REQ_MEDIA, tcpMediaCb, &pStreamInfo->getStreamRequest, &streamConfig);
                            }

                            if (localForceConfigOnCamera == TRUE)
                            {
                                for (loop = 0; loop < pStreamInfo->getStreamRequest.numOfRequest; loop++)
                                {
                                    if (pStreamInfo->getStreamRequest.url[loop].requestType == CAM_REQ_CONTROL)
                                    {
                                        break;
                                    }
                                }

                                pStreamInfo->getStreamRequest.requestCount = loop;

                                if (loop < pStreamInfo->getStreamRequest.numOfRequest)
                                {
                                    ReadSingleIpCameraConfig(realCamIndex, &ipCameraCfg);
                                    retVal = sendReqToCamera(&pStreamInfo->getStreamRequest, &ipCameraCfg, realVideoType);
                                    if (retVal == CMD_RESOURCE_LIMIT)
                                    {
                                        // Start Retry Timer
                                        if (startStreamRetry(complexCamIndex) == SUCCESS)
                                        {
                                            localStreamState = CI_STREAM_STATE_RETRY;

                                            // Notify Clients, that we are retrying
                                            respParam.respCode = CI_STREAM_RESP_RETRY;
                                            LOCK_STREAM_STATE;
                                            LOCAL_COPY_CB;
                                            pStreamInfo->forceConfigOnCamera = TRUE;
                                            pStreamInfo->currState = localStreamState;
                                            UNLOCK_STREAM_STATE;
                                        }
                                        else
                                        {
                                            /* Set state OFF if camera offline else set READY */
                                            CI_SET_LOCAL_STATE;
                                            LOCK_STREAM_STATE;
                                            LOCAL_COPY_CB_AND_REMOVE;
                                            pStreamInfo->currState = localStreamState;
                                            UNLOCK_STREAM_STATE;
                                            respParam.respCode = CI_STREAM_RESP_CLOSE;
                                        }

                                        NOTIFY_LIVE_STREAM_CLIENTS;
                                        PRINT_STATE_CHANGE(localStreamState);
                                    }
                                    else
                                    {
                                        pStreamInfo->streamRetryRunning = FALSE;
                                        DeleteTimer(&pStreamInfo->retryTimerHandle);
                                    }
                                }
                                else
                                {
                                    localStreamState = CI_STREAM_STATE_READY;
                                }
                            }
                            else
                            {
                                localStreamState = CI_STREAM_STATE_READY;
                            }

                            if (localStreamState == CI_STREAM_STATE_READY)
                            {
                                LOCK_STREAM_STATE;
                                CALC_STREAM_CLIENTS;
                                pStreamInfo->currState = CI_STREAM_STATE_READY;
                                UNLOCK_STREAM_STATE;
                                PRINT_STATE_CHANGE(CI_STREAM_STATE_READY);

                                if (streamReqCnt > 0)
                                {
                                    internalTrigger.camIndex = complexCamIndex;
                                    internalTrigger.type = CI_STREAM_EXT_TRG_START_STREAM;
                                    internalTrigger.clientType = MAX_CI_STREAM_CLIENT;
                                    retVal = ciStreamExtTriggerHandler(&internalTrigger);
                                }
                            }
                        }
                        // Request stream from the camera
                        else
                        {
                            localStreamState = CI_STREAM_STATE_OFF;
                            ReadSingleIpCameraConfig(realCamIndex, &ipCameraCfg);

                            // Check whether the function is registered or not
                            if (brand == CAMERA_BRAND_GENERIC)
                            {
                                snprintf(pStreamInfo->getStreamRequest.url[0].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s", ipCameraCfg.url);
                                pStreamInfo->getStreamRequest.numOfRequest = 1;
                                pStreamInfo->getStreamRequest.url[0].requestType = CAM_REQ_MEDIA;

                                switch(model)
                                {
                                    case GENERIC_HTTP_MODEL:
                                    {
                                        pStreamInfo->getStreamRequest.url[0].protocolType = CAM_HTTP_PROTOCOL;
                                        pStreamInfo->getStreamRequest.url[0].httpRequestType = GET_REQUEST;
                                        pStreamInfo->getStreamRequest.url[0].authMethod = AUTH_TYPE_ANY;
                                    }
                                    break;

                                    case GENERIC_RTSP_UDP_MODEL:
                                    {
                                        pStreamInfo->getStreamRequest.url[0].protocolType = CAM_RTSP_PROTOCOL;
                                        pStreamInfo->getStreamRequest.url[0].rtspTransportType = OVER_UDP;
                                        pStreamInfo->getStreamRequest.url[0].authMethod = AUTH_TYPE_ANY;
                                    }
                                    break;

                                    case GENERIC_RTSP_TCP_MODEL:
                                    default:
                                    {
                                        pStreamInfo->getStreamRequest.url[0].protocolType = CAM_RTSP_PROTOCOL;
                                        pStreamInfo->getStreamRequest.url[0].rtspTransportType = TCP_INTERLEAVED;
                                        pStreamInfo->getStreamRequest.url[0].authMethod = AUTH_TYPE_ANY;
                                    }
                                    break;
                                }
                                pStreamInfo->getStreamRequest.requestCount = 0;
                                localStreamState = CI_STREAM_STATE_READY;
                            }
                            else
                            {
                                if (localForceConfigOnCamera == FALSE)
                                {
                                    getStreamProfileNum(realVideoType, realCamIndex, &streamProfileIndex);
                                    if (CMD_SUCCESS != GetBrandModelReqestUrl(realCamIndex, brand, model, REQ_URL_GET_STREAM_CFG,
                                                                              &pStreamInfo->getStreamRequest, &realVideoType, &streamProfileIndex, NULL))
                                    {
                                        EPRINT(CAMERA_INTERFACE, "fail to get stream config: [camera=%d], [brand=%d], [model=%d]", realCamIndex, brand, model);
                                    }
                                    else if (GetCurrentCameraProfileConfig(&pStreamInfo->getStreamRequest, complexCamIndex, streamProfileIndex, cameraConfigGetProfileParamCb) == FAIL)
                                    {
                                        EPRINT(CAMERA_INTERFACE, "fail to get stream params: [camera=%d], [streamType=%d]", realCamIndex, realVideoType);
                                    }
                                }

                                if (CMD_SUCCESS != GetBrandModelReqestUrl(realCamIndex, brand, model, REQ_URL_GET_STREAM, &pStreamInfo->getStreamRequest,
                                                                          &realVideoType, &streamConfig, &localForceConfigOnCamera))
                                {
                                    EPRINT(CAMERA_INTERFACE, "fail to get stream media: [camera=%d], [brand=%d], [model=%d]", realCamIndex, brand, model);
                                }
                                else
                                {
                                    /* Check if Any Control URL is there to be sent */
                                    pStreamInfo->getStreamRequest.requestCount = 0;
                                    if(localForceConfigOnCamera == TRUE)
                                    {
                                        for (loop = 0; loop < pStreamInfo->getStreamRequest.numOfRequest; loop++)
                                        {
                                            if (pStreamInfo->getStreamRequest.url[loop].requestType == CAM_REQ_CONTROL)
                                            {
                                                break;
                                            }
                                        }

                                        pStreamInfo->getStreamRequest.requestCount = loop;
                                        if (loop < pStreamInfo->getStreamRequest.numOfRequest)
                                        {
                                            retVal = sendReqToCamera(&pStreamInfo->getStreamRequest, &ipCameraCfg, realVideoType);
                                        }
                                        else
                                        {
                                            localStreamState = CI_STREAM_STATE_READY;
                                        }
                                    }
                                    else
                                    {
                                        localStreamState = CI_STREAM_STATE_READY;
                                    }
                                }
                            }

                            if (localStreamState == CI_STREAM_STATE_READY)
                            {
                                LOCK_STREAM_STATE;
                                CALC_STREAM_CLIENTS;
                                pStreamInfo->currState = CI_STREAM_STATE_READY;
                                UNLOCK_STREAM_STATE;
                                PRINT_STATE_CHANGE(CI_STREAM_STATE_READY);

                                if (streamReqCnt > 0)
                                {
                                    internalTrigger.camIndex = complexCamIndex;
                                    internalTrigger.type = CI_STREAM_EXT_TRG_START_STREAM;
                                    internalTrigger.clientType = MAX_CI_STREAM_CLIENT;
                                    retVal = ciStreamExtTriggerHandler(&internalTrigger);
                                }
                            }
                        }

                        if (MAIN_STREAM == realVideoType)
                        {
                            /* Set camera date and time. Skip for ONVIF. In ONVIF, it will be set after getting stream URL */
                            if ((OnvifSupportF(realCamIndex) == FALSE) && (CMD_SUCCESS != setCameraDateTime(realCamIndex)))
                            {
                                EPRINT(CAMERA_INTERFACE, "fail to set camera date-time: [camera=%d]", realCamIndex);
                            }

                            /* Get image capability and parameters */
                            getImageSettingCapability(realCamIndex);

                            /* Set OSD if it is pending to set */
                            if (TRUE == setOsdsRequest[realCamIndex].isOsdSetPending)
                            {
                                if (CMD_SUCCESS == setOsds(realCamIndex, &setOsdsRequest[realCamIndex].osdParam))
                                {
                                    setOsdsRequest[realCamIndex].isOsdSetPending = FALSE;
                                }
                            }
                        }

                        if (retVal != CMD_SUCCESS)
                        {
                            /* In case of CMD_PROCESS_ERROR or any other case Timer would not be deleted.
                             * So, camera will be stuck in retry loop. In this case it is ambiguous to delete the time or not!! */
                            EPRINT(CAMERA_INTERFACE, "stream request failed: [camera=%d], [cmdSts=%d]", complexCamIndex, retVal);
                            if((retVal == CMD_CAM_DISCONNECTED) || (retVal == CMD_CHANNEL_DISABLED))
                            {
                                DeleteTimer(&pStreamInfo->retryTimerHandle);
                            }
                        }
                    }
                    break;
                    /* STATE END: CI_STREAM_STATE_OFF, CI_STREAM_STATE_RETRY */

                    case CI_STREAM_STATE_READY:
                    case CI_STREAM_STATE_OFF_WAIT:
                    case CI_STREAM_STATE_ON:
                    case CI_STREAM_STATE_ON_WAIT:
                    case CI_STREAM_STATE_RESTART:
                    {
                        PRINT_IGNORE_INFO;
                        UNLOCK_STREAM_STATE;
                    }
                    break;
                }
            }
            /* If camera connection state is CAM_NOT_REACHABLE means error in camera connectivity
             * 1)If READY => OFF
             * 2)except READY for all other do nothing, ignore the trigger */
            else if (triggerParam->connState == CAM_NOT_REACHABLE)
            {
                /* Make all camera event inactive/normal on offline */
                MakeAllCameraEventInactive(realCamIndex);

                switch (pStreamInfo->currState)
                {
                    default:
                    {
                        UNLOCK_STREAM_STATE;
                    }
                    break;

                    case CI_STREAM_STATE_READY:
                    {
                        pStreamInfo->currState = CI_STREAM_STATE_OFF;
                        UNLOCK_STREAM_STATE;
                        PRINT_TRG_INFO(CI_STREAM_STATE_READY);
                        PRINT_STATE_CHANGE(CI_STREAM_STATE_OFF);
                    }
                    break;

                    case CI_STREAM_STATE_OFF:
                    case CI_STREAM_STATE_RETRY:
                    case CI_STREAM_STATE_OFF_WAIT:
                    case CI_STREAM_STATE_ON:
                    case CI_STREAM_STATE_ON_WAIT:
                    case CI_STREAM_STATE_RESTART:
                    {
                        PRINT_IGNORE_INFO;
                        UNLOCK_STREAM_STATE;
                    }
                    break;
                }
            }
            else
            {
                /* Invalid camera connection state */
                UNLOCK_STREAM_STATE;
            }
        }
        break;
        /* TRIGGER END: CI_STREAM_EXT_TRG_CONNECTIVITY */

        /* TRIGGER START: CI_STREAM_EXT_TRG_CONTROL_CB */
        case CI_STREAM_EXT_TRG_CONTROL_CB:
        {
            LOCK_STREAM_STATE;

            switch (pStreamInfo->currState)
            {
                default:
                {
                    UNLOCK_STREAM_STATE;
                }
                break;

                case CI_STREAM_STATE_ON_WAIT:
                case CI_STREAM_STATE_RETRY:
                case CI_STREAM_STATE_RESTART:
                case CI_STREAM_STATE_ON:
                {
                    PRINT_UNHANDLED_INFO;
                    UNLOCK_STREAM_STATE;
                }
                break;

                /* STATE START: CI_STREAM_STATE_OFF */
                case CI_STREAM_STATE_OFF:
                {
                    UNLOCK_STREAM_STATE;
                    PRINT_TRG_INFO(CI_STREAM_STATE_OFF);
                    if (triggerParam->status == CMD_SUCCESS)
                    {
                        localStreamState = CI_STREAM_STATE_OFF;
                        pStreamInfo->getStreamRequest.requestCount++;

                        // Check if Any Control URl is left to be sent
                        if ((pStreamInfo->getStreamRequest.requestCount < pStreamInfo->getStreamRequest.numOfRequest)
                            && (pStreamInfo->getStreamRequest.url[pStreamInfo->getStreamRequest.requestCount].requestType == CAM_REQ_CONTROL))
                        {
                            ReadSingleIpCameraConfig(realCamIndex, &ipCameraCfg);
                            if ((retVal = sendReqToCamera(&pStreamInfo->getStreamRequest, &ipCameraCfg, realVideoType)) != CMD_SUCCESS)
                            {
                                // Check if This trigger is from Retry Timer
                                if (pStreamInfo->streamRetryRunning == TRUE)
                                {
                                    LOCK_STREAM_STATE;
                                    pStreamInfo->currState = CI_STREAM_STATE_RETRY;
                                    UNLOCK_STREAM_STATE;
                                    PRINT_STATE_CHANGE(CI_STREAM_STATE_RETRY);
                                }
                                else
                                {
                                    localStreamState = CI_STREAM_STATE_READY;
                                }
                            }
                        }
                        else
                        {
                            localStreamState = CI_STREAM_STATE_READY;
                        }

                        if (localStreamState == CI_STREAM_STATE_READY)
                        {
                            LOCK_STREAM_STATE;
                            pStreamInfo->forceConfigOnCamera = FALSE;
                            CALC_STREAM_CLIENTS;
                            pStreamInfo->currState = localStreamState;
                            UNLOCK_STREAM_STATE;
                            PRINT_STATE_CHANGE(localStreamState);

                            // Check if Any One requested this stream
                            if (streamReqCnt > 0)
                            {
                                internalTrigger.camIndex = complexCamIndex;
                                internalTrigger.type = CI_STREAM_EXT_TRG_START_STREAM;
                                internalTrigger.clientType = MAX_CI_STREAM_CLIENT;
                                retVal = ciStreamExtTriggerHandler(&internalTrigger);
                            }
                            else
                            {
                                pStreamInfo->streamRetryRunning = FALSE;
                                DeleteTimer(&pStreamInfo->retryTimerHandle);
                            }
                        }
                    }
                    else
                    {
                        // Check if This trigger is from Retry Timer
                        if (pStreamInfo->streamRetryRunning == TRUE)
                        {
                            localStreamState = CI_STREAM_STATE_RETRY;
                        }
                        else
                        {
                            localStreamState = CI_STREAM_STATE_OFF;
                        }

                        LOCK_STREAM_STATE;
                        pStreamInfo->currState = localStreamState;
                        UNLOCK_STREAM_STATE;
                        PRINT_STATE_CHANGE(localStreamState);
                    }
                }
                break;
                /* STATE END: CI_STREAM_STATE_OFF */

                case CI_STREAM_STATE_OFF_WAIT:
                {
                    UNLOCK_STREAM_STATE;
                    streamInfo[realCamIndex][realVideoType].offWaitUnHandleCnt = 0;
                    CI_STREAM_END_STATE;
                }
                break;
            }
        }
        break;
        /* TRIGGER END: CI_STREAM_EXT_TRG_CONTROL_CB */

        /* TRIGGER START: CI_STREAM_EXT_TRG_START_STREAM */
        case CI_STREAM_EXT_TRG_START_STREAM:
        {
            LOCK_STREAM_STATE;
            switch (pStreamInfo->currState)
            {
                default:
                {
                    UNLOCK_STREAM_STATE;
                }
                break;

                case CI_STREAM_STATE_RESTART:
                {
                    PRINT_UNHANDLED_INFO;
                    UNLOCK_STREAM_STATE;
                    retVal = CMD_RESOURCE_LIMIT;
                }
                break;

                /* STATE START: CI_STREAM_STATE_OFF */
                case CI_STREAM_STATE_OFF:
                {
                    UNLOCK_STREAM_STATE;
                    ReadSingleCameraConfig(realCamIndex, &cameraConfig);
                    if (cameraConfig.camera == DISABLE)
                    {
                        retVal = CMD_CHANNEL_DISABLED;
                    }
                    else if (GetCamEventStatus(realCamIndex, CONNECTION_FAILURE) == INACTIVE)
                    {
                        retVal = CMD_CAM_DISCONNECTED;
                    }
                    else if (OnvifSupportF(realCamIndex) == TRUE)
                    {
                        MUTEX_LOCK(onvifCamDetail[realCamIndex].onvifAccessLock);
                        BOOL authFailF = onvifCamDetail[realCamIndex].authFailF;
                        MUTEX_UNLOCK(onvifCamDetail[realCamIndex].onvifAccessLock);
                        if (authFailF == TRUE)
                        {
                            retVal = CMD_CRED_INVALID;
                        }
                        else
                        {
                            retVal = CMD_ONVIF_CAM_CAPABILITY_ERROR;
                        }

                        updateVideoLossStatus(realCamIndex, FALSE);
                    }
                    else
                    {
                        LOCK_STREAM_STATE;
                        CALC_STREAM_CLIENTS;
                        if (streamReqCnt > 0)
                        {
                            // Store Client Information
                            pStreamInfo->clientCb[triggerParam->clientType] = triggerParam->callback;
                        }
                        else
                        {
                            retVal = CMD_RESOURCE_LIMIT;
                        }

                        UNLOCK_STREAM_STATE;
                    }
                }
                break;
                /* STATE END: CI_STREAM_STATE_OFF */

                /* STATE START: CI_STREAM_STATE_RETRY, CI_STREAM_STATE_READY */
                case CI_STREAM_STATE_RETRY:
                case CI_STREAM_STATE_READY:
                {
                    PRINT_TRG_INFO(pStreamInfo->currState);
                    if ((pStreamInfo->currState == CI_STREAM_STATE_RETRY) && (triggerParam->clientType != MAX_CI_STREAM_CLIENT))
                    {
                        pStreamInfo->clientCb[triggerParam->clientType] = triggerParam->callback;
                        UNLOCK_STREAM_STATE;
                    }
                    else
                    {
                        // Change State to CI_STREAM_STATE_ON_WAIT & Store Client Information
                        pStreamInfo->currState = CI_STREAM_STATE_ON_WAIT;

                        // When StartStream Request is triggered internally client is MAX_CI_STREAM_CLIENT
                        // So There won't be any callback in request
                        if (triggerParam->clientType != MAX_CI_STREAM_CLIENT)
                        {
                            pStreamInfo->clientCb[triggerParam->clientType] = triggerParam->callback;
                        }

                        UNLOCK_STREAM_STATE;
                        PRINT_STATE_CHANGE(CI_STREAM_STATE_ON_WAIT);
                        //Read config files
                        ReadSingleCameraConfig(realCamIndex, &cameraConfig);
                        ReadSingleStreamConfig(realCamIndex, &streamConfig);

                        //Update configFrameRate in case it is not updated by config Notify function
                        if(streamInfo[realCamIndex][MAIN_STREAM].configFrameRate !=  streamConfig.framerate)
                        {
                            streamInfo[realCamIndex][MAIN_STREAM].configFrameRate = streamConfig.framerate;
                        }

                        if(streamInfo[realCamIndex][SUB_STREAM].configFrameRate !=  streamConfig.framerateSub)
                        {
                            streamInfo[realCamIndex][SUB_STREAM].configFrameRate = streamConfig.framerateSub;
                        }

                        if (AUTO_ADDED_CAMERA == cameraConfig.type)
                        {
                            ReadSingleIpCameraConfig(realCamIndex, &ipCameraCfg);

                            // Adjust Request Count
                            for (loop = 0; loop < pStreamInfo->getStreamRequest.numOfRequest; loop++)
                            {
                                if ( pStreamInfo->getStreamRequest.url[loop].requestType == CAM_REQ_MEDIA)
                                {
                                    break;
                                }
                            }

                            pStreamInfo->getStreamRequest.requestCount = loop;

                            retVal = sendReqToCamera(&pStreamInfo->getStreamRequest, &ipCameraCfg, realVideoType);
                            if (retVal != CMD_SUCCESS)
                            {
                                EPRINT(CAMERA_INTERFACE, "stream request failed: [camera=%d], [cmdSts=%d]", complexCamIndex, retVal);
                            }
                        }
                        else
                        {
                            ReadSingleIpCameraConfig(realCamIndex, &ipCameraCfg);
                            getCameraCodecNum(realCamIndex, realVideoType, &codecNo);
                            GetCameraBrandModelNumber(realCamIndex, &brand, &model);

                            // Adjust Request Count
                            for (loop = 0; loop < pStreamInfo->getStreamRequest.numOfRequest; loop++)
                            {
                                if (pStreamInfo->getStreamRequest.url[loop].requestType == CAM_REQ_MEDIA)
                                {
                                    break;
                                }
                            }

                            pStreamInfo->getStreamRequest.requestCount = loop;

                            retVal = sendReqToCamera(&pStreamInfo->getStreamRequest, &ipCameraCfg, realVideoType);
                            if (retVal != CMD_SUCCESS)
                            {
                                EPRINT(CAMERA_INTERFACE, "stream request failed: [camera=%d], [cmdSts=%d]", complexCamIndex, retVal);
                            }
                        }

                        // Check For Failure
                        if (retVal != CMD_SUCCESS)
                        {
                            // When StartStream Request is triggered internally client is MAX_CI_STREAM_CLIENT
                            // don't give callback to all clients from here because
                            // in that case Response should be CI_STREAM_RESP_CLOSE not CI_STREAM_RESP_START
                            if (triggerParam->clientType != MAX_CI_STREAM_CLIENT)
                            {
                                LOCK_STREAM_STATE;
                                pStreamInfo->currState = CI_STREAM_STATE_OFF_WAIT;

                                // Remove Information of the clients
                                LOCAL_COPY_CB_AND_REMOVE;
                                UNLOCK_STREAM_STATE;
                                respParam.respCode = CI_STREAM_RESP_START;
                                respParam.cmdStatus = retVal;

                                // Give Notification to other client who are in wait of stream starting
                                NOTIFY_CI_STREAM_CLIENTS;
                                localStreamState = CI_STREAM_STATE_READY;
                                updateVideoLossStatus(realCamIndex, FALSE);
                            }
                            else
                            {
                                localStreamState = CI_STREAM_STATE_RETRY;
                            }

                            LOCK_STREAM_STATE;
                            pStreamInfo->currState = localStreamState;
                            UNLOCK_STREAM_STATE;
                            PRINT_STATE_CHANGE(localStreamState);
                        }
                    }
                }
                break;
                /* STATE END: CI_STREAM_STATE_RETRY, CI_STREAM_STATE_READY */

                case CI_STREAM_STATE_ON:
                {
                    // Store Client Information
                    pStreamInfo->clientCb[triggerParam->clientType] = triggerParam->callback;
                    UNLOCK_STREAM_STATE;
                    respParam.respCode = CI_STREAM_RESP_START;
                    respParam.cmdStatus = retVal;
                    respParam.clientIndex = triggerParam->clientType;
                    triggerParam->callback(&respParam);
                    PRINT_TRG_INFO(CI_STREAM_STATE_ON);
                }
                break;

                case CI_STREAM_STATE_ON_WAIT:
                {
                    // Store Client Information
                    pStreamInfo->clientCb[triggerParam->clientType] = triggerParam->callback;
                    UNLOCK_STREAM_STATE;
                    PRINT_TRG_INFO(CI_STREAM_STATE_ON_WAIT);
                }
                break;

                case CI_STREAM_STATE_OFF_WAIT:
                {
                    UNLOCK_STREAM_STATE;
                    EPRINT(CAMERA_INTERFACE, "can't handle camera trigger: [camera=%d], [type=%s], [state=%s]", \
                           complexCamIndex, ciStreamExtTriggerStr[CI_STREAM_EXT_TRG_START_STREAM], ciStreamStateStr[CI_STREAM_STATE_OFF_WAIT]);
                    streamInfo[realCamIndex][realVideoType].offWaitUnHandleCnt++;
                    if(streamInfo[realCamIndex][realVideoType].offWaitUnHandleCnt > 5)
                    {
                        EPRINT(CAMERA_INTERFACE, "max try reach, hence stopped: [camera=%d], [state=%s]",
                               complexCamIndex, ciStreamStateStr[CI_STREAM_STATE_OFF_WAIT]);

                        streamInfo[realCamIndex][realVideoType].offWaitUnHandleCnt = 0;
                        stopAllStreamRequests(complexCamIndex, CI_STREAM_INT_RESP_CLOSE);
                        CI_STREAM_END_STATE;
                        PRINT_STATE_CHANGE(localStreamState);
                    }

                    retVal = CMD_RESOURCE_LIMIT;
                }
                break;
            }
        }
        break;
        /* TRIGGER END: CI_STREAM_EXT_TRG_START_STREAM */

        /* TRIGGER START: CI_STREAM_EXT_TRG_STOP_STREAM */
        case CI_STREAM_EXT_TRG_STOP_STREAM:
        {
            stopStream = TRUE;
            LOCK_STREAM_STATE;

            if (triggerParam->clientType != MAX_CI_STREAM_CLIENT)
            {
                if (pStreamInfo->clientCb[triggerParam->clientType] == NULL)
                {
                    // Buggy Client : Stop Stream Request without Start Stream
                    UNLOCK_STREAM_STATE;
                    break;
                }

                // Remove Client Information
                pStreamInfo->clientCb[triggerParam->clientType] = NULL;

                // Check If Any One Else Wants this Stream
                CALC_STREAM_CLIENTS;

                if (streamReqCnt > 0)
                {
                    stopStream = FALSE;
                }
            }

            switch (pStreamInfo->currState)
            {
                default:
                {
                    UNLOCK_STREAM_STATE;
                }
                break;

                case CI_STREAM_STATE_OFF_WAIT:
                {
                    PRINT_IGNORE_INFO;
                    UNLOCK_STREAM_STATE;
                }
                break;

                case CI_STREAM_STATE_OFF:
                {
                    PRINT_TRG_INFO(pStreamInfo->currState);
                    LOCAL_COPY_CB_AND_REMOVE;
                    UNLOCK_STREAM_STATE;

                    // Give Failure Callback To All Clients,
                    respParam.respCode = CI_STREAM_RESP_CLOSE;
                    NOTIFY_CI_STREAM_CLIENTS;
                }
                break;

                case CI_STREAM_STATE_ON:
                case CI_STREAM_STATE_ON_WAIT:
                case CI_STREAM_STATE_RESTART:
                {
                    PRINT_TRG_INFO(pStreamInfo->currState);

                    if (stopStream == TRUE)
                    {
                        // Change this state so that stream will be stopped in CI_STREAM_EXT_TRG_CONTROL_CB
                        pStreamInfo->currState = CI_STREAM_STATE_OFF_WAIT;
                    }

                    UNLOCK_STREAM_STATE;

                    // Just TO Put DEbug we are checking this condition again
                    if (stopStream == TRUE)
                    {
                        PRINT_STATE_CHANGE(CI_STREAM_STATE_OFF_WAIT);
                    }
                }
                break;

                case CI_STREAM_STATE_RETRY:
                {
                    EPRINT(CAMERA_INTERFACE, "unhandled state, this case is not expected to be generated: [camera=%d]", complexCamIndex);
                    LOCAL_COPY_CB_AND_REMOVE;
                    pStreamInfo->currState = CI_STREAM_STATE_OFF_WAIT;
                    UNLOCK_STREAM_STATE;

                    stopAllStreamRequests(complexCamIndex, triggerParam->mediaResp);

                    // Give Failure Callback To All Clients,
                    respParam.respCode = CI_STREAM_RESP_CLOSE;
                    NOTIFY_CI_STREAM_CLIENTS;
                    streamInfo[realCamIndex][realVideoType].offWaitUnHandleCnt = 0;
                    CI_STREAM_END_STATE;
                    PRINT_TRG_INFO(CI_STREAM_STATE_RETRY);
                    PRINT_STATE_CHANGE(localStreamState);
                }
                break;
            }
        }
        break;
        /* TRIGGER END: CI_STREAM_EXT_TRG_STOP_STREAM */

        /* TRIGGER START: CI_STREAM_EXT_TRG_MEDIA_CB */
        case CI_STREAM_EXT_TRG_MEDIA_CB:
        {
            LOCK_STREAM_STATE;

            /* If media response received with new frame */
            if (triggerParam->mediaResp == CI_STREAM_INT_RESP_FRAME)
            {
                switch (pStreamInfo->currState)
                {
                    default:
                    {
                        UNLOCK_STREAM_STATE;
                    }
                    break;

                    case CI_STREAM_STATE_OFF:
                    case CI_STREAM_STATE_RETRY:
                    {
                        PRINT_UNHANDLED_INFO;
                        UNLOCK_STREAM_STATE;
                    }
                    break;

                    /* STATE START: CI_STREAM_STATE_ON_WAIT */
                    case CI_STREAM_STATE_ON_WAIT:
                    {
                        localStreamState = CI_STREAM_STATE_ON_WAIT;

                        // Check if any request is pending
                        if ((pStreamInfo->getStreamRequest.requestCount + 1) < pStreamInfo->getStreamRequest.numOfRequest)
                        {
                            UNLOCK_STREAM_STATE;

                            // Send next URL to the camera
                            requestCount = ++pStreamInfo->getStreamRequest.requestCount;
                            ReadSingleIpCameraConfig(realCamIndex, &ipCameraCfg);

                            if ((retVal = sendReqToCamera(&pStreamInfo->getStreamRequest, &ipCameraCfg, realVideoType)) != CMD_SUCCESS)
                            {
                                EPRINT(CAMERA_INTERFACE, "fail to send start request: [camera=%d], [requestCount=%d]", complexCamIndex, requestCount);

                                // Check if This trigger is from Retry Timer
                                if (pStreamInfo->streamRetryRunning == TRUE)
                                {
                                    stopAllStreamRequests(complexCamIndex,triggerParam->mediaResp);
                                    LOCK_STREAM_STATE;
                                    pStreamInfo->currState = CI_STREAM_STATE_RETRY;
                                    UNLOCK_STREAM_STATE;
                                    PRINT_STATE_CHANGE(CI_STREAM_STATE_RETRY);
                                }
                                else
                                {
                                    LOCK_STREAM_STATE;
                                    pStreamInfo->currState = CI_STREAM_STATE_OFF_WAIT;
                                    LOCAL_COPY_CB_AND_REMOVE;
                                    UNLOCK_STREAM_STATE;
                                    stopAllStreamRequests(complexCamIndex,triggerParam->mediaResp);

                                    // Give Failure Callback To All Clients,
                                    respParam.respCode = CI_STREAM_RESP_CLOSE;
                                    NOTIFY_CI_STREAM_CLIENTS;
                                    CI_STREAM_END_STATE;
                                    PRINT_STATE_CHANGE(localStreamState);
                                }
                            }
                            else
                            {
                                /* Write received frame in frame buffer to serve registered clients */
                                writeToStreamBuff(complexCamIndex, triggerParam->streamType, triggerParam->bufferPtr, triggerParam->mediaFrame);
                            }
                        }
                        else
                        {
                            localStreamState = CI_STREAM_STATE_ON;
                        }

                        if (localStreamState == CI_STREAM_STATE_ON)
                        {
                            LOCAL_COPY_CB;
                            pStreamInfo->currState = CI_STREAM_STATE_ON;
                            UNLOCK_STREAM_STATE;
                            DeleteTimer(&pStreamInfo->retryTimerHandle);
                            pStreamInfo->streamRetryRunning = FALSE;
                            PRINT_STATE_CHANGE(CI_STREAM_STATE_ON);

                            // Check if we are coming from RETRY->ON
                            pStreamInfo->recStrmRetryCnt = 0;
                            respParam.cmdStatus = CMD_SUCCESS;
                            respParam.respCode = CI_STREAM_RESP_START;

                            getCameraCodecNum(realCamIndex, realVideoType, &codecNo);
                            GetCameraBrandModelNumber(realCamIndex, &brand, &model);

                            /* Write received frame in frame buffer to serve registered clients */
                            writeToStreamBuff(complexCamIndex, triggerParam->streamType, triggerParam->bufferPtr, triggerParam->mediaFrame);
                            NOTIFY_CI_STREAM_CLIENTS;
                        }
                    }
                    break;
                    /* STATE END: CI_STREAM_STATE_ON_WAIT */

                    /* STATE START: CI_STREAM_STATE_ON */
                    case CI_STREAM_STATE_ON:
                    {
                        LOCAL_COPY_CB;
                        UNLOCK_STREAM_STATE;
                        writeToStreamBuff(complexCamIndex, triggerParam->streamType, triggerParam->bufferPtr, triggerParam->mediaFrame);
                        respParam.respCode = CI_STREAM_RESP_MEDIA;

                        for (loop = 0; loop <= CI_STREAM_CLIENT_RECORD; loop++)
                        {
                            if (clientCb[loop] != NULL)
                            {
                                respParam.clientIndex = loop;
                                clientCb[loop](&respParam);
                            }
                        }
                    }
                    break;
                    /* STATE END: CI_STREAM_STATE_ON */

                    case CI_STREAM_STATE_OFF_WAIT:
                    case CI_STREAM_STATE_RESTART:
                    {
                        UNLOCK_STREAM_STATE;
                        stopAllStreamRequests(complexCamIndex, triggerParam->mediaResp);
                    }
                    break;
                }
            }
            /* If media response received as connection close */
            else if (triggerParam->mediaResp == CI_STREAM_INT_RESP_CLOSE)
            {
                switch (pStreamInfo->currState)
                {
                    default:
                    {
                        UNLOCK_STREAM_STATE;
                    }
                    break;

                    case CI_STREAM_STATE_OFF:
                    case CI_STREAM_STATE_RETRY:
                    {
                        PRINT_UNHANDLED_INFO;
                        UNLOCK_STREAM_STATE;
                    }
                    break;

                    /* STATE START: CI_STREAM_STATE_ON_WAIT */
                    case CI_STREAM_STATE_ON_WAIT:
                    {
                        PRINT_CLOSE_INFO;
                        localStreamState = CI_STREAM_STATE_OFF_WAIT;

                        // Check if This trigger is from Retry Timer
                        if (pStreamInfo->streamRetryRunning == TRUE)
                        {
                            if (pStreamInfo->retryTimerHandle == INVALID_TIMER_HANDLE)
                            {
                                UNLOCK_STREAM_STATE;
                                if (startStreamRetry(complexCamIndex) == SUCCESS)
                                {
                                    localStreamState = CI_STREAM_STATE_RETRY;
                                }
                                LOCK_STREAM_STATE;
                            }
                            else
                            {
                                localStreamState = CI_STREAM_STATE_RETRY;
                            }
                        }

                        if (localStreamState == CI_STREAM_STATE_RETRY)
                        {
                            pStreamInfo->currState = localStreamState;
                            UNLOCK_STREAM_STATE;
                            PRINT_STATE_CHANGE(localStreamState);
                        }
                        else
                        {
                            LOCAL_COPY_CB_AND_REMOVE;
                            pStreamInfo->currState = CI_STREAM_STATE_OFF_WAIT;
                            UNLOCK_STREAM_STATE;
                            stopAllStreamRequests(complexCamIndex,triggerParam->mediaResp);

                            if(CameraType(realCamIndex) == AUTO_ADDED_CAMERA)
                            {
                                respParam.respCode = CI_STREAM_RESP_CLOSE;
                            }
                            else
                            {
                                respParam.respCode = CI_STREAM_RESP_CLOSE;
                            }
                            respParam.cmdStatus = CMD_PROCESS_ERROR;

                            // Give Failure Callback To All Clients,
                            NOTIFY_CI_STREAM_CLIENTS;
                            streamInfo[realCamIndex][realVideoType].offWaitUnHandleCnt = 0;
                            CI_STREAM_END_STATE;
                            PRINT_STATE_CHANGE(localStreamState);
                        }

                        updateVideoLossStatus(realCamIndex, FALSE);
                    }
                    break;
                    /* STATE END: CI_STREAM_STATE_ON_WAIT */

                    /* STATE START: CI_STREAM_STATE_ON, CI_STREAM_STATE_RESTART */
                    case CI_STREAM_STATE_ON:
                    case CI_STREAM_STATE_RESTART:
                    {
                        PRINT_CLOSE_INFO;
                        UNLOCK_STREAM_STATE;
                        stopAllStreamRequests(complexCamIndex,triggerParam->mediaResp);

                        // Start Retry Timer
                        if (startStreamRetry(complexCamIndex) == SUCCESS)
                        {
                            // Notify Clients, that we are retrying
                            localStreamState = CI_STREAM_STATE_RETRY;
                            respParam.respCode = CI_STREAM_RESP_RETRY;
                            LOCK_STREAM_STATE;
                            LOCAL_COPY_CB;
                            pStreamInfo->currState = localStreamState;
                            UNLOCK_STREAM_STATE;
                        }
                        else
                        {
                            /* Set state OFF if camera offline else set READY */
                            CI_SET_LOCAL_STATE
                            LOCK_STREAM_STATE;
                            LOCAL_COPY_CB_AND_REMOVE;
                            pStreamInfo->currState = localStreamState;
                            UNLOCK_STREAM_STATE;
                            respParam.respCode = CI_STREAM_RESP_CLOSE;
                        }

                        NOTIFY_LIVE_STREAM_CLIENTS;
                        PRINT_STATE_CHANGE(localStreamState);
                    }
                    break;
                    /* STATE END: CI_STREAM_STATE_ON, CI_STREAM_STATE_RESTART */

                    case CI_STREAM_STATE_OFF_WAIT:
                    {
                        LOCAL_COPY_CB_AND_REMOVE;
                        UNLOCK_STREAM_STATE;
                        PRINT_CLOSE_INFO;
                        stopAllStreamRequests(complexCamIndex, triggerParam->mediaResp);
                        respParam.respCode = CI_STREAM_RESP_CLOSE;
                        NOTIFY_CI_STREAM_CLIENTS;
                        streamInfo[realCamIndex][realVideoType].offWaitUnHandleCnt = 0;
                        CI_STREAM_END_STATE;
                        PRINT_STATE_CHANGE(localStreamState);
                    }
                    break;
                }
            }
            else
            {
                /* Invalid frame response received */
                UNLOCK_STREAM_STATE;
            }
        }
        break;
        /* TRIGGER END: CI_STREAM_EXT_TRG_MEDIA_CB */

        /* TRIGGER START: CI_STREAM_EXT_TRG_CONFIG_CHANGE */
        case CI_STREAM_EXT_TRG_CONFIG_CHANGE:
        {
            ReadSingleStreamConfig(realCamIndex, &streamConfig);
            localForceConfigOnCamera = isStreamConfigValid(&streamConfig, realVideoType);
            LOCK_STREAM_STATE

            switch (pStreamInfo->currState)
            {
                default:
                {
                    UNLOCK_STREAM_STATE;
                }
                break;

                case CI_STREAM_STATE_ON_WAIT:
                case CI_STREAM_STATE_OFF_WAIT:
                case CI_STREAM_STATE_RESTART:
                {
                    PRINT_IGNORE_INFO;
                    UNLOCK_STREAM_STATE;
                }
                break;

                case CI_STREAM_STATE_OFF:
                {
                    UNLOCK_STREAM_STATE;
                    if (GetCamEventStatus(realCamIndex, CONNECTION_FAILURE) == ACTIVE)
                    {
                        internalTrigger.camIndex = complexCamIndex;
                        internalTrigger.connState = CAM_REACHABLE;
                        internalTrigger.type = CI_STREAM_EXT_TRG_CONNECTIVITY;
                        ciStreamExtTriggerHandler(&internalTrigger);
                    }
                }
                break;

                case CI_STREAM_STATE_READY:
                {
                    pStreamInfo->currState = CI_STREAM_STATE_OFF;
                    PRINT_TRG_INFO(CI_STREAM_STATE_READY);
                    PRINT_STATE_CHANGE(CI_STREAM_STATE_OFF);
                    UNLOCK_STREAM_STATE;

                    internalTrigger.camIndex = complexCamIndex;
                    internalTrigger.connState = CAM_REACHABLE;
                    internalTrigger.type = CI_STREAM_EXT_TRG_CONNECTIVITY;
                    ciStreamExtTriggerHandler(&internalTrigger);
                }
                break;

                case CI_STREAM_STATE_RETRY:
                {
                    if (localForceConfigOnCamera == TRUE)
                    {
                        pStreamInfo->forceConfigOnCamera = TRUE;
                    }
                    UNLOCK_STREAM_STATE;
                    PRINT_TRG_INFO(CI_STREAM_STATE_RETRY);
                }
                break;

                case CI_STREAM_STATE_ON:
                {
                    if (localForceConfigOnCamera == TRUE)
                    {
                        pStreamInfo->forceConfigOnCamera = TRUE;
                    }
                    pStreamInfo->currState = CI_STREAM_STATE_RESTART;
                    UNLOCK_STREAM_STATE;
                    PRINT_TRG_INFO(CI_STREAM_STATE_ON);
                    PRINT_STATE_CHANGE(CI_STREAM_STATE_RESTART);
                }
                break;
            }
        }
        break;
        /* TRIGGER END: CI_STREAM_EXT_TRG_CONFIG_CHANGE */

        /* TRIGGER START: CI_STREAM_EXT_TRG_TIMER_CB */
        case CI_STREAM_EXT_TRG_TIMER_CB:
        {
            LOCK_STREAM_STATE

            switch (pStreamInfo->currState)
            {
                default:
                {
                    UNLOCK_STREAM_STATE;
                }
                break;

                case CI_STREAM_STATE_OFF:
                case CI_STREAM_STATE_ON_WAIT:
                case CI_STREAM_STATE_ON:
                case CI_STREAM_STATE_OFF_WAIT:
                case CI_STREAM_STATE_RESTART:
                {
                    PRINT_IGNORE_INFO;
                    UNLOCK_STREAM_STATE;
                }
                break;

                /* STATE START: CI_STREAM_STATE_RETRY */
                case CI_STREAM_STATE_RETRY:
                {
                    ReadGeneralConfig(&generalConfig);
                    totalVideoLossDuration = generalConfig.preVideoLossDuration;
                    localForceConfigOnCamera = pStreamInfo->forceConfigOnCamera;
                    CALC_STREAM_CLIENTS;
                    UNLOCK_STREAM_STATE;
                    PRINT_TRG_INFO(CI_STREAM_STATE_RETRY);

                    if(CameraType(realCamIndex) == AUTO_ADDED_CAMERA)
                    {
                        localStreamState = CI_STREAM_STATE_RETRY;
                        LOCAL_COPY_CB;
                        internalTrigger.camIndex = complexCamIndex;

                        if (GetCamEventStatus(realCamIndex, CONNECTION_FAILURE) == ACTIVE)
                        {
                            internalTrigger.camIndex = complexCamIndex;

                            if (localForceConfigOnCamera == TRUE)
                            {
                                LOCK_STREAM_STATE;
                                pStreamInfo->currState = CI_STREAM_STATE_OFF;
                                UNLOCK_STREAM_STATE;

                                internalTrigger.type = CI_STREAM_EXT_TRG_CONNECTIVITY;
                                internalTrigger.connState = CAM_REACHABLE;
                                retVal = ciStreamExtTriggerHandler(&internalTrigger);
                            }
                            else
                            {
                                /* Checking if we came due to frame time out or stream config change timeout */
                                ReadSingleStreamConfig(realCamIndex, &streamConfig);
                                templocalStreamConfigF = isStreamConfigValid(&streamConfig, realVideoType);

                                if (streamReqCnt > 0)
                                {
                                    if (templocalStreamConfigF == FALSE)
                                    {
                                        LOCK_STREAM_STATE;
                                        pStreamInfo->currState = CI_STREAM_STATE_OFF;
                                        UNLOCK_STREAM_STATE;
                                        internalTrigger.type = CI_STREAM_EXT_TRG_CONNECTIVITY;
                                        internalTrigger.connState = CAM_REACHABLE;
                                    }
                                    else
                                    {
                                        internalTrigger.type = CI_STREAM_EXT_TRG_START_STREAM;
                                        internalTrigger.clientType = MAX_CI_STREAM_CLIENT;
                                    }

                                    pStreamInfo->recStrmRetryCnt++;
                                    retVal = ciStreamExtTriggerHandler(&internalTrigger);
                                }
                                else
                                {
                                    pStreamInfo->streamRetryRunning = FALSE;
                                    localStreamState = CI_STREAM_STATE_READY;
                                }
                            }
                        }
                        else
                        {
                            localStreamState = CI_STREAM_STATE_OFF;
                            pStreamInfo->recStrmRetryCnt = 0;
                            respParam.respCode = CI_STREAM_RESP_RETRY;
                            NOTIFY_RECORD_STREAM_CLIENTS;
                        }

                        if(pStreamInfo->recStrmRetryCnt >= totalVideoLossDuration)
                        {
                            respParam.respCode = CI_STREAM_RESP_RETRY;
                            NOTIFY_RECORD_STREAM_CLIENTS;
                            pStreamInfo->recStrmRetryCnt = 0;
                            pStreamInfo->streamRetryRunning = FALSE;
                        }

                        if (localStreamState != CI_STREAM_STATE_RETRY)
                        {
                            DeleteTimer(&pStreamInfo->retryTimerHandle);
                            LOCK_STREAM_STATE;
                            pStreamInfo->currState = localStreamState;
                            UNLOCK_STREAM_STATE;
                            PRINT_STATE_CHANGE(localStreamState);
                        }
                    }
                    else
                    {
                        localStreamState = CI_STREAM_STATE_RETRY;
                        LOCAL_COPY_CB;

                        if (GetCamEventStatus(realCamIndex, CONNECTION_FAILURE) == ACTIVE)
                        {
                            internalTrigger.camIndex = complexCamIndex;

                            if (localForceConfigOnCamera == TRUE)
                            {
                                LOCK_STREAM_STATE;
                                pStreamInfo->currState = CI_STREAM_STATE_OFF;
                                UNLOCK_STREAM_STATE;

                                internalTrigger.type = CI_STREAM_EXT_TRG_CONNECTIVITY;
                                internalTrigger.connState = CAM_REACHABLE;
                                retVal = ciStreamExtTriggerHandler(&internalTrigger);
                            }
                            else
                            {
                                /* Checking if we came due to frame time out or stream config change timeout */
                                ReadSingleStreamConfig(realCamIndex, &streamConfig);
                                templocalStreamConfigF = isStreamConfigValid(&streamConfig, realVideoType);

                                if (streamReqCnt > 0)
                                {
                                    /* Get stream/URL config if not available */
                                    if ((templocalStreamConfigF == FALSE) || (FALSE == isStreamUrlAvailable(&pStreamInfo->getStreamRequest)))
                                    {
                                        LOCK_STREAM_STATE;
                                        pStreamInfo->currState = CI_STREAM_STATE_OFF;
                                        UNLOCK_STREAM_STATE;
                                        internalTrigger.type = CI_STREAM_EXT_TRG_CONNECTIVITY;
                                        internalTrigger.connState = CAM_REACHABLE;
                                    }
                                    else
                                    {
                                        internalTrigger.type = CI_STREAM_EXT_TRG_START_STREAM;
                                        internalTrigger.clientType = MAX_CI_STREAM_CLIENT;
                                    }

                                    pStreamInfo->recStrmRetryCnt++;
                                    retVal = ciStreamExtTriggerHandler(&internalTrigger);
                                }
                                else
                                {
                                    pStreamInfo->streamRetryRunning = FALSE;
                                    localStreamState = CI_STREAM_STATE_READY;
                                }
                            }
                        }
                        else
                        {
                            localStreamState = CI_STREAM_STATE_OFF;
                            pStreamInfo->recStrmRetryCnt = 0;
                            respParam.respCode = CI_STREAM_RESP_RETRY;
                            NOTIFY_RECORD_STREAM_CLIENTS;
                        }

                        if(pStreamInfo->recStrmRetryCnt >= totalVideoLossDuration)
                        {
                            respParam.respCode = CI_STREAM_RESP_RETRY;
                            NOTIFY_RECORD_STREAM_CLIENTS;
                            pStreamInfo->recStrmRetryCnt = 0;
                            pStreamInfo->streamRetryRunning = FALSE;
                        }

                        if (localStreamState != CI_STREAM_STATE_RETRY)
                        {
                            DeleteTimer(&pStreamInfo->retryTimerHandle);
                            LOCK_STREAM_STATE;
                            pStreamInfo->currState = localStreamState;
                            UNLOCK_STREAM_STATE;
                            PRINT_STATE_CHANGE(localStreamState);
                        }
                    }
                }
                break;
                /* STATE END: CI_STREAM_STATE_RETRY */
            }
        }
        break;
        /* TRIGGER END: CI_STREAM_EXT_TRG_TIMER_CB */
    }

    return retVal;

#undef LOCK_STREAM_STATE
#undef UNLOCK_STREAM_STATE
#undef LOCAL_COPY_CB
#undef LOCAL_COPY_CB_AND_REMOVE
#undef NOTIFY_CI_STREAM_CLIENTS
#undef PRINT_TRG_INFO
#undef PRINT_UNHANDLED_INFO
#undef PRINT_IGNORE_INFO
#undef PRINT_CLOSE_INFO
#undef PRINT_STATE_CHANGE
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Write PTZ message in queue
 * @param   camIndex
 * @param   pPtzMsg
 * @param   emitPtzSignal
 * @return
 */
static BOOL writePtzMsg(UINT8 camIndex, PTZ_MSG_t *pPtzMsg, BOOL emitPtzSignal)
{
    UINT32  writeIdx;
    UINT32  readIdx;

    MUTEX_LOCK(ptzList[camIndex].ptzSignalLock);
    readIdx = ptzList[camIndex].ptzMagQue.readIdx;
    writeIdx = ptzList[camIndex].ptzMagQue.writeIdx + 1; /* Check for space availibility */
    if (writeIdx >= PTZ_MSG_QUEUE_MAX)
    {
        writeIdx = 0;
    }

    /* If both are same then there is no space in queue */
    if (writeIdx == readIdx)
    {
        EPRINT(CAMERA_INTERFACE, "no space in ptz msg queue: [camera=%d], [writeIdx=%d], [readIdx=%d]", camIndex, ptzList[camIndex].ptzMagQue.writeIdx, readIdx);
        MUTEX_UNLOCK(ptzList[camIndex].ptzSignalLock);
        return FAIL;
    }

    memcpy(&ptzList[camIndex].ptzMagQue.ptzMsg[ptzList[camIndex].ptzMagQue.writeIdx], pPtzMsg, sizeof(PTZ_MSG_t));
    DPRINT(CAMERA_INTERFACE, "write ptz msg: [camera=%d], [writeIdx=%d], [readIdx=%d]", camIndex, ptzList[camIndex].ptzMagQue.writeIdx, readIdx);
    ptzList[camIndex].ptzMagQue.writeIdx = writeIdx;

    if (emitPtzSignal == TRUE)
    {
        DPRINT(CAMERA_INTERFACE, "signal to ptz thread: [camera=%d], [writeIdx=%d], [readIdx=%d]", camIndex, writeIdx, readIdx);
        pthread_cond_signal(&ptzList[camIndex].ptzSignal);
    }
    MUTEX_UNLOCK(ptzList[camIndex].ptzSignalLock);

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send PTZ action message to PTZ thread
 * @param   cameraIndex
 * @param   sessionIndex
 * @param   pPtzMsg
 */
static NET_CMD_STATUS_e sendMsgToPtzCtrlThread(UINT8 cameraIndex, UINT8 sessionIndex, PTZ_MSG_t *pPtzMsg)
{
    BOOL                    ptzPauseStatus = FALSE;
    PRESET_TOUR_CONFIG_t    presetTourConfig;

    /* Check PTZ action is running or not for other client */
    MUTEX_LOCK(ptzList[cameraIndex].ptzMutex);
    if ((TRUE == ptzList[cameraIndex].runningStatus) && (sessionIndex != ptzList[cameraIndex].sessionIndex))
    {
        /* PTZ is busy with other client */
        MUTEX_UNLOCK(ptzList[cameraIndex].ptzMutex);
        DPRINT(CAMERA_INTERFACE, "ptz request is busy: [camera=%d]", cameraIndex);
        return CMD_CAM_REQUEST_IN_PROCESS;
    }
    MUTEX_UNLOCK(ptzList[cameraIndex].ptzMutex);

    /* Pause PTZ tour if active before doing any action */
    ReadSinglePresetTourConfig(cameraIndex, &presetTourConfig);
    if (presetTourConfig.activeTourOverride == ENABLE)
    {
        ptzPauseStatus = PausePtzTour(cameraIndex);
    }

    /* Update PTZ pause status */
    MUTEX_LOCK(ptzPauseState[cameraIndex].ptzPauseLock);
    ptzPauseState[cameraIndex].ptzFunctionPause = ptzPauseStatus;
    MUTEX_UNLOCK(ptzPauseState[cameraIndex].ptzPauseLock);

    /* Check again if PTZ action is running or not for other client during above checking */
    MUTEX_LOCK(ptzList[cameraIndex].ptzMutex);
    if ((TRUE == ptzList[cameraIndex].runningStatus) && (sessionIndex != ptzList[cameraIndex].sessionIndex))
    {
        MUTEX_UNLOCK(ptzList[cameraIndex].ptzMutex);
        EPRINT(CAMERA_INTERFACE, "ptz request is busy, glare condition: [camera=%d]", cameraIndex);
        return CMD_CAM_REQUEST_IN_PROCESS;
    }

    /* Add P2P message in queue */
    if (FAIL == writePtzMsg(cameraIndex, pPtzMsg, FALSE))
    {
        /* PTZ is busy with other client */
        MUTEX_UNLOCK(ptzList[cameraIndex].ptzMutex);
        return CMD_PROCESS_ERROR;
    }

    /* If thread is already running then do not start again */
    if (TRUE == ptzList[cameraIndex].runningStatus)
    {
        /* Successfully message added in queue */
        MUTEX_UNLOCK(ptzList[cameraIndex].ptzMutex);
        return CMD_SUCCESS;
    }

    /* Update thread status before starting the thread */
    ptzList[cameraIndex].runningStatus = TRUE;
    ptzList[cameraIndex].sessionIndex = sessionIndex;
    MUTEX_UNLOCK(ptzList[cameraIndex].ptzMutex);

    /* Create the ptz control thread */
    if (FAIL == Utils_CreateThread(NULL, ptzCtrlThread, &ptzList[cameraIndex], DETACHED_THREAD, PTZ_CTRL_THREAD_STACK_SZ))
    {
        /* Cleanup the thread on failure */
        ptzCtrlThreadCleanup(&ptzList[cameraIndex]);
        EPRINT(CAMERA_INTERFACE, "fail to create ptz control thread: [camera=%d]", cameraIndex);
        return CMD_PROCESS_ERROR;
    }

    /* Successfully started the thread */
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   PTZ thread cleanup
 * @param   pPtzCmdInfo
 */
static void ptzCtrlThreadCleanup(PTZ_COMMAND_INFO_t *pPtzCmdInfo)
{
    PTZ_MSG_t ptzMsg;

    MUTEX_LOCK(pPtzCmdInfo->ptzMutex);
    pPtzCmdInfo->runningStatus = FALSE;
    pPtzCmdInfo->sessionIndex = MAX_NW_CLIENT;

    /* Traverse whole queue and flush all messages */
    MUTEX_LOCK(pPtzCmdInfo->ptzSignalLock);
    while(pPtzCmdInfo->ptzMagQue.readIdx != pPtzCmdInfo->ptzMagQue.writeIdx)
    {
        /* Read ptz message from the queue. Ideally it should not happen */
        memcpy(&ptzMsg, &pPtzCmdInfo->ptzMagQue.ptzMsg[pPtzCmdInfo->ptzMagQue.readIdx], sizeof(PTZ_MSG_t));
        EPRINT(CAMERA_INTERFACE, "redundant msg in queue: [camera=%d], [writeIdx=%d], [readIdx=%d], [ptzFunction=%d]",
               pPtzCmdInfo->cameraIndex, pPtzCmdInfo->ptzMagQue.writeIdx, pPtzCmdInfo->ptzMagQue.readIdx, ptzMsg.ptzFunction);

        /* Update queue read index and handle rollover case */
        pPtzCmdInfo->ptzMagQue.readIdx++;
        if (pPtzCmdInfo->ptzMagQue.readIdx >= PTZ_MSG_QUEUE_MAX)
        {
            pPtzCmdInfo->ptzMagQue.readIdx = 0;
        }

        /* If callback is set then send success response to client (no meaning) */
        if (ptzMsg.cmdRespCallback != NULL)
        {
            ptzMsg.cmdRespCallback(CMD_SUCCESS, ptzMsg.clientSocket, TRUE);
            ptzMsg.cmdRespCallback = NULL;
        }
    }
    pPtzCmdInfo->ptzMagQue.readIdx = pPtzCmdInfo->ptzMagQue.writeIdx = 0;
    MUTEX_UNLOCK(pPtzCmdInfo->ptzSignalLock);
    MUTEX_UNLOCK(pPtzCmdInfo->ptzMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief ptzCtrlThread
 * @param threadArg
 * @return None
 */
static void *ptzCtrlThread(void* threadArg)
{
    BOOL					threadExitFlag = TRUE;
    PTZ_COMMAND_INFO_t 		*ptzInfoPtr = (PTZ_COMMAND_INFO_t *)threadArg;
    PTZ_MSG_t				ptzMsg;
    PTZ_MSG_t				prevPtzMsg = {0};
    NET_CMD_STATUS_e 		requestStatus = CMD_SUCCESS;
    IP_CAMERA_CONFIG_t		ipCamCfg;
    TIMER_INFO_t			timerInfo;
    CAMERA_REQUEST_t		*camReq = NULL;
    CAMERA_TYPE_e           camType = MAX_CAMERA_TYPE;
    UINT8                   cameraIndex = ptzInfoPtr->cameraIndex;
    BOOL                    forceStop = FALSE;
    BOOL                    stopReqPending = FALSE;

    THREAD_START_INDEX("PTZ_CTRL", cameraIndex);
    DPRINT(CAMERA_INTERFACE, "ptz control thread start: [camera=%d]", cameraIndex);

    prevPtzMsg.ptzFunction = MAX_PTZ_FUNCTION;

    while(TRUE)
    {
        /* Lock for queue data operations */
        MUTEX_LOCK(ptzInfoPtr->ptzSignalLock);

        /* If read and write index are same means queue is empty and no new operations to be performed */
        if (ptzInfoPtr->ptzMagQue.readIdx == ptzInfoPtr->ptzMagQue.writeIdx)
        {
            /* ptz operation stop timer is not running & not waiting for any response.
             * when stop request is not pending and queue is empty then nothing to do and exit from the thread */
            if ((threadExitFlag == TRUE) && (stopReqPending == FALSE))
            {
                MUTEX_UNLOCK(ptzInfoPtr->ptzSignalLock);
                IPRINT(CAMERA_INTERFACE, "ptz resp rcvd and timer is not active, exiting..!!: [camera=%d]", cameraIndex);
                break;
            }

            /* We have to wait for response or stop request timer to expire */
            if (camReq->timerHandle != INVALID_TIMER_HANDLE)
            {
                IPRINT(CAMERA_INTERFACE, "ptz queue is empty but timer is running: [camera=%d], [threadExitFlag=%d]", cameraIndex, threadExitFlag);
            }
            else
            {
                IPRINT(CAMERA_INTERFACE, "ptz queue is empty but response is pending: [camera=%d], [threadExitFlag=%d]", cameraIndex, threadExitFlag);
            }
        }

        /* Do we have to wait for response? */
        if ((threadExitFlag == FALSE) && (camReq != NULL))
        {
            /* Wait for signal. Response callback or stop timer callback will send signal to wakeup */
            DPRINT(CAMERA_INTERFACE, "ptz control sleep: [camera=%d], [writeIdx=%d], [readIdx=%d]",
                   cameraIndex, ptzInfoPtr->ptzMagQue.writeIdx, ptzInfoPtr->ptzMagQue.readIdx);

            pthread_cond_wait(&ptzInfoPtr->ptzSignal, &ptzInfoPtr->ptzSignalLock);

            DPRINT(CAMERA_INTERFACE, "ptz control run: [camera=%d], [writeIdx=%d], [readIdx=%d]",
                   cameraIndex, ptzInfoPtr->ptzMagQue.writeIdx, ptzInfoPtr->ptzMagQue.readIdx);

            /* If camera request is not busy (response received from camera) and stop timer is not running then exit from the thread if queue is empty */
            if ((FALSE == camReq->camReqBusyF) && (camReq->timerHandle == INVALID_TIMER_HANDLE))
            {
                /* All operations are performed */
                threadExitFlag = TRUE;
            }

            /* Unlock the mutex to acquire it again */
            MUTEX_UNLOCK(ptzInfoPtr->ptzSignalLock);
            continue;
        }

        /* Read ptz message if queue is not empty */
        if (ptzInfoPtr->ptzMagQue.readIdx != ptzInfoPtr->ptzMagQue.writeIdx)
        {
            ptzMsg = ptzInfoPtr->ptzMagQue.ptzMsg[ptzInfoPtr->ptzMagQue.readIdx];
            forceStop = FALSE;
        }
        else
        {
            forceStop = TRUE;
            DPRINT(CAMERA_INTERFACE, "force stop request : [function=%d], [pan=%d], [tilt=%d], [zoom=%d]",
                   prevPtzMsg.ptzFunction, prevPtzMsg.pan, prevPtzMsg.tilt, prevPtzMsg.zoom);
        }

        /* Check if change in ptz actoion */
        /* NOTE : We need to compare pan values because pan tilt enum is common */
        if ((forceStop == TRUE) || ((prevPtzMsg.ptzFunction != MAX_PTZ_FUNCTION) && (prevPtzMsg.action != STOP) && ((ptzMsg.ptzFunction != prevPtzMsg.ptzFunction) ||
                ((prevPtzMsg.ptzFunction == PAN_TILT_FUNCTION) && ((ptzMsg.pan != prevPtzMsg.pan) || (ptzMsg.tilt != prevPtzMsg.tilt))))))
        {
            /* Prepare force stop request for previous command */
            ptzMsg = prevPtzMsg;
            ptzMsg.speed = MAX_PTZ_SPEED_STEPS;
            ptzMsg.cmdRespCallback = NULL;
            ptzMsg.clientSocket = INVALID_CONNECTION;
            ptzMsg.action = STOP;
            stopReqPending = FALSE;

            if (forceStop == FALSE)
            {
                DPRINT(CAMERA_INTERFACE, "stop request on action change : [function=%d], [pan=%d], [tilt=%d], [zoom=%d]",
                       prevPtzMsg.ptzFunction, prevPtzMsg.pan, prevPtzMsg.tilt, prevPtzMsg.zoom);
            }
        }
        else
        {
            /* Update read index and handle queue rollover case */
            ptzInfoPtr->ptzMagQue.readIdx++;

            if (ptzInfoPtr->ptzMagQue.readIdx >= PTZ_MSG_QUEUE_MAX)
            {
                /* Rollover ptz queue read index */
                ptzInfoPtr->ptzMagQue.readIdx = 0;
            }
        }
        MUTEX_UNLOCK(ptzInfoPtr->ptzSignalLock);

        /* Get the camera request as per the ptz operation type */
        if ((ptzMsg.ptzFunction == PAN_TILT_FUNCTION) || (ptzMsg.ptzFunction == ZOOM_FUNCTION))
        {
            camReq = getCameraReqStruct(CAM_REQ_STRUCT_PTZ, cameraIndex);
        }
        else if (ptzMsg.ptzFunction == FOCUS_FUNCTION)
        {
            camReq = getCameraReqStruct(CAM_REQ_STRUCT_FOCUS, cameraIndex);
        }
        else if (ptzMsg.ptzFunction == IRIS_FUNCTION)
        {
            camReq = getCameraReqStruct(CAM_REQ_STRUCT_IRIS, cameraIndex);
        }
        else
        {
            EPRINT(CAMERA_INTERFACE, "invld ptz function: [camera=%d], [ptzFunction=%d]", cameraIndex, ptzMsg.ptzFunction);
            break;
        }

        /* If it is not ptz operation stop request then start ptz operation timer.
         * On timer expiry, callback will send cond signal */
        MUTEX_LOCK(camReq->camReqFlagLock);
        if (ptzMsg.action == START)
        {
            stopReqPending = TRUE;
            timerInfo.data = ((ptzMsg.ptzFunction << 8) | cameraIndex);

            /* Providing extra 100 ms time for initial timer to ensure minimum 300 ms delay */
            timerInfo.count = CONVERT_MSEC_TO_TIMER_COUNT((TIMER_COUNT_FOR_PTZ_REQ + ((prevPtzMsg.ptzFunction == MAX_PTZ_FUNCTION) ? 100 : 0)));
            timerInfo.funcPtr = ptzControlTimerCb;

            ptzList[cameraIndex].stopTimeoutCnt = 0;

            DeleteTimer(&camReq->timerHandle);
            StartTimer(timerInfo, &camReq->timerHandle);
        }
        MUTEX_UNLOCK(camReq->camReqFlagLock);

        /* Get camera type and read ip camera configuration for request */
        ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);
        camType = CameraType(cameraIndex);
        requestStatus = CMD_SUCCESS;
        threadExitFlag = TRUE;
        prevPtzMsg = ptzMsg;

        /* Make camera request busy and init other params */
        MUTEX_LOCK(camReq->camReqFlagLock);
        camReq->camReqBusyF = BUSY;
        MUTEX_UNLOCK(camReq->camReqFlagLock);

        camReq->clientCb[ON] = ptzMsg.cmdRespCallback;
        camReq->clientSocket[ON] = ptzMsg.clientSocket;
        camReq->requestCount = 0;
        camReq->requestStatus = CMD_PROCESS_ERROR;

        switch(ptzMsg.ptzFunction)
        {
            case PAN_TILT_FUNCTION:
            case ZOOM_FUNCTION:
            {
                requestStatus = ValidatePtzFuncSupport(cameraIndex, ptzMsg.ptzFunction);
                if (requestStatus != CMD_SUCCESS)
                {
                    break;
                }

                DPRINT(CAMERA_INTERFACE, "pan tilt zoom function: [camera=%d], [function=%d], [pan=%d], [tilt=%d], [zoom=%d], [speed=%d], [action=%d]",
                       cameraIndex, ptzMsg.ptzFunction, ptzMsg.pan, ptzMsg.tilt, ptzMsg.zoom, ptzMsg.speed, ptzMsg.action);
                if (ipCamCfg.onvifSupportF == TRUE)
                {
                    /* Send ptz request to camera */
                    requestStatus = sendOnvifPtzCmd(cameraIndex, ONVIF_SET_PTZ, onvifSetPtzCb, &ipCamCfg, ptzMsg.pan, ptzMsg.tilt,
                                                    ptzMsg.zoom, ptzMsg.action, ((float)1.0 * ptzMsg.speed), MAX_FOCUS_OPTION, MAX_IRIS_OPTION);
                }
                else if (camType == AUTO_ADDED_CAMERA)
                {
                    /* Get ptz request url of camera */
                    GetCiRequestUrl(cameraIndex, REQ_URL_SET_PTZ, 0, CAM_REQ_CONTROL, tcpSetPtzControlCb, camReq, &ptzMsg);

                    /* Send ptz request to camera */
                    requestStatus =	sendReqToCamera(camReq, &ipCamCfg, MAIN_STREAM);
                }
                else
                {
                    /* Get ptz request url of camera */
                    requestStatus = GetCameraBrandModelUrl(cameraIndex, REQ_URL_SET_PTZ, camReq, &ptzMsg, NULL, NULL, NULL);
                    if (requestStatus != CMD_SUCCESS)
                    {
                        EPRINT(CAMERA_INTERFACE, "fail to get ptz request url: [camera=%d], [status=%d]", cameraIndex, requestStatus);
                        break;
                    }

                    /* Is any request url prepared? */
                    if (camReq->numOfRequest == 0)
                    {
                        requestStatus = CMD_PROCESS_ERROR;
                        break;
                    }

                    /* Send ptz request to camera */
                    requestStatus =	sendReqToCamera(camReq, &ipCamCfg, MAIN_STREAM);
                }
            }
            break;

            case FOCUS_FUNCTION:
            {
                requestStatus = ValidatePtzFuncSupport(cameraIndex, ptzMsg.ptzFunction);
                if (requestStatus != CMD_SUCCESS)
                {
                    break;
                }

                DPRINT(CAMERA_INTERFACE, "focus function: [camera=%d], [function=%d], [focus=%d], [action=%d]", cameraIndex, ptzMsg.ptzFunction, ptzMsg.focus, ptzMsg.action);
                if(ipCamCfg.onvifSupportF == TRUE)
                {
                    /* Send focus request to camera */
                    requestStatus = sendOnvifPtzCmd(cameraIndex, ONVIF_SET_FOCUS, onvifSetFocusCb, &ipCamCfg,
                                                    MAX_PTZ_PAN_OPTION, MAX_PTZ_TILT_OPTION, MAX_PTZ_ZOOM_OPTION, ptzMsg.action, ptzMsg.speed, ptzMsg.focus, MAX_IRIS_OPTION);
                }
                else if(camType == AUTO_ADDED_CAMERA)
                {
                    /* Get focus request url of camera */
                    GetCiRequestUrl(cameraIndex, REQ_URL_SET_FOCUS, 0, CAM_REQ_CONTROL, tcpSetFocusControlCb, camReq, &ptzMsg);

                    /* Send focus request to camera */
                    requestStatus =	sendReqToCamera(camReq, &ipCamCfg, MAIN_STREAM);
                }
                else
                {
                    /* Get focus request url of camera */
                    requestStatus = GetCameraBrandModelUrl(cameraIndex, REQ_URL_SET_FOCUS, camReq, &ptzMsg, NULL, NULL, NULL);
                    if (requestStatus != CMD_SUCCESS)
                    {
                        EPRINT(CAMERA_INTERFACE, "set focus request failed: [camera=%d]", cameraIndex);
                        break;
                    }

                    /* Is any request url prepared? */
                    if (camReq->numOfRequest == 0)
                    {
                        requestStatus = CMD_PROCESS_ERROR;
                        break;
                    }

                    /* Send focus request to camera */
                    requestStatus =	sendReqToCamera(camReq, &ipCamCfg, MAIN_STREAM);
                }
            }
            break;

            case IRIS_FUNCTION:
            {
                requestStatus = ValidatePtzFuncSupport(cameraIndex, ptzMsg.ptzFunction);
                if (requestStatus != CMD_SUCCESS)
                {
                    break;
                }

                DPRINT(CAMERA_INTERFACE, "iris function: [camera=%d], [function=%d], [iris=%d], [action=%d]", cameraIndex, ptzMsg.ptzFunction, ptzMsg.iris, ptzMsg.action);
                if(ipCamCfg.onvifSupportF == TRUE)
                {
                    /* Send iris request to camera */
                    requestStatus = sendOnvifPtzCmd(cameraIndex, ONVIF_SET_IRIS, onvifSetIrisCb, &ipCamCfg,
                                                    MAX_PTZ_PAN_OPTION, MAX_PTZ_TILT_OPTION, MAX_PTZ_ZOOM_OPTION, ptzMsg.action, 0, MAX_FOCUS_OPTION, ptzMsg.iris);
                }
                else if(camType == AUTO_ADDED_CAMERA)
                {
                    /* Matrix camera doesn't support iris operation */
                    requestStatus =	CMD_FEATURE_NOT_SUPPORTED;
                }
                else
                {
                    /* Get iris request url of camera */
                    requestStatus = GetCameraBrandModelUrl(cameraIndex, REQ_URL_SET_IRIS, camReq, &ptzMsg, NULL, NULL, NULL);
                    if (requestStatus != CMD_SUCCESS)
                    {
                        EPRINT(CAMERA_INTERFACE, "set iris request failed: [camera=%d]", cameraIndex);
                        break;
                    }

                    /* Is any request url prepared? */
                    if (camReq->numOfRequest == 0)
                    {
                        requestStatus = CMD_PROCESS_ERROR;
                        break;
                    }

                    /* Send iris request to camera */
                    requestStatus =	sendReqToCamera(camReq, &ipCamCfg, MAIN_STREAM);
                }
            }
            break;

            default:
            {
                /* Invalid operation */
                requestStatus = CMD_PROCESS_ERROR;
            }
            break;
        }

        /* Is request send successfully? */
        if (requestStatus == CMD_SUCCESS)
        {
            /* Wait for the response of ptz operation request */
            threadExitFlag = FALSE;
            continue;
        }

        /* We don't have to send request (feature not supported, no need to perform operation, failure, etc.)
         * Free request related resource and delete the stop operation request timer */
        MUTEX_LOCK(camReq->camReqFlagLock);
        camReq->camReqBusyF = FREE;
        DeleteTimer(&camReq->timerHandle);
        MUTEX_UNLOCK(camReq->camReqFlagLock);

        camReq->clientCb[ON] = NULL;
        camReq->clientSocket[ON] = INVALID_CONNECTION;

        DPRINT(CAMERA_INTERFACE, "ptz request not sent: [camera=%d], [status=%d], [pan=%d], [tilt=%d], [zoom=%d], [focus=%d], [iris=%d], [speed=%d]",
               cameraIndex, requestStatus, ptzMsg.pan, ptzMsg.tilt, ptzMsg.zoom, ptzMsg.focus, ptzMsg.iris, ptzMsg.speed);

        /* Is response pending to provide the client? */
        if (ptzMsg.cmdRespCallback != NULL)
        {
            /* Give response to the client */
            ptzMsg.cmdRespCallback(requestStatus, ptzMsg.clientSocket, TRUE);
            ptzMsg.cmdRespCallback = NULL;
        }
    }

    /* Cleanup ptz thread */
    ptzCtrlThreadCleanup(ptzInfoPtr);
    DPRINT(CAMERA_INTERFACE, "ptz control thread exit: [camera=%d]", cameraIndex);
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief ptzControlTimerCb
 * @param userData
 */
static void ptzControlTimerCb(UINT32 userData)
{
    UINT32				cameraIndex = (userData & 0x000000FF);
    PTZ_FUNCTION_e		ptzFunction = ((userData >> 8) & 0x000000FF);
    CAMERA_REQUEST_t	*camReq = NULL;

    if (cameraIndex >= getMaxCameraForCurrentVariant())
    {
        EPRINT(CAMERA_INTERFACE, "invld camera: [camera=%d]", cameraIndex);
        return;
    }

    switch (ptzFunction)
    {
        case PAN_TILT_FUNCTION:
        case ZOOM_FUNCTION:
            camReq = getCameraReqStruct(CAM_REQ_STRUCT_PTZ, cameraIndex);
            break;

        case FOCUS_FUNCTION:
            camReq = getCameraReqStruct(CAM_REQ_STRUCT_FOCUS, cameraIndex);
            break;

        case IRIS_FUNCTION:
            camReq = getCameraReqStruct(CAM_REQ_STRUCT_IRIS, cameraIndex);
            break;

        default:
            return;
    }

    /* PARASOFT : Rule CERT_C-STR31-a marked false positive */
    MUTEX_LOCK(camReq->camReqFlagLock);
    if((camReq->camReqBusyF == TRUE) && (ptzList[cameraIndex].stopTimeoutCnt < MAX_PTZ_REQ_TIMEOUT))
    {
        /* Is it first retry? then update the timer */
        if (ptzList[cameraIndex].stopTimeoutCnt == 0)
        {
            /* Relaod timer with 100ms for every retry */
            ReloadTimer(camReq->timerHandle, CONVERT_MSEC_TO_TIMER_COUNT(TIMER_COUNT_FOR_PTZ_REQ_RETRY));
        }
        
        ptzList[cameraIndex].stopTimeoutCnt++;
        WPRINT(CAMERA_INTERFACE, "timer expired but ptz operation resp not rcvd yet: [camera=%d], [ptzFunction=%d], [stopTimeoutCnt=%d]",
               cameraIndex, ptzFunction, ptzList[cameraIndex].stopTimeoutCnt);
        MUTEX_UNLOCK(camReq->camReqFlagLock);
    }
    else
    {
        if (ptzList[cameraIndex].stopTimeoutCnt >= MAX_PTZ_REQ_TIMEOUT)
        {
            EPRINT(CAMERA_INTERFACE, "maximum time out reached for ptz request: [camera=%d], [ptzFunction=%d], [stopTimeoutCnt=%d]",
                   cameraIndex, ptzFunction, ptzList[cameraIndex].stopTimeoutCnt);
            camReq->camReqBusyF = FALSE;
        }

        DeleteTimer(&camReq->timerHandle);
        MUTEX_UNLOCK(camReq->camReqFlagLock);

        /* Send signal to ptzCtrlThread */
        /* PARASOFT: BD-TRS-DIFCS Variable used in multiple critical sections */
        MUTEX_LOCK(ptzList[cameraIndex].ptzSignalLock);
        pthread_cond_signal(&ptzList[cameraIndex].ptzSignal);
        MUTEX_UNLOCK(ptzList[cameraIndex].ptzSignalLock);
        WPRINT(CAMERA_INTERFACE, "ptz control timer timeout: [camera=%d], [ptzFunction=%d]", cameraIndex, ptzFunction);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief ValidatePtzFuncSupport
 * @param cameraIndex
 * @param ptzFunction
 * @return Command Status
 */
NET_CMD_STATUS_e ValidatePtzFuncSupport(UINT8 cameraIndex, PTZ_FUNCTION_e ptzFunction)
{
    IP_CAMERA_CONFIG_t		ipCamCfg;
    CAMERA_BRAND_e			brand = MAX_CAMERA_BRAND;
    CAMERA_MODEL_e			model = CAMERA_MODEL_NONE;

    /* Get camera type and read ip camera configuration for request */
    ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);

    if (ipCamCfg.onvifSupportF == TRUE)
    {
        /* Are brand and model of camera valid? */
        if ((ipCamCfg.brand[0] == '\0') || (ipCamCfg.model[0] == '\0'))
        {
            /* Onvif capability is not received yet */
            EPRINT(CAMERA_INTERFACE, "onvif capability not recv, brand-model unknown: [camera=%d]", cameraIndex);
            return CMD_ONVIF_CAM_CAPABILITY_ERROR;
        }

        return ((ValidatePtzFuncForOnvif(cameraIndex, ptzFunction) == SUCCESS) ? CMD_SUCCESS : CMD_FEATURE_NOT_SUPPORTED);
     }
    else
    {
        /* Get camera brand and model */
        GetCameraBrandModelNumber(cameraIndex, &brand, &model);

        /* Check ptz support in camera */
        return ((ValidatePtzFuncForBrandModel(brand, model, ptzFunction) == SUCCESS) ? CMD_SUCCESS : CMD_FEATURE_NOT_SUPPORTED);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief ValidatePtzFuncForBrandModel
 * @param brand
 * @param model
 * @param ptzFunction
 * @return SUCCESS/FAIL
 */
static BOOL ValidatePtzFuncForBrandModel(CAMERA_BRAND_e brand, CAMERA_MODEL_e model, PTZ_FUNCTION_e ptzFunction)
{
    if (ptzFunction == ZOOM_FUNCTION)
    {
        return ValidatePTZFeatureForBrandModel(brand, model, PTZ_ZOOM_CONTROL);
    }

    if (ptzFunction == FOCUS_FUNCTION)
    {
        return ValidatePTZFeatureForBrandModel(brand, model, PTZ_FOCUS_CONTROL);
    }

    if (ptzFunction == IRIS_FUNCTION)
    {
        return ValidatePTZFeatureForBrandModel(brand, model, PTZ_IRIS_CONTROL);
    }

    if (ptzFunction == PAN_TILT_FUNCTION)
    {
        if ((ValidatePTZFeatureForBrandModel(brand, model, PTZ_PAN_CONTROL) == SUCCESS)
                && (ValidatePTZFeatureForBrandModel(brand, model, PTZ_TILT_CONTROL) == SUCCESS))
        {
            return SUCCESS;
        }
    }

    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief onvifMediaProfileUpdateCb
 * @param responseData
 * @return SUCCESS
 */
static BOOL onvifMediaProfileUpdateCb(ONVIF_RESPONSE_PARA_t *responseData)
{
    UINT8           cameraIndex = GET_STREAM_INDEX(responseData->cameraIndex);
    VIDEO_TYPE_e    streamType = GET_STREAM_TYPE(responseData->cameraIndex);

    MUTEX_LOCK(profileShedularProcParam[cameraIndex][streamType].profileSchedularProcRunLock);
    profileShedularProcParam[cameraIndex][streamType].profileSchedularProcRunF = FALSE;
    MUTEX_UNLOCK(profileShedularProcParam[cameraIndex][streamType].profileSchedularProcRunLock);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief Update profile related parameter on schedule time.
 * @param data
 */
static void profileSchedulerCb(UINT32 data)
{
    UINT8				cameraIndex;
    UINT8				streamType;
    CAMERA_CONFIG_t		cameraConfig;
    UINT8				streamProfileIndex = 0;

    for(cameraIndex = 0; cameraIndex < getMaxCameraForCurrentVariant(); cameraIndex++)
    {
        ReadSingleCameraConfig(cameraIndex, &cameraConfig);
        if ((cameraConfig.camera == DISABLE) || (GetCamEventStatus(cameraIndex, CONNECTION_FAILURE) == INACTIVE))
        {
            continue;
        }

        for (streamType = MAIN_STREAM; streamType < MAX_STREAM; streamType++)
        {
            getStreamProfileNum(streamType, cameraIndex, &streamProfileIndex);
            if (CMD_SUCCESS != GetProfileParam(cameraIndex, streamProfileIndex, streamType, NULL, INVALID_CONNECTION, CLIENT_CB_TYPE_NATIVE))
            {
                continue;
            }

            MUTEX_LOCK(profileShedularProcParam[cameraIndex][streamType].profileSchedularProcRunLock);
            profileShedularProcParam[cameraIndex][streamType].profileSchedularProcRunF = TRUE;
            MUTEX_UNLOCK(profileShedularProcParam[cameraIndex][streamType].profileSchedularProcRunLock);
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief tcpSetOverlayCnfgCb
 * @param tcpHandle
 * @param dataInfo
 */
static void tcpSetOverlayCnfgCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo)
{
    UINT8 cameraIndex = (GET_STREAM_INDEX(dataInfo->camIndex));

    restoreDefaultOsdParam(cameraIndex);
    MUTEX_LOCK(setOsdsRequest[cameraIndex].request.camReqFlagLock);
    setOsdsRequest[cameraIndex].request.camReqBusyF = FREE;
    MUTEX_UNLOCK(setOsdsRequest[cameraIndex].request.camReqFlagLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief tcpSetDateTimeCb
 * @param tcpHandle
 * @param dataInfo
 */
static void tcpSetDateTimeCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo)
{
    UINT8 cameraIndex = (GET_STREAM_INDEX(dataInfo->camIndex));

    switch(dataInfo->tcpResp)
    {
        case TCP_CLOSE_ON_SUCCESS:
            DPRINT(CAMERA_INTERFACE, "date-time updated sucessfully: [camera=%d]", cameraIndex);
            break;

        default:
            EPRINT(CAMERA_INTERFACE, "date-time sync failed: [camera=%d], [status=%d]", cameraIndex, dataInfo->tcpResp);
            break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief tcpStartSndAudReqCb : This function is a call back to the start snd aud to cam req
 * @param tcpHandle
 * @param dataInfo
 */
static void tcpStartSndAudReqCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo)
{
    UINT8 cameraIndex = (GET_STREAM_INDEX(dataInfo->camIndex));

    switch(dataInfo->ciCmdResp)
    {
        case CI_CMD_SUCCESS:
            DPRINT(CAMERA_INTERFACE, "start audio to camera request success: [camera=%d], [fd=%d], [handle=%d]", cameraIndex, dataInfo->connFd, tcpHandle);
            UpdateAudioSendFd(dataInfo->connFd);
            break;

        case CI_CMD_AUD_OUT_IN_PROGRESS:
            EPRINT(CAMERA_INTERFACE, "start audio to camera request failed, already busy: [camera=%d]", cameraIndex);
            ProcTxAudioToCameraSetupFailure(CMD_AUD_CHANNEL_BUSY);
            break;

        case CI_CMD_AUDIO_DISABLE:
            EPRINT(CAMERA_INTERFACE, "start audio to camera request failed, audio disabled, check settings: [camera=%d]", cameraIndex);
            ProcTxAudioToCameraSetupFailure(CMD_AUDIO_DISABLED);
            break;

        case CI_CMD_PROCESS_ERROR:
            EPRINT(CAMERA_INTERFACE, "start audio to camera request failed, processing error: [camera=%d]", cameraIndex);
            ProcTxAudioToCameraSetupFailure(CMD_PROCESS_ERROR);
            break;

        default:
            EPRINT(CAMERA_INTERFACE, "start audio to camera request failed, unknown error: [camera=%d], [CmdResp=%d]", cameraIndex, dataInfo->ciCmdResp);
            ProcTxAudioToCameraSetupFailure(CMD_PROCESS_ERROR);
            break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief tcpStopSndAudReqCb
 * @param tcpHandle
 * @param dataInfo
 */
static void tcpStopSndAudReqCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo)
{
    UINT8 cameraIndex = (GET_STREAM_INDEX(dataInfo->camIndex));

    switch(dataInfo->tcpResp)
    {
        case TCP_CLOSE_ON_SUCCESS:
            DPRINT(CAMERA_INTERFACE, "stop audio to camera request success: [camera=%d], [fd=%d], [handle=%d]", cameraIndex, dataInfo->connFd, tcpHandle);
            break;
        default:
            EPRINT(CAMERA_INTERFACE, "stop audio to camera request failed: [camera=%d], [status=%d]", cameraIndex, dataInfo->tcpResp);
            break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief tcpGetMotionCnfgCb
 * @param tcpHandle
 * @param dataInfo
 */
static void tcpGetMotionCnfgCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo)
{
    UINT8 cameraIndex = GET_STREAM_INDEX(dataInfo->camIndex);

    switch(dataInfo->tcpResp)
    {
        case TCP_CLOSE_ON_SUCCESS:
        case TCP_SUCCESS:
        {
            if (dataInfo->ciCfgData == NULL)
            {
                break;
            }

            /* Save data to global structure */
            MOTION_BLOCK_METHOD_PARAM_t *pMotionConfig = dataInfo->ciCfgData;
            pMotionConfig->noMotionSupportF = IsNoMotionEventSupported(cameraIndex);
            motionWindowRequest[cameraIndex].motionParam = *pMotionConfig;

            /* Send motion area config response to client */
            sendGetMotionWindowResponseToClient(cameraIndex, CMD_SUCCESS);
            FREE_MEMORY(dataInfo->ciCfgData);
        }
        break;

        case TCP_CLOSE_ON_ERROR:
        case TCP_ERROR:
        {
            if (motionWindowRequest[cameraIndex].request.clientCb[ON] != NULL)
            {
                ((NW_CMD_REPLY_CB)motionWindowRequest[cameraIndex].request.clientCb[ON])
                        (CMD_PROCESS_ERROR, motionWindowRequest[cameraIndex].request.clientSocket[ON], TRUE);
                motionWindowRequest[cameraIndex].request.clientCb[ON] = NULL;
            }
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }

    restoreDefaultMotionConfig(cameraIndex);
    MUTEX_LOCK(motionWindowRequest[cameraIndex].request.camReqFlagLock);
    motionWindowRequest[cameraIndex].request.camReqBusyF = FREE;
    MUTEX_UNLOCK(motionWindowRequest[cameraIndex].request.camReqFlagLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief tcpSetMotionCnfgCb
 * @param tcpHandle
 * @param dataInfo
 * @return
 */
static void tcpSetMotionCnfgCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo)
{

    UINT8               requestCount;
    UINT8               cameraIndex = GET_STREAM_INDEX(dataInfo->camIndex);
    CAMERA_CONFIG_t     cameraCnfg;
    IP_CAMERA_CONFIG_t  ipCamCfg;
    BOOL                sessionClose = FALSE;

    switch(dataInfo->tcpResp)
    {
        case TCP_SUCCESS:
        default:
        {
            /* Nothing to do */
        }
        break;

        case TCP_CLOSE_ON_SUCCESS:
        case TCP_CLOSE_ON_ERROR:
        case TCP_ERROR:
        {
            for (requestCount = 0; requestCount < motionWindowRequest[cameraIndex].request.numOfRequest; requestCount++)
            {
                if (motionWindowRequest[cameraIndex].request.http[requestCount] == tcpHandle)
                {
                    requestCount++;
                    break;
                }
            }

            if (requestCount < motionWindowRequest[cameraIndex].request.numOfRequest)
            {
                motionWindowRequest[cameraIndex].request.requestCount++;
                motionWindowRequest[cameraIndex].request.http[requestCount-1] = INVALID_HTTP_HANDLE;
                ReadSingleIpCameraConfig(cameraIndex,&ipCamCfg);
                if (sendReqToCamera(&motionWindowRequest[cameraIndex].request, &ipCamCfg, MAIN_STREAM) != CMD_SUCCESS)
                {
                    sessionClose = TRUE;
                }
            }

            if ((sessionClose == FALSE) && (requestCount == motionWindowRequest[cameraIndex].request.numOfRequest))
            {
                motionWindowRequest[cameraIndex].request.requestStatus = CMD_SUCCESS;
                ReadSingleCameraConfig(cameraIndex, &cameraCnfg);

                /*updating motion detection status*/
                cameraCnfg.motionDetectionStatus = motionWindowRequest[cameraIndex].configStatus;

                pthread_rwlock_wrlock(&camCnfgNotifyControl[cameraIndex].configNotifyLock);
                camCnfgNotifyControl[cameraIndex].cameraConfigNotifyF = FALSE;
                pthread_rwlock_unlock(&camCnfgNotifyControl[cameraIndex].configNotifyLock);
                WriteSingleCameraConfig(cameraIndex,&cameraCnfg);
                pthread_rwlock_wrlock(&camCnfgNotifyControl[cameraIndex].configNotifyLock);
                camCnfgNotifyControl[cameraIndex].cameraConfigNotifyF = TRUE;
                pthread_rwlock_unlock(&camCnfgNotifyControl[cameraIndex].configNotifyLock);
                sessionClose = TRUE;
            }

            if(sessionClose == TRUE)
            {
                if (motionWindowRequest[cameraIndex].request.clientCb[ON] != NULL)
                {
                    ((NW_CMD_REPLY_CB)motionWindowRequest[cameraIndex].request.clientCb[ON])
                            (motionWindowRequest[cameraIndex].request.requestStatus, motionWindowRequest[cameraIndex].request.clientSocket[ON], TRUE);
                    motionWindowRequest[cameraIndex].request.clientCb[ON] = NULL;
                }

                restoreDefaultMotionConfig(cameraIndex);
                MUTEX_LOCK(motionWindowRequest[cameraIndex].request.camReqFlagLock);
                motionWindowRequest[cameraIndex].request.camReqBusyF = FREE;
                MUTEX_UNLOCK(motionWindowRequest[cameraIndex].request.camReqFlagLock);
            }
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief tcpGetMaxPrivacyMaskCnfgCb
 * @param tcpHandle
 * @param dataInfo
 * @return
 */
static void tcpGetMaxPrivacyMaskCnfgCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo)
{
    UINT8           cameraIndex = GET_STREAM_INDEX(dataInfo->camIndex);
    UINT8           maxPrivacyCnt;
    CAMERA_BRAND_e  brand = CAMERA_BRAND_NONE;
    CAMERA_MODEL_e  model = CAMERA_MODEL_NONE;

    privacyMaskRequest[cameraIndex].maxSupportedMaskWindow = 0;
    switch(dataInfo->tcpResp)
    {
        case TCP_CLOSE_ON_SUCCESS:
        case TCP_SUCCESS:
        {
            if (dataInfo->ciCfgData == NULL)
            {
                break;
            }

            CI_OVERLAY_CONFIG_t *pOverLayConfig = dataInfo->ciCfgData;

            GetCiCameraBrandModelNumber(cameraIndex, &brand, &model);

            /* Get max supported privacy mask from database */
            privacyMaskRequest[cameraIndex].maxSupportedMaskWindow = GetMaxSupportedPrivacyMaskWindow(brand, model);

            memset(privacyMaskRequest[cameraIndex].privacyArea, 0, sizeof(privacyMaskRequest[cameraIndex].privacyArea));
            for(maxPrivacyCnt = 0; maxPrivacyCnt < privacyMaskRequest[cameraIndex].maxSupportedMaskWindow; maxPrivacyCnt++)
            {
                if (pOverLayConfig->privacyMaskArea[maxPrivacyCnt].maskEnable == DISABLE)
                {
                    continue;
                }

                privacyMaskRequest[cameraIndex].privacyArea[maxPrivacyCnt].startXPoint = pOverLayConfig->privacyMaskArea[maxPrivacyCnt].startXPoint;
                privacyMaskRequest[cameraIndex].privacyArea[maxPrivacyCnt].startYPoint = pOverLayConfig->privacyMaskArea[maxPrivacyCnt].startYPoint;
                privacyMaskRequest[cameraIndex].privacyArea[maxPrivacyCnt].width = pOverLayConfig->privacyMaskArea[maxPrivacyCnt].width;
                privacyMaskRequest[cameraIndex].privacyArea[maxPrivacyCnt].height = pOverLayConfig->privacyMaskArea[maxPrivacyCnt].height;
            }

            if (IsPrivacyMaskMappingRequired(brand, model) == TRUE)
            {
                privacyMaskRequest[cameraIndex].status = MapPrivacyMaskCameraToNvr(brand, model, privacyMaskRequest[cameraIndex].privacyArea);
                if (FAIL == privacyMaskRequest[cameraIndex].status)
                {
                    EPRINT(CAMERA_INTERFACE, "fail to map privacy mask: [camera=%d], [brand=%d], [model=%d]", cameraIndex, brand, model);
                }
            }

            sendGetPrivacyMaskResponseToClient(cameraIndex, (privacyMaskRequest[cameraIndex].status == FAIL) ? CMD_ONVIF_CAM_CAPABILITY_ERROR : CMD_SUCCESS);
            FREE_MEMORY(dataInfo->ciCfgData);
        }
        break;

        case TCP_CLOSE_ON_ERROR:
        case TCP_ERROR:
        {
            if (privacyMaskRequest[cameraIndex].request.clientCb[ON] != NULL)
            {
                ((NW_CMD_REPLY_CB)privacyMaskRequest[cameraIndex].request.clientCb[ON])
                        (CMD_PROCESS_ERROR, privacyMaskRequest[cameraIndex].request.clientSocket[ON], TRUE);
                privacyMaskRequest[cameraIndex].request.clientCb[ON] = NULL;
            }
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }

    restoreDefaultPrivacyMaskConfig(cameraIndex);
    MUTEX_LOCK(privacyMaskRequest[cameraIndex].request.camReqFlagLock);
    privacyMaskRequest[cameraIndex].request.camReqBusyF = FREE;
    MUTEX_UNLOCK(privacyMaskRequest[cameraIndex].request.camReqFlagLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief tcpSetPrivacyMaskCnfgCb
 * @param tcpHandle
 * @param dataInfo
 */
static void tcpSetPrivacyMaskCnfgCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo)
{
    NET_CMD_STATUS_e    requestStatus = CMD_SUCCESS;
    UINT8               requestCount;
    UINT8               cameraIndex = GET_STREAM_INDEX(dataInfo->camIndex);
    CAMERA_CONFIG_t     cameraCnfg;
    IP_CAMERA_CONFIG_t  ipCamCfg;
    BOOL                sessionClose = FALSE;

    switch(dataInfo->tcpResp)
    {
        case TCP_SUCCESS:
        default:
        {
            /* Nothing to do */
        }
        break;

        case TCP_CLOSE_ON_SUCCESS:
        case TCP_CLOSE_ON_ERROR:
        case TCP_ERROR:
        {
            for (requestCount = 0; requestCount < privacyMaskRequest[cameraIndex].request.numOfRequest; requestCount++)
            {
                if (privacyMaskRequest[cameraIndex].request.http[requestCount] == tcpHandle)
                {
                    requestCount++;
                    break;
                }
            }

            if (dataInfo->tcpResp == TCP_CLOSE_ON_SUCCESS)
            {
                privacyMaskRequest[cameraIndex].request.requestStatus = CMD_SUCCESS;
            }
            else
            {
                privacyMaskRequest[cameraIndex].request.requestStatus = CMD_PROCESS_ERROR;
            }

            if (requestCount < privacyMaskRequest[cameraIndex].request.numOfRequest)
            {
                privacyMaskRequest[cameraIndex].request.requestCount++;
                privacyMaskRequest[cameraIndex].request.http[requestCount-1] = INVALID_HTTP_HANDLE;

                ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);
                requestStatus =	sendReqToCamera(&privacyMaskRequest[cameraIndex].request, &ipCamCfg, MAIN_STREAM);
                if (requestStatus != CMD_SUCCESS)
                {
                    sessionClose = TRUE;
                }
            }

            if ((sessionClose == FALSE ) && (requestCount == privacyMaskRequest[cameraIndex].request.numOfRequest))
            {
                ReadSingleCameraConfig(cameraIndex, &cameraCnfg);
                if (privacyMaskRequest[cameraIndex].request.clientCb[ON] != NULL)
                {
                    cameraCnfg.privacyMaskStaus = TRUE;
                }

                pthread_rwlock_wrlock(&camCnfgNotifyControl[cameraIndex].configNotifyLock);
                camCnfgNotifyControl[cameraIndex].cameraConfigNotifyF = FALSE;
                pthread_rwlock_unlock(&camCnfgNotifyControl[cameraIndex].configNotifyLock);
                WriteSingleCameraConfig(cameraIndex,&cameraCnfg);
                pthread_rwlock_wrlock(&camCnfgNotifyControl[cameraIndex].configNotifyLock);
                camCnfgNotifyControl[cameraIndex].cameraConfigNotifyF = TRUE;
                pthread_rwlock_unlock(&camCnfgNotifyControl[cameraIndex].configNotifyLock);
                sessionClose = TRUE;
            }

            if (sessionClose == TRUE)
            {
                if (privacyMaskRequest[cameraIndex].request.clientCb[ON] != NULL)
                {
                    ((NW_CMD_REPLY_CB)privacyMaskRequest[cameraIndex].request.clientCb[ON])
                            (privacyMaskRequest[cameraIndex].request.requestStatus, privacyMaskRequest[cameraIndex].request.clientSocket[ON], TRUE);
                    privacyMaskRequest[cameraIndex].request.clientCb[ON] = NULL;
                }

                restoreDefaultPrivacyMaskConfig(cameraIndex);
                MUTEX_LOCK(privacyMaskRequest[cameraIndex].request.camReqFlagLock);
                privacyMaskRequest[cameraIndex].request.camReqBusyF = FREE;
                MUTEX_UNLOCK(privacyMaskRequest[cameraIndex].request.camReqFlagLock);
            }
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief tcpCamAlarmOutCb
 * @param tcpHandle
 * @param dataInfo
 */
static void tcpCamAlarmOutCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo)
{
    NET_CMD_STATUS_e 		requestStatus = CMD_SUCCESS;
    UINT8 					cameraIndex = GET_STREAM_INDEX(dataInfo->camIndex);
    UINT8 					alarmIndex;
    UINT8 					requestCount;
    BOOL 					breakLoop = FALSE;
    BOOL 					alarmAction;
    BOOL					otherAlarmAction;

    if (cameraIndex >= getMaxCameraForCurrentVariant())
    {
        return;
    }

    for (alarmIndex = 0; alarmIndex < MAX_CAMERA_ALARM; alarmIndex++)
    {
        for(requestCount = 0; requestCount < alarmActionRequest[cameraIndex][alarmIndex].request.numOfRequest; requestCount++)
        {
            if(alarmActionRequest[cameraIndex][alarmIndex].request.http[requestCount] == tcpHandle)
            {
                breakLoop = TRUE;
                break;
            }
        }

        if(breakLoop == TRUE)
        {
            break;
        }
    }

    if ((alarmIndex >= MAX_CAMERA_ALARM) || (requestCount >= alarmActionRequest[cameraIndex][alarmIndex].request.numOfRequest))
    {
        return;
    }

    alarmAction = (alarmActionRequest[cameraIndex][alarmIndex].request.url[requestCount].requestType == CAM_ALARM_ACTIVE) ? ACTIVE : INACTIVE;

    switch(dataInfo->tcpResp)
    {
        case TCP_SUCCESS:
        case TCP_CLOSE_ON_SUCCESS:
        {
            DPRINT(CAMERA_INTERFACE, "tcp request with success: [camera=%d], [alarm=%d], [alarmAction=%s], [requestCount=%d]",
                   cameraIndex, alarmIndex, alarmReqStr[alarmAction], requestCount);
            alarmActionRequest[cameraIndex][alarmIndex].request.http[requestCount] = INVALID_HTTP_HANDLE;

            // It is done, now reset request status
            if (alarmAction == ACTIVE)
            {
                MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                alarmActionRequest[cameraIndex][alarmIndex].requestCounter++;
                alarmActionRequest[cameraIndex][alarmIndex].reqStatus[alarmAction] = FREE;
                MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
            }
            else
            {
                MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
                alarmActionRequest[cameraIndex][alarmIndex].reqStatus[alarmAction] = FREE;
                MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
            }
        }
        break;

        default:
        case TCP_CLOSE_ON_ERROR:
        case TCP_ERROR:
        {
            EPRINT(CAMERA_INTERFACE, "tcp request with error: [camera=%d], [alarm=%d], [alarmAction=%s], [requestCount=%d], [tcpResp=%d]",
                   cameraIndex, alarmIndex, alarmReqStr[alarmAction], requestCount, dataInfo->tcpResp);
            alarmActionRequest[cameraIndex][alarmIndex].request.http[requestCount] = INVALID_HTTP_HANDLE;
            requestStatus = CMD_CAM_REQUEST_FAILED;

            // REquest failed, reset request status
            MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
            alarmActionRequest[cameraIndex][alarmIndex].reqStatus[alarmAction] = FREE;
            MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
        }
        break;
    }

    // If callback is not null, give callback with request status
    if (alarmActionRequest[cameraIndex][alarmIndex].request.clientCb[alarmAction] != NULL)
    {
        ((ALARM_ACTION_REQUEST_CB)alarmActionRequest[cameraIndex][alarmIndex].request.clientCb[alarmAction])(cameraIndex, alarmIndex, requestStatus);
    }

    // If request is free and other request is pending, serve it
    MUTEX_LOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);

    if (alarmActionRequest[cameraIndex][alarmIndex].reqStatus[alarmAction] == FREE)
    {
        otherAlarmAction = (alarmAction == ACTIVE) ? INACTIVE : ACTIVE;
        if(alarmActionRequest[cameraIndex][alarmIndex].reqStatus[otherAlarmAction] == PENDING)
        {
            MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
            CameraAlarmAction(cameraIndex, alarmIndex, otherAlarmAction,
                              alarmActionRequest[cameraIndex][alarmIndex].request.clientCb[otherAlarmAction]);
        }
        else
        {
            MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
        }
    }
    else
    {
        MUTEX_UNLOCK(alarmActionRequest[cameraIndex][alarmIndex].reqStatusLock);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief tcpGetTestCamCb
 * @param tcpHandle
 * @param dataInfo
 */
static void tcpGetTestCamCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo)
{
    UINT8 cameraIndex = dataInfo->camIndex;
    UINT8 requestCount;

    /* Check camera index. It should be in valid range */
    if (cameraIndex >= getMaxCameraForCurrentVariant())
    {
        DPRINT(CAMERA_INTERFACE, "invld camera index found: [camera=%d]", cameraIndex);
        return;
    }

    for(requestCount = 0; requestCount < getImageRequest[cameraIndex].numOfRequest; requestCount++)
    {
        if (getImageRequest[cameraIndex].http[requestCount] == (HTTP_HANDLE)tcpHandle)
        {
            break;
        }
    }

    switch(dataInfo->tcpResp)
    {
        case TCP_CLOSE_ON_SUCCESS:
        {
            DPRINT(CAMERA_INTERFACE, "tcp success: [camera=%d], [requestCount=%d], [tcpResp=%s]", cameraIndex, requestCount, httpRespStr[dataInfo->tcpResp]);
            getImageRequest[cameraIndex].requestStatus = CMD_SUCCESS;
            getImageRequest[cameraIndex].http[requestCount] = INVALID_HTTP_HANDLE;
        }
        break;

        case TCP_CLOSE_ON_ERROR:
        {
            EPRINT(CAMERA_INTERFACE, "tcp error: [camera=%d], [requestCount=%d], [tcpResp=%s]", cameraIndex, requestCount, httpRespStr[dataInfo->tcpResp]);
            getImageRequest[cameraIndex].requestStatus = CMD_PROCESS_ERROR;
            getImageRequest[cameraIndex].http[requestCount] = INVALID_HTTP_HANDLE;
        }
        break;

        default:
        {
            getImageRequest[cameraIndex].requestStatus = CMD_PROCESS_ERROR;
            DPRINT(CAMERA_INTERFACE, "tcp unhandled error: [camera=%d], [requestCount=%d], [tcpResp=%d]", cameraIndex, requestCount, dataInfo->tcpResp);
        }
        break;
    }

    if (getImageRequest[cameraIndex].clientCb[ON] != NULL)
    {
        ((IMAGE_REQUEST_CB)getImageRequest[cameraIndex].clientCb[ON])(cameraIndex, getImageRequest[cameraIndex].requestStatus,
                 dataInfo->storagePtr, dataInfo->storageLen, getImageRequest[cameraIndex].clientCbType);
        getImageRequest[cameraIndex].clientCb[ON] = NULL;
    }

    MUTEX_LOCK(getImageRequest[cameraIndex].camReqFlagLock);
    getImageRequest[cameraIndex].camReqBusyF = FREE;
    MUTEX_UNLOCK(getImageRequest[cameraIndex].camReqFlagLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief tcpStreamControlCb
 * @param tcpHandle
 * @param dataInfo
 */
static void tcpStreamControlCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo)
{
    UINT8 						requestCount;
    CI_STREAM_EXT_TRG_PARAM_t	triggerParam;
    STREAM_INFO_t				*pStreamInfo;

    switch(dataInfo ->tcpResp)
    {
        default:
        {
            /* Nothing to do */
        }
        break;

        case TCP_CLOSE_ON_SUCCESS:
        case TCP_CLOSE_ON_ERROR:
        case TCP_ERROR:
        {
            triggerParam.camIndex = dataInfo->camIndex;
            pStreamInfo = &streamInfo[GET_STREAM_INDEX(triggerParam.camIndex)][GET_STREAM_TYPE(triggerParam.camIndex)];

            for (requestCount = 0; requestCount < pStreamInfo->getStreamRequest.numOfRequest; requestCount++)
            {
                if (pStreamInfo->getStreamRequest.http[requestCount] == tcpHandle)
                {
                    break;
                }
            }

            if (requestCount < pStreamInfo->getStreamRequest.numOfRequest)
            {
                pStreamInfo->getStreamRequest.http[requestCount] = INVALID_HTTP_HANDLE;
                triggerParam.status = CMD_SUCCESS;
                triggerParam.type = CI_STREAM_EXT_TRG_CONTROL_CB;
                ciStreamExtTriggerHandler(&triggerParam);
            }
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief tcpSetPtzControlCb
 * @param tcpHandle
 * @param dataInfo
 */
static void tcpSetPtzControlCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo)
{
    tcpCommonControlCb(CAM_REQ_STRUCT_PTZ, tcpHandle, dataInfo);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief tcpSetFocusControlCb
 * @param tcpHandle
 * @param dataInfo
 */
static void tcpSetFocusControlCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo)
{
    tcpCommonControlCb(CAM_REQ_STRUCT_FOCUS, tcpHandle, dataInfo);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief tcpStorePresetControlCb
 * @param tcpHandle
 * @param dataInfo
 */
static void tcpStorePresetControlCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo)
{
    tcpCommonControlCb(CAM_REQ_STRUCT_STORE_PRESET, tcpHandle, dataInfo);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief tcpGotoPresetControlCb
 * @param tcpHandle
 * @param dataInfo
 */
static void tcpGotoPresetControlCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo)
{
    tcpCommonControlCb(CAM_REQ_STRUCT_GOTO_PRESET, tcpHandle, dataInfo);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief tcpCommonControlCb
 * @param reqType
 * @param tcpHandle
 * @param dataInfo
 */
static void tcpCommonControlCb(CAM_REQ_STRUCT_TYPE_e reqType, TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo)
{
    BOOL				freeReq = TRUE;
    NET_CMD_STATUS_e 	requestStatus = CMD_SUCCESS;
    UINT8 				cameraIndex = dataInfo->camIndex;
    UINT8               requestCount;
    IP_CAMERA_CONFIG_t 	ipCamCfg;

    if (cameraIndex >= getMaxCameraForCurrentVariant())
    {
        return;
    }

    CAMERA_REQUEST_t *reqPtr = getCameraReqStruct(reqType, cameraIndex);

    if (reqPtr == NULL)
    {
        EPRINT(CAMERA_INTERFACE, "invld request type found: [camera=%d]", cameraIndex);
        return;
    }

    // Find the request number
    for (requestCount = 0; requestCount < reqPtr->numOfRequest; requestCount++)
    {
        if (reqPtr->http[requestCount] == tcpHandle)
        {
            break;
        }
    }

    if (requestCount >= reqPtr->numOfRequest)
    {
        return;
    }

    switch (dataInfo->tcpResp)
    {
        case TCP_SUCCESS:
        case TCP_CLOSE_ON_SUCCESS:
        {
            reqPtr->requestStatus = CMD_SUCCESS;
            reqPtr->http[requestCount] = INVALID_HTTP_HANDLE;

            if ((reqPtr->requestCount + 1) < reqPtr->numOfRequest)
            {
                reqPtr->requestCount++;
                reqPtr->requestStatus = CMD_PROCESS_ERROR;

                if (CameraType(cameraIndex) == IP_CAMERA)
                {
                    ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);
                }

                if((requestStatus =	sendReqToCamera(reqPtr, &ipCamCfg, MAIN_STREAM)) == CMD_SUCCESS)
                {
                    freeReq = FALSE;
                }
            }
        }
        break;

        case TCP_CLOSE_ON_ERROR:
        case TCP_ERROR:
        default:
        {
            requestStatus = CMD_CAM_REQUEST_FAILED;
            reqPtr->http[requestCount] = INVALID_HTTP_HANDLE;
        }
        break;
    }

    if (freeReq == FALSE)
    {
        return;
    }

    if (reqPtr->clientCb[ON] != NULL)
    {
        ((NW_CMD_REPLY_CB)reqPtr->clientCb[ON])(requestStatus, reqPtr->clientSocket[ON], TRUE);
        reqPtr->clientCb[ON] = NULL;
    }

    DPRINT(CAMERA_INTERFACE, "tcp common control response: [camera=%d], [reqType=%d]", cameraIndex, reqType);
    MUTEX_LOCK(reqPtr->camReqFlagLock);
    reqPtr->camReqBusyF = FREE;
    MUTEX_UNLOCK(reqPtr->camReqFlagLock);

    MUTEX_LOCK(ptzList[cameraIndex].ptzSignalLock);
    DPRINT(CAMERA_INTERFACE,"tcp commonCtrl signal ptzCtrlThread");
    pthread_cond_signal(&ptzList[cameraIndex].ptzSignal);
    MUTEX_UNLOCK(ptzList[cameraIndex].ptzSignalLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Gets internet connection status
 * @return  internet status
 */
BOOL getInternetConnStatus(void)
{
    return internetConnectivityStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get image settings supported capability of camera
 * @param   camIndex
 * @param   pCapability
 */
NET_CMD_STATUS_e GetImagingCapability(UINT8 camIndex, CAPABILITY_TYPE *pCapability)
{
    CAMERA_CONFIG_t cameraConfig;

    ReadSingleCameraConfig(camIndex, &cameraConfig);
    if (cameraConfig.camera == DISABLE)
    {
        EPRINT(CAMERA_INTERFACE, "camera channel disabled: [camera=%d]", camIndex);
        return CMD_CHANNEL_DISABLED;
    }

    /* Check if camera is connected or not */
    if (GetCamEventStatus(camIndex, CONNECTION_FAILURE) == INACTIVE)
    {
        EPRINT(CAMERA_INTERFACE, "camera disconnected: [camera=%d]", camIndex);
        return CMD_CAM_DISCONNECTED;
    }

    /* Check whether camera supports image setting or not */
    if (imageSettingRequest[camIndex].imageCapsInfo.imagingCapability == 0)
    {
        EPRINT(CAMERA_INTERFACE, "feature not supported by [camera=%d]", camIndex);
        return CMD_FEATURE_NOT_SUPPORTED;
    }

    /* Set image setting capability */
    *pCapability = imageSettingRequest[camIndex].imageCapsInfo.imagingCapability;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Default the image setting config and reset image capability
 * @param   camIndex
 */
void DefaultCameraImageCapability(UINT8 camIndex)
{
    /* Reset image setting param of camera */
    memset(&imageSettingRequest[camIndex].imageCapsInfo, 0, sizeof(IMAGE_CAPABILITY_INFO_t));

    /* Default camera's image setting configuration */
    DfltSingleImageSettingConfig(camIndex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set image settings values in camera database and write into configuration
 * @param   camIndex
 * @param   pImageCapsInfo
 */
void saveCameraImageCapability(UINT8 camIndex, IMAGE_CAPABILITY_INFO_t *pImageCapsInfo)
{
    IMG_SETTING_CONFIG_t imgSettingConfig = DfltImageSettingCfg;

    DPRINT(CAMERA_INTERFACE, "image setting config added: [camera=%d], [capability=0x%X]", camIndex, pImageCapsInfo->imagingCapability);
    imgSettingConfig.brightness = pImageCapsInfo->brightness.value;
    imgSettingConfig.contrast = pImageCapsInfo->contrast.value;
    imgSettingConfig.saturation = pImageCapsInfo->saturation.value;
    imgSettingConfig.hue = pImageCapsInfo->hue.value;
    imgSettingConfig.sharpness = pImageCapsInfo->sharpness.value;
    imgSettingConfig.whiteBalanceMode = pImageCapsInfo->whiteBalance.mode;
    imgSettingConfig.wdrMode = pImageCapsInfo->wdr.mode;
    imgSettingConfig.wdrStrength = pImageCapsInfo->wdrStrength.value;
    imgSettingConfig.backlightMode = pImageCapsInfo->backlightControl.mode;
    imgSettingConfig.exposureRatioMode = pImageCapsInfo->exposureRatioMode.mode;
    imgSettingConfig.exposureRatio = pImageCapsInfo->exposureRatio.value;
    imgSettingConfig.exposureMode = pImageCapsInfo->exposureMode.mode;
    imgSettingConfig.flicker = pImageCapsInfo->flicker.mode;
    imgSettingConfig.flickerStrength = pImageCapsInfo->flickerStrength.value;
    imgSettingConfig.hlc = pImageCapsInfo->hlc.mode;
    imgSettingConfig.exposureTime = pImageCapsInfo->exposureTime.value;
    imgSettingConfig.exposureGain = pImageCapsInfo->exposureGain.value;
    imgSettingConfig.exposureIris = pImageCapsInfo->exposureIris.value;
    imgSettingConfig.normalLightGain = pImageCapsInfo->normalLightGain.mode;
    imgSettingConfig.normalLightLuminance = pImageCapsInfo->normalLightLuminance.value;
    imgSettingConfig.irLedMode = pImageCapsInfo->irLed.mode;
    imgSettingConfig.irLedSensitivity = pImageCapsInfo->irLedSensitivity.value;

    /* As we have received this config and capability from camera, we don't required to set it again in camera */
    MUTEX_LOCK(imageSettingRequest[camIndex].setCnfgToCameraLock);
    imageSettingRequest[camIndex].setCnfgToCameraF = FALSE;
    MUTEX_UNLOCK(imageSettingRequest[camIndex].setCnfgToCameraLock);
    WriteSingleImageSettingConfig(camIndex, &imgSettingConfig);
    MUTEX_LOCK(imageSettingRequest[camIndex].setCnfgToCameraLock);
    imageSettingRequest[camIndex].setCnfgToCameraF = TRUE;
    MUTEX_UNLOCK(imageSettingRequest[camIndex].setCnfgToCameraLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Write image setting configuration for current camera and copy that config to other
 *          camera if given in mask
 * @param   sourceCamera
 * @param   pImgSettingCfg
 * @param   copyToCamera
 * @return
 */
NET_CMD_STATUS_e WriteImageSettingAfterAdjust(UINT8 sourceCamera, IMG_SETTING_CONFIG_t *pSrcImgSettingCfg, CAMERA_BIT_MASK_t copyToCamera)
{
    UINT8                       camIndex;
    BOOL                        isNewImageParamApply;
    IMG_SETTING_CFG_FIELD_e     capability;
    IMG_SETTING_CONFIG_t        dstImgSettingCfg;
    IMAGE_CAPABILITY_INFO_t     *pSrcImgCapsInfo;
    IMAGE_CAPABILITY_INFO_t     *pDstImgCapsInfo;
    CAMERA_CONFIG_t             srcCameraConfig;
    IP_CAMERA_CONFIG_t          srcIpCameraConfig;
    CAMERA_CONFIG_t             dstCameraConfig;
    IP_CAMERA_CONFIG_t          dstIpCameraConfig;
    CHAR                        details[MAX_EVENT_DETAIL_SIZE];
    CHAR                        advncDetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];

    /* Read source camera config */
    ReadSingleCameraConfig(sourceCamera, &srcCameraConfig);
    ReadSingleIpCameraConfig(sourceCamera, &srcIpCameraConfig);

    /* if camera is disabled then continue */
    if (srcCameraConfig.camera == DISABLE)
    {
        EPRINT(CAMERA_INTERFACE, "src camera disabled: [camera=%d]", sourceCamera);
        return CMD_CHANNEL_DISABLED;
    }

    /* Check camera connectivity */
    if (GetCamEventStatus(sourceCamera, CONNECTION_FAILURE) == INACTIVE)
    {
        EPRINT(CAMERA_INTERFACE, "source camera offline: [camera=%d]", sourceCamera);
        return CMD_CAM_DISCONNECTED;
    }

    /* Write source camera config */
    WriteSingleImageSettingConfig(sourceCamera, pSrcImgSettingCfg);

    /* Get imaging capability of source camera */
    pSrcImgCapsInfo = &imageSettingRequest[sourceCamera].imageCapsInfo;

    /* Is Copy to camera given? */
    if (TRUE == IS_ALL_CAMERA_MASK_BIT_CLR(copyToCamera))
    {
        /* Nothing to do for copy to camera */
        return CMD_SUCCESS;
    }

    /* Copy config from source camera to given camera mask */
    DPRINT(CAMERA_INTERFACE, "write image setting config for selected camera: [source=%d], [copyToCameraMask1=0x%llX], [copyToCameraMask2=0x%llX]",
           sourceCamera, copyToCamera.bitMask[0], copyToCamera.bitMask[1]);

    for (camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
    {
        /* Is camera added in copy mask? */
        if (GET_CAMERA_MASK_BIT(copyToCamera, camIndex) == 0)
        {
            /* do not copy to camera */
            continue;
        }

        /* Read camera config */
        ReadSingleCameraConfig(camIndex, &dstCameraConfig);

        /* if camera is disabled then continue */
        if (dstCameraConfig.camera == DISABLE)
        {
            EPRINT(CAMERA_INTERFACE, "camera disabled: [camera=%d]", camIndex);
            continue;
        }

        /* Check camera connectivity */
        snprintf(details, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(camIndex));
        if (GetCamEventStatus(camIndex, CONNECTION_FAILURE)== INACTIVE)
        {
            EPRINT(CAMERA_INTERFACE, "camera offline: [camera=%d]", camIndex);
            snprintf(advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "Destination camera is disconnected");
            WriteEvent(LOG_CAMERA_EVENT, LOG_IMAGE_SETTING, details, advncDetail, EVENT_FAIL);
            continue;
        }

        /* Check onvif support of both cameras and camera type */
        ReadSingleIpCameraConfig(camIndex, &dstIpCameraConfig);
        if (srcIpCameraConfig.onvifSupportF != dstIpCameraConfig.onvifSupportF)
        {
            EPRINT(CAMERA_INTERFACE, "camera added via different mode: [camera=%d], [srcOnvifSupport=%d], [dstOnvifSupport=%d]",
                   camIndex, srcIpCameraConfig.onvifSupportF, dstIpCameraConfig.onvifSupportF);
            snprintf(advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "Destination camera is added via different mode");
            WriteEvent(LOG_CAMERA_EVENT, LOG_IMAGE_SETTING, details, advncDetail, EVENT_FAIL);
            continue;
        }

        /* get imaging capability of camera */
        pDstImgCapsInfo = &imageSettingRequest[camIndex].imageCapsInfo;

        /* if camera does not support image settings then write fail event and continue for next camera */
        if (pDstImgCapsInfo->imagingCapability == 0)
        {
            EPRINT(CAMERA_INTERFACE, "camera doesn't support image setting: [camera=%d], [cameraType=%d], [onvifSupport=%d]",
                   camIndex, dstCameraConfig.type, dstIpCameraConfig.onvifSupportF);
            snprintf(advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "Destination camera doesn't support image setting");
            WriteEvent(LOG_CAMERA_EVENT, LOG_IMAGE_SETTING, details, advncDetail, EVENT_FAIL);
            continue;
        }

        /* if source config does not contain specific value then keep the previously set value of destination  */
        isNewImageParamApply = FALSE;
        ReadSingleImageSettingConfig(camIndex, &dstImgSettingCfg);
        for (capability = 0; capability < IMAGE_SETTING_CAPABILITY_MAX; capability++)
        {
            /* case-1: if any parameter does not exits at destination, skip that parameter
             * case-2: if a particular value of destination does not exist in source parameter, keep the previous destination value */
            if ((GET_BIT(pDstImgCapsInfo->imagingCapability, capability) == 0) || (GET_BIT(pSrcImgCapsInfo->imagingCapability, capability) == 0))
            {
                /* Neither source camera nor destination camera support this capability */
                continue;
            }

            /* We have found common supported param */
            isNewImageParamApply = TRUE;
            switch(capability)
            {
                case IMG_SETTING_BRIGHTNESS:
                {
                    dstImgSettingCfg.brightness = pSrcImgSettingCfg->brightness;
                }
                break;

                case IMG_SETTING_CONTRAST:
                {
                    dstImgSettingCfg.contrast = pSrcImgSettingCfg->contrast;
                }
                break;

                case IMG_SETTING_SATURATION:
                {
                    dstImgSettingCfg.saturation = pSrcImgSettingCfg->saturation;
                }
                break;

                case IMG_SETTING_HUE:
                {
                    dstImgSettingCfg.hue = pSrcImgSettingCfg->hue;
                }
                break;

                case IMG_SETTING_SHARPNESS:
                {
                    dstImgSettingCfg.sharpness = pSrcImgSettingCfg->sharpness;
                }
                break;

                case IMG_SETTING_WHITE_BALANCE:
                {
                    /* if src mode is supported by destination only then copy */
                    if (GET_BIT(pDstImgCapsInfo->whiteBalance.modeSupported, pSrcImgSettingCfg->whiteBalanceMode))
                    {
                         dstImgSettingCfg.whiteBalanceMode = pSrcImgSettingCfg->whiteBalanceMode;
                    }
                }
                break;

                case IMG_SETTING_WDR_MODE:
                {
                    /* if src mode is supported by destination only then copy */
                    if (GET_BIT(pDstImgCapsInfo->wdr.modeSupported, pSrcImgSettingCfg->wdrMode))
                    {
                         dstImgSettingCfg.wdrMode = pSrcImgSettingCfg->wdrMode;
                    }
                }
                break;

                case IMG_SETTING_WDR_STRENGTH:
                {
                    dstImgSettingCfg.wdrStrength = pSrcImgSettingCfg->wdrStrength;
                }
                break;

                case IMG_SETTING_BACKLIGHT:
                {
                    /* if src mode is supported by destination only then copy */
                    if (GET_BIT(pDstImgCapsInfo->backlightControl.modeSupported, pSrcImgSettingCfg->backlightMode))
                    {
                         dstImgSettingCfg.backlightMode = pSrcImgSettingCfg->backlightMode;
                    }
                }
                break;

                case IMG_SETTING_EXPOSURE_RATIO_MODE:
                {
                    /* if src mode is supported by destination only then copy */
                    if (GET_BIT(pDstImgCapsInfo->exposureRatioMode.modeSupported, pSrcImgSettingCfg->exposureRatioMode))
                    {
                         dstImgSettingCfg.exposureRatioMode = pSrcImgSettingCfg->exposureRatioMode;
                    }
                }
                break;

                case IMG_SETTING_EXPOSURE_RATIO:
                {
                    dstImgSettingCfg.exposureRatio = pSrcImgSettingCfg->exposureRatio;
                }
                break;

                case IMG_SETTING_EXPOSURE_MODE:
                {
                    /* if src mode is supported by destination only then copy */
                    if (GET_BIT(pDstImgCapsInfo->exposureMode.modeSupported, pSrcImgSettingCfg->exposureMode))
                    {
                         dstImgSettingCfg.exposureMode = pSrcImgSettingCfg->exposureMode;
                    }
                }
                break;

                case IMG_SETTING_FLICKER:
                {
                    /* if src mode is supported by destination only then copy */
                    if (GET_BIT(pDstImgCapsInfo->flicker.modeSupported, pSrcImgSettingCfg->flicker))
                    {
                         dstImgSettingCfg.flicker = pSrcImgSettingCfg->flicker;
                    }
                }
                break;

                case IMG_SETTING_FLICKER_STRENGTH:
                {
                    dstImgSettingCfg.flickerStrength = pSrcImgSettingCfg->flickerStrength;
                }
                break;

                case IMG_SETTING_HLC:
                {
                    /* if src mode is supported by destination only then copy */
                    if (GET_BIT(pDstImgCapsInfo->hlc.modeSupported, pSrcImgSettingCfg->hlc))
                    {
                         dstImgSettingCfg.hlc = pSrcImgSettingCfg->hlc;
                    }
                }
                break;

                case IMG_SETTING_EXPOSURE_TIME:
                {
                    dstImgSettingCfg.exposureTime = pSrcImgSettingCfg->exposureTime;
                }
                break;

                case IMG_SETTING_EXPOSURE_GAIN:
                {
                    dstImgSettingCfg.exposureGain = pSrcImgSettingCfg->exposureGain;
                }
                break;

                case IMG_SETTING_EXPOSURE_IRIS:
                {
                    dstImgSettingCfg.exposureIris = pSrcImgSettingCfg->exposureIris;
                }
                break;

                case IMG_SETTING_NORMAL_LIGHT_GAIN:
                {
                    /* if src mode is supported by destination only then copy */
                    if (GET_BIT(pDstImgCapsInfo->normalLightGain.modeSupported, pSrcImgSettingCfg->normalLightGain))
                    {
                         dstImgSettingCfg.normalLightGain = pSrcImgSettingCfg->normalLightGain;
                    }
                }
                break;

                case IMG_SETTING_NORMAL_LIGHT_LUMINANCE:
                {
                    dstImgSettingCfg.normalLightLuminance = pSrcImgSettingCfg->normalLightLuminance;
                }
                break;

                case IMG_SETTING_LED_MODE:
                {
                    /* if src mode is supported by destination only then copy */
                    if (GET_BIT(pDstImgCapsInfo->irLed.modeSupported, pSrcImgSettingCfg->irLedMode))
                    {
                         dstImgSettingCfg.irLedMode = pSrcImgSettingCfg->irLedMode;
                    }
                }
                break;

                case IMG_SETTING_LED_SENSITIVITY:
                {
                    dstImgSettingCfg.irLedSensitivity = pSrcImgSettingCfg->irLedSensitivity;
                }
                break;

                default:
                {
                    /* Nothing to do */
                }
                break;
            }
        }

        /* Write configuration if param updated */
        if (TRUE == isNewImageParamApply)
        {
            /* Write configuration in destination camera */
            WriteSingleImageSettingConfig(camIndex, &dstImgSettingCfg);
        }
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get/Set image setting param onvif callback
 * @param   responseData
 * @return
 */
static BOOL onvifImagingSettingsRespCb(ONVIF_RESPONSE_PARA_t *responseData)
{
    UINT8 cameraIndex = responseData->cameraIndex;

    /* Validate camera index */
    if (cameraIndex >= getMaxCameraForCurrentVariant())
    {
        EPRINT(CAMERA_INTERFACE, "invld camera found: [camera=%d], [requestType=%s], [response=%d]",
               cameraIndex, GetOnvifReqName(responseData->requestType), responseData->response);
        return FAIL;
    }

    /* Free image setting process lock */
    MUTEX_LOCK(imageSettingRequest[cameraIndex].request.camReqFlagLock);
    imageSettingRequest[cameraIndex].request.camReqBusyF = FREE;
    MUTEX_UNLOCK(imageSettingRequest[cameraIndex].request.camReqFlagLock);

    /* Check response and log debug */
    if (responseData->response != ONVIF_CMD_SUCCESS)
    {
        if ((responseData->requestType == ONVIF_GET_IMAGING_CAPABILITY) || (responseData->requestType == ONVIF_GET_IMAGING_SETTING))
        {
            /* Reset image setting capability due to error in response */
            memset(&imageSettingRequest[cameraIndex].imageCapsInfo, 0, sizeof(imageSettingRequest[cameraIndex].imageCapsInfo));
        }
        EPRINT(CAMERA_INTERFACE, "image setting fail: [camera=%d], [requestType=%s], [response=%d]",
               cameraIndex, GetOnvifReqName(responseData->requestType), responseData->response);
        return SUCCESS;
    }

    /* If it is image capability request then we have to get config also */
    switch (responseData->requestType)
    {
        case ONVIF_GET_IMAGING_CAPABILITY:
        {
            /* Get image settings parameters */
            DPRINT(CAMERA_INTERFACE, "get image setting capability done: [camera=%d]", cameraIndex);
            if (FAIL == getImageSettingConfig(cameraIndex))
            {
                EPRINT(CAMERA_INTERFACE, "fail to get image setting config: [camera=%d]", cameraIndex);
            }
        }
        break;

        case ONVIF_GET_IMAGING_SETTING:
        {
            DPRINT(CAMERA_INTERFACE, "get image setting param done: [camera=%d]", cameraIndex);
            saveCameraImageCapability(cameraIndex, &imageSettingRequest[cameraIndex].imageCapsInfo);
        }
        break;

        default:
        {
            DPRINT(CAMERA_INTERFACE, "image setting param done: [camera=%d], [requestType=%s]", cameraIndex, GetOnvifReqName(responseData->requestType));
        }
        break;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get periodic update of image setting from camera
 * @param   data
 */
static void imagingSettingTimerSyncCb(UINT32 data)
{
    UINT8 cameraIndex;

    for (cameraIndex = 0; cameraIndex < getMaxCameraForCurrentVariant(); cameraIndex++)
    {
        /* Get imaging setting from camera */
        getImageSettingConfig(cameraIndex);
    }

    /* It is reloaded with actual timer because we have started image setting timer with some delay than profile
     * update timer at power on. It is added to restrict the execution of both the processes at same time */
    ReloadTimer(imagingSettingSyncTimerHandle, CONVERT_SEC_TO_TIMER_COUNT(IMAGE_SETTING_UPDATE_TIME*60));
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Image setting config change notify
 * @param   newCopy
 * @param   oldCopy
 */
void ImageSettingConfigNotify(IMG_SETTING_CONFIG_t newCopy, IMG_SETTING_CONFIG_t *oldCopy, UINT8 camIndex)
{
    /* Do not set config into camera when we get config from camera and written into database */
    MUTEX_LOCK(imageSettingRequest[camIndex].setCnfgToCameraLock);
    if (FALSE == imageSettingRequest[camIndex].setCnfgToCameraF)
    {
        /* We don't require to set param in camera */
        MUTEX_UNLOCK(imageSettingRequest[camIndex].setCnfgToCameraLock);
        return;
    }
    MUTEX_UNLOCK(imageSettingRequest[camIndex].setCnfgToCameraLock);

    /* Update local copy of image capability */
    imageSettingRequest[camIndex].imageCapsInfo.brightness.value = newCopy.brightness;
    imageSettingRequest[camIndex].imageCapsInfo.contrast.value = newCopy.contrast;
    imageSettingRequest[camIndex].imageCapsInfo.saturation.value = newCopy.saturation;
    imageSettingRequest[camIndex].imageCapsInfo.hue.value = newCopy.hue;
    imageSettingRequest[camIndex].imageCapsInfo.sharpness.value = newCopy.sharpness;
    imageSettingRequest[camIndex].imageCapsInfo.whiteBalance.mode = newCopy.whiteBalanceMode;
    imageSettingRequest[camIndex].imageCapsInfo.wdr.mode = newCopy.wdrMode;
    imageSettingRequest[camIndex].imageCapsInfo.wdrStrength.value = newCopy.wdrStrength;
    imageSettingRequest[camIndex].imageCapsInfo.backlightControl.mode = newCopy.backlightMode;
    imageSettingRequest[camIndex].imageCapsInfo.exposureRatioMode.mode = newCopy.exposureRatioMode;
    imageSettingRequest[camIndex].imageCapsInfo.exposureRatio.value = newCopy.exposureRatio;
    imageSettingRequest[camIndex].imageCapsInfo.exposureMode.mode = newCopy.exposureMode;
    imageSettingRequest[camIndex].imageCapsInfo.flicker.mode = newCopy.flicker;
    imageSettingRequest[camIndex].imageCapsInfo.flickerStrength.value = newCopy.flickerStrength;
    imageSettingRequest[camIndex].imageCapsInfo.hlc.mode = newCopy.hlc;
    imageSettingRequest[camIndex].imageCapsInfo.exposureTime.value = newCopy.exposureTime;
    imageSettingRequest[camIndex].imageCapsInfo.exposureGain.value = newCopy.exposureGain;
    imageSettingRequest[camIndex].imageCapsInfo.exposureIris.value = newCopy.exposureIris;
    imageSettingRequest[camIndex].imageCapsInfo.normalLightGain.mode = newCopy.normalLightGain;
    imageSettingRequest[camIndex].imageCapsInfo.normalLightLuminance.value = newCopy.normalLightLuminance;
    imageSettingRequest[camIndex].imageCapsInfo.irLed.mode = newCopy.irLedMode;
    imageSettingRequest[camIndex].imageCapsInfo.irLedSensitivity.value = newCopy.irLedSensitivity;

    /* Set imaging setting in camera */
    setImageSettingConfig(camIndex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get image setting capability from camera
 * @param   cameraIndex
 * @return
 */
static BOOL getImageSettingCapability(UINT8 cameraIndex)
{
    NET_CMD_STATUS_e    requestStatus;
    CAMERA_CONFIG_t     camConfig;
    IP_CAMERA_CONFIG_t 	ipCamCfg;
    CAMERA_REQUEST_t    *pCameraRequest;

    /* Validate input param */
    if (cameraIndex >= getMaxCameraForCurrentVariant())
    {
        EPRINT(CAMERA_INTERFACE, "invld camera index found: [camera=%d]", cameraIndex);
        return FAIL;
    }

    ReadSingleCameraConfig(cameraIndex, &camConfig);
    if (camConfig.camera == DISABLE)
    {
        return FAIL;
    }

    pCameraRequest = &imageSettingRequest[cameraIndex].request;
    DPRINT(CAMERA_INTERFACE, "get image settings capability request: [camera=%d]", cameraIndex);
    MUTEX_LOCK(pCameraRequest->camReqFlagLock);
    if(pCameraRequest->camReqBusyF == BUSY)
    {
        MUTEX_UNLOCK(pCameraRequest->camReqFlagLock);
        EPRINT(CAMERA_INTERFACE, "session already running: [camera=%d]", cameraIndex);
        return FAIL;
    }
    pCameraRequest->camReqBusyF = BUSY;
    MUTEX_UNLOCK(pCameraRequest->camReqFlagLock);

    /* Reset current capability and get fresh from camera */
    memset(&imageSettingRequest[cameraIndex].imageCapsInfo, 0, sizeof(imageSettingRequest[cameraIndex].imageCapsInfo));
    pCameraRequest->requestStatus = CMD_PROCESS_ERROR;

    ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);
    if (CameraType(cameraIndex) == AUTO_ADDED_CAMERA)
    {
        /* Generate get camera image setting capability request */
        pCameraRequest->requestCount = 0;
        GetCiRequestUrl(cameraIndex, REQ_URL_GET_IMAGE_CAPABILITY, 0, CAM_REQ_IMG_APPEARANCE, tcpGetImageSettingCapabilityCb, pCameraRequest, NULL);
        DPRINT(CAMERA_INTERFACE, "get image setting capability request: [camera=%d], [data=%s]", cameraIndex, pCameraRequest->url[0].relativeUrl);
        requestStatus =	sendReqToCamera(&imageSettingRequest[cameraIndex].request, &ipCamCfg, MAIN_STREAM);
    }
    else if (ipCamCfg.onvifSupportF == TRUE)
    {
        /* Send GetImagingSettings command to camera */
        requestStatus = sendOnvifCommonCmd(cameraIndex, ONVIF_GET_IMAGING_CAPABILITY, onvifImagingSettingsRespCb,
                                           &ipCamCfg, &imageSettingRequest[cameraIndex].imageCapsInfo);
    }
    else
    {
        requestStatus = GetCameraBrandModelUrl(cameraIndex, REQ_URL_GET_IMAGE_CAPABILITY, pCameraRequest, NULL, NULL, NULL, NULL);
        if (requestStatus != CMD_SUCCESS)
        {
            EPRINT(CAMERA_INTERFACE, "fail to get image capability: [camera=%d], [numOfreq=%d]", cameraIndex, pCameraRequest->numOfRequest);
        }
        else
        {
            DPRINT(CAMERA_INTERFACE, "get image capability request successfully: [camera=%d], [numOfreq=%d]", cameraIndex, pCameraRequest->numOfRequest);
            pCameraRequest->requestCount = 0;
            pCameraRequest->httpCallback[CAM_REQ_IMG_APPEARANCE] = httpGetImageSettingCapabilityCb;
            pCameraRequest->httpCallback[CAM_REQ_IMG_ADV_APPEARANCE] = httpGetImageSettingCapabilityCb;
            pCameraRequest->httpCallback[CAM_REQ_IMG_DAY_NIGHT] = httpGetImageSettingCapabilityCb;
            requestStatus = sendReqToCamera(&imageSettingRequest[cameraIndex].request, &ipCamCfg, MAIN_STREAM);
        }
    }

    /* Error in get image setting process */
    if (requestStatus != CMD_SUCCESS)
    {
        MUTEX_LOCK(pCameraRequest->camReqFlagLock);
        pCameraRequest->camReqBusyF = FREE;
        MUTEX_UNLOCK(pCameraRequest->camReqFlagLock);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get image setting config from camera
 * @param   cameraIndex
 * @return
 */
static BOOL getImageSettingConfig(UINT8 cameraIndex)
{
    NET_CMD_STATUS_e    requestStatus;
    CAMERA_CONFIG_t     cameraCfg;
    IP_CAMERA_CONFIG_t 	ipCamCfg;
    CAMERA_REQUEST_t    *pCameraRequest;

    /* Validate input param */
    if (cameraIndex >= getMaxCameraForCurrentVariant())
    {
        EPRINT(CAMERA_INTERFACE, "invld camera index found: [camera=%d]", cameraIndex);
        return FAIL;
    }

    ReadSingleCameraConfig(cameraIndex, &cameraCfg);
    if (cameraCfg.camera == DISABLE)
    {
        return FAIL;
    }

    /* Check if camera is connected or not */
    if (GetCamEventStatus(cameraIndex, CONNECTION_FAILURE) == INACTIVE)
    {
        return FAIL;
    }

    pCameraRequest = &imageSettingRequest[cameraIndex].request;
    DPRINT(CAMERA_INTERFACE, "get image settings config request: [camera=%d]", cameraIndex);
    MUTEX_LOCK(pCameraRequest->camReqFlagLock);
    if(pCameraRequest->camReqBusyF == BUSY)
    {
        MUTEX_UNLOCK(pCameraRequest->camReqFlagLock);
        EPRINT(CAMERA_INTERFACE, "session already running: [camera=%d]", cameraIndex);
        return FAIL;
    }
    pCameraRequest->camReqBusyF = BUSY;
    MUTEX_UNLOCK(pCameraRequest->camReqFlagLock);

    ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);
    if (CameraType(cameraIndex) == AUTO_ADDED_CAMERA)
    {
        /* Generate get camera image setting appearance param request */
        pCameraRequest->requestCount = 0;
        GetCiRequestUrl(cameraIndex, REQ_URL_GET_IMAGE_SETTING, 0, CAM_REQ_IMG_APPEARANCE, tcpGetImageSettingParamCb, pCameraRequest, NULL);
        DPRINT(CAMERA_INTERFACE, "get image setting appearance request: [camera=%d], [data=%s]", cameraIndex, pCameraRequest->url[0].relativeUrl);
        ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);
        requestStatus = sendReqToCamera(pCameraRequest, &ipCamCfg, MAIN_STREAM);
    }
    else if (ipCamCfg.onvifSupportF == TRUE)
    {
        /* send GetImagingSettings command to camera */
        requestStatus = sendOnvifCommonCmd(cameraIndex, ONVIF_GET_IMAGING_SETTING, onvifImagingSettingsRespCb,
                                           &ipCamCfg, &imageSettingRequest[cameraIndex].imageCapsInfo);
    }
    else
    {
        requestStatus = GetCameraBrandModelUrl(cameraIndex, REQ_URL_GET_IMAGE_SETTING, pCameraRequest, NULL, NULL, NULL, NULL);
        if (requestStatus != CMD_SUCCESS)
        {
            EPRINT(CAMERA_INTERFACE, "fail to get image setting: [camera=%d], [numOfreq=%d]", cameraIndex, pCameraRequest->numOfRequest);
        }
        else
        {
            DPRINT(CAMERA_INTERFACE, "get image setting request successfully: [camera=%d], [numOfreq=%d]", cameraIndex, pCameraRequest->numOfRequest);
            pCameraRequest->requestCount = 0;
            pCameraRequest->httpCallback[CAM_REQ_IMG_APPEARANCE] = httpGetImageSettingParamCb;
            pCameraRequest->httpCallback[CAM_REQ_IMG_ADV_APPEARANCE] = httpGetImageSettingParamCb;
            pCameraRequest->httpCallback[CAM_REQ_IMG_DAY_NIGHT] = httpGetImageSettingParamCb;
            requestStatus = sendReqToCamera(pCameraRequest, &ipCamCfg, MAIN_STREAM);
        }
    }

    /* Error in get image setting process */
    if (requestStatus != CMD_SUCCESS)
    {
        MUTEX_LOCK(pCameraRequest->camReqFlagLock);
        pCameraRequest->camReqBusyF = FREE;
        MUTEX_UNLOCK(pCameraRequest->camReqFlagLock);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get image setting capability http callback
 * @param   httpHandle
 * @param   dataInfo
 */
static void httpGetImageSettingCapabilityCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo)
{
    UINT8               cameraIndex;
    CAMERA_REQUEST_t    *pCameraRequest;

    /* Validate input param */
    if (NULL == dataInfo)
    {
        EPRINT(CAMERA_INTERFACE, "http get image setting param response data null");
        return;
    }

    /* Get camera index from response data */
    cameraIndex = (UINT8)(GET_STREAM_INDEX(dataInfo->userData));
    pCameraRequest = &imageSettingRequest[cameraIndex].request;
    switch(dataInfo->httpResponse)
    {
        case HTTP_SUCCESS:
        {
            CAMERA_BRAND_e brand;
            CAMERA_MODEL_e model;

            /* Input data params are not valid */
            if ((dataInfo->frameSize == 0) || (dataInfo->storagePtr == NULL))
            {
                pCameraRequest->requestStatus = CMD_PROCESS_ERROR;
                break;
            }

            /* Get brand and model and acquire get image setting response paraser function */
            GetCameraBrandModelNumber(cameraIndex, &brand, &model);
            PARSER_IMAGE_SETTING_RESPONSE *pParseImageSettingResponse = ParseImageSettingResponseFunc(brand);
            if (pParseImageSettingResponse == NULL)
            {
                pCameraRequest->requestStatus = CMD_FEATURE_NOT_SUPPORTED;
                break;
            }

            if ((*pParseImageSettingResponse)(model, dataInfo->frameSize, dataInfo->storagePtr, IMAGE_SETTING_ACTION_GET_CAPABILITY, &imageSettingRequest[cameraIndex].imageCapsInfo) != CMD_SUCCESS)
            {
                EPRINT(CAMERA_INTERFACE, "fail to parse response: [camera=%d], [requestCount=%d]", cameraIndex, pCameraRequest->requestCount);
                pCameraRequest->requestStatus = CMD_PROCESS_ERROR;
                break;
            }

            /* Response parsed successfully */
            pCameraRequest->requestStatus = CMD_SUCCESS;
        }
        break;

        case HTTP_CLOSE_ON_SUCCESS:
        case HTTP_CLOSE_ON_ERROR:
        {
            IP_CAMERA_CONFIG_t ipCamCfg;

            /* Is success in response? */
            pCameraRequest->http[pCameraRequest->requestCount] = INVALID_HTTP_HANDLE;
            if (pCameraRequest->requestStatus == CMD_SUCCESS)
            {
                /* Check if any requests are pending to compose */
                pCameraRequest->requestCount++;
                if (pCameraRequest->requestCount < pCameraRequest->numOfRequest)
                {
                    ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);
                    pCameraRequest->requestStatus = sendReqToCamera(&imageSettingRequest[cameraIndex].request, &ipCamCfg, MAIN_STREAM);
                }
                else
                {
                    /* Free the get capability request and start get config request */
                    MUTEX_LOCK(pCameraRequest->camReqFlagLock);
                    pCameraRequest->camReqBusyF = FREE;
                    MUTEX_UNLOCK(pCameraRequest->camReqFlagLock);

                    /* Get image settings parameters */
                    if (FAIL == getImageSettingConfig(cameraIndex))
                    {
                        EPRINT(CAMERA_INTERFACE, "fail to get image setting config: [camera=%d]", cameraIndex);
                    }
                }
            }

            /* Is error found in response then terminate get image setting process */
            if (pCameraRequest->requestStatus != CMD_SUCCESS)
            {
                MUTEX_LOCK(pCameraRequest->camReqFlagLock);
                pCameraRequest->camReqBusyF = FREE;
                MUTEX_UNLOCK(pCameraRequest->camReqFlagLock);

                /* Reset image setting capability due to error in response */
                memset(&imageSettingRequest[cameraIndex].imageCapsInfo, 0, sizeof(imageSettingRequest[cameraIndex].imageCapsInfo));
                EPRINT(CAMERA_INTERFACE, "fail to get image setting capability: [camera=%d], [status=%d], [requestCount=%d], [numOfRequest=%d]", cameraIndex,
                       pCameraRequest->requestStatus, pCameraRequest->requestCount, pCameraRequest->numOfRequest);
            }
        }
        break;

        case HTTP_ERROR:
        {
            /* Error found in http query */
            pCameraRequest->requestStatus = CMD_PROCESS_ERROR;
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
 * @brief   Get image setting param http callback
 * @param   httpHandle
 * @param   dataInfo
 */
static void httpGetImageSettingParamCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo)
{
    UINT8               cameraIndex;
    CAMERA_REQUEST_t    *pCameraRequest;

    /* Validate input param */
    if (NULL == dataInfo)
    {
        EPRINT(CAMERA_INTERFACE, "http get image setting param response data null");
        return;
    }

    /* Get camera index from response data */
    cameraIndex = (UINT8)(GET_STREAM_INDEX(dataInfo->userData));
    pCameraRequest = &imageSettingRequest[cameraIndex].request;
    switch(dataInfo->httpResponse)
    {
        case HTTP_SUCCESS:
        {
            CAMERA_BRAND_e brand;
            CAMERA_MODEL_e model;

            /* Input data params are not valid */
            if ((dataInfo->frameSize == 0) || (dataInfo->storagePtr == NULL))
            {
                pCameraRequest->requestStatus = CMD_PROCESS_ERROR;
                break;
            }

            /* Get brand and model and acquire get image setting response paraser function */
            GetCameraBrandModelNumber(cameraIndex, &brand, &model);
            PARSER_IMAGE_SETTING_RESPONSE *pParseImageSettingResponse = ParseImageSettingResponseFunc(brand);
            if (pParseImageSettingResponse == NULL)
            {
                pCameraRequest->requestStatus = CMD_FEATURE_NOT_SUPPORTED;
                break;
            }

            if ((*pParseImageSettingResponse)(model, dataInfo->frameSize, dataInfo->storagePtr, IMAGE_SETTING_ACTION_GET_PARAM, &imageSettingRequest[cameraIndex].imageCapsInfo) != CMD_SUCCESS)
            {
                EPRINT(CAMERA_INTERFACE, "fail to parse response: [camera=%d], [requestCount=%d]", cameraIndex, pCameraRequest->requestCount);
                pCameraRequest->requestStatus = CMD_PROCESS_ERROR;
                break;
            }

            /* Response parsed successfully */
            pCameraRequest->requestStatus = CMD_SUCCESS;
        }
        break;

        case HTTP_CLOSE_ON_SUCCESS:
        case HTTP_CLOSE_ON_ERROR:
        {
            /* Is success in response? */
            pCameraRequest->http[pCameraRequest->requestCount] = INVALID_HTTP_HANDLE;
            if (pCameraRequest->requestStatus == CMD_SUCCESS)
            {
                /* Check if any requests are pending to compose */
                pCameraRequest->requestCount++;
                if (pCameraRequest->requestCount < pCameraRequest->numOfRequest)
                {
                    IP_CAMERA_CONFIG_t ipCamCfg;
                    ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);
                    pCameraRequest->requestStatus = sendReqToCamera(&imageSettingRequest[cameraIndex].request, &ipCamCfg, MAIN_STREAM);
                }
            }

            /* Terminate get image setting process if error found in response or successfully get image setting capabilities */
            if ((pCameraRequest->requestStatus != CMD_SUCCESS) || (pCameraRequest->requestCount >= pCameraRequest->numOfRequest))
            {
                MUTEX_LOCK(pCameraRequest->camReqFlagLock);
                pCameraRequest->camReqBusyF = FREE;
                MUTEX_UNLOCK(pCameraRequest->camReqFlagLock);
            }

            /* Reset image setting param if error found in response */
            if (pCameraRequest->requestStatus != CMD_SUCCESS)
            {
                /* Reset image setting capability due to error in response */
                memset(&imageSettingRequest[cameraIndex].imageCapsInfo, 0, sizeof(imageSettingRequest[cameraIndex].imageCapsInfo));
                EPRINT(CAMERA_INTERFACE, "fail to get image setting param: [camera=%d], [status=%d], [requestCount=%d], [numOfRequest=%d]", cameraIndex,
                       pCameraRequest->requestStatus, pCameraRequest->requestCount, pCameraRequest->numOfRequest);
            }

            /* Are all http queries imposed successfully? */
            if (pCameraRequest->requestCount >= pCameraRequest->numOfRequest)
            {
                DPRINT(CAMERA_INTERFACE, "get image setting param done: [camera=%d]", cameraIndex);
                saveCameraImageCapability(cameraIndex, &imageSettingRequest[cameraIndex].imageCapsInfo);
            }
        }
        break;

        case HTTP_ERROR:
        {
            /* Error found in http query */
            pCameraRequest->requestStatus = CMD_PROCESS_ERROR;
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
 * @brief   Get image setting capability tcp callback
 * @param   tcpHandle
 * @param   dataInfo
 */
static void tcpGetImageSettingCapabilityCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo)
{
    UINT8               cameraIndex = GET_STREAM_INDEX(dataInfo->camIndex);
    CAMERA_REQUEST_t    *pCameraRequest = &imageSettingRequest[cameraIndex].request;

    /* Set error status by default */
    pCameraRequest->requestStatus = CMD_PROCESS_ERROR;
    switch(dataInfo->tcpResp)
    {
        case TCP_CLOSE_ON_SUCCESS:
        case TCP_SUCCESS:
        {
            /* Validate input params */
            if (dataInfo->ciCfgData == NULL) break;

            /* Store image setting capability */
            IMAGE_CAPABILITY_INFO_t *pImageCapsInfo = dataInfo->ciCfgData;
            imageSettingRequest[cameraIndex].imageCapsInfo = *pImageCapsInfo;
            pCameraRequest->requestStatus = CMD_SUCCESS;

            /* Free the get capability request and start get config request */
            MUTEX_LOCK(pCameraRequest->camReqFlagLock);
            pCameraRequest->camReqBusyF = FREE;
            MUTEX_UNLOCK(pCameraRequest->camReqFlagLock);

            /* Get image settings parameters */
            if (FAIL == getImageSettingConfig(cameraIndex))
            {
                EPRINT(CAMERA_INTERFACE, "fail to get image setting config: [camera=%d]", cameraIndex);
            }
        }
        break;

        case TCP_CLOSE_ON_ERROR:
        case TCP_ERROR:
        default:
        {
            /* Nothing to do */
        }
        break;
    }

    /* Free memory */
    FREE_MEMORY(dataInfo->ciCfgData);

    /* On failure, stop image setting process */
    if (pCameraRequest->requestStatus != CMD_SUCCESS)
    {
        MUTEX_LOCK(pCameraRequest->camReqFlagLock);
        pCameraRequest->camReqBusyF = FREE;
        MUTEX_UNLOCK(pCameraRequest->camReqFlagLock);

        /* Reset image setting capability due to error in response */
        memset(&imageSettingRequest[cameraIndex].imageCapsInfo, 0, sizeof(imageSettingRequest[cameraIndex].imageCapsInfo));
        EPRINT(CAMERA_INTERFACE, "fail to get image setting capability: [camera=%d], [status=%d]", cameraIndex, dataInfo->tcpResp);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get image setting param tcp callback
 * @param   tcpHandle
 * @param   dataInfo
 */
static void tcpGetImageSettingParamCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo)
{
    UINT8               cameraIndex = GET_STREAM_INDEX(dataInfo->camIndex);
    CAMERA_REQUEST_t    *pCameraRequest = &imageSettingRequest[cameraIndex].request;

    /* Set error status by default */
    pCameraRequest->requestStatus = CMD_PROCESS_ERROR;
    switch(dataInfo->tcpResp)
    {
        case TCP_CLOSE_ON_SUCCESS:
        case TCP_SUCCESS:
        {
            IP_CAMERA_CONFIG_t      ipCamCfg;
            IMAGE_CAPABILITY_INFO_t *pImageCapsInfo = &imageSettingRequest[cameraIndex].imageCapsInfo;

            /* Validate input params */
            if (dataInfo->ciCfgData == NULL) break;

            /* Check previous request type */
            IMAGE_CAPABILITY_INFO_t *pCiImageCapsCfg = dataInfo->ciCfgData;
            switch(pCameraRequest->url[0].requestType)
            {
                case CAM_REQ_IMG_APPEARANCE:
                {
                    /* Reset previous request callback */
                    pCameraRequest->tcpCallback[pCameraRequest->url[0].requestType] = NULL;

                    /* Store received image setting param */
                    pImageCapsInfo->brightness.value = GET_IMAGE_SETTING_VALUE((float)pCiImageCapsCfg->brightness.value, pImageCapsInfo->brightness.step,
                                                                               pImageCapsInfo->brightness.min, pImageCapsInfo->brightness.max, BRIGHTNESS_MIN);
                    pImageCapsInfo->contrast.value = GET_IMAGE_SETTING_VALUE((float)pCiImageCapsCfg->contrast.value, pImageCapsInfo->contrast.step,
                                                                             pImageCapsInfo->contrast.min, pImageCapsInfo->contrast.max, CONTRAST_MIN);
                    pImageCapsInfo->saturation.value = GET_IMAGE_SETTING_VALUE((float)pCiImageCapsCfg->saturation.value, pImageCapsInfo->saturation.step,
                                                                               pImageCapsInfo->saturation.min, pImageCapsInfo->saturation.max, SATURATION_MIN);
                    pImageCapsInfo->hue.value = GET_IMAGE_SETTING_VALUE((float)pCiImageCapsCfg->hue.value, pImageCapsInfo->hue.step,
                                                                        pImageCapsInfo->hue.min, pImageCapsInfo->hue.max, HUE_MIN);
                    pImageCapsInfo->sharpness.value = GET_IMAGE_SETTING_VALUE((float)pCiImageCapsCfg->sharpness.value, pImageCapsInfo->sharpness.step,
                                                                              pImageCapsInfo->sharpness.min, pImageCapsInfo->sharpness.max, SHARPNESS_MIN);
                    pImageCapsInfo->whiteBalance.mode = pCiImageCapsCfg->whiteBalance.mode;

                    /* Generate get camera image setting advance appearance param request */
                    pCameraRequest->requestCount = 0;
                    GetCiRequestUrl(cameraIndex, REQ_URL_GET_IMAGE_SETTING, 0, CAM_REQ_IMG_ADV_APPEARANCE, tcpGetImageSettingParamCb, pCameraRequest, NULL);
                    DPRINT(CAMERA_INTERFACE, "get image setting advance appearance request: [camera=%d], [data=%s]", cameraIndex, pCameraRequest->url[0].relativeUrl);
                    ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);
                    pCameraRequest->requestStatus = sendReqToCamera(&imageSettingRequest[cameraIndex].request, &ipCamCfg, MAIN_STREAM);
                }
                break;

                case CAM_REQ_IMG_ADV_APPEARANCE:
                {
                    /* Reset previous request callback */
                    pCameraRequest->tcpCallback[pCameraRequest->url[0].requestType] = NULL;

                    /* Store received image setting param */
                    pImageCapsInfo->wdr.mode = pCiImageCapsCfg->wdr.mode;
                    pImageCapsInfo->backlightControl.mode = pCiImageCapsCfg->backlightControl.mode;
                    pImageCapsInfo->wdrStrength.value = GET_IMAGE_SETTING_VALUE((float)pCiImageCapsCfg->wdrStrength.value, pImageCapsInfo->wdrStrength.step,
                                                                                pImageCapsInfo->wdrStrength.min, pImageCapsInfo->wdrStrength.max, WDR_STRENGTH_MIN);
                    pImageCapsInfo->exposureRatioMode.mode = pCiImageCapsCfg->exposureRatioMode.mode;
                    pImageCapsInfo->exposureRatio.value = GET_IMAGE_SETTING_VALUE((float)pCiImageCapsCfg->exposureRatio.value, pImageCapsInfo->exposureRatio.step,
                                                                                  pImageCapsInfo->exposureRatio.min, pImageCapsInfo->exposureRatio.max, EXPOSURE_RATIO_MIN);
                    pImageCapsInfo->exposureMode.mode = pCiImageCapsCfg->exposureMode.mode;
                    pImageCapsInfo->flicker.mode = pCiImageCapsCfg->flicker.mode;
                    pImageCapsInfo->flickerStrength.value = GET_IMAGE_SETTING_VALUE((float)pCiImageCapsCfg->flickerStrength.value, pImageCapsInfo->flickerStrength.step,
                                                                                    pImageCapsInfo->flickerStrength.min, pImageCapsInfo->flickerStrength.max, FLICKER_STRENGTH_MIN);
                    pImageCapsInfo->hlc.mode = pCiImageCapsCfg->hlc.mode;
                    pImageCapsInfo->exposureTime.value = GET_IMAGE_SETTING_VALUE((float)pCiImageCapsCfg->exposureTime.value, pImageCapsInfo->exposureTime.step,
                                                                                 pImageCapsInfo->exposureTime.min, pImageCapsInfo->exposureTime.max, EXPOSURE_TIME_MIN);
                    pImageCapsInfo->exposureGain.value = GET_IMAGE_SETTING_VALUE((float)pCiImageCapsCfg->exposureGain.value, pImageCapsInfo->exposureGain.step,
                                                                                 pImageCapsInfo->exposureGain.min, pImageCapsInfo->exposureGain.max, EXPOSURE_GAIN_MIN);
                    pImageCapsInfo->exposureIris.value = GET_IMAGE_SETTING_VALUE((float)pCiImageCapsCfg->exposureIris.value, pImageCapsInfo->exposureIris.step,
                                                                                 pImageCapsInfo->exposureIris.min, pImageCapsInfo->exposureIris.max, EXPOSURE_IRIS_MIN);
                    pImageCapsInfo->normalLightGain.mode = pCiImageCapsCfg->normalLightGain.mode;
                    pImageCapsInfo->normalLightLuminance.value = GET_IMAGE_SETTING_VALUE((float)pCiImageCapsCfg->normalLightLuminance.value, pImageCapsInfo->normalLightLuminance.step,
                                                                                         pImageCapsInfo->normalLightLuminance.min, pImageCapsInfo->normalLightLuminance.max, NORMAL_LIGHT_LUMINANCE_MIN);

                    /* Generate get camera image setting day-night param request */
                    pCameraRequest->requestCount = 0;
                    GetCiRequestUrl(cameraIndex, REQ_URL_GET_IMAGE_SETTING, 0, CAM_REQ_IMG_DAY_NIGHT, tcpGetImageSettingParamCb, pCameraRequest, NULL);
                    DPRINT(CAMERA_INTERFACE, "get image setting day-night request: [camera=%d], [data=%s]", cameraIndex, pCameraRequest->url[0].relativeUrl);
                    ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);
                    pCameraRequest->requestStatus = sendReqToCamera(&imageSettingRequest[cameraIndex].request, &ipCamCfg, MAIN_STREAM);
                }
                break;

                case CAM_REQ_IMG_DAY_NIGHT:
                {
                    /* Reset previous request callback */
                    pCameraRequest->tcpCallback[pCameraRequest->url[0].requestType] = NULL;

                    /* Store received image setting param */
                    pImageCapsInfo->irLed.mode = pCiImageCapsCfg->irLed.mode;
                    pImageCapsInfo->irLedSensitivity.value = GET_IMAGE_SETTING_VALUE((float)pCiImageCapsCfg->irLedSensitivity.value, pImageCapsInfo->irLedSensitivity.step,
                                                                                     pImageCapsInfo->irLedSensitivity.min, pImageCapsInfo->irLedSensitivity.max, LED_SENSITIVITY_MIN);

                    /* Stop image setting process as required params received */
                    MUTEX_LOCK(pCameraRequest->camReqFlagLock);
                    pCameraRequest->camReqBusyF = FREE;
                    MUTEX_UNLOCK(pCameraRequest->camReqFlagLock);

                    /* Save image setting capability and param */
                    DPRINT(CAMERA_INTERFACE, "get image setting param done: [camera=%d]", cameraIndex);
                    saveCameraImageCapability(cameraIndex, &imageSettingRequest[cameraIndex].imageCapsInfo);
                    pCameraRequest->requestStatus = CMD_SUCCESS;
                }
                break;

                default:
                {
                    /* Nothing to do */
                }
                break;
            }
        }
        break;

        case TCP_CLOSE_ON_ERROR:
        case TCP_ERROR:
        default:
        {
            /* Nothing to do */
        }
        break;
    }

    /* Free memory */
    FREE_MEMORY(dataInfo->ciCfgData);

    /* On failure, stop image setting process */
    if (pCameraRequest->requestStatus != CMD_SUCCESS)
    {
        MUTEX_LOCK(pCameraRequest->camReqFlagLock);
        pCameraRequest->camReqBusyF = FREE;
        MUTEX_UNLOCK(pCameraRequest->camReqFlagLock);
        EPRINT(CAMERA_INTERFACE, "fail to get image setting capability: [camera=%d], [status=%d]", cameraIndex, dataInfo->tcpResp);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set image setting capability to camera
 * @param   cameraIndex
 * @return
 */
static BOOL setImageSettingConfig(UINT8 cameraIndex)
{
    NET_CMD_STATUS_e    requestStatus;
    CAMERA_CONFIG_t     cameraCfg;
    IP_CAMERA_CONFIG_t 	ipCamCfg;
    CAMERA_REQUEST_t    *pCameraRequest;

    /* Validate input param */
    if (cameraIndex >= getMaxCameraForCurrentVariant())
    {
        EPRINT(CAMERA_INTERFACE, "invld camera index found: [camera=%d]", cameraIndex);
        return FAIL;
    }

    ReadSingleCameraConfig(cameraIndex, &cameraCfg);
    if (cameraCfg.camera == DISABLE)
    {
        EPRINT(CAMERA_INTERFACE, "camera channel disabled: [camera=%d]", cameraIndex);
        return FAIL;
    }

    /* Check if camera is connected or not */
    if (GetCamEventStatus(cameraIndex, CONNECTION_FAILURE) == INACTIVE)
    {
        EPRINT(CAMERA_INTERFACE, "camera disconnected: [camera=%d]", cameraIndex);
        return FAIL;
    }

    pCameraRequest = &imageSettingRequest[cameraIndex].request;
    DPRINT(CAMERA_INTERFACE, "set image settings request recv: [camera=%d]", cameraIndex);
    MUTEX_LOCK(pCameraRequest->camReqFlagLock);
    if(pCameraRequest->camReqBusyF == BUSY)
    {
        MUTEX_UNLOCK(pCameraRequest->camReqFlagLock);
        EPRINT(CAMERA_INTERFACE, "session already running: [camera=%d]", cameraIndex);
        return CMD_PROCESS_ERROR;
    }
    pCameraRequest->camReqBusyF = BUSY;
    MUTEX_UNLOCK(pCameraRequest->camReqFlagLock);

    /* Set error status by default */
    pCameraRequest->requestStatus = CMD_PROCESS_ERROR;
    ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);
    if (CameraType(cameraIndex) == AUTO_ADDED_CAMERA)
    {
        /* Generate set camera image setting appearance param request */
        pCameraRequest->requestCount = 0;
        GetCiRequestUrl(cameraIndex, REQ_URL_SET_IMAGE_SETTING, 0, CAM_REQ_IMG_APPEARANCE, tcpSetImageSettingParamCb,
                        pCameraRequest, &imageSettingRequest[cameraIndex].imageCapsInfo);
        DPRINT(CAMERA_INTERFACE, "set image setting appearance request: [camera=%d], [data=%s]", cameraIndex, pCameraRequest->url[0].relativeUrl);
        requestStatus = sendReqToCamera(&imageSettingRequest[cameraIndex].request, &ipCamCfg, MAIN_STREAM);
    }
    else if (ipCamCfg.onvifSupportF == TRUE)
    {
        /* Send SetImagingSettings command to camera */
        requestStatus = sendOnvifCommonCmd(cameraIndex, ONVIF_SET_IMAGING_SETTING, onvifImagingSettingsRespCb,
                                           &ipCamCfg, &imageSettingRequest[cameraIndex].imageCapsInfo);
    }
    else
    {
        /* Set image setting into camera */
        requestStatus = GetCameraBrandModelUrl(cameraIndex, REQ_URL_SET_IMAGE_SETTING, pCameraRequest, &imageSettingRequest[cameraIndex].imageCapsInfo, NULL, NULL, NULL);
        if (requestStatus != CMD_SUCCESS)
        {
            EPRINT(CAMERA_INTERFACE, "fail to set image setting: [camera=%d], [numOfreq=%d]", cameraIndex, pCameraRequest->numOfRequest);
        }
        else
        {
            DPRINT(CAMERA_INTERFACE, "set image setting request successfully: [camera=%d], [numOfreq=%d]", cameraIndex, pCameraRequest->numOfRequest);
            pCameraRequest->requestCount = 0;
            pCameraRequest->httpCallback[CAM_REQ_IMG_APPEARANCE] = httpSetImageSettingParamCb;
            pCameraRequest->httpCallback[CAM_REQ_IMG_ADV_APPEARANCE] = httpSetImageSettingParamCb;
            pCameraRequest->httpCallback[CAM_REQ_IMG_DAY_NIGHT] = httpSetImageSettingParamCb;
            requestStatus = sendReqToCamera(&imageSettingRequest[cameraIndex].request, &ipCamCfg, MAIN_STREAM);
        }
    }

    /* Error in set image setting process */
    if (requestStatus != CMD_SUCCESS)
    {
        MUTEX_LOCK(pCameraRequest->camReqFlagLock);
        pCameraRequest->camReqBusyF = FREE;
        MUTEX_UNLOCK(pCameraRequest->camReqFlagLock);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set image setting param http callback
 * @param   httpHandle
 * @param   dataInfo
 */
static void httpSetImageSettingParamCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo)
{
    UINT8               cameraIndex;
    CAMERA_REQUEST_t    *pCameraRequest;

    /* Validate input param */
    if (NULL == dataInfo)
    {
        EPRINT(CAMERA_INTERFACE, "http set image setting param response data null");
        return;
    }

    /* Get camera index from response data */
    cameraIndex = (UINT8)(GET_STREAM_INDEX(dataInfo->userData));
    pCameraRequest = &imageSettingRequest[cameraIndex].request;
    switch(dataInfo->httpResponse)
    {
        case HTTP_SUCCESS:
        {
            CAMERA_BRAND_e brand;
            CAMERA_MODEL_e model;

            /* Input data params are not valid */
            if ((dataInfo->frameSize == 0) || (dataInfo->storagePtr == NULL))
            {
                pCameraRequest->requestStatus = CMD_PROCESS_ERROR;
                break;
            }

            /* Get brand and model and acquire set image setting param function */
            GetCameraBrandModelNumber(cameraIndex, &brand, &model);
            PARSER_IMAGE_SETTING_RESPONSE *pParseImageSettingResponse = ParseImageSettingResponseFunc(brand);
            if (pParseImageSettingResponse == NULL)
            {
                pCameraRequest->requestStatus = CMD_FEATURE_NOT_SUPPORTED;
                break;
            }

            if ((*pParseImageSettingResponse)(model, dataInfo->frameSize, dataInfo->storagePtr, IMAGE_SETTING_ACTION_SET_PARAM, NULL) != CMD_SUCCESS)
            {
                EPRINT(CAMERA_INTERFACE, "error in response: [camera=%d], [requestCount=%d]", cameraIndex, pCameraRequest->requestCount);
                pCameraRequest->requestStatus = CMD_PROCESS_ERROR;
                break;
            }

            /* Response parsed successfully */
            pCameraRequest->requestStatus = CMD_SUCCESS;
        }
        break;

        case HTTP_CLOSE_ON_SUCCESS:
        case HTTP_CLOSE_ON_ERROR:
        {
            /* Is success in response? */
            pCameraRequest->http[pCameraRequest->requestCount] = INVALID_HTTP_HANDLE;
            if (pCameraRequest->requestStatus == CMD_SUCCESS)
            {
                /* Check if any requests are pending to compose */
                pCameraRequest->requestCount++;
                if (pCameraRequest->requestCount < pCameraRequest->numOfRequest)
                {
                    IP_CAMERA_CONFIG_t ipCamCfg;
                    ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);
                    pCameraRequest->requestStatus = sendReqToCamera(&imageSettingRequest[cameraIndex].request, &ipCamCfg, MAIN_STREAM);
                }
            }

            /* Terminate set image setting process if error found in response or successfully set image setting param */
            if ((pCameraRequest->requestStatus != CMD_SUCCESS) || (pCameraRequest->requestCount >= pCameraRequest->numOfRequest))
            {
                DPRINT(CAMERA_INTERFACE, "set image setting param done: [camera=%d], [status=%d], [requestCount=%d], [numOfRequest=%d]", cameraIndex,
                       pCameraRequest->requestStatus, pCameraRequest->requestCount, pCameraRequest->numOfRequest);
                MUTEX_LOCK(pCameraRequest->camReqFlagLock);
                pCameraRequest->camReqBusyF = FREE;
                MUTEX_UNLOCK(pCameraRequest->camReqFlagLock);
            }
        }
        break;

        case HTTP_ERROR:
        {
            /* Error found in http query */
            pCameraRequest->requestStatus = CMD_PROCESS_ERROR;
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
 * @brief   tcpSetImageSettingParamCb
 * @param   tcpHandle
 * @param   dataInfo
 */
static void tcpSetImageSettingParamCb(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo)
{
    UINT8               cameraIndex = GET_STREAM_INDEX(dataInfo->camIndex);
    CAMERA_REQUEST_t    *pCameraRequest = &imageSettingRequest[cameraIndex].request;

    /* Set error status by default */
    pCameraRequest->requestStatus = CMD_PROCESS_ERROR;
    switch(dataInfo->tcpResp)
    {
        case TCP_CLOSE_ON_SUCCESS:
        case TCP_SUCCESS:
        {
            IP_CAMERA_CONFIG_t  ipCamCfg;

            /* Check previous request type */
            switch(pCameraRequest->url[0].requestType)
            {
                case CAM_REQ_IMG_APPEARANCE:
                {
                    /* Reset previous request callback */
                    pCameraRequest->tcpCallback[pCameraRequest->url[0].requestType] = NULL;

                    /* Generate set camera image setting advance appearance param request */
                    pCameraRequest->requestCount = 0;
                    GetCiRequestUrl(cameraIndex, REQ_URL_SET_IMAGE_SETTING, 0, CAM_REQ_IMG_ADV_APPEARANCE, tcpSetImageSettingParamCb,
                                    pCameraRequest, &imageSettingRequest[cameraIndex].imageCapsInfo);
                    DPRINT(CAMERA_INTERFACE, "image setting set advance appearance request: [camera=%d], [data=%s]", cameraIndex, pCameraRequest->url[0].relativeUrl);
                    ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);
                    pCameraRequest->requestStatus = sendReqToCamera(&imageSettingRequest[cameraIndex].request, &ipCamCfg, MAIN_STREAM);
                }
                break;

                case CAM_REQ_IMG_ADV_APPEARANCE:
                {
                    /* Reset previous request callback */
                    pCameraRequest->tcpCallback[pCameraRequest->url[0].requestType] = NULL;

                    /* Generate set camera image setting day-night param request */
                    pCameraRequest->requestCount = 0;
                    GetCiRequestUrl(cameraIndex, REQ_URL_SET_IMAGE_SETTING, 0, CAM_REQ_IMG_DAY_NIGHT, tcpSetImageSettingParamCb,
                                    pCameraRequest, &imageSettingRequest[cameraIndex].imageCapsInfo);
                    DPRINT(CAMERA_INTERFACE, "image setting set day-night request: [camera=%d], [data=%s]", cameraIndex, pCameraRequest->url[0].relativeUrl);
                    ReadSingleIpCameraConfig(cameraIndex, &ipCamCfg);
                    pCameraRequest->requestStatus = sendReqToCamera(&imageSettingRequest[cameraIndex].request, &ipCamCfg, MAIN_STREAM);
                }
                break;

                case CAM_REQ_IMG_DAY_NIGHT:
                {
                    /* Reset previous request callback */
                    pCameraRequest->tcpCallback[pCameraRequest->url[0].requestType] = NULL;

                    /* Stop image setting process as required params received */
                    MUTEX_LOCK(pCameraRequest->camReqFlagLock);
                    pCameraRequest->camReqBusyF = FREE;
                    MUTEX_UNLOCK(pCameraRequest->camReqFlagLock);
                    DPRINT(CAMERA_INTERFACE, "set image setting param done: [camera=%d]", cameraIndex);
                    pCameraRequest->requestStatus = CMD_SUCCESS;
                }
                break;

                default:
                {
                    /* Nothing to do */
                }
                break;
            }
        }
        break;

        case TCP_CLOSE_ON_ERROR:
        case TCP_ERROR:
        default:
        {
            /* Nothing to do */
        }
        break;
    }

    /* On failure, stop image setting process */
    if (pCameraRequest->requestStatus != CMD_SUCCESS)
    {
        MUTEX_LOCK(pCameraRequest->camReqFlagLock);
        pCameraRequest->camReqBusyF = FREE;
        MUTEX_UNLOCK(pCameraRequest->camReqFlagLock);

        /* Reset image setting capability due to error in response */
        memset(&imageSettingRequest[cameraIndex].imageCapsInfo, 0, sizeof(imageSettingRequest[cameraIndex].imageCapsInfo));
        EPRINT(CAMERA_INTERFACE, "fail to set image setting capability: [camera=%d], [status=%d]", cameraIndex, dataInfo->tcpResp);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetOnvifCamMediaSupportCapability
 * @param camIndex
 * @return - media2 supported/Not supported
 */
BOOL GetOnvifMedia2SupportCapability(UINT8 camIndex)
{
    CAMERA_CONFIG_t cameraConfig;
    IP_CAMERA_CONFIG_t ipCamConfig;

    ReadSingleCameraConfig(camIndex, &cameraConfig);
    if (cameraConfig.camera == DISABLE)
    {
        return FALSE;
    }

    ReadSingleIpCameraConfig(camIndex, &ipCamConfig);
    if (ipCamConfig.onvifSupportF == FALSE)
    {
        return FALSE;
    }

    return GetOnvifMedia2Support(camIndex);
}

/**
 * @brief resetMotionWindowConfig : resets motion detection zone & sends it to Camera
 * @param cameraIndex
 */
static void resetMotionWindowConfig(UINT8 cameraIndex)
{
    /* Deriving motion area config method */
    if (CheckforVideoAnalyticsSupport(cameraIndex) != MOTION_AREA_METHOD_BLOCK)
    {
        return;
    }

    /* Reseting motion detection window config */
    memset(&motionWindowRequest[cameraIndex].motionParam, 0, sizeof(motionWindowRequest[cameraIndex].motionParam));
    motionWindowRequest[cameraIndex].motionParam.sensitivity = 5;
    motionWindowRequest[cameraIndex].motionParam.noMotionDuration = 5;
    SetMotionWindowConfig(cameraIndex, &motionWindowRequest[cameraIndex].motionParam, NULL, 0, CLIENT_CB_TYPE_NATIVE, DISABLE);
}

//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
