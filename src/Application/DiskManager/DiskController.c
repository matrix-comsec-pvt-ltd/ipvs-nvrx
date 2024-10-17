//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		DiskController.c
@brief      This file describes functions to used the full functionality of disk manager module.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <dirent.h>
#include <stdint.h>

/* Application Includes */
#include "DiskUtility.h"
#include "DiskController.h"
#include "FtpClient.h"
#include "RecordManager.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define FOLDER_DELETE_TIMEOUT		5	//second
#define NDD_CONFIG_TIME_OUT			5	//Second
#define MAX_LEVEL_ONE_DIRS			8
#define FTP_OP_RETRY_TIME			5
#define MAX_CLEANUP_PECENTAGE_REQ	(90)

/* Minimum hard-disk delete size */
#define MINIMUM_HDD_DEL_SIZE        (20UL * (GIGA_BYTE / KILO_BYTE))

/* Use Default Stack Size*/
#define HDD_CONFIG_THREAD_STACK_SZ              (1*MEGA_BYTE)
#define STORAGE_ALLOC_CONFIG_THREAD_STACK_SZ    (512*KILO_BYTE)
#define FORMAT_MEDIA_THREAD_STACK_SZ            (0*MEGA_BYTE)
#define FORMAT_USB_THREAD_STACK_SZ              (0*MEGA_BYTE)
#define UNPLUG_DEVICE_THREAD_STACK_SZ           (0*MEGA_BYTE)
#define RECOVERY_MEDIA_THREAD_STACK_SZ          (0*MEGA_BYTE)
#define AVI_CONVERT_THREAD_STACK_SZ             (0*MEGA_BYTE)
#define BUILD_INDEX_THREAD_STACK_SZ             (0*MEGA_BYTE)
#define STM_TO_AVI_CONVERT_THREAD_STACK_SZ      (0*MEGA_BYTE)
#define RM_OLDEST_FILE_THREAD_STACK_SZ          (0*MEGA_BYTE)
#define RM_REC_DAY_THREAD_STACK_SZ              (0*MEGA_BYTE)
#define RM_BKP_DAY_THREAD_STACK_SZ              (0*MEGA_BYTE)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
    DELETE_OLDEST_FILE,
    CLEANUP_BY_DAY,
    MAX_CLEANUP_MODE

}CLEANUP_MODE_e;

typedef enum
{
    FTP_DIR_LEVEL_ONE, 	// Where you find Directories like HDD1, NDD1, RAID, etc..
    FTP_DIR_LEVEL_TWO,  // Where you find Directories like Camera01, Camera05 etc..
    FTP_DIR_LEVEL_THREE,// Where you find Directories like 23_May_2013, 8_Aug_3333 !!!! etc..
    MAX_FTP_DIR_LEVEL,

}FTP_DIR_LEVEL_e;

typedef struct
{
    BOOL				runFlg;
    BOOL				threadExit;
    UINT32              deleteVolumeMask;
    pthread_mutex_t		clenupMutex;
    UINT64				deleteSize;

}CLEANUP_INFO_t;

typedef struct
{
    BOOL				state;
    pthread_mutex_t		mutex;
    FTP_SERVER_e		ftpIndex;
    FTP_SYS_e 			sysType;
    FTP_UPLOAD_CONFIG_t	ftpCfg;
    FTP_DIR_LEVEL_e		currDirLevel;
    FTP_RESPONSE_e		ftpResp;
    UINT8				channelNo;
    STORAGE_CONFIG_t	storageCfg;
    time_t				currTime;

}BACKUP_CLEANUP_INFO_t;

typedef struct
{
    BOOL					threadActive;
    RAW_MEDIA_FORMAT_TYPE_e	formatType;
    UINT8					mediaNo;        /* On which media format was performed */
    pthread_mutex_t			formatMutex;
    CHAR					advncDetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];

}FORMAT_INFO_t;

typedef struct
{
    BOOL					threadActive;
    HDD_CONFIG_t			newHddConfig;
    HDD_CONFIG_t			oldHddConfig;
    pthread_mutex_t			configMutex;

}HDD_CONFIG_THREAD_INFO_t;

typedef struct
{
    BOOL                    threadActive;
    pthread_mutex_t         configMutex;

}STORAGE_ALLOC_THREAD_INFO_t;

typedef struct
{
    CHAR                    mountPoint[MOUNT_POINT_SIZE];
    RECORD_ON_DISK_e        diskId;

}BUILD_REC_INDEX_PARAMS_t;

typedef struct
{
    BOOL					threadActive;
    UINT8					mediaType;
    pthread_mutex_t			recoveryMutex;

}RECOVERY_THREAD_INFO_t;

