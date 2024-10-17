#if !defined DISKUTILITY_H
#define DISKUTILITY_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		DiskUtility.h
@brief      This file describes API and data structures of disk manager module.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "SysTimer.h"
#include "DateTime.h"
#include "DiskManagerComn.h"
#include "CameraDatabase.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
// Minimum size of hard disk
#define MINIMUM_HDD_SIZE        (10UL * (GIGA_BYTE / MEGA_BYTE))
#define HDD_NORMAL_STATUS_SIZE  (MINIMUM_HDD_SIZE + 500)

//#################################################################################################
// @EXTERN VARIABLES
//#################################################################################################
extern const CHARPTR mediaNameStr[MAX_RAW_MEDIA];
extern const CHARPTR storageModeStr[MAX_HDD_MODE+1];

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
BOOL InitDiskUtility(void);
//-------------------------------------------------------------------------------------------------
BOOL DeInitDiskUtility(void);
//-------------------------------------------------------------------------------------------------
BOOL DetectStorageMedia(UDEV_DEVICE_INFO_t *deviceInfo);
//-------------------------------------------------------------------------------------------------
void UpdateLocalStorageVolumeInfo(BOOL isStorageCfgChange, BOOL logEventF);
//-------------------------------------------------------------------------------------------------
BOOL GetHddNonFuncStatus(void);
//-------------------------------------------------------------------------------------------------
void SetHddNonFuncStatus(BOOL status);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e CheckHddCofigUpdate(HDD_CONFIG_t *newHddConfig, HDD_CONFIG_t *oldHddConfig);
//-------------------------------------------------------------------------------------------------
BOOL DmHddCfgUpdate(HDD_CONFIG_t *newHddConfig, HDD_CONFIG_t *oldHddConfig);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e StopRaidVolume(RAID_VOLUME_NO_e raidVolNo);
//-------------------------------------------------------------------------------------------------
BOOL GetPhysicalDiskInfo(UINT8 hddCnt, PHYSICAL_DISK_INFO_t *diskInfo);
//-------------------------------------------------------------------------------------------------
BOOL GetDiskVolumeInfo(UINT8 volumeNum, DISK_VOLUME_INFO_t *diskVolumeInfo);
//-------------------------------------------------------------------------------------------------
void SetDiskVolumeStatus(HDD_MODE_e mode, UINT8 recordingDisk, DISK_VOL_STATUS_e status);
//-------------------------------------------------------------------------------------------------
STORAGE_HEALTH_STATUS_e GetDiskHealthStatus(void);
//-------------------------------------------------------------------------------------------------
void SetDiskHealthStatus(STORAGE_HEALTH_STATUS_e healthStatus);
//-------------------------------------------------------------------------------------------------
BOOL GetUsbHealthStatus(UINT8 usbIndex, UINT8 dummy);
//-------------------------------------------------------------------------------------------------
BOOL GetBackupDiskInfo(DM_BACKUP_TYPE_e backupType, USB_DISK_INFO_t *diskInfo);
//-------------------------------------------------------------------------------------------------
BOOL ReadyHddPartion(UINT8 volumeNum, CHARPTR advncDetail);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e CheckHddFormatDevice(UINT8 formatDevice);
//-------------------------------------------------------------------------------------------------
void ReadyUsbMedia(RAW_MEDIA_FORMAT_TYPE_e formatType, DM_BACKUP_TYPE_e backupDevice);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e CheckBackupDiskFormatStatus(DM_BACKUP_TYPE_e backupDevice);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e CheckBackupDiskPresent(DM_BACKUP_TYPE_e backupDevice);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e GiveHddStatus(HDD_MODE_e hddMode);
//-------------------------------------------------------------------------------------------------
void GetHddRecordingPath(UINT8 cameraIndex, CHARPTR path);
//-------------------------------------------------------------------------------------------------
void HddErrorHandle(UINT8 cameraIndex, HDD_CONFIG_t *hddCfgPtr);
//-------------------------------------------------------------------------------------------------
BOOL SetDiskVolumeFault(UINT8 cameraIndex, HDD_MODE_e mode, DISK_VOL_STATUS_e volStatus, CHAR *advanceDetails);
//-------------------------------------------------------------------------------------------------
UINT32 GetTotalDiskNumber(HDD_MODE_e mode);
//-------------------------------------------------------------------------------------------------
BOOL GetMountPointFromLocalDiskId(UINT32 diskId, CHARPTR path, HDD_MODE_e hddMode);
//-------------------------------------------------------------------------------------------------
BOOL GetMountPointForBackupDevice(UINT32 diskId, CHARPTR path);
//-------------------------------------------------------------------------------------------------
BOOL UpdateUsbDiskSize(UINT32 diskId);
//-------------------------------------------------------------------------------------------------
BOOL GetRecordingDiskSize(UINT8 cameraIndex, HDD_MODE_e mode, DISK_SIZE_t *diskSize);
//-------------------------------------------------------------------------------------------------
void GetVolumeSize(HDD_MODE_e mode, DISK_SIZE_t *diskSize);
//-------------------------------------------------------------------------------------------------
void GetCameraGroupVolumeSize(UINT8 cameraIndex, HDD_MODE_e mode, DISK_SIZE_t *diskSize);
//-------------------------------------------------------------------------------------------------
UINT8 GetVolumeNo(HDD_MODE_e mode);
//-------------------------------------------------------------------------------------------------
BOOL GetHddVolBuildStatus(void);
//-------------------------------------------------------------------------------------------------
void SetHddVolBuildStatus(BOOL status);
//-------------------------------------------------------------------------------------------------
BOOL RemoveBackupDisk(UINT8	deviceIndex);
//-------------------------------------------------------------------------------------------------
void StopAllCameraRecordingOnVolumeFull(BOOL forceAllStop, UINT8 fullVolumeId);
//-------------------------------------------------------------------------------------------------
BOOL SwitchAllCameraRecordingVolume(UINT8 switchVolumeId, BOOL forceSwitch);
//-------------------------------------------------------------------------------------------------
BOOL SwitchRecordingMediaVolume(UINT8 cameraIndex, BOOL forceSwitch);
//-------------------------------------------------------------------------------------------------
UINT8 GetCurRecordingDisk(UINT8 cameraIndex);
//-------------------------------------------------------------------------------------------------
void CreateCoredumpScript(CHARPTR mountPoint);
//-------------------------------------------------------------------------------------------------
BOOL isDriveMounted(CHARPTR mountPath);
//-------------------------------------------------------------------------------------------------
BOOL IsMediaVolumeNormal(HDD_MODE_e hddMode, UINT8 mediaVol);
//-------------------------------------------------------------------------------------------------
BOOL IsNormalStorageVolumeAvailable(HDD_MODE_e hddMode);
//-------------------------------------------------------------------------------------------------
const CHAR *GetStorageEventDetail(HDD_MODE_e mode, RECORD_ON_DISK_e recordDisk);
//-------------------------------------------------------------------------------------------------
void SetAllCameraVolumeHealthStatus(STORAGE_HEALTH_STATUS_e healthStatus);
//-------------------------------------------------------------------------------------------------
void SetCameraVolumeHealthStatus(UINT8 volumeId, STORAGE_HEALTH_STATUS_e healthStatus);
//-------------------------------------------------------------------------------------------------
STORAGE_HEALTH_STATUS_e GetCameraVolumeHealthStatus(UINT8 cameraIndex, RECORD_ON_DISK_e recordDisk);
//-------------------------------------------------------------------------------------------------
UINT32 GetCameraStorageVolumeAllocationMask(UINT8 cameraIndex, HDD_MODE_e mode);
//-------------------------------------------------------------------------------------------------
UINT8 GetCameraStorageGroupNoramlVolumeCnt(UINT8 cameraIndex, HDD_MODE_e mode);
//-------------------------------------------------------------------------------------------------
BOOL IsNormalVolumeAvailableInCameraStorageGroup(UINT8 cameraIndex, HDD_MODE_e mode);
//-------------------------------------------------------------------------------------------------
UINT8 GetCameraStorageAllocationGroup(UINT8 cameraIndex, HDD_MODE_e mode);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif	// end of file DISKUTILITY_H
