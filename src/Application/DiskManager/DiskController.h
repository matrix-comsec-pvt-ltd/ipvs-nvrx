#if !defined DISKCONTROLLER_H
#define DISKCONTROLLER_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		DiskController.h
@brief      This file describes API and data structures to used the full functionality of disk manager module.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "SysTimer.h"
#include "DateTime.h"
#include "DiskManagerComn.h"
#include "DiskManager.h"
#include "NwDriveManager.h"

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitDiskController(void);
//-------------------------------------------------------------------------------------------------
void DeInitDiskController(void);
//-------------------------------------------------------------------------------------------------
BOOL DmCfgChange(HDD_CONFIG_t newHddConfig, HDD_CONFIG_t *oldHddConfig);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e ValidateHddConfig(HDD_CONFIG_t newHddConfig, HDD_CONFIG_t *oldHddConfig);
//-------------------------------------------------------------------------------------------------
BOOL DmNddCfgChange(NETWORK_DRIVE_CONFIG_t newNddConfig, NETWORK_DRIVE_CONFIG_t *oldNddConfig, UINT8 nwDriveIndex);
//-------------------------------------------------------------------------------------------------
void DmStorageCfgUpdate(STORAGE_CONFIG_t newStorageConfig, STORAGE_CONFIG_t *oldStorageConfig);
//-------------------------------------------------------------------------------------------------
void StorageAllocationConfigNotify(STORAGE_ALLOCATION_CONFIG_t newCopy, STORAGE_ALLOCATION_CONFIG_t *oldCopy);
//-------------------------------------------------------------------------------------------------
void NddSizeUpdateNotify(void);
//-------------------------------------------------------------------------------------------------
void UpdateAllCameraStorage(void);
//-------------------------------------------------------------------------------------------------
BOOL UpdateCameraStorage(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
BOOL UpdateStorageMedia(UDEV_DEVICE_INFO_t *deviceInfo);
//-------------------------------------------------------------------------------------------------
BOOL GetDiskNonFuncStatus(RECORD_ON_DISK_e storageType);
//-------------------------------------------------------------------------------------------------
STORAGE_HEALTH_STATUS_e GetDiskHealthFunc(RECORD_ON_DISK_e storageType);
//-------------------------------------------------------------------------------------------------
BOOL IsStorageOperationalForRead(RECORD_ON_DISK_e storageType);
//-------------------------------------------------------------------------------------------------
BOOL GetCurrentNddStatus(RECORD_ON_DISK_e storageType);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e StopRaid(RAID_VOLUME_NO_e raidVolNo);
//-------------------------------------------------------------------------------------------------
BOOL GetPhysicalDiskPara(UINT8 hddCnt, PHYSICAL_DISK_INFO_t *physicalDiskInfo);
//-------------------------------------------------------------------------------------------------
BOOL GetBackupDiskPara(DM_BACKUP_TYPE_e backupType, USB_DISK_INFO_t *diskInfo);
//-------------------------------------------------------------------------------------------------
BOOL GetRecordingPath(UINT8 cameraIndex, CHARPTR path);
//-------------------------------------------------------------------------------------------------
BOOL UpdateUsbHealthStatus(UINT8 usbIndex, UINT8 dummy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e FormatMedia(UINT8 formatDevice, CHARPTR advDetail);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e FormatBackupMedia(DM_BACKUP_TYPE_e backupDevice);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e RemoveBackupMedia(DM_BACKUP_TYPE_e remDevice, NW_CMD_REPLY_CB callBack, INT32 clientSocket);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e GiveMediaStatus(RECORD_ON_DISK_e storageType);
//-------------------------------------------------------------------------------------------------
void HandleFileIOError(UINT8 cameraIndex, HDD_CONFIG_t *hddConfig);
//-------------------------------------------------------------------------------------------------
BOOL GetMediaHealthStatus(UINT8 dummy1, UINT8 dummy2);
//-------------------------------------------------------------------------------------------------
void SetStorageDriveStatus(RECORD_ON_DISK_e strgDrive, DISK_ACT_e action);
//-------------------------------------------------------------------------------------------------
DISK_ACT_e GetStorageDriveStatus(RECORD_ON_DISK_e strgDrive);
//-------------------------------------------------------------------------------------------------
UINT8 GetTotalMediaNo(RECORD_ON_DISK_e searchRecStorageType);
//-------------------------------------------------------------------------------------------------
BOOL GetMountPointFromDiskId(UINT8 diskId, CHARPTR mntPoint, RECORD_ON_DISK_e SearchRecStorageType);
//-------------------------------------------------------------------------------------------------
void UpdateRecordingMedia(UINT8 mediaType);
//-------------------------------------------------------------------------------------------------
BOOL CleanupRecordFileByDay(void);
//-------------------------------------------------------------------------------------------------
BOOL CleanupBkupFileByDay(void);
//-------------------------------------------------------------------------------------------------
void StopBackUpCleanUp(void);
//-------------------------------------------------------------------------------------------------
void ExitRecordCleanUpThread(void);
//-------------------------------------------------------------------------------------------------
void ExitBkUpCleanUpThread(void);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif	// end of file DISKCONTROLLER_H
