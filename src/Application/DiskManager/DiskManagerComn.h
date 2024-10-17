#if !defined DISKMANAGER_COMN_H
#define DISKMANAGER_COMN_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		DiskManagerComn.h
@brief      This file describes some comman define and data structure that will used by whole disk
            management functionality.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "CommonApi.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
// mount point size
#define	MOUNT_POINT_SIZE			150

#define	SINGLE_DISK					"HDD"
#define	SINGLE_DISK_NAME			SINGLE_DISK"%d/"
#define RAID_DISK					"RAID"
#define	RAID_DISK_NAME				RAID_DISK"%d/"
#define	NDD_DRIVE					"NDD"
#define	NDD_DRIVE_NAME				NDD_DRIVE"%d/"
#define HARDDISK_SIGNATURE_FILE		".signature"
#define CHANNEL_NAME_REC			"Camera"
#define REC_FOLDER_CHNL				"%s"CHANNEL_NAME_REC"%02d/"
#define BACKUP_REC_FOLDER_CHNL		"%s%s/Camera%02d/"
#define TFP_CHARACTER				'_'     // time field separator character
#define STM_FILE_SEP				'.'     // stream file extention separator
#define DIR_SEP						"/"     // directory separator
#define TFP							"_"     // time field separator
#define REC_FOLDER_DATE_FORMAT		"%02d"TFP"%s"TFP"%04d"
#define REC_FOLDER_HOUR_FORMAT		"%02d/"
#define REC_FOLDER_DATE_HOUR_FORMAT REC_FOLDER_DATE_FORMAT DIR_SEP REC_FOLDER_HOUR_FORMAT
#define	REC_FOLDER_DATE_FORMAT_SEP	REC_FOLDER_DATE_FORMAT DIR_SEP
#define REC_FOLDER_TIME_FORMAT		"%02d"TFP"%02d"TFP"%02d"
#define REC_INDEX_FILE_YEAR			"REC"TFP"%04d%s"

#define	RECOVERY_FOLDER				"%s.Recovery/"
#define	RECOVERY_CHANNEL_FILE		"%sChannel%d.info"
#define	OLDEST_CORRUPT_FILE			".corruptFiles"
#define	SKIPP_FILE					"skip.txt"
#define	FILE_NOT_REMOVED_STR		"File Not Removed"

// Maximum file name size specially for stream file
#define MAX_FILE_NAME_SIZE			MOUNT_POINT_SIZE

// Maximum size fo system cmd
#define SYSTEM_COMMAND_SIZE			(500)

// folder access permission in harddisk
#define	FOLDER_PERMISSION           USR_RWE_GRP_RE_OTH_RE

// file access permission for harddisk
#define	FILE_PERMISSION             USR_RWE_GRP_RE_OTH_RE

// Device serial number size : for CMS 32 character
#define	HD_SERIAL_NO_SIZE           (41)

//volume name size
#define MAX_VOLUME_NAME_SIZE        (50)
#define MAX_HDD_CAPACITY            (8)
#define MAX_PERCENTAGE_CHAR         (6)
#define MAX_USB_DISK_TYPE           (10)

/* This was a macro which is used to notify the corresponding module to stop the recording, playback
 * and backup because of disk formatting should be started as per user input. The corresponding module
 * should register its function in this macro with blocking call to this function. */
#define NOTIFY_PRE_FORMAT_HDD(x)	{StopRecordOnHddEvent();                    \
                                     StopBackupOnHddFormat();                   \
                                     RemovePlayBackForAllSessionId();           \
                                     RemoveSyncPlayBackForAllSessionId(x);      \
                                     RemoveInstantPlayBackForAllSessionId();    \
                                     ExitRecordCleanUpThread();}
#define NOTIFY_POST_FORMAT_HDD      {StartRecordOnHddEvent();}
#define HDD_FULL_ALERT(status)      {HarDiskFull(status);}
#define HDD_STORAGE_ALERT(status)   {StorageAlert(status);}

//#################################################################################################
// @DATA TYPES
//#################################################################################################
// This was enum of which type of media present on system whether it was USD of HDD
typedef enum
{
    HDD1,
    HDD2,
    HDD3,
    HDD4,
    HDD5,
    HDD6,
    HDD7,
    HDD8,
    MANUAL_BACKUP_DISK,
    SCHEDULE_BACKUP_DISK,
    MAX_RAW_MEDIA

}STORAGE_MEDIA_TYPE_e;

typedef enum
{
    RAID_VOLUME_1,
    RAID_VOLUME_2,
    RAID_VOLUME_3,
    RAID_VOLUME_4,
    MAX_RAID_VOLUME

}RAID_VOLUME_NO_e;

typedef enum
{
    RAID_VOLUME_GROUP_1,
    RAID_VOLUME_GROUP_2,
    RAID_VOLUME_GROUP_3,
    RAID_VOLUME_GROUP_4,
    MAX_RAID_VOLUME_GRP

}RAID_VOLUME_GROUP_NO_e;

typedef enum
{
    DM_MANUAL_BACKUP = 0,
    DM_SCHEDULE_BACKUP,
    DM_MAX_BACKUP

}DM_BACKUP_TYPE_e;

