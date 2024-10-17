#if !defined RECORDMANAGER_H
#define RECORDMANAGER_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file   RecordManager.h
@brief  Record manager manges starting and stopping of all type of recording (Schedule/Manual/Alarm)
        for each configured channels. It provides API to start and stop recording. When one recording
        of one channel was going on and new recording request is inserted for same channel with
        different type of recording, then it will not create new session for recording. When recording
        was going on it uses "Camera Interface" module's api to get media frame and it will passes it
        to "Disk Manager" module with sufficient metadata information.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "NetworkCommand.h"
#include "Utils.h"
#include "DebugLog.h"
#include "DiskController.h"
#include "CameraInterface.h"
#include "EventHandler.h"
#include "DiskUtility.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
// This record duration is dummy value for SCHEDULE, ALARM, and MANUAL recording
#define DEFAULT_REC_DURATION        (0)

//#################################################################################################
// @DATA_TYPES
//#################################################################################################
typedef enum
{
    MANUAL_RECORD = 0,
    ALARM_RECORD,
    SCHEDULE_RECORD,
    COSEC_RECORD,
    MAX_RECORD

}RECORD_TYPE_e;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
BOOL InitRecordManager(void);
//-------------------------------------------------------------------------------------------------
BOOL DeInitRecordManager(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e StartRecord(UINT8 channelNo, RECORD_TYPE_e recType, UINT32 recDuration, CHARPTR advncDetail);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e StopRecord(UINT8 channelNo, RECORD_TYPE_e recordType, CHARPTR advncDetail);
//-------------------------------------------------------------------------------------------------
BOOL StopCameraRecordOnStorageFull(CAMERA_BIT_MASK_t cameraMask);
//-------------------------------------------------------------------------------------------------
void StopCameraRecordOnHddGroupChange(void);
//-------------------------------------------------------------------------------------------------
BOOL StopRecordDueToDiskFaultEvent(RECORD_ON_DISK_e recordDisk);
//-------------------------------------------------------------------------------------------------
BOOL EntPreAlrmRecStrm(UINT8 channelNo);
//-------------------------------------------------------------------------------------------------
BOOL ExitPreAlrmRecStrm(UINT8 channelNo);
//-------------------------------------------------------------------------------------------------
BOOL StopRecordOnHddEvent(void);
//-------------------------------------------------------------------------------------------------
BOOL StartRecordOnHddEvent(void);
//-------------------------------------------------------------------------------------------------
BOOL GetRecordStatus(UINT8 channelNo, UINT8 recType);
//-------------------------------------------------------------------------------------------------
BOOL GetAdaptiveRecordStatus(UINT8 channelNo, UINT8 value);
//-------------------------------------------------------------------------------------------------
void RmConfigChangeNotify(MANUAL_RECORD_CONFIG_t newManualRecordConfig, MANUAL_RECORD_CONFIG_t *oldManualRecordConfig, UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
BOOL SwitchRecordSession(CAMERA_BIT_MASK_t cameraMask);
//-------------------------------------------------------------------------------------------------
void RMConfigNotify(CAMERA_CONFIG_t newCameraConfig, CAMERA_CONFIG_t *oldCameraConfig, UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
void RestartRecSession(UINT8 channelNo, UINT32 errorCode);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif
