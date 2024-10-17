#ifndef AUTOCONFIGCAMERA_H_
#define AUTOCONFIGCAMERA_H_
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		AutoConfigCamera.h
@brief      This file provides to configure camera automatically
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "ConfigApi.h"
#include "CameraSearch.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define MAX_CONFIGURE_STATUS_LIST		50
#define MAX_CONFIGURE_CAMERA_LIST		MAX_CAMERA

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
    AUTO_CONFIG_AUTH_FAIL,
    AUTO_CONFIG_NO_FREE_INDEX,
    AUTO_CONFIG_REQUEST_FAILED,
    AUTO_CONFIG_MAX_IP_ADDRESS_RANGE,
    AUTO_CONFIG_IP_FAMILY_MISMATCH,
    MAX_AUTO_CONFIG_FAIL_REASON

}AUTO_CONFIG_FAIL_REASON_e;

typedef enum
{
    AUTO_CONFIG_REPORT_START,
    AUTO_CONFIG_REPORT_ABOUT_TO_CLEAR,
    AUTO_CONFIG_REPORT_CLEAR,
    MAX_AUTO_CONFIG_STATE
}AUTO_CONFIG_REPORT_STATE_e;

typedef enum
{
    AUTO_CONFIG_DEVICE_INFO,
    AUTO_CONFIG_IP_CHANGE,
    MAX_AUTO_CONFIG_REQUEST
}AUTO_CONFIG_REQUEST_e;

typedef struct
{
	UINT8	autoConfigStatus;
	UINT8	cameraIndex;
	UINT8	configFailReason;
    CHAR 	detectedIpAddress[IPV6_ADDR_LEN_MAX];
    CHAR 	changedIpAddress[IPV6_ADDR_LEN_MAX];

}AUTO_CONFIG_STATUS_t;

typedef struct
{
    AUTO_CONFIG_STATUS_t autoConfigStatusReport[MAX_CONFIGURE_STATUS_LIST];
	UINT8				 currentListIndex;
}AUTO_CONFIG_STATUS_LIST_t;

typedef struct
{
    CHAR					ipAddr[IPV6_ADDR_LEN_MAX];
    CHAR					brand[MAX_BRAND_NAME_LEN];
    CHAR					model[MAX_MODEL_NAME_LEN];
    UINT16					httpPort;
    UINT16					onvifPort;
    BOOL					onvifSupport;
    CAM_STATUS_e			camStatus;
    CHAR					camName[MAX_CAMERA_NAME_WIDTH];

}AUTO_CONFIG_REQ_PARAM_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitAutoCameraConfig(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e StartAutoConfigProcess(AUTO_CONFIG_REQ_PARAM_t *autoConfigList, UINT8 numOfResult, UINT8 sessionId);
//-------------------------------------------------------------------------------------------------
BOOL CheckIfConfiguredLessMax(UINT8PTR firstFreeIndex);
//-------------------------------------------------------------------------------------------------
void SetAutoConfigReportFlag(AUTO_CONFIG_REPORT_STATE_e state, UINT8 sessionId);
//-------------------------------------------------------------------------------------------------
AUTO_CONFIG_REPORT_STATE_e GetAutoConfigReportFlag(UINT8 sessionId);
//-------------------------------------------------------------------------------------------------
AUTO_CONFIG_STATUS_LIST_t *GetAutoConfigStatusReportData(UINT8 sessionId);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif /* AUTOCONFIGCAMERA_H_ */