typedef enum
{
    DM_DISK_VOL_NORMAL,
    DM_DISK_VOL_CREATING,
    DM_DISK_VOL_FORMATTING,
    DM_DISK_VOL_FULL,
    DM_DISK_VOL_FAULT,
    DM_DISK_VOL_INCOMP_VOLUME,
    DM_DISK_VOL_CLEANUP,
    DM_DISK_NOT_CONNECTED,
    DM_DISK_VOL_READ_ONLY,
    DM_DISK_VOL_MAX

}DISK_VOL_STATUS_e;

//Gives information of status of present usb disk on USB (backup port)
typedef enum
{
    BACKUP_NO_DISK,
    BACKUP_DISK_DETECTED,
    BACKUP_DISK_FORMATTING,
    BACKUP_DISK_FAULT,
    BACKUP_DISK_UNPLUGGING

}USB_DISK_STATUS_e;

typedef enum
{
    DM_HDD_NO_DISK,
    DM_HDD_DISK_NORMAL,
    DM_HDD_DISK_FAULT

}PHYSICAL_DISK_STATUS_e;

typedef enum
{
    STRG_HLT_DISK_NORMAL,
    STRG_HLT_NO_DISK,
    STRG_HLT_DISK_FULL,
    STRG_HLT_LOW_MEMORY,
    STRG_HLT_ERROR,
    STRG_HLT_MAX

}STORAGE_HEALTH_STATUS_e;

typedef enum
{
    BACKUP_ON_USB,
    BACKUP_ON_NAS1,
    BACKUP_ON_NAS2,
    MAX_BACKUP_DEVICE

}BACKUP_DEVICE_e;

// This was enum of format type of file system on USB (Backup) device and hard disk
typedef enum
{
    EXT_4,
    FAT,
    NTFS,
    FORMAT_MAX

}RAW_MEDIA_FORMAT_TYPE_e;

typedef struct
{
    // total size in MB
    UINT32					totalSize;

    // free size in MB
    UINT32					freeSize;

    // used size in MB
    UINT32					usedSize;

}DISK_SIZE_t;

//Gives information of about serial number of disk, capacity and status of that disk.
typedef struct
{
    DISK_VOL_STATUS_e 		status;
    CHAR					volumeName[MAX_VOLUME_NAME_SIZE];	// volume name size
    CHAR					totalSize[MAX_HDD_CAPACITY + 1];		// total size of disk
    CHAR					freeSize[MAX_HDD_CAPACITY + 1];			// free size of disk
    CHAR					percentageFormat[MAX_PERCENTAGE_CHAR ];

}DISK_VOLUME_INFO_t;

//Gives information of about serial number of disk, capacity and status of that disk.
typedef struct
{
    PHYSICAL_DISK_STATUS_e	status;							// status of disk
    CHAR					serialNo[HD_SERIAL_NO_SIZE];	// serial number of disk
    CHAR					capacity[MAX_HDD_CAPACITY];		// total size of disk

}PHYSICAL_DISK_INFO_t;

//Gives information of about USB disk type, capacity and status of that disk.
typedef struct
{
    CHAR					diskType[MAX_USB_DISK_TYPE];	// disk type whether its USB CD-WR/ USB disk
    CHAR					totalSize[MAX_HDD_CAPACITY];	// total size of disk
    CHAR					freeSize[MAX_HDD_CAPACITY];		// free size of disk
    USB_DISK_STATUS_e		status;							// status of disk
    CHAR					percentageFormat[MAX_PERCENTAGE_CHAR];
    UINT8					backupStatus;

}USB_DISK_INFO_t;

typedef struct
{
    CHAR 					action[16];
    CHAR 					path[128];
    CHAR 					serial[256];
    CHAR 					baseNode[32];

}UDEV_DEVICE_INFO_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
BOOL GetSizeOfMountFs(CHARPTR mountPoint, DISK_SIZE_t *diskSize);
//-------------------------------------------------------------------------------------------------
BOOL GetFolderSize(CHARPTR dirName, UINT64PTR dirSize);
//-------------------------------------------------------------------------------------------------
BOOL RemoveDirectory(CHARPTR dirPath);
//-------------------------------------------------------------------------------------------------
void DmFileAccessCfgUpdate(FILE_ACCESS_CONFIG_t newFileAccessCfg, FILE_ACCESS_CONFIG_t *oldFileAccessCfg);
//-------------------------------------------------------------------------------------------------
BOOL WriteHddSignature(CHARPTR mountPoint);
//-------------------------------------------------------------------------------------------------
BOOL VerifyHddSignature(CHARPTR	mountPoint);
//-------------------------------------------------------------------------------------------------
void UpdateServices(BOOL status);
//-------------------------------------------------------------------------------------------------
BOOL CreateRecursiveDir(CHARPTR path, UINT32PTR pErrorCode);
//-------------------------------------------------------------------------------------------------
BOOL GetDiskNameFromMountPoint(CHARPTR srcStr, CHARPTR destStr);
//-------------------------------------------------------------------------------------------------
BOOL ParseRemoteStrmFileName(CHARPTR srcStr, CHARPTR destStr);
//-------------------------------------------------------------------------------------------------
void PreFormatDiskFunc(DISK_ACT_e action);
//-------------------------------------------------------------------------------------------------
BOOL GetHddConfigFromDiskName(CHARPTR diskName, HDD_CONFIG_t *hddConfig);
//-------------------------------------------------------------------------------------------------
BOOL EntryOldestDeleteFile(CHARPTR folderName);
//-------------------------------------------------------------------------------------------------
BOOL CheckSkipOldestFolder(CHARPTR folderName);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif	// end of file DISKMANAGER_COMN_H