typedef struct
{
    BOOL					threadActive;
    UINT8					mediaNo;
    NW_CMD_REPLY_CB 		callBack;
    INT32 					clientSocket;
    pthread_mutex_t			mutex;

}UNPLUG_DEV_INFO_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void disableStorageAlert(void);
//-------------------------------------------------------------------------------------------------
static void setStorageMemoryAlert(STORAGE_HEALTH_STATUS_e healthStatus, HDD_MODE_e mode, RECORD_ON_DISK_e recordDisk);
//-------------------------------------------------------------------------------------------------
static VOIDPTR formatMedia(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
static VOIDPTR formatUsbMedia(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
static VOIDPTR unplugDevice(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
static VOIDPTR recoveryMedia(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
static VOIDPTR buildRecIndex(VOIDPTR mntPoint);
//-------------------------------------------------------------------------------------------------
static VOIDPTR remOldestFile(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
static VOIDPTR remRecordByDay(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
static VOIDPTR remBkUpByDay(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
static BOOL deleteOldestFileBySize(UINT32 deleteVolumeMask, UINT64 deleteSize);
//-------------------------------------------------------------------------------------------------
static BOOL getOldestFolder(UINT32 deleteVolumeMask, CHARPTR oldestFolder);
//-------------------------------------------------------------------------------------------------
static BOOL getFolderOlderThanDay(CHARPTR fromPath, UINT16 olderDay, CHARPTR oldFolder, RECOVERY_INFO_t *recvryInfo);
//-------------------------------------------------------------------------------------------------
static void setCleanUpThreadExit(CLEANUP_MODE_e cleanupMode);
//-------------------------------------------------------------------------------------------------
static BOOL getCleanUpThreadInfo(CLEANUP_MODE_e cleanupMode);
//-------------------------------------------------------------------------------------------------
static void nddPostConfigState(UINT32 data);
//-------------------------------------------------------------------------------------------------
static VOIDPTR hddConfigThread(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
static VOIDPTR storageAllocationConfigChangeThread(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
static BOOL ftpListCb(CHARPTR list, UINT32 size, VOIDPTR userData);
//-------------------------------------------------------------------------------------------------
static BOOL fileAvailableInDirectory(CHARPTR dirPath);
//-------------------------------------------------------------------------------------------------
static BOOL getFormatThreadStatus(void);
//-------------------------------------------------------------------------------------------------
static void setFormatThreadStatus(BOOL status);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static BOOL                         storageAlertState = INACTIVE;
static BOOL                         allowNddRecordRemoveF = TRUE;
static BOOL                         lastStorageStatus[STORAGE_ALLOCATION_GROUP_MAX];
static AVI_CONVERT_t                aviConvertInfo[MAX_RECORDING_MODE];
static FORMAT_INFO_t				formatInfo;
static HDD_CONFIG_THREAD_INFO_t     hddConfigThreadInfo;
static STORAGE_ALLOC_THREAD_INFO_t  storageAllocConfigThreadInfo;
static RECOVERY_THREAD_INFO_t		recoryInfo[MAX_RECORDING_MODE];
static UNPLUG_DEV_INFO_t			unplugDevInfo[DM_MAX_BACKUP];
static CLEANUP_INFO_t				cleanUpThreadInfo[MAX_CLEANUP_MODE];
static TIMER_HANDLE					nddPostCfgTmrHdl = INVALID_TIMER_HANDLE;
static BACKUP_CLEANUP_INFO_t		bkupCleanUp;
static DISK_ACT_e					storageDriveActionStatus[MAX_RECORDING_MODE];
static pthread_mutex_t				storageDriveActionStatusMutex;
static pthread_mutex_t				buildIndexSyncMutex[MAX_RECORDING_MODE];
static const CHARPTR                levelOneDirectories[MAX_LEVEL_ONE_DIRS] = {"HDD1/", "HDD2/", "HDD3/", "HDD4/", "NDD1/", "NDD2/", "RAID0/", "RAID1/"};

//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initialise all module of disk manager.
 */
void InitDiskController(void)
{
    UINT8 index;

    InitDiskUtility();

    formatInfo.threadActive = FALSE;
    hddConfigThreadInfo.threadActive = FALSE;
    storageAllocConfigThreadInfo.threadActive = FALSE;
    MUTEX_INIT(formatInfo.formatMutex, NULL);
    MUTEX_INIT(storageDriveActionStatusMutex, NULL);
    MUTEX_INIT(hddConfigThreadInfo.configMutex, NULL);
    MUTEX_INIT(storageAllocConfigThreadInfo.configMutex, NULL);
    memset(lastStorageStatus, FALSE, sizeof(lastStorageStatus));

    for(index = 0; index < MAX_RECORDING_MODE; index++)
    {
        storageDriveActionStatus[index] = DISK_ACT_NORMAL;
        recoryInfo[index].threadActive = FALSE;
        MUTEX_INIT(recoryInfo[index].recoveryMutex, NULL);
        MUTEX_INIT(buildIndexSyncMutex[index], NULL);
    }

    for(index = 0; index < DM_MAX_BACKUP; index++)
    {
        unplugDevInfo[index].threadActive = FALSE;
        MUTEX_INIT(unplugDevInfo[index].mutex, NULL);
    }

    for(index = 0; index < MAX_CLEANUP_MODE; index++)
    {
        MUTEX_INIT(cleanUpThreadInfo[index].clenupMutex, NULL);
        cleanUpThreadInfo[index].runFlg = FALSE;
        cleanUpThreadInfo[index].threadExit = FALSE;
        cleanUpThreadInfo[index].deleteVolumeMask = 0;
        cleanUpThreadInfo[index].deleteSize = 0;
    }

    MUTEX_INIT(bkupCleanUp.mutex, NULL);
    bkupCleanUp.state = INACTIVE;
    bkupCleanUp.ftpIndex = MAX_FTP_SERVER;

    InitNwDriveManager();
    InitDiskManager();
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function de-initialise all module of disk manager.
 */
void DeInitDiskController(void)
{
    SetStorageDriveStatus(ALL_DRIVE, DISK_ACT_NORMAL);
    PreFormatDiskFunc(DISK_ACT_NORMAL);
    DeInitDiskManager();

    // at shutdown forcefully stop recovery status
    DPRINT(DISK_MANAGER, "wait for recovery thread to complete");
    do
    {
        SetRecoveryStatus(STOP);
        sleep(1);

    }while(GetRecoveryStatus() == START);

    DPRINT(DISK_MANAGER, "wait for cleanup hdd thread to complete");
    ExitRecordCleanUpThread();

    DPRINT(DISK_MANAGER, "wait for format hdd thread to complete");
    while(TRUE == getFormatThreadStatus())
    {
        setFormatThreadStatus(FALSE);
        sleep(1);
    }

    /* stop file access services */
    UpdateServices(STOP);

    DeInitDiskUtility();
    DeInitNwDriveManager();
    DPRINT(DISK_MANAGER, "disk controller de-initialize successfully");
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function invokes whenever configuration was changed of Hard disk config
 * @param   newHddConfig
 * @param   oldHddConfig
 * @return  SUCCESS/FAIL
 */
BOOL DmCfgChange(HDD_CONFIG_t newHddConfig, HDD_CONFIG_t *oldHddConfig)
{
    MUTEX_LOCK(hddConfigThreadInfo.configMutex);
    if (hddConfigThreadInfo.threadActive == TRUE)
    {
        MUTEX_UNLOCK(hddConfigThreadInfo.configMutex);
        EPRINT(DISK_MANAGER, "hdd config change thread is already running");
        return FAIL;
    }
    hddConfigThreadInfo.threadActive = TRUE;
    memcpy(&hddConfigThreadInfo.newHddConfig, &newHddConfig, sizeof(HDD_CONFIG_t));
    memcpy(&hddConfigThreadInfo.oldHddConfig, oldHddConfig, sizeof(HDD_CONFIG_t));
    MUTEX_UNLOCK(hddConfigThreadInfo.configMutex);

    DPRINT(DISK_MANAGER, "recording config changed: [oldDisk=%d], [newDisk=%d]", oldHddConfig->recordDisk, newHddConfig.recordDisk);
    if (FALSE == Utils_CreateThread(NULL, hddConfigThread, NULL, DETACHED_THREAD, HDD_CONFIG_THREAD_STACK_SZ))
    {
        MUTEX_LOCK(hddConfigThreadInfo.configMutex);
        hddConfigThreadInfo.threadActive = FALSE;
        MUTEX_UNLOCK(hddConfigThreadInfo.configMutex);
        EPRINT(DISK_MANAGER, "fail to create hdd config change thread");
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This was a thread function and any hard disk configuration change it takes effect in this function.
 * @param   newHddConfig
 * @param   oldHddConfig
 * @return
 */
NET_CMD_STATUS_e ValidateHddConfig(HDD_CONFIG_t newHddConfig, HDD_CONFIG_t *oldHddConfig)
{
    NET_CMD_STATUS_e retVal = CMD_SUCCESS;

    DPRINT(DISK_MANAGER, "recording config change validation: [oldDisk=%d], [newDisk=%d]", oldHddConfig->recordDisk, newHddConfig.recordDisk);
    if ((newHddConfig.mode != oldHddConfig->mode) || (newHddConfig.recordDisk == LOCAL_HARD_DISK))
    {
        retVal = CheckHddCofigUpdate(&newHddConfig, oldHddConfig);
        if (retVal != CMD_SUCCESS)
        {
            if((retVal != CMD_HDD_PORT_COMBINATION_WRONG) && (retVal != CMD_HDD_LOGICAL_VOLUME_SIZE))
            {
                return CMD_PROCESS_ERROR;
            }

            /* Return on failure */
            return retVal;
        }
    }
    else
    {
        if (CheckNddHddCfgUpdate(&newHddConfig, oldHddConfig) == FAIL)
        {
            return CMD_PROCESS_ERROR;
        }
    }

    MUTEX_LOCK(hddConfigThreadInfo.configMutex);
    if(hddConfigThreadInfo.threadActive == TRUE)
    {
        EPRINT(DISK_MANAGER, "config change thread is already running");
        retVal = CMD_PROCESS_ERROR;
    }
    MUTEX_UNLOCK(hddConfigThreadInfo.configMutex);

    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This was a thread function and any hard disk configuration change it takes effect in this function.
 * @param   threadArg
 * @return
 */
static VOIDPTR hddConfigThread(VOIDPTR threadArg)
{
    TIMER_INFO_t	timerInfo;
    UINT8			newNwDriveIdx;

    THREAD_START("HDD_CNFG_CHNG");

    if((hddConfigThreadInfo.newHddConfig.mode != hddConfigThreadInfo.oldHddConfig.mode) || (hddConfigThreadInfo.newHddConfig.recordDisk == LOCAL_HARD_DISK))
    {
        if((hddConfigThreadInfo.newHddConfig.recordDisk == LOCAL_HARD_DISK) || (hddConfigThreadInfo.oldHddConfig.recordDisk == LOCAL_HARD_DISK))
        {
            SetHddNonFuncStatus(TRUE);
            SetStorageDriveStatus(ALL_DRIVE, DISK_ACT_CONFIG_CHANGE);
            PreFormatDiskFunc(DISK_ACT_CONFIG_CHANGE);
        }

        DmHddCfgUpdate(&hddConfigThreadInfo.newHddConfig, &hddConfigThreadInfo.oldHddConfig);
        SetStorageDriveStatus(ALL_DRIVE, DISK_ACT_NORMAL);
    }

    if((hddConfigThreadInfo.newHddConfig.recordDisk != LOCAL_HARD_DISK) && (hddConfigThreadInfo.newHddConfig.recordDisk != hddConfigThreadInfo.oldHddConfig.recordDisk))
    {
        newNwDriveIdx = (hddConfigThreadInfo.newHddConfig.recordDisk - REMOTE_NAS1);
        NddHddCfgUpdate(&hddConfigThreadInfo.newHddConfig, &hddConfigThreadInfo.oldHddConfig);

        if(GetNddNonFuncStatus(newNwDriveIdx) == FALSE)
        {
            if(nddPostCfgTmrHdl != INVALID_TIMER_HANDLE)
            {
                DPRINT(DISK_MANAGER, "ndd drive record timer already running, so delete it");
                DeleteTimer(&nddPostCfgTmrHdl);
            }

            timerInfo.count = CONVERT_SEC_TO_TIMER_COUNT(NDD_CONFIG_TIME_OUT);
            timerInfo.data = 0;
            timerInfo.funcPtr = &nddPostConfigState;
            StartTimer(timerInfo, &nddPostCfgTmrHdl);
        }

        UpdateAllCameraStorage();
    }

    MUTEX_LOCK(hddConfigThreadInfo.configMutex);
    hddConfigThreadInfo.threadActive = FALSE;
    MUTEX_UNLOCK(hddConfigThreadInfo.configMutex);
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function invokes whenever configuration was changed of network drive
 * @param   newNddConfig
 * @param   oldNddConfig
 * @param   nwDriveIndex
 * @return  SUCCESS/FAIL
 */
BOOL DmNddCfgChange(NETWORK_DRIVE_CONFIG_t newNddConfig, NETWORK_DRIVE_CONFIG_t *oldNddConfig, UINT8 nwDriveIndex)
{
    return NetworkDriveConfigChangeNotify(newNddConfig, oldNddConfig, nwDriveIndex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is timer function for giving post indication of network drive config change.
 *          So, Record manager can start recording after notification.
 * @param   data
 */
static void nddPostConfigState(UINT32 data)
{
    DPRINT(DISK_MANAGER, "ndd drive record start timer expired");
    SetStorageDriveStatus(ALL_DRIVE, DISK_ACT_NORMAL);
    NOTIFY_POST_FORMAT_HDD;
    DeleteTimer(&nddPostCfgTmrHdl);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function Gives notification of configuration was changed and it gives a local copy of updated config.
 * @param   newStorageConfig
 * @param   oldStorageConfig
 */
void DmStorageCfgUpdate(STORAGE_CONFIG_t newStorageConfig, STORAGE_CONFIG_t *oldStorageConfig)
{
    if((newStorageConfig.hddStorage.alert != oldStorageConfig->hddStorage.alert) && (newStorageConfig.hddStorage.alert == DISABLE))
    {
        disableStorageAlert();
    }

    /* Is action mode change? then update all camera storage */
    if (newStorageConfig.hddFull.actionMode != oldStorageConfig->hddFull.actionMode)
    {
        UpdateAllCameraStorage();
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Storage allocation config change notify
 * @param   newCopy
 * @param   oldCopy
 */
void StorageAllocationConfigNotify(STORAGE_ALLOCATION_CONFIG_t newCopy, STORAGE_ALLOCATION_CONFIG_t *oldCopy)
{
    HDD_CONFIG_t hddConfig;

    /* Check all volume groups */
    if (memcmp(&newCopy, oldCopy, sizeof(STORAGE_ALLOCATION_CONFIG_t)) == 0)
    {
        /* No change in configuration */
        return;
    }

    /* Read hdd configuration */
    ReadHddConfig(&hddConfig);
    if (hddConfig.recordDisk != LOCAL_HARD_DISK)
    {
        /* Recording is not on local disk */
        return;
    }

    /* Volume is building then we don't have to do anything. Recording will be started after volume building */
    if (TRUE == GetHddVolBuildStatus())
    {
        /* Volume is under building */
        return;
    }

    /* Change in storage allocation config */
    MUTEX_LOCK(storageAllocConfigThreadInfo.configMutex);
    if (storageAllocConfigThreadInfo.threadActive == TRUE)
    {
        MUTEX_UNLOCK(storageAllocConfigThreadInfo.configMutex);
        EPRINT(DISK_MANAGER, "storage allocation config change thread is already running");
        return;
    }
    storageAllocConfigThreadInfo.threadActive = TRUE;
    MUTEX_UNLOCK(storageAllocConfigThreadInfo.configMutex);

    DPRINT(DISK_MANAGER, "storage allocation config changed: [mode=%d]", hddConfig.mode);
    if (FALSE == Utils_CreateThread(NULL, storageAllocationConfigChangeThread, NULL, DETACHED_THREAD, STORAGE_ALLOC_CONFIG_THREAD_STACK_SZ))
    {
        MUTEX_LOCK(storageAllocConfigThreadInfo.configMutex);
        storageAllocConfigThreadInfo.threadActive = FALSE;
        MUTEX_UNLOCK(storageAllocConfigThreadInfo.configMutex);
        EPRINT(DISK_MANAGER, "fail to create storage allocation config change thread");
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This was a thread function and any hard disk configuration change it takes effect in this function.
 * @param   threadArg
 * @note    Recording will be started automatically if schedule recording is configured. Other recording will
 *          be started on request only.
 */
static VOIDPTR storageAllocationConfigChangeThread(VOIDPTR threadArg)
{
    THREAD_START("STRG_ALLOC_CNG");

    /* Set all camera's health status as invalid */
    SetAllCameraVolumeHealthStatus(STRG_HLT_MAX);

    /* Stop recording of all cameras */
    StopCameraRecordOnHddGroupChange();

    /* Wait for channel writer to exit */
    ChannelWriterExitWait();

    /* Update local storage volume info for all cameras */
    UpdateLocalStorageVolumeInfo(TRUE, FALSE);

    /* Update all camera storage */
    UpdateAllCameraStorage();

    /* Make config change allow for storage allocation */
    MUTEX_LOCK(storageAllocConfigThreadInfo.configMutex);
    storageAllocConfigThreadInfo.threadActive = FALSE;
    MUTEX_UNLOCK(storageAllocConfigThreadInfo.configMutex);
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   NDD size is updated after deleting the record
 */
void NddSizeUpdateNotify(void)
{
    /* Now we can remove the NDD records if required size is still not available */
    allowNddRecordRemoveF = TRUE;
    DPRINT(DISK_MANAGER, "ndd size updated, now allowed to remove more records");
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Update storage of all cameras
 */
void UpdateAllCameraStorage(void)
{
    UINT8 cameraIndex;

    for (cameraIndex = 0; cameraIndex < getMaxCameraForCurrentVariant(); cameraIndex++)
    {
        /* Update storage of all cameras */
        UpdateCameraStorage(cameraIndex);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was get the usage of storage disk and generate appropriate notification
 *          to give indication of storage alert
 * @param   cameraIndex
 * @return  SUCCESS/FAIL
 */
BOOL UpdateCameraStorage(UINT8 cameraIndex)
{
    UINT32                  minDiskVol;
    UINT8                   nddId = 0, recordVolumeId = 0;
    UINT8                   volGrpId = 0, totalStorageGroupVolume;
    HDD_CONFIG_t            hddConfig;
    DISK_SIZE_t             diskSize = {0};
    STORAGE_CONFIG_t        storageCfg;
    STORAGE_HEALTH_STATUS_e healthStatus;

    ReadHddConfig(&hddConfig);
    healthStatus = GetCameraVolumeHealthStatus(cameraIndex, hddConfig.recordDisk);
    if(hddConfig.recordDisk == LOCAL_HARD_DISK)
    {
        if((healthStatus == STRG_HLT_NO_DISK) || (healthStatus >= STRG_HLT_MAX))
        {
            EPRINT(DISK_MANAGER, "storage status not proper: [camera=%d], [healthStatus=%d]", cameraIndex, healthStatus);
            return FAIL;
        }

        /* Get current recording volume and proceed only if it is valid */
        recordVolumeId = GetCurRecordingDisk(cameraIndex);
        if (recordVolumeId >= MAX_VOLUME)
        {
            EPRINT(DISK_MANAGER, "invld record volume: [camera=%d], [volume=%d], [healthStatus=%d]", cameraIndex, recordVolumeId, healthStatus);
            return FAIL;
        }

        /* Get total available volumes for recording and minimum size */
        totalStorageGroupVolume = GetCameraStorageGroupNoramlVolumeCnt(cameraIndex, hddConfig.mode);
        minDiskVol = (MINIMUM_HDD_SIZE * totalStorageGroupVolume);

        /* If there is disk fault in system then check current recording disk is working or not */
        if (healthStatus == STRG_HLT_ERROR)
        {
            if ((totalStorageGroupVolume == 0) || (FALSE == IsNormalVolumeAvailableInCameraStorageGroup(cameraIndex, hddConfig.mode)))
            {
                EPRINT(DISK_MANAGER, "all storage volumes are faulty: [camera=%d], [mode=%s]", cameraIndex, storageModeStr[hddConfig.mode]);
                return FAIL;
            }

            /* Is current recording disk is not normal then we have to switch the recording */
            if (FALSE == IsMediaVolumeNormal(hddConfig.mode, recordVolumeId))
            {
                WPRINT(DISK_MANAGER, "current recording volume is faulty, switch the recording: [camera=%d], [mode=%s], [totalVolume=%d]",
                       cameraIndex, storageModeStr[hddConfig.mode], totalStorageGroupVolume);

                /* Force switch the recording volume on fault */
                if (SwitchAllCameraRecordingVolume(recordVolumeId, TRUE) == FAIL)
                {
                    /* Failed to switch recording volume */
                    EPRINT(DISK_MANAGER, "fail to switch recording volume on fault: [camera=%d], [mode=%s]", cameraIndex, storageModeStr[hddConfig.mode]);
                    return FAIL;
                }

                /* Get current recording volume because we forced to change it due to failure */
                recordVolumeId = GetCurRecordingDisk(cameraIndex);

                /* Restart record session in new volume */
                WPRINT(DISK_MANAGER, "force switching recording volume: [camera=%d], [mode=%s], [totalVolume=%d]",
                       cameraIndex, storageModeStr[hddConfig.mode], totalStorageGroupVolume);
            }
        }

        /* Get camera volume group */
        volGrpId = GetCameraStorageAllocationGroup(cameraIndex, hddConfig.mode);
        if (volGrpId >= STORAGE_ALLOCATION_GROUP_MAX)
        {
            volGrpId = 0;
        }

        /* Get the current recording volume size */
        if (GetRecordingDiskSize(cameraIndex, hddConfig.mode, &diskSize) == SUCCESS)
        {
            /* Free space was less than or equal to 10 GB */
            if (diskSize.freeSize < MINIMUM_HDD_SIZE)
            {
                /* If more storage volumes are available then switch the volume */
                if (totalStorageGroupVolume > 1)
                {
                    DPRINT(DISK_MANAGER, "current recording volume full, try to switch recording: [camera=%d], [recordVolume=%d], [freeSize=%d MB], [totalVolume=%d]",
                           cameraIndex, recordVolumeId, diskSize.freeSize, totalStorageGroupVolume);
                    SwitchAllCameraRecordingVolume(recordVolumeId, FALSE);

                    /* Get current recording volume because we change it due to volume full */
                    recordVolumeId = GetCurRecordingDisk(cameraIndex);
                }
            }
            else
            {
                /* NOTE: condition for after all hard disk full and then overwrite creates space for recording.
                 * so in such case, disk status should shown as normal. Here we have considered 500MB offset */
                if (diskSize.freeSize >= HDD_NORMAL_STATUS_SIZE)
                {
                    SetDiskVolumeStatus(hddConfig.mode, recordVolumeId, DM_DISK_VOL_NORMAL);
                }
            }

            /* Get total size of all volumes if more volumes are available */
            if (totalStorageGroupVolume > 1)
            {
                GetCameraGroupVolumeSize(cameraIndex, hddConfig.mode, &diskSize);
            }

            if (hddConfig.mode == SINGLE_DISK_VOLUME)
            {
                DPRINT(DISK_MANAGER, "local disk recording: [camera=%d], [diskId=%s], [volGrpId=%d], [health=%s], [totalDisk=%d], [minDiskVol=%d MB], [totalSize=%d MB], [freeSize=%d MB]",
                       cameraIndex, mediaNameStr[recordVolumeId], volGrpId, (healthStatus == STRG_HLT_ERROR) ? "FAULTY" : "NORMAL",
                        totalStorageGroupVolume, minDiskVol, diskSize.totalSize, diskSize.freeSize);
            }
            else
            {
                DPRINT(DISK_MANAGER, "raid recording: [camera=%d], [mode=%s], [volume=%d], [volGrpId=%d], [health=%s], [totalVolume=%d], [minDiskVol=%d MB], [totalSize=%d MB], [freeSize=%d MB]",
                       cameraIndex, storageModeStr[hddConfig.mode], recordVolumeId, volGrpId, (healthStatus == STRG_HLT_ERROR) ? "FAULTY" : "NORMAL",
                        totalStorageGroupVolume, minDiskVol, diskSize.totalSize, diskSize.freeSize);
            }
        }
        else
        {
            EPRINT(DISK_MANAGER, "disk size not found: [camera=%d], [mode=%s]", cameraIndex, storageModeStr[hddConfig.mode]);
        }
    }
    else
    {
        if((healthStatus == STRG_HLT_NO_DISK) || (healthStatus == STRG_HLT_ERROR))
        {
            return FAIL;
        }

        nddId = hddConfig.recordDisk - REMOTE_NAS1;
        if(GetNasDriveSize(nddId, &diskSize) == FAIL)
        {
            return FAIL;
        }

        minDiskVol = MINIMUM_HDD_SIZE;
        DPRINT(DISK_MANAGER, "network drive recording: [camera=%d], [ndd=%d], [totalSize=%d MB], [freeSize=%d MB], [minDiskVol=%d MB]",
               cameraIndex, nddId, diskSize.totalSize, diskSize.freeSize, minDiskVol);

        /* If NDD is disconnected and we call statfs for NDD storage size then it may block for undefined time.
         * Hence, we have scheduled NDD storage size update task. So that, It will take some time to update the free size
         * after removing the older records. If we consecutive call this function then it may remove all records.
         * Hence, After removing some records, wait for the free storage size to reflect. */
        if (FALSE == allowNddRecordRemoveF)
        {
            /* If we have removed some records in last function call then wait for the free storage size to update */
            if (diskSize.freeSize > minDiskVol)
            {
                /* Free size found. Hence now allow to remove the other records */
                allowNddRecordRemoveF = TRUE;
                DPRINT(DISK_MANAGER, "free size is updated, now ndd record is allowed to remove: [camera=%d], [ndd=%d]", cameraIndex, nddId);
            }
        }
    }

    ReadStorageConfig(&storageCfg);
    if(diskSize.freeSize < minDiskVol)
    {
        DPRINT(DISK_MANAGER, "storage full: [camera=%d], [volGrpId=%d], [action=%d], [lastStorageStatus=%d]",
               cameraIndex, volGrpId, storageCfg.hddFull.actionMode, lastStorageStatus[volGrpId]);
        if(diskSize.freeSize < (GIGA_BYTE / MEGA_BYTE))
        {
            if(hddConfig.recordDisk == LOCAL_HARD_DISK)
            {
                SetDiskVolumeStatus(hddConfig.mode, recordVolumeId, DM_DISK_VOL_FULL);
                SetDiskHealthStatus(STRG_HLT_DISK_FULL);
                StopAllCameraRecordingOnVolumeFull(FALSE, recordVolumeId);
            }
            else
            {
                SetNddNonFuncStatus(TRUE, nddId);
                SetNddHealthStatus(nddId, STRG_HLT_DISK_FULL);
                SetNddVolumeStatus(nddId, DM_DISK_VOL_FULL);
                StopAllCameraRecordingOnVolumeFull(TRUE, MAX_VOLUME);
            }

            lastStorageStatus[volGrpId] = TRUE;
            EPRINT(DISK_MANAGER, "hard-disk free size less than 1GB: [camera=%d], [freeSize=%d MB]", cameraIndex, diskSize.freeSize);
        }

        if (storageCfg.hddFull.actionMode == ALERT_AND_STOP)
        {
            if (lastStorageStatus[volGrpId] == FALSE)
            {
                if(hddConfig.recordDisk == LOCAL_HARD_DISK)
                {
                    SetDiskVolumeStatus(hddConfig.mode, recordVolumeId, DM_DISK_VOL_FULL);
                    SetDiskHealthStatus(STRG_HLT_DISK_FULL);
                    StopAllCameraRecordingOnVolumeFull(FALSE, recordVolumeId);
                }
                else
                {
                    SetNddNonFuncStatus(TRUE, nddId);
                    SetNddHealthStatus(nddId, STRG_HLT_DISK_FULL);
                    SetNddVolumeStatus(nddId, DM_DISK_VOL_FULL);
                    StopAllCameraRecordingOnVolumeFull(TRUE, MAX_VOLUME);
                }

                lastStorageStatus[volGrpId] = TRUE;
                SetSystemStatusLed(SYS_FULL_VOL, ON);
                WriteEvent(LOG_STORAGE_EVENT, LOG_HDD_STATUS, GetStorageEventDetail(hddConfig.mode, hddConfig.recordDisk), NULL, EVENT_FULL);
                HDD_FULL_ALERT(ACTIVE);
            }
        }
        else if(storageCfg.hddFull.actionMode == OVERWRITE)
        {
            if (hddConfig.recordDisk == LOCAL_HARD_DISK)
            {
                /* Reset all bits of camera */
                UINT32 volumeAllocMask = GetCameraStorageVolumeAllocationMask(cameraIndex, hddConfig.mode);
                if (volumeAllocMask)
                {
                    deleteOldestFileBySize(volumeAllocMask, (UINT64)MINIMUM_HDD_DEL_SIZE);
                }
            }
            else
            {
                /* Is allow to remove the NDD records? */
                if (TRUE == allowNddRecordRemoveF)
                {
                    /* Remove NDD records */
                    deleteOldestFileBySize(UINT16_MAX, (UINT64)MINIMUM_HDD_DEL_SIZE);
                    allowNddRecordRemoveF = FALSE;
                }
                else
                {
                    WPRINT(DISK_MANAGER, "storage full but ndd record not allowed to remove in overwrite: [camera=%d], [ndd=%d]", cameraIndex, nddId);
                }
            }
        }
        else if(storageCfg.hddFull.actionMode == CLEANUP)
        {
            UINT8 clenupPercent = (storageCfg.hddFull.percentCleanup > MAX_CLEANUP_PECENTAGE_REQ) ?
                                            MAX_CLEANUP_PECENTAGE_REQ : storageCfg.hddFull.percentCleanup;
            UINT64 sizeToFree = (((UINT64)((UINT64)diskSize.totalSize * KILO_BYTE) * clenupPercent) / 100);

            if(hddConfig.recordDisk == LOCAL_HARD_DISK)
            {
                /* Reset all bits of camera */
                UINT32 volumeAllocMask = GetCameraStorageVolumeAllocationMask(cameraIndex, hddConfig.mode);
                if (volumeAllocMask)
                {
                    WriteEvent(LOG_STORAGE_EVENT, LOG_HDD_CLEAN_UP, NULL, NULL, EVENT_FULL);
                    deleteOldestFileBySize(volumeAllocMask, sizeToFree);
                }
            }
            else
            {
                if (TRUE == allowNddRecordRemoveF)
                {
                    WriteEvent(LOG_STORAGE_EVENT, LOG_HDD_CLEAN_UP, NULL, NULL, EVENT_FULL);
                    deleteOldestFileBySize(UINT16_MAX, sizeToFree);
                    allowNddRecordRemoveF = FALSE;
                }
                else
                {
                    WPRINT(DISK_MANAGER, "storage full but ndd record not allowed to remove in cleanup: [camera=%d], [ndd=%d]", cameraIndex, nddId);
                }
            }
        }
    }
    else if(lastStorageStatus[volGrpId] == TRUE)
    {
        DPRINT(DISK_MANAGER, "space available after storage full: [camera=%d], [action=%d], [volGrpId=%d], [lastStorageStatus=%d]",
               cameraIndex, storageCfg.hddFull.actionMode, volGrpId, lastStorageStatus[volGrpId]);
        if(hddConfig.recordDisk == LOCAL_HARD_DISK)
        {
            SetDiskHealthStatus(STRG_HLT_DISK_NORMAL);
            SetDiskVolumeStatus(hddConfig.mode, recordVolumeId, DM_DISK_VOL_NORMAL);
            SetCameraVolumeHealthStatus(recordVolumeId, STRG_HLT_DISK_NORMAL);
        }
        else
        {
            SetNddNonFuncStatus(FALSE, nddId);
            SetNddHealthStatus(nddId, STRG_HLT_DISK_NORMAL);
            SetNddVolumeStatus(nddId, DM_DISK_VOL_NORMAL);
        }

        SetSystemStatusLed(SYS_FULL_VOL, OFF);
        WriteEvent(LOG_STORAGE_EVENT, LOG_HDD_STATUS, GetStorageEventDetail(hddConfig.mode, hddConfig.recordDisk), NULL, EVENT_NORMAL);
        HDD_FULL_ALERT(INACTIVE);
        StartRecordOnHddEvent();
        lastStorageStatus[volGrpId] = FALSE;
    }

    /* storage alert is enabled? */
    if(storageCfg.hddStorage.alert == ENABLE)
    {
        /* free size was less than storage size */
        if(((UINT32)storageCfg.hddStorage.remainingCapacity * KILO_BYTE) > diskSize.freeSize)
        {
            /* If storage alert is inactive then make it active */
            if(storageAlertState == INACTIVE)
            {
                storageAlertState = ACTIVE;
                setStorageMemoryAlert(STRG_HLT_LOW_MEMORY, hddConfig.mode, hddConfig.recordDisk);
            }
        }
        else /* free size was larger than storage size */
        {
            /* If storage alert is active then make it inactive */
            if(storageAlertState == ACTIVE)
            {
                storageAlertState = INACTIVE;
                setStorageMemoryAlert(STRG_HLT_DISK_NORMAL, hddConfig.mode, hddConfig.recordDisk);
            }
        }
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Disable storage alert if enabled
 */
static void disableStorageAlert(void)
{
    HDD_CONFIG_t hddConfig;

    ReadHddConfig(&hddConfig);
    storageAlertState = INACTIVE;
    setStorageMemoryAlert(STRG_HLT_DISK_NORMAL, hddConfig.mode, hddConfig.recordDisk);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set storage alert on memory full
 * @param   healthStatus
 * @param   mode
 * @param   recordDisk
 */
static void setStorageMemoryAlert(STORAGE_HEALTH_STATUS_e healthStatus, HDD_MODE_e mode, RECORD_ON_DISK_e recordDisk)
{
    BOOL                alertStatus;
    LOG_EVENT_STATE_e   eventState;

    if (healthStatus == STRG_HLT_LOW_MEMORY)
    {
        alertStatus = ACTIVE;
        eventState = EVENT_LOW_MEMORY;
    }
    else
    {
        alertStatus = INACTIVE;
        eventState = EVENT_NORMAL;
    }

    SetSystemStatusLed(SYS_STORAGE_ALERT, alertStatus);
    WriteEvent(LOG_STORAGE_EVENT, LOG_HDD_STATUS, GetStorageEventDetail(mode, recordDisk), NULL, eventState);
    HDD_STORAGE_ALERT(alertStatus);

    if (recordDisk == LOCAL_HARD_DISK)
    {
        SetDiskHealthStatus(healthStatus);
    }
    else
    {
        SetNddHealthStatus((recordDisk - REMOTE_NAS1), healthStatus);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function invokes whenever any disk was inserted or removed. It will stores the information
 *          of storage media and wake up the thread which will take the action against Storage media.
 * @param   deviceInfo
 * @return  SUCCESS/FAIL
 */
BOOL UpdateStorageMedia(UDEV_DEVICE_INFO_t *deviceInfo)
{
    return DetectStorageMedia(deviceInfo);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function returns hdd function status
 * @param   storageType
 * @return  TRUE/FALSE
 */
BOOL GetDiskNonFuncStatus(RECORD_ON_DISK_e storageType)
{
    if (storageType == MAX_RECORDING_MODE)
    {
        HDD_CONFIG_t hddConfig;
        ReadHddConfig(&hddConfig);
        storageType = hddConfig.recordDisk;
    }

    if (storageType == LOCAL_HARD_DISK)
    {
        return GetHddNonFuncStatus();
    }
    else if ((storageType == REMOTE_NAS1) || (storageType == REMOTE_NAS2))
    {
        return GetNddNonFuncStatus(storageType - REMOTE_NAS1);
    }

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function returns hdd function status
 * @param   storageType
 * @return
 */
STORAGE_HEALTH_STATUS_e GetDiskHealthFunc(RECORD_ON_DISK_e storageType)
{
    if(storageType == MAX_RECORDING_MODE)
    {
        HDD_CONFIG_t hddConfig;
        ReadHddConfig(&hddConfig);
        storageType = hddConfig.recordDisk;
    }

    if(storageType == LOCAL_HARD_DISK)
    {
        return GetDiskHealthStatus();
    }
    else if((storageType == REMOTE_NAS1) || (storageType == REMOTE_NAS2))
    {
        return GetNddHealthStatus(storageType - REMOTE_NAS1);
    }

    return STRG_HLT_ERROR;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Check for storage media, whether it is available or not for read operation
 * @param   storageType
 * @return  TRUE / FALSE
 */
BOOL IsStorageOperationalForRead(RECORD_ON_DISK_e storageType)
{
    HDD_CONFIG_t hddConfig;

    if (storageType == MAX_RECORDING_MODE)
    {
        ReadHddConfig(&hddConfig);
        storageType = hddConfig.recordDisk;
    }

    STORAGE_HEALTH_STATUS_e healthStatus = GetDiskHealthFunc(storageType);
    if (storageType == LOCAL_HARD_DISK)
    {
        if (healthStatus == STRG_HLT_NO_DISK)
        {
            return FALSE;
        }

        if (healthStatus == STRG_HLT_ERROR)
        {
            ReadHddConfig(&hddConfig);
            if (FALSE == IsNormalStorageVolumeAvailable(hddConfig.mode))
            {
                return FALSE;
            }
        }
    }
    else
    {
        if ((healthStatus == STRG_HLT_NO_DISK) || (healthStatus == STRG_HLT_ERROR))
        {
            return FALSE;
        }
    }

    if ((healthStatus != STRG_HLT_DISK_FULL) && (healthStatus != STRG_HLT_LOW_MEMORY) && (TRUE == GetDiskNonFuncStatus(storageType)))
    {
        return FALSE;
    }

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function returns Ndd function status, Only for Network Drive status maintain
 * @param   storageType
 * @return  TRUE/FALSE
 */
BOOL GetCurrentNddStatus(RECORD_ON_DISK_e storageType)
{
    return GetUpdatedNddStatus(storageType - REMOTE_NAS1);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function stops Raid array building process.
 * @param   raidVolNo
 * @return  SUCCESS/FAIL
 */
NET_CMD_STATUS_e StopRaid(RAID_VOLUME_NO_e raidVolNo)
{
    return StopRaidVolume(raidVolNo);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function used whenever user was request for physical disk info. It will gives Model
 *          & Serial No,Disk Capacity and status as out put parameters.
 * @param   hddCnt
 * @param   physicalDiskInfo
 * @return  SUCCESS/FAIL
 */
BOOL GetPhysicalDiskPara(UINT8 hddCnt, PHYSICAL_DISK_INFO_t *physicalDiskInfo)
{
    return GetPhysicalDiskInfo(hddCnt, physicalDiskInfo);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function used whenever user was request to get status of connected USB disk on USB
 *          (Backup) port. It will gives type of disk, total size, free size and status of disk.
 * @param   backupType
 * @param   diskInfo
 * @return  SUCCESS/FAIL
 */
BOOL GetBackupDiskPara(DM_BACKUP_TYPE_e backupType, USB_DISK_INFO_t *diskInfo)
{
    return GetBackupDiskInfo(backupType, diskInfo);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives health status of USB Media.
 * @param   usbIndex
 * @param   dummy
 * @return
 */
BOOL UpdateUsbHealthStatus(UINT8 usbIndex, UINT8 dummy)
{
    return GetUsbHealthStatus(usbIndex, dummy);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was format the hard disk present on system.
 * @param   formatDevice
 * @param   advDetail
 * @return  SUCCESS/FAIL
 */
NET_CMD_STATUS_e FormatMedia(UINT8 formatDevice, CHARPTR advDetail)
{
    NET_CMD_STATUS_e retVal;

    MUTEX_LOCK(formatInfo.formatMutex);
    if (formatInfo.threadActive == TRUE)
    {
        MUTEX_UNLOCK(formatInfo.formatMutex);
        return CMD_UNABLE_TO_FORMAT;
    }
    formatInfo.threadActive = TRUE;
    formatInfo.mediaNo = formatDevice;
    MUTEX_UNLOCK(formatInfo.formatMutex);

    if(formatDevice < MAX_VOLUME)
    {
        retVal = CheckHddFormatDevice(formatDevice);
    }
    else
    {
        retVal = CheckNddFormatDevice(formatDevice - MAX_VOLUME);
    }

    if (retVal != CMD_SUCCESS)
    {
        setFormatThreadStatus(FALSE);
        return retVal;
    }

    if(formatDevice < MAX_VOLUME)
    {
        SetHddVolBuildStatus(TRUE);
    }
    else
    {
        SetNddVolBuildStatus(TRUE);
    }

    snprintf(formatInfo.advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s", advDetail);
    if (FALSE == Utils_CreateThread(NULL, formatMedia, NULL, DETACHED_THREAD, FORMAT_MEDIA_THREAD_STACK_SZ))
    {
        if(formatDevice < MAX_VOLUME)
        {
            SetHddVolBuildStatus(FALSE);
        }
        else
        {
            SetNddVolBuildStatus(FALSE);
        }

        setFormatThreadStatus(FALSE);
        return CMD_PROCESS_ERROR;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was thread function which will format the hdd media.
 * @param   threadArg
 * @return
 */
static VOIDPTR formatMedia(VOIDPTR threadArg)
{
    BOOL                recStopped = FALSE;
    UINT8               mediaNo;
    RECORD_ON_DISK_e    storageDrive;
    RECORD_ON_DISK_e    storageFunDrive;

    MUTEX_LOCK(formatInfo.formatMutex);
    mediaNo = formatInfo.mediaNo;
    MUTEX_UNLOCK(formatInfo.formatMutex);

    if(mediaNo < MAX_VOLUME)
    {
        storageDrive = LOCAL_HARD_DISK;
        storageFunDrive = storageDrive;
    }
    else
    {
        storageDrive = (mediaNo - MAX_VOLUME);
        storageFunDrive = (storageDrive + REMOTE_NAS1);
    }

    if(GetDiskNonFuncStatus(storageFunDrive) == TRUE)
    {
        recStopped = TRUE;

        SetStorageDriveStatus(storageFunDrive, DISK_ACT_FORMAT);
        PreFormatDiskFunc(DISK_ACT_FORMAT);

        ResetAllChannelBuffer();
    }

    if(mediaNo < MAX_VOLUME)
    {
        if(ReadyHddPartion(mediaNo, formatInfo.advncDetail) == SUCCESS)
        {
            if(GetHddNonFuncStatus() == TRUE)
            {
                SetHddNonFuncStatus(FALSE);
            }
        }

        SetHddVolBuildStatus(FALSE);
    }
    else
    {
        HDD_CONFIG_t hddConfig;
        ReadHddConfig(&hddConfig);

        if(ReadyNetworkDevice(storageDrive,formatInfo.advncDetail) == SUCCESS)
        {
            if(hddConfig.recordDisk == storageFunDrive)
            {
                SetNddNonFuncStatus(FALSE, storageDrive);
            }
        }

        SetNddVolBuildStatus(FALSE);
    }

    GenerateLocalRecDatabase(storageFunDrive, INVALID_CAMERA_INDEX);
    UpdateAllCameraStorage();

    SetStorageDriveStatus(storageFunDrive, DISK_ACT_NORMAL);
    if((GetDiskNonFuncStatus(storageFunDrive) == FALSE) && (recStopped == TRUE))
    {
        NOTIFY_POST_FORMAT_HDD;
    }

    setFormatThreadStatus(FALSE);
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was format the hard disk present on system.
 * @param   backupDevice
 * @return  SUCCESS/FAIL
 */
NET_CMD_STATUS_e FormatBackupMedia(DM_BACKUP_TYPE_e backupDevice)
{
    NET_CMD_STATUS_e retVal;

    retVal = CheckBackupDiskFormatStatus(backupDevice);
    if (retVal != CMD_SUCCESS)
    {
        EPRINT(DISK_MANAGER, "format status not proper: [backupDevice=%d], [status=%d]", backupDevice, retVal);
        return retVal;
    }

    if (GetBackupInfo(backupDevice) == BACKUP_IN_PROGRESS)
    {
        /* Backup is already going on */
        EPRINT(DISK_MANAGER, "backup in progress: [backupDevice=%d]", backupDevice);
        return CMD_BACKUP_IN_PROCESS;
    }

    /* Get hdd volume build status */
    if (TRUE == GetHddVolBuildStatus())
    {
        EPRINT(DISK_MANAGER, "volume build status is not proper: [backupDevice=%d]", backupDevice);
        return CMD_UNABLE_TO_FORMAT;
    }

    MUTEX_LOCK(formatInfo.formatMutex);
    if (formatInfo.threadActive == TRUE)
    {
        MUTEX_UNLOCK(formatInfo.formatMutex);
        WPRINT(DISK_MANAGER, "format of other device is already going on: [backupDevice=%d]", backupDevice);
        return CMD_UNABLE_TO_FORMAT;
    }
    formatInfo.threadActive = TRUE;
    formatInfo.formatType = FAT;
    formatInfo.mediaNo = backupDevice;
    MUTEX_UNLOCK(formatInfo.formatMutex);

    if (FAIL == Utils_CreateThread(NULL, formatUsbMedia, NULL, DETACHED_THREAD, FORMAT_USB_THREAD_STACK_SZ))
    {
        setFormatThreadStatus(FALSE);
        EPRINT(DISK_MANAGER, "fail to create thread: [media=%s]", mediaNameStr[backupDevice + MANUAL_BACKUP_DISK]);
        return CMD_PROCESS_ERROR;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was thread function which will format the hdd media.
 * @param   threadArg
 * @return
 */
static VOIDPTR formatUsbMedia(VOIDPTR threadArg)
{
    MUTEX_LOCK(formatInfo.formatMutex);
    RAW_MEDIA_FORMAT_TYPE_e formatType = formatInfo.formatType;
    UINT8 mediaNo = formatInfo.mediaNo;
    MUTEX_UNLOCK(formatInfo.formatMutex);

    ReadyUsbMedia(formatType, mediaNo);
    setFormatThreadStatus(FALSE);
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was unmount the back up disk for safe removal.
 * @param   remDevice
 * @param   callBack
 * @param   clientSocket
 * @return
 */
NET_CMD_STATUS_e RemoveBackupMedia(DM_BACKUP_TYPE_e remDevice, NW_CMD_REPLY_CB callBack, INT32 clientSocket)
{
    NET_CMD_STATUS_e retVal;

    retVal = CheckBackupDiskPresent(remDevice);
    if (retVal != CMD_SUCCESS)
    {
        return retVal;
    }

    MUTEX_LOCK(unplugDevInfo[remDevice].mutex);
    if(unplugDevInfo[remDevice].threadActive == TRUE)
    {
        MUTEX_UNLOCK(unplugDevInfo[remDevice].mutex);
        return CMD_PROCESS_ERROR;
    }
    unplugDevInfo[remDevice].threadActive = TRUE;
    MUTEX_UNLOCK(unplugDevInfo[remDevice].mutex);

    unplugDevInfo[remDevice].callBack = callBack;
    unplugDevInfo[remDevice].clientSocket = clientSocket;
    unplugDevInfo[remDevice].mediaNo = remDevice;

    if (FAIL == Utils_CreateThread(NULL, unplugDevice, &unplugDevInfo[remDevice], DETACHED_THREAD, UNPLUG_DEVICE_THREAD_STACK_SZ))
    {
        MUTEX_LOCK(unplugDevInfo[remDevice].mutex);
        unplugDevInfo[remDevice].threadActive = FALSE;
        MUTEX_UNLOCK(unplugDevInfo[remDevice].mutex);
        EPRINT(DISK_MANAGER, "fail to create media removal thread: [media=%s]", mediaNameStr[remDevice + MANUAL_BACKUP_DISK]);
        return CMD_PROCESS_ERROR;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is thread function which removes backup device. It is removing disk by unmounting disk.
 * @param   threadArg
 * @return
 */
static VOIDPTR unplugDevice(VOIDPTR threadArg)
{
    NET_CMD_STATUS_e        retVal = CMD_SUCCESS;
    UNPLUG_DEV_INFO_t       *devInfo = threadArg;
    STORAGE_MEDIA_TYPE_e    deviceIndex;
    CHAR                    eventDetail[MAX_EVENT_DETAIL_SIZE];

    deviceIndex = (devInfo->mediaNo + MANUAL_BACKUP_DISK);
    if (FAIL == RemoveBackupDisk(deviceIndex))
    {
        retVal = CMD_USB_DEVICE_IN_USE;
        EPRINT(DISK_MANAGER, "fail to unmound device: [media=%s]", mediaNameStr[deviceIndex]);
    }

    if(devInfo->callBack != NULL)
    {
        devInfo->callBack(retVal, devInfo->clientSocket, TRUE);
        devInfo->callBack = NULL;
        devInfo->clientSocket = INVALID_CONNECTION;
    }

    if(retVal == CMD_SUCCESS)
    {
        snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE,"%02d", (devInfo->mediaNo + 1));
        WriteEvent(LOG_OTHER_EVENT, LOG_USB_STATUS, eventDetail, NULL, EVENT_DISCONNECT);
    }

    MUTEX_LOCK(devInfo->mutex);
    devInfo->threadActive = FALSE;
    MUTEX_UNLOCK(devInfo->mutex);
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives hard disk status in which recording is enable
 * @param   storageType
 * @return
 */
NET_CMD_STATUS_e GiveMediaStatus(RECORD_ON_DISK_e storageType)
{
    HDD_CONFIG_t hddConfig;

    ReadHddConfig(&hddConfig);
    if(storageType == MAX_RECORDING_MODE)
    {
        storageType = hddConfig.recordDisk;
    }

    if(storageType == LOCAL_HARD_DISK)
    {
        return GiveHddStatus(hddConfig.mode);
    }
    else if(storageType == REMOTE_NAS1 || storageType == REMOTE_NAS2)
    {
        return GiveNddStatus(storageType - REMOTE_NAS1);
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   When writting data into hard disk generates error then this function invokes and stops
 *          hard disk related functionality.
 * @param   cameraIndex
 * @param   hddConfig
 */
void HandleFileIOError(UINT8 cameraIndex, HDD_CONFIG_t *hddConfig)
{
    if(hddConfig->recordDisk == LOCAL_HARD_DISK)
    {
        SetHddNonFuncStatus(TRUE);
    }
    else
    {
        SetNddNonFuncStatus(TRUE, (hddConfig->recordDisk - REMOTE_NAS1));
    }

    SetStorageDriveStatus(hddConfig->recordDisk, DISK_ACT_IO_ERROR);
    PreFormatDiskFunc(DISK_ACT_IO_ERROR);

    if(hddConfig->recordDisk == LOCAL_HARD_DISK)
    {
        HddErrorHandle(cameraIndex, hddConfig);
    }
    else
    {
        NddErrorHandle(hddConfig);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives health status of hard disk.
 * @param   dummy1
 * @param   dummy2
 * @return
 */
BOOL GetMediaHealthStatus(UINT8 dummy1, UINT8 dummy2)
{
    HDD_CONFIG_t hddConfig;

    ReadHddConfig(&hddConfig);
    if(hddConfig.recordDisk == LOCAL_HARD_DISK)
    {
        return GetDiskHealthStatus();
    }
    else
    {
        return GetNddHealthStatus(hddConfig.recordDisk - REMOTE_NAS1);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function set the status of current action on drive.
 * @param   strgDrive
 * @param   action
 */
void SetStorageDriveStatus(RECORD_ON_DISK_e strgDrive, DISK_ACT_e action)
{
    if(strgDrive == MAX_RECORDING_MODE)
    {
        HDD_CONFIG_t hddConfig;
        ReadHddConfig(&hddConfig);
        strgDrive = hddConfig.recordDisk;
    }

    MUTEX_LOCK(storageDriveActionStatusMutex);
    if(strgDrive < MAX_RECORDING_MODE)
    {
        storageDriveActionStatus[strgDrive] = action;
    }
    else if(strgDrive == ALL_DRIVE)
    {
        for (strgDrive = 0; strgDrive < MAX_RECORDING_MODE; strgDrive++)
        {
            storageDriveActionStatus[strgDrive] = action;
        }
    }
    MUTEX_UNLOCK(storageDriveActionStatusMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function get the status of current action on drive.
 * @param   strgDrive
 * @return
 */
DISK_ACT_e GetStorageDriveStatus(RECORD_ON_DISK_e strgDrive)
{
    DISK_ACT_e diskAction = DISK_ACT_MAX;

    if(strgDrive == MAX_RECORDING_MODE)
    {
        HDD_CONFIG_t hddConfig;
        ReadHddConfig(&hddConfig);
        strgDrive = hddConfig.recordDisk;
    }

    if (strgDrive < MAX_RECORDING_MODE)
    {
        MUTEX_LOCK(storageDriveActionStatusMutex);
        diskAction = storageDriveActionStatus[strgDrive];
        MUTEX_UNLOCK(storageDriveActionStatusMutex);
    }

    return diskAction;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives total number of recordable media present.
 * @param   searchRecStorageType
 * @return
 */
UINT8 GetTotalMediaNo(RECORD_ON_DISK_e searchRecStorageType)
{
    HDD_CONFIG_t hddConfig;

    ReadHddConfig(&hddConfig);
    if(searchRecStorageType == MAX_RECORDING_MODE)
    {
        searchRecStorageType = hddConfig.recordDisk;
    }

    if(searchRecStorageType == LOCAL_HARD_DISK)
    {
        return GetTotalDiskNumber(hddConfig.mode);
    }
    else if((searchRecStorageType == REMOTE_NAS1) || (searchRecStorageType == REMOTE_NAS2))
    {
        return 1;
    }

    return 0;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function mount point from disk id, if media properly present.
 * @param   diskId
 * @param   mntPoint
 * @param   searchRecStorageType
 * @return  SUCCESS/FAIL
 */
BOOL GetMountPointFromDiskId(UINT8 diskId, CHARPTR mntPoint, RECORD_ON_DISK_e searchRecStorageType)
{
    HDD_CONFIG_t hddConfig;

    ReadHddConfig(&hddConfig);
    if(searchRecStorageType == MAX_RECORDING_MODE)
    {
        searchRecStorageType = hddConfig.recordDisk;
    }

    if(searchRecStorageType == LOCAL_HARD_DISK)
    {
        return GetMountPointFromLocalDiskId(diskId, mntPoint, hddConfig.mode);
    }
    else if((searchRecStorageType == REMOTE_NAS1) || (searchRecStorageType == REMOTE_NAS2))
    {
        return GetMountPointFromNetworkDiskId((searchRecStorageType - REMOTE_NAS1), mntPoint);
    }

    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives recording path.
 * @param   cameraIndex
 * @param   path
 * @return
 */
BOOL GetRecordingPath(UINT8 cameraIndex, CHARPTR path)
{
    HDD_CONFIG_t hddConfig;

    ReadHddConfig(&hddConfig);
    if (GetDiskNonFuncStatus(MAX_RECORDING_MODE) == TRUE)
    {
        return FAIL;
    }

    if(hddConfig.recordDisk == LOCAL_HARD_DISK)
    {
        GetHddRecordingPath(cameraIndex, path);
    }
    else
    {
        if (FAIL == GetMountPointFromNetworkDiskId((hddConfig.recordDisk - REMOTE_NAS1), path))
        {
            return FAIL;
        }
    }

    if (path[0] == '\0')
    {
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   When any media detected in system and mounted properly then we need to start recovery.
 * @param   mediaType
 */
void UpdateRecordingMedia(UINT8 mediaType)
{
    if (mediaType >= MAX_RECORDING_MODE)
    {
        return;
    }

    MUTEX_LOCK(recoryInfo[mediaType].recoveryMutex);
    if(recoryInfo[mediaType].threadActive == TRUE)
    {
        MUTEX_UNLOCK(recoryInfo[mediaType].recoveryMutex);
        return;
    }
    recoryInfo[mediaType].threadActive = TRUE;
    recoryInfo[mediaType].mediaType = mediaType;
    MUTEX_UNLOCK(recoryInfo[mediaType].recoveryMutex);

    if (FAIL == Utils_CreateThread(NULL, recoveryMedia, &recoryInfo[mediaType], DETACHED_THREAD, RECOVERY_MEDIA_THREAD_STACK_SZ))
    {
        MUTEX_LOCK(recoryInfo[mediaType].recoveryMutex);
        recoryInfo[mediaType].threadActive = FALSE;
        MUTEX_UNLOCK(recoryInfo[mediaType].recoveryMutex);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function starts recovery of media files.
 * @param   threadArg
 * @return
 */
static VOIDPTR recoveryMedia(VOIDPTR threadArg)
{
    BOOL                        status = TRUE;
    BOOL                        recPostNotify = FALSE;
    RECOVERY_THREAD_INFO_t      *recInfo = (RECOVERY_THREAD_INFO_t *)threadArg;
    BOOL                        recoveryEventLogF = FALSE;
    CHAR                        mntPoint[MOUNT_POINT_SIZE];
    UINT8                       diskCnt;
    UINT8                       totalDiskCnt, cameraIndex;
    HDD_CONFIG_t                hddConfig;
    GENERAL_CONFIG_t            aviRecGenConfig;
    CHAR                        recoveryFldr[MAX_FILE_NAME_SIZE];
    CHAR                        recvrFilename[MAX_FILE_NAME_SIZE];
    INT32                       recFileFd;
    BUILD_REC_INDEX_PARAMS_t    *pBuildParams = NULL;

    THREAD_START_INDEX("RCVRY_MEDIA", recInfo->mediaType);
    ReadGeneralConfig(&aviRecGenConfig);
    ReadHddConfig(&hddConfig);

    if(recInfo->mediaType == LOCAL_HARD_DISK)
    {
        totalDiskCnt = GetTotalDiskNumber(hddConfig.mode);
        for(diskCnt = 0; diskCnt < totalDiskCnt; diskCnt++)
        {
            if(GetMountPointFromLocalDiskId(diskCnt, mntPoint, hddConfig.mode) == FAIL)
            {
                continue;
            }

            /* Check recovery start event logged or not */
            if (FALSE == recoveryEventLogF)
            {
                /* Write recovery start event */
                WriteEvent(LOG_STORAGE_EVENT, LOG_HDD_STATUS, GetStorageEventDetail(hddConfig.mode, recInfo->mediaType), "Recovery Start", EVENT_DISK_BUSY);

                /* No need log recovery start event for all disks. It will be logged for first valid disk. */
                recoveryEventLogF = TRUE;
            }

            if((aviRecGenConfig.recordFormatType == REC_AVI_FORMAT) || (aviRecGenConfig.recordFormatType == REC_BOTH_FORMAT))
            {
                snprintf(aviConvertInfo[recInfo->mediaType].aviRecMountPoint, MOUNT_POINT_SIZE, "%s", mntPoint);
                snprintf(recoveryFldr, MAX_FILE_NAME_SIZE, RECOVERY_FOLDER, mntPoint);
                if(access(recoveryFldr, F_OK) == STATUS_OK)
                {
                    for (cameraIndex = 0; cameraIndex < getMaxCameraForCurrentVariant(); cameraIndex++)
                    {
                        /* Check channel file was present */
                        snprintf(recvrFilename, MAX_FILE_NAME_SIZE, RECOVERY_CHANNEL_FILE, recoveryFldr, GET_CAMERA_NO(cameraIndex));
                        if(access(recvrFilename, F_OK) != STATUS_OK)
                        {
                            continue;
                        }

                        /* Open file for camera channel */
                        recFileFd = open(recvrFilename, READ_WRITE_MODE, FILE_PERMISSION);
                        if (recFileFd == INVALID_FILE_FD)
                        {
                            EPRINT(DISK_MANAGER, "fail to open recovery file: [path=%s], [err=%s]", recvrFilename, STR_ERR);
                            continue;
                        }

                        /* Read the data from file. Read last modified date and hour from that file */
                        if(read(recFileFd, &aviConvertInfo[recInfo->mediaType].recoveryInfoForAvi[cameraIndex], RECOVERY_FIELD_SIZE) != RECOVERY_FIELD_SIZE)
                        {
                            EPRINT(DISK_MANAGER, "fail to read recovery file: [path=%s], [err=%s]", recvrFilename, STR_ERR);
                        }
                        close(recFileFd);
                    }
                }

                SetStorageDriveStatus(recInfo->mediaType, DISK_ACT_RECOVERY);
                StartRecovery(mntPoint, recInfo->mediaType);

                DPRINT(DISK_MANAGER, "recovery mount point for avi: [mntPoint=%s]", aviConvertInfo[recInfo->mediaType].aviRecMountPoint);
                if (FAIL == Utils_CreateThread(NULL, StmToAviConvertor, &aviConvertInfo[recInfo->mediaType], DETACHED_THREAD, AVI_CONVERT_THREAD_STACK_SZ))
                {
                    EPRINT(DISK_MANAGER, "fail to create stmToAviConvertor thread");
                }
            }
            else
            {
                SetStorageDriveStatus(recInfo->mediaType, DISK_ACT_RECOVERY);
                StartRecovery(mntPoint, recInfo->mediaType);
            }

            /* Alloc memory as we have to pass the info to index build thread */
            status = FALSE;
            pBuildParams = (BUILD_REC_INDEX_PARAMS_t*)malloc(sizeof(BUILD_REC_INDEX_PARAMS_t));
            if (NULL == pBuildParams)
            {
                EPRINT(DISK_MANAGER, "fail to alloc memory for record build index thread: [path=%s]", mntPoint);
                continue;
            }

            snprintf(pBuildParams->mountPoint, MOUNT_POINT_SIZE, "%s", mntPoint);
            pBuildParams->diskId = LOCAL_HARD_DISK;
            if (FAIL == Utils_CreateThread(NULL, buildRecIndex, pBuildParams, DETACHED_THREAD, BUILD_INDEX_THREAD_STACK_SZ))
            {
                EPRINT(DISK_MANAGER, "fail to create record build index thread: [path=%s]", mntPoint);
                FREE_MEMORY(pBuildParams);
            }
        }

        SetHddVolBuildStatus(FALSE);
        SetHddNonFuncStatus(status);
        recPostNotify = TRUE;

        /* Write recovery stop event if recovery start event generated */
        if (TRUE == recoveryEventLogF)
        {
            WriteEvent(LOG_STORAGE_EVENT, LOG_HDD_STATUS, GetStorageEventDetail(hddConfig.mode, recInfo->mediaType), "Recovery Stop", EVENT_NORMAL);
        }
    }
    else
    {
        if(GetMountPointFromNetworkDiskId((recInfo->mediaType - REMOTE_NAS1), mntPoint) == SUCCESS)
        {
            /* Network drive found for recovery */
            WriteEvent(LOG_STORAGE_EVENT, LOG_HDD_STATUS, GetStorageEventDetail(MAX_HDD_MODE, recInfo->mediaType), "Recovery Start", EVENT_DISK_BUSY);

            if((aviRecGenConfig.recordFormatType == REC_AVI_FORMAT) || (aviRecGenConfig.recordFormatType == REC_BOTH_FORMAT))
            {
                snprintf(aviConvertInfo[recInfo->mediaType].aviRecMountPoint, MOUNT_POINT_SIZE, "%s", mntPoint);
                snprintf(recoveryFldr, MAX_FILE_NAME_SIZE, RECOVERY_FOLDER , mntPoint);
                if(access(recoveryFldr, F_OK) == STATUS_OK)
                {
                    for (cameraIndex = 0; cameraIndex < getMaxCameraForCurrentVariant(); cameraIndex++)
                    {
                        /* Check channel file was present */
                        snprintf(recvrFilename, MAX_FILE_NAME_SIZE, RECOVERY_CHANNEL_FILE,recoveryFldr, GET_CAMERA_NO(cameraIndex));
                        if(access(recvrFilename, F_OK) != STATUS_OK)
                        {
                            continue;
                        }

                        /* Open file for camera channel */
                        recFileFd = open(recvrFilename,READ_WRITE_MODE, FILE_PERMISSION);
                        if (recFileFd == INVALID_FILE_FD)
                        {
                            EPRINT(DISK_MANAGER, "fail to open recovery file: [path=%s], [err=%s]", recvrFilename, STR_ERR);
                            continue;
                        }

                        /* Read the data from file. Read last modified date and hour from that file */
                        if(read(recFileFd, &aviConvertInfo[recInfo->mediaType].recoveryInfoForAvi[cameraIndex], RECOVERY_FIELD_SIZE) != RECOVERY_FIELD_SIZE)
                        {
                            EPRINT(DISK_MANAGER, "fail to read recovery file: [path=%s], [err=%s]", recvrFilename, STR_ERR);
                        }
                    }
                }

                if((UINT8)hddConfig.recordDisk == recInfo->mediaType)
                {
                    SetNddNonFuncStatus(TRUE, (recInfo->mediaType - REMOTE_NAS1));
                    PreFormatDiskFunc(DISK_ACT_RECOVERY);
                }

                SetStorageDriveStatus(recInfo->mediaType, DISK_ACT_RECOVERY);
                status = StartRecovery(mntPoint, recInfo->mediaType);

                if (FAIL == Utils_CreateThread(NULL, StmToAviConvertor, &aviConvertInfo[recInfo->mediaType], DETACHED_THREAD, STM_TO_AVI_CONVERT_THREAD_STACK_SZ))
                {
                    EPRINT(DISK_MANAGER, "fail to create stmToAviConvertor thread");
                }
            }
            else
            {
                /** Before the below condition was not present, due to that when Network device is mounted then both recovery
                 *  and recording is started in that case the recovery of presently recorded file is started which fails and
                 *  because of that whole hour folder got deleted, With below condition recording will only start after the recovery */
                if((UINT8)hddConfig.recordDisk == recInfo->mediaType)
                {
                    SetNddNonFuncStatus(TRUE, (recInfo->mediaType - REMOTE_NAS1));
                    PreFormatDiskFunc(DISK_ACT_RECOVERY);
                }

                SetStorageDriveStatus(recInfo->mediaType, DISK_ACT_RECOVERY);
                status = StartRecovery(mntPoint, recInfo->mediaType);
            }

            if (((UINT8)hddConfig.recordDisk == recInfo->mediaType) && (NETWORK_DRIVE_MOUNTED == GetNwDriveStatus(recInfo->mediaType - REMOTE_NAS1)))
            {
                SetNddNonFuncStatus(FALSE, (recInfo->mediaType - REMOTE_NAS1));
                recPostNotify = TRUE;
            }
            else if ((status == SUCCESS) && (NETWORK_DRIVE_MOUNTED == GetNwDriveStatus(recInfo->mediaType - REMOTE_NAS1)))
            {
                /* Update status of network drive for recording search */
                SetNddNonFuncStatus(FALSE, (recInfo->mediaType - REMOTE_NAS1));
            }

            /* Alloc memory as we have to pass the info to index build thread */
            pBuildParams = (BUILD_REC_INDEX_PARAMS_t*)malloc(sizeof(BUILD_REC_INDEX_PARAMS_t));
            if (NULL == pBuildParams)
            {
                EPRINT(DISK_MANAGER, "fail to alloc memory for record build index thread: [path=%s]", mntPoint);
            }
            else
            {
                snprintf(pBuildParams->mountPoint, MOUNT_POINT_SIZE, "%s", mntPoint);
                pBuildParams->diskId = (RECORD_ON_DISK_e)recInfo->mediaType;
                if (FAIL == Utils_CreateThread(NULL, buildRecIndex, pBuildParams, DETACHED_THREAD, BUILD_INDEX_THREAD_STACK_SZ))
                {
                    EPRINT(DISK_MANAGER, "fail to create record build index thread: [path=%s]", mntPoint);
                    FREE_MEMORY(pBuildParams);
                }
            }

            WriteEvent(LOG_STORAGE_EVENT, LOG_HDD_STATUS, GetStorageEventDetail(MAX_HDD_MODE, recInfo->mediaType), "Recovery Stop", EVENT_NORMAL);
        }
        else if((UINT8)hddConfig.recordDisk == recInfo->mediaType)
        {
            SetNddNonFuncStatus(TRUE, (recInfo->mediaType - REMOTE_NAS1));
            PreFormatDiskFunc(DISK_ACT_RECOVERY);
        }
    }

    UpdateAllCameraStorage();
    SetStorageDriveStatus(recInfo->mediaType, DISK_ACT_NORMAL);
    if(recPostNotify == TRUE)
    {
        NOTIFY_POST_FORMAT_HDD;
    }

    sleep(1);
    MUTEX_LOCK(buildIndexSyncMutex[recInfo->mediaType]);
    GenerateLocalRecDatabase((RECORD_ON_DISK_e)recInfo->mediaType, INVALID_CAMERA_INDEX);
    MUTEX_UNLOCK(buildIndexSyncMutex[recInfo->mediaType]);

    MUTEX_LOCK(recInfo->recoveryMutex);
    recInfo->threadActive = FALSE;
    MUTEX_UNLOCK(recInfo->recoveryMutex);
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This thread calls the function which build the recording index map file with prev stored data.
 * @param   threadArg
 * @return
 */
static VOIDPTR buildRecIndex(VOIDPTR threadArg)
{
    BUILD_REC_INDEX_PARAMS_t *buildParam = (BUILD_REC_INDEX_PARAMS_t *)threadArg;

    DPRINT(DISK_MANAGER, "recovery index building started: [path=%s]", buildParam->mountPoint);
    MUTEX_LOCK(buildIndexSyncMutex[buildParam->diskId]);
    BuildRecIndexFromPrevStoredData(buildParam->mountPoint);
    MUTEX_UNLOCK(buildIndexSyncMutex[buildParam->diskId]);
    DPRINT(DISK_MANAGER, "recovery index building completed: [path=%s]", buildParam->mountPoint);
    FREE_MEMORY(buildParam);
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function exits current running cleanup thread.
 */
void ExitRecordCleanUpThread(void)
{
    UINT8           tRetryCnt;
    CLEANUP_MODE_e  cleanupMode;

    for(cleanupMode = 0; cleanupMode < MAX_CLEANUP_MODE; cleanupMode++)
    {
        tRetryCnt = 0;
        setCleanUpThreadExit(cleanupMode);

        while (TRUE == getCleanUpThreadInfo(cleanupMode))
        {
            sleep(1);
            tRetryCnt++;
            if(tRetryCnt > 30)
            {
                EPRINT(DISK_MANAGER, "not waiting for cleanup thread: [cleanupMode=%d]", cleanupMode);
                break;
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function exits current running back up cleanup thread.
 */
void ExitBkUpCleanUpThread(void)
{
    /* Give Trigger to Thread */
    MUTEX_LOCK(bkupCleanUp.mutex);
    if (bkupCleanUp.state != ACTIVE)
    {
        MUTEX_UNLOCK(bkupCleanUp.mutex);
        return;
    }
    bkupCleanUp.state = INTERRUPTED;
    MUTEX_UNLOCK(bkupCleanUp.mutex);

    while(TRUE)
    {
        MUTEX_LOCK(bkupCleanUp.mutex);
        if (bkupCleanUp.state == INACTIVE)
        {
            MUTEX_UNLOCK(bkupCleanUp.mutex);
            break;
        }
        MUTEX_UNLOCK(bkupCleanUp.mutex);

        /* Give some sleep to change status by other thread */
        usleep(10000);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function exits current running cleanup thread.
 * @param   cleanupMode
 */
static void setCleanUpThreadExit(CLEANUP_MODE_e cleanupMode)
{
    MUTEX_LOCK(cleanUpThreadInfo[cleanupMode].clenupMutex);
    cleanUpThreadInfo[cleanupMode].threadExit = TRUE;
    MUTEX_UNLOCK(cleanUpThreadInfo[cleanupMode].clenupMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives running status of cleanup mode.
 * @param   cleanupMode
 * @return
 */
static BOOL getCleanUpThreadInfo(CLEANUP_MODE_e cleanupMode)
{
    MUTEX_LOCK(cleanUpThreadInfo[cleanupMode].clenupMutex);
    BOOL threadStatus = cleanUpThreadInfo[cleanupMode].runFlg;
    MUTEX_UNLOCK(cleanUpThreadInfo[cleanupMode].clenupMutex);
    return threadStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will remove oldest file by size
 * @param   deleteVolumeMask
 * @param   deleteSize
 * @return  SUCCESS/FAIL
 */
static BOOL deleteOldestFileBySize(UINT32 deleteVolumeMask, UINT64 deleteSize)
{
    CLEANUP_MODE_e cleanupMode = DELETE_OLDEST_FILE;

    MUTEX_LOCK(cleanUpThreadInfo[cleanupMode].clenupMutex);
    if(cleanUpThreadInfo[cleanupMode].runFlg == TRUE)
    {
        MUTEX_UNLOCK(cleanUpThreadInfo[cleanupMode].clenupMutex);
        return SUCCESS;
    }

    cleanUpThreadInfo[cleanupMode].deleteVolumeMask = deleteVolumeMask;
    cleanUpThreadInfo[cleanupMode].deleteSize = deleteSize;
    cleanUpThreadInfo[cleanupMode].runFlg = TRUE;
    cleanUpThreadInfo[cleanupMode].threadExit = FALSE;
    MUTEX_UNLOCK(cleanUpThreadInfo[cleanupMode].clenupMutex);

    /* Creates a thread which will remove the oldest file from disk and its argument is 20 GB or required size in MB */
    if (FAIL == Utils_CreateThread(NULL, remOldestFile, NULL, DETACHED_THREAD, RM_OLDEST_FILE_THREAD_STACK_SZ))
    {
        MUTEX_LOCK(cleanUpThreadInfo[cleanupMode].clenupMutex);
        cleanUpThreadInfo[cleanupMode].runFlg = FALSE;
        cleanUpThreadInfo[cleanupMode].threadExit = FALSE;
        MUTEX_UNLOCK(cleanUpThreadInfo[cleanupMode].clenupMutex);
        EPRINT(DISK_MANAGER, "fail to create delete oldest file thread");
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This was a thread function. This function will remove oldest file with its folder from
 *          hard disk. In this function if oldest file for two channel was same then it was delete
 *          the lowest channel files. This function create create free space of input size.
 * @param   threadArg
 * @return
 */
static VOIDPTR remOldestFile(VOIDPTR threadArg)
{
    CLEANUP_MODE_e	cleanupMode = DELETE_OLDEST_FILE;
    UINT64			dirSize;
    UINT64			removeSize = 0;
    UINT64			delFileSize;
    UINT32          deleteVolumeMask;
    CHAR			oldestFolder[MAX_FILE_NAME_SIZE];
    HDD_CONFIG_t    hddConfig;

    THREAD_START("REM_OLDEST_FILE");

    /* We generate disk full event at 10 GB space was available in storage media and we remove atleast
     * 20 GB or user supply (size) data which is oldest in recorded stream form */
    MUTEX_LOCK(cleanUpThreadInfo[cleanupMode].clenupMutex);
    deleteVolumeMask = cleanUpThreadInfo[cleanupMode].deleteVolumeMask;
    delFileSize = cleanUpThreadInfo[cleanupMode].deleteSize;

    do
    {
        MUTEX_UNLOCK(cleanUpThreadInfo[cleanupMode].clenupMutex);
        /* here we need to find out oldest folder in recorded data */
        if(getOldestFolder(deleteVolumeMask, oldestFolder) == FAIL)
        {
            EPRINT(DISK_MANAGER, "oldest folder not found for cleanup");
            MUTEX_LOCK(cleanUpThreadInfo[cleanupMode].clenupMutex);
            sleep(FOLDER_DELETE_TIMEOUT);
            break;
        }

        if(GetFolderSize(oldestFolder, &dirSize) == SUCCESS)
        {
            /* Remove oldest file */
            removeSize += (dirSize/KILO_BYTE);
            if(RemoveDirectory(oldestFolder) == FAIL)
            {
                /* Folder was not delete means some process uses those folder. so retry after some time to overhead process rate. */
                sleep(FOLDER_DELETE_TIMEOUT);
                EntryOldestDeleteFile(oldestFolder);
            }
        }
        else
        {
            EntryOldestDeleteFile(oldestFolder);
        }

        DPRINT(DISK_MANAGER, "remove oldest file info: [actualDeleteSize=%lld], [deletedSize=%lld], [oldestFolder=%s]",
               delFileSize, removeSize, oldestFolder);
        MUTEX_LOCK(cleanUpThreadInfo[cleanupMode].clenupMutex);

    } while((delFileSize > removeSize) && (cleanUpThreadInfo[cleanupMode].threadExit == FALSE));

    cleanUpThreadInfo[cleanupMode].runFlg = FALSE;
    cleanUpThreadInfo[cleanupMode].threadExit = FALSE;
    MUTEX_UNLOCK(cleanUpThreadInfo[cleanupMode].clenupMutex);

    /* Get storage config and if recording running on ndd then update the storage status */
    ReadHddConfig(&hddConfig);
    if (hddConfig.recordDisk != LOCAL_HARD_DISK)
    {
        /* Force update ndd storage status after deleting records */
        DPRINT(DISK_MANAGER, "force update ndd storage status after removing records: [ndd=%d]", hddConfig.recordDisk - REMOTE_NAS1);
        UpdateNddStorageStatus();
    }

    UpdateAllCameraStorage();
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will remove oldest all file which is older than input days.
 * @return  SUCCESS/FAIL
 */
BOOL CleanupRecordFileByDay(void)
{
    CLEANUP_MODE_e cleanupMode = CLEANUP_BY_DAY;

    MUTEX_LOCK(cleanUpThreadInfo[cleanupMode].clenupMutex);
    if(cleanUpThreadInfo[cleanupMode].runFlg == TRUE)
    {
        MUTEX_UNLOCK(cleanUpThreadInfo[cleanupMode].clenupMutex);
        return SUCCESS;
    }

    cleanUpThreadInfo[cleanupMode].runFlg = TRUE;
    cleanUpThreadInfo[cleanupMode].threadExit = FALSE;
    MUTEX_UNLOCK(cleanUpThreadInfo[cleanupMode].clenupMutex);

    if (FAIL == Utils_CreateThread(NULL, remRecordByDay, NULL, DETACHED_THREAD, RM_REC_DAY_THREAD_STACK_SZ))
    {
        MUTEX_LOCK(cleanUpThreadInfo[cleanupMode].clenupMutex);
        cleanUpThreadInfo[cleanupMode].runFlg = FALSE;
        cleanUpThreadInfo[cleanupMode].threadExit = FALSE;
        MUTEX_UNLOCK(cleanUpThreadInfo[cleanupMode].clenupMutex);
        EPRINT(DISK_MANAGER, "fail to create remove record by day thread");
        return FAIL;
    }

    WriteEvent(LOG_STORAGE_EVENT, LOG_HDD_CLEAN_UP, NULL, NULL, EVENT_REGULAR);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will remove all back Up File Older than configured day.
 * @return  SUCCESS/FAIL
 */
BOOL CleanupBkupFileByDay(void)
{
    MUTEX_LOCK(bkupCleanUp.mutex);
    if (bkupCleanUp.state != INACTIVE)
    {
        MUTEX_UNLOCK(bkupCleanUp.mutex);
        EPRINT(DISK_MANAGER, "backup cleanUp thread already running");
        return FAIL;
    }
    bkupCleanUp.state = ACTIVE;
    MUTEX_UNLOCK(bkupCleanUp.mutex);

    if (FAIL == Utils_CreateThread(NULL, remBkUpByDay, NULL, DETACHED_THREAD, RM_BKP_DAY_THREAD_STACK_SZ))
    {
        MUTEX_LOCK(bkupCleanUp.mutex);
        bkupCleanUp.state = INACTIVE;
        MUTEX_UNLOCK(bkupCleanUp.mutex);
        EPRINT(DISK_MANAGER, "fail to create remove backup by day thread");
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is config change notification function. It Checks whether the configuration of FTP
 *          server on which backup is being taken is changed, then it Interrupts cleanup.
 * @param   index
 * @return  SUCCESS/FAIL
 */
BOOL DmFTPUploadCfgUpdate(FTP_SERVER_e index)
{
    MUTEX_LOCK(bkupCleanUp.mutex);
    if (bkupCleanUp.state == ACTIVE)
    {
        /* This is read only variable for this thread, so it accessed without lock at some place */
        if (bkupCleanUp.ftpIndex == index)
        {
            bkupCleanUp.state = INTERRUPTED;
        }
    }
    MUTEX_UNLOCK(bkupCleanUp.mutex);

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is function trigger back up clean up to stop.
 */
void StopBackUpCleanUp(void)
{
    MUTEX_LOCK(bkupCleanUp.mutex);
    if (bkupCleanUp.state == ACTIVE)
    {
        bkupCleanUp.state = INTERRUPTED;
    }
    MUTEX_UNLOCK(bkupCleanUp.mutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This was a thread function. This function will remove oldest file with its folder from
 *          hard disk. In this function if oldest file for two channel was same then it was delete
 *          the lowest channel files. This function create create free space of input size.
 * @param   threadArg
 * @return
 */
static VOIDPTR remRecordByDay(VOIDPTR threadArg)
{
    BOOL				retFromCleanUp = FALSE;
    UINT8				cameraIndex;
    UINT8				diskCnt, totalHddCnt;
    UINT16				olderDay[MAX_CAMERA] = { 0 };
    CHAR				mntPoint[MOUNT_POINT_SIZE];
    CHAR				channelFolder[SYSTEM_COMMAND_SIZE];
    CHAR				olderFolder[SYSTEM_COMMAND_SIZE];
    CLEANUP_MODE_e		cleanUpMode = CLEANUP_BY_DAY;
    STORAGE_CONFIG_t	storageConfig;
    RECOVERY_INFO_t	    recveryInfo = {0};
    HDD_CONFIG_t	    hddConfig;

    THREAD_START("REMOVE_RECORD");

    while(GetDiskNonFuncStatus(MAX_RECORDING_MODE) == TRUE)
    {
        MUTEX_LOCK(cleanUpThreadInfo[cleanUpMode].clenupMutex);
        if(cleanUpThreadInfo[cleanUpMode].threadExit == TRUE)
        {
            MUTEX_UNLOCK(cleanUpThreadInfo[cleanUpMode].clenupMutex);
            retFromCleanUp = TRUE;
            break;
        }
        MUTEX_UNLOCK(cleanUpThreadInfo[cleanUpMode].clenupMutex);
        sleep(FOLDER_DELETE_TIMEOUT);
    }

    if(retFromCleanUp == FALSE)
    {
        ReadHddConfig(&hddConfig);
        ReadStorageConfig(&storageConfig);

        if (storageConfig.recordRetentionType == RECORDING_DRIVE_WISE_RECORD_RETENTION)
        {
            for(cameraIndex = 0; cameraIndex < getMaxCameraForCurrentVariant(); cameraIndex++)
            {
                olderDay[cameraIndex] = storageConfig.driveWiseRecordCleanupByDays;
            }
        }
        else
        {
            for(cameraIndex = 0; cameraIndex < getMaxCameraForCurrentVariant(); cameraIndex++)
            {
                olderDay[cameraIndex] = storageConfig.cameraWiseRecordCleanupByDays[cameraIndex];
            }
        }

        totalHddCnt = GetTotalMediaNo(MAX_RECORDING_MODE);
        for(diskCnt = 0; diskCnt < totalHddCnt; diskCnt++)
        {
            if(GetMountPointFromDiskId(diskCnt, mntPoint, MAX_RECORDING_MODE) == FAIL)
            {
                continue;
            }

            for(cameraIndex = 0; cameraIndex < getMaxCameraForCurrentVariant(); cameraIndex++)
            {
                snprintf(channelFolder, SYSTEM_COMMAND_SIZE, REC_FOLDER_CHNL, mntPoint, GET_CAMERA_NO(cameraIndex));
                while(getFolderOlderThanDay(channelFolder, olderDay[cameraIndex], olderFolder, &recveryInfo) == SUCCESS)
                {
                    if (RemoveIndexesForFolder(mntPoint, cameraIndex, &recveryInfo, TRUE) != SUCCESS) /* TRUE means remove indexes for retention files */
                    {
                        EPRINT(DISK_MANAGER, "fail to remove year map indexes folder: [path=%s]", channelFolder);
                    }

                    if(RemoveDirectory(olderFolder) == FAIL)
                    {
                        /* Folder was not delete means some process uses those folder so retry after some time to overhead process rate. */
                        sleep(FOLDER_DELETE_TIMEOUT);
                    }
                    else
                    {
                        EntryOldestDeleteFile(olderFolder);
                    }

                    MUTEX_LOCK(cleanUpThreadInfo[cleanUpMode].clenupMutex);
                    if(cleanUpThreadInfo[cleanUpMode].threadExit == TRUE)
                    {
                        MUTEX_UNLOCK(cleanUpThreadInfo[cleanUpMode].clenupMutex);
                        retFromCleanUp = TRUE;
                        break;
                    }
                    MUTEX_UNLOCK(cleanUpThreadInfo[cleanUpMode].clenupMutex);
                }

                GenerateLocalRecDatabase(hddConfig.recordDisk, cameraIndex);
                MUTEX_LOCK(cleanUpThreadInfo[cleanUpMode].clenupMutex);
                if(cleanUpThreadInfo[cleanUpMode].threadExit == TRUE)
                {
                    MUTEX_UNLOCK(cleanUpThreadInfo[cleanUpMode].clenupMutex);
                    retFromCleanUp = TRUE;
                    break;
                }
                MUTEX_UNLOCK(cleanUpThreadInfo[cleanUpMode].clenupMutex);
            }

            if (retFromCleanUp == TRUE)
            {
                break;
            }
        }
    }

    MUTEX_LOCK(cleanUpThreadInfo[cleanUpMode].clenupMutex);
    cleanUpThreadInfo[cleanUpMode].runFlg = FALSE;
    cleanUpThreadInfo[cleanUpMode].threadExit = FALSE;
    MUTEX_UNLOCK(cleanUpThreadInfo[cleanUpMode].clenupMutex);

    UpdateAllCameraStorage();
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is a thread function. This function will remove oldest file with its folder from hard disk.
 * @param   threadArg
 * @return
 */
static VOIDPTR remBkUpByDay(VOIDPTR threadArg)
{
    BOOL                        retFromCleanUp = FALSE;
    UINT8                       cameraIndex;
    CHAR                        mntPoint[MOUNT_POINT_SIZE];
    CHAR                        channelFolder[SYSTEM_COMMAND_SIZE];
    CHAR                        olderFolder[SYSTEM_COMMAND_SIZE];
    SCHEDULE_BACKUP_CONFIG_t    scheduleBkupCfg;
    DIR                         *dir;
    struct dirent               *entry;
    struct tm                   refBrokenTime;
    BOOL                        retVal = FAIL;
    RECOVERY_INFO_t             recvryInfo;

    THREAD_START("REMOVE_BACKUP");

    ReadScheduleBackupConfig(&scheduleBkupCfg);
    ReadStorageConfig(&bkupCleanUp.storageCfg);

    if ((scheduleBkupCfg.backupLocation == SB_TO_FTP_SERVER_1) || (scheduleBkupCfg.backupLocation == SB_TO_FTP_SERVER_2))
    {
        /* No other thread is writing this Variable, so it accessed without lock at some place */
        MUTEX_LOCK(bkupCleanUp.mutex);
        bkupCleanUp.ftpIndex = (scheduleBkupCfg.backupLocation - SB_TO_FTP_SERVER_1);
        MUTEX_UNLOCK(bkupCleanUp.mutex);

        ReadSingleFtpUploadConfig(bkupCleanUp.ftpIndex, &bkupCleanUp.ftpCfg);
        do
        {
            if (bkupCleanUp.ftpCfg.ftp == DISABLE)
            {
                break;
            }

            bkupCleanUp.currDirLevel = FTP_DIR_LEVEL_ONE;
            bkupCleanUp.ftpResp = FTP_SUCCESS;

            GetLocalTimeInBrokenTm(&refBrokenTime);
            refBrokenTime.tm_hour = 0;
            refBrokenTime.tm_min = 0;
            refBrokenTime.tm_sec = 0;
            ConvertLocalTimeInSec(&refBrokenTime, &bkupCleanUp.currTime);

            while (TRUE)
            {
                bkupCleanUp.ftpResp = GetFtpServerSystemType(&bkupCleanUp.ftpCfg, &bkupCleanUp.sysType);
                if ((bkupCleanUp.ftpResp == FTP_SUCCESS) && (bkupCleanUp.sysType != MAX_FTP_SYS))
                {
                    break;
                }

                sleep(FTP_OP_RETRY_TIME);
                MUTEX_LOCK(bkupCleanUp.mutex);
                if (bkupCleanUp.state == INTERRUPTED)
                {
                    MUTEX_UNLOCK(bkupCleanUp.mutex);
                    retFromCleanUp = TRUE;
                    break;
                }
                MUTEX_UNLOCK(bkupCleanUp.mutex);
            }

            if (TRUE == retFromCleanUp)
            {
                break;
            }

            while (TRUE)
            {
                bkupCleanUp.ftpResp = ListFromFtp(&bkupCleanUp.ftpCfg, bkupCleanUp.sysType, TRUE, NULL, NULL, ftpListCb);
                if (bkupCleanUp.ftpResp == FTP_SUCCESS)
                {
                    break;
                }

                sleep(FTP_OP_RETRY_TIME);
                MUTEX_LOCK(bkupCleanUp.mutex);
                if (bkupCleanUp.state == INTERRUPTED)
                {
                    MUTEX_UNLOCK(bkupCleanUp.mutex);
                    retFromCleanUp = TRUE;
                    break;
                }
                MUTEX_UNLOCK(bkupCleanUp.mutex);
            }

        }while(0);
    }
    else
    {
        while(TRUE)
        {
            if (scheduleBkupCfg.backupLocation == SB_TO_USB_DEVICE)
            {
                retVal = GetMountPointForBackupDevice(SCHEDULE_BACKUP_DISK, mntPoint);
            }
            else
            {
                retVal = GetPathOfNetworkDrive((scheduleBkupCfg.backupLocation - SB_TO_NETWORK_DRIVE_1), MATRIX_BACKUP_DIR, mntPoint);
            }

            if (retVal == SUCCESS)
            {
                break;
            }

            MUTEX_LOCK(bkupCleanUp.mutex);
            if (bkupCleanUp.state == INTERRUPTED)
            {
                MUTEX_UNLOCK(bkupCleanUp.mutex);
                break;
            }
            EPRINT(DISK_MANAGER, "waiting for backup cleanup thread state to be interrupted: [backupState=%d]", bkupCleanUp.state);
            MUTEX_UNLOCK(bkupCleanUp.mutex);
            sleep(FOLDER_DELETE_TIMEOUT);
        }

        do
        {
            if (FAIL == retVal)
            {
                break;
            }

            DPRINT(DISK_MANAGER, "backup cleanup mount point: [mntPoint=%s]", mntPoint);
            dir = opendir(mntPoint);
            if (dir == NULL)
            {
                break;
            }

            while((entry = readdir(dir)) != NULL)
            {
                if (entry->d_type != DT_DIR)
                {
                    continue;
                }

                /* These are system files */
                if ((strcmp(entry->d_name, ".") == STATUS_OK) || (strcmp(entry->d_name, "..") == STATUS_OK))
                {
                    continue;
                }

                for (cameraIndex = 0; cameraIndex < getMaxCameraForCurrentVariant(); cameraIndex++)
                {
                    snprintf(channelFolder, SYSTEM_COMMAND_SIZE, "%s%s/"CHANNEL_NAME_REC"%02d/", mntPoint, entry->d_name, GET_CAMERA_NO(cameraIndex));

                    while (getFolderOlderThanDay(channelFolder, bkupCleanUp.storageCfg.cameraWiseBackUpCleanupByDays[cameraIndex], olderFolder, &recvryInfo) == SUCCESS)
                    {
                        DPRINT(DISK_MANAGER, "older folder found: [path=%s]", olderFolder);
                        if (RemoveDirectory(olderFolder) == FAIL)
                        {
                            /* Folder was not delete means some process uses those folder. so retry after some time to overhead process rate. */
                            sleep(FOLDER_DELETE_TIMEOUT);
                        }

                        MUTEX_LOCK(bkupCleanUp.mutex);
                        if (bkupCleanUp.state == INTERRUPTED)
                        {
                            MUTEX_UNLOCK(bkupCleanUp.mutex);
                            retFromCleanUp = TRUE;
                            break;
                        }
                        MUTEX_UNLOCK(bkupCleanUp.mutex);
                    }

                    MUTEX_LOCK(bkupCleanUp.mutex);
                    if (bkupCleanUp.state == INTERRUPTED)
                    {
                        MUTEX_UNLOCK(bkupCleanUp.mutex);
                        retFromCleanUp = TRUE;
                        break;
                    }
                    MUTEX_UNLOCK(bkupCleanUp.mutex);
                }

                if (retFromCleanUp == TRUE)
                {
                    break;
                }
            }

            /* Close the directory */
            closedir(dir);

        }while(0);
    }

    MUTEX_LOCK(bkupCleanUp.mutex);
    bkupCleanUp.state = INACTIVE;
    bkupCleanUp.ftpIndex = MAX_FTP_SERVER;
    MUTEX_UNLOCK(bkupCleanUp.mutex);
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function find out oldest folder with respect to day.
 * @param   path
 * @param   refTime
 * @return  SUCCESS/FAIL
 */
static BOOL isFolderOlderThan(CHARPTR path, time_t refTime)
{
    CHARPTR     tmpPath = path;
    CHAR        destFolder[MAX_FILE_NAME_SIZE];
    UINT8       mon;
    UINT64      date;
    UINT64      year;
    time_t      curTimeSec = 0;
    struct tm   currBrokenTime;

    if (ParseStr(&tmpPath, TFP_CHARACTER, destFolder, MAX_FILE_NAME_SIZE) == FAIL)
    {
        return FALSE;
    }

    if (AsciiToInt(destFolder, &date) == FAIL)
    {
        return FALSE;
    }

    if (ParseStr(&tmpPath, TFP_CHARACTER, destFolder, MAX_FILE_NAME_SIZE) == FAIL)
    {
        return FALSE;
    }

    mon = GetMonthNo(destFolder);
    if (mon >= MAX_MONTH)
    {
        return FALSE;
    }

    if (ParseStringGetVal(&tmpPath, &year, 1, '/') == FAIL)
    {
        return FALSE;
    }

    currBrokenTime.tm_sec = 0;
    currBrokenTime.tm_min = 0;
    currBrokenTime.tm_hour = 0;
    currBrokenTime.tm_mday = date;
    currBrokenTime.tm_mon = mon;
    currBrokenTime.tm_year = year;
    RESET_EXTRA_BORKEN_TIME_VAR(currBrokenTime);
    ConvertLocalTimeInSec(&currBrokenTime, &curTimeSec);
    return (curTimeSec >= refTime) ? FALSE : TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function find out oldest folder with respect to day.
 * @param   fromPath
 * @param   olderDay
 * @param   oldFolder
 * @param   recvryInfo
 * @return  SUCCESS/FAIL
 */
static BOOL getFolderOlderThanDay(CHARPTR fromPath, UINT16 olderDay, CHARPTR oldFolder, RECOVERY_INFO_t *recvryInfo)
{
    CHAR 			removeDir[MAX_FILE_NAME_SIZE];
    CHAR 			destFolder[MAX_FILE_NAME_SIZE];
    CHARPTR 		oldChannelFolder;
    UINT8		 	mon;
    UINT64		 	date;
    UINT64		 	year;
    struct tm 		refBrokenTime = {0};
    struct tm 		currBrokenTime;
    time_t 			recTimeSec;
    time_t 			curTimeSec = 0;
    DIR 			*dir;
    struct dirent 	*entry;

    if ((fromPath == NULL) || (oldFolder == NULL))
    {
        return FAIL;
    }

    oldFolder[0] = '\0';
    GetLocalTimeInBrokenTm(&refBrokenTime);
    refBrokenTime.tm_hour = 0;
    refBrokenTime.tm_min = 0;
    refBrokenTime.tm_sec = 0;
    ConvertLocalTimeInSec(&refBrokenTime, &recTimeSec);
    recTimeSec -= (olderDay * HOUR_IN_ONE_DAY * SEC_IN_ONE_HOUR);

    dir = opendir(fromPath);
    if (dir == NULL)
    {
        return FAIL;
    }

    while((entry = readdir(dir)) != NULL)
    {
        /* These are system files */
        if ((strcmp(entry->d_name, ".") == STATUS_OK) || (strcmp(entry->d_name, "..") == STATUS_OK))
        {
            continue;
        }

        oldChannelFolder = entry->d_name;
        snprintf(removeDir, MAX_FILE_NAME_SIZE, "%s%s/", fromPath, entry->d_name);
        if(rmdir(removeDir) == STATUS_OK)
        {
            continue;
        }

        if(ParseStr(&oldChannelFolder, TFP_CHARACTER, destFolder, MAX_FILE_NAME_SIZE) == FAIL)
        {
            continue;
        }

        if(AsciiToInt(destFolder, &date) == FAIL)
        {
            continue;
        }

        if(ParseStr(&oldChannelFolder, TFP_CHARACTER, destFolder, MAX_FILE_NAME_SIZE) == FAIL)
        {
            continue;
        }

        mon = GetMonthNo(destFolder);
        if(mon >= MAX_MONTH)
        {
            continue;
        }

        if(AsciiToInt(oldChannelFolder, &year) == FAIL)
        {
            continue;
        }

        if(fileAvailableInDirectory(removeDir) == FALSE)
        {
            /* SD-2851: Check that folder contains any file or not? If not then skip that folder. Actually if any file
             * available in folder, then rmdir() gives error and then only we reached upto this point. But in CIFS,
             * sometime folder were not removed due to some garbage files made by CIFS and which are only visible on PC contains CIFS server. */
            continue;
        }

        memset(&currBrokenTime, 0, sizeof(currBrokenTime));
        currBrokenTime.tm_mday = date;
        currBrokenTime.tm_mon = mon;
        currBrokenTime.tm_year = year;

        recvryInfo->date = currBrokenTime.tm_mday;
        recvryInfo->mon = currBrokenTime.tm_mon;
        recvryInfo->year = currBrokenTime.tm_year;
        ConvertLocalTimeInSec(&currBrokenTime, &curTimeSec);
        if (curTimeSec >= recTimeSec)
        {
            continue;
        }

        snprintf(oldFolder, SYSTEM_COMMAND_SIZE, "%s", removeDir);
        DPRINT(DISK_MANAGER, "oldest folder found: [path=%s]", oldFolder);
        closedir(dir);
        return SUCCESS;
    }

    closedir(dir);
    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function find out oldest folder with 3 directory depth.
 * @param   deleteVolumeMask
 * @param   oldestFolder
 * @return  SUCCESS/FAIL
 */
static BOOL getOldestFolder(UINT32 deleteVolumeMask, CHARPTR oldestFolder)
{
    BOOL			hourWasZero;
    UINT8			cameraIndex;
    UINT8			oldestDisk = HDD1;
    UINT8			oldestCameraIndex = 0;
    CHAR			mntPoint[MOUNT_POINT_SIZE];
    CHAR			channelFolder[MAX_FILE_NAME_SIZE];
    CHAR		 	removeDir[MAX_FILE_NAME_SIZE];
    CHAR			destFolder[MAX_FILE_NAME_SIZE];
    CHAR			checkOldestFldr[MAX_FILE_NAME_SIZE];
    CHAR			checkOldestCorFldr[MAX_FILE_NAME_SIZE];
    CHARPTR			oldChannelFolder;
    UINT64			date,year,hour;
    UINT8			mon;
    struct tm 		oldestTime;
    struct tm 		currTime;
    time_t 			curSec;
    time_t 			oldestFolderTime = 0;
    time_t 			oldChannelSec = 0;
    UINT32			diskCnt, totalDisk;
    DIR 			*dir;
    struct dirent	*entry;
    RECOVERY_INFO_t	recoveryInfo;

    totalDisk = GetTotalMediaNo(MAX_RECORDING_MODE);
    for(diskCnt = 0; diskCnt < totalDisk; diskCnt++)
    {
        /* Skip current volume if it is not part of the current group */
        if (FALSE == GET_BIT(deleteVolumeMask, diskCnt))
        {
            continue;
        }

        if(GetMountPointFromDiskId(diskCnt, mntPoint, MAX_RECORDING_MODE) == FAIL)
        {
            continue;
        }

        for(cameraIndex = 0; cameraIndex < getMaxCameraForCurrentVariant(); cameraIndex++)
        {
            memset(&oldestTime, 0, sizeof(struct tm));
            hourWasZero = FALSE;
            snprintf(channelFolder, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL, mntPoint,GET_CAMERA_NO(cameraIndex));
            dir = opendir(channelFolder);
            if (dir == NULL)
            {
                continue;
            }

            while((entry = readdir(dir)) != NULL)
            {
                oldChannelFolder = entry->d_name;
                snprintf(removeDir, MAX_FILE_NAME_SIZE, "%s%s", channelFolder, entry->d_name);
                snprintf(checkOldestFldr, MAX_FILE_NAME_SIZE, "%s%s/%s", channelFolder, entry->d_name, SKIPP_FILE);
                snprintf(checkOldestCorFldr, MAX_FILE_NAME_SIZE, "%s%s/", channelFolder, entry->d_name);
                
				/* PARASOFT : No need to validate file path */
                if ((access(checkOldestFldr, F_OK) == STATUS_OK) || (CheckSkipOldestFolder(checkOldestCorFldr) == SUCCESS))
                {
                    EPRINT(DISK_MANAGER, "oldest folder should skip: [path=%s]", checkOldestFldr);
                    continue;
                }

                if(rmdir(removeDir) == 0)
                {
                    continue;
                }

                if(ParseStr(&oldChannelFolder, TFP_CHARACTER, destFolder, MAX_FILE_NAME_SIZE) == FAIL)
                {
                    continue;
                }

                if(AsciiToInt(destFolder,&date) == FAIL)
                {
                    continue;
                }

                if(ParseStr(&oldChannelFolder, TFP_CHARACTER, destFolder, MAX_FILE_NAME_SIZE) == FAIL)
                {
                    continue;
                }

                mon = GetMonthNo(destFolder);
                if(mon >= MAX_MONTH)
                {
                    continue;
                }

                if(AsciiToInt(oldChannelFolder, &year) == FAIL)
                {
                    continue;
                }

                if(fileAvailableInDirectory(removeDir) == FALSE)
                {
                    /* SD-2851: Check that folder contains any file or not? If not then skip that folder. Actually if any file
                     * available in folder, then rmdir() gives error and then only we reached upto this point. But in CIFS,
                     * sometime folder were not removed due to some garbage files made by CIFS and which are only visible on PC contains CIFS server. */
                    continue;
                }

                memset(&currTime, 0, sizeof(currTime));
                currTime.tm_mday = date;
                currTime.tm_mon = mon;
                currTime.tm_year = year;
                ConvertLocalTimeInSec(&currTime, &curSec);

                if((oldChannelSec == 0) || (oldChannelSec > curSec))
                {
                    memcpy(&oldestTime, &currTime, sizeof(struct tm));
                    oldChannelSec = curSec;
                }
            }
            closedir(dir);

            /* PARASOFT : No need to validate file path */
            snprintf(channelFolder, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_FOLDER_DATE_FORMAT DIR_SEP, mntPoint,
                     GET_CAMERA_NO(cameraIndex), oldestTime.tm_mday, GetMonthName(oldestTime.tm_mon), oldestTime.tm_year);
            dir = opendir(channelFolder);
            if (dir == NULL)
            {
                continue;
            }

            while((entry = readdir(dir)) != NULL)
            {
                oldChannelFolder = entry->d_name;
                snprintf(checkOldestFldr, MAX_FILE_NAME_SIZE, "%s%s/%s", channelFolder, entry->d_name, SKIPP_FILE);
                snprintf(checkOldestCorFldr, MAX_FILE_NAME_SIZE, "%s%s/", channelFolder, entry->d_name);
                /* PARASOFT : No need to validate file path */
                if((access(checkOldestFldr, F_OK) == STATUS_OK) || (CheckSkipOldestFolder(checkOldestCorFldr) == SUCCESS))
                {
                    EPRINT(DISK_MANAGER, "oldest folder should skip: [path=%s]", checkOldestFldr);
                    continue;
                }

                if(AsciiToInt(oldChannelFolder, &hour) == FAIL)
                {
                    continue;
                }

                if(fileAvailableInDirectory(checkOldestCorFldr) == FALSE)
                {
                    /* SD-2851: Check that folder contains any file or not? If not then skip that folder. Actually if any file
                     * available in folder, then rmdir() gives error and then only we reached upto this point. But in CIFS,
                     * sometime folder were not removed due to some garbage files made by CIFS and which are only visible on PC contains CIFS server. */
                    continue;
                }

                if(((oldestTime.tm_hour == 0) || (oldestTime.tm_hour > (UINT8)hour)) && (hourWasZero == FALSE))
                {
                    if(hour == 0)
                    {
                        hourWasZero = TRUE;
                    }
                    oldestTime.tm_hour = hour;
                }
            }
            closedir(dir);

            ConvertLocalTimeInSec(&oldestTime, &oldChannelSec);
            if((oldestFolderTime == 0) || (oldestFolderTime > oldChannelSec))
            {
                oldestFolderTime = oldChannelSec;
                oldestCameraIndex = cameraIndex;
                oldestDisk = diskCnt;
            }
        }
    }

    /* SD-2851: If oldest folder time zero, means there is no recording available. To avoid unnecessary
     * processing on folder with default time i.e. 1 Jan 1970 when going to convert 0 sec into BrokenTm. */
    if(oldestFolderTime == 0)
    {
        return FAIL;
    }

    ConvertLocalTimeInBrokenTm(&oldestFolderTime, &oldestTime);
    GetLocalTimeInBrokenTm(&currTime);

    if((currTime.tm_year != oldestTime.tm_year) || (currTime.tm_mday != oldestTime.tm_mday)
            || (currTime.tm_mon != oldestTime.tm_mon) || (currTime.tm_hour != oldestTime.tm_hour))
    {
        HDD_CONFIG_t hddConfig;

        ReadHddConfig(&hddConfig);
        GetMountPointFromDiskId(oldestDisk, mntPoint, MAX_RECORDING_MODE);
        snprintf(oldestFolder, MAX_FILE_NAME_SIZE, REC_FOLDER_CHNL REC_FOLDER_DATE_HOUR_FORMAT, mntPoint,
                 GET_CAMERA_NO(oldestCameraIndex), oldestTime.tm_mday, GetMonthName(oldestTime.tm_mon), oldestTime.tm_year, oldestTime.tm_hour);

        recoveryInfo.date = oldestTime.tm_mday;
        recoveryInfo.mon = oldestTime.tm_mon;
        recoveryInfo.year = oldestTime.tm_year;
        recoveryInfo.hour = oldestTime.tm_hour;
        RemoveIndexesForFolder(mntPoint, oldestCameraIndex, &recoveryInfo, FALSE);  /* FALSE means remove indexes for overwrite old files */
        GenerateLocalRecDatabase(hddConfig.recordDisk, oldestCameraIndex);
        DPRINT(DISK_MANAGER, "oldest folder found: [path=%s]", oldestFolder);
        return SUCCESS;
    }

    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is callback Function to ListFromFtp API, It deletes all files in list.
 * @param   list
 * @param   size
 * @param   userData
 * @return
 */
static BOOL ftpListCb(CHARPTR list, UINT32 size, VOIDPTR userData)
{
    CHARPTR tmpListPtr = list;
    CHAR    entry[FTP_REMOTE_PATH_LEN];
    BOOL    isDir;
    UINT16  outLen;
    UINT8   dirIndex;
    UINT16  channelNo;
    time_t  recTimeSec;
    BOOL    state;

    if ((list == NULL) || (size == 0))
    {
        return SUCCESS;
    }

    /* Check if we need to exit from cleanup */
    MUTEX_LOCK(bkupCleanUp.mutex);
    state = bkupCleanUp.state;
    MUTEX_UNLOCK(bkupCleanUp.mutex);
    if (state == INTERRUPTED)
    {
        return SUCCESS;
    }

    if (bkupCleanUp.currDirLevel == FTP_DIR_LEVEL_ONE)
    {
        bkupCleanUp.currDirLevel++;
        while ((state != INTERRUPTED) && ((tmpListPtr - list) < (INT32)size))
        {
            /* Parse till all entry are resolved */
            if (ParseFTPListReply(&tmpListPtr, entry, FTP_REMOTE_PATH_LEN, &isDir) == FAIL)
            {
                continue;
            }

            for (dirIndex = 0; dirIndex < MAX_LEVEL_ONE_DIRS; dirIndex++)
            {
                if (strcmp(entry, levelOneDirectories[dirIndex]) == STATUS_OK)
                {
                    break;
                }
            }

            if (dirIndex >= MAX_LEVEL_ONE_DIRS)
            {
                continue;
            }

            while (TRUE)
            {
                bkupCleanUp.ftpResp = ListFromFtp(&bkupCleanUp.ftpCfg, bkupCleanUp.sysType, TRUE, entry, (VOIDPTR)entry, ftpListCb);
                if (bkupCleanUp.ftpResp == FTP_SUCCESS)
                {
                    break;
                }

                /* Check if we need to exit from cleanup */
                MUTEX_LOCK(bkupCleanUp.mutex);
                state = bkupCleanUp.state;
                MUTEX_UNLOCK(bkupCleanUp.mutex);
                if (state == INTERRUPTED)
                {
                    break;
                }
                sleep(FTP_OP_RETRY_TIME);
            }
        }

        bkupCleanUp.currDirLevel--;
    }
    else if (bkupCleanUp.currDirLevel == FTP_DIR_LEVEL_TWO)
    {
        bkupCleanUp.currDirLevel++;
        snprintf(entry, FTP_REMOTE_PATH_LEN, "%s", (CHARPTR)userData);
        outLen = strlen(entry);

        while ((state != INTERRUPTED) && ((tmpListPtr - list) < (INT32)size))
        {
            /* Parse till all entry are resolved */
            if (ParseFTPListReply(&tmpListPtr, (entry + outLen), FTP_REMOTE_PATH_LEN, &isDir) == FAIL)
            {
                continue;
            }

            if (sscanf((entry + outLen), CHANNEL_NAME_REC"%02hd/", &channelNo) != 1)
            {
                continue;
            }

            /* Convert 1 to x Range to 0 to x-1 for ease of use */
            channelNo--;
            if (channelNo >= getMaxCameraForCurrentVariant())
            {
                continue;
            }

            bkupCleanUp.channelNo = channelNo;
            while (TRUE)
            {
                bkupCleanUp.ftpResp = ListFromFtp(&bkupCleanUp.ftpCfg, bkupCleanUp.sysType, TRUE, entry, (VOIDPTR)entry, ftpListCb);
                if (bkupCleanUp.ftpResp == FTP_SUCCESS)
                {
                    break;
                }

                /* Check if we need to exit from cleanup */
                MUTEX_LOCK(bkupCleanUp.mutex);
                state = bkupCleanUp.state;
                MUTEX_UNLOCK(bkupCleanUp.mutex);
                if (state == INTERRUPTED)
                {
                    break;
                }
                sleep(FTP_OP_RETRY_TIME);
            }
        }

        bkupCleanUp.currDirLevel--;
    }
    else if (bkupCleanUp.currDirLevel == FTP_DIR_LEVEL_THREE)
    {
        bkupCleanUp.currDirLevel++;
        snprintf(entry, FTP_REMOTE_PATH_LEN, "%s", (CHARPTR)userData);
        outLen = strlen(entry);
        recTimeSec = (bkupCleanUp.currTime -
                      (bkupCleanUp.storageCfg.cameraWiseBackUpCleanupByDays[bkupCleanUp.channelNo] * HOUR_IN_ONE_DAY * SEC_IN_ONE_HOUR));

        while ((state != INTERRUPTED) && ((tmpListPtr - list) < (INT32)size))
        {
            /* Parse till all entry are resolved */
            if (ParseFTPListReply(&tmpListPtr, (entry + outLen), FTP_REMOTE_PATH_LEN, &isDir) == FAIL)
            {
                continue;
            }

            if (isFolderOlderThan((entry + outLen), recTimeSec) == FALSE)
            {
                continue;
            }

            while (TRUE)
            {
                bkupCleanUp.ftpResp = DeleteFromFtp(&bkupCleanUp.ftpCfg, bkupCleanUp.sysType, entry, TRUE, FALSE);
                if (bkupCleanUp.ftpResp == FTP_SUCCESS)
                {
                    break;
                }

                /* Check if we need to exit from cleanup */
                MUTEX_LOCK(bkupCleanUp.mutex);
                state = bkupCleanUp.state;
                MUTEX_UNLOCK(bkupCleanUp.mutex);
                if (state == INTERRUPTED)
                {
                    break;
                }
                sleep(FTP_OP_RETRY_TIME);
            }
        }

        bkupCleanUp.currDirLevel--;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It checks that whether folder contains any file or not
 * @param   dirPath
 * @return  Returns FALSE if no file available else TRUE.
 */
static BOOL fileAvailableInDirectory(CHARPTR dirPath)
{
    CHAR            subDir[MAX_FILE_NAME_SIZE];
    DIR             *dir;
    struct dirent   *entry;

    /* PARASOFT : No need to validate file path */
    dir = opendir(dirPath);
    if (dir == NULL)
    {
        return FALSE;
    }

    while((entry = readdir(dir)) != NULL)
    {
        if((strcmp(entry->d_name,".") == STATUS_OK) || (strcmp(entry->d_name,"..") == STATUS_OK))
        {
            continue;
        }

        if(entry->d_type == DT_REG)
        {
            closedir(dir);
            return TRUE;
        }

        if(entry->d_type != DT_DIR)
        {
            continue;
        }

        /* Recursive call */
        snprintf(subDir, MAX_FILE_NAME_SIZE, "%s/%s", dirPath,entry->d_name);
        if (fileAvailableInDirectory(subDir) == TRUE)
        {
            closedir(dir);
            return TRUE;
        }
    }

    closedir(dir);
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get status format media thread
 * @return  Status
 */
static BOOL getFormatThreadStatus(void)
{
    MUTEX_LOCK(formatInfo.formatMutex);
    BOOL status = formatInfo.threadActive;
    MUTEX_UNLOCK(formatInfo.formatMutex);
    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set status format media thread
 * @param   status
 */
static void setFormatThreadStatus(BOOL status)
{
    MUTEX_LOCK(formatInfo.formatMutex);
    formatInfo.threadActive = status;
    MUTEX_UNLOCK(formatInfo.formatMutex);
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
