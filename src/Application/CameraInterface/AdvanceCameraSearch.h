#ifndef ADVANCECAMERASEARCH_H_
#define ADVANCECAMERASEARCH_H_
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		AdvanceCameraSearch.h
@brief      This File Provides to Search Camera using UPnP Protocol 1.2 unicast request and http
            get device information request.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "ConfigComnDef.h"

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef struct
{
    CHAR    startRangeIpAddr[IPV4_ADDR_LEN_MAX];
    CHAR    endRangeIpAddr[IPV4_ADDR_LEN_MAX];
    CHAR    brand[MAX_BRAND_NAME_LEN];
    UINT16  httpPort;
    CHAR    camUsername[MAX_CAMERA_USERNAME_WIDTH];		// user name for authentication of camera
    CHAR    camPassword[MAX_CAMERA_PASSWORD_WIDTH];		// password for authentication of camera

}ADV_IP_CAM_SEARCH_INPARAM_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitAdvCameraSearch(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e StartAdvanceCameraSearch(UINT8 nwSessionIndex, ADV_IP_CAM_SEARCH_INPARAM_t *inParam);
//-------------------------------------------------------------------------------------------------
void StopAdvanceCameraSearch(UINT8	sessionIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e GetAcqListOfAdvSearchCameras(UINT8 sessionIndex, IP_CAM_SEARCH_RESULT_t *result, UINT8PTR numOfResult);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e GenerateFailureReport(INT32 connFd, CLIENT_CB_TYPE_e callbackType, UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
BOOL IsAdvCamSearchActiveForClient(UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif /* ADVANCECAMERASEARCH_H_ */
