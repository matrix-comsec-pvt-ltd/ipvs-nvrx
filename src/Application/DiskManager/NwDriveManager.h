#if !defined NWDRIVEMANAGER_H
#define NWDRIVEMANAGER_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		NwDriveManager.h
@brief      This file contains all the defines and data types to use functionality of a network drive
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "Config.h"
#include "DiskManagerComn.h"
#include "BackupManager.h"
#include "PlaybackMediaStreamer.h"
#include "SyncPlaybackMediaStreamer.h"
#include "InstantPlaybackMediaStreamer.h"

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
    NETWORK_DRIVE_DISABLE,
    NETWORK_DRIVE_FORMATTING,
    NETWORK_DRIVE_MOUNTED,
    NETWORK_DRIVE_DISCONNECTED,
    NETWORK_DRIVE_UNMOUNTING,
    MAX_NETWORK_DRIVE_STATUS
}NETWORK_DRIVE_STATUS_e;

typedef enum
{
	NAS_SERVER_1,
	NAS_SERVER_2,
	MAX_NAS_SERVER,
}NAS_SERVER_e;

// Response indicating result of NAS transfer.
typedef enum
{
	NAS_SUCCESS,
	NAS_CONNECTION_ERROR,
	NAS_SIZE_NOT_AVAILABLE,
    NAS_FAIL,
    NAS_QUEUE_FULL,
    NAS_RESPONSE_MAX
}NAS_RESPONSE_e;

typedef enum
{
	MATRIX_REC_DIR,
	MATRIX_BACKUP_DIR,
	MATRIX_IMG_DIR,
	MAX_MATRIX_DIR,
}NDD_MATRIX_DIR_e;

typedef struct
{
	NAS_SERVER_e 	nasServer;
    UINT8           camIndex;
	CHAR			localFileName[MAX_FILE_NAME_SIZE];
	CHAR			remoteFile[MAX_FILE_NAME_SIZE];
}NAS_FILE_INFO_t;

//-------------------------------------------------------------------------------------------------
typedef UINT8 NAS_HANDLE;
//-------------------------------------------------------------------------------------------------
typedef void (*NAS_CALLBACK)(NAS_HANDLE	nasHandle, NAS_RESPONSE_e response, UINT16 userData);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitNwDriveManager(void);
//-------------------------------------------------------------------------------------------------
void DeInitNwDriveManager(void);
//-------------------------------------------------------------------------------------------------
BOOL NetworkDriveConfigChangeNotify(NETWORK_DRIVE_CONFIG_t newNddConfig, NETWORK_DRIVE_CONFIG_t *oldNddConfig, UINT8 nwDriveIndex);
//-------------------------------------------------------------------------------------------------
void SetNddVolBuildStatus(BOOL status);
//-------------------------------------------------------------------------------------------------
BOOL GetNddVolBuildStatus(void);
//-------------------------------------------------------------------------------------------------
BOOL ReadyNetworkDevice(UINT8 nwDriveIdx, CHARPTR advncDetail);
//-------------------------------------------------------------------------------------------------
NETWORK_DRIVE_STATUS_e GetNwDriveStatus(UINT8 iDiskId);
//-------------------------------------------------------------------------------------------------
void SetNddNonFuncStatus(BOOL status, UINT8 diskId);
//-------------------------------------------------------------------------------------------------
BOOL GetNddNonFuncStatus( UINT8 diskId);
//-------------------------------------------------------------------------------------------------
BOOL GetNasDriveSize(UINT8 nddId, DISK_SIZE_t *diskSize);
//-------------------------------------------------------------------------------------------------
BOOL GetMountPointFromNetworkDiskId(UINT32 diskId, CHARPTR path);
//-------------------------------------------------------------------------------------------------
void SetNddHealthStatus(UINT8 recDiskId, STORAGE_HEALTH_STATUS_e status);
//-------------------------------------------------------------------------------------------------
void SetNddVolumeStatus(UINT8 recordingDisk, DISK_VOL_STATUS_e status);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e GiveNddStatus(UINT8 diskId);
//-------------------------------------------------------------------------------------------------
STORAGE_HEALTH_STATUS_e GetNddHealthStatus(UINT8 diskId);
//-------------------------------------------------------------------------------------------------
BOOL GetNddVolumeInfo(UINT8 diskId, DISK_VOLUME_INFO_t *diskVolumeInfo);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e CheckNddFormatDevice(UINT8 formatDevice);
//-------------------------------------------------------------------------------------------------
BOOL CheckNddHddCfgUpdate(HDD_CONFIG_t *newHddConfig, HDD_CONFIG_t *oldHddConfig);
//-------------------------------------------------------------------------------------------------
BOOL NddHddCfgUpdate(HDD_CONFIG_t *newHddConfig, HDD_CONFIG_t *oldHddConfig);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e StartNasUpload(NAS_FILE_INFO_t *fileInfo, NAS_CALLBACK userCallback, UINT16 userData, NAS_HANDLE *nasHandle);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e NasImageUpload(NAS_FILE_INFO_t *fileInfo);
//-------------------------------------------------------------------------------------------------
BOOL GetPathOfNetworkDrive(NAS_SERVER_e serverId, NDD_MATRIX_DIR_e location, CHARPTR path);
//-------------------------------------------------------------------------------------------------
void NddErrorHandle(HDD_CONFIG_t *hddConfigPtr);
//-------------------------------------------------------------------------------------------------
void UpdateNddDiskSize(UINT8 nddId);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e NddTestConnection(UINT8 nddIdx, NETWORK_DRIVE_CONFIG_t *nddConfig, NW_CMD_REPLY_CB callBack, INT32 clientSocket);
//-------------------------------------------------------------------------------------------------
BOOL GetMountPointFromNetworkDiskIdForBackUp(UINT32 diskId, CHARPTR path);
//-------------------------------------------------------------------------------------------------
BOOL GetUpdatedNddStatus(UINT8 diskId);
//-------------------------------------------------------------------------------------------------
void UpdateNddStorageStatus(void);
//-------------------------------------------------------------------------------------------------
const CHARPTR GetNddStatusStr(NAS_RESPONSE_e nddStatus);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif /* NWDRIVEMANAGER_H_ */
