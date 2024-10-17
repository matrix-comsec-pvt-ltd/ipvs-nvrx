#ifndef CAMERA_INITIATION_H
#define CAMERA_INITIATION_H
// #################################################################################################
//  FILE BRIEF
// #################################################################################################
/**
@file		CameraInitiation.h
@brief      File containing the prototype of different functions for camera initiation module
*/
// #################################################################################################
//  @INCLUDES
// #################################################################################################
/* Application Includes */
#include "ConfigApi.h"
#include "EventLogger.h"
#include "NetworkManager.h"
#include "SysTimer.h"

// #################################################################################################
//  @DEFINES
// #################################################################################################
#define MAX_AUTO_INIT_CAM_LIST  64
#define MAX_PRODUCT_NAME_SIZE   40
#define MAX_MODEL_NAME_SIZE     40
#define MAX_CMD_QUEUE           20

#define MATRIX_PRODUCT_NAME     "Matrix IP Camera"
#define MATRIX_PRODUCT_NAME_LEN 16
#define MATRIX_BRAND_NAME       "MATRIX COMSEC"
#define MATRIX_BRAND_NAME_LEN   16

// #################################################################################################
//  @DATA TYPES
// #################################################################################################
typedef enum
{
    CI_CMD_SUCCESS = 0,
    CI_CMD_OP_FAIL = 1,
    CI_CMD_REQUEST_IN_PROGRESS = 2,
    CI_CMD_INVALID_MESSAGE = 3,
    CI_CI_CMD_INVALID_SYNTAX = 4,
    CI_CMD_RESOURCE_LIMIT = 5,
    CI_CMD_INVALID_TABLE_ID = 6,
    CI_CMD_INVALID_INDEX_ID = 7,
    CI_CMD_INVALID_FIELD_ID = 8,
    CI_CMD_INVALID_INDEX_RANGE = 9,
    CI_CMD_INVALID_FILED_RANGE = 10,
    CI_CMD_INVALID_FIELD_VALUE = 11,
    CI_CMD_MAX_STREAM_LIMIT = 12,
    CI_CMD_MAX_BUFFER_LIMIT = 13,
    CI_CMD_PROCESS_ERROR = 14,
    CI_CMD_AUDIO_DISABLE = 15,
    CI_CMD_STREAM_ALREADY_ON = 16,
    CI_CMD_MAX_FIELD_LIMIT = 17,
    CI_CMD_AUD_OUT_IN_PROGRESS = 18,
    MAX_CI_CMD_STATUS,

} CI_NET_CMD_STATUS_e;

typedef struct
{
    CHAR cameraMacAddr[MAX_MAC_ADDRESS_WIDTH];
    CHAR productName[MAX_PRODUCT_NAME_SIZE];
    CHAR cameraModelname[MAX_MODEL_NAME_SIZE];
    CHAR camIpAddr[IPV6_ADDR_LEN_MAX];

} AUTO_INIT_CAM_LIST_INFO;

// #################################################################################################
//  @PROTOTYPES
// #################################################################################################
//-------------------------------------------------------------------------------------------------
void InitCameraInitiationTcpNw(void);
//-------------------------------------------------------------------------------------------------
void DeInitCameraInitiationTcpNw(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e GetCamInitList(AUTO_INIT_CAM_LIST_INFO *searchResult, UINT8 *resultCnt);
//-------------------------------------------------------------------------------------------------
BOOL AddCmdCamCount(UINT8 camIndex, UINT8PTR handle);
//-------------------------------------------------------------------------------------------------
BOOL GetCmdFd(UINT8 camIndex, INT32 *pCmdFd, UINT8 *pHandle, BOOL isTimeOut);
//-------------------------------------------------------------------------------------------------
void CloseCamCmdFd(UINT8 camIndex, UINT8 handle, BOOL isFdNeeded);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e AddInitiatedCam(AUTO_INIT_CAM_LIST_INFO *cameraList, UINT8 cameraCnt);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e RejectInitiatedCam(AUTO_INIT_CAM_LIST_INFO *cameraList, UINT8 cameraCnt);
//-------------------------------------------------------------------------------------------------
UINT8 GetCameraConnectionStatus(UINT8 camIndex);
//-------------------------------------------------------------------------------------------------
void CiServerConfigChangeNotify(GENERAL_CONFIG_t newGeneralConfig, GENERAL_CONFIG_t *oldGeneralConfig);
//-------------------------------------------------------------------------------------------------
void CiStreamCamConfigChange(CAMERA_CONFIG_t newCameraConfig, CAMERA_CONFIG_t *oldCameraConfig, UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
void GetCameraBrandModelString(UINT8 camIndex, CHARPTR *brand, CHARPTR *model);
//-------------------------------------------------------------------------------------------------
UINT8 GetCiPollDuration(void);
//-------------------------------------------------------------------------------------------------
UINT8 GetCiPollTimeInSec(void);
//-------------------------------------------------------------------------------------------------
// #################################################################################################
// @END OF FILE
// #################################################################################################
#endif /* CAMERA_INITIATION_H */
